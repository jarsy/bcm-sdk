/*
 * $Id: rx.c,v 1.99 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        rx.c
 * Purpose:     Receive packet mechanism
 *
 *****************************************************************
 * Requires:
 * 
 * See firmware/orion/doc/txrx.txt and pkt.txt for
 * information on the RX API and implementation.
 *
 * Quick overview:
 *
 *     Packet buffer allocation/deallocation is user configurable.
 *     This expects to be given monolithic (single block) buffers.
 *     When "HANDLED_OWNED" is returned by a handler, that means
 *     that the data buffer is stolen, not the packet structure.
 *
 *     Callback functions may be registered in interrupt or non-
 *     interrupt mode.  Non-interrupt is preferred.
 *
 *     Interrupt load is limitted by setting overall rate limit
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
 *     A channel is:
 *          Physically:  A separate hardware DMA process
 *          Logically:  A collection of COS bundled together.
 *     Rate limitting per channel is no longer supported (replaced
 *     by COS queue rate limitting).
 *
 *     Channels may be enabled and disabled separately from starting RX
 *     running.  However, stopping RX disables all channels.
 *
 *     Packets are started in groups called "chains", each of which
 *     is controlled by a "DV" (DMA-descriptor vector).
 *
 *     Updates to the handler linked list need to be synchronized
 *     both with thread packet processing (mutex) and interrupt
 *     packet processing (spl).
 *
 *     If no real callouts are registered (other than internal discard)
 *     don't bother starting DVs, nor queuing input pkts into cos queues.
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/core/dpc.h>

#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/dma.h>

#include <bcm_int/robo/rx.h>
#include <bcm_int/control.h>
#include <bcm/rx.h>

#define RX_PRINT(stuff)        bsl_printf stuff

#define BCM_RX_CTRL_ACTIVE_UNITS_UPDATE (1 << 0)

/* Indicate to generate the CRC for rx packets */
#define BCM_ROBO_RX_CRC_GEN     0

/* Initial packet count of rx pool */
#define BCM_ROBO_RX_POOL_PKT_CNT_DEFAULT    -1

/* Single RX control structure, common for all units */
static struct {
    /* Thread control */
    sal_sem_t         pkt_notify;       /* Semaphore for pkt thrd */
    volatile int      pkt_notify_given; /* Semaphore already given */
    sal_thread_t      rx_tid;           /* packet thread id */
    int               sleep_cur;        /* Current sleep time */
#define MIN_SLEEP_US 5000               /* Always sleep at least 5 ms */
#define MAX_SLEEP_US 100000             /* For non-rate limiting, .1 sec */
    volatile int      thread_running;   /* Input signal to thread */
    volatile int      thread_exit_complete; /* Output signal from thread */

    /* System wide rate limiting */
    int               system_pps;
    int               system_tokens;
    sal_usecs_t       system_last_fill;

    /* Thread protection mutexes. */
    sal_mutex_t       system_lock;  

    /* RX started units misc info. */
    int               rx_unit_first;
    bcm_cos_queue_t   system_cosq_max;

    /* System flags. */ 
    int               system_flags;

    /* Scheduler function pointer. */
    bcm_rx_sched_cb   rx_sched_cb;
} robo_rx_control;

STATIC int
_bcm_robo_rx_default_scheduler(int unit, int *sched_unit, 
                          bcm_cos_queue_t *sched_cosq, int *sched_count);

#define _BCM_ROBO_RX_SYSTEM_LOCK                                                \
          sal_mutex_take(robo_rx_control.system_lock, sal_mutex_FOREVER)

#define _BCM_ROBO_RX_SYSTEM_UNLOCK sal_mutex_give(robo_rx_control.system_lock)

#define _BCM_ROBO_RX_SCHED_DEFAULT_CB (_bcm_robo_rx_default_scheduler)
#define _BCM_ROBO_RX_CHECK_THREAD_DONE  \
        if (!robo_rx_control.thread_running) break 

#define ROBO_RX_UNIT_STARTED(unit) \
    (RX_IS_SETUP(unit) && (rx_units_started & (1 << (unit & 0x1f))))

/*
 * give semaphor.  Notification is given:
 *      When a packet is marked as processed.
 *      When the thread is stopped
 *      When a channel is enabled
 *      When a packet is queued to be serviced
 *      When a packet is queued to be freed
 */
#define ROBO_RX_THREAD_NOTIFY(unit) {                                        \
    if (!robo_rx_control.pkt_notify_given) {                                 \
        robo_rx_control.pkt_notify_given = TRUE;                             \
        sal_sem_give(robo_rx_control.pkt_notify);                            \
    }                                                                   \
} 

#define _Q_DEC_TOKENS(q) if ((q)->pps) ((q)->tokens)--;

/*
 * This should be called each time a packet is handled, owned, etc.
 * It counts when all packets for a DV are processed; it
 * changes the DV state and notifies the thread at that point.
 */
#define ROBO_MARK_PKT_PROCESSED(unit, chan, dv)                         \
    do {(void)chan;                                                          \
        if (++ROBO_DV_PKTS_PROCESSED(dv) == RX_PPC(unit)) {                  \
            ROBO_DV_STATE(dv) = DV_S_NEEDS_FILL;                             \
            ROBO_RX_THREAD_NOTIFY(unit);                                     \
        }                                                               \
    } while (0)

/* Sleep .5 seconds when not running, .1 seconds when running */
#define NON_RUNNING_SLEEP 500000

static int _rx_chan_run_count;

int bcm_robo_rx_token_check_us = BCM_RX_TOKEN_CHECK_US_DEFAULT;

/* Check if given quantity is < cur sleep secs; set to that value if so */
#define BASE_SLEEP_VAL                                            \
    ((_rx_chan_run_count > 0) ? bcm_robo_rx_token_check_us : NON_RUNNING_SLEEP)

/* Set sleep to base value */
#define INIT_SLEEP robo_rx_control.sleep_cur = BASE_SLEEP_VAL

/* Lower sleep time if val is < current */
#define SLEEP_MIN_SET(val)                                        \
    (robo_rx_control.sleep_cur = ((val) < robo_rx_control.sleep_cur) ?         \
     (val) : robo_rx_control.sleep_cur)

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

/* Number of packets allocated for a channel */
#define RX_CHAN_PKT_COUNT(unit, chan) (RX_PPC(unit) * RX_CHAINS(unit, chan))

/* The main control structure for RX subsystem.  See bcm_int/rx.h */
static uint32      rx_units_started;
static uint32      rx_units_rxsusped = 0;


static int         rx_dv_count;     /* Debug: count number of DVs used */

/* Forward declarations */
STATIC void        robo_rx_pkt_thread(void *param);
STATIC int         robo_rx_thread_start(int unit);
STATIC void        robo_rx_process_packet(int unit, bcm_pkt_t *pkt);
STATIC void        robo_rx_intr_process_packet(int unit, eth_dv_t *dv, eth_dcb_t *dcb,
                                  bcm_pkt_t *pkt);
STATIC bcm_rx_t    robo_rx_discard_packet(int unit, bcm_pkt_t *pkt, void *cookie);

STATIC int         robo_rx_channel_dv_setup(int unit, int chan);
STATIC void        robo_rx_channel_shutdown(int unit, int chan);
STATIC void        robo_rx_user_cfg_check(int unit);
STATIC void        robo_rx_dcb_per_pkt(int unit);
STATIC int         robo_rx_discard_handler_setup(int unit, rx_ctl_t *rx_ctrl_ptr);

STATIC void        robo_rx_dv_fill(int unit, int chan, eth_dv_t *dv);
STATIC eth_dv_t   *robo_rx_dv_alloc(int unit, int chan, int dv_idx);
STATIC void        robo_rx_dv_dealloc(int unit, int chan, int dv_idx);
STATIC void        robo_rx_queue_init(int unit, rx_ctl_t *rx_ctrl_ptr);

STATIC void        robo_rx_init_all_tokens(int unit);
STATIC void        robo_rx_cleanup(int unit);

#define RX_DEFAULT_ALLOC    bcm_rx_pool_alloc
#define RX_DEFAULT_FREE     bcm_rx_pool_free

/* Default data for configuring RX system */
static bcm_rx_cfg_t _rx_dflt_cfg = {
    ROBO_RX_PKT_SIZE_DFLT,       /* packet alloc size */
    ROBO_RX_PPC_DFLT,            /* Packets per chain */
    0,                      /* Default pkt rate, global */
    1,                      /* Burst */
    {                       /* Just configure channel 1 */
        {                   /* Only Channel 0, default RX */
            ROBO_RX_CHAINS_DFLT, /* DV count (number of chains) */
            0,              /* Default pkt rate, DEPRECATED */
            0,              /* No flags */
            0xff            /* All COS to this channel */
        }
    },
    RX_DEFAULT_ALLOC,          /* alloc function */
    RX_DEFAULT_FREE,           /* free function */
    0                       /* flags */
};

/* CPU cosq and reason code mapping */
#define _bcm_robo_rx_cpu_cosmap_key_max    4 
#define _bcm_robo_tb_rx_cpu_cosmap_key_max 14 

static bcm_rx_reason_t 
_bcm_robo_cpu_cos_map_key [] =
{
    bcmRxReasonCpuLearn,       /* SA Learning */
    bcmRxReasonControl,        /* Protocol Termination */
    bcmRxReasonProtocol,       /* Protocol Snooping */
    bcmRxReasonExceptionFlood  /* Exception Flood */
};

static bcm_rx_reason_t 
_bcm_robo_tb_cpu_cos_map_key [] =
{
    bcmRxReasonL2Cpu,                /* Known-DA forwarging */
    bcmRxReasonL2DestMiss,           /* Unknown-DA flooding */
    bcmRxReasonControl,              /* 802.1 protocol trapping */
    bcmRxReasonProtocol,             /* Application protocol Snooping */
    bcmRxReasonEgressFilterRedirect, /* Vlan based direct forwarding */
    bcmRxReasonFilterRedirect,       /* CFP based direct forwarding */
    bcmRxReasonLoopback,             /* Indicated loopback */
    bcmRxReasonSampleSource,         /* Ingress SFlow */
    bcmRxReasonSampleDest,           /* Egress SFlow */
    bcmRxReasonL2Move,               /* SA movement */
    bcmRxReasonL2SourceMiss,         /* SA unknown*/
    bcmRxReasonL2LearnLimit,         /* SA overlimit */
    bcmRxReasonIngressFilter,        /* Vlan member violation*/
    bcmRxReasonUnknownVlan           /* Vlan unregistered */
};


/****************************************************************
 *
 * User visible API routines:
 *     bcm_rx_init                 Init,
 *     bcm_rx_start                     Setup,
 *     bcm_rx_stop                          teardown,
 *     bcm_rx_cfg_get                            get configuration
 *     bcm_rx_cfg_init             Reinitial user configuration
 *     bcm_rx_register             Register/unregister handlers
 *     bcm_rx_unregister
 *     bcm_rx_channels_running
 *     bcm_rx_alloc                Gateways to packet alloc/free a buffer
 *     bcm_rx_free
 *     bcm_rx_rate_get/set         Get/set interrupt load rate limits
 *     bcm_rx_cos_rate_get/set     Get/set COS rate limits
 *     bcm_rx_cos_burst_get/set    Get/set burst rate limits
 *     bcm_rx_cos_max_len_get/set  Get/set max q len limits
 * Deprecated functions
 *     bcm_rx[_chan]_rate_get/set  Get/set rate control
 *     bcm_rx_cos_wt_get/set       Get/set COS weights for dequeuing
 *     bcm_rx_channels_enable      Start/stop channels running, get
 *     bcm_rx_channels_disable
 *
 * Since we demand doing an RX stop to change some parts of the
 * configuration (like pkts/chain setting), the DVs must be
 * aborted and deallocated on stop.
 *
 ****************************************************************/
/* 
 * Function:
 *      _bcm_robo_rx_ctrl_lock
 * Purpose:
 *      Lock rx control structures for all units.
 * Parameters: 
 *      None.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_robo_rx_ctrl_lock(void) 
{
    int unit;          /* Unit iterator. */

    _BCM_ROBO_RX_SYSTEM_LOCK;

    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
        /* Skip rx uninitialized units. */
        if (!RX_IS_SETUP(unit)) {
            continue;
        }
        RX_LOCK(unit);
    }
    return (BCM_E_NONE);
}

/* 
 * Function:
 *      _bcm_robo_rx_ctrl_unlock
 * Purpose:
 *      Unlock rx control structures for all units.
 * Parameters: 
 *      None.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_robo_rx_ctrl_unlock(void) 
{
    int unit;          /* Unit iterator. */

    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
        /* Skip rx uninitialized units. */
        if (!RX_IS_SETUP(unit)) {
            continue;
        }
        RX_UNLOCK(unit);
    }

    _BCM_ROBO_RX_SYSTEM_UNLOCK;
        
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_robo_rx_unit_list_update
 * Purpose:
 *      Update list of units with rx started.
 * Parameters: 
 *      None.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_robo_rx_unit_list_update(void) 
{
    int unit;              /* Unit iterator.                  */
    int prev_unit;         /* Last unit with rx initialized.  */
    int rv = BCM_E_NONE;   /* Operation return status.        */

    /* Lock all units rx control structure. */ 
    _bcm_robo_rx_ctrl_lock();

    /* Reset first unit in the list & max cos queue number. */
    robo_rx_control.rx_unit_first = prev_unit = -1;
    robo_rx_control.system_cosq_max = -1;

    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
        /* Reset next unit on initialized units. */
        if (RX_IS_SETUP(unit)) {
            rx_ctl[unit]->next_unit = -1;
        }

        /* Skip rx not started units. */
        if (!ROBO_RX_UNIT_STARTED(unit)) {
            continue;
        }

        /* Fill rx started units into a linked list .*/
        if (-1 == prev_unit) {
            robo_rx_control.rx_unit_first = unit;
        } else {
            rx_ctl[prev_unit]->next_unit = unit;
        }
        prev_unit = unit;
        rx_ctl[unit]->next_unit = -1;

        if (RX_QUEUE_MAX(unit) > robo_rx_control.system_cosq_max) {
            robo_rx_control.system_cosq_max = RX_QUEUE_MAX(unit);
        }
    }

    /* Unlock all units rx control structure. */ 
    _bcm_robo_rx_ctrl_unlock(); 
    return (rv);
}

/*
 * Function:
 *      bcm_robo_rx_sched_register
 * Purpose:
 *      Rx scheduler registration function. 
 * Parameters:
 *      unit       - (IN) Unused. 
 *      sched_cb   - (IN) Rx scheduler routine.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_sched_register(int unit, bcm_rx_sched_cb sched_cb)
{
    /* Input parameters check. */
    if (NULL == sched_cb) {
        return (BCM_E_PARAM);
    }

    _bcm_robo_rx_ctrl_lock(); 

    robo_rx_control.rx_sched_cb = sched_cb; 

    _bcm_robo_rx_ctrl_unlock(); 

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_rx_sched_unregister
 * Purpose:
 *      Rx scheduler de-registration function. 
 * Parameters:
 *      unit  - (IN) Unused. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_sched_unregister(int unit)
{
    _bcm_robo_rx_ctrl_lock(); 

    robo_rx_control.rx_sched_cb = _BCM_ROBO_RX_SCHED_DEFAULT_CB;

    _bcm_robo_rx_ctrl_unlock(); 
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_rx_unit_next_get
 * Purpose:
 *      Rx started units iteration routine.
 * Parameters:
 *      unit       - (IN)  BCM device number. 
 *      unit_next  - (OUT) Next attached unit with started rx.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_unit_next_get(int unit, int *unit_next)
{
    /* Input parameters check. */
    if (NULL == unit_next) {
        return (BCM_E_PARAM);
    }

    if (!RX_IS_SETUP(unit)) {
        *unit_next = -1;
    } else {
        RX_LOCK(unit);
        if (!ROBO_RX_UNIT_STARTED(unit)) {
            *unit_next = -1;
        } else {
            *unit_next = rx_ctl[unit]->next_unit;
        }
        RX_UNLOCK(unit);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_rx_queue_max_get
 * Purpose:
 *      Get maximum cos queue number for the device.
 * Parameters:
 *      unit       - (IN) BCM device number. 
 *      queue_max  - (IN) Maximum queue priority.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_queue_max_get(int unit, bcm_cos_queue_t *cosq)
{
    /* Input parameters check. */
    if (NULL == cosq) {
        return (BCM_E_PARAM);
    }

    *cosq = NUM_CPU_COSQ(unit) - 1;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_rx_queue_packet_count_get
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
bcm_robo_rx_queue_packet_count_get(int unit, bcm_cos_queue_t cosq, int *packet_count)
{
    /* Input parameters check. */
    if (NULL == packet_count) {
        return (BCM_E_PARAM);
    }

    if (0 == RX_IS_SETUP(unit)) {
        return (BCM_E_INIT);
    }

    if (cosq > RX_QUEUE_MAX(unit)) {
        return (BCM_E_PARAM);
    }

    RX_LOCK(unit);
    if (ROBO_RX_UNIT_STARTED(unit)) {
        *packet_count = RX_QUEUE(unit, cosq)->count;
    } else {
        *packet_count = 0;
    }   
    RX_UNLOCK(unit);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_robo_rx_queue_rate_limit_status
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
bcm_robo_rx_queue_rate_limit_status_get(int unit, bcm_cos_queue_t cosq, 
                                       int *packet_tokens)
{
    /* Input parameters check. */
    if (NULL == packet_tokens) {
        return (BCM_E_PARAM);
    }

    if (0 == RX_IS_SETUP(unit)) {
        return (BCM_E_INIT);
    }

    if (cosq > RX_QUEUE_MAX(unit)) {
        return (BCM_E_PARAM);
    }

    RX_LOCK(unit);
    if (ROBO_RX_UNIT_STARTED(unit)) {
        if (RX_QUEUE(unit, cosq)->pps > 0) {
            *packet_tokens = RX_QUEUE(unit, cosq)->tokens;
        } else {
            *packet_tokens = BCM_RX_SCHED_ALL_PACKETS;
        }
    } else {
        /* Don't schedule anything for this device. */ 
        *packet_tokens = 0;  
    }
    RX_UNLOCK(unit);
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_rx_control_get(int unit, bcm_rx_control_t type, int *value)
 * Description:
 *      Get the status of specified RX feature.
 * Parameters:
 *      unit - Device number
 *      type - RX control parameter
 *      value - (OUT) Current value of control parameter
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 */

int
bcm_robo_rx_control_get(int unit, bcm_rx_control_t type, int *value)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(type);
    COMPILER_REFERENCE(value);

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rx_control_set(int unit, bcm_rx_control_t type, int value)
 * Description:
 *      Enable/Disable specified RX feature.
 * Parameters:
 *      unit - Device number
 *      type - RX control parameter
 *      value - new value of control parameter
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 *
 */

int
bcm_robo_rx_control_set(int unit, bcm_rx_control_t type, int value)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(type);
    COMPILER_REFERENCE(value);

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rx_init
 * Purpose:
 *      Software initialization for RX API
 * Parameters:
 *      unit - Unit to init
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Allocates rx control structure
 *      Copies default config into active config
 *      Adds discard handler
 */
int
bcm_robo_rx_init(int unit)
{
    int rv;
    rx_ctl_t *rx_ctrl_ptr;        
    
    if (!bcm_unit_valid(unit)) {
        return BCM_E_PARAM;
    }

    /*
     * Reset dma to initial state. 
     * To make sure all queued packets(before calling rx bcm init) can be 
     * properly removed.
     */
    soc_eth_dma_reinit(unit);
    
    if (rx_ctl[unit] != NULL) {
        return BCM_E_NONE;
    }

    rx_ctrl_ptr = sal_alloc(sizeof(rx_ctl_t), "rx_ctl");
    if (rx_ctrl_ptr == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(rx_ctrl_ptr, 0, sizeof(rx_ctl_t));
    sal_memcpy(&rx_ctrl_ptr->user_cfg, &_rx_dflt_cfg, sizeof(bcm_rx_cfg_t));

    rv = bcm_rx_queue_max_get(unit, &rx_ctrl_ptr->queue_max);
    if (BCM_FAILURE(rv)) {
        sal_free(rx_ctrl_ptr);
        return (rv);
    }

    rx_ctrl_ptr->pkt_queue = 
        sal_alloc(sizeof(rx_queue_t) * NUM_CPU_COSQ(unit), 
            "pkt_queue");
    if (rx_ctrl_ptr->pkt_queue == NULL) {
        sal_free(rx_ctrl_ptr);
        return BCM_E_MEMORY;
    }

    sal_memset(rx_ctrl_ptr->pkt_queue, 0,
               sizeof(rx_queue_t) * NUM_CPU_COSQ(unit));

    robo_rx_queue_init(unit, rx_ctrl_ptr);

    rx_ctrl_ptr->rx_mutex = sal_mutex_create("RX_MUTEX");
    rv = robo_rx_discard_handler_setup(unit, rx_ctrl_ptr);
    if (BCM_FAILURE(rv)) {
        sal_mutex_destroy(rx_ctrl_ptr->rx_mutex);
        sal_free(rx_ctrl_ptr->pkt_queue);
        sal_free(rx_ctrl_ptr);
        return (rv);
    }

    if (!bcm_rx_pool_setup_done()) {
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META_U(unit,
                             "RX: Starting rx pool\n")));
        rv = (bcm_rx_pool_setup(BCM_ROBO_RX_POOL_PKT_CNT_DEFAULT,
                  rx_ctrl_ptr->user_cfg.pkt_size + sizeof(void *)));
        if (BCM_FAILURE(rv)) {
            sal_free((void *)rx_ctrl_ptr->rc_callout);
            sal_mutex_destroy(rx_ctrl_ptr->rx_mutex);
            sal_free(rx_ctrl_ptr->pkt_queue);
            sal_free(rx_ctrl_ptr);
            return (rv);
        }
    }

    if (NULL == robo_rx_control.system_lock) {                            
        robo_rx_control.system_lock = sal_mutex_create("RX system lock"); 
        if (NULL == robo_rx_control.system_lock) {
            bcm_rx_pool_cleanup(); 
            sal_free((void *)rx_ctrl_ptr->rc_callout);
            sal_mutex_destroy(rx_ctrl_ptr->rx_mutex);
            sal_free(rx_ctrl_ptr->pkt_queue);
            sal_free(rx_ctrl_ptr);
            return (BCM_E_MEMORY);
        }
    }    
    
    rx_ctrl_ptr->next_unit = -1;
    _BCM_ROBO_RX_SYSTEM_LOCK;
    rx_ctl[unit] = rx_ctrl_ptr; 
    _BCM_ROBO_RX_SYSTEM_UNLOCK;
    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX: Initialized unit %d\n"), unit));

    return BCM_E_NONE;
}               

int
bcm_robo_rx_deinit(int unit)
{
    return bcm_rx_clear(unit);    
}               

/*
 * Function:
 *      bcm_rx_cfg_init
 * Purpose:
 *      Re-initialize the user level configuration
 * Parameters:
 *      unit - RoboSwitch unit number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Can't use if currently running.  Should be called before
 *      doing a simple modification of the RX configuration in case
 *      the previous user has left it in a strange state.
 */

int
bcm_robo_rx_cfg_init(int unit)
{
    RX_INIT_CHECK(unit);

    if (rx_units_started & (1 << unit)) {
        return BCM_E_BUSY;
    }

    sal_memcpy(&rx_ctl[unit]->user_cfg, &_rx_dflt_cfg, sizeof(bcm_rx_cfg_t));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_rx_start
 * Purpose:
 *      Initialize and configure the RX subsystem for a given unit
 * Parameters:
 *      unit - Unit to configure
 *      cfg - Configuration to use.  See include/bcm/rx.h
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Starts a packet receive thread
 *      cfg may be null:  Use default config.
 *      alloc/free in cfg may be null:  Use default alloc/free functions
 */

int
bcm_robo_rx_start(int unit, bcm_rx_cfg_t *cfg)
{
    int rv = BCM_E_NONE;
    int chan;
    int test_unit;

    RX_INIT_CHECK(unit);

    if (rx_units_started & (1 << unit)) {
        if (rx_units_rxsusped) {
            LOG_INFO(BSL_LS_BCM_RX,
                     (BSL_META_U(unit,
                                 "RxMon start\n")));
            soc_eth_dma_rxenable(unit);
            rx_units_rxsusped = 0;
            return BCM_E_NONE;
        } else {
            LOG_INFO(BSL_LS_BCM_RX,
                     (BSL_META_U(unit,
                                 "RX start:  Already started\n")));
            return BCM_E_BUSY;
        }
    }

    if (cfg) {  /* Use default if config not specified. */
#if defined (IPROC_CMICD) 
#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
        if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
            bcm_rx_chan_cfg_t chan_cfg;
            int chan;
    
            /* rx channel 0 should be configured */
            chan = 0;
            chan_cfg = cfg->chan_cfg[chan];
            if (!chan_cfg.chains) {
                return BCM_E_CONFIG;
            }
            /* rx channel 1,2,3 are not supported */
            for (chan = 1; chan < BCM_RX_CHANNELS; chan++) {
                chan_cfg = cfg->chan_cfg[chan];
                if (chan_cfg.chains) {
                    return BCM_E_CONFIG;
                }
            }
        }
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */		
#endif /* IPROC_CMICD */

        if ((cfg->pkt_size == 0) || (cfg->pkts_per_chain == 0)) {
            return BCM_E_PARAM;
        }
        sal_memcpy(&rx_ctl[unit]->user_cfg, cfg, sizeof(bcm_rx_cfg_t));
        if (!cfg->rx_alloc) {
            rx_ctl[unit]->user_cfg.rx_alloc = RX_DEFAULT_ALLOC;
        }
        if (!cfg->rx_free) {
            rx_ctl[unit]->user_cfg.rx_free = RX_DEFAULT_FREE;
        }
        robo_rx_user_cfg_check(unit);
    }

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX: Starting unit %d\n"), unit));

    robo_rx_dcb_per_pkt(unit);
    robo_rx_init_all_tokens(unit);

    rx_ctl[unit]->pkts_since_start = 0;
    rx_ctl[unit]->pkts_owned = 0;

    /* Set up each channel */
    FOREACH_SETUP_CHANNEL(unit, chan) {
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META_U(unit,
                             "RX: Starting unit %d chan %d\n"), unit, chan));

        soc_eth_dma_occupancy_get(chan, &test_unit);
        if ((test_unit >= 0) && (test_unit != unit)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "RX: DMA %d already be used by unit %d\n"),
                       chan, test_unit));
            robo_rx_cleanup(unit);
            return BCM_E_PARAM;
        }

        rv = robo_rx_channel_dv_setup(unit, chan);
        if (rv < 0) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "RX: Error on setup unit %d, chan %d\n"),
                       unit, chan));
            robo_rx_cleanup(unit);
            return rv;
        }
        rx_ctl[unit]->chan_running |= (1 << chan);
        ++_rx_chan_run_count;
    }

    RX_INTR_LOCK;
    if (!robo_rx_control.thread_running) {
        robo_rx_control.thread_running = TRUE;
        RX_INTR_UNLOCK;
    /* Start up the thread */
        soc_eth_dma_rxenable(unit);
        if ((rv = robo_rx_thread_start(unit)) < 0) {
            robo_rx_control.thread_running = FALSE;
            robo_rx_cleanup(unit);
            return rv;
        }
        rx_units_started |= (1 << unit);
        rx_units_rxsusped = 0;
    } else {
        RX_INTR_UNLOCK;
    }
    _BCM_ROBO_RX_SYSTEM_LOCK;
    robo_rx_control.system_flags |= BCM_RX_CTRL_ACTIVE_UNITS_UPDATE;
    _BCM_ROBO_RX_SYSTEM_UNLOCK;

    return rv;
}               
    
/*
 * Function:
 *      bcm_rx_stop
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
bcm_robo_rx_stop(int unit, bcm_rx_cfg_t *cfg)
{
    int i;

    RX_INIT_CHECK(unit);

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX: Stopping unit %d\n"), unit));

    if (cfg != NULL) {    /* Save configuration */
        sal_memcpy(cfg, &rx_ctl[unit]->user_cfg, sizeof(bcm_rx_cfg_t));
    }


    RX_INTR_LOCK;

    /* Okay, no one else is running.  Kill thread */
    if (robo_rx_control.thread_running) {
        robo_rx_control.thread_exit_complete = FALSE;
        robo_rx_control.thread_running = FALSE;
        RX_INTR_UNLOCK;
        ROBO_RX_THREAD_NOTIFY(unit);
        for (i = 0; i < 10; i++) {
            if (robo_rx_control.thread_exit_complete) {
                break;
            }
            /* Sleep a bit to allow thread to stop */
            sal_usleep(500000);
        }
        if (!robo_rx_control.thread_exit_complete) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "RX %d: Thread %p running after signaled "
                                 "to stop); \nDVs may not be cleaned up.\n"),
                      unit, (void *)robo_rx_control.rx_tid));
        } else {
            robo_rx_control.rx_tid = NULL;
        }
        rx_units_rxsusped = 1;
        rx_units_started = 0;

    } else {
        RX_INTR_UNLOCK;
    }

    _BCM_ROBO_RX_SYSTEM_LOCK;
    robo_rx_control.system_flags |= BCM_RX_CTRL_ACTIVE_UNITS_UPDATE;
    _BCM_ROBO_RX_SYSTEM_UNLOCK;


    return BCM_E_NONE;


}               
    
/*
 * Function:
 *      bcm_rx_clear
 * Purpose:
 *      Clear all RX info
 * Returns:
 *      BCM_E_NONE
 */

int
bcm_robo_rx_clear(int unit)
{
    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
        if (RX_IS_SETUP(unit)) {
            bcm_rx_stop(unit, NULL);
            robo_rx_cleanup(unit);
            sal_mutex_destroy(rx_ctl[unit]->rx_mutex);
            sal_free(rx_ctl[unit]->pkt_queue);
            sal_free(rx_ctl[unit]);
            rx_ctl[unit] = NULL;
        }
    }

    return BCM_E_NONE;
}
    
/*
 * Function:
 *      bcm_rx_cfg_get
 * Purpose:
 *      Check if init done; get the current RX configuration
 * Parameters:
 *      unit - Robo device ID
 *      cfg - OUT Configuration copied to this parameter.  May be NULL
 * Returns:
 *      BCM_E_INIT if not running on unit
 *      BCM_E_NONE if running on unit
 *      < 0 BCM_E_XXX error code
 * Notes:
 */

int
bcm_robo_rx_cfg_get(int unit, bcm_rx_cfg_t *cfg)
{
    RX_INIT_CHECK(unit);

    /* Copy config */
    if (cfg != NULL) {
        sal_memcpy(cfg, &rx_ctl[unit]->user_cfg, sizeof(bcm_rx_cfg_t));
    }

    return (rx_units_started & (1 << unit)) ? BCM_E_NONE : BCM_E_INIT;
}

/* Install callback handle */ 
static int
_bcm_robo_rx_callback_install(int unit, const char * name, rx_callout_t *rco, 
                             uint8 priority, uint32 flags)
{
    volatile rx_callout_t    *list_rco, *prev_rco;
    int                       i;
    
    RX_LOCK(unit);
    RX_INTR_LOCK;

    /* Need to double check duplicate callback */
    for (list_rco = rx_ctl[unit]->rc_callout; list_rco;
         list_rco = list_rco->rco_next) {
        if (list_rco->rco_function == rco->rco_function &&
            list_rco->rco_priority == rco->rco_priority) {
            if (((list_rco->rco_flags & BCM_RCO_F_INTR) == 
                        (rco->rco_flags & BCM_RCO_F_INTR)) &&
                (list_rco->rco_cookie == rco->rco_cookie)) {
                /* get duplicate handle, update cosq */
                for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                     if (SHR_BITGET(rco->rco_cos, i)) {
                         SHR_BITSET(list_rco->rco_cos, i);
                     }
                }
                RX_INTR_UNLOCK;
                RX_UNLOCK(unit);
                sal_free ((void *)rco);
                return BCM_E_NONE;
            }
            LOG_VERBOSE(BSL_LS_BCM_RX,
                        (BSL_META_U(unit,
                                    "RX: %s registered with diff params\n"),
                         name));
                                                                                                          
            RX_INTR_UNLOCK;
            RX_UNLOCK(unit);
            sal_free((void *)rco);
            return BCM_E_PARAM;
        }
    }
    
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
                            "RX: %s registered %s%s.\n"),
                 name,
                 prev_rco ? "after " : "first",
                 prev_rco ? prev_rco->rco_name : ""));
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_rx_queue_register
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
bcm_robo_rx_queue_register(int unit, const char *name, bcm_cos_queue_t cosq, 
                          bcm_rx_cb_f callback, uint8 priority, void *cookie, 
                          uint32 flags)
{
    volatile rx_callout_t      *rco;
    volatile rx_callout_t      *list_rco;
    int i;

    /*
     * If the caller wants to use the same callback function for different
     * queues, they have to call multiple times. Note that in the driver, 
     * we keep track of the queues using a bitmap so that only one RCO entry 
     * is needed for each callback.
     */
    if (callback == NULL) {
        return BCM_E_PARAM;
    }

    /* Check unit */
    RX_INIT_CHECK(unit);

    /* Check cosq number */
    if ((cosq != BCM_RX_COS_ALL) &&
        (cosq < 0 || cosq > RX_QUEUE_MAX(unit))) {
        return BCM_E_PARAM;
    }

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX: Registering %s on %d, cosq 0x%x flags 0x%x%s\n"),
              name, unit, cosq, flags, flags & BCM_RCO_F_INTR ? "(intr)" : ""));

    /* 
     * In the case of re-installing a callback with the same priority, 
     * unlike bcm_esw_rx_register(), this API will just update the cosq 
     * bitmap for the callback if other parameters are the same. This allows 
     * multiple cosq to be added to same callback via calling 
     * bcm_esw_rx_queue_register() multiple times.
     */

     RX_LOCK(unit);
     RX_INTR_LOCK;
     for (list_rco = rx_ctl[unit]->rc_callout; list_rco;
          list_rco = list_rco->rco_next) {
          uint32 flag_int = flags & BCM_RCO_F_INTR;

          if (list_rco->rco_function == callback &&
              list_rco->rco_priority == priority) {
              if ((list_rco->rco_flags & BCM_RCO_F_INTR) == flag_int &&
                  list_rco->rco_cookie == cookie) {
                  if (cosq == BCM_RX_COS_ALL) {
                      for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                           SHR_BITSET(list_rco->rco_cos, i);
                      }
                  } else {
                    SHR_BITSET(list_rco->rco_cos, cosq);
                    /* Support legacy cosq input via flags */
                    if (flags & BCM_RCO_F_ALL_COS) {
                        for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                            SHR_BITSET(list_rco->rco_cos, i);
                        }
                    } else {
                        for (i = 0; i < 16; i++) {
                          if ((flags & BCM_RCO_F_COS_ACCEPT_MASK) &
                              BCM_RCO_F_COS_ACCEPT(i)) {
                              SHR_BITSET(list_rco->rco_cos, i);
                          }
                      }
                    }
                }
                  RX_INTR_UNLOCK;
                  RX_UNLOCK(unit);
                  return BCM_E_NONE;
              }
              LOG_VERBOSE(BSL_LS_BCM_RX,
                          (BSL_META_U(unit,
                                      "RX: %s registered with diff params\n"), name));

              RX_INTR_UNLOCK;
              RX_UNLOCK(unit);
              return BCM_E_PARAM;
          }
     }

    RX_INTR_UNLOCK;
    RX_UNLOCK(unit);

    /* Alloc a callback struct */
    if ((rco = sal_alloc(sizeof(*rco), "rx_callout")) == NULL) {
        return(BCM_E_MEMORY);
    }

    /* Init the callback struct */
    SETUP_RCO(rco, name, callback, priority, cookie, NULL, flags);

    if (cosq == BCM_RX_COS_ALL) {
        for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
             SHR_BITSET(rco->rco_cos, i);
        }
    } else {
        SHR_BITSET(rco->rco_cos, cosq);
        /* Support legacy cosq input via flags */
        if (flags & BCM_RCO_F_ALL_COS) {
            for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
                SHR_BITSET(rco->rco_cos, i);
            }
        } else {
            for (i = 0; i < 16; i++) {
                if ((flags & BCM_RCO_F_COS_ACCEPT_MASK) & 
                        BCM_RCO_F_COS_ACCEPT(i)) {
                    SHR_BITSET(rco->rco_cos, i);
                }
            }
        }
    }

    return _bcm_robo_rx_callback_install(unit, name, (rx_callout_t *)rco, 
                                       priority, flags);
}


/*
 * Function:
 *      bcm_rx_register
 * Purpose:
 *      Register an upper layer driver
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      chan - DMA channel number
 *      name - constant character string for debug purposes.
 *      priority - priority of handler in list (0 is lowest priority).
 *      f - function to call for that driver.
 *      cookie - cookie passed to driver when packet arrives.
 *      flags - Register for interrupt or non-interrupt callback
 * Returns:
 *      BCM_E_NONE - callout registered.
 *      BCM_E_MEMORY - memory allocation failed.
 */

int
bcm_robo_rx_register(int unit, const char *name, bcm_rx_cb_f callback,
                uint8 priority, void *cookie, uint32 flags)
{
    volatile rx_callout_t      *rco, *list_rco;
    int i;

    RX_INIT_CHECK(unit);

    if (NULL == callback) {
        return BCM_E_PARAM;
    }

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX: Registering %s on %d, flags 0x%x%s\n"),
              name, unit, flags, flags & BCM_RCO_F_INTR ? "(intr)" : ""));

    if (!(flags & BCM_RCO_F_COS_ACCEPT_MASK) &&
            !(flags & BCM_RCO_F_ALL_COS)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "RX unit %d: "
                             "Registering callback with no COS accepted.\n"), unit));
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "    Callbacks will not occur to %s\n"),
                  name));
    }

    RX_LOCK(unit);
    RX_INTR_LOCK; 
    /* First check if already registered */
    for (list_rco = rx_ctl[unit]->rc_callout; list_rco;
         list_rco = list_rco->rco_next) {
        if (list_rco->rco_function == callback &&
            list_rco->rco_priority == priority) {
            if (list_rco->rco_flags == flags &&
                list_rco->rco_cookie == cookie) {
                RX_INTR_UNLOCK;
                RX_UNLOCK(unit);
                return BCM_E_NONE;
            }
            LOG_VERBOSE(BSL_LS_BCM_RX,
                        (BSL_META_U(unit,
                                    "RX: %s registered with diff params\n"),
                         name));
            RX_INTR_UNLOCK;
            RX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    }
    RX_INTR_UNLOCK;
    RX_UNLOCK(unit);

    if ((rco = sal_alloc(sizeof(*rco), "rx_callout")) == NULL) {
        return(BCM_E_MEMORY);
    }
    SETUP_RCO(rco, name, callback, priority, cookie, NULL, flags);

    /* Older implementation used only rco_flags field to carry
     * cos information. Since there are now devices which support > 32
     * cos queues, rco_cos bitmap has been added. Add the specified
     * cos to rco_cos here.
     */
    if (flags & BCM_RCO_F_ALL_COS) {
        for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
            SETUP_RCO_COS_SET(rco, i);
        }
    } else {
        for (i = 0; i < 16; i++) {
            if ((flags & BCM_RCO_F_COS_ACCEPT_MASK) & BCM_RCO_F_COS_ACCEPT(i)) {
                SETUP_RCO_COS_SET(rco, i);
            }
        }
    }
    return _bcm_robo_rx_callback_install(unit, name, (rx_callout_t *)rco, 
                                 priority, flags);
}

/* Common function for bcm_rx_unregister and bcm_rx_queue_unregister */
static int
_bcm_robo_rx_callback_unregister(int unit, bcm_rx_cb_f callback, 
             uint8 priority,  bcm_cos_queue_t cosq)
{
    volatile rx_callout_t *rco;
    volatile rx_callout_t *prev_rco = NULL;
    const char *name;
    uint32 flags;
    int i;

    RX_INIT_CHECK(unit);

    if ((cosq != BCM_RX_COS_ALL) &&
        (cosq < 0 || cosq > RX_QUEUE_MAX(unit))) {
         return BCM_E_PARAM;
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
        return BCM_E_NOT_FOUND;
    }

    if (cosq != BCM_RX_COS_ALL) {
        SHR_BITCLR(rco->rco_cos, cosq);
        for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
             if (SHR_BITGET(rco->rco_cos, i)) {
                 /* Don't delete callback if any cosq still associated */
                 RX_INTR_UNLOCK;
                 RX_UNLOCK(unit);
                 return BCM_E_NONE;
             }
        }
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
                         "RX: Unregistered %s on %d\n"), name, unit));
    sal_free((void *)rco);

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_rx_unregister
 * Purpose:
 *      De-register a callback function
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
bcm_robo_rx_unregister(int unit, bcm_rx_cb_f callback, uint8 priority)
{
    return _bcm_robo_rx_callback_unregister(unit, callback, priority, 
                                           BCM_RX_COS_ALL);
}

/*
 * Function:
 *      bcm_rx_queue_unregister
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
bcm_robo_rx_queue_unregister(int unit, bcm_cos_queue_t cosq,
                            bcm_rx_cb_f callback, uint8 priority)
{
    return _bcm_robo_rx_callback_unregister (unit, callback, priority, cosq);
}

/*
 * Function:
 *      bcm_rx_cosq_mapping_size_get
 * Purpose:
 *      Get number of COSQ mapping entries
 * Parameters:
 *      unit - Unit reference
 *      size - (OUT) number of entries
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_cosq_mapping_size_get(int unit, int *size)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_rx_active
 * Purpose:
 *      Return boolean as to whether unit is running
 * Parameters:
 *      unit - Robo to check
 * Returns:
 *      Boolean:   TRUE if unit is running.
 * Notes:
 *      
 */

int
bcm_robo_rx_active(int unit)
{
    if (((rx_units_started & (1 << unit)) != 0) && (rx_units_rxsusped==0)) {        
        return 1;    
    } else {        
        return 0;    
    }
}       

/*
 * Function:
 *      bcm_rx_debug
 * Purpose:
 *      Dump the DMA register and information.
 * Parameters:
 *      unit - Robo to check
 * Returns:
 *      Boolean:   TRUE.
 * Notes:
 *      
 */

int
bcm_rx_debug(int unit)
{
    et_soc_debug(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_rx_running_channels_get
 * Purpose:
 *      Returns a bitmap indicating which channels are active
 * Parameters:
 *      unit       - Which unit to operate on
 * Returns:
 *      Bitmap of active channels
 * Notes:
 */

int
bcm_robo_rx_channels_running(int unit, uint32 *channels)
{
    if (!RX_INIT_DONE(unit)) {
    return 0;
    }
    *channels = rx_ctl[unit]->chan_running;
    return BCM_E_NONE;
}               
    
/*
 * Function:
 *      bcm_rx_alloc
 * Purpose:
 *      Gateway to configured RX allocation function
 * Parameters:
 *      unit - Unit reference
 *      pkt_size - Packet size, see notes.
 *      flags - Used to set up packet flags
 * Returns:
 *      Pointer to new packet buffer or NULL if cannot alloc memory
 * Notes:
 *      Although the packet size is normally configured per unit,
 *      the option of using a different size is given here.  If
 *      pkt_size <= 0, then the default packet size for the unit
 *      is used.
 */

int
bcm_robo_rx_alloc(int unit, int pkt_size, uint32 flags, void **buf)
{
    if (unit < 0) {
        unit = BCM_RX_DFLT_UNIT;
    }

    if (!RX_INIT_DONE(unit)) {
        buf = NULL;                    
        return BCM_E_INIT;
    }

    if (pkt_size <= 0) {
        pkt_size = rx_ctl[unit]->user_cfg.pkt_size;
    }

    return rx_ctl[unit]->user_cfg.rx_alloc(unit, pkt_size, flags, buf);
}
/*
 * Function:
 *      bcm_rx_free
 * Purpose:
 *      Gateway to configured RX free function.  Generally, packet
 *      buffer was allocated with bcm_rx_alloc.
 * Parameters:
 *      unit - Unit reference
 *      pkt - Packet to free
 * Returns:
 * Notes:
 */

int
bcm_robo_rx_free(int unit, void *pkt_data)
{
    if (unit < 0) {
        unit = BCM_RX_DFLT_UNIT;
    }

#if defined(BROADCOM_DEBUG)
    if (rx_ctl[unit] == NULL || !rx_ctl[unit]->user_cfg.rx_free) {
        return BCM_E_MEMORY;
    }
#endif

    if (pkt_data) {
        rx_ctl[unit]->user_cfg.rx_free(unit, pkt_data);
    }
    return BCM_E_NONE;
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
bcm_robo_rx_free_enqueue(int unit, void *pkt_data)
{
    if (pkt_data == NULL) {
        return BCM_E_PARAM;
    }

    if (!RX_INIT_DONE(unit) || robo_rx_control.rx_tid == NULL) {
        return BCM_E_INIT;
    }

    RX_INTR_LOCK;
    PKT_TO_FREE_NEXT(pkt_data) = (void *)(rx_ctl[unit]->free_list);
    rx_ctl[unit]->free_list = pkt_data;
    RX_INTR_UNLOCK;

    ROBO_RX_THREAD_NOTIFY(unit);

    return BCM_E_NONE;
}


/****************************************************************
 *
 * Global and per COS rate limitting configuration
 *
 ****************************************************************/


/*
 * Functions:
 *      bcm_rx_burst_set, get; bcm_rx_rate_set, get
 *      bcm_rx_cos_burst_set, get; bcm_rx_cos_rate_set, get;
 *      bcm_rx_cos_max_len_set, get
 * Purpose:
 *      Get/Set the global and per COS limits:
 *           rate:      Packets/second
 *           burst:     Packets (max tokens in bucket)
 *           max_len:   Packets (max permitted in queue).
 * Parameters:
 *      unit - Unit reference
 *      cos - For per COS functions, which COS queue affected
 *      pps - Rate in packets per second (OUT for get functions)
 *      burst - Burst rate for the system in packets (OUT for get functions)
 *      max_q_len - Burst rate for the system in packets (OUT for get functions)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Burst must be >= 1,
 *      PPS must be >= 0 and
 *      Max queue length must be >= 0;
 *          otherwise param error.
 *
 *      PPS == 0 -> rate limitting disabled.
 *      max_q_len == 0 -> no limit on queue length (not recommended)
 */

int
bcm_robo_rx_rate_set(int unit, int pps)
{
    RX_INIT_CHECK(unit);

    if (pps < 0) {
        return BCM_E_PARAM;
    }
    RX_PPS(unit) = pps;

    return BCM_E_NONE;
}               
                                    
int
bcm_robo_rx_rate_get(int unit, int *pps)
{
    RX_INIT_CHECK(unit);

    if (pps) {
        *pps = RX_PPS(unit);
    }
    return BCM_E_NONE;
}

int 
bcm_robo_rx_cpu_rate_set(int unit, int pps)
{
    if (pps < 0) {
        return BCM_E_PARAM;
    }
    robo_rx_control.system_pps = pps;

    return BCM_E_NONE;
}

int 
bcm_robo_rx_cpu_rate_get(int unit, int *pps)
{
    if (pps) {
        *pps = robo_rx_control.system_pps;
    } 
    return BCM_E_NONE;
}
    
int
bcm_robo_rx_burst_set(int unit, int burst)
{
    RX_INIT_CHECK(unit);

    RX_BURST(unit) = burst;
    RX_TOKENS(unit) = burst;

    return BCM_E_NONE;
}               
    
int
bcm_robo_rx_burst_get(int unit, int *burst)
{
    RX_INIT_CHECK(unit);

    if (burst) {
        *burst = RX_BURST(unit);
    }
    return BCM_E_NONE;
}               
    
#define LEGAL_COS(cos) ((cos) >= BCM_RX_COS_ALL && (cos) < BCM_RX_COS)
                                                    
int
bcm_robo_rx_cos_rate_set(int unit, int cos, int pps)
{
    if (!LEGAL_COS(cos) || pps < 0) {
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);

    if (cos > RX_QUEUE_MAX(unit)) {
        return BCM_E_PARAM;
    }
    
    if (cos == BCM_RX_COS_ALL) {
        for (cos = 0; cos <= RX_QUEUE_MAX(unit); cos++) {
            RX_COS_PPS(unit, cos) = pps;
        }
    } else {
        RX_COS_PPS(unit, cos) = pps;
    }

    return BCM_E_NONE;
}               
    
int
bcm_robo_rx_cos_rate_get(int unit, int cos, int *pps)
{
    RX_INIT_CHECK(unit);

    if (cos > RX_QUEUE_MAX(unit)) {
        return BCM_E_PARAM;
    }
    
    if (pps) {
        *pps = RX_COS_PPS(unit, cos);
    }

    return BCM_E_NONE;
}


int
bcm_robo_rx_cos_burst_set(int unit, int cos, int burst)
{
    rx_queue_t *queue;
    int i;
    
    if (!LEGAL_COS(cos) || burst < 1) {
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);

    if (cos > RX_QUEUE_MAX(unit)) {
        return BCM_E_PARAM;
    }
    
    if (cos == BCM_RX_COS_ALL) {
        for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
            queue = RX_QUEUE(unit, i);
            queue->burst = burst;
            queue->tokens = burst;
        }
    } else {
        queue = RX_QUEUE(unit, cos);
        queue->burst = burst;
        queue->tokens = burst;
    }

    return BCM_E_NONE;
}               
    
int
bcm_robo_rx_cos_burst_get(int unit, int cos, int *burst)
{
    RX_INIT_CHECK(unit);
    if (cos > RX_QUEUE_MAX(unit)) {
        return BCM_E_PARAM;
    }
    
    if (burst) {
        *burst = RX_COS_BURST(unit, cos);
    }

    return BCM_E_NONE;
}

int
bcm_robo_rx_cos_max_len_set(int unit, int cos, int max_q_len)
{
    if (!LEGAL_COS(cos) || max_q_len < 0) {
        return BCM_E_PARAM;
    }

    RX_INIT_CHECK(unit);

    if (cos > RX_QUEUE_MAX(unit)) {
        return BCM_E_PARAM;
    }
    
    if (cos == BCM_RX_COS_ALL) {
        for (cos = 0; cos <= RX_QUEUE_MAX(unit); cos++) {
            RX_COS_MAX_LEN(unit, cos) = max_q_len;
        }
    } else {
        RX_COS_MAX_LEN(unit, cos) = max_q_len;
    }

    return BCM_E_NONE;
}               
    
int
bcm_robo_rx_cos_max_len_get(int unit, int cos, int *max_q_len)
{
    RX_INIT_CHECK(unit);

    if (cos > RX_QUEUE_MAX(unit)) {
        return BCM_E_PARAM;
    }
    
    if (max_q_len) {
        *max_q_len = RX_COS_MAX_LEN(unit, cos);
    }

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *      bcm_rx_remote_pkt_enqueue
 * Purpose:
 *      Enqueue a remote packet for normal RX processing
 * Parameters:
 *      unit          - The BCM unit in which queue the pkt is placed
 *      pkt           - The packet to enqueue
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_rx_remote_pkt_enqueue(int unit, bcm_pkt_t *pkt)
{
    return BCM_E_UNAVAIL;
}

/****************************************************************
 *
 * The non-interrupt thread
 *
 ****************************************************************/

STATIC int
robo_rx_thread_start(int unit)
{
    int priority;
    
    /* Timer/Event semaphore thread sleeping on. */
    if (NULL == robo_rx_control.pkt_notify) {
        robo_rx_control.pkt_notify = 
            sal_sem_create("RX pkt ntfy", sal_sem_BINARY, 0);
        if (NULL == robo_rx_control.pkt_notify) {
            return (BCM_E_MEMORY);
        }
        robo_rx_control.pkt_notify_given = FALSE;
    }

    /* RX start/stop on one of the units protection mutex. */
    if (NULL == robo_rx_control.system_lock) {
        robo_rx_control.system_lock = sal_mutex_create("RX system lock");
        if (NULL == robo_rx_control.system_lock) {
            sal_sem_destroy(robo_rx_control.pkt_notify);
            return (BCM_E_MEMORY);
        }
    }

    if (SOC_UNIT_VALID(unit)) {
        priority = soc_property_get(unit,
                                    spn_BCM_RX_THREAD_PRI,
                                    RX_THREAD_PRI_DFLT);
    } else {
        priority = RX_THREAD_PRI_DFLT;
    }

    if (NULL == robo_rx_control.rx_sched_cb) {
        robo_rx_control.rx_sched_cb = _BCM_ROBO_RX_SCHED_DEFAULT_CB;
    }

    /* Start rx thread. */
    robo_rx_control.rx_tid = sal_thread_create("bcmRX",
                                          SAL_THREAD_STKSZ,
                                          priority,
                                          robo_rx_pkt_thread, NULL);
    /* Thread creation error handling. */
    if (NULL == robo_rx_control.rx_tid) {
        sal_sem_destroy(robo_rx_control.pkt_notify);
        sal_mutex_destroy(robo_rx_control.system_lock);
        robo_rx_control.pkt_notify = NULL;
        robo_rx_control.system_lock = NULL;
        return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}


/*
 * Calculate number of tokens that should be added to a bucket
 *
 * Add pps tokens per second to the bucket (up to max_burst).
 * Find the number of us per token (assuming pps is much less than 1M).
 *
 *     (us per token) = 1000000 us/sec / (pps tokens/sec)
 *
 * For example, for 2000 pps, add one token every 500 us.  We then
 * check the elapsed time since the last addition and divide that
 * by us per token.
 *
 * That is:
 *
 *     (tokens to add) = (elapsed us since last add) / (us per token)
 *                     = (cur - last) / (1000000 / pps)
 *                     = ((cur - last) * pps) / 1000000
 *
 * Note:  When pps > 10000, there can be an integer overflow from
 * the multiplication.  We fix this by changing order of the calculation
 * depending on the size of pps.
 */

#define CALC_TOKENS(cur_time, last_time, pps) \
    ( ((pps) < 1000) ? \
        ((SAL_USECS_SUB(cur_time, last_time) * (pps)) / 1000000) : \
        (((SAL_USECS_SUB(cur_time, last_time) / 10000) * (pps))/100) )

#define us_PER_TOKEN(pps) (1000000 / (pps))
static sal_usecs_t last_fill_check = 0;    /* Last time tokens checked */

/*
 * Function:
 *      _robo_token_update
 * Purpose:
 *      Update the tokens for one bucket
 * Parameters:
 *      cur_time - current time in us
 *      last_time - (IN/OUT) last time bucket was filled
 *      pps - rate limit for bucket; 0 means disabled
 *      burst - burst limit for bucket
 *      token_bucket - (IN/OUT) bucket to be updated
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Assumes burst > 0 (as forced by config functions)
 */
STATIC INLINE void
_robo_token_update(sal_usecs_t cur_time,int pps, int burst, 
        volatile int *token_bucket, sal_usecs_t *last_fill)
{
    int new_tokens;
    int max_add;
    int bucket_top;
    int time_diff;

    max_add = bucket_top = pps > burst ? pps : burst;

    time_diff = SAL_USECS_SUB(cur_time, *last_fill);
    if (time_diff < 0) {
        /* In case the clock has changed, update last fill to cur_time */
        *last_fill = cur_time;
        return;
    }
    if (time_diff == 0) {
        return;
    }

    /* Cap the number of additions by the fraction of a second refreshed */
    if (time_diff < 1000000) {
        max_add = pps / (1000000/time_diff);
    }

    /* Not enough time passed to add tokens at this rate */
    if (max_add == 0) {
        return;
    }

    if (*token_bucket < bucket_top) {
        new_tokens = CALC_TOKENS(cur_time, *last_fill, pps);
        if (new_tokens > max_add) {
            new_tokens = max_add;
        }
        if (new_tokens > 0) {
            *token_bucket += new_tokens;
            if (*token_bucket > bucket_top) {
                *token_bucket = bucket_top;
            }
            *last_fill = cur_time;
        }
    }
}

/* Add tokens to all active token buckets */
STATIC INLINE void
_robo_all_tokens_update(sal_usecs_t cur_time)
{
    int cos, unit = 0;
    rx_queue_t *queue;
  
    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
        if (RX_IS_SETUP(unit)) {
            for (cos = 0; cos <= RX_QUEUE_MAX(unit); cos++) {
                queue = RX_QUEUE(unit, cos);
                if (queue->pps > 0) {
                    _robo_token_update(cur_time, queue->pps, queue->burst,
                                  &queue->tokens, &queue->last_fill);
                }
            }
            if (RX_PPS(unit) > 0) {
                _robo_token_update(cur_time, RX_PPS(unit), RX_BURST(unit),
                              &rx_ctl[unit]->tokens, &rx_ctl[unit]->last_fill);
            }
        }
    }

    /* Check system wide rate limiting */
    if (robo_rx_control.system_pps > 0) {
        _robo_token_update(cur_time, robo_rx_control.system_pps, 0,
                      &robo_rx_control.system_tokens,
                      &robo_rx_control.system_last_fill);
    }   

    last_fill_check = cur_time;
}
    
/* Free all buffers listed in pending free list */
STATIC void
robo_rx_free_queued(int unit)
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
 * Thread support functions
 */

/* Start a chain and update the tokens */
STATIC INLINE int
robo_rx_chain_start(int unit, int chan, eth_dv_t *dv)
{
    int rv = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_RX,
                (BSL_META_U(unit,
                            "RX: Starting %d/%d/%d\n"),
                 unit, chan, ROBO_DV_INDEX(dv)));

    if (!RX_INIT_DONE(unit) || !robo_rx_control.thread_running) {
        /* Revert state in case scheduled */
        ROBO_DV_STATE(dv) = DV_S_FILLED;
        return BCM_E_NONE;
    }

    /* Start the DV */
    ROBO_DV_STATE(dv) = DV_S_ACTIVE;

    if ((rv = soc_eth_dma_start(unit, dv)) < 0) {
        ROBO_DV_STATE(dv) = DV_S_ERROR;
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "RX: Could not start dv, u %d, chan %d\n"), unit, chan));
    }

    return rv;
}

/*
 * The DV (chain) given is ready
 * to go.  If rate limitting allows that, start the chain.  If
 * not, schedule the chain in the future.
 *
 * This is implemented by:  First updating the global and
 * per channel token buckets.  If a bucket is negative, this
 * indicates a time in the future when the DV can be scheduled,
 * based on the pkts/sec of the limit.
 *
 * Calculate both (global and channel) start times and take the
 * later one.
 *
 * NOTE:  This routine should only be called from inside of
 * the rx thread b/c it affects variables that that thread
 * depends on for timing.
 */
#define USEC_DELAY(tokens, ppc, pps) \
    ((-(tokens) + (ppc)) * us_PER_TOKEN(pps))
STATIC INLINE int
robo_rx_chain_start_or_sched(int unit, int chan, eth_dv_t *dv)
{
    int global_usecs = 0;
    int system_usecs = 0;
    sal_usecs_t cur_time;

    LOG_VERBOSE(BSL_LS_BCM_RX,
                (BSL_META_U(unit,
                            "RX: Chain. glob tok %d.\n"),
                 RX_TOKENS(unit)));

    /*
     * Decrement the token buckets for global limits.
     * If negative, we must schedule the packet in the
     * future.  The time to schedule is based on the number of usecs
     * needed to get the bucket up to pkts/chain again.
     */
    if (robo_rx_control.system_pps > 0) { /* System rate limiting */
        robo_rx_control.system_tokens -= RX_PPC(unit);
        if (robo_rx_control.system_tokens < 0) {
            system_usecs = USEC_DELAY(robo_rx_control.system_tokens,
                                      RX_PPC(unit), robo_rx_control.system_pps);
        }
    }
    if (RX_PPS(unit)) { /* Global rate limitting is on */
        RX_TOKENS(unit) -= RX_PPC(unit);
        if (RX_TOKENS(unit) < 0) {
            global_usecs = USEC_DELAY(rx_ctl[unit]->tokens,
                                      RX_PPC(unit), RX_PPS(unit));
        }
    }

    /* Use the maximum delay */
    if (global_usecs < system_usecs) {
        global_usecs = system_usecs;
    }

    if (!global_usecs) { /* Can start immediately */
        return robo_rx_chain_start(unit, chan, dv);
    } else {

        /***** The DV must be scheduled into the future *****/

        ROBO_DV_STATE(dv) = DV_S_SCHEDULED;
        cur_time = sal_time_usecs();
        DV_SCHED_TIME(dv) = cur_time;
        DV_TIME_DIFF(dv) = global_usecs;
        SLEEP_MIN_SET(global_usecs);
        LOG_INFO(BSL_LS_BCM_RX,
                 (BSL_META_U(unit,
                             "RX: Scheduling %d/%d/%d in %d us; "
                             "cur %u; sleep %u\n"), unit, chan, ROBO_DV_INDEX(dv),
                  global_usecs, cur_time, robo_rx_control.sleep_cur));
    }

    return BCM_E_NONE;
}

/*
 * Based on the state of the DV, carry out an action.
 * If it is scheduled, check if it may be started.
 *
 * NOTE:  This routine should only be called from inside of
 * the rx thread b/c it affects variables that that thread
 * depends on for timing.
 */

STATIC INLINE int
robo_rx_update_dv(int unit, int chan, eth_dv_t *dv)
{
    sal_usecs_t cur_usecs;
    int sched_usecs;
    int rv = BCM_E_NONE;

    /* If no real callouts, don't bother starting DVs */
    if (!robo_rx_control.thread_running || NO_REAL_CALLOUTS(unit)) {
        if (ROBO_DV_STATE(dv) == DV_S_SCHEDULED) {
            ROBO_DV_STATE(dv) = DV_S_FILLED;
        }
        return BCM_E_NONE;
    }

    assert(dv);

    switch (ROBO_DV_STATE(dv)) {
    case DV_S_NEEDS_FILL:
        robo_rx_dv_fill(unit, chan, dv);
        /* If fill succeeded, try to schedule/start; otherwise break. */
        if (ROBO_DV_STATE(dv) != DV_S_FILLED) {
            break;
        }
        /* Fall thru:  Ready to be scheduled/started */
    case DV_S_FILLED: /* See if we can start or schedule this DV */
         rv = robo_rx_chain_start_or_sched(unit, chan, dv);
        break;
    case DV_S_SCHEDULED:
        cur_usecs = sal_time_usecs();
        sched_usecs = DV_FUTURE_US(dv, cur_usecs);
        if (sched_usecs <= 0) {
            /* Start the dv */
            LOG_INFO(BSL_LS_BCM_RX,
                     (BSL_META_U(unit,
                                 "RX: Starting scheduled %d/%d/%d, diff %d "
                                 "@ %u\n"), unit, chan, ROBO_DV_INDEX(dv),
                      sched_usecs, cur_usecs));
            rv = robo_rx_chain_start(unit, chan, dv);
        } else {
            /* Still not ready to be started; set sleep min time */
            LOG_INFO(BSL_LS_BCM_RX,
                     (BSL_META_U(unit,
                                 "RX: %d/%d/%d not ready at %u, diff %d\n"),
                      unit, chan, ROBO_DV_INDEX(dv), cur_usecs, sched_usecs));
            SLEEP_MIN_SET(sched_usecs);
        }
        break;
    default: /* Don't worry about other states here. */
        break;
    }

    return rv;
}

/*
 * Function:
 *      robo_rx_thread_dv_check
 * Purpose:
 *      Check if any DVs need refilling, starting or scheduling
 * Parameters:
 *      unit - unit to examine
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      To avoid starvation, a "current" channel is maintained that
 *      rotates through the possible channels.
 */
#define _CUR_CHAN(unit) (rx_ctl[unit]->cur_chan)

STATIC void
robo_rx_thread_dv_check(int unit)
{
    int chan;
    int i, j;

    chan = _CUR_CHAN(unit);

    for (j=0; j < BCM_ROBO_RX_CHANNELS; j++) {
        if (RX_CHAN_RUNNING(unit, chan)) {
            for (i = 0; i < RX_CHAINS(unit, chan); i++) {
                robo_rx_update_dv(unit, chan, ROBO_RX_DV(unit, chan, i));
            }
        }
        chan = (chan + 1) % BCM_ROBO_RX_CHANNELS;
    }

    _CUR_CHAN(unit) = (_CUR_CHAN(unit) + 1) % BCM_ROBO_RX_CHANNELS;
}

#undef _CUR_CHAN


/*
 * Function:
 *      robo_rx_thread_pkts_process
 * Purpose:
 *      Process all pending packets for a particular COS
 * Parameters:
 *      unit - the unit being checked
 *      cos - the COS to check
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Along the way, check DV states.
 *      
 */

STATIC INLINE int
robo_rx_thread_pkts_process(int unit, int cos, int count)
{
    bcm_pkt_t *pkt_list = NULL;
    bcm_pkt_t *next_pkt;
    rx_queue_t *queue;
    int rv = BCM_E_NONE;

    /*
     * Steal the queue's packets; if rate limiting on, just take as many
     * as can be handled.
     */
    queue = RX_QUEUE(unit, cos);

    if ((count > queue->count) || (count < 0)) {
        count = queue->count;
    }

    if ((queue->pps > 0) && (count > queue->tokens)) {
        count = queue->tokens;
    }

    if (0 == count) {
        return (BCM_E_NONE);
    }

    _Q_STEAL(queue, pkt_list, count);
    if (pkt_list == NULL) {
        return (BCM_E_NONE);
    }

    /* Process packets */
    while (pkt_list) {
        next_pkt = pkt_list->_next;

        /* Make sure scheduler properly enforces queue rate limiting. */
        if (queue->pps > 0) {
            if (queue->tokens > 0) {
                queue->tokens--;
            } 
        }

        robo_rx_process_packet(unit, pkt_list);       
        
        pkt_list = next_pkt;

        _BCM_ROBO_RX_CHECK_THREAD_DONE;
    }  

    LOG_VERBOSE(BSL_LS_BCM_RX,
                (BSL_META_U(unit,
                            "Processed (%d) packets"), count));
    return rv;
}

/*
 * Clean up queues on thread exit.  The queued packets still belong
 * to DVs, so they'll be freed when robo_rx_dv_dealloc is called.
 */
STATIC INLINE void
robo_rx_cleanup_queues(int unit)
{
    int i;
    rx_queue_t *queue;

    robo_rx_free_queued(unit);
    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
        queue = RX_QUEUE(unit, i);
        if (!_Q_EMPTY(queue)) {
            queue->count = 0;
            queue->head = NULL;
            queue->tail = NULL;
        }
    }
}

/* Called on error or thread exit to clean up DMA, etc. */
STATIC void
robo_rx_cleanup(int unit)
{
    int chan;

    /* Abort running DVs */
    /* Robo doesn't have dma abort
    if (soc_robo_dma_abort(unit) < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "RX: Error aborting DMA\n")));
    }
    */
    /* Sleep .1 sec to allow abort to complete and pending pkts thru */
    sal_usleep(100000);
    robo_rx_cleanup_queues(unit);
    /* For each channel, dealloc DVs and mark not running */
    FOREACH_SETUP_CHANNEL(unit, chan) {
        robo_rx_channel_shutdown(unit, chan);
    }      
}

/*
 * Function:
 *      _bcm_robo_rx_default_scheduler
 * Purpose:
 *      Schedule number of packets to be processed 
 *      on a specific unit, queue pair.
 * Parameters:
 *      unit         - (IN)  BCM device number. 
 *      sched_unit   - (OUT) Device to process packets. 
 *      sched_cosq   - (OUT) Queue to process packets. 
 *      sched_count  - (OUT) Number of packets to process.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_robo_rx_default_scheduler(int unit, int *sched_unit, 
                          bcm_cos_queue_t *sched_cosq, int *sched_count) 
{
    int pkt_in_queue;         /* Number of packets waiting.   */ 
    int pkt_limit;            /* Max packets to schedule due  */ 
                              /* to rate limiting.            */ 
    static bcm_cos_queue_t cosq_idx = -1; /* Queue iterator.  */
    static int unit_next = -1;/* Bcm device iterator.         */
    
    
    /* Get maximum cosq supported by the system. */
    if ((-1 == unit_next) &&
        (cosq_idx == -1)) { 
        /* Queues & units iteration initialization. */
        cosq_idx = robo_rx_control.system_cosq_max;
        unit_next = unit;
    } else {
        /* Proceed to the next unit. */
        BCM_IF_ERROR_RETURN(bcm_robo_rx_unit_next_get(unit_next, &unit_next));
        /* If no other unit found proceed to the next queue.*/
        if (-1 == unit_next) {
            unit_next = unit;
            cosq_idx--;
        }
    }

    for (; cosq_idx >= 0; cosq_idx--) {
        /* Check if thread exit request was issued. */
        _BCM_ROBO_RX_CHECK_THREAD_DONE;

        /* Check if queue is supported by the system. */
        for(;unit_next != -1;) {
            /* Check if thread exit request was issued. */
            _BCM_ROBO_RX_CHECK_THREAD_DONE;

            /* Check if queue is supported by the device. */ 
            if (cosq_idx > RX_QUEUE_MAX(unit_next)) {
                continue;
            }

            /* Get number of packets awaiting processing. */
            BCM_IF_ERROR_RETURN
                (bcm_robo_rx_queue_packet_count_get(unit_next, 
                                                   cosq_idx, &pkt_in_queue));
            /* Queue rate limiting enforcement. */
            if (pkt_in_queue) {
                BCM_IF_ERROR_RETURN
                    (bcm_robo_rx_queue_rate_limit_status_get(unit_next, cosq_idx, 
                                                            &pkt_limit));
                *sched_unit = unit_next;
                *sched_cosq = cosq_idx;
                if (BCM_RX_SCHED_ALL_PACKETS == pkt_limit) {
                    *sched_count = pkt_in_queue;
                } else if (pkt_limit < pkt_in_queue) {
                    *sched_count = pkt_limit;;
                }  else {
                    *sched_count = pkt_in_queue;
                }
                return (BCM_E_NONE);
            }
            BCM_IF_ERROR_RETURN(bcm_robo_rx_unit_next_get(unit_next, &unit_next));
        }
        unit_next = unit;
    }

    /* Prep for the next scheduling cycle. */
    cosq_idx = -1;   
    unit_next = -1;   
    /* Done indicator. */
    *sched_count = 0;

    return (BCM_E_NONE);
}


/*
 * Function:
 *      robo_rx_pkt_thread
 * Purpose:
 *      User level thread that handles packets and DV refills
 * Parameters:
 *      param    - Unit number for this thread
 * Notes:
 *      This can get awoken for one or more of the following reasons:
 *      1.  Descriptor(s) are done (packets ready to process)
 *      2.  RX stop is called ending processing
 *      3.  Interrupt processing has handled packets and DV may be
 *          ready for refill.
 *      4.  Periodic check in case memory has freed up.
 *
 *      When a DV is scheduled to start, it may change the
 * next waking time of this thread by setting sleep_cur.
 * 
 */
STATIC void
robo_rx_pkt_thread(void *param)
{
    int unit = PTR_TO_INT(param);
    int unit_update_idx;
    bcm_cos_queue_t cosq; 
    int count;
    sal_usecs_t cur_time;

    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX:  Packet thread starting\n")));

    INIT_SLEEP;
    /* Sleep on sem */
    while (robo_rx_control.thread_running) {

        /* Get current time. */
        cur_time = sal_time_usecs();
    
        /* Protection agains clock moving back. */
        if (SAL_USECS_SUB(cur_time, last_fill_check) < 0) {
            last_fill_check = cur_time;
        }

        /* Time based tokens refill check. */
        if (SAL_USECS_SUB(cur_time, last_fill_check) >= 
            bcm_robo_rx_token_check_us) {
            _robo_all_tokens_update(cur_time);
        }

        /* Make sure scheduler callback was registered. */
        if (NULL == robo_rx_control.rx_sched_cb) {
            break;
        }

        /* Lock system rx start/stop mechanism. */
        _BCM_ROBO_RX_SYSTEM_LOCK;

        /* Check if system active units list update is required.  */
        if (robo_rx_control.system_flags & BCM_RX_CTRL_ACTIVE_UNITS_UPDATE) {
            _bcm_robo_rx_unit_list_update(); 
            robo_rx_control.system_flags &= ~BCM_RX_CTRL_ACTIVE_UNITS_UPDATE;
        }

        unit_update_idx =  robo_rx_control.rx_unit_first;

         /* Schedule number of packets to be processed. */
        while (BCM_SUCCESS(robo_rx_control.rx_sched_cb(
                    robo_rx_control.rx_unit_first, &unit, &cosq, &count))) {
            /* Check for  completion. */
            if (!count) {
                break;
            }
            /* Process packets based on scheduler allocation. */
            if (BCM_FAILURE(robo_rx_thread_pkts_process(unit, cosq, count))) {
                LOG_INFO(BSL_LS_BCM_RX,
                         (BSL_META_U(unit,
                                     "Packet rx failed - check the scheduler.\n")));
            }
            /* 
             *  Free queued packets and check DVs for all units 
             *  in rx started list before the scheduled one.
             */
            for (;-1 != unit_update_idx;) {
                robo_rx_free_queued(unit_update_idx);
                robo_rx_thread_dv_check(unit_update_idx);
                if (unit_update_idx == unit) {
                    break;
                }
                bcm_robo_rx_unit_next_get(unit_update_idx, &unit_update_idx);
            }
            _BCM_ROBO_RX_CHECK_THREAD_DONE;
        }

        /* 
         *  Free queued packets and check DVs for all units 
         *  in rx started list after last scheduled unit. 
         */
        for (;-1 != unit_update_idx;) {
            /* Free queued packets and check DVs */
            robo_rx_free_queued(unit_update_idx);
            robo_rx_thread_dv_check(unit_update_idx);
            bcm_robo_rx_unit_next_get(unit_update_idx, &unit_update_idx);
        }

        /* Unlock system rx start/stop mechanism. */
        _BCM_ROBO_RX_SYSTEM_UNLOCK;

        _BCM_ROBO_RX_CHECK_THREAD_DONE;

        SLEEP_MIN_SET(BASE_SLEEP_VAL);

        sal_sem_take(robo_rx_control.pkt_notify, robo_rx_control.sleep_cur);
        robo_rx_control.pkt_notify_given = FALSE;

        INIT_SLEEP;
    }

    soc_eth_dma_rxstop(unit);
    robo_rx_cleanup(unit);
    robo_rx_control.thread_exit_complete = TRUE;
    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "RX: Packet thread exitting\n")));

    sal_thread_exit(0);
}

/****************************************************************
 *
 * Interrupt handling routines:
 *     Packet done
 *     Chain done
 *
 ****************************************************************/

/*
 * Function:
 *      robo_rx_done_packet
 * Purpose:
 *      Process a packet done done notification.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      dv - pointer to dv structure active
 *      dcb - pointer to dcb completed.
 * Returns:
 *      Nothing.
 * Notes:
 *      This routine is called in interrupt context
 *
 *      We count on the non-interrupt thread to update
 *      the tokens for each queue.
 */

STATIC void
robo_rx_done_packet(int unit, eth_dv_t *dv, eth_dcb_t *dcb)
{
    volatile bcm_pkt_t *pkt;
    int pkt_dcb_idx;
    int chan;
    uint8 offset = 0;
    uint16 dev_id;
    uint8 rev_id;

    chan = ROBO_DV_CHANNEL(dv);
    pkt_dcb_idx = (dcb - dv->dv_dcb) / RX_DCB_PER_PKT(unit, chan);
    pkt = ROBO_DV_PKT(dv, pkt_dcb_idx);

    /*
     * Workaround for BCM53242/53262A0,
     * Also see et_soc.c, et_soc_sendup()
     */
    soc_cm_get_id(unit, &dev_id, &rev_id);
    if (((dev_id == BCM53242_DEVICE_ID) && (rev_id == BCM53242_A0_REV_ID)) || 
        ((dev_id == BCM53262_DEVICE_ID) && (rev_id == BCM53262_A0_REV_ID))) {
        offset = dv->dv_public4.u8;
        pkt->_pkt_data.data += offset;
    }

    /* Shift 4 bytes of buffer pointer for the packets with timestamp */
    if ((soc_feature(unit, soc_feature_eav_support)) && 
        (dv->dv_opcode == BRCM_OP_EGR_TS)) {
        pkt->_pkt_data.data += sizeof(uint32);
    }

#if BCM_ROBO_RX_CRC_GEN
    {
        uint32 crc;
        uint8   *tail_p = NULL;
        
        if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit)) {
            /* Calculate original CRC */
            crc = ~_shr_crc32(~0, pkt->_pkt_data.data, dv->dv_length - sizeof(uint32));
            tail_p = (uint8*)(pkt->_pkt_data.data);
            sal_memcpy(tail_p + (dv->dv_length - sizeof(uint32)), 
                &crc, sizeof(uint32));
        }
    }
#endif /* BCM_ROBO_RX_CRC_GEN */

    assert(pkt_dcb_idx == pkt->_idx);

    rx_ctl[unit]->tot_pkts++;
    rx_ctl[unit]->pkts_since_start++;
    if (!robo_rx_control.thread_running) {
        /* Thread is not running; Ignore packet */
        rx_ctl[unit]->thrd_not_running++;
        if (++ROBO_DV_PKTS_PROCESSED(dv) == RX_PPC(unit)) {
            ROBO_DV_STATE(dv) = DV_S_NEEDS_FILL;
        }
        return;
    }

    if (RX_CHAN_RUNNING(unit, chan)) {
        /* Do not process packet if an error is noticed. */
        if (!dv->dv_flags) {
            robo_rx_intr_process_packet(unit, dv, dcb, (bcm_pkt_t *)pkt);
        } else {
            rx_ctl[unit]->dcb_errs++;
            ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
        }
    } else {/* If not active, mark the packet as handled */
        rx_ctl[unit]->not_running++;
        ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
    }
}               
    

STATIC INLINE void
robo_pkt_len_get(int unit, int chan, bcm_pkt_t *pkt, eth_dv_t *dv)
{
    
    pkt->tot_len = dv->dv_length;
    pkt->pkt_len = dv->dv_length;

#if BCM_ROBO_RX_CRC_GEN
    if (BCM_PKT_RX_CRC_STRIP(pkt)) {
        pkt->pkt_len -= sizeof(uint32);
    }
#else /* !BCM_ROBO_RX_CRC_GEN */    
    if (BCM_PKT_RX_CRC_STRIP(pkt)) {
        if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit)) {
            /* The packets didn't include the original CRC value */
            ;
        } else {
            pkt->pkt_len -= sizeof(uint32);
        }
    }
#endif /* BCM_ROBO_RX_CRC_GEN */    

    
}


STATIC void
_pkt_robo_reasons_get(int unit, bcm_pkt_t *pkt, uint32 reasons)
{
    uint32 cos_max = 0, cos = 0;
#ifdef BCM_TB_SUPPORT
    uint32 temp1 = 0, temp2 = 0;
#endif

    pkt->rx_reason = 0;
    BCM_RX_REASON_CLEAR_ALL(pkt->rx_reasons);
    
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
        SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if (reasons & BCM_ROBO_RX_REASON_MIRROR) {
            pkt->flags |= BCM_RX_MIRRORED;
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_MIRRORING, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & BCM_ROBO_RX_REASON_SW_LEARN) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonCpuLearn);
            pkt->rx_reason = bcmRxReasonCpuLearn;
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_SA_LEARNING, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & BCM_ROBO_RX_REASON_SWITCHING) {
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_SWITCHING, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & BCM_ROBO_RX_REASON_PROTOCOL_TERMINATION) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonProtocol);
            pkt->rx_reason = bcmRxReasonControl;
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_PROTO_TERM, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & BCM_ROBO_RX_REASON_PROTOCOL_SNOOP) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonProtocol);
            pkt->rx_reason = bcmRxReasonProtocol;
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_PROTO_SNOOP, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & BCM_ROBO_RX_REASON_EXCEPTION_FLOOD) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonExceptionFlood);
            pkt->rx_reason = bcmRxReasonExceptionFlood;
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_EXCEPTION, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }

        pkt->cos = cos_max;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (reasons & _BCM_ROBO_TB_RX_REASON_CFP_CPU_COPY) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonFilterMatch);
            pkt->rx_reason = bcmRxReasonFilterMatch;
            /* No cosq number for CFP generated CPU copy action */
        }
        if (reasons & _BCM_ROBO_TB_RX_REASON_ARL_MASK) {
            switch (reasons & _BCM_ROBO_TB_RX_REASON_ARL_MASK) {
                case _BCM_ROBO_TB_RX_REASON_ARL_KNOWN_DA_FORWARD:
                    temp1 = bcmRxReasonL2Cpu;
                    temp2 = DRV_RX_REASON_SWITCHING;
                    break;
                case _BCM_ROBO_TB_RX_REASON_ARL_UNKNOWN_DA_FLOOD:
                    temp1 = bcmRxReasonL2DestMiss;
                    temp2 = DRV_RX_REASON_EXCEPTION;
                    break;
                case _BCM_ROBO_TB_RX_REASON_ARL_CONTROL_PKT:
                    temp1 = bcmRxReasonControl;
                    temp2 = DRV_RX_REASON_PROTO_TERM;
                    break;
                case _BCM_ROBO_TB_RX_REASON_ARL_APPL_PKT:
                    temp1 = bcmRxReasonProtocol;
                    temp2 = DRV_RX_REASON_PROTO_SNOOP;
                    break;
                case _BCM_ROBO_TB_RX_REASON_ARL_VLAN_FORWARD:
                    temp1 = bcmRxReasonEgressFilterRedirect;
                    temp2 = DRV_RX_REASON_ARL_VALN_DIR_FWD;
                    break;
                case _BCM_ROBO_TB_RX_REASON_ARL_CFP_FORWARD:
                    temp1 = bcmRxReasonFilterRedirect;
                    temp2 = DRV_RX_REASON_ARL_CFP_FWD;
                    break;
                case _BCM_ROBO_TB_RX_REASON_ARL_CPU_LOOPBACK:
                    temp1 = bcmRxReasonLoopback;
                    temp2 = DRV_RX_REASON_ARL_LOOPBACK;
                    break;
            }
            BCM_RX_REASON_SET(pkt->rx_reasons, temp1);
            pkt->rx_reason = temp1;
            DRV_QUEUE_RX_REASON_GET(unit, temp2, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & _BCM_ROBO_TB_RX_REASON_MIRROR_COPY) {
            pkt->flags |= BCM_RX_MIRRORED;
            DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_MIRRORING, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & _BCM_ROBO_TB_RX_REASON_INGRESS_SFLOW) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonSampleSource);
            pkt->rx_reason = bcmRxReasonSampleSource;
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_INGRESS_SFLOW, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & _BCM_ROBO_TB_RX_REASON_EGRESS_SFLOW) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonSampleDest);
            pkt->rx_reason = bcmRxReasonSampleDest;
            DRV_QUEUE_RX_REASON_GET
                (unit, DRV_RX_REASON_EGRESS_SFLOW, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & _BCM_ROBO_TB_RX_REASON_SA_MASK) {
            switch (reasons & _BCM_ROBO_TB_RX_REASON_SA_MASK) {
                case _BCM_ROBO_TB_RX_REASON_SA_MOVE:
                    temp1 = bcmRxReasonL2Move;
                    temp2 = DRV_RX_REASON_SA_MOVEMENT_EVENT;
                    break;
                case _BCM_ROBO_TB_RX_REASON_SA_UNKNOWN:
                    temp1 = bcmRxReasonL2SourceMiss;
                    temp2 = DRV_RX_REASON_SA_UNKNOWN_EVENT;
                    break;
                case _BCM_ROBO_TB_RX_REASON_SA_OVERLIMIT:
                    temp1 = bcmRxReasonL2LearnLimit;
                    temp2 = DRV_RX_REASON_SA_OVER_LIMIT_EVENT;
                    break;
            }
            BCM_RX_REASON_SET(pkt->rx_reasons, temp1);
            pkt->rx_reason = temp1;
            DRV_QUEUE_RX_REASON_GET(unit, temp2, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & _BCM_ROBO_TB_RX_REASON_VLAN_NONMEMBER) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonIngressFilter);
            pkt->rx_reason = bcmRxReasonIngressFilter;
            DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_INP_NON_MEMBER, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }
        if (reasons & _BCM_ROBO_TB_RX_REASON_VLAN_UNKNOWN) {
            BCM_RX_REASON_SET(pkt->rx_reasons, bcmRxReasonUnknownVlan);
            pkt->rx_reason = bcmRxReasonUnknownVlan;
            DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_VLAN_UNKNOWN, &cos);
            if (cos > cos_max) {
                cos_max=cos;
            }
        }

        pkt->cos = cos_max;
    }
#endif
}               
    
/*
 * Function:
 *      robo_rx_intr_process_packet
 * Purpose:
 *      Processes a received packet in interrupt context
 *      calling out to the handlers until
 *      the packet is consumed.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      dcb - pointer to completed DCB.
 * Returns:
 *      Nothing.
 * Notes:
 *      THIS ROUTINE IS CALLED AT INTERRUPT LEVEL.
 */

