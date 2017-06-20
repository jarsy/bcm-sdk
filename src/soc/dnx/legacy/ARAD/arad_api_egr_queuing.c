#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_egr_queuing.c,v 1.16 Broadcom SDK $
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
/* { */
#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_egr_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */
/*********************************************************************
*     Sets Outgoing FAP Port (OFP) threshold type, per port.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_ofp_thresh_type_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID    tm_port,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_thresh_type
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_OFP_THRESH_TYPE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;


  res = jer2_arad_egr_ofp_thresh_type_verify(
    unit,
    tm_port,
    ofp_thresh_type
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_ofp_thresh_type_set_unsafe(
    unit,
	core_id,
    tm_port,
    ofp_thresh_type
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_ofp_thresh_type_set()",0,0);
}

/*********************************************************************
*     Sets Outgoing FAP Port (OFP) threshold type, per port.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_ofp_thresh_type_get(
    DNX_SAND_IN  int                 unit,
	DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID         ofp_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_PORT_THRESH_TYPE *ofp_thresh_type
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_OFP_THRESH_TYPE_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(ofp_thresh_type);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_ofp_thresh_type_get_unsafe(
    unit,
	core_id,
    ofp_ndx,
    ofp_thresh_type
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_ofp_thresh_type_get()",0,0);
}

/*********************************************************************
*     Set scheduled drop thresholds for egress queues per
*     queue-priority.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_sched_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  int                 profile,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_DROP_THRESH     *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *exact_thresh
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_SCHED_DROP_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(thresh);
  DNX_SAND_CHECK_NULL_INPUT(exact_thresh);

  res = jer2_arad_egr_sched_drop_verify(
    unit,
    prio_ndx,
    thresh
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_sched_drop_set_unsafe(
    unit,
    core,
    profile,
    prio_ndx,
    thresh,
    exact_thresh
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_sched_drop_set()",0,0);
}

/*********************************************************************
*     Set scheduled drop thresholds for egress queues per
*     queue-priority.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_sched_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *thresh
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_SCHED_DROP_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(thresh);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_sched_drop_get_unsafe(
    unit,
    prio_ndx,
    thresh
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_sched_drop_get()",0,0);
}

/*********************************************************************
*     Set unscheduled drop thresholds for egress queues, per
*     queue-priority and drop precedence.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_unsched_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  int                 profile,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_DROP_THRESH     *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *exact_thresh
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_UNSCHED_DROP_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(thresh);
  DNX_SAND_CHECK_NULL_INPUT(exact_thresh);

  res = jer2_arad_egr_unsched_drop_verify(
    unit,
    prio_ndx,
    dp_ndx,
    thresh
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_unsched_drop_set_unsafe(
    unit,
    core,
    profile,
    prio_ndx,
    dp_ndx,
    thresh,
    exact_thresh
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_unsched_drop_set()",0,0);
}

uint32
  jer2_arad_egr_sched_port_fc_thresh_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  int                threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH *thresh
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_SCHED_PORT_FC_THRESH_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(thresh);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_sched_port_fc_thresh_set_unsafe(
    unit,
    core,
    threshold_type,
    thresh
  );

  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_sched_port_fc_thresh_set()",0,0);

}

uint32
  jer2_arad_egr_sched_q_fc_thresh_set(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  int       core,
    DNX_SAND_IN  int       prio,
    DNX_SAND_IN  int threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH  *thresh
  )
{
  uint32 res;
  JER2_ARAD_EGR_THRESH_INFO thresh_info;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_SCHED_Q_FC_THRESH_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(thresh);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  thresh_info.dbuff = thresh->data_buffers;/* queue_words_consumed parameter used for Data buffers in Arad */
  thresh_info.packet_descriptors = thresh->packet_descriptors;

  res = jer2_arad_egr_ofp_fc_q_pair_thresh_set_unsafe(
               unit,
               core,
               prio,
               threshold_type,
               &thresh_info);

  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);


exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_sched_q_fc_thresh_set()",0,0);
  
}

/*********************************************************************
*     Set unscheduled drop thresholds for egress queues, per
*     queue-priority and drop precedence.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_unsched_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *thresh
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_UNSCHED_DROP_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(thresh);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_unsched_drop_get_unsafe(
    unit,
    prio_ndx,
    dp_ndx,
    thresh
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_unsched_drop_get()",0,0);
}

/*********************************************************************
*     Set Flow Control thresholds for egress queues, based on
*     device-level resources. Threshold are set for overall
*     resources, and scheduled resources.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_egr_dev_fc_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_FC_DEVICE_THRESH *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEVICE_THRESH *exact_thresh
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh);
  DNXC_NULL_CHECK(exact_thresh);

  res = jer2_arad_egr_dev_fc_verify(
    unit,
    thresh
  );
  DNXC_SAND_IF_ERR_EXIT(res);

  res = jer2_arad_egr_dev_fc_set_unsafe(
    unit,
    core,
    thresh,
    exact_thresh
  );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set Flow Control thresholds for egress queues, based on
*     device-level resources. Threshold are set for overall
*     resources, and scheduled resources.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_egr_dev_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEVICE_THRESH *thresh
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh);

  res = jer2_arad_egr_dev_fc_get_unsafe(
    unit,
    core,
    thresh
  );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}
/*********************************************************************
*     Set Flow Control thresholds for egress queues, per port
*     queue priority and threshold type, based on Outgoing FAP
*     Port (OFP) resources.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_egr_ofp_fc_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_FC_OFP_THRESH   *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH   *exact_thresh
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh);
  DNXC_NULL_CHECK(exact_thresh);

  res = jer2_arad_egr_ofp_fc_verify(
    unit,
    prio_ndx,
    ofp_type_ndx,
    thresh
  );
  DNXC_SAND_IF_ERR_EXIT(res);

  res = jer2_arad_egr_ofp_fc_set_unsafe(
    unit,
    core,
    prio_ndx,
    ofp_type_ndx,
    thresh,
    exact_thresh
  );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set Flow Control thresholds for egress queues, per port
*     queue priority and threshold type, based on Outgoing FAP
*     Port (OFP) resources.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_egr_ofp_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH   *thresh
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;


  DNXC_NULL_CHECK(thresh);

  res = jer2_arad_egr_ofp_fc_get_unsafe(
    unit,
    core,
    prio_ndx,
    ofp_type_ndx,
    thresh
  );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set per-port egress scheduling information.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_ofp_scheduling_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_EGR_OFP_SCH_INFO    *info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_OFP_SCHEDULING_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(info);

  res = jer2_arad_egr_ofp_scheduling_verify(
    unit,
    tm_port,
    info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_ofp_scheduling_set_unsafe(
    unit,
    core,
    tm_port,
    info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_ofp_scheduling_set()",0,0);
}

/*********************************************************************
*     Set per-port egress scheduling information.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_ofp_scheduling_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_OUT JER2_ARAD_EGR_OFP_SCH_INFO    *info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_OFP_SCHEDULING_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_ofp_scheduling_get_unsafe(
    unit,
    core,
    tm_port,
    info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_ofp_scheduling_get()",0,0);
}

/*********************************************************************
*     Function description
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_dsp_pp_to_base_q_pair_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  int                      core,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID         ofp_ndx,
    DNX_SAND_OUT uint32                   *base_q_pair
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_DSP_PP_TO_BASE_Q_PAIR_GET);
  DNX_SAND_CHECK_NULL_INPUT(base_q_pair);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_egr_dsp_pp_to_base_q_pair_get_verify(
          unit,
          core,
          ofp_ndx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_dsp_pp_to_base_q_pair_get_unsafe(
          unit, 
          core,
          ofp_ndx,
          base_q_pair
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egr_dsp_pp_to_base_q_pair_get()", ofp_ndx, 0);
}

/*********************************************************************
*     Function description
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_dsp_pp_priorities_mode_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID               ofp_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_PORT_PRIORITY_MODE    *priority_mode
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_DSP_PP_PRIORITIES_MODE_GET);
  DNX_SAND_CHECK_NULL_INPUT(priority_mode);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_egr_dsp_pp_priorities_mode_get_verify(
          unit,
          ofp_ndx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_dsp_pp_priorities_mode_get_unsafe(
          unit,
          ofp_ndx,
          priority_mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egr_dsp_pp_priorities_mode_get()", ofp_ndx, 0);
}

/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_dev_set/get
 * TYPE:
 *   PROC
 *    Set the thresholds of the Multicast / Unicast service pools at device-level.
 * INPUT
 *    DNX_SAND_IN(DNX_SAND_OUT in get)
 *    JER2_ARAD_EGR_QUEUING_DEV_TH    *info -
 *        Service pool parameters
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_egr_queuing_dev_set(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core,
      DNX_SAND_IN  JER2_ARAD_EGR_QUEUING_DEV_TH  *info
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_dev_set_unsafe(
          unit,
          core,
          info
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


int
  jer2_arad_egr_queuing_dev_get(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     int    core,
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_DEV_TH    *info
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_dev_get_unsafe(
          unit,
          core,
          info
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_global_drop_set/get
 * TYPE:
 *   PROC
 *    Set the thresholds of global drop.
 * INPUT
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
  jer2_arad_egr_queuing_global_drop_set(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_IN  int    threshold_value, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_global_drop_set_unsafe(
          unit,
          core,
          threshold_type,
          threshold_value,
          drop_type
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_arad_egr_queuing_global_drop_get(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_OUT int*    threshold_value, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_global_drop_get_unsafe(
          unit,
          core,
          threshold_type,
          threshold_value,
          drop_type
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_global_fc_set/get
 * TYPE:
 *   PROC
 *    Set the thresholds of global flow control.
 * INPUT
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
  jer2_arad_egr_queuing_global_fc_set(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_IN  int    threshold_value, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_global_fc_set_unsafe(
          unit,
          core,
          threshold_type,
          threshold_value,
          drop_type
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_arad_egr_queuing_global_fc_get(
      DNX_SAND_IN  int   unit,
      DNX_SAND_IN  int   core, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_type_t threshold_type,
      DNX_SAND_OUT int*    threshold_value, 
      DNX_SAND_IN  soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_global_fc_get_unsafe(
          unit,
          core,
          threshold_type,
          threshold_value,
          drop_type
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_mc_tc_fc_set/get
 * TYPE:
 *   PROC
 *    Set the thresholds of multicast with specific tc flow control threshold.
 * INPUT
 *    DNX_SAND IN
 *    int tc
 *    DNX_SAND_IN
 *    soc_dnx_cosq_threshold_type_t threshold_type
 *    DNX_SAND_IN(DNX_SAND_OUT in get)
 *    int   threshold_value
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_egr_queuing_mc_tc_fc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_mc_tc_fc_set_unsafe(
          unit,
          core,
          tc,
          threshold_type,
          threshold_value
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_arad_egr_queuing_mc_tc_fc_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*    threshold_value
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_mc_tc_fc_get_unsafe(
          unit,
          core,
          tc,
          threshold_type,
          threshold_value
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}



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
  jer2_arad_egr_queuing_sp_tc_drop_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

    res = jer2_arad_egr_queuing_sp_tc_drop_set_unsafe(
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
  jer2_arad_egr_queuing_sp_tc_drop_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

    res = jer2_arad_egr_queuing_sp_tc_drop_get_unsafe(
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

/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_sp_reserved_set/get
 * TYPE:
 *   PROC
 *    Set the reserved resources for a specific spXtc.
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
  jer2_arad_egr_queuing_sp_reserved_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

    res = jer2_arad_egr_queuing_sp_reserved_set_unsafe(
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
  jer2_arad_egr_queuing_sp_reserved_get(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*    threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

    res = jer2_arad_egr_queuing_sp_reserved_get_unsafe(
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


/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_sch_unsch_drop_set/get
 * TYPE:
 *   PROC
 *    Set the thresholds of a sched/unsched with no traffic class.
 * INPUT
 *    DNX_SAND_IN
 *    int core
 *    DNX_SAND_IN
 *    int threshold_type
 *    DNX_SAND_IN(DNX_SAND_OUT in get)
 *    int   threshold_value
 *    DNX_SAND_IN
 *    DNX_TMC_EGR_QUEUING_DEV_TH dev_thresh
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
  jer2_arad_egr_queuing_sch_unsch_drop_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    threshold_type,
    DNX_SAND_IN    DNX_TMC_EGR_QUEUING_DEV_TH *dev_thresh
  )
{
    uint32 res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_SCH_UNSCH_DROP_SET);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;

    res = jer2_arad_egr_queuing_sch_unsch_drop_set_unsafe(
          unit,
          core,
          threshold_type,
          dev_thresh
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);
exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egr_queuing_sch_unsch_drop_set()", 0, 0);
}

/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_mc_cos_map_set/get
 * TYPE:
 *   PROC
 *    Set the COS mapping for egress multicast packets: TC-Group, Service Pool id and Service Pool eligibility.
 * INPUT
 *    DNX_SAND_IN    uint32    tc_ndx -
 *        Traffic Class. Range: 0 - 7.
 *    DNX_SAND_IN    uint32    dp_ndx -
 *        Drop Precedence. Range: 0 - 3.
 *    DNX_SAND_IN    (DNX_SAND_OUT in get) JER2_ARAD_EGR_QUEUING_MC_COS_MAP    *info -
 *        COS mapping parameters
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

int
  jer2_arad_egr_queuing_mc_cos_map_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    tc_ndx,
    DNX_SAND_IN    uint32    dp_ndx,
    DNX_SAND_IN JER2_ARAD_EGR_QUEUING_MC_COS_MAP    *info
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_mc_cos_map_set_unsafe(
          unit,
          core,
          tc_ndx,
          dp_ndx,
          info
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


uint32    
  jer2_arad_egr_queuing_mc_cos_map_get(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     int    core,
    DNX_SAND_IN     uint32    tc_ndx,
    DNX_SAND_IN     uint32    dp_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_MC_COS_MAP    *info
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_MC_COS_MAP_GET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_queuing_mc_cos_map_get_unsafe(
          unit,
          core,
          tc_ndx,
          dp_ndx,
          info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_jer2_arad_egr_queuing_mc_cos_map_get()", 0, 0);
}


/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_if_fc_set/get
 * TYPE:
 *   PROC
 *    Set the Interface Flow Control profiles for UC and MC.
 * INPUT
 *    DNX_SAND_IN    JER2_ARAD_INTERFACE_ID    if_ndx -
 *        Interface ID.
 *    DNX_SAND_IN    (DNX_SAND_OUT in get) JER2_ARAD_EGR_QUEUING_IF_FC    *info -
 *        Interface Flow Control profiles
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_egr_queuing_if_fc_set(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     JER2_ARAD_INTERFACE_ID    if_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_QUEUING_IF_FC    *info
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_if_fc_set_unsafe(
          unit,
          if_ndx,
          info
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


uint32
  jer2_arad_egr_queuing_if_fc_get(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     JER2_ARAD_INTERFACE_ID    if_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_IF_FC    *info
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_IF_FC_GET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_queuing_if_fc_get_unsafe(
          unit,
          if_ndx,
          info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_jer2_arad_egr_queuing_if_fc_get()", 0, 0);
}

/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_if_fc_uc_max_set
 * TYPE:
 *   PROC
 *    Set the Interface Flow Control profile attributes for UC traffic.
 * INPUT
 *    DNX_SAND_IN    uint32    uc_if_profile_ndx-
 *        Unicast interface threshold profile.
 *    DNX_SAND_IN    (DNX_SAND_OUT for get) JER2_ARAD_EGR_QUEUING_IF_UC_FC    *info -
 *        Interface Flow Control profile attributes
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

int
  jer2_arad_egr_queuing_if_fc_uc_max_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    uc_if_profile_ndx,
    DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC    *info
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_if_fc_uc_set_unsafe(
          unit,
          core,
          uc_if_profile_ndx,
          info
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_if_fc_uc_set/get
 * TYPE:
 *   PROC
 *    Set the Interface Flow Control profile attributes for UC traffic.
 * INPUT
 *    DNX_SAND_IN    uint32    uc_if_profile_ndx-
 *        Unicast interface threshold profile.
 *    DNX_SAND_IN    (DNX_SAND_OUT for get) JER2_ARAD_EGR_QUEUING_IF_UC_FC    *info -
 *        Interface Flow Control profile attributes
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

int
  jer2_arad_egr_queuing_if_fc_uc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    uc_if_profile_ndx,
    DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC    *info
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_if_fc_uc_set_unsafe(
          unit,
          core,
          uc_if_profile_ndx,
          info
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


int
  jer2_arad_egr_queuing_if_fc_uc_get(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     int    core,
    DNX_SAND_IN     uint32    uc_if_profile_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_IF_UC_FC    *info
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_if_fc_uc_get_unsafe(
          unit,
          core,
          uc_if_profile_ndx,
          info
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


/*********************************************************************
* NAME:
 *   jer2_arad_egr_queuing_if_fc_mc_set/get
 * TYPE:
 *   PROC
 *    Set the Interface Flow Control profile attributes for MC traffic.
 * INPUT
 *    DNX_SAND_IN    uint32    mc_if_profile_ndx-
 *        Unicast interface threshold profile.
 *    DNX_SAND_IN    (DNX_SAND_OUT for get) uint32    pd_th -
 * Total consumed Multicast PD per interface threshold.
 * Range: 0 - 0x7FFF.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_egr_queuing_if_fc_mc_set(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    uint32    mc_if_profile_ndx,
    DNX_SAND_IN uint32    pd_th
  )
{

  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_if_fc_mc_set_unsafe(
          unit,
          core,
          mc_if_profile_ndx,
          pd_th
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}


int
  jer2_arad_egr_queuing_if_fc_mc_get(
    DNX_SAND_IN     int    unit,
    DNX_SAND_IN     uint32    mc_if_profile_ndx,
    DNX_SAND_OUT uint32   *pd_th
  )
{
  int
    res;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_egr_queuing_if_fc_mc_get_unsafe(
          unit,
          mc_if_profile_ndx,
          pd_th
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

uint32 
  jer2_arad_egr_queuing_if_uc_map_set(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  int        core,
    DNX_SAND_IN  soc_port_if_t interface_type,
    DNX_SAND_IN  uint32     internal_if_id,
    DNX_SAND_IN  int        profile
  )
{
    uint32
        res = DNX_SAND_OK;
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_IF_UC_MAP_SET);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;
    res = jer2_arad_egr_queuing_if_uc_map_set_unsafe(
          unit,
          core,
          interface_type,
          internal_if_id,
          profile
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egr_queuing_if_uc_map_set()", 0, 0);

}

uint32 
  jer2_arad_egr_queuing_if_mc_map_set(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  int        core,
    DNX_SAND_IN  soc_port_if_t interface_type,
    DNX_SAND_IN  uint32     internal_if_id,
    DNX_SAND_IN  int        profile
  )
{
    uint32
        res = DNX_SAND_OK;
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_IF_MC_MAP_SET);

    DNX_SAND_CHECK_DRIVER_AND_DEVICE;

    DNX_SAND_TAKE_DEVICE_SEMAPHORE;
    res = jer2_arad_egr_queuing_if_mc_map_set_unsafe(
          unit,
          core,
          interface_type,
          internal_if_id,
          profile
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egr_queuing_if_mc_map_set()", 0, 0);

}

/*********************************************************************
*     Associate the queue-pair (Port,Priority) to traffic class
*    groups (TCG) attributes.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_queuing_ofp_tcg_set(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  int                            core,
    DNX_SAND_IN  uint32                         tm_port,    
    DNX_SAND_IN  JER2_ARAD_EGR_QUEUING_TCG_INFO      *tcg_info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_OFP_TCG_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(tcg_info);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_queuing_ofp_tcg_set_verify(
    unit,
    core,
    tm_port,    
    tcg_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore); 

  res = jer2_arad_egr_queuing_ofp_tcg_set_unsafe(
    unit,
    core,
    tm_port,    
    tcg_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_queuing_ofp_tcg_set()",tm_port,0);
}

/*********************************************************************
 *    Associate the queue-pair (Port,Priority) to traffic class
 *    groups (TCG) attributes.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_queuing_ofp_tcg_get(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  int                            core,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID               tm_port,    
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_TCG_INFO      *tcg_info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_OFP_TCG_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(tcg_info);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_queuing_ofp_tcg_get_unsafe(
    unit,
    core,
    tm_port,    
    tcg_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_queuing_ofp_tcg_get()",tm_port,0);
}

/*********************************************************************
*     Sets, for a specified TCG within a certain OFP
*     its excess rate. Excess traffic is scheduled between other TCGs
*     according to a weighted fair queueing or strict priority policy.
*     Set invalid, in case TCG not take part of this policy.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_queuing_tcg_weight_set(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID          tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX              tcg_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_TCG_SCH_WFQ      *tcg_weight
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(tcg_weight);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_queuing_tcg_weight_set_verify_unsafe(
    unit,
    core,
    tm_port,
    tcg_ndx,
    tcg_weight
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);

  res = jer2_arad_egr_queuing_tcg_weight_set_unsafe(
    unit,
    core,
    tm_port,
    tcg_ndx,
    tcg_weight
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_queuing_tcg_weight_set()",tm_port,tcg_ndx);
}

/*********************************************************************
 *     Sets, for a specified TCG within a certain OFP
*     its excess rate. Excess traffic is scheduled between other TCGs
*     according to a weighted fair queueing or strict priority policy.
*     Set invalid, in case TCG not take part of this policy.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_egr_queuing_tcg_weight_get(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  uint32                    tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX              tcg_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_TCG_SCH_WFQ      *tcg_weight
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(tcg_weight);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_egr_queuing_tcg_weight_get_verify_unsafe(
    unit,
    core,
    tm_port,
    tcg_ndx,
    tcg_weight
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore); 

  res = jer2_arad_egr_queuing_tcg_weight_get_unsafe(
    unit,
    core,
    tm_port,
    tcg_ndx,
    tcg_weight
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_queuing_tcg_weight_get()",tm_port,tcg_ndx);
}

void
  jer2_arad_JER2_ARAD_EGR_Q_PRIORITY_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_Q_PRIORITY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_EGR_Q_PRIORITY));
  info->tc = 0;
  info->dp = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_DROP_THRESH_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_DROP_THRESH_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_THRESH_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_THRESH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_THRESH_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_QUEUING_TH_DB_GLOBAL_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_TH_DB_GLOBAL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_TH_DB_GLOBAL_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_QUEUING_TH_DB_POOL_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_TH_DB_POOL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_TH_DB_POOL_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_QUEUING_TH_DB_PORT_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_TH_DB_PORT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_TH_DB_PORT_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_QUEUING_DEV_TH_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_DEV_TH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_DEV_TH_clear(info);

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_QUEUING_MC_COS_MAP_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_MC_COS_MAP *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_MC_COS_MAP_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_QUEUING_IF_FC_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_IF_FC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_IF_FC_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_QUEUING_IF_UC_FC_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_IF_UC_FC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_IF_UC_FC_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_DEV_THRESH_INNER_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEV_THRESH_INNER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_DEV_THRESH_INNER_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_DEVICE_THRESH_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_FC_DEVICE_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_DEVICE_THRESH_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_CHNIF_THRESH_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_FC_CHNIF_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_CHNIF_THRESH_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_OFP_THRESH_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_OFP_THRESH_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_OFP_SCH_WFQ_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_OFP_SCH_WFQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_OFP_SCH_WFQ_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_OFP_SCH_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_OFP_SCH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_OFP_SCH_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  JER2_ARAD_EGR_QUEUING_TCG_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_TCG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_TCG_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  JER2_ARAD_EGR_TCG_SCH_WFQ_clear(
    DNX_SAND_OUT JER2_ARAD_EGR_TCG_SCH_WFQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_TCG_SCH_WFQ_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if JER2_ARAD_DEBUG_IS_LVL1

void
  JER2_ARAD_EGR_Q_PRIORITY_print(
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIORITY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "tc: %u\n\r"),info->tc));
  LOG_CLI((BSL_META_U(unit,
                      "dp: %u\n\r"),info->dp));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

const char*
  jer2_arad_JER2_ARAD_EGR_Q_PRIO_to_string(
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO enum_val
  )
{
  return DNX_TMC_EGR_Q_PRIO_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_EGR_PORT_THRESH_TYPE_to_string(
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE enum_val
  )
{
  return DNX_TMC_EGR_PORT_THRESH_TYPE_to_string(enum_val);
}


const char*
  jer2_arad_JER2_ARAD_EGR_OFP_INTERFACE_PRIO_to_string(
    DNX_SAND_IN  JER2_ARAD_EGR_OFP_INTERFACE_PRIO enum_val
  )
{
  return DNX_TMC_EGR_OFP_INTERFACE_PRIO_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_EGR_Q_PRIO_MAPPING_TYPE_to_string(
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO_MAPPING_TYPE enum_val
  )
{
  return DNX_TMC_EGR_Q_PRIO_MAPPING_TYPE_to_string(enum_val);
}

void
  jer2_arad_JER2_ARAD_EGR_DROP_THRESH_print(
    DNX_SAND_IN  JER2_ARAD_EGR_DROP_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_DROP_THRESH_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_THRESH_INFO_print(
    DNX_SAND_IN  JER2_ARAD_EGR_THRESH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_THRESH_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_DEV_THRESH_INNER_print(
    DNX_SAND_IN  JER2_ARAD_EGR_FC_DEV_THRESH_INNER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_DEV_THRESH_INNER_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_DEVICE_THRESH_print(
    DNX_SAND_IN  JER2_ARAD_EGR_FC_DEVICE_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_DEVICE_THRESH_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_CHNIF_THRESH_print(
    DNX_SAND_IN  JER2_ARAD_EGR_FC_CHNIF_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_CHNIF_THRESH_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_FC_OFP_THRESH_print(
    DNX_SAND_IN  JER2_ARAD_EGR_FC_OFP_THRESH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_FC_OFP_THRESH_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_OFP_SCH_WFQ_print(
    DNX_SAND_IN  JER2_ARAD_EGR_OFP_SCH_WFQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_OFP_SCH_WFQ_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_EGR_OFP_SCH_INFO_print(
    DNX_SAND_IN  JER2_ARAD_EGR_OFP_SCH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_OFP_SCH_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
void
  JER2_ARAD_EGR_QUEUING_TCG_INFO_print(
    DNX_SAND_IN  JER2_ARAD_EGR_QUEUING_TCG_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_QUEUING_TCG_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  JER2_ARAD_EGR_TCG_SCH_WFQ_print(
    DNX_SAND_IN  JER2_ARAD_EGR_TCG_SCH_WFQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_EGR_TCG_SCH_WFQ_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/* } */
#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */

