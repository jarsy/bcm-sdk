/*
 * $Id: mbcm.h,v 1.72 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mbcm.h
 * Purpose:     Multiplexing of the bcm layer
 *
 * Different chip families require such different implementations
 * of some basic BCM layer functionality that the functions are
 * multiplexed to allow a fast runtime decision as to which function
 * to call.  This file contains the basic declarations for this
 * process.
 *
 * See internal/design/soft_arch/xgs_plan.txt for more info.
 *
 * Conventions:
 *    MBCM is the multiplexed bcm prefix
 *    _f is the function type declaration postfix
 */

#ifndef _BCM_INT_SBX_MBCM_H_
#define _BCM_INT_SBX_MBCM_H_

#include <soc/macipadr.h>
#include <soc/drv.h>

#include <bcm/port.h>
#include <bcm/trunk.h>
#include <bcm/multicast.h>
#include <bcm/vlan.h>
#include <bcm/switch.h>
#include <bcm/fabric.h>
#include <bcm/cosq.h>
#include <bcm/stack.h>
#include <bcm/stat.h>

#include <bcm_int/sbx/cosq.h>
#include <bcm_int/common/trunk.h>

/****************************************************************
 *
 * SOC Family type:
 *
 ****************************************************************/
typedef enum soc_sbx_chip_family_e {
    BCM_FAMILY_FE2000,
    BCM_FAMILY_QE2000,
    BCM_FAMILY_BM3200,
    BCM_FAMILY_BM9600,
    BCM_FAMILY_SIRIUS
} soc_sbx_chip_family_t;

#define BCM_SBX_CHIP_FAMILY(unit)       (mbcm_sbx_family[unit])

/****************************************************************
 *
 * Type definitions for multiplexed BCM functions.
 *
 ****************************************************************/
/* modid/nodeid functions */
typedef int (*mbcm_sbx_stk_modid_set_f)(int unit,
                                        int modid);
typedef int (*mbcm_sbx_stk_modid_get_f)(int unit,
                                        int *modid);
typedef int (*mbcm_sbx_stk_my_modid_set_f)(int unit,
                                           int modid);
typedef int (*mbcm_sbx_stk_my_modid_get_f)(int unit,
                                           int *modid);
typedef int (*mbcm_sbx_module_enable_f)(int unit,
                                        int modid,
                                        int nports,
                                        int enable);
typedef int (*mbcm_sbx_module_protocol_set_f)(int unit,
                                              int node,
                                              bcm_module_protocol_t  protocol);
typedef int (*mbcm_sbx_module_protocol_get_f)(int unit,
                                              int node,
                                              bcm_module_protocol_t *protocol);
typedef int (*mbcm_sbx_stk_fabric_map_set_f)(int unit,
                                             bcm_gport_t switch_port,
                                             bcm_gport_t fabric_port);

typedef int (*mbcm_sbx_stk_fabric_map_get_f)(int unit,
                                             bcm_gport_t switch_port,
                                             bcm_gport_t *fabric_port);


/* fabric control functions */
typedef int (*mbcm_sbx_fabric_crossbar_connection_set_f)(int unit,
                                                         int xbar,
                                                         int src_modid,
                                                         bcm_port_t src_xbport,
                                                         int dst_modid,
                                                         bcm_port_t dst_xbport);
typedef int (*mbcm_sbx_fabric_crossbar_connection_get_f)(int unit,
                                                         int xbar,
                                                         int src_modid,
                                                         bcm_port_t src_xbport,
                                                         int dst_modid,
                                                         bcm_port_t *dst_xbport);
typedef int (*mbcm_sbx_fabric_tdm_enable_set_f) (int unit,
						 int enable);

typedef int (*mbcm_sbx_fabric_tdm_enable_get_f) (int unit,
						 int *enable);

typedef int (*mbcm_sbx_fabric_calendar_max_get_f) (int unit,
						   int *max_size);

typedef int (*mbcm_sbx_fabric_calendar_size_set_f)(int unit, int config_size);

typedef int (*mbcm_sbx_fabric_calendar_size_get_f)(int unit, int *config_size);

typedef int (*mbcm_sbx_fabric_calendar_set_f)(int unit,
					      int cindex,
					      int connection,
					      bcm_gport_t dest_port,
					      bcm_cos_queue_t dest_cosq);

typedef int (*mbcm_sbx_fabric_calendar_get_f)(int unit,
					      int array_size,
					      int *connection,
					      bcm_gport_t *dest_port,
					      bcm_gport_t *dest_cosq);

typedef int (*mbcm_sbx_fabric_calendar_multi_set_f) (int unit,
						     int array_size,
						     int *connection_array,
						     bcm_gport_t *dest_port_array,
						     bcm_gport_t *dest_cosq_array);

typedef int (*mbcm_sbx_fabric_calendar_multi_get_f) (int unit,
						     int array_size,
						     int *connection_array,
						     bcm_gport_t *dest_port_array,
						     bcm_gport_t *dest_cosq_array);

typedef int (*mbcm_sbx_fabric_calendar_active_f) (int unit);

typedef int (*mbcm_sbx_fabric_crossbar_mapping_set_f)(int unit,
                                                      int modid,
                                                      int switch_fabric_arbiter_id,
                                                      int xbar,
                                                      bcm_port_t port);
typedef int (*mbcm_sbx_fabric_crossbar_mapping_get_f)(int unit,
                                                      int modid,
                                                      int switch_fabric_arbiter_id,
                                                      int xbar,
                                                      bcm_port_t *port);
typedef int (*mbcm_sbx_fabric_crossbar_enable_set_f)(int unit,
                                                     uint64 xbars);
typedef int (*mbcm_sbx_fabric_crossbar_enable_get_f)(int unit,
                                                     uint64 *xbars);
typedef int (*mbcm_sbx_fabric_crossbar_status_get_f)(int unit,
                                                     uint64 *xbars);
typedef int (*mbcm_sbx_fabric_control_set_f)(int unit,
                                             bcm_fabric_control_t type,
                                             int arg);
typedef int (*mbcm_sbx_fabric_control_get_f)(int unit,
                                             bcm_fabric_control_t type,
                                             int *arg);
typedef int (*mbcm_sbx_fabric_port_create_f)(int unit, bcm_gport_t parent_port,
					     int offset, uint32 flags, int *subport);
typedef int (*mbcm_sbx_fabric_port_destroy_f)(int unit, bcm_gport_t child_gport);
typedef int (*mbcm_sbx_fabric_congestion_size_set_f)(int unit, bcm_module_t module_id, int max_ports);
typedef int (*mbcm_sbx_fabric_congestion_size_get_f)(int unit, bcm_module_t module_id, int *max_ports);

/* voq functions */
typedef int (*mbcm_sbx_cosq_init_f)(int unit);
typedef int (*mbcm_sbx_cosq_detach_f)(int unit);

typedef int (*mbcm_sbx_cosq_add_queue_f)(int unit,
                                         int queue,
                                         bcm_sbx_cosq_queue_region_type_t queue_region,
                                         int sysport,
                                         int eset,
                                         int dest_node,
                                         int dest_port,
                                         int dest_mc,
                                         int dest_cos,
                                         int multicast,
                                         bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                                         bcm_sbx_cosq_bw_group_params_t *p_bwparams,
                                         int inhibit_write);


typedef int (*mbcm_sbx_cosq_delete_queue_f)(int unit,
                                            int queue,
                                            bcm_sbx_cosq_queue_region_type_t queue_region);

typedef int (*mbcm_sbx_cosq_enable_queue_f)(int unit,
                                            int queue, 
					    bcm_sbx_cosq_queue_region_type_t queue_region);

typedef int (*mbcm_sbx_cosq_disable_queue_f)(int unit,
                                             int queue,
					     bcm_sbx_cosq_queue_region_type_t queue_region);

typedef int (*mbcm_sbx_cosq_enable_fifo_f)(int unit,
					   bcm_gport_t gport, 
					   bcm_cos_queue_t cosq,
					   bcm_sbx_cosq_queue_region_type_t queue_region);

typedef int (*mbcm_sbx_cosq_disable_fifo_f)(int unit,
					    bcm_gport_t gport,
					    bcm_cos_queue_t cosq,
					    bcm_sbx_cosq_queue_region_type_t queue_region);

typedef int (*mbcm_sbx_cosq_enable_get_f)(int unit,
					  bcm_gport_t gport, 
					  bcm_cos_queue_t cosq,
					  int *enable);

typedef int (*mbcm_sbx_cosq_overlay_queue_f)(int unit,
                                             int queue,
                                             bcm_sbx_cosq_queue_region_type_t queue_region,
                                             int sysport,
                                             int dest_node,
                                             int dest_port,
                                             int dest_mc,
                                             int dest_cos,
                                             int multicast);

typedef int (*mbcm_sbx_cosq_delete_overlay_queue_f)(int unit,
                                                    int queue,
                                                    int base_queue,
                                                    bcm_sbx_cosq_queue_region_type_t queue_region);

