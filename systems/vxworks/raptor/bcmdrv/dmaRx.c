/*
 * $Id: dmaRx.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include "salIntf.h"
#include "dmaOps.h"
#include "assert.h"
#include "config.h"

#define BCM_RX_COS        8 /* Number of COS supported */
#define BCM_RX_COS_ALL   -1 /* Affect all Cos */
#define BCM_RX_CHANNELS   4 /* Max. number of RX channels */
#define BCM_RX_CHAN_DFLT  1 /* Default RX channel */

#define BCM_RX_POOL_COUNT_DEFAULT 256
#define BCM_RX_POOL_BYTES_DEFAULT (2 * 1024)

/* Some default values */
#define RX_PKT_SIZE_DFLT   (2 * 1024) /* 12 K packets, plus spare */
#define RX_PPC_DFLT        1          /* 4 pkts/chain */
#define RX_PPS_DFLT        1000       /* 1000 pkts/sec */
#define RX_CHAINS_DFLT     16         /* 4 chains */
#define RX_THREAD_PRI_DFLT 200

/* Could probably be pretty large now */
#define RX_PPC_MAX             SOC_DV_PKTS_MAX

/* Default unit to use for alloc/free */
#define BCM_RX_DFLT_UNIT 0

#define RX_CHAINS_MAX 20

/*
 * DV related defines and DV state transition diagram
 *
 * State desriptions:
 *    DV_S_NEEDS_FILL     The DV needs to have its packets checked
 *    DV_S_FILLED         The DV has packets and is ready to be sent
 *                        to the SOC layer.
 *    DV_S_SCHEDULED      The DV could not be started due to rate
 *                        limiting, so it has been scheduled for a DPC
 *    DV_S_ACTIVE         The DV has been handed to the SOC layer and
 *                        is receiving packets.
 *    DV_S_CHN_DONE       A chain done interrupt has occurred for this
 *                        DV.  The SOC layer is done with the DV and it
 *                        now contains packets to process.
 *    DV_S_ERROR          An error has occurred.  The DV will not be
 *                        scheduled until RX is restarted.
 *
 * DVs start in the state NEEDS_FILL.
 *
 *     NEEDS_FILL  -->  FILLED          rx_dv_fill
 *     FILLED      -->  ACTIVE          rx_chain_start
 *     FILLED      -->  SCHEDULED       rx_chain_start_or_schedule
 *     SCHEDULED   -->  ACTIVE          rx_chain_start
 *     ACTIVE      -->  CHN_DONE        rx_done_chain
 *     ACTIVE      -->  NEEDS_FILL      all pkts processed, see below
 *     CHN_DONE    -->  NEEDS_FILL      see below
 *
 * A DV transitions from CHN_DONE or ACITVE to NEEDS_FILL when all
 * its packets have been processed.  This is detected by the
 * pkt_done_cnt of DV_INFO reaching the pkts/chain level.  Note that
 * if all pkts are being handled in the interrupt handler, this may
 * occur before the DV has transitioned to CHN_DONE which occurs in
 * the chain done interrupt handler.
 */

typedef enum dv_states_e {
    DV_S_NEEDS_FILL,  /* Packets are processed, needs refill */
    DV_S_FILLED,      /* DV filled, ready to be started */
    DV_S_SCHEDULED,   /* DV has been scheduled via a DPC */
    DV_S_ACTIVE,      /* DV has been started (given to soc layer) */
    DV_S_CHN_DONE,    /* DV is done, needs service (chain done) */
    DV_S_ERROR        /* DV has seen error */
} dv_states_t;

#define DV_STATE_STRINGS { \
        "Needs Fill", \
        "Filled", \
        "Scheduled", \
        "Active", \
        "Chain Done", \
        "Error" \
    }

/* This is all the extra information associated with a DV for RX */
typedef struct rx_dv_info_s {
    volatile dv_states_t state;
    volatile sal_usecs_t sched_time; /* When was DV scheduled */
    volatile int time_diff;          /* Delta time until started */
    volatile int abort_cleanup;      /* First packet after Start Channel */
    uint8 idx;
    uint8 chan;
    volatile uint8 pkt_done_cnt;
} rx_dv_info_t;

/*
 * DV and DCB related macros
 */
#define _DV_INFO dv_public1.ptr
#define DV_INFO(dv)          (((rx_dv_info_t *)(((dv_t *)dv)->_DV_INFO)))
#define DV_STATE(dv)         (DV_INFO(dv)->state)
#define DV_INDEX(dv)         (DV_INFO(dv)->idx)
#define DV_STATE(dv)         (DV_INFO(dv)->state)
#define DV_CHANNEL(dv)       (DV_INFO(dv)->chan)
#define DV_ABORT_CLEANUP(dv) (DV_INFO(dv)->abort_cleanup)

/**************** RX ***************/
typedef struct rx_chan_ctl_s {
    dv_t   *dv[RX_CHAINS_MAX];
    int    dcb_per_pkt; /* Number DCB/pkt */

    /* Some stats counters */
    int    rpkt;        /* Received packets */
    int    rbyte;       /* Received bytes */
    int    dpkt;        /* Discard packets */
    int    dbyte;       /* Discard bytes */
    int    mem_fail;    /* How many packet allocation errors */
    bcm_pkt_t *all_pkts;
} rx_chan_ctl_t;

/**************************************************************/
/**************************************************************/
/**************************************************************/

typedef void *(*bcm_rx_alloc_f)(int unit, int size, uint32 flags);
typedef void (*bcm_rx_free_f)(int unit, void *pkt_buf);

#define bcm_data_alloc_f bcm_rx_alloc_f
#define bcm_data_free_f bcm_rx_free_f

typedef struct bcm_rx_chan_cfg_s {
    int         chains;          /* Number of chains (DVs) set up. */
    /* 0 means channel not used. */
    int         rate_pps;        /* DEPRECATED:  Use */
    /* bcm_rx_cos_rate_set/get */
    int         flags;           /* Flags: */

#define BCM_RX_F_CRC_STRIP   0x1 /* Strip CRC from packets */
#define BCM_RX_F_VTAG_STRIP  0x2 /* Strip VLAN tag from packets */
#define BCM_RX_F_RATE_STALL  0x4 /* Use "stall" (vs discard) */
#define BCM_RX_F_MULTI_DCB   0x8 /* Scatter data with multiple DCBs */

    uint32      cos_bmp;         /* COS bitmap, if supported */
} bcm_rx_chan_cfg_t;

typedef struct bcm_rx_cfg_s {
    int               pkt_size;           /* Default pkt size. */
    int               pkts_per_chain;     /* # ptks/chain */
    int               global_pps;         /* Global rate limiting */
    int               max_burst;          /* In pkts */
    bcm_rx_chan_cfg_t chan_cfg[BCM_RX_CHANNELS]; /* Channel config */
    bcm_rx_alloc_f    rx_alloc;           /* Packet alloc routine */
    bcm_rx_free_f     rx_free;            /* Packet free routine */
    int32             flags;              /* config flags */
#define BCM_RX_F_IGNORE_HGHDR 0x1         /* Force HG Header into pkt for fabric */
#define BCM_RX_F_IGNORE_SLTAG 0x2         /* Force SL tag into pkt on SL stacking */
} bcm_rx_cfg_t;

/**************************************************************/
/**************************************************************/
/**************************************************************/

/*
 * Typedef:
 *      rx_ctl_t
 * Purpose:
 *      Control structure per unit for RX
 * Notes:
 *      Tokens must be signed
 */
typedef struct rx_ctl_s {
    /**************** LOCAL ONLY UNIT CONTROL ****************/
    volatile int      hndlr_intr_cnt;     /* Number of interrupt callouts */
    volatile uint32   chan_running;       /* Bitmap of channels enabled */
    int               cur_chan;           /* For fairness in refill/start */
    rx_chan_ctl_t     chan_ctl[BCM_RX_CHANNELS]; /* internal chan ctl */

    /**************** LOCAL AND REMOTE UNIT CONTROL ****************/
    bcm_rx_cfg_t      user_cfg;           /* User configuration */
    /* volatile rx_callout_t  *rc_callout; */  /* Callout linked list */
    volatile int      hndlr_cnt;          /* Non-intr callouts, no disc */

    /* For global rate control */
    sal_usecs_t       last_fill;          /* Last time bucket updated */
    sal_mutex_t       rx_mutex;           /* Sync for handler list */

    int               flags;
#define BCM_RX_F_STARTED 0x1              /* Started? */

    /**************** Counters ****************/
    int               tot_pkts;
    int               pkts_since_start;   /* Since last start, how many pkts */
    int               bad_hndlr_rv;
    int               no_hndlr;
    int               not_running;
    int               thrd_not_running;
    int               dcb_errs;
    int               pkts_owned;
    int               tunnelled;

    volatile void    *free_list;          /* Deferred free list */
} rx_ctl_t;

static rx_ctl_t *rx_ctl[BCM_MAX_NUM_UNITS];

struct _rxctrl rx_control[BCM_MAX_NUM_UNITS];
struct _rxctrl *bcm_get_rx_control_ptr(int unit) { return &rx_control[unit]; }

/*****************************************************************/
/***************************** RX POOL ***************************/
/*****************************************************************/

static uint8 *rxp_all_bufs;    /* Allocation pointer */
static uint8 *rxp_end_bufs;    /* Points to first byte after all bufs */
static uint8 *rxp_free_list;   /* Free list pointer */
static int rxp_pkt_size;       /* Base packet size */
static int rxp_pkt_count;      /* How many packets in pool */

static sal_mutex_t rxp_mutex;

#define RXP_LOCK       sal_mutex_take(rxp_mutex, sal_mutex_FOREVER)
#define RXP_UNLOCK     sal_mutex_give(rxp_mutex)

#define RXP_INIT \
    if (rxp_mutex == NULL && \
    (rxp_mutex = sal_mutex_create("rx_pool")) == NULL) \
    return BCM_E_MEMORY

    /* Any byte in a buffer may be used to reference that buffer when freeing */

    /* Buffer index; use any byte in buffer to get proper index */
#define RXP_BUF_IDX(buf) \
    ( ((buf) - rxp_all_bufs) / rxp_pkt_size )

    /*
     * Get the start/end addresses of a buffer given the index of the pkt
     */

#define RXP_BUF_START(idx) \
    (rxp_all_bufs + ((idx) * rxp_pkt_size))

    /*
     * Get the start address of a buffer given a pointer to any byte in the
     * buffer.
     */

#define RXP_PTR_TO_START(ptr) RXP_BUF_START(RXP_BUF_IDX(ptr))

    /*
     * Does "val" point into the buffer space at all?
     */

#define RXP_IN_BUFFER_SPACE(val) \
    (((val) < rxp_end_bufs) && ((val) >= rxp_all_bufs))

    /*
     * Does "val" point to the start of a buffer?
     */

#define RXP_IS_BUF_START(val) \
    (RXP_IN_BUFFER_SPACE(val) && \
    (((val) - rxp_all_bufs) % rxp_pkt_size == 0))

    /*
     * When buffers are not allocated, the first word is used as a pointer
     * for linked lists.  Requires alignment of the buffers.
     */
#define RXP_PKT_NEXT(buf) ( ((uint8 **)(RXP_PTR_TO_START(buf)))[0])

static int
bcm_rx_pool_setup(int pkt_count, int bytes_per_pkt)
{
    uint8 *buf, *next_buf;
    int i;

    RXP_INIT;

    if (rxp_all_bufs != NULL) {
        return BCM_E_BUSY;
    }

    if (pkt_count < 0) {
        pkt_count = BCM_RX_POOL_COUNT_DEFAULT;
    }

    if (bytes_per_pkt < 0) {
        bytes_per_pkt = BCM_RX_POOL_BYTES_DEFAULT;
    }

    /* Force packet alignment */
    bytes_per_pkt = (bytes_per_pkt + 7) & (~0x7);
    rxp_pkt_size = bytes_per_pkt;
    rxp_pkt_count = pkt_count;
    rxp_all_bufs = sal_dma_alloc(pkt_count * rxp_pkt_size, "bcm_rx_pool");
    if (rxp_all_bufs == NULL) {
        return BCM_E_MEMORY;
    }
    /* Set to all 0xee for RXP debug */
    sal_memset(rxp_all_bufs, 0xee, rxp_pkt_count * rxp_pkt_size);
    rxp_end_bufs = rxp_all_bufs + rxp_pkt_count * rxp_pkt_size;

    RXP_LOCK;
    buf = rxp_all_bufs;
    rxp_free_list = buf;
    for (i = 0; i < pkt_count - 1; i++) {
        next_buf = buf + rxp_pkt_size;
        RXP_PKT_NEXT(buf) = next_buf;
        buf = next_buf;
    }

    RXP_PKT_NEXT(buf) = NULL;

    RXP_UNLOCK;

    return BCM_E_NONE;
}


static int
bcm_rx_pool_setup_done(void)
{
    return rxp_all_bufs != NULL;
}


void *
bcm_rx_pool_alloc(int unit, int size, uint32 flags)
{
    uint8 *rv;

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(flags);

    if (rxp_mutex == NULL) {
        return NULL;
    }

    if (size > rxp_pkt_size) {
        PRINTF_ERROR(("bcm_rx_pool_alloc: %d > %d\n", size, rxp_pkt_size));
        return NULL;
    }

    RXP_LOCK;
    if (rxp_free_list == NULL) {
        RXP_UNLOCK;
        return NULL;
    }
    rv = rxp_free_list;
    rxp_free_list = RXP_PKT_NEXT(rxp_free_list);

    RXP_UNLOCK;

    return (void *)rv;
}


/*
 * Function:
 *      bcm_rx_pool_free
 * Purpose:
 *      Return a buffer allocated by bcm_rx_pool_alloc to the free list
 * Parameters:
 *      unit       - Ignored
 *      buf        - The buffer to free
 * Returns:
 *      BCM_E_XXX
 */

void
bcm_rx_pool_free(int unit, void *buf)
{
    uint8 *start;
    uint8 *buf8;

    COMPILER_REFERENCE(unit);

    RXP_LOCK;

    if (rxp_all_bufs == NULL) { /* not running */
        RXP_UNLOCK;
        return;
    }

    buf8 = (uint8 *)buf;
    start = RXP_PTR_TO_START(buf8);

    RXP_PKT_NEXT(start) = rxp_free_list;
    rxp_free_list = start;
    RXP_UNLOCK;
}


#define RX_CHAN_CFG(unit, chan)     (rx_ctl[unit]->user_cfg.chan_cfg[chan])
#define RX_CHAN_FLAGS(unit, chan)   (RX_CHAN_CFG(unit, chan).flags)
#define RX_CHAN_USED(unit, chan)    (RX_CHAN_CFG(unit, chan).chains != 0)
#define RX_CHAN_RUNNING(unit, chan) (rx_ctl[unit]->chan_running & (1 << chan))
#define RX_BURST(unit)              (rx_ctl[unit]->user_cfg.max_burst)
#define RX_PPC(unit)                (rx_ctl[unit]->user_cfg.pkts_per_chain)
#define RX_CHAINS(unit, chan)       (RX_CHAN_CFG(unit, chan).chains)
#define RX_CHAN_PKTS(unit, chan)    (rx_ctl[unit]->chan_ctl[chan].all_pkts)
#define RX_CHAN_PKT_COUNT(unit, chan) (RX_PPC(unit) * RX_CHAINS(unit, chan))

#define RX_CHAN_CTL(unit, chan)     (rx_ctl[unit]->chan_ctl[chan])
#define RX_PKT(unit, chan, dv_idx, idx) \
    (&(RX_CHAN_CTL(unit, chan).all_pkts[dv_idx * RX_PPC(unit) + idx]))
#define RX_DV(unit, chan, i) (RX_CHAN_CTL(unit, chan).dv[i])
#define DV_PKT(dv, i) \
    (RX_PKT(dv->dv_unit, DV_CHANNEL(dv), DV_INFO(dv)->idx, i))
#define DV_PKTS_PROCESSED(dv)        (DV_INFO(dv)->pkt_done_cnt)
#define INTR_CALLOUTS(unit) \
    (rx_ctl[unit]->hndlr_intr_cnt != 0)
#define NON_INTR_CALLOUTS(unit) \
    (rx_ctl[unit]->hndlr_cnt != 0)
#define NO_REAL_CALLOUTS(unit) \
    (rx_ctl[unit]->hndlr_intr_cnt == 0 && rx_ctl[unit]->hndlr_cnt == 0)

static void
rx_dcb_per_pkt_init(int unit)
{
    int chan;
    int dcb;

    dcb = 3;

    for (chan = 0; chan < BCM_RX_CHANNELS; chan++) {
        if (RX_CHAN_FLAGS(unit, chan) & BCM_RX_F_MULTI_DCB) {
            RX_CHAN_CTL(unit, chan).dcb_per_pkt = dcb;
        } else {
            RX_CHAN_CTL(unit, chan).dcb_per_pkt = 1;
        }
    }

}


sal_usecs_t sal_time_usecs(void);

dvt_t soc_dma_chan_dvt_get( int unit, dma_chan_t chan );
int soc_dma_chan_config(int unit, dma_chan_t c, dvt_t type, uint32 flags);

dv_t * soc_dma_dv_alloc(int unit, dvt_t op, int cnt);

/* DV and DCB related macros */
#define RX_DCB_PER_PKT(unit, chan)  (RX_CHAN_CTL(unit, chan).dcb_per_pkt)
#define RX_DCB_PER_DV(unit, chan) \
    (RX_DCB_PER_PKT(unit, chan) * RX_PPC(unit))

#define RX_THREAD_NOTIFY(unit) { \
        if (!rx_control[unit].pkt_notify_given) \
        { \
            sal_sem_give(rx_control[unit].pkt_notify); \
            rx_control[unit].pkt_notify_given = TRUE; \
        } \
    }

static void
rx_done_chain(int unit, dv_t *dv)
{
    int chan;

    chan = DV_CHANNEL(dv);

    if (DV_STATE(dv) == DV_S_ACTIVE) {
        DV_STATE(dv) = DV_S_CHN_DONE;
    }

    /*	RX_THREAD_NOTIFY( unit );*/
    /* Might implement stall here */
}


#define MARK_PKT_PROCESSED_LOCAL(unit, chan, dv, idx) \
    do \
    { \
        if (++DV_PKTS_PROCESSED(dv) == RX_PPC(unit)) \
        { \
            DV_STATE(dv) = DV_S_NEEDS_FILL; \
            /*RX_THREAD_NOTIFY(unit);*/ \
        } \
    } while (0)

    static void rx_intr_process_packet(int unit, dv_t *dv, dcb_t *dcb, bcm_pkt_t *pkt);

static void
rx_done_packet(int unit, dv_t *dv, dcb_t *dcb)
{
    volatile bcm_pkt_t *pkt;
    int pkt_dcb_idx;
    int flags, chan;
    uint32 count;

    chan = DV_CHANNEL(dv);
    pkt_dcb_idx = SOC_DCB_PTR2IDX(unit, dcb, dv->dv_dcb) /
        RX_DCB_PER_PKT(unit, chan);
    pkt = DV_PKT(dv, pkt_dcb_idx);
    assert(pkt_dcb_idx == pkt->_idx);

    /* for jumbo packet */
    flags = SOC_DCB_INTRINFO(unit, dcb, 0, &count);
    if (0 == (flags & SOC_DCB_INFO_PKTEND)) {
        /* No matter DCB mode is multi or single mode, the DV of packet
           will be free if the packet is !end & pkt_size > dma_len */
        rx_ctl[unit]->dcb_errs++;
        MARK_PKT_PROCESSED_LOCAL(unit, chan, dv, pkt_dcb_idx);
        return;
    }

    rx_ctl[unit]->tot_pkts++;
    rx_ctl[unit]->pkts_since_start++;

    if (RX_CHAN_RUNNING(unit, chan)) {
        /* Do not process packet if an error is noticed. */
        /*
         * XGS 3 RX error as a result of previously aborting RX DMA
         * First DCB in the RX packet must have Start bit set
         */
        int     abort_cleanup_done;

        if (DV_ABORT_CLEANUP(dv)) {
            dcb_t   *sop_dcb;

            DV_ABORT_CLEANUP(dv) = 0;

            sop_dcb = SOC_DCB_IDX2PTR(unit,
                dv->dv_dcb,
                pkt_dcb_idx * RX_DCB_PER_PKT(unit, chan));
            if (SOC_DCB_RX_START_GET(unit, sop_dcb)) {
                abort_cleanup_done = 1;
            } else {
                abort_cleanup_done = 0;
            }
        } else {
            abort_cleanup_done = 1;
        }

        if (!SOC_DCB_RX_ERROR_GET(unit, dcb) && abort_cleanup_done) {
            rx_intr_process_packet(unit, dv, dcb, (bcm_pkt_t *)pkt);
        } else {
            rx_ctl[unit]->dcb_errs++;
            MARK_PKT_PROCESSED_LOCAL(unit, chan, dv, pkt_dcb_idx);
        }
    } else { /* If not active, mark the packet as handled */
        rx_ctl[unit]->not_running++;
        MARK_PKT_PROCESSED_LOCAL(unit, chan, dv, pkt_dcb_idx);
    }

#ifndef POLLING_MODE
    RX_THREAD_NOTIFY( unit );
#endif
}


#define DV_RX_IDX_SET(dv, val) (dv)->dv_public2.u32 = (val)
static int rx_dv_count;    /* Debug: count number of DVs used */

static dv_t *
rx_dv_alloc(int unit, int chan, int dv_idx)
{
    dv_t *dv;
    rx_dv_info_t *dv_info;
    int clr_len;

    if ((dv_info = sal_alloc(sizeof(rx_dv_info_t), "dv_info")) == NULL) {
        return NULL;
    }
    sal_memset(dv_info, 0, sizeof(rx_dv_info_t));
    if ((dv = soc_dma_dv_alloc(unit, DV_RX, RX_DCB_PER_DV(unit, chan))) == NULL) {
        sal_free(dv_info);
        return NULL;
    }

    /* DCBs MUST start off 0 */
    clr_len = SOC_DCB_SIZE(unit) * RX_DCB_PER_DV(unit, chan);
    sal_memset(dv->dv_dcb, 0, clr_len);

    /* Set up and install the DV and its info structure */
    dv->dv_done_chain = rx_done_chain;
    dv->dv_done_packet  = rx_done_packet;

    dv_info->idx = dv_idx;
    dv_info->chan = chan;
    dv_info->state = DV_S_NEEDS_FILL;
    dv->_DV_INFO = dv_info;

    DV_RX_IDX_SET(dv, rx_dv_count++); /* Debug: Indicate DVs index */

    return dv;

}


static int
rx_channel_dv_setup(int unit, int chan)
{
    int i, j;
    dv_t *dv;
    bcm_pkt_t *all_pkts;
    bcm_pkt_t *pkt;

    /* If channel is not configured for RX, configure it. */
    switch (soc_dma_chan_dvt_get(unit, chan)) {
        case DV_RX: /* Okay, already RX */
            break;
        case DV_NONE: /* Need to configure the channel */
            SOC_IF_ERROR_RETURN
                (soc_dma_chan_config(unit, chan, DV_RX, SOC_DMA_F_MBM));
            break;
        default: /* Problem */
            PRINTF_ERROR(("Incompatible channel setup for %d/%d\n", unit, chan));
            return BCM_E_PARAM;
            break;
    }

    /* Allocate the packet structures we need for the DVs */
    if (RX_CHAN_PKTS(unit, chan) == NULL) {
        all_pkts = sal_alloc(sizeof(bcm_pkt_t) *
            RX_CHAN_PKT_COUNT(unit, chan), "rx_pkts");
        if (all_pkts == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(all_pkts, 0,
                   sizeof(bcm_pkt_t) * RX_PPC(unit) * RX_CHAINS(unit, chan));
        RX_CHAN_PKTS(unit, chan) = all_pkts;
    }

    /* Set up packets and allocate DVs */
    for (i = 0; i < RX_CHAINS(unit, chan); i++) {
        if ((dv = rx_dv_alloc(unit, chan, i)) == NULL) {
            PRINTF_ERROR(("FATAL ERROR : NO MEMORY IN DV ALLOCATION!n"));
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

        DV_STATE(dv) = DV_S_NEEDS_FILL;
        RX_DV(unit, chan, i) = dv;
    }

    return BCM_E_NONE;
}


/* Default data for configuring RX system */
static bcm_rx_cfg_t _rx_dflt_cfg = {
    RX_PKT_SIZE_DFLT,        /* packet alloc size */
    RX_PPC_DFLT,             /* Packets per chain */
    RX_PPS_DFLT,             /* Default pkt rate, global */
    0,  /* Burst */
    {   /* Just configure channel 1 */
        {0},                 /* Channel 0 is usually TX */
        {                    /* Channel 1, default RX */
            RX_CHAINS_DFLT,  /* DV count (number of chains) */
            1000,            /* Default pkt rate, DEPRECATED */
            0,               /* No flags */
            0xff             /* COS bitmap channel to receive */
        }
    },
    bcm_rx_pool_alloc,       /* alloc function */
    bcm_rx_pool_free,        /* free function */
    0                        /* flags */
};

int
bcm_rx_init(int unit)
{
    rx_ctl[unit] = sal_alloc(sizeof(rx_ctl_t), "rx_ctl");
    if (rx_ctl[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(rx_ctl[unit], 0, sizeof(rx_ctl_t));
    sal_memcpy(&rx_ctl[unit]->user_cfg, &_rx_dflt_cfg,
        sizeof(bcm_rx_cfg_t));

    rx_ctl[unit]->rx_mutex = sal_mutex_create("RX_MUTEX");

    if (!bcm_rx_pool_setup_done()) {
        BCM_IF_ERROR_RETURN
            (bcm_rx_pool_setup(-1,
                        rx_ctl[unit]->user_cfg.pkt_size + sizeof(void *)));
    }

    PRINTF_DEBUG(("RX: Initialized unit %d\n", unit));

    return BCM_E_NONE;
}


static int         rx_spl;
#define RX_INTR_LOCK            rx_spl = sal_splhi()
#define RX_INTR_UNLOCK          sal_spl(rx_spl)

static int _rx_chan_run_count;

/* Check if given quantity is < cur sleep secs; set to that value if so */
#define BASE_SLEEP_VAL  10000

/* Set sleep to base value */
/*#define INIT_SLEEP    rx_control[unit].sleep_cur = BASE_SLEEP_VAL*/

#define CHECK_THREAD_DONE \
    if (!rx_control[unit].thread_running) goto thread_done

#define PKT_TO_FREE_NEXT(data) (((void **)data)[0])

void
bcm_rx_free(int unit, void *pkt_data)
{
    if (pkt_data != NULL) {
        rx_ctl[unit]->user_cfg.rx_free(unit, pkt_data);
    }
}


void *
bcm_rx_alloc(int unit, int pkt_size, uint32 flags)
{
    uint8 *ptr;

    if (pkt_size <= 0) {
        pkt_size = rx_ctl[unit]->user_cfg.pkt_size;
    }

    ptr = rx_ctl[unit]->user_cfg.rx_alloc(unit, pkt_size, flags);

    return ptr;
}


#ifndef POLLING_MODE

void rx_pkt_thread(void *param);

static int
rx_thread_start(int unit)
{
    if (!rx_control[unit].pkt_notify) {
        if ((rx_control[unit].pkt_notify =
        sal_sem_create("RX pkt ntfy", sal_sem_BINARY, 0)) == NULL) {
            return BCM_E_MEMORY;
        }

        rx_control[unit].pkt_notify_given = FALSE;
    }

    {
        char buf[10];
        sprintf(buf, "bcmRX%d", unit);
        if ((rx_control[unit].rx_tid = sal_thread_create(buf,
            SAL_THREAD_STKSZ,
            rx_control[unit].thread_pri,
            rx_pkt_thread,
            unit)) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    return BCM_E_NONE;
}

#else
static int rx_thread_start(int unit) {
    return BCM_E_NONE;
}
#endif

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
 *      Starts the packet receive thread if not already running.
 *      cfg may be null:  Use default config.
 *      alloc/free in cfg may be null:  Use default alloc/free functions
 */
int
bcm_rx_start(int unit, bcm_rx_cfg_t *cfg)
{
    int rv = BCM_E_NONE;
    int chan;

    #define RX_INIT_DONE(unit)          (rx_ctl[unit] != NULL)
    #define RX_INIT_CHECK(unit)         if (!RX_INIT_DONE(unit)) \
        BCM_IF_ERROR_RETURN(bcm_rx_init(unit))
    RX_INIT_CHECK(unit);

    #define RX_IS_SETUP(unit)     (rx_ctl[unit] != NULL)
    #define RX_UNIT_STARTED(unit) \
        (RX_IS_SETUP(unit) && rx_ctl[unit]->flags & BCM_RX_F_STARTED)

    if (RX_UNIT_STARTED(unit)) {
        PRINTF_ERROR(("RX start %d:  Already started\n", unit));
        return BCM_E_BUSY;
    }

    PRINTF_DEBUG(("RX: Starting unit %d\n", unit));

    rx_dcb_per_pkt_init(unit);

    /*  rx_init_all_tokens(unit);*/
    rx_ctl[unit]->pkts_since_start = 0;
    rx_ctl[unit]->pkts_owned = 0;

    /** one interrupt handler */
    rx_ctl[unit]->hndlr_intr_cnt++;

    {
        uint32  cos_pbmp = 0;
        int  rx_cos_en = -1;

        /* For each channel that is setup (could be or is active) */
        #define FOREACH_SETUP_CHANNEL(unit, chan) \
            for (chan = 0; chan < BCM_RX_CHANNELS; chan++) \
            if (RX_CHAN_USED(unit, chan))

            /* Set up each channel */
        FOREACH_SETUP_CHANNEL(unit, chan) {
            if (RX_CHAN_CFG(unit, chan).cos_bmp) {
                rx_cos_en++;
                #define RCC_COS_MAP_SHFT(ch)            ((ch) * 8)
                cos_pbmp |= (RX_CHAN_CFG(unit, chan).cos_bmp <<
                    RCC_COS_MAP_SHFT(chan));
            }
            PRINTF_DEBUG(("SETTING  UP CHANNEL %d on %d to do RX %x\n",
                chan, unit, cos_pbmp));
            SOC_IF_ERROR_RETURN( rx_channel_dv_setup(unit, chan) );
            rx_ctl[unit]->chan_running |= (1 << chan);
            ++_rx_chan_run_count;
        }

        /*
         * RX cos based DMA
         */
        #define CMIC_RX_COS_CONTROL   0x00000180
        soc_pci_write(unit, CMIC_RX_COS_CONTROL, cos_pbmp);
        soc_pci_write(unit, CMIC_CONFIG,
            (soc_pci_read(unit, CMIC_CONFIG) |
            CC_COS_QUALIFIED_DMA_RX_EN));
    }

    RX_INTR_LOCK;

    if (!rx_control[unit].thread_running) {
        rx_control[unit].thread_running = TRUE;
        RX_INTR_UNLOCK;
        if (rx_control[unit].thread_pri == 0) { /* Check for uninitialized pri */
            rx_control[unit].thread_pri = RX_THREAD_PRI_DFLT;
        }
        /* Start up the thread */
        if ((rv = rx_thread_start(unit)) < 0) {
            rx_control[unit].thread_running = FALSE;
            /* rx_cleanup(unit); */
            return rv;
        }
    } else {
        RX_INTR_UNLOCK;
    }

    rx_ctl[unit]->flags |= BCM_RX_F_STARTED;

    return rv;
}


#define BCM_PKT_HAS_HGHDR(pkt) (((pkt)->flags & BCM_PKT_F_HGHDR) != 0)
#define BCM_PKT_HAS_SLTAG(pkt) (((pkt)->flags & BCM_PKT_F_SLTAG) != 0)
#define BCM_PKT_NO_VLAN_TAG(pkt) (((pkt)->flags & BCM_PKT_F_NO_VTAG) != 0)
#define BCM_PKT_RX_CRC_STRIP(pkt)       ((pkt)->flags & BCM_RX_CRC_STRIP)
#define BCM_PKT_RX_VLAN_TAG_STRIP(pkt)   BCM_PKT_NO_VLAN_TAG(pkt)

static void
pkt_len_get(int unit, int chan, bcm_pkt_t *pkt, dv_t *dv)
{
    int len = 0;
    int i;

    for (i = (pkt->_idx * RX_DCB_PER_PKT(unit, chan));
    i < (pkt->_idx + 1) * RX_DCB_PER_PKT(unit, chan); i++) {
        len += SOC_DCB_XFERCOUNT_GET(unit,
            SOC_DCB_IDX2PTR(unit, dv->dv_dcb, i));
    }

    pkt->tot_len = len;
    pkt->pkt_len = len;

    /* Don't include some tags in "pkt_len" */
    if (BCM_PKT_HAS_HGHDR(pkt)) {
        /*
         * HiGig pkts (Hercules only):  Do not include header length;
         * but, VLAN is inserted into the packet, so need to add that
         * back in.
         */
        pkt->pkt_len -= (sizeof(soc_higig_hdr_t) - sizeof(uint32));
    }

    if (BCM_PKT_HAS_SLTAG(pkt)) {
        pkt->pkt_len -= sizeof(uint32);
    }
    if (BCM_PKT_RX_CRC_STRIP(pkt)) {
        pkt->pkt_len -= sizeof(uint32);
    }
    if (BCM_PKT_RX_VLAN_TAG_STRIP(pkt)) {
        pkt->pkt_len -= sizeof(uint32);
    }
}


typedef enum {
    BCM_RX_INVALID,      /* Invalid return value */
    BCM_RX_NOT_HANDLED,  /* Packet not processed */
    BCM_RX_HANDLED,      /* Packet handled, not owned */
    BCM_RX_HANDLED_OWNED /* Packet handled and owned */
} bcm_rx_t;

static void
multi_dcb_fixup(int unit, bcm_pkt_t *pkt, dv_t *dv, int idx)
{
    if (BCM_PKT_HAS_SLTAG(pkt)) {
        /* SL tag was DMA'd to DV buffer; copy to pkt->_sltag */
        sal_memcpy(pkt->_sltag, SOC_DV_SL_TAG(dv, idx), sizeof(uint32));
    }

    if (BCM_PKT_RX_VLAN_TAG_STRIP(pkt)) {
        /* VLAN tag was DMA'd to DV buffer; copy to pkt->_vtag */
        sal_memcpy(pkt->_vtag, SOC_DV_VLAN_TAG(dv, idx), sizeof(uint32));
    }
}


/* Move data around if it was DMA'd in a single data buffer */

static void
single_dcb_fixup(int unit, bcm_pkt_t *pkt)
{
    uint8 tmp_buf[20];

    /* Single buffer was used for packet; move data as necessary */

    /* No HG header */
    if (BCM_PKT_HAS_SLTAG(pkt)) { /* Strip SL tag */
        sal_memcpy(pkt->_sltag, &pkt->_pkt_data.data[16], 4);
        sal_memcpy(tmp_buf, pkt->_pkt_data.data, 16);
        pkt->_pkt_data.data += 4; /* Eliminated 4 byte SL tag */
        sal_memcpy(pkt->_pkt_data.data, tmp_buf, 16);
    }

    if (BCM_PKT_RX_VLAN_TAG_STRIP(pkt)) { /* Strip VLAN tag */
        sal_memcpy(pkt->_vtag, &pkt->_pkt_data.data[12], 4);
        sal_memcpy(tmp_buf, pkt->_pkt_data.data, 12);
        pkt->_pkt_data.data += 4; /* Eliminated 4 byte VLAN tag */
        sal_memcpy(pkt->_pkt_data.data, tmp_buf, 12);
    }
}


#define BCM_PKT_IEEE(pkt)      ((uint8*)((pkt)->pkt_data[0].data))
#define BCM_PKT_IEEE_LEN(pkt)  ((pkt)->pkt_len)

#define BCM_PKT_DMAC(pkt)      BCM_PKT_IEEE(pkt)

#define BCM_PKT_VLAN_PTR(pkt) \
    (((pkt)->flags & BCM_PKT_F_NO_VTAG) ? (pkt)->_vtag : \
    ((BCM_PKT_DMAC(pkt) + 2*sizeof(bcm_mac_t))))
/* The VLAN control tag itself */
#define BCM_PKT_VLAN_CONTROL(pkt) \
    ((uint16) ((BCM_PKT_VLAN_PTR(pkt)[2] << 8) | (BCM_PKT_VLAN_PTR(pkt)[3])))
#define BCM_VLAN_CTRL_PRIO(c)           ((c) >> 13 & 0x007)
#define BCM_PKT_VLAN_PRI(pkt)  BCM_VLAN_CTRL_PRIO(BCM_PKT_VLAN_CONTROL(pkt))

extern int netdrvProcessRecv( bcm_pkt_t * );

static void
rx_intr_process_packet(int unit, dv_t *dv, dcb_t *dcb, bcm_pkt_t *pkt)
{
    bcm_dma_chan_t        chan;
    int                   idx;

    idx = pkt->_idx;
    chan = DV_CHANNEL(dv);

    pkt_len_get(unit, chan, (bcm_pkt_t *)pkt, dv);
    pkt->rx_port = SOC_DCB_RX_INGPORT_GET(unit, dcb);

    /* If single buffer used to DMA packet, may need cleanup */
    if (RX_CHAN_FLAGS(unit, chan) & BCM_RX_F_MULTI_DCB) {
        multi_dcb_fixup(unit, pkt, dv, idx);
    } else {
        single_dcb_fixup(unit, pkt);
    }

    /* Fill in other XGS parameters */
    pkt->opcode = SOC_DCB_RX_OPCODE_GET(unit, dcb);
    pkt->dest_mod = SOC_DCB_RX_DESTMOD_GET(unit, dcb);
    pkt->dest_port = SOC_DCB_RX_DESTPORT_GET(unit, dcb);
    pkt->src_mod = SOC_DCB_RX_SRCMOD_GET(unit, dcb);
    pkt->src_port = SOC_DCB_RX_SRCPORT_GET(unit, dcb);
    pkt->cos = SOC_DCB_RX_COS_GET(unit, dcb);

    pkt->prio_int = BCM_PKT_VLAN_PRI(pkt);

    RX_CHAN_CTL(unit, chan).rpkt++;
    RX_CHAN_CTL(unit, chan).rbyte += pkt->tot_len;

    /* Get data into packet structure */
    pkt->dma_channel = chan;
    pkt->rx_unit = pkt->unit = unit;
    pkt->rx_reason = SOC_DCB_RX_REASON_GET(unit, dcb);
    pkt->rx_untagged = SOC_DCB_RX_UNTAGGED_GET(unit, dcb);

    netdrvProcessRecv(pkt );

    MARK_PKT_PROCESSED_LOCAL(unit, chan, dv, idx);
}


/* Set up a DV for multiple DCBs (pkt scatter) according to packet format */
static int
rx_multi_dv_pkt(int unit, volatile bcm_pkt_t *pkt, dv_t *dv, int idx)
{
    int bytes_used;
    uint8 *data_ptr;

    /* HIGIG HEADER: Always into DV struct if present */
    if (BCM_PKT_HAS_HGHDR(pkt)) {
        SOC_IF_ERROR_RETURN(SOC_DCB_ADDRX(unit, dv,
            (sal_vaddr_t)SOC_DV_HG_HDR(dv, idx),
            sizeof(soc_higig_hdr_t), 0));
    }

    /* Set up DCB for DMAC and SMAC:  Always into packet */
    SOC_IF_ERROR_RETURN(SOC_DCB_ADDRX(unit, dv,
        (sal_vaddr_t)BCM_PKT_DMAC(pkt),
        2 * sizeof(bcm_mac_t), 0));
    bytes_used = 2 * sizeof(bcm_mac_t);

    /* Setup DCB for VLAN:  Either into DV struct, or pkt as per flags */
    {
        /* Assume VLAN tag goes into packet. */
        data_ptr = &pkt->_pkt_data.data[12];
        if (BCM_PKT_RX_VLAN_TAG_STRIP(pkt)) {
            /* Oops, VLAN tag must be placed in non-pkt buffer */
            data_ptr = SOC_DV_VLAN_TAG(dv, idx);
        } else {
            /* Okay, VLAN in pkt.  Adjust byte count for pkt buffer */
            bytes_used += sizeof(uint32); /* Used for VLAN in pkt */
        }
        /* data_ptr now points to where VLAN tag should be DMA'd */
        SOC_IF_ERROR_RETURN(SOC_DCB_ADDRX(unit, dv,
            (sal_vaddr_t)data_ptr,
            sizeof(uint32), 0));
    }

    /* Setup DCB for SL TAG when present:  Always put into DV struct */
    if (BCM_PKT_HAS_SLTAG(pkt)) {
        SOC_IF_ERROR_RETURN(SOC_DCB_ADDRX(unit, dv,
            (sal_vaddr_t)SOC_DV_SL_TAG(dv, idx),
            sizeof(uint32), 0));
    }

    /* Setup DCB for PAYLOAD + CRC:  Always into packet */
    SOC_IF_ERROR_RETURN(SOC_DCB_ADDRX(unit, dv,
        (sal_vaddr_t)&(pkt->_pkt_data.data[bytes_used]),
        pkt->_pkt_data.len - bytes_used,
        0));

    return BCM_E_NONE;
}


void soc_dma_desc_end_packet(dv_t *dv);

static int
rx_dv_add_pkt(int unit, volatile bcm_pkt_t *pkt, int idx, dv_t *dv)
{
    if (RX_CHAN_FLAGS(unit, DV_CHANNEL(dv)) & BCM_RX_F_MULTI_DCB) {
        BCM_IF_ERROR_RETURN(rx_multi_dv_pkt(unit, pkt, dv, idx));
    } else {
        /* Setup single DCB for all headers + PAYLOAD + CRC */
        SOC_IF_ERROR_RETURN(SOC_DCB_ADDRX(unit, dv,
            (sal_vaddr_t)(pkt->_pkt_data.data),
            pkt->_pkt_data.len,
            0));
    }

    soc_dma_desc_end_packet(dv);

    return BCM_E_NONE;
}


void soc_dma_dv_reset(dvt_t op, dv_t *dv);

static void
rx_dv_fill(int unit, int chan, dv_t *dv)
{
    int         i;
    volatile bcm_pkt_t   *pkt;
    rx_dv_info_t   *save_info;
    static int warned = 0;
    void *pkt_data;
    int rv;

    save_info = DV_INFO(dv);  /* Save info before resetting dv */
    soc_dma_dv_reset(DV_RX, dv);
    dv->_DV_INFO = save_info; /* Restore info to DV */

    assert(DV_STATE(dv) == DV_S_NEEDS_FILL);

    for (i = 0; i < RX_PPC(unit); i++) {
        pkt = DV_PKT(dv, i);
        if (pkt->_pkt_data.data == NULL) {
            /* No pkt buffer; it needs to be allocated; use dflt size */

            pkt_data = bcm_rx_alloc(unit, -1, RX_CHAN_FLAGS(unit, chan));

            if (pkt_data == NULL) { /* Failed to allocate a pkt for this DV. */
                if (!warned) {
                    warned = 1;
                    PRINTF_ERROR(("RX: Failed to allocate mem\n"));
                }
                RX_CHAN_CTL(unit, chan).mem_fail++;
                return; /* Not an error, try again later */
            }
            pkt->_pkt_data.data = pkt_data;
            pkt->alloc_ptr = pkt_data;
            pkt->_pkt_data.len = rx_ctl[unit]->user_cfg.pkt_size;
        } else {
            pkt->_pkt_data.data = pkt->alloc_ptr;
        }

        /* Set up CRC stripping */
        if (RX_CHAN_FLAGS(unit, chan) & BCM_RX_F_CRC_STRIP) {
            pkt->flags |= BCM_RX_CRC_STRIP;
        } else {
            pkt->flags &= ~BCM_RX_CRC_STRIP;
        }

        /* Set up Vlan Tag stripping */
        if (RX_CHAN_FLAGS(unit, chan) & BCM_RX_F_VTAG_STRIP) {
            pkt->flags |= BCM_PKT_F_NO_VTAG;
        } else {
            pkt->flags &= ~BCM_PKT_F_NO_VTAG;
        }


        /* Set up the packet in the DCBs of the DV */
        if ((rv = rx_dv_add_pkt(unit, pkt, i, dv)) < 0) {
            DV_STATE(dv) = DV_S_ERROR;
            PRINTF_DEBUG2((
                "Failed to add pkt %d to dv on unit %d: %d\n",
                i, unit, rv));
            break;
        }
    }

    if (DV_STATE(dv) != DV_S_ERROR) { /* Mark as ready to be started */
        DV_STATE(dv) = DV_S_FILLED;
        DV_PKTS_PROCESSED(dv) = 0;
    }

    return;
}


int soc_dma_start(int unit, dma_chan_t channel, dv_t *dv_chain);

/* Start a chain and update the tokens */
static int
rx_chain_start(int unit, int chan, dv_t *dv)
{
    int rv = BCM_E_NONE;

    PRINTF_DEBUG2(("RX: Starting %d/%d/%d\n",
        unit, chan, DV_INDEX(dv)));

    /* Start the DV */
    DV_STATE(dv) = DV_S_ACTIVE;

    /*soc_feature(unit, soc_feature_rxdma_cleanup);*/
    DV_ABORT_CLEANUP(dv) = 1;

    if ((rv = soc_dma_start(unit, chan, dv)) < 0) {
        DV_STATE(dv) = DV_S_ERROR;
        PRINTF_ERROR(("RX: Could not start dv, u %d, chan %d\n", unit, chan));
    }

    return rv;
}


static int
rx_chain_start_or_sched(int unit, int chan, dv_t *dv)
{

    PRINTF_DEBUG2(("RX: Chain. starting RX chain.\n"));

    return rx_chain_start(unit, chan, dv);
}


static int
rx_update_dv(int unit, int chan, dv_t *dv)
{
    int rv = BCM_E_NONE;

    /* If no real callouts, don't bother starting DVs */
    assert(dv);

    switch (DV_STATE(dv)) {

        case DV_S_NEEDS_FILL:
            rx_dv_fill(unit, chan, dv);
            /* If fill succeeded, try to schedule/start; otherwise break. */
            if (DV_STATE(dv) != DV_S_FILLED) {
                break;
            }
            /* Fall thru: Ready to be scheduled/started */
        case DV_S_FILLED: /* See if we can start or schedule this DV */
            rv = rx_chain_start_or_sched(unit, chan, dv);
            break;
        default:          /* Don't worry about other states here. */
            break;
    }

    return rv;
}


#define _CUR_CHAN(unit) (rx_ctl[unit]->cur_chan)

int
rx_thread_dv_check(int unit)
{
    int chan;
    int i, j;

    chan = _CUR_CHAN(unit);

    for (j = 0; j < BCM_RX_CHANNELS; j++) {
        if (RX_CHAN_RUNNING(unit, chan)) {
            for (i = 0; i < RX_CHAINS(unit, chan); i++) {
                SOC_IF_ERROR_RETURN(rx_update_dv(unit, chan, RX_DV(unit, chan, i)));
            }
        }
        chan = (chan + 1) % BCM_RX_CHANNELS;
    }

    _CUR_CHAN(unit) = (_CUR_CHAN(unit) + 1) % BCM_RX_CHANNELS;
    return SOC_E_NONE;
}
