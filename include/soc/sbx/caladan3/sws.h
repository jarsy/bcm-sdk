/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: sws.h,v 1.24.16.4 Broadcom SDK $
 *
 * sws.h : SWS defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_SWS_H_
#define _SBX_CALADN3_SWS_H_

#include <soc/types.h>
#include <soc/sbx/caladan3/sws_params.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/util.h>

typedef enum {
    LINE_SIDE,
    FABRIC_SIDE,
    MAX_LINE_FABRIC_ID
} line_fabric_id_t;


typedef struct soc_sbx_caladan3_sws_buffer_param_s {
    int cmic_page_max;
    int cmic_page_min;
    int cmic_hdr_page_min;
    int xlport_page_max;
    int xlport_page_min;
    int xlport_hdr_page_min;
    int rt_queue_page_max;
    int rt_queue_page_min;
    int rt_queue_hdr_page_min;
    int bubble_queue_page_max;
    int bubble_queue_page_min;
    int bubble_queue_hdr_page_min;
    int num_static_pages;
    int intf_buffer_usage_percent;
    int queue_buffer_usage_percent;
    int de1_threshold_percent;
    int de2_threshold_percent;
    int delta_threshold_percent;
    int min_page_percent;
    int min_hdr_page_percent;
    int min_threshold_min_pages;
    int min_threshold_min_hdr_pages;
    int min_queue_max_pages;
} soc_sbx_caladan3_sws_buffer_param_t;

typedef struct soc_sbx_caladan3_sws_intf_param_s {
    int min_ether_frame_size;
    int ether_mtu;
    int ether_ifg;
    int higig_hdr_size;
    int higig_ifg;
    int min_il_frame_size;
    int il_mtu;
    int il_ifg;
} soc_sbx_caladan3_sws_intf_param_t;


typedef struct soc_sbx_caladan3_sws_flow_ctrl_thres_param_s {
    /* Worst-case time from detection in C3 to response at attached device scheduler */
    int hcfc_latency_usec;
    /* Worst-case data transmitted after stopping scheduling */
    int hdfc_xoff_kb;
    int il_channel_latency_usec;
    int il_channel_xoff_kb;
    /* Squelch time for sending XOFF->XON transitions */
    int safc_min_timer_nsec;
    /* Flow control latency in usecs for link level flow control. 
       Most devices have no additional scheduler latency as with channelized flow control. */
    int safc_xoff_latency_nsec;
    int il_llfc_latency_nsec;
    int intf_flow_ctrl_percent; /* Specify actual interface thresholds as percentage of default value */
    int queue_flow_ctrl_percent; /* Specify actual queue thresholds as percentage of default value */
    int min_flow_ctrl_threshold; /* Minimum flow control threshold for queues */
} soc_sbx_caladan3_sws_flow_ctrl_thres_param_t;

typedef struct soc_sbx_caladan3_sws_lrp_param_s {
    int lrp_num_pe;
    int line_intf_lrp_cxt;
    int fabric_intf_lrp_cxt;
    int lrp_max_inst_per_epoch;
    int lrp_max_epoch_time_nsec;
    int lrp_inst_per_epoch;
    int lrp_epoch_time_nsec;
    int lrp_max_kpps;
    int lrp_line_latency_nsec;
    int lrp_fabric_latency_nsec;
} soc_sbx_caladan3_sws_lrp_param_t;

typedef struct soc_sbx_caladan3_sws_common_param_s {
    int LineRxLogicalShapedKpps;
    int FabricRxLogicalShapedKpps;
    int LineRxLogicalKpps;
    int FabricRxLogicalKpps;
    /* Min page / hdr computation for line/fabric */
    int line_min_pages;
    int line_min_hdr_pages;
    int fabric_min_pages;
    int fabric_min_hdr_pages;
    /* flow control */
    int line_head_room;
    int fabric_head_room;
    int line_enq_req_head_room_pages;
    int fabric_enq_req_head_room_pages;
    int line_queue_fc_threshold_sum;
    int fabric_queue_fc_threshold_sum;
    int line_min_max_pages;
    int fabric_min_max_pages;
    /* Max page compuatation */
    int buffer_max_pages;
    int buffer_min_pages;
    int total_line_queue_max_pages;
    int total_fabric_queue_max_pages;
    int line_max_pages;
    int fabric_max_pages;
} soc_sbx_caladan3_sws_common_param_t;

