/* 
 * $Id: counter.c,v 1.43 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        counter.c
 * Purpose:     Software Counter Collection module.
 *
 */

#include <shared/bsl.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/counter.h>
#include <soc/sbx/qe2000_counter.h>
#include <soc/sbx/sirius_counter.h>
#include <soc/sbx/sirius_counter.h>
#include <soc/sbx/caladan3_counter.h>
#include <bcm/stat.h>

#ifdef BCM_QE2000_SUPPORT
#include <soc/sbx/hal_ka_auto.h>
#endif

#define SBX_COUNTER_QM_COUNT 32

#define COUNTER_IDX_PORTBASE(unit, port) \
        ((port) * SOC_CONTROL(unit)->counter_perport)

#define COUNTER_IDX_GET(unit, ctr_ref, port) \
        (COUNTER_IDX_PORTBASE(unit, port) + \
         SOC_REG_CTR_IDX(unit, (ctr_ref)->reg) + (ctr_ref)->index)

/* Per port mapping to counter map structures */
#define PORT_CTR_REG(unit, port, idx) \
        (&SOC_CONTROL(unit)->counter_map[port]->cmap_base[idx])

#define PORT_CTR_NUM(unit, port) \
        (SOC_CONTROL(unit)->counter_map[port]->cmap_size)

/*
 * Atomic operation
 */
#define COUNTER_ATOMIC_DEF         int
#define COUNTER_ATOMIC_BEGIN(s)    ((s) = sal_splhi())
#define COUNTER_ATOMIC_END(s)      (sal_spl(s))


/*
 * Counter Data
 *
 * Contains software accumulated values on a counter.
 */
typedef struct _counter_data_s {
    uint64  prev;   /* Last read value of counter */
    uint64  val;    /* Software accumulated counter */
    uint64  delta;  /* Delta over last two collections */
} _counter_data_t;

typedef _counter_data_t    *_counter_set_t;  /* Data buffer to a counter set */


/*
 * Counter Block Info
 *
 * Contains definition for a set of counters to collect statistics for
 * as well as the buffer where the data is stored.
 *
 * The counter block has the following fields:
 *
 *   info - Defines the set of counters in the block, the total number of
 *          available counter-sets in the block, and the number of counters
 *          in a set.
 *
 *   sets - Buffer where data is stored.  Counter collects data on
 *          those counter sets that were added by the driver
 *          (all counter sets are NULL during initialization).
 */
typedef struct _counter_block_info_s {
    soc_sbx_counter_block_info_t  *info;  /* Information on counter block */
    _counter_set_t                *sets;  /* Array of counter sets buffer */
} _counter_block_info_t;

/*
 * Counter Block
 *
 * Contains information on the Counter Collection module on a given unit.
 */
typedef struct _counter_block_s {
    _counter_block_info_t *block;      /* Counter blocks   */
    int                   num_blocks;  /* Number of blocks */
} _counter_block_t;


#define COUNTER_CONTROL(_unit)             (SOC_CONTROL(_unit)->counter_buf32)
#define COUNTER_BLOCK(_unit)               (((_counter_block_t *)COUNTER_CONTROL(_unit))->block)
#define COUNTER_NUM_BLOCKS(_unit)  \
  (((_counter_block_t *)COUNTER_CONTROL(_unit))->num_blocks)

#define COUNTER_BLOCK_INFO(_unit, _block)  (COUNTER_BLOCK(_unit)[_block].info)
#define COUNTER_BLOCK_SETS(_unit, _block)  (COUNTER_BLOCK(_unit)[_block].sets)

#define COUNTER_BLOCK_NUM_SETS(_unit, _block)  \
    COUNTER_BLOCK_INFO(_unit, _block)->num_sets

#define COUNTER_BSET_NUM_COUNTERS(_unit, _block)  \
    COUNTER_BLOCK_INFO(_unit, _block)->num_counters

#define COUNTER_BSET(_unit, _block, _set)  \
    (COUNTER_BLOCK_SETS(_unit, _block)[_set])

#define COUNTER_DATA(_unit, _block, _set, _counter)  \
    (COUNTER_BSET(_unit, _block, _set)[_counter])

#define COUNTER_READ(_unit, _block, _set, _counter, _val, _width)    \
    (COUNTER_BLOCK_INFO(_unit, _block)->read(_unit, _set, _counter,  \
                                             _val, _width))

#define COUNTER_WRITE(_unit, _block, _set, _counter, _val)  \
    (COUNTER_BLOCK_INFO(_unit, _block)->write(_unit, _set, _counter, _val))

/*
 * General Utility Macros
 */
#define UNIT_VALID_CHECK(_unit) \
    if (((_unit) < 0) || ((_unit) >= SOC_MAX_NUM_DEVICES)) { \
        return SOC_E_UNIT; \
    }

#define UNIT_INIT_CHECK(_unit)  \
    do { \
        UNIT_VALID_CHECK(_unit);  \
        if (COUNTER_CONTROL(_unit) == NULL) { return SOC_E_INIT; } \
        if (COUNTER_BLOCK(_unit) == NULL) { return SOC_E_INIT; } \
    } while (0)


#define BCMSIM_STAT_TIMEOUT       200000000

#if defined (BCM_CALADAN3_SUPPORT)
extern void sbx_c3_gather_hw_counters(int unit);
#endif

