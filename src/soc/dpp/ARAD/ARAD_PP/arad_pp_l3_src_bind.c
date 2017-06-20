/* $Id: arad_pp_l3_src_bind.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_L3

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

#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_l3_src_bind.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif_table.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_isem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv4.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>

#include <soc/dpp/PPD/ppd_api_frwrd_ipv4.h>


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

#define ARAD_PP_SRC_BIND_GET_16LSB(lif) (lif & 0xFFFF)
#define ARAD_PP_SRC_BIND_IP6_COMPRESSION_PREFIX(is_poof, tt_result) \
                        ((is_poof) && (((tt_result) & 0x1) == 0))

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC SOC_PROCEDURE_DESC_ELEMENT
  Arad_pp_procedure_desc_element_src_bind[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_TABLE_CLEAR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_TABLE_CLEAR_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV4_TABLE_CLEAR_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_TABLE_CLEAR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_TABLE_CLEAR_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_TABLE_CLEAR_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_IPV6_STATIC_TCAM_INIT_UNSAFE),

  /*
   * } Auto generated. Do not edit previous section.
   */

  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_SRC_BIND_GET_ERRS_PTR),

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_src_bind[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_SRC_BIND_INVALID_SMAC_ERR,
    "ARAD_PP_SRC_BIND_INVALID_SMAC_ERR",
    "The parameter 'smac' is zero when parameter 'smac_valid' is true. \n\r "
    "'smac' can't be zero when 'smac_valid' is true.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_SRC_BIND_IPV4_INVALID_SIP_ERR,
    "ARAD_PP_SRC_BIND_IPV4_INVALID_SIP_ERR",
    "The parameter 'sip' is zero. \n\r "
    "'sip' can't be zero.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_SRC_BIND_IPV6_INVALID_SIP_ERR,
    "ARAD_PP_SRC_BIND_IPV6_INVALID_SIP_ERR",
    "The parameter 'sip6' is zero. \n\r "
    "'sip6' can't be zero.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_SRC_BIND_INFO_IS_NULL_ERR,
    "ARAD_PP_SRC_BIND_INFO_IS_NULL_ERR",
    "Both IPv4 info and IPv6 info is NULL. \n\r "
    "Should keep 1 info isn't NULL.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_SRC_BIND_SW_DB_NO_RES_ERR,
    "ARAD_PP_SRC_BIND_SW_DB_NO_RES_ERR",
    "No enough resource in source bind SW DB.\n\r "
    "Max number of bind information is 4096.\n\r ",
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
static uint32
  arad_pp_l3_src_bind_get_vsi_unsafe (
      SOC_SAND_IN  int                                       unit,
      SOC_SAND_IN  SOC_PPC_LIF_ID                               lif_ndx,
      SOC_SAND_OUT SOC_PPC_VSI_ID                              *vsi_id
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_LIF_ENTRY_INFO
    lif_entry_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(vsi_id);

  SOC_PPC_LIF_ENTRY_INFO_clear(&lif_entry_info);

  res = arad_pp_lif_table_entry_get_unsafe(
          unit,
          lif_ndx,
          &lif_entry_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  *vsi_id = lif_entry_info.value.ac.vsid;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_l3_src_bind_get_vsi_unsafe()", lif_ndx, 0);
}

static uint32
  arad_pp_l3_src_bind_key_to_sem_key_unsafe (
    SOC_SAND_IN  int                                       unit,
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY                 *ipv4_info,
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY                 *ipv6_info,
    SOC_SAND_OUT ARAD_PP_ISEM_ACCESS_KEY                     *sem_key
  )
{

  SOC_PPC_VSI_ID
    vsi_id = 0;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  if (ipv4_info != NULL)
  {
    /* Fill sem_key as ARAD_PP_ISEM_ACCESS_KEY_TYPE_SPOOF_IPV4  */
    sem_key->key_type = ARAD_PP_ISEM_ACCESS_KEY_TYPE_SPOOF_IPV4;
    /* Only bits [15:8] will be inserted to DB */
    sem_key->key_info.spoof_v4.smac = ipv4_info->smac;
    sem_key->key_info.spoof_v4.sip = ipv4_info->sip;
  }
  else if (ipv6_info != NULL)
  {
    res = arad_pp_l3_src_bind_get_vsi_unsafe(unit, ipv6_info->lif_ndx, &vsi_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    sem_key->key_type = ARAD_PP_ISEM_ACCESS_KEY_TYPE_SPOOF_IPV6;
    sem_key->key_info.spoof_v6.smac = ipv6_info->smac;
    sem_key->key_info.spoof_v6.sip6.prefix_len = 0;
    sem_key->key_info.spoof_v6.sip6.ipv6_address = ipv6_info->sip6;
  }
  else
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_SRC_BIND_INFO_IS_NULL_ERR, 30, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_l3_src_bind_key_to_sem_key_unsafe()", 0, 0);
}

/* build lem access key for IpV4 host address */
STATIC uint32
    arad_pp_src_bind_ip_dhcp_lem_request_key_build(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY_TYPE type,
      SOC_SAND_IN uint16                      spoof_id,
      SOC_SAND_IN SOC_PPC_LIF_ID              lif_id,
      SOC_SAND_IN SOC_SAND_PP_MAC_ADDRESS     smac,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY    *key
    )
{
  uint32
    num_bits = 0;
  uint32
    res = SOC_SAND_OK;
  int index_mac = 0;
  int index_lif = 0;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_PP_LEM_ACCESS_KEY_clear(key);

  key->type = type;

  key->nof_params = ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IP_SPOOF_DHCP;

  num_bits = (SOC_IS_JERICHO(unit))?ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IP_SPOOF_DHCP_JERICHO:ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IP_SPOOF_DHCP;
  index_mac = (SOC_IS_JERICHO(unit))?1:0;
  index_lif = (SOC_IS_JERICHO(unit))?0:1;
  
  key->param[index_mac].nof_bits = (uint8)num_bits;
  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */ 
  /* coverity[overrun-buffer-val : FALSE] */  
  res = soc_sand_pp_mac_address_struct_to_long(
          &smac,
          key->param[index_mac].value
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* MAC address bits[40:32] is valid */
  key->param[index_mac].value[1] &= 0x1ff;

  num_bits =( SOC_IS_JERICHO(unit))? ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IP_SPOOF_DHCP_JERICHO:ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IP_SPOOF_DHCP;
  key->param[index_lif].nof_bits = num_bits;
  key->param[index_lif].value[0] = lif_id; 

  num_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_IP_SPOOF_DHCP;
  key->param[2].nof_bits = num_bits;
  key->param[2].value[0] = spoof_id;  

  key->prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IP_SPOOF_DHCP;

  if (SOC_IS_JERICHO(unit)) {
      key->prefix.value = 0x8;
  }else{
      key->prefix.value = 0x1;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ip_dhcp_lem_request_key_build()", 0, 0);
}

/* build lem access key for IpV4 host address */
STATIC void
    arad_pp_src_bind_ipv4_static_lem_request_key_build(
      SOC_SAND_IN  int                        unit,
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY_TYPE type,
      SOC_SAND_IN SOC_PPC_LIF_ID              lif_id,
      SOC_SAND_IN uint32                      sip,
      SOC_SAND_IN SOC_PPC_VSI_ID              vsi_id,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY    *key
    )
{
  uint32
    num_bits;
  
  ARAD_PP_LEM_ACCESS_KEY_clear(key);
  
  key->type = type;

  key->nof_params = ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IPV4_SPOOF_STATIC;

  num_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IPV4_SPOOF_STATIC;
  key->param[0].nof_bits = (uint8)num_bits;
  key->param[0].value[0] = sip;

  num_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IPV4_SPOOF_STATIC(unit);
  key->param[1].nof_bits = (uint8)num_bits;
  if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit) && lif_id)
  {
    key->param[1].value[0] = vsi_id;
  } else {
    key->param[1].value[0] = lif_id;
  }
  key->prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IPV4_SPOOF_STATIC;
  key->prefix.value =    ARAD_PP_LEM_ACCESS_KEY_PREFIX_IPV4_SPOOF_STATIC;

}

/* Parse lem access key for IpV4 host address */
STATIC uint32
    arad_pp_src_bind_ipv4_static_lem_request_key_parse(
      SOC_SAND_IN  int                           unit,
      SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY       *key,
      SOC_SAND_OUT SOC_PPC_SRC_BIND_IPV4_ENTRY  *ipv4_info
    )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(key);
  SOC_SAND_CHECK_NULL_INPUT(ipv4_info);

  ipv4_info->is_network = 0;
  ipv4_info->sip = key->param[0].value[0];
  if (!SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit))
  {
    ipv4_info->lif_ndx = key->param[1].value[0];
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_static_lem_request_key_parse()", 0, 0);
}

STATIC uint32
    arad_pp_src_bind_lem_payload_build(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO               *frwrd_decision,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD                *payload
    )
{
  uint32 res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(payload);

  res = arad_pp_fwd_decision_in_buffer_build(
          unit,
          ARAD_PP_FRWRD_DECISION_APPLICATION_TYPE_DFLT,
          frwrd_decision,
          &(payload->dest),
          &(payload->asd)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if(frwrd_decision->additional_info.outlif.type != SOC_PPC_OUTLIF_ENCODE_TYPE_NONE) {
      payload->flags = ARAD_PP_FWD_DECISION_PARSE_OUTLIF;
  }
  else if(frwrd_decision->additional_info.eei.type != SOC_PPC_EEI_TYPE_EMPTY) {
      payload->flags = ARAD_PP_FWD_DECISION_PARSE_EEI;
  }
  else{
      payload->flags = ARAD_PP_FWD_DECISION_PARSE_DEST;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_lem_payload_build()", 0, 0);
}


static
  uint32
    arad_pp_src_bind_ipv4_lem_add(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY               *ipv4_info,
      SOC_SAND_IN  uint8                                      dhcp,
      SOC_SAND_IN  uint32                                     spoof_id,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;
  SOC_PPC_FRWRD_DECISION_INFO   
    frwrd_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ipv4_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);
  SOC_PPC_FRWRD_DECISION_INFO_clear(&frwrd_info);

  request.command = ARAD_PP_LEM_ACCESS_CMD_INSERT;
  if (dhcp)
  {
    /* spoof id is always <=0xFFFF */
    res = arad_pp_src_bind_ip_dhcp_lem_request_key_build(
          unit,
          ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_SPOOF_DHCP,
          (spoof_id & 0xFFFF),
          ipv4_info->lif_ndx,
          ipv4_info->smac,
          &request.key
        );
   SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }
  else
  {
    arad_pp_src_bind_ipv4_static_lem_request_key_build(
          unit,
          ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC,
          ipv4_info->lif_ndx,
          ipv4_info->sip,
          ipv4_info->vsi_id,
          &request.key
    );
  }

  payload.flags = ARAD_PP_FWD_DECISION_PARSE_DEST;
  payload.is_dynamic = 0;
  if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit)) {
    ARAD_PP_FRWRD_DECISION_FEC_SET(unit, &frwrd_info, ipv4_info->lif_ndx);
    arad_pp_src_bind_lem_payload_build(unit, &frwrd_info, &payload);
  }
  res = arad_pp_lem_access_entry_add_unsafe(
          unit,
          &request,
          &payload,
          &ack);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
 
  if(ack.is_success)
  {
    *success = SOC_SAND_SUCCESS;
  }
  else
  {
    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
  }
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_src_bind_ipv4_lem_add()",0,0);
}

static
  uint32
    arad_pp_src_bind_ipv4_lem_get(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY               *ipv4_info,
      SOC_SAND_IN  uint8                                      dhcp,
      SOC_SAND_IN  uint32                                     spoof_id,
      SOC_SAND_OUT uint8                                     *found
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ipv4_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);

  if (dhcp)
  {
    /* spoof id is always <=0xFFFF */
    res = arad_pp_src_bind_ip_dhcp_lem_request_key_build(
          unit,
          ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_SPOOF_DHCP,
          (spoof_id & 0xFFFF),
          ipv4_info->lif_ndx,
          ipv4_info->smac,
          &request.key
        );
   SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }
  else
  {
    arad_pp_src_bind_ipv4_static_lem_request_key_build(
      unit,
      ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC,
      ipv4_info->lif_ndx,
      ipv4_info->sip,
      ipv4_info->vsi_id,
      &request.key
    );
  }

  res = arad_pp_lem_access_entry_by_key_get_unsafe(
          unit,
          &(request.key),
          &payload,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_frwrd_ipv4_lem_route_get()",0,0);
}

static
  uint32
    arad_pp_src_bind_ipv4_lem_remove(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY               *ipv4_info,
      SOC_SAND_IN  uint8                                      dhcp,
      SOC_SAND_IN  uint32                                     spoof_id,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS             *ack_status
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ipv4_info);
  SOC_SAND_CHECK_NULL_INPUT(ack_status);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);

  request.command = ARAD_PP_LEM_ACCESS_CMD_DELETE;

  if (dhcp)
  {
    /* spoof id is always <=0xFFFF */
    res = arad_pp_src_bind_ip_dhcp_lem_request_key_build(
          unit,
          ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_SPOOF_DHCP,
          (spoof_id & 0xFFFF),
          ipv4_info->lif_ndx,
          ipv4_info->smac,
          &request.key
        );
   SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }
  else
  {
    arad_pp_src_bind_ipv4_static_lem_request_key_build(
      unit,
      ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC,
      ipv4_info->lif_ndx,
      ipv4_info->sip,
      ipv4_info->vsi_id,
      &request.key
    );
  }

  res = arad_pp_lem_access_entry_remove_unsafe(
          unit,
          &request,
          ack_status
        );

  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (!((ack_status->is_success == TRUE) || (ack_status->reason == ARAD_PP_LEM_ACCESS_FAIL_REASON_DELETE_UNKNOWN)))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LEM_ACCESS_FAIL_REASON_OUT_OF_RANGE_ERR, 50, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_src_bind_ipv4_lem_remove()",0,0);
}



