/* $Id: ppc_api_trap_mgmt.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_PPC_API_TRAP_MGMT_INCLUDED__
/* { */
#define __SOC_PPC_API_TRAP_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_frwrd_mact.h>
#include <soc/dpp/TMC/tmc_api_packet.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Value used to assign discard action to trapped packet.
 *     Use as detailed in the APIs refering to this definition. */
#define  SOC_PPC_TRAP_ACTION_PKT_DISCARD_ID (0xffffffff)
/*     Value used to assign first cpu destination to trapped packet.
 *     Use as detailed in the APIs refering to this definition. */
#define  SOC_PPC_TRAP_ACTION_PKT_CPU_ID (0xffffffff - 1)
/*     Value used to assign discard actio to trapped packet for learned cases.
 *     Used only for ARAD PLUS and below in case of egress learning
 *     Use as detailed in the APIs refering to this definition. */
#define  SOC_PPC_TRAP_ACTION_PKT_DISCARD_AND_LEARN_ID (0xffffffff - 2)
/*     Value used to disable egress trap.
*     Use as detailed in the APIs refering to this definition. */
#define  SOC_PPC_TRAP_EG_NO_ACTION       (0xffffffff)
/*     Maximum buffer size for events.                         */
#define  SOC_PPC_TRAP_EVENT_BUFF_MAX_SIZE (5)

/* Number of user-defines for ITMH Snoop field parsing */
#define SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM   (16)

/* user define trap used for drop */
#define SOC_PPC_TRAP_CODE_USER_DEFINED_DROP_TRAP  /* SOC_PPC_TRAP_CODE_USER_DEFINED_43 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 0 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM) 
#define SOC_PPC_TRAP_CODE_USER_DEFINED_DFLT_TRAP  /* SOC_PPC_TRAP_CODE_USER_DEFINED_42 */ \
            (SOC_PPC_TRAP_CODE_USER_DEFINED_LAST - 1 - SOC_PPC_TRAP_CODE_NOF_USER_DEFINED_SNOOPS_FOR_TM) 

/* Max Snoop strength */
#define PPC_TRAP_MGMT_SNP_STRENGTH_MAX                  (3)
#define PPC_EG_TRAP_SHIFT_SIZE                          (16)
/* Used to specify the length of the arrays of fields per core in SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO */
#define MAX_NUM_OF_CORES_EGRESS_ACTION_PROFILE          (2)

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

/*  Prefixes,  IHB_FWD_ACT_PROFILE table offsets*/
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX  (0x0<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX  (0x1<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX  (0x2<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX  (0x3<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX  (0x4<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT2_PREFIX  (0x5<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX  (0x6<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX  (0x7<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX  (0x8<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX  (0x9<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX  (0xa<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_PMF_PREFIX   (0xb<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX   (0xc<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_IHP_TIMNA_PREFIX (0xd<<4)
#define SOC_PPC_TRAP_CODE_INTERNAL_USER_PREFIX      ((0xf<<4) | (0x0 << 3))
#define SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_PREFIX     ((0xf<<4) | (0x1 << 3))
#define SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX       (0xe<<4)




typedef enum
{
  /*  Link-Layer Cpu-Trap-Codes */
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_SA_DROP0                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_SA_DROP1                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_SA_DROP2                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_SA_DROP3                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_TE_TRANSPLANT                    = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_TE_UNKNOWN_TUNNEL                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_TRANSPLANT                       = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PBP_LEARN_SNOOP                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_SA_AUTHENTICATION_FAILED             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_PORT_NOT_PERMITTED                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0x9,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_UNEXPECTED_VID                       = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0xa,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_SA_MULTICAST                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0xb,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_SA_EQUALS_DA                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0xc,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_8021X                                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0xd,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ACCEPTABLE_FRAME_TYPE0               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0xe,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ACCEPTABLE_FRAME_TYPE1               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR0_PREFIX + 0xf,

  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ACCEPTABLE_FRAME_TYPE2               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ACCEPTABLE_FRAME_TYPE3               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_MY_ARP                               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ARP                                  = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_IGMP_MEMBERSHIP_QUERY                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_IGMP_REPORT_LEAVE_MSG                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_IGMP_UNDEFINED                       = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ICMPV6_MLD_MC_LISTENER_QUERY         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_RESERVED_MC_PREFIX                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR1_PREFIX + 0x8, /*  prefix */

  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ICMPV6_MLD_REPORT_DONE_MSG           = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_ICMPV6_MLD_UNDEFINED                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_DHCP_SERVER                          = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_DHCP_CLIENT                          = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_DHCPV6_SERVER                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_DHCPV6_CLIENT                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_GENERAL_TRAP0                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_GENERAL_TRAP1                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_GENERAL_TRAP2                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_LLR_GENERAL_TRAP3                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_LLR2_PREFIX + 0x9,

    /*  VTT Cpu-Trap-Codes */
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_PORT_NOT_VLAN_MEMBER                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_HEADER_SIZE_ERR                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MY_B_MAC_AND_LEARN_NULL              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MY_B_DA_UNKNOWN_I_SID                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MY_MAC_AND_IP_DISABLE                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MY_MAC_AND_MPLS_DISABLE              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MY_MAC_AND_ARP                       = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MY_MAC_AND_UNKNOWN_L3                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_IP_COMP_MC_INVALID_IP                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_VERSION                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0x9,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_INVALID_TTL                    = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0xa,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_CHBH                           = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0xb,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_NO_REVERSE_FEC                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0xc,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_CITE                           = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0xd,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_ILLEGAL_INNER_MC               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0xe,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_DISABLE_BRIDGE_IF_DESIGNTAED   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT0_PREFIX + 0xf,

  /* changed for ARAD */
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_INVALID_LIF_ENTRY                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_UNEXPECTED_BOS                    = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_UNEXPECTED_NO_BOS                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_TTL_0                             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_TTL_1                             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_CONTROL_WORD_TRAP                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_CONTROL_WORD_DROP                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_STP_STATE_BLOCK                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_STP_STATE_LEARN                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_ILLEGEL_PFC                            = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0x9,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_IPV4_TUNNEL_TERMINATION_AND_FRAGMENTED = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0xA,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_ETHER_IP_VERSION_ERROR                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0xB,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_PROTECTION_PATH_INVALID                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0xC,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_TRILL_NO_ADJACENT                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0xD,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MPLS_TERMINATION_ERR                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0xE,
  SOC_PPC_TRAP_CODE_INTERNAL_VTT_MY_B_MAC_MC_CONTINUE                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT1_PREFIX + 0xF,


  /*Jericho only*/
  SOC_PPC_TRAP_CODE_INTERNAL_MPLS_ILLEGAL_LABEL                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT2_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_INNER_IP_COMP_MC_INVALID_IP_0              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT2_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_INNER_ETHERNET_MY_MAC_IP_DISABLED         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_VTT2_PREFIX + 0x8,

  /*  Forwarding-Lookup Cpu-Trap-Codes */
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_L2CP_PEER                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_L2CP_DROP                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_FL_IGMP_MEMBERSHIP_QUERY         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_FL_IGMP_REPORT_LEAVE_MSG         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_FL_IGMP_UNDEFINED                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_FL_ICMPV6_MLD_MC_LISTENER_QUERY  = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_FL_ICMPV6_MLD_REPORT_DONE        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ETH_FL_ICMPV6_MLD_UNDEFINED          = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_VERSION_ERROR                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_CHECKSUM_ERROR                  = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0x9,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_HEADER_LENGTH_ERROR             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0xa,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_TOTAL_LENGTH_ERROR              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0xb,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_TTL0                            = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0xc,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_HAS_OPTIONS                     = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0xd,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_TTL1                            = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0xe,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_SIP_EQUAL_DIP                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP0_PREFIX + 0xf,

  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_DIP_ZERO                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV4_SIP_IS_MC                       = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_VERSION_ERROR                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_HOP_COUNT0                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_HOP_COUNT1                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_UNSPECIFIED_DESTINATION         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_LOOPBACK_ADDRESS                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_MULTICAST_SOURCE                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_NEXT_HEADER_NULL                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_UNSPECIFIED_SOURCE              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0x9,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_LOCAL_LINK_DESTINATION          = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0xa,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_LOCAL_SITE_DESTINATION          = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0xb,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_LOCAL_LINK_SOURCE               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0xc,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_LOCAL_SITE_SOURCE               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0xd,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_IPV4_COMPATIBLE_DESTINATION     = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0xe,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_IPV4_MAPPED_DESTINATION         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP1_PREFIX + 0xf,

  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IPV6_MULTICAST_DESTINATION           = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_TTL0                            = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_TTL1                            = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TCP_SN_FLAGS_ZERO                    = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TCP_SN_ZERO_FLAGS_SET                = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TCP_SYN_FIN                          = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TCP_EQUAL_PORTS                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TCP_FRAGMENT_INCOMPLETE_HEADER       = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TCP_FRAGMENT_OFFSET_LT8              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_UDP_EQUAL_PORTS                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0x9,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ICMP_DATA_GT_576                     = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0xa,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ICMP_FRAGMENTED                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0xb,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_DROP0                             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0xc,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_DROP1                             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0xd,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_DROP2                             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0xe,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_DROP3                             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP2_PREFIX + 0xf,

  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_NOT_FOUND0                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_NOT_FOUND1                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_NOT_FOUND2                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SA_NOT_FOUND3                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x3,

  SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_UNKNOWN_LABEL                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_P2P_MISCONFIGURATION                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ARP                                  = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_FCF                                  = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x7,



  SOC_PPC_TRAP_CODE_INTERNAL_FLP_IEEE_1588_PREFIX                     = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP3_PREFIX + 0x8, /*  prefix */


  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ELK_ERROR                            = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_ELK_REJECTED                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_LEVEL                               =  SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x2, /* used for OAM cpu trap, not configured in HW */
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_SAME_INTERFACE                       = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_UC                     = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_MC                     = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_UC_LOOSE_RPF_FAIL                    = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x7,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_MCV6                         = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x8,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_P2P_NO_BOS                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0x9,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_CONTROL_WORD_TRAP               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0xa,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_CONTROL_WORD_DROP               = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0xb,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PASSIVE                              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0xc, /* used for OAM cpu trap, not configured in HW */
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_P2P_MPLSX4                      = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0xd,
  SOC_PPC_TRAP_CODE_INTERNAL_FLP_FABRIC_PROVIDED_SECURITY             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FLP4_PREFIX + 0xe,


  /*  PMF Cpu-Trap-Codes */
  SOC_PPC_TRAP_CODE_INTERNAL_PMF_GENERAL                              = SOC_PPC_TRAP_CODE_INTERNAL_IHP_PMF_PREFIX + 0x0,


  /*  Fec-Resolution Cpu-Trap-Codes */
  SOC_PPC_TRAP_CODE_INTERNAL_FER_FACILITY_INVALID                     = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX  + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_FER_FEC_ENTRY_ACCESSED                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX  + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_FER_UC_STRICT_RPF_FAIL                   = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX  + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_FER_MC_EXPLICIT_RPF_FAIL                 = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX  + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_FER_MC_USE_SIP_AS_IS_RPF_FAIL            = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX  + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_FER_SIP_TRANSPLANT_DETECTION             = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX  + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_FER_ICMP_REDIRECT                        = SOC_PPC_TRAP_CODE_INTERNAL_IHP_FER_PREFIX  + 0x6,



  /*  External device */
  SOC_PPC_TRAP_CODE_INTERNAL_USER_OAMP                                = SOC_PPC_TRAP_CODE_INTERNAL_USER_PREFIX     + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_USER_ETHERNET_OAM_ACCELERATED            = SOC_PPC_TRAP_CODE_INTERNAL_USER_PREFIX     + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_USER_MPLS_OAM_ACCELERATED                = SOC_PPC_TRAP_CODE_INTERNAL_USER_PREFIX     + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_USER_BFD_IP_OAM_TUNNEL_ACCELERATED       = SOC_PPC_TRAP_CODE_INTERNAL_USER_PREFIX     + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_USER_BFD_PWE_OAM_ACCELERATED             = SOC_PPC_TRAP_CODE_INTERNAL_USER_PREFIX     + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_USER_ETHERNET_OAM_UP_ACCELERATED         = SOC_PPC_TRAP_CODE_INTERNAL_USER_PREFIX     + 0x5,

  /* Block traps */
  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_TRAP_VTT_VT                        =  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_TRAP_VTT_TT                        =  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_TRAP_FLP                           =  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_TRAP_PMF_1ST_PASS                  =  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_TRAP_PMF_2ND_PASS                  =  SOC_PPC_TRAP_CODE_INTERNAL_BLOCK_PREFIX + 0x4,
  
  /* OAM traps */
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_ETH_OAM                         =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x0,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_Y1731_O_MPLSTP                  =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x1,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_Y1731_O_PWE                     =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x2,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_BFD_O_IPV4                      =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x3,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_BFD_O_MPLS                      =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x4,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_BFD_O_PWE                       =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x5,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_BFDCC_O_MPLSTP                  =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x6,
  SOC_PPC_TRAP_CODE_INTERNAL_OAM_BFDCV_O_MPLSTP                  =  SOC_PPC_TRAP_CODE_INTERNAL_OAM_PREFIX + 0x7



} SOC_PPC_TRAP_CODE_INTERNAL;


/* $Id: ppc_api_trap_mgmt.h,v 1.14 Broadcom SDK $
 *	FTMH destination extension (sometimes called CUD/OUT_LIF)
 */
typedef enum
{
  /*
   *	Will never appear.
   */
  SOC_PPC_TRAP_MGMT_FTMH_DEST_EXT_MODE_NEVER,
  /*
   *	Appears on Multicast packets.
   */
  SOC_PPC_TRAP_MGMT_FTMH_DEST_EXT_MODE_MC,
  /*
   *  Always appears.
   */
  SOC_PPC_TRAP_MGMT_FTMH_DEST_EXT_MODE_ALWAYS
} SOC_PPC_TRAP_MGMT_FTMH_DEST_EXT_MODE;

typedef enum
{
  /*
   * packet is normally forwarded
   */
  SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE_NORMAL,
  /*
   * packet was trapped
   */
  SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE_TRAP,
  /*
   * packet was snooped
   */
  SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE_SNOOP,
  /*
   * packet was mirrored
   */
  SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE_IN_MIRROR,
  /*
   *	Always appears.
   */
  SOC_PPC_NOF_TRAP_MGMT_PKT_FRWRD_TYPES
} SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE;

typedef enum
{
  /*
   *  Snoop 64 bytes from the packet.
   */
  SOC_PPC_TRAP_SNOOP_ACTION_SIZE_64B = 0,
  /*
   *  Snoop 128 bytes from the packet.
   */
  SOC_PPC_TRAP_SNOOP_ACTION_SIZE_128B = 1,
  /*
   *  Snoop the full packet.
   */
  SOC_PPC_TRAP_SNOOP_ACTION_SIZE_FULL_PACKET = 2,
  /*
   *  Number of types in SOC_PPC_TRAP_SNOOP_ACTION_SIZE
   */
  SOC_PPC_NOF_TRAP_SNOOP_ACTION_SIZES = 3
}SOC_PPC_TRAP_SNOOP_ACTION_SIZE;

typedef enum
{
  /*
   *  None of the optional fields of the forwarding action is
   *  overwritten.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_NONE = 0,
  /*
   *  Overwrite the packet destination.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DEST = 0x1,
  /*
   *  Overwrite the packet TC.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_TC = 0x2,
  /*
   *  Overwrite the packet DP
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DP = 0x4,
  /*
   *  Overwrite the policer associated with the packet
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_POLICER = 0x8,
  /*
   *  Overwrite the packet forwarding offset. Relevant only
   *  for forwarding action.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_FRWRD_OFFSET = 0x10,
  /*
   *  Overwrite the packet CUD. Relevant only for Egress
   *  action.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_CUD = 0x20,
  /*
   *  Overwrite the packet meter command.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DP_METER_CMD = 0x40,
  /*
   *  All of the optional fields of the forwarding action are
   *  overwritten.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE
   */
  SOC_PPC_NOF_TRAP_ACTION_PROFILE_OVERWRITES_PB = 8,
  /*
   *  Overwrite the packet destination additional info (EEI)
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DEST_EEI = 0x80,
  /*
   *  Overwrite the packet destination additional info (outlif)
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DEST_OUTLIF = 0x100,
  /*
   *  Overwrite the packet forward type.
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_FRWRD_TYPE = 0x200,
  /*
   *  Overwrite da type, UC/MC/BC, affects policing
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DA_TYPE = 0x400,
  /*
   *  Overwrite meter-0
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_METER_0 = 0x800,
  /*
   *  Overwrite meter-1
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_METER_1 = 0x1000,
  /*
   *  Overwrite counter-0
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_COUNTER_0 = 0x2000,
  /*
   *  Overwrite counter-1
   *  Arad only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_COUNTER_1 = 0x4000,
  /*
   *  Overwrite pp-dsp
   *  jericho and above only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_PP_DSP = 0x8000,
  /*
   *  if set, destinations will be overwritten from dests arr.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_USE_ARR = 0x10000,
  /*
   *  Overwrite cos-profile
   *  Jericho and above only.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_COS_PROFILE = 0x20000,
  /*
   *  Number of types in SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE ARAD
   */
 SOC_PPC_NOF_TRAP_ACTION_PROFILE_OVERWRITES_ARAD = 21
}SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE;

typedef enum
{
  /*
   *  No valid event is found.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_NONE = 0,
  /*
   *  Entry has been aged out.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_AGED_OUT = 1,
  /*
   *  Entry has been learned.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_LEARN = 2,
  /*
   *  Entry has been transplanted, i.e. key already exit in
   *  MACT but learnt with new value.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_TRANSPLANT = 3,
  /*
   *  Entry has been refreshed, i.e. key and value already
   *  exit in MACT but age status has been updated.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_REFRESH = 4,
  /*
   *  Acknowledgment event upon CPU request
   *  (insert/learn/delete)
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_ACK = 5,
  /*
   *  MACT entry returned upon CPU request using
   *  soc_ppd_frwrd_mact_traverse() with action type RETRIEVE. T20E
   *  only.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_RETRIEVE = 6,
  /*
   *  Insertion to MACT failed due to exceeding the limit
   *  assigned to the particular VSI. T20E only.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_LIMIT_EXCEED = 7,
  /*
   *  CPU request to the MACT has failed. T20E only.
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE_CPU_REQ_FAIL = 8,
  /*
   *  Number of types in SOC_PPC_TRAP_MACT_EVENT_TYPE
   */
  SOC_PPC_NOF_TRAP_MACT_EVENT_TYPES = 9
}SOC_PPC_TRAP_MACT_EVENT_TYPE;


typedef enum
{
  /*
   *    Egress snoop from Egress Field Processor profile 0
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_0,  
  /*
   *    Egress snoop from Egress Field Processor profile 1
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_1,  
   /*
   *    Egress snoop from Egress Field Processor profile 2
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_2, 
   /*
   *    Egress snoop from Egress Field Processor profile 3
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_3, 
   /*
   *    Egress snoop from Egress Field Processor profile 4
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_4, 
   /*
   *    Egress snoop from Egress Field Processor profile 5
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_5, 
   /*
   *    Egress snoop from Egress Field Processor profile 6
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_6, 
   /*
   *    Egress snoop from Egress Field Processor profile 7
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_7, 
   /*
   *    Egress snoop from Egress Field Processor profile 8
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_8, 
   /*
   *    Egress snoop from Egress Field Processor profile 9
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_9, 
   /*
   *    Egress snoop from Egress Field Processor profile 10
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_10, 
   /*
   *    Egress snoop from Egress Field Processor profile 11
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_11, 
   /*
   *    Egress snoop from Egress Field Processor profile 12
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_12, 
   /*
   *    Egress snoop from Egress Field Processor profile 13
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_13, 
   /*
   *    Egress snoop from Egress Field Processor profile 14
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_14, 
   /*
   *    Egress snoop from Egress Field Processor profile 15
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_FIELD_SNOOP_15, 
   /*
   *    Egress snoop from Egress Field Processor profile 15
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_OUT_VPORT_DISCARD,  
   /*
   *    Terminated Inner-My-MAC over IP, but routing is disabled for InRIF.
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_STP_STATE_FAIL,  
   /*
   *    Terminated Inner-My-MAC over IP, but routing is disabled for InRIF.
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_PROTECTION_PATH_UNEXPECTED,  
   /*
   *    Terminated Inner-My-MAC over IP, but routing is disabled for InRIF.
   *    Jericho only
   */
  SOC_PPC_TRAP_CODE_ETPP_VPORT_LOOKUP_FAIL, 
  /*
   *  Terminated MTU value per Out-LIF higher than Enabled
   *  Jericho-B0 and above
   */   
  SOC_PPC_TRAP_CODE_ETPP_MTU_FILTER,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_ETPP_ACC_FRAME_TYPE,
  /*
   *
   */
  SOC_PPC_TRAP_CODE_ETPP_SPLIT_HORIZON,

  /* number of ETPP traps */
  SOC_PPC_NOF_TRAP_ETPP_TYPES_ARAD

}SOC_PPC_TRAP_ETPP_TYPE;




typedef enum
{
  /*
   *
   */
  SOC_PPC_TRAP_EG_TYPE_NONE = 0x0,
  /*
   *  Mapping system VSI to Local VSI is not valid
   */
  SOC_PPC_TRAP_EG_TYPE_NO_VSI_TRANSLATION = 0x1,
  /*
   *  VSI membership filter (analogical to vlan membership) i.e.
   *  packet transmitted out from port that is not member in
   *  the packet's VSI.
   *  Soc_petra-B: if VSI updated upon egress encapsulation
   *  and the out-port is not member in this VSI then the packet
   *  will be dropped, regardless the setting of egress action profile
   */
  SOC_PPC_TRAP_EG_TYPE_VSI_MEMBERSHIP = 0x2,
  /*
   *  Packet frame type is unacceptable see
   *  soc_ppd_eg_filter_port_acceptable_frames_set()
   *  if packet tag structure is not accepted then packet will
   *  be dropped regardless the setting of egress action profile
   */
  SOC_PPC_TRAP_EG_TYPE_ACC_FRM = 0x4,
  /*
   *  Hair-pin filtering, packet is filtered if Source
   *  interface equals destination interface.
   *  in egress filtering referred by SOC_PPC_EG_FILTER_PORT_ENABLE_SAME_INTERFACE
   */
  SOC_PPC_TRAP_EG_TYPE_HAIR_PIN = 0x8,
  /*
   *  (Ethernet Filter) for bridged packet when packet's DA
   *  not found in the MACT.
   */
  SOC_PPC_TRAP_EG_TYPE_UNKNOWN_DA = 0x10,
  /*
   *  Packet is transmitted from Hub to Hub
   */
  SOC_PPC_TRAP_EG_TYPE_SPLIT_HORIZON = 0x20,
  /*
   *  Packet is transmitted from Isolated port to not
   *  promiscuous port
   */
  SOC_PPC_TRAP_EG_TYPE_PRIVATE_VLAN = 0x40,
  /*
   *  (for router packets) Packet's TTL is lower than the
   *  configured value
   */
  SOC_PPC_TRAP_EG_TYPE_TTL_SCOPE = 0x80,
  /*
   *  (for router packets) Packet's MTU is higher than the
   *  configured value (see soc_ppd_port_info_set)
   */
  SOC_PPC_TRAP_EG_TYPE_MTU_VIOLATION = 0x100,
  /*
   *  (TRILL) packet's TTL is equal to zero.
   */
  SOC_PPC_TRAP_EG_TYPE_TRILL_TTL_0 = 0x200,
  /*
   *  (TRILL) Packet sent back to where it came from (from/to
   *  same Adjacent-RBridge)
   */
  SOC_PPC_TRAP_EG_TYPE_TRILL_SAME_INTERFACE = 0x400,
  /*
   *  (TRILL) This filter is used in order to prevent bounce
   *  back of trill terminated unicast packets by the egress
   *  router bridge in case of a DA not found.
   */
  SOC_PPC_TRAP_EG_TYPE_TRILL_BOUNCE_BACK = 0x800,
  /*
   * in stacking: packet is to be forwarded to
   * to TM-domain marked to be filtered.
   */
  SOC_PPC_TRAP_EG_TYPE_DSS_STACKING	= 0x1000,
  /*
   * packet sent to port where packet's LB key
   * doesn't fill in the LB-range of the port
   */
  SOC_PPC_TRAP_EG_TYPE_LAG_MULTICAST = 0x2000,
  /*
   * packet forwarded back to incoming port (considering LAG) 
   * In Arad, SOC_PPC_TRAP_EG_TYPE_HAIR_PIN is used instead, 
   * even for LAG 
   */
  SOC_PPC_TRAP_EG_TYPE_EXCLUDE_SRC = 0x4000,
  /*
   * packet with out-AC, where AC. CFM-valid and
   * AC. CFM-Max-Level >= packet. CFM-Level
   */
  SOC_PPC_TRAP_EG_TYPE_CFM_TRAP = 0x8000,
  /*
   *  Use to set all traps
   */
  SOC_PPC_TRAP_EG_TYPE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_TRAP_EG_TYPE
   */
  SOC_PPC_NOF_TRAP_EG_TYPES_PB = 15,
  /*
   * invalid out-port
   */
  SOC_PPC_TRAP_EG_TYPE_INVALID_OUT_PORT = (1 << PPC_EG_TRAP_SHIFT_SIZE), /* 0x10000, */
  /*
   * CNM packet
   */
  SOC_PPC_TRAP_EG_TYPE_CNM_PACKET = (2 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IPv4 version error
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IPV4_VERSION_ERROR  = (3 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IPv4 header checksum error
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IPV4_CHECKSUM_ERROR = (4 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IPv4 header length is < 20B
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IPV4_HEADER_LENGTH_ERROR = (5 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IPv4 total length is < 20B
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IPV4_TOTAL_LENGTH_ERROR = (6 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IP TTL=0
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IP_TTL0 = (7 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IP with options
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IP_HAS_OPTIONS = (8 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IP TTL=1
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IP_TTL1 = (9 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  SIP equal DIP
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IPV4_SIP_EQUAL_DIP = (10 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  IPv4 DIP is 0.0.0.0
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IPV4_DIP_ZERO = (11 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   * . SIP is multicast
   *  ARAD only.
   *  not relevant for Soc_petra-B.
   */
  SOC_PPC_TRAP_EG_TYPE_IPV4_SIP_IS_MC = (12 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  Forwarding header DIP = ::
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_UNSPECIFIED_DESTINATION = (13 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  DIP = ::1 or SIP = ::1
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_LOOPBACK_ADDRESS = (14 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  The MSB of the SIP = 0xFF
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_MULTICAST_SOURCE = (15 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  SIP = ::
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_UNSPECIFIED_SOURCE = (16 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  Bits 127:118 of the destination-IP are
   *  equal to 0x3FA
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_LOCAL_LINK_DESTINATION = (17 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  Bits 127:118 of the DIP = 0x3FB
   *  (deprecated)
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_LOCAL_SITE_DESTINATION = (18 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  Bits 127:118 of the SIP = 0x3FA
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_LOCAL_LINK_SOURCE = (19 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  Bits 127:118 of the SIP = 0x3FB
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_LOCAL_SITE_SOURCE = (20 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  Bits 127:32 of the DIP = 0
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_IPV4_COMPATIBLE_DESTINATION = (21 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  Bits 127:32 of the DIP are equal to
   *  0000_FFF_0000_0000_0000_0000
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_IPV4_MAPPED_DESTINATION = (22 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  MSB of the DIP=0xFF
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_MULTICAST_DESTINATION = (23 << PPC_EG_TRAP_SHIFT_SIZE),
  /*
   *  hob-by-hop address
   */
  SOC_PPC_TRAP_EG_TYPE_IPV6_HOP_BY_HOP  = (24 << PPC_EG_TRAP_SHIFT_SIZE),
  /* Highest value for verification */
  SOC_PPC_TRAP_EG_TYPE_HIGHEST_ARAD  = SOC_PPC_TRAP_EG_TYPE_IPV6_HOP_BY_HOP,

  SOC_PPC_NOF_TRAP_EG_TYPES_ARAD = 39
  
}SOC_PPC_TRAP_EG_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *	FTMH destination extension.
   */
  SOC_PPC_TRAP_MGMT_FTMH_DEST_EXT_MODE dest_ext_mode;
  /*
   *	FTMH load balancing extension enable. Relevant only if
   *  ftmh_mode_is_petra_B is TRUE.
   */
  uint8 lb_ext_en;
  /*
   *	Internal Mode. If true, Soc_petra-B. Else, Soc_petra-A.
   */
  uint8 internal_eep_ext;
  /*
   *	Stacking enabled.
   */
  uint8 stacking_enabled;
  /*
   *	TDM enabled.
   */
  uint8 tdm_enabled;
  /*
   * Size of user-headers between the system-headers 
   * and the network headers. Arad-only. Units: bytes.
   */
  uint32 user_header_size;
  /*
   *	receive mode (MSB to lsb or vice versa).
   */
  SOC_TMC_PKT_PACKET_RECV_MODE recv_mode;
} SOC_PPC_TRAP_MGMT_PACKET_PARSE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  is the packet designated to the OAM processor. T20E
   *  only.
   */
  uint8 is_oam_dest;
  /*
   *  Destination to forward the packet to,
   *  only type and dest_id are relevant.
   *  frwrd_dest.additional_info is not relevant
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_dest;
  /*
   *  Indicates that the VSI value should be added to the
   *  Forward-Destination, i.e., the Forward-Destination above
   *  is to be treated as a base value. This is useful to
   *  define the default VSI forwarding. Note: This considers
   *  the local VSI and not the system VSI. For Snoop action,
   *  this has to be FALSE.
   */
  uint8 add_vsi;
  /*
   *  When add_vsi is TRUE, the packets belonging to VSI v
   *  will be forwarded to destination: dest + (v <<
   *  vsi_shift).
   *  Range: 0 - 3.
   */
  uint32 vsi_shift;

} SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  For each packet, up to two counters may be performed.
   *  Thus, two counter pointers are associated with each
   *  packet-this selects one of these pointers. Range: 0 - 1.
   *  not relevant for Arad.
   */
  uint32 counter_select;
  /*
   *  The counter to associate with the packets. Range: 0 -
   *  4095.
   */
  uint32 counter_id;

} SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  For each packet, up to two meters may be performed.
   *  Thus, two meter pointers are associated with each
   *  packet-this selects one of these pointers. Range: 0 - 1.
   *  not relevant for arad.
   */
  uint32 meter_select;
  /*
   *  The meter to associate with the packets. Range: 0 -
   *  4095.
   */
  uint32 meter_id;
  /*
   *  Identifies how to apply the meter result to the packet
   *  copy made with this action. Used by the Traffic Manager
   *  Action. Range: 0 - 3.
   */
  uint32 meter_command;

} SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ethernet policer to associate with the packets. Range: 0
   *  - 511.
   */
  uint32 ethernet_police_id;

} SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Traffic Class. Range: 0 -7.
   */
  SOC_SAND_PP_TC tc;
  /*
   *  Drop Precedence. Range: 0 -3
   */
  SOC_SAND_PP_DP dp;
} SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Set to FALSE to disable learning for packets assigned
   *  with this action profile. Set to TRUE will enable
   *  learning. In T20E has to be TRUE, since learning cannot
   *  be disabled for Trap action.
   */
  uint8 enable_learning;
  /*
   *  Indicates whether the packet is trapped to CPU. If TRUE
   *  the CPU-Trap-Code and the corresponding
   *  Cpu-Trap-Qualifier is inserted into the packet header.
   */
  uint8 is_trap;
  /*
   *  Indicates that packet is control packet. If TRUE, packet
   *  bypasses all Egress filters
   */
  uint8 is_control;
  /*
   *  Index in the packet
   *  header determines the forwarding header of the packet.
   *  Range: 0 - 7.
   */
  uint32 frwrd_offset_index;
  /*
   *  forwarding header type
   *  IPv4, Ipv6, MPLS, Ethernet, 
   *  Arad only.
   */
  SOC_TMC_PKT_FRWRD_TYPE frwrd_type;
  /*
   *  bytes to add to forward offset
   *  header determines the forwarding header of the packet.
   *  Range: -31 - 31.
   */
  int32 frwrd_offset_bytes_fix;
  /*
   *  Packet MAC address type (UC/MC/BC).
   *  will affect policing.
   */
  SOC_SAND_PP_ETHERNET_DA_TYPE da_type;

} SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Value that identifies the strength of the assignment of
   *  Destination parameters to the packet. Through the packet
   *  processing pipe, many forwarding decisions may be taken;
   *  yet one decision may overwrite a former decision only if
   *  it stronger, i.e., its strength is higher than the last
   *  decision that was taken. Range: 0 - 7.
   */
  uint32 strength;
  /*
   *  The strength of tunnel termination trap
   *  Range: 0 - 7.
   */
  uint32 tunnel_termination_strength;
  /*
   *  Bitmap that indicates which of the fields to overwrite.
   *  If packet is assigned to a profile action that is
   *  stronger than the previous one, then only masked fields
   *  will be taken from this action.
   *  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE details what the
   *  meaning of each bit in the bitmap. For example to
   *  overwrite destination and dp set bitmap_mask =
   *  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DEST |
   *  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DP;
   */
  uint32 bitmap_mask;
  /*
   *  Information to determine the Destination, according to
   *  which to forward packets.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO dest_info;
  /*
   *  COS parameters (TC and DP) to assign to packets.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO cos_info;
  /*
   *  Counting information includes counters to assign to
   *  packets.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO count_info;
  /*
   *  Metering information includes meters to assign to
   *  packets.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO meter_info;
  /*
   *  Policing information includes Ethernet policer to assign
   *  to packets.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO policing_info;
  /*
   *  Processing information, including whether to perform
   *  learning/filtering/Trapping to the packet.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO processing_info;

} SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Value that identifies the strength of the assignment of
   *  Destination parameters to the packet. Through the packet
   *  processing pipe, many forwarding decisions may be taken;
   *  yet one decision may overwrite a former decision only if
   *  it stronger, i.e., its strength is higher than the last
   *  decision that was taken. Range: 0 - 7.
   */
  uint32 strength;
  /*
   *  The strength of tunnel termination trap
   *  Range: 0 - 7.
   */
  uint32 tunnel_termination_strength;
   /*
   *  Snoop command set in the system
   *  headers and sent to Soc_petra. The snooping is configured
   *  and performed by the Soc_petra-TM. Snoop command zero means
   *  no Snooping, to configure the snoop command use Soc_petra-TM
   *  api, see Soc_petra-TM UM. Range: Soc_petraB: 0-15. T20E: 0-3.
   */
  uint32 snoop_cmnd;

} SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO;

/* Additional Header Data (Format of TRAP field) */ 
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* Copy Unique Data (Out-LIF or ACE pointer) - 16 bits (15:0) */
  uint32 cud;

  /* DSP pointer (equal to TM-port) - 8 bits (23:16) */
  uint8 dsp_ptr;

  /* COS Profile - 4 bits (27:24) */
  uint8 cos_profile;

  /* Mirror Profile - 4 bits (31:28) */
  uint8 mirror_profile;

} SOC_PPC_TRAP_EG_ACTION_HDR_DATA;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Bitmap that indicates which of the fields to overwrite.
   *  If packet is assigned to a profile action that is
   *  stronger than the previous one, then only masked fields
   *  will be taken from this action.
   *  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE details what the
   *  meaning of each bit in the bitmap. For example to
   *  overwrite destination and dp set bitmap_mask =
   *  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DEST |
   *  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_DP;
   */
  uint32 bitmap_mask;
  /*
   *  The local TM port to send the packet out through. Set to
   *  SOC_PPC_TRAP_ACTION_PKT_DISCARD_ID in order to discard
   *  packets.
   */
  SOC_PPC_TM_PORT out_tm_port;
   /*
   *  
   *  SOC_PPC_TM_PORT arr
   */
  SOC_PPC_TM_PORT out_tm_port_arr[MAX_NUM_OF_CORES_EGRESS_ACTION_PROFILE];
  /*
   *  COS parameters (TC and DP) to assign to packets.
   */
  SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO cos_info;
  /*
   *  Copy unique data of the packet, Range: 0 - 0xFFFF.
   */
  uint32 cud;

  /*SOC_PPC_TRAP_EG_ACTION_CUD cud;*/
  SOC_PPC_TRAP_EG_ACTION_HDR_DATA header_data;

  /*SOC_PPC_TRAP_EG_ACTION_CUD cud arr*/
  SOC_PPC_TRAP_EG_ACTION_HDR_DATA header_data_arr[MAX_NUM_OF_CORES_EGRESS_ACTION_PROFILE];

} SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  mirror command
   */
  uint32 mirror_cmd;
  /*
   *  mirror strength
   */
  uint32 mirror_strength;
  /*
   *  mirror enable
   */
  uint32 mirror_enable;
  /*
   *  forward strength
   */
  uint32 fwd_strength;
  /*
   *  forward enable
   */
  uint32 fwd_enable;

} SOC_PPC_TRAP_ETPP_INFO;



typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  MACT event Type (aged-out/refresh/learn etc...)
   */
  SOC_PPC_TRAP_MACT_EVENT_TYPE type;
  /*
   *  This may be union of many types: learning information,
   *  VSI, VID or packet headers.
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY key;
  /*
   *  This may be union of many types: learning information,
   *  VSI, VID or packet headers.
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE val;
  /*
   *  Whether the Event was generated from a packet whose
   *  Destination is a LAG.
   */
  uint8 is_part_of_lag;

} SOC_PPC_TRAP_MACT_EVENT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  
  /*
   * packet type: snooped, trapped, mirrored or normally forwarded..
   */
   SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE frwrd_type;
  /*
   * is trapped packet.
   * TRUE: then packet is trapped to CPU and trap-code/qualifier are valid
   * FALSE: then packet is forwarded (Bridged, Routed,...)  to CPU.
   */
   uint8 is_trapped;
  /*
   * is punted packet.
   * TRUE: then packet was punted to CPU by OAMP
   * FALSE: then packet was not punted by OAMP
   */
   uint8 is_punted;
  /*
   *  Each packet that is trapped to the CPU from the PP
   *  engine is attached with a trap code ID, specifying the
   *  location in the processing Pipe that causes the
   *  TRAP. Range: 0 - 255.
   */
  SOC_PPC_TRAP_CODE cpu_trap_code;
  /*
   *  Additional information on top of the Trap-Code, e.g, the
   *  VSI that the packet was assigned to. Range: 0 - 4095.
   */
  uint32 cpu_trap_qualifier;
  /*
   *  relevant only if packet snooped, 2 lsbs are masked
   */
  uint32 snoop_cmnd;
  /*
   *  The source system port of the packet. Useful to conclude
   *  the device that originated the packet.
   */
  uint32 src_sys_port;
  /*
   *  packet total size in bytes (including system headers if exist, NIF CRC)
   */
  uint32 size_in_bytes;
  /*
   *  Offset to the Network header /link-layer Ethernet header of the trapped
   *  packet. Units: Byte. The user may access the Network header Ethernet
   *  header by pointing to (buff + ntwrk_header_ptr)
   */
  uint32 ntwrk_header_ptr;
  /*
   *  Offset to the User headers. The size of the User-headers can be retreived by
   *  substracting ntwrk_header_ptr - user_header_ptr.
   *  Units: Byte. The user may access the User header by pointing to
   *  (buff + user_header_ptr)
   */
  uint32 user_header_ptr[2];
  uint32 internal_inlif_inrif;
  /*
   *  In-AC relevant for Bridged packets
   */
  uint32 in_ac;
  /*
   *  RIF relevant for Routed packets, out-RIF for unicast and In-RIF for multicast 
   */
  uint32 rif;
  /*
   *  VRF relevant for Routed packets in the ARAD 
   */
  uint32 vrf;
  /*
  FTMH:PP_DSP
  */
  uint32 otm_port;
  /*
  FTMH:FTMH.Out-LIF (or MC-ID)
  */
  uint32 outlif_mcid;
  /*
  FTMH LB-Key Extension 
  */
  uint32 lb_key;
  /*
  FTMH DSP Extension 
  */
  uint32 dsp_ext;
  /*
  FTMH stacking Extension 
  */
  uint32 stacking_ext;

  /*
  FTMH TC 
  */
  uint32 tc;
  /*
  FTMH DP 
  */
  uint32 dp;
   /*
  Internal fwd_code 
  */
  uint32 internal_fwd_code;
  /*
  Internal unknown_addr 
  */
  uint32 internal_unknown_addr;

  /*
  Internal fwd_hdr_offset
  */

  uint32 internal_fwd_hdr_offset;

  /*OAM fields*/
  uint32    oam_type;
  uint32    oam_offset;
  uint32    oam_sub_type;
  uint32    mep_type;
  uint32    oam_tp_cmd;
  uint32    oam_ts_encaps;
  uint64    oam_data;
  uint8     ext_mac_1588;    /* 1 bit indicates an external MAC for 1588v2 */

  /* TDM fields*/
  uint8		tdm_pkt_size; /*Field is overwritten by FDT to indicate the size of the TDM packet in bytes */
  uint8		tdm_type; /* 0 = unicast, 1= multicast */
  uint16	tdm_dest_fap_id; /* Unicast only*/
  uint8		tdm_otm_port;	 /* Unicast only*/
  uint32	tdm_mc_id;	 /* MC only*/

} SOC_PPC_TRAP_PACKET_INFO;


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

void SOC_PPC_TRAP_MGMT_PACKET_PARSE_INFO_clear(
        SOC_SAND_OUT SOC_PPC_TRAP_MGMT_PACKET_PARSE_INFO *info
     );

void
  SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO *info
  );

void
  SOC_PPC_TRAP_EG_ACTION_HDR_DATA_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_EG_ACTION_HDR_DATA *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO *info
  );

void
  SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO *info
  );

void
  SOC_PPC_TRAP_ETPP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_ETPP_INFO *info
  );

void
  SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO *info
  );

void
  SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO *info
  );

void
  SOC_PPC_TRAP_MACT_EVENT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_MACT_EVENT_INFO *info
  );

void
  SOC_PPC_TRAP_PACKET_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRAP_PACKET_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_TRAP_SNOOP_ACTION_SIZE_to_string(
    SOC_SAND_IN  SOC_PPC_TRAP_SNOOP_ACTION_SIZE enum_val
  );

const char*
  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE_to_string(
    SOC_SAND_IN  SOC_PPC_TRAP_ACTION_PROFILE_OVERWRITE enum_val
  );

const char*
  SOC_PPC_TRAP_MACT_EVENT_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_TRAP_MACT_EVENT_TYPE enum_val
  );

const char*
  SOC_PPC_TRAP_CODE_to_api_string(
    SOC_SAND_IN  SOC_PPC_TRAP_CODE enum_val
   );

const char*
  SOC_PPC_TRAP_CODE_to_string(
    SOC_SAND_IN  SOC_PPC_TRAP_CODE enum_val
  );

const char*
  SOC_PPC_TRAP_EG_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_TRAP_EG_TYPE enum_val
  );

const char*
  SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_TRAP_MGMT_PKT_FRWRD_TYPE enum_val
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO *info
  );

void
  SOC_PPC_TRAP_EG_ACTION_HDR_DATA_print(
    SOC_SAND_IN  SOC_PPC_TRAP_EG_ACTION_HDR_DATA *info
  );

void
  SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO *info
  );

void
  SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO *info
  );

void
  SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO *info
  );

void
  SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO *info
  );

void
  SOC_PPC_TRAP_MACT_EVENT_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_MACT_EVENT_INFO *info
  );

void
  SOC_PPC_TRAP_PACKET_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRAP_PACKET_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_TRAP_MGMT_INCLUDED__*/
#endif

