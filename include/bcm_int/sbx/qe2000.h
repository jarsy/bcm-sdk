/*
 * $Id: qe2000.h,v 1.38 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Internal header
 */

#ifndef _BCM_INT_SBX_QE2000_H_
#define _BCM_INT_SBX_QE2000_H_

#include <soc/sbx/counter.h>

#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm/fabric.h>
#include <bcm/multicast.h>
#include <bcm/cosq.h>
#include <bcm/vlan.h>
#include <bcm/switch.h>
#include <bcm/stack.h>
#include <bcm/stat.h>
#include <bcm/trunk.h>

#include <bcm_int/common/trunk.h>

#define BCM_INT_SBX_ARBPORT_STATE_GLOBAL  0
#define BCM_INT_SBX_ARBPORT_STATE_LOCAL   1
#define BCM_INT_SBX_ARBPORT_STATE_RESERVED  2

typedef struct {
  int32 base_queue;
  int16 ref_count;
  int8 free;
  int8 state;  /* BCM_INT_SBX_ARBPORT_STATE_GLOBAL, ... */
  int8  arb_group; /* break up arbiter ports into 32 groups of 32 entries */
                   /* supporting the 2 levels of arbitration in TME mode  */
                   /* there are 32 arb groups                             */

}virtual_node_port_t;


#define BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX 32
#define BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX 32

typedef struct {
  int32 dest_node[BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX];
  int32 dest_port[BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX];
}arb_group_t;


/* modid/nodeid functions */
extern int
bcm_qe2000_stk_modid_set(int unit, int modid);

extern int
bcm_qe2000_stk_modid_get(int unit, int *modid);

extern int
bcm_qe2000_stk_my_modid_set(int unit, int modid);

extern int
bcm_qe2000_stk_my_modid_get(int unit, int *modid);

extern int
bcm_qe2000_stk_module_enable(int unit, int modid, int nports, int enable);

extern int
bcm_qe2000_stk_module_protocol_set(int unit, int node, bcm_module_protocol_t  protocol);

extern int
bcm_qe2000_stk_module_protocol_get(int unit, int node, bcm_module_protocol_t *protocol);


/* fabric control functions */
extern int
bcm_qe2000_fabric_crossbar_connection_set(int unit,
					  int xbar,
					  int src_modid,
					  bcm_port_t src_xbport,
					  int dst_modid,
					  bcm_port_t dst_xbport);

extern int
bcm_qe2000_fabric_crossbar_connection_get(int unit,
					  int xbar,
					  int src_modid,
					  bcm_port_t src_xbport,
					  int dst_modid,
					  bcm_port_t *dst_xbport);

extern int
bcm_qe2000_fabric_crossbar_mapping_set(int unit,
				       int modid,
				       int switch_fabric_arbiter_id,
				       int xbar,
				       bcm_port_t port);

extern int
bcm_qe2000_fabric_crossbar_mapping_get(int unit,
				       int modid,
				       int switch_fabric_arbiter_id,
				       int xbar,
				       bcm_port_t *port);

extern int
bcm_qe2000_fabric_crossbar_enable_set(int unit, uint64 xbars);

extern int
bcm_qe2000_fabric_crossbar_enable_get(int unit, uint64 *xbars);

extern int
bcm_qe2000_fabric_crossbar_status_get(int unit, uint64 *xbars);

extern int
bcm_qe2000_fabric_control_set(int unit,
			      bcm_fabric_control_t type,
			      int arg);

extern int
bcm_qe2000_fabric_control_get(int unit,
			      bcm_fabric_control_t type,
			      int *arg);

extern int
bcm_qe2000_fabric_control_redundancy_register(int unit,
					      bcm_fabric_control_redundancy_handler_t f);

extern int
bcm_qe2000_fabric_control_redundancy_unregister(int unit,
						bcm_fabric_control_redundancy_handler_t f);


/* VOQ functions */
extern int
bcm_qe2000_cosq_init(int unit);

extern int
bcm_qe2000_cosq_add_queue(int unit,
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
bcm_qe2000_cosq_delete_queue(int unit,
			     int queue,
                             bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_qe2000_cosq_enable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_qe2000_cosq_disable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_qe2000_cosq_enable_fifo(int  unit, 
			    bcm_gport_t gport, 
			    bcm_cos_queue_t cosq,
			    bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_qe2000_cosq_disable_fifo(int  unit, 
			     bcm_gport_t gport, 
			     bcm_cos_queue_t cosq,
			     bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_qe2000_cosq_enable_get(int  unit, 
			   bcm_gport_t gport, 
			   bcm_cos_queue_t cosq,
			   int *enable);

extern int
bcm_qe2000_cosq_overlay_queue(int unit,
			      int queue,
                              bcm_sbx_cosq_queue_region_type_t queue_region,
			      int sysport,
			      int dest_node,
			      int dest_port,
			      int dest_mc,
			      int dest_cos,
			      int dest_type);

extern int
bcm_qe2000_cosq_delete_overlay_queue(int unit,
				     int queue,
				     int base_queue,
                                     bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_qe2000_cosq_set_ingress_params(int unit,
				   int32 queue,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
				   bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
				   bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                   bcm_sbx_queue_size_info_t           *p_sizeInfo);

extern int
bcm_qe2000_cosq_set_ingress_shaper(int unit,
				   int base_queue,
				   bcm_cos_queue_t cosq,
				   int num_cos_levels,
				   uint32 shape_limit_kbps,
				   int set_logical_port_shaper,
				   int enable_shaping);
extern int
bcm_qe2000_cosq_set_template_gain (int unit,
				   int queue,
				   int template,
				   int gain);
extern int
bcm_qe2000_cosq_gport_discard_set(int unit,
				  bcm_gport_t gport,
				  bcm_cos_t priority,
				  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
				  bcm_cosq_gport_discard_t *discard);

extern int
bcm_qe2000_cosq_gport_discard_get(int unit,
				  bcm_gport_t gport,
				  bcm_cos_t priority,
				  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
				  bcm_cosq_gport_discard_t *discard);


extern int
bcm_qe2000_cosq_gport_stat_enable_set(int unit,
        bcm_gport_t gport,
        int   enable);


extern int
bcm_qe2000_cosq_gport_stat_enable_get(int unit,
        bcm_gport_t gport,
        int   *enable);

extern int
bcm_qe2000_cosq_gport_stat_set(int unit,
        bcm_gport_t gport,
        bcm_cos_t   cos,
        bcm_cosq_gport_stats_t stat,
        uint64  value);

extern int
bcm_qe2000_cosq_gport_stat_get(int unit,
        bcm_gport_t gport,
        bcm_cos_t   cos,
        bcm_cosq_gport_stats_t stat,
        uint64   *value);

extern int
bcm_qe2000_cosq_sysport_port_remap(int unit,
				   int sysport,
				   int ef,
				   int remap_port);

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
extern int
bcm_qe2000_cosq_state_get(int unit,
			  char *pbuf);
#endif /* BCM_EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* BCM_EASY_RELOAD_SUPPORT */

extern int
bcm_qe2000_cosq_egress_size_set(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 bytes_min,
                                uint32 bytes_max);

extern int
bcm_qe2000_cosq_egress_size_get(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 *bytes_min,
                                uint32 *bytes_max);

extern int
bcm_qe2000_stat_init(int unit);

extern int
bcm_qe2000_stat_get(int unit,
		    bcm_port_t port,
		    bcm_stat_val_t type,
		    uint64 *val);
extern int
bcm_qe2000_stat_get32(int unit,
		      bcm_port_t port,
		      bcm_stat_val_t type,
		      uint32 *val);

extern int
bcm_qe2000_stat_clear(int unit,
		      bcm_port_t port);

extern int
bcm_qe2000_stat_custom_set(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   uint32 flags);

extern int
bcm_qe2000_stat_custom_get(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   uint32 *flags);

extern int
bcm_qe2000_stat_custom_add(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   bcm_custom_stat_trigger_t trigger);

extern int
bcm_qe2000_stat_custom_delete(int unit,
			      bcm_port_t port,
			      bcm_stat_val_t type,
			      bcm_custom_stat_trigger_t trigger);

extern int
bcm_qe2000_stat_custom_delete_all(int unit,
				  bcm_port_t port,
				  bcm_stat_val_t type);


int
bcm_qe2000_stat_custom_check(int unit,
			     bcm_port_t port,
			     bcm_stat_val_t type,
			     bcm_custom_stat_trigger_t trigger,
			     int *result);

extern int bcm_qe2000_stat_sync(int unit);

extern int
bcm_qe2000_stat_scoreboard_get(int unit,
			       uint32 *pBufLost,
			       uint32 *pBufFrees,
			       uint32 *pWatchdogErrs,
			       uint32 *pShortIntervals,
			       uint32 *pScoreboardTicks);


/* multicast functions */
extern int
bcm_qe2000_fabric_distribution_create(int unit,
				      bcm_fabric_distribution_t *ds_id);

extern int
bcm_qe2000_fabric_distribution_destroy(int unit,
				       bcm_fabric_distribution_t  ds_id);

extern int
bcm_qe2000_fabric_distribution_set(int unit,
				   bcm_fabric_distribution_t  ds_id,
				   int modid_count,
				   int *dist_modids);

extern int
bcm_qe2000_fabric_distribution_get(int unit,
				   bcm_fabric_distribution_t  ds_id,
				   int max_count,
				   int *dist_modids,
				   int *count);

extern int
bcm_qe2000_fabric_packet_adjust_set(int unit,
                                    int pkt_adjust_selector,
                                    int pkt_adjust_len);

extern int
bcm_qe2000_fabric_packet_adjust_get(int unit,
                                    int pkt_adjust_selector,
                                    int *pkt_adjust_len);

extern int
bcm_qe2000_vlan_control_vlan_set(int unit,
				 bcm_vlan_t vlan,
				 bcm_vlan_control_vlan_t control);

extern int
bcm_qe2000_vlan_init(int unit);

extern int
bcm_qe2000_vlan_create(int unit,
                       bcm_vlan_t vid);

extern int
bcm_qe2000_vlan_destroy_internal(int unit,
                                 bcm_vlan_t vid,
                                 int diags);

extern int
bcm_qe2000_vlan_destroy(int unit,
                        bcm_vlan_t vid);

extern int
bcm_qe2000_vlan_destroy_all(int unit);

extern int
bcm_qe2000_vlan_port_add(int unit,
			 bcm_vlan_t vid,
			 bcm_pbmp_t pbmp,
			 bcm_pbmp_t ubmp);

extern int
bcm_qe2000_vlan_port_remove(int unit,
			    bcm_vlan_t vid,
			    bcm_pbmp_t pbmp);
extern int
bcm_qe2000_vlan_port_get(int unit,
                      bcm_vlan_t vid,
                      pbmp_t *pbmp,
                      pbmp_t *ubmp);
extern int
bcm_qe2000_vlan_list(int unit,
                  bcm_vlan_data_t **listp,
                  int *countp);
extern int
bcm_qe2000_vlan_list_by_pbmp(int unit,
                          pbmp_t pbmp,
                          bcm_vlan_data_t **listp,
                          int *countp);
extern int
bcm_qe2000_vlan_list_destroy(int unit,
                          bcm_vlan_data_t *list,
                          int count);
extern int
bcm_qe2000_vlan_default_set(int unit,
                            bcm_vlan_t vid);
extern int
bcm_qe2000_vlan_default_get(int unit,
                            bcm_vlan_t *vid_ptr);
extern int
bcm_qe2000_multicast_create(int unit,
                            uint32 flags,
                            bcm_multicast_t *group);

extern int
bcm_qe2000_multicast_destroy(int unit,
                             bcm_multicast_t group);

extern int
bcm_qe2000_multicast_group_get(int unit,
                               bcm_multicast_t group,
                               uint32 *flags);

extern int
bcm_qe2000_multicast_group_traverse(int unit,
                                    bcm_multicast_group_traverse_cb_t cb,
                                    uint32 flags,
                                    void *user_data);

extern int
bcm_qe2000_multicast_egress_add(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_if_t encap_id);

extern int
bcm_qe2000_multicast_egress_delete(int unit,
                                   bcm_multicast_t group,
                                   bcm_gport_t port,
                                   bcm_if_t encap_id);

extern int
bcm_qe2000_multicast_egress_delete_all(int unit,
                                bcm_multicast_t group);

extern int
bcm_qe2000_multicast_egress_set(int unit,
                                bcm_multicast_t group,
                                int port_count,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array);

extern int
bcm_qe2000_multicast_egress_get(int unit,
                                bcm_multicast_t group,
                                int port_max,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array,
                                int *port_count);

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
extern int
bcm_qe2000_multicast_state_get(int unit,
			       char *pbuf);
#endif
#endif

/* port functions */
extern int
bcm_qe2000_port_init(int unit);

extern int
bcm_qe2000_port_enable_set(int unit, bcm_port_t port, int enable);

extern int
bcm_qe2000_port_enable_get(int unit, bcm_port_t port, int *enable);

extern int
bcm_qe2000_port_speed_set(int unit, bcm_port_t port, int speed);

extern int
bcm_qe2000_port_speed_get(int unit, bcm_port_t port, int *speed);

extern int
bcm_qe2000_port_frame_max_set(int unit, bcm_port_t port, int size);

extern int
bcm_qe2000_port_frame_max_get(int unit, bcm_port_t port, int *size);

extern int
bcm_qe2000_port_link_get(int unit, bcm_port_t port, int *up);

extern int
bcm_qe2000_port_link_status_get(int unit, bcm_port_t port, int *up);

extern int
bcm_qe2000_port_loopback_set(int unit, bcm_port_t port, int loopback);

extern int
bcm_qe2000_port_loopback_get(int unit, bcm_port_t port, int *loopback);

extern int
bcm_qe2000_port_control_set(int unit, bcm_port_t port,
			    bcm_port_control_t type, int value);

extern int
bcm_qe2000_port_control_get(int unit, bcm_port_t port,
			    bcm_port_control_t type, int *value);

extern int
bcm_qe2000_port_rate_egress_set(int unit, bcm_port_t port,
				uint32 kbits_sec, uint32 kbits_burst);

extern int
bcm_qe2000_port_rate_egress_get(int unit, bcm_port_t port,
				uint32 *kbits_sec, uint32 *kbits_burst);


extern int
bcm_qe2000_port_rate_egress_shaper_set(int unit, bcm_port_t port, uint32 shaper_id,
				uint32 kbits_sec, uint32 kbits_burst);

extern int
bcm_qe2000_port_rate_egress_shaper_get(int unit, bcm_port_t port, uint32 shaper_id,
				uint32 *kbits_sec, uint32 *kbits_burst);

extern int
bcm_qe2000_port_rate_egress_traffic_set(int unit, bcm_port_t port,  uint32 shaper_id,
				uint32 traffic_types, uint32 kbits_sec, uint32 kbits_burst);

extern int
bcm_qe2000_port_rate_egress_traffic_get(int unit, bcm_port_t port, uint32 shaper_id,
				uint32 *trafffic_types, uint32 *kbits_sec, uint32 *kbits_burst);


int
bcm_qe2000_fifo_egress_shaper_get (int unit, int  nFifoParmIndex,
				   uint32 *pShaperIdHiPort, uint32 *pShaperIdLoFifo);

extern int
bcm_qe2000_port_linkscan_get(int unit, bcm_port_t port, int *linkscan);

extern int
bcm_qe2000_port_linkscan_set(int unit, bcm_port_t port, int linkscan);

extern int
bcm_qe2000_port_autoneg_get(int unit, bcm_port_t port, int *autoneg);

extern int
bcm_qe2000_port_autoneg_set(int unit, bcm_port_t port, int autoneg);

extern int
bcm_qe2000_port_ability_get(int unit, bcm_port_t port,
                            bcm_port_abil_t *ability_mask);

extern int
bcm_qe2000_port_fifo_enable_set(int unit,
				int cos,
				int sysport,
				int port,
				int local,
				int enable);

extern int
bcm_qe2000_port_fifo_enable_get(int unit,
				int cos,
				int sysport,
				int port,
				int local,
				int *enable);

int
bcm_qe2000_port_size_set(int unit, bcm_port_t port, int cos,
                                            uint32  bytes_min, uint32  bytes_max);

int
bcm_qe2000_port_size_get(int unit, bcm_port_t port, int cos,
                                            uint32  *bytes_min, uint32  *bytes_max);

int
bcm_qe2000_port_multicast_size_set(int unit,
				   int cos,
				   uint32  bytes_min,
				   uint32  bytes_max);

int
bcm_qe2000_port_multicast_size_get(int unit,
				   int cos,
				   uint32  *bytes_min,
				   uint32  *bytes_max);

extern int
bcm_qe2000_switch_control_set(int unit, bcm_switch_control_t type, int val);

extern int
bcm_qe2000_switch_control_get(int unit, bcm_switch_control_t type, int *val);

extern int 
bcm_qe2000_switch_event_register(int unit, 
				 bcm_switch_event_cb_t cb, 
				 void *userdata);
extern int 
bcm_qe2000_switch_event_unregister(int unit, 
				   bcm_switch_event_cb_t cb, 
				   void *userdata);

extern int
bcm_qe2000_trunk_init(int unit);

extern int
bcm_qe2000_trunk_create(int unit, bcm_trunk_t *tid);

extern int
bcm_qe2000_trunk_create_id(int unit, bcm_trunk_t tid);

extern int
bcm_qe2000_trunk_destroy(int unit, bcm_trunk_t tid);

extern int
bcm_qe2000_trunk_detach(int unit);

extern int
bcm_qe2000_trunk_find(int unit, bcm_module_t modid, bcm_port_t port, bcm_trunk_t *tid);

extern int
bcm_qe2000_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data);

extern int
bcm_qe2000_trunk_chip_info_get(int unit,
                               bcm_trunk_chip_info_t *ta_info);

extern int
bcm_qe2000_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info);


extern int
bcm_qe2000_multicast_source_knockout_get(int unit, bcm_multicast_t group, int *source_knockout);

extern int
bcm_qe2000_multicast_source_knockout_set(int unit, bcm_multicast_t group, int source_knockout);

/* failover functions */
extern int
bcm_qe2000_failover_enable(int unit, int sysport, int node, int port, int old_node, int old_port);

extern int
bcm_qe2000_failover_set(int unit,
			int sysport,
			int protect_node,
			int protect_port,
			int active_node,
			int active_port);

extern int
bcm_qe2000_cosq_control_set(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            bcm_cosq_control_t type,
                            int value,
                            bcm_sbx_cosq_control_t *control_info);

extern int
bcm_qe2000_cosq_control_get(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            bcm_cosq_control_t type,
                            int *value,
                            bcm_sbx_cosq_control_t *control_info);

extern int
bcm_qe2000_cosq_subscriber_map_add(int unit,
				bcm_cosq_subscriber_map_t *map);

extern int
bcm_qe2000_cosq_subscriber_map_delete(int unit,
				   bcm_cosq_subscriber_map_t *map);

extern int
bcm_qe2000_cosq_subscriber_map_delete_all(int unit);

extern int
bcm_qe2000_cosq_subscriber_map_get(int unit,
				bcm_cosq_subscriber_map_t *map);

extern int
bcm_qe2000_cosq_subscriber_traverse(int unit,
				    bcm_cosq_subscriber_map_traverse_cb cb,
				    void *user_data);

extern int
bcm_qe2000_cosq_gport_sched_config_set(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 flags);

extern int
bcm_qe2000_cosq_gport_sched_config_get(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 *flags);

extern int
bcm_qe2000_cosq_detach(int unit);


#endif /* _BCM_INT_SBX_QE2000_H_ */