static
  uint32
    arad_pp_src_bind_ipv6_lem_add(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *ipv6_info,
      SOC_SAND_IN  uint32                                     spoof_id,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;
  SOC_PPC_LIF_ENTRY_INFO
    lif_entry_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ipv6_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);

  request.command = ARAD_PP_LEM_ACCESS_CMD_INSERT;
  /* spoof id is always <=0xFFFF */
  res = arad_pp_src_bind_ip_dhcp_lem_request_key_build(
        unit,
        ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_SPOOF_DHCP,
        (spoof_id & 0xFFFF),
        ipv6_info->lif_ndx,
        ipv6_info->smac,
        &request.key
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_lif_table_entry_get_unsafe(
          unit,
          ipv6_info->lif_ndx,
          &lif_entry_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  payload.flags = ARAD_PP_FWD_DECISION_PARSE_DEST;
  payload.is_dynamic = 0;

  /* LEM configuration. Don't call this function because of ucode isn't ready */
  res = arad_pp_lem_access_entry_add_unsafe(
          unit,
          &request,
          &payload,
          &ack);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
 
  if(ack.is_success)
  {
    *success = SOC_SAND_SUCCESS;
  }
  else
  {
    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_src_bind_ipv6_lem_add()",0,0);

}

STATIC uint32
  arad_pp_src_bind_ipv6_lem_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *ipv6_info,
    SOC_SAND_IN  uint32                                     spoof_id,
    SOC_SAND_OUT uint8                                     *found
  )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ipv6_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);

  /* spoof id is always <=0xFFFF */
  res = arad_pp_src_bind_ip_dhcp_lem_request_key_build(
        unit,
        ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_SPOOF_DHCP,
        (spoof_id & 0xFFFF),
        ipv6_info->lif_ndx,
        ipv6_info->smac,
        &request.key
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* LEM configuration. Don't call this function because of ucode isn't ready */
  res = arad_pp_lem_access_entry_by_key_get_unsafe(
          unit,
          &(request.key),
          &payload,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_src_bind_ipv6_lem_get()",0,0);

}

static
  uint32
    arad_pp_src_bind_ipv6_lem_remove(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *ipv6_info,
      SOC_SAND_IN  uint32                                     spoof_id,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS             *ack_status
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ipv6_info);
  SOC_SAND_CHECK_NULL_INPUT(ack_status);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);

  request.command = ARAD_PP_LEM_ACCESS_CMD_DELETE;

  /* spoof id is always <=0xFFFF */
  res = arad_pp_src_bind_ip_dhcp_lem_request_key_build(
        unit,
        ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_SPOOF_DHCP,
        ARAD_PP_SRC_BIND_GET_16LSB(spoof_id),
        ipv6_info->lif_ndx,
        ipv6_info->smac,
        &request.key
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* LEM configuration. Don't call this function because of ucode isn't ready */
  res = arad_pp_lem_access_entry_remove_unsafe(
          unit,
          &request,
          ack_status
        );
  
  if (!((ack_status->is_success == TRUE) || (ack_status->reason == ARAD_PP_LEM_ACCESS_FAIL_REASON_DELETE_UNKNOWN)))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LEM_ACCESS_FAIL_REASON_OUT_OF_RANGE_ERR, 20, exit);
  }

  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_src_bind_ipv6_lem_remove()",0,0);
}

STATIC
  uint32
    arad_pp_src_bind_ipv6_static_tcam_callback(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32  user_data
    )
{
  uint32
    res = SOC_SAND_OK,
    tcam_db_id,
    access_profile_id;
  uint8
    ingress_learn_enable,
    ingress_learn_oppurtunistic;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Set access_profile id depending on TCAM DB ID */
  tcam_db_id = user_data;

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(
          unit,
          tcam_db_id,
          0, /* No 320b DB */
          &access_profile_id
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  switch (user_data) /* tcam DB ID */
  {

  case ARAD_PP_TCAM_STATIC_ACCESS_ID_SRC_BIND_IPV6:

    res = arad_pp_flp_tk_epon_uni_v6_ing_learn_get(unit, &ingress_learn_enable, &ingress_learn_oppurtunistic);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    
    res = arad_pp_flp_lookups_ethernet_tk_epon_uni_v6 (
               unit,
               access_profile_id,
               ingress_learn_enable,
               ingress_learn_oppurtunistic
               );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    break;
  }

  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_callback()", 0, 0);
}

uint32
  arad_pp_src_bind_ipv6_static_tcam_init_unsafe(
    SOC_SAND_IN int unit
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_SUCCESS_FAILURE
    success;
  ARAD_TCAM_ACCESS_INFO
    tcam_access_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_STATIC_TCAM_INIT_UNSAFE);

  arad_ARAD_TCAM_ACCESS_INFO_clear(&tcam_access_info);

  tcam_access_info.action_bitmap_ndx                   = ARAD_TCAM_ACTION_SIZE_FIRST_20_BITS;
  tcam_access_info.bank_owner                          = ARAD_TCAM_BANK_OWNER_FLP_TCAM;    
  tcam_access_info.callback                            = arad_pp_src_bind_ipv6_static_tcam_callback;
  tcam_access_info.entry_size                          = ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS;
  tcam_access_info.is_direct                           = FALSE;
  tcam_access_info.min_banks                           = 0;
  tcam_access_info.prefix_size                         = 4;
  tcam_access_info.user_data                           = ARAD_PP_TCAM_STATIC_ACCESS_ID_SRC_BIND_IPV6;

  res = arad_tcam_access_create_unsafe(
          unit,
          ARAD_PP_TCAM_STATIC_ACCESS_ID_SRC_BIND_IPV6,
          &tcam_access_info,
          &success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);      

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_init_unsafe()", 0, 0);
}

