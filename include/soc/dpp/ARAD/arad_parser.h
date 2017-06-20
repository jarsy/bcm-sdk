/* $Id: arad_parser.h,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PARSER_INCLUDED__
/* { */
#define __ARAD_PARSER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_framework.h>
#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/TMC/tmc_api_pmf_low_level_db.h>

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

#define ARAD_PARSER_PROG_RAW_ADDR_START                          (0x0000)
#define ARAD_PARSER_PROG_ETH_ADDR_START                          (0x0001)
#define ARAD_PARSER_PROG_TM_ADDR_START                           (0x0002)
#define ARAD_PARSER_PROG_PPH_TM_ADDR_START                       (0x0003)
#define ARAD_PARSER_PROG_FTMH_ADDR_START                         (0x0004)
#define ARAD_PARSER_PROG_RAW_MPLS_ADDR_START                     (0x0005)

/* IS_PROTO flags per PFC */
#define IS_ETH      0x01
#define IS_IPV4     0x02
#define IS_IPV6     0x04
#define IS_MPLS     0x08
#define IS_TRILL    0x10
#define IS_UD       0x20

typedef enum
{
    DPP_PLC_TM              = 0x0,
    DPP_PLC_TM_IS           = 0xD,
    DPP_PLC_TM_MC_FLOW      = 0xD,
    DPP_PLC_TM_OUT_LIF      = 0xC,
    DPP_PLC_RAW             = 0x7,
    DPP_PLC_FTMH            = 0x8,
    DPP_PLC_VXLAN           = 0x1,
    DPP_PLC_BFD_SINGLE_HOP  = 0x6,
    DPP_PLC_IP_UDP_GTP1     = 0xC,
    DPP_PLC_IP_UDP_GTP2     = 0xD,
    DPP_PLC_PP_L4           = 0xE,
    DPP_PLC_PP              = 0xF,
    DPP_PLC_MPLS_5          = 0xC,
    DPP_PLC_GAL_GACH_BFD    = 0xD,
    DPP_PLC_FC              = 0x4,
    DPP_PLC_FC_VFT          = 0x5
} DPP_PLC_E;

typedef enum
{
    DPP_PLC_PROFILE_NA = 0,
    DPP_PLC_PROFILE_TM,
    DPP_PLC_PROFILE_TM_IS,
    DPP_PLC_PROFILE_TM_MC_FLOW,
    DPP_PLC_PROFILE_TM_OUT_LIF,
    DPP_PLC_PROFILE_RAW_FTMH,
    DPP_PLC_PROFILE_PP,
    DPP_PLC_PROFILE_MPLS,
    DPP_PLC_PROFILE_FC
} DPP_PLC_PROFILE_E;

typedef enum
{
  /*
   * No mask is requested, so skip
  */
  DPP_PLC_MATCH_NA = 0,
  /*
   * Entire PFC is unmasked - means that match will be for specific PFC only
   */
  DPP_PLC_MATCH_ONE = 1,
  /*
   * Only PLC for PP traffic
   */
  DPP_PLC_MATCH_PP,
  /*
   * Only PLC for PP traffic
   */
  DPP_PLC_MATCH_FCOE,
  /*
   * Any PLC
   */
  DPP_PLC_MATCH_ANY

} DPP_PLC_MASK_E;


/* ParserLeafContext */
#define ARAD_PARSER_PLC_TM                     (0x0)
#define ARAD_PARSER_PLC_RAW                    (0x7)
#define ARAD_PARSER_PLC_FTMH                   (0x8)
#define ARAD_PARSER_PLC_TM_OUT_LIF             (0xC)
#define ARAD_PARSER_PLC_TM_MC_FLOW             (0xD)
#define ARAD_PARSER_PLC_TM_IS                  (0xD)

#define ARAD_PARSER_PLC_FCOE                   (0x4) /* for both encap and not encap when VFT not exist */
#define ARAD_PARSER_PLC_FCOE_VFT               (0x5) /* for both encap and not encap when VFT exist */

#define ARAD_PARSER_PLC_VXLAN                  (0x1)
#define ARAD_PARSER_PLC_BFD_SINGLE_HOP         (0x6)
#define ARAD_PARSER_PLC_IP_UDP_GTP1            (0xC)
#define ARAD_PARSER_PLC_IP_UDP_GTP2            (0xD)
#define ARAD_PARSER_PLC_PP_L4                  (0xE) /* The _L4 leaf context was created to separate the handling
                                                        of IP packets with and without L4 headers. Until this was
                                                        added, TCP/UDP packets were also parsed the same as IP
                                                        packets with no layer 4, which caused wrong load-balancing. */
#define ARAD_PARSER_PLC_PP                     (0xF)

#define ARAD_PARSER_PLC_MPLS_5                 (0xC)    /* At least 5 MPLS labels, to be used only when PFC is ARAD_PARSER_PFC_MPLS3_ETH */
#define ARAD_PARSER_PLC_GAL_GACH_BFD           (0xD)

#define ARAD_PARSER_PLC_NOF                    (16)
#define ARAD_PARSER_PLC_MAX                    (ARAD_PARSER_PLC_NOF - 1)

#define ARAD_PARSER_PLC_MATCH_ONE              0x0
#define ARAD_PARSER_PLC_MATCH_ANY_FCOE         0x1
/* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
#define ARAD_PARSER_PLC_MATCH_ANY_FLP_PP       0x3
#define ARAD_PARSER_PLC_MATCH_ANY              0xF

/* Macros for FLP program selection */
#define DPP_PARSER_PLC_MATCH_ANY_FLP_PP(mc_cam_tbl)                                         \
                mc_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_PP;                   \
                mc_cam_tbl.parser_leaf_context_mask = ARAD_PARSER_PLC_MATCH_ANY_FLP_PP

#define DPP_PARSER_PLC_MATCH_ANY_FLP_PP_LTD(mc_cam_tbl)                                     \
                mc_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_PP;                   \
                mc_cam_tbl.parser_leaf_context_mask = 0x01

#define DPP_PARSER_PLC_MATCH_BFD_SINGLE_HOP(mc_cam_tbl)                                     \
                mc_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_BFD_SINGLE_HOP;       \
                mc_cam_tbl.parser_leaf_context_mask = ARAD_PARSER_PLC_MATCH_ONE

#define DPP_PARSER_PLC_MATCH_FCOE_VFT(mc_cam_tbl)                                           \
                mc_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_FCOE_VFT;             \
                mc_cam_tbl.parser_leaf_context_mask = ARAD_PARSER_PLC_MATCH_ONE

#define DPP_PARSER_PLC_MATCH_TM(mc_cam_tbl)                                                 \
                mc_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_TM;                   \
                mc_cam_tbl.parser_leaf_context_mask = ARAD_PARSER_PLC_MATCH_ONE

#define DPP_PARSER_PLC_MATCH_ANY_FCOE(mc_cam_tbl)                                           \
                mc_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_FCOE_VFT;             \
                mc_cam_tbl.parser_leaf_context_mask = ARAD_PARSER_PLC_MATCH_ANY_FCOE

#define DPP_PARSER_PLC_MATCH_ANY(mc_cam_tbl)                                                \
                mc_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_PP;                   \
                mc_cam_tbl.parser_leaf_context_mask = ARAD_PARSER_PLC_MATCH_ANY

/* This Ethertype is used as WA for additional TPID in single-tag packets.
 * It is read from Outer Tag field if there is no match to port tag.
 */
#define ARAD_PARSER_ETHER_PROTO_4_ADD_TPID                 (0x3)
#define ARAD_PARSER_ETHER_PROTO_4_ADD_TPID_TYPE            (0x88a8) /* Additional TPID */
 
#define ARAD_PARSER_ETHER_PROTO_5_EoE                      (0x4)
#define ARAD_PARSER_ETHER_PROTO_5_EoE_TPID_TYPE            (0xE0E0)

#define ARAD_PARSER_ETHER_PROTO_3_EoE                      (0x2)
#define ARAD_PARSER_ETHER_PROTO_3_EoE_TPID_TYPE            (0xE0EC)


/* Coupling */
#define ARAD_PARSER_ETHER_PROTO_6_1588                     (0x5) 
#define ARAD_PARSER_ETHER_PROTO_6_1588_ETHER_TYPE          (0x88f7) /* 1588 */ 
#define ARAD_PARSER_ETHER_PROTO_7_MPLS_MC                  (0x6) 
#define ARAD_PARSER_ETHER_PROTO_7_MPLS_MC_ETHER_TYPE       (0x8848) /* MPLS multicast */ 


/* IHP_PARSER_IP_PROTOCOLS table size */
#define ARAD_PARSER_IP_PROTO_NOF_ENTRIES                    7

/* IPv6 Extension headers - Protocol values taken from:
 * http://www.iana.org/assignments/ipv6-parameters/ipv6-parameters.xhtml
 */
