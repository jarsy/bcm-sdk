/* $Id: jer2_jer_ingress_traffic_mgmt.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_JER_INGRESS_TRAFFIC_MGMT_INCLUDED__
/* { */
#define __JER2_JER_INGRESS_TRAFFIC_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h> 
#include <soc/error.h> 
#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_api_ingress_traffic_mgmt.h> 

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
/* } */


/*********************************************************************
* NAME:
*     jer2_jer_itm_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
int
  jer2_jer_itm_init(
    DNX_SAND_IN  int  unit
  );
/*********************************************************************
*     Set ECN as enabled or disabled for the device
*********************************************************************/
uint32
  jer2_jer_itm_enable_ecn_set(
    DNX_SAND_IN  int   unit,
    DNX_SAND_IN  uint32   enabled /* ECN will be enabled/disabled for non zero/zero values */
  );

/*********************************************************************
*     Return if ECN is enabled for the device
*********************************************************************/
uint32
  jer2_jer_itm_enable_ecn_get(
    DNX_SAND_IN  int   unit,
    DNX_SAND_OUT uint32   *enabled /* will return non zero if /ECN is enabled */
  );

/********************************************************************* 
* NAME: 
*     jer2_arad_itm_ingress_shape_set_unsafe 
* TYPE: 
*   PROC 
* DATE: 
*   May 12 2008 
* FUNCTION: 
*     The device may have one continues range of queues that 
*     belongs to ingress shaping. And set the Ingress shaping 
*     credit generator. 
* INPUT: 
*  DNX_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  DNX_SAND_IN  JER2_ARAD_ITM_INGRESS_SHAPE_INFO *info - 
*     Ingress shaping configuration. 
* REMARKS: 
*     Base Queue number and add/subtract mode must be set 
*     prior to calling this API. To set base-q configuration, 
*     use ipq_explicit_mapping_mode_info_set API. 
* RETURNS: 
*     OK or ERROR indication. 
*********************************************************************/ 
uint32 
  jer2_jer_itm_ingress_shape_set( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 core_id, 
    DNX_SAND_IN  DNX_TMC_ITM_INGRESS_SHAPE_INFO *info 
  ); 
 
/********************************************************************* 
* NAME: 
*     jer2_jer_itm_ingress_shape_get 
* TYPE: 
*   PROC 
* FUNCTION: 
*     Sets ingress shaping configuration. This includes 
*     ingress shaping queues range, and credit generation 
*     configuration. 
* INPUT: 
*  DNX_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  DNX_SAND_OUT JER2_ARAD_ITM_INGRESS_SHAPE_INFO *info - 
*     Ingress shaping configuration. 
* REMARKS: 
*     Base Queue number and add/subtract mode must be set 
*     prior to calling this API. To set base-q configuration, 
*     use ipq_explicit_mapping_mode_info_set API. 
* RETURNS: 
*     OK or ERROR indication. 
*********************************************************************/ 
uint32 
  jer2_jer_itm_ingress_shape_get( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 core_id, 
    DNX_SAND_OUT DNX_TMC_ITM_INGRESS_SHAPE_INFO *info 
  ); 
 
/********************************************************************* 
* NAME: 
*     jer2_arad_itm_category_rngs_set 
* TYPE: 
*   PROC 
* DATE: 
*   May 12 2008 
* FUNCTION: 
*     Defines packet queues categories - in contiguous blocks. 
*     IQM queues are divided to 4 categories in contiguous 
*     blocks. Category-4 from 'category-end-3' till the last 
*     queue (32K). 
* INPUT: 
*  DNX_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  DNX_SAND_IN  JER2_ARAD_ITM_CATEGORY_RNGS *info - 
*     pointer to configuration structure. 
* REMARKS: 
*     None. 
* RETURNS: 
*     OK or ERROR indication. 
*********************************************************************/ 
int 
  jer2_jer_itm_category_rngs_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  DNX_TMC_ITM_CATEGORY_RNGS *info
  );
/********************************************************************* 
* NAME: 
*     jer2_arad_itm_category_rngs_get 
* TYPE: 
*   PROC 
* DATE: 
*   May 12 2008 
* FUNCTION: 
*     Defines packet queues categories - in contiguous blocks. 
*     IQM queues are divided to 4 categories in contiguous 
*     blocks. Category-4 from 'category-end-3' till the last 
*     queue (32K). 
* INPUT: 
*  DNX_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  DNX_SAND_OUT JER2_ARAD_ITM_CATEGORY_RNGS *info - 
*     pointer to configuration structure. 
* REMARKS: 
*     None. 
* RETURNS: 
*   OK or ERROR indication. 
*********************************************************************/ 
int 
  jer2_jer_itm_category_rngs_get( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 core_id, 
    DNX_SAND_OUT DNX_TMC_ITM_CATEGORY_RNGS *info 
  );
/*********************************************************************
*     In order to admit a packet to a queue, the packet must
*     pass the admit-test-logic. The packet belogs to some VSQs
*     out of the 4 types of VSQs. For each VSQ which the packet
*     belongs to it encounters WRED and Tail-Drop mechanisms.
*     The admit-test-template determines which, if at all, of
*     the VSQ groups and their reject mechanisms must the packet
*     consider. A test template consists of two optional combinations
*     of VSQ groups to consider (testA, testB).
*     Each queue (VOQ) is assigned with a test template.
*     Notice that in a queue, is a packet is chosen to be rejected
*     normally, the admit test logic will not affect it.
*     From the Data Sheet:
*     The Packet Queue Rate Class is used to select one of four
*     Admission Logic Templates. Each template is an 8-bit variable
*     {a1,b1,c1,d1,a2,b2,c2,d2} applied as detailed below:
*
*     Final-Admit =
*       GL-Admit & PQ-Admit &
*       ((a1 | CT-Admit) & (b1 | CTTC-Admit) &
*             (c1 | CTCC-Admit) & (d1 |STF-Admit ) OR
*         (a2 | CT-Admit) & (b2 | CTTC-Admit)  &
*             (c2 | CTCC-Admit) & (d2 |STF-Admit)) &
*       (!PQ-Sys-Red-Ena | SR-Admit)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_jer_itm_admit_test_tmplt_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  uint32                 admt_tst_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_ADMIT_TEST_TMPLT_INFO *info
  );
/*********************************************************************
*     In order to admit a packet to a queue, the packet must
*     pass the admit-test-logic. The packet belogs to some VSQs
*     out of the 4 types of VSQs. For each VSQ which the packet
*     belongs to it encounters WRED and Tail-Drop mechanisms.
*     The admit-test-template determines which, if at all, of
*     the VSQ groups and their reject mechanisms must the packet
*     consider. A test template consists of two optional combinations
*     of VSQ groups to consider (testA, testB).
*     Each queue (VOQ) is assigned with a test template.
*     Notice that in a queue, is a packet is chosen to be rejected
*     normally, the admit test logic will not affect it.
*     From the Data Sheet:
*     The Packet Queue Rate Class is used to select one of four
*     Admission Logic Templates. Each template is an 8-bit variable
*     {a1,b1,c1,d1,a2,b2,c2,d2} applied as detailed below:
*
*     Final-Admit =
*       GL-Admit & PQ-Admit &
*       ((a1 | CT-Admit) & (b1 | CTTC-Admit) &
*             (c1 | CTCC-Admit) & (d1 |STF-Admit ) OR
*         (a2 | CT-Admit) & (b2 | CTTC-Admit)  &
*             (c2 | CTCC-Admit) & (d2 |STF-Admit)) &
*       (!PQ-Sys-Red-Ena | SR-Admit)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_jer_itm_admit_test_tmplt_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  uint32              admt_tst_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_ADMIT_TEST_TMPLT_INFO *info
  );
int
  jer2_jer_itm_vsq_wred_gen_set(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    DNX_SAND_IN  uint32                         vsq_rt_cls_ndx,
    DNX_SAND_IN  int                            pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  );
int
  jer2_jer_itm_vsq_wred_gen_get(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    DNX_SAND_IN  uint32                         vsq_rt_cls_ndx,
    DNX_SAND_IN  int                            pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  );
int
    jer2_jer_itm_vsq_wred_get(
        DNX_SAND_IN  int                    unit,
        DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
        DNX_SAND_IN  uint32                 drop_precedence_ndx,
        DNX_SAND_IN  int                    pool_id,
        DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO *info);

int
    jer2_jer_itm_vsq_wred_set(
        DNX_SAND_IN  int                    unit,
        DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
        DNX_SAND_IN  uint32                 drop_precedence_ndx,
        DNX_SAND_IN  int                    pool_id,
        DNX_SAND_IN  DNX_TMC_ITM_WRED_QT_DP_INFO *info, 
        DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO *exact_info);
int
  jer2_jer_itm_vsq_tail_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP       vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  int                    is_headroom,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info,
    DNX_SAND_OUT  DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *exact_info
  );
int
  jer2_jer_itm_vsq_tail_drop_default_get(
      DNX_SAND_IN  int                 unit,
      DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  );
int
  jer2_jer_itm_vsq_tail_drop_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  int                    is_headroom,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  );
int
  jer2_jer_itm_vsq_fc_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP       vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  int                        pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_FC_INFO     *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_FC_INFO     *exact_info
  );
int
  jer2_jer_itm_vsq_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP       vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_FC_INFO     *info
  );

/**
 * Initialize the Mrps-In-Dp-Mapping and 
 * Drop-Precedence-Mapping. 
 * MrpsInDpMapping: 
 * <dp-meter-cmd,incoming-dp,Ethernet-meter-dp,Ethernet-meter-invalid> 
 * -> <mrps-in-dp,mrps-in-drop> 
 * DropPrecedenceMapping:
 * <dp-meter-cmd,incoming-dp,Ethernet-meter-dp,Meter-processor-dp> 
 * -> <ingress-dp,egress-dp>  
 * The (auto-detected) mode (PP or TM) determines the mapping 
 * (for TM we ignore the meter and the ingress-dp is passed 
 * through). 
 * 
 * @param unit 
 * 
 * @return uint32 
 */
