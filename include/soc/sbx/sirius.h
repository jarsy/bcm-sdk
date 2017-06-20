/*
 * $Id: sirius.h,v 1.98 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SIRIUS (BCM83200) SOC Initialization
 */
#ifndef _SIRIUS_H
#define _SIRIUS_H

#include "sbTypes.h"
#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/sbx_drv.h>

#include <bcm/types.h>
#include <bcm/error.h>

extern soc_error_t
soc_sirius_modid_set(int unit, int node);

extern sbBool_t
sirius_reg_done_timeout(int unit, uint32 reg, uint32 port, uint32 index, uint32 donefield, uint32 timeout);

extern int
phy_reg_ci_read(int unit, uint32 ci, uint32 reg_addr, uint32 *data);

extern int
phy_reg_ci_write(int unit, uint32 ci, uint32 reg_addr, uint32 data);

extern int
phy_reg_ci_modify(int unit, uint32 ci, uint32 reg_addr, uint32 data, uint32 mask);

extern int
soc_sirius_ts_node_state_init(int unit, int32 level, int32 node);

extern int
soc_sirius_ts_node_multipath_config(int unit, int level, uint32 node,
				  int32 next_node, int32 last);

extern int
soc_sirius_ts_node_hierachy_config(int unit, uint32 level, uint32 node, int32 parent,
                                   int32 first_child, int32 num_child);

extern int
soc_sirius_ts_node_hierachy_config_get(int unit, uint32 level, uint32 node, int32 *parent,
				       int32 *first_child, int32 *num_child);

extern int
soc_sirius_ts_node_shaper_config(int unit, uint32 level, uint32 node, int32 enable,
                                 int32 shape_rate, int32 max_burst);

extern int
soc_sirius_ts_node_creditor_config(int unit, uint32 level, uint32 node, int32 enable,
                                   int32 creditor_rate, int32 profile_id);

extern int
soc_sirius_ts_node_profile_config(int unit, int level, int index, int creditor_state_map, int adopt_pri_map,
                                  int pri2, int pri1, int pri0, int mult2, int mult1);

extern int
soc_sirius_ts_node_child_weight_config(int unit, uint32 level, uint32 node, int32 child_offset,
                                       int32 weight);

extern int
soc_sirius_ts_node_mapping_config(int unit, int level, uint32 node, int32 node_type, int32 bucket_type,
                                  int32 bucket_table_pointer);

extern int
soc_sirius_ts_node_mapping_config_get(int unit, int level, int node, int32 *node_type,
				      int32 *bucket_type, int32 *bucket_table_pointer);

extern int
soc_sirius_ts_level_config(int unit, int level, int bypass, int num_node, int leak_cycle,
                           int aperiodic_interval);

extern int
soc_sirius_ts_bucket_config(int unit, int level, uint32 node, int32 bucket_type,
                            int32 bucket_table_pointer);

extern int
soc_sirius_ts_leaf_node_dest_fifo_set(int unit, uint32 leaf_node, int32 fifo);

extern int
soc_sirius_ts_fullmap_config(int unit, int full, int level, int node);

extern int
soc_sirius_ts_child_weight_update(int unit, int level, int node, 
				  int num_child, int op,
				  int target_child1, int target_child2);
extern int
soc_sirius_ts_node_clone(int unit, int level, int orig_node, int dest_node);

extern int
soc_sirius_ts_node_get_pri(int unit, int level, int node, uint32 *pri);

extern int
soc_sirius_ts_node_regenerate_pri(int unit, int level, int node, uint32 *new_pri, uint32 bubble_up);

extern int
soc_sirius_ts_node_update_child_pri(int unit, int level, int node, int child,
				    uint32 child_pri, uint32 *new_pri);

extern int
soc_sirius_qs_queue_to_leaf_node_set(int unit, uint32 queue, int32 mc, int32 leaf_node);

extern int
soc_sirius_qs_queue_to_leaf_node_get(int unit, uint32 queue, int32 *mc, int32 *leaf_node);

extern int
soc_sirius_qs_queue_to_sysport_cos_set(int unit, uint32 queue, int32 mc, int32 sysport, int32 cos);

extern int
soc_sirius_qs_queue_to_sysport_cos_get(int unit, uint32 queue, int32 *mc, int32 *sysport, int32 *cos);

extern int
soc_sirius_qs_sysport_to_node_set(int unit, int32 sysport, int32 node);

extern int
soc_sirius_qs_sysport_to_queue_set(int unit, int32 mc, int32 sysport, int32 base_queue);

extern int
soc_sirius_qs_leaf_node_to_queue_set(int unit, uint32 leaf_node, uint32 queue);

extern int
soc_sirius_qs_leaf_node_to_queue_get(int unit, uint32 leaf_node, uint32 *queue);

extern int
soc_sirius_qs_baa_rate_set(int unit, uint32 queue, uint32 mant, uint32 exp);

extern int
soc_sirius_es_node_shaper_config(int unit, int level, int node, int min, int enable,
                                 int rate, int threshold, int tick_select);

extern int
soc_sirius_es_node_hierachy_config(int unit, int level, int node, int enable, int intf,
                                   int channel, int subport, int ef);
extern int
soc_sirius_es_node_hierachy_config_get(int unit, int level, int node, int* enable,
				       int *intf, int *channel, int *subport,
				       int *index, int *ef);
extern int
soc_sirius_es_node_scheduler_config(int unit, int level, int node, int min,
                                    int mode, int param);
extern int
soc_sirius_es_root_scheduler_config(int unit);

extern int
soc_sirius_es_node_get_info(int unit, int level, int node, int *parent, int *index);

extern int
soc_sirius_es_node_group_shaper_member_config(int unit, int group, int node, int enable);

extern int
soc_sirius_es_node_map_index_first_available(int unit, int level, int node, int *index);

extern int
soc_sirius_es_fc_base_table_set(int unit, int module, int max_ports, int base, int is_enable);

extern int
soc_sirius_es_fc_base_table_get(int unit, int module, int *max_ports, int *base, int *is_enable);

extern int
soc_sirius_es_fc_map_table_set(int unit, int port, int fifo_base, int fc_state_index);

extern int
soc_sirius_es_fc_map_table_get(int unit, int port, int fifo_base, int *fc_state_index);

extern int
soc_sirius_es_fc_mc_merge_set(int unit, int mc_merge_enable);

extern int
soc_sirius_es_fc_mc_merge_get(int unit, int *mc_merge_enable);

extern int
soc_sirius_scheduler_init(int unit, int level, int node, int egress);

extern int
soc_sirius_qm_buffer_template_config(int unit, uint32 uMinBufferingInKbytes, uint32 uMaxBufferinginKbytes, uint32 *pTemplateId);

extern int
soc_sirius_rb_higig2_remote_cpu_config(int unit, int32 destMod, int32 destPort, int32 nQueue);

extern int
soc_sirius_rb_higig2_header_unicast_queue_map_config(int unit, uint32 uDestNode, uint32 uDestPort, int32 nQueue);

extern int
soc_sirius_rb_higig2_header_multicast_queue_map_config(int unit, int32 nMcGroup, int32 nQueue);

extern int
soc_sirius_rb_higig2_header_multicast_queue_map_read(int unit, int32 nMcGroup, int32 *nQueue);

extern int
soc_sirius_qs_queue_parameter_set(int unit, int32 nQueue, int32 q_type, int32 hold_ts);

extern int
soc_sirius_qs_queue_parameter_get(int unit, int32 nQueue, int32 *q_type, int32 *hold_ts);

extern int
_soc_sirius_hw_init_qs_shaper(int unit, int32 nMaxVoq);

extern int
soc_sirius_ability_matching_speed_set(int unit, int32 nSi, int ability);

extern int
soc_sirius_hypercore_loopback_set(int unit, int32 uPhyAddr, int32 uLaneAddr, int32 bEnable);

extern int
soc_sirius_hypercore_loopback_get(int unit, int32 uPhyAddr, int32 uLaneAddr, int32 *pEnable);

extern int
soc_sirius_hypercore_8b10b_set(int unit, int32 uPhyAddr, int32 uLaneAddr, int32 bEnable);

extern int
soc_sirius_serdes_phyaddr(int unit, int32 nSi, uint32 *uPhyAddr, uint32 *uLaneAddr);

extern int
soc_sirius_mdio_hc_read(int unit, uint32 uPhyAddr, uint32 uLane, uint32 uRegAddr, uint32 *pReadData);

extern int
soc_sirius_mdio_hc_write(int unit, uint32 uPhyAddr, uint32 uLane, uint32 uRegAddr, uint32 uData);

extern int
soc_sirius_config_linkdriver(int unit, int nLink, sbLinkDriverConfig_t *pLinkDriverConfig);

extern int 
soc_sirius_hc_encoding_set(int unit, int32 nSi, int32 bSerdesEncoding);

extern int
soc_sirius_hc_speed_set(int unit, int32 nSi, uint32 uSerdesSpeed);

/* ddr23 register access */
#define SOC_SIRIUS_MAX_NUM_CI_BLKS     (10)

#define DDR23_REG_READ(_unit, _pc, flags, _reg_addr, _val) \
            phy_reg_ci_read((_unit), (_pc), (_reg_addr), (_val))
#define DDR23_REG_WRITE(_unit, _pc, _flags, _reg_addr, _val) \
            phy_reg_ci_write((_unit), (_pc), (_reg_addr), (_val))
#define DDR23_REG_MODIFY(_unit, _pc, _flags, _reg_addr, _val, _mask) \
            phy_reg_ci_modify((_unit), (_pc), (_reg_addr), (_val), (_mask))
#define DDR23_GET_FIELD(m,c,r,f) \
            GET_FIELD(m,c,r,f)
#define DDR23_SET_FIELD(m,c,r,f,d) \
            SET_FIELD(m,c,r,f,d)

#define SOC_SIRIUS_API_PARAM_NO_CHANGE                 (SOC_SBX_API_PARAM_NO_CHANGE)

/* local wred */
#define SB_FAB_DEVICE_SIRIUS_MAX_WRED_TEMPLATES        (512)

/* Node Profile flags */
#define SIRIUS_TS_NODE_PROFILE_ADOPT_ALL               (0)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_15_OR_HIGHER  (1)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_14_OR_HIGHER  (2)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_13_OR_HIGHER  (3)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_12_OR_HIGHER  (4)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_11_OR_HIGHER  (5)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_10_OR_HIGHER  (6)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_9_OR_HIGHER   (7)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_8_OR_HIGHER   (8)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_7_OR_HIGHER   (9)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_6_OR_HIGHER   (10)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_5_OR_HIGHER   (11)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_4_OR_HIGHER   (12)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_3_OR_HIGHER   (13)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_2_OR_HIGHER   (14)
#define SIRIUS_TS_NODE_PROFILE_ADOPT_PRI_1_OR_HIGHER   (15)
#define SIRIUS_TS_NODE_PROFILE_FORCE_PRI_0             (31)

/* predefined Node Profile template ID */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT        (0)  /* template 0, template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_DISABLED       (1)  /* template 0, template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF0            (2)  /* template 0 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF1            (3)  /* template 0 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF2            (4)  /* template 0 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF3            (5)  /* template 0 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP0            (6)  /* template 0, template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP1            (7)  /* template 0, template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_BE             (8)  /* template 0 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP0       (9)  /* template 0, template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP1       (10) /* template 0, template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP2            (12) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP3            (13) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP4            (14) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP5            (15) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP6            (16) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP2       (17) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP3       (18) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP4       (19) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP5       (20) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP6       (21) /* template 1 */
#define SIRIUS_TS_NODE_PROFILE_TEMPLATE_MAX            (31)

#define SIRIUS_TS_NODE_PROFILE_MULTI1_PERCENT          (60)
#define SIRIUS_TS_NODE_PROFILE_MULTI2_PERCENT          (80)

/* Node type */
#define SIRIUS_TS_NODE_TYPE_DEFAULT                    (0)
#define SIRIUS_TS_NODE_TYPE_LEAF_RELAY                 (1)
#define SIRIUS_TS_NODE_TYPE_INNER_RELAY                (2)
#define SIRIUS_TS_NODE_TYPE_ROOT_RELAY                 (3)

/* Bucket type */
#define SIRIUS_TS_BUCKET_TYPE_BUCKET                   (0)
#define SIRIUS_TS_BUCKET_TYPE_MULTIPATH                (1)
#define SIRIUS_TS_BUCKET_TYPE_MULTIPATH_MASTER         (2)
#define SIRIUS_TS_BUCKET_TYPE_SUBTREE                  (3)

#define SIRIUS_TS_LOCAL_SYSPORT_LIMIT                  (4096)
#define SIRIUS_TS_LOCAL_SYSPORT_SIZE                   (512)
#define SIRIUS_TS_LOCAL_SYSPORT_BASE                   (SIRIUS_TS_LOCAL_SYSPORT_LIMIT - SIRIUS_TS_LOCAL_SYSPORT_SIZE)
#define SIRIUS_TS_LOCAL_SYSPORT_MC_OFFSET              (265)

/* group shapers */
#define SIRIUS_TS_LEVEL_GROUP_SHAPER                   (SB_FAB_DEVICE_GROUP_SHAPER_LEVEL)
#define SIRIUS_ES_LEVEL_GROUP_SHAPER                   (SB_FAB_DEVICE_GROUP_SHAPER_LEVEL)
#define SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER  (100)
#define SB_FAB_DEVICE_SIRIUS_MC_EGRESS_GROUP_SHAPER   (0)

/* ingress nodes */
#define SIRIUS_TS_LEVEL_8                              (8)
#define SIRIUS_TS_LEVEL_7                              (7)
#define SIRIUS_TS_LEVEL_6                              (6)
#define SIRIUS_TS_LEVEL_5                              (5)
#define SIRIUS_TS_LEVEL_4                              (4)
#define SIRIUS_TS_LEVEL_3                              (3)
#define SIRIUS_TS_LEVEL_2                              (2)
#define SIRIUS_TS_LEVEL_1                              (1)
#define SIRIUS_TS_LEVEL_0                              (0)
#define SIRIUS_TS_LEVEL_MAX                            (SIRIUS_TS_LEVEL_8)

