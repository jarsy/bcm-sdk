/* 
 * $Id: qe2000_scoreboard.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        qe2000_scoreboard.c
 * Purpose:     ZScoreboard and Free Buffer Management module.
 *
 */

#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_scoreboard.h>
#include <soc/sbx/hal_ka_auto.h>
#include <bcm_int/sbx/lock.h>


#define SCOREBOARD_DEBUG_2
#define INTERVAL_TOLERANCE 100

/*
 * Mutex lock
 */
#ifdef SCOREBOARD_LOCK
#undef SCOREBOARD_LOCK
#endif
#ifdef SCOREBOARD_UNLOCK
#undef SCOREBOARD_UNLOCK
#endif

static  sal_mutex_t           _mlock[SOC_MAX_NUM_DEVICES];
#define SCOREBOARD_LOCK(unit)    sal_mutex_take(_mlock[unit], sal_mutex_FOREVER)
#define SCOREBOARD_UNLOCK(unit)  sal_mutex_give(_mlock[unit])


/*
 * Atomic operation
 */
#define SCOREBOARD_ATOMIC_DEF         int
#define SCOREBOARD_ATOMIC_BEGIN(s)    ((s) = sal_splhi())
#define SCOREBOARD_ATOMIC_END(s)      (sal_spl(s))

#define FREED_BUF_MODULO  0x10000
#define LOST_BUF_MODULO   0x10000

/*
 * Scoreboard Data
 *
 * Contains software accumulated values on a scoreboard.
 */
typedef struct _scoreboard_data_s {
    uint64  prev;   /* Last read value of scoreboard */
    uint64  val;    /* Software accumulated scoreboard */
    uint64  delta;  /* Delta over last two collections */
} _scoreboard_data_t;

typedef _scoreboard_data_t    *_scoreboard_set_t;  /* Data buffer to a scoreboard set */


/* Debugging, Start */

#define SB_FAB_DEVICE_QE2000_MAX_POLL_SAMPLES           128

typedef struct {
        sal_usecs_t      ScoreboardTime;
        uint32         nBufFreed; 
} Qe2000_PollInfo_t;            

static uint32 Qe2000_Debug = 0x00;
static sal_usecs_t           Debug_StartScoreboardTime;   
static Qe2000_PollInfo_t     Debug_PollInfo[SB_FAB_DEVICE_QE2000_MAX_POLL_SAMPLES];
static uint32              Debug_pollIndex = 0;
static uint32              nTotalLost;
static uint32              nTotalFreed;
static uint32              nTotalDogs;
static uint32              nShortIntervals;


#ifdef NOT_NEEDED

/*
 * Counter Block
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
typedef struct _counter_block_s {
    soc_sbx_counter_block_info_t  *info;  /* Information on counter block */
    _counter_set_t                *sets;  /* Array of counter sets buffer */
} _counter_block_t;
#endif


/*
 * Scoreboard Collector
 *
 * Contains information on the scoreboard collector thread.
 */
typedef struct _scoreboard_collector_s {
    VOL sal_thread_t  thread_id;        /* scoreboard collector task id */
    char              thread_name[16];  /* scoreboard collector task name */
    VOL int           interval;         /* Update interval in usec */
                                        /* (Zero when thread not running) */

    VOL sal_sem_t     trigger;          /* Trigger scoreboard thread attn */
    VOL sal_sem_t     intr;             /* scoreboard H/W interrupt notifier */

    sal_usecs_t       prev;             /* Timestamp of previous collection */
    sal_usecs_t       cur;              /* Timestamp of current collection */

    VOL int           sync_req;         /* soc_scoreboard_sync() support */
} _scoreboard_collector_t;


/*
 * scoreboard Control
 *
 * Contains information on the scoreboard Collection module on a given unit.
 */
typedef struct _scoreboard_control_s {
    VOL uint32            flags;       /* SOC_COUNTER_F_XXX */
    _scoreboard_collector_t  collect;     /* scoreboard collector thread */
    int32    nBuffersSbBuf;
    int32    nBuffers;
    int32    nBufsDelta;
    int32    bIsHalfBus;
    int32    bIs256MbMemPart;
    int32    bIsFaulted;
    int32    nScoreboardPolls;
    int32    nMinPollIntvlInUsecs;
    int32    nCurrentAgeValue;
    uint8     *pScoreboardBuf;
    sal_usecs_t lastScoreboardTime;
} _scoreboard_control_t;

static _scoreboard_control_t    *_scoreboard_control[SOC_MAX_NUM_DEVICES];

#define SCOREBOARD_CONTROL(_unit)             (_scoreboard_control[_unit])

#define SCOREBOARD_COLLECT(_unit)             (SCOREBOARD_CONTROL(_unit)->collect)


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
        if (SCOREBOARD_CONTROL(_unit) == NULL) { return SOC_E_INIT; } \
    } while (0)

STATIC int _sbx_qe2000_scoreboard_check (int unit, int *p_nbr_buffers, int *p_min_poll_intvl_in_usec );

STATIC int _sbx_qe2000_scoreboard_read( int unit, uint32 *num_buffers_to_free) ;

STATIC int _sbx_qe2000_get_buf_info(int  unit, uint32 startBufAddr,
        uint32 endBufAddr, uint32 *pMask, uint32 *pNbrBufs);

STATIC void _sbx_qe2000_check_fb_depth(int unit);

#define BCMSIM_STAT_TIMEOUT       200000000


/*
 * Function:
 *     _sbx_qe2000_check_free_buffer_list
 * Purpose:
 *     check Qe2000 free buffer list
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */

int
_sbx_qe2000_check_free_buffer_list (int unit )

{
    int rv = SOC_E_NONE;
    int RegVal, nBuffers;

    RegVal = SAND_HAL_READ(unit, KA, QM_CONFIG0);
    nBuffers = SAND_HAL_GET_FIELD(KA, QM_CONFIG0, TOTAL_BUFS_AVL, RegVal);

    SCOREBOARD_CONTROL(unit)->nBuffers = nBuffers;
#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "RegVal %d assigning %d buffers\n"),RegVal, nBuffers));
#endif

    return(rv);

}


/*
 * Function:
 *     _sbx_qe2000_poll_intvl
 * Purpose:
 *     check Qe2000 poll interval for minimum delay
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */

STATIC uint32
_sbx_qe2000_poll_intvl(sal_usecs_t *pLastTime, sal_usecs_t *pCurTime)
{
    uint64      nLastUsecs;
    uint64      nCurUsecs;
    uint64      uuTmp;

    /* 
     * This function is for consistency with FLIB2. Time keeping is less critical
     * in the new model due to use of timer threads
     * FLIB2 dealt with Secs and Usecs. SAL combines sec and usecs when sampled
     */

    /* NOTE: currently wrap around is not checked */


    COMPILER_64_SET(nLastUsecs,0, *pLastTime);
    COMPILER_64_SET(nCurUsecs,0, *pCurTime);

    /* consistency checks */
    if COMPILER_64_LT(nCurUsecs, nLastUsecs) {
#ifdef SCOREBOARD_DEBUG
        /* update the last poll time, This will result in only 1 poll being missed */

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("Sb Wraparound Detected\n")));
        
        LOG_CLI((BSL_META("- Prev 0x%x%08x  Cur 0x%x%08x ... Wraparound detected\n"), COMPILER_64_HI(nLastUsecs), COMPILER_64_LO(nLastUsecs), COMPILER_64_HI(nCurUsecs), COMPILER_64_LO(nCurUsecs)));
#endif

        *pLastTime = *pCurTime;

        return(0);
    }
    uuTmp = nCurUsecs;
    COMPILER_64_SUB_64(uuTmp, nLastUsecs);
    return((COMPILER_64_HI(uuTmp) != 0) ? 0xFFFFFFFF : COMPILER_64_LO(uuTmp));
}




STATIC int
_sbx_qe2000_set_churner_state(int unit, sbBool_t bIsEnabled)
{
    int                  status = SB_FAB_STATUS_OK;


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "SetChurnerState (%s)\n"),
                 ((bIsEnabled == TRUE) ? "Enable" : "Disable")));

    if (bIsEnabled == FALSE) {
        SAND_HAL_RMW_FIELD(unit, KA, QM_FB_CONFIG0, FB_BUF_ALLOC_PER_TS, QE2000_BUF_ALLOC_PER_TS_DISABLE);
    }
    else {
        SAND_HAL_RMW_FIELD(unit, KA, QM_FB_CONFIG0, FB_BUF_ALLOC_PER_TS, QE2000_BUF_ALLOC_PER_TS);
    }

    return(status);
}



STATIC int
_sbx_qe2000_qm_fb_write(int unit, uint32 nBufAddress)
{
    int                        status = SB_FAB_STATUS_OK;
    uint32                   uData;
    uint32                   ulTimeOut;


    /* Write the buffer address into the DATA field first. */
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_DATA, BUF_ADDR, uData, nBufAddress);
    SAND_HAL_WRITE(unit, KA, QM_FB_CACHE_ACC_DATA, uData);

    /* Initiating a WRITE */
    uData = 0x00;       /* Initiating a WRITE */
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, RD_WR_N, uData, 0);
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, REQ, uData, 1);
    SAND_HAL_WRITE(unit, KA, QM_FB_CACHE_ACC_CTRL, uData);

    /* Wait for the ACK. Also check if there was a cache overflow error */
    ulTimeOut = 100;
    while(ulTimeOut--) {
        uData = SAND_HAL_READ(unit, KA, QM_FB_CACHE_ACC_CTRL);
        if (SAND_HAL_GET_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData) == 1) {
            /* clear ack */
            uData = 0x00;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(unit, KA, QM_FB_CACHE_ACC_CTRL, uData);

            /* check that there is no Tail Cache Overflow */
            uData = SAND_HAL_READ(unit, KA, QM_ERROR2);
            if (SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_TAIL_CACHE_OVERFLOW, uData) == 1) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "QM_ERROR2, "
                                       " FB_TAIL_CACHE_OVERFLOW Set\n")));
#ifdef SCOREBOARD_DEBUG_2
                LOG_CLI((BSL_META_U(unit,
                                    "FB_TAIL_CACHE_OVERFLOW\n")));
#endif

                _sbx_qe2000_check_fb_depth(unit);

                /* clear error */
                uData = 0x00;
                uData = SAND_HAL_MOD_FIELD(KA, QM_ERROR2, FB_TAIL_CACHE_OVERFLOW, uData, 1);
                SAND_HAL_WRITE(unit, KA, QM_ERROR2, uData);

                status = SB_FAB_STATUS_TARGET_INDIRECT_MEMORY_TIMEOUT; /* need to add specific error code */
            }

            return(status);
        }

        uData = SAND_HAL_READ(unit, KA, QM_ERROR2);
        if (SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_TAIL_CACHE_OVERFLOW, uData) == 1) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "QM_ERROR2, FB_TAIL_CACHE_OVERFLOW Set\n")));

            _sbx_qe2000_check_fb_depth(unit);

            /* clear error */
            uData = 0x00;
            uData = SAND_HAL_MOD_FIELD(KA, QM_ERROR2, FB_TAIL_CACHE_OVERFLOW, uData, 1);
            SAND_HAL_WRITE(unit, KA, QM_ERROR2, uData);
            break;
        }
    }

    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Timeout waiting for QM_FB_CACHE_ACC_CTRL ACK\n")));
    return(SB_FAB_STATUS_TARGET_INDIRECT_MEMORY_TIMEOUT); /* need to add specific error code */
}


