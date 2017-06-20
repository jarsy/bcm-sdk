#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_scheduler_element_converts.c,v 1.6 Broadcom SDK $
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
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_scheduler_element_converts.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_framework.h>
#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
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

/*****************************************************
 * See details in jer2_arad_sch_scheduler.h
 *****************************************************/
DNX_SAND_RET
  jer2_arad_sch_INTERNAL_CLASS_TYPE_to_CLASS_TYPE_convert(
    DNX_SAND_IN     JER2_ARAD_SCH_SCT_TBL_DATA     *internal_class_type,
    DNX_SAND_OUT    JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type
  )
{
  JER2_ARAD_INIT_ERR_DEFS;

  JER2_ARAD_ERR_IF_NULL(class_type, 10);
  JER2_ARAD_ERR_IF_NULL(internal_class_type, 20);

  switch (internal_class_type->clconfig)
  {
  case 15:
    class_type->mode = JER2_ARAD_SCH_CL_MODE_1;
    break;
  case 3:
    class_type->mode = JER2_ARAD_SCH_CL_MODE_2;
    break;
  case 8:
    class_type->mode = JER2_ARAD_SCH_CL_MODE_3;
    break;
  case 1:
    class_type->mode = JER2_ARAD_SCH_CL_MODE_4;
    break;
  case 0:
    class_type->mode = JER2_ARAD_SCH_CL_MODE_5;
    break;
  default:
    JER2_ARAD_SET_ERR_AND_EXIT(JER2_ARAD_SCH_CLCONFIG_OUT_OF_RANGE_ERR);
  }
  class_type->weight[0] = internal_class_type->af0_inv_weight;
  class_type->weight[1] = internal_class_type->af1_inv_weight;
  class_type->weight[2] = internal_class_type->af2_inv_weight;
  class_type->weight[3] = internal_class_type->af3_inv_weight;

  class_type->weight_mode = internal_class_type->wfqmode;

  switch(internal_class_type->enh_clen)
  {
  case 0:
    class_type->enhanced_mode = JER2_ARAD_CL_ENHANCED_MODE_DISABLED;
    break;
  case 1:
    if (internal_class_type->enh_clsphigh)
    {
      class_type->enhanced_mode = JER2_ARAD_CL_ENHANCED_MODE_ENABLED_HP;
    }
    else
    {
      class_type->enhanced_mode = JER2_ARAD_CL_ENHANCED_MODE_ENABLED_LP;
    }
    break;
  default:
    JER2_ARAD_SET_ERR_AND_EXIT(JER2_ARAD_SCH_ENH_MODE_OUT_OF_RANGE_ERR);
  }

exit:
  JER2_ARAD_RETURN;
}

/*****************************************************
 * See details in jer2_arad_sch_scheduler.h
 *****************************************************/
DNX_SAND_RET
  jer2_arad_sch_CLASS_TYPE_to_INTERNAL_CLASS_TYPE_convert(
    DNX_SAND_IN     int                 unit,
    DNX_SAND_IN   JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type,
    DNX_SAND_OUT  JER2_ARAD_SCH_SCT_TBL_DATA     *internal_class_type
  )
{
  DNX_SAND_RET
    ret = DNX_SAND_OK;

  JER2_ARAD_INIT_ERR_DEFS;

  JER2_ARAD_ERR_IF_NULL(class_type, 10);
  JER2_ARAD_ERR_IF_NULL(internal_class_type, 20);

  ret = dnx_sand_os_memset(
    internal_class_type,
    0x0,
    sizeof(JER2_ARAD_SCH_SCT_TBL_DATA)
  );
  JER2_ARAD_EXIT_IF_ERR(ret, 25);

 
 


  switch (class_type->mode)
  {
  case JER2_ARAD_SCH_CL_MODE_1:
    internal_class_type->clconfig = 0xf;
    break;
  case JER2_ARAD_SCH_CL_MODE_2:
    internal_class_type->clconfig = 0x3;
    break;
  case JER2_ARAD_SCH_CL_MODE_3:
    internal_class_type->clconfig = 0x8;
    break;
  case JER2_ARAD_SCH_CL_MODE_4:
    internal_class_type->clconfig = 0x1;
    break;
  case JER2_ARAD_SCH_CL_MODE_5:
    internal_class_type->clconfig = 0x0;
    break;
  default:
    JER2_ARAD_SET_ERR_AND_EXIT(JER2_ARAD_SCH_CLCONFIG_OUT_OF_RANGE_ERR);
  }

  internal_class_type->af0_inv_weight = class_type->weight[0];
  DNX_SAND_LIMIT_FROM_ABOVE(internal_class_type->af0_inv_weight, 0x3FF);

  internal_class_type->af1_inv_weight = class_type->weight[1];
  DNX_SAND_LIMIT_FROM_ABOVE(internal_class_type->af1_inv_weight, 0x3FF);

  internal_class_type->af2_inv_weight = class_type->weight[2];
  DNX_SAND_LIMIT_FROM_ABOVE(internal_class_type->af2_inv_weight, 0x3FF);

  internal_class_type->af3_inv_weight = class_type->weight[3];
  DNX_SAND_LIMIT_FROM_ABOVE(internal_class_type->af3_inv_weight, 0x3FF);

  internal_class_type->wfqmode = class_type->weight_mode;

  /*
   *  There is a limitation in the device, that when work
   *  in mode 1 or mode 2, the weighting can't be
   *  independent per flow
   */
  if(
     (class_type->mode == JER2_ARAD_SCH_CL_MODE_1) ||
     (class_type->mode == JER2_ARAD_SCH_CL_MODE_2)
    )
  {
    if(class_type->weight_mode == JER2_ARAD_SCH_CL_WEIGHTS_MODE_INDEPENDENT_PER_FLOW)
    {
      internal_class_type->wfqmode = JER2_ARAD_SCH_CL_WEIGHTS_MODE_DISCRETE_PER_FLOW;
    }
  }

  /* In Mode 1 the weighting must be Discrete per Flow */
  if(class_type->mode == JER2_ARAD_SCH_CL_MODE_1)
  {
    internal_class_type->wfqmode = JER2_ARAD_SCH_CL_WEIGHTS_MODE_DISCRETE_PER_FLOW;
  }

  switch(class_type->enhanced_mode)
  {
  case JER2_ARAD_CL_ENHANCED_MODE_DISABLED:
    internal_class_type->enh_clen = 0;
    internal_class_type->enh_clsphigh = 0;
    break;
  case JER2_ARAD_CL_ENHANCED_MODE_ENABLED_HP:
    internal_class_type->enh_clen = 1;
    internal_class_type->enh_clsphigh = 1;
    break;
  case JER2_ARAD_CL_ENHANCED_MODE_ENABLED_LP:
    internal_class_type->enh_clen = 1;
    internal_class_type->enh_clsphigh = 0;
    break;
  default:
    JER2_ARAD_SET_ERR_AND_EXIT(JER2_ARAD_SCH_ENH_MODE_OUT_OF_RANGE_ERR);
    break;
  }

exit:
  JER2_ARAD_RETURN;
}


/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */
