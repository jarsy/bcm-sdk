/*
 * $Id: sirius.h,v 1.98 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SIRIUS Internal header
 */

#ifndef _BCM_INT_SBX_SIRIUS_H_
#define _BCM_INT_SBX_SIRIUS_H_

#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm/fabric.h>
#include <bcm/multicast.h>
#include <bcm/cosq.h>
#include <bcm/vlan.h>
#include <bcm/switch.h>
#include <bcm/stack.h>
#include <bcm/trunk.h>
#include <bcm/stat.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/common/trunk.h>


#define BCM_INT_SBX_ARBPORT_STATE_GLOBAL  0
#define BCM_INT_SBX_ARBPORT_STATE_LOCAL   1

#define CU_NUM_QUEUE       0
#define CU_NUM_STATS_LABEL 0
#define CU_NUM_PHYSICAL    1
#define CU_NUM_DEQUEUE     1
#define CU_NUM_EP          2

typedef enum {
  BCM_META0_DISABLE,
  BCM_META0_SYS_PORT_ID,
  BCM_META0_CHAN_ID,
  BCM_META0_FIFO_NUM,
  BCM_META0_INTF_NUM,
  BCM_META0_SUBPORT_ID,
  BCM_META0_PKT_CLASSS,
  BCM_META0_BIT_VECTOR
} ep_stat_meta0_t;

typedef enum ep_pred_meta_sel_s {
  ep_frame_data,
  ep_fab_sys_port_id,
  ep_chan_id,
  ep_fifo_num,
  ep_intf_num,
  ep_subport_id
} ep_pred_meta_sel_t;

typedef enum {
  qm_addr_queue,
  qm_addr_stats_label,
  qm_addr_interface,
  qm_addr_histogram
} qm_statscfg_addr_t;

typedef struct {
  int8          in_use;
  bcm_module_t  module;
} bcm_sbx_sirius_fc_state;

typedef struct bcm_sbx_sirius_ingress_scheduler_state_s {
    int16    in_use;               /* TRUE/FALSE/RESERVED, physical scheduler in use state */
    int16    parent;               /* parent physical scheduler node */
    bcm_gport_t parent_gport;      /* parent gport (type child corresponding to front panel port) */
    int32    first_child;          /* first physical scheduler child node */
    uint16   num_child;            /* number of children */
    int32    logical_scheduler;    /* logical scheduler id */
    int      scheduler_mode;
    int      scheduler_param;
    int      min_shaper_rate_kbps;
    int      min_shaper_threshold_bits;
    int      max_shaper_rate_kbps;
    int      max_shaper_threshold_bits;
    int      bucket;
    int8     adopt_all;
}bcm_sbx_sirius_ingress_scheduler_state_t;

typedef struct bcm_sbx_sirius_egress_scheduler_state_s {
    sbBool_t in_use;        /* TRUE/FALSE */
    int      intf;          /* interface of the scheduler node */
    int      channel;       /* channel of the scheduler node */
    int      subport;       /* subport of the scheduler node */
    int      num_child;     /* number of attached child */
    int      index;         /* index within the same parent */
    int      scheduler_mode;
    int      scheduler_param;
    int      min_shaper_rate_kbps;
    int      min_shaper_threshold_bits;
    int      max_shaper_rate_kbps;
    int      max_shaper_threshold_bits;
}bcm_sbx_sirius_egress_scheduler_state_t;

typedef enum {
    BCM_SIRIUS_COSQ_INTERNAL_PORT_STANDARD = 0,
    BCM_SIRIUS_COSQ_INTERNAL_PORT_MULTICAST,
    BCM_SIRIUS_COSQ_INTERNAL_PORT_UNICAST_OVERSUB
} bcm_sirius_cosq_internal_port_class_t;

/* physical scheduler state */
extern bcm_sbx_sirius_ingress_scheduler_state_t *is_state[SOC_MAX_NUM_DEVICES][SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS];
extern bcm_sbx_sirius_egress_scheduler_state_t  *es_state[SOC_MAX_NUM_DEVICES][SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS];

/* modid/nodeid functions */
extern int
bcm_sirius_stk_modid_set(int unit, int modid);

extern int
bcm_sirius_stk_modid_get(int unit, int *modid);

extern int
bcm_sirius_stk_my_modid_set(int unit, int modid);

extern int
bcm_sirius_stk_my_modid_get(int unit, int *modid);

extern int
bcm_sirius_stk_module_enable(int unit, int modid, int nports, int enable);

extern int
bcm_sirius_stk_module_protocol_set(int unit, int node, bcm_module_protocol_t  protocol);

extern int
bcm_sirius_stk_module_protocol_get(int unit, int node, bcm_module_protocol_t *protocol);

extern int
bcm_sirius_stk_fabric_map_set(int unit, bcm_gport_t switch_port, bcm_gport_t fabric_port);

extern int
bcm_sirius_stk_fabric_map_get(int unit, bcm_gport_t switch_port, bcm_gport_t *fabric_port);

extern int
bcm_sirius_cosq_get_ingress_scheduler(int unit,
				      int level,
				      int node,
				      bcm_sbx_sirius_ingress_scheduler_state_t *p_is);

/* fabric control functions */
extern int
bcm_sirius_fabric_crossbar_connection_set(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t dst_xbport);

extern int
bcm_sirius_fabric_crossbar_connection_get(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport);

extern int
bcm_sirius_fabric_tdm_enable_set(int unit,
				 int enable);

extern int
bcm_sirius_fabric_tdm_enable_get(int unit,
				 int *enable);

extern int
bcm_sirius_fabric_calendar_max_get(int unit, int *max_size);

extern int
bcm_sirius_fabric_calendar_size_set(int unit, int config_size);

extern int
bcm_sirius_fabric_calendar_size_get(int unit, int *config_size);

extern int
bcm_sirius_fabric_calendar_set(int unit,
			       int cindex,
			       int connection,
			       bcm_gport_t dest_port,
			       bcm_cos_queue_t dest_cosq);

extern int
bcm_sirius_fabric_calendar_get(int unit,
			       int cindex,
			       int *connection,
			       bcm_gport_t *dest_port,
			       bcm_cos_queue_t *dest_cosq);

extern int
bcm_sirius_fabric_calendar_multi_set(int unit,
				     int array_size,
				     int *connection_array,
				     bcm_gport_t *dest_port_array,
				     bcm_gport_t *dest_cosq_array);
extern int
bcm_sirius_fabric_calendar_multi_get(int unit,
				     int array_size,
				     int *connection_array,
				     bcm_gport_t *dest_port_array,
				     bcm_gport_t *dest_cosq_array);

 
extern int
bcm_sirius_fabric_global_lchan_get(int unit, 
                                   int lxbar, 
                                   int *used_in_a,
                                   int *used_in_b);                        
 
extern int
bcm_sirius_fabric_update_lchan_for_port_ability(int unit, 
                                                int sfi_port, 
                                                int new_ability, 
                                                int old_ability);
extern int
bcm_sirius_fabric_update_lchan_for_port_enable_set(int unit, 
                                                   int sfi_port, 
                                                   int enable);

extern int
bcm_sirius_fabric_crossbar_mapping_set(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t port);

extern int
bcm_sirius_fabric_crossbar_mapping_get(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t *port);

extern int
bcm_sirius_fabric_crossbar_enable_set(int unit, uint64 xbars);

extern int
bcm_sirius_fabric_crossbar_enable_get(int unit, uint64 *xbars);

extern int
bcm_sirius_fabric_crossbar_status_get(int unit, uint64 *xbars);

extern int
bcm_sirius_fabric_control_set(int unit,
                              bcm_fabric_control_t type,
                              int arg);

extern int
bcm_sirius_fabric_control_get(int unit,
                              bcm_fabric_control_t type,
                              int *arg);

extern int
bcm_sirius_fabric_control_redundancy_register(int unit,
                                              bcm_fabric_control_redundancy_handler_t f);

extern int
bcm_sirius_fabric_control_redundancy_unregister(int unit,
                                                bcm_fabric_control_redundancy_handler_t f);


/* VOQ functions */
extern int
bcm_sirius_cosq_init(int unit);

extern int
bcm_sirius_cosq_detach(int unit);

extern int
bcm_sirius_cosq_add_queue(int unit,
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
bcm_sirius_cosq_delete_queue(int unit,
                             int queue,
                             bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_sirius_cosq_enable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_sirius_cosq_disable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_sirius_cosq_enable_fifo(int  unit, 
			    bcm_gport_t gport, 
			    bcm_cos_queue_t cosq,
			    bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_sirius_cosq_disable_fifo(int  unit, 
			     bcm_gport_t gport, 
			     bcm_cos_queue_t cosq,
			     bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_sirius_cosq_enable_get(int  unit, 
			   bcm_gport_t gport, 
			   bcm_cos_queue_t cosq,
			   int *enable);

extern int
bcm_sirius_cosq_overlay_queue(int unit,
                              int queue,
                              bcm_sbx_cosq_queue_region_type_t queue_region,
                              int sysport,
                              int dest_node,
                              int dest_port,
                              int dest_mc,
                              int dest_cos,
                              int dest_type);

extern int
bcm_sirius_cosq_delete_overlay_queue(int unit,
                                     int queue,
                                     int base_queue,
                                     bcm_sbx_cosq_queue_region_type_t queue_region);

extern int
bcm_sirius_cosq_target_set(int unit,
                           bcm_gport_t gport,
                           bcm_module_t module,
                           int sysport,
                           int port);

extern int
bcm_sirius_cosq_set_ingress_params(int unit,
                                   int32 queue,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
                                   bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
                                   bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
                                   bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
                                   bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                   bcm_sbx_queue_size_info_t           *p_sizeInfo);

extern int
bcm_sirius_cosq_set_ingress_shaper(int unit,
                                   int base_queue,
                                   bcm_cos_queue_t cosq,
                                   int num_cos_levels,
                                   uint32 shape_limit_kbps,
                                   int set_logical_port_shaper,
                                   int enable_shaping);
extern int
bcm_sirius_cosq_set_template_gain (int unit,
                                   int queue,
                                   int template,
                                   int gain);
extern int
bcm_sirius_cosq_gport_discard_set(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard);

extern int
bcm_sirius_cosq_gport_discard_get(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard);


extern int
bcm_sirius_cosq_gport_stat_enable_set(int unit,
        bcm_gport_t gport,
        int   enable);


extern int
bcm_sirius_cosq_gport_stat_enable_get(int unit,
        bcm_gport_t gport,
        int   *enable);

extern int
bcm_sirius_cosq_gport_stat_set(int unit,
        bcm_gport_t gport,
        bcm_cos_t   cos,
        bcm_cosq_gport_stats_t stat,
        uint64  value);

extern int
bcm_sirius_cosq_gport_stat_get(int unit,
        bcm_gport_t gport,
        bcm_cos_t   cos,
        bcm_cosq_gport_stats_t stat,
        uint64   *value);


extern int
bcm_sirius_cosq_gport_stat_config_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_gport_t lgl_gport,
                                      bcm_cos_queue_t cosq,
                                      uint32 flags,
                                      bcm_cosq_gport_stat_profile_t profile);

extern int
bcm_sirius_cosq_gport_stat_config_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_gport_t lgl_gport,
                                      bcm_cos_queue_t cosq,
                                      uint32 flags,
                                      bcm_cosq_gport_stat_profile_t *profile);

extern int
bcm_sirius_cosq_gport_statistic_set(int unit,
                                  bcm_gport_t gport,
                                  bcm_gport_t lgl_gport,
                                  bcm_cos_queue_t cosq,
                                  uint32 flags,
                                  bcm_cosq_gport_stats_t stat,
                                  uint64 value);

extern int
bcm_sirius_cosq_gport_statistic_get(int unit,
                                  bcm_gport_t gport,
                                  bcm_gport_t lgl_gport,
                                  bcm_cos_queue_t cosq,
                                  uint32 flags,
                                  bcm_cosq_gport_stats_t stat,
                                  uint64 *value);

extern int
bcm_sirius_cosq_gport_statistic_multi_set(int unit,
                                  bcm_gport_t gport,
                                  bcm_gport_t lgl_gport,
                                  bcm_cos_queue_t cosq,
                                  int stat_count,
				  bcm_cosq_gport_stats_t *stats_array,
                                  uint64 value);


extern int
bcm_sirius_cosq_gport_statistic_multi_get(int unit,
                                  bcm_gport_t gport,
                                  bcm_gport_t lgl_gport,
                                  bcm_cos_queue_t cosq,
                                  int stat_count,
                                  bcm_cosq_gport_stats_t *stats_array,
                                  int value_count,
				  uint64 *value_array);

extern int
bcm_sirius_cosq_attach_scheduler(int unit,
                                 bcm_gport_t parent_gport,
                                 bcm_gport_t child_gport,
                                 bcm_cos_queue_t cosq,
                                 int egress);

extern int
bcm_sirius_cosq_detach_scheduler(int unit,
                                 bcm_gport_t parent_gport,
                                 bcm_gport_t child_gport,
				 bcm_cos_queue_t cosq,
                                 int egress);

extern int
bcm_sirius_cosq_scheduler_attach_get(int unit,
				     bcm_gport_t in_gport,
				     int egress,
				     bcm_cos_queue_t *cosq,
				     bcm_gport_t *out_gport);

extern int
bcm_sirius_cosq_sysport_port_remap(int unit,
                                   int sysport,
                                   int ef,
                                   int remap_port);

extern int
bcm_sirius_cosq_set_egress_scheduler_params(int unit,
					    int level,
					    int node,
					    int mode,
					    int weight);

extern int
bcm_sirius_cosq_get_egress_scheduler_params(int unit,
					    int level,
					    int node,
					    int *mode,
					    int *weight);

extern int
bcm_sirius_cosq_set_egress_shaper_params(int unit,
					 int level,
					 int node,
					 int kbits_sec_min,
					 int bits_thresh_min,
					 int kbits_sec_max,
					 int bits_thresh_max);

extern int
bcm_sirius_cosq_get_egress_shaper_params(int unit,
					 int level,
					 int node,
					 int *kbits_sec_min,
					 int *bits_thresh_min,
					 int *kbits_sec_max,
					 int *bits_thresh_max);

extern int
bcm_sirius_cosq_set_ingress_scheduler_params(int unit,
					    int level,
					    int node,
					    int mode,
					    int weight);

extern int
bcm_sirius_cosq_get_ingress_scheduler_params(int unit,
					    int level,
					    int node,
					    int *mode,
					    int *weight);

extern int
bcm_sirius_cosq_set_ingress_shaper_params(int unit,
					 int level,
					 int node,
					 int kbits_sec_min,
					 int bits_thresh_min,
					 int kbits_sec_max,
					 int bits_thresh_max);

extern int
bcm_sirius_cosq_get_ingress_shaper_params(int unit,
					 int level,
					 int node,
					 int *kbits_sec_min,
					 int *bits_thresh_min,
					 int *kbits_sec_max,
					 int *bits_thresh_max);

extern int
bcm_sirius_cosq_control_set(int unit,
			    bcm_gport_t gport,
			    bcm_cos_queue_t cosq,
			    bcm_cosq_control_t type,
			    int value,
                            bcm_sbx_cosq_control_t *control_info);

extern int
bcm_sirius_cosq_control_get(int unit,
			    bcm_gport_t gport,
			    bcm_cos_queue_t cosq,
			    bcm_cosq_control_t type,
			    int *value,
                            bcm_sbx_cosq_control_t *control_info);

extern int
bcm_sirius_cosq_control_adjust(int unit,
                               bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               bcm_cosq_control_t type,
                               int *value,
                               bcm_sbx_cosq_control_t *control_info);

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
extern int
bcm_sirius_cosq_state_get(int unit,
                          char *pbuf);
#endif /* BCM_EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* BCM_EASY_RELOAD_SUPPORT */

extern int
bcm_sirius_cosq_flow_control_set(int unit,
                                 bcm_gport_t port,
                                 bcm_cos_t int_pri,
                                 uint32 flow_control_mask,
                                 void *info);

extern int
bcm_sirius_cosq_flow_control_get(int unit,
                                 bcm_gport_t port,
                                 bcm_cos_t int_pri,
                                 uint32 *flow_control_mask,
                                 void *info);

extern int
bcm_sirius_cosq_congestion_set(int unit,
                               bcm_port_t port,
                               bcm_sbx_port_congestion_info_t *congestion_info,
                               bcm_port_congestion_config_t *config);

extern int
bcm_sirius_cosq_pfc_config_set(int unit, 
			       bcm_gport_t gport, 
			       bcm_cos_queue_t cosq, 
			       uint32 flags, 
			       bcm_cosq_pfc_config_t *config);

extern int
bcm_sirius_cosq_pfc_config_get(int unit, 
			       bcm_gport_t gport, 
			       bcm_cos_queue_t cosq, 
			       uint32 flags, 
			       bcm_cosq_pfc_config_t *config);

extern int
bcm_sirius_cosq_port_congestion_set(int unit,
				    bcm_gport_t congestion_port,
				    bcm_gport_t port,
				    uint32 flags,
				    bcm_port_congestion_config_t config,
				    int channel_id);

extern int
bcm_sirius_cosq_port_congestion_get(int unit,
				    bcm_gport_t congestion_port,
				    bcm_gport_t port,
				    uint32 flags,
				    int *channel_id);

extern int
bcm_sirius_cosq_set_template_pfc(int unit,
				 int32 template,
				 int pfc_en,
				 int pfc_pg);

extern int
bcm_sirius_cosq_get_template_pfc(int unit,
				 int32 template,
				 int *pfc_en,
				 int *pfc_pg);

extern int
bcm_sirius_cosq_egress_size_set(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 bytes_min,
                                uint32 bytes_max);

extern int
bcm_sirius_cosq_egress_size_get(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 *bytes_min,
                                uint32 *bytes_max);
extern int
bcm_sirius_port_size_set(int unit, 
			 int fcd,
			 int cosq,
			 uint32  bytes_min, 
			 uint32  bytes_max);

extern int
bcm_sirius_port_size_get(int unit, 
			 int fcd, 
			 int cosq,
			 uint32  *bytes_min, 
			 uint32  *bytes_max);

extern int
bcm_sirius_port_multicast_size_set(int unit,
				   int fcd,
				   int cosq,
				   uint32  bytes_min,
				   uint32  bytes_max);

extern int
bcm_sirius_port_multicast_size_get(int unit,
				   int fcd,
				   int cosq,
				   uint32  *bytes_min,
				   uint32  *bytes_max);

extern int
bcm_sirius_stat_init(int unit);

extern int
bcm_sirius_stat_get(int unit,
		    bcm_port_t port,
		    bcm_stat_val_t type,
		    uint64 *val);
extern int
bcm_sirius_stat_get32(int unit,
		      bcm_port_t port,
		      bcm_stat_val_t type,
		      uint32 *val);

extern int
bcm_sirius_stat_multi_get(int unit, bcm_port_t port, int nstat, 
			  bcm_stat_val_t *stat_arr, uint64 *value_arr);

extern int 
bcm_sirius_stat_multi_get32(int unit, bcm_port_t port, int nstat, 
			    bcm_stat_val_t *stat_arr, uint32 *value_arr);

extern int 
bcm_sirius_stat_clear(int unit,
		      bcm_port_t port);

extern int
bcm_sirius_stat_custom_set(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   uint32 flags);

extern int
bcm_sirius_stat_custom_get(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   uint32 *flags);

extern int 
bcm_sirius_stat_custom_add(int unit,
			   bcm_port_t port,
			   bcm_stat_val_t type,
			   bcm_custom_stat_trigger_t trigger);

extern int 
bcm_sirius_stat_custom_delete(int unit,
			      bcm_port_t port,
			      bcm_stat_val_t type, 
			      bcm_custom_stat_trigger_t trigger);

extern int 
bcm_sirius_stat_custom_delete_all(int unit,
				  bcm_port_t port,
				  bcm_stat_val_t type);


int 
bcm_sirius_stat_custom_check(int unit,
			     bcm_port_t port,
			     bcm_stat_val_t type, 
			     bcm_custom_stat_trigger_t trigger,
			     int *result);

extern int bcm_sirius_stat_sync(int unit);

/* multicast functions */
extern int
bcm_sirius_fabric_distribution_create(int unit,
                                      bcm_fabric_distribution_t *ds_id);

extern int
bcm_sirius_fabric_distribution_destroy(int unit,
                                       bcm_fabric_distribution_t  ds_id);

extern int
bcm_sirius_fabric_distribution_set(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int modid_count,
                                   int *dist_modids,
				   int mc_full_eval_min);

extern int
bcm_sirius_fabric_distribution_get(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int max_count,
                                   int *dist_modids,
                                   int *count);

extern int
bcm_sirius_fabric_packet_adjust_set(int unit,
                                    int pkt_adjust_selector,
                                    int pkt_adjust_len);

extern int
bcm_sirius_fabric_packet_adjust_get(int unit,
                                    int pkt_adjust_selector,
                                    int *pkt_adjust_len);

extern int
bcm_sirius_fabric_port_create(int unit, bcm_gport_t parent_port, 
			      int offset, uint32 flags, int *subport);

extern int
bcm_sirius_fabric_port_create_es(int unit,
				 bcm_gport_t physical_gport,
				 int offset,
				 int num_fifos,
				 uint32 flags,
				 int *handle);

extern int
bcm_sirius_fabric_port_create_ts(int unit,
				 bcm_gport_t physical_gport,
				 uint32 flags,
				 int handle);

extern int
bcm_sirius_fabric_port_destroy(int unit, bcm_gport_t child_gport);

extern int
bcm_sirius_fabric_congestion_size_set(int unit, bcm_module_t module_id, int max_ports);

extern int
bcm_sirius_fabric_congestion_size_get(int unit, bcm_module_t module_id, int *max_ports);

int
bcm_sirius_fabric_sci_plane_get(int unit, bcm_port_t port, int *plane_a);

int
bcm_sirius_fabric_unmap_link_status(int unit, int sfi_port);

int
bcm_sirius_fabric_map_link_status(int unit, int lxbar, int sfi_port);

extern int
bcm_sirius_multicast_create(int unit,
                            uint32 flags,
                            bcm_multicast_t *group);

extern int
bcm_sirius_multicast_destroy(int unit,
                             bcm_multicast_t group);

extern int
bcm_sirius_multicast_egress_add(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_if_t encap_id);

extern int
bcm_sirius_multicast_egress_subscriber_add(int unit,
                                           bcm_multicast_t group,
                                           bcm_gport_t port,
                                           bcm_if_t encap_id,
                                           bcm_gport_t queue_id);

extern int
bcm_sirius_multicast_egress_delete(int unit,
                                   bcm_multicast_t group,
                                   bcm_gport_t port,
                                   bcm_if_t encap_id);

extern int
bcm_sirius_multicast_egress_subscriber_delete(int unit,
                                              bcm_multicast_t group,
                                              bcm_gport_t port,
                                              bcm_if_t encap_id,
                                              bcm_gport_t queue_id);

extern int
bcm_sirius_multicast_egress_delete_all(int unit,
                                bcm_multicast_t group);

extern int
bcm_sirius_multicast_egress_set(int unit,
                                bcm_multicast_t group,
                                int port_count,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array);

extern int
bcm_sirius_multicast_egress_subscriber_set(int unit,
                                           bcm_multicast_t group,
                                           int port_count,
                                           bcm_gport_t *port_array,
                                           bcm_if_t *encap_id_array,
                                           bcm_gport_t *queue_id_array);

extern int
bcm_sirius_multicast_egress_get(int unit,
                                bcm_multicast_t group,
                                int port_max,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array,
                                int *port_count);

extern int
bcm_sirius_multicast_egress_subscriber_get(int unit,
                                           bcm_multicast_t group,
                                           int port_max,
                                           bcm_gport_t *port_array,
                                           bcm_if_t *encap_id_array,
                                           bcm_gport_t *queue_id_array,
                                           int *port_count);

extern int
bcm_sirius_multicast_fabric_distribution_set(int unit,
                                             bcm_multicast_t            group,
                                             bcm_fabric_distribution_t  ds_id);

extern int
bcm_sirius_multicast_fabric_distribution_get(int unit,
                                             bcm_multicast_t            group,
                                             bcm_fabric_distribution_t *ds_id);

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
extern int
bcm_sirius_multicast_state_get(int unit,
                               char *pbuf);
#endif
#endif

/* port functions */
extern int
bcm_sirius_port_init(int unit);

extern int
bcm_sirius_port_deinit(int unit);

extern int
bcm_sirius_port_enable_set(int unit, bcm_port_t port, int enable);

extern int
bcm_sirius_port_enable_get(int unit, bcm_port_t port, int *enable);

extern int
bcm_sirius_port_speed_set(int unit, bcm_port_t port, int speed);

extern int
bcm_sirius_port_speed_get(int unit, bcm_port_t port, int *speed);

extern int
bcm_sirius_port_link_get(int unit, bcm_port_t port, int *up);

extern int
bcm_sirius_port_link_status_get(int unit, bcm_port_t port, int *up);

extern int
bcm_sirius_port_loopback_set(int unit, bcm_port_t port, int loopback);

extern int
bcm_sirius_port_loopback_get(int unit, bcm_port_t port, int *loopback);

extern int
bcm_sirius_port_control_set(int unit, bcm_port_t port,
                            bcm_port_control_t type, int value);

extern int
bcm_sirius_port_control_get(int unit, bcm_port_t port,
                            bcm_port_control_t type, int *value);

extern int
bcm_sirius_port_linkscan_get(int unit, bcm_port_t port, int *linkscan);

extern int
bcm_sirius_port_linkscan_set(int unit, bcm_port_t port, int linkscan);

extern int
bcm_sirius_port_autoneg_get(int unit, bcm_port_t port, int *autoneg);

extern int
bcm_sirius_port_autoneg_set(int unit, bcm_port_t port, int autoneg);

extern int
bcm_sirius_port_ability_get(int unit, bcm_port_t port,
                            bcm_port_abil_t *ability_mask);

extern int
bcm_sirius_port_congestion_config_set(int unit, bcm_gport_t port,
                                           bcm_port_congestion_config_t *config, void *info);

extern int
bcm_sirius_port_congestion_config_get(int unit, bcm_gport_t port,
                                           bcm_port_congestion_config_t *config, void *info);

extern int
bcm_sirius_port_pause_get(int unit, bcm_port_t port, int *pause_tx, int *pause_rx);

extern int
bcm_sirius_port_pause_set(int unit, bcm_port_t port, int pause_tx, int pause_rx);

extern int
bcm_sirius_port_update(int unit, bcm_port_t port, int new_link);

extern int
bcm_sirius_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp);

extern int
bcm_sirius_port_scheduler_get(int unit, bcm_gport_t gport,
			      int *scheduler_level, int *scheduler_node);

extern int
bcm_sirius_port_is_egress_multicast(int unit,
                                    bcm_gport_t gport,
                                    bcm_cos_queue_t cosq,
                                    int *is_multicast);

extern int
bcm_sirius_port_egress_multicast_scheduler_get(int unit,
                                               bcm_gport_t gport,
                                               bcm_cos_queue_t cosq,
                                               int *scheduler_level,
                                               int *nbr_scheduler_nodes,
                                               int *scheduler_nodes);

extern int
bcm_sirius_port_egress_multicast_group_get(int unit,
                                           bcm_gport_t gport,
                                           bcm_cos_queue_t cosq,
                                           int *nbr_fifos,
                                           int *nbr_egress_groups,
                                           int *egress_groups);

extern int
_bcm_switch_stat_threshold_set(int unit, int32 threshold);

extern int
_bcm_switch_stat_threshold_get(int unit, int32 *threshold);

extern int
bcm_sirius_switch_control_set(int unit, bcm_switch_control_t type, int val);

extern int
bcm_sirius_switch_control_get(int unit, bcm_switch_control_t type, int *val);

extern int
bcm_sirius_trunk_init(int unit);

extern int
bcm_sirius_trunk_create(int unit, bcm_trunk_t *tid);

extern int
bcm_sirius_trunk_create_id(int unit, bcm_trunk_t tid);

extern int
bcm_sirius_trunk_destroy(int unit, bcm_trunk_t tid);

extern int
bcm_sirius_trunk_detach(int unit);

extern int
bcm_sirius_trunk_find(int unit, bcm_module_t modid, bcm_port_t port, bcm_trunk_t *tid);

extern int
bcm_sirius_trunk_get(int unit,
                     bcm_trunk_t tid,
                     bcm_trunk_info_t *t_data,
                     int member_max,
                     bcm_trunk_member_t *member_array,
                     int *member_count);

extern int
bcm_sirius_trunk_chip_info_get(int unit,
                               bcm_trunk_chip_info_t *ta_info);

extern int
bcm_sirius_trunk_set(int unit,
                     bcm_trunk_t tid,
                     bcm_trunk_info_t *trunk_info,
                     int member_count,
                     bcm_trunk_member_t *member_array);

extern int
bcm_sirius_multicast_source_knockout_get(int unit, bcm_multicast_t group, int *source_knockout);

extern int
bcm_sirius_multicast_source_knockout_set(int unit, bcm_multicast_t group, int source_knockout);

/* failover functions */
extern int
bcm_sirius_failover_enable(int unit, int sysport, int node, int port, int old_node, int old_port);

extern int
bcm_sirius_failover_set(int unit,
                        int sysport,
                        int protect_node,
                        int protect_port,
                        int active_node,
                        int active_port);
extern int
bcm_sirius_failover_destroy(int unit,
			    int sysport);

extern int
bcm_sirius_slq_stats(uint32 unit,
                     int queue_base,
                     bcm_cos_queue_t cosq,
                     sbBool_t enable);

extern int
bcm_sirius_scheduler_init(int unit,
			  int create_ports);

extern int
bcm_sirius_cosq_create_egress_internal_port(int unit,
					    bcm_gport_t physical_gport,
                                            bcm_sirius_cosq_internal_port_class_t type,
					    int num_fifos,
                                            bcm_gport_t original_subport,
					    int *internal_port);

extern int
bcm_sirius_cosq_destroy_egress_internal_port(int unit,
                                             bcm_gport_t physical_gport,
                                             int internal_port);

extern int 
bcm_sirius_cosq_subscriber_map_add(int unit, 
				bcm_cosq_subscriber_map_t *map);

extern int 
bcm_sirius_cosq_subscriber_map_delete(int unit, 
				   bcm_cosq_subscriber_map_t *map);

extern int 
bcm_sirius_cosq_subscriber_map_delete_all(int unit);

extern int 
bcm_sirius_cosq_subscriber_map_get(int unit, 
				bcm_cosq_subscriber_map_t *map);

extern int 
bcm_sirius_cosq_subscriber_traverse(int unit, 
				    bcm_cosq_subscriber_map_traverse_cb cb, 
				    void *user_data);

extern int
bcm_sirius_cosq_scheduler_free(int unit, int level, int node, int egress);

extern int
bcm_sirius_cosq_scheduler_init(int unit, int level, int node, int egress);

extern int
bcm_sirius_cosq_scheduler_allocate(int unit, int level, int node_requested,
                                   int num_nodes, int egress, int *p_node);

extern int
bcm_sirius_cosq_scheduler_connect(int unit, int parent_level, int parent_node,
                                  int child_level, int child_node, int queue, int egress, int fifo_num);

extern int
bcm_sirius_cosq_scheduler_disconnect(int unit, int parent_level, int parent_node,
                                     int child_level, int child_node,
                                     int queue, int egress, int *discon_child);


extern int
bcm_sirius_cosq_gport_queue_attach(int unit, uint32 flags, bcm_gport_t ingress_queue,
                                   bcm_cos_t ingress_int_pri, bcm_gport_t egress_queue,
                                   bcm_cos_t egress_int_pri, int *attach_id);

extern int
bcm_sirius_cosq_gport_queue_attach_get(int unit, bcm_gport_t ingress_queue,
                                       bcm_cos_t ingress_int_pri, bcm_gport_t *egress_queue,
                                       bcm_cos_t *egress_int_pri, int attach_id);

extern int bcm_sirius_cosq_gport_queue_detach(int unit, bcm_gport_t ingress_queue,
                                              bcm_cos_t ingress_int_pri, int attach_id);

extern int
bcm_sirius_cosq_mapping_set(int unit, bcm_cos_t priority, bcm_cos_queue_t cosq);

extern int
bcm_sirius_cosq_mapping_get(int unit, bcm_cos_t priority, bcm_cos_queue_t *cosq);


/* frame steering functions */
extern int
bcm_sirius_stk_steering_unicast_set(int unit,
                                    int steer_id,
                                    bcm_module_t destmod_lo,
                                    bcm_module_t destmod_hi,
                                    int num_queue_groups,
                                    bcm_gport_t *queue_groups);

extern int
bcm_sirius_stk_steering_multicast_set(int unit,
                                      int steer_id,
                                      bcm_multicast_t mgid_lo,
                                      bcm_multicast_t mgid_hi,
                                      int num_queue_groups,
                                      bcm_gport_t *queue_groups);

extern int
bcm_sirius_stk_steering_clear(int unit,
                              int steer_id);

extern int
bcm_sirius_stk_steering_clear_all(int unit);

/* predicate control */
extern int
bcm_sirius_fabric_predicate_create(int unit,
                                   bcm_fabric_predicate_info_t *pred_info,
                                   bcm_fabric_predicate_t *pred_id);

extern int
bcm_sirius_fabric_predicate_destroy(int unit,
                                    bcm_fabric_predicate_t pred_id);

extern int
bcm_sirius_fabric_predicate_destroy_all(int unit);

extern int
bcm_sirius_fabric_predicate_get(int unit,
                                bcm_fabric_predicate_t pred_id,
                                bcm_fabric_predicate_info_t *pred_info);

extern int
bcm_sirius_fabric_predicate_traverse(int unit,
                                     bcm_fabric_predicate_traverse_cb cb,
                                     void *user_data);

/* parser (action) control */
extern int
bcm_sirius_fabric_action_create(int unit,
                                bcm_fabric_action_info_t *action_info,
                                bcm_fabric_action_t *action_id);

extern int
bcm_sirius_fabric_action_destroy(int unit,
                                 bcm_fabric_action_t action_id);

extern int
bcm_sirius_fabric_action_destroy_all(int unit);

extern int
bcm_sirius_fabric_action_get(int unit,
                             bcm_fabric_action_t action_id,
                             bcm_fabric_action_info_t *action_info);

extern int
bcm_sirius_fabric_action_traverse(int unit,
                                  bcm_fabric_action_traverse_cb cb,
                                  void *user_data);
/* type resolution (predicate_action) control */
extern int
bcm_sirius_fabric_predicate_action_create(int unit,
                                          bcm_fabric_predicate_action_info_t *predicate_action,
                                          bcm_fabric_predicate_action_t *predicate_action_id);

extern int
bcm_sirius_fabric_predicate_action_get(int unit,
                                       bcm_fabric_predicate_action_t predicate_action_id,
                                       bcm_fabric_predicate_action_info_t *predicate_action_info);

extern int
bcm_sirius_fabric_predicate_action_destroy(int unit,
                                           bcm_fabric_predicate_action_t predicate_action_id);

extern int
bcm_sirius_fabric_predicate_action_destroy_all(int unit);

extern int
bcm_sirius_fabric_predicate_action_traverse(int unit,
                                            bcm_fabric_predicate_action_traverse_cb cb,
                                            void *user_data);

/* QUEUE_MAP (qsel) control */
extern int
bcm_sirius_fabric_qsel_create(int unit,
                              uint32 flags,
                              int base,
                              int count,
                              bcm_fabric_qsel_t *qsel_id);

extern int
bcm_sirius_fabric_qsel_destroy(int unit,
                               bcm_fabric_qsel_t qsel_id);

extern int
bcm_sirius_fabric_qsel_destroy_all(int unit);

extern int
bcm_sirius_fabric_qsel_get(int unit,
                           bcm_fabric_qsel_t qsel_id,
                           uint32 *flags,
                           int *base,
                           int *count);

extern int
bcm_sirius_fabric_qsel_traverse(int unit,
                                bcm_fabric_qsel_traverse_cb cb,
                                void *user_data);

extern int
bcm_sirius_fabric_qsel_entry_set(int unit,
                                 bcm_fabric_qsel_t qsel_id,
                                 int offset,
                                 bcm_gport_t queue,
                                 bcm_fabric_qsel_offset_t qsel_offset_id);

extern int
bcm_sirius_fabric_qsel_entry_get(int unit,
                                 bcm_fabric_qsel_t qsel_id,
                                 int offset,
                                 bcm_gport_t *queue,
                                 bcm_fabric_qsel_offset_t *qsel_offset_id);

extern int
bcm_sirius_fabric_qsel_entry_multi_set(int unit,
                                       bcm_fabric_qsel_t qsel_id,
                                       int offset,
                                       int count,
                                       bcm_gport_t *queue,
                                       bcm_fabric_qsel_offset_t *qsel_offset_id);

extern int
bcm_sirius_fabric_qsel_entry_multi_get(int unit,
                                       bcm_fabric_qsel_t qsel_id,
                                       int offset,
                                       int count,
                                       bcm_gport_t *queue,
                                       bcm_fabric_qsel_offset_t *qsel_offset_id);

extern int
bcm_sirius_fabric_qsel_entry_traverse(int unit,
                                      bcm_fabric_qsel_t qsel_id,
                                      bcm_fabric_qsel_entry_traverse_cb cb,
                                      void *user_data);

/* This function tries to translate a queue into an appropriate GPORT */
extern int
_bcm_sirius_fabric_qsel_entry_gport_extrapolate(int unit,
                                                unsigned int queue,
                                                bcm_gport_t *gport);

/* COS_MAP (qsel_offset) control */
extern int
bcm_sirius_fabric_qsel_offset_create(int unit,
                                     uint32 flags,
                                     bcm_fabric_qsel_offset_t *qsel_offset_id);

extern int
bcm_sirius_fabric_qsel_offset_destroy(int unit,
                                      bcm_fabric_qsel_offset_t qsel_offset_id);

extern int
bcm_sirius_fabric_qsel_offset_destroy_all(int unit);

extern int
bcm_sirius_fabric_qsel_offset_traverse(int unit,
                                       bcm_fabric_qsel_offset_traverse_cb cb,
                                       void *user_data);

extern int
bcm_sirius_fabric_qsel_offset_entry_set(int unit,
                                        bcm_fabric_qsel_offset_t qsel_offset_id,
                                        bcm_cos_t int_pri,
                                        int offset);

extern int
bcm_sirius_fabric_qsel_offset_entry_get(int unit,
                                        bcm_fabric_qsel_offset_t qsel_offset_id,
                                        bcm_cos_t int_pri,
                                        int *offset);

extern int
bcm_sirius_fabric_qsel_offset_entry_traverse(int unit,
                                             bcm_fabric_qsel_offset_t qsel_offset_id,
                                             bcm_fabric_qsel_offset_entry_traverse_cb cb,
                                             void *user_data);


/* Flow Control */
extern int bcm_sirius_fd_fct_get(int unit, int32 sysport, int32 ef, int32 mc, int32 *fct);

/*
 *  Certain special functions in the Sirius multicast implementation...
 */
extern int _bcm_sirius_multicast_oitt_enable_set(int unit, int newstate);
extern int _bcm_sirius_multicast_defrag(int unit, int newstate);
extern int _bcm_sirius_multicast_dump(int unit,
                                      unsigned int level,
                                      unsigned int first,
                                      unsigned int last);
extern int bcm_sirius_multicast_init(int unit);
extern int bcm_sirius_multicast_detach(int unit);
extern int bcm_sirius_multicast_group_get(int unit,
                                          bcm_multicast_t group,
                                          uint32 *flags);
extern int bcm_sirius_multicast_group_traverse(int unit,
                                               bcm_multicast_group_traverse_cb_t cb,
                                               uint32 flags,
                                               void *user_data);

extern int soc_sirius_hw_xgs_remote_replication_range_set(int unit,
                                                          int first,
                                                          int last);

/* multipath shapers */
extern int bcm_sirius_cosq_multipath_allocate(int unit,
					      int egress,
					      int *p_multipath);

extern int bcm_sirius_cosq_multipath_free(int unit,
					  int egress,
					  int multipath);

extern int bcm_sirius_cosq_multipath_add(int unit,
					 int egress,
					 int multipath,
					 int level,
					 int node);

extern int bcm_sirius_cosq_multipath_delete(int unit,
					    int egress,
					    int multipath,
					    int level,
					    int node);

extern int bcm_sirius_cosq_multipath_get(int unit, int egress, int multipath,
					 int *member_count,
					 bcm_gport_t *member_gport_array,
					 int *member_gport_info_array);


#ifdef BCM_WARM_BOOT_SUPPORT

extern int bcm_sirius_wb_cosq_state_sync(int unit,
					 uint16 default_ver,
					 uint16 recovered_ver,
					 uint32 *tmp_len,
					 uint8 **ptr,
					 uint8 **eptr,
					 int operation);

extern int _bcm_sirius_wb_port_state_sync(int unit,
                                          uint32 *tmp_len,
                                          uint8 **ptr,
                                          uint8 **eptr,
                                          int operation);

extern int bcm_sirius_wb_trunk_state_sync(int unit,
					  uint16 default_ver,
					  uint16 recovered_ver,
					  uint32 *tmp_len,
					  uint8 **ptr,
					  uint8 **eptr,
					  int operation);

extern int bcm_sirius_wb_multicast_state_sync(int unit,
					      uint16 default_ver,
					      uint16 recovered_ver,
					      uint32 *tmp_len,
					      uint8 **ptr,
                                              uint8 **eptr,
					      int operation);

#endif /* BCM_WARM_BOOT_SUPPORT */

extern int
bcm_sirius_mirror_port_set(int unit,
                           bcm_port_t port,
                           bcm_module_t dest_mod,
                           bcm_port_t dest_port,
                           uint32 flags);

extern int
bcm_sirius_mirror_port_get(int unit,
                           bcm_port_t port,
                           bcm_module_t *dest_mod,
                           bcm_port_t *dest_port,
                           uint32 *flags);

#endif /* _BCM_INT_SBX_SIRIUS_H_ */