STATIC  void
robo_rx_intr_process_packet(int unit, eth_dv_t *dv, eth_dcb_t *dcb, bcm_pkt_t *pkt)
{
    volatile rx_callout_t *rco;
    bcm_rx_t              handler_rc;
    int                   chan;
    int                   idx;
    rx_queue_t           *queue;
    int                   handled = FALSE;
    uint32              rx_reasons = 0;
#ifdef BCM_TB_SUPPORT
    uint32              temp = 0;
#endif
    uint8 *vlan_ptr;

    idx = pkt->_idx;
    pkt->cos = 0;
    if (pkt->cos > BCM_RX_COS) {
        pkt->cos = 0;
    }
    chan = ROBO_DV_CHANNEL(dv);
    robo_pkt_len_get(unit, chan, (bcm_pkt_t *)pkt, dv);

    RX_CHAN_CTL(unit, chan).rpkt++;
    RX_CHAN_CTL(unit, chan).rbyte += pkt->tot_len; 

    /* Get data into packet structure */

    /* Copy the VID information (in pkt->pkt_data) to the pkt->vlan */
    vlan_ptr = 
        &pkt->pkt_data[idx].data[2*ENET_MAC_SIZE+ENET_TAG_SIZE-sizeof(pkt->vlan)];
    sal_memcpy(&pkt->vlan, vlan_ptr, sizeof(pkt->vlan));
    pkt->vlan = bcm_ntohs(pkt->vlan);
    pkt->vlan_pri = BCM_VLAN_CTRL_PRIO(pkt->vlan);
    pkt->vlan_cfi = BCM_VLAN_CTRL_CFI(pkt->vlan);
    pkt->vlan = BCM_VLAN_CTRL_ID(pkt->vlan);

    pkt->dma_channel = chan;
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        rx_reasons = dv->dv_reason_53242;
        _pkt_robo_reasons_get(unit, pkt, rx_reasons);
        pkt->cos = dv->dv_cosq_53242;
        pkt->rx_port = dv->dv_src_portid_53242;
    } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        rx_reasons = dv->dv_reason_53115;
        _pkt_robo_reasons_get(unit, pkt, rx_reasons);
        pkt->prio_int = dv->dv_imp_egress_tc_53115;
        pkt->rx_port = dv->dv_src_portid_53115;
        if (dv->dv_opcode == BRCM_OP_EGR_TS) {
            if (SOC_ROBO_DV_TS_TX_EVENT_53115(dv)) {
                /* 
                 * This packets send out by witch itself to report the egress time
                 */
                pkt->flags |= BCM_TX_TIME_STAMP_REPORT;
            } else {
                /* Receive a Time sync packets */
                pkt->flags |= BCM_PKT_F_TIMESYNC;
            }
            pkt->rx_timestamp = dv->rx_timestamp;
        }
    } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        rx_reasons = dv->dv_reason_53280;
        _pkt_robo_reasons_get(unit, pkt, rx_reasons);
        DRV_QUEUE_QOS_CONTROL_GET(unit, -1, DRV_QOS_CTL_USE_TC, &temp);
        if (temp) {
            /* Use TC2COSQ mapping result as the cosq number to cpu */
            DRV_QUEUE_PRIO_GET(unit, -1, dv->dv_tc_53280, (uint8 *)&temp);
            pkt->cos = temp;
        } 
        pkt->opcode = dv->dv_opcode_53280;
        pkt->prio_int = dv->dv_tc_53280;
        pkt->rx_port = dv->dv_port_id_53280;
        pkt->vlan = dv->dv_egr_vid_53280;

        if (dv->dv_dp_53280 == 0) {
            pkt->color = bcmColorPreserve; 
        } else if (dv->dv_dp_53280 == 1) {
            pkt->color = bcmColorGreen; 
        } else if (dv->dv_dp_53280 == 2) {
            pkt->color = bcmColorYellow; 
        } else {
            pkt->color = bcmColorRed; 
        }

        BCM_GPORT_SUBPORT_PORT_SET(pkt->src_gport, dv->dv_vport_id_53280);
        if (dv->dv_lrn_dis_53280) {
            pkt->flags |= BCM_RX_LEARN_DISABLED;
        } else {
            pkt->flags &= ~BCM_RX_LEARN_DISABLED;
        }
        if (dv->dv_fm_sel_53280) {
            pkt->flow_id = -1;
            pkt->multicast_group = dv->dv_fm_id_53280;
        } else {
            pkt->flow_id = dv->dv_fm_id_53280;
            pkt->multicast_group = -1;
        }
#endif
    } else {
        pkt->rx_reason = dv->dv_opcode;
        pkt->rx_port = dv->dv_src_portid;
    }

    /* Check if bandwidth available for this pkt */
    queue = RX_QUEUE(unit, pkt->cos);  

    
#if 0
    if (BCM_PKT_RX_VLAN_TAG_STRIP(pkt)) {
        /* VLAN tag was DMA'd to DV buffer; copy to pkt->_vtag */
        sal_memcpy(pkt->_vtag, SOC_ROBO_DV_VLAN_TAG(dv, idx), sizeof(uint32));
    }
#endif

    if (!RX_INIT_DONE(pkt->rx_unit)) {
        ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
        return;
    }

    if (INTR_CALLOUTS(pkt->rx_unit)) {

        /* Check if bandwidth available for this pkt */
        if (queue->pps > 0 && queue->tokens <= 0) {
            /* Rate limiting on and not enough tokens */
            queue->rate_disc++;
            ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
            return;
        }
        
        /* Loop through registered drivers looking for intr handlers. */
        for (rco = rx_ctl[pkt->rx_unit]->rc_callout; rco; rco = rco->rco_next) {
            if (!(rco->rco_flags & BCM_RCO_F_INTR)) {
                continue;
            }
             if (!(SHR_BITGET(rco->rco_cos, pkt->cos))) {
                /* callback is not interested in this COS */
                continue;
            }

            handler_rc = rco->rco_function(pkt->rx_unit, pkt, rco->rco_cookie);
            switch (handler_rc) {
            case BCM_RX_NOT_HANDLED:
                break;                      /* Next callout */
            case BCM_RX_HANDLED:            /* Account for the packet */
                _Q_DEC_TOKENS(queue);
                ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
                handled = TRUE;
                rco->rco_pkts_handled++;
                break;
            case BCM_RX_HANDLED_OWNED:      /* Packet stolen */
                _Q_DEC_TOKENS(queue);
                ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
                pkt->alloc_ptr = NULL;  /* Mark as stolen */
                pkt->_pkt_data.data = NULL;  /* Mark as stolen */
                handled = TRUE;
                rx_ctl[unit]->pkts_owned++;
                rco->rco_pkts_owned++;
                break;
            default: /* Treated as NOT_HANDLED */
                rx_ctl[unit]->bad_hndlr_rv++;
                break;
            }
        }
    }

    if (!handled) {  /* Not processed, enqueue for non-interrupt handling. */
        if (queue->max_len && queue->count < queue->max_len) {
            if (NON_INTR_CALLOUTS(pkt->rx_unit)) {
                _Q_ENQUEUE_LOCK(queue, pkt);
                ROBO_RX_THREAD_NOTIFY(unit);
            } else {
                ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
                rx_ctl[unit]->no_hndlr++;
            }
        } else { /* No space; discard */
            ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
            queue->rate_disc++;
        }
    } 
}

/*
 * Function:
 *      robo_rx_process_packet
 * Purpose:
 *      Processes a received packet in non-interrupt context
 *      calling out to the handlers until the packet is consumed.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      rx_pkt - The packet to process
 * Returns:
 *      Nothing.
 * Notes:
 *      Assumes discard handler is registered with lowest priority
 */

STATIC  void
robo_rx_process_packet(int unit, bcm_pkt_t *pkt)
{
    volatile rx_callout_t *rco;
    bcm_rx_t              handler_rc;
    eth_dv_t                 *dv;
    int                   chan;
    int                   handled = FALSE;
    assert(pkt);

    dv = pkt->_dv;
    chan = pkt->dma_channel;
    assert(pkt == ROBO_DV_PKT(dv, pkt->_idx));

#ifdef  BROADCOM_DEBUG
    if (bsl_check(bslLayerBcm, bslSourcePacket, bslSeverityNormal, unit)) {
        /* Dump the packet info */
        LOG_INFO(BSL_LS_BCM_PACKET,
                 (BSL_META_U(unit,
                             "robo_rx_process_packet: packet in\n")));
    /* Mask for complier ok */
        if (bsl_check(bslLayerBcm, bslSourceDma, bslSeverityVerbose, unit)) {
            soc_eth_dma_dump_dv(unit, "rx dv: ", dv);
        }
    }
#endif  /* BROADCOM_DEBUG */

    if (!RX_INIT_DONE(pkt->rx_unit)) {
        ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
        return;
    }

    if (!NON_INTR_CALLOUTS(unit)) {
        /* No one to talk to.  Mark processed and return */
        ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
        rx_ctl[unit]->no_hndlr++;
        return;
    }

    RX_LOCK(unit);  /* Can't modify handler list while doing callbacks */
    /* Loop through registered drivers until packet consumed */
    for (rco = rx_ctl[pkt->rx_unit]->rc_callout; rco; rco = rco->rco_next) {
        if (rco->rco_flags & BCM_RCO_F_INTR) { /* Non interrupt context */
            continue;
        }

        handler_rc = rco->rco_function(pkt->rx_unit, pkt, rco->rco_cookie);

        switch (handler_rc) {
        case BCM_RX_NOT_HANDLED:
            break;                      /* Next callout */
        case BCM_RX_HANDLED:
            handled = TRUE;
            ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
            LOG_VERBOSE(BSL_LS_BCM_PACKET,
                        (BSL_META_U(unit,
                                    "rx: pkt handled by %s\n"),
                         rco->rco_name));
            rco->rco_pkts_handled++;
            break;
        case BCM_RX_HANDLED_OWNED:
            handled = TRUE;
            ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
            LOG_VERBOSE(BSL_LS_BCM_PACKET,
                        (BSL_META_U(unit,
                                    "rx: pkt owned by %s\n"),
                         rco->rco_name));
            pkt->alloc_ptr = NULL;  /* Mark as stolen */
            pkt->_pkt_data.data = NULL;
            rx_ctl[unit]->pkts_owned++;
            rco->rco_pkts_owned++;
            break;
        default:
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "robo_rx_process_packet: unit=%d: "
                                 "Invalid callback return value=%d\n"), unit, handler_rc));
            break;
        }

        if (handled) {
            break;
        }
    }
    RX_UNLOCK(unit);

    if (!handled) {
        /* Internal error as discard should have handled packet */
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "bcm_rx_process_packet: No handler processed packet: "
                              "Unit %d Port %s\n"),
                   unit, SOC_PORT_NAME(pkt->rx_unit, pkt->rx_port)));
        ROBO_MARK_PKT_PROCESSED(unit, chan, dv);
    }
}

/****************************************************************/


/****************************************************************
 *
 * Setup routines
 *
 ****************************************************************/

/*
 * Function:
 *      robo_rx_channel_dv_setup
 * Purpose:
 *      Set up DVs and packets for a channel according to current config
 * Parameters:
 *      unit, chan - what's being configured
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This is only called during the init (bcm_rx_start)
 */

STATIC int
robo_rx_channel_dv_setup(int unit, int chan)
{
    int i, j;
    eth_dv_t *dv;
    bcm_pkt_t *all_pkts;
    bcm_pkt_t *pkt;

    /* Allocate the packet structures we need for the DVs */
    if (RX_CHAN_PKTS(unit, chan) == NULL) {
        all_pkts = sal_alloc(sizeof(bcm_pkt_t) *
                             RX_CHAN_PKT_COUNT(unit, chan), "rx_pkts");
        if (all_pkts == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(all_pkts, 0, sizeof(bcm_pkt_t) * RX_PPC(unit) *
                   RX_CHAINS(unit, chan));
        RX_CHAN_PKTS(unit, chan) = all_pkts;
    }

    /* Set up packets and allocate DVs */
    for (i = 0; i < RX_CHAINS(unit, chan); i++) {
        if ((dv = robo_rx_dv_alloc(unit, chan, i)) == NULL) {
            for (j = 0; j < i; j++) {
                robo_rx_dv_dealloc(unit, chan, j);
                ROBO_RX_DV(unit, chan, i) = NULL;
            }
            sal_free(RX_CHAN_PKTS(unit, chan));
            RX_CHAN_PKTS(unit, chan) = NULL;
            return BCM_E_MEMORY;
        }
        for (j = 0; j < RX_PPC(unit); j++) {
            pkt = RX_PKT(unit, chan, i, j);
            pkt->_idx = j;
            pkt->_dv = dv;
            pkt->unit = unit;
            pkt->pkt_data = &pkt->_pkt_data;
            pkt->blk_count = 1;
        }

        ROBO_DV_STATE(dv) = DV_S_NEEDS_FILL;
        ROBO_RX_DV(unit, chan, i) = dv;
    }

    soc_eth_dma_occupancy_set(chan, unit);

    return BCM_E_NONE;
}               
    

STATIC int
robo_rx_discard_handler_setup(int unit, rx_ctl_t *rx_ctrl_ptr) 
{
    volatile rx_callout_t *rco;
    int i;

    if ((rco = sal_alloc(sizeof(*rco), "rx_callout")) == NULL) {
        return BCM_E_MEMORY;
    }
    SETUP_RCO(rco, "Discard", robo_rx_discard_packet, BCM_RX_PRIO_MIN, NULL, NULL,
              BCM_RCO_F_ALL_COS);   /* Accept any cos; non-interrupt */
    for (i = 0; i <= rx_ctrl_ptr->queue_max; i++) {
        SETUP_RCO_COS_SET(rco, i);
    }
    rx_ctrl_ptr->rc_callout = rco;

    return BCM_E_NONE;
}               
    
/*
 * Check the channel configuration passed in by the user for legal values
 * The burst must be at least pkts/chain; otherwise, can never
 * start packet
 */

STATIC void
robo_rx_user_cfg_check(int unit)
{
    int chan;
    int chan_count = 0;
    bcm_rx_cfg_t *cfg;
    int cos;
    rx_queue_t *queue;

    cfg = &rx_ctl[unit]->user_cfg;

    FOREACH_SETUP_CHANNEL(unit, chan) {
        if (RX_CHAINS(unit, chan) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "rx_config %d %d: Warning: chains < 0."),
                      unit, chan));
            RX_CHAINS(unit, chan) = 0;
        } else {
            chan_count++;
            if (RX_CHAINS(unit, chan) > ROBO_RX_CHAINS_MAX) {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "rx_config %d %d: Warning: "
                                     "Bad chain cnt %d.  Now %d.\n"),
                          unit, chan, RX_CHAINS(unit, chan), ROBO_RX_CHAINS_MAX));
                RX_CHAINS(unit, chan) = ROBO_RX_CHAINS_MAX;
            }
        }
    }

    if (RX_PPS(unit) < 0) {
        RX_PPS(unit) = 0;
    }  

    for (cos = 0; cos <= RX_QUEUE_MAX(unit); cos++) {
        queue = RX_QUEUE(unit, cos);
        if (queue->pps < 0) {
            queue->pps = 0;
        }
        if (queue->max_len < 0) {
            queue->max_len = 0;
        }
    }

    if (cfg->pkts_per_chain <= 0 ||
            cfg->pkts_per_chain > RX_PPC_MAX) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "rx_config: Warning: bad pkts/chn %d. "
                             "Now %d.\n"), cfg->pkts_per_chain, ROBO_RX_PPC_DFLT));
        cfg->pkts_per_chain = ROBO_RX_PPC_DFLT;
    }
}

/****************************************************************
 *
 * Tear down routines
 *
 ****************************************************************/

/*
 * Function:
 *      robo_rx_channel_shutdown
 * Purpose:
 *      Shutdown a channel, deallocating DVs and packets
 * Parameters:
 *      unit, chan - what's being configured
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This is only called when thread is brought down (bcm_rx_stop)
 *      or on an error
 */

STATIC void
robo_rx_channel_shutdown(int unit, int chan)
{
    int i;
    bcm_pkt_t *pkt;

    rx_ctl[unit]->chan_running &= ~(1 << chan);
    if (RX_CHAN_USED(unit, chan)) {
        /* Deallocate all packets */
        if (RX_CHAN_PKTS(unit, chan) != NULL) {
            /* Free any buffers held by packets */
            for (i = 0; i < RX_CHAN_PKT_COUNT(unit, chan); i++) {
                pkt = &RX_CHAN_PKTS(unit, chan)[i];
                if (pkt->alloc_ptr) {
                    bcm_rx_free(unit, pkt->alloc_ptr);
                    pkt->_pkt_data.data = NULL;
                    pkt->alloc_ptr = NULL;
                }
            }
            sal_free(RX_CHAN_PKTS(unit, chan));
            RX_CHAN_PKTS(unit, chan) = NULL;
        }
        /* Deallocate the DVs */
        for (i = 0; i < RX_CHAINS(unit, chan); i++) {
            robo_rx_dv_dealloc(unit, chan, i);
        }
        soc_eth_dma_occupancy_set(chan, -1);
    }
}

/****************************************************************
 *
 * Info/calculation routines
 *
 ****************************************************************/

/*
 * Calculate number of DCBs per packet based on system cfg.
 * The MACs are always set up separately.  The VLAN is skipped
 * on Hercules.
 *
 * The following always get a DCB:
 *    HiGig header
 *    SL tag
 *    Everything past the SL tag
 *
 * If the NO_VTAG flag is given, then an extra DCB is needed for that.
 * In that case, we also need a DCB for the MAC addresses.
 *
 */

STATIC void
robo_rx_dcb_per_pkt(int unit)
{
    int chan;
    int dcb = 1;

    for (chan = 0; chan < BCM_ROBO_RX_CHANNELS; chan++) {
        RX_CHAN_CTL(unit, chan).dcb_per_pkt = dcb;
    }
}

/****************************************************************
 *
 * Channel enable/disable
 *
 ****************************************************************/


/****************************************************************
 *
 * DV manipulation functions
 *     robo_rx_dv_add_pkt     Add a packet to a DV
 *     robo_rx_dv_fill        Prepare a DV to be sent to the SOC layer
 *     robo_rx_dv_alloc       Allocate an RX DV
 *     robo_rx_dv_dealloc     Free an RX DV
 *
 ****************************************************************/

/*
 * Set up a packet in a DV.  Use packet scatter for parts of packet
 */

STATIC int
robo_rx_dv_add_pkt(int unit, volatile bcm_pkt_t *pkt, int idx, eth_dv_t *dv)
{
    /* Setup DCB for the Packet, including ETHER Header+Payload+CRC:
     * Always into packet
     */
    SOC_IF_ERROR_RETURN(soc_eth_dma_desc_add(dv, (sal_vaddr_t)pkt->alloc_ptr,
                                         pkt->_pkt_data.len));

    return BCM_E_NONE;
}


/*
 * Function:
 *      robo_rx_dv_fill
 * Purpose:
 *      Try to fill a DV with any packets it needs
 * Parameters:
 *      unit
 *      chan
 *      dv
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If successful changes the state of the DV.
 *      If memory error occurs, the state of the DV is not
 *      changed, but an error is not returned.
 */

STATIC void
robo_rx_dv_fill(int unit, int chan, eth_dv_t *dv)
{
    int         i;
    volatile bcm_pkt_t   *pkt;
    rx_dv_info_t   *save_info;
    static int warned = 0;
    void *pkt_data;
    int rv;
    uint32    hwrxoff = ETC47XX_HWRXOFF;

    save_info = ROBO_DV_INFO(dv);  /* Save info before resetting dv */
    soc_eth_dma_dv_reset(DV_RX, dv);
    dv->_DV_INFO = save_info;  /* Restore info to DV */
    pkt_data = NULL;
    assert(ROBO_DV_STATE(dv) == DV_S_NEEDS_FILL);

    for (i = 0; i < RX_PPC(unit); i++) {
        pkt = ROBO_DV_PKT(dv, i);
        if (pkt->_pkt_data.data == NULL) {
            /* No pkt buffer; it needs to be allocated; use dflt size */
            bcm_rx_alloc(unit, -1, RX_CHAN_FLAGS(unit, chan), &pkt_data);
            if (pkt_data == NULL) {/* Failed to allocate a pkt for this DV. */
                if (!warned) {
                    warned = 1;
                    LOG_WARN(BSL_LS_BCM_COMMON,
                             (BSL_META_U(unit,
                                         "RX: Failed to allocate mem\n")));
                }
                RX_CHAN_CTL(unit, chan).mem_fail++;
                return; /* Not an error, try again later */
            }
            pkt->alloc_ptr = pkt_data;

            soc_eth_dma_hwrxoff_get(unit, &hwrxoff);
            /* No BRCM types (2 bytes) for BCM53115, BCM53118 */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                pkt->_pkt_data.data = (void *)((uint8 *)pkt_data + hwrxoff +
                                   ENET_BRCM_SHORT_TAG_SIZE);
            } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                pkt->_pkt_data.data = (void *)((uint8 *)pkt_data + hwrxoff +
                                   ENET_BRCM_SHORT_TAG_SIZE*2);
#endif
            } else {
                pkt->_pkt_data.data = (void *)((uint8 *)pkt_data + hwrxoff +
                                   ENET_BRCM_TAG_SIZE);
            }
            pkt->_pkt_data.len = rx_ctl[unit]->user_cfg.pkt_size;
        }
        
        /* Set up CRC stripping */
        if (RX_CHAN_FLAGS(unit, chan) & BCM_RX_F_CRC_STRIP) {
            pkt->flags |= BCM_RX_CRC_STRIP;
        } else {
            pkt->flags &= ~BCM_RX_CRC_STRIP;
        }
        
        /* Set up the packet in the DCBs of the DV */
        if ((rv = robo_rx_dv_add_pkt(unit, pkt, i, dv)) < 0) {
            ROBO_DV_STATE(dv) = DV_S_ERROR;
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Failed to add pkt %d to dv on unit %d: %s\n"),
                      i, unit, bcm_errmsg(rv)));
            break;
        }
    }

    if (ROBO_DV_STATE(dv) != DV_S_ERROR) { /* Mark as ready to be started */
        ROBO_DV_STATE(dv) = DV_S_FILLED;
        ROBO_DV_PKTS_PROCESSED(dv) = 0;
    }

    return;
}               
    

/* Allocate and clear a DV (DMA chain control unit) */
STATIC eth_dv_t *
robo_rx_dv_alloc(int unit, int chan, int dv_idx)
{
    eth_dv_t *dv;
    rx_dv_info_t *dv_info;
    int clr_len;

    if ((dv_info = sal_alloc(sizeof(rx_dv_info_t), "dv_info")) == NULL) {
        return NULL;
    }
    sal_memset(dv_info, 0, sizeof(rx_dv_info_t));

    if ((dv = soc_eth_dma_dv_alloc(unit, DV_RX, RX_DCB_PER_DV(unit, chan)))
                                  == NULL) {
        sal_free(dv_info);
        return NULL;
    }

    /* Initialize dv member */
    dv->dv_next = NULL;

    /* DCBs MUST start off 0 */
    clr_len = sizeof(eth_dcb_t) * RX_DCB_PER_DV(unit, chan);
    sal_memset(dv->dv_dcb, 0, clr_len);

    /* Set up and install the DV and its info structure */
    dv->dv_done_chain = NULL;
    dv->dv_done_packet  = robo_rx_done_packet;

    dv_info->idx = dv_idx;
    dv_info->chan = chan;
    dv_info->state = DV_S_NEEDS_FILL;
    dv->_DV_INFO = dv_info;
    dv->dv_channel = chan;
    DV_RX_IDX_SET(dv, rx_dv_count++);  /* Debug: Indicate DVs index */

    return dv;
}

/* Free a DV and any packets it controls */
STATIC void
robo_rx_dv_dealloc(int unit, int chan, int dv_idx)
{
    eth_dv_t *dv;

    dv = ROBO_RX_DV(unit, chan, dv_idx);

    if (dv) {
        ROBO_RX_DV(unit, chan, dv_idx) = NULL;
        sal_free(ROBO_DV_INFO(dv));
        soc_eth_dma_dv_free(unit, dv);
    }
}


/****************************************************************
 *
 * Rate limitting code
 *
 ****************************************************************/

/*
 * Function:
 *      robo_rx_init_all_tokens
 * Purpose:
 *      Initialize all RX token buckets
 * Parameters:
 *      unit - Robo unit number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Includes top level limit, per channel limits and per cos limits.
 */

STATIC void
robo_rx_init_all_tokens(int unit)
{
    int cos;
    sal_usecs_t cur_time;
    rx_queue_t *queue;

    cur_time = sal_time_usecs();

    RX_TOKENS(unit) = RX_BURST(unit);
    rx_ctl[unit]->last_fill = cur_time;

    last_fill_check = sal_time_usecs();
    for (cos = 0; cos <= RX_QUEUE_MAX(unit); cos++) {
        queue = RX_QUEUE(unit, cos);
        queue->tokens = queue->burst;
        queue->last_fill = cur_time;
    }

    robo_rx_control.system_tokens = robo_rx_control.system_pps;
    robo_rx_control.system_last_fill = cur_time;
}


/* Initialize the queues. */
STATIC void
robo_rx_queue_init(int unit, rx_ctl_t *rx_ctrl_ptr)
{
    int cos;
    rx_queue_t *queue;
    
    for (cos = 0; cos <= rx_ctrl_ptr->queue_max; cos++) {
        queue = rx_ctrl_ptr->pkt_queue + cos;
        queue->head = NULL;
        queue->tail = NULL;
        queue->count = 0;
        queue->max_len = ROBO_RX_Q_MAX_DFLT;
    }
}               
    
/****************************************************************
 *
 * Other functions accessed through RX subsystem:
 *     Discard packet handler:  Always registered first
 *     Default alloc/free routines:  Provided for default config
 *
 ****************************************************************/

/*
 * Function:
 *      robo_rx_discard_packet
 * Purpose:
 *      Lowest priority registered handler that discards packet.
 * Parameters:
 *      unit - RoboSwitch Unit number.
 *      pkt - The packet to handle
 *      cookie - (Not used)
 * Returns:
 *      bcm_rx_handled
 */

STATIC bcm_rx_t
robo_rx_discard_packet(int unit, bcm_pkt_t *pkt, void *cookie)
{
    int chan;

    COMPILER_REFERENCE(cookie);
    chan = pkt->dma_channel;

    ++(rx_ctl[unit]->chan_ctl[chan].dpkt);
    rx_ctl[unit]->chan_ctl[chan].dbyte += pkt->tot_len;

    return(BCM_RX_HANDLED);
}               
    
#if defined(BROADCOM_DEBUG)

static char *dv_state_names[] = DV_STATE_STRINGS;

/*
 * Function:
 *      bcm_rx_show
 * Purpose:
 *      Show RX information for the specified device.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      Nothing.
 */

int
bcm_robo_rx_show(int unit)
{
    volatile rx_callout_t      *rco;
    int chan;
    eth_dv_t *dv;
    int i;
    sal_usecs_t cur_time;
    rx_queue_t *queue;

    if (!RX_INIT_DONE(unit)) {
        LOG_CLI((BSL_META_U(unit,
                            "Initializing RX subsystem (%d)\n"),
                 bcm_rx_init(unit)));
    }

    cur_time = sal_time_usecs();
    RX_PRINT(("RX Info @ time=%u: %sstarted. Last fill %u. Thread is %srunning.\n"
                 "    +verbose for more info\n",
              cur_time,
              rx_units_started & (1 << unit) ? "" : "not ",
              last_fill_check,
              robo_rx_control.thread_running ? "" : "not "));

    LOG_CLI((BSL_META_U(unit,
                        "    Pkt Size %d. Pkts/Chain %d. Pkts/sec %d. Burst %d.\n"),
             RX_PKT_SIZE(unit),
             RX_PPC(unit), RX_PPS(unit), RX_BURST(unit)));
    LOG_CLI((BSL_META_U(unit,
                        "    Cntrs:  Pkts %d. Last start %d. Bad Hndlr %d.\n"
                        "        No Hndlr %d. Rstr Err %d. Not Running %d.\n"
                        "        Thrd Not Running %d\n"),
             rx_ctl[unit]->tot_pkts,
             rx_ctl[unit]->pkts_since_start,
             rx_ctl[unit]->bad_hndlr_rv,
             rx_ctl[unit]->no_hndlr,
             rx_ctl[unit]->restart_errs,
             rx_ctl[unit]->not_running,
             rx_ctl[unit]->thrd_not_running));
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "    Tokens %d. Sleep %d.\n"
                            "    Thread: %p. run %d. exitted %d. pri %d.\n"),
                 RX_TOKENS(unit),
                 robo_rx_control.sleep_cur,
                 (void *)robo_rx_control.rx_tid,
                 robo_rx_control.thread_running,
                 robo_rx_control.thread_exit_complete,
                 soc_property_get(unit,
                 spn_BCM_RX_THREAD_PRI,
                 RX_THREAD_PRI_DFLT)));

    LOG_CLI((BSL_META_U(unit,
                        "  Registered callbacks:\n")));
    /* Display callouts and priority in order */
    for (rco = rx_ctl[unit]->rc_callout; rco; rco = rco->rco_next) {
        RX_PRINT(("        %-10s Priority=%3d%s. "
                     "Argument=0x%x. COS 0x%x.\n",
                     rco->rco_name, (uint32)rco->rco_priority,
                     (rco->rco_flags & BCM_RCO_F_INTR) ? " Interrupt" : "",
                     PTR_TO_INT(rco->rco_cookie),
                     rco->rco_flags & BCM_RCO_F_COS_ACCEPT_MASK));
        RX_PRINT(("        %10s Packets handled %u, owned %u.\n", " ",
                  rco->rco_pkts_handled,
                  rco->rco_pkts_owned));
    }

    LOG_CLI((BSL_META_U(unit,
                        "  Channel Info\n")));

    FOREACH_SETUP_CHANNEL(unit, chan) {
        LOG_CLI((BSL_META_U(unit,
                            "    Chan %d is %s: Chains %d. COS 0x%x. DCB/pkt %d\n"),
                 chan,
                 RX_CHAN_RUNNING(unit, chan) ? "running" : "setup",
                 RX_CHAINS(unit, chan), 
                 RX_CHAN_CFG(unit, chan).cos_bmp,
                 RX_CHAN_CTL(unit, chan).dcb_per_pkt));
        LOG_CLI((BSL_META_U(unit,
                            "        rpkt %d. rbyte %d. dpkt %d. dbyte %d. "
                            "mem fail %d.\n"), 
                 RX_CHAN_CTL(unit, chan).rpkt, RX_CHAN_CTL(unit, chan).rbyte,
                 RX_CHAN_CTL(unit, chan).dpkt, RX_CHAN_CTL(unit, chan).dbyte,
                 RX_CHAN_CTL(unit, chan).mem_fail));

        /* Display all DVs allocated */
        if (bsl_check(bslLayerBcm, bslSourceCommon, bslSeverityVerbose, unit)) {
            for (i = 0; i < RX_CHAINS(unit, chan); i++) {
                dv = ROBO_RX_DV(unit, chan, i);
                if (dv == NULL || !soc_eth_dma_dv_valid(dv)) {
                    LOG_CLI((BSL_META_U(unit,
                                        "        DV %d (%p) is not valid\n"), i, (void *)dv));
                } else {
                    LOG_CLI((BSL_META_U(unit,
                                        "        DV %d (%d, %p) %s. chan %d. "
                                        "pkt cnt %d. pkt[%d] %p\n"),
                             i, ROBO_DV_INDEX(dv), (void *)dv,
                             dv_state_names[ROBO_DV_STATE(dv)],
                             ROBO_DV_CHANNEL(dv),
                             ROBO_DV_PKTS_PROCESSED(dv),
                             ROBO_DV_PKTS_PROCESSED(dv),
                             (void *)ROBO_DV_PKT(dv, ROBO_DV_PKTS_PROCESSED(dv))));
                    if (ROBO_DV_STATE(dv) == DV_S_SCHEDULED) {
                        LOG_CLI((BSL_META_U(unit,
                                            "            Sched at %u for %d; "
                                            "future %d\n"), DV_SCHED_TIME(dv),
                                 DV_TIME_DIFF(dv),
                                 DV_FUTURE_US(dv, cur_time)));
                    }
                }
            }
        }
    }

    LOG_CLI((BSL_META_U(unit,
                        "  Queue Info\n")));

    /* Display queue info */
    for (i = 0; i <= RX_QUEUE_MAX(unit); i++) {
        queue = RX_QUEUE(unit, i);
        LOG_CLI((BSL_META_U(unit,
                            "    Queue %d: PPS %d. CurPkts %d. TotPkts %d. "
                            "Disc %d.\n"),
                 i, queue->pps,
                 queue->count,
                 queue->tot_pkts,
                 queue->rate_disc));
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "        Tokens %d. Last_fill %u. Max %d. "
                                "Brst %d. Head %p. Tail %p.\n"),
                     queue->tokens,
                     queue->last_fill,
                     queue->max_len,
                     queue->burst,
                     (void *)queue->head,
                     (void *)queue->tail));
    }
    return BCM_E_NONE;
}               
    
#endif  /* BROADCOM_DEBUG */


/****************************************************************
 * DEPRECATED FUNCTIONS
 *
 * Channel rates and COS weights have been DEPRECATED
 * Channel enable/disable have been DEPRECATED
 *     Channels go/stop on bcm_rx_start/stop
 *
 ****************************************************************/

/*
 * Function:
 *      bcm_rx_chan_rate_set
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit - Unit reference
 *      pps - packets per second to accept
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_rx_chan_rate_set(int unit, int chan, int pps)
{
    return BCM_E_NONE;
}               
    

/*
 * Function:
 *      bcm_rx_chan_rate_get
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit - Unit reference
 *      pps - (OUT) Packets per second, current setting
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_rx_chan_rate_get(int unit, int chan, int *pps)
{
    return BCM_E_NONE;
}               
    

/*
 * Function:
 *      bcm_rx_cos_wt_set
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit - Unit reference
 *      cos_wt - Pointer to an array of COS weights
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_rx_cos_wt_set(int unit, int *cos_wt)
{
    if (!cos_wt) {
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}               
    

/*
 * Function:
 *      bcm_rx_cos_wt_get
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit - Unit reference
 *      cos_wt - Pointer to an array of COS weights
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_rx_cos_wt_get(int unit, int *cos_wt)
{
    if (!cos_wt) {
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}               
    

/*
 * Function:
 *      bcm_rx_free_dpc
 * Purpose:
 *      DEPRECATED; still queues packet to be freed
 * Parameters:
 *      unit - Unit reference
 *      pkt - Packet to free
 * Returns:
 * Notes:
 *      Queues the packet to be freed using bcm_rx_free_enqueue
 */

void
bcm_rx_free_dpc(void *p_unit, void *pkt_p, void *p3, void *p4, void *p5)
{
    bcm_rx_free_enqueue(PTR_TO_INT(p_unit), pkt_p);
}

/*
 * Function:
 *      bcm_rx_channels_all_enable
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit       - Which unit to operate on
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      All channels which are set up will be enabled on start
 *      and disabled on stop
 *      This call will not have any effect.
 */

int
bcm_rx_channels_all_enable(int unit)
{
    COMPILER_REFERENCE(unit);

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *      bcm_rx_channels_enable
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit       - Which unit to operate on
 *      channels   - Which channels to start (bitmap)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      All channels which are set up will be enabled on start
 *      and disabled on stop
 *      This call will not have any effect.
 */

int
bcm_rx_channels_enable(int unit, uint32 channels)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(channels);

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_rx_channels_all_disable
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit       - Which unit to operate on
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      All channels which are set up will be enabled on start
 *      and disabled on stop
 *      This call will not have any effect.
 */

int
bcm_rx_channels_all_disable(int unit)
{
    COMPILER_REFERENCE(unit);

    return BCM_E_NONE;
}               
    

/*
 * Function:
 *      bcm_rx_channels_disable
 * Purpose:
 *      DEPRECATED
 * Parameters:
 *      unit       - Which unit to operate on
 *      channels   - Which channels to stop (bitmap)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      All channels which are set up will be enabled on start
 *      and disabled on stop
 *      This call will not have any effect.
 */

int
bcm_rx_channels_disable(int unit, uint32 channels)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(channels);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_rx_cosq_mapping_set
 * Purpose:
 *      Set the COSQ mapping to map qualified packets to the a CPU cos queue.
 * Parameters:
 *      unit - Unit reference
 *      index - Index into COSQ mapping table (0 is lowest match priority)
 *      reasons - packet "reasons" bitmap
 *      reasons_mask - mask for packet "reasons" bitmap
 *      int_prio - internal priority value
 *      int_prio_mask - mask for internal priority value
 *      packet_type - packet type bitmap (BCM_RX_COSQ_PACKET_TYPE_*)
 *      packet_type_mask - mask for packet type bitmap
 *      cosq - CPU cos queue
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_cosq_mapping_set(int unit, int index,
 bcm_rx_reasons_t reasons, bcm_rx_reasons_t reasons_mask,
                            uint8 int_prio, uint8 int_prio_mask,
                            uint32 packet_type, uint32 packet_type_mask,
                            bcm_cos_queue_t cosq)
{
    int cosq_num = 0;
    int rv = BCM_E_NONE;
    uint32 temp = 0;
    int idx;
    bcm_rx_reason_t  ridx;
    bcm_rx_reasons_t reasons_remain;
    bcm_rx_reasons_t reasons_set;
    uint32 sz = 0, bit = 0;

    /* Verify COSQ */
    cosq_num = NUM_CPU_COSQ(unit);

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
        SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit)) {
        if (cosq >= cosq_num) {
            return BCM_E_PARAM;
        }
    } else {
        return BCM_E_UNAVAIL;
    }

    /* Prio to cosq is not supported */
    if (int_prio_mask) {
        return BCM_E_PARAM;
    }

    /* Process packet type */
    if (packet_type_mask) {
        temp = packet_type & packet_type_mask;
        if (temp & BCM_RX_COSQ_PACKET_TYPE_SWITCHED) {
            rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_SWITCHING, cosq);
            BCM_IF_ERROR_RETURN(rv);
            temp &= ~BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
        }
        if (temp & BCM_RX_COSQ_PACKET_TYPE_MIRROR) {
            rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_MIRRORING, cosq);
            BCM_IF_ERROR_RETURN(rv);
            temp &= ~BCM_RX_COSQ_PACKET_TYPE_MIRROR;
        }

        /* check whether there are packet types unsupported */
        if (temp) {
            return BCM_E_PARAM;
        }
    }

    /* Process reason codes */
    reasons_remain = reasons_mask;
    memset(&reasons_set, 0, sizeof(bcm_rx_reasons_t));

    if (SOC_IS_TBX(unit)) {
        sz = _bcm_robo_tb_rx_cpu_cosmap_key_max;
        for (bit = 0; bit < sz; bit++) {
             /* Find reason being set */
             ridx  = _bcm_robo_tb_cpu_cos_map_key[bit];
             if (!BCM_RX_REASON_GET(reasons_mask, ridx)) {
                 continue;
             }

             if (BCM_RX_REASON_GET(reasons, ridx)) {
                 BCM_RX_REASON_SET(reasons_set, ridx);
             }

             /* clean the bit of reasons_remain */
             BCM_RX_REASON_CLEAR(reasons_remain, ridx);
        } /* for */
    } else {
        sz = _bcm_robo_rx_cpu_cosmap_key_max;
        for (bit = 0; bit < sz; bit++) {
             /* Find reason being set */
             ridx  = _bcm_robo_cpu_cos_map_key[bit];
             if (!BCM_RX_REASON_GET(reasons_mask, ridx)) {
                 continue;
             }
    
             if (BCM_RX_REASON_GET(reasons, ridx)) {
                 BCM_RX_REASON_SET(reasons_set, ridx);
             }

             /* clean the bit of reasons_remain */
             BCM_RX_REASON_CLEAR(reasons_remain, ridx);
        } /* for */
    }
        
    /* check whether there are reasons unsupported */
    for (idx = bcmRxReasonInvalid; idx < bcmRxReasonCount; idx++) {
         if (BCM_RX_REASON_GET(reasons_remain, idx)) {
             return BCM_E_PARAM;
         }
    }

    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonCpuLearn)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_SA_LEARNING, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonControl)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_PROTO_TERM, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonProtocol)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_PROTO_SNOOP, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonExceptionFlood)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_EXCEPTION, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonL2Cpu)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_SWITCHING, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonL2DestMiss)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_EXCEPTION, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonEgressFilterRedirect)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_ARL_VALN_DIR_FWD, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonFilterRedirect)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_ARL_CFP_FWD, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonLoopback)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_ARL_LOOPBACK, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonSampleSource)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_INGRESS_SFLOW, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonSampleDest)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_EGRESS_SFLOW, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonL2Move)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_SA_MOVEMENT_EVENT, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonL2SourceMiss)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_SA_UNKNOWN_EVENT, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonL2LearnLimit)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_SA_OVER_LIMIT_EVENT, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonIngressFilter)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_INP_NON_MEMBER, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (BCM_RX_REASON_GET(reasons_set, bcmRxReasonUnknownVlan)) {
        rv = DRV_QUEUE_RX_REASON_SET(unit, DRV_RX_REASON_VLAN_UNKNOWN, cosq);
        BCM_IF_ERROR_RETURN(rv);
    }

    return rv;
}

/*
 * Function:
 *      bcm_rx_cosq_mapping_get
 * Purpose:
 *      Get the COSQ mapping at the specified index
 * Parameters:
 *      unit - Unit reference
 *      index - Index into COSQ mapping table (0 is lowest match priority)
 *      reasons - packet "reasons" bitmap
 *      reasons_mask - mask for packet "reasons" bitmap
 *      int_prio - internal priority value
 *      int_prio_mask - mask for internal priority value
 *      packet_type - packet type bitmap (BCM_RX_COSQ_PACKET_TYPE_*)
 *      packet_type_mask - mask for packet type bitmap
 *      cosq - CPU cos queue
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_cosq_mapping_get(int unit, int index,
                            bcm_rx_reasons_t *reasons, bcm_rx_reasons_t *reasons_mask,
                            uint8 *int_prio, uint8 *int_prio_mask,
                            uint32 *packet_type, uint32 *packet_type_mask,
                            bcm_cos_queue_t *cosq)
{
    uint32 temp = 0, val = 0;
    int rv = BCM_E_NONE;
    bcm_rx_reasons_t reasons_remain, reasons_get;
    bcm_rx_reason_t  ridx;
    uint32 sz = 0, bit = 0;
    int idx;
    
    /* NULL pointer check */
    if (reasons == NULL  || reasons_mask == NULL  || 
        int_prio == NULL || int_prio_mask == NULL || 
        packet_type == NULL || packet_type_mask == NULL || 
        cosq == NULL) {
        return BCM_E_PARAM;
    }

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
        SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit)) {
        /* Prio to cosq is not supported */
        if (*int_prio_mask) {
            return BCM_E_PARAM;
        }
    } else {
        return BCM_E_UNAVAIL;
    }

    /* Process packet type */
    if (*packet_type_mask) {
        temp = *packet_type & *packet_type_mask;
        if (temp & BCM_RX_COSQ_PACKET_TYPE_SWITCHED) {
            rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_SWITCHING, &val);
            BCM_IF_ERROR_RETURN(rv);
            *cosq = val;
            temp &= ~BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
        }
        if (temp & BCM_RX_COSQ_PACKET_TYPE_MIRROR) {
            rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_MIRRORING, &val);
            BCM_IF_ERROR_RETURN(rv);
            *cosq = val;
            temp &= ~BCM_RX_COSQ_PACKET_TYPE_MIRROR;
        }

        /* check whether there are packet types unsupported */
        if (temp) {
            return BCM_E_PARAM;
        }
    }


    /* Process reason codes */
    reasons_remain = *reasons_mask;
    memset(&reasons_get, 0, sizeof(bcm_rx_reasons_t));

    if (SOC_IS_TBX(unit)) {
        sz = _bcm_robo_tb_rx_cpu_cosmap_key_max;
        for (bit = 0; bit < sz; bit++) {
             /* Find reason being set */
             ridx  = _bcm_robo_tb_cpu_cos_map_key[bit];
             if (!BCM_RX_REASON_GET(*reasons_mask, ridx)) {
                 continue;
             }

             if (BCM_RX_REASON_GET(*reasons, ridx)) {
                 BCM_RX_REASON_SET(reasons_get, ridx);
             }

             /* clean the bit of reasons_remain */
             BCM_RX_REASON_CLEAR(reasons_remain, ridx);
        } /* for */
    } else {
        sz = _bcm_robo_rx_cpu_cosmap_key_max;
        for (bit = 0; bit < sz; bit++) {
             /* Find reason being set */
             ridx  = _bcm_robo_cpu_cos_map_key[bit];
             if (!BCM_RX_REASON_GET(*reasons_mask, ridx)) {
                 continue;
             }
    
             if (BCM_RX_REASON_GET(*reasons, ridx)) {
                 BCM_RX_REASON_SET(reasons_get, ridx);
             }

             /* clean the bit of reasons_remain */
             BCM_RX_REASON_CLEAR(reasons_remain, ridx);
        } /* for */
    }
        
    /* check whether there are reasons unsupported */
    for (idx = bcmRxReasonInvalid; idx < bcmRxReasonCount; idx++) {
         if (BCM_RX_REASON_GET(reasons_remain, idx)) {
             return BCM_E_PARAM;
         }
    }

    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonCpuLearn)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_SA_LEARNING, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonControl)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_PROTO_TERM, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonProtocol)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_PROTO_SNOOP, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonExceptionFlood)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_EXCEPTION, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonL2Cpu)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_SWITCHING, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonL2DestMiss)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_EXCEPTION, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonEgressFilterRedirect)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_ARL_VALN_DIR_FWD, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonFilterRedirect)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_ARL_CFP_FWD, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonLoopback)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_ARL_LOOPBACK, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonSampleSource)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_INGRESS_SFLOW, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonSampleDest)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_EGRESS_SFLOW, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonL2Move)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_SA_MOVEMENT_EVENT, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonL2SourceMiss)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_SA_UNKNOWN_EVENT, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonL2LearnLimit)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_SA_OVER_LIMIT_EVENT, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonIngressFilter)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_INP_NON_MEMBER, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }
    if (BCM_RX_REASON_GET(reasons_get, bcmRxReasonUnknownVlan)) {
        rv = DRV_QUEUE_RX_REASON_GET(unit, DRV_RX_REASON_VLAN_UNKNOWN, &val);
        BCM_IF_ERROR_RETURN(rv);
        *cosq = val;
    }

    return rv;
}


/*
 * Function:
 *      bcm_rx_cosq_mapping_delete
 * Purpose:
 *      Delete the COSQ mapping at the specified index
 * Parameters:
 *      unit - Unit reference
 *      index - Index into COSQ mapping table
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_cosq_mapping_delete(int unit, int index)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rx_reasons_get
 * Purpose:
 *      Get all supported easons for rx packets
 * Parameters:
 *      unit - Unit reference
 *      reasons - rx packet "reasons" bitmap
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_rx_reasons_get (int unit, bcm_rx_reasons_t *reasons)
{
    int rv = BCM_E_NONE;
    
    if (!SOC_UNIT_VALID(unit)) {
        
        return BCM_E_INTERNAL;
    }

    BCM_RX_REASON_CLEAR_ALL(*reasons);

    if (SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        BCM_RX_REASON_SET(*reasons, bcmRxReasonCpuLearn);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonControl);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonProtocol);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonExceptionFlood);
    } else if (SOC_IS_TBX(unit)) {
        BCM_RX_REASON_SET(*reasons, bcmRxReasonFilterMatch);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonL2Cpu);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonL2DestMiss);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonControl);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonProtocol);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonEgressFilterRedirect);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonFilterRedirect);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonLoopback);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonSampleSource);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonSampleDest);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonL2Move);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonL2SourceMiss);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonL2LearnLimit);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonIngressFilter);
        BCM_RX_REASON_SET(*reasons, bcmRxReasonUnknownVlan);
    } else {
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *      bcm_rx_cosq_mapping_reasons_get
 * Purpose:
 *      Get all supported reasons for CPU cosq mapping  
 * Parameters:
 *      unit - Unit reference
 *      reasons - cpu cosq "reasons" mapping bitmap
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_robo_rx_cosq_mapping_reasons_get(int unit, bcm_rx_reasons_t * reasons)
{
    uint32          sz = 0;
    uint32          ix = 0;
    int id = 0;

    if (reasons == NULL) {
        return BCM_E_PARAM;
    }

    BCM_RX_REASON_CLEAR_ALL(*reasons);

    if (SOC_IS_ROBO53242(unit) ||
        SOC_IS_ROBO53262(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        sz = _bcm_robo_rx_cpu_cosmap_key_max;
        for (ix = 0; ix < sz; ix++) {
             id = _bcm_robo_cpu_cos_map_key[ix];
             if (id != (bcm_rx_reason_t)bcmRxReasonInvalid) {
                 BCM_RX_REASON_SET(*reasons, id);
             }
        }
    } else if (SOC_IS_TBX(unit)) {
        sz = _bcm_robo_tb_rx_cpu_cosmap_key_max;
        for (ix = 0; ix < sz; ix++) {
             id = _bcm_robo_tb_cpu_cos_map_key[ix];
             if (id != (bcm_rx_reason_t)bcmRxReasonInvalid) {
                 BCM_RX_REASON_SET(*reasons, id);
             }
        }
    } else {
        return BCM_E_UNAVAIL;
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_rx_queue_packet_count_get
 * Purpose:
 *      Get number of packets awaiting processing in the specific device/queue.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      cosq         - (IN) Queue priority.
 *      packet_count - (OUT) Next attached unit with initialized rx.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_rx_queue_packet_get(int unit, bcm_cos_queue_t cosq, int *packet_count)
{
    return (BCM_E_UNAVAIL);
}

/*  
 * Function:
 *      bcm_rx_channel_queue_add
 * Purpose:
 *      Associate a CPU cos queue with a RX channel
 * Parameters:
 *      unit - Unit reference
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 *      queue_id - CPU cos queue index (0 - (max cosq - 1)) 
 *                                     (Negative for all)
 * Returns:
 *      BCM_E_XXX
 * Note: 
 *      This API does not perform channel enabling;
 */
int
bcm_robo_rx_channel_queue_add (int unit, bcm_rx_chan_t chan_id, 
                          bcm_cos_queue_t queue_id)
{
    return (BCM_E_UNAVAIL);
}

/*  
 * Function:
 *      bcm_rx_channel_queue_delete
 * Purpose:
 *      De-couple a CPU cos queue from a channel
 * Parameters:
 *      unit - Unit reference
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 *      queue_id - CPU cos queue index (0 - (max cos - 1))
 * Returns:
 *      BCM_E_XXX
 * Note: 
 *      This API does not perform channel disabling;
 */
int
bcm_robo_rx_channel_queue_delete (int unit, bcm_rx_chan_t chan_id, 
                          bcm_cos_queue_t queue_id)
{
    return (BCM_E_UNAVAIL);
}

/*  
 * Function:
 *      bcm_rx_channel_queue_get
 * Purpose:
 *      Get the enabled COS queue associated with a RX channel
 * Parameters:
 *      unit - Unit reference
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 *      queue_max - Max queue elements on <queue_array>
 *      queue_array - array to store the queue number starting from 0
 *      queue_count - actual number filled in the <queue_array>
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This API is currently used for Triumph and Scorpion only
 */
int
bcm_robo_rx_channel_queue_get(int unit, bcm_rx_chan_t chan_id, int queue_max,
                         bcm_cos_queue_t *queue_array, int *queue_count)
{
    return (BCM_E_UNAVAIL);
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
bcm_robo_rx_queue_channel_set (int unit, bcm_cos_queue_t queue_id, 
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
bcm_robo_rx_queue_channel_get(int unit, bcm_cos_queue_t queue_id, 
                             bcm_rx_chan_t *chan_id)
{
    return BCM_E_UNAVAIL;
}


