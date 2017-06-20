/*
 * $Id: cosq.h,v 1.89 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: cosq common interface
 */

#ifndef _BCM_INT_SBX_COSQ_H_
#define _BCM_INT_SBX_COSQ_H_

#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/sbDq.h>
#include <bcm/types.h>
#include <bcm/cosq.h>

#define BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE         0
#define BCM_SBX_COSQ_DISCARD_AVAILABLE_TEMPLATE_START 1
#define BCM_SBX_COSQ_DISCARD_MAX_COLORS               4
#define BCM_SBX_COSQ_DISCARD_COLOR_GREEN_DP0          0
#define BCM_SBX_COSQ_DISCARD_COLOR_YELLOW_DP1         1
#define BCM_SBX_COSQ_DISCARD_COLOR_RED_DP2            2
#define BCM_SBX_COSQ_DISCARD_COLOR_BLACK_DP3          3
#define BCM_SBX_COSQ_DEFAULT_GAIN                     3
#define BCM_SBX_COSQ_MIN_GAIN                         0
#define BCM_SBX_COSQ_MAX_GAIN                         15
#define BCM_SBX_COSQ_MIN_DROP_PROBABILITY             0
#define BCM_SBX_COSQ_MAX_DROP_PROBABILITY             99
#define BCM_SBX_COSQ_GLOBAL_MAX_NO_COS                8
#define BCM_SBX_COSQ_LOCAL_MAX_NO_COS                 16
#define BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS     4 /* standard/default 4 FIFO set model */

#define BCM_SBX_COSQ_GPORT_DELETE 0                     /* MCRemoveAll */
#define BCM_SBX_COSQ_GPORT_SHOW 1                       /* fabric cli command */
#define BCM_SBX_COSQ_GPORT_SHOW_ALL 2
#define BCM_SBX_COSQ_GPORT_REMOVE_ALL 3
#define BCM_SBX_COSQ_GPORT_REMOVE 4


#define BCM_INT_SBX_SYSPORT_STATE_GLOBAL              1 /* available for FIC flows */
#define BCM_INT_SBX_SYSPORT_STATE_LOCAL               2 /* Reserved for TME flows  */
#define BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET       3 /* Reserved for supporting Extended ESETs */
#define BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC    4 /* Reserved for individual FIFO flow control*/
#define BCM_INT_SBX_SYSPORT_STATE_RESERVED_GENERAL    5 /* Reserved for general case */
#define BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE    (1<<0)
#define BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE (1<<1)

#define BCM_INT_SBX_SYSPORT_FLAGS_DEFAULT             0 /* depending on independent_fc, allocate */
							/* one or two sysports                   */
#define BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE           1 /* When independent_fc enabled, allocate */
							/* only one fc and free next if odd      */

#define BCM_SBX_COSQ_SUBSCRIBER_MAP_GET        0
#define BCM_SBX_COSQ_SUBSCRIBER_MAP_ADD        1
#define BCM_SBX_COSQ_SUBSCRIBER_MAP_DELETE     2
#define BCM_SBX_COSQ_SUBSCRIBER_MAP_DELETE_ALL 3

#define BCM_SBX_COSQ_RCPQ_CIR_EQUAL_ZERO       1

/* sysport defination for FIC flows in DMODE - Hybrid configuration */
#define BCM_INT_SBX_DMODE_SYSPORT_SET(node, port)              \
                            (((node << 6) & 0x3C0) | ((port << 0) & 0x03F))

#define BCM_INT_SBX_DMODE_SYSPORT_NODE_GET(sys_port)           \
                            ((sys_port & 0x3C0) >> 6)

#define BCM_INT_SBX_DMODE_SYSPORT_PORT_GET(sys_port)           \
                            ((sys_port & 0x03F) >> 0)


#define BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sys_port)            \
                            bcm_sbx_cosq_sysport_to_ef_sysport(unit, sys_port)

#define BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sys_port)           \
                            bcm_sbx_cosq_sysport_to_nef_sysport(unit, sys_port)

#define BCM_INT_SBX_PORT_TO_EF_PORT(port)                      \
                            (port * 2)

#define BCM_INT_SBX_PORT_TO_NEF_PORT(port)                     \
                            ((port * 2) + 1)

#define BCM_INT_SBX_SCHEDULER_GPORT_SET(gport, egress, id)  \
    (BCM_GPORT_SCHEDULER_SET(gport, (((egress)?((_SHR_GPORT_SCHEDULER_MASK+1) >> 1) : 0) | ((id) & (_SHR_GPORT_SCHEDULER_MASK >> 1)))))

#define BCM_INT_SBX_SCHEDULER_ID_GET(gport)                    \
    ((BCM_GPORT_SCHEDULER_GET(gport)) & (_SHR_GPORT_SCHEDULER_MASK >> 1))

#define BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)             \
    ((BCM_GPORT_SCHEDULER_GET(gport) & ((_SHR_GPORT_SCHEDULER_MASK+1) >> 1)) ? 1 : 0)

#define BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport)             \
    ((BCM_GPORT_SCHEDULER_GET(gport) & ((_SHR_GPORT_SCHEDULER_MASK+1) >> 1)) ? 0 : 1)

#define BCM_INT_SBX_SCHEDULER_LEVEL_GET(gport)                    \
    (_SHR_GPORT_SCHEDULER_LEVEL_GET(gport))

#define BCM_INT_SBX_SCHEDULER_NODE_GET(gport)                    \
    (_SHR_GPORT_SCHEDULER_NODE_GET(gport))

#define BCM_INT_SBX_MULTIPATH_GPORT_SET(gport, egress, id)  \
    (BCM_COSQ_GPORT_MULTIPATH_SET(gport, (((((egress)?1:0) & 0x1) << (_SHR_COSQ_GPORT_TYPE_SHIFT-1)) | ((id) & (_SHR_COSQ_GPORT_MULTIPATH_MASK >> 1)))))

#define BCM_INT_SBX_MULTIPATH_ID_GET(gport)                    \
    ((BCM_COSQ_GPORT_MULTIPATH_GET(gport)) & (_SHR_COSQ_GPORT_MULTIPATH_MASK >> 1))

#define BCM_INT_SBX_MULTIPATH_IS_EGRESS(gport)             \
    (((BCM_COSQ_GPORT_MULTIPATH_GET(gport) >> (_SHR_COSQ_GPORT_TYPE_SHIFT-1)) & 0x1))

#define BCM_INT_SBX_MULTIPATH_IS_INGRESS(gport)             \
    (!((BCM_COSQ_GPORT_MULTIPATH_GET(gport) >> (_SHR_COSQ_GPORT_TYPE_SHIFT-1)) & 0x1))

typedef enum {
  bcm_sbx_cosq_queue_region_global = 0,
  bcm_sbx_cosq_queue_region_local,
  bcm_sbx_cosq_queue_region_last
} bcm_sbx_cosq_queue_region_type_t;

typedef struct bcm_sbx_cosq_queue_region_s {
    sbBool_t is_valid;
    int32 start;
    int32 end;
} bcm_sbx_cosq_queue_region_t;

typedef struct bcm_sbx_cosq_bw_group_params_s {
    int32 bag_rate_kbps; /* rate of the flows within the bag */
} bcm_sbx_cosq_bw_group_params_t;


#ifdef BCM_EASY_RELOAD_SUPPORT
typedef struct bcm_sbx_cosq_bw_group_state_cache_s{
    int32 base_queue_of_bag;
    uint32 bag_rate_bytes_per_epoch;
}bcm_sbx_cosq_bw_group_state_cache_t;
#endif 

/* Bandwidth Allocation Group structure */
typedef struct bcm_sbx_cosq_bw_group_state_s {
    int32 in_use;
    bcm_gport_t gport;
    int32 base_queue;
    int32 num_cos;
    int32 dest_node; /* if mcast, this is the upper 1 bit of the eset */
    int32 dest_port; /* if mcast, this is the lower 6 bits of the eset */
    int32 dest_mc;   /* if mcast, this could be 0 or 1 */
#define  BCM_INT_SBX_DEST_TYPE_UNICAST 0
#define  BCM_INT_SBX_DEST_TYPE_MULTICAST 1
    int32 dest_type;
    int32 dest_port_kbits_sec_max; /* this is the total bandwidth of the destination port */
    bcm_sbx_cosq_bw_group_params_t path;
    int32 flags; /* saved from gport add command */

    int32 overlay_in_use;    /* if overlay, set to TRUE */
    int32 overlay_dest_node; /* if mcast, this is the upper 1 bit of the eset */
    int32 overlay_dest_port; /* if mcast, this is the lower 6 bits of the eset */
    int32 overlay_base_queue;
    int32 overlay_num_cos;
    uint8 cosq_init_added;  /* if TRUE bw_group was allocated during cosq_init (bcm_cosq_init=1) */
    uint8 cos_map;

} bcm_sbx_cosq_bw_group_state_t;

typedef struct bcm_sbx_cosq_bw_value_e {
    int32 guarantee_kbps;
    int32 sp_priority;
    int32 wfq_weight;
}bcm_sbx_cosq_bw_value_t;

#define BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT                 -1
#define BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING    -2
#define BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITHOUT_SHAPING -3
#define BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT -1
#define BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT -1
#define BCM_INT_SBX_REQUESTED_EGROUP_AUTO_SELECT -1
#define BCM_INT_SBX_REQUESTED_FCD_AUTO_SELECT -1
#define BCM_INT_SBX_REQUESTED_MULTIPATH_AUTO_SELECT -1

#define BCM_INT_SBX_QUEUE_STATE_AVAILABLE      (0x0)
#define BCM_INT_SBX_QUEUE_STATE_IN_USE         (0x1)
#define BCM_INT_SBX_QUEUE_STATE_RESERVED_TME   (0x3)
#define BCM_INT_SBX_STAT_ENQUEUE   0x1
#define BCM_INT_SBX_STAT_DEQUEUE   0x1

#define BCM_DEFAULT_INT32_VALUE -1 /* indicates flib will override unless user sets */

typedef struct bcm_sbx_cosq_queue_params_ingress_s {
    int32 bw_mode; /* one of BCM_COSQ_SP, BCM_COSQ_EF, BCM_COSQ_SP_GLOBAL, BCM_COSQ_AF */
                   /* BCM_COSQ_WEIGHTED_FAIR_QUEUING  BCM_COSQ_BE                             */
    bcm_sbx_cosq_bw_value_t bw_value; /* can mean priority or weight or guarantee */
    int32 given_weight; /* WFQ only, this is the 128 adjusted weight value for this queue */
    int32 min_physical_queue_depth_bytes; /* Minimum number of bytes to reserve for the queue */
    /* this must be large enough for 2 maximum sized packets */
    int32 max_physical_queue_depth_bytes; /* Maximum allowed physical queue depth in bytes*/
    int32 anemic_watermark_select;
    /* sel     watermark */
    /* 0       2/10 linesPerTimeslot */
    /* 1       3/10 linesPerTimeslot */
    /* 2       4/10 linesPerTimeslot */
    /* 3 EF    5/10 linesPerTimeslot */
    /* 4       6/10 linesPerTimeslot */
    /* 5       7/10 linesPerTimeslot */
    /* 6 other 8/10 linesPerTimeslot */
    /* 7       9/10 linesPerTimeslot */
    int32 ingress_spi4;
    int32 ingress_spi4_port;
    int32 qla_demand_mask;
    int32 queue_header_adjust_sign;
    int32 queue_header_adjust;
    int32 anemic_age_threshold_key;
        /* Now, set the age threshold for each key as follows */
        /* key           anemic thresh      EF thresh         */
        /* 0  EF          0x04  (32us)      0xff              */
        /* 1              0x08  (64us)      0xff              */
        /* 2  SP7         0x0C  (96us)      0xff              */
        /* 3              0x10 (128us)      0xff              */
        /* 4  SP6         0x14 (160us)      0xff              */
        /* 5              0x18 (192us)      0xff              */
        /* 6  other       0x1D (240us)      0xff              */
        /* 7              0x20 (256us)      0xff              */
        /* 8              0x24 (288us)      0xff              */
        /* 9              0x28 (320us)      0xff              */
        /* 10             0x2C (352us)      0xff              */
        /* 11             0x30 (384us)      0xff              */
        /* 12             0x34 (416us)      0xff              */
        /* 13             0x38 (448us)      0xff              */
        /* 14             0x3C (480us)      0xff              */

        /* 10             0x40 (512us)      0xff              */
    int32 shape_limit_kbps;
    int32 enable_shaping; /* TRUE/FALSE for enable/disable shaping on the queue for this flow */
    int32 shape_burst_kbps;
    int32 local;   /* TRUE if the queue is marked as a local queue (arbitration is local - TME) */
    int32 hold_pri_num_timeslots; /* hold timeslot - priority will go to HOLDPRI for hold_ts timeslots if queue not emptied during */
                                  /* timeslot - for up to hold_ts timeslots default setting: 1, for full bus.  For half-bus mode   */
                                  /* set to 3 - this is to optimize for 104 lines/ts and 8 links                                   */
    int32 template; /* WRED template */
    int32 gain;     /* WRED gain */
    int32 enabled;  /* queue is enabled if non-zero */
}bcm_sbx_cosq_queue_params_ingress_t;

typedef struct bcm_sbx_mgid_list_e {
  dq_t node;
  int data;
} bcm_sbx_mgid_list_t;

typedef struct bcm_sbx_cosq_queue_state_s {
    uint8 state;   /* one of BCM_INT_SBX_QUEUE_STATE_AVAILABLE, IN_USE... */
    int32 bw_group;/* internal: bw group id (BAG) associated with this flow */
    int16 sysport; /* not used for BM3200 sysport id associated with this flow */
    int16 default_sysport;
    bcm_sbx_cosq_queue_params_ingress_t ingress;
    int8  enq_stat_in_use;
    int8  deq_stat_in_use;
    int8  attached_fifo;
    bcm_sbx_mgid_list_t *mgid_list; /* linked list of multicast group ids associated with q */
}bcm_sbx_cosq_queue_state_t;

typedef struct bcm_sbx_cosq_sysport_state_s {
    int8  flags;    /* in_use, olay_done */
    int16 state;    /* one of BCM_INT_SBX_SYSPORT_STATE_GLOBAL, ... */
    int16 fcd;      /* Flow Control Domain associated with remote sysport */
    int8  fifo;     /* First FIFO associated with this sysport */
    int8  use_cnt;  /* currently using this sysport */
    int32 node;     /* node of the sysport */
    int32 port;     /* port of the sysport */
    uint32 egport;  /* egress gport attached via sysport */
}bcm_sbx_cosq_sysport_state_t;

typedef struct bcm_sbx_cosq_fcd_state_s {
    int8 in_use;    /* FREE/INIT/ALLOC */
    int8 ref_cnt;   /* how many queues attached to the fifo */
    uint16 fifo;    /* Fifo index value */
    uint32 sysport; /* not used for BM3200 sysport id associated with this flow */
    int32 node;     /* node of the flow control domain */
    int32 port;     /* port of the flow control domain */
}bcm_sbx_cosq_fcd_state_t;

#define FCD_FREE                                  0
#define FCD_TAKE                                  1
#define FCD_ALLOC                                 2
#define FCD_BUSY                                (-2)
#define BCM_INT_SBX_INVALID_FCD                 (-1)
#define BCM_INT_SBX_INVALID_SYSPORT             (-1)
#define BCM_INT_SBX_SYSPORT_TYPE_SHIFT          (16)
#define BCM_INT_SBX_SYSPORT_MASK                ((1<<BCM_INT_SBX_SYSPORT_TYPE_SHIFT) - 1)
#define BCM_INT_SBX_SYSPORT_TYPE_DUMMY          (0xBAD)
#define BCM_INT_SBX_SYSPORT_DUMMY(sysport)      ( (BCM_INT_SBX_SYSPORT_TYPE_DUMMY << BCM_INT_SBX_SYSPORT_TYPE_SHIFT) | (sysport & BCM_INT_SBX_SYSPORT_MASK) )
#define BCM_INT_SBX_SYSPORT_REAL(dummy_sysport) ( dummy_sysport & ((1<<BCM_INT_SBX_SYSPORT_TYPE_SHIFT) - 1) )
#define BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport)   ( (sysport & ~BCM_INT_SBX_SYSPORT_MASK) == (BCM_INT_SBX_SYSPORT_TYPE_DUMMY << BCM_INT_SBX_SYSPORT_TYPE_SHIFT) )

#define BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP  (14)
#define BCM_INT_SBX_MAX_SYSPORT_GROUP           (64)
#define BCM_INT_SBX_NODE_PORT_THRESHOLD         (8)
#define BCM_INT_SBX_DP_IGNORE                   (1 << 15)
#define BCM_INT_SBX_DP_IGNORE_MASK              (0x7f)

typedef struct bcm_sbx_cosq_destport_state_s {
    int16 ref_cnt;  /* number of sysports using dest_port */
}bcm_sbx_cosq_destport_state_t;

typedef struct bcm_sbx_cosq_sysport_group_state_s {
    int16 state;   /* BCM_INT_SBX_SYSPORT_STATE_GLOBAL, ... */
    int16 size;    /* size of group, ie, how many sysport the group has */
    int32 used;    /* usage count, number of sysport used */
    uint8 node_cnt[128];  /* Node per group count */
    uint8 node_port[128]; /* dest_port associated with node */
}bcm_sbx_cosq_sysport_group_state_t;

typedef struct bcm_sbx_cosq_discard_state_s {
    int8                        is_free;              /* template is not utlized */
    int                         template;             /* template number */
    bcm_cosq_gport_discard_t    config[BCM_SBX_COSQ_DISCARD_MAX_COLORS];            /* DPs/Colors */
    uint32                      queue_size;           /* queue size for this template */
    uint32                      mtu_sz;               /* mtu size, currently global   */
                                                      /* setting via SOC parameter    */
    uint32                      min_queue_size;        /* minimum queue size, only used when 
						       * soc_feature_ingress_size_templates is TRUE
						       */
    sbBool_t                    pfc_en;               /* TRUE/FALSE, enable PFC */
    uint8                       pfc_pg;               /* priority group */
    uint32                      ref_cnt;              /* number of queues using this  */
                                                      /* template.                    */
} bcm_sbx_cosq_discard_state_t;

typedef struct bcm_sbx_gport_cb_params_s {
  int cmd;
  int verbose;
  int modid;
  int port;
} bcm_sbx_gport_cb_params_t;

/* logical scheduler state */
typedef struct bcm_sbx_cosq_ingress_scheduler_state_s {
    sbBool_t in_use;        /* TRUE/FALSE */
    int      level;         /* physical scheduler level */
    int      node;          /* physical scheduler node */
    int      num_childs;    /* num of children reserved */
}bcm_sbx_cosq_ingress_scheduler_state_t;

typedef struct bcm_sbx_cosq_egress_scheduler_state_s {
    sbBool_t in_use;        /* TRUE/FALSE */
    int      level;         /* physical scheduler level */
    int      node;          /* physical scheduler node */
}bcm_sbx_cosq_egress_scheduler_state_t;

typedef struct bcm_sbx_cosq_egress_group_state_s {
    sbBool_t in_use;        /* TRUE/FALSE */
    int      child_port;    /* Child port number */
    int      eg_in_use;     /* egress group used */
    int      eg_scheduler;  /* scheduler used for the egress group, -1 if not used */
}bcm_sbx_cosq_egress_group_state_t;

typedef struct bcm_sbx_cosq_ingress_multipath_state_s {
    sbBool_t in_use;        /* TRUE/FALSE */
    int      level;         /* physical master scheduler level */
    int      node;          /* physical master scheduler node */
    int      num_nodes;     /* num of member nodes in multipath */
}bcm_sbx_cosq_ingress_multipath_state_t;

#define BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER (528)
#define BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE ((BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER+31)/32)

typedef struct bcm_sbx_cosq_egress_multipath_state_s {
    sbBool_t in_use;        /* TRUE/FALSE */
    int      node;          /* physical group shaper id */
    uint32   member[BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE];    /* physical fifo member bitmask, enough for 544 members */
}bcm_sbx_cosq_egress_multipath_state_t;

typedef struct bcm_sbx_cosq_egress_flow_control_state_s {
    bcm_cos_t int_pri;
    uint32 flow_control_mask;
} bcm_sbx_cosq_egress_flow_control_state_t;

typedef struct bcm_sbx_subscriber_map_cb_params_s {
  int cmd;
  int verbose;
} bcm_sbx_subscriber_map_cb_params_t;


#define BCM_INT_SBX_MAX_COS_SHIFT 4 /* shift at most 4 for 16 cos levels */



/* These are temporary until macros are provided to decode the QE ports */
#define BCM_INT_SBX_QE2000_SOC_SPI_PORT_MIN 0 /* taken from looking at pbmp on metrocore from bcm.user */
#define BCM_INT_SBX_QE2000_SOC_CPU_PORT_MIN 49 /* assume port 49 is the CPU in the pbmp bit mask */

#define BCM_INT_SBX_SIRIUS_SOC_HG_PORT_MIN 0 /* taken from looking at pbmp on metrocore from bcm.user */
#define BCM_INT_SBX_SIRIUS_SOC_CPU_PORT_MIN 128 /* assume port 128 is the CPU in the pbmp bit mask */

#define BCM_INT_SBX_QE2000_IS_SPI_PORT(gport_port) ( ((gport_port) >= BCM_INT_SBX_QE2000_SOC_SPI_PORT_MIN) && ((gport_port) <  BCM_INT_SBX_QE2000_SOC_CPU_PORT_MIN) )
#define BCM_INT_SBX_SIRIUS_IS_HG_PORT(gport_port) ( ((gport_port) >= BCM_INT_SBX_SIRIUS_SOC_HG_PORT_MIN) && ((gport_port) <  BCM_INT_SBX_SIRIUS_SOC_CPU_PORT_MIN) )


#define BCM_INT_SBX_GPORT_GET_FABRICPORT_FROM_MODPORT(modport, p_fabricport) \
    { \
        int modport_port; /* the port field of modport */ \
        modport_port = BCM_GPORT_MODPORT_PORT_GET(modport); \
	if (modport_port <= BCM_INT_SBX_QE2000_SOC_CPU_PORT_MIN) {      \
	    *p_fabricport = modport_port;				\
	} else {							\
	    *p_fabricport = -1;						\
	}								\
    }
#define BCM_INT_SBX_GPORT_GET_MODPORT_FROM_FABRICPORT(qetype, fabricport, p_modport_port) \
   { \
       *p_modport_port = fabricport; \
   }

#define BCM_INT_SBX_COSQ_GET_ESET_FROM_NODE_PORT(unit, node, port, mc, eset_p)  \
              bcm_sbx_cosq_get_eset_from_node_port(unit, node, port, mc, eset_p)

#define BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_ESET(unit, eset, node_p, port_p, mc_p)  \
              bcm_sbx_cosq_get_node_port_from_eset(unit, eset, node_p, port_p, mc_p)

#define BCM_INT_SBX_COSQ_ESET_TO_EF_ESET(unit, eset)            \
                            bcm_sbx_cosq_eset_to_ef_eset(unit, eset)

#define BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, eset)           \
                            bcm_sbx_cosq_eset_to_nef_eset(unit, eset)

#define BCM_INT_SBX_COSQ_ESET_TO_COS_ESET(unit, eset, cos)      \
                            bcm_sbx_cosq_eset_to_cos_eset(unit, eset, cos)

#define BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_SYSPORT(unit, sysport, node_p, port_p, mc_p)    \
                    bcm_sbx_cosq_get_node_port_from_sysport(unit, sysport, node_p, port_p, mc_p)

#define BCM_INT_SBX_COSQ_GET_SYSPORT_FROM_NODE_PORT(unit, node, port, mc, sysport_p)    \
                    bcm_sbx_cosq_get_sysport_from_node_port(unit, node, port, mc, sysport_p)

#define BCM_INT_SBX_QUEUE_SIZE_TEMPLATE  (0)
#define BCM_INT_SBX_FIC_WRED_TEMPLATE    (1)
#define BCM_INT_SBX_LOCAL_WRED_TEMPLATE  (2)
#define BCM_INT_SBX_PREALLOCED_TEMPLATE  (-1)

typedef struct bcm_sbx_cosq_connect_s {
    int queue;
    int num_cos;
    bcm_sbx_cosq_queue_region_type_t queue_region;
    int template;
    int is_allocated;
} bcm_sbx_cosq_connect_t;

typedef struct bcm_sbx_cosq_control_s {
    union {
        bcm_sbx_cosq_connect_t    conn;
    } u;
} bcm_sbx_cosq_control_t;

typedef struct bcm_sbx_queue_size_info_s {
    int template;
    int is_allocated;
} bcm_sbx_queue_size_info_t;

#ifdef BCM_WARM_BOOT_SUPPORT

extern int
bcm_sbx_wb_cosq_state_sync(int unit, int sync);

#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_cosq_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

int32
bcm_sbx_cosq_bw_group_verify_queues(int unit, int bw_group);

uint32
bcm_sbx_cosq_get_bw_guarantee(int unit, int queue);

int32
bcm_sbx_cosq_get_num_sp_queues(int unit, int bw_group);

int32
bcm_sbx_cosq_get_num_queues_in_bag(int unit, int bw_group, bcm_sbx_cosq_queue_region_type_t queue_region, int *num_queues, int *start_queue);

int
bcm_sbx_cosq_update_given_weights(int unit, int bw_group);

int
bcm_sbx_update_templates(int unit, int nbr_modules);

int
bcm_sbx_update_template(int unit, int template);

int
bcm_sbx_cosq_get_base_queue_from_gport(int unit, bcm_gport_t gport, int *p_base_queue, int *p_num_cos_levels);

int
bcm_sbx_cosq_get_gport_cos_from_qid(int unit, int queue, bcm_gport_t *gport, bcm_cos_queue_t *dest_cosq);

int
bcm_sbx_cosq_gport_show(int unit,bcm_gport_t gport, int verbose);

int
bcm_sbx_cosq_speed_get(int unit, int port, int32 *port_speed_in_mbps);

int
bcm_sbx_cosq_num_ports_get(int unit, int interface, int32 *p_num_ports);

void
bcm_sbx_cosq_multicast_queue_group_get(int unit, bcm_fabric_distribution_t ds_id, int32 *p_base_queue);

