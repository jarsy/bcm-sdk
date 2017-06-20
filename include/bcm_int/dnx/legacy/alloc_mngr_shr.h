/*
 * $Id: alloc_mngr_shr.h,v 1.45 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        alloc_mngr_shr.h
 * Purpose:     Resource allocation shared between alloc and wb.
 *
 */

#ifndef  INCLUDE_DNX_ALLOC_MNGR_SHR_H
#define  INCLUDE_DNX_ALLOC_MNGR_SHR_H

#include <soc/dnx/legacy/drv.h>
#include <shared/swstate/sw_state_resmgr.h>
#include <shared/dnx_shr_template.h>
#include <soc/dnx/legacy/SAND/Utils/sand_occupation_bitmap.h>


/* Res managemr Defines and Structures. */
typedef enum _dnx_am_res_e {
    /****************************************/
    /************   TM POOLS   **************/
    /****************************************/
    dnx_am_res_mc_dynamic = 0,
    dnx_am_res_trap_single_instance,
    dnx_am_res_trap_user_define,
    dnx_am_res_trap_virtual,
    dnx_am_res_trap_reserved_mc,
    dnx_am_res_trap_prog,
    dnx_am_res_trap_egress,
    dnx_am_res_snoop_cmd,
    dnx_am_res_qos_egr_pcp_vlan,

    /****************************************/
    /************   PP POOLS   **************/
    /****************************************/
    dnx_am_res_vsi_vlan,
    dnx_am_res_vsi_mstp,
    dnx_am_res_lif_pwe,
    dnx_am_res_lif_ip_tnl,
    dnx_am_res_lif_dynamic,
    dnx_am_res_fec_global,
    dnx_am_res_failover_common_id,
    dnx_am_res_failover_ingress_id,
    dnx_am_res_failover_fec_id,
    dnx_am_res_failover_egress_id,
    dnx_am_res_eep_global,
    dnx_am_res_eep_ip_tnl, /* used for ip tunnels*/
    dnx_am_res_eep_mpls_tunnel, /* used for mpls/pwe tunnels*/
    dnx_am_res_glbl_src_ip,
    dnx_am_res_glbl_ttl,
    dnx_am_res_glbl_tos,
    dnx_am_res_eg_out_ac,
    dnx_am_res_eg_out_rif,
    dnx_am_res_eg_data_erspan,
    dnx_am_res_ipv6_tunnel,
    dnx_am_res_eg_data_trill_invalid_entry,
    dnx_am_res_qos_ing_elsp,
    dnx_am_res_qos_ing_lif_cos,
    dnx_am_res_qos_ing_pcp_vlan,
    dnx_am_res_qos_ing_cos_opcode,
    dnx_am_res_qos_egr_remark_id,
    dnx_am_res_qos_egr_mpls_php_id,
    dnx_am_res_meter_a, /* must be directly before meter_b */
    dnx_am_res_meter_b, /* must be directly after meter_a */
    dnx_am_res_ether_policer,
#ifdef BCM_JER2_ARAD_SUPPORT
    dnx_am_res_ecmp_id,
#endif 
    dnx_am_res_qos_egr_l2_i_tag,
#ifdef BCM_88660
    dnx_am_res_qos_egr_dscp_exp_marking,
#endif /* BCM_88660 */
    dnx_am_res_rp_id, /* RPIDs for IPMC BIDIR*/
    dnx_am_res_oam_ma_index,
    dnx_am_res_oam_mep_id_short,
    dnx_am_res_oam_mep_id_long,
    dnx_am_res_oam_rmep_id,
    dnx_am_res_oam_trap_code_upmep_ftmh_header,
    dnx_am_res_pon_channel_profile,
    dnx_am_res_vlan_edit_action_ingress,
    dnx_am_res_vlan_edit_action_egress, 
    dnx_am_res_trill_virtual_nick_name,
    dnx_am_res_local_inlif_common,
    dnx_am_res_local_inlif_wide,
    dnx_am_res_local_outlif,
    dnx_am_res_eep_trill,
    dnx_am_res_global_inlif,
    dnx_am_res_global_outlif,
    dnx_am_res_obs_inlif,
    dnx_am_res_obs_eg_encap,
    dnx_am_res_vsq_src_port,
    dnx_am_res_vsq_pg,
    dnx_am_res_map_encap_intpri_color,
    dnx_am_res_field_hw_group_id,
    dnx_am_res_field_entry_id,
    dnx_am_res_field_direct_extraction_entry_id,
    dnx_am_res_oam_mep_id,
    dnx_am_res_oam_mep_db_id,
    dnx_am_res_trap_etpp,
    dnx_am_res_global_sync_lif,
    dnx_am_res_oam_sd_sf_id,
    dnx_am_res_oam_y_1711_sd_sf_id,
    dnx_am_res_oam_sd_sf_profile_id,
    dnx_am_res_qos_ing_color,
    dnx_am_res_ipsec_sa_id,
    dnx_am_res_ipsec_tunnel_id,
    dnx_am_res_count /* MUST BE LAST -- NOT A VALID ID */
} _dnx_am_res_t;

/* 
 * Pool structure for all pools other than cosq. 
 * for cosq see bcm_dnx_am_cosq_pool_info_t
 * A pool_id represents mapping between resource_id and core_id.
 */
typedef struct {
    int                 pool_id;        /* The pool on which the resource is allocated. */
    int                 res_id;         /* The resource using the pool. */
    int                 core_id;        /* The core using the pool.*/    
    sw_state_res_allocator_t res_type;       /* resource storage type */
    int                 start;          /* resource start */
    int                 count;          /* count of resources */
    int                 max_elements_per_allocation; /* required for wb storage */
} bcm_dnx_am_pool_info_t;


/* Template management Defines and Structures */
typedef enum _dnx_am_template_e {
    /****************************************/
    /************ TM TEMPLATES **************/
    /****************************************/
    dnx_am_template_egress_thresh = 0,
    dnx_am_template_mirror_action_profile,
    dnx_am_template_egress_interface_unicast_thresh,
    dnx_am_template_egress_interface_multicast_thresh,
    dnx_am_template_user_defined_traps,
    dnx_am_template_snoop_cmd,
    dnx_am_template_trap_reserved_mc,
    dnx_am_template_prog_trap,
    dnx_am_template_queue_rate_cls,
    dnx_am_template_system_red_dp_pr,
    dnx_am_template_vsq_pg_tc_mapping,
    dnx_am_template_vsq_rate_cls_ct,
    dnx_am_template_vsq_rate_cls_cttc,
    dnx_am_template_vsq_rate_cls_ctcc,
    dnx_am_template_vsq_rate_cls_pp,
    dnx_am_template_vsq_rate_cls_src_port,
    dnx_am_template_vsq_rate_cls_pg,
    dnx_am_template_queue_discount_cls,
    dnx_am_template_egress_port_discount_cls_type_raw,
    dnx_am_template_egress_port_discount_cls_type_cpu,
    dnx_am_template_egress_port_discount_cls_type_eth,
    dnx_am_template_egress_port_discount_cls_type_tm,
    dnx_am_template_cosq_port_hr_flow_control, /* e2e port hr flow control profile */
    dnx_am_template_cosq_sched_class,          /* scheduler (cl) class */
    dnx_am_template_fabric_tdm_link_ptr,       /* TDM Direct routing link pointer */
    dnx_am_template_ingress_flow_tc_mapping,    /* Ingress Flow TC Mapping Profiles */
    dnx_am_template_ingress_uc_tc_mapping,      /* Ingress UC TC Mapping Profiles */
    dnx_am_template_fc_generic_pfc_mapping,     /* Flow Control Generic PFC Profiles */
    dnx_am_template_fc_generic_pfc_mapping_c0,     /* Flow Control Generic PFC Profiles for priority 0 */
    dnx_am_template_fc_generic_pfc_mapping_c1,     /* Flow Control Generic PFC Profiles for priority 1 */
    dnx_am_template_fc_generic_pfc_mapping_c2,     /* Flow Control Generic PFC Profiles for priority 2 */
    dnx_am_template_fc_generic_pfc_mapping_c3,     /* Flow Control Generic PFC Profiles for priority 3 */
    dnx_am_template_fc_generic_pfc_mapping_c4,     /* Flow Control Generic PFC Profiles for priority 4 */
    dnx_am_template_fc_generic_pfc_mapping_c5,     /* Flow Control Generic PFC Profiles for priority 5 */
    dnx_am_template_fc_generic_pfc_mapping_c6,     /* Flow Control Generic PFC Profiles for priority 6 */
    dnx_am_template_fc_generic_pfc_mapping_c7,     /* Flow Control Generic PFC Profiles for priority 7 */
    dnx_am_template_cnm_queue_profile,
    dnx_am_template_tpid_profile,
    dnx_am_template_egress_queue_mapping,
    dnx_am_template_trap_egress,
    dnx_am_template_nrdy_threshold,

    /****************************************/
    /************   PP POOLS   **************/
    /****************************************/
    dnx_am_template_vlan_edit_profile_mapping,    /* VLAN Edit Profile */
    dnx_am_template_vlan_edit_profile_mapping_eg, /* Egress VLAN Edit Profile */      
    dnx_am_template_vsi_egress_profile,
    dnx_am_template_vsi_ingress_profile,
    dnx_am_template_reserved_mc,
    dnx_am_template_tpid_class,
    dnx_am_template_mpls_push_profile,
    dnx_am_template_lif_term_profile,
    dnx_am_template_port_mact_sa_drop_profile,
    dnx_am_template_port_mact_da_unknown_profile,
    /* don't separate between below 4 templates */
    dnx_am_template_meter_profile_a_low, 
    dnx_am_template_meter_profile_b_low, 
    /* don't separate between above 4 templates */
    dnx_am_template_l2_event_handle,            /* from even-profile, to <event x self/learn/shadow> */
    dnx_am_template_l2_vsi_learn_profile,       /* from learn-profile to <limit, event-profile, aging-profile> */
    dnx_am_template_l2_flooding_profile,        /* flooding by <port_profile,lif_profile> */
    dnx_am_template_vlan_port_protocol_profile, /* port protocol mapping. <port,profile> ->10ethertype,tc,vlan */
    dnx_am_template_ip_tunnel_src_ip,
    dnx_am_template_ip_tunnel_ttl,
    dnx_am_template_ip_tunnel_tos,
    dnx_am_template_ttl_scope_index, 
    dnx_am_template_oam_icc_map,
    dnx_am_template_oam_sa_mac,
    dnx_am_template_oam_punt_event_hendling_profile,
    dnx_am_template_oam_mep_profile_non_accelerated,
    dnx_am_template_oam_mep_profile_accelerated,
    dnx_am_template_bfd_mep_profile_non_accelerated,
    dnx_am_template_bfd_mep_profile_accelerated,
    dnx_am_template_oam_tx_priority,
    dnx_am_template_bfd_ip_dip,
    dnx_am_template_mpls_pwe_push_profile,
    dnx_am_template_bfd_req_interval_pointer,
    dnx_am_template_bfd_tos_ttl_profile,
    dnx_am_template_bfd_src_ip_profile,
    dnx_am_template_bfd_tx_rate_profile,
    dnx_am_template_bfd_flags_profile,
    dnx_am_template_oam_lmm_nic_tables_profile,
    dnx_am_template_oam_lmm_oui_tables_profile,
    dnx_am_template_oam_eth1731_profile,
    dnx_am_template_oam_local_port_2_system_port,
    dnx_am_template_oam_oamp_pe_gen_mem,
    dnx_am_template_port_tpid_class_egress_acceptable_frame_type,
    dnx_am_template_ptp_port_profile,
    dnx_am_template_l3_vrrp,
    dnx_am_template_l3_rif_mac_termination_combination,
    dnx_am_template_eedb_roo_ll_format_eth_type_index,
    dnx_am_template_tpid_class_eg,
    dnx_am_template_out_rif_profile,
    dnx_am_template_oamp_pe_gen_mem_maid_48, 
    dnx_am_template_ip_tunnel_encapsulation_mode,
    dnx_am_template_crps_inpp_port_compensation_profile,
    dnx_am_template_crps_intm_port_compensation_profile,
    dnx_am_template_stat_interface_ing_port_compensation_profile,
    dnx_am_template_stat_interface_egr_port_compensation_profile,
    dnx_am_template_scheduler_adjust_size_final_delta, /* relevant for Jericho only */
    dnx_am_template_lif_mtu_profile, /*Relevant only for JB0 and above*/
    dnx_am_template_oamp_flex_ver_mask,
    dnx_am_template_oamp_cls_flex_crc,
    dnx_am_template_l2cp_egress_profile,
    dnx_am_template_count /* MUST BE LAST -- NOT A VALID ID */
} _dnx_am_template_t;

/* 
 * Template structure for templates other than those already supported by cosq
 * for warmboot.
 * A pool_id represents mapping between template id and core id.
 */
typedef struct {
    int                    pool_id;         /* Pool id of the pool. */
    int                    template_id;     /* Template id using the pool. */
    int                    core_id;         /* Core id using the pool. */
    dnx_shr_template_manage_t  manager;
    int                    start;
    int                    count;
    int                    max_entities; /* max referring entities */
    uint32                 global_max;  /* Max referring entinties on all templates. */
    size_t                 data_size;
    dnx_shr_template_manage_hash_compare_extras_t hashExtra;
} bcm_dnx_am_template_info_t;

/* An array representing the various hash compare callbacks. */
typedef enum _bcm_dnx_am_template_hash_compare_cb_idx_e {
    _bcm_dnx_am_template_hash_compare_cb_idx_memcpy,
    _bcm_dnx_am_template_hash_compare_cb_idx_user_defined_trap,
    _bcm_dnx_am_template_hash_compare_cb_idx_snoop_cmd,
    _bcm_dnx_am_template_hash_compare_cb_idx_discount_cls,
    _bcm_dnx_am_template_hash_compare_cb_idx_count
} _bcm_dnx_am_template_hash_compare_cb_idx_t;

extern dnx_shr_template_to_stream_t _bcm_dnx_am_template_to_stream_arr[_bcm_dnx_am_template_hash_compare_cb_idx_count];
extern dnx_shr_template_from_stream_t _bcm_dnx_am_template_from_stream_arr[_bcm_dnx_am_template_hash_compare_cb_idx_count];


/*Below is the global array of call back functions used to print the template data */
extern dnx_shr_template_print_func_t _bcm_dnx_am_template_print_func[dnx_am_template_count];

int
_bcm_dnx_pp_resource_setup(int unit, int res_id, int core_id, int pool_id);

int
_bcm_dnx_template_setup(int unit, int template_id, int core_id, int pool_id);

int
bcm_dnx_am_mc_alloc(int unit,
                    uint32 flags, /* flags should be SHR_RES_ALLOC_WITH_ID; */
                    DNX_TMC_MULT_ID  *mc_id,
                    uint8 is_egress);


int
bcm_dnx_am_qos_egr_dscp_exp_marking_alloc(int unit,
                                  uint32 flags,
                                  int *qos_id);

#endif /* INCLUDE_DNX_ALLOC_MNGR_SHR_H */
