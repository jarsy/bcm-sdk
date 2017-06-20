#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_end2end_scheduler.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COSQ
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>

#include <soc/dnx/legacy/ARAD/arad_scheduler_end2end.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flows.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_ports.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_elements.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flow_converts.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_device.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>

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
 *     Sets the Independent-Per-Flow Weight configuration mode
 *     (proportional or inverse-proportional). The mode affects
 *     all subsequent configurations of the flow weight in
 *     Independent-Per-Flow mode, as configured by flow_set and
 *     aggregate_set APIs.
 *     Details: in the H file. (search for prototype)
 *********************************************************************/
uint32
  jer2_arad_sch_flow_ipf_config_mode_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE mode
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET);

  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_sch_flow_ipf_config_mode_set_verify(
          unit,
          mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_sch_flow_ipf_config_mode_set_unsafe(
          unit,
          mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_flow_ipf_config_mode_set()", 0, 0);
}


uint32
  jer2_arad_sch_flow_ipf_config_mode_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE *mode
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_GET);

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(mode);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_flow_ipf_config_mode_get_unsafe(
          unit,
          mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_flow_ipf_config_mode_get()", 0, 0);
}


/*********************************************************************
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling, OLP, ERP) its weight index. Range: 0-7. The
*     actual weight value (one of 8, configurable) is in range
*     1-1023, 0 meaning inactive interface. This API is only
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS section
*     below.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_device_if_weight_idx_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_port_t          port,
    DNX_SAND_IN  uint32              weight_index
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DEVICE_IF_WEIGHT_IDX_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */


  res = jer2_arad_sch_device_if_weight_idx_verify(
    unit,
    weight_index
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_device_if_weight_idx_set_unsafe(
    unit,
    port,
    weight_index
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_device_if_weight_idx_set()",0,0);
}

/*********************************************************************
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling, OLP, ERP) its weight index. Range: 0-7. The
*     actual weight value (one of 8, configurable) is in range
*     1-1023, 0 meaning inactive interface. This API is only
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS section
*     below.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_device_if_weight_idx_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_port_t          port,
    DNX_SAND_OUT  uint32             *weight_index
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DEVICE_IF_WEIGHT_IDX_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */


  DNX_SAND_CHECK_NULL_INPUT(weight_index);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_device_if_weight_idx_get_unsafe(
    unit,
    port,
    weight_index
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_device_if_weight_idx_get()",0,0);
}

/*********************************************************************
*     This function sets the device interfaces scheduler
*     weight configuration. Up to 8 weight configuration can
*     be pre-configured. Each scheduler interface will be
*     configured to use one of these pre-configured weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_if_weight_conf_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_IF_WEIGHTS      *if_weights
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_IF_WEIGHT_CONF_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(if_weights);

  res = jer2_arad_sch_if_weight_conf_verify(
    unit,
    if_weights
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_if_weight_conf_set_unsafe(
    unit,
    if_weights
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_if_weight_conf_set()",0,0);
}

/*********************************************************************
*     This function sets the device interfaces scheduler
*     weight configuration. Up to 8 weight configuration can
*     be pre-configured. Each scheduler interface will be
*     configured to use one of these pre-configured weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_if_weight_conf_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_SCH_IF_WEIGHTS      *if_weights
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_IF_WEIGHT_CONF_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(if_weights);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_if_weight_conf_get_unsafe(
    unit,
    if_weights
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_if_weight_conf_get()",0,0);
}

/*********************************************************************
*     Sets a single class type in the table. The driver writes
*     to the following tables: CL-Schedulers Type (SCT)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_TYPE_ID cl_type_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_INFO *exact_class_type
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_CLASS_TYPE_PARAMS_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(class_type);
  DNX_SAND_CHECK_NULL_INPUT(exact_class_type);

  res = jer2_arad_sch_class_type_params_verify(
    unit,
    cl_type_ndx,
    class_type
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_class_type_params_set_unsafe(
    unit, core,
    class_type,
    exact_class_type
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_class_type_params_set()",0,0);
}

/*********************************************************************
*     Sets a single class type in the table. The driver writes
*     to the following tables: CL-Schedulers Type (SCT)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_TYPE_ID cl_type_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type
  )
{
  uint32  res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_CLASS_TYPE_PARAMS_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(class_type);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_class_type_params_get_unsafe(
    unit, core,
    cl_type_ndx,
    class_type
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_class_type_params_get()",0,0);
}

/*********************************************************************
*     Sets the scheduler class type table as a whole.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_table_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_TABLE *exact_sct
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_CLASS_TYPE_PARAMS_TABLE_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(sct);
  DNX_SAND_CHECK_NULL_INPUT(exact_sct);

  res = jer2_arad_sch_class_type_params_table_verify(
    unit,
    sct
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_class_type_params_table_set_unsafe(
    unit,core,
    sct,
    exact_sct
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_class_type_params_table_set()",0,0);
}

/*********************************************************************
*     Sets the scheduler class type table as a whole.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_table_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct
  )
{
  uint32  res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_CLASS_TYPE_PARAMS_TABLE_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(sct);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_class_type_params_table_get_unsafe(
    unit, core,
    sct
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_class_type_params_table_get()",0,0);
}

/*********************************************************************
*     Sets the scheduler-port state (enable/disable), and its
*     HR mode of operation (single or dual). The driver writes
*     to the following tables: Scheduler Enable Memory (SEM),
*     HR-Scheduler-Configuration (SHC), Flow Group Memory
*     (FGM)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_sched_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_SCH_PORT_INFO      *port_info
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_SCHED_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(port_info);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_port_sched_verify(
    unit, core,
    tm_port,
    port_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore); 
  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = jer2_arad_sch_port_sched_set_unsafe(
    unit, core,
    tm_port,
    port_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_sched_set()",0,0);
}

/*********************************************************************
*     Sets the scheduler-port state (enable/disable), and its
*     HR mode of operation (single or dual). The driver writes
*     to the following tables: Scheduler Enable Memory (SEM),
*     HR-Scheduler-Configuration (SHC), Flow Group Memory
*     (FGM)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_sched_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_INFO  *port_info
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_SCHED_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(port_info);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_port_sched_get_unsafe(
    unit,
    core,
    tm_port,
    port_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_sched_get()",0,0);
}
/*********************************************************************
*     This function sets the slow rates. A flow might be in
*     slow state, and in that case lower rate is needed.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_slow_max_rates_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 slow_rate_type,
    DNX_SAND_IN  int                 slow_rate_val
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SLOW_MAX_RATES_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */


  res = jer2_arad_sch_slow_max_rates_verify(
    unit
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_slow_max_rates_set_unsafe(
    unit,
    slow_rate_type,
    slow_rate_val
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_slow_max_rates_set()",0,0);
}

/*********************************************************************
*     This function sets the slow rates. A flow might be in
*     slow state, and in that case lower rate is needed.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_slow_max_rates_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 slow_rate_type,
    DNX_SAND_OUT int      *slow_rate_val
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SLOW_MAX_RATES_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(slow_rate_val);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_slow_max_rates_get_unsafe(
    unit,
    slow_rate_type,
    slow_rate_val
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_slow_max_rates_get()",0,0);
}

/*********************************************************************
*     Sets an aggregate scheduler. It configures an elementary
*     scheduler, and defines a credit flow to this scheduler
*     from a 'father' scheduler. The driver writes to the
*     following tables: Scheduler Enable Memory (SEM),
*     HR-Scheduler-Configuration (SHC), CL-Schedulers
*     Configuration (SCC), Flow Group Memory (FGM) Shaper
*     Descriptor Memory (SHD) Flow Sub-Flow (FSF) Flow
*     Descriptor Memory (FDM) Shaper Descriptor Memory
*     Static(SHDS) Flow Descriptor Memory Static (FDMS)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_aggregate_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO        *se,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW           *exact_flow
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_AGGREGATE_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(se);
  DNX_SAND_CHECK_NULL_INPUT(flow);

  res = jer2_arad_sch_aggregate_verify(
    unit, core,
    se_ndx,
    se,
    flow
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_aggregate_set_unsafe(
    unit,
    core,
    se_ndx,
    se,
    flow
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_aggregate_set()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_aggregate_group_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO        *se,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_AGGREGATE_GROUP_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(se);
  DNX_SAND_CHECK_NULL_INPUT(flow);

  res = jer2_arad_sch_aggregate_verify(
    unit, core,
    se_ndx,
    se,
    flow
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_aggregate_group_set_unsafe(
    unit, core,
    se_ndx,
    se,
    flow
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_aggregate_group_set()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_aggregate_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_INFO        *se,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_AGGREGATE_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_aggregate_get_unsafe(
    unit, core,
    se_ndx,
    se,
    flow
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_aggregate_get()",0,0);
}

/*********************************************************************
*     Configures a flow to a value reserved for 'deleted-flow'
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_delete(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID         flow_ndx
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_DELETE);

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  if (!jer2_arad_is_flow_valid(unit, flow_ndx))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FLOW_ID_OUT_OF_RANGE_ERR, 2, exit)
  }

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_flow_delete_unsafe(
          unit, core,
          flow_ndx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_delete()",0,0);
}

uint32
  jer2_arad_sch_flow_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW           *exact_flow
  )
{
  uint32  res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_flow_set_unsafe(
    unit, core,
    flow_ndx,
    flow
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_set()",flow_ndx,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32  res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(flow);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_flow_get_unsafe(
    unit, core,
    flow_ndx,
    flow
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_get()",0,0);
}

/*********************************************************************
*     Set flow state to off/on. The state of the flow will be
*     updated, unless was configured otherwise. Note: useful
*     for virtual flows, for which the flow state must be
*     explicitly set
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_status_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_STATUS    state
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_STATUS_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */


  res = jer2_arad_sch_flow_status_verify(
    unit,
    flow_ndx,
    state
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_flow_status_set_unsafe(
    unit, core,
    flow_ndx,
    state
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_status_set()",0,0);
}

/*********************************************************************
*     Sets configuration for 1K flows/aggregates (256
*     quartets). Flows interdigitated mode configuration must
*     match the interdigitated mode configurations of the
*     queues they are mapped to. Note1: the following flow
*     configuration is not allowed: interdigitated = TRUE,
*     odd_even = FALSE. The reason for this is that
*     interdigitated configuration defines flow-queue mapping,
*     but a flow with odd_even configuration = FALSE cannot be
*     mapped to a queue. Note2: this configuration is only
*     relevant to flow_id-s in the range 24K - 56K.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_per1k_info_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 k_flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PER1K_INFO_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(per1k_info);

  res = jer2_arad_sch_per1k_info_verify(
    unit,
    k_flow_ndx,
    per1k_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_per1k_info_set_unsafe(
    unit, core,
    k_flow_ndx,
    per1k_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_per1k_info_set()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_per1k_info_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              k_flow_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  )
{
  uint32  res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PER1K_INFO_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_per1k_info_get_unsafe(
    unit, core,
    k_flow_ndx,
    per1k_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_per1k_info_get()",0,0);
}

/*********************************************************************
*     Sets the mapping from flow to queue and to source fap.
*     The configuration is per quartet (up to 4 quartets). The
*     mapping depends on the following parameters: -
*     interdigitated mode - composite mode The driver writes
*     to the following tables: Flow to Queue Mapping (FQM)
*     Flow to FIP Mapping (FFM)
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_to_queue_mapping_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 quartet_ndx,
    DNX_SAND_IN  uint32                 nof_quartets_to_map,
    DNX_SAND_IN  JER2_ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  )
{
  uint32  res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_SET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(quartet_flow_info);

  res = jer2_arad_sch_flow_to_queue_mapping_verify(
    unit, core,
    quartet_ndx,
    nof_quartets_to_map,
    quartet_flow_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_flow_to_queue_mapping_set_unsafe(
    unit, core,
    quartet_ndx,
    nof_quartets_to_map,
    quartet_flow_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_to_queue_mapping_set()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_to_queue_mapping_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 quartet_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  )
{
  uint32  res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_GET);
  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  DNX_SAND_CHECK_NULL_INPUT(quartet_flow_info);
  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_flow_to_queue_mapping_get_unsafe(
    unit, core,
    quartet_ndx,
    quartet_flow_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_to_queue_mapping_get()",0,0);
}

uint32
  jer2_arad_sch_flow_id_verify_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_id
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SIMPLE_FLOW_ID_VERIFY_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(flow_id, JER2_ARAD_SCH_MAX_FLOW_ID, JER2_ARAD_SCH_INVALID_FLOW_ID_ERR,10,exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_id_verify_unsafe()",0,0);
}

uint8
  jer2_arad_sch_is_flow_id_valid(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID       flow_id
  )
{
  uint8
    flow_id_is_valid = FALSE;

  flow_id_is_valid = (flow_id <= JER2_ARAD_SCH_MAX_FLOW_ID)?TRUE:FALSE;

  return flow_id_is_valid;
}

/*********************************************************************
*     Verifies validity of scheduling element id
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_se_id_verify_unsafe(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID        se_id
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SE_ID_VERIFY_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(se_id, JER2_ARAD_SCH_MAX_SE_ID, JER2_ARAD_SCH_INVALID_SE_ID_ERR,10,exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_se_id_verify_unsafe()",0,0);
}

uint8
  jer2_arad_sch_is_se_id_valid(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID        se_id
  )
{
  uint8
    se_id_is_valid = FALSE;

  se_id_is_valid = (se_id <= JER2_ARAD_SCH_MAX_SE_ID)?TRUE:FALSE;

  return se_id_is_valid;
}

/*********************************************************************
*     Calculates se_id given the appropriate flow_id
*     Details: in the H file. (search for prototype)
*********************************************************************/

JER2_ARAD_SCH_SE_ID
  jer2_arad_sch_flow2se_id(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_id
  )
{
  JER2_ARAD_SCH_SE_ID
    base_quartet_flow_id,
    flow_id_in_quartet,
    tmp_flow_id,
    se_id = 0;

  if (DNX_SAND_IS_VAL_OUT_OF_RANGE(
       flow_id, JER2_ARAD_SCH_FLOW_ID_FIXED_TYPE0_END(unit), JER2_ARAD_SCH_MAX_FLOW_ID) )
  {
    se_id = JER2_ARAD_SCH_SE_ID_INVALID;
    goto exit;
  }

  tmp_flow_id = flow_id - JER2_ARAD_SCH_FLOW_BASE_AGGR_FLOW_ID;
  base_quartet_flow_id = JER2_ARAD_SCH_FLOW_BASE_QRTT_ID(tmp_flow_id);
  flow_id_in_quartet = JER2_ARAD_SCH_FLOW_ID_IN_QRTT(tmp_flow_id);

  switch(flow_id_in_quartet)
  {
  case JER2_ARAD_SCH_CL_OFFSET_IN_QUARTET:
    /* This is a CL */
    se_id = JER2_ARAD_CL_SE_ID_MIN + JER2_ARAD_SCH_FLOW_TO_QRTT_ID(base_quartet_flow_id);
    break;
  case JER2_ARAD_SCH_FQ_HR_OFFSET_IN_QUARTET:
    /* This is a FQ/HR */
    se_id = JER2_ARAD_FQ_SE_ID_MIN + JER2_ARAD_SCH_FLOW_TO_QRTT_ID(base_quartet_flow_id);
    break;
  default:
     se_id = JER2_ARAD_SCH_SE_ID_INVALID;
     goto exit;
  }

exit:
  return se_id;
}
/********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
JER2_ARAD_SCH_SE_TYPE
  jer2_arad_sch_se_get_type_by_id(
    JER2_ARAD_SCH_SE_ID                   se_id
  )
{
  JER2_ARAD_SCH_SE_TYPE
    se_type = JER2_ARAD_SCH_SE_TYPE_NONE;

  /* JER2_ARAD_CL_SE_ID_MIN may be changed and be more then 0 */
  /* coverity[unsigned_compare : FALSE] */
  if (DNX_SAND_IS_VAL_IN_RANGE(se_id, JER2_ARAD_CL_SE_ID_MIN, JER2_ARAD_CL_SE_ID_MAX))
  {
    se_type = JER2_ARAD_SCH_SE_TYPE_CL;
  }
  else if (DNX_SAND_IS_VAL_IN_RANGE(se_id, JER2_ARAD_FQ_SE_ID_MIN, JER2_ARAD_FQ_SE_ID_MAX))
  {
    se_type = JER2_ARAD_SCH_SE_TYPE_FQ;
  }
  else if (DNX_SAND_IS_VAL_IN_RANGE(se_id, JER2_ARAD_HR_SE_ID_MIN, JER2_ARAD_HR_SE_ID_MAX))
  {
    se_type = JER2_ARAD_SCH_SE_TYPE_HR;
  }
  else
  {
    se_type = JER2_ARAD_SCH_SE_TYPE_NONE;
  }

  return se_type;
}

/*********************************************************************
*     Calculates flow_id given the appropriate se_id
*     Details: in the H file. (search for prototype)
*********************************************************************/

JER2_ARAD_SCH_FLOW_ID
  jer2_arad_sch_se2flow_id(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_id
  )
{
  JER2_ARAD_SCH_FLOW_ID
    flow_id = 0;

  /* The macro JER2_ARAD_SCH_SE_IS_CL may be used for signed varible also */
  /* coverity[unsigned_compare : FALSE] */
  if (JER2_ARAD_SCH_SE_IS_CL(se_id))
  {
    flow_id = JER2_ARAD_SCH_QRTT_TO_FLOW_ID(se_id - JER2_ARAD_CL_SE_ID_MIN);
    flow_id += JER2_ARAD_SCH_CL_OFFSET_IN_QUARTET;
  }
  else if ((JER2_ARAD_SCH_SE_IS_FQ(se_id)) ||
            (JER2_ARAD_SCH_SE_IS_HR(se_id))
          )
  {
    flow_id = JER2_ARAD_SCH_QRTT_TO_FLOW_ID(se_id - JER2_ARAD_FQ_SE_ID_MIN);
    flow_id += JER2_ARAD_SCH_FQ_HR_OFFSET_IN_QUARTET;
  }
  else
  {
    flow_id = JER2_ARAD_SCH_FLOW_ID_INVALID;
    goto exit;
  }

  flow_id += JER2_ARAD_SCH_FLOW_BASE_AGGR_FLOW_ID;

  if (DNX_SAND_IS_VAL_OUT_OF_RANGE(
       flow_id, JER2_ARAD_SCH_FLOW_BASE_AGGR_FLOW_ID, JER2_ARAD_SCH_MAX_FLOW_ID) )
  {
    flow_id = JER2_ARAD_SCH_FLOW_ID_INVALID;
    goto exit;
  }

exit:
  return flow_id;
}

/*********************************************************************
*     Calculates port id given the appropriate scheduling
*     element id
*     Details: in the H file. (search for prototype)
*********************************************************************/

JER2_ARAD_SCH_PORT_ID
  jer2_arad_sch_se2port_id(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_id
  )
{
  JER2_ARAD_SCH_PORT_ID
    port_id = 0;

  if (DNX_SAND_IS_VAL_OUT_OF_RANGE(
       se_id, JER2_ARAD_HR_SE_ID_MIN, JER2_ARAD_HR_SE_ID_MIN + JER2_ARAD_SCH_MAX_PORT_ID))
  {
    port_id = JER2_ARAD_SCH_PORT_ID_INVALID;
  }
  else
  {
    port_id = se_id - JER2_ARAD_HR_SE_ID_MIN;
  }

  return port_id;
}

/*********************************************************************
*     Calculates port id and TC given the appropriate scheduling
*     element id. 
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_se2port_tc_id(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  int               core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID    se_id,
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_ID *port_id,
    DNX_SAND_OUT uint32           *tc 
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SE2PORT_TC_ID);

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */
  DNX_SAND_CHECK_NULL_INPUT(port_id);
  DNX_SAND_CHECK_NULL_INPUT(tc);

  res = jer2_arad_sch_se_id_verify_unsafe(
          unit,
          se_id
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_se2port_tc_id_get_unsafe(
          unit, core,
          se_id,
          port_id,
          tc
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_se2port_tc_id()", se_id, 0);
}

/*********************************************************************
*     Calculates scheduling element id given the appropriate
*     port id
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_tc2se_id(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  int               core,
    DNX_SAND_IN  uint32            tm_port,
    DNX_SAND_IN  uint32            tc,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_ID    *se_id
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_TC2SE_ID);

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */
  DNX_SAND_CHECK_NULL_INPUT(se_id);

  res = jer2_arad_sch_port_id_verify_unsafe(unit, tm_port);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_sch_port_tc2se_id_get_unsafe(unit, core, tm_port, tc, se_id);
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_port_tc2se_id(). port_id %d, tc 0x%08lX", tm_port, tc);
}
  
/*********************************************************************
*     Sets, for a specified TCG within a certain Port
*     its excess rate. Excess traffic is scheduled between other TCGs
*     according to a weighted fair queueing or strict priority policy. 
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_tcg_weight_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX        tcg_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_TCG_WEIGHT *tcg_weight
  )
{
  uint32
    res = DNX_SAND_OK;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_TCG_WEIGHT_SET);

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */
  DNX_SAND_CHECK_NULL_INPUT(tcg_weight);

   /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */
  
  res = jer2_arad_sch_port_tcg_weight_set_verify_unsafe(
          unit,
          core,
          tm_port,
          tcg_ndx,
          tcg_weight
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);

  res = jer2_arad_sch_port_tcg_weight_set_unsafe(
          unit, core,
          tm_port,
          tcg_ndx,
          tcg_weight
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_port_tcg_weight_set()", tm_port, tcg_ndx);
}

/*********************************************************************
*     Sets, for a specified TCG within a certain Port
*     its excess rate. Excess traffic is scheduled between other TCGs
*     according to a weighted fair queueing or strict priority policy.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_tcg_weight_get(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  int                  core,
    DNX_SAND_IN  uint32               tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX         tcg_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_TCG_WEIGHT  *tcg_weight
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_TCG_WEIGHT_GET);

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */
  DNX_SAND_CHECK_NULL_INPUT(tcg_weight);

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */
  res = jer2_arad_sch_port_tcg_weight_get_verify_unsafe(
          unit,
          core,
          tm_port,
          tcg_ndx,
          tcg_weight
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);

  

    res = jer2_arad_sch_port_tcg_weight_get_unsafe(
          unit, core,
          tm_port,
          tcg_ndx,
          tcg_weight
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_port_tcg_weight_get()", tm_port, tcg_ndx);
}  
  
/*********************************************************************
*     Verifies validity of port id
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_id_verify_unsafe(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  JER2_ARAD_SCH_PORT_ID       port_id
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_ID_VERIFY_UNSAFE);

  if(port_id != JER2_ARAD_ERP_PORT_ID)
  {
    DNX_SAND_ERR_IF_ABOVE_MAX(port_id, JER2_ARAD_SCH_MAX_PORT_ID, JER2_ARAD_SCH_INVALID_PORT_ID_ERR,10,exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_id_verify_unsafe()",0,0);
}

uint8
  jer2_arad_sch_is_port_id_valid(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  JER2_ARAD_SCH_PORT_ID       port_id
  )
{
  uint8
    port_id_is_valid = FALSE;

  port_id_is_valid = (port_id <= JER2_ARAD_SCH_MAX_PORT_ID)?TRUE:FALSE;

  return port_id_is_valid;
}

/*********************************************************************
*     Verifies validity of per_1_k configurations id
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_k_flow_id_verify_unsafe(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  uint32        k_flow_id
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_K_FLOW_ID_VERIFY_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    JER2_ARAD_SCH_1K_TO_FLOW_ID(k_flow_id), JER2_ARAD_SCH_MAX_FLOW_ID,
    JER2_ARAD_SCH_INVALID_K_FLOW_ID_ERR, 10, exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_id_verify_unsafe()",0,0);
}

/*********************************************************************
*     Verifies validity of quartet id
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_quartet_id_verify_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 quartet_id
  )
{
  JER2_ARAD_SCH_FLOW_ID
    flow_id;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_QUARTET_ID_VERIFY_UNSAFE);

  flow_id = JER2_ARAD_SCH_QRTT_TO_FLOW_ID(quartet_id);

  DNX_SAND_ERR_IF_ABOVE_MAX(flow_id, JER2_ARAD_SCH_MAX_FLOW_ID, JER2_ARAD_SCH_INVALID_QUARTET_ID_ERR,10,exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_id_verify_unsafe()",0,0);
}


void
  jer2_arad_JER2_ARAD_SCH_DEVICE_RATE_ENTRY_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_DEVICE_RATE_ENTRY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_DEVICE_RATE_ENTRY_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_DEVICE_RATE_TABLE_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_DEVICE_RATE_TABLE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_DEVICE_RATE_TABLE_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_IF_WEIGHT_ENTRY_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_IF_WEIGHT_ENTRY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_IF_WEIGHT_ENTRY_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_IF_WEIGHTS_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_IF_WEIGHTS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_IF_WEIGHTS_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_PORT_HP_CLASS_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_HP_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_PORT_HP_CLASS_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_PORT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_PORT_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  jer2_arad_JER2_ARAD_SCH_SE_HR_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SE_HR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_HR_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_CL_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_CL_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_FQ_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SE_FQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_FQ_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_CL_CLASS_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_CL_CLASS_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_CL_CLASS_TABLE_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_TABLE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_CL_CLASS_TABLE_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_PER_TYPE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SE_PER_TYPE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_PER_TYPE_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_SHAPER_clear(
    int unit,
    DNX_SAND_OUT JER2_ARAD_SCH_SUB_FLOW_SHAPER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_SHAPER_clear(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_HR_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SUB_FLOW_HR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_HR_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_CL_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SUB_FLOW_CL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_CL_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_SE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SUB_FLOW_SE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_SE_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_CREDIT_SOURCE_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SUB_FLOW_CREDIT_SOURCE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUBFLOW_clear(
    int unit,
    DNX_SAND_OUT JER2_ARAD_SCH_SUBFLOW *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUBFLOW_clear(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_FLOW_clear(
    int unit,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_FLOW_clear(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_GLOBAL_PER1K_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_GLOBAL_PER1K_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_GLOBAL_PER1K_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_QUARTET_MAPPING_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_QUARTET_MAPPING_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_QUARTET_MAPPING_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SLOW_RATE_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_SLOW_RATE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SLOW_RATE_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_FLOW_AND_UP_INFO_clear(
    int unit,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW_AND_UP_INFO *info,
    DNX_SAND_IN uint32                         is_full /*is_full == false --> clear the relevant fields for the next stage algorithm*/
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_FLOW_AND_UP_INFO_clear(unit, info, is_full);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_FLOW_AND_UP_PORT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW_AND_UP_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_FLOW_AND_UP_PORT_INFO_clear(info);

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_FLOW_AND_UP_SE_INFO_clear(
    int unit,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW_AND_UP_SE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_FLOW_AND_UP_SE_INFO_clear(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


#if JER2_ARAD_DEBUG_IS_LVL1


const char*
  jer2_arad_JER2_ARAD_SCH_PORT_LOWEST_HP_HR_CLASS_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_PORT_LOWEST_HP_HR_CLASS enum_val
  )
{
  return DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_CL_CLASS_MODE_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_MODE enum_val
  )
{
  return DNX_TMC_SCH_CL_CLASS_MODE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_CL_CLASS_WEIGHTS_MODE_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_WEIGHTS_MODE enum_val
  )
{
  return DNX_TMC_SCH_CL_CLASS_WEIGHTS_MODE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_CL_ENHANCED_MODE_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_CL_ENHANCED_MODE enum_val
  )
{
  return DNX_TMC_SCH_CL_ENHANCED_MODE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_GROUP_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_GROUP enum_val
  )
{
  return DNX_TMC_SCH_GROUP_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_SE_TYPE_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_TYPE enum_val
  )
{
  return DNX_TMC_SCH_SE_TYPE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_SE_STATE_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_STATE enum_val
  )
{
  return DNX_TMC_SCH_SE_STATE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_SE_HR_MODE_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_HR_MODE enum_val
  )
{
  return DNX_TMC_SCH_SE_HR_MODE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_HR_CLASS_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_SUB_FLOW_HR_CLASS enum_val
  )
{
  return DNX_TMC_SCH_SUB_FLOW_HR_CLASS_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_SUB_FLOW_CL_CLASS enum_val
  )
{
  return DNX_TMC_SCH_SUB_FLOW_CL_CLASS_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_SLOW_RATE_NDX_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_SLOW_RATE_NDX enum_val
  )
{
  return DNX_TMC_SCH_SLOW_RATE_NDX_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_FLOW_TYPE_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_TYPE enum_val
  )
{
  return DNX_TMC_SCH_FLOW_TYPE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_SCH_FLOW_STATUS_to_string(
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_STATUS enum_val
  )
{
  return DNX_TMC_SCH_FLOW_STATUS_to_string(enum_val);
}

void
  jer2_arad_JER2_ARAD_SCH_DEVICE_RATE_ENTRY_print(
    DNX_SAND_IN  JER2_ARAD_SCH_DEVICE_RATE_ENTRY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_DEVICE_RATE_ENTRY_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_DEVICE_RATE_TABLE_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_DEVICE_RATE_TABLE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_DEVICE_RATE_TABLE_print(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_IF_WEIGHT_ENTRY_print(
    DNX_SAND_IN  JER2_ARAD_SCH_IF_WEIGHT_ENTRY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_IF_WEIGHT_ENTRY_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_IF_WEIGHTS_print(
    DNX_SAND_IN  JER2_ARAD_SCH_IF_WEIGHTS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_IF_WEIGHTS_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_PORT_HP_CLASS_INFO_print(
    DNX_SAND_IN  JER2_ARAD_SCH_PORT_HP_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_PORT_HP_CLASS_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* 
 * Copy of TMC print structure except new fields in JER2_ARAD
 */
void
  jer2_arad_JER2_ARAD_SCH_PORT_INFO_print(
    DNX_SAND_IN JER2_ARAD_SCH_PORT_INFO *info,
    DNX_SAND_IN uint32           port_id
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Port[%-2u] - %s, \n\r"),
           port_id,
           info->enable?"enabled":"disabled"
           ));

  for (ind=0; ind<DNX_TMC_NOF_TRAFFIC_CLASSES; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "    HR[%u] mode: %s\n\r"),ind,DNX_TMC_SCH_SE_HR_MODE_to_string(info->hr_modes[ind])));
  }

  if (info->max_expected_rate != DNX_TMC_SCH_PORT_MAX_EXPECTED_RATE_AUTO)
  {
    LOG_CLI((BSL_META_U(unit,
                        ", Max_expected_rate: %u[Mbps]\n\r"),info->max_expected_rate));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "\n\r")));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  jer2_arad_JER2_ARAD_SCH_SE_HR_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_HR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_HR_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_CL_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_CL_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_FQ_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_FQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_FQ_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_CL_CLASS_INFO_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_CL_CLASS_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_CL_CLASS_TABLE_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_TABLE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_CL_CLASS_TABLE_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_PER_TYPE_INFO_print(
    DNX_SAND_IN JER2_ARAD_SCH_SE_PER_TYPE_INFO *info,
    DNX_SAND_IN JER2_ARAD_SCH_SE_TYPE type
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_PER_TYPE_INFO_print(info, type);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SE_INFO_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SE_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_SHAPER_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SUB_FLOW_SHAPER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_SHAPER_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_HR_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SUB_FLOW_HR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_HR_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_CL_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SUB_FLOW_CL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_CL_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_FQ_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SUB_FLOW_FQ *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_FQ_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_SE_INFO_print(
    DNX_SAND_IN JER2_ARAD_SCH_SUB_FLOW_SE_INFO *info,
    DNX_SAND_IN JER2_ARAD_SCH_SE_TYPE se_type
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_SE_INFO_print(info, se_type);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUB_FLOW_CREDIT_SOURCE_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SUB_FLOW_CREDIT_SOURCE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SUBFLOW_print(
    DNX_SAND_IN JER2_ARAD_SCH_SUBFLOW *info,
    DNX_SAND_IN uint8 is_table_flow,
    DNX_SAND_IN uint32 subflow_id
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SUBFLOW_print(info, is_table_flow, subflow_id);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_FLOW_print(
    DNX_SAND_IN JER2_ARAD_SCH_FLOW *info,
    DNX_SAND_IN uint8 is_table
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_FLOW_print(info, is_table);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_GLOBAL_PER1K_INFO_print(
    DNX_SAND_IN  JER2_ARAD_SCH_GLOBAL_PER1K_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_GLOBAL_PER1K_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_QUARTET_MAPPING_INFO_print(
    DNX_SAND_IN  JER2_ARAD_SCH_QUARTET_MAPPING_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_QUARTET_MAPPING_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_SLOW_RATE_print(
    DNX_SAND_IN  JER2_ARAD_SCH_SLOW_RATE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SCH_SLOW_RATE_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32
  jer2_arad_flow_and_up_info_get(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_IN     int                          core,
    DNX_SAND_IN     uint32                          flow_id,
    DNX_SAND_IN     uint32                          reterive_status,
    DNX_SAND_INOUT  JER2_ARAD_SCH_FLOW_AND_UP_INFO    *flow_and_up_info
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  /* DNX_SAND_CHECK_DRIVER_AND_DEVICE; */

  /* DNX_SAND_TAKE_DEVICE_SEMAPHORE; */

  res = jer2_arad_flow_and_up_info_get_unsafe(
          unit,
          core,
          flow_id,
          reterive_status,
          flow_and_up_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);

  JER2_ARAD_DO_NOTHING_AND_EXIT;
exit_semaphore:
  /* DNX_SAND_GIVE_DEVICE_SEMAPHORE; */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_flow_and_up_print()", 0, 0);

}

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */
