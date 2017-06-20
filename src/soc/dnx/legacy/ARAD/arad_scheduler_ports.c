#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_scheduler_ports.c,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COSQ

#include <soc/mem.h>


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/mbcm.h>

#include <soc/dnx/legacy/ARAD/arad_scheduler_ports.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_elements.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_end2end.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>

#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Utils/sand_conv.h>

#include <soc/dnx/legacy/port_sw_db.h>

#include <shared/swstate/access/sw_state_access.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_SCH_TCG_NDX_DEFAULT                    (0)
#define JER2_ARAD_SCH_SINGLE_MEMBER_TCG_START            (4)
#define JER2_ARAD_SCH_SINGLE_MEMBER_TCG_END              (7)
#define JER2_ARAD_SCH_TCG_WEIGHT_MIN                     (0)
#define JER2_ARAD_SCH_TCG_WEIGHT_MAX                     (1023)
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


/*****************************************************
* NAME
*    jer2_arad_sch_hr_lowest_hp_class_select_get
* TYPE:
*   PROC
* DATE:
*   13/11/2007
* FUNCTION:
*   Get selected hp class out of the available configurations
*   (access device, SHC)
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     int             core -
*     Identifier of core on device to access.
*   DNX_SAND_IN  JER2_ARAD_SCH_PORT_ID  tm_port -
*     Port index. Range: 0 - 63.
*   DNX_SAND_IN  uint32           hp_class_conf_idx -
*     Selects which of the 4 available high priority class configurations
*     will be applied to the specified port. Range 0-3.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
static uint32
  jer2_arad_sch_hr_lowest_hp_class_select_get(
    DNX_SAND_IN  int                          unit,
    DNX_SAND_IN  int                          core,
    DNX_SAND_IN  uint32                       tm_port,
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_LOWEST_HP_HR_CLASS  *hp_class_conf_idx
  )
{
  uint32
    offset,
    res;
  JER2_ARAD_SCH_SHC_TBL_DATA
    shc_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_HR_LOWEST_HP_CLASS_SELECT_GET);

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  /*
   * Read indirect from SHC table
   */

  offset  = tm_port;

  res = jer2_arad_sch_shc_tbl_get_unsafe(
          unit, core,
          offset,
          &shc_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  *hp_class_conf_idx = shc_tbl_data.hrmask_type;

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_hr_lowest_hp_class_select_get", 0, 0);
}

/*****************************************************
* NAME
*   jer2_arad_sch_hr_to_port_assign_set
* TYPE:
*   PROC
* DATE:
*   26/12/2007
* FUNCTION:
*   Assign HR scheduling element to port.
*   This will direct port credits to the HR.
* INPUT:
*   DNX_SAND_IN  JER2_ARAD_SCH_PORT_ID  port_ndx -
*     The index of the port to set.
*     Range: 0 - 79
*   DNX_SAND_IN  uint8           is_port_hr -
*     If TRUE, the HR will be assigned to the port.
*     Otherwise - unasigned.
*     HR that is not assigned to port can be used as
*     HR scheduler.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_hr_to_port_assign_set(
    DNX_SAND_IN  int           unit,
    DNX_SAND_IN  int           core,
    DNX_SAND_IN  uint32        tm_port,
    DNX_SAND_IN  uint8         is_port_hr
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    start_idx,
    reg_idx,
    fld_val = 0,
    is_port_hr_val;
  uint32
    base_port_tc,
    nof_priorities,
    priority_i;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_HR_TO_PORT_ASSIGN_SET); 

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);

  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);

  is_port_hr_val = DNX_SAND_BOOL2NUM(is_port_hr);

  /* 
   *  Force flow control on HR before disabling HR. This is a workaround for Dynamic port during traffic
   *  that leads to scheduler being stuck due to toggling of SCH_PORT_ENABLE_PORTEN.
   */
  if (!is_port_hr_val)
  {
      reg_idx = JER2_ARAD_REG_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_FORCE_FC_REG_LINE);

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 900, exit, READ_SCH_FORCE_PORT_FC_REGISTERr(unit, core, reg_idx, &fld_val));
      
      start_idx = JER2_ARAD_FLD_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_FORCE_FC_REG_LINE);
       
      /* Enable HR flow control for all related q-pairs */
      for (priority_i = 0; priority_i < nof_priorities; priority_i++)
      {
        /* No coverity here, the shift value can't be greater than 32 */
        /* coverity[large_shift:FALSE] */              
        DNX_SAND_SET_BIT(fld_val, 1, start_idx+priority_i);
      }

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 905, exit, WRITE_SCH_FORCE_PORT_FC_REGISTERr(unit, core, reg_idx, fld_val));
  }

  /* enable/disble HR as port scheduler */
  {
      fld_val = 0;
      reg_idx = JER2_ARAD_REG_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_SCH_PORT_ENABLE_PORTENm(unit,  SCH_BLOCK(unit, core), reg_idx, &fld_val));
      
      start_idx = JER2_ARAD_FLD_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);
       
      /* Set HR enable for all related q-pairs */
      for (priority_i = 0; priority_i < nof_priorities; priority_i++)
      {
        /* No coverity here, the shift value can't be greater than 32 */
        /* coverity[large_shift:FALSE] */               
        DNX_SAND_SET_BIT(fld_val, is_port_hr_val, start_idx+priority_i);
      }
      /* Access WRITE_SCH_PORT_ENABLE_PORTEN (jer2_jer/jer2_arad) - Access specific core. */
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_SCH_PORT_ENABLE_PORTENm(unit, SCH_BLOCK(unit,core), reg_idx, &fld_val));
  }


  /* 
   *  Disable force flow control on HR after enabling HR. This is a workaround for Dynamic port during traffic
   *  that leads to scheduler being stuck due to toggling of SCH_PORT_ENABLE_PORTEN.
   */
  if (is_port_hr_val)
  {
      fld_val = 0;
      reg_idx = JER2_ARAD_REG_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_FORCE_FC_REG_LINE);

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 900, exit, READ_SCH_FORCE_PORT_FC_REGISTERr(unit, core, reg_idx, &fld_val));

      start_idx = JER2_ARAD_FLD_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_FORCE_FC_REG_LINE);

      /* Disable HR flow control for all related q-pairs */
      for (priority_i = 0; priority_i < nof_priorities; priority_i++)
      {    
        /* No coverity here, the shift value can't be greater than 32 */
        /* coverity[large_shift:FALSE] */ 
        DNX_SAND_SET_BIT(fld_val, 0, start_idx+priority_i);
      }

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 905, exit, WRITE_SCH_FORCE_PORT_FC_REGISTERr(unit, core, reg_idx, fld_val));
  }


exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_hr_to_port_assign_set", 0, 0);
}

/*****************************************************
* NAME
*   jer2_arad_sch_hr_tcg_map_set
* TYPE:
*   PROC
* DATE:
*   26/12/2007
* FUNCTION:
*   Assign HR scheduling element to TCG specific group.
* INPUT:
*   DNX_SAND_IN  JER2_ARAD_SCH_SE_ID     se_id -
*     The index of the scheduler index to set. Must be an HR Port-Priority.
*     Range: 0 - 32K-1
*   DNX_SAND_IN  JER2_ARAD_TCG_NDX       tcg_ndx -
*     TCG (Traffic class groups) that the HR is mapped to.
*     Range: 0 - 7.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_hr_tcg_map_set(
    DNX_SAND_IN  int           unit,
    DNX_SAND_IN  int           core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID      se_id,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX        tcg_ndx
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    start_idx,
    table_ndx,
    data,
    fld_val = 0;
  uint32
    hr_ndx,
    dummy_tc,
    port_id;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_HR_TO_PORT_ASSIGN_SET); 

  res = jer2_arad_sch_se_id_verify_unsafe(
          unit,
          se_id
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (DNX_SAND_IS_VAL_OUT_OF_RANGE(
       se_id, JER2_ARAD_HR_SE_ID_MIN, JER2_ARAD_HR_SE_ID_MIN + JER2_ARAD_SCH_MAX_PORT_ID))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_SE_HR_ID_ERR, 20, exit);
  }

  /* Validate HR given is enabled HR Port Priority and not just a reserved one */
  res = jer2_arad_sch_se2port_tc_id_get_unsafe(
          unit, core,
          se_id,
          &port_id,
          &dummy_tc
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (port_id == JER2_ARAD_SCH_PORT_ID_INVALID)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR, 35, exit);
  }

  /* HR index: out of 256 */
  hr_ndx = se_id - JER2_ARAD_HR_SE_ID_MIN;

  table_ndx = JER2_ARAD_REG_IDX_GET(hr_ndx, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);
  start_idx = JER2_ARAD_FLD_IDX_GET(hr_ndx, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);
  
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_SCH_PORT_SCHEDULER_MAP_PSMm(unit, SCH_BLOCK(unit,core), table_ndx, &data));
  fld_val = soc_SCH_PORT_SCHEDULER_MAP_PSMm_field32_get(unit,&data,TC_PG_MAPf);

  res = dnx_sand_bitstream_set_any_field(&tcg_ndx,start_idx * JER2_ARAD_NOF_TCG_IN_BITS,JER2_ARAD_NOF_TCG_IN_BITS,&fld_val);
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  soc_SCH_PORT_SCHEDULER_MAP_PSMm_field32_set(unit,&data,TC_PG_MAPf,fld_val);  
  /* Set tcg index */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_SCH_PORT_SCHEDULER_MAP_PSMm(unit, SCH_BLOCK(unit,core), table_ndx, &data));    

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_hr_tcg_map_set", 0, 0);
}

/*****************************************************
* NAME
*   jer2_arad_sch_hr_tcg_map_get
* TYPE:
*   PROC
* DATE:
*   26/12/2007
* FUNCTION:
*   Retreive HR scheduling element to TCG specific group.
* INPUT:
*   DNX_SAND_IN  JER2_ARAD_SCH_SE_ID     se_id -
*     The index of the scheduler index to set. Must be an HR Port-Priority.
*     Range: 0 - 32K-1
*   DNX_SAND_IN  JER2_ARAD_TCG_NDX       tcg_ndx -
*     TCG (Traffic class groups) that the HR is mapped to.
*     Range: 0 - 7.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
static uint32
  jer2_arad_sch_hr_tcg_map_get(
    DNX_SAND_IN  int           unit,
    DNX_SAND_IN  int           core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID      se_id,
    DNX_SAND_OUT JER2_ARAD_TCG_NDX       *tcg_ndx
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    start_idx,
    table_ndx,
    data,
    fld_val = 0;
  uint32
    hr_ndx,
    dummy_tc,
    port_id;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0); 

  res = jer2_arad_sch_se_id_verify_unsafe(
          unit,
          se_id
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (DNX_SAND_IS_VAL_OUT_OF_RANGE(
       se_id, JER2_ARAD_HR_SE_ID_MIN, JER2_ARAD_HR_SE_ID_MIN + JER2_ARAD_SCH_MAX_PORT_ID))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_SE_HR_ID_ERR, 20, exit);
  }

  /* Validate HR given is enabled HR Port Priority and not just a reserved one */
  res = jer2_arad_sch_se2port_tc_id_get_unsafe(
          unit, core,
          se_id,
          &port_id,
          &dummy_tc
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (port_id == JER2_ARAD_SCH_PORT_ID_INVALID)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR, 35, exit);
  }

  /* HR index: out of 256 */
  hr_ndx = se_id - JER2_ARAD_HR_SE_ID_MIN;

  table_ndx = JER2_ARAD_REG_IDX_GET(hr_ndx, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);
  start_idx = JER2_ARAD_FLD_IDX_GET(hr_ndx, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);
  
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_SCH_PORT_SCHEDULER_MAP_PSMm(unit, SCH_BLOCK(unit,core), table_ndx, &data));
  fld_val = soc_SCH_PORT_SCHEDULER_MAP_PSMm_field32_get(unit,&data,TC_PG_MAPf);

  res = dnx_sand_bitstream_get_any_field(&fld_val,start_idx * JER2_ARAD_NOF_TCG_IN_BITS,JER2_ARAD_NOF_TCG_IN_BITS,tcg_ndx);
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
 
exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_hr_tcg_map_get", 0, 0);
}

/*****************************************************
* NAME
*   jer2_arad_sch_hr_to_port_assign_set
* TYPE:
*   PROC
* DATE:
*   26/12/2007
* FUNCTION:
*   Check if an HR scheduling element is assigned to port.
*   This will direct port credits to the HR.
* INPUT:
*   DNX_SAND_IN  JER2_ARAD_SCH_PORT_ID  port_ndx -
*     The index of the port to set.
*     Range: 0 - 79
*   DNX_SAND_OUT  uint8           is_port_hr -
*     If TRUE, the HR is assigned to the port.
*     Otherwise - unasigned.
*     HR that is not assigned to port can be used as
*     HR scheduler.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_hr_to_port_assign_get(
    DNX_SAND_IN  int           unit,
    DNX_SAND_IN  int           core,
    DNX_SAND_IN  uint32        tm_port,
    DNX_SAND_OUT uint8         *is_port_hr
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    fld_idx,
    reg_idx,
    fld_val = 0,
    is_port_hr_val;
  uint32
    base_port_tc;
   
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_HR_TO_PORT_ASSIGN_GET);

  res = jer2_arad_sch_port_id_verify_unsafe(
          unit,
          tm_port
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  reg_idx = JER2_ARAD_REG_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);
  fld_idx = JER2_ARAD_FLD_IDX_GET(base_port_tc, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1110, exit, READ_SCH_PORT_ENABLE_PORTENm(unit, SCH_BLOCK(unit, core), reg_idx, &fld_val));

  /* Assuming first is enable, all HRs related are enabled */
  is_port_hr_val = DNX_SAND_GET_BIT(fld_val, fld_idx);
  *is_port_hr = DNX_SAND_NUM2BOOL(is_port_hr_val);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_hr_to_port_assign_get", 0, 0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_sched_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_SCH_PORT_INFO  *port_info
  )
{
  uint32
    res;
  uint32
    priority_i,
    nof_priorities,
    tcg_i;
  uint8
    is_one_member;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_SCHED_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(port_info);

  res = jer2_arad_sch_port_id_verify_unsafe(
          unit,
          tm_port
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities); 
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /* No support in JER2_ARAD */
  DNX_SAND_ERR_IF_OUT_OF_RANGE(
      port_info->lowest_hp_class, DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_LAST, DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_LAST,
      JER2_ARAD_SCH_HP_CLASS_OUT_OF_RANGE_ERR, 30, exit
    );

  DNX_SAND_ERR_IF_NOT_EQUALS_VALUE(
      port_info->hr_mode, DNX_TMC_SCH_SE_HR_MODE_LAST,
      JER2_ARAD_SCH_HR_MODE_INVALID_ERR, 35, exit
    );


  for (priority_i = 0; priority_i < nof_priorities; priority_i++)
  {
    DNX_SAND_ERR_IF_OUT_OF_RANGE(
        port_info->hr_modes[priority_i], DNX_TMC_SCH_HR_MODE_NONE, DNX_TMC_SCH_HR_MODE_ENHANCED_PRIO_WFQ,
        JER2_ARAD_SCH_HR_MODE_INVALID_ERR, 35, exit
      );    
  }

  if (nof_priorities == JER2_ARAD_TCG_NOF_PRIORITIES_SUPPORT)
  {
    /* TCG enable */
    for (priority_i = 0; priority_i < nof_priorities; priority_i++) 
    {
      /*
       * COVERITY
       *
       * JER2_ARAD_TCG_MIN may be changed to be bigger than 0.
       */
      /* coverity[unsigned_compare] */
      DNX_SAND_ERR_IF_OUT_OF_RANGE(
          port_info->tcg_ndx[priority_i], JER2_ARAD_TCG_MIN, JER2_ARAD_TCG_MAX, 
          JER2_ARAD_TCG_OUT_OF_RANGE_ERR, 35, exit
        );
    }

    /* Check each single member TCG that only one priority is mapped to { */
    for (tcg_i = JER2_ARAD_SCH_SINGLE_MEMBER_TCG_START; tcg_i <= JER2_ARAD_SCH_SINGLE_MEMBER_TCG_END; tcg_i++)
    {
      is_one_member = FALSE;
      for (priority_i = 0; priority_i < nof_priorities; priority_i++) 
      {
        if (port_info->tcg_ndx[priority_i] == tcg_i)
        {
          if (is_one_member)
          {
            /* More than one member set to this tcg */
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TCG_SINGLE_MEMBER_ERR, 100+tcg_i, exit);
          }
          else
          {
            is_one_member = TRUE;
          }
        }     
      }
    }
    /* Check each single member TCG that only one priority is mapped to } */
  }
  else
  {
    /* Verify all tcgs are mapped to default */
    for (priority_i = 0; priority_i < nof_priorities; priority_i++) {
      DNX_SAND_ERR_IF_NOT_EQUALS_VALUE(
        port_info->tcg_ndx[priority_i], JER2_ARAD_SCH_TCG_NDX_DEFAULT,
        JER2_ARAD_TCG_NOT_SUPPORTED_ERR, 110, exit
      );
    }
  }

  if (port_info->enable == FALSE)
  {
    DNX_SAND_ERR_IF_ABOVE_MAX(
      port_info->max_expected_rate, JER2_ARAD_IF_MAX_RATE_MBPS(unit),
      JER2_ARAD_SCH_SE_PORT_RATE_OUT_OF_RANGE_ERR, 140, exit
    );
  }
  else
  {
    DNX_SAND_ERR_IF_OUT_OF_RANGE(
      port_info->max_expected_rate, 1, JER2_ARAD_IF_MAX_RATE_MBPS(unit),
      JER2_ARAD_SCH_SE_PORT_RATE_OUT_OF_RANGE_ERR, 150, exit
    );
  }

  if (port_info->group == JER2_ARAD_SCH_GROUP_NONE)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_GROUP_ERR, 160, exit)
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_sched_verify()",0,0);
}

/*********************************************************************
* NAME:
*     jer2_arad_sch_port_tcg_weight_set/get _unsafe
* TYPE:
*   PROC
* DATE:
*  
* FUNCTION:
*     Sets, for a specified TCG within a certain Port
*     its excess rate. Excess traffic is scheduled between other TCGs
*     according to a weighted fair queueing or strict priority policy. 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_PORT_ID          port_id -
*     Port id, 0 - 255. Set invalid in case of invalid attributes.
*  DNX_SAND_IN  JER2_ARAD_TCG_NDX              tcg_ndx -
*     TCG index. 0-7.
*  DNX_SAND_IN  JER2_ARAD_SCH_TCG_WEIGHT      *tcg_weight -
*     TCG weight information.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   This function must only be called for eight priorities port.
*********************************************************************/
uint32
  jer2_arad_sch_port_tcg_weight_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX        tcg_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_TCG_WEIGHT *tcg_weight
  )
{
  uint32
    res,
    data,
    is_tcg_weight_val,
    field_val;
  uint32
    base_port_tc,
    ps;
  soc_reg_above_64_val_t
    data_above_64;
  soc_field_t
    field_name;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_TCG_WEIGHT_SET_UNSAFE);
  
  SOC_REG_ABOVE_64_CLEAR(data_above_64);


  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  ps = JER2_ARAD_BASE_PORT_TC2PS(base_port_tc);
#else 
  ps = base_port_tc;
#endif 

  /* Set TCG weight valid { */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_SCH_PORT_SCHEDULER_MAP_PSMm(unit, SCH_BLOCK(unit, core), ps, &data));
  field_val = soc_SCH_PORT_SCHEDULER_MAP_PSMm_field32_get(unit,&data,PG_WFQ_VALIDf);
 
  is_tcg_weight_val = tcg_weight->tcg_weight_valid ? 1:0; 
  DNX_SAND_SET_BIT(field_val, is_tcg_weight_val, tcg_ndx);
  
  soc_SCH_PORT_SCHEDULER_MAP_PSMm_field32_set(unit,&data,PG_WFQ_VALIDf,field_val);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_SCH_PORT_SCHEDULER_MAP_PSMm(unit, SCH_BLOCK(unit, core), ps, &data));    
  /* Set TCG weight valid } */

  /* Set TCG weight { */
  if (tcg_weight->tcg_weight_valid)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1020, exit, READ_SCH_PORT_SCHEDULER_WEIGHTS_PSWm(unit, SCH_BLOCK(unit, core), ps, data_above_64));
    switch (tcg_ndx)
    {
    case 0:
      field_name = WFQ_PG_0_WEIGHTf;
      break;
    case 1:
      field_name = WFQ_PG_1_WEIGHTf;
      break;
    case 2:
      field_name = WFQ_PG_2_WEIGHTf;
      break;
    case 3:
      field_name = WFQ_PG_3_WEIGHTf;
      break;
    case 4:
      field_name = WFQ_PG_4_WEIGHTf;
      break;
    case 5:
      field_name = WFQ_PG_5_WEIGHTf;
      break;
    case 6:
      field_name = WFQ_PG_6_WEIGHTf;
      break;
    case 7:
      field_name = WFQ_PG_7_WEIGHTf;
      break;
    default:
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TCG_OUT_OF_RANGE_ERR, 50, exit);
    }

    field_val = tcg_weight->tcg_weight;

    soc_SCH_PORT_SCHEDULER_WEIGHTS_PSWm_field32_set(unit,data_above_64,field_name,field_val);

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1030, exit, WRITE_SCH_PORT_SCHEDULER_WEIGHTS_PSWm(unit, SCH_BLOCK(unit, core), ps, data_above_64));    
  }
  /* Set TCG weight } */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_tcg_weight_set_unsafe()",tm_port,tcg_ndx);
}

uint32
  jer2_arad_sch_port_tcg_weight_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX        tcg_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_TCG_WEIGHT *tcg_weight
  )
{
  uint32
    res,
    data,
    is_tcg_weight_val,
    field_val;
  uint32
    base_port_tc,
    ps;
  soc_reg_above_64_val_t
    data_above_64;
  soc_field_t
    field_name;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_TCG_WEIGHT_GET_UNSAFE);
  
  SOC_REG_ABOVE_64_CLEAR(data_above_64);

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  ps = JER2_ARAD_BASE_PORT_TC2PS(base_port_tc);
#else 
  ps = base_port_tc;
#endif 


  /* Get TCG weight valid { */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_SCH_PORT_SCHEDULER_MAP_PSMm(unit, SCH_BLOCK(unit, core), ps, &data));
  field_val = soc_SCH_PORT_SCHEDULER_MAP_PSMm_field32_get(unit,&data,PG_WFQ_VALIDf);
 
  is_tcg_weight_val = DNX_SAND_GET_BIT(field_val, tcg_ndx);
  tcg_weight->tcg_weight_valid = DNX_SAND_NUM2BOOL(is_tcg_weight_val);
  /* Get TCG weight valid } */

  /* Get TCG weight { */
  if (tcg_weight->tcg_weight_valid)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1020, exit, READ_SCH_PORT_SCHEDULER_WEIGHTS_PSWm(unit, SCH_BLOCK(unit, core), ps, data_above_64));
    switch (tcg_ndx)
    {
    case 0:
      field_name = WFQ_PG_0_WEIGHTf;
      break;
    case 1:
      field_name = WFQ_PG_1_WEIGHTf;
      break;
    case 2:
      field_name = WFQ_PG_2_WEIGHTf;
      break;
    case 3:
      field_name = WFQ_PG_3_WEIGHTf;
      break;
    case 4:
      field_name = WFQ_PG_4_WEIGHTf;
      break;
    case 5:
      field_name = WFQ_PG_5_WEIGHTf;
      break;
    case 6:
      field_name = WFQ_PG_6_WEIGHTf;
      break;
    case 7:
      field_name = WFQ_PG_7_WEIGHTf;
      break;
    default:
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TCG_OUT_OF_RANGE_ERR, 50, exit);
    }

    field_val = soc_SCH_PORT_SCHEDULER_WEIGHTS_PSWm_field32_get(unit,data_above_64,field_name);
    tcg_weight->tcg_weight = field_val;
  }
  /* Get TCG weight } */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_tcg_weight_get_unsafe()",tm_port,tcg_ndx);
}

uint32
  jer2_arad_sch_port_tcg_weight_set_verify_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  uint32                    tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX              tcg_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_TCG_WEIGHT       *tcg_weight
  )
{
  uint32
    res;
  uint32
    base_port_tc,
    nof_priorities;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_TCG_WEIGHT_SET_VERIFY_UNSAFE);
  
  /* Verify port */
  res = jer2_arad_sch_port_id_verify_unsafe(
          unit,
          tm_port
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);

  if (base_port_tc == JER2_ARAD_SCH_PORT_ID_INVALID)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR, 17, exit)
  }

  /* API functionality only when port is with 8 priorities. */
  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities); 
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 19, exit);

  if (!(nof_priorities == JER2_ARAD_TCG_NOF_PRIORITIES_SUPPORT))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TCG_NOT_SUPPORTED_ERR, 20, exit);
  }

  /* Verify TCG */
/*
 * COVERITY
 *
 * JER2_ARAD_TCG_MIN may be changed to be bigger than 0.
 */
/* coverity[unsigned_compare] */
  DNX_SAND_ERR_IF_OUT_OF_RANGE(
          tcg_ndx, JER2_ARAD_TCG_MIN, JER2_ARAD_TCG_MAX, 
          JER2_ARAD_TCG_OUT_OF_RANGE_ERR, 35, exit
        );

  /* Verify TCG weight */
  if (tcg_weight->tcg_weight_valid)
  {
    DNX_SAND_ERR_IF_OUT_OF_RANGE(
            tcg_weight->tcg_weight, JER2_ARAD_SCH_TCG_WEIGHT_MIN, JER2_ARAD_SCH_TCG_WEIGHT_MAX, 
            JER2_ARAD_SCH_TCG_WEIGHT_OUT_OF_RANGE_ERR, 35, exit
          );
  }
  
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_tcg_weight_set_verify_unsafe()",tm_port,tcg_ndx);
}

uint32
  jer2_arad_sch_port_tcg_weight_get_verify_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX        tcg_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_TCG_WEIGHT *tcg_weight
  )
{
  uint32
    res;
  uint32
    base_port_tc,
    nof_priorities;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_TCG_WEIGHT_GET_VERIFY_UNSAFE);
  
  /* Verify port */
  res = jer2_arad_sch_port_id_verify_unsafe(
          unit,
          tm_port
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);


  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);

  if (base_port_tc == JER2_ARAD_SCH_PORT_ID_INVALID)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR, 17, exit)
  }

  /* API functionality only when port is with 8 priorities. */

  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities); 
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 19, exit);

  if (!(nof_priorities == JER2_ARAD_TCG_NOF_PRIORITIES_SUPPORT))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TCG_NOT_SUPPORTED_ERR, 20, exit);
  }

  /* Verify TCG */
/*
 * COVERITY
 *
 * JER2_ARAD_TCG_MIN may be changed to be bigger than 0.
 */
/* coverity[unsigned_compare] */
  DNX_SAND_ERR_IF_OUT_OF_RANGE(
          tcg_ndx, JER2_ARAD_TCG_MIN, JER2_ARAD_TCG_MAX, 
          JER2_ARAD_TCG_OUT_OF_RANGE_ERR, 35, exit
        );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_tcg_weight_get_verify_unsafe()",tm_port, tcg_ndx);
}

