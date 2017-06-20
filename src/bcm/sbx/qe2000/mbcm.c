/*
 * $Id: mbcm.c,v 1.63 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mbcm.c
 */

#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/qe2000.h>

mbcm_sbx_functions_t mbcm_qe2000_driver = {
    /* modid/nodeid functions */
    bcm_qe2000_stk_modid_set,
    bcm_qe2000_stk_modid_get,
    bcm_qe2000_stk_my_modid_set,
    bcm_qe2000_stk_my_modid_get,
    bcm_qe2000_stk_module_enable,
    bcm_qe2000_stk_module_protocol_set,
    bcm_qe2000_stk_module_protocol_get,
    NULL, /* bcm_stk_fabric_map_set */ 
    NULL, /* bcm_stk_fabric_map_get */ 

    /* fabric control functions */
    bcm_qe2000_fabric_crossbar_connection_set,
    bcm_qe2000_fabric_crossbar_connection_get,
    NULL, /* bcm_qe2000_fabric_tdm_enable_set */
    NULL, /* bcm_qe2000_fabric_tdm_enable_get */
    NULL, /* bcm_qe2000_fabric_calendar_max_get */
    NULL, /* bcm_qe2000_fabric_calendar_size_set */
    NULL, /* bcm_qe2000_fabric_calendar_size_get */
    NULL, /* bcm_qe2000_fabric_calendar_set */
    NULL, /* bcm_qe2000_fabric_calendar_get */
    NULL, /* bcm_qe2000_fabric_calendar_multi_set */
    NULL, /* bcm_qe2000_fabric_calendar_multi_get */
    NULL, /* bcm_qe2000_fabric_calendar_active */
    bcm_qe2000_fabric_crossbar_mapping_set,
    bcm_qe2000_fabric_crossbar_mapping_get,
    bcm_qe2000_fabric_crossbar_enable_set,
    bcm_qe2000_fabric_crossbar_enable_get,
    bcm_qe2000_fabric_crossbar_status_get,
    bcm_qe2000_fabric_control_set,
    bcm_qe2000_fabric_control_get,
    NULL /* bcm_qe2000_fabric_port_create; */,
    NULL /* bcm_qe2000_fabric_port_destroy; */,
    NULL /* bcm_qe2000_fabric_congestion_size_set */,
    NULL /* bcm_qe2000_fabric_congestion_size_get */,

    /* voq functions */
    bcm_qe2000_cosq_init,
    bcm_qe2000_cosq_detach,
    bcm_qe2000_cosq_add_queue,
    bcm_qe2000_cosq_delete_queue,
    bcm_qe2000_cosq_enable_queue,
    bcm_qe2000_cosq_disable_queue,
    bcm_qe2000_cosq_enable_fifo,
    bcm_qe2000_cosq_disable_fifo,
    bcm_qe2000_cosq_enable_get,
    bcm_qe2000_cosq_overlay_queue,
    bcm_qe2000_cosq_delete_overlay_queue,
    bcm_qe2000_cosq_set_ingress_params,
    bcm_qe2000_cosq_set_ingress_shaper,
    bcm_qe2000_cosq_set_template_gain,
    NULL,
    NULL, /* bcm_qe2000_cosq_set_template_pfc */
    NULL, /* bcm_qe2000_cosq_get_template_pfc */
    bcm_qe2000_cosq_gport_discard_set,
    bcm_qe2000_cosq_gport_discard_get,
    bcm_qe2000_cosq_gport_stat_enable_set,
    bcm_qe2000_cosq_gport_stat_enable_get,
    bcm_qe2000_cosq_gport_stat_set,
    bcm_qe2000_cosq_gport_stat_get,
    NULL /* bcm_qe2000_cosq_gport_statistic_config_set */,
    NULL /* bcm_qe2000_cosq_gport_statistic_config_get */,
    NULL /* bcm_qe2000_cosq_gport_statistic_set        */,
    NULL /* bcm_qe2000_cosq_gport_statistic_get        */,
    NULL /* bcm_qe2000_cosq_gport_statistic_multi_set  */,
    NULL /* bcm_qe2000_cosq_gport_statistic_multi_get  */,
    NULL /* bcm_qe2000_cosq_attach_scheduler           */,
    NULL /* bcm_qe2000_cosq_detach_scheduler           */,
    NULL /* bcm_qe2000_cosq_scheduler_attach_get       */,
    NULL /* bcm_qe2000_cosq_set_egress_scheduler_params*/,
    NULL /* bcm_qe2000_cosq_get_egress_scheduler_params*/,
    NULL /* bcm_qe2000_cosq_set_egress_shaper_params   */,
    NULL /* bcm_qe2000_cosq_get_egress_shaper_params   */,
    NULL /* bcm_qe2000_cosq_set_ingress_scheduler_params*/,
    NULL /* bcm_qe2000_cosq_get_ingress_scheduler_params*/,
    NULL /* bcm_qe2000_cosq_set_ingress_shaper_params   */,
    NULL /* bcm_qe2000_cosq_get_ingress_shaper_params   */,
    bcm_qe2000_cosq_control_set,
    bcm_qe2000_cosq_control_get,
    NULL /* bcm_qe2000_cosq_target_set */,
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    bcm_qe2000_cosq_state_get,
#endif
#endif
    bcm_qe2000_cosq_egress_size_set,
    bcm_qe2000_cosq_egress_size_get,
    NULL /* bcm_qe2000_cosq_flow_control_set */,
    NULL /* bcm_qe2000_cosq_flow_control_get */,
    NULL /* bcm_qe2000_cosq_pfc_config_set */,
    NULL /* bcm_qe2000_cosq_pfc_config_get */,
    NULL /* bcm_qe2000_cosq_port_congestion_set */,
    NULL /* bcm_qe2000_cosq_port_congestion_get */,
    bcm_qe2000_cosq_gport_sched_config_set,
    bcm_qe2000_cosq_gport_sched_config_get,
    NULL /* bcm_qe2000_cosq_scheduler_allocate */,
    NULL /* bcm_qe2000_cosq_scheduler_free */,
    NULL /* bcm_qe2000_cosq_gport_queue_attach */,
    NULL /* bcm_qe2000_cosq_gport_queue_attach_get */,
    NULL /* bcm_qe2000_cosq_gport_queue_detach */,
    NULL /* bcm_qe2000_cosq_mapping_set */,
    NULL /* bcm_qe2000_cosq_mapping_get */,
    NULL /* bcm_qe2000_cosq_multipath_allocate */,
    NULL /* bcm_qe2000_cosq_multipath_free */,
    NULL /* bcm_qe2000_cosq_multipath_add */,
    NULL /* bcm_qe2000_cosq_multipath_delete */,
    NULL /* bcm_qe2000_cosq_multipath_get */,
  
    /* multicast functions */
    NULL /* bcm_qe2000_fabric_distribution_create */,
    NULL /* bcm_qe2000_fabric_distribution_destroy */,
    NULL /* bcm_qe2000_fabric_distribution_set */,
    NULL /* bcm_qe2000_fabric_distribution_get */,
    NULL /* bcm_qe2000_fabric_distribution_control_set */,
    NULL /* bcm_qe2000_fabric_distribution_control_get */,
    bcm_qe2000_fabric_packet_adjust_set,
    bcm_qe2000_fabric_packet_adjust_get,
    bcm_qe2000_vlan_control_vlan_set,
    bcm_qe2000_vlan_init,
    bcm_qe2000_vlan_create,
    bcm_qe2000_vlan_port_add,
    bcm_qe2000_vlan_port_remove,
    bcm_qe2000_vlan_destroy,
    bcm_qe2000_vlan_destroy_all,
    bcm_qe2000_vlan_port_get,
    bcm_qe2000_vlan_list,
    bcm_qe2000_vlan_list_by_pbmp,
    bcm_qe2000_vlan_list_destroy,
    bcm_qe2000_vlan_default_set,
    bcm_qe2000_vlan_default_get,
    NULL,/* _multicast_init */
    NULL,/* _multicast_detach */
    bcm_qe2000_multicast_create,
    bcm_qe2000_multicast_destroy,
    bcm_qe2000_multicast_group_get,
    NULL,/* _multicast_group_traverse */
    bcm_qe2000_multicast_egress_add,
    bcm_qe2000_multicast_egress_delete,
    NULL,/* _multicast_egress_subscriber_add */
    NULL,/* _multicast_egress_subscriber_delete */
    bcm_qe2000_multicast_egress_delete_all,
    bcm_qe2000_multicast_egress_set,
    bcm_qe2000_multicast_egress_get,
    NULL,/* _multicast_egress_subscriber_set */
    NULL,/* _multicast_egress_subscriber_get */
    NULL,/* _multicast_fabric_distribution_set */
    NULL,/* _multicast_fabric_distribution_get */

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    bcm_qe2000_multicast_state_get,
#endif
#endif

    /* port functions */
    bcm_qe2000_port_init,
    bcm_qe2000_port_enable_set,
    bcm_qe2000_port_enable_get,
    bcm_qe2000_port_speed_set,
    bcm_qe2000_port_speed_get,
    bcm_qe2000_port_frame_max_set,
    bcm_qe2000_port_frame_max_get,
    bcm_qe2000_port_link_status_get,
    bcm_qe2000_port_loopback_set,
    bcm_qe2000_port_loopback_get,
    bcm_qe2000_port_control_set,
    bcm_qe2000_port_control_get,
    NULL /* bcm_qe2000_port_linkscan_set; */,
    NULL /* bcm_qe2000_port_linkscan_get; */,
    bcm_qe2000_port_rate_egress_shaper_set,
    bcm_qe2000_port_rate_egress_shaper_get,
    bcm_qe2000_port_rate_egress_traffic_set,
    bcm_qe2000_port_rate_egress_traffic_get,
    NULL /* bcm_qe2000_port_probe; */,
    bcm_qe2000_port_ability_get,
    NULL /* bcm_qe2000_port_congestion_config_set; */,
    NULL /* bcm_qe2000_port_congestion_config_get; */,
    NULL /* bcm_qe2000_port_scheduler_get; */,
    NULL /* bcm_qe2000_port_is_egress_multicast */,
    NULL /* bcm_qe2000_port_egress_multicast_scheduler_get */,
    NULL /* bcm_qe2000_port_egress_multicast_group_get */,

    bcm_qe2000_trunk_init,
    bcm_qe2000_trunk_create,
    NULL /* bcm_*_trunk_create (new) */,
    bcm_qe2000_trunk_create_id,
    bcm_qe2000_trunk_destroy,
    bcm_qe2000_trunk_detach,
    bcm_qe2000_trunk_find,
    bcm_qe2000_trunk_get,
    NULL /* bcm_*_trunk_get (new) */,
    bcm_qe2000_trunk_chip_info_get,
    bcm_qe2000_trunk_set,
    NULL /* bcm_*_trunk_set (new) */,

    /* stat functions */
    bcm_qe2000_stat_init,
    bcm_qe2000_stat_sync,
    bcm_qe2000_stat_get,
    bcm_qe2000_stat_get32,
    NULL, /* bcm_qe2000_stat_multi_get */
    NULL, /* bcm_qe2000_stat_multi_get32 */
    bcm_qe2000_stat_clear,
    bcm_qe2000_stat_scoreboard_get,
    bcm_qe2000_stat_custom_set,
    bcm_qe2000_stat_custom_get,
    bcm_qe2000_stat_custom_add,
    bcm_qe2000_stat_custom_delete,
    bcm_qe2000_stat_custom_delete_all,
    bcm_qe2000_stat_custom_check,

    /* switch functions */

    bcm_qe2000_switch_control_set,
    bcm_qe2000_switch_control_get,
    bcm_qe2000_switch_event_register,
    bcm_qe2000_switch_event_unregister,

    /* subscriber map functions */

    bcm_qe2000_cosq_subscriber_map_add,
    bcm_qe2000_cosq_subscriber_map_delete,
    bcm_qe2000_cosq_subscriber_map_delete_all,
    bcm_qe2000_cosq_subscriber_map_get,
    bcm_qe2000_cosq_subscriber_traverse,

    /* failover functions */
    bcm_qe2000_failover_enable,
    bcm_qe2000_failover_set,
    NULL, /* bcm_qe2000_failover_destroy */

    /* frame steering functions */
    NULL /* bcm_qe2000_stk_steering_unicast_set */,
    NULL /* bcm_qe2000_stk_steering_multicast_set */,
    NULL /* bcm_qe2000_stk_steering_clear */,
    NULL /* bcm_qe2000_stk_steering_clear_all */,

    /* predicate control */
    NULL /* _fabric_predicate_create */,
    NULL /* _fabric_predicate_destroy */,
    NULL /* _fabric_predicate_destroy_all */,
    NULL /* _fabric_predicate_get */,
    NULL /* _fabric_predicate_traverse */,

    /* parser (action) control */
    NULL /* _fabric_action_create */,
    NULL /* _fabric_action_destroy */,
    NULL /* _fabric_action_destroy_all */,
    NULL /* _fabric_action_get */,
    NULL /* _fabric_action_traverse */,

    /* type resolution (predicate_action) control */
    NULL /* _fabric_predicate_action_create */,
    NULL /* _fabric_predicate_action_get */,
    NULL /* _fabric_predicate_action_destroy */,
    NULL /* _fabric_predicate_action_destroy_all */,
    NULL /* _fabric_predicate_action_traverse */,

    /* QUEUE_MAP (qsel) control */
    NULL /* _fabric_qsel_create */,
    NULL /* _fabric_qsel_destroy */,
    NULL /* _fabric_qsel_destroy_all */,
    NULL /* _fabric_qsel_get */,
    NULL /* _fabric_qsel_traverse */,
    NULL /* _fabric_qsel_entry_set */,
    NULL /* _fabric_qsel_entry_get */,
    NULL /* _fabric_qsel_entry_multi_set */,
    NULL /* _fabric_qsel_entry_multi_get */,
    NULL /* _fabric_qsel_entry_traverse */,

    /* COS_MAP (qsel_offset) control */
    NULL /* _fabric_qsel_offset_create */,
    NULL /* _fabric_qsel_offset_destroy */,
    NULL /* _fabric_qsel_offset_destroy_all */,
    NULL /* _fabric_qsel_offset_traverse */,
    NULL /* _fabric_qsel_offset_entry_set */,
    NULL /* _fabric_qsel_offset_entry_get */,
    NULL /* _fabric_qsel_offset_entry_traverse */,

    /* Flow Control */
    NULL /* _fd_fct_get */,
};
