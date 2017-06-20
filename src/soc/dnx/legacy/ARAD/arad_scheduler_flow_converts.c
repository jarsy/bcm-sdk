#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_scheduler_flow_converts.c,v 1.8 Broadcom SDK $
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
#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>

#include <soc/dnx/legacy/ARAD/arad_scheduler_flow_converts.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flows.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_elements.h>

#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>
#include <soc/dnx/legacy/ARAD/arad_chip_tbls.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/mbcm.h>

#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/*
 * HR COS Strict Priority classes
 */
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF1           0xfc
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF2           0xfd
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF3           0xfe
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_BE            0xff
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF4           0x30
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF5           0x70
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF6           0xb0
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF7           0xf0
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF8           0xe4
#define JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF9           0xe1

/*
 * CL COS Strict Priority classes
 */
#define JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP_ENCHANCED  0xfb
#define JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP1           0xfc
#define JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2           0xfd
#define JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3           0xfe
#define JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4           0xff

#define JER2_ARAD_SCH_SUB_FLOW_COS_CL_MIN_AF        1
#define JER2_ARAD_SCH_SUB_FLOW_COS_CL_MAX_AF        0xfa

#define JER2_ARAD_SCH_FDM_COS_HR_MAN_MS_BIT         4
#define JER2_ARAD_SCH_FDM_COS_HR_MAN_LS_BIT         0

#define JER2_ARAD_SCH_FDM_COS_HR_EXP_MS_BIT         7
#define JER2_ARAD_SCH_FDM_COS_HR_EXP_LS_BIT         5

#define JER2_ARAD_SCH_SHPR_CLKS_PER_TOKEN           12

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

#define IS_AF_WEIGHT(w) (((w) >= JER2_ARAD_SCH_SUB_FLOW_COS_CL_MIN_AF) && \
((w) <= JER2_ARAD_SCH_SUB_FLOW_COS_CL_MAX_AF))

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



uint32
  jer2_arad_sch_from_internal_HR_weight_convert(
    DNX_SAND_IN     uint32                    internal_weight,
    DNX_SAND_OUT    uint32*                   weight
  )
{
  uint32
    mnt,
    exp;
  uint32
    weight_denominator;
  JER2_ARAD_REG_FIELD
    cos_hr_man,
    cos_hr_exp;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_SCH_FROM_INTERNAL_HR_WEIGHT_CONVERT);

  cos_hr_man.lsb = JER2_ARAD_SCH_FDM_COS_HR_MAN_LS_BIT;
  cos_hr_man.msb = JER2_ARAD_SCH_FDM_COS_HR_MAN_MS_BIT;

  cos_hr_exp.lsb = JER2_ARAD_SCH_FDM_COS_HR_EXP_LS_BIT;
  cos_hr_exp.msb = JER2_ARAD_SCH_FDM_COS_HR_EXP_MS_BIT;

  /*
   * Check input parameter
   */
  DNX_SAND_CHECK_NULL_INPUT(weight);

  mnt = DNX_SAND_GET_FLD_FROM_PLACE(internal_weight,
    JER2_ARAD_FLD_SHIFT_OLD(cos_hr_man),
    JER2_ARAD_FLD_MASK_OLD(cos_hr_man));
  exp = DNX_SAND_GET_FLD_FROM_PLACE(internal_weight,
    JER2_ARAD_FLD_SHIFT_OLD(cos_hr_exp),
    JER2_ARAD_FLD_MASK_OLD(cos_hr_exp));

  dnx_sand_mnt_binary_fraction_exp_to_abs_val(
    JER2_ARAD_FLD_NOF_BITS_OLD(cos_hr_man),
    JER2_ARAD_FLD_NOF_BITS_OLD(cos_hr_exp),
    JER2_ARAD_SCH_FLOW_HR_MAX_WEIGHT,
    mnt,
    exp,
    weight,
    &weight_denominator
    );

  *weight /= weight_denominator;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_from_internal_HR_weight_convert()",0,0);
}



uint32
  jer2_arad_sch_to_internal_HR_weight_convert(
    DNX_SAND_IN     uint32                    weight,
    DNX_SAND_OUT    uint32*                   internal_weight
  )
{
  uint32
    mnt,
    exp;
  JER2_ARAD_REG_FIELD
    cos_hr_man,
    cos_hr_exp;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_SCH_TO_INTERNAL_HR_WEIGHT_CONVERT);
  /*
   * Check input parameter
   */
  DNX_SAND_CHECK_NULL_INPUT(internal_weight);

  cos_hr_man.lsb = JER2_ARAD_SCH_FDM_COS_HR_MAN_LS_BIT;
  cos_hr_man.msb = JER2_ARAD_SCH_FDM_COS_HR_MAN_MS_BIT;

  cos_hr_exp.lsb = JER2_ARAD_SCH_FDM_COS_HR_EXP_LS_BIT;
  cos_hr_exp.msb = JER2_ARAD_SCH_FDM_COS_HR_EXP_MS_BIT;

  dnx_sand_abs_val_to_mnt_binary_fraction_exp(
    weight,
    1,
    JER2_ARAD_FLD_NOF_BITS_OLD(cos_hr_man),
    JER2_ARAD_FLD_NOF_BITS_OLD(cos_hr_exp),
    JER2_ARAD_SCH_FLOW_HR_MAX_WEIGHT,
    &mnt,
    &exp
  );

  DNX_SAND_LIMIT_FROM_ABOVE(mnt, JER2_ARAD_FLD_MAX_OLD(cos_hr_man));
  DNX_SAND_LIMIT_FROM_ABOVE(exp, JER2_ARAD_FLD_MAX_OLD(cos_hr_exp));

  *internal_weight = 0;

  *internal_weight |= DNX_SAND_SET_FLD_IN_PLACE(mnt,
    JER2_ARAD_FLD_SHIFT_OLD(cos_hr_man),
    JER2_ARAD_FLD_MASK_OLD(cos_hr_man));
  *internal_weight |= DNX_SAND_SET_FLD_IN_PLACE(exp,
    JER2_ARAD_FLD_SHIFT_OLD(cos_hr_exp),
    JER2_ARAD_FLD_MASK_OLD(cos_hr_exp));

    /*
     * Some COS values are invalid for the WFQ, because their value
     * is saved value to represent the other wheels.
     * for this purpose, this function, keeps a predefined value
     * for those weights.
     */
  if(*internal_weight == JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF1)
  {
    *internal_weight = 0xCE; /*FC 0.006835938 146.2857 =CE or A7 (exactly same value) 146.2857*/
  }
  else if(*internal_weight == JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF2)
  {
    *internal_weight = 0xCF; /*FD 0.007080078 141.2413793 to ~CF (0.007324219) 136.5333333 */
  }
  else if(*internal_weight == JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF3)
  {
    *internal_weight = 0xCF; /*FE 0.007324219 136.5333333 to CF (exactly same value) 136.5333333*/
  }
  else if(*internal_weight >= JER2_ARAD_SCH_SUB_FLOW_COS_HR_BE)
  {
    *internal_weight = 0xD0; /*FF 0.007568359 132.1290323  to ~D0 or A8 (0.0078125) 128 */
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_to_internal_HR_weight_convert()",0,0);
}

uint32
  jer2_arad_sch_from_internal_HR_subflow_convert(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_HR*        hr_properties,
    DNX_SAND_IN     JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC* internal_sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_SUBFLOW*           sub_flow
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FROM_INTERNAL_HR_SUBFLOW_CONVERT);
  DNX_SAND_CHECK_NULL_INPUT(hr_properties);
  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);

  if (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF1 == internal_sub_flow->cos)
  {
    sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_CLASS_EF1;
  }
  else if (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF2 == internal_sub_flow->cos)
  {
    sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_CLASS_EF2;
  }
  else if (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF3 == internal_sub_flow->cos)
  {
    sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_CLASS_EF3;
  }
  else if (JER2_ARAD_SCH_SUB_FLOW_COS_HR_BE == internal_sub_flow->cos)
  {
    switch (hr_properties->mode)
    {
    case JER2_ARAD_SCH_HR_MODE_SINGLE_WFQ:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_SINGLE_CLASS_BE1;
      break;
    case JER2_ARAD_SCH_HR_MODE_DUAL_WFQ:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE2;
      break;
    case JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE2;
      break;
    default:
      break;
    }
  }
  else if ((JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ == hr_properties->mode) &&
           (internal_sub_flow->hr_sel_dual == FALSE) &&
           ((JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF4 == internal_sub_flow->cos) ||
            (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF5 == internal_sub_flow->cos) ||
            (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF6 == internal_sub_flow->cos) ||
            (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF7 == internal_sub_flow->cos) ||
            (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF8 == internal_sub_flow->cos) ||
            (JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF9 == internal_sub_flow->cos)
           )
          )
  {
    switch (internal_sub_flow->cos)
    {
    case JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF4:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF1;
      break;
    case JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF5:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF2;
      break;
    case JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF6:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF3;
      break;
    case JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF7:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF4;
      break;
    case JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF8:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF5;
      break;
    case JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF9:
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF6;
      break;
    default:
      break;
    }
  }
  else
  {
    if (JER2_ARAD_SCH_HR_MODE_SINGLE_WFQ == hr_properties->mode)
    {
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_SINGLE_CLASS_AF1_WFQ;
    }
    else if (JER2_ARAD_SCH_HR_MODE_DUAL_WFQ == hr_properties->mode)
    {
      sub_flow->credit_source.se_info.hr.sp_class = internal_sub_flow->hr_sel_dual ?
      JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ : JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_AF1_WFQ;
    }
    else if (JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ == hr_properties->mode)
    {
      sub_flow->credit_source.se_info.hr.sp_class = JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ;
    }
    jer2_arad_sch_from_internal_HR_weight_convert(
      internal_sub_flow->cos,
      &sub_flow->credit_source.se_info.hr.weight
    );
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_from_internal_HR_subflow_convert()",0,0);
}

