/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef __JER2_QAX_EGR_QUEUING_INCLUDED__

#define __JER2_QAX_EGR_QUEUING_INCLUDED__

#include <soc/dnx/legacy/JER/jer_egr_queuing.h>
int
  jer2_qax_egr_queuing_if_fc_uc_max_set(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  );

int
  jer2_qax_egr_queuing_if_fc_uc_set(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  );

int
  jer2_qax_egr_dev_fc_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_FC_DEVICE_THRESH *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEVICE_THRESH *exact_thresh
  );

int
  jer2_qax_egr_dev_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEVICE_THRESH *thresh
  );

int
  jer2_qax_egr_ofp_fc_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_FC_OFP_THRESH   *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH   *exact_thresh
  );

int
  jer2_qax_egr_ofp_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH   *thresh
  );

int
  jer2_qax_egr_queuing_global_drop_set(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_IN  int    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_global_drop_get(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_OUT int*    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_sp_tc_drop_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_sp_tc_drop_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_sp_reserved_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_sp_reserved_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_global_fc_set(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_IN  int    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_global_fc_get(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_OUT int*    threshold_value,
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  );

int
  jer2_qax_egr_queuing_mc_tc_fc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value
  );

int
  jer2_qax_egr_queuing_mc_tc_fc_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value
  );

int
  jer2_qax_egr_queuing_dev_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN          JER2_ARAD_EGR_QUEUING_DEV_TH    *info
  );

int
  jer2_qax_egr_queuing_dev_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_OUT          JER2_ARAD_EGR_QUEUING_DEV_TH    *info
  );

int
  jer2_qax_egr_queuing_mc_cos_map_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    tc_ndx,
    DNX_SAND_IN    uint32    dp_ndx,
    DNX_SAND_IN JER2_ARAD_EGR_QUEUING_MC_COS_MAP    *info
  );

int
  jer2_qax_egr_queuing_if_fc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    JER2_ARAD_INTERFACE_ID    if_ndx,
    DNX_SAND_IN          JER2_ARAD_EGR_QUEUING_IF_FC    *info
  );

int
  jer2_qax_egr_queuing_if_fc_uc_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    uc_if_profile_ndx,
    DNX_SAND_OUT          JER2_ARAD_EGR_QUEUING_IF_UC_FC    *info
  );

int
  jer2_qax_egr_queuing_if_fc_mc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    mc_if_profile_ndx,
    DNX_SAND_IN uint32    pd_th
  );

int
  jer2_qax_egr_queuing_if_fc_mc_get(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     uint32    mc_if_profile_ndx,
    DNX_SAND_OUT uint32   *pd_th
  );

int soc_jer2_qax_egr_congestion_statistics_get(
      DNX_SAND_IN int unit,
      DNX_SAND_IN int core,
      DNX_SAND_INOUT JER2_ARAD_EGR_CGM_CONGENSTION_STATS *cur_stats,   /* place current statistics output here */
      DNX_SAND_INOUT JER2_ARAD_EGR_CGM_CONGENSTION_STATS *max_stats,   /* place maximum statistics output here */
      DNX_SAND_INOUT JER2_ARAD_EGR_CGM_CONGENSTION_COUNTERS *counters, /* place counters output here */
      DNX_SAND_IN int disable_updates /* should the function disable maximum statistics updates when it collects them */
  );
#endif /*__JER2_QAX_EGR_QUEUING_INCLUDED__*/