uint32 
  jer2_jer_itm_setup_dp_map(
    DNX_SAND_IN  int unit
  );
int
jer2_jer_itm_vsq_src_reserve_set(
   DNX_SAND_IN  int                     unit,
   DNX_SAND_IN  int                     core_id,
   DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP   vsq_type,
   DNX_SAND_IN  uint32                  vsq_rt_cls_ndx,
   DNX_SAND_IN  uint32                  drop_precedence_ndx,
   DNX_SAND_IN  int                     pool_id,
   DNX_SAND_IN  uint32                  reserved_amount,
   DNX_SAND_OUT uint32*                 exact_reserved_amount
   );
int
jer2_jer_itm_vsq_src_reserve_get(
   DNX_SAND_IN  int                     unit,
   DNX_SAND_IN  int                     core_id,
   DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP   vsq_type,
   DNX_SAND_IN  uint32                  vsq_rt_cls_ndx,
   DNX_SAND_IN  uint32                  drop_precedence_ndx,
   DNX_SAND_IN  int                     pool_id,
   DNX_SAND_OUT uint32                  *reserved_amount
   );
uint32
jer2_jer_itm_resource_allocation_set(
   DNX_SAND_IN  int                     unit,
   DNX_SAND_IN  int                     core_id,
   DNX_SAND_IN  uint8                   pool_id,
   DNX_SAND_IN  uint8                   is_ocb_only,
   DNX_SAND_IN  uint32                  max_shared_pool,
   DNX_SAND_IN  uint32                  max_headroom
   );
uint32
jer2_jer_itm_resource_allocation_get(
   DNX_SAND_IN  int                     unit,
   DNX_SAND_IN  int                     core_id,
   DNX_SAND_IN  uint8                   pool_id,
   DNX_SAND_IN  uint8                   is_ocb_only,
   DNX_SAND_OUT uint32                  *max_shared_pool,
   DNX_SAND_OUT uint32                  *max_headroom
   );
uint32
  jer2_jer_itm_global_resource_allocation_get(
      DNX_SAND_IN  int unit, 
      DNX_SAND_IN  int core, 
      DNX_SAND_OUT  DNX_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  );
uint32
  jer2_jer_itm_global_resource_allocation_set(
      DNX_SAND_IN  int unit, 
      DNX_SAND_IN  int core, 
      DNX_SAND_IN  DNX_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  );

int
  jer2_jer_itm_dyn_total_thresh_set(
    DNX_SAND_IN  int      unit,
    DNX_SAND_IN  int      core_id,
    DNX_SAND_IN  uint8    is_ocb_only,
                 int32    reservation_increase[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES] /* the (signed) amount in which the thresholds should decrease (according to 100% as will be set for DP 0) */
  );

uint32
  jer2_jer_ingress_drop_status(
    DNX_SAND_IN int   unit,
    DNX_SAND_OUT uint32 *is_max_size 
  );

int
jer2_jer_itm_vsq_pg_tc_profile_mapping_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN uint32 src_pp_port,
    DNX_SAND_IN int pg_tc_profile
   );

int
jer2_jer_itm_vsq_pg_tc_profile_mapping_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN uint32 src_pp_port,
    DNX_SAND_OUT int *pg_tc_profile
   );

int
jer2_jer_itm_vsq_pg_tc_profile_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN int pg_tc_profile_id,
    DNX_SAND_IN uint32 pg_tc_bitmap
   );

int
jer2_jer_itm_vsq_pg_tc_profile_get(
    DNX_SAND_IN int         unit,
    DNX_SAND_IN int         core_id,
    DNX_SAND_IN int         pg_tc_profile_id,
    DNX_SAND_OUT uint32     *pg_tc_bitmap
   );
int
  jer2_jer_itm_vsq_pg_prm_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              vsq_rt_cls_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_PG_PRM *info
  );
int
  jer2_jer_itm_vsq_pg_prm_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              vsq_rt_cls_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_PG_PRM *info
  );
int
jer2_jer_itm_src_vsqs_mapping_set(
       DNX_SAND_IN int unit,
       DNX_SAND_IN int core_id,
       DNX_SAND_IN int src_port_vsq_index,
       DNX_SAND_IN int src_pp_port,
       DNX_SAND_IN int pg_base,
       DNX_SAND_IN uint8 enable
       );
int
jer2_jer_itm_src_vsqs_mapping_get(
       DNX_SAND_IN int unit,
       DNX_SAND_IN int core_id,
       DNX_SAND_IN int src_pp_port,
       DNX_SAND_OUT int *src_port_vsq_index,
       DNX_SAND_OUT int *pg_base,
       DNX_SAND_OUT uint8 *enable
       );
int
    jer2_jer_itm_vsq_src_port_get(
       DNX_SAND_IN int    unit,
       DNX_SAND_IN int    core_id,
       DNX_SAND_IN int    src_port_vsq_index,
       DNX_SAND_OUT uint32 *src_pp_port,
       DNX_SAND_OUT uint8  *enable
       );
int
jer2_jer_itm_vsq_pg_mapping_get(
   DNX_SAND_IN int     unit,
   DNX_SAND_IN int     core_id,
   DNX_SAND_IN uint32  pg_vsq_base,
   DNX_SAND_IN int     cosq,
   DNX_SAND_OUT uint32 *src_pp_port,
   DNX_SAND_OUT uint8  *enable
   );
uint32
jer2_jer_itm_vsq_pg_ocb_set(
       DNX_SAND_IN int unit,
       DNX_SAND_IN int core_id,
       DNX_SAND_IN int pg_base,
       DNX_SAND_IN int numq,
       DNX_SAND_IN uint8* ocb_only
       );
uint32
jer2_jer_itm_vsq_pg_ocb_get(
       DNX_SAND_IN int unit,
       DNX_SAND_IN int core_id,
       DNX_SAND_IN int pg_base,
       DNX_SAND_IN int numq,
       DNX_SAND_OUT uint8* ocb_only
       );
int
jer2_jer_itm_glob_rcs_drop_get(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core_id,
    DNX_SAND_OUT  DNX_TMC_ITM_GLOB_RCS_DROP_TH  *info
    );
int
  jer2_jer_itm_glob_rcs_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  DNX_TMC_ITM_GLOB_RCS_DROP_TH *info,
    DNX_SAND_OUT DNX_TMC_ITM_GLOB_RCS_DROP_TH *exact_info
  );
int
  jer2_jer_itm_sys_red_queue_size_boundaries_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_ITM_SYS_RED_QT_INFO *info,
    DNX_SAND_OUT JER2_ARAD_ITM_SYS_RED_QT_INFO *exact_info
  );
int
  jer2_jer_itm_sys_red_queue_size_boundaries_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_OUT JER2_ARAD_ITM_SYS_RED_QT_INFO *info
  );
int
    jer2_jer_itm_sys_red_glob_rcs_set(
        DNX_SAND_IN  int                 unit,
        DNX_SAND_IN  JER2_ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info
  );
int
  jer2_jer_itm_sys_red_glob_rcs_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info
  );
int jer2_jer_itm_credits_adjust_size_set (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    DNX_SAND_IN DNX_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    DNX_SAND_IN int   delta
  );

int jer2_jer_itm_credits_adjust_size_get (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    DNX_SAND_IN DNX_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    DNX_SAND_OUT int   *delta
  );
 
int jer2_jer_itm_sch_final_delta_map_set (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   delta,
    DNX_SAND_IN uint32   delta_profile
  );

int jer2_jer_itm_sch_final_delta_map_get (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   delta,
    DNX_SAND_OUT uint32   *delta_profile
  );

int jer2_jer_itm_sch_final_delta_set (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN uint32   delta_profile,
    DNX_SAND_IN int   final_delta
  );

int jer2_jer_itm_sch_final_delta_get (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN uint32   delta_profile,
    DNX_SAND_OUT int   *final_delta
  );

int 
  jer2_jer_itm_congestion_statistics_get(
      DNX_SAND_IN int unit,
      DNX_SAND_IN int core,
      DNX_SAND_OUT JER2_ARAD_ITM_CGM_CONGENSTION_STATS *stats 
  );

int
  jer2_jer_itm_min_free_resources_stat_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core,
    DNX_SAND_IN DNX_TMC_ITM_CGM_RSRC_STAT_TYPE type,
    DNX_SAND_OUT uint64 *value
  );

int jer2_jer_itm_ingress_latency_init(int unit);
int jer2_jer_itm_max_latency_pkts_get(int unit, int max_count, DNX_TMC_MAX_LATENCY_PACKETS_INFO *max_latency_packets, int *actual_count);

int jer2_jer_itm_rate_limit_mpps_set(int unit, int rate);
int jer2_jer_itm_rate_limit_mpps_get(int unit, int* rate);


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h> 
/* } __JER2_JER_INGRESS_TRAFFIC_MGMT_INCLUDED__*/ 
#endif