/*
 * Function:
 *     _soc_sbx_counter_bset_remove_all
 * Purpose:
 *     Remove all counter sets for given unit.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
STATIC int
_soc_sbx_counter_bset_remove_all(int unit)
{
    int  block = 0;
    int  set = 0;

    for (block = 0; block< COUNTER_NUM_BLOCKS(unit); block++) {
        for (set = 0; set < COUNTER_BLOCK_NUM_SETS(unit, block); set++) {
            if (COUNTER_BSET(unit, block, set)) {
                sal_free(COUNTER_BSET(unit, block, set));
                COUNTER_BSET(unit, block, set) = NULL;
            }
        }
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_counter_bset_add
 * Purpose:
 *     Add given counter set for the thread collector to gather statistics on.
 * Parameters:
 *     unit  - Device number
 *     block - Counter block
 *     set   - Counter set to collect statistics on
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_counter_bset_add(int unit, int block, int set)
{
    int              n_bytes = 0;
    _counter_data_t  *buffer = NULL;

    UNIT_INIT_CHECK(unit);

    COUNTER_LOCK(unit);

    if ((block < 0) || (block >= COUNTER_NUM_BLOCKS(unit))) {
        COUNTER_UNLOCK(unit);        
        return SOC_E_PARAM;
    }

    if ((set < 0) || (set >= COUNTER_BLOCK_NUM_SETS(unit, block))) {
        COUNTER_UNLOCK(unit);        
        return SOC_E_PARAM;
    }

    if (COUNTER_BSET(unit, block, set) != NULL) {
        COUNTER_UNLOCK(unit);        
        return SOC_E_NONE;
    }

    n_bytes = COUNTER_BSET_NUM_COUNTERS(unit, block) * sizeof(_counter_data_t);
    if ((buffer = sal_alloc(n_bytes, "soc_counter_data")) == NULL) {
        COUNTER_UNLOCK(unit);
        return SOC_E_MEMORY;
    }

    sal_memset(buffer, 0, n_bytes);
    COUNTER_BSET(unit, block, set) = buffer;

    COUNTER_UNLOCK(unit);

    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_sbx_counter_bset_collect_autozero
 * Purpose:
 *     Collect statistics for specified qe2000 counter set.
 *     We are attempting to reuse Macros, structures, etc
 *     QE2000 hardware returns two counters at a time so we collect all at once
 *     and then process down to sw array
 * Parameters:
 *     unit    - Device number
 *     discard - If true, the software counters are not updated; this
 *               results in only synchronizing the previous count buffer
 *     block   - Counter block
 *     set     - Counter set in given block to collect statistics on
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
STATIC int
_soc_sbx_counter_bset_collect_autozero(int unit, int discard, int block, int set)
{
    _counter_data_t  *data = NULL;
    int              counter = 0;
    int              num_counters = 0;
    int              width = 0;
    uint64           val_new = COMPILER_64_INIT(0,0);
    uint64           val_prev = COMPILER_64_INIT(0,0);
    uint64           val_diff = COMPILER_64_INIT(0,0);
    uint64  raw_data[SBX_COUNTER_QM_COUNT];

    /* Check for empty counter sets */
    if (COUNTER_BSET(unit, block, set) == NULL) {
        return SOC_E_PARAM;
    }

    num_counters = COUNTER_BSET_NUM_COUNTERS(unit, block);

    /* Read array of hardware count values */

    counter = 0;
    SOC_IF_ERROR_RETURN
            (COUNTER_READ(unit, block, set, counter, &raw_data[0], &width));

    for (counter = 0; counter < num_counters; counter++) {
        volatile uint64 *vptr;
        COUNTER_ATOMIC_DEF s;

        val_new = raw_data[counter];  /* the appropriate collected data to the  traditional processing element */


        data = &(COUNTER_DATA(unit, block, set, counter));

        /* Store prev value */
        /* Atomic because soc_sbx_counter_set may update 64-bit value */
        COUNTER_ATOMIC_BEGIN(s);
        val_prev = data->prev;
        COUNTER_ATOMIC_END(s);

 LOG_VERBOSE(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "cntr %d/%d/%d val_new %x%08x %x%08x. %x... "),
              set, counter, num_counters, COMPILER_64_HI(val_new), COMPILER_64_LO(val_new), COMPILER_64_HI(val_prev), COMPILER_64_LO(val_prev), (uint32)data));

        /* QE2000 Counters are zeroed on read so we always add the value */

        if (discard) {
            /* prev = new; delta = 0 */
            /* Update the previous value buffer */
            COUNTER_ATOMIC_BEGIN(s);
            data->prev = val_new;
            COMPILER_64_ZERO(data->delta);
            COUNTER_ATOMIC_END(s);
            LOG_INFO(BSL_LS_SOC_COUNTER,
                     (BSL_META_U(unit,
                                 "discard")));
            continue;
        }

        LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                    (BSL_META_U(unit,
                                "soc_sbx_counter_collect: ctr %d => 0x%08x_%08x\n"),
                     counter,
                     COMPILER_64_HI(val_new), COMPILER_64_LO(val_new)));

        vptr = &data->val;

        val_diff = val_new;


        COUNTER_ATOMIC_BEGIN(s);
        data->prev  = val_new;
        COMPILER_64_ADD_64(*vptr, val_diff);
        data->delta = val_diff;
        COUNTER_ATOMIC_END(s);

        /*
         * Allow other tasks to run between processing each port.
         */
        sal_thread_yield();
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_sbx_counter_bset_clear
 * Purpose:
 *     Clear soft statistics for specified a block of counters.
 *
 * Parameters:
 *     unit    - Device number
 *     discard - If true, the software counters are not updated; this
 *               results in only synchronizing the previous count buffer
 *     block   - Counter block
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
STATIC int
_soc_sbx_counter_bset_clear(int unit, int block)
{
    _counter_data_t  *data = NULL;
    int              counter = 0;
    int              num_counters = 0;
    int              set = 0;
   

    num_counters = COUNTER_BSET_NUM_COUNTERS(unit, block);

    for (set = 0; set < COUNTER_BLOCK_NUM_SETS(unit, block); set++) {
        /* Check for empty counter sets */
        if (COUNTER_BSET(unit, block, set) == NULL) {
            continue;
        }

        for (counter = 0; counter < num_counters; counter++) {
            COUNTER_ATOMIC_DEF s;

            data = &(COUNTER_DATA(unit, block, set, counter));

            COUNTER_ATOMIC_BEGIN(s);
            COMPILER_64_ZERO(data->val);
            COMPILER_64_ZERO(data->prev);
            COMPILER_64_ZERO(data->delta);
            COUNTER_ATOMIC_END(s);

            /*
             * Allow other tasks to run between processing each port.
             */
        }
        sal_thread_yield();
    }

    return SOC_E_NONE;
}



/*
 * Function:
 *     _soc_sbx_counter_block_init
 * Purpose:
 *     Allocate and initialize the counter blocks for given unit.
 * Parameters:
 *     unit        - Device number
 *     block_info  - Array of counter block information
 *     block_count - Number of counter blocks in device
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     All counter sets in unit are set to NULL.
 *     Driver needs to 'add' the counter sets that it wants the
 *     collector to gather statistics on.
 */
STATIC int
_soc_sbx_counter_block_init(int unit,
                            soc_sbx_counter_block_info_t *block_info,
                            int block_count)
{
    int  i = 0;
    int  n_bytes = 0;

    if (COUNTER_BLOCK(unit)) {
        return SOC_E_NONE;
    }
        
    /* Allocate array of counter blocks */
    COUNTER_BLOCK(unit) = sal_alloc(block_count * sizeof(_counter_block_info_t),
                                    "soc_counter_blocks");
    if (COUNTER_BLOCK(unit) == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(COUNTER_BLOCK(unit), 0, block_count * sizeof(_counter_block_info_t));

    /*
     * Initialize counter blocks
     *
     * NOTE:  Actual buffer for the data is allocated later on
     *        when a counter block set is added.
     */
    for (i = 0; i < block_count; i++) {
        /* Set block information */
        COUNTER_BLOCK_INFO(unit, i) = &block_info[i];

        /* Allocate only the array of pointers to counter sets */
        n_bytes = block_info[i].num_sets * sizeof(_counter_set_t);
        COUNTER_BLOCK_SETS(unit, i) = sal_alloc(n_bytes,
                                                "soc_counter_sets");

        if (COUNTER_BLOCK_SETS(unit, i) == NULL) {
            /* Free previous block memory allocated, and break */
            while (--i >= 0) {
                sal_free(COUNTER_BLOCK_SETS(unit, i));
            }
            sal_free(COUNTER_BLOCK(unit));
            COUNTER_BLOCK(unit) = NULL;
            return SOC_E_MEMORY;
        }

        sal_memset(COUNTER_BLOCK_SETS(unit, i), 0, n_bytes); 
    }
    
    COUNTER_NUM_BLOCKS(unit) = block_count;

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_counter_init
 * Purpose:
 *     Initialize software counter collection module.
 * Parameters:
 *     unit        - Device number
 *     block_info  - Array of counter block information
 *     block_count - Number of counter blocks
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     The counter module state is as follows, after a successful 'attach':
 *     - Collector thread is not running
 *     - Counter block sets are NULL (this means, caller must
 *       'add' the counter sets that it wants the collector to
 *       gather statistics on.
 */
int
soc_sbx_counter_init(int unit,
                     soc_sbx_counter_block_info_t *block_info,
                     int block_count)
{
    int             rv = SOC_E_NONE;
    int             block;
    int             i;
    soc_control_t  *soc = NULL;

    UNIT_VALID_CHECK(unit);

    soc = SOC_CONTROL(unit);

    /* Create Mutex lock */
    if (soc->counterMutex == NULL) {
        if ((soc->counterMutex = sal_mutex_create("soc_counter_lock")) == NULL) {
            return SOC_E_MEMORY;
        }
    }

    COUNTER_LOCK(unit);

    /*
     * Counter Handler
     *
     * Hijack the counter_buf32 from the soc_control
     * structure. If counter_buf32 and counter_buf64
     * are equal, then initialize counter_buf32.
     * SBX devices do not use counter_buf32.
     */

    if (soc->counter_buf32 != NULL) {

        for (block = 0; block < COUNTER_NUM_BLOCKS(unit); block++) {
            for (i = 0; i < CALADAN3_COUNTER_BLOCK_QM_NUM_SETS; i++) {
                sal_free(COUNTER_BSET(unit, block, i));
            }
        }
        for (i = 0; i < COUNTER_NUM_BLOCKS(unit); i++) {
            sal_free(COUNTER_BLOCK_SETS(unit, i));
        }
        sal_free(COUNTER_BLOCK(unit));

        soc_cm_sfree(unit, soc->counter_buf32);

        soc->counter_buf32 = NULL;
        soc->counter_buf64 = NULL;

    }

    /* soc->counter_buf32 is the same pointer as COUNTER_CONTROL()
     */
    COUNTER_CONTROL(unit) = soc_cm_salloc(unit, sizeof(_counter_block_t),
                                          "soc_counter_control");
    if (COUNTER_CONTROL(unit) == NULL) {
        rv = SOC_E_MEMORY;
        goto error;
    }
    sal_memset(COUNTER_CONTROL(unit), 0, sizeof(_counter_block_t));


    soc->counter_buf64 = (uint64 *)&soc->counter_buf32[0];
    sal_memset(COUNTER_CONTROL(unit), 0, sizeof(_counter_block_t));
    
    soc->counter_pid = SAL_THREAD_ERROR;
    soc->counter_interval  = 0;
    soc->counter_trigger   = NULL;
    soc->counter_intr      = NULL;
    
    /* Initialize counter block information */
    rv = _soc_sbx_counter_block_init(unit, block_info, block_count);
    if (SOC_FAILURE(rv)) {
        rv = SOC_E_MEMORY;
        goto error;
    }

    COUNTER_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit=%d rv=%d\n"),
                 FUNCTION_NAME(), unit, rv));


    return rv;

error:
    _soc_sbx_counter_bset_remove_all(unit);

    if (soc->counter_buf32 != NULL) {
        soc_cm_sfree(unit, soc->counter_buf32);
        soc->counter_buf32 = NULL;
        soc->counter_buf64 = NULL;
    }

    COUNTER_UNLOCK(unit);
 

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s failed: unit=%d rv=%d\n"),
                 FUNCTION_NAME(), unit, rv));

    return rv;
}


/*
 * Function:
 *     soc_sbx_counter_detach
 * Purpose:
 *     Stop and deallocate software counter collection module.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Stop counter task if running.
 *     Deallocate counter collection buffers.
 *     Destroy handler semaphores.
 *     Deallocate counter control handler.
 */
int
soc_sbx_counter_detach(int unit)
{
    int             block = 0;
    soc_control_t   *soc = NULL;
    int             rv = SOC_E_NONE;

    UNIT_INIT_CHECK(unit);

    soc = SOC_CONTROL(unit);

    rv = soc_sbx_counter_stop(unit);
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    COUNTER_LOCK(unit);

    if (soc->counter_hw_val != NULL) {
        sal_free(soc->counter_hw_val);
        soc->counter_hw_val = NULL;
    }

    if (soc->counter_sw_val != NULL) {
        sal_free(soc->counter_sw_val);
        soc->counter_sw_val = NULL;
    }

    if (soc->counter_delta != NULL) {
        sal_free(soc->counter_delta);
        soc->counter_delta = NULL;
    }

    /* Deallocate all counter block sets */
    _soc_sbx_counter_bset_remove_all(unit);

    /* Deallocate array of pointers to counter sets in each block */
    for (block = 0; block< COUNTER_NUM_BLOCKS(unit); block++) {
        sal_free(COUNTER_BLOCK_SETS(unit, block));
    }

    /* Deallocate counter block */
    sal_free(COUNTER_BLOCK(unit));
    COUNTER_BLOCK(unit) = NULL;

    /* Destroy semaphores */
    if ((soc->counter_trigger) != NULL) {
        sal_sem_destroy(soc->counter_trigger);
        soc->counter_trigger = NULL;
    }
    if ((soc->counter_intr) != NULL) {
        sal_sem_destroy(soc->counter_intr);
        soc->counter_intr = NULL;
    }

    if ((COUNTER_CONTROL(unit) != NULL) || 
        (soc->counter_buf64 != NULL)) {

        if (COUNTER_CONTROL(unit) == (uint32 *) soc->counter_buf64) {

            soc_cm_sfree(unit, soc->counter_buf32);
            soc->counter_buf32 = NULL;
            soc->counter_buf64 = NULL;
        } else {
            if (COUNTER_CONTROL(unit) != NULL) {

                sal_free(COUNTER_CONTROL(unit));
                COUNTER_CONTROL(unit) = NULL;
            }
            if (soc->counter_buf64 != NULL) {

                soc_cm_sfree(unit, soc->counter_buf64);
                soc->counter_buf64 = NULL;
            }
        }
    }
    
    COUNTER_UNLOCK(unit);

    sal_mutex_destroy(soc->counterMutex);
    soc->counterMutex = NULL;


    return SOC_E_NONE;
}



#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
/*
 * Function:
 *      soc_sbx_counter_collect64
 * Purpose:
 *      This routine gets called each time the counter transfer has
 *      completed a cycle.  It collects the 64-bit counters.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      It computes the deltas in the hardware counters from the last
 *      time it was called and adds them to the high resolution (64-bit)
 *      software counters in counter_sw_val[].  It takes wrapping into
 *      account for counters that are less than 64 bits wide.
 *      It also computes counter_delta[].
 */

static int
soc_sbx_counter_collect64(int unit, int discard)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_port_t          port = 0;
    soc_reg_t           ctr_reg = 0;
    uint64              ctr_new = COMPILER_64_INIT(0,0), 
      ctr_prev = COMPILER_64_INIT(0,0), ctr_diff = COMPILER_64_INIT(0,0);
    int                 index = 0;
    int                 port_base = 0, port_base_dma  COMPILER_ATTRIBUTE((unused));
    int                 ar_idx = 0;

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                (BSL_META_U(unit,
                            "soc_sbx_counter_collect64: unit=%d discard=%d\n"),
                 unit, discard));


    PBMP_ITER(soc->counter_pbmp, port) {

        if (SOC_IS_SIRIUS(unit) && (!(IS_GX_PORT(unit, port)))) {
            continue;
        } else if (SOC_IS_CALADAN3(unit) &&
                   (!((IS_E_PORT(unit, port) || IS_HG_PORT(unit, port))))) {
            continue;
        }
        
        if (SAL_BOOT_BCMSIM && (port > 3)) {
            continue;
        }
        
        if (SOC_IS_CALADAN3(unit) && SAL_BOOT_BCMSIM) {
            continue;
        }

	/*
	 * counter_ports32 is non-dma base start, which is 0 for Sirius
	 */

        port_base = COUNTER_IDX_PORTBASE(unit, port);
        port_base_dma = COUNTER_IDX_PORTBASE(unit, port - soc->counter_ports32);

        for (index = 0; index < PORT_CTR_NUM(unit, port); index++) {
            volatile uint64 *vptr = NULL;
            COUNTER_ATOMIC_DEF s = 0;

            ctr_reg = PORT_CTR_REG(unit, port, index)->reg;
            ar_idx = PORT_CTR_REG(unit, port, index)->index;

            if (SOC_COUNTER_INVALID(unit, ctr_reg)) {
                continue;
            }

            /* Atomic because soc_counter_set may update 64-bit value */
            COUNTER_ATOMIC_BEGIN(s);
            ctr_prev = soc->counter_hw_val[port_base + index];
            COUNTER_ATOMIC_END(s);

            SOC_IF_ERROR_RETURN
                (soc_reg_get(unit, ctr_reg, port, ar_idx, &ctr_new));

            if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                continue;
            }

            if (discard) {

                COUNTER_ATOMIC_BEGIN(s);

                /* Update the previous value buffer */
                soc->counter_hw_val[port_base + index] = ctr_new;
                soc->counter_sw_val[port_base + index] = soc->counter_hw_val[port_base + index];
                COMPILER_64_ZERO(soc->counter_delta[port_base + index]);
                COUNTER_ATOMIC_END(s);

                continue;
            }

            LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                        (BSL_META_U(unit,
                                    "soc_sbx_counter_collect64: ctr %d => 0x%08x_%08x\n"),
                         port_base + index,
                         COMPILER_64_HI(ctr_new), COMPILER_64_LO(ctr_new)));

            vptr = &soc->counter_sw_val[port_base + index];

            ctr_diff = ctr_new;

            if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                int             width;
                uint64          wrap_amt;

                /*
                 * Counter must have wrapped around.
                 * Add the proper wrap-around amount.
                 */
                width = SOC_REG_INFO(unit, ctr_reg).fields[0].len;
                if (width < 32) {
                    COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                } else if (width < 64) {
                    COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                }
            }

            COMPILER_64_SUB_64(ctr_diff, ctr_prev);

            COUNTER_ATOMIC_BEGIN(s);
            COMPILER_64_ADD_64(*vptr, ctr_diff);
            soc->counter_delta[port_base + index] = ctr_diff;
            soc->counter_hw_val[port_base + index] = ctr_new;
            COUNTER_ATOMIC_END(s);

        }

        /*
         * Allow other tasks to run between processing each port.
         */
        sal_thread_yield();
    }


    return SOC_E_NONE;
}

#if defined(BCM_CALADAN3_SUPPORT)
/*
 * Function:
 *      soc_sbx_controlled_counters_collect64
 * Purpose:
 *      This routine gets called each time the counter transfer has
 *      completed a cycle.  It collects the 64-bit controlled counters.
 */
