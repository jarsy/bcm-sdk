/* $Id: ppd_api_llp_cos.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_llp_cos.h
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

#ifndef __SOC_PPD_API_LLP_COS_INCLUDED__
/* { */
#define __SOC_PPD_API_LLP_COS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_llp_cos.h>

#include <soc/dpp/PPD/ppd_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     use to indicates no matching source/destination l4 ports
 *     ranges                                                  */

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
  SOC_PPD_LLP_COS_GLBL_INFO_SET = SOC_PPD_PROC_DESC_BASE_LLP_COS_FIRST,
  SOC_PPD_LLP_COS_GLBL_INFO_SET_PRINT,
  SOC_PPD_LLP_COS_GLBL_INFO_GET,
  SOC_PPD_LLP_COS_GLBL_INFO_GET_PRINT,
  SOC_PPD_LLP_COS_PORT_INFO_SET,
  SOC_PPD_LLP_COS_PORT_INFO_SET_PRINT,
  SOC_PPD_LLP_COS_PORT_INFO_GET,
  SOC_PPD_LLP_COS_PORT_INFO_GET_PRINT,
  SOC_PPD_LLP_COS_MAPPING_TABLE_ENTRY_SET,
  SOC_PPD_LLP_COS_MAPPING_TABLE_ENTRY_SET_PRINT,
  SOC_PPD_LLP_COS_MAPPING_TABLE_ENTRY_GET,
  SOC_PPD_LLP_COS_MAPPING_TABLE_ENTRY_GET_PRINT,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_INFO_SET,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_INFO_SET_PRINT,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_INFO_GET,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_INFO_GET_PRINT,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_TO_TC_INFO_SET,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_TO_TC_INFO_SET_PRINT,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_TO_TC_INFO_GET,
  SOC_PPD_LLP_COS_L4_PORT_RANGE_TO_TC_INFO_GET_PRINT,
  SOC_PPD_LLP_COS_IPV4_SUBNET_BASED_SET,
  SOC_PPD_LLP_COS_IPV4_SUBNET_BASED_SET_PRINT,
  SOC_PPD_LLP_COS_IPV4_SUBNET_BASED_GET,
  SOC_PPD_LLP_COS_IPV4_SUBNET_BASED_GET_PRINT,
  SOC_PPD_LLP_COS_PROTOCOL_BASED_SET,
  SOC_PPD_LLP_COS_PROTOCOL_BASED_SET_PRINT,
  SOC_PPD_LLP_COS_PROTOCOL_BASED_GET,
  SOC_PPD_LLP_COS_PROTOCOL_BASED_GET_PRINT,
  SOC_PPD_LLP_COS_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LLP_COS_PROCEDURE_DESC_LAST
} SOC_PPD_LLP_COS_PROCEDURE_DESC;

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
 *   soc_ppd_llp_cos_glbl_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets global information for COS resolution, including
 *   default drop precedence.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_GLBL_INFO                   *glbl_info -
 *     Global information for COS resolution.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_cos_glbl_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_COS_GLBL_INFO                   *glbl_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_cos_glbl_info_set" API.
 *     Refer to "soc_ppd_llp_cos_glbl_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_llp_cos_glbl_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_LLP_COS_GLBL_INFO                   *glbl_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_cos_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets port information for COS resolution, including
 *   which mechanisms to perform, table instance to use for
 *   mapping,...
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_PORT_INFO                   *port_info -
 *     Port information for COS resolution.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_cos_port_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                                         core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_COS_PORT_INFO                   *port_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_cos_port_info_set" API.
 *     Refer to "soc_ppd_llp_cos_port_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_llp_cos_port_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_COS_PORT_INFO                   *port_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_cos_mapping_table_entry_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from packet fields to TC/DP, includes the
 *   following mappings:- UP to DE and TC.- IPv4 TOS to DP
 *   and TC.- IPv6 TC to DP and TC.- DE to DP- UP to DP.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_MAPPING_TABLE               mapping_tbl_ndx -
 *     Selects the mapping table to modify (TOS to TC, UP to TC
 *     etc...)
 *   SOC_SAND_IN  uint32                                table_id_ndx -
 *     Selects the instance of the table. One mapping may have
 *     more than one table, for example, TOS to TC has two
 *     tables, and this parameter selects one of these tables.
 *     Each port is assigned one of these tables to be used -
 *     see soc_ppd_llp_cos_port_info_set()
 *   SOC_SAND_IN  uint32                                entry_ndx -
 *     Mapped value may be UP/TOS/TC/DE/EXP according to the
 *     selected table.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO    *entry -
 *     The value to map to. May be TC or DP according to the
 *     selected table.
 * REMARKS:
 *   - The order of values in the entry (value1 and value2)
 *   is determined according to their appearance in the
 *   Enumerator SOC_PPC_LLP_COS_MAPPING_TABLE - Example for
 *   mapping IPv4 To TC and DP:
 *   mapping_tbl_ndx=SOC_PPC_LLP_COS_MAPPING_TABLE_IPV4_TOS_TO_DP_TC_VALID
 *   table_id_ndx = 0 or 1 entry_ndx = 0..255 (this is
 *   actually the mapped TOS) entry.value1 = 0...3 (DP value)
 *   entry.value2 = 0...7 (TC value) entry.valid = is TC valid
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_cos_mapping_table_entry_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_COS_MAPPING_TABLE               mapping_tbl_ndx,
    SOC_SAND_IN  uint32                                table_id_ndx,
    SOC_SAND_IN  uint32                                entry_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO    *entry
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_cos_mapping_table_entry_set" API.
 *     Refer to "soc_ppd_llp_cos_mapping_table_entry_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_cos_mapping_table_entry_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_COS_MAPPING_TABLE               mapping_tbl_ndx,
    SOC_SAND_IN  uint32                                table_id_ndx,
    SOC_SAND_IN  uint32                                entry_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO    *entry
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_cos_l4_port_range_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets range of L4 ports. Packets L4 ports will be
 *   compared to this range in order to set TC value
 *   correspondingly.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                range_ndx -
 *     Range ID. Range: 0 - 2.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO          *range_info -
 *     Range of L4 ports.
 * REMARKS:
 *   - Lower value range_ndx has priority over higher value.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_cos_l4_port_range_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                range_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO          *range_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_cos_l4_port_range_info_set" API.
 *     Refer to "soc_ppd_llp_cos_l4_port_range_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_cos_l4_port_range_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                range_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO          *range_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_cos_l4_port_range_to_tc_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping to Traffic class according to L4 information
 *   includingSource/Destination and protocol type (TCP or
 *   UDP). Packet L4 destination and source ports are
 *   compared to the ranges set by
 *   soc_ppd_llp_cos_l4_port_range_info_set(). Then according to
 *   comparison result: - 0-3 for source port (where 3 means
 *   was not match) - 0-3 for destination port (where 3 means
 *   was not match)and according to protocol type (UDP/TCP),
 *   this API is used to determine the TC value to set to the
 *   packet
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_L4_PRTCL_TYPE                   l4_prtcl_type_ndx -
 *     L4 Protocol type (UDP/TCP).
 *   SOC_SAND_IN  uint32                                src_port_match_range_ndx -
 *     The ranges match the packet L4 source port - set to
 *     SOC_PPD_LLP_COS_l4_RANG_RANGE_NO_MATCH for packet with no
 *     match.
 *   SOC_SAND_IN  uint32                                dest_port_match_range_ndx -
 *     The ranges match the packet L4 destination port - set to
 *     SOC_PPD_LLP_COS_l4_RANG_RANGE_NO_MATCH for packet with no
 *     match.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_TC_INFO                     *tc_info -
 *     TC information value and valid.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_cos_l4_port_range_to_tc_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_L4_PRTCL_TYPE                   l4_prtcl_type_ndx,
    SOC_SAND_IN  uint32                                src_port_match_range_ndx,
    SOC_SAND_IN  uint32                                dest_port_match_range_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_COS_TC_INFO                     *tc_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_cos_l4_port_range_to_tc_info_set" API.
 *     Refer to "soc_ppd_llp_cos_l4_port_range_to_tc_info_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_llp_cos_l4_port_range_to_tc_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_L4_PRTCL_TYPE                   l4_prtcl_type_ndx,
    SOC_SAND_IN  uint32                                src_port_match_range_ndx,
    SOC_SAND_IN  uint32                                dest_port_match_range_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_COS_TC_INFO                     *tc_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_cos_ipv4_subnet_based_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set COS parameters based on source IPv4 subnet.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                entry_ndx -
 *     Entry index in the subnet table. Range: Soc_petra-B: 0 - 15.
 *     T20E: 0 - 23.
 *   SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                     *subnet -
 *     Source Ipv4 Subnet.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_IPV4_SUBNET_INFO            *subnet_based_info -
 *     The COS resolution Information according to subnet.
 * REMARKS:
 *   - that in-order to set TC according to subnet, user has to call first
 *     soc_ppd_llp_vid_assign_ipv4_subnet_based_set() with same entry-index, subnet,
 *     and with vid_valid = TRUE. And then call
 *     soc_ppd_llp_cos_ipv4_subnet_based_set().
 *   - The domain of entry_ndx is shared for VID assignment
 *   and TC mapping - see
 *   soc_ppd_llp_vid_assign_ipv4_subnet_based_set()- If there is
 *   more than one match in the table, then the entry with
 *   the lower index will be taken.- In Soc_petra-B: lower index
 *   with match will be taken if the is_vid_valid/is_tc_valid
 *   of the first matched entry is FALSE, and then the
 *   traverse over the subnets will stop and the TC value
 *   will not be changed.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_cos_ipv4_subnet_based_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET                     *subnet,
    SOC_SAND_IN  SOC_PPC_LLP_COS_IPV4_SUBNET_INFO            *subnet_based_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_cos_ipv4_subnet_based_set" API.
 *     Refer to "soc_ppd_llp_cos_ipv4_subnet_based_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_cos_ipv4_subnet_based_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IPV4_SUBNET                     *subnet,
    SOC_SAND_OUT SOC_PPC_LLP_COS_IPV4_SUBNET_INFO            *subnet_based_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_cos_protocol_based_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets mapping from Ethernet Type and profile to COS
 *   parameters.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                profile_ndx -
 *     Profile for COS resolution (and VID assignment)
 *     according to L2 Protocol (Ethernet Type). Profile is
 *     determined per port see soc_ppd_port_info_set() Range: 0 -
 *     7.
 *   SOC_SAND_IN  SOC_SAND_PP_ETHER_TYPE                      ether_type_ndx -
 *     Ethernet Type, up to 10 different Ethernet Types,
 *     including
 *     IPv4(0x0800),IPv6(0x86DD),ARP(0x0806),CFM(0x8902), TRILL
 *     MPLS(0x8847). Use SOC_PPC_L2_NEXT_PRTCL_TYPE enumeration to
 *     refer to these constant values.
 *   SOC_SAND_IN  SOC_PPC_LLP_COS_PRTCL_INFO                  *prtcl_assign_info -
 *     COS resolution information according to Ethernet Type.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the set operation succeeded. Operation may fail
 *     if there are no available resources to support the given
 *     (new) Ethernet Type.
 * REMARKS:
 *   - This API uses the same DB used for VID protocol-based
 *   assignment. See soc_ppd_llp_vid_assign_protocol_based_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_cos_protocol_based_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHER_TYPE                      ether_type_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_COS_PRTCL_INFO                  *prtcl_assign_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_cos_protocol_based_set" API.
 *     Refer to "soc_ppd_llp_cos_protocol_based_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_cos_protocol_based_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHER_TYPE                      ether_type_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_COS_PRTCL_INFO                  *prtcl_assign_info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LLP_COS_INCLUDED__*/
#endif

