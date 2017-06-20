/*
 * $Id: tx.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        tx.c
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/higig.h>

#include <bcm/error.h>
#include <bcm/tx.h>

#include <bcm_int/control.h>


#define _ARRAY_SZ(x) (sizeof(x) / sizeof(x[0]))

#define G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES 12

int sbx_sirius_tx_pkt_setup(int unit, bcm_pkt_t *tx_pkt)
{
    int i, port, hdr_size = 0;
    soc_higig_hdr_t *pkt_hg_hdr = (soc_higig_hdr_t *) tx_pkt->pkt_data->data;
    uint32 baseQueue COMPILER_ATTRIBUTE((unused));
#ifdef BCM_FE2000_SUPPORT
    uint32 queue;
#endif

    baseQueue  = 0;
    if (!BCM_PKT_TX_ETHER(tx_pkt)) {
      if (!BCM_PKT_TX_HG_READY(tx_pkt)) {
	  hdr_size = SOC_HIGIG2_HDR_SIZE;
	  sal_memcpy(tx_pkt->_higig, pkt_hg_hdr, hdr_size);
      }
      
      /*
       * Now that the higig header has been moved to the _higig buffer,
       * move the data to the beginning of the buffer
       */
      
      for (i=hdr_size; i < tx_pkt->pkt_data->len; i++) 
	tx_pkt->pkt_data->data[i-hdr_size] = tx_pkt->pkt_data->data[i];
      
      /*
       * zero the remaining buffer 
       */

      sal_memset(&tx_pkt->pkt_data->data[i - hdr_size], 0, hdr_size);
      
      /*
       * Remove the header size from the packet length
       */
      
      tx_pkt->pkt_data->len -= hdr_size;
      tx_pkt->tot_len -= hdr_size;

    } else {

      /*
       * Has SBX RH
       */

      hdr_size = _ARRAY_SZ(tx_pkt->_sbx_rh);
      sal_memset(tx_pkt->_sbx_rh, 0, hdr_size);
	  
      switch (SOC_SBX_CFG(unit)->erh_type) {
          case SOC_SBX_G2P3_ERH_SIRIUS:
              tx_pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_SIRIUS;
          break;
          case SOC_SBX_G2P3_ERH_DEFAULT:
          default:
              tx_pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_DEFAULT;
          break;
      }

      /*
       * If dest_mod uninitialized, 
       * use default
       */
      if (tx_pkt->dest_mod == 0)
	tx_pkt->dest_mod = SBX_QE_BASE_MODID;

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
        return BCM_E_PARAM;
      }
      
      /* set KSOP */
      soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                            SBX_rhf_ksop, SOC_HIGIG_START);

      /* default queue will be lowest priority (baseQueue + NUM_COS -1;
       * decrease queue number to increas priority
       */
      queue = baseQueue + ((NUM_COS(unit) - 1) - tx_pkt->cos);
      LOG_INFO(BSL_LS_BCM_TX,
               (BSL_META_U(unit,
                           "RH QID=0x%04x cos=%d baseQid=0x%04x\n"),
                queue, tx_pkt->cos, baseQueue));
      
      soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
			    SBX_rhf_queue_id, queue);
      /* coverity[result_independent_of_operands] */
      if (!BCM_PKT_NO_VLAN_TAG(tx_pkt) &&		\
	  BCM_VLAN_VALID(BCM_PKT_VLAN_ID(tx_pkt))) {
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
			      SBX_rhf_outunion, BCM_PKT_VLAN_ID(tx_pkt));
      } else {
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
			      SBX_rhf_outunion, SBX_RAW_OHI_BASE);
      }
      
      if (tx_pkt->opcode & BCM_PKT_OPCODE_MC) {
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                              SBX_rhf_mc, TRUE);
      }
      
      soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, hdr_size, SBX_rhf_test, 
			    !!(tx_pkt->flags & BCM_PKT_F_TEST));
      
#endif /* BCM_FE2000_SUPPORT */
      
    }
    return BCM_E_NONE;
}

