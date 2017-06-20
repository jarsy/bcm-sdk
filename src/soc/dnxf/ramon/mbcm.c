/*
 * $Id: mbcm.c,v 1.1.2.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        ramon_mbcm.c
 * Purpose:     Init the bcm multiplexing function table for RAMON
 *
 */


#include <soc/dnxf/cmn/mbcm.h>

#ifdef BCM_88790_A0
#include <soc/dnxc/legacy/dnxc_captured_buffer.h>
#include <soc/dnxc/legacy/dnxc_cells_buffer.h>
/* 3200 includes */
#include <soc/dnxf/ramon/ramon_defs.h>
#include <soc/dnxf/ramon/ramon_stat.h>
#include <soc/dnxf/ramon/ramon_property.h>
#include <soc/dnxf/ramon/ramon_stack.h>
#include <soc/dnxf/ramon/ramon_drv.h>
#include <soc/dnxf/ramon/ramon_diag.h>
#include <soc/dnxf/ramon/ramon_link.h>
#include <soc/dnxf/ramon/ramon_intr.h>
#include <soc/dnxf/ramon/ramon_fabric_flow_control.h>
#include <soc/dnxf/ramon/ramon_fabric_links.h>
#include <soc/dnxf/ramon/ramon_fabric_topology.h>
#include <soc/dnxf/ramon/ramon_fabric_cell.h>
#include <soc/dnxf/ramon/ramon_port.h>
#include <soc/dnxf/ramon/ramon_fabric_cell_snake_test.h>
#include <soc/dnxf/ramon/ramon_warm_boot.h>
#include <soc/dnxf/ramon/ramon_fifo_dma.h>
#include <soc/dnxf/ramon/ramon_cosq.h>
#include <soc/dnxf/ramon/ramon_multicast.h>
#include <soc/dnxf/ramon/ramon_rx.h>
#include <soc/dnxf/ramon/ramon_fabric_status.h>
#include <soc/dnxf/ramon/ramon_fabric_multicast.h>

/* portmod include */
#include <soc/portmod/portmod.h>
/* 1600 includes */
#include <soc/dnxf/fe1600/fe1600_port.h>
#include <soc/dnxf/fe1600/fe1600_fabric_topology.h>
#include <soc/dnxf/fe1600/fe1600_stat.h>
#include <soc/dnxf/fe1600/fe1600_multicast.h>
#include <soc/dnxf/fe1600/fe1600_fabric_links.h>
#include <soc/dnxf/fe1600/fe1600_fabric_multicast.h>
#include <soc/dnxf/fe1600/fe1600_drv.h>
#include <soc/dnxf/fe1600/fe1600_fabric_status.h>
#include <soc/dnxf/fe1600/fe1600_fabric_cell.h>


mbcm_dnxf_functions_t mbcm_ramon_driver = {
    soc_ramon_tbl_is_dynamic,
    soc_ramon_ser_init,
    soc_ramon_reset_device,
    soc_ramon_drv_soft_init,
    soc_ramon_drv_blocks_reset,
    soc_ramon_drv_reg_access_only_reset,
    NULL,/*mbcm_dnxf_TDM_fragment_validate*/
    soc_ramon_fe1600_sr_cell_send,
    soc_ramon_cell_filter_set,
    soc_ramon_cell_filter_clear,
    soc_ramon_cell_filter_count_get,
    soc_ramon_control_cell_filter_set,
    soc_ramon_control_cell_filter_clear,
    soc_ramon_control_cell_filter_receive,
    soc_ramon_cell_snake_test_prepare,
    soc_ramon_cell_snake_test_run,
    soc_ramon_diag_fabric_cell_snake_test_interrupts_name_get,
    soc_ramon_diag_cell_pipe_counter_get,
    soc_ramon_diag_mesh_topology_get,
    soc_ramon_drv_graceful_shutdown_set,
    soc_ramon_drv_fe13_graceful_shutdown_set,
    soc_ramon_drv_fe13_isolate_set,
    soc_ramon_drv_soc_properties_validate,
    soc_ramon_drv_mbist,/*mbcm_dnxf_drv_mbist*/
    soc_ramon_fe1600_drv_link_to_block_mapping,
    soc_ramon_drv_block_pbmp_get,
    soc_ramon_fabric_cell_get,
    soc_ramon_fabric_cell_type_get,
    soc_ramon_fabric_cell_parse_table_get,
    soc_ramon_fabric_cell_is_cell_in_two_parts,
    soc_ramon_fabric_flow_control_rci_gci_control_source_set,
    soc_ramon_fabric_flow_control_rci_gci_control_source_get,
    soc_ramon_fabric_flow_control_thresholds_flags_validate,
    soc_ramon_fabric_flow_control_rx_llfc_threshold_validate,
    soc_ramon_fabric_flow_control_rx_llfc_threshold_set,
    soc_ramon_fabric_flow_control_rx_llfc_threshold_get,
    soc_ramon_fabric_flow_control_rx_gci_threshold_validate,
    soc_ramon_fabric_flow_control_rx_gci_threshold_set,
    soc_ramon_fabric_flow_control_rx_gci_threshold_get,
    soc_ramon_fabric_flow_control_rx_rci_threshold_validate,
    soc_ramon_fabric_flow_control_rx_rci_threshold_set,
    soc_ramon_fabric_flow_control_rx_rci_threshold_get,
    NULL,/*mbcm_dnxf_fabric_flow_control_rx_drop_threshold_validate*/
    NULL,/*mbcm_dnxf_fabric_flow_control_rx_drop_threshold_set*/
    NULL,/*mbcm_dnxf_fabric_flow_control_rx_drop_threshold_get*/
    soc_ramon_fabric_flow_control_rx_full_threshold_validate,
    soc_ramon_fabric_flow_control_rx_full_threshold_set,
    soc_ramon_fabric_flow_control_rx_full_threshold_get,
    soc_ramon_fabric_flow_control_rx_fifo_size_threshold_validate,
    soc_ramon_fabric_flow_control_rx_fifo_size_threshold_set,
    soc_ramon_fabric_flow_control_rx_fifo_size_threshold_get,
    soc_ramon_fabric_flow_control_rx_multicast_low_prio_drop_threshold_validate,
    soc_ramon_fabric_flow_control_rx_multicast_low_prio_drop_threshold_set,
    soc_ramon_fabric_flow_control_rx_multicast_low_prio_drop_threshold_get,
    soc_ramon_fabric_flow_control_tx_rci_threshold_validate,
    soc_ramon_fabric_flow_control_tx_rci_threshold_set,
    soc_ramon_fabric_flow_control_tx_rci_threshold_get,
    soc_ramon_fabric_flow_control_tx_bypass_llfc_threshold_validate,
    soc_ramon_fabric_flow_control_tx_bypass_llfc_threshold_set,
    soc_ramon_fabric_flow_control_tx_bypass_llfc_threshold_get,
    soc_ramon_fabric_flow_control_tx_gci_threshold_validate,
    soc_ramon_fabric_flow_control_tx_gci_threshold_set,
    soc_ramon_fabric_flow_control_tx_gci_threshold_get,
    soc_ramon_fabric_flow_control_tx_drop_threshold_validate,
    soc_ramon_fabric_flow_control_tx_drop_threshold_set,
    soc_ramon_fabric_flow_control_tx_drop_threshold_get,
    soc_ramon_fabric_flow_control_tx_rci_threshold_validate,
    soc_ramon_fabric_flow_control_tx_rci_threshold_set,
    soc_ramon_fabric_flow_control_tx_rci_threshold_get,
    soc_ramon_fabric_flow_control_tx_almost_full_threshold_validate,
    soc_ramon_fabric_flow_control_tx_almost_full_threshold_set,
    soc_ramon_fabric_flow_control_tx_almost_full_threshold_get,
    soc_ramon_fabric_flow_control_tx_fifo_size_threshold_validate,
    soc_ramon_fabric_flow_control_tx_fifo_size_threshold_set,
    soc_ramon_fabric_flow_control_tx_fifo_size_threshold_get,
    soc_ramon_fabric_flow_control_mid_gci_threshold_validate,
    soc_ramon_fabric_flow_control_mid_gci_threshold_set,
    soc_ramon_fabric_flow_control_mid_gci_threshold_get,
    soc_ramon_fabric_flow_control_mid_rci_threshold_validate,
    soc_ramon_fabric_flow_control_mid_rci_threshold_set,
    soc_ramon_fabric_flow_control_mid_rci_threshold_get,
    soc_ramon_fabric_flow_control_mid_prio_drop_threshold_validate,
    soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set,
    soc_ramon_fabric_flow_control_mid_prio_drop_threshold_get,
    soc_ramon_fabric_flow_control_mid_almost_full_threshold_validate,
    soc_ramon_fabric_flow_control_mid_almost_full_threshold_set,
    soc_ramon_fabric_flow_control_mid_almost_full_threshold_get,
    soc_ramon_fabric_flow_control_mid_fifo_size_threshold_validate,
    soc_ramon_fabric_flow_control_mid_fifo_size_threshold_set,
    soc_ramon_fabric_flow_control_mid_fifo_size_threshold_get,
    soc_ramon_fabric_flow_control_mid_full_threshold_validate,
    soc_ramon_fabric_flow_control_mid_full_threshold_set,
    soc_ramon_fabric_flow_control_mid_full_threshold_get,
    soc_ramon_fabric_links_link_type_set,
    soc_ramon_fabric_links_link_type_get,
    NULL,/*mbcm_dnxf_fabric_links_nof_links_get*/
    soc_ramon_fe1600_fabric_links_validate_link,
    soc_ramon_fe1600_fabric_links_isolate_set,
    soc_ramon_fe1600_fabric_links_bmp_isolate_set,
    soc_ramon_fe1600_fabric_links_isolate_get,
    soc_ramon_fabric_links_cell_format_verify,
    NULL, /*mbcm_dnxf_fabric_links_cell_format_set*/
    soc_ramon_fabric_links_cell_format_get,
    soc_ramon_fabric_links_flow_status_control_cell_format_set,
    soc_ramon_fabric_links_flow_status_control_cell_format_get,
    NULL, /*mbcm_dnxf_fabric_links_cell_interleaving_set*/
    NULL, /*mbcm_dnxf_fabric_links_cell_interleaving_get*/
    soc_ramon_fabric_links_weight_validate,
    soc_ramon_fabric_links_weight_set,
    soc_ramon_fabric_links_weight_get,
    soc_ramon_fabric_links_secondary_only_set,
    soc_ramon_fabric_links_secondary_only_get,
    soc_ramon_fe1600_fabric_links_llf_control_source_set,
    soc_ramon_fe1600_fabric_links_llf_control_source_get,
    soc_ramon_fe1600_fabric_links_aldwp_config,
    soc_ramon_fe1600_fabric_links_aldwp_init,
    soc_ramon_fabric_links_pcp_enable_set,
    soc_ramon_fabric_links_pcp_enable_get,
    soc_ramon_fe1600_fabric_multicast_low_prio_drop_select_priority_set,
    soc_ramon_fe1600_fabric_multicast_low_prio_drop_select_priority_get,
    soc_ramon_fe1600_fabric_multicast_low_prio_threshold_validate,
    soc_ramon_fe1600_fabric_multicast_low_prio_threshold_set,
    soc_ramon_fe1600_fabric_multicast_low_prio_threshold_get,
    soc_ramon_fe1600_fabric_multicast_low_priority_drop_enable_set,
    soc_ramon_fe1600_fabric_multicast_low_priority_drop_enable_get,
    soc_ramon_fe1600_fabric_multicast_priority_range_validate,
    soc_ramon_fe1600_fabric_multicast_priority_range_set,
    soc_ramon_fe1600_fabric_multicast_priority_range_get,
    soc_ramon_fe1600_fabric_multicast_multi_set,
    soc_ramon_fe1600_fabric_multicast_multi_get,
    soc_ramon_fabric_multicast_multi_write_range,
    soc_ramon_fabric_multicast_multi_read_info_get,
    soc_ramon_fabric_link_repeater_enable_set,
    soc_ramon_fabric_link_repeater_enable_get,
    soc_ramon_fe1600_fabric_link_status_all_get,
    soc_ramon_fabric_link_status_get,
    soc_ramon_fe1600_fabric_reachability_status_get,
    soc_ramon_fe1600_fabric_link_connectivity_status_get,
    soc_ramon_fabric_links_pipe_map_set,
    soc_ramon_fabric_links_pipe_map_get,
    soc_ramon_fabric_links_repeater_nof_remote_pipe_set,
    soc_ramon_fabric_links_repeater_nof_remote_pipe_get,
    soc_ramon_fabric_topology_isolate_set,
    soc_ramon_fe1600_fabric_topology_isolate_get,
    soc_ramon_fe1600_fabric_topology_rmgr_set,
    soc_ramon_fe1600_fabric_link_topology_set,
    soc_ramon_fe1600_fabric_link_topology_get,
    soc_ramon_fabric_topology_min_nof_links_set,
    soc_ramon_fabric_topology_min_nof_links_get,
    soc_ramon_fabric_topology_nof_links_to_min_nof_links_default,
    soc_ramon_fe1600_fabric_topology_repeater_destination_set,
    soc_ramon_fe1600_fabric_topology_repeater_destination_get,
    soc_ramon_fabric_topology_reachability_mask_set,
    soc_ramon_fifo_dma_channel_init,
    soc_ramon_fifo_dma_channel_deinit,
    soc_ramon_fifo_dma_channel_clear,
    soc_ramon_fifo_dma_channel_read_entries,
    soc_ramon_fifo_dma_fabric_cell_validate,
    soc_ramon_nof_interrupts,
    soc_ramon_nof_block_instances,
    soc_ramon_fe1600_multicast_egress_add,
    soc_ramon_fe1600_multicast_egress_delete,
    soc_ramon_fe1600_multicast_egress_delete_all,
    soc_ramon_fe1600_multicast_egress_get,
    soc_ramon_fe1600_multicast_egress_set,
    soc_ramon_multicast_mode_get,
    soc_ramon_multicast_table_size_get,
    soc_ramon_multicast_table_entry_size_get,
    soc_ramon_port_soc_init,
    soc_ramon_port_init,
    soc_ramon_port_deinit,
    soc_ramon_port_detach,
    soc_ramon_port_probe,
    soc_dnxc_port_control_pcs_set,
    soc_dnxc_port_control_pcs_get,
    soc_dnxc_port_control_power_set,
    soc_dnxc_port_control_power_get,
    NULL,/*mbcm_dnxf_port_control_strip_crc_set*/
    NULL,/*mbcm_dnxf_port_control_strip_crc_get*/
    NULL,/*mbcm_dnxf_port_control_rx_enable_set*/
    NULL,/*mbcm_dnxf_port_control_tx_enable_set*/
    NULL,/*mbcm_dnxf_port_control_rx_enable_get,*/
    NULL,/*mbcm_dnxf_port_control_tx_enable_get*/
    soc_dnxc_port_control_low_latency_llfc_set,
    soc_dnxc_port_control_low_latency_llfc_get,
    soc_dnxc_port_control_fec_error_detect_set,
    soc_dnxc_port_control_fec_error_detect_get,
    soc_ramon_port_phy_enable_set,
    soc_ramon_port_phy_enable_get,
    soc_dnxc_port_cl72_set,
    soc_dnxc_port_cl72_get,
    soc_dnxc_port_phy_control_set,
    soc_dnxc_port_phy_control_get,
    soc_dnxc_port_loopback_set,
    soc_dnxc_port_loopback_get,
    soc_ramon_fe1600_port_fault_get,
    soc_ramon_port_speed_get,
    soc_ramon_port_speed_max,
    soc_ramon_port_speed_set,
    soc_ramon_port_interface_set,
    soc_ramon_port_interface_get,
    soc_ramon_port_serdes_power_disable,
    soc_ramon_port_link_status_get,
    soc_ramon_fe1600_port_bucket_fill_rate_validate,
    soc_dnxc_port_prbs_tx_enable_set,
    soc_dnxc_port_prbs_tx_enable_get,
    soc_dnxc_port_prbs_rx_enable_set,
    soc_dnxc_port_prbs_rx_enable_get,
    soc_dnxc_port_prbs_rx_status_get,
    soc_dnxc_port_prbs_polynomial_set,
    soc_dnxc_port_prbs_polynomial_get,
    soc_dnxc_port_prbs_tx_invert_data_set,
    soc_dnxc_port_prbs_tx_invert_data_get,
    soc_ramon_port_pump_enable_set,
    soc_ramon_fe1600_port_rate_egress_ppt_set,
    soc_ramon_fe1600_port_rate_egress_ppt_get,
    soc_ramon_port_burst_control_set,
    soc_dnxc_port_extract_cig_from_llfc_enable_set,
    soc_dnxc_port_extract_cig_from_llfc_enable_get,
    soc_dnxc_port_phy_reg_get,
    soc_dnxc_port_phy_reg_set,
    soc_dnxc_port_phy_reg_modify,
    soc_ramon_fe1600_port_dynamic_port_update,
    soc_dnxc_port_enable_set,
    soc_dnxc_port_enable_get,
    soc_ramon_port_sync_e_link_set,
    soc_ramon_port_sync_e_divider_set,
    soc_ramon_port_sync_e_link_get,
    soc_ramon_port_sync_e_divider_get,
    soc_ramon_fe1600_set_mesh_topology_config,
    soc_ramon_stk_modid_set,
    soc_ramon_stk_modid_get,
    soc_ramon_stk_module_max_all_reachable_verify,
    soc_ramon_stk_module_max_all_reachable_set,
    soc_ramon_stk_module_max_all_reachable_get,
    soc_ramon_stk_module_max_fap_verify,
    soc_ramon_stk_module_max_fap_set,
    soc_ramon_stk_module_max_fap_get,
    soc_ramon_stk_module_all_reachable_ignore_id_set,
    soc_ramon_stk_module_all_reachable_ignore_id_get,
    soc_ramon_stk_valid_module_id_verify,
    soc_ramon_stat_init,
    soc_ramon_fe1600_stat_is_supported_type,
    soc_ramon_stat_counter_length_get,
    soc_ramon_fe1600_stat_get,
    soc_ramon_mapping_stat_get,
    soc_ramon_counters_get_info,
    soc_ramon_queues_get_info,
    soc_ramon_fe1600_fabric_link_device_mode_get,
    soc_ramon_controlled_counter_set,
    soc_ramon_soc_properties_array_get,
    soc_ramon_avs_value_get,
    soc_ramon_linkctrl_init,
    soc_ramon_interrupts_init,
    soc_ramon_interrupts_deinit,
    NULL,/*ramon_fe1600_interrupts_dnxf_control_data_init*/
    NULL,/*ramon_fe1600_interrupts_dnxf_control_data_deinit*/
    soc_ramon_interrupt_all_enable_set,
    soc_ramon_interrupt_all_enable_get,
    soc_ramon_drv_sw_ver_set,
    soc_ramon_drv_temperature_monitor_get,
    soc_ramon_drv_test_reg_filter,
    soc_ramon_drv_test_reg_default_val_filter,
    soc_ramon_drv_test_mem_filter,
    soc_ramon_drv_test_brdc_blk_filter,
    soc_ramon_drv_test_brdc_blk_info_get,
    soc_ramon_drv_asymmetrical_quad_get,
    soc_ramon_warm_boot_buffer_id_supported_get,
    soc_ramon_cosq_pipe_rx_weight_set,
    soc_ramon_cosq_pipe_rx_weight_get,
    soc_ramon_cosq_pipe_mid_weight_set,
    soc_ramon_cosq_pipe_mid_weight_get,
    soc_ramon_cosq_pipe_tx_weight_set,
    soc_ramon_cosq_pipe_tx_weight_get,
    soc_ramon_cosq_pipe_rx_threshold_set,
    soc_ramon_cosq_pipe_rx_threshold_get,
    soc_ramon_cosq_pipe_mid_threshold_set,
    soc_ramon_cosq_pipe_mid_threshold_get,
    soc_ramon_cosq_pipe_tx_threshold_set,
    soc_ramon_cosq_pipe_tx_threshold_get,
    soc_ramon_cosq_pipe_rx_rate_set,
    soc_ramon_cosq_pipe_rx_rate_get,
    soc_ramon_cosq_pipe_tx_rate_set,
    soc_ramon_cosq_pipe_tx_rate_get,
    soc_ramon_cosq_pipe_tx_rate_enable_set,
    soc_ramon_cosq_pipe_tx_rate_enable_get,
    soc_ramon_rx_cpu_address_modid_set,
    soc_ramon_rx_cpu_address_modid_init,
    soc_ramon_port_quad_disabled,
    soc_ramon_drv_block_valid_get
};

#endif /* BCM_88790_A0 */

