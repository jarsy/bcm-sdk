#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_scheduler_end2end.c,v 1.32 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COSQ

/*************
#include <soc/mem.h>
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/mem.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/cosq.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_params.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Management/sand_callback_handles.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>


#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_end2end.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flows.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_ports.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flow_converts.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_device.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_elements.h>
#include <soc/dnx/legacy/ARAD/arad_ofp_rates.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>

#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>

#include <soc/dnx/legacy/mbcm.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* Define calendar access period. Currently static */
#define JER2_ARAD_SCH_CALENDER_ACCESS_PERIOD (3)
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
* NAME:
*     jer2_arad_scheduler_end2end_regs_init
* FUNCTION:
*   Initialization of the Petra blocks configured in this module.
*   This function directly accesses registers/tables for
*   initializations that are not covered by API-s
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_scheduler_end2end_regs_init(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    res,
    data,
    residue,
    field_val,
    index;
  int
    core;

 DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCHEDULER_END2END_REGS_INIT);
  
 SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {   
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, core, 0, SUB_FLOW_ENABLEf,  0x1));
     
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_SCH_SMP_BACK_UP_MESSAGESr(unit, core, 0x10ff));
     
     /* Fill CSC , PSC tables. Each entry describe HR, TCG ID */
     for(index = 0; index <= 255; index++) {
         DNX_SAND_SOC_IF_ERROR_RETURN(res, 2, exit, WRITE_SCH_CIR_SHAPER_CALENDAR_CSCm(unit, SCH_BLOCK(unit,core), index, &index));
         DNX_SAND_SOC_IF_ERROR_RETURN(res, 3, exit, WRITE_SCH_PIR_SHAPER_CALENDAR_PSCm(unit, SCH_BLOCK(unit,core), index, &index));
     }

     /* TCG and HR static configuration shaper */
     DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_SCH_CIR_SHAPERS_CONFIGURATIONr(unit, core, &data));

     /* Calendar access period. Currently static (3.0). */
     residue = 0;
     field_val = (JER2_ARAD_SCH_CALENDER_ACCESS_PERIOD << 4) + residue;
     soc_reg_field_set(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, &data, CIR_SHAPERS_CAL_ACCESS_PERIODf, field_val);


     /* Calendar length equals nof HRs, TCGS (Set is length-1)*/
     field_val = JER2_ARAD_NOF_TCG_IDS - 1;
     soc_reg_field_set(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, &data, CIR_SHAPERS_CAL_LENGTHf, field_val);
     DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_SCH_CIR_SHAPERS_CONFIGURATIONr(unit, core, data));

     if(SOC_IS_JERICHO(unit))
     {
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 21, exit, WRITE_SCH_CIR_SHAPERS_CONFIGURATION_1r(unit, core, data));
     }
     DNX_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_SCH_PIR_SHAPERS_CONFIGURATIONr(unit, core, &data));

     /* Calendar access period. Currently static. */
     residue = 0;
     field_val = (JER2_ARAD_SCH_CALENDER_ACCESS_PERIOD << 4) + residue;
     soc_reg_field_set(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, &data, PIR_SHAPERS_CAL_ACCESS_PERIODf, field_val);

     /* NOF HRS */
     field_val = JER2_ARAD_HR_SE_ID_MAX - JER2_ARAD_HR_SE_ID_MIN;
     soc_reg_field_set(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, &data, PIR_SHAPERS_CAL_LENGTHf, field_val);
     DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_SCH_PIR_SHAPERS_CONFIGURATIONr(unit, core, data)); 

     if(SOC_IS_JERICHO(unit))
     {
       DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_SCH_PIR_SHAPERS_CONFIGURATION_1r(unit, core, data));
     }

     /* DLM enable */
     if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
         DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_SCH_REG_10Br(unit,  0x3FF7));
     }

#ifdef PLISIM
     if (!SAL_BOOT_PLISIM)
#endif
     {
         uint32 entry = 0x7FFFFFF;
          /* Fill PSST and CSST with all ones so it will be dont care shapers. */
         res = jer2_arad_fill_table_with_entry(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, SCH_BLOCK(unit,core), &entry);
         DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

         res = jer2_arad_fill_table_with_entry(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, SCH_BLOCK(unit,core), &entry);
         DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit);
     }
 }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_scheduler_end2end_regs_init()",0,0);

}



