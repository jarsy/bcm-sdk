/*
 * $Id: c3Pp.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _CALADAN3_PP_H_
#define _CALADAN3_PP_H_

#include <sal/types.h>

typedef enum c3PpExcType_e
{
  CALADAN3_INV_PPP_ADDR_CTL          = 0x10, /**< PPP Invalid Address or Control */
  CALADAN3_INV_PPP_PID               = 0x11, /**< PPP Invalid PID field */
  CALADAN3_ENET_VLAN_EQUAL_3FF_TAG1  = 0x15, /**< Ethernet VLAN= 0x3FF for Tag 1 */
  CALADAN3_ENET_VLAN_EQUAL_3FF_TAG2  = 0x16, /**< Ethernet VLAN= 0x3FF for Tag 2 */
  CALADAN3_ENET_VLAN_EQUAL_3FF_TAG3  = 0x17, /**< Ethernet VLAN= 0x3FF for Tag 3 */
  CALADAN3_ENET_SMAC_EQ_DMAC         = 0x18, /**< Ethernet SMAC equals DMAC */
  CALADAN3_INV_GRE_RES0              = 0x19, /**< Invalid GRE Reserved0 field */
  CALADAN3_UNK_MPLS_LBL_LABEL0       = 0x1A, /**< MPLS Label out of range for label 0 */
  CALADAN3_UNK_MPLS_LBL_LABEL1       = 0x1B, /**< MPLS Label out of range for label 1 */
  CALADAN3_UNK_MPLS_LBL_LABEL2       = 0x1C, /**< MPLS Label out of range for label 2 */
  CALADAN3_ENET_SMAC_EQ_DMAC_ZERO    = 0x1D, /**< Ethernet SMAC or DMAC equal zero */
  CALADAN3_ENET_TYPE_BETWEEN_1501_AND_1536 = 0x1E, /**< Ethernet Type value is
                                                        between [1501 and 1536] */
  CALADAN3_ENET_SMAC_EQ_MULTICAST    = 0x1F, /**< SMAC is multicast */
  CALADAN3_IPV4_RUNT_PKT             = 0x20, /**< Packet too small for an IPv4 packet */
  CALADAN3_IPV4_OPTIONS              = 0x21, /**< IPv4 options detected */
  CALADAN3_INV_IPV4_CHECKSUM         = 0x22, /**< Invalid IPv4 header checksum */
  CALADAN3_INV_IPV4_VER              = 0x23, /**< Invalid IPv4 version */
  CALADAN3_INV_IPV4_RUNT_HDR         = 0x24, /**< IPv4 header length less than 5 */
  CALADAN3_INV_IPV4_LEN_ERR          = 0x25, /**< IPv4 Length must be greater than 20 bytes */
  CALADAN3_INV_IPV4_PKT_LEN_ERR      = 0x26, /**< IPv4 Length must be smaller than
                                                  the packet length */
  CALADAN3_INV_IPV4_SA               = 0x27, /**< Invalid IPv4 Source Address */
  CALADAN3_INV_IPV4_DA               = 0x28, /**< Invalid IPv4 Destination Address */
  CALADAN3_INV_IPV4_SA_EQ_DA         = 0x29, /**< IPv4 Source Address equals
                                                  Destination Address */
  CALADAN3_INV_IPV4_SA_OR_DA_IS_LOOPBACK     = 0x2A, /**< IPv4 Source Address or
                                                          Destination Address is Loopback */
  CALADAN3_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS  = 0x2B, /**< IPv4 Martian address check */
  CALADAN3_IPV4_USR_ADDR_0           = 0x30, /**< IPv4 Filter 0 */
  CALADAN3_IPV4_USR_ADDR_1           = 0x31, /**< IPv4 Filter 1 */
  CALADAN3_IPV4_USR_ADDR_2           = 0x32, /**< IPv4 Filter 2 */
  CALADAN3_IPV4_USR_ADDR_3           = 0x33, /**< IPv4 Filter 3 */
  CALADAN3_IPV4_USR_ADDR_4           = 0x34, /**< IPv4 Filter 4 */
  CALADAN3_IPV4_USR_ADDR_5           = 0x35, /**< IPv4 Filter 5 */
  CALADAN3_IPV4_USR_ADDR_6           = 0x36, /**< IPv4 Filter 6 */
  CALADAN3_IPV4_USR_ADDR_7           = 0x37, /**< IPv4 Filter 7 */
  CALADAN3_IPV4_FRAG_ICMP_PROTOCOL   = 0x38, /**< IPv4 ICMP packet is fragmented */
  CALADAN3_IPV6_RUNT_PKT             = 0x40, /**< Packet too small for an IPv6 packet */
  CALADAN3_INV_IPV6_VER              = 0x41, /**< Invalid IPv6 version */
  CALADAN3_IPV6_PKT_LEN_ERR          = 0x42, /**< IPv6 Length must be smaller than
                                                  the packet length */
  CALADAN3_INV_IPV6_SA               = 0x43, /**< Invalid IPv6 Source Address */
  CALADAN3_INV_IPV6_DA               = 0x44, /**< Invalid IPv6 Destination Address */
  CALADAN3_IPV6_SA_EQ_DA             = 0x45, /**< IPv6 Source Address equals
                                                  Destination Address */
  CALADAN3_IPV6_SA_OR_DA_LOOPBACK    = 0x46, /**< IPv6 Source Address or Destination
                                                  Address is Loopback */
  CALADAN3_IPV6_USR_ADDR_0           = 0x48, /**< IPv6 Filter 0 */
  CALADAN3_IPV6_USR_ADDR_1           = 0x49, /**< IPv6 Filter 1 */
  CALADAN3_IPV6_USR_ADDR_2           = 0x4A, /**< IPv6 Filter 2 */
  CALADAN3_IPV6_USR_ADDR_3           = 0x4B, /**< IPv6 Filter 3 */
  CALADAN3_TCP_OR_UDP_DP_EQUAL_SP    = 0x50, /**< TCP/UDP SP=DP */
  CALADAN3_TCP_SQ_EQ_ZERO_AND_FLAG_ZERO         = 0x51, /**< TCP Sequence number is 0
                                                             and the Flag is 0 */
  CALADAN3_TCP_SQ_EQ_ZERO_AND_FIN_URG_PSH_ZERO  = 0x52, /**< If the TCP sequence number is 0
                                                             and the FIN, URG & PSH is 0 */
  CALADAN3_TCP_SYN_AND_FIN_BOTH_SET = 0x53, /**< If the TCP SYN and FIN bits are both set */
  CALADAN3_L4_TINY_FRAG             = 0x54, /**< If FO=0 and PROTOCOL=TCP and TRANSPORTLEN
                                                 PP_XXXX.TFRAG_MIN or if FO=1 and PROTOCOL=TCP */
  CALADAN3_L4_SYN_SPORT_LT_1024     = 0x55, /**< Protocol is TCP, SYN bit is set and SP < 1024 */
  CALADAN3_MAX_HW_EXC
} c3PpExcType_t;

#endif /* _CALADAN3_PP_H_ */

