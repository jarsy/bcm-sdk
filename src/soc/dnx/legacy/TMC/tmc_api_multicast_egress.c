/* $Id: jer2_tmc_api_multicast_egress.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_multicast_egress.c
*
* MODULE PREFIX:  soc_jer2_tmcmult_eg
*
* FILE DESCRIPTION:  refer to H file.
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
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/TMC/tmc_api_multicast_egress.h>
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
  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_clear(
    DNX_SAND_OUT DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE));
  info->mc_id_low = 0;
  info->mc_id_high = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_EG_ENTRY_clear(
    DNX_SAND_OUT DNX_TMC_MULT_EG_ENTRY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_EG_ENTRY));
  info->type = DNX_TMC_MULT_EG_ENTRY_TYPE_OFP;
  info->vlan_mc_id = 0;
  info->cud = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_clear(
    DNX_SAND_OUT DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_MULT_EG_ENTRY_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_MULT_EG_ENTRY_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_MULT_EG_ENTRY_TYPE_OFP:
    str = "ofp";
  break;
  case DNX_TMC_MULT_EG_ENTRY_TYPE_VLAN_PTR:
    str = "vlan_ptr";
  break;
  case DNX_TMC_MULT_EG_ENTRY_NOF_TYPES:
    str = "nof_types";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_print(
    DNX_SAND_IN DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Mc_id_low:  %u, Mc_id_high: %u\n\r"),info->mc_id_low, info->mc_id_high));
  LOG_CLI((BSL_META_U(unit,
                      "Mc_id_high: %u\n\r"),info->mc_id_high));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_EG_ENTRY_print(
    DNX_SAND_IN DNX_TMC_MULT_EG_ENTRY *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Type %s "), DNX_TMC_MULT_EG_ENTRY_TYPE_to_string(info->type)));
  LOG_CLI((BSL_META_U(unit,
                      "Port-id: %u, Copy-unique-data: %u\n\r"),info->port, info->cud));
  LOG_CLI((BSL_META_U(unit,
                      "Vlan_mc_id: %u\n\r"),info->vlan_mc_id));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_print(
    DNX_SAND_IN DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *info
  )
{
  uint32
    ind=0,
    bit_idx,
    reg_idx,
    bit_val,
    cnt = 0;
  uint8
    found = FALSE;
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "Member FAP Ports:\n\r")));
  for (ind=0; ind<DNX_TMC_NOF_FAP_PORTS_MAX; ++ind) 
  {
    bit_idx = ind % DNX_SAND_REG_SIZE_BITS;
    reg_idx = ind / DNX_SAND_REG_SIZE_BITS;
    bit_val = DNX_SAND_GET_BIT(info->bitmap[reg_idx], bit_idx);
    if (bit_val)
    {
      found = TRUE;
      LOG_CLI((BSL_META_U(unit,
                          "%02u "), ind));

      if ((++cnt % 10) == 0)
      {
        LOG_CLI((BSL_META_U(unit,
                            "\n\r")));
      }
    }
  }
  if (!found)
  {
    LOG_CLI((BSL_META_U(unit,
                        "None")));
  }
  else
  {
    LOG_CLI((BSL_META_U(unit,
                        "\n\rTotal: %u"), cnt));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

