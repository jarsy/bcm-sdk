/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_SAND_PP_GENERAL_INCLUDED__
/* { */
#define __DNX_SAND_PP_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/SAND/Utils/sand_pp_mac.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */



#define DNX_SAND_PP_IPV4_SUBNET_PREF_MAX_LEN 32
#define DNX_SAND_PP_IPV4_MAX_IP_STRING 18

#define DNX_SAND_PP_IPV4_MC_ADDR_PREFIX 0xE0000000




#define DNX_SAND_PP_PORT_L2_TYPE_CEP_VAL 4
#define DNX_SAND_PP_PORT_L2_TYPE_PCEP_VAL 5



/* $Id$
 * Set port (CEP/PNP/CNP)
 */
#define  DNX_SAND_PP_LOCAL_PORT_CEP_SET(port_id) \
          (DNX_SAND_SET_BITS_RANGE(DNX_SAND_PP_PORT_L2_TYPE_CEP_VAL,31,26) | DNX_SAND_SET_BITS_RANGE(port_id,6,0) )


/************************************************************************
 *   31-26  | 25-22     |      21-10        |  9 - 7     |    6-0       *
 ************************************************************************
 *  type    | reserved  | s-vlan (reserved*)|  reserved  | port id      *
 ************************************************************************/

  /*
   * s-vlan if relevant only for internal port types (PEP/CNP)
   */

 /************************************************************************
  * Type:
  *   00000  - VBP.
  *   00000  - CEP.
  *   00101  - PEP.
  *   00110  - CNP.
  *   00111  - PNP.
  ***********************************************************************/

/*
 * Set internal port (PEP/CNP) given CEP port and s-vlan-id
 */

#define  DNX_SAND_PP_LOCAL_PORT_PEP_SET(port_id,s_vid) \
          (DNX_SAND_SET_BITS_RANGE(DNX_SAND_PP_PORT_L2_TYPE_PCEP_VAL,31,26) | DNX_SAND_SET_BITS_RANGE(s_vid,21,10) | DNX_SAND_SET_BITS_RANGE(port_id,6,0) )


/*
 * number of longs in IPV4/6 address
 */
#define DNX_SAND_PP_IPV4_ADDRESS_NOF_UINT32S 1
#define DNX_SAND_PP_IPV6_ADDRESS_NOF_UINT32S 4

/*
 * number of longs in IPV4/6 address
 */
#define  DNX_SAND_PP_IPV4_ADDRESS_NOF_BITS (DNX_SAND_PP_IPV4_ADDRESS_NOF_UINT32S * DNX_SAND_NOF_BITS_IN_UINT32)
#define  DNX_SAND_PP_IPV6_ADDRESS_NOF_BITS (DNX_SAND_PP_IPV6_ADDRESS_NOF_UINT32S * DNX_SAND_NOF_BITS_IN_UINT32)

/*
 * number of characters in IPV4/6 address string.
 */
#define  DNX_SAND_PP_IPV4_ADDRESS_STRING_LEN 8
#define  DNX_SAND_PP_IPV6_ADDRESS_STRING_LEN 32
/*
 * number of bytes in IPV4/6 address
 */


#define DNX_SAND_PP_NOF_BITS_IN_EXP                                (8)

#define DNX_SAND_PP_VLAN_ID_MAX                                    (4*1024-1)
#define DNX_SAND_PP_TC_MAX                                         (7)
#define DNX_SAND_PP_DP_MAX                                         (3)
#define DNX_SAND_PP_DEI_CFI_MAX                                    (1)
#define DNX_SAND_PP_PCP_UP_MAX                                     (7)
#define DNX_SAND_PP_ETHER_TYPE_MAX                                 (0xffff)
#define DNX_SAND_PP_TPID_MAX                                       (0xffff)
#define DNX_SAND_PP_IP_TTL_MAX                                     (255)
#define DNX_SAND_PP_IPV4_TOS_MAX                                   (255)
#define DNX_SAND_PP_IPV6_TC_MAX                                    (255)
#define DNX_SAND_PP_MPLS_LABEL_MAX                                 ((1<<20)-1)
#define DNX_SAND_PP_MPLS_EXP_MAX                                   (7)
#define DNX_SAND_PP_ISID_MAX                                       (0xffffff)
#define DNX_SAND_PP_L3_DSCP_MAX                                    (DNX_SAND_PP_IPV4_TOS_MAX) /* = DNX_SAND_PP_IPV6_TOS_MAX */
#define DNX_SAND_PP_MPLS_DSCP_MAX                                  (DNX_SAND_PP_MPLS_EXP_MAX)
#define DNX_SAND_PON_TUNNEL_ID_MAX                                 (0x7ff)
#define DNX_SAND_PP_IP_TTL_NOF_BITS                          (8)

#define DNX_SAND_PP_ETHER_TYPE_NOF_BITS                      (16)
#define DNX_SAND_PP_TPID_NOF_BITS                            (16)
#define DNX_SAND_PP_UP_NOF_BITS                              (3)
#define DNX_SAND_PP_VID_NOF_BITS                             (12)
#define DNX_SAND_PP_CFI_NOF_BITS                             (1)
#define DNX_SAND_PP_PCP_NOF_BITS                             (3)

#define DNX_SAND_PP_NOF_TC                                   (8)


/* Ethernet - Networking Standard */
#define DNX_SAND_PP_ETHERNET_ETHERTYPE_IPV4       (0x0800)
#define DNX_SAND_PP_ETHERNET_ETHERTYPE_TRILL      (0x22F3)
#define DNX_SAND_PP_ETHERNET_ETHERTYPE_IPV6       (0x86DD)
#define DNX_SAND_PP_ETHERNET_ETHERTYPE_MPLS       (0x8847)

/* Trill - Networking Standard */

#define DNX_SAND_PP_TRILL_NICK_NAME_NOF_BITS      (16)
#define DNX_SAND_PP_TRILL_M_NOF_BITS              (1)


/* IP protocol numbers - Networking Standard - */
#define DNX_SAND_PP_IP_PROTOCOL_NOF_BITS (8)

#define DNX_SAND_PP_IP_PROTOCOL_HOPOPT (0x00)
#define DNX_SAND_PP_IP_PROTOCOL_ICMP (0x01)
#define DNX_SAND_PP_IP_PROTOCOL_IGMP (0x02)
#define DNX_SAND_PP_IP_PROTOCOL_GGP (0x03)
#define DNX_SAND_PP_IP_PROTOCOL_IP_in_IP (0x04)
#define DNX_SAND_PP_IP_PROTOCOL_ST (0x05)
#define DNX_SAND_PP_IP_PROTOCOL_TCP (0x06)
#define DNX_SAND_PP_IP_PROTOCOL_CBT (0x07)
#define DNX_SAND_PP_IP_PROTOCOL_EGP (0x08)
#define DNX_SAND_PP_IP_PROTOCOL_IGP (0x09)
#define DNX_SAND_PP_IP_PROTOCOL_BBN_RCC_MON (0x0A)
#define DNX_SAND_PP_IP_PROTOCOL_NVP_II (0x0B)
#define DNX_SAND_PP_IP_PROTOCOL_PUP (0x0C)
#define DNX_SAND_PP_IP_PROTOCOL_ARGUS (0x0D)
#define DNX_SAND_PP_IP_PROTOCOL_EMCON (0x0E)
#define DNX_SAND_PP_IP_PROTOCOL_XNET (0x0F)
#define DNX_SAND_PP_IP_PROTOCOL_CHAOS (0x10)
#define DNX_SAND_PP_IP_PROTOCOL_UDP (0x11)
#define DNX_SAND_PP_IP_PROTOCOL_MUX (0x12)
#define DNX_SAND_PP_IP_PROTOCOL_DCN_MEAS (0x13)
#define DNX_SAND_PP_IP_PROTOCOL_HMP (0x14)
#define DNX_SAND_PP_IP_PROTOCOL_PRM (0x15)
#define DNX_SAND_PP_IP_PROTOCOL_XNS_IDP (0x16)
#define DNX_SAND_PP_IP_PROTOCOL_TRUNK_1 (0x17)
#define DNX_SAND_PP_IP_PROTOCOL_TRUNK_2 (0x18)
#define DNX_SAND_PP_IP_PROTOCOL_LEAF_1 (0x19)
#define DNX_SAND_PP_IP_PROTOCOL_LEAF_2 (0x1A)
#define DNX_SAND_PP_IP_PROTOCOL_RDP (0x1B)
#define DNX_SAND_PP_IP_PROTOCOL_IRTP (0x1C)
#define DNX_SAND_PP_IP_PROTOCOL_ISO_TP4 (0x1D)
#define DNX_SAND_PP_IP_PROTOCOL_NETBLT (0x1E)
#define DNX_SAND_PP_IP_PROTOCOL_MFE_NSP (0x1F)
#define DNX_SAND_PP_IP_PROTOCOL_MERIT_INP (0x20)
#define DNX_SAND_PP_IP_PROTOCOL_DCCP (0x21)
#define DNX_SAND_PP_IP_PROTOCOL_3PC (0x22)
#define DNX_SAND_PP_IP_PROTOCOL_IDPR (0x23)
#define DNX_SAND_PP_IP_PROTOCOL_XTP (0x24)
#define DNX_SAND_PP_IP_PROTOCOL_DDP (0x25)
#define DNX_SAND_PP_IP_PROTOCOL_IDPR_CMTP (0x26)
#define DNX_SAND_PP_IP_PROTOCOL_TP_PLUS_PLUS (0x27)
#define DNX_SAND_PP_IP_PROTOCOL_IL (0x28)
#define DNX_SAND_PP_IP_PROTOCOL_IPv6 (0x29)
#define DNX_SAND_PP_IP_PROTOCOL_SDRP (0x2A)
#define DNX_SAND_PP_IP_PROTOCOL_IPv6_Route (0x2B)
#define DNX_SAND_PP_IP_PROTOCOL_IPv6_Frag (0x2C)
#define DNX_SAND_PP_IP_PROTOCOL_IDRP (0x2D)
#define DNX_SAND_PP_IP_PROTOCOL_RSVP (0x2E)
#define DNX_SAND_PP_IP_PROTOCOL_GRE (0x2F)
#define DNX_SAND_PP_IP_PROTOCOL_MHRP (0x30)
#define DNX_SAND_PP_IP_PROTOCOL_BNA (0x31)
#define DNX_SAND_PP_IP_PROTOCOL_ESP (0x32)
#define DNX_SAND_PP_IP_PROTOCOL_AH (0x33)
#define DNX_SAND_PP_IP_PROTOCOL_I_NLSP (0x34)
#define DNX_SAND_PP_IP_PROTOCOL_SWIPE (0x35)
#define DNX_SAND_PP_IP_PROTOCOL_NARP (0x36)
#define DNX_SAND_PP_IP_PROTOCOL_MOBILE (0x37)
#define DNX_SAND_PP_IP_PROTOCOL_TLSP (0x38)
#define DNX_SAND_PP_IP_PROTOCOL_SKIP (0x39)
#define DNX_SAND_PP_IP_PROTOCOL_IPv6_ICMP (0x3A)
#define DNX_SAND_PP_IP_PROTOCOL_IPv6_NoNxt (0x3B)
#define DNX_SAND_PP_IP_PROTOCOL_IPv6_Opts (0x3C)
#define DNX_SAND_PP_IP_PROTOCOL_CFTP (0x3E)
#define DNX_SAND_PP_IP_PROTOCOL_SAT_EXPAK (0x40)
#define DNX_SAND_PP_IP_PROTOCOL_KRYPTOLAN (0x41)
#define DNX_SAND_PP_IP_PROTOCOL_RVD (0x42)
#define DNX_SAND_PP_IP_PROTOCOL_IPPC (0x43)
#define DNX_SAND_PP_IP_PROTOCOL_SAT_MON (0x45)
#define DNX_SAND_PP_IP_PROTOCOL_VISA (0x46)
#define DNX_SAND_PP_IP_PROTOCOL_IPCU (0x47)
#define DNX_SAND_PP_IP_PROTOCOL_CPNX (0x48)
#define DNX_SAND_PP_IP_PROTOCOL_CPHB (0x49)
#define DNX_SAND_PP_IP_PROTOCOL_WSN (0x4A)
#define DNX_SAND_PP_IP_PROTOCOL_PVP (0x4B)
#define DNX_SAND_PP_IP_PROTOCOL_BR_SAT_MON (0x4C)
#define DNX_SAND_PP_IP_PROTOCOL_SUN_ND (0x4D)
#define DNX_SAND_PP_IP_PROTOCOL_WB_MON (0x4E)
#define DNX_SAND_PP_IP_PROTOCOL_WB_EXPAK (0x4F)
#define DNX_SAND_PP_IP_PROTOCOL_ISO_IP (0x50)
#define DNX_SAND_PP_IP_PROTOCOL_VMTP (0x51)
#define DNX_SAND_PP_IP_PROTOCOL_SECURE_VMTP (0x52)
#define DNX_SAND_PP_IP_PROTOCOL_VINES (0x53)
#define DNX_SAND_PP_IP_PROTOCOL_TTP (0x54)
#define DNX_SAND_PP_IP_PROTOCOL_IPTM (0x54)
#define DNX_SAND_PP_IP_PROTOCOL_NSFNET_IGP (0x55)
#define DNX_SAND_PP_IP_PROTOCOL_DGP (0x56)
#define DNX_SAND_PP_IP_PROTOCOL_TCF (0x57)
#define DNX_SAND_PP_IP_PROTOCOL_EIGRP (0x58)
#define DNX_SAND_PP_IP_PROTOCOL_OSPF (0x59)
#define DNX_SAND_PP_IP_PROTOCOL_Sprite_RPC (0x5A)
#define DNX_SAND_PP_IP_PROTOCOL_LARP (0x5B)
#define DNX_SAND_PP_IP_PROTOCOL_MTP (0x5C)
#define DNX_SAND_PP_IP_PROTOCOL_AX_25 (0x5D)
#define DNX_SAND_PP_IP_PROTOCOL_IPIP (0x5E)
#define DNX_SAND_PP_IP_PROTOCOL_MICP (0x5F)
#define DNX_SAND_PP_IP_PROTOCOL_SCC_SP (0x60)
#define DNX_SAND_PP_IP_PROTOCOL_ETHERIP (0x61)
#define DNX_SAND_PP_IP_PROTOCOL_ENCAP (0x62)
#define DNX_SAND_PP_IP_PROTOCOL_GMTP (0x64)
#define DNX_SAND_PP_IP_PROTOCOL_IFMP (0x65)
#define DNX_SAND_PP_IP_PROTOCOL_PNNI (0x66)
#define DNX_SAND_PP_IP_PROTOCOL_PIM (0x67)
#define DNX_SAND_PP_IP_PROTOCOL_ARIS (0x68)
#define DNX_SAND_PP_IP_PROTOCOL_SCPS (0x69)
#define DNX_SAND_PP_IP_PROTOCOL_QNX (0x6A)
#define DNX_SAND_PP_IP_PROTOCOL_A_N (0x6B)
#define DNX_SAND_PP_IP_PROTOCOL_IPComp (0x6C)
#define DNX_SAND_PP_IP_PROTOCOL_SNP (0x6D)
#define DNX_SAND_PP_IP_PROTOCOL_Compaq_Peer (0x6E)
#define DNX_SAND_PP_IP_PROTOCOL_IPX_in_IP (0x6F)
#define DNX_SAND_PP_IP_PROTOCOL_VRRP (0x70)
#define DNX_SAND_PP_IP_PROTOCOL_PGM (0x71)
#define DNX_SAND_PP_IP_PROTOCOL_L2TP (0x73)
#define DNX_SAND_PP_IP_PROTOCOL_DDX (0x74)
#define DNX_SAND_PP_IP_PROTOCOL_IATP (0x75)
#define DNX_SAND_PP_IP_PROTOCOL_STP (0x76)
#define DNX_SAND_PP_IP_PROTOCOL_SRP (0x77)
#define DNX_SAND_PP_IP_PROTOCOL_UTI (0x78)
#define DNX_SAND_PP_IP_PROTOCOL_SMP (0x79)
#define DNX_SAND_PP_IP_PROTOCOL_SM (0x7A)
#define DNX_SAND_PP_IP_PROTOCOL_PTP (0x7B)
#define DNX_SAND_PP_IP_PROTOCOL_IS_IS over IPv4 (0x7C)
#define DNX_SAND_PP_IP_PROTOCOL_FIRE (0x7D)
#define DNX_SAND_PP_IP_PROTOCOL_CRTP (0x7E)
#define DNX_SAND_PP_IP_PROTOCOL_CRUDP (0x7F)
#define DNX_SAND_PP_IP_PROTOCOL_SSCOPMCE (0x80)
#define DNX_SAND_PP_IP_PROTOCOL_IPLT (0x81)
#define DNX_SAND_PP_IP_PROTOCOL_SPS (0x82)
#define DNX_SAND_PP_IP_PROTOCOL_PIPE (0x83)
#define DNX_SAND_PP_IP_PROTOCOL_SCTP (0x84)
#define DNX_SAND_PP_IP_PROTOCOL_FC (0x85)
#define DNX_SAND_PP_IP_PROTOCOL_RSVP_E2E_IGNORE (0x86)
#define DNX_SAND_PP_IP_PROTOCOL_Mobility Header (0x87)
#define DNX_SAND_PP_IP_PROTOCOL_UDPLite (0x88)
#define DNX_SAND_PP_IP_PROTOCOL_MPLS_in_IP (0x89)
#define DNX_SAND_PP_IP_PROTOCOL_manet (0x8A)
#define DNX_SAND_PP_IP_PROTOCOL_HIP (0x8B)
#define DNX_SAND_PP_IP_PROTOCOL_Shim6 (0x8C)
#define DNX_SAND_PP_IP_PROTOCOL_WESP (0x8D)
#define DNX_SAND_PP_IP_PROTOCOL_ROHC (0x8E)


#define DNX_SAND_PP_IP_NO_OPTION_NOF_BITS (20 * DNX_SAND_NOF_BITS_IN_BYTE)

#define DNX_SAND_PP_UDP_NOF_BITS (8 * DNX_SAND_NOF_BITS_IN_BYTE)

#define DNX_SAND_PP_VXLAN_NOF_BITS (8 * DNX_SAND_NOF_BITS_IN_BYTE)

#define DNX_SAND_PP_GRE2_NOF_BITS (2 * DNX_SAND_NOF_BITS_IN_BYTE)

#define DNX_SAND_PP_GRE4_NOF_BITS (4 * DNX_SAND_NOF_BITS_IN_BYTE)

#define DNX_SAND_PP_GRE8_NOF_BITS (8 * DNX_SAND_NOF_BITS_IN_BYTE)

#define DNX_SAND_PP_ERSPAN_TYPEII_NOF_BITS  (8 * DNX_SAND_NOF_BITS_IN_BYTE) 


/* } */
/*************
 * MACROS    *
 *************/
/* { */
/* given tag format return number of tags */
#define DNX_SAND_PP_NOF_TAGS_IN_VLAN_FORMAT(tag_format) \
      (tag_format == DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_NONE)? 0: \
      (tag_format == DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_S_TAG || tag_format == DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_C_TAG || \
       tag_format == DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_I_TAG || tag_format == DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_PRIORITY_TAG)?1:2;

#define DNX_SAND_PP_TOS_SET(__tos, __tos_val, __is_uniform) \
        (__tos) =  ((__is_uniform << 8) | (__tos_val))

#define DNX_SAND_PP_TOS_VAL_GET(__tos) \
         (0xff & (__tos))

#define DNX_SAND_PP_TOS_IS_UNIFORM_GET(__tos) \
         ((__tos) >> 8)


#define DNX_SAND_PP_TTL_SET(__ttl, __ttl_val, __is_uniform) \
        (__ttl) =  ((__is_uniform << 8) | (__ttl_val))

#define DNX_SAND_PP_TTL_VAL_GET(__ttl) \
         (0xff & (__ttl))

#define DNX_SAND_PP_TTL_IS_UNIFORM_GET(__ttl) \
         ((__ttl) >> 8)



/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/*
 * The local port id.
 *  may refer to ports of the following types
 *   - VBP.
 *   - CEP.
 *   - PEP.
 *   - CNP.
 *   - PNP.
 * use the proper Macro.
 * - for VBP ports local port id equal to the port local port number.
 */
typedef uint32 DNX_SAND_PP_LOCAL_PORT_ID;

/* 
*  Virtual LAN ID. Range: 0 - 4K-1.
*/
typedef uint32 DNX_SAND_PP_VLAN_ID; 

/* 
*  Tunnel ID. Range: 0 - 4K-1.
*/
typedef uint32 DNX_SAND_PON_TUNNEL_ID;

/* 
*  Traffic class. Range: 0 - 7.                    
*/
typedef uint8 DNX_SAND_PP_TC; 


/* 
*  Drop precedence. Range: 0 - 3.                          
*/
typedef uint8 DNX_SAND_PP_DP; 


/* 
 *  Canonical Format Indicator. Range: 0 - 1.               
 */
typedef uint8 DNX_SAND_PP_DEI_CFI; 


/* 
*  Priority code point / User Priority. Range: 0 - 7.      
*/
typedef uint8 DNX_SAND_PP_PCP_UP; 


/* 
*  Ethernet frame Ethertype field (16b). Range: 0 - 0xffff. 
*/
typedef uint16 DNX_SAND_PP_ETHER_TYPE; 


/* 
*  Tag Protocol Identifier. Range: 0 - 0xffff.             
*/
typedef uint16 DNX_SAND_PP_TPID; 


/* 
*  Time To Live. Range: 0 - 255.                           
*  extended to 16 bit as may has encoding of
*  uniform or pipe
*  see DNX_SAND_PP_TTL_SET(__ttl, _ttl_val, __is_uniform)
*/
typedef uint16 DNX_SAND_PP_IP_TTL; 


/* 
*  Type Of Services (Aka Differentiated Services). Range: 0 
*  - 255.
*  extended to 16 bit as may has encoding of
*  uniform or pipe
*  see DNX_SAND_PP_TOS_SET(__tos, _tos_val, __is_uniform)
*/
typedef uint16 DNX_SAND_PP_IPV4_TOS; 


/* 
*  Traffic Class. Range: 0 - 255.                          
*/
typedef uint8 DNX_SAND_PP_IPV6_TC; 


/* 
*  MPLS Label. Range: 0 - 2^20-1.                          
*/
typedef uint32 DNX_SAND_PP_MPLS_LABEL; 


/* 
*  Experimenal bits in MPLS header. Used for QoS. Range: 0 
*  - 7.                                                    
*/
typedef uint8 DNX_SAND_PP_MPLS_EXP; 


/* 
*  ???                                                     
*/
typedef uint32 DNX_SAND_PP_ISID; 



typedef enum
{
  /*
   *                                                          
   */
  DNX_SAND_PP_L4_PORT_TYPE_UDP = 0,
  /*
   *                                                          
   */
  DNX_SAND_PP_L4_PORT_TYPE_TCP = 1,
  /*
   *  Number of types in DNX_SAND_PP_L4_PORT_TYPE
   */
  DNX_SAND_PP_L4_PORT_TYPE_LAST
}DNX_SAND_PP_L4_PORT_TYPE;





typedef enum
{
  /*
   *  No Forwarding Action
   */
  DNX_SAND_PP_FRWRD_ACTION_TYPE_NONE=0,
  /*
   *  The packet should be forwarded normally
   */
  DNX_SAND_PP_FRWRD_ACTION_TYPE_NORMAL=1,
  /*
   *  The packet should be intercepted, usually to the CPU.
   *  This means that the internal header carries a CPU code.
   */
  DNX_SAND_PP_FRWRD_ACTION_TYPE_INTERCEPT=2,
  /*
   *  The packet is a control packet, usually destined to the
   *  CPU. This means that the internal header carries a CPU
   *  code
   */
  DNX_SAND_PP_FRWRD_ACTION_TYPE_CONTROL=3,
  /*
   *  Must be the last value
   */
  DNX_SAND_PP_NOF_FRWRD_ACTION_TYPES
}DNX_SAND_PP_FRWRD_ACTION_TYPE;


typedef enum
{
  /*
   *  No TAG Type-this may refer to Priority TAG or No Tag (in 
   *  edit command).                                          
   */
  DNX_SAND_PP_VLAN_TAG_TYPE_NONE = 0,
  /*
   *  The tag type is C-Tag. Tag includes TPID, C-VID, CFI and 
   *  UP. In some places, it may refer to PCP, but when C-tag 
   *  is used/generated this is actually the UP of the C-tag. 
   *  Where DEI is used, it may refer to the CFI and it should 
   *  be zero.                                                
   */
  DNX_SAND_PP_VLAN_TAG_TYPE_CTAG = 1,
  /*
   *  The tag type is S-Tag. Tag includes TPID, S-VID, DEI and 
   *  PCP.                                                    
   */
  DNX_SAND_PP_VLAN_TAG_TYPE_STAG = 2,
  /*
   *  Priority Tag (VID = 0).                                 
   */
  DNX_SAND_PP_VLAN_TAG_TYPE_PRIORITY = 3,
  /*
   *  VLAN TAG is unspecified. Use for example for Soc_petra-B 
   *  where TAG-type in TPID profile info is irrelevant.      
   */
  DNX_SAND_PP_VLAN_TAG_TYPE_ANY = 4,
  /*
   *  802.1ah Service Instance TAG                            
   */
  DNX_SAND_PP_VLAN_TAG_TYPE_ITAG = 5,
  /*
   *  Number of types in DNX_SAND_PP_VLAN_TAG_TYPE
   */
  DNX_SAND_PP_NOF_VLAN_TAG_TYPES = 6
}DNX_SAND_PP_VLAN_TAG_TYPE;

typedef enum
{
   /*
   *  Destination MAC address is invalid. used when destination MAC is not exist
   */
    DNX_SAND_PP_ETHERNET_DA_TYPE_TYPE_INVALID = -1,
  /*
   *  Destination MAC address is UC.                          
   */
  DNX_SAND_PP_ETHERNET_DA_TYPE_TYPE_MC = 0,
  /*
   *  Destination MAC address is MC.                          
   */
  DNX_SAND_PP_ETHERNET_DA_TYPE_TYPE_BC = 1,
  /*
   *  Destination MAC address is BC.                          
   */
  DNX_SAND_PP_ETHERNET_DA_TYPE_TYPE_UC = 2,
  /*
   *  Number of types in DNX_SAND_PP_ETHERNET_DA_TYPE
   */
  DNX_SAND_PP_NOF_ETHERNET_DA_TYPES = 3
}DNX_SAND_PP_ETHERNET_DA_TYPE;

typedef enum
{
  /*
   *  VLAN Bridge Port                                        
   */
  DNX_SAND_PP_PORT_L2_TYPE_VBP = 0,
  /*
   *  Customer Network Port: An S-VLAN component Port on a 
   *  Provider Bridge or within a Provider Edge Bridge that 
   *  receives and transmits frame for a single customer.     
   */
  DNX_SAND_PP_PORT_L2_TYPE_CNP = 1,
  /*
   *  Customer Edge Port: A C-VLAN component Port on a 
   *  Provider Edge Bridge that is connected to customer owned 
   *  equipment and receives and transmits frames for a single 
   *  customer.                                               
   */
  DNX_SAND_PP_PORT_L2_TYPE_CEP = 2,
  /*
   *  Provider Network Port: An S-VLAN component Port on a 
   *  Provider Bridge that can transmit and receive frames for 
   *  multiple customers.                                     
   */
  DNX_SAND_PP_PORT_L2_TYPE_PNP = 3,
  /*
   *  Number of types in DNX_SAND_PP_PORT_L2_TYPE
   */
  DNX_SAND_PP_NOF_PORT_L2_TYPES = 4
}DNX_SAND_PP_PORT_L2_TYPE;
typedef enum
{
  /*
   *  Single port                                             
   */
  DNX_SAND_PP_SYS_PORT_TYPE_SINGLE_PORT = 0,
  /*
   *  LAG                                                     
   */
  DNX_SAND_PP_SYS_PORT_TYPE_LAG = 1,
  /*
   *  Number of types in DNX_SAND_PP_SYS_PORT_TYPE
   */
  DNX_SAND_PP_NOF_SYS_PORT_TYPES = 2
}DNX_SAND_PP_SYS_PORT_TYPE;
typedef enum
{
  /*
   *                                                          
   */
  DNX_SAND_PP_L4_PRTCL_TYPE_UDP = 0x1,
  /*
   *                                                          
   */
  DNX_SAND_PP_L4_PRTCL_TYPE_TCP = 0x2,
  /*
   *  For both TCP/UDP.                                       
   */
  DNX_SAND_PP_L4_PRTCL_TYPE_TCP_UDP = 0x3,
  /*
   *  Number of types in DNX_SAND_PP_L4_PRTCL_TYPE
   */
  DNX_SAND_PP_NOF_L4_PRTCL_TYPES
}DNX_SAND_PP_L4_PRTCL_TYPE;
typedef enum
{
  /*
   *  None                                                    
   */
  DNX_SAND_PP_IP_TYPE_NONE = 0x0,
  /*
   *  IPv4 Unicast                                            
   */
  DNX_SAND_PP_IP_TYPE_IPV4_UC = 0x1,
  /*
   *  IPv4 Multicast                                          
   */
  DNX_SAND_PP_IP_TYPE_IPV4_MC = 0x2,
  /*
   *  IPv4 Unicast/Multicast                                  
   */
  DNX_SAND_PP_IP_TYPE_IPV4 = 0x3,
  /*
   *  IPv6 Unicast                                            
   */
  DNX_SAND_PP_IP_TYPE_IPV6_UC = 0x4,
  /*
   *  IPv6 Multicast                                          
   */
  DNX_SAND_PP_IP_TYPE_IPV6_MC = 0x8,
  /*
   *  IPv6 Unicast/Multicast                                  
   */
  DNX_SAND_PP_IP_TYPE_IPV6 = 0xC,
  /*
   *  IPv4/6 Unicast/Multicast                                
   */
  DNX_SAND_PP_IP_TYPE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in DNX_SAND_PP_IP_TYPE
   */
  DNX_SAND_PP_NOF_IP_TYPES = 7
}DNX_SAND_PP_IP_TYPE;

typedef enum
{
  /*
   *  The orientation of the interface/tunnel is hub i.e. 
   *  connected from the Network side. Traffic may not be 
   *  forwarded between two hub interfaces, to prevent 
   *  loopback on the network side.                           
   */
  DNX_SAND_PP_HUB_SPOKE_ORIENTATION_HUB = 0,
  /*
   *  The orientation of the interface/tunnel is 'spoke', 
   *  i.e., connected from the Access side. Traffic entering 
   *  from spoke interface may be forwarded either to spoke or 
   *  hub interfaces.                                         
   */
  DNX_SAND_PP_HUB_SPOKE_ORIENTATION_SPOKE = 1,
  /*
   *  Number of types in DNX_SAND_PP_HUB_SPOKE_ORIENTATION
   */
  DNX_SAND_PP_NOF_HUB_SPOKE_ORIENTATIONS = 2
}DNX_SAND_PP_HUB_SPOKE_ORIENTATION;
typedef enum
{
  /*
   *  The orientation of the interface/tunnel is 'split'.     
   */
  DNX_SAND_PP_SPLIT_HORIZON_ORIENTATION_SPLIT = 0,
  /*
   *  The orientation of the interface/tunnel is 'horizon'.   
   */
  DNX_SAND_PP_HUB_SPOKE_ORIENTATION_SPOKE_HORIZON = 1,
  /*
   *  Number of types in DNX_SAND_PP_SPLIT_HORIZON_ORIENTATION
   */
  DNX_SAND_PP_NOF_SPLIT_HORIZON_ORIENTATIONS = 2
}DNX_SAND_PP_SPLIT_HORIZON_ORIENTATION;
typedef enum
{
  /*
   *  Drop destination                                        
   */
  DNX_SAND_PP_DEST_TYPE_DROP = 0,
  /*
   *  Router destination. Relevant only for Soc_petra-A.          
   */
  DNX_SAND_PP_DEST_TYPE_ROUTER = 1,
  /*
   *  Single port                                             
   */
  DNX_SAND_PP_DEST_SINGLE_PORT = 2,
  /*
   *  Explicit flow.Also referred to as direct flow.Enable 
   *  allocating dedicated QoS resources for a PP destination. 
   */
  DNX_SAND_PP_DEST_EXPLICIT_FLOW = 3,
  /*
   *  LAG                                                     
   */
  DNX_SAND_PP_DEST_LAG = 4,
  /*
   *  LIF Group Destination                                   
   */
  DNX_SAND_PP_DEST_MULTICAST = 5,
  /*
   *  FEC Destination. 
   */
  DNX_SAND_PP_DEST_FEC = 6,
  /*
   *  Trap Destination. 
   */
  DNX_SAND_PP_DEST_TRAP = 7,
  /*
   *  Number of types in DNX_SAND_PP_DEST_TYPE
   */
  DNX_SAND_PP_NOF_DEST_TYPES = 8
}DNX_SAND_PP_DEST_TYPE;
typedef enum
{
  /*
   *  Pipe Model: A MPLS network acts like a circuit when MPLS 
   *  packets traverse the network such that only the LSP 
   *  ingress and egress points are visible to nodes that are 
   *  outside the tunnel.                                     
   */
  DNX_SAND_PP_MPLS_TUNNEL_MODEL_PIPE = 0,
  /*
   *  Uniform Model: Makes all the nodes that a LSP traverses 
   *  visible to nodes outside the tunnel.                    
   */
  DNX_SAND_PP_MPLS_TUNNEL_MODEL_UNIFORM = 1,

  /* From JER2_QAX: Set ttl or exp value according to push profile table*/
  DNX_SAND_PP_MPLS_TUNNEL_MODEL_SET = 2,
  /* From JER2_QAX: Copy ttl or exp value fro mprevious header */
  DNX_SAND_PP_MPLS_TUNNEL_MODEL_COPY = 3,
  /*
   *  Number of types in DNX_SAND_PP_MPLS_TUNNEL_MODEL
   */
  DNX_SAND_PP_NOF_MPLS_TUNNEL_MODELS = 4
}DNX_SAND_PP_MPLS_TUNNEL_MODEL;
typedef enum
{
  /*
   *  Route command: The packet is routed and sent to next 
   *  destination.                                            
   */
  DNX_SAND_PP_FEC_COMMAND_TYPE_ROUTE = 0,
  /*
   *  Trap command: The packet is forwarded to the CPU with a 
   *  "cpu_code".                                             
   */
  DNX_SAND_PP_FEC_COMMAND_TYPE_TRAP = 1,
  /*
   *  Drop command: The packet is dropped-no further 
   *  processing is done for the packet.                      
   */
  DNX_SAND_PP_FEC_COMMAND_TYPE_DROP = 2,
  /*
   *  Number of types in DNX_SAND_PP_FEC_COMMAND_TYPE
   */
  DNX_SAND_PP_NOF_FEC_COMMAND_TYPES = 3
}DNX_SAND_PP_FEC_COMMAND_TYPE;

typedef enum
{
  /*
   *  Frame has no VLAN TAGs                                  
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_NONE = 0,
  /*
   *  Frame has C-VLAN TAG                                    
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_C_TAG = 1,
  /*
   *  Frame has S-VLAN TAG                                    
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_S_TAG = 2,
  /*
   *  Frame has Only Priority TAG                             
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_PRIORITY_TAG = 3,
  /*
   *  Frame has C-C-VLAN TAGs                        
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_C_C_TAG = 5,
  /*
   *  Frame has Priority-C-VLAN TAGs                          
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_PRIORITY_C_TAG = 7,
  /*
   *  Frame has S-C-VLAN TAGs                                 
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_S_C_TAG = 6,
  /*
   *  Frame has C-S-VLAN TAGs                         
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_C_S_TAG = 9,
  /*
   *  Frame has S-S-VLAN TAGs                          
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_S_S_TAG = 10,
  /*
   *  Frame has Priority-S-VLAN TAGs                          
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_PRIORITY_S_TAG = 11,
  /*
   *  Frame has I-TAG
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_I_TAG = 4,
  /*
   *  Frame with any VLAN format. Packet may have any 
   *  number/type of tags. This type is used for setting the 
   *  acceptable frame types. When this type is used, all VLAN 
   *  formats are accepted or not, according to the accept 
   *  parameter value. See, for example, 
   *  soc_ppd_llp_filter_ingress_acceptable_frames_set().         
   */
  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY = (int)0xFFFFFFFF,
  /*
   *  Number of types in DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT
   */
  DNX_SAND_PP_NOF_ETHERNET_FRAME_VLAN_FORMATS = 16
}DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT;

typedef enum
{
  /*
   *  Ethernet II                                             
   */
  DNX_SAND_PP_ETH_ENCAP_TYPE_ETH2 = 0,
  /*
   *  Ethernet with LLC (logical link control) 802.3          
   */
  DNX_SAND_PP_ETH_ENCAP_TYPE_LLC = 1,
  /*
   *  Ethernet with LLC and snap                              
   */
  DNX_SAND_PP_ETH_ENCAP_TYPE_LLC_SNAP = 2,
  /*
   *  Other encapsulation type                                
   */
  DNX_SAND_PP_ETH_ENCAP_TYPE_OTHER = 3,
  /*
   *  Number of types in DNX_SAND_PP_ETH_ENCAP_TYPE
   */
  DNX_SAND_PP_NOF_ETH_ENCAP_TYPES = 4
}DNX_SAND_PP_ETH_ENCAP_TYPE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The type of the following destination ID                
   */
  DNX_SAND_PP_DEST_TYPE dest_type;
  /*
   *  The destination value, according to the type (Single, 
   *  LAG, Multicast...)For the LAG type, the value is the LAG 
   *  ID. Range: 0 - 255.For the DROP type, value is not 
   *  relevant.For the ROUTER type, value is not relevant.For 
   *  the MULTICAST type, range: 0 - 16383.For the Single_Port 
   *  type, range: 0 - 4095.                                  
   */
  uint32 dest_val;

} DNX_SAND_PP_DESTINATION_ID;

/*
 *  IPV6.mac[0] includes the lsb of the MAC address
 *  (network order).
 */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The Ipv4 address                
   */
  uint32 address[DNX_SAND_PP_IPV4_ADDRESS_NOF_UINT32S];

} DNX_SAND_PP_IPV4_ADDRESS;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  User Priority. Range 0-7.
   */
  uint32 user_priority;
  /*
   *  Traffic Class. Range0-7.
   */
  uint32 traffic_class;
  /*
   *  Drop Precedence. Range 0-3
   */
  uint32 drop_precedence;
}DNX_SAND_PP_COS_PARAMS;


 /*  System FEC. This is a system-level identifier.It may
  *  refer to one FEC entry, or to ECMP, which includes a
  *  number of FEC entries.
  */
typedef uint32 DNX_SAND_PP_SYSTEM_FEC_ID;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Ipv4 address.                               
   */
  uint32 ip_address;
  /*
   *  Number of bits to consider in the IP address starting 
   *  from the msb. Range: 0 - 32.Example for key ip_address 
   *  192.168.1.0 and prefix_len 24 would match any IP Address 
   *  of the form 192.168.1.x                                 
   */
  uint8 prefix_len;

} DNX_SAND_PP_IPV4_SUBNET;
/*
 *  IPV6.mac[0] includes the lsb of the MAC address
 *  (network order).
 */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The Ipv6 address composed of four longs.                
   */
  uint32 address[DNX_SAND_PP_IPV6_ADDRESS_NOF_UINT32S];

} DNX_SAND_PP_IPV6_ADDRESS;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Ipv6 address.                                           
   */
  DNX_SAND_PP_IPV6_ADDRESS ipv6_address;
  /*
   *  Number of bits to consider in the IP address starting 
   *  from the msb.Range: 0 - 128.                            
   */
  uint8 prefix_len;

} DNX_SAND_PP_IPV6_SUBNET;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The system port type single/LAG.                        
   */
  DNX_SAND_PP_SYS_PORT_TYPE sys_port_type;
  /*
   *  The system port value, according to the type (Single or 
   *  LAG)For LAG the value is the group ID.                  
   */
  uint32 sys_id;

} DNX_SAND_PP_SYS_PORT_ID;

typedef struct
{
  /*
   *  Value
   */
  uint32 val;
  /*
   *  Bitmap mask over the value:
   *      1  corresponding bit is considered
   *      0  the corresponding bit is ignored
   */
  uint32 mask;
}DNX_SAND_PP_MASKED_VAL;
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Tag Protocol Identifier. Range: 0 - 0xffff.             
   */
  DNX_SAND_PP_TPID tpid;
  /*
   *  VLAN ID.Range: 0 - 4095.                                
   */
  DNX_SAND_PP_VLAN_ID vid;
  /*
   *  Priority Code Point. Refers to the IEEE 802.1p priority. 
   *  For C-Tag it is the User Priority. Range: 0 - 7.        
   */
  DNX_SAND_PP_PCP_UP pcp;
  /*
   *  Drop Eligibility Indicator.For C-tag, this is the CFI 
   *  (Canonical Format Indicator) and has to be 0.Range: 0 - 
   *  1.                                                      
   */
  DNX_SAND_PP_DEI_CFI dei;

} DNX_SAND_PP_VLAN_TAG;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Source subnet .
   */
  DNX_SAND_PP_IPV4_SUBNET source;
  /*
   *  The multicast IP address of the destination group. Class
   *  D. Range 224.0.0.0 to 239.255.255.255.
   */
  DNX_SAND_PP_IPV4_SUBNET group;
  /*
   *  The vid of the incoming packets.
   */
  DNX_SAND_PP_MASKED_VAL vid;
  /*
   *  The port of the incoming packets.
   */
  DNX_SAND_PP_MASKED_VAL port;
}DNX_SAND_PP_IPV4_MC_ROUTE_KEY;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
 /*
  *  Source subnet .
  */
  DNX_SAND_PP_IPV6_SUBNET dest;

  DNX_SAND_PP_MASKED_VAL vid;
  /*
  *  Is the in_vid valid.
  */
  DNX_SAND_PP_MASKED_VAL port;

}DNX_SAND_PP_IPV6_ROUTE_KEY;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Is the TRILL packet is sent as multicast (flooded in the 
   *  Distribution Tree) or sent as Unicast to Egress Nick.   
   */
  uint8 is_multicast;
  /*
   *  If multicast set to FALSE, then this is the egress-Nick. 
   *  If multicast set to TRUE, then this is the 
   *  Dist-Tree-Nick to identify the distribution tree.       
   */
  uint32 dest_nick;

} DNX_SAND_PP_TRILL_DEST;

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

uint32
  dnx_sand_pp_ipv4_subnet_verify(
    DNX_SAND_IN  DNX_SAND_PP_IPV4_SUBNET *subnet
  );

uint32
  DNX_SAND_PP_TRILL_DEST_verify(
    DNX_SAND_IN  DNX_SAND_PP_TRILL_DEST *info
  );

uint32
  dnx_sand_pp_ipv4_address_string_parse(
    DNX_SAND_IN char               ipv4_string[DNX_SAND_PP_IPV4_ADDRESS_STRING_LEN],
    DNX_SAND_OUT DNX_SAND_PP_IPV4_ADDRESS   *ipv4_addr
  );

uint32
  dnx_sand_pp_ipv6_address_string_parse(
    DNX_SAND_IN char               ipv6_string[DNX_SAND_PP_IPV6_ADDRESS_STRING_LEN],
    DNX_SAND_OUT DNX_SAND_PP_IPV6_ADDRESS   *ipv6_addr
  );

void
  dnx_sand_SAND_PP_DESTINATION_ID_clear(
    DNX_SAND_OUT DNX_SAND_PP_DESTINATION_ID *info
  );

void
  dnx_sand_SAND_PP_COS_PARAMS_clear(
    DNX_SAND_OUT DNX_SAND_PP_COS_PARAMS *info
  );

void
  dnx_sand_SAND_PP_SYS_PORT_ID_clear(
    DNX_SAND_OUT DNX_SAND_PP_SYS_PORT_ID *info
  );

void
  dnx_sand_SAND_PP_IPV4_SUBNET_clear(
    DNX_SAND_OUT DNX_SAND_PP_IPV4_SUBNET *info
  );

void
  dnx_sand_SAND_PP_IPV4_ADDRESS_clear(
    DNX_SAND_OUT DNX_SAND_PP_IPV4_ADDRESS *info
  );

void
  dnx_sand_SAND_PP_IPV6_ADDRESS_clear(
    DNX_SAND_OUT DNX_SAND_PP_IPV6_ADDRESS *info
  );

void
  dnx_sand_SAND_PP_IPV6_SUBNET_clear(
    DNX_SAND_OUT DNX_SAND_PP_IPV6_SUBNET *info
  );

void
  dnx_sand_SAND_PP_TRILL_DEST_clear(
    DNX_SAND_OUT DNX_SAND_PP_TRILL_DEST *info
  );

void
  DNX_SAND_PP_VLAN_TAG_clear(
    DNX_SAND_OUT DNX_SAND_PP_VLAN_TAG *info
  );


void
  dnx_sand_SAND_PP_IPV4_MC_ROUTE_KEY_clear(
    DNX_SAND_OUT DNX_SAND_PP_IPV4_MC_ROUTE_KEY *info
  );

void
dnx_sand_SAND_PP_IPV6_ROUTE_KEY_clear(
  DNX_SAND_OUT DNX_SAND_PP_IPV6_ROUTE_KEY *info
  );

uint32
  DNX_SAND_PP_DESTINATION_ID_encode(
    DNX_SAND_IN  DNX_SAND_PP_DESTINATION_ID               *dest_info,
    DNX_SAND_OUT uint32                             *encoded_dest_val
  );

uint32
  DNX_SAND_PP_DESTINATION_ID_decode(
    DNX_SAND_IN  uint32                             encoded_dest_val,
    DNX_SAND_OUT DNX_SAND_PP_DESTINATION_ID               *dest_info
  );


#if DNX_SAND_DEBUG

const char*
  dnx_sand_SAND_PP_COS_PARAMS_to_string(
    DNX_SAND_IN DNX_SAND_PP_COS_PARAMS enum_val
  );

const char*
  dnx_sand_SAND_PP_DEST_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_DEST_TYPE enum_val
  );

const char*
  dnx_sand_SAND_PP_FRWRD_ACTION_TYPE_to_string(
    DNX_SAND_IN DNX_SAND_PP_FRWRD_ACTION_TYPE enum_val
  );

const char*
  dnx_sand_SAND_PP_FRWRD_ACTION_TYPE_to_string_short(
    DNX_SAND_IN DNX_SAND_PP_FRWRD_ACTION_TYPE enum_val
  );

const char*
  dnx_sand_SAND_PP_PORT_L2_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_PORT_L2_TYPE enum_val
  );

const char*
  dnx_sand_SAND_PP_PORT_L2_TYPE_to_string_short(
    DNX_SAND_IN DNX_SAND_PP_PORT_L2_TYPE enum_val
  );

const char*
  dnx_sand_SAND_PP_VLAN_TAG_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_VLAN_TAG_TYPE enum_val
  );

void
  dnx_sand_SAND_PP_IPV4_SUBNET_print_short(
    DNX_SAND_IN DNX_SAND_PP_IPV4_SUBNET *info
  );

const char*
  dnx_sand_SAND_PP_ETHERNET_DA_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_ETHERNET_DA_TYPE enum_val
  );

const char*
  dnx_sand_SAND_PP_SYS_PORT_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_SYS_PORT_TYPE enum_val
  );

const char*
  dnx_sand_SAND_PP_L4_PORT_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_L4_PORT_TYPE enum_val
  );

const char*
  dnx_sand_pp_ip_long_to_string(
    DNX_SAND_IN   uint32    ip_addr,
    DNX_SAND_IN   uint8     short_format,
    DNX_SAND_OUT   char  decimal_ip[DNX_SAND_PP_IPV4_MAX_IP_STRING]
  );

const char*
  dnx_sand_SAND_PP_FEC_COMMAND_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_FEC_COMMAND_TYPE enum_val
  );

void
  dnx_sand_SAND_PP_SYS_PORT_ID_table_format_print(
    DNX_SAND_IN DNX_SAND_PP_SYS_PORT_ID *info
  );

const char*
  dnx_sand_SAND_PP_L4_PRTCL_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_L4_PRTCL_TYPE enum_val
  );

const char*
  DNX_SAND_PP_IP_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_IP_TYPE enum_val
  );
void
  dnx_sand_SAND_PP_COS_PARAMS_print(
    DNX_SAND_IN DNX_SAND_PP_COS_PARAMS *info
  );

const char*
  dnx_sand_SAND_PP_HUB_SPOKE_ORIENTATION_to_string(
    DNX_SAND_IN  DNX_SAND_PP_HUB_SPOKE_ORIENTATION enum_val
  );

const char*
  dnx_sand_SAND_PP_SPLIT_HORIZON_ORIENTATION_to_string(
    DNX_SAND_IN  DNX_SAND_PP_SPLIT_HORIZON_ORIENTATION enum_val
  );

const char*
  dnx_sand_SAND_PP_MPLS_TUNNEL_MODEL_to_string(
    DNX_SAND_IN  DNX_SAND_PP_MPLS_TUNNEL_MODEL enum_val
  );

const char*
  dnx_sand_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_to_string(
    DNX_SAND_IN  DNX_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT enum_val
  );

const char*
  DNX_SAND_PP_ETH_ENCAP_TYPE_to_string(
    DNX_SAND_IN  DNX_SAND_PP_ETH_ENCAP_TYPE enum_val
  );


void
  dnx_sand_SAND_PP_DESTINATION_ID_table_format_print(
    DNX_SAND_IN DNX_SAND_PP_DESTINATION_ID *info
  );

void
  dnx_sand_SAND_PP_DESTINATION_ID_print(
    DNX_SAND_IN  DNX_SAND_PP_DESTINATION_ID *info
  );

void
  dnx_sand_SAND_PP_IPV4_SUBNET_print(
    DNX_SAND_IN  DNX_SAND_PP_IPV4_SUBNET *info
  );

void
  dnx_sand_SAND_PP_IPV4_ADDRESS_print(
    DNX_SAND_IN  DNX_SAND_PP_IPV4_ADDRESS *info
  );

void
  dnx_sand_SAND_PP_IPV6_ADDRESS_print(
    DNX_SAND_IN  DNX_SAND_PP_IPV6_ADDRESS *info
  );

void
  dnx_sand_SAND_PP_IPV6_SUBNET_print(
    DNX_SAND_IN  DNX_SAND_PP_IPV6_SUBNET *info
  );

void
  dnx_sand_SAND_PP_TRILL_DEST_print(
    DNX_SAND_IN  DNX_SAND_PP_TRILL_DEST *info
  );

void
  dnx_sand_SAND_PP_IPV4_MC_ROUTE_KEY_print(
    DNX_SAND_IN DNX_SAND_PP_IPV4_MC_ROUTE_KEY *info
  );

void
  dnx_sand_SAND_PP_IPV6_ROUTE_KEY_print(
    DNX_SAND_IN DNX_SAND_PP_IPV6_ROUTE_KEY *info
  );

void
  dnx_sand_SAND_PP_SYS_PORT_ID_print(
    DNX_SAND_IN  DNX_SAND_PP_SYS_PORT_ID *info
  );

void
  DNX_SAND_PP_VLAN_TAG_print(
    DNX_SAND_IN  DNX_SAND_PP_VLAN_TAG *info
  );


#endif /* DNX_SAND_DEBUG */
/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_SAND_PP_GENERAL_INCLUDED__*/
#endif
