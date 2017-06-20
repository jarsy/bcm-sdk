/* $Id: soc_ppc_api_fp_egr.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_PPC_API_FP_EGR_INCLUDED__
/* { */
#define __SOC_PPC_API_FP_EGR_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  Add an Egress Database -ID.
   */
  SOC_PPC_FP_EGR_MANAGE_TYPE_ADD = 0,
  /*
   *  Remove an Egress Database -ID.
   */
  SOC_PPC_FP_EGR_MANAGE_TYPE_RMV = 1,
  /*
   *  Get an Egress Database -ID.
   */
  SOC_PPC_FP_EGR_MANAGE_TYPE_GET = 2,
  /*
   *  Number of types in SOC_PPC_FP_EGR_MANAGE_TYPE
   */
  SOC_PPC_NOF_FP_EGR_MANAGE_TYPES = 3
}SOC_PPC_FP_EGR_MANAGE_TYPE;


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
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FP_EGR_INCLUDED__*/
#endif