#define ARAD_PARSER_IP_PROTO_IPV6_EXT_HOP_BY_HOP            0       /* Hop-by-Hop (0) */
#define ARAD_PARSER_IP_PROTO_IPV6_EXT_DEST_OPTIONS          60      /* Destination-Options (60) */
#define ARAD_PARSER_IP_PROTO_IPV6_EXT_ROUTING               43      /* Routing (43) */
#define ARAD_PARSER_IP_PROTO_IPV6_EXT_FRAGMENT              44      /* Fragment (44) */
#define ARAD_PARSER_IP_PROTO_IPV6_EXT_MOBILITY              135     /* Mobility (135) */
#define ARAD_PARSER_IP_PROTO_IPV6_EXT_HIP                   139     /* Host-Identity-Protocol (HIP) (139) */
#define ARAD_PARSER_IP_PROTO_IPV6_EXT_SHIM6                 140     /* Shim6 (140) */

typedef enum
{
  /*
   *  Ethernet
   */
  DPP_PFC_ETH            = 0x00,
  /*
   *  MAC-in-MAC
   */
  DPP_PFC_ETH_ETH        = 0x01,
  /*
   *  IPv4 over Ethernet
   */
  DPP_PFC_IPV4_ETH       = 0x02,
  /*
   *  IPv6 over Ethernet
   */
  DPP_PFC_IPV6_ETH       = 0x03,
  /*
   *  MPLS over Ethernet
   */
  DPP_PFC_MPLS1_ETH      = 0x05,
  /*
   *  MPLS x 2 over Ethernet
   */
  DPP_PFC_MPLS2_ETH      = 0x06,
  /*
   *  MPLS x 3 over Ethernet
   */
  DPP_PFC_MPLS3_ETH      = 0x07,
  /*
   *  FC standard over Ethernet
   */
  DPP_PFC_FC_STD_ETH     = 0x08,
  /*
   *  FC with Encap over Ethernet
   */
  DPP_PFC_FC_ENCAP_ETH   = 0x09,
  /*
   *  Ethernet over IP over Ethernet
   */
  DPP_PFC_ETH_IPV4_ETH   = 0x0A,
  /*
   *  Ethernet over TRILL over Ethernet
   */
  DPP_PFC_ETH_TRILL_ETH  = 0x0C,
  /*
   *  Ethernet over MPLS over Ethernet
   */
  DPP_PFC_ETH_MPLS1_ETH  = 0x0D,
  /*
   *  Ethernet over MPLSx2 over Ethernet
   */
  DPP_PFC_ETH_MPLS2_ETH  = 0x0E,
  /*
   *  Ethernet over MPLSx3 over Ethernet
   */
  DPP_PFC_ETH_MPLS3_ETH  = 0x0F,
  /*
   *  IPv4 over IPv4 over Ethernet
   */
  DPP_PFC_IPV4_IPV4_ETH  = 0x12,
  /*
   *  IPv4 over IPv6 over Ethernet
   */
  DPP_PFC_IPV4_IPV6_ETH  = 0x13,
  /*
   *  IPv4 over MPLS over Ethernet
   */
  DPP_PFC_IPV4_MPLS1_ETH = 0x15,
  /*
   *  IPv4 over MPLSx2 over Ethernet
   */
  DPP_PFC_IPV4_MPLS2_ETH = 0x16,
  /*
   *  IPv4 over MPLSx3 over Ethernet
   */
  DPP_PFC_IPV4_MPLS3_ETH = 0x17,
  /*
   *  IPv6 over IPv4 over Ethernet
   */
  DPP_PFC_IPV6_IPV4_ETH  = 0x1A,
  /*
   *  IPv6 over IPv6 over Ethernet
   */
  DPP_PFC_IPV6_IPV6_ETH  = 0x1B,
  /*
   *  IPv6 over MPLS over Ethernet
   */
  DPP_PFC_IPV6_MPLS1_ETH = 0x1D,
  /*
   *  IPv6 over MPLSx2 over Ethernet
   */
  DPP_PFC_IPV6_MPLS2_ETH = 0x1E,
  /*
   *  IPv6 over MPLSx3 over Ethernet
   */
  DPP_PFC_IPV6_MPLS3_ETH = 0x1F,
  /*
   * Packet format code - non Ethernet
   */
  DPP_PFC_RAW_AND_FTMH   = 0x20,
  /*
   * TM without extensions, identical to Raw - The PMF program selection is per PP-port profile
   */
  DPP_PFC_TM             = 0x30,
  /*
   * TM with IS
   */
  DPP_PFC_TM_IS          = 0x31,
  /*
   * TM with MC-FLOW
   */
  DPP_PFC_TM_MC_FLOW     = 0x3A,
  /*
   * TM with OUTLIF
   */
  DPP_PFC_TM_OUT_LIF     = 0x3C
} DPP_PFC_E;

typedef enum
{
    /*
     * No mask is requested, so skip
    */
  DPP_PFC_MATCH_NA = 0,
  /*
   * Entire PFC is unmasked - means that match will be for specific PFC only
   */
  DPP_PFC_MATCH_ONE = 1,
  /*
   * Any PFC with MPLS 1/2/3 and any encapsulated traffic
   */
  DPP_PFC_MATCH_ANY_MPLS,
  /*
   * Any encapsulated traffic, only tunnel matters
   */
  DPP_PFC_MATCH_ANY_ENCAP,
  /*
   * Match only type - either ETH or TM
   */
  DPP_PFC_MATCH_TYPE

} DPP_PFC_MASK_E;

typedef struct
{
    DPP_PLC_E   plc_sw;
    int         plc_hw;
    int         lb_set;
    char*       name;
}  dpp_parser_plc_info_t;

typedef struct
{
    DPP_PLC_PROFILE_E       id;
    char*                   name;
    dpp_parser_plc_info_t*  plc_info;
}  dpp_parser_plc_profile_t;

typedef struct
{
    DPP_PFC_E   sw;
    char*       name;
    int         hw;
    int         is_proto;
    int         l4_location;
    int         secondary;
    uint32      hdr_type_map;
    uint32      vtt;
    uint32      pmf;
    DPP_PLC_PROFILE_E plc_profile_id;
}  dpp_parser_pfc_info_t;

extern dpp_parser_pfc_info_t dpp_parser_pfc_info[];
extern dpp_parser_plc_profile_t dpp_parser_plc_profiles[];

#define ARAD_PARSER_PFC_NOF_BITS            6

#define ARAD_PARSER_PFC_ETH                 0x00
#define ARAD_PARSER_PFC_ETH_ETH             0x01
#define ARAD_PARSER_PFC_IPV4_ETH            0x02
#define ARAD_PARSER_PFC_IPV6_ETH            0x03
#define ARAD_PARSER_PFC_MPLS1_ETH           0x05
#define ARAD_PARSER_PFC_MPLS2_ETH           0x06
#define ARAD_PARSER_PFC_MPLS3_ETH           0x07
#define ARAD_PARSER_PFC_FC_STD_ETH          0x08
#define ARAD_PARSER_PFC_FC_ENCAP_ETH        0x09
#define ARAD_PARSER_PFC_ETH_IPV4_ETH        0x0A
#define ARAD_PARSER_PFC_ETH_TRILL_ETH       0x0C
#define ARAD_PARSER_PFC_ETH_MPLS1_ETH       0x0D
#define ARAD_PARSER_PFC_ETH_MPLS2_ETH       0x0E
#define ARAD_PARSER_PFC_ETH_MPLS3_ETH       0x0F
#define ARAD_PARSER_PFC_IPV4_IPV4_ETH       0x12
#define ARAD_PARSER_PFC_IPV4_IPV6_ETH       0x13
#define ARAD_PARSER_PFC_IPV4_MPLS1_ETH      0x15
#define ARAD_PARSER_PFC_IPV4_MPLS2_ETH      0x16
#define ARAD_PARSER_PFC_IPV4_MPLS3_ETH      0x17
#define ARAD_PARSER_PFC_IPV6_IPV4_ETH       0x1A
#define ARAD_PARSER_PFC_IPV6_IPV6_ETH       0x1B
#define ARAD_PARSER_PFC_IPV6_MPLS1_ETH      0x1D
#define ARAD_PARSER_PFC_IPV6_MPLS2_ETH      0x1E
#define ARAD_PARSER_PFC_IPV6_MPLS3_ETH      0x1F
#define ARAD_PARSER_PFC_RAW_AND_FTMH        0x20 /* LSB must be 0 to avoid conflict with TM */
#define ARAD_PARSER_PFC_TM                  0x30 /* TM without extensions, identical to Raw - The PMF program selection is per PP-port profile */
#define ARAD_PARSER_PFC_TM_IS               0x31 /* TM with IS */
#define ARAD_PARSER_PFC_TM_MC_FLOW          0x3A /* TM with MC-FLOW */
#define ARAD_PARSER_PFC_TM_OUT_LIF          0x3C /* TM with OUTLIF */