/*********************************************************************
*     Sets the scheduler-port state (enable/disable), and its
*     HR mode of operation (single or dual). The driver writes
*     to the following tables: Scheduler Enable Memory (SEM),
*     HR-Scheduler-Configuration (SHC), Flow Group Memory
*     (FGM)
*     Details: in the H file. (search for prototype)
*
*     See also: jer2_arad_sch_port_sched_get_unsafe()
*********************************************************************/
uint32
  jer2_arad_sch_port_sched_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  JER2_ARAD_SCH_PORT_INFO  *port_info
  )
{
  uint32
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_PORT_HP_CLASS_INFO
    hp_class_info;
  JER2_ARAD_SCH_SE_INFO
    se;
  uint32 
    nof_priorities,
    priority_i,
    group;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_SCHED_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(port_info);

  res = jer2_arad_sch_port_id_verify_unsafe(
          unit,
          tm_port
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities); 
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 13, exit);
      
  /*
   *  Assign scheduler group {
   */
  if (port_info->enable == FALSE)
  {
    res = jer2_arad_sch_hr_to_port_assign_set(
            unit, core,
            tm_port,
            FALSE
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 16, exit);

    /* Go over all HRs (per priority) to disable them */    
    for (priority_i = 0; priority_i < nof_priorities; priority_i++)
    {
      res = jer2_arad_sch_port_tc2se_id(
              unit, core,
              tm_port,
              priority_i,
              &(se.id)
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit);

      res = jer2_arad_sch_se_state_set(
            unit, core,
            se.id,
            FALSE
          );
      DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);

      if (nof_priorities == JER2_ARAD_TCG_NOF_PRIORITIES_SUPPORT)
      {
        /* By default, Map SE to TCG 0, in case of 8 priorities */
        res = jer2_arad_sch_hr_tcg_map_set(
                unit,
                core,
                se.id,
                JER2_ARAD_SCH_TCG_NDX_DEFAULT
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);
      }

    }
  }
  else
  {

    /* all the HRs in a port will have the same group (limitation cause of dual shapers) */
    res = jer2_arad_sch_port_hr_group_get(unit, core, tm_port, &group); /* get the new group */
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    for (priority_i = 0; priority_i < nof_priorities; priority_i++)
    {
      res = jer2_arad_sch_port_tc2se_id(
              unit, core,
              tm_port,
              priority_i,
              &(se.id)
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit);

      se.state = (port_info->enable == TRUE)?JER2_ARAD_SCH_SE_STATE_ENABLE:JER2_ARAD_SCH_SE_STATE_DISABLE;
      se.is_dual = FALSE;
      se.type = JER2_ARAD_SCH_SE_TYPE_HR;
      se.type_info.hr.mode = port_info->hr_modes[priority_i];

      /*
       * The port HR scheduler element group is set here
       * based on port info, not on se info.
       */
      se.group = JER2_ARAD_SCH_GROUP_NONE;

      res = jer2_arad_sch_se_set_unsafe(
              unit, core,
              &se,
              1
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 13, exit);
      
      /* set new group for each HR in */
      res = jer2_arad_sch_se_group_set(unit, core, se.id, group);
      DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);

      /* Map SE to TCG, only in case of 8 priorities */
      if (nof_priorities == JER2_ARAD_TCG_NOF_PRIORITIES_SUPPORT)
      {
        res = jer2_arad_sch_hr_tcg_map_set(
                unit,
                core,
                se.id,
                port_info->tcg_ndx[priority_i]
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);
      }
    }

    jer2_arad_JER2_ARAD_SCH_PORT_HP_CLASS_INFO_clear(&hp_class_info);

  /*
   *  Assign scheduler group }
   */

    res = jer2_arad_sch_hr_to_port_assign_set(
            unit, core,
            tm_port,
            TRUE
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 16, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_sched_set_unsafe()",0,0);
}



/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_sched_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_INFO  *port_info
  )
{
  uint32
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_SE_INFO
    se;
  JER2_ARAD_SCH_PORT_LOWEST_HP_HR_CLASS
    hp_class_select_idx = 0;
  JER2_ARAD_SCH_PORT_HP_CLASS_INFO
    hp_class_info;
  uint8
    is_port_hr;
  uint32
    nof_priorities,
    priority_i;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_SCHED_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(port_info);

  res = jer2_arad_sch_port_id_verify_unsafe(
          unit,
          tm_port
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = jer2_arad_sch_hr_to_port_assign_get(unit, core, tm_port, &is_port_hr);
  DNX_SAND_CHECK_FUNC_RESULT(res, 25, exit);

  port_info->enable = is_port_hr;

  if (is_port_hr)
  {
    res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);

    for (priority_i = 0; priority_i < nof_priorities; priority_i++)
    {
      res = jer2_arad_sch_port_tc2se_id(unit, core,tm_port,priority_i,&(se.id));
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      /*
       * Read scheduler properties from the device
       */
      res = jer2_arad_sch_se_get_unsafe(
              unit, core,
              se.id,
              &se
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 35, exit);

      if ((se.state == JER2_ARAD_SCH_SE_STATE_DISABLE) && (is_port_hr == TRUE))
      {
        port_info->enable = FALSE;
      }
      
      if (se.type != JER2_ARAD_SCH_SE_TYPE_HR)
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SE_PORT_SE_TYPE_NOT_HR_ERR, 30, exit);
      }

      port_info->hr_modes[priority_i] = se.type_info.hr.mode;
      
      if (nof_priorities == JER2_ARAD_TCG_NOF_PRIORITIES_SUPPORT)
      {
        res = jer2_arad_sch_hr_tcg_map_get(
                unit, core,
                se.id,
                &(port_info->tcg_ndx[priority_i])
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);
      }
    }
  } 

  res = jer2_arad_sch_hr_lowest_hp_class_select_get(
          unit, core,
          tm_port,
          &hp_class_select_idx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  jer2_arad_JER2_ARAD_SCH_PORT_HP_CLASS_INFO_clear(&hp_class_info);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_sched_get_unsafe()",0,0);
}
/*
 * Note that for this procedure, 'core' is the core on which se_id
 * is specified so, within the loop over all local ports, each matched tm_port
 * should also be checked for core match.
 */
uint32
  jer2_arad_sch_se2port_tc_id_get_unsafe(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  int               core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID    se_id,
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_ID  *port_id,
    DNX_SAND_OUT uint32            *tc 
  )
{
  uint32
    port_ndx,
    base_port_tc,
    port_tc_to_check,
    nof_priorities,
    res,
    tm_port,
    flags;
  soc_pbmp_t
    ports_bm;
  int
    loc_core ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if (DNX_SAND_IS_VAL_OUT_OF_RANGE(
       se_id, JER2_ARAD_HR_SE_ID_MIN, JER2_ARAD_HR_SE_ID_MIN + JER2_ARAD_SCH_MAX_PORT_ID))
  {
    *port_id = JER2_ARAD_SCH_PORT_ID_INVALID;
  }
  else
  {
    *port_id = JER2_ARAD_SCH_PORT_ID_INVALID;
    port_tc_to_check = se_id - JER2_ARAD_HR_SE_ID_MIN;

    /* Find match range of base_port_tc <= port_tc_to_check(HR) < base_port_tc + nof_priorities */
    res = dnx_port_sw_db_valid_ports_get(unit, 0, &ports_bm);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    SOC_PBMP_ITER(ports_bm, port_ndx)
    {
        res = dnx_port_sw_db_flags_get(unit, port_ndx, &flags);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);

        if (!(DNX_PORT_IS_ELK_INTERFACE(flags) || DNX_PORT_IS_STAT_INTERFACE(flags) || DNX_PORT_IS_LB_MODEM(flags))) {
            res = dnx_port_sw_db_local_to_tm_port_get(unit, port_ndx, &tm_port, &loc_core);
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 13, exit);

            res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, loc_core, tm_port, &base_port_tc);
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);

            res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, loc_core, tm_port, &nof_priorities); 
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

            if (DNX_SAND_IS_VAL_IN_RANGE(port_tc_to_check,base_port_tc,base_port_tc+nof_priorities-1))
            {
                if (loc_core == core)
                {
                    /* Match */
                    *tc = port_tc_to_check - base_port_tc;
                    *port_id = tm_port;
                    break;
                }
            }
        }
    }    
  }

  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_se2port_tc_id_get_unsafe()",se_id,0);
}


uint32
  jer2_arad_sch_port_tc2se_id_get_unsafe(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  int               core,
    DNX_SAND_IN  uint32            tm_port,
    DNX_SAND_IN  uint32            tc,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_ID    *se_id
  )
{
  uint32
    base_port_tc,
    nof_priorities,
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);
  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities); 
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  if (tm_port == JER2_ARAD_SCH_PORT_ID_INVALID || 
      tm_port > JER2_ARAD_SCH_MAX_PORT_ID ||
      base_port_tc == JER2_ARAD_SCH_PORT_ID_INVALID ||
      tc >= nof_priorities)
  {
    *se_id = JER2_ARAD_SCH_SE_ID_INVALID;
  }
  else
  {
    *se_id = base_port_tc + tc + JER2_ARAD_HR_SE_ID_MIN;
  }

  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_tc2se_id_get_unsafe()",tm_port,tc);
}

/*
 * Convert a rate given in Kbits/sec units to quanta
 * The conversion is done according to:
 *                       
 *  Rate [Kbits/Sec] =   Credit [Kbits] * Num_of_clocks_in_sec [clocks/sec] * quanta [1/clocks] 
 *  
 *                                          1
 *  Where quanta = -------------------------------------------------------------
 *                        interval_between_credits_in_clocks [clocks]
 */
int
  jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(
    DNX_SAND_IN       int       unit,
    DNX_SAND_IN       uint32    rate,     /* in Kbits/sec */
    DNX_SAND_IN       uint32    credit_div,/*REBOUNDED/ASSIGNED_CREDIT*/
    DNX_SAND_IN       uint32    ticks_per_sec,
    DNX_SAND_OUT      uint32*   quanta  /* in device clocks */
  )
{
  uint32
    calc2,
    calc;
  uint32 credit_worth;

  DNXC_INIT_FUNC_DEFS;

  if (NULL == quanta)
  {
    _rv = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }
  if (0 == rate)
  {
    /* Divide by zero */
    *quanta = 0;
    goto exit ;
  }
  /* get credit worth*/
  DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &credit_worth)));

  /* calc = credit_worth [kbits] * ticks_per_sec * 8 / (1000 [clocks/sec] * ASSIGNED_CREDIT) */
   /* full calc = credit_worth [kbits] * 8 * system_frequency(khz)  / (1000 * ASSIGNED_CREDIT*cal_len*acc_period) */
  calc = DNX_SAND_DIV_ROUND(ticks_per_sec*DNX_SAND_NOF_BITS_IN_CHAR,1000);
  calc = DNX_SAND_DIV_ROUND(credit_worth * calc, credit_div); 
  /* calc2 = rate / calc [1/clocks] */
  calc2 = DNX_SAND_DIV_ROUND_UP(rate,calc); 
  if(calc2 != 0){
      *quanta = calc2;
  }
  else{
      *quanta = 1;
  }

exit:
   DNXC_FUNC_RETURN;
}

/* 
*   Convert quanta given in
*   device clocks to rate in Kbits/sec units.
*   The conversion is done according to:
*  Rate [Kbits/Sec] =   Credit [Kbits] * Num_of_clocks_in_sec [clocks/sec] * quanta 
*  
*                                          1
*  Where quanta = -------------------------------------------------------------
*                        interval_between_credits_in_clocks [clocks]
*/
int
  jer2_arad_sch_port_qunta_to_rate_kbits_per_sec(
    DNX_SAND_IN       int       unit,
    DNX_SAND_IN       uint32    quanta, /* in device clocks */
    DNX_SAND_IN       uint32    credit_div,   /*REBOUNDED/ASSIGNED_CREDIT*/
    DNX_SAND_IN       uint32    ticks_per_sec,
    DNX_SAND_OUT      uint32*   rate      /* in Kbits/sec */
  )
{
  DNX_SAND_U64
    calc2;
  uint32
    calc,
    tmp;
  uint32 credit_worth;

  DNXC_INIT_FUNC_DEFS;

  if (NULL == rate)
  {
    _rv = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }
  if (0 == quanta)
  {
    /* Divide by zero */
    _rv = DNX_SAND_DIV_BY_ZERO_ERR ;
    goto exit ;
  }
  /* get credit worth*/
  DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &credit_worth)));

  /* calc = credit_worth [kbits] * ticks_per_sec * 8 / (1000 [clocks/sec] * ASSIGNED_CREDIT) */
  /* full calc = credit_worth [kbits] * 8 * system_frequency(khz)  / (1000 * ASSIGNED_CREDIT*cal_len*acc_period) */
  calc = DNX_SAND_DIV_ROUND(ticks_per_sec*DNX_SAND_NOF_BITS_IN_CHAR,1000);
  calc = DNX_SAND_DIV_ROUND(credit_worth * calc, credit_div);                     
  dnx_sand_u64_multiply_longs(calc, quanta, &calc2);
  if (dnx_sand_u64_to_long(&calc2, &tmp))
  {
    /* Overflow */
    _rv = DNX_SAND_OVERFLOW_ERR ;
    goto exit ;
  }
  *rate = tmp;
exit:
   DNXC_FUNC_RETURN;
}
 

/*********************************************************************
*     Sets, for a specified port_priority 
*     its maximal credit rate. This API is
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_port_priority_shaper_rate_set_unsafe(
    DNX_SAND_IN     int                  unit,
    DNX_SAND_IN     int                  core,
    DNX_SAND_IN     uint32               tm_port,
    DNX_SAND_IN     uint32               priority_ndx,
    DNX_SAND_IN     uint32               rate
  )
{
  uint32
    res,
    quanta,
    nof_ticks,
    quanta_nof_bits,
    credit_div,  
    tbl_data;
  uint32
    base_port_tc,
    offset;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_PRIORITY_SHAPER_RATE_SET_UNSAFE);

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 1, exit);

  res = jer2_arad_sch_calendar_info_get(unit, core, 0, 1, &credit_div, &nof_ticks, &quanta_nof_bits);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 2, exit);


  /*
   * Get Device Interface Max Credit Rate
   */
  if (0 == rate)
  {
    quanta = 0;
  }
  else
  {    
    /* 3. calculate quanta */
    res = jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(
            unit,
            rate,
            credit_div,
            nof_ticks,
            &quanta
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 3, exit);

    DNX_SAND_LIMIT_FROM_ABOVE(quanta, DNX_SAND_BITS_MASK(quanta_nof_bits-1,0));
  }

  offset = base_port_tc + priority_ndx;

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
  soc_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm_field32_set(unit,&tbl_data,QUANTA_TO_ADDf,quanta);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1140, exit, WRITE_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));

  /* update sw db */
  res = jer2_arad_sw_db_sch_port_tcg_rate_set(unit, core, offset, rate, 1);
  DNX_SAND_CHECK_FUNC_RESULT(res, 550, exit);
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_priority_shaper_rate_set_unsafe()",0,0);
}

uint32 
jer2_arad_sch_port_priority_shaper_max_burst_set_unsafe(
    DNX_SAND_IN     int                  unit,
    DNX_SAND_IN     int                  core,
    DNX_SAND_IN     uint32               tm_port,
    DNX_SAND_IN     uint32               priority_ndx,
    DNX_SAND_IN     uint32               burst
  )
{
    uint32 base_port_tc, offset, tbl_data;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    offset = base_port_tc + priority_ndx;

    DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
    soc_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm_field32_set(unit,&tbl_data,MAX_BURSTf,burst);
    DNXC_IF_ERR_EXIT(WRITE_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
        
exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_arad_sch_port_priority_shaper_hw_set_unsafe(
   DNX_SAND_IN   int    unit,
   DNX_SAND_IN   int    core)
{

    uint32 offset, mem_val, quanta, credit_div, nof_ticks, quanta_nof_bits;
    int rate;
    uint8 valid;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, 0, 1, &credit_div, &nof_ticks, &quanta_nof_bits));

    for (offset = 0 ; offset < JER2_ARAD_EGR_NOF_Q_PAIRS; offset++)
    {
        DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_priority_port_rate_get(unit, core, offset, &rate, &valid));
        if (valid > 0)
        {
            if (rate == 0)
            {
                quanta = 0;
            }
            else
            {    
                DNXC_IF_ERR_EXIT(jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(unit,rate,credit_div,nof_ticks,&quanta));
                DNX_SAND_LIMIT_FROM_ABOVE(quanta, DNX_SAND_BITS_MASK(quanta_nof_bits-1,0));
            }

            DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),offset,&mem_val));
            soc_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm_field32_set(unit,&mem_val,QUANTA_TO_ADDf,quanta);
            DNXC_IF_ERR_EXIT(WRITE_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),offset,&mem_val));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_sch_port_priority_shaper_rate_get_unsafe(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     uint32            priority_ndx,    
    DNX_SAND_OUT    JER2_ARAD_SCH_PORT_PRIORITY_RATE_INFO *info
  )
{
  uint32
    res,
    offset,
    quanta,
    quanta_nof_bits,
    nof_ticks,
    rate_internal,
    credit_div,
    tbl_data;
  uint32
    base_port_tc;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_PRIORITY_SHAPER_RATE_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);
  
  jer2_arad_JER2_ARAD_SCH_PORT_PRIORITY_RATE_INFO_clear(info);  
 
  res= jer2_arad_sch_calendar_info_get(unit, core, 0, 1, &credit_div, &nof_ticks, &quanta_nof_bits);
  DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit);
    
  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 3, exit);

  offset = base_port_tc + priority_ndx;

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
    quanta = soc_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm_field32_get(unit,&tbl_data,QUANTA_TO_ADDf);
    info->max_burst = soc_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm_field32_get(unit,&tbl_data,MAX_BURSTf);

  if (0 == quanta)
  {
    rate_internal = 0;
  }
  else
  {
    res = jer2_arad_sch_port_qunta_to_rate_kbits_per_sec(
            unit,
            quanta,
            credit_div,
            nof_ticks,
            &rate_internal
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  }
  info->rate = rate_internal;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_port_priority_shaper_rate_get_unsafe()",0,0);
}

/*********************************************************************
*     Sets, for a specified tcg 
*     its maximal credit rate. This API is
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_tcg_shaper_rate_set_unsafe(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     JER2_ARAD_TCG_NDX      tcg_ndx,
    DNX_SAND_IN     int               rate
  )
{
  uint32
    res,
    quanta,
    nof_ticks,
    quanta_nof_bits,
    credit_div,
    tbl_data;
  uint32
    base_port_tc,
    offset;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_PRIORITY_SHAPER_RATE_SET_UNSAFE);

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port,&base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 1, exit);

  res = jer2_arad_sch_calendar_info_get(unit, core, 0, 0, &credit_div, &nof_ticks, &quanta_nof_bits);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 2, exit);

  /*
   * Get Device Interface Max Credit Rate
   */
  if (0 == rate)
  {
    quanta = 0;
  }
  else
  {    
    /* 3. calculate quanta */
    res = jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(
            unit,
            rate,
            credit_div,
            nof_ticks,
            &quanta
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 3, exit);

    DNX_SAND_LIMIT_FROM_ABOVE(quanta, DNX_SAND_BITS_MASK(quanta_nof_bits-1,0));
  }

  offset = JER2_ARAD_SCH_PORT_TCG_ID_GET(base_port_tc,tcg_ndx);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
  soc_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm_field32_set(unit,&tbl_data,QUANTA_TO_ADDf,quanta);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1140, exit, WRITE_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
  
  res = jer2_arad_sw_db_sch_port_tcg_rate_set(unit, core, offset, rate, 1);
  DNX_SAND_CHECK_FUNC_RESULT(res, 550, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_tcg_shaper_rate_set_unsafe()",tm_port,tcg_ndx);
}

uint32
jer2_arad_sch_tcg_shaper_max_burst_set_unsafe(
    DNX_SAND_IN     int                  unit,
    DNX_SAND_IN     int                  core,
    DNX_SAND_IN     uint32               tm_port,
    DNX_SAND_IN     JER2_ARAD_TCG_NDX         tcg_ndx,
    DNX_SAND_IN     uint32               burst
  )
{
    uint32 base_port_tc, offset, tbl_data;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    offset = JER2_ARAD_SCH_PORT_TCG_ID_GET(base_port_tc,tcg_ndx); 

    DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
    soc_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm_field32_set(unit,&tbl_data,MAX_BURSTf,burst);
    DNXC_IF_ERR_EXIT(WRITE_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
        
exit:
    DNXC_FUNC_RETURN;
}


uint32
  jer2_arad_sch_tcg_shaper_rate_get_unsafe(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     JER2_ARAD_TCG_NDX      tcg_ndx,    
    DNX_SAND_OUT    JER2_ARAD_SCH_TCG_RATE_INFO *info
  )
{
  uint32
    res,
    offset,
    quanta,
    nof_ticks,
    rate_internal,
    credit_div,    
    tbl_data,
    quanta_nof_bits,
    base_port_tc;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PORT_PRIORITY_SHAPER_RATE_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);
  
  jer2_arad_JER2_ARAD_SCH_TCG_RATE_INFO_clear(info);  

  res= jer2_arad_sch_calendar_info_get(unit, core, 0, 0, &credit_div, &nof_ticks, &quanta_nof_bits);
  DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit);

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);

  offset = JER2_ARAD_SCH_PORT_TCG_ID_GET(base_port_tc,tcg_ndx);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),offset,&tbl_data));
  quanta = soc_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm_field32_get(unit,&tbl_data,QUANTA_TO_ADDf);
  info->max_burst = soc_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm_field32_get(unit,&tbl_data,MAX_BURSTf);

  if (0 == quanta)
  {
    rate_internal = 0;
  }
  else
  {
    res = jer2_arad_sch_port_qunta_to_rate_kbits_per_sec(
            unit,
            quanta,
            credit_div,
            nof_ticks,
            &rate_internal
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  }
  info->rate = rate_internal;  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_tcg_shaper_rate_get_unsafe()",tm_port,tcg_ndx);
}

