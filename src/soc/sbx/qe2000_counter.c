/* 
 * $Id: qe2000_counter.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        qe2000_counter.c
 * Purpose:     Software Counter Collection module for QE2000.
 *
 */

#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/counter.h>
#include <soc/debug.h>
#include <soc/sbx/qe2000_counter.h>
#include <soc/sbx/hal_ca_auto.h>


static int soc_qe2000_counter_enable_flag;
static int soc_qe2000_counter_current_base;

/*
 * Counter Blocks Types
 */
typedef enum soc_qe2000_counter_block_e {
    qe2000CounterBlockQm = 0,
    qe2000CounterBlockCount
} soc_qe2000_counter_block_t;

/*
 * QE2000 has 16 counter blocks at a time.
 * The base can be moved as desired.
 *
 */
#define QE2000_COUNTER_BLOCK_QM_NUM_SETS    16
#define QE2000_COUNTER_REG_QM_WIDTH         32


STATIC int _soc_qe2000_counter_qm_read(int unit, int set, int counter,
                                       uint64 *val, int *width);
STATIC int _soc_qe2000_counter_qm_write(int unit, int set, int counter,
                                        uint64 val);

/*
 * Counter Blocks
 *
 * Counter blocks for the Software Counter module to collect statistics on.
 *
 * NOTE:  The order of the counter blocks must be the same
 *        as the blocks defined in 'fe2000_counter_block_t'
 */
soc_sbx_counter_block_info_t    qe2000_counter_blocks[] = {
    { qe2000CounterBlockQm,
      QE2000_COUNTER_BLOCK_QM_NUM_SETS,
      QE2000_COUNTER_QM_COUNT,
      _soc_qe2000_counter_qm_read,
      _soc_qe2000_counter_qm_write,
    },
};


/*
 * Function:
 *     _soc_qe2000_counter_qm_read
 * Purpose:
 *     Read specified am counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in AM block
 *     counter - Counter in given set to read
 *     val     - (OUT) Counter value
 *     width   - (OUT) Bits in hardware register counter
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_qe2000_counter_qm_read(int unit, int set, int counter,
                            uint64 *val, int *width)
{
    int     rv;
    uint32  data[QE2000_COUNTER_QM_COUNT];
    int counter_index;

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                (BSL_META_U(unit,
                            "_soc_qe2000_counter_qm_read set %d cntr %d enb %d\n"),
                 set, counter, soc_qe2000_counter_enable_flag));

    rv = SOC_E_NONE;

    if (soc_qe2000_counter_enable_flag != TRUE) {
        for ( counter_index = 0 ; counter_index < QE2000_COUNTER_QM_COUNT; counter_index++) {
          COMPILER_64_ZERO(*val);
         val++;
        }
        return SOC_E_NONE;
    }

    rv = soc_qe2000_qm_counter_read(unit, set, &data[0]);

    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "qe2000 counter read failure\n")));
        return rv;
    }

    /*
     * Skip 'get' field from data value, since it's not needed here
     */
    for ( counter_index = 0 ; counter_index < QE2000_COUNTER_QM_COUNT; counter_index++) {
        if ( data[counter_index]) {
            LOG_INFO(BSL_LS_SOC_COUNTER,
                     (BSL_META_U(unit,
                                 " Count %d:%d: %08x, "), set, counter_index, data[counter_index]));
        } else {
            LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                        (BSL_META_U(unit,
                                    " Count %d:%d: %08x, "), set, counter_index, data[counter_index]));
        }

         COMPILER_64_SET((*val), 0, data[counter_index]);
         val++;

    }
    *width = QE2000_COUNTER_REG_QM_WIDTH;

    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_qe2000_counter_qm_write
 * Purpose:
 *     Write specified AM counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in AM block
 *     counter - Counter in given set
 *     val     - Counter value to write
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_qe2000_counter_qm_write(int unit, int set, int counter, uint64 val)
{
    int     rv = SOC_E_NONE;

    return rv;
}


/*
 * Function:
 *     soc_sbx_qe2000_counter_init
 * Purpose:
 *     Initialize and start the counter collection, software
 *     accumulation process.
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
soc_sbx_qe2000_counter_init(int unit, uint32 flags, int interval, pbmp_t pbmp)
{
    int         block;
    int         set;
    int         rv = SOC_E_NONE;
    
    block = 0;  /* At present only one block possible */

    /* Init Software Counter Collection module */
    SOC_IF_ERROR_RETURN(soc_sbx_counter_init(unit, &qe2000_counter_blocks[0],
                                             qe2000CounterBlockCount));

    /* Add counter sets to main counter collector */

    for (set = 0; set < QE2000_COUNTER_BLOCK_QM_NUM_SETS; set++) {
   
        soc_sbx_counter_bset_add(unit, block, set);

    }

    soc_qe2000_counter_enable_flag = FALSE;
    soc_qe2000_counter_current_base = -1;

    /* Start software counter collector */
    SOC_IF_ERROR_RETURN(soc_sbx_counter_start(unit, flags, interval, pbmp));

    return rv;
}


/*
 * Function:
 *     soc_sbx_qe2000_counter_port_get
 * Purpose:
 *     Get the specified software-accumulated counter value for given port.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     counter - Counter
 *     val     - (OUT) Software accumulated value
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_qe2000_counter_port_get(int unit, int base_queue, 
                                int block, int set,
                                int counter, uint64 *val)
{
    
    /* Should validate base counter is in agreement */

    return soc_sbx_counter_get(unit, block, set, counter, val);
}


/*
 * Function:
 *     soc_sbx_qe2000_counter_port_set
 * Purpose:
 *     Set the counter value for given port to the requested value.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     counter - Counter to set (a negative value means all counters for port)
 *     val     - Counter value
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_qe2000_counter_port_set(int unit, int base_queue,
                                int block, int set,
                                int counter, uint64 val)
{
    int rv = SOC_E_NONE;
    int  num_counters;
    uint64 uuZero = COMPILER_64_INIT(0,0);
    
    /* Should add check that base_queue address is in agreement with set value */

    /* Set value to all counters on given port */
    num_counters = qe2000_counter_blocks[block].num_counters;

    if( counter >= num_counters ) return SOC_E_PARAM;

    rv = soc_sbx_counter_set(unit, block, set, counter, uuZero);

    return rv;
}

/*
 * Get the state of the enable and base
 */

int
soc_qe2000_counter_enable_get(int unit, int *base, int *enable)
{
    *enable = soc_qe2000_counter_enable_flag;
    return BCM_E_UNAVAIL;
}

int
soc_qe2000_counter_enable_set(int unit, int base)
{
    if(soc_qe2000_counter_enable_flag) return BCM_E_INTERNAL;

    soc_sbx_counter_bset_clear( unit, 0);

    soc_qe2000_counter_enable_flag = TRUE;
    soc_qe2000_counter_current_base = base ;
    return BCM_E_NONE;
}

/*
 * Clear the current counter. We do not currently check for correct base 
 */
int
soc_qe2000_counter_enable_clear(int unit)
{
    soc_qe2000_counter_enable_flag = FALSE;
    soc_qe2000_counter_current_base = -1;
    
    return BCM_E_NONE;
}


