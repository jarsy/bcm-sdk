#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_scheduler_flows.c,v 1.10 Broadcom SDK $
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

#include <soc/dnx/legacy/ARAD/arad_scheduler_flows.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flow_converts.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_end2end.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_elements.h>

#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/mbcm.h>
#include <shared/swstate/access/sw_state_access.h>

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
  jer2_arad_sch_flow_ipf_config_mode_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE mode
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET_UNSAFE);
  
  DNX_SAND_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.ipf_config_mode.set(unit, mode));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_flow_ipf_config_mode_set_unsafe()", 0, 0);
}


uint32
  jer2_arad_sch_flow_ipf_config_mode_set_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE mode
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(mode, JER2_ARAD_SCH_NOF_FLOW_IPF_CONFIG_MODES, JER2_ARAD_END2END_SCHEDULER_MODE_OUT_OF_RANGE_ERR, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_flow_ipf_config_mode_set_verify()", 0, 0);
}

/*********************************************************************
 *     Sets the Independent-Per-Flow Weight configuration mode
 *     (proportional or inverse-proportional). The mode affects
 *     all subsequent configurations of the flow weight in
 *     Independent-Per-Flow mode, as configured by flow_set and
 *     aggregate_set APIs.
 *     Details: in the H file. (search for prototype)
 *********************************************************************/
uint32
  jer2_arad_sch_flow_ipf_config_mode_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE *mode
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_GET_UNSAFE);
    
  DNX_SAND_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.ipf_config_mode.get(unit, mode));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_flow_ipf_config_mode_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_nof_subflows_get(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     int                   core,
    DNX_SAND_IN     JER2_ARAD_SCH_FLOW_ID          base_flow_id,
    DNX_SAND_OUT    uint32                   *nof_subflows
  )
{
  uint32
    offset = 0,
    idx = 0,
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_FSF_TBL_DATA
    fsf_tbl_data;
  JER2_ARAD_SCH_GLOBAL_PER1K_INFO
    global_per1k_info;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_NOF_SUBFLOWS_GET);

  res = jer2_arad_sch_per1k_info_get_unsafe(
          unit, core,
          JER2_ARAD_SCH_FLOW_TO_1K_ID(base_flow_id),
          &global_per1k_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  offset  = JER2_ARAD_REG_IDX_GET(base_flow_id, DNX_SAND_REG_SIZE_BITS);

  idx = JER2_ARAD_SCH_SUB_FLOW_BASE_FLOW(base_flow_id, global_per1k_info.is_odd_even);
  idx = JER2_ARAD_FLD_IDX_GET(idx, DNX_SAND_REG_SIZE_BITS);
  idx = idx / 2 + idx % 2;

  res = jer2_arad_sch_fsf_tbl_get_unsafe(
          unit, core,
          offset,
          &fsf_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  *nof_subflows = DNX_SAND_GET_BIT(fsf_tbl_data.sfenable,idx)? 2 : 1;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_nof_subflows_get()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_nof_subflows_set(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     int                   core,
    DNX_SAND_IN     JER2_ARAD_SCH_FLOW_ID           base_flow_id,
    DNX_SAND_IN     uint32                    nof_subflows,
    DNX_SAND_IN     uint8                   is_odd_even
  )
{
  uint32
    offset = 0,
    idx = 0,
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_FSF_TBL_DATA
    fsf_tbl_data;
  uint32
    bit_val_to_set = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_NOF_SUBFLOWS_SET);

  offset  = JER2_ARAD_REG_IDX_GET(base_flow_id, DNX_SAND_REG_SIZE_BITS);

  idx = JER2_ARAD_SCH_SUB_FLOW_BASE_FLOW(base_flow_id, is_odd_even);
  idx = JER2_ARAD_FLD_IDX_GET(idx, DNX_SAND_REG_SIZE_BITS);
  idx = (idx/2) + (idx%2);

  res = jer2_arad_sch_fsf_tbl_get_unsafe(
          unit, core,
          offset,
          &fsf_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  bit_val_to_set = (nof_subflows > 1) ? 0x1 : 0x0;

  if (DNX_SAND_GET_BIT(fsf_tbl_data.sfenable,idx) != bit_val_to_set)
  {
     DNX_SAND_SET_BIT(fsf_tbl_data.sfenable,bit_val_to_set,idx);
     res = jer2_arad_sch_fsf_tbl_set_unsafe(
             unit, core,
             offset,
             &fsf_tbl_data
           );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_nof_subflows_set()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_slow_enable_set(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     int                   core,
    DNX_SAND_IN     JER2_ARAD_SCH_FLOW_ID           flow_ndx,
    DNX_SAND_IN     uint8                   is_slow_enabled
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    value,
    offset = 0,
    idx = 0;
  JER2_ARAD_SCH_FQM_TBL_DATA
    fqm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_SLOW_ENABLE_SET);

  value = DNX_SAND_BOOL2NUM(is_slow_enabled);

  offset  = JER2_ARAD_SCH_FLOW_TO_QRTT_ID(flow_ndx);
  idx = JER2_ARAD_SCH_FLOW_ID_IN_QRTT(flow_ndx);

  res = jer2_arad_sch_fqm_tbl_get_unsafe(
          unit, core,
          offset,
          &fqm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_sand_bitstream_set_any_field(
          &value,
          idx,
          1,
          &(fqm_tbl_data.flow_slow_enable)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  res = jer2_arad_sch_fqm_tbl_set_unsafe(
          unit, core,
          offset,
          &fqm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_slow_enable_set()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_slow_enable_get(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     int                   core,
    DNX_SAND_IN     JER2_ARAD_SCH_FLOW_ID          flow_ndx,
    DNX_SAND_OUT    uint8                   *is_slow_enabled
  )
{
  uint32
    offset = 0,
    idx = 0,
    res = DNX_SAND_OK,
    value = 0;
  JER2_ARAD_SCH_FQM_TBL_DATA
    fqm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_SLOW_ENABLE_GET);

  /*
   * Check input parameter
   */
  DNX_SAND_CHECK_NULL_INPUT(is_slow_enabled);

  offset  = JER2_ARAD_SCH_FLOW_TO_QRTT_ID(flow_ndx);
  idx = JER2_ARAD_SCH_FLOW_ID_IN_QRTT(flow_ndx);

  res = jer2_arad_sch_fqm_tbl_get_unsafe(
          unit, core,
          offset,
          &fqm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_sand_bitstream_get_any_field(
          &(fqm_tbl_data.flow_slow_enable),
          idx,
          1,
          &value
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  *is_slow_enabled = (value == 0x1) ? TRUE : FALSE;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_se_state_get()",0,0);
}

/*****************************************************
* NAME
*    jer2_arad_sch_flow_subflow_get
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Get a single subflow of a flow.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     int             core -
*     Identifier of core on device to access.
*   DNX_SAND_INOUT  JER2_ARAD_SCH_SUBFLOW      *subflow -
*     The subflow info
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
static uint32
  jer2_arad_sch_flow_subflow_get(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  int                  core,
    DNX_SAND_INOUT  JER2_ARAD_SCH_SUBFLOW      *subflow
  )
{
  uint32
    offset = 0,
    res;
  JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC
    internal_sub_flow;
  JER2_ARAD_SCH_FDMS_TBL_DATA
    fdms_tbl_data;
  JER2_ARAD_SCH_SHDS_TBL_DATA
    shds_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_SUBFLOW_GET);

  sal_memset(&shds_tbl_data, 0x0, sizeof(shds_tbl_data));
  /*
   * Read indirect from FDMS table
   */

  offset  = subflow->id;

  res = jer2_arad_sch_fdms_tbl_get_unsafe(
          unit, core,
          offset,
          &fdms_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (fdms_tbl_data.cos)
  {
    internal_sub_flow.cos = fdms_tbl_data.cos;
    internal_sub_flow.hr_sel_dual = fdms_tbl_data.hrsel_dual;
    internal_sub_flow.sch_number = fdms_tbl_data.sch_number;

    /*
     * Read indirect from SHDS table
     */

    offset  = subflow->id/2;

    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_shds_tbl_get_unsafe, (unit, core, offset, &shds_tbl_data)) ;
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if (subflow->id%2 == 0)
    {
      internal_sub_flow.max_burst = shds_tbl_data.max_burst_even;
      internal_sub_flow.max_burst_update = shds_tbl_data.max_burst_update_even;
      internal_sub_flow.peak_rate_man = shds_tbl_data.peak_rate_man_even;
      internal_sub_flow.peak_rate_exp = shds_tbl_data.peak_rate_exp_even;
      internal_sub_flow.slow_rate_index = shds_tbl_data.slow_rate2_sel_even;
    }
    else
    {
      internal_sub_flow.max_burst = shds_tbl_data.max_burst_odd;
      internal_sub_flow.max_burst_update = shds_tbl_data.max_burst_update_odd;
      internal_sub_flow.peak_rate_man = shds_tbl_data.peak_rate_man_odd;
      internal_sub_flow.peak_rate_exp = shds_tbl_data.peak_rate_exp_odd;
      internal_sub_flow.slow_rate_index = shds_tbl_data.slow_rate2_sel_odd;
    }

    JER2_ARAD_DEVICE_CHECK(unit, exit);
    /*
     * Convert to user representation.
     */
    res = jer2_arad_sch_INTERNAL_SUB_FLOW_to_SUB_FLOW_convert(
            unit,
            core,
            &internal_sub_flow,
            subflow
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    subflow->is_valid = TRUE;
  }
  else
  {
    subflow->is_valid = FALSE;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_subflow_get()",0,0);
}


/*****************************************************
* NAME
*    jer2_arad_sch_flow_subflow_set
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     int             core -
*     Identifier of core on device to access.
*   DNX_SAND_IN     uint32                 slow_rate_index -
*   DNX_SAND_INOUT  JER2_ARAD_SCH_SUBFLOW      *subflow -
*     The subflow info
*     Note that, specifically, 'subflow->update_bw_only' is
*     also input to this procedure!!
*   DNX_SAND_IN     JER2_ARAD_SCH_SUBFLOW        *prev_subflow -
*     previous subflow info
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
static uint32
  jer2_arad_sch_flow_subflow_set(
    DNX_SAND_IN     int                unit,
    DNX_SAND_IN     int                core,
    DNX_SAND_IN     uint32                slow_rate_index,
    DNX_SAND_IN     JER2_ARAD_SCH_SUBFLOW        *subflow
  )
{
  uint32
    offset = 0,
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC
    internal_sub_flow;
  JER2_ARAD_SCH_FDMS_TBL_DATA
    fdms_tbl_data;
  JER2_ARAD_SCH_SHDS_TBL_DATA
    shds_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_SUBFLOW_SET);

  sal_memset(&shds_tbl_data, 0x0, sizeof(shds_tbl_data));

  /*
   * Convert to internal representation.
   */
  res = jer2_arad_sch_SUB_FLOW_to_INTERNAL_SUB_FLOW_convert(
          unit,
          core,
          subflow,
          &internal_sub_flow
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /*
   *  Slow rate index
   */
  switch(slow_rate_index) {
    case JER2_ARAD_SCH_SLOW_RATE_NDX_1:
      internal_sub_flow.slow_rate_index = 0x0;
      break;
    case JER2_ARAD_SCH_SLOW_RATE_NDX_2:
      internal_sub_flow.slow_rate_index = 0x1;
      break;
    default:
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SLOW_RATE_INDEX_INVALID_ERR, 25, exit);
  }

  /*
   * Get current value - we only update part of the fields (odd/even)
   */
  offset  = subflow->id / 2;
  res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_shds_tbl_get_unsafe, (unit, core, offset, &shds_tbl_data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (subflow->id % 2 == 0)
  {
    if (
        (shds_tbl_data.max_burst_even >= internal_sub_flow.max_burst) &&
        (internal_sub_flow.max_burst != 0x0)
       )
    {
      internal_sub_flow.max_burst_update = 0;
    }
    else
    {
      internal_sub_flow.max_burst_update = 1;
    }
    shds_tbl_data.max_burst_even = internal_sub_flow.max_burst;
    shds_tbl_data.max_burst_update_even = internal_sub_flow.max_burst_update;
    shds_tbl_data.peak_rate_man_even = internal_sub_flow.peak_rate_man;
    shds_tbl_data.peak_rate_exp_even = internal_sub_flow.peak_rate_exp;
    shds_tbl_data.slow_rate2_sel_even = internal_sub_flow.slow_rate_index;
  }
  else
  {
    if (
        (shds_tbl_data.max_burst_odd >= internal_sub_flow.max_burst) &&
        (internal_sub_flow.max_burst != 0x0)
       )
    {
      internal_sub_flow.max_burst_update = 0;
    }
    else
    {
      internal_sub_flow.max_burst_update = 1;
    }
    shds_tbl_data.max_burst_odd = internal_sub_flow.max_burst;
    shds_tbl_data.max_burst_update_odd = internal_sub_flow.max_burst_update;
    shds_tbl_data.peak_rate_man_odd = internal_sub_flow.peak_rate_man;
    shds_tbl_data.peak_rate_exp_odd = internal_sub_flow.peak_rate_exp;
    shds_tbl_data.slow_rate2_sel_odd = internal_sub_flow.slow_rate_index;
  }

  /*
   * Write indirect to SHDS table
   */
  res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_shds_tbl_set_unsafe, (unit, core, offset, &shds_tbl_data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  JER2_ARAD_DEVICE_CHECK(unit, exit);
  if (!subflow->update_bw_only) {
    /*
     * Write indirect from FDMS table
     */
    offset  = subflow->id;

    fdms_tbl_data.cos = internal_sub_flow.cos;
    fdms_tbl_data.hrsel_dual = internal_sub_flow.hr_sel_dual;
    fdms_tbl_data.sch_number = internal_sub_flow.sch_number;
    res = jer2_arad_sch_fdms_tbl_set_unsafe(
          unit, core,
          offset,
          &fdms_tbl_data
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_subflow_set()",0,0);
}


/*****************************************************
* NAME
*    jer2_arad_sch_sub_flow_spouse_id
* TYPE:
*   PROC
* DATE:
*   24/10/2007
* FUNCTION:
*  in composite mode, return the id of the other subflow
* INPUT:
*   JER2_ARAD_SCH_FLOW_ID flow_id - flow index.
*   uint8 is_odd_even -
*     odd-even configuration:
*     TRUE is 0-1 configuration, FALSE is 0-2 configuration.
* RETURNS:
*   The id of the other subflow.
* REMARKS:
*    None.
*****************************************************/
static JER2_ARAD_SCH_FLOW_ID
  jer2_arad_sch_sub_flow_spouse_id(
    DNX_SAND_IN JER2_ARAD_SCH_FLOW_ID flow_id,
    DNX_SAND_IN uint8 is_odd_even
  )
{
  JER2_ARAD_SCH_FLOW_ID
    spouse_fid = 0;

  if (is_odd_even)
  {
    spouse_fid =
      (flow_id==JER2_ARAD_SCH_SUB_FLOW_BASE_FLOW_0_1(flow_id))?flow_id+1:flow_id-1;
  }
  else
  {
    spouse_fid =
      (flow_id==JER2_ARAD_SCH_SUB_FLOW_BASE_FLOW_0_2(flow_id))?flow_id+2:flow_id-2;
  }

  return spouse_fid;
}

/*****************************************************
* NAME
*    jer2_arad_sch_is_sub_flow_second
* TYPE:
*   PROC
* DATE:
*   24/10/2007
* FUNCTION:
*   In composite mode,
*   check if the id is the second (not independent) subflow.
* INPUT:
*   JER2_ARAD_SCH_FLOW_ID flow_id -
*   uint8 is_odd_even -
*     odd-even configuration:
*     TRUE is 0-1 configuration, FALSE is 0-2 configuration.
* RETURNS:
*   TRUE if this is second (not independent) subflow.
* REMARKS:
*    None.
*****************************************************/
static uint8
  jer2_arad_sch_is_sub_flow_second(
    JER2_ARAD_SCH_FLOW_ID flow_id,
    uint8 is_odd_even
  )
{
  uint8
    is_second = FALSE;

  is_second =
      (flow_id==JER2_ARAD_SCH_SUB_FLOW_BASE_FLOW(flow_id, is_odd_even))?FALSE:TRUE;

  return is_second;
}

uint32
  jer2_arad_sch_is_hr_subflow_valid(
    DNX_SAND_IN     JER2_ARAD_SCH_SUB_FLOW_HR*  hr_properties,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_INFO*      se
    )
{
  JER2_ARAD_SCH_SE_HR_MODE
    hr_sched_mode;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_SCH_IS_HR_SUBFLOW_VALID);

  if (se->type != JER2_ARAD_SCH_SE_TYPE_HR)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_TYPE_MISMATCH_ERR, 10, exit);
  }
  /*
   * Check if sp_class matches the se mode
   */
  hr_sched_mode = se->type_info.hr.mode;

  switch (hr_properties->sp_class)
  {
  /*
   * EF1-EF3 are the highest SP level in all HR modes
   */
  case JER2_ARAD_SCH_FLOW_HR_CLASS_EF1:
  case JER2_ARAD_SCH_FLOW_HR_CLASS_EF2:
  case JER2_ARAD_SCH_FLOW_HR_CLASS_EF3:
    /* Nothing to check */
    break;

  /* SINGLE WFQ MODE */
  case JER2_ARAD_SCH_FLOW_HR_SINGLE_CLASS_AF1_WFQ:
  case JER2_ARAD_SCH_FLOW_HR_SINGLE_CLASS_BE1:
    if (hr_sched_mode != JER2_ARAD_SCH_HR_MODE_SINGLE_WFQ)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 20, exit);
    }
    break;

  /* DUAL WFQ MODE */
  case JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_AF1_WFQ:
  case JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ:
  case JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE2:
    if (hr_sched_mode != JER2_ARAD_SCH_HR_MODE_DUAL_WFQ)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 30, exit);
    }
    break;

  /* ENHANCED PRIORITY MODE */
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF1:
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF2:
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF3:
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF4:
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF5:
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF6:
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ:
  case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE2:
    if (hr_sched_mode != JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 40, exit);
    }
    break;

  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_CLASS_OUT_OF_RANGE_ERR, 50, exit);
  }
/*
 * if the class is WFQ, check that the weight is within range (1-4096)
 */
  if ((JER2_ARAD_SCH_FLOW_HR_SINGLE_CLASS_AF1_WFQ   == hr_properties->sp_class) ||
      (JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_AF1_WFQ     == hr_properties->sp_class) ||
      (JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ     == hr_properties->sp_class) ||
      (JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ == hr_properties->sp_class))
  {
    if ((hr_properties->weight < JER2_ARAD_SCH_FLOW_HR_MIN_WEIGHT) ||
        (hr_properties->weight > JER2_ARAD_SCH_FLOW_HR_MAX_WEIGHT))
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_WEIGHT_OUT_OF_RANGE_ERR, 60, exit);
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_is_hr_subflow_valid",0,0);
}

uint32
  jer2_arad_sch_is_cl_subflow_valid(
    DNX_SAND_IN     int                        unit,
    DNX_SAND_IN     int                        core,
    DNX_SAND_IN     JER2_ARAD_SCH_SUB_FLOW_CL*             cl_properties,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_INFO*                 se
 )
{
  uint32
      res;
  JER2_ARAD_SCH_SE_CL_CLASS_INFO
    class_type;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_IS_CL_SUBFLOW_VALID);

  if (se->type != JER2_ARAD_SCH_SE_TYPE_CL)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 10, exit);
  }

  res = jer2_arad_sch_class_type_params_get_unsafe(
          unit, core,
          se->type_info.cl.id,
          &class_type
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /*
   * Check if class matches the se mode
   */
  switch (cl_properties->sp_class)
  {
    /*
     * SP1: The se operates in one of the following configurations:
     *      1. Mode 1 (the WFQ mode is don't care)
     *      2. Mode 2 and discrete WFQ mode
     *      3. Mode 4 (the WFQ mode is don't care)
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1:
    if (!((JER2_ARAD_SCH_CL_MODE_1 == class_type.mode) ||
          ((JER2_ARAD_SCH_CL_MODE_2 == class_type.mode) &&
           JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode))      ||
          (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 30, exit);
    }
    break;

    /*
     * SP2: The se operates in one of the following configurations:
     *      1. Mode 1 (the WFQ mode is don't care)
     *      2. Mode 2 and discrete WFQ mode
     *      3. Mode 3 (the WFQ mode is don't care)
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2:
    if (!((JER2_ARAD_SCH_CL_MODE_1 == class_type.mode)        ||
          ((JER2_ARAD_SCH_CL_MODE_2 == class_type.mode) &&
           JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode))             ||
          (JER2_ARAD_SCH_CL_MODE_3 == class_type.mode))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 40, exit);
    }
    break;

    /*
     * SP3,SP4: The se operates in Mode 1 (the WFQ mode is don't care)
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP4:
    if (!(JER2_ARAD_SCH_CL_MODE_1 == class_type.mode))
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 50, exit);
    }
    break;

    /*
     * SP1_WFQ: The se operates in one of the following configurations:
     *          1. Mode 3 and non-discrete WFQ mode
     *          2. Mode 5 and non-discrete WFQ mode
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ:
    if (!(JER2_ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode)  &&
          ((JER2_ARAD_SCH_CL_MODE_3 == class_type.mode) ||
           (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode)))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 60, exit);
    }
    break;

    /*
     * SP1_WFQ1, SP1_WFQ2, SP1_WFQ3: The se operates in one of
     *          the following configurations:
     *          1. Mode 3 and discrete WFQ mode
     *          2. Mode 5 and discrete WFQ mode
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ1:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ2:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ3:
    if (!(JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)  &&
          ((JER2_ARAD_SCH_CL_MODE_3 == class_type.mode) ||
           (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode)))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 70, exit);
    }
    break;

    /*
     * SP1_WFQ4: The se operates in Mode 5 and discrete WFQ mode
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ4:
    if (!(JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode) &&
          (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 80, exit);
    }
    break;

    /*
     * SP2_WFQ: The se operates in Mode 4 and non-discrete WFQ mode
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ:
    if (!(JER2_ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode) &&
          (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 90, exit);
    }
    break;

    /*
     * SP2_WFQ1, SP2_WFQ2, SP2_WFQ3: The se operates in
     *           mode 4 and discrete WFQ mode
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ1:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ2:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ3:
    if (!(JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode) &&
          (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 100, exit);
    }
    break;

    /*
     * SP3_WFQ1, SP3_WFQ2: The se operates in
     *           mode 2 and discrete WFQ mode
     */
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ1:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ2:
    if (!(JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode) &&
          (JER2_ARAD_SCH_CL_MODE_2 == class_type.mode))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 110, exit);
    }
    break;
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP_0_ENHANCED:
    if (class_type.enhanced_mode != JER2_ARAD_CL_ENHANCED_MODE_ENABLED_HP)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_ENHANCED_SP_MODE_MISMATCH_ERR, 120, exit);
    }
    break;
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP_5_ENHANCED:
    if (class_type.enhanced_mode != JER2_ARAD_CL_ENHANCED_MODE_ENABLED_LP)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_ENHANCED_SP_MODE_MISMATCH_ERR, 130, exit);
    }
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_CLASS_OUT_OF_RANGE_ERR, 140, exit);
  }
  /*
   * check that independent weight is within range
   */
  if (JER2_ARAD_SCH_IS_WFQ_CLASS_VAL(cl_properties->sp_class) &&
      JER2_ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode)
     )
  {
    if ((cl_properties->weight < JER2_ARAD_SCH_SUB_FLOW_CL_MIN_WEIGHT) ||
        (((JER2_ARAD_SCH_CL_MODE_3 == class_type.mode)  ||
          (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode)) &&
         (cl_properties->weight > JER2_ARAD_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODES_3_4)) ||
        ((JER2_ARAD_SCH_CL_MODE_5 == class_type.mode)  &&
         (cl_properties->weight > JER2_ARAD_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODE_5))
        )
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_WEIGHT_OUT_OF_RANGE_ERR, 150, exit);
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_is_cl_subflow_valid",0,0);
}


static uint32
  jer2_arad_sch_is_subflow_valid(
    DNX_SAND_IN     int           unit,
    DNX_SAND_IN     int           core,
    DNX_SAND_IN     uint32           sub_flow_i,
    DNX_SAND_IN     JER2_ARAD_SCH_FLOW_ID   flow_id,
    DNX_SAND_IN     JER2_ARAD_SCH_FLOW     *flow,
    DNX_SAND_IN     uint8           is_odd_even
    )
{
  uint32
    res;
  const JER2_ARAD_SCH_SUBFLOW*
    sub_flow;
  JER2_ARAD_SCH_SE_INFO
    se;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_IS_SUBFLOW_VALID);

  sub_flow = (const JER2_ARAD_SCH_SUBFLOW*)&flow->sub_flow[sub_flow_i];

  /*
   * Check that the credit source se element is valid and enabled
   */
  se.id = sub_flow->credit_source.id;

  res = jer2_arad_sch_se_get_unsafe(
          unit, core,
          se.id,
          &se
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  if (JER2_ARAD_SCH_SE_STATE_ENABLE != se.state)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_ATTACHED_TO_DISABLED_SCHEDULER_ERR, 6, exit);
  }

  switch(sub_flow->credit_source.se_type)
  {
  case JER2_ARAD_SCH_SE_TYPE_HR:
    res = jer2_arad_sch_is_hr_subflow_valid(
            &sub_flow->credit_source.se_info.hr, &se
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    break;

  case JER2_ARAD_SCH_SE_TYPE_CL:
    res = jer2_arad_sch_is_cl_subflow_valid(
            unit, core,
            &sub_flow->credit_source.se_info.cl,
            &se
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    break;

  case JER2_ARAD_SCH_SE_TYPE_FQ:
    /* Nothing to check for FQ subflows */
    break;

  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SUB_FLOW_SE_TYPE_OUT_OF_RANGE_ERR, 90, exit);
  }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_is_subflow_valid()",0,0);
}

uint32
  jer2_arad_sch_subflows_verify_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32
    res;
  uint32
    sub_flow_i = 0;
  uint8
    is_flow_odd_even = FALSE,
    flow_gap_found = FALSE;
  JER2_ARAD_SCH_GLOBAL_PER1K_INFO
    per1k_info;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SUBFLOWS_VERIFY_UNSAFE);

  /*
   * 1. At least one subflow should be valid
   * 2. No gaps are allowed in the subflows array
   * 3. All credit sources of the subflows should be associated with the
   *    same flow group.
   * 4. For each valid subflow check:
   *    a. The subflow id is legal
   *    b. The credit source is a valid se
   *    c. The subflow type is legal
   *    d, The type-specific properties are valid
   */

  res = jer2_arad_sch_per1k_info_get_unsafe(
          unit, core,
          JER2_ARAD_SCH_FLOW_TO_1K_ID(flow_ndx),
          &per1k_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  is_flow_odd_even = per1k_info.is_odd_even;

  for(sub_flow_i = 0; sub_flow_i < JER2_ARAD_SCH_NOF_SUB_FLOWS; sub_flow_i++)
  {
    if(flow->sub_flow[sub_flow_i].is_valid == TRUE)
    {
      if (flow_gap_found)
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_GAP_IN_SUB_FLOW_ERR, 15, exit);
      }
      res = jer2_arad_sch_is_subflow_valid(
              unit, core,
              sub_flow_i,
              flow_ndx,
              flow,
              is_flow_odd_even
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
    else
    {
      flow_gap_found = TRUE;
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_subflows_verify()",0,0);
}

/*********************************************************************
*     Input verification for jer2_arad_sch_flow_verify_unsafe
*********************************************************************/
uint32
  jer2_arad_sch_flow_verify_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_VERIFY_UNSAFE);

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_verify_unsafe()",0,0);
}

/*********************************************************************
*     Sets a se flow, from a scheduling element (or
*     elements) another element . The driver writes to the
*     following tables: Scheduler Enable Memory (SEM), Shaper
*     Descriptor Memory Static(SHDS) Flow Sub-Flow (FSF) Flow
*     Descriptor Memory Static (FDMS)
*     Details: in the H file. (search for prototype)
*     Note that 'flow->sub_flow[0].update_bw_only' is input to this procedure!!
*********************************************************************/
uint32
  jer2_arad_sch_flow_set_unsafe(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32
    nof_subflows = 0,
    nof_subflows_curr,
    res = DNX_SAND_OK;
  uint32
    slow_index[JER2_ARAD_SCH_NOF_SUB_FLOWS],
    sub_flow_i,
    spouse_subf_id;
  JER2_ARAD_SCH_GLOBAL_PER1K_INFO
    per1k_info;
  JER2_ARAD_SCH_SE_ID
    se_id = JER2_ARAD_SCH_SE_ID_INVALID;
  uint8
    is_slow_enabled,
    is_flow_odd_even = FALSE;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_SET_UNSAFE);

  res = jer2_arad_sch_per1k_info_get_unsafe(
          unit, core,
          JER2_ARAD_SCH_FLOW_TO_1K_ID(flow_ndx),
          &per1k_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  is_flow_odd_even = per1k_info.is_odd_even;

  for (sub_flow_i = 0; sub_flow_i < JER2_ARAD_SCH_NOF_SUB_FLOWS; sub_flow_i++)
  {
    if (flow->sub_flow[sub_flow_i].is_valid)
    {
      ++nof_subflows;
    }
  }
  /*
   * Verify that flow_ndx is an independent flow, and not a subflow.
   */
  if ((nof_subflows > 1) && JER2_ARAD_SCH_COMPOSITE_IS_SECOND_SUBFLOW(flow_ndx,is_flow_odd_even))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_ID_IS_SECOND_SUB_FLOW_ERR, 20, exit);
  }
  if (!flow->sub_flow[0].update_bw_only) {
    /*
     *	Delete the second sub-flow if needed
     */
    if (flow->sub_flow[JER2_ARAD_SCH_NOF_SUB_FLOWS-1].is_valid == FALSE)
    {
      /*
       *  Check if previously this was an active sub-flow.
       *  If so - delete it.
       */
      /*
       *  Get flow <=> sub flows mapping of the base flow id.
       *  It is flow_ndx itself if it is base-flow, or the flow-d
       *  of the spouse flow.
       */
      res = jer2_arad_sch_flow_nof_subflows_get(
              unit, core,
              flow_ndx,
              &nof_subflows_curr
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  
      if (nof_subflows_curr > 1)
      {
        /*
         *  This is currently a composite flow
         */
  
        spouse_subf_id = jer2_arad_sch_sub_flow_spouse_id(
                           flow_ndx,
                           is_flow_odd_even
                         );
  
         res = jer2_arad_sch_flow_delete_unsafe(
                  unit, core,
                  spouse_subf_id
                );
          DNX_SAND_CHECK_FUNC_RESULT(res, 44, exit);
      }
    }
  
    res = jer2_arad_sch_flow_nof_subflows_set(
            unit, core,
            flow_ndx,
            nof_subflows,
            is_flow_odd_even
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  
    if (flow->sub_flow[0].is_valid == FALSE)
    {
      /*
       *	Delete the flow and exit
       */
      res = jer2_arad_sch_flow_delete_unsafe(
                unit, core,
                flow_ndx
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 46, exit);
  
        JER2_ARAD_DO_NOTHING_AND_EXIT;
    }
  
    /*
     * Simple flows that use IDs that might also be used as aggregate,
     * should be disabled as elementary schedulers.
     */
    for (sub_flow_i = 0; sub_flow_i < nof_subflows; sub_flow_i++)
    {
      se_id = jer2_arad_sch_flow2se_id(unit, flow->sub_flow[sub_flow_i].id);
      if((JER2_ARAD_SCH_INDICATED_SE_ID_IS_VALID(se_id)) &&
         (flow->flow_type == JER2_ARAD_FLOW_SIMPLE))
      {
        res = jer2_arad_sch_se_state_set(
                unit, core,
                se_id,
                FALSE
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
      }
    }
  }
  /*
   * When the second sub-flow feature is enabled,
   *   each sub-flow might be set to use different slow value.
   */
  is_slow_enabled = flow->is_slow_enabled;

  /*
   * Write sub-flow/s properties (credit source, QoS params, shaper)
   * to FDMS & SHDS
   */
  for (sub_flow_i = 0; sub_flow_i < nof_subflows; sub_flow_i++)
  {
    if (flow->is_slow_enabled == FALSE)
    {
      slow_index[sub_flow_i] = JER2_ARAD_SCH_SLOW_RATE_NDX_1;
    }
    else
    {
      slow_index[sub_flow_i] = flow->sub_flow[sub_flow_i].slow_rate_ndx;
    }

    res = jer2_arad_sch_flow_subflow_set(
            unit, core,
            slow_index[sub_flow_i],
            &flow->sub_flow[sub_flow_i]
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }

  if (!flow->sub_flow[0].update_bw_only) {
    res = jer2_arad_sch_flow_slow_enable_set(
          unit, core,
          flow_ndx,
          DNX_SAND_BOOL2NUM(is_slow_enabled)
        );
  }
  DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);
exit:

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_set_unsafe()",flow_ndx,0);
}



/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32
    nof_subflows,
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_GLOBAL_PER1K_INFO
    per1k_info;
  uint8
    is_invalid,
    is_odd_even = FALSE,
    is_aggregate = FALSE;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(flow);
  jer2_arad_JER2_ARAD_SCH_FLOW_clear(unit, flow);

  /* verify flow id */
  res = jer2_arad_sch_flow_id_verify_unsafe(
            unit,
            flow_ndx
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = jer2_arad_sch_flow_is_deleted_get_unsafe(
          unit, core,
          flow_ndx,
          &is_invalid
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  flow->sub_flow[0].is_valid = (is_invalid)?FALSE:TRUE;

  if(!flow->sub_flow[0].is_valid)
  {
    jer2_arad_JER2_ARAD_SCH_FLOW_clear(unit, flow);
  }
  res = jer2_arad_sch_per1k_info_get(
      unit, core,
      JER2_ARAD_SCH_FLOW_TO_1K_ID(flow_ndx),
      &per1k_info
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  is_odd_even = per1k_info.is_odd_even;
  /*
   *  Get flow <=> sub flows mapping of the base flow id.
   *  It is flow_ndx itself if it is base-flow, or the flow-d
   *  of the spouse flow.
   */
  res = jer2_arad_sch_flow_nof_subflows_get(
          unit, core,
          flow_ndx,
          &nof_subflows
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (nof_subflows > 1)
  {
    /*
     *  This is a composite flow
     */
    if (jer2_arad_sch_is_sub_flow_second(flow_ndx, is_odd_even))
    {
      flow->flow_type = JER2_ARAD_FLOW_NONE;
      goto exit;
    }
  }
  /*
   * If the flow ID is within the aggregate range, check if it is indeed an
   * aggregate flow or a regular one
   */
  if (jer2_arad_sch_is_flow_id_se_id(unit, flow_ndx))
  {
    res = jer2_arad_sch_se_state_get(
            unit, core,
            jer2_arad_sch_flow2se_id(unit, flow_ndx),
            &is_aggregate
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    flow->flow_type  = is_aggregate?JER2_ARAD_FLOW_AGGREGATE:JER2_ARAD_FLOW_SIMPLE;
  }
  else
  {
    flow->flow_type  = JER2_ARAD_FLOW_SIMPLE;
  }

  /*
   * Read sub-flow/s properties (credit source, QoS params, shaper)
   * from FDMS & SHDS
   */

  if (nof_subflows == JER2_ARAD_SCH_NOF_SUB_FLOWS)
  {
    /* composite flow */
    flow->sub_flow[0].id = flow_ndx;
    flow->sub_flow[1].id = jer2_arad_sch_sub_flow_spouse_id(
                             flow_ndx,
                             is_odd_even
                           );

    res = jer2_arad_sch_flow_is_deleted_get_unsafe(
            unit, core,
            flow->sub_flow[1].id,
            &is_invalid
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

    flow->sub_flow[1].is_valid = DNX_SAND_NUM2BOOL_INVERSE(is_invalid);

    if(flow->sub_flow[0].is_valid)
    {
      res = jer2_arad_sch_flow_subflow_get(
            unit, core,
              &flow->sub_flow[0]
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      res = jer2_arad_sch_flow_subflow_get(
              unit, core,
              &flow->sub_flow[1]
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }
  }
  else
  {
    /* single (not composite) flow */
    flow->sub_flow[0].id = flow_ndx;

    if(flow->sub_flow[0].is_valid)
    {
      res = jer2_arad_sch_flow_subflow_get(
              unit, core,
              &flow->sub_flow[0]
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    }

    jer2_arad_JER2_ARAD_SCH_SUBFLOW_clear(
            unit,
            &flow->sub_flow[1]
          );
  }

  if (!is_aggregate)
  {
    jer2_arad_sch_flow_slow_enable_get(
      unit, core,
      flow_ndx,
      &flow->is_slow_enabled
    );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }
  else
  {
    flow->is_slow_enabled = FALSE;
  }

  if(flow->sub_flow[0].is_valid)
  {
    /*
     * Check if the flow is valid
     */
    res = jer2_arad_sch_flow_verify_unsafe(
            unit, core,
            flow_ndx,
            flow
          );
    if(dnx_sand_get_error_code_from_error_word(res) != DNX_SAND_OK)
    {
      flow->flow_type = JER2_ARAD_FLOW_NONE;
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_get_unsafe()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_status_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_STATUS    state
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_STATUS_VERIFY);

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_status_verify()",0,0);
}

/*********************************************************************
*     Set flow state to off/on. The state of the flow will be
*     updated, unless was configured otherwise. Note: useful
*     for virtual flows, for which the flow state must be
*     explicitly set
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_status_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_STATUS    state
  )
{
  uint32
    offset,
    res;
  JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_DATA
    fsm_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_STATUS_SET_UNSAFE);
  /*
   * Write to the FSM table.
   * This is a one-entry table, so the offset is always 0
   */
  offset = 0;
  fsm_data.message_flow_id = flow_ndx;
  fsm_data.message_type = state;

  res = jer2_arad_sch_force_status_message_tbl_set_unsafe(
          unit, core,
          offset,
          &fsm_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_status_set_unsafe()",0,0);
}

/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */
