#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_scheduler_flows.c,v 1.10 Broadcom SDK $
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
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_scheduler_flows.h>
#include <soc/dpp/ARAD/arad_scheduler_flow_converts.h>
#include <soc/dpp/ARAD/arad_scheduler_end2end.h>
#include <soc/dpp/ARAD/arad_scheduler_elements.h>

#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/arad_general.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/mbcm.h>
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
  arad_sch_flow_ipf_config_mode_set_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  ARAD_SCH_FLOW_IPF_CONFIG_MODE mode
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET_UNSAFE);
  
  SOC_SAND_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.tm_info.ipf_config_mode.set(unit, mode));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_sch_flow_ipf_config_mode_set_unsafe()", 0, 0);
}


uint32
  arad_sch_flow_ipf_config_mode_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  ARAD_SCH_FLOW_IPF_CONFIG_MODE mode
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(mode, ARAD_SCH_NOF_FLOW_IPF_CONFIG_MODES, ARAD_END2END_SCHEDULER_MODE_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_sch_flow_ipf_config_mode_set_verify()", 0, 0);
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
  arad_sch_flow_ipf_config_mode_get_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_OUT ARAD_SCH_FLOW_IPF_CONFIG_MODE *mode
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_IPF_CONFIG_MODE_GET_UNSAFE);
    
  SOC_SAND_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.tm_info.ipf_config_mode.get(unit, mode));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_sch_flow_ipf_config_mode_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_nof_subflows_get(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID          base_flow_id,
    SOC_SAND_OUT    uint32                   *nof_subflows
  )
{
  uint32
    offset = 0,
    idx = 0,
    res = SOC_SAND_OK;
  ARAD_SCH_FSF_TBL_DATA
    fsf_tbl_data;
  ARAD_SCH_GLOBAL_PER1K_INFO
    global_per1k_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_NOF_SUBFLOWS_GET);

  res = arad_sch_per1k_info_get_unsafe(
          unit, core,
          ARAD_SCH_FLOW_TO_1K_ID(base_flow_id),
          &global_per1k_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  offset  = ARAD_REG_IDX_GET(base_flow_id, SOC_SAND_REG_SIZE_BITS);

  idx = ARAD_SCH_SUB_FLOW_BASE_FLOW(base_flow_id, global_per1k_info.is_odd_even);
  idx = ARAD_FLD_IDX_GET(idx, SOC_SAND_REG_SIZE_BITS);
  idx = idx / 2 + idx % 2;

  res = arad_sch_fsf_tbl_get_unsafe(
          unit, core,
          offset,
          &fsf_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  *nof_subflows = SOC_SAND_GET_BIT(fsf_tbl_data.sfenable,idx)? 2 : 1;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_nof_subflows_get()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_nof_subflows_set(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID           base_flow_id,
    SOC_SAND_IN     uint32                    nof_subflows,
    SOC_SAND_IN     uint8                   is_odd_even
  )
{
  uint32
    offset = 0,
    idx = 0,
    res = SOC_SAND_OK;
  ARAD_SCH_FSF_TBL_DATA
    fsf_tbl_data;
  uint32
    bit_val_to_set = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_NOF_SUBFLOWS_SET);

  offset  = ARAD_REG_IDX_GET(base_flow_id, SOC_SAND_REG_SIZE_BITS);

  idx = ARAD_SCH_SUB_FLOW_BASE_FLOW(base_flow_id, is_odd_even);
  idx = ARAD_FLD_IDX_GET(idx, SOC_SAND_REG_SIZE_BITS);
  idx = (idx/2) + (idx%2);

  res = arad_sch_fsf_tbl_get_unsafe(
          unit, core,
          offset,
          &fsf_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  bit_val_to_set = (nof_subflows > 1) ? 0x1 : 0x0;

  if (SOC_SAND_GET_BIT(fsf_tbl_data.sfenable,idx) != bit_val_to_set)
  {
     SOC_SAND_SET_BIT(fsf_tbl_data.sfenable,bit_val_to_set,idx);
     res = arad_sch_fsf_tbl_set_unsafe(
             unit, core,
             offset,
             &fsf_tbl_data
           );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_nof_subflows_set()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_slow_enable_set(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID           flow_ndx,
    SOC_SAND_IN     uint8                   is_slow_enabled
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    value,
    offset = 0,
    idx = 0;
  ARAD_SCH_FQM_TBL_DATA
    fqm_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_SLOW_ENABLE_SET);

  value = SOC_SAND_BOOL2NUM(is_slow_enabled);

  offset  = ARAD_SCH_FLOW_TO_QRTT_ID(flow_ndx);
  idx = ARAD_SCH_FLOW_ID_IN_QRTT(flow_ndx);

  res = arad_sch_fqm_tbl_get_unsafe(
          unit, core,
          offset,
          &fqm_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = soc_sand_bitstream_set_any_field(
          &value,
          idx,
          1,
          &(fqm_tbl_data.flow_slow_enable)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  res = arad_sch_fqm_tbl_set_unsafe(
          unit, core,
          offset,
          &fqm_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_slow_enable_set()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_slow_enable_get(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID          flow_ndx,
    SOC_SAND_OUT    uint8                   *is_slow_enabled
  )
{
  uint32
    offset = 0,
    idx = 0,
    res = SOC_SAND_OK,
    value = 0;
  ARAD_SCH_FQM_TBL_DATA
    fqm_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_SLOW_ENABLE_GET);

  /*
   * Check input parameter
   */
  SOC_SAND_CHECK_NULL_INPUT(is_slow_enabled);

  offset  = ARAD_SCH_FLOW_TO_QRTT_ID(flow_ndx);
  idx = ARAD_SCH_FLOW_ID_IN_QRTT(flow_ndx);

  res = arad_sch_fqm_tbl_get_unsafe(
          unit, core,
          offset,
          &fqm_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = soc_sand_bitstream_get_any_field(
          &(fqm_tbl_data.flow_slow_enable),
          idx,
          1,
          &value
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  *is_slow_enabled = (value == 0x1) ? TRUE : FALSE;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_se_state_get()",0,0);
}

/*****************************************************
* NAME
*    arad_sch_flow_subflow_get
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Get a single subflow of a flow.
* INPUT:
*   SOC_SAND_IN     int             unit -
*     Identifier of device to access.
*   SOC_SAND_IN     int             core -
*     Identifier of core on device to access.
*   SOC_SAND_INOUT  ARAD_SCH_SUBFLOW      *subflow -
*     The subflow info
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
STATIC uint32
  arad_sch_flow_subflow_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  int                  core,
    SOC_SAND_INOUT  ARAD_SCH_SUBFLOW      *subflow
  )
{
  uint32
    offset = 0,
    res;
  ARAD_SCH_INTERNAL_SUB_FLOW_DESC
    internal_sub_flow;
  ARAD_SCH_FDMS_TBL_DATA
    fdms_tbl_data;
  ARAD_SCH_SHDS_TBL_DATA
    shds_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_SUBFLOW_GET);

  sal_memset(&shds_tbl_data, 0x0, sizeof(shds_tbl_data));
  /*
   * Read indirect from FDMS table
   */

  offset  = subflow->id;

  res = arad_sch_fdms_tbl_get_unsafe(
          unit, core,
          offset,
          &fdms_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (fdms_tbl_data.cos)
  {
    internal_sub_flow.cos = fdms_tbl_data.cos;
    internal_sub_flow.hr_sel_dual = fdms_tbl_data.hrsel_dual;
    internal_sub_flow.sch_number = fdms_tbl_data.sch_number;

    /*
     * Read indirect from SHDS table
     */

    offset  = subflow->id/2;

    res = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_sch_shds_tbl_get_unsafe, (unit, core, offset, &shds_tbl_data)) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

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

    ARAD_DEVICE_CHECK(unit, exit);
    /*
     * Convert to user representation.
     */
    res = arad_sch_INTERNAL_SUB_FLOW_to_SUB_FLOW_convert(
            unit,
            core,
            &internal_sub_flow,
            subflow
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    subflow->is_valid = TRUE;
  }
  else
  {
    subflow->is_valid = FALSE;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_subflow_get()",0,0);
}


/*****************************************************
* NAME
*    arad_sch_flow_subflow_set
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   SOC_SAND_IN     int             unit -
*     Identifier of device to access.
*   SOC_SAND_IN     int             core -
*     Identifier of core on device to access.
*   SOC_SAND_IN     uint32                 slow_rate_index -
*   SOC_SAND_INOUT  ARAD_SCH_SUBFLOW      *subflow -
*     The subflow info
*     Note that, specifically, 'subflow->update_bw_only' is
*     also input to this procedure!!
*   SOC_SAND_IN     ARAD_SCH_SUBFLOW        *prev_subflow -
*     previous subflow info
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
STATIC uint32
  arad_sch_flow_subflow_set(
    SOC_SAND_IN     int                unit,
    SOC_SAND_IN     int                core,
    SOC_SAND_IN     uint32                slow_rate_index,
    SOC_SAND_IN     ARAD_SCH_SUBFLOW        *subflow
  )
{
  uint32
    offset = 0,
    res = SOC_SAND_OK;
  ARAD_SCH_INTERNAL_SUB_FLOW_DESC
    internal_sub_flow;
  ARAD_SCH_FDMS_TBL_DATA
    fdms_tbl_data;
  ARAD_SCH_SHDS_TBL_DATA
    shds_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_SUBFLOW_SET);

  sal_memset(&shds_tbl_data, 0x0, sizeof(shds_tbl_data));

  /*
   * Convert to internal representation.
   */
  res = arad_sch_SUB_FLOW_to_INTERNAL_SUB_FLOW_convert(
          unit,
          core,
          subflow,
          &internal_sub_flow
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /*
   *  Slow rate index
   */
  switch(slow_rate_index) {
    case ARAD_SCH_SLOW_RATE_NDX_1:
      internal_sub_flow.slow_rate_index = 0x0;
      break;
    case ARAD_SCH_SLOW_RATE_NDX_2:
      internal_sub_flow.slow_rate_index = 0x1;
      break;
    default:
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SLOW_RATE_INDEX_INVALID_ERR, 25, exit);
  }

  /*
   * Get current value - we only update part of the fields (odd/even)
   */
  offset  = subflow->id / 2;
  res = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_sch_shds_tbl_get_unsafe, (unit, core, offset, &shds_tbl_data)) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

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
  res = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_sch_shds_tbl_set_unsafe, (unit, core, offset, &shds_tbl_data)) ;
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  ARAD_DEVICE_CHECK(unit, exit);
  if (!subflow->update_bw_only) {
    /*
     * Write indirect from FDMS table
     */
    offset  = subflow->id;

    fdms_tbl_data.cos = internal_sub_flow.cos;
    fdms_tbl_data.hrsel_dual = internal_sub_flow.hr_sel_dual;
    fdms_tbl_data.sch_number = internal_sub_flow.sch_number;
    res = arad_sch_fdms_tbl_set_unsafe(
          unit, core,
          offset,
          &fdms_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_subflow_set()",0,0);
}


/*****************************************************
* NAME
*    arad_sch_sub_flow_spouse_id
* TYPE:
*   PROC
* DATE:
*   24/10/2007
* FUNCTION:
*  in composite mode, return the id of the other subflow
* INPUT:
*   ARAD_SCH_FLOW_ID flow_id - flow index.
*   uint8 is_odd_even -
*     odd-even configuration:
*     TRUE is 0-1 configuration, FALSE is 0-2 configuration.
* RETURNS:
*   The id of the other subflow.
* REMARKS:
*    None.
*****************************************************/
STATIC ARAD_SCH_FLOW_ID
  arad_sch_sub_flow_spouse_id(
    SOC_SAND_IN ARAD_SCH_FLOW_ID flow_id,
    SOC_SAND_IN uint8 is_odd_even
  )
{
  ARAD_SCH_FLOW_ID
    spouse_fid = 0;

  if (is_odd_even)
  {
    spouse_fid =
      (flow_id==ARAD_SCH_SUB_FLOW_BASE_FLOW_0_1(flow_id))?flow_id+1:flow_id-1;
  }
  else
  {
    spouse_fid =
      (flow_id==ARAD_SCH_SUB_FLOW_BASE_FLOW_0_2(flow_id))?flow_id+2:flow_id-2;
  }

  return spouse_fid;
}

/*****************************************************
* NAME
*    arad_sch_is_sub_flow_second
* TYPE:
*   PROC
* DATE:
*   24/10/2007
* FUNCTION:
*   In composite mode,
*   check if the id is the second (not independent) subflow.
* INPUT:
*   ARAD_SCH_FLOW_ID flow_id -
*   uint8 is_odd_even -
*     odd-even configuration:
*     TRUE is 0-1 configuration, FALSE is 0-2 configuration.
* RETURNS:
*   TRUE if this is second (not independent) subflow.
* REMARKS:
*    None.
*****************************************************/
STATIC uint8
  arad_sch_is_sub_flow_second(
    ARAD_SCH_FLOW_ID flow_id,
    uint8 is_odd_even
  )
{
  uint8
    is_second = FALSE;

  is_second =
      (flow_id==ARAD_SCH_SUB_FLOW_BASE_FLOW(flow_id, is_odd_even))?FALSE:TRUE;

  return is_second;
}

uint32
  arad_sch_is_hr_subflow_valid(
    SOC_SAND_IN     ARAD_SCH_SUB_FLOW_HR*  hr_properties,
    SOC_SAND_IN     ARAD_SCH_SE_INFO*      se
    )
{
  ARAD_SCH_SE_HR_MODE
    hr_sched_mode;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(ARAD_SCH_IS_HR_SUBFLOW_VALID);

  if (se->type != ARAD_SCH_SE_TYPE_HR)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_TYPE_MISMATCH_ERR, 10, exit);
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
  case ARAD_SCH_FLOW_HR_CLASS_EF1:
  case ARAD_SCH_FLOW_HR_CLASS_EF2:
  case ARAD_SCH_FLOW_HR_CLASS_EF3:
    /* Nothing to check */
    break;

  /* SINGLE WFQ MODE */
  case ARAD_SCH_FLOW_HR_SINGLE_CLASS_AF1_WFQ:
  case ARAD_SCH_FLOW_HR_SINGLE_CLASS_BE1:
    if (hr_sched_mode != ARAD_SCH_HR_MODE_SINGLE_WFQ)
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 20, exit);
    }
    break;

  /* DUAL WFQ MODE */
  case ARAD_SCH_FLOW_HR_DUAL_CLASS_AF1_WFQ:
  case ARAD_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ:
  case ARAD_SCH_FLOW_HR_DUAL_CLASS_BE2:
    if (hr_sched_mode != ARAD_SCH_HR_MODE_DUAL_WFQ)
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 30, exit);
    }
    break;

  /* ENHANCED PRIORITY MODE */
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF1:
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF2:
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF3:
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF4:
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF5:
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF6:
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ:
  case ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE2:
    if (hr_sched_mode != ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ)
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 40, exit);
    }
    break;

  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_CLASS_OUT_OF_RANGE_ERR, 50, exit);
  }
/*
 * if the class is WFQ, check that the weight is within range (1-4096)
 */
  if ((ARAD_SCH_FLOW_HR_SINGLE_CLASS_AF1_WFQ   == hr_properties->sp_class) ||
      (ARAD_SCH_FLOW_HR_DUAL_CLASS_AF1_WFQ     == hr_properties->sp_class) ||
      (ARAD_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ     == hr_properties->sp_class) ||
      (ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ == hr_properties->sp_class))
  {
    if ((hr_properties->weight < ARAD_SCH_FLOW_HR_MIN_WEIGHT) ||
        (hr_properties->weight > ARAD_SCH_FLOW_HR_MAX_WEIGHT))
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_WEIGHT_OUT_OF_RANGE_ERR, 60, exit);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("arad_sch_is_hr_subflow_valid",0,0);
}

uint32
  arad_sch_is_cl_subflow_valid(
    SOC_SAND_IN     int                        unit,
    SOC_SAND_IN     int                        core,
    SOC_SAND_IN     ARAD_SCH_SUB_FLOW_CL*             cl_properties,
    SOC_SAND_IN     ARAD_SCH_SE_INFO*                 se
 )
{
  uint32
      res;
  ARAD_SCH_SE_CL_CLASS_INFO
    class_type;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_IS_CL_SUBFLOW_VALID);

  if (se->type != ARAD_SCH_SE_TYPE_CL)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 10, exit);
  }

  res = arad_sch_class_type_params_get_unsafe(
          unit, core,
          se->type_info.cl.id,
          &class_type
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

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
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP1:
    if (!((ARAD_SCH_CL_MODE_1 == class_type.mode) ||
          ((ARAD_SCH_CL_MODE_2 == class_type.mode) &&
           ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode))      ||
          (ARAD_SCH_CL_MODE_4 == class_type.mode))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 30, exit);
    }
    break;

    /*
     * SP2: The se operates in one of the following configurations:
     *      1. Mode 1 (the WFQ mode is don't care)
     *      2. Mode 2 and discrete WFQ mode
     *      3. Mode 3 (the WFQ mode is don't care)
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP2:
    if (!((ARAD_SCH_CL_MODE_1 == class_type.mode)        ||
          ((ARAD_SCH_CL_MODE_2 == class_type.mode) &&
           ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode))             ||
          (ARAD_SCH_CL_MODE_3 == class_type.mode))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 40, exit);
    }
    break;

    /*
     * SP3,SP4: The se operates in Mode 1 (the WFQ mode is don't care)
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP3:
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP4:
    if (!(ARAD_SCH_CL_MODE_1 == class_type.mode))
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 50, exit);
    }
    break;

    /*
     * SP1_WFQ: The se operates in one of the following configurations:
     *          1. Mode 3 and non-discrete WFQ mode
     *          2. Mode 5 and non-discrete WFQ mode
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ:
    if (!(ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode)  &&
          ((ARAD_SCH_CL_MODE_3 == class_type.mode) ||
           (ARAD_SCH_CL_MODE_5 == class_type.mode)))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 60, exit);
    }
    break;

    /*
     * SP1_WFQ1, SP1_WFQ2, SP1_WFQ3: The se operates in one of
     *          the following configurations:
     *          1. Mode 3 and discrete WFQ mode
     *          2. Mode 5 and discrete WFQ mode
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ1:
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ2:
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ3:
    if (!(ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)  &&
          ((ARAD_SCH_CL_MODE_3 == class_type.mode) ||
           (ARAD_SCH_CL_MODE_5 == class_type.mode)))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 70, exit);
    }
    break;

    /*
     * SP1_WFQ4: The se operates in Mode 5 and discrete WFQ mode
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ4:
    if (!(ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode) &&
          (ARAD_SCH_CL_MODE_5 == class_type.mode))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 80, exit);
    }
    break;

    /*
     * SP2_WFQ: The se operates in Mode 4 and non-discrete WFQ mode
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ:
    if (!(ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode) &&
          (ARAD_SCH_CL_MODE_4 == class_type.mode))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 90, exit);
    }
    break;

    /*
     * SP2_WFQ1, SP2_WFQ2, SP2_WFQ3: The se operates in
     *           mode 4 and discrete WFQ mode
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ1:
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ2:
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ3:
    if (!(ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode) &&
          (ARAD_SCH_CL_MODE_4 == class_type.mode))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 100, exit);
    }
    break;

    /*
     * SP3_WFQ1, SP3_WFQ2: The se operates in
     *           mode 2 and discrete WFQ mode
     */
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ1:
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ2:
    if (!(ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode) &&
          (ARAD_SCH_CL_MODE_2 == class_type.mode))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR, 110, exit);
    }
    break;
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP_0_ENHANCED:
    if (class_type.enhanced_mode != ARAD_CL_ENHANCED_MODE_ENABLED_HP)
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_ENHANCED_SP_MODE_MISMATCH_ERR, 120, exit);
    }
    break;
  case ARAD_SCH_SUB_FLOW_CL_CLASS_SP_5_ENHANCED:
    if (class_type.enhanced_mode != ARAD_CL_ENHANCED_MODE_ENABLED_LP)
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_ENHANCED_SP_MODE_MISMATCH_ERR, 130, exit);
    }
    break;
  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_CLASS_OUT_OF_RANGE_ERR, 140, exit);
  }
  /*
   * check that independent weight is within range
   */
  if (ARAD_SCH_IS_WFQ_CLASS_VAL(cl_properties->sp_class) &&
      ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode)
     )
  {
    if ((cl_properties->weight < ARAD_SCH_SUB_FLOW_CL_MIN_WEIGHT) ||
        (((ARAD_SCH_CL_MODE_3 == class_type.mode)  ||
          (ARAD_SCH_CL_MODE_4 == class_type.mode)) &&
         (cl_properties->weight > ARAD_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODES_3_4)) ||
        ((ARAD_SCH_CL_MODE_5 == class_type.mode)  &&
         (cl_properties->weight > ARAD_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODE_5))
        )
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_WEIGHT_OUT_OF_RANGE_ERR, 150, exit);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("arad_sch_is_cl_subflow_valid",0,0);
}


STATIC uint32
  arad_sch_is_subflow_valid(
    SOC_SAND_IN     int           unit,
    SOC_SAND_IN     int           core,
    SOC_SAND_IN     uint32           sub_flow_i,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID   flow_id,
    SOC_SAND_IN     ARAD_SCH_FLOW     *flow,
    SOC_SAND_IN     uint8           is_odd_even
    )
{
  uint32
    res;
  const ARAD_SCH_SUBFLOW*
    sub_flow;
  ARAD_SCH_SE_INFO
    se;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_IS_SUBFLOW_VALID);

  sub_flow = (const ARAD_SCH_SUBFLOW*)&flow->sub_flow[sub_flow_i];

  /*
   * Check that the credit source se element is valid and enabled
   */
  se.id = sub_flow->credit_source.id;

  res = arad_sch_se_get_unsafe(
          unit, core,
          se.id,
          &se
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  if (ARAD_SCH_SE_STATE_ENABLE != se.state)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_ATTACHED_TO_DISABLED_SCHEDULER_ERR, 6, exit);
  }

  switch(sub_flow->credit_source.se_type)
  {
  case ARAD_SCH_SE_TYPE_HR:
    res = arad_sch_is_hr_subflow_valid(
            &sub_flow->credit_source.se_info.hr, &se
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    break;

  case ARAD_SCH_SE_TYPE_CL:
    res = arad_sch_is_cl_subflow_valid(
            unit, core,
            &sub_flow->credit_source.se_info.cl,
            &se
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    break;

  case ARAD_SCH_SE_TYPE_FQ:
    /* Nothing to check for FQ subflows */
    break;

  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_SCH_SUB_FLOW_SE_TYPE_OUT_OF_RANGE_ERR, 90, exit);
  }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_is_subflow_valid()",0,0);
}

uint32
  arad_sch_subflows_verify_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW           *flow
  )
{
  uint32
    res;
  uint32
    sub_flow_i = 0;
  uint8
    is_flow_odd_even = FALSE,
    flow_gap_found = FALSE;
  ARAD_SCH_GLOBAL_PER1K_INFO
    per1k_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_SUBFLOWS_VERIFY_UNSAFE);

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

  res = arad_sch_per1k_info_get_unsafe(
          unit, core,
          ARAD_SCH_FLOW_TO_1K_ID(flow_ndx),
          &per1k_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  is_flow_odd_even = per1k_info.is_odd_even;

  for(sub_flow_i = 0; sub_flow_i < ARAD_SCH_NOF_SUB_FLOWS; sub_flow_i++)
  {
    if(flow->sub_flow[sub_flow_i].is_valid == TRUE)
    {
      if (flow_gap_found)
      {
        SOC_SAND_SET_ERROR_CODE(ARAD_SCH_GAP_IN_SUB_FLOW_ERR, 15, exit);
      }
      res = arad_sch_is_subflow_valid(
              unit, core,
              sub_flow_i,
              flow_ndx,
              flow,
              is_flow_odd_even
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
    else
    {
      flow_gap_found = TRUE;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_subflows_verify()",0,0);
}

/*********************************************************************
*     Input verification for arad_sch_flow_verify_unsafe
*********************************************************************/
uint32
  arad_sch_flow_verify_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW           *flow
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_VERIFY_UNSAFE);

  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_verify_unsafe()",0,0);
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
  arad_sch_flow_set_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  int                core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW           *flow
  )
{
  uint32
    nof_subflows = 0,
    nof_subflows_curr,
    res = SOC_SAND_OK;
  uint32
    slow_index[ARAD_SCH_NOF_SUB_FLOWS],
    sub_flow_i,
    spouse_subf_id;
  ARAD_SCH_GLOBAL_PER1K_INFO
    per1k_info;
  ARAD_SCH_SE_ID
    se_id = ARAD_SCH_SE_ID_INVALID;
  uint8
    is_slow_enabled,
    is_flow_odd_even = FALSE;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_SET_UNSAFE);

  res = arad_sch_per1k_info_get_unsafe(
          unit, core,
          ARAD_SCH_FLOW_TO_1K_ID(flow_ndx),
          &per1k_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  is_flow_odd_even = per1k_info.is_odd_even;

  for (sub_flow_i = 0; sub_flow_i < ARAD_SCH_NOF_SUB_FLOWS; sub_flow_i++)
  {
    if (flow->sub_flow[sub_flow_i].is_valid)
    {
      ++nof_subflows;
    }
  }
  /*
   * Verify that flow_ndx is an independent flow, and not a subflow.
   */
  if ((nof_subflows > 1) && ARAD_SCH_COMPOSITE_IS_SECOND_SUBFLOW(flow_ndx,is_flow_odd_even))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_SCH_FLOW_ID_IS_SECOND_SUB_FLOW_ERR, 20, exit);
  }
  if (!flow->sub_flow[0].update_bw_only) {
    /*
     *	Delete the second sub-flow if needed
     */
    if (flow->sub_flow[ARAD_SCH_NOF_SUB_FLOWS-1].is_valid == FALSE)
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
      res = arad_sch_flow_nof_subflows_get(
              unit, core,
              flow_ndx,
              &nof_subflows_curr
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  
      if (nof_subflows_curr > 1)
      {
        /*
         *  This is currently a composite flow
         */
  
        spouse_subf_id = arad_sch_sub_flow_spouse_id(
                           flow_ndx,
                           is_flow_odd_even
                         );
  
         res = arad_sch_flow_delete_unsafe(
                  unit, core,
                  spouse_subf_id
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);
      }
    }
  
    res = arad_sch_flow_nof_subflows_set(
            unit, core,
            flow_ndx,
            nof_subflows,
            is_flow_odd_even
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  
    if (flow->sub_flow[0].is_valid == FALSE)
    {
      /*
       *	Delete the flow and exit
       */
      res = arad_sch_flow_delete_unsafe(
                unit, core,
                flow_ndx
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);
  
        ARAD_DO_NOTHING_AND_EXIT;
    }
  
    /*
     * Simple flows that use IDs that might also be used as aggregate,
     * should be disabled as elementary schedulers.
     */
    for (sub_flow_i = 0; sub_flow_i < nof_subflows; sub_flow_i++)
    {
      se_id = arad_sch_flow2se_id(unit, flow->sub_flow[sub_flow_i].id);
      if((ARAD_SCH_INDICATED_SE_ID_IS_VALID(se_id)) &&
         (flow->flow_type == ARAD_FLOW_SIMPLE))
      {
        res = arad_sch_se_state_set(
                unit, core,
                se_id,
                FALSE
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
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
      slow_index[sub_flow_i] = ARAD_SCH_SLOW_RATE_NDX_1;
    }
    else
    {
      slow_index[sub_flow_i] = flow->sub_flow[sub_flow_i].slow_rate_ndx;
    }

    res = arad_sch_flow_subflow_set(
            unit, core,
            slow_index[sub_flow_i],
            &flow->sub_flow[sub_flow_i]
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }

  if (!flow->sub_flow[0].update_bw_only) {
    res = arad_sch_flow_slow_enable_set(
          unit, core,
          flow_ndx,
          SOC_SAND_BOOL2NUM(is_slow_enabled)
        );
  }
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
exit:

  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_set_unsafe()",flow_ndx,0);
}



/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_OUT ARAD_SCH_FLOW           *flow
  )
{
  uint32
    nof_subflows,
    res = SOC_SAND_OK;
  ARAD_SCH_GLOBAL_PER1K_INFO
    per1k_info;
  uint8
    is_invalid,
    is_odd_even = FALSE,
    is_aggregate = FALSE;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(flow);
  arad_ARAD_SCH_FLOW_clear(unit, flow);

  /* verify flow id */
  res = arad_sch_flow_id_verify_unsafe(
            unit,
            flow_ndx
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(core, SOC_DPP_DEFS_GET(unit, nof_cores) , ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = arad_sch_flow_is_deleted_get_unsafe(
          unit, core,
          flow_ndx,
          &is_invalid
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  flow->sub_flow[0].is_valid = (is_invalid)?FALSE:TRUE;

  if(!flow->sub_flow[0].is_valid)
  {
    arad_ARAD_SCH_FLOW_clear(unit, flow);
  }
  res = arad_sch_per1k_info_get(
      unit, core,
      ARAD_SCH_FLOW_TO_1K_ID(flow_ndx),
      &per1k_info
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  is_odd_even = per1k_info.is_odd_even;
  /*
   *  Get flow <=> sub flows mapping of the base flow id.
   *  It is flow_ndx itself if it is base-flow, or the flow-d
   *  of the spouse flow.
   */
  res = arad_sch_flow_nof_subflows_get(
          unit, core,
          flow_ndx,
          &nof_subflows
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (nof_subflows > 1)
  {
    /*
     *  This is a composite flow
     */
    if (arad_sch_is_sub_flow_second(flow_ndx, is_odd_even))
    {
      flow->flow_type = ARAD_FLOW_NONE;
      goto exit;
    }
  }
  /*
   * If the flow ID is within the aggregate range, check if it is indeed an
   * aggregate flow or a regular one
   */
  if (arad_sch_is_flow_id_se_id(unit, flow_ndx))
  {
    res = arad_sch_se_state_get(
            unit, core,
            arad_sch_flow2se_id(unit, flow_ndx),
            &is_aggregate
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    flow->flow_type  = is_aggregate?ARAD_FLOW_AGGREGATE:ARAD_FLOW_SIMPLE;
  }
  else
  {
    flow->flow_type  = ARAD_FLOW_SIMPLE;
  }

  /*
   * Read sub-flow/s properties (credit source, QoS params, shaper)
   * from FDMS & SHDS
   */

  if (nof_subflows == ARAD_SCH_NOF_SUB_FLOWS)
  {
    /* composite flow */
    flow->sub_flow[0].id = flow_ndx;
    flow->sub_flow[1].id = arad_sch_sub_flow_spouse_id(
                             flow_ndx,
                             is_odd_even
                           );

    res = arad_sch_flow_is_deleted_get_unsafe(
            unit, core,
            flow->sub_flow[1].id,
            &is_invalid
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

    flow->sub_flow[1].is_valid = SOC_SAND_NUM2BOOL_INVERSE(is_invalid);

    if(flow->sub_flow[0].is_valid)
    {
      res = arad_sch_flow_subflow_get(
            unit, core,
              &flow->sub_flow[0]
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      res = arad_sch_flow_subflow_get(
              unit, core,
              &flow->sub_flow[1]
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }
  }
  else
  {
    /* single (not composite) flow */
    flow->sub_flow[0].id = flow_ndx;

    if(flow->sub_flow[0].is_valid)
    {
      res = arad_sch_flow_subflow_get(
              unit, core,
              &flow->sub_flow[0]
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    }

    arad_ARAD_SCH_SUBFLOW_clear(
            unit,
            &flow->sub_flow[1]
          );
  }

  if (!is_aggregate)
  {
    arad_sch_flow_slow_enable_get(
      unit, core,
      flow_ndx,
      &flow->is_slow_enabled
    );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
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
    res = arad_sch_flow_verify_unsafe(
            unit, core,
            flow_ndx,
            flow
          );
    if(soc_sand_get_error_code_from_error_word(res) != SOC_SAND_OK)
    {
      flow->flow_type = ARAD_FLOW_NONE;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_get_unsafe()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_max_burst_save_and_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID    flow_id,
    SOC_SAND_IN  uint32              new_max_burst,
    SOC_SAND_OUT uint32              *old_max_burst
  )
{
    uint32 res = SOC_SAND_OK;
    ARAD_SCH_SHDS_TBL_DATA shds_tbl_data;
    uint32              offset_shds;
    

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_MAX_BURST_SAVE_AND_SET);

    sal_memset(&shds_tbl_data, 0x0, sizeof(shds_tbl_data));
    offset_shds = flow_id/2;

    /* get old shds configuration in order to save the old burst and keep other configurations */
    res = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_sch_shds_tbl_get_unsafe, (unit, core, offset_shds, &shds_tbl_data)) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 1, exit);

    if (flow_id % 2 == 0)
    {
      *old_max_burst = shds_tbl_data.max_burst_even;
      shds_tbl_data.max_burst_even = new_max_burst;
      shds_tbl_data.max_burst_update_even = 0x1;
    }
    else
    {
      *old_max_burst = shds_tbl_data.max_burst_odd;
      shds_tbl_data.max_burst_odd = new_max_burst;
      shds_tbl_data.max_burst_update_odd = 0x1;
    }

    res = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_sch_shds_tbl_set_unsafe, (unit, core, offset_shds, &shds_tbl_data)) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 1, exit);

exit:

  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_max_burst_save_and_set()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_status_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW_STATUS    state
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_STATUS_VERIFY);

  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_status_verify()",0,0);
}

/*********************************************************************
*     Set flow state to off/on. The state of the flow will be
*     updated, unless was configured otherwise. Note: useful
*     for virtual flows, for which the flow state must be
*     explicitly set
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_sch_flow_status_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW_STATUS    state
  )
{
  uint32
    offset,
    res;
  ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_DATA
    fsm_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_SCH_FLOW_STATUS_SET_UNSAFE);
  /*
   * Write to the FSM table.
   * This is a one-entry table, so the offset is always 0
   */
  offset = 0;
  fsm_data.message_flow_id = flow_ndx;
  fsm_data.message_type = state;

  res = arad_sch_force_status_message_tbl_set_unsafe(
          unit, core,
          offset,
          &fsm_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_sch_flow_status_set_unsafe()",0,0);
}

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */
