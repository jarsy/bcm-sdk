/* $Id: ppd_api_llp_vid_assign.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_llp_vid_assign.h
*
* MODULE PREFIX:  soc_ppd_llp
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

#ifndef __SOC_PPD_API_LLP_VID_ASSIGN_INCLUDED__
/* { */
#define __SOC_PPD_API_LLP_VID_ASSIGN_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_llp_vid_assign.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_llp_sa_auth.h>

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

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPD_LLP_VID_ASSIGN_PORT_INFO_SET = SOC_PPD_PROC_DESC_BASE_LLP_VID_ASSIGN_FIRST,
  SOC_PPD_LLP_VID_ASSIGN_PORT_INFO_SET_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_PORT_INFO_GET,
  SOC_PPD_LLP_VID_ASSIGN_PORT_INFO_GET_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_ADD,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_ADD_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_REMOVE,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_REMOVE_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_GET,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_GET_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK,
  SOC_PPD_LLP_VID_ASSIGN_MAC_BASED_GET_BLOCK_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET,
  SOC_PPD_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_SET_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET,
  SOC_PPD_LLP_VID_ASSIGN_IPV4_SUBNET_BASED_GET_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_PROTOCOL_BASED_SET,
  SOC_PPD_LLP_VID_ASSIGN_PROTOCOL_BASED_SET_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_PROTOCOL_BASED_GET,
  SOC_PPD_LLP_VID_ASSIGN_PROTOCOL_BASED_GET_PRINT,
  SOC_PPD_LLP_VID_ASSIGN_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LLP_VID_ASSIGN_PROCEDURE_DESC_LAST
} SOC_PPD_LLP_VID_ASSIGN_PROCEDURE_DESC;


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

/*********************************************************************
* NAME:
 *   soc_ppd_llp_vid_assign_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets port information for VID assignment, including PVID
 *   and which assignment mechanism to enable for this port.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
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
  soc_ppd_llp_vid_assign_port_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_vid_assign_port_info_set" API.
 *     Refer to "soc_ppd_llp_vid_assign_port_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_vid_assign_port_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_PORT_INFO            *port_vid_assign_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_vid_assign_mac_based_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set assignment to VLAN ID according to source MAC
 *   address.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key -
 *     MAC address, according to which to assign VID.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info -
 *     VID assignment information for the given MAC.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
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
  soc_ppd_llp_vid_assign_mac_based_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_vid_assign_mac_based_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove assignment to VLAN ID according to source MAC
 *   address.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key -
 *     MAC address, according to which to assign the VID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_vid_assign_mac_based_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_vid_assign_mac_based_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get VLAN ID assignment information according to source
 *   MAC address.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key -
 *     MAC address, according to which to assign VID.
 *   SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info -
 *     VID assignment information for the given MAC.
 *   SOC_SAND_OUT uint8                               *found -
 *     Is mac_address_key was found
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_vid_assign_mac_based_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *mac_based_info,
    SOC_SAND_OUT uint8                               *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_vid_assign_mac_based_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get VLAN ID assignment information according to source
 *   MAC address.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE           *rule -
 *     Get only entries that match this rule.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Range for iteration
 *   SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key_arr -
 *     MAC address array.
 *   SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *vid_assign_info_arr -
 *     VID assignment information
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of returned enties in vid_assign_info_arr
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_vid_assign_mac_based_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MATCH_RULE           *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                     *mac_address_key_arr,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO             *vid_assign_info_arr,
    SOC_SAND_OUT uint32                                *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_vid_assign_ipv4_subnet_based_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set assignment of VLAN ID based on source IPv4 subnet.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                entry_ndx -
 *     Index in the subnet table where to write the
 *     information. Range: Soc_petraB: 0 - 15. T20E: 0 - 23.
 *   SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                     *subnet -
 *     Source Ipv4 Subnet.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info -
 *     The VLAN assignment Information according to subnet.
 * REMARKS:
 *   - At most one subnet can be set for each etrny_ndx.- The
 *   domain of entry_ndx is shared for VID assignment and TC
 *   mapping, so when use soc_ppd_llp_cos_ipv4_subnet_based_set
 *   API it may override the subnet and entry_is_valid.- If
 *   there is more than one match in the table, then the
 *   entry with the lower index will be taken.- In Soc_petra-B:
 *   lower index with match will be taken if the
 *   is_vid_valid/is_tc_valid of the first matched entry is
 *   FALSE, and then the traverse over the subnets will stop
 *   and the TC value will not be changed.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_vid_assign_ipv4_subnet_based_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                     *subnet,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_vid_assign_ipv4_subnet_based_set" API.
 *     Refer to "soc_ppd_llp_vid_assign_ipv4_subnet_based_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_llp_vid_assign_ipv4_subnet_based_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IPV4_SUBNET                     *subnet,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_IPV4_SUBNET_INFO     *subnet_based_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_vid_assign_protocol_based_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets mapping from Ethernet Type and profile to VID.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                port_profile_ndx -
 *     Profile for VID assignment (and COS resolution)
 *     according to L2 Protocol (Ethernet Type). Profile is
 *     determined per port see soc_ppd_port_info_set(). Range: 0 -
 *     7.
 *   SOC_SAND_IN  uint16                                ether_type_ndx -
 *     Ethernet Type, up to 10 different Ethernet Types,
 *     including
 *     IPv4(0x0800),IPv6(0x86DD),ARP(0x0806),CFM(0x8902), TRILL
 *     MPLS(0x8847). Use SOC_PPC_L2_NEXT_PRTCL_TYPE enumeration to
 *     refer to these constant values.
 *   SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info -
 *     VID assignment information according to Ethernet Type.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
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
  soc_ppd_llp_vid_assign_protocol_based_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_IN  uint16                                ether_type_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_vid_assign_protocol_based_set" API.
 *     Refer to "soc_ppd_llp_vid_assign_protocol_based_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_vid_assign_protocol_based_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_IN  uint16                                ether_type_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_PRTCL_INFO           *prtcl_assign_info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LLP_VID_ASSIGN_INCLUDED__*/
#endif

