/*
 * $Id: cmicd_dma.c,v 1.05 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:     CMICDV2 interface drvier for SOC DMA
 *
 */

#include <sal/core/boot.h>
#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <shared/bsl.h>

#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/dma.h>
#include <soc/drv.h>
#include <soc/dcb.h>
#include <soc/cmicm.h>

#ifdef INCLUDE_KNET
#include <soc/knet.h>
#endif

#ifdef BCM_CMICDV2_SUPPORT

#define DCB_LAST_DWORD 0xF
#define DCB_DONE_BIT   0x80000000

/* CMICD needs access to CMICM driver functions */
soc_cmic_dma_drv_t soc_cmicm_dma_drv[SOC_MAX_NUM_DEVICES];

/* Static Driver Functions */

/*
 * Function:
 *      cmicd_dma_ctrl_reset
 * Purpose:
 *      Reset the CMICD DMA Control to a known good state for
 *      the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *
 */

int
cmicd_dma_ctrl_reset(int unit)
{
    return(soc_cmicm_dma_drv[unit].ctrl_reset(unit));
}


/*
 * Function:
 *      cmicd_dma_chan_config
 * Purpose:
 *      Initialize the CMICD DMA channel for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 *      type - tx or rx
 *      f_intr - interrupt flags
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *
 */
static int
cmicd_dma_chan_config(int unit, int vchan, dvt_t type, uint32 flags)
{
    int             rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    sdc_t           *sc = &soc->soc_channels[vchan];
    uint32          bits;
    int             cmc = vchan / N_DMA_CHAN;
    int             chan = vchan % N_DMA_CHAN;
    uint32          cr;

    /* Define locals and turn flags into easy to use things */
    /*int f_mbm = (flags & SOC_DMA_F_MBM) != 0; */
    int f_interrupt = (flags & SOC_DMA_F_POLL) == 0;
    /*int f_drop = (flags & SOC_DMA_F_TX_DROP) != 0; */
    int f_default = (flags & SOC_DMA_F_DEFAULT) != 0;
    int f_desc_intr = (flags & SOC_DMA_F_INTR_ON_DESC) != 0;

    /* Set channel to Continuous Mode */
    SOC_DMA_MODE(unit) = SOC_DMA_MODE_CONTINUOUS;

    /* Initial state of channel */
    sc->sc_flags = 0;                   /* Clear flags to start */

    (void)soc_cmicm_cmcx_intr0_disable(unit, cmc, IRQ_CMCx_DESC_DONE(chan) |
                                        IRQ_CMCx_CNTLD_DESC_DONE(chan) |
                                        IRQ_CMCx_CHAIN_DONE(chan));

    cr = soc_pci_read(unit,CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc,chan));
    cr &= ~PKTDMA_ENABLE; /* clearing enable will also clear CHAIN_DONE */
    soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan), cr);

    cr = soc_pci_read(unit, CMIC_CMCx_DMA_STAT_CLR_OFFSET(cmc)); 
    soc_pci_write(unit, CMIC_CMCx_DMA_STAT_CLR_OFFSET(cmc),
                  DS_CNTLD_DESC_CLR(chan));
    soc_pci_write(unit, CMIC_CMCx_DMA_STAT_CLR_OFFSET(cmc), cr);

    /* Setup new mode */
    /* MBM Deprecated in CMICm */
    /* DROP_TX Deprecated in CMICm */
    bits = f_desc_intr ? PKTDMA_SEL_INTR_ON_DESC_OR_PKT : 0;

    if (type == DV_TX) {
        bits |= PKTDMA_DIRECTION;
        if (f_default) {
            soc->soc_dma_default_tx = sc;
        }
    } else if (type == DV_RX) {
        bits &= ~PKTDMA_DIRECTION;
        if (f_default) {
            soc->soc_dma_default_rx = sc;
        }
    } else if (type == DV_NONE) {
        f_interrupt = FALSE;            /* Force off */
    } else {
        assert(0);
    }

#ifdef INCLUDE_KNET
    if (SOC_KNET_MODE(unit)) {
        /* Interrupt are handled by kernel */
        f_interrupt = FALSE;
    }
#endif

    /* Only enable controlled descriptor
       done interrupt for TX AND RX Continuous Mode */
    if (f_interrupt) {
        (void)soc_cmicm_cmcx_intr0_enable(unit, cmc, IRQ_CMCx_CNTLD_DESC_DONE(chan));
    }

    sc->sc_type = type;
    cr = soc_pci_read(unit,CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc,chan));

    /* Clear everything except Endianess */
    cr &= (PKTDMA_BIG_ENDIAN | PKTDMA_DESC_BIG_ENDIAN);
    cr |= bits;

    /* Use Controlled Descriptor Interrupt Mode */
    cr |= PKTDMA_CNTLD_DESC_INTR_MODE;

    soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan), cr);

    return rv;
}

/*
 * Function:
 *      cmicd_dma_chan_start
 * Purpose:
 *      Start the CMICd DMA channel for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      sc   - SOC channel data structure pointer
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *
 */
static int
cmicd_dma_chan_start(int unit, sdc_t *sc)
{
    int             rv = SOC_E_NONE;
    dv_t            *dv;
    int             cmc = sc->sc_channel / N_DMA_CHAN;
    int             chan = sc->sc_channel % N_DMA_CHAN;
    uint32          val;

    if ((dv = sc->sc_dv_active = sc->sc_q) == NULL) {
        sc->sc_q_tail = NULL;
        return rv;
    }

#ifdef INCLUDE_KNET
    if (SOC_KNET_MODE(unit)) {
        return rv;
    }
#endif

    LOG_VERBOSE(BSL_LS_SOC_PACKETDMA,
                      (BSL_META_U(unit,
                                  "Starting channel %d\n"),
                       sc->sc_channel));

    /* Set up DMA descriptor address */
    soc_pci_write(unit, CMIC_CMCx_DMA_DESCy_OFFSET(cmc, chan),
                  soc_cm_l2p(unit, sc->sc_q->dv_dcb));

    /* Start DMA - Enable Continuous Mode for this channel */
    val = soc_pci_read(unit,CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan));
    val |= (PKTDMA_ENABLE | PKTDMA_CONTINUOUS_ENABLE);
    soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan), val);

    SDK_CONFIG_MEMORY_BARRIER;

    if (!(sc->sc_flags & SOC_DMA_F_POLL)) {
        (void)soc_cmicm_cmcx_intr0_enable(unit, cmc,
                                     IRQ_CMCx_CNTLD_DESC_DONE(chan));
    }

    return rv;
}

/*
 * Function:
 *      cmicd_dma_chan_poll
 * Purpose:
 *      Poll the CMICD DMA channel for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 *      type - poll type (chain done or desc done)
 *      detected - poll result pointer to be updated
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *      Assumes that SOC_DMA_LOCK is held when called.
 */
static int
cmicd_dma_chan_poll(int unit,
                    int vchan,
                    soc_dma_poll_type_t type,
                    int * detected)
{
    return (soc_cmicm_dma_drv[unit].chan_poll(unit, vchan, type, detected));
}

/*
 * Function:
 *      cmicd_dma_chan_abort
 * Purpose:
 *      Initialize the CMICD DMA channel for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 *      type - tx or rx
 *      f_intr - interrupt flags
 * Returns:
 *      SOC_E_NONE   - Success
 *      SOC_E_TIMEOUT - failed (Timeout)
 * Notes:
 *      Assumes SOC_DMA_LOCK is held for CMIC_DMA_CTRL manipulation.
 *
 */
static int
cmicd_dma_chan_abort(int unit, int vchan)
{
    int             rv = SOC_E_NONE;
    int             cmc = vchan / N_DMA_CHAN;
    int             chan = vchan % N_DMA_CHAN;
    int		        to;
    uint32          ctrl;
    soc_control_t  *soc = SOC_CONTROL(unit);
    sdc_t          *sc = &soc->soc_channels[chan];

    if ((soc_pci_read(unit, CMIC_CMCx_DMA_STAT_OFFSET(cmc))
         & DS_CMCx_DMA_ACTIVE(chan)) != 0) {
        ctrl = soc_pci_read(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan));
        ctrl |= PKTDMA_ENABLE;
        soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan), ctrl);
        soc_pci_write(unit,
                      CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan),
                      ctrl | PKTDMA_ABORT);
        SDK_CONFIG_MEMORY_BARRIER;

        to = soc_property_get(unit, spn_PDMA_TIMEOUT_USEC, 500000);
        while ((to >= 0) &&
               ((soc_pci_read(unit, CMIC_CMCx_DMA_STAT_OFFSET(cmc))
                 & DS_CMCx_DMA_ACTIVE(chan)) != 0)) {
            sal_udelay(1000);
            to -= 1000;
        }
        if ((soc_pci_read(unit, CMIC_CMCx_DMA_STAT_OFFSET(cmc))
             & DS_CMCx_DMA_ACTIVE(chan)) != 0) {
            LOG_ERROR(BSL_LS_SOC_PACKETDMA,
                      (BSL_META_U(unit,
                                  "soc_dma_abort_channel unit %d: "
                                  "channel %d abort timeout\n"),
                       unit, chan));
            rv = SOC_E_TIMEOUT;
            if (SOC_WARM_BOOT(unit)) {
                return rv;
            }
        }
    }
    LOG_VERBOSE(BSL_LS_SOC_PACKETDMA,
                (BSL_META_U(unit,
                            "soc_dma_abort_channel unit %d: "
                            "channel %d\n"),
                 unit, chan));

    ctrl = soc_pci_read(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan));
    /* Clearing enable will also clear CHAIN_DONE */
    ctrl &= ~(PKTDMA_ENABLE|PKTDMA_ABORT);
    soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, chan), ctrl);

    soc_pci_write(unit,
                  CMIC_CMCx_DMA_STAT_CLR_OFFSET(cmc),
                  (DS_DESCRD_CMPLT_CLR(chan) | DS_CNTLD_DESC_CLR(chan)));
    SDK_CONFIG_MEMORY_BARRIER;

    /* Mark channel as not started */
    sc->sc_dma_started = 0;

    return rv;
}

