/* $Id: ppc_api_frwrd_ipv4.h,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_ipv4.h
*
* MODULE PREFIX:  soc_ppc_frwrd
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

#ifndef __SOC_PPC_API_FRWRD_IPV4_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_IPV4_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_array_memory_allocator.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     The value SOC_PPC_FRWRD_IP_ALL_VRFS_ID stands for all VRFs.
 *     May Used to apply setting to all VRFs (when mentioned)  */
#define  SOC_PPC_FRWRD_IP_ALL_VRFS_ID (0xFFFFFFFF)

/*   host only call*/
#define  SOC_PPC_FRWRD_IP_HOST_ONLY (0x1)

/*   lpm only call */
#define  SOC_PPC_FRWRD_IP_LPM_ONLY (0x2)

/* get exact match entry in LPM/DB */
#define  SOC_PPC_FRWRD_IP_EXACT_MATCH  (0x1)
/* clear hit indication on get  */
#define  SOC_PPC_FRWRD_IP_CLEAR_ON_GET  (0x2)

/* host flags */

/* clear accessed indication   */
#define  SOC_PPC_FRWRD_IP_HOST_CLEAR_ON_GET  (0x1)

/* for host-get-block get accessed entries only */
#define  SOC_PPC_FRWRD_IP_HOST_GET_ACCESSED_ONLY  (0x2)

/* for get-block get accessed status (require another access to HW) */
#define  SOC_PPC_FRWRD_IP_HOST_GET_ACCESSS_STATUS   (0x4)

/* for host_traverse kbp indication to distinguish between host and route */
#define  SOC_PPC_FRWRD_IP_HOST_KBP_HOST_INDICATION  (0x8)

/* for IPMC BIDIR mask Intf */
#define  SOC_PPC_FRWRD_IP_MC_BIDIR_IGNORE_RIF   (0x1)


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
   *  The default action is the FEC pointer that uses the
   *  entry from the FEC table.
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPE_FEC = 0,
  /*
   *  The default action is the action profile.
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPE_ACTION_PROFILE = 1,
  /*
   *  Number of types in SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPE
   */
  SOC_PPC_NOF_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPES = 2
}SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPE;

typedef enum
{
  /*
   *  For Ipv4 hosts (keys with prefix length = 32), use the
   *  longest prefix match DB, and only when no available
   *  space, use the Large Exact Match DB.
   */
  SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE_LPM_THEN_LEM = 0,
  /*
   *  For Ipv4 hosts (keys with prefix length = 32), use ONLY
   *  longest prefix match DB.
   */
  SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE_LPM_ONLY = 1,
  /*
   *  For Ipv4 hosts (keys with prefix length = 32), use Large
   *  Exact Match DB, and only when no available space, use
   *  the longest prefix match DB.
   */
  SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE_LEM_THEN_LPM = 2,
  /*
   *  For Ipv4 hosts (keys with prefix length = 32), use ONLY
   *  Large Exact Match DB.
   */
  SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE_LEM_ONLY = 3,
  /*
   *  Number of types in SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE
   */
  SOC_PPC_NOF_FRWRD_IPV4_HOST_TABLE_RESOURCES = 4
}SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE;

typedef enum
{
  /*
   *  For Ipv4 MC hosts (keys with prefix length = 32), use
   *  ONLY TCAM.
   */
  SOC_PPC_FRWRD_IPV4_MC_HOST_TABLE_RESOURCE_TCAM_ONLY = 0,
  /*
   *  Number of types in SOC_PPC_FRWRD_IPV4_MC_HOST_TABLE_RESOURCE
   */
  SOC_PPC_NOF_FRWRD_IPV4_MC_HOST_TABLE_RESOURCES = 1
}SOC_PPC_FRWRD_IPV4_MC_HOST_TABLE_RESOURCE;

typedef enum
{
  /*
   *  no caching all
   */
  SOC_PPC_FRWRD_IP_CACHE_MODE_NONE = 0,
  /*
   *  cache all routes added to LPM
   */
  SOC_PPC_FRWRD_IP_CACHE_MODE_IPV4_UC_LPM = 1,

  SOC_PPC_NOF_FRWRD_IP_CACHE_MODES = 2
}SOC_PPC_FRWRD_IP_CACHE_MODE;

typedef enum
{
  /*
   *  The route exists in both sofwate cache and in device.
   */
  SOC_PPC_FRWRD_IP_ROUTE_STATUS_COMMITED = 1,
  /*
   *  The route exists in software cache and is pending to be
   *  commited (added) into device.
   */
  SOC_PPC_FRWRD_IP_ROUTE_STATUS_PEND_ADD = 2,
  /*
   *  The route was removed from software cache and is pending
   *  to be commited (removed) into device.
   */
  SOC_PPC_FRWRD_IP_ROUTE_STATUS_PEND_REMOVE = 4,
  /*
   *  entry was accessed by traffic lookup
   */
  SOC_PPC_FRWRD_IP_ROUTE_STATUS_ACCESSED = 8,
  /*
   *  Number of types in SOC_PPC_FRWRD_IP_ROUTE_STATUS
   */
  SOC_PPC_NOF_FRWRD_IP_ROUTE_STATUSS = 3
}SOC_PPC_FRWRD_IP_ROUTE_STATUS;

typedef enum
{
  /*
   *  The route (host) exists in host DB.
   */
  SOC_PPC_FRWRD_IP_ROUTE_LOCATION_HOST = 0,
  /*
   *  The route (host/subnet) exists in LPM DB
   */
  SOC_PPC_FRWRD_IP_ROUTE_LOCATION_LPM = 1,
  /*
   *  The route (host/subnet) exists in TCAM
   */
  SOC_PPC_FRWRD_IP_ROUTE_LOCATION_TCAM = 2,
  /*
   *  Number of types in SOC_PPC_FRWRD_IP_ROUTE_LOCATION
   */
  SOC_PPC_NOF_FRWRD_IP_ROUTE_LOCATIONS = 3
}SOC_PPC_FRWRD_IP_ROUTE_LOCATION;

typedef union
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Forward according to FEC entry in the FEC table.
   */
  uint32 route_val;
  /*
   *  Action profile. To forward packet to CPU. In this case,
   *  the VRF ID will be set as CPU-Trap-Qualifier.
   *  trap_code range 0-255.
   */
  SOC_PPC_ACTION_PROFILE action_profile;

} SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Type of the default forwarding action may be FEC or
   *  Action profile.
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPE type;
  /*
   *  The default action.
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL value;

} SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The default action of the Router for IPv4 UC lookups.
   *  May be FEC pointer or action profile.
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION uc_default_action;
  /*
   *  The default action of the Router for IPv4 MC lookups.
   *  May be FEC pointer or action profile.
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION mc_default_action;
  /*
   * if IP MC lookup fail then flood packet on in-VLAN 
   * if not set then packet will be forwarded according VRF default 
   * This is a global configuration and no per VRF 
   * ARAD only
   */
  uint8 flood_unknown_mc;

  /*
   * when IPMC routing is disabled on in-RIF. 
   * then this determine what is the L2 lookup to perform 
   * 0:<FID,DA>, 1:<FID,DIP>
   * ARAD only
   */
  uint8 ipv4_mc_l2_lookup;
/*
   * for strict RPF check whether to consider, VRF default destination (FEC) 
   * in the RPF check. 
   * if TRUE: then for IP packets, if DIP lookup fail then Router default destination 
   * is taken in RPF check.
   * This is a global configuration and not per VRF 
   * ARAD only
   */
  uint8 rpf_default_route;

} SOC_PPC_FRWRD_IPV4_ROUTER_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Information for routing, including the default actions
   *  for UC and MC routes.
   *  soc_petra-B: this is not effective, as in LPM
   *  defatul action is taken from default route.
   *  0.0.0.0/0
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_INFO router_info;
  /*
   *  When TRUE, if lookup in this VRF routing table fails, a
   *  lookup will performed in default routing table (VRF
   *  zero). Note: A system can either do L3VPN default
   *  forwarding or RPF, not both.
   */
  uint8 use_dflt_non_vrf_routing;

} SOC_PPC_FRWRD_IPV4_VRF_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Global information for routing including the default
   *  actions for UC and MC routes.
   *  soc_petra-B: this is not effective, as in LPM
   *  defatul action is taken from default route.
   *  0.0.0.0/0
   */
  SOC_PPC_FRWRD_IPV4_ROUTER_INFO router_info;
  /*
   *  The resources to use for IPv4 UC (VRF = 0).
   */
  SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE uc_table_resouces;
  /*
   *  The resources to use for IPv4 MC (VRF = 0)
   */
  SOC_PPC_FRWRD_IPV4_MC_HOST_TABLE_RESOURCE mc_table_resouces;

} SOC_PPC_FRWRD_IPV4_GLBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPv4 subnet IP-address/Prefix has to be Unicast
   *  IP-address.
   */
  SOC_SAND_PP_IPV4_SUBNET subnet;

  /*
   *  Relevant for Jericho.
   *  indicates that this entry should go to to the secondary IPv4 UC tables.
   *  Meant to scale the number of IPv4 routes.
   */
  uint8 route_scale;

} SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The multicast IP address of the destination group. Class
   *  D. Range: 224.0.0.0 - 239.255.255.255.
   *  0.0.0.0 to indicate invalid (for default route)
   */
  uint32 group;

  /*
   *  Number of bits to consider in the group starting
   *  from the msb. Range: 0 - 32.Example for key ip_address
   *  192.168.1.0 and prefix_len 24 would match any IP Address
   *  of the form 192.168.1.x
   */
  uint8 group_prefix_len;

  /*
   *  Source Subnet.
   */
  SOC_SAND_PP_IPV4_SUBNET source;
  /*
   *  The Incoming router interface. May be masked using
   *  inrif_valid.
   */
  SOC_PPC_RIF_ID inrif;
  /*
   *  If set to FALSE then inrif is masked.
   */
  uint8 inrif_valid;
 /*
  * Filtering ID. Set according to VSI.
  */
  SOC_PPC_FID fid;

  /*
   *  VRF ID.
   */
  SOC_PPC_VRF_ID vrf_ndx;

} SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * see SOC_PPC_FRWRD_IP_MC_BIDIR 
   */
  uint32 flags;
  /*
   *  rendezvous point
   */
  uint32 rp_id;
  /*
   *  active ingress L3 interface for a rendezvous point
   */
  uint32 inrif;

} SOC_PPC_FRWRD_IP_MC_RP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPv4 subnet IP-address/Prefix. IPv4 address can be
   *  either UC or MC.
   */
  SOC_SAND_PP_IPV4_SUBNET subnet;

  /*
   *  Relevant for Jericho.
   *  indicates that this entry should go to to the secondary IPv4 UC tables.
   *  Meant to scale the number of IPv4 routes.
   */
  uint8 route_scale;

} SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VRF ID. Range: 0 - 255.
   */
  SOC_PPC_VRF_ID vrf_ndx;
  /*
   *  Ipv4 address.
   */
  uint32 ip_address;

  /*
   *  indicates that host should be add to LEM
   *  (when working with external tcam).
   */
  uint8 is_local_host;

  /*
   *  Relevant for Jericho.
   *  indicates to that this entry should go to to the secondary IPv4 UC LEM table.
   *  Meant to scale the number of IPv4 routes.
   */
  uint8 route_scale;

} SOC_PPC_FRWRD_IPV4_HOST_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  FEC ID. Range: 0 - 16383.
   *  on set
   *  Petra-B only
   */
  SOC_PPC_FEC_ID fec_id;
  /*
   *  destination.
   *  Relevant for ARAD
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_decision;
  /*
   *  Egress Encapsulation pointer. Points to link-layer
   *  next-hop/Tunnel information set to SOC_PPC_EEP_NULL to be
   *  ignored. Range: 0 - 16K-1. Remarks: for routed packets,
   *  EEP value can be obtained by two ways (only for host
   *  entries exist in LEM Database):- directly, as a result
   *  of the host lookup in the LEM.- As a part of the FEC
   *  entry information. The EEP is taken from the FEC entry
   *  unless it value is SOC_PPC_EEP_NULL.
   *  Petra-B only
   */
  uint32 eep;
  /*
   *  MAC address (DA) LSBs. To extend ARP table, out-LIF (EEP)
   *  points at one DA in the ARP table and additional LSBs
   *  (4 bits) are written in the EEI (in the bits that are
   *  not used for the out-LIF). This way nore MAC addresses
   *  (DAs) can be pointed at. Range: 0-15.
   *  ARAD only.
   */
  uint8 mac_lsb;

  /* 
   *  native-vsi. Relevant for ROO application.
   *  Provide native-SA and native vlan of the native ethernet
   */ 
  SOC_PPC_VSI_ID native_vsi;

} SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Destination. Can be:- FEC-ID. Range: 0 - 16383.-
   *  Multicast group. Range: 0 - 16K-1. When FEC is used then
   *  FEC has to include MC-group as destination. When MC-group
   *  is used then no RPF check is perfromed.
   */
  SOC_SAND_PP_DESTINATION_ID dest_id;

  uint32 flags;

} SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Status of memory, including number of fragments, max
   *  fragment size and total free memory size.
   */
  SOC_SAND_ARR_MEM_ALLOCATOR_MEM_STATUS mem_stat;

} SOC_PPC_FRWRD_IPV4_MEM_STATUS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Range determine how many iterations to
   *  perform.entries_to_scan: number of writes to HW. set to
   *  SOC_SAND_TBL_ITER_SCAN_ALL to ignore.entries_to_act: number
   *  of fragments to unify. set to SOC_SAND_TBL_ITER_SCAN_ALL to
   *  perform full defragmentation.iter: iterator, use
   *  SOC_SAND_TBL_ITER_IS_END to check if defragmention is fully
   *  done.
   */
  SOC_SAND_TABLE_BLOCK_RANGE range;

} SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO;


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
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL *info
  );

void
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION *info
  );

void
  SOC_PPC_FRWRD_IPV4_ROUTER_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_ROUTER_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_VRF_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_VRF_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IP_MC_RP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_MC_RP_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY *info
  );


void
  SOC_PPC_FRWRD_IPV4_MEM_STATUS_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MEM_STATUS *info
  );

void
  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *info
  );


void
  SOC_PPC_FRWRD_IPV4_HOST_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE enum_val
  );

const char*
  SOC_PPC_FRWRD_IPV4_MC_HOST_TABLE_RESOURCE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_HOST_TABLE_RESOURCE enum_val
  );

const char*
  SOC_PPC_FRWRD_IP_CACHE_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_CACHE_MODE enum_val
  );

const char*
  SOC_PPC_FRWRD_IP_ROUTE_STATUS_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_ROUTE_STATUS enum_val
  );

const char*
  SOC_PPC_FRWRD_IP_ROUTE_LOCATION_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_ROUTE_LOCATION enum_val
  );

void
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL *info
  );

void
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION *info
  );

void
  SOC_PPC_FRWRD_IPV4_ROUTER_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_ROUTER_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_VRF_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VRF_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IP_MC_RP_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV4_MEM_STATUS_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_STATUS *info
  );

void
  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *info
  );


void
  SOC_PPC_FRWRD_IPV4_HOST_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_IPV4_INCLUDED__*/
#endif

