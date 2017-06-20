#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_llp_vid_assign.c,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_LLP

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_vid_assign.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_sa_auth.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_cos.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_MAX                     (15)
#define ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_MAX              (7)
#define ARAD_PP_LLP_VID_ASSIGN_RULE_TYPE_MAX                     (SOC_PPC_NOF_LLP_VID_ASSIGN_MATCH_RULE_TYPES-1)

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

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC
  SOC_PROCEDURE_DESC_ELEMENT
    Arad_pp_procedure_desc_element_llp_vid_assign[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LLP_VID_ASSIGN_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC
SOC_ERROR_DESC_ELEMENT
    Arad_pp_error_desc_element_llp_vid_assign[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_LLP_VID_ASSIGN_SUCCESS_OUT_OF_RANGE_ERR,
    "ARAD_PP_LLP_VID_ASSIGN_SUCCESS_OUT_OF_RANGE_ERR",
    "The parameter 'success' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_NOF_SUCCESS_FAILURES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'entry_ndx' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'port_profile_ndx' is out of range. \n\r "
    "The range is: 0 - 7.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LLP_VID_ASSIGN_ETHER_TYPE_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_LLP_VID_ASSIGN_ETHER_TYPE_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'ether_type_ndx' is out of range. \n\r "
    "The range is: 0 - 7.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LLP_VID_ASSIGN_RULE_TYPE_OUT_OF_RANGE_ERR,
    "ARAD_PP_LLP_VID_ASSIGN_RULE_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'rule_type' is out of range. \n\r "
    "The range is: 0 - ARAD_PP_NOF_LLP_SA_MATCH_RULE_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */
  {
    ARAD_PP_LLP_VID_ASSIGN_SUBNET_PREF_IS_ZERO_ERR,
    "ARAD_PP_LLP_VID_ASSIGN_SUBNET_PREF_IS_ZERO_ERR",
    "subnet prefix cannot be zero. \n\r "
    "arad_pp_llp_vid_assign_ipv4_subnet_based_set, \n\r "
    "The range is: 1 - 32.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LLP_VID_ASSIGN_DIS_AUTH_EN_ERR,
    "ARAD_PP_LLP_VID_ASSIGN_DIS_AUTH_EN_ERR",
    "Cannot disable VID assignment on port when Authentication. \n\r "
    "is enabled on port first call: soc_ppd_llp_sa_auth_port_info_set\n\r "
    "to disable authentication.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
   
  

  /*
   * Last element. Do no touch.
   */
SOC_ERR_DESC_ELEMENT_DEF_LAST
};

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

uint32
  arad_pp_llp_vid_assign_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_init_unsafe()", 0, 0);
}

/*********************************************************************
*     Sets port information for VID assignment, including PVID
 *     and which assignment mechanism to enable for this port.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_port_info_set_unsafe(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_IN  int                                         core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  )
{
  ARAD_PP_IHP_PINFO_LLR_TBL_DATA
    ihp_pinfo_llr_tbl_data;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(port_vid_assign_info);

  res =   arad_pp_ihp_pinfo_llr_tbl_get_unsafe(
            unit,
            core_id,
            local_port_ndx,
            &ihp_pinfo_llr_tbl_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  if (ihp_pinfo_llr_tbl_data.enable_sa_authentication && !port_vid_assign_info->enable_sa_based)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LLP_VID_ASSIGN_DIS_AUTH_EN_ERR,15,exit);
  }
  ihp_pinfo_llr_tbl_data.default_initial_vid = port_vid_assign_info->pvid;
  ihp_pinfo_llr_tbl_data.ignore_incoming_vid = SOC_SAND_BOOL2NUM(port_vid_assign_info->ignore_incoming_tag);
  ihp_pinfo_llr_tbl_data.vid_port_protocol_enable = SOC_SAND_BOOL2NUM(port_vid_assign_info->enable_protocol);
  ihp_pinfo_llr_tbl_data.vid_ipv4_subnet_enable = SOC_SAND_BOOL2NUM(port_vid_assign_info->enable_subnet_ip);
  ihp_pinfo_llr_tbl_data.sa_lookup_enable = SOC_SAND_BOOL2NUM(port_vid_assign_info->enable_sa_based);

  res = arad_pp_ihp_pinfo_llr_tbl_set_unsafe(
            unit,
            core_id,
            local_port_ndx,
            &ihp_pinfo_llr_tbl_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

 /*
  * save to SW DB
  */
  res = sw_state_access[unit].dpp.soc.arad.pp.llp_vid_assign.vid_sa_based_enable.bit_set(unit, local_port_ndx);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_port_info_set_unsafe()", local_port_ndx, 0);
}

uint32
  arad_pp_llp_vid_assign_port_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(local_port_ndx, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO, port_vid_assign_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_port_info_set_verify()", local_port_ndx, 0);
}

uint32
  arad_pp_llp_vid_assign_port_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(local_port_ndx, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_port_info_get_verify()", local_port_ndx, 0);
}

/*********************************************************************
*     Sets port information for VID assignment, including PVID
 *     and which assignment mechanism to enable for this port.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_port_info_get_unsafe(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_IN  int                                        core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  )
{
  ARAD_PP_IHP_PINFO_LLR_TBL_DATA
    ihp_pinfo_llr_tbl_data;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(port_vid_assign_info);

  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_clear(port_vid_assign_info);

  res = arad_pp_ihp_pinfo_llr_tbl_get_unsafe(
            unit,
            core_id,
            local_port_ndx,
            &ihp_pinfo_llr_tbl_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  port_vid_assign_info->pvid = ihp_pinfo_llr_tbl_data.default_initial_vid;
  port_vid_assign_info->ignore_incoming_tag = SOC_SAND_NUM2BOOL(ihp_pinfo_llr_tbl_data.ignore_incoming_vid);
  port_vid_assign_info->enable_protocol = SOC_SAND_NUM2BOOL(ihp_pinfo_llr_tbl_data.vid_port_protocol_enable);
  port_vid_assign_info->enable_subnet_ip = SOC_SAND_NUM2BOOL(ihp_pinfo_llr_tbl_data.vid_ipv4_subnet_enable);
  port_vid_assign_info->enable_sa_based = SOC_SAND_NUM2BOOL(ihp_pinfo_llr_tbl_data.sa_lookup_enable);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_port_info_get_unsafe()", local_port_ndx, 0);
}

/*********************************************************************
*     Set assignment to VLAN ID according to source MAC
 *     address.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *mac_address_key,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO        *mac_based_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
  )
{
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack_status;
  uint8
    is_found = FALSE;
  
  SOC_PPC_LLP_SA_AUTH_MAC_INFO
    old_sa_auth_info;
  uint32
    old_asd,
    new_asd = 0;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(mac_address_key);
  SOC_SAND_CHECK_NULL_INPUT(mac_based_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack_status);

  request.stamp = 0;
  /* convert to LEM key */
  res = arad_pp_sa_based_key_to_lem_key_map(
          unit,
          mac_address_key,
          &(request.key)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  /* set payload */

  /* get previous payload. ASD , as part of the ASD used for VID-assignment */
  /* if key is found and destination is not relevant then keep exist destination payload.dest*/
  res = arad_pp_lem_access_entry_by_key_get_unsafe(
          unit,
          &(request.key),
          &payload,
          &is_found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  old_asd = payload.asd;

  /* set defaults */
  payload.age = ARAD_PP_SA_AUTH_ENTRY_AGE;
  payload.is_dynamic = ARAD_PP_SA_AUTH_ENTRY_IS_DYNAMIC;

 /*
  * if found then leave SA-Auth untouched (except VID which is shared)
  * if not found then set to zeros, i.e. don't used authentication.
  */
  SOC_PPC_LLP_SA_AUTH_MAC_INFO_clear(&old_sa_auth_info);

  if (is_found)
  {
    res = arad_pp_lem_access_sa_based_asd_parse(
            unit,
            old_asd,
            &old_sa_auth_info,
            NULL
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
    
    if (old_sa_auth_info.expect_tag_vid != SOC_PPC_LLP_SA_AUTH_ACCEPT_ALL_VIDS)
    {
      old_sa_auth_info.expect_tag_vid = mac_based_info->vid;
    }
  }
  else
  {
    ARAD_PP_LLP_SA_SET_NO_AUTH(&old_sa_auth_info);
  }

  res = arad_pp_lem_access_sa_based_asd_build(
          unit,
          &old_sa_auth_info,
          mac_based_info,
          &new_asd
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  
   /*
    * set updated bit in old ASD: use for-tagged/untagged and VID
    
    tmp = 0;
    res = soc_sand_bitstream_get_any_field(&new_asd, ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_LSB, 2, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    res = soc_sand_set_field(&updated_asd, ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_UNTAGGED_MSB, ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_LSB, tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    tmp = 0;
    res = soc_sand_bitstream_get_any_field(&new_asd, ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_LSB, ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_LEN, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    res = soc_sand_set_field(&updated_asd, ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_MSB, ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_LSB, tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  }
  else
  {
    updated_asd = new_asd;
  }
*/
  payload.asd = new_asd;
  payload.flags = ARAD_PP_FWD_DECISION_PARSE_FORMAT_3;
  /* add entry */
  request.command = ARAD_PP_LEM_ACCESS_CMD_INSERT;
  res = arad_pp_lem_access_entry_add_unsafe(
          unit,
          &request,
          &payload,
          &ack_status
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  *success = (ack_status.is_success ? SOC_SAND_SUCCESS : SOC_SAND_FAILURE_INTERNAL_ERR);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_add_unsafe()", 0, 0);
}

uint32
  arad_pp_llp_vid_assign_mac_based_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO           *mac_based_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_VERIFY);

  /* ARAD_PP_STRUCT_VERIFY(SOC_SAND_PP_MAC_ADDRESS, mac_address_key, 10, exit); */
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LLP_VID_ASSIGN_MAC_INFO, mac_based_info, 20, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_add_verify()", 0, 0);
}

/*********************************************************************
*     Remove assignment to VLAN ID according to source MAC
 *     address.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key
  )
{
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack_status;
  SOC_PPC_LLP_SA_AUTH_MAC_INFO
    old_sa_auth_info;
  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO
    no_vid_assign_info;
  uint8
    is_found = FALSE;
  uint32
    old_asd,
    new_asd = 0;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(mac_address_key);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack_status);

  request.stamp = 0;
  /* convert to LEM key */
  res = arad_pp_sa_based_key_to_lem_key_map(
          unit,
          mac_address_key,
          &(request.key)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* set payload */

  /* get previous payload. ASD , as part of the ASD used for SA-AUTH*/
  res = arad_pp_lem_access_entry_by_key_get_unsafe(
          unit,
          &(request.key),
          &payload,
          &is_found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* if remove and not found then done */
  if (!is_found)
  {
    goto exit;
  }

  old_asd = payload.asd;

  /* set defaults */
  payload.age = ARAD_PP_SA_AUTH_ENTRY_AGE;
  payload.is_dynamic = ARAD_PP_SA_AUTH_ENTRY_IS_DYNAMIC;

  /* check if auth field are not in use */
  /* i.e. Permit all ports is set and drop-if-vid-diff is unset, accept untagged is set*/
  SOC_PPC_LLP_SA_AUTH_MAC_INFO_clear(&old_sa_auth_info);
  
  res = arad_pp_lem_access_sa_based_asd_parse(
          unit,
          old_asd,
          &old_sa_auth_info,
          NULL
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  

  if (ARAD_PP_LLP_SA_NO_AUTH(&old_sa_auth_info)) /* not in use --> remove entry */
  {
    request.command = ARAD_PP_LEM_ACCESS_CMD_DELETE;
    res = arad_pp_lem_access_entry_remove_unsafe(
            unit,
            &request,
            &ack_status
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }
  else /* in use --> update ASD entry to disable VID assignment */
  {
    SOC_PPC_LLP_VID_ASSIGN_MAC_INFO_clear(&no_vid_assign_info);
    no_vid_assign_info.override_tagged = FALSE;
    no_vid_assign_info.use_for_untagged = FALSE;
    no_vid_assign_info.vid = old_sa_auth_info.expect_tag_vid;
    res = arad_pp_lem_access_sa_based_asd_build(
            unit,
            &old_sa_auth_info,
            &no_vid_assign_info,
            &new_asd
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    
    /* add entry */
    request.command = ARAD_PP_LEM_ACCESS_CMD_INSERT;
    payload.asd = new_asd;
    payload.flags = ARAD_PP_FWD_DECISION_PARSE_FORMAT_3;
    res = arad_pp_lem_access_entry_add_unsafe(
            unit,
            &request,
            &payload,
            &ack_status
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_remove_unsafe()", 0, 0);
}

uint32
  arad_pp_llp_vid_assign_mac_based_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_VERIFY);

  /* ARAD_PP_STRUCT_VERIFY(SOC_SAND_PP_MAC_ADDRESS, mac_address_key, 10, exit); */

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_remove_verify()", 0, 0);
}

/*********************************************************************
*     Get VLAN ID assignment information according to source
 *     MAC address.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info,
    SOC_SAND_OUT uint8                                 *found
  )
{
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  uint8
    is_found = FALSE;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(mac_address_key);
  SOC_SAND_CHECK_NULL_INPUT(mac_based_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO_clear(mac_based_info);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);

  request.stamp = 0;
  /* convert to LEM key */
  res = arad_pp_sa_based_key_to_lem_key_map(
          unit,
          mac_address_key,
          &(request.key)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* get  payload. ASD , as part of the ASD used for VID-assignment */
  res = arad_pp_lem_access_entry_by_key_get_unsafe(
          unit,
          &(request.key),
          &payload,
          &is_found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* if not found then done */
  if (!is_found)
  {
    *found = FALSE;
    goto exit;
  }

  *found = TRUE;
  /*found, check get values*/
  
  res = arad_pp_lem_access_sa_based_asd_parse(
          unit,
          payload.asd,
          NULL,
          mac_based_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_get_unsafe()", 0, 0);
}

uint32
  arad_pp_llp_vid_assign_mac_based_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_VERIFY);

  /* ARAD_PP_STRUCT_VERIFY(SOC_SAND_PP_MAC_ADDRESS, mac_address_key, 10, exit); */

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_get_verify()", 0, 0);
}

/*********************************************************************
*     Get VLAN ID assignment and SA authentation information
 *     according to source MAC address.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE           *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key_arr,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *vid_assign_info_arr,
    SOC_SAND_OUT uint32                                  *nof_entries
  )
{
  uint32
    res = SOC_SAND_OK,
    access_only = FALSE,
    read_index = 0,
    tmp_nof_entries = 0;     
  ARAD_PP_LEM_ACCESS_KEY
    key;
  ARAD_PP_LEM_ACCESS_KEY            
    key_mask;
  ARAD_PP_LEM_ACCESS_KEY
    *read_keys = NULL;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    *read_vals = NULL;
  SOC_SAND_PP_MAC_ADDRESS
    mac_address_key;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(rule);
  SOC_SAND_CHECK_NULL_INPUT(block_range);
  SOC_SAND_CHECK_NULL_INPUT(mac_address_key_arr);
  SOC_SAND_CHECK_NULL_INPUT(vid_assign_info_arr);
  SOC_SAND_CHECK_NULL_INPUT(nof_entries);

  *nof_entries = 0;

  ARAD_PP_LEM_ACCESS_KEY_clear(&key);
  ARAD_PP_LEM_ACCESS_KEY_clear(&key_mask);
  soc_sand_SAND_PP_MAC_ADDRESS_clear(&mac_address_key);

  /* convert to LEM key */
  res = arad_pp_sa_based_key_to_lem_key_map(
          unit,
          &mac_address_key,
          &key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  /* consider all lem entries for SA Auth */
  key.type = ARAD_PP_LEM_ACCESS_KEY_TYPE_SA_AUTH;
  key_mask.prefix.value = ARAD_PP_LEM_ACCESS_KEY_PREFIX_ALL_MASKED;
  key_mask.prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_SA_AUTH(unit);

   /* malloc place for keys and vals */
  read_keys = soc_sand_os_malloc_any_size(sizeof(ARAD_PP_LEM_ACCESS_KEY) * block_range->entries_to_act,"read_keys");
  read_vals = soc_sand_os_malloc_any_size(sizeof(ARAD_PP_LEM_ACCESS_PAYLOAD) * block_range->entries_to_act,"read_vals");

  if (!read_keys)
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 40, exit);
  }

  if (!read_vals)
  {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 45, exit);
  }

  /* initialize block range */
  res = soc_sand_os_memset(
          read_keys,
          0x0,
          sizeof(ARAD_PP_LEM_ACCESS_KEY) * block_range->entries_to_act
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  res = soc_sand_os_memset(
          read_vals,
          0x0,
          sizeof(ARAD_PP_LEM_ACCESS_PAYLOAD) * block_range->entries_to_act
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);

  res = arad_pp_frwrd_lem_get_block_unsafe(unit,&key,&key_mask,NULL,access_only ,block_range,read_keys,read_vals,&tmp_nof_entries);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  
  /*
   * Loop on the entry index
   */
  for ( read_index = 0; read_index < tmp_nof_entries; ++read_index)
  {
    if ((read_keys[read_index].type == ARAD_PP_LEM_ACCESS_KEY_TYPE_SA_AUTH))    
    {
      res = arad_pp_sa_based_key_from_lem_key_map(
              unit,
              &read_keys[read_index],
              &(mac_address_key_arr[*nof_entries])
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      SOC_PPC_LLP_VID_ASSIGN_MAC_INFO_clear(&vid_assign_info_arr[*nof_entries]);

      res = arad_pp_lem_access_sa_based_asd_parse(
          unit,
          read_vals[read_index].asd,
          NULL,
          &vid_assign_info_arr[*nof_entries]
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      if ((!(rule->rule_type & SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE_TYPE_TAGGED))&& vid_assign_info_arr[*nof_entries].override_tagged)
      {
        continue;
      }
      if ((!(rule->rule_type & SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE_TYPE_UNTAGGED))&& vid_assign_info_arr[*nof_entries].use_for_untagged)
      {
        continue;
      }
      ++*nof_entries;
    }
 }

exit:
  if (read_keys)
  {
    soc_sand_os_free_any_size(read_keys);
  }
  if (read_vals)
  {
    soc_sand_os_free_any_size(read_vals);
  }
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_get_block_unsafe()", 0, 0);
}

uint32
  arad_pp_llp_vid_assign_mac_based_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE           *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE, rule, 10, exit);
  /* ARAD_PP_STRUCT_VERIFY(SOC_SAND_TABLE_BLOCK_RANGE, block_range, 20, exit); */

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_mac_based_get_block_verify()", 0, 0);
}

/*********************************************************************
*     Set assignment of VLAN ID based on source IPv4 subnet.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                       *subnet,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  )
{
  ARAD_PP_IHP_SUBNET_CLASSIFY_TBL_DATA
    tbl_data;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(subnet);
  SOC_SAND_CHECK_NULL_INPUT(subnet_based_info);

  res = arad_pp_ihp_subnet_classify_tbl_get_unsafe(
          unit,
          entry_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* Override subnet only if vid is valid, in order not to ruin the cos entry (if valid) */
  if (subnet_based_info->vid_is_valid)
  {
    tbl_data.ipv4_subnet = subnet->ip_address;
    /* set number of invalid bits*/
    tbl_data.ipv4_subnet_mask = (32 - subnet->prefix_len);
  }
  tbl_data.ipv4_subnet_vid = subnet_based_info->vid;
  tbl_data.ipv4_subnet_valid = SOC_SAND_BOOL2NUM(subnet_based_info->vid_is_valid);

  res = arad_pp_ihp_subnet_classify_tbl_set_unsafe(
          unit,
          entry_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_ipv4_subnet_based_set_unsafe()", entry_ndx, 0);
}

uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                       *subnet,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_MAX, ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO, subnet_based_info, 30, exit);

  res = soc_sand_pp_ipv4_subnet_verify(subnet);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /* prefix cannot be zero if entry is valid */
  if (subnet_based_info->vid_is_valid && (subnet->prefix_len == 0))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LLP_VID_ASSIGN_SUBNET_PREF_IS_ZERO_ERR,50,exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_ipv4_subnet_based_set_verify()", entry_ndx, 0);
}

uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_MAX, ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_ipv4_subnet_based_get_verify()", entry_ndx, 0);
}

/*********************************************************************
*     Set assignment of VLAN ID based on source IPv4 subnet.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IPV4_SUBNET                       *subnet,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  )
{
  ARAD_PP_IHP_SUBNET_CLASSIFY_TBL_DATA
    tbl_data;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(subnet);
  SOC_SAND_CHECK_NULL_INPUT(subnet_based_info);

  soc_sand_SAND_PP_IPV4_SUBNET_clear(subnet);
  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO_clear(subnet_based_info);

  res = arad_pp_ihp_subnet_classify_tbl_get_unsafe(
          unit,
          entry_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  subnet->ip_address = tbl_data.ipv4_subnet;
  subnet->prefix_len = (uint8)(32 - tbl_data.ipv4_subnet_mask);
  subnet_based_info->vid = (SOC_SAND_PP_VLAN_ID)tbl_data.ipv4_subnet_vid;
  subnet_based_info->vid_is_valid = SOC_SAND_NUM2BOOL(tbl_data.ipv4_subnet_valid);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_ipv4_subnet_based_get_unsafe()", entry_ndx, 0);
}

/*********************************************************************
*     Sets mapping from Ethernet Type and profile to VID.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_protocol_based_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO         *prtcl_assign_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  )
{
  uint32
    res = SOC_SAND_OK,
    tbl_offset,
    ether_type_internal;
  uint8
    update_hw,
    alloc_ether_type,
    found;
  ARAD_PP_IHP_PORT_PROTOCOL_TBL_DATA
    ihp_port_protocol_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(prtcl_assign_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  tbl_offset = 0;
  *success = SOC_SAND_SUCCESS;
  alloc_ether_type = TRUE;
  update_hw = TRUE;

  res = arad_pp_l2_next_prtcl_type_find(
          unit,
          ether_type_ndx,
          &ether_type_internal,
          &found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (found)
  {
    /* Ether type already allocated. Let's get the table entry */
    tbl_offset =
      ARAD_IHP_PORT_PROTOCOL_TBL_KEY_ENTRY_OFFSET(ether_type_internal, port_profile_ndx);

    res = arad_pp_ihp_port_protocol_tbl_get_unsafe(
            unit,
            tbl_offset,
            &ihp_port_protocol_tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if (ihp_port_protocol_tbl_data.vid_valid)
    {
      if (prtcl_assign_info->vid_is_valid)
      {
        if (ihp_port_protocol_tbl_data.vid == prtcl_assign_info->vid)
        {
          /* Same data is already configured. Nothing to do. */
          update_hw = FALSE;
          alloc_ether_type = FALSE;
        }
        else /* (ihp_port_protocol_tbl_data.vid != prtcl_assign_info->vid) */
        {
          /* Entry will be later update to new value */
          alloc_ether_type = FALSE;
        }
      }
      else /* !prtcl_assign_info->vid_is_valid */
      {
        /* Entry is currently valid, but will be configured to be invalid */
        res = arad_pp_l2_next_prtcl_type_deallocate(
                unit,
                ether_type_ndx
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        alloc_ether_type = FALSE;
      }
    }
  }
  
  if (alloc_ether_type)
  {
    /* Allocate ether type */
    res = arad_pp_l2_next_prtcl_type_allocate(
            unit,
            ether_type_ndx,
            &ether_type_internal,
            success
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }

  if (*success == SOC_SAND_SUCCESS)
  {
    if (!found)
    {
      tbl_offset =
        ARAD_IHP_PORT_PROTOCOL_TBL_KEY_ENTRY_OFFSET(ether_type_internal, port_profile_ndx);

      res = arad_pp_ihp_port_protocol_tbl_get_unsafe(
              unit,
              tbl_offset,
              &ihp_port_protocol_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    }

    if (update_hw)
    {
      ihp_port_protocol_tbl_data.vid = prtcl_assign_info->vid;
      ihp_port_protocol_tbl_data.vid_valid = SOC_SAND_BOOL2NUM(prtcl_assign_info->vid_is_valid);

      res = arad_pp_ihp_port_protocol_tbl_set_unsafe(
              unit,
              tbl_offset,
              &ihp_port_protocol_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }
  }
 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_protocol_based_set_unsafe()", port_profile_ndx, ether_type_ndx);
}

uint32
  arad_pp_llp_vid_assign_protocol_based_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(port_profile_ndx, ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_MAX, ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO, prtcl_assign_info, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_protocol_based_set_verify()", port_profile_ndx, ether_type_ndx);
}

uint32
  arad_pp_llp_vid_assign_protocol_based_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(port_profile_ndx, ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_MAX, ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_protocol_based_get_verify()", port_profile_ndx, ether_type_ndx);
}

/*********************************************************************
*     Sets mapping from Ethernet Type and profile to VID.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_protocol_based_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info
  )
{
  uint32
    res = SOC_SAND_OK,
    tbl_offset,
    ether_type_internal;
  uint8
    found;
  ARAD_PP_IHP_PORT_PROTOCOL_TBL_DATA
    ihp_port_protocol_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(prtcl_assign_info);

  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO_clear(prtcl_assign_info);

  res = arad_pp_l2_next_prtcl_type_find(
          unit,
          ether_type_ndx,
          &ether_type_internal,
          &found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (found)
  {
    tbl_offset =
      ARAD_IHP_PORT_PROTOCOL_TBL_KEY_ENTRY_OFFSET(ether_type_internal, port_profile_ndx);

    res = arad_pp_ihp_port_protocol_tbl_get_unsafe(
      unit,
      tbl_offset,
      &ihp_port_protocol_tbl_data
      );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    prtcl_assign_info->vid_is_valid =
      SOC_SAND_NUM2BOOL(ihp_port_protocol_tbl_data.vid_valid);
    prtcl_assign_info->vid = (SOC_SAND_PP_VLAN_ID)ihp_port_protocol_tbl_data.vid;
  }
  else /* if not found, then VID assignment is not valid*/
  {
    prtcl_assign_info->vid = 0;
    prtcl_assign_info->vid_is_valid = FALSE;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_llp_vid_assign_protocol_based_get_unsafe()", port_profile_ndx, ether_type_ndx);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_llp_vid_assign module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_llp_vid_assign_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_llp_vid_assign;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_llp_vid_assign module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_llp_vid_assign_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_llp_vid_assign;
}
uint32
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->pvid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_verify()",0,0);
}

uint32
  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->vid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LLP_VID_ASSIGN_MAC_INFO_verify()",0,0);
}

uint32
  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->vid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO_verify()",0,0);
}

uint32
  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->vid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  /*SOC_SAND_ERR_IF_ABOVE_MAX(info->rule_type, ARAD_PP_LLP_VID_ASSIGN_RULE_TYPE_MAX, ARAD_PP_LLP_VID_ASSIGN_RULE_TYPE_OUT_OF_RANGE_ERR, 10, exit);*/
  if (info->vid != ARAD_PP_IGNORE_VAL) {
    SOC_SAND_ERR_IF_ABOVE_MAX(info->vid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 11, exit);
  }
  

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE_verify()",0,0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