uint32
  jer2_arad_sch_to_internal_HR_subflow_convert(
    DNX_SAND_IN     int                         unit,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_HR*                  hr_properties,
    DNX_SAND_IN     JER2_ARAD_SCH_SUBFLOW*                sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC* internal_sub_flow
  )
{
  uint32
    val;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FROM_INTERNAL_HR_SUBFLOW_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(hr_properties);
  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);
  
  if (JER2_ARAD_SCH_FLOW_HR_CLASS_EF1 == sub_flow->credit_source.se_info.hr.sp_class)
  {
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF1;
  }
  else if (JER2_ARAD_SCH_FLOW_HR_CLASS_EF2 == sub_flow->credit_source.se_info.hr.sp_class)
  {
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF2;
  }
  else if (JER2_ARAD_SCH_FLOW_HR_CLASS_EF3 == sub_flow->credit_source.se_info.hr.sp_class)
  {
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF3;
  }
  else if (
    ((JER2_ARAD_SCH_FLOW_HR_SINGLE_CLASS_BE1 ==
    sub_flow->credit_source.se_info.hr.sp_class) &&
    (JER2_ARAD_SCH_HR_MODE_SINGLE_WFQ == hr_properties->mode)) ||
    ((JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE2 ==
    sub_flow->credit_source.se_info.hr.sp_class) &&
    (JER2_ARAD_SCH_HR_MODE_DUAL_WFQ == hr_properties->mode))   ||
    ((JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE2 ==
    sub_flow->credit_source.se_info.hr.sp_class) &&
    (JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ == hr_properties->mode))
    )
  {
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_BE;
  }
  else if ((JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ == hr_properties->mode) &&
    ((JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF1 ==
    sub_flow->credit_source.se_info.hr.sp_class) ||
    (JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF2 ==
    sub_flow->credit_source.se_info.hr.sp_class) ||
    (JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF3 ==
    sub_flow->credit_source.se_info.hr.sp_class) ||
    (JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF4 ==
    sub_flow->credit_source.se_info.hr.sp_class) ||
    (JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF5 ==
    sub_flow->credit_source.se_info.hr.sp_class) ||
    (JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF6 ==
    sub_flow->credit_source.se_info.hr.sp_class))
    )
  {
    switch (sub_flow->credit_source.se_info.hr.sp_class)
    {
    case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF1:
      internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF4;
      break;
    case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF2:
      internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF5;
      break;
    case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF3:
      internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF6;
      break;
    case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF4:
      internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF7;
      break;
    case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF5:
      internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF8;
      break;
    case JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_AF6:
      internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF9;
      break;
    default:
      break;
    }
  }
  else if (
    /* Single mode WFQ */
    ((JER2_ARAD_SCH_FLOW_HR_SINGLE_CLASS_AF1_WFQ ==
    sub_flow->credit_source.se_info.hr.sp_class) &&
    (JER2_ARAD_SCH_HR_MODE_SINGLE_WFQ == hr_properties->mode)) ||
    /* Dual mode WFQ */
    (((JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ ==
    sub_flow->credit_source.se_info.hr.sp_class) ||
    (JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_AF1_WFQ ==
    sub_flow->credit_source.se_info.hr.sp_class)) &&
    (JER2_ARAD_SCH_HR_MODE_DUAL_WFQ == hr_properties->mode)) ||
    /* Enhanced mode WFQ */
    ((JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ ==
    sub_flow->credit_source.se_info.hr.sp_class) &&
    (JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ == hr_properties->mode))
    )
  {
    if (JER2_ARAD_SCH_HR_MODE_DUAL_WFQ == hr_properties->mode)
    {
      internal_sub_flow->hr_sel_dual =
        (JER2_ARAD_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ ==
        sub_flow->credit_source.se_info.hr.sp_class) ? 1 : 0;
    }

    if(JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ == hr_properties->mode)
    {
      internal_sub_flow->hr_sel_dual =
        (JER2_ARAD_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ ==
        sub_flow->credit_source.se_info.hr.sp_class) ? 1 : 0;
    }

    jer2_arad_sch_to_internal_HR_weight_convert(
      sub_flow->credit_source.se_info.hr.weight,
      &val
    );
    internal_sub_flow->cos = val;
  }
  else
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_HR_AND_SCHEDULER_MODE_MISMATCH_ERR, 20, exit);
  }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_to_internal_HR_subflow_convert()",0,0);
}

uint32
  jer2_arad_sch_from_internal_CL_weight_convert(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     JER2_ARAD_SCH_CL_CLASS_MODE       mode,
    DNX_SAND_IN     uint32                     internal_weight,
    DNX_SAND_OUT    uint32*                     weight
  )
{
  uint32
    i_w,
    max_weight,
    weight_denominator,
    res;
  JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE
    config_mode;
  soc_field_info_t
    cos_fld;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FROM_INTERNAL_CL_WEIGHT_CONVERT);
  /*
  * Check input parameter
  */
  DNX_SAND_CHECK_NULL_INPUT(weight);

  res = jer2_arad_sch_flow_ipf_config_mode_get_unsafe(
          unit,
          &config_mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (config_mode == JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_PROPORTIONAL)
  {
    i_w = (mode == JER2_ARAD_SCH_CL_MODE_5) ?internal_weight:internal_weight/4;
  }
  else if(config_mode == JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_INVERSE)
  {
    max_weight = (mode == JER2_ARAD_SCH_CL_MODE_5) ?
    JER2_ARAD_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODE_5:JER2_ARAD_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODES_3_4;

    JER2_ARAD_TBL_REF(unit, SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm, COSf, &cos_fld);
 

    dnx_sand_mnt_binary_fraction_exp_to_abs_val(
      JER2_ARAD_FLD_NOF_BITS(cos_fld),
      0,
      max_weight,
      internal_weight,
      0,
      &i_w,
      &weight_denominator
      );
    i_w /= weight_denominator;
  }
  else{
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FLD_OUT_OF_RANGE, 40, exit);
  }

  *weight = i_w;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_from_internal_CL_weight_convert()",0,0);
}

