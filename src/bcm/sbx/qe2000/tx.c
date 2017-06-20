/*
 * $Id: tx.c,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        tx.c
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/qe2000_util.h>

#include <bcm/error.h>
#include <bcm/tx.h>

#include <bcm_int/control.h>


#define _ARRAY_SZ(x) (sizeof(x) / sizeof(x[0]))

int sbx_qe2000_tx_pkt_setup(int unit, bcm_pkt_t *tx_pkt)
{
    uint8 hdr_size = _ARRAY_SZ(tx_pkt->_sbx_rh);
    int i, port;
    uint32 baseQueue COMPILER_ATTRIBUTE((unused));
#ifdef BCM_FE2000_SUPPORT
    uint32 queue;
#endif

    baseQueue = 0;
    /* Packets for CPU go to the PCI port, port 49 */
    if (tx_pkt->opcode & BCM_PKT_OPCODE_CPU) {
        BCM_PBMP_PORT_ADD(tx_pkt->tx_pbmp, 49);
    }

    /* one port per packet */
    BCM_PBMP_COUNT(tx_pkt->tx_pbmp, i);
    if (i != 1) {
        return BCM_E_PARAM;
    }
    sal_memset(tx_pkt->_sbx_rh, 0, hdr_size);

    switch (SOC_SBX_CFG(unit)->erh_type) {
        case SOC_SBX_G2P3_ERH_QESS:
            tx_pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_QESS;
        break;
        case SOC_SBX_G2P3_ERH_DEFAULT:
        default:
            tx_pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_DEFAULT;
        break;
    }

    /* should only occur once - */
    BCM_PBMP_ITER(tx_pkt->tx_pbmp, port) { 
        int node;

        SOC_SBX_NODE_FROM_MODID(tx_pkt->dest_mod, node);
        if (node < 0) {
            LOG_ERROR(BSL_LS_BCM_TX,
                      (BSL_META_U(unit,
                                  "invalid destination module: %d; must be a QE modid\n"),
                       tx_pkt->dest_mod));
            return BCM_E_PARAM;
        }
        baseQueue = SOC_SBX_NODE_PORT_TO_QID(unit,node, port, NUM_COS(unit));
    }

#ifdef BCM_FE2000_SUPPORT

    /* set a non-existant source id to avoid split horizion checks on the FE */
    soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                          SBX_rhf_sid, SBX_MAX_PORTS);

    if (tx_pkt->cos > NUM_COS(unit)) {
        LOG_ERROR(BSL_LS_BCM_TX,
                  (BSL_META_U(unit,
                              "Invalid COS param: %d\n"),
                   tx_pkt->cos));
        return BCM_E_PARAM;
    }
    
    /* default queue will be lowest priority (baseQueue + NUM_COS -1;
     * decrease queue number to increas priority
     */
    queue = baseQueue + ((NUM_COS(unit) - 1) - tx_pkt->cos);
    LOG_INFO(BSL_LS_BCM_TX,
             (BSL_META_U(unit,
                         "RH QID=0x%04x cos=%d baseQid=0x%04x flags=0x%x vid=0x%x _vtag=0x%x\n"),
              queue, tx_pkt->cos, baseQueue, tx_pkt->flags,
              BCM_VLAN_VALID(BCM_PKT_VLAN_ID(tx_pkt)),
              *((uint32 *)tx_pkt->_vtag)));
    
    soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                          SBX_rhf_queue_id, queue);

    if (tx_pkt->flags & BCM_PKT_F_TX_UNTAG) {
        /* no tag added, tx raw */
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                              SBX_rhf_outunion, SBX_RAW_OHI_BASE);     
        LOG_VERBOSE(BSL_LS_BCM_TX,
                    (BSL_META_U(unit,
                                "pkt flagged as untagged; sending raw\n")));

    } else if (BCM_PKT_NO_VLAN_TAG(tx_pkt) && 
               BCM_VLAN_VALID(BCM_PKT_VLAN_ID(tx_pkt))) {
        /* No tag in the packet, and valid tag provided */
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                              SBX_rhf_outunion, BCM_PKT_VLAN_ID(tx_pkt));
        LOG_VERBOSE(BSL_LS_BCM_TX,
                    (BSL_META_U(unit,
                                "No tag supplied, but set; sending tagged\n")));

    } else {
        /* send raw by default */
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                              SBX_rhf_outunion, SBX_RAW_OHI_BASE);
        LOG_VERBOSE(BSL_LS_BCM_TX,
                    (BSL_META_U(unit,
                                "default; raw\n")));

    }

    if (tx_pkt->opcode & BCM_PKT_OPCODE_MC) {
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                              SBX_rhf_mc, TRUE);
    }
 
    soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, hdr_size, SBX_rhf_test, 
                          !!(tx_pkt->flags & BCM_PKT_F_TEST));

    /* add 4 bytes to packet length to account for the shim.  The QE 
     * considers the Shim header part of the payload and if the lengths
     * mismatch, it will drop it
     */
    soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, hdr_size, SBX_rhf_length,
                          tx_pkt->tot_len + 4);
#endif /* BCM_FE2000_SUPPORT */

    return BCM_E_NONE;
}

