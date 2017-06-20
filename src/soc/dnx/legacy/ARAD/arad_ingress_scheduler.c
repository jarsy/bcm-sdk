/* $Id: jer2_arad_ingress_scheduler.c,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INGRESS

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_ingress_scheduler.h>

#include <soc/dnx/legacy/ARAD/arad_reg_access.h>

#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>

#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>
#include <soc/dnx/legacy/SAND/Utils/sand_conv.h>

#include <soc/mcm/memregs.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_CONVERSION_TEST_REGRESSION_STEP 9999
#define JER2_ARAD_CONVERSION_TEST_MAX_RATE_DIFF 250000

#define JER2_ARAD_ING_SCH_CLOS_NOF_SHAPERS (JER2_ARAD_ING_SCH_CLOS_NOF_GLOBAL_SHAPERS + JER2_ARAD_ING_SCH_CLOS_NOF_HP_SHAPERS + JER2_ARAD_ING_SCH_CLOS_NOF_LP_SHAPERS)

/*Max time configuration of slow start mechanism above the multicast queues*/
#define JER2_ARAD_IPT_MC_SLOW_START_TIMER_MAX    (17)
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
*     jer2_arad_ingress_scheduler_regs_init
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
static uint32
  jer2_arad_ingress_scheduler_regs_init(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    res = DNX_SAND_OK;   
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_REGS_INIT);

  /*Slow start mechanism for multicast queues*/
  /*Disable mechanism by default*/
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_4_SLOW_START_ENABLEf,  0 ));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_5_SLOW_START_ENABLEf,  0 ));
 /*Configure mask time of slow start phases to max*/
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  3,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_4_SLOW_START_CFG_TIMER_PERIOD_0f,  JER2_ARAD_IPT_MC_SLOW_START_TIMER_MAX ));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_4_SLOW_START_CFG_TIMER_PERIOD_1f,  JER2_ARAD_IPT_MC_SLOW_START_TIMER_MAX ));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_5_SLOW_START_CFG_TIMER_PERIOD_0f,  JER2_ARAD_IPT_MC_SLOW_START_TIMER_MAX ));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  6,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_5_SLOW_START_CFG_TIMER_PERIOD_1f,  JER2_ARAD_IPT_MC_SLOW_START_TIMER_MAX ));
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_regs_init()",0,0);
}

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_init
* FUNCTION:
*     Initialization of the Petra blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_init(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    res = DNX_SAND_OK;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_ingress_scheduler_regs_init(
          unit
        );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:    
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info
  )
{
  uint32
    index;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_MESH_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(mesh_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_mesh_info);

  DNX_SAND_MAGIC_NUM_VERIFY(mesh_info);
  DNX_SAND_MAGIC_NUM_VERIFY(exact_mesh_info);

  for (index = 0;index < JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS;index++)
  {
    if (mesh_info->contexts[index].weight > JER2_ARAD_ING_SCH_MAX_WEIGHT_VALUE)
    {
      /*ERROR - weight out of range*/
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_WEIGHT_OUT_OF_RANGE_ERR, 10, exit);
    }
    if (mesh_info->contexts[index].shaper.max_burst > JER2_ARAD_ING_SCH_MAX_MAX_CREDIT_VALUE)
    {
      /*ERROR - max_credit out of range*/
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_MAX_CREDIT_OUT_OF_RANGE_ERR, 20, exit);
    }
    if (mesh_info->contexts[index].id > JER2_ARAD_ING_SCH_MAX_ID_VALUE)
    {
      /*ERROR - id out of range*/
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_MESH_ID_OUT_OF_RANGE_ERR, 30, exit);
    }
  }
  if (mesh_info->total_rate_shaper.max_burst > JER2_ARAD_ING_SCH_MAX_MAX_CREDIT_VALUE)
  {
    /*ERROR - max_credit out of range*/
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_MAX_CREDIT_OUT_OF_RANGE_ERR, 40, exit);
  }
  if (mesh_info->nof_entries > JER2_ARAD_ING_SCH_MAX_NOF_ENTRIES)
  {
    /*ERROR - nof_entries out of range*/
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_MESH_NOF_ENTRIES_OUT_OF_RANGE_ERR, 50, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_mesh_verify()",0,0);
}

/*********************************************************************
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info
  )
{
  uint32
    res,
    exact_total_max_rate,
    exact_max_rate;
  uint32
    index;
  JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO
    current_context;
   
  reg_field 
      wfq_weights [JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
      shaper_max_crdts[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
      shaper_delays[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
      shaper_cals[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS];
  
  reg_field max_credit, delay, cal;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_MESH_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(mesh_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_mesh_info);

  res = jer2_arad_ingress_scheduler_mesh_reg_flds_db_get(
          unit,
          wfq_weights,
          shaper_max_crdts,
          shaper_delays,
          shaper_cals
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,10,exit);

  dnx_sand_os_memcpy(
    exact_mesh_info,
    mesh_info,
    sizeof(JER2_ARAD_ING_SCH_MESH_INFO)
  );

  for (index=0;index<(mesh_info->nof_entries);index++)
  {
    current_context = mesh_info->contexts[index];

    /* write WfqWeight field */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, wfq_weights[current_context.id].reg, REG_PORT_ANY, 0, wfq_weights[current_context.id].field,  current_context.weight));

    /*write Shaper fields*/
    res = jer2_arad_ingress_scheduler_shaper_values_set(
            unit,
            TRUE,
            &(current_context.shaper),
            &(shaper_max_crdts[current_context.id]),
            &(shaper_delays[current_context.id]),
            &(shaper_cals[current_context.id]),
            &exact_max_rate
          );

    DNX_SAND_CHECK_FUNC_RESULT(res,30,exit);

    exact_mesh_info->contexts[index].shaper.max_rate = exact_max_rate;

  }

  /*write Total Shaper regs and fields*/
  max_credit.reg = IPT_SHAPER_8_MAX_CREDITr;
  max_credit.field = SHAPER_8_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_8_DELAYr;
  delay.field = SHAPER_8_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_8_CALr;      
  cal.field = SHAPER_8_CALf;
  cal.index = 0;
    
  res = jer2_arad_ingress_scheduler_shaper_values_set(
          unit,
          TRUE,
          &mesh_info->total_rate_shaper,
          &max_credit,
          &delay,
          &cal,
          &exact_total_max_rate
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,40,exit);

  exact_mesh_info->total_rate_shaper.max_rate = exact_total_max_rate;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_mesh_set_unsafe()",0,0);
}

