/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_qax_mgmt.h
 */

#ifndef __JER2_QAX_INGRESS_TRAFFIC_MGMT_INCLUDED__

#define __JER2_QAX_INGRESS_TRAFFIC_MGMT_INCLUDED__

#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>

int
  jer2_qax_itm_init(
    DNX_SAND_IN  int  unit
  );

int
  jer2_qax_itm_per_queue_info_set(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          queue,
    DNX_SAND_IN   JER2_ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  );

int
  jer2_qax_itm_per_queue_info_get(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          queue,
    DNX_SAND_OUT   JER2_ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  );


int
  jer2_qax_itm_profile_ocb_only_set(
    DNX_SAND_IN   int unit,
    DNX_SAND_IN   int rate_class,
    DNX_SAND_IN   int is_ocb_only
  );

int
  jer2_qax_itm_profile_ocb_only_get(
    DNX_SAND_IN   int unit,
    DNX_SAND_IN   int rate_class,
    DNX_SAND_OUT  int *is_ocb_only
  );

int 
  jer2_qax_itm_congestion_statistics_get(
      DNX_SAND_IN int unit,
      DNX_SAND_IN int core,
      DNX_SAND_OUT JER2_ARAD_ITM_CGM_CONGENSTION_STATS *stats 
  );

int
  jer2_qax_itm_min_free_resources_stat_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core,
    DNX_SAND_IN DNX_TMC_ITM_CGM_RSRC_STAT_TYPE type,
    DNX_SAND_OUT uint64 *value
  );

int
  jer2_qax_itm_dp_discard_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint32                  discard_dp
  );

int
  jer2_qax_itm_dp_discard_get(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_OUT uint32                  *discard_dp
  );

int
  jer2_qax_itm_committed_q_size_set(
      DNX_SAND_IN  int                  unit,
      DNX_SAND_IN  uint32                  rt_cls_ndx,
      DNX_SAND_IN  DNX_TMC_ITM_GUARANTEED_INFO *info, 
      DNX_SAND_OUT DNX_TMC_ITM_GUARANTEED_INFO *exact_info 
      );
int
  jer2_qax_itm_committed_q_size_get(
      DNX_SAND_IN  int                  unit,
      DNX_SAND_IN  uint32                  rt_cls_ndx,
      DNX_SAND_OUT DNX_TMC_ITM_GUARANTEED_INFO *exact_info
      );

int
  jer2_qax_itm_fadt_tail_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_FADT_DROP_INFO  *info,
    DNX_SAND_OUT DNX_TMC_ITM_FADT_DROP_INFO  *exact_info
  );

int
  jer2_qax_itm_fadt_tail_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_FADT_DROP_INFO  *info
  );
int
  jer2_qax_itm_tail_drop_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  JER2_ARAD_ITM_TAIL_DROP_INFO  *info,
    DNX_SAND_OUT JER2_ARAD_ITM_TAIL_DROP_INFO  *exact_info
          );

int
  jer2_qax_itm_tail_drop_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT JER2_ARAD_ITM_TAIL_DROP_INFO  *info
          );

int
  jer2_qax_itm_wred_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  JER2_ARAD_ITM_WRED_QT_DP_INFO *info,
    DNX_SAND_OUT JER2_ARAD_ITM_WRED_QT_DP_INFO *exact_info
  );

int
  jer2_qax_itm_wred_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT JER2_ARAD_ITM_WRED_QT_DP_INFO *info
  );

int
  jer2_qax_itm_wred_exp_wq_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                  exp_wq,
    DNX_SAND_IN  uint8                   enable
          );
int
  jer2_qax_itm_wred_exp_wq_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_OUT  uint32                  *exp_wq
          );

int
  jer2_qax_itm_dyn_total_thresh_set(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN int    core,
    DNX_SAND_IN uint8  is_ocb_only,
                int32  reservation_increase_array[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES] /* the (signed) amount in which the thresholds should decrease (according to 100% as will be set for DP 0) */
          );
int
  jer2_qax_itm_queue_dyn_info_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_QUEUE_DYN_INFO *info
          );  

int
  jer2_qax_itm_dram_bound_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
                 DNX_TMC_ITM_DRAM_BOUND_INFO  *info,
    DNX_SAND_OUT DNX_TMC_ITM_DRAM_BOUND_INFO  *exact_info
          );

int
  jer2_qax_itm_dram_bound_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_DRAM_BOUND_INFO  *info
          );


/* VSQ functions */

int
  jer2_qax_itm_vsq_pg_tc_profile_mapping_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN uint32 src_pp_port,
    DNX_SAND_IN int pg_tc_profile
  );

int
  jer2_qax_itm_vsq_pg_tc_profile_mapping_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN uint32 src_pp_port,
    DNX_SAND_OUT int *pg_tc_profile
  );

int
  jer2_qax_itm_vsq_pg_tc_profile_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN int pg_tc_profile_id,
    DNX_SAND_IN uint32 pg_tc_bitmap
  );

int
  jer2_qax_itm_vsq_pg_tc_profile_get(
    DNX_SAND_IN int         unit,
    DNX_SAND_IN int         core_id,
    DNX_SAND_IN int         pg_tc_profile_id,
    DNX_SAND_OUT uint32     *pg_tc_bitmap
  );