int
bcm_sbx_cosq_multicast_ds_id_from_queue_group(int unit, int32 base_queue, bcm_fabric_distribution_t *ds_id);
  
int
bcm_sbx_gport_fifo_get(int unit, bcm_gport_t gport, int *p_fifo, bcm_module_t *module);

extern int
bcm_sbx_cosq_qinfo_get(int unit, 
		       int q,
		       int *ds_id, int *fifo,
		       bcm_sbx_cosq_queue_state_t *p_qstate, 
		       bcm_sbx_cosq_bw_group_state_t *p_bwstate, 
		       bcm_sbx_cosq_queue_region_type_t *queue_region);

extern int 
bcm_sbx_cosq_egress_group_info_get(int unit, bcm_gport_t egress_group, int *fabric_port, int *egroup_num, 
				   int *num_fifos);

extern int
bcm_sbx_cosq_egress_group_allocate(int unit, bcm_gport_t eg_requested, int modid, int child_port, 
				   int num_fifos, bcm_gport_t *p_eg_gport);

extern int
bcm_sbx_cosq_egress_group_free(int unit, bcm_gport_t eg_gport);

extern int
bcm_sirius_cosq_group_shaper_allocate(int unit, int egress, int *shaper);

extern int
bcm_sirius_cosq_group_shaper_free(int unit, int egress, int shaper);

extern int
bcm_sirius_cosq_module_congestion_allocate(int unit, bcm_module_t module, int max_ports, int *base_index);

extern int
bcm_sirius_cosq_module_congestion_deallocate(int unit, bcm_module_t module, int max_ports);

extern int
bcm_sirius_cosq_module_congestion_allocate_get(int unit, bcm_module_t module, int *max_ports, int *base_index);

extern int
bcm_sirius_cosq_module_congestion_set(int unit, bcm_port_t higig_port, bcm_module_t module);

extern int
_bcm_sbx_cosq_queue_regions_set(int unit);

extern int
bcm_sbx_cosq_sysport_to_ef_sysport(int unit, int sysport);

extern int
bcm_sbx_cosq_sysport_to_nef_sysport(int unit, int sysport);

extern int
bcm_sbx_cosq_get_eset_from_node_port(int unit, int node, int port, int mc, int *eset_p);

extern int
bcm_sbx_cosq_get_node_port_from_eset(int unit, int eset, int *node_p, int *port_p, int *mc_p);

extern int
bcm_sbx_cosq_eset_to_ef_eset(int unit, int eset);

extern int
bcm_sbx_cosq_eset_to_nef_eset(int unit, int eset);

extern int
bcm_sbx_cosq_eset_to_cos_eset(int unit, int eset, int cos);

extern int
bcm_sbx_cosq_eset_to_cos_eset_fcd(int unit, int eset, int cos, int *cos_eset, int *fcd);

extern int
bcm_sbx_cosq_get_node_port_from_sysport(int unit, int sysport, int *node_p, int *port_p, int *mc_p);

extern int
bcm_sbx_cosq_get_sysport_from_node_port(int unit, int node, int port, int mc, int *sysport_p);

extern int
_bcm_sbx_cosq_is_all(int unit, bcm_gport_t gport);

extern int
bcm_sbx_cosq_fcd_get_from_fifo(int unit, int fifo, int *fcd, int node);

extern int
bcm_sbx_cosq_gport_sysport_from_egress_object(int unit, bcm_gport_t port, int *ef_sysport, int *nef_sysport);

extern int
_bcm_sbx_cosq_queue_type_get(int unit, int32 queue, bcm_sbx_cosq_queue_region_type_t *queue_region);

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_sbx_cosq_get_state(int unit, char *pbuf);

#endif /* BCM_EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* BCM_EASY_RELOAD_SUPPORT */


#endif /* _BCM_INT_SBX_COSQ_H_ */
