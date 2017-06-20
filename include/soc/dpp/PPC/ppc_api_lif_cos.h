/* $Id: ppc_api_lif_cos.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_lif_cos.h
*
* MODULE PREFIX:  soc_ppc_lif
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

#ifndef __SOC_PPC_API_LIF_COS_INCLUDED__
/* { */
#define __SOC_PPC_API_LIF_COS_INCLUDED__

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
   *  No action. DP and TC are not modified.
   */
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE_NONE = 0,
  /*
   *  Set constant values for the TC and DP, regardless of the
   *  packet fields.
   */
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE_FORCE_ALWAYS = 1,
  /*
   *  If packet has VLAN tag, then map from Tag fields
   *  (UP/PCP, DEI, Tag-TPIDs). Otherwise, set to constant
   *  values.
   */
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE_MAP_IF_TAG_ELSE_FORCE = 2,
  /*
   *  If packet has IP header tag, then map from DSCP in the
   *  Header. Otherwise, set to constant values.
   */
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE_MAP_IF_IP_ELSE_FORCE = 3,
  /*
   *  If packet has VLAN tag, then map from Tag fields
   *  (UP/PCP, DEI, Tag-TPIDs). Otherwise, no change.
   */
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE_MAP_IF_TAG_ELSE_NONE = 4,
  /*
   *  If packet has IP header tag, then map from DSCP in the
   *  Header. Otherwise, no change.
   */
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE_MAP_IF_IP_ELSE_NONE = 5,
  /*
   *  Number of types in SOC_PPC_LIF_COS_AC_PROFILE_TYPE
   */
  SOC_PPC_NOF_LIF_COS_AC_PROFILE_TYPES = 6
}SOC_PPC_LIF_COS_AC_PROFILE_TYPE;

typedef enum
{
  /*
   *  No action. DP and TC are not modified.
   */
  SOC_PPC_LIF_COS_PWE_PROFILE_TYPE_NONE = 0,
  /*
   *  Set constant values for the TC and DP, regardless of the
   *  packet fields.
   */
  SOC_PPC_LIF_COS_PWE_PROFILE_TYPE_FORCE_ALWAYS = 1,
  /*
   *  Map the EXP field in the label
   */
  SOC_PPC_LIF_COS_PWE_PROFILE_TYPE_MAP = 2,
  /*
   *  Number of types in SOC_PPC_LIF_COS_PWE_PROFILE_TYPE
   */
  SOC_PPC_NOF_LIF_COS_PWE_PROFILE_TYPES = 3
}SOC_PPC_LIF_COS_PWE_PROFILE_TYPE;

