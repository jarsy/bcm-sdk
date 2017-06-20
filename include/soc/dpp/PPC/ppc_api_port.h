/* $Id: ppc_api_port.h,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_port.h
*
* MODULE PREFIX:  soc_ppc_port
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

#ifndef __SOC_PPC_API_PORT_INCLUDED__
/* { */
#define __SOC_PPC_API_PORT_INCLUDED__

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

/* */
#define SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_MAX      (0xf)
#define SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_SHIFT    (0)
#define SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_MASK     (0xf)

#define SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_MAX    (4095)
#define SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_SHIFT  (0)
#define SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_MASK   (0xfff)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_SET(_port_property, _value)  \
    ((_port_property) = ((_port_property & ~(SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_MASK << SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_SHIFT)) \
    | (_value & (SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_MASK << SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_SHIFT))))

#define SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_GET(_port_property)  \
    ((_port_property & (SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_MASK << SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_SHIFT)) \
    >> SOC_PPC_PORT_PROPERTY_VLAN_TRANSLATION_SHIFT)

#define SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_SET(_port_property, _value)  \
    ((_port_property) = ((_port_property & ~(SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_MASK << SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_SHIFT)) \
    | (_value & (SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_MASK << SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_SHIFT))))

#define SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_GET(_port_property)  \
    ((_port_property & (SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_MASK << SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_SHIFT)) \
    >> SOC_PPC_PORT_PROPERTY_TUNNEL_TERMINATION_SHIFT)

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  Port direction - incoming .
   */
  SOC_PPC_PORT_DIRECTION_INCOMING = 1,
  /*
   *  Port direction - outgoing .
   */
  SOC_PPC_PORT_DIRECTION_OUTGOING = 2,
  /*
   *  Port direction - both incoming and outgoing.
   */
  SOC_PPC_PORT_DIRECTION_BOTH = 3,
  /*
   *  Total number of Port directions.
   */
  SOC_PPC_PORT_NOF_DIRECTIONS = 4
}SOC_PPC_PORT_DIRECTION;

#define SOC_PPC_IS_DIRECTION_INCOMING(dir) \
  SOC_SAND_NUM2BOOL(((dir) == SOC_PPC_PORT_DIRECTION_INCOMING  ) || ((dir) == SOC_PPC_PORT_DIRECTION_BOTH))

#define SOC_PPC_IS_DIRECTION_OUTGOING(dir) \
  SOC_SAND_NUM2BOOL(((dir) == SOC_PPC_PORT_DIRECTION_OUTGOING  ) || ((dir) == SOC_PPC_PORT_DIRECTION_BOTH))


typedef enum
{
  /*
   *  Packet is dropped. SA not learned.
   */
  SOC_PPC_PORT_STP_STATE_BLOCK = 0,
  /*
   *  Packet is dropped. SA learned
   */
  SOC_PPC_PORT_STP_STATE_LEARN = 1,
  /*
   *  Packet is forwarded and learned.
   */
  SOC_PPC_PORT_STP_STATE_FORWARD = 2,
  /*
   *  Number of types in SOC_PPC_PORT_STP_STATE
   */
  SOC_PPC_NOF_PORT_STP_STATES = 3
}SOC_PPC_PORT_STP_STATE;

typedef enum
{
  SOC_PPC_PORT_STP_STATE_FLD_VAL_BLOCK = 2,
  SOC_PPC_PORT_STP_STATE_FLD_VAL_LEARN = 1,
  SOC_PPC_PORT_STP_STATE_FLD_VAL_FORWARD = 0
} SOC_PPC_PORT_STP_STATE_FLD_VAL;

/* ARAD only enum defines { */
typedef enum
{
  /* Note: All Initial-VID assume LSB is set. All non Initial-VID assume LSB is unset */

  /*
   *  Default VT profile, no special settings per port.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_DEFAULT = 0,
  /* 
   *  Search both DBS(double tag and single tag), a match on double VIDs has higher priority.
   *  Initial-VID is used instead of Outer-VID.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_DOUBLE_TAG_PRIORITY_INITIAL_VID = 3,  
  /*
   *  Port Support FRR, coupling and ignores the outer-tag (use initial VID database)
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_FRR_COUPLING_USE_INITIAL_VID = 5,
  /* 
   *  Port support FRR, coupling and ignore the outer-tag (use initial VID database) and
   *  in case of double tag a match on double VIDs has higher priority.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_FRR_COUPLING_DOUBLE_TAG_PRIORITY_USE_INITIAL_VID = 7,
  /* 
   *  Ignores the outer VID and use initial vid only
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_USE_INITIAL_VID = 1,
  /* 
   *  Port support Trill only and ignores the outer VID and use initial vid only
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_TRILL_USE_INITIAL_VID = 5,
  /*
   *  Port support FRR and coupling
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_FRR_COUPLING = 4,
  /* 
   *  Port support Explicit NULL
   *  Overwrite FRR Coupling configuration.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_EXPLICIT_NULL = 4,
  /* 
   *  Search both DBs (double tag and single tag),
   *   a match on double VIDs has higher priority
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_DOUBLE_TAG_PRIORITY = 2,  
  /* 
   *  Port Support Trill only
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_TRILL = 4,
  
  /* 
   *  Port support PON application only.
   *  In case PON application is enabled, VT profile
   *  overwrites FRR Coupling Profiles.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_PON_DEFAULT = 4,
  /* 
   *  Port support PON application only and support additional lookup of Tunnel ID. 
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_PON_TUNNEL_ID = 5,
  /* 
   *  Port support Extender PE, overrides PON 
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_VRRP_PORT = 5,
  /* 
   *  Port support Extender PE, overrides PON 
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_EXTENDER_PE = 5,
  /* 
   *  Port ignores the inner-tag.
   *  In case PON application is enabled, VT profile
   *  overwrites Trill Profile.
   *  overwrites FRR Profiles.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_IGNORE_2ND_TAG = 6,
  /* 
   *  Port ignores the inner-tag.
   *  Port support Explicit-NULL
   *  overwrite EVB.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_IGNORE_2ND_TAG_EXPLICIT_NULL = 7,
  /* 
   *  Port support EVB application. In that case, all lookups no matter packet format code
   *  are the same.
   *  VD x S-Channel x Initial-C-VLAN
   *  Assuming, no system with both FRR coupling and EVB.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_EVB = 7,  
  /* 
   *  Port support Flexible QinQ application. In that case, vlan translation is based on IPv4 5-tuples.
   *  Assuming, no system with both FRR coupling  EVB , and IPv4 Match.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_FLEXIBLE_Q_IN_Q  = 7,  
  /*
   *  Port support PCP Profile. Relevant for TST2 only
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_PCP_ENABLE       = 7,
/*
   *  Port support PP port based mpls label lookup
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE_VLAN_DOMAIN_MPLS       = 7,

  /* Below Port profiles values are available for Jericho and above*/ 

  /*
   *  Port support PP port based port termination
   */

  SOC_PPC_PORT_DEFINED_VT_PORT_TERMINATION = 8,
  
   /*
   *  Number of types in SOC_PPC_PORT_DEFINED_VT_PROFILE
   */

  SOC_PPC_NOF_PORT_DEFINED_VT_PROFILES = 9
}SOC_PPC_PORT_DEFINED_VT_PROFILE;

typedef enum
{
  /*
   *  Default VT profile, no special settings per port.
   */
  SOC_PPC_PORT_DEFINED_TT_PROFILE_DEFAULT = 0,
  /*
   *  Port support FRR and Coupling.
   */
  SOC_PPC_PORT_DEFINED_TT_PROFILE_FRR_COUPLING = 1,
  /*
   *  Port support Explicit NULL.
   *  Overwrites FRR Coupling configuration.
   */
  SOC_PPC_PORT_DEFINED_TT_PROFILE_EXPLICIT_NULL = 1,
  /*
   *  Port support MPLS RAW processing.
   */
  SOC_PPC_PORT_DEFINED_TT_PROFILE_FORCE_MY_MAC = 2,  
  /* 
   *  Port TT profile for port extender control bridge.
   */ 
  SOC_PPC_PORT_DEFINED_TT_PROFILE_PORT_EXTENDER_CB = 3,
  /*
   *  Port TT profile for port extender control bridge with untag check.
   */
  SOC_PPC_PORT_DEFINED_TT_PROFILE_PORT_EXTENDER_UNTAG_CB = 4,
/*
   *  Port support PP port based mpls label lookup
   */
  SOC_PPC_PORT_DEFINED_TT_PROFILE_VLAN_DOMAIN_MPLS       = 5,

   /*
   *  Port support PP port based port termination
   */
   
  SOC_PPC_PORT_DEFINED_TT_PROFILE_PORT_TERMINATION = 6,

  /*
   *  Number of types in SOC_PPC_PORT_DEFINED_TT_PROFILE
   */
  SOC_PPC_NOF_PORT_DEFINED_TT_PROFILES
}SOC_PPC_PORT_DEFINED_TT_PROFILE;

/* ARAD only enum defines } */

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Whether the L2 VPN service is enabled for this port. If
   *  TRUE, then per AC a learn destination may be defined,
   *  where this destination may be: FEC, direct flow, system
   *  port, or multicast ID (msb of the destination is defined
   *  per port). If FALSE, then the system port is learned. If
   *  the port is L2-VPN, then routed packets entering from
   *  this port are assigned to VRF 0 and have no RPF check
   *  for UC packets.
   */
  uint8 enable_l2_vpn;
  /*
   *  Learned destination Type. The value of the destination
   *  may be set per AC.
   */
  SOC_SAND_PP_DEST_TYPE learn_dest_type;

} SOC_PPC_PORT_L2_VPN_INFO;

#define SOC_PPC_PORT_IHP_PINFO_LLR_TBL                   (0x1 << 0)
#define SOC_PPC_PORT_IHP_PP_PORT_INFO_TBL                (0x1 << 1)
#define SOC_PPC_PORT_IHP_VTT_PP_PORT_CONFIG_TBL          (0x1 << 2)
#define SOC_PPC_PORT_IHB_PINFO_FLP_TBL                   (0x1 << 3)
#define SOC_PPC_PORT_EGQ_PCT_TBL                         (0x1 << 4)
#define SOC_PPC_PORT_EPNI_PP_PCT_TBL                     (0x1 << 5)
#define SOC_PPC_PORT_EGQ_PP_PPCT_TBL                     (0x1 << 6)
#define SOC_PPC_PORT_IHP_VTT_PP_PORT_VSI_PROFILE_TBL     (0x1 << 7)
#define SOC_PPC_PORT_ALL_TBL                             (0xff)

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Port profile in the context of:- Acceptable frame types
   *  (VLAN Tag Structure). See
   *  soc_ppd_llp_filter_ingress_acceptable_frames_set().
   *  - TPID selection see
   *  soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set()
   *  in ARAD, port profile is used only for Acceptable frame types.
   */
  uint32 port_profile;
  
  /*
   *  Port profile for egress in the context of:- Acceptable frame types in egress
   *  (VLAN Tag Structure for Egress). 
   */
  uint32 port_profile_eg;
  /*
   *  Sets the TPID profile which includes selection of two
   *  TPIDs from the Global TPIDs set by
   *  soc_ppd_llp_parse_tpid_values_set(). Range: 0 - 3. Soc_petra-B
   *  only.
   */
  uint32 tpid_profile;
  /*
   *  VLAN Domain ID. The VLAN domain is either a port, or a
   *  group of ports that share the same VLAN IDs space. In
   *  T20E has to be equal to local-port-id.
   */
  uint32 vlan_domain;
  /*
   *  L2 type (CEP, CNP, PNP, VBP). Only CEP and VBP is
   *  possible. Otherwise error is returned.this configuration
   *  affects: - egress vlan editing (in case forwarded packet
   *  is not associated with out-AC), in this case:if port
   *  type is CEP and packet has C-tag then lookup in Vlan
   *  Editing DB is according to out-VLAN-domain and
   *  CVID. Otherwise (port type is VBP or packet has no C-tag)
   *  then lookup in VLAN editing DB is according to
   *  out-VLAN-domain and VSI.
   */
  SOC_SAND_PP_PORT_L2_TYPE port_type;
  /*
   *  Initial Action Profile pointer set to packets entering
   *  from the port, and according to which to process and
   *  forward the packet. This action profile may be
   *  overwritten by further processing decisions. Soc_petra-B:
   *  CPU code range 0-255. T20E: CPU code range 0-63.
   */
  SOC_PPC_ACTION_PROFILE initial_action_profile;
  /*
   *  Is the port PBP Provider Backbone Port, i.e., the port
   *  is facing the backbone in MAC-in-MAC application. If
   *  TRUE, then the EEI arriving to this port at the egress
   *  will be decoded as ISID values. If this set to TRUE,
   *  then enable_sa_lookup has to be TRUE also.
   */
  uint8 is_pbp;
  /*
   *  Is the port network for FCoE App. this value is valid
   *  only if the switch is NPV. when n_port == 1 it means n_port
   *  (source routing). when n_port == 0 it means no_port
   *  (destination routing).
   */
  uint8 is_n_port;
  /*
   *  Whether SA learning is enabled for this port. In order
   *  to learn packets also the LIF (AC/PWE) has to be
   *  configured to enable learning. i.e., this enable is
   *  necessary but not sufficient to learn a packet's SA.
   *  
   *  ARAD:
   *  - this control learning of packets ingress from this port
   *  - this is valid for both ingress and egress learning
   */
  uint8 enable_learning;
  /*
   *  Whether SA learning is enabled for packets outgoing from this port. 
   *  
   *  ARAD only:
   *  this control learning of packets egress from this port
   */
  uint8 enable_outgoing_learning;
  /*
   *  Enable same-interface/hair-pin Filtering.
   *  i.e. filter packets which incoming interface equal to outgoing
   *  interface (Hair-Pin).
   *  this enable filtering for both packets incomming and outgoing from
   *  this port.
   *  to change configuration only on egress side use soc_ppd_eg_filter_port_info_set
   *  to change configuration only on ingress side use soc_ppd_llp_trap_port_info_set
   *  - if this filter occurs at ingress (For unicast packets) then
   *    SOC_PPC_TRAP_CODE_SAME_INTERFACE trap code is raised
   *  - if this filter occurs at egress (For unicast/multicast packets) then
   *    SOC_PPC_TRAP_EG_TYPE_HAIR_PIN is raised
   * get API returns port ingress status.
   */
  uint8 enable_same_interfac_filter;
  /* 
   *  Skip same-interface/hair-pin Filtering settings.
   *  Use it in case filter value is not the same for both ingress and egress sides.
   *  Get returns TRUE when filter is not the same for both sides.
   *  Valid for ARAD only.
   */
  uint8 same_interface_filter_skip;
  /*
   *  TRUE: The AC is learnedFALSE: The AC is not
   *  learned.(only destination is learnt: destination may be
   *  source-system-port, FEC or flow see soc_ppd_l2_lif_pwe_add()
   *  and soc_ppd_l2_lif_ac_add().- When a packet is forwarded to
   *  a learned-AC destination, the AC is used to point to the
   *  egress editing database.- Otherwise, egress editing and
   *  processing is according to <outPort x VSI>.- Setting
   *  this flag enables performing hairpin filter on the
   *  egress port.- When the AC is learned, the AC ID is the
   *  LIF entry ID (lif_index).
   */
  uint8 is_learn_ac;
  /*
   *  Port orientation, HUB or Spoke. Packets forwarded from
   *  hub interface to hub interface will be filtered. See
   *  also soc_ppd_l2_lif_pwe_set()/ soc_ppd_l2_lif_ac_set() to set
   *  in-LIF orientation, and
   *  soc_ppd_eg_filter_split_horizon_out_ac_orientation_set()/
   *  soc_ppd_eg_encap_mpls_encap_entry_add() to set out-LIF
   *  orientation. Port orientation may have lowest priority,
   *  i.e., only at egress when out-AC is not associated with
   *  the packet and packet is not transmitted by tunnel. Not
   *  relevant for T20E and may be ignored.
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;
  /*
   *  Port profile for l2-protocol-based VID and TC
   *  assignment. See soc_ppd_llp_vid_assign_protocol_based_set()
   *  and soc_ppd_llp_cos_protocol_based_se(). Range: 0 - 7.
   */
  uint32 ether_type_based_profile;
  /*
   *  If TRUE, then port operates in non-authorized 802.1x
   *  mode and only 802.1x traffic (EAPOL packets) is allowed.
   *  (Ether-type=0x888e according to IEEE Std
   *  802.1X-Port-based network access. May be changed by
   *  logical access.) If FALSE, then the port is authorized
   *  and all traffic is allowed. EAPOL packets in
   *  unauthorized port are forwarded Normally
   */
  uint8 not_authorized_802_1x;
  /*
   *  Maximum Transmission Unit. Packets with size over this
   *  value will be processed according to action profile
   *  assigned to this event. The Check of the MTU is
   *  performed at the egress, comparing the size to packet
   *  size starting from forwarding-header (aka Forwarding
   *  Layer MTU Filter). Size in bytes. 
   * Soc_petra-B: considered packet size is network headers
   */
  uint32 mtu;
  /*
   *  Maximum Transmission Unit. Packets with size over this
   *  value will be processed according to action profile
   *  assigned to this event. The Check of the MTU is
   *  performed at the egress, comparing the size to packet
   *  size starting including encapsulated headers (aka
   *  Link Layer MTU Filter). Size in bytes.
   * Soc_petra-B: considered packet size is network headers
   */
  uint32 mtu_encapsulated;
  /* 
   * Default Port forwarding decision to apply when there is 
   * no hit in the MAC table. The profile ID is part of the 
   * 'dflt_frwrd_key', used by 
   * soc_ppd_l2_lif_default_frwrd_info_set() 
   * range: 0 - 1. 
   * Relevant for ARAD only.
   * 
   */ 
  uint32 da_not_found_profile;
  /*
   *  Default Egress AC. 
   *  Used for vlan editing DB in case of lookup PortxVSI                                                .
   *  or PortxVSIxCVID (in case of CEP) are not found                                                    .
   *  Range: 0-64K-1                                                                                     .
   *  Relevant for ARAD only.                                                                                                   .
   */
  SOC_PPC_AC_ID dflt_egress_ac;
  /*
   *  vlan translation profile in the context of:
   *  - AC map key, (port, portxVlan, PortxVlanxVlan). See
   *  soc_ppd_l2_lif_ac_map_key_set()Range: SOC_PPC_NOF_PORT_DEFINED_VT_PROFILES - 7. Used for user-define programs.
   *  - AC lookup pre define programs, usages:
   *  See SOC_PPC_PORT_DEFINED_VT_PROFILE_XXX programs.
   *  
   *  Relevant for ARAD only.
   */
  SOC_PPC_PORT_DEFINED_VT_PROFILE vlan_translation_profile;
  /*
   *  tunnel termination profile in the context of:
   *  - MPLS key, (FRR).
   *  Range: SOC_PPC_NOF_PORT_DEFINED_TT_PROFILES - 7. Used for user-define programs.
   *  - TT lookups pre define programs, usages:
   *  See SOC_PPC_PORT_DEFINED_TT_PROFILE_XXX programs.
   *  
   *  Relevant for ARAD only.
   */
  SOC_PPC_PORT_DEFINED_TT_PROFILE tunnel_termination_profile;

  /*
   *  AC P2P to PBB VSI profile:
   *  used to get the VSI-base from "Vsi Profile To Vsi Table"
   *  for the AC-P2P-to-PBB LIF.
   *  Range: 0 - 7. 
   *  
   *  Relevant for ARAD only.
   */
  uint32 ac_p2p_to_pbb_vsi_profile;
  /*
   *  Label PWE P2P VSI profile:
   *  used to get the VSI-base from "Vsi Profile To Vsi Table"
   *  for the PWE P2P LIF.
   *  Range: 0 - 7. 
   *  
   *  Relevant for ARAD only.
   */
  uint32 label_pwe_p2p_vsi_profile;

  /*
   *  Enable pon port double lookup:
   *  used for pon tunnel-id lookup in case of VDxTunnel-IDxSVIDxCVID,
   *  VDxTunnel-IDxSVID
   *  
   *  Relevant for ARAD only.
   */
  uint8 enable_pon_double_lookup;

  /*
   *  Enable speculative MPLS next header parsing.
   *  MPLS Headers do not have a next header field,
   *  therefore the parser speculates the next header type based on the first nibble.
   *
   */
  uint8 enable_mpls_speculative_parsing;
  /*
   *  Default port LIF. 
   *  Used for vlan translating in case of lookup Port                                               .
   *  or PortxSVID or PortxSVIDxCVID are not found                                                    .
   *  Range: 0-128K-1                                                                                     .
   *  Relevant for JERICHO and ARAD.                                                                                                   .
   */
  uint32 dflt_port_lif;

  /*
   *  It's used to indicate which hardware tables will be configured.
   */
  uint32 flags;

} SOC_PPC_PORT_INFO;

typedef enum {
    soc_ppc_port_property_vlan_translation,
    soc_ppc_port_property_tunnel_termination
} SOC_PPC_PORT_PROPERTY;


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
  SOC_PPC_PORT_L2_VPN_INFO_clear(
    SOC_SAND_OUT SOC_PPC_PORT_L2_VPN_INFO *info
  );

void
  SOC_PPC_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_PORT_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_PORT_DIRECTION_to_string(
    SOC_SAND_IN  SOC_PPC_PORT_DIRECTION enum_val
  );

const char*
  SOC_PPC_PORT_STP_STATE_to_string(
    SOC_SAND_IN  SOC_PPC_PORT_STP_STATE enum_val
  );


void
  SOC_PPC_PORT_L2_VPN_INFO_print(
    SOC_SAND_IN  SOC_PPC_PORT_L2_VPN_INFO *info
  );

void
  SOC_PPC_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_PORT_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_PORT_INCLUDED__*/
#endif