uint32
  jer2_arad_sch_to_internal_CL_weight_convert(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     JER2_ARAD_SCH_CL_CLASS_MODE     mode,
    DNX_SAND_IN     uint32                    weight,
    DNX_SAND_OUT    uint32*                  internal_weight
  )
{
  uint32
    max_weight,
    demi_exp,
    weight_calc,
    i_w,
    res;
  JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE
    config_mode;
  soc_field_info_t
    cos_fld;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_TO_INTERNAL_CL_WEIGHT_CONVERT);
  /*
   * Check input parameter
   */
  DNX_SAND_CHECK_NULL_INPUT(internal_weight);

  res = jer2_arad_sch_flow_ipf_config_mode_get_unsafe(
          unit,
          &config_mode
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (config_mode == JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_PROPORTIONAL)
  {
    i_w = weight;
  }
  else if(config_mode == JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_INVERSE)
  {
 
    JER2_ARAD_TBL_REF(unit, SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm, COSf, &cos_fld);

    weight_calc = (mode == JER2_ARAD_SCH_CL_MODE_3) || (mode == JER2_ARAD_SCH_CL_MODE_4) ? (weight * 4) : (weight);
    max_weight = JER2_ARAD_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODE_5;

    dnx_sand_abs_val_to_mnt_binary_fraction_exp(
      weight_calc,
      1,
      JER2_ARAD_FLD_NOF_BITS(cos_fld),
      0,
      max_weight,
      &i_w,
      &demi_exp
      );
  }
  else{
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FLD_OUT_OF_RANGE, 40, exit);
  }
  /* In modes 3, 4, the weight is 6 MSB */
  i_w = (mode == JER2_ARAD_SCH_CL_MODE_3) || (mode == JER2_ARAD_SCH_CL_MODE_4) ? (i_w << 2) : i_w;

  DNX_SAND_LIMIT_VAL(i_w, JER2_ARAD_SCH_SUB_FLOW_COS_CL_MIN_AF, JER2_ARAD_SCH_SUB_FLOW_COS_CL_MAX_AF);

  /* Two LSB should be zeroed in modes 3, 4 */
  i_w = (mode == JER2_ARAD_SCH_CL_MODE_3) || (mode == JER2_ARAD_SCH_CL_MODE_4) ? ((i_w >> 2) << 2):i_w;

  *internal_weight = i_w;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_to_internal_CL_weight_convert()",0,0);
}

uint32
  jer2_arad_sch_from_internal_CL_subflow_convert(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     int                   core,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_CL*            cl_properties,
    DNX_SAND_IN     JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC* internal_sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_SUBFLOW*          sub_flow
  )
{
  uint32
    res;
  JER2_ARAD_SCH_SE_CL_CLASS_INFO
    class_type;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FROM_INTERNAL_CL_SUBFLOW_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(cl_properties);
  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);

  class_type.id = cl_properties->id;
  res = jer2_arad_sch_class_type_params_get_unsafe(
    unit, core,
    class_type.id,
    &class_type
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit);

  /*
   * SP_ENHANCED: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP_ENCHANCED,
   *      (the SE Mode and the WFQ mode - don't care)
   */
  if (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP_ENCHANCED == internal_sub_flow->cos)
  {
    if (class_type.enhanced_mode == JER2_ARAD_CL_ENHANCED_MODE_ENABLED_HP)
    {
      sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP_0_ENHANCED;
    }
    else if (class_type.enhanced_mode == JER2_ARAD_CL_ENHANCED_MODE_ENABLED_LP)
    {
      sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP_5_ENHANCED;
    }
    else
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_AND_SE_TYPE_MISMATCH_ERR, 10, exit);
    }
  }
  /*
   * SP1: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP1, and the se operates
   *      in one of the following configurations:
   *      1. Mode 1 (the WFQ mode is don't care)
   *      2. Mode 2 and discrete WFQ mode
   *      3. Mode 4 (the WFQ mode is don't care)
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP1 == internal_sub_flow->cos) &&
    ((JER2_ARAD_SCH_CL_MODE_1 == class_type.mode)        ||
    ((JER2_ARAD_SCH_CL_MODE_2 == class_type.mode) &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode))             ||
    (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode))
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1;
  }
  /*
   * SP2: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2, and the se operates
   *      in one of the following configurations:
   *      1. Mode 1 (the WFQ mode is don't care)
   *      2. Mode 2 and discrete WFQ mode
   *                   Or
   *      CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4, and the se operates
   *      in the following configuration:
   *      1. Mode 3 (the WFQ mode is don't care)
   */
  else if (
    ((JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2 == internal_sub_flow->cos) &&
    ((JER2_ARAD_SCH_CL_MODE_1 == class_type.mode)        ||
    ((JER2_ARAD_SCH_CL_MODE_2 == class_type.mode) &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode))))
    ||
    ((JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_3 == class_type.mode))
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2;
  }
  /*
   * SP3: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3, and the se operates
   *      in Mode 1 (the WFQ mode is don't care)
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_1 == class_type.mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3;
  }
  /*
   * SP4: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4, and the se operates
   *      in Mode 1 (the WFQ mode is don't care)
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_1 == class_type.mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP4;
  }
  /*
   * SP1_WFQ: CoS is in the range 1-251, and the se operates
   *          in one of the following configurations:
   *          1. Mode 3 and non-discrete WFQ mode
   *          2. Mode 5 and non-discrete WFQ mode
   */
  else if (
    IS_AF_WEIGHT(internal_sub_flow->cos)                       &&
    JER2_ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode)            &&
    ((JER2_ARAD_SCH_CL_MODE_3 == class_type.mode) ||
    (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode))
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ;
    jer2_arad_sch_from_internal_CL_weight_convert(
      unit,
      class_type.mode,
      internal_sub_flow->cos,
      &sub_flow->credit_source.se_info.cl.weight
    );
  }
  /*
   * SP1_WFQ1: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP1, and the se operates
   *           in one of the following configurations:
   *           1. Mode 3 and discrete WFQ mode
   *           2. Mode 5 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP1 == internal_sub_flow->cos) &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)             &&
    ((JER2_ARAD_SCH_CL_MODE_3 == class_type.mode) ||
    (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode))
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ1;
  }
  /*
   * SP1_WFQ2: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2, and the se operates
   *           in one of the following configurations:
   *           1. Mode 3 and discrete WFQ mode
   *           2. Mode 5 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2 == internal_sub_flow->cos) &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)             &&
    ((JER2_ARAD_SCH_CL_MODE_3 == class_type.mode) ||
    (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode))
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ2;
  }
  /*
   * SP1_WFQ3: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3, and the se operates
   *           in one of the following configurations:
   *           1. Mode 3 and discrete WFQ mode
   *           2. Mode 5 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3 == internal_sub_flow->cos) &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)             &&
    ((JER2_ARAD_SCH_CL_MODE_3 == class_type.mode) ||
    (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode))
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ3;
  }
  /*
   * SP1_WFQ4: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4, and the se operates
   *           in Mode 5 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_5 == class_type.mode)      &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ4;
  }
  /*
   * SP2_WFQ: CoS is in the range 1-251, and the se operates
   *          in Mode 4 and non-discrete WFQ mode
   */
  else if (
    IS_AF_WEIGHT(internal_sub_flow->cos)                     &&
    (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode)      &&
    JER2_ARAD_SCH_IS_INDEPENDENT_WFQ_MODE(class_type.weight_mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ;
    jer2_arad_sch_from_internal_CL_weight_convert(
      unit,
      class_type.mode,
      internal_sub_flow->cos,
      &sub_flow->credit_source.se_info.cl.weight
    );
  }
  /*
   * SP2_WFQ1: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2, and the se operates
   *           in Mode 4 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode)      &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ1;
  }
  /*
   * SP2_WFQ2: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3, and the se operates
   *           in Mode 4 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode)      &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ2;
  }
  /*
   * SP2_WFQ3: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4, and the se operates
   *           in Mode 4 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_4 == class_type.mode)      &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ3;
  }
  /*
   * SP3_WFQ1: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3, and the se operates
   *           in Mode 2 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_2 == class_type.mode)      &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ1;
  }
  /*
   * SP3_WFQ2: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4, and the se operates
   *           in Mode 2 and discrete WFQ mode
   */
  else if (
    (JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4 == internal_sub_flow->cos) &&
    (JER2_ARAD_SCH_CL_MODE_2 == class_type.mode)      &&
    JER2_ARAD_SCH_IS_DISCRETE_WFQ_MODE(class_type.weight_mode)
    )
  {
    sub_flow->credit_source.se_info.cl.sp_class = JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ2;
  }
  /*
   * CL-Scheduler Configuration Mismatch
   */
  else
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SE_TYPE_SE_CONFIG_MISMATCH_ERR, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_from_internal_CL_subflow_convert()",0,0);
}

