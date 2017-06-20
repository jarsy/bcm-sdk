/*
 * $Id: bfd.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_BFD_H_
#define _BCM_INT_SBX_FE2000_BFD_H_

#if defined(INCLUDE_BFD)

#include <bcm/bfd.h>


/* BFD currently only supports point to point bidirecitonal BFD */
#define OAM_BFD_MAX_FE2000_ENDPOINT (2)

#define OAM_IS_LSP_BFD(t)  ((t) == bcmBFDTunnelTypeMplsTpLspCc      || \
                            (t) == bcmBFDTunnelTypeMplsTpLspCcCv)

#define OAM_IS_BFD(t)   ((t) == bcmBFDTunnelTypeMplsTpPwCc          || \
                         (t) == bcmBFDTunnelTypeMplsTpPwCcCv )


#define _BCM_FE2000_BFD_SUPPORTED_FLAGS (BCM_BFD_ENDPOINT_PWE_RAW | \
                                         BCM_BFD_ENDPOINT_MPLS_TP_POLL_SEQUENCE_ENABLE )

#define _BCM_FE2000_BFD_SUPPORTED_EPTYPE (bcmBFDTunnelTypeMplsTpLspCc    | \
                                          bcmBFDTunnelTypeMplsTpLspCcCv  | \
                                          bcmBFDTunnelTypeMplsTpPwCc     | \
                                          bcmBFDTunnelTypeMplsTpPwCcCv)

extern int _bcm_fe2000_validate_oam_lsp_bfd_endpoint(int unit, 
                                                 bcm_bfd_endpoint_info_t *endpoint_info);
extern int _bcm_fe2000_validate_oam_bfd_endpoint(int unit, 
                                                 bcm_bfd_endpoint_info_t *endpoint_info);

extern int _oam_bfd_endpoint_set(int unit,
                                 bcm_bfd_endpoint_info_t *ep_info, 
                                 uint32 ep_rec_index, 
                                 egr_path_desc_t *egrPath, 
                                 tcal_id_t *tcal_id,
                                 uint32 label);

extern int _oam_bfd_endpoint_get(int unit,
                                 bcm_bfd_endpoint_info_t *ep_info, 
                                 uint32 ep_rec_index);
extern int _oam_bfd_endpoint_delete(int unit, 
                                    bcm_bfd_endpoint_info_t *ep_info);

extern int _oam_bfd_egr_path_update (int unit,
                                     egr_path_desc_t *egrPath,  
                                     bcm_bfd_endpoint_info_t *ep_info,
                                     _fe2k_vpn_sap_t *vpn_sap);

extern int _oam_fe2000_bfd_chain_psc (int      unit, 
                                      uint32 bfd_ep_index,
                                      uint32 psc_ep_index,
                                      uint8  chain /* 0 -unchain, 1- chain*/);

#endif /* defined(INCLUDE_BFD) */

#endif  /* _BCM_INT_SBX_FE2000_BFD_H_  */
