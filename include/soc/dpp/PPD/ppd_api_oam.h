
/* $Id: ppd_api_oam.h,v 1.41 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_PPD_API_OAM_INCLUDED__
/* { */
#define __SOC_PPD_API_OAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_oam.h>

#include <soc/dpp/PPD/ppd_api_lif.h>
#include <soc/dpp/PPD/ppd_api_eg_ac.h>
#include <soc/dpp/drv.h>

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

#define SOC_PPD_OAM_IS_CLASSIFIER_ADVANCED_MULTIPLE_MEPS_ON_LIF_MODE(unit)   (SOC_DPP_CONFIG(unit)->pp.oam_classifier_advanced_mode==2)
#define SOC_PPD_OAM_IS_CLASSIFIER_ADVANCED_MODE(unit)                        ((SOC_DPP_CONFIG(unit)->pp.oam_classifier_advanced_mode==1) || (SOC_DPP_CONFIG(unit)->pp.oam_classifier_advanced_mode==2))
#define SOC_PPD_OAM_IS_CLASSIFIER_ARAD_MODE(unit)                            (!SOC_PPD_OAM_IS_CLASSIFIER_ADVANCED_MODE(unit) && SOC_IS_ARADPLUS_AND_BELOW(unit))
#define SOC_PPD_OAM_IS_CLASSIFIER_JERICHO_MODE(unit)                         (SOC_IS_JERICHO(unit))
#define SOC_PPD_OAM_IS_CLASSIFIER_JERICHO_PLUS_MODE(unit)                    (SOC_IS_JERICHO_PLUS(unit))

#define SOC_PPD_OAM_IS_Y1711_LM(unit,endpoint_info)  ((SOC_IS_QAX(unit))&&(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "oam_1711_enable",0) == 1)&&(endpoint_info->type == bcmOAMEndpointTypeMPLSNetwork))

#define SOC_PPD_OAM_SERVER_CLIENT_SIDE_BIT 24

#define SOC_PPD_OAM_MEP_INDEX_IS_SERVER_CLIENT_SIDE(endpoint) \
	(((endpoint >> SOC_PPD_OAM_SERVER_CLIENT_SIDE_BIT) & 1) == 1)

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
  * Auto generated. Do not edit following section {
  */
  /*
  * } Auto generated. Do not edit previous section.
  */

  SOC_PPD_OAM_ETH_ING_AC_KEY_MP_INFO_SET = SOC_PPD_PROC_DESC_BASE_OAM_FIRST,
  SOC_PPD_OAM_ETH_ING_AC_KEY_MP_INFO_GET,
  SOC_PPD_OAM_ETH_EGR_PORT_VSI_MP_INFO_SET,
  SOC_PPD_OAM_ETH_EGR_PORT_VSI_MP_INFO_GET,
  SOC_PPD_OAM_ETH_EGR_PORT_CVID_MP_INFO_SET,
  SOC_PPD_OAM_ETH_EGR_PORT_CVID_MP_INFO_GET,
  SOC_PPD_OAM_ETH_EGR_OUT_AC_MP_INFO_SET,
  SOC_PPD_OAM_ETH_EGR_OUT_AC_MP_INFO_GET,
  SOC_PPD_OAM_ETH_TRAP_CODE_SET,
  SOC_PPD_OAM_ETH_TRAP_CODE_GET,
  /* ARAD OAM APIs*/
  SOC_PPD_OAM_INIT,
  SOC_PPD_OAM_DEINIT,
  SOC_PPD_OAM_ICC_MAP_REGISTER_SET,
  SOC_PPD_OAM_CLASSIFIER_MEP_SET,
  SOC_PPD_OAM_CLASSIFIER_MEP_DELETE,
  SOC_PPD_OAMP_RMEP_SET,
  SOC_PPD_OAMP_RMEP_GET,
  SOC_PPD_OAMP_RMEP_DELETE,
  SOC_PPD_OAMP_RMEP_INDEX_GET,
  SOC_PPD_OAMP_MEP_DB_ENTRY_SET,
  SOC_PPD_OAMP_MEP_DB_ENTRY_GET,
  SOC_PPD_OAMP_MEP_DB_ENTRY_DELETE,
  SOC_PPD_OAM_CLASSIFIER_OEM_MEP_ADD,
  SOC_PPD_CLASSIFIER_FIND_MEP_BY_LIF_AND_MD_LEVEL,
  SOC_PPD_CLASSIFIER_OEM1_ENTRY_SET,
  SOC_PPD_CLASSIFIER_OEM1_ENTRY_GET,
  SOC_PPD_CLASSIFIER_OEM1_ENTRY_DELETE,
  SOC_PPD_CLASSIFIER_OEM2_ENTRY_SET,
  SOC_PPD_CLASSIFIER_OEM2_ENTRY_GET,
  SOC_PPD_CLASSIFIER_OEM2_ENTRY_DELETE,
  SOC_PPD_OAM_CLASSIFIER_OAM1_ENTRIES_INSERT_DEFAULT_PROFILE,
  SOC_PPD_OAM_CLASSIFIER_OAM1_2_ENTRIES_INSERT_ACCORDING_TO_PROFILE,
  SOC_PPD_OAM_CLASSIFIER_OEM_MEP_PROFILE_REPLACE,
  SOC_PPD_OAM_COUNTER_RANGE_SET,
  SOC_PPD_OAM_COUNTER_RANGE_GET,
  SOC_PPD_OAM_BFD_IPV4_TOS_TTL_SELECT_SET,
  SOC_PPD_OAM_BFD_IPV4_TOS_TTL_SELECT_GET,
  SOC_PPD_OAM_BFD_IPV4_SRC_ADDR_SELECT_SET,
  SOC_PPD_OAM_BFD_IPV4_SRC_ADDR_SELECT_GET,
  SOC_PPD_OAM_BFD_TX_RATE_SET,
  SOC_PPD_OAM_BFD_TX_RATE_GET,
  SOC_PPD_OAM_BFD_REQ_INTERVAL_POINTER_SET,
  SOC_PPD_OAM_BFD_REQ_INTERVAL_POINTER_GET,
  SOC_PPD_OAM_MPLS_PWE_PROFILE_SET,
  SOC_PPD_OAM_MPLS_PWE_PROFILE_GET,
  SOC_PPD_OAM_BFD_MPLS_UDP_SPORT_SET,
  SOC_PPD_OAM_BFD_MPLS_UDP_SPORT_GET,
  SOC_PPD_OAM_BFD_IPV4_UDP_SPORT_SET,
  SOC_PPD_OAM_BFD_IPV4_UDP_SPORT_GET,
  SOC_PPD_OAM_BFD_PDU_STATIC_REGISTER_SET,
  SOC_PPD_OAM_BFD_PDU_STATIC_REGISTER_GET,
  SOC_PPD_OAM_BFD_DISCRIMINATOR_RANGE_REGISTERS_SET,
  SOC_PPD_OAM_BFD_DISCRIMINATOR_RANGE_REGISTERS_GET,
  SOC_PPD_OAM_BFD_MY_BFD_DIP_IPV4_SET,
  SOC_PPD_OAM_BFD_MY_BFD_DIP_IPV4_GET,
  SOC_PPD_OAM_BFD_TX_IPV4_MULTI_HOP_SET,
  SOC_PPD_OAM_BFD_TX_IPV4_MULTI_HOP_GET,
  SOC_PPD_OAM_BFD_TX_MPLS_SET,
  SOC_PPD_OAM_BFD_TX_MPLS_GET,
  SOC_PPD_OAM_BFDCC_TX_MPLS_SET,
  SOC_PPD_OAM_BFDCC_TX_MPLS_GET,
  SOC_PPD_OAM_BFDCV_TX_MPLS_SET,
  SOC_PPD_OAM_BFDCV_TX_MPLS_GET,
  SOC_PPD_OAM_OAMP_TX_PRIORITY_REGISTERS_SET,
  SOC_PPD_OAM_OAMP_TX_PRIORITY_REGISTERS_GET,
  SOC_PPD_OAM_OAMP_ENABLE_INTERRUPT_MESSAGE_EVENT_SET,
  SOC_PPD_OAM_OAMP_ENABLE_INTERRUPT_MESSAGE_EVENT_GET,
  SOC_PPD_OAM_EVENT_FIFO_READ,
  SOC_PPD_OAM_PP_PCT_PROFILE_SET,
  SOC_PPD_OAM_PP_PCT_PROFILE_GET,
  /*
  * Last element. Do no touch.
  */
  SOC_PPD_OAM_PROCEDURE_DESC_LAST
} SOC_PPD_OAM_PROCEDURE_DESC;

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_ing_ac_key_mp_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an incoming Attachment-Circuit (port x VLAN x VLAN)
 *   and MD-level, and to determine the action to perform
 *   (see Table 8: Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the
 *   MP is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY           *ac_key -
 *     Identifier of the incoming attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   Should only be used if the MP is down MEP or MIP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_ing_ac_key_mp_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY           *ac_key,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_ing_ac_key_mp_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an incoming Attachment-Circuit (port x VLAN x VLAN)
 *   and MD-level, and to determine the action to perform
 *   (see Table 8: Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the
 *   MP is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY           *ac_key -
 *     Identifier of the incoming attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   Should only be used if the MP is down MEP or MIP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_ing_ac_key_mp_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY           *ac_key,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_egr_port_vsi_mp_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an outgoing Attachment-Circuit (port x VSI) and
 *   MD-level, and to determine the action to perform (see
 *   Table 8: Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the MP
 *   is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY           *port_vsi_key -
 *     Identifier of the outgoing attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   Should only be used if the MP is up MEP or MIP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_egr_port_vsi_mp_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY           *port_vsi_key,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_egr_port_vsi_mp_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an outgoing Attachment-Circuit (port x VSI) and
 *   MD-level, and to determine the action to perform (see
 *   Table 8: Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the MP
 *   is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY           *port_vsi_key -
 *     Identifier of the outgoing attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   Should only be used if the MP is up MEP or MIP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_egr_port_vsi_mp_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY           *port_vsi_key,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_egr_port_cvid_mp_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an outgoing Attachment-Circuit (port x CVID) and
 *   MD-level, and to determine the action to perform (see
 *   Table 8: Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the MP
 *   is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY      *port_cep_key -
 *     Identifier of the outgoing attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_egr_port_cvid_mp_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY      *port_cep_key,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_egr_port_cvid_mp_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an outgoing Attachment-Circuit (port x CVID) and
 *   MD-level, and to determine the action to perform (see
 *   Table 8: Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the MP
 *   is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY      *port_cep_key -
 *     Identifier of the outgoing attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_egr_port_cvid_mp_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY      *port_cep_key,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_egr_out_ac_mp_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an outgoing Attachment-Circuit (Out-AC) and MD-level,
 *   and to determine the action to perform (see Table 8:
 *   Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the MP is one of
 *   the 4K accelerated MEPs, the function configures the
 *   related OAMP databases and associates the AC and
 *   MD-Level with a user-provided handle. This handle is
 *   later used by user to access OAMP database for this MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    out_ac_ndx -
 *     Identifier of the outgoing attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_egr_out_ac_mp_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                    out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_egr_out_ac_mp_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an outgoing Attachment-Circuit (Out-AC) and MD-level,
 *   and to determine the action to perform (see Table 8:
 *   Enumerator - SOC_PPC_OAM_ETH_ACC_FUNC_TYPE). If the MP is one of
 *   the 4K accelerated MEPs, the function configures the
 *   related OAMP databases and associates the AC and
 *   MD-Level with a user-provided handle. This handle is
 *   later used by user to access OAMP database for this MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    out_ac_ndx -
 *     Identifier of the outgoing attachment circuit on which
 *     the MP is set.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx -
 *     The level of the MEP. Range: 0-7
 *   SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info -
 *     MP configuration information
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_egr_out_ac_mp_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                    out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL        level_ndx,
    SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO         *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_trap_code_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function maps the CFM opcode to local opcode. The
 *   local opcode is used to construct a key to determine the
 *   packet trap code
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE   func_type_ndx -
 *     MP type. Range: 0-7
 *   SOC_SAND_IN  uint32                   opcode_ndx -
 *     Four least significant bits of the CFM opcode as defined
 *     in IEEE 802.1ag standard
 *   SOC_SAND_IN  SOC_PPC_ACTION_PROFILE          *action_profile -
 *     Action profile.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_trap_code_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE   func_type_ndx,
    SOC_SAND_IN  uint32                   opcode_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE          *action_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_trap_code_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function maps the CFM opcode to local opcode. The
 *   local opcode is used to construct a key to determine the
 *   packet trap code
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE   func_type_ndx -
 *     MP type. Range: 0-7
 *   SOC_SAND_IN  uint32                   opcode_ndx -
 *     Four least significant bits of the CFM opcode as defined
 *     in IEEE 802.1ag standard
 *   SOC_SAND_OUT SOC_PPC_ACTION_PROFILE          *action_profile -
 *     Action profile.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_trap_code_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE   func_type_ndx,
    SOC_SAND_IN  uint32                   opcode_ndx,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE          *action_profile
  );

/* } */


/*
* ARAD OAM APIs
*/

/*********************************************************************
* NAME:
 *   soc_ppd_oam_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function initializes oam registers and tables
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_INIT_TRAP_INFO init_trap_info_oam -
 *     Initial TRAP id info for OAM.
 *  SOC_SAND_IN  uint8                     is_bfd -
 *     Init OAM or BFD indication
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_init(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint8                     is_bfd
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_deinit
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function deinitializes oam registers and tables
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  SOC_SAND_IN  uint8                     is_bfd -
 *     Init OAM or BFD indication 
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_deinit(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint8                     is_bfd,
    SOC_SAND_IN  uint8                    tcam_db_destroy
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_icc_map_register_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Icc Map Register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                       icc_ndx -
 *     Index of ICC register to access.
 *   SOC_SAND_IN   SOC_PPC_OAM_ICC_MAP_DATA    * data -
 *     Data to write to register. 
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_icc_map_register_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                        icc_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_ICC_MAP_DATA     * data
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_oam1_entries_insert_default_profile
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set OAM1 table default profile entries
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_MEP_PROFILE_DATA  * profile_data -
 *     New non-accelerated profile data.
 *   SOC_SAND_IN  uint8                            is_bfd -
 *     Is BFD indication.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_oam1_entries_insert_default_profile(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_OAM_LIF_PROFILE_DATA       *profile_data,
    SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry,
    SOC_SAND_IN  uint8                            is_bfd
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_oam1_2_entries_insert_according_to_profile
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set OAM1 and OAM2 tables in the classifier
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  SOC_SAND_IN uint8                         is_server,
 *   SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY  * classifier_mep_entry -
 *     Data to set into classifier.
 *   SOC_SAND_IN  SOC_PPC_OAM_MEP_PROFILE_DATA  * profile_data -
 *     New non-accelerated profile data.
 *   SOC_SAND_IN  SOC_PPC_OAM_MEP_PROFILE_DATA  * profile_data_acc -
 *     New accelerated profile data.
 *     This pointer is NULL in case of passive profile configuration.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_oam1_2_entries_insert_according_to_profile(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN uint8                         is_server,
    SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry,
    SOC_SAND_IN  SOC_PPC_OAM_LIF_PROFILE_DATA       *profile_data,
    SOC_SAND_IN  SOC_PPC_OAM_LIF_PROFILE_DATA       *profile_data_acc
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_oem_mep_profile_replace
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Replace rellevant entries in OEM1 and OEM2
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY  * classifier_mep_entry -
 *     Data containinfg the key and payload of the mep. 
 *  SOC_SAND_IN  uint32                           update_mp_type,
 * 1 if the mp type is to be updated, 0 if the mp profile is to be updated.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_oem_mep_profile_replace(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry,
    SOC_SAND_IN  uint32                           update_mp_type
	);

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_oem_mep_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Icc Map Register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY    classifier_mep_entry -
 *     Mep entry to insert
 *	SOC_SAND_IN  uint8                    update -
 *    Flag to indicate that entry already exist (value 0f 1)
 *    Value 0f 2 indicates that the entry does not exist but a profile do and should be updated
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_oem_mep_add(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   mep_index,
    SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY  * classifier_mep_entry,
	SOC_SAND_IN  uint8                    update
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_mep_delete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Icc Map Register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   mep_index -
 *     MEP index.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_mep_delete(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   mep_index,
	SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY   *classifier_mep_entry
  );



/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_rmep_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set RMEP
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   rmep_index -
 *     RMEP index in db
 *   SOC_SAND_IN  uint16                   rmep_id -
 *     RMEP protocol id
 *   SOC_SAND_IN  uint32                   mep_index -
 *     Index of a local MEP connected with this RMEP
 *   SOC_SAND_IN  SOC_PPC_OAM_MEP_TYPE     mep_type -
 *     Type of endpoint 
 *   SOC_SAND_IN  uint32                   rmep_db_entry -
 *     RMEP db entry
 *    SOC_SAND_IN  uint8                    update -
 *      Indication whether to update the entry or insert new one
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_rmep_set(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   rmep_index,
	SOC_SAND_IN  uint16                   rmep_id,
	SOC_SAND_IN  uint32                   mep_index,
	SOC_SAND_IN  SOC_PPC_OAM_MEP_TYPE     mep_type,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY  *rmep_db_entry,
    SOC_SAND_IN  uint8                    update
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_rmep_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get RMEP
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   rmep_index -
 *     RMEP index in db
  *   SOC_SAND_OUT  SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY rmep_db_entry -
 *     RMEP db entry
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_rmep_get(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   rmep_index,
    SOC_SAND_OUT  SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY  *rmep_db_entry
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_rmep_delete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Delete RMEP
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   rmep_index -
 *     RMEP index in db
 *   SOC_SAND_IN  uint16                   rmep_id -
 *     RMEP protocol id
 *   SOC_SAND_IN  uint32                   mep_index -
 *     Index of a local MEP connected with this RMEP
 *   SOC_SAND_IN  SOC_PPC_OAM_MEP_TYPE     mep_type -
 *     Type of endpoint 
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_rmep_delete(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   rmep_index,
	SOC_SAND_IN  uint16                   rmep_id,
	SOC_SAND_IN  uint32                   mep_index,
	SOC_SAND_IN  SOC_PPC_OAM_MEP_TYPE     mep_type
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_rmep_index_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get RMEP index from Exact match
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint16                   rmep_id -
 *     RMEP protocol id
 *   SOC_SAND_IN  uint32                   mep_index -
 *     Index of a local MEP connected with this RMEP
 *   SOC_SAND_IN  SOC_PPC_OAM_MEP_TYPE     mep_type -
 *     Type of endpoint
 *   SOC_SAND_OUT  uint32                  *rmep_index -
 *     RMEP index in db found in EM
 *   SOC_SAND_OUT  uint8                   *is_found -
 *     was match in EM
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_rmep_index_get(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint16                   rmep_id,
	SOC_SAND_IN  uint32                   mep_index,
	SOC_SAND_IN  SOC_PPC_OAM_MEP_TYPE     mep_type,
	SOC_SAND_OUT uint32                   *rmep_index,
	SOC_SAND_OUT  uint8                   *is_found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_mep_db_entry_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set MEP
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   mep_index -
 *     RMEP index in db
  *   SOC_SAND_IN  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY                   mep_db_entry -
 *     MEP db entry
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_mep_db_entry_set(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   mep_index,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY  *mep_db_entry,
	SOC_SAND_IN  uint8                    allocate_icc_ndx,
	SOC_SAND_IN  SOC_PPC_OAM_MA_NAME      name
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_mep_db_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get MEP
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   mep_index -
 *     RMEP index in db
  *   SOC_SAND_OUT SOC_PPC_OAM_OAMP_MEP_DB_ENTRY  mep_db_entry -
 *     MEP db entry
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_mep_db_entry_get(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   mep_index,
    SOC_SAND_OUT  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY  *mep_db_entry
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_mep_db_entry_delete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set MEP
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   mep_index -
 *     MEP index in db
 *   SOC_SAND_IN  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY *mep_db_entry -
 *     pre-retrieved mep db entry data. Can be NULL.
 *   SOC_SAND_IN  uint8                    deallocate_icc_ndx -
 *     Set if icc index was deallocated
 *   SOC_SAND_IN  uint8                    is_last_mep -
 *     Set if removing the last mep
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_mep_db_entry_delete(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   mep_index,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY *mep_db_entry,
	SOC_SAND_IN  uint8                    deallocate_icc_ndx,
	SOC_SAND_IN  uint8                    is_last_mep
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_find_mep_and_profile_by_lif_and_mdlevel
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Finding if mep with given lif&mdlevel exists
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   lif -
 *     Lif to search by
 *   SOC_SAND_IN  uint8                    md_level -
 *     Level to search by
 *   SOC_SAND_IN uint8                    is_upmep -
 *     Direction to search by
 *   SOC_SAND_OUT uint8                    *found -
 *     Indication to whether a MEP was found on the given MD-Level, LIF, direction
 * 	SOC_SAND_OUT uint32                   *profile -
 *     MP-Profile existing on this lif
 *	SOC_SAND_OUT uint8                    *found_profile -
 *     Indication if lif found in OEM1.
 *     In Jericho this will be:
 1 --> An active MEP/MIP is defined on the OEM entry
 2 --> Only passive MEP on the OEM entry
 0 --> OEM entry previously undefined.
 *  SOC_SAND_OUT uint8                    *is_mp_type_flexible -
 *     Indication if the mp_type can be changed,
 *      (there are no constrains given by existing meps and mips)
 *     Arad+ only
 *  SOC_SAND_OUT  uint8                    *is_mip -
 *     Indication if the found endpoint is mip
 *     ARAD+ use only
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_find_mep_and_profile_by_lif_and_mdlevel(
    SOC_SAND_IN  int                   unit,
	SOC_SAND_IN  uint32                   lif,
	SOC_SAND_IN  uint8                    md_level,
	SOC_SAND_IN  uint8                    is_upmep,
	SOC_SAND_OUT uint8                    *found_mep, /*In Jericho may be 0,1 or2. See above*/
	SOC_SAND_OUT uint32                   *profile,
	SOC_SAND_OUT uint8                    *found_profile,
	SOC_SAND_OUT uint8                    *is_mp_type_flexible,
    SOC_SAND_OUT uint8                    *is_mip
  );

uint32
  soc_ppd_oam_classifier_oem1_entry_set(
    SOC_SAND_IN   int                                     unit,
	SOC_SAND_IN   SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY      *oem1_key,
	SOC_SAND_IN   SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD  *oem1_payload
  );

uint32
  soc_ppd_oam_classifier_oem1_entry_get(
    SOC_SAND_IN   int                                     unit,
	SOC_SAND_IN   SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY      *oem1_key,
	SOC_SAND_OUT  SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD  *oem1_payload,
	SOC_SAND_OUT  uint8                                      *is_found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_oem1_entry_delete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Delete OEM1 entry
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY      *oem1_key -
 *     Key struct
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_oem1_entry_delete(
    SOC_SAND_IN   int                                     unit,
	SOC_SAND_IN   SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY      *oem1_key
  );

uint32
  soc_ppd_oam_classifier_oem2_entry_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_KEY       *oem2_key,
	SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_PAYLOAD   *oem2_payload
  );

uint32
  soc_ppd_oam_classifier_oem2_entry_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_KEY       *oem2_key,
	SOC_SAND_OUT  SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_PAYLOAD  *oem2_payload,
	SOC_SAND_OUT  uint8                                      *is_found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_oem2_entry_delete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Delete OEM2 entry
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY      *oem1_key -
 *     Key struct
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_oem2_entry_delete(
    SOC_SAND_IN   int                                     unit,
	SOC_SAND_IN   SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_KEY      *oem2_key
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_counter_range_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set OAM counter range
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   counter_range_min -
 *     Min counter range
 *   SOC_SAND_IN  uint32                   counter_range_max -
 *     Max counter range
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_counter_range_set(
    SOC_SAND_IN   int                                     unit,
	SOC_SAND_IN   uint32                                     counter_range_min,
	SOC_SAND_IN   uint32                                     counter_range_max
  );

uint32
  soc_ppd_oam_counter_range_get(
    SOC_SAND_IN   int                                     unit,
	SOC_SAND_OUT  uint32                                     *counter_range_min,
	SOC_SAND_OUT  uint32                                     *counter_range_max
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_eth_oam_opcode_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure mapping of network opcode to internal opcode.
 *   The information is taken from WB variables.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_eth_oam_opcode_map_set(
    SOC_SAND_IN   int                                     unit
  );
/*
 * ARAD BFD functions
*/

/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_ipv4_tos_ttl_select_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Ipv4 Tos Ttl Select register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                   ipv4_tos_ttl_select_index -
 *     Entry index.
 *   SOC_SAND_IN  SOC_PPC_BFD_IP_MULTI_HOP_TOS_TTL_DATA      *tos_ttl_data -
 *     Data struct
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_ipv4_tos_ttl_select_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        ipv4_tos_ttl_select_index,
	SOC_SAND_IN  SOC_PPC_BFD_IP_MULTI_HOP_TOS_TTL_DATA        *tos_ttl_data
  );

uint32
  soc_ppd_oam_bfd_ipv4_tos_ttl_select_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        ipv4_tos_ttl_select_index,
	SOC_SAND_OUT SOC_PPC_BFD_IP_MULTI_HOP_TOS_TTL_DATA        *tos_ttl_data
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_ipv4_src_addr_select_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Ipv4 Src Addr Select register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                   ipv4_src_addr_select_index -
 *     Entry index.
 *   SOC_SAND_IN  uint32                  src_addr -
 *     Data - src address.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_ipv4_src_addr_select_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        ipv4_src_addr_select_index,
	SOC_SAND_IN  uint32                                       src_addr
  );

uint32
  soc_ppd_oam_bfd_ipv4_src_addr_select_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        ipv4_src_addr_select_index,
	SOC_SAND_OUT uint32                                       *src_addr
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_tx_rate_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd Tx Rate register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                   bfd_tx_rate_index -
 *     Entry index.
 *   SOC_SAND_IN  uint32                  tx_rate -
 *     TX rate data.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_tx_rate_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        bfd_tx_rate_index,
	SOC_SAND_IN  uint32                                       tx_rate
  );

uint32
  soc_ppd_oam_bfd_tx_rate_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        bfd_tx_rate_index,
	SOC_SAND_OUT uint32                                       *tx_rate
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_req_interval_pointer_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd Req Interval Pointer register (MPLS Push profile)
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                   req_interval_pointer -
 *     Entry index.
 *   SOC_SAND_IN  uint32                  req_interval -
 *     Rate data.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_req_interval_pointer_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        req_interval_pointer,
	SOC_SAND_IN  uint32                                       req_interval
  );

uint32
  soc_ppd_oam_bfd_req_interval_pointer_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        req_interval_pointer,
	SOC_SAND_OUT uint32                                       *req_interval
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_req_interval_pointer_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd Req Interval Pointer register (MPLS Push profile)
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                   push_profile -
 *     Entry index.
 *   SOC_SAND_IN  SOC_PPC_MPLS_PWE_PROFILE_DATA *push_data -
 *     Push profile data struct.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_mpls_pwe_profile_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        push_profile,
	SOC_SAND_IN  SOC_PPC_MPLS_PWE_PROFILE_DATA            *push_data
  );

uint32
  soc_ppd_oam_mpls_pwe_profile_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        push_profile,
	SOC_SAND_OUT SOC_PPC_MPLS_PWE_PROFILE_DATA            *push_data
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_mpls_udp_sport_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd MPLS UDP Sport register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                   udp_sport -
 *     register data.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_mpls_udp_sport_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint16            							  udp_sport
  );

uint32
  soc_ppd_oam_bfd_mpls_udp_sport_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_OUT uint16                                       *udp_sport
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_ipv4_udp_sport_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd IPV4 UDP Sport register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                   udp_sport -
 *     register data.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_ipv4_udp_sport_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint16            							  udp_sport
  );

uint32
  soc_ppd_oam_bfd_ipv4_udp_sport_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_OUT  uint16                                      *udp_sport
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_pdu_static_register_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd pdu static register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BFD_PDU_STATIC_REGISTER  bfd_pdu -
 *     register data struct.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_pdu_static_register_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  SOC_PPC_BFD_PDU_STATIC_REGISTER              *bfd_pdu
  );

uint32
  soc_ppd_oam_bfd_pdu_static_register_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_OUT SOC_PPC_BFD_PDU_STATIC_REGISTER              *bfd_pdu
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_cc_packet_static_register_set / get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd CC packet static register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BFD_CC_PACKET_STATIC_REGISTER  bfd_cc_packet -
 *     register data struct.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_cc_packet_static_register_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  SOC_PPC_BFD_CC_PACKET_STATIC_REGISTER              *bfd_cc_packet
  );

uint32
  soc_ppd_oam_bfd_cc_packet_static_register_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_OUT SOC_PPC_BFD_CC_PACKET_STATIC_REGISTER              *bfd_cc_packet
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_discriminator_range_registers_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd pdu static register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint16                   range -
 *     range of your discriminator
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_discriminator_range_registers_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint32  						              range
  );

uint32
  soc_ppd_oam_bfd_discriminator_range_registers_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_OUT  uint32  						              *range
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_my_bfd_dip_ip_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set My Bfd Dip table with IPv4 values register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   dip_index -
 *     Entry index
	SOC_SAND_IN  SOC_SAND_PP_IPV6_ADDRESS                                  *dip							
 *     IPv6 address
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_my_bfd_dip_ip_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint16                                dip_index,
	SOC_SAND_IN  SOC_SAND_PP_IPV6_ADDRESS                                 *dip							
  );

uint32
  soc_ppd_oam_bfd_my_bfd_dip_ip_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint16                                     dip_index,
	SOC_SAND_OUT  SOC_SAND_PP_IPV6_ADDRESS                                  *dip							
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_tx_ipv4_multi_hop_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Bfd Tx Ipv4 Multi Hop register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES  *tx_ipv4_multi_hop_att -
 *     Register values
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_tx_ipv4_multi_hop_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES              *tx_ipv4_multi_hop_att
  );

uint32
  soc_ppd_oam_bfd_tx_ipv4_multi_hop_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_OUT SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES               *tx_ipv4_multi_hop_att
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_tx_priority_registers_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set OAMP priority TC and DP registers
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    priority -
 *     Priority profile (0-7)
 *   SOC_SAND_IN  SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES    *tx_oam_att -
 *     Register values
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_tx_priority_registers_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint32                     	              priority,
	SOC_SAND_IN  SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES              *tx_oam_att
  );

uint32
  soc_ppd_oam_oamp_tx_priority_registers_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint32                     	              priority,
	SOC_SAND_OUT  SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES             *tx_oam_att
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_enable_interrupt_message_event_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set oamp_enable_interrupt_message_event register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                    interrupt_message_event_bmp -
 *     Bitmap of the events to set
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_enable_interrupt_message_event_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        *interrupt_message_event_bmp
  );

uint32
  soc_ppd_oam_oamp_enable_interrupt_message_event_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_OUT  uint8                                       *interrupt_message_event_bmp
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_event_fifo_read
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Read oam event fifo
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT  uint32                    *rmeb_db_ndx -
 *     Rmep db index
 *   SOC_SAND_OUT  uint32                    *event_id -
 *     ID of event
 *   SOC_SAND_OUT  uint32                    *valid -
 *     Is valid
 *   SOC_SAND_OUT  uint32                    *event_data -
 *     Data convayed in the event
 * SOC_PPC_OAM_INTERRUPT_GLOBAL_DATA               *interrupt_data
 Global" data used by soc layer.
 The reason this is not used as a global variable is so that the interrupts will be reentrable.
 The data is only global per interrupt.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_event_fifo_read(
    SOC_SAND_IN   int                                           unit,
    SOC_SAND_IN   SOC_PPC_OAM_DMA_EVENT_TYPE                 event_type,
	SOC_SAND_OUT  uint32                                       *rmeb_db_ndx,
	SOC_SAND_OUT  uint32                                       *event_id,
	SOC_SAND_OUT  uint32                                       *valid,
	SOC_SAND_OUT  uint32                                       *event_data,
    SOC_PPC_OAM_INTERRUPT_GLOBAL_DATA               *interrupt_data
    );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_pp_pct_profile_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set PP_PCT oam port profile field
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT  SOC_PPC_PORT              local_port_ndx -
 *     Rort index
 *   SOC_SAND_OUT  uint32                    oam_profile -
 *     Binary profile
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_pp_pct_profile_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_IN  uint8                                  oam_profile
  );