/* MASKS for program selections choices 0 - bit is meaningful, 1 don't care */
#define ARAD_PARSER_PFC_PS_MATCH_ONE        0x00
#define ARAD_PARSER_PFC_PS_MATCH_ANY_FCOE   0x01
#define ARAD_PARSER_PFC_PS_MATCH_ANY_ENCAP  0x18
#define ARAD_PARSER_PFC_PS_MATCH_TYPE       0x1F
#define ARAD_PARSER_PFC_PS_MATCH_ANY        0x3F

/* MASKS for acl choices, opposite to program selection ones (VTT/FLP/PMF) - 1 means bit is meaningful, 0 - don't care */
#define ARAD_PARSER_PFC_ACL_MATCH_ANY       0x00
#define ARAD_PARSER_PFC_ACL_MATCH_ANY_MPLS  0x04
#define ARAD_PARSER_PFC_ACL_MATCH_ANY_ENCAP 0x07 /* Care only about channel, don't care o what is inside like 0x18 for VTT/FLP */
#define ARAD_PARSER_PFC_ACL_MATCH_ONE       0x3F /* Match specific PFC */

/*
 * This section is oriented for FLP, due to the fact that it uses hw pfc and not mapped ones
 */
#define DPP_PARSER_PFC_HW_MATCH_ANY(mc_cam_tbl)                                             \
        mc_cam_tbl.packet_format_code_mask = ARAD_PARSER_PFC_PS_MATCH_ANY

#define DPP_PARSER_PFC_HW_MATCH_ANY_TM(mc_cam_tbl)                                          \
        mc_cam_tbl.packet_format_code      = ARAD_PARSER_PFC_RAW_AND_FTMH;                  \
        mc_cam_tbl.packet_format_code_mask = ARAD_PARSER_PFC_PS_MATCH_TYPE

#define DPP_PARSER_PFC_HW_MATCH_ANY_FCOE(mc_cam_tbl)                                        \
        mc_cam_tbl.packet_format_code      = ARAD_PARSER_PFC_FC_STD_ETH;                    \
        mc_cam_tbl.packet_format_code_mask = ARAD_PARSER_PFC_PS_MATCH_ANY_FCOE

#define DPP_PARSER_PFC_HW_MATCH_FCOE_STD(mc_cam_tbl)                                        \
        mc_cam_tbl.packet_format_code      = ARAD_PARSER_PFC_FC_STD_ETH;                    \
        mc_cam_tbl.packet_format_code_mask = ARAD_PARSER_PFC_PS_MATCH_ONE

#define DPP_PARSER_PFC_HW_MATCH_ANY_IPV4(mc_cam_tbl)                                        \
        mc_cam_tbl.packet_format_code      = ARAD_PARSER_PFC_IPV4_ETH;                      \
        mc_cam_tbl.packet_format_code_mask = ARAD_PARSER_PFC_PS_MATCH_ANY_ENCAP

#define DPP_PARSER_PFC_HW_MATCH_ANY_IPV6(mc_cam_tbl)                                        \
        mc_cam_tbl.packet_format_code      = ARAD_PARSER_PFC_IPV6_ETH;                      \
        mc_cam_tbl.packet_format_code_mask = ARAD_PARSER_PFC_PS_MATCH_ANY_ENCAP

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
   /*
    * IPv4 over UDP tunnel
    */
   ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_IPV4 = 0,
    /*
    * IPv6 over UDP tunnel
    */
   ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_IPV6 = 1,
   /*
    * MPLS over UDP tunnel
    */
   ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_MPLS = 2
}ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE;

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
/* Return pointer to string describing MACRO */
char *arad_parser_get_macro_str(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 macro_sel
  );

/*
 * Return hw pfc generated by parser
 */
int
dpp_parser_pfc_get_hw_by_sw(
        DPP_PFC_E pfc_sw,
        DPP_PFC_MASK_E pfc_sw_mask,
        uint32 *pfc_hw_p,
        uint32 *pfc_hw_mask_p);

/*
 * Return vtt pfc generated by parser
 */
int
dpp_parser_pfc_get_vtt_by_sw(
        DPP_PFC_E pfc_sw,
        DPP_PFC_MASK_E pfc_sw_mask,
        uint32 *pfc_vtt_p,
        uint32 *pfc_vtt_mask_p);

/*
 * Given sw pfc and mask return pfc and mask mapped for ACL
 */
int
dpp_parser_pfc_get_acl_by_sw(
        DPP_PFC_E pfc_sw,
        DPP_PFC_MASK_E pfc_sw_mask,
        uint32 *pfc_pmf_p,
        uint32 *pfc_pmf_mask_p);

/*
 * Given acl pfc and mask return sw pfc and mask
 */
int
dpp_parser_pfc_get_sw_by_acl(
        uint32 pfc_pmf,
        uint32 pfc_pmf_mask,
        DPP_PFC_E *pfc_sw_p,
        DPP_PFC_MASK_E *pfc_sw_mask_p);

/*
 * Given sw pfc and mask return pfc and mask mapped for pmf program selection
 */
int
dpp_parser_pfc_get_pmf_by_sw(
        DPP_PFC_E pfc_sw,
        DPP_PFC_MASK_E pfc_sw_mask,
        uint32 *pfc_pmf_p,
        uint32 *pfc_pmf_mask_p);

/*
 * Given pmf pfc (program selection) and mask return sw pfc and mask
 */
int
dpp_parser_pfc_get_sw_by_pmf(
        uint32 pfc_pmf,
        uint32 pfc_pmf_mask,
        DPP_PFC_E *pfc_sw_p,
        DPP_PFC_MASK_E *pfc_sw_mask_p);

/*
 * Given hw pfc return pointer to pfc name
 */
char *
dpp_parser_pfc_string_by_hw(
        int pfc_hw);

/*
 * Given sw pfc return pointer to pfc name
 */
char *
dpp_parser_pfc_string_by_pfc_sw(
        DPP_PFC_E pfc_sw);

int dpp_parser_plc_hw_by_sw(
        DPP_PLC_PROFILE_E plc_profile_id,
        DPP_PLC_E plc_sw,
        DPP_PLC_MASK_E plc_sw_mask,
        uint32 *plc_hw_p,
        uint32 *plc_hw_mask_p);

/*
 * Initialize L4 header location per pfc
 */
uint32
dpp_parser_pfc_l4_location_init(
        int unit);

/*
 * Initialize VTT, PMF ACL and PS mapping per pfc
 */
uint32
dpp_parser_pfc_map_init(
        int unit);

SOC_PPC_PKT_HDR_TYPE
  dpp_parser_pfc_hdr_type_at_index(
    int    pfc_hw,
    uint32 hdr_index);

char *
  dpp_parser_plc_string_by_hw(
            int pfc_hw,
            int plc_hw);

uint32
  arad_parser_init(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_parser_ingress_shape_state_set(
     SOC_SAND_IN int                                 unit,
     SOC_SAND_IN uint8                                 enable,
     SOC_SAND_IN uint32                                  q_low,
     SOC_SAND_IN uint32                                  q_high
  );

int
  arad_parser_nof_bytes_to_remove_set(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN int         core,
    SOC_SAND_IN uint32      tm_port,
    SOC_SAND_IN uint32      nof_bytes_to_skip
  );

/* use to update cfg attributes of the vxlan program, e.g. upd-dest port */
uint32
arad_parser_vxlan_program_info_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN uint16 udp_dest_port
 );

uint32
arad_parser_vxlan_program_info_get(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT uint16 *udp_dest_port
 );

int
  arad_parser_nof_bytes_to_remove_get(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN int         core,
    SOC_SAND_IN uint32      tm_port,
    SOC_SAND_OUT uint32     *nof_bytes_to_skip
  );

uint32
  arad_parser_pp_port_nof_bytes_to_remove_set(
    SOC_SAND_IN int      unit,
    SOC_SAND_IN int      core,
    SOC_SAND_IN uint32      pp_port_ndx,
    SOC_SAND_IN uint32      nof_bytes_to_skip
  );

uint32
arad_parser_udp_tunnel_dst_port_set(
    SOC_SAND_IN int                                      unit,
    SOC_SAND_IN ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE   udp_dst_port_type,
    SOC_SAND_IN int                                      udp_dst_port_val
  );

uint32
arad_parser_udp_tunnel_dst_port_get(
    SOC_SAND_IN  int                                        unit,
    SOC_SAND_IN  ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE     udp_dst_port_type,
    SOC_SAND_OUT int                                        *udp_dst_port_val
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PARSER_INCLUDED__*/
#endif


