/*
 * $Id: g2p3.h,v 1.85 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        g2p3.h
 * Purpose:     g2p3 internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_G2P3_H_
#define _BCM_INT_SBX_FE2000_G2P3_H_

#include <bcm/types.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/mpls.h>
#include <bcm/stg.h>
#include <bcm/multicast.h>
#include <bcm/qos.h>
#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/fe2000/mpls.h>
#include <soc/sbx/g2p3/g2p3.h>


/*  PORT */
extern int _bcm_fe2000_g2p3_port_ilib_common_init(int unit);
extern int _bcm_fe2000_g2p3_port_ilib_common_detach(int unit);
extern int _bcm_fe2000_g2p3_port_ilib_entry_init(int unit,
                                                 bcm_port_t port,
                                                 bcm_vlan_data_t *vd);
extern int _bcm_fe2000_g2p3_port_ilib_egr_init(int unit, bcm_port_t port);
extern int _bcm_fe2000_g2p3_port_ilib_tpid_init(int unit, bcm_port_t port);
extern int _bcm_fe2000_g2p3_port_ilib_fte_init(int unit, int node,
                                               bcm_port_t port);
extern int _bcm_fe2000_g2p3_port_ilib_well_known_egr_init(int unit);
extern int _bcm_fe2000_g2p3_port_qos_init(int unit);
extern int _bcm_fe2000_g2p3_port_qos_detach(int unit);
extern int _bcm_fe2000_g2p3_port_ilib_lp_init(int unit, bcm_module_t modid,
                                              bcm_port_t port);
extern int _bcm_fe2000_g2p3_port_vlan_lp_set(int unit, bcm_module_t modid,
                                             bcm_port_t port, bcm_vlan_t vlan);
extern int _bcm_fe2000_g2p3_port_tpid_set(int unit, bcm_port_t port,
                                          uint16 tpid);
extern int _bcm_fe2000_g2p3_port_tpid_get(int unit, bcm_port_t port,
                                          uint16 *tpid);
extern int _bcm_fe2000_g2p3_port_inner_tpid_get(int unit,
                                                bcm_port_t port,
                                                uint16 *tpid);
extern int _bcm_fe2000_g2p3_port_inner_tpid_set(int unit,
                                                bcm_port_t port,
                                                uint16 tpid);
extern int _bcm_fe2000_g2p3_port_tpid_add(int unit, bcm_port_t port,
                                          uint16 tpid, int color_select);
extern int _bcm_fe2000_g2p3_port_tpid_delete(int unit, bcm_port_t port,
                                             uint16 tpid);
extern int _bcm_fe2000_g2p3_port_tpid_delete_all(int unit, bcm_port_t port);
extern int _bcm_fe2000_g2p3_port_dtag_mode_set(int unit, bcm_port_t port,
                                               int mode);
extern int _bcm_fe2000_g2p3_port_dtag_mode_get(int unit, bcm_port_t port,
                                               int *mode);
extern int _bcm_fe2000_g2p3_port_untagged_priority_get(int unit,
                                                       bcm_port_t port,
                                                       int *priority);
extern int _bcm_fe2000_g2p3_port_vlan_priority_map_set(int unit,
                                                       bcm_port_t port,
                                                       int pkt_pri,
                                                       int cfi,
                                                       int internal_pri,
                                                       bcm_color_t color,
                                                       int remark_pri,
                                                       int remark_color,
                                                       int policer_off);
extern int _bcm_fe2000_g2p3_port_vlan_priority_map_get(int unit,
                                                       bcm_port_t port,
                                                       int pkt_pri,
                                                       int cfi,
                                                       int *internal_pri,
                                                       bcm_color_t *color,
                                                       int *remark_pri,
                                                       bcm_color_t *remark_color,
                                                       int *policer_off);
extern int _bcm_fe2000_g2p3_port_vlan_priority_unmap_get(int unit,
                                                         bcm_port_t port,
                                                         int internal_pri,
                                                         bcm_color_t color,
                                                         int *pkt_pri,
                                                         int *cfi);
extern int _bcm_fe2000_g2p3_port_vlan_priority_unmap_set(int unit,
                                                         bcm_port_t port,
                                                         int internal_pri,
                                                         bcm_color_t color,
                                                         int pkt_pri,
                                                         int cfi);
extern int _bcm_fe2000_g2p3_port_dscp_map_mode_set(int unit, bcm_port_t port, int mode);

extern int _bcm_fe2000_g2p3_port_dscp_map_mode_get(int unit, bcm_port_t port, int *mode);

extern int _bcm_fe2000_g2p3_port_dscp_unmap_set(int unit,
                                                bcm_port_t port,
                                                int internal_pri,
                                                bcm_color_t color,
                                                int pkt_dscp);

extern int _bcm_fe2000_g2p3_port_dscp_unmap_get(int unit,
                                                bcm_port_t port,
                                                int internal_pri,
                                                bcm_color_t color,
                                                int *pkt_dscp);
extern int _bcm_fe2000_g2p3_port_vlan_priority_dscp_map_get(int unit,
                                                            bcm_port_t port,
                                                            int pkt_dscp,
                                                            int *internal_pri,
                                                            bcm_color_t *color,
                                                            int *remark_pri,
                                                            bcm_color_t *remark_color,
                                                            int *policer_offset);

extern int _bcm_fe2000_g2p3_port_vlan_dscp_map_get(int unit,
                                                  bcm_port_t port,
                                                  bcm_vlan_t vid,
                                                  int pkt_dscp,
                                                  int *internal_pri,
                                                  bcm_color_t *color);


extern int _bcm_fe2000_g2p3_port_dscp_map_get(int unit,
                                             bcm_port_t port,
                                             int pkt_dscp,
                                             int *internal_pri,
                                             int *color);

extern int _bcm_fe2000_g2p3_port_dscp_map_set(int unit,
                                   bcm_port_t port, 
                                   int dscp, 
                                   int internal_pri, 
                                   bcm_color_t color);

extern int _bcm_fe2000_g2p3_port_vlan_dscp_map_set(int unit,
                                                   bcm_port_t port, 
                                                   bcm_vlan_t vlan, 
                                                   int dscp, 
                                                   int internal_pri, 
                                                   bcm_color_t color);
extern int _bcm_fe2000_g2p3_port_vlan_pri_map_set(int unit,
                                                  bcm_port_t port,
                                                  bcm_vlan_t vid,
                                                  int pkt_pri,
                                                  int cfi,
                                                  int internal_pri,
                                                  int color);
extern int _bcm_fe2000_g2p3_port_vlan_pri_map_get(int unit,
                                                  bcm_port_t port,
                                                  bcm_vlan_t vid,
                                                  int pkt_pri,
                                                  int cfi,
                                                  int *internal_pri,
                                                  bcm_color_t *color);
extern int _bcm_fe2000_g2p3_port_vlan_priority_mapping_get(int unit,
                                                           bcm_port_t port,
                                                           bcm_vlan_t vid,
                                                           int pkt_pri,
                                                           int cfi,
                                                           bcm_priority_mapping_t *pri_map);
extern int
_bcm_fe2000_g2p3_port_vlan_priority_mapping_set(int unit,
                                                bcm_port_t port,
                                                bcm_vlan_t vid,
                                                int pkt_pri,
                                                int cfi,
                                                bcm_priority_mapping_t *pri_map);
extern int _bcm_fe2000_g2p3_port_strip_tag(int unit, bcm_port_t port, 
                                           int strip);
extern int _bcm_fe2000_g2p3_port_lp_discard_access(int unit, 
                                                   bcm_port_t port, 
                                                   int *discardTagged, 
                                                   int *discardUntagged,
                                                   int set);
extern int _bcm_fe2000_g2p3_port_lp_untagged_vlan_access(int unit, 
                                                         bcm_port_t port, 
                                                         bcm_vlan_t *vid,
                                                         int set);
extern int _bcm_fe2000_g2p3_port_lp_untagged_priority_access(int unit,
                                                             bcm_port_t port,
                                                             int *priority,
                                                             int set);

extern int _bcm_fe2000_g2p3_port_frame_max_access(int unit, bcm_port_t port,
                                                  int *size, int set);
extern int
_bcm_fe2000_g2p3_port_default_qos_profile_set(int unit, bcm_port_t port, 
                                              bcm_vlan_t vid);
