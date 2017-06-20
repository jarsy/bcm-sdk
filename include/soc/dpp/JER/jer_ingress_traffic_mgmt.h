/* $Id: jer_ingress_traffic_mgmt.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER_INGRESS_TRAFFIC_MGMT_INCLUDED__
/* { */
#define __JER_INGRESS_TRAFFIC_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_framework.h> 
#include <soc/error.h> 
#include <soc/dpp/TMC/tmc_api_ingress_traffic_mgmt.h>
#include <soc/dpp/ARAD/arad_api_ingress_traffic_mgmt.h> 

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
/* } */


/*********************************************************************
* NAME:
*     jer_itm_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
int
  jer_itm_init(
    SOC_SAND_IN  int  unit
  );
/*********************************************************************
*     Set ECN as enabled or disabled for the device
*********************************************************************/
uint32
  jer_itm_enable_ecn_set(
    SOC_SAND_IN  int   unit,
    SOC_SAND_IN  uint32   enabled /* ECN will be enabled/disabled for non zero/zero values */
  );

/*********************************************************************
*     Return if ECN is enabled for the device
*********************************************************************/
uint32
  jer_itm_enable_ecn_get(
    SOC_SAND_IN  int   unit,
    SOC_SAND_OUT uint32   *enabled /* will return non zero if /ECN is enabled */
  );

/********************************************************************* 
* NAME: 
*     arad_itm_ingress_shape_set_unsafe 
* TYPE: 
*   PROC 
* DATE: 
*   May 12 2008 
* FUNCTION: 
*     The device may have one continues range of queues that 
*     belongs to ingress shaping. And set the Ingress shaping 
*     credit generator. 
* INPUT: 
*  SOC_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  SOC_SAND_IN  ARAD_ITM_INGRESS_SHAPE_INFO *info - 
*     Ingress shaping configuration. 
* REMARKS: 
*     Base Queue number and add/subtract mode must be set 
*     prior to calling this API. To set base-q configuration, 
*     use ipq_explicit_mapping_mode_info_set API. 
* RETURNS: 
*     OK or ERROR indication. 
*********************************************************************/ 
uint32 
  jer_itm_ingress_shape_set( 
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_IN  SOC_TMC_ITM_INGRESS_SHAPE_INFO *info 
  ); 
 
/********************************************************************* 
* NAME: 
*     jer_itm_ingress_shape_get 
* TYPE: 
*   PROC 
* FUNCTION: 
*     Sets ingress shaping configuration. This includes 
*     ingress shaping queues range, and credit generation 
*     configuration. 
* INPUT: 
*  SOC_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  SOC_SAND_OUT ARAD_ITM_INGRESS_SHAPE_INFO *info - 
*     Ingress shaping configuration. 
* REMARKS: 
*     Base Queue number and add/subtract mode must be set 
*     prior to calling this API. To set base-q configuration, 
*     use ipq_explicit_mapping_mode_info_set API. 
* RETURNS: 
*     OK or ERROR indication. 
*********************************************************************/ 
uint32 
  jer_itm_ingress_shape_get( 
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_OUT SOC_TMC_ITM_INGRESS_SHAPE_INFO *info 
  ); 
 
/********************************************************************* 
* NAME: 
*     arad_itm_category_rngs_set 
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
*  SOC_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  SOC_SAND_IN  ARAD_ITM_CATEGORY_RNGS *info - 
*     pointer to configuration structure. 
* REMARKS: 
*     None. 
* RETURNS: 
*     OK or ERROR indication. 
*********************************************************************/ 
int 
  jer_itm_category_rngs_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  SOC_TMC_ITM_CATEGORY_RNGS *info
  );
/********************************************************************* 
* NAME: 
*     arad_itm_category_rngs_get 
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
*  SOC_SAND_IN  int                 unit - 
*     Identifier of the device to access. 
*  SOC_SAND_OUT ARAD_ITM_CATEGORY_RNGS *info - 
*     pointer to configuration structure. 
* REMARKS: 
*     None. 
* RETURNS: 
*   OK or ERROR indication. 
*********************************************************************/ 
int 
  jer_itm_category_rngs_get( 
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_OUT SOC_TMC_ITM_CATEGORY_RNGS *info 
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
  jer_itm_admit_test_tmplt_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint32                 admt_tst_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_ADMIT_TEST_TMPLT_INFO *info
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
  jer_itm_admit_test_tmplt_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint32              admt_tst_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_ADMIT_TEST_TMPLT_INFO *info
  );
int
  jer_itm_vsq_wred_gen_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    SOC_SAND_IN  uint32                         vsq_rt_cls_ndx,
    SOC_SAND_IN  int                            pool_id,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  );
int
  jer_itm_vsq_wred_gen_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    SOC_SAND_IN  uint32                         vsq_rt_cls_ndx,
    SOC_SAND_IN  int                            pool_id,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  );
int
    jer_itm_vsq_wred_get(
        SOC_SAND_IN  int                    unit,
        SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
        SOC_SAND_IN  uint32                 drop_precedence_ndx,
        SOC_SAND_IN  int                    pool_id,
        SOC_SAND_OUT SOC_TMC_ITM_WRED_QT_DP_INFO *info);

int
    jer_itm_vsq_wred_set(
        SOC_SAND_IN  int                    unit,
        SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
        SOC_SAND_IN  uint32                 drop_precedence_ndx,
        SOC_SAND_IN  int                    pool_id,
        SOC_SAND_IN  SOC_TMC_ITM_WRED_QT_DP_INFO *info, 
        SOC_SAND_OUT SOC_TMC_ITM_WRED_QT_DP_INFO *exact_info);
int
  jer_itm_vsq_tail_drop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  int                    pool_id,
    SOC_SAND_IN  int                    is_headroom,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *info,
    SOC_SAND_OUT  SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *exact_info
  );
int
  jer_itm_vsq_tail_drop_default_get(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_OUT SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  );
int
  jer_itm_vsq_tail_drop_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  int                    pool_id,
    SOC_SAND_IN  int                    is_headroom,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  );
int
  jer_itm_vsq_fc_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  int                        pool_id,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_FC_INFO     *info,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_FC_INFO     *exact_info
  );
int
  jer_itm_vsq_fc_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  int                    pool_id,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_FC_INFO     *info
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
  jer_itm_setup_dp_map(
    SOC_SAND_IN  int unit
  );
int
jer_itm_vsq_src_reserve_set(
   SOC_SAND_IN  int                     unit,
   SOC_SAND_IN  int                     core_id,
   SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP   vsq_type,
   SOC_SAND_IN  uint32                  vsq_rt_cls_ndx,
   SOC_SAND_IN  uint32                  drop_precedence_ndx,
   SOC_SAND_IN  int                     pool_id,
   SOC_SAND_IN  uint32                  reserved_amount,
   SOC_SAND_OUT uint32*                 exact_reserved_amount
   );
int
jer_itm_vsq_src_reserve_get(
   SOC_SAND_IN  int                     unit,
   SOC_SAND_IN  int                     core_id,
   SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP   vsq_type,
   SOC_SAND_IN  uint32                  vsq_rt_cls_ndx,
   SOC_SAND_IN  uint32                  drop_precedence_ndx,
   SOC_SAND_IN  int                     pool_id,
   SOC_SAND_OUT uint32                  *reserved_amount
   );
int
jer_itm_resource_allocation_set(
   SOC_SAND_IN  int                     unit,
   SOC_SAND_IN  int                     core_id,
   SOC_SAND_IN  uint8                   pool_id,
   SOC_SAND_IN  uint8                   is_ocb_only,
   SOC_SAND_IN  uint32                  max_shared_pool,
   SOC_SAND_IN  uint32                  max_headroom
   );
int
jer_itm_resource_allocation_get(
   SOC_SAND_IN  int                     unit,
   SOC_SAND_IN  int                     core_id,
   SOC_SAND_IN  uint8                   pool_id,
   SOC_SAND_IN  uint8                   is_ocb_only,
   SOC_SAND_OUT uint32                  *max_shared_pool,
   SOC_SAND_OUT uint32                  *max_headroom
   );
int
  jer_itm_global_resource_allocation_get(
      SOC_SAND_IN  int unit, 
      SOC_SAND_IN  int core, 
      SOC_SAND_OUT  SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  );
int
  jer_itm_global_resource_allocation_set(
      SOC_SAND_IN  int unit, 
      SOC_SAND_IN  int core, 
      SOC_SAND_IN  SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  );

int
  jer_itm_dyn_total_thresh_set(
    SOC_SAND_IN  int      unit,
    SOC_SAND_IN  int      core_id,
    SOC_SAND_IN  uint8    is_ocb_only,
                 int32    reservation_increase[SOC_DPP_DEFS_MAX(NOF_CORES)][SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] /* the (signed) amount in which the thresholds should decrease (according to 100% as will be set for DP 0) */
  );

int
  jer_ingress_drop_status(
    SOC_SAND_IN int     unit,
    SOC_SAND_OUT uint32 *is_max_size 
  );

int
jer_itm_vsq_pg_tc_profile_mapping_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32 src_pp_port,
    SOC_SAND_IN int pg_tc_profile
   );

int
jer_itm_vsq_pg_tc_profile_mapping_get(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32 src_pp_port,
    SOC_SAND_OUT int *pg_tc_profile
   );

int
jer_itm_vsq_pg_tc_profile_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id,
    SOC_SAND_IN int pg_tc_profile_id,
    SOC_SAND_IN uint32 pg_tc_bitmap
   );

int
jer_itm_vsq_pg_tc_profile_get(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN int         core_id,
    SOC_SAND_IN int         pg_tc_profile_id,
    SOC_SAND_OUT uint32     *pg_tc_bitmap
   );
int
  jer_itm_vsq_pg_prm_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              vsq_rt_cls_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_PG_PRM *info
  );
int
  jer_itm_vsq_pg_prm_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              vsq_rt_cls_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_PG_PRM *info
  );
int
jer_itm_src_vsqs_mapping_set(
       SOC_SAND_IN int unit,
       SOC_SAND_IN int core_id,
       SOC_SAND_IN int src_port_vsq_index,
       SOC_SAND_IN int src_pp_port,
       SOC_SAND_IN int pg_base,
       SOC_SAND_IN uint8 enable
       );
int
jer_itm_src_vsqs_mapping_get(
       SOC_SAND_IN int unit,
       SOC_SAND_IN int core_id,
       SOC_SAND_IN int src_pp_port,
       SOC_SAND_OUT int *src_port_vsq_index,
       SOC_SAND_OUT int *pg_base,
       SOC_SAND_OUT uint8 *enable
       );
int
    jer_itm_vsq_src_port_get(
       SOC_SAND_IN int    unit,
       SOC_SAND_IN int    core_id,
       SOC_SAND_IN int    src_port_vsq_index,
       SOC_SAND_OUT uint32 *src_pp_port,
       SOC_SAND_OUT uint8  *enable
       );
int
jer_itm_vsq_pg_mapping_get(
   SOC_SAND_IN int     unit,
   SOC_SAND_IN int     core_id,
   SOC_SAND_IN uint32  pg_vsq_base,
   SOC_SAND_IN int     cosq,
   SOC_SAND_OUT uint32 *src_pp_port,
   SOC_SAND_OUT uint8  *enable
   );
uint32
jer_itm_vsq_pg_ocb_set(
       SOC_SAND_IN int unit,
       SOC_SAND_IN int core_id,
       SOC_SAND_IN int pg_base,
       SOC_SAND_IN int numq,
       SOC_SAND_IN uint8* ocb_only
       );
uint32
jer_itm_vsq_pg_ocb_get(
       SOC_SAND_IN int unit,
       SOC_SAND_IN int core_id,
       SOC_SAND_IN int pg_base,
       SOC_SAND_IN int numq,
       SOC_SAND_OUT uint8* ocb_only
       );
int
jer_itm_glob_rcs_drop_get(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   int                           core_id,
    SOC_SAND_OUT  SOC_TMC_ITM_GLOB_RCS_DROP_TH  *info
    );
int
  jer_itm_glob_rcs_drop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  SOC_TMC_ITM_GLOB_RCS_DROP_TH *info,
    SOC_SAND_OUT SOC_TMC_ITM_GLOB_RCS_DROP_TH *exact_info
  );
int
  jer_itm_sys_red_queue_size_boundaries_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_INFO *info,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *exact_info
  );
int
  jer_itm_sys_red_queue_size_boundaries_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *info
  );
int
    jer_itm_sys_red_glob_rcs_set(
        SOC_SAND_IN  int                 unit,
        SOC_SAND_IN  ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info
  );
int
  jer_itm_sys_red_glob_rcs_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info
  );

int
  jer_itm_in_pp_port_scheduler_compensation_profile_set(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32          in_pp_port,
    SOC_SAND_IN   int  scheduler_profile
          );

int
  jer_itm_in_pp_port_scheduler_compensation_profile_get(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32          in_pp_port,
    SOC_SAND_OUT  int*  scheduler_profile
          );

int jer_itm_credits_adjust_size_set (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    SOC_SAND_IN SOC_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    SOC_SAND_IN int   delta,
    SOC_SAND_IN SOC_TMC_ITM_PKT_SIZE_ADJUST_INFO* additional_info
  );

int jer_itm_credits_adjust_size_get (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    SOC_SAND_IN SOC_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    SOC_SAND_OUT int   *delta
  );

int jer_itm_sched_compensation_offset_and_delta_get (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   queue_profile,
    SOC_SAND_IN int   queue_delta, 
    SOC_SAND_IN int   in_pp_port_profile,
    SOC_SAND_IN int   in_pp_port_delta, 
    SOC_SAND_OUT int* offset,
    SOC_SAND_OUT int* delta
        ); 
 
int jer_itm_sch_final_delta_map_set (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN int   delta,
    SOC_SAND_IN uint32   delta_profile
  );

int jer_itm_sch_final_delta_map_get (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN int   delta,
    SOC_SAND_OUT uint32   *delta_profile
  );

int jer_itm_sch_final_delta_set (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN uint32   delta_profile,
    SOC_SAND_IN int   final_delta
  );

int jer_itm_sch_final_delta_get (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN uint32   delta_profile,
    SOC_SAND_OUT int   *final_delta
  );

int 
  jer_itm_congestion_statistics_get(
      SOC_SAND_IN int unit,
      SOC_SAND_IN int core,
      SOC_SAND_OUT ARAD_ITM_CGM_CONGENSTION_STATS *stats 
  );

int
  jer_itm_min_free_resources_stat_get(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core,
    SOC_SAND_IN SOC_TMC_ITM_CGM_RSRC_STAT_TYPE type,
    SOC_SAND_OUT uint64 *value
  );

int jer_itm_ingress_latency_init(int unit);
int jer_itm_max_latency_pkts_get(int unit, int max_count, SOC_TMC_MAX_LATENCY_PACKETS_INFO *max_latency_packets, int *actual_count);

int jer_itm_rate_limit_mpps_set(int unit, int rate);
int jer_itm_rate_limit_mpps_get(int unit, int* rate);


#include <soc/dpp/SAND/Utils/sand_footer.h> 
/* } __JER_INGRESS_TRAFFIC_MGMT_INCLUDED__*/ 
#endif
