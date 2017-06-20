/*
 * $Id: mbcm.c,v 1.74 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mbcm.c
 */

#include <soc/dpp/mbcm_pp.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mymac.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_port.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_fcf.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_fec.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_occupation_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mpls_term.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oamp_pe.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_l3_src_bind.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ip_tcam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_extender.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_dbal.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_metering.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_parse.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ptp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv4.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_extend_p2p.h>

CONST mbcm_pp_functions_t mbcm_pp_arad_driver = {
	/**********************************************/
	/* insert functions here according to category*/
	/**********************************************/

	/*OAM*/
	arad_pp_oam_oamp_protection_packet_header_raw_get,
	arad_pp_oam_oamp_protection_packet_header_raw_set,
	arad_pp_oam_oamp_1dm_get,
	arad_pp_oam_oamp_1dm_set,
	arad_pp_oam_bfd_acc_endpoint_tx_disable,
	arad_pp_oam_classifier_default_profile_add,
	arad_pp_oam_classifier_default_profile_remove,
	NULL,                         /*pp_oam_classifier_oem1_entry_set_unsafe*/
	NULL,                         /*pp_oam_classifier_oem1_init*/
	arad_pp_oam_counter_increment_bitmap_set,
	NULL,                    /* mbcm_pp_oam_dm_trigger_set */
	NULL  /* mbcm_pp_oam_egress_pcp_set_by_profile_and_tc*/,
	arad_pp_oam_inlif_profile_map_get,
	arad_pp_oam_inlif_profile_map_set,
	arad_pp_oam_my_cfm_mac_delete,
	arad_pp_oam_my_cfm_mac_get,
	arad_pp_oam_my_cfm_mac_set,
	arad_pp_oam_bfd_discriminator_rx_range_get,
	arad_pp_oam_bfd_discriminator_rx_range_set,
	arad_pp_oam_oamp_dm_get,
	arad_pp_oam_oamp_pe_gen_mem_set,
	NULL, /*soc_jer_pp_oam_loopback_tst_info_init*/	
	NULL, /*soc_jer_pp_oam_oamp_lb_tst_header_set*/
	NULL, /* mbcm_pp_oam_oamp_lm_dm_delete       */
	NULL, /* mbcm_pp_oam_oamp_lm_dm_pointed_find */
	NULL, /* mbcm_pp_oam_oamp_lm_dm_set          */
	arad_pp_oam_oamp_lm_get,
	arad_pp_oam_oamp_loopback_get,
	arad_pp_oam_oamp_loopback_set,
	arad_pp_oam_oamp_punt_event_hendling_profile_get,
	arad_pp_oam_oamp_punt_event_hendling_profile_set,
	arad_pp_oam_oamp_rx_trap_codes_delete,/*mbcm_pp_oam_oamp_rx_trap_codes_delete*/
	arad_pp_oam_oamp_rx_trap_codes_set ,/*mbcm_pp_oam_oamp_rx_trap_codes_set*/
	arad_pp_oam_oamp_search_for_lm_dm,
	NULL,                   /* mbcm_pp_oam_sa_addr_msbs_get */
	NULL,                   /* mbcm_pp_oam_sa_addr_msbs_set */
	 arad_pp_oam_slm_set,
	arad_pp_oam_tod_set,
	arad_pp_oamp_pe_use_1dm_check,
	NULL,                     /*arad_pp_oamp_report_mode_get*/
	NULL,                    /*arad_pp_oamp_report_mode_set*/
	arad_pp_flp_dbal_bfd_echo_program_tables_init,
	arad_pp_lem_access_bfd_one_hop_lem_entry_add,
	arad_pp_lem_access_bfd_one_hop_lem_entry_remove,
	NULL, /*  soc_qax_pp_oam_oamp_rmep_db_ext_get */
	NULL, /*  soc_qax_pp_oam_oamp_sd_sf_profile_set  */
	NULL, /*  soc_qax_pp_oam_oamp_sd_sf_profile_get */
	NULL, /*  soc_qax_pp_oam_oamp_sd_sf_1711_config_set */
	NULL, /*  soc_qax_pp_oam_oamp_sd_sf_1711_config_get */
	NULL, /*  soc_qax_pp_oam_oamp_sd_sf_scanner_set */
	NULL, /*  soc_qax_pp_oam_oamp_sd_sf_1711_db_set */
	NULL, /*  soc_qax_pp_oam_oamp_sd_sf_1711_db_get */
	NULL, /*   soc_qax_pp_oam_oamp_sd_sf_db_set */
	NULL, /*   soc_qax_pp_oam_oamp_sd_sf_db_get */	
    arad_pp_oam_classifier1_mip_passive_entries,
    NULL, /*mbcm_pp_oam_bfd_flexible_verification_set_f*/
    NULL, /*mbcm_pp_oam_bfd_flexible_verification_get_f*/
    NULL, /*mbcm_pp_oam_bfd_flexible_verification_delete_f*/
    NULL, /*mbcm_pp_oam_bfd_mep_db_ext_data_set_f   */
    NULL, /*mbcm_pp_oam_bfd_mep_db_ext_data_get_f   */
    NULL, /*mbcm_pp_oam_bfd_mep_db_ext_data_delete_f*/
    NULL, /*mbcm_pp_oamp_control_lm_pcp_counter_range_min_get_f*/
    NULL, /*mbcm_pp_oamp_control_lm_pcp_counter_range_max_get_f*/
    soc_arad_pp_oamp_control_punt_packet_int_pri_get,
    soc_arad_pp_oamp_control_ccm_weight_get,
    soc_arad_pp_oamp_control_sat_weight_get,
    soc_arad_pp_oamp_control_response_weight_get,
    soc_arad_oamp_cpu_port_dp_tc_set,
    soc_arad_oamp_tmx_arb_weight_set,
    soc_arad_oamp_sat_arb_weight_set,
    soc_arad_oamp_response_weight_set,
    arad_pp_oam_eth_oam_opcode_map_set_unsafe,
    soc_arad_pp_oam_classifier_oem_mep_mip_conflict_check,
    soc_arad_pp_set_mep_data_in_gen_mem,
    arad_pp_oamp_pe_program_profile_get,
    soc_arad_pp_oam_dma_clear,
    NULL, /* mbcm_pp_soc_qax_diag_oamp_counter_set */
    NULL, /* mbcm_pp_soc_qax_diag_oamp_counter_get */
    NULL, /*  soc_arad_pp_get_mep_data_in_gen_mem */
   /** OAM ----> END */

	/*EGRESS_ENCAP*/
	NULL, /* mbcm_dpp_pp_eg_encap_null_value_get */
	NULL, /* mbcm_dpp_pp_eg_encap_null_value_set */
	arad_pp_eg_encap_access_clear_bank,
	NULL,                    /* mbcm_pp_eg_encap_direct_bank_set */
	NULL,                    /*soc_jer_eg_encap_ether_type_index_clear*/
	NULL,                    /*soc_jer_eg_encap_ether_type_index_get*/
	NULL,                    /*soc_jer_eg_encap_ether_type_index_set*/
	NULL,                    /* mbcm_pp_eg_encap_extension_mapping_get */
	NULL,                    /* mbcm_pp_eg_encap_extension_mapping_set */
	NULL,                    /* mbcm_pp_eg_encap_extension_type_get */
	NULL,                    /* mbcm_pp_eg_encap_extension_type_set */
	NULL,    /* mbcm_pp_eg_encap_ip_tunnel_size_protocol_template_set */
	NULL,                     /*soc_jer_eg_encap_map_encap_intpri_color_get*/
	NULL,                     /*soc_jer_eg_encap_map_encap_intpri_color_set*/
	NULL, /*mbcm_pp_eg_encap_push_2_swap_init*/
    NULL, /*soc_qax_eg_encap_header_compensation_per_cud_set*/
    NULL, /*soc_qax_eg_encap_header_compensation_per_cud_get*/
	/*FRWRD*/
	arad_pp_frwrd_fec_entry_uc_rpf_mode_set,
	arad_pp_frwrd_fcf_npv_switch_get,
	arad_pp_frwrd_fcf_npv_switch_set,
	arad_pp_frwrd_fcf_vsan_mode_get,
	arad_pp_frwrd_fcf_vsan_mode_set,
	arad_pp_frwrd_fec_is_protected_get,
    arad_pp_frwrd_extend_p2p_lem_entry_add,
    arad_pp_frwrd_extend_p2p_lem_entry_remove,
    arad_pp_frwrd_extend_p2p_lem_entry_get,
	/*MACT management*/
	arad_pp_aging_num_of_cycles_get,
	arad_pp_frwrd_mact_clear_access_bit,
	arad_pp_frwrd_mact_cpu_counter_learn_limit_get,
	arad_pp_frwrd_mact_cpu_counter_learn_limit_set,
	NULL,   /* arad_pp_frwrd_mact_event_handle_info_set_dma */
	NULL,                   /* arad_pp_frwrd_mact_event_handler_callback_register */
	NULL,                   /* arad_pp_frwrd_mact_learning_dma_set */
	NULL,                   /* arad_pp_frwrd_mact_learning_dma_unset */
	arad_pp_frwrd_mact_opport_mode_get,
	arad_pp_frwrd_mact_transplant_static_get,
	arad_pp_frwrd_mact_transplant_static_set,
	/*OCC_MANAGEMENT*/
	arad_pp_occ_mgmt_app_get,
	arad_pp_occ_mgmt_app_set,
	arad_pp_occ_mgmt_deinit,
	arad_pp_occ_mgmt_get_app_mask,
	arad_pp_occ_mgmt_init,
	/*PORT EXTENDER*/
	arad_pp_l2_lif_extender_get,
	arad_pp_l2_lif_extender_remove,
	arad_pp_extender_deinit,
	arad_pp_extender_eve_etag_format_get,
	arad_pp_extender_eve_etag_format_set,
	arad_pp_extender_global_etag_ethertype_get,
	arad_pp_extender_global_etag_ethertype_set,
	arad_pp_extender_init,
	arad_pp_extender_port_info_get,
	arad_pp_extender_port_info_set,
	/*IP_MC*/
	arad_pp_frwrd_ip_ipmc_ssm_add,/*mbcm_pp_ipmc_ssm_entry_add*/
	arad_pp_frwrd_ip_ipmc_ssm_delete,/*mbcm_pp_ipmc_ssm_entry_delete*/
	arad_pp_frwrd_ip_ipmc_ssm_get,/*mbcm_pp_ipmc_ssm_entry_get*/
	NULL,                /* mbcm_pp_ipmc_ssm_tcam_entry_add */
	NULL,             /* mbcm_pp_ipmc_ssm_tcam_entry_delete */
	NULL,                /* mbcm_pp_ipmc_ssm_tcam_entry_get */
	/*MPLS*/
	NULL,                   /* mbcm_pp_mpls_termination_range_action_get */
	NULL,                   /* mbcm_pp_mpls_termination_range_action_set */
	NULL,                   /* mbcm_pp_mpls_termination_range_profile_get */
	NULL,                   /* mbcm_pp_mpls_termination_range_profile_set */
	arad_pp_mpls_termination_spacial_labels_init,
	NULL, /*mbcm_pp_pp_mpls_termination_vccv_type_ttl1_oam_classification_set*/
	NULL, /*mbcm_pp_pp_mpls_termination_vccv_type_ttl1_oam_classification_get*/
	/*MYMAC*/
	NULL,                   /* mbcm_pp_mymac_protocol_group_get_group_by_protocols */
	NULL,                   /* mbcm_pp_mymac_protocol_group_get_protocol_by_group */
	NULL,                   /* mbcm_pp_mymac_protocol_group_set_f */
	arad_pp_mymac_vrid_mymac_map_set_to_all_vsi,
	arad_pp_mymac_vrrp_cam_info_delete,
	arad_pp_mymac_vrrp_cam_info_get,
	arad_pp_mymac_vrrp_cam_info_set,
	/*POLICER*/
	arad_pp_mtr_policer_global_sharing_get,
	arad_pp_mtr_policer_global_sharing_set,
	NULL, /*mbcm_pp_mtr_policer_hdr_compensation_get*/
	NULL, /*mbcm_pp_mtr_policer_hdr_compensation_set*/
	arad_pp_mtr_policer_ipg_compensation_get,
	arad_pp_mtr_policer_ipg_compensation_set,
	NULL, /*arad_pp_mtr_eth_policer_glbl_profile_get*/
	NULL, /*arad_pp_mtr_eth_policer_glbl_profile_map_get*/
	NULL, /*arad_pp_mtr_eth_policer_glbl_profile_map_set*/
	NULL, /*arad_pp_mtr_eth_policer_glbl_profile_set*/
	NULL, /*arad_pp_mtr_eth_policer_params_get*/
	NULL, /*arad_pp_mtr_eth_policer_params_set*/
	/*LIF*/
	NULL,                   /*soc_jer_pp_network_group_config_get*/
	NULL,                   /*soc_jer_pp_network_group_config_set*/
	NULL,                   /*mbcm_pp_lif_additional_data_get*/
	NULL,                  /*mbcm_pp_lif_additional_data_set*/
	NULL,                   /* mbcm_pp_lif_glem_access_entry_add_f */
	NULL,                   /* mbcm_pp_lif_glem_access_entry_by_key_get_f */
	NULL,                   /* mbcm_pp_lif_glem_access_entry_remove_f */
	NULL,                   /*mbcm_pp_lif_is_wide_entry*/
	/*IP6_COMPRESSION*/
	arad_pp_ip6_compression_add,
	arad_pp_ip6_compression_delete,
	arad_pp_ip6_compression_get,
	/*METERING*/
	arad_pp_mtr_meter_ins_bucket_get,
	arad_pp_metering_dp_map_entry_get,
	arad_pp_metering_dp_map_entry_set,
	arad_pp_metering_pcd_entry_get,
	arad_pp_metering_pcd_entry_set,
	/*PORT*/
	arad_pp_port_additional_tpids_get,
	arad_pp_port_additional_tpids_set,
	arad_pp_port_property_get,
	arad_pp_port_property_set,
	/*DIAG*/
	NULL,/*soc_jer_diag_glem_signals_get*/
	NULL,                    /*soc_jer_pp_diag_kaps_lkup_info_get*/
	/*TRAP*/
	NULL,                 /*    soc_jer_eg_etpp_trap_get    */
	NULL,                /*    soc_jer_eg_etpp_trap_set    */
    NULL,               /*soc_jer_pp_eg_pmf_mirror_params_set*/
    NULL,               /*soc_jer_eg_etpp_out_lif_mtu_map_set*/
    NULL,               /*soc_jer_eg_etpp_out_lif_mtu_map_get*/
    NULL,               /*soc_jer_eg_etpp_out_lif_mtu_check_set*/
    NULL,               /*soc_qax_pp_trap_ingress_ser_set*/
	/*RIF*/
	NULL,                     /*mbcm_pp_rif_global_urpf_mode_set*/
	NULL, /* soc_qax_pp_eg_rif_profile_delete */
	NULL, /* soc_qax_pp_eg_rif_profile_get */
	NULL, /* soc_qax_pp_eg_rif_profile_set */
	/*VLAN*/
	NULL, /*mbcm_pp_eg_vlan_edit_packet_is_tagged_get*/
	NULL, /*mbcm_pp_eg_vlan_edit_packet_is_tagged_set*/
	/*VSWITCH*/
	NULL, /*mbcm_pp_eg_filter_default_port_membership_get*/
	NULL, /*mbcm_pp_eg_filter_default_port_membership_set*/
	arad_pp_llp_parse_packet_format_eg_info_get,
	arad_pp_llp_parse_packet_format_eg_info_set,
	/*P2P*/
	arad_pp_ptp_p2p_delay_get,
	arad_pp_ptp_p2p_delay_set,
	/*Protection*/
	NULL,                    /* mbcm_pp_egress_protection_state_get_f */
	NULL,                   /* mbcm_pp_egress_protection_state_set_f */
	NULL,                   /* mbcm_pp_ingress_protection_state_get_f */
	NULL,                   /* mbcm_pp_ingress_protection_state_set_f */
	/*Field processor*/
	arad_pp_fp_presel_max_id_get,
	/*TRILL*/
	arad_pp_eg_trill_entry_set,
	/*Load Balancing*/
	NULL,                    /* soc_jer_pp_lag_print_ecmp_lb_data */
	NULL, /*mbcm_pp_ecmp_hash_slb_combine_set*/
	NULL, /*mbcm_pp_ecmp_hash_slb_combine_get*/
	NULL, /*mbcm_pp_eg_encap_additional_label_profile_set*/
	NULL, /*mbcm_pp_eg_encap_additional_label_profile_get*/
	/*QOS*/
	arad_pp_lif_ing_vlan_edit_pcp_map_dp_set,
	arad_pp_lif_ing_vlan_edit_pcp_map_dp_get,
	arad_pp_lif_ing_vlan_edit_inner_global_info_set,
	arad_pp_lif_ing_vlan_edit_inner_global_info_get,
	arad_pp_lif_ing_vlan_edit_outer_global_info_set,
	arad_pp_lif_ing_vlan_edit_outer_global_info_get,
	NULL, /*mbcm_pp_port_eg_ttl_inheritance_set*/
	NULL, /*mbcm_pp_port_eg_ttl_inheritance_get*/
	NULL, /*mbcm_pp_port_eg_qos_inheritance_set*/
	NULL, /*mbcm_pp_port_eg_qos_inheritance_get*/
    NULL,  /*mbcm_pp_port_eg_qos_marking_set;*/
    NULL, /*mbcm_pp_port_eg_qos_marking_get;*/
    NULL, /* mbcm_pp_lif_default_native_ac_outlif_set*/
    NULL, /* mbcm_pp_lif_default_native_ac_outlif_get*/
    arad_pp_eg_encap_access_default_ac_entry_vlan_edit_profile_set, 
    arad_pp_eg_encap_access_default_ac_entry_vlan_edit_profile_get, 
};
