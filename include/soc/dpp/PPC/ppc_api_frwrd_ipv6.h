/* $Id: ppc_api_frwrd_ipv6.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_ipv6.h
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

#ifndef __SOC_PPC_API_FRWRD_IPV6_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_IPV6_INCLUDED__

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
   *  The default action is the FEC pointer that uses entry
   *  from the FEC table.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE_FEC = 0,
  /*
   *  The default action is the action profile.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE_ACTION_PROFILE = 1,
  /*
   *  Number of types in SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE
   */
  SOC_PPC_NOF_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPES = 2
}SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Forward according to FEC entry in the FEC table.
   */
  SOC_PPC_FEC_ID fec_id;
  /*
   *  Action profile. To forward packet to CPU. In this case,
   *  the VRF ID will be set as CPU-Trap-Qualifier.
   *  trap_code range 0-255.
   */
  SOC_PPC_ACTION_PROFILE action_profile;

} SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Type of the default forwarding action may be FEC or
   *  Action profile.
   *  in Soc_petra-B: only SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE_ACTION_PROFILE
   *  is supported, and action_profile.trap_code has to be
   *  SOC_PPC_TRAP_CODE_DEFAULT_UCV6/SOC_PPC_TRAP_CODE_DEFAULT_MCV6
   *  for uc/mc action respectively.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE type;
  /*
   *  The default action.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_VAL value;

} SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The default action of the Router for IPv6 MC lookups.
   *  May be FEC pointer or action profile.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION uc_default_action;
  /*
   *  The default action of the Router for IPv6 MC lookups.
   *  May be FEC pointer or action profile.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION mc_default_action;

} SOC_PPC_FRWRD_IPV6_ROUTER_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Global information for routing including the default
   *  actions for UC and MC routes.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_INFO router_info;

} SOC_PPC_FRWRD_IPV6_GLBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Global information for routing including the default
   *  actions for UC and MC routes.
   */
  SOC_PPC_FRWRD_IPV6_ROUTER_INFO router_info;

} SOC_PPC_FRWRD_IPV6_VRF_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IPv6 subnet IP-address/Prefix has to be Unicast
   *  IP-address.
   */
  SOC_SAND_PP_IPV6_SUBNET subnet;

  /*
   *  Relevant for Jericho.
   *  indicates that this entry should go to to the secondary IPv6 UC tables.
   *  Meant to scale the number of IPv6 routes.
   */
  uint8 route_scale;

} SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The multicast IPv6 address of the destination group. Has
   *  to be multicast address.
   */
  SOC_SAND_PP_IPV6_SUBNET group;
  /*
   *  The Incoming router interface. May be bitmap masked.
   */
  SOC_SAND_PP_MASKED_VAL inrif;
  /*
   *  Source Subnet (in Arad, available only in external TCAM).
   */
  SOC_SAND_PP_IPV6_SUBNET source;
  /*
   *  VRF ID.
   */
  SOC_PPC_VRF_ID vrf_ndx;

} SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY;

/* Identical structures, to avoid a lot of changes */
typedef SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY                 SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY;

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

} SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO;


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
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_VAL_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_VAL *info
  );

void
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION *info
  );

void
  SOC_PPC_FRWRD_IPV6_ROUTER_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_ROUTER_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV6_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV6_VRF_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_VRF_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_TYPE enum_val
  );

void
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_VAL_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_VAL *info
  );

void
  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_ROUTER_DEFAULT_ACTION *info
  );

void
  SOC_PPC_FRWRD_IPV6_ROUTER_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_ROUTER_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV6_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV6_VRF_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VRF_INFO *info
  );

void
  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO *info
  );
#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_IPV6_INCLUDED__*/
#endif

