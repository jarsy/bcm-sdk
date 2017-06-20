/* $Id: arad_pp_llp_vid_assign.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_LLP_VID_ASSIGN_INCLUDED__
/* { */
#define __ARAD_PP_LLP_VID_ASSIGN_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

#include <soc/dpp/PPC/ppc_api_llp_vid_assign.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_SW_DB_PP_PORTS_NOF_U32  (ARAD_PP_NOF_PORTS/32)

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

typedef struct ARAD_LLP_VID_ASSIGN_s
{
  SHR_BITDCL vid_sa_based_enable[ARAD_PP_SW_DB_PP_PORTS_NOF_U32];
} ARAD_LLP_VID_ASSIGN_t;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET = ARAD_PP_PROC_DESC_BASE_LLP_VID_ASSIGN_FIRST,
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_PRINT,
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_UNSAFE,
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_SET_VERIFY,
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET,
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_PRINT,
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_VERIFY,
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_GET_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_ADD_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_REMOVE_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_PRINT,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_VERIFY,
  ARAD_PP_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_UNSAFE,
  ARAD_PP_LLP_VID_ASSIGN_GET_PROCS_PTR,
  ARAD_PP_LLP_VID_ASSIGN_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_VID_ASSIGN_PROCEDURE_DESC_LAST
} ARAD_PP_LLP_VID_ASSIGN_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LLP_VID_ASSIGN_SUCCESS_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LLP_VID_ASSIGN_FIRST,
  ARAD_PP_LLP_VID_ASSIGN_ENTRY_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_VID_ASSIGN_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_VID_ASSIGN_ETHER_TYPE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_VID_ASSIGN_RULE_TYPE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
   ARAD_PP_LLP_VID_ASSIGN_SUBNET_PREF_IS_ZERO_ERR,
   ARAD_PP_LLP_VID_ASSIGN_DIS_AUTH_EN_ERR,



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_VID_ASSIGN_ERR_LAST
} ARAD_PP_LLP_VID_ASSIGN_ERR;

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

uint32
  arad_pp_llp_vid_assign_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_port_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets port information for VID assignment, including PVID
 *   and which assignment mechanism to enable for this port.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info -
 *     The port information for VID assignment.
 * REMARKS:
 *   OK or Error indicationREMARKS
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_port_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                         core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  );

uint32
  arad_pp_llp_vid_assign_port_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  );

uint32
  arad_pp_llp_vid_assign_port_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_vid_assign_port_info_set_unsafe" API.
 *     Refer to "arad_pp_llp_vid_assign_port_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_port_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                         core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_mac_based_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set assignment to VLAN ID according to source MAC
 *   address.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key -
 *     MAC address, according to which to assign VID.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info -
 *     VID assignment information for the given MAC.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Negated when there is no place in the VID assignment
 *     SA-based DB.
 * REMARKS:
 *   - The DB used for VID assignment is shared with
 *   SA-Authorization. When setting Authorization over a SA
 *   MAC according to VID, this is also the VID that may be
 *   used for VID assignment.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_llp_vid_assign_mac_based_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_mac_based_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove assignment to VLAN ID according to source MAC
 *   address.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key -
 *     MAC address, according to which to assign the VID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key
  );

uint32
  arad_pp_llp_vid_assign_mac_based_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_mac_based_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get VLAN ID assignment information according to source
 *   MAC address.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key -
 *     MAC address, according to which to assign VID.
 *   SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info -
 *     VID assignment information for the given MAC.
 *   SOC_SAND_OUT uint8                                 *found -
 *     Is mac_address_key was found
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_llp_vid_assign_mac_based_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_mac_based_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get VLAN ID assignment information according to source
 *   MAC address.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE           *rule -
 *     Get only entries that match this rule.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range -
 *     Range for iteration
 *   SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key_arr -
 *     MAC address array.
 *   SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *vid_assign_info_arr -
 *     VID assignment information
 *   SOC_SAND_OUT uint32                                  *nof_entries -
 *     Number of returned enties in vid_assign_info_arr
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_mac_based_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE           *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                       *mac_address_key_arr,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *vid_assign_info_arr,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_llp_vid_assign_mac_based_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE           *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_ipv4_subnet_based_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set assignment of VLAN ID based on source IPv4 subnet.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  entry_ndx -
 *     Index in the subnet table where to write the
 *     information. Range: AradB: 0 - 15. T20E: 0 - 23.
 *   SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                       *subnet -
 *     Source Ipv4 Subnet.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info -
 *     The VLAN assignment Information according to subnet.
 * REMARKS:
 *   - At most one subnet can be set for each etrny_ndx.- The
 *   domain of entry_ndx is shared for VID assignment and TC
 *   mapping, so when use soc_ppd_llp_cos_ipv4_subnet_based_set
 *   API it may override the subnet and entry_is_valid.- If
 *   there is more than one match in the table, then the
 *   entry with the lower index will be taken.- In Arad-B:
 *   lower index with match will be taken if the
 *   is_vid_valid/is_tc_valid of the first matched entry is
 *   FALSE, and then the traverse over the subnets will stop
 *   and the TC value will not be changed.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                       *subnet,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  );

uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                       *subnet,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  );

uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_vid_assign_ipv4_subnet_based_set_unsafe" API.
 *     Refer to
 *     "arad_pp_llp_vid_assign_ipv4_subnet_based_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_ipv4_subnet_based_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IPV4_SUBNET                       *subnet,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_protocol_based_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets mapping from Ethernet Type and profile to VID.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  port_profile_ndx -
 *     Profile for VID assignment (and COS resolution)
 *     according to L2 Protocol (Ethernet Type). Profile is
 *     determined per port see soc_ppd_port_info_set(). Range: 0 -
 *     7.
 *   SOC_SAND_IN  uint16                                  ether_type_ndx -
 *     Ethernet Type, up to 10 different Ethernet Types,
 *     including
 *     IPv4(0x0800),IPv6(0x86DD),ARP(0x0806),CFM(0x8902), TRILL
 *     MPLS(0x8847). Use SOC_PPC_L2_NEXT_PRTCL_TYPE enumeration to
 *     refer to these constant values.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info -
 *     VID assignment information according to Ethernet Type.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the set operation succeeded. Operation may fail
 *     if there are no available resources to support the given
 *     (new) Ethernet Type.
 * REMARKS:
 *   - This API uses the same DB used for TC assignment
 *   according to L2 protocol. See
 *   soc_ppd_llp_vid_assign_protocol_based_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_protocol_based_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_llp_vid_assign_protocol_based_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info
  );

uint32
  arad_pp_llp_vid_assign_protocol_based_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_vid_assign_protocol_based_set_unsafe" API.
 *     Refer to
 *     "arad_pp_llp_vid_assign_protocol_based_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_llp_vid_assign_protocol_based_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint16                                  ether_type_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_llp_vid_assign module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_llp_vid_assign_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_llp_vid_assign_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_llp_vid_assign module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_llp_vid_assign_get_errs_ptr(void);

uint32
  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO *info
  );

uint32
  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO *info
  );

uint32
  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO *info
  );

uint32
  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO *info
  );

uint32
  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE_verify(
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LLP_VID_ASSIGN_INCLUDED__*/
#endif

