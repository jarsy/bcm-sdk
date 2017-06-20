/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: port.h,v 1.30.16.9 Broadcom SDK $
 *
 * ocm.h : OCM defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_PORT_H_
#define _SBX_CALADN3_PORT_H_

#include <soc/types.h>

typedef enum sbx_caladan3_port_client_intf_type_e {
    SOC_SBX_CALADAN3_CLIENT_IL,
    SOC_SBX_CALADAN3_CLIENT_CL,
    SOC_SBX_CALADAN3_CLIENT_XT,
    SOC_SBX_CALADAN3_CLIENT_XL,
    SOC_SBX_CALADAN3_CLIENT_CMIC,
    SOC_SBX_CALADAN3_MAX_CLIENT
} sbx_caladan3_sws_client_intf_type_t;

/*
 * This enumeration is coupled with the
 * sbx_caladan3_intf_attr struct defined in port.c
 * in terms of its ordering. Coder beware.
 * New additions permitted only at the end
 */
typedef enum sbx_caladan3_port_intf_type_e {

    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE,
    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE,
    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE,
    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE,

    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10,
    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25,
    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42,
    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126,

    SOC_SBX_CALADAN3_PORT_INTF_XTPORT,

    SOC_SBX_CALADAN3_PORT_INTF_IL50w,
    SOC_SBX_CALADAN3_PORT_INTF_IL50n,
    SOC_SBX_CALADAN3_PORT_INTF_IL100,

    SOC_SBX_CALADAN3_PORT_INTF_CMIC,
    SOC_SBX_CALADAN3_PORT_INTF_XLPORT,
    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE,

    SOC_SBX_CALADAN3_MAX_PORT_INTF_TYPE
} sbx_caladan3_port_intf_type_t;


#define SOC_SBX_IS_ETH_INTF_TYPE(type) \
  (type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE ||  \
   type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE || \
   type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE || \
   type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE || \
   type == SOC_SBX_CALADAN3_PORT_INTF_XTPORT || \
   type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE ||   \
   type == SOC_SBX_CALADAN3_PORT_INTF_XLPORT)

#define SOC_SBX_IS_HG_INTF_TYPE(type) \
  (type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25 || \
   type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42 || \
   type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10 || \
   type == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126)

#define SOC_SBX_IS_ILKN_INTF_TYPE(type) \
  (type == SOC_SBX_CALADAN3_PORT_INTF_IL50w || \
   type == SOC_SBX_CALADAN3_PORT_INTF_IL50n || \
   type == SOC_SBX_CALADAN3_PORT_INTF_IL100)

#define SOC_SBX_CALADAN3_MAX_WCORE_LINE_COUNT (12)
#define SOC_SBX_CALADAN3_GIG_SPEED (1000)     /* units of Mbps */

#define SOC_SBX_CALADAN3_MAX_XLPORT_PORT (4)


/* line , fabric interfaces */
#define SOC_SBX_CALADAN3_MAX_SYSTEM_INTF (2)

/* Mac config */
#define SOC_SBX_CALADAN3_INDEPENDENT_MODE        0
#define SOC_SBX_CALADAN3_AGGREGATE_MODE          1

#define SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE   0
#define SOC_SBX_CALADAN3_CORE_PORT_DUAL_MODE     1
#define SOC_SBX_CALADAN3_CORE_PORT_QUAD_MODE     2
#define SOC_SBX_CALADAN3_CORE_PORT_DISABLE       3

#define SOC_SBX_CALADAN3_PHY_PORT_SINGLE_MODE    0
#define SOC_SBX_CALADAN3_PHY_PORT_DUAL_MODE      1
#define SOC_SBX_CALADAN3_PHY_PORT_QUAD_MODE      2


#define SOC_SBX_CALADAN3_CORE_PER_MAC  (3)
#define SOC_SBX_CALADAN3_PHY_PER_MAC   (3)

/* The #define below is used to signal the use of Expanded Port Number Space.
 * One can comment this #define out to revert to Traditional Port Number Space.
 *
 * It coincides with the same #define in sbx_drv.h.  The use of TWO identical 
 * #defines is to avoid introducing significant, physical build
 * dependencies by inclusion of sbx_drv.h here.
 */
/*#define BCM_CALADAN3_EXPANDED_PORT_NUM_SPACE 1*/

#if defined(BCM_CALADAN3_EXPANDED_PORT_NUM_SPACE)

/* Maximum number of line ports. Also currently where the reserved ports start. */
#define SOC_SBX_CALADAN3_MAX_LINE_PORT        53
#else
#define SOC_SBX_CALADAN3_MAX_LINE_PORT        48
#endif

__inline__ static int soc_sbx_caladan3_is_expanded_port_space(int unit)
{
#if defined(BCM_CALADAN3_EXPANDED_PORT_NUM_SPACE)
    return 1;
#else
    return 0;
#endif
}
 

/* The #define below is used to signal the use of Expanded Port Number Space.
 * One can comment this #define out to revert to Traditional Port Number Space.
 *
 * It coincides with the same #define in sbx_drv.h.  The use of TWO identical 
 * #defines is to avoid introducing significant, physical build
 * dependencies by inclusion of sbx_drv.h here.
 */
/*#define BCM_CALADAN3_EXPANDED_PORT_NUM_SPACE 1*/

#if defined(BCM_CALADAN3_EXPANDED_PORT_NUM_SPACE)

/* Maximum number of line ports. Also currently where the reserved ports start. */
#define SOC_SBX_CALADAN3_MAX_LINE_PORT        53
#else
#define SOC_SBX_CALADAN3_MAX_LINE_PORT        48
#endif




/* Refer to regs file */
#define SOC_SBX_CALADAN3_CLPORT0_PHY_PORT     1

/*
 * This was originally hard-coded to a  number that was 5 above MAX_LINE_PORT.  
 * However it was not documented so my best estimation is
 * it accounts for 4 XL queues and 1 CMIC queue
 */
#define SOC_SBX_CALADAN3_CLPORT1_PHY_PORT     (SOC_SBX_CALADAN3_MAX_LINE_PORT + 5)

#define SOC_SBX_CALADAN3_IL0_PHY_PORT         65
#define SOC_SBX_CALADAN3_IL1_PHY_PORT         66

#define SOC_SBX_CALADAN3_CLPORT0_L2P_IDX     ((SOC_MAX_NUM_PORTS) - 1)
#define SOC_SBX_CALADAN3_CLPORT1_L2P_IDX     ((SOC_SBX_CALADAN3_CLPORT0_L2P_IDX) - 1)
#define SOC_SBX_CALADAN3_LINE_OOB_L2P_IDX    ((SOC_SBX_CALADAN3_CLPORT0_L2P_IDX) - 2)
#define SOC_SBX_CALADAN3_FAB_OOB_L2P_IDX     ((SOC_SBX_CALADAN3_CLPORT0_L2P_IDX) - 3)

/*
Changing to below due to:
*** ERROR:  Over subscription exceeds threshold.Requested line BW[12375]G  Requested Fabric BW[127]G
after enabling ILKN on Line Side
 
#define SOC_SBX_CALADAN3_ILKN50w_SPEED 6250
#define SOC_SBX_CALADAN3_ILKN50n_SPEED 6187
#define SOC_SBX_CALADAN3_ILKN100_SPEED 12375
*/

#define SOC_SBX_CALADAN3_ILKN50w_SPEED 63
#define SOC_SBX_CALADAN3_ILKN50n_SPEED 62
#define SOC_SBX_CALADAN3_ILKN100_SPEED 124

typedef struct sbx_caladan3_intf_attr_s {
    char   *name; /* used as key for parsing soc parameters */
    uint32  speed; /* speed */
    int     max_instance_per_blk; /* eg., xport (3) per clport block, ge - 1 per clport block */
    int     max_port_per_instance; /* eg., ge - 12 ports per instance */
    soc_block_type_t type;
    int     wcore_lines_per_port; /* number of warp core line required for port 0-N/A */
    int     channelized;          /* if the interface is channelized */
    int     rate_mbps;            /* effective port rates in mbps */
    int     max_pkt_rate_kpps;    /* max packet rate in kpps */
    int     interval;             /* port interval for calendar scheduling */
    int     pt_fifo_level;        /* pt fifo allocation */
} sbx_caladan3_intf_attr_t;

/* Caladan 3 Interface */
/* LIF - Physical Line Interface */
/* 1GE, 10GE, 40GE, 100GE, HG42, HG25, ILKN50w, ILKN50n, ILKN100n */
/* FIF - Fabric Interface  */
/* 10GE, HG25, HG42, HG126, ILKN50w, ILKN50n, ILKN100n */
/* ucode_port.1=xe0.0:hg126.0 */
#define SOC_SBX_CALADAN3_LINE_WCORE   (0)
#define SOC_SBX_CALADAN3_FABRIC_WCORE (1)

typedef enum soc_sbx_caladan3_flow_control_type_e {
    SOC_SBX_CALADAN3_FC_TYPE_NONE      = 0,
    SOC_SBX_CALADAN3_FC_TYPE_PAUSE     = 1,
    SOC_SBX_CALADAN3_FC_TYPE_SAFC      = 2, 
    SOC_SBX_CALADAN3_FC_TYPE_HCFC_OOB  = 3, /* OOB only */
    SOC_SBX_CALADAN3_FC_TYPE_ILKN      = 4, /* Inband */
    SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB  = 5,
    SOC_SBX_CALADAN3_FC_TYPE_MAX       
} soc_sbx_caladan3_flow_control_type_t;

#define SOC_SBX_CALADAN3_IS_FLOW_CTRL_ENABLED(val) \
                ((val) >= SOC_SBX_CALADAN3_FC_TYPE_NONE && \
                 (val) <= SOC_SBX_CALADAN3_FC_TYPE_MAX)


/*
 * Port queues
 *     Each port will have either a single sq/dq or a set of sq/dqs
 *     based on if they are channelized or non-channelized ports
 *     If channelization is used, PR ICC configuration might be needed
 *     to steer the packet into the right queue
 */
#define SOC_SBX_CALADAN3_MAX_SOURCE_QUEUES  (128) 

#define SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES  (2) 
#define SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES  (2)

/* Queue Type */
#define INGRESS_SQUEUE (0)    
#define EGRESS_SQUEUE  (1)    
#define INGRESS_DQUEUE (2)
#define EGRESS_DQUEUE  (3)


typedef struct soc_sbx_caladan3_queues_s {
    int spri0_qid;   /* strict priority queue 0 */
    int spri1_qid;   /* strict priority queue 1 */
    int spri_set;    /* Enable strict priority queues, valid only on channelized port */
    int squeue_base; /* source queue in case of non-channelized ports, lowest queue in case of channelized */
    int dqueue_base; /* dest queue in case of non-channelized ports, lowest queue in case of channelized */
    int num_squeue;  /* > 1 for channelized case */
    int num_dqueue;  /* > 1 for channelized case */
    uint32 squeue_bmp[SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES]; /* source queue bitmap */
    uint32 dqueue_bmp[SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES]; /* dest queue bitmap   */
    /*Following 3 state vars for hotswap support*/
    int squeue_base_pr_icc_table_index;
    int squeue_base_pr_icc_table_spread;
    int squeue_base_pr_icc_table_index_valid;
} soc_sbx_caladan3_queues_t;



/*
 * Port Map Info
 *    This structure represents either a Line port or a Fabric port
 */
typedef struct soc_sbx_caladan3_port_map_info_s {
    int blk;       /* block number for this port   */
    int instance;  /* block instance for this port */
    int bindex;    /* index within the block       */
    int port;      /* port or channel number       */
    int base_port; /* base port if channelized or cos port */
    int uport;     /* corresponding ucode port */
    int intf_instance;    /* interface type instance */
    soc_block_type_t blktype; /* block type        */
    sbx_caladan3_port_intf_type_t intftype; /* if type */
    sbx_caladan3_sws_client_intf_type_t clienttype; /* if type */
    int physical_intf; /* 0 - line side wcore, 1- fabric side wcore */
    soc_sbx_caladan3_flow_control_type_t flow_control;
    soc_sbx_caladan3_queues_t port_queues; /* Queues associated with the port */
    int refcnt;  
    uint32 flags;
} soc_sbx_caladan3_port_map_info_t;

/* stored in soc_sbx_caladan3_port_map_info_t->flags */
#define SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT 0x1
#define SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID   0x2

/* Number of line entries to allocate in table */
#define SOC_SBX_CALADAN3_PORT_MAP_ENTRIES       64