uint32
  jer2_arad_sch_to_internal_CL_subflow_convert(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     int                     core,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_CL*        cl_properties,
    DNX_SAND_IN     JER2_ARAD_SCH_SUBFLOW*           sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC* internal_sub_flow
  )
{
  uint32
    res;
  JER2_ARAD_SCH_SE_CL_CLASS_INFO
    class_type;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_TO_INTERNAL_CL_SUBFLOW_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(cl_properties);
  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);


  class_type.id = cl_properties->id;
  res = jer2_arad_sch_class_type_params_get_unsafe(
    unit, core,
    class_type.id,
    &class_type
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  switch (sub_flow->credit_source.se_info.cl.sp_class)
  {
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ1:
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP1;
    break;

  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2:
    internal_sub_flow->cos = (JER2_ARAD_SCH_CL_MODE_3 == class_type.mode) ?
JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4: JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2;
    break;

  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ2:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ1:
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP2;
    break;

  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ3:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ2:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ1:
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP3;
    break;

  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP4:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ4:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ3:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ2:
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP4;
    break;

  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ:
    jer2_arad_sch_to_internal_CL_weight_convert(
      unit,
      class_type.mode,
      sub_flow->credit_source.se_info.cl.weight,
      &internal_sub_flow->cos
    );
    break;
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP_0_ENHANCED:
  case JER2_ARAD_SCH_SUB_FLOW_CL_CLASS_SP_5_ENHANCED:
    if (class_type.enhanced_mode == JER2_ARAD_CL_ENHANCED_MODE_DISABLED)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_SE_TYPE_SE_CONFIG_MISMATCH_ERR, 20, exit);
    }

    /*
     * SP_ENHANCED: CoS is JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP_ENCHANCED,
     *      (the SE Mode and the WFQ mode - don't care)
     */
    internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_CL_SP_ENCHANCED;
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_FLOW_HR_CLASS_OUT_OF_RANGE_ERR, 50, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_to_internal_CL_subflow_convert()",0,0);
}