#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L0  (0x10000)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L1  (0x4000)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L2  (0x1000)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L3  (0x400)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L4  (0x108)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L5  (0x84)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L6  (0x38)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L7  (0x7)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L8  (0x1)
#define SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER (SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L0 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L1 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L2 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L3 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L4 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L5 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L6 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L7 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L8)

/* egress nodes */
#define SIRIUS_ES_LEVEL_ROOT                           (4)
#define SIRIUS_ES_LEVEL_INTERFACE                      (3)
#define SIRIUS_ES_LEVEL_CHANNEL                        (2)
#define SIRIUS_ES_LEVEL_SUBPORT                        (1)
#define SIRIUS_ES_LEVEL_FIFO                           (0)
#define SIRIUS_ES_LEVEL_MAX                            (SIRIUS_ES_LEVEL_ROOT)

#define SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0  (528)
#define SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1  (132)
#define SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2  (132)
#define SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3  (7)
#define SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L4  (1)
#define SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER  (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3 + \
						    SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L4 )
#define SB_FAB_DEVICE_SIRIUS_MAX_SCHEDULER         (SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER + \
						    SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER)

#define SB_FAB_DEVICE_SIRIUS_CPU_INTF   (0)
#define SB_FAB_DEVICE_SIRIUS_HG0_INTF   (1)
#define SB_FAB_DEVICE_SIRIUS_HG1_INTF   (SB_FAB_DEVICE_SIRIUS_HG0_INTF + 1)
#define SB_FAB_DEVICE_SIRIUS_HG2_INTF   (SB_FAB_DEVICE_SIRIUS_HG1_INTF + 1)
#define SB_FAB_DEVICE_SIRIUS_HG3_INTF   (SB_FAB_DEVICE_SIRIUS_HG2_INTF + 1)
#define SB_FAB_DEVICE_SIRIUS_RQ0_INTF   (SB_FAB_DEVICE_SIRIUS_HG0_INTF + SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS)
#define SB_FAB_DEVICE_SIRIUS_RQ1_INTF   (SB_FAB_DEVICE_SIRIUS_RQ0_INTF + 1)
#define SB_FAB_DEVICE_SIRIUS_DUMMY_INTF (SB_FAB_DEVICE_SIRIUS_RQ0_INTF + SB_FAB_DEVICE_SIRIUS_NUM_REQUEUE_PORTS)

/* NOTE: make sure the scheduler NODE is matching the intface ID */
#define SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_CPU_NODE  (SB_FAB_DEVICE_SIRIUS_CPU_INTF)
#define SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_HG0_NODE  (SB_FAB_DEVICE_SIRIUS_HG0_INTF)
#define SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ0_NODE  (SB_FAB_DEVICE_SIRIUS_RQ0_INTF)
#define SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ1_NODE  (SB_FAB_DEVICE_SIRIUS_RQ1_INTF)

#define SB_FAB_DEVICE_SIRIUS_EGRESS_SCHEDULER_CPU_NODE   (SB_FAB_DEVICE_SIRIUS_CPU_INTF)
#define SB_FAB_DEVICE_SIRIUS_EGRESS_SCHEDULER_HG0_NODE   (SB_FAB_DEVICE_SIRIUS_HG0_INTF)
#define SB_FAB_DEVICE_SIRIUS_EGRESS_SCHEDULER_RQ0_NODE   (SB_FAB_DEVICE_SIRIUS_RQ0_INTF)
#define SB_FAB_DEVICE_SIRIUS_EGRESS_SCHEDULER_RQ1_NODE   (SB_FAB_DEVICE_SIRIUS_RQ1_INTF)

#define SB_FAB_DEVICE_SIRIUS_FIFO_EMPTY_DELAY            (1 * 1000 * 1000) /* 1 msec,thin_delay */
#define SB_FAB_DEVICE_SIRIUS_FIFO_EMPTY_ITER_DELAY       (10)              /* 10 usec,sal_udelay */
#define SB_FAB_DEVICE_SIRIUS_FIFO_EMPTY_ITER_COUNT       (1000)

#define SIRIUS_ES_SCHEDULER_MODE_SP                    (1)
#define SIRIUS_ES_SCHEDULER_MODE_WERR                  (0)
#define SIRIUS_ES_SCHEDULER_MODE_EF                    (2)

#define SIRIUS_HCFC_MAX_CHANNEL_BASE                   (0x1F)
#define SIRIUS_HCFC_MAX_CHANNEL_OFFSET                 (95)

#define SIRIUS_HCFC_FC_BASE_INDEX_SET(intf, msg_type, channel_base)	\
      ((((intf) & 0x3)<<6) | (((msg_type) & 0x2)<<4) | ((channel_base) & 0x1F))

#define SIRIUS_HCFC_FC_CHANNEL_BASE_GET(index)         ((index) & 0x1F)


#define SIRIUS_ES_FC_TRANSLATE_TABLE_SIZE              (256)
#define SIRIUS_ES_FC_STATE_TABLE_SIZE                  (132)
#define SIRIUS_FC_NOT_MAPPED_FIFO_GROUP_STATE_TABLE_INDEX  131

#define SIRIUS_FR_FC_MC_DOMAIN_EF                      (264)
#define SIRIUS_FR_FC_MC_DOMAIN_NON_EF                  (265)
#define SIRIUS_FR_FC_MC_DOMAIN                         (SIRIUS_FR_FC_MC_DOMAIN_EF)

#define SIRIUS_Q_BASE_INVALID      0xFFF8
#define SIRIUS_Q_TYPE_BE           0
#define SIRIUS_Q_TYPE_AF           1
#define SIRIUS_Q_TYPE_EF           2
#define SIRIUS_Q_TYPE_TDM_CALENDAR 3
#define SIRIUS_Q_TYPE_SP0          8
#define SIRIUS_Q_TYPE_SP1          9
#define SIRIUS_Q_TYPE_SP2          10
#define SIRIUS_Q_TYPE_SP3          11
#define SIRIUS_Q_TYPE_SP4          12
#define SIRIUS_Q_TYPE_SP5          13
#define SIRIUS_Q_TYPE_SP6          14
#define SIRIUS_Q_TYPE_SP7          15
#define SIRIUS_Q_TYPE_LOCAL_BE    (SIRIUS_Q_TYPE_BE)
#define SIRIUS_Q_TYPE_LOCAL_EF     16
#define SIRIUS_Q_TYPE_LOCAL_SP0    17
/* #define SIRIUS_Q_TYPE_LOCAL_SP1    18, queue type 18 is reserved by hardware for other purpose */
#define SIRIUS_Q_TYPE_LOCAL_SP1    31
#define SIRIUS_Q_TYPE_LOCAL_SP2    19
#define SIRIUS_Q_TYPE_LOCAL_SP3    20
#define SIRIUS_Q_TYPE_LOCAL_SP4    21
#define SIRIUS_Q_TYPE_LOCAL_SP5    22
#define SIRIUS_Q_TYPE_LOCAL_SP6    23
#define SIRIUS_Q_TYPE_LOCAL_SP7    24
#define SIRIUS_Q_TYPE_LOCAL_AF0    25
#define SIRIUS_Q_TYPE_LOCAL_AF1    26
#define SIRIUS_Q_TYPE_LOCAL_AF2    27
#define SIRIUS_Q_TYPE_LOCAL_AF3    28
#define SIRIUS_Q_TYPE_LOCAL_RCPQ0  29
#define SIRIUS_Q_TYPE_LOCAL_RCPQ1  30