/*********************************************************************
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info
  )
{
  uint32
    res,
    index;
  JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO
    current_context;
  JER2_ARAD_ING_SCH_SHAPER
    current_shaper;
  reg_field
    wfq_weights[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    shaper_max_crdts[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    shaper_delays[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    shaper_cals[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS];
   
  reg_field max_credit, delay, cal;  

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_MESH_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(mesh_info);
  
  jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_clear(mesh_info);

  res = jer2_arad_ingress_scheduler_mesh_reg_flds_db_get(
          unit,
          wfq_weights,
          shaper_max_crdts,
          shaper_delays,
          shaper_cals
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,10,exit);

  for (index=0;index<JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS;index++)
  {
    current_context.id = index;

    /*write WfqWeight field*/
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, wfq_weights[index].reg, REG_PORT_ANY, 0, wfq_weights[index].field, &current_context.weight));

    /*write Shaper fields*/
    res = jer2_arad_ingress_scheduler_shaper_values_get(
            unit,
            TRUE,
            &(shaper_max_crdts[index]),
            &(shaper_delays[index]),
            &(shaper_cals[index]),
            &(current_context.shaper)
          );

    DNX_SAND_CHECK_FUNC_RESULT(res,30,exit);

    dnx_sand_os_memcpy(
      &(mesh_info->contexts[index]),
      &(current_context),
      sizeof(JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO)
    );
  }

  /* write Total Shaper regs and fields */
  max_credit.reg = IPT_SHAPER_8_MAX_CREDITr;
  max_credit.field = SHAPER_8_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_8_DELAYr;
  delay.field = SHAPER_8_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_8_CALr;      
  cal.field = SHAPER_8_CALf;
  cal.index = 0;
  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &current_shaper
        );

  DNX_SAND_CHECK_FUNC_RESULT(res,40,exit);

  dnx_sand_os_memcpy(
    &(mesh_info->total_rate_shaper),
    &(current_shaper),
    sizeof(JER2_ARAD_ING_SCH_SHAPER)
  );
  mesh_info->nof_entries = JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_mesh_get_unsafe()",0,0);
}

/*********************************************************************
*     This procedure reads the values of the the shaper (max_burst, max_rate)
*     from the suitable registers fields (ShaperMaxCredit, ShaperDelay, ShaperCal).
*     The max_rate value of the Shaper structure is retrieved from the
*     ShaperDelay & ShaperCal fields, using an additional function that
*     converts the suitable values to max_rate.
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
  jer2_arad_ingress_scheduler_shaper_values_get(
    DNX_SAND_IN   int                 unit,
    DNX_SAND_IN   int                 is_delay_2_clocks,
    DNX_SAND_IN   reg_field              *max_credit,
    DNX_SAND_IN   reg_field              *delay,
    DNX_SAND_IN   reg_field              *cal,
    DNX_SAND_OUT  JER2_ARAD_ING_SCH_SHAPER    *shaper
  )
{

  uint32
    res;
  uint32
    shaper_delay_2_clocks,
    shaper_cal;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_SHAPER_VALUES_GET);

  DNX_SAND_CHECK_NULL_INPUT(shaper);

  /* Get Shaper Values { */

  /* read ShaperMaxCredit field */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, max_credit->reg, REG_PORT_ANY, max_credit->index, max_credit->field, &shaper->max_burst));

  /* read ShaperDelay field */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, delay->reg, REG_PORT_ANY, delay->index, delay->field, &shaper_delay_2_clocks));

  /* read ShaperCal field */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, cal->reg, REG_PORT_ANY, cal->index, cal->field, &shaper_cal));

  /* if delay is in 2 clocks resolution */
  if (is_delay_2_clocks) 
  {
      shaper_delay_2_clocks *=2;
  }

  res = jer2_arad_ingress_scheduler_delay_cal_to_max_rate_form(
          unit,
          shaper_delay_2_clocks,
          shaper_cal,
          &(shaper->max_rate)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,4,exit);

  /* Get Shaper Values } */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_shaper_values_get()",0,0);
}

/*********************************************************************
*     This procedure converts the ingress scheduler shaper register field
*     values:
*     1. ShaperDelay: Time interval to add the credit.
*        in two clocks cycles resolution.
*     2. ShaperCal: Credit to add, in bytes resolution.
*
*     to the max_rate value of the shaper structure in the API (in kbps)
*
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
  jer2_arad_ingress_scheduler_delay_cal_to_max_rate_form(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint32          shaper_delay,
    DNX_SAND_IN  uint32          shaper_cal,
    DNX_SAND_OUT uint32          *max_rate
  )
{
  uint32
    res,
    device_ticks_per_sec,
    exact_rate_in_kbits_per_sec;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_DELAY_CAL_TO_MAX_RATE_FORM);

  DNX_SAND_CHECK_NULL_INPUT(max_rate);

  device_ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);

  if (shaper_cal == 0)
  {
    exact_rate_in_kbits_per_sec = 0;
  }
  else
  {
    res = dnx_sand_clocks_to_kbits_per_sec(
            shaper_delay,
            shaper_cal,
            device_ticks_per_sec,
            &exact_rate_in_kbits_per_sec
            );
    if (res == DNX_SAND_OVERFLOW_ERR)
    {
      /*
       *    Unshaped
       */
      exact_rate_in_kbits_per_sec = JER2_ARAD_ING_SCH_MAX_MAX_CREDIT_VALUE;
    }
  }

  *max_rate = exact_rate_in_kbits_per_sec;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_delay_cal_to_max_rate_form()",0,0);
}