typedef enum
{
  /*
   *  Opcode mapping is according to Layer3 header. The
   *  AC-offset is mapped according to IPv4/IPv6 TOS fieldWhen
   *  there is no IP header, it is according to the resolved
   *  TC and DP
   */
  SOC_PPC_LIF_COS_OPCODE_TYPE_L3 = 1,
  /*
   *  Opcode mapping is according to Layer2 header. The
   *  AC-offset is mapped according to the VLAN Tag
   */
  SOC_PPC_LIF_COS_OPCODE_TYPE_L2 = 2,
  /*
   *  Opcode mapping is according to the resolved Traffic
   *  Class and Drop Precedence
   */
  SOC_PPC_LIF_COS_OPCODE_TYPE_TC_DP = 4,
  /*
   *  Number of types in SOC_PPC_LIF_COS_OPCODE_TYPE
   */
  SOC_PPC_NOF_LIF_COS_OPCODE_TYPES = 8
}SOC_PPC_LIF_COS_OPCODE_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Profile type, whether to force to const values or to map
   *  from packet fields.
   */
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE type;
  /*
   *  Traffic Class value. Relevant only if the type has
   *  FORCE. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Drop Precedence value. Relevant only if the type has
   *  FORCE. Range: 0 - 3.
   */
  SOC_SAND_PP_DP dp;
  /*
   *  Selects table for mapping. Relevant only if the type has
   *  MAP. To fill this mapping tables use
   *  soc_ppd_lif_cos_profile_map_l2_info_set()/soc_ppd_lif_cos_profile_map_ip_info_set()
   *  Range: 1 - 15.
   */
  uint32 map_table;

} SOC_PPC_LIF_COS_AC_PROFILE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Profile type. Whether to force to const values or to map
   *  from packet fields.
   */
  SOC_PPC_LIF_COS_PWE_PROFILE_TYPE type;
  /*
   *  Traffic Class value. Relevant only if the type has
   *  FORCE. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Drop Precedence value. Relevant only if the type has
   *  FORCE. Range: 0 - 3.
   */
  SOC_SAND_PP_DP dp;
  /*
   *  Selects table for mapping. Relevant only if the type has
   *  MAP. In case mapping is used, use
   *  soc_ppd_lif_cos_profile_map_mpls_label_info_set() to set
   *  this mapping from EXP to TC/DP. Range: 0 - 3.
   */
  uint32 map_table;

} SOC_PPC_LIF_COS_PWE_PROFILE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  TRUE: Enable mapping from IP.
   *  soc_ppd_lif_cos_profile_map_ip_info_set() set the mapping of
   *  TOS(DSCP) to TC & DP according to the CoS-Profile
   */
  uint8 map_when_ip;
  /*
   *  TRUE: Enable mapping from MPLS.
   *  soc_ppd_lif_cos_profile_map_mpls_info_set() set the mapping
   *  of EXP to TC & DP according to the CoS-Profile
   */
  uint8 map_when_mpls;
  /*
   *  TRUE: Enable mapping from L2 VLAN tag.
   *  soc_ppd_lif_cos_profile_map_l2_info_set() set the mapping of
   *  Tag type, UP/PCP and DEI to TC & DP according to the
   *  CoS-Profile
   */
  uint8 map_when_l2;
  /*
   *  TRUE: Enable mapping from TC & DP.
   *  soc_ppd_lif_cos_profile_map_tc_dp_info_set() set the mapping
   *  of TC & DP to new TC & DP according to the CoS-Profile
   */
  uint8 map_from_tc_dp;
  /*
   *  Forced TC, when mapping is disabled, from the specific
   *  header type.
   */
  SOC_SAND_PP_TC forced_tc;
  /*
   *  Forced DP, when mapping is disabled, from the specific
   *  header type.
   */
  SOC_SAND_PP_DP forced_dp;
  /*
   * TRUE: Enable mapping from Outer-VLAN.PCP to In-DSCP-EXP before remark.
   * Done between Tunnel termination and Forwarding layer handling.
   * Only valid for the first 16 COS-profiles.
   * Valid for ARAD Plus and above.
   * Note: Device assume all packets are tagged.
   */
  uint8 use_layer2_pcp;

} SOC_PPC_LIF_COS_PROFILE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Traffic Class value. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Drop Precedence value. Range: 0 - 3.
   */
  SOC_SAND_PP_DP dp;
  /*
   *  Dscp value. Range: 0 - 255.
   *  ARAD only.
   */
  uint32 dscp;

} SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IP packet type IPv4 or IPv6. In Soc_petra has to be either
   *  SOC_SAND_PP_IP_TYPE_IPV6 or SOC_SAND_PP_IP_TYPE_IPV4. In T20E
   *  has to be SOC_SAND_PP_IP_TYPE_ALL.
   */
  SOC_SAND_PP_IP_TYPE ip_type;
  /*
   *  TOS (Terms of service ) in the header of IP, (refer to
   *  TC in IPv6 Header)
   */
  SOC_SAND_PP_IPV4_TOS tos;
  /*DSCP-EXP value before remark and CoS profile -> DSCP-EXP remark */
  SOC_SAND_PP_IPV4_TOS dscp_exp;
} SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Incoming EXP in the label of the packet.
   */
  SOC_SAND_PP_MPLS_EXP in_exp;

} SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  In Petra-B:
   *  The outer TPID on the packet could be 0 - for None 1 -
   *  port outer TPID2 - port inner TPID3 - ISID - TPID
   *  In ARAD:
   *  outer_tpid means the incoming_tag_index
   *  0 - outer_tag, 1 - inner_tag
   */
  uint8 outer_tpid;
  /*
   *  Incoming UP/PCP on the VLAN tag of the packet
   */
  SOC_SAND_PP_PCP_UP incoming_up;
  /*
   *  Incoming DEI on the packet. If the packet has C-Tag,
   *  this should be zero
   */
  SOC_SAND_PP_DEI_CFI incoming_dei;

} SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Traffic Class (0-7)
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Drop Precedence value. (0-3)
   */
  SOC_SAND_PP_DP dp;

} SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  When FALSE, the packet is invalid. For example, packets
   *  may be allowed on a specific port*VLAN only with
   *  specific UP/PCP values. In T20E has to be TRUE.
   */
  uint8 is_packet_valid;
  /*
   *  When TRUE, the packet Logical Interface ID is not
   *  affected by the 'ac_offset', but it is affecting the QoS
   *  related proccessnig. Soc_petra-B only, in T20E has to be
   *  FALSE.
   */
  uint8 is_qos_only;
  /*
   *  AC-Offset. The AC ID is Base-LIF-ID + the AC-Offset. The
   *  updated AC ID may affect the QoS processing, or the
   *  entire processing. E. G, when 'is_qos_only' is negated,
   *  and AC learning is supported, the AC that to be learn on
   *  the remote devices is Base-LIF-ID + the AC-Offset. In
   *  T20E AC ID always affects the entire process and
   *  is_qos_only has to be FALSE.
   */
  uint8 ac_offset;

} SOC_PPC_LIF_COS_OPCODE_ACTION_INFO;


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
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_AC_PROFILE_INFO *info
  );

void
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_PWE_PROFILE_INFO *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_INFO *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY *info
  );

void
  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_COS_OPCODE_ACTION_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LIF_COS_AC_PROFILE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_LIF_COS_AC_PROFILE_TYPE enum_val
  );

const char*
  SOC_PPC_LIF_COS_PWE_PROFILE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PWE_PROFILE_TYPE enum_val
  );

const char*
  SOC_PPC_LIF_COS_OPCODE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_TYPE enum_val
  );

void
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_AC_PROFILE_INFO *info
  );

void
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PWE_PROFILE_INFO *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_INFO *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY *info
  );

void
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY *info
  );

void
  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LIF_COS_INCLUDED__*/
#endif

