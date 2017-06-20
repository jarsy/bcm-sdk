/* $Id: ppc_api_llp_trap.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_llp_trap.h
*
* MODULE PREFIX:  soc_ppc_llp
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

#ifndef __SOC_PPC_API_LLP_TRAP_INCLUDED__
/* { */
#define __SOC_PPC_API_LLP_TRAP_INCLUDED__

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

/*     Number of my-IP addresses, in usage to trap ARP
 *     requests.                                               */
#define  SOC_PPC_TRAP_NOF_MY_IPS (2)

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
   *  Disable all traps on the port
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_NONE = 0,
  /*
   *  Enable ARP request traps on the port.
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_ARP = 1,
  /*
   *  Enable IGMP traps on the port (including Query, Leave)
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_IGMP = 2,
  /*
   *  Enable MLD traps on the port (including Query, Done,
   *  Others)
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_MLD = 4,
  /*
   *  Enable DHCP traps on the port (including server/client)
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_DHCP = 8,
  /*
   *  Enable programmable trap '0' on the port
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_PROG_TRAP_0 = 0x10,
  /*
   *  Enable programmable trap '1' on the port
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_PROG_TRAP_1 = 0x20,
  /*
   *  Enable programmable trap '2' on the port
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_PROG_TRAP_2 = 0x40,
  /*
   *  Enable programmable trap '3' on the port
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_PROG_TRAP_3 = 0x80,
  /* 
   *  Enable ingress vlan membership check on port 
   */ 
  SOC_PPC_LLP_TRAP_PORT_ENABLE_ING_VLAN_MEMBERSHIP = 0x100, 
  /* 
   * Enable ingress same interface check on port 
   * ARAD only
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_SAME_INTERFACE = 0x200,
  /* 
   * enable ICMP redirect on incommong port
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_ICMP_REDIRECT = 0x400,
  /*
   *  Enable all (supported) traps on the port
   */
  SOC_PPC_LLP_TRAP_PORT_ENABLE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_LLP_TRAP_PORT_ENABLE
   */
  SOC_PPC_NOF_LLP_TRAP_PORT_ENABLES = 11
}SOC_PPC_LLP_TRAP_PORT_ENABLE;

typedef enum
{
  /*
   *  None of the optional conditions of the programmable trap
   *  is tested.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT_NONE = 0,
  /*
   *  Compare packet MAC DA with programmable trap DA. The
   *  configured DA may be set to compare part of the value -
   *  see SOC_PPC_LLP_TRAP_PROG_TRAP_L2_INFO
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT_DA = 1,
  /*
   *  Compare packet Ethernet type with programmable trap
   *  Ethernet type.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT_ETHER_TYPE = 2,
  /*
   *  Compare first nibble after link layer header with
   *  programmable trap Sub-type.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT_SUB_TYPE = 4,
  /*
   *  If packet is IP over Ethernet, compare protocol from IP
   *  header with programmable trap IP-protocol.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT_IP_PRTCL = 8,
  /*
   *  If packet is IP over Ethernet, then compare L4 ports of
   *  the packet with the programmable trap ranges. Ranges
   *  definition may have masking over the values - see
   *  SOC_PPC_LLP_TRAP_PROG_TRAP_L4_INFO.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT_L4_PORTS = 16,
  /*
   *  Number of types in SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT
   */
  SOC_PPC_NOF_LLP_TRAP_PROG_TRAP_COND_SELECTS = 6
}SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Reserved MC Trap profile Range: 0 - 3.
   */
  uint32 reserved_mc_profile;
  /*
   *  Bitmap Enable/disable for Traps. See
   *  SOC_PPC_LLP_TRAP_PORT_ENABLE to what is the usag of each
   *  bit. Example: to enable IGMP and ARP trap, set .
   *  trap_enable_mask = SOC_PPC_LLP_TRAP_PORT_ENABLE_ARP |
   *  SOC_PPC_LLP_TRAP_PORT_ENABLE_IGMP;Relevant only For Soc_petra-B,
   *  in T20E has to be SOC_PPC_LLP_TRAP_PORT_ENABLE_ALL
   */
  uint32 trap_enable_mask;

} SOC_PPC_LLP_TRAP_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  IP addresses to identify ARP requests to this device.
   *  i.e. all ARP requests with TPA equal to one of these IP
   *  addresses, will be considered as request to this device
   *  and will be trapped with the trap-code =
   *  SOC_PPC_TRAP_CODE_MY_ARP. ARP-Requests that do not match any
   *  of these IPs will be trapped with trap-code =
   *  SOC_PPC_TRAP_CODE_ARP.
   */
  uint32 my_ips[SOC_PPC_TRAP_NOF_MY_IPS];
  /*
   *  If set to FALSE then only packets with DA equal to
   *  ff-ff-ff-ff-ff-ff will be trapped (match ARP requests
   *  messages). If set to TRUE then packet will be trapped
   *  regardless the DA of the packet, may be useful to trap
   *  both ARP requests and responses. Note: Unicast ARP
   *  request/replies may be trapped (with trap-code =
   *  SOC_PPC_TRAP_CODE_MY_MAC_AND_ARP) if packet has my-MAC and
   *  next-protocol is ARP.
   */
  uint8 ignore_da;

} SOC_PPC_LLP_TRAP_ARP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Reserved MC profile. The user may set a profile per
   *  port. See soc_ppd_llp_trap_port_info_set() Range: 0 - 3.
   */
  uint32 reserved_mc_profile;
  /*
   *  The 6 lsb of the Destination MAC address (DA[5:0]). The
   *  msb bits DA[47:6] are 01-80-c2-00-00-XX where XX =
   *  8'b00xx_xxxx)Range: 0 - 63.
   */
  uint32 da_mac_address_lsb;

} SOC_PPC_LLP_TRAP_RESERVED_MC_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Destination MAC address to compare
   */
  SOC_SAND_PP_MAC_ADDRESS dest_mac;
  /*
   *  Number of (MS) bits to consider from the above MAC
   *  address. The 'dest_mac_nof_bits ' (MS) bits of Packet DA
   *  and dest_mac are compared. Compares successes if packet
   *  DA MAC equal to dest_mac, considering only the
   *  'dest_mac_nof_bits' MS-bitsRange: 0 - 48.
   */
  uint8 dest_mac_nof_bits;
  /*
   *  Ethernet Type to compare with the Ethernet of the
   *  packets. Up to 10 different Ethernet Types, including
   *  802.1x(0x88ea), IPv4 (0x0800), IPv6 (0x86DD), ARP
   *  (0x0806), CFM (0x8902), and TRILL MPLS(0x8847). Use
   *  SOC_PPC_L2_NEXT_PRTCL_TYPE enumeration to refer to these
   *  constant values
   */
  SOC_SAND_PP_ETHER_TYPE ether_type;
  /*
   *  First nibble after link layer header. Range: 0 - 15.
   */
  uint8 sub_type;
  /*
   *  Bitmap mask over the sub_type. 0 to mask corresponding
   *  bit, and 1 to consider it. Compare successes if packet's
   *  subtype value equal to sub_type ignoring the bits masked
   *  (set to zero) by sub_type_bitmap
   */
  uint8 sub_type_bitmap;

} SOC_PPC_LLP_TRAP_PROG_TRAP_L2_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The IP protocol in the IP header Examples: TCP, UDP,
   *  ICMP etc... Compare successes if packet IP-protocol (in IP
   *  header) equal to this value. See SOC_PPC_L3_NEXT_PRTCL_TYPE
   *  which introduce subset of the possible next protocols.
   */
  uint8 ip_protocol;

} SOC_PPC_LLP_TRAP_PROG_TRAP_L3_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The layer 4 source port
   */
  uint16 src_port;
  /*
   *  Bitmap mask over the source port. 0 to mask
   *  corresponding bit, and 1 to consider it. Compare
   *  successes if packet l4 source port value equal to
   *  src_port ignoring the bits masked by src_port_bitmap.
   */
  uint16 src_port_bitmap;
  /*
   *  The layer 4 destination port
   */
  uint16 dest_port;
  /*
   *  Bitmap mask over the destination port. 0 to mask
   *  corresponding bit, and 1 to consider it. Compare
   *  successes if packet l4 destination port value equal to
   *  dest_port ignoring the bits masked by dest_port_bitmap.
   */
  uint16 dest_port_bitmap;

} SOC_PPC_LLP_TRAP_PROG_TRAP_L4_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  L2 information to test.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_L2_INFO l2_info;
  /*
   *  L3 information to test.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_L3_INFO l3_info;
  /*
   *  L4 information to test.
   */
  SOC_PPC_LLP_TRAP_PROG_TRAP_L4_INFO l4_info;
  /*
   *  Specifies which conditions are to be tested. See
   *  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT. For example, user
   *  may select to check packet DA/Ethernet-type/
   *  IP-protocol/Sub-Type/L4 ports etc... Programmable traps 2
   *  and 3 may consider only DA and Ethernet-Type. Error will
   *  be returned otherwise.
   */
  uint32 enable_bitmap;
  /*
   *  Specifies conditions that will be met only if the
   *  compare failed. See SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT.
   *  For fields that are not selected here, the condition
   *  will be met if the compare succeeds. For example, user
   *  may choose to trap all packets with DA not equal to some
   *  value.
   */
  uint32 inverse_bitmap;

} SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER;


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
  SOC_PPC_LLP_TRAP_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PORT_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_ARP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_ARP_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY_clear(
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_RESERVED_MC_KEY *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_L2_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PROG_TRAP_L2_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_L3_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PROG_TRAP_L3_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_L4_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PROG_TRAP_L4_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER_clear(
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LLP_TRAP_PORT_ENABLE_to_string(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PORT_ENABLE enum_val
  );

const char*
  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT_to_string(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_COND_SELECT enum_val
  );

void
  SOC_PPC_LLP_TRAP_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PORT_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_ARP_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_ARP_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY_print(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_L2_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_L2_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_L3_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_L3_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_L4_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_L4_INFO *info
  );

void
  SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER_print(
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LLP_TRAP_INCLUDED__*/
#endif