/*********************************************************************
*     This procedure is used for the Mesh topology. This procedure initializes
*     arrays of shaper_max_crdts, shaper_delays and shaper_cals with the
*     appropriate fields. This is done fo0r easier implementation of the set and
*     get functions.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_reg_flds_db_get(
    DNX_SAND_IN  int       unit,
    reg_field          wfq_weights[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    reg_field          shaper_max_crdts[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    reg_field          shaper_delays[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    reg_field          shaper_cals[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS]
    )
{ 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_MESH_REG_FLDS_DB_GET);

  /* Set Mesh Regs and Fields */
  
  /* wfq weights */
  wfq_weights[0].field = WFQ_0_WEIGHTf;
  wfq_weights[0].reg = IPT_WFQ_WEIGHT_0r;
  wfq_weights[0].index = 0;
  wfq_weights[1].field = WFQ_1_WEIGHTf;
  wfq_weights[1].reg = IPT_WFQ_WEIGHT_0r;
  wfq_weights[1].index = 0;
  wfq_weights[2].field = WFQ_2_WEIGHTf;
  wfq_weights[2].reg = IPT_WFQ_WEIGHT_0r;
  wfq_weights[2].index = 0;
  wfq_weights[3].field = WFQ_3_WEIGHTf;
  wfq_weights[3].reg = IPT_WFQ_WEIGHT_0r;
  wfq_weights[3].index = 0;

  wfq_weights[4].field = WFQ_4_WEIGHTf;
  wfq_weights[4].reg = IPT_WFQ_WEIGHT_1r;
  wfq_weights[4].index = 0;
  wfq_weights[5].field = WFQ_5_WEIGHTf;
  wfq_weights[5].reg = IPT_WFQ_WEIGHT_1r;
  wfq_weights[5].index = 0;
  wfq_weights[6].field = WFQ_6_WEIGHTf;
  wfq_weights[6].reg = IPT_WFQ_WEIGHT_1r;
  wfq_weights[6].index = 0;
  wfq_weights[7].field = WFQ_7_WEIGHTf;
  wfq_weights[7].reg = IPT_WFQ_WEIGHT_1r;
  wfq_weights[7].index = 0;
  
  /* shaper max credit */
  shaper_max_crdts[0].field = SHAPER_0_MAX_CREDITf;
  shaper_max_crdts[0].reg = IPT_SHAPER_01_MAX_CREDITr;
  shaper_max_crdts[0].index = 0;
  shaper_max_crdts[1].field = SHAPER_1_MAX_CREDITf;
  shaper_max_crdts[1].reg = IPT_SHAPER_01_MAX_CREDITr;
  shaper_max_crdts[1].index = 0;
  
  shaper_max_crdts[2].field = SHAPER_2_MAX_CREDITf;
  shaper_max_crdts[2].reg = IPT_SHAPER_23_MAX_CREDITr;
  shaper_max_crdts[2].index = 0;
  shaper_max_crdts[3].field = SHAPER_3_MAX_CREDITf;
  shaper_max_crdts[3].reg = IPT_SHAPER_23_MAX_CREDITr;  
  shaper_max_crdts[3].index = 0;

  shaper_max_crdts[4].field = SHAPER_4_MAX_CREDITf;
  shaper_max_crdts[4].reg = IPT_SHAPER_45_MAX_CREDITr;
  shaper_max_crdts[4].index = 0;
  shaper_max_crdts[5].field = SHAPER_5_MAX_CREDITf;
  shaper_max_crdts[5].reg = IPT_SHAPER_45_MAX_CREDITr;
  shaper_max_crdts[5].index = 0;

  shaper_max_crdts[6].field = SHAPER_6_MAX_CREDITf;
  shaper_max_crdts[6].reg = IPT_SHAPER_67_MAX_CREDITr;
  shaper_max_crdts[6].index = 0;
  shaper_max_crdts[7].field = SHAPER_7_MAX_CREDITf;
  shaper_max_crdts[7].reg = IPT_SHAPER_67_MAX_CREDITr;
  shaper_max_crdts[7].index = 0;    

  /* shaper delay */
  shaper_delays[0].field = SHAPER_0_DELAYf;
  shaper_delays[0].reg = IPT_SHAPER_01_DELAYr;
  shaper_delays[0].index = 0;
  shaper_delays[1].field = SHAPER_1_DELAYf;
  shaper_delays[1].reg = IPT_SHAPER_01_DELAYr;
  shaper_delays[1].index = 0;

  shaper_delays[2].field = SHAPER_2_DELAYf;
  shaper_delays[2].reg = IPT_SHAPER_23_DELAYr;
  shaper_delays[2].index = 0;
  shaper_delays[3].field = SHAPER_3_DELAYf;
  shaper_delays[3].reg = IPT_SHAPER_23_DELAYr;
  shaper_delays[3].index = 0;

  shaper_delays[4].field = SHAPER_4_DELAYf;
  shaper_delays[4].reg = IPT_SHAPER_45_DELAYr;
  shaper_delays[4].index = 0;
  shaper_delays[5].field = SHAPER_5_DELAYf;
  shaper_delays[5].reg = IPT_SHAPER_45_DELAYr;
  shaper_delays[5].index = 0;
  
  shaper_delays[6].field = SHAPER_6_DELAYf;
  shaper_delays[6].reg = IPT_SHAPER_67_DELAYr;
  shaper_delays[6].index = 0;
  shaper_delays[7].field = SHAPER_7_DELAYf;
  shaper_delays[7].reg = IPT_SHAPER_67_DELAYr;
  shaper_delays[7].index = 0;

  /* shaper cal */ 
  shaper_cals[0].field = SHAPER_0_CALf;
  shaper_cals[0].reg = IPT_SHAPER_01_CALr;
  shaper_cals[0].index= 0;
  shaper_cals[1].field = SHAPER_1_CALf;
  shaper_cals[1].reg = IPT_SHAPER_01_CALr;
  shaper_cals[1].index= 0;

  shaper_cals[2].field = SHAPER_2_CALf;
  shaper_cals[2].reg = IPT_SHAPER_23_CALr;
  shaper_cals[2].index= 0;
  shaper_cals[3].field = SHAPER_3_CALf;
  shaper_cals[3].reg = IPT_SHAPER_23_CALr;
  shaper_cals[3].index= 0;

  shaper_cals[4].field = SHAPER_4_CALf;
  shaper_cals[4].reg = IPT_SHAPER_45_CALr;
  shaper_cals[4].index= 0;
  shaper_cals[5].field = SHAPER_5_CALf;
  shaper_cals[5].reg = IPT_SHAPER_45_CALr;
  shaper_cals[5].index= 0;

  shaper_cals[6].field = SHAPER_6_CALf;
  shaper_cals[6].reg = IPT_SHAPER_67_CALr;
  shaper_cals[6].index= 0;
  shaper_cals[7].field = SHAPER_7_CALf;
  shaper_cals[7].reg = IPT_SHAPER_67_CALr;
  shaper_cals[7].index= 0;

  /* } Set Mesh Fields */

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_mesh_reg_flds_db_get()",0,0);
}

/*********************************************************************
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
  )
{
  uint32
    index,
    weights[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    shapers_max_burst[JER2_ARAD_ING_SCH_CLOS_NOF_SHAPERS];

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_clos_info);

  DNX_SAND_MAGIC_NUM_VERIFY(clos_info);
  DNX_SAND_MAGIC_NUM_VERIFY(exact_clos_info);

  if(clos_info->weights.global_hp.weight2 != 1) {
      /*forced to 1 by Arad HW*/
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_WEIGHT_OUT_OF_RANGE_ERR, 1, exit);
  }

  weights[0] = clos_info->weights.fabric_lp.weight1;
  weights[1] = clos_info->weights.fabric_lp.weight2;
  weights[2] = clos_info->weights.fabric_hp.weight1;
  weights[3] = clos_info->weights.fabric_hp.weight2;
  weights[4] = clos_info->weights.global_lp.weight1;
  weights[5] = clos_info->weights.global_lp.weight2;
  weights[6] = clos_info->weights.global_hp.weight1;
  weights[7] = clos_info->weights.global_hp.weight2;

  shapers_max_burst[0] = clos_info->shapers.local.max_burst;
  shapers_max_burst[1] = clos_info->shapers.fabric.max_burst;
  shapers_max_burst[2] = clos_info->shapers.hp.local.max_burst;
  shapers_max_burst[3] = clos_info->shapers.hp.fabric_unicast.max_burst;
  shapers_max_burst[4] = clos_info->shapers.hp.fabric_multicast.max_burst;
  shapers_max_burst[5] = clos_info->shapers.lp.fabric_multicast.max_burst;
  shapers_max_burst[6] = clos_info->shapers.lp.fabric_unicast.max_burst;

  for (index = 0 ;index < JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS ; ++index)
  {
    if (weights[index] > JER2_ARAD_ING_SCH_MAX_WEIGHT_VALUE)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_WEIGHT_OUT_OF_RANGE_ERR, 10, exit);
    }
  }
  for (index = 0 ;index < JER2_ARAD_ING_SCH_CLOS_NOF_SHAPERS; ++index)
  {
    if (shapers_max_burst[index] > JER2_ARAD_ING_SCH_MAX_MAX_CREDIT_VALUE)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_MAX_CREDIT_OUT_OF_RANGE_ERR, 20, exit);
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_verify()",0,0);
}

