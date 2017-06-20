/*
 * $Id: bm9600.h,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 Internal header
 */

#ifndef _BCM_INT_SBX_BM9600_H_
#define _BCM_INT_SBX_BM9600_H_

#include <bcm/port.h>
#include <bcm/cosq.h>
#include <bcm/fabric.h>
#include <bcm/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm/vlan.h>

extern int
bcm_bm9600_port_init(int unit);

/* modid/nodeid functions */
extern int
bcm_bm9600_stk_modid_set(int unit, int modid);

extern int
bcm_bm9600_stk_modid_get(int unit, int *modid);

extern int
bcm_bm9600_stk_my_modid_set(int unit, int modid);

extern int
bcm_bm9600_stk_my_modid_get(int unit, int *modid);

extern int
bcm_bm9600_stk_module_enable(int unit, int modid, int nports, int enable);

extern int
bcm_bm9600_stk_module_protocol_set(int unit, int node, bcm_module_protocol_t  protocol);

extern int
bcm_bm9600_stk_module_protocol_get(int unit, int node, bcm_module_protocol_t *protocol);

/* fabric control functions */
extern int
bcm_bm9600_fabric_crossbar_connection_set(int unit,
					  int xbar,
					  int src_modid,
					  bcm_port_t src_xbport,
					  int dst_modid,
					  bcm_port_t dst_xbport);

extern int
bcm_bm9600_fabric_crossbar_connection_get(int unit,
					  int xbar,
					  int src_modid,
					  bcm_port_t src_xbport,
					  int dst_modid,
					  bcm_port_t *dst_xbport);

extern int
bcm_bm9600_fabric_calendar_active(int unit);

extern int
bcm_bm9600_fabric_crossbar_mapping_set(int unit,
				       int modid,
				       int switch_fabric_arbiter_id,
				       int xbar,
				       bcm_port_t port);

extern int
bcm_bm9600_fabric_crossbar_mapping_get(int unit,
				       int modid,
				       int switch_fabric_arbiter_id,
				       int xbar,
				       bcm_port_t *port);

extern int
bcm_bm9600_fabric_crossbar_enable_set(int unit, uint64 xbars);

extern int
bcm_bm9600_fabric_crossbar_enable_get(int unit, uint64 *xbars);

extern int
bcm_bm9600_fabric_crossbar_status_get(int unit, uint64 *xbars);

extern int
bcm_bm9600_fabric_control_set(int unit,
			      bcm_fabric_control_t type,
			      int arg);

extern int
bcm_bm9600_fabric_control_get(int unit,
			      bcm_fabric_control_t type,
			      int *arg);

extern int
bcm_bm9600_fabric_control_redundancy_register(int unit,
					      bcm_fabric_control_redundancy_handler_t f);

extern int
bcm_bm9600_fabric_control_redundancy_unregister(int unit,
						bcm_fabric_control_redundancy_handler_t f);


/* VOQ functions */
extern int
bcm_bm9600_cosq_init(int unit);

extern int
bcm_bm9600_cosq_add_queue(int unit,
			  int queue,
                          bcm_sbx_cosq_queue_region_type_t queue_region,
			  int sysport,
                          int eset,
			  int dest_node,
			  int dest_port,
			  int dest_mc,
			  int dest_cos,
			  int32 dest_type,
			  bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
			  bcm_sbx_cosq_bw_group_params_t *p_bwparams,
                          int inhibit_write);