typedef int (*mbcm_sbx_cosq_set_ingress_params_f)(int unit,
                                                  int queue,
                                                  bcm_sbx_cosq_queue_region_type_t queue_region,
                                                  bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
                                                  bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
                                                  bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
                                                  bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                                  bcm_sbx_queue_size_info_t           *p_sizeInfo);

typedef int (*mbcm_sbx_cosq_set_ingress_shaper_f)(int unit,
                                                  int base_queue,
                                                  bcm_cos_queue_t cosq,
                                                  int num_cos_levels,
                                                  uint32 shape_limit_kbps,
                                                  int set_logical_port_shaper,
                                                  int enable_shaping);

typedef int (*mbcm_sbx_cosq_set_template_gain_f)(int unit,
                                                 int queue,
                                                 int template,
                                                 int gain);

typedef int (*mbcm_sbx_cosq_get_template_gain_f)(int unit,
                                                 int queue,
                                                 int *template,
                                                 int *gain);

typedef int (*mbcm_sbx_cosq_set_template_pfc_f)(int unit,
						int template,
						int pfc_en,
						int pfc_pg);

typedef int (*mbcm_sbx_cosq_get_template_pfc_f)(int unit,
						int template,
						int *pfc_en,
						int *pfc_pg);

typedef int (*mbcm_sbx_cosq_gport_discard_set_f)(int unit,
                                                 bcm_gport_t gport,  
                                                 bcm_cos_t priority, 
                                                 uint32 color,
                                                 uint32 template,
                                                 uint32 queue_size,
						 uint32 min_queue_size,
                                                 bcm_cosq_gport_discard_t *discard);
typedef int (*mbcm_sbx_cosq_gport_discard_get_f)(int unit,
                                                 bcm_gport_t gport,  
                                                 bcm_cos_t priority, 
                                                 uint32 color,
                                                 uint32 template,
                                                 uint32 queue_size,
						 uint32 min_queue_size,
                                                 bcm_cosq_gport_discard_t *discard);

typedef int (*mbcm_sbx_cosq_gport_stat_enable_set_f)(int unit,
                                                 bcm_gport_t gport,
                         int enable);

typedef int (*mbcm_sbx_cosq_gport_stat_enable_get_f)(int unit,
                                                 bcm_gport_t gport,
                         int *enable);

typedef int (*mbcm_sbx_cosq_gport_stat_set_f)(int unit,
                          bcm_gport_t gport,
                          bcm_cos_queue_t cosq,
                          bcm_cosq_gport_stats_t stat,
                          uint64 value);

typedef int (*mbcm_sbx_cosq_gport_stat_get_f)(int unit,
                          bcm_gport_t gport,
                          bcm_cos_queue_t cosq,
                          bcm_cosq_gport_stats_t stat,
                          uint64 *value);

typedef int (*mbcm_sbx_cosq_gport_stat_config_get_f)(int unit,
                          bcm_gport_t gport,
                          bcm_gport_t lgl_gport,
                          bcm_cos_queue_t cosq,
                          uint32 flags,
                          bcm_cosq_gport_stat_profile_t *profile);

typedef int (*mbcm_sbx_cosq_gport_stat_config_set_f)(int unit,
                          bcm_gport_t gport,
                          bcm_gport_t lgl_gport,
                          bcm_cos_queue_t cosq,
                          uint32 flags,
                          bcm_cosq_gport_stat_profile_t profile);

typedef int (*mbcm_sbx_cosq_gport_statistic_get_f)(int unit,
                          bcm_gport_t gport,
                          bcm_gport_t lgl_gport,
                          bcm_cos_queue_t cosq,
                          uint32 flags,
                          bcm_cosq_gport_stats_t stat,
                          uint64 *value);

typedef int (*mbcm_sbx_cosq_gport_statistic_set_f)(int unit,
                          bcm_gport_t gport,
                          bcm_gport_t lgl_gport,
                          bcm_cos_queue_t cosq,
                          uint32 flags,
                          bcm_cosq_gport_stats_t stat,
                          uint64 value);

typedef int (*mbcm_sbx_cosq_gport_statistic_multi_get_f)(int unit,
                          bcm_gport_t gport,
                          bcm_gport_t lgl_gport,
                          bcm_cos_queue_t cosq,
                          int stat_count,
			  bcm_cosq_gport_stats_t *stats_array,
                          int value_count,
			  uint64 *value_array);

typedef int (*mbcm_sbx_cosq_gport_statistic_multi_set_f)(int unit,
                          bcm_gport_t gport,
                          bcm_gport_t lgl_gport,
                          bcm_cos_queue_t cosq,
                          int stat_count,
                          bcm_cosq_gport_stats_t *stats_array,
                          uint64 value);

typedef int (*mbcm_sbx_cosq_attach_scheduler_f)(int unit,
                           bcm_gport_t parent_gport,
                           bcm_gport_t child_gport,
                           bcm_cos_queue_t cosq,
                           int egress);

typedef int (*mbcm_sbx_cosq_detach_scheduler_f)(int unit,
                           bcm_gport_t parent_gport,
                           bcm_gport_t child_gport,
			   bcm_cos_queue_t cosq,
                           int egress);

typedef int (*mbcm_sbx_cosq_scheduler_attach_get_f)(int unit,
						    bcm_gport_t in_gport,
						    int egress,
						    bcm_cos_queue_t *cosq,
						    bcm_gport_t *out_gport);

typedef int (*mbcm_sbx_cosq_set_egress_scheduler_params_f)(int unit,
							   int level,
							   int node,
							   int mode,
							   int weight);

typedef int (*mbcm_sbx_cosq_get_egress_scheduler_params_f)(int unit,
							   int level,
							   int node,
							   int *mode,
							   int *weight);

typedef int (*mbcm_sbx_cosq_set_egress_shaper_params_f)(int unit,
							int level,
							int node,
							int kbits_sec_min,
							int bits_thresh_min,
							int kbits_sec_max,
							int bits_thresh_max);

typedef int (*mbcm_sbx_cosq_get_egress_shaper_params_f)(int unit,
							int level,
							int node,
							int *kbits_sec_min,
							int *bits_thresh_min,
							int *kbits_sec_max,
							int *bits_thresh_max);

typedef int (*mbcm_sbx_cosq_set_ingress_scheduler_params_f)(int unit,
							   int level,
							   int node,
							   int mode,
							   int weight);

typedef int (*mbcm_sbx_cosq_get_ingress_scheduler_params_f)(int unit,
							   int level,
							   int node,
							   int *mode,
							   int *weight);

typedef int (*mbcm_sbx_cosq_set_ingress_shaper_params_f)(int unit,
							int level,
							int node,
							int kbits_sec_min,
							int bits_thresh_min,
							int kbits_sec_max,
							int bits_thresh_max);

typedef int (*mbcm_sbx_cosq_get_ingress_shaper_params_f)(int unit,
							int level,
							int node,
							int *kbits_sec_min,
							int *bits_thresh_min,
							int *kbits_sec_max,
							int *bits_thresh_max);

typedef int (*mbcm_sbx_cosq_control_set_f)(int unit,
					   bcm_gport_t gport,
					   bcm_cos_queue_t cosq,
					   bcm_cosq_control_t type,
					   int value,
                                           bcm_sbx_cosq_control_t *control_info);

typedef int (*mbcm_sbx_cosq_control_get_f)(int unit,
					   bcm_gport_t gport,
					   bcm_cos_queue_t cosq,
					   bcm_cosq_control_t type,
					   int *value,
                                           bcm_sbx_cosq_control_t *control_info);

typedef int (*mbcm_sbx_cosq_target_set_f)(int unit,
                                          bcm_gport_t gport,
                                          bcm_module_t module,
                                          int sysport,
                                          int port);
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
typedef int (*mbcm_sbx_cosq_state_get_f)(int unit,
                                         char *pbuf);
#endif
#endif

typedef int (*mbcm_sbx_cosq_egress_size_set_f)(int unit,
                                               bcm_gport_t gport,
                                               bcm_cos_queue_t cosq,
                                               uint32 bytes_min,
                                               uint32 bytes_max);

typedef int (*mbcm_sbx_cosq_egress_size_get_f)(int unit,
                                               bcm_gport_t gport,
                                               bcm_cos_queue_t cosq,
                                               uint32 *bytes_min,
                                               uint32 *bytes_max);

typedef int (*mbcm_sbx_cosq_flow_control_set_f)(int unit,
                                                bcm_gport_t port,
                                                bcm_cos_t int_pri,
                                                uint32 flow_control_mask,
                                                void *info);

typedef int (*mbcm_sbx_cosq_flow_control_get_f)(int unit,
                                                bcm_gport_t port,
                                                bcm_cos_t int_pri,
                                                uint32 *flow_control_mask,
                                                void *info);

typedef int (*mbcm_sbx_cosq_mapping_set_f)(int unit,
                                           bcm_cos_t priority,
                                           bcm_cos_queue_t cosq);

typedef int (*mbcm_sbx_cosq_mapping_get_f)(int unit,
                                           bcm_cos_t priority,
                                           bcm_cos_queue_t *cosq);

typedef int (*mbcm_sbx_cosq_pfc_config_set_f)(int unit, 
					      bcm_gport_t gport, 
					      bcm_cos_queue_t cosq, 
					      uint32 flags, 
					      bcm_cosq_pfc_config_t *config);

typedef int (*mbcm_sbx_cosq_pfc_config_get_f)(int unit, 
					      bcm_gport_t gport, 
					      bcm_cos_queue_t cosq, 
					      uint32 flags, 
					      bcm_cosq_pfc_config_t *config);

typedef int (*mbcm_sbx_cosq_port_congestion_set_f)(int unit, bcm_gport_t congestion_port,
						   bcm_gport_t port, uint32 flags, bcm_port_congestion_config_t config,
						   int channel_id);

typedef int (*mbcm_sbx_cosq_port_congestion_get_f)(int unit, bcm_gport_t congestion_port,
						   bcm_gport_t port, uint32 flags, int *channel_id);


/* multicast functions */
typedef int (*mbcm_sbx_fabric_distribution_create_f)(int unit,
                                                     bcm_fabric_distribution_t *ds_id);
typedef int (*mbcm_sbx_fabric_distribution_destroy_f)(int unit,
                                                      bcm_fabric_distribution_t  ds_id);
typedef int (*mbcm_sbx_fabric_distribution_set_f)(int unit,
                                                  bcm_fabric_distribution_t  ds_id,
                                                  int modid_count,
                                                  int *dist_modids,
                                                  int mc_full_eval_min);
typedef int (*mbcm_sbx_fabric_distribution_get_f)(int unit,
                                                  bcm_fabric_distribution_t  ds_id,
                                                  int max_count,
                                                  int *dist_modids,
                                                  int *count);
typedef int (*mbcm_sbx_fabric_distribution_control_set_f)(int unit,
                                                          bcm_fabric_distribution_t ds_id,
                                                          bcm_fabric_distribution_control_t type,
                                                          int value);

typedef int (*mbcm_sbx_fabric_distribution_control_get_f)(int unit,
                                                          bcm_fabric_distribution_t ds_id,
                                                          bcm_fabric_distribution_control_t type,
                                                          int *value);

typedef int (*mbcm_sbx_fabric_packet_adjust_set_f)(int unit,
                                                   int pkt_adjust_selector,
                                                   int pkt_adjust_len);
typedef int (*mbcm_sbx_fabric_packet_adjust_get_f)(int unit,
                                                   int pkt_adjust_selector,
                                                   int *pkt_adjust_len);

typedef int (*mbcm_sbx_vlan_control_vlan_set_f)(int unit,
                                                bcm_vlan_t vlan,
                                                bcm_vlan_control_vlan_t control);
typedef int (*mbcm_sbx_vlan_init_f)(int unit);
typedef int (*mbcm_sbx_vlan_create_f)(int unit,
                                      bcm_vlan_t vid);
typedef int (*mbcm_sbx_vlan_port_add_f)(int unit,
                                        bcm_vlan_t vid,
                                        bcm_pbmp_t pbmp,
                                        bcm_pbmp_t ubmp);
typedef int (*mbcm_sbx_vlan_port_remove_f)(int unit,
                                           bcm_vlan_t vid,
                                           bcm_pbmp_t pbmp);
typedef int (*mbcm_sbx_vlan_destroy_f)(int unit,
                                      bcm_vlan_t vid);
typedef int (*mbcm_sbx_vlan_destroy_all_f)(int unit);
typedef int (*mbcm_sbx_vlan_port_get_f)(int unit,
                                        bcm_vlan_t vid,
                                        pbmp_t *pbmp,
                                        pbmp_t *ubmp);
typedef int (*mbcm_sbx_vlan_list_f)(int unit,
                                    bcm_vlan_data_t **listp,
                                    int *countp);
typedef int (*mbcm_sbx_vlan_list_by_pbmp_f)(int unit,
                                            pbmp_t pbmp,
                                            bcm_vlan_data_t **listp,
                                            int *countp);
typedef int (*mbcm_sbx_vlan_list_destroy_f)(int unit,
                                            bcm_vlan_data_t *list,
                                            int count);
typedef int (*mbcm_sbx_vlan_default_set_f)(int unit,
                                            bcm_vlan_t vid);
typedef int (*mbcm_sbx_vlan_default_get_f)(int unit,
                                            bcm_vlan_t *vid_ptr);

typedef int (*mbcm_sbx_multicast_init_f)(int unit);

typedef int (*mbcm_sbx_multicast_detach_f)(int unit);

typedef int (*mbcm_sbx_multicast_create_f)(int unit,
                                           uint32 flags,
                                           bcm_multicast_t *group);

typedef int (*mbcm_sbx_multicast_destroy_f)(int unit,
                                           bcm_multicast_t group);

typedef int (*mbcm_sbx_multicast_group_get_f)(int unit,
                                              bcm_multicast_t group,
                                              uint32 *flags);

typedef int (*mbcm_sbx_multicast_group_traverse_f)(int unit,
                                                   bcm_multicast_group_traverse_cb_t cb,
                                                   uint32 flags,
                                                   void *user_data);

typedef int (*mbcm_sbx_multicast_egress_add_f)(int unit,
                                               bcm_multicast_t group,
                                               bcm_gport_t port,
                                               bcm_if_t encap_id);

typedef int (*mbcm_sbx_multicast_egress_delete_f)(int unit,
                                                  bcm_multicast_t group,
                                                  bcm_gport_t port,
                                                  bcm_if_t encap_id);

typedef int (*mbcm_sbx_multicast_egress_subscriber_add_f)(int unit,
                                                          bcm_multicast_t group,
                                                          bcm_gport_t port,
                                                          bcm_if_t encap_id,
                                                          bcm_gport_t subscriber_queue);

typedef int (*mbcm_sbx_multicast_egress_subscriber_delete_f)(int unit,
                                                             bcm_multicast_t group,
                                                             bcm_gport_t port,
                                                             bcm_if_t encap_id,
                                                             bcm_gport_t subscriber_queue);

typedef int (*mbcm_sbx_multicast_egress_delete_all_f)(int unit,
                                                      bcm_multicast_t group);

typedef int (*mbcm_sbx_multicast_egress_set_f)(int unit,
                                               bcm_multicast_t group,
                                               int port_count,
                                               bcm_gport_t *port_array,
                                               bcm_if_t *encap_id_array);

typedef int (*mbcm_sbx_multicast_egress_get_f)(int unit,
                                               bcm_multicast_t group,
                                               int port_max,
                                               bcm_gport_t *port_array,
                                               bcm_if_t *encap_id_array,
                                               int *port_count);

typedef int (*mbcm_sbx_multicast_egress_subscriber_set_f)(int unit,
                                                          bcm_multicast_t group,
                                                          int port_count,
                                                          bcm_gport_t *port_array,
                                                          bcm_if_t *encap_id_array,
                                                          bcm_gport_t *subscriber_queue_array);

typedef int (*mbcm_sbx_multicast_egress_subscriber_get_f)(int unit,
                                                          bcm_multicast_t group,
                                                          int port_max,
                                                          bcm_gport_t *port_array,
                                                          bcm_if_t *encap_id_array,
                                                          bcm_gport_t *subscriber_queue_array,
                                                          int *port_count);

typedef int (*mbcm_sbx_multicast_fabric_distribution_set_f)(int unit,
                                                            bcm_multicast_t            group,
                                                            bcm_fabric_distribution_t  ds_id);
typedef int (*mbcm_sbx_multicast_fabric_distribution_get_f)(int unit,
                                                            bcm_multicast_t            group,
                                                            bcm_fabric_distribution_t *ds_id);

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
typedef int (*mbcm_sbx_multicast_state_get_f)(int unit,
                                              char *pbuf);
#endif
#endif


/* port functions */
typedef int (*mbcm_sbx_port_init_f)(int unit);

typedef int (*mbcm_sbx_port_enable_set_f)(int unit,
                                          bcm_port_t port,
                                          int enable);
typedef int (*mbcm_sbx_port_enable_get_f)(int unit,
                                          bcm_port_t port,
                                          int *enable);
typedef int (*mbcm_sbx_port_speed_set_f)(int unit,
                                         bcm_port_t port,
                                         int speed);
typedef int (*mbcm_sbx_port_speed_get_f)(int unit,
                                         bcm_port_t port,
                                         int *speed);
typedef int (*mbcm_sbx_port_frame_max_set_f)(int unit,
                                             bcm_port_t port,
                                             int size);
typedef int (*mbcm_sbx_port_frame_max_get_f)(int unit,
                                             bcm_port_t port,
                                             int *size);
typedef int (*mbcm_sbx_port_link_status_get_f)(int unit,
                                               bcm_port_t port,
                                               int *up);
typedef int (*mbcm_sbx_port_loopback_set_f)(int unit,
                                            bcm_port_t port,
                                            int loopback);
