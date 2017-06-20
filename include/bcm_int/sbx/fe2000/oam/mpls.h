/*
 * $Id: mpls.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_MPLS_H_
#define _BCM_INT_SBX_FE2000_OAM_MPLS_H_

#define OAM_IS_MPLS_Y1731(t) ((t) == bcmOAMEndpointTypeMPLSPerformance)

#define OAM_MPLS_DEFAULT_TTL (0xff)
#define OAM_MPLS_DEFAULT_EXP (0) /* since control traffic set exp to 0 */
#define OAM_MPLS_LSP_GAL_LABEL (13)

#define _BCM_FE2000_MPLS_SUPPORTED_EPTYPE (bcmOAMEndpointTypeMPLSPerformance)

typedef enum bcm_fe2000_mpls_tunnel_direction_e {
    _FE2K_OUTGOING_LSP,
    _FE2K_INCOMING_LSP,
    _FE2K_MAX_DIRECTION
} bcm_fe2000_mpls_tunnel_direction_t;

extern int
_bcm_fe2000_get_tunnel_interface_info(int unit, 
                                      bcm_if_t l3_egr_ifid,
                                      bcm_fe2000_mpls_tunnel_direction_t direction);

extern int _bcm_fe2000_validate_oam_mpls_endpoint(int unit, 
                                                  bcm_oam_endpoint_info_t *endpoint_info);

extern int _oam_mpls_egr_path_update (int unit,
                                      egr_path_desc_t *egrPath,  
                                      bcm_oam_endpoint_info_t *endpoint_info);

#if defined(INCLUDE_BFD)
extern int _oam_mpls_bfd_egr_path_update (int unit,
                                      egr_path_desc_t *egrPath,  
                                      bcm_bfd_endpoint_info_t *endpoint_info);
#endif /* defined(INCLUDE_BFD) */


extern int _oam_mpls_endpoint_set(int unit,
                                  bcm_oam_endpoint_info_t *ep_info, 
                                  uint32 ep_rec_index, 
                                  egr_path_desc_t *egrPath,
                                  tcal_id_t *tcal_id) ;

extern int _oam_mpls_endpoint_get(int unit,
                                  bcm_oam_endpoint_info_t *ep_info, 
                                  uint32 ep_rec_index);

extern int _oam_mpls_endpoint_delete(int unit, 
                                     bcm_oam_endpoint_info_t *ep_info);

/*
 *   Function
 *     _bcm_fe2000_mpls_oam_loss_set
 *   Purpose
 *      Provision Delay measurement on MPLS LSP OAM endpoint
 *   Parameters
 *       unit        = BCM device number
 *       delay_ptr   = delay parameter
 *       local_ep    = mpls oam local endpoint datum
 *       add_lm_endpoint = TRUE(add loss measurement)
 *                         FALSE(delete loss measurement)
 *   Returns
 *       BCM_E_*
 */
extern  int 
_bcm_fe2000_mpls_oam_loss_set(int unit, 
                              bcm_oam_loss_t *loss_ptr,
                              oam_sw_hash_data_t *local_ep,
                              uint8 add_lm_endpoint);

/*
 *   Function
 *     _bcm_fe2000_mpls_oam_delay_set
 *   Purpose
 *      Provision Delay measurement on MPLS LSP OAM endpoint
 *   Parameters
 *       unit        = BCM device number
 *       delay_ptr   = delay parameter
 *       local_ep    = mpls oam local endpoint datum
 *       peer_dp     = mpls peer oam endpoint datum
 *       add_dm_endpoint = TRUE(add delay measurement)
 *                         FALSE(delete delay measurement)
 *   Returns
 *       BCM_E_*
 */
extern 
int _bcm_fe2000_mpls_oam_delay_set(int unit, 
                                   bcm_oam_delay_t *delay_ptr,
                                   oam_sw_hash_data_t *local_ep,
                                   oam_sw_hash_data_t *peer_ep,
                                   uint8 add_dm_endpoint); 

extern int
_bcm_fe2000_lsp_intf_ep_list_add(int unit, 
                                 bcm_if_t l3_egr_ifid,
                                 oam_sw_hash_data_t *hash_data);

int
extern _bcm_fe2000_lsp_intf_ep_list_remove(int unit, 
                                           bcm_if_t l3_egr_ifid,
                                           oam_sw_hash_data_t *hash_data);

#endif  /* _BCM_INT_SBX_FE2000_OAM_MPLS_H_  */