extern int _bcm_fe2000_g2p3_port_qosmap_set(int unit, bcm_port_t port, 
                                            int ingrMap, int egrMap,
                                            uint32 ingFlags, uint32 egrFlags);
extern int _bcm_fe2000_g2p3_port_qosmap_get(int unit,bcm_port_t  port, 
                                            int *ing_map, int *egr_map,
                                            uint32 *ing_flags, uint32 *egr_flags);
extern int _bcm_fe2000_g2p3_port_vlan_qosmap_set(int unit, bcm_port_t port, 
                                                 bcm_vlan_t vid,
                                                 int ing_idx, int egr_idx,
                                                 uint32 ing_flags, uint32 egr_flags);
extern int _bcm_fe2000_g2p3_port_vlan_qosmap_get(int unit, bcm_port_t port, 
                                                 bcm_vlan_t vid,
                                                 int *ing_idx, int *egr_idx,
                                                 uint32 *ing_flags, uint32 *egr_flags);

extern int bcm_fe2k_egr_path_get(int unit, uint32 fti,
                                 soc_sbx_g2p3_ft_t *ft,
                                 soc_sbx_g2p3_oi2e_t *oh,
                                 soc_sbx_g2p3_eteencap_t *eteencap,
                                 soc_sbx_g2p3_etel2_t *etel2,
                                 uint32               *etei);

/* STG */
extern int _bcm_fe2000_g2p3_stg_init(int unit);
extern uint32 _bcm_fe2000_g2p3_stg_stp_translate(int unit,
                                                   int bcm_state);
extern bcm_stg_stp_t _bcm_fe2000_g2p3_stg_stp_translate_to_bcm(int unit,
                                                               int stpstate);
extern int _bcm_fe2000_g2p3_stg_vid_stp_set(int unit,
                                            bcm_vlan_t vid,
                                            bcm_port_t port,
                                            int stp_state);
extern int _bcm_fe2000_g2p3_stg_vid_stp_get(int unit,
                                            bcm_vlan_t vid,
                                            bcm_port_t port,
                                            int *stp_state);
extern int _bcm_fe2000_g2p3_stg_stacked_vid_stp_set_get(int unit,
                                                    bcm_vlan_t ovid,
                                                    bcm_vlan_t ivid,
                                                    bcm_port_t port,
                                                    int *stp_state,
                                                    int set_or_get);
extern int _bcm_fe2000_g2p3_stg_label_stp_set_get(int unit, 
                                                  bcm_mpls_label_t label,
                                                  int *stp_state,
                                                  int set_or_get);
extern int _bcm_fe2000_g2p3_stg_fast_stp_set(int unit, bcm_stg_t stg,
                                             bcm_port_t port, int stp_state);
extern int _bcm_fe2000_g2p3_stp_fast_set(int unit, bcm_port_t port, 
                                         uint32 stpState,
                                         int fastSets[BCM_VLAN_COUNT]);

/* IGMP */
extern int _bcm_fe2000_g2p3_igmp_snooping_init(int unit);

/* L2 */
extern int _bcm_fe2000_g2p3_l2_hw_init(int unit);
extern int _bcm_fe2000_g2p3_l2addr_hw_add(int unit, bcm_l2_addr_t *l2addr);
extern int _bcm_fe2000_g2p3_smac_dump(int unit, bcm_mac_t mac,
                                      bcm_vlan_t vid, int max_count);
extern int _bcm_fe2000_g2p3_dmac_dump(int unit, bcm_mac_t mac,
                                      bcm_vlan_t vid, int max_count);
extern int _bcm_fe2000_g2p3_l2_addr_delete_by(int unit, bcm_l2_addr_t *match,
                                              uint32 cmp_flags);
extern int _bcm_fe2000_g2p3_l2_addr_get(int unit, sal_mac_addr_t mac,
                                        bcm_vlan_t vid,
                                        bcm_l2_addr_t *l2addr);
extern int _bcm_fe2000_g2p3_l2_addr_delete(int unit, bcm_mac_t mac,
                                           bcm_vlan_t vid);
extern int _bcm_fe2000_g2p3_l2_flush_cache(int unit);
extern int _bcm_fe2000_g2p3_l2_addr_update_dest(int unit,
                                                bcm_l2_addr_t *l2addr,
                                                int qidunion);

extern int _bcm_fe2000_g2p3_l2_mac_size_get(int unit, int *table_size);
extern int _bcm_fe2000_g2p3_l2_age_ager_set(int unit, uint32 ager);
extern int _bcm_fe2000_g2p3_l2_age_remove(int unit, int *mackey, bcm_l2_addr_t *payload);


/* L2 Egress */
extern int _bcm_fe2000_g2p3_l2_egress_hw_entry_delete(int unit,
                                                      bcm_if_t encap_id);
extern int _bcm_fe2000_g2p3_l2_egress_hw_entry_add(int unit,
                                                   bcm_if_t encap_id,
                                                   bcm_l2_egress_t *egr);
extern int _bcm_fe2000_g2p3_l2_egress_hw_entry_get(int unit,
                                                   bcm_if_t encap_id,
                                                   bcm_l2_egress_t *egr);

/* Multicast */
extern int _bcm_fe2000_g2p3_multicast_l2_encap_get(int   unit,
                                        bcm_multicast_t  group,
                                        bcm_gport_t      gport,
                                        bcm_vlan_t       vlan,
                                        bcm_if_t        *encap_id);

/* MPLS */
extern int _bcm_fe2000_g2p3_mpls_update_vpxs_tunnel_hw(_fe2k_l3_fe_instance_t  *l3_fe,
                                                       _fe2k_vpn_sap_t         *vpn_sap,
                                                       bcm_mpls_port_t         *mpls_port);
extern int _bcm_fe2000_g2p3_mpls_update_vpxs_hw(_fe2k_l3_fe_instance_t  *l3_fe,
                                                _fe2k_vpn_sap_t         *vpn_sap,
                                                bcm_mpls_port_t         *mpls_port);
extern int _bcm_fe2000_g2p3_mpls_free_vpncb(_fe2k_l3_fe_instance_t  *l3_fe,
                                            _fe2k_vpn_control_t    **vpnc);

extern int _bcm_fe2000_g2p3_mpls_map_set_vlan2etc(_fe2k_l3_fe_instance_t   *l3_fe,
                                                  _fe2k_vpn_control_t      *vpnc,
                                                  int                       program_vsi);

extern int _bcm_fe2000_g2p3_mpls_map_set_vpn_bc_fte(_fe2k_l3_fe_instance_t  *l3_fe,
                                                    _fe2k_vpn_control_t     *vpnc);
extern int _bcm_fe2000_g2p3_mpls_invalidate_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t *l3_fe,
                                                                 _fe2k_vpn_sap_t    *vpn_sap);
extern int _bcm_fe2000_g2p3_mpls_map_set_mpls_vpn_fte(_fe2k_l3_fe_instance_t  *l3_fe,
                                                      _fe2k_vpn_control_t     *vpnc,
                                                      _fe2k_vpn_sap_t         *vpn_sap,
                                                      bcm_mpls_port_t         *mpls_port);
extern int _bcm_fe2000_g2p3_mpls_match_port2etc(_fe2k_l3_fe_instance_t  *l3_fe,
                                                int                      action,
                                                int                      logicalPort,
                                                _fe2k_vpn_control_t     *vpnc,
                                                _fe2k_vpn_sap_t         *vpn_sap,
                                                bcm_mpls_port_t         *mpls_port);
extern int _bcm_fe2000_g2p3_mpls_match_pvid2etc(_fe2k_l3_fe_instance_t  *l3_fe,
                                                int                      action,
                                                uint32                 logicalPort,
                                                _fe2k_vpn_control_t     *vpnc,
                                                _fe2k_vpn_sap_t         *vpn_sap,
                                                bcm_mpls_port_t         *mpls_port);
extern int _bcm_fe2000_g2p3_mpls_match_pstackedvid2etc(_fe2k_l3_fe_instance_t  *l3_fe,
                                                       int                      action,
                                                       uint32                 logicalPort,
                                                       _fe2k_vpn_control_t     *vpnc,
                                                       _fe2k_vpn_sap_t         *vpn_sap,
                                                       bcm_mpls_port_t         *mpls_port);
extern int _bcm_fe2000_g2p3_mpls_program_vpn_sap_vlan2etc(_fe2k_l3_fe_instance_t   *l3_fe,
                                                          _fe2k_vpn_control_t      *vpnc,
                                                          _fe2k_vpn_sap_t          *vpn_sap);
extern int _bcm_fe2000_g2p3_mpls_vpws_fte_connect(_fe2k_l3_fe_instance_t *l3_fe,
                                                  uint32 p1_fte_idx,
                                                  uint32 p2_fte_idx,
                                                  uint32 connect);
extern int _bcm_fe2000_g2p3_mpls_lp_get(int unit, bcm_mpls_port_t *mpls_port,
                                        uint32 *lp);
extern int _bcm_fe2000_g2p3_mpls_lp_write(int unit, uint32 logicalPort,
                                          _fe2k_vpn_sap_t   *vpn_sap,
                                          bcm_mpls_port_t   *mpls_port,
                                          int                action);
extern int _bcm_fe2000_g2p3_mpls_get_fte(_fe2k_l3_fe_instance_t *l3_fe,
                                         uint32                  fte_idx,
                                         int                     action,
                                         _fe2k_l3_or_mpls_egress_t *egr);
extern int _bcm_fe2000_g2p3_mpls_port_discard_set(int unit, bcm_port_t port,
                                                  int mode);
extern int
_bcm_fe2000_g2p3_mpls_tunnel_ete_set(int                      unit,
                                     _fe2k_l3_fe_instance_t  *l3_fe,
                                     _fe2k_l3_intf_t         *l3_intf,
                                     _fe2k_l3_ete_t          *v4_ete,
                                     int                      num_labels,
                                     bcm_mpls_egress_label_t *label_array,
                                     _fe2k_l3_ete_t          *mpls_sw_ete,
                                     int                      ete_allocated);

extern int
_bcm_fe2000_g2p3_mpls_tunnel_switch_update(int                       unit,
                                           _fe2k_l3_fe_instance_t   *l3_fe,
                                           bcm_mpls_tunnel_switch_t *info);

extern int
_bcm_fe2000_g2p3_map_set_l2_ete_for_mpls(_fe2k_l3_fe_instance_t  *l3_fe,
                                    _fe2k_l3_intf_t         *l3_intf,
                                    _fe2k_l3_ete_t          *mpls_ete);
extern int
_bcm_fe2000_g2p3_enable_mpls_tunnel(_fe2k_l3_fe_instance_t *l3_fe,
                               _fe2k_l3_intf_t        *l3_intf,
                               _fe2k_l3_ete_t         *v4_ete);

/* L3 */
extern int _bcm_fe2000_g2p3_l3_invalidate_l3_or_mpls_fte(_fe2k_l3_fe_instance_t   *l3_fe,
                                                         uint32                    fte_idx);
extern int _bcm_fe2000_g2p3_l3_hw_init(int unit, _fe2k_l3_fe_instance_t *l3_fe);

extern int _bcm_fe2000_g2p3_l3_update_smac_tables(_fe2k_l3_fe_instance_t *l3_fe,
                                                  bcm_mac_t               smac,
                                                  int                     op,
                                                  uint32                  ismacIdx,
                                                  uint32                  esmacIdx);

extern int _bcm_fe2000_g2p3_map_l2_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                       _fe2k_l3_ete_vidop_t       vidop,
                       uint32                     smac_idx,
                       uint32                     vid,
                       uint32                     mtu,
                       soc_sbx_g2p3_etel2_t      *l2_hw_ete);

extern int _bcm_fe2000_g2p3_set_l2_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                       _fe2k_ete_idx_t           *ete_idx,
                       soc_sbx_g2p3_etel2_t      *l2_hw_ete);
int
_bcm_fe2000_g2p3_update_l2_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                          _fe2k_ete_idx_t           *ete_idx,
                          uint32                     smac_idx,
                          uint32                     vid,
                          uint32                     mtu,
                          uint32                     change_flags);