uint32
  jer2_arad_sch_from_internal_subflow_shaper_convert(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC  *internal_sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_SUBFLOW           *sub_flow
  )
{
  uint32
    res,
    max_rate_in_kbits_per_sec,
    shaper_weight,
    shaper_weight_denominator,
    credit_worth;
  soc_field_info_t
    peak_rate_man_fld, 
	peak_rate_exp_fld;
  DNX_SAND_U64
    intermediate_val;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_TO_INTERNAL_CL_SUBFLOW_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);

 
 

  /*
   * since we are only interested in number of bits, there is no difference
   * between odd and even fields
   */
  JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_MAN_EVENf, &peak_rate_man_fld);
  JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_EXP_EVENf, &peak_rate_exp_fld);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
    res,10,exit,JER2_ARAD_GET_ERR_TEXT_001,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &credit_worth))) ;

  max_rate_in_kbits_per_sec = ((credit_worth * DNX_SAND_NOF_BITS_IN_CHAR) *
    jer2_arad_chip_mega_ticks_per_sec_get(unit)) / JER2_ARAD_SCH_SHPR_CLKS_PER_TOKEN;
  max_rate_in_kbits_per_sec *= 1000;

  if (
      (internal_sub_flow->peak_rate_man == JER2_ARAD_FLD_MAX(peak_rate_man_fld)) &&
      (internal_sub_flow->peak_rate_exp == 0)
     )
  {
    sub_flow->shaper.max_rate = JER2_ARAD_SCH_SUB_FLOW_SHAPE_NO_LIMIT;
  }
  else
  {
    dnx_sand_mnt_binary_fraction_exp_to_abs_val(JER2_ARAD_FLD_NOF_BITS(peak_rate_man_fld),
      JER2_ARAD_FLD_NOF_BITS(peak_rate_exp_fld),
      max_rate_in_kbits_per_sec,
      internal_sub_flow->peak_rate_man,
      internal_sub_flow->peak_rate_exp,
      &shaper_weight,
      &shaper_weight_denominator
      );
    if(!shaper_weight)
    {
      sub_flow->shaper.max_rate = max_rate_in_kbits_per_sec;
    }
    else
    {
      dnx_sand_u64_multiply_longs(
        max_rate_in_kbits_per_sec,
        shaper_weight_denominator,
        &intermediate_val
      );
      dnx_sand_u64_devide_u64_long(
        &intermediate_val,
        shaper_weight,
        &intermediate_val
      );

      if(intermediate_val.arr[1] != 0)
      {
        intermediate_val.arr[1] = 0;
        intermediate_val.arr[0] = 0xFFFFFFFF;
      }
      sub_flow->shaper.max_rate = intermediate_val.arr[0];
    }
  }

  sub_flow->shaper.max_burst = credit_worth * internal_sub_flow->max_burst;

  /*
   *  Slow rate index
   */
  switch(internal_sub_flow->slow_rate_index) {
    case 0x0:
      sub_flow->slow_rate_ndx = JER2_ARAD_SCH_SLOW_RATE_NDX_1;
      break;
    case 0x1:
      sub_flow->slow_rate_ndx = JER2_ARAD_SCH_SLOW_RATE_NDX_2;
      break;
    default:
      sub_flow->slow_rate_ndx = JER2_ARAD_SCH_SLOW_RATE_NDX_1;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_from_internal_subflow_shaper_convert()",0,0);
}

/* 
 * Returns TRUE if weight1/denominator1
 * is bigger then  weight2/denominator2
 */
static
  uint8
    jer2_arad_sch_weight1_is_bigger(
      DNX_SAND_IN uint32 weight1,
      DNX_SAND_IN uint32 denominator1,
      DNX_SAND_IN uint32 weight2,
      DNX_SAND_IN uint32 denominator2
    )
{
  DNX_SAND_U64
    val64_1,
    val64_2;
  uint32
    is_bigger;
  
  dnx_sand_u64_multiply_longs(
    weight1,
    1000,
    &val64_1
    );
  dnx_sand_u64_devide_u64_long(
    &val64_1,
    denominator1,
    &val64_1
    );

  dnx_sand_u64_multiply_longs(
    weight2,
    1000,
    &val64_2
    );
  dnx_sand_u64_devide_u64_long(
    &val64_2,
    denominator2,
    &val64_2
    );

  is_bigger = dnx_sand_u64_is_bigger(&val64_1, &val64_2);
  return DNX_SAND_NUM2BOOL(is_bigger);
}

