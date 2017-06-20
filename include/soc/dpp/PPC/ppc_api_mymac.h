/* $Id: ppc_api_mymac.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_mymac.h
*
* MODULE PREFIX:  soc_ppc_mymac
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

#ifndef __SOC_PPC_API_MYMAC_INCLUDED__
/* { */
#define __SOC_PPC_API_MYMAC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/dpp_config_defs.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define SOC_PPC_MYMAC_VIRTUAL_NICK_NAMES_SIZE 3
/* Arad support up to 4 my_nickname (my nickname + 3 virtual my nickname) */
#define SOC_PPC_MYMAC_NOF_MY_NICKNAMES        (SOC_PPC_MYMAC_VIRTUAL_NICK_NAMES_SIZE + 1)

#define SOC_PPC_VRRP_CAM_MAX_NOF_ENTRIES(_unit)         (SOC_DPP_DEFS_GET(unit, vrrp_mymac_cam_size))

#define SOC_PPC_VRRP_MAX_VSI(_unit)                     (SOC_DPP_DEFS_GET(unit, vrrp_mymac_map_size))

#define SOC_PPC_L3_NOF_PROTOCOL_GROUPS(unit)            (SOC_DPP_DEFS_GET(unit, vrrp_nof_protocol_groups))

#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_NONE             (0)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_IPV4             (1 << soc_ppc_parsed_l2_next_protcol_ipv4)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_IPV6             (1 << soc_ppc_parsed_l2_next_protcol_ipv6)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_ARP              (1 << soc_ppc_parsed_l2_next_protcol_arp)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_MPLS             (1 << soc_ppc_parsed_l2_next_protcol_mpls)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_MIM              (1 << soc_ppc_parsed_l2_next_protcol_mim)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_TRILL            (1 << soc_ppc_parsed_l2_next_protcol_trill)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_FCOE             (1 << soc_ppc_parsed_l2_next_protcol_fcoe)
#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_INVALID          ((uint32) -1)

#define SOC_PPC_L3_VRRP_PROTOCOL_GROUP_ALL_VALID\
        (SOC_PPC_L3_VRRP_PROTOCOL_GROUP_NONE    \
         |SOC_PPC_L3_VRRP_PROTOCOL_GROUP_IPV4   \
         |SOC_PPC_L3_VRRP_PROTOCOL_GROUP_IPV6   \
         |SOC_PPC_L3_VRRP_PROTOCOL_GROUP_ARP    \
         |SOC_PPC_L3_VRRP_PROTOCOL_GROUP_MPLS   \
         |SOC_PPC_L3_VRRP_PROTOCOL_GROUP_MIM    \
         |SOC_PPC_L3_VRRP_PROTOCOL_GROUP_TRILL  \
         |SOC_PPC_L3_VRRP_PROTOCOL_GROUP_FCOE)    


/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define SOC_PPC_PARSED_L2_NEXT_PROTOCOL_TO_L3_PROTOCOL_FLAG(_l2_parsed)  \
        (1 << (_l2_parsed))


/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  The LSB of the MAC addresses is set per VSI. In Soc_petra: Up to 2 MAC
   *  addresses may be set per VSI. In ARAD: bitmap of 8 MAC addresses.
   */
  SOC_PPC_MYMAC_VRRP_MODE_ALL_VSI_BASED = 0,
  /*
   *  The LSB of the MAC addresses is set for each VSI-only
   *  for VSIs [0,256]. In Soc_petra: Up to 32 MAC addresses may be set for
   *  each such VSI. In ARAD: Up to 16 or 8 (if IPV6 is set) MAC addresses.
   */
  SOC_PPC_MYMAC_VRRP_MODE_256_VSI_BASED = 1,
  /*
   *  The LSB of the MAC addresses is set for each VSI-only
   *  for VSIs [0,512]. Up to 8 or 4 (if IPV6 is set) MAC addresses.
   *  ARAD only.
   */
  SOC_PPC_MYMAC_VRRP_MODE_512_VSI_BASED = 2,
  /*
   *  The LSB of the MAC addresses is set for each VSI-only
   *  for VSIs [0,1K]. Up to 4 or 2 (if IPV6 is set) MAC addresses.
   *  ARAD only.
   */
  SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_BASED = 3,
  /*
   *  The LSB of the MAC addresses is set for each VSI-only
   *  for VSIs [0,2K]. Up to 2 MAC addresses.
   *  ARAD only.
   */
  SOC_PPC_MYMAC_VRRP_MODE_2K_VSI_BASED = 4,
  /*
   *  Number of types in SOC_PPC_MYMAC_VRRP_MODE
   */
  SOC_PPC_NOF_MYMAC_VRRP_MODES = 5
}SOC_PPC_MYMAC_VRRP_MODE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable MyMAC according to Virtual Router Redundancy
   *  Protocol. Choose one of My MAC VRRP support mythologies:
   *  per Port / VSI / 256 VSIs
   */
  uint8 enable;
  /*
   *  May be: - Port based - VSI based- 256 VSIs based
   *  Relevant only if enable is TRUE.
   */
  SOC_PPC_MYMAC_VRRP_MODE mode;
  /*
   *  Set the MSB of the VRID MAC address for IPV4 of the device.
   *  Used for ingress termination. Note that the various
   *  modes use a different number of bits from this address.
   */
  SOC_SAND_PP_MAC_ADDRESS vrid_my_mac_msb;
  /*
   *  Set the MSB of the VRID MAC address for IPV6 of the device.
   *  Used for ingress termination                              .
   *  ARAD only.                                                          .
   */
  SOC_SAND_PP_MAC_ADDRESS ipv6_vrid_my_mac_msb;
  /*
   *  Determines whether MYMAC_VRRP_MODE supports IPV6 or not
   *  ARAD only.
   */
  uint8 ipv6_enable;

} SOC_PPC_MYMAC_VRRP_INFO;


/* Used to configure the vrrp cam table. Only available in ARADPLUS and above. */
typedef struct 
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Mac address to be used as mymac.
   */
  SOC_SAND_PP_MAC_ADDRESS mac_addr;
  /*
   * Mask for the DA, used only in Jericho.
   */
  SOC_SAND_PP_MAC_ADDRESS mac_mask;
  /*
   * Protocol group of the tcam entry. 
   */
  uint32 protocol_group;
  /*
   * Protocol group mask of the tcam entry. 
   */
  uint32 protocol_group_mask;
  /*
   *  Determines whether this entry is ipv4 only.
   */
  uint8 is_ipv4_entry;
  /* 
   *  Index to global CAM table.
   */
  uint8 vrrp_cam_index;
  /*
   * flags
   */
  uint32 flags;

} SOC_PPC_VRRP_CAM_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The Device TRILL nickname. Used for terminate or
   *  encapsulate TRILL packets. (at the ingress/egress
   *  devices)
   */
  uint32 my_nick_name;
  /*  
   * virtual nicknames.
   */
  uint32 virtual_nick_names[SOC_PPC_MYMAC_VIRTUAL_NICK_NAMES_SIZE];

  /*
   *  To terminate/encapsulate Multicast TRILL packets.
   */
  SOC_SAND_PP_MAC_ADDRESS all_rbridges_mac;
  /*
   *  End System Address Distribution Instance. Packets with
   *  this MAC will be trapped.
   */
  SOC_SAND_PP_MAC_ADDRESS all_esadi_rbridges;

  /* 
   * Indicate if skip ethernet for my nickname. 
   */
  uint32 skip_ethernet_my_nickname; 

  /*
   * Indicate if skip ethernet for each of the virtual nicknames* 
   */
  uint32 skip_ethernet_virtual_nick_names[SOC_PPC_MYMAC_VIRTUAL_NICK_NAMES_SIZE]; 

} SOC_PPC_MYMAC_TRILL_INFO;

typedef enum {
    soc_ppc_parsed_l2_next_protcol_none            =   0 ,
    soc_ppc_parsed_l2_next_protcol_user_def_first  =   1 ,
    soc_ppc_parsed_l2_next_protcol_user_def_last   =   7 ,
    soc_ppc_parsed_l2_next_protcol_trill           =   8 ,
    soc_ppc_parsed_l2_next_protcol_mim             =   9 ,
    soc_ppc_parsed_l2_next_protcol_arp             =   10,
    soc_ppc_parsed_l2_next_protcol_cfm             =   11,
    soc_ppc_parsed_l2_next_protcol_fcoe            =   12,
    soc_ppc_parsed_l2_next_protcol_ipv4            =   13,
    soc_ppc_parsed_l2_next_protcol_ipv6            =   14,
    soc_ppc_parsed_l2_next_protcol_mpls            =   15,
    soc_ppc_parsed_l2_next_protcol_count           =   16
} soc_ppc_parsed_l2_next_prtocol_t;

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
  SOC_PPC_MYMAC_VRRP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MYMAC_VRRP_INFO *info
  );

void
  SOC_PPC_MYMAC_TRILL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MYMAC_TRILL_INFO *info
  );

#ifdef BCM_88660_A0

void 
  SOC_PPC_VRRP_CAM_INFO_clear(
   SOC_SAND_OUT SOC_PPC_VRRP_CAM_INFO *info
  );

#endif /*BCM_88660_A0*/

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_MYMAC_VRRP_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_MODE enum_val
  );

void
  SOC_PPC_MYMAC_VRRP_INFO_print(
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO *info
  );

void
  SOC_PPC_MYMAC_TRILL_INFO_print(
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_MYMAC_INCLUDED__*/
#endif

