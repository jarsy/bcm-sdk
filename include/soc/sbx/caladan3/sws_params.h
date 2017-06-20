/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: sws_params.h,v 1.3.18.4 Broadcom SDK $
 * 
 */

#ifdef BCM_CALADAN3_SUPPORT

#ifndef _CALADAN3_SWS_CFG_H
#define _CALADAN3_SWS_CFG_H

#define SOC_SBX_CALADAN3_SWS_CLOCK_RATE              (641)  /* Mhz */

#define SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION        (64)
#define SOC_SBX_CALADAN3_SWS_PT_MAX_PORT_CALENDAR    (256)
#define SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR  (256)
#define SOC_SBX_CALADAN3_SWS_PT_MAX_PORTS            (51)
#define SOC_SBX_CALADAN3_SWS_PT_PORTX                (-1)
#define SOC_SBX_CALADAN3_SWS_MAX_TDM                 (100)
#define PORTX                                        (SOC_SBX_CALADAN3_SWS_PT_PORTX)

/*
 * Profile Based PR buffer allocation
 * - This feature allows user to pick a profile for pr buffer level allocation
 *   this is used in conjunction with Hotswap/Fastreset to setup
 *   universal pr buffer scheme. The profile id selects the scheme to apply
 */
#define SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_MAX         (2)

#define SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_ID_CL_XT    (1)
#define SOC_SBX_CALADAN3_PR_BUFFER_PROFILE_ID_CL_ONLY  (2)


typedef enum {
    CLIENT0,
    CLIENT1,
    CLIENT2,
    CLIENT3,
    CLIENT4,
    CLIENT5,
    CLIENTX    /* Used for an unused entry with active=0 */
} client_id_t;


/* QM config paramters */
typedef struct qm_buffer_cfg_s {
    unsigned int total_buff_max_pages;
    unsigned int total_buff_drop_thres_de1;
    unsigned int total_buff_drop_thres_de2;
    unsigned int fc_total_buffer_xoff_thresh;
    unsigned int num_pages_reserved;
    unsigned int total_buff_hysteresis_delta;
    unsigned int ingress_max_pages;
    unsigned int ingress_drop_thres_de1;
    unsigned int ingress_drop_thres_de2;
    unsigned int fc_ingress_xoff_thresh;
    unsigned int ingress_hysteresis_delta;
    unsigned int egress_max_pages;
    unsigned int egress_drop_thres_de1;
    unsigned int egress_drop_thres_de2;
    unsigned int fc_egress_xoff_thresh;
    unsigned int egress_hysteresis_delta;
    unsigned int per_queue_drop_hysteresis_delta;
} sws_qm_buffer_cfg_t;

typedef struct _sws_qm_flow_ctrl_cfg_s_ {
    unsigned int fc_total_buffer_xoff_thresh;
    unsigned int fc_ingress_xoff_thresh;
    unsigned int fc_egress_xoff_thresh;
} sws_qm_flow_ctrl_cfg_t;


typedef struct qm_source_queue_cfg_s {
    unsigned int max_pages;
    unsigned int drop_thres_de1;
    unsigned int drop_thres_de2;
    unsigned int flow_ctrl_thresh;
    unsigned int min_pages_data;
    unsigned int min_pages_header;
} sws_qm_source_queue_cfg_t;

typedef struct pr_config_s {
    unsigned int de0_threshold;
    unsigned int de1_threshold;
    unsigned int de2_threshold;
    unsigned int de3_threshold;
    unsigned int nfull_threshold;
} sws_pr_idp_thresholds_config_t;

typedef struct pt_client_cal_s {
    int num_elements;
    client_id_t client_id[SOC_SBX_CALADAN3_SWS_PT_MAX_CLIENT_CALENDAR];
} sws_pt_client_cal_t;

typedef struct pt_port_cal_s {
    int num_elements;
    int port_id[SOC_SBX_CALADAN3_SWS_PT_MAX_PORT_CALENDAR];
} sws_pt_port_cal_t;

typedef struct pt_port_fifo_entry_s {
    unsigned int port;
    unsigned int base;
    unsigned int size;
    unsigned int page_threshold;
} pt_port_fifo_entry_t;

typedef struct pt_port_fifo_s {
    int num_elements;  /* Set to 0 for auto allocation */
    pt_port_fifo_entry_t entry[SOC_SBX_CALADAN3_SWS_PT_MAX_PORTS];
} sws_pt_port_fifo_t;

typedef struct pr_port_buffer_entry_s {
    unsigned int port;
    unsigned int start;
    unsigned int size;
} pr_port_buffer_entry_t;

typedef struct pr_port_buffer_s {
    unsigned int num_elements;  /* Set to 0 for auto allocation */
    pr_port_buffer_entry_t entry[SOC_SBX_CALADAN3_SWS_PT_MAX_PORTS];
} sws_pr_port_buffer_t;

typedef struct _sws_qm_config_s {

    /* buffer config */
    sws_qm_buffer_cfg_t buffer;

    /* queue config */
    unsigned int line_queues_used;
    unsigned int fabric_queues_used;
    sws_qm_source_queue_cfg_t line_queues[SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION];
    sws_qm_source_queue_cfg_t fabric_queues[SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION];

    /* PCIe queue config */
    sws_qm_source_queue_cfg_t cmic_queue;

    /* SGMII queue config */
    sws_qm_source_queue_cfg_t xlport_queue[2];

    /* redirection queue config */
    sws_qm_source_queue_cfg_t ing_rt_queue[2];
    sws_qm_source_queue_cfg_t egr_rt_queue[2];

    /* bubble queue config */
    sws_qm_source_queue_cfg_t ing_bubble_queue;
    sws_qm_source_queue_cfg_t egr_bubble_queue;


} sws_qm_config_t;

typedef struct pt_cfg_s {
    sws_pt_port_fifo_t  pt_fifo;
    sws_pt_client_cal_t pt_client_cal;
    sws_pt_port_cal_t   pt_port_cal;
} sws_pt_config_t;

typedef struct pr_cfg_s {
    sws_pr_port_buffer_t rx_buffer_cfg;
    sws_pr_idp_thresholds_config_t pr_idp_cfg;
} sws_pr_config_t;

typedef struct soc_sbx_caladan3_sws_pt_s {
    sws_pt_port_fifo_t  *pt_fifo;
    sws_pt_client_cal_t *pt_client_cal;
    sws_pt_port_cal_t   *pt_port_cal;
} soc_sbx_caladan3_sws_pt_cfg_t;

typedef struct soc_sbx_caladan3_sws_pr_s {
    int                              pr_buf_cfg_valid;
    sws_pr_port_buffer_t            *pr_buf_cfg;
    sws_pr_idp_thresholds_config_t  *pr_idp_cfg;
} soc_sbx_caladan3_sws_pr_cfg_t;


/*
 * SWS Configuration
 *   sws_config_t struct represents all the configurable parameters in SWS
 * Each supported TDM will have a separate stuct that carries it params
 * Each supported TDM will be identified by a name and id as given below
 */

/* TDM identification */
typedef enum tdmids {
    TDMNONE = -1,
    TDMNULL = 0,
    TDM1,
    TDM2,
    TDM3,
    TDM4,
    TDM5,
    TDM6,
    TDM7,
    TDM8,
    TDM9,
    TDM10,
    TDM11,
    TDM12,
    TDM13,
    TDM14,
    TDM15,
    TDM16,
    TDM17,
    TDM18,
    TDM19,
    TDM20,
    TDM21,
    TDM22,
    TDM23,
    TDM24,
    TDM25,
    TDM26,
    TDM27,
    TDM28,
    TDM29,
    TDM30,
    TDM31,
    TDM32,
    TDM33,
    TDM34,
    TDM35,
    TDM36,
    TDM37,
    TDM38,
    TDM39,
    TDM40,
    TDM41,
    TDM42,
    TDM43,
    TDM44,
    TDMLAST
} tdmid_t;

typedef struct {
    tdmid_t id;
    char *name;
} tdm_identifier_t;

/* Specify queue and PT params at the interface level */
typedef struct {
    sws_qm_source_queue_cfg_t lqcfg; 
    sws_qm_source_queue_cfg_t fqcfg; 
    pt_port_fifo_entry_t      pt_fifo_entry;
} sws_intf_cfg_t;

typedef struct {
    /* This is actually maxed at SOC_SBX_CALADAN3_MAX_PORT_INTF_TYPE 
     * which is an enumeration and not visible without including port.h
     * not including it here just for the sake of one declaration
     */
    sws_intf_cfg_t intf[SOC_SBX_CALADAN3_SWS_MAX_TDM];
} sws_intf_config_t;


typedef struct {
    uint32 tdm_is_oversubscribed;
} sws_global_config_t;


typedef struct sws_config_s {

    /* qm config */
    sws_qm_config_t qm_cfg;

    /* Line PT config */
    sws_pt_config_t line_pt_cfg;

    /* Fabric PT config */
    sws_pt_config_t fabric_pt_cfg;

    /* Line PR config */
    sws_pr_config_t line_pr_cfg;

    /* Fabric PR config */
    sws_pr_config_t fabric_pr_cfg;

    /* Interface config */
    sws_intf_config_t intf_cfg;

    sws_global_config_t global_cfg;

} sws_config_t;


/* Prototypes */

/*
 * Function: soc_sbx_caladan3_tdm_identifier_get
 * Purpose:  Get the TDM identifier given an id
 * Returns:  tdm identifier
 */
extern tdm_identifier_t *
soc_sbx_caladan3_tdm_identifier_get(int unit, int tdmid);

/*
 * Function: soc_sbx_caladan3_sws_tdm_register
 * Purpose:  given an sws config, name and TDM Id, register the TDM
 * Returns:  sws_cfg associated with this TDM
 */
extern int
soc_sbx_caladan3_sws_tdm_register(int unit, int id, char *name, sws_config_t *sws_cfg);

/*
 * Function: soc_sbx_caladan3_sws_tdm_unregister
 * Purpose:  given a name or an Id, unregister the TDM
 * Returns:  sws_cfg associated with this TDM
 */
extern int
soc_sbx_caladan3_sws_tdm_unregister(int unit, int id, char *name, sws_config_t **sws_cfg);






#endif /* CALADAN3_SWS_CFG_H */

#endif /* BCM_CALADAN3_SUPPORT */