INLINE int
soc_sbx_controlled_counters_collect64(int unit, int discard)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_port_t          port;
    uint64              ctr_new[SOC_SBX_CALADAN3_IL_NUM_CHANNEL_STATS]; /* per channel + aggregate at 0 */
    int                 index, port_base, channel_index, num_channels;
    soc_controlled_counter_t* ctr;

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                        (BSL_META_U(unit, "soc_controlled_counters_collect64: unit=%d "
                                    "discard=%d\n"), unit, discard));

    if (!soc_feature(unit, soc_feature_controlled_counters)) {
        return SOC_E_NONE;
    }

    PBMP_ITER(soc->counter_pbmp, port) {
        if (!soc->controlled_counters) {
            break;
        }
        for (index = 0; soc->controlled_counters[index].controlled_counter_f != NULL; index++) {
            volatile uint64 *vptr;
            COUNTER_ATOMIC_DEF s;

            ctr = &soc->controlled_counters[index];

            if(!COUNTER_IS_COLLECTED(soc->controlled_counters[index])) {
                continue;
            }

            ctr->controlled_counter_f(unit, ctr->counter_id, port, &ctr_new[0]);
    
            /* Counters are Clear-on-Read. Add ctr_new to 
             * previously stored sw value if discard is not set
             * NOTE: assuming index 0 is the aggregate counter
             */
            if (COMPILER_64_IS_ZERO(ctr_new[0])) {
                continue;
            }

            if (IS_IL_PORT(unit, port)) {
                num_channels = SOC_SBX_CALADAN3_IL_NUM_CHANNEL_STATS;
            } else {
                num_channels = 1;
            }

            port_base = COUNTER_IDX_PORTBASE(unit, port);

            if (discard) {
                /* Update the previous value buffer */
                COUNTER_ATOMIC_BEGIN(s);
                for (channel_index = 0; channel_index < num_channels; channel_index++) {
                    soc->counter_hw_val[port_base + (ctr->counter_idx * num_channels)] = ctr_new[channel_index];
                    COMPILER_64_ZERO(soc->counter_delta[port_base + (ctr->counter_idx * num_channels)]);
                }
                COUNTER_ATOMIC_END(s);
                continue;
            }

            for (channel_index = 0; channel_index < num_channels; channel_index++) {
                if (COMPILER_64_IS_ZERO(ctr_new[channel_index])) {
                    continue;
                }
                LOG_INFO(BSL_LS_SOC_COUNTER,
                         (BSL_META_U(unit, "soc_sbx_controlled_counters_collect64: ctr(index %d) %d => "
                                     "0x%08x_%08x\n"), port_base + ctr->counter_idx * num_channels, index,
                          COMPILER_64_HI(ctr_new[channel_index]), COMPILER_64_LO(ctr_new[index])));
            }

            vptr = &soc->counter_sw_val[port_base + (ctr->counter_idx * num_channels)];
            COUNTER_ATOMIC_BEGIN(s);
            for (channel_index = 0; channel_index < num_channels; channel_index++, vptr++) {
                COMPILER_64_ADD_64(*vptr, ctr_new[channel_index]);
                soc->counter_delta[port_base + ctr->counter_idx * num_channels] = ctr_new[channel_index];
                soc->counter_hw_val[port_base + ctr->counter_idx * num_channels] = ctr_new[channel_index];
            }
            COUNTER_ATOMIC_END(s);
        }

        /* If signaled to exit then return  */
        if (!soc->counter_interval) {
            return SOC_E_NONE;
        }
    
        /* Allow other tasks to run between processing each port */
        sal_thread_yield();
    }

    return SOC_E_NONE;
}
#endif
#endif


/*
 * Function:
 *     soc_sbx_sirius_process_custom_stats
 * Purpose:
 *     Process the Custom Stats Memory
 *
 * Parameters:
 *     unit     - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_process_custom_stats(int unit, int links)
{
      int rv = SOC_E_NONE;
      soc_sbx_config_t *sbx = SOC_SBX_CFG(unit);
      uint32 *bp = NULL;
      uint32 regval0 = 0, regval1 = 0, portlist = 0;
      int type = 0, base = 0, ports = 0;
#ifdef BCM_QE2000_SUPPORT
      int link = 0, bip = 0, rx_err = 0, lost_time = 0, min_sfi = 0;
#endif
      int8 v1 = 0, v2 = 0;

      /*
       * Only update statistic if the field is configured in the custom stats array
       */

      for (type = 0; type <= (snmpBcmCustomReceive8-snmpBcmCustomReceive0); type++) {
	  for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
	      portlist |= sbx->soc_sbx_dbg_cntr_rx[type].ports[base];
	  }
      }

      /* 
       * Nothing to process
       */
      if (portlist == 0) {
	  return SOC_E_NONE;
      }

      for (ports = 0; ports < links; ports++) {
	  /* 
	   * If port not in custom stat list, avoid the register read
	   */
	  if ((portlist & (1 << ports)) == 0) {
	      continue;
	  }
	  if (SOC_IS_SBX_SIRIUS(unit)) {
	      regval0 = regval1 = 0;
	      if (ports < 12) {
		  SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_ERROR0r(unit, ports, &regval0));
		  SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_ERROR1r(unit, ports, &regval1));
	      } else {
		  SOC_IF_ERROR_RETURN(READ_SI_ERROR0r(unit, ports - 12, &regval0));
		  SOC_IF_ERROR_RETURN(READ_SI_ERROR1r(unit, ports - 12, &regval1));
	      }
	      for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
		  bp = (uint32 *) &sbx->custom_stats[base * links];
		  switch(base) {
		      case 0:
			  bp[ports] += soc_reg_field_get(unit, SC_TOP_SI_ERROR0r, regval0, RX_ERR_CNTf);
			  break;
		      case 1:
			  v1 = soc_reg_field_get(unit, SC_TOP_SI_ERROR0r, regval0, LOST_TIME_ALIGNMENT_EVEN_EVENTf);
			  bp[ports] += v1;
			  break;
		      case 2:
			  v2 = soc_reg_field_get(unit, SC_TOP_SI_ERROR0r, regval0, LOST_TIME_ALIGNMENT_ODD_EVENTf);
			  bp[ports] += v2;
			  break;
		      case 3:
			  bp[ports] += soc_reg_field_get(unit, SC_TOP_SI_ERROR1r, regval1, BIP_EVEN_ERR_CNTf);
			  break;
		      case 4:
			  bp[ports] += soc_reg_field_get(unit, SC_TOP_SI_ERROR1r, regval1, BIP_ODD_ERR_CNTf);
			  break;
		  }
	      }
	      /* 
	       * If error bits are set, then write register to clear,
	       * otherwise, avoid register access
	       */
	      if (v1 + v2) {
		  regval0 = 0;
		  soc_reg_field_set(unit, SC_TOP_SI_ERROR0r, &regval0, LOST_TIME_ALIGNMENT_EVEN_EVENTf, 1);
		  soc_reg_field_set(unit, SC_TOP_SI_ERROR0r, &regval0, LOST_TIME_ALIGNMENT_ODD_EVENTf, 1);
		  if (ports < 12) {
		      SOC_IF_ERROR_RETURN(WRITE_SC_TOP_SI_ERROR0r(unit, ports, regval0));
		  } else {
		      SOC_IF_ERROR_RETURN(WRITE_SI_ERROR0r(unit, ports - 12, regval0));
		  }
	      }
	  } 
#ifdef BCM_QE2000_SUPPORT
	  else if (SOC_IS_SBX_QE2000(unit)) { 
	      
	      min_sfi = SOC_PORT_MIN(unit, sfi);
	      if (IS_SFI_PORT(unit, ports + min_sfi)) {
		  link = SOC_PORT_BLOCK_INDEX(unit, ports + min_sfi);
		  regval0 = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, link, SF0_SI_ERROR);
		  bip = SAND_HAL_GET_FIELD(KA, SF0_SI_ERROR, BIP_ERR_CNT, regval0);
		  rx_err = SAND_HAL_GET_FIELD(KA, SF0_SI_ERROR, RCVD_8B10B_ERR_CNT, regval0);
		  lost_time = SAND_HAL_GET_FIELD(KA, SF0_SI_ERROR, GAINED_TIME_ALIGNMENT_EVENT, regval0);
	      } else if (IS_SCI_PORT(unit, ports + min_sfi)) {
		  link = SOC_PORT_BLOCK_INDEX(unit, ports + min_sfi);
		  if (link == 0) {
		      regval0 = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_ERROR);
		      bip = SAND_HAL_GET_FIELD(KA, SC_SI0_ERROR, BIP_ERR_CNT, regval0);
		      rx_err = SAND_HAL_GET_FIELD(KA, SC_SI0_ERROR, RCVD_8B10B_ERR_CNT, regval0);
		      lost_time = SAND_HAL_GET_FIELD(KA, SC_SI0_ERROR, GAINED_TIME_ALIGNMENT_EVENT, regval0);
		  } else if (link == 1) {
		      regval0 = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_ERROR);
		      bip = SAND_HAL_GET_FIELD(KA, SC_SI1_ERROR, BIP_ERR_CNT, regval0);
		      rx_err = SAND_HAL_GET_FIELD(KA, SC_SI1_ERROR, RCVD_8B10B_ERR_CNT, regval0);
		      lost_time = SAND_HAL_GET_FIELD(KA, SC_SI1_ERROR, GAINED_TIME_ALIGNMENT_EVENT, regval0);
		  }
	      }
	      for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
		  bp = (uint32 *) &sbx->custom_stats[base * links];
		  switch(base) {
		      case 0:
			  bp[ports] += rx_err;
			  break;
		      case 1:
			  bp[ports] += lost_time;
			  break;
		      case 2:
			  bp[ports] += 0; /* unused on QE2000 */
			  break;
		      case 3:
			  bp[ports] += bip;
			  break;
		      case 4:
			  bp[ports] += 0; /* unused on QE2000 */
			  break;
		  }
	      }
	      if (IS_SFI_PORT(unit, ports + min_sfi)) {
		  link = SOC_PORT_BLOCK_INDEX(unit, ports + min_sfi);
		  if (bip)
		      regval1 = SAND_HAL_MOD_FIELD(KA, SF0_SI_ERROR, BIP_ERR_CNT, regval1, -1);
		  if (rx_err)
		      regval1 = SAND_HAL_MOD_FIELD(KA, SF0_SI_ERROR, RCVD_8B10B_ERR_CNT, regval1, 1);
		  if (lost_time)
		      regval1 = SAND_HAL_MOD_FIELD(KA, SF0_SI_ERROR, GAINED_TIME_ALIGNMENT_EVENT, regval1, 1);
		  if (bip + rx_err + lost_time) 
		      SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, link, SF0_SI_ERROR, regval1);
	      } else if (IS_SCI_PORT(unit, ports + min_sfi)) {
		  link = SOC_PORT_BLOCK_INDEX(unit, ports + min_sfi);
		  if (link == 0) {
		      if (bip)
			  regval1 = SAND_HAL_MOD_FIELD(KA, SC_SI0_ERROR, BIP_ERR_CNT, regval1, -1);
		      if (rx_err)
			  regval1 = SAND_HAL_MOD_FIELD(KA, SC_SI0_ERROR, RCVD_8B10B_ERR_CNT, regval1, -1);
		      if (lost_time)
			  regval1 = SAND_HAL_MOD_FIELD(KA, SC_SI0_ERROR, GAINED_TIME_ALIGNMENT_EVENT, regval1, -1);
		      if (bip + rx_err + lost_time) 
			  SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_ERROR, regval1);
		  } else if (link == 1) {
		      if (bip)
			  regval1 = SAND_HAL_MOD_FIELD(KA, SC_SI1_ERROR, BIP_ERR_CNT, regval1, -1);
		      if (rx_err)
			  regval1 = SAND_HAL_MOD_FIELD(KA, SC_SI1_ERROR, RCVD_8B10B_ERR_CNT, regval1, -1);
		      if (lost_time)
			  regval1 = SAND_HAL_MOD_FIELD(KA, SC_SI1_ERROR, GAINED_TIME_ALIGNMENT_EVENT, regval1, -1);
		      if (bip + rx_err + lost_time) 
			  SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_ERROR, regval1);
		  }
	      }

	  }
#endif
      } 
      
      return rv;
}


/*
 * Function:
 *      _soc_sbx_counter_collect
 * Purpose:
 *      This routine gets called each time the counter transfer has
 *      completed a cycle.
 * Parameters:
 *      unit    - Device unit number
 *      discard - If true, the software counters are not updated; this
 *                results in only synchronizing the previous hardware
 *                count buffer.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      It computes the deltas in the hardware counters from the last
 *      time it was called and adds them to the high resolution (64-bit)
 *      software counters in 'val'.  It takes wrapping into
 *      account for counters that are less than 64 bits wide.
 *      It also computes the 'delta'.
 */
STATIC int
_soc_sbx_counter_collect(int unit, int discard)
{
    int  block = 0;
    int  set = 0;
    int rv = SOC_E_NONE;

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                (BSL_META_U(unit,
                            "soc_counter_collect: unit=%d discard=%d\n"),
                 unit, discard));


    for (block = 0; block < COUNTER_NUM_BLOCKS(unit); block++) {

        for (set = 0; set < COUNTER_BLOCK_NUM_SETS(unit, block); set++) {

            /* Skip on 'null' counter sets */
            if (COUNTER_BSET(unit, block, set) == NULL) {
                continue;
            }

            /* Collect all counters for this set */
            _soc_sbx_counter_bset_collect_autozero(unit, discard, block, set);
        }
    }

#ifdef BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)) {
      /*
       * Collect Custom statistics
       */

      if ((rv = soc_sbx_process_custom_stats(unit,(SB_FAB_DEVICE_QE2000_SFI_LINKS+SB_FAB_DEVICE_QE2000_SCI_LINKS))) != SOC_E_NONE) {
	LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                    (BSL_META_U(unit,
                                "soc_counter_collect: unit=%d discard=%d, CUSTOM STATS ERROR (%d)\n"),
                     unit, discard, rv));
      }
    }
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
      soc_sbx_sirius_config_t *sir = SOC_SBX_CFG_SIRIUS(unit);

      /*
       * For collection of Global debug statistics
       * Only collect when enabled by user.
       */
      
      if (sir->cs.flags & CS_GBL_ENABLE) {
	  if ((rv = soc_sbx_sirius_process_global_stats(unit)) != SOC_E_NONE) {
	      if (rv == SOC_E_FULL)
		  LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                              (BSL_META_U(unit,
                                          "soc_counter_collect: unit=%d discard=%d, GLOBAL STATS OVERFLOW\n"),
                               unit, discard));
	      else
		  LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                              (BSL_META_U(unit,
                                          "soc_counter_collect: unit=%d discard=%d, GLOBAL STATS ERROR (%d)\n"),
                               unit, discard, rv));
	  }
      }

      /*
       * Collect SLQ statistics per queue within range
       * of 16 queues from base queue only if any queue
       * is enabled.
       */

      if (sir->cs.flags & CS_SLQ_ENABLE) {
	  if ((rv = soc_sbx_sirius_process_slq_stats(unit)) != SOC_E_NONE) {
	      if (rv == SOC_E_FULL)
		  LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                              (BSL_META_U(unit,
                                          "soc_counter_collect: unit=%d discard=%d, SLQ STATS OVERFLOW\n"),
                               unit, discard));
	      else
		  LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                              (BSL_META_U(unit,
                                          "soc_counter_collect: unit=%d discard=%d, SLQ STATS ERROR (%d)\n"),
                               unit, discard, rv));
	  }
      }

      /*
       * Collect FDM Admission Control Statistics
       */

      if ((rv = soc_sbx_sirius_process_fd_drop_stats(unit, 0)) != SOC_E_NONE) {
	  if (rv == SOC_E_FULL)
	      LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                          (BSL_META_U(unit,
                                      "soc_counter_collect: unit=%d discard=%d, ADMISSION CONTROL DROP STATS OVERFLOW\n"),
                           unit, discard));
	  else
	      LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                          (BSL_META_U(unit,
                                      "soc_counter_collect: unit=%d discard=%d, ADMISSION CONTROL DROP STATS ERROR (%d)\n"),
                           unit, discard, rv));
      }

      /*
       * Collect Custom statistics
       */

      if ((rv = soc_sbx_process_custom_stats(unit, (SB_FAB_DEVICE_SIRIUS_SFI_LINKS+SB_FAB_DEVICE_SIRIUS_SCI_LINKS))) != SOC_E_NONE) {
	LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                    (BSL_META_U(unit,
                                "soc_counter_collect: unit=%d discard=%d, CUSTOM STATS ERROR (%d)\n"),
                     unit, discard, rv));
      }

      rv = SOC_E_NONE;
    }
#endif /* BCM_SIRIUS_SUPPORT */

    return rv;
}


/*
 * Function:
 *     _soc_sbx_counter_thread
 * Purpose:
 *     Master counter collection and accumulation thread.
 * Parameters:
 *     unit_vp - Device unit number
 * Returns:
 *     Nothing, does not return.
 * Notes:
 *     Assumes valid unit and counter initialized for that unit.
 */
STATIC void
_soc_sbx_counter_thread(void *unit_vp)
{
    int                   unit = PTR_TO_INT(unit_vp);
    soc_control_t        *soc = SOC_CONTROL(unit);
    int                   rv = 0;
    int                   interval = 0;
    uint32                err_count = 0;
    sal_usecs_t           cdma_timeout = 0, now = 0;
    COUNTER_ATOMIC_DEF    s = 0;
    int                   sync_gnt = FALSE;


    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_sbx_counter_thread: unit=%d\n"), unit));


    /*
     * Create a semaphore used to time the trigger scans, and if DMA is
     * used, monitor for the Stats DMA Iteration Done interrupt.
     */

    if (SAL_BOOT_QUICKTURN) {
        cdma_timeout = CDMA_TIMEOUT_QT;
    } else if (SAL_BOOT_BCMSIM) {
        cdma_timeout = CDMA_TIMEOUT_BCMSIM;
    } else {
        cdma_timeout = CDMA_TIMEOUT;
    }
    cdma_timeout = soc_property_get(unit, spn_CDMA_TIMEOUT_USEC, cdma_timeout);
    

    /*
     * The hardware timer can only be used for intervals up to about
     * 1.048 seconds.  This implementation uses a software timer (via
     * semaphore timeout) instead of the hardware timer.
     */

    while ((interval = soc->counter_interval) != 0) {
#ifdef COUNTER_BENCH
        sal_usecs_t     start_time = 0;
#endif
        int             err = 0;

        /*
         * Use a semaphore timeout instead of a sleep to achieve the
         * desired delay between scans.  This allows this thread to exit
         * immediately when soc_sbx_counter_stop wants it to.
         */

        LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                    (BSL_META_U(unit,
                                "soc_sbx_counter_thread: sleep %d\n"), interval));

#if defined (BCM_CALADAN3_SUPPORT)
#ifndef BCM_CALADAN3_SIM
	if (SOC_SBX_CFG_CALADAN3(unit)->c3_64bit_pc){
	    sbx_c3_gather_hw_counters(unit);
	}
#endif
#endif
        (void)sal_sem_take(soc->counter_trigger, interval);

        if (soc->counter_interval == 0) {       /* Exit signaled */
            break;
        }

        if (soc->counter_sync_req) {
            sync_gnt = TRUE;
        }

#ifdef COUNTER_BENCH
        start_time = sal_time_usecs();
#endif

        /*
         * Add up changes to counter values.
         */
        now = sal_time_usecs();
        COUNTER_ATOMIC_BEGIN(s);
        soc->counter_coll_prev = soc->counter_coll_cur;
        soc->counter_coll_cur  = now;
        COUNTER_ATOMIC_END(s);

        if (!err) {
            COUNTER_LOCK(unit);

            rv = _soc_sbx_counter_collect(unit, FALSE);
            COUNTER_UNLOCK(unit);

            if (rv < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_counter_thread: non DMA counter collect failed: %s\n"),
                           soc_errmsg(rv)));
                err = 1;
            }
        }

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
	if (SOC_IS_SBX_SIRIUS(unit) || 
	    (SOC_IS_SBX_CALADAN3(unit) && !SAL_BOOT_BCMSIM)) {

            COUNTER_LOCK(unit);
	    /* 
	     * Collect 64 bit XPORT Bigmac registers
	     * Remove SAL_BOOT_BCMSIM when model supports 64 bit BIGMAC regs
	     */
            if (!(soc->counter_flags & SOC_COUNTER_F_DMA)) {
	        if ((soc->counter_n64 > 0)) {
                    rv = soc_sbx_counter_collect64(unit, FALSE);
                }
            }

            if (rv < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_counter_thread: DMA counter collect failed: %s\n"),
                                      soc_errmsg(rv)));
            }

#if defined(BCM_CALADAN3_SUPPORT)
            if (SOC_IS_SBX_CALADAN3(unit) && soc_feature(unit, soc_feature_controlled_counters)) {
                rv = soc_sbx_controlled_counters_collect64(unit, FALSE);
            }
#endif
            COUNTER_UNLOCK(unit);

            if (rv < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_counter_thread: Controlled counter collect failed: %s\n"),
                           soc_errmsg(rv)));
            }
        }
#endif /* BCM_SIRIUS_SUPPORT */


        /*
         * Forgive spurious errors
         */
        if (err) {
            if (++err_count == soc_property_get(unit,
                                                spn_SOC_CTR_MAXERR, 5)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_counter_thread: Too many errors\n")));
                rv = SOC_E_INTERNAL;
                goto done;
            }
        } else if (err_count > 0) {
            err_count--;
        }

#ifdef COUNTER_BENCH
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Iteration time: %d usec\n"),
                     SAL_USECS_SUB(sal_time_usecs(), start_time)));
#endif

        if (sync_gnt) {
            soc->counter_sync_req = 0;
            sync_gnt = 0;
        }
    }

    rv = SOC_E_NONE;

 done:
    if (rv < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_counter_thread: Operation failed; exiting\n")));
    }

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_sbx_counter_thread: exiting\n")));

    soc->counter_pid = SAL_THREAD_ERROR;
    soc->counter_interval  = 0;

    sal_thread_exit(0);
}

/*
 * Function:
 *     soc_sbx_counter_start
 * Purpose:
 *     Start the counter collection, software accumulation process.
 * Parameters:
 *     unit     - Device number
 *     flags    - SOC_COUNTER_F_xxx flags
 *     interval - Collection period in micro-seconds,
 *                using 0 is the same as calling soc_sbx_counter_stop()
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_counter_start(int unit, uint32 flags, int interval, pbmp_t pbmp)
{
    soc_control_t       *soc = NULL;
    char                 pfmt[SOC_PBMP_FMT_LEN];
    sal_sem_t            sem = NULL;
    int                  rv = SOC_E_NONE;
#if defined(BCM_CALADAN3_SUPPORT)
    int                  is_channelized = FALSE;
    int                  requires_phy_setup = FALSE;
    soc_port_t           port;
#endif /* BCM_CALADAN3_SUPPORT */
    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_sbx_counter_start: unit=%d flags=0x%x "
                         "interval=%d pbmp=%s\n"),
              unit, flags, interval, SOC_PBMP_FMT(pbmp, pfmt)));



    UNIT_INIT_CHECK(unit);

    soc = SOC_CONTROL(unit);


    /* Stop if already running */
    rv = soc_sbx_counter_stop(unit);
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    COUNTER_LOCK(unit);

    /* Interval of 0 just stops thread */
    if (interval == 0) {
        COUNTER_UNLOCK(unit);
        return SOC_E_NONE;
    }

    /* Create fresh semaphores */

    if ((sem = soc->counter_trigger) != NULL) {
        soc->counter_trigger = NULL;    /* Stop others from waking sem */
        sal_sem_destroy(sem);           /* Then destroy it */
    }

    soc->counter_trigger =
        sal_sem_create("counter_trigger", sal_sem_BINARY, 0);

    if ((sem = soc->counter_intr) != NULL) {
        soc->counter_intr = NULL;       /* Stop intr from waking sem */
        sal_sem_destroy(sem);           /* Then destroy it */
    }

    soc->counter_intr =
        sal_sem_create("counter_intr", sal_sem_BINARY, 0);

    if ((soc->counter_trigger == NULL) || (soc->counter_intr == NULL)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_counter_start: sem create failed\n")));
        COUNTER_UNLOCK(unit);
        return SOC_E_INTERNAL;
    }

    SOC_PBMP_ASSIGN(soc->counter_pbmp, pbmp);
    if (SOC_IS_SBX_SIRIUS(unit)) {
        if (NUM_HG_PORT(unit) != 0) {
            SOC_PBMP_AND(soc->counter_pbmp, SOC_PORT_BITMAP(unit,hg));
        }
        if (NUM_XE_PORT(unit) != 0) {
            SOC_PBMP_AND(soc->counter_pbmp, SOC_PORT_BITMAP(unit,xe));
        }
    }