int
  jer2_qax_itm_vsq_pb_prm_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              pg_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_PG_PRM *pg_prm
  );

int
  jer2_qax_itm_vsq_pb_prm_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              pg_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_PG_PRM *pg_prm
  );

int
  jer2_qax_itm_src_vsqs_mapping_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN int src_pp_port,
    DNX_SAND_IN int src_port_vsq_index,
    DNX_SAND_IN int pg_base,
    DNX_SAND_IN uint8 enable
  );

int
  jer2_qax_itm_src_vsqs_mapping_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core_id,
    DNX_SAND_IN  int src_pp_port,
    DNX_SAND_OUT int *src_port_vsq_index,
    DNX_SAND_OUT int *pg_base,
    DNX_SAND_OUT uint8 *enable
  );

int
  jer2_qax_itm_vsq_wred_get(
      DNX_SAND_IN  int                    unit,
      DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
      DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
      DNX_SAND_IN  uint32                 drop_precedence_ndx,
      DNX_SAND_IN  int                    pool_id,
      DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO *info
  );

int
  jer2_qax_itm_vsq_wred_set(
      DNX_SAND_IN  int                    unit,
      DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
      DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
      DNX_SAND_IN  uint32                 drop_precedence_ndx,
      DNX_SAND_IN  int                    pool_id,
      DNX_SAND_IN  DNX_TMC_ITM_WRED_QT_DP_INFO *info,
      DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO *exact_info
  );

int
  jer2_qax_itm_vsq_wred_gen_set(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    DNX_SAND_IN  uint32                         vsq_rt_cls_ndx,
    DNX_SAND_IN  int                            pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  );

int
  jer2_qax_itm_vsq_wred_gen_get(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    DNX_SAND_IN  uint32                         vsq_rt_cls_ndx,
    DNX_SAND_IN  int                            pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  );

int
  jer2_qax_itm_vsq_tail_drop_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  int                    is_headroom,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *exact_info
  );

int
  jer2_qax_itm_vsq_tail_drop_default_get(
      DNX_SAND_IN  int                 unit,
      DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  );

int
  jer2_qax_itm_vsq_tail_drop_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  int                    is_headroom,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  );

int
  jer2_qax_itm_vsq_src_port_rjct_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_SRC_PORT_INFO       *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_SRC_PORT_INFO       *exact_info
  );

int
  jer2_qax_itm_vsq_src_port_rjct_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_SRC_PORT_INFO       *exact_info
  );

int
  jer2_qax_itm_vsq_pg_rjct_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                     drop_precedence_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_PG_INFO    *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_PG_INFO    *exact_info
  );

int
  jer2_qax_itm_vsq_pg_rjct_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                     drop_precedence_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_PG_INFO    *exact_info
  );

int
  jer2_qax_itm_vsq_qt_rt_cls_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  uint8               is_ocb_only,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_NDX    vsq_in_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls
  );

int
  jer2_qax_itm_vsq_qt_rt_cls_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  uint8               is_ocb_only,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_NDX    vsq_in_group_ndx,
    DNX_SAND_OUT uint32                 *vsq_rt_cls
  );

int
  jer2_qax_itm_vsq_src_port_get(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN int    core_id,
    DNX_SAND_IN int    src_port_vsq_index,
    DNX_SAND_OUT uint32 *src_pp_port,
    DNX_SAND_OUT uint8  *enable
  );

int
  jer2_qax_itm_vsq_pg_mapping_get(
    DNX_SAND_IN int     unit,
    DNX_SAND_IN int     core_id,
    DNX_SAND_IN uint32  pg_vsq_base,
    DNX_SAND_IN int     cosq,
    DNX_SAND_OUT uint32 *src_pp_port,
    DNX_SAND_OUT uint8  *enable
  );

int 
  jer2_qax_itm_category_rngs_set( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 core_id, 
    DNX_SAND_IN  DNX_TMC_ITM_CATEGORY_RNGS *info 
  ); 

int 
  jer2_qax_itm_category_rngs_get( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 core_id, 
    DNX_SAND_OUT DNX_TMC_ITM_CATEGORY_RNGS *info 
  ); 

int
  jer2_qax_itm_glob_rcs_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  DNX_TMC_ITM_GLOB_RCS_DROP_TH *info,
    DNX_SAND_OUT DNX_TMC_ITM_GLOB_RCS_DROP_TH *exact_info
  );

int
  jer2_qax_itm_glob_rcs_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_OUT  DNX_TMC_ITM_GLOB_RCS_DROP_TH *info
  );

int
  jer2_qax_itm_vsq_fc_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  int                        pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_FC_INFO    *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_FC_INFO    *exact_info
  );

int
  jer2_qax_itm_vsq_fc_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  int                        pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_FC_INFO    *info
  );

int jer2_qax_itm_credits_adjust_size_set (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    DNX_SAND_IN DNX_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    DNX_SAND_IN int   delta
        );

int jer2_qax_itm_credits_adjust_size_get (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    DNX_SAND_IN DNX_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    DNX_SAND_OUT int   *delta
        ); 


#endif /*__JER2_QAX_INGRESS_TRAFFIC_MGMT_INCLUDED__ */

