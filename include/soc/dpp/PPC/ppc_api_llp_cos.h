/* $Id: ppc_api_llp_cos.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_llp_cos.h
*
* MODULE PREFIX:  soc_ppc_api_llp_cos
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

#ifndef __SOC_PPC_API_LLP_COS_INCLUDED__
/* { */
#define __SOC_PPC_API_LLP_COS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     use to indicates no matching source/destination l4 ports
 *     ranges                                                  */
#define  SOC_PPC_LLP_COS_L4_RANGE_NO_MATCH (0xFFFFFFFF)

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
   *  UP to DE and TC. Decodes the UP/PCP of the incoming
   *  packet to Traffic Class and Drop Eligibility (internal
   *  representation of the COS parameters)
   *  In Soc_petra-B: the mapping to DE is used if packet has S-tag,
   *  then the DE is used to set the DP value.
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_UP_TO_DE_TC = 0,
  /*
   *  IPv4 TOS to DP and TC. Maps the IPv4 header TOS to the
   *  following values:
   *     DP - Drop Precedence
   *     TC - Traffic Class
   *     TC-Valid
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_IPV4_TOS_TO_DP_TC_VALID = 1,
  /*
   *  IPv6 TC to DP and TC. Maps the IPv6 header TC to the
   *  following values:
   *     DP - Drop Precedence
   *     TC - Traffic Class
   *     TC-Valid
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_IPV6_TC_TO_DP_TC_VALID = 2,
  /*
   *  TC to DP. Relevant only for T20E.
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_TC_TO_DP = 3,
  /*
   *  Maps the incoming user priority / PCP to a user priority
   *  (i.e. the internal representation)Relevant only for
   *  Soc_petra-A compatibility. Otherwise has no use.
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_INCOMING_UP_TO_UP = 4,
  /*
   *  Maps the traffic class to user priority. Relevant only
   *  for Soc_petra-A compatibility. Otherwise has no use.
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_TC_TO_UP = 5,
  /*
   *  DE to DP. Maps the drop eligibility to a drop precedence
   *  In Soc_petra-B: This mapping is used when the packet has S-Tag.
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_DE_TO_DP = 6,
  /*
   *  UP to DP. Maps the user priority to drop precedence.
   *  In Soc_petra-B: This mapping is used if packet has C-tag.
   *  in T20E: this mapping is not relevant, and DP
   *  is always mapped from DE.
   */
  SOC_PPC_LLP_COS_MAPPING_TABLE_UP_TO_DP = 7,
  /*
   *  Number of types in SOC_PPC_LLP_COS_MAPPING_TABLE
   */
  SOC_PPC_NOF_LLP_COS_MAPPING_TABLES = 8
}SOC_PPC_LLP_COS_MAPPING_TABLE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Default Drop Precedence. Range: 0 - 3. Default TC may be
   *  set per port-See SOC_PPC_LLP_COS_PORT_INFO.
   */
  SOC_SAND_PP_DP default_dp;

} SOC_PPC_LLP_COS_GLBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Traffic Class value. Range: 0 - 7.
   */
  uint8 tc;
  /*
   *  Is the TC valid
   */
  uint8 valid;

} SOC_PPC_LLP_COS_TC_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Should the UP that arrived with the packet take place in
   *  the calculation of TC. Relevant only for Soc_petra-A-compatibility.
   */
  uint8 use_for_tc;
  /*
   *  When valid, the incoming UP also affects the calculation
   *  of the UP to be sent when leaving the bridge.
   *  not used in Soc_petra-B.
   */
  uint8 use_for_out_up;

} SOC_PPC_LLP_COS_UP_USE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Selects the table for mapping the incoming UP/PCP field
   *  (from the Tag) to TC and DE. To configure the mapping of
   *  this table use soc_ppd_llp_cos_mapping_table_entry_set(),
   *  with mapping_tbl  = SOC_PPC_LLP_COS_MAPPING_TABLE_UP_TO_DE_TC.
   *  table_id_ndx = in_up_to_tc_and_de_index.
   *  Range 0 -3.
   */
  uint32 in_up_to_tc_and_de_index;
  /*
   *  Selects the table for mapping TC to UP. Range: 0 -
   *  3. Relevant only for Soc_petra-B (For Soc_petra-A Compatibility).
   *  To configure the mapping of this table use
   *  soc_ppd_llp_cos_mapping_table_entry_set(),  with
   *  mapping_tbl  = SOC_PPC_LLP_COS_MAPPING_TABLE_TC_TO_UP.
   *  table_id_ndx =  in_ tc_to_up_index.
   */
  uint32 tc_to_up_index;
  /*
   *  Selects the table for mapping UP to DP. Range: 0 - 1.
   *  Relevant only for Soc_petra-B.
   *  To configure the mapping of this table use
   *  soc_ppd_llp_cos_mapping_table_entry_set(),  with
   *  mapping_tbl  = SOC_PPC_LLP_COS_MAPPING_TABLE_UP_TO_DP.
   *  table_id_ndx = up_to_dp_index.
   */
  uint32 up_to_dp_index;
  /*
   *  Selects the table for mapping TC to DP. Range: 0 - 1.
   *  Relevant only for T20E.
   *  To configure the mapping of this table use
   *  soc_ppd_llp_cos_mapping_table_entry_set(),  with
   *  mapping_tbl  = SOC_PPC_LLP_COS_MAPPING_TABLE_TC_TO_DP.
   *  table_id_ndx = tc_to_dp_index.
   */
  uint32 tc_to_dp_index;

} SOC_PPC_LLP_COS_PORT_L2_TABLE_INDEXES;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  How to use the incoming UP in the COS resolution.
   */
  SOC_PPC_LLP_COS_UP_USE up_use;
  /*
   *  If TRUE, then use L2 Ethernet type for mapping to TC
   */
  uint8 use_l2_protocol;
  /*
   *  If TRUE, then ignore PCP from incoming packet for TC calculation,
   *  and calculation will be according to other criteria (IP tos,....)
   *  if FALSE, then for tagged packet TC is a mapping of packet PCP
   */
  uint8 ignore_pkt_pcp_for_tc;
  /*
   *  If set, the port uses the DEI.
   */
  uint8 use_dei;
  /*
   *  Select tables for mapping from/to L2 attributes
   */
  SOC_PPC_LLP_COS_PORT_L2_TABLE_INDEXES tbls_select;

} SOC_PPC_LLP_COS_PORT_L2_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, then use IPv4 header (TOS) and IPv6 header (TC)
   *  for mapping to TC
   */
  uint8 use_ip_qos;
  /*
   *  Selects the table for mapping the TOS/TC field in
   *  IPv4/IPv6 Header to TC. Relevant only if use_ip_qos is
   *  TRUE
   */
  uint32 ip_qos_to_tc_index;
  /*
   *  If TRUE, then use IPv4 source subnet for mapping to TC
   */
  uint8 use_ip_subnet;
  /*
   *  If TRUE, then when MPLS label is terminated use mapping
   *  according to label cos. Otherwise, the COS params
   *  resolved by this module will not be affected upon tunnel
   *  termination. T20E only. In Soc_petra-B has to be 0.
   */
  uint8 use_mpls_term_lbl;

} SOC_PPC_LLP_COS_PORT_L3_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, then use L4 header protocol (TCP and UDP) and
   *  ports range for mapping to TC
   */
  uint8 use_l4_prtcl;

} SOC_PPC_LLP_COS_PORT_L4_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Layer 2 information
   */
  SOC_PPC_LLP_COS_PORT_L2_INFO l2_info;
  /*
   *  Layer 3 information
   */
  SOC_PPC_LLP_COS_PORT_L3_INFO l3_info;
  /*
   *  Layer 4 information
   */
  SOC_PPC_LLP_COS_PORT_L4_INFO l4_info;
  /*
   *  Default TC of the port
   */
  SOC_SAND_PP_TC default_tc;

} SOC_PPC_LLP_COS_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The first value of the entry
   */
  uint32 value1;
  /*
   *  The second value of the entry (if present)
   */
  uint32 value2;
  /*
   *  The valid field of the entry (if present). Refers to
   *  value2. See SOC_PPC_LLP_COS_MAPPING_TABLE..
   */
  uint8 valid;

} SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Traffic Class. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  TC validity for this entry. If TRUE, then when there are
   *  subnet matches, this TC is taken.
   */
  uint8 tc_is_valid;

} SOC_PPC_LLP_COS_IPV4_SUBNET_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Traffic Class. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  TC validity for this entry. If TRUE, then when there are
   *  subnet matches, this TC is taken.
   */
  uint8 tc_is_valid;

} SOC_PPC_LLP_COS_PRTCL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Low limit of the ports range. Range: 0 - 65535.
   */
  uint16 start;
  /*
   *  High limit of the ports range. Range: 0 -
   *  65535.last_port >= first_port.
   */
  uint16 end;
  /*
   *  If TRUE, then there is match if the port is inside the
   *  range, i.e., port x matches if start<=x<= end. If FALSE,
   *  there is match if the port is outside the range, i.e., x
   *  > end or x <start.
   */
  uint8 in_range;

} SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO;


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
  SOC_PPC_LLP_COS_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_GLBL_INFO *info
  );