#if defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_CALADAN3(unit)) {
        /* remove any ports which are channelized and not the base port */
        for (port = 0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {
            is_channelized = FALSE;
            SOC_IF_ERROR_RETURN(soc_sbx_caladan3_port_is_channelized_subport(unit, port, 
                                                                             &is_channelized,
                                                                             &requires_phy_setup));
            if (is_channelized == TRUE) {
                SOC_PBMP_PORT_REMOVE(soc->counter_pbmp, port);
            }
        }
    }
#endif /* BCM_CALADAN3_SUPPORT */

    /* Flags */
    soc->counter_flags = flags;

    /* HW takes care of this */
    soc->counter_flags &= ~SOC_COUNTER_F_SWAP64;

    if (!soc_feature(unit, soc_feature_stat_dma))
        soc->counter_flags &= ~SOC_COUNTER_F_DMA;

    soc->counter_flags &= ~SOC_COUNTER_F_HOLD;

#if defined (BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || 
	(SOC_IS_SBX_CALADAN3(unit) && !SAL_BOOT_BCMSIM)) {    
	rv = soc_counter_autoz(unit, 0);
	if (SOC_FAILURE(rv)) {
	    COUNTER_UNLOCK(unit);
	    return rv;
	}
    }
#endif /* BCM_SIRIUS_SUPPORT  || BCM_CALADAN3_SUPPORT */

    /* Synchronize counter 'prev' values with current hardware counters */
    soc->counter_coll_prev = soc->counter_coll_cur = sal_time_usecs();

    /*
     * Non DMA counters
     */
    rv = _soc_sbx_counter_collect(unit, TRUE);
    if (SOC_FAILURE(rv)) {
        COUNTER_UNLOCK(unit);
        return rv;
    }

    /*
     * DMA counters
     */
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) ||
	(SOC_IS_SBX_CALADAN3(unit) && !SAL_BOOT_BCMSIM)) {


        if ((soc->counter_n64 > 0)) {
            rv = soc_sbx_counter_collect64(unit, TRUE);
            if (SOC_FAILURE(rv)) {
                COUNTER_UNLOCK(unit);
                return rv;
            }
        }
    }
#endif /* BCM_SIRIUS_SUPPORT  */


    
    /* Start counter collector thread */
    if (interval != 0) {
        soc->counter_interval = interval;

        sal_snprintf(soc->counter_name, sizeof(soc->counter_name),
                     "bcmCNTR.%d", unit);

        soc->counter_pid = sal_thread_create(soc->counter_name,
					     SAL_THREAD_STKSZ,
					     soc_property_get(unit,
							      spn_COUNTER_THREAD_PRI, 
							      50),
					     _soc_sbx_counter_thread,
					     INT_TO_PTR(unit));

        if (soc->counter_pid == SAL_THREAD_ERROR) {
            soc->counter_interval = 0;
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_counter_start: thread create failed\n")));
            rv = SOC_E_INTERNAL;
        } else {
            LOG_INFO(BSL_LS_SOC_COUNTER,
                     (BSL_META_U(unit,
                                 "soc_sbx_counter_start: complete\n")));
        }
        if (soc->counter_flags & SOC_COUNTER_F_DMA) {
#if 0
#ifdef BCM_SBUSDMA_SUPPORT
            
            if (soc_feature(unit, soc_feature_sbusdma)) {
                int rv = _soc_counter_sbusdma_setup(unit);
                if (rv) {
                    (void)_soc_counter_sbudma_desc_free_all(unit);
                }
            }
#endif
#endif
        }
    }
    
    COUNTER_UNLOCK(unit);
    
    return rv;
}


/*
 * Function:
 *      soc_sbx_counter_status
 * Purpose:
 *      Get the status of counter collection, S/W accumulation process.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      flags - SOC_COUNTER_F_xxx flags.
 *      interval - collection period in micro-seconds.
 *      pbmp - bit map of ports to collact counters on.
 * Returns:
 *      SOC_E_XXX
 */

int
soc_sbx_counter_status(int unit, uint32 *flags, int *interval, pbmp_t *pbmp)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
 
    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_sbx_counter_status: unit=%d\n"), unit));

    *interval = soc->counter_interval;
    *flags = soc->counter_flags;
    SOC_PBMP_ASSIGN(*pbmp, soc->counter_pbmp);

    return (SOC_E_NONE);
}


/*
 * Function:
 *     soc_sbx_counter_sync
 * Purpose:
 *     Force an immediate counter update.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *     Ensures that ALL counter activity that occurred before the sync
 *     is reflected in the results of any soc_counter_get()-type
 *     routine that is called after the sync.
 */
int
soc_sbx_counter_sync(int unit)
{
    soc_control_t        *soc = NULL;
    soc_timeout_t         to;
    int                   interval = 0;
    uint32                stat_sync_timeout = 0;

    UNIT_INIT_CHECK(unit);

    soc = SOC_CONTROL(unit);

    COUNTER_LOCK(unit);

    if ((interval = soc->counter_interval) == 0) {
        COUNTER_UNLOCK(unit);
        return SOC_E_DISABLED;
    }

    /* Trigger a collection */
    soc->counter_sync_req = TRUE;
    sal_sem_give(soc->counter_trigger);

    COUNTER_UNLOCK(unit);

    if (SAL_BOOT_QUICKTURN) {
        stat_sync_timeout = STAT_SYNC_TIMEOUT_QT;
    } else if (SAL_BOOT_BCMSIM) {
        stat_sync_timeout = STAT_SYNC_TIMEOUT_BCMSIM;
    } else {
        stat_sync_timeout = STAT_SYNC_TIMEOUT;
    }
    stat_sync_timeout = soc_property_get(unit,
                                         spn_BCM_STAT_SYNC_TIMEOUT,
                                         stat_sync_timeout);
    soc_timeout_init(&to, stat_sync_timeout, 0);
    while (soc->counter_sync_req) {
        if (soc_timeout_check(&to)) {
            if (soc->counter_sync_req) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_counter_sync: "
                                      "counter thread not responding\n")));
                soc->counter_sync_req = FALSE;
                return SOC_E_INTERNAL;
            }
        }

        sal_usleep(10000);
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_counter_stop
 * Purpose:
 *     Terminate the counter collection, software accumulation process.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Returns:
 *     SOC_E_XXX
 */
int
soc_sbx_counter_stop(int unit)
{
    soc_control_t     *soc = NULL;
    int                rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_counter_stop: unit=%d\n"), unit));

    UNIT_INIT_CHECK(unit);
   
    soc = SOC_CONTROL(unit);

    /* Stop thread if present */
    if (soc->counter_interval != 0) {
        sal_thread_t   sample_pid;
        sal_usecs_t    timeout;
        soc_timeout_t  to;

        if (SAL_BOOT_QUICKTURN) {
            timeout = CDMA_TIMEOUT_QT;
        } else if (SAL_BOOT_BCMSIM) {
            timeout = CDMA_TIMEOUT_BCMSIM;
        } else {
            timeout = CDMA_TIMEOUT;
        }
        timeout = soc_property_get(unit, spn_CDMA_TIMEOUT_USEC, timeout);
 
        /*
         * Signal by setting interval to 0, and wake up thread to speed
         * its exit.  It may also be waiting for the hardware interrupt
         * semaphore.  Wait a limited amount of time for it to exit.
         */

        soc->counter_interval = 0;

        sal_sem_give(soc->counter_intr);
        sal_sem_give(soc->counter_trigger);

        soc_timeout_init(&to, timeout, 0);

        while ((sample_pid = soc->counter_pid) != SAL_THREAD_ERROR) {
            if (soc_timeout_check(&to)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_counter_stop: thread did not exit  timeout:%d\n"), timeout));
                soc->counter_pid = SAL_THREAD_ERROR;
                rv = SOC_E_INTERNAL;
                break;
            }

            sal_usleep(10000);
        }
    } 
    
    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_sbx_counter_stop: unit=%d rv=%d\n"),
              unit, rv));
    return rv;
}


