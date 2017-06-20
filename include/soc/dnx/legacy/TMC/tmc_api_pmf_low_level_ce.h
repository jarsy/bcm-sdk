/* $Id: jer2_jer2_jer2_tmc_api_pmf_low_level_ce.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/include/soc_jer2_jer2_jer2_tmcapi_pmf_low_level.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmcpmf
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

#ifndef __DNX_TMC_API_PMF_LOW_LEVEL_CE_INCLUDED__
/* { */
#define __DNX_TMC_API_PMF_LOW_LEVEL_CE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_ports.h>

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
   *  Points to the first bit of the first packet header once
   *  the packet parsed.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_0 = 0,
  /*
   *  Points to the first bit of the second packet header.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_1 = 1,
  /*
   *  Points to the first bit of the third packet header.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_2 = 2,
  /*
   *  Points to the first bit of the fourth packet header.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_3 = 3,
  /*
   *  Points to the first bit of the fifth packet header.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_4 = 4,
  /*
   *  Points to the first bit of the sixth packet header.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_5 = 5,
  /*
   *  Points to the first bit of the forwarding header.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_FWD = 6,
  /*
   *  Points to the first bit of the header after the
   *  forwarding header.
   */
  DNX_TMC_PMF_CE_SUB_HEADER_FWD_POST = 7,
  /*
   *  Number of types in DNX_TMC_PMF_CE_SUB_HEADER
   */
  DNX_TMC_NOF_PMF_CE_SUB_HEADERS = 8
}DNX_TMC_PMF_CE_SUB_HEADER;

typedef enum
{
  /*
   *  Local incoming TM port
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_TM_PORT = 0,
  /*
   *  The size of the incoming header
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_HEADER_SIZE = 1,
  /*
   *  Source System-Port ID
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_SYST_PORT_SRC = 2,
  /*
   *  Local incoming PP port
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_IN_PP_PORT = 3,
  /*
   *  PP Context
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PP_CONTEXT = 4,
  /*
   *  Pointer to Header offset 0
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_HEADER_OFFSET_0 = 5,
  /*
   *  Pointer to Header offset 1
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_HEADER_OFFSET_1 = 6,
  /*
   *  Pointer to Header offset 2
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_HEADER_OFFSET_2 = 7,
  /*
   *  Pointer to Header offset 3
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_HEADER_OFFSET_3 = 8,
  /*
   *  Pointer to Header offset 4
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_HEADER_OFFSET_4 = 9,
  /*
   *  Pointer to Header offset 5
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_HEADER_OFFSET_5 = 10,
  /*
   *  A coded value giving additional information for header 0
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_FORMAT_QUAL_0 = 11,
  /*
   *  A coded value giving additional information for header 1
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_FORMAT_QUAL_1 = 12,
  /*
   *  A coded value giving additional information for header 2
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_FORMAT_QUAL_2 = 13,
  /*
   *  A coded value giving additional information for header 3
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_FORMAT_QUAL_3 = 14,
  /*
   *  A coded value giving additional information for header 4
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_FORMAT_QUAL_4 = 15,
  /*
   *  A coded value giving additional information for header 5
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_FORMAT_QUAL_5 = 16,
  /*
   *  Extension of the Packet format Qualifier 1, in general
   *  Ethernet. This field holds the result of comparing
   *  Ethernet-Type to 6 values
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_FORMAT_QUAL1_EXT = 17,
  /*
   *  PMF profile resolved by the parser
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PARSER_PMF_PROFILE = 18,
  /*
   *  Identifies the packet header stack
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PACKET_FORMAT_CODE = 19,
  /*
   *  Used together with the VLAN-Edit-Profile to generate the
   *  VLAN edit command
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LLVP_INCOMING_TAG_STRUCT = 20,
  /*
   *  Strength of the forward action
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWD_ACTION_STRENGTH = 21,
  /*
   *  Destination (and its validity) of the forward action
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWD_ACTION_DEST_VALID = 22,
  /*
   *  Traffic Class (and its validity) of the forward action
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWD_ACTION_TC_VALID = 23,
  /*
   *  Drop Precedence (and its validity) of the forward action
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWD_ACTION_DP_VALID = 24,
  /*
   *  CPU Trap code of the Forwarding Action
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWD_ACTION_CPU_TRAP_CODE = 25,
  /*
   *  CPU Trap qualifier of the Forwarding Action
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWD_ACTION_CPU_TRAP_QUAL = 26,
  /*
   *  CPU Trap code profile
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_CPU_TRAP_CODE_PRO = 27,
  /*
   *  User-Priority of the Packet
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_UP = 28,
  /*
   *  Snoop strength
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_SNOOP_STRENGTH = 29,
  /*
   *  Snoop code
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_SNOOP_CODE = 30,
  /*
   *  Destination learnt of the Learn-Information
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LEARN_DEST = 31,
  /*
   *  ASD learnt of the Learn-Information
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LEARN_ASD = 32,
  /*
   *  SEM Index, aka AC
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_SEM_NDX = 33,
  /*
   *  LL-Mirror-Profile
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LL_MIRROR_PROFILE = 34,
  /*
   *  Virtual Switch Instance. Identifies topology (aka
   *  flooding domain) and/or service
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_VSI = 35,
  /*
   *  System VSI
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_SYSTEM_VSI = 36,
  /*
   *  Orientation=Is-Hub
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_ORIENTATION_IS_HUB = 37,
  /*
   *  VLAN Edit Tag
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_VLAN_EDIT_TAG = 38,
  /*
   *  State in the Spanning Tree Protocol (Learn, Block)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_STP_STATE = 39,
  /*
   *  Forwarding-Code (TM, CPU, Ethernet, IPv4 UC or MC, etc.)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWDING_CODE = 40,
  /*
   *  Forwarding Offset Index
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWDING_OFFSET_NDX = 41,
  /*
   *  Forwarding Offset Extension
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_FWDING_OFFSET_EXT = 42,
  /*
   *  The Tunnel-Termination-Code may be used to control
   *  various filtering, in the PMF, regarding the packet
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_TUNNEL_TERMINATION_CODE = 43,
  /*
   *  In-RIF
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_IN_RIF = 44,
  /*
   *  Local incoming TM port
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_VRF = 45,
  /*
   *  Packet-Is-Compatible-Multicast
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PCKT_IS_COMP_MC = 46,
  /*
   *  My-Backbone-MAC
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_MY_BMAC = 47,
  /*
   *  EEI-or-Out-LIF: this structure describes either an EEI
   *  or an Out-LIF
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_EEI_OR_OUTLIF = 48,
  /*
   *  In-TTL
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_IN_TTL = 49,
  /*
   *  In-DSCP-EXP
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_IN_DSCP_EXP = 50,
  /*
   *  RPF-FEC-Pointer (and its validity)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_RPF_FEC_PTR_VALID = 51,
  /*
   *  Small-EM-First-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_SEM_1ST_LOOKUP_RESULT = 52,
  /*
   *  Small-EM-Second-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_SEM_2ND_LOOKUP_RESULT = 53,
  /*
   *  Large-EM-First-Lkp-Result (and if found)
   */
   DNX_TMC_PMF_IRPP_INFO_FIELD_LEM_1ST_LOOKUP_RESULT_DEST = 54,
  /*
   *  Large-EM-Second-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LEM_2ND_LOOKUP_RESULT_DEST = 55,
  /*
   *  LPM-First-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LPM_1ST_LOOKUP_RESULT = 56,
  /*
   *  LPM-Second-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LPM_2ND_LOOKUP_RESULT = 57,
  /*
   *  TCAM-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_TCAM_LOOKUP_RESULT = 58,
  /*
   *  ELK-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_ELK_LOOKUP_RESULT = 59,
  /*
   *  In-PP-Port. Key-Gen-Var: generic input into the key
   *  generation engine
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_KEY_GEN_VAR = 60,
  /*
   *  Copy-Program-Variable
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_COPY_PGM_VARIABLE = 61,
  /*
   *  Packet-Size-Range
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PACKET_SIZE_RANGE = 62,
  /*
   *  L4Ops
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_L4_OPS = 63,
  /*
   *  Key-Program-Profile
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_KEY_PGM_PROFILE = 64,
  /*
   *  In-AC, Out-AC, Eth-Tag-Format
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_CUSTOM1 = 65,
  /*
   *  In-Port, Ethernet-Type-Code
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_CUSTOM2 = 66,
  /*
   *  My-B-MAC, Forwarding-Action DP, TC and Destination
   *  Valid, LEM, SEM, LPM and TCAM found,
   *  Packet-Is-Compatible-MC, RPF-FEC-Ptr-Valid,
   *  CPU-Trap-Code-Profile, Orientation-Is-Hub and Learn-Info
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_CUSTOM3 = 67,
  /*
   *  Local incoming TM port - 32b
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_TM_PORT32 = 68,
  /*
   *  8 / 16 / 32 '0's
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_ZERO_8 = 69,
  DNX_TMC_PMF_IRPP_INFO_FIELD_ZERO_16 = 70,
  DNX_TMC_PMF_IRPP_INFO_FIELD_ZERO_32 = 71,
  DNX_TMC_PMF_IRPP_INFO_FIELD_ONES_32 = 72,
  DNX_TMC_PMF_IRPP_INFO_FIELD_LEARN_INFO = 73,
  /*
   *  Large-EM-First-Lkp-Result (and if found)
   */
   DNX_TMC_PMF_IRPP_INFO_FIELD_LEM_1ST_LOOKUP_RESULT_ASD = 74,
  /*
   *  Large-EM-Second-Lkp-Result (and if found)
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_LEM_2ND_LOOKUP_RESULT_ASD = 75,
  /*
   *  Identifies the packet header stack MSB
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD_PACKET_FORMAT_CODE_2MSB = 76,

  /*
   *  Number of types in DNX_TMC_PMF_IRPP_INFO_FIELD
   */
  DNX_TMC_NOF_PMF_IRPP_INFO_FIELDS = 77
}DNX_TMC_PMF_IRPP_INFO_FIELD;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Sub-Header index: The packet header is divided to
   *  sub-header according to type (Ethernet/IPv4/IPv6/MPLS),
   *  indicates in which sub-header the field must be
   *  extracted from.
   */
  DNX_TMC_PMF_CE_SUB_HEADER sub_header;
  /*
   *  Internal-offset: The offset from the first bit of the
   *  sub-header, to be copied to the PMF input vector Units:
   *  Bits. Range: -256 - 252 (Soc_petra-B), in multiple of 4.
   */
  int32 offset;
  /*
   *  Number of bits to be copied to the PMF input
   *  vector. Units: Bits. Range: 0 - 32. (Soc_petra-B)
   */
  uint32 nof_bits;

} DNX_TMC_PMF_CE_PACKET_HEADER_INFO;


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
  DNX_TMC_PMF_CE_PACKET_HEADER_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PMF_CE_PACKET_HEADER_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_PMF_CE_SUB_HEADER_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_CE_SUB_HEADER enum_val
  );

const char*
  DNX_TMC_PMF_IRPP_INFO_FIELD_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_IRPP_INFO_FIELD enum_val
  );

void
  DNX_TMC_PMF_CE_PACKET_HEADER_INFO_print(
    DNX_SAND_IN  DNX_TMC_PMF_CE_PACKET_HEADER_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_PMF_LOW_LEVEL_CE_INCLUDED__*/
#endif

