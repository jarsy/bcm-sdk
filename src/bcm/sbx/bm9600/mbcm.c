/*
 * $Id: mbcm.c,v 1.61 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mbcm.c
 */

#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/bm9600.h>

mbcm_sbx_functions_t mbcm_bm9600_driver = {
    /* modid/nodeid functions */
    bcm_bm9600_stk_modid_set,
    bcm_bm9600_stk_modid_get,
    bcm_bm9600_stk_my_modid_set,
    bcm_bm9600_stk_my_modid_get,
    bcm_bm9600_stk_module_enable,
    bcm_bm9600_stk_module_protocol_set,
    bcm_bm9600_stk_module_protocol_get,
    NULL, /* bcm_stk_fabric_map_set */
    NULL, /* bcm_stk_fabric_map_get */

    /* fabric control functions */
    bcm_bm9600_fabric_crossbar_connection_set,
    bcm_bm9600_fabric_crossbar_connection_get,
    NULL, /* bcm_bm9600_fabric_tdm_enable_set */
    NULL, /* bcm_bm9600_fabric_tdm_enable_get */
    NULL, /* bcm_bm9600_fabric_calendar_max_get */
    NULL, /* bcm_bm9600_fabric_calendar_size_set */
    NULL, /* bcm_bm9600_fabric_calendar_size_get */
    NULL, /* bcm_bm9600_fabric_calendar_set */
    NULL, /* bcm_bm9600_fabric_calendar_get */
    NULL, /* bcm_bm9600_fabric_calendar_multi_set */
    NULL, /* bcm_bm9600_fabric_calendar_multi_get */
    bcm_bm9600_fabric_calendar_active,
    bcm_bm9600_fabric_crossbar_mapping_set,
    bcm_bm9600_fabric_crossbar_mapping_get,
    bcm_bm9600_fabric_crossbar_enable_set,
    bcm_bm9600_fabric_crossbar_enable_get,
    bcm_bm9600_fabric_crossbar_status_get,
    bcm_bm9600_fabric_control_set,
    bcm_bm9600_fabric_control_get,
    NULL /* bcm_bm9600_fabric_port_create; */,
    NULL /* bcm_bm9600_fabric_port_destroy; */,
    NULL /* bcm_bm9600_fabric_congestion_size_set */,
    NULL /* bcm_bm9600_fabric_congestion_size_get */,

    /* voq functions */
    bcm_bm9600_cosq_init,
    NULL /* bcm_bm9600_cosq_detach */,
    bcm_bm9600_cosq_add_queue,
    bcm_bm9600_cosq_delete_queue,
    NULL /* bcm_bm9600_cosq_enable_queue */,
    NULL /* bcm_bm9600_cosq_disable_queue */,
    NULL /* bcm_bm9600_cosq_enable_fifo */,
    NULL /* bcm_bm9600_cosq_disable_fifo */,
    NULL /* bcm_bm9600_cosq_enable_get */,
    bcm_bm9600_cosq_overlay_queue,
    bcm_bm9600_cosq_delete_overlay_queue,
    bcm_bm9600_cosq_set_ingress_params,
    bcm_bm9600_cosq_set_ingress_shaper,
    bcm_bm9600_cosq_set_template_gain,
    bcm_bm9600_cosq_get_template_gain,
    NULL, /* bcm_bm9600_cosq_set_template_pfc */
    NULL, /* bcm_bm9600_cosq_get_template_pfc */
    bcm_bm9600_cosq_gport_discard_set,
    bcm_bm9600_cosq_gport_discard_get,
    NULL /* bcm_bm9600_cosq_gport_stat_enable_set */,
    NULL /* bcm_bm9600_cosq_gport_stat_enable_get */,
    NULL /* bcm_bm9600_cosq_gport_stat_set */,
    NULL /* bcm_bm9600_cosq_gport_stat_get */,
    NULL /* bcm_bm9600_cosq_gport_statistic_config_set */,
    NULL /* bcm_bm9600_cosq_gport_statistic_config_get */,
    NULL /* bcm_bm9600_cosq_gport_statistic_set        */,
    NULL /* bcm_bm9600_cosq_gport_statistic_get        */,
    NULL /* bcm_bm9600_cosq_gport_statistic_multi_set  */,
    NULL /* bcm_bm9600_cosq_gport_statistic_multi_get  */,
    NULL /* bcm_bm9600_cosq_attach_scheduler           */,
    NULL /* bcm_bm9600_cosq_dettach_scheduler          */,
    NULL /* bcm_bm9600_cosq_scheduler_attach_get       */,
    NULL /* bcm_bm9600_cosq_set_egress_scheduler_params*/,
    NULL /* bcm_bm9600_cosq_get_egress_scheduler_params*/,
    NULL /* bcm_bm9600_cosq_set_egress_shaper_params   */,
    NULL /* bcm_bm9600_cosq_get_egress_shaper_params   */,
    NULL /* bcm_bm9600_cosq_set_ingress_scheduler_params*/,
    NULL /* bcm_bm9600_cosq_get_ingress_scheduler_params*/,
    NULL /* bcm_bm9600_cosq_set_ingress_shaper_params   */,
    NULL /* bcm_bm9600_cosq_get_ingress_shaper_params   */,
    NULL /* bcm_bm9600_cosq_control_set */,
    NULL /* bcm_bm9600_cosq_control_get */,
    NULL /* bcm_bm9600_cosq_target_set */,
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    NULL /* bcm_bm9600_cosq_gport_state_get */,
#endif
#endif
    NULL /* bcm_bm9600_cosq_egress_size_set */,
    NULL /* bcm_bm9600_cosq_egress_size_get */,
    NULL /* bcm_bm9600_cosq_flow_control_set */,
    NULL /* bcm_bm9600_cosq_flow_control_get */,
    NULL /* bcm_bm9600_cosq_pfc_config_set */,
    NULL /* bcm_bm9600_cosq_pfc_config_get */,
    NULL /* bcm_bm9600_cosq_port_congestion_set; */,
    NULL /* bcm_bm9600_cosq_port_congestion_get; */,
    bcm_bm9600_cosq_gport_sched_config_set,
    bcm_bm9600_cosq_gport_sched_config_get,
    NULL /* bcm_bm9600_cosq_scheduler_allocate */,
    NULL /* bcm_bm9600_cosq_scheduler_free */,
    bcm_bm9600_cosq_gport_queue_attach,
    bcm_bm9600_cosq_gport_queue_attach_get,
    bcm_bm9600_cosq_gport_queue_detach,
    NULL /* bcm_bm9600_cosq_mapping_set */,
    NULL /* bcm_bm9600_cosq_mapping_get */,
    NULL /* bcm_bm9600_cosq_multipath_allocate */,
    NULL /* bcm_bm9600_cosq_multipath_free */,
    NULL /* bcm_bm9600_cosq_multipath_add */,
    NULL /* bcm_bm9600_cosq_multipath_delete */,
    NULL /* bcm_bm9600_cosq_multipath_get */,

    /* multicast functions */
    bcm_bm9600_fabric_distribution_create,
    bcm_bm9600_fabric_distribution_destroy,
    bcm_bm9600_fabric_distribution_set,
    bcm_bm9600_fabric_distribution_get,
    bcm_bm9600_fabric_distribution_control_set,
    bcm_bm9600_fabric_distribution_control_get,
    NULL,/* _fabric_packet_adjust_set */
    NULL,/* _fabric_packet_adjust_get */
    NULL,/* bcm_bm9600_vlan_control_vlan_set */
    NULL,/* _vlan_init */
    NULL,/* _vlan_create */
    NULL /* bcm_bm9600_vlan_port_add*/,
    NULL /* bcm_bm9600_vlan_port_remove*/,
    NULL /* bcm_bm9600_vlan_destroy */,
    NULL /* bcm_bm9600_vlan_destroy_all */,
    NULL /* bcm_bm9600_vlan_port_get */,
    NULL /* bcm_bm9600_vlan_list */,
    NULL /* bcm_bm9600_vlan_list_by_pbmp */,
    NULL /* bcm_bm9600_vlan_list_destroy */,
    NULL /* bcm_bm9600_vlan_default_set */,
    NULL /* bcm_bm9600_vlan_default_get */,
    NULL,/* _multicast_init */
    NULL,/* _multicast_detach */
    NULL,/* _multicast_create */
    NULL,/* _multicast_destroy */
    NULL,/* _multicast_group_get */
    NULL,/* _multicast_group_traverse */
    NULL,/* _multicast_egress_add */
    NULL,/* _multicast_egress_delete */
    NULL,/* _multicast_egress_subscriber_add */
    NULL,/* _multicast_egress_subscriber_delete */
    NULL,/* _multicast_egress_delete_all */
    NULL,/* _multicast_egress_set */
    NULL,/* _multicast_egress_get */
    NULL,/* _multicast_egress_subscriber_set */
    NULL,/* _multicast_egress_subscriber_get */
    NULL,/* _multicast_fabric_distribution_set */
    NULL,/* _multicast_fabric_distribution_get */
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    NULL,/*bcm_bm9600_multicast_state_get */
#endif
#endif

    /* port functions */
    bcm_bm9600_port_init,
    bcm_bm9600_port_enable_set,
    bcm_bm9600_port_enable_get,
    bcm_bm9600_port_speed_set,
    bcm_bm9600_port_speed_get,
    NULL /* bcm_bm9600_port_frame_max_set */,
    NULL /* bcm_bm9600_port_frame_max_get */,
    bcm_bm9600_port_link_status_get,
    bcm_bm9600_port_loopback_set,
    bcm_bm9600_port_loopback_get,
    bcm_bm9600_port_control_set,
    bcm_bm9600_port_control_get,
    NULL /* bcm_bm9600_port_linkscan_set; */,
    NULL /* bcm_bm9600_port_linkscan_get; */,
    NULL /* bcm_bm9600_port_rate_egress_shaper_set; */,
    NULL /* bcm_bm9600_port_rate_egress_shaper_get; */,
    NULL /* bcm_bm9600_port_rate_egress_traffic_set; */,
    NULL /* bcm_bm9600_port_rate_egress_traffic_get; */,
    bcm_bm9600_port_probe,
    bcm_bm9600_port_ability_get,
    NULL /* bcm_bm9600_port_congestion_config_set; */,
    NULL /* bcm_bm9600_port_congestion_config_get; */,
    NULL /* bcm_bm9600_port_scheduler_get; */,
    NULL /* bcm_bm9600_port_is_egress_multicast */,
    NULL /* bcm_bm9600_port_egress_multicast_scheduler_get */,
    NULL /* bcm_bm9600_port_egress_multicast_group_get */,


    NULL, /*bcm_bm9600_trunk_init*/
    NULL, /*bcm_bm9600_trunk_create*/
    NULL, /* bcm_*_trunk_create (new) */
    NULL, /*bcm_bm9600_trunk_create_id*/
    NULL, /*bcm_bm9600_trunk_destroy*/
    NULL, /*bcm_bm9600_trunk_detach*/
    NULL, /*bcm_bm9600_trunk_find*/
    NULL, /* bcm_*_trunk_get (old) */
    NULL, /* bcm_*_trunk_get */
    NULL, /*bcm_bm9600_trunk_chip_info_get*/
    NULL, /* bcm_*_trunk_set (old) */
    NULL, /*bcm_bm9600_trunk_set*/

    /* stat functions */
    NULL,  /*bcm_bm9600_stat_init */
    NULL,  /*bcm_bm9600_stat_sync */
    NULL,  /*bcm_bm9600_stat_get */
    NULL,  /*bcm_bm9600_stat_get32 */
    NULL,  /*bcm_bm9600_stat_multi_get */
    NULL,  /*bcm_bm9600_stat_multi_get32 */
    NULL,  /*bcm_bm9600_stat_clear */
    NULL,  /*bcm_bm9600_stat_scoreboard_get */
    NULL,  /*bcm_bm9600_stat_custom_set */
    NULL,  /*bcm_bm9600_stat_custom_get */
    NULL,  /*bcm_bm9600_stat_custom_add */
    NULL,  /*bcm_bm9600_stat_custom_delete */
    NULL,  /*bcm_bm9600_stat_custom_delete_all */
    NULL,  /*bcm_bm9600_stat_custom_check */

    /* switch control functions */
    NULL,  /*bcm_bm9600_switch_control_set */
    NULL,  /*bcm_bm9600_switch_control_get */
    NULL, /* bcm_bm9600_switch_event_register */
    NULL, /* bcm_bm9600_switch_event_unregister */

    /* subscriber map functions */

    NULL,  /* bcm_bm9600_cosq_subscriber_map_add */   
    NULL,  /* bcm_bm9600_cosq_subscriber_map_delete */
    NULL,  /* bcm_bm9600_cosq_subscriber_map_delete_all */
    NULL,  /* bcm_bm9600_cosq_subscriber_map_get */
    NULL,  /* bcm_bm9600_cosq_subscriber_traverse */

    /* failover functions */
    bcm_bm9600_failover_enable,
    bcm_bm9600_failover_set,
    bcm_bm9600_failover_destroy,

    /* frame steering functions */
    NULL /* bcm_bm9600_stk_steering_unicast_set */,
    NULL /* bcm_bm9600_stk_steering_multicast_set */,
    NULL /* bcm_bm9600_stk_steering_clear */,
    NULL /* bcm_bm9600_stk_steering_clear_all */,

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