STATIC int
_sbx_qe2000_qm_fb_read(int unit, uint32 *pBufAddr)
{
    int              status = SB_FAB_STATUS_OK;
    uint32                   uData;
    uint32                   ulTimeOut;

    /* Initiating a WRITE */
    uData = 0x00;       /* Initiating a WRITE */
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, RD_WR_N, uData, 1);
    uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, REQ, uData, 1);
    SAND_HAL_WRITE(unit, KA, QM_FB_CACHE_ACC_CTRL, uData);

    /* Wait for the ACK. Also check if there was a cache overflow error */
    ulTimeOut = 100;
    while(ulTimeOut--) {
        uData = SAND_HAL_READ(unit, KA, QM_FB_CACHE_ACC_CTRL);
        if (SAND_HAL_GET_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData) == 1) {
            /* clear ack */
            uData = 0x00;
            uData = SAND_HAL_MOD_FIELD(KA, QM_FB_CACHE_ACC_CTRL, ACK, uData, 1);
            SAND_HAL_WRITE(unit, KA, QM_FB_CACHE_ACC_CTRL, uData);


            /* read data */
            uData = SAND_HAL_READ(unit, KA, QM_FB_CACHE_ACC_DATA);
            (*pBufAddr) = SAND_HAL_GET_FIELD(KA, QM_FB_CACHE_ACC_DATA, BUF_ADDR, uData);

            return(status);
        }

        uData = SAND_HAL_READ(unit, KA, QM_ERROR2);
        if (SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, uData) == 1) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW Set\n")));

            /* clear error */
            uData = 0x00;
            uData = SAND_HAL_MOD_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, uData, 1);
            SAND_HAL_WRITE(unit, KA, QM_ERROR2, uData);
            break;
        }
    }

    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Timeout waiting for QM_FB_CACHE_ACC_CTRL ACK\n")));
    return(SB_FAB_STATUS_TARGET_INDIRECT_MEMORY_TIMEOUT); /* need to add specific error code */
}

STATIC sbBool_t
_sbx_qe2000_is_buf_valid(int unit, uint32 BufAddr)
{
    uint32                   nMaskBit;


    if (BufAddr >= SCOREBOARD_CONTROL(unit)->nBuffersSbBuf) {
        return(FALSE);
    }

    if (SCOREBOARD_CONTROL(unit)->bIs256MbMemPart == FALSE) {
        return(TRUE);
    }

    /* 256 Mb Part */
    nMaskBit = (SCOREBOARD_CONTROL(unit)->bIsHalfBus == TRUE) ? QE2000_SB_HALFBUS_256_BIT2IGNORE :
        QE2000_SB_FULLBUS_256_BIT2IGNORE;
    return(((BufAddr & nMaskBit) ? FALSE : TRUE));
}



STATIC int
_sbx_qe2000_scoreboard_recover_buffer(int unit, uint32 nBufAddress)
{
    sbFabStatus_t              status = SB_FAB_STATUS_OK;

    /* Add this buffer into the Qm FreeBuffer Cache */
    if (Qe2000_Debug & 1) { /* for debugging */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Addr: 0x%x\n"),
                     nBufAddress));
    }

/*    LOG_CLI((BSL_META_U(unit,
                          "Add 0x%x\n"), nBufAddress)); */

    status = _sbx_qe2000_qm_fb_write(unit, nBufAddress);

    return(status);
}


STATIC int
_sbx_qe2000_init_fb_cache(int unit, sbBool_t bHdCacheEmpty)
{
    int              status = SB_FAB_STATUS_OK;
    uint32         nBufs, nBufAddr, nRdBufAddr, RegVal;
    sbBool_t         bMarkedFound = FALSE;

    SB_FAB_ENTER();

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Initializing Free Buffer Cache\n")));
#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "Initializing Free Buffer Cache\n")));
#endif

    if (bHdCacheEmpty == FALSE) {
        /* It is possible that tail cache overflow condition exists. Pop buffers */
        /* from head cache to ensure that buffers can be pushed into tail cache. */
        /* NOTE: Following code added to recover from tail cache overflow.       */

        /* clear interrupt */
        RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2);
        RegVal = SAND_HAL_MOD_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal, 1);
        SAND_HAL_WRITE(unit, KA, QM_ERROR2, RegVal);

        /* read back status */
        RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2);
        if (SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal) == 1) {
            ;
        }
        else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Poping Byffers from Free Buffer"
                                  " Cache (head cache Not Empty\n")));
            for (; (SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal) == 0); ) {
                status = _sbx_qe2000_qm_fb_read(unit, &nRdBufAddr);
                if( status ) {
                    LOG_CLI((BSL_META_U(unit,
                                        "nRdBufAddr 0x%x status 0x%x\n"), nRdBufAddr, status));
                }
                if (status != SB_FAB_STATUS_OK) {
                    return(status);
                }
                /* This was added to prevent infinite loop */
                RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2); 
            }
        }
    }

    /* add Marked buffer */
    status = _sbx_qe2000_qm_fb_write(unit, QE2000_MARKED_BUFFER_NUMBER);
    if (status != SB_FAB_STATUS_OK) {
        return(status);
    }

    /*
     * add all buffers to the free buffer cache. Tail buffer cache needs to
     * be flushed. Flushing the tail buffer cache ensure that there are no
     * duplicate buffers in thefree buffer cache.
     *   - start adding free buffers.
     *   - if marked buffer is not removed, check header buffer cache for
     *     available buffers. Read buffers from the header cache till marked
     *     buffer is not read. This results in flushing the free tail buffer
     *     cache.
     */
    for (nBufs = 0, nBufAddr = 0;
            nBufs < (SCOREBOARD_CONTROL(unit)->nBuffers - SCOREBOARD_CONTROL(unit)->nBufsDelta); nBufAddr++) {

        if (_sbx_qe2000_is_buf_valid(unit, nBufAddr) == FALSE) {
            continue;
        }

        status = _sbx_qe2000_qm_fb_write(unit, nBufAddr);
        if (status != SB_FAB_STATUS_OK) {
            return(status);
        }

        nBufs++;

        if (bMarkedFound == TRUE) {
            continue;
        }

        /* attempt popping Marked buffer */
        if ((nBufs % QE2000_MARKED_BUFFER_CHECK_THRESHOLD) == 0) {

            /* clear interrupt */
            RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2);
            RegVal = SAND_HAL_MOD_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal, 1);
            SAND_HAL_WRITE(unit, KA, QM_ERROR2, RegVal);

            /* read back status */
            RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2);
            if (SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal) == 1) {
                continue;
            }

            for (; (SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal) == 0); ) {
                status = _sbx_qe2000_qm_fb_read(unit, &nRdBufAddr);
                if (status != SB_FAB_STATUS_OK) {
                    return(status);
                }

                if (nRdBufAddr == QE2000_MARKED_BUFFER_NUMBER) {
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "Found Marked Buffer\n")));
                    bMarkedFound = TRUE;
                    break;
                }

                RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2);
            }

        }

    }

    /* clear interrupt */
    RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2);
    RegVal = SAND_HAL_MOD_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal, 1);
    SAND_HAL_WRITE(unit, KA, QM_ERROR2, RegVal);

    SB_FAB_LEAVE();

    return(status);
}



STATIC void 
_sbx_qe2000_check_fb_depth(int unit)
{
    uint32                   hdCacheWptr, hdCacheRptr, nHeadDepth, hdCacheData;
    uint32                   tailCacheWptr, tailCacheRptr, nTailDepth, tailCacheData;
    uint32                   extMemWptr, extMemRptr, nExtMemDepth, extMemData1, extMemData2;

    COMPILER_REFERENCE(nHeadDepth);
    COMPILER_REFERENCE(nTailDepth);
    COMPILER_REFERENCE(nExtMemDepth);

    /* free buffer tail cache depth */
    tailCacheData = SAND_HAL_READ(unit, KA, QM_FB_TAIL_CACHE_DEBUG);
    tailCacheWptr = SAND_HAL_GET_FIELD(KA, QM_FB_TAIL_CACHE_DEBUG, FB_TAIL_CACHE_WPTR, tailCacheData);
    tailCacheRptr = SAND_HAL_GET_FIELD(KA, QM_FB_TAIL_CACHE_DEBUG, FB_TAIL_CACHE_RPTR, tailCacheData);
    nTailDepth = ((tailCacheWptr > tailCacheRptr) ? (tailCacheWptr - tailCacheRptr) :
            ((0x100 - tailCacheRptr) + tailCacheWptr));

    /* free buffer head cache depth */
    hdCacheData = SAND_HAL_READ(unit, KA, QM_FB_HEAD_CACHE_DEBUG);
    hdCacheWptr = SAND_HAL_GET_FIELD(KA, QM_FB_HEAD_CACHE_DEBUG, FB_HEAD_CACHE_WPTR, hdCacheData);
    hdCacheRptr = SAND_HAL_GET_FIELD(KA, QM_FB_HEAD_CACHE_DEBUG, FB_HEAD_CACHE_RPTR, hdCacheData);
    nHeadDepth = ((hdCacheWptr > hdCacheRptr) ? (hdCacheWptr - hdCacheRptr) :
            ((0x100 - hdCacheRptr) + hdCacheWptr));

    /* free buffer external memory depth */
    extMemData1 = SAND_HAL_READ(unit, KA, QM_FB_EXT_MEMORY_DEBUG0);
    extMemRptr = SAND_HAL_GET_FIELD(KA, QM_FB_EXT_MEMORY_DEBUG0, FB_EXT_MEMORY_RPTR, extMemData1);

    extMemData2 = SAND_HAL_READ(unit, KA, QM_FB_EXT_MEMORY_DEBUG1);
    extMemWptr = SAND_HAL_GET_FIELD(KA, QM_FB_EXT_MEMORY_DEBUG1, FB_EXT_MEMORY_WPTR, extMemData2);

    nExtMemDepth = extMemWptr - extMemRptr; /* Need to be changed */

    LOG_WARN(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "tailCacheData: 0x%x, hdCacheData: 0x%x,"
                          " extMemData1: 0x%x, extMemData2: 0x%x\n"),
              tailCacheData, hdCacheData, extMemData1,
              extMemData2));

}


