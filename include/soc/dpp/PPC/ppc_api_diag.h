/* $Id: ppc_api_diag.h,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_diag.h
*
* MODULE PREFIX:  soc_ppc_diag
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

#ifndef __SOC_PPC_API_DIAG_INCLUDED__
/* { */
#define __SOC_PPC_API_DIAG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_trap_mgmt.h>
#include <soc/dpp/PPC/ppc_api_frwrd_bmact.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ipv4.h>
#include <soc/dpp/PPC/ppc_api_frwrd_fcf.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ilm.h>
#include <soc/dpp/PPC/ppc_api_lif.h>
#include <soc/dpp/PPC/ppc_api_rif.h>
#include <soc/dpp/PPC/ppc_api_llp_sa_auth.h>
#include <soc/dpp/PPC/ppc_api_llp_vid_assign.h>
#include <soc/dpp/PPC/ppc_api_llp_parse.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ipv6.h>
#include <soc/dpp/PPC/ppc_api_frwrd_trill.h>
#include <soc/dpp/PPC/ppc_api_lif_ing_vlan_edit.h>
#include <soc/dpp/PPC/ppc_api_eg_encap.h>
#include <soc/dpp/PPC/ppc_api_eg_vlan_edit.h>
#include <soc/dpp/PPC/ppc_api_eg_ac.h>

#include <soc/dpp/TMC/tmc_api_ports.h>
#include <soc/dpp/TMC/tmc_api_pmf_low_level_db.h>

#include <soc/dpp/ARAD/arad_parser.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximum number of headers can be identified by device   */
#define  SOC_PPC_DIAG_MAX_NOF_HDRS (7)

/*     Maximum size in uint32s of a TCAM entry                   */
#define  SOC_PPC_DIAG_TCAM_KEY_NOF_UINT32S_MAX (9)

/*     Maximum size in longs of buffer used to include packet's
 *     header                                                  */
#define  SOC_PPC_DIAG_BUFF_MAX_SIZE (32)

#define SOC_PPC_DIAG_FILTER_SECOND_SET   (0x80000000)

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
  SOC_PPC_DIAG_OK = 0,
  SOC_PPC_DIAG_NOT_FOUND = 1
} SOC_PPC_DIAG_RESULT;

typedef enum
{
  /*
   *  No forwarding lookup was performed, possibly the packet
   *  was forwarded as P2P.
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_NONE = 0,
  /*
   *  Bridging
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_MACT = 1,
  /*
   *  MAC-in-MAC
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_BMACT = 2,
  /*
   *  IPv4 routing UC
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_IPV4_UC = 3,
  /*
   *  IPv4 routing MC
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_IPV4_MC = 4,
  /*
   *  IPv4 with VRF != 0
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_IPV4_VPN = 5,
  /*
   *  IPv6 routing UC
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_IPV6_UC = 6,
  /*
   *  IPv6 routing MC
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_IPV6_MC = 7,
  /*
   *  IPv6 with VRF != 0
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_IPV6_VPN = 8,
  /*
   *  LSR: ILM
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_ILM = 9,
  /*
   *  TRILL UC
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_TRILL_UC = 10,
  /*
   *  TRILL MC
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_TRILL_MC = 11,
  /*
   *  IPv4 host
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_IPV4_HOST = 12,
  /*
   *  FCF
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE_FCF = 13,
  /*
   *  Number of types in SOC_PPC_DIAG_FWD_LKUP_TYPE
   */
  SOC_PPC_NOF_DIAG_FWD_LKUP_TYPES = 14
}SOC_PPC_DIAG_FWD_LKUP_TYPE;


typedef enum
{
  /*
   *  number of ingress learning lookups
   */
  SOC_PPC_DIAG_LEARN_COUNTER_INGRESS_LKUP = 0,
  /*
   *  number of egress learning lookups
   */
  SOC_PPC_DIAG_LEARN_COUNTER_EGRESS_LKUP = 1,
  /*
   *  number of mact events (learn, refresh, age-out)
   */
  SOC_PPC_DIAG_LEARN_COUNTER_MACT_EVENTS = 2,
  /*
   *  number of olp requests toward MACT
   */
  SOC_PPC_DIAG_LEARN_COUNTER_OLP_REQUESTS = 3,
  /*
   *  number of olp sent messages
   */
  SOC_PPC_DIAG_LEARN_COUNTER_OLP_TX_MSGS = 4,
  /*
   *  number of olp messages
   */
  SOC_PPC_DIAG_LEARN_COUNTER_MACT_REQUESTS = 5,
  /*
   *  Number of types in SOC_PPC_DIAG_FWD_LKUP_TYPE
   */
  SOC_PPC_NOF_LEARN_COUNTER_TYPES = 10
}SOC_PPC_DIAG_LEARN_COUNTER_TYPE;


typedef enum
{
  /*
   *  The TCAM User is the Forwarding (for IPv4 MC and IPv6)
   */
  SOC_PPC_DIAG_TCAM_USAGE_FRWRDING = 0,
  /*
   *  The TCAM User is the IRPP PMF
   */
  SOC_PPC_DIAG_TCAM_USAGE_PMF = 1,
  /*
   *  The TCAM User is the Egress ACL
   */
  SOC_PPC_DIAG_TCAM_USAGE_EGR_ACL = 2,
  /*
   *  Number of types in SOC_PPC_DIAG_TCAM_USAGE
   */
  SOC_PPC_NOF_DIAG_TCAM_USAGES = 3
}SOC_PPC_DIAG_TCAM_USAGE;

typedef enum
{
  /*
   *  none of the below flavors
   */
  SOC_PPC_DIAG_FLAVOR_NONE = 0x0,
  /*
   *  The returned value by Diag API is a raw bit-stream i.e.
   *  without parsing into structure.
   */
  SOC_PPC_DIAG_FLAVOR_RAW = 0x1,
  /*
   *  after getting packet trace/log info clear this
   *  information.
   */
  SOC_PPC_DIAG_FLAVOR_CLEAR_ON_GET = 0x2,
  /*
   *  Number of types in SOC_PPC_DIAG_FLAVOR
   */
  SOC_PPC_NOF_DIAG_FLAVORS = 0x4
}SOC_PPC_DIAG_FLAVOR;

typedef enum
{
  /*
   *  No valid lookup perform in LEM DB.
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_NONE = 0,
  /*
   *  Bridging
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_MACT = 1,
  /*
   *  MAC-in-MAC
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_BMACT = 2,
  /*
   *  IPv4 routing UC
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_HOST = 3,
  /*
   *  ILM
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_ILM = 4,
  /*
   *  sa authentication
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_SA_AUTH = 5,
  /*
   *  sa authentication
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_EXTEND_P2P = 6,
  /*
   *  Trill adjacent
   */
  SOC_PPC_DIAG_LEM_LKUP_TYPE_TRILL_ADJ = 7,
  /*
   *  Number of types in SOC_PPC_DIAG_LEM_LKUP_TYPE
   */
  SOC_PPC_NOF_DIAG_LEM_LKUP_TYPES = 8
}SOC_PPC_DIAG_LEM_LKUP_TYPE;

typedef enum
{
  /*
   *  No valid lookup perform in LEM DB.
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_NONE = 0,
  /*
   *  AC identification according to port-vlan-[vlan]
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_AC = 1,
  /*
   *  PWE lookup according to packet's VC-label
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_PWE = 2,
  /*
   *  MPLS lookup according to packet's tunnel-label
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_MPLS_TUNNEL = 3,
  /*
   *  IPv4 tunnel lookup according to packet's DIP
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_IPV4_TUNNEL = 4,
  /*
   *  Trill lookup according to packet's Nick-name
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_TRILL = 5,
  /*
   *  MAC-IN-MAC lookup according to packet's ISID
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_MIM_ISID = 6,

  /* 
   * l2gre lookup according to vsid 
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_L2GRE = 7,
  /* 
   * vxlan lookup according to vni
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_VXLAN = 8,
  /* 
   * extender lookup according to port-ecid-cvid
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE_EXTENDER = 9,
  /*
   *  Number of types in SOC_PPC_DIAG_LIF_LKUP_TYPE
   */
  SOC_PPC_NOF_DIAG_LIF_LKUP_TYPES = 10
}SOC_PPC_DIAG_LIF_LKUP_TYPE;

typedef enum
{
  /*
   *  The LIF DB
   */
  SOC_PPC_DIAG_DB_TYPE_LIF = 0,
  /*
   *  The forwarding DB (LEM)
   */
  SOC_PPC_DIAG_DB_TYPE_LEM = 1,
  /*
   *  The longest prefix Match DB, used for IPv4/6 forwarding
   *  and IPv4 RPF check.
   */
  SOC_PPC_DIAG_DB_TYPE_LPM = 2,
  /*
   *  The TCAM DB used for forwarding
   */
  SOC_PPC_DIAG_DB_TYPE_TCAM_FRWRD = 3,
  /*
   *  The TCAM DB used for PMF
   */
  SOC_PPC_DIAG_DB_TYPE_TCAM_PMF = 4,
  /*
   *  The TCAM DB used for egress ACL
   */
  SOC_PPC_DIAG_DB_TYPE_TCAM_EGR = 5,
  /*
   *  The TCAM DB used for egress LIF
   */
  SOC_PPC_DIAG_DB_TYPE_EG_LIF = 6,
  /*
   *  The TCAM DB used for egress Tunneling
   */
  SOC_PPC_DIAG_DB_TYPE_EG_TUNNEL = 7,
  /*
   *  The TCAM DB used for egress Link Layer encapsulation
   */
  SOC_PPC_DIAG_DB_TYPE_EG_LL = 8,
  /*
   *  Number of types in SOC_PPC_DIAG_DB_TYPE
   */
  SOC_PPC_NOF_DIAG_DB_TYPES = 9
}SOC_PPC_DIAG_DB_TYPE;

typedef enum
{
  /*
   *  The initial destination/trap set to port
   */
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_INIT_PORT = 0,
  /*
   *  The initial destination set to LIF entry, relevant when
   *  LIF (AC/PWE/ISID...) is P2P
   */
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_LIF = 1,
  /*
   *  The destination set according to lookup in the
   *  forwarding DB, if for bridged packet DA not found, then
   *  packet is forwarded according to trap (unknown-da)
   */
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_LKUP_FOUND = 2,
  /*
   *  The destination is set according to PMF
   */
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_PMF = 3,
  /*
   *  The destination is set according to FEC
   */
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_FEC = 4,
  /*
   *  The destination is set according to Trap
   */
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_TRAP = 5,
  /*
   *  The ultimate forwarding decision set by ingress
   *  processing
   */
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_ING_RESOLVED = 6,
  /*
   *  Number of types in SOC_PPC_DIAG_FRWRD_DECISION_PHASE
   */
  SOC_PPC_NOF_DIAG_FRWRD_DECISION_PHASES = 7
}SOC_PPC_DIAG_FRWRD_DECISION_PHASE;

typedef enum
{
  /*
   *  Label was not terminated
   */
  SOC_PPC_DIAG_MPLS_TERM_TYPE_NONE = 0,
  /*
   *  Label was terminated according to Range
   */
  SOC_PPC_DIAG_MPLS_TERM_TYPE_RANGE = 1,
  /*
   *  Label was terminated according to lookup in LIF table
   */
  SOC_PPC_DIAG_MPLS_TERM_TYPE_LIF_LKUP = 2,
  /*
   *  Number of types in SOC_PPC_DIAG_MPLS_TERM_TYPE
   */
  SOC_PPC_NOF_DIAG_MPLS_TERM_TYPES = 3
}SOC_PPC_DIAG_MPLS_TERM_TYPE;

typedef enum
{
  /*
   *  Trace of packet over the LIF table, including hit bit
   *  for each LIF entry
   */
  SOC_PPC_DIAG_PKT_TRACE_LIF = 1,
  /*
   *  Trace of packet over tunnel termination by range
   */
  SOC_PPC_DIAG_PKT_TRACE_TUNNEL_RNG = 0x2,
  /*
   *  Trace of packet over traps hit bit
   */
  SOC_PPC_DIAG_PKT_TRACE_TRAP = 0x4,
  /*
   *  Trace of packet over FEC hit bit
   */
  SOC_PPC_DIAG_PKT_TRACE_FEC = 0x8,
  /*
   *  Clear packets drop log at egress
   */
  SOC_PPC_DIAG_PKT_TRACE_EG_DROP_LOG = 0x10,
  /*
   *  Trace of packet over the FLP table
   */
  SOC_PPC_DIAG_PKT_TRACE_FLP = 0x20,
  /*
   *  Trace of packet over the PMF table
   */
  SOC_PPC_DIAG_PKT_TRACE_PMF = 0x40,
  /*
   *  Clear all the above traces
   */
  SOC_PPC_DIAG_PKT_TRACE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_DIAG_PKT_TRACE
   */
  SOC_PPC_NOF_DIAG_PKT_TRACES = 6
}SOC_PPC_DIAG_PKT_TRACE;

typedef enum
{
  /*
   *  Trace of packet over the LIF table, including hit bit
   *  for each LIF entry
   */
  SOC_PPC_DIAG_EG_DROP_REASON_NONE = 0x0,
  /*
   *  If set then a packet was discarded by CNM intercept
   *  filter. Cleared when '1' is written.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_CNM = 0x1,
  /*
   *  If set then a packet was discarded by CFM trap filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_CFM_TRAP = 0x2,
  /*
   *  If set then a packet was discarded by VSI translation
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_NO_VSI_TRANSLATION = 0x4,
  /*
   *  If set then a packet was discarded by DSS stacking
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_DSS_STACKING = 0x8,
  /*
   *  If set then a packet was discarded by LAG multicast
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_LAG_MULTICAST = 0x10,
  /*
   *  If set then a packet was discarded by source port
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_EXCLUDE_SRC = 0x20,
  /*
   *  If set then a packet was discarded by VLAN membership
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_VLAN_MEMBERSHIP = 0x40,
  /*
   *  If set then a packet was discarded by Unacceptable Frame
   *  type filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_UNACCEPTABLE_FRAME_TYPE = 0x80,
  /*
   *  If set then a packet was discarded by source port
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_SRC_EQUAL_DEST = 0x100,
  /*
   *  If set then a packet was discarded by unknown DA filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_UNKNOWN_DA = 0x200,
  /*
   *  If set then a packet was discarded by split-horizon
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_SPLIT_HORIZON = 0x400,
  /*
   *  If set then a packet was discarded by private vlan
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_PRIVATE_VLAN = 0x800,
  /*
   *  If set then a packet was discarded by TTL scope filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_TTL_SCOPE = 0x1000,
  /*
   *  If set then a packet was discarded by MTU violation
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_MTU_VIOLATION = 0x2000,
  /*
   *  If set then a packet was discarded by Trill TTL filter
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_TRILL_TTL_ZERO = 0x4000,
  /*
   *  If set then a packet was discarded by Trill same
   *  interface filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_TRILL_SAME_INTERFACE = 0x8000,
  /*
   *  If set then a packet was discarded by bounce back
   *  filter.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_BOUNCE_BACK = 0x10000,
  /*
   *  If set then a packet was discarded because illegal EEP.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_ILLEGAL_EEP = 0x20000,
  /*
   *  EEI type (MSB) doesn't match any application (TRILL, MiM, MPLS, out-lif)
   */
  SOC_PPC_DIAG_EG_DROP_REASON_ILLEGAL_EEI = 0x40000,
  /*
   *  aslo EEI and out-LIF indicate PHP opertaion
   */
  SOC_PPC_DIAG_EG_DROP_REASON_PHP_CONFLICT = 0x80000,
  /*
   *  Egress PP performed PHP to IP and IP version error was identified.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_POP_IP_VERSION_ERR = 0x100000,
  /*
   *  build tunnel (TRILL/MPLS) for snooped/mirrored copy
   */
  SOC_PPC_DIAG_EG_DROP_REASON_MODIFY_SNOOPED_PACKET = 0x200000,
  /*
   *  build IP tunnel for snooped/mirrored copy
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IP_TUNNEL_SNOOPED_PACKET = 0x400000,
  /*
   *  egress procressing need to build LL, but packet does not point to EEDB of type LL.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_EEDB_LINK_LAYER_ENTRY_NEEDED = 0x2000000,
  /*
   *  egress port stp state is block
   */
  SOC_PPC_DIAG_EG_DROP_REASON_STP_BLOCK = 0x4000000,
  /*
   *  outlif with action drop set
   */
  SOC_PPC_DIAG_EG_DROP_REASON_OUT_LIF_WITH_DROP = 0x8000000,
  /*
   *  Last EEDB entry is not AC
   */
  SOC_PPC_DIAG_EG_DROP_REASON_EEDB_LAST_ENTRY_NOT_AC = 0x10000000,
  /*
   *  OTM value is invalid.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_OTM_INVALID = 0x20000000,
  /*
   *  IPv4 packet has a version different than 4.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_VERSION_ERR = 0x40000000,
  /*
   *  IPv6 packet has a version different than 6.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV6_VERSION_ERR = (int)0x80000001,
  /*
   *  IPv4 checksum error.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_CHECKSUM_ERR = (int)0x80000002,
  /*
   *  the packet is IPv4 and IHL is less than 5.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_IHL_SHORT = (int)0x80000004,
  /*
   *  the packet is IPv4 and total length is less than 20.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_TOTAL_LEGNTH =(int) 0x80000008,
  /*
   *  the packet is IPv4 and incoming TTL is 1.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_TTL_1 = (int)0x80000010,
  /*
   *  the packet is IPv6 and incoming TTL is 1.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV6_TTL_1 = (int)0x80000020,
  /*
   *  packet has IPv4 options.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_WITH_OPTIONS = (int)0x80000040,
  /*
   *  packet type is IPv4 and TTL is 0.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_TTL_0 = (int)0x80000080,
  /*
   *  packet type is IPv6 and TTL is 0.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV6_TTL_0 = (int)0x80000100,
  /*
   *  packet type is IPv4 and Source IP equals Destination IP.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_SIP_EQUAL_DIP = (int)0x80000200,
  /*
   *  packet type is IPv4 and Destination IP equals 0.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_DIP_IS_ZERO = (int)0x80000400,
  /*
   *  packet type is IPv4 and Source IP is Multicast.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV4_SIP_IS_MC = (int)0x80000800,
  /*
   *  packet type is IPv6 and Source IP is Multicast.
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV6_SIP_IS_MC = (int)0x80001000,
  /*
   *  packet type is IPv6 and Destination IP is unspecified (i.e. all zeros).
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV6_DIP_UNSPECIFIED = (int)0x80002000,
  /*
   *  packet type is IPv6 and Source IP is unspecified (i.e. all zeros).
   */
  SOC_PPC_DIAG_EG_DROP_REASON_IPV6_SIP_UNSPECIFIED = (int)0x80004000,
  /*
   *  loopback is detected: Source IP or Destination IP equal 0:0:0:0:0:0:0:1.
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_LOOPBACK = (int)0x80008000,
  /*
   *  Packet is filtered or trapped as packet-type is IPv6 and Hop-by-Hop header detected (next header is zero).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_HOP_BY_HOP = (int)0x80010000,
  /*
   *  Packet is filtered or trapped as packet-type is IPv6 and Destination IP is link local (Dest-IP[127:118] is equal to 10`b1111_1110_10).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_LINK_LOCAL_DEST = (int)0x80020000,
  /*
   *  Packet is filtered or trapped as packet-type is IPv6 and Destination IP is site local (Dest-IP[127:118] is equal to 10`b1111_1110_11).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_SITE_LOCAL_DEST = (int)0x80040000,
  /*
   *  Packet is filtered or trapped as packet-type is IPv6 and Source IP is link local (Dest-IP[127:118] is equal to 10`b1111_1110_10).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_LINK_LOCAL_SRC = (int)0x80080000,
  /*
   *  Packet is filtered or trapped as packet-type is IPv6 and Source IP is site local (Dest-IP[127:118] is equal to 10`b1111_1110_11).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_SITE_LOCAL_SRC = (int)0x80100000,
  /*
   *  Destination address is IPv4 compatible (Dst-IP[127:32] is equal to 0).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_IPV4_COMPATIBLE = (int)0x80200000,
  /*
   *  destination address is mapped to IPv4 (Dst-IP[127:32] is equal to 96`h0000_FFFF_0000_0000_0000_0000).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_IPV4_MAPPED = (int)0x80400000,
  /*
   *  destination address is multicast (Dst-IP[127:120] is equal to 8`hFF).
   */
   SOC_PPC_DIAG_EG_DROP_REASON_IPV6_DEST_MC = (int)0x80800000,
  /*
   *  Number of types in SOC_PPC_DIAG_EG_DROP_REASON
   */
  SOC_PPC_NOF_DIAG_EG_DROP_REASONS = 56
}SOC_PPC_DIAG_EG_DROP_REASON;

typedef enum
{
  /*
   *  TC field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_TC = 0x1,
  /*
   *  DP field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_DP = 0x2,
  /*
   *  DEST field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_DEST = 0x4,
  /*
   *  Meter1 field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_METER1 = 0x8,
  /*
   *  Meter2 field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_METER2 = 0x10,
  /*
   *  Meter command field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_MTR_CMD = 0x20,
  /*
   *  Counter1 field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_COUNTER1 = 0x40,
  /*
   *  Counter2 field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_COUNTER2 = 0x80,
  /*
   *  CUD field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_CUD = 0x100,
  /*
   *  ETH Meter pointer field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_ETH_METER_PTR = 0x200,
  /*
   *  Ingress shaping DA field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_ING_SHAPING_DA = 0x400,
  /*
   *  ECN Capable field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_ECN_CAPABLE = 0x800,
  /*
   *  CNI field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_CNI = 0x1000,
  /*
   *  DA Type field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_DA_TYPE = 0x2000,
  /*
   *  ST VSQ pointer field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_ST_VSQ_PTR = 0x4000,
  /*
   *  LAG LB Key field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_LAG_LB_KEY = 0x8000,
  /*
   *  Ignore CP field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_IGNORE_CP = 0x10000,
  /*
   *  Snoop Id field is valid
   */
  SOC_PPC_DIAG_PKT_TM_FIELD_SNOOP_ID = 0x20000,
  /*
   *  Number of types in SOC_PPC_DIAG_PKT_TM_FIELD
   */
  SOC_PPC_NOF_DIAG_PKT_TM_FIELDS = 18
}SOC_PPC_DIAG_PKT_TM_FIELD;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Buffer used by diagnostics APIs to hold packets header
   *  and other information. Information/packet is copied to
   *  buff starting from buff[0] lsb.
   */
  uint32 buff[SOC_PPC_DIAG_BUFF_MAX_SIZE];
  /*
   *  the actual length of the returned buffer (in longs)
   */
  uint32 buff_len;

} SOC_PPC_DIAG_BUFFER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Flavor indicates/affects the way the diagnostic APIs
   *  work. Bitmap as decoded by SOC_PPC_DIAG_FLAVOR
   */
  uint32 flavor;

} SOC_PPC_DIAG_MODE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VSI decision result.
   */
  uint32 vsi;

} SOC_PPC_DIAG_VSI_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Lookup number, some DB has more than one lookup
   *  lkup_num - 0 first (VT), 1 - second (TT)
   */
  uint32 lkup_num;

  /*
   *  for SEM 0 (SEM-A), 1 (SEM-B), 2 (TCAM).
   *  for LEM has to be 0.                           .
   *  ARAD only.
   */
  uint32 bank_id;

} SOC_PPC_DIAG_DB_USE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Lookup number, some DB has more than one lookup
   */
  uint32 lkup_num;
  /*
   *  The lookup usage for forwarding/PMF/Egress
   */
  SOC_PPC_DIAG_TCAM_USAGE lkup_usage;

} SOC_PPC_DIAG_TCAM_USE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Packet header injected into the device. Size is up to
   *  128 Bytes.
   */
  SOC_PPC_DIAG_BUFFER packet_header;
  /*
   *  The local TM port from which the packet entered this
   *  device.
   */
  uint32 in_tm_port;
  /*
   *  The system port from which the packet entered the
   *  system. This is the system port to be learned when
   *  learning committed according to incoming-src-port.
   */
  SOC_SAND_PP_SYS_PORT_ID src_sys_port;
  /*
   *  The incoming local PP port the packet associated to,
   *  this is the local port the packet processing refers to.
   */
  SOC_PPC_PORT in_pp_port;
  /*
   *  Port type which according the packet will be processed.
   *  Port Type (RAW, TM, Ethernet, Programmable, CPU, ...)
   */
  SOC_TMC_PORT_HEADER_TYPE pp_context;
  /*
   *  Packet qualifier that may be used during the processing.
   *  Currently used to determine whether CFM-trapping is
   *  applicable on the packet.
   */
  uint32 packet_qual;


} SOC_PPC_DIAG_RECEIVED_PACKET_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Was packet trapped? TRUE then packet was trapped and
   *  processed/forwarded according to below trapped. FALSE:
   *  then packet was not trapped and forwarded/processed
   *  according to normal processing. Packet is trapped only
   *  if the strength of the resolved trap is higher than
   *  processing strength. See soc_ppd_diag_frwrd_lkup_info_get()
   *  to find out according to what processing the packet was
   *  forwarded according.
   */
  uint8 is_pkt_trapped;
  /*
   *  Trap code
   */
  SOC_PPC_TRAP_CODE code;
  /*
   *  Forwarding action information as set by
   *  soc_ppd_trap_frwrd_profile_info_set() for the above trap
   *  code.
   */
  SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO info;

} SOC_PPC_DIAG_TRAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Trap code of the snoop action
   */
  SOC_PPC_TRAP_CODE code;
  /*
   *  Forwarding action information as set by
   *  soc_ppd_trap_snoop_profile_info_set() for the above trap
   *  code.
   */
  SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO info;

} SOC_PPC_DIAG_SNOOP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Bitmap indicates which traps/snoops were subject to be
   *  applied, i.e. traps/snoops that the trigger condition
   *  was fulfilled for them. BIT y is set to '1' if and only
   *  if the SOC_PPC_TRAP_CODE y condition was fulfilled. Note:
   *  when API called after injecting more than one packet
   *  then this attribute indicates traps for all injected
   *  packets. Note: to find out what is the committed
   *  trap/snoop (if any) see committed_trap and
   *  committed_snoop below.
   */
  uint32 trap_stack[SOC_PPC_NOF_TRAP_CODES/SOC_SAND_NOF_BITS_IN_UINT32];
  /*
   *  Trap strength for the trap stack. Only traps in the stack have a value.
   */
  uint8 trap_stack_strength[SOC_PPC_NOF_TRAP_CODES];
  /*
   *  additional trap strengths for the trap stack.
   *  currently is used to store tunnel termination strengths for bcmRxIpv4Ttl0, bcmRxIpv4Ttl1
   *  bcmRxIpv6HopCount0, bcmRxIpv6HopCount1 which are additional to the forwarding strengths
   *  stored in trap_stack_strength
   */
  uint8 additional_trap_stack_strength[SOC_PPC_NOF_TRAP_CODES];
  /*
   *  The committed trap action, (if any) packet was trapped
   *  if is_pkt_trapped is TRUE.
   */
  SOC_PPC_DIAG_TRAP_INFO committed_trap;
  /*
   *  The committed snoop. If any. Snoop-command zero means no
   *  snooping was performed
   */
  SOC_PPC_DIAG_SNOOP_INFO committed_snoop;
  /*
   *  The trap that cause updating the destination of the
   *  packet.
   */
  SOC_PPC_DIAG_TRAP_INFO trap_updated_dest;

} SOC_PPC_DIAG_TRAPS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  System port that designated to the CPU.
   */
  uint32 cpu_dest;

} SOC_PPC_DIAG_TRAP_TO_CPU_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPv4 subnet IP-address/Prefix
   */
   SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY subnet;
  /*
   *  VRF-id
   */
   uint32 vrf_id;

} SOC_PPC_DIAG_IPV4_VPN_ROUTE_KEY;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  System port id
   */
   SOC_SAND_PP_SYS_PORT_ID sys_port_id;
  /*
   *  MAC key
   */
   SOC_SAND_PP_MAC_ADDRESS mac_key;

} SOC_PPC_DIAG_TRILL_ADJ_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * Trill keys as defined in ISEM
   */  
  /* 
   * 0 - TRILL_MC_RPF 
   * 1 - TRILL_NICK 
   * 2 - TRILL_APPOINTED_FORWARDER 
   * 3 - TRILL_NATIVE_INNER_TPID 
   * 4 - TRILL_VSI 
   */
  uint32 trill_key_type;
  uint32 nick_name;
  uint32 ing_nick;
  uint32 dist_tree;
  uint32 link_adj_id;
  uint32 native_inner_tpid;
  uint32 port;
  uint32 vsi;
  uint32 high_vid;
  uint32 low_vid;
  uint32 flags;  
} SOC_PPC_DIAG_TRILL_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Relevant when diag_flavor = SOC_PPC_DIAG_FLAVOR_RAW
   */
  uint32 raw[6];
  /*
   *  AC, key: <port, Vlan, vlan>
   */
  SOC_PPC_L2_LIF_AC_KEY ac;
  /*
   *  PWE, key: <VC-label>
   */
  SOC_SAND_PP_MPLS_LABEL pwe;
  /*
   *  MPLS, key: <VSI, label>
   */
  SOC_PPC_MPLS_LABEL_RIF_KEY mpls;
  /*
   *  IP-tunnel, key: <DIP,SIP>
   */
  SOC_PPC_RIF_IP_TERM_KEY ip_tunnel;
  /*
   *  MAC-in-MAC,key: <is-domain, isid>
   */
  SOC_PPC_L2_LIF_ISID_KEY mim;
  /*
   *  TRILL,key: <nick_name>
   */
  uint32 nick_name; 
  /* 
   *  ARAD Trill key
   */
  SOC_PPC_DIAG_TRILL_KEY trill;
  /* 
   * l2gre, key: <vsid>
   */
  SOC_PPC_L2_LIF_GRE_KEY l2gre;
  /* 
   * vxlan, key: <vni>
   */
  SOC_PPC_L2_LIF_VXLAN_KEY vxlan;
  /* 
   * extender, key: <port, e-cid, c-vid>
   */
  SOC_PPC_L2_LIF_EXTENDER_KEY extender;

} SOC_PPC_DIAG_LIF_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  lif key
   */
   SOC_PPC_DIAG_LIF_KEY key;
  /*
   *  lif type
   */
   SOC_PPC_DIAG_LIF_LKUP_TYPE type;

} SOC_PPC_DIAG_EXTEND_P2P_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Relevant when diag_flavor = SOC_PPC_DIAG_FLAVOR_RAW
   */
  uint32 raw[4];
  /*
   *  MAC-Table, key: <FID, DA>
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY mact;
  /*
   *  Backbone -MAC-Table, key: <B-FID, B-DA>
   */
  SOC_PPC_BMACT_ENTRY_KEY bmact;
  /*
   *  IP-Hosts-Table, key: <VRF, DIP>
   */
  SOC_PPC_DIAG_IPV4_VPN_ROUTE_KEY host;
  /*
   *  ILM-Table, key: <[exp],[in-RIF], [in-port], label>
   */
  SOC_PPC_FRWRD_ILM_KEY ilm;
  /*
   *  SA-Based-VID / Authentication,key: <SA>
   */
  SOC_SAND_PP_MAC_ADDRESS sa_auth;
  /*
   *  TRILL adjacent key: <Port, SA>
   */
  SOC_PPC_DIAG_TRILL_ADJ_KEY trill_adj;
  /*
   *  P2P key
   */
  SOC_PPC_DIAG_EXTEND_P2P_KEY extend_p2p_key;

} SOC_PPC_DIAG_LEM_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Relevant when diag_flavor = SOC_PPC_DIAG_FLAVOR_RAW
   */
  uint32 raw[2];
  /*
   *  AC, Value: AC-attributes<vlan-edit,...>
   */
  SOC_PPC_L2_LIF_AC_INFO ac;
  /*
   *  PWE, Value: PWE-attributes <P2P/MP, learn-info,...)
   */
  SOC_PPC_L2_LIF_PWE_INFO pwe;
  /*
   *  MPLS, Value: <VRF-id, Rif-id, ...>
   */
  SOC_PPC_MPLS_TERM_INFO mpls;
  /*
   *  IP-tunnel, Value: <VRF-id, Rif-id, ...>
   */
  SOC_PPC_RIF_IP_TERM_INFO ip;
  /*
   *  MAC-in-MAC,Value: <P2P/MP, default-dest, learn-enable,
   *  ...>
   */
  SOC_PPC_L2_LIF_ISID_INFO mim;
  /* 
   *  Trill info
   */
  SOC_PPC_L2_LIF_TRILL_INFO trill;
  /* 
   * L2GRE info
   */
  SOC_PPC_L2_LIF_GRE_INFO l2gre;
  /* 
   * vxlan info
   */
  SOC_PPC_L2_LIF_VXLAN_INFO vxlan;
  /* 
   * extender info
   */
  SOC_PPC_L2_LIF_EXTENDER_INFO extender;

} SOC_PPC_DIAG_LIF_VALUE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Lookup type
   */
  SOC_PPC_DIAG_LIF_LKUP_TYPE type;
  /*
   *  Lookup key
   */
  SOC_PPC_DIAG_LIF_KEY key;
  /*
   *  The Base index in the LIF table
   */
  uint32 base_index;
  /*
   *  Op-ode ID for COS mapping
   */
  uint32 opcode_id;
  /*
   *  Lookup result
   */
  SOC_PPC_DIAG_LIF_VALUE value;
  /*
   *  Is lookup was success and result valid. if FALSE then
   *  value is not relevant
   */
  uint8 found;

} SOC_PPC_DIAG_LIF_LKUP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Value of the rule.
   */
  uint32 val[SOC_PPC_DIAG_TCAM_KEY_NOF_UINT32S_MAX];
  /*
   *  Mask of the rule. Set '1' for the corresponding bit to
   *  be meaningful
   */
  uint32 mask[SOC_PPC_DIAG_TCAM_KEY_NOF_UINT32S_MAX];
  /*
   *  Key length in bytes
   */
  uint32 length;

} SOC_PPC_DIAG_TCAM_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The FEC pointer resulted from TCAM lookup, relevant for
   *  forwarding lookup.
   */
  uint32 fec_ptr;

} SOC_PPC_DIAG_TCAM_VALUE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Relevant when diag_flavor = SOC_PPC_DIAG_FLAVOR_RAW
   */
  uint32 raw[2];
  /*
   *  MAC-Table, value: <destination, ...>
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE mact;
  /*
   *  Backbone -MAC-Table, Value: <Destination, ...>
   */
  SOC_PPC_BMACT_ENTRY_INFO bmact;
  /*
   *  IP-Hosts-Table, Value: <FEC-ptr>
   */
  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO host;
  /*
   *  ILM-Table, Value: <forwarding decision>
   */
  SOC_PPC_FRWRD_DECISION_INFO ilm;
  /*
   *  SA-Based-VID / Authentication,Value: <Vid, permit on
   *  port, vlan, ...>
   */
  SOC_PPC_LLP_SA_AUTH_MAC_INFO sa_auth;
  /*
   *  SA-Based-VID : <Vid, for tagged/untagged >
   */
  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO sa_vid_assign;

} SOC_PPC_DIAG_LEM_VALUE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ethernet encapsulation type (Ethernet 2, LLC, SNAP)
   */
  SOC_SAND_PP_ETH_ENCAP_TYPE encap_type;
  /*
   *  Found TPID indexes.
   */
  SOC_PPC_LLP_PARSE_INFO tag_fromat;
  /*
   *  Found TPIDs SOC_SAND_PP_VLAN_TAG[0] is outer vlan, and
   *  SOC_SAND_PP_VLAN_TAG[1] is inner VLAN.
   */
  SOC_SAND_PP_VLAN_TAG vlan_tags[2];
  /*
   *  Vlan tag format (S-C, C, Priority-C,...) Relevant only for
   *  Ethernet as most outer header.
   */
  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT vlan_tag_format;
  /*
   *  L2 next protocol (e.g. ARP, IPv4, ...). Relevant only for
   *  Ethernet as most outer header.
   */
  SOC_PPC_L2_NEXT_PRTCL_TYPE next_prtcl;

} SOC_PPC_DIAG_PARSING_L2_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Is this label is bottom of stack
   */
  uint8 bos;

} SOC_PPC_DIAG_PARSING_MPLS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  L2 next protocol (e.g. ARP, IPv4, ...)
   */
  SOC_PPC_L3_NEXT_PRTCL_TYPE next_prtcl;
  /*
   *  Is IP Multicast
   */
  uint8 is_mc;
  /*
   *  Is packet fragmented, relevant only for IPv4 for IPv6
   *  always set to FALSE.
   */
  uint8 is_fragmented;
  /*
   *  Has the header check sum error or version error
   */
  uint8 hdr_err;

} SOC_PPC_DIAG_PARSING_IP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Header type, the below fields are relevant depending on
   *  header type
   */
  SOC_PPC_PKT_HDR_TYPE hdr_type;
  /*
   *  The offset of this header in the header stack (in
   *  bytes). Header-Offset0 is start of packet
   */
  uint32 hdr_offset;
  /*
   *  Ethernet Header information
   */
  SOC_PPC_DIAG_PARSING_L2_INFO eth;
  /*
   *  IPv4/6 header information
   */
  SOC_PPC_DIAG_PARSING_IP_INFO ip;
  /*
   *  MPLS header information
   */
  SOC_PPC_DIAG_PARSING_MPLS_INFO mpls;

} SOC_PPC_DIAG_PARSING_HEADER_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The network header stack. E.g. (Eth, IPv4oEth,
   *  MPLSoEth,...)
   */
  SOC_PPC_PKT_HDR_STK_TYPE hdr_type;
  int pfc_hw;

  /* 
   *  initial vlan id, if packet tagged initial_vlan_id = packet.VID
   *  if packet untagged, initial_vlan_id = port.PVID
   */ 
  SOC_SAND_PP_VLAN_ID initial_vid;

  /*
   *  Header information, number of valid entries and type of
   *  each entry according to hdr_type
   */
  SOC_PPC_DIAG_PARSING_HEADER_INFO hdrs_stack[SOC_PPC_DIAG_MAX_NOF_HDRS];

} SOC_PPC_DIAG_PARSING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  ID of range, 0xFF indicates not match for range
   */
  uint8 range_index;
  /*
   *  Range of MPLS labels.
   */
  SOC_PPC_MPLS_TERM_LABEL_RANGE range;

} SOC_PPC_DIAG_TERM_MPLS_LABEL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Terminated headers (None, Ethernet, IPv4 and Ethernet,
   *  ...)
   */
  SOC_PPC_PKT_TERM_TYPE term_type;
  /*
   *  The header to be used for forwarding lookup, after all
   *  above terminations
   */
  SOC_PPC_PKT_FRWRD_TYPE frwrd_type;
  /*
   *  Is 2nd MyMac
   */
  uint8 is_scnd_my_mac;

} SOC_PPC_DIAG_TERM_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VRF
   */
  uint32 vrf;
  /*
   *  subnet
   */
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY key;

} SOC_PPC_DIAG_IPV4_VPN_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VRF
   */
  uint32 vrf;
  /*
   *  subnet
   */
  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY key;

} SOC_PPC_DIAG_IPV6_VPN_KEY;

#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * Vrf
 */
    uint16 vrf;
/*
 * Source ip
 */
    uint32 sip;
/*
 * Destination ip
 */
    uint32 dip;
}SOC_PPC_DIAG_IPV4_UNICAST_RPF;

typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * Vrf
 */
    uint16 vrf;
/*
 * In_Rif
 */
    uint16 in_rif;
/*
 * Source ip
 */
    uint32 sip;
/*
 * Destination ip
 */
    uint32 dip;
}SOC_PPC_DIAG_IPV4_MULTICAST;

typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * Vrf
 */
    uint16 vrf;
/*
 * Source ip
 */
    SOC_SAND_PP_IPV6_ADDRESS sip;
/*
 * Destination ip
 */
    SOC_SAND_PP_IPV6_ADDRESS dip;
}SOC_PPC_DIAG_IPV6_UNICAST_RPF;

typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * Vrf
 */
    uint16 vrf;
/*
 * In_Rif
 */
    uint16 in_rif;
/*
 * Source ip
 */
    SOC_SAND_PP_IPV6_ADDRESS sip;
/*
 * Destination ip
 */
    SOC_SAND_PP_IPV6_ADDRESS dip;
}SOC_PPC_DIAG_IPV6_MULTICAST;

typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * In_Rif
 */
    uint16 in_rif;
/*
 * In_Port
 */
    uint8 in_port;
/*
 * exp
 */
    uint8 exp;
/*
 * mpls label
 */
    uint32 mpls_label;
}SOC_PPC_DIAG_MPLS;

typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * Egress Nick
 */
    uint16 egress_nick;
}SOC_PPC_DIAG_TRILL_UNICAST;

typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * ESDAI
 */
    uint8 esdai;
/*
 * FID/VSI
 */
    uint16 fid_vsi;
/*
 * Dist Tree Nick
 */
    uint16 dist_tree_nick;
}SOC_PPC_DIAG_TRILL_MULTICAST;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MAC-Table, key: <FID, DA>
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY mact;
  /*
   *  Backbone -MAC-Table, key: <B-FID, B-DA>
   */
  SOC_PPC_BMACT_ENTRY_KEY bmact;
  /*
   *  IPv4-UC/VPN routing key: <VRF, DIP>
   */
  SOC_PPC_DIAG_IPV4_VPN_KEY ipv4_uc;
  /*
   *  IPv4-MC routingkey: <InRIF, Group, SIP >
   */
  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY ipv4_mc;
  /*
   *  IPv6-UC/VPN routing key: <VRF, DIP>
   */
  SOC_PPC_DIAG_IPV6_VPN_KEY ipv6_uc;
  /*
   *  IPv6-MC routingkey: <InRIF, Group >
   */
  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY ipv6_mc;
  /*
   *  ILM-Table, key: <[exp],[in-RIF], [in-port], label>
   */
  SOC_PPC_FRWRD_ILM_KEY ilm;
  /*
   *  TRILL-MC-Key, key: < nickname >
   */
  uint32 trill_uc;
  /*
   *  TRILL-MC-Key, key: < adjacent_nick, ing_nick, tree_nick,
   *  fid>
   */
  SOC_PPC_TRILL_MC_ROUTE_KEY trill_mc;
  /*
   *  ILM-Table, key: <[exp],[in-RIF], [in-port], label>
   */
  SOC_PPC_FRWRD_FCF_ROUTE_KEY fcf;
  /*
   *  Relevant when diag_flavor = SOC_PPC_DIAG_FLAVOR_RAW
   */
  uint32 raw[2];
#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
  /*
   * ipv4 unicast rpf
   */
  SOC_PPC_DIAG_IPV4_UNICAST_RPF kbp_ipv4_unicast_rpf;
  /*
   * ipv4 multicast
   */
  SOC_PPC_DIAG_IPV4_MULTICAST kbp_ipv4_multicast;
  /*
   * ipv6 unicast rpf
   */
  SOC_PPC_DIAG_IPV6_UNICAST_RPF kbp_ipv6_unicast_rpf;
  /*
   * ipv6 multicast
   */
  SOC_PPC_DIAG_IPV6_MULTICAST kbp_ipv6_multicast;
  /*
   * mpls
   */
  SOC_PPC_DIAG_MPLS kbp_mpls;
  /*
   * trill unicast
   */
  SOC_PPC_DIAG_TRILL_UNICAST kbp_trill_unicast;
  /*
   * trill multicast
   */
  SOC_PPC_DIAG_TRILL_MULTICAST kbp_trill_multicast;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */

} SOC_PPC_DIAG_FRWRD_LKUP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MAC-Table entry value, value: <destination, ...>
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE mact;
  /*
   *  Backbone -MAC-Table entry value, Value: <Destination, ...>
   */
  SOC_PPC_BMACT_ENTRY_INFO bmact;
  /*
   *  Forwarding decision, For IP routing has to be FECFor
   *  Trill UC has to be FECFro Trill MC has to be MC-group
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_decision;
  /*
   *  Result upon IPv4-host lookup
   */
  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO host_info;
  /*
   *  Forwarding decision, For IP routing has to be FECFor
   *  Trill UC has to be FECFro Trill MC has to be MC-group
   */
  uint32 raw[2];

} SOC_PPC_DIAG_FRWRD_LKUP_VALUE;

#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * match status
 */
    uint8 match_status;
/*
 * is dynamic
 */
    uint8 is_synamic;
/*
 * P2P Service
 */
    uint8 p2p_service;
/*
 * Identifier
 */
    uint8 identifier;
/*
 * Out LIF Valid (If identifier == 0x0)
 */
    uint8 out_lif_valid;
/*
 * Out LIF (If identifier == 0x0)
 */
    uint16 out_lif;
/*
 * Destination (If identifier == 0x0)
 */
    SOC_PPC_FRWRD_DECISION_INFO destination;
/*
 * EEI (If identifier == 0x1)
 */
    uint32 eei;
/*
 * FEC-Ptr (If identifier == 0x1)
 */
    uint16 fec_ptr;
}SOC_PPC_DIAG_IP_REPLY_RECORD;

typedef struct{
    SOC_SAND_MAGIC_NUM_VAR
/*
 * Match status
 */
    uint8 match_status;
/*
 * Dest id
 */
    uint16 dest_id;
} SOC_PPC_DIAG_SECOND_IP_REPLY_RECORD;

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  According to which engine/processing the packet was
   *  forwarded:Bridged according to outer-MACRouted according
   *  IPv4/6 UC/MCMPLS label switchingRBRIDGE for TRILLOr
   *  Bridged after termination (VPLS, MAC-in-MAC). Note: this
   *  is the forwarding according to 'forwarding lookup' it
   *  not the finally forwarding destination the packet was
   *  sent to, as there the PMF and Traps that may overwrite
   *  this decision, to get final forwarding decision use
   *  soc_ppd_diag_frwrd_decision_get()
   */
  SOC_PPC_DIAG_FWD_LKUP_TYPE frwrd_type;
  /*
   *  The placement of the header the packet is forwarded
   *  according, 0: is outer most, 1: is inner header and so
   *  on. Note: if packet has consecutive MPLS headers, then
   *  each has its own index.
   */
  uint8 frwrd_hdr_index;
  /*
   *  The key used for forwarding lookup
   */
  SOC_PPC_DIAG_FRWRD_LKUP_KEY lkup_key;
  /*
   *  the lookup succeeded or failed
   */
  uint8 key_found;
  /*
   *  The lookup result, mainly includes the forwarding
   *  decision
   */
  SOC_PPC_DIAG_FRWRD_LKUP_VALUE lkup_res;
#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
/*
 * The lookup is external
 */
  uint8 is_kbp;
/*
 * ip reply record
 */
  SOC_PPC_DIAG_IP_REPLY_RECORD ip_reply_record;
/*
 * second ip reply result
 */
  SOC_PPC_DIAG_SECOND_IP_REPLY_RECORD second_ip_reply_result;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */

} SOC_PPC_DIAG_FRWRD_LKUP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Whether learning is enabled/relevant for the packet. IF
   *  false then no learning will be performed for the packet.
   */
  uint8 valid;
  /*
   *  TRUE: then learning is in ingress device. FALSE: then
   *  learning is in egress device.
   */
  uint8 ingress;
  /*
   *  Is this new key,if TRUE this is learn operation.if FALSE
   *  this is transplant operation.
   */
  uint8 is_new_key;
  /*
   *  The MACT key, e.g. (FID, MAC address)
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY key;
  /*
   *  The value of the MACT entry including forwarding and
   *  aging information
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE value;

} SOC_PPC_DIAG_LEARN_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Committed vlan edit command ID.
   */
  uint32 cmd_id;
  /*
   *  Committed vlan edit command information.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO cmd_info;
  /*
   *  When VID/PCP source is "SRC_AC_EDIT_INFO"
   *  then this information is considered
   */
  SOC_SAND_PP_VLAN_TAG ac_tag;
  /*
   *  When VID/PCP source is "SRC_AC_EDIT_INFO_2"
   *  then this information is considered
   *  Not supported for PetraB
   */
  SOC_SAND_PP_VLAN_TAG ac_tag2;

  /*
   *  When set, vlan edit command is taken from LIF.IVEC-command
   *  otherwise, from VTT-LLVP (same as Arad) 
   *  Supported in Jericho and above
   */
  uint32 adv_mode;

} SOC_PPC_DIAG_VLAN_EDIT_RES;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Forwarding decision including destination and associated
   *  information
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_decision;

 /*
  * ARAD only.
  * outlif value in this stage
  */
  uint32 outlif;

} SOC_PPC_DIAG_FRWRD_DECISION_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Forwarding information in phases of processing.
   */
  SOC_PPC_DIAG_FRWRD_DECISION_INFO frwrd[SOC_PPC_NOF_DIAG_FRWRD_DECISION_PHASES];

  /*
   *  Trap information in phases of processing.
   */
  SOC_PPC_TRAP_INFO trap[SOC_PPC_NOF_DIAG_FRWRD_DECISION_PHASES];

  /*
   *  Trap information in phases of processing.
   */
  uint32 trap_qual[SOC_PPC_NOF_DIAG_FRWRD_DECISION_PHASES];

} SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Bitmap indicates which field are valid, bitmap
   *  interpreted as described in SOC_PPC_DIAG_PKT_TM_FIELD
   */
  uint32 valid_fields;
  /*
   *  Forwarding destination, only destination is relevant.
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_decision;
  /*
   *  Traffic class. Range: 0 - 7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Drop precedence. Range: 0 - 3.
   */
  SOC_SAND_PP_DP dp;
  /*
   *  First meter ID associated with the packet
   */
  uint32 meter1;
  /*
   *  Second meter ID associated with the packet
   */
  uint32 meter2;
  /*
   *  DP meter command, set how to consider the metering
   *  result.
   */
  uint32 dp_meter_cmd;
  /*
   *  First Counter ID associated with the packet
   */
  uint32 counter1;
  /*
   *  Second Counter ID associated with the packet
   */
  uint32 counter2;
  /*
   *  Copy unique Data, for Multicast packets.
   */
  uint32 cud;
  /*
   *  Ethernet Meter Pointer
   */
  uint32 eth_meter_ptr;
  /*
   *  Ingress Shaping Destination
   */
  uint32 ingress_shaping_da;
  /*
   *  ECN Capable
   */
  uint32 ecn_capable;
  /*
   *  CNI
   */
  uint32 cni;
  /*
   *  DA Type
   */
  SOC_SAND_PP_ETHERNET_DA_TYPE da_type;
  /*
   *  ST VSQ pointer
   */
  uint32 st_vsq_ptr;
  /*
   *  LAG LB Key
   */
  uint32 lag_lb_key;
  /*
   *  Ignore Congestion Point
   */
  uint32 ignore_cp;
  /*
   * Snoop id
   */
  uint32 snoop_id;
} SOC_PPC_DIAG_PKT_TM_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MPLS command within EEI
   */
  SOC_PPC_MPLS_COMMAND mpls_cmd;
  /*
   *  include egress encapsulation entries
   */
  SOC_PPC_EG_ENCAP_ENTRY_INFO encap_info[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_MAX];
  /*
   *  Egress encapsulation pointers
   */
  uint32 eep[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_MAX];
  /*
   *  number of valid Egress encapsulation
   */
  uint32 nof_eeps;
  /*
   *  Link layer VSI, to commit vlan editing according
   */
  SOC_PPC_VSI_ID ll_vsi;
  /*
   *  OUT-AC commit vlan editing according
   */
  SOC_PPC_AC_ID out_ac;
  /*
   *  local tm port packet egress from
   */
  SOC_PPC_TM_PORT tm_port;
  /*
   *  local PP port packet egress from
   */
  SOC_PPC_PORT pp_port;
  /*
   * Out-RIF is valid
   */
  uint8 rif_is_valid;
  /*
   * Native out-RIF
   */
  uint32 out_rif;
  /* 
   * Out-LIF profile (Jericho only)
   */
  uint32 lif_profile;
  /*
   * ENCAP information within EEI
   */
  SOC_PPC_EEI eei;

} SOC_PPC_DIAG_ENCAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Bitmap indicates which drop reasons at egress. See
   *  SOC_PPC_DIAG_EG_DROP_REASON
   */
  uint32 drop_log[2];

} SOC_PPC_DIAG_EG_DROP_LOG_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ethernet Type
   */
  uint32 ether_type;
  /*
   *  Destination MAC address
   */
  SOC_SAND_PP_MAC_ADDRESS da;
  /*
   *  Src MAC address
   */
  SOC_SAND_PP_MAC_ADDRESS sa;
  /*
   *  Whether this the Ethernet header includes VLAN (IEEE
   *  802.1Q tag based VLAN) uses an extra tag in the header
   *  to identify the VLAN
   */
  uint8 is_tagged;
  /*
   *  A tagged frame is 4 bytes longer than an untagged frame
   *  and contains two bytes of TPID. (Tag Protocol
   *  Identifier) and two bytes of TCI (Tag Control
   *  Information).
   */
  SOC_SAND_PP_VLAN_TAG tag;

} SOC_PPC_DIAG_ETH_PACKET_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*   
   *  Committed egress vlan edit key .
   */
  SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY key;  
  /*   
   *  Committed vlan edit command information.   
   */  
  SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO cmd_info;  
  /*   
   *  When VID/PCP source is "SRC_AC_EDIT_INFO"   
   *  then this information is considered   
   */ 
  SOC_SAND_PP_VLAN_TAG ac_tag;  
  /* 
   *  When VID/PCP source is "SRC_AC_EDIT_INFO_2"
   *  then this information is considered
   *  Not supported for PetraB
   */ 
   SOC_SAND_PP_VLAN_TAG ac_tag2;
} SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO;



typedef enum soc_ppc_diag_glem_outlif_source_e {
    soc_ppc_diag_glem_outlif_source_ftmh,
    soc_ppc_diag_glem_outlif_source_eei,
    soc_ppc_diag_glem_outlif_source_cud1,
    soc_ppc_diag_glem_outlif_source_cud2,
    soc_ppc_diag_glem_outlif_source_user,
    soc_ppc_diag_glem_outlif_source_count
} soc_ppc_diag_glem_outlif_source_t;

typedef struct soc_ppc_diag_glem_outlif_s {
    int global_outlif;
    int local_outlif;
    soc_ppc_diag_glem_outlif_source_t source;
    uint8 found;
    uint8 accessed;
    uint8 mapped;
} soc_ppc_diag_glem_outlif_t;

typedef struct soc_ppc_diag_glem_signals_s {
    soc_ppc_diag_glem_outlif_t  outlifs[2];
} soc_ppc_diag_glem_signals_t;


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
  SOC_PPC_DIAG_BUFFER_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_BUFFER *info
  );

void
  SOC_PPC_DIAG_MODE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_MODE_INFO *info
  );

void
  SOC_PPC_DIAG_VSI_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_VSI_INFO *info
  );

void
  SOC_PPC_DIAG_DB_USE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_DB_USE_INFO *info
  );

void
  SOC_PPC_DIAG_TCAM_USE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TCAM_USE_INFO *info
  );

void
  SOC_PPC_DIAG_RECEIVED_PACKET_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_RECEIVED_PACKET_INFO *info
  );

void
  SOC_PPC_DIAG_TRAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TRAP_INFO *info
  );

void
  SOC_PPC_DIAG_SNOOP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_SNOOP_INFO *info
  );

void
  SOC_PPC_DIAG_TRAPS_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TRAPS_INFO *info
  );

void
  SOC_PPC_DIAG_TRAP_TO_CPU_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TRAP_TO_CPU_INFO *info
  );

void
  SOC_PPC_DIAG_IPV4_VPN_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IPV4_VPN_ROUTE_KEY *info
  );

void
  SOC_PPC_DIAG_LEM_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_KEY *info
  );

void
  SOC_PPC_DIAG_LIF_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_LIF_KEY *info
  );

void
  SOC_PPC_DIAG_LIF_VALUE_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_LIF_VALUE *info
  );

void
  SOC_PPC_DIAG_LIF_LKUP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_LIF_LKUP_INFO *info
  );

void
  SOC_PPC_DIAG_TCAM_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TCAM_KEY *info
  );

void
  SOC_PPC_DIAG_TCAM_VALUE_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TCAM_VALUE *info
  );

void
  SOC_PPC_DIAG_LEM_VALUE_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_VALUE *info
  );

void
  SOC_PPC_DIAG_PARSING_L2_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_PARSING_L2_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_MPLS_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_PARSING_MPLS_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_IP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_PARSING_IP_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_HEADER_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_PARSING_HEADER_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_PARSING_INFO *info
  );

void
  SOC_PPC_DIAG_TERM_MPLS_LABEL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TERM_MPLS_LABEL_INFO *info
  );

void
  SOC_PPC_DIAG_TERM_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TERM_INFO *info
  );

void
  SOC_PPC_DIAG_IPV4_VPN_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IPV4_VPN_KEY *info
  );

void
  SOC_PPC_DIAG_IPV6_VPN_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IPV6_VPN_KEY *info
  );

#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
void
  SOC_PPC_DIAG_IPV4_UNICAST_RPF_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IPV4_UNICAST_RPF *info
  );

void
  SOC_PPC_DIAG_IPV4_MULTICAST_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IPV4_MULTICAST *info
  );

void
  SOC_PPC_DIAG_IPV6_UNICAST_RPF_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IPV6_UNICAST_RPF *info
  );

void
  SOC_PPC_DIAG_IPV6_MULTICAST_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IPV6_MULTICAST *info
  );

void
  SOC_PPC_DIAG_MPLS_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_MPLS *info
  );

void
  SOC_PPC_DIAG_TRILL_UNICAST_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TRILL_UNICAST *info
  );

void
  SOC_PPC_DIAG_TRILL_MULTICAST_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TRILL_MULTICAST *info
  );

void
  SOC_PPC_DIAG_IP_REPLY_RECORD_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_IP_REPLY_RECORD *info
  );

void
  SOC_PPC_DIAG_SECOND_IP_REPLY_RECORD_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_SECOND_IP_REPLY_RECORD *info
  );
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */

void
  SOC_PPC_DIAG_FRWRD_LKUP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_LKUP_KEY *info
  );

void
  SOC_PPC_DIAG_FRWRD_LKUP_VALUE_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_LKUP_VALUE *info
  );

void
  SOC_PPC_DIAG_FRWRD_LKUP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_LKUP_INFO *info
  );

void
  SOC_PPC_DIAG_LEARN_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_LEARN_INFO *info
  );

void
  SOC_PPC_DIAG_VLAN_EDIT_RES_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_VLAN_EDIT_RES *info
  );

void
  SOC_PPC_DIAG_FRWRD_DECISION_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_DECISION_INFO *info
  );

void
  SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO *info
  );

void
  SOC_PPC_DIAG_PKT_TM_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_PKT_TM_INFO *info
  );

void
  SOC_PPC_DIAG_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_ENCAP_INFO *info
  );

void
  SOC_PPC_DIAG_EG_DROP_LOG_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_EG_DROP_LOG_INFO *info
  );

void
  SOC_PPC_DIAG_ETH_PACKET_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_ETH_PACKET_INFO *info
  );

void 
  SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO *prm_vec_res
  );

void
  SOC_PPC_DIAG_TRILL_KEY_clear(
    SOC_SAND_OUT SOC_PPC_DIAG_TRILL_KEY *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_DIAG_FWD_LKUP_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_FWD_LKUP_TYPE enum_val
  );

const char*
  SOC_PPC_DIAG_TCAM_USAGE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_TCAM_USAGE enum_val
  );

const char*
  SOC_PPC_DIAG_FLAVOR_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_FLAVOR enum_val
  );

const char*
  SOC_PPC_DIAG_LEM_LKUP_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_LEM_LKUP_TYPE enum_val
  );

const char*
  SOC_PPC_DIAG_LIF_LKUP_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_LIF_LKUP_TYPE enum_val
  );

const char*
  SOC_PPC_DIAG_DB_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_DB_TYPE enum_val
  );

const char*
  SOC_PPC_DIAG_FRWRD_DECISION_PHASE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_FRWRD_DECISION_PHASE enum_val
  );

const char*
  SOC_PPC_DIAG_MPLS_TERM_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_MPLS_TERM_TYPE enum_val
  );

const char*
  SOC_PPC_DIAG_PKT_TRACE_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_PKT_TRACE enum_val
  );

const char*
  SOC_PPC_DIAG_EG_DROP_REASON_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_EG_DROP_REASON enum_val
  );

const char*
  SOC_PPC_DIAG_PKT_TM_FIELD_to_string(
    SOC_SAND_IN  SOC_PPC_DIAG_PKT_TM_FIELD enum_val
  );

void
  SOC_PPC_DIAG_BUFFER_print(
    SOC_SAND_IN  SOC_PPC_DIAG_BUFFER *info
  );

void
  SOC_PPC_DIAG_MODE_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_MODE_INFO *info
  );

void
  SOC_PPC_DIAG_VSI_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_VSI_INFO *info
  );

void
  SOC_PPC_DIAG_DB_USE_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO *info
  );

void
  SOC_PPC_DIAG_TCAM_USE_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TCAM_USE_INFO *info
  );

void
  SOC_PPC_DIAG_RECEIVED_PACKET_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_RECEIVED_PACKET_INFO *info
  );

void
  SOC_PPC_DIAG_TRAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TRAP_INFO *info
  );

void
  SOC_PPC_DIAG_SNOOP_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_SNOOP_INFO *info
  );

void
  SOC_PPC_DIAG_TRAPS_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TRAPS_INFO *info
  );

void
  SOC_PPC_DIAG_TRAP_TO_CPU_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TRAP_TO_CPU_INFO *info
  );

void
  SOC_PPC_DIAG_IPV4_VPN_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_ROUTE_KEY *info
  );

void
  SOC_PPC_DIAG_EXTEND_P2P_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_EXTEND_P2P_KEY *info
  );


void
  SOC_PPC_DIAG_LEM_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_LEM_KEY *info,
    SOC_SAND_IN  SOC_PPC_DIAG_LEM_LKUP_TYPE type
  );

void
  SOC_PPC_DIAG_LIF_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_LIF_KEY *info,
    SOC_SAND_IN  SOC_PPC_DIAG_LIF_LKUP_TYPE lkup_type
  );

void
  SOC_PPC_DIAG_LIF_VALUE_print(
    SOC_SAND_IN  SOC_PPC_DIAG_LIF_VALUE *info,
    SOC_SAND_IN  SOC_PPC_DIAG_LIF_LKUP_TYPE lkup_type
  );

void
  SOC_PPC_DIAG_LIF_LKUP_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_LIF_LKUP_INFO *info
  );

void
  SOC_PPC_DIAG_TCAM_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TCAM_KEY *info
  );

void
  SOC_PPC_DIAG_TCAM_VALUE_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TCAM_VALUE *info
  );

void
  SOC_PPC_DIAG_LEM_VALUE_print(
    SOC_SAND_IN  SOC_PPC_DIAG_LEM_VALUE *info,
    SOC_SAND_IN  SOC_PPC_DIAG_LEM_LKUP_TYPE type
  );

void
  SOC_PPC_DIAG_PARSING_L2_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_PARSING_L2_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_MPLS_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_PARSING_MPLS_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_IP_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_PARSING_IP_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_HEADER_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_PARSING_HEADER_INFO *info
  );

void
  SOC_PPC_DIAG_PARSING_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_PARSING_INFO *info
  );

void
  SOC_PPC_DIAG_TERM_MPLS_LABEL_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TERM_MPLS_LABEL_INFO *info
  );

void
  SOC_PPC_DIAG_TERM_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_TERM_INFO *info
  );

void
  SOC_PPC_DIAG_IPV4_VPN_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_KEY *info
  );

void
  SOC_PPC_DIAG_IPV6_VPN_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_IPV6_VPN_KEY *info
  );

#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
void
  SOC_PPC_DIAG_IPV4_UNICAST_RPF_print(
    SOC_SAND_IN SOC_PPC_DIAG_IPV4_UNICAST_RPF *info
  );

void
  SOC_PPC_DIAG_IPV4_MULTICAST_print(
    SOC_SAND_IN SOC_PPC_DIAG_IPV4_MULTICAST *info
  );

void
  SOC_PPC_DIAG_IPV6_UNICAST_RPF_print(
    SOC_SAND_IN SOC_PPC_DIAG_IPV6_UNICAST_RPF *info
  );

void
  SOC_PPC_DIAG_IPV6_MULTICAST_print(
    SOC_SAND_IN SOC_PPC_DIAG_IPV6_MULTICAST *info
  );

void
  SOC_PPC_DIAG_MPLS_print(
    SOC_SAND_IN SOC_PPC_DIAG_MPLS *info
  );

void
  SOC_PPC_DIAG_TRILL_UNICAST_print(
    SOC_SAND_IN SOC_PPC_DIAG_TRILL_UNICAST *info
  );

void
  SOC_PPC_DIAG_TRILL_MULTICAST_print(
    SOC_SAND_IN SOC_PPC_DIAG_TRILL_MULTICAST *info
  );

void
  SOC_PPC_DIAG_IP_REPLY_RECORD_print(
    SOC_SAND_IN SOC_PPC_DIAG_IP_REPLY_RECORD *info
  );

void
  SOC_PPC_DIAG_SECOND_IP_REPLY_RECORD_print(
    SOC_SAND_IN SOC_PPC_DIAG_SECOND_IP_REPLY_RECORD *info
  );
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */

void
  SOC_PPC_DIAG_FRWRD_LKUP_KEY_print(
    SOC_SAND_IN  SOC_PPC_DIAG_FRWRD_LKUP_KEY *info,
    SOC_SAND_IN  SOC_PPC_DIAG_FWD_LKUP_TYPE frwrd_type,
    SOC_SAND_IN  uint8                      is_kbp
  );

void
  SOC_PPC_DIAG_FRWRD_LKUP_VALUE_print(
    SOC_SAND_IN  SOC_PPC_DIAG_FRWRD_LKUP_VALUE *info,
    SOC_SAND_IN  SOC_PPC_DIAG_FWD_LKUP_TYPE frwrd_type
  );

void
  SOC_PPC_DIAG_FRWRD_LKUP_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_FRWRD_LKUP_INFO *info
  );

void
  SOC_PPC_DIAG_LEARN_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_LEARN_INFO *info
  );

void
  SOC_PPC_DIAG_VLAN_EDIT_RES_print(
    SOC_SAND_IN  SOC_PPC_DIAG_VLAN_EDIT_RES *info
  );

void
  SOC_PPC_DIAG_FRWRD_DECISION_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_FRWRD_DECISION_INFO *info
  );

void
  SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO *info
  );

void
  SOC_PPC_DIAG_PKT_TM_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_PKT_TM_INFO *info
  );

void
  SOC_PPC_DIAG_ENCAP_INFO_print(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_DIAG_ENCAP_INFO *info
  );

void
  SOC_PPC_DIAG_EG_DROP_LOG_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_EG_DROP_LOG_INFO *info
  );

void
  SOC_PPC_DIAG_ETH_PACKET_INFO_print(
    SOC_SAND_IN  SOC_PPC_DIAG_ETH_PACKET_INFO *info
  );

void 
  SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO_print(
     SOC_SAND_IN SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO *prm_vec_res
  );

void
  SOC_PPC_DIAG_TRILL_KEY_print(
    SOC_SAND_IN SOC_PPC_DIAG_TRILL_KEY *info
  );


void
 soc_ppc_diag_glem_outlif_print(
    soc_ppc_diag_glem_outlif_t *info
    );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_DIAG_INCLUDED__*/
#endif

