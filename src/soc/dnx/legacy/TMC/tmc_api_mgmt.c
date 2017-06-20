/* $Id: jer2_tmc_api_mgmt.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_mgmt.c
*
* MODULE PREFIX:  soc_jer2_tmcmgmt
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

#include <soc/dnx/legacy/TMC/tmc_api_mgmt.h>

#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
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
  DNX_TMC_MGMT_PCKT_SIZE_clear(
    DNX_SAND_OUT DNX_TMC_MGMT_PCKT_SIZE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MGMT_PCKT_SIZE));
  info->min = 0;
  info->max = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MGMT_OCB_VOQ_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MGMT_OCB_VOQ_INFO *info
  )
{
  int32
    index;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MGMT_OCB_VOQ_INFO));
  info->voq_eligible = TRUE;
  for(index = 0; index < DNX_TMC_MGMT_OCB_VOQ_NOF_THRESHOLDS; ++index)
  {
      info->th_words[index] = DNX_TMC_MGMT_OCB_PRM_EN_TH_DEFAULT;
      info->th_buffers[index] = DNX_TMC_MGMT_OCB_PRM_EN_TH_DEFAULT;
  }

  DNX_SAND_MAGIC_NUM_SET;

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_MGMT_FABRIC_HDR_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_MGMT_FABRIC_HDR_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_MGMT_FABRIC_HDR_TYPE_PETRA:
    str = "petra";
  break;
  case DNX_TMC_MGMT_FABRIC_HDR_TYPE_FAP20:
    str = "fap20";
  break;
  case DNX_TMC_MGMT_FABRIC_HDR_TYPE_FAP10M:
    str = "fap10m";
  break;
  case DNX_TMC_MGMT_NOF_FABRIC_HDR_TYPES:
    str = " Not initialized";
  }
  return str;
}


const char*
  DNX_TMC_MGMT_TDM_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_MGMT_TDM_MODE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_MGMT_TDM_MODE_PACKET:
    str = "packet";
  break;
  case DNX_TMC_MGMT_TDM_MODE_TDM_OPT:
    str = "tdm_opt";
  break;
  case DNX_TMC_MGMT_TDM_MODE_TDM_STA:
    str = "tdm_sta";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_EXTERN:
    str = "extern";
  break;
  case DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_INTERN:
    str = "intern";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_MGMT_PCKT_SIZE_print(
    DNX_SAND_IN DNX_TMC_MGMT_PCKT_SIZE *info
  )
{
  char
    min_str[30],
    max_str[30];

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  if (info->min == DNX_TMC_MGMT_PCKT_SIZE_EXTERN_NO_LIMIT)
  {
    sal_sprintf(min_str, "Not Limited by Original Size");
  }
  else
  {
    sal_sprintf(min_str, "%u[Bytes]", info->min);
  }

  if (info->max == DNX_TMC_MGMT_PCKT_SIZE_EXTERN_NO_LIMIT)
  {
    sal_sprintf(max_str, "Not Limited by Original Size");
  }
  else
  {
    sal_sprintf(max_str, "%u[Bytes]", info->max);
  }

  LOG_CLI((BSL_META_U(unit,
                      "Min: %s, Max: %s\n\r"),min_str, max_str));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