uint32
  soc_ppd_oam_pp_pct_profile_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_OUT uint8                                  *oam_profile
  );

/* ARAD+ functions */
/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_diag_profile_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set bfd diag profile
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                    profile_ndx -
 *     Profile index
 *   SOC_SAND_IN  uint32                   diag_profile -
 *     Value to setin the enable register
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_diag_profile_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        profile_ndx,
	SOC_SAND_IN  uint32                                       diag_profile
  );

uint32
  soc_ppd_oam_bfd_diag_profile_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        profile_ndx,
	SOC_SAND_OUT  uint32                                       *diag_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_bfd_flags_profile_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set bfd flags profile
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                    profile_ndx -
 *     Profile index
 *   SOC_SAND_IN  uint32                   flags_profile -
 *     Value to setin the enable register
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_bfd_flags_profile_set(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        profile_ndx,
	SOC_SAND_IN  uint32                                       flags_profile
  );

uint32
  soc_ppd_oam_bfd_flags_profile_get(
    SOC_SAND_IN  int                                       unit,
	SOC_SAND_IN  uint8                                        profile_ndx,
	SOC_SAND_OUT  uint32                                       *flags_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_mep_passive_active_enable_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set oam_mep_passive_active_enable register - MP_type of each profile
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   profile_ndx -
 *     Profile index
 *   SOC_SAND_IN  uint8                    enable -
 *     Value to setin the enable register
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_mep_passive_active_enable_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 profile_ndx,
    SOC_SAND_IN  uint8                                  enable
  );

uint32
  soc_ppd_oam_mep_passive_active_enable_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 profile_ndx,
    SOC_SAND_OUT  uint8                                 *enable
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_punt_event_hendling_profile_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set oamp_punt_event_hendling register
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   profile_ndx -
 *     Profile index
 *   SOC_SAND_IN  SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA     *punt_profile_data -
 *     Profile fields
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_punt_event_hendling_profile_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 profile_ndx,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA     *punt_profile_data
  );

uint32
  soc_ppd_oam_oamp_punt_event_hendling_profile_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 profile_ndx,
    SOC_SAND_OUT  SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA     *punt_profile_data
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_error_trap_id_and_destination_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set PORT2CPU OAMP register fields trap id and system port with the given
 *   values according to the error type.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_OAM_OAMP_TRAP_TYPE             trap_type -
 *     Type of the error to set
 *   SOC_SAND_IN  uint32                                 trap_id -
 *     Id 0-255 to put in trap id field
 *   SOC_SAND_IN  SOC_TMC_DEST_INFO                        dest_info -
 *     TM destination information
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_error_trap_id_and_destination_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_TRAP_TYPE             trap_type,
    SOC_SAND_IN  uint32                                 trap_id,
    SOC_SAND_IN  SOC_TMC_DEST_INFO                         dest_info
  );

uint32
  soc_ppd_oam_oamp_error_trap_id_and_destination_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_TRAP_TYPE             trap_type,
    SOC_SAND_OUT  uint32                                *trap_id,
    SOC_SAND_OUT  SOC_TMC_DEST_INFO                        *dest_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_lm_dm_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Manage LM and DM on the soc layer.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  SOC_SAND_IN SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY     *eth1731_profile
 *   SOC_SAND_IN  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY                   mep_db_entry -
 *     MEP DB entry
 * REMARKS:
 *   Used for Arad+.
 *   Functions are shared by LM and DM due to the similarity in the implementation of the two functionalities.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_lm_dm_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_LM_DM_MEP_DB_ENTRY     *mep_db_entry
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_lm/dm_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Return statistics form soc layer.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT  SOC_PPD_OAM_OAMP_(L|D)M_INFO_GET     *(l|d)m_info
 * statistics go in there.
 *   SOC_SAND_OUT uint8                                      * is_1dm
 * (DM only) returns 1 if the entry is set as 1DM.
 * REMARKS:
 *   Used for Arad+.
 *   Functions are shared by LM and DM due to the similarity in the implementation of the two functionalities.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_lm_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT  SOC_PPC_OAM_OAMP_LM_INFO_GET     *lm_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_next_index_get, 
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Manage LM and DM on the soc layer.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *   SOC_SAND_IN  uint32                                 endpoint_id,
 * index of the endpoint in the OAMP MEP DB (CCM entry)
 *   SOC_SAND_OUT uint32                               *next_index
 * Next available index for additional LM/DM entries. Either endpoint id+1, endpoint_id+2 or endpoint_id+3
 * 0 if none available.
    SOC_SAND_OUT    uint8                              *has_dm
    returns 1 iff MEP has DM entry associated with it
 * REMARKS:
 *   Used for Arad+.
 *   Function is used to create LM and DM entries. Function changes nothing in the HW. 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_next_index_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 endpoint_id,
    SOC_SAND_OUT uint32                               *next_index,
    SOC_SAND_OUT    uint8                              *has_dm
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_eth1731_and_oui_profiles_get, 
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Manage LM and DM on the soc layer.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *   SOC_SAND_IN  uint32                                 endpoint_id,
 * index of the endpoint in the OAMP MEP DB (Must be of type ETH OAM to return oui profile.)
 *   SOC_SAND_IN  uint8                          remove_mode,
 *  SOC_SAND_OUT uint32                               *eth1731_prof
 *  SOC_SAND_OUT uint32                               *da_oui_prof
 *      Profiles associated with mep.
 * REMARKS:
 *   Used for Arad+. Function returns both because the oui profile cannot be recovered without
 * First finding the eth1731 profile.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_eth1731_and_oui_profiles_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 endpoint_id,
    SOC_SAND_OUT uint32                               *eth1731_prof,
    SOC_SAND_OUT uint32                               *da_oui_prof
  );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_nic_profile_get, 
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Manage LM and DM on the soc layer.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *   SOC_SAND_IN  uint32                                 endpoint_id,
 * index of the endpoint in the OAMP MEP DB (Must be of type ETH OAM)
 *  SOC_SAND_OUT uint32                               *da_nic_prof
 *      Profile associated with mep.
 * REMARKS:
 *   Used for Arad+.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
    soc_ppd_oam_oamp_nic_profile_get(
       SOC_SAND_IN  int                                 unit,
       SOC_SAND_IN  uint32                                 endpoint_id,
       SOC_SAND_OUT uint32                               *da_nic_prof
       );




/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_create_new_eth1731_profile, 
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Manage LM and DM on the soc layer.
 * INPUT:
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 endpoint_id,
    SOC_SAND_IN  uint8                          was_previously_alloced,
    SOC_SAND_IN  uint8                          profile_indx,
    SOC_SAND_IN SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY     *eth1731_profile
 *      
 * REMARKS:
 *   Used for Arad+.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_create_new_eth1731_profile(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 endpoint_id,
    SOC_SAND_IN  uint8                          was_previously_alloced,
    SOC_SAND_IN  uint8                          profile_indx,
    SOC_SAND_IN SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY     *eth1731_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_set_oui_nic_registers, 
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Manage OUI, NIC registers (MAC DA addresses)
 * INPUT:
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 endpoint_id,
    SOC_SAND_IN  uint32                                 msb_to_oui,
    SOC_SAND_IN  uint32                                 lsb_to_nic,
    SOC_SAND_IN  uint8                          profile_indx_oui 
    SOC_SAND_IN  uint8                          profile_indx_nic 
 
 *      
 * REMARKS:
 *   Used for Arad+.
 *  OUI profile should be written seperately by the create_new_eth1731_profile() function.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_set_oui_nic_registers(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 endpoint_id,
    SOC_SAND_IN  uint32                                 msb_to_oui,
    SOC_SAND_IN  uint32                                 lsb_to_nic,
    SOC_SAND_IN  uint8                          profile_indx_oui,
    SOC_SAND_IN  uint8                          profile_indx_nic
  );



/*********************************************************************
* NAME:
 *   soc_ppd_oam_oamp_lm_dm_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Manage LM and DM on the soc layer.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *     SOC_SAND_IN uint32                                        endpoint_id -
 * Entry number of the endpoint.
 *   SOC_SAND_IN  uint8                          is_lm,
 *   1 if lm needs removing, 0 otherwise
 *   SOC_SAND_IN uint8                           exists_piggy_back_down,
 In case we are removing a DM entry and the existing a piggybacked LM exists for an LM, and the direction is down, the MEP PE profile should be 0
		   so as not to select the UP-MEP-MAC fix LSB program.
 *   SOC_SAND_OUT uint8                               * num_removed,
 *    2 if LM + LM-STAT were removed, 1 if only an LM/DM was removed. Should never return 0.
 *   SOC_SAND_OUT uint32                              * removed_index
 * The index that was freed. If 2 entries were removed then this will be the index of the first of the two.
 * REMARKS:
 *   Used for Arad+.
 *   Functions are shared by LM and DM due to the similarity in the implementation of the two functionalities.
 * Function should be called only when there are entries to remove.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_oamp_lm_dm_remove(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN uint32                                        endpoint_id,
    SOC_SAND_IN  uint8                          is_lm,
    SOC_SAND_IN uint8                           exists_piggy_back_down,
    SOC_SAND_OUT uint8                               * num_removed,
    SOC_SAND_OUT uint32                              * removed_index
  );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_classifier_counter_disable_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set classifier counter disable map.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                  packet_is_oam -
 *     Bit to signal OAM/Data packets setting
 *   SOC_SAND_IN  uint8                                  profile -
 *     OAMA MP-Profile to set 
 *   SOC_SAND_IN  uint8                                  counter_enable -
 *     Value of the map - counter enable/disable
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_classifier_counter_disable_map_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint8                                  packet_is_oam,
    SOC_SAND_IN  uint8                                  profile,
    SOC_SAND_IN  uint8                                  counter_enable
  );

uint32
  soc_ppd_oam_classifier_counter_disable_map_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint8                                  packet_is_oam,
    SOC_SAND_IN  uint8                                  profile,
    SOC_SAND_OUT  uint8                                 *counter_enable
  );



/*********************************************************************
* NAME:
 *    soc_ppd_oam_oamp_loopback_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *    End OAM Loopback session
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 * REMARKS:
 *   Used for Arad+.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
    soc_ppd_oam_oamp_loopback_remove(
       SOC_SAND_IN  int                                                  unit
       );

/*********************************************************************
* NAME:
 *    soc_ppd_oam_dma_reset
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Reset the DMA.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 * REMARKS:
 *   Used for Arad+. To be used after WB.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
    soc_ppd_oam_dma_reset(
       SOC_SAND_IN  int                                                  unit
       );

/*********************************************************************
* NAME:
 *    soc_ppd_oam_dma_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Reset the DMA.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 * REMARKS:
 *   Used for Arad+. 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
    soc_ppd_oam_dma_clear(
       SOC_SAND_IN  int                                                  unit
       );


/*********************************************************************
* NAME:
 *    soc_ppd_oam_register_dma_event_handler_callback
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Reset the DMA.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *   SOC_SAND_IN        int (*event_handler_cb)(int)
 Function called upon DMA interrupt. Paramater is declared as SOC_SAND_INOUT as opposed to SOC_SAND_IN
 for compilation.
 * REMARKS:
 *   Used for Arad+. 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
    soc_ppd_oam_register_dma_event_handler_callback(
       SOC_SAND_IN  int                                                  unit,
       SOC_SAND_INOUT        dma_event_handler_cb_t          event_handler_cb
       );



/*********************************************************************
* NAME:
 *    soc_ppd_oam_dma_event_handler
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Reset the DMA.
 * INPUT:
      SOC_SAND_INOUT  void        *     unit_ptr -
 *  device id stored in void* form.
 
      SOC_SAND_INOUT  void        *     event_type_ptr -
 *  Event type (stat, normal, ..) stored in void*

      SOC_SAND_INOUT  void        *     unused2,
      SOC_SAND_INOUT  void        *     unused3,
       SOC_SAND_INOUT  void        *     unused4
 * REMARKS:
 *   Used for Arad+.  To be called when DMA related interrupt is triggered.
 * RETURNS:
 *   Nothing.
*********************************************************************/
void 
   soc_ppd_oam_dma_event_handler(
      SOC_SAND_INOUT  void        *     unit_ptr,
      SOC_SAND_INOUT  void        *     event_type_ptr,
      SOC_SAND_INOUT  void        *     cmc_ptr,
      SOC_SAND_INOUT  void        *     ch_ptr,
       SOC_SAND_INOUT  void       *     unused4
       );



/* ARAD+ function end*/

/***********************************************************************************/
/*   *****************************************************FUNCTIONS USED FOR DIAGNOSTICS*/


/*********************************************************************
* NAME:
 *   soc_ppd_get_crps_counter
 * TYPE:
 *   PROC
 * FUNCTION:
 *   get crps cnts mem
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                           crps_counter_number,
 *     may be 0 ,1, 2,3
 *   SOC_SAND_IN  uint32                                  reg_number,
 *     may be 0 - (32K -1)
 *  SOC_SAND_OUT uint64*                               value
 *  result goes here
 * REMARKS:
 *   Function used for oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_get_crps_counter(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint8                           crps_counter_number,
    SOC_SAND_IN  uint32                                  reg_number,
    SOC_SAND_OUT uint32*                               value
  );



/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_lookup
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print lookup information from OAM exact matches.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 * REMARKS:
 *   Used by oam diagnostics
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_lookup(
     SOC_SAND_IN int unit
   );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_rx
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oam rx information.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 * REMARKS:
 *   Used by oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_rx(
     SOC_SAND_IN int unit,
     SOC_SAND_IN int core_id

   );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_print_em
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oam EM-1 information.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
     SOC_SAND_IN int LIF, use to  construct key
 * REMARKS:
 *   Used by oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_em(
     SOC_SAND_IN int unit,
     SOC_SAND_IN int LIF
   );


/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_print_ak
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oam OAM-1/2 lookup key information.
 * INPUT:
 *   SOC_SAND_IN int unit
     SOC_SAND_IN SOC_PPC_OAM_ACTION_KEY_PARAMS *key_params
 * REMARKS:
 *   Used by oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_ak(
     SOC_SAND_IN int unit,
     SOC_SAND_IN SOC_PPC_OAM_ACTION_KEY_PARAMS *key_params
   );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_print_ks
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oam action key selction information for O-EM1 table.
 * INPUT:
 *   SOC_SAND_IN int unit
     SOC_SAND_IN SOC_PPC_OAM_KEY_SELECT_PARAMS *key_params
 * REMARKS:
 *   Used by oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_ks(
     SOC_SAND_IN int unit,
     SOC_SAND_IN SOC_PPC_OAM_KEY_SELECT_PARAMS *key_params
   );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_print_debug
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oamp counter information.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 * REMARKS:
 *   Used by oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_oamp_counter(
     SOC_SAND_IN int unit
   );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_print_debug
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oam id debug information.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
     SOC_SAND_IN int cfg, whether to  configure debug mode
     SOC_SAND_IN int mode, use to  configure debug mode
 * REMARKS:
 *   Used by oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_debug(
     SOC_SAND_IN int unit,
	 SOC_SAND_IN int cfg,
     SOC_SAND_IN int mode
   );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_print_oam_id
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oam id debug information.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
     SOC_SAND_IN int mode, use to  configure debug mode
 * REMARKS:
 *   Used by oam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_oam_id(
     SOC_SAND_IN int unit,
     SOC_SAND_IN int core_id
   );

/*********************************************************************
* NAME:
 *   soc_ppd_oam_diag_print_tcam_entries
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print oam tcam key.
 * INPUT:
 *   SOC_SAND_IN  int                   unit
     SOC_SAND_IN  int                   core_id
 * REMARKS:
 *   Used by oam tcam diagnostics.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_oam_diag_print_tcam_entries(
     SOC_SAND_IN int unit,
     SOC_SAND_IN int core_id
   );

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_PPD_INCLUDED__*/
#endif
