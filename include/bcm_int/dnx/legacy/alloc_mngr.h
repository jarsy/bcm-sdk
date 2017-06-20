/*
 * $Id: alloc_mngr.h,v 1.119 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        alloc_mngr.h
 * Purpose:     Resource allocation manager for DNX_SAND chips.
 *
 */

#ifndef  INCLUDE_DNX_ALLOC_MNGR_H
#define  INCLUDE_DNX_ALLOC_MNGR_H

#include <soc/dnx/legacy/TMC/tmc_api_end2end_scheduler.h>
#include <soc/dnx/legacy/TMC/tmc_api_tdm.h>
#include <soc/dnx/legacy/drv.h>
#include <bcm_int/dnx/legacy/alloc_mngr_shr.h>
#include <bcm_int/dnx/legacy/cosq.h>
#include <bcm_int/dnx/legacy/utils.h>
#include <bcm/rx.h>
#include <shared/dnx_shr_template.h>
#include <shared/swstate/sw_state_resmgr.h>


#define BCM_DNX_AM_TEMPLATE_FLAG_CHECK      (1)


/* Egress thresh */
#define _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_COUNT(unit) (16)

/* CRPS compensation profiles size */
#define _DNX_CRPS_COMPENSATION_PROFILES_SIZE (16)
#define _DNX_STAT_INTERFACE_COMPENSATION_PROFILES_SIZE (16)

/* Scheduler adjust size (compensation) final delta mapping  */
#define _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_PROFILE_LOW_ID       (0)
#define _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_PROFILE_COUNT        (32) /* support up to 32 different compensation values */
#define _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_MAX_ENTITIES         (256)
#define _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_CLS_SIZE             (sizeof(int))


typedef int (*dnx_am_cosq_flow_region_set_f)(int, int, int, int, int, int);
typedef int (*dnx_am_cosq_queue_region_set_f)(int, int, int, int, int);

typedef struct dnx_am_cosq_resource_info_s {
    dnx_am_cosq_flow_region_set_f      dnx_am_cosq_flow_region_set;
    dnx_am_cosq_queue_region_set_f     dnx_am_cosq_queue_region_set;
} dnx_am_cosq_resource_info_t;


typedef struct {
    uint32  cir;
    uint32  eir;
    uint32  max_cir;
    uint32  max_eir;    
    uint32  cbs;
    uint32  ebs;
    uint32  flags;
    uint32  mode;
    uint32  entropy_id;
    uint32  ir_rev_exp; /* relevant only for MEF mode (cascade) */
} bcm_dnx_am_meter_entry_t;


typedef struct _dnx_res_pool_cosq_s {
    int dnx_res_pool_cosq_connector_cont;
    int dnx_res_pool_cosq_connector_non_cont_region_type_1;
    int dnx_res_pool_cosq_connector_non_cont_region_type_2;
    int dnx_res_pool_cosq_se_cl_fq_region_type_1;
    int dnx_res_pool_cosq_se_cl_fq_region_type_2;
    int dnx_res_pool_cosq_se_cl_hr;
    int dnx_res_pool_cosq_fq_connector_region_type_2_sync;
    int dnx_res_pool_cosq_hr_connector_region_type_2_sync;
    int dnx_res_pool_cosq_e2e_end;
    int dnx_res_pool_cosq_queue_unicast_cont;
    int dnx_res_pool_cosq_queue_unicast_non_cont;
    int dnx_res_pool_cosq_queue_multicast_cont;
    int dnx_res_pool_cosq_queue_multicast_non_cont;
    int dnx_res_pool_cosq_queue_isq_cont;
    int dnx_res_pool_cosq_queue_isq_non_cont;
    int dnx_res_pool_cosq_queue_dynamic_start;
    int dnx_res_pool_cosq_queue_dynamic_end;
} _dnx_res_pool_cosq_t;


typedef struct bcm_dnx_am_cosq_quad_allocation_s {
    int flow1;
    int flow2;
    int flow3;
    int flow4;
}bcm_dnx_am_cosq_quad_allocation_t;

typedef struct _dnx_res_type_cosq_s {
    int dnx_res_type_cosq_connector_cont;
    int dnx_res_type_cosq_connector_non_cont_region_type_1;
    int dnx_res_type_cosq_connector_non_cont_region_type_2;
    int dnx_res_type_cosq_se_cl_fq_region_type_1;
    int dnx_res_type_cosq_se_cl_fq_region_type_2;
    int dnx_res_type_cosq_se_cl_hr;
    int dnx_res_type_cosq_fq_connector_region_type_2_sync;
    int dnx_res_type_cosq_hr_connector_region_type_2_sync;
    int dnx_res_type_cosq_e2e_end;
    int dnx_res_type_cosq_queue_unicast_cont;
    int dnx_res_type_cosq_queue_unicast_non_cont;
    int dnx_res_type_cosq_queue_multicast_cont;
    int dnx_res_type_cosq_queue_multicast_non_cont;
    int dnx_res_type_cosq_queue_isq_cont;
    int dnx_res_type_cosq_queue_isq_non_cont;
    int dnx_res_type_cosq_queue_dynamic_start;
    int dnx_res_type_cosq_queue_dynamic_end;
} _dnx_res_type_cosq_t;


typedef struct {
    uint8 fec_bank_groups[SOC_DNX_DEFS_MAX(NOF_FEC_BANKS)];
    PARSER_HINT_ARR int *resource_to_pool_map;
    PARSER_HINT_ARR int *template_to_pool_map;
    int egr_thresh_map[SOC_DNX_DEFS_MAX(NOF_LOGICAL_PORTS)];
    int egr_interface_unicast_thresh_map[SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES)*SOC_DNX_DEFS_MAX(NOF_CORES)];
    int egr_interface_multicast_thresh_map[SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES)*SOC_DNX_DEFS_MAX(NOF_CORES)];
} bcm_dnx_alloc_mngr_info_t;

int
bcm_dnx_am_attach(
    int unit);

int
bcm_dnx_am_clear(
    int unit);

int
bcm_dnx_am_detach(
    int unit);


int
bcm_dnx_am_mc_dealloc(
        int unit,
        DNX_TMC_MULT_ID mc_id,
        uint8 is_egress);

int
bcm_dnx_am_mc_is_alloced(
        int unit,
        DNX_TMC_MULT_ID mc_id,
        uint8 is_egress);



int
bcm_dnx_am_voq_connector_alloc(
        int unit,
        uint32 flags,
        DNX_TMC_SCH_FLOW_ID *flow_id);

int
bcm_dnx_am_voq_connector_dealloc(
        int unit,
        uint32 flags,
        DNX_TMC_SCH_FLOW_ID flow_id);

int
bcm_dnx_am_queue_alloc(
        int unit,
        uint32 flags,
        uint32 *queue_id);

int
bcm_dnx_am_queue_dealloc(
        int unit,
        uint32 flags,
        uint32 queue_id);
int
bcm_dnx_am_vsq_src_port_alloc(
    int unit,
    int core_id,
    uint32 flags,
    int src_pp_port,
    int *vsq_index);
int
bcm_dnx_am_vsq_src_port_free(
    int unit,
    int core_id,
    int vsq_index);
int
bcm_dnx_am_pg_init(
   int unit
   );
int
bcm_dnx_am_pg_alloc(
    int unit,
    int core_id,
    uint32 flags,
    int numq,
    int *pg_base);
int
bcm_dnx_am_pg_check(
    int unit, 
    int core_id, 
    int pg_base);
int
bcm_dnx_am_pg_get(
    int unit,
    int core_id,
    int pg_base,
    int *numq
    );
int
bcm_dnx_am_pg_free(
    int unit,
    int core_id,
    int pg_base);