/* static system parameters */
typedef struct soc_sbx_caladan3_sws_config_s {
    soc_sbx_caladan3_sws_buffer_param_t buffer;
    soc_sbx_caladan3_sws_intf_param_t intf;
    soc_sbx_caladan3_sws_flow_ctrl_thres_param_t flowctrl;
    soc_sbx_caladan3_sws_lrp_param_t lrp;
    soc_sbx_caladan3_sws_common_param_t common;
} soc_sbx_caladan3_sws_config_t;


#define SOC_SBX_CALADAN3_SWS_QM_COSQ_INVALID (-1)
#define SOC_SBX_CALADAN3_SWS_QM_MAX_QUEUES (256)
#define SOC_SBX_CALADAN3_SWS_MAX_PR_INSTANCE (2)
#define SOC_SBX_CALADAN3_SWS_MAX_PR_POLICER (128)
#define SOC_SBX_CALADAN3_SWS_MAX_HPTE_MAP_ENTRIES (128)

/* PR Policer */
#define SOC_SBX_CALADAN3_PR_POLICER_COLOR_BLIND   0x00000001
#define SOC_SBX_CALADAN3_PR_POLICER_DROP_ON_RED   0x00000002
#define SOC_SBX_CALADAN3_PR_POLICER_OVERFLOW      0x00000004
#define SOC_SBX_CALADAN3_PR_POLICER_RFC2698       0x00000008
#define SOC_SBX_CALADAN3_PR_POLICER_BKT_C_STRICT  0x00000010
#define SOC_SBX_CALADAN3_PR_POLICER_BKT_E_STRICT  0x00000020
#define SOC_SBX_CALADAN3_PR_POLICER_BKT_C_LOCKED  0x00000040
#define SOC_SBX_CALADAN3_PR_POLICER_BKT_E_LOCKED  0x00000080
#define SOC_SBX_CALADAN3_PR_POLICER_PKT_MODE      0x00000100

/* PR Queue Action */
#define SOC_SBX_CALADAN3_PR_QUEUE_ACTION_DEFAULT   0
#define SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP    2
#define SOC_SBX_CALADAN3_PR_QUEUE_ACTION_INDEXED   3 /* Default queue + Lookup queue */

/* PR TCAM Valid */
#define SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID   3
#define SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_INVALID   0


typedef struct soc_sbx_caladan3_pr_policer_s {
    uint32 cir;
    uint32 cbs;
    uint32 eir;
    uint32 ebs;
    uint32 flags;
} soc_sbx_caladan3_pr_policer_t;

typedef struct soc_sbx_caladan3_pr_policer_cfg_s {
    uint32 refcnt[SOC_SBX_CALADAN3_SWS_MAX_PR_POLICER];
    soc_sbx_caladan3_pr_policer_t policer[SOC_SBX_CALADAN3_SWS_MAX_PR_POLICER];
    int enabled;
} soc_sbx_caladan3_sws_pr_policer_cfg_t;
 
 
typedef struct soc_sbx_caladan3_pr_icc_lookup_data_s {
    uint32 state;
    uint32 shift;
    uint32 select_de;
    uint32 queue_action;
    uint32 queue;
    uint32 last;
    uint32 drop;
    uint32 dp;
    uint32 default_de;
    uint32 cos;
} soc_sbx_caladan3_pr_icc_lookup_data_t;

/* sws queue database */
typedef struct soc_sbx_caladan3_sws_queue_info_s {
    int port; /* associated ucode/front panel port */
    int qid;  /* Qid of this entry */
    int cos; /* <0 invalid */
    int enabled; /* 0 disabled queue */
    sws_qm_source_queue_cfg_t *squeue_config;
} soc_sbx_caladan3_sws_queue_info_t;

typedef struct soc_sbx_caladan3_hpte_map_entry_s {
    int ing_port; /* PBMP port */
    int egr_port; /* PBMP port */
    uint8 squeue; /* 0-128   */
    uint8 dqueue; /* 128-256 */
    uint8 dir;    /* 0 ingress, 1 egress */
    uint8 valid;  /* 0 invalid, 1 valid, 2 dirty */
} soc_sbx_caladan3_hpte_map_entry_t;


