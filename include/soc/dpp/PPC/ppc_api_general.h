/* $Id: ppc_api_general.h,v 1.38 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_PPC_API_GENERAL_INCLUDED__
/* { */
#define __SOC_PPC_API_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_PPC_DEBUG                               (SOC_SAND_DEBUG)
#define SOC_PPC_DEBUG_IS_LVL1                       (SOC_PPC_DEBUG >= SOC_SAND_DBG_LVL1)
#define SOC_PPC_DEBUG_IS_LVL2                       (SOC_PPC_DEBUG >= SOC_SAND_DBG_LVL2)
#define SOC_PPC_DEBUG_IS_LVL3                       (SOC_PPC_DEBUG >= SOC_SAND_DBG_LVL3)

#define SOC_PPC_VLAN_TAGS_MAX                         2




#define SOC_PPC_EEI_IDENTIFIER_POP_VAL                       (8)
#define SOC_PPC_EEI_IDENTIFIER_SWAP_VAL                      (9)
#define SOC_PPC_EEI_IDENTIFIER_TRILL_VAL                     (10)
#define SOC_PPC_EEI_IDENTIFIER_ENCAP_VAL                     (15)
#define SOC_PPC_EEI_IDENTIFIER_ENCAP_VAL_2_MSBS               (3)

#define SOC_PPC_RIF_NULL  (0xffffffff)
#define SOC_PPC_EEP_NULL  (0xffffffff)

#define SOC_PPC_MAX_NOF_LOCAL_PORTS_PETRA (64)
#define SOC_PPC_MAX_NOF_LOCAL_PORTS_ARAD  (256)
#define SOC_PPC_MAX_NOF_ADDITIONAL_TPID_VALS (4)

/* } */
/*************
 * MACROS    *
 *************/
/* { */



#define SOC_PPC_IP_ROUTING_TABLE_ITER_IS_END(iter) \
          ((((iter)->arr[0] == SOC_SAND_U32_MAX) && (((iter)->arr[1] == SOC_SAND_U32_MAX))))

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* $Id: ppc_api_general.h,v 1.38 Broadcom SDK $
 *  Local tm port. Range : 0 - 63.
 */
typedef uint32 SOC_PPC_TM_PORT;


/*
 *  Local PP port. Range : 0 - 63.
 */
typedef uint32 SOC_PPC_PORT;


/*
 *  Filtering ID. Range: Soc_petraB: 0 - 16K-1. T20E: 0-64K-1.
 */
typedef uint32 SOC_PPC_FID;


/*
 *  Virtual switch instance ID. Range: 0 - 16K-1.
 */
typedef uint32 SOC_PPC_VSI_ID;


/*
 *  System VSI. Range: 0 - 64K-1.
 */
typedef uint32 SOC_PPC_SYS_VSI_ID;


/*
 *  Forwarding Equivalence Class ID. Range: 0 - 16K-1.
 */
typedef uint32 SOC_PPC_FEC_ID;


/*
 *  Virtual Router ID. Range: 1 - 255.
 */
typedef uint32 SOC_PPC_VRF_ID;


/*
 *  Attachment Circuit ID. Range: Soc_petraB: 0 - 16K-1. T20E: 0
 *  - 64K-1.
 */
typedef uint32 SOC_PPC_AC_ID;

/*
 *	Designates invalid/non-existing AC-id
 */
#define SOC_PPC_AC_ID_INVALID 	(0xFFFFFFFF)

/*
 *	Designates invalid/non-existing VRF
 */
#define SOC_PPC_VRF_INVALID 	(0xFFFFFFFF)

/*
 *  Router Interface ID. Range: 0 - 4K-1.
 */
typedef uint32 SOC_PPC_RIF_ID;


/*
 *  Logical Interface ID. Range: Soc_petraB: 0 - 16K-1. T20E: 0
 *  - 64K-1.
 */
typedef uint32 SOC_PPC_LIF_ID;


/*
 *  MP Level.
 */
typedef uint32 SOC_PPC_MP_LEVEL;

/*
 *  E-CID Port Extender value.
 */
typedef uint32 SOC_SAND_PP_ECID;


typedef enum
{
  /*
   *  No EEI is used; ingress PP will not transmit EEI in the
   *  header. EEI value is not relevant in this case.
   */
  SOC_PPC_EEI_TYPE_EMPTY = 0,
  /*
   *  The EEI includes TRILL information (Dist-Nick and
   *  multicast indication). The TRILL info is relevant in
   *  this case.
   */
  SOC_PPC_EEI_TYPE_TRILL = 1,
  /*
   *  EEI used as mpls command. To pop/push/swap VC/Tunnel
   *  Label. The MPLS info is relevant in this case. When
   *  using the EEI for mpls command, then push_profile 0
   *  cannot be used.
   */
  SOC_PPC_EEI_TYPE_MPLS = 2,
  /*
   *  The EEI is used as ISID for MAC in MAC application. In
   *  this case, the destination port that the EEI/ISID is
   *  transmitted to must be a PBP port. Otherwise,
   *  mis-configuration and behavior is unexpected. The ISID
   *  field is relevant in this case.
   */
  SOC_PPC_EEI_TYPE_MIM = 3,
  /*
   *  Number of types in SOC_PPC_EEI_TYPE for PB
   */
  SOC_PPC_NOF_EEI_TYPES_PB = 4,
  /*
   *  outlif: the EEI include encapsulation pointer
   *  that points to egress ENCAP DB.
   */
  SOC_PPC_EEI_TYPE_OUTLIF = 4,
  /*
   *  all information already set in EEI val.
   */
  SOC_PPC_EEI_TYPE_RAW = 5,
  /*
   *  Number of types in SOC_PPC_EEI_TYPE for PB
   */
  SOC_PPC_NOF_EEI_TYPES_ARAD = 6
}SOC_PPC_EEI_TYPE;

typedef enum
{
  /*
   *  No Out-LIF is used.
   */
  SOC_PPC_OUTLIF_ENCODE_TYPE_NONE = 0,
  /*
   *  The Out-LIF encoding type is AC or EEP. Relevant only
   *  for T20E.
   */
  SOC_PPC_OUTLIF_ENCODE_TYPE_RAW = 1,
  /*
   *  The Out-LIF encoding type is AC. Egress Processing is
   *  done according to the AC database.
   */
  SOC_PPC_OUTLIF_ENCODE_TYPE_AC = 2,
  /*
   *  The Out-LIF encoding type is EEP. The Out-LIF may be a
   *  PWE, MinM interface, RIF-Tunnel or RIF-Trill. Egress
   *  Processing is done according to the Egress Editing
   *  database. Valid for bridging, VPLS, Mac-in-Mac, TRILL and
   *  Routing applications.
   */
  SOC_PPC_OUTLIF_ENCODE_TYPE_EEP = 3,
  /*
   *  The Out-LIF encoding type is VSI. The Out-LIF is
   *  RIF-VSI. Valid for IP-Routing
   */
  SOC_PPC_OUTLIF_ENCODE_TYPE_VSI = 4,
  /*
   *  The Out-LIF encoding type is RAW,
   *  and the valid bit will be reset.
   */
  SOC_PPC_OUTLIF_ENCODE_TYPE_RAW_INVALID = 5,
  /*
   *  Number of types in SOC_PPC_OUTLIF_ENCODE_TYPE
   */
  SOC_PPC_NOF_OUTLIF_ENCODE_TYPES = 6
}SOC_PPC_OUTLIF_ENCODE_TYPE;

typedef enum
{
  /*
   *  Drop the packet.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_DROP = 0,
  /*
   *  Unicast forwarding without a FEC.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_UC_FLOW = 1,
  /*
   *  Unicast forwarding without a FEC.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_UC_LAG = 2,
  /*
   *  Unicast forwarding without a FEC.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_UC_PORT = 3,
  /*
   *  Multicast forwarding without a FEC.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_MC = 4,
  /*
   *  Forwarding via a FEC. May be either Unicast forwarding or
   *  Multicast forwarding.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_FEC = 5,
  /*
   *  Trapping to the control plane. This type can be only
   *  upon lookup in the MACT/ILM/FEC. Soc_petra-B only.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_TRAP = 6,
  /*
   *  Number of types in SOC_PPC_FRWRD_DECISION_TYPE
   */
  SOC_PPC_NOF_FRWRD_DECISION_TYPES = 7
}SOC_PPC_FRWRD_DECISION_TYPE;

#define SOC_PPC_MPLS_COMMAND_TYPE_PUSH                     SOC_TMC_MPLS_COMMAND_TYPE_PUSH
#define SOC_PPC_MPLS_COMMAND_TYPE_POP                      SOC_TMC_MPLS_COMMAND_TYPE_POP
#define SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_PIPE       SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_PIPE
#define SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_UNIFORM    SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_UNIFORM
#define SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_PIPE       SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_PIPE
#define SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_UNIFORM    SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_UNIFORM
#define SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_PIPE       SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_PIPE
#define SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_UNIFORM    SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_UNIFORM
#define SOC_PPC_MPLS_COMMAND_TYPE_POP_INTO_ETHERNET        SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_ETHERNET
#define SOC_PPC_MPLS_COMMAND_TYPE_SWAP                     SOC_TMC_MPLS_COMMAND_TYPE_SWAP
#define SOC_PPC_NOF_MPLS_COMMAND_TYPES                     SOC_TMC_NOF_MPLS_COMMAND_TYPES
typedef SOC_TMC_MPLS_COMMAND_TYPE                          SOC_PPC_MPLS_COMMAND_TYPE;

typedef enum
{
  /*
   *  MAC-in-MAC. MAC in MAC header will be identified
   *  according special TPID, ISID-TPID.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_MAC_IN_MAC = 0x88E7,
  /*
   *  Trill packet.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_TRILL = 0x22F3,
  /*
   *  IPv4 packet.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_IPV4 = 0x800,
  /*
   *  IPv6 packet
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_IPV6 = 0x86DD,
  /*
   *  ARP packet.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_ARP = 0x806,
  /*
   *  CFM (Connectivity Fault Management) packet.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_CFM = 0x8902,
  /*
   *  MPLS packet.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_MPLS = 0x8847,
  /*
   *  Fiber Channel over Ethernet
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_FC_ETH = 0x8096,
  /*
   *  Other Next protocol type, not one of the above.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE_OTHER = 0x0,
  /*
   *  Number of types in SOC_PPC_L2_NEXT_PRTCL_TYPE
   */
  SOC_PPC_NOF_L2_NEXT_PRTCL_TYPES = 11
}SOC_PPC_L2_NEXT_PRTCL_TYPE;

typedef enum
{
  /*
   *  None of the listed protocol types, useful to indicate
   *  that packet next protocol doesn't match any of the
   *  configured protocol types.
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_NONE = 0x0,
  /*
   *  Next header is TCP. i.e. TCP over IP
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_TCP = 0x6,
  /*
   *  Next header is TCP. i.e. UDP over IP
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_UDP = 0x11,
  /*
   *  Next header is IGMP
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_IGMP = 0x2,
  /*
   *  Next header is ICMP
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_ICMP = 0x1,
  /*
   *  Next header is ICMPv6
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_ICMPV6 = 0x3A,
  /*
   *  Next header is IPv4: IP in IP (encapsulation)
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_IPV4 = 0x4,
  /*
   *  Next header is IPv6, encapsulated IPv6.
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_IPV6 = 0x29,
  /*
   *  Next header is MPLS: MPLS-in-IP
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE_MPLS = 0x89,
  /*
   *  Number of types in SOC_PPC_L3_NEXT_PRTCL_TYPE
   */
  SOC_PPC_NOF_L3_NEXT_PRTCL_TYPES = 9
}SOC_PPC_L3_NEXT_PRTCL_TYPE;


typedef enum
{
  /*
   *  No header is terminated (Bridging)
   */
  SOC_PPC_PKT_TERM_TYPE_NONE = 0,
  /*
   *  Most outer LL header wasterminated (Routing, MAC-in-MAC,
   *  VPLS)
   */
  SOC_PPC_PKT_TERM_TYPE_ETH = 1,
  /*
   *  Link-Layer and IPv4 tunnel were terminated
   */
  SOC_PPC_PKT_TERM_TYPE_IPV4_ETH = 3,
  /*
   *  Link-Layer and MPLS tunnel were terminated
   */
  SOC_PPC_PKT_TERM_TYPE_MPLS_ETH = 4,
  /*
   *  Link-Layer and MPLS (with Control Word) tunnel were
   *  terminated
   */
  SOC_PPC_PKT_TERM_TYPE_CW_MPLS_ETH = 5,
  /*
   *  Link-Layer and MPLSx2 tunnel were terminated
   */
  SOC_PPC_PKT_TERM_TYPE_MPLS2_ETH = 6,
  /*
   *  Link-Layer and MPLSx2 (with Control Word) tunnel were
   *  terminated
   */
  SOC_PPC_PKT_TERM_TYPE_CW_MPLS2_ETH = 7,
  /*
   *  Link-Layer and MPLSx3 tunnel were terminated
   */
  SOC_PPC_PKT_TERM_TYPE_MPLS3_ETH = 8,
  /*
   *  Link-Layer and MPLSx3 (with Control Word) tunnel were
   *  terminated
   */
  SOC_PPC_PKT_TERM_TYPE_CW_MPLS3_ETH = 9,
  /*
   *  Link-Layer TRILL tunnel were terminated
   */
  SOC_PPC_PKT_TERM_TYPE_TRILL = 10,
  /*
   *  Link-Layer and IPv6 tunnel were terminated.
   *  Arad-only
   */
  SOC_PPC_PKT_TERM_TYPE_IPV6_ETH = 11,
  /*
   *  Number of types in SOC_PPC_PKT_TERM_TYPE
   */
  SOC_PPC_NOF_PKT_TERM_TYPES = 11
}SOC_PPC_PKT_TERM_TYPE;

/*
 *	Defined in the TM part for the PMF module
 */
#define SOC_PPC_PKT_FRWRD_TYPE_BRIDGE                 SOC_TMC_PKT_FRWRD_TYPE_BRIDGE
#define SOC_PPC_PKT_FRWRD_TYPE_IPV4_UC                SOC_TMC_PKT_FRWRD_TYPE_IPV4_UC
#define SOC_PPC_PKT_FRWRD_TYPE_IPV4_MC                SOC_TMC_PKT_FRWRD_TYPE_IPV4_MC
#define SOC_PPC_PKT_FRWRD_TYPE_IPV6_UC                SOC_TMC_PKT_FRWRD_TYPE_IPV6_UC
#define SOC_PPC_PKT_FRWRD_TYPE_IPV6_MC                SOC_TMC_PKT_FRWRD_TYPE_IPV6_MC
#define SOC_PPC_PKT_FRWRD_TYPE_MPLS                   SOC_TMC_PKT_FRWRD_TYPE_MPLS
#define SOC_PPC_PKT_FRWRD_TYPE_TRILL                  SOC_TMC_PKT_FRWRD_TYPE_TRILL
#define SOC_PPC_PKT_FRWRD_TYPE_CPU_TRAP               SOC_TMC_PKT_FRWRD_TYPE_CPU_TRAP
#define SOC_PPC_PKT_FRWRD_TYPE_BRIDGE_AFTER_TERM      SOC_TMC_PKT_FRWRD_TYPE_BRIDGE_AFTER_TERM
#define SOC_PPC_PKT_FRWRD_TYPE_CUSTOM1                 SOC_TMC_PKT_FRWRD_TYPE_CUSTOM1
#define SOC_PPC_PKT_FRWRD_TYPE_TM                     SOC_TMC_PKT_FRWRD_TYPE_TM
#define SOC_PPC_NOF_PKT_FRWRD_TYPES                   SOC_TMC_NOF_PKT_FRWRD_TYPES
typedef SOC_TMC_PKT_FRWRD_TYPE                        SOC_PPC_PKT_FRWRD_TYPE;

typedef enum
{
  /*
   *  no network header
   */
  SOC_PPC_PKT_HDR_TYPE_NONE = 0,
  /*
   *  Ethernet header
   */
  SOC_PPC_PKT_HDR_TYPE_ETH = 1,
  /*
   *  IPv4 header
   */
  SOC_PPC_PKT_HDR_TYPE_IPV4 = 2,
  /*
   *  IPv6 over Ethernet
   */
  SOC_PPC_PKT_HDR_TYPE_IPV6 = 3,
  /*
   *  MPLS header
   */
  SOC_PPC_PKT_HDR_TYPE_MPLS = 4,
  /*
   *  TRILL header
   */
  SOC_PPC_PKT_HDR_TYPE_TRILL = 5,
  /*
   *  Number of types in SOC_PPC_PKT_HDR_TYPE
   */
  SOC_PPC_NOF_PKT_HDR_TYPES = 6
}SOC_PPC_PKT_HDR_TYPE;

typedef enum
{
  /*
   *  Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_ETH = 0x1,
  /*
   *  MAC-in-MAC
   */
  SOC_PPC_PKT_HDR_STK_TYPE_ETH_ETH = 0X11,
  /*
   *  IPv4 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV4_ETH = 0x21,
  /*
   *  IPv4 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_ETH_IPV4_ETH = 0x121,
  /*
   *  IPv6 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV6_ETH = 0x31,
  /*
   *  MPLS over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_MPLS1_ETH = 0x41,
  /*
   *  FC with Encap over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_FC_ENCAP_ETH = 0x51,
  /*
   *  FC without Encap over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_FC_STD_ETH = 0x61,
  /*
   *  MPLS x 2 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_MPLS2_ETH = 0x441,
  /*
   *  MPLS x 3 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_MPLS3_ETH = 0x4441,
  /*
   *  Ethernet over TRILL over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_ETH_TRILL_ETH = 0x151,
  /*
   *  Ethernet over MPLS over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_ETH_MPLS1_ETH = 0x141,
  /*
   *  Ethernet over MPLSx2 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_ETH_MPLS2_ETH = 0x1441,
  /*
   *  Ethernet over MPLSx3 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_ETH_MPLS3_ETH = 0x14441,
  /*
   *  IPv4 over IPv4 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV4_IPV4_ETH = 0x221,
  /*
   *  IPv4 over MPLS over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV4_MPLS1_ETH = 0x241,
  /*
   *  IPv4 over MPLSx2 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV4_MPLS2_ETH = 0x2441,
  /*
   *  IPv4 over MPLSx3 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV4_MPLS3_ETH = 0x24441,
  /*
   *  IPv6 over IPv4 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV6_IPV4_ETH = 0x321,
  /*
   *  IPv6 over MPLS over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV6_MPLS1_ETH = 0x341,
  /*
   *  IPv6 over MPLSx2 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV6_MPLS2_ETH = 0x3441,
  /*
   *  IPv6 over MPLSx3 over Ethernet
   */
  SOC_PPC_PKT_HDR_STK_TYPE_IPV6_MPLS3_ETH = 0x34441,
  /*
   *  Number of types in SOC_PPC_PKT_HDR_STK_TYPE
   */
  SOC_PPC_NOF_PKT_HDR_STK_TYPES = 19
}SOC_PPC_PKT_HDR_STK_TYPE;

typedef enum
{
  /*
   *  According to this type the iterator, traverse the table
   *  unordered, but it provides an efficient traverse of the
   *  table.
   */
  SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE_FAST = 0,
  /*
   *  According to this type the iterator, traverse the table
   *  ordered according to (IP, Prefix)-slower than the fast
   *  type.
   */
  SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE_IP_PREFIX_ORDERED = 1,
  /*
   *  According to this type the iterator, traverse the table
   *  ordered according to (Prefix, IP)-slower than the
   *  previous types (fast and (IP, Prefix)).
   */
  SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE_PREFIX_IP_ORDERED = 2,
  /*
   *  Number of types in SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE
   */
  SOC_PPC_NOF_IP_ROUTING_TABLE_ITER_TYPES = 3
}SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE;

typedef enum
{
  /*
   *
   */
  SOC_PPC_HASH_MASKS_MAC_SA = 0x1,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_MAC_DA = 0x2,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_VSI = 0x4,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_ETH_TYPE_CODE = 0x8,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_MPLS_LABEL_1 = 0x10,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_MPLS_LABEL_2 = 0x20,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_MPLS_LABEL_3 = 0x40,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_IPV4_SIP = 0x80,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_IPV4_DIP = 0x100,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_IPV4_PROTOCOL = 0x200,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_IPV6_SIP = 0x400,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_IPV6_DIP = 0x800,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_IPV6_PROTOCOL = 0x1000,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_L4_SRC_PORT = 0x2000,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_L4_DEST_PORT = 0x4000,
  /*
   *  Fibre channel. (FCoE packets)
   */
  SOC_PPC_HASH_MASKS_FC_DEST_ID = 0x8000,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_FC_SRC_ID = 0x10000,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_FC_SEQ_ID = 0x20000,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_FC_ORG_EX_ID = 0x40000,
  /*
   *
   */
  SOC_PPC_HASH_MASKS_FC_RES_EX_ID = 0x80000,
  /*
   * FCoE, VFT VFI - ARAD Only
   */
  SOC_PPC_HASH_MASKS_FC_VFI = 0x100000,
  /*
   * Trill Egress-Nickname
   */
  SOC_PPC_HASH_MASKS_TRILL_EG_NICK = 0x200000,
  /*
   * MPLS label 4 for 5 or more label packets
   */
  SOC_PPC_HASH_MASKS_MPLS_LABEL_4 = 0x400000,
  /*
   * MPLS label 5 for 5 or more label packets
   */
  SOC_PPC_HASH_MASKS_MPLS_LABEL_5 = 0x800000,
  /*
   * Number of types in SOC_PPC_HASH_MASKS
   */
  SOC_PPC_NOF_HASH_MASKS_PB = 21,
  SOC_PPC_NOF_HASH_MASKS = 24
}SOC_PPC_HASH_MASKS;

/* Reserved for OAM user defined Trap Codes */
/* OAM ingress trap codes */
#define SOC_PPC_TRAP_CODE_OAM_CPU  /* SOC_PPC_TRAP_CODE_USER_DEFINED_38 - 265 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 5 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)
#define SOC_PPC_TRAP_CODE_OAM_CPU_SNOOP  /* SOC_PPC_TRAP_CODE_USER_DEFINED_39 - 266 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 4 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)
#define SOC_PPC_TRAP_CODE_OAM_RECYCLE  /* SOC_PPC_TRAP_CODE_USER_DEFINED_40 - 267 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 3 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)
#define SOC_PPC_TRAP_CODE_OAM_CPU_UP  /* SOC_PPC_TRAP_CODE_USER_DEFINED_41 - 268 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 2 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)
#define SOC_PPC_TRAP_CODE_OAM_MIP_EGRESS_SNOOP_WITH_FTMH  /* SOC_PPC_TRAP_CODE_USER_DEFINED_33 - 260 */\
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 10 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)



/*    ITMH snoop 0-15     */

#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_0     SOC_PPC_TRAP_CODE_USER_DEFINED_44  /* SOC_PPC_TRAP_CODE_USER_DEFINED_44  - 208 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_1     SOC_PPC_TRAP_CODE_USER_DEFINED_45  /* SOC_PPC_TRAP_CODE_USER_DEFINED_45  - 209 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_2     SOC_PPC_TRAP_CODE_USER_DEFINED_46  /* SOC_PPC_TRAP_CODE_USER_DEFINED_46  - 210 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_3     SOC_PPC_TRAP_CODE_USER_DEFINED_47  /* SOC_PPC_TRAP_CODE_USER_DEFINED_47  - 211 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_4     SOC_PPC_TRAP_CODE_USER_DEFINED_48  /* SOC_PPC_TRAP_CODE_USER_DEFINED_48  - 212 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_5     SOC_PPC_TRAP_CODE_USER_DEFINED_49  /* SOC_PPC_TRAP_CODE_USER_DEFINED_49  - 213 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_6     SOC_PPC_TRAP_CODE_USER_DEFINED_50  /* SOC_PPC_TRAP_CODE_USER_DEFINED_50  - 214 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_7     SOC_PPC_TRAP_CODE_USER_DEFINED_51  /* SOC_PPC_TRAP_CODE_USER_DEFINED_51  - 215 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_8     SOC_PPC_TRAP_CODE_USER_DEFINED_52  /* SOC_PPC_TRAP_CODE_USER_DEFINED_52  - 216 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_9     SOC_PPC_TRAP_CODE_USER_DEFINED_53  /* SOC_PPC_TRAP_CODE_USER_DEFINED_53  - 217 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_10    SOC_PPC_TRAP_CODE_USER_DEFINED_54  /* SOC_PPC_TRAP_CODE_USER_DEFINED_54  - 218 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_11    SOC_PPC_TRAP_CODE_USER_DEFINED_55  /* SOC_PPC_TRAP_CODE_USER_DEFINED_55  - 219 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_12    SOC_PPC_TRAP_CODE_USER_DEFINED_56  /* SOC_PPC_TRAP_CODE_USER_DEFINED_56  - 220 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_13    SOC_PPC_TRAP_CODE_USER_DEFINED_57  /* SOC_PPC_TRAP_CODE_USER_DEFINED_57  - 221 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_14    SOC_PPC_TRAP_CODE_USER_DEFINED_58  /* SOC_PPC_TRAP_CODE_USER_DEFINED_58  - 222 */
#define SOC_PPC_TRAP_CODE_ITMH_SNOOP_15    SOC_PPC_TRAP_CODE_USER_DEFINED_59  /* SOC_PPC_TRAP_CODE_USER_DEFINED_59  - 223 */

#define SOC_PPC_TRAP_CODE_SAT_0            SOC_PPC_TRAP_CODE_USER_DEFINED_26  /* SOC_PPC_TRAP_CODE_USER_DEFINED_26 - 204 */
#define SOC_PPC_TRAP_CODE_SAT_1            SOC_PPC_TRAP_CODE_USER_DEFINED_25  /* SOC_PPC_TRAP_CODE_USER_DEFINED_25 - 203 */
#define SOC_PPC_TRAP_CODE_SAT_2            SOC_PPC_TRAP_CODE_USER_DEFINED_24  /* SOC_PPC_TRAP_CODE_USER_DEFINED_24 - 202 */

/* OAM upmep trap codes: FTMH header WA: need to be 4 consecutive user-header traps */
#define SOC_PPC_TRAP_CODE_OAM_CPU_MIRROR  /* SOC_PPC_TRAP_CODE_USER_DEFINED_34 - 261 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 9 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)
#define SOC_PPC_TRAP_CODE_OAM_OAMP_MIRROR  /* SOC_PPC_TRAP_CODE_USER_DEFINED_35 - 262 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 8 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)
#define SOC_PPC_TRAP_CODE_OAM_CPU_SNOOP_UP  /* SOC_PPC_TRAP_CODE_USER_DEFINED_36 - 263 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 7 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)
#define SOC_PPC_TRAP_CODE_OAM_CPU_FREE_UP  /* SOC_PPC_TRAP_CODE_USER_DEFINED_37 - 264 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 6 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)

/* Number of already used trap codes from those allocated for upmep with ftmh header - cpu and oamp */
#define SOC_PPC_TRAP_CODE_OAM_FTMH_MIRROR_TRAP_CODES_FIRST (SOC_PPC_TRAP_CODE_OAM_CPU_MIRROR)
#define SOC_PPC_TRAP_CODE_OAM_FTMH_MIRROR_TRAP_CODES_NUM (4)
#define SOC_PPC_TRAP_CODE_OAM_FTMH_MIRROR_TRAP_CODES_MIP_EGRESS_SNOOP (SOC_PPC_TRAP_CODE_OAM_FTMH_MIRROR_TRAP_CODES_FIRST -1)

#define SOC_PPC_TRAP_CODE_OAM_IS_FTMH_MIRROR(trap_code)  ((trap_code>=SOC_PPC_TRAP_CODE_OAM_FTMH_MIRROR_TRAP_CODES_FIRST) && \
                                                          (trap_code<=(SOC_PPC_TRAP_CODE_OAM_FTMH_MIRROR_TRAP_CODES_FIRST + SOC_PPC_TRAP_CODE_OAM_FTMH_MIRROR_TRAP_CODES_NUM)))

#define SOC_PPC_TRAP_CODE_FCOE_WA_FIX_OFFSET       (SOC_PPC_TRAP_CODE_USER_DEFINED_19)/*246*/

#define SOC_PPC_TRAP_CODE_IPV6_UC_RPF_2PASS  /* SOC_PPC_TRAP_CODE_USER_DEFINED_32 - 259 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 11 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)

#define SOC_PPC_TRAP_CODE_BFD_ECHO  /* SOC_PPC_TRAP_CODE_USER_DEFINED_31 - 258 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 12 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)


#define SOC_PPC_TRAP_CODE_BFD_IPV4_YOUR_DISCR_NOT_FOUND  /* SOC_PPC_TRAP_CODE_USER_DEFINED_30 - 257 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 13 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM)

#define SOC_PPC_TRAP_CODE_BFD_IPV6_YOUR_DISCR_NOT_FOUND  SOC_PPC_TRAP_CODE_USER_DEFINED_29

#define SOC_PPC_TRAP_CODE_TRAP_BFD_O_IPV6   SOC_PPC_TRAP_CODE_USER_DEFINED_28

#define SOC_PPC_TRAP_CODE_SER    SOC_PPC_TRAP_CODE_USER_DEFINED_23      
#define SOC_PPC_TRAP_CODE_SER_IS_SUPPORTED(unit)    (SOC_IS_JERICHO_PLUS(unit)) /*Supported in QAX QUX and J+*/


#define SOC_PPC_TRAP_CODE_BFD_IPV4_IPV6_YOUR_DISCR_NOT_FOUND SOC_PPC_TRAP_CODE_BFD_IPV4_YOUR_DISCR_NOT_FOUND

typedef enum
{
  /*
   *  SA MACT lookup result's sa_drop is TRUE
   */   
  SOC_PPC_TRAP_CODE_PBP_SA_DROP_0,
  /*
   *  SA MACT lookup result's sa_drop is TRUE
   */
  SOC_PPC_TRAP_CODE_PBP_SA_DROP_1,
  /*
   *  SA MACT lookup result's sa_drop is TRUE
   */
  SOC_PPC_TRAP_CODE_PBP_SA_DROP_2,
  /*
   *  SA MACT lookup result's sa_drop is TRUE
   */
  SOC_PPC_TRAP_CODE_PBP_SA_DROP_3,
  /*
   *  Packet's BVID is in BVID TE range, SA lookup succeeds,
   *  and System Port ID of SA lookup is different from source
   *  system Port ID
   */
  SOC_PPC_TRAP_CODE_PBP_TE_TRANSPLANT,
  /*
   *  Packet's BVID is in BVID TE range, SA lookup fails
   */
  SOC_PPC_TRAP_CODE_PBP_TE_UNKNOWN_TUNNEL,
  /*
   *  Packet's BVID is not in BVID TE range, SA lookup
   *  succeeds
   */
  SOC_PPC_TRAP_CODE_PBP_TRANSPLANT,
  /*
   *  Packet's BVID is not in BVID TE range, SA lookup fails
   */
  SOC_PPC_TRAP_CODE_PBP_LEARN_SNOOP,
  /*
   *  SA authentication fails: SA lookup fails
   */
  SOC_PPC_TRAP_CODE_SA_AUTHENTICATION_FAILED,
  /*
   *  SA authentication fails: SA lookup succeeds in an
   *  unpermitted port
   */
  SOC_PPC_TRAP_CODE_PORT_NOT_PERMITTED,
  /*
   *  SA authentication fails: SA lookup succeeds in an
   *  unpermitted VLAN
   */
  SOC_PPC_TRAP_CODE_UNEXPECTED_VID,
  /*
   *  SA is multicast
   */
  SOC_PPC_TRAP_CODE_SA_MULTICAST,
  /*
   *  SA equals DA
   */
  SOC_PPC_TRAP_CODE_SA_EQUALS_DA,
  /*
   *  8021x authentication fails
   */
  SOC_PPC_TRAP_CODE_8021X,
  /*
   *  Preconfigured. Used to decide whether to accept or drop
   *  packets. See soc_pb_pp_llp_parse_packet_format_info_set
   */
  SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_DROP,
  /*
   *  Preconfigured. Used to decide whether to accept or drop
   *  packets. See soc_pb_pp_llp_parse_packet_format_info_set
   */
  SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_ACCEPT,
  /*
   *  User configured. Used by LLVP table to decide whether to
   *  accept or drop packets. See
   *  soc_pb_pp_llp_parse_packet_format_info_set
   */
  SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_CUSTOM_1,
  /*
   *  User configured. Used by LLVP table to decide whether to
   *  accept or drop packets. See
   *  soc_pb_pp_llp_parse_packet_format_info_set
   */
  SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_CUSTOM_2,
  /*
   * . If InPort is enabled for ARP trapping AND
   *  ARP request TPA is equal to My-IP (two configurable IP
   *  addresses)
   */
  SOC_PPC_TRAP_CODE_MY_ARP,
  /*
   *  ARP Request incoming on InPort which is enabled for ARP
   *  trapping. If incoming DA is set to be ignored, ARP
   *  replies are also trapped with this trap code.
   */
  SOC_PPC_TRAP_CODE_ARP,
  /*
   *  Packet is IGMP and first octet in the IGMP payload is
   *  0x11
   */
  SOC_PPC_TRAP_CODE_IGMP_MEMBERSHIP_QUERY,
  /*
   *  Packet is IGMP and first octet in the IGMP payload is
   *  0x12, 0x16, 0x17, or 0x22
   */
  SOC_PPC_TRAP_CODE_IGMP_REPORT_LEAVE_MSG,
  /*
   *  Packet is IGMP and first octet in the IGMP payload is
   *  not 0x11, 0x12, 0x16, 0x17, or 0x22
   */
  SOC_PPC_TRAP_CODE_IGMP_UNDEFINED,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_ICMPV6_MLD_MC_LISTENER_QUERY,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_0,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_1,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_2,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_3,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_4,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_5,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_6,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_RESERVED_MC_7,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_ICMPV6_MLD_REPORT_DONE_MSG,
  /*
   *  Mapped from (port profile (2), Protocol-LSB (6b)). See
   *  soc_pb_pp_llp_trap_reserved_mc_info_set
   */
  SOC_PPC_TRAP_CODE_ICMPV6_MLD_UNDEFINED,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DHCP_SERVER,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DHCP_CLIENT,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DHCPV6_SERVER,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DHCPV6_CLIENT,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_PROG_TRAP_0,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_PROG_TRAP_1,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_PROG_TRAP_2,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_PROG_TRAP_3,
  /*
   *  Initial VID membership error
   */
  SOC_PPC_TRAP_CODE_PORT_NOT_VLAN_MEMBER,
  /*
   *  Parser indicates header size error.
   */
  SOC_PPC_TRAP_CODE_HEADER_SIZE_ERR,
  /*
   *  Parser error. Soc_petra-B only.
   */
  SOC_PPC_TRAP_CODE_HEADER_SIZE_ERR_O_MPLS,
  /*
   *  BSA lookup fail and (BDA = My-BDA)
   */
  SOC_PPC_TRAP_CODE_MY_B_MAC_AND_LEARN_NULL,
  /*
   *  Unknown I-SID
   */
  SOC_PPC_TRAP_CODE_MY_B_DA_UNKNOWN_I_SID,
  /*
   *  Terminated BMAC header indicates Multicast.
   *  Arad_B0 and above only.
   *  not Relevant for Soc_petra-B and Arad_A0.
   */
  SOC_PPC_TRAP_CODE_MY_B_MAC_MC_CONTINUE,
  /*
   *  Packet ingresses a blocked port. Note - BPDU will be
   *  forwarded with strength 6.
   */
  SOC_PPC_TRAP_CODE_STP_STATE_BLOCK,
  /*
   *  Packet ingresses a port in a LEARN state
   */
  SOC_PPC_TRAP_CODE_STP_STATE_LEARN,
  /*
   *  L2 compatible MC, but IP does not match
   */
  SOC_PPC_TRAP_CODE_IP_COMP_MC_INVALID_IP,
  /*
   *  Terminated My-MAC over IP, but routing is
   *  disabled for InRIF
   */
  SOC_PPC_TRAP_CODE_MY_MAC_AND_IP_DISABLE,
  /*
   *  Ether over IP packet with unexpected
   *  IP version (has to be 0x4)
   *  Arad only.
   *  not Relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_ETHER_IP_VERSION_ERROR,
  /*
   *  for 1+1 protection. pointer path is not as expected i.e.
   *  Path-Select[Protection-Pointer] is not equal Protection-Path
   *  should be configured to drop.
   *  Arad only.
   *  not Relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_LIF_PROTECT_PATH_INVALID,
  /*
   * . Invalid TRILL version
   */
  SOC_PPC_TRAP_CODE_TRILL_VERSION,
  /*
   *  Invalid TRILL TTL
   */
  SOC_PPC_TRAP_CODE_TRILL_INVALID_TTL,
  /*
   *  Unsupported TRILL option flags
   */
  SOC_PPC_TRAP_CODE_TRILL_CHBH,
  /*
   *  Unknown adjacent router bridge
   */
  SOC_PPC_TRAP_CODE_TRILL_NO_REVERSE_FEC,
  /*
   * . Unsupported TRILL options flags
   */
  SOC_PPC_TRAP_CODE_TRILL_CITE,
  /*
   *  Invalid packet format
   */
  SOC_PPC_TRAP_CODE_TRILL_ILLEGAL_INNER_MC,
 /*
   *  TRILL adjacent check failed.
   *  Arad only.
   *  not Relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_TRILL_NO_ADJACENT,
  /*
   *  Terminated My-MAC over MPLS, but MPLS is disabled for
   *  InRIF
   */
  SOC_PPC_TRAP_CODE_MY_MAC_AND_MPLS_DISABLE,
  /*
   *  ARP reply to My-MAC
   */
  SOC_PPC_TRAP_CODE_MY_MAC_AND_ARP,
  /*
   *  Terminated My-MAC but L3 protocol is unknown
   */
  SOC_PPC_TRAP_CODE_MY_MAC_AND_UNKNOWN_L3,
  /*
   *  Reserved MPLS label (0..15) is mapped to
   *  this action by soc_ppd_mpls_term_reserved_label_info_set()
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_0,
  /*
   *  Reserved MPLS label (0..15) is mapped to
   *  this action by soc_ppd_mpls_term_reserved_label_info_set()
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_1,
  /*
   * . Reserved MPLS label (0..15) is mapped to
   *  this action by soc_ppd_mpls_term_reserved_label_info_set()
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_2,
  /*
   *  Reserved MPLS label (0..15) is mapped to
   *  this action by soc_ppd_mpls_term_reserved_label_info_set()
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_MPLS_LABEL_VALUE_3,
  /*
   *  MPLS already terminated twice according to range.
   *  Additional labels may not be terminated according to
   *  range.
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_MPLS_NO_RESOURCES,
  /*
   *  MPLS label is in termination range, but not marked for
   *  termination in Tunnel Termination DB
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   *  see INVALID_LIF_ENTRY
   */
  SOC_PPC_TRAP_CODE_INVALID_LABEL_IN_RANGE,
  /*
   *  Label is found in SEM but marked invalid
   *  in SEM result
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   *  see INVALID_LIF_ENTRY
   */
  SOC_PPC_TRAP_CODE_MPLS_INVALID_LABEL_IN_SEM,
  /*
   *  LIF entry type is invalid
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_MPLS_INVALID_LIF,
 /*
   *  mpls termination error, no resource for MPLS termination.
   *  or number of terminated larger than expected
   *  Arad only.
   *  not Relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_MPLS_TERM_ERROR,
  /*
   *  Terminated MPLS label indicates that
   *  another MPLS shim is to follow, however the terminated
   *  label is BOS
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   *  see UNEXPECTED_NO_BOS
   */
  SOC_PPC_TRAP_CODE_MPLS_LSP_BOS,
  /*
   *  Reserved MPLS label (0..15) is mapped to
   *  this action by soc_ppd_mpls_term_reserved_label_info_set()
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   *  see MPLS_UNEXPECTED_BOS
   */
  SOC_PPC_TRAP_CODE_MPLS_PWE_NO_BOS_LABEL_14,
  /*
   *  MPLS label of type PWE is found in ISEM but it is not in
   *  the bottom of the MPLS stack
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   *  see MPLS_UNEXPECTED_BOS
   *  
   */
  SOC_PPC_TRAP_CODE_MPLS_PWE_NO_BOS,
  /*
   *  MPLS termination suggests that next
   *  protocol is IP, but the terminated label it is not in
   *  the bottom of the MPLS stack
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   *  see MPLS_UNEXPECTED_BOS
   */
  SOC_PPC_TRAP_CODE_MPLS_VRF_NO_BOS,
  /*
   *  expected bos after mpls terminated labels
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_MPLS_UNEXPECTED_NO_BOS,
  /*
   *  expected not bos after mpls terminated labels
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_MPLS_UNEXPECTED_BOS,
  /*
   *  The TTL of the terminated MPLS label is equal to 0
   */
  SOC_PPC_TRAP_CODE_MPLS_TERM_TTL_0,
  /*
   *  The TTL of the terminated MPLS label is equal to 1
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_MPLS_TERM_TTL_1,
  /*
   *  First nibble of control word is equal to 0x1
   */
  SOC_PPC_TRAP_CODE_MPLS_TERM_CONTROL_WORD_TRAP,
  /*
   *  First nibble of control word is equal to 0x0
   */
  SOC_PPC_TRAP_CODE_MPLS_TERM_CONTROL_WORD_DROP,
  /*
   *  IPv4 version error
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_VERSION_ERROR,
  /*
   *  IPv4 header checksum error
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_CHECKSUM_ERROR,
  /*
   *  IPv4 header length is < 20B
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_HEADER_LENGTH_ERROR,
  /*
   *  IPv4 total length is < 20B
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_TOTAL_LENGTH_ERROR,
  /*
   *  IPv4 TTL=0
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_TTL0,
  /*
   *  IPv4 with options
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_HAS_OPTIONS,
  /*
   *  IPv4 TTL=1
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_TTL1,
  /*
   *  SIP equal DIP
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_SIP_EQUAL_DIP,
  /*
   *  IPv4 DIP is 0.0.0.0
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_DIP_ZERO,
  /*
   * . SIP is multicast
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_IPV4_TERM_SIP_IS_MC,
  /*
   *  ITU Y.1731 packet observed (CFM)
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_CFM_ACCELERATED_INGRESS,
  /*
   *  Parser failure
   */
  SOC_PPC_TRAP_CODE_ILLEGEL_PFC,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_DROP_0,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_DROP_1,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_DROP_2,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_DROP_3,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_NOT_FOUND_0,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_NOT_FOUND_1,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_NOT_FOUND_2,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_SA_NOT_FOUND_3,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_0,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_1,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_2,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_3,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_4,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_5,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_6,
  /*
   * 
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_UNKNOWN_DA_7,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_0,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_1,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_2,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_3,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_4,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_5,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_6,
  /*
   *  1588 packet, trap according to filter table
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_1588_PACKET_7,
  /*
   *  FCoE packet mismatch SRC-id and SA
   *  Ethernet-Header.SA[23:0] is not equal to FC-Header.Source-ID
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_FCOE_SRC_SA_MISMATCH,
  /*
   *  SIP transplant, packet incomming from different system port than in FEC entry
   *  Ethernet-Header.SA[23:0] is not equal to FC-Header.Source-ID
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_SIP_TRANSLANT,
  /*
   *  ELK is accessed and returns an error
   */
  SOC_PPC_TRAP_CODE_ELK_ERROR,
  /*
   *  ELK lookup fail
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_ELK_REJECTED,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DA_NOT_FOUND_0,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DA_NOT_FOUND_1,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DA_NOT_FOUND_2,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_DA_NOT_FOUND_3,
  /*
   *  Transparent P2P Lookup key is not found in the ELK/LEM
   */
  SOC_PPC_TRAP_CODE_P2P_MISCONFIGURATION,
  /*
   *  Source interface is equal to destination interface
   *  (hair-pin)
   */
  SOC_PPC_TRAP_CODE_SAME_INTERFACE,
  /*
   *  Forwarding Code is TRILL, lookup key is not found in
   *  ELK/LEM
   */
  SOC_PPC_TRAP_CODE_TRILL_UNKNOWN_UC,
  /*
   *  Forwarding Code is TRILL, lookup key is not found in
   *  ELK/LEM
   */
  SOC_PPC_TRAP_CODE_TRILL_UNKNOWN_MC,
  /*
   *  Forwarding Code is IPv4 UC and RPF FEC Pointer Valid is
   *  not set
   */
  SOC_PPC_TRAP_CODE_UC_LOOSE_RPF_FAIL,
  /*
   *  IPv6 UC default forwarding
   */
  SOC_PPC_TRAP_CODE_DEFAULT_UCV6,
  /*
   *  IPv6 MC default forwarding
   */
  SOC_PPC_TRAP_CODE_DEFAULT_MCV6,
  /*
   *  Lookup key is found in ELK/LEM,
   *  P2P-Service If lookup result is set, and header is not
   *  BOS.
   */
  SOC_PPC_TRAP_CODE_MPLS_P2P_NO_BOS,
  /*
   *  Lookup key is found in ELK/LEM,
   *  P2P-Service If lookup result is set, lookup result
   *  determines that there's a control word, nibble after
   *  MPLS stack equal '1'
   */
  SOC_PPC_TRAP_CODE_MPLS_CONTROL_WORD_TRAP,
  /*
   *  Lookup key is found in ELK/LEM,
   *  P2P-Service If lookup result is set, lookup result
   *  determines that there's a control word, nibble after
   *  MPLS stack does not equal '1' or '0'.
   */
  SOC_PPC_TRAP_CODE_MPLS_CONTROL_WORD_DROP,
  /*
   *  Lookup key is not found in ELK/LEM
   *  Soc_petra-B only.
   *  not supported for ARAD.
   */
  SOC_PPC_TRAP_CODE_MPLS_UNKNOWN_LABEL,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_MPLS_P2P_MPLSX4,
  /*
   *  Matches known protocol (reserved MC). See
   *  soc_pb_pp_l2_lif_l2cp_trap_set()
   */
  SOC_PPC_TRAP_CODE_ETH_L2CP_PEER,
  /*
   *  Tunnel-Termination-Code is TTC-None, DA
   *  matches 01-80-c2-00-00-XX,
   *  L2CP-Drop-Bitmap[{MEF-L2CP-Profile, DA[5:0]}] is set
   *  (see )
   */
  SOC_PPC_TRAP_CODE_ETH_L2CP_DROP,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_ETH_FL_IGMP_MEMBERSHIP_QUERY,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_ETH_FL_IGMP_REPORT_LEAVE_MSG,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_ETH_FL_IGMP_UNDEFINED,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_ETH_FL_ICMPV6_MLD_MC_LISTENER_QUERY,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_ETH_FL_ICMPV6_MLD_REPORT_DONE,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_ETH_FL_ICMPV6_MLD_UNDEFINED,
  /*
   *  Version is different than 4
   */
  SOC_PPC_TRAP_CODE_IPV4_VERSION_ERROR,
  /*
   *  IHL is 5 and the checksum over the first
   *  20 bytes doesn't verify.
   */
  SOC_PPC_TRAP_CODE_IPV4_CHECKSUM_ERROR,
  /*
   *  IHL (Internet Header Length) is less than
   *  5
   */
  SOC_PPC_TRAP_CODE_IPV4_HEADER_LENGTH_ERROR,
  /*
   *  Total length is less than 20
   */
  SOC_PPC_TRAP_CODE_IPV4_TOTAL_LENGTH_ERROR,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_IPV4_TTL0,
  /*
   *  IHL (Internet Header Length) is greater
   *  than 5
   */
  SOC_PPC_TRAP_CODE_IPV4_HAS_OPTIONS,
  /*
   *  In-TTL is 0
   */
  SOC_PPC_TRAP_CODE_IPV4_TTL1,
  /*
   *  Source-IP is equal to destination IP
   */
  SOC_PPC_TRAP_CODE_IPV4_SIP_EQUAL_DIP,
  /*
   *  Destination IP is 0
   */
  SOC_PPC_TRAP_CODE_IPV4_DIP_ZERO,
  /*
   *  Source-IP is multicast
   */
  SOC_PPC_TRAP_CODE_IPV4_SIP_IS_MC,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_IPV4_TUNNEL_TERMINATION_AND_FRAGMENTED,
  /*
   *  Version is different than 6
   */
  SOC_PPC_TRAP_CODE_IPV6_VERSION_ERROR,
  /*
   *  Hop count (TTL) is 0
   */
  SOC_PPC_TRAP_CODE_IPV6_HOP_COUNT0,
  /*
   *  Hop count (TTL) is 1
   */
  SOC_PPC_TRAP_CODE_IPV6_HOP_COUNT1,
  /*
   *  Forwarding header DIP = ::
   */
  SOC_PPC_TRAP_CODE_IPV6_UNSPECIFIED_DESTINATION,
  /*
   *  DIP = ::1 or SIP = ::1
   */
  SOC_PPC_TRAP_CODE_IPV6_LOOPBACK_ADDRESS,
  /*
   *  The MSB of the SIP = 0xFF
   */
  SOC_PPC_TRAP_CODE_IPV6_MULTICAST_SOURCE,
  /*
   *  Next-protocol is zero
   */
  SOC_PPC_TRAP_CODE_IPV6_NEXT_HEADER_NULL,
  /*
   *  SIP = ::
   */
  SOC_PPC_TRAP_CODE_IPV6_UNSPECIFIED_SOURCE,
  /*
   *  Bits 127:118 of the destination-IP are
   *  equal to 0x3FA
   */
  SOC_PPC_TRAP_CODE_IPV6_LOCAL_LINK_DESTINATION,
  /*
   *  Bits 127:118 of the DIP = 0x3FB
   *  (deprecated)
   */
  SOC_PPC_TRAP_CODE_IPV6_LOCAL_SITE_DESTINATION,
  /*
   *  Bits 127:118 of the SIP = 0x3FA
   */
  SOC_PPC_TRAP_CODE_IPV6_LOCAL_LINK_SOURCE,
  /*
   *  Bits 127:118 of the SIP = 0x3FB
   */
  SOC_PPC_TRAP_CODE_IPV6_LOCAL_SITE_SOURCE,
  /*
   *  Bits 127:32 of the DIP = 0
   */
  SOC_PPC_TRAP_CODE_IPV6_IPV4_COMPATIBLE_DESTINATION,
  /*
   *  Bits 127:32 of the DIP are equal to
   *  0000_FFF_0000_0000_0000_0000
   */
  SOC_PPC_TRAP_CODE_IPV6_IPV4_MAPPED_DESTINATION,
  /*
   *  MSB of the DIP=0xFF
   */
  SOC_PPC_TRAP_CODE_IPV6_MULTICAST_DESTINATION,
  /*
   *  TTL is equal to 0
   */
  SOC_PPC_TRAP_CODE_MPLS_TTL0,
  /*
   *  TTL is equal to 1
   */
  SOC_PPC_TRAP_CODE_MPLS_TTL1,
  /*
   *  L4 Sequence-Number and Flags (6) are both
   *  zero
   */
  SOC_PPC_TRAP_CODE_TCP_SN_FLAGS_ZERO,
  /*
   *  L4 Sequence-Number is zero and either
   *  Flags. FIN, Flags. URG or FLAGS. PSH are set
   */
  SOC_PPC_TRAP_CODE_TCP_SN_ZERO_FLAGS_SET,
  /*
   *  Both Flags. SYN and Flags. FIN are set
   */
  SOC_PPC_TRAP_CODE_TCP_SYN_FIN,
  /*
   *  Source-Port equals Destination-Port
   */
  SOC_PPC_TRAP_CODE_TCP_EQUAL_PORTS,
  /*
   *  L3 is IPv4 and IP-Header. Fragmented and
   *  IP-Header. Fragment-Offset zero and
   *  (IPv4-Header. Total-Length - 4 * IPv4-Header. IHL) is less
   *  than 20B
   */
  SOC_PPC_TRAP_CODE_TCP_FRAGMENT_INCOMPLETE_HEADER,
  /*
   *  L3 is IPv4 and IP-Header. Fragmented and
   *  IP-Header. Fragment-Offset is less than 8
   */
  SOC_PPC_TRAP_CODE_TCP_FRAGMENT_OFFSET_LT8,
  /*
   *  Source-Port equals Destination-Port
   */
  SOC_PPC_TRAP_CODE_UDP_EQUAL_PORTS,
  /*
   *  L3 is IPv4 and (IPv4-Header. Total-Length -
   *  4* IPv4-Header. IHL) is greater than 576B or layer-3 is
   *  IPv6 and IPv6-Header. Payload-Length is greater than 576B
   */
  SOC_PPC_TRAP_CODE_ICMP_DATA_GT_576,
  /*
   *  IP-Header. Fragmented is set
   */
  SOC_PPC_TRAP_CODE_ICMP_FRAGMENTED,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_GENERAL,
  /*
   *  Both Destination-0-Valid and Destination-1-Valid are not
   *  set
   */
  SOC_PPC_TRAP_CODE_FACILITY_INVALID,
  /*
   *  FEC-Entry. Trap-If-Accessed is set and
   *  first packet has been sent to this FEC entry
   */
  SOC_PPC_TRAP_CODE_FEC_ENTRY_ACCESSED,
  /*
   *  RPF-FEC-Entry. UC-RPF-Mode is 'Strict' and
   *  RPF-FEC-Entry. OutRIF is not equal to packet InRIF
   *  Relevant when FEC-Entry. UC-RPF-Mode is 'Stict'
   */
  SOC_PPC_TRAP_CODE_UC_STRICT_RPF_FAIL,
  /*
   *  RPF-Entry. Expected-InRIF is not equal to
   *  packet InRIF
   *  Relevant when FEC-Entry. MC-RPF-Mode is 'Explicit'
   */
  SOC_PPC_TRAP_CODE_MC_EXPLICIT_RPF_FAIL,
  /*
   * 
   *  check is fail if
   *  RPF-FEC-Entry-Valid is not set, or
   *  RPF-FEC-Entry. Out-RIF is not equal to In-RIF
   *  Relevant when FEC-Entry. MC-RPF-Mode is 'Use-SIP-WITH-ECMP'
   *  Note: 	in this mode if the RPF-FEC-Entry is part of an ECMP then the ECMP base is used
   *  
   */
  SOC_PPC_TRAP_CODE_MC_USE_SIP_AS_IS_RPF_FAIL,
  /*
   *  RPF-FEC-Entry-Valid is not set, or
   *  (RPF-ECMP-Group-Size <= '1' and RPF-FEC-Entry. OutRIF is
   *  not equal to InRIF)
   *  Relevant when FEC-Entry. MC-RPF-Mode is 'Use-SIP'
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_MC_USE_SIP_RPF_FAIL,
  /*
   *  Found RPF-ECMP-Group-Size > '1'
   *  Relevant when FEC-Entry. MC-RPF-Mode is 'Use-SIP'
   *  Soc_petra-B only.
   *  not relevant for ARAD.
   */
  SOC_PPC_TRAP_CODE_MC_USE_SIP_ECMP,
  /*
   *  ICMP-Redirect is enabled, Forwarding-Code
   *  is IPv4|6-UC, and packet InRIF is equal to
   *  FEC-Entry. OutRIF
   */
  SOC_PPC_TRAP_CODE_ICMP_REDIRECT,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_MP_NON_ACCELERATED,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_MORE_THAN_ONE_ING_COUNTER,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_PWE_INVALID_PROTOCOL,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_IP_OVER_MPLS,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_TIME_SYNC,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_AC_KEY_MISS,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_MEP_NON_ACCELERATED,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_VSI_MEMBERSHIP,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_HAIR_PIN,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_SPLIT_HORIZON,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_UNKNOWN_DA_UC,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_UNKNOWN_DA_MC,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_UNKNOWN_DA_BC,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_MTU_VIOLATE,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_STP,
  /*
   *  T20E only.
   */
  SOC_PPC_TRAP_CODE_EG_FILTER_ACCEPT_FRAME,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_OAMP,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_ETH_OAM_ACCELERATED,
  /*
   *  In T20E this trap code cannot be used in
   *  soc_ppd_trap_frwrd_profile_info_set(),soc_ppd_trap_snoop*_profile_info_set()
   *  as trap_ndx, it's only used as Trap-code in the packet
   *  header
   */
  SOC_PPC_TRAP_CODE_USER_MPLS_OAM_ACCELERATED,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_BFD_IP_OAM_TUNNEL_ACCELERATED,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_BFD_PWE_OAM_ACCELERATED,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_ETH_OAM_UP_ACCELERATED,
  /*
   * EoTrilloE packet is not designated vlan but with my-mac, or designated vlan but without my-mac.
   * Disable bridging for such packets.
   */
  SOC_PPC_TRAP_CODE_TRILL_DISABLE_BRIDGE_IF_DESIGNATED,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_ETH_OAM,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_Y1731_O_MPLS_TP,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_Y1731_O_PWE,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_BFD_O_IPV4,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_BFD_O_MPLS,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_BFD_O_PWE,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_BFDCC_O_MPLS_TP,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_TRAP_BFDCV_O_MPLS_TP,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_OAM_LEVEL,
  /* 
   * OAM/BFD trap
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_OAM_PASSIVE,
  /* 
   * ARP packet (Ethernet packet with Ethertype 0x0806 
   * and DA = all ones) forwarded as ARP packet 
   * i.e. lookup is TPA and VRF, and lookup fail 
   * arad only.
   * not relevant for Soc_petra-B
   */
  SOC_PPC_TRAP_CODE_ARP_FLP_FAIL,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_0,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_1,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_2,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_3,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_4,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_5,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_6,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_7,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_8,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_9,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_10,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_11,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_12,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_13,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_14,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_15,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_16,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_17,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_18,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_19,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_20,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_21,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_22,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_23,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_24,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_25,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_26,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_27,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_28,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_29,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_30,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_31,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_32,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_33,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_34,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_35,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_36,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_37,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_38,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_39,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_40,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_41,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_42,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_43,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_44,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_45,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_46,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_47,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_48,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_49,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_50,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_51,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_52,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_53,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_54,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_55,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_56,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_57,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_58,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_59,
  /*
   * 
   */
  SOC_PPC_TRAP_CODE_USER_DEFINED_LAST = SOC_PPC_TRAP_CODE_USER_DEFINED_59,
  /*
   *  FCoE packet routed (FCF)
   *  and either forwarding lookup fail or zoning lookup fail will handle packet according to this trap
   *  Arad only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_CODE_FCOE_FCF_FLP_LOOKUP_FAIL,
  /*
   *    L2 compatible MC in inner Ethernet, but IP does not match.
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_INNER_IP_COMP_MC_INVALID_IP_0,
  /*
   *    Unexpected MPLS reserved label.
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_MPLS_ILLEGAL_LABEL,
  /*
   *    Terminated Inner-My-MAC over IP, but routing is disabled for InRIF.
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_INNER_ETHERNET_MY_MAC_IP_DISABLED,
  /*
   *    MPLS tunnel termination trap with TTL equal to 0.
   */
  SOC_PPC_TRAP_CODE_MPLS_TUNNEL_TERMINATION_TTL0,
  /*
   *    MPLS tunnel termination trap with TTL equal to 1.
   */
  SOC_PPC_TRAP_CODE_MPLS_TUNNEL_TERMINATION_TTL1,
  /*
   *    MPLS forwarding trap with TTL equal to 0.
   */
  SOC_PPC_TRAP_CODE_MPLS_FORWARDING_TTL0,
  /*
   *    MPLS forwarding trap with TTL equal to 1.
   */
  SOC_PPC_TRAP_CODE_MPLS_FORWARDING_TTL1,
  /*
   *  Number of types in SOC_PPC_TRAP_CODE
   */
  SOC_PPC_NOF_TRAP_CODES
}SOC_PPC_TRAP_CODE;

#define SOC_PPC_NOF_TRAP_QUALIFIERS 4096

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Sets the CPU trap code. For trapped packet the CPU
   *  code are associated into the packet headers. The CPU
   *  code also used as index for the forwarding/snooping
   *  actions. see to soc_ppd_trap_frwrd_profile_info_set(),
   *  soc_ppd_trap_snoop_profile_info_set(). Range Soc_petra-B: 0-255,
   *  T20E:0-63. To know the legal range in specific use of
   *  this field, see the documentation in that specific
   *  use. In Soc_petra-B this field affects the CPU code set on
   *  the packets header, In T20E it's only select the
   *  forwarding and snooping profiles.
   */
  SOC_PPC_TRAP_CODE trap_code;
  /*
   *  Value that identifies the strength of the assignment of
   *  Forward action to the packet. Through the packet
   *  processing pipe, many forwarding decisions may be taken;
   *  yet one decision may overwrite a former decision only if
   *  it stronger, i.e., its strength is higher than the last
   *  decision that was taken. Range 0-7. Relevant only for
   *  Soc_petra-B.
   */
  uint32 frwrd_action_strength;
  /*
   *  Value that identifies the strength of the assignment of
   *  Snoop action to the packet. Through the packet
   *  processing pipe, many snooping decisions may be taken;
   *  yet one decision may overwrite a former decision only if
   *  it stronger, i.e., its strength is higher than the last
   *  decision that was taken. Relevant only for Soc_petra-B.
   */
  uint32 snoop_action_strength;

} SOC_PPC_ACTION_PROFILE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Set the Trap code to be associated with the packet,
   *  this trap code also used as index for forward/snoop
   *  actions. Set also the strength for snoop and forward
   *  actions. In this case trap_code range: 0 - 255.
   */
  SOC_PPC_ACTION_PROFILE action_profile;

} SOC_PPC_TRAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Outgoing Logical interface type: Layer 2 or Layer 3.
   */
  SOC_PPC_OUTLIF_ENCODE_TYPE type;
  /*
   *  Out-LIF value, according to type. May be AC / EEP /
   *  VSI. AC: Attachmet-Circuit IDWhen Out-LIF is AC, the
   *  'ac_id' points to the Out-AC database. EEP: Egress
   *  Editing Pointer IDWhen Out-LIF is PWE, the 'eep_id'
   *  points to the PWE information, or to the tunnel
   *  information when the Out-VC label is sent over the EEI.
   *  When Out-LIF is MinM interface, the EEP points to the
   *  backbone editing database. When Out-LIF is RIF-Tunnel,
   *  'eep_id' points to the tunneling information. When
   *  Out-LIF is RIF-TRILL the EEP points to the adjacent
   *  RBridge Nickname. VSI: Out Virtual Switch Interface
   *  ID. Valid when Out-LIF Type is VSI. When Out-LIF is
   *  RIF-VSI, 'outvsi_id' is the Out-RIF.
   */
  uint32 val;

} SOC_PPC_OUTLIF;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The MPLS command type. For NOP, use Push command with
   *  push_profile = 0 and Label = 0. To select Push-profile
   *  for Push Command use the push_profile attribute.
   */
  SOC_PPC_MPLS_COMMAND_TYPE command;
  /*
   *  Label. Relevant if the command is push or swap.
   */
  SOC_SAND_PP_MPLS_LABEL label;
  /*
   *  If the command is push, this is the push profile used to
   *  construct the label's TTL and EXP. Otherwise, this field
   *  is ignored. Range: 0 - 7. EEI may include the MPLS
   *  command, but then the command cannot be Push with
   *  push_profile 0. See SOC_PPC_EEI.
   */
  uint32 push_profile;
  /*
   *  when pop into ethernet then this is the TPID profile
   *  to use for parsing inner Ethernet
   *  relevant ony for pop command.
   *  Arad only.
   */
  uint32 tpid_profile;
  /*
   *  wether pop label include control world
   *  relevant ony for pop command
   *  Arad only.
   */
  SOC_SAND_PP_MPLS_TUNNEL_MODEL model;
  /*
   *  wether pop label include control world
   *  relevant ony for pop command
   *  Arad only.
   */
  uint8 has_cw;
  /*
   *  type of next header
   *  IPv4, Ipv6, MPLS, Ethernet, 
  *  relevant only for POP command
   */
  SOC_PPC_PKT_FRWRD_TYPE pop_next_header;

} SOC_PPC_MPLS_COMMAND;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Trill Destination (dest-nick or dist-tree-nick). Relevant
   *  when EEI type is TRILL.
   */
  SOC_SAND_PP_TRILL_DEST trill_dest;
  /*
   *  MPLS command to push/swap or pop VC/Tunnel
   *  labels. Relevant when EEI type is mpls_command.
   */
  SOC_PPC_MPLS_COMMAND mpls_command;
  /*
   *  I-SID to be encapsulated on packets from the customers
   *  toward the backbone. Relevant when EEI type is MIM
   *  (MAC-in-MAC).
   */
  SOC_SAND_PP_ISID isid;
  /*
   *  EEI value is raw
   *  used only for set. never returned in get.
   */
  uint32 raw;
  /*
   *  out-LIF: encapsulation pointer
   *  to access egress encapsulation DB
   */
  uint32 outlif;

} SOC_PPC_EEI_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  EEI type (MIM/MPLS command/TRILL)
   */
  SOC_PPC_EEI_TYPE type;
  /*
   *  The value in the EEI according to type. See SOC_PPC_EEI_VAL.
   */
  SOC_PPC_EEI_VAL val;

} SOC_PPC_EEI;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Egress Edit Information. Relevant when the forwarding
   *  decision type is FEC or multicast
   */
  SOC_PPC_EEI eei;
  /*
   *  Layer 2 Out-LIF. Relevant when the forwarding decision
   *  type is Unicast
   */
  SOC_PPC_OUTLIF outlif;
  /*
   *  Trap-info. Relevant when the forwarding decision type is
   *  Trap
   */
  SOC_PPC_TRAP_INFO trap_info;

} SOC_PPC_FRWRD_DECISION_TYPE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Forwarding decision type:Unicast without FEC; Multicast
   *  without FEC; FEC; Drop or Trap
   */
  SOC_PPC_FRWRD_DECISION_TYPE type;
  /*
   *  Destination ID, according to type:
   *  Multicast   : Multicast id
   *  FEC         : FEC id
   *  Trap/Drop   : Ignored
   *  Unicast port: System port.
   *  Unicast flow: Explicit flow
   *  Unicast LAG : LAG id
   */
  uint32 dest_id;
  /*
   *  Per forwarding decision type additional information:
   *  FEC / MC      : EEI
   *  UC without FEC: Out-LIF
   *  TRAP          : Trap information
   *  When the forwarding is multicast with EEI, all the copies
   *  are made with the same EEI. E. G, in MinM application, the
   *  EEI may carry the I-SID.
   */
  SOC_PPC_FRWRD_DECISION_TYPE_INFO additional_info;

  uint32 flags;

} SOC_PPC_FRWRD_DECISION_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Selects one of four global TPIDs (usually the outer).
   *  Used to parse the VLAN Tags in the ingress/egress. Range:
   *  0 - 3.
   */
  uint8 tpid1_index;
  /*
   *  Selects one of four global TPIDs (usually the inner).
   *  Used to parse the VLAN Tags in the ingress/egress and to
   *  construct the VLAN Tags in the egress. Range: 0 - 3.
   */
  uint8 tpid2_index;

} SOC_PPC_TPID_PROFILE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Local port ID. Has to be of type CEP.
   */
  SOC_PPC_PORT port;
  /*
   *  The VSI the packet associated with. For Link Layer
   *  encapsulated packets, this is the VSI from the Egress
   *  Encapsulation.
   */
  SOC_PPC_VSI_ID vsi;

} SOC_PPC_PEP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The type of the iterator. (fast, ordered ...)
   */
  SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE type;
  /*
   *  Iterator information to be used to traverse the L3
   *  routing tables. This information should be changed only
   *  by the given macros. To set the iterator to start
   *  scanning the routing table from the beginning, use
   *  SOC_PPC_IP_ROUTING_TABLE_ITER_BEGIN() To check if the
   *  returned iterator points to the end of the table, use
   *  SOC_PPC_IP_ROUTING_TABLE_ITER_IS_END()
   */
  SOC_SAND_U64 payload;

} SOC_PPC_IP_ROUTING_TABLE_ITER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The index to start reading from in the Routing Table and
   *  the order to traverse the routing table.
   */
  SOC_PPC_IP_ROUTING_TABLE_ITER start;
  /*
   *  The number of entries to scan. Stop after scanning this
   *  number of entries.
   */
  uint32 entries_to_scan;
  /*
   *  Number of entries to read/modify. Stop after retrieving
   *  this number of entries.
   */
  uint32 entries_to_act;
  /* Whether read/modify entries from LEM DB */
  uint32 entries_from_lem;
} SOC_PPC_IP_ROUTING_TABLE_RANGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Device additional TPID values. Used for Link-layer parsing to
   *  identify the VLAN tags on the packets. 
   *  tpid_vals[0] is used as Tpid1AdditionalValue1. 
   *  tpid_vals[1] is used as Tpid1AdditionalValue2.
   *  tpid_vals[2] is used as Tpid2AdditionalValue1.
   *  tpid_vals[3] is used as Tpid2AdditionalValue2.
   *  For Jericho Only.
   */
  SOC_SAND_PP_TPID tpid_vals[SOC_PPC_MAX_NOF_ADDITIONAL_TPID_VALS];
} SOC_PPC_ADDITIONAL_TPID_VALUES;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  In Lif (local)
   */
  uint32 in_lif;
  /*
   *  Inner tag
   */
  uint32 inner_tag;
  /*
   *  Outer tag
   */
  uint32 outer_tag;
  /*
   *  number of tags (1-2)
   */
  uint32 nof_tags;

} SOC_PPC_FRWRD_MATCH_INFO;

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
  SOC_PPC_TRAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_INFO *info
  );

void
  SOC_PPC_OUTLIF_clear(
    SOC_SAND_OUT SOC_PPC_OUTLIF *info
  );

void
  SOC_PPC_MPLS_COMMAND_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_COMMAND *info
  );

void
  SOC_PPC_EEI_VAL_clear(
    SOC_SAND_OUT SOC_PPC_EEI_VAL *info
  );

void
  SOC_PPC_EEI_clear(
    SOC_SAND_OUT SOC_PPC_EEI *info
  );

void
  SOC_PPC_FRWRD_DECISION_TYPE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_TYPE_INFO *info
  );

void
  SOC_PPC_FRWRD_DECISION_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO *info
  );

void
  SOC_PPC_ACTION_PROFILE_clear(
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE *info
  );

void
  SOC_PPC_TPID_PROFILE_clear(
    SOC_SAND_OUT SOC_PPC_TPID_PROFILE *info
  );

void
  SOC_PPC_PEP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_PEP_KEY *info
  );

void
  SOC_PPC_IP_ROUTING_TABLE_ITER_clear(
    SOC_SAND_OUT SOC_PPC_IP_ROUTING_TABLE_ITER *info
  );

void
  SOC_PPC_IP_ROUTING_TABLE_RANGE_clear(
    SOC_SAND_OUT SOC_PPC_IP_ROUTING_TABLE_RANGE *info
  );

void
  SOC_PPC_ADDITIONAL_TPID_VALUES_clear(
    SOC_SAND_OUT SOC_PPC_ADDITIONAL_TPID_VALUES *info
  );

void
  SOC_PPC_FRWRD_MATCH_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MATCH_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_EEI_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_EEI_TYPE enum_val
  );

const char*
  SOC_PPC_OUTLIF_ENCODE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_OUTLIF_ENCODE_TYPE enum_val
  );

void
  SOC_PPC_FRWRD_DECISION_INFO_print_table_format(
    SOC_SAND_IN  char                *line_pref,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO *info
  );

const char*
  SOC_PPC_FRWRD_DECISION_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_TYPE enum_val
  );

const char*
  SOC_PPC_MPLS_COMMAND_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_MPLS_COMMAND_TYPE enum_val
  );

const char*
  SOC_PPC_L2_NEXT_PRTCL_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L2_NEXT_PRTCL_TYPE enum_val
  );

const char*
  SOC_PPC_L3_NEXT_PRTCL_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_L3_NEXT_PRTCL_TYPE enum_val
  );

const char*
  SOC_PPC_PKT_TERM_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_PKT_TERM_TYPE enum_val
  );

const char*
  SOC_PPC_PKT_FRWRD_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_PKT_FRWRD_TYPE enum_val
  );

const char*
  SOC_PPC_PKT_HDR_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_PKT_HDR_TYPE enum_val
  );

const char*
  SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_IP_ROUTING_TABLE_ITER_TYPE enum_val
  );

const char*
  SOC_PPC_HASH_MASKS_to_string(
    SOC_SAND_IN  SOC_PPC_HASH_MASKS enum_val
  );

void
  SOC_PPC_TRAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_INFO *info
  );

void
  SOC_PPC_OUTLIF_print(
    SOC_SAND_IN  SOC_PPC_OUTLIF *info
  );

void
  SOC_PPC_MPLS_COMMAND_print(
    SOC_SAND_IN  SOC_PPC_MPLS_COMMAND *info
  );

void
  SOC_PPC_EEI_VAL_print(
    SOC_SAND_IN  SOC_PPC_EEI_VAL *info
  );

void
  SOC_PPC_EEI_print(
    SOC_SAND_IN  SOC_PPC_EEI *info
  );

void
  SOC_PPC_FRWRD_DECISION_TYPE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_TYPE      frwrd_type,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_TYPE_INFO *info
  );

void
  SOC_PPC_FRWRD_DECISION_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO *info
  );

void
  SOC_PPC_ACTION_PROFILE_print(
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE *info
  );

void
  SOC_PPC_TPID_PROFILE_print(
    SOC_SAND_IN  SOC_PPC_TPID_PROFILE *info
  );

void
  SOC_PPC_PEP_KEY_print(
    SOC_SAND_IN  SOC_PPC_PEP_KEY *info
  );

void
  SOC_PPC_IP_ROUTING_TABLE_ITER_print(
    SOC_SAND_IN  SOC_PPC_IP_ROUTING_TABLE_ITER *info
  );

void
  SOC_PPC_IP_ROUTING_TABLE_RANGE_print(
    SOC_SAND_IN  SOC_PPC_IP_ROUTING_TABLE_RANGE *info
  );

void
  SOC_PPC_ADDITIONAL_TPID_VALUES_print(
    SOC_SAND_IN  SOC_PPC_ADDITIONAL_TPID_VALUES *info
  );

void
  SOC_PPC_FRWRD_MATCH_INFO_print(
    SOC_SAND_IN SOC_PPC_FRWRD_MATCH_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_GENERAL_INCLUDED__*/
#endif