/*
 * Function:
 *     _soc_sbx_counter_get
 * Purpose:
 *     Get the software-accumulated counter value.
 *     The software-accumulated counter value is zeroed if requested.
 * Parameters:
 *     unit    - Device number
 *     block   - Counter block
 *     set     - Counter set in given block
 *     counter - Counter in given set to get value for
 *     zero    - If TRUE, current counter is zeroed after reading
 *     val     - (OUT) 64-bit counter value
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_counter_get(int unit, int block, int set, int counter,
                     int zero, uint64 *val)
{
    volatile uint64     *vptr = NULL;
    uint64              value = COMPILER_64_INIT(0,0);
    COUNTER_ATOMIC_DEF  s = 0;

    UNIT_INIT_CHECK(unit);

    if ((block < 0) || (block >= COUNTER_NUM_BLOCKS(unit))) {
        return SOC_E_PARAM;
    }

    if (((set < 0) || (set >= COUNTER_BLOCK_NUM_SETS(unit, block))) ||
        ((counter < 0) ||(counter >= COUNTER_BSET_NUM_COUNTERS(unit, block)))) {
        return SOC_E_PARAM;
    }

    /* Check for empty counter sets */
    if (COUNTER_BSET(unit, block, set) == NULL) {
        return SOC_E_PARAM;
    }

    /* Try to minimize the atomic section as much as possible */
    vptr = &(COUNTER_DATA(unit, block, set, counter).val);

    if (zero) {
        COUNTER_ATOMIC_BEGIN(s);
        value = *vptr;
        COMPILER_64_ZERO(*vptr);
        COUNTER_ATOMIC_END(s);
    } else {
        COUNTER_ATOMIC_BEGIN(s);
        value = *vptr;
        COUNTER_ATOMIC_END(s);
    }

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                (BSL_META_U(unit,
                            "Counter get unit=%d block=%d set=%d counter=%d"
                            "val=0x%08x_%08x\n"),
                 unit, block, set, counter,
                 COMPILER_64_HI(value), COMPILER_64_LO(value)));

    *val = value;

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_counter_get
 *     soc_sbx_counter_get_zero
 * Purpose:
 *     Get the software-accumulated counter value.
 *     The software-accumulated counter value is zeroed for the _zero()
 *     version.
 * Parameters:
 *     unit    - Device number
 *     block   - Counter block
 *     set     - Counter set in given block
 *     counter - Counter in given set to get value for
 *     val     - (OUT) 64-bit counter value
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_counter_get(int unit, int block, int set, int counter, uint64 *val)
{
    return _soc_sbx_counter_get(unit, block, set, counter, FALSE, val);
}


int
soc_sbx_counter_get_zero(int unit, int block, int set, int counter, uint64 *val)
{
    return _soc_sbx_counter_get(unit, block, set, counter, TRUE, val);
}


/*
 * Function:
 *     soc_sbx_counter_set
 * Purpose:
 *     Set both, the hardware and the software counter, to the
 *     given value.
 * Parameters:
 *     unit    - Device number
 *     block   - Counter block
 *     set     - Counter set in given block
 *     counter - Counter to set
 *     val     - Value to set
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_counter_set(int unit, int block, int set, int counter, uint64 val)
{
    int                 rv = SOC_E_NONE;
    _counter_data_t     *data = NULL;

    COUNTER_ATOMIC_DEF  s = 0;

    UNIT_INIT_CHECK(unit);

    COUNTER_LOCK(unit);

    if ((block < 0) || (block >= COUNTER_NUM_BLOCKS(unit))) {
        COUNTER_UNLOCK(unit);
        return SOC_E_PARAM;
    }

    if (((set < 0) || (set >= COUNTER_BLOCK_NUM_SETS(unit, block))) ||
        ((counter < 0) ||(counter >= COUNTER_BSET_NUM_COUNTERS(unit, block)))) {
        COUNTER_UNLOCK(unit);
        return SOC_E_PARAM;
    }

    /* Check for empty counter sets */
    if (COUNTER_BSET(unit, block, set) == NULL) {
        COUNTER_UNLOCK(unit);
        return SOC_E_PARAM;
    }
        
    /* Set value in hardware counter */    
    if (SOC_FAILURE(rv = COUNTER_WRITE(unit, block, set, counter, val))) {
        COUNTER_UNLOCK(unit);
        return rv;
    }

    /* Set value in software-accumulated counter */
    data = &(COUNTER_DATA(unit, block, set, counter));

    /* The following section updates 64-bit values and must be atomic */
    COUNTER_ATOMIC_BEGIN(s);
    data->val  = val;
    data->prev = val;
    COMPILER_64_ZERO(data->delta);
    COUNTER_ATOMIC_END(s);

    COUNTER_UNLOCK(unit);

    return SOC_E_NONE;
}


int
soc_sbx_controlled_counter_clear(int unit, soc_port_t port)
{
#ifdef BCM_CALADAN3_SUPPORT
    soc_control_t       *soc = SOC_CONTROL(unit);
    uint64              ctr_new[SOC_SBX_CALADAN3_IL_NUM_CHANNEL_STATS];
    int                 num_channels, chan_index;
    int                 index, port_base;
    soc_controlled_counter_t* ctr;
    
    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
            (BSL_META_U(unit,
                        "soc_controlled_counter_clear: unit=%d port=%d\n"),
             unit, port));

    if ((!soc_feature(unit, soc_feature_controlled_counters)) ||
         (!soc->controlled_counters)) {
        return SOC_E_NONE;
    }

    port_base = COUNTER_IDX_PORTBASE(unit, port);

    COUNTER_LOCK(unit);
    
    if (IS_IL_PORT(unit, port)) {
        num_channels = SOC_SBX_CALADAN3_IL_NUM_CHANNEL_STATS;
    } else {
        num_channels = 1;
    }

    for (index = 0; soc->controlled_counters[index].controlled_counter_f != NULL; index++) {
        COUNTER_ATOMIC_DEF s;

        /* assume clear on read */
        ctr = &soc->controlled_counters[index];
        ctr->controlled_counter_f(unit, ctr->counter_id, port, &ctr_new[0]);

        /*check if counter is relevant*/
        if(!COUNTER_IS_COLLECTED(soc->controlled_counters[index])) {
            continue;
        }

        /* The following section updates 64-bit values and must be atomic */
        COUNTER_ATOMIC_BEGIN(s);
        for (chan_index = 0; chan_index < num_channels; chan_index++) {
            COMPILER_64_SET(soc->counter_sw_val[port_base + ctr->counter_idx * num_channels + chan_index], 0, 0);
        }
        COMPILER_64_SET(soc->counter_hw_val[port_base + ctr->counter_idx * num_channels], 0, 0);
        COMPILER_64_SET(soc->counter_delta[port_base + ctr->counter_idx * num_channels], 0, 0);
        COUNTER_ATOMIC_END(s);
    }
    
    COUNTER_UNLOCK(unit);
#endif

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_counter_dump
 * Purpose:
 *     Display software accumulated counter values on given unit.
 * Parameters:
 *     unit    - Device number
 * Returns:
 *     SOC_E_NONE - Success
 */
int
soc_sbx_counter_dump(int unit)
{
    int              block = 0;
    int              set = 0;
    int              counter = 0;
    _counter_data_t  *data = NULL;


    for (block = 0; block < COUNTER_NUM_BLOCKS(unit); block++) {

        LOG_CLI((BSL_META_U(unit,
                            "Block %d:\n"), block));

        for (set = 0; set < COUNTER_BLOCK_NUM_SETS(unit, block); set++) {

            /* Skip 'null' counter sets */
            if (COUNTER_BSET(unit, block, set) == NULL) {
                continue;
            }

            LOG_CLI((BSL_META_U(unit,
                                "  Set %d\n"), set));
            for (counter = 0; counter < COUNTER_BSET_NUM_COUNTERS(unit, block);
                 counter++) {
                data = &(COUNTER_DATA(unit, block, set, counter));

                if ( COMPILER_64_IS_ZERO(data->val)) {
                    continue;
                }

                LOG_CLI((BSL_META_U(unit,
                                    "        Counter %d - 0x%08x%08x \n"),
                         counter, COMPILER_64_HI(data->val),
                         COMPILER_64_LO(data->val)));
                
            }

        }
    }

    return SOC_E_NONE;
}


int soc_sbx_counter_bset_clear(int unit, int block)
    
{

      return _soc_sbx_counter_bset_clear( unit, block);

}
