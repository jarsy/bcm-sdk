
/* $Id: jer2_arad_fabric.c,v 1.96 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS

/*************
 * INCLUDES  *
 *************/

#include <shared/bsl.h> 
#include <soc/mcm/memregs.h> 
#include <soc/mem.h>
#include <soc/dnx/legacy/JER/jer_api_egr_queuing.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>

/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_sp_tc_drop_set/get
 * TYPE:
 *   PROC
 *    Set the thresholds of a specific service pool, for a certain traffic class.
 * INPUT
 *    DNX_SAND_IN
 *    int tc
 *    DNX_SAND_IN
 *    soc_dnx_cosq_threshold_type_t threshold_type
 *    DNX_SAND_IN(DNX_SAND_OUT in get)
 *    int   threshold_value
 *    DNX_SAND_IN
 *    soc_dnx_cosq_threshold_global_type_t drop_type
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

int
  jer2_jer_egr_queuing_sp_tc_drop_set(
        int    unit,
        int    core,
        int    tc,
        soc_dnx_cosq_threshold_type_t threshold_type,
        int    threshold_value,
        soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    int
      res;

    DNXC_INIT_FUNC_DEFS;

    res = soc_jer2_jer_egr_queuing_sp_tc_drop_set_unsafe(
          unit,
          core,
          tc,
          threshold_type,
          threshold_value,
          drop_type
        );
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_jer_egr_queuing_sp_tc_drop_get(
        int    unit,
        int    core,
        int    tc,
        soc_dnx_cosq_threshold_type_t threshold_type,
        int*   threshold_value,
        soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    int
      res;

    DNXC_INIT_FUNC_DEFS;

    res = soc_jer2_jer_egr_queuing_sp_tc_drop_get_unsafe(
          unit,
          core,
          tc,
          threshold_type,
          threshold_value,
          drop_type
        );
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}


uint32 
  jer2_jer_egr_queuing_sch_unsch_drop_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    threshold_type,
    DNX_SAND_IN    DNX_TMC_EGR_QUEUING_DEV_TH *dev_thresh
  )
{
    uint32 res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = soc_jer2_jer_egr_queuing_sch_unsch_drop_set_unsafe(
          unit,
          core,
          threshold_type,
          dev_thresh
        );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_jer_egr_queuing_sch_unsch_drop_set()", 0, 0);
}

uint32
  jer2_jer_egr_unsched_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  int                 profile,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_DROP_THRESH     *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *exact_thresh
  )
{
    uint32 res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = soc_jer2_jer_egr_unsched_drop_set_unsafe(
       unit,
       core,
       profile,
       prio_ndx,
       dp_ndx,
       thresh,
       exact_thresh);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_jer_egr_unsched_drop_set()", 0, 0);
}

uint32
  jer2_jer_egr_unsched_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *thresh
  )
{
    uint32 res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = soc_jer2_jer_egr_unsched_drop_get_unsafe(
    unit,
    core,
    prio_ndx,
    dp_ndx,
    thresh);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_jer_egr_unsched_drop_get()", 0, 0);
}



uint32
  jer2_jer_egr_sched_port_fc_thresh_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  int                threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH *thresh
  )
{
    uint32 res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = soc_jer2_jer_egr_sched_port_fc_thresh_set_unsafe(
       unit,
       core,
       threshold_type,
       thresh
       );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_jer_egr_sched_port_fc_thresh_set()", 0, 0);
}

uint32
  jer2_jer_egr_sched_q_fc_thresh_set(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  int       core,
    DNX_SAND_IN  int       prio,
    DNX_SAND_IN  int threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH  *thresh
  )
{
    uint32 res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = soc_jer2_jer_egr_sched_q_fc_thresh_set_unsafe(
       unit,
       core,
       prio,
       threshold_type,
       thresh
       );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_jer_egr_sched_q_fc_thresh_set()", 0, 0);
}

int
  jer2_jer_egr_queuing_if_fc_uc_set(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  )
{
    int
      res;

    DNXC_INIT_FUNC_DEFS;

    res = soc_jer2_jer_egr_queuing_if_fc_uc_set_unsafe(
       unit,
       core,
       uc_if_profile_ndx,
       info
       );
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#undef _ERR_MSG_MODULE_NAME