/* Cosq Egress Thresh - Start */
  /* 
   * Port level drop & fc thresholds type
   * pointed by tm-port,
   * used by bcm_petra_cosq_threshold_set
   */
int _bcm_dnx_am_template_cosq_egr_thresh_init(int unit,
                                              int core,
                                              int template_init_id,
                                              bcm_dnx_cosq_egress_thresh_key_info_t *egr_thresh_data_init);
int _bcm_dnx_am_template_cosq_egr_thresh_data_get(int unit,
                                                  int core,
                                                  int port,
                                                  bcm_dnx_cosq_egress_thresh_key_info_t * data);
int _bcm_dnx_am_template_cosq_egr_thresh_exchange(int unit, 
                                                 int core,
                                                 int port, 
                                                 bcm_dnx_cosq_egress_thresh_key_info_t * data, 
                                                 int *old_template,
                                                 int *is_last, 
                                                 int *template,
                                                 int *is_allocated);
int _bcm_dnx_am_template_cosq_egr_thresh_ref_get(int unit, int template, uint32 *ref_count);

int _bcm_dnx_am_template_cosq_egr_thresh_allocate_group(int unit, uint32 flags, bcm_dnx_cosq_egress_thresh_key_info_t *data, int ref_count, int *is_allocated, int *template);

int _bcm_dnx_am_template_cosq_egr_thresh_tdata_get(int unit, int template, bcm_dnx_cosq_egress_thresh_key_info_t *data);

/* Cosq Egress Thresh - End */

/* Cosq Egress Interface Unicast Thresh - Start */
  /* 
        * Port level drop & fc thresholds type ?
        * pointed by tm-port, ?
        * used by bcm_petra_cosq_threshold_set ?
   */
int _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_init(int unit,
                                                                int core,
                                              int template_init_id,
                                              bcm_dnx_cosq_egress_interface_unicast_thresh_key_info_t *egr_interface_unicast_thresh_data_init);
int _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_data_get(int unit,
                                                                    int core,
                                                  int interface_id,
                                                  bcm_dnx_cosq_egress_interface_unicast_thresh_key_info_t * data);
int _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_exchange(int unit, 
                                                                    int core, 
                                                 int interface_id, 
                                                 bcm_dnx_cosq_egress_interface_unicast_thresh_key_info_t * data, 
                                                 int *old_template,
                                                 int *is_last, 
                                                 int *template,
                                                 int *is_allocated);
/* Cosq Egress Interface Unicast Thresh - End */

/* Cosq Egress Interface Multicast Thresh - Start */
  /* 
        * Port level drop & fc thresholds type ?
        * pointed by tm-port, ?
        * used by bcm_petra_cosq_threshold_set ?
   */
int _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_init(int unit,
                                                                  int core,
                                              int template_init_id,
                                              bcm_dnx_cosq_egress_interface_multicast_thresh_key_info_t *egr_interface_multicast_thresh_data_init);
int _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_data_get(int unit,
                                                                      int core,
                                                  int interface_id,
                                                  bcm_dnx_cosq_egress_interface_multicast_thresh_key_info_t * data);
int _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_exchange(int unit, 
                                                                      int core,
                                                 int interface_id, 
                                                 bcm_dnx_cosq_egress_interface_multicast_thresh_key_info_t * data, 
                                                 int *old_template,
                                                 int *is_last, 
                                                 int *template,
                                                 int *is_allocated);
/* Cosq Egress Interface Multicast Thresh - End */
/* COSQ Egress queue mapping - Start */
int _bcm_dnx_am_template_egress_queue_mapping_init(int unit,
                                                   int template_init_id,
                                                   int core,
                                                   const bcm_dnx_cosq_egress_queue_mapping_info_t* mapping_profile);

int _bcm_dnx_am_template_egress_queue_mapping_data_get(int unit,
                                                       uint32 tm_port,
                                                       int core,
                                                       bcm_dnx_cosq_egress_queue_mapping_info_t * mapping_profile);

