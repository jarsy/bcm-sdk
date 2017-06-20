/*
 * $Id: rx.c,v 1.99 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        rx.c
 * Purpose:     Receive packet mechanism
 * Requires:
 *
 * See sdk/doc/txrx.txt and pkt.txt for
 * information on the RX API and implementation.
 *
 * Quick overview:
 *
 *     Packet buffer allocation/deallocation is user configurable.
 *     This expects to be given monolithic (single block) buffers.
 *     When "HANDLED_OWNED" is returned by a handler, that means
 *     that the data buffer is stolen, not the packet structure.
 *     When the packet buffer (pkt->pkt_data->data) is stolen, the
 *     handler/handling process is responsible for freeing the buffer
 *     by calling bcm_rx_free; or bcm_rx_free_enqueue.
 *
 *     Callback functions may be registered in interrupt or non-
 *     interrupt mode.  Non-interrupt is preferred.
 *
 *     Interrupt load is limited by setting overall rate limit
 *     (bcm_rx_rate_burst_set/get).
 *
 *     If a packet is not serviced in interrupt mode, it is queued
 *     based on its COS.
 *
 *     Each queue has a rate limit (bcm_rx_cos_rate_set/get) which
 *     controls the number of callbacks that will be made for the queue.
 *     The non-interrupt thread services these queues from highest to
 *     lowest and will discard packets in the queue when they exceed
 *     the queue's rate limit.
 *
 *     Packets handled at interrupt level are still accounted for in
 *     the COS rate limiting.
 *
 *     Channels may be enabled and disabled separately from starting RX
 *     running.  However, stopping RX disables all channels.
 *
 *     Updates to the handler linked list need to be synchronized
 *     both with thread packet processing (mutex) and interrupt
 *     packet processing (spl).
 *
 *     If no real callouts are registered (other than internal discard)
 *     don't bother starting DVs, nor queuing input pkts into cos queues.
 *
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/c3Pp.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#endif /* BCM_CALADAN3_SUPPORT */


#include <bcm/error.h>
#include <bcm/rx.h>
#include <bcm_int/control.h>
#include <bcm_int/common/rx.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/rx.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/error.h>

#define RX_UNIT_VALID_CHECK(unit) \
    if (((unit) < 0) || ((unit) >= BCM_MAX_NUM_UNITS)) { return BCM_E_UNIT; }

#if 0
#define RX_VERY_VERBOSE(stuff)  LOG_VERBOSE(BSL_LS_BCM_RX, stuff)
#else
#define RX_VERY_VERBOSE(stuff)
#endif

/* Forward declarations */
STATIC void         sbx_rx_queue_init(int unit);
STATIC int          sbx_rx_discard_handler_setup(int unit);
STATIC void         sbx_rx_user_cfg_check(int unit);
STATIC void         sbx_rx_pkt_thread(void *param);
STATIC int          sbx_rx_thread_start(int unit);
STATIC void         sbx_rx_cleanup(int unit);
STATIC INLINE void  sbx_rx_cleanup_queue(int unit);
STATIC void         sbx_rx_free_queued(int unit);
STATIC INLINE int   sbx_rx_thread_pkts_process(int unit);
STATIC void         sbx_rx_process_packet(int unit, bcm_pkt_t *pkt);                                         
void                sbx_rx_parse_pkt(int unit, bcm_pkt_t *pkt);
void                sbx_rx_parse_pkt_ext(int unit, bcm_pkt_t *pkt, uint32 erh_type, int fe_unit);
STATIC int          sbx_rx_check_buffer_level(int unit);
#if defined(BROADCOM_DEBUG)
STATIC int          sbx_rx_show_buffer_level(int unit);
#endif  /* BROADCOM_DEBUG */

void                sbx_rx_pkt_intr(int unit, soc_sbx_txrx_active_p_t pDone);

#ifdef BCM_CALADAN3_SUPPORT
STATIC int          sbx_rx_init_g3p1_reason_map(int unit, int fe_unit);
#endif

#define RX_DEFAULT_ALLOC    bcm_rx_pool_alloc
#define RX_DEFAULT_FREE     bcm_rx_pool_free

/* Default data for configuring RX system */
static bcm_rx_cfg_t _rx_dflt_cfg = {
    SBX_RX_PKT_SIZE_DFLT,       /* packet alloc size */
    SBX_RX_PPC_DFLT,            /* Packets per chain */
    SBX_RX_PPS_DFLT,            /* Default pkt rate, global (all COS, one unit) */
    0,                      /* Burst */
    {                       /* Invalid for SBX. For XGS compatibility */
        {0},                
        {                   
            0, 
            0,
            0,
            0
        }
    },
    RX_DEFAULT_ALLOC,       /* alloc function */
    RX_DEFAULT_FREE,        /* free function */
    0                       /* flags */
};

/* Sleep .01 seconds when running. */
#define RUNNING_SLEEP 10000

/* Sleep .5 seconds when not running. */
#define NON_RUNNING_SLEEP 500000

/* Check if given quantity is < cur sleep secs; set to that value if so */
#define BASE_SLEEP_VAL RUNNING_SLEEP

/* Set sleep to base value */
#define INIT_SLEEP    rx_control.sleep_cur = BASE_SLEEP_VAL

/* Lower sleep time if val is < current */
#define SLEEP_MIN_SET(val)                                           \
    (rx_control.sleep_cur = ((val) < rx_control.sleep_cur) ?         \
     (val) : rx_control.sleep_cur)

enum RxThreadState {
  RX_THREAD_STATE_NONE,
  RX_THREAD_STATE_SLEEP,
  RX_THREAD_STATE_PROCESSING,
  RX_THREAD_STATE_CALLBACK
} rx_thread_state = RX_THREAD_STATE_NONE;
const char *rco_name_being_served = NULL;
     
/*
 * Boolean:
 *     Is discard the only callout registered?
 */
#define INTR_CALLOUTS(unit) \
    (rx_ctl[unit]->hndlr_intr_cnt != 0)
#define NON_INTR_CALLOUTS(unit) \
    (rx_ctl[unit]->hndlr_cnt != 0)
#define NO_REAL_CALLOUTS(unit) \
    (rx_ctl[unit]->hndlr_intr_cnt == 0 && rx_ctl[unit]->hndlr_cnt == 0)

#define SBX_RX_PKT(unit, idx)           (rx_ctl[unit]->all_pkts[idx])
#define SBX_RX_QUEUE(unit)              (&(rx_ctl[unit]->pkt_queue[0]))

/* Steal all packets from the queue.  Locking. */
#define SBX_Q_STEAL_ALL(q, first_pkt, qcount)  \
    do {                            \
        _Q_LOCK(q);                 \
        (first_pkt) = (q)->head;    \
        (q)->head = NULL;           \
        (q)->tail = NULL;           \
        (qcount)  = (q)->count;     \
        (q)->count = 0;             \
        _Q_UNLOCK(q);               \
    } while (0)

#ifdef BROADCOM_DEBUG
static void 
_sbx_dump_buffer(char* pBuf, int nBufLen)
{
    int i;
    char printBuf[100];
    char *pPrintLoc;

    pPrintLoc = printBuf;
    for (i=0; i<nBufLen; i++) {
        pPrintLoc += sal_sprintf(pPrintLoc, "%02x", (uint8)pBuf[i]);
        if ((i&3)==3) {
            pPrintLoc += sal_sprintf(pPrintLoc, " ");
        }
        if ((i&31)==31) {
            LOG_INFO(BSL_LS_BCM_RX,
                     (BSL_META("%s\n"), printBuf));
            pPrintLoc = printBuf;
        }
    }

    if((i&31)) {
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META("%s\n"),
                  printBuf));
    } else {
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META("\n")));
    }
}

void
_sbx_dump_bcm_reasons(bcm_rx_reasons_t reasons)
{
    static char* reasonNames[] = _SHR_RX_REASON_NAMES_INITIALIZER;
    int idx;

    for (idx = 0; idx < _SHR_RX_REASON_COUNT; idx++) {
        if (BCM_RX_REASON_GET(reasons, idx)) {
            LOG_INFO(BSL_LS_BCM_RX,
                     (BSL_META(" reason=%s\n"),
                      reasonNames[idx]));
        }
    }

}
#endif /* BROADCOM_DEBUG */

/*
 * Function:
 *      bcm_sbx_rx_sched_register
 * Purpose:
 *      Rx scheduler registration function. 
 * Parameters:
 *      unit       - (IN) Unused. 
 *      sched_cb   - (IN) Rx scheduler routine.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_sched_register(int unit, bcm_rx_sched_cb sched_cb)
{
#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        return (_bcm_common_rx_sched_register(unit, sched_cb));
    }
#endif    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_rx_sched_unregister
 * Purpose:
 *      Rx scheduler de-registration function. 
 * Parameters:
 *      unit  - (IN) Unused. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_sched_unregister(int unit)
{
#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        return (_bcm_common_rx_sched_unregister(unit));
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_rx_queue_max_get
 * Purpose:
 *      Get maximum cos queue number for the device.
 * Parameters:
 *      unit    - (IN) BCM device number. 
 *      cosq    - (OUT) Maximum queue priority.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_queue_max_get(int unit,
                         bcm_cos_queue_t *cosq)
{
    /* This should be the MAX of the highest local RX queue number,
       and the largest value that pkt->prio_int can be set to (for RX
       tunnelling). */
    *cosq = sbx_num_cosq[unit];
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_sbx_rx_queue_packet_count_get
 * Purpose:
 *      Get number of packets awaiting processing in the specific device/queue.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      cosq         - (IN) Queue priority.
 *      packet_count - (OUT) Number of packets awaiting processing. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_queue_packet_count_get(int unit,
                                  bcm_cos_queue_t cosq,
                                  int *packet_count)
{
    RX_UNIT_VALID_CHECK(unit);

    if (!RX_IS_SETUP(unit)) {
        return BCM_E_PARAM;
    }
    *packet_count = rx_ctl[unit]->q_depth;

    return BCM_E_NONE;

}

/*
 * Function:
 *      bcm_sbx_rx_queue_rate_limit_status
 * Purpose:
 *      Get number of packet that can be rx scheduled 
 *      until system hits queue rx rate limit. 
 * Parameters:
 *      unit           - (IN) BCM device number. 
 *      cosq           - (IN) Queue priority.
 *      packet_tokens  - (OUT)Maximum number of packets that can be  scheduled.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_queue_rate_limit_status_get(int unit,
                                       bcm_cos_queue_t cosq, 
                                       int *packet_tokens)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_rx_init/bcm_rx_init
 * Purpose:
 *      Initialize internal data structures for bcm_rx, thread is not started.
 *      Checked and called by most other bcm_rx routines.
 * Parameters:
 *      unit - bcm unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_init(int unit)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_init(unit));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

#ifdef INCLUDE_LIB_AEDEV /* Allow for BME for ATP/RPC support */
    if (!(SOC_IS_SBX_FE(unit) || SOC_IS_SBX_QE(unit) || 
	  SOC_IS_SBX_BME(unit))) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }
#else
    if (!(SOC_IS_SBX_FE(unit) || SOC_IS_SBX_QE(unit))) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }
#endif
    
    if (rx_ctl[unit] != NULL) {
        if (bcm_rx_active(unit)) {
            if ((rv = bcm_rx_stop(unit, NULL)) < 0) {
                LOG_INFO(BSL_LS_BCM_RX,
                         (BSL_META_U(unit,
                                     "Error RX Stop returned %s\n"),
                          bcm_errmsg(rv)));
		BCM_SBX_UNLOCK(unit);
                return rv;
            }
        }
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    rx_ctl[unit] = sal_alloc(sizeof(rx_ctl_t), "rx_ctl");
    if (rx_ctl[unit] == NULL) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(rx_ctl[unit], 0, sizeof(rx_ctl_t));
    sal_memcpy(&rx_ctl[unit]->user_cfg, &_rx_dflt_cfg, sizeof(bcm_rx_cfg_t));
    rx_ctl[unit]->pkt_queue = sal_alloc(sizeof(rx_queue_t)*BCM_RX_COS,"pkt_queue");
    if (rx_ctl[unit]->pkt_queue == NULL) {
        BCM_SBX_UNLOCK(unit);
        sal_free(rx_ctl[unit]);
        return BCM_E_MEMORY;
    }

    sal_memset(rx_ctl[unit]->pkt_queue, 0, sizeof(rx_queue_t)*BCM_RX_COS);

    sbx_rx_queue_init(unit);

    rx_ctl[unit]->rx_mutex = sal_mutex_create("RX_MUTEX");
    if ((rv = sbx_rx_discard_handler_setup(unit)) < 0) {
	BCM_SBX_UNLOCK(unit);
	return rv;
    }

    rx_ctl[unit]->reasonMapInitialized = 0;
    rx_ctl[unit]->rx_thread_pri = soc_property_get(unit, spn_BCM_RX_THREAD_PRI, RX_THREAD_PRI_DFLT);

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "Initialized unit %d\n"),
              unit));

    BCM_SBX_UNLOCK(unit);
    return rv;
}


/*
 * Function:
 *      bcm_sbx_rx_cfg_init/bcm_rx_cfg_init
 * Purpose:
 *      Re-initalize the user level configuration
 * Parameters:
 *      unit - bcm unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_cfg_init(int unit)
{
    int rv;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cfg_init(unit));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (RX_UNIT_STARTED(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_BUSY;
    }
    
    sal_memcpy(&rx_ctl[unit]->user_cfg, &_rx_dflt_cfg, sizeof(bcm_rx_cfg_t));

    /* coverity[overrun-local] */
    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_sbx_rx_start/bcm_rx_start
 * Purpose:
 *      Starts bcm_rx thread for SBX devices, allocates packet descriptors,
 *      packet buffers, and initializes HW to recveive data into buffers.
 * Parameters:
 *      unit - bcm unit number
 *      cfg  - rx config parameters, if null, default is used
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_start(int unit,
                 bcm_rx_cfg_t *cfg)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = _bcm_common_rx_start(unit, cfg);
        BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif


#ifdef INCLUDE_LIB_AEDEV /* Allow for BME for ATP/RPC support */
    if (!(SOC_IS_SBX_FE(unit) || SOC_IS_SBX_QE(unit) || 
	  SOC_IS_SBX_BME(unit))) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }
#else
    if (!(SOC_IS_SBX_FE(unit) || SOC_IS_SBX_QE(unit))) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }
#endif
    
    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (RX_UNIT_STARTED(unit)) {
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META_U(unit,
                             "Error: already started\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_BUSY;
    }

    if (cfg) {  /* Use default if config not specified. */
        if ((cfg->pkt_size == 0) || (cfg->pkts_per_chain == 0)) {
            /* coverity[overrun-local] */
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        if (SOC_IS_SBX_QE2000(unit)) {
            /* coverity[overrun-local] */
			/* coverity[overrun-call : FALSE] */
            rv = sbx_qe2000_rx_config(unit, cfg);
        } else {
            rv = BCM_E_PARAM;
        }

        if (BCM_FAILURE(rv)) {
            /* coverity[overrun-local] */
	    BCM_SBX_UNLOCK(unit);
            return rv;
        }

        sal_memcpy(&rx_ctl[unit]->user_cfg, cfg, sizeof(bcm_rx_cfg_t));
        if (cfg->rx_alloc == NULL) {
            rx_ctl[unit]->user_cfg.rx_alloc = RX_DEFAULT_ALLOC;
        }
        if (cfg->rx_free == NULL) {
            rx_ctl[unit]->user_cfg.rx_free = RX_DEFAULT_FREE;
        }
        sbx_rx_user_cfg_check(unit);
    }

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "Starting rx thread\n")));

    if (!bcm_rx_pool_setup_done()) {
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META_U(unit,
                             "Starting rx pool\n")));
        if ((rv = (bcm_rx_pool_setup(-1, rx_ctl[unit]->user_cfg.pkt_size + sizeof(void *)))) < 0) {
            /* coverity[overrun-local] */
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }
    
#ifdef INCLUDE_LIB_AEDEV
    if (!SOC_IS_SBX_BME(unit)){
#endif    
    /* allocate packet descriptors, packet buffers, 
     * and give packet buffers to HW for RX */
    if ((rv = sbx_rx_check_buffer_level(unit)) < 0) {
        /* coverity[overrun-local] */
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#ifdef INCLUDE_LIB_AEDEV
    }
#endif

    rx_ctl[unit]->pkts_since_start = 0;
    rx_ctl[unit]->pkts_owned = 0;
    
#ifdef INCLUDE_LIB_AEDEV
    if (!SOC_IS_SBX_BME(unit)){
#endif    
    RX_INTR_LOCK;
    if (!rx_control.thread_running) {
        rx_control.thread_running = TRUE;
        RX_INTR_UNLOCK;
        /* Start up the thread */
        if ((rv = sbx_rx_thread_start(unit)) < 0) {
            rx_control.thread_running = FALSE;
            sbx_rx_cleanup(unit);
            /* coverity[overrun-local] */
	    BCM_SBX_UNLOCK(unit);
            return rv;
        }
    } else {
        RX_INTR_UNLOCK;
    }
#ifdef INCLUDE_LIB_AEDEV
    }
#endif    

    
    rx_ctl[unit]->flags |= BCM_RX_F_STARTED;
    
    /* coverity[overrun-local] */
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_sbx_rx_stop/bcm_rx_stop
 * Purpose:
 *      Stop RX for the given unit; saves current configuration
 * Parameters:
 *      unit - The unit to stop
 *      cfg - OUT Configuration copied to this parameter
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This signals the thread to exit.
 */
int
bcm_sbx_rx_stop(int unit, bcm_rx_cfg_t *cfg)
{
    int i;
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_stop(unit, cfg));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "Stopping rx thread\n")));

    if (cfg != NULL) {    /* Save configuration */
        sal_memcpy(cfg, &rx_ctl[unit]->user_cfg, sizeof(bcm_rx_cfg_t));
    }

    /*
     * If no units are active, signal thread to stop and wait to
     * see it exit; simple semaphore
     */
    RX_INTR_LOCK;
    for (i = 0; i < BCM_CONTROL_MAX; i++) {

        if (!RX_IS_SETUP(i) || i == unit) {
            continue;
        }
        if (rx_ctl[i]->flags & BCM_RX_F_STARTED) {
            /* Some other unit is active */
            rx_ctl[unit]->flags &= ~BCM_RX_F_STARTED;
            RX_INTR_UNLOCK;
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_NONE;
        }
    }

    /* Okay, no one else is running.  Kill thread */
    if (rx_control.thread_running) {
        rx_control.thread_exit_complete = FALSE;
        rx_control.thread_running = FALSE;
        RX_INTR_UNLOCK;

        RX_THREAD_NOTIFY(unit);

        for (i = 0; i < 10; i++) {
            if (rx_control.thread_exit_complete) {
                break;
            }
            /* Sleep a bit to allow thread to stop */
            sal_usleep(500000);
        }
        if (!rx_control.thread_exit_complete) {
            LOG_WARN(BSL_LS_BCM_RX,
                     (BSL_META_U(unit,
                                 "Thread %p running after signaled "
                                  "to stop; \n"), (void *)rx_control.rx_tid));
        } else {
            rx_control.rx_tid = NULL;
        }
    } else {
        RX_INTR_UNLOCK;
    }

    rx_ctl[unit]->flags &= ~BCM_RX_F_STARTED;

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_sbx_rx_clear
 * Purpose:
 *      Clear all RX info
 * Returns:
 *      BCM_E_NONE
 */

int
bcm_sbx_rx_clear(int unit)
{
    int rv = BCM_E_NONE;

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
	rv = (_bcm_common_rx_clear(unit));
	return rv;
    }
#endif

    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
        if (RX_IS_SETUP(unit)) {
            bcm_rx_stop(unit, NULL);
            sbx_rx_cleanup(unit);
            sal_free(rx_ctl[unit]);
            rx_ctl[unit] = NULL;
        }
    }

    return rv;
}


/*
 * Function:
 *      bcm_sbx_rx_cfg_get
 * Purpose:
 *      Check if init done; get the current RX configuration
 * Parameters:
 *      unit - device ID
 *      cfg - OUT Configuration copied to this parameter.  May be NULL
 * Returns:
 *      BCM_E_INIT if not running on unit
 *      BCM_E_NONE if running on unit
 *      < 0 BCM_E_XXX error code
 * Notes:
 */

int
bcm_sbx_rx_cfg_get(int unit, bcm_rx_cfg_t *cfg)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cfg_get(unit, cfg));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    /* Copy config */
    if (cfg != NULL) {
        sal_memcpy(cfg, &rx_ctl[unit]->user_cfg, sizeof(bcm_rx_cfg_t));
    }

    rv = (RX_UNIT_STARTED(unit)) ? BCM_E_NONE : BCM_E_INIT;

    /* Coverity gets confused because RX_UNIT_VALID_CHECK limits unit <= 18, but RX_UNIT_STARTED
       checks unit < 128 and somehow coverity continues with the greater limit */
    /* coverity[overrun-local] */
    BCM_SBX_UNLOCK(unit);
    return rv;

}


/*
 * Function:
 *      bcm_sbx_rx_register
 * Purpose:
 *      Register an upper layer driver
 */
int
bcm_sbx_rx_register(int unit, const char *name, bcm_rx_cb_f callback,
                    uint8 priority, void *cookie, uint32 flags)
{
    volatile rx_callout_t      *rco, *list_rco, *prev_rco;
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_register(unit, name, callback, priority, cookie, flags));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (NULL == callback) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "Registering %s on %d, flags 0x%x%s\n"),
              name, unit, flags, flags & BCM_RCO_F_INTR ? "(intr)" : ""));

    /* First check if already registered */
    for (list_rco = rx_ctl[unit]->rc_callout; list_rco;
         list_rco = list_rco->rco_next) {
        if (list_rco->rco_function == callback &&
            list_rco->rco_priority == priority) {
            if (list_rco->rco_flags == flags &&
                list_rco->rco_cookie == cookie) {
		BCM_SBX_UNLOCK(unit);
                return BCM_E_NONE;
            }
            LOG_VERBOSE(BSL_LS_BCM_RX,
                        (BSL_META_U(unit,
                                    "%s registered with diff params\n"),
                         name));
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    }

    if ((rco = sal_alloc(sizeof(*rco), "rx_callout")) == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_MEMORY);
    }
    SETUP_RCO(rco, name, callback, priority, cookie, NULL, flags);

    RX_LOCK(unit);
    RX_INTR_LOCK;

    /*
     * Find correct place to insert handler, this code assumes that
     * the discard handler has been registered on init.  Handlers
     * of the same priority are placed in the list in the order
     * they are registered
     */

    prev_rco = NULL;
    for (list_rco = rx_ctl[unit]->rc_callout; list_rco;
         list_rco = list_rco->rco_next) {
        if (list_rco->rco_priority < priority) {
            break;
        }

        prev_rco = list_rco;
    }

    if (prev_rco) {                     /* Insert after prev_rco */
        rco->rco_next = prev_rco->rco_next;
        prev_rco->rco_next = rco;
    } else {                            /* Insert first */
        rco->rco_next = list_rco;
        rx_ctl[unit]->rc_callout = rco;
    }

    if (flags & BCM_RCO_F_INTR) {
        rx_ctl[unit]->hndlr_intr_cnt++;
    } else {
        rx_ctl[unit]->hndlr_cnt++;
    }
    RX_INTR_UNLOCK;
    RX_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_RX,
                (BSL_META_U(unit,
                            "%s registered %s%s.\n"),
                 name,
                 prev_rco ? "after " : "first",
                 prev_rco ? prev_rco->rco_name : ""));
    
    BCM_SBX_UNLOCK(unit);

    return(BCM_E_NONE);
}


/*
 * Function:
 *      bcm_sbx_rx_unregister
 * Purpose:
 *      Unregister a callback function
 * Parameters:
 *      unit - Unit reference
 *      priority - Priority of registered callback
 *      callback - The function being registered
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Run through linked list looking for match of function and priority
 */

int
bcm_sbx_rx_unregister(int unit, bcm_rx_cb_f callback, uint8 priority)
{
    volatile rx_callout_t *rco;
    volatile rx_callout_t *prev_rco = NULL;
    const char *name;
    uint32 flags;
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_unregister(unit, callback, priority));
	BCM_SBX_UNLOCK(unit);
	return rv;	
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    RX_LOCK(unit);
    RX_INTR_LOCK;
    for (rco = rx_ctl[unit]->rc_callout; rco; rco = rco->rco_next) {
        if (rco->rco_function == callback && rco->rco_priority == priority) {
            break;
        }
        prev_rco = rco;
    }

    if (!rco) {
        RX_INTR_UNLOCK;
        RX_UNLOCK(unit);
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    name = rco->rco_name;
    flags = rco->rco_flags;

    if (!prev_rco) {  /* First elt on list */
        rx_ctl[unit]->rc_callout = rco->rco_next;
    } else {          /* skip current */
        prev_rco->rco_next = rco->rco_next;
    }

    if (flags & BCM_RCO_F_INTR) {
        rx_ctl[unit]->hndlr_intr_cnt--;
    } else {
        rx_ctl[unit]->hndlr_cnt--;
    }
    RX_INTR_UNLOCK;
    RX_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "Unregistered %s on %d\n"),
              name, unit));
    sal_free((void *)rco);

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_sbx_rx_queue_register
 * Purpose:
 *      Register an application callback for the specified CPU queue
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      name - constant character string for debug purposes.
 *      cosq - CPU cos queue
 *      callback - callback function pointer.
 *      priority - priority of handler in list (0 is lowest priority).
 *      cookie - cookie passed to driver when packet arrives.
 *      flags - Register for interrupt or non-interrupt callback
 * Returns:
 *      BCM_E_NONE - callout registered.
 *      BCM_E_MEMORY - memory allocation failed.
 */

int
bcm_sbx_rx_queue_register(int unit,
                          const char *name,
                          bcm_cos_queue_t cosq, 
                          bcm_rx_cb_f callback,
                          uint8 priority,
                          void *cookie, 
                          uint32 flags)
{
#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        return (_bcm_common_rx_queue_register(unit, name, cosq, callback,
                                              priority, cookie, flags));
    }
#endif /* BCM_SBX_CMIC_SUPPORT */


    return BCM_E_UNAVAIL;      
}

/*
 * Function:
 *      bcm_sbx_rx_queue_unregister
 * Purpose:
 *      Unregister a callback function
 * Parameters:
 *      unit - Unit reference
 *      cosq - CPU cos queue
 *      priority - Priority of registered callback
 *      callback - The function being registered
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sbx_rx_queue_unregister(int unit,
                            bcm_cos_queue_t cosq,
                            bcm_rx_cb_f callback,
                            uint8 priority)
{
#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        return (_bcm_common_rx_queue_unregister(unit, cosq, 
                                                callback, priority));
    }
#endif

    return BCM_E_UNAVAIL;
}


int
bcm_sbx_rx_reasons_get(int unit, bcm_rx_reasons_t * reasons)
{
    return BCM_E_UNAVAIL;
}

/*  
 * Function:
 *      bcm_rx_queue_channel_set
 * Purpose:
 *      Assign a RX channel to a cosq 
 * Parameters:
 *      unit - Unit reference
 *      queue_id - CPU cos queue index (0 - (max cosq - 1)) 
 *                                      (Negative for all)
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_queue_channel_set (int unit,
                              bcm_cos_queue_t queue_id, 
                              bcm_rx_chan_t chan_id)
{
    return BCM_E_UNAVAIL;
}

/*  
 * Function:
 *      bcm_rx_queue_channel_get
 * Purpose:
 *      Get the associated rx channel with a given cosq
 * Parameters:
 *      unit - Unit reference
 *      queue_id - CPU cos queue index (0 - (max cosq - 1)) 
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_queue_channel_get(int unit,
                             bcm_cos_queue_t queue_id, 
                             bcm_rx_chan_t *chan_id)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_sbx_rx_active
 * Purpose:
 *      Return boolean as to whether unit is running
 * Parameters:
 *      unit - to check
 * Returns:
 *      Boolean:   TRUE if unit is running.
 * Notes:
 *
 */

int
bcm_sbx_rx_active(int unit)
{
    int rv;

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_active(unit));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    BCM_SBX_UNLOCK(unit);

    rv = ((RX_UNIT_STARTED(unit)) != 0);
    return rv;
}

/*
 * Function:
 *      bcm_sbx_rx_channels_running / bcm_rx_channels_running
 * Purpose:
 *      provided soley for backwards compatibility, no effect on SBX devices
 * Parameters:
 *      
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sbx_rx_channels_running(int unit,
                            uint32 *channels)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_channels_running(unit, channels));
	BCM_SBX_UNLOCK(unit);
        return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_sbx_rx_alloc / bcm_rx_alloc
 * Purpose:
 *      Allocate a buffer for RX using the configured allcator
 * Parameters:
 *      unit      - bcm unit number
 *      pkt_size  - size of packet to allocate
 *      flags     - flags passed to allocator
 *      buf       - (OUT)allocated buffer
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_sbx_rx_alloc(int unit, int pkt_size, uint32 flags, void **buf)
{
    int rv;

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_alloc(unit, pkt_size, flags, buf));
	BCM_SBX_UNLOCK(unit);
        return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
        *buf = NULL;                    
	BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }
    
    if (pkt_size <= 0) {
        pkt_size = rx_ctl[unit]->user_cfg.pkt_size;
    }
    
    rv = rx_ctl[unit]->user_cfg.rx_alloc(unit, pkt_size, flags, buf);

    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_sbx_rx_free / bcm_rx_free
 * Purpose:
 *      Free a buffer from the RX pool using the configured de-allocator
 * Parameters:
 *      unit      - bcm unit number
 *      pkt_data  - buffer to free
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sbx_rx_free(int unit, void *pkt_data)
{
    int rv = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_free(unit, pkt_data));
	BCM_SBX_UNLOCK(unit);
        return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }

    if (pkt_data != NULL) {
        int err;

        err = rx_ctl[unit]->user_cfg.rx_free(unit, pkt_data);

        if (BCM_FAILURE(err)) {
	    BCM_SBX_UNLOCK(unit);
            return err;
        }
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


/*
 * Function:
 *      bcm_rx_free_enqueue
 * Purpose:
 *      Queue a packet to be freed by the RX thread.
 * Parameters:
 *      unit - Unit reference
 *      pkt - Packet to free
 * Returns:
 * Notes:
 *      This may be called in interrupt context to queue
 *      a packet to be freed.
 *
 *      Assumes pkt_data is 32-bit aligned.
 *      Uses the first word of the freed data as a "next" pointer
 *      for the free list.
 */

#define PKT_TO_FREE_NEXT(data) (((void **)data)[0])
int
bcm_sbx_rx_free_enqueue(int unit, void *pkt_data)
{
    int rv = BCM_E_NONE;

    if (pkt_data == NULL) {
        return BCM_E_PARAM;
    }

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_free_enqueue(unit, pkt_data));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit) || rx_control.rx_tid == NULL) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_INIT;
    }

    RX_INTR_LOCK;
    PKT_TO_FREE_NEXT(pkt_data) = (void *)(rx_ctl[unit]->free_list);
    rx_ctl[unit]->free_list = pkt_data;
    RX_INTR_UNLOCK;

    RX_THREAD_NOTIFY(unit);

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_rx_rate_set(int unit,
                    int pps)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_rate_set(unit, pps));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (pps < 0) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    RX_PPS(unit) = pps;

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_rx_rate_get(int unit,
                    int *pps)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_rate_get(unit, pps));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (!pps) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (pps) {
        *pps = RX_PPS(unit);
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;

}

int 
bcm_sbx_rx_cpu_rate_set(int unit,
                        int pps)
{
    int rv = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cpu_rate_set(unit, pps));
	BCM_SBX_UNLOCK(unit);
        return rv;	
    }
#endif

    if (pps < 0) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }
    rx_control.system_pps = pps;

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int 
bcm_sbx_rx_cpu_rate_get(int unit,
                        int *pps)
{
    int rv = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cpu_rate_get(unit, pps));
 	BCM_SBX_UNLOCK(unit);
        return rv;	
    }
#endif

    if (pps) {
        *pps = rx_control.system_pps;
    } 
    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_rx_burst_set(int unit,
                     int burst)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_burst_set(unit, burst));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    RX_BURST(unit) = burst;
    RX_TOKENS(unit) = burst;

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_rx_burst_get(int unit,
                     int *burst)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_burst_get(unit, burst));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (burst) {
        *burst = RX_BURST(unit);
    }
    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

#define LEGAL_COS(cos) ((cos) >= BCM_RX_COS_ALL && (cos) < BCM_RX_COS)

int
bcm_sbx_rx_cos_rate_set(int unit,
                        int cos,
                        int pps)
{
    int rv = BCM_E_UNAVAIL;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cos_rate_set(unit, cos, pps));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!LEGAL_COS(cos) || pps < 0) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_rx_cos_rate_get(int unit,
                        int cos,
                        int *pps)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cos_rate_get(unit, cos, pps));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_rx_cos_burst_set(int unit,
                         int cos,
                         int burst)
{
    int rv = BCM_E_UNAVAIL;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cos_burst_set(unit, cos, burst));
	BCM_SBX_UNLOCK(unit);
        return rv;
    }
#endif

    if (!LEGAL_COS(cos)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_rx_cos_burst_get(int unit,
                         int cos,
                         int *burst)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cos_burst_get(unit, cos, burst));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_rx_cos_max_len_set(int unit,
                           int cos,
                           int max_q_len)
{
    int rv = BCM_E_UNAVAIL;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);


#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cos_max_len_set(unit, cos, max_q_len));
	BCM_SBX_UNLOCK(unit);
	return rv;	
    }
#endif

    if (!LEGAL_COS(cos) || max_q_len < 0) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_rx_cos_max_len_get(int unit,
                           int cos,
                           int *max_q_len)
{
    int rv = BCM_E_NONE;

    RX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        rv = (_bcm_common_rx_cos_max_len_get(unit, cos, max_q_len));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }
#endif

    if (!RX_INIT_DONE(unit)) {
	if ( (rv = bcm_rx_init(unit)) < 0 ) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}


#if defined(BCM_RPC_SUPPORT)

int
bcm_sbx_rx_remote_pkt_enqueue(int unit,
                              bcm_pkt_t *pkt)
{
    int rv = BCM_E_UNAVAIL;

    if (!RX_IS_SETUP(unit) || !RX_IS_REMOTE(unit)) {
        return BCM_E_PARAM;
    }

#ifdef BCM_SIRIUS_SUPPORT
    rv = (_bcm_common_rx_remote_pkt_enqueue(unit, pkt));
#endif

    return rv;
}

int
bcm_sbx_rx_remote_pkt_alloc(int len,
                        bcm_pkt_t **pkt)
{
        return (bcm_rx_remote_pkt_alloc(len, pkt));
}

int
bcm_sbx_rx_remote_pkt_free(bcm_pkt_t *pkt)
{
    if (pkt == NULL) {
        return BCM_E_INTERNAL;
    }

    return (bcm_rx_remote_pkt_free(pkt));
}

#else

int
bcm_sbx_rx_remote_pkt_enqueue(int unit,
                              bcm_pkt_t *pkt)
{
#ifdef BCM_SIRIUS_SUPPORT
    return (_bcm_common_rx_remote_pkt_enqueue(unit, pkt));
#endif
    return BCM_E_UNAVAIL;
}
#endif  /* BCM_RPC_SUPPORT */

#if defined(BROADCOM_DEBUG)

int
bcm_sbx_rx_show(int unit)
{
    volatile rx_callout_t      *rco;
    sal_usecs_t cur_time;

    RX_UNIT_VALID_CHECK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
        return (_bcm_common_rx_show(unit));
    }
#endif

    if (!RX_INIT_DONE(unit)) {
        LOG_CLI(("Initializing RX subsystem (%d)\n",
                 bcm_rx_init(unit)));
    }

    cur_time = sal_time_usecs();
    LOG_CLI(("RX Info @ time=%u: %sstarted. Thread is %srunning. Thread state %d\n"
             "    +verbose for more info\n",
             cur_time,
             RX_UNIT_STARTED(unit) ? "" : "not ",
             rx_control.thread_running ? "" : "not ",
             rx_thread_state));
    if (rx_thread_state == RX_THREAD_STATE_CALLBACK) {
      LOG_CLI(("    %s is being called\n", rco_name_being_served));
    }


    LOG_CLI(("    Pkt Size %d. Pkts/Chain %d. All COS PPS %d. Burst %d\n",
             RX_PKT_SIZE(unit), RX_PPC(unit), RX_PPS(unit), RX_BURST(unit)));
    LOG_CLI(("    Sys PPS %d. Sys tokens %d. Sys fill %u.\n",
             rx_control.system_pps, rx_control.system_tokens,
             rx_control.system_last_fill));
    LOG_CLI(("    Cntrs:  Pkts %d. Last start %d. Tunnel %d. Owned %d.\n"
             "        Bad Hndlr %d. No Hndlr %d. Not Running %d.\n"
             "        Thrd Not Running %d. DCB Errs %d.\n",
             rx_ctl[unit]->tot_pkts,
             rx_ctl[unit]->pkts_since_start,
             rx_ctl[unit]->tunnelled,
             rx_ctl[unit]->pkts_owned,
             rx_ctl[unit]->bad_hndlr_rv,
             rx_ctl[unit]->no_hndlr,
             rx_ctl[unit]->not_running,
             rx_ctl[unit]->thrd_not_running,
             rx_ctl[unit]->dcb_errs));
    LOG_VERBOSE(BSL_LS_BCM_RX,
                (BSL_META_U(unit,
                            "    Tokens %d. Sleep %d.\n"
                             "    Thread: %p. run %d. exitted %d. pri %d.\n"),
                 RX_TOKENS(unit),
                 rx_control.sleep_cur,
                 (void *)rx_control.rx_tid,
                 rx_control.thread_running,
                 rx_control.thread_exit_complete,
                 rx_ctl[unit]->rx_thread_pri));

    LOG_CLI(("  Registered callbacks:\n"));
    /* Display callouts and priority in order */
    for (rco = rx_ctl[unit]->rc_callout; rco; rco = rco->rco_next) {
        LOG_CLI(("        %-10s Priority=%3d%s. "
                 "Argument=0x%x. COS 0x%x%08x.\n",
                 rco->rco_name, (uint32)rco->rco_priority,
                 (rco->rco_flags & BCM_RCO_F_INTR) ? " Interrupt" : "",
                 PTR_TO_INT(rco->rco_cookie),
                 rco->rco_cos[1], rco->rco_cos[0]));
        LOG_CLI(("        %10s Packets handled %u, owned %u.\n", " ",
                 rco->rco_pkts_handled,
                 rco->rco_pkts_owned));
    }

    sbx_rx_show_buffer_level(unit);

    return BCM_E_NONE;
}


#endif  /* BROADCOM_DEBUG */


STATIC int
_sbx_rx_init_buffer_mgt(int unit)
{
    int rv = BCM_E_NONE;
    int i;
    bcm_pkt_t *pkt_block = NULL;
    bcm_pkt_t *pkt = NULL;
    
    if (BCM_SUCCESS(rv)) {
        rx_ctl[unit]->all_pkts = sal_alloc(RX_PPC(unit) * sizeof(bcm_pkt_t*), 
                                           "Rx pkt array");
            
        if (rx_ctl[unit]->all_pkts == NULL) {
            LOG_ERROR(BSL_LS_BCM_RX,
                      (BSL_META_U(unit,
                                  "Failed to alloc packet descriptors\n")));
            rv = BCM_E_MEMORY;
        }
    }

    if (BCM_SUCCESS(rv)) {
        pkt_block = sal_alloc(RX_PPC(unit) * sizeof(bcm_pkt_t), "Rx pkt block");
        if (pkt_block == NULL) {
            LOG_ERROR(BSL_LS_BCM_RX,
                      (BSL_META_U(unit,
                                  "failed to alloc packet descriptor block\n")));
            rv = BCM_E_MEMORY;
        }
    }

    if (BCM_SUCCESS(rv)) {
        rx_ctl[unit]->pkt_load = sal_alloc(RX_PPC(unit) * sizeof(void*), "RxPktLoad");
        if (rx_ctl[unit]->pkt_load == NULL) {
            LOG_ERROR(BSL_LS_BCM_RX,
                      (BSL_META_U(unit,
                                  "failed to alloc rx load block\n")));
            rv = BCM_E_MEMORY;
        }
    }

    if (BCM_SUCCESS(rv)) {
        rx_ctl[unit]->pkt_load_ids = sal_alloc(RX_PPC(unit) * sizeof(int), "RxPktLoadIDs");
        if (rx_ctl[unit]->pkt_load_ids == NULL) {
            LOG_ERROR(BSL_LS_BCM_RX,
                      (BSL_META_U(unit,
                                  "failed to alloc rx load IDs block\n")));
            rv = BCM_E_MEMORY;
        }
    }

    if (BCM_FAILURE(rv)) {
        if (pkt_block) {
            sal_free(pkt_block);
            pkt_block = NULL;
        }
            
        if (rx_ctl[unit]->all_pkts) {
            sal_free(rx_ctl[unit]->all_pkts);
            rx_ctl[unit]->all_pkts = NULL;
        }

        if (rx_ctl[unit]->pkt_load) {
            sal_free(rx_ctl[unit]->pkt_load);
            rx_ctl[unit]->pkt_load = NULL;
        }
            
        if (rx_ctl[unit]->pkt_load_ids) {
            sal_free(rx_ctl[unit]->pkt_load_ids);
            rx_ctl[unit]->pkt_load_ids = NULL;
        }
            
        return rv;
    }

    sal_memset(rx_ctl[unit]->pkt_load, 0, RX_PPC(unit) * sizeof(void*)); 
    sal_memset(rx_ctl[unit]->pkt_load_ids, 0, RX_PPC(unit) * sizeof(int));
    sal_memset(pkt_block, 0, RX_PPC(unit) * sizeof(bcm_pkt_t));

    rx_ctl[unit]->all_pkts[0] = pkt_block; /* add this extra line to shut up coverity */

    for (i=0; i < RX_PPC(unit); i++) {
        pkt = SBX_RX_PKT(unit, i) = pkt_block++;            
        pkt->unit = unit;
        pkt->pkt_data = &pkt->_pkt_data;
        pkt->blk_count = 1;
        pkt->_idx = i;
    }

    return BCM_E_NONE;
}



/*
 * Function:
 *      sbx_rx_check_buffer_level
 * Purpose:
 *      Fill the HW's rx buffer fifo with any and all available
 *      buffers from the buffer pool.  Allocates buffer pool and
 *      buffer tracking data structures if needed.
 * Parameters:
 *      unit - bcm unit number
 * Returns:
 *      BCM_E_XXX
 *
 */

STATIC int
sbx_rx_check_buffer_level(int unit)
{
    int rv = BCM_E_NONE;
    bcm_pkt_t *pkt = NULL;
    int i, load_idx = 0;

    /* allocate packet descriptors, packet buffers, 
     * and give packet buffers to HW for RX */
    if (rx_ctl[unit]->all_pkts == NULL) {

        rv = _sbx_rx_init_buffer_mgt(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    for (i=0; i < RX_PPC(unit); i++) {
        void *pkt_data = NULL;
        pkt = SBX_RX_PKT(unit, i);

        if (pkt->pkt_data->data == NULL) {
            rv = bcm_rx_alloc(unit, rx_ctl[unit]->user_cfg.pkt_size, 
                              0, &pkt_data);

            if (pkt_data == NULL) {
                LOG_WARN(BSL_LS_BCM_RX,
                         (BSL_META_U(unit,
                                     "Failed to allocate pkt buffer\n")));
                
                return BCM_E_NONE;
            }

            pkt->_pkt_data.data = pkt_data;
            pkt->_pkt_data.len  = rx_ctl[unit]->user_cfg.pkt_size;
            pkt->alloc_ptr = pkt_data;

            rx_ctl[unit]->pkt_load_ids[load_idx] = pkt->_idx;
            rx_ctl[unit]->pkt_load[load_idx]     = pkt_data;
            load_idx++;
        }
    }

    if (load_idx) {
        rv = soc_sbx_txrx_give_rx_buffers(unit, load_idx, 
                                          rx_ctl[unit]->pkt_load,
                                          sbx_rx_pkt_intr,
                                          (void**)rx_ctl[unit]->pkt_load_ids);
        
        
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_RX,
                      (BSL_META_U(unit,
                                  "Failed to give soc buffers: %d %s\n"),
                       rv, bcm_errmsg(rv)));
            return rv;
            
        }
    }



    return rv;
}

#if defined(BROADCOM_DEBUG)
STATIC int
sbx_rx_show_buffer_level(int unit)
{
    int rv = BCM_E_NONE;
    bcm_pkt_t *pkt = NULL;
    int i, available = 0;

    if (rx_ctl[unit]->all_pkts == NULL) {
      LOG_CLI(("No RX buffers\n"));
      return rv;
    }

    for (i=0; i < RX_PPC(unit); i++) {
        pkt = SBX_RX_PKT(unit, i);

        if (pkt->pkt_data->data) {
	  available++;
	}
    }

    LOG_CLI(("%d out of %d buffers are available\n", available, RX_PPC(unit)));

    return rv;
}
#endif  /* BROADCOM_DEBUG */

/*
 * Function:
 *      sbx_rx_discard_packet
 * Purpose:
 *      Lowest priority registered handler that discards packet.
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      pkt - The packet to handle
 *      cookie - (Not used)
 * Returns:
 *      bcm_rx_handled
 */

STATIC bcm_rx_t
sbx_rx_discard_packet(int unit, bcm_pkt_t *pkt, void *cookie)
{
    COMPILER_REFERENCE(cookie);
    
    ++(rx_ctl[unit]->dpkt);
    rx_ctl[unit]->dbyte += pkt->tot_len;
    
    return(BCM_RX_HANDLED);
}


STATIC int
sbx_rx_discard_handler_setup(int unit)
{
    volatile rx_callout_t *rco;

    if ((rco = sal_alloc(sizeof(*rco), "rx_callout")) == NULL) {
        return BCM_E_MEMORY;
    }
    SETUP_RCO(rco, "Discard", sbx_rx_discard_packet, BCM_RX_PRIO_MIN, 
              NULL, NULL, 0);
    rx_ctl[unit]->rc_callout = rco;

    return BCM_E_NONE;
}

/*
 * Check the channel configuration passed in by the user for legal values
 */
STATIC void
sbx_rx_user_cfg_check(int unit)
{

    if (RX_PPS(unit) < 0) {
        RX_PPS(unit) = 0;
    }
}

/*
 * Clean up queues on thread exit.
 */
STATIC INLINE void
sbx_rx_cleanup_queue(int unit)
{
    rx_queue_t *queue;

    sbx_rx_free_queued(unit);
    queue = SBX_RX_QUEUE(unit);
    if (!_Q_EMPTY(queue)) {
        queue->count = 0;
        queue->head = NULL;
        queue->tail = NULL;
    }
}


/* 
 * Called on error or thread exit to clean up DMA, etc. 
 */
STATIC void
sbx_rx_cleanup(int unit)
{
    sbx_rx_cleanup_queue(unit);

    if (SOC_UNIT_VALID(unit)) {
	/* remove buffers from PCI Load */
	soc_sbx_txrx_remove_rx_buffers(unit, RX_PPC(unit));
    } else {
	
    }

    /* Packets are allocated in a single contiguous block, packet index 0
     * points to the single block, don't free each individual packet
     */
    if (rx_ctl[unit]->all_pkts) {
        sal_free(SBX_RX_PKT(unit, 0));
        sal_free(rx_ctl[unit]->all_pkts);
        rx_ctl[unit]->all_pkts = NULL;            
    }

    if (rx_ctl[unit]->pkt_load) {
        sal_free(rx_ctl[unit]->pkt_load);
        rx_ctl[unit]->pkt_load = NULL;
    }
            
    if (rx_ctl[unit]->pkt_load_ids) {
        sal_free(rx_ctl[unit]->pkt_load_ids);
        rx_ctl[unit]->pkt_load_ids = NULL;            
    }

    if (rx_ctl[unit]->pkt_queue) {
        sal_free(rx_ctl[unit]->pkt_queue);
    }
    
    bcm_rx_pool_cleanup();
}

/*
 * The non-interrupt thread
 */
STATIC int
sbx_rx_thread_start(int unit)
{
    int priority;
    
    if (!rx_control.pkt_notify) {
        if ((rx_control.pkt_notify =
             sal_sem_create("RX pkt ntfy", sal_sem_BINARY, 0)) == NULL) {
            return BCM_E_MEMORY;
        }

        rx_control.pkt_notify_given = FALSE;
    }

    if (SOC_UNIT_VALID(unit)) {
        priority =  rx_ctl[unit]->rx_thread_pri;
    } else {
        priority = RX_THREAD_PRI_DFLT;
    }

    if ((rx_control.rx_tid = sal_thread_create("bcmRX",
                                               SAL_THREAD_STKSZ,
                                               priority,
                                               sbx_rx_pkt_thread,
                                               NULL)) == NULL) {
        return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      sbx_rx_pkt_thread
 * Purpose:
 *      User level thread that handles packets
 * Parameters:
 *      param    - Unit number for this thread
 * Notes:
 *      This can get awoken for one or more of the following reasons:
 *      1.  Descriptor(s) are done (packets ready to process)
 *      2.  RX stop is called ending processing
 *      3.  Interrupt processing has handled packets 
 *      4.  Periodic check in case memory has freed up.
 *
 */

#define CHECK_THREAD_DONE    \
    if (!rx_control.thread_running) goto thread_done


STATIC void
sbx_rx_pkt_thread(void *param)
{
    int unit;

    COMPILER_REFERENCE(param);

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META("RX: Packet thread starting\n")));

    INIT_SLEEP;

    /* Sleep on sem */
    while (rx_control.thread_running) {

        rx_thread_state = RX_THREAD_STATE_PROCESSING;
        CHECK_THREAD_DONE;
        for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
            if (!RX_UNIT_STARTED(unit)) {
                continue;
            }
            CHECK_THREAD_DONE;

            while (sbx_rx_thread_pkts_process(unit)) {
                /* void *bufs[0]; */

                CHECK_THREAD_DONE;

                /* free pkt data on the free list */
                sbx_rx_free_queued(unit);
                sbx_rx_check_buffer_level(unit);
            }

            /* Free queued packets */
            sbx_rx_free_queued(unit);
            sbx_rx_check_buffer_level(unit);
        }
	rx_thread_state = RX_THREAD_STATE_SLEEP;

        SLEEP_MIN_SET(BASE_SLEEP_VAL);
        RX_VERY_VERBOSE((BSL_META_U(unit,
                                    "Sleeping %d at %u\n"),
                         rx_control.sleep_cur, sal_time_usecs()));
    
        sal_sem_take(rx_control.pkt_notify, rx_control.sleep_cur);
        rx_control.pkt_notify_given = FALSE;

        RX_VERY_VERBOSE((BSL_META_U(unit,
                                    "Woke %u\n"),
                         sal_time_usecs()));
        INIT_SLEEP;
    }

thread_done:
    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
        if (RX_IS_SETUP(unit)) {
            sbx_rx_cleanup(unit);
        }
    }
    rx_control.thread_exit_complete = TRUE;
    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX: Packet thread exitting\n")));

    rx_thread_state = RX_THREAD_STATE_NONE;
    sal_thread_exit(0);
}

/* 
 * Free all buffers listed in pending free list 
 */
STATIC void
sbx_rx_free_queued(int unit)
{
    void *free_list, *next_free;

    /* Steal list of pkts to be freed for unit */
    RX_INTR_LOCK;
    free_list = (bcm_pkt_t *)rx_ctl[unit]->free_list;
    rx_ctl[unit]->free_list = NULL;
    RX_INTR_UNLOCK;


    while (free_list) {
        next_free = PKT_TO_FREE_NEXT(free_list);
        bcm_rx_free(unit, free_list);
        free_list = next_free;
    }
}


/*
 * Function:
 *      sbx_rx_thread_pkts_process
 * Purpose:
 *      Process all pending packets for a particular unit
 * Parameters:
 *      unit - the unit being checked
 * Returns:
 *      TRUE (keep processing) if some pkts were processed
 *      FALSE if queue is empty
 */

STATIC INLINE int
sbx_rx_thread_pkts_process(int unit)
{
    bcm_pkt_t *pkt_list = NULL;
    bcm_pkt_t *next_pkt;
    rx_queue_t *queue;
    int keep_going = TRUE;
    int q_len;

    /*
     * Steal the queue's packets; if rate limiting on, just take as many
     * as can be handled.
     */
    queue = SBX_RX_QUEUE(unit);

    /* Take everything off the queue */
    SBX_Q_STEAL_ALL(queue, pkt_list, q_len);

    if (pkt_list == NULL) {
        return FALSE;   /* Queue is empty. */
    }

    /* since the thread steals all packets and processes the queue completely,
     * rx_ctl[unit]->q_depth should be 0 at this point 
     */
    assert(rx_ctl[unit]->q_depth == 0);
    rx_ctl[unit]->q_depth = q_len;

    /* Process packets */
    while (pkt_list) {
        next_pkt = pkt_list->_next;

        rx_ctl[unit]->q_depth--;

         /* No rate limiting imposed */
        sbx_rx_process_packet(unit, pkt_list);
        pkt_list = next_pkt;

        if (!rx_control.thread_running) { /* Thread is being shut down */
            break;
        }
    }

    return keep_going;
}


/* Initialize the queue */
STATIC void
sbx_rx_queue_init(int unit)
{
    rx_queue_t *queue;

    queue = SBX_RX_QUEUE(unit);
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    queue->max_len = RX_Q_MAX_DFLT;
}


/*
 * Function:
 *      sbx_rx_process_packet
 * Purpose:
 *      Processes a received packet in non-interrupt context
 *      calling out to the handlers until the packet is consumed.
 * Parameters:
 *      unit -  unit number.
 *      rx_pkt - The packet to process
 * Returns:
 *      Nothing.
 * Notes:
 *      Assumes discard handler is registered with lowest priority
 */

STATIC void
sbx_rx_process_packet(int unit, bcm_pkt_t *pkt)
{
    volatile rx_callout_t *rco;
    bcm_rx_t              handler_rc;
    int                   handled = FALSE;

    assert(pkt);

    switch (SOC_SBX_CFG(unit)->erh_type) {
        case SOC_SBX_G2P3_ERH_SIRIUS:
            pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_SIRIUS;
        break;
        case SOC_SBX_G2P3_ERH_QESS:
            pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_QESS;
        break;
        case SOC_SBX_G2P3_ERH_DEFAULT:
        default:
            pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_DEFAULT;
    }
    if (SOC_SBX_CFG(unit)->parse_rx_erh) {
        sal_memcpy(pkt->_sbx_rh, pkt->pkt_data->data, pkt->_sbx_hdr_len);
    } else {
        pkt->pkt_len = pkt->pkt_data->len;
        pkt->_sbx_hdr_len = 0;
    }

#ifdef  BROADCOM_DEBUG
    if (LOG_CHECK(BSL_LS_BCM_PKT | BSL_INFO)) {
        /* Dump the packet info */
        LOG_INFO(BSL_LS_BCM_PKT,
                 (BSL_META_U(unit,
                             "packet in\n")));

        _sbx_dump_buffer((char *) pkt->pkt_data->data, pkt->pkt_data->len);
    }
#endif  /* BROADCOM_DEBUG */

    if (!NON_INTR_CALLOUTS(unit)) {
        /* No one to talk to.  free the packet and return */
        rx_ctl[unit]->no_hndlr++;
        LOG_VERBOSE(BSL_LS_BCM_RX,
                    (BSL_META_U(unit,
                                "No handlers, returning\n")));

        bcm_sbx_rx_free(unit, pkt->pkt_data->data);
        pkt->pkt_data->data = NULL;

        return;
    }

    /* parse the packet outside of the interrupt */
    if (SOC_SBX_CFG(unit)->parse_rx_erh) {
        sbx_rx_parse_pkt(unit, pkt);
    }

    RX_LOCK(unit);  /* Can't modify handler list while doing callbacks */
    /* Loop through registered drivers until packet consumed */
    for (rco = rx_ctl[unit]->rc_callout; rco; rco = rco->rco_next) {

        if (rco->rco_flags & BCM_RCO_F_INTR) { /* Non interrupt context */
            continue;
        }

	rx_thread_state = RX_THREAD_STATE_CALLBACK;
	rco_name_being_served = rco->rco_name;
        handler_rc = rco->rco_function(unit, pkt, rco->rco_cookie);
	rx_thread_state = RX_THREAD_STATE_PROCESSING;

        switch (handler_rc) {
        case BCM_RX_NOT_HANDLED:
            break;                      /* Next callout */
        case BCM_RX_HANDLED:
            handled = TRUE;
            LOG_VERBOSE(BSL_LS_BCM_PKT,
                        (BSL_META_U(unit,
                                    "pkt handled by %s\n"), rco->rco_name));
            rco->rco_pkts_handled++;
            break;
        case BCM_RX_HANDLED_OWNED:
            handled = TRUE;
            pkt->_pkt_data.data = NULL;
            pkt->alloc_ptr = NULL;
            LOG_VERBOSE(BSL_LS_BCM_PKT,
                        (BSL_META_U(unit,
                                    "pkt owned by %s\n"), rco->rco_name));
            rx_ctl[unit]->pkts_owned++;
            rco->rco_pkts_owned++;
            break;
        default:
            LOG_WARN(BSL_LS_BCM_RX,
                     (BSL_META_U(unit,
                                 "Invalid callback return value=%d\n"),
                      handler_rc));
            break;
        }

        if (handled) {
            break;
        }
    }

    RX_UNLOCK(unit);
    
    if (pkt->pkt_data->data) {
        bcm_sbx_rx_free(unit, pkt->pkt_data->data);
        pkt->pkt_data->data = NULL;
    }

    if (!handled) {
        /* Internal error as discard should have handled packet */
        LOG_ERROR(BSL_LS_BCM_RX,
                  (BSL_META_U(unit,
                              "No handler processed packet: Unit %d Port %s\n"),
                   unit, SOC_PORT_NAME(unit, pkt->rx_port)));
    }
}


int
_bcm_to_sbx_reasons(int unit, bcm_rx_reason_t rx_reason, uint32 *sb_reasons,
                    int *reason_count)
{
    /* verify array storage capacity */
    assert(*reason_count > 12);
    *reason_count = 0;

    switch(rx_reason) {
    case bcmRxReasonMartianAddr:
    case bcmRxReasonL3HeaderError:
    case bcmRxReasonL4Error:
        break;
    case bcmRxReasonL2SourceMiss:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
                    BCM_IF_ERROR_RETURN
                       (soc_sbx_g3p1_exc_smac_drop_idx_get(unit, sb_reasons));
                    sb_reasons++;
                    BCM_IF_ERROR_RETURN
                       (soc_sbx_g3p1_exc_smac_unknown_idx_get(unit, sb_reasons));
                    *reason_count = 2;
#endif /* BCM_CALADAN3_SUPPORT */
                }
        break;
    case bcmRxReasonL3SourceMiss:
    case bcmRxReasonUrpfFail:
        break;
    case bcmRxReasonL2MtuFail:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
                (soc_sbx_g3p1_exc_mtu_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }
        break;
    case bcmRxReasonTtl:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
                (soc_sbx_g3p1_exc_ttl_expired_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }
        break;
    case bcmRxReasonBpdu:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
        BCM_IF_ERROR_RETURN
                (soc_sbx_g3p1_exc_l2cp_copy_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }
        break;
    case bcmRxReasonUnknownVlan:
        break;
    case bcmRxReasonCpuLearn:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
                (soc_sbx_g3p1_exc_smac_learn_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;
    case bcmRxReasonIgmp:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_igmp_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_pim_idx_get(unit, sb_reasons));
            *reason_count = 2;
#endif /* BCM_CALADAN3_SUPPORT */
        }
        break;
    case bcmRxReasonIngressFilter:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_rt_copy_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;
    case bcmRxReasonIpMcastMiss:
        break;
    case bcmRxReasonL2Cpu:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
                (soc_sbx_g3p1_exc_dmac_copy_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;
    case bcmRxReasonSamePortBridge:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_mac_hairpin_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }
        break;
    case bcmRxReasonSplitHorizon:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
                (soc_sbx_g3p1_exc_split_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;
    case bcmRxReasonFilterMatch:
    case bcmRxReasonMplsTtl:
        break;
    case bcmRxReasonStp:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_egr_stp_blocked_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_stp_blocked_idx_get(unit, sb_reasons));
            *reason_count = 2;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;
    case bcmRxReasonOAMError:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_mismatch_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_peer_not_found_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_no_endpoint_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_rdi_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_threshold_exceeded_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_copy_to_host_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_param_change_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_internal_error_idx_get(unit, sb_reasons));
            *reason_count = 8;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;
    case bcmRxReasonOAMSlowpath:
        if (SOC_IS_SBX_G3P1(unit)) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_oam_unk_type_idx_get(unit, sb_reasons));
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }
        break;
    case bcmRxReasonOAMLMDM:
        break;

    case bcmRxReasonMplsLabelMiss:
        if (SOC_IS_SBX_G3P1(unit) && SOC_SBX_CFG(unit)->mplstp_ena) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_bad_inner_label_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_bad_outer_label_idx_get(unit, sb_reasons));
            sb_reasons++;
            *reason_count = 2;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;

    case bcmRxReasonMplsError:
        if (SOC_IS_SBX_G3P1(unit) && SOC_SBX_CFG(unit)->mplstp_ena) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_too_many_labels_idx_get(unit, sb_reasons));
            sb_reasons++;
            *reason_count = 1;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;

    case bcmRxReasonControl:
        if (SOC_IS_SBX_G3P1(unit) && SOC_SBX_CFG(unit)->mplstp_ena) {
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_lsp_ping_idx_get(unit, sb_reasons));
            sb_reasons++;
            BCM_IF_ERROR_RETURN
               (soc_sbx_g3p1_exc_dcn_idx_get(unit, sb_reasons));
            sb_reasons++;
            *reason_count = 2;
#endif /* BCM_CALADAN3_SUPPORT */
        }

        break;
        
    default:
        *reason_count = 0;
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}


#ifdef BCM_CALADAN3_SUPPORT
STATIC bcm_rx_reason_t
_sbx_g3p1_to_bcm_rx_reasons(int unit, int fe_unit, uint32 sbx_rx_reason)
{
    int rv, idx;

    if (!SOC_IS_SBX_CALADAN3(fe_unit)) {
        LOG_ERROR(BSL_LS_BCM_RX,
                  (BSL_META_U(unit,
                              "Invalid FE unit: %d\n"),
                   fe_unit));
        return bcmRxReasonInvalid;
    }

    if (rx_ctl[unit]->reasonMapInitialized == FALSE) {
        rv = sbx_rx_init_g3p1_reason_map(unit, fe_unit);
        if( BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_RX,
                      (BSL_META_U(unit,
                                  "Failed to init static reason map\n")));
            return bcmRxReasonInvalid;
        }
    }

    for(idx = 0; idx < SBX_RX_MAX_REASONS; idx++) {
        if(rx_ctl[unit]->reasonMap[idx].exc == sbx_rx_reason) {
            return rx_ctl[unit]->reasonMap[idx].reason;
        }
    }

    LOG_WARN(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "SbxReason not found in map: %d\n"),
              sbx_rx_reason));
    return bcmRxReasonInvalid;

}
#endif /* BCM_CALADAN3_SUPPORT */



bcm_rx_reason_t
_sbx_to_bcm_rx_reasons(int unit, int fe_unit, uint32 sbx_rx_reason)
{
    if (SOC_IS_SBX_G3P1(fe_unit)) {
#ifdef BCM_CALADAN3_SUPPORT
        return (_sbx_g3p1_to_bcm_rx_reasons(unit, fe_unit, sbx_rx_reason));
#endif /* BCM_CALADAN3_SUPPORT */
    }

    return bcmRxReasonInvalid;
}


#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
STATIC int
sbx_rx_init_g3p1_reason_map(int unit, int fe_unit)
{
    int           idx=0;
    int           rv;
    /* add the dynamic exceptions to the map */
#define _ADD_G3P1_D_REASON(sbx_exc, bcmR)   \
    rv = soc_sbx_g3p1_exc_##sbx_exc##_get(fe_unit, &rx_ctl[unit]->reasonMap[idx].exc);  \
    if (BCM_FAILURE(rv)) { \
        LOG_ERROR(BSL_LS_BCM_RX, \
                  (BSL_META_U(unit, \
                              "call to #func failed: %d %s\n"), \
                   rv, bcm_errmsg(rv))); \
        return rv; \
    } \
    rx_ctl[unit]->reasonMap[idx].reason = (bcmR); \
    idx++;

    _ADD_G3P1_D_REASON( smac_learn_idx, bcmRxReasonCpuLearn);
    _ADD_G3P1_D_REASON( smac_unknown_idx, bcmRxReasonL2SourceMiss);
    _ADD_G3P1_D_REASON( l2cp_copy_idx, bcmRxReasonBpdu);
    _ADD_G3P1_D_REASON( rt_copy_idx, bcmRxReasonIngressFilter);
    _ADD_G3P1_D_REASON( stp_blocked_idx,  bcmRxReasonStp);
    _ADD_G3P1_D_REASON( smac_drop_idx, bcmRxReasonL2SourceMiss);
    _ADD_G3P1_D_REASON( mac_hairpin_idx, bcmRxReasonSamePortBridge);
    _ADD_G3P1_D_REASON( dmac_drop_idx, bcmRxReasonL2DestMiss);
    _ADD_G3P1_D_REASON( ttl_expired_idx, bcmRxReasonTtl);
    _ADD_G3P1_D_REASON( egr_stp_blocked_idx, bcmRxReasonStp);
    _ADD_G3P1_D_REASON( split_idx, bcmRxReasonSplitHorizon);
    _ADD_G3P1_D_REASON( mtu_idx, bcmRxReasonL2MtuFail);
    _ADD_G3P1_D_REASON( pim_idx, bcmRxReasonIgmp);
    _ADD_G3P1_D_REASON( igmp_idx, bcmRxReasonIgmp);
    _ADD_G3P1_D_REASON( dmac_copy_idx, bcmRxReasonL2Cpu);
    _ADD_G3P1_D_REASON( oam_mismatch_idx, bcmRxReasonOAMError);
    _ADD_G3P1_D_REASON( oam_peer_not_found_idx, bcmRxReasonOAMError);
    _ADD_G3P1_D_REASON( oam_no_endpoint_idx, bcmRxReasonOAMError);
    _ADD_G3P1_D_REASON( oam_rdi_idx, bcmRxReasonOAMError);
    _ADD_G3P1_D_REASON( oam_threshold_exceeded_idx, bcmRxReasonOAMError);
    _ADD_G3P1_D_REASON( oam_copy_to_host_idx, bcmRxReasonOAMError);
    _ADD_G3P1_D_REASON( oam_unk_type_idx, bcmRxReasonOAMSlowpath);
    _ADD_G3P1_D_REASON( oam_param_change_idx, bcmRxReasonOAMError);
    _ADD_G3P1_D_REASON( oam_internal_error_idx, bcmRxReasonOAMError);

    if(SOC_SBX_CFG(fe_unit)->mplstp_ena) {
        _ADD_G3P1_D_REASON( bad_inner_label_idx, bcmRxReasonMplsLabelMiss);
        _ADD_G3P1_D_REASON( bad_outer_label_idx, bcmRxReasonMplsLabelMiss);
        _ADD_G3P1_D_REASON( lsp_ping_idx, bcmRxReasonControl);
        _ADD_G3P1_D_REASON( dcn_idx, bcmRxReasonControl);
        _ADD_G3P1_D_REASON( too_many_labels_idx, bcmRxReasonMplsError);
    }
#undef _ADD_G3P1_D_REASON


    /* add the static exception ids to the map */
#define _ADD_G3P1_S_REASON(sbx_exc, bcmR)   \
    rx_ctl[unit]->reasonMap[idx].exc = sbx_exc; \
    rx_ctl[unit]->reasonMap[idx].reason = (bcmR); \
    idx++;

    _ADD_G3P1_S_REASON( CALADAN3_UNK_MPLS_LBL_LABEL0, bcmRxReasonMplsLabelMiss);
    _ADD_G3P1_S_REASON( CALADAN3_UNK_MPLS_LBL_LABEL1, bcmRxReasonMplsLabelMiss);
    _ADD_G3P1_S_REASON( CALADAN3_UNK_MPLS_LBL_LABEL2, bcmRxReasonMplsLabelMiss);

    _ADD_G3P1_S_REASON( CALADAN3_ENET_SMAC_EQ_DMAC, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_ENET_SMAC_EQ_DMAC_ZERO, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_ENET_TYPE_BETWEEN_1501_AND_1536, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_ENET_SMAC_EQ_MULTICAST, bcmRxReasonMartianAddr);

    _ADD_G3P1_S_REASON( CALADAN3_IPV4_RUNT_PKT, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_IPV4_OPTIONS, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_CHECKSUM, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_VER, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_RUNT_HDR, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_LEN_ERR, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_PKT_LEN_ERR, bcmRxReasonL3HeaderError);

    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_SA, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_DA, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_SA_EQ_DA, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_SA_OR_DA_IS_LOOPBACK, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS, bcmRxReasonMartianAddr);

    _ADD_G3P1_S_REASON( CALADAN3_IPV4_FRAG_ICMP_PROTOCOL, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_IPV6_RUNT_PKT, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV6_VER, bcmRxReasonL3HeaderError);
    _ADD_G3P1_S_REASON( CALADAN3_IPV6_PKT_LEN_ERR, bcmRxReasonL3HeaderError);


    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV6_SA, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_INV_IPV6_DA, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_IPV6_SA_EQ_DA, bcmRxReasonMartianAddr);
    _ADD_G3P1_S_REASON( CALADAN3_IPV6_SA_OR_DA_LOOPBACK, bcmRxReasonMartianAddr);

    _ADD_G3P1_S_REASON( CALADAN3_TCP_OR_UDP_DP_EQUAL_SP, bcmRxReasonL4Error);
    _ADD_G3P1_S_REASON( CALADAN3_TCP_SQ_EQ_ZERO_AND_FLAG_ZERO, bcmRxReasonL4Error);
    _ADD_G3P1_S_REASON( CALADAN3_TCP_SQ_EQ_ZERO_AND_FIN_URG_PSH_ZERO, bcmRxReasonL4Error);
    _ADD_G3P1_S_REASON( CALADAN3_TCP_SYN_AND_FIN_BOTH_SET, bcmRxReasonL4Error);
    _ADD_G3P1_S_REASON( CALADAN3_L4_TINY_FRAG, bcmRxReasonL4Error);
    _ADD_G3P1_S_REASON( CALADAN3_L4_SYN_SPORT_LT_1024, bcmRxReasonL4Error);

#undef _ADD_G3P1_S_REASON

    assert(idx < SBX_RX_MAX_REASONS);

    /* invalidate the rest */
    for(; idx<SBX_RX_MAX_REASONS; idx++) {
        rx_ctl[unit]->reasonMap[idx].reason = bcmRxReasonInvalid;
    }

    for(idx=0; idx<SBX_RX_MAX_REASONS; idx++) {
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META_U(unit,
                             "%d: sbxReason %d mapped to bcmReason %d\n"),
                  idx,
                  rx_ctl[unit]->reasonMap[idx].exc,
                  rx_ctl[unit]->reasonMap[idx].reason));
    }

    rx_ctl[unit]->reasonMapInitialized = TRUE;

    return BCM_E_NONE;
}
#endif /* BCM_CALADAN3_SUPPORT */

void
sbx_rx_parse_pkt(int unit, bcm_pkt_t *pkt)
{
    uint32 erh_type = 0;

    erh_type = SOC_SBX_CFG(unit)->erh_type;
    sbx_rx_parse_pkt_ext(unit, pkt, erh_type, unit);
}

void
sbx_rx_parse_pkt_ext(int unit, bcm_pkt_t *pkt, uint32 erh_type, int fe_unit)
{
    if ((erh_type == SOC_SBX_G2P3_ERH_SIRIUS) || (erh_type == SOC_SBX_G3P1_ERH_SIRIUS)) {
        /* Exception ERH format:
         *
         *     3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
         *     1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
         *    |  ksop         |a|b|T|M| LenAdj|         reQID + Fcos          |
         *    |         OI or McGroup         |  SID                      |   |
         *    | dp| fcos|           free          | rcos|rdp|s|  TTL or EIDX  |
         *
         *   (a) - ECT
         *   (b) - ECN
         */
        pkt->src_mod    = pkt->_sbx_rh[6];
        pkt->src_port   = (pkt->_sbx_rh[7] >> 2) & 0x3f;

        pkt->rx_unit    = unit;
        pkt->rx_port    = pkt->src_port;
        pkt->rx_cpu_cos = pkt->_sbx_rh[3] & 0x7;
        pkt->rx_reason  = pkt->_sbx_rh[11];

        /* copy VSI from ERH to vlan */
        pkt->vlan       = (*(((uint32 *) pkt->_sbx_rh) + 1) >> 16) & 0xffff;

        if (pkt->rx_port < SBX_MAX_PORTS) {
	    if (SOC_UNIT_VALID(unit) && SOC_IS_SBX_QE2000(unit)) {
                /* When on a QE, we must get the FE associated with this front
                 * panel port to determine the reason code mapping
                 */
                fe_unit = SOC_SBX_FE_FROM_QE(unit, pkt->rx_port);
            }
            
            sal_memset(&pkt->rx_reasons, 0, sizeof(pkt->rx_reasons));

            /* MUST get reason mapping from an FE */
            BCM_RX_REASON_SET
                (pkt->rx_reasons, _sbx_to_bcm_rx_reasons(unit, fe_unit, 
                                                         pkt->rx_reason));
        }

#ifdef BROADCOM_DEBUG
        if (LOG_CHECK(BSL_LS_BCM_PKT | BSL_INFO)) {
            _sbx_dump_bcm_reasons(pkt->rx_reasons);
        }
#endif /* BROADCOM_DEBUG */

        pkt->pkt_data->data += pkt->_sbx_hdr_len;
       
    } else if (erh_type == SOC_SBX_G3P1_ERH_ARAD) {
		 /* Exception ERH/ARAD format:
		 *
		 *     3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
		 *     1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
		 *    |  ksop         |f|addr|reserve| system source port            |
		 *    |t  |m|snoop   |fcos|fdp |   destination data                  |
		 *    |out header index /vlan         |sid                       |   |
		 *    | dp| cos |           0              |rcos|rdp|s|  TTL or EIDX |
		 *
		 */
	    pkt->src_mod = pkt->_sbx_rh[10];
		pkt->src_port = (pkt->_sbx_rh[11] >> 2) & 0x3f;

        pkt->rx_unit    = unit;
        pkt->rx_port    = pkt->src_port;
        pkt->rx_cpu_cos = (pkt->_sbx_rh[14] >> 3) & 0x7;
        pkt->rx_reason  = pkt->_sbx_rh[15];

		if (pkt->rx_port < SBX_MAX_PORTS) {
			sal_memset(&pkt->rx_reasons, 0, sizeof(pkt->rx_reasons));

			/* MUST get reason mapping from an FE */
			BCM_RX_REASON_SET
				(pkt->rx_reasons, _sbx_to_bcm_rx_reasons(unit, fe_unit, 
														 pkt->rx_reason));
		}
		
#ifdef BROADCOM_DEBUG
                if (LOG_CHECK(BSL_LS_BCM_PKT | BSL_INFO)) {
			_sbx_dump_bcm_reasons(pkt->rx_reasons);
		}
#endif /* BROADCOM_DEBUG */

	} else if (erh_type == SOC_SBX_G2P3_ERH_QESS) {
        /* Exception ERH format:
         *
         *     3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
         *     1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
         *    |     QID + Fcos            |FDP|E|T|         Length            |
         *    |LenAdj |  0  |(a)| OI[14:0] or MGGROUP         |M| Hw use      |
         *    |          SID              |DP |FCOS |                         |
         *    |   |RCOS |RDP|S|  TTL or EIDX  |
         *
         *   (a) - OI[16:15]
         *    M  - Multicast bit
         */
        pkt->src_mod    = pkt->_sbx_rh[8];
        pkt->src_port   = ((pkt->_sbx_rh[9]) >> 2) & 0x3f;

        pkt->rx_unit    = unit;
        pkt->rx_port    = pkt->src_port;
        pkt->rx_cpu_cos = (pkt->_sbx_rh[12] >> 1) & 0x7;
        pkt->rx_reason  = pkt->_sbx_rh[13];
        /* copy VSI from ERH to vlan */
        pkt->vlan       = (*(((uint32 *) pkt->_sbx_rh) + 1) >> 8) & 0xffff;

        if (pkt->rx_port < SBX_MAX_PORTS) {
            if (SOC_UNIT_VALID(unit) && SOC_IS_SBX_QE2000(unit)) {
                /* When on a QE, we must get the FE associated with this front
                 * panel port to determine the reason code mapping
                 */
                fe_unit = SOC_SBX_FE_FROM_QE(unit, pkt->rx_port);
            }
            
            sal_memset(&pkt->rx_reasons, 0, sizeof(pkt->rx_reasons));

            /* MUST get reason mapping from an FE */
            BCM_RX_REASON_SET
                (pkt->rx_reasons, _sbx_to_bcm_rx_reasons(unit, fe_unit, 
                                                         pkt->rx_reason));
        }

#ifdef BROADCOM_DEBUG
        if (LOG_CHECK(BSL_LS_BCM_PKT | BSL_INFO)) {
            _sbx_dump_bcm_reasons(pkt->rx_reasons);
        }
#endif /* BROADCOM_DEBUG */

        /* subtract the shim length from the total packet length */
        pkt->pkt_len = ((((pkt->_sbx_rh[2] << 8) | pkt->_sbx_rh[3]) & 0x3FFF) - 6);

        pkt->pkt_data->data += pkt->_sbx_hdr_len;

    } else { /* assume SOC_SBX_G2P3_ERH_DEFAULT */
        /* Exception ERH format:
         *
         *     3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
         *     1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
         *    |     QID + Fcos            |FDP|E|T|         Length            |
         *    |LenAdj |  0  |(a)| OI[14:0] or MGGROUP         |M| Hw use      |
         *    |          SID              |  0    |RCOS |RDP|S|  TTL or EIDX  |
         *
         *   (a) - OI[16:15]
         *    M  - Multicast bit
         */
        unsigned char *shim = (unsigned char *) pkt->_sbx_rh + 8;

        pkt->src_mod    = shim[0];
        pkt->src_port   = ((shim[1]) >> 2) & 0x3f;

#ifdef BROADCOM_DEBUG
        if (LOG_CHECK(BSL_LS_BCM_PKT | BSL_INFO)) {
            LOG_INFO(BSL_LS_BCM_RX,
                     (BSL_META_U(unit,
                                 "shim= %02x %02x %02x %02x\n"), 
                      shim[0], shim[1], shim[2], shim[3]));
        }
#endif

        pkt->rx_unit    = unit;
        pkt->rx_port    = pkt->src_port;
        pkt->rx_cpu_cos = (shim[2] >> 4) & 0x7;
        pkt->rx_reason  = shim[3];
        /* copy VSI from ERH to vlan */
        pkt->vlan       = (*(((uint32 *) pkt->_sbx_rh) + 1) >> 8) & 0xffff;

        if (pkt->rx_port < SBX_MAX_PORTS) {
            if (SOC_UNIT_VALID(unit) && SOC_IS_SBX_QE2000(unit)) {
                /* When on a QE, we must get the FE associated with this front
                 * panel port to determine the reason code mapping
                 */
                fe_unit = SOC_SBX_FE_FROM_QE(unit, pkt->rx_port);
            }
            
            sal_memset(&pkt->rx_reasons, 0, sizeof(pkt->rx_reasons));

            /* MUST get reason mapping from an FE */
            BCM_RX_REASON_SET
                (pkt->rx_reasons, _sbx_to_bcm_rx_reasons(unit, fe_unit, 
                                                         pkt->rx_reason));
        }

#ifdef BROADCOM_DEBUG
        if (LOG_CHECK(BSL_LS_BCM_PKT | BSL_INFO)) {
            _sbx_dump_bcm_reasons(pkt->rx_reasons);
        }
#endif /* BROADCOM_DEBUG */

        /* subtract the shim length from the total packet length */
        pkt->pkt_len = ((((pkt->_sbx_rh[2] << 8) | pkt->_sbx_rh[3]) & 0x3FFF) - 4);

        pkt->pkt_data->data += pkt->_sbx_hdr_len;

    }
}


/*
 * Function:
 *      sbx_rx_pkt_intr
 * Purpose:
 *      Processes a received packet in interrupt context
 *      enqueing it and waking up rx thread
 * Parameters:
 *      unit -  unit number.
 *      rx_pkt - The packet to process
 * Returns:
 *      Nothing.
 */
void
sbx_rx_pkt_intr(int unit, soc_sbx_txrx_active_p_t pDone)
{   
    rx_queue_t *queue;
    bcm_pkt_t *pkt;
    int notify = 0;

    queue = SBX_RX_QUEUE(unit);

    for (;pDone; pDone = pDone->next) {
        pkt = SBX_RX_PKT(unit, (uint32)pDone->cookie);

        pkt->_pkt_data.len = pDone->rxlen;

        if ((queue->max_len > 0) && (queue->count < queue->max_len)) {
            _Q_ENQUEUE(queue, pkt);
            notify = 1;
        } else {
        /* No space in queue. Discard */
            queue->qlen_disc++;
        }
    }
    if (notify) {
        RX_THREAD_NOTIFY(unit);
    }
}

int
bcm_sbx_rx_reasons_policer_set(int unit, bcm_rx_reasons_t rx_reasons,
                           bcm_policer_t polid)
{
    bcm_rx_reason_t reason = bcmRxReasonInvalid;
    int             rv = BCM_E_NONE;

    if (!SOC_IS_SBX_G3P1(unit)) {

        return BCM_E_UNAVAIL;
    }

    /* valid exception policer ids are 0-255 */
    if (polid > 0xff) {
        return BCM_E_PARAM;
    }

    /* coverity [mixed_enums] */
    while (reason < bcmRxReasonCount) {
         if (BCM_RX_REASON_GET(rx_reasons, reason)) {
             uint32       sb_reasons[13];
             int          idx, count = 13;

             BCM_IF_ERROR_RETURN
                (_bcm_to_sbx_reasons(unit, reason, sb_reasons, &count));

             for (idx = 0; idx < count; idx++) {
                 if(SOC_IS_SBX_G3P1(unit)){
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
                     soc_sbx_g3p1_xt_t xt;

                     soc_sbx_g3p1_xt_t_init(&xt);
                     BCM_IF_ERROR_RETURN
                         (soc_sbx_g3p1_xt_get(unit, sb_reasons[idx], &xt));
                     xt.policer = polid;
                     BCM_IF_ERROR_RETURN
                         (soc_sbx_g3p1_xt_set(unit, sb_reasons[idx], &xt));
#endif /* BCM_CALADAN3_SUPPORT */
                 }
             }
         }
         
         reason++;
    }

    return rv;
}
