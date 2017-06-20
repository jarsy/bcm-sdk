/* $Id: jer2_arad_api_egr_queuing.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_JER_API_EGR_QUEUING_INCLUDED__

#define __JER2_JER_API_EGR_QUEUING_INCLUDED__

#include <soc/dnx/legacy/JER/jer_egr_queuing.h>

int
  jer2_jer_egr_queuing_sp_tc_drop_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  );
int
  jer2_jer_egr_queuing_sp_tc_drop_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  );
uint32
  jer2_jer_egr_queuing_sch_unsch_drop_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    threshold_type,
    DNX_SAND_IN    DNX_TMC_EGR_QUEUING_DEV_TH *dev_thresh
  );
uint32
  jer2_jer_egr_unsched_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  int                 profile,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_DROP_THRESH     *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *exact_thresh
  );
uint32
  jer2_jer_egr_sched_port_fc_thresh_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  int                threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH *thresh
  );
uint32
  jer2_jer_egr_sched_q_fc_thresh_set(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  int       core,
    DNX_SAND_IN  int       prio,
    DNX_SAND_IN  int threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH  *thresh
  );

int
  jer2_jer_egr_queuing_if_fc_uc_set(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  );

uint32
  jer2_jer_egr_unsched_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *thresh
  );

#endif