typedef struct soc_sbx_calandan3_icc_state_info_s
{
    uint8   port;
    uint8   profile;
    uint8   stage;
    uint8   flag;
}soc_sbx_calandan3_icc_state_info_t;

/* Estimate of free pages in FIFO waiting to be freed */
#define SOC_SBX_CALADAN3_SWS_FREE_PAGE_FIFO_OCCUPANCY (300)


/* flow control parameters */
#define SOC_SBX_CALADAN3_SWS_DEFAULT_HCFC_LATENCY_USEC (2)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_XOFF_KB (10)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_IL_CH_LATENCY (3/10)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_IL_XOFF_KB (10)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_SAFC_MIN_TIMER (1000)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_SAFC_XOFF_LATENCY (200)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_IL_LLFC_LATENCY (300)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_INTF_FLOW_CTRL_PERCENT (90)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_QUEUE_FLOW_CTRL_PERCENT (105)
#define SOC_SBX_CALADAN3_SWS_DEFAULT_MIN_FLOW_CTRL_THRESHOLD_PAGES (300)

/* Physical size of enqueue request FIFO in port receive (PR) */
#define SOC_SBX_CALADAN3_SWS_ENQ_REQ_FIFO_SIZE (256)

#define SOC_SBX_CALADAN3_MAX_CI_CONFIG (3)
#define SOC_SBX_CALADAN3_SWS_PR_RX_PORT_PAGE_BUFFER_MAX (612)

#define SOC_SBX_CALADAN3_SWS_PR_ICC_ENABLE  (0)
#define SOC_SBX_CALADAN3_SWS_PR_ICC_BYPASS  (1)

#define SOC_SBX_SWS_PR_RX_PORT_PAGE_GE_PORT (12)

/* PR Flow control mappings */
#define SOC_SBX_CALADAN3_SWS_PR_FC_SQUEUE_XOFF       (1)
#define SOC_SBX_CALADAN3_SWS_PR_FC_FABRIC_LLFC_XOFF  (2)
#define SOC_SBX_CALADAN3_SWS_PR_FC_LINE_LLFC_XOFF    (4)
#define SOC_SBX_CALADAN3_SWS_PR_FC_ENQR_FIFO_XOFF    (8)

#define SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE (2)
#define SOC_SBX_CALADAN3_SWS_PT_MAX_IPTE_CONFIG_REG (3)
#define SOC_SBX_CALADAN3_SWS_PT_CLIENT_MAX (6)

#define SOC_SBX_CALADAN3_SWS_PT_QM_PB_LATENCY_NSEC (353)
#define SOC_SBX_CALADAN3_SWS_PT_32B_LINE_PER_PAGE (8)
#define SOC_SBX_CALADAN3_SWS_PT_PORT_FIFO_DEPTH_32B (816)
#define SOC_SBX_CALADAN3_SWS_PT_PORT_FIFO_DEPTH_PAGES (816/8)
#define SOC_SBX_CALADAN3_SWS_PT_PORT_FIFO_MIN_DEPTH (2)

/* QM Pause interface deltas */
#define SOC_SBX_CALADAN3_SWS_QM_PAUSE_HDR_MIN_PAGES_DELTA         (8)
#define SOC_SBX_CALADAN3_SWS_QM_PAUSE_MAX_PAGES_DELTA             (24)
#define SOC_SBX_CALADAN3_SWS_QM_PAUSE_INTF_MAX_PAGES_DELTA        (24)
#define SOC_SBX_CALADAN3_SWS_QM_PAUSE_TOTAL_RSVD_PAGES_DELTA      (52)
#define SOC_SBX_CALADAN3_SWS_QM_PAUSE_TOTAL_BUFF_MAX_PAGES_DELTA  (52)


/*
 * Ingress               Egress
 *   +----------------------+
 *   |   0-63  ---> 128-191 |  
 *   | 192-255 <---  64-127 |  
 *   +----------------------+
 */
#define SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE (0)
#define SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE (192)
#define SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE (64)
#define SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE (128)

#define SOC_SBX_CALADAN3_SWS_MIN_QUEUE_ID (255)
#define SOC_SBX_CALADAN3_SWS_MAX_QUEUE_ID (255)
#define SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES (128)
#define SOC_SBX_CALADAN3_SWS_MAX_EGRESS_QUEUES  (128)
#define SOC_SBX_CALADAN3_SWS_MAX_SOURCE_QUEUES  (128)