STATIC
  uint32
    arad_pp_src_bind_ipv6_static_tcam_add(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_TCAM_ACTION
    action;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    key;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  ARAD_TCAM_ACTION_clear(&action);

  /* Action is doesn't matter the same as do in LEM. It can be 0. 
     * Since all care about is found key or not
     */
  action.value[0] = 0;





  key.type = ARAD_IP_TCAM_ENTRY_TYPE_IPV6_STATIC_SRC_BIND;
  key.key.ipv6_static_src_bind = *src_bind_info;
  key.vrf_ndx = 0;

  
  res = arad_pp_frwrd_ip_tcam_route_add_unsafe(
		unit,
		&key,
		&action,
		success
	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10,exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_add()", 0, 0);
}

uint32
  arad_pp_src_bind_ipv6_static_tcam_remove(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *src_bind_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    key;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  key.type = ARAD_IP_TCAM_ENTRY_TYPE_IPV6_STATIC_SRC_BIND;
  key.key.ipv6_static_src_bind = *src_bind_info;
  key.vrf_ndx = 0;

  res = arad_pp_frwrd_ip_tcam_route_remove_unsafe(
  		unit,
  		&key
  	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_remove()", 0, 0);
}

STATIC
  uint32
    arad_pp_src_bind_ipv6_static_tcam_get(
      SOC_SAND_IN  int                         unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY   *src_bind_info,
      SOC_SAND_OUT uint8                         *found
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    key;
  ARAD_TCAM_ACTION
    action;
  uint8
    hit_bit = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(found);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  ARAD_TCAM_ACTION_clear(&action);

  key.type = ARAD_IP_TCAM_ENTRY_TYPE_IPV6_STATIC_SRC_BIND;
  key.key.ipv6_static_src_bind = *src_bind_info;
  key.vrf_ndx = 0;

  res = arad_pp_frwrd_ip_tcam_route_get_unsafe(
  		unit,
  		&key,
  		TRUE,
  		&action,
  		found,
  		&hit_bit
  	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_get()", 0, 0);
}


uint32
  arad_pp_src_bind_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  if (SOC_DPP_L3_SRC_BIND_IPV6_ENABLE(unit))
  {
    if(!(SOC_IS_JERICHO(unit))){
       res = arad_pp_src_bind_ipv6_static_tcam_init_unsafe(unit);
    }
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_init_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv4_add_verify(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_ADD_VERIFY);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_SRC_BIND_IPV4_ENTRY, src_bind_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_add_verify()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv4_add_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_src_bind_ipv4_add_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_add_unsafe()", 0, 0);
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
STATIC uint32 arad_pp_src_bind_ipv4_static_kaps_add_dbal(
      SOC_SAND_IN  int                                        unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY               *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
    )
{
    uint32  res = SOC_SAND_OK;
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint32 payload = 0;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    DBAL_QUAL_VALS_CLEAR(qual_vals);
    DBAL_QUAL_VAL_ENCODE_IPV4_SIP(&qual_vals[0], src_bind_info->sip, src_bind_info->prefix_len);
    DBAL_QUAL_VAL_ENCODE_FID(&qual_vals[1], src_bind_info->vsi_id);

    payload = JER_PP_KAPS_FEC_ENCODE(src_bind_info->lif_ndx);

    res = arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS,
                                 qual_vals, 0, &payload, success);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_static_kaps_add_dbal()", 0, 0);
}

STATIC uint32 arad_pp_src_bind_ipv4_static_kaps_get_dbal(
    SOC_SAND_IN  int                                        unit,
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY               *src_bind_info,
    SOC_SAND_OUT uint8                                     *found
    )
{
    uint32   res = SOC_SAND_OK;
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint32 payload = 0;
    uint32 priority = 0;
    uint8  hit_bit = 0;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    DBAL_QUAL_VALS_CLEAR(qual_vals);
    DBAL_QUAL_VAL_ENCODE_IPV4_SIP(&qual_vals[0], src_bind_info->sip, src_bind_info->prefix_len);
    DBAL_QUAL_VAL_ENCODE_FID(&qual_vals[1], src_bind_info->vsi_id);

    res = arad_pp_dbal_entry_get( unit,  SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS, qual_vals, &payload, &priority, &hit_bit, found);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_static_kaps_get_dbal()", 0, 0);
}


STATIC uint32 arad_pp_src_bind_ipv4_static_kaps_remove_dbal(
      SOC_SAND_IN  int                                        unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY               *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
    )
{
    uint32 res = SOC_SAND_OK;
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    DBAL_QUAL_VALS_CLEAR(qual_vals);
    DBAL_QUAL_VAL_ENCODE_IPV4_SIP(&qual_vals[0], src_bind_info->sip, src_bind_info->prefix_len);
    DBAL_QUAL_VAL_ENCODE_FID(&qual_vals[1], src_bind_info->vsi_id);

    res = arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS, qual_vals, success);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_remove_dbal()", 0, 0);
}
#endif

uint32 
  arad_pp_src_bind_ipv4_add_internal_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info,
      SOC_SAND_IN  uint8                                  ignore_key,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;
  SOC_SAND_OCC_BM_PTR
    srcbind_arb_occ;
  uint32
    place;
  uint8
    found, found_isem, found_lem;
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY
    route_key;
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY
    uc_route_key;
  uint16 spoof_id;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_ADD_INTERNAL_UNSAFE);
    
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY_clear(&route_key);
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY_clear(&uc_route_key);
  found = found_isem = found_lem = 0;
  spoof_id  = 0;
  /* Check the added entry is existed, if existed, do nothing */
  res = arad_pp_src_bind_ipv4_get_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          &found,
          &found_isem,
          &found_lem,
          &spoof_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  sem_entry.sem_result_ndx = spoof_id;

  if (!found)
  {
    if (src_bind_info->smac_valid == 0)
    {
      if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit) &&
          src_bind_info->is_network) {
          if (SOC_IS_JERICHO(unit)) {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
            res = arad_pp_src_bind_ipv4_static_kaps_add_dbal(unit, src_bind_info, success);
#else
            /* NO support for other LPM DB in Jericho - we can add support for TCAM */
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_FAILURE_OUT_OF_RESOURCES, SOC_SAND_NULL_POINTER_ERR, exit);
#endif
          } 
          else 
          {
              if (src_bind_info->vsi_id) {
                /* MP service.
                 * Add a entry for IPv4 network, configure the entry to LPM DB
                 */
                route_key.subnet.prefix_len = src_bind_info->prefix_len;
                route_key.subnet.ip_address = src_bind_info->sip;
                res = soc_ppd_frwrd_ipv4_vrf_route_add(unit, 
                                                     src_bind_info->vsi_id,
                                                     &route_key,
                                                     src_bind_info->lif_ndx,
                                                     0,
                                                     success);
              } else {
                /* P2P service */
                uc_route_key.subnet.prefix_len = src_bind_info->prefix_len;
                uc_route_key.subnet.ip_address = src_bind_info->sip;
                res = soc_ppd_frwrd_ipv4_uc_route_add(unit,
                                                      &uc_route_key,
                                                      src_bind_info->lif_ndx,
                                                      success);
              }
          }
          SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit); 
      } else {
        /* Add a entry for IPv4 host, Static DB, call LEM interfaces. needn't spoof_id at static DB, set it as 0 */
        res = arad_pp_src_bind_ipv4_lem_add(
            unit,
            src_bind_info,
            FALSE,
            0,
            success
          );
         SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit); 
      }
    } 
    else
    {
      /* DHCP DB, should configure SEM+LEM */
      /* SEM configuration */
      res = sw_state_access[unit].dpp.soc.arad.pp.arad_sw_db_src_binds.srcbind_arb_occ.get(unit, &srcbind_arb_occ);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
      if (!ignore_key)
      {
        if (!found_isem) {
          ARAD_PP_CLEAR(&sem_key, ARAD_PP_ISEM_ACCESS_KEY, 1);
          
          res = arad_pp_l3_src_bind_key_to_sem_key_unsafe(unit, src_bind_info, NULL, &sem_key);
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
          
          ARAD_PP_CLEAR(&sem_entry, ARAD_PP_ISEM_ACCESS_ENTRY, 1);
          res = soc_sand_occ_bm_get_next(
                       unit,
                       srcbind_arb_occ,
                       &place,
                       &found
                     );
          SOC_SAND_CHECK_FUNC_RESULT(res, 31, exit);
          if (!found)
          {
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_SRC_BIND_SW_DB_NO_RES_ERR, 12, exit);
          }
          sem_entry.sem_result_ndx = place;
          res = arad_pp_isem_access_entry_add_unsafe(
                  unit,
                  &sem_key,
                  &sem_entry,
                  success
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 32, exit);

        /* Always need to add a entry to LEM because have a new spoof id */
          res = arad_pp_src_bind_ipv4_lem_add(
            unit,
            src_bind_info,
            TRUE,
            sem_entry.sem_result_ndx,
            success
          );
          SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);
          res = soc_sand_occ_bm_occup_status_set(
                       unit,
                       srcbind_arb_occ,
                       sem_entry.sem_result_ndx,
                       TRUE
                     );
          SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
        }else {
          if (!found_lem) {
            res = arad_pp_src_bind_ipv4_lem_add(
              unit,
              src_bind_info,
              TRUE,
              sem_entry.sem_result_ndx,
              success
            );
            SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
          }
        }

        if (!found_isem || (found_isem && !found_lem)) {
          res = arad_pp_sw_db_src_bind_spoof_id_ref_count_set(unit, sem_entry.sem_result_ndx, TRUE);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit);
        }
      } 
      else
      {
        *success = SOC_SAND_SUCCESS;
      }
    }
  }

  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_l2_lif_ac_add_internal_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv4_get_verify(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_GET_VERIFY);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);

  SOC_SAND_ERR_IF_ABOVE_NOF(src_bind_info->lif_ndx, SOC_DPP_DEFS_GET(unit, nof_local_lifs), SOC_PPC_LIF_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_SRC_BIND_IPV4_ENTRY, src_bind_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_get_verify()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv4_get_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info,
      SOC_SAND_OUT uint8                                 *found
    )
{
  uint32
    res = SOC_SAND_OK;
  uint8 found_isem, found_lem;
  uint16 spoof_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  found_isem = found_lem = spoof_id = 0;
  res = arad_pp_src_bind_ipv4_get_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          found,
          &found_isem,
          &found_lem,
          &spoof_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_get_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv4_get_internal_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info,
      SOC_SAND_IN  uint8                                  ignore_key,
      SOC_SAND_OUT uint8                                 *found,
      SOC_SAND_OUT uint8                                 *found_isem,
      SOC_SAND_OUT uint8                                 *found_lem,
      SOC_SAND_OUT uint16                                *spoof_id
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY
    route_key;
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY
    uc_route_key;
  SOC_PPC_FRWRD_IP_ROUTE_STATUS 
    route_status;
  SOC_PPC_FRWRD_IP_ROUTE_LOCATION
    location;
  SOC_PPC_FEC_ID 
    fec_id = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_GET_INTERNAL_UNSAFE);
  
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  ARAD_PP_CLEAR(&sem_key, ARAD_PP_ISEM_ACCESS_KEY, 1);
  ARAD_PP_CLEAR(&sem_entry, ARAD_PP_ISEM_ACCESS_ENTRY, 1);
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY_clear(&route_key);
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY_clear(&uc_route_key);

  if (src_bind_info->smac_valid == 0)
  {
    if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit) &&
        src_bind_info->is_network) {
          if (SOC_IS_JERICHO(unit)) {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
            res = arad_pp_src_bind_ipv4_static_kaps_get_dbal(unit, src_bind_info, found);
#else
            /* NO support for other LPM DB in Jericho - we can add support for TCAM */
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_FAILURE_OUT_OF_RESOURCES, SOC_SAND_NULL_POINTER_ERR, exit);
#endif
      } 
      else
      {
            if (src_bind_info->vsi_id) {
              /* MP service
               * Get a entry for IPv4 network from LPM DB 
               */
              route_key.subnet.prefix_len = src_bind_info->prefix_len;
              route_key.subnet.ip_address = src_bind_info->sip;
              res = soc_ppd_frwrd_ipv4_vrf_route_get(unit, 
                                                   src_bind_info->vsi_id,
                                                   &route_key, 
                                                   TRUE,
                                                   &fec_id,
                                                   &route_status,
                                                   &location,
                                                    found);
            } else {
              /* P2P service */
              uc_route_key.subnet.prefix_len = src_bind_info->prefix_len;
              uc_route_key.subnet.ip_address = src_bind_info->sip;
              res = soc_ppd_frwrd_ipv4_uc_route_get(unit, 
                                                     &uc_route_key, 
                                                     TRUE,
                                                     &fec_id,
                                                     &route_status,
                                                     &location,
                                                      found);
            }
        }
        SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit); 
    } else {
      /* Static DB, call LEM interfaces */
       res = arad_pp_src_bind_ipv4_lem_get(
               unit,
               src_bind_info,
               FALSE,
               0,
               found);
      SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit); 
    }
  }
  else
  {
    /* DHCP DB, first call SEM interfaces, then call LEM interfaces */ 
    if(!ignore_key)
    {
      res = arad_pp_l3_src_bind_key_to_sem_key_unsafe(
              unit,
              src_bind_info,
              NULL,
              &sem_key);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      res = arad_pp_isem_access_entry_get_unsafe(
              unit,
              &sem_key,
              &sem_entry,
              found_isem
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 21, exit);

      res = arad_pp_src_bind_ipv4_lem_get(
              unit,
              src_bind_info,
              TRUE,
              sem_entry.sem_result_ndx,
              found_lem);
      SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);
      *found = *found_isem & *found_lem;
      if (*found_isem) {
        *spoof_id = sem_entry.sem_result_ndx;
      }
    }
    else
    {
      *found = TRUE;
    }
  }

  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_get_internal_unsafe()", 0, 0);
}

uint32
  arad_pp_src_bind_ipv4_remove_verify(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_REMOVE_VERIFY);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);

  SOC_SAND_ERR_IF_ABOVE_NOF(src_bind_info->lif_ndx, SOC_DPP_DEFS_GET(unit, nof_local_lifs), SOC_PPC_LIF_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_SRC_BIND_IPV4_ENTRY, src_bind_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_remove_verify()", 0, 0);
}

uint32
  arad_pp_src_bind_ipv4_remove_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_REMOVE_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_src_bind_ipv4_remove_internal_unsafe(
          unit,
          src_bind_info,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_remove_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv4_remove_internal_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY           *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  uint32
    res = SOC_SAND_OK;
  uint8 
    found, found_isem, found_lem;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;
  SOC_SAND_OCC_BM_PTR
    srcbind_arb_occ;
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY
    route_key;
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY
    uc_route_key;
  uint16 ref_count = 0;
  uint16 spoof_id = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_REMOVE_INTERNAL_UNSAFE);
  
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY_clear(&route_key);
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY_clear(&uc_route_key);
  found = found_isem = found_lem = 0;

 *success = SOC_SAND_SUCCESS;
  /* Check the removed entry is existed */
  res = arad_pp_src_bind_ipv4_get_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          &found,
          &found_isem,
          &found_lem,
          &spoof_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  sem_entry.sem_result_ndx = spoof_id;

  if(found)
  {
    if (src_bind_info->smac_valid == 0)
    {
      if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit) &&
          src_bind_info->is_network) {
        if (SOC_IS_JERICHO(unit)) {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
          res = arad_pp_src_bind_ipv4_static_kaps_remove_dbal(unit, src_bind_info, success);
#else
          /* NO support for other LPM DB in Jericho - we can add support for TCAM */
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_FAILURE_OUT_OF_RESOURCES, SOC_SAND_NULL_POINTER_ERR, exit);
#endif
            } 
            else
            {
              if (src_bind_info->vsi_id) {
                /* MP service
    		     * Remove a entry for IPv4 network from LPM DB
    		     */
                route_key.subnet.prefix_len = src_bind_info->prefix_len;
                route_key.subnet.ip_address = src_bind_info->sip;
                res = soc_ppd_frwrd_ipv4_vrf_route_remove(unit, 
                                                        src_bind_info->vsi_id,
                                                        &route_key, 
                                                         success);
              } else {
                /* P2P service */
                uc_route_key.subnet.prefix_len = src_bind_info->prefix_len;
                uc_route_key.subnet.ip_address = src_bind_info->sip;
                res = soc_ppd_frwrd_ipv4_uc_route_remove(unit, 
                                                         &uc_route_key, 
                                                         success);
              }
          }
          SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit); 
      } else {
        /* Static DB, remove LEM configuration */
        res = arad_pp_src_bind_ipv4_lem_remove(unit, src_bind_info, FALSE, 0, &ack);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
        if (ack.is_success == FALSE)
        {
          *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
        }
      }
    }
    else
    {
      /* DHCP DB, first remove LEM configuration, then remove SEM configuration  */   
      res = arad_pp_l3_src_bind_key_to_sem_key_unsafe(unit, src_bind_info, NULL, &sem_key);
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      res = arad_pp_src_bind_ipv4_lem_remove(
              unit,
              src_bind_info,
              TRUE,
              sem_entry.sem_result_ndx,
              &ack);
      SOC_SAND_CHECK_FUNC_RESULT(res, 31, exit);
      if (ack.is_success == FALSE)
      {
        *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
      }

      res = arad_pp_sw_db_src_bind_spoof_id_ref_count_set(unit, sem_entry.sem_result_ndx, FALSE);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
      res = sw_state_access[unit].dpp.soc.arad.pp.arad_sw_db_src_binds.spoof_id_ref_count.get(unit, sem_entry.sem_result_ndx, &ref_count);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 33, exit);
      /* last one */
      if (!ref_count) {
        res = arad_pp_isem_access_entry_remove_unsafe(
                unit,
                &sem_key
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
        res = sw_state_access[unit].dpp.soc.arad.pp.arad_sw_db_src_binds.srcbind_arb_occ.get(unit, &srcbind_arb_occ);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
        res = soc_sand_occ_bm_occup_status_set(
                     unit,
                     srcbind_arb_occ,
                     sem_entry.sem_result_ndx,
                     FALSE
                   );
        SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
      }
    }
  }

  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_remove_internal_unsafe()", 0, 0);
}

uint32
  arad_pp_src_bind_ipv4_table_clear_verify(
    SOC_SAND_IN  int                                 unit
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_TABLE_CLEAR_VERIFY);

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_table_clear_verify()", 0, 0);
}


uint32
  arad_pp_src_bind_ipv4_table_clear_unsafe(
      SOC_SAND_IN  int                                 unit
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_TABLE_CLEAR_UNSAFE);

  res = arad_pp_src_bind_ipv4_table_clear_internal_unsafe(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_table_clear_unsafe()", 0, 0);
}

STATIC uint32
  arad_pp_src_bind_ipv4_host_clear_unsafe(
    SOC_SAND_IN  int                                  unit
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_LEM_ACCESS_KEY            key;
  ARAD_PP_LEM_ACCESS_KEY            key_mask;
  SOC_SAND_TABLE_BLOCK_RANGE        block_range;
  uint32                            nof_entries;
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION        action;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_PP_LEM_ACCESS_KEY_clear(&key);
  ARAD_PP_LEM_ACCESS_KEY_clear(&key_mask);
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_clear(&action);
  soc_sand_SAND_TABLE_BLOCK_RANGE_clear(&block_range);

  /* build key to include host */
  arad_pp_src_bind_ipv4_static_lem_request_key_build(
        unit,
        ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC,
        0,
        0,
        0,
        &key
  );

  /* consider all ipv4 entries*/
  key_mask.type = ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC;
  key_mask.prefix.value = SOC_IS_JERICHO(unit)? 0xf: ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK;
  key_mask.prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IPV4_SPOOF_STATIC;


  /* set action */
  action.type = SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_REMOVE;
  res = arad_pp_frwrd_lem_traverse_internal_unsafe(unit,&key,&key_mask,NULL,&block_range,&action, TRUE,&nof_entries);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_host_clear_unsafe()", 0, 0);
}


/*********************************************************************
*     Gets the Ipv4 source bind host table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
    arad_pp_src_bind_ipv4_host_get_block_unsafe(
      SOC_SAND_IN    int                                        unit,
      SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range,
      SOC_SAND_OUT   SOC_PPC_SRC_BIND_IPV4_ENTRY               *ipv4_info,
      SOC_SAND_OUT   SOC_PPC_FRWRD_IP_ROUTE_STATUS             *ipv4_status,
      SOC_SAND_OUT   uint32                                    *nof_entries
  )
{
  uint32
    res = SOC_SAND_OK,
    read_index = 0,
    valid_index = 0;
  ARAD_PP_LEM_ACCESS_KEY            key;
  ARAD_PP_LEM_ACCESS_KEY            key_mask;
  uint32                            access_only=0;
  ARAD_PP_LEM_ACCESS_KEY            *read_keys=NULL;
  ARAD_PP_LEM_ACCESS_PAYLOAD        *read_vals=NULL;
  SOC_SAND_PP_DESTINATION_ID        dest_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_PP_LEM_ACCESS_KEY_clear(&key);
  ARAD_PP_LEM_ACCESS_KEY_clear(&key_mask);
  *nof_entries = 0;

  /* build key to include host */
  arad_pp_src_bind_ipv4_static_lem_request_key_build(
        unit,
        ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC,
        0,
        0,
        0,
        &key
  );

  key_mask.type = ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC;
  key_mask.prefix.value = ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK;
  key_mask.prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IPV4_SPOOF_STATIC;

  /* malloc place for keys and vals */
  read_keys = soc_sand_os_malloc_any_size(sizeof(ARAD_PP_LEM_ACCESS_KEY) * block_range->entries_to_act,"read_keys");
  read_vals = soc_sand_os_malloc_any_size(sizeof(ARAD_PP_LEM_ACCESS_PAYLOAD) * block_range->entries_to_act,"read_vals");

  res = arad_pp_frwrd_lem_get_block_unsafe(unit,&key,&key_mask,NULL,access_only, block_range,read_keys,read_vals,nof_entries);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *  Loop on the entry index
   */
  for (read_index = 0; read_index < *nof_entries; ++read_index)
  {
    if ((read_keys[read_index].type == ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC))
    {
      arad_pp_src_bind_ipv4_static_lem_request_key_parse(
                                                        unit,
                                                        &read_keys[read_index],
                                                        &ipv4_info[valid_index]
                                                        );

      /* get access information, so read entry according to key */
      ipv4_status[valid_index] = SOC_PPC_FRWRD_IP_ROUTE_STATUS_COMMITED;

      if(read_vals[read_index].flags & ARAD_PP_FWD_DECISION_PARSE_ACCESSED)
      {
        ipv4_status[valid_index] |= SOC_PPC_FRWRD_IP_ROUTE_STATUS_ACCESSED;
      }

      if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit)) 
      {
        sal_memset(&dest_id, 0, sizeof(SOC_SAND_PP_DESTINATION_ID));
        res = arad_pp_frwrd_ipv4_em_dest_to_sand_dest(
                                                unit,
                                                read_vals[read_index].dest,
                                                &dest_id
          );
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
        ipv4_info->lif_ndx = dest_id.dest_val;
        
      }
      ++valid_index;
    }
  }

  *nof_entries =  valid_index;

exit:
  if (read_keys!=NULL) {
    soc_sand_os_free_any_size(read_keys);
  }

  if (read_vals!=NULL) {
    soc_sand_os_free_any_size(read_vals);
  }

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_host_get_block_unsafe()", 0, 0);
}

/*********************************************************************
*     Gets the Ipv6 source bind table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_src_bind_ipv6_get_block_unsafe(
    SOC_SAND_IN    int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE     *block_range_key,
    SOC_SAND_OUT   SOC_PPC_SRC_BIND_IPV6_ENTRY        *src_bind_ipv6,
    SOC_SAND_OUT   SOC_PPC_FRWRD_IP_ROUTE_STATUS      *ipv6_status,
    SOC_SAND_OUT   uint32                             *nof_entries
  )
{
  uint32
    res = SOC_SAND_OK,
    indx;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    *keys = NULL;
  ARAD_TCAM_ACTION
    *actions = NULL;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(block_range_key);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_ipv6);
  SOC_SAND_CHECK_NULL_INPUT(ipv6_status);
  SOC_SAND_CHECK_NULL_INPUT(nof_entries);

  if(block_range_key->entries_to_act == 0)
  {
    *nof_entries = 0;
    goto exit;
  }

  ARAD_ALLOC_ANY_SIZE(keys,ARAD_PP_IP_TCAM_ENTRY_KEY,block_range_key->entries_to_act,"keys src_bind_ipv6_get_block");
  ARAD_ALLOC_ANY_SIZE(actions,ARAD_TCAM_ACTION,block_range_key->entries_to_act,"actions src_bind_ipv6_get_block");

  for(indx = 0; indx < block_range_key->entries_to_act; ++indx)
  {
    keys[indx].type = ARAD_IP_TCAM_ENTRY_TYPE_IPV6_STATIC_SRC_BIND;
    keys[indx].key.ipv6_static_src_bind = src_bind_ipv6[indx];
  }

  res = arad_pp_frwrd_ip_tcam_route_get_block_unsafe(
           unit,
           block_range_key,
           keys,
           actions,
           nof_entries
         );
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  for(indx = 0; indx < *nof_entries; ++indx)
  {
    src_bind_ipv6[indx] = keys[indx].key.ipv6_static_src_bind;
    src_bind_ipv6[indx].is_network = (keys[indx].key.ipv6_static_src_bind.prefix_len ? 1 : 0);
    if(ipv6_status)
    {
      ipv6_status[indx] = SOC_PPC_FRWRD_IP_ROUTE_STATUS_COMMITED;
    }
  }
exit:
  if(keys != NULL)
  {
    soc_sand_os_free(keys);
  }
  if(actions != NULL)
  {
    ARAD_FREE_ANY_SIZE(actions);
  }

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_get_block_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv4_table_clear_internal_unsafe(
      SOC_SAND_IN  int                                 unit
    )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    flags = SOC_PPC_FRWRD_IP_LPM_ONLY;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV4_TABLE_CLEAR_INTERNAL_UNSAFE);

  /* Static DB, Remove LPM entries */
  if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit))
  {
    res = arad_pp_frwrd_ipv4_vrf_all_routing_tables_clear_unsafe(unit, flags);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  /* Static DB, Remove LEM entries */
  res = arad_pp_src_bind_ipv4_host_clear_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* DHCP DB, first call SEM interfaces, then call LEM interfaces */   

  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv4_remove_internal_unsafe()", 0, 0);
}


uint32 
  arad_pp_src_bind_ipv6_add_verify(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_ADD_VERIFY);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);

  SOC_SAND_ERR_IF_ABOVE_NOF(src_bind_info->lif_ndx, SOC_DPP_DEFS_GET(unit, nof_local_lifs), SOC_PPC_LIF_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_SRC_BIND_IPV6_ENTRY, src_bind_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_add_verify()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv6_add_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_src_bind_ipv6_add_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_add_unsafe()", 0, 0);

}

uint32 arad_pp_src_bind_ipv6_static_tcam_add_dbal(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
    )
{
    uint32  res = SOC_SAND_OK;
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint32 tcam_payload[2] = {0};
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    DBAL_QUAL_VALS_CLEAR(qual_vals);

    /* Static DB, configure TCAM */
    qual_vals[0].type= SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW;
    qual_vals[0].val.arr[0]= src_bind_info->sip6.address[0];
    qual_vals[0].val.arr[1]= src_bind_info->sip6.address[1];
    qual_vals[0].is_valid.arr[0]= SOC_SAND_U32_MAX;
    qual_vals[0].is_valid.arr[1]= SOC_SAND_U32_MAX;
    qual_vals[1].type= SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;
    qual_vals[1].val.arr[0]= src_bind_info->sip6.address[2];
    qual_vals[1].val.arr[1]= src_bind_info->sip6.address[3];
    qual_vals[1].is_valid.arr[0]= SOC_SAND_U32_MAX;
    qual_vals[1].is_valid.arr[1]= SOC_SAND_U32_MAX;

    qual_vals[2].type= SOC_PPC_FP_QUAL_IRPP_IN_LIF;
    qual_vals[2].val.arr[0]= src_bind_info->lif_ndx;
    qual_vals[2].is_valid.arr[0]= 0x3ffff;/*18 bit*/
    /*
     * COVERITY
     * Tcam payload in FLP stage uint32[2] ,which will not overrun.
     */
    /* coverity[overrun-buffer-arg] */
    /* coverity[overrun-buffer-val : FALSE] */
    res = arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_TCM,
                                 qual_vals, 0, &tcam_payload, success);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_add_dbal()", 0, 0);
}

uint32 arad_pp_src_bind_ipv6_static_tcam_get_dbal(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *src_bind_info,
    SOC_SAND_OUT uint8                         *found
    )
{
    uint32   res = SOC_SAND_OK;
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint32 tcam_payload = 0;
    uint32 priority = 0;
    uint8  hit_bit = 0;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    DBAL_QUAL_VALS_CLEAR(qual_vals);

    /* Static DB, configure TCAM */
    qual_vals[0].type= SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW;
    qual_vals[0].val.arr[0]= src_bind_info->sip6.address[0];
    qual_vals[0].val.arr[1]= src_bind_info->sip6.address[1];
    qual_vals[0].is_valid.arr[0]= SOC_SAND_U32_MAX;
    qual_vals[0].is_valid.arr[1]= SOC_SAND_U32_MAX;
    
    qual_vals[1].type= SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;
    qual_vals[1].val.arr[0]= src_bind_info->sip6.address[2];
    qual_vals[1].val.arr[1]= src_bind_info->sip6.address[3];
    qual_vals[1].is_valid.arr[0]= SOC_SAND_U32_MAX;
    qual_vals[1].is_valid.arr[1]= SOC_SAND_U32_MAX;
    qual_vals[2].type= SOC_PPC_FP_QUAL_IRPP_IN_LIF;
    qual_vals[2].val.arr[0]= src_bind_info->lif_ndx;
    qual_vals[2].is_valid.arr[0]= 0x3ffff;/*18 bit*/
    res = arad_pp_dbal_entry_get( unit,  SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_TCM, qual_vals, &tcam_payload, &priority, &hit_bit, found);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_get_dbal()", 0, 0);
}


uint32 arad_pp_src_bind_ipv6_static_tcam_remove_dbal(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY               *src_bind_info
    )
{
    uint32 res = SOC_SAND_OK;
    SOC_SAND_SUCCESS_FAILURE success = SOC_SAND_SUCCESS;
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    DBAL_QUAL_VALS_CLEAR(qual_vals);
    
    /* Static DB, configure TCAM */
    qual_vals[0].type= SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW;
    qual_vals[0].val.arr[0]= src_bind_info->sip6.address[0];
    qual_vals[0].val.arr[1]= src_bind_info->sip6.address[1];
    qual_vals[0].is_valid.arr[0]= SOC_SAND_U32_MAX;
    qual_vals[0].is_valid.arr[1]= SOC_SAND_U32_MAX;
    
    qual_vals[1].type= SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;
    qual_vals[1].val.arr[0]= src_bind_info->sip6.address[2];
    qual_vals[1].val.arr[1]= src_bind_info->sip6.address[3];
    qual_vals[1].is_valid.arr[0]= SOC_SAND_U32_MAX;
    qual_vals[1].is_valid.arr[1]= SOC_SAND_U32_MAX;
    qual_vals[2].type= SOC_PPC_FP_QUAL_IRPP_IN_LIF;
    qual_vals[2].val.arr[0]= src_bind_info->lif_ndx;
    qual_vals[2].is_valid.arr[0]= 0x3ffff;/*18 bit*/
    
    res = arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_TCM,
    qual_vals, &success);
    
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_remove_dbal()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv6_add_internal_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info,
      SOC_SAND_IN  uint8                                  ignore_key,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_OCC_BM_PTR
    srcbind_arb_occ;
  uint32
    place;
  uint8
    found, found_isem, found_lem;
  uint16 spoof_id = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_ADD_INTERNAL_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);
  found = found_isem = found_lem = 0;
  
  /* Check the added entry is existed, if existed, do nothing */
  res = arad_pp_src_bind_ipv6_get_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          &found,
          &found_isem,
          &found_lem,
          &spoof_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  sem_entry.sem_result_ndx = spoof_id;

  if (!found)
  {
    if (src_bind_info->smac_valid == 0)
    {
      /* Static DB, configure TCAM */
      if(SOC_IS_JERICHO(unit)){
         res = arad_pp_src_bind_ipv6_static_tcam_add_dbal(
                    unit,
                    src_bind_info,
                    success
                    );
      }
      else{
        res = arad_pp_src_bind_ipv6_static_tcam_add(
                   unit,
                   src_bind_info,
                   success
                   );
      }
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    } 
    else
    {
      /* DHCP DB, configure TCAM and LEM */
      if (!ignore_key)
      {
        res = sw_state_access[unit].dpp.soc.arad.pp.arad_sw_db_src_binds.srcbind_arb_occ.get(unit, &srcbind_arb_occ);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
        if (!found_isem) {
          ARAD_PP_CLEAR(&sem_key, ARAD_PP_ISEM_ACCESS_KEY, 1);
          
          res = arad_pp_l3_src_bind_key_to_sem_key_unsafe(
                  unit,
                  NULL,
                  src_bind_info,
                  &sem_key);
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
          
          ARAD_PP_CLEAR(&sem_entry, ARAD_PP_ISEM_ACCESS_ENTRY, 1);
          res = soc_sand_occ_bm_get_next(
                       unit,
                       srcbind_arb_occ,
                       &place,
                       &found
                     );
          SOC_SAND_CHECK_FUNC_RESULT(res, 31, exit);
          if (!found)
          {
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_SRC_BIND_SW_DB_NO_RES_ERR, 12, exit);
          }
          sem_entry.sem_result_ndx = place;
          res = arad_pp_isem_access_entry_add_unsafe(
                  unit,
                  &sem_key,
                  &sem_entry,
                  success
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 32, exit);
          /* Always need to add a entry to LEM because have a new spoof id */
          res = arad_pp_src_bind_ipv6_lem_add(
            unit,
            src_bind_info,
            sem_entry.sem_result_ndx,
            success
          );
          SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);

          res = soc_sand_occ_bm_occup_status_set(
                       unit,
                       srcbind_arb_occ,
                       sem_entry.sem_result_ndx,
                       TRUE
                     );
          SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
        } else {
          if(!found_lem) {
            res = arad_pp_src_bind_ipv6_lem_add(
              unit,
              src_bind_info,
              sem_entry.sem_result_ndx,
              success
            );
            SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
          }
        }

         if (!found_isem || (found_isem && !found_lem)) {
           res = arad_pp_sw_db_src_bind_spoof_id_ref_count_set(unit, sem_entry.sem_result_ndx, TRUE);
           SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit);
         }
        } 
        else
        {
          *success = SOC_SAND_SUCCESS;
        }
      }
    }
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    

  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_add_internal_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv6_get_verify(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_GET_VERIFY);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);

  SOC_SAND_ERR_IF_ABOVE_NOF(src_bind_info->lif_ndx, SOC_DPP_DEFS_GET(unit, nof_local_lifs), SOC_PPC_LIF_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_SRC_BIND_IPV6_ENTRY, src_bind_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_get_verify()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv6_get_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info,
      SOC_SAND_OUT uint8                                 *found
    )
{
  uint32
    res = SOC_SAND_OK;
  uint8 found_isem, found_lem;
  uint16 spoof_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(found);
  found_isem = found_lem = spoof_id = 0;
  res = arad_pp_src_bind_ipv6_get_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          found,
          &found_isem,
          &found_lem,
          &spoof_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_get_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv6_get_internal_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info,
      SOC_SAND_IN  uint8                                  ignore_key,
      SOC_SAND_OUT uint8                                 *found,
      SOC_SAND_OUT uint8                                 *found_isem,
      SOC_SAND_OUT uint8                                 *found_lem,
      SOC_SAND_OUT uint16                                *spoof_id
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_GET_INTERNAL_UNSAFE);
  
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  if (src_bind_info->smac_valid == 0)
  {
      /* Static DB, get TCAM */
      if(SOC_IS_JERICHO(unit)){
          res = arad_pp_src_bind_ipv6_static_tcam_get_dbal(
                     unit,
                     src_bind_info,
                     found
                     );
      }
      else{
          res = arad_pp_src_bind_ipv6_static_tcam_get(
                     unit,
                     src_bind_info,
                     found
                     );
      }
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  } 
  else
  {
    /* DHCP DB, get TCAM and LEM */
    if(!ignore_key)
    {
      res = arad_pp_l3_src_bind_key_to_sem_key_unsafe(unit, NULL, src_bind_info, &sem_key);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      res = arad_pp_isem_access_entry_get_unsafe(
              unit,
              &sem_key,
              &sem_entry,
              found_isem
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 21, exit);

      res = arad_pp_src_bind_ipv6_lem_get(
              unit,
              src_bind_info,
              sem_entry.sem_result_ndx,
              found_lem);
      SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);
      *found = *found_isem & *found_lem;
      if (*found_isem) {
        *spoof_id = sem_entry.sem_result_ndx;
      }
    }
    else
    {
      *found = TRUE;
    }
  }

  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_get_internal_unsafe()", 0, 0);
}


uint32
  arad_pp_src_bind_ipv6_remove_verify(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_REMOVE_VERIFY);
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);

  SOC_SAND_ERR_IF_ABOVE_NOF(src_bind_info->lif_ndx, SOC_DPP_DEFS_GET(unit, nof_local_lifs), SOC_PPC_LIF_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_SRC_BIND_IPV6_ENTRY, src_bind_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_remove_verify()", 0, 0);
}

uint32
  arad_pp_src_bind_ipv6_remove_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_REMOVE_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_src_bind_ipv6_remove_internal_unsafe(
          unit,
          src_bind_info,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_remove_unsafe()", 0, 0);
}

uint32 
  arad_pp_src_bind_ipv6_remove_internal_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY           *src_bind_info,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;
  uint8 
    found, found_isem, found_lem;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;
  SOC_SAND_OCC_BM_PTR
    srcbind_arb_occ;
  uint16 ref_count = 0;
  uint16 spoof_id = 0;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_REMOVE_INTERNAL_UNSAFE);
  
  SOC_SAND_CHECK_NULL_INPUT(src_bind_info);
  SOC_SAND_CHECK_NULL_INPUT(success);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);
  found = found_isem = found_lem = 0;
  *success = SOC_SAND_SUCCESS;

  res = arad_pp_src_bind_ipv6_get_internal_unsafe(
          unit,
          src_bind_info,
          FALSE,
          &found,
          &found_isem,
          &found_lem,
          &spoof_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  sem_entry.sem_result_ndx = spoof_id;

  if (found)
  {
    if (src_bind_info->smac_valid == 0) 
    {
        /* Static DB, remove TCAM configuration */
        if(SOC_IS_JERICHO(unit)){
            res = arad_pp_src_bind_ipv6_static_tcam_remove_dbal(
                       unit,
                       src_bind_info
                       );
        }
        else{
            res = arad_pp_src_bind_ipv6_static_tcam_remove(
                       unit,
                       src_bind_info
                       );
        }
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    } 
    else
    {
      /* DHCP DB, remove TCAM and LEM configuration */
      res = arad_pp_l3_src_bind_key_to_sem_key_unsafe(
              unit,
              NULL,
              src_bind_info,
              &sem_key);
      SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);
      
      res = arad_pp_src_bind_ipv6_lem_remove(
             unit,
             src_bind_info,
             sem_entry.sem_result_ndx,
             &ack);
      SOC_SAND_CHECK_FUNC_RESULT(res, 31, exit);
      if (ack.is_success == FALSE)
      {
        *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
      }

      res = arad_pp_sw_db_src_bind_spoof_id_ref_count_set(unit, sem_entry.sem_result_ndx, FALSE);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
      res = sw_state_access[unit].dpp.soc.arad.pp.arad_sw_db_src_binds.spoof_id_ref_count.get(unit, sem_entry.sem_result_ndx, &ref_count);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 33, exit);

      if (!ref_count) {
        res = arad_pp_isem_access_entry_remove_unsafe(
                unit,
                &sem_key
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
        res = sw_state_access[unit].dpp.soc.arad.pp.arad_sw_db_src_binds.srcbind_arb_occ.get(unit, &srcbind_arb_occ);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
        res = soc_sand_occ_bm_occup_status_set(
                     unit,
                     srcbind_arb_occ,
                     sem_entry.sem_result_ndx,
                     FALSE
                   );
        SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
      }
    }
  }

  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_remove_internal_unsafe()", 0, 0);
}


uint32
  arad_pp_src_bind_ipv6_table_clear_verify(
    SOC_SAND_IN  int                                 unit
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_TABLE_CLEAR_VERIFY);

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_table_clear_verify()", 0, 0);
}


uint32
  arad_pp_src_bind_ipv6_table_clear_unsafe(
      SOC_SAND_IN  int                                 unit
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_TABLE_CLEAR_UNSAFE);

  res = arad_pp_src_bind_ipv6_table_clear_internal_unsafe(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_table_clear_unsafe()", 0, 0);

}

uint32 
  arad_pp_src_bind_ipv6_table_clear_internal_unsafe(
      SOC_SAND_IN  int                                 unit
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    key;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_SRC_BIND_IPV6_TABLE_CLEAR_INTERNAL_UNSAFE);

  /* STATIC DB, clear TCAM */
  sal_memset(&key, 0x0, sizeof(key));
  key.type = ARAD_IP_TCAM_ENTRY_TYPE_IPV6_STATIC_SRC_BIND;

  res = arad_pp_frwrd_ip_tcam_routing_table_clear_unsafe(
          unit,
          &key,
          FALSE
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* DHCP DB, clear TCAM and LEM  */


  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_clear_internal_unsafe()", 0, 0);
}

STATIC
  uint32
    arad_pp_ip6_compression_tcam_callback(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32  user_data
    )
{
  uint32
    res = SOC_SAND_OK,
    tcam_db_id,
    access_profile_id;
  uint8 prog_id = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Set access_profile id depending on TCAM DB ID */
  tcam_db_id = user_data;

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(
          unit,
          tcam_db_id,
          0, /* No 320b DB */
          &access_profile_id
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = arad_pp_flp_app_to_prog_index_get(unit,
                                          PROG_FLP_ETHERNET_PON_DEFAULT_DOWNSTREAM,
                                          &prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  switch (user_data) /* tcam DB ID */
  {

  case ARAD_PP_TCAM_STATIC_ACCESS_ID_SIP:
    res = arad_pp_flp_lookups_ethernet_pon_default_downstream(unit, access_profile_id, TRUE,FALSE, prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    break;
  default:
    break;
  }

  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_tcam_callback()", 0, 0);
}

uint32
  arad_pp_ip6_compression_tcam_init_unsafe(
    SOC_SAND_IN int unit
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_SUCCESS_FAILURE
    success;
  ARAD_TCAM_ACCESS_INFO
    tcam_access_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_ARAD_TCAM_ACCESS_INFO_clear(&tcam_access_info);

  tcam_access_info.action_bitmap_ndx                   = ARAD_TCAM_ACTION_SIZE_FIRST_20_BITS;
  tcam_access_info.bank_owner                          = ARAD_TCAM_BANK_OWNER_FLP_TCAM;    
  tcam_access_info.callback                            = arad_pp_ip6_compression_tcam_callback;
  tcam_access_info.entry_size                          = ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS;
  tcam_access_info.is_direct                           = FALSE;
  tcam_access_info.min_banks                           = 0;
  tcam_access_info.prefix_size                         = 4;
  tcam_access_info.user_data                           = ARAD_PP_TCAM_STATIC_ACCESS_ID_SIP;

  res = arad_tcam_access_create_unsafe(
          unit,
          ARAD_PP_TCAM_STATIC_ACCESS_ID_SIP,
          &tcam_access_info,
          &success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);      

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_init_unsafe()", 0, 0);
}

STATIC
  uint32
    arad_pp_ip6_compression_tcam_add(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_IPV6_COMPRESSED_ENTRY          *ip6_compression,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_TCAM_ACTION
    action;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    key;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(success);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  ARAD_TCAM_ACTION_clear(&action);

  action.value[0] = ip6_compression->ip6_compressed_result;

  key.type = ARAD_IP_TCAM_ENTRY_TYPE_SIPV6_COMPRESSION;
  sal_memcpy(&(key.key.ip6_compression), ip6_compression, sizeof(SOC_PPC_IPV6_COMPRESSED_ENTRY));
  key.vrf_ndx = 0;

  res = arad_pp_frwrd_ip_tcam_route_add_unsafe(
		unit,
		&key,
		&action,
		success
	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10,exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_tcam_add()", 0, 0);
}

uint32
  arad_pp_ip6_compression_tcam_delete(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_IPV6_COMPRESSED_ENTRY          *ip6_compression
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    key;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  key.type = ARAD_IP_TCAM_ENTRY_TYPE_SIPV6_COMPRESSION;
  sal_memcpy(&(key.key.ip6_compression), ip6_compression, sizeof(SOC_PPC_IPV6_COMPRESSED_ENTRY));
  key.vrf_ndx = 0;

  res = arad_pp_frwrd_ip_tcam_route_remove_unsafe(
  		unit,
  		&key
  	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_tcam_delete()", 0, 0);
}

STATIC
  uint32
    arad_pp_ip6_compression_tcam_get(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_INOUT  SOC_PPC_IPV6_COMPRESSED_ENTRY   *ip6_compression,
      SOC_SAND_OUT uint8                           *found
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IP_TCAM_ENTRY_KEY
    key;
  ARAD_TCAM_ACTION
    action;
  uint8
    hit_bit = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(found);
  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  ARAD_TCAM_ACTION_clear(&action);

  key.type = ARAD_IP_TCAM_ENTRY_TYPE_SIPV6_COMPRESSION;
  sal_memcpy(&(key.key.ip6_compression), ip6_compression, sizeof(SOC_PPC_IPV6_COMPRESSED_ENTRY));
  key.vrf_ndx = 0;

  res = arad_pp_frwrd_ip_tcam_route_get_unsafe(
  		unit,
  		&key,
  		TRUE,
  		&action,
  		found,
  		&hit_bit
  	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (*found) {
    ip6_compression->ip6_compressed_result = action.value[0];
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_src_bind_ipv6_static_tcam_get()", 0, 0);
}

STATIC void
  arad_pp_ip6_compression_key_to_sem_key (
    SOC_SAND_IN  int                                          unit,
    SOC_SAND_IN  SOC_PPC_IPV6_COMPRESSED_ENTRY               *ip6_compression,
    SOC_SAND_OUT ARAD_PP_ISEM_ACCESS_KEY                     *sem_key
  )
{
    if (ip6_compression->is_spoof) {
      sem_key->key_type = ARAD_PP_ISEM_ACCESS_KEY_TYPE_SPOOF_SIP6_TCAM_COMPRESSION;
      sem_key->key_info.spoof_v6.sip6 = ip6_compression->ip6;
      sem_key->key_info.spoof_v6.vsi = ip6_compression->vsi_id;
    } else {
      sem_key->key_type = ARAD_PP_ISEM_ACCESS_KEY_TYPE_DIP6_TCAM_COMPRESSION;
      sem_key->key_info.ip6_compression.ip6 = ip6_compression->ip6;
    }
}

STATIC uint32
    arad_pp_ip6_compression_lem_request_key_build(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_IPV6_COMPRESSED_ENTRY      *ip6_compression,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY             *key
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_PP_LEM_ACCESS_KEY_clear(key);

  key->type = (ip6_compression->is_spoof ? ARAD_PP_LEM_ACCESS_KEY_TYPE_IP6_SPOOF_STATIC : ARAD_PP_LEM_ACCESS_KEY_TYPE_IP6_COMPRESSION_DIP);
  if (ip6_compression->is_spoof) {
    /* IP6 SAV compression:
     *   compressed_127_64[8:0], DIP[63:0] -> expected LIF
     *   1LSB should be 1 to indicate it's SAV IP6 prefix or SAV IP6 host
     */
    key->nof_params = 2;
    key->param[0].nof_bits = 64;
    res = soc_sand_bitstream_get_any_field(ip6_compression->ip6.ipv6_address.address, 0, 64, key->param[0].value);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    key->param[1].nof_bits = 9;
    key->param[1].value[0] = ip6_compression->ip6_tt_compressed_result; 
    
    key->prefix.nof_bits = 0x1;
    key->prefix.value = 0x1;
  } else {
    /* DIP6 compression:
     *   compressed_127_56[11:0], DIP[55:0] -> compressed_DIP_key_to_pmf[11:0]
     */
    key->nof_params = 2;
    key->param[0].nof_bits = 56;
    res = soc_sand_bitstream_get_any_field(ip6_compression->ip6.ipv6_address.address, 0, 56, key->param[0].value);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    key->param[1].nof_bits = 12;
    key->param[1].value[0] = ip6_compression->ip6_tt_compressed_result; 

    key->prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_DIP6_COMPRESSION;
    key->prefix.value = ARAD_PP_LEM_ACCESS_KEY_PREFIX_DIP6_COMPRESSION;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_lem_request_key_build()", 0, 0);
}


STATIC uint32
    arad_pp_ip6_compression_lem_add(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  int                                     is_dip,
      SOC_SAND_IN  SOC_PPC_IPV6_COMPRESSED_ENTRY          *ip6_compression,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;
  SOC_PPC_FRWRD_DECISION_INFO   
    frwrd_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(success);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);
  SOC_PPC_FRWRD_DECISION_INFO_clear(&frwrd_info);

  request.command = ARAD_PP_LEM_ACCESS_CMD_INSERT;
  res = arad_pp_ip6_compression_lem_request_key_build(
          unit,
          ip6_compression,
          &request.key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  payload.flags = ARAD_PP_FWD_DECISION_PARSE_DEST;
  payload.is_dynamic = 0;
  ARAD_PP_FRWRD_DECISION_FEC_SET(unit, &frwrd_info, ip6_compression->ip6_compressed_result);
  arad_pp_src_bind_lem_payload_build(unit, &frwrd_info, &payload);

  res = arad_pp_lem_access_entry_add_unsafe(
          unit,
          &request,
          &payload,
          &ack);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
 
  if(ack.is_success)
  {
    *success = SOC_SAND_SUCCESS;
  }
  else
  {
    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
  }
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_ip6_compression_lem_add()",0,0);
}

STATIC uint32
    arad_pp_ip6_compression_lem_get(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_OUT SOC_PPC_IPV6_COMPRESSED_ENTRY          *ip6_compression,
      SOC_SAND_OUT uint8                                  *found
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;
  SOC_SAND_PP_DESTINATION_ID dest_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(found);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);

  res = arad_pp_ip6_compression_lem_request_key_build(
          unit,
          ip6_compression,
          &request.key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_lem_access_entry_by_key_get_unsafe(
          unit,
          &(request.key),
          &payload,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (*found) {
    res = arad_pp_frwrd_ipv4_em_dest_to_sand_dest(unit, payload.dest, &dest_id);
    if (dest_id.dest_type == SOC_SAND_PP_DEST_FEC) {
      ip6_compression->ip6_compressed_result = dest_id.dest_val;
    }
  }

  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_ip6_compression_lem_get()",0,0);
}

STATIC uint32
    arad_pp_ip6_compression_lem_remove(
      SOC_SAND_IN  int                                     unit,
      SOC_SAND_IN  SOC_PPC_IPV6_COMPRESSED_ENTRY          *ip6_compression,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS          *ack_status
    )
{
  uint32
    res;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(ack_status);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);

  request.command = ARAD_PP_LEM_ACCESS_CMD_DELETE;
  res = arad_pp_ip6_compression_lem_request_key_build(
          unit,
          ip6_compression,
          &request.key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_lem_access_entry_remove_unsafe(
          unit,
          &request,
          ack_status
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (!((ack_status->is_success == TRUE) || (ack_status->reason == ARAD_PP_LEM_ACCESS_FAIL_REASON_DELETE_UNKNOWN)))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LEM_ACCESS_FAIL_REASON_OUT_OF_RANGE_ERR, 50, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_ip6_compression_lem_remove()",0,0);
}


STATIC uint32 
  arad_pp_ip6_compression_get_internal(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_INOUT  SOC_PPC_IPV6_COMPRESSED_ENTRY   *ip6_compression,
      SOC_SAND_OUT uint8                              *tcam_found,
      SOC_SAND_OUT uint8                              *lem_found,
      SOC_SAND_OUT uint8                              *found
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(found);

  /* TCAM */
  ARAD_PP_CLEAR(&sem_key, ARAD_PP_ISEM_ACCESS_KEY, 1);
  sem_entry.sem_result_ndx = 0;
  arad_pp_ip6_compression_key_to_sem_key(unit, ip6_compression, &sem_key);
  res = arad_pp_isem_access_entry_get_unsafe(
          unit,
          &sem_key,
          &sem_entry,
          tcam_found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  if (*tcam_found) {
    if (ARAD_PP_SRC_BIND_IP6_COMPRESSION_PREFIX(ip6_compression->is_spoof, ip6_compression->ip6_tt_compressed_result)) {
      /* SAV prefix: 1LSB is 0 */
      *found = *tcam_found;
    } else {
      ip6_compression->ip6_tt_compressed_result = sem_entry.sem_result_ndx;
      /* LEM, the output is ip6_compression->ip6_compressed_result */
       res = arad_pp_ip6_compression_lem_get(
               unit,
               ip6_compression,
               lem_found);
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit); 
      *found = *tcam_found && *lem_found;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_get_internal()", 0, 0);
}

soc_error_t 
  arad_pp_ip6_compression_add(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_INOUT  SOC_PPC_IPV6_COMPRESSED_ENTRY   *ip6_compression,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE           *success
    )
{
  uint32
    res = SOC_SAND_OK;
  uint8 sip_compression = 0;
  uint8 tcam_found, lem_found, is_dip, found;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_ISEM_ACCESS_ENTRY
    sem_entry;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(success);

  tcam_found = lem_found = is_dip = found = 0;
  *success = SOC_SAND_SUCCESS;
  sip_compression = ip6_compression->flags & SOC_PPC_IP6_COMPRESSION_SIP; 
  if (sip_compression) {
    res = arad_pp_ip6_compression_tcam_get(unit, ip6_compression, &found);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (!found) {
      res = arad_pp_ip6_compression_tcam_add(unit, ip6_compression, success);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    } else {
      *success = SOC_SAND_SUCCESS;
    }
  } else {
    res = arad_pp_ip6_compression_get_internal(
            unit,
            ip6_compression,
            &tcam_found,
            &lem_found,
            &found
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    if (!found) {
      /* TCAM */
      if (!tcam_found) {
        ARAD_PP_CLEAR(&sem_key, ARAD_PP_ISEM_ACCESS_KEY, 1);
        sem_entry.sem_result_ndx = ip6_compression->ip6_tt_compressed_result;
        arad_pp_ip6_compression_key_to_sem_key(unit, ip6_compression, &sem_key);
        res = arad_pp_isem_access_entry_add_unsafe(
                unit,
                &sem_key,
                &sem_entry,
                success
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
      }
      /* LEM */
      if (!lem_found && (*success == SOC_SAND_SUCCESS) &&
          !ARAD_PP_SRC_BIND_IP6_COMPRESSION_PREFIX(ip6_compression->is_spoof, ip6_compression->ip6_tt_compressed_result)) {
        res = arad_pp_ip6_compression_lem_add(unit, is_dip, ip6_compression, success);
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      }
    } else {
      *success = SOC_SAND_SUCCESS;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_add()", 0, 0);
}

soc_error_t
  arad_pp_ip6_compression_delete(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_INOUT  SOC_PPC_IPV6_COMPRESSED_ENTRY   *ip6_compression,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE           *success
    )
{
  uint32
    res = SOC_SAND_OK;
  uint8 sip_compression = 0;
  uint8 tcam_found, lem_found, found;
  ARAD_PP_ISEM_ACCESS_KEY
    sem_key;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack_status;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(success);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack_status);

  tcam_found = lem_found = found = 0;
  *success = SOC_SAND_SUCCESS;
  sip_compression = ip6_compression->flags & SOC_PPC_IP6_COMPRESSION_SIP; 
  if (sip_compression) {
    res = arad_pp_ip6_compression_tcam_get(unit, ip6_compression, &found);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (found) {
      res = arad_pp_ip6_compression_tcam_delete(unit, ip6_compression);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
  } else {
    res = arad_pp_ip6_compression_get_internal(
            unit,
            ip6_compression,
            &tcam_found,
            &lem_found,
            &found
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    /* TCAM */
    if (tcam_found && 
       (ip6_compression->flags & SOC_PPC_IP6_COMPRESSION_TCAM)) {
      ARAD_PP_CLEAR(&sem_key, ARAD_PP_ISEM_ACCESS_KEY, 1);
      arad_pp_ip6_compression_key_to_sem_key(unit, ip6_compression, &sem_key);
      res = arad_pp_isem_access_entry_remove_unsafe(
              unit,
              &sem_key
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    }

    if (lem_found && 
        !ARAD_PP_SRC_BIND_IP6_COMPRESSION_PREFIX(ip6_compression->is_spoof, ip6_compression->ip6_tt_compressed_result) &&
        (ip6_compression->flags & SOC_PPC_IP6_COMPRESSION_LEM)) {
      res = arad_pp_ip6_compression_lem_remove(unit, ip6_compression, &ack_status);
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      if (ack_status.is_success == FALSE)
      {
        *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
      }
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_delete()", 0, 0);
}


soc_error_t 
  arad_pp_ip6_compression_get(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_INOUT  SOC_PPC_IPV6_COMPRESSED_ENTRY   *ip6_compression,
      SOC_SAND_OUT uint8                              *found
    )
{
  uint32
    res = SOC_SAND_OK;
  uint8 sip_compression = 0;
  uint8 tcam_found, lem_found, is_dip;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(ip6_compression);
  SOC_SAND_CHECK_NULL_INPUT(found);
  tcam_found = lem_found = is_dip= 0;

  sip_compression = ip6_compression->flags & SOC_PPC_IP6_COMPRESSION_SIP;
  if (sip_compression) {
    res = arad_pp_ip6_compression_tcam_get(unit, ip6_compression, found);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  } else {
    res = arad_pp_ip6_compression_get_internal(
            unit,
            ip6_compression,
            &tcam_found,
            &lem_found,
            found
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip6_compression_get()", 0, 0);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_lif module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_src_bind_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_src_bind;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_lif module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_src_bind_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_src_bind;
}

uint32
  SOC_PPC_SRC_BIND_IPV4_ENTRY_verify(
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV4_ENTRY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  if ((info->smac_valid != 0) && SOC_SAND_PP_MAC_ADDRESS_IS_ZERO(info->smac)) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_SRC_BIND_INVALID_SMAC_ERR, 10, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_SRC_BIND_IPV4_ENTRY_verify()",0,0);
}

uint32
  SOC_PPC_SRC_BIND_IPV6_ENTRY_verify(
    SOC_SAND_IN  SOC_PPC_SRC_BIND_IPV6_ENTRY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  if ((info->smac_valid != 0) && SOC_SAND_PP_MAC_ADDRESS_IS_ZERO(info->smac)) {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_SRC_BIND_INVALID_SMAC_ERR, 10, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_SRC_BIND_IPV4_ENTRY_verify()",0,0);
}


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */


