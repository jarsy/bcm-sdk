/* $Id: ppc_api_fp.h,v 1.95 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __SOC_PPC_API_FP_INCLUDED__
/* { */
#define __SOC_PPC_API_FP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

#include <soc/dpp/TMC/tmc_api_header_parsing_utils.h>
#include <soc/dpp/TMC/tmc_pmf_pgm_mgmt.h>
#include <soc/dpp/TMC/tmc_api_tcam.h>

#include <soc/dpp/dpp_config_defs.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#ifndef SOC_PPC_FP_NOF_QUALS_PER_DB_MAX
/*     Maximum number of Qualifier fields per Database.        */
#define  SOC_PPC_FP_NOF_QUALS_PER_DB_MAX (32)
#endif /* ifndef SOC_PPC_FP_NOF_QUALS_PER_DB_MAX */

/* 
 * Maximum number of Qualifier fields per PFG. 
 * Use the same number than for Entries for BCM level simplicity 
 */
#define  SOC_PPC_FP_NOF_QUALS_PER_PFG_MAX (SOC_PPC_FP_NOF_QUALS_PER_DB_MAX)

#ifndef SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX
/*     Maximum number of Actions per Database.                 */
#define  SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX (16)
#endif /* ifndef SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX */

/*     Maximum number of field values that can be extracted to
 *     build the action value.                                 */
#define  SOC_PPC_FP_DIR_EXTR_MAX_NOF_FIELDS (16)

/*     Maximum number of Control values.                       */
#define  SOC_PPC_FP_NOF_CONTROL_VALS (8)
/*     Maximum number of Databases.                            */
#define  SOC_PPC_FP_NOF_DBS (128)

/*     Number of cycles in TCAM and Macros.                    */
#define  SOC_PPC_FP_NOF_CYCLES (2)

/*     Number of TCAM Banks.                                 */
#define  SOC_PPC_FP_TCAM_NOF_BANKS (4)

#define SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit)   (SOC_DPP_DEFS_GET(unit, nof_tcam_big_banks) + SOC_DPP_DEFS_GET(unit, nof_tcam_small_banks))
#define SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS         (SOC_DPP_DEFS_MAX(NOF_TCAM_BIG_BANKS) + SOC_DPP_DEFS_MAX(NOF_TCAM_SMALL_BANKS))

/* Maximal Number of programs */
#define SOC_PPC_FP_NOF_PROGS_MAX              32

/* Number of Copy Engeins */
#define SOC_PPC_FP_NOF_CES                     32

/* Number of keys */
#define SOC_PPC_FP_NOF_KEYS                    4

/* Number of lines per small bank */
#define SOC_PPC_TCAM_NOF_LINES_ARAD_SMALL 128

/*     Number of Macros FEM per cycle.                                       */
#define  SOC_PPC_FP_NOF_MACROS (8)

/*     Number of Macros FES per cycle.                                       */
#define  SOC_PPC_FP_NOF_MACRO_SIMPLES (16)

/*     Maxmimum number of TCAM Entries per Database.           */
#define SOC_DPP_DEFS_GET_NOF_ENTRY_IDS(unit)                        (SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit) * SOC_DPP_DEFS_GET(unit, nof_tcam_big_bank_lines) * 2)
#define SOC_DPP_DEFS_MAX_NOF_ENTRY_IDS                              (SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS * SOC_DPP_DEFS_MAX(NOF_TCAM_BIG_BANK_LINES) * 2)

#define SOC_DPP_DEFS_GET_TCAM_DT_MAX_KEY                            SOC_DPP_DEFS_MAX(TCAM_BIG_BANK_KEY_NOF_BITS)

/*     Number of Egress Actions.                                       */
#define  SOC_PPC_FP_NOF_EGRESS_ACTIONS (8)

/*     Number of ingress FLP Actions.                                       */
#define  SOC_PPC_FP_NOF_INGRESS_FLP_ACTIONS ((SOC_PPC_FP_ACTION_TYPE_FLP_END - SOC_PPC_FP_ACTION_TYPE_FLP_START) + 1)
#define  SOC_PPC_FP_NOF_ELK_ACTIONS           ((SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_5 - SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_0) + 1)

/* SLB actions: semantic, to differentiate between pre-hashing and post-hashing keys */
#define  SOC_PPC_FP_NOF_INGRESS_SLB_ACTIONS (2)

/* Number of databases in longs (32b) */
#define SOC_PPC_FP_NOF_DBS_IN_LONGS			4

/* Number of program selection lines in longs (32b) */
#define SOC_PPC_FP_NOF_PS_LINES_IN_LONGS    2

/* Maximum number of Databases per bank */
#define SOC_PPC_FP_MAX_NOF_DBS_PER_BANK     16

/* Number of Program selection lines */
#define SOC_PPC_FP_NOF_PS_LINES             48

/* Number of PFGs */
#define SOC_PPC_FP_NOF_PFGS_ARAD            128

/* Number of Program selection lines in longs */
#define SOC_PPC_FP_NOF_PFGS_IN_LONGS_ARAD             4

/* Max number of keys per Database  */
#define SOC_PPC_FP_KEY_NOF_KEYS_PER_DB_MAX 	2

/*     Maximum number of Copy Engines per Database.        */
#define  SOC_PPC_FP_NOF_CES_PER_DB_MAX (32)

/*     Maximum number of U32-s of packet */
#define  SOC_PPC_FP_IRE_TRAFFIC_BUFFER_SIZE                    (50)

/* Maximum number of parameters in case of error */
#define SOC_PPC_FP_RESOURCE_DIAG_NOF_PARAMS 					10

/* Maximum number of error value types per error type */
#define SOC_PPC_FP_RESOURCE_DIAG_NOF_ERROR_VALUES 		20

/* flag determine if the offset value is negative */
#define SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_FLAG_NEGATIVE      (1 << 31)

/* SOC_PPC_FP_DATABASE_INFO_FLAGS */
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SMALL_BANKS (1 << 0)
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_NO_INSERTION_PRIORITY_ORDER (1 << 1)
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_SPARSE_PRIORITIES     (1 << 2)
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_LSB          (1 << 3)
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_MSB          (1 << 4)

#define SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL              (1 << 5)
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_HANDLE_ENTRIES_BY_KEY (1 << 6)
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_HEADER_SELECTION      (1 << 7)
#define SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SINGLE_BANK       (1 << 8)


#define SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES    (1 << 9) /* databases in the KBP that are defined in init for FLP stage */

#define SOC_PPC_FP_DATABASE_INFO_FLAGS_ALLOCATE_FES     (1 << 11) /* Allocate FES for direct extraction */

#define SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS         (1 << 12) /* using direct access in KAPS */

#define BCM_FIELD_ENTRY_INVALID                         (-1)
/* } */
/*************
 * MACROS    *
 *************/
/* { */
#define SOC_PPC_BIT_TO_U32(nof_bits) (((nof_bits)+31)/32)

/* Qualifier is user defined*/
#define SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(qual_type) \
   (((qual_type >= SOC_PPC_FP_QUAL_HDR_USER_DEF_0) && (qual_type <= SOC_PPC_FP_QUAL_HDR_USER_DEF_LAST))? 0x1: 0x0)
/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */



typedef SOC_TMC_HPU_FTMH                                        SOC_PPC_FP_FTMH;
typedef SOC_TMC_PORT_PP_PORT_INFO                               SOC_PPC_FP_DIAG_PP_PORT_INFO;
typedef SOC_TMC_TCAM_BANK_ENTRY_SIZE                            SOC_PPC_FP_TCAM_KEY_SIZE;

#define SOC_PPC_FP_PKT_HDR_TYPE_RAW                          SOC_TMC_FP_PKT_HDR_TYPE_RAW
#define SOC_PPC_FP_PKT_HDR_TYPE_FTMH                         SOC_TMC_FP_PKT_HDR_TYPE_FTMH
#define SOC_PPC_FP_PKT_HDR_TYPE_TM                           SOC_TMC_FP_PKT_HDR_TYPE_TM
#define SOC_PPC_FP_PKT_HDR_TYPE_TM_IS                        SOC_TMC_FP_PKT_HDR_TYPE_TM_IS
#define SOC_PPC_FP_PKT_HDR_TYPE_TM_PPH                       SOC_TMC_FP_PKT_HDR_TYPE_TM_PPH
#define SOC_PPC_FP_PKT_HDR_TYPE_TM_IS_PPH                    SOC_TMC_FP_PKT_HDR_TYPE_TM_IS_PPH
#define SOC_PPC_FP_PKT_HDR_TYPE_ETH                          SOC_TMC_FP_PKT_HDR_TYPE_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_ETH_ETH                      SOC_TMC_FP_PKT_HDR_TYPE_ETH_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV4_ETH                     SOC_TMC_FP_PKT_HDR_TYPE_IPV4_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV6_ETH                     SOC_TMC_FP_PKT_HDR_TYPE_IPV6_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_MPLS1_ETH                    SOC_TMC_FP_PKT_HDR_TYPE_MPLS1_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_MPLS2_ETH                    SOC_TMC_FP_PKT_HDR_TYPE_MPLS2_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_MPLS3_ETH                    SOC_TMC_FP_PKT_HDR_TYPE_MPLS3_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_ETH_MPLS1_ETH                SOC_TMC_FP_PKT_HDR_TYPE_ETH_MPLS1_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_ETH_MPLS2_ETH                SOC_TMC_FP_PKT_HDR_TYPE_ETH_MPLS2_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_ETH_MPLS3_ETH                SOC_TMC_FP_PKT_HDR_TYPE_ETH_MPLS3_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV4_MPLS1_ETH               SOC_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS1_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV4_MPLS2_ETH               SOC_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS2_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV4_MPLS3_ETH               SOC_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS3_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV6_MPLS1_ETH               SOC_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS1_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV6_MPLS2_ETH               SOC_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS2_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV6_MPLS3_ETH               SOC_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS3_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV4_IPV4_ETH                SOC_TMC_FP_PKT_HDR_TYPE_IPV4_IPV4_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_IPV6_IPV4_ETH                SOC_TMC_FP_PKT_HDR_TYPE_IPV6_IPV4_ETH
#define SOC_PPC_FP_PKT_HDR_TYPE_ETH_TRILL_ETH                SOC_TMC_FP_PKT_HDR_TYPE_ETH_TRILL_ETH
#define SOC_PPC_PP_NOF_FP_PKT_HDR_TYPES                      SOC_TMC_NOF_FP_PKT_HDR_TYPES
typedef SOC_TMC_FP_PKT_HDR_TYPE                              SOC_PPC_FP_PKT_HDR_TYPE;



/* $Id: ppc_api_fp.h,v 1.95 Broadcom SDK $
 * 	NOTE: the Forwarding qualifiers can be used
 *        only with predefined Keys.
 */
typedef enum
{
  /*
   *  VLAN Tag (16b) of the default Ethernet Header. The
   *  default Ethernet Header is the forwarding Header (if
   *  Ethernet), otherwise the outermost Ethernet Header.
   */
  SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG = 0,
  /*
   *  Default Source MAC address. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _MAC_ADDRESS (48b).
   */
  SOC_PPC_FP_QUAL_HDR_FWD_SA = 1,
  /*
   *  Default Destination MAC address. The Qualifier value can
   *  be constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _MAC_ADDRESS (48b).
   */
  SOC_PPC_FP_QUAL_HDR_FWD_DA = 2,
  /*
   *  Ethernet-Type as encoded by the parser.
   *  User-defined EtherTypes can be
   *  added via the soc_ppd_fp_control_set API using the type
   *  SOC_PPC_FP_CONTROL_TYPE_ETHERTYPE (16b) and
   *  SOC_PPC_FP_CONTROL_TYPE_EGR_L2_ETHERTYPES for egress (16b).
   *  The exact index (0- 15) of the encoded EtherTypes
   *  can be obtained via the soc_ppd_fp_control_get API.
   */
  SOC_PPC_FP_QUAL_HDR_FWD_ETHERTYPE = 3,
  /*
   *  Default VLAN TAG (2nd one in the header) - (16b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_2ND_VLAN_TAG = 4,
  /*
   *  Inner VLAN Tag (16b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_VLAN_TAG = 5,
  /*
   *  Inner Source MAC address. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _MAC_ADDRESS (48b).
   */
  SOC_PPC_FP_QUAL_HDR_INNER_SA = 6,
  /*
   *  Inner Destination MAC address. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _MAC_ADDRESS (48b).
   */
  SOC_PPC_FP_QUAL_HDR_INNER_DA = 7,
  /*
   *  Inner Ethernet-Type. The number of VLAN tags in the
   *  inner Ethernet header can be defined per PFG via the
   *  soc_ppd_fp_control_set API using the type
   *  SOC_PPC_FP_CONTROL_TYPE_INNER_ETH_NOF_VLAN_TAGS
   */
  SOC_PPC_FP_QUAL_HDR_INNER_ETHERTYPE = 8,
  /*
   *  Inner VLAN TAG (2nd one in the header) - (16b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_2ND_VLAN_TAG = 9,
  /*
   *  VLAN Format of the packet. Its value can be built using
   *  the enum SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT (4b)
   */
  SOC_PPC_FP_QUAL_HDR_VLAN_FORMAT = 10,
  /*
   *  Outer VLAN Tag (16b)
   */
  SOC_PPC_FP_QUAL_HDR_VLAN_TAG = 11,
  /*
   *  Source MAC address. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _MAC_ADDRESS (48b).
   */
  SOC_PPC_FP_QUAL_HDR_SA = 12,
  /*
   *  Destination MAC address. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _MAC_ADDRESS (48b).
   */
  SOC_PPC_FP_QUAL_HDR_DA = 13,
  /*
   *  Ethernet-Type code. 
   */
  SOC_PPC_FP_QUAL_HDR_ETHERTYPE = 14,
  /*
   *  VLAN TAG (2nd one in the header) - (16b). The 2nd VLAN
   *  Tag can be NOT part of the predefined L2 Key if the
   *  In-LIF field replaces it. The replacement is set via the
   *  soc_ppd_fp_control_set API with type
   *  SOC_PPC_FP_CONTROL_TYPE_L2_L3_KEY_IN_LIF_ENABLE
   */
  SOC_PPC_FP_QUAL_HDR_2ND_VLAN_TAG = 15,
  /*
   *  Next-Protocol of the default IPv4 Header as encoded
   *  by the parser. The default IPv4 Header is the forwarding
   *  Header (if IPv4), otherwise the outermost IPv4 Header.
   *  User-defined Next-Protocol can be added via the
   *  soc_ppd_fp_control_set API using the type
   *  SOC_PPC_FP_CONTROL_TYPE_NEXT_PROTOCOL_IP (8b) for ingress
   *  parsing and SOC_PPC_FP_CONTROL_TYPE_EGR_IPV4_NEXT_PROTOCOL
   *  for egress (16b). The exact index (0- 15) of the encoded
   *  Next-Protocol can be obtained via the soc_ppd_fp_control_get
   *  API at ingress.
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_NEXT_PRTCL = 16,
  /*
   *  Dont-Fragment (1b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DF = 17,
  /*
   *  More-Fragment (1b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_MF = 18,
  /*
   *  Source-IP. The Qualifier value can be constructed using
   *  the macro SOC_PPC_FP_QUAL_VAL_ENCODE _IPV4_SUBNET (32b).
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP = 19,
  /*
   *  Destination-IP. The Qualifier value can be constructed
   *  using the macro SOC_PPC_FP_QUAL_VAL_ENCODE _IPV4_SUBNET
   *  (32b).
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP = 20,
  /*
   *  L4-Src-Port (16b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SRC_PORT = 21,
  /*
   *  L4-Dest-Port (16b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DEST_PORT = 22,
  /*
   *  Type of Service (8b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_TOS = 23,
  /*
   *  TCP-Control field (Bits URG to FIN of the TCP header)
   *  (6b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_TCP_CTL = 24,
  /*
   *  Incoming-VID (12b)
   */
  SOC_PPC_FP_QUAL_HDR_FWD_IPV4_IN_VID = 25,
  /*
   *  Inner Next-Protocol. (8b).
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_NEXT_PRTCL = 26,
  /*
   *  Inner Dont-Fragment (1b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_DF = 27,
  /*
   *  Inner More-Fragment (1b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_MF = 28,
  /*
   *  Inner Source-IP. The Qualifier value can be constructed
   *  using the macro SOC_PPC_FP_QUAL_VAL_ENCODE _IPV4_SUBNET
   *  (32b).
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_SIP = 29,
  /*
   *  Inner Destination-IP. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _IPV4_SUBNET (32b).
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_DIP = 30,
  /*
   *  Inner L4-Src-Port (16b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_SRC_PORT = 31,
  /*
   *  Inner L4-Dest-Port (16b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_DEST_PORT = 32,
  /*
   *  Inner Type of Service (8b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_TOS = 33,
  /*
   *  Inner TCP-Control field (Bits URG to FIN of the TCP
   *  header) (6b)
   */
  SOC_PPC_FP_QUAL_HDR_INNER_IPV4_TCP_CTL = 34,
  /*
   *  Next-Protocol. (8b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_NEXT_PRTCL = 36,
  /*
   *  Dont-Fragment (1b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_DF = 37,
  /*
   *  More-Fragment (1b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_MF = 38,
  /*
   *  Source-IP. The Qualifier value can be constructed using
   *  the macro SOC_PPC_FP_QUAL_VAL_ENCODE _IPV4_SUBNET (32b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_SIP = 39,
  /*
   *  Destination-IP. The Qualifier value can be constructed
   *  using the macro SOC_PPC_FP_QUAL_VAL_ENCODE _IPV4_SUBNET
   *  (32b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_DIP = 40,
  /*
   *  L4-Src-Port (16b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_SRC_PORT,
  /*
   *  L4-Dest-Port (16b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_DEST_PORT,
  /*
   *  L4-Dest-Port and Src-Port (32b)
   */

  SOC_PPC_FP_QUAL_HDR_IPV4_SRC_DEST_PORT,
  /*
   *  Type of Service (8b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_TOS,
  /*
   *  TCP-Control field (Bits URG to FIN of the TCP header)
   *  (6b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_TCP_CTL,  /* 45 */
  /*
   *  Source-IP after the forwarding header.
   *  The Qualifier value can be constructed using the macro DBAL_QUAL_VAL_ENCODE_AFTER_FWD_IPV4_SIP
   *  (32b).
   */
  SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_SIP,
  /*
   *  Destination-IP after the forwarding header.
   *  The Qualifier value can be constructed using the macro DBAL_QUAL_VAL_ENCODE_AFTER_FWD_IPV4_DIP
   *  (32b).
   */
  SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_DIP,
  /*
   *  Layer-4 Ops High value (L4Ops[23:16]). Its value is defined according to the L4Ops
   *  ranges which can be set via the soc_ppd_fp_control_set API
   *  with the type SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE (8b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_L4OPS_HI,
  /*
   *  Layer-4 Ops Low value (L4Ops[15:0]). Its value is defined according to the L4Ops
   *  ranges which can be set via the soc_ppd_fp_control_set API
   *  with the type SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE (16b).
   *  Note: the L4Ops low is not part of the predefined qualifiers if using the macro.
   */
  SOC_PPC_FP_QUAL_HDR_IPV4_L4OPS_LOW,
  /*
   *  Source-IP MSBs. The Qualifier value can be constructed
   *  using the macro SOC_PPC_FP_QUAL_VAL_ENCODE _IPV6_SUBNET
   *  (64b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH, /* 50 */
  /*
   *  Source-IP LSBs. The Qualifier value can be constructed
   *  using the macro SOC_PPC_FP_QUAL_VAL_ENCODE _IPV6_SUBNET
   *  (64b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW,
  /*
   *  Destination-IP MSBs. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _IPV6_SUBNET (64b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH,
  /*
   *  Destination-IP LSBs. The Qualifier value can be
   *  constructed using the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _IPV6_SUBNET (64b).
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_DIP_LOW,
  /*
   *  Next-Protocol (8b). Similar of use as IPv4
   *  Next-Protocol (see above).
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_NEXT_PRTCL,
  /*
   *  TCP-Control (6b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_TCP_CTL,
  /*
   *  L4-Ops. Its value is defined according to the L4Ops
   *  ranges which can be set via the soc_ppd_fp_control_set API
   *  with the type SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE (24b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_L4OPS,
  /*
   *  IPv6 Traffic Class (8b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_TC,
  /*
   *  Forwarding MPLS Label (32b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_FWD,
  /*
   *  Forwarding MPLS Experimental Use (3b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_EXP_FWD,
  /*
   *  Forwarding MPLS Time-To-Live (8b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_TTL_FWD,         /* 60 */
  /*
   *  Forwarding MPLS Bottom-of-Stack (32b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_BOS_FWD,
  /*
   *  MPLS Label 1 (20b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_LABEL1,
  /*
   *  MPLS Experimental Use 1 (3b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_EXP1,
  /*
   *  MPLS Time-To-Live 1 (8b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_TTL1,
  /*
   *  MPLS Bottom-of-Stack 1 (32b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_BOS1,
  /*
   *  MPLS Label 2 (20b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_LABEL2,
  /*
   *  MPLS Experimental Use 2 (3b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_EXP2,
  /*
   *  MPLS Time-To-Live 2 (8b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_TTL2,
  /*
   *  MPLS Bottom-of-Stack 2 (32b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_BOS2,
  /*
   *  MPLS Label 3 (20b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_LABEL3,   /* 70 */
  /*
   *  MPLS Experimental Use 3 (3b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_EXP3,
  /*
   *  MPLS Time-To-Live 3 (8b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_TTL3,
  /*
   *  MPLS Bottom-of-Stack 3 (32b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_BOS3,
  /*
   *  MPLS Label 4 (20b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_LABEL4,
  /*
   *  MPLS Experimental Use 4 (3b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_EXP4,
  /*
   *  MPLS Time-To-Live 4 (8b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_TTL4,
  /*
   *  MPLS Bottom-of-Stack 4 (32b)
   */
  SOC_PPC_FP_QUAL_HDR_MPLS_BOS4,
  /*
   *  Local incoming TM port. Range: 0 - 79.
   */
  SOC_PPC_FP_QUAL_IRPP_SRC_TM_PORT,
  /*
   *  Source System-Port ID. Range: 0 - 4K-1.
   */
  SOC_PPC_FP_QUAL_IRPP_SRC_SYST_PORT,
  /*
   *  Local incoming PP port. Range: 0 - 63.
   */
  SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT,   /* 80 */
  /*
   *  Processing type set by the parser for this Packet. The
   *  value is set according to the SOC_PPC_FP_PROCESSING_TYPE
   *  enum
   */
  SOC_PPC_FP_QUAL_IRPP_PROCESSING_TYPE,
  /*
   *  Packet header type: header stack of the packet.
   *  In Soc_petra-B, this field value is set according to
   *  the SOC_PB_PARSER_PKT_HDR_STK_TYPE enum. - Packet Format Code
   */
  SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,
  /*
   *  Ethernet Tag format of the packet. Identifies the VLAN
   *  tags structure on the packet. The field value can be
   *  built using the macro
   *  SOC_PPC_FP_QUAL_VAL_ENCODE_ETH_TAG_FORMAT.
   */
  SOC_PPC_FP_QUAL_IRPP_ETH_TAG_FORMAT,
  /*
   *  The Forwarding Decision taken for the packet. The
   *  Qualifier value can be constructed using the macro
   *  SOC_PPC_FP_QUAL_VAL_ENCODE_FWD_DECISION (16b).
   *  The 18th bit corresponds to a valid bit:
   *  if set, then the destination qualifier is relevant.
   *  Note: valid bits should be used by the user in
   *  case of Direct Extraction with caution (at most one
   *  qualifier per action type).
   */
  SOC_PPC_FP_QUAL_IRPP_FWD_DEC_DEST,
  /*
   *  Traffic Class of the Forwarding decision (3b).
   *  The last bit (4th) corresponds to a valid bit:
   *  if set, then the TC qualifier is relevant.
   *  Note: valid bits should be used by the user in case
   *  of Direct Extraction with caution (at most one
   *  qualifier per action type).
   */
  SOC_PPC_FP_QUAL_IRPP_FWD_DEC_TC,   /* 85 */
  /*
   *  Drop Precedence of the Forwarding decision (2b).
   *  The last bit (3th) corresponds to a valid bit: if set,
   *  then the DP qualifier is relevant. Note: valid bits
   *  should be used by the user in case of Direct Extraction
   *  with caution (at most one qualifier per action type).
   */
  SOC_PPC_FP_QUAL_IRPP_FWD_DEC_DP,
  /*
   *  CPU Trap code of the Forwarding Decision (8b)
   */
  SOC_PPC_FP_QUAL_IRPP_FWD_DEC_CPU_TRAP_CODE,
  /*
   *  CPU Trap qualifier of the Forwarding Decision (14b)
   */
  SOC_PPC_FP_QUAL_IRPP_FWD_DEC_CPU_TRAP_QUAL,
  /*
   *  User-Priority of the Packet (3b)
   */
  SOC_PPC_FP_QUAL_IRPP_UP,
  /*
   *  Snoop code (8b)
   */
  SOC_PPC_FP_QUAL_IRPP_SNOOP_CODE,   /* 90 */
  /*
   *  Learn Decision Destination. The Qualifier value can be
   *  constructed using the macro
   *  SOC_PPC_FP_QUAL_VAL_ENCODE_FWD_DECISION (16b).
   */
  SOC_PPC_FP_QUAL_IRPP_LEARN_DECISION_DEST,
  /*
   *  Learn Decision Addition Information. The Qualifier value
   *  can be constructed using the macro
   *  SOC_PPC_FP_QUAL_VAL_ENCODE_FWD_DECISION (24b).
   */
  SOC_PPC_FP_QUAL_IRPP_LEARN_ADD_INFO,
  /*
   *  In-LIF (16b). The In-LIF can appear in the predefined L2
   *  and L3 IPv4 ACL Keys. In this case, use the
   *  soc_ppd_fp_control_set API with type
   *  SOC_PPC_FP_CONTROL_TYPE_L2_L3_KEY_IN_LIF_ENABLE
   */
  SOC_PPC_FP_QUAL_IRPP_IN_LIF,
  /*
   *  LL Mirror Command (4b)
   */
  SOC_PPC_FP_QUAL_IRPP_LL_MIRROR_CMD,
  /*
   *  System VSI (16b)
   */
  SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI,
  /*
   *  Orientation=Is-Hub (1b)
   */
  SOC_PPC_FP_QUAL_IRPP_ORIENTATION_IS_HUB,
  /*
   *  VLAN-ID result of the VLAN Ingress Editing (12b)
   */
  SOC_PPC_FP_QUAL_IRPP_VLAN_ID,
  /*
   *  PCP result of the VLAN Ingress Editing  (3b)
   */
  SOC_PPC_FP_QUAL_IRPP_VLAN_PCP,
  /*
   *  DEI result of the VLAN Ingress Editing  (1b)
   */
  SOC_PPC_FP_QUAL_IRPP_VLAN_DEI,
  /*
   *  State in the Spanning Tree Protocol (2b)
   */
  SOC_PPC_FP_QUAL_IRPP_STP_STATE,   /* 100 */
  /*
   *  Type of forwarding processed on the packet. The field
   *  value is set according to the SOC_PPC_FP_FWD_TYPE enum.
   */
  SOC_PPC_FP_QUAL_IRPP_FWD_TYPE,
  /*
   *  Indicate on which header the forwarding occurs (3b)
   */
  SOC_PPC_FP_QUAL_IRPP_SUB_HEADER_NDX,
  /*
   *  If set, this Database happens in the second cycle,
   *  and the value taken for this Qualifier corresponds
   *  to the action value of a first-cycle Database with
   *  an Action type SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY (12b).
   *  Note: up to one Database per Packet-Format-Group
   *  can have this Qualifier. Besides, this Database
   *  must be created previously to the first-cycle
   *  Database defining the Qualifier value.
   *  The Key change size can be modified via the
   *  SOC_PPC_FP_CONTROL_TYPE_KEY_CHANGE_SIZE control type.
   */
   SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED,
  /*
   *  In-RIF (12b)
   */
  SOC_PPC_FP_QUAL_IRPP_IN_RIF,
  /*
   *  VRF (8b)
   */
  SOC_PPC_FP_QUAL_IRPP_VRF,    /* 105 */
  /*
   *  Packet-Is-Compatible-Multicast (1b)
   */
  SOC_PPC_FP_QUAL_IRPP_PCKT_IS_COMP_MC,
  /*
   *  My-Backbone-MAC (1b). Arad only
   */
  SOC_PPC_FP_QUAL_IRPP_MY_BMAC,
  /*
   *  In-TTL (8b)
   */
  SOC_PPC_FP_QUAL_IRPP_IN_TTL,
  /*
   *  In-DSCP-EXP (8b)
   */
  SOC_PPC_FP_QUAL_IRPP_IN_DSCP_EXP,
  /*
   *  Packet-Size-Range (2b). The soc_ppd_fp_control_set API can
   *  be called to set the packet size ranges.
   */
  SOC_PPC_FP_QUAL_IRPP_PACKET_SIZE_RANGE,   /* 110 */
  /*
   *  Packet termination type (4b) indicating the number
   *  of headers that have been terminated.
   *  Its encoding is based on the enum SOC_PPC_PKT_TERM_TYPE.
   */
  SOC_PPC_FP_QUAL_IRPP_TERM_TYPE,
  /*
   *  Out-PP-Port data part of the predefined Egress ACL Keys.
   *  The value of each PP-Port can be defined via the
   *  soc_ppd_fp_control_set API with type
   *  SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA (6b).
   */
  SOC_PPC_FP_QUAL_ERPP_PP_PORT_DATA,
  /*
   *  IPv4 Next-Protocol matches. The IPv4 Next-Protocol is
   *  compared to values which can be set via the
   *  soc_ppd_fp_control_set API with type
   *  SOC_PPC_FP_CONTROL_TYPE_EGR_IPV4_NEXT_PROTOCOL (4b).
   */
  SOC_PPC_FP_QUAL_ERPP_IPV4_NEXT_PROTOCOL,
  /*
   *  Packet FTMH (Base + CUD Extension). Needed for the
   *  predefined egress MPLS and TM ACL Keys (64b). Its value
   *  can be built using the macro
   *  SOC_PPC_FP_QUAL_VAL_ENCODE_FTMH.
   */
  SOC_PPC_FP_QUAL_ERPP_FTMH,
  /*
   *  Packet payload coming after the FTMH (Base + CUD
   *  Extension). Needed for the predefined egress MPLS (32b)
   *  and TM ACL Keys (72b).
   */
  SOC_PPC_FP_QUAL_ERPP_PAYLOAD,   /* 115 */
  /*
   *  User-Defined field from the Packet Header. The field
   *  corresponds to bits extracted from a specific location
   *  which can be set via the soc_ppd_fp_control_set API with
   *  type SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_0,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_1,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_2,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_3,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_4,   /* 120 */
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_5,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_6,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_7,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_8,
  /*
   *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
   */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_9,
 /*
  *  See SOC_PPC_FP_QUAL_HDR_USER_DEF_0.
  */
  SOC_PPC_FP_QUAL_HDR_USER_DEF_LAST = SOC_PPC_FP_QUAL_HDR_USER_DEF_0 + 63,  /* = 116 + 63 = 179 */
    /*
     *  Source-IP MSBs.
     */
    SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH,
    /*
     *  Source-IP LSBs.
     */
    SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW,
    /*
     *  Destination-IP MSBs.
     */    
    SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH,
    /*
     *  Destination-IP MSBs.
     */   
    SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW,
    /*
     *  TTL (8b)
     */
    SOC_PPC_FP_QUAL_HDR_IPV4_TTL,
    /*
     *  Inner TTL (8b)
     */
    SOC_PPC_FP_QUAL_HDR_INNER_IPV4_TTL,
    /*
     *  Flags (3b)
     */
    SOC_PPC_FP_QUAL_HDR_IPV4_FLAGS,    /* 186 */

  /*
   *  IPv6 Flow Label (20b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_FLOW_LABEL,
  /*
   *  IPv6 Hop Limit (8b)
   */
  SOC_PPC_FP_QUAL_HDR_IPV6_HOP_LIMIT,

  /* 2nd LEM lookup result */
  SOC_PPC_FP_QUAL_IRPP_LEM_2ND_LKUP_ASD,

    /*
     *  Forwarding MPLS Label ID (20b)
     */
    SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD,  /* 190 */
    SOC_PPC_FP_QUAL_HDR_MPLS_LABEL1_ID,
    SOC_PPC_FP_QUAL_HDR_MPLS_LABEL2_ID,
    SOC_PPC_FP_QUAL_HDR_MPLS_LABEL3_ID,
    SOC_PPC_FP_QUAL_HDR_MPLS_LABEL4_ID,

  /* 
   * Out-LIF qualifier (16b) 
   * In Soc_petra-B, in a signal called EEI-Or-OutLIF 
   */
  SOC_PPC_FP_QUAL_OUT_LIF,         /* 195 */

  /* 
   *  Arad qualifiers
   */
  /* INVALID qualifier: if set, the CE is invalid and skipped */
  SOC_PPC_FP_QUAL_IRPP_INVALID,

    /* Port bitmap - Program selection only */
    SOC_PPC_FP_QUAL_IRPP_IN_PORT_BITMAP,

  /* in_port_key_gen_var */
  SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR,
  SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR_PS, /* Program selection qualifier */

  /* ptc_key_gen_var */
  SOC_PPC_FP_QUAL_IRPP_PTC_KEY_GEN_VAR,

  /* in_port_key_gen_var */
  SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,

  /* Arad ITMH Header */
  SOC_PPC_FP_QUAL_HDR_ITMH,

  /* Arad ITMH Destination Extension. Always after ITMH */
  SOC_PPC_FP_QUAL_HDR_ITMH_EXT,

  /* Arad ITMH Destination according to the Forwarding Header */
  SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,

  /* Arad FTMH Header */
    /* Arad ITMH PMF-Header extension: 160b after end of ITMH */
  SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,

  SOC_PPC_FP_QUAL_HDR_PTCH2_OPAQUE,
  SOC_PPC_FP_QUAL_HDR_ITMH_PPH_TYPE,

  SOC_PPC_FP_QUAL_HDR_OAM_ETHERTYPE,
  SOC_PPC_FP_QUAL_HDR_OAM_2ND_ETHERTYPE,
  SOC_PPC_FP_QUAL_HDR_BFD_PPH_FWD_CODE,
  SOC_PPC_FP_QUAL_HDR_BFD_1ST_NIBBLE_AFTER_LABEL,

  SOC_PPC_FP_QUAL_HDR_FTMH,

  /* Arad FTMH LB KEY extension */
  SOC_PPC_FP_QUAL_HDR_FTMH_LB_KEY_EXT_AFTER_FTMH,
    SOC_PPC_FP_QUAL_HDR_FTMH_LB_KEY_START_OF_PACKET,

  /* Arad DSP extension for FTMH Header */
  SOC_PPC_FP_QUAL_HDR_DSP_EXTENSION_AFTER_FTMH,

  /* Arad DSP extension for FTMH Header (over DSP_EXTENSION)*/
    SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT,
  SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT_PETRA,

    /*  Outer VLAN Tag TPID */
    SOC_PPC_FP_QUAL_HDR_VLAN_TAG_TPID,
    SOC_PPC_FP_QUAL_HDR_VLAN_TAG_ID,
    SOC_PPC_FP_QUAL_HDR_VLAN_TAG_CFI,
    SOC_PPC_FP_QUAL_HDR_VLAN_TAG_PRI,
    SOC_PPC_FP_QUAL_HDR_VLAN_TAG_PRI_CFI,
    /*  Inner VLAN Tag TPID */
    SOC_PPC_FP_QUAL_HDR_2ND_VLAN_TAG_TPID,
    SOC_PPC_FP_QUAL_HDR_2ND_VLAN_TAG_ID,
    SOC_PPC_FP_QUAL_HDR_2ND_VLAN_TAG_CFI,
    SOC_PPC_FP_QUAL_HDR_2ND_VLAN_TAG_PRI,

    SOC_PPC_FP_QUAL_HDR_INNER_VLAN_TAG_ID,
    SOC_PPC_FP_QUAL_HDR_INNER_VLAN_TAG_CFI,
    SOC_PPC_FP_QUAL_HDR_INNER_VLAN_TAG_PRI,

    SOC_PPC_FP_QUAL_HDR_INNER_2ND_VLAN_TAG_ID,
    SOC_PPC_FP_QUAL_HDR_INNER_2ND_VLAN_TAG_CFI,
    SOC_PPC_FP_QUAL_HDR_INNER_2ND_VLAN_TAG_PRI,

    SOC_PPC_FP_QUAL_OUTER_VLAN_ACTION_RANGE,
    SOC_PPC_FP_QUAL_INNER_VLAN_ACTION_RANGE,

    /* All zeroes */
    SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,

    /* All ones */
    SOC_PPC_FP_QUAL_IRPP_ALL_ONES,

    /* PEM  */
    SOC_PPC_FP_QUAL_IRPP_PEM_GENERAL_DATA,

    SOC_PPC_FP_QUAL_IRPP_PROG_VAR,

    /* Add all the PPD signals in a pack - same name than the signal */
    SOC_PPC_FP_QUAL_PACKET_HEADER_SIZE,
    SOC_PPC_FP_QUAL_HEADER_OFFSET0,
    SOC_PPC_FP_QUAL_HEADER_OFFSET1,
    SOC_PPC_FP_QUAL_HEADER_OFFSET2,
    SOC_PPC_FP_QUAL_HEADER_OFFSET3,
    SOC_PPC_FP_QUAL_HEADER_OFFSET4,
    SOC_PPC_FP_QUAL_HEADER_OFFSET5,
    SOC_PPC_FP_QUAL_HEADER_OFFSET_0_UNTIL_5,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER0,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER1,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER1_NEXT_PROTOCOL,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER1_ENCAPSULATION,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER1_OUTER_TAG,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER1_PRIORITY,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER1_INNER_TAG,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER2,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER2_BOS,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER2_IP_FRAGMENTED,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER2_GRE_PARSED, 
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER2_IP_HAS_OPTIONS,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER2_NEXT_PROTOCOL,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER3,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER3_IP_FRAGMENTED,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER4,
    SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER5,
    SOC_PPC_FP_QUAL_FORWARDING_ACTION_STRENGTH,
    SOC_PPC_FP_QUAL_FORWARDING_ACTION_METER_TRAFFIC_CLASS,
    SOC_PPC_FP_QUAL_SNOOP_STRENGTH,
    SOC_PPC_FP_QUAL_VSI_PROFILE,
    SOC_PPC_FP_QUAL_FID,
    SOC_PPC_FP_QUAL_VLAN_EDIT_CMD_COMMAND,
    SOC_PPC_FP_QUAL_VLAN_EDIT_CMD_VID2,
    SOC_PPC_FP_QUAL_VLAN_EDIT_CMD_VID1,
    SOC_PPC_FP_QUAL_FORWARDING_OFFSET_EXTENSION,
    SOC_PPC_FP_QUAL_FORWARDING_HEADER_ENCAPSULATION,
    SOC_PPC_FP_QUAL_IGNORE_CP,
    SOC_PPC_FP_QUAL_SEQUENCE_NUMBER_TAG,
    SOC_PPC_FP_QUAL_EEI,
    SOC_PPC_FP_QUAL_RPF_DESTINATION,
    SOC_PPC_FP_QUAL_RPF_DESTINATION_VALID,
    SOC_PPC_FP_QUAL_INGRESS_LEARN_ENABLE,
    SOC_PPC_FP_QUAL_EGRESS_LEARN_ENABLE,
    SOC_PPC_FP_QUAL_LEARN_KEY,
    SOC_PPC_FP_QUAL_LEARN_KEY_MAC,
    SOC_PPC_FP_QUAL_LEARN_KEY_VLAN,
    SOC_PPC_FP_QUAL_IN_LIF_PROFILE,
    SOC_PPC_FP_QUAL_IN_LIF_PROFILE_SAME_IF,
    SOC_PPC_FP_QUAL_LEARN_OR_TRANSPLANT,
    SOC_PPC_FP_QUAL_PACKET_IS_BOOTP_DHCP,
    SOC_PPC_FP_QUAL_UNKNOWN_ADDR,
    SOC_PPC_FP_QUAL_FWD_PRCESSING_PROFILE,
    SOC_PPC_FP_QUAL_ELK_ERROR,
    SOC_PPC_FP_QUAL_ELK_LKP_PAYLOAD,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_FOUND_0,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_FOUND_1,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_FOUND_2,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_FOUND_3,
	SOC_PPC_FP_QUAL_ELK_LOOKUP_FOUND_4,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_FOUND_5,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_0,/* res_0 to res_5 has to be in this order and without qualifiers in between */
    SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_1,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_2,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_3,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_4,
    SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_5,
    SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_FOUND,
    SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_RESULT,
    SOC_PPC_FP_QUAL_COUNTER_UPDATE_A,
    SOC_PPC_FP_QUAL_COUNTER_POINTER_A,
    SOC_PPC_FP_QUAL_COUNTER_UPDATE_B,
    SOC_PPC_FP_QUAL_COUNTER_POINTER_B,
    SOC_PPC_FP_QUAL_PROGRAM_INDEX,
    SOC_PPC_FP_QUAL_LEARN_DATA,
    SOC_PPC_FP_QUAL_LEM_2ND_LOOKUP_FOUND,
    SOC_PPC_FP_QUAL_LEM_2ND_LOOKUP_RESULT,
    SOC_PPC_FP_QUAL_LPM_1ST_LOOKUP_FOUND,
    SOC_PPC_FP_QUAL_LPM_1ST_LOOKUP_RESULT,
    SOC_PPC_FP_QUAL_LPM_2ND_LOOKUP_FOUND,
    SOC_PPC_FP_QUAL_LPM_2ND_LOOKUP_RESULT,
    SOC_PPC_FP_QUAL_TCAM_MATCH,
    SOC_PPC_FP_QUAL_TCAM_RESULT,
    SOC_PPC_FP_QUAL_TCAM_TRAPS0_MATCH,
    SOC_PPC_FP_QUAL_TCAM_TRAPS0_RESULT,
    SOC_PPC_FP_QUAL_TCAM_TRAPS1_MATCH,
    SOC_PPC_FP_QUAL_TCAM_TRAPS1_RESULT,
    SOC_PPC_FP_QUAL_TT_PROCESSING_PROFILE,
    SOC_PPC_FP_QUAL_TT_LOOKUP0_FOUND,
    SOC_PPC_FP_QUAL_TT_LOOKUP0_PAYLOAD,
    SOC_PPC_FP_QUAL_TT_LOOKUP1_FOUND,
    SOC_PPC_FP_QUAL_TT_LOOKUP1_PAYLOAD,
    SOC_PPC_FP_QUAL_VT_PROCESSING_PROFILE,
    SOC_PPC_FP_QUAL_VT_LOOKUP0_FOUND,
    SOC_PPC_FP_QUAL_VT_LOOKUP0_PAYLOAD,
    SOC_PPC_FP_QUAL_VT_LOOKUP1_FOUND,
    SOC_PPC_FP_QUAL_VT_LOOKUP1_PAYLOAD,
    SOC_PPC_FP_QUAL_CPU_TRAP_CODE_PROFILE,
    SOC_PPC_FP_QUAL_VID_VALID,
    SOC_PPC_FP_QUAL_DA_IS_BPDU,
    SOC_PPC_FP_QUAL_PACKET_IS_IEEE1588,
    SOC_PPC_FP_QUAL_IEEE1588_ENCAPSULATION,
    SOC_PPC_FP_QUAL_IEEE1588_COMPENSATE_TIME_STAMP,
    SOC_PPC_FP_QUAL_IEEE1588_COMMAND,
    SOC_PPC_FP_QUAL_IEEE1588_HEADER_OFFSET,
    SOC_PPC_FP_QUAL_OAM_UP_MEP,
    SOC_PPC_FP_QUAL_OAM_SUB_TYPE,
    SOC_PPC_FP_QUAL_OAM_OFFSET,
    SOC_PPC_FP_QUAL_OAM_STAMP_OFFSET,
    SOC_PPC_FP_QUAL_OAM_METER_DISABLE,
    SOC_PPC_FP_QUAL_OAM_ID,
    SOC_PPC_FP_QUAL_TUNNEL_ID,
    SOC_PPC_FP_QUAL_ARP_SENDER_IP4,
    SOC_PPC_FP_QUAL_ARP_TARGET_IP4,
    SOC_PPC_FP_QUAL_ARP_OPCODE_IP4,

  /* XGS HG FRC Header */
  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC,

  /* XGS HG PPD Header */
  SOC_PPC_FP_QUAL_HDR_HIGIG_PPD,

  /* XGS HG PPD EXTENSION Header */
  SOC_PPC_FP_QUAL_HDR_HIGIG_PPD_EXT,

  SOC_PPC_FP_QUAL_HDR_MH_FLOW,
  SOC_PPC_FP_QUAL_HDR_MH_TC2,
  SOC_PPC_FP_QUAL_HDR_MH_DP0,
  SOC_PPC_FP_QUAL_HDR_MH_CAST,
  SOC_PPC_FP_QUAL_HDR_MH_DP1,
  SOC_PPC_FP_QUAL_HDR_MH_TC10,



		/* special inPheader  */
		SOC_PPC_FP_QUAL_HDR_INPHEADER_UC,
		SOC_PPC_FP_QUAL_HDR_INPHEADER_TB,
		SOC_PPC_FP_QUAL_HDR_INPHEADER_UC_TC,
		SOC_PPC_FP_QUAL_HDR_INPHEADER_MC_TC,
		SOC_PPC_FP_QUAL_HDR_INPHEADER_DP,


    /* PTCH Reserve LSB */
    SOC_PPC_FP_QUAL_HDR_PTCH_RESERVE_LSB,

  /* Egress Arad qualifiers */
  SOC_PPC_FP_QUAL_ERPP_ONES,
  SOC_PPC_FP_QUAL_ERPP_ZEROES,
  SOC_PPC_FP_QUAL_ERPP_OAM_TS,
  SOC_PPC_FP_QUAL_ERPP_LEARN_EXT,
  SOC_PPC_FP_QUAL_ERPP_LEARN_EXT_SRC_PORT,
  SOC_PPC_FP_QUAL_ERPP_LEARN_EXT_IN_VPORT,
  SOC_PPC_FP_QUAL_ERPP_FHEI,
    SOC_PPC_FP_QUAL_ERPP_FHEI_EXP,
    SOC_PPC_FP_QUAL_ERPP_FHEI_DSCP,
  SOC_PPC_FP_QUAL_ERPP_FHEI_IPV4_TTL,
  SOC_PPC_FP_QUAL_ERPP_OUT_TM_PORT_PMF_DATA,
    SOC_PPC_FP_QUAL_ERPP_OUT_PP_PORT_PMF_DATA_PS, /* Program selection qualifier */
  SOC_PPC_FP_QUAL_ERPP_OUT_PP_PORT_PMF_DATA,
  SOC_PPC_FP_QUAL_ERPP_EEI,
  SOC_PPC_FP_QUAL_ERPP_EXT_IN_LIF,
  SOC_PPC_FP_QUAL_ERPP_EXT_OUT_LIF,
  SOC_PPC_FP_QUAL_ERPP_STACKING_ROUTE_HISTORY_BITMAP,
  SOC_PPC_FP_QUAL_ERPP_DSP_EXT,
  SOC_PPC_FP_QUAL_ERPP_PACKET_SIZE,
  SOC_PPC_FP_QUAL_ERPP_DST_SYSTEM_PORT,
  SOC_PPC_FP_QUAL_ERPP_SRC_SYSTEM_PORT,
  SOC_PPC_FP_QUAL_ERPP_VSI_OR_VRF,
  SOC_PPC_FP_QUAL_ERPP_VSI_OR_VRF_ORIG,
  SOC_PPC_FP_QUAL_ERPP_FWD_OFFSET,
  SOC_PPC_FP_QUAL_ERPP_ETH_TAG_FORMAT,
  SOC_PPC_FP_QUAL_ERPP_SYS_VALUE1,
  SOC_PPC_FP_QUAL_ERPP_SYS_VALUE2,
  SOC_PPC_FP_QUAL_ERPP_DSP_PTR_ORIG,
  SOC_PPC_FP_QUAL_ERPP_DSP_PTR,
  SOC_PPC_FP_QUAL_ERPP_OUT_TM_PORT,
  SOC_PPC_FP_QUAL_ERPP_OUT_PP_PORT,
  SOC_PPC_FP_QUAL_ERPP_LB_KEY,
  SOC_PPC_FP_QUAL_ERPP_TC,
  SOC_PPC_FP_QUAL_ERPP_FORMAT_CODE,
  SOC_PPC_FP_QUAL_ERPP_FWD_CODE,
  SOC_PPC_FP_QUAL_ERPP_FWD_CODE_ORIG,
  SOC_PPC_FP_QUAL_ERPP_ACTION_PROFILE,
  SOC_PPC_FP_QUAL_ERPP_HEADER_CODE,
  SOC_PPC_FP_QUAL_ERPP_ETH_TYPE_CODE,
  SOC_PPC_FP_QUAL_ERPP_IN_LIF_ORIENTATION,
  SOC_PPC_FP_QUAL_ERPP_SNOOP_CPU_CODE,
  SOC_PPC_FP_QUAL_ERPP_FTMH_RESERVED,
  SOC_PPC_FP_QUAL_ERPP_ECN_CAPABLE,
  SOC_PPC_FP_QUAL_ERPP_PPH_TYPE,
  SOC_PPC_FP_QUAL_ERPP_TM_ACTION_TYPE,
  SOC_PPC_FP_QUAL_ERPP_DP,
  SOC_PPC_FP_QUAL_ERPP_FHEI_CODE,
  SOC_PPC_FP_QUAL_ERPP_LEARN_ALLOWED,
  SOC_PPC_FP_QUAL_ERPP_UNKNOWN_ADDR,
  SOC_PPC_FP_QUAL_ERPP_LEARN_EXT_VALID,
  SOC_PPC_FP_QUAL_ERPP_BYPASS_FILTERING,
  SOC_PPC_FP_QUAL_ERPP_EEI_VALID,
  SOC_PPC_FP_QUAL_ERPP_CNI,
  SOC_PPC_FP_QUAL_ERPP_DSP_EXT_VALID,
  SOC_PPC_FP_QUAL_ERPP_SYSTEM_MC,
  SOC_PPC_FP_QUAL_ERPP_OUT_MIRROR_DISABLE,
  SOC_PPC_FP_QUAL_ERPP_EXCLUDE_SRC,
  SOC_PPC_FP_QUAL_ERPP_DISCARD,
  SOC_PPC_FP_QUAL_ERPP_FABRIC_OR_EGRESS_MC,
  SOC_PPC_FP_QUAL_ERPP_RESEVED,
  SOC_PPC_FP_QUAL_ERPP_FIRST_COPY,
  SOC_PPC_FP_QUAL_ERPP_LAST_COPY,
  SOC_PPC_FP_QUAL_ERPP_START_BUFFER,
  SOC_PPC_FP_QUAL_ERPP_CONTEXT,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_DA,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_SA,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_DATA,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_ADDITIONAL_DATA,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_CPID0,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_CPID1,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_CPID2,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_OUTER_TPID,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_INNER_TPID,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_OUTER_TAG_ID,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_OUTER_TAG_CFI,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_OUTER_TAG_PRI,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_OUTER_TAG_PRI_CFI,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_OUTER_TAG,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_INNER_TAG,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_INNER_TAG_ID,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_INNER_TAG_CFI,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_ETH_INNER_TAG_PRI,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_OUTER_TAG_ID,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_OUTER_TAG_CFI,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_OUTER_TAG_PRI,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_OUTER_TAG,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_INNER_TAG,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_INNER_TAG_ID,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_INNER_TAG_CFI,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_FHEI_ETH_INNER_TAG_PRI,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_OPTIONS_PRESENT,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_TOTAL_LEN_ERROR,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_HEADER_LEN_ERROR,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_CHECKSUM_ERROR,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_VERSION_ERROR,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_TOS,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_SIP,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_DIP,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_PROTOCOL,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_L4_SRC_PORT,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV4_L4_DEST_PORT,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_MSBS_TC,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_MSBS_HOP_LIMIT,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_MSBS_FLOW_LABEL,
    SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_MSBS_NEXT_PROTOCOL,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_MC_DST,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_IPV4_MAPPED_DST,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_IPV4_CMP_DST,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_SITE_LOCAL_SRC,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_LINK_LOCAL_SRC,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_SITE_LOCAL_DST,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_LINK_LOCAL_DST,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_NEXT_HEADER_IS_ZERO,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_LOOPBACK_SRC_OR_DST,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_SIP_IS_ALL_ZEROES,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_DIP_IS_ALL_ZEROES,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_SIP_IS_MC,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_IPV6_VERSION_ERROR,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_MPLS_EXTRA_DATA,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_MPLS_HDR,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_EXTRA_DATA,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_HDR,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_VERSION,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_MULTI_DESTINATION,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_OP_LENGTH,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_HOP_COUNT,
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_EGRESS_RBRIDGE, 
  SOC_PPC_FP_QUAL_ERPP_NWK_RCRD_TRILL_INGRESS_RBRIDGE, 

    /* 
     * New qualifiers in Arad plus 
     */
  SOC_PPC_FP_QUAL_STAMP_NATIVE_VSI,
  SOC_PPC_FP_QUAL_NATIVE_VSI,
  SOC_PPC_FP_QUAL_IS_EQUAL,
  SOC_PPC_FP_QUAL_EXTENSION_HEADER_TYPE,

    /*
     * New qualifiers in FLP
     */
    SOC_PPC_FP_QUAL_CPU2FLP_C_INTERNAL_FIELDS_DATA,
    SOC_PPC_FP_QUAL_SERVICE_TYPE,
    SOC_PPC_FP_QUAL_VSI_UNKNOWN_DA_DESTINATION,
    SOC_PPC_FP_QUAL_I_SID,
    SOC_PPC_FP_QUAL_TT_LEARN_ENABLE,
    SOC_PPC_FP_QUAL_TT_LEARN_DATA,
    SOC_PPC_FP_QUAL_IN_LIF_UNKNOWN_DA_PROFILE,
    SOC_PPC_FP_QUAL_IN_RIF_UC_RPF_ENABLE,
    SOC_PPC_FP_QUAL_L3VPN_DEFAULT_ROUTING,
    SOC_PPC_FP_QUAL_TERMINATED_TTL_VALID,
    SOC_PPC_FP_QUAL_TERMINATED_TTL,
    SOC_PPC_FP_QUAL_TERMINATED_DSCP_EXP,
    SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_ESADI,
    SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_MPLS_EXP,
    SOC_PPC_FP_QUAL_TERMINATED_PROTOCOL,
    SOC_PPC_FP_QUAL_COS_PROFILE,
    SOC_PPC_FP_QUAL_VTT_OAM_LIF_VALID,
    SOC_PPC_FP_QUAL_VTT_OAM_LIF,
    SOC_PPC_FP_QUAL_LL_LEM_1ST_LOOKUP_FOUND,
    SOC_PPC_FP_QUAL_LL_LEM_1ST_LOOKUP_RESULT,
    SOC_PPC_FP_QUAL_VT_ISA_KEY,

    /* 
     * New qualifiers in Arad plus - SLB stage
     */
    SOC_PPC_FP_QUAL_KEY_AFTER_HASHING,
    SOC_PPC_FP_QUAL_IS_FEC_DEST_14_0,

    /* 
     * New qualifiers in Jericho
     */
    SOC_PPC_FP_QUAL_IRPP_ELK_LKP_PAYLOAD_LSB,
    SOC_PPC_FP_QUAL_IRPP_IN_RIF_PROFILE,
    SOC_PPC_FP_QUAL_IRPP_TCAM0_MATCH,
    SOC_PPC_FP_QUAL_IRPP_TCAM0_RESULT,
    SOC_PPC_FP_QUAL_IRPP_TCAM1_MATCH,
    SOC_PPC_FP_QUAL_IRPP_TCAM1_RESULT,
    SOC_PPC_FP_QUAL_IRPP_IN_LIF_DATA,
    SOC_PPC_FP_QUAL_IRPP_RPF_STAMP_NATIVE_VSI,
    SOC_PPC_FP_QUAL_IRPP_RPF_NATIVE_VSI,
    SOC_PPC_FP_QUAL_IRPP_IN_PORT_MAPPED_PP_PORT,
    SOC_PPC_FP_QUAL_IRPP_IN_LIF_DATA_INDEX,
    SOC_PPC_FP_QUAL_IRPP_LOCAL_IN_LIF,
    SOC_PPC_FP_QUAL_IRPP_CONSISTENT_HASHING_PGM_KEY_GEN_VAR,
    SOC_PPC_FP_QUAL_IRPP_PACKET_FORMAT_CODE_ACL,

    /*new oam qual*/
    SOC_PPC_FP_QUAL_ETH_OAM_HEADER_BITS_0_31,
    SOC_PPC_FP_QUAL_ETH_OAM_HEADER_BITS_32_63,
    SOC_PPC_FP_QUAL_MPLS_OAM_HEADER_BITS_0_31,
    SOC_PPC_FP_QUAL_MPLS_OAM_HEADER_BITS_32_63,
    SOC_PPC_FP_QUAL_MPLS_OAM_ACH  ,
    SOC_PPC_FP_QUAL_OAM_HEADER_BITS_0_31,
    SOC_PPC_FP_QUAL_OAM_HEADER_BITS_32_63,
    SOC_PPC_FP_QUAL_OAM_OPCODE,
  	SOC_PPC_FP_QUAL_OAM_MD_LEVEL_UNTAGGED,
  	SOC_PPC_FP_QUAL_OAM_MD_LEVEL_SINGLE_TAG,
  	SOC_PPC_FP_QUAL_OAM_MD_LEVEL_DOUBLE_TAG,
    SOC_PPC_FP_QUAL_OAM_MD_LEVEL,
  	SOC_PPC_FP_QUAL_TM_OUTER_TAG,
  	SOC_PPC_FP_QUAL_TM_INNER_TAG,
  	SOC_PPC_FP_QUAL_MY_DISCR_IPV4,
  	SOC_PPC_FP_QUAL_MY_DISCR_MPLS,
  	SOC_PPC_FP_QUAL_MY_DISCR_PWE,
    SOC_PPC_FP_QUAL_TRAP_QUALIFIER_FHEI,
    SOC_PPC_FP_QUAL_EID,

    /*Trill Qulifiers*/
    SOC_PPC_FP_QUAL_TRILL_INGRESS_NICK,
    SOC_PPC_FP_QUAL_TRILL_EGRESS_NICK,
    SOC_PPC_FP_QUAL_TRILL_NATIVE_VLAN_VSI,
    SOC_PPC_FP_QUAL_TRILL_NATIVE_ETH_INNER_TPID,
    SOC_PPC_FP_QUAL_TRILL_NATIVE_INNER_VLAN_VSI,

    /* GRE Qualifiers */
    SOC_PPC_FP_QUAL_GRE_CRKS,
    SOC_PPC_FP_QUAL_GRE_KEY,
    SOC_PPC_FP_QUAL_GRE_PROTOCOL_TYPE,
    SOC_PPC_FP_QUAL_NATIVE_VLAN_VSI,

    /* ETH Header I-SID */
    SOC_PPC_FP_QUAL_ETH_HEADER_ISID,

    SOC_PPC_FP_QUAL_VXLAN_VNI,

    SOC_PPC_FP_QUAL_UNTAG_HDR_ETHERTYPE,
    SOC_PPC_FP_QUAL_ONE_TAG_HDR_ETHERTYPE,
    SOC_PPC_FP_QUAL_DOUBLE_TAG_HDR_ETHERTYPE,

    /* PTC_Profile qualifier for In-TM-Port */  
    SOC_PPC_FP_QUAL_IRPP_PTC_KEY_GEN_VAR_PS,  

    
    SOC_PPC_FP_QUAL_VLAN_DOMAIN                           ,
    SOC_PPC_FP_QUAL_INITIAL_VID                           ,
    SOC_PPC_FP_QUAL_MPLS_KEY3                             ,
    SOC_PPC_FP_QUAL_MPLS_KEY2                             ,
    SOC_PPC_FP_QUAL_MPLS_KEY1                             ,
    SOC_PPC_FP_QUAL_MPLS_KEY0                             ,
    SOC_PPC_FP_QUAL_CMPRSD_INNER_VID                      ,
    SOC_PPC_FP_QUAL_CMPRSD_OUTER_VID                      ,
    SOC_PPC_FP_QUAL_MPLS_LABEL_RANGE_BASE_LIF2            ,
    SOC_PPC_FP_QUAL_MPLS_LABEL_RANGE_BASE_LIF1            ,
    SOC_PPC_FP_QUAL_MPLS_LABEL_RANGE_BASE_LIF0            ,
    SOC_PPC_FP_QUAL_LABEL3_IDX                            ,
    SOC_PPC_FP_QUAL_LABEL2_IDX                            ,
    SOC_PPC_FP_QUAL_LABEL1_IDX                            ,
    SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_FOUND_LEM_DOMAIN       ,
    SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_FOUND_LEM_SA_DROP      ,
    SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_FOUND_LEM_IS_LEARN_LIF ,
    SOC_PPC_FP_QUAL_LEM_DYNAMIC_LEM_1ST_LOOKUP_FOUND_LEM  ,
    SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_FOUND_DESTINATION      ,
    SOC_PPC_FP_QUAL_ISB_FOUND_IN_LIF_IDX                  ,
    SOC_PPC_FP_QUAL_ISB_FOUND                             ,
    SOC_PPC_FP_QUAL_ISB_IN_LIF_IDX                        ,
    SOC_PPC_FP_QUAL_ISA_FOUND_IN_LIF_IDX                  ,
    SOC_PPC_FP_QUAL_ISA_FOUND                             ,
    SOC_PPC_FP_QUAL_ISA_IN_LIF_IDX                        ,
    SOC_PPC_FP_QUAL_VT_TCAM_MATCH_IN_LIF_IDX              ,
    SOC_PPC_FP_QUAL_VT_TCAM_MATCH                         ,
    SOC_PPC_FP_QUAL_VT_TCAM_IN_LIF_IDX                    ,
    SOC_PPC_FP_QUAL_INITIAL_VSI                           ,
    SOC_PPC_FP_QUAL_IN_RIF_VALID_VRF                      ,
    SOC_PPC_FP_QUAL_IN_RIF_VALID_RIF_PROFILE              ,
    SOC_PPC_FP_QUAL_IN_RIF_VALID_IN_RIF                   ,
    SOC_PPC_FP_QUAL_IN_RIF_VALID                          ,
    SOC_PPC_FP_QUAL_VT_LEM_1ST_LOOKUP_FOUND               ,
    SOC_PPC_FP_QUAL_VT_LEM_1ST_LOOKUP_NOT_FOUND           ,
    SOC_PPC_FP_QUAL_MACT_DOMAIN                           ,
    SOC_PPC_FP_QUAL_MACT_SA_DROP                          ,
    SOC_PPC_FP_QUAL_MACT_IS_LEARN                         ,
    SOC_PPC_FP_QUAL_MACT_DYNAMIC                          ,
    SOC_PPC_FP_QUAL_MACT_DESTINATION                      ,
    SOC_PPC_FP_QUAL_KEY3                                  ,
    SOC_PPC_FP_QUAL_KEY3_16_INST0                         ,
    SOC_PPC_FP_QUAL_KEY3_16_INST1                         ,
    SOC_PPC_FP_QUAL_KEY3_16_INST2                         ,
    SOC_PPC_FP_QUAL_KEY3_32_INST0                         ,
    SOC_PPC_FP_QUAL_KEY3_32_INST1                         ,
    SOC_PPC_FP_QUAL_KEY3_LABEL                            ,
    SOC_PPC_FP_QUAL_KEY3_OUTER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY3_OUTER_VID                        ,
    SOC_PPC_FP_QUAL_KEY3_INNER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY3_INNER_VID                        ,
    SOC_PPC_FP_QUAL_KEY2                                  ,
    SOC_PPC_FP_QUAL_KEY2_16_INST0                         ,
    SOC_PPC_FP_QUAL_KEY2_16_INST1                         ,
    SOC_PPC_FP_QUAL_KEY2_16_INST2                         ,
    SOC_PPC_FP_QUAL_KEY2_32_INST0                         ,
    SOC_PPC_FP_QUAL_KEY2_32_INST1                         ,
    SOC_PPC_FP_QUAL_KEY2_LABEL                            ,
    SOC_PPC_FP_QUAL_KEY2_OUTER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY2_OUTER_VID                        ,
    SOC_PPC_FP_QUAL_KEY2_INNER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY2_INNER_VID                        ,
    SOC_PPC_FP_QUAL_KEY1                                  ,
    SOC_PPC_FP_QUAL_KEY1_16_INST0                         ,
    SOC_PPC_FP_QUAL_KEY1_16_INST1                         ,
    SOC_PPC_FP_QUAL_KEY1_16_INST2                         ,
    SOC_PPC_FP_QUAL_KEY1_32_INST0                         ,
    SOC_PPC_FP_QUAL_KEY1_32_INST1                         ,
    SOC_PPC_FP_QUAL_KEY1_LABEL                            ,
    SOC_PPC_FP_QUAL_KEY1_OUTER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY1_OUTER_VID                        ,
    SOC_PPC_FP_QUAL_KEY1_INNER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY1_INNER_VID                        ,
    SOC_PPC_FP_QUAL_KEY0                                  ,
    SOC_PPC_FP_QUAL_KEY0_16_INST0                         ,
    SOC_PPC_FP_QUAL_KEY0_16_INST1                         ,
    SOC_PPC_FP_QUAL_KEY0_16_INST2                         ,
    SOC_PPC_FP_QUAL_KEY0_32_INST0                         ,
    SOC_PPC_FP_QUAL_KEY0_32_INST1                         ,
    SOC_PPC_FP_QUAL_KEY0_LABEL                            ,
    SOC_PPC_FP_QUAL_KEY0_OUTER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY0_OUTER_VID                        ,
    SOC_PPC_FP_QUAL_KEY0_INNER_VID_VALID                  ,
    SOC_PPC_FP_QUAL_KEY0_INNER_VID                        ,


    /* Port extender etag qualities */
    SOC_PPC_FP_QUAL_PORT_EXTENDER_ETAG                    ,
    SOC_PPC_FP_QUAL_PORT_EXTENDER_ECID                    ,

   /* Additional FWD Header */

   /*
   *  VLAN Tag Id (12b) of the default Ethernet Header. The
   *  default Ethernet Header is the forwarding Header (if
   *  Ethernet), otherwise the outermost Ethernet Header.
   */
    SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG_ID,

   /*
   *  Inner-most VLAN Tag Id (12b) of the default Ethernet Header. The
   *  default Ethernet Header is the forwarding Header (if
   *  Ethernet), otherwise the outermost Ethernet Header.
   *  Inner-most is the inner-tag in case of double tageed packet
   *  or outer in case of single-tagged packet
   */
    SOC_PPC_FP_QUAL_HDR_FWD_INNERMOST_VLAN_TAG_ID,


  /*
   *  Bos expected inidcation for EVPN application
   */
    SOC_PPC_FP_QUAL_KEY0_EVPN_BOS_EXPECTED,
    SOC_PPC_FP_QUAL_KEY1_EVPN_BOS_EXPECTED,
    SOC_PPC_FP_QUAL_KEY2_EVPN_BOS_EXPECTED,
    SOC_PPC_FP_QUAL_KEY3_EVPN_BOS_EXPECTED,
      
      
  /*
   *  Custom PP port 
   */
    SOC_PPC_FP_QUAL_CUSTOM_PP_HEADER_OUTPUT_FP,

   /*
    * FCOE
    */
     SOC_PPC_FP_QUAL_FC_WITH_VFT_D_ID,
     SOC_PPC_FP_QUAL_FC_D_ID,
     SOC_PPC_FP_QUAL_FC_WITH_VFT_VFT_ID,

    /*
	 * new qualifier for 1st pass presel-id (staggered mode)
	 */
    SOC_PPC_FP_QUAL_IRPP_PRESEL_ID,
    SOC_PPC_FP_QUAL_IRPP_PRESEL_PROFILE0,
    SOC_PPC_FP_QUAL_IRPP_PRESEL_PROFILE1,
    SOC_PPC_FP_QUAL_IRPP_PRESEL_PROFILE2,
    SOC_PPC_FP_QUAL_IRPP_PRESEL_PROFILE3,
    SOC_PPC_FP_QUAL_IRPP_PRESEL_PROFILE_KAPS,


    /* 
     * New qualifiers in Jericho+ and QAX
     */
    SOC_PPC_FP_QUAL_IRPP_TCAM_0_RESULT,
    SOC_PPC_FP_QUAL_IRPP_TCAM_1_RESULT,
    SOC_PPC_FP_QUAL_IRPP_TCAM_2_RESULT,
    SOC_PPC_FP_QUAL_IRPP_TCAM_3_RESULT,
    SOC_PPC_FP_QUAL_IRPP_KAPS_PASS1_PAYLOAD,

    /* 
     * Out-LIF range preselector (2b) 
     * Preselector only.
     * See
     *   soc_IPPD_OUT_LIF_RANGEr_fields,
     *   BCM_FIELD_RANGE_OUT_VPORT,
     *   SOC_PPC_FP_CONTROL_TYPE_OUT_LIF_RANGE
     */
    SOC_PPC_FP_QUAL_OUT_LIF_RANGE,
   /*
   *  Number of types in SOC_PPC_FP_QUAL_TYPE
   */
  SOC_PPC_NOF_FP_QUAL_TYPES
}SOC_PPC_FP_QUAL_TYPE;