/*********************************************************************
* NAME:
*     jer2_arad_scheduler_end2end_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_scheduler_end2end_init(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    port_i,
    weight_i,
    ind,
    flags;
  JER2_ARAD_SCH_GLOBAL_PER1K_INFO
    *global_per1k_info = NULL;
  JER2_ARAD_SCH_SE_CL_CLASS_TABLE
    *cl_class_table = NULL,
    *exact_cl_class_table = NULL;
  JER2_ARAD_SCH_PORT_INFO
    *sch_port_info = NULL;
  JER2_ARAD_SCH_IF_WEIGHTS
    *weights = NULL;
  uint32 
    tm_port;
  int 
    core;
  soc_pbmp_t
    ports_bm;
    

 DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCHEDULER_END2END_INIT);

   res = jer2_arad_sch_flow_ipf_config_mode_set_unsafe(
          unit,
          JER2_ARAD_SCH_NOF_FLOW_IPF_CONFIG_MODES
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  res = jer2_arad_scheduler_end2end_regs_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  JER2_ARAD_ALLOC(global_per1k_info, JER2_ARAD_SCH_GLOBAL_PER1K_INFO, 1, "global_per1k_info");
  SOC_DNX_CORES_ITER(BCM_CORE_ALL, core) {
    JER2_ARAD_CLEAR_STRUCT(global_per1k_info, JER2_ARAD_SCH_GLOBAL_PER1K_INFO);
    global_per1k_info->is_cl_cir = FALSE;
    global_per1k_info->is_interdigitated = FALSE;
    global_per1k_info->is_odd_even = FALSE;
    for (ind = JER2_ARAD_SCH_FLOW_BASE_AGGR_FLOW_ID; ind <= JER2_ARAD_SCH_MAX_FLOW_ID; ind += 1024)
    {
      res = jer2_arad_sch_per1k_info_set_unsafe(
            unit, core,
            JER2_ARAD_SCH_FLOW_TO_1K_ID(ind),
            global_per1k_info
          );
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
  }

  JER2_ARAD_ALLOC(cl_class_table, JER2_ARAD_SCH_SE_CL_CLASS_TABLE, 1, "cl_class_table");
  JER2_ARAD_ALLOC(exact_cl_class_table, JER2_ARAD_SCH_SE_CL_CLASS_TABLE, 1, "exact_cl_class_table");
  SOC_DNX_CORES_ITER(BCM_CORE_ALL, core) {
    JER2_ARAD_CLEAR_STRUCT(cl_class_table, JER2_ARAD_SCH_SE_CL_CLASS_TABLE);
    JER2_ARAD_CLEAR_STRUCT(exact_cl_class_table, JER2_ARAD_SCH_SE_CL_CLASS_TABLE);
    res =
      jer2_arad_sch_class_type_params_table_set(
          unit, core,
          cl_class_table,
          exact_cl_class_table
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }
  JER2_ARAD_ALLOC_AND_CLEAR_STRUCT(sch_port_info, JER2_ARAD_SCH_PORT_INFO,"sch_port_info");
  JER2_ARAD_ALLOC_AND_CLEAR_STRUCT(weights, JER2_ARAD_SCH_IF_WEIGHTS,"weights");
  sch_port_info->enable = FALSE;
  sch_port_info->group = JER2_ARAD_SCH_GROUP_AUTO;
  for (ind = 0; ind < JER2_ARAD_NOF_TRAFFIC_CLASSES; ind++) 
  {
    sch_port_info->hr_modes[ind] = JER2_ARAD_SCH_HR_MODE_NONE;
  }
  
  sch_port_info->lowest_hp_class = JER2_ARAD_SCH_PORT_LOWEST_HP_HR_CLASS_EF1;
  sch_port_info->max_expected_rate = JER2_ARAD_SCH_PORT_MAX_EXPECTED_RATE_AUTO;

  res = dnx_port_sw_db_valid_ports_get(unit, 0, &ports_bm);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  SOC_PBMP_ITER(ports_bm, port_i)
  {
    res = dnx_port_sw_db_flags_get(unit, port_i, &flags);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit);
    if (!(DNX_PORT_IS_ELK_INTERFACE(flags) || DNX_PORT_IS_STAT_INTERFACE(flags))) {
      res = dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

      res = jer2_arad_sch_port_sched_set_unsafe(
            unit, core,
            tm_port,
            sch_port_info
          );
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    }
  }

  for (weight_i = 0; weight_i < JER2_ARAD_SCH_NOF_IF_WEIGHTS; weight_i++)
  {
    weights->weight[weight_i].id = weight_i;
    weights->weight[weight_i].val = 0x1;
  }
  weights->nof_enties = JER2_ARAD_SCH_NOF_IF_WEIGHTS;

  res = jer2_arad_sch_if_weight_conf_set_unsafe(
          unit,
          weights
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

exit:
  if (global_per1k_info) {
      JER2_ARAD_FREE(global_per1k_info);
  }
  JER2_ARAD_FREE(cl_class_table);
  JER2_ARAD_FREE(exact_cl_class_table);
  JER2_ARAD_FREE(sch_port_info);
  JER2_ARAD_FREE(weights);
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_scheduler_end2end_init()",0,0);
}

static uint32
  jer2_arad_sch_group_to_se_assign(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  int             core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID      father_se_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_GROUP      *group
  )
{
  uint32
    res = 0;
  JER2_ARAD_SCH_GROUP
    grp,
    father_grp;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_GROUP_TO_SE_ASSIGN);

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = jer2_arad_sch_se_group_get(
          unit, core,
          father_se_ndx,
          &father_grp
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  grp = (father_grp + 1) % JER2_ARAD_SCH_NOF_GROUPS;

  *group = grp;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_group_to_se_assign()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_se_group_get(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_ID        se_ndx,
    DNX_SAND_OUT    JER2_ARAD_SCH_GROUP*  group
  )
{
  uint32
    offset,
    idx,
    res;
  JER2_ARAD_SCH_FGM_TBL_DATA
    fgm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SE_GROUP_GET);

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  offset  = se_ndx/8;
  idx = se_ndx%8;

  /*
   * Read indirect from FGM table
   */
  res = jer2_arad_sch_fgm_tbl_get_unsafe(
          unit, core,
          offset,
          &fgm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  *group = fgm_tbl_data.flow_group[idx];

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_se_group_get()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_se_group_set(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_ID        se_ndx,
    DNX_SAND_IN     JER2_ARAD_SCH_GROUP        group
  )
{
  uint32
    offset,
    idx,
    hr_ndx,
    field_val,
    data = 0,
    res;  
  JER2_ARAD_SCH_FGM_TBL_DATA
    fgm_tbl_data;
   
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SE_GROUP_SET); 

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  offset  = se_ndx/8;
  idx     = se_ndx%8;

  /*
   * Write indirect from FGM table {
   */
  res = jer2_arad_sch_fgm_tbl_get_unsafe(
          unit, core,
          offset,
          &fgm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (fgm_tbl_data.flow_group[idx] != (uint32)group)
  {
    fgm_tbl_data.flow_group[idx] = group;
    res = jer2_arad_sch_fgm_tbl_set_unsafe(
          unit, core,
          offset,
          &fgm_tbl_data
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }
  /*
   * Write indirect from FGM table }
   */

  /* 
   * In case of HR Write indirect from PFGM {
   */
  if (DNX_SAND_IS_VAL_IN_RANGE(
       se_ndx, JER2_ARAD_HR_SE_ID_MIN, JER2_ARAD_HR_SE_ID_MIN + JER2_ARAD_SCH_MAX_PORT_ID))
  {
    hr_ndx = se_ndx - JER2_ARAD_HR_SE_ID_MIN;
    offset = JER2_ARAD_REG_IDX_GET(hr_ndx, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE); /* PG id */
    idx = JER2_ARAD_FLD_IDX_GET(hr_ndx, JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE);

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, READ_SCH_PORT_GROUP_PFGMm(unit, SCH_BLOCK(unit,core), offset, &data));

    field_val = soc_SCH_PORT_GROUP_PFGMm_field32_get(unit,&data,PORT_GROUPf);

    res = dnx_sand_bitstream_set_any_field(&group,idx * 2,2,&field_val);
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    soc_SCH_PORT_GROUP_PFGMm_field32_set(unit,&data,PORT_GROUPf,field_val);  
    
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1020, exit, WRITE_SCH_PORT_GROUP_PFGMm(unit, SCH_BLOCK(unit,core), offset, &field_val));       
  }
  /* 
   * In case of Port Write indirect from PFGM }
   */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_se_group_set()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_slow_max_rates_verify(
    DNX_SAND_IN  int                 unit
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SLOW_MAX_RATES_VERIFY);

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_slow_max_rates_verify()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_slow_max_rates_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 slow_rate_type,
    DNX_SAND_IN  int                 slow_rate_val
  )
{
  uint32
    slow_fld_val,
    res, reg_val32;
  JER2_ARAD_SCH_SUBFLOW
    sub_flow;
  JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC
    internal_sub_flow;
  soc_field_info_t
    peak_rate_man_fld, 
    peak_rate_exp_fld;


    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SLOW_MAX_RATES_SET_UNSAFE);

    jer2_arad_JER2_ARAD_SCH_SUBFLOW_clear(unit, &sub_flow);

   /*
    * The rate register value is interpreted like \{PeakRateExp,
    * PeakRateMan\} in the SHDS table.
    * Get the fields database for the interpretation.
    */
    JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_MAN_EVENf, &peak_rate_man_fld);
    JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_EXP_EVENf, &peak_rate_exp_fld);

    sub_flow.shaper.max_rate = slow_rate_val;

    res = jer2_arad_sch_to_internal_subflow_shaper_convert(unit, &sub_flow, &internal_sub_flow, TRUE);
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    slow_fld_val = 0;
    slow_fld_val |= JER2_ARAD_FLD_IN_PLACE(internal_sub_flow.peak_rate_exp, peak_rate_exp_fld);
    slow_fld_val |= JER2_ARAD_FLD_IN_PLACE(internal_sub_flow.peak_rate_man, peak_rate_man_fld);
   
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 50, exit, JER2_ARAD_REG_ACCESS_ERR, READ_SCH_SHAPER_CONFIGURATION_REGISTER_1r(unit, SOC_CORE_ALL, &reg_val32));
  
    soc_reg_field_set(unit, SCH_SHAPER_CONFIGURATION_REGISTER_1r, &reg_val32, (slow_rate_type == 1)? SHAPER_SLOW_RATE_1f:SHAPER_SLOW_RATE_2f, slow_fld_val);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 60, exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_SCH_SHAPER_CONFIGURATION_REGISTER_1r(unit, SOC_CORE_ALL, reg_val32));


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_slow_max_rates_set_unsafe()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_slow_max_rates_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 slow_rate_type,
    DNX_SAND_OUT int      *slow_rate_val
  )
{
  uint32
    slow_fld_val,
    res,
    reg_val32;
  JER2_ARAD_SCH_SUBFLOW
    sub_flow;
  JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC
    internal_sub_flow;
  soc_field_info_t
    peak_rate_man_fld, 
    peak_rate_exp_fld;


  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SLOW_MAX_RATES_GET_UNSAFE);

  /* These values are accesed but have no influence on the max rates*/
  internal_sub_flow.max_burst = 0;
  internal_sub_flow.slow_rate_index = 0;

  /*
   * The rate register value is interpreted like \{PeakRateExp,
   * PeakRateMan\} in the SHDS table.
   * Get the fields database for the interpretation.
   */
    JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_MAN_EVENf, &peak_rate_man_fld);
    JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_EXP_EVENf, &peak_rate_exp_fld);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR, READ_SCH_SHAPER_CONFIGURATION_REGISTER_1r(unit, SOC_CORE_ALL, &reg_val32));
    slow_fld_val = soc_reg_field_get(unit, SCH_SHAPER_CONFIGURATION_REGISTER_1r, reg_val32, (slow_rate_type == 1)? SHAPER_SLOW_RATE_1f:SHAPER_SLOW_RATE_2f);

    internal_sub_flow.peak_rate_exp = JER2_ARAD_FLD_FROM_PLACE(slow_fld_val, peak_rate_exp_fld);
    internal_sub_flow.peak_rate_man = JER2_ARAD_FLD_FROM_PLACE(slow_fld_val, peak_rate_man_fld);

    /*
    * The slow setting is equivalent to the SHDS setting.
    */
    res = jer2_arad_sch_from_internal_subflow_shaper_convert(unit, &internal_sub_flow, &sub_flow);
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    *slow_rate_val = sub_flow.shaper.max_rate;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_slow_max_rates_get_unsafe()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_aggregate_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO        *se,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow
  )
{
  JER2_ARAD_SCH_FLOW_ID flow_ndx ;
	
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_AGGREGATE_VERIFY);

  flow_ndx = jer2_arad_sch_se2flow_id(se_ndx);

  if (flow_ndx != flow->sub_flow[0].id)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_AGGR_SE_AND_FLOW_ID_MISMATCH_ERR, 30, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_aggregate_verify()",se_ndx,0);
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
  jer2_arad_sch_aggregate_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO        *se,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32
    res;
  uint32
    sub_flow_i,
    nof_subflows = 0;
  JER2_ARAD_SCH_GROUP
    group = JER2_ARAD_SCH_GROUP_LAST;
  JER2_ARAD_SCH_SE_ID
    subflow_se_id;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_AGGREGATE_SET_UNSAFE);

  /*
   * Set aggregate flow parameters
   */
  if(se->state == JER2_ARAD_SCH_SE_STATE_ENABLE)
  {
    for (sub_flow_i = 0; sub_flow_i < JER2_ARAD_SCH_NOF_SUB_FLOWS; sub_flow_i++)
    {
      if (flow->sub_flow[sub_flow_i].is_valid)
      {
        ++nof_subflows;
      }
    }

    res = jer2_arad_sch_se_set_unsafe(
            unit, core,
            se,
            nof_subflows
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /*
     * Assign scheduler group.
     * For composite flows - use always the first subflow
     * to define the group {
     */
    if (se->group == JER2_ARAD_SCH_GROUP_AUTO)
    {
      res = jer2_arad_sch_group_to_se_assign(
        unit, core,
        flow->sub_flow[0].credit_source.id,
        &group
      );
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
    else
    {
      group = se->group;
    }

    res = jer2_arad_sch_se_group_set(
            unit, core,
            se->id,
            group
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    if (nof_subflows > 1)
    {
      subflow_se_id = jer2_arad_sch_flow2se_id(unit,
                        flow->sub_flow[1].id
                      );
      if (JER2_ARAD_SCH_INDICATED_SE_ID_IS_VALID(subflow_se_id))
      {
        res = jer2_arad_sch_se_group_set(
            unit, core,
            subflow_se_id,
            group
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
      }
    }

    /*
     *  } Assign scheduler group
     */

    /* Install the aggregate, using the first subflow index */
    res = jer2_arad_sch_flow_set_unsafe(
            unit, core,
            flow->sub_flow[0].id,
            flow
           );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }
  else /* se->state == JER2_ARAD_SCH_SE_STATE_DISABLE */
  {
    for (sub_flow_i = 0; sub_flow_i < JER2_ARAD_SCH_NOF_SUB_FLOWS; sub_flow_i++)
    {
      if (flow->sub_flow[sub_flow_i].is_valid)
      {
        ++nof_subflows;
      }
    }

    res = jer2_arad_sch_se_set_unsafe(
            unit, core,
            se,
            nof_subflows
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Install the aggregate, using the first subflow index */
    res = jer2_arad_sch_flow_set_unsafe(
            unit, core,
            flow->sub_flow[0].id,
            flow
           );
    DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_aggregate_set_unsafe()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_aggregate_group_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO        *se,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32
    res;
  uint32
    sub_flow_i,
    nof_subflows = 0;
  JER2_ARAD_SCH_GROUP
    group = JER2_ARAD_SCH_GROUP_LAST;
  JER2_ARAD_SCH_SE_ID
    subflow_se_id;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_AGGREGATE_GROUP_SET_UNSAFE);

  if(se->state == JER2_ARAD_SCH_SE_STATE_ENABLE)
  {
    for (sub_flow_i = 0; sub_flow_i < JER2_ARAD_SCH_NOF_SUB_FLOWS; sub_flow_i++)
    {
      if (flow->sub_flow[sub_flow_i].is_valid)
      {
        ++nof_subflows;
      }
    }

    /*
     * Assign scheduler group.
     * For composite flows - use always the first subflow
     * to define the group {
     */
    if (se->group == JER2_ARAD_SCH_GROUP_AUTO)
    {
      res = jer2_arad_sch_group_to_se_assign(
        unit, core,
        flow->sub_flow[0].credit_source.id,
        &group
      );
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
    else
    {
      group = se->group;
    }
    res = jer2_arad_sch_se_group_set(
            unit, core,
            se->id,
            group
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    if (nof_subflows > 1)
    {
      subflow_se_id = jer2_arad_sch_flow2se_id(unit,
                        flow->sub_flow[1].id
                      );
      if (JER2_ARAD_SCH_INDICATED_SE_ID_IS_VALID(subflow_se_id))
      {
        res = jer2_arad_sch_se_group_set(
            unit, core,
            subflow_se_id,
            group
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
      }
    }

    /*
     *  } Assign scheduler group
     */

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_aggregate_group_set_unsafe",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_aggregate_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_INFO        *se,
    DNX_SAND_OUT JER2_ARAD_SCH_FLOW           *flow
  )
{
  uint32
    res;
  JER2_ARAD_SCH_FLOW_ID
    flow_ndx = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_AGGREGATE_GET_UNSAFE);

  flow_ndx = jer2_arad_sch_se2flow_id(
               se_ndx
             );

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  res = jer2_arad_sch_se_get_unsafe(
          unit, core,
          se_ndx,
          se
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  res = jer2_arad_sch_flow_get_unsafe(
          unit, core,
          flow_ndx,
          flow
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_aggregate_get_unsafe()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_per1k_info_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 k_flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PER1K_INFO_VERIFY);

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_per1k_info_verify()",0,0);
}
/*
 * Flag indicating whether allocation of cache has already been done.
 */
static int __regions_cache_was_allocated[SOC_MAX_NUM_DEVICES] = {0};
static uint8 *__regions_is_cl_cir[SOC_MAX_NUM_DEVICES] = {NULL};
static uint8 *__regions_is_interdigitated[SOC_MAX_NUM_DEVICES] = {NULL};
static int *__regions_p_is_oddEven[SOC_MAX_NUM_DEVICES] = {NULL};
/*
 * Function:
 *   jer2_arad_sch_allocate_cache_for_per1k
 * Purpose:
 *   Internal routine to allocate cache that speeds up
 *   jer2_arad_sch_per1k_info_get_unsafe() from dynamic memory heap.
 * Input:
 *   Direct:
 *     int unit -
 *       Unit identifying this system.
 *   Indirect
 *     SOC_MAX_NUM_DEVICES -
 *       Maximal number of devices that could be connected to this device
 *     NOF_FLOWS_PER_PIPE -
 *       Maximal number of flows per pipe (divide by 1K to get number of per1K entries)
 *     NOF_CORES -
 *       Maximal number of cores on selected device.
 * Output:
 *   Direct:
 *     If non-zero then allocation has failed.
 *   Indirect:
 *     Allocated memory pointed by:
 *       __regions_is_cl_cir (size: SOC_MAX_NUM_DEVICES * (NOF_FLOWS_PER_PIPE/1024) * NOF_CORES ).
 *       __regions_is_interdigitated (size: SOC_MAX_NUM_DEVICES * (NOF_FLOWS_PER_PIPE/1024) * NOF_CORES ).
 *       __regions_p_is_oddEven (size: SOC_MAX_NUM_DEVICES * (NOF_FLOWS_PER_PIPE/1024) * NOF_CORES ).
 * Remarks:
 *   None.
 * See also:
 *   tools\autocoder\DNXDefines\soc_dnx_defines.csv
 *   SOC_DNX_DEFS_MAX, SOC_DNX_DEFS_GET
 */
static int
jer2_arad_sch_allocate_cache_for_per1k(DNX_SAND_IN int unit)
{
  int ret ;
  int size, counter, total_size ;
  int *int_p ;

  ret = 1 ;
  __regions_is_cl_cir[unit] = NULL ;
  __regions_is_interdigitated[unit] = NULL ;
  __regions_p_is_oddEven[unit] = NULL ;
  size =
    ((SOC_DNX_DEFS_GET(unit,nof_flows_per_pipe) / 1024) * SOC_DNX_DEFS_GET(unit,nof_cores)) ;
  total_size = sizeof(*(__regions_is_cl_cir[unit])) * size ;
  __regions_is_cl_cir[unit] = sal_alloc(total_size,"per1k_cache") ;
  if (__regions_is_cl_cir[unit] == NULL)
  {
    goto exit ;
  }
  total_size = sizeof(*(__regions_is_interdigitated[unit])) * size ;
  __regions_is_interdigitated[unit] = sal_alloc(total_size,"per1k_cache") ;
  if (__regions_is_interdigitated[unit] == NULL)
  {
    goto exit ;
  }
  total_size = sizeof(*(__regions_p_is_oddEven[unit])) * size ;
  __regions_p_is_oddEven[unit] = sal_alloc(total_size,"per1k_cache") ;
  if (__regions_p_is_oddEven[unit] == NULL)
  {
    goto exit ;
  }
  /*
   * Fill with '-1' to signify arrays are empty (contain no meaningful data).
   */
  int_p = __regions_p_is_oddEven[unit] ;
  for (counter = 0 ; counter < size ; counter++)
  {
    *int_p++ = (int)(-1) ;
  }
  ret = 0 ;
  __regions_cache_was_allocated[unit] = 1 ;
  /*
   * Roll down to exit.
   */
exit:
  if (ret)
  {
    __regions_cache_was_allocated[unit] = 0 ;
    if (__regions_is_cl_cir[unit])
    {
      sal_free(__regions_is_cl_cir[unit]) ;
    }
    if (__regions_is_interdigitated[unit])
    {
      sal_free(__regions_is_interdigitated[unit]) ;
    }
    if (__regions_p_is_oddEven[unit])
    {
      sal_free(__regions_p_is_oddEven[unit]) ;
    }
    __regions_is_cl_cir[unit] = NULL ;
    __regions_is_interdigitated[unit] = NULL ;
    __regions_p_is_oddEven[unit] = NULL ;
  }
  return (ret) ;
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
  jer2_arad_sch_per1k_info_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 k_flow_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  )
{
  uint32
    res,
    reg_val = 0;
  uint8
    is_cl_cir         = FALSE,
    is_odd_even       = FALSE,
    is_interdigitated = FALSE;
  uint32
    k_idx,
    k_idx_arr_i,
    k_idx_i;


  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PER1K_INFO_SET_UNSAFE);

  {
    if (!__regions_cache_was_allocated[unit])
    {
      int err ;

      err = jer2_arad_sch_allocate_cache_for_per1k(unit) ;
      if(err)
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_INTERRUPT_INSUFFICIENT_MEMORY_ERR, 15, exit);
      }
    }
    /*
     * If we are going to set to values which are already in memory then
     * no memory access is required.
     */
    {
      int table_index ;

      table_index = k_flow_ndx * SOC_DNX_DEFS_GET(unit,nof_cores) + core;
      if ((per1k_info->is_interdigitated == __regions_is_interdigitated[unit][table_index]) &&
                     (per1k_info->is_odd_even == __regions_p_is_oddEven[unit][table_index]) &&
                     (per1k_info->is_cl_cir == __regions_is_cl_cir[unit][table_index]))
      {
        goto exit ;
      }
    }
  }
    /*
     * Note: those id-s are allowed for get - values valid for
     * first 24K flows are returned.
     * It is not valid for set (only 24K - 56K-1 range is valid)
     */
  k_idx = (k_flow_ndx - JER2_ARAD_SCH_FLOW_TO_1K_ID(JER2_ARAD_SCH_FLOW_BASE_AGGR_FLOW_ID));
  k_idx_arr_i =(k_idx < 32) ? 0 : 1;
  k_idx_i = k_idx - (32 * k_idx_arr_i);
  /*
   * OddEven {
   */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_FSF_COMPOSITE_CONFIGURATIONr, core, k_idx_arr_i , FSF_COMP_ODD_EVEN_Nf, &(reg_val)));

  is_odd_even = (uint8)DNX_SAND_GET_BIT(reg_val, k_idx_i);

  if(per1k_info->is_odd_even != is_odd_even)
  {
    DNX_SAND_SET_BIT(reg_val, per1k_info->is_odd_even, k_idx_i);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_FSF_COMPOSITE_CONFIGURATIONr, core, k_idx_arr_i, FSF_COMP_ODD_EVEN_Nf,  reg_val));
  }
  /*
   * OddEven }
   */
  /*
   * Interdigitated {
   */
   DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_SELECT_FLOW_TO_QUEUE_MAPPINGr, core, k_idx_arr_i, INTER_DIG_Nf, &(reg_val)));

  is_interdigitated = (uint8)DNX_SAND_GET_BIT(reg_val, k_idx_i);

  if(per1k_info->is_interdigitated != is_interdigitated)
  {
    DNX_SAND_SET_BIT(reg_val, per1k_info->is_interdigitated, k_idx_i);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_SELECT_FLOW_TO_QUEUE_MAPPINGr, core, k_idx_arr_i, INTER_DIG_Nf,  reg_val));
  }
  /*
   * Interdigitated }
   */
  /*
   * CIR/EIR {
   */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_SWITCH_CIR_EIR_IN_DUAL_SHAPERSr, core,  k_idx_arr_i, SWITCH_CIR_EIR_Nf, &reg_val));

  is_cl_cir = (uint8)DNX_SAND_GET_BIT(reg_val, k_idx_i);

  if(per1k_info->is_cl_cir != is_cl_cir)
  {
    DNX_SAND_SET_BIT(reg_val, per1k_info->is_cl_cir, k_idx_i);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_SWITCH_CIR_EIR_IN_DUAL_SHAPERSr, core, k_idx_arr_i, SWITCH_CIR_EIR_Nf,  reg_val));
  }
  /*
   * CIR/EIR }
   */
  {
    int table_index ;

    table_index = k_flow_ndx * SOC_DNX_DEFS_GET(unit,nof_cores) + core;
    __regions_is_interdigitated[unit][table_index] = per1k_info->is_interdigitated ;
    __regions_p_is_oddEven[unit][table_index] = per1k_info->is_odd_even ;
    __regions_is_cl_cir[unit][table_index] = per1k_info->is_cl_cir ;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_per1k_info_set_unsafe()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_per1k_info_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 k_flow_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  )
{
  uint32
    reg_val,
    res = DNX_SAND_OK;
  uint8
    is_cl_cir         = FALSE,
    is_odd_even       = FALSE,
    is_interdigitated = FALSE;
  uint32
    k_idx,
    k_idx_arr_i,
    k_idx_i;


  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_PER1K_INFO_GET_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);

  /*
   * In the standard case, do not expect 'get' before 'set' but it is possible,
   * in case of 'warm' startup.
   */
  {
    if (!__regions_cache_was_allocated[unit])
    {
      int err ;

      err = jer2_arad_sch_allocate_cache_for_per1k(unit) ;
      if(err)
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_INTERRUPT_INSUFFICIENT_MEMORY_ERR, 15, exit);
      }
    }
  }

  if (!JER2_ARAD_SCH_1K_FLOWS_IS_IN_AGGR_RANGE(k_flow_ndx))
  {
    jer2_arad_JER2_ARAD_SCH_GLOBAL_PER1K_INFO_clear(per1k_info);
  }
  else
  {
    int already_loaded ;

    already_loaded = 0 ;
    {
      int table_index ;

      table_index = k_flow_ndx * SOC_DNX_DEFS_GET(unit,nof_cores) + core;
      if (__regions_p_is_oddEven[unit][table_index] != -1)
      {
        per1k_info->is_interdigitated = __regions_is_interdigitated[unit][table_index];
        per1k_info->is_odd_even = __regions_p_is_oddEven[unit][table_index];
        per1k_info->is_cl_cir = __regions_is_cl_cir[unit][table_index];
        already_loaded = 1 ;
      }
    }
    if (!already_loaded)
    {
      k_idx = (k_flow_ndx - JER2_ARAD_SCH_FLOW_TO_1K_ID(JER2_ARAD_SCH_FLOW_BASE_AGGR_FLOW_ID));
      k_idx_arr_i =(k_idx < 32) ? 0 : 1;
      k_idx_i = k_idx - (32 * k_idx_arr_i);
      /*
       * OddEven {
       */
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_FSF_COMPOSITE_CONFIGURATIONr, core,  k_idx_arr_i, FSF_COMP_ODD_EVEN_Nf, &(reg_val)));
  
      is_odd_even = (uint8)DNX_SAND_GET_BIT(reg_val, k_idx_i);
  
      per1k_info->is_odd_even = is_odd_even;
      /*
       * OddEven }
       */
      /*
       * Interdigitated {
       */
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_SELECT_FLOW_TO_QUEUE_MAPPINGr, core,  k_idx_arr_i, INTER_DIG_Nf, &(reg_val)));
  
      is_interdigitated = (uint8)DNX_SAND_GET_BIT(reg_val, k_idx_i);
  
      per1k_info->is_interdigitated = is_interdigitated;
      /*
       * Interdigitated }
       */
      /*
       * CIR/EIR {
       */
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_SWITCH_CIR_EIR_IN_DUAL_SHAPERSr, core,  k_idx_arr_i, SWITCH_CIR_EIR_Nf, &reg_val));
  
      is_cl_cir = (uint8)DNX_SAND_GET_BIT(reg_val, k_idx_i);
  
      per1k_info->is_cl_cir = is_cl_cir;
      /*
       * CIR/EIR }
       */
      {
        int table_index ;

        table_index = k_flow_ndx * SOC_DNX_DEFS_GET(unit,nof_cores) + core;
        __regions_is_interdigitated[unit][table_index] = per1k_info->is_interdigitated ;
        __regions_p_is_oddEven[unit][table_index] = per1k_info->is_odd_even ;
        __regions_is_cl_cir[unit][table_index] = per1k_info->is_cl_cir ;
      }
    }
  }
exit:

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_per1k_info_get_unsafe()",0,0);
}

/*****************************************************
* NAME
*   jer2_arad_sch_nof_quartets_to_map_get
* TYPE:
*   PROC
* DATE:
*   14/01/2008
* FUNCTION:
* Calculate number of quartets to map according to the following table:
*
*    |InterDigitated | Composite | nof_quartets_to_map |
*     -------------------------------------------------
*    |      0        |     0     |         1           |
*     -------------------------------------------------
*    |      1        |     0     |         2           |
*     -------------------------------------------------
*    |      0        |     1     |         2           |
*     -------------------------------------------------
*    |      1        |     1     |         4           |
*     -------------------------------------------------
*
* INPUT:
*   DNX_SAND_IN  uint8                 is_interdigitated -
*     Interdigitated mode per-1k configuration
*   DNX_SAND_IN  uint8                 is_composite -
*     Composite per-quartet configuration
*   DNX_SAND_OUT uint32                 *nof_quartets_to_map -
*     Number of quartets to map, according to the table above
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_nof_quartets_to_map_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 is_interdigitated,
    DNX_SAND_IN  uint8                 is_composite,
    DNX_SAND_OUT uint32                 *nof_quartets_to_map
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_NOF_QUARTETS_TO_MAP_GET);

  switch(is_interdigitated)
  {
  case FALSE:
    if (is_composite == FALSE)
    {
      *nof_quartets_to_map = 1;
    }
    else
    {
      *nof_quartets_to_map = 2;
    }
    break;
  case TRUE:
    if (is_composite == FALSE)
    {
      *nof_quartets_to_map = 2;
    }
    else
    {
      *nof_quartets_to_map = 4;
    }
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_TO_Q_INVALID_GLOBAL_CONF_ERR, 10, exit);
    break;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_nof_quartets_to_map_get()", 0, 0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
* Note:
*   If quartet_flow_info->fip_id == DNX_TMC_MAX_FAP_ID then this is
*   a 'remove entry' operation.
*********************************************************************/
uint32
  jer2_arad_sch_flow_to_queue_mapping_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 quartet_ndx,
    DNX_SAND_IN  uint32                 nof_quartets_to_map,
    DNX_SAND_IN  JER2_ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  )
{
  uint32
    offset,
    res;
  JER2_ARAD_SCH_FLOW_ID
    flow_ndx;
  uint32
    k_flow_ndx,
    quartets_to_map_calculated = 0;
  JER2_ARAD_SCH_GLOBAL_PER1K_INFO
    per1k_info;
  JER2_ARAD_SCH_FFM_TBL_DATA
    ffm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_VERIFY);

  flow_ndx = JER2_ARAD_SCH_QRTT_TO_FLOW_ID(quartet_ndx);
  k_flow_ndx = JER2_ARAD_SCH_FLOW_TO_1K_ID(flow_ndx);

  res = jer2_arad_sch_per1k_info_get_unsafe(
          unit, core,
          k_flow_ndx,
          &per1k_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (per1k_info.is_odd_even == FALSE)
  {
    /*
     * Flows with Odd-Even configuration set to FALSE
     * (0-2 configuration) cannot be mapped to queues
     */
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_TO_Q_ODD_EVEN_IS_FALSE_ERR, 40, exit);
  }

  res = jer2_arad_sch_nof_quartets_to_map_get(
          unit,
          per1k_info.is_interdigitated,
          quartet_flow_info->is_composite,
          &quartets_to_map_calculated
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  if (quartets_to_map_calculated != nof_quartets_to_map)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_TO_Q_NOF_QUARTETS_MISMATCH_ERR, 60, exit);
  }

  DNX_SAND_ERR_IF_ABOVE_MAX(
    quartet_flow_info->fip_id, JER2_ARAD_MAX_FAP_ID,
    JER2_ARAD_FAP_PORT_ID_INVALID_ERR , 65, exit
  );

  /*
   * Validate input FIP ID for either odd or even quartets
   * with per-8 flow_id-s configuration.
   */
  {
    offset = quartet_ndx / 2;
    res = jer2_arad_sch_ffm_tbl_get_unsafe(
          unit, core,
          offset,
          &ffm_tbl_data
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    /*
     * Compare input quartet with its containing '8 flow_id-s configuration' and verify they
     * have the same 'fip_id'
     * Important note:
     *   If 'fip_id' is DNX_TMC_MAX_FAP_ID then the object (input quartet or
     *   '8 flow_id-s configuration') is empty and there is nothing to compare to.
     */
    if ((ffm_tbl_data.device_number != DNX_TMC_MAX_FAP_ID) && (quartet_flow_info->fip_id != DNX_TMC_MAX_FAP_ID))
    {
      if (ffm_tbl_data.device_number != quartet_flow_info->fip_id)
      { 
      	if (!SOC_IS_QUX(unit)) {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_TO_FIP_SECOND_QUARTET_MISMATCH_ERR, 80, exit);
      }
      }
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_to_queue_mapping_verify()",0,0);
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
  jer2_arad_sch_flow_to_queue_mapping_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 quartet_ndx,
    DNX_SAND_IN  uint32                 nof_quartets_to_map,
    DNX_SAND_IN  JER2_ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  )
{
  uint32
    offset,
    quartet_i,
    res;
  JER2_ARAD_SCH_FQM_TBL_DATA
    fqm_tbl_data;
  JER2_ARAD_SCH_FFM_TBL_DATA
    ffm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_SET_UNSAFE);

  for (quartet_i = 0; quartet_i < nof_quartets_to_map; ++quartet_i)
  {
    /*
     * RMW from FQM {
     * (don't just write because of the flow_slow_enable field)
     */
    offset  = quartet_ndx + quartet_i;

    res = jer2_arad_sch_fqm_tbl_get_unsafe(
            unit, core,
            offset,
            &fqm_tbl_data
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    fqm_tbl_data.base_queue_num = quartet_flow_info->base_q_qrtt_id;
    fqm_tbl_data.sub_flow_mode = quartet_flow_info->is_composite;

    res = jer2_arad_sch_fqm_tbl_set_unsafe(
            unit, core,
            offset,
            &fqm_tbl_data
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    /*
     * RMW from FQM }
     */

    if (nof_quartets_to_map == 1)
    {
      /*
       * Enter if user wishes to only map one quartet.
       * In that case, quartet_i must be zero.
       * Quartet may either be even or odd.
       */
      int update_table ;

      update_table = 1 ;
      if ((quartet_flow_info->fip_id == DNX_TMC_MAX_FAP_ID) &&
                          (quartet_flow_info->other_quartet_is_valid))
      {
        /*
         * If caller wishes to invalidate table entry but other quartet,
         * within current group of 8 flow id-s, is valid, then do not
         * invalidate. Table invalidation takes place only when both
         * quartets are invalid.
         */
        update_table = 0 ;
      }
      if (update_table)
      {
        /*
         * Write to FFM {
         * Set offset the containing group of 8 flow id-s
         */
        offset = quartet_ndx / 2 ;
        ffm_tbl_data.device_number = quartet_flow_info->fip_id ;
        res =
          jer2_arad_sch_ffm_tbl_set_unsafe(
            unit, core,
            offset,
            &ffm_tbl_data
          ) ;
        DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        /*
         * Write to FFM }
         */
      }
    }
    else
    {
    if (((quartet_ndx + quartet_i) % 2) == 0)
    {
      /*
       * Write to FFM {
       * - only for even quartets (per 8 flow id-s configuration)
       */
      offset = (quartet_ndx + quartet_i) / 2;

      ffm_tbl_data.device_number = quartet_flow_info->fip_id;

      res = jer2_arad_sch_ffm_tbl_set_unsafe(
              unit, core,
              offset,
              &ffm_tbl_data
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 45, exit);
      /*
       * Write to FFM }
       */
      }
    }
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_to_queue_mapping_set_unsafe()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_flow_to_queue_mapping_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 quartet_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  )
{
  uint32
    offset,
    res;
  JER2_ARAD_SCH_FQM_TBL_DATA
    fqm_tbl_data;
  JER2_ARAD_SCH_FFM_TBL_DATA
    ffm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_GET_UNSAFE);

  /*
   * Read from FQM {
   */
  offset  = quartet_ndx;

  res = jer2_arad_sch_fqm_tbl_get_unsafe(
          unit, core,
          offset,
          &fqm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  quartet_flow_info->base_q_qrtt_id = fqm_tbl_data.base_queue_num;
  quartet_flow_info->is_composite = (uint8)fqm_tbl_data.sub_flow_mode;
  /*
   * Read from FQM }
   */

  /*
   * Read from FFM {
   */
  offset = quartet_ndx/2;

  res = jer2_arad_sch_ffm_tbl_get_unsafe(
          unit, core,
          offset,
          &ffm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  quartet_flow_info->fip_id = ffm_tbl_data.device_number;

  /*
   * Read from FFM }
   */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_to_queue_mapping_get_unsafe()",0,0);
}


/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint8
  jer2_arad_sch_is_flow_id_se_id(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID        flow_id
  )
{
  JER2_ARAD_SCH_SE_ID
    se_id = 0;

  se_id = jer2_arad_sch_flow2se_id(unit, flow_id);

  return (JER2_ARAD_SCH_INDICATED_SE_ID_IS_VALID(se_id))?TRUE : FALSE;
}


uint32
  jer2_arad_sch_flow_delete_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID         flow_ndx
  )
{
  uint32
      offset, 
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_FDMS_TBL_DATA
    sch_fdms_tbl_data;
  JER2_ARAD_SCH_SHDS_TBL_DATA
    shds_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_DELETE_UNSAFE);

  /*
   *    Set max-burst token-bucket to '0'.
   *  This protects from turning on an interrupt
   *  when the FDMS is zeroed
   */
  /*
   * Get current value - we only update part of the fields (odd/even)
   */
  offset  = flow_ndx / 2;
  res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_shds_tbl_get_unsafe, (unit, core, offset, &shds_tbl_data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (flow_ndx % 2 == 0)
  {
    shds_tbl_data.max_burst_even = 0x0;
    shds_tbl_data.max_burst_update_even = 0x1;
  }
  else
  {
    shds_tbl_data.max_burst_odd = 0x0;
    shds_tbl_data.max_burst_update_odd = 0x1;
  }

  /*
   * Write indirect to SHDS table
   */
  res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_shds_tbl_set_unsafe, (unit, core, offset, &shds_tbl_data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  JER2_ARAD_DEVICE_CHECK(unit, exit);
  /*
   *    Set FDMS to 0 as invalid-flow indication
   */
  sch_fdms_tbl_data.cos = 0;
  sch_fdms_tbl_data.hrsel_dual = 0;
  sch_fdms_tbl_data.sch_number = 0;
  res = jer2_arad_sch_fdms_tbl_set_unsafe(
          unit, core,
          flow_ndx,
          &sch_fdms_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_delete_unsafe()",flow_ndx,0);
}

uint32
  jer2_arad_sch_flow_is_deleted_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_FLOW_ID         flow_ndx,
    DNX_SAND_OUT uint8                 *flow_is_reset
  )
{
  uint32
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_FDMS_TBL_DATA
    sch_fdms_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FLOW_IS_RESET_GET_UNSAFE);

  res = jer2_arad_sch_fdms_tbl_get_unsafe(
          unit, core,
          flow_ndx,
          &sch_fdms_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  *flow_is_reset = (sch_fdms_tbl_data.cos == 0) ? TRUE : FALSE;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_sch_flow_is_deleted_get_unsafe()",flow_ndx,0);
}

#if JER2_ARAD_DEBUG

uint32
  jer2_arad_flow_status_info_get(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  int               core,
    DNX_SAND_IN  uint32               flow_id,
    DNX_SAND_OUT uint32               *credit_rate
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    credit_cnt;
  uint32
    credit_worth;
   
    

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FLOW_STATUS_INFO_GET);

  

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_CREDIT_COUNTER_CONFIGURATION_REG_1r, SOC_CORE_ALL, 0, FILTER_FLOWf,  flow_id));
/*  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  15,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_CREDIT_COUNTER_CONFIGURATION_REG_1r, SOC_CORE_ALL, 0, FILTER_FLOW_MASKf,  0xffff));*/


  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_CREDIT_COUNTER_CONFIGURATION_REG_2r, SOC_CORE_ALL, 0, FILTER_BY_FLOWf,  0x1));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  21,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_CREDIT_COUNTER_CONFIGURATION_REG_2r, SOC_CORE_ALL, 0, FILTER_BY_SUB_FLOWf,  0x0));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  22,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_CREDIT_COUNTER_CONFIGURATION_REG_2r, SOC_CORE_ALL, 0, FILTER_DEST_FAPf,  0x0));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_SCH_DBG_CREDIT_COUNTERr(unit, core, &credit_cnt));

  dnx_sand_os_task_delay_milisec(1008);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_SCH_DBG_CREDIT_COUNTERr(unit, core, &credit_cnt));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
    res,50,exit,JER2_ARAD_GET_ERR_TEXT_001,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &credit_worth))) ;

  *credit_rate = credit_cnt * ((DNX_SAND_NOF_BITS_IN_CHAR * credit_worth) / JER2_ARAD_RATE_1K);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_flow_status_info_get()",flow_id,0);
}