typedef int (*mbcm_sbx_port_loopback_get_f)(int unit,
                                            bcm_port_t port,
                                            int *loopback);
typedef int (*mbcm_sbx_port_control_set_f)(int unit,
                                           bcm_port_t port,
                                           bcm_port_control_t type,
                                           int value);
typedef int (*mbcm_sbx_port_control_get_f)(int unit,
                                           bcm_port_t port,
                                           bcm_port_control_t type,
                                           int *value);
typedef int (*mbcm_sbx_port_linkscan_set_f)(int unit,
                                            bcm_port_t port,
                                            int linkscan);
typedef int (*mbcm_sbx_port_linkscan_get_f)(int unit,
                                            bcm_port_t port,
                                            int *linkscan);
typedef int (*mbcm_sbx_port_rate_egress_shaper_set_f)(int unit,
                                                      bcm_port_t port,
                                                      uint32 uShaperId,
                                                      uint32 kbits_sec,
                                                      uint32 kbits_burst);
typedef int (*mbcm_sbx_port_rate_egress_shaper_get_f)(int unit,
                                                      bcm_port_t port,
                                                      uint32 uShaperId,
                                                      uint32 *kbits_sec,
                                                      uint32 *kbits_burst);
typedef int (*mbcm_sbx_port_rate_egress_traffic_set_f)(int unit,
                                                       bcm_port_t port,
                                                       uint32 uShaperId,
                                                       uint32 traffic_types,
                                                       uint32 kbits_sec,
                                                       uint32 kbits_burst);
typedef int (*mbcm_sbx_port_rate_egress_traffic_get_f)(int unit,
                                                       bcm_port_t port,
                                                       uint32 uShaperId,
                                                       uint32 *traffic_types,
                                                       uint32 *kbits_sec,
                                                       uint32 *kbits_burst);
typedef int (*mbcm_sbx_port_probe_f)(int unit,
                                     pbmp_t pbmp,
                                     pbmp_t *okay_pbmp);
typedef int (*mbcm_sbx_port_ability_get_f)(int unit,
                                           bcm_port_t port,
                                           bcm_port_abil_t *ability_mask);

typedef int (*mbcm_sbx_port_congestion_config_set_f)(int unit, bcm_gport_t port,
                                           bcm_port_congestion_config_t *config, void *info);

typedef int (*mbcm_sbx_port_congestion_config_get_f)(int unit, bcm_gport_t port,
                                           bcm_port_congestion_config_t *config, void *info);

typedef int (*mbcm_sbx_port_scheduler_get_f)(int unit,
					     bcm_gport_t gport,
					     int *scheduler_level,
					     int *scheduler_node);

typedef int (*mbcm_sbx_port_is_egress_multicast_f)(int unit,
                                                   bcm_gport_t gport,
                                                   bcm_cos_queue_t cosq,
                                                   int *is_multicast);

typedef int (*mbcm_sbx_port_egress_multicast_scheduler_get_f)(int unit,
                                                              bcm_gport_t gport,
                                                              bcm_cos_queue_t cosq,
                                                              int *scheduler_level,
                                                              int *nbr_scheduler_nodes,
                                                              int *scheduler_nodes);

typedef int (*mbcm_sbx_port_egress_multicast_group_get_f)(int unit,
                                                          bcm_gport_t gport,
                                                          bcm_cos_queue_t cosq,
                                                          int *nbr_fifos,
                                                          int *nbr_egress_groups,
                                                          int *egress_groups);

typedef int (*mbcm_sbx_trunk_init_f)(int unit);

typedef int (*mbcm_sbx_trunk_create_old_f)(int unit, bcm_trunk_t *tid);

typedef int (*mbcm_sbx_trunk_create_f)(int unit, uint32 flags, bcm_trunk_t *tid);

typedef int (*mbcm_sbx_trunk_create_id_f)(int unit, bcm_trunk_t tid);

typedef int (*mbcm_sbx_trunk_destroy_f)(int unit, bcm_trunk_t tid);

typedef int (*mbcm_sbx_trunk_detach_f)(int unit);

typedef int (*mbcm_sbx_trunk_find_f)(int unit, bcm_module_t modid, bcm_port_t port,
                                     bcm_trunk_t *tid);

typedef int (*mbcm_sbx_trunk_get_old_f)(int unit, bcm_trunk_t tid,
                                        bcm_trunk_add_info_t *t_data);

typedef int (*mbcm_sbx_trunk_get_f)(int unit,
                                    bcm_trunk_t tid,
                                    bcm_trunk_info_t *t_data,
                                    int member_max,
                                    bcm_trunk_member_t *member_array,
                                    int *member_count);

typedef int (*mbcm_sbx_trunk_chip_info_get_f)(int unit,
                                              bcm_trunk_chip_info_t *ta_info);

typedef int (*mbcm_sbx_trunk_set_old_f)(int unit, bcm_trunk_t tid,
                                        bcm_trunk_add_info_t *add_info);

typedef int (*mbcm_sbx_trunk_set_f)(int unit,
                                    bcm_trunk_t tid,
                                    bcm_trunk_info_t *trunk_info,
                                    int member_count,
                                    bcm_trunk_member_t *member_array);

typedef int (*mbcm_sbx_stat_init_f)(int unit);

typedef int (*mbcm_sbx_stat_sync_f)(int unit);

typedef int (*mbcm_sbx_stat_get_f)(int unit,
				   bcm_port_t port,
				   bcm_stat_val_t type,
				   uint64 *val);
typedef int (*mbcm_sbx_stat_get32_f)(int unit,
				     bcm_port_t port,
				     bcm_stat_val_t type,
				     uint32 *val);

typedef int (*mbcm_sbx_stat_multi_get_f)(int unit,
					 bcm_port_t port, 
					 int nstat, 
					 bcm_stat_val_t *stat_arr, 
					 uint64 *value_arr);

typedef int (*mbcm_sbx_stat_multi_get32_f)(int unit,
					   bcm_port_t port, 
					   int nstat, 
					   bcm_stat_val_t *stat_arr, 
					   uint32 *value_arr);

typedef int (*mbcm_sbx_stat_clear_f)(int unit,
				    bcm_port_t port);

typedef int (*mbcm_sbx_stat_scoreboard_get_f)(int unit,
                                    uint32 * pBufLost,
                                    uint32 * pBufFrees,
                                    uint32 * pWatchdogErrs,
                                    uint32 * pShortIntervals,
                                    uint32 * pScoreboardTicks);

typedef int (*mbcm_sbx_stat_custom_set_f) (int unit,
					   bcm_port_t port,
					   bcm_stat_val_t type,
					   uint32 flags);

typedef int (*mbcm_sbx_stat_custom_get_f) (int unit,
					   bcm_port_t port,
					   bcm_stat_val_t type,
					   uint32 *flags);

typedef int (*mbcm_sbx_stat_custom_add_f) (int unit,
					   bcm_port_t port,
					   bcm_stat_val_t type,
					   bcm_custom_stat_trigger_t trigger);

typedef int (*mbcm_sbx_stat_custom_delete_f) (int unit,
					      bcm_port_t port,
					      bcm_stat_val_t type,
					      bcm_custom_stat_trigger_t trigger);

typedef int (*mbcm_sbx_stat_custom_delete_all_f) (int unit,
						  bcm_port_t port,
						  bcm_stat_val_t type);

typedef int (*mbcm_sbx_stat_custom_check_f) (int unit,
					     bcm_port_t port,
					     bcm_stat_val_t type,
					     bcm_custom_stat_trigger_t trigger,
					     int *result);


/* switch control functions */

typedef int (*mbcm_sbx_switch_control_set_f)(int unit,
                                           bcm_switch_control_t type,
                                           int value);

typedef int (*mbcm_sbx_switch_control_get_f)(int unit,
                                           bcm_switch_control_t type,
                                           int *value);

typedef int (*mbcm_sbx_switch_event_register_f)(int unit,
						bcm_switch_event_cb_t cb, 
						void *userdata);

typedef int (*mbcm_sbx_switch_event_unregister_f)(int unit,
						  bcm_switch_event_cb_t cb, 
						  void *userdata);

    /* subscriber map functions */

typedef int (*mbcm_sbx_cosq_subscriber_map_add_f) (int unit,
						   bcm_cosq_subscriber_map_t *map);

typedef int (*mbcm_sbx_cosq_subscriber_map_delete_f) (int unit,
						      bcm_cosq_subscriber_map_t *map);

typedef int (*mbcm_sbx_cosq_subscriber_map_delete_all_f) (int unit);

typedef int (*mbcm_sbx_cosq_subscriber_map_get_f) (int unit,
						   bcm_cosq_subscriber_map_t *map);

typedef int (*mbcm_sbx_cosq_subscriber_traverse_f) (int unit,
						    bcm_cosq_subscriber_map_traverse_cb cb,
						    void *user_data);

typedef int (*mbcm_sbx_cosq_gport_sched_config_set_f)(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 flags);

typedef int (*mbcm_sbx_cosq_gport_sched_config_get_f)(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 *flags);

