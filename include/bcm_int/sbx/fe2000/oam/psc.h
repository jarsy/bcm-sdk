/*
 * $Id: psc.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_PSC_H_
#define _BCM_INT_SBX_FE2000_PSC_H_

#define _BCM_FE2000_MPLS_PSC_BURST_PKT_COUNT_ (3)

#define OAM_IS_PSC(t)  ((t) == bcmOAMEndpointTypePSC)

#define _BCM_FE2000_MPLS_PSC_SUPPORTED_EPTYPE (bcmOAMEndpointTypePSC)

#define _BCM_FE2000_MPLS_PSC_SUPPORTED_FLAGS (BCM_OAM_PSC_TX_BURST  | \
                                              BCM_OAM_PSC_TX_ENABLE | \
                                              BCM_OAM_PSC_REVERTIVE | \
                                              BCM_OAM_PSC_FIRST_RX_COPY_TO_CPU | \
                                              BCM_OAM_PSC_ALL_RX_COPY_TO_CPU)

#define _BCM_FE2000_MPLS_PSC_TX_ENABLE_FLAGS_ (BCM_OAM_PSC_TX_BURST  | \
                                               BCM_OAM_PSC_TX_ENABLE)

extern int _bcm_fe2000_validate_oam_lsp_psc_endpoint(int unit, 
                                      bcm_oam_endpoint_info_t *endpoint_info);

extern int _oam_psc_endpoint_set(int unit,
                                 bcm_oam_endpoint_info_t *ep_info,
                                 uint32 ep_rec_index, 
                                 egr_path_desc_t *egrPath,
                                 tcal_id_t *tcal_id,
                                 uint32     rx_lsp_label);

extern int _oam_psc_endpoint_delete(int unit, 
                                    bcm_oam_endpoint_info_t *ep_info);

extern int _oam_psc_endpoint_get(int unit,
                                 bcm_oam_endpoint_info_t *ep_info, 
                                 uint32 ep_rec_index);

#endif  /* _BCM_INT_SBX_FE2000_PSC_H_  */