uint32
  jer2_arad_port_status_info_get(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  int               core,
    DNX_SAND_IN  uint32               port_id,
    DNX_SAND_IN  uint32               priority_ndx,
    DNX_SAND_OUT uint32               *credit_rate,
    DNX_SAND_OUT uint32               *fc_cnt,
    DNX_SAND_OUT uint32               *fc_percent
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    credit_cnt,
    crd_reg_val,
    fc_reg_val,
    fld_val;
  uint32
    credit_worth,  
    base_port_tc,
    offset;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  
  /* 
   * Convert Port to Base HR 
   */ 
  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, port_id, &base_port_tc);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  offset = base_port_tc + priority_ndx;


 /*
  * count credits for the port
  */
  crd_reg_val = 0;
  fld_val = offset;

  JER2_ARAD_FLD_TO_REG(SCH_DBG_DVS_CREDIT_COUNTER_CONFIGURATIONr, DVS_FILTER_PORTf, fld_val, crd_reg_val, 10, exit);

  fld_val = 0x1;
  JER2_ARAD_FLD_TO_REG(SCH_DBG_DVS_CREDIT_COUNTER_CONFIGURATIONr, CNT_BY_PORTf, fld_val, crd_reg_val, 20, exit);

  fld_val = 0x0;
  JER2_ARAD_FLD_TO_REG(SCH_DBG_DVS_CREDIT_COUNTER_CONFIGURATIONr, CNT_BY_NIFf, fld_val, crd_reg_val, 21, exit);

 /*
  * count flow control for the port
  */
  fc_reg_val = 0;
  fld_val = offset;
  JER2_ARAD_FLD_TO_REG(SCH_DBG_DVS_FC_COUNTERS_CONFIGURATIONr, FC_CNT_PORTf, fld_val, fc_reg_val, 30, exit);

  fld_val = 0x1;
  JER2_ARAD_FLD_TO_REG(SCH_DBG_DVS_FC_COUNTERS_CONFIGURATIONr, CNT_PORT_FCf, fld_val, fc_reg_val, 40, exit);

 /*
  * start counting
  */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_SCH_DBG_DVS_CREDIT_COUNTER_CONFIGURATIONr(unit, core, crd_reg_val));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_SCH_DBG_DVS_FC_COUNTERS_CONFIGURATIONr(unit, core, fc_reg_val));
 /*
  * clear counters
  */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_SCH_DVS_CREDIT_COUNTERr(unit, core, &credit_cnt));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  90,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_SCH_DBG_DVS_FLOW_CONTROL_COUNTERr(unit, core, fc_cnt));
 /*
  * wait ~one second
  */
  dnx_sand_os_task_delay_milisec(1008);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  100,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_SCH_DVS_CREDIT_COUNTERr(unit, core, &credit_cnt));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_SCH_DBG_DVS_FLOW_CONTROL_COUNTERr(unit, core, fc_cnt));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
    res,120,exit,JER2_ARAD_GET_ERR_TEXT_001,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &credit_worth))) ;

  *credit_rate = credit_cnt * ((DNX_SAND_NOF_BITS_IN_CHAR * credit_worth) / JER2_ARAD_RATE_1K);


  if (*fc_cnt != 0)
  {
    *fc_percent = DNX_SAND_DIV_ROUND_UP((*fc_cnt * 100), jer2_arad_chip_ticks_per_sec_get(unit));
    DNX_SAND_LIMIT_FROM_ABOVE(*fc_percent, 100);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_port_status_info_get()",port_id,priority_ndx);

}

uint32
  jer2_arad_agg_status_info_get(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  int               core,
    DNX_SAND_IN  uint32               se_id,
    DNX_SAND_OUT uint32               *credit_rate,
    DNX_SAND_OUT uint32               *overflow


  )
{
  uint32
    credit_cnt,
    res=0;
  uint32
    credit_worth;
   
    

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_AGG_STATUS_INFO_GET);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_DBG_CML_CREDIT_SCHEDULER_COUNTER_CONFIGURATIONr, core, 0, FILTER_SCH_MASKf,  0x7fff));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_DBG_CML_CREDIT_SCHEDULER_COUNTER_CONFIGURATIONr, core, 0, FILTER_SCHf,  se_id));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  0x30,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_DBG_CML_SCHEDULER_COUNTERr, core, 0, CML_SCH_CREDIT_CNTf, &credit_cnt));

  dnx_sand_os_task_delay_milisec(1008);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  0x40,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_DBG_CML_SCHEDULER_COUNTERr, core, 0, CML_SCH_CREDIT_CNTf, &credit_cnt));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  0x45,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_DBG_CML_SCHEDULER_COUNTERr, core, 0, CML_SCH_CREDIT_OVFf, overflow));

  if (!DNX_SAND_NUM2BOOL(*overflow) == TRUE)
  {
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
      res,50,exit,JER2_ARAD_GET_ERR_TEXT_001,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &credit_worth))) ;
    *credit_rate = credit_cnt * ((DNX_SAND_NOF_BITS_IN_CHAR * credit_worth)/JER2_ARAD_RATE_1K);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_agg_status_info_get()",se_id,0);
}

uint32
jer2_arad_flow_and_up_info_get_unsafe(
        DNX_SAND_IN     int                             unit,
        DNX_SAND_IN     int                             core,
        DNX_SAND_IN     uint32                             flow_id,
        DNX_SAND_IN     uint32                             reterive_status,
        DNX_SAND_INOUT  JER2_ARAD_SCH_FLOW_AND_UP_INFO           *flow_and_up_info
)
{
    uint32
    res = DNX_SAND_OK;
    uint32
    credit_source_i;
    JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO
    *dummy_shaper = NULL;
    JER2_ARAD_SCH_SE_CL_CLASS_INFO class_type;
    JER2_ARAD_SCH_FLOW_AND_UP_PORT_INFO *port_sch_info;
    JER2_ARAD_SCH_FLOW_AND_UP_SE_INFO   *se_sch_info;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    JER2_ARAD_ALLOC_AND_CLEAR_STRUCT(dummy_shaper, JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO,"dummy_shaper");


    if (flow_and_up_info->credit_sources_nof == 0)
    {
        /*
         * First level print.
         */
        res = jer2_arad_sch_flow_get_unsafe(
                unit, core,
                flow_id,
                &(flow_and_up_info->sch_consumer)
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        if(reterive_status)
        {
            res = jer2_arad_flow_status_info_get(unit, core, flow_id, &(flow_and_up_info->credit_rate));
            DNX_SAND_CHECK_FUNC_RESULT(res, 11, exit);
        }
        /*
         * In each level, the current entity, get the credit
         *  source entity, and print it's configuration.
         * Flows and aggregates have credit sources, and therefore
         *  we will get up in the hierarchy, until we arrive
         *  to the port level.
         */

        if((flow_and_up_info->sch_consumer).sub_flow[0].is_valid == TRUE)
        {
            flow_and_up_info->credit_sources[0] = (flow_and_up_info->sch_consumer).sub_flow[0].credit_source.id;
            flow_and_up_info->credit_sources_nof++;
        }
        if((flow_and_up_info->sch_consumer).sub_flow[1].is_valid == TRUE)
        {
            flow_and_up_info->credit_sources[1] = (flow_and_up_info->sch_consumer).sub_flow[1].credit_source.id;
            flow_and_up_info->credit_sources_nof++;
        }
    }

    for(credit_source_i = 0; credit_source_i < (flow_and_up_info->credit_sources_nof); ++credit_source_i)
    {

        res = jer2_arad_sch_se2port_tc_id(
            unit, core,
                (flow_and_up_info->credit_sources)[credit_source_i],
                &(flow_and_up_info->sch_port_id[credit_source_i]),
                &(flow_and_up_info->sch_priority_ndx[credit_source_i]));
        DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

        if(jer2_arad_sch_is_port_id_valid(unit, flow_and_up_info->sch_port_id[credit_source_i]))
        {
            flow_and_up_info->is_port_sch[credit_source_i] = 1;
            port_sch_info = &((flow_and_up_info->sch_union_info[credit_source_i]).port_sch_info);
            jer2_arad_JER2_ARAD_SCH_FLOW_AND_UP_PORT_INFO_clear(port_sch_info);

            res = jer2_arad_sch_port_sched_get_unsafe(
                    unit,
                    core,
                    flow_and_up_info->sch_port_id[credit_source_i],
                    &(port_sch_info->port_info)
            );
            DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

            /*Get port sch rate*/
            res = jer2_arad_ofp_rates_sch_single_port_rate_hw_get(unit, core, flow_and_up_info->sch_port_id[credit_source_i], &(port_sch_info->ofp_rate_info.sch_rate));
            if (res != SOC_E_NONE) {
                DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR,25,exit);
            }
            /*Get port egq rate*/
            res = jer2_arad_ofp_rates_egq_single_port_rate_hw_get(unit,core,flow_and_up_info->sch_port_id[credit_source_i], &(port_sch_info->ofp_rate_info.egq_rate));
            if (res != SOC_E_NONE) {
                DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR,27,exit);
            }
            /*Get port max_burst*/
            res = jer2_arad_ofp_rates_single_port_max_burst_get(unit,core,flow_and_up_info->sch_port_id[credit_source_i],&(port_sch_info->ofp_rate_info.max_burst));
            if (res != SOC_E_NONE) {
                DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR,29,exit);
            }

            /* get port id */
            port_sch_info->ofp_rate_info.port_id = flow_and_up_info->sch_port_id[credit_source_i];

            if(reterive_status)
            {
                jer2_arad_port_status_info_get(unit,
                        core,
                        flow_and_up_info->sch_port_id[credit_source_i],
                        flow_and_up_info->sch_priority_ndx[credit_source_i],
                        &(port_sch_info->credit_rate),
                        &(port_sch_info->fc_cnt),
                        &(port_sch_info->fc_percent)
                );
            }
        } else if(jer2_arad_sch_is_se_id_valid(unit,flow_and_up_info->credit_sources[credit_source_i]))
        {
            flow_and_up_info->is_port_sch[credit_source_i] = 0;

            /*print agg.*/
            se_sch_info = &((flow_and_up_info->sch_union_info[credit_source_i]).se_sch_info);
            jer2_arad_JER2_ARAD_SCH_FLOW_AND_UP_SE_INFO_clear(unit, se_sch_info);

            (se_sch_info->se_info).id = flow_and_up_info->credit_sources[credit_source_i];

            res = jer2_arad_sch_aggregate_get_unsafe(
                    unit, core,
                    (se_sch_info->se_info).id,
                    &(se_sch_info->se_info),
                    &(se_sch_info->sch_consumer)
            );
            DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            if (se_sch_info->se_info.type == JER2_ARAD_SCH_SE_TYPE_CL){
                res = jer2_arad_sch_class_type_params_get_unsafe(unit, core, se_sch_info->se_info.type_info.cl.id, &class_type);
                DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);
                se_sch_info->cl_mode = class_type.weight_mode;
                res = jer2_arad_sch_flow_ipf_config_mode_get(unit, &(se_sch_info->ipf_mode));
                DNX_SAND_CHECK_FUNC_RESULT(res, 32, exit);
            }
            if(reterive_status)
            {
                res = jer2_arad_agg_status_info_get(unit,
                        core,
                        (se_sch_info->se_info).id,
                        &(se_sch_info->credit_rate),
                        &(se_sch_info->credit_rate_overflow));
                DNX_SAND_CHECK_FUNC_RESULT(res, 33, exit);

            }
            if((se_sch_info->sch_consumer).sub_flow[0].is_valid == TRUE && (flow_and_up_info->next_level_credit_sources_nof < DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES))
            {
                flow_and_up_info->next_level_credit_sources[flow_and_up_info->next_level_credit_sources_nof] = (se_sch_info->sch_consumer).sub_flow[0].credit_source.id;
                (flow_and_up_info->next_level_credit_sources_nof)++;
            }
            if((se_sch_info->sch_consumer).sub_flow[1].is_valid == TRUE && (flow_and_up_info->next_level_credit_sources_nof < DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES))
            {
                (flow_and_up_info->next_level_credit_sources)[(flow_and_up_info->next_level_credit_sources_nof)] = (se_sch_info->sch_consumer).sub_flow[1].credit_source.id;
                (flow_and_up_info->next_level_credit_sources_nof)++;
            }
        }
        else
        {
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_SE_ID_ERR,40,exit);
            /*error: flow can't be credit source*/
        }
    }
#ifdef JER2_ARAD_PRINT_FLOW_AND_UP_PRINT_DRM_AND_MAL_RATES
    if ( (flow_and_up_info->next_level_credit_sources_nof) == 0)
    {
        JER2_ARAD_INTERFACE_ID   if_id;
        uint32              source_if_id;
        uint32              dsp_pp_i;
        DNX_TMC_OFP_RATES_TBL_INFO *ofp_rates_tbl = &flow_and_up_info->ofp_rates_table;

        res = jer2_arad_port_to_interface_map_get(
                unit,
                core,
                flow_and_up_info->sch_port_id[credit_source_i],
                &if_id,
                NULL
        );
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 52, exit);

        /*Go ovber all the if's ports, count them and get their rates*/
        for (ofp_rate_tbl->nof_valid_entries = 0, dsp_pp_i = 0; dsp_pp_i < JER2_ARAD_NOF_FAP_PORTS; ++dsp_pp_i)
        {
            res = jer2_arad_port_to_interface_map_get(
                    unit,
                    core,
                    dsp_pp_i,
                    &if_id,
                    NULL
            );
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

            if (if_id == source_if_id)
            {
                res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, dsp_pp_i, &base_q_pair);
                DNX_SAND_SOC_IF_ERROR_RETURN(res,63,exit);

                if (base_q_pair == JER2_ARAD_NOF_FAP_PORTS)
                {
                    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_OFP_RATES_INVALID_PORT_ID_ERR, 55, exit);
                }
                /*Get port sch rate*/
                res = jer2_arad_ofp_rates_sch_single_port_rate_hw_get(unit,core, dsp_pp_i,&ofp_rate_tbl->rates[flow_and_up_info->ofp_rates_tbl.nof_valid_entries].sch_rate);
                if (res != SOC_E_NONE) {
                    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR,65,exit);
                }
                /*Get port egq rate*/
                res = jer2_arad_ofp_rates_egq_single_port_rate_hw_get(unit,core,dsp_pp_i,&ofp_rate_tbl->rates[flow_and_up_info->ofp_rates_tbl.nof_valid_entries].egq_rate);
                if (res != SOC_E_NONE) {
                    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR,75,exit);
                }
                /*Get port max_burst*/
                res = jer2_arad_ofp_rates_single_port_max_burst_get(unit,core,dsp_pp_i,&ofp_rate_tbl->rates[flow_and_up_info->ofp_rates_tbl.nof_valid_entries].max_burst);
                if (res != SOC_E_NONE) {
                    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_INVALID_PORT_ID_ERR,85,exit);
                }

                ofp_rates_tbl->rates[ofp_rate_tbl->nof_valid_entries++].port_id = dsp_pp_i;
            }
        }

        for (dsp_pp_i = ofp_rate_tbl->nof_valid_entries; dsp_pp_i < JER2_ARAD_EGR_NOF_BASE_Q_PAIRS; ++dsp_pp_i)
        {
            jer2_arad_JER2_ARAD_OFP_RATE_INFO_clear(ofp_rate_tbl->rates + dsp_pp_i);
        }
        flow_and_up_info->ofp_rate_valid = 1;
    }
#endif
    exit:

    JER2_ARAD_FREE(dummy_shaper);
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_flow_and_up_info_get_unsafe()",0,0);
}

#endif

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
#endif /* of #if defined(BCM_88690_A0) */