#ifndef SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION
#define SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION (64)
#endif


/* special queues */
#define SOC_SBX_CALADAN3_SWS_ING_MAX_SPECIAL_QUEUE (3)
#define SOC_SBX_CALADAN3_SWS_EGR_MAX_SPECIAL_QUEUE (3)

#define SOC_SBX_CALADAN3_SWS_ING_NUM_RESERVED_QUEUES        \
            (SOC_SBX_CALADAN3_NUM_RESV_PORTS)

#define SOC_SBX_CALADAN3_SWS_EGR_NUM_RESERVED_QUEUES (0)

#define SOC_SBX_CALADAN3_SWS_AVAILABLE_INGRESS_QUEUES            \
            (SOC_SBX_CALADAN3_SWS_MAX_INGRESS_QUEUES -           \
                 SOC_SBX_CALADAN3_SWS_ING_NUM_RESERVED_QUEUES -  \
                     SOC_SBX_CALADAN3_SWS_ING_MAX_SPECIAL_QUEUE)

#define SOC_SBX_CALADAN3_SWS_AVAILABLE_EGRESS_QUEUES             \
           (SOC_SBX_CALADAN3_SWS_MAX_EGRESS_QUEUES -             \
                SOC_SBX_CALADAN3_SWS_EGR_NUM_RESERVED_QUEUES -   \
                      SOC_SBX_CALADAN3_SWS_EGR_MAX_SPECIAL_QUEUE)

#define SOC_SBX_CALADAN3_SWS_INGRESS_XLPORT1_SQUEUE    (SOC_SBX_CALADAN3_MAX_LINE_PORT)
#define SOC_SBX_CALADAN3_SWS_INGRESS_XLPORT2_SQUEUE    (SOC_SBX_CALADAN3_SWS_INGRESS_XLPORT1_SQUEUE + 1)
#define SOC_SBX_CALADAN3_SWS_INGRESS_CMIC_SQUEUE       (SOC_SBX_CALADAN3_SWS_INGRESS_XLPORT1_SQUEUE + 2)

#define SOC_SBX_CALADAN3_SWS_INGRESS_BUBBLE_SQUEUE     (61)
#define SOC_SBX_CALADAN3_SWS_FAB_TO_LINE_REDIRECT_QID0 (62)
#define SOC_SBX_CALADAN3_SWS_FAB_TO_LINE_REDIRECT_QID1 (63)

#define SOC_SBX_CALADAN3_SWS_EGRESS_XLPORT1_SQUEUE \
(SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + SOC_SBX_CALADAN3_SWS_INGRESS_XLPORT1_SQUEUE)

#define SOC_SBX_CALADAN3_SWS_EGRESS_XLPORT2_SQUEUE \
(SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + SOC_SBX_CALADAN3_SWS_INGRESS_XLPORT2_SQUEUE)

#define SOC_SBX_CALADAN3_SWS_EGRESS_CMIC_SQUEUE  \
(SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + SOC_SBX_CALADAN3_SWS_INGRESS_CMIC_SQUEUE)

#define SOC_SBX_CALADAN3_SWS_EGRESS_BUBBLE_SQUEUE \
(SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + SOC_SBX_CALADAN3_SWS_INGRESS_BUBBLE_SQUEUE)

#define SOC_SBX_CALADAN3_SWS_LINE_TO_FAB_REDIRECT_QID0 \
(SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + SOC_SBX_CALADAN3_SWS_FAB_TO_LINE_REDIRECT_QID0)

#define SOC_SBX_CALADAN3_SWS_LINE_TO_FAB_REDIRECT_QID1 \
(SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE + SOC_SBX_CALADAN3_SWS_FAB_TO_LINE_REDIRECT_QID1)


#define SOC_SBX_CALADAN3_SWS_XL_QUEUE_BASE    (SOC_SBX_CALADAN3_SWS_INGRESS_XLPORT1_SQUEUE)
#define SOC_SBX_CALADAN3_SWS_CMIC_QUEUE_BASE  (SOC_SBX_CALADAN3_SWS_INGRESS_CMIC_SQUEUE)

#define SOC_SBX_CALADAN3_SWS_MAX_PAGES  (0x3FFF)
#define SOC_SBX_CALADAN3_SWS_BUBBLE_RESERVED_PAGES  (256)

/* Currently only reserved for bubbles */
#define SOC_SBX_CALADAN3_SWS_RESERVED_PAGES  (SOC_SBX_CALADAN3_SWS_BUBBLE_RESERVED_PAGES)

#define SOC_SBX_CALADAN3_SWS_MAX_PAGES_USABLE \
          (SOC_SBX_CALADAN3_SWS_MAX_PAGES - SOC_SBX_CALADAN3_SWS_RESERVED_PAGES)

#define SOC_SBX_CALADAN3_SWS_PAGE_REFCNT_MAX (0xfff)
#define SOC_SBX_CALADAN3_SWS_PAGE_RESERVED   (0xf)


#define SOC_SBX_CALADAN3_DQUEUE_QUEUE_PER_GROUP (32)
#define SOC_SBX_CALADAN3_DQUEUE_MAX_GROUP (4)


#define SOC_SBX_CALADAN3_SWS_PT_MAX_CHANNELIZED_PORT (3)
#define SOC_SBX_CALADAN3_SWS_PT_MAX_CHANNELIZED_PORT_REVB (12)

#define SOC_SBX_CALADAN3_SWS_XT_PORT_BASE   (12)
/* XL0/XL1/CMIC are hardcoded to ports 48/49/50 on PR */
#define SOC_SBX_CALADAN3_SWS_XL_PORT_BASE   (48)
#define SOC_SBX_CALADAN3_SWS_CMIC_PORT_BASE (50)

/* Default de1/de2 tresholds */
#define SOC_SBX_CALADAN3_SWS_GLOBAL_DE1_TRESHOLD_PERCENT         (90)
#define SOC_SBX_CALADAN3_SWS_GLOBAL_DE2_TRESHOLD_PERCENT         (85)
#define SOC_SBX_CALADAN3_SWS_DE1_TRESHOLD_PERCENT         (75)
#define SOC_SBX_CALADAN3_SWS_DE2_TRESHOLD_PERCENT         (50)
#define SOC_SBX_CALADAN3_SWS_FC_TRESHOLD_PERCENT          (90)
#define SOC_SBX_CALADAN3_SWS_GLOBAL_HYST_DELTA_PERCENT    (10)
#define SOC_SBX_CALADAN3_SWS_INGRESS_HYST_DELTA_PERCENT   (10)
#define SOC_SBX_CALADAN3_SWS_EGRESS_HYST_DELTA_PERCENT    (10)
/* Assumed 9000 (MAX_MTU) / 256 */
#define SOC_SBX_CALADAN3_SWS_QUEUE_HYSTERIS_DELTA         (35)


#define PR_INSTANCE(inst) SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE((inst))
#define PR0_INSTANCE (SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(0))
#define PR1_INSTANCE (SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(1))

/*
 * QM SWS helpers 
 */
sws_qm_config_t *
soc_sbx_caladan3_sws_qm_cfg_get(int unit);

soc_sbx_caladan3_sws_queue_info_t *
soc_sbx_caladan3_sws_queue_info_get(int unit);

extern int
soc_sbx_caladan3_sws_queue_alloc(int unit, int start_queue, int num_queues);
extern int
soc_sbx_caladan3_sws_queue_free(int unit, int start_queue, int num_queues);

int 
soc_sbx_caladan3_sws_qm_init(int unit);

int 
soc_sbx_caladan3_sws_pb_init(int unit);

extern char *
soc_sbx_caladan3_sws_config_param_str_get(int unit, char *str, int port);

extern int
soc_sbx_caladan3_sws_config_param_data_get(int unit, char *str, int port, int defl);

extern int
soc_sbx_caladan3_buffer_param_config_get(int unit, sws_qm_buffer_cfg_t *buf_cfg);

/*
 * PR SWS helpers 
 */
int
soc_sbx_caladan3_sws_pr_port_buffer_cfg_is_valid(int unit, int instance);

sws_pr_port_buffer_t *
soc_sbx_caladan3_sws_pr_port_buffer_cfg_get(int unit, int instance);

sws_pr_idp_thresholds_config_t *
soc_sbx_caladan3_sws_pr_idp_cfg_get(int unit, int instance);

int 
soc_sbx_caladan3_sws_pr_init(int unit);



/* 
 * PT SWS helpers 
 */
soc_sbx_caladan3_sws_pt_cfg_t *
soc_sbx_caladan3_sws_pt_cfg_get(int unit, int instance);

int 
soc_sbx_caladan3_sws_pt_init(int unit);

extern int
soc_sbx_caladan3_sws_pt_fifo_auto_set(int unit, sws_config_t *sws_cfg);
extern int
soc_sbx_caladan3_sws_pt_client_calendar_auto_set(int unit, sws_config_t *sws_cfg);
extern int
soc_sbx_caladan3_sws_pt_port_calendar_auto_set(int unit, sws_config_t *sws_cfg);
extern int
soc_sbx_caladan3_sws_pt_port_calendar_config(int unit, int instance, sws_pt_port_cal_t *cal);
extern int
soc_sbx_caladan3_sws_pt_client_calendar_config(int unit, int instance, sws_pt_client_cal_t *cal);
extern int
soc_sbx_caladan3_pt_port_credit_set_hotswap(int unit);
extern int
soc_sbx_caladan3_pt_port_fifo_alloc(int unit, int instance, sws_pt_port_fifo_t *pt_fifo);
extern int
soc_sbx_caladan3_pt_port_fifo_set_hotswap(int unit, int instance, sws_pt_port_fifo_t *pt_fifo);
extern int
soc_sbx_caladan3_sws_pt_port_enable_all_rev_b(int unit);
extern int
soc_sbx_caladan3_pt_port_update(int unit, int instance, int port, int page_threshold,
    int size, int base, int update_fifo,
    int credits, int update_credits);
extern int
soc_sbx_caladan3_sws_pt_os_scheduler_config(int unit);

/*
 * PT SWS Loopback diagnostics helpers 
 */
int
soc_sbx_caladan3_sws_cmic_mac_loopback_set(int unit, int cmic_port, int port, int enable);
int
soc_sbx_caladan3_sws_cmic_hpp_loopback_dir_set(int unit, int ingress, int enable);
int
soc_sbx_caladan3_sws_cmic_hpp_loopback_map_set(int unit, int cmic_port, int enable);

/*
 * Port Module SWS helpers 
 */
int
soc_sbx_caladan3_sws_pbmp_to_line_port(int unit, int port, int *index);
int
soc_sbx_caladan3_sws_pbmp_to_fab_port(int unit, int port, int *index);


/* 
 * TDM SWS helpers 
 */
char *
soc_sbx_caladan3_sws_tdm_name(int id);

int
soc_sbx_caladan3_sws_tdm_lookup(int unit, int *tdmid, sws_config_t **sws_cfg);

int
soc_sbx_caladan3_sws_tdm_config_init(int unit);

extern tdmid_t
soc_sbx_caladan3_sws_tdm_id_current(int unit);

extern tdmid_t
soc_sbx_caladan3_sws_tdm_id_last(int unit);

extern int
soc_sbx_caladan3_sws_tdm_is_swapable(int unit);

/* 
 *
 * SWS externally usable functions 
 *  Keep all internal module functions above this
 */

extern void
soc_sbx_caladan3_sws_pr_icc_program_port_match_entries(int unit, 
                                                       int remove_entry, 
                                                       int q_to_remove,
                                                       int dont_program_resv_ports);
extern
int soc_sbx_caladan3_sws_pr_port_enable_all(int unit);

extern
int soc_sbx_caladan3_sws_pr_port_enable(int unit, int port, int enable);

extern
int soc_sbx_caladan3_pr_port_buffer_set(int unit, int instance,
                                        sws_pr_port_buffer_t *pr_buf);
extern
int soc_sbx_caladan3_pr_port_buffer_alloc(int unit, int instance,
                                          sws_pr_port_buffer_t *pr_buf);

extern int 
soc_sbx_caladan3_sws_pt_shaper_set(int unit, int queue, uint32 kbits_sec, uint32 kbits_burst);

extern int 
soc_sbx_caladan3_sws_pt_shaper_get(int unit, int queue, uint32 *kbits_sec, uint32 *kbits_burst);

extern
int soc_sbx_caladan3_sws_ipte_init(int unit);

void
soc_sbx_caladan3_sws_set_icc_state(int unit, int instance, int state);

int
soc_sbx_caladan3_pr_set_icc_state(int unit, int pr, int flag);

int
soc_sbx_caladan3_sws_check_icc_state(int unit, int instance);

soc_sbx_caladan3_sws_pr_policer_cfg_t *
soc_sbx_caladan3_sws_pr_policer_cfg_get(int unit, int instance);

int 
soc_sbx_caladan3_sws_driver_init(int unit);

int 
soc_sbx_caladan3_sws_hotswap(int unit);

int 
soc_sbx_caladan3_sws_driver_param_uninit(int unit);

/*
 * Function
 *     soc_sbx_caladan3_sws_is_tdm_reusable
 * Purpose
 *     Determine if the last used TDM allows us to reuse paramteers
 *     Returns TRUE/FALSE
 */
extern int
soc_sbx_caladan3_sws_is_tdm_reusable(int unit);

/*
 * Function:
 *     soc_sbx_caladan3_sws_get_queue_status
 * Purpose
 *     Get queue Status
 *     *enabled = True if enabled/False if disabled
 *     returns one of SOC_E*
 */
int 
soc_sbx_caladan3_sws_get_queue_status(int unit, int qid, int *enabled); 

extern int
soc_sbx_caladan3_sws_source_queue_update(int unit);

/*
 * Function:
 *     soc_sbx_caladan3_sws_get_queue_status
 * Purpose
 *     Get the Port which is using this squeue
 */
int 
soc_sbx_caladan3_get_port_from_squeue(int unit, int squeue, int *port);


/*
 * Function:
 *     soc_sbx_caladan3_get_squeue_from_port
 * Purpose
 *     Get the Source queue given port,cos
 */
int 
soc_sbx_caladan3_get_squeue_from_port(int unit, int port, int direction, 
                                      int cos, int *squeue);
/*
 * Function:
 *     soc_sbx_caladan3_get_dqueue_from_port
 * Purpose
 *     Get the Destination queue given port,cos,dir
 */
int 
soc_sbx_caladan3_get_dqueue_from_port(int unit, int port, int direction, 
                                      int cos, int *dqueue);
/*
 * Function:
 *     soc_sbx_caladan3_get_queues_from_port
 * Purpose
 *     Get the SQ, DQ and Num of queues that is assigned to the given port
 */
int
soc_sbx_caladan3_get_queues_from_port(int unit, int port, int *sq, int*dq, int *num_cos);
int
soc_sbx_caladan3_get_queues_from_fabric_port_info(int unit, int fpidx, int *sq, int*dq, int *num_cos, soc_port_t *port);

/*
 * Ucode/Application based ICC setup 
 */
void
soc_sbx_caladan3_sws_pr_icc_program_sirius_header(int unit, int force, 
                                                  int remove_entry, 
                                                  int lport_to_remove,
                                                  int dont_program_resv_ports);
void
soc_sbx_caladan3_sws_pr_icc_program_arad_header(int unit, int force);
void
soc_sbx_caladan3_sws_pr_icc_program_loopback_header(int unit, int force);

/*
 * Flow control 
 */
int
soc_sbx_caladan3_sws_pt_instance_fc_init(int unit, int instance);

/*
 * CLI & Diag helpers
 */
extern void soc_sbx_caladan3_sws_config_dump(int unit);

extern int soc_sbx_caladan3_sws_cmic_port_hpp_loopback(int unit, 
                                                       int ingress,
                                                       int egress,
                                                       int free);

extern int soc_sbx_caladan3_sws_cmic_to_port_mac_loopback(int unit, 
                                                          int port,
                                                          int reset);

extern int soc_sbx_caladan3_sws_tdm_show_all(int unit);


/* 
 * Function: 
 *     soc_sbx_caladan3_sws_pr_icc_tcam_entry_get 
 * Purpose: 
 *     Fetch a TCam entry 
 */ 
extern void 
soc_sbx_caladan3_sws_pr_icc_tcam_entry_get( 
                                 int unit, int pr, int idx, int *valid, 
                                 uint8 *key, uint8 *mask,  
                                 uint8 *state, uint8 *state_mask,  
                                 soc_sbx_caladan3_pr_icc_lookup_data_t *data); 


/*
 * Function:
 *     soc_sbx_caladan3_sws_pr_icc_tcam_program
 * Purpose:
 *     Program a PR ICC tcam entry
 *     Effective Key = {key, state}
 *     Effective Mask = {mask, state_mask}
 *     Effective Result = {data}
 */