extern int _bcm_fe2000_g2p3_map_l3_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                                       _fe2k_l3_ete_t            *sw_ete,
                                       _fe2k_ete_idx_t           *l2_ete,
                                       bcm_port_t                 port,
                                       uint32                     flags,
                                       soc_sbx_g2p3_eteencap_t   *hw_ete,
                                       soc_sbx_g2p3_oi2e_t       *hw_ohi2etc);

extern int _bcm_fe2000_g2p3_set_l3_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                                       _fe2k_l3_ete_t            *l3_sw_ete,
                                       soc_sbx_g2p3_eteencap_t   *hw_ete,
                                       soc_sbx_g2p3_oi2e_t       *hw_ohi2etc);

extern int _bcm_fe2000_g2p3_update_ipv4_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                            _fe2k_l3_ete_t            *l3_sw_ete);

extern int _bcm_fe2000_g2p3_map_set_l3_ucast_fte(_fe2k_l3_fe_instance_t  *l3_fe,
                                                 _fe2k_fte_idx_t        *fte_idx, 
                                                 bcm_l3_egress_t        *bcm_egr);

extern int _bcm_fe2000_g2p3_l3_get_egrif_from_fte(_fe2k_l3_fe_instance_t *l3_fe,
                       uint32                    fte_idx,
                       uint32                    flags,
                       bcm_l3_egress_t          *bcm_egr);

extern int _bcm_fe2000_g2p3_get_local_l3_egress_from_ohi(_fe2k_l3_fe_instance_t *l3_fe,
                                                         bcm_l3_egress_t        *bcm_egr);

extern int bcm_fe2000_g2p3_l3_route_add( _fe2k_l3_fe_instance_t  *l3_fe,
                              bcm_l3_route_t          *info,
                              uint32                   vid,
                              uint32                   ecmp_size);

extern int _bcm_fe2000_g2p3_ip_route_delete(_fe2k_l3_fe_instance_t *l3_fe,
                                            bcm_l3_route_t *info);

extern int _bcm_fe2000_g2p3_ip_da_route_get(_fe2k_l3_fe_instance_t  *l3_fe,
                                            bcm_l3_route_t          *info,
                                            _fe2k_g2p3_da_route_t   *da_info);

extern int _bcm_fe2000_g2p3_ip_sa_route_get(_fe2k_l3_fe_instance_t  *l3_fe,
                                            bcm_l3_route_t          *info,
                                            _fe2k_g2p3_sa_route_t   *sa_info);

extern int _bcm_fe2000_g2p3_ipv4_route_delete_all(_fe2k_l3_fe_instance_t *l3_fe,
                                                  uint32 fte_idx,
                                                  uint32 flags);

extern int _bcm_fe2000_g2p3_ip_ecmp_route_get( _fe2k_l3_fe_instance_t *l3_fe,
                                               bcm_l3_route_t         *info,
                                               uint32                 *fte_base);

extern void* _bcm_fe2000_g2p3_alloc_fte(uint32 count);

extern int _bcm_fe2000_g2p3_fte_op(_fe2k_l3_fe_instance_t    *l3_fe,
                                    uint32                    ft_idx,
                                    void                      *fte,
                                    _fe2k_fte_opcode           op);

extern int _bcm_fe2000_g2p3_l3_flush_cache(_fe2k_l3_fe_instance_t *l3_fe,
                                            int flag);

/* Multicast */
 int
_bcm_fe2000_g2p3_ipmc_add(int                     unit,
                          _fe2k_l3_fe_instance_t *l3_fe,
                          bcm_ipmc_addr_t        *data);
 int
_bcm_fe2000_g2p3_ipmc_replace(int                     unit,
                              _fe2k_l3_fe_instance_t *l3_fe,
                              bcm_ipmc_addr_t        *data);
int
_bcm_fe2000_g2p3_ipmc_find(int                      unit,
                      _fe2k_l3_fe_instance_t  *l3_fe,
                      bcm_ipmc_addr_t         *data);

int
_bcm_fe2000_g2p3_ipmc_flow_delete(int                     unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress);

extern int _bcm_fe2000_g2p3_ipmc_flush_cache(int unit);
/* mim */
extern int _bcm_fe2000_mim_qosmap_set(int unit, bcm_gport_t port, 
                                      int ing_idx, int egr_idx,
                                      uint32 ingFlags, uint32 egrFlags);

extern int _bcm_fe2000_mim_qosmap_get(int unit, bcm_gport_t port, 
                                      int *ing_idx, int *egr_idx,
                                      uint32 *ing_flags, uint32 *egr_flags);

extern int _bcm_fe2000_mim_stp_update(int unit, 
                                      bcm_gport_t port, 
                                      int *stp_state,
                                      int set);

extern int _bcm_fe2000_multicast_mim_encap_get(int unit,
                                               bcm_multicast_t group,
                                               bcm_gport_t port,
                                               bcm_gport_t mim_port_id,
                                               bcm_if_t *encap_id);

extern int _bcm_fe2000_mim_fte_gport_get(int unit,
                                         bcm_gport_t mim_port_id,
                                         uint32 *ftidx);

extern bcm_gport_t _bcm_fe2000_mim_fte_to_gport_id(int unit,
                                                   uint32 ftidx);

extern int _bcm_fe2000_mim_port_vlan_vector_internal(int unit,
                                                     bcm_gport_t gport,
                                                     bcm_port_t *phy_port,
                                                     bcm_vlan_t *vsi,
                                                     uint32   *logicalPort);

extern int _bcm_fe2000_mim_port_vlan_vector_set(int unit,
                                                bcm_gport_t gport,
                                                bcm_vlan_vector_t vlan_vec);

extern int _bcm_fe2000_mim_port_vlan_vector_get(int unit,
                                                bcm_gport_t gport,
                                                bcm_vlan_vector_t vlan_vec);
extern int _bcm_fe2000_mim_vpn_policer_set(int unit, 
                                           bcm_mim_vpn_t vpn,
                                           bcm_policer_t pol_id,
                                           uint8 verify);

extern int _bcm_fe2000_mim_vpn_policer_get(int unit, 
                                           bcm_mim_vpn_t vpn,
                                           bcm_policer_t *pol_id);

extern int _bcm_fe2000_mim_policer_set(int unit, 
                                       bcm_gport_t port,
                                       bcm_policer_t pol_id);

extern int _bcm_fe2000_mim_policer_get(int unit, 
                                       bcm_gport_t port,
                                       bcm_policer_t *pol_id);

extern int bcm_fe2k_mim_frame_max_access(int unit, bcm_gport_t gport,
                                         int *size, int set);

extern int _bcm_fe2000_g2p3_num_policer_counters_get(int unit,
                                                     bcm_policer_t policer_id,
                                                     int *ncounters);

extern int 
_bcm_fe2000_g2p3_logical_interface_ingress_stats_enable_update (int unit, 
                                                                uint32 lpi,
                                                                int get,
                                                                int *statsEnabled);

extern int
_bcm_fe2000_g2p3_logical_interface_egress_stats_enable_update (int unit, 
                                                               uint32 ohi,
                                                               int get,
                                                               int *statsEnabled);

extern int 
_bcm_fe2000_mpls_policer_set(int unit, 
                             bcm_gport_t port,
                             bcm_policer_t pol_id);

extern int 
_bcm_fe2000_mpls_policer_get(int unit, 
                             bcm_gport_t port,
                             bcm_policer_t *pol_id);

extern int
_bcm_fe2000_g2p3_mpls_vpws_port_failover_update(int unit,
                                       uint32 fte_idx_to_update,
                                       uint32 fte_idx_fo,
                                       bcm_failover_t failover_id);
extern int
_bcm_fe2000_g2p3_mpls_failover_update(_fe2k_l3_fe_instance_t *l3_fe,
                                      _fe2k_vpn_control_t *vpnc,
                                      _fe2k_vpn_sap_t *vpn_sap,
                                      bcm_mpls_port_t *mpls_port,
                                      uint8          is_pw_failover);
extern int
_bcm_fe2000_g2p3_mpls_vpws_pw_fo_connect(_fe2k_l3_fe_instance_t *l3_fe,
                                         _fe2k_vpn_control_t    *vpnc,
                                         _fe2k_vpn_sap_t        *vpn_sap,
                                         bcm_mpls_port_t        *mpls_pw_port,
                                         uint8                 connect);


#endif /* _BCM_INT_SBX_FE2000_G2P3_H_ */