/*
 * Function:
 *      cmicd_dma_chan_dv_start
 * Purpose:
 *      Adds DV to the ready queue for CMICD DMA for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *      Assumes SOC_DMA_LOCK is held for CMIC_DMA_CTRL manipulation.
 *
 */
static int
cmicd_dma_chan_dv_start(int unit, sdc_t *sc, dv_t *dv_chain)
{
    int             rv = SOC_E_NONE;
    int             cmc = sc->sc_channel / N_DMA_CHAN;
    int             chan = sc->sc_channel % N_DMA_CHAN;
    dcb_t           *dcb;
    sal_paddr_t     reg_addr = 0;
    sal_paddr_t     reg_val = 0;

    dv_chain->dv_next = NULL;

#ifdef INCLUDE_KNET
    if (SOC_KNET_MODE(unit)) {
        /* KNET handling of DV start */
        if (sc->sc_q_cnt != 0) {
            sc->sc_q_tail->dv_next = dv_chain;
        } else {
            sc->sc_q = dv_chain;
        }
        sc->sc_q_tail = dv_chain;
        sc->sc_q_cnt++;
        if (sc->sc_dv_active == NULL) {
            if (sc->sc_dma_started) {
                sc->sc_dv_active = sc->sc_q;
            } else {
                /* Start DMA if channel not active */
                rv = cmicd_dma_chan_start(unit, sc);
                sc->sc_dma_started = 1;
            }
        }
        return rv;
    }
#endif

    if (sc->sc_q_cnt != 0) {
        sc->sc_q_tail->dv_next = dv_chain;
        /* Set the reload addr of tail to the address of the appended chain */
        dcb = SOC_DCB_IDX2PTR(unit, sc->sc_q_tail->dv_dcb,
                              sc->sc_q_tail->dv_vcnt - 1);
        SOC_DCB_ADDR_SET(dv_chain->dv_unit, dcb,
                         (sal_vaddr_t)dv_chain->dv_dcb);
    } else {
        sc->sc_q = dv_chain;
    }
    sc->sc_q_tail = dv_chain;
    sc->sc_q_cnt++;

    /* Set the HW register for the new DESC_HALT_ADDR- new for CMICDV2 */
    /* Need logical to physical mapping for addr */
    /* If DMA is currently halted, writing the new rld addr starts it again */
    reg_addr = (sal_paddr_t)CMIC_CMCx_DMA_CHy_DESC_HALT_ADDR_OFFSET(cmc, chan);

    reg_val = soc_cm_l2p(unit,
                         (void*)SOC_DCB_IDX2PTR(unit, dv_chain->dv_dcb,
                                                (dv_chain->dv_vcnt - 1)));
    soc_pci_write(unit, reg_addr, reg_val);
    SDK_CONFIG_MEMORY_BARRIER;

    /* Start DMA if channel not active */
    if ((sc->sc_dv_active == NULL) && (!sc->sc_dma_started)) {
        rv = cmicd_dma_chan_start(unit, sc);
        sc->sc_dma_started = 1;
    }

    return rv;
}

/*
 * Function:
 *      cmicd_dma_chan_chain_done
 * Purpose:
 *      on the CMICD DMA channel for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 *      mitigation - >0 = interrupt mitigation on
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *      INTERRUPT LEVEL ROUTINE.
 *
 */
static int
cmicd_dma_chan_chain_done(int unit, int vchan, int mitigation)
{
    return (soc_cmicm_dma_drv[unit].chan_chain_done(unit, vchan, mitigation));
}

/*
 * Function:
 *      cmicd_dma_chan_desc_done
 * Purpose:
 *      Clears desc done on the CMICD DMA channel for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *      INTERRUPT LEVEL ROUTINE.
 *
 */
static int
cmicd_dma_chan_desc_done(int unit, int vchan)
{
    int             rv = SOC_E_NONE;
    int             cmc = vchan / N_DMA_CHAN;
    int             chan = vchan % N_DMA_CHAN;

    /*
     * Older CMIC devices required a read/write/modify to clear done
     * interrupts. That does not work here for continuous DMA.  Need only
     * to set the bit for the corresponding interrupt to clear.
     */
    soc_pci_write(unit,
                  CMIC_CMCx_DMA_STAT_CLR_OFFSET(cmc),
                  DS_CNTLD_DESC_CLR(chan));

    /* Flush posted writes from PCI bridge */
    (void)soc_pci_read(unit, CMIC_CMCx_DMA_STAT_OFFSET(cmc));

    return rv;
}

/*
 * Function:
 *      cmicd_dma_chan_desc_done_intr_enable
 * Purpose:
 *      Clears desc done on the CMICD DMA channel for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *      INTERRUPT LEVEL ROUTINE.
 *
 */
static int
cmicd_dma_chan_desc_done_intr_enable(int unit, int vchan)
{
    return (soc_cmicm_dma_drv[unit].chan_desc_done_intr_enable(unit, vchan));
}

/*
 * Function:
 *      cmicd_dma_chan_reload_done
 * Purpose:
 *      Detect for reload descriptor completed
 * Parameters:
 *      unit - SOC unit #
 *      sc   - SOC channel data structure pointer
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *
 */
static int
cmicd_dma_chan_reload_done(int unit, int vchan, dcb_t * dcb, int *rld_done)
{
    int             rv = SOC_E_NONE;
    uint32          *rld_dcb;

    *rld_done = 0; /* Default to not reload */

    /* There is no pkt data to process for a reload descriptor */
    if(SOC_DCB_RELOAD_GET(unit, dcb)) {
        /* HW sets bit 31 in the dcb's last dword to indicate done */
        rld_dcb = (uint32 *)dcb;
        if (rld_dcb[DCB_LAST_DWORD] & DCB_DONE_BIT) {
            *rld_done = 1;
        }
    }

    return rv;
}

/*
 * Function:
 *      cmicd_dma_chan_tx_purge
 * Purpose:
 *      Purge partial pkt left in the pipeline from TX DMA abort.
 * Parameters:
 *      unit - SOC unit #
 *      chan - channel #
 * Returns:
 *      SOC_E_NONE   - Success
 *      SOC_E_TIMEOUT - failed (Timeout)
 * Notes:
 *      Assumes SOC_DMA_LOCK is held for CMIC_DMA_CTRL manipulation.
 *
 */
static int
cmicd_dma_chan_tx_purge(int unit, int vchan, dv_t *dv)
{
    return (soc_cmicm_dma_drv[unit].chan_tx_purge(unit, vchan, dv));
}

/*
 * Function:
 *      cmicd_dma_init
 * Purpose:
 *      Initialize the CMICD DMA for the specified SOC unit.
 * Parameters:
 *      unit - SOC unit #
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *
 */
static int
cmicd_dma_init(int unit)
{
    return (soc_cmicm_dma_drv[unit].init(unit));
}

/* Public Assignment Function */

/*
 * Function:
 *      soc_cmicd_dma_drv_init
 * Purpose:
 *      Initialize the CMICD DMA driver for the specified SOC unit.
 *      Assign function pointers for SOC DMA driver interface.
 * Parameters:
 *      unit - SOC unit #
 *      drv  - pointer to the function vector list
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_XXXX - error code
 * Notes:
 *
 */
int
soc_cmicd_dma_drv_init(int unit, soc_cmic_dma_drv_t *drv)
{
    int rv = SOC_E_NONE;

    drv->init = cmicd_dma_init;
    drv->ctrl_reset = cmicd_dma_ctrl_reset;
    drv->chan_config = cmicd_dma_chan_config;
    drv->chan_dv_start = cmicd_dma_chan_dv_start;
    drv->chan_start = cmicd_dma_chan_start;
    drv->chan_poll = cmicd_dma_chan_poll;
    drv->chan_abort = cmicd_dma_chan_abort;
    drv->chan_tx_purge = cmicd_dma_chan_tx_purge;
    drv->chan_desc_done_intr_enable = cmicd_dma_chan_desc_done_intr_enable;
    drv->chan_desc_done = cmicd_dma_chan_desc_done;
    drv->chan_chain_done = cmicd_dma_chan_chain_done;
    drv->chan_reload_done = cmicd_dma_chan_reload_done;

    soc_cmicm_dma_drv_init(unit, &soc_cmicm_dma_drv[unit]);

    return rv;
}

#endif /* BCM_CMICDV2_SUPPORT */