extern int
bcm_bm9600_cosq_delete_queue(int unit,
			     int queue,
                             bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_bm9600_cosq_overlay_queue(int unit,
			      int queue,
                              bcm_sbx_cosq_queue_region_type_t queue_region,
			      int sysport,
			      int dest_node,
			      int dest_port,
			      int dest_mc,
			      int dest_cos,
			      int dest_type);

extern int
bcm_bm9600_cosq_delete_overlay_queue(int unit,
				     int queue,
				     int base_queue,
                                     bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_bm9600_cosq_set_ingress_params(int unit,
				   int32 queue,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
				   bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
				   bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                   bcm_sbx_queue_size_info_t           *p_sizeInfo);

extern int
bcm_bm9600_cosq_gport_queue_attach(int unit, uint32 flags,
				   bcm_gport_t ingress_queue,
				   bcm_cos_t ingress_int_pri,
				   bcm_gport_t egress_queue,
				   bcm_cos_t egress_int_pri,
				   int *attach_id);

extern int
bcm_bm9600_cosq_gport_queue_attach_get(int unit,
				       bcm_gport_t ingress_queue,
				       bcm_cos_t ingress_int_pri,
				       bcm_gport_t *egress_queue,
				       bcm_cos_t *egress_int_pri,
				       int attach_id);

extern int
bcm_bm9600_cosq_gport_queue_detach(int unit,
				   bcm_gport_t ingress_queue,
				   bcm_cos_t ingress_int_pri,
				   int attach_id);

extern int
bcm_bm9600_cosq_set_ingress_shaper(int unit,
				   int base_queue,
				   bcm_cos_queue_t cosq,
				   int num_cos_levels,
				   uint32 shape_limit_kbps,
				   int set_logical_port_shaper,
				   int enable_shaping);

extern int
bcm_bm9600_cosq_set_template_gain (int unit,
				   int queue,
				   int template,
				   int gain);

extern int
bcm_bm9600_cosq_get_template_gain (int unit,
				   int queue,
				   int *template,
				   int *gain);

extern int
bcm_bm9600_cosq_gport_discard_set(int unit,
				  bcm_gport_t gport,
				  bcm_cos_t priority,
				  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
				  bcm_cosq_gport_discard_t *discard);

extern int
bcm_bm9600_cosq_gport_discard_get(int unit,
				  bcm_gport_t gport,
				  bcm_cos_t priority,
				  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
				  bcm_cosq_gport_discard_t *discard);

extern int
bcm_bm9600_map_sysport_to_nodeport(int unit,
				   int sysport,
				   int node,
				   int port);

extern int
bcm_bm9600_unmap_sysport(int unit,
			 int sysport);


extern int
bcm_bm9600_get_portset_from_sysport(int unit,
				    int sysport,
				    int *portset,
				    int *offset);

extern int
bcm_bm9600_map_sysport_to_portset(int unit,
				  int sysport,
				  int portset,
				  int offset,
				  int new_row);

extern int
bcm_bm9600_cosq_gport_sched_config_set(int unit, bcm_gport_t gport,
				       int sched_mode, int int_pri, uint32 flags);

extern int
bcm_bm9600_cosq_gport_sched_config_get(int unit, bcm_gport_t gport,
				       int sched_mode, int int_pri, uint32 *flags);


/* multicast functions */
extern int
bcm_bm9600_fabric_distribution_create(int unit,
				      bcm_fabric_distribution_t *ds_id);

extern int
bcm_bm9600_fabric_distribution_destroy(int unit,
				       bcm_fabric_distribution_t  ds_id);

extern int
bcm_bm9600_fabric_distribution_set(int unit,
				   bcm_fabric_distribution_t  ds_id,
				   int modid_count,
				   int *dist_modids,
				   int mc_full_eval_min);

extern int
bcm_bm9600_fabric_distribution_get(int unit,
				   bcm_fabric_distribution_t  ds_id,
				   int max_count,
				   int *dist_modids,
				   int *count);
extern int
bcm_bm9600_fabric_distribution_control_set(int unit,
					   bcm_fabric_distribution_t ds_id,
					   bcm_fabric_distribution_control_t type,
					   int value);

extern int
bcm_bm9600_fabric_distribution_control_get(int unit,
					   bcm_fabric_distribution_t ds_id,
					   bcm_fabric_distribution_control_t type,
					   int *value);

/* port functions */
extern int
bcm_bm9600_port_enable_set(int unit, bcm_port_t port, int enable);

extern int
bcm_bm9600_port_enable_get(int unit, bcm_port_t port, int *enable);

extern int
bcm_bm9600_port_speed_set(int unit, bcm_port_t port, int speed);

extern int
bcm_bm9600_port_speed_get(int unit, bcm_port_t port, int *speed);

extern int
bcm_bm9600_port_link_get(int unit, bcm_port_t port, int *up);

extern int
bcm_bm9600_port_link_status_get(int unit, bcm_port_t port, int *up);

extern int
bcm_bm9600_port_loopback_set(int unit, bcm_port_t port, int loopback);

extern int
bcm_bm9600_port_loopback_get(int unit, bcm_port_t port, int *loopback);

extern int
bcm_bm9600_port_control_set(int unit, bcm_port_t port,
			    bcm_port_control_t type, int value);

extern int
bcm_bm9600_port_control_get(int unit, bcm_port_t port,
			    bcm_port_control_t type, int *value);

extern int
bcm_bm9600_port_ability_get(int unit, bcm_port_t port,
                            bcm_port_abil_t *ability_mask);

extern int
bcm_bm9600_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp);


extern int
bcm_bm9600_ability_matching_speed_set(int unit, bcm_port_t port, int ability);

/* failover functions */
extern int
bcm_bm9600_failover_enable(int unit, int sysport, int node, int port, int old_node, int old_port);

extern int
bcm_bm9600_failover_set(int unit,
			int sysport,
			int protect_node,
			int protect_port,
			int active_node,
			int active_port);

extern int
bcm_bm9600_failover_destroy(int unit, int sysport);

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_bm9600_cosq_state_get(int unit,
			  char *pbuf);
#endif
#endif

#endif /* _BCM_INT_SBX_BM9600_H_ */
