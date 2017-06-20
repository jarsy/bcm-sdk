/* $Id: jer2_tmc_api_tdm.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_tdm.c
*
* MODULE PREFIX:  jer2_tmc
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

/*************
 * INCLUDES  *
 *************/
/* { */



#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/TMC/tmc_api_tdm.h>

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
  DNX_TMC_TDM_FTMH_OPT_UC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_OPT_UC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_FTMH_OPT_UC));
  info->dest_if = 0;
  info->dest_fap_id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_OPT_MC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_OPT_MC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_FTMH_OPT_MC));
  info->mc_id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_STANDARD_UC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_STANDARD_UC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_FTMH_STANDARD_UC));
  info->user_def = 0;
  info->sys_phy_port = 0;
  info->user_def_2 = 0;
  info->dest_fap_port = 0;
  info->dest_fap_id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_STANDARD_MC_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_STANDARD_MC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_FTMH_STANDARD_MC));
  info->user_def = 0;
  info->mc_id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_FTMH));
  DNX_TMC_TDM_FTMH_OPT_UC_clear(&(info->opt_uc));
  DNX_TMC_TDM_FTMH_OPT_MC_clear(&(info->opt_mc));
  DNX_TMC_TDM_FTMH_STANDARD_UC_clear(&(info->standard_uc));
  DNX_TMC_TDM_FTMH_STANDARD_MC_clear(&(info->standard_mc));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_FTMH_INFO));
  info->action_ing = DNX_TMC_TDM_NOF_ING_ACTIONS;
  DNX_TMC_TDM_FTMH_clear(&(info->ftmh));
  info->action_eg = DNX_TMC_TDM_NOF_EG_ACTIONS;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_MC_STATIC_ROUTE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TDM_MC_STATIC_ROUTE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_MC_STATIC_ROUTE_INFO));
  dnx_sand_u64_clear(&(info->link_bitmap));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_DIRECT_ROUTING_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TDM_DIRECT_ROUTING_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_TDM_DIRECT_ROUTING_INFO));
  dnx_sand_u64_clear(&(info->link_bitmap));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_TDM_ING_ACTION_to_string(
    DNX_SAND_IN  DNX_TMC_TDM_ING_ACTION enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_TDM_ING_ACTION_ADD:
    str = "add";
  break;
  case DNX_TMC_TDM_ING_ACTION_NO_CHANGE:
    str = "no_change";
  break;
  case DNX_TMC_TDM_ING_ACTION_CUSTOMER_EMBED:
    str = "customer_embed";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_TDM_EG_ACTION_to_string(
    DNX_SAND_IN  DNX_TMC_TDM_EG_ACTION enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_TDM_EG_ACTION_REMOVE:
    str = "remove";
  break;
  case DNX_TMC_TDM_EG_ACTION_NO_CHANGE:
    str = "no_change";
  break;
  case DNX_TMC_TDM_EG_ACTION_CUSTOMER_EXTRACT:
    str = "customer_extract";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_TDM_FTMH_OPT_UC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_OPT_UC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "dest_if: %u\n\r"),info->dest_if));
  LOG_CLI((BSL_META_U(unit,
                      "dest_fap_id: %u\n\r"),info->dest_fap_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_OPT_MC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_OPT_MC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "mc_id: %u\n\r"),info->mc_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_STANDARD_UC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_STANDARD_UC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "user_def: %u\n\r"),info->user_def));
  LOG_CLI((BSL_META_U(unit,
                      "sys_phy_port: %u\n\r"),info->sys_phy_port));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_STANDARD_MC_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_STANDARD_MC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "user_def: %u\n\r"),info->user_def));
  LOG_CLI((BSL_META_U(unit,
                      "mc_id: %u\n\r"),info->mc_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "\n\ropt_uc:\n\r")));
  DNX_TMC_TDM_FTMH_OPT_UC_print(&(info->opt_uc));
  LOG_CLI((BSL_META_U(unit,
                      "opt_mc:\n\r")));
  DNX_TMC_TDM_FTMH_OPT_MC_print(&(info->opt_mc));
  LOG_CLI((BSL_META_U(unit,
                      "standard_uc:\n\r")));
  DNX_TMC_TDM_FTMH_STANDARD_UC_print(&(info->standard_uc));
  LOG_CLI((BSL_META_U(unit,
                      "standard_mc:\n\r")));
  DNX_TMC_TDM_FTMH_STANDARD_MC_print(&(info->standard_mc));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_TDM_FTMH_INFO_print(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "action_ing %s "), DNX_TMC_TDM_ING_ACTION_to_string(info->action_ing)));
  LOG_CLI((BSL_META_U(unit,
                      "ftmh:")));
  DNX_TMC_TDM_FTMH_print(&(info->ftmh));
  LOG_CLI((BSL_META_U(unit,
                      "is_mc: %u\n\r"),info->is_mc));
  LOG_CLI((BSL_META_U(unit,
                      "action_eg %s "), DNX_TMC_TDM_EG_ACTION_to_string(info->action_eg)));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
void
  DNX_TMC_TDM_MC_STATIC_ROUTE_INFO_print(
    DNX_SAND_IN  DNX_TMC_TDM_MC_STATIC_ROUTE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "link_bitmap:")));
  dnx_sand_u64_print(&(info->link_bitmap), 1, 0);

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