typedef enum
{
  /*
   *  The packet was bridged according to outer most LL header
   */
  SOC_PPC_FP_FWD_TYPE_BRIDGED = 0,
  /*
   *  The packet was IPv4 UC routed
   */
  SOC_PPC_FP_FWD_TYPE_IPV4_UC = 1,
  /*
   *  The packet was IPv4 MC routed
   */
  SOC_PPC_FP_FWD_TYPE_IPV4_MC = 2,
  /*
   *  The packet was IPv6 UC routed
   */
  SOC_PPC_FP_FWD_TYPE_IPV6_UC = 3,
  /*
   *  The packet was IPv6 MC routed
   */
  SOC_PPC_FP_FWD_TYPE_IPV6_MC = 4,
  /*
   *  The packet was forwarded according to MPLS label,
   *  Ingress-label-mapping
   */
  SOC_PPC_FP_FWD_TYPE_MPLS = 5,
  /*
   *  The packet was forwarded according to TRILL
   */
  SOC_PPC_FP_FWD_TYPE_TRILL = 6,
  /*
   *  The packet was bridged according to inner LL header,
   *  outer LL header was terminated. For example in VPLS
   *  application.
   */
  SOC_PPC_FP_FWD_TYPE_BRIDGED_AFTER_TERM = 7,
  /*
   *  The packet was trapped
   */
  SOC_PPC_FP_FWD_TYPE_CPU_TRAP = 8,
  /*
   *  The packet is not Ethernet-based (e.g., TM traffic).
   */
  SOC_PPC_FP_FWD_TYPE_TM = 9,
  /*
   *  Number of types in SOC_PPC_FP_FWD_TYPE
   */
  SOC_PPC_NOF_FP_FWD_TYPES = 10

    


}SOC_PPC_FP_FWD_TYPE;

typedef enum
{
  SOC_PPC_FP_PARSED_ETHERTYPE_NO_MATCH = 0,
  SOC_PPC_FP_PARSED_ETHERTYPE_USER_DEFINED_FIRST = 1,
  SOC_PPC_FP_PARSED_ETHERTYPE_USER_DEFINED_LAST = 7,
  SOC_PPC_FP_PARSED_ETHERTYPE_TRILL = 8,
  SOC_PPC_FP_PARSED_ETHERTYPE_MAC_IN_MAC = 9,
  SOC_PPC_FP_PARSED_ETHERTYPE_ARP = 10,
  SOC_PPC_FP_PARSED_ETHERTYPE_CFM = 11,
  SOC_PPC_FP_PARSED_ETHERTYPE_FC_E = 12,
  SOC_PPC_FP_PARSED_ETHERTYPE_IPV4 = 13,
  SOC_PPC_FP_PARSED_ETHERTYPE_IPV6 = 14,
  SOC_PPC_FP_PARSED_ETHERTYPE_MPLS = 15,
  /*
   *  Number of types in SOC_PPC_FP_PARSED_ETHERTYPE
   */
  SOC_PPC_NOF_FP_PARSED_ETHERTYPES
}SOC_PPC_FP_PARSED_ETHERTYPE;

typedef enum
{
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_NO_MATCH = 0,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_USER_DEFINED_FIRST = 8,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_USER_DEFINED_LAST = 15,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_TRILL = 6,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_MAC_IN_MAC = 1,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_ARP = 4,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_CFM = 5,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_IPV4 = 2,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_IPV6 = 3,
  SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS_MPLS = 7,
  /*
   *  Number of types in SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS
   */
  SOC_PPC_NOF_FP_PARSED_ETHERTYPE_EGRESSS
}SOC_PPC_FP_PARSED_ETHERTYPE_EGRESS;

typedef enum
{
  SOC_PPC_FP_ETH_ENCAPSULATION_ETH_II = 0,
  SOC_PPC_FP_ETH_ENCAPSULATION_LLC = 1,
  SOC_PPC_FP_ETH_ENCAPSULATION_LLC_SNAP = 2,
  SOC_PPC_FP_ETH_ENCAPSULATION_UNDEF = 3,
  /*
   *  Number of types in SOC_PPC_FP_ETH_ENCAPSULATION
   */
  SOC_PPC_NOF_FP_ETH_ENCAPSULATIONS
}SOC_PPC_FP_ETH_ENCAPSULATION;


typedef enum
{
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_NO_MATCH = 0,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_USER_DEFINED_FIRST = 1,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_USER_DEFINED_LAST = 7,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_TCP = 8,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_UDP = 9,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_IGMP = 10,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_ICMP= 11,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_ICMP_V6 = 12,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_IPV4 = 13,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_IPV6 = 14,
  SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL_MPLS = 15,
  /*
   *  Number of types in SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL
   */
  SOC_PPC_NOF_FP_PARSED_IP_NEXT_PROTOCOLS
}SOC_PPC_FP_PARSED_IP_NEXT_PROTOCOL;

typedef enum
{
  /*
   *  The packet headers are not parsed: all the packet is
   *  considered as payload (e.g., for Raw ports).
   */
  SOC_PPC_FP_PROCESSING_TYPE_RAW = 0,
  /*
   *  The first parsed packet Header is of Ethernet type.
   */
  SOC_PPC_FP_PROCESSING_TYPE_ETH = 1,
  /*
   *  The first parsed packet Header is ITMH
   */
  SOC_PPC_FP_PROCESSING_TYPE_TM = 2,
  /*
   *  The first parsed packet Header is FTMH
   */
  SOC_PPC_FP_PROCESSING_TYPE_FTMH = 3,
  /*
   *  Number of types in SOC_PPC_FP_PROCESSING_TYPE
   */
  SOC_PPC_NOF_FP_PROCESSING_TYPES = 4
}SOC_PPC_FP_PROCESSING_TYPE;

typedef enum
{
  /*
   *  First header as identified by the parser. For example, a
   *  Custom Header before Ethernet, or Sequence Number for
   *  Fat Pipe packets.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_0 = 0,
  /*
   *  Second header as identified by the parser: Ethernet for
   *  Packets with Ethernet header, or ITMH for TM traffic.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_1 = 1,
  /*
   *  Third header as identified by the parser. For example,
   *  IPv4 for IPv4 over Ethernet packets.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_2 = 2,
  /*
   *  Fourth header as identified by the parser. For example,
   *  IPv6 for IPv6 over IPv4 over Ethernet packets.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_3 = 3,
  /*
   *  Fifth header as identified by the parser. For example,
   *  the third MPLS header for MPLS * 3 over Ethernet
   *  packets.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_4 = 4,
  /*
   *  Sixth header as identified by the parser.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_5 = 5,
  /*
   *  Forwarding header.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_FWD = 6,
  /*
   *  Header following the forwarding header.
   */
  SOC_PPC_FP_BASE_HEADER_TYPE_FWD_POST = 7,
  /*
   *  Number of types in SOC_PPC_FP_BASE_HEADER_TYPE
   */
  SOC_PPC_NOF_FP_BASE_HEADER_TYPES = 8
}SOC_PPC_FP_BASE_HEADER_TYPE;

typedef enum
{
  /*
   *  Action on the Destination field. Output value encoded in
   *  17 bits, use the macro SOC_PPC_FP_QUAL_VAL_ENCODE
   *  _FWD_DECISION for its construction.
   */
  SOC_PPC_FP_ACTION_TYPE_DEST = 0,
  /*
   *  Action on the Drop Precedence field. Output value (4
   *  bits): '1' (1b), '1' (1b), Drop Precedence (2b).
   *  For DP-Meter-Command modification, the encoding is:
   *  '1' (1b), '0' (1b), DP-Meter-Command (2b).
   */
  SOC_PPC_FP_ACTION_TYPE_DP = 1,
  /*
   *  Action on the Traffic Class field. Output value (4 bits):
   *  '1' (1b), Traffic Class (3b).
   */
  SOC_PPC_FP_ACTION_TYPE_TC = 2,
  /*
   *  Action on the Forward field. Output value (11 bits):
   *  Strength (3b), Trap code (8b).
   *  In Soc_petra-B, the internal trap code can be converted
   *  from the user trap code via the
   *  soc_pb_pp_trap_mgmt_trap_code_to_internal function.
   *
   */
  SOC_PPC_FP_ACTION_TYPE_TRAP = 3,
  /*
   *  Action on the Snoop field. Output value (10 bits):
   *  Strength (2b), Snoop code (8b).
   */
  SOC_PPC_FP_ACTION_TYPE_SNP = 4,
  /*
   *  Action on the Mirror field. Output value (4 bits): Mirror
   *  profile (3b).
   */
  SOC_PPC_FP_ACTION_TYPE_MIRROR = 5,
  /*
   *  Action on the Outbound-Mirror-Disable field. Output value
   *  (1 bit): Mirror-Disable (1b).
   */
  SOC_PPC_FP_ACTION_TYPE_MIR_DIS = 6,
  /*
   *  Action on the Exclude-Source field. Output value (1 bit):
   *  Exclude-Source (1b).
   */
  SOC_PPC_FP_ACTION_TYPE_EXC_SRC = 7,
  /*
   *  Action on the Ingress Shaping field. Output value (16
   *  bits): IS-Destination (16b).
   */
  SOC_PPC_FP_ACTION_TYPE_IS = 8,
  /*
   *  Action on the Meter field. Output value (14 bits):
   *  Meter-Instance (13b), Meter-Processor (1b).Soc_petra-B only.
   */
  SOC_PPC_FP_ACTION_TYPE_METER = 9,
  /*
   *  Action on the Counter field. Output value (14 bits):
   *  Counter-Instance (13b), Counter-Processor (1b). Soc_petra-B only.
   */
  SOC_PPC_FP_ACTION_TYPE_COUNTER = 10,
  /*
   *  Action on the Statistic-Tag field. Output value (18
   *  bits).
   *  Note: this action is of type TAG, which means that it
   *  does not accept any qualifier mask in case of Direct
   *  Extraction. Besides, databases cannot mix actions of
   *  type TAG and the other actions.
   *  Note: Due to HW implementation reasons, the user
   *  must define besides another Database with
   *  SOC_PPC_FP_ACTION_TYPE_VSQ_PTR as action type for the
   *  same supported PFGs.
   */
  SOC_PPC_FP_ACTION_TYPE_STAT = 11,
  /*
   *  Action on the Out-LIF field. Output value (16 bits):
   *  Out-LIF value (16b).
   */
  SOC_PPC_FP_ACTION_TYPE_OUTLIF = 12,
  /*
   *  Action on the LAG Load-Balancing field. Output value (30
   *  bits).
   *  Note: this action is of type TAG, which means that it
   *  does not accept any qualifier mask in case of Direct
   *  Extraction. Besides, databases cannot mix actions of
   *  type TAG and the other actions.
   */
  SOC_PPC_FP_ACTION_TYPE_LAG_LB = 13,
  /*
   *  Action on the ECMP Load-Balancing field. Output value
   *  (20 bits): ECMP Load-Balancing (20b).
   *  Note: this action is of type TAG, which means that it
   *  does not accept any qualifier mask in case of Direct
   *  Extraction. Besides, databases cannot mix actions of
   *  type TAG and the other actions.
   */
  SOC_PPC_FP_ACTION_TYPE_ECMP_LB = 14,
  /*
   *  Action on the Stacking-Route-History field. Output value
   *  (16 bits): Stacking-Route-History data (16b).
   *  Note: this action is of type TAG, which means that it
   *  does not accept any qualifier mask in case of Direct
   *  Extraction. Besides, databases cannot mix actions of
   *  type TAG and the other actions.
   */
  SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST = 15,
  /*
   *  Action on the VSQ-Pointer field. Output value (8 bits):
   *  the ST-VSQ-Pointer value is modified also according to
   *  the Flow Control mode (set per PP-Port).
   */
  SOC_PPC_FP_ACTION_TYPE_VSQ_PTR = 16,
  /*
   *  Action on the Key of another Database. When this action
   *  is defined for a Database, this Database happens on the
   *  first cycle. The result of this Database is to modify the
   *  Key built in a second-cycle Database (in the location of
   *  the Qualifier SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED).
   *  This type of action is very helpful when the abilities
   *  of the "regular" Databases are not sufficient (for
   *  example, when two database types are simultaneously
   *  required to get the desired action value).
   *  Output value (12 bits): the modified Key value.
   *  Note: in the modified Key, these 12 bits are always
   *  located at the LSB. The second-cycle Database must
   *  be defined previously to this first-cycle Database.
   */
   SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY = 17,
  /*
   *  No action
   */
  SOC_PPC_FP_ACTION_TYPE_NOP = 18,
  /*
   *  Egress Trapping. The trap code range is: 0 - 7.
   */
  SOC_PPC_FP_ACTION_TYPE_EGR_TRAP = 19,
  /*
   *  Modify the Out-TM-Port (aka OFP). Range: 0 - 79.
   */
  SOC_PPC_FP_ACTION_TYPE_EGR_OFP = 20,
  /*
   *  Traffic Class and Drop Precedence modification.
   *  Encoding: {TC(3b), Predefined-DP(1b)}.
   *  The two predefined DP can be set via the
   *  soc_ppd_api_control_set with SOC_PPC_FP_CONTROL_TYPE_EGRESS_DP type.
   */
  SOC_PPC_FP_ACTION_TYPE_EGR_TC_DP = 21,
  /*
   *  Out-LIF (Copy-Unique-Data) modification. Range: 0 -
   *  65K-1.
   */
  SOC_PPC_FP_ACTION_TYPE_EGR_OUTLIF = 22,
  /*
   *  PB: Number of types in SOC_PPC_FP_ACTION_TYPE
   *  ARAD: INVALID action means end of actions
   */
    SOC_PPC_NOF_FP_ACTION_TYPES_PB,

    SOC_PPC_FP_ACTION_TYPE_INVALID = SOC_PPC_NOF_FP_ACTION_TYPES_PB,
	/* 
	 * Arad-Only
	 */
	/* EEI */
	SOC_PPC_FP_ACTION_TYPE_EEI,
	/* InPort */
	SOC_PPC_FP_ACTION_TYPE_IN_PORT,
	/* UserPriority */
	SOC_PPC_FP_ACTION_TYPE_USER_PRIORITY,
	/* MeterA */
	SOC_PPC_FP_ACTION_TYPE_METER_A = SOC_PPC_FP_ACTION_TYPE_METER,
	/* MeterB */
	SOC_PPC_FP_ACTION_TYPE_METER_B = SOC_PPC_FP_ACTION_TYPE_USER_PRIORITY+1,
	/* CounterA */
	SOC_PPC_FP_ACTION_TYPE_COUNTER_A = SOC_PPC_FP_ACTION_TYPE_COUNTER,
	/* CounterB */
	SOC_PPC_FP_ACTION_TYPE_COUNTER_B = SOC_PPC_FP_ACTION_TYPE_METER_B+1,
	/* DpMeterCommand */
	SOC_PPC_FP_ACTION_TYPE_DP_METER_COMMAND,
	/* SrcSystemPortID */
	SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT,     /* 30 */
	/* ForwardingCode */
	SOC_PPC_FP_ACTION_TYPE_FWD_CODE,
	/* ForwardingOffset */
	SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET,
	/* BytesToRemove */
	SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE,
	/* SystemHeaderProfileIndex */
	SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID,
	/* VSI */
	SOC_PPC_FP_ACTION_TYPE_VSI,
	/* OrientationIsHub */
	SOC_PPC_FP_ACTION_TYPE_ORIENTATION_IS_HUB,
	/* VlanEditCommand */
	SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_COMMAND,
	/* VlanEditVid1 */
	SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_1,
	/* VlanEditVid2 */
	SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_2,
	/* VlanEditPcpDei */
	SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_PCP_DEI,     /* 40 */
	/* InRif */
	SOC_PPC_FP_ACTION_TYPE_IN_RIF,
	/* VRF */
	SOC_PPC_FP_ACTION_TYPE_VRF,
	/* InTTL */
	SOC_PPC_FP_ACTION_TYPE_IN_TTL,
	/* InDscpExp */
	SOC_PPC_FP_ACTION_TYPE_IN_DSCP_EXP,
	/* RpfDestinationValid */
	SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION_VALID,
	/* RpfDestination */
	SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION,
	/* IngressLearnEnable */
	SOC_PPC_FP_ACTION_TYPE_INGRESS_LEARN_ENABLE,
	/* EgressLearnEnable */
	SOC_PPC_FP_ACTION_TYPE_EGRESS_LEARN_ENABLE,
	/* LearnFid */
	SOC_PPC_FP_ACTION_TYPE_LEARN_FID,
	/* LearnSa_0to15 */
	SOC_PPC_FP_ACTION_TYPE_LEARN_SA_0_TO_15,      /* 50 */
	/* LearnSa_16to47 */
	SOC_PPC_FP_ACTION_TYPE_LEARN_SA_16_TO_47,
	/* LearnData_0to15 */
	SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_0_TO_15,
	/* LearnData_16to39 */
	SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_16_TO_39,
	/* LearnOrTransplant */
	SOC_PPC_FP_ACTION_TYPE_LEARN_OR_TRANSPLANT,
	/* InLiF */
	SOC_PPC_FP_ACTION_TYPE_IN_LIF,
	/* SequenceNumberTag */
	SOC_PPC_FP_ACTION_TYPE_SEQUENCE_NUMBER_TAG,
	/* IgnoreCp */
	SOC_PPC_FP_ACTION_TYPE_IGNORE_CP,
	/* PphType */
	SOC_PPC_FP_ACTION_TYPE_PPH_TYPE,
	/* PacketIsBootpDhcp */
	SOC_PPC_FP_ACTION_TYPE_PACKET_IS_BOOTP_DHCP,
	/* UnknownAddr */
	SOC_PPC_FP_ACTION_TYPE_UNKNOWN_ADDR,          /* 60 */
	/* FwdHdrEncapsulation */
	SOC_PPC_FP_ACTION_TYPE_FWD_HDR_ENCAPSULATION,
	/* Ieee1588 */
	SOC_PPC_FP_ACTION_TYPE_IEEE_1588,
	/* Oam */
	SOC_PPC_FP_ACTION_TYPE_OAM,
	/* UserHeader1 */
	SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1,
	/* UserHeader2 */
	SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2,
	/*
	 *  InvalidNext
	 */
	SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,
    /* Cos-Profile */
    SOC_PPC_FP_ACTION_TYPE_COS_PROFILE,
    /* Counter-Profile */
    SOC_PPC_FP_ACTION_TYPE_COUNTER_PROFILE,
    /* ACE-Pointer */
    SOC_PPC_FP_ACTION_TYPE_ACE_POINTER,

    /* Arad + actions */
    SOC_PPC_FP_ACTION_TYPE_NATIVE_VSI,                /* 70 */
    SOC_PPC_FP_ACTION_TYPE_IN_LIF_PROFILE,

    /* Combined Actions */

    /* Double Actions */
    SOC_PPC_FP_ACTION_TYPE_COUNTER_AND_METER,
    SOC_PPC_FP_ACTION_TYPE_SNOOP_AND_TRAP,

    /* FLP Actions */
    SOC_PPC_FP_ACTION_TYPE_FLP_START,
    SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_0 = SOC_PPC_FP_ACTION_TYPE_FLP_START,
    SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_1,
    SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_2,
    SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_3,
    SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_4,
    SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_5,
    SOC_PPC_FP_ACTION_TYPE_FLP_LEM_1ST_RESULT_0, /* first 32 bit of the payload */
    SOC_PPC_FP_ACTION_TYPE_FLP_LEM_1ST_RESULT_1,/*  last 16 bit of the payload */
    SOC_PPC_FP_ACTION_TYPE_FLP_LEM_2ND_RESULT_0,
    SOC_PPC_FP_ACTION_TYPE_FLP_LEM_2ND_RESULT_1,
    SOC_PPC_FP_ACTION_TYPE_FLP_END = SOC_PPC_FP_ACTION_TYPE_FLP_LEM_2ND_RESULT_1,

    /* SLB actions */
    SOC_PPC_FP_ACTION_TYPE_SLB_HASH_VALUE,            /* 84 */

    /* Jericho action */
    SOC_PPC_FP_ACTION_TYPE_IN_RIF_PROFILE,
    SOC_PPC_FP_ACTION_TYPE_ACE_TYPE,

    /*
     *  Action trap. Output value (11 bits):
     *  Strength (3b), Trap code (8b).
     */
    SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED,             /* 83 */

    /* QAX action :
     *  Action dest-drop (1b)
     */
    SOC_PPC_FP_ACTION_TYPE_DEST_DROP,                /* 85 */



    /* QAX actions */
    SOC_PPC_FP_ACTION_TYPE_USER_HEADER_3,        
    SOC_PPC_FP_ACTION_TYPE_USER_HEADER_4,            
    SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1_TYPE,
    SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2_TYPE,       
    SOC_PPC_FP_ACTION_TYPE_USER_HEADER_3_TYPE,       
    SOC_PPC_FP_ACTION_TYPE_USER_HEADER_4_TYPE,       
    SOC_PPC_FP_ACTION_TYPE_ITPP_DELTA,               
    SOC_PPC_FP_ACTION_TYPE_STATISTICS_POINTER_0,     
    SOC_PPC_FP_ACTION_TYPE_STATISTICS_POINTER_1,     
    SOC_PPC_FP_ACTION_TYPE_ADMIT_PROFILE,            
    SOC_PPC_FP_ACTION_TYPE_LATENCY_FLOW_ID,          
    SOC_PPC_FP_ACTION_TYPE_PPH_RESERVE_VALUE,        
    SOC_PPC_FP_ACTION_TYPE_PEM_CONTEXT,              
    SOC_PPC_FP_ACTION_TYPE_PEM_GENERAL_DATA_0,       
    SOC_PPC_FP_ACTION_TYPE_PEM_GENERAL_DATA_1,
    SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_0,  
    SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_1,
    SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_2,
    SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_3,
    SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_KAPS,     

    /* this used as number of possible acction in ARAD
     * besides common code for PB/ARAD should use this enum value
     */
    SOC_PPC_NOF_FP_ACTION_TYPES,
    SOC_PPC_NOF_FP_ACTION_TYPES_ARAD = SOC_PPC_NOF_FP_ACTION_TYPES

}SOC_PPC_FP_ACTION_TYPE;
/*
 * Range of values for SOC_PPC_FP_ACTION_TYPE_ACE_TYPE
 */
typedef enum
{
  FIRST_SOC_PPC_FP_ACE_TYPE_VALUES,
    /*
     * Ace is NULL: Redirection only.
     */
  ACE_TYPE_VALUE_NULL = FIRST_SOC_PPC_FP_ACE_TYPE_VALUES,
    /*
     * Ace overrides CUD: Counting and (redirection and/or outlif)
     */
  ACE_TYPE_VALUE_OVERRIDES_CUD,
    /*
     * Ace is counter pointer: Counting only.
     */
  ACE_TYPE_VALUE_COUNTER_ONLY,
  NUM_SOC_PPC_FP_ACE_TYPE_VALUES
} SOC_PPC_FP_ACE_TYPE_VALUES ;

typedef enum
{
  /*
   *  TCAM database: when a packet is qualified to an entry in
   *  the database, the entry's actions are applied to
   *  it. Entries are inserted to this database using the API
   *  soc_ppd_fp_entry_add().
   */
  SOC_PPC_FP_DB_TYPE_TCAM = 0,
  /*
   *  Direct Access Table database: when a packet is qualified
   *  to an entry in the database, the entry's actions are
   *  applied to it. Up to one Database can be created of this
   *  type. Entries are inserted to this database using the API
   *  soc_ppd_fp_entry_add().
   */
  SOC_PPC_FP_DB_TYPE_DIRECT_TABLE = 1,
  /*
   *  Direct Extraction Database: When a packet is qualified
   *  to an entry in the database, the entry's action applies
   *  configurable logic to bits extracted from specific
   *  qualifier fields, and uses the result as the resolved
   *  action value. E.g.: One can define a database that
   *  attaches packets to meters according to incoming
   *  Attachment Circuit. A single entry in this database can
   *  attach meters 2K-3K to packets with In-ACs 0-1K,
   *  Respectively. Entries are inserted to this database
   *  using the API soc_ppd_fp_direct_extraction_entry_add()
   */
  SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION = 2,
  /*
   *  TCAM Database with actions changed at egress. At egress,
   *  a packet is mapped to a single Database according to the
   *  soc_ppd_fp_egr_db_map_set API. For a same Database, all the
   *  Egress actions can be done altogether. Up to 7 Egress
   *  Databases can be defined. Entries are inserted to this
   *  database using the API soc_ppd_fp_entry_add ().
   */
  SOC_PPC_FP_DB_TYPE_EGRESS = 3,
  SOC_PPC_NOF_FP_DATABASE_TYPES_PETRA_B = 4,
  /*
   * FLP stage - External TCAM
   */
  SOC_PPC_FP_DB_TYPE_FLP = SOC_PPC_NOF_FP_DATABASE_TYPES_PETRA_B,
  /*
   * SLB stage - Hashing, no real action
   */
  SOC_PPC_FP_DB_TYPE_SLB,
  SOC_PPC_FP_DB_TYPE_VT,
  SOC_PPC_FP_DB_TYPE_TT,
  /*
   *  Number of types in SOC_PPC_FP_DATABASE_TYPE
   */
  SOC_PPC_NOF_FP_DATABASE_TYPES
}SOC_PPC_FP_DATABASE_TYPE ;

typedef enum
{
  /*
   *  Ingress PMF - regular one (default).
   */
  SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
    /*
     *  Egress PMF - single one at egress.
     */
  SOC_PPC_FP_DATABASE_STAGE_EGRESS,
    /*
     *  Ingress FLP block (used also for External lookup).
     */
  SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
    /*
     *  Ingress SLB block (used for stateful load balancing).
     */
  SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB,
    /*
     * Number of PMF stages on Arad and Jericho.
     */
  SOC_PPC_NOF_FP_DATABASE_STAGES_ARAD,
     /*
      *  VT block.
      */
  SOC_PPC_FP_DATABASE_STAGE_INGRESS_VT = SOC_PPC_NOF_FP_DATABASE_STAGES_ARAD,
  /*
   *  TT block.
   */
  SOC_PPC_FP_DATABASE_STAGE_INGRESS_TT,
  /*
   *  Number of types in SOC_PPC_FP_DATABASE_STAGE
   */
  SOC_PPC_NOF_FP_DATABASE_STAGES
}SOC_PPC_FP_DATABASE_STAGE;

typedef enum
{
  /*
   *  The database builds an ACL Key according to the
   *  predefined Layer 2 (Ethernet) ACL Key.
   */
  SOC_PPC_FP_PREDEFINED_ACL_KEY_L2 = 0,
  /*
   *  The database builds an ACL Key according to the
   *  predefined Layer 3 (IPv4) ACL Key.
   */
  SOC_PPC_FP_PREDEFINED_ACL_KEY_IPV4 = 1,
  /*
   *  The database builds an ACL Key according to the
   *  predefined Layer 3 (IPv6) ACL Key.
   */
  SOC_PPC_FP_PREDEFINED_ACL_KEY_IPV6 = 2,
  /*
   *  The database builds an ACL Key according to the
   *  predefined Egress Ethernet ACL Key.
   */
  SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_ETH = 3,
  /*
   *  The database builds an ACL Key according to the
   *  predefined Egress IPv4 ACL Key.
   */
  SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_IPV4 = 4,
  /*
   *  The database builds an ACL Key according to the
   *  predefined Egress TM ACL Key.
   */
  SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_TM = 5,
  /*
   *  The database builds an ACL Key according to the
   *  predefined Egress MPLS ACL Key.
   */
  SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_MPLS = 6,
  /*
   *  Number of types in SOC_PPC_FP_PREDEFINED_ACL_KEY
   */
  SOC_PPC_NOF_FP_PREDEFINED_ACL_KEYS = 7
}SOC_PPC_FP_PREDEFINED_ACL_KEY ;

