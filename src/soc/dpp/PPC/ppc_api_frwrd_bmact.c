/* $Id: ppc_api_frwrd_bmact.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/src/soc_ppc_api_frwrd_bmact.c
*
* MODULE PREFIX:  soc_ppc_frwrd
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

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PPC

#include <shared/bsl.h>

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/PPC/ppc_api_frwrd_bmact.h>

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
  SOC_PPC_BMACT_BVID_INFO_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_BVID_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(SOC_PPC_BMACT_BVID_INFO));
  info->stp_topology_id = 0;
  info->b_fid_profile = 0;
  SOC_PPC_FRWRD_DECISION_INFO_clear(&(info->uknown_da_dest));
  info->default_forward_profile = 0;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  SOC_PPC_BMACT_PBB_TE_VID_RANGE_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_PBB_TE_VID_RANGE *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(SOC_PPC_BMACT_PBB_TE_VID_RANGE));
  info->first_vid = 0;
  info->last_vid = 0;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  SOC_PPC_BMACT_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_KEY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(SOC_PPC_BMACT_ENTRY_KEY));
  info->b_vid = 0;
  info->flags = 0;
  info->local_port_ndx = 0;
  info->core = 0;
  soc_sand_SAND_PP_MAC_ADDRESS_clear(&(info->b_mac_addr));
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  SOC_PPC_BMACT_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(SOC_PPC_BMACT_ENTRY_INFO));
  info->sys_port_id = 0;
  info->i_sid_domain = 0;
  info->sa_learn_fec_id = 0;
  info->drop_sa = 0;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_BMACT_BVID_INFO_print(
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "stp_topology_id: %u\n\r"),info->stp_topology_id));
  LOG_CLI((BSL_META_U(unit,
                      "b_fid_profile: %u\n\r"),info->b_fid_profile));
  LOG_CLI((BSL_META_U(unit,
                      "da_not_found_action_profile_ndx: %u\n\r"),info->uknown_da_dest.dest_id));
  /*LOG_CLI((BSL_META_U(unit,
                        "uknown_da_dest:")));
  SOC_PPC_FRWRD_DECISION_INFO_print(&(info->uknown_da_dest));
  LOG_CLI((BSL_META_U(unit,
                      "default_forward_profile: %u\n\r"),info->default_forward_profile));*/
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  SOC_PPC_BMACT_PBB_TE_VID_RANGE_print(
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "first_vid: %u\n\r"),info->first_vid));
  LOG_CLI((BSL_META_U(unit,
                      "last_vid: %u\n\r"),info->last_vid));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  SOC_PPC_BMACT_ENTRY_KEY_print(
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "b_vid: %u\n\r"),info->b_vid));
  LOG_CLI((BSL_META_U(unit,
                      "b_mac_addr:")));
  soc_sand_SAND_PP_MAC_ADDRESS_print(&(info->b_mac_addr));
  LOG_CLI((BSL_META_U(unit,
                      "flags: 0x%x\n\r"),info->flags));
  LOG_CLI((BSL_META_U(unit,
                      "local_port_ndx: %u\n\r"),info->local_port_ndx));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  SOC_PPC_BMACT_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "sys_port_id: %u\n\r"),info->sys_port_id));
  LOG_CLI((BSL_META_U(unit,
                      "i_sid_domain: %u\n\r"),info->i_sid_domain));
  LOG_CLI((BSL_META_U(unit,
                      "sa_learn_fec_id: %u\n\r"),info->sa_learn_fec_id));
  LOG_CLI((BSL_META_U(unit,
                      "drop_sa: %u\n\r"),info->drop_sa));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

