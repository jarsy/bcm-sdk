/* $Id: ppc_api_rif.h,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_rif.h
*
* MODULE PREFIX:  soc_ppc_rif
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

#ifndef __SOC_PPC_API_RIF_INCLUDED__
/* { */
#define __SOC_PPC_API_RIF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_mpls_term.h>
#include <soc/dpp/PPC/ppc_api_frwrd_fec.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Indicates that the RIF is not updated.                  */
#define  SOC_PPC_RIF_IS_NOT_UPDATED (0)

/*     SOC_PPC_RIF_MPLS_LABEL_XXX flags */
#define  SOC_PPC_RIF_MPLS_LABEL_FRR         (0x1)
#define  SOC_PPC_RIF_MPLS_LABEL_ELI         (0x4)
#define  SOC_PPC_RIF_MPLS_LABEL_EXPECT_BOS  (0x8)

/* No SEM/TCAM lookup for termination is required, only define LIF */
#define  SOC_PPC_RIF_MPLS_LABEL_LOOKUP_NONE             (0x10)
/* Indicate IML termination in EVPN */
#define  SOC_PPC_RIF_MPLS_LABEL_EVPN_IML                (0x20)

#define  SOC_PPC_RIF_MAX_NOF_ROUTING_ENABLERS_BITMAPS       16

/* SOC_PPC_RIF_PROFILE_TO_ROUTING_ENABLERS_VECTOR_BEFORE_INIT represents the enablers that are not updated by the SOC properties
   according to spec: bits 2-15 reserved 16,17 trill 18,19 MiM 20,21 arp, 22,23 cfm, 24,25 fcoe bits 26-31 defined below
   Disable routing for arp. */
#define  SOC_PPC_RIF_PROFILE_TO_ROUTING_ENABLERS_VECTOR_BEFORE_INIT         0x3CFFFFC;/* b 0000 0011 1100 1111 1111 1111 1111 1100 */

#define  SOC_PPC_ROUTING_ENABLE_MIM_UC_BIT                  18
#define  SOC_PPC_ROUTING_ENABLE_MIM_MC_BIT                  19
#define  SOC_PPC_ROUTING_ENABLE_IPV4UC_BIT                  26
#define  SOC_PPC_ROUTING_ENABLE_IPV4MC_BIT                  27
#define  SOC_PPC_ROUTING_ENABLE_IPV6UC_BIT                  28
#define  SOC_PPC_ROUTING_ENABLE_IPV6MC_BIT                  29
#define  SOC_PPC_ROUTING_ENABLE_MPLS_UC_BIT                 30
#define  SOC_PPC_ROUTING_ENABLE_MPLS_MC_BIT                 31

/*
 * termination key to include both SIP
 */
#define  SOC_PPC_RIF_IP_TERM_FLAG_USE_DIP       0x1
#define  SOC_PPC_RIF_IP_TERM_FLAG_USE_SIP       0x2
#define  SOC_PPC_RIF_IP_TERM_FLAG_IPV6          0x4
#define  SOC_PPC_RIF_IP_TERM_FLAG_KEY_ONLY      0x8
#define  SOC_PPC_RIF_IP_TERM_IPMC_BIDIR         0x10
#define  SOC_PPC_RIF_IP_TERM_FLAG_USE_DIP_DUMMY 0x20
#define  SOC_PPC_RIF_IP_TERM_FLAG_VRF_IS_VALID  0x40
#define  SOC_PPC_RIF_IP_TERM_FLAG_MY_VTEP_INDEX_IS_VALID 0x80
#define  SOC_PPC_RIF_IP_TERM_FLAG_IP_GRE_TUNNEL_IS_VALID 0x100


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
   *  Disable all following routing on RIF.
   */
  SOC_PPC_RIF_ROUTE_ENABLE_TYPE_NONE = 0x0,
  /*
   *  Enable IPv4/v6 UC routing.
   */
  SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV4_UC = 0x1,
  SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV6_UC = 0x2,  

  /*
   *  Enable IPv4/v6 MC routing.
   */
  SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV4_MC = 0x4,
  SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV6_MC = 0x8,  

  /*
   *  Enable MPLS routing (LSR)
   */
  SOC_PPC_RIF_ROUTE_ENABLE_TYPE_MPLS = 0x10
    
} SOC_PPC_RIF_ROUTE_ENABLE_TYPE;

#define SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IP_UC   (SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV4_UC + SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV6_UC)
#define SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IP_MC   (SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV4_MC + SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IPV6_MC)
#define SOC_PPC_RIF_ROUTE_ENABLE_TYPE_ALL     (SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IP_MC + SOC_PPC_RIF_ROUTE_ENABLE_TYPE_IP_UC + SOC_PPC_RIF_ROUTE_ENABLE_TYPE_MPLS)


/* we use a flag to identify what we're passing in lif_id.
   In most cases, it's lif_id.
   But it could be also:
   - result from SIP lookup in ISEM is learn information: IP tunnel FEC
   - result from DIP lookup in ISEM is my-vtep-index.
*/
/* since most of cases are LIF we use value 0 */
#define SOC_PPC_RIF_IP_TUNNEL_TERM_ADD_LIF_ID_IS_LIF_INDEX (0x0)
 /* result from SIP lookup in ISEM is learn information: IP tunnel FEC */
#define SOC_PPC_RIF_IP_TUNNEL_TERM_ADD_LIF_ID_IS_FEC (0x1)
/* result from DIP lookup in ISEM is my-vtep-index. */
#define SOC_PPC_RIF_IP_TUNNEL_TERM_ADD_LIF_ID_IS_MY_VTEP_INDEX (0x2)


typedef enum 
{ 
  /* 
   * Outermost (first) label 
   */ 
  SOC_PPC_MPLS_LABEL_INDEX_FIRST = 0x1, 
  /* 
   * Second label 
   */ 
  SOC_PPC_MPLS_LABEL_INDEX_SECOND = 0x2, 
  /* 
   * Third label 
   */ 
  SOC_PPC_MPLS_LABEL_INDEX_THIRD = 0x3, 
  /* 
   * The location of the label is not important, 
   * can appear in different places in the MPLS stack. 
   */ 
  SOC_PPC_MPLS_LABEL_INDEX_ALL = 0x0, 
  /* 
   * Number of types in SOC_PPC_MPLS_LABEL_INDEX 
   */ 
  SOC_PPC_NOF_MPLS_LABEL_INDEX = 4 
} SOC_PPC_MPLS_LABEL_INDEX; 



typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  First MPLS label, to be used for Router Interface
   *  mapping.
   */
  uint32 first_label;
  /*
   *  Last MPLS label, to be used for Router Interface
   *  mapping.
   */
  uint32 last_label;

} SOC_PPC_RIF_MPLS_LABELS_RANGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MPLS label ID.
   */
  SOC_SAND_PP_MPLS_LABEL label_id;
  /*
   *  The VSID was assigned to the packet, according to the
   *  Link Layer Ethernet header's AC.
   */
  SOC_PPC_VSI_ID vsid;

  /* 
   * The location of the label in the MPLS stack. 
   * Invalid for Soc_petra-B. 
   */ 
  SOC_PPC_MPLS_LABEL_INDEX label_index; 

  /*
   *  MPLS label ID second.
   *  Used in case of FRR or Coupling. Adding new entry with two labels.
   *  Invalid for Soc_petra-B.                                                                                                                                              .
   */
  SOC_SAND_PP_MPLS_LABEL label_id_second;

  /* 
   *  MPLS RIF Key flags
   *  See SOC_PPC_RIF_MPLS_LABEL_XXX
   *  Invalid for Soc_petra-B.
   */
  uint32 flags;

} SOC_PPC_MPLS_LABEL_RIF_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable RPF for Unicast traffic. Only relevant when the
   *  RIF is utilized for IP routing. (The same RIF may be
   *  used for both IP Routing and MPLS LSR.)
   */
  uint8 uc_rpf_enable;
  /*
   *  If enable, use bridge V4MC forwarding or double capacity for IPv4 in external TCAM.
   */
  uint8 custom_rif_bit;

  /*
   *  Virtual routing and forwarding instance ID. Only relevant
   *  when the RIF is utilized for IP routing.
   */
  SOC_PPC_VRF_ID vrf_id;
  /*
   *  Class of Service mapping profile. Setting the profile to
   *  '0' keeps the previous TC and DP values
   */
  uint32 cos_profile;
  /*
   *  When packet is routed (IP/MPLS routing) to this RIF then
   *  packet's TTL is compared againsy the TTL pointed by this
   *  index and if it less or equal then packet is filter. Use
   *  soc_ppd_rif_ttl_scope_set to set the TTL value. Range: 0 - 7.
   *  use
   *  soc_ppd_trap_eg_profile_info_set(SOC_PPC_TRAP_EG_TYPE_TTL_SCOPE,
   *  eg_trap_info) to set how to handle packets match
   *  condition of this filter.
   */
  uint32 ttl_scope_index;
  /*
   *  BIT MAP of routing enables options. Bit 0: IP Unicast
   *  Routing EnableBit 1: IP Multicast Routing EnableBit 2:
   *  MPLS Processing EnableIn Soc_petra-B, only valid for VSID
   *  RIFs see SOC_PPC_RIF_ROUTE_ENABLE_TYPE
   */
  uint32 routing_enablers_bm;

  /*
   *  only for Jericho, the routing_enablers_bm id to use in the rif profile.
   */
  uint8  routing_enablers_bm_id;

  /*
   *  only for Jericho, routing enabler bm rif profile.
   */
  uint32  routing_enablers_bm_rif_profile;

  /*
   *  only for Jericho, in case this is the first time we use the enabler_bm_id we need to configure the HW .
   */
  int  is_configure_enabler_needed;

  /* 
   * enable default routing, i.e.
   * for IP packets if first lookup according to <VRF,DIP> fails consider second lookup according to <0, DIP>. 
   * ARAD only.
   */
  uint8 enable_default_routing;
#ifdef BCM_88660_A0
  /*
   * Control unicast rpf mode - strict or loose. 
   * Default is NONE. 
   * Valid values are NONE, UC_LOOSE and UC_STRICT. 
   */
	SOC_PPC_FRWRD_FEC_RPF_MODE uc_rpf_mode;
#endif

  /* Configurable profile ID for FP */
  uint8 intf_class;

} SOC_PPC_RIF_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  see SOC_PPC_RIF_IP_TERM_FLAG_xxx
   */
  uint32 flags;
  /*
   *  Destination IP
   */
  uint32 dip;
  /*
   *  Destination IP Mask
   */
  uint32 dip_prefix_len;
  /*
   *  Source IP
   */
  uint32 sip;
  /*
   *  Source IP Mask
   */
  uint32 sip_prefix_len;

  /* ipv6 only when IPv6 flag set*/
  SOC_SAND_PP_IPV6_SUBNET dip6;
  /*
   *  VRF from VTT termination
   */
  uint32 vrf;
  /* 
   *  Next protocol above IP tunnel
   *  In ARAD, ARAD+ supported protocols above are:
   *  IPV4, IPV6, GRE. Other protocol types should be masked.
   */
  uint16 ipv4_next_protocol;
  /* 
   *  Next protocol above IP tunnel Mask
   */
  uint16 ipv4_next_protocol_prefix_len;
  /*
   *  Port property to be used as part of the key.
   */
  uint32 port_property;
  /*
   *  Indication whether port property should be used.
   */
  uint8 port_property_en;
  /*
   * my-vtep-index. Result from DIP lookup in VT. 
   */
  uint8 my_vtep_index; 
  /*
   * GRE.ethertype: Next protocol after IP-GRE tunnel 
   */
  uint16 gre_ethertype; 
  /*
   * GRE.ethertype mask
   */
  uint16 gre_ethertype_prefix_len; 
  /*
   * indicate the tunnel is an IP-GRE tunnel 
   */
  uint8 ip_gre_tunnel; 

} SOC_PPC_RIF_IP_TERM_KEY;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  RIF Id. When RIF == SOC_PPC_RIF_NULL, the default RIF ID and
   *  RPF enable flag are not updated.
   */
  SOC_PPC_RIF_ID rif;
  /*
   *  Class of Service mapping profile. Setting the profile to
   *  '0' keeps the previous TC and DP values T20E: Ignored.
   */
  uint32 cos_profile;

  /*
   * lif general profile,
   * used for OAM and PMF setting
   * range: 0-15.
   * Used only in Arad.
   */
  uint32 lif_profile;

  /*
   * points to one of the global tpid profiles, which set by soc_ppd_llp_parse_tpid_profile_info_set
   * Needed in case next header (after termination) is Ethernet
   * used for parsing the Inner L2 header,
   * range: 0 -3.
   * Used only in Arad.
   */
  uint32 tpid_profile;

  /*
   * Default LIF forwarding decision to apply when there is
   * no hit in the MAC table. The profile ID is part of the
   * 'dflt_frwrd_key', used by
   * soc_ppd_lif_default_frwrd_info_set() Soc_petra only.
   * range: 0 -3.
   * Used only in Arad.
   */
  uint32 default_forward_profile;

  /*
   * Virtual Switch ID.
   * needed in case of EthoIP
   * this is the VSI used for switching according to the internal
   * Ethernet header
   * Used only in Arad.
   */
  SOC_PPC_VSI_ID vsi;

  /*
   * Learning information
   * needed in case of EthoIP.
   * Arad: can learn FEC or flow,
   * additional info has to be empty/none, in-LIF is learned according to LIF-index was access.
   * relevant only if learn_enable set to TRUE
   * Used only in Arad.
   */
  SOC_PPC_FRWRD_DECISION_INFO learn_record;
  /*
   * enable Learning on this tunnel relevant in case of EthoIP.
   * Used only in Arad.
   */
  uint8 learn_enable;

  /*
   * VSI assignment mode,
   * used to refine above VSI value
   * range: 0-3.
   * Used only in Arad.
   */
  uint32 vsi_assignment_mode;

  /*
   * OAM instance to observe as failover id
   * Used only in Arad.
   */
  uint32 protection_pointer;
  /*
   * OAM instance pass value.
   * oam_instance_id.value != oam_instance_pass_val;
   * then packet is dropped otherwise packet is forwarded
   * range: 0 - 1.
   * Used only in Arad.
   */
  uint8 protection_pass_value;

  /*
   * is this LIF support OAM,
   * Used only in Arad.
   */
  uint8 oam_valid;
  /*
   * model pipe or uniform
   * Used only in Arad.
   */
  SOC_PPC_MPLS_TERM_MODEL_TYPE processing_type;
  /*
   * orientation hub or spoke
   * Used only in Arad.
   */
  SOC_SAND_PP_HUB_SPOKE_ORIENTATION orientation;

  /* 
   * skip ethernet enable ethernet termination
   * regardless of the ethernet values 
   */ 
  uint8 skip_ethernet;
  /*
   * flag to indicate LIF has extension. Only relevant in Jericho.
   */
  uint8 is_extended;

  /*
   * Extension lif id for this lif. Only relevant in Jericho.
   */
  int ext_lif_id;

  /* 
   * Global lif that this ip term rif is pointing to.
   *
   */
  int global_lif;

} SOC_PPC_RIF_IP_TERM_INFO;




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
  SOC_PPC_RIF_MPLS_LABELS_RANGE_clear(
    SOC_SAND_OUT SOC_PPC_RIF_MPLS_LABELS_RANGE *info
  );

void
  SOC_PPC_MPLS_LABEL_RIF_KEY_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_LABEL_RIF_KEY *info
  );

void
  SOC_PPC_RIF_INFO_clear(
    SOC_SAND_OUT SOC_PPC_RIF_INFO *info
  );

void
  SOC_PPC_RIF_IP_TERM_INFO_clear(
    SOC_SAND_OUT SOC_PPC_RIF_IP_TERM_INFO *info
  );

void
  SOC_PPC_RIF_IP_TERM_KEY_clear(
    SOC_SAND_OUT SOC_PPC_RIF_IP_TERM_KEY *key
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_RIF_ROUTE_ENABLE_TYPE_to_string(
    SOC_SAND_IN  uint32 enum_val
  );

void
  SOC_PPC_RIF_MPLS_LABELS_RANGE_print(
    SOC_SAND_IN  SOC_PPC_RIF_MPLS_LABELS_RANGE *info
  );

void
  SOC_PPC_MPLS_LABEL_RIF_KEY_print(
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY *info
  );

void
  SOC_PPC_RIF_INFO_print(
    SOC_SAND_IN  SOC_PPC_RIF_INFO *info
  );

void
  SOC_PPC_RIF_IP_TERM_INFO_print(
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO *info
  );

void
  SOC_PPC_RIF_IP_TERM_KEY_print(
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY *key
  );

const char*
  SOC_PPC_MPLS_LABEL_INDEX_to_string(
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_INDEX enum_val
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_RIF_INCLUDED__*/
#endif