#define SOC_SBX_CALADAN3_INTF_MAX_INST_PER_BLK (3)

#define CALADAN3_LANE_USED 1
#define CALADAN3_LANE_FREE 0

typedef struct soc_sbx_caladan3_pbmp2idx_s {
    int line;  /* TRUE if line port, FALSE otherwise */
    int portidx;
} soc_sbx_caladan3_pbmp2idx_t;

typedef struct soc_sbx_caladan3_port_map_s {
    soc_sbx_caladan3_port_map_info_t *line_port_info;
    soc_sbx_caladan3_port_map_info_t *fabric_port_info;
    int max_line_bw;
    int max_fabric_bw;
    int max_port;
    int first_reserved_port;    /* This will be the CMIC port */
    int num_ports_found;
    int borrowed_wcore_count; 
    int num_1g_ports;
    sbx_caladan3_intf_attr_t *intf_attr;
    int line_serdes_usage[SOC_SBX_CALADAN3_MAX_WCORE_LINE_COUNT];
    int fab_serdes_usage[SOC_SBX_CALADAN3_MAX_WCORE_LINE_COUNT];
    soc_sbx_caladan3_pbmp2idx_t pbmp2idx[SOC_MAX_NUM_PORTS];
    int default_hpte_hiprio_ingress_squeues[3];
    int default_hpte_hiprio_egress_squeues[3];
    int reserved_ports_configured;
    int queue_mapping[SOC_SBX_CALADAN3_MAX_SOURCE_QUEUES]; /* Indexed by Squeue, yeilds Dqueue */
} soc_sbx_caladan3_port_map_t;

#define SOC_SBX_CALADAN3_NUM_RESV_PORTS (1 + SOC_SBX_CALADAN3_MAX_XLPORT_PORT)
#define SOC_SBX_CALADAN3_RESV_XL_OFFSET (0)
#define SOC_SBX_CALADAN3_RESV_CMIC_OFFSET (2)

typedef struct soc_sbx_caladan3_port_config_s {
    uint32  valid;              /* True if entry is valid */
    uint32  encaps;             /* Encapsulation type */
    uint32  speed;              /* Speed if not defined by interface type */
    uint32  phy_port;           /* related phy port. */
    soc_port_if_t if_type;    /* interface type */
} soc_sbx_caladan3_port_config_t;

typedef struct soc_sbx_caladan3_port_queue_s {
    uint32          valid;              /* True if entry is valid */
    int             line_sq_base;
    int             fabric_sq_base;
    int             sq_count;
    int             line_dq_base;
    int             fabric_dq_base;
    int             dq_count;
    soc_port_t      fabric_port;
} soc_sbx_caladan3_port_queue_t;

/* C3 Clocks */
#define SOC_SBX_CALADAN3_CORE_CLOCK 416



extern int soc_sbx_caladan3_port_info_load(int unit);
extern int soc_sbx_caladan3_port_info_update(int unit);
extern int soc_sbx_caladan3_port_queues_update_bmp(int unit, soc_sbx_caladan3_queues_t *queues);
extern int soc_sbx_caladan3_reconfig_port_cleanup(int unit);
extern int soc_sbx_caladan3_hotswap(int unit);
extern void soc_sbx_caladan3_port_info_dump(int unit, int verbose, int port);
extern void soc_sbx_caladan3_port_init(int unit);
extern int soc_sbx_caladan3_port_remove(int unit, soc_port_t port);
extern int soc_sbx_caladan3_update_interface_config(int unit, soc_sbx_caladan3_port_config_t *config);
extern int soc_sbx_caladan3_flush_ports(int unit, pbmp_t *pbmp);
extern int soc_sbx_caladan3_cmic_port_get(int unit, int *port);
extern int soc_sbx_caladan3_get_intftype(int unit, int port, int *intftype);
extern int soc_sbx_caladan3_port_info_get(int unit, int port, soc_sbx_caladan3_port_map_info_t **info);
extern int soc_sbx_caladan3_port_is_wc_remapped(int unit);
int soc_sbx_caladan3_mac_init(int unit);
int soc_sbx_caladan3_is_line_port(int unit, soc_port_t port);
int soc_sbx_caladan3_port_to_phyid(int unit, int port, int *phyidx);
int soc_sbx_caladan3_get_max_ports(int unit);
int soc_sbx_caladan3_port_speed_get(int unit, int port, int *speed);
int soc_sbx_caladan3_xgxs_reset(int unit, int port, int wcidx);
int soc_sbx_caladan3_fabric_port_client_type(int unit, soc_port_t port) ;
int soc_sbx_caladan3_port_flow_control_set(int unit, int port);
int soc_sbx_caladan3_port_flow_control_mode_set(int unit, int port, 
                                                soc_sbx_caladan3_flow_control_type_t mode);
int soc_sbx_caladan3_port_flow_control_mode_get(int unit, int port, 
                                                soc_sbx_caladan3_flow_control_type_t *mode);
int soc_sbx_caladan3_port_oob_fc_config(int unit, int line, int fc_type);
int soc_sbx_caladan3_port_is_channelized_subport(int unit, soc_port_t port, int *is_channelized,
    int *requires_phy_setup);
extern int soc_sbx_caladan3_il_oob_init(int unit, int ifnum, int fc_type);

/*
 * Function: soc_sbx_caladan3_port_queues_add
 * Purpose: 
 *      add the queues to the port
 *      type = INGRESS_SQUEUE/INGRESS_DQUEUE/EGRESS_DQUEUE/EGRESS_SQUEUE
 */
int
soc_sbx_caladan3_port_queues_add(int unit, soc_port_t port, int qid, int type);

/*
 * Function: soc_sbx_caladan3_port_queues_remove
 * Purpose: 
 *      remove the queue from port
 *      type = INGRESS_SQUEUE/INGRESS_DQUEUE/EGRESS_DQUEUE/EGRESS_SQUEUE
 */
int
soc_sbx_caladan3_port_queues_remove(int unit, soc_port_t port, int qid, int type);

/*
 * Function: soc_sbx_caladan3_port_queues_get
 * Purpose: 
 *      Get the queues associated with the port
 *      dir = 0 for ingress, 1 for egress
 */
int
soc_sbx_caladan3_port_queues_get(int unit, soc_port_t port, int dir, soc_sbx_caladan3_queues_t **queueinfo);

/*
 * Function: soc_sbx_caladan3_il_oob_hcfc_remap_default
 * Purpose:  Setup hcfc remap to default one to one mapping
 * Input:    ifnum -> 0/1 for line or fabric side hcfc
 *           unit  -> bcm unit number
 *           dir  -> 1 = TX, 2 = RX, 3 = Both
 * Output:   SOC_E_NONE on success, else one of SOC_E_* error codes
 */
extern int
soc_sbx_caladan3_il_oob_hcfc_remap_default(int unit, int ifnum, int dir);
/*
 * Function: soc_sbx_caladan3_il_oob_hcfc_remap_set
 * Purpose:  Setup hcfc remap to default one to one mapping
 * Input:    ifnum -> 0/1 for line or fabric side hcfc
 *           unit  -> bcm unit number
 *           dir   -> 1 = TX, 2 = RX, 3 = Both
 *           channel -> FC channel index
 *           Qid   -> Source/Dest queue that will be mapped to index
 * Output:   SOC_E_NONE on success, else one of SOC_E_* error codes
 */
extern int
soc_sbx_caladan3_il_oob_hcfc_remap_set(int unit, int ifnum, int qid, int dir, int channel);
/*
 * Function: soc_sbx_caladan3_il_oob_hcfc_remap_get
 * Purpose:  Setup hcfc remap to default one to one mapping
 * Input:    ifnum -> 0/1 for line or fabric side hcfc
 *           unit  -> bcm unit number
 *           dir   -> 1 = TX, 2 = RX
 *           Qid   -> Source/Dest queue that will be mapped to index
 * Outputs:
 *           channel -> FC channel index
 *           SOC_E_NONE on success, else one of SOC_E_* error codes
 */
extern int
soc_sbx_caladan3_il_oob_hcfc_remap_get(int unit, int ifnum, int queue, int dir, int *channel);

int soc_sbx_caladan3_take_mac_out_of_reset(int unit, int port);


int
soc_sbx_caladan3_port_enable(int unit, int port, int subport, int enable);
int soc_sbx_caladan3_port_reset_mib_counters(int unit, int port, int subport);

#endif /* _SBX_CALADN3_PORT_H_ */
