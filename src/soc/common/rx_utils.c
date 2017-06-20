/*
 * $Id: rx.c,v 1.1.10.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        rx_utils.c
 * Purpose:
 * Requires:
 */


#include <shared/bsl.h>

#include <sal/core/boot.h>
#include <sal/core/dpc.h>
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/mem.h>

#include <soc/rx.h>

#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>

int
soc_rx_queue_channel_set(int unit, int queue_id,
                         int chan_id)
{
    uint32 ix;
    uint32 chan_id_max = SOC_RX_CHANNELS;
    uint32 reg_addr, reg_val;
    int cmc;
    int pci_cmc = SOC_PCI_CMC(unit);
    uint32 chan_off;
    int startq = 0;
    int numq, endq, countq;
    int start_chan_id;
#ifdef BCM_KATANA_SUPPORT
    uint32 i = 0;
#endif

    if (0 != SOC_CMCS_NUM(unit)) {
        chan_id_max *= SOC_CMCS_NUM(unit);
    }

    if (SOC_WARM_BOOT(unit)) {
        return SOC_E_NONE;
    }
    if ((chan_id < 0) || (chan_id >= chan_id_max)) {
        /* Verify the chan id */
        return SOC_E_PARAM;
    } else if (queue_id >= NUM_CPU_COSQ(unit)) {
            return SOC_E_PARAM;
    }

    if(soc_feature(unit, soc_feature_cmicm)) {
        /* We institute the normalizing assumption that the
         * channels are numbered first in the PCI CMC, then in order of the
         * ARM CMCs.  This is why we have the shuffling steps below.
         */
        if (chan_id < SOC_RX_CHANNELS) {
            cmc = pci_cmc;
        } else {
            cmc = SOC_ARM_CMC(unit, ((chan_id / SOC_RX_CHANNELS) - 1));
            /* compute start queue number for any CMC */
            startq = NUM_CPU_ARM_COSQ(unit, pci_cmc);
            for (ix = 0; ix < cmc; ix++) {
                startq += (ix != pci_cmc) ? NUM_CPU_ARM_COSQ(unit, ix) : 0;
            }
        }

        numq = NUM_CPU_ARM_COSQ(unit, cmc);
        start_chan_id = (cmc != pci_cmc) ? cmc * SOC_RX_CHANNELS : 0;

        if (queue_id < 0) { /* All COS Queues */
            /* validate the queue range of CMC is in the valid range
             * for this CMC
             */
            SHR_BITCOUNT_RANGE(CPU_ARM_QUEUE_BITMAP(unit, cmc),
                    countq, startq, numq);
            if (numq != countq) {
                return SOC_E_PARAM;
            }

            /* We know chan_id != -1 from the parameter check at the
             * start of the function */
            endq = startq + NUM_CPU_ARM_COSQ(unit, cmc);
            for (ix = start_chan_id; ix < (start_chan_id + SOC_RX_CHANNELS); ix++) {
                /* set CMIC_CMCx_CHy_COS_CTRL_RX_0/1 based on CMC's start
                 * and end queue number
                 */
                chan_off = ix % SOC_RX_CHANNELS;
                reg_val = 0;
                if (ix == (uint32)chan_id) {
                    reg_val |= (endq < 32) ?
                        ((uint32)1 << endq) - 1 : 0xffffffff;
                    reg_val &= (startq < 32) ?
                        ~(((uint32)1 << startq) - 1) : 0;
                }

                reg_addr = CMIC_CMCx_CHy_COS_CTRL_RX_0_OFFSET(cmc, chan_off);
                /* Incoporate the reserved queues (if any on this device)
                 * into the CMIC programming,  */
                reg_val |= CPU_ARM_RSVD_QUEUE_BITMAP(unit,cmc)[0];
                soc_pci_write(unit, reg_addr, reg_val);
                reg_addr = CMIC_CMCx_CHy_COS_CTRL_RX_1_OFFSET(cmc, chan_off);

#if defined(BCM_KATANA_SUPPORT)
                if (SOC_IS_KATANAX(unit)) {
                    /*
                     * Katana queues have been disabled to prevent packets
                     * that cannot egress from reaching the CMIC. At this
                     * point those queues can be enabled.
                     */
                    for (i = startq; i < endq; i++) {
                        reg_val = 0;
                        soc_reg_field_set(unit,
                                THDO_QUEUE_DISABLE_CFG2r, &reg_val, QUEUE_NUMf, i);
                        SOC_IF_ERROR_RETURN(
                                WRITE_THDO_QUEUE_DISABLE_CFG2r(unit, reg_val));
                        reg_val = 0;
                        soc_reg_field_set(unit,
                                THDO_QUEUE_DISABLE_CFG1r, &reg_val, QUEUE_WRf, 1);
                        SOC_IF_ERROR_RETURN(
                                WRITE_THDO_QUEUE_DISABLE_CFG1r(unit, reg_val));
                    }
                }
#endif /*BCM_KATANA_SUPPORT */
                reg_val = 0;
                /* Incoporate the reserved queues (if any on this device)
                 * into the CMIC programming,  */
                reg_val |= CPU_ARM_RSVD_QUEUE_BITMAP(unit,cmc)[1];

                if (ix == (uint32)chan_id) {
                    reg_val |= ((endq >= 32) && (endq < 64)) ?
                        (((uint32)1 << (endq % 32)) - 1) :
                        ((endq < 32) ? 0 : 0xffffffff);
                    reg_val &= (startq >= 32) ?
                        ~(((uint32)1 << (startq % 32)) - 1) : 0xffffffff;
                    if (SOC_IS_TD2P_TT2P(unit)) {
                        soc_pci_write(unit, reg_addr, reg_val);
                    }

                }
                if (!SOC_IS_TD2P_TT2P(unit)) {
                    soc_pci_write(unit, reg_addr, reg_val);
                }
            }
        } else {
            return SOC_E_PARAM;
        }
    } else {
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}
#endif