typedef int (*mbcm_sbx_cosq_scheduler_allocate_f) (int unit, int level, int node_requested, int num_nodes, int egress, int *p_node);
typedef int (*mbcm_sbx_cosq_scheduler_free_f) (int unit, int level, int node, int egress);
typedef int (*mbcm_sbx_cosq_gport_queue_attach_f) (int unit, uint32 flags, bcm_gport_t ingress_queue, bcm_cos_t ingress_int_pri,
						   bcm_gport_t egress_queue, bcm_cos_t egress_int_pri, int *attach_id);
typedef int (*mbcm_sbx_cosq_gport_queue_attach_get_f) (int unit, bcm_gport_t ingress_queue, bcm_cos_t ingress_int_pri,
						       bcm_gport_t *egress_queue, bcm_cos_t *egress_int_pri, int attach_id);
typedef int (*mbcm_sbx_cosq_gport_queue_detach_f) (int unit, bcm_gport_t ingress_queue, bcm_cos_t ingress_int_pri, int attach_id);

typedef int (*mbcm_sbx_cosq_multipath_allocate_f) (int unit, int egress, int *p_multipath);
typedef int (*mbcm_sbx_cosq_multipath_free_f) (int unit, int egress, int multipath);
typedef int (*mbcm_sbx_cosq_multipath_add_f) (int unit, int egress, int multipath, int level, int node);
typedef int (*mbcm_sbx_cosq_multipath_delete_f) (int unit, int egress, int multipath, int level, int node);
typedef int (*mbcm_sbx_cosq_multipath_get_f) (int unit, int egress, int multipath,
					      int *member_count,
					      bcm_gport_t *member_gport_array,
					      int *member_gport_info_array);

/* failover functions */
typedef int (*mbcm_sbx_failover_enable_f)(int unit,
                                          int sysport,
                                          int node,
                                          int port,
                                          int old_node,
                                          int old_port);

typedef int (*mbcm_sbx_failover_set_f)(int unit,
                                       int sysport,
                                       int protect_node,
                                       int protect_port,
                                       int active_node,
                                       int active_port);

typedef int (*mbcm_sbx_failover_destroy_f)(int unit,
                                           int sysport);

/* frame steering functions */
typedef int (*mbcm_sbx_stk_steering_unicast_set_f)(int unit,
                                                   int steer_id,
                                                   bcm_module_t destmod_lo,
                                                   bcm_module_t destmod_hi,
                                                   int num_queue_groups,
                                                   bcm_gport_t *queue_groups);

typedef int (*mbcm_sbx_stk_steering_multicast_set_f)(int unit,
                                                     int steer_id,
                                                     bcm_multicast_t mgid_lo,
                                                     bcm_multicast_t mgid_hi,
                                                     int num_queue_groups,
                                                     bcm_gport_t *queue_groups);

typedef int (*mbcm_sbx_stk_steering_clear_f)(int unit,
                                             int steer_id);

typedef int (*mbcm_sbx_stk_steering_clear_all_f)(int unit);

/* predicate control */
typedef int (*mbcm_sbx_fabric_predicate_create_f)(int unit,
                                                  bcm_fabric_predicate_info_t *pred_info,
                                                  bcm_fabric_predicate_t *pred_id);

typedef int (*mbcm_sbx_fabric_predicate_destroy_f)(int unit,
                                                  bcm_fabric_predicate_t pred_id);

typedef int (*mbcm_sbx_fabric_predicate_destroy_all_f)(int unit);

typedef int (*mbcm_sbx_fabric_predicate_get_f)(int unit,
                                               bcm_fabric_predicate_t pred_id,
                                               bcm_fabric_predicate_info_t *pred_info);

typedef int (*mbcm_sbx_fabric_predicate_traverse_f)(int unit,
                                                    bcm_fabric_predicate_traverse_cb cb,
                                                    void *user_data);

/* parser (action) control */
typedef int (*mbcm_sbx_fabric_action_create_f)(int unit,
                                               bcm_fabric_action_info_t *action_info,
                                               bcm_fabric_action_t *action_id);

typedef int (*mbcm_sbx_fabric_action_destroy_f)(int unit,
                                                bcm_fabric_action_t action_id);

typedef int (*mbcm_sbx_fabric_action_destroy_all_f)(int unit);

typedef int (*mbcm_sbx_fabric_action_get_f)(int unit,
                                            bcm_fabric_action_t action_id,
                                            bcm_fabric_action_info_t *action_info);

typedef int (*mbcm_sbx_fabric_action_traverse_f)(int unit,
                                                 bcm_fabric_action_traverse_cb cb,
                                                 void *user_data);

/* type resolution (predicate_action) control */
typedef int (*mbcm_sbx_fabric_predicate_action_create_f)(int unit,
                                                         bcm_fabric_predicate_action_info_t *predicate_action,
                                                         bcm_fabric_predicate_action_t *predicate_action_id);

typedef int (*mbcm_sbx_fabric_predicate_action_get_f)(int unit,
                                                      bcm_fabric_predicate_action_t predicate_action_id,
                                                      bcm_fabric_predicate_action_info_t *predicate_action_info);

typedef int (*mbcm_sbx_fabric_predicate_action_destroy_f)(int unit,
                                                          bcm_fabric_predicate_action_t predicate_action_id);

typedef int (*mbcm_sbx_fabric_predicate_action_destroy_all_f)(int unit);

typedef int (*mbcm_sbx_fabric_predicate_action_traverse_f)(int unit,
                                                           bcm_fabric_predicate_action_traverse_cb cb,
                                                           void *user_data);

/* QUEUE_MAP (qsel) control */
typedef int (*mbcm_sbx_fabric_qsel_create_f)(int unit,
                                           uint32 flags,
                                           int base,
                                           int count,
                                           bcm_fabric_qsel_t *qsel_id);

typedef int (*mbcm_sbx_fabric_qsel_destroy_f)(int unit,
                                            bcm_fabric_qsel_t qsel_id);

typedef int (*mbcm_sbx_fabric_qsel_destroy_all_f)(int unit);

typedef int (*mbcm_sbx_fabric_qsel_get_f)(int unit,
                                          bcm_fabric_qsel_t qsel_id,
                                          uint32 *flags,
                                          int *base,
                                          int *count);

typedef int (*mbcm_sbx_fabric_qsel_traverse_f)(int unit,
                                               bcm_fabric_qsel_traverse_cb cb,
                                               void *user_data);

typedef int (*mbcm_sbx_fabric_qsel_entry_set_f)(int unit,
                                                bcm_fabric_qsel_t qsel_id,
                                                int offset,
                                                bcm_gport_t queue,
                                                bcm_fabric_qsel_offset_t qsel_offset_id);

typedef int (*mbcm_sbx_fabric_qsel_entry_get_f)(int unit,
                                                bcm_fabric_qsel_t qsel_id,
                                                int offset,
                                                bcm_gport_t *queue,
                                                bcm_fabric_qsel_offset_t *qsel_offset_id);

typedef int (*mbcm_sbx_fabric_qsel_entry_multi_set_f)(int unit,
                                                      bcm_fabric_qsel_t qsel_id,
                                                      int offset,
                                                      int count,
                                                      bcm_gport_t *queue,
                                                      bcm_fabric_qsel_offset_t *qsel_offset_id);

typedef int (*mbcm_sbx_fabric_qsel_entry_multi_get_f)(int unit,
                                                      bcm_fabric_qsel_t qsel_id,
                                                      int offset,
                                                      int count,
                                                      bcm_gport_t *queue,
                                                      bcm_fabric_qsel_offset_t *qsel_offset_id);

typedef int (*mbcm_sbx_fabric_qsel_entry_traverse_f)(int unit,
                                                     bcm_fabric_qsel_t qsel_id,
                                                     bcm_fabric_qsel_entry_traverse_cb cb,
                                                     void *user_data);

/* COS_MAP (qsel_offset) control */
typedef int (*mbcm_sbx_fabric_qsel_offset_create_f)(int unit,
                                                    uint32 flags,
                                                    bcm_fabric_qsel_offset_t *qsel_offset_id);

typedef int (*mbcm_sbx_fabric_qsel_offset_destroy_f)(int unit,
                                                     bcm_fabric_qsel_offset_t qsel_offset_id);

typedef int (*mbcm_sbx_fabric_qsel_offset_destroy_all_f)(int unit);

typedef int (*mbcm_sbx_fabric_qsel_offset_traverse_f)(int unit,
                                                      bcm_fabric_qsel_offset_traverse_cb cb,
                                                      void *user_data);

typedef int (*mbcm_sbx_fabric_qsel_offset_entry_set_f)(int unit,
                                                       bcm_fabric_qsel_offset_t qsel_offset_id,
                                                       bcm_cos_t int_pri,
                                                       int offset);

typedef int (*mbcm_sbx_fabric_qsel_offset_entry_get_f)(int unit,
                                                       bcm_fabric_qsel_offset_t qsel_offset_id,
                                                       bcm_cos_t int_pri,
                                                       int *offset);

typedef int (*mbcm_sbx_fabric_qsel_offset_entry_traverse_f)(int unit,
                                                            bcm_fabric_qsel_offset_t qsel_offset_id,
                                                            bcm_fabric_qsel_offset_entry_traverse_cb cb,
                                                            void *user_data);

/* Flow Control */
typedef int (*mbcm_sbx_fd_fct_get_f)  (int unit, int32 sysport, int32 ef, int32 mc, int32 *fct);


/****************************************************************
 *
 * Multiplexed BCM Driver Structure
 *
 ****************************************************************/
typedef struct mbcm_sbx_functions_s {
    /* modid/nodeid functions */
    mbcm_sbx_stk_modid_set_f                            mbcm_stk_modid_set;
    mbcm_sbx_stk_modid_get_f                            mbcm_stk_modid_get;
    mbcm_sbx_stk_my_modid_set_f                         mbcm_stk_my_modid_set;
    mbcm_sbx_stk_my_modid_get_f                         mbcm_stk_my_modid_get;
    mbcm_sbx_module_enable_f                            mbcm_stk_module_enable;
    mbcm_sbx_module_protocol_set_f                      mbcm_stk_module_protocol_set;
    mbcm_sbx_module_protocol_get_f                      mbcm_stk_module_protocol_get;
    mbcm_sbx_stk_fabric_map_set_f                       mbcm_stk_fabric_map_set;
    mbcm_sbx_stk_fabric_map_get_f                       mbcm_stk_fabric_map_get;

    /* fabric control functions */
    mbcm_sbx_fabric_crossbar_connection_set_f           mbcm_fabric_crossbar_connection_set;
    mbcm_sbx_fabric_crossbar_connection_get_f           mbcm_fabric_crossbar_connection_get;
    mbcm_sbx_fabric_tdm_enable_set_f                    mbcm_fabric_tdm_enable_set;
    mbcm_sbx_fabric_tdm_enable_get_f                    mbcm_fabric_tdm_enable_get;
    mbcm_sbx_fabric_calendar_max_get_f                  mbcm_fabric_calendar_max_get;
    mbcm_sbx_fabric_calendar_size_set_f                 mbcm_fabric_calendar_size_set;
    mbcm_sbx_fabric_calendar_size_get_f                 mbcm_fabric_calendar_size_get;
    mbcm_sbx_fabric_calendar_set_f                      mbcm_fabric_calendar_set;
    mbcm_sbx_fabric_calendar_get_f                      mbcm_fabric_calendar_get;
    mbcm_sbx_fabric_calendar_multi_set_f                mbcm_fabric_calendar_multi_set;
    mbcm_sbx_fabric_calendar_multi_get_f                mbcm_fabric_calendar_multi_get;
    mbcm_sbx_fabric_calendar_active_f                   mbcm_fabric_calendar_active;
    mbcm_sbx_fabric_crossbar_mapping_set_f              mbcm_fabric_crossbar_mapping_set;
    mbcm_sbx_fabric_crossbar_mapping_get_f              mbcm_fabric_crossbar_mapping_get;
    mbcm_sbx_fabric_crossbar_enable_set_f               mbcm_fabric_crossbar_enable_set;
    mbcm_sbx_fabric_crossbar_enable_get_f               mbcm_fabric_crossbar_enable_get;
    mbcm_sbx_fabric_crossbar_status_get_f               mbcm_fabric_crossbar_status_get;
    mbcm_sbx_fabric_control_set_f                       mbcm_fabric_control_set;
    mbcm_sbx_fabric_control_get_f                       mbcm_fabric_control_get;
    mbcm_sbx_fabric_port_create_f                       mbcm_fabric_port_create;
    mbcm_sbx_fabric_port_destroy_f                      mbcm_fabric_port_destroy;
    mbcm_sbx_fabric_congestion_size_set_f               mbcm_fabric_congestion_size_set;
    mbcm_sbx_fabric_congestion_size_get_f               mbcm_fabric_congestion_size_get;

    /* voq functions */
    mbcm_sbx_cosq_init_f                                mbcm_cosq_init;
    mbcm_sbx_cosq_detach_f                              mbcm_cosq_detach;
    mbcm_sbx_cosq_add_queue_f                           mbcm_cosq_add_queue;
    mbcm_sbx_cosq_delete_queue_f                        mbcm_cosq_delete_queue;
    mbcm_sbx_cosq_enable_queue_f                        mbcm_cosq_enable_queue;
    mbcm_sbx_cosq_disable_queue_f                       mbcm_cosq_disable_queue;
    mbcm_sbx_cosq_enable_fifo_f                         mbcm_cosq_enable_fifo;
    mbcm_sbx_cosq_disable_fifo_f                        mbcm_cosq_disable_fifo;
    mbcm_sbx_cosq_enable_get_f                          mbcm_cosq_enable_get;
    mbcm_sbx_cosq_overlay_queue_f                       mbcm_cosq_overlay_queue;
    mbcm_sbx_cosq_delete_overlay_queue_f                mbcm_cosq_delete_overlay_queue;
    mbcm_sbx_cosq_set_ingress_params_f                  mbcm_cosq_set_ingress_params;
    mbcm_sbx_cosq_set_ingress_shaper_f                  mbcm_cosq_set_ingress_shaper;
    mbcm_sbx_cosq_set_template_gain_f                   mbcm_cosq_set_template_gain;
    mbcm_sbx_cosq_get_template_gain_f                   mbcm_cosq_get_template_gain;
    mbcm_sbx_cosq_set_template_pfc_f                    mbcm_cosq_set_template_pfc;
    mbcm_sbx_cosq_get_template_pfc_f                    mbcm_cosq_get_template_pfc;
    mbcm_sbx_cosq_gport_discard_set_f                   mbcm_cosq_gport_discard_set; /* discard/WRED */
    mbcm_sbx_cosq_gport_discard_get_f                   mbcm_cosq_gport_discard_get; /* discard/WRED */
    mbcm_sbx_cosq_gport_stat_enable_set_f               mbcm_cosq_gport_stat_enable_set;
    mbcm_sbx_cosq_gport_stat_enable_get_f               mbcm_cosq_gport_stat_enable_get;
    mbcm_sbx_cosq_gport_stat_set_f                      mbcm_cosq_gport_stat_set;
    mbcm_sbx_cosq_gport_stat_get_f                      mbcm_cosq_gport_stat_get;
    mbcm_sbx_cosq_gport_stat_config_set_f               mbcm_cosq_gport_stat_config_set;
    mbcm_sbx_cosq_gport_stat_config_get_f               mbcm_cosq_gport_stat_config_get;
    mbcm_sbx_cosq_gport_statistic_set_f                 mbcm_cosq_gport_statistic_set;
    mbcm_sbx_cosq_gport_statistic_get_f                 mbcm_cosq_gport_statistic_get;
    mbcm_sbx_cosq_gport_statistic_multi_set_f           mbcm_cosq_gport_statistic_multi_set;
    mbcm_sbx_cosq_gport_statistic_multi_get_f           mbcm_cosq_gport_statistic_multi_get;
    mbcm_sbx_cosq_attach_scheduler_f                    mbcm_cosq_attach_scheduler;
    mbcm_sbx_cosq_detach_scheduler_f                    mbcm_cosq_detach_scheduler;
    mbcm_sbx_cosq_scheduler_attach_get_f                mbcm_cosq_scheduler_attach_get;
    mbcm_sbx_cosq_set_egress_scheduler_params_f         mbcm_cosq_set_egress_scheduler_params;
    mbcm_sbx_cosq_get_egress_scheduler_params_f		mbcm_cosq_get_egress_scheduler_params;
    mbcm_sbx_cosq_set_egress_shaper_params_f		mbcm_cosq_set_egress_shaper_params;
    mbcm_sbx_cosq_get_egress_shaper_params_f		mbcm_cosq_get_egress_shaper_params;
    mbcm_sbx_cosq_set_ingress_scheduler_params_f        mbcm_cosq_set_ingress_scheduler_params;
    mbcm_sbx_cosq_get_ingress_scheduler_params_f	mbcm_cosq_get_ingress_scheduler_params;
    mbcm_sbx_cosq_set_ingress_shaper_params_f		mbcm_cosq_set_ingress_shaper_params;
    mbcm_sbx_cosq_get_ingress_shaper_params_f		mbcm_cosq_get_ingress_shaper_params;
    mbcm_sbx_cosq_control_set_f                         mbcm_cosq_control_set;
    mbcm_sbx_cosq_control_get_f                         mbcm_cosq_control_get;
    mbcm_sbx_cosq_target_set_f                          mbcm_cosq_target_set;
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    mbcm_sbx_cosq_state_get_f                           mbcm_cosq_state_get;
#endif
#endif
    mbcm_sbx_cosq_egress_size_set_f                     mbcm_cosq_egress_size_set;
    mbcm_sbx_cosq_egress_size_get_f                     mbcm_cosq_egress_size_get;
    mbcm_sbx_cosq_flow_control_set_f                    mbcm_cosq_flow_control_set;
    mbcm_sbx_cosq_flow_control_get_f                    mbcm_cosq_flow_control_get;
    mbcm_sbx_cosq_pfc_config_set_f                      mbcm_cosq_pfc_config_set;
    mbcm_sbx_cosq_pfc_config_get_f                      mbcm_cosq_pfc_config_get;
    mbcm_sbx_cosq_port_congestion_set_f                 mbcm_cosq_port_congestion_set;
    mbcm_sbx_cosq_port_congestion_get_f                 mbcm_cosq_port_congestion_get;
    mbcm_sbx_cosq_gport_sched_config_set_f              mbcm_cosq_gport_sched_config_set;
    mbcm_sbx_cosq_gport_sched_config_get_f              mbcm_cosq_gport_sched_config_get;
    mbcm_sbx_cosq_scheduler_allocate_f                  mbcm_cosq_scheduler_allocate;
    mbcm_sbx_cosq_scheduler_free_f                      mbcm_cosq_scheduler_free;
    mbcm_sbx_cosq_gport_queue_attach_f                  mbcm_cosq_gport_queue_attach;
    mbcm_sbx_cosq_gport_queue_attach_get_f              mbcm_cosq_gport_queue_attach_get;
    mbcm_sbx_cosq_gport_queue_detach_f                  mbcm_cosq_gport_queue_detach;
    mbcm_sbx_cosq_mapping_set_f                         mbcm_cosq_mapping_set;
    mbcm_sbx_cosq_mapping_get_f                         mbcm_cosq_mapping_get;
    mbcm_sbx_cosq_multipath_allocate_f                  mbcm_cosq_multipath_allocate;
    mbcm_sbx_cosq_multipath_free_f                      mbcm_cosq_multipath_free;
    mbcm_sbx_cosq_multipath_add_f                       mbcm_cosq_multipath_add;
    mbcm_sbx_cosq_multipath_delete_f                    mbcm_cosq_multipath_delete;
    mbcm_sbx_cosq_multipath_get_f                       mbcm_cosq_multipath_get;

    /* multicast functions */
    mbcm_sbx_fabric_distribution_create_f               mbcm_fabric_distribution_create;
    mbcm_sbx_fabric_distribution_destroy_f              mbcm_fabric_distribution_destroy;
    mbcm_sbx_fabric_distribution_set_f                  mbcm_fabric_distribution_set;
    mbcm_sbx_fabric_distribution_get_f                  mbcm_fabric_distribution_get;
    mbcm_sbx_fabric_distribution_control_set_f          mbcm_fabric_distribution_control_set;
    mbcm_sbx_fabric_distribution_control_get_f          mbcm_fabric_distribution_control_get;
    mbcm_sbx_fabric_packet_adjust_set_f                 mbcm_fabric_packet_adjust_set;
    mbcm_sbx_fabric_packet_adjust_get_f                 mbcm_fabric_packet_adjust_get;
    mbcm_sbx_vlan_control_vlan_set_f                    mbcm_vlan_control_vlan_set;
    mbcm_sbx_vlan_init_f                                mbcm_vlan_init;
    mbcm_sbx_vlan_create_f                              mbcm_vlan_create;
    mbcm_sbx_vlan_port_add_f                            mbcm_vlan_port_add;
    mbcm_sbx_vlan_port_remove_f                         mbcm_vlan_port_remove;
    mbcm_sbx_vlan_destroy_f                             mbcm_vlan_destroy;
    mbcm_sbx_vlan_destroy_all_f                         mbcm_vlan_destroy_all;
    mbcm_sbx_vlan_port_get_f                            mbcm_vlan_port_get;
    mbcm_sbx_vlan_list_f                                mbcm_vlan_list;
    mbcm_sbx_vlan_list_by_pbmp_f                        mbcm_vlan_list_by_pbmp;
    mbcm_sbx_vlan_list_destroy_f                        mbcm_vlan_list_destroy;
    mbcm_sbx_vlan_default_set_f                         mbcm_vlan_default_set;
    mbcm_sbx_vlan_default_get_f                         mbcm_vlan_default_get;
    mbcm_sbx_multicast_init_f                           mbcm_multicast_init;
    mbcm_sbx_multicast_detach_f                         mbcm_multicast_detach;
    mbcm_sbx_multicast_create_f                         mbcm_multicast_create;
    mbcm_sbx_multicast_destroy_f                        mbcm_multicast_destroy;
    mbcm_sbx_multicast_group_get_f                      mbcm_multicast_group_get;
    mbcm_sbx_multicast_group_traverse_f                 mbcm_multicast_group_traverse;
    mbcm_sbx_multicast_egress_add_f                     mbcm_multicast_egress_add;
    mbcm_sbx_multicast_egress_delete_f                  mbcm_multicast_egress_delete;
    mbcm_sbx_multicast_egress_subscriber_add_f          mbcm_multicast_egress_subscriber_add;
    mbcm_sbx_multicast_egress_subscriber_delete_f       mbcm_multicast_egress_subscriber_delete;
    mbcm_sbx_multicast_egress_delete_all_f              mbcm_multicast_egress_delete_all;
    mbcm_sbx_multicast_egress_set_f                     mbcm_multicast_egress_set;
    mbcm_sbx_multicast_egress_get_f                     mbcm_multicast_egress_get;
    mbcm_sbx_multicast_egress_subscriber_set_f          mbcm_multicast_egress_subscriber_set;
    mbcm_sbx_multicast_egress_subscriber_get_f          mbcm_multicast_egress_subscriber_get;
    mbcm_sbx_multicast_fabric_distribution_set_f        mbcm_multicast_fabric_distribution_set;
    mbcm_sbx_multicast_fabric_distribution_get_f        mbcm_multicast_fabric_distribution_get;
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    mbcm_sbx_multicast_state_get_f                      mbcm_multicast_state_get;
#endif /* BCM_EASY_RELOAD_SUPPORT */
#endif /* BCM_EASY_RELOAD_SUPPORT_SW_DUMP */


    /* port functions */
    mbcm_sbx_port_init_f                                mbcm_port_init;
    mbcm_sbx_port_enable_set_f                          mbcm_port_enable_set;
    mbcm_sbx_port_enable_get_f                          mbcm_port_enable_get;
    mbcm_sbx_port_speed_set_f                           mbcm_port_speed_set;
    mbcm_sbx_port_speed_get_f                           mbcm_port_speed_get;
    mbcm_sbx_port_frame_max_set_f                       mbcm_port_frame_max_set;
    mbcm_sbx_port_frame_max_get_f                       mbcm_port_frame_max_get;
    mbcm_sbx_port_link_status_get_f                     mbcm_port_link_status_get;
    mbcm_sbx_port_loopback_set_f                        mbcm_port_loopback_set;
    mbcm_sbx_port_loopback_get_f                        mbcm_port_loopback_get;
    mbcm_sbx_port_control_set_f                         mbcm_port_control_set;
    mbcm_sbx_port_control_get_f                         mbcm_port_control_get;
    mbcm_sbx_port_linkscan_set_f                        mbcm_port_linkscan_set;
    mbcm_sbx_port_linkscan_get_f                        mbcm_port_linkscan_get;
    mbcm_sbx_port_rate_egress_shaper_set_f              mbcm_port_rate_egress_shaper_set;
    mbcm_sbx_port_rate_egress_shaper_get_f              mbcm_port_rate_egress_shaper_get;
    mbcm_sbx_port_rate_egress_traffic_set_f             mbcm_port_rate_egress_traffic_set;
    mbcm_sbx_port_rate_egress_traffic_get_f             mbcm_port_rate_egress_traffic_get;
    mbcm_sbx_port_probe_f                               mbcm_port_probe;
    mbcm_sbx_port_ability_get_f                         mbcm_port_ability_get;
    mbcm_sbx_port_congestion_config_set_f               mbcm_port_congestion_config_set;
    mbcm_sbx_port_congestion_config_get_f               mbcm_port_congestion_config_get;
    mbcm_sbx_port_scheduler_get_f                       mbcm_port_scheduler_get;
    mbcm_sbx_port_is_egress_multicast_f                 mbcm_port_is_egress_multicast;
    mbcm_sbx_port_egress_multicast_scheduler_get_f      mbcm_port_egress_multicast_scheduler_get;
    mbcm_sbx_port_egress_multicast_group_get_f          mbcm_port_egress_multicast_group_get;


    mbcm_sbx_trunk_init_f                       mbcm_trunk_init;
    mbcm_sbx_trunk_create_old_f                 mbcm_trunk_create_old;
    mbcm_sbx_trunk_create_f                     mbcm_trunk_create;
    mbcm_sbx_trunk_create_id_f                  mbcm_trunk_create_id;
    mbcm_sbx_trunk_destroy_f                    mbcm_trunk_destroy;
    mbcm_sbx_trunk_detach_f                     mbcm_trunk_detach;
    mbcm_sbx_trunk_find_f                       mbcm_trunk_find;
    mbcm_sbx_trunk_get_old_f                    mbcm_trunk_get_old;
    mbcm_sbx_trunk_get_f                        mbcm_trunk_get;
    mbcm_sbx_trunk_chip_info_get_f              mbcm_trunk_chip_info_get;
    mbcm_sbx_trunk_set_old_f                    mbcm_trunk_set_old;
    mbcm_sbx_trunk_set_f                        mbcm_trunk_set;

    /* stat functions */

    mbcm_sbx_stat_init_f                        mbcm_stat_init;
    mbcm_sbx_stat_sync_f                        mbcm_stat_sync;
    mbcm_sbx_stat_get_f                         mbcm_stat_get;
    mbcm_sbx_stat_get32_f                       mbcm_stat_get32;
    mbcm_sbx_stat_multi_get_f                   mbcm_stat_multi_get;
    mbcm_sbx_stat_multi_get32_f                 mbcm_stat_multi_get32;
    mbcm_sbx_stat_clear_f                       mbcm_stat_clear;
    mbcm_sbx_stat_scoreboard_get_f              mbcm_stat_scoreboard_get;

    mbcm_sbx_stat_custom_set_f                  mbcm_stat_custom_set;
    mbcm_sbx_stat_custom_get_f                  mbcm_stat_custom_get;
    mbcm_sbx_stat_custom_add_f                  mbcm_stat_custom_add;
    mbcm_sbx_stat_custom_delete_f               mbcm_stat_custom_delete;
    mbcm_sbx_stat_custom_delete_all_f           mbcm_stat_custom_delete_all;
    mbcm_sbx_stat_custom_check_f                mbcm_stat_custom_check;

    /* switch functions */

    mbcm_sbx_switch_control_set_f                       mbcm_switch_control_set;
    mbcm_sbx_switch_control_get_f                       mbcm_switch_control_get;
    mbcm_sbx_switch_event_register_f                    mbcm_switch_event_register;
    mbcm_sbx_switch_event_unregister_f                  mbcm_switch_event_unregister;

    /* subscriber map functions */

    mbcm_sbx_cosq_subscriber_map_add_f          mbcm_cosq_subscriber_map_add;
    mbcm_sbx_cosq_subscriber_map_delete_f	mbcm_cosq_subscriber_map_delete;
    mbcm_sbx_cosq_subscriber_map_delete_all_f	mbcm_cosq_subscriber_map_delete_all;
    mbcm_sbx_cosq_subscriber_map_get_f		mbcm_cosq_subscriber_map_get;
    mbcm_sbx_cosq_subscriber_traverse_f  	mbcm_cosq_subscriber_traverse;

    /* failover functions */
    mbcm_sbx_failover_enable_f                  mbcm_failover_enable;
    mbcm_sbx_failover_set_f                     mbcm_failover_set;
    mbcm_sbx_failover_destroy_f                 mbcm_failover_destroy;

    /* frame steering functions */
    mbcm_sbx_stk_steering_unicast_set_f         mbcm_stk_steering_unicast_set;
    mbcm_sbx_stk_steering_multicast_set_f       mbcm_stk_steering_multicast_set;
    mbcm_sbx_stk_steering_clear_f               mbcm_stk_steering_clear;
    mbcm_sbx_stk_steering_clear_all_f           mbcm_stk_steering_clear_all;

    /* predicate control */
    mbcm_sbx_fabric_predicate_create_f          mbcm_fabric_predicate_create;
    mbcm_sbx_fabric_predicate_destroy_f         mbcm_fabric_predicate_destroy;
    mbcm_sbx_fabric_predicate_destroy_all_f     mbcm_fabric_predicate_destroy_all;
    mbcm_sbx_fabric_predicate_get_f             mbcm_fabric_predicate_get;
    mbcm_sbx_fabric_predicate_traverse_f        mbcm_fabric_predicate_traverse;

    /* parser (action) control */
    mbcm_sbx_fabric_action_create_f             mbcm_fabric_action_create;
    mbcm_sbx_fabric_action_destroy_f            mbcm_fabric_action_destroy;
    mbcm_sbx_fabric_action_destroy_all_f        mbcm_fabric_action_destroy_all;
    mbcm_sbx_fabric_action_get_f                mbcm_fabric_action_get;
    mbcm_sbx_fabric_action_traverse_f           mbcm_fabric_action_traverse;

    /* type resolution (predicate_action) control */
    mbcm_sbx_fabric_predicate_action_create_f   mbcm_fabric_predicate_action_create;
    mbcm_sbx_fabric_predicate_action_get_f      mbcm_fabric_predicate_action_get;
    mbcm_sbx_fabric_predicate_action_destroy_f  mbcm_fabric_predicate_action_destroy;
    mbcm_sbx_fabric_predicate_action_destroy_all_f mbcm_fabric_predicate_action_destroy_all;
    mbcm_sbx_fabric_predicate_action_traverse_f mbcm_fabric_predicate_action_traverse;

    /* QUEUE_MAP (qsel) control */
    mbcm_sbx_fabric_qsel_create_f               mbcm_fabric_qsel_create;
    mbcm_sbx_fabric_qsel_destroy_f              mbcm_fabric_qsel_destroy;
    mbcm_sbx_fabric_qsel_destroy_all_f          mbcm_fabric_qsel_destroy_all;
    mbcm_sbx_fabric_qsel_get_f                  mbcm_fabric_qsel_get;
    mbcm_sbx_fabric_qsel_traverse_f             mbcm_fabric_qsel_traverse;
    mbcm_sbx_fabric_qsel_entry_set_f            mbcm_fabric_qsel_entry_set;
    mbcm_sbx_fabric_qsel_entry_get_f            mbcm_fabric_qsel_entry_get;
    mbcm_sbx_fabric_qsel_entry_multi_set_f      mbcm_fabric_qsel_entry_multi_set;
    mbcm_sbx_fabric_qsel_entry_multi_get_f      mbcm_fabric_qsel_entry_multi_get;
    mbcm_sbx_fabric_qsel_entry_traverse_f       mbcm_fabric_qsel_entry_traverse;

    /* COS_MAP (qsel_offset) control */
    mbcm_sbx_fabric_qsel_offset_create_f        mbcm_fabric_qsel_offset_create;
    mbcm_sbx_fabric_qsel_offset_destroy_f       mbcm_fabric_qsel_offset_destroy;
    mbcm_sbx_fabric_qsel_offset_destroy_all_f   mbcm_fabric_qsel_offset_destroy_all;
    mbcm_sbx_fabric_qsel_offset_traverse_f      mbcm_fabric_qsel_offset_traverse;
    mbcm_sbx_fabric_qsel_offset_entry_set_f     mbcm_fabric_qsel_offset_entry_set;
    mbcm_sbx_fabric_qsel_offset_entry_get_f     mbcm_fabric_qsel_offset_entry_get;
    mbcm_sbx_fabric_qsel_offset_entry_traverse_f mbcm_fabric_qsel_offset_entry_traverse;

    /* Flow Control */
    mbcm_sbx_fd_fct_get_f                       mbcm_fd_fct_get;

} mbcm_sbx_functions_t;

extern mbcm_sbx_functions_t *mbcm_sbx_driver[BCM_MAX_NUM_UNITS]; /* USE _CALL MACRO FOR DISPATCH */
extern soc_sbx_chip_family_t mbcm_sbx_family[BCM_MAX_NUM_UNITS];

extern mbcm_sbx_functions_t mbcm_qe2000_driver;
extern mbcm_sbx_functions_t mbcm_bm3200_driver;
extern mbcm_sbx_functions_t mbcm_bm9600_driver;
extern mbcm_sbx_functions_t mbcm_sirius_driver;

extern int mbcm_sbx_init(int unit);

#define MBCM_SBX_DRIVER_CALL(_unit, _function, _args) ((mbcm_sbx_driver[_unit] && mbcm_sbx_driver[_unit]->_function)?(mbcm_sbx_driver[_unit]->_function)_args:BCM_E_UNAVAIL)

/*
 *  This macro will complain BCM_E_UNAVAIL if the referenced unit does not
 *  have an SBX mbcm_sbx_driver structure, will return BCM_E_NONE if the
 *  unit does have the mbcm_sbx_driver structure but no implementation of the
 *  specified function, or the function's result if the unit has the structure
 *  and the function is implemented for the unit.
 *
 *  The intent is that it would be used for functions which may not be required
 *  for all units, rather than having to build dummy functions and waste the
 *  call/ret overhead to call them.
 *
 *  Note that args is NOT a vararg (some platforms puke on these still).
 *  Instead, it should be the entire args for the function in parens.
 */
#define MBCM_SBX_DRIVER_MAYBE_CALL(_unit, _function, _args) ((mbcm_sbx_driver[_unit])?((mbcm_sbx_driver[_unit]->_function)?(mbcm_sbx_driver[_unit]->_function)_args:BCM_E_NONE):BCM_E_UNAVAIL)



#endif  /* _BCM_INT_SBX_MBCM_H_ */