void
  SOC_PPC_LLP_COS_TC_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_TC_INFO *info
  );

void
  SOC_PPC_LLP_COS_UP_USE_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_UP_USE *info
  );

void
  SOC_PPC_LLP_COS_PORT_L2_TABLE_INDEXES_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_PORT_L2_TABLE_INDEXES *info
  );

void
  SOC_PPC_LLP_COS_PORT_L2_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_PORT_L2_INFO *info
  );

void
  SOC_PPC_LLP_COS_PORT_L3_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_PORT_L3_INFO *info
  );

void
  SOC_PPC_LLP_COS_PORT_L4_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_PORT_L4_INFO *info
  );

void
  SOC_PPC_LLP_COS_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_PORT_INFO *info
  );

void
  SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO *info
  );

void
  SOC_PPC_LLP_COS_IPV4_SUBNET_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_IPV4_SUBNET_INFO *info
  );

void
  SOC_PPC_LLP_COS_PRTCL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_PRTCL_INFO *info
  );

void
  SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LLP_COS_MAPPING_TABLE_to_string(
    SOC_SAND_IN  SOC_PPC_LLP_COS_MAPPING_TABLE enum_val
  );

void
  SOC_PPC_LLP_COS_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_GLBL_INFO *info
  );

void
  SOC_PPC_LLP_COS_TC_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_TC_INFO *info
  );

void
  SOC_PPC_LLP_COS_UP_USE_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_UP_USE *info
  );

void
  SOC_PPC_LLP_COS_PORT_L2_TABLE_INDEXES_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_PORT_L2_TABLE_INDEXES *info
  );

void
  SOC_PPC_LLP_COS_PORT_L2_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_PORT_L2_INFO *info
  );

void
  SOC_PPC_LLP_COS_PORT_L3_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_PORT_L3_INFO *info
  );

void
  SOC_PPC_LLP_COS_PORT_L4_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_PORT_L4_INFO *info
  );

void
  SOC_PPC_LLP_COS_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_PORT_INFO *info
  );

void
  SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_MAPPING_TABLE_ENTRY_INFO *info
  );

void
  SOC_PPC_LLP_COS_IPV4_SUBNET_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_IPV4_SUBNET_INFO *info
  );

void
  SOC_PPC_LLP_COS_PRTCL_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_PRTCL_INFO *info
  );

void
  SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_COS_L4_PORT_RANGE_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LLP_COS_INCLUDED__*/
#endif