STATIC int
_sbx_qe2000_queue_state(int unit, uint8 *pState, sbBool_t bIsSave)
{
    int                  status = SB_FAB_STATUS_OK;
    uint32                       nQueue, nOper;
    uint32                       uData0, uData1, uData2, uData3, nIndex, nBit;


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "QueueState(%s)\n"),
                 ((bIsSave == TRUE) ? "Save" : "Restore")));

    for (nQueue = 0, nOper = 0;  nQueue  < SB_FAB_DEVICE_QE2000_NUM_QUEUES; nQueue++) {

        nIndex = nQueue / 8;
        nBit   = nQueue % 8;

        /* determine if restore operation need to be done */
        if ( (bIsSave == FALSE) && ((*(pState + nIndex) & (1 << nBit)) == 0) ) {
            /* queue was not enabled */
            continue;
        }

        /* read queue state entry */
        status = soc_qe2000_qm_mem_read(unit, nQueue, 0x00, &uData0, &uData1, &uData2, &uData3);
        if (status != SB_FAB_STATUS_OK) {
            return(status);
        }

        if (bIsSave == TRUE) { /* Save State */
            if ( ((uData0 & QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_MASK) >>
                        QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_SHFT) == 1) {
                /* queue enabled, disable queue */
                uData0 &= ~(QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_MASK);
            }
            else {
                /* queue already disabled */
                continue;
            }
        }
        else { /* restore state */
            /* enable queue */
            uData0 |= QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_MASK;
        }

        status = soc_qe2000_qm_mem_write(unit, nQueue, 0x00, uData0, uData1, uData2, uData3);
        if (status != SB_FAB_STATUS_OK) {
            return(status);
        }

        /* update state maintained in FLIB (required for restore) */
        nOper++;
        if (bIsSave == TRUE) {
            *(pState + nIndex) |= (1 << nBit);
        }
        else {
            *(pState + nIndex) &= ~(1 << nBit);
        }
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "QueueState(%s): Oper: 0x%x\n"),
                 ((bIsSave == TRUE) ? "Save" : "Restore"), nOper));

    return(status);
}



STATIC int
_sbx_qe2000_init_free_buffer_cache(int unit)
{
    int    status = SB_FAB_STATUS_OK, rc = SB_FAB_STATUS_OK;
    uint8                       *pState;
    uint32                       RegVal, nFbHeadCacheVal, hdCacheWptr, hdCacheRptr;
    sbBool_t                       bHdCacheEmpty = TRUE;



    /* consistency check */
    nFbHeadCacheVal = SAND_HAL_READ(unit, KA, QM_FB_HEAD_CACHE_DEBUG);
    hdCacheWptr = SAND_HAL_GET_FIELD(KA, QM_FB_HEAD_CACHE_DEBUG, FB_HEAD_CACHE_WPTR, nFbHeadCacheVal);
    hdCacheRptr = SAND_HAL_GET_FIELD(KA, QM_FB_HEAD_CACHE_DEBUG, FB_HEAD_CACHE_RPTR, nFbHeadCacheVal);
    RegVal = SAND_HAL_READ(unit, KA, QM_ERROR2);

    if ((SAND_HAL_GET_FIELD(KA, QM_ERROR2, FB_HEAD_CACHE_UNDERFLOW, RegVal) != 1) &&
            (hdCacheWptr != hdCacheRptr) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Consistency Check Failed, Head Cache"
                               " UnderFlow Not Detected %x %x Error 0x%x\n"),
                   hdCacheWptr, hdCacheRptr, RegVal));
        bHdCacheEmpty = FALSE;
    }

    /* allocate buffer to store queue state */
    pState = sal_alloc((SB_FAB_DEVICE_QE2000_NUM_QUEUES / 8) + 1, "queue_state");;
    if (pState == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "QueueState, Malloc Failed\n")));
        return(SB_FAB_STATUS_USER_MALLOC_FAILED);
    }
    sal_memset((uint8 *)pState, 0, ((SB_FAB_DEVICE_QE2000_NUM_QUEUES / 8) + 1));

    /* disable traffic and save queue state */
    status = _sbx_qe2000_queue_state(unit, pState, TRUE);
    if (status!= SB_FAB_STATUS_OK) {
        rc = status;
    }

    /* stop scoreboard churner */
    _sbx_qe2000_set_churner_state(unit, FALSE);

    /* initialize free buffer cache */
    status = _sbx_qe2000_init_fb_cache(unit, bHdCacheEmpty);
    if (status!= SB_FAB_STATUS_OK) {
        rc = status;
    }

    /* restore scoreboard churner */
    _sbx_qe2000_set_churner_state(unit, TRUE);

    /* enable traffic and restore queue state */
    status = _sbx_qe2000_queue_state(unit, pState, FALSE);
    if (status!= SB_FAB_STATUS_OK) {
        rc = status;
    }

    /* free queue state buffer */
    sal_free(pState);

    if (rc != SB_FAB_STATUS_OK) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Marking Checkboard State as Faulted\n")));
        SCOREBOARD_CONTROL(unit)->bIsFaulted = TRUE;
    }
    return(rc);
}


STATIC int
_sbx_qe2000_scoreboard_process(int unit, uint32 *pNbrBufFreed)
{
    int                            status = SB_FAB_STATUS_OK;
    uint32                       i, j, nBufs;
    uint8                        nUsed;
    uint32                       nLostBuffer;
    uint32                       nAddr, nBuffers;
    uint8                       *pScoreboardBuf;
    uint32                       startBufAddr, endBufAddr, nMask, nbrBufs;
    int                            flag = 0;

    COMPILER_REFERENCE(flag);

    pScoreboardBuf = SCOREBOARD_CONTROL(unit)->pScoreboardBuf;
    nBuffers = SCOREBOARD_CONTROL(unit)->nBuffers;

    (*pNbrBufFreed) = 0x00;

#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "scoreboard process nBuffers 0x%x Scoreboard at 0x%x\n"), nBuffers, (unsigned int) pScoreboardBuf));
#endif

    for (i = 0, startBufAddr = 0; (i < (nBuffers - SCOREBOARD_CONTROL(unit)->nBufsDelta)) && (status == SB_FAB_STATUS_OK); ) {
        nBufs = ((startBufAddr + QE2000_SB_NBUFS_IN_BYTE) < SCOREBOARD_CONTROL(unit)->nBuffersSbBuf) ?
            QE2000_SB_NBUFS_IN_BYTE :
            (SCOREBOARD_CONTROL(unit)->nBuffersSbBuf - startBufAddr);

        endBufAddr = startBufAddr + nBufs;
        nAddr = startBufAddr / QE2000_SB_NBUFS_IN_BYTE;

        status = _sbx_qe2000_get_buf_info(unit, startBufAddr, endBufAddr, &nMask, &nbrBufs);

        /* determine if there are no valid buffers in this range */
        if (nbrBufs == 0) {
            startBufAddr += nBufs;
            i += nbrBufs;
            continue;
        }

        nUsed = *(pScoreboardBuf + nAddr);

        for (j = 0; (j < nBufs) && (status == SB_FAB_STATUS_OK); j++) {

            /* check if it is a valid buffer */
            if ( (nMask & (1 << j)) == 0) {
                continue;
            }

            /* Check if USED bit is not set. If not set the buffer is lost */
            if ((nUsed & (1 << j)) == 0) {
                nLostBuffer = (nAddr * QE2000_SB_NBUFS_IN_BYTE) + j;

                /* recover buffer */
#ifdef SCOREBOARD_DEBUG
                if (nAddr < 0x20) {
                    LOG_CLI((BSL_META_U(unit,
                                        "* RECOVER * 0x%x:0x%x nUsed 0x%x 0x%x 0x%x 0x%x\n"),
                             nAddr, nLostBuffer, nUsed, j, startBufAddr, endBufAddr));
                }
#endif
                status = _sbx_qe2000_scoreboard_recover_buffer(unit, nLostBuffer);
                if (status == SB_FAB_STATUS_OK) {
                    (*pNbrBufFreed)++;
                    nTotalFreed++;
                    if ((nTotalFreed % FREED_BUF_MODULO) == 0) {
                        flag = 1;
                    }

                } else {
                    LOG_CLI((BSL_META_U(unit,
                                        "recover error 0x%x\n"), status));
                }
            }
        }

        startBufAddr += nBufs;
        i += nbrBufs;
    }
#ifdef SCOREBOARD_DEBUG
    if (flag) {
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard Freed Buffers %d, Lost Buffers %d \n"),nTotalFreed, nTotalLost));
    }
#endif

    return(status);
}



/*
 * Function:
 *     _sbx_qe2000_check_scoreboard
 * Purpose:
 *     check Qe2000 scoreboard
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */

STATIC int
_sbx_qe2000_scoreboard_check(int unit, int *p_nbr_buffers, int *p_min_poll_intvl_in_usec )

{
    int         rv = SOC_E_NONE;
    uint32    RegVal;
    uint32    fb_buf_alloc_per_ts;
    uint32    RegLnaConfigVal, AgeVal;
    uint32    OldAge, NewAge;
    uint64    nScoreBoardTimePeriodNs, nAgingTimePeriodNs;
    uint32      minPollTmp = 0;
    uint32      nBufTmp = 0;
    sbBool_t    bAgingJustConfigured = FALSE; 
    sal_usecs_t curScoreboardTime;
    uint32    nPollIntvlDiffInUsecs, nBufsFreed = 0, nBufToFree;
    sbBool_t    bForceBufInit = FALSE;
#ifdef NOT_YET
    sbBool_t    bSuccess;
#endif

#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "start _sbx_qe2000_scoreboard_check....\n")));
#endif

    /* consistency check, determine if the device is initialized */
    RegVal = SAND_HAL_READ(unit , KA, QM_FB_CONFIG0);
    fb_buf_alloc_per_ts = SAND_HAL_GET_FIELD(KA, QM_FB_CONFIG0, FB_BUF_ALLOC_PER_TS, RegVal);
    if ((fb_buf_alloc_per_ts == QE2000_BUF_ALLOC_PER_TS_DISABLE) ||
            (fb_buf_alloc_per_ts == QE2000_BUF_ALLOC_PER_TS_DEFAULT) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Scoreboarding(unit %d) not performed,"
                               " Incorrect Setup. (%d=%d) (%d=%d)\n"),
                   unit, fb_buf_alloc_per_ts,
                   QE2000_BUF_ALLOC_PER_TS_DISABLE,
                   fb_buf_alloc_per_ts,
                   QE2000_BUF_ALLOC_PER_TS_DEFAULT));
        return(SOC_E_INTERNAL);
    } else {
#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "fb_buf_alloc_per_ts %d\n"), fb_buf_alloc_per_ts));
#endif
    }

    /* check if the functionality is faulted */
    if (SCOREBOARD_CONTROL(unit)->bIsFaulted == TRUE) {
        return(SB_FAB_STATUS_INIT_FAILED);
    }

     RegLnaConfigVal = SAND_HAL_READ(unit, KA, QS_LNA_CONFIG);


    /* scoreboard related initialization */
    if ((SCOREBOARD_CONTROL(unit)->nBuffers == 0) || (SCOREBOARD_CONTROL(unit)->pScoreboardBuf == SB_FAB_NULL_POINTER)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(%d) First Poll\n"),
                     unit));
        AgeVal = SAND_HAL_READ(unit, KA, QM_CONFIG3);
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(%d) AgeVal: 0x%x(%d)\n"),
                     unit, AgeVal, AgeVal));

        RegVal = SAND_HAL_READ(unit, KA, QM_CONFIG0);
        SCOREBOARD_CONTROL(unit)->nBuffers = SAND_HAL_GET_FIELD(KA, QM_CONFIG0, TOTAL_BUFS_AVL, RegVal);

        /* check if hw aging needs to be enabled */
        if ( (AgeVal == 0) || (AgeVal == QE2000_HW_AGE_SCALER_DEF_VAL) ||
                (SAND_HAL_GET_FIELD(KA, QM_CONFIG0, HW_AGE_ENABLE, RegVal) == 0) ) {
            if ((AgeVal == 0) || (AgeVal == QE2000_HW_AGE_SCALER_DEF_VAL) ) {
                if (SAND_HAL_GET_FIELD(KA, QS_LNA_CONFIG, TME_MODE, RegLnaConfigVal) == 0) { /* FIC Mode */
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "QE (%d) in FIC Mode\n"),
                                 unit));
                    AgeVal = QE2000_HW_AGE_FIC_SCALER_DEF_VAL;
                }
                else { /* TME MODE */
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "QE ( %d) in TME Mode\n"),
                                 unit));
                    AgeVal = QE2000_HW_AGE_TME_SCALER_DEF_VAL;
                }
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "Scoreboard(%d) Updated"
                                         " AgeVal: 0x%x(%d)\n"),
                             unit, AgeVal, AgeVal));
                SAND_HAL_WRITE(unit, KA, QM_CONFIG3, AgeVal);
            }

            /* enable aging */
#ifdef SCOREBOARD_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "* Scoreboard(%d) Enable Aging 0x%x \n"), unit, AgeVal));
#endif

            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Scoreboard(%d) Enable Aging\n"),
                         unit));
            RegVal = SAND_HAL_MOD_FIELD(KA, QM_CONFIG0, HW_AGE_ENABLE, RegVal, 1);
            SAND_HAL_WRITE(unit, KA, QM_CONFIG0, RegVal);
            bAgingJustConfigured = TRUE;
        }
        SCOREBOARD_CONTROL(unit)->nCurrentAgeValue = AgeVal;

#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "calc poll _sbx_qe2000_scoreboard_check....\n")));
#endif

        /* calculate minimum poll interval */
        AgeVal += QE2000_HW_AGE_SCALER_2SW_OFFSET_VAL;
	

	nBufTmp = (SCOREBOARD_CONTROL(unit)->nBuffers / fb_buf_alloc_per_ts) + 1;
        COMPILER_64_SET(nScoreBoardTimePeriodNs,0, nBufTmp);
        COMPILER_64_UMUL_32(nScoreBoardTimePeriodNs, SB_FAB_MAX_TIMESLOT_IN_NS);
        COMPILER_64_SET(nAgingTimePeriodNs,0, AgeVal);
        COMPILER_64_UMUL_32(nAgingTimePeriodNs, 4 * 15);
	if (COMPILER_64_GT(nScoreBoardTimePeriodNs, nAgingTimePeriodNs)) {
	    if (soc_sbx_div64(nScoreBoardTimePeriodNs, 1000, &minPollTmp) == -1) {
		return (SOC_E_INTERNAL);
	    }
	    SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs = minPollTmp + 1;
	} else {
	    if (soc_sbx_div64(nAgingTimePeriodNs, 1000, &minPollTmp) == -1) {
		return (SOC_E_INTERNAL);
	    }
	    SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs = minPollTmp + 1;
	}
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard(%d) Time(nsecs): (0x%x%08x) Aging Time(nsecs): (0x%x%08x)\n"),
                 unit, COMPILER_64_HI(nScoreBoardTimePeriodNs), COMPILER_64_LO(nScoreBoardTimePeriodNs),  COMPILER_64_HI(nAgingTimePeriodNs), COMPILER_64_LO(nAgingTimePeriodNs)));
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(%d) Time(nsecs): (0x%x%08x)"
                                 " Aging Time(nsecs): (0x%x%08x)\n"),
                     unit, COMPILER_64_HI(nScoreBoardTimePeriodNs), COMPILER_64_LO(nScoreBoardTimePeriodNs),
                     COMPILER_64_HI(nAgingTimePeriodNs), COMPILER_64_LO(nAgingTimePeriodNs)));
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(unit %d) Current Time"
                                 " usecs: (%d) (0x%x)\n"), unit,
                     SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs,
                     SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs));

        if (SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs < QE2000_MIN_SCOREBOARD_POLL_INTVL_US) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Scoreboard(%d) Setting Default"
                                     " Time: (%d)\n"), unit,
                         SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs));
            SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs = QE2000_MIN_SCOREBOARD_POLL_INTVL_US;
        }

        /* Set scoreboard polling interval */
        if (SAND_HAL_GET_FIELD(KA, QS_LNA_CONFIG, TME_MODE, RegLnaConfigVal) == 0) { /* FIC Mode */
            /* Double the polling interval */
            SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs =
                QE2000_SCOREBOARD_FIC_POLL_FACTOR * SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs;
        }
        else { /* TME MODE */
            /* quadruple the polling interval */
            SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs =
                QE2000_SCOREBOARD_TME_POLL_FACTOR * SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs;
        }

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(%d) FinalTime in"
                                 " usecs (0x%x) secs (0x%x)\n"), unit,
                     SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs,
                     SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs/(1000*1000)));

#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard(%d) FinalTime in usecs (0x%x) secs (0x%x)\n"),
                 unit, SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs,
                 SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs/(1000*1000)));
        LOG_CLI((BSL_META_U(unit,
                            "modes _sbx_qe2000_scoreboard_check.... AgingTP 0x%x%08x, AgeVal 0x%x\n"), COMPILER_64_HI(nAgingTimePeriodNs), COMPILER_64_LO(nAgingTimePeriodNs), AgeVal));
#endif

        /* Determine Half Bus/Full Bus mode, 512Mb/256Mb configuration */
        RegVal = SAND_HAL_READ(unit, KA, PM_CONFIG0);
        SCOREBOARD_CONTROL(unit)->bIsHalfBus = SAND_HAL_GET_FIELD(KA, PM_CONFIG0, MEM_MODE, RegVal);


        if (SCOREBOARD_CONTROL(unit)->bIsHalfBus == TRUE) {

            if ( (SCOREBOARD_CONTROL(unit)->nBuffers != QE2000_BUF_AVL_HALFBUS_512_CONFIG) &&
                    (SCOREBOARD_CONTROL(unit)->nBuffers != QE2000_BUF_AVL_HALFBUS_256_CONFIG) ) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Scoreboarding(nId %d) not performed,"
                                       " Incorrect Setup Buffers"
                                       " Available: 0x%x\n"), unit,
                           SCOREBOARD_CONTROL(unit)->nBuffers));
                SCOREBOARD_CONTROL(unit)->bIsFaulted = TRUE;
                return(SB_FAB_STATUS_INIT_FAILED);
            }

            SCOREBOARD_CONTROL(unit)->bIs256MbMemPart =
                (SCOREBOARD_CONTROL(unit)->nBuffers == QE2000_BUF_AVL_HALFBUS_256_CONFIG) ? TRUE : FALSE;

        }

        if (SCOREBOARD_CONTROL(unit)->bIsHalfBus == FALSE) {

            if ( (SCOREBOARD_CONTROL(unit)->nBuffers != QE2000_BUF_AVL_FULLBUS_512_CONFIG) &&
                    (SCOREBOARD_CONTROL(unit)->nBuffers != QE2000_BUF_AVL_FULLBUS_256_CONFIG) ) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Scoreboarding(nId %d) not performed,"
                                       " Incorrect Setup Buffers Available:"
                                       " 0x%x\n"), unit,
                           SCOREBOARD_CONTROL(unit)->nBuffers));
                SCOREBOARD_CONTROL(unit)->bIsFaulted = TRUE;
                return(SB_FAB_STATUS_INIT_FAILED);
            }

            SCOREBOARD_CONTROL(unit)->bIs256MbMemPart =
                (SCOREBOARD_CONTROL(unit)->nBuffers == QE2000_BUF_AVL_FULLBUS_256_CONFIG) ? TRUE : FALSE;

        }

        /* determine the size of the scoreboard buffer to allocate */
        SCOREBOARD_CONTROL(unit)->nBuffersSbBuf = SCOREBOARD_CONTROL(unit)->nBuffers;
        if (SCOREBOARD_CONTROL(unit)->bIs256MbMemPart == TRUE) {
            SCOREBOARD_CONTROL(unit)->nBuffersSbBuf = SCOREBOARD_CONTROL(unit)->nBuffers * 2;
            SCOREBOARD_CONTROL(unit)->nBufsDelta = QE2000_SB_BUFS_DELTA;
        }

        /* allocate scoreboard buffer: each QE-2000 buffer has a bit in scoreboard. With a maximum of 512M
	 memory, we need 131072 bits, or 16384 bytes, or 4096 32-bit words*/
        SCOREBOARD_CONTROL(unit)->pScoreboardBuf = soc_cm_salloc(unit, 4096*4,
								 "scoreboard_buffer");
        if (SCOREBOARD_CONTROL(unit)->pScoreboardBuf == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Scoreboard(%d) Malloc Failed\n"),
                       unit));
            return(SB_FAB_STATUS_USER_MALLOC_FAILED);
        }
#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard(%d) IsHalfBus: %d Is256MbPart: %d nBuffers: 0x%x NSbBuffers: 0x%x\n"),
                 unit, SCOREBOARD_CONTROL(unit)->bIsHalfBus, SCOREBOARD_CONTROL(unit)->bIs256MbMemPart,
                 SCOREBOARD_CONTROL(unit)->nBuffers, SCOREBOARD_CONTROL(unit)->nBuffersSbBuf));
        LOG_CLI((BSL_META_U(unit,
                            "**********   Aging 0x%x   **********************\n"), AgeVal));
        LOG_CLI((BSL_META_U(unit,
                            "**********   Scoreboard Alloc 0x%x  size 0x%x ********\n"),
                 (unsigned int) SCOREBOARD_CONTROL(unit)->pScoreboardBuf,
                 QE2000_MARKED_BUFFER_NUMBER + 1));
#endif
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(%d) IsHalfBus: %d Is256MbPart: %d"
                                 " nBuffers: 0x%x NSbBuffers: 0x%x\n"),
                     unit, SCOREBOARD_CONTROL(unit)->bIsHalfBus,
                     SCOREBOARD_CONTROL(unit)->bIs256MbMemPart,
                     SCOREBOARD_CONTROL(unit)->nBuffers,
                     SCOREBOARD_CONTROL(unit)->nBuffersSbBuf));
    } else {

        /* Check for Aging reconfig */

        NewAge = SAND_HAL_READ(unit, KA, QM_CONFIG3);
        AgeVal = NewAge;
        OldAge = SCOREBOARD_CONTROL(unit)->nCurrentAgeValue;

        if(OldAge != NewAge) {
            /* Aging value was changed recalculate interval and defer to next cycle */
            LOG_CLI((BSL_META_U(unit,
                                "Aging Old 0x%x New 0x%x\n"), OldAge, NewAge));

            bAgingJustConfigured = 1;
            SCOREBOARD_CONTROL(unit)->nCurrentAgeValue = NewAge;


            /* calculate minimum poll interval */
            AgeVal += QE2000_HW_AGE_SCALER_2SW_OFFSET_VAL;
	    
	    nBufTmp = (SCOREBOARD_CONTROL(unit)->nBuffers / fb_buf_alloc_per_ts) + 1;
	    COMPILER_64_SET(nScoreBoardTimePeriodNs,0, nBufTmp);
	    COMPILER_64_UMUL_32(nScoreBoardTimePeriodNs, SB_FAB_MAX_TIMESLOT_IN_NS);

            COMPILER_64_SET(nAgingTimePeriodNs, 0, AgeVal);
            COMPILER_64_UMUL_32(nAgingTimePeriodNs, 4*15);

	    if (COMPILER_64_GT(nScoreBoardTimePeriodNs, nAgingTimePeriodNs)) {
		if (soc_sbx_div64(nScoreBoardTimePeriodNs, 1000, &minPollTmp) == -1) {
		    return (SOC_E_INTERNAL);
		}
		SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs = minPollTmp + 1;
	    } else {
		if (soc_sbx_div64(nAgingTimePeriodNs, 1000, &minPollTmp) == -1) {
		    return (SOC_E_INTERNAL);
		}
		SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs = minPollTmp + 1;
	    }
#ifdef SCOREBOARD_DEBUG_2
            LOG_CLI((BSL_META_U(unit,
                                "Scoreboard(%d) Time(nsecs): (0x%x%08x) Aging Time(nsecs): (0x%x%08x)\n"),
                     unit, COMPILER_64_HI(nScoreBoardTimePeriodNs), COMPILER_64_LO(nScoreBoardTimePeriodNs),  COMPILER_64_HI(nAgingTimePeriodNs), COMPILER_64_LO(nAgingTimePeriodNs)));
#endif

            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Scoreboard(%d) Time(nsecs): (0x%x%08x)"
                                     " Aging Time(nsecs): (0x%x%08x)\n"),
                         unit, COMPILER_64_HI(nScoreBoardTimePeriodNs), COMPILER_64_LO(nScoreBoardTimePeriodNs),
                         COMPILER_64_HI(nAgingTimePeriodNs), COMPILER_64_LO(nAgingTimePeriodNs)));

            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Scoreboard(unit %d) Current Time"
                                     " usecs: (%d) (0x%x)\n"), unit,
                         SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs,
                         SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs));

            if (SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs < QE2000_MIN_SCOREBOARD_POLL_INTVL_US) {
                SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs = QE2000_MIN_SCOREBOARD_POLL_INTVL_US;
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "Scoreboard(%d) Setting Default"
                                         " Time: (%d)\n"), unit,
                             SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs));
            }

            /* Set scoreboard polling interval */
            if (SAND_HAL_GET_FIELD(KA, QS_LNA_CONFIG, TME_MODE, RegLnaConfigVal) == 0) { /* FIC Mode */
                /* Double the polling interval */
                SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs =
                    QE2000_SCOREBOARD_FIC_POLL_FACTOR * SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs;
            }
            else { /* TME MODE */
                /* quadruple the polling interval */
                SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs =
                    QE2000_SCOREBOARD_TME_POLL_FACTOR * SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs;
            }

            LOG_CLI((BSL_META_U(unit,
                                "Scoreboard(%d) FinalTime in usecs (0x%x) secs (0x%x) Default:%d\n"),
                     unit, SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs,
                     SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs/(1000*1000), QE2000_MIN_SCOREBOARD_POLL_INTVL_US));

            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Scoreboard(%d) FinalTime in usecs"
                                     " (0x%x) secs (0x%x)\n"), unit,
                         SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs,
                         SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs/(1000*1000)));

            LOG_CLI((BSL_META_U(unit,
                                "************ Aging Reconfigured Was 0x%x, Now 0x%x. Updating Interval 0x%x. ********"),
                     OldAge, NewAge, SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs));

            /* Set the modified interval control */
            SCOREBOARD_COLLECT(unit).interval = SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs;

        }

    }

    /* update minimum poll interval */
#if 0
    SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs = INTERVAL_TOLERANCE; /* Add a small factor to avoid counter jitter */
#endif
    (*p_min_poll_intvl_in_usec) = SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs + INTERVAL_TOLERANCE;
    (*p_nbr_buffers) = 0;

    /* if this is first poll, simply return */
    if (bAgingJustConfigured == TRUE) {
#ifdef SCOREBOARD_DEBUG_2
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard(%d) Aging just configured      \n"), unit));
#endif
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(%d) Aging just configured\n"),
                     unit));
            Debug_StartScoreboardTime = SCOREBOARD_CONTROL(unit)->lastScoreboardTime;
            SCOREBOARD_CONTROL(unit)->lastScoreboardTime = sal_time_usecs();
        return(rv);
    }
    if (SCOREBOARD_CONTROL(unit)->lastScoreboardTime == 0) {
        /* update poll time */
#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "*******************Scoreboard(%d) First Poll ******************\n"), unit));
#endif
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboard(%d) First Poll\n"),
                     unit));
        Debug_StartScoreboardTime = SCOREBOARD_CONTROL(unit)->lastScoreboardTime;
        SCOREBOARD_CONTROL(unit)->lastScoreboardTime = sal_time_usecs();
        return(rv);
    }

    /* determine if the poll frequency is correct */
     curScoreboardTime = sal_time_usecs();
     nPollIntvlDiffInUsecs = _sbx_qe2000_poll_intvl(
             &SCOREBOARD_CONTROL(unit)->lastScoreboardTime, &curScoreboardTime);


     if (nPollIntvlDiffInUsecs < (SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs )) {
         /* 0 is returned  when a wrap occurs. Just retry next time */
         if(nPollIntvlDiffInUsecs) {
              LOG_CLI((BSL_META_U(unit,
                                  "Poll interval not met cur %d prev %d diff %d min %d\n"),
                       curScoreboardTime, SCOREBOARD_CONTROL(unit)->lastScoreboardTime,
                       nPollIntvlDiffInUsecs, SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs ));
              LOG_WARN(BSL_LS_SOC_COMMON,
                       (BSL_META_U(unit,
                                   "Scoreboard(nId %d) Poll Interval"
                                    " not met\n"), unit));
              nShortIntervals++;
         }
         return(rv);
     }

#ifdef SCOREBOARD_DEBUG
     LOG_CLI((BSL_META_U(unit,
                         "interval cur 0x%x diff 0x%x interval %d\n"), curScoreboardTime,
              nPollIntvlDiffInUsecs,  SCOREBOARD_CONTROL(unit)->nMinPollIntvlInUsecs));
#endif

     SCOREBOARD_CONTROL(unit)->nScoreboardPolls++; /* for debugging */

     LOG_VERBOSE(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Scoreboard INVOKED (%d)\n"),
                  SCOREBOARD_CONTROL(unit)->nScoreboardPolls));
#ifdef SCOREBOARD_DEBUG
     LOG_CLI((BSL_META_U(unit,
                         "Scoreboard INVOKED (%d)\n"), SCOREBOARD_CONTROL(unit)->nScoreboardPolls));
#endif

     /* Read and clear scoreboard buffer, PIO operation */

     nBufToFree = 0;

     rv = _sbx_qe2000_scoreboard_read(unit, &nBufToFree);
     if (rv != SB_FAB_STATUS_OK) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "qe2000_scoreboard_read(), Error: 0x%x"
                                " Unit %d\n"), rv, unit));
         goto ErrHandle;
     }

#ifdef SCOREBOARD_DEBUG
     LOG_CLI((BSL_META_U(unit,
                         "read_scoreboard returns  nBufToFree 0x%x\n"), nBufToFree));
#endif



     /* check if not in TME mode */
     /* NOTE: TME/Non-TME mode can also be cached by the scoreboard mechanism */
     RegVal = SAND_HAL_READ(unit, KA, QS_LNA_CONFIG);
     if (SAND_HAL_GET_FIELD(KA, QS_LNA_CONFIG, TME_MODE, RegVal) == 0) {

         /* Determine if SOTs are being received. During initialization "hwQe2000InitSci()" */
         /* configures "sc_config0.sci_sot_watchdog_thresh".                                */
         RegVal = SAND_HAL_READ(unit, KA, SC_ERROR);
         if (SAND_HAL_GET_FIELD(KA, SC_ERROR, SOT_WATCHDOG_TIMEOUT, RegVal) == 1) {

#ifndef _SB_SCOREBOARD_CLEAR_ERROR_

             /* It is the users responsibility to clear errors */

#ifdef SCOREBOARD_DEBUG
             LOG_CLI((BSL_META_U(unit,
                                 "Scoreboard Clear Error Error 0x%x\n"), RegVal));
             LOG_CLI((BSL_META_U(unit,
                                 "ATTEMPT to clear Watchdog\n")));
#endif
             LOG_WARN(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "SOT WatchDog Expired (0x%x),"
                                   " Checkboarding Skipped\n"), RegVal));
             nTotalDogs++;
             nTotalFreed = nTotalLost = 0;
             RegVal &= SAND_HAL_KA_SC_ERROR_SOT_WATCHDOG_TIMEOUT_MASK;
             SAND_HAL_WRITE(unit, KA, SC_ERROR, RegVal);
             goto ErrHandle;
#else /* _SB_SCOREBOARD_CLEAR_ERROR_ */
#ifdef SCOREBOARD_DEBUG
             LOG_CLI((BSL_META_U(unit,
                                 "ATTEMPT to clear Watchdog\n")));