void
  jer2_arad_JER2_ARAD_SCH_PORT_PRIORITY_RATE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_PORT_PRIORITY_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(JER2_ARAD_SCH_PORT_PRIORITY_RATE_INFO));
    
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SCH_TCG_RATE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SCH_TCG_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(JER2_ARAD_SCH_TCG_RATE_INFO));
    
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


int 
jer2_arad_sch_e2e_interface_allocate(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  soc_port_t      port
    )
{
    return SOC_E_NONE;
}


int 
jer2_arad_sch_e2e_interface_deallocate(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  soc_port_t      port
    )
{
    return SOC_E_NONE;
}


uint32
jer2_arad_sch_calendar_info_get(int unit, int core, int hr_calendar_num ,  int is_priority_rate_calendar ,
                          uint32 *credit_div, uint32 *nof_ticks, uint32 *quanta_nof_bits)
{
    uint32 reg_val, field_val, device_ticks_per_sec, access_period, cal_length, divider;
    DNXC_INIT_FUNC_DEFS;

    if (is_priority_rate_calendar)
    {
        *quanta_nof_bits = soc_mem_field_length(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, QUANTA_TO_ADDf);
    }
    else
    {
        *quanta_nof_bits = soc_mem_field_length(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, QUANTA_TO_ADDf);
    }

    if (hr_calendar_num == 0) /* in jer2_jericho two copies of calendar are available */
    {
        DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATIONr(unit, core, &reg_val));
        divider = soc_reg_field_get(unit, SCH_REBOUNDED_CREDIT_CONFIGURATIONr, reg_val, REBOUNDED_CREDIT_WORTHf);    

        if (is_priority_rate_calendar)
        {
            DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_CONFIGURATIONr(unit, core, &reg_val));
            cal_length = soc_reg_field_get(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, reg_val, PIR_SHAPERS_CAL_LENGTHf) + 1;
            field_val = soc_reg_field_get(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, reg_val, PIR_SHAPERS_CAL_ACCESS_PERIODf);

        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_CONFIGURATIONr(unit, core, &reg_val));
            cal_length = soc_reg_field_get(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, reg_val, CIR_SHAPERS_CAL_LENGTHf) +1;
            field_val = soc_reg_field_get(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, reg_val, CIR_SHAPERS_CAL_ACCESS_PERIODf);
        }       
    }
    else
    {
        DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATION_1r(unit, core, &reg_val));
        divider = soc_reg_field_get(unit, SCH_REBOUNDED_CREDIT_CONFIGURATION_1r, reg_val, REBOUNDED_CREDIT_WORTH_1f);

        if (is_priority_rate_calendar)
        {
            DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_CONFIGURATION_1r(unit, core, &reg_val));
            cal_length = soc_reg_field_get(unit, SCH_PIR_SHAPERS_CONFIGURATION_1r, reg_val, PIR_SHAPERS_CAL_LENGTH_1f) + 1;
            field_val = soc_reg_field_get(unit, SCH_PIR_SHAPERS_CONFIGURATION_1r, reg_val, PIR_SHAPERS_CAL_ACCESS_PERIOD_1f);
        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_CONFIGURATION_1r(unit, core, &reg_val));
            cal_length = soc_reg_field_get(unit, SCH_CIR_SHAPERS_CONFIGURATION_1r, reg_val, CIR_SHAPERS_CAL_LENGTH_1f) + 1;
            field_val = soc_reg_field_get(unit, SCH_CIR_SHAPERS_CONFIGURATION_1r, reg_val, CIR_SHAPERS_CAL_ACCESS_PERIOD_1f);
        }
    }

    access_period = (field_val >> 4);
          
    *credit_div = divider;

    device_ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);
    *nof_ticks = DNX_SAND_DIV_ROUND_UP(device_ticks_per_sec,(cal_length*access_period));

exit:
    DNXC_FUNC_RETURN;
}


int
jer2_arad_sch_port_sched_min_bw_group_get(
    DNX_SAND_IN     int                  unit,
    DNX_SAND_IN     int                  core,
    DNX_SAND_OUT    uint32               *group
  )
{

    int i, res;
    int min_bw = 0, group_bw = 0;

    DNXC_INIT_FUNC_DEFS;

    *group = 0;
    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.groups_bw.get(unit, core, *group, &group_bw);
    DNXC_IF_ERR_EXIT(res);
    min_bw = group_bw;

    for (i = 0; i < JER2_ARAD_SCH_NOF_GROUPS ; i++) {
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.groups_bw.get(unit, core, i, &group_bw);
        DNXC_IF_ERR_EXIT(res);
        if (group_bw < min_bw) {
            min_bw = group_bw;
            *group = i;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
jer2_arad_sch_port_hr_group_get(
    DNX_SAND_IN     int                  unit,
    DNX_SAND_IN     int                  core,
    DNX_SAND_IN     uint32               tm_port,
    DNX_SAND_OUT    uint32               *group
  )
{
    uint32 interface_max_rate, res, base_q_pair, se_id;
    int group_rate, hr_rate;
    soc_port_t port;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_rate_get(unit, port, &interface_max_rate));

    /* get priority 0 HR se_id (all port HRs are associated to the same group) */
    res = jer2_arad_sch_port_tc2se_id(unit, core, tm_port, 0, &se_id);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.hr_group_bw.get(unit, core, base_q_pair, &hr_rate);
    DNXC_IF_ERR_EXIT(res);

    /* if group was alrady set, try to switch the group */
    if (hr_rate) {
        res = jer2_arad_sch_se_group_get(unit, core, se_id, group);
        DNXC_SAND_IF_ERR_EXIT(res);

        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.groups_bw.get(unit, core, *group, &group_rate);
        DNXC_IF_ERR_EXIT(res);

        if ((group_rate - hr_rate) > 0) {
            group_rate = group_rate - hr_rate;
        } else {
            group_rate = 0;
        }
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.groups_bw.set(unit, core, *group, group_rate);
        DNXC_IF_ERR_EXIT(res);
    }

    /* update new group and hr rate */
    DNXC_IF_ERR_EXIT(jer2_arad_sch_port_sched_min_bw_group_get(unit, core, group));
    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.groups_bw.get(unit, core, *group, &group_rate);
    DNXC_IF_ERR_EXIT(res);
    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.groups_bw.set(unit, core, *group, (group_rate + interface_max_rate));
    DNXC_IF_ERR_EXIT(res);
    sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.hr_group_bw.set(unit, core, base_q_pair, interface_max_rate);
    DNXC_IF_ERR_EXIT(res);


exit:
    DNXC_FUNC_RETURN;
}

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */

