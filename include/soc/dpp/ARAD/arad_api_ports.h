/* $Id: arad_api_ports.h,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_API_PORTS_INCLUDED__
/* { */
#define __ARAD_API_PORTS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/TMC/tmc_api_stack.h>
#include <soc/dpp/TMC/tmc_api_ports.h>
#include <soc/dpp/PPC/ppc_api_lag.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximal System Physical Port index. Physical Port
 *     uniquely identifies a FAP port in the system.           */
#define  ARAD_PORTS_SYS_PHYS_PORT_ID_MAX (SOC_TMC_PORTS_SYS_PHYS_PORT_ID_MAX)

/*     Maximal number of LAG-s in the system                   */
#define  ARAD_PORT_LAGS_MAX (ARAD_MAX_LAG_GROUP_ID)

/*     Maximal number of ports in Out-LAG                      */
#define  ARAD_PORTS_LAG_OUT_MEMBERS_MAX (ARAD_MAX_LAG_ENTRY_ID)

/*
 *   Maximal LAG member index.
 *   Note: though incoming LAG can contain up to 80 members,
 *   the maximal member-id is 15.
 *   If the LAG-member-id is not used by a higher protocol
 *   in the CPU, it's value is not significant (not used for LAG-based pruning)
 */
#define  ARAD_PORTS_LAG_IN_MEMBERS_MAX (SOC_TMC_PORTS_LAG_IN_MEMBERS_MAX_ARAD)

/*     Maximal number of member ports in In-LAG               */
#define  ARAD_PORTS_LAG_MEMBERS_MAX (SOC_TMC_PORTS_LAG_MEMBERS_MAX)
#define ARAD_PORTS_LAG_MEMBER_ID_MAX (ARAD_MAX_LAG_ENTRY_ID)

#define ARAD_PORT_NOF_PP_PORTS                       (256)

#define ARAD_PORTS_PP_PORT_RCY_OVERLAY_PTCH (SOC_TMC_PORT_PP_PORT_RCY_OVERLAY_PTCH)


#define ARAD_PORTS_TEST1_PORT_PP_PORT (SOC_TEST1_PORT_PP_PORT)


/* Preserving DSCP on a per out-port bases */
#define ARAD_PORTS_PRESERVING_DSCP_PORT_PP_PORT (SOC_PRESERVING_DSCP_PORT_PP_PORT)

#define ARAD_PORTS_MIM_SPB_PORT_PP_PORT (SOC_MIM_SPB_PORT)

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

#define ARAD_PORT_DIRECTION_INCOMING                      SOC_TMC_PORT_DIRECTION_INCOMING
#define ARAD_PORT_DIRECTION_OUTGOING                      SOC_TMC_PORT_DIRECTION_OUTGOING
#define ARAD_PORT_DIRECTION_BOTH                          SOC_TMC_PORT_DIRECTION_BOTH
#define ARAD_PORT_NOF_DIRECTIONS                          SOC_TMC_PORT_NOF_DIRECTIONS
typedef SOC_TMC_PORT_DIRECTION                                 ARAD_PORT_DIRECTION;

#define ARAD_PORT_HEADER_TYPE_NONE                        SOC_TMC_PORT_HEADER_TYPE_NONE
#define ARAD_PORT_HEADER_TYPE_ETH                         SOC_TMC_PORT_HEADER_TYPE_ETH
#define ARAD_PORT_HEADER_TYPE_RAW                         SOC_TMC_PORT_HEADER_TYPE_RAW
#define ARAD_PORT_HEADER_TYPE_TM                          SOC_TMC_PORT_HEADER_TYPE_TM
#define ARAD_PORT_HEADER_TYPE_PROG                        SOC_TMC_PORT_HEADER_TYPE_PROG
#define ARAD_PORT_HEADER_TYPE_CPU                         SOC_TMC_PORT_HEADER_TYPE_CPU
#define ARAD_PORT_HEADER_TYPE_STACKING                    SOC_TMC_PORT_HEADER_TYPE_STACKING
#define ARAD_PORT_HEADER_TYPE_TDM                         SOC_TMC_PORT_HEADER_TYPE_TDM
#define ARAD_PORT_HEADER_TYPE_TDM_RAW                     SOC_TMC_PORT_HEADER_TYPE_TDM_RAW
#define ARAD_PORT_HEADER_TYPE_INJECTED                    SOC_TMC_PORT_HEADER_TYPE_INJECTED
#define ARAD_PORT_HEADER_TYPE_INJECTED_PP                 SOC_TMC_PORT_HEADER_TYPE_INJECTED_PP
#define ARAD_PORT_HEADER_TYPE_INJECTED_2                  SOC_TMC_PORT_HEADER_TYPE_INJECTED_2
#define ARAD_PORT_HEADER_TYPE_INJECTED_2_PP               SOC_TMC_PORT_HEADER_TYPE_INJECTED_2_PP
#define ARAD_PORT_HEADER_TYPE_XGS_HQoS                    SOC_TMC_PORT_HEADER_TYPE_XGS_HQoS
#define ARAD_PORT_HEADER_TYPE_XGS_DiffServ                SOC_TMC_PORT_HEADER_TYPE_XGS_DiffServ
#define ARAD_PORT_HEADER_TYPE_XGS_MAC_EXT                 SOC_TMC_PORT_HEADER_TYPE_XGS_MAC_EXT
#define ARAD_PORT_HEADER_TYPE_TDM_PMM                     SOC_TMC_PORT_HEADER_TYPE_TDM_PMM
#define ARAD_PORT_HEADER_TYPE_L2_REMOTE_CPU               SOC_TMC_PORT_HEADER_TYPE_REMOTE_CPU
#define ARAD_PORT_HEADER_TYPE_UDH_ETH                     SOC_TMC_PORT_HEADER_TYPE_UDH_ETH
#define ARAD_PORT_HEADER_TYPE_MPLS_RAW                    SOC_TMC_PORT_HEADER_TYPE_MPLS_RAW
#define ARAD_PORT_HEADER_TYPE_FCOE_N_PORT                 SOC_TMC_PORT_HEADER_TYPE_FCOE_N_PORT
#define ARAD_PORT_HEADER_TYPE_L2_ENCAP_EXTERNAL_CPU		  SOC_TMC_PORT_HEADER_TYPE_L2_ENCAP_EXTERNAL_CPU
#define ARAD_PORT_HEADER_TYPE_MIRROR_RAW                  SOC_TMC_PORT_HEADER_TYPE_MIRROR_RAW

#define ARAD_PORT_HEADER_TYPE_COE                         SOC_TMC_PORT_HEADER_TYPE_COE

#define ARAD_PORT_NOF_HEADER_TYPES                        SOC_TMC_PORT_NOF_HEADER_TYPES
typedef SOC_TMC_PORT_HEADER_TYPE                               ARAD_PORT_HEADER_TYPE;

#define ARAD_PORTS_SNOOP_SIZE_BYTES_64                    SOC_TMC_PORTS_SNOOP_SIZE_BYTES_64
#define ARAD_PORTS_SNOOP_SIZE_BYTES_192                   SOC_TMC_PORTS_SNOOP_SIZE_BYTES_192
#define ARAD_PORTS_SNOOP_SIZE_ALL                         SOC_TMC_PORTS_SNOOP_SIZE_ALL
#define ARAD_PORTS_NOF_SNOOP_SIZES                        SOC_TMC_PORTS_NOF_SNOOP_SIZES
typedef SOC_TMC_PORTS_SNOOP_SIZE                               ARAD_PORTS_SNOOP_SIZE;

#define ARAD_PORTS_FTMH_EXT_OUTLIF_NEVER                  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_NEVER
#define ARAD_PORTS_FTMH_EXT_OUTLIF_IF_MC                  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_IF_MC
#define ARAD_PORTS_FTMH_EXT_OUTLIF_ALWAYS                 SOC_TMC_PORTS_FTMH_EXT_OUTLIF_ALWAYS
#define ARAD_PORTS_FTMH_EXT_OUTLIF_DOUBLE_TAG             SOC_TMC_PORTS_FTMH_EXT_OUTLIF_DOUBLE_TAG
#define ARAD_PORTS_FTMH_NOF_EXT_OUTLIFS                   SOC_TMC_PORTS_FTMH_NOF_EXT_OUTLIFS
typedef SOC_TMC_PORTS_FTMH_EXT_OUTLIF                          ARAD_PORTS_FTMH_EXT_OUTLIF;

#define ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE_A              SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_A
#define ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE_B              SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_B
#define ARAD_PORT_NOF_EGR_HDR_CR_DISCOUNT_TYPES           SOC_TMC_PORT_NOF_EGR_HDR_CR_DISCOUNT_TYPES
typedef SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE                  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE;

typedef SOC_TMC_PORT2IF_MAPPING_INFO                           ARAD_PORT2IF_MAPPING_INFO;
typedef SOC_TMC_PORTS_LAG_MEMBER                               ARAD_PORTS_LAG_MEMBER;
typedef SOC_TMC_PORTS_OVERRIDE_INFO                            ARAD_PORTS_OVERRIDE_INFO;
typedef SOC_TMC_PORT_INBOUND_MIRROR_INFO                       ARAD_PORT_INBOUND_MIRROR_INFO;
typedef SOC_TMC_PORT_OUTBOUND_MIRROR_INFO                      ARAD_PORT_OUTBOUND_MIRROR_INFO;
typedef SOC_TMC_PORT_SNOOP_INFO                                ARAD_PORT_SNOOP_INFO;
typedef SOC_TMC_PORTS_ITMH_BASE                                ARAD_PORTS_ITMH_BASE;
typedef SOC_TMC_PORTS_ITMH_EXT_SRC_PORT                        ARAD_PORTS_ITMH_EXT_SRC_PORT;
typedef SOC_TMC_PORTS_ITMH                                     ARAD_PORTS_ITMH;
typedef SOC_TMC_PORTS_ISP_INFO                                 ARAD_PORTS_ISP_INFO;
typedef SOC_TMC_PORTS_STAG_FIELDS                              ARAD_PORTS_STAG_FIELDS;
typedef SOC_TMC_PORTS_OTMH_EXTENSIONS_EN                       ARAD_PORTS_OTMH_EXTENSIONS_EN;
typedef SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO                  ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO;

typedef SOC_TMC_PORT_LAG_SYS_PORT_INFO                         ARAD_PORT_LAG_SYS_PORT_INFO;

#define ARAD_PORTS_FC_TYPE_NONE                              SOC_TMC_PORTS_FC_TYPE_NONE
#define ARAD_PORTS_FC_TYPE_LL                                SOC_TMC_PORTS_FC_TYPE_LL
#define ARAD_PORTS_FC_TYPE_CB2                               SOC_TMC_PORTS_FC_TYPE_CB2
#define ARAD_PORTS_FC_TYPE_CB8                               SOC_TMC_PORTS_FC_TYPE_CB8
#define ARAD_PORTS_NOF_FC_TYPES                              SOC_TMC_PORTS_NOF_FC_TYPES
typedef SOC_TMC_PORTS_FC_TYPE                                  ARAD_PORTS_FC_TYPE;

typedef SOC_TMC_PORT_PP_PORT_INFO                              ARAD_PORT_PP_PORT_INFO;
typedef SOC_TMC_PORT_COUNTER_INFO                              ARAD_PORT_COUNTER_INFO;
typedef SOC_TMC_PORTS_FORWARDING_HEADER_INFO                   ARAD_PORTS_FORWARDING_HEADER_INFO;

#define ARAD_PORT_LAG_MODE_1K_16                   SOC_TMC_PORT_LAG_MODE_1K_16     
#define ARAD_PORT_LAG_MODE_512_32                  SOC_TMC_PORT_LAG_MODE_512_32     
#define ARAD_PORT_LAG_MODE_256_64                  SOC_TMC_PORT_LAG_MODE_256_64    
#define ARAD_PORT_LAG_MODE_128_128                 SOC_TMC_PORT_LAG_MODE_128_128   
#define ARAD_PORT_LAG_MODE_64_256                  SOC_TMC_PORT_LAG_MODE_64_256      
#define ARAD_NOF_PORT_LAG_MODES                    SOC_TMC_NOF_PORT_LAG_MODES      
typedef SOC_TMC_PORT_LAG_MODE                            ARAD_PORT_LAG_MODE;

typedef SOC_TMC_PORTS_SWAP_INFO                    ARAD_PORTS_SWAP_INFO;

typedef SOC_TMC_PORTS_PON_TUNNEL_INFO              ARAD_PORTS_PON_TUNNEL_INFO;

typedef SOC_TMC_L2_ENCAP_INFO                      ARAD_L2_ENCAP_INFO;

#define ARAD_PORTS_VT_PROFILE_NONE                 SOC_TMC_PORTS_VT_PROFILE_NONE
#define ARAD_PORTS_VT_PROFILE_OVERLAY_RCY          SOC_TMC_PORTS_VT_PROFILE_OVERLAY_RCY
#define ARAD_PORTS_VT_PROFILE_CUSTOM_PP            SOC_TMC_PORTS_VT_PROFILE_CUSTOM_PP
#define ARAD_PORTS_NOF_VT_PROFILES                 SOC_TMC_PORTS_NOF_VT_PROFILES
typedef SOC_TMC_PORTS_VT_PROFILE                   ARAD_PORTS_VT_PROFILE;

#define ARAD_PORTS_TT_PROFILE_NONE                 SOC_TMC_PORTS_TT_PROFILE_NONE
#define ARAD_PORTS_TT_PROFILE_OVERLAY_RCY          SOC_TMC_PORTS_TT_PROFILE_OVERLAY_RCY
#define ARAD_PORTS_TT_PROFILE_PON                  SOC_TMC_PORTS_TT_PROFILE_PON
#define ARAD_PORTS_NOF_TT_PROFILES                 SOC_TMC_PORTS_NOF_TT_PROFILES
typedef SOC_TMC_PORTS_TT_PROFILE                   ARAD_PORTS_TT_PROFILE;

#define ARAD_PORTS_FLP_PROFILE_NONE                SOC_TMC_PORTS_FLP_PROFILE_NONE
#define ARAD_PORTS_FLP_PROFILE_OVERLAY_RCY         SOC_TMC_PORTS_FLP_PROFILE_OVERLAY_RCY
#define ARAD_PORTS_NOF_FLP_PROFILES                SOC_TMC_PORTS_NOF_FLP_PROFILES
typedef SOC_TMC_PORTS_FLP_PROFILE                  ARAD_PORTS_FLP_PROFILE;

typedef SOC_TMC_PORTS_PROGRAMS_INFO                ARAD_PORTS_PROGRAMS_INFO;

#define ARAD_PORTS_APPLICATION_MAPPING_TYPE_XGS_MAC_EXTENDER SOC_TMC_PORTS_APPLICATION_MAPPING_TYPE_XGS_MAC_EXTENDER
#define ARAD_PORTS_APPLICATION_MAPPING_TYPE_PP_PORT_EXTENDER SOC_TMC_PORTS_APPLICATION_MAPPING_TYPE_PP_PORT_EXTENDER
#define ARAD_PORTS_APPLICATION_MAPPING_NOF_TYPES             SOC_TMC_PORTS_APPLICATION_MAPPING_NOF_TYPES
typedef SOC_TMC_PORTS_APPLICATION_MAPPING_INFO               ARAD_PORTS_APPLICATION_MAPPING_INFO;

typedef SOC_TMC_PORTS_ILKN_CONFIG                  ARAD_PORTS_ILKN_CONFIG;                     
typedef SOC_TMC_PORTS_CAUI_CONFIG                  ARAD_PORTS_CAUI_CONFIG;                     

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
*   arad_sys_phys_to_local_port_map_set
* TYPE:
*   PROC
* FUNCTION:
*   Map System Physical FAP Port to a <mapped_fap_id, mapped_fap_port_id>
*   pair. The mapping is unique - single System Physical
*   Port is mapped to a single local port per specified
*   device. This configuration effects: 1. Resolving
*   destination FAP Id and OFP Id 2. Per-port pruning
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 sys_phys_port_ndx -
*     The index of system physical port. Range: 0 - 4095.
*   SOC_SAND_IN  uint32                                 mapped_fap_id -
*     The device id of the port that is mapped. Range: 0 -
*     2047.
*   SOC_SAND_IN  uint32                                 mapped_fap_port_id -
*     Local (per device) FAP Port id. Range: 0 - 79.
* REMARKS:
*   1. The mapping is identical for incoming and outgoing
*   FAP Ports. 2. Mapping the device to system fap port must
*   be performed before calling this API.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sys_phys_to_local_port_map_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 sys_phys_port_ndx,
    SOC_SAND_IN  uint32                 mapped_fap_id,
    SOC_SAND_IN  uint32                 mapped_fap_port_id
  );

/*********************************************************************
* NAME:
*   arad_sys_phys_to_local_port_map_get
* TYPE:
*   PROC
* FUNCTION:
*   Map System Physical FAP Port to a <mapped_fap_id, mapped_fap_port_id>
*   pair. The mapping is unique - single System Physical
*   Port is mapped to a single local port per specified
*   device. This configuration effects: 1. Resolving
*   destination FAP Id and OFP Id 2. Per-port pruning
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 sys_phys_port_ndx -
*     The index of system physical port. Range: 0 - 4095.
*   SOC_SAND_OUT uint32                                 *mapped_fap_id -
*     The device id of the port that is mapped. Range: 0 -
*     2047.
*   SOC_SAND_OUT uint32                                 *mapped_fap_port_id -
*     Local (per device) FAP Port id. Range: 0 - 79.
* REMARKS:
*   1. The mapping is identical for incoming and outgoing
*   FAP Ports. 2. Mapping the device to system fap port must
*   be performed before calling this API.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sys_phys_to_local_port_map_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 sys_phys_port_ndx,
    SOC_SAND_OUT uint32                 *mapped_fap_id,
    SOC_SAND_OUT uint32                 *mapped_fap_port_id
  );

/*********************************************************************
* NAME:
*     arad_local_to_sys_phys_port_map_get
* TYPE:
*   PROC
* FUNCTION:
*     Get a System Physical FAP Port mapped to a FAP port in
*     the local device. The mapping is unique - single System
*     Physical Port is mapped to a single local port per
*     specified device. This configuration effects: 1.
*     Resolving destination FAP Id and OFP Id 2. Per-port
*     pruning
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 fap_ndx -
*     Local (per device) FAP Port id. Range: 0 - 79.
*  SOC_SAND_IN  uint32                 fap_local_port_ndx -
*     The device id of the port that is mapped.
*  SOC_SAND_OUT uint32                 *sys_phys_port_id -
*     The index of system physical port. Range: 0 - 4095.
* REMARKS:
*     The mapping is identical for incoming and outgoing FAP
*     Ports.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_local_to_sys_phys_port_map_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 fap_ndx,
    SOC_SAND_IN  uint32                 fap_local_port_ndx,
    SOC_SAND_OUT uint32                 *sys_phys_port_id
  );

/*********************************************************************
* Get the System Physical FAP Port (sysport) mapped to the given modport
*********************************************************************/
uint32
  arad_modport_to_sys_phys_port_map_get(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  uint32  fap_id,            /* input FAP ID /module */
    SOC_SAND_IN  uint32  tm_port,           /* input TM port, make a modport with fap_id */
    SOC_SAND_OUT uint32  *sys_phys_port_id  /* output sysport */
  );

/*********************************************************************
* NAME:
*     arad_port_to_interface_map_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Maps the specified FAP Port to interface and channel.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     Fap port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the mapped port
*     (incoming/outgoing/both).
*  SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO *info -
*     Port to Interface mapping configuration.
* REMARKS:
*   1. ch_id is only relevant for channelized interfaces -
*   ignored otherwise.
*   2. To unmap a port without mapping to another interface -
*   use ARAD_IF_ID_NONE as info.if_id value.
*   3. The get function is not entirely symmetric to the set function
*     (where only incoming, outgoing or both directions can be defined).
*     The get function returns both directions.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_to_dynamic_interface_map_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  ARAD_PORT2IF_MAPPING_INFO *info,
    SOC_SAND_IN  uint8                    is_init
  );

int
  arad_port_to_interface_map_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  soc_port_t          port,
    SOC_SAND_IN  int                 unmap
  );

/********************************************************************* 
* NAME: 
*   arad_ports_is_port_lag_member  
* FUNCTION: 
*   Gives LAG information of . 
* INPUT: 
*   SOC_SAND_IN  int                                 unit - 
*     Identifier of the device to access. 
*   SOC_SAND_IN  uint32                                 port_id - 
*     local port index. 
*   SOC_SAND_OUT  uint8                                *is_in_lag - 
*     set to TRUE if the port is a lag member. 
*   SOC_SAND_OUT  uint32                                *lag_ndx - 
*     is is_in_lag = TRUE, set to LAG index of the port. Range: 0 - 255. 
* 
* RETURNS: 
*   OK or ERROR indication. 
*********************************************************************/ 
   
uint32  
    arad_ports_is_port_lag_member( 
              SOC_SAND_IN  int                                 unit, 
              SOC_SAND_IN  int                                 core_id,
              SOC_SAND_IN  uint32                                 port_id, 
              SOC_SAND_OUT uint8                                 *is_in_lag, 
              SOC_SAND_OUT uint32                                 *lag_id); 

/*********************************************************************
* NAME:
*   arad_ports_lag_set
* TYPE:
*   PROC
* FUNCTION:
*   Configure a LAG. A LAG is defined by a group of System
*   Physical Ports that compose it. This configuration
*   affects 1. LAG resolution for queuing at the ingress. 2.
*   LAG-based pruning.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_PORT_DIRECTION                      direction_ndx -
*     LAG direction - incoming, outgoing or both. For
*     symmetrical LAG configuration - set direction as "both".
*     For different Incoming and Outgoing LAG configuration -
*     per-direction configuration is required.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255. SOC_SAND_OUT uint32
*   SOC_SAND_IN  SOC_PPC_LAG_INFO                      *info -
*     Lag members. Maximal number of out-going LAG members is
*     16. The number of incoming LAG members is not limited,
*     and it can be the number of Local FAP ports in each
*     device (80).
* REMARKS:
*   1. Local to system port mapping must be configured
*   before using this API (Incoming and Outgoing) - for LAG
*   pruning. 2. LAG configuration must be consistent
*   system-wide, for incoming and outgoing ports. 3. The
*   same system port can be added multiple times. This
*   affects the load-balancing, according to the number of
*   times the port appears in the LAG. 4. The _get function
*   is not symmetric to the set function: both incoming and
*   outgoing settings are returned (direction_ndx is not
*   passed). 5. For the INCOMING-LAG, the _get function
*   returns only LAG member sys-ports that are mapped to
*   local FAP ports, on the local device. 6. Setting LAG
*   with a group of system ports, will first clean-up any
*   previous configuration of the LAG. For example setting
*   LAG 1 with system members 1,2,3,4 and then setting the
*   same LAG with members 3,4,5,6 will clean up the effect
*   of the previous configuration and set up the new
*   configuration.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  uint32                 lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_INFO      *info
  );

/*********************************************************************
* NAME:
*   arad_ports_lag_get
* TYPE:
*   PROC
* FUNCTION:
*   Configure a LAG. A LAG is defined by a group of System
*   Physical Ports that compose it. This configuration
*   affects 1. LAG resolution for queuing at the ingress. 2.
*   LAG-based pruning.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_PORT_DIRECTION                      direction_ndx -
*     LAG direction - incoming, outgoing or both. For
*     symmetrical LAG configuration - set direction as "both".
*     For different Incoming and Outgoing LAG configuration -
*     per-direction configuration is required.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255. SOC_SAND_OUT uint32
*   SOC_SAND_OUT SOC_PPC_LAG_INFO                      *info -
*     Lag members. Maximal number of out-going LAG members is
*     16. The number of incoming LAG members is not limited,
*     and it can be the number of Local FAP ports in each
*     device (80).
* REMARKS:
*   1. Local to system port mapping must be configured
*   before using this API (Incoming and Outgoing) - for LAG
*   pruning. 2. LAG configuration must be consistent
*   system-wide, for incoming and outgoing ports. 3. The
*   same system port can be added multiple times. This
*   affects the load-balancing, according to the number of
*   times the port appears in the LAG. 4. The _get function
*   is not symmetric to the set function: both incoming and
*   outgoing settings are returned (direction_ndx is not
*   passed). 5. For the INCOMING-LAG, the _get function
*   returns only LAG member sys-ports that are mapped to
*   local FAP ports, on the local device. 6. Setting LAG
*   with a group of system ports, will first clean-up any
*   previous configuration of the LAG. For example setting
*   LAG 1 with system members 1,2,3,4 and then setting the
*   same LAG with members 3,4,5,6 will clean up the effect
*   of the previous configuration and set up the new
*   configuration.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 lag_ndx,
    SOC_SAND_OUT SOC_PPC_LAG_INFO      *info_incoming,
    SOC_SAND_OUT SOC_PPC_LAG_INFO      *info_outgoing
  );

/*********************************************************************
* NAME:
*   arad_ports_lag_member_add
* TYPE:
*   PROC
* FUNCTION:
*   Add a system port as a member in LAG.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_PORT_DIRECTION                      direction_ndx -
*     LAG direction - incoming, outgoing or both. For
*     symmetrical LAG configuration - set direction as "both".
*     For different Incoming and Outgoing LAG configuration -
*     per-direction configuration is required.
*   SOC_SAND_IN  uint32                                 lag_ndx -
*     LAG index. Range: 0 - 255.
*   SOC_SAND_IN  ARAD_PORTS_LAG_MEMBER                    *lag_member -
*     System port to be added as a member, and the
*     member-index.
*   SOC_SAND_OUT uint8                                 *success -
*     TRUE if the sys-port was added to the LAG. FALSE if not
*     added due to reaching maximal number of LAG members in
*     the specified direction.
* REMARKS:
*   1. Replaces arad_ports_lag_sys_port_add (the later kept
*   for backward-compatability).
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_member_add(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_PORT_DIRECTION                      direction_ndx,
    SOC_SAND_IN  uint32                                 lag_ndx,
    SOC_SAND_IN  ARAD_PORTS_LAG_MEMBER                    *lag_member,
    SOC_SAND_OUT uint8                                 *success
  );

/*********************************************************************
* NAME:
*     arad_ports_lag_sys_port_remove
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Remove Physical System port to be  member of a a LAG. A LAG is defined by a group of System
*     Physical Ports that compose it. This configuration
*     affects 1. LAG resolution for queuing at the ingress. 2.
*     LAG-based pruning.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     LAG direction - incoming, outgoing or both. For
*     symmetrical LAG configuration - set direction as "both".
*     For different Incoming and Outgoing LAG configuration -
*     per-direction configuration is required.
*  SOC_SAND_IN  uint32                 lag_ndx -
*     LAG index. Range: 0 - 255. SOC_SAND_OUT uint32
*  SOC_SAND_IN  uint32                 sys_port -
*     physical system port to remove.
* REMARKS:
*   If the port cannot be removed (the lag is empty),
*   returns without any action, and without setting an error
*   indication.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_sys_port_remove(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  uint32                 lag_ndx,
    SOC_SAND_IN  ARAD_PORTS_LAG_MEMBER  *lag_member
  );

/*********************************************************************
* NAME:
*     arad_ports_lag_order_preserve_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Per-Lag information
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                  lag_ndx -
*     lag_ndx- The RLAG index of which to enable/disable
*     round robin.
*  SOC_SAND_IN  uint8                 is_order_preserving -
*     is_order_preserving If set, the LAG outlif is chosen
*     according to the Hash mechanism, this gives order
*     preserving for all packets. Otherwise LAG Round Robin
*     takes place, and the outlif are chosen sequentially.
*     Note that this case provides better load balancing but
*     does not preserve order of packets.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_order_preserve_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  lag_ndx,
    SOC_SAND_IN  uint8                 is_order_preserving
  );

/*********************************************************************
* NAME:
*     arad_ports_lag_order_preserve_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Per-Lag information
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                  lag_ndx -
*     LAG index. Range: 0 - 255.
*  SOC_SAND_OUT uint8                 *is_order_preserving -
*     is_order_preserving If set, the LAG outlif is chosen
*     according to the Hash mechanism, this gives order
*     preserving for all packets. Otherwise LAG Round Robin
*     takes place, and the outlif are chosen sequentially.
*     Note that this case provides better load balancing but
*     does not preserve order of packets.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_lag_order_preserve_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  lag_ndx,
    SOC_SAND_OUT uint8                 *is_order_preserving
  );

uint32 arad_ports_lag_lb_key_range_set(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  SOC_PPC_LAG_INFO      *info);

/*********************************************************************
* NAME:
*     arad_port_header_type_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure FAP port header parsing type. The
*     configuration can be for incoming FAP ports, outgoing
*     FAP ports or both.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     FAP Port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the fap port to configure - incoming,
*     outgoing or both.
*  SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    header_type -
*     Port header parsing type.
* REMARKS:
*     1. Not all header types are valid for all directions.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_header_type_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    header_type
  );

/*********************************************************************
* NAME:
*     arad_port_header_type_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure FAP port header parsing type. The
*     configuration can be for incoming FAP ports, outgoing
*     FAP ports or both.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     FAP Port index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
*     The direction of the fap port to configure - incoming,
*     outgoing or both.
*  SOC_SAND_OUT ARAD_PORT_HEADER_TYPE    *header_type -
*     Port header parsing type.
* REMARKS:
*     1. Not all header types are valid for all directions.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_header_type_get(
    SOC_SAND_IN  int                 unit,
     SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint32                 port_ndx,
    SOC_SAND_OUT ARAD_PORT_HEADER_TYPE    *header_type_incoming,
    SOC_SAND_OUT ARAD_PORT_HEADER_TYPE    *header_type_outgoing
  );

/*********************************************************************
* NAME:
*     arad_ports_mirror_inbound_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure inbound mirroring for the specified port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 ifp_ndx -
*     The index of the incoming FAP Port to inbound mirror.
*     Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_INBOUND_MIRROR_INFO *info -
*     Inbound mirroring configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_mirror_inbound_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 ifp_ndx,
    SOC_SAND_IN  ARAD_PORT_INBOUND_MIRROR_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_ports_mirror_inbound_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure inbound mirroring for the specified port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 ifp_ndx -
*     The index of the incoming FAP Port to inbound mirror.
*     Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORT_INBOUND_MIRROR_INFO *info -
*     Inbound mirroring configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_mirror_inbound_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 ifp_ndx,
    SOC_SAND_OUT ARAD_PORT_INBOUND_MIRROR_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_ports_mirror_outbound_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure outbound mirroring for the specified port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 ofp_ndx -
*     The index of the outgoing FAP Port to outbound mirror.
*     Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_OUTBOUND_MIRROR_INFO *info -
*     Outbound mirroring configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_mirror_outbound_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 ofp_ndx,
    SOC_SAND_IN  ARAD_PORT_OUTBOUND_MIRROR_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_ports_mirror_outbound_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Configure outbound mirroring for the specified port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 ofp_ndx -
*     The index of the outgoing FAP Port to outbound mirror.
*     Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORT_OUTBOUND_MIRROR_INFO *info -
*     Outbound mirroring configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_mirror_outbound_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 ofp_ndx,
    SOC_SAND_OUT ARAD_PORT_OUTBOUND_MIRROR_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_ports_snoop_set
* TYPE:
*   PROC
* FUNCTION:
*     Configure the snooping function.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 snoop_cmd_ndx -
*     One of the 15 snoop commands. Note that when the packet
*     is received with snoop command 0, it means that the
*     packet is not to be snooped, Therefore there is no
*     meaning to set snoop_cmd_ndx 0. Range: 1-15.
*  SOC_SAND_IN  ARAD_PORT_SNOOP_INFO     *info -
*     Snooping configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_snoop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 snoop_cmd_ndx,
    SOC_SAND_IN  ARAD_PORT_SNOOP_INFO     *info
  );

/*********************************************************************
* NAME:
*     arad_ports_snoop_get
* TYPE:
*   PROC
* FUNCTION:
*     Configure the snooping function.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 snoop_cmd_ndx -
*     One of the 15 snoop commands. Note that when the packet
*     is received with snoop command 0, it means that the
*     packet is not to be snooped, Therefore there is no
*     meaning to set snoop_cmd_ndx 0. Range: 1-15.
*  SOC_SAND_OUT ARAD_PORT_SNOOP_INFO     *info -
*     Snooping configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_snoop_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 snoop_cmd_ndx,
    SOC_SAND_OUT ARAD_PORT_SNOOP_INFO     *info
  );

/*********************************************************************
* NAME:
*     arad_ports_shaping_header_set
* TYPE:
*   PROC
* FUNCTION:
*     Set static ingress shaping configuration per FAP port.
*     A packet is ingress-shaped if the queue id in the
*     IS-ITMH is within the ingress-shaping range (set by the
*     API: 'arad_itm_ingress_shape_set'). The IS-ITMH is
*     stripped of the packet and the ITMH is expected to
*     follow.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORTS_ISP_INFO      *info -
*     Ingress Shaping header configuration.
* REMARKS:
*     If static shaping is enabled, it is illegal for the
*     incoming packet to have additional IS-ITMH header.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_shaping_header_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PORTS_ISP_INFO      *info
  );

/*********************************************************************
* NAME:
*     arad_ports_shaping_header_get
* TYPE:
*   PROC
* FUNCTION:
*     Set static ingress shaping configuration per FAP port.
*     A packet is ingress-shaped if the queue id in the
*     IS-ITMH is within the ingress-shaping range (set by the
*     API: 'arad_itm_ingress_shape_set'). The IS-ITMH is
*     stripped of the packet and the ITMH is expected to
*     follow.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORTS_ISP_INFO      *info -
*     Ingress Shaping header configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_shaping_header_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_PORTS_ISP_INFO      *info
  );

/*********************************************************************
* NAME:
*     arad_ports_forwarding_header_set
* TYPE:
*   PROC
* FUNCTION:
*     Set a raw port with the ITMH to be added to the incoming
*     packets.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORTS_ITMH          *info -
*     The information that is put in the header inc. the
*     header's extension information.
* REMARKS:
*   The get functions always returns an enable field for the
*   extension set to False. To enable the extension, the
*   user should explicitly sets this field to True.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_forwarding_header_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PORTS_ITMH          *info
  );

/*********************************************************************
* NAME:
*     arad_ports_forwarding_header_get
* TYPE:
*   PROC
* FUNCTION:
*     Set a raw port with the ITMH to be added to the incoming
*     packets.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORTS_ITMH          *info -
*     The information that is put in the header inc. the
*     header's extension information.
* REMARKS:
*     The get functions always returns an enable field for the extension
*     set to False. To enable the extension, the user should explicitly
*     sets this field to True.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_forwarding_header_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_PORTS_ITMH          *info
  );

/*********************************************************************
* NAME:
*     arad_ports_stag_set
* TYPE:
*   PROC
* FUNCTION:
*     The Statistics-Tag is a configurable collection of
*     fields and various packet attributes copied from the
*     packet header. For each field, there is a per Incoming
*     FAP Port selector, indicating whether to add the field
*     to the tag or to omit it.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORTS_STAG_FIELDS   *info -
*     Fields enablers/disablers.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_stag_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PORTS_STAG_FIELDS   *info
  );

/*********************************************************************
* NAME:
*     arad_ports_stag_get
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     The Statistics-Tag is a configurable collection of
*     fields and various packet attributes copied from the
*     packet header. For each field, there is a per Incoming
*     FAP Port selector, indicating whether to add the field
*     to the tag or to omit it.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORTS_STAG_FIELDS   *info -
*     Fields enablers/disablers.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_stag_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_PORTS_STAG_FIELDS   *info
  );

/*********************************************************************
* NAME:
*     arad_ports_ftmh_extension_set
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This function sets a system wide configuration of the
*     ftmh. The FTMH has 3 options for the FTMH-extension:
*     always allow, never allow, allow only when the packet is
*     multicast.
* INPUT:
*  SOC_SAND_IN  int                   unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORTS_FTMH_EXT_OUTLIF ext_option -
*     There 3 options for the FTMH extension.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_ftmh_extension_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_PORTS_FTMH_EXT_OUTLIF ext_option
  );

/*********************************************************************
* NAME:
*     arad_ports_ftmh_extension_get
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This function sets a system wide configuration of the
*     ftmh. The FTMH has 3 options for the FTMH-extension:
*     always allow, never allow, allow only when the packet is
*     multicast.
* INPUT:
*  SOC_SAND_IN  int                   unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_PORTS_FTMH_EXT_OUTLIF *ext_option -
*     There 3 options for the FTMH extension.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_ftmh_extension_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT ARAD_PORTS_FTMH_EXT_OUTLIF *ext_option
  );

/*********************************************************************
* NAME:
*     arad_ports_otmh_extension_get
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This function sets what extensions are to be added to
*     the OTMH per port. The OTMH has 3 optional extensions:
*     Outlif (always allow/ never allow/ allow only when the
*     packet is multicast.) Source Sys-Port and Destination
*     Sys-Port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx -
*     Local port index. Port Index. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORTS_OTMH_EXTENSIONS_EN *info -
*     There 3 options for the OTMH-outlif extension, src-port
*     & dest-port extensions Enable/Disable.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_ports_otmh_extension_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_PORTS_OTMH_EXTENSIONS_EN *info
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_type_set
* TYPE:
*   PROC
* FUNCTION:
*     Per discount type, set the available egress credit
*     compensation value to adjust the credit rate for the
*     various headers: PP (if present), FTMH, DRAM-CRC,
*     Ethernet-IPG, NIF-CRC.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx -
*     The port header type for which the credit discount is
*     configured (TM/ETH/RAW).
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx -
*     The preset (A/B) that is configured.
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info -
*     The discount values (signed, can be negative) for the
*     specified discount type and port header type.
* REMARKS:
*     1. The configuration is per port header type. In
*     practice, each port will use the configuration
*     accordingly to its header type. For example, if all
*     ports are TM-ports, the configuration for ETH ports is
*     irrelevant. 2. Credit discount should also be configured
*     in the ingress, using arad_itm_cr_discount_set API. 3.
*     This API only configures the available presets. The
*     specific preset that is used, per port, is configured
*     using the arad_port_egr_hdr_credit_discount_select_set
*     API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_type_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_type_get
* TYPE:
*   PROC
* FUNCTION:
*     Per discount type, set the available egress credit
*     compensation value to adjust the credit rate for the
*     various headers: PP (if present), FTMH, DRAM-CRC,
*     Ethernet-IPG, NIF-CRC.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx -
*     The port header type for which the credit discount is
*     configured (TM/ETH/RAW).
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx -
*     The preset (A/B) that is configured.
*  SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info -
*     The discount values (signed, can be negative) for the
*     specified discount type and port header type.
* REMARKS:
*     1. The configuration is per port header type. In
*     practice, each port will use the configuration
*     accordingly to its header type. For example, if all
*     ports are TM-ports, the configuration for ETH ports is
*     irrelevant. 2. Credit discount should also be configured
*     in the ingress, using arad_itm_cr_discount_set API. 3.
*     This API only configures the available presets. The
*     specific preset that is used, per port, is configured
*     using the arad_port_egr_hdr_credit_discount_select_set
*     API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_type_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_HEADER_TYPE    port_hdr_type_ndx,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type_ndx,
    SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_select_set
* TYPE:
*   PROC
* FUNCTION:
*     Select from the available egress credit compensation
*     values to adjust the credit rate for the various
*     headers: PP (if present), FTMH, DRAM-CRC, Ethernet-IPG,
*     NIF-CRC. This API selects the discount type. The values
*     per port header type and discount type are configured
*     using arad_port_egr_hdr_credit_discount_type_set API.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     The index of the port to configure. Range: 0 - 79.
*  SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type -
*     The preset (A/B) that is selected.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_select_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32              tm_port,
    SOC_SAND_IN  ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE cr_discnt_type
  );

/*********************************************************************
* NAME:
*     arad_port_egr_hdr_credit_discount_select_get
* TYPE:
*   PROC
* FUNCTION:
*     Select from the available egress credit compensation
*     values to adjust the credit rate for the various
*     headers: PP (if present), FTMH, DRAM-CRC, Ethernet-IPG,
*     NIF-CRC. This API selects the discount type. The values
*     per port header type and discount type are configured
*     using arad_port_egr_hdr_credit_discount_type_set API.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 port_ndx -
*     The index of the port to configure. Range: 0 - 79.
*  SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE *cr_discnt_type -
*     The preset (A/B) that is selected.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_port_egr_hdr_credit_discount_select_get(
    SOC_SAND_IN  int                    unit, 
    SOC_SAND_IN  int                    core, 
    SOC_SAND_IN  uint32                 tm_port, 
    SOC_SAND_OUT ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE *cr_discnt_type
  );

uint32 arad_port_stacking_info_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  uint32                              local_port_ndx,
    SOC_SAND_IN  uint32                              is_stacking,
    SOC_SAND_IN  uint32                              peer_tmd);

uint32 arad_port_stacking_info_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  uint32                              local_port_ndx,
    SOC_SAND_OUT  uint32                          *is_stacking,
    SOC_SAND_OUT  uint32                          *peer_tmd);

uint32 arad_port_stacking_route_history_bitmap_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  uint32                           tm_port,
    SOC_SAND_IN  SOC_TMC_STACK_EGR_PROG_TM_PORT_PROFILE_STACK tm_port_profile_stack,
    SOC_SAND_IN  uint32                              bitmap);

/*********************************************************************
* NAME:
 *   arad_port_pp_port_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the Port profile for ports of type TM and Raw.
 * INPUT:
 *   SOC_SAND_IN  int                       unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                       pp_port_ndx -
 *     TM Port Profile Index. Range: 0 - 63.
 *   SOC_SAND_IN  ARAD_PORT_PP_PORT_INFO        *info -
 *     Attributes of the TM Port Profile. Ignored for Raw
 *     Ports.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE         *success -
 *     If True, then the TM Port Profile is added. Otherwise,
 *     not enough resources may be available.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_pp_port_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                    core,
    SOC_SAND_IN  uint32                 pp_port_ndx,
    SOC_SAND_IN  ARAD_PORT_PP_PORT_INFO         *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE         *success
  );

/*********************************************************************
* NAME:
 *   arad_port_pp_port_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the Port profile settings.
 * INPUT:
 *   SOC_SAND_IN  int                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    pp_port_ndx -
 *     TM Port Profile Index. Range: 0 - 63.
 *   SOC_SAND_OUT ARAD_PORT_PP_PORT_INFO     *info -
 *     Attributes of the TM Port Profile
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_pp_port_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                     core_id,
    SOC_SAND_IN  uint32                    pp_port_ndx,
    SOC_SAND_OUT ARAD_PORT_PP_PORT_INFO     *info
  );

/*********************************************************************
* NAME:
 *   arad_port_to_pp_port_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map the Port to its Port profile for ports of type TM
 *   and Raw.
 * INPUT:
 *   SOC_SAND_IN  int                       unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                       port_ndx -
 *     TM Port Index. Range: 0 - 79.
 *   SOC_SAND_IN  uint32    pp_port -
 *     Mapping of the TM Port to its Profile
 * REMARKS:
 *   TM and Raw Ports can be mapped only to existing TM Port
 *   Profile. To add a new TM Port Profile, use the
 *   arad_port_pp_port_set API.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_to_pp_port_map_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  soc_port_t             port,
    SOC_SAND_IN  ARAD_PORT_DIRECTION    direction_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_port_to_pp_port_map_set" API.
 *     Refer to "arad_port_to_pp_port_map_set" API for
 *     details.
*********************************************************************/
uint32
  arad_port_to_pp_port_map_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    port_ndx,
    SOC_SAND_OUT uint32                    *pp_port_in,
    SOC_SAND_OUT uint32                    *pp_port_out
  );
  
/*
 * direct lb_key min/max set/get
*/

uint32 
  arad_port_direct_lb_key_min_set(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32  local_port,
    SOC_SAND_IN uint32 min_lb_key
   );

uint32 
  arad_port_direct_lb_key_max_set(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id, 
    SOC_SAND_IN uint32  local_port,
    SOC_SAND_IN uint32 max_lb_key
   );

uint32 
  arad_port_direct_lb_key_min_get(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id, 
    SOC_SAND_IN uint32  local_port,
    uint32* min_lb_key
    );

uint32 
  arad_port_direct_lb_key_max_get(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id, 
    SOC_SAND_IN uint32  local_port,
    uint32* max_lb_key
    );

#ifdef BCM_88660_A0
/*********************************************************************
* NAME:
*     arad_port_synchronize_lb_key_tables_at_egress
* TYPE:
*   PROC
* FUNCTION:
*     Synchronized lb-key range tables according to the used table.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
   arad_port_synchronize_lb_key_tables_at_egress(
   SOC_SAND_IN int unit
    );

/*********************************************************************
* NAME:
*     arad_port_switch_lb_key_tables
* TYPE:
*   PROC
* FUNCTION:
*     use the shadow table instead of the primary table.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_port_switch_lb_key_tables(
     SOC_SAND_IN int unit
    );

#endif /* BCM_88660_A0 */

uint32
  arad_port_encap_config_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   core_id,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_L2_ENCAP_INFO       *info
  );
uint32
  arad_port_encap_config_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   core_id,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_L2_ENCAP_INFO       *info
  );

/**********************************************************************
 * Function:
 *      arad_ports_swap_set
 * Purpose:
 *      Set a port with per port swap configuration.
 * Parameters:
 *      unit     -   (IN)    unit number.
 *      port_ndx      -   (IN)    Fap local port index. Range: 0 - 255.
 *      info          -   (IN)    Swap data per port.
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/
uint32
  arad_ports_swap_set(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID           port_ndx,
    SOC_SAND_IN  ARAD_PORTS_SWAP_INFO       *info
  );

/**********************************************************************
 * Function:
 *      arad_ports_swap_get
 * Purpose:
 *      Get per port swap configuration.
 * Parameters:
 *      unit     -   (IN)    unit number.
 *      port_ndx      -   (IN)    Fap local port index. Range: 0 - 255.
 *      info          -   (OUT)   Swap data per port.
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/
uint32
  arad_ports_swap_get(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID           port_ndx,
    SOC_SAND_OUT ARAD_PORTS_SWAP_INFO       *info
  );

/**********************************************************************
 * Function:
 *      arad_ports_pon_tunnel_info_set
 * Purpose:
 *      Set PON port and tunnel_ID to logical PP port mapping.
 * Parameters:
 *      unit     -   (IN)    unit number.
 *      port_ndx      -   (IN)    Fap PON port index. Range: 0 - 7.
 *      tunnel        -   (IN)    PON port channel ID. Range: 0 - 4095
 *      info          -   (IN)    Logical PP port.
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

uint32
  arad_ports_pon_tunnel_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PON_TUNNEL_ID       tunnel,
    SOC_SAND_IN  ARAD_PORTS_PON_TUNNEL_INFO *info
  );

/**********************************************************************
 * Function:
 *      arad_ports_pon_tunnel_info_get
 * Purpose:
 *      Get PON port and tunnel_ID to logical PP port mapping.
 * Parameters:
 *      unit     -   (IN)    unit number.
 *      port_ndx      -   (IN)    Fap PON port index. Range: 0 - 7.
 *      tunnel        -   (IN)    PON port channel ID. Range: 0 - 4095
 *      info          -   (OUT)   Logical PP port.
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

uint32
  arad_ports_pon_tunnel_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PON_TUNNEL_ID       tunnel,
    SOC_SAND_OUT ARAD_PORTS_PON_TUNNEL_INFO *info
  );

/**********************************************************************
 * Function:
 *      arad_ports_programs_info_set
 * Purpose:
 *      Set TM port properties for programmable blocks (VT, TT, FLP).
 * Parameters:
 *      unit     -   (IN)    unit number.
 *      port_ndx      -   (IN)    Fap port index. Range: 0 - 255.
 *      info          -   (IN)    Programs port.
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

uint32
  arad_ports_programs_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_IN  ARAD_PORTS_PROGRAMS_INFO *info
  );

/**********************************************************************
 * Function:
 *      arad_ports_programs_info_get
 * Purpose:
 *      Get TM port properties for programmable blocks (VT, TT, FLP).
 * Parameters:
 *      unit     -   (IN)    unit number.
 *      port_ndx      -   (IN)    Fap port index. Range: 0 - 255.
 *      info          -   (OUT)   Logical PP port.
 * Returns:
 *      BCM_E_NONE     No Error  
 *      BCM_E_UNAVAIL  Feature unavailable  
 *      BCM_E_PORT     Invalid Port number specified  
 *      BCM_E_XXX      Error occurred  
 **********************************************************************/

uint32
  arad_ports_programs_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         port_ndx,
    SOC_SAND_OUT ARAD_PORTS_PROGRAMS_INFO *info
  );

void
  ARAD_PORT_PP_PORT_INFO_clear(
    SOC_SAND_OUT ARAD_PORT_PP_PORT_INFO *info
  );

void
  ARAD_PORT_COUNTER_INFO_clear(
    SOC_SAND_OUT ARAD_PORT_COUNTER_INFO *info
  );


void
  arad_ARAD_PORT2IF_MAPPING_INFO_clear(
    SOC_SAND_OUT ARAD_PORT2IF_MAPPING_INFO *info
  );
void
  arad_ARAD_PORTS_LAG_MEMBER_clear(
    SOC_SAND_OUT ARAD_PORTS_LAG_MEMBER *info
  );



/*********************************************************************
* NAME:
*     arad_ports_application_mapping_info_set
* TYPE:
*   PROC
* FUNCTION:
*     Allows different mappings for (packet, TM-PTC-port)
*     to PP port and might also apply opposite.
*     For XGS MAC extender, it allows packet mapping
*     (HG.Port,HG.Modid) to PP port.
*     Might be used in the future for other applications that have
*     not typical Port mappings.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  port -
*       Port #
*  SOC_SAND_IN  uint32                  info -
*       Application mapping information.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 
  arad_ports_application_mapping_info_set (
    SOC_SAND_IN int unit, 
    SOC_SAND_IN ARAD_FAP_PORT_ID port_ndx, 
    SOC_SAND_IN ARAD_PORTS_APPLICATION_MAPPING_INFO *info    
    );

/*********************************************************************
* NAME:
*     arad_ports_application_mapping_info_get
* TYPE:
*   PROC
* FUNCTION:
*     Reterive Configuration of the ports mappings according
*     to application.
* INPUT:
*  SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  port -
*       Port #
*  SOC_SAND_IN  uint32                  info -
*       Application mapping information.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 
  arad_ports_application_mapping_info_get (
    SOC_SAND_IN int unit, 
    SOC_SAND_IN ARAD_FAP_PORT_ID port_ndx, 
    SOC_SAND_INOUT ARAD_PORTS_APPLICATION_MAPPING_INFO *info    
    );

void
  arad_ARAD_PORTS_ITMH_clear(
    SOC_SAND_OUT ARAD_PORTS_ITMH *info
  );
void
  arad_ARAD_PORT_LAG_SYS_PORT_INFO_clear(
    SOC_SAND_OUT ARAD_PORT_LAG_SYS_PORT_INFO *info
  );

void
  arad_ARAD_PORTS_OTMH_EXTENSIONS_EN_clear(
    SOC_SAND_OUT ARAD_PORTS_OTMH_EXTENSIONS_EN *info
  );

void
  arad_ARAD_PORTS_SWAP_INFO_clear(
    SOC_SAND_OUT ARAD_PORTS_SWAP_INFO *info
  );

void
  arad_ARAD_PORTS_PON_TUNNEL_INFO_clear(
    SOC_SAND_OUT ARAD_PORTS_PON_TUNNEL_INFO *info
  );

void
  arad_ARAD_PORTS_PROGRAMS_INFO_clear(
    SOC_SAND_OUT ARAD_PORTS_PROGRAMS_INFO *info
  );

#if ARAD_DEBUG_IS_LVL1
void
  ARAD_PORT_PP_PORT_INFO_print(
    SOC_SAND_IN  ARAD_PORT_PP_PORT_INFO *info
  );
const char*
  arad_ARAD_PORT_HEADER_TYPE_to_string(
    SOC_SAND_IN ARAD_PORT_HEADER_TYPE enum_val
  );

const char*
  arad_ARAD_PORTS_FTMH_EXT_OUTLIF_to_string(
    SOC_SAND_IN ARAD_PORTS_FTMH_EXT_OUTLIF enum_val
  );
void
  arad_ARAD_PORT2IF_MAPPING_INFO_print(
    SOC_SAND_IN ARAD_PORT2IF_MAPPING_INFO *info
  );

void
  arad_ARAD_PORTS_SWAP_INFO_print(
    SOC_SAND_IN ARAD_PORTS_SWAP_INFO *info
  );


#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_PORTS_INCLUDED__*/
#endif


