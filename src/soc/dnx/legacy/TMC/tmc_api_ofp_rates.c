/* $Id: jer2_tmc_api_ofp_rates.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_ofp_rates.c
*
* MODULE PREFIX:  soc_jer2_tmcofp_rates
*
* FILE DESCRIPTION:
*   Rates and burst configuration of the Outgoing FAP Ports.
*   The configuration envolves End-to-end scheduler and Egress processor.
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
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_ofp_rates.h>
#include <soc/dnx/legacy/TMC/tmc_api_general.h>

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
  DNX_TMC_OFP_RATES_MAL_SHPR_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_MAL_SHPR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATES_MAL_SHPR));
  info->rate_update_mode = DNX_TMC_OFP_SHPR_UPDATE_MODE_SUM_OF_PORTS;
  info->rate = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_MAL_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_MAL_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATES_MAL_SHPR_INFO));
  DNX_TMC_OFP_RATES_MAL_SHPR_clear(&(info->sch_shaper));
  DNX_TMC_OFP_RATES_MAL_SHPR_clear(&(info->egq_shaper));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_INTERFACE_SHPR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATES_INTERFACE_SHPR));
  info->rate_update_mode = DNX_TMC_OFP_SHPR_UPDATE_MODE_SUM_OF_PORTS;
  info->rate = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO));
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_clear(&(info->sch_shaper));
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_clear(&(info->egq_shaper));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATE_INFO));
  info->port_id = DNX_TMC_OFP_RATES_ILLEGAL_PORT_ID;
  info->sch_rate = 0;
  info->egq_rate = 0;
  info->max_burst = DNX_TMC_OFP_RATES_BURST_LIMIT_MAX;
  info->port_priority = 0;
  info->tcg_ndx = 0;
  info->sch_max_burst = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_TBL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_TBL_INFO *info
  )
{
  uint32 ind;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATES_TBL_INFO));
  info->nof_valid_entries = 0;
  for (ind=0; ind<DNX_TMC_NOF_FAP_PORTS; ++ind)
  {
    DNX_TMC_OFP_RATE_INFO_clear(&(info->rates[ind]));
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_FAT_PIPE_RATE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_FAT_PIPE_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_FAT_PIPE_RATE_INFO));
  info->sch_rate = 0;
  info->egq_rate = 0;
  info->max_burst = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO));
  DNX_TMC_OFP_RATE_SHPR_INFO_clear(&info->sch_shaper);
  info->sch_shaper.max_burst = DNX_TMC_OFP_RATES_SCH_BURST_LIMIT_MAX;
  DNX_TMC_OFP_RATE_SHPR_INFO_clear(&info->egq_shaper);
  info->egq_shaper.max_burst = DNX_TMC_OFP_RATES_BURST_LIMIT_MAX;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATE_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATE_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATE_SHPR_INFO));
  info->rate = 0;  
  info->max_burst = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_TCG_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_TCG_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_OFP_RATES_TCG_SHPR_INFO));
  DNX_TMC_OFP_RATE_SHPR_INFO_clear(&info->sch_shaper);
  info->sch_shaper.max_burst = DNX_TMC_OFP_RATES_SCH_BURST_LIMIT_MAX;
  DNX_TMC_OFP_RATE_SHPR_INFO_clear(&info->egq_shaper); 
  info->egq_shaper.max_burst = DNX_TMC_OFP_RATES_BURST_LIMIT_MAX;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_OFP_RATES_CAL_SET_to_string(
    DNX_SAND_IN DNX_TMC_OFP_RATES_CAL_SET enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_OFP_RATES_CAL_SET_A:
    str = "a";
  break;
  case DNX_TMC_OFP_RATES_CAL_SET_B:
    str = "b";
  break;
  case DNX_TMC_OFP_NOF_RATES_CAL_SETS:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_OFP_SHPR_UPDATE_MODE_to_string(
    DNX_SAND_IN DNX_TMC_OFP_SHPR_UPDATE_MODE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_OFP_SHPR_UPDATE_MODE_SUM_OF_PORTS:
    str = "sum_of_ports";
  break;
  case DNX_TMC_OFP_SHPR_UPDATE_MODE_OVERRIDE:
    str = "override";
  break;
  case DNX_TMC_OFP_SHPR_UPDATE_MODE_DONT_TUCH:
    str = "dont_tuch";
  break;
  case DNX_TMC_OFP_NOF_SHPR_UPDATE_MODES:
    str = " Not initialized";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_OFP_RATES_MAL_SHPR_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_MAL_SHPR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Rate_update_mode: %s \n\r"),
           DNX_TMC_OFP_SHPR_UPDATE_MODE_to_string(info->rate_update_mode)
           ));
  LOG_CLI((BSL_META_U(unit,
                      "  Rate: %u[Kbps]\n\r"),info->rate));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_MAL_SHPR_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_MAL_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Sch_shaper:\n\r")));
  DNX_TMC_OFP_RATES_MAL_SHPR_print(&(info->sch_shaper));
  LOG_CLI((BSL_META_U(unit,
                      "Egq_shaper:\n\r")));
  DNX_TMC_OFP_RATES_MAL_SHPR_print(&(info->egq_shaper));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_INTERFACE_SHPR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "  Rate_update_mode: %s \n\r"),
           DNX_TMC_OFP_SHPR_UPDATE_MODE_to_string(info->rate_update_mode)
           ));
  LOG_CLI((BSL_META_U(unit,
                      "  Rate: %u[Kbps]\n\r"),info->rate));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Sch_shaper:\n\r")));
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_print(&(info->sch_shaper));
  LOG_CLI((BSL_META_U(unit,
                      "Egq_shaper:\n\r")));
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_print(&(info->egq_shaper));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATE_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Port[%-2u] rate: "),info->port_id));
  LOG_CLI((BSL_META_U(unit,
                      "SCH: %-8u[Kbps], "),info->sch_rate));
  LOG_CLI((BSL_META_U(unit,
                      "EGQ: %-8u[Kbps], "),info->egq_rate));

  if (info->max_burst == DNX_TMC_OFP_RATES_BURST_LIMIT_MAX)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Max Burst: No Limit.\n\r")));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "Max Burst: %-6u[Byte].\n\r"),info->max_burst));
  }

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_TBL_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_TBL_INFO *info
  )
{
  uint32
    ind=0,
    nof_zero_rate=0;
  uint8
    zero_rate;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Nof_valid_entries: %u[Entries]\n\r"),info->nof_valid_entries));
  LOG_CLI((BSL_META_U(unit,
                      "Rates:\n\r")));
  for (ind=0; ind<info->nof_valid_entries; ++ind)
  {
    zero_rate = DNX_SAND_NUM2BOOL((info->rates[ind].egq_rate == 0) && (info->rates[ind].sch_rate == 0));
    if(!zero_rate)
    {
      DNX_TMC_OFP_RATE_INFO_print(&(info->rates[ind]));
    }
    else
    {
      nof_zero_rate++;
    }
  }

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_FAT_PIPE_RATE_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_FAT_PIPE_RATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Sch_rate:  %u[Kbps]\n\r"),info->sch_rate));
  LOG_CLI((BSL_META_U(unit,
                      "Egq_rate:  %u[Kbps]\n\r"),info->egq_rate));
  LOG_CLI((BSL_META_U(unit,
                      "Max_burst: %u[Bytes]\n\r"),info->max_burst));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO_print(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "SCH: ")));
  DNX_TMC_OFP_RATE_SHPR_INFO_print(&info->sch_shaper);
  LOG_CLI((BSL_META_U(unit,
                      "EGQ: ")));
  DNX_TMC_OFP_RATE_SHPR_INFO_print(&info->egq_shaper);  
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATE_SHPR_INFO_print(
    DNX_SAND_OUT DNX_TMC_OFP_RATE_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "rate:  %u[Kbps]\n\r"),info->rate));  
  LOG_CLI((BSL_META_U(unit,
                      "Max_burst: %u[Bytes]\n\r"),info->max_burst));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_OFP_RATES_TCG_SHPR_INFO_print(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_TCG_SHPR_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "SCH: ")));
  DNX_TMC_OFP_RATE_SHPR_INFO_print(&info->sch_shaper);
  LOG_CLI((BSL_META_U(unit,
                      "EGQ: ")));
  DNX_TMC_OFP_RATE_SHPR_INFO_print(&info->egq_shaper);  
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