/*
 * following are queue types for template-2. There are limited number of queue types
 * and thus they are reused across templates. For template-2 it is possible to minimize
 * the overlap by using the unused queue types in the FIC range. 
 */
#define SIRIUS_Q_TYPE_LOCAL_RCPQ2  SIRIUS_Q_TYPE_LOCAL_SP7
#define SIRIUS_Q_TYPE_LOCAL_RCPQ3  SIRIUS_Q_TYPE_LOCAL_AF0
#define SIRIUS_Q_TYPE_LOCAL_RCPQ4  SIRIUS_Q_TYPE_LOCAL_AF1
#define SIRIUS_Q_TYPE_LOCAL_RCPQ5  SIRIUS_Q_TYPE_LOCAL_AF2
#define SIRIUS_Q_TYPE_LOCAL_RCPQ6  SIRIUS_Q_TYPE_LOCAL_AF3

/*
 * following are queue types for template-4 only. It's assumed that all queue types
 * are used by the local queues, so there is no queue type left for fic queues.
 * also queue type with _NOANEMIC will treat anemic pri to be 0
 */
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_EF     1 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP0    2 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP1    3 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP2    4 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP3    5 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP4    6 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP5    7 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP6    8 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ0  9 
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ1  10
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ2  11
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ3  12
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ4  13
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ5  14
#define SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ6  15





#define SIRIUS_HYPERCORE_8017_8B10B_SHIFT           12
#define SIRIUS_HYPERCORE_8017_KCHAR_SHIFT            8
#define SIRIUS_HYPERCORE_8019_PRBS_INVERT_SHIFT      2
#define SIRIUS_HYPERCORE_8019_PRBS_ENABLE_SHIFT      3
#define SIRIUS_HYPERCORE_8019_PRBS_POLY_SHIFT        0

#define SIRIUS_HYPERCORE_8019_LANE_SHIFT             4
#define SIRIUS_HYPERCORE_80B1_STATUS_MASK            7
#define SIRIUS_HYPERCORE_80B1_PRBS_STATUS_VALUE      7
#define SIRIUS_HYPERCORE_80B1_SIGDET_STATUS_VALUE    0

#define SIRIUS_HYPERCORE_8065_TX_SEL_HALF_RATE_MASK  1
#define SIRIUS_HYPERCORE_8065_TX_SEL_HALF_RATE_SHIFT 3

#define SIRIUS_HYPERCORE_8000_PLL_SEQUENCER_MASK    0x2000
#define SIRIUS_HYPERCORE_833C_8B10B_SCRAMBLER_MASK  0x0100

#define SS_SI_PER_HC (4)
/*  Hypercore Lane Mode values */
#define SS_HC_LANE_MODE_HALF_SPEED 0
#define SS_HC_LANE_MODE_FULL_SPEED 1

/* Loopback register offset */
#define HW_SIRIUS_HYPERCORE_XGXSBLK1_LANECTRL2_OFFSET             0x08017
#define HW_SIRIUS_HYPERCORE_SERDESDIGITAL_MISC_CONTROL2_OFFSET    0x08309
#define HW_SIRIUS_HYPERCORE_AER_LANE_SELECT                       0x0ffde

/*
 * Ingress Shaper definitions
 */
#define SIRIUS_MAX_SHAPER_QUEUES                                  (16 * 1024)

#define SIRIUS_INGRESS_SHAPER_MIN_EXPONENT                        0
#define SIRIUS_INGRESS_SHAPER_MIN_MANTISSA                        1
#define SIRIUS_INGRESS_SHAPER_MAX_EXPONENT                        15
#define SIRIUS_INGRESS_SHAPER_MAX_MANTISSA                        255

#define SIRIUS_INGRESS_SHAPER_CLOCK_CYCLE_SCALER                  (10000)
#define SIRIUS_INGRESS_SHAPER_CLOCK_CYCLE_SCALER2                 (10)
#define SIRIUS_INGRESS_SHAPER_CLOCK_CYCLE_DEN_MHZ                 (1000000)

#define SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE                      (255 * 32768)

#define SIRIUS_CLOCK_CYCLE_SCALER2                            (10)
#define SIRIUS_CLOCK_CYCLE_SCALER                             (1000 * SIRIUS_CLOCK_CYCLE_SCALER2)
#define SIRIUS_CLOCK_CYCLE_AGER_TICK                          (3200)
#define SIRIUS_CLOCK_CYCLE_BUFFER_AGER_FACTOR                 (96 * 32 * 1000 * 6)



#define SIRIUS_MAX_QBUF_TEMPLATES                                 (512)

#define SIRIUS_NUM_SFI_CHANNELS     (46)
#define SIRIUS_MAX_NODE_TYPES       (4)
#define SIRIUS_NODE_TYPE_FIC        (0)
#define SIRIUS_NODE_TYPE_QE2K       (1)
#define SIRIUS_NODE_TYPE_HYBRID     (2)
#define SIRIUS_NODE_TYPE_RESERVED   (3)

/* Sirius max burst size (num channels) based on Node type */
#define SIRIUS_BURST_SIZE_FIC       (46)
#define SIRIUS_BURST_SIZE_QE2K      (18)
#define SIRIUS_BURST_SIZE_HYBRID    (22)

#define SIRIUS_RATE_CLOCK                                         (405)     /* in MHz */
#define SIRIUS_MAX_CLOCKS_PER_EPOCH                               (0x7FFFFF)

typedef struct siriusInitParamsRb_s {
  sbBool_t bBringUp;
} siriusInitParamsRb_t;

typedef struct siriusInitParamsQm_s {
  sbBool_t bBringUp;
} siriusInitParamsQm_t;

typedef struct siriusInitParamsQs_s {
  sbBool_t bBringUp;
} siriusInitParamsQs_t;

typedef struct siriusInitParamsTx_s {
  sbBool_t bBringUp;
} siriusInitParamsTx_t;

/* DDR23 phy parameters */
typedef struct siriusDDR23PhyParams_s {
  uint32 slew;                      /* driver slew rate control (0-Fast,1-Reduced) */
  uint32 seltxdrv_ci;               /* driver output strength (0-full,1-half) */
  uint32 rt60b;                     /* ODT (0=60 Ohm, 1=120 Ohm) */
  /* others tbd */
} siriusDDR23PhyParams_t;

typedef int (*siriusClearDdrFunc_t)(int unit);


typedef struct siriusInitParamsCi_s {
  sbBool_t bBringUp;
  sbBool_t bHwRunTimeDDRCalibration;
  uint32 rrd;                      /* Ras to Ras Delay setting */
  uint32 ccd;                      /* Cas to Cas Delay setting */
  uint32 wr_latency;               /* DDR controller write latency (in cycles) */
  uint32 wr_recovery;              /* Write Recovery */
  uint32 rc;                       /* Same bank to bank activate timer */
  uint32 mrdtmod;                  /* mode register set timer */
  uint32 wb_full_threshold;        /* Write buffer full threshold */
  uint32 wb_congestion_threshold;  /* Write buffer congestion threshold */
  uint32 rfc;                      /* refresh timer */
  uint32 faw;                      /* Window size in DDR clocks for max number of RASs */
  uint32 al;                       /* Additive latency */
  uint32 rtw;                      /* Read to Write collision timer */
  uint32 wtr;                      /* Write to Read collision timer */
  uint32 read_enb;                 /* Number of DDR clocks from CAS to read data from phy available */
  uint32 odt_enable;               /* enables ODT active during reads */
  uint32 idle_enable;              /* Enables IDLE pin to phy to place phy in low power mode */
  uint32 rr_bank_search;           /* When set arbiter disregards fifo fill level when selecting next
                                      bank to service, and uses round robin */
  uint32 bank_wr_unavailable;      /* Active to Active for same bank */
  uint32 arb_refresh_delay;        /* Arbiter will wait this number of core clocks after refresh,
                                    * before selecting nex memory access */
  uint32 min_first_read_delay;     /* Arb guarantees that this number of core clocks has elapsed
                                      from the first read request of a timeslot before issuing
                                      the read to memory. */
  uint32 write_to_read_delay;       /* Write to read minimum delay */
  uint32 refresh_interval;          /* The number of 400 MHz clock cycles between refresh requests
                                       issued to the DDR controller. Refreshes are scheduled
                                       after dequeues, when the dequeue FIFO is empty */
  uint32 trp;                       /* precharge timer */
  uint32 cke_init_count;            /* DDR controller reset to CKE first active timer */
  uint32 mem_grade;                 /* memory grade, 
				     *  0: 9-9-9
				     *  1: 7-7-7
				     *  2: 10-10-10
				     */

  /* allow the option to adjust phy settings per ci block
   * this allows for slighty different phy settings for a CI block
   * if board requires it.
   */
  siriusDDR23PhyParams_t ddr23PhyParams;
} siriusInitParamsCi_t;

typedef struct siriusInitParamsSc_s {
  sbBool_t bBringUp;
  uint32   uDefaultBmId;
  uint32   uSerdesSpeed;
  sbBool_t bSerdesEncoding;
  uint32 uLinkEnRemap[SB_FAB_DEVICE_SIRIUS_LINKS];
  uint32 uLinkStatusRemap[SB_FAB_DEVICE_SIRIUS_INTERNAL_LINKS];
  uint32 uJitTolerance[SB_FAB_DEVICE_SIRIUS_LINKS];
  uint32 bEvenChannelOn[SB_FAB_DEVICE_SIRIUS_LINKS];
  uint32 bOddChannelOn[SB_FAB_DEVICE_SIRIUS_LINKS];
  uint32 uQuadDone;

} siriusInitParamsSc_t;

typedef struct siriusInitParamsEg_s {
  sbBool_t bBringUp;
} siriusInitParamsEg_t;

typedef struct siriusInitParamsSf_s {
  sbBool_t bBringUp;
  sbBool_t bTmeOnly;
  uint32 uSfiPortRemap[SB_FAB_DEVICE_SIRIUS_SFI_PORTS];
  uint32 uSfiPortLoopback[SB_FAB_DEVICE_SIRIUS_SFI_PORTS];
  uint32 uSfiPortBackpressure[SB_FAB_DEVICE_SIRIUS_SFI_PORTS];
  uint32 uSfiPortIdleFreq[SB_FAB_DEVICE_SIRIUS_SFI_PORTS];
  uint32 uSfiPortEnable[SB_FAB_DEVICE_SIRIUS_SFI_PORTS];
} siriusInitParamsSf_t;

typedef struct siriusInitParamsCs_s {
  sbBool_t bBringUp;
} siriusInitParamsCs_t;

typedef struct siriusInitParamsTs_s {
  sbBool_t bBringUp;
  sbBool_t bEgressFifoIndependentFlowControl;
  sbBool_t b8kNodes;                 /* 8k level 1 nodes */
  sbBool_t bDualLocalGrants;         /* dual local grants */
  int      nSysportBase;             /* sysport base for local queues */
  uint32   uLeafBgPeriod;            /* leaf backgroupd task period */
  sbBool_t bBypassLevel[SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS];            /* bypass level */
  uint32   uLeakCycle[SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS];              /* periodic update interval */
  uint32   uNumTsNode[SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS];                /* number of nodes on each level */
  uint32   uAperiodicInterval[SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS];      /* aperiodic update interval */
  uint32   uInterfacePlane[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES];  /* interface planes */
  uint32   uInterfaceWeight[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES]; /* interface planes */
} siriusInitParamsTs_t;

typedef struct siriusInitParamsFr_s {
  sbBool_t bBringUp;
} siriusInitParamsFr_t;

typedef struct siriusInitParamsEb_s {
  sbBool_t bBringUp;
} siriusInitParamsEb_t;

typedef struct siriusInitParamsEp_s {
  sbBool_t bBringUp;
  uint8    shapingBusLengthAdj[SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES];   /* shaping bus length adjust values */
} siriusInitParamsEp_t;

typedef struct siriusInitParamsEs_s {
  sbBool_t bBringUp;
} siriusInitParamsEs_t;

typedef struct siriusInitParamsFd_s {
  sbBool_t bBringUp;
  uint8    mvrMaxSize;
} siriusInitParamsFd_t;

typedef struct siriusInitParamsFf_s {
  sbBool_t bBringUp;
} siriusInitParamsFf_t;

typedef struct siriusInitParamsXp_s {
  sbBool_t bBringUp;
} siriusInitParamsXp_t;

typedef struct siriusInitParamsBp_s {
  sbBool_t bBringUp;
} siriusInitParamsBp_t;

typedef struct siriusInitParamsHc_s {
  sbBool_t bBringUp;
} siriusInitParamsHc_t;

typedef struct siriusInitParamsCmic_s {
  sbBool_t bBringUp;
} siriusInitParamsCmic_t;

typedef struct siriusInitParamsXmac_s {
  sbBool_t bBringUp;
} siriusInitParamsXmac_t;


/*
 * Sirius Initialization Structure
 *
 */
typedef struct siriusInitParams_s {
    /* Global configuration */
    int32     unit;
    uint32     reset;                              /* reset qe device if non-zero */
    uint32     bResetOnly;                         /* only reset all blocks and config cmic block if non-zero */
    uint32     uClockSpeedInMHz;                   /* clock speed of device 129-250 nominally 250 */
    uint32     uNodeId;                            /* Node id number for this node */
    sbBool_t     bHybrid;                            /* hybrid mode */
    sbBool_t     bStandAlone;                        /* stand alone switch mode */
    uint32     uStandAloneTsInNs;                  /* timeslot duration in stand alone switch mode */
    sbBool_t     bCleanUp;                           /* clean up after init */

    uint32     uEpochSizeInNs;                     /* epoch duration */
    uint32     uQmMaxArrivalRateMbs;               /* maximum arrival rate in Mbps */
    uint32     uSiLsThreshold;                     /* si (sf/sc blocks) error threshold */
    uint32     uSiLsWindow;                        /* si (sf/sc blocks) error window */
    siriusClearDdrFunc_t clearDdr;                 /* clear external memory */
    int32 nMaxVoq; /* VOQs occur in the range 0 to 32K-1 max followed by local queues */

    int32      spMode;

    /* RB Block receive buffer */
    siriusInitParamsRb_t rb;

    /* QM Block queue manager */
    siriusInitParamsQm_t qm;

    /* QS Block quality of service */
    siriusInitParamsQs_t qs;

    /* TX Block transmit dma */
    siriusInitParamsTx_t tx;

    /* CI Block channel interface */
    uint32     uBufferSize;                        /* external DDR3 buffer size */
    uint32     uMaxBuffer;                         /* external DDR3 max number of buffer */
    uint32     uNumColBits;                        /* external DDR3 number of column bits */
    uint32     uDdr3NumMemories;                   /* external DDR3 devices */
    uint32     uDdr3ClockMhz;                      /* clock frequency for DDR3 interface */
    siriusInitParamsCi_t ci[SOC_SIRIUS_MAX_NUM_CI_BLKS];

    /* SC Block switch fabric control interface */
    siriusInitParamsSc_t sc;

    /* EG Block egress */
    siriusInitParamsEg_t eg;

    /* SF Block switch fabric interface */
    siriusInitParamsSf_t sf;

    /* CS Block central statistics */
    siriusInitParamsCs_t cs;

    /* TS Block traffic scheduler */
    siriusInitParamsTs_t ts;

    /* FR BLock fabric receive */
    siriusInitParamsFr_t fr;

    /* EB Block egress buffer */
    siriusInitParamsEb_t eb;

    /* EP Block egress processor */
    siriusInitParamsEp_t ep;

    /* ES Block egress scheduler */
    siriusInitParamsEs_t es;

    /* FD Block frame disposition manager */
    siriusInitParamsFd_t fd;

    /* FF Block frame fifo manager */
    siriusInitParamsFf_t ff;

    /* XP Block xport */
    siriusInitParamsXp_t xp;

    /* BP Block bypass */
    siriusInitParamsBp_t bp;

    /* HC Block hypercore */
    siriusInitParamsHc_t hc;

    /* CMIC Block cpu interface controller */
    siriusInitParamsCmic_t cmic;

    /* XMAC block */
    siriusInitParamsXmac_t xmac;

} siriusInitParams_t;

/*
 * Ingress Shaper definitions
 */
typedef struct siriusIngressRateInfo_s {
    uint32 rate;
    uint32 exponent;
    uint32 mantissa;
} siriusIngressRateInfo_t;

typedef struct siriusIngressShaperInfo_s {
    siriusIngressRateInfo_t     *basic_rate_info;
    siriusIngressRateInfo_t     *burst_size_info;
} siriusIngressShaperInfo_t;

extern void
soc_sirius_isr(void *_unit);

extern int
soc_sirius_init(int unit, soc_sbx_config_t *cfg);

extern void
soc_sirius_state_cleanup(int unit);

extern int
soc_sirius_detach(int unit);

extern int
soc_sirius_reset(int unit);

extern int
soc_sbx_sirius_ddr23_clear(int unit);

extern int
soc_sirius_port_info_config(int unit, int drv_dev_id, int dev_id);

#define SIRIUS_PREDICATE_FLAGS_INGRESS 1
#define SIRIUS_PREDICATE_FLAGS_EGRESS 2
#define SIRIUS_PREDICATE_FLAGS_WITH_ID 4
#define SIRIUS_PREDICATE_FLAGS_REPLACE 8
#define SIRIUS_PREDICATE_FLAGS_SDK 0x8000000
#define SIRIUS_PREDICATE_OFFSET_INGRESS 0
#define SIRIUS_PREDICATE_OFFSET_EGRESS 16
extern int
soc_sirius_predicate_allocate(int unit,
                              uint32 flags,
                              unsigned int *predId);

extern int
soc_sirius_predicate_free(int unit,
                          unsigned int predId);

extern int
soc_sirius_predicate_check(int unit,
                           unsigned int predId);
extern int
soc_sirius_predicate_check_sdk(int unit,
                               unsigned int predId);

extern int
soc_sirius_predicate_set(int unit,
                         unsigned int predicate,
                         unsigned int offset,
                         int meta,
                         int range,
                         unsigned int low_mask,
                         unsigned int high_data);
extern int
soc_sirius_predicate_set_sdk(int unit,
                             unsigned int predicate,
                             unsigned int offset,
                             int meta,
                             int range,
                             unsigned int low_mask,
                             unsigned int high_data);

extern int
soc_sirius_predicate_get(int unit,
                         unsigned int predicate,
                         unsigned int *offset,
                         int *meta,
                         int *range,
                         unsigned int *low_mask,
                         unsigned int *high_data);

extern int
soc_sirius_predicate_next(int unit,
                          unsigned int bcm_current,
                          unsigned int *next);

#define SIRIUS_COS_MAP_BLOCK_FLAGS_INGRESS 1
#define SIRIUS_COS_MAP_BLOCK_FLAGS_WITH_ID 4
#define SIRIUS_COS_MAP_BLOCK_FLAGS_REPLACE 8
#define SIRIUS_COS_MAP_BLOCK_FLAGS_SDK 0x80000000
#define SIRIUS_COS_MAP_BLOCK_OFFSET_INGRESS 0
extern int
soc_sirius_cos_map_block_allocate(int unit,
                                  uint32 flags,
                                  unsigned int *cosMapBlock);

extern int
soc_sirius_cos_map_block_free(int unit,
                              unsigned int cosMapBlock);

extern int
soc_sirius_cos_map_block_check(int unit,
                               unsigned int cosMapBlock);
extern int
soc_sirius_cos_map_block_check_sdk(int unit,
                                   unsigned int cosMapBlock);

extern int
soc_sirius_cos_map_block_next(int unit,
                              unsigned int bcm_current,
                              unsigned int *next);

extern int
soc_sirius_cos_map_block_entry_set(int unit,
                                   unsigned int cosMapBlock,
                                   unsigned int entryIndex,
                                   unsigned int entryValue);
extern int
soc_sirius_cos_map_block_entry_set_sdk(int unit,
                                       unsigned int cosMapBlock,
                                       unsigned int entryIndex,
                                       unsigned int entryValue);

extern int
soc_sirius_cos_map_block_entry_get(int unit,
                                   unsigned int cosMapBlock,
                                   unsigned int entryIndex,
                                   unsigned int *entryValue);

#define SIRIUS_QUEUE_MAP_BLOCK_FLAGS_INGRESS 1
#define SIRIUS_QUEUE_MAP_BLOCK_FLAGS_WITH_ID 4
#define SIRIUS_QUEUE_MAP_BLOCK_FLAGS_REPLACE 8
#define SIRIUS_QUEUE_MAP_BLOCK_FLAGS_SDK 0x80000000
#define SIRIUS_QUEUE_MAP_BLOCK_OFFSET_INGRESS 0
extern int
soc_sirius_queue_map_block_allocate(int unit,
                                    uint32 flags,
                                    unsigned int base,
                                    unsigned int size,
                                    unsigned int *queueMapBlock);

extern int
soc_sirius_queue_map_block_free(int unit,
                                unsigned int queueMapBlock);

extern int
soc_sirius_queue_map_block_check(int unit,
                                 unsigned int queueMapBlock);
extern int
soc_sirius_queue_map_block_check_sdk(int unit,
                                     unsigned int queueMapBlock);

extern int
soc_sirius_queue_map_block_get(int unit,
                               unsigned int queueMapBlock,
                               unsigned int *base,
                               unsigned int *size);

extern int
soc_sirius_queue_map_block_next(int unit,
                                unsigned int bcm_current,
                                unsigned int *next);

extern int
soc_sirius_queue_map_block_entry_set(int unit,
                                     unsigned int queueMapBlock,
                                     unsigned int offset,
                                     unsigned int cosMapBlock,
                                     unsigned int queueId);
extern int
soc_sirius_queue_map_block_entry_set_sdk(int unit,
                                         unsigned int queueMapBlock,
                                         unsigned int offset,
                                         unsigned int cosMapBlock,
                                         unsigned int queueId);

extern int
soc_sirius_queue_map_block_entry_get(int unit,
                                     unsigned int queueMapBlock,
                                     unsigned int offset,
                                     unsigned int *cosMapBlock,
                                     unsigned int *queueId);

extern int
soc_sirius_queue_map_block_entry_multi_set(int unit,
                                           unsigned int queueMapBlock,
                                           unsigned int baseOffset,
                                           unsigned int count,
                                           const unsigned int *cosMapBlock,
                                           const unsigned int *queueId);
extern int
soc_sirius_queue_map_block_entry_multi_set_sdk(int unit,
                                               unsigned int queueMapBlock,
                                               unsigned int baseOffset,
                                               unsigned int count,
                                               const unsigned int *cosMapBlock,
                                               const unsigned int *queueId);

extern int
soc_sirius_queue_map_block_entry_multi_get(int unit,
                                           unsigned int queueMapBlock,
                                           unsigned int baseOffset,
                                           unsigned int count,
                                           unsigned int *cosMapBlock,
                                           unsigned int *queueId);

extern int
soc_sirius_queue_map_block_entry_next(int unit,
                                      unsigned int queueMapBlock,
                                      unsigned int bcm_current,
                                      unsigned int *next);

#define SIRIUS_PARSER_FLAGS_INGRESS 1
#define SIRIUS_PARSER_FLAGS_EGRESS 2
#define SIRIUS_PARSER_FLAGS_WITH_ID 4
#define SIRIUS_PARSER_FLAGS_REPLACE 8
#define SIRIUS_PARSER_FLAGS_QUEUE_DEFAULT 0x00000010
#define SIRIUS_PARSER_FLAGS_USE_COS_MAP 0x00000020
#define SIRIUS_PARSER_FLAGS_USE_TAG_OFFSET 0x00000040
#define SIRIUS_PARSER_FLAGS_OVERWRITE_DEST 0x00000080
#define SIRIUS_PARSER_FLAGS_LENGTH_ADJUST 0x00000100
#define SIRIUS_PARSER_FLAGS_CLEAR_TEST_BIT 0x00000200
#define SIRIUS_PARSER_FLAGS_LOOKUP_ENCAPID 0x00000400
#define SIRIUS_PARSER_FLAGS_CLEAR_MC_BIT 0x00000800
#define SIRIUS_PARSER_FLAGS_LEN_ADJ_ON_SHAPING 0x00001000
#define SIRIUS_PARSER_FLAGS_LOOKUP_QUEUEID 0x00002000
#define SIRIUS_PARSER_FLAGS_XGS_MODE     0x00004000
#define SIRIUS_PARSER_FLAGS_STAT0_ADJ_PER_PACKET 0x00008000
#define SIRIUS_PARSER_FLAGS_STAT1_ADJ_PER_PACKET 0x00010000
#define SIRIUS_PARSER_FLAGS_STAT_COMBINE 0x00020000
#define SIRIUS_PARSER_FLAGS_SDK 0x80000000
#define SIRIUS_PARSER_OFFSET_INGRESS 0
#define SIRIUS_PARSER_OFFSET_EGRESS 32

/*
 *  In structs within union below, 'uint32 flags' must always be first elem.
 */
typedef union soc_sirius_parser_info_u {
    struct {
        uint32 flags;
        unsigned int segmentSel;
        unsigned int cosProfile;
        unsigned int queue_byte0;
        unsigned int queue_bit0;
        unsigned int queue_length0;
        unsigned int queue_byte1;
        unsigned int queue_bit1;
        unsigned int queue_length1;
        unsigned int cos_byte;
        unsigned int cos_bit;
        signed int dp;
        signed int ecn;
        unsigned int stat_byte0;
        unsigned int stat_bit0;
        unsigned int stat_length0;
        unsigned int stat_byte1;
        unsigned int stat_bit1;
        unsigned int stat_length1;
    } ingress;
    struct {
        uint32 flags;
        unsigned int stat0_segment;
        unsigned int stat0_source0;
        unsigned int stat0_offset0;
        unsigned int stat0_length0;
        unsigned int stat0_source1;
        unsigned int stat0_offset1;
        unsigned int stat0_length1;
        signed int stat0_adjust;
        unsigned int stat1_segment;
        unsigned int stat1_source0;
        unsigned int stat1_offset0;
        unsigned int stat1_length0;
        unsigned int stat1_source1;
        unsigned int stat1_offset1;
        unsigned int stat1_length1;
        signed int stat1_adjust;
        unsigned int oi_write_byte;
        unsigned int oi_write_length;
        unsigned int oi_read_byte;
        unsigned int oi_read_length;
        unsigned int oi_index_offset;
        unsigned int requeueSel;
        unsigned int lengthAdj_nybble;
        unsigned int lengthAdj_length;
        unsigned int fcos_bit;
        unsigned int fcos_length;
        signed int eh_bit;
        unsigned int queue_write_byte;
        unsigned int queue_write_length;
        unsigned int hdrRemove_bytes;
        signed int fourByteRemove_quadbyte;
        signed int mp_bit;
        unsigned int insertSel;
    } egress;
} soc_sirius_parser_info_t;

extern int
soc_sirius_parser_allocate(int unit,
                           uint32 flags,
                           unsigned int *parserId);

extern int
soc_sirius_parser_free(int unit,
                       unsigned int parserId);

extern int
soc_sirius_parser_check(int unit,
                        unsigned int parserId);
extern int
soc_sirius_parser_check_sdk(int unit,
                            unsigned int parserId);

extern int
soc_sirius_parser_set(int unit,
                      unsigned int parserId,
                      const soc_sirius_parser_info_t *parserInfo);
extern int
soc_sirius_parser_set_sdk(int unit,
                          unsigned int parserId,
                          const soc_sirius_parser_info_t *parserInfo);

extern int
soc_sirius_parser_get(int unit,
                      unsigned int parserId,
                      soc_sirius_parser_info_t *parserInfo);

extern int
soc_sirius_parser_next(int unit,
                       unsigned int bcm_current,
                       unsigned int *next);

extern int
soc_sirius_parser_clear_unused_ingress(int unit);

#define SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS 0x01
#define SIRIUS_PREDICATE_PARSER_RULE_FLAG_EGRESS  0x02
#define SIRIUS_PREDICATE_PARSER_RULE_FLAG_WITH_ID 0x04
#define SIRIUS_PREDICATE_PARSER_RULE_FLAG_REPLACE 0x08
#define SIRIUS_PREDICATE_PARSER_RULE_FLAG_DELETE  0x80
#define SIRIUS_PREDICATE_PARSER_RULE_OFFSET_INGRESS 0
#define SIRIUS_PREDICATE_PARSER_RULE_OFFSET_EGRESS 2048

typedef struct soc_sirius_predicate_parser_rule_s {
    uint16  flags;
    uint16  parser;
    uint16  priority;
    uint16  predState;
    uint16  predMask;
} soc_sirius_predicate_parser_rule_t;

extern int
soc_sirius_predicate_parser_map_add(int unit,
                                    const soc_sirius_predicate_parser_rule_t *ruleInfo,
                                    unsigned int *ruleId);

extern int
soc_sirius_predicate_parser_map_delete(int unit,
                                       unsigned int ruleId);

extern int
soc_sirius_predicate_parser_map_get(int unit,
                                    unsigned int ruleId,
                                    soc_sirius_predicate_parser_rule_t *ruleInfo);

extern int
soc_sirius_predicate_parser_map_check(int unit,
                                      unsigned int ruleId);

extern int
soc_sirius_predicate_parser_map_get_next(int unit,
                                         unsigned int bcm_current,
                                         unsigned int *next,
                                         soc_sirius_predicate_parser_rule_t *ruleInfo);


extern int
soc_sirius_port_control_default_queue_access(int unit,
                                             int write,
                                             unsigned int portIndex,
                                             int *value);

extern int
soc_sirius_hw_update_trt(uint32 unit);

extern int
soc_sirius_hw_update_crt(uint32 unit);

#define FF_MEM_ALLOCATE 0
#define FF_MEM_UPDATE   1
#define FF_MEM_EXPAND   2
#define FF_MEM_DEFRAG   3
#define FF_MEM_DELETE   4

extern int
soc_sirius_config_ff(uint32 unit, uint16 subport, uint8 egroup, uint8 flags);

extern int
soc_sirius_fd_unicast_gmt_set(int unit, int32 sysport, int32 port);

extern int
soc_sirius_fd_fct_set(int unit, int32 sysport, int32 ef, int32 mc, int32 fct);

extern int
soc_sirius_fd_fct_get(int unit, int32 sysport, int32 ef, int32 mc, int32 *fct);

extern void
soc_sirius_block_error(void *unit_vp, void *d1, void *d2,
		       void *d3, void *d4);

/*
 * Ingress Shaper definitions
 */
extern int 
soc_sirius_get_ingress_rate_info(int unit, uint32 rate,
                                 uint32 *rate_mantissa, uint32 *rate_exponent,
                                 uint32 *burst_mantissa, uint32 *burst_exponent);

extern int
soc_sirius_queue_min_util_set(int unit, int32 nQueue, int template);

extern int
soc_sirius_queue_min_util_get(int unit, int32 nQueue, int *template);

extern int
soc_sirius_queue_max_age_set(int unit, int32 nQueue, int template);

extern int
soc_sirius_queue_max_age_get(int unit, int32 nQueue, int *template);

extern int
soc_sirius_template_min_util_recall(int unit, int hwUtil, int *util);

extern int
soc_sirius_template_min_util_adjust(int unit, int util, int *hwUtil);

extern int
soc_sirius_template_min_util_set(int unit, int32 template, int hwUtil);

extern int
soc_sirius_template_max_age_recall(int unit, int hwAge, int *age);

extern int
soc_sirius_template_max_age_adjust(int unit, int age, int *hwAge);

extern int
soc_sirius_template_max_age_set(int unit, int32 template, int hwAge);

extern int
soc_sirius_qm_queue_size_in_bufs_get(int unit, uint32 buff_sz, uint32 *nbr_bufs);

extern int
soc_sirius_qm_queue_min_buff_set(int unit, int32 template_id, uint32 min_buff_sz);

extern int
soc_sirius_qm_queue_max_buff_set(int unit, int32 template_id, uint32 max_buff_sz);

extern int
soc_sirius_qm_queue_buffs_profile_entry_set(int unit, int32 queue, uint32 buffs_template, uint32 enable);

extern int
soc_sirius_qm_queue_buffs_profile_entry_get(int unit, int32 queue, uint32 *pbuffs_template, uint32 *penable);

extern int
soc_sirius_si_to_port(int unit, int32 nSi, bcm_port_t *port);

extern int
soc_sirius_ci_ddr_verify(int unit, int ci);

extern int soc_sirius_ts_burst_size_bytes_get(int unit, int els, int node,
                                            int num_channels, int *burst_size);

extern int soc_sirius_bp_burst_size_bytes_set(int unit);

extern int soc_sirius_port_flush(int unit, int fifo);

extern int soc_sirius_fcd_empty(int unit, int fcd, int *is_empty);

extern int soc_sirius_fifo_empty(int unit, int fifo, int *is_empty);

extern int soc_sirius_fifo_force_full_set(int unit, int fifo, int is_asserted);

extern int soc_sirius_dt_mem_set(int unit, int fcd, int cosq, uint32  bytes_min, uint32  bytes_max);

extern int soc_sirius_dt_mem_get(int unit, int fcd, int cosq, uint32 *bytes_min, uint32 *bytes_max);

extern int soc_sirius_dmt_mem_set(int unit, int sysport, int domain0, int domain1);
 
extern int soc_sirius_dmt_mem_get(int unit, int sysport, int *domain0, int *domain1); 

typedef struct soc_sirius_ddr_bw_table_entry_s {
    int     ddr_num;
    int     ddr_mhz;
    int     ddr_bw;
    int     ddr_cong_bw;    /* Congestion bandwidth */
}soc_sirius_ddr_bw_table_entry_t;

extern int soc_sirius_ddr_bandwidth_get(int unit, uint32 *ddr_bw, 
                                        uint32 *ddr_cong_bw);
#endif /* _SIRIUS_H */