extern void
soc_sbx_caladan3_sws_pr_icc_tcam_program(
                                int unit, int pr, int idx, int valid,
                                uint8 *key, uint8 *mask, 
                                uint8 *state, uint8 *state_mask, 
                                soc_sbx_caladan3_pr_icc_lookup_data_t *data);
/*
 * Function:
 *     soc_sbx_caladan3_sws_redirect_queues_get
 * Purpose:
 *     Get ingress and Egress redirect Queues
 */

int
soc_sbx_caladan3_sws_redirect_queues_get(int unit,
                                         int *irq0, int *irq1,
                                         int *erq0, int *erq1);
/*
 * Function:
 *     soc_sbx_caladan3_sws_bubble_queues_get
 * Purpose:
 *     Get the Ingress and Egress Bubble queues
 */
int
soc_sbx_caladan3_sws_bubble_queues_get(int unit,
                                       int *ibq, int *ebq);

/*
 * Function:
 *     soc_sbx_caladan3_sws_app_queues_set
 * Purpose:
 *     Set base and number of queues reserved by application
 */
int
soc_sbx_caladan3_sws_app_queues_set(int unit, int start, int numq);

/*
 * Function:
 *     soc_sbx_caladan3_sws_app_queues_get
 * Purpose:
 *     Get base and number of queues reserved by application
 */
int
soc_sbx_caladan3_sws_app_queues_get(int unit, int *start, int *numq);


/**
 * Function:
 *     soc_sbx_caladan3_sws_qm_source_queue_get
 * Purpose:
 *     get the configure of a given source queue
 */
int
soc_sbx_caladan3_sws_qm_source_queue_get(int unit, int qid, sws_qm_source_queue_cfg_t* queue_cfg, int* en);



/**
 * Function:
 *     soc_sbx_caladan3_sws_qm_source_queue_set
 * Purpose:
 *     get the configure of a given source queue
 */
int
soc_sbx_caladan3_sws_qm_source_queue_set(int unit, int qid, sws_qm_source_queue_cfg_t* queue_cfg, int en);

/*
 * Function:
 *     soc_sbx_caladan3_sws_qm_source_queue_config
 * Purpose:
 *     configure a given source queue, programs the QM
 */
int
soc_sbx_caladan3_sws_qm_source_queue_config(int unit, int qid, sws_qm_source_queue_cfg_t* queue_cfg, int en);


/*
 *     soc_sbx_calandan3_sws_icc_state_key_unpack
 * Purpose:
 *     unpack state value to state info
 *      input: state;   output: state_ino
 */
int
soc_sbx_calandan3_sws_icc_state_key_unpack(soc_sbx_calandan3_icc_state_info_t* state_info, uint8* state);

/*
 * Function:
 *     soc_sbx_calandan3_sws_icc_state_key_pack
 * Purpose:
 *     pack state info to state value
 *      input: state_ino;   output: state
 */
int
soc_sbx_calandan3_sws_icc_state_key_pack(soc_sbx_calandan3_icc_state_info_t* state_info, uint8* state);


/*
 * Function:
 *     soc_sbx_caladan3_sws_hpte_map_setup
 * Purpose:
 *     configure the hpte map attaching a default dq to an sq
 */
int
soc_sbx_caladan3_sws_hpte_map_setup(int unit, uint32 squeue, uint32 dqueue);


/*
 * Function:
 *     soc_sbx_caladan3_sws_qm_src_queue_enable
 * Purpose:
 *     Enable/disable source queue
 */
int
soc_sbx_caladan3_sws_qm_src_queue_enable(int unit, int qid, int enable);
int
soc_sbx_caladan3_sws_qm_dest_queue_enable(int unit, int qid, int enable);

/*
 * Function:
 *     soc_sbx_caladan3_sws_pt_remap_enabled
 * Purpose:
 *     Returns True if PT remap is enabled
 *     This makes the B0 version PT to behave like A0 version
 */
int
soc_sbx_caladan3_sws_pt_remap_enabled(int unit);


int soc_sbx_caladan3_sws_tdm_is_oversubscribed(int unit, int tdm_id);


#endif /* _SBX_CALADN3_SWS_H_ */