/*********************************************************************
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_clos_info);

  dnx_sand_os_memcpy(
    exact_clos_info,
    clos_info,
    sizeof(JER2_ARAD_ING_SCH_CLOS_INFO)
    );
  /* Set Weights */
  res = jer2_arad_ingress_scheduler_clos_weights_set(
          unit,
          clos_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,1,exit);
  /* Set Global Shapers */
  res = jer2_arad_ingress_scheduler_clos_global_shapers_set(
          unit,
          clos_info,
          exact_clos_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);

  /* Set HP Shapers */
  res = jer2_arad_ingress_scheduler_clos_hp_shapers_set(
          unit,
          clos_info,
          exact_clos_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,3,exit);  
  
  /* Set HP Shapers */
  res = jer2_arad_ingress_scheduler_clos_lp_shapers_set(
          unit,
          clos_info,
          exact_clos_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,4,exit);  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_set_unsafe()",0,0);
}

/*********************************************************************
*     This procedure writes all the weights in the clos_info structure
*     to the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
   jer2_arad_ingress_scheduler_clos_weights_set(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO*  clos_info
    )
{

  uint32
    res;

   
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_WEIGHTS_SET);

  /* Set Weights { */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2 ,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_2_WEIGHTf,  clos_info->weights.global_hp.weight1));
  /*clos_info->weights.global_hp.weight2 is locked to be 1 in HW, not configurable*/

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  6 ,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_0_WEIGHTf,  clos_info->weights.fabric_hp.weight1));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8 ,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_1_WEIGHTf,  clos_info->weights.fabric_hp.weight2));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_3_WEIGHTf,  clos_info->weights.fabric_lp.weight1));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  12,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_WFQ_WEIGHT_1r, REG_PORT_ANY, 0, WFQ_4_WEIGHTf,  clos_info->weights.fabric_lp.weight2));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  14,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_WFQ_WEIGHT_1r, REG_PORT_ANY, 0, WFQ_5_WEIGHTf,  clos_info->weights.global_lp.weight1));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  16,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_WFQ_WEIGHT_1r, REG_PORT_ANY, 0, WFQ_6_WEIGHTf,  clos_info->weights.global_lp.weight2));
  /* } Set Weights */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_weights_set()",0,0);
}

/*********************************************************************
*     This procedure writes the values of the global shapers (fabric and local)
*     in the clos_info structure to the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
   jer2_arad_ingress_scheduler_clos_global_shapers_set(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
     DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
   )
{

  uint32
    res,
    exact_local_max_rate,
    exact_fab_max_rate;
   
  reg_field max_credit, delay, cal;   

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_GLOBAL_SHAPERS_SET);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_clos_info);

  
  /* Set Global Shapers { */

  /* Set Global Local Shaper */
  max_credit.reg = IPT_SHAPER_01_MAX_CREDITr;
  max_credit.field = SHAPER_0_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_01_DELAYr;
  delay.field = SHAPER_0_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_01_CALr;      
  cal.field = SHAPER_0_CALf;
  cal.index = 0;
    
  res = jer2_arad_ingress_scheduler_shaper_values_set(
        unit,
        TRUE,
        &(clos_info->shapers.local),
        &max_credit,
        &delay,
        &cal,
        &exact_local_max_rate
        );

  DNX_SAND_CHECK_FUNC_RESULT(res,1,exit);
  exact_clos_info->shapers.local.max_rate = exact_local_max_rate;

  max_credit.field = SHAPER_1_MAX_CREDITf;
  delay.field = SHAPER_1_DELAYf;
  cal.field = SHAPER_1_CALf;
  
  /* Set Global Fabric Shaper */
  res = jer2_arad_ingress_scheduler_shaper_values_set(
          unit,
          TRUE,
          &(clos_info->shapers.fabric),
          &max_credit,
          &delay,
          &cal,
          &exact_fab_max_rate
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);
  exact_clos_info->shapers.fabric.max_rate = exact_fab_max_rate;

  /* } Set Global Shapers */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_global_shapers_set()",0,0);
}

/*********************************************************************
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);

  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_clear(clos_info);

  /* Get Weights */
  res = jer2_arad_ingress_scheduler_clos_weights_get(
          unit,
          clos_info
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* Get Global Shapers */
  res = jer2_arad_ingress_scheduler_clos_global_shapers_get(
          unit,
          clos_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit);

  /* Get HP Shapers */
  res = jer2_arad_ingress_scheduler_clos_hp_shapers_get(
          unit,
          clos_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,30,exit);
  
  /* Get LP Shapers */
  res = jer2_arad_ingress_scheduler_clos_lp_shapers_get(
          unit,
          clos_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,40,exit);
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_get_unsafe()",0,0);
}

/*********************************************************************
*     This procedure reads the values of the global shapers (fabric and local)
*     to the clos_info structure from the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
   jer2_arad_ingress_scheduler_clos_weights_get(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
    )
{

  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_WEIGHTS_GET);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  0,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_2_WEIGHTf, &clos_info->weights.global_hp.weight1));
  clos_info->weights.global_hp.weight2 = 1; /*Hard coded in the device*/

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_0_WEIGHTf, &clos_info->weights.fabric_hp.weight1));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  6,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_1_WEIGHTf, &clos_info->weights.fabric_hp.weight2));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_WFQ_WEIGHT_0r, REG_PORT_ANY, 0, WFQ_3_WEIGHTf, &clos_info->weights.fabric_lp.weight1));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  12,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_WFQ_WEIGHT_1r, REG_PORT_ANY, 0, WFQ_4_WEIGHTf, &clos_info->weights.fabric_lp.weight2));

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  14,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_WFQ_WEIGHT_1r, REG_PORT_ANY, 0, WFQ_5_WEIGHTf, &clos_info->weights.global_lp.weight1));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  16,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_WFQ_WEIGHT_1r, REG_PORT_ANY, 0, WFQ_6_WEIGHTf, &clos_info->weights.global_lp.weight2));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_weights_get()",0,0);
}

/*********************************************************************
*     This procedure reads the values of the global shapers (fabric and local)
*     to the clos_info structure from the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
   jer2_arad_ingress_scheduler_clos_global_shapers_get(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
    )
{
  uint32
    res;
   
  reg_field max_credit, delay, cal;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_GLOBAL_SHAPERS_GET);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);

  /* Get Global Shapers { */

  /* Get Global Local Shaper */
  max_credit.reg = IPT_SHAPER_01_MAX_CREDITr;
  max_credit.field = SHAPER_0_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_01_DELAYr;
  delay.field = SHAPER_0_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_01_CALr;
  cal.field = SHAPER_0_CALf;
  cal.index = 0;
  
  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &(clos_info->shapers.local)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,1,exit);

  /* Get Global Fabric Shaper */
  max_credit.field = SHAPER_1_MAX_CREDITf;
  delay.field = SHAPER_1_DELAYf;
  cal.field = SHAPER_1_CALf;
  
  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &(clos_info->shapers.fabric)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);

  /* } Get Global Shapers */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_global_shapers_get()",0,0);
}

/*********************************************************************
*     This procedure writes the values a given shapers (max_burst, max_rate)
*     in the clos_info structure to the suitable registers fields
*     (ShaperMaxCredit, ShaperDelay, ShaperCal). The ShaperDelay & ShaperCal
*     fields are retrieved using an additional function that converts max_rate
*     to the suitable values.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_shaper_values_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  int                  is_delay_2_clocks,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_SHAPER     *shaper,
    DNX_SAND_IN  reg_field               *max_credit,
    DNX_SAND_IN  reg_field               *delay,
    DNX_SAND_IN  reg_field               *cal,
    DNX_SAND_OUT uint32                  *exact_max_rate
  )
{

  uint32
    res;

  uint32
    shaper_delay_2_clocks,
    shaper_cal,
    exact;
  JER2_ARAD_ING_SCH_SHAPER
    out_shaper_struct;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_SHAPER_VALUES_SET);

  DNX_SAND_CHECK_NULL_INPUT(shaper);

  /* Set Shaper { */

  /*write ShaperMaxCredit field*/
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, max_credit->reg, REG_PORT_ANY, max_credit->index, max_credit->field,  shaper->max_burst));

  if(shaper->max_rate != DNX_TMC_ING_SCH_DONT_TOUCH) {
      /* convert max_rate to delay and cal, delay is returned in 2 clocks resolution  */
      res = jer2_arad_ingress_scheduler_rate_to_delay_cal_form(
           unit,
           shaper->max_rate,
           is_delay_2_clocks,
           &shaper_delay_2_clocks,
           &shaper_cal,
           &exact
      );
      DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);

      /*write ShaperDelay field*/
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  3, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, delay->reg, REG_PORT_ANY, delay->index, delay->field,  shaper_delay_2_clocks));

      /*write ShaperCal field*/

      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, cal->reg, REG_PORT_ANY, cal->index, cal->field,  shaper_cal));

      *exact_max_rate = exact;
  } else {
      out_shaper_struct=*shaper;
      res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          is_delay_2_clocks,
          max_credit,
          delay,
          cal,
          &out_shaper_struct
      );
      DNX_SAND_CHECK_FUNC_RESULT(res,6,exit);
      *exact_max_rate = out_shaper_struct.max_rate;
  }

  /* } Set Shaper */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_shaper_values_set()",0,0);
}

/*********************************************************************
*     This procedure converts the ingress scheduler shaper max_rate
*     given in kbps to the values of the register fields-
*     1. ShaperDelay: Time interval to add the credit.
*        in two clocks cycles resolution.
*     2. ShaperCal: Credit to add, in bytes resolution.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_rate_to_delay_cal_form(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  uint32             max_rate,
    DNX_SAND_IN  int                is_delay_2_clocks,
    DNX_SAND_OUT uint32*            shaper_delay_2_clocks,
    DNX_SAND_OUT uint32*            shaper_cal,
    DNX_SAND_OUT uint32*            exact_max_rate
  )
{
  uint32
    res,
    device_ticks_per_sec,
    delay_value,
    cal_value,
    exact_cal_value_long,
    exact_rate_in_kbits_per_sec,
    divider;
  uint8
    cal_and_delay_in_range = FALSE;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_RATE_TO_DELAY_CAL_FORM);

  DNX_SAND_CHECK_NULL_INPUT(shaper_delay_2_clocks);
  DNX_SAND_CHECK_NULL_INPUT(shaper_cal);

  DNX_SAND_CHECK_NULL_INPUT(exact_max_rate);

  cal_value = JER2_ARAD_ING_SCH_FIRST_CAL_VAL;

  if(is_delay_2_clocks) {
      divider = 2;
  } else {
      divider = 1;
  }

  device_ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);

  if (0 == max_rate)
  {
    *shaper_cal = JER2_ARAD_ING_SCH_MIN_CAL_VAL;
    *shaper_delay_2_clocks = JER2_ARAD_ING_SCH_MAX_DELAY_VAL;
    *exact_max_rate = 0;
    goto exit;
  }

  while ((cal_value > 1) && \
    (cal_value < JER2_ARAD_ING_SCH_MAX_CAL_VAL) && \
    !(cal_and_delay_in_range)
  )
  {
    res = dnx_sand_kbits_per_sec_to_clocks(
            max_rate,
            cal_value,
            device_ticks_per_sec,
            &delay_value
          );
    DNX_SAND_CHECK_FUNC_RESULT(res,10,exit);

    if (delay_value > (JER2_ARAD_ING_SCH_MAX_DELAY_VAL*divider))
    {
      cal_value /= 2;
    }
    else if (delay_value < (JER2_ARAD_ING_SCH_MIN_DELAY_VAL*divider))
    {
      cal_value *= 2;
      if (cal_value > JER2_ARAD_ING_SCH_MAX_CAL_VAL)
      {
        cal_value = JER2_ARAD_ING_SCH_MAX_CAL_VAL;
      }
    }
    else
    {
      cal_and_delay_in_range = TRUE;
    }
  }

  res = dnx_sand_kbits_per_sec_to_clocks(
          max_rate,
          cal_value,
          device_ticks_per_sec,
          &delay_value
         );
  DNX_SAND_CHECK_FUNC_RESULT(res,20,exit);

  if(delay_value > (JER2_ARAD_ING_SCH_MAX_DELAY_VAL*divider))
  {
    delay_value = (JER2_ARAD_ING_SCH_MAX_DELAY_VAL*divider);
  }

  /*
   * the delay value has to be an Even Number
   * (for delay in 2 clocks resolution).
   */
  if(is_delay_2_clocks) {
      delay_value = (((delay_value + 1) / 2)*2);
  }

  res = dnx_sand_clocks_to_kbits_per_sec(
          delay_value,
          cal_value,
          device_ticks_per_sec,
          &exact_rate_in_kbits_per_sec
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  jer2_arad_ingress_scheduler_exact_cal_value(
          cal_value,
          max_rate,
          exact_rate_in_kbits_per_sec,
          &exact_cal_value_long
        );

  if (exact_cal_value_long > JER2_ARAD_ING_SCH_MAX_CAL_VAL)
  {
    exact_cal_value_long = JER2_ARAD_ING_SCH_MAX_CAL_VAL;
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ING_SCH_EXACT_CAL_LARGER_THAN_MAXIMUM_VALUE_ERR, 40, exit);
  }

  res = dnx_sand_clocks_to_kbits_per_sec(
          delay_value,
          exact_cal_value_long,
          device_ticks_per_sec,
          &exact_rate_in_kbits_per_sec
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  *shaper_delay_2_clocks = delay_value / divider;
  *shaper_cal = exact_cal_value_long;
  *exact_max_rate = exact_rate_in_kbits_per_sec;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_rate_to_delay_cal_form()",0,0);
}

/*********************************************************************
*     This procedure returns a more exact cal value.
*     in order to get an exact cal value
*     calculate:
*        cal_value * max_rate / exact_rate_in_kbits_per_sec;
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_exact_cal_value(
    DNX_SAND_IN uint32  cal_value,
    DNX_SAND_IN uint32  max_rate,
    DNX_SAND_IN uint32  exact_rate_in_kbits_per_sec,
    DNX_SAND_OUT uint32 *exact_cal_value_long
  )
{

  DNX_SAND_U64
    tmp_result,
    exact_cal_value_u64,
    round_helper;

  dnx_sand_u64_clear(&tmp_result);

  dnx_sand_u64_clear(&exact_cal_value_u64);

  dnx_sand_u64_multiply_longs(
    cal_value,
    max_rate,
    &tmp_result
  );

  dnx_sand_u64_devide_u64_long(
    &tmp_result,
    exact_rate_in_kbits_per_sec,
    &exact_cal_value_u64
  );

  dnx_sand_u64_to_long(
    &exact_cal_value_u64,
    exact_cal_value_long
  );

  dnx_sand_u64_multiply_longs(
    *exact_cal_value_long,
    exact_rate_in_kbits_per_sec,
    &round_helper
  );

  if(dnx_sand_u64_is_bigger(&tmp_result,&round_helper))
  {
    *exact_cal_value_long = *exact_cal_value_long + 1;
  }

  if(*exact_cal_value_long == 0)
  {
    *exact_cal_value_long = 1;
  }

  return 0;
}

/*********************************************************************
*     This procedure perform a test that compares the rate values to
*     the exact rate values that are received after the
*     conversion function.
*     The procedure tests 2 main criteria:
*     1. That the exact error percentage from the rate does not exceed limit.
*     2. That the exact is always larger than rate.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint8
  jer2_arad_ingress_scheduler_conversion_test(
    DNX_SAND_IN uint8 is_regression,
    DNX_SAND_IN uint8 silent
  )
{

  uint32
    ret,
    index,
    rate = JER2_ARAD_ING_SCH_MAX_RATE_MIN,
    tmp_delay,
    tmp_cal,
    exact,
    err_percent,
    max_err_percent=0;

  uint8
    pass = TRUE;

   for (index = JER2_ARAD_ING_SCH_MAX_RATE_MIN; index < JER2_ARAD_ING_SCH_MAX_RATE_MAX; index = (index + 1))
   {
     if(rate > 1000 && is_regression)
     {
       index += JER2_ARAD_CONVERSION_TEST_REGRESSION_STEP;
     }
     rate = index;

     ret = jer2_arad_ingress_scheduler_rate_to_delay_cal_form(
       0,
       rate,
       TRUE,
       &tmp_delay,
       &tmp_cal,
       &exact
       );

     if(dnx_sand_get_error_code_from_error_word(ret) != DNX_SAND_OK)
     {
       if (!silent)
       {
         LOG_INFO(BSL_LS_SOC_INGRESS,
                  (BSL_META("jer2_arad_ingress_scheduler_conversion_test:"
                            "jer2_arad_ingress_scheduler_rate_to_delay_cal_form FAIL (100)"
                            "\n\r"
                      )));
       }
       pass = FALSE;
       goto exit;
     }

     if(exact < rate)
     {
       if (!silent)
       {
         LOG_INFO(BSL_LS_SOC_INGRESS,
                  (BSL_META("jer2_arad_ingress_scheduler_conversion_test: FAIL (200)"
                            "exact rate value is smaller than rate"
                            " \n\r"
                      )));
       }
       pass = FALSE;
       goto exit;
     }

     if((exact - rate) > JER2_ARAD_CONVERSION_TEST_MAX_RATE_DIFF)
     {
       if (!silent)
       {
         LOG_INFO(BSL_LS_SOC_INGRESS,
                  (BSL_META("jer2_arad_ingress_scheduler_conversion_test: FAIL (300)"
                            "difference between exact_rate and rate is OUT OF LIMIT"
                            "\n\r"
                      )));
       }
       pass = FALSE;
       goto exit;
     }

     err_percent = (((exact - rate)*10000)/rate);
     if (err_percent > 10)
     {
       if (!silent)
       {
         LOG_INFO(BSL_LS_SOC_INGRESS,
                  (BSL_META("jer2_arad_ingress_scheduler_conversion_test: FAIL (400)"
                            "error percentage is OUT OF LIMIT"
                            "\n\r"
                      )));
       }
        pass = FALSE;
        goto exit;
     }
     if(max_err_percent < err_percent)
     {
       max_err_percent = err_percent;
     }
   }
   if (!silent)
   {
     LOG_INFO(BSL_LS_SOC_INGRESS,
              (BSL_META("jer2_arad_ingress_scheduler_conversion_test:"
                        "\n\r"
                        "max_err_percent =  %u . %02u \n\r"),
               max_err_percent/100,
               max_err_percent%100
               ));
   }

exit:
   return pass;
}

/*********************************************************************
*     This procedure writes the values of the high priority shapers
*     (fabric multicast, fabric unicast and local) in the clos_info
*     structure to the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_hp_shapers_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
    )
{

  uint32
    res,
    exact_local_max_rate,
    exact_fabm_max_rate,
    exact_fabu_max_rate;
  
  reg_field max_credit, delay, cal;
  JER2_ARAD_ING_SCH_SHAPER shaper_temp;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_HP_SHAPERS_SET);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_clos_info);

  /* Set HP Shapers { */

  /* Set HP Local Shaper */
  max_credit.reg = IPT_SHAPER_23_MAX_CREDITr;
  max_credit.field = SHAPER_2_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_23_DELAYr;
  delay.field = SHAPER_2_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_23_CALr;
  cal.field = SHAPER_2_CALf;
  cal.index = 0;
  
  res = jer2_arad_ingress_scheduler_shaper_values_set(
    unit,
    TRUE,
    &(clos_info->shapers.hp.local),
    &max_credit,
    &delay,
    &cal,
    &exact_local_max_rate
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,1,exit);
  exact_clos_info->shapers.hp.local.max_rate = exact_local_max_rate;

  /* Set HP Unicast Shaper */
  max_credit.field = SHAPER_3_MAX_CREDITf;
  delay.field = SHAPER_3_DELAYf;
  cal.field = SHAPER_3_CALf;
  
  res = jer2_arad_ingress_scheduler_shaper_values_set(
    unit,
    TRUE,
    &(clos_info->shapers.hp.fabric_unicast),
    &max_credit,
    &delay,
    &cal,
    &exact_fabu_max_rate
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);
  exact_clos_info->shapers.hp.fabric_unicast.max_rate = exact_fabu_max_rate;

  /* Set HP Multicast Shaper */
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_4_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_45_DELAYr;
  delay.field = SHAPER_4_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_45_CALr;
  cal.field = SHAPER_4_CALf;
  cal.index = 0;
  
  res = jer2_arad_ingress_scheduler_shaper_values_set(
    unit,
    TRUE,
    &(clos_info->shapers.hp.fabric_multicast),
    &max_credit,
    &delay,
    &cal,
    &exact_fabm_max_rate
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,3,exit);
  exact_clos_info->shapers.hp.fabric_multicast.max_rate = exact_fabm_max_rate;

  /*Set HP multicast values of slow start mechanism*/
  /*Enable field*/
  if (clos_info->shapers.hp.fabric_multicast.slow_start_enable != DNX_TMC_ING_SCH_DONT_TOUCH)
  {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  7,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_4_SLOW_START_ENABLEf,  clos_info->shapers.hp.fabric_multicast.slow_start_enable ? 1  : 0 ));
  }
  /*Set HP Multicast slow rate phase 0*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_4_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_4_SLOW_START_DELAYr;
  delay.field = SHAPER_4_SLOW_START_DELAY_0f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_4_SLOW_START_CALr;
  cal.field = SHAPER_4_SLOW_START_CAL_0f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  if (clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_0  == DNX_TMC_ING_SCH_DONT_TOUCH) {
      shaper_temp.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
      shaper_temp.max_burst = exact_clos_info->shapers.hp.fabric_multicast.max_burst;
  } else {
      shaper_temp.max_rate = exact_clos_info->shapers.hp.fabric_multicast.max_rate * clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_0 / 100;
      shaper_temp.max_burst = exact_clos_info->shapers.hp.fabric_multicast.max_burst;
  }
  res = jer2_arad_ingress_scheduler_shaper_values_set(
     unit,
     TRUE,
     &shaper_temp,
     &max_credit,
     &delay,
     &cal,
     &exact_fabm_max_rate
  );
  DNX_SAND_CHECK_FUNC_RESULT(res,5,exit);
  if (exact_clos_info->shapers.hp.fabric_multicast.max_rate != 0)
  {
      exact_clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_0  = exact_fabm_max_rate * 100 / exact_clos_info->shapers.hp.fabric_multicast.max_rate;
  } else {
      exact_clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_0  = 0;
  }
  /*Set HP Multicast slow rate phase 1*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_4_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_4_SLOW_START_DELAYr;
  delay.field = SHAPER_4_SLOW_START_DELAY_1f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_4_SLOW_START_CALr;
  cal.field = SHAPER_4_SLOW_START_CAL_1f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  if (clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_1  == DNX_TMC_ING_SCH_DONT_TOUCH) {
      shaper_temp.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
      shaper_temp.max_burst = exact_clos_info->shapers.hp.fabric_multicast.max_burst;
  } else {
      shaper_temp.max_rate = exact_clos_info->shapers.hp.fabric_multicast.max_rate * clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_1 / 100;
      shaper_temp.max_burst = exact_clos_info->shapers.hp.fabric_multicast.max_burst;
  }
  res = jer2_arad_ingress_scheduler_shaper_values_set(
     unit,
     TRUE,
     &shaper_temp,
     &max_credit,
     &delay,
     &cal,
     &exact_fabm_max_rate
  );
  DNX_SAND_CHECK_FUNC_RESULT(res,6,exit);
  if (exact_clos_info->shapers.hp.fabric_multicast.max_rate != 0)
  {
      exact_clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_1  = exact_fabm_max_rate * 100 / exact_clos_info->shapers.hp.fabric_multicast.max_rate;
  } else {
      exact_clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_1  = 0;
  }


  /* } Set HP Shapers */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_hp_shapers_set()",0,0);
}


/*********************************************************************
*     This procedure reads the values of the high priority shapers
*     (fabric multicast, fabric unicast and local) to the clos_info
*     structure from the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_hp_shapers_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  )
{
  uint32
    res;

  reg_field max_credit, delay, cal;
  JER2_ARAD_ING_SCH_SHAPER shaper_temp;
  uint32 slow_start_enable;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_HP_SHAPERS_GET);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);

  /* Get HP Shapers { */

  /* Get HP Local Shaper */
  max_credit.reg = IPT_SHAPER_23_MAX_CREDITr;
  max_credit.field = SHAPER_2_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_23_DELAYr;
  delay.field = SHAPER_2_DELAYf;
  delay.index = 0;;
  cal.reg = IPT_SHAPER_23_CALr;
  cal.field = SHAPER_2_CALf;
  cal.index = 0;
  
  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &(clos_info->shapers.hp.local)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,1,exit);

  /* Get HP Unicast Shaper */
  max_credit.field = SHAPER_3_MAX_CREDITf;
  delay.field = SHAPER_3_DELAYf;
  cal.field = SHAPER_3_CALf;

  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &(clos_info->shapers.hp.fabric_unicast)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);

  /* Get HP Multicast Shaper */
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_4_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_45_DELAYr;
  delay.field = SHAPER_4_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_45_CALr;
  cal.field = SHAPER_4_CALf;
  cal.index = 0;

  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &(clos_info->shapers.hp.fabric_multicast)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,3,exit);

  /*Get HP multicast values of slow start mechanism*/
  /*Enable field*/
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_4_SLOW_START_ENABLEf, &slow_start_enable));
  clos_info->shapers.hp.fabric_multicast.slow_start_enable = (slow_start_enable == 0) ? 0 : 1;
  /*Get HP Multicast slow rate phase 0*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_4_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_4_SLOW_START_DELAYr;
  delay.field = SHAPER_4_SLOW_START_DELAY_0f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_4_SLOW_START_CALr;
  cal.field = SHAPER_4_SLOW_START_CAL_0f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  res = jer2_arad_ingress_scheduler_shaper_values_get(
    unit,
    TRUE,
    &max_credit,
    &delay,
    &cal,
    &shaper_temp
  );
  DNX_SAND_CHECK_FUNC_RESULT(res,5,exit);
  if (clos_info->shapers.hp.fabric_multicast.max_rate != 0)
  {
      clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_0 = shaper_temp.max_rate * 100 / clos_info->shapers.hp.fabric_multicast.max_rate;
  } else {
      clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_0 = 0;
  }

  /*Get HP Multicast slow rate phase 1*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_4_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_4_SLOW_START_DELAYr;
  delay.field = SHAPER_4_SLOW_START_DELAY_1f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_4_SLOW_START_CALr;
  cal.field = SHAPER_4_SLOW_START_CAL_1f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  res = jer2_arad_ingress_scheduler_shaper_values_get(
    unit,
    TRUE,
    &max_credit,
    &delay,
    &cal,
    &shaper_temp
  );
  DNX_SAND_CHECK_FUNC_RESULT(res,5,exit);
  if (clos_info->shapers.hp.fabric_multicast.max_rate != 0)
  {
      clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_1 = shaper_temp.max_rate * 100 / clos_info->shapers.hp.fabric_multicast.max_rate;
  } else {
      clos_info->shapers.hp.fabric_multicast.slow_start_rate_phase_1 = 0;
  }
  
  /* } Get HP Shapers */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_hp_shapers_get()",0,0);
}

/*********************************************************************
*     This procedure writes the values of the high priority shapers
*     (fabric multicast, fabric unicast and local) in the clos_info
*     structure to the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_lp_shapers_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
    )
{

  uint32
    res,
    exact_fabm_max_rate,
    exact_fabu_max_rate;
  
  reg_field max_credit, delay, cal;
  JER2_ARAD_ING_SCH_SHAPER shaper_temp;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_LP_SHAPERS_SET);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_clos_info);

  /* Set LP Shapers { */

  /* Set LP Multicast Shaper */
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_5_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_45_DELAYr;
  delay.field = SHAPER_5_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_45_CALr;
  cal.field = SHAPER_5_CALf;
  cal.index = 0;
  
  res = jer2_arad_ingress_scheduler_shaper_values_set(
    unit,
    TRUE,
    &(clos_info->shapers.lp.fabric_multicast),
    &max_credit,
    &delay,
    &cal,
    &exact_fabm_max_rate
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,1,exit);
  exact_clos_info->shapers.lp.fabric_multicast.max_rate = exact_fabm_max_rate;

  /*Set LP multicast values of slow start mechanism*/
  /*Enable field*/
  if (clos_info->shapers.lp.fabric_multicast.slow_start_enable  != DNX_TMC_ING_SCH_DONT_TOUCH) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  7,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_5_SLOW_START_ENABLEf,  clos_info->shapers.lp.fabric_multicast.slow_start_enable ? 1  : 0 ));
  } 

  /*Set LP Multicast slow rate phase 0*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_5_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_5_SLOW_START_DELAYr;
  delay.field = SHAPER_5_SLOW_START_DELAY_0f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_5_SLOW_START_CALr;
  cal.field = SHAPER_5_SLOW_START_CAL_0f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  if (clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_0 == DNX_TMC_ING_SCH_DONT_TOUCH)
  {
      shaper_temp.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
      shaper_temp.max_burst = exact_clos_info->shapers.lp.fabric_multicast.max_burst;
  } else {
      shaper_temp.max_rate = exact_clos_info->shapers.lp.fabric_multicast.max_rate * clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_0 / 100;
      shaper_temp.max_burst = exact_clos_info->shapers.lp.fabric_multicast.max_burst;
  }
  res = jer2_arad_ingress_scheduler_shaper_values_set(
     unit,
     TRUE,
     &shaper_temp,
     &max_credit,
     &delay,
     &cal,
     &exact_fabm_max_rate
  );
  DNX_SAND_CHECK_FUNC_RESULT(res,5,exit);
  if (exact_clos_info->shapers.lp.fabric_multicast.max_rate != 0)
  {
      exact_clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_0  = exact_fabm_max_rate * 100 / exact_clos_info->shapers.lp.fabric_multicast.max_rate;
  } else {
      exact_clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_0  = 0;
  }

  /*Set lP Multicast slow rate phase 1*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_5_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_5_SLOW_START_DELAYr;
  delay.field = SHAPER_5_SLOW_START_DELAY_1f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_5_SLOW_START_CALr;
  cal.field = SHAPER_5_SLOW_START_CAL_1f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  if (clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_1 == DNX_TMC_ING_SCH_DONT_TOUCH)
  {
      shaper_temp.max_rate = DNX_TMC_ING_SCH_DONT_TOUCH;
      shaper_temp.max_burst = exact_clos_info->shapers.lp.fabric_multicast.max_burst;
  } else {
      shaper_temp.max_rate = exact_clos_info->shapers.lp.fabric_multicast.max_rate * clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_1 / 100;
      shaper_temp.max_burst = exact_clos_info->shapers.lp.fabric_multicast.max_burst;
  }
  res = jer2_arad_ingress_scheduler_shaper_values_set(
       unit,
       TRUE,
       &shaper_temp,
       &max_credit,
       &delay,
       &cal,
       &exact_fabm_max_rate
  );
  DNX_SAND_CHECK_FUNC_RESULT(res,6,exit);
  if (exact_clos_info->shapers.lp.fabric_multicast.max_rate != 0)
  {
      exact_clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_1  = exact_fabm_max_rate * 100 / exact_clos_info->shapers.lp.fabric_multicast.max_rate;
  } else {
      exact_clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_1 = 0;
  }

  


  /* Set LP Unicast Shaper */
  max_credit.reg = IPT_SHAPER_67_MAX_CREDITr;
  max_credit.field = SHAPER_6_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_67_DELAYr;
  delay.field = SHAPER_6_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_67_CALr;
  cal.field = SHAPER_6_CALf;
  cal.index = 0;
  
  res = jer2_arad_ingress_scheduler_shaper_values_set(
    unit,
    TRUE,
    &(clos_info->shapers.lp.fabric_unicast),
    &max_credit,
    &delay,
    &cal,
    &exact_fabu_max_rate
    );
  DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);
  exact_clos_info->shapers.lp.fabric_unicast.max_rate = exact_fabu_max_rate;

  /* } Set LP Shapers */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_lp_shapers_set()",0,0);
}


/*********************************************************************
*     This procedure reads the values of the high priority shapers
*     (fabric multicast, fabric unicast and local) to the clos_info
*     structure from the suitable registers fields.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_lp_shapers_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  )
{
  uint32
    res;
  JER2_ARAD_ING_SCH_SHAPER shaper_temp;
  reg_field max_credit, delay, cal;
  uint32 slow_start_enable;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_CLOS_LP_SHAPERS_GET);

  DNX_SAND_CHECK_NULL_INPUT(clos_info);

  /* Get LP Shapers { */

  /* Get LP Multicast Shaper */
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_5_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_45_DELAYr;
  delay.field = SHAPER_5_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_45_CALr;
  cal.field = SHAPER_5_CALf;
  cal.index = 0;

  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &(clos_info->shapers.lp.fabric_multicast)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,1,exit);


  /*Get LP multicast values of slow start mechanism*/
  /*Enable field*/
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPT_SLOW_START_CFG_TIMER_PERIODr, REG_PORT_ANY, 0, SHAPER_5_SLOW_START_ENABLEf, &slow_start_enable));
  clos_info->shapers.lp.fabric_multicast.slow_start_enable = (slow_start_enable == 0) ? 0 : 1;
  /*Get LP Multicast slow rate phase 0*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_5_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_5_SLOW_START_DELAYr;
  delay.field = SHAPER_5_SLOW_START_DELAY_0f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_5_SLOW_START_CALr;
  cal.field = SHAPER_5_SLOW_START_CAL_0f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  res = jer2_arad_ingress_scheduler_shaper_values_get(
        unit,
        TRUE,
        &max_credit,
        &delay,
        &cal,
        &shaper_temp
      );
  DNX_SAND_CHECK_FUNC_RESULT(res,5,exit);
  if (clos_info->shapers.lp.fabric_multicast.max_rate != 0)
  {
      clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_0 = shaper_temp.max_rate * 100 / clos_info->shapers.lp.fabric_multicast.max_rate;
  } else {
      clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_0 = 0;
  }

  /*Get LP Multicast slow rate phase 1*/
  max_credit.reg = IPT_SHAPER_45_MAX_CREDITr;
  max_credit.field = SHAPER_5_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_5_SLOW_START_DELAYr;
  delay.field = SHAPER_5_SLOW_START_DELAY_1f;
  delay.index = 0;
  cal.reg = IPT_SHAPER_5_SLOW_START_CALr;
  cal.field = SHAPER_5_SLOW_START_CAL_1f;
  cal.index = 0;

  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(&shaper_temp);
  res = jer2_arad_ingress_scheduler_shaper_values_get(
        unit,
        TRUE,
        &max_credit,
        &delay,
        &cal,
        &shaper_temp
      );
  DNX_SAND_CHECK_FUNC_RESULT(res,6,exit);
  if (clos_info->shapers.lp.fabric_multicast.max_rate != 0)
  {
      clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_1 = shaper_temp.max_rate * 100 / clos_info->shapers.lp.fabric_multicast.max_rate;
  } else {
      clos_info->shapers.lp.fabric_multicast.slow_start_rate_phase_1 = 0;
  }

  /* Get LP Unicast Shaper */
  max_credit.reg = IPT_SHAPER_67_MAX_CREDITr;
  max_credit.field = SHAPER_6_MAX_CREDITf;
  max_credit.index = 0;
  delay.reg = IPT_SHAPER_67_DELAYr;
  delay.field = SHAPER_6_DELAYf;
  delay.index = 0;
  cal.reg = IPT_SHAPER_67_CALr;
  cal.field = SHAPER_6_CALf;
  cal.index = 0;
  
  res = jer2_arad_ingress_scheduler_shaper_values_get(
          unit,
          TRUE,
          &max_credit,
          &delay,
          &cal,
          &(clos_info->shapers.lp.fabric_unicast)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res,2,exit);
  
  /* } Get LP Shapers */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_clos_lp_shapers_get()",0,0);
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