#endif
             /* Attempt clearing SOT error condtion */
             RegVal &= SAND_HAL_KA_SC_ERROR_SOT_WATCHDOG_TIMEOUT_MASK;
             SAND_HAL_WRITE(unit, KA, SC_ERROR, RegVal);

             /* delay, for SOT condition to be again asserted. By introducing delay, "state" */
             /* does not have to be maintained internally. This value should be larger then  */
             /* the one configured in  "sc_config0.sci_sot_watchdog_thresh"                  */
             ((sbFabTargetDevice_t*)pQe)->pFabTarget->pTargetServices->pfnDelay((1000 * 25)); /* 25 us delay */

             /* If SOT error not cleared skip checkboarding */
             RegVal = SAND_HAL_READ(unit, KA, SC_ERROR);
             if (SAND_HAL_GET_FIELD(KA, SC_ERROR, SOT_WATCHDOG_TIMEOUT, RegVal) == 1) {
                 LOG_WARN(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "SOT WatchDog Expired (0x%x),"
                                       " Checkboarding Skipped\n"), RegVal));
                 goto ErrHandle;
             }

             /* if SOT error cleared, force initialization of free buffer list */
             LOG_WARN(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "SOT WatchDog Cleared\n")));
             bForceBufInit = TRUE;
#endif /* !_SB_CHECKBOARD_CLEAR_ERROR_ */
         }
     }

#ifdef SCOREBOARD_DEBUG
     LOG_CLI((BSL_META_U(unit,
                         "bForceBufInit %d nBufToFree %d > (nBuffers %d - nBufsDelta %d - ACCESS_SZ %d)\n"),
              bForceBufInit, nBufToFree, SCOREBOARD_CONTROL(unit)->nBuffers,SCOREBOARD_CONTROL(unit)->nBufsDelta,- QE2000_FB_CACHE_EXT_CACHE_ACCESS_SZ));
#endif

     /* determine if end condition/corner case is hit */
     if ( (bForceBufInit == TRUE) || (nBufToFree >
                 (SCOREBOARD_CONTROL(unit)->nBuffers - SCOREBOARD_CONTROL(unit)->nBufsDelta
                  - QE2000_FB_CACHE_EXT_CACHE_ACCESS_SZ)) ) {
#ifdef SCOREBOARD_DEBUG
         LOG_CLI((BSL_META_U(unit,
                             "****** Initialize Free Buffer Cache ******* \n")));
#endif
         LOG_WARN(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Initialize Free Buffer Cache,"
                               " unit (%d), Reason: %s\n"),
                   unit, ((bForceBufInit == TRUE) ?
                   "Watchdog Cleared" : "Flush Tail Cache")));
         rv = _sbx_qe2000_init_free_buffer_cache(unit);
         if (rv != SB_FAB_STATUS_OK) {
             LOG_ERROR(BSL_LS_SOC_COMMON,
                       (BSL_META_U(unit,
                                   "Scoreboarding(nId %d) Stopped,"
                                    " Free Buffer Init failed, error: 0x%x\n"),
                        unit, rv));
             rv = SB_FAB_STATUS_INIT_FAILED;
         }
         else {
             nBufsFreed = SCOREBOARD_CONTROL(unit)->nBuffers - SCOREBOARD_CONTROL(unit)->nBufsDelta;
         }
         goto ErrHandle;
     }

     /* process scoreboard buffer */
#ifdef SCOREBOARD_DEBUG
     LOG_CLI((BSL_META_U(unit,
                         "Process Scoreboard \n")));
#endif
     rv = _sbx_qe2000_scoreboard_process(unit, &nBufsFreed);
#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "Process Scoreboard.. freed 0x%x status 0x%x\n"), nBufsFreed, rv));
#endif
    if (rv != SB_FAB_STATUS_OK) {
#if 0
        rv = (nBufsFreed > 0) ? SB_FAB_STATUS_OK : rv;
#endif /* 0 */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sbx_qe2000_scoreboard_process,"
                               " Error: 0x%x nId %d\n"), rv, unit));

        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "Initialize Free Buffer Cache, (%d),"
                              " Reason: ERROR in Processing scoreboard"
                              " Buffer\n"), unit));
        rv = _sbx_qe2000_init_free_buffer_cache(unit);
        if (rv != SB_FAB_STATUS_OK) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Scoreboarding(%d) Stopped,"
                                   " Free Buffer Init failed, error: 0x%x\n"),
                       unit, rv));
            rv = SB_FAB_STATUS_INIT_FAILED;
        }
        else {
            nBufsFreed = SCOREBOARD_CONTROL(unit)->nBuffers - SCOREBOARD_CONTROL(unit)->nBufsDelta;
        }

        goto ErrHandle;
    }
    (*p_nbr_buffers) = nBufsFreed;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Scoreboard INVOKED (%d), FREED: 0x%x\n"),
                 SCOREBOARD_CONTROL(unit)->nScoreboardPolls,
                 nBufsFreed));


    SCOREBOARD_CONTROL(unit)->lastScoreboardTime = sal_time_usecs();
    Debug_PollInfo[Debug_pollIndex].ScoreboardTime = SCOREBOARD_CONTROL(unit)->lastScoreboardTime;
    Debug_PollInfo[Debug_pollIndex].nBufFreed = nBufsFreed;
    Debug_pollIndex = (Debug_pollIndex + 1) & (SB_FAB_DEVICE_QE2000_MAX_POLL_SAMPLES - 1);

    SB_FAB_LEAVE();

    return(rv);

ErrHandle:
    (*p_nbr_buffers) = nBufsFreed;


    SCOREBOARD_CONTROL(unit)->lastScoreboardTime = sal_time_usecs();
    Debug_PollInfo[Debug_pollIndex].ScoreboardTime = SCOREBOARD_CONTROL(unit)->lastScoreboardTime;
    Debug_PollInfo[Debug_pollIndex].nBufFreed = nBufsFreed;
    Debug_pollIndex = (Debug_pollIndex + 1) & (SB_FAB_DEVICE_QE2000_MAX_POLL_SAMPLES - 1);

    SB_FAB_LEAVE();


#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "end _sbx_qe2000_scoreboard_check....\n")));
#endif

    return(rv);

}

/*
 * Function:
 *     _sbx_qe2000_get_buf_info
 * Purpose:
 *     get Qe2000 buffer information
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */

STATIC int
_sbx_qe2000_get_buf_info(int  unit, uint32 startBufAddr,
        uint32 endBufAddr, uint32 *pMask, uint32 *pNbrBufs)
{
    int              rv = SOC_E_NONE;
    uint32                   i, j, nMaskBit;


    (*pMask) = 0x00;

    /* check if 512 Mb part */
    if (SCOREBOARD_CONTROL(unit)->bIs256MbMemPart == FALSE) {
        (*pNbrBufs) = endBufAddr - startBufAddr;
        if ((*pNbrBufs) == QE2000_SB_NBUFS_IN_BYTE) {
            (*pMask) = 0xFF;
        }
        else {
            for (j = 0; j < (*pNbrBufs); j++) {
                (*pMask) |= (1 << j);
            }
        }

        return(rv);
    }


    /* processing for 256 Mb part */
    nMaskBit = (SCOREBOARD_CONTROL(unit)->bIsHalfBus == TRUE) ? QE2000_SB_HALFBUS_256_BIT2IGNORE :
        QE2000_SB_FULLBUS_256_BIT2IGNORE;
    (*pNbrBufs) = 0;
    for (i = startBufAddr, j = 0; i < endBufAddr; i++, j++) {
        if (i & nMaskBit) {
            continue;
        }
        (*pNbrBufs)++;
        (*pMask) |= (1 << j);
    }

    return(rv);
}


STATIC int
_sbx_qe2000_scoreboard_read( int unit, uint32 *num_buffers_to_free) 

{
    uint32   uData0, uData1, uData2, uData3;
    uint32   i, j,  nBufs, nAddr, nBuffers;
    uint32   start_buf_addr, end_buf_addr, nMask;
    uint32   nbrBufs;
    int        rv = SOC_E_NONE;
    sbBool_t   bBufsNotFreed = FALSE;
    uint8    *pScoreboardBuf;
    uint32     regval0 = 0, regval1 = 0, regval2 = 0;
    uint32     check = 0, cnt = 0;


    uData0 = uData1 = uData2 = uData3 = 0;

    pScoreboardBuf = SCOREBOARD_CONTROL(unit)->pScoreboardBuf;
    nBuffers = SCOREBOARD_CONTROL(unit)->nBuffers;

#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "scoreboard read nBuffers 0x%x 0x%x Scoreboard @ 0x%x\n"), nBuffers, SCOREBOARD_CONTROL(unit)->nBuffersSbBuf, (unsigned int) pScoreboardBuf));
#endif

    /*
     * Read entire Scoreboard via DMA
     */

    *(uint32 *)(pScoreboardBuf+4095) = 0x01234567;

    regval0 = SAND_HAL_MOD_FIELD(KA, PC_DMA_COUNT, COUNT, regval0, 4096);
    SAND_HAL_WRITE(unit, KA, PC_DMA_COUNT, regval0);

    regval1 = (uint32) soc_cm_l2p(unit, (void *) pScoreboardBuf);
    SAND_HAL_WRITE(unit, KA, PC_DMA_PCI_ADDR, regval1);

    /* 
     * Use PRESERVE_ON_READ like a semaphore to indicate DMA done
     */
    regval2 = SAND_HAL_MOD_FIELD(KA, PC_DMA_CTRL, PRESERVE_ON_READ, regval2, 0);
    regval2 = SAND_HAL_MOD_FIELD(KA, PC_DMA_CTRL, REQ, regval2, 1);
    regval2 = SAND_HAL_MOD_FIELD(KA, PC_DMA_CTRL, ACK, regval2, 1);
    SAND_HAL_WRITE(unit, KA, PC_DMA_CTRL, regval2);

    check = 0;
    cnt = 0;
    while ((check == 0) && (++cnt < 50)) {
	sal_thread_yield();
	sal_usleep(1000); /* check after 1 millisecond */
	regval2 = SAND_HAL_READ(unit, KA, PC_DMA_CTRL);
	check = SAND_HAL_GET_FIELD(KA, PC_DMA_CTRL, PRESERVE_ON_READ, regval2);
	check |= SAND_HAL_GET_FIELD(KA, PC_DMA_CTRL, ACK, regval2);
    }

    /*
     * DMA did not complete in 50 milliseconds
     */
    if (cnt >= 50) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "scoreboard read failed after %d milliseconds, RegVal 0x%x\n"),
	           cnt, regval2));
	/* Double-check with the marker */
	if (*(uint32 *)(pScoreboardBuf+4095) == 0x01234567) {
	  SCOREBOARD_CONTROL(unit)->bIsFaulted = TRUE;
	  rv = SB_FAB_STATUS_TARGET_INDIRECT_MEMORY_TIMEOUT;
	  goto ErrHandle;
	}
    }
#ifdef SCOREBOARD_DEBUG
    else {
	LOG_WARN(BSL_LS_SOC_COMMON,
	         (BSL_META_U(unit,
	                     "scoreboard read DMA succeeded\n")));
    }
#endif

    for (i = 0, start_buf_addr = 0; i < nBuffers - SCOREBOARD_CONTROL(unit)->nBufsDelta; ) {

        nBufs = ((start_buf_addr + QE2000_SB_NBUFS_IN_BYTE) < SCOREBOARD_CONTROL(unit)->nBuffersSbBuf) ?
	    QE2000_SB_NBUFS_IN_BYTE : SCOREBOARD_CONTROL(unit)->nBuffersSbBuf - start_buf_addr;

        end_buf_addr = start_buf_addr + nBufs;

        nAddr = start_buf_addr/ QE2000_SB_NBUFS_IN_BYTE;

        /* Determine if there are no valid buffers in this range */
        rv = _sbx_qe2000_get_buf_info(unit, start_buf_addr, end_buf_addr, &nMask, &nbrBufs);


        if (nbrBufs == 0) {
            start_buf_addr += nBufs;
            i += nbrBufs;
            continue;
        }

#ifdef SCOREBOARD_POLL
        rv = soc_qe2000_qm_mem_read (unit, nAddr, 0x07, &uData0, &uData1, &uData2, &uData3);


        if (rv != SB_FAB_STATUS_OK) {
            goto ErrHandle;
        }

        /* update global scoreboard buffer */
        *(pScoreboardBuf + nAddr) = uData0;

        /* determine if the scoreboard location has to be cleared */
        if ((uData0 & nMask) != 0x00) {
            rv = soc_qe2000_qm_mem_write(unit, nAddr, 0x07, 0x00, uData1, uData2, uData3);
            if (rv != SB_FAB_STATUS_OK) {
                goto ErrHandle;
            }
        }
#else /* SCOREBOARD DMA */
	uData0 = *(pScoreboardBuf + nAddr);
#endif
        if ( (uData0 & nMask) != (0xFF & nMask) ) { /* Buffer Lost */

            for (j = 0; j < nBufs; j++) {

                /* check if it is a valid buffer */
                if ( (nMask & (1 << j)) == 0) {
                    continue;
                }

                /* Check if USED bit is not set. If not set the buffer is lost */
                if ((uData0 & (1 << j)) == 0) {
                    (*num_buffers_to_free)++;
                    nTotalLost++;
                }
            }

            if (bBufsNotFreed == FALSE) { /* for debugging */
                /* could also be the end condition */

                for (j = 0; j < nBufs; j++) {
                    /* check if it is a valid buffer */
                    if ( (nMask & (1 << j)) == 0) {
                        continue;
                    }

                    /* Check if USED bit is not set. If not set the buffer is lost */
                    if ((uData0 & (1 << j)) == 0) {
                        break;
                    }
                }

                if ( (start_buf_addr + nBufs) < SCOREBOARD_CONTROL(unit)->nBuffersSbBuf)  {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Buffers Lost, Addr: 0x%x"
                                          " Data: 0x%x, 1stBuf: 0x%x\n"),
                              (nAddr * 8),
                              (uData0 & nMask),
                              ((nAddr * 8) + j)));
                }
                else {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Buffers Lost (END CONDITION),"
                                          " Addr: 0x%x Data: 0x%x,"
                                          " 1stBuf: 0x%x\n"),
                              (nAddr * 8), (uData0 & nMask),
                              ((nAddr * 8) + j)));
                }
                bBufsNotFreed = TRUE;
            }
            if (Qe2000_Debug & 1) { /* for debugging */
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "Addr: 0x%x Data: 0x%x"),
                          (nAddr * 8), (uData0 & nMask)));
            }


	}

	start_buf_addr += nBufs;
	i += nbrBufs;
    }
ErrHandle:
#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "End scoreboard read\n")));
#endif
    return(rv);
}


/*
 * Function:
 *     soc_sbx_scoreboard_init
 * Purpose:
 *     Initialize software scoreboard collection module.
 * Parameters:
 *     unit        - Device number
 *     block_info  - Array of scoreboard block information
 *     block_count - Number of scoreboard blocks
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     The scoreboard module state is as follows, after a successful 'attach':
 *     - Collector thread is not running
 *     - scoreboard block sets are NULL (this means, caller must
 *       'add' the scoreboard sets that it wants the collector to
 *       gather statistics on.
 */
int
soc_sbx_qe2000_scoreboard_init(int unit)
{
    int  rv = SOC_E_NONE;
    int flags = 0;
    int interval = 1000000;
    int new_interval, nbr_buffers;
    int env_interval;

    if (soc_property_get(unit, spn_SOC_SCOREBOARD_ENABLE, 0) == 0) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Scoreboarding Disabled\n")));
        return(0);
        /* Get the environment variables */

    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_scoreboard_enable set - Enabling Scoreboard\n")));
    }


    if ((env_interval = soc_property_get(unit, spn_SOC_SCOREBOARD_INTERVAL, 2000000)) == 0) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_scoreboard_interval not set or 0 - using default %d\n"),
                     interval));
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_scoreboard_interval set %d. Applying... \n"),
                     env_interval));
        interval = env_interval;
    }

    UNIT_VALID_CHECK(unit);

    nTotalLost = 0;
    nTotalFreed = 0;
    nTotalDogs = 0;
    nShortIntervals = 0;

    /* Create Mutex lock */
    if (_mlock[unit] == NULL) {
        if ((_mlock[unit] = sal_mutex_create("soc_scoreboard_lock")) == NULL) {
            return SOC_E_MEMORY;
        }
    }

    SCOREBOARD_LOCK(unit);

    /*
     * scoreboard Handler
     *
     * If handler is null, allocate handler
     * Else, stop scoreboard thread and reset fields
     */
    if (SCOREBOARD_CONTROL(unit) == NULL) {
        SCOREBOARD_CONTROL(unit) = sal_alloc(sizeof(_scoreboard_control_t),
                                          "soc_scoreboard_control");
        if (SCOREBOARD_CONTROL(unit) == NULL) {
            SCOREBOARD_UNLOCK(unit);
            return SOC_E_MEMORY;
        }

        sal_memset(SCOREBOARD_CONTROL(unit), 0, sizeof(_scoreboard_control_t));

        SCOREBOARD_COLLECT(unit).thread_id = SAL_THREAD_ERROR;
        SCOREBOARD_COLLECT(unit).interval  = 0;
        SCOREBOARD_COLLECT(unit).trigger   = NULL;
        SCOREBOARD_COLLECT(unit).intr      = NULL;

    } else {
        /*
         * Stop scoreboard thread, if running, and
         * remove any previous scoreboard sets
         */
        /* Note that there is no current provision for stopping the scoreboard thread once started */
        rv = soc_sbx_scoreboard_stop(unit);
        if (SOC_SUCCESS(rv)) {
               LOG_VERBOSE(BSL_LS_SOC_COMMON,
                           (BSL_META_U(unit,
                                       " _soc_sbx_scoreboard_set_remove_all?\n")));
        }
        if (SOC_FAILURE(rv)) {
            SCOREBOARD_UNLOCK(unit);
            return rv;
        }
    }

    SCOREBOARD_CONTROL(unit)->nBufsDelta = 0;

    _sbx_qe2000_scoreboard_check(unit, &nbr_buffers, &new_interval);

#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "*************** New Interval %d  ************\n"),new_interval));
#endif

    if (new_interval > interval) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Requested interval too low %d. Adjusting %d\n"),
                     interval, new_interval));
        interval = new_interval;
    }

    SCOREBOARD_CONTROL(unit)->bIsHalfBus = FALSE;
    SCOREBOARD_CONTROL(unit)->bIs256MbMemPart = FALSE;
    SCOREBOARD_CONTROL(unit)->bIsFaulted = FALSE;


    SCOREBOARD_CONTROL(unit)->nBuffersSbBuf = SCOREBOARD_CONTROL(unit)->nBuffers;
     if(SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2 == FALSE) {
         /* Handle the 256 MB case */
        SCOREBOARD_CONTROL(unit)->bIs256MbMemPart = TRUE;
        SCOREBOARD_CONTROL(unit)->nBuffersSbBuf = SCOREBOARD_CONTROL(unit)->nBuffers * 2;
        SCOREBOARD_CONTROL(unit)->nBufsDelta = QE2000_SB_BUFS_DELTA;
    }


    SCOREBOARD_UNLOCK(unit);

#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "Starting scoreboard thread flags 0x%x Interval %d\n"), flags, interval));
#endif

    /* Start software counter collector */
    SOC_IF_ERROR_RETURN(soc_sbx_scoreboard_start(unit, flags, interval));


#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "soc_scoreboard_init: unit=%d rv=%d\n"), unit, rv));
#endif

    return rv;
}


/*
 * Function:
 *     soc_sbx_scoreboard_detach
 * Purpose:
 *     Stop and deallocate software scoreboard collection module.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Stop scoreboard task if running.
 *     Deallocate scoreboard collection buffers.
 *     Destroy handler semaphores.
 *     Deallocate scoreboard control handler.
 */
int
soc_sbx_scoreboard_detach(int unit)
{
#ifdef NOT_NEEDED
    int  block;
#endif

    UNIT_INIT_CHECK(unit);

    SOC_IF_ERROR_RETURN(soc_sbx_scoreboard_stop(unit));

    SCOREBOARD_LOCK(unit);

    /* Destroy semaphores */
    if ((SCOREBOARD_COLLECT(unit).trigger) != NULL) {
        sal_sem_destroy(SCOREBOARD_COLLECT(unit).trigger);
    }
    if ((SCOREBOARD_COLLECT(unit).intr) != NULL) {
        sal_sem_destroy(SCOREBOARD_COLLECT(unit).intr);
    }

    sal_free(SCOREBOARD_CONTROL(unit));
    SCOREBOARD_CONTROL(unit) = NULL;

    SCOREBOARD_UNLOCK(unit);

    sal_mutex_destroy(_mlock[unit]);
    _mlock[unit] = NULL;

    return SOC_E_NONE;
}



/*
 * Function:
 *      _soc_sbx_scoreboard_collect
 * Purpose:
 *      This routine gets called each time the scoreboard transfer has
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
int
_soc_sbx_scoreboard_collect(int unit)
{

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                (BSL_META_U(unit,
                            "soc_scoreboard_collect: unit=%d \n"),
                 unit));

    if (SOC_IS_SBX_QE2000(unit)) {

#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "Qe2000\n")));
#endif
        return SOC_E_NONE;
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_sbx_scoreboard_thread
 * Purpose:
 *     Master scoreboard collection and accumulation thread.
 * Parameters:
 *     unit_vp - Device unit number
 * Returns:
 *     Nothing, does not return.
 * Notes:
 *     Assumes valid unit and scoreboard initialized for that unit.
 */
STATIC void
_soc_sbx_scoreboard_thread(void *unit_vp)
{
    int                   unit = PTR_TO_INT(unit_vp);
    int                   rv;
    int                   interval;
    uint32                err_count = 0;
    sal_usecs_t           now;
    SCOREBOARD_ATOMIC_DEF    s;
    int                   sync_gnt = FALSE;
    _scoreboard_collector_t  *collect;
    int              nbr_buffers;
    int              new_interval;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "soc_scoreboard_thread: unit=%d\n"), unit));

    /*
     * The hardware timer can only be used for intervals up to about
     * 1.048 seconds.  This implementation uses a software timer (via
     * semaphore timeout) instead of the hardware timer.
     */

    collect = &SCOREBOARD_COLLECT(unit);

#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "soc_scoreboard_thread: unit=%d, interval %d\n"), unit, collect->interval));
#endif

    while ((interval = collect->interval) != 0) {
#ifdef COUNTER_BENCH
        sal_usecs_t     start_time;
#endif
        int             err = 0;

        /*
         * Use a semaphore timeout instead of a sleep to achieve the
         * desired delay between scans.  This allows this thread to exit
         * immediately when soc_sbx_scoreboard_stop wants it to.
         */

#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "soc_scoreboard_thread: sleep %d\n"), interval));
#endif
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_scoreboard_thread: sleep %d\n"), interval));

        (void)sal_sem_take(collect->trigger, interval + INTERVAL_TOLERANCE);

#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "soc_scoreboard_thread: slept %d \n"), collect->interval));
#endif
        if (collect->interval == 0) {       /* Exit signaled */

            break;
        }

        if (collect->sync_req) {
            sync_gnt = TRUE;
        }

#ifdef COUNTER_BENCH
        start_time = sal_time_usecs();
#endif

        /*
         * Add up changes to scoreboard values.
         */
        now = sal_time_usecs();
#ifdef SCOREBOARD_DEBUG
         LOG_CLI((BSL_META_U(unit,
                             "Thread 0x%x 0x%x delta %d  err %d\n"), now, collect->cur, now - collect->cur, err));
#endif
        SCOREBOARD_ATOMIC_BEGIN(s);
        collect->prev = collect->cur;
        collect->cur  = now;
        SCOREBOARD_ATOMIC_END(s);
#ifdef SCOREBOARD_DEBUG
         LOG_CLI((BSL_META_U(unit,
                             "Thread 0x%x 0x%x err %d\n"), collect->prev, collect->cur, err));
#endif

        if (!err) {
#ifdef SCOREBOARD_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "SCOREBOARD Lock\n")));
#endif
	    /* take bcm lock first */
	    BCM_SBX_LOCK(unit);
            SCOREBOARD_LOCK(unit);

            rv =  _sbx_qe2000_scoreboard_check(unit, &nbr_buffers, &new_interval);

#ifdef SCOREBOARD_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "scoreboard check returned nbrBuf 0x%x intvl 0x%x\n"), nbr_buffers, new_interval));
#endif

#if 0
            rv = SOC_E_NONE; 
#endif

            SCOREBOARD_UNLOCK(unit);
	    BCM_SBX_UNLOCK(unit);

#ifdef SCOREBOARD_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "SCOREBOARD Unlock\n")));
#endif
            if (rv != SOC_E_NONE ) {
                LOG_CLI((BSL_META_U(unit,
                                    "DEBUG Error on Scoreboard READ\n")));
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_scoreboard_thread: collect failed: %s\n"),
                           soc_errmsg(rv)));
                err = 1;
            } else {
#ifdef SCOREBOARD_DEBUG
                LOG_CLI((BSL_META_U(unit,
                                    "DEBUG Scoreboard Check done\n")));
#endif

            }

        }

        /*
         * Forgive spurious errors
         */
        if (err) {
            if (++err_count == soc_property_get(unit,
                                                spn_SOC_CTR_MAXERR, 5)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_scoreboard_thread: Too many errors\n")));
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
            collect->sync_req = 0;
            sync_gnt = 0;
        }
#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "SB While at end Interval %d TotLost %d TotFreed %d Total Watchdog Resets %d\n\n"),
                 collect->interval, nTotalLost, nTotalFreed, nTotalDogs));
#endif
    }

    rv = SOC_E_NONE;

 done:
#ifdef SCOREBOARD_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "SCOREBOARD_DEBUG thread done\n")));
#endif
    if (rv < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_scoreboard_thread: Operation failed; exiting\n")));
    }

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_scoreboard_thread: exiting\n")));

    collect->thread_id = SAL_THREAD_ERROR;
    collect->interval  = 0;

    sal_thread_exit(0);
}

/*
 * Function:
 *soc_sbx_scoreboard_start
 * Purpose:
 *     Start the scoreboard collection, software accumulation process.
 * Parameters:
 *     unit     - Device number
 *     flags    - SOC_COUNTER_F_xxx flags
 *     interval - Collection period in micro-seconds,
 *                using 0 is the same as calling soc_sbx_scoreboard_stop()
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_scoreboard_start(int unit, uint32 flags, int interval)
{
    int                   rv = SOC_E_NONE;
    _scoreboard_collector_t  *collect;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "soc_scoreboard_start: unit=%d flags=0x%x interval=%d\n"),
                 unit, flags, interval));

    UNIT_INIT_CHECK(unit);

    SCOREBOARD_LOCK(unit);

    /* Stop if already running */
    rv = soc_sbx_scoreboard_stop(unit);
    if (SOC_FAILURE(rv)) {
        SCOREBOARD_UNLOCK(unit);
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard Stop failure\n")));
        return rv;
    }

    /* Interval of 0 just stops thread */
    if (interval == 0) {
        SCOREBOARD_UNLOCK(unit);
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard Interval 0\n")));
        return SOC_E_NONE;
    }

    collect = &SCOREBOARD_COLLECT(unit);

    /* Create fresh semaphores */
    if (collect->trigger == NULL) {
        collect->trigger = sal_sem_create("scoreboard_trigger",
                                          sal_sem_BINARY, 0);
    }
    if (collect->intr == NULL) {
        collect->intr = sal_sem_create("scoreboard_intr", sal_sem_BINARY, 0);
    }
    if ((collect->trigger == NULL) || (collect->intr == NULL)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_scoreboard_start: sem create failed\n")));
        SCOREBOARD_UNLOCK(unit);
        LOG_CLI((BSL_META_U(unit,
                            "Scoreboard semaphore failure 0\n")));
        return SOC_E_INTERNAL;
    }

    /* Synchronize scoreboard 'prev' values with current hardware scoreboards */
    collect->prev = collect->cur = sal_time_usecs();

    /* Start scoreboard collector thread */
    if (interval != 0) {
        collect->interval = interval;

        sal_snprintf(collect->thread_name, sizeof(collect->thread_name),
                     "bcmScoreboard.%d", unit);

#ifdef SCOREBOARD_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "DEBUG: Start Scoreboard Thread %s. Interval %d \n"),
                 collect->thread_name, collect->interval));
#endif

        collect->thread_id = sal_thread_create(collect->thread_name,
                                               SAL_THREAD_STKSZ,
                                               soc_property_get
                                               (unit,
                                                spn_COUNTER_THREAD_PRI, 50),
                                               _soc_sbx_scoreboard_thread,
                                               INT_TO_PTR(unit));

        if (collect->thread_id == SAL_THREAD_ERROR) {
            collect->interval = 0;
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_scoreboard_start: thread create failed\n")));
            rv = SOC_E_INTERNAL;
        } else {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "soc_scoreboard_start: complete \n")));
#ifdef SCOREBOARD_DEBUG
            LOG_CLI((BSL_META_U(unit,
                                "soc_sbx_scoreboard_start: complete %d \n"),
                     collect->interval ));
#endif
        }
    }

    SCOREBOARD_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *     soc_sbx_scoreboard_stop
 * Purpose:
 *     Terminate the scoreboard collection, software accumulation process.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Returns:
 *     SOC_E_XXX
 */
int
soc_sbx_scoreboard_stop(int unit)
{
    int                   rv = SOC_E_NONE;
    _scoreboard_collector_t  *collect;

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_scoreboard_stop: unit=%d\n"), unit));

#ifdef SCOREBOARD_COUNTER
    LOG_CLI((BSL_META_U(unit,
                        "soc_sbx_scoreboard_stop: unit=%d\n"), unit));
#endif

    UNIT_INIT_CHECK(unit);

    SCOREBOARD_LOCK(unit);

    collect = &SCOREBOARD_COLLECT(unit);

    /* Stop thread if present */
    if (collect->interval != 0) {
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

        collect->interval = 0;

        sal_sem_give(collect->intr);
        sal_sem_give(collect->trigger);

        soc_timeout_init(&to, timeout, 0);

        while ((sample_pid = collect->thread_id) != SAL_THREAD_ERROR) {
            if (soc_timeout_check(&to)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_scoreboard_stop: thread did not exit\n")));
                collect->thread_id = SAL_THREAD_ERROR;
                rv = SOC_E_INTERNAL;
                break;
            }

            sal_usleep(10000);
        }
    }

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_scoreboard_stop: unit=%d rv=%d\n"),
              unit, rv));

    SCOREBOARD_UNLOCK(unit);

    return rv;
}


int
soc_qe2000_pkt_age_get(int unit, int *value)
{
    uint32    AgeVal;

        AgeVal = SAND_HAL_READ(unit, KA, QM_CONFIG3);

        /* Do we need to transform the age? For now just pass */

        LOG_CLI((BSL_META_U(unit,
                            "soc_qe2000_pkt_age_get 0%x\n"), AgeVal));
        *value = AgeVal;

        return BCM_E_NONE;

}

int
soc_qe2000_pkt_age_set(int unit, int value)
{

        uint32    AgeVal;

        /* Do we need to transform the age? For now just pass */

        AgeVal = value;

        LOG_CLI((BSL_META_U(unit,
                            "soc_qe2000_pkt_age_set 0%x\n"), AgeVal));
        SAND_HAL_WRITE(unit, KA, QM_CONFIG3, AgeVal);

        return BCM_E_NONE;
}

int
soc_qe2000_scoreboard_stats_get(int unit, uint32 *pBufLost, uint32 *pBufFrees,
        uint32 *pWatchdogErrs, uint32 *pShortIntervals, uint32 *pScoreboardTicks)

{

    *pBufLost = nTotalLost;
    *pBufFrees = nTotalFreed;
    *pWatchdogErrs = nTotalDogs;
    *pShortIntervals = nTotalDogs;
    LOG_CLI((BSL_META_U(unit,
                        "Scoreboard stats BufErrs 0x%x, Buf Frees 0x%x SOT Watchdogs 0x%x Short Intervals 0x%x Scoreboard Ticks 0x%x\n"),
             nTotalLost, nTotalFreed, nTotalDogs, nShortIntervals, 
             SCOREBOARD_CONTROL(unit)->nScoreboardPolls++)); 

    return BCM_E_NONE;
}