typedef enum
{
  /*
   *  Set a L4Ops range. The qualifier value L4Ops is a bitmap
   *  of the source & destination ports in the found
   *  ranges.'val_ndx' indicates the L4Ops range index.
   *  Range: 0 - 23. Val[0]
   *  (resp. Val[1]) is the min (resp. max) source port.
   *  Val[2] (resp. Val[3]) is the min (resp. max) destination
   *  port.
   */
  SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE = 0,
  /*
   *  Set a packet size range. The qualifier value Packet size
   *  is set according to the first found range (inclusive
   *  comparison).'val_ndx' indicates the Packet size range. Range: 0 - 2.
   *  index. Val[0] (resp. Val[1]) is the min (resp. max)
   *  packet size of this range.
   *  The packet size measured corresponds to the packet size
   *  if under 128 Bytes, otherwise is equal to 128 Bytes.
   */
  SOC_PPC_FP_CONTROL_TYPE_PACKET_SIZE_RANGE = 1,
  /*
   *  Add an EtherType (Ethernet Next-Protocol). By default,
   *  the EtherTypes of MPLS, MAC in MAC, ARP, CFM, IPv4,
   *  IPv6, FCoE and TRILL are always identified. Up to 7
   *  user-defined EtherTypes can be defined. 'val_ndx'
   *  indicates the EtherType value (e.g., IPv4 is 0x0800) -
   *  (16b). Val[0] indicates if this EtherType must be
   *  identified (1b).
   *  In the get API, Val[1] indicates the encoded
   *  EtherType index.
   */
  SOC_PPC_FP_CONTROL_TYPE_ETHERTYPE = 2,
  /*
   *  Add an IP Next-Protocol to be identified. By default,
   *  the Next-Protocols of TCP, UDP, IGMP, ICMP, ICMPv6,
   *  IPv4, IPv6 and MPLS are always identified. Up to 7
   *  user-defined IPv4 Next-Protocol can be defined. The
   *  Next-Protocol values are shared between the Ipv4 and the
   *  IPv6 packets. 'val_ndx' indicates the IP Next-Protocol
   *  value (e.g., TCP is 6) - (8b).val[0] indicates if this
   *  IP Next-Protocol must be identified (1b).
   *  In the get API, Val[1] indicates the encoded IP
   *  Next-Protocol index.
   */
  SOC_PPC_FP_CONTROL_TYPE_NEXT_PROTOCOL_IP = 3,
  /*
   *  Set the data of each PP-Port for the Egress Databases.
   *  'val_ndx' indicates the PP-Port index. Range: 0 -
   *  63 (256 in Arad).val[0] indicates the PP-Port data (6b / 32b).
   */
  SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA = 4,
  /*
   *  Set the L2 EtherTypes indexes for the Ethernet Egress
   *  Databases. Up to 8 User-Defined L2 EtherTypes can be defined.
   *  'val_ndx' indicates the Next-Protocol index. Range: 8 - 15.
   *  val[0] indicates the L2 EtherTypes value (8b).
   *  Note: the lowest EtherType values are: 0 - No match,
   *  1 - MAC-In-MAC, 2 - IPv4, 3 - IPv6, 4 - ARP, 5 - CFM,
   *  6 - Trill, 7 - MPLS.
   */
  SOC_PPC_FP_CONTROL_TYPE_EGR_L2_ETHERTYPES = 5,
  /*
   *  Set the IPv4 Next-Protocol indexes for the IPv4 Egress
   *  Databases. Up to 15 User-Defined IPv4 Next-Protocols can
   *  be defined. 'val_ndx' indicates the Next-Protocol index.
   *  Range: 1 - 15.val[0] indicates the IPv4 Next-Protocol
   *  value (8b).
   */
  SOC_PPC_FP_CONTROL_TYPE_EGR_IPV4_NEXT_PROTOCOL = 6,
  /*
   *  Set the PP-Port profile. This PP-Port profile and the
   *  packet header type is mapped to an Egress ACL Database
   *  according to the soc_ppd_fp_egr_db_map_set API. 'val_ndx'
   *  indicates the PP-Port. Range: 0 - 63.val[0] indicates
   *  the PP-Port profile value (2b).
   */
  SOC_PPC_FP_CONTROL_TYPE_PP_PORT_PROFILE = 7,
  /*
   *  Modify the predefined L2 & L3 ACL Key format. If
   *  enabled, the In-LIF field replaces the Inner-VID in the
   *  L2 Key and the L3 IPv4 Key. Must be set before any
   *  Database definition. 'val_ndx' is not used. val[0]
   *  indicates if the In-LIF is part of the Key (value 1) or
   *  not (0).
   */
  SOC_PPC_FP_CONTROL_TYPE_L2_L3_KEY_IN_LIF_ENABLE = 8,
  /*
   *  Modify the predefined L3 ACL IPv6 Key format. If
   *  enabled, the TCP-Control replaces the PP-Port in the L3
   *  IPv6 Key. Must be set before any Database definition.
   *  'val_ndx' is not used val[0] indicates if the TCP-Control
   *  is part of the Key (value 1) or not (0).
   */
  SOC_PPC_FP_CONTROL_TYPE_L3_IPV6_TCP_CTL_ENABLE = 9,
  /*
   *  Define a User-Defined field. Must be set before the
   *  Database definition using it. 'val_ndx' indicates the
   *  User-Defined field index. Range: 0 - 9.val[0] indicates
   *  the Base-Header type (via the SOC_PPC_FP_BASE_HEADER_TYPE
   *  enum). val[1] indicates the offset from this
   *  Base-Header. Units: bit. Resolution: nibbles (4b).
   *  Range: 0 - 508.val[2] indicates the number of bits to
   *  copy. Units: bit. Resolution: bit. Range: 1- 32.
   */
  SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF = 10,
  /*
   * Define a Drop Precedence value. Two Drop precedence
   *  values can be set and are used in the Egress action.
   *  'val_ndx' indicates the DP field index. Range: 0 - 1.
   *  val[0] indicates the DP value. Range: 0- 3.
   */
  SOC_PPC_FP_CONTROL_TYPE_EGRESS_DP = 11,
  /*
   * Define the number of VLAN tags in the inner Ethernet
   * header per Packet-Format-Group. 'val_ndx' indicates
   * the Packet-Format-Group index. Range: 0 - 4.
   * val[0] indicates the number of VLAN tags in the
   * inner Ethernet header. Range: 0- 2.
   */
   SOC_PPC_FP_CONTROL_TYPE_INNER_ETH_NOF_VLAN_TAGS = 12,
  /*
   * Define the size in bits of the Key change.
   * 'val_ndx' is not used and must be Null.
   * val[0] indicates the size in bits of the Key change
   * qualifier. Range: 1 - 12.
   */
   SOC_PPC_FP_CONTROL_TYPE_KEY_CHANGE_SIZE = 13,
  /*
   *  Set the data of each TM/PP-Port for the Egress/Ingress Databases.
   */
  SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA,
  SOC_PPC_FP_CONTROL_TYPE_FLP_PP_PORT_DATA,
    SOC_PPC_FP_CONTROL_TYPE_ING_PP_PORT_DATA,
    SOC_PPC_FP_CONTROL_TYPE_ING_TM_PORT_DATA,
  /*
   *  Set an out-lif range. The qualifier value OUT-LIF range
   *  is set according to the first found range (inclusive
   *  comparison).'val_ndx' indicates the Out-LIF range. Range: 0 - 1.
   *  index. Val[0] (resp. Val[1]) is the min (resp. max)
   *  of this range.
   */
  SOC_PPC_FP_CONTROL_TYPE_OUT_LIF_RANGE,
    /*
     *  Set an ACE-Pointer PP_Port configuration.
     *  'val_ndx' indicates the ACE-Pointer. Range: 0 - 4K-1.
     *  index. Val[0] is the PP-Port.
     */
    SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_PP_PORT,
    /*
     *  Set an ACE-Pointer PP_Port configuration.
     *  'val_ndx' indicates the ACE-Pointer. Range: 0 - 4K-1.
     *  index. Val[0] is PRGE_VAR.
     */
    SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_ONLY,
    /*
     *  Set an ACE-Pointer outlif configuration.
     *  'val_ndx' indicates the ACE-Pointer. Range: 0 - 1K-1.
     *  index. Val[0] is the Outlif-id.
     */
    SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_OUT_LIF,
  /*
   *  Set the In-Port profile. This In-Port profile is used
   *  for pre-selection. Packets which arrive from in-ports
   *  that belong to this profile will be handled with
   *  appropriate program. 'val_ndx' indicates the Profile
   *  (1-7, 0 is reserved). Range: 0 - 255.val[0] indicates
   *  the In-Ports that belong to this profile.
   */
  SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE,

  /*
   *  Set the Out-Port profile. This Out-Port profile is used
   *  for pre-selection. Packets which are destined to out-ports
   *  that belong to this profile will be handled with
   *  appropriate program. 'val_ndx' indicates the Profile
   *  (1-7, 0 is reserved). Range: 0 - 255.val[0] indicates
   *  the Out-Ports that belong to this profile.
   */
  SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE,

  /*
   *  Set the FLP program processing profile. This profile is used
   *  for pre-selection in FLP and PMF, and for key in PMF.
   *  'val_ndx' indicates the Profile
   *  (1-7, 0 is reserved). Range: 0 - 23.val[0] indicates
   *  the programs that belong to this profile.
   */
  SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE,

  SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE,

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /*
   *  Set KBP caching option. If set, configured KBP entries will
   *  be cached. In this case, cached KBP entries will be commited
   *  only when commit call is evoked (using SOC_PPC_FP_CONTROL_TYPE_KBP_COMMIT).
   *  If not set, KBP entries will be committed to HW regularly
   *  (similar to internal TCAM). 'val_ndx' is not used. 
   *  val[0] indicates the if caching is set (value 1) or not (0).
   */
  SOC_PPC_FP_CONTROL_TYPE_KBP_CACHE,

  /*
   *  Commit KBP cached configuration.
   *  'val_ndx' is not used.
   */
  SOC_PPC_FP_CONTROL_TYPE_KBP_COMMIT,
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

  /*
   *  Number of types in SOC_PPC_FP_CONTROL_TYPE
   */
  SOC_PPC_NOF_FP_CONTROL_TYPES
}SOC_PPC_FP_CONTROL_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Outer Tag. Range: 0 - 3
   */
  SOC_SAND_PP_VLAN_TAG_TYPE tag_outer;
  /*
   *  Inner Tag. Range: 0 - 3
   */
  SOC_SAND_PP_VLAN_TAG_TYPE tag_inner;
  /*
   *  If True, then Priority Tag.
   */
  uint8 is_priority;

} SOC_PPC_FP_ETH_TAG_FORMAT;


typedef enum
{
  /*
   *  For macro SOC_PPC_FP_QUAL_VAL_ENCODE_FWD_DECISION.
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_FWD_DECISION = 0,
  /*
   *  For macro SOC_PPC_FP_QUAL_VAL_ENCODE_MAC_ADDRESS.
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_MAC_ADDRESS = 1,
  /*
   *  For macro SOC_PPC_FP_QUAL_VAL_ENCODE_IPV4_SUBNET.
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_IPV4_SUBNET = 2,
  /*
   *  For macro SOC_PPC_FP_QUAL_VAL_ENCODE_IPV6_SUBNET.
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_IPV6_SUBNET = 3,
  /*
   *  For macro SOC_PPC_FP_QUAL_VAL_ENCODE_ETH_TAG_FORMAT.
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_ETH_TAG_FORMAT = 4,
  /*
   *  For macro SOC_PPC_FP_QUAL_VAL_ENCODE_FTMH.
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_FTMH = 5,
  /*
   *  Number of types in SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE
   */
  SOC_PPC_NOF_FP_QUAL_VAL_ENCODE_INFO_TYPES = 6
}SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE ;


typedef struct
{
  SOC_PPC_FRWRD_DECISION_INFO fwd_dec;
  uint32 dest_nof_bits;
} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_FWD_DECISION;

typedef struct
{
  SOC_SAND_PP_MAC_ADDRESS  mac;
  SOC_SAND_PP_MAC_ADDRESS  is_valid;
} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_MAC;

typedef struct
{
  SOC_PPC_FP_ETH_TAG_FORMAT tag_format;
} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_ETF;


typedef struct
{
  SOC_SAND_PP_IPV4_ADDRESS ip;
  uint32 subnet_length;
} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_IPV4;

typedef struct
{
  SOC_SAND_PP_IPV6_ADDRESS ip;
  uint32 subnet_length;
  uint8 is_low;
} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_IPV6;

typedef struct
{
  SOC_PPC_FP_FTMH ftmh;
} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_FTMH;

typedef union
{
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_FWD_DECISION fd;
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_MAC mac;
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_IPV4 ipv4;
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_IPV6 ipv6;
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_ETF etf;
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL_FTMH ftmh;

} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL;


typedef struct
{
  /*
   *  Qual val encode type: MAC, IPv4, ..
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE type;
  /*
   *  Qualifier value (union according to type).
   */
  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_VAL val;

} SOC_PPC_FP_QUAL_VAL_ENCODE_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Qualifier Field type.
   */
  SOC_PPC_FP_QUAL_TYPE type;
  /*
   *  Qualifier field rule value. The 'is_valid' parameter is
   *  used as a mask. A Packet field value is qualified for
   *  this Qualifier Field type if the Packet field value
   *  masked by 'is_valid' equals the Qualifier field value
   *  'val'.
   */
  SOC_SAND_U64 val;
  /*
   *  Qualifier field mask value. Bitmap parameter of 64 bits.
   *  For each bit, if set, then the respective bit value is
   *  significant. Otherwise, the bit value of the rule value
   *  'val' is ignored.
   */
  SOC_SAND_U64 is_valid;

} SOC_PPC_FP_QUAL_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * 1st Pass presel-id
   */
  uint32 first_pass_presel_id; 
  /*
   * 2nd Pass presel-id
   */
  uint32 second_pass_presel_id; 
  /*
   * presel_res_0_val
   */
  uint32 presel_res_0_key;
    /*
   * presel_res_1_val
   */
  uint32 presel_res_1_key;
    /*
   * presel_res_2_val
   */
  uint32 presel_res_2_key;
    /*
   * presel_res_3_val
   */
  uint32 presel_res_3_key;
   /*
   * presel_kaps_val
   */
  uint32 presel_kaps_key;

  uint8 valid;

}SOC_PPC_FP_DATABASE_STAGGERED_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Database type: The database may be:TCAM, Direct Table,
   *  or Direct Extraction: Enable using determining the
   *  action value, according to the qualifier value. for other cases the db_type signals the stage
   */
  SOC_PPC_FP_DATABASE_TYPE db_type;
  /*
   *  Packet Format Groups (PFGs) bitmap for which this
   *  Database-ID must be performed. The PFGs are configured
   *  via the soc_ppd_fp_packet_format_group_set API. Not relevant
   *  for Egress Database. Petra-B only.
   */
  uint32 supported_pfgs;

  /* For Arad, support up to 48 PFGs */
  uint32 supported_pfgs_arad[SOC_PPC_FP_NOF_PFGS_IN_LONGS_ARAD];
  /*
   *  Types of the Qualifier fields for this database. Selects
   *  the header / processing attributes, to be searched in
   *  the database. For a Database of type 'Direct Table', the
   *  sum of the qualifier sizes must be inferior to 10
   *  bits. For Databases using predefined ACL Keys (L2, L3
   *  IPv4 or IPv6 or Egress), use the SOC_PPC_FP_QUAL_TYPE_PRESET
   *  to define the qualifier types.
   */
  SOC_PPC_FP_QUAL_TYPE qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
  /*
   *  Action types applied by the database. Select the list of
   *  actions to be applied to matching entries. For the
   *  Egress Database, the action type list can be set using
   *  the macro SOC_PPD_FP_EGRESS_ACTION_TYPE_PRESET
   */
  SOC_PPC_FP_ACTION_TYPE action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  /*
   * Action widths for each action inside the database.
   * The action width determines how much bits the action is
   * going to really use.  This is reflected by allocating the
   * exact needed bits in the action buffer, and by updating
   * the FES_VALID_BITS field accordingly inside the
   * IHB_PMF_FES_PROGRAM memory.
   * This struct is ordered in compliance with action_types.
   * This logic is used starting JER+/QAX.
   */
  uint32                 action_widths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  /*
   *  Database strength (i.e., priority compared to the other
   *  databases). When several databases result in updating the
   *  same action, the database priority selects the action to
   *  be committed. Highest priority: 0. Lowest priority: 127.
   *  In Arad, invert the order for BCM compatibility:
   *  Highest priority: 127. Lowest priority: 0.
   */
  uint32 strength;

  /* 
   * Coupled cascaded Database Id 
   * Relevant only in case of Cascaded Database. 
   */
  uint32 cascaded_coupled_db_id;

  /* 
   * DB flags - according to SOC_PPC_FP_DATABASE_INFO_FLAGS
   */
  uint32 flags;


    /* Min Priority - the minimal priority (logically, not numerically)
	 * value that shall be set for an entry in this table.
	 * Only relevant to external TCAM.
	 */
  uint32 min_priority;

  /* 
   * vlan translation classification Id 
   * Relevant only in case of Vlan translation stage. 
   */
  uint32 internal_table_id; /* the internal table ID that it is used for the group */

  uint32 handle_by_key_entry_id; /* a unique entry for groups that handles entry by key */

  uint32 physicalDB; /* phsical DB type incase that we are using DBAL */

  SOC_PPC_FP_DATABASE_STAGGERED_INFO db_staggered_info[SOC_DPP_NOF_INGRESS_PMF_PROGRAM_SELECTION_LINES_ARAD];

} SOC_PPC_FP_DATABASE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Action type done by the entry.
   */
  SOC_PPC_FP_ACTION_TYPE type;
  /*
   *  Action value, according to the action type. E.g.: When
   *  the type is Out-LIF-update, and 'action_val' is 6, the
   *  action is attaching the packet to Out-LIF #6.
   */
  uint32 val;

} SOC_PPC_FP_ACTION_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Qualifier field values of the database entry. All the
   *  Qualifier field types must match the qualifier types
   *  list defined for the database.
   */
  SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
  /*
   *  Actions of this Entry. All the action types must match
   *  the action types list defined for the database
   */
  SOC_PPC_FP_ACTION_VAL actions[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  /*
   *  Entry priority inside the database. Highest priority: 0.
   *  Lowest priority: 16K-1.
   */
  uint32 priority;

  /* 
   * If True, then entry is not valid but the place is saved with 
   * its content. Single difference: valid bit unset. Arad-only.
   */
  uint8 is_invalid;
  /* 
   * If True, then entry is already existing, just 
   * its content is updated. Arad-only.
   */
  uint8 is_for_update;
  /* 
   * If True, then the entry is inserted in the top of 
   * the bank and not in the middle 
   * Relevant for Arad when inserting from Field group 
   * install and not entry install (many entries in a row 
   * sorted by decreasing priority) 
   */
  uint8 is_inserted_top;

} SOC_PPC_FP_ENTRY_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Extracted Field type.
   */
  SOC_PPC_FP_QUAL_TYPE type;
  /*
   *  LSB of the field where the value extraction starts. Is
   *  ignored and set to '0' if the field type refers to an
   *  enum (the all field is extracted).
   */
  uint32 fld_lsb;
  /*
   *  Constant value to insert. If set, this value is inserted
   *  instead of the Qualifier of type 'type'. In this case,
   *  only 'nof_bits' must be set.
   *  For a '0' value to insert, set a high Constant value where only
   *  the lsb (i.e., nof_bits bits) are null.
   */
  uint32 cst_val;
  /*
   *  Number of bits to extract. Is ignored if the field type
   *  refers to an enum (the effective number set corresponds
   *  to the minimum number of bits necessary to encode all
   *  the field values, i.e. 4 values are encoded in 2
   *  bits). Range: 0 - 17. The sum of all the extracted field
   *  size must be under the action size (determined according
   *  to the action type).
   */
  uint32 nof_bits;

} SOC_PPC_FP_DIR_EXTR_ACTION_LOC;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Action type done by the entry.
   */
  SOC_PPC_FP_ACTION_TYPE type;
  /*
   *  Indicate the composition of the action value: which
   *  field and the number of bits to extract. When extracting
   *  several qualifier values fld_ext[0] result is the result
   *  LSB, and fld_ext[nof_fields-1] result will be the MSB.
   */
  SOC_PPC_FP_DIR_EXTR_ACTION_LOC fld_ext[SOC_PPC_FP_DIR_EXTR_MAX_NOF_FIELDS];
  /*
   *  The number of valid entries in 'fld_ext'
   */
  uint32 nof_fields;
  /*
   *  Base-value to add to the output field value. Range: 0 -
   *  16K-1.
   */
  uint32 base_val;

} SOC_PPC_FP_DIR_EXTR_ACTION_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Qualifier field values of the database entry. Enable
   *  specifying packets to the direct extraction entries,
   *  according to the packet / processing attributes. All the
   *  Qualifier field types must be part of the Database
   *  Qualifier field list. For Soc_petra-B, at the most a single
   *  qualifier field is supported with at the most a 4
   *  consecutive bits mask
   */
  SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
  /*
   *  Entry action values. All the action types must be part of
   *  the Database Action type list.
   */
  SOC_PPC_FP_DIR_EXTR_ACTION_VAL actions[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  /*
   *  Priorities entries inside the database. Highest
   *  priority: 0. Lowest priority: 7.
   *  In Arad, invert the order for BCM compatibility:
   *  Highest priority: 7. Lowest priority: 0.
   */
  uint32 priority;

} SOC_PPC_FP_DIR_EXTR_ENTRY_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Database index. Range: 0 - 127.
   */
  uint32 db_id;
  /*
   *  Control type
   */
  SOC_PPC_FP_CONTROL_TYPE type;
  /*
   *  Value index. Meaningful according to the 'type'.
   */
  uint32 val_ndx;
  /*
   *  When TRUE value should be cleared
   */
  uint8 clear_val;

} SOC_PPC_FP_CONTROL_INDEX;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Control Values. Their meaning depends on the control
   *  type
   */
  uint32 val[SOC_PPC_FP_NOF_CONTROL_VALS];

} SOC_PPC_FP_CONTROL_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  TM Port of the Packet
   */
  uint32 tm_port;
  /*
   *  PP Port of the Packetheader_type SOC_PETRA_PORT_HEADER_TYPE
   *  Header type parsed
   */
  uint32 pp_port;
  /*
   *  Packet header
   */
  SOC_TMC_PORT_HEADER_TYPE header_type;
  /*
   *  HW PFC
   */
  int pfc_hw;
  /*
   *  VLAN Tag structure as parsed
   */
  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT vlan_tag_structure;

} SOC_PPC_FP_PACKET_DIAG_PARSER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  PP Port attributes of this program
   */
  SOC_PPC_FP_DIAG_PP_PORT_INFO pp_port_info;
  /*
   *  Packet-Format-Group id
   */
  uint32 pfg_id[SOC_PPC_NOF_FP_DATABASE_STAGES];
  /*
   *  Program id
   */
  uint32 pgm_id[SOC_PPC_NOF_FP_DATABASE_STAGES];

} SOC_PPC_FP_PACKET_DIAG_PGM;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Qualifier type
   */
  SOC_PPC_FP_QUAL_TYPE type;
  /*
   *  Qualifier value. The relevant bits depend on
   *  the size of the considered qualifier
   */
  uint32 val[2];

} SOC_PPC_FP_PACKET_DIAG_QUAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Database ID
   */
  uint32 db_id;
  /* 
   * Database stage - useful for the prints
   */
  SOC_PPC_FP_DATABASE_STAGE stage;
  /*
   *  Qualifier type and value
   */
  SOC_PPC_FP_PACKET_DIAG_QUAL qual[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

} SOC_PPC_FP_PACKET_DIAG_DB_QUAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Qualifier values per Database
   */
  SOC_PPC_FP_PACKET_DIAG_DB_QUAL db_id_quals[SOC_PPC_FP_NOF_DBS];

} SOC_PPC_FP_PACKET_DIAG_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, an entry matched
   */
  uint8 is_match;
  /*
   *  Database ID
   */
  uint32 db_id;
  /*
   *  Actions types and value
   */
  SOC_PPC_FP_ACTION_VAL actions[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];

} SOC_PPC_FP_PACKET_DIAG_TCAM_DT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Database ID
   */
  uint32 db_id;
  /*
   *  Entry ID (in case of Direct Extraction)
   */
  uint32 entry_id;
  /*
   *  Actions types and value
   */
  SOC_PPC_FP_ACTION_VAL input_actions[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  /*
   *  Value of the qualifier used for the mask (in case of
   *  Direct Extraction)
   */
  SOC_PPC_FP_PACKET_DIAG_QUAL qual_mask;
  /*
   *  Action done in this macro
   */
  SOC_PPC_FP_ACTION_VAL action;

} SOC_PPC_FP_PACKET_DIAG_MACRO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Database ID
   */
  uint32 db_id;
  /*
   *  Action done in this macro
   */
  SOC_PPC_FP_ACTION_VAL action;

} SOC_PPC_FP_PACKET_DIAG_MACRO_SIMPLE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Lookup hit
   */
  uint32 hit;
  /*
   *  Action done in this macro
   */
  SOC_PPC_FP_ACTION_VAL action;

} SOC_PPC_FP_PACKET_DIAG_ACTION_ELK;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Parser information identifying the packet format and
   *  header types.
   */
  SOC_PPC_FP_PACKET_DIAG_PARSER parser;
  /*
   *  FP Program attributes
   */
  SOC_PPC_FP_PACKET_DIAG_PGM pgm;
  /*
   *  Built key to use
   */
  SOC_PPC_FP_PACKET_DIAG_KEY key;
  /*
   *   TCAM matches per bank and cycle
   */
  SOC_PPC_FP_PACKET_DIAG_TCAM_DT tcam[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_TCAM_NOF_BANKS];
  /*
   *  Direct Table match
   */
  SOC_PPC_FP_PACKET_DIAG_TCAM_DT dt[SOC_PPC_FP_NOF_CYCLES];
  /*
   *  Indicates the new action values - with FEM.
   */
  SOC_PPC_FP_PACKET_DIAG_MACRO macro[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_MACROS];
  /*
   *  Indicates the new action values - with FES.
   */
  SOC_PPC_FP_PACKET_DIAG_MACRO_SIMPLE macro_simple[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_MACRO_SIMPLES];
  /*
   *  Indicates the new action values - at egress.
   */
  SOC_PPC_FP_ACTION_VAL egress_action[SOC_DPP_DEFS_MAX(NOF_EGRESS_PMF_ACTIONS)];
  /*
   *  Indicates the new action values - at FLP - ELK results.
   */
  SOC_PPC_FP_PACKET_DIAG_ACTION_ELK elk_action[SOC_PPC_FP_NOF_ELK_ACTIONS];

  /* Is stage valid: do not print it otherwise. For FLP and ELK */
  uint8 valid_stage[SOC_PPC_NOF_FP_DATABASE_STAGES];

} SOC_PPC_FP_PACKET_DIAG_INFO;

typedef enum
{
  /* Retrieve only the resource usage */
  SOC_PPC_FP_RESOURCE_MODE_USAGE  = 0,

  /* Perform all possible validation and resource usage analysis and available resources */
  SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES    = 1,

  /* Diagnose the consistency between SW and HW */
  SOC_PPC_FP_RESOURCE_MODE_DIAG   = 2,

  /* Perform all possible validation and resource usage analysis */
  SOC_PPC_FP_RESOURCE_MODE_ALL    = 3
  
} SOC_PPC_FP_RESOURCE_DIAG_MODE;

/* Representation of the bit type - LSB or MSB */
typedef enum {
  /* The Least Significant Bits */
  SOC_PPC_FP_KEY_BIT_TYPE_LSB = 0,

  /* The Most Significant Bits */
  SOC_PPC_FP_KEY_BIT_TYPE_MSB,

  /* The number of bit types */
  SOC_PPC_FP_NOF_KEY_BIT_TYPES

} SOC_PPC_FP_KEY_BIT_TYPE_LSB_MSB;

typedef struct
{
  /* LSB (Least significant bit) */ 
  uint32 lsb;

  /* MSB (most significant bit) */
  uint32 msb;

} SOC_PPC_FP_RESOURCE_KEY_LSB_MSB;

typedef struct
{
  /* If TRUE, then the Qualifier is valid. */
  uint8 valid;

  /* Qualifier type */
  SOC_PPC_FP_QUAL_TYPE qual_type;

  /* 
  * If TRUE, then the qualifiers is part of the 
  * second Key (only for 320b key size)
  */
  uint8 is_second_key;

  /* 
  * If TRUE, then the qualifiers is part of the 
  * MSB part of the key (location 80 - 159) 
  * else it is in hte LSB (location 0 - 79) 
  */
  uint8 is_msb;

  /* Location inside the qualifier */
  SOC_PPC_FP_RESOURCE_KEY_LSB_MSB qual_loc;

  /* Location inside the key */
  SOC_PPC_FP_RESOURCE_KEY_LSB_MSB key_loc;

} SOC_PPC_FP_RESOURCE_KEY;

typedef struct
{
  /* If TRUE, then the bank is used. */
  uint8 valid;

  /* Number of entries used by this Database in this bank */
  uint32 entries_used;

  /* Number of entries free in this bank */
  uint32 entries_free;

  /* Bitmap of the used action tables */
  uint32 action_tbl_bmp;

} SOC_PPC_FP_RESOURCE_DB_BANK;

typedef struct
{
  /* If TRUE, then the action is valid. */
  uint8 valid;

  /* Action type */
  SOC_PPC_FP_ACTION_TYPE action_type;

  /* Location inside the TCAM action table */
  SOC_PPC_FP_RESOURCE_KEY_LSB_MSB action_loc;

} SOC_PPC_FP_RESOURCE_ACTION;

typedef struct
{
  /* TCAM Entry prefix value. Not relevant for Direct table DBs */
  uint32 prefix_val;

  /* TCAM Entry prefix length. Not relevant for Direct table DBs */
  uint32 prefix_len;

  uint32 access_profile_id[SOC_PPC_FP_KEY_NOF_KEYS_PER_DB_MAX];

  /* The number of keys used for db*/
  uint32 nof_keys_per_db;

  /* Bank information for this DB */
  SOC_PPC_FP_RESOURCE_DB_BANK bank[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS];

  /* Action structure in the TCAM Action table */
  SOC_PPC_FP_RESOURCE_ACTION action[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];

  /* Number of actions used by this TCAM DB (nof valid elements in action array)*/
  uint32 nof_actions;

} SOC_PPC_FP_RESOURCE_DB_TCAM;

typedef struct
{
  /* Direct extraction entry is valid */
  uint8 valid[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_MACROS];

  /* FEM entry ID per cycle per FEM */
  uint32 fem_entry_id[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_MACROS];

  /* Direct extraction entry information */
  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO de_entry[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_MACROS];

  /* Direct extraction entry ID */
  uint32 de_entry_id[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_MACROS];

} SOC_PPC_FP_RESOURCE_DB_DE;

typedef struct
{
  /* 
  * If TRUE, then the Database is valid. Otherwise, 
  * the Database does not exist.
  */
  uint8 valid;

  /* Database type */
  SOC_PPC_FP_DATABASE_TYPE type;

  /* Stage of the Database: e.g. ingress PMF. */
  SOC_PPC_FP_DATABASE_STAGE stage;

  /* The priority of this Database */
  uint32 db_priority;

  /* Key size in bits. Expected values: 10/32/80/160/320 */
  uint32 key_size;

  /* Key structure after its construction with the Copy Engines */
  SOC_PPC_FP_RESOURCE_KEY key_qual[SOC_PPC_FP_NOF_CES_PER_DB_MAX];

  /* Number of Copy Engines used (nof valid elements in key_qual array) */
  uint32 nof_ces;

  /* TCAM information for TCAM and Direct Table databases */
  SOC_PPC_FP_RESOURCE_DB_TCAM db_tcam;

  /* Direct extraction information for Direct extraction databases */
  SOC_PPC_FP_RESOURCE_DB_DE db_de;

} SOC_PPC_FP_RESOURCE_DB;

typedef struct
{
  /* Database ID if existing. */
  uint32 db_id;

  /* Number of entries used for this database */
  uint32 nof_entries;

  /* Database prefix */
  SOC_TMC_TCAM_DB_PREFIX prefix;

} SOC_PPC_FP_RESOURCE_BANK_DB;

typedef struct
{
 /* If TRUE, then the bank is allocated */
  uint8 is_used;

  /* Bank entry size (80/160/320) */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE entry_size;

  /* Bank block user */
  SOC_TMC_TCAM_BANK_OWNER owner;

  /* Bank usage */
  SOC_PPC_FP_RESOURCE_BANK_DB db[SOC_PPC_FP_MAX_NOF_DBS_PER_BANK];

  /* Indicates the number of databases using this bank */
  uint32 nof_dbs;

  /* Number of entries free in this bank */
  uint32 entries_free;

} SOC_PPC_FP_RESOURCE_BANK;

typedef struct
{
  /* 
   * If TRUE, then this Program selection line is used for 
   * Ethernet packets
   */
  uint8 is_valid;

  /* Bitmap of the used preselectors */
  
  uint32 presel_bmp[SOC_PPC_FP_NOF_PS_LINES_IN_LONGS]; 

  /* PMF-Program ID */
  uint32 pmf_pgm;

  /* Bitmap of Databases used by this PMF-Program */
  uint32 db_bmp[SOC_PPC_FP_NOF_DBS_IN_LONGS];

} SOC_PPC_FP_RESOURCE_PRESEL;

typedef enum
{
  /*
   * Valid-Bank error: 
   * 0  Bank-ID, 1  SW-valid indication
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_VALID = 0, 
    /*
   * Bank Entry size error:
   * 0  Bank-ID, 1  TCAM-DB-Profile-0, 2  Entry-Size-0, 
   * 2  TCAM-DB-Profile-1, 3  Entry-Size-1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_ENTRY_SIZE = 1, 
    /*
   * Valid-Entry error:  
   * 0 - TCAM-DB, 1 - Entry-DB-Id, 2 - HW-DB-Profile (according to prefix),  
   * 3  Bank-ID, 4  Entry-HW-Line, 5  Entry-HW-Valid, 
   * 6  Entry-SW-Valid
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_ENTRY_VALID = 2, 
    /*
   * Number of entries per bank error:
   * 0  Bank-ID, 1  SW-Nof-entries, 2  HW-Nof-Entries
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_NOF_ENTRIES = 3, 
    /*
   * Valid-Databases for this bank error:
   * 0  Bank-ID, 1  DB-Not-Found in one of the SW DB
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_DBS = 4, 
    /*
   * Database prefixes repartition error:
   * 0  Bank-ID, 1  TCAM-DB-0, 2  DB-0, 3  prefix-0,  4  TCAM-DB-1, 5  DB-1, 6  prefix-1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_PREFIX_DB = 5, 
    
  /*
   * Unique HW DB profile error:
   * 0  HW DB Profile, 1  TCAM-DB-0, 2  PMF-DB-0, 3 - TCAM-DB-1, 4 - PMF-DB-1 
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_UNIQUE_PROFILE = 0,
  /*
   * Error in TCAM DB Prefix:
   * * 0  TCAM-DB-ID, 1 - DB-ID, 2  HW-DB-Profile, 3  SW-Value, 4  SW-Length 
   * 5  HW-Value, 6  HW-Length  
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_PREFIX = 1,
  /*
   * Error in TCAM DB entry size:
   * 0  TCAM-DB-ID, 1 - DB-ID, 2  HW-DB-Profile, 3- Bank ID, 4  SW-Value, 5  HW-Value 
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_ENTRY_SIZE = 2,
  /*
   * Error in TCAM DB banks:
   * 0  TCAM-DB-ID, 1  DB-ID, 2 - Bank ID, 3  HW-DB-Profile, 4  SW-Value, 5  HW-Value
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_BANKS = 3,
  /*
   * Error in TCAM DB action bitmap:
   * 0  TCAM-DB-ID, 1  DB-ID, 2  HW-DB-Profile, 3  SW-Value, 4  HW-Value
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_ACTION_BITMAP = 4,
  /*
   * Entry priority error:
   * 0  TCAM-DB-ID, 1  DB-ID, 2/3  Entry-ID-0/1, 4/5  Entry-Bank-0/1, 
   * 6/7  Entry-HW-Line-0/1, 8/9  Entry-Priority-0/1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_PRIORITY = 5,
  /*
   * Valid-Entry error:
   * 0  DB-ID, 1  Entry-ID, 2  SW-Entry-Valid, 
   * 3 - HW-Entry-Valid, 4 - SW-Prefix, 5 - HW-Prefix
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_ENTRY_VALID = 6,

  /*
   * Same Key for all PMF-Programs error:
   * 0  PMF-Pgm0/1, 2  Key-ID0/1, 4  Cycle-ID-0/1, 6  CE-ID-0/1, 
   * 8 - DB-ID, 9 - TCAM DB ID
   */	
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_COHERENCY = 0,
    /* 
     * Not same Key in SW and HW error:
     * 0  PMF-Pgm, 1 - Qual-Type, 2  Key-ID, 3  Cycle-ID, 4  CE-ID,
     * 5  SW qual lsb, 6  HW lsb 
     * 7  SW qual msb, 8  HW length
  */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_SW_HW = 1,
  /*
   * Indication this CE is used error:
   * 0  PMF-Pgm, 1  Key-ID, 2  Cycle-ID, 3  CE-ID, 
   * 4  SW-Is-Used, 5 - HW-Is-Used
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_CE_USED = 2,

  /*
   * Indication that the CE bitmap does not match:
   * 0  Stage, 1  Program, 2  Cycle-ID, 3  CE-bitmap-0, 
   * 4  CE-bitmap-1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_CE_BITMAP = 3, 

  /*
   * Indication that the KEY bitmap does not match:
   * 0  Stage, 1  Program, 2  Key-ID, 3  Key-bitmap-0, 
   * 4  Key-bitmap-1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_KEY_BITMAP = 4,

  /*
   * FEM used error:
   * 0  PMF-Pgm, 1  FEM-ID, 2  DB-ID, 3  TCAM-DB-ID, 4  HW Action Type, 
   * 5 - SW Action type
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_FEM = 0,
  /*
   * FES used error:
   * 0  PMF-Pgm, 1  FES-ID, 2  DB-ID, 3 - TCAM DB ID, 
   * 4  HW Action type, 5  SW Action type 
   * 6 - HW lsb, 7 - SW lsb
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_FES = 1,
  /*
   * FEM Base value used error:
   * 0  PMF-Pgm, 1 FEM-ID, 2  DB-ID, 3  TCAM-DB-ID, 
   * 4  HW Base value, 5  SW Base value
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_FEM_BASE = 2,
  /*
   * Priorirty error between DBs or FEM Entries error:
   * 0  PMF-Pgm, 1  Action-ID, 2  DB-ID-0, 4 - TCAM-DB-ID-0, 
   * 5 - DB-ID 1, 6 - TCAM-DB-ID-1, 7  IS_FES-0, 8  IS_FES-1, 
   * 9  FES/FEM-ID-0, 10 - FES/FEM-ID-1, 11 Priority 0, 12 - Priority 1, 
   * [13/14  Entry-ID0/1]
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_PRIORITY = 3,

  /*
   * Indication that the FES bitmap does not match:
   * 0  Stage, 1  Program, 2  FES-bitmap-0, 
   * 5  FES-bitmap-1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_FES_BITMAP = 4,

  /*
   * Preselection line error:
   * 0  HW line, 1  SW-Valid-ID, 2  HW-Valid-ID
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_PRESEL_LINE_VALID = 0,
  /*
   * Program-valid error:
   * 0  PMF-Pgm ID, 1  SW-Valid-ID, 2  HW-Valid-ID, 3  Preselector/HW line pointing to it
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_PRESEL_PGM_VALID = 1,
  /*
   * Database bitmap inconsistency error:
   * 0  PMF-Program, 1  SW-Database-bitmap-0, 2  SW-Database-bitmap-1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_PRESEL_DB_BMP = 2,

  /*
   * Indication that the PROG bitmap does not match:
   * 0  Stage, 1  Prog-bitmap-0, Prog  FES-bitmap-1
   */
  SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_PRESEL_PROG_BITMAP = 3


} SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE;

typedef enum
{
  /* No error happens */
  /*SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_NONE,*/

  /* Error in TCAM Bank */
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_BANK ,

  /* Error in TCAM Database */
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB, 

  /* Error in Key */
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_KEY, 

  /* Error in Action */
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_ACTION, 

  /* Error in Preselection */
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_PRESEL,

  /* Number of error types */
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_NOF_TYPE_LAST

} SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE;

typedef struct 
{
  /* True if Error Detected */
  uint8 is_error;

  /* The value related to the error param_type */ 
  uint32 value[SOC_PPC_FP_RESOURCE_DIAG_NOF_ERROR_VALUES];

} SOC_PPC_FP_RESOURCE_DIAG_ERROR_PARAM;

typedef struct 
{
  /* Error parameters: their meaning depends on the error type */ 
	SOC_PPC_FP_RESOURCE_DIAG_ERROR_PARAM params[SOC_PPC_FP_RESOURCE_DIAG_NOF_PARAMS];
	
} SOC_PPC_FP_RESOURCE_DIAG;

typedef struct
{
  /*
   *  Used by instruction allocation code
   */
  uint8 is_used;

 /*
  * how many bits of this CE is useful.
  */
  uint8 lsb;
 /*
  * how many bits of this CE is useful.
  */
  uint8 msb;

  /* Indicates the location inside the key - LSB or MSB */
  uint8 is_msb;

  /* In case of 320b, differentiate between the 2 keys */
  uint8 is_second_key;

  uint8 qual_lsb;
  /*
   * qual-type set by this CE
   */
  SOC_PPC_FP_QUAL_TYPE                qual_type;
  /*
   * key id
   */
  uint32 key_id;
} SOC_PPC_FP_RESOURCE_PMF_CE;

typedef struct{
  /*
   *  Used by instruction allocation code
   */
  uint8 is_used;
  /*
   * 16b-lsb
   */
  uint32 lsb_16b;
  /*
   * 32b-lsb
   */
  uint32 lsb_32b;
  /*
   * 16b-msb
   */
  uint32 msb_16b;
  /*
   * 32b-msb
   */
  uint32 msb_32b;
}SOC_PPC_FP_RESOURCE_FREE_INSTRUCTIONS;

typedef struct{
  /*
   *  Used by instruction allocation code
   */
  uint8 is_used;
  /*
   * Is LSB Data Base
   */
  uint8 is_lsb_db;
  /*
   * Is MSB Data Base
   */
  uint8 is_msb_db;
}SOC_PPC_FP_RESOURCE_KEY_DB_DATA;

typedef struct{
  /*
   *  Used by instruction allocation code
   */
  uint8 is_used;
  /*
   * action type
   */
  SOC_PPC_FP_ACTION_TYPE action_type;
  /*
   * action lsb
   */
  uint32 action_lsb;
}SOC_PPC_FP_RESOURCE_PMF_FES;

typedef struct{
  /*
   *  Used by instruction allocation code
   */
  uint8 is_used;
  /*
   * Fes free
   */
  uint32 fes_free;
}SOC_PPC_FP_RESOURCE_PMF_FES_FREE;

typedef struct
{
  /*
   *  Used by instruction allocation code
   */
  uint8 is_used;
  /*
   *  If True, this FEM is for a Direct Extraction entry.
   */
  uint8 is_for_entry;
  /*
   *  Database strength of this FEM
   */
  uint32 db_strength;
  /*
   *  Entry strength of this FEM. Relevant only if
   *  'is_for_entry' is True.
   */
  uint32 entry_strength;
  /*
   *  Entry-ID of this FEM (relevant only if 'is_for_entry' is
   *  True).
   */
  uint32 entry_id;
  /*
   *  Required action type. Needed to know the minimal FEM
   *  size.
   */
  SOC_PPC_FP_ACTION_TYPE action_type;
} SOC_PPC_FP_RESOURCE_PMF_FEM_ENTRY;

typedef struct{
  /*
   * Used by instruction allocation code
   */
  uint8 is_used;
  /*
   * Instruction information
   */
  SOC_PPC_FP_RESOURCE_PMF_CE pgm_ce[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PROGS_MAX][SOC_PPC_FP_NOF_DBS][SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_CES];
  /*
   * Free instructions
   */
  SOC_PPC_FP_RESOURCE_FREE_INSTRUCTIONS free_instructions[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PROGS_MAX][SOC_PPC_FP_NOF_CYCLES];
  /*
   * Key Resources
   */
  SOC_PPC_FP_RESOURCE_KEY_DB_DATA key[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PROGS_MAX][SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_KEYS];
  /*
   * Fes actions information
   */
  SOC_PPC_FP_RESOURCE_PMF_FES fes[SOC_PPC_FP_NOF_PROGS_MAX][SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_DBS][SOC_PPC_FP_NOF_CES];
  /*
   * Fes Free
   */
  SOC_PPC_FP_RESOURCE_PMF_FES_FREE fes_free[SOC_PPC_FP_NOF_PROGS_MAX][SOC_PPC_FP_NOF_CYCLES];
  /*
   * Fem
   */
  SOC_PPC_FP_RESOURCE_PMF_FEM_ENTRY fem[SOC_PPC_FP_NOF_CYCLES][SOC_PPC_FP_NOF_DBS][SOC_PPC_FP_NOF_MACROS];
  /*
   * Preselectors - Pfg bitmap 
   */
  uint32 pfgs[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PS_LINES][SOC_PPC_FP_NOF_PROGS_MAX][SOC_PPC_BIT_TO_U32(SOC_PPC_FP_NOF_PFGS_ARAD)];
  /*
   * Preselectors - Quals bitmap 
   */
  uint32 quals[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PS_LINES][SOC_PPC_FP_NOF_PROGS_MAX][SOC_PPC_BIT_TO_U32(SOC_PPC_NOF_FP_QUAL_TYPES)];
  /*
   * Preselctors - Data base bitmap for each (stage X Pfg)
   */
  uint32 pfgs_db_pmb[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PFGS_ARAD][SOC_PPC_BIT_TO_U32(SOC_PPC_FP_NOF_DBS)];
  /*
   * Preselctors - Qualifiers per (stage X Pfg)
   */
  SOC_PPC_FP_QUAL_VAL pfgs_qualifiers[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PFGS_ARAD][SOC_PPC_FP_NOF_QUALS_PER_PFG_MAX];
}SOC_PPC_FP_RESOURCE_AVAILABLE;

typedef struct
{
  /* Database information */
  SOC_PPC_FP_RESOURCE_DB db[SOC_PPC_FP_NOF_DBS];

  /* Bank usage. */
  SOC_PPC_FP_RESOURCE_BANK bank[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS];

  /* Program selection usage. */
  SOC_PPC_FP_RESOURCE_PRESEL presel[SOC_PPC_NOF_FP_DATABASE_STAGES][SOC_PPC_FP_NOF_PS_LINES];
  
  /* Diagnostic information. */
  SOC_PPC_FP_RESOURCE_DIAG diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_NOF_TYPE_LAST];

  /* Available resources information */
  SOC_PPC_FP_RESOURCE_AVAILABLE available;
} SOC_PPC_FP_RESOURCE_DIAG_INFO;


/* TMC related ported to here for include reasons */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Header Format bitmap (based on the SOC_TMC_FP_PKT_HDR_TYPE
   *  values) of the supported formats for this Packet Format
   *  Group. Only the Ethernet-based header formats are
   *  allowed. Default value: 0x0. (None)
   *  Petra-B only.
   */
  uint32 hdr_format_bmp;
  /*
   *  VLAN Tag structure bitmap based on the
   *  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT values. Default
   *  value: 0xFFFF_FFFF (all VLAN format).
   *  Petra-B only.
   */
  uint32 vlan_tag_structure_bmp;
  /*
   *  Bitmap of the PP-Ports supporting this Packet Format
   *  Group. The PP-Port configuration must be set previously
   *  (in Soc_petra-B, via the soc_pb_port_pp_port_set API).
   *  Petra-B only.
   */
  SOC_SAND_U64 pp_ports_bmp;

  /* 
   * Alternative implementation: for BCM simpler usage, 
   * transform these parameters to an array (similar to 
   * the Database definition) 
   * The mapping between qualifiers and parameters: 
   * - hdr_format_bmp <-> SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE 
   * - vlan_tag_structure_bmp <-> SOC_PPC_FP_QUAL_IRPP_ETH_TAG_FORMAT 
   * - pp_ports_bmp <-> SOC_PPC_FP_QUAL_IRPP_IN_PORT_BITMAP
   */
  /* Indicate if using this mode. Mandatory for Arad, otherwise this PFG should be deleted */
  uint8 is_array_qualifier;

  /* Type, Value and Mask - Mask is ignored for Soc_petra-B */
  SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_PFG_MAX];

  /* Stage of the PFG */
  SOC_PPC_FP_DATABASE_STAGE stage;

    /* flag determining if to commit presel line in HW */
  uint8 is_for_hw_commit;

    /* flag determining if  presel line second-pass */
  uint8 is_staggered;

} SOC_PPC_PMF_PFG_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * source local port
   */
  uint32 local_port_src; 
  /*
   * the size of the packet in bytes (between 64 and 200) 
   */
  uint8 size_bytes;
  /*
   * The buffer in which the data is stored
   */
  uint32 buffer[SOC_PPC_FP_IRE_TRAFFIC_BUFFER_SIZE];
}SOC_PPC_FP_PACKET;



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
  SOC_PPC_PMF_PFG_INFO_clear(
    SOC_SAND_OUT SOC_PPC_PMF_PFG_INFO *info
  );

void
  SOC_PPC_FP_PACKET_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET *info
  );

void
  SOC_PPC_FP_QUAL_VAL_clear(
    SOC_SAND_OUT SOC_PPC_FP_QUAL_VAL *info
  );

void
  SOC_PPC_FP_ETH_TAG_FORMAT_clear(
    SOC_SAND_OUT SOC_PPC_FP_ETH_TAG_FORMAT *info
  );

void
  SOC_PPC_FP_DATABASE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FP_DATABASE_INFO *info
  );

void
  SOC_PPC_FP_ACTION_VAL_clear(
    SOC_SAND_OUT SOC_PPC_FP_ACTION_VAL *info
  );

void
  SOC_PPC_FP_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FP_ENTRY_INFO *info
  );

void
  SOC_PPC_FP_DIR_EXTR_ACTION_LOC_clear(
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ACTION_LOC *info
  );

void
  SOC_PPC_FP_DIR_EXTR_ACTION_VAL_clear(
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ACTION_VAL *info
  );

void
  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *info
  );

void
  SOC_PPC_FP_CONTROL_INDEX_clear(
    SOC_SAND_OUT SOC_PPC_FP_CONTROL_INDEX *info
  );

void
  SOC_PPC_FP_CONTROL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FP_CONTROL_INFO *info
  );

void
  SOC_PPC_FP_DIAG_PP_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FP_DIAG_PP_PORT_INFO *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_PARSER_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_PARSER *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_PGM_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_PGM *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_QUAL_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_QUAL *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_DB_QUAL_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_DB_QUAL *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_KEY *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_TCAM_DT_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_TCAM_DT *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_MACRO_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_MACRO *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_MACRO_SIMPLE_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_MACRO_SIMPLE *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_ACTION_ELK_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_ACTION_ELK *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_INFO *info
  );

void
  SOC_PPC_FP_RESOURCE_KEY_LSB_MSB_clear(
	  SOC_SAND_OUT SOC_PPC_FP_RESOURCE_KEY_LSB_MSB *info
	);
void
  SOC_PPC_FP_RESOURCE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_KEY *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_BANK_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DB_BANK *info
  );
void
  SOC_PPC_FP_RESOURCE_ACTION_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_ACTION *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_TCAM_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DB_TCAM *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_DE_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DB_DE *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DB *info
  );
void
  SOC_PPC_FP_RESOURCE_BANK_DB_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_BANK_DB *info
  );
void
  SOC_PPC_FP_RESOURCE_BANK_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_BANK *info
  );
void
  SOC_PPC_FP_RESOURCE_PRESEL_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_PRESEL *info
  );
void
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_PARAM_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DIAG_ERROR_PARAM *info
  );
void
  SOC_PPC_FP_RESOURCE_DIAG_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DIAG *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_CE_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_PMF_CE *info
  );
void
  SOC_PPC_FP_RESOURCE_FREE_INSTRUCTIONS_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_FREE_INSTRUCTIONS *info
  );
void
  SOC_PPC_FP_RESOURCE_KEY_DB_DATA_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_KEY_DB_DATA *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_FES_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_PMF_FES *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_FES_FREE_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_PMF_FES_FREE *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_FEM_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_PMF_FEM_ENTRY *info
  );
void
  SOC_PPC_FP_RESOURCE_AVAILABLE_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_AVAILABLE *info
  );  
void
  SOC_PPC_FP_RESOURCE_DIAG_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DIAG_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_FP_QUAL_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE enum_val
  );

const char*
  SOC_PPC_FP_FWD_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE enum_val
  );

const char*
  SOC_PPC_FP_PROCESSING_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_PROCESSING_TYPE enum_val
  );

const char*
  SOC_PPC_FP_BASE_HEADER_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_BASE_HEADER_TYPE enum_val
  );

const char*
  SOC_PPC_FP_ACTION_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE enum_val
  );

const char*
  SOC_PPC_FP_DATABASE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_TYPE enum_val
  );

const char*
  SOC_PPC_FP_DATABASE_STAGE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE enum_val
  );

const char*
  SOC_PPC_FP_PREDEFINED_ACL_KEY_to_string(
    SOC_SAND_IN  SOC_PPC_FP_PREDEFINED_ACL_KEY  enum_val
  );

const char*
  SOC_PPC_FP_CONTROL_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_TYPE enum_val
  );

void
  SOC_PPC_FP_PACKET_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET *info
  );

void
  SOC_PPC_PMF_PFG_INFO_print(
    SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO *info
  );

void
  SOC_PPC_FP_QUAL_VAL_print(
    SOC_SAND_IN  SOC_PPC_FP_QUAL_VAL *info
  );

void
  SOC_PPC_FP_DATABASE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO *info
  );

void
  SOC_PPC_FP_ACTION_VAL_print(
    SOC_SAND_IN  SOC_PPC_FP_ACTION_VAL *info
  );

void
  SOC_PPC_FP_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO *info
  );

void
  SOC_PPC_FP_DIR_EXTR_ACTION_LOC_print(
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ACTION_LOC *info
  );

void
  SOC_PPC_FP_DIR_EXTR_ACTION_VAL_print(
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ACTION_VAL *info
  );

void
  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_print(
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *info
  );

void
  SOC_PPC_FP_CONTROL_INDEX_print(
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX *info
  );

void
  SOC_PPC_FP_CONTROL_INFO_print(
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INFO *info
  );

void
  SOC_PPC_FP_DIAG_PP_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_FP_DIAG_PP_PORT_INFO *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_PARSER_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_PARSER *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_PGM_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_PGM *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_QUAL_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_QUAL *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_DB_QUAL_print(
    SOC_SAND_IN  uint32                  ind,
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_DB_QUAL *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_KEY_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_KEY *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_TCAM_DT_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_TCAM_DT *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_MACRO_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_MACRO *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_MACRO_SIMPLE_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_MACRO_SIMPLE *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_ACTION_ELK_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_ACTION_ELK *info
  );

void
  SOC_PPC_FP_PACKET_DIAG_INFO_print(
    SOC_SAND_IN  SOC_PPC_FP_PACKET_DIAG_INFO *info
  );

void
  SOC_PPC_FP_RESOURCE_KEY_LSB_MSB_print(
	SOC_SAND_IN SOC_PPC_FP_RESOURCE_KEY_LSB_MSB *info
  );
void
  SOC_PPC_FP_RESOURCE_KEY_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_KEY *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_BANK_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DB_BANK *info
  );
void
  SOC_PPC_FP_RESOURCE_ACTION_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_ACTION *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_TCAM_print(
    SOC_SAND_IN int                         unit,
    SOC_PPC_FP_DATABASE_TYPE                type,
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DB_TCAM *info
  );
void
  SOC_PPC_FP_RESOURCE_QUAL_VAL_print(
    SOC_SAND_IN SOC_PPC_FP_QUAL_VAL *info
  );
void
  SOC_PPC_FP_RESOURCE_ACTION_VAL_print(
    SOC_SAND_IN SOC_PPC_FP_DIR_EXTR_ACTION_VAL *info
  );
void
  SOC_PPC_FP_RESOURCE_DIR_EXTR_ENTRY_INFO_print(
    SOC_SAND_IN SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_DE_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DB_DE *info
  );
void
  SOC_PPC_FP_RESOURCE_DB_print(
    SOC_SAND_IN int                    unit,
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DB *info
  );
void
  SOC_PPC_FP_DB_PREFIX_print(
    SOC_SAND_IN SOC_TMC_TCAM_DB_PREFIX *info
  );
void
  SOC_PPC_FP_RESOURCE_BANK_DB_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_BANK_DB *info
  );
void
  SOC_PPC_FP_RESOURCE_BANK_ALL_DBS_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_BANK *info
  );
void
  SOC_PPC_FP_RESOURCE_BANK_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_BANK *info
  );
void
  SOC_PPC_FP_RESOURCE_PRESEL_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_PRESEL *info
  );
void
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_PARAM_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE err_type,
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE param_type,
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DIAG_ERROR_PARAM *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_CE_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_PMF_CE *info
  );
void
  SOC_PPC_FP_RESOURCE_FREE_INSTRUCTIONS_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_FREE_INSTRUCTIONS *info
  );
void
  SOC_PPC_FP_RESOURCE_KEY_DB_DATA_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_KEY_DB_DATA *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_FES_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_PMF_FES *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_FES_FREE_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_PMF_FES_FREE *info
  );
void
  SOC_PPC_FP_RESOURCE_PMF_FEM_ENTRY_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_PMF_FEM_ENTRY *info
  );
void
  SOC_PPC_FP_RESOURCE_AVAILABLE_print(
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_AVAILABLE *info
  );  
void
  SOC_PPC_FP_RESOURCE_DIAG_INFO_print(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN SOC_PPC_FP_RESOURCE_DIAG_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FP_INCLUDED__*/
#endif

