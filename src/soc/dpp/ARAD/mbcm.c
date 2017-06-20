/*
 * $Id: mbcm.c,v 1.74 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mbcm.c
 */

#include <soc/dpp/mbcm.h>
#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/ARAD/arad_drv.h>
#include <soc/dpp/ARAD/arad_api_action_cmd.h>
#include <soc/dpp/ARAD/arad_ingress_traffic_mgmt.h>
#include <soc/dpp/ARAD/arad_api_ingress_packet_queuing.h>
#include <soc/dpp/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dpp/ARAD/arad_api_mgmt.h>
#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/arad_ports.h>
#include <soc/dpp/ARAD/arad_api_multicast_fabric.h>
#include <soc/dpp/ARAD/arad_multicast_fabric.h>
#include <soc/dpp/ARAD/arad_tdm.h>
#include <soc/dpp/multicast_imp.h>
#include <soc/dpp/ARAD/arad_multicast_imp.h>
#include <soc/dpp/ARAD/arad_api_fabric.h>
#include <soc/dpp/ARAD/arad_api_ofp_rates.h>
#include <soc/dpp/ARAD/arad_ofp_rates.h>
#include <soc/dpp/ARAD/arad_flow_control.h>
#include <soc/dpp/ARAD/arad_api_cnt.h>
#include <soc/dpp/ARAD/arad_api_diagnostics.h>
#include <soc/dpp/ARAD/arad_api_ingress_scheduler.h>
#include <soc/dpp/ARAD/arad_ingress_scheduler.h>
#include <soc/dpp/ARAD/arad_mgmt.h>
#include <soc/dpp/ARAD/arad_init.h>
#include <soc/dpp/ARAD/arad_fabric.h>
#include <soc/dpp/ARAD/arad_interrupts.h>
#include <soc/dpp/ARAD/arad_cnt.h>
#include <soc/dpp/ARAD/arad_dram.h>
#include <soc/dpp/ARAD/arad_scheduler_device.h>
#include <soc/dpp/ARAD/arad_scheduler_ports.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_egr_queuing.h>
#include <soc/dpp/ARAD/arad_parser.h>
#include <soc/dpp/PORT/arad_ps_db.h>
#include <soc/dpp/ARAD/arad_egr_prog_editor.h>
#include <soc/dpp/ARAD/arad_cell.h>
#include <soc/dpp/ARAD/NIF/ports_manager.h>
#include <soc/dpp/ARAD/NIF/common_drv.h>
#include <soc/dpp/multicast_imp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_metering.h>
#include <soc/dpp/ARAD/arad_init.h>



CONST mbcm_dpp_functions_t mbcm_arad_driver = {
    arad_action_cmd_snoop_set,
    arad_action_cmd_snoop_get, 
    arad_action_cmd_mirror_set, 
    arad_action_cmd_mirror_get, 
    soc_arad_ports_stop_egq,   
    NULL, /* arad_cnm_cp_set */ 
    NULL, /* arad_cnm_cp_get */ 
    NULL, /* arad_cnm_q_mapping_set */ 
    NULL, /* arad_cnm_q_mapping_get */ 
    NULL, /* arad_cnm_congestion_test_set */ 
    NULL, /* arad_cnm_congestion_test_get */ 
    NULL, /* arad_cnm_cp_profile_set */ 
    NULL, /* arad_cnm_cp_profile_get */ 
    NULL, /* arad_cnm_sampling_profile_set */ 
    NULL, /* arad_cnm_sampling_profile_get */ 
    NULL, /* arad_cnm_cpq_pp_set */ 
    NULL, /* arad_cnm_cpq_pp_get */ 
    NULL, /* arad_cnm_cpq_sampling_set */ 
    NULL, /* arad_cnm_cpq_sampling_get */ 
    NULL, /* arad_cnm_intercept_timer_set */ 
    NULL, /* arad_cnm_intercept_timer_get */ 
    arad_cnt_counters_set,
    arad_cnt_channel_to_fifo_mapping_set,
    arad_cnt_channel_to_fifo_mapping_get,
    arad_cnt_dma_unset,
    arad_cnt_dma_set,
    arad_cnt_counters_get,
    arad_cnt_status_get,
    arad_cnt_engine_to_fifo_dma_index,
    arad_cnt_fifo_dma_offset_in_engine,
    arad_cnt_max_we_val_get,
    arad_cnt_algorithmic_read,
    arad_cnt_direct_read,
    arad_cnt_q2cnt_id,
    NULL, /* arad_cnt_cnt2q_id */
    arad_cnt_lif_counting_set,
    arad_cnt_lif_counting_get,
    arad_cnt_lif_counting_range_set,
    arad_cnt_lif_counting_range_get,
    NULL, /* mbcm_dpp_cnt_out_lif_counting_range_get */
    NULL, /*arad_cnt_base_val_set*/
    arad_cnt_epni_regs_set_unsafe,
    arad_cnt_meter_hdr_compensation_set,
    arad_cnt_meter_hdr_compensation_get,
    arad_diag_last_packet_info_get, /* arad_diag_last_packet_info_get */ 
    NULL, /* arad_diag_sample_enable_set */ 
    NULL, /* arad_diag_sample_enable_get */ 
    NULL, /* arad_diag_signals_dump */ 
    arad_egr_q_prio_set,
    arad_egr_q_prio_get,
    arad_egr_q_profile_map_set,
    arad_egr_q_profile_map_get,
    arad_egr_q_cgm_interface_set,
    arad_egr_q_fqp_scheduler_config,
    soc_arad_egr_congestion_statistics_get,
    NULL, /* arad_fabric_line_coding_set */ 
    NULL, /* arad_fabric_line_coding_get */ 
    NULL, /* arad_fabric_pcp_dest_mode_config_set */
    NULL, /* arad_fabric_pcp_dest_mode_config_get */
    arad_port_fabric_clk_freq_init,
    soc_arad_pll_info_get,
    NULL, /* arad_interface_id_verify */
    NULL, /* arad_if_type_from_id */ 
    NULL, /* arad_mal_equivalent_id_verify */  
    arad_mgmt_ipt_init,
    soc_arad_fabric_stat_init,
    soc_arad_stat_nif_init,
    soc_arad_stat_path_info_get,
    arad_info_config_custom_reg_access,
    soc_arad_mapping_stat_get,
    soc_arad_stat_counter_length_get,
    soc_arad_stat_controlled_counter_enable_get,
    arad_itm_committed_q_size_set,
    arad_itm_committed_q_size_get,
    NULL, /* mbcm_dpp_itm_max_latency_pkts_get */    
    NULL, /* mbcm_dpp_itm_rate_limit_mpps_set */
    NULL, /* mbcm_dpp_itm_rate_limit_mpps_get */
    NULL, /* soc_pb_cosq_gport_sched_set */
    NULL, /* soc_pb_cosq_gport_sched_get */
    arad_port_pp_port_set, 
    arad_port_pp_port_get,
    arad_port_to_pp_port_map_set, 
    NULL, /* arad_port_to_pp_port_map_get */
    arad_port_control_low_latency_set,
    arad_port_control_fec_error_detect_set,
    arad_port_control_low_latency_get,
    arad_port_control_fec_error_detect_get, 
    NULL, /*mbcm_dpp_port_extract_cig_from_llfc_enable_set*/
    NULL, /*mbcm_dpp_port_extract_cig_from_llfc_enable_get*/
    NULL, /* arad_port_forwarding_header_set */ 
    NULL, /* arad_port_forwarding_header_get */
    NULL, /* mbcm_dpp_port_nrdy_th_profile_set */
    NULL, /* mbcm_dpp_port_nrdy_th_profile_get */
    NULL, /* mbcm_dpp_port_nrdy_th_optimial_value_get */   
    arad_ports_logical_sys_id_build,
    arad_sys_virtual_port_to_local_port_map_set_unsafe,
    NULL, /* arad_ssr_buff_size_get */ 
    NULL, /* arad_ssr_to_buff */ 
    NULL, /* arad_ssr_from_buff */ 
    NULL, /* arad_ssr_is_device_init_done */ 
    NULL, /* arad_stack_global_info_set */ 
    NULL, /* arad_stack_global_info_get */ 
    NULL, /* arad_stack_port_distribution_info_set */ 
    NULL, /* arad_stack_port_distribution_info_get */
    arad_tdm_ftmh_set,
    arad_tdm_ftmh_get,
    arad_tdm_opt_size_set,
    arad_tdm_opt_size_get,
    arad_tdm_stand_size_range_set,
    arad_tdm_stand_size_range_get,
    NULL, /* arad_tdm_mc_static_route_set INVALID for ARAD */ 
    NULL, /* arad_tdm_mc_static_route_get INVALID for ARAD */ 
    arad_tdm_port_packet_crc_set,
    arad_tdm_port_packet_crc_get,
    arad_tdm_direct_routing_set,
    arad_tdm_direct_routing_get,
    arad_tdm_direct_routing_profile_map_set,
    arad_tdm_direct_routing_profile_map_get,
    arad_tdm_ifp_get,
    arad_tdm_ifp_set,
    NULL, /* arad_read_from_fe600 */ 
    NULL, /* arad_write_to_fe600 */ 
    NULL, /* arad_indirect_read_from_fe600 */ 
    NULL, /* arad_indirect_write_to_fe600 */ 
    NULL, /* arad_cpu2cpu_write */ 
    NULL, /* arad_cpu2cpu_read */ 
    NULL, /* arad_cell_mc_tbl_write */ 
    NULL, /* arad_cell_mc_tbl_read */ 
    NULL, /* arad_diag_ipt_rate_get */ 
    NULL, /* arad_diag_iddr_set */ 
    NULL, /* arad_diag_iddr_get */ 
    NULL, /* arad_diag_regs_dump */ 
    NULL, /* arad_diag_tbls_dump */ 
    NULL, /* arad_diag_tbls_dump_all */ 
    NULL, /* arad_nif_diag_last_packet_get */ 
    NULL, /* arad_diag_soft_error_test_start */ 
    NULL, /* arad_egq_resources_print */
    arad_egr_threshold_types_verify, 
    arad_egr_ofp_thresh_type_set,
    arad_egr_ofp_thresh_type_get,
    arad_egr_sched_drop_set,
    arad_egr_sched_drop_get,
    arad_egr_unsched_drop_set,
    arad_egr_unsched_drop_get,
    arad_egr_dev_fc_set,
    arad_egr_dev_fc_get,
    NULL, /* arad_egr_xaui_spaui_fc_set */ 
    NULL, /* arad_egr_xaui_spaui_fc_get */ 
    arad_egr_ofp_fc_set,
    arad_egr_sched_port_fc_thresh_set,
    arad_egr_sched_q_fc_thresh_set,
    arad_egr_ofp_fc_get,
    NULL, /* mbcm_dpp_egr_mci_fc_set Invalid for ARAD */ 
    NULL, /* mbcm_dpp_egr_mci_fc_get, Invalid for ARAD */ 
    NULL, /* mbcm_dpp_egr_mci_fc_enable_set, Invalid for ARAD */ 
    NULL, /* mbcm_dpp_egr_mci_fc_enable_get, Invalid for ARAD */ 
    NULL, /* mbcm_dpp_egr_ofp_sch_mode_set, Invalid for ARAD */ 
    NULL, /* mbcm_dpp_egr_ofp_sch_mode_get,Invalid for ARAD */ 
    arad_egr_ofp_scheduling_set,
    arad_egr_ofp_scheduling_get,
    NULL, /* mbcm_dpp_egr_unsched_drop_prio_set, Invalid for ARAD */ 
    NULL, /* mbcm_dpp_egr_unsched_drop_prio_get, Invalid for ARAD */ 
    arad_sch_device_rate_entry_set_unsafe,
    arad_sch_device_rate_entry_get_unsafe,
    NULL, /* mbcm_dpp_sch_device_rate_entry_core_set, Invalid for ARAD */
    NULL, /* mbcm_dpp_sch_device_rate_entry_core_get, Invalid for ARAD */
    arad_sch_if_shaper_rate_set,
    arad_sch_if_shaper_rate_get,
    arad_sch_device_if_weight_idx_set,
    arad_sch_device_if_weight_idx_get,
    arad_sch_if_weight_conf_set,
    arad_sch_if_weight_conf_get,
    arad_sch_class_type_params_set, 
    arad_sch_class_type_params_get, 
    arad_sch_class_type_params_table_set, 
    arad_sch_class_type_params_table_get, 
    arad_sch_port_sched_set, 
    arad_sch_port_sched_get, 
    NULL, /*   arad_sch_port_hp_class_conf_set, Invalid for ARAD */
    NULL, /*   arad_sch_port_hp_class_conf_get, Invalid for ARAD */
    arad_sch_slow_max_rates_set, 
    arad_sch_slow_max_rates_get,
    NULL ,/*mbcm_dpp_sch_slow_max_rates_per_level_set*/ 
    NULL ,/*mbcm_dpp_sch_slow_max_rates_per_level_get*/ 
    arad_sch_aggregate_set, 
    arad_sch_aggregate_group_set, 
    arad_sch_aggregate_get, 
    arad_sch_flow_delete, 
    arad_sch_flow_set, 
    arad_sch_flow_get, 
    arad_sch_flow_status_set,
    arad_sch_flow_ipf_config_mode_set,
    arad_sch_flow_ipf_config_mode_get,
    arad_sch_per1k_info_set, 
    arad_sch_per1k_info_get, 
    arad_sch_flow_to_queue_mapping_set, 
    arad_sch_flow_to_queue_mapping_get, 
    arad_sch_flow_id_verify_unsafe, 
    arad_sch_se_id_verify_unsafe, 
    arad_sch_port_id_verify_unsafe, 
    arad_sch_k_flow_id_verify_unsafe, 
    arad_sch_quartet_id_verify_unsafe, 
    NULL, /*arad_sch_se2port_id, */
    arad_sch_se2port_tc_id,
    arad_sch_flow2se_id, 
    NULL, /* arad_sch_port2se_id, */
    arad_sch_port_tc2se_id,
    arad_sch_se2flow_id, 
    arad_sch_se_get_type_by_id,
    arad_sch_e2e_interface_allocate,
    arad_sch_e2e_interface_deallocate,
    NULL, /* mbcm_dpp_sch_prio_propagation_enable_set */
    NULL, /* mbcm_dpp_sch_prio_propagation_enable_get */
    NULL, /* mbcm_dpp_sch_prio_propagation_port_set*/
    NULL, /* mbcm_dpp_sch_prio_propagation_port_get*/
    arad_sch_shds_tbl_get_unsafe,
    arad_sch_shds_tbl_set_unsafe,
    arad_fabric_fc_enable_set,
    arad_fabric_fc_enable_get,    
    arad_fabric_fc_shaper_get,
    arad_fabric_fc_shaper_set,
    NULL, /* arad_fabric_cell_format_get */ 
    NULL, /* arad_fabric_coexist_set */ 
    NULL, /* arad_fabric_coexist_get */ 
    NULL, /* arad_fabric_stand_alone_fap_mode_get */ 
    NULL, /* arad_fabric_connect_mode_set */ 
    NULL, /* arad_fabric_connect_mode_get */ 
    NULL, /* arad_fabric_fap20_map_set */ 
    NULL, /* arad_fabric_fap20_map_get */  
    arad_fabric_priority_set,
    arad_fabric_priority_get,
    arad_fabric_topology_status_connectivity_get,
    NULL, /* arad_fabric_links_status_get */ 
    arad_fabric_aldwp_config,
    NULL, /* arad_fabric_topology_status_connectivity_print */
    arad_fabric_nof_links_get,
    arad_fabric_gci_enable_set,
    arad_fabric_gci_enable_get,
    arad_fabric_gci_config_set,
    arad_fabric_gci_config_get,
    arad_fabric_gci_backoff_masks_init,
    arad_fabric_llfc_threshold_set,
    arad_fabric_llfc_threshold_get,
    arad_fabric_rci_enable_set,
    arad_fabric_rci_enable_get,
    arad_fabric_rci_config_set,
    arad_fabric_rci_config_get,
    arad_fabric_minimal_links_to_dest_set,
    arad_fabric_minimal_links_to_dest_get,
    NULL, /*arad_fabric_minimal_links_all_reachable_set*/
    NULL, /*arad_fabric_minimal_links_all_reachable_get*/
    arad_fabric_link_tx_traffic_disable_set,
    arad_fabric_link_tx_traffic_disable_get,
    NULL, /* arad_fabric_link_thresholds_pipe_set */
    NULL, /* arad_fabric_link_thresholds_pipe_get */
    NULL, /* arad_fabric_cosq_control_backward_flow_control_set */
    NULL, /* arad_fabric_cosq_control_backward_flow_control_get */
    NULL, /* arad_fabric_egress_core_cosq_gport_sched_set */
    NULL, /* arad_fabric_egress_core_cosq_gport_sched_get */
    NULL, /* arad_fabric_cosq_gport_rci_threshold_set */
    NULL, /* arad_fabric_cosq_gport_rci_threshold_get */
    NULL, /* arad_fabric_cosq_gport_priority_drop_threshold_set */
    NULL, /* arad_fabric_cosq_gport_priority_drop_threshold_get */
    NULL, /* arad_fabric_link_topology_set*/
    NULL, /* arad_fabric_link_topology_get */
    NULL, /* arad_fabric_link_topology_unset */
    NULL, /* jer_mesh_tdm_multicast_set */
    NULL, /* arad_fabric_multicast_set */
    NULL, /* arad_fabric_multicast_get */
    NULL, /* arad_fabric_modid_group_set */
    NULL, /* arad_fabric_modid_group_get */
    NULL, /* arad_fabric_local_dest_id_verify */
    NULL, /* arad_fabric_rci_thresholds_config_set*/
    NULL, /* arad_fabric_rci_thresholds_config_get*/
    NULL, /* arad_fabric_link_repeater_enable_set*/
    NULL, /* arad_fabric_link_repeater_enable_get*/
    NULL, /* arad_fabric_queues_info_get*/
    arad_fabric_cpu2cpu_write,
    NULL, /* arad_fabric_mesh_topology_get*/
    arad_fabric_mesh_check,
    NULL, /* arad_fabric_rx_fifo_status_get*/
    NULL, /* arad_fabric_port_sync_e_link_set*/
    NULL, /* arad_fabric_port_sync_e_link_get*/
    NULL, /* arad_port_sync_e_divider_set*/
    NULL, /* arad_port_sync_e_divider_get*/
    NULL, /* arad_fabric_sync_e_enable_get*/
    NULL, /* mbcm_dpp_fabric_force_set*/
    NULL, /* arad_fabric_stack_module_all_reachable_ignore_id_set */
    NULL, /* arad_fabric_stack_module_all_reachable_ignore_id_get */
    NULL, /* arad_fabric_stack_module_max_all_reachable_set */
    NULL, /* arad_fabric_stack_module_max_all_reachable_get */ 
    NULL, /* arad_fabric_stack_module_max_set */
    NULL, /* arad_fabric_stack_module_max_get */ 
    NULL, /* arad_fabric_stack_module_devide_by_32_verify*/ 
    arad_fabric_cell_cpu_data_get,
    NULL,/* arad_fabric_efms_enable_set */
    NULL,/* arad_fabric_efms_enable_get */
    arad_ipq_explicit_mapping_mode_info_set,
    arad_ipq_explicit_mapping_mode_info_get,
    arad_ipq_traffic_class_map_set,
    arad_ipq_traffic_class_map_get,
    arad_ipq_traffic_class_multicast_priority_map_set,
    arad_ipq_traffic_class_multicast_priority_map_get,
    arad_ipq_destination_id_packets_base_queue_id_set,
    arad_ipq_destination_id_packets_base_queue_id_get,
    arad_ipq_queue_interdigitated_mode_set,
    arad_ipq_queue_interdigitated_mode_get,
    arad_ipq_queue_to_flow_mapping_set,
    arad_ipq_queue_to_flow_mapping_get,
    arad_ipq_queue_qrtt_unmap,
    arad_ipq_quartet_reset,
    NULL, /* arad_ipq_attached_flow_port_get */ 
    arad_ipq_tc_profile_set,
    arad_ipq_tc_profile_get,
    arad_ipq_tc_profile_map_set,
    arad_ipq_tc_profile_map_get,
    arad_ipq_stack_lag_packets_base_queue_id_set,
    arad_ipq_stack_lag_packets_base_queue_id_get,
    arad_ipq_stack_fec_map_stack_lag_set,
    arad_ipq_stack_fec_map_stack_lag_get,
    NULL, /* mbcm_dpp_default_invalid_queue_set */
    NULL, /* mbcm_dpp_default_invalid_queue_get */
    arad_ingress_scheduler_clos_bandwidth_set,
    arad_ingress_scheduler_clos_bandwidth_get,
    arad_ingress_scheduler_mesh_bandwidth_set,
    arad_ingress_scheduler_mesh_bandwidth_get,
    arad_ingress_scheduler_clos_sched_set,
    arad_ingress_scheduler_clos_sched_get,
    arad_ingress_scheduler_mesh_sched_set,
    arad_ingress_scheduler_mesh_sched_get,
    arad_ingress_scheduler_clos_burst_set,
    arad_ingress_scheduler_clos_burst_get,
    arad_ingress_scheduler_mesh_burst_set,
    arad_ingress_scheduler_mesh_burst_get,
    arad_ingress_scheduler_clos_set,
    arad_ingress_scheduler_clos_get,
    NULL, /* mbcm_dpp_ingress_scheduler_mesh_slow_start_set */
    NULL, /* mbcm_dpp_ingress_scheduler_mesh_slow_start_get */ 
    arad_ingress_scheduler_init,
    NULL, /* mbcm_dpp_cosq_control_range_dram_mix_dbuff_threshold_get */
    NULL, /* mbcm_dpp_cosq_control_range_dram_mix_dbuff_threshold_set */
    NULL, /* mbcm_dpp_cosq_control_range_ocb_committed_multicast_get */
    NULL, /* mbcm_dpp_cosq_control_range_ocb_committed_multicast_set */
    NULL, /* mbcm_dpp_cosq_control_range_ocb_eligible_multicast_get */
    NULL, /* mbcm_dpp_cosq_control_range_ocb_eligible_multicast_set */
    arad_itm_dram_buffs_get,
    soc_ardon_dram_info_verify,
    soc_arad_user_buffer_dram_write,
    soc_arad_user_buffer_dram_read,
    soc_arad_validate_dram_address,
    NULL, /* mbcm_dpp_dram_recovery_init */
    soc_arad_cache_table_update_all,
    arad_itm_glob_rcs_fc_set, 
    arad_itm_glob_rcs_fc_get, 
    arad_itm_glob_rcs_drop_set, 
    arad_itm_glob_rcs_drop_get, 
    arad_itm_category_rngs_set, 
    arad_itm_category_rngs_get, 
    arad_itm_admit_test_tmplt_set, 
    arad_itm_admit_test_tmplt_get,
    NULL, /* arad_itm_init */
    arad_itm_cr_request_set, 
    arad_itm_cr_request_get, 
    arad_itm_cr_discount_set, 
    arad_itm_cr_discount_get, 
    arad_itm_queue_test_tmplt_set, 
    arad_itm_queue_test_tmplt_get, 
    arad_itm_wred_exp_wq_set, 
    arad_itm_wred_exp_wq_get, 
    arad_itm_wred_set, 
    arad_itm_wred_get, 
    arad_itm_tail_drop_set, 
    arad_itm_tail_drop_get, 
    arad_itm_fadt_tail_drop_set,
    arad_itm_fadt_tail_drop_get,   
    arad_itm_cr_wd_set, 
    arad_itm_cr_wd_get, 
    arad_itm_enable_ecn,
    arad_itm_get_ecn_enabled,
    NULL, /* mbcm_dpp_itm_dram_bound_set */
    NULL, /* mbcm_dpp_itm_dram_bound_get */
    arad_itm_vsq_qt_rt_cls_set, 
    arad_itm_vsq_qt_rt_cls_get, 
    arad_itm_vsq_fc_set, 
    arad_itm_vsq_fc_get, 
    arad_itm_vsq_tail_drop_set, 
    arad_itm_vsq_tail_drop_get, 
    arad_itm_vsq_tail_drop_default_get,
    NULL, /* mbcm_dpp_itm_vsq_fc_default_get */
    NULL, /* mbcm_dpp_itm_vsq_src_port_rjct_set */
    NULL, /* mbcm_dpp_itm_vsq_src_port_rjct_get */
    NULL, /* mbcm_dpp_itm_vsq_pg_rjct_set */
    NULL, /* mbcm_dpp_itm_vsq_pg_rjct_get */
    arad_itm_vsq_wred_gen_set,
    arad_itm_vsq_wred_gen_get,
    arad_itm_vsq_wred_set, 
    arad_itm_vsq_wred_get, 
    arad_itm_vsq_counter_set, 
    arad_itm_vsq_counter_get, 
    arad_itm_vsq_counter_read, 
    NULL, /* arad_itm_vsq_pg_tc_profile_mapping_set */
    NULL, /* arad_itm_vsq_pg_tc_profile_mapping_get */
    NULL, /* arad_itm_vsq_pg_tc_profile_set         */
    NULL, /* arad_itm_vsq_pg_tc_profile_get         */
    NULL, /* arad_itm_vsq_pg_prm_set                */
    NULL, /* arad_itm_vsq_pg_prm_get                */
    NULL, /* qax_itm_vsq_pb_prm_set                 */
    NULL, /* qax_itm_vsq_pb_prm_get                 */
    NULL, /* arad_itm_src_vsqs_mapping_set          */
    NULL, /* arad_itm_src_vsqs_mapping_set          */
    NULL, /* arad_itm_vsq_src_port_get              */
    NULL, /* arad_itm_vsq_pg_mapping_get            */
    NULL, /* arad_itm_vsq_pg_ocb_set                */
    NULL, /* arad_itm_vsq_pg_ocb_get                */
    arad_itm_queue_is_ocb_only_get,
    arad_itm_queue_info_set, 
    arad_itm_queue_info_get, 
    arad_itm_ingress_shape_set, 
    arad_itm_ingress_shape_get, 
    arad_itm_priority_map_tmplt_set, 
    arad_itm_priority_map_tmplt_get, 
    arad_itm_priority_map_tmplt_select_set, 
    arad_itm_priority_map_tmplt_select_get, 
    arad_itm_sys_red_drop_prob_set, 
    arad_itm_sys_red_drop_prob_get, 
    arad_itm_sys_red_queue_size_boundaries_set, 
    arad_itm_sys_red_queue_size_boundaries_get, 
    arad_itm_sys_red_q_based_set, 
    arad_itm_sys_red_q_based_get, 
    arad_itm_sys_red_eg_set, 
    arad_itm_sys_red_eg_get, 
    arad_itm_sys_red_glob_rcs_set, 
    arad_itm_sys_red_glob_rcs_get, 
    arad_itm_vsq_index_global2group, 
    arad_itm_vsq_index_group2global,
    NULL, /* arad_itm_vsq_src_reserve_set,      */
    NULL, /* arad_itm_vsq_src_reserve_get,      */
    NULL, /* arad_itm_resource_allocation_set,  */
    NULL, /* arad_itm_resource_allocation_get,  */
    NULL, /* mbcm_dpp_itm_global_resource_allocation_set,  */
    NULL, /* mbcm_dpp_itm_global_resource_allocation_get,  */
    arad_itm_dyn_total_thresh_set,
    arad_itm_queue_dyn_info_get, 
    arad_iqm_static_tbl_set_unsafe,
    arad_iqm_static_tbl_get_unsafe,
    NULL, /*mbcm_dpp_itm_in_pp_port_scheduler_compensation_profile_set*/
    NULL, /*mbcm_dpp_itm_in_pp_port_scheduler_compensation_profile_get*/
    arad_iqm_dynamic_tbl_get_unsafe,
    arad_itm_congestion_statistics_get,
    NULL, /* mbcm_dpp_itm_congestion_statistics_get */
    arad_ingress_drop_status,
    NULL, /* mbcm_dpp_itm_profile_ocb_only_set */
    NULL, /* mbcm_dpp_itm_profile_ocb_only_get */
    NULL, /* mbcm_dpp_itm_credits_adjust_size_set */
    NULL, /* mbcm_dpp_itm_credits_adjust_size_get */
    NULL, /* mbcm_dpp_itm_sch_final_delta_map_set */
    NULL, /* mbcm_dpp_itm_sch_final_delta_map_get */
    NULL, /* mbcm_dpp_itm_sch_final_delta_set */
    NULL, /* mbcm_dpp_itm_sch_final_delta_get */          
    dpp_mult_eg_bitmap_group_range_set,
    dpp_mult_eg_bitmap_group_range_get,
    NULL, /* arad_mult_eg_group_open */
    NULL, /* arad_mult_eg_group_update */
    dpp_mult_eg_group_set,
    dpp_mult_eg_group_close,
    NULL, /*dpp_mult_eg_port_add*/
    dpp_mult_eg_reps_add,
    NULL, /*dpp_mult_eg_port_remove*/
    dpp_mult_eg_reps_remove,
    dpp_mult_eg_group_size_get,
    NULL, /* arad_mult_eg_group_get */
    dpp_mult_eg_get_group,
    NULL, /* dpp_mult_eg_vlan_membership_group_open, */
    dpp_mult_eg_bitmap_group_create,
    dpp_mult_eg_bitmap_group_update,
    dpp_mult_eg_bitmap_group_close,
    dpp_mult_eg_bitmap_group_port_add, 
    dpp_mult_eg_bitmap_group_port_remove,
    dpp_mult_eg_bitmap_group_bm_add, 
    dpp_mult_eg_bitmap_group_bm_remove,
    dpp_mult_eg_bitmap_group_get,
    dpp_mcds_multicast_init2,
    arad_mult_fabric_traffic_class_to_multicast_cls_map_set,
    arad_mult_fabric_traffic_class_to_multicast_cls_map_get,
    arad_mult_fabric_base_queue_set,
    arad_mult_fabric_base_queue_get,
    arad_mult_fabric_credit_source_set,
    arad_mult_fabric_credit_source_get,
    arad_mult_fabric_enhanced_set,
    arad_mult_fabric_enhanced_get,
    arad_mult_fabric_flow_control_set,
    arad_mult_fabric_flow_control_get,
    NULL, /* arad_mult_fabric_active_links_set */ 
    NULL, /* arad_mult_fabric_active_links_get */ 
    dpp_mult_does_group_exist_ext,
    dpp_mult_ing_traffic_class_map_set,
    dpp_mult_ing_traffic_class_map_get,
    dpp_mult_ing_group_open,
    dpp_mult_ing_group_update,
    dpp_mult_ing_group_close,
    dpp_mult_ing_destination_add,
    arad_mult_ing_encode_entry,
    dpp_mult_ing_destination_remove,
    dpp_mult_ing_group_size_get,
    NULL, /* arad_mult_ing_group_get */
    dpp_mult_ing_get_group,
    /* arad_ofp_rates_set doesn't exist for ARAD */
    /* arad_ofp_rates_get doesn't exist for ARAD */
    /* arad_ofp_all_ofp_rates_get */ 
    /* arad_ofp_rates_single_port_set not exist for ARAD */ 
    /* arad_ofp_rates_single_port_get not exist for ARAD */ 
    /* arad_ofp_rates_mal_shaper_set not exist for ARAD */ 
    /* arad_ofp_rates_mal_shaper_get not exist for ARAD */ 
    /*arad_ofp_rates_egq_calendar_validate,*/
    /* arad_ofp_rates_update_device_set */
    /*arad_ofp_rates_update_device_get*/
    NULL, /* arad_pkt_packet_callback_set */ 
    NULL, /* arad_pkt_packet_callback_get */ 
    NULL, /* arad_pkt_packet_send */ 
    NULL, /* arad_pkt_packet_recv */ 
    NULL, /* arad_pkt_packet_receive_mode_set */ 
    NULL, /* arad_pkt_packet_receive_mode_get */ 
    arad_sys_phys_to_local_port_map_set,
    arad_sys_phys_to_local_port_map_get,
    arad_local_to_sys_phys_port_map_get,
    arad_modport_to_sys_phys_port_map_get,
    arad_port_to_interface_map_set,
    arad_port_to_interface_map_get,
    arad_ports_is_port_lag_member,
    NULL, /* arad_ports_lag_set */ 
    NULL, /* arad_ports_lag_get */ 
    NULL, /* arad_ports_lag_sys_port_add */ 
    NULL, /* arad_ports_lag_member_add */ 
    NULL, /* arad_ports_lag_sys_port_remove */ 
    NULL, /* arad_ports_lag_order_preserve_set */ 
    NULL, /* arad_ports_lag_order_preserve_get */ 
    arad_port_header_type_set,
    arad_port_header_type_get,
    NULL, /* arad_ports_mirror_inbound_set */ 
    NULL, /* arad_ports_mirror_inbound_get */ 
    NULL, /* arad_ports_mirror_outbound_set */ 
    NULL, /* arad_ports_mirror_outbound_get */ 
    NULL, /* arad_ports_snoop_set */ 
    NULL, /* arad_ports_snoop_get */ 
    NULL, /* arad_ports_itmh_extension_set, */
    NULL, /* arad_ports_itmh_extension_get, */
    NULL, /* arad_ports_shaping_header_set */ 
    NULL, /* arad_ports_shaping_header_get */ 
    NULL, /* arad_ports_forwarding_header_set */ 
    NULL, /* arad_ports_forwarding_header_get */ 
    NULL, /* arad_ports_stag_set */ 
    NULL, /* arad_ports_stag_get */ 
    arad_ports_ftmh_extension_set,
    arad_ports_ftmh_extension_get,
    arad_ports_reference_clock_set,
    arad_ports_port_to_nif_id_get,
    arad_ports_mirrored_channel_and_context_map,
    arad_port_egr_hdr_credit_discount_type_set,
    arad_port_egr_hdr_credit_discount_type_get,
    arad_port_egr_hdr_credit_discount_select_set,
    arad_port_egr_hdr_credit_discount_select_get,
    arad_port_stacking_info_set,
    arad_port_stacking_info_get,
    arad_port_stacking_route_history_bitmap_set,
    arad_port_direct_lb_key_set,
    arad_port_direct_lb_key_get,
    arad_port_direct_lb_key_min_set,
    arad_port_direct_lb_key_max_set,
    arad_port_direct_lb_key_min_get,
    arad_port_direct_lb_key_max_get,
    arad_port_synchronize_lb_key_tables_at_egress,
    arad_port_switch_lb_key_tables,
    arad_port_rx_enable_get,
    arad_port_rx_enable_set,
    NULL, /* mbcm_dpp_port_tx_enable_get */ 
    NULL, /* mbcm_dpp_port_tx_enable_set */ 
    arad_port_ingr_reassembly_context_get,
    arad_port_rate_egress_pps_set,
    arad_port_rate_egress_pps_get,
    arad_port_protocol_offset_verify,
    arad_port_cable_diag,
    arad_ports_swap_set,
    arad_ports_swap_get,
    arad_ports_pon_tunnel_info_set,
    arad_ports_pon_tunnel_info_get,
    arad_ports_extender_mapping_enable_set,
    arad_ports_extender_mapping_enable_get,
	arad_ports_tm_port_var_set,
	arad_ports_tm_port_var_get,
    NULL, /* arad_read_fld */ 
    NULL, /* arad_write_fld */ 
    NULL, /* arad_read_reg */ 
    NULL, /* arad_write_reg */ 
    NULL, /* arad_status_fld_poll */
    arad_brdc_fsrd_blk_id_set,
    arad_mgmt_credit_worth_set,  
    arad_mgmt_credit_worth_get,
	arad_plus_mgmt_module_to_credit_worth_map_set,
	arad_plus_mgmt_module_to_credit_worth_map_get,
	arad_plus_mgmt_credit_worth_remote_set,
	arad_plus_mgmt_credit_worth_remote_get,
	arad_plus_mgmt_change_all_faps_credit_worth_unsafe,
	arad_mgmt_all_ctrl_cells_enable_get,
    arad_mgmt_all_ctrl_cells_enable_set,
    arad_force_tdm_bypass_traffic_to_fabric_set,
    arad_force_tdm_bypass_traffic_to_fabric_get,
    arad_mgmt_enable_traffic_set,
    arad_mgmt_enable_traffic_get,
    arad_register_device,
    arad_unregister_device,
    arad_mgmt_system_fap_id_set,
    arad_mgmt_system_fap_id_get_unsafe,
    arad_mgmt_tm_domain_set,
    arad_mgmt_tm_domain_get,
    NULL, /* mbcm_dpp_mgmt_dma_fifo_channel_free_find */
    NULL, /* mbcm_dpp_mgmt_dma_fifo_channel_set */
    NULL, /* mbcm_dpp_mgmt_dma_fifo_channel_get */    
    NULL, /* arad_hpu_itmh_build_verify */ 
    NULL, /* arad_hpu_ftmh_build_verify */ 
    NULL, /* arad_hpu_otmh_build_verify */ 
    NULL, /* arad_hpu_itmh_build */ 
    NULL, /* arad_hpu_itmh_parse */ 
    NULL, /* arad_hpu_ftmh_build */ 
    NULL, /* arad_hpu_ftmh_parse */ 
    NULL, /* arad_hpu_otmh_build */ 
    NULL, /* arad_hpu_otmh_parse */ 
    arad_port_loopback_set,
    arad_port_loopback_get,
    arad_nif_synce_clk_sel_port_set, 
    arad_nif_synce_clk_sel_port_get, 
    arad_nif_synce_clk_div_set, 
    arad_nif_synce_clk_div_get, 
    arad_port_link_up_mac_update,
    arad_flow_and_up_info_get,
    arad_ips_non_empty_queues_info_get,
    arad_itm_pfc_tc_map_set,
    arad_itm_pfc_tc_map_get,
    arad_fc_gen_cal_set,
    arad_fc_gen_cal_get,
    arad_fc_gen_inbnd_set,
    arad_fc_gen_inbnd_get,
    arad_fc_gen_inbnd_glb_hp_set,
    arad_fc_gen_inbnd_glb_hp_get,
    arad_fc_rec_cal_set,
    arad_fc_rec_cal_get,
    arad_fc_pfc_generic_bitmap_set,
    arad_fc_pfc_generic_bitmap_get,
    arad_fc_port_fifo_threshold_set,
    arad_fc_port_fifo_threshold_get,
    arad_egr_dsp_pp_to_base_q_pair_get,
    arad_egr_dsp_pp_to_base_q_pair_set,
    arad_egr_dsp_pp_priorities_mode_get,
    arad_egr_dsp_pp_priorities_mode_set,
    arad_egr_dsp_pp_shaper_mode_set_unsafe,
    arad_egr_queuing_dev_set,
    arad_egr_queuing_dev_get,
    arad_egr_queuing_global_drop_set,
    arad_egr_queuing_global_drop_get,
    arad_egr_queuing_sp_tc_drop_set,
    arad_egr_queuing_sp_tc_drop_get,
    arad_egr_queuing_sch_unsch_drop_set,
    NULL, /* mbcm_dpp_egr_queuing_sch_unsch_drop_get_unsafe */
    arad_egr_queuing_sp_reserved_set,
    arad_egr_queuing_sp_reserved_get,
    arad_egr_queuing_global_fc_set,
    arad_egr_queuing_global_fc_get,
    arad_egr_queuing_mc_tc_fc_set,
    arad_egr_queuing_mc_tc_fc_get,
    arad_egr_queuing_mc_cos_map_set,
    arad_egr_queuing_mc_cos_map_get,
    arad_egr_queuing_if_fc_set,
    arad_egr_queuing_if_fc_get,
    arad_egr_queuing_if_fc_uc_max_set,
    arad_egr_queuing_if_fc_uc_set,
    arad_egr_queuing_if_fc_uc_get,
    arad_egr_queuing_if_fc_mc_set,
    arad_egr_queuing_if_fc_mc_get,
    arad_egr_queuing_if_uc_map_set,
    arad_egr_queuing_if_mc_map_set,
    /*arad_ofp_rates_port_priority_shaper_set*/
    /*arad_ofp_rates_port_priority_shaper_get*/
    /*arad_ofp_rates_tcg_shaper_set,*/
    /*arad_ofp_rates_tcg_shaper_get,*/
    arad_egr_queuing_ofp_tcg_set,
    arad_egr_queuing_ofp_tcg_get,
    arad_egr_queuing_tcg_weight_set,
    arad_egr_queuing_tcg_weight_get,
    arad_egr_queuing_is_high_priority_port_get,
    arad_egr_queuing_init_thresholds,
    arad_egr_queuing_egr_interface_alloc,
    arad_egr_queuing_egr_interface_free,
    NULL, /* mbcm_dpp_egr_queuing_nrdy_th_profile_data_set */
    NULL, /* mbcm_dpp_egr_queuing_nrdy_th_profile_data_get */
    arad_sch_port_tcg_weight_set,
    arad_sch_port_tcg_weight_get,
    soc_pm_frame_max_set,
    soc_pm_frame_max_get,
    arad_mgmt_ocb_voq_eligible_dynamic_set,
    NULL, /* mbcm_dpp_mgmt_voq_is_ocb_eligible_get */
    dpp_mult_cud_to_port_map_set,
    dpp_mult_cud_to_port_map_get,
    soc_arad_nof_interrupts,
    arad_mgmt_nof_block_instances,
    arad_mgmt_temp_pvt_get,
    arad_mgmt_avs_value_get,
    arad_itm_dp_discard_set,
    arad_itm_dp_discard_get,
    arad_plus_itm_alpha_set,
    arad_plus_itm_alpha_get,
    arad_plus_itm_fair_adaptive_tail_drop_enable_set,
    arad_plus_itm_fair_adaptive_tail_drop_enable_get,
    arad_ports_application_mapping_info_set,
    arad_ports_application_mapping_info_get,
    arad_ofp_rates_max_credit_empty_port_set,
    arad_ofp_rates_max_credit_empty_port_get,
    soc_arad_allocate_rcy_port,
    soc_arad_free_tm_port_and_recycle_channel,
    soc_arad_info_config_device_ports,
    soc_arad_is_olp,
    soc_arad_is_oamp,
    soc_arad_validate_fabric_mode,
    soc_arad_prop_fap_device_mode_get,
    soc_arad_deinit,
    soc_arad_attach,
    soc_arad_fc_oob_mode_validate,
    NULL, /* mbcm_dpp_port_to_interface_egress_map_set */
    arad_ofp_rates_port_priority_max_burst_for_fc_queues_set,          
    arad_ofp_rates_port_priority_max_burst_for_empty_queues_set,       
    arad_ofp_rates_port_priority_max_burst_for_fc_queues_get,          
    arad_ofp_rates_port_priority_max_burst_for_empty_queues_get,       
    arad_ofp_rates_sch_single_port_rate_sw_set,                     
    arad_ofp_rates_sch_single_port_rate_hw_set,                     
    arad_ofp_rates_egq_single_port_rate_sw_set,
    arad_ofp_rates_egq_single_port_rate_sw_get_unsafe,
    arad_ofp_rates_egq_single_port_rate_hw_set,                     
    arad_ofp_rates_sch_single_port_rate_hw_get,                           
    arad_ofp_rates_egq_single_port_rate_hw_get,                           
    arad_ofp_rates_single_port_max_burst_set,                          
    arad_ofp_rates_single_port_max_burst_get,                                                  
    arad_ofp_rates_egq_interface_shaper_set,                                                  
    arad_ofp_rates_egq_interface_shaper_get,                           
    arad_ofp_rates_egq_tcg_rate_sw_set,                             
    arad_ofp_rates_egq_tcg_rate_hw_set,                             
    arad_ofp_rates_sch_tcg_rate_set,                                   
    arad_ofp_rates_egq_tcg_rate_hw_get,                                   
    arad_ofp_rates_sch_tcg_rate_get,                                   
    arad_ofp_rates_egq_tcg_max_burst_set,                              
    arad_ofp_rates_sch_tcg_max_burst_set,                              
    arad_ofp_rates_egq_tcg_max_burst_get,                              
    arad_ofp_rates_sch_tcg_max_burst_get,                              
    arad_ofp_rates_egq_port_priority_rate_sw_set,                   
    arad_ofp_rates_egq_port_priority_rate_hw_set,                   
    arad_ofp_rates_sch_port_priority_rate_set,                         
    arad_ofp_rates_sch_port_priority_rate_sw_set,
    arad_ofp_rates_sch_port_priority_hw_set,
    arad_ofp_rates_egq_port_priority_rate_hw_get,                         
    arad_ofp_rates_sch_port_priority_rate_get,                         
    arad_ofp_rates_egq_port_priority_max_burst_set,                    
    arad_ofp_rates_sch_port_priority_max_burst_set,                    
    arad_ofp_rates_egq_port_priority_max_burst_get,                    
    arad_ofp_rates_sch_port_priority_max_burst_get,
    arad_ofp_rates_port2chan_cal_get,
    arad_ofp_rates_retrieve_egress_shaper_reg_field_names,
    arad_ofp_rates_egress_shaper_reg_field_read,
    arad_ofp_rates_egress_shaper_reg_field_write,
    arad_egq_scm_chan_arb_id2scm_id,
    NULL, /*mbcm_dpp_ofp_rates_interface_internal_rate_get*/
    arad_ofp_rates_packet_mode_packet_size_get,
    arad_ofp_rates_egress_shaper_cal_write,
    arad_ofp_rates_egress_shaper_cal_read,
    arad_port2egress_offset,
    arad_ofp_rates_is_channalized,
    arad_sch_cal_tbl_set,
    arad_sch_cal_tbl_get,
    arad_sch_cal_max_size_get,
    soc_port_sw_db_local_to_tm_port_get,
    soc_port_sw_db_local_to_pp_port_get,
    soc_port_sw_db_tm_to_local_port_get,
    soc_port_sw_db_pp_to_local_port_get,
    arad_port_init,
    arad_port_post_init,
    arad_port_deinit,
    arad_port_enable_set,
    arad_port_enable_get,
    arad_port_speed_set,
    arad_port_speed_get,
    arad_port_interface_set,
    arad_port_interface_get,
    arad_port_link_state_get,
    arad_port_is_supported_encap_get,
    arad_egr_q_nif_cal_set_unsafe,
    soc_common_drv_fast_port_set,
    arad_parser_nof_bytes_to_remove_set,
    arad_ps_db_find_free_binding_ps,
    arad_ps_db_release_binding_ps,
    arad_ps_db_alloc_binding_ps_with_id,
    arad_egr_prog_editor_profile_set,
    NULL,/*mbcm_dpp_egr_prog_editor_prog_sel_value1_set,*/
    arad_fc_pfc_mapping_set,
    arad_fc_pfc_mapping_get,
    arad_port_probe,
    soc_pm_port_deinit,
    NULL, /* mbcm_dpp_port_fabric_detach_f */
    NULL, /* mbcm_dpp_port_is_pcs_loopback_f */
    NULL, /* mbcm_dpp_qsgmii_offsets_add_f */
    NULL, /* mbcm_dpp_qsgmii_offsets_remove_f */
    NULL, /* mbcm_dpp_qsgmii_offsets_add_pbmp_f */
    NULL, /* mbcm_dpp_qsgmii_offsets_remove_pbmp_f */
    NULL, /* mbcm_dpp_port_sch_config_f */
    NULL, /* mbcm_dpp_port_open_fab_o_nif_path */
    NULL, /* mbcm_dpp_port_open_ilkn_path */
    NULL, /* mbcm_dpp_nif_sif_set*/    
    NULL, /* mbcm_dpp_port_close_ilkn_path */
    NULL, /* mbcm_dpp_port_ilkn_init */
    NULL, /* mbcm_dpp_port_nif_ilkn_pbmp_get */
    NULL, /* mbcm_dpp_port_nif_ilkn_phys_aligned_pbmp_get */
    NULL, /* mbcm_dpp_port_nif_qsgmii_pbmp_get */
    NULL, /* mbcm_dpp_port_ilkn_nif_port_get */
    NULL, /* mbcm_dpp_port_ilkn_nof_segments_set */
    NULL, /* mbcm_port_ilkn_bypass_interface_enable */
    NULL, /* mbcm_port_fabric_o_nif_bypass_interface_enable */
    NULL, /* mbcm_dpp_port_prd_enable_set_f */
    NULL, /* mbcm_dpp_port_prd_enable_get_f */
    NULL, /* mbcm_dpp_port_prd_config_set_f */
    NULL, /* mbcm_dpp_port_prd_config_get_f */
    NULL, /* mbcm_dpp_port_prd_threshold_set_f */
    NULL, /* mbcm_dpp_port_prd_threshold_get_f */
    NULL, /* mbcm_dpp_port_prd_map_set_f */
    NULL, /* mbcm_dpp_port_prd_map_get_f */
    NULL, /* mbcm_dpp_port_prd_drop_count_get_f */
    NULL, /* mbcm_dpp_port_prd_tpid_set_f */
    NULL, /* mbcm_dpp_port_prd_tpid_get_f */
    soc_arad_port_speed_sku_restrictions,
    NULL, /* mbcm_dpp_port_prd_ignore_ip_dscp_set */
    NULL, /* mbcm_dpp_port_prd_ignore_ip_dscp_get */
    NULL, /* mbcm_dpp_port_prd_ignore_mpls_exp_set */
    NULL, /* mbcm_dpp_port_prd_ignore_mpls_exp_get */
    NULL, /* mbcm_dpp_port_prd_ignore_inner_tag_set */
    NULL, /* mbcm_dpp_port_prd_ignore_inner_tag_get */
    NULL, /* mbcm_dpp_port_prd_ignore_outer_tag_set */
    NULL, /* mbcm_dpp_port_prd_ignore_outer_tag_get */
    NULL, /* mbcm_dpp_port_prd_default_priority_set */
    NULL, /* mbcm_dpp_port_prd_default_priority_get */
    NULL, /* mbcm_dpp_port_prd_custom_ether_type_set */
    NULL, /* mbcm_dpp_port_prd_custom_ether_type_get */
    NULL, /* mbcm_dpp_port_prd_control_frame_set */
    NULL, /* mbcm_dpp_port_prd_control_frame_get */
    NULL, /* mbcm_dpp_port_prd_flex_key_construct_set */
    NULL, /* mbcm_dpp_port_prd_flex_key_construct_get */
    NULL, /* mbcm_dpp_port_prd_flex_key_entry_set */
    NULL, /* mbcm_dpp_port_prd_flex_key_entry_get */
    NULL, /* mbcm_dpp_port_prd_restore_hw_defaults */
    arad_port_control_pcs_set,
    arad_port_control_pcs_get,
    arad_link_power_set,
    arad_link_power_get,
    arad_link_control_rx_enable_set,
    arad_link_control_tx_enable_set,
    soc_arad_port_control_tx_nif_enable_set,
    arad_link_control_rx_enable_get,
    arad_link_control_tx_enable_get,
    soc_arad_port_control_tx_nif_enable_get,
    arad_link_control_strip_crc_set,
    arad_link_control_strip_crc_get,
    arad_port_prbs_tx_enable_set,    
    arad_port_prbs_tx_enable_get,       
    arad_port_prbs_rx_enable_set,      
    arad_port_prbs_rx_enable_get,      
    arad_port_prbs_rx_status_get,      
    arad_fabric_prbs_polynomial_set,      
    arad_fabric_prbs_polynomial_get,    
    arad_port_prbs_tx_invert_data_set, 
    arad_port_prbs_tx_invert_data_get,
    soc_pm_pfc_refresh_set,
    soc_pm_pfc_refresh_get,
    soc_pm_local_fault_clear,
    soc_pm_remote_fault_clear,
    soc_pm_pad_size_set,
    soc_pm_pad_size_get,
    arad_port_phy_reset,
    arad_port_phy_control_set,
    arad_port_phy_control_get,
    arad_port_phy_reg_get,
    arad_port_phy_reg_set,
    arad_port_phy_reg_modify,
    soc_pm_mac_sa_set,
    soc_pm_mac_sa_get,
    soc_pm_eee_enable_get,
    arad_port_eee_enable_set,
    soc_pm_eee_tx_idle_time_get,
    soc_pm_eee_tx_idle_time_set,
    soc_pm_eee_tx_wake_time_get,
    soc_pm_eee_tx_wake_time_set,
    soc_pm_eee_link_active_duration_get,
    soc_pm_eee_link_active_duration_set,
    arad_port_eee_statistics_clear,
    arad_port_eee_event_count_symmetric_set,
    arad_port_eee_tx_event_count_get,
    arad_port_eee_tx_duration_get,
    arad_port_eee_rx_event_count_get,
    arad_port_eee_rx_duration_get,
    arad_port_eee_event_count_symmetric_get,
    arad_fc_enables_set,
    arad_fc_enables_get,
    arad_fc_ilkn_mub_channel_set,
    arad_fc_ilkn_mub_channel_get,
    arad_fc_ilkn_mub_gen_cal_set,
    arad_fc_ilkn_mub_gen_cal_get,
    arad_fc_cat_2_tc_hcfc_bitmap_set,
    arad_fc_cat_2_tc_hcfc_bitmap_get,
    arad_fc_glb_hcfc_bitmap_set,
    arad_fc_glb_hcfc_bitmap_get,
    arad_fc_inbnd_mode_set,
    arad_fc_inbnd_mode_get,
    NULL, /* mbcm_fc_glb_rcs_mask_set_f */
    NULL, /* mbcm_fc_glb_rcs_mask_get_f */
    arad_fc_init_pfc_mapping,
    arad_fc_ilkn_llfc_set,
    arad_fc_ilkn_llfc_get,
    arad_nif_ipg_set_unsafe,
    arad_nif_ipg_get_unsafe,
    arad_port_link_get,
    NULL, /* mbcm_dpp_port_ilkn_bypass_interface_rx_check_and_enable_f */
    arad_port_autoneg_set,
    arad_port_autoneg_get,
    arad_port_ability_local_get,
    arad_port_ability_remote_get,
    arad_port_ability_advert_set,
    arad_port_ability_advert_get,
    arad_port_mdix_set,
    arad_port_mdix_get,
    arad_port_mdix_status_get,
    arad_port_duplex_set,
    arad_port_duplex_get,
    arad_port_nif_nof_lanes_get,
    NULL, /* mbcm_dpp_port_nif_quad_to_core_validate */
    arad_port_close_path,
    arad_fc_hcfc_watchdog_set,
    arad_fc_hcfc_watchdog_get,
	arad_port_fault_get,
    arad_link_port_fault_get,
    NULL,  /*mbcm_dpp_stat_if_queue_range_set*/
    NULL,  /*mbcm_dpp_stat_if_queue_range_get*/
    arad_ports_logical_sys_id_parse,
    arad_fc_vsq_index_group2global,
    NULL, /*mbcm_dpp_stat_oversize_set*/
    NULL, /*mbcm_dpp_stat_oversize_get*/
    arad_fc_status_info_get,
    NULL,  /*mbcm_dpp_stat_counter_filter_set*/
    NULL,  /*mbcm_dpp_stat_counter_filter_get*/	
    NULL, /* mbcm_dpp_cnt_ingress_compensation_profile_delta_set */
    NULL, /* mbcm_dpp_cnt_ingress_compensation_port_profile_set */
    NULL, /* mbcm_dpp_cnt_ingress_compensation_outLif_delta_set */
    NULL, /* mbcm_dpp_cnt_ingress_compensation_outLif_delta_get */
    NULL, /* mbcm_dpp_cnt_ingress_compensation_port_delta_and_profile_get */
    NULL, /* mbcm_dpp_stat_egress_receive_tm_pointer_format_set*/
    NULL, /* mbcm_dpp_stat_egress_receive_tm_pointer_format_get*/
    NULL, /* mbcm_dpp_cnt_filter_config_ingress_set_get */
    NULL, /* mbcm_dpp_cnt_filter_config_ingress_set_get */    
    arad_cnt_crps_iqm_cmd_get,
    _jer_cnt_counter_bmap_mem_by_src_type_get,
    _arad_cnt_do_not_count_field_by_src_type_get,
    arad_cnt_stif_ingress_pp_source_set,
    arad_fc_pfc_generic_bitmap_valid_update,
    arad_fc_pfc_generic_bitmap_used_update,
    NULL,  /*mbcm_dpp_interrupts_array_init*/
    NULL,   /*mbcm_dpp_interrupts_array_deinit*/	
    NULL,    /*mbcm_dpp_interrupt_cb_init*/	
#ifdef PORTMOD_SUPPORT
    NULL,   /*mbcm_dpp_soc_pm_instances_get_f*/
#endif
    NULL,   /* mbcm_dpp_soc_pml_table_get_f */
    NULL,   /*mbcm_dpp_port_quad_get*/
    NULL,    /*mbcm_dpp_port_ports_to_same_quad_get*/
    NULL, /* mbcm_dpp_nif_priority_set */
    NULL, /* mbcm_dpp_nif_priority_get */
    NULL, /* mbcm_dpp_nif_priority_quad_tdm_high_low_clear */
    NULL, /* mbcm_dpp_nif_priority_ilkn_tdm_clear */
    NULL, /* mbcm_dpp_nif_priority_ilkn_high_low_clear */
    soc_arad_nif_sku_restrictions,
    NULL,   /* mbcm_dpp_fabric_link_config_ovrd */
    NULL,    /* mbcm_dpp_wait_gtimer_trigger */
    NULL,   /* mbcm_dpp_phy_nif_measure */
    soc_bist_all_arad,
    soc_arad_device_reset,
    arad_fc_cmic_rx_set,
    arad_fc_cmic_rx_get,
    dpp_mult_get_entry,
    NULL, /* mbcm_dpp_port_ilkn_over_fabric_set */
    NULL,        /* mbcm_dpp_lb_ing_total_shared_buffer_config_set */
    NULL,        /* mbcm_dpp_lb_ing_total_shared_buffer_config_get */
    NULL,        /* mbcm_dpp_lb_ing_guaranteed_buffer_config_set */
    NULL,        /* mbcm_dpp_lb_ing_guaranteed_buffer_config_get */
    NULL,        /* mbcm_dpp_lb_ing_modem_to_lbg_map_set */
    NULL,        /* mbcm_dpp_lb_ing_modem_to_lbg_unmap_set */
    NULL,        /* mbcm_dpp_lb_ing_modem_to_lbg_map_get */
    NULL,        /* mbcm_dpp_lb_ing_expected_seqeunce_num_set */
    NULL,        /* mbcm_dpp_lb_ing_expected_seqeunce_num_get */
    NULL,        /* mbcm_dpp_lb_ing_glb_seqeunce_num_width_set */
    NULL,        /* mbcm_dpp_lb_ing_glb_seqeunce_num_width_get */
    NULL,        /* mbcm_dpp_lb_ing_packet_to_modem_map_set */
    NULL,        /* mbcm_dpp_lb_ing_packet_to_modem_map_get */
    NULL,        /* mbcm_dpp_lb_ing_reorder_config_set */
    NULL,        /* mbcm_dpp_lb_ing_reorder_config_get */
    NULL,        /* mbcm_dpp_lb_ing_flush_config_set */
    NULL,        /* mbcm_dpp_lb_ing_flush_config_get */
    NULL,        /* mbcm_dpp_lb_ing_status_get */
    NULL,        /* mbcm_dpp_lb_ing_modem_status_get */
    NULL,        /* mbcm_dpp_lb_ing_glb_stat_get */
    NULL,        /* mbcm_dpp_lb_ing_modem_stat_get */
    NULL,        /* mbcm_dpp_lb_egr_modem_to_port_map_set */
    NULL,        /* mbcm_dpp_lb_egr_modem_to_port_unmap_set */
    NULL,        /* mbcm_dpp_lb_egr_modem_to_port_map_get */
    NULL,        /* mbcm_dpp_lb_egr_lbg_enable_set */
    NULL,        /* mbcm_dpp_lb_egr_lbg_enable_get */
    NULL,        /* mbcm_dpp_lb_egr_modem_to_lbg_map_set */
    NULL,        /* mbcm_dpp_lb_egr_modem_to_lbg_unmap_set */
    NULL,        /* mbcm_dpp_lb_egr_modem_to_lbg_map_get */
    NULL,        /* mbcm_dpp_lb_egr_expected_seqeunce_num_set */
    NULL,        /* mbcm_dpp_lb_egr_expected_seqeunce_num_get */
    NULL,        /* mbcm_dpp_lb_egr_seqeunce_num_width_set */
    NULL,        /* mbcm_dpp_lb_egr_seqeunce_num_width_get */
    NULL,        /* mbcm_dpp_lb_egr_modem_seqeunce_num_width_set */
    NULL,        /* mbcm_dpp_lb_egr_modem_seqeunce_num_width_get */
    NULL,        /* mbcm_dpp_lb_global_packet_config_set */
    NULL,        /* mbcm_dpp_lb_global_packet_config_get */
    NULL,        /* mbcm_dpp_lb_modem_packet_config_set */
    NULL,        /* mbcm_dpp_lb_modem_packet_config_get */
    NULL,        /* mbcm_dpp_lb_tc_dp_to_packet_priority_config_set */
    NULL,        /* mbcm_dpp_lb_tc_dp_to_packet_priority_config_get */
    NULL,        /* mbcm_dpp_lb_egr_sched_set */
    NULL,        /* mbcm_dpp_lb_egr_sched_get */
    NULL,        /* mbcm_dpp_lb_egr_segment_config_set */
    NULL,        /* mbcm_dpp_lb_egr_segment_config_get */
    NULL,        /* mbcm_dpp_lb_egr_modem_shaper_set */
    NULL,        /* mbcm_dpp_lb_egr_modem_shaper_get */
    NULL,        /* mbcm_dpp_lb_egr_modem_segment_shaper_set */
    NULL,        /* mbcm_dpp_lb_egr_modem_segment_shaper_get */
    NULL,        /* mbcm_dpp_lb_egr_status_get */
    NULL,        /* mbcm_dpp_lb_egr_stat_get */
    NULL,        /* mbcm_dpp_lb_egr_modem_stat_get */
    NULL,        /* mbcm_dpp_lb_ports_on_reserve_intf_get */
    NULL,        /* mbcm_dpp_lb_config_info_get */
    arad_pp_mtr_ir_val_to_max_rev_exp_optimized_for_bucket_rate,
    NULL,        /* mbcm_dpp_fc_pfc_index_get*/
};