uint32
  jer2_arad_sch_to_internal_subflow_shaper_convert(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     JER2_ARAD_SCH_SUBFLOW           *sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC  *internal_sub_flow,
    DNX_SAND_IN     uint32                     round_up
  )
{
  uint32
    res,
    max_rate_in_kbits_per_sec,
    shaper_weight_numerator,
    shaper_weight_denominator,
    exact_shaper_weight,
    exact_shaper_weight_denominator,
    credit_worth,
    peak_rate_man,
    peak_rate_exp;
  soc_field_info_t
    peak_rate_man_fld, 
	peak_rate_exp_fld,
    max_burst_fld;
  DNX_SAND_U64
    intermediate_numerator;
                                                    

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_TO_INTERNAL_SUBFLOW_SHAPER_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);

  /*
   * since we are only interested in number of bits / max value, there is no difference
   * between odd and even fields
   */
  JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_MAN_EVENf, &peak_rate_man_fld);
  JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_EXP_EVENf, &peak_rate_exp_fld);
  JER2_ARAD_TBL_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, MAX_BURST_EVENf, &max_burst_fld);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
    res,10,exit,JER2_ARAD_GET_ERR_TEXT_001,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit, &credit_worth))) ;

  max_rate_in_kbits_per_sec = (
    (credit_worth * DNX_SAND_NOF_BITS_IN_CHAR) * jer2_arad_chip_mega_ticks_per_sec_get(unit))
    / JER2_ARAD_SCH_SHPR_CLKS_PER_TOKEN;
  max_rate_in_kbits_per_sec *= 1000;

  if (
       (0 == sub_flow->shaper.max_rate) ||
       (JER2_ARAD_SCH_SUB_FLOW_SHAPE_NO_LIMIT == sub_flow->shaper.max_rate)
     )
  {
    internal_sub_flow->peak_rate_man = JER2_ARAD_FLD_MAX(peak_rate_man_fld);
    internal_sub_flow->peak_rate_exp = 0;
  }
  else
  {
    shaper_weight_denominator = 64;
    dnx_sand_u64_multiply_longs(
      max_rate_in_kbits_per_sec,
      shaper_weight_denominator,
      &intermediate_numerator
    );
    dnx_sand_u64_devide_u64_long(
      &intermediate_numerator,
      sub_flow->shaper.max_rate,
      &intermediate_numerator
    );

    if(intermediate_numerator.arr[1] != 0)
    {
      intermediate_numerator.arr[1] = 0;
      intermediate_numerator.arr[0] = 0xFFFFFFFF;
    }
    shaper_weight_numerator = intermediate_numerator.arr[0];
    
    dnx_sand_abs_val_to_mnt_binary_fraction_exp(
      shaper_weight_numerator,
      shaper_weight_denominator,
      JER2_ARAD_FLD_NOF_BITS(peak_rate_man_fld),
      JER2_ARAD_FLD_NOF_BITS(peak_rate_exp_fld),
      max_rate_in_kbits_per_sec,
      &peak_rate_man,
      &peak_rate_exp
    );

    internal_sub_flow->peak_rate_man = peak_rate_man;
    internal_sub_flow->peak_rate_exp = peak_rate_exp;

    dnx_sand_mnt_binary_fraction_exp_to_abs_val(
      JER2_ARAD_FLD_NOF_BITS(peak_rate_man_fld),
      JER2_ARAD_FLD_NOF_BITS(peak_rate_exp_fld),
      max_rate_in_kbits_per_sec,
      peak_rate_man,
      peak_rate_exp,
      &exact_shaper_weight,
      &exact_shaper_weight_denominator
      );

    if(round_up)
    {
      if (jer2_arad_sch_weight1_is_bigger(exact_shaper_weight, exact_shaper_weight_denominator, shaper_weight_numerator, shaper_weight_denominator))
      {
        if (internal_sub_flow->peak_rate_man == JER2_ARAD_FLD_MAX(peak_rate_man_fld))
        {
          if (internal_sub_flow->peak_rate_exp > 0)
          {
            internal_sub_flow->peak_rate_man = DNX_SAND_DIV_ROUND_UP(internal_sub_flow->peak_rate_man,2);
            internal_sub_flow->peak_rate_exp--;
          }
        }
        else
        {
          internal_sub_flow->peak_rate_man++;
        }
      }
    }

    /*
     * Just in case - shouldn't get here
     */
    DNX_SAND_LIMIT_FROM_ABOVE(internal_sub_flow->peak_rate_man, JER2_ARAD_FLD_MAX(peak_rate_man_fld));
    DNX_SAND_LIMIT_FROM_ABOVE(internal_sub_flow->peak_rate_exp, JER2_ARAD_FLD_MAX(peak_rate_exp_fld));

      /* The FAP can't support Exp == 15 and bit 0 of the Man == 1*/
    if( ((internal_sub_flow->peak_rate_man & 0x1) == 1) &&
      (internal_sub_flow->peak_rate_exp == 15)
      )
    {
      internal_sub_flow->peak_rate_man = DNX_SAND_DIV_ROUND_UP(internal_sub_flow->peak_rate_man,2);
      internal_sub_flow->peak_rate_exp--;
    }
  }

  internal_sub_flow->max_burst = DNX_SAND_DIV_ROUND_UP(sub_flow->shaper.max_burst,credit_worth);
  DNX_SAND_LIMIT_FROM_ABOVE(internal_sub_flow->max_burst, JER2_ARAD_FLD_MAX(max_burst_fld));

  internal_sub_flow->max_burst_update = 1;

  /*
   *  Slow rate index
   */
  switch(sub_flow->slow_rate_ndx) {
    case JER2_ARAD_SCH_SLOW_RATE_NDX_1:
      internal_sub_flow->slow_rate_index = 0x0;
      break;
    case JER2_ARAD_SCH_SLOW_RATE_NDX_2:
      internal_sub_flow->slow_rate_index = 0x1;
      break;
    default:
      internal_sub_flow->slow_rate_index = 0x0;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_to_internal_subflow_shaper_convert()",0,0);
}

uint32
  jer2_arad_sch_INTERNAL_SUB_FLOW_to_SUB_FLOW_convert(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     int                     core,
    DNX_SAND_IN     JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC  *internal_sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_SUBFLOW           *sub_flow
  )
{
  uint32
    res;
  JER2_ARAD_SCH_SE_INFO
    se;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_INTERNAL_SUB_FLOW_TO_SUB_FLOW_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);

  /*
   * Credit source
   */
  se.id = sub_flow->credit_source.id = internal_sub_flow->sch_number;

  res = jer2_arad_sch_se_get_unsafe(
    unit, core,
    se.id,
    &se
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /*
   * Shaper
   */
  res = jer2_arad_sch_from_internal_subflow_shaper_convert(
          unit,
          internal_sub_flow,
          sub_flow
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /*
   * Translate the sub-flow properties according to the se type
   */
  switch (se.type)
  {
  case JER2_ARAD_SCH_SE_TYPE_HR:
    sub_flow->credit_source.se_type = JER2_ARAD_SCH_SE_TYPE_HR;
    res = jer2_arad_sch_from_internal_HR_subflow_convert(
            unit,
            &se.type_info.hr,
            internal_sub_flow,
            sub_flow
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    break;

  case JER2_ARAD_SCH_SE_TYPE_CL:
    sub_flow->credit_source.se_type = JER2_ARAD_SCH_SE_TYPE_CL;
    res = jer2_arad_sch_from_internal_CL_subflow_convert(
      unit, core,
      &se.type_info.cl,
      internal_sub_flow,
      sub_flow);
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    break;

  case JER2_ARAD_SCH_SE_TYPE_FQ:
    sub_flow->credit_source.se_type = JER2_ARAD_SCH_SE_TYPE_FQ;
    break;

  default:
    sub_flow->credit_source.se_type = JER2_ARAD_SCH_SE_TYPE_NONE;
    break;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_sch_INTERNAL_SUB_FLOW_to_SUB_FLOW_convert()",0,0);
}

uint32
  jer2_arad_sch_SUB_FLOW_to_INTERNAL_SUB_FLOW_convert(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     int                     core,
    DNX_SAND_IN     JER2_ARAD_SCH_SUBFLOW           *sub_flow,
    DNX_SAND_OUT    JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC  *internal_sub_flow
  )
{
  uint32
    res;
  JER2_ARAD_SCH_SE_INFO
    se;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SUB_FLOW_TO_INTERNAL_SUB_FLOW_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(internal_sub_flow);
  DNX_SAND_CHECK_NULL_INPUT(sub_flow);
  if (!sub_flow->update_bw_only) {
    /*
     * Credit source
     */
    res = jer2_arad_sch_se_get_unsafe(
          unit, core,
          sub_flow->credit_source.id,
          &se
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    internal_sub_flow->sch_number = sub_flow->credit_source.id;
  }
  /*
   * Shaper
   */
  res = jer2_arad_sch_to_internal_subflow_shaper_convert(
          unit,
          sub_flow,
          internal_sub_flow,
          TRUE
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (!sub_flow->update_bw_only) {
    /*
     * Translate the sub-flow properties according to the se type
     */
    internal_sub_flow->hr_sel_dual = 0;
    switch (se.type)
    {
    case JER2_ARAD_SCH_SE_TYPE_HR:
      res = jer2_arad_sch_to_internal_HR_subflow_convert(
              unit,
              &se.type_info.hr,
              sub_flow,
              internal_sub_flow
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
      break;
  
    case JER2_ARAD_SCH_SE_TYPE_CL:
      res = jer2_arad_sch_to_internal_CL_subflow_convert(
              unit, core,
              &se.type_info.cl,
              sub_flow,
              internal_sub_flow
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
      break;
  
    case JER2_ARAD_SCH_SE_TYPE_FQ:
      internal_sub_flow->cos = JER2_ARAD_SCH_SUB_FLOW_COS_HR_EF1;
      break;
  
    default:
      break;
    }
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_SUB_FLOW_to_INTERNAL_SUB_FLOW_convert", 0, 0);
}

uint32
  jer2_arad_sch_INTERNAL_HR_MODE_to_HR_MODE_convert(
    DNX_SAND_IN     uint32                internal_hr_mode,
    DNX_SAND_OUT    JER2_ARAD_SCH_SE_HR_MODE  *hr_mode
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_SCH_INTERNAL_HR_MODE_TO_HR_MODE_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(hr_mode);

  switch (internal_hr_mode)
  {
  case 0x0:
    *hr_mode = JER2_ARAD_SCH_HR_MODE_SINGLE_WFQ;
    break;

  case 0x1:
    *hr_mode = JER2_ARAD_SCH_HR_MODE_DUAL_WFQ;
    break;

  case 0x3:
    *hr_mode = JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ;
    break;

  default:
    *hr_mode = JER2_ARAD_SCH_HR_MODE_NONE;
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_HR_MODE_OUT_OF_RANGE_ERR, 10, exit);
    break;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_INTERNAL_HR_MODE_to_HR_MODE_convert", internal_hr_mode, 0);
}

uint32
  jer2_arad_sch_HR_MODE_to_INTERNAL_HR_MODE_convert(
    DNX_SAND_IN    JER2_ARAD_SCH_SE_HR_MODE  hr_mode,
    DNX_SAND_OUT   uint32               *internal_hr_mode
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_SCH_INTERNAL_HR_MODE_TO_HR_MODE_CONVERT);

  DNX_SAND_CHECK_NULL_INPUT(internal_hr_mode);

  switch (hr_mode)
  {
  case JER2_ARAD_SCH_HR_MODE_SINGLE_WFQ:
    *internal_hr_mode = 0x0;
    break;

  case JER2_ARAD_SCH_HR_MODE_DUAL_WFQ:
    *internal_hr_mode = 0x1;
    break;

  case JER2_ARAD_SCH_HR_MODE_ENHANCED_PRIO_WFQ:
    *internal_hr_mode = 0x3;
    break;

  default:
    *internal_hr_mode = 0x0;
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SCH_HR_MODE_OUT_OF_RANGE_ERR, 10, exit);
    break;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("jer2_arad_sch_HR_MODE_to_INTERNAL_HR_MODE_convert", 0, 0);
}


/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */

