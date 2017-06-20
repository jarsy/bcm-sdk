/* $Id: ppc_api_fp_egr.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PPC

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/PPC/ppc_api_fp_egr.h>

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

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_FP_EGR_MANAGE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_EGR_MANAGE_TYPE  enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case SOC_PPC_FP_EGR_MANAGE_TYPE_ADD:
    str = "add";
  break;
  case SOC_PPC_FP_EGR_MANAGE_TYPE_RMV:
    str = "rmv";
  break;
  case SOC_PPC_FP_EGR_MANAGE_TYPE_GET:
    str = "get";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

