/* $Id: jer2_tmc_api_debug.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_debug.c
*
* MODULE PREFIX:  Soc_petra
*
* SYSTEM:         DuneDriver/jer2_tmc/src/
*
* FILE DESCRIPTION: Different APIs for helping for debugging.
*
* REMARKS:  None.
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
*******************************************************************/

/*************
 * INCLUDES  *
 *************/
/* { */


#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/TMC/tmc_api_debug.h>

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

void
  DNX_TMC_DBG_AUTOCREDIT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_DBG_AUTOCREDIT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_DBG_AUTOCREDIT_INFO));
  info->first_queue = 0;
  info->last_queue = 0;
  info->rate = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_DBG_FORCE_MODE_to_string(
    DNX_SAND_IN DNX_TMC_DBG_FORCE_MODE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_DBG_FORCE_MODE_NONE:
    str = "none";
  break;
  case DNX_TMC_DBG_FORCE_MODE_LOCAL:
    str = "local";
  break;
  case DNX_TMC_DBG_FORCE_MODE_FABRIC:
    str = "fabric";
  break;
  case DNX_TMC_DBG_NOF_FORCE_MODES:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_DBG_FLUSH_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_DBG_FLUSH_MODE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_DBG_FLUSH_MODE_DEQUEUE:
    str = "dequeue";
  break;
  case DNX_TMC_DBG_FLUSH_MODE_DELETE:
    str = "delete";
  break;
  case DNX_TMC_DBG_NOF_FLUSH_MODES:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_DBG_AUTOCREDIT_INFO_print(
    DNX_SAND_IN DNX_TMC_DBG_AUTOCREDIT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "first_queue: %u\n\r"),info->first_queue));
  LOG_CLI((BSL_META_U(unit,
                      "last_queue: %u\n\r"),info->last_queue));
  LOG_CLI((BSL_META_U(unit,
                      "rate: %u[Mbps]\n\r"),info->rate));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

