/*
 * $Id: enet.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_ENET_H_
#define _BCM_INT_SBX_FE2000_OAM_ENET_H_

#define OAM_IS_ETHERNET(t) ((t) == bcmOAMEndpointTypeEthernet)

extern int _bcm_fe2000_init_upmep(int unit);

extern int _oam_enet_endpoint_set(int unit, 
                                  bcm_oam_endpoint_info_t *ep_info, 
                                  uint32 ep_rec_index, 
                                  egr_path_desc_t *egrPath, 
                                  tcal_id_t *tcal_id,
                                  bcm_trunk_add_info_t *trunk_info);

extern int _oam_enet_endpoint_delete(int unit, 
                                     bcm_oam_endpoint_info_t *ep_info,
                                     bcm_trunk_add_info_t *trunk_info);

extern int _bcm_fe2000_validate_oam_eth_endpoint(int unit, 
                                                 bcm_oam_endpoint_info_t *endpoint_info);

extern int _bcm_fe2000_oam_enet_coco_configure(int unit,
                                         oam_sw_hash_data_t *ep_data, 
                                         int coco_idx,
                                         uint8 pkt_pri_bitmap,
                                         int single_ended,
                                         int mep_switch_side,
                                         int count_yellow);

extern int _bcm_fe2000_oam_enet_dm_create(int unit, int flags, uint16 dmIdx, 
                                          uint16 epIdx, uint32 ftIdx, int int_pri);

#endif  /* _BCM_INT_SBX_FE2000_OAM_ENET_H_  */