int _bcm_dnx_am_template_egress_queue_mapping_exchange(int unit, 
                                                       uint32 tm_port, 
                                                       int core,
                                                       CONST bcm_dnx_cosq_egress_queue_mapping_info_t * mapping_profile, 
                                                       int *old_profile, 
                                                       int *is_last, 
                                                       int *new_profile,
                                                       int *is_allocated);

/* COSQ Egress queue mapping - End */

/* NRDY threshold - Start */
int _bcm_dnx_am_template_nrdy_threshold_alloc(int unit, int core, int flags, uint32 *data, int *is_allocated, int *profile);

int _bcm_dnx_am_template_nrdy_threshold_free(int unit, int core, int profile, int *is_last);

int _bcm_dnx_am_template_nrdy_threshold_exchange(int unit, int core, uint32 tm_port, int flags, uint32 *data,
                                             int *old_profile, int *is_last, int *profile, int *is_allocated);

/* NRDY threshold - End */

/* Ingress queue rate class - Start */
int _bcm_dnx_am_template_queue_rate_cls_init(int unit,int template_init_id,const bcm_dnx_cosq_ingress_rate_class_info_t* data_rate_cls);
int _bcm_dnx_am_template_queue_rate_cls_data_get(int unit, int core, int queue,bcm_dnx_cosq_ingress_rate_class_info_t * data_rate_cls);
int _bcm_dnx_am_template_queue_rate_cls_exchange(int unit, int core, int queue, bcm_dnx_cosq_ingress_rate_class_info_t * data_rate_cls, int *old_rate_cls, int *is_last, int *new_rate_cls,int *is_allocated);
int _bcm_dnx_am_template_queue_rate_cls_exchange_test(int unit, int core, int queue, bcm_dnx_cosq_ingress_rate_class_info_t * data_rate_cls, int *old_rate_cls, int *is_last, int *new_rate_cls,int *is_allocated);
int _bcm_dnx_am_template_queue_rate_cls_ref_get(int unit, int template, uint32 *ref_count);
int _bcm_dnx_am_template_queue_rate_cls_allocate_group(int unit, uint32 flags, bcm_dnx_cosq_ingress_rate_class_info_t *data_rate_cls, int ref_count, int *is_allocated, int *template);
int _bcm_dnx_am_template_queue_rate_cls_tdata_get(int unit, int template, bcm_dnx_cosq_ingress_rate_class_info_t *data_rate_cls);

/* Ingress queue rate class - End */


/* VSQ PG TC mapping profile - Start */
int _bcm_dnx_am_template_vsq_pg_tc_mapping_profile_init(int unit, int core_id, uint32 pg_tc_bitmap);
int _bcm_dnx_am_template_vsq_pg_tc_mapping_profile_data_get(int unit, int core_id, int pg_tc_profile, uint32 *pg_tc_bitmap);
int _bcm_dnx_am_template_vsq_pg_tc_mapping_exchange(int unit, int core_id, uint32 pg_tc_bitmap, int old_pg_tc_profile, int *is_last, int *new_pg_tc_profile,int *is_allocated);
/* VSQ PG TC mapping profile - End*/
/* VSQ queue rate class - Start */

int _bcm_dnx_am_template_vsq_rate_cls_init(int unit,int template_init_id,const bcm_dnx_cosq_vsq_rate_class_info_t* data_rate_cls);
int _bcm_dnx_am_template_vsq_rate_cls_data_get(int unit, int core_id, int vsq_id, bcm_dnx_cosq_vsq_rate_class_info_t * data_rate_cls);
int _bcm_dnx_am_template_vsq_rate_cls_exchange(int unit, int core_id, int vsq_id, bcm_dnx_cosq_vsq_rate_class_info_t * data_rate_cls, int *old_rate_cls, int *is_last, int *new_rate_cls,int *is_allocated);
int _bcm_dnx_am_template_vsq_rate_cls_ref_get(int unit, int template, DNX_TMC_ITM_VSQ_GROUP vsq_group, uint32 *ref_count);
int _bcm_dnx_am_template_vsq_rate_cls_allocate_group(int unit, uint32 flags, DNX_TMC_ITM_VSQ_GROUP vsq_group, bcm_dnx_cosq_vsq_rate_class_info_t *data_rate_cls, int ref_count, int *is_allocated, int *template);
int _bcm_dnx_am_template_vsq_rate_cls_tdata_get(int unit, int template, DNX_TMC_ITM_VSQ_GROUP vsq_group, bcm_dnx_cosq_vsq_rate_class_info_t *data_rate_cls);
/* VSQ queue rate class - End */

/* System RED drop probability value class - Start */

int _bcm_dnx_am_template_system_red_dp_pr_init(int unit,int template_init_id,const bcm_dnx_cosq_ingress_system_red_dp_pr_info_t* data_sys_red_dp_pr);
int _bcm_dnx_am_template_system_red_dp_pr_data_get(int unit,int dp_pr,bcm_dnx_cosq_ingress_system_red_dp_pr_info_t * data_sys_red_dp_pr);
int _bcm_dnx_am_template_system_red_dp_pr_exchange(int unit, int dp_pr, bcm_dnx_cosq_ingress_system_red_dp_pr_info_t *data_sys_red_dp_pr, int *old_sys_red_dp_pr, int *is_last, int *new_sys_red_dp_pr,int *is_allocated);
int _bcm_dnx_am_template_system_red_dp_pr_ref_get(int unit, int template, uint32 *ref_count);
int _bcm_dnx_am_template_system_red_dp_pr_allocate_group(int unit, uint32 flags, bcm_dnx_cosq_ingress_system_red_dp_pr_info_t *data_sys_red_dp_pr, int ref_count, int *is_allocated, int *template);
int _bcm_dnx_am_template_system_red_dp_pr_tdata_get(int unit, int template, bcm_dnx_cosq_ingress_system_red_dp_pr_info_t *data_sys_red_dp_pr);
/* Ingress queue discount class - Start */

int _bcm_dnx_am_template_queue_discount_cls_init(int unit, int template_init_id,const DNX_TMC_ITM_CR_DISCOUNT_INFO* data_discount_cls);
int _bcm_dnx_am_template_queue_discount_cls_mapping_get(int unit, int core, int queue, int *discount_cls);
int _bcm_dnx_am_template_queue_discount_cls_data_get(int unit, int core, int queue,DNX_TMC_ITM_CR_DISCOUNT_INFO * data_discount_cls);
int _bcm_dnx_am_template_queue_discount_cls_exchange(int unit, int core, int queue, DNX_TMC_ITM_CR_DISCOUNT_INFO * data_discount_cls, int *old_discount_cls, int *is_last, int *new_discount_cls,int *is_allocated);
int _bcm_dnx_am_template_queue_discount_cls_ref_get(int unit, int core, int template, uint32 *ref_count);
int _bcm_dnx_am_template_queue_discount_cls_allocate_group(int unit, int core, uint32 flags, DNX_TMC_ITM_CR_DISCOUNT_INFO *data_discount_cls, int ref_count, int *is_allocated, int *template);
int _bcm_dnx_am_template_queue_discount_cls_tdata_get(int unit, int core, int template, DNX_TMC_ITM_CR_DISCOUNT_INFO *data_discount_cls);

/* Ingress queue discount class - End */

/* Egress port discount class - Start */
int _bcm_dnx_am_template_egress_port_discount_cls_free(int unit, int core, uint32 tm_port, int* is_last);
int _bcm_dnx_am_template_egress_port_discount_cls_init(int unit, int core, uint32 tm_port, const DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO* data_discount_cls);
int _bcm_dnx_am_template_egress_port_discount_cls_data_get(int unit,int core, uint32 tm_port, DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO * data_discount_cls);
int _bcm_dnx_am_template_egress_port_discount_cls_exchange(int unit, int core, uint32 tm_port, DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO * data_discount_cls, int *old_discount_cls, int *is_last, int *new_discount_cls,int *is_allocated);
/* int _bcm_dnx_am_template_egress_port_discount_cls_ref_get(int unit, int template, uint32 *ref_count); */
int _bcm_dnx_am_template_egress_port_discount_cls_per_header_type_ref_get(int unit, int core, int template , DNX_TMC_PORT_HEADER_TYPE header_type ,uint32 *ref_count);
/* int _bcm_dnx_am_template_egress_port_discount_cls_allocate_group(int unit, uint32 flags, DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *data_discount_cls, int ref_count, int *is_allocated, int *template); */
/* int _bcm_dnx_am_template_egress_port_discount_cls_tdata_get(int unit, int template, DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *data_discount_cls); */
int _bcm_dnx_am_template_egress_port_discount_cls_per_header_type_tdata_get(int unit, int core, int template, DNX_TMC_PORT_HEADER_TYPE header_type, DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *data_discount_cls);

/* Egress port discount class - End */

/* COSQ Port hr flow control profile - Start */

int _bcm_dnx_am_template_cosq_port_hr_fc_profile_mapping_set(int unit, int core, int port, int profile);
int _bcm_dnx_am_template_cosq_port_hr_fc_init(int unit, int core, int template_init_id, int *fc_data_init);

int _bcm_dnx_am_template_cosq_port_hr_fc_data_get(int unit, int core, int port, int *data);

int _bcm_dnx_am_template_cosq_port_hr_fc_exchange(int unit, int core, int port, int *data, int *old_template, int *is_last, int *template, int *is_allocated);

int _bcm_dnx_am_template_cosq_hr_fc_ref_get(int unit, int core, int template, uint32 *ref_count);

int _bcm_dnx_am_template_cosq_hr_fc_allocate_group(int unit, int core, uint32 flags, int *data, int ref_count, int *is_allocated, int *template);

/* COSQ Port Hr flow control profile - End */


/* COSQ CL scheduler profile - Start */

int _bcm_dnx_am_template_cosq_sched_class_init(int unit);

int _bcm_dnx_am_template_cosq_sched_class_allocate(int unit, int core, int flags, int *data, int *is_allocated, int *class_template);

int _bcm_dnx_am_template_cosq_sched_class_free(int unit, int core, int class_template, int *is_last);

int _bcm_dnx_am_template_cosq_sched_class_data_get(int unit, int core, int class_template, int *data);

int _bcm_dnx_am_template_cosq_sched_class_exchange(int unit, int core, int flags,int *data, int old_class_template, int *is_last, int *class_template, int *is_allocated);

int _bcm_dnx_am_template_cosq_sched_class_ref_get(int unit, int core, int template, uint32 *ref_count);

int _bcm_dnx_am_template_cosq_sched_class_allocate_group(int unit, int core, uint32 flags, int *data, int ref_count, int *is_allocated, int *template);

/* COSQ CL scheduler profile - End */



/* Fabric TDM direct routing - Start */
int _bcm_dnx_am_template_fabric_tdm_link_ptr_init(int unit,int template_init_id,const DNX_TMC_TDM_DIRECT_ROUTING_INFO* routing_info);
int _bcm_dnx_am_template_fabric_tdm_link_ptr_data_get(int unit,int port,DNX_TMC_TDM_DIRECT_ROUTING_INFO* routing_info);
int _bcm_dnx_am_template_fabric_tdm_link_ptr_exchange(int unit, int port, DNX_TMC_TDM_DIRECT_ROUTING_INFO * routing_info, int *old_link_ptr, int *is_last, int *new_link_ptr,int *is_allocated);
/* Fabric TDM direct routing - End */



/* credit adjust size scheduler final delta, relevant only for Jericho */
int _bcm_dnx_am_template_scheduler_adjust_size_final_delta_mapping_init(int unit, int template_init_id, int *final_delta);
int _bcm_dnx_am_template_scheduler_adjust_size_final_delta_mapping_data_get(int unit, int core, int delta , int *final_delta);
int _bcm_dnx_am_template_scheduler_adjust_size_final_delta_mapping_exchange(int unit, int core, int delta, int *final_delta, int *old_profile, int *is_last, int *new_profile,int *is_allocated);
/* Resource manager flags */

#define BCM_DNX_AM_FLAG_ALLOC_WITH_ID               (SW_STATE_RES_ALLOC_WITH_ID) /*0x00000001*/
/* if exist then reuse, can be set only if with id is present */
#define BCM_DNX_AM_FLAG_ALLOC_REPLACE               (SW_STATE_RES_ALLOC_REPLACE) /*0x00000004*/
#define BCM_DNX_AM_FLAG_ALLOC_EG_DECOUPLED_EVEN     (0x00000004)
#define BCM_DNX_AM_FLAG_ALLOC_INGRESS               (0x00000002)
#define BCM_DNX_AM_FLAG_ALLOC_EGRESS                (0x00000004)

#define BCM_DNX_AM_FLAG_ALLOC_RESERVE               (0x00010000)
#define SOC_DNX_OAM_LOCAL_PORT_2_SYSTEM_PORT_RESERVED_VALUE (-1)

/* relevant for IP tunnel allocation if present then allocate tunnel for IPv6 otherwise allocate for IPv4 */
#define BCM_DNX_AM_FLAG_ALLOC_IPV6                  (0x00020000)



/* Template manager falgs */
#define BCM_DNX_AM_TEMPLATE_FLAG_ALLOC_WITH_ID      (DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID) /*0x00000001*/



/* TM */
int _bcm_dnx_am_template_ingress_uc_tc_mapping_data_get(int unit,int core_id, int dest_ndx,DNX_TMC_ITM_TC_MAPPING *tc_mapping);
int _bcm_dnx_am_template_ingress_uc_tc_mapping_init(int unit,int template_init_id,const DNX_TMC_ITM_TC_MAPPING* tc_mapping);
int _bcm_dnx_am_template_ingress_uc_tc_mapping_exchange(int unit, int core_id, int dest_ndx, DNX_TMC_ITM_TC_MAPPING *tc_mapping, uint32 *old_mapping_profile, int *is_last, int *new_mapping_profile,int *is_allocated);
int _bcm_dnx_am_template_ingress_flow_tc_mapping_data_get(int unit, int core_id, int dest_ndx,DNX_TMC_ITM_TC_MAPPING *tc_mapping);
int _bcm_dnx_am_template_ingress_flow_tc_mapping_init(int unit, int template_init_id, const DNX_TMC_ITM_TC_MAPPING* tc_mapping);
int _bcm_dnx_am_template_ingress_flow_tc_mapping_exchange(int unit, int core_id, int dest_ndx, DNX_TMC_ITM_TC_MAPPING *tc_mapping, uint32 *old_mapping_profile, int *is_last, int *new_mapping_profile,int *is_allocated);

int _bcm_dnx_am_template_fc_generic_pfc_mapping_init(int unit, int priority, int template_id, DNX_TMC_FC_PFC_GENERIC_BITMAP *generic_bm);
int _bcm_dnx_am_template_fc_generic_pfc_mapping_data_get(int unit, int priority, int generic_bm_id, DNX_TMC_FC_PFC_GENERIC_BITMAP *generic_bm);
int _bcm_dnx_am_template_fc_generic_pfc_mapping_exchange(int unit, int priority, int old_generic_bm_id, DNX_TMC_FC_PFC_GENERIC_BITMAP *new_generic_bm, int *is_last, int *new_mapping_profile,int *is_allocated);

int _bcm_dnx_am_template_ttl_scope_init(int unit, int template_init_id, uint16 *ttl);
int _bcm_dnx_am_template_ttl_scope_exchange(int unit, int ttl_scope_index, uint16 *ttl, int *is_last, int *new_ttl_scope_index, int *is_allocated);
int _bcm_dnx_am_template_ttl_scope_index_ttl_mapping_get(int unit, int ttl_scope_index, uint16 *ttl);

int _bcm_dnx_am_template_crps_pktSize_compensation_init(int unit, int core ,_dnx_am_template_t template_id, int delta, int template);
int _bcm_dnx_am_template_crps_pktSize_compensation_exchange(int unit, int core, _dnx_am_template_t template_id, int port, int data, int old_template, int *is_last, int *template,int *is_allocated);

int _bcm_dnx_am_template_stat_interface_pktSize_compensation_init(int unit, int core ,_dnx_am_template_t template_id, int delta, int template);
int _bcm_dnx_am_template_stat_interface_pktSize_compensation_exchange(int unit, int core, _dnx_am_template_t template_id, int port, int data, int old_template, int *is_last, int *template,int *is_allocated);
/*
 *  Following macros provide declaration of various resource calls.
 */
#define _MGR_ALLOC(_type) \
extern int \
bcm_dnx_##_type##_alloc(int unit, uint32 flags, int count, int *elem);
#define _MGR_ALLOC_ALIGN(_type) \
extern int \
bcm_dnx_##_type##_alloc_align(int unit, \
                              uint32 flags, \
                              int align, \
                              int offset, \
                              int count, \
                              int *elem);
#define _MGR_FREE(_type) \
extern int \
bcm_dnx_##_type##_free(int unit, int count, int elem);
#define _MGR_CHECK(_type) \
extern int \
bcm_dnx_##_type##_check(int unit, int count, int elem);
#define _MGR_ALLOC_GROUP(_type) \
extern int \
bcm_dnx_##_type##_alloc_group(int unit, \
                              uint32 grp_flags, \
                              int grp_size, \
                              int *grp_done, \
                              const uint32 *elem_flags, \
                              const int *elem_count, \
                              int *elem);
#define _MGR_ALLOC_ALIGN_GROUP(_type) \
extern int \
bcm_dnx_##_type##_alloc_align_group(int unit, \
                                    uint32 grp_flags, \
                                    int grp_size, \
                                    int *grp_done, \
                                    const uint32 *elem_flags, \
                                    const int *elem_align, \
                                    const int *elem_offset, \
                                    const int *elem_count, \
                                    int *elem);
#define _MGR_FREE_GROUP(_type) \
extern int \
bcm_dnx_##_type##_free_group(int unit, \
                             uint32 grp_flags, \
                             int grp_size, \
                             int *grp_done, \
                             const int *elem_count, \
                             const int *elem);
#define _MGR_CHECK_GROUP(_type) \
extern int \
bcm_dnx_##_type##_check_group(int unit, \
                              uint32 grp_flags, \
                              int grp_size, \
                              int *grp_done, \
                              const int *elem_count, \
                              const int *elem, \
                              int *status);
#define _MGR_ALL_ACTIONS(_type) \
_MGR_ALLOC(_type) \
_MGR_ALLOC_ALIGN(_type) \
_MGR_ALLOC_GROUP(_type) \
_MGR_ALLOC_ALIGN_GROUP(_type) \
_MGR_FREE(_type) \
_MGR_FREE_GROUP(_type) \
_MGR_CHECK(_type) \
_MGR_CHECK_GROUP(_type)
/*
 *  NOTE: must update the below list whenever changing resource types.
 */
/*
 *  Don't export the declaration macros.
 */
#undef _MGR_ALLOC
#undef _MGR_ALLOC_ALIGN
#undef _MGR_FREE
#undef _MGR_CHECK
#undef _MGR_ALLOC_GROUP
#undef _MGR_ALLOC_ALIGN_GROUP
#undef _MGR_FREE_GROUP
#undef _MGR_CHECK_GROUP
#undef _MGR_ALL_ACTIONS


#endif /* INCLUDE_DNX_ALLOC_MNGR_H */
