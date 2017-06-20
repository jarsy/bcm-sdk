/* $Id: arad_pp_isem_access.h,v 1.57 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $Copyright: (c) 2013 Broadcom Corporation All Rights Reserved.$
 * $
*/

#ifndef __ARAD_PP_ISEM_ACCESS_INCLUDED__
/* { */
#define __ARAD_PP_ISEM_ACCESS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ip_tcam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>

#include <soc/dpp/ARAD/arad_tcam_mgmt.h>



/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Field overlaps other fields. MUST be set first */
#define ARAD_PP_ISEM_ACCESS_LSB_PREFIX                              (SOC_IS_JERICHO(unit)? 44: 37)
#define ARAD_PP_ISEM_ACCESS_MSB_PREFIX                              (SOC_IS_JERICHO(unit)? 49: 40)
#define ARAD_PP_ISEM_ACCESS_NOF_BITS_PREFIX                         (ARAD_PP_ISEM_ACCESS_MSB_PREFIX - ARAD_PP_ISEM_ACCESS_LSB_PREFIX + 1)

#define ARAD_PP_ISEM_ACCESS_KEY_SIZE                                  (2)

#define ARAD_PP_ISEM_ACCESS_NOF_TABLES                                (3)
#define ARAD_PP_ISEM_ACCESS_ID_ISEM_A                                 (0)
#define ARAD_PP_ISEM_ACCESS_ID_ISEM_B                                 (1)
#define ARAD_PP_ISEM_ACCESS_ID_TCAM                                   (2)

#define DPP_PP_ISEM_PROG_SEL_1ST_CAM_DEF_OFFSET(unit)                (SOC_DPP_DEFS_GET(unit, nof_vtt_program_selection_lines) - 6)
#define DPP_PP_ISEM_PROG_SEL_1ST_CAM_USR_LAST_ENTRY(unit)            (DPP_PP_ISEM_PROG_SEL_1ST_CAM_DEF_OFFSET(unit) -1)

/* ARAD_PP_IHP_VTT_VT_PROCESSING_PROFILE */
#define ARAD_PP_IHP_VTT_VT_PROCESSING_PROFILE_NONE                             (0)
#define ARAD_PP_IHP_VTT_VT_PROCESSING_PROFILE_COUPLING                         (1)
#define ARAD_PP_IHP_VTT_VT_PROCESSING_PROFILE_VRRP                             (2)

/* ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE */
#define ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_NONE                             (0)
#define ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_MAC_IN_MAC                       (1)
#define ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_MAC_IN_MAC_MC_FALLBACK           (2)
#define ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_FC                               (3)
#define ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_PWEoGRE                          (4)


/* 
 * VTT TCAM defines { 
 */
#define ARAD_PP_ISEM_ACCESS_TCAM_DEF_PREFIX_SIZE  (4)
#define ARAD_PP_ISEM_ACCESS_TCAM_DEF_MIN_BANKS    (0)


/* ARAD PON flags */
#define ARAD_PP_ISEM_ACCESS_L2_PON_TLS                                         (0x1)

/* ARAD Trill flags */
#define ARAD_PP_ISEM_ACCESS_TRILL_VSI_VL                                       (0x1)
#define ARAD_PP_ISEM_ACCESS_TRILL_VSI_FGL                                      (0x2)
#define ARAD_PP_ISEM_ACCESS_TRILL_VSI_TTS                                      (0x4)


#define ARAD_PP_ISEM_ACCESS_NAMESPACE_TO_PREFIX_WO_ELI(mpls_namespace) \
  ((mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1) ? ARAD_PP_ISEM_ACCESS_MPLS_L1_PREFIX: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L2) ? ARAD_PP_ISEM_ACCESS_MPLS_L2_PREFIX: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L3) ? ARAD_PP_ISEM_ACCESS_MPLS_L3_PREFIX: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1_L3) ? ARAD_PP_ISEM_ACCESS_MPLS_L1L3_PREFIX:ARAD_PP_ISEM_ACCESS_MPLS_L1L2_PREFIX)

#define ARAD_PP_ISEM_ACCESS_NAMESPACE_TO_PREFIX_WO_ELI_4_bit(mpls_namespace) \
  ((mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1) ? ARAD_PP_ISEM_ACCESS_MPLS_L1_PREFIX_4bit: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L2) ? ARAD_PP_ISEM_ACCESS_MPLS_L2_PREFIX_4bit: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L3) ? ARAD_PP_ISEM_ACCESS_MPLS_L3_PREFIX_4bit: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1_L3) ? ARAD_PP_ISEM_ACCESS_MPLS_L1L3_PREFIX_4bit:ARAD_PP_ISEM_ACCESS_MPLS_L1L2_PREFIX_4bit)

#define ARAD_PP_ISEM_ACCESS_NAMESPACE_TO_PREFIX_ELI(mpls_namespace) \
  ((mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1) ? ARAD_PP_ISEM_ACCESS_MPLS_L1ELI_PREFIX: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L2) ? ARAD_PP_ISEM_ACCESS_MPLS_L2ELI_PREFIX: \
  (mpls_namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1_L3) ? ARAD_PP_ISEM_ACCESS_MPLS_L1L3ELI_PREFIX:ARAD_PP_ISEM_ACCESS_MPLS_L1L2ELI_PREFIX)

#define ARAD_PP_ISEM_ACCESS_NAMESPACE_TO_PREFIX(mpls_namespace,is_eli) \
  ((is_eli) ? ARAD_PP_ISEM_ACCESS_NAMESPACE_TO_PREFIX_ELI(mpls_namespace):ARAD_PP_ISEM_ACCESS_NAMESPACE_TO_PREFIX_WO_ELI(mpls_namespace))

#define ARAD_PP_VTT_FIND_NAMESPACE_L1(namespace) \
  (namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1 || namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1_L3 \
   || namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1_L2)

#define ARAD_PP_VTT_FIND_NAMESPACE_L2(namespace) \
  (namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L2 || namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1_L2)

#define ARAD_PP_VTT_FIND_NAMESPACE_L3(namespace) \
  (namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L3 || namespace == SOC_PPC_MPLS_TERM_NAMESPACE_L1_L3)

/* Manage programs */
typedef enum
{
    /* TM: Used for both 1st and 2nd . No lookups */
    PROG_VT_TM_PROG,
    /* Unindex MPLS: ISA: VD x Initial VLAN , ISB: MPLS L1 */
    PROG_VT_VDxINITIALVID_L1,
    /* Unindex MPLS: ISA: VD x Outer Tag , ISB: MPLS 1 */
    PROG_VT_VDxOUTERVID_L1,
    /* Bridge: ISB: VD x Outer x Inner (Double Tagging) */
    PROG_VT_VD_OUTER_INNER,
    /* Q-in-Q: ISA: High Priority(VD x OuterTag x InnerTag), ISB: Low Priority(VD x Outer Tag) */
    PROG_VT_HIGH_VD_OUTER_INNER_OR_LOW_VD_OUTER,
    /* Unindex MPLS: ISA: VD x initial VLAN , ISB: MPLS L1-ELI */
    PROG_VT_VDxINITIALVID_L1_L2ELI,
    /* Unindex MPLS: ISA: VD x outer VLAN , ISB: MPLS L1-ELI */
    PROG_VT_VDxOUTERVID_L1_L2ELI,
    /* Trill */
    PROG_VT_VD_INITIALVID_TRILL,
    PROG_VT_VD_OUTER_TRILL,
    PROG_VT_VD_OUTER_INNER_TRILL,
    /* Unindex MPLS FRR: ISA: MPLS-FRR(L1+L2), ISB: MPLS L1, TCAM: VD x initial VLAN */
    PROG_VT_VD_INITIALVID_L1FRR,
    /* Unindex MPLS FRR: ISA: MPLS-FRR(L1+L2), ISB: MPLS L1, TCAM: VD x outer VLAN */
    PROG_VT_VD_OUTERVID_L1FRR,
    /* Q-in-Q: ISA: VD x Outer Tag x Priority , ISB: VD x Outer Tag */
    PROG_VT_VD_OUTER_OR_VD_OUTER_PCP,
    /* Q-in-Q: ISA: VDxOuterxInner , TCAM: VDxOuterxInnerxOuter-PCP */
    PROG_VT_VD_OUTER_INNER_OR_VD_OUTER_INNER_OUTERPCP,
    /* Q-in-Q: ISA: VDxOuterxInner, ISB: VDxOuter, TCAM: High priority VDxOuterxInnerxOuter-PCP */
    PROG_VT_HIGH_VD_OUTER_INNER_OUTERPCP_OR_VD_OUTER_INNER_OR_VD_OUTER,
    /* Q-in-Q full DB ISA: VDxInitial-VID , ISB: VDxInitial-VID */
    PROG_VT_VD_INITIALVID_OR_VD_INITIALVID,
    /* Q-in-Q full DB ISA: VDxOuter, ISB: VDxOuter */
    PROG_VT_VD_OUTER_OR_VD_OUTER,
    /* Q-in-Q full DB ISA: VDxOuterxInner , ISB: VDxOuterxInner */
    PROG_VT_VD_OUTER_INNER_OR_VD_OUTER_INNER,
    /* Indexed MPLS: ISA: VD x Initial-VID, ISB: L1 MPLS DB */
    PROG_VT_INDX_VD_INITIALVID_L1,
    /* Indexed MPLS: ISA: VD x Outer-VID, ISB: L1 MPLS DB */
    PROG_VT_INDX_VD_OUTERVID_L1,
    /* Indexed MPLS: ISA: VD x Outer-VID, ISB: 1 lookups with a fixed key(0x3) which will alwyas result with a hit in case of port termination */
    PROG_VT_INDX_MPLS_PORT_L1,
    /* Indexed MPLS FRR: ISA: FRR(L1+L2), ISB: L3, TCAM: VD x Initial-VID */
    PROG_VT_INDX_VD_INITIALVID_FRR_L1,
    /* Indexed MPLS FRR: ISA: FRR(L1+L2), ISB: L3, TCAM: VD x Outer-VID */
    PROG_VT_INDX_VD_OUTERVID_FRR_L1,
    /* Indexed MPLS and indexed-in-rif (mpls_context=interface/port_and_interface) P1: ISA: VD x Initial-VID, ISB: L3 MPLS DB */
    PROG_VT_INDX_VD_INITIALVID_L3,
    /* Indexed MPLS ISA: VD x Outer-VID, ISB: L3 MPLS DB */
    PROG_VT_INDX_VD_OUTERVID_L3,
    /* PON ISB: VD(3bits) x Tunnel-ID */
    PROG_VT_PON_UNTAGGED,
    /* PON ISA : VD(3bits) x Tunnel-ID x Outer-VID , ISB: VD(3bits) x Tunnel-ID */
    PROG_VT_PON_ONE_TAG,
    /* PON ISA : VD(3bits) x Tunnel-ID x Outer-VID x Inner-VID , ISB: VD(3bits) x Tunnel-ID x Outer-VID */
    PROG_VT_PON_TWO_TAGS,
    /* ISA: VD(3bits) x Tunnel-ID x Outer-VID x Inner-VID , ISB: VD (3bits) x Tunnel-ID */
    PROG_VT_PON_TWO_TAGS_VS_TUNNEL_ID,
    /* IP : for DIP IP lookup at VT for (NVGRE | VXLAN | IP-Tunnel-termination) enabled */
    PROG_VT_IPV4_INITIAL_VID,
    /* IP : for DIP IP lookup at VT for (NVGRE | VXLAN | IP-Tunnel-termination) enabled ISA: VD x Outer-VID ISB: SIP */
    PROG_VT_IPV4_OUTER_VID,
    /* IP-LIF-DUMMY: for DIP IP lookup at VT for (NVGRE | VXLAN | IP-Tunnel-termination) enabled ISA: VD x Outer-VID ISB: DIP */
    PROG_VT_IPV4_PORT,
    /* EVB: ISB : VD x S-Channel x Initial-C-VID */
    PROG_VT_EVB_S_TAG,
    /* ISB : VD x Default-S-Channel x Initial-C-VID */
    PROG_VT_EVB_UN_C_TAG,
    /* Q-in-Q: ISA: High priority (VD x Initial-VID x Inner-VLAN) Low priority (VD x Initial-VID) */
    PROG_VT_DOUBLE_TAG_PRIORITY_INITIAL_VID,
    /* Trill designated-VLAN */
    PROG_VT_VD_DESIGNATED_VID_TRILL,
    /* Q-in-Q 5 tuple: ISA: High priority (VD x Initial-VID x Inner-VLAN) Low priority (VD x Initial-VID) TCAM: 5-tuple */
    PROG_VT_DOUBLE_TAG_5_IP_TUPLE_Q_IN_Q,
    /* Single tag 5 tuple: ISB (VD x Initial-VID) TCAM: 5-tuple */
    PROG_VT_SINGLE_TAG_5_IP_TUPLE_Q_IN_Q,
    /* IP-LIF-DUMMY : for DIP IP lookup at VT for (NVGRE | VXLAN | IP-Tunnel-termination) enabled */
    PROG_VT_IPV4_INITIAL_VID_AFTER_RECYCLE,
    /* IP-LIF-DUMMY: for DIP IP lookup at VT for (NVGRE | VXLAN | IP-Tunnel-termination) enabled ISA: VD x Outer-VID ISB: DIP */
    PROG_VT_IPV4_OUTER_VID_AFTER_RECYCLE,
	/* VT stage AC only SEM-1: lookup VD x Initial-VID, TCAM: Initial-VID x VD x Ethertype */
    PROG_VT_VD_INITIALVID,
	/* VT stage AC only SEM-1: lookup VD x Compressed-Outer, SEM-2: VD x Initial-VID, 
	   TCAM: Initial-VIDxVDxCompressed-OuterxPCP_DEIxEthertype */
    PROG_VT_VD_SINGLE_TAG,
	/* VT stage AC only SEM-1: lookup VD x Compresed-Outer x Compressed-Inner
	   SEM-2: VD x Compressed-Outer. 
	   TCAM: Initial-VIDxInner-VIDxVDxCompressed-OuterxComprssed-InnerxPCP-DEI, Ethertype */
    PROG_VT_VD_DOUBLE_TAG,
    /* VT stage AC only SEM-1: lookup VD x Initial-VID, TCAM: Initial-VID x VD x Ethertype */
    PROG_VT_VD_INITIALVID_VRRP,
    /* VT stage AC only SEM-1: lookup VD x Compressed-Outer, SEM-2: VD x Initial-VID,
       TCAM: Initial-VIDxVDxCompressed-OuterxPCP_DEIxEthertype */
    PROG_VT_VD_SINGLE_TAG_VRRP,
    /* VT stage AC only SEM-1: lookup VD x Compresed-Outer x Compressed-Inner
       SEM-2: VD x Compressed-Outer.
      TCAM: Initial-VIDxInner-VIDxVDxCompressed-OuterxComprssed-InnerxPCP-DEI, Ethertype */
    PROG_VT_VD_DOUBLE_TAG_VRRP,
	/* Same as VD_InitialVID but different TCAM: lookup Explicit NULL reserved label */
    PROG_VT_VD_INITIALVID_EXPLICIT_NULL,
	/* Same as VD Single TAG but different TCAM: lookup Explicit NULL reserved label */ 
    PROG_VT_VD_SINGLE_TAG_EXPLICIT_NULL,
	/* Same as VD Double TAG but different TCAM: lookup Explicit NULL reserved label */
    PROG_VT_VD_DOUBLE_TAG_EXPLICIT_NULL,

    PROG_VT_OUTER_INNER_PCP_1_TST2,
    PROG_VT_OUTER_INNER_PCP_2_TST2,
    PROG_VT_OUTER_PCP_1_TST2,
    PROG_VT_OUTER_PCP_2_TST2,
    PROG_VT_OUTER_INNER_1_TST2,
    PROG_VT_OUTER_INNER_2_TST2,
    PROG_VT_OUTER_1_TST2,
    PROG_VT_OUTER_2_TST2,
    PROG_VT_UNTAGGED_TST2,
    PROG_VT_TEST2,
    PROG_VT_EXTENDER_PE,
    PROG_VT_EXTENDER_PE_UT,
	PROG_VT_CUSTOM_PP_PORT_TUNNEL,
	PROG_VT_VD_INITIALVID_VLAN_DOMAIN_L1, /*used for vlan domain * mpls L1*/
    /* nof progs */
    PROG_VT_NOF_PROGS
} PROG_VT_PROGRAMS;

typedef enum
{
    /* TM: Used for both 1st and 2nd . No lookups */
    PROG_TT_TM_PROG,
    /* Unindex MPLS FRR: P1: ISB: MPLS L2 in case of not found FRR or regular process of MPLS */
    PROG_TT_L2,
    /* Unindex MPLS ISB: MPLS L3 in case of found FRR, port property FRR */
    PROG_TT_L3,
    /* Unindex MPLS ELI: ISB: MPLS L2+ELI in case of regular process of MPLS */
    PROG_TT_L2_L3ELI,
    /* ARP */
    PROG_TT_ARP_2ND_PROG,
    /* Bridging */
    PROG_TT_BRIDGE_STAR_2ND_PROG,
    /* IPV4: DIP found or dont care. */
    PROG_TT_IPV4_ROUTER_DIP_FOUND_PROG,
    /* IPV4: DIP found or dont care. */
    PROG_TT_IPV4_ROUTER_PORT_VXLAN_PROG,
    /* IPV4: DIP found or dont care. */
    PROG_TT_IPV4_ROUTER_PORT_L2_GRE_PROG,
    /* PWEoGRE: DIP found and signal to FLP it is PWEoGRE packet */
    PROG_TT_IPV4_ROUTER_PWE_GRE_DIP_FOUND_PROG,
    /* IPV4: DIP not found. Only used in case of RPA */
    PROG_TT_IPV4_ROUTER_MC_DIP_NOT_FOUND_PROG,
    /* IPV4: DIP not found. Only used in case of RPA */
    PROG_TT_IPV4_ROUTER_UC_DIP_NOT_FOUND_PROG,
    /* IP Router */
    PROG_TT_IP_ROUTER_2ND_PROG,
    /* IPV6 */
    PROG_TT_IPV6_ROUTER_2ND_PROG,
    /* IP compatible MC */
    PROG_TT_IP_ROUTER_CompatibleMc_2ND_PROG,
    /* Unknown L3 */
    PROG_TT_UNKNOWN_L3_PROG,
    /* MIM: with BTAG: ISA: ISID */
    PROG_TT_MAC_IN_MAC_WITH_BTAG_PROG,
    /* MIM: MC lookup : ISA: ISID */
    PROG_TT_MAC_IN_MAC_MC_PROG,
    /* FCoE with VFT : ISA: FC-D-ID */
    PROG_TT_FC_WITH_VFT_PROG,
    /* FCoE wo VFT : ISA: FC-D-ID */
    PROG_TT_FC_PROG,
    /* MPLS indexed with RIF */
    /* ISA: L2, ISB: L1+INRIF */
    PROG_TT_INDX_L1_L2_INRIF,
    /* ISA: L2+ELI, ISB: L1+INRIF */
    PROG_TT_INDX_L1_L2_L3ELI_INRIF,
    /* ISA: L2+ELI, ISB: L1+INRIF */
    PROG_TT_INDX_L1_L2ELI_INRIF,
    /* MPLS indexed */
    /* ISA: L2, ISB: L3 */
    PROG_TT_INDX_L3_L2,
    /* ISA: L2, ISB: L3 ,2 lookups with a fixed key(0x3) which will alwyas result with a hit in case of port termination */ 
    PROG_TT_INDX_MPLS_PORT_L3_L2, 
    /* ISA: L3 (L2 MPLS DB), ISB: (L3 MPLS DB) L4 */
    PROG_TT_INDX_L3_L4,
    /* ISA: L2-ELI (L2 MPLS DB) */
    PROG_TT_INDX_L2_L3ELI,
    /* MPLS tunnel termination only in TT stage { */
    /* Two lookups in SEM-A and B same Label. Done only in case MPLS1oE is defined */
    PROG_TT_INDX_L1_L1,
    /* Two lookups in SEM-A and B same Label. Done only in case MPLS2oE and second label is GAL. */
    PROG_TT_INDX_L1_L1_GAL,
    /* Two lookups in SEM-A L1 and SEM-B L2 */
    PROG_TT_INDX_L1_L2,
    /* Two lookups in SEM-A and B same Label. Done only in case MPLS3oE and third label is GAL. */
    PROG_TT_INDX_L1_L2_GAL,
    /* Two lookups in ISEM-A and B same Label (2). Done only in case MPLS2oE and first label is NULL (VT TCAM found) */
    PROG_TT_INDX_L2_L2,
    /* Two lookups in ISEM-A L2 and B L3. Done only in case 4 labels we have and the last one is GAL. First label is NULL (VT TCAM found) */
    PROG_TT_INDX_L2_L3_GAL,
    /* IP-LIF-DUMMY: for Dummy-DIP IP lookup at TT for mLDP(1 or 2 MPLS Labels) => 2 CAM lines in use */
    PROG_TT_MLDP_AFTER_RECYCLE,

    /* MPLS tunnel termination only in TT stage } */
    /* Trill */
    PROG_TT_TRILL_2ND_PROG,
    PROG_TT_TRILL_2ND_PROG_TWO_VLANS_AT_NATIVE_ETH,
    PROG_TT_TRILL_2ND_PROG_TRILL_TRAP_PROG,
    PROG_TT_DIP6_COMPRESSION_PROG,
    /* OAM statistics*/
    PROG_TT_OAM_STAT_PROG,
    PROG_TT_BFD_STAT_PROG,
    /* Port extender with untag check + MC routing*/
    PROG_TT_EXTENDER_UNTAG_CHECK_IP_MC,
    /* Port extender with untag check + UC routing*/
    PROG_TT_EXTENDER_UNTAG_CHECK_IP_UC,
    /* Port extender with untag check */
    PROG_TT_EXTENDER_UNTAG_CHECK,
    /* dummy lookup with IPV4  next protocol = GRE. */
    PROG_TT_IPV4_ROUTER_L3_GRE_PORT_PROG,
    /* DIP+SIP+VRF lookup. IP tunnel termination using VRF. */
    PROG_TT_IPV4_ROUTER_DIP_SIP_VRF_PROG,
    /* Dummy IP-LIF for Explicit NULL WA */
    PROG_TT_IPV4_FOR_EXPLICIT_NULL_PROG,
    /* Explicit NULL x2 with Tunnel */
    PROG_TT_TUNNEL_FOR_TWO_EXPLICIT_NULL_PROG,
    /* Explicit NULL x3 with Tunnel */
    PROG_TT_TUNNEL_FOR_THREE_EXPLICIT_NULL_PROG,
    /* Port extender + MC routing */
    PROG_TT_EXTENDER_IP_MC_PROG,
    /* Port extender + UC routing */
    PROG_TT_EXTENDER_IP_UC_PROG,
    /* Port extender */
    PROG_TT_EXTENDER_PROG,
    /* One lookup  in SEM-B ,for packet with 1 mpls labels*/
    PROG_TT_INDX_L1_GAL_ONLY,
    /* One lookup  in SEM-A ,for packet with 2 mpls labels*/
    PROG_TT_INDX_L2_GAL_ONLY,
    /*used for vlan domain * mpls L1*/
    PROG_TT_VLAN_DOMAIN_L1,
    /* nof progs */
    PROG_TT_NOF_PROGS
} PROG_TT_PROGRAMS;

/* length of the key stream that
* is big enough to contain either ipv6 uc / ipvc mc / ipv4 mc
* key stream
*/
#define ARAD_PP_ISEM_ACCESS_TCAM_KEY_LEN_BYTES 20
#define ARAD_PP_ISEM_ACCESS_TCAM_KEY_LEN_LONGS (ARAD_PP_ISEM_ACCESS_TCAM_KEY_LEN_BYTES / sizeof(uint32))

/* VTT TCAM DEFINES } */
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
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_ISEM_ACCESS_GET_PROCS_PTR = ARAD_PP_PROC_DESC_BASE_ISEM_ACCESS_FIRST,
  ARAD_PP_ISEM_ACCESS_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_ISEM_ACCESS_PROCEDURE_DESC_LAST
} ARAD_PP_ISEM_ACCESS_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_ISEM_ACCESS_UNKNOWN_KEY_TYPE_ERR = ARAD_PP_ERR_DESC_BASE_ISEM_ACCESS_FIRST,
  ARAD_PP_ISEM_ACCESS_NOT_READY_ERR,
  ARAD_PP_ISEM_ACCESS_MPLS_IN_RIF_NOT_SUPPORTED_ERR,  
  ARAD_PP_ISEM_ACCESS_LABEL_INDEX_NOT_SUPPORTED_ERR,
  ARAD_PP_ISEM_ACCESS_LABEL_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_ISEM_ACCESS_PROG_SEL_INCORRECT_DEF_ALLOCATION_ERR,
  ARAD_PP_ISEM_ACCESS_PROG_SEL_USR_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_ISEM_ACCESS_PROG_SEL_INVALID_LIF_KEY_RANGE_ERR,
  ARAD_PP_ISEM_ACCESS_AC_KEY_SET_NOT_SUPPORTED_ERR,
  ARAD_PP_ISEM_ACCESS_SOC_PROPERTIES_ERR,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TCAM_INVALID_ERR,
  ARAD_PP_ISEM_ACCESS_PROGRAMS_FULL_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_ISEM_ACCESS_ERR_LAST
} ARAD_PP_ISEM_ACCESS_ERR;

typedef enum
{
  /* VLAN domain lookup, done nothing on isem. Configured using Port-Default */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VD = 0,                                
  /* VLAN domain x Outer-VID type (includes Compression VLAN) */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VD_VID,                             
  /* VLAN domain x Outer-VID x Inner-VID type (includes Compression VLAN) */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VD_VID_VID,                         
  /* VLAN domain x Initial-VID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VD_INITIAL_VID,                     
  /* VLAN domain x Outer-PCP x Outer-VID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VD_PCP_VID,                         
  /* VLAN domain x Outer-PCP x Outer-VID x Inner-VID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VD_PCP_VID_VID,   
  /* PON VLAN domain x Tunnel-ID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_PON_VD_TUNNEL,
  /* PON VLAN domain x Tunnel-ID x Outer-VID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_PON_VD_TUNNEL_VID,
  /* PON VLAN domain x Tunnel-ID x Outer-VID x Inner-VID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_PON_VD_TUNNEL_VID_VID,
  /* PON VLAN domain x Tunnel-ID x Ethertype x Outer-PCP x Outer-VID x Inner-VID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_PON_VD_ETHERTYPE_TUNNEL_PCP_VID_VID,
  /* (MPLS/PWE) unindexed type */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS,                               
  /* (MPLS/PWE) indexed label outermost */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_L1,                            
  /* (MPLS/PWE) indexed label with In-RIF outermost */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_L1_IN_RIF,                     
  /* (MPLS/PWE) indexed label 2 (one after the outermost) */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_L2,                            
  /* (MPLS/PWE) indexed label 3 */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_L3,                            
  /* FRR */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_FRR,                           
  /* (MPLS/PWE) unindexed type with ELI */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_ELI,                           
  /* (MPLS/PWE) indexed label outermost with ELI */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_L1_ELI,                        
  /* (MPLS/PWE) indexed label 2 (one after the outermost) with ELI */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_L2_ELI,                       
  /* (MPLS/PWE) indexed label outermost with In-RIF and ELI */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_L1_IN_RIF_ELI,                
  /* ISID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_PBB,                               
  /* SIP - Learn info Tunnel */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_IP_TUNNEL_SIP,
  /* IP tunnel standard DIP in sem*/
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_IP_TUNNEL_DIP,                     
  /* IP tunnel extent in TCAM */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_IP_TUNNEL_SIP_DIP,
  /* TRILL NICK */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TRILL_NICK,  
  /* TRILL Appointed Forward */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TRILL_APPOINTED_FORWARDER,
  /* TRILL native inner TPID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TRILL_NATIVE_INNER_TPID,
  /* TRILL global to local VSI mapping */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TRILL_VSI,
  /* FCoE */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_FCOE,                              
  /* Spoof with IPV4 */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_SPOOF_IPV4,                        
  /* Spoof with IPV6 */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_SPOOF_IPV6,                         
  /* EoIP for SIP,DIP,Inner-VLAN */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_EoIP,                               
  /* GRE Key - 8 bytes */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_GRE,                                
  /* IPv6 tunnel */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_IPV6_TUNNEL_DIP,                                
  /* VNI Key */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VNI,  
  /* RPA key */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_RPA,  
  /* TRILL Designated-VID */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TRILL_DESIGNATED_VID,
  /* Flexible QinQ single tag */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_FLEXIBLE_Q_IN_Q_SINGLE_TAG, 
  /* Flexible QinQ double tag */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_FLEXIBLE_Q_IN_Q_DOUBLE_TAG, 
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TST_INITIAL_TAG, 
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TST_COMPRESSED_TAG, 
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TST_UNTAG, 
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TST_DOUBLE_TAG,                          
  /* (MPLS/PWE) indexed label 2 (one after the outermost) with GAL */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_PWE_L2_GAL,   
  /* Explicit NULL lookup (done in TCAM) */    
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_MPLS_EXPLICIT_NULL,
  /* IP6 compression, DIP6[127:64]xVSI[7:0] in TCAM */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_DIP6_TCAM_COMPRESSION,
  /* Spoof with SIP6[127:64]xVSI[7:0] in TCAM */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_SPOOF_SIP6_TCAM_COMPRESSION,
  /*oam statistics per MEP*/
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_OAM_STAT_TT,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_OAM_STAT_VT,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_BFD_STAT_VT,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_BFD_STAT_TT_ISA,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_BFD_STAT_TT_ISB,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_SRC_PORT_DA_DB_TCAM,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VSI_DA_DB_VRRP_TCAM,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_AC_TST2,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TST2_TCAM,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_TEST2_TCAM,
  ARAD_PP_ISEM_ACCESS_KEY_TEST2,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_EXTENDER_UNTAG_CHECK,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_EXTENDER_PE,
  /* IP tunnel extent in TCAM */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_IP_TUNNEL_SIP_DIP_VRF,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_IP_TUNNEL_MY_VTEP_INDEX_SIP_VRF,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_IPV4_MATCH_VT_DB_PROFILE,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_EFP,
  /* Custom PP port x tunnel */
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_CUSTOM_PP_PORT_TUNNEL,
  ARAD_PP_ISEM_ACCESS_KEY_TYPE_VLAN_DOMAIN_MPLS_L1,
  ARAD_PP_ISEM_ACCESS_NOF_KEY_TYPES
} ARAD_PP_ISEM_ACCESS_KEY_TYPE;

/* L2 Ethernet includes all L2 AC lookups */
  typedef struct arad_pp_isem_access_l2_eth_s
  {
    uint32 vlan_domain;
    uint32 outer_vid; /* outer_vid/initial_vid/bvid */
    uint32 inner_vid;
    uint32 outer_pcp;
    uint32 outer_dei; /* one bit for DEI */
    int    core_id;
  } arad_pp_isem_access_l2_eth_t;

  typedef struct arad_pp_isem_access_mpls_s
  {
    uint32 in_rif;  
    uint32 label;
    uint32 label2; /* used in case of Coupling and FRR */
    uint32 is_bos; /* indication label is bos */
    uint32 flags;  /* used to identify the entry is MPLS-LIF-dummy. */
    uint32 vlan_domain;
  } arad_pp_isem_access_mpls_t;

  typedef struct arad_pp_isem_access_ip_tunnel_s
  {
    uint32 dip;
    uint32 sip;    
    uint32 sip_prefix_len;
    uint32 dip_prefix_len;    
    SOC_SAND_PP_IPV6_SUBNET dip6;
    uint32 ipv4_next_protocol;
    uint32 ipv4_next_protocol_prefix_len;
    uint32 port_property; /* VTT Key var (only 4 lsbs) */
    uint8  port_property_en; /* VTT Key var enable - take into account */
    uint32 flags; /* used to identify the entry is IP-LIF-dummy. */
    uint32 vrf; /* VRF for tunnel termination */
    uint32 vrf_prefix_len; /* VRF for tunnel termination */
    uint32 my_vtep_index; 
    uint32 gre_ethertype;  /* GRE.ethertype */
    uint32 gre_ethertype_len; 
    uint32 ip_gre_tunnel; /* is IP-GRE tunnel */
    uint32 ip_gre_tunnel_en; 
  } arad_pp_isem_access_ip_tunnel_t;

  typedef struct arad_pp_isem_access_trill_s
  {
    uint32 nick_name;
    uint32 ing_nick;
    uint32 dist_tree;
    uint32 link_adj_id;
    uint32 native_inner_tpid;
    uint32 port;
    uint32 vsi;
    uint32 high_vid;
    uint32 low_vid;
    uint32 flags;
  } arad_pp_isem_access_trill_t;

  typedef struct arad_pp_isem_access_pbb_s
  {
    uint32 isid_domain;
    uint32 isid;
  } arad_pp_isem_access_pbb_t;

  typedef struct arad_pp_isem_access_l2_gre_s
  {
    uint32 gre_key;
    uint32 match_port_class;
  } arad_pp_isem_access_l2_gre_t;

  typedef struct arad_pp_isem_access_l2_vni_s
  {
    uint32 vni_key;
    uint32 match_port_class;
  } arad_pp_isem_access_l2_vni_t;

  typedef struct arad_pp_isem_access_fcoe_s
  {
    uint32 d_id; /* FC-D_ID, only bits [23:16] will be inserted to DB */
    uint32 vsan; /* 13 bits VFT of the packet */
  } arad_pp_isem_access_fcoe_t;

  /* PON */
  typedef struct arad_pp_isem_access_l2_pon_s
  {
    uint32 port; /* In sem converts to vlan domain */
    uint8  port_valid;
    uint32 outer_vid; 
    uint8  outer_vid_valid;
    uint32 inner_vid;
    uint8  inner_vid_valid;
    uint32 outer_pcp;
    uint8  outer_pcp_valid;
    uint32 outer_dei; 
    uint8  outer_dei_valid; 
    uint32 ether_type;
    uint8  ether_type_valid;
    uint32 tunnel_id;
    uint8  tunnel_id_valid;
    uint32 flags;
	int    core;
  } arad_pp_isem_access_l2_pon_t;

  typedef struct arad_pp_isem_access_spoof_v4_s
  {
    SOC_SAND_PP_MAC_ADDRESS smac; /* SA , only bits [47:41] will be inserted to DB */
    uint32 sip;     
  } arad_pp_isem_access_spoof_v4_t;

  typedef struct arad_pp_isem_access_spoof_v6_s
  {
    SOC_SAND_PP_MAC_ADDRESS smac; /* SA , only bits [47:32] will be inserted to DB */
    SOC_SAND_PP_IPV6_SUBNET sip6;
    uint32 vsi;
  } arad_pp_isem_access_spoof_v6_t;

  typedef struct arad_pp_isem_access_rpa_s
  {
    uint32 dip; /* only the 28 lsbs */
    uint32 vrf; /* only the 9 lsbs */
  } arad_pp_isem_access_rpa_t;

  typedef struct arad_pp_isem_access_ip6_compression_s
  {
    SOC_SAND_PP_IPV6_SUBNET ip6;
  } arad_pp_isem_access_ip6_compression_t;

  typedef struct arad_pp_isem_access_oam_stat_s
  {
    uint32 cfm_eth_type;
    uint32 opaque;
    uint32 bfd_format;
    uint32 pph_type;
  } arad_pp_isem_access_oam_stat_t;
   

typedef union
{
  /* L2 Ethernet includes all L2 AC lookups */
  arad_pp_isem_access_l2_eth_t l2_eth;
  arad_pp_isem_access_mpls_t mpls;
  arad_pp_isem_access_ip_tunnel_t ip_tunnel;
  arad_pp_isem_access_trill_t trill;
  arad_pp_isem_access_pbb_t pbb;
  arad_pp_isem_access_l2_gre_t l2_gre;
  arad_pp_isem_access_l2_vni_t l2_vni;
  arad_pp_isem_access_fcoe_t fcoe;
  arad_pp_isem_access_l2_pon_t l2_pon;
  arad_pp_isem_access_spoof_v4_t spoof_v4;
  arad_pp_isem_access_spoof_v6_t spoof_v6;
  arad_pp_isem_access_rpa_t rpa;
  arad_pp_isem_access_ip6_compression_t ip6_compression;
  arad_pp_isem_access_oam_stat_t oam_stat;
} ARAD_PP_ISEM_ACCESS_KEY_INFO;

typedef struct
{
  ARAD_PP_ISEM_ACCESS_KEY_TYPE key_type;
  ARAD_PP_ISEM_ACCESS_KEY_INFO key_info;
} ARAD_PP_ISEM_ACCESS_KEY;

typedef struct
{
  uint32 sem_result_ndx;

} ARAD_PP_ISEM_ACCESS_ENTRY;

#define ARAD_PP_ISEM_ACCESS_NOF_LABEL_HANDLES 3

typedef struct
{
  int prog_used; /* prog ID in HW, -1 if not used */    
} ARAD_PP_ISEM_ACCESS_PROGRAM_INFO;

#define ARAD_PP_SW_DB_PP_PORTS_NOF_U32  (ARAD_PP_NOF_PORTS/32)

typedef struct
{
  SHR_BITDCL pon_double_lookup_enable[ARAD_PP_SW_DB_PP_PORTS_NOF_U32];
} ARAD_PON_DOUBLE_LOOKUP;

typedef struct
{
  uint8 mpls_use_in_rif;
  uint8 port_vlan_pcp_lookup;
  uint8 match_port_vlan_critiria_64K;
  uint8 mpls_index;
  uint8 spoof_enable;
  uint8 trill_mode; /* 0 - disabled. 1 - VL legacy. 2 - FGL supported */
  uint8 trill_appointed_fwd; /* fine-grained-A mode */
  uint8 ipv6_term_enable;
  uint8 ipv4_term_enable;
  uint8 ipv4_term_dip_sip_enable; /* 0 - disable, 1 - enable, 2 - include IPV4 next protocol, 3 - include VRF, 4 - my-vtep-index, SIP, VRF in ISEM and DIP SIP VRF in TCAM */
  uint8 e_o_ip_enable;
  uint8 vxlan_enable;
  uint8 nvgre_enable;
  uint8 spoof_ipv6_enable;
  uint8 fast_reroute_labels_enable;
  uint8 is_bos_in_key_enable;
  uint8 pon_enable; /* If set, PON application is enabled (at least one port is PON port) */
  uint8 use_pon_tcam_lkup; /* If set, Also enable loookup of VD(3bits), Tunnel, 2VLANs, Ethertype and COS */
  uint8 tls_db_enable; /* If set, PON application include also TLS DB */
  uint8 custom_pon_enable; /* custom PON two tags application for ARAD+ */
  uint8 tls_in_tcam_enable; /* If set, TLS DB resides in TCAM */
  uint8 evb_enable; /* If set, EVB application is on */
  uint8 fcoe_enable; /* If set, FCOE is enabled*/
  uint8 ipv4mc_bidir_enable; /* If set, IPV4 MC BIDIR is enabled*/
  uint8 mim_enable; /* If set, MiM is enabled */
  uint8 eli_enable; /* If set, device supports ELI programs */
  uint8 mpls_1_namespace;  /* 0 - L1, 1 - L2, 2 - L3, 4 - L1L3, 5 - L1L2, 6 - L2L3 , 255 - Invalid */
  uint8 mpls_2_namespace;  /* 0 - L1, 1 - L2, 2 - L3, 4 - L1L3, 5 - L1L2, 6 - L2L3 , 255 - Invalid */
  uint8 mpls_3_namespace;  /* 0 - L1, 1 - L2, 2 - L3, 4 - L1L3, 5 - L1L2, 6 - L2L3 , 255 - Invalid */
  uint8 mpls_1_database;   /* 0 - SEM-A, 1 - SEM-B, 255 - invalid */
  uint8 mpls_2_database;   /* 0 - SEM-A, 1 - SEM-B, 255 - invalid */
  uint8 mpls_3_database;   /* 0 - SEM-A, 1 - SEM-B, 255 - invalid */
  uint8 mpls_tp_mac_address; /* 1 - enable */
  uint8 trill_disable_designated_vlan_check; /* If set, bcmPortControlTrillDesignatedVlan is not applicable*/
  uint8 designated_vlan_inlif_enable;
  uint8 q_in_q_ip_5_tuple; /* If set, special port property prefer IPV4 5-tuple */
  uint8 custom_feature_vt_tst1; /* If set, Test 1 is set */
  uint8 custom_feature_vt_tst2; /* If set, Test 2 is set */
  uint8 custom_feature_vrrp_scaling_tcam;
  uint8 ingress_full_mymac_1; /* If set, full myMac 1 is set */
  uint8 tunnel_termination_in_tt_only; /* If set, Tunnel termination only in TT stage */
  uint8 pwe_gal_support; /* IF set, PWE-GAL database exist */
  uint8 trill_transparent_service; /* 0 - disabled. 1 - enabled */
  uint8 explicit_null_arad_plus_support; /* If set, Explicit-null database exist */
  uint8 port_raw_mpls; /* 1 - enable */
  uint8 compression_spoof_ip6_enable;
  uint8 compression_ip6_enable;
  uint8 test1;
  uint8 extender_cb_enable;     /* If set, Port Extender Control Bridge is Enabled */
  uint8 evpn_enable;
  uint8 custom_pon_ipmc; /*  If set, IPMC in PON port is disabled and IPMC in NNI port is enabled */
  uint8 oam_enable; /*If set oam is enable */
  uint8 oam_statistics_enable; /* If statistics enable,relevant for arad+ only */
}ARAD_PP_ISEM_ACCESS_PROGRAMS_SOC_PROPERTIES;

typedef struct
{
  SOC_SAND_HASH_TABLE_PTR isem_key_to_entry_id;
} ARAD_VTT;

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

extern ARAD_PP_ISEM_ACCESS_PROGRAM_INFO vt_programs[SOC_MAX_NUM_DEVICES][PROG_VT_NOF_PROGS];
extern ARAD_PP_ISEM_ACCESS_PROGRAM_INFO tt_programs[SOC_MAX_NUM_DEVICES][PROG_TT_NOF_PROGS];

extern ARAD_PP_ISEM_ACCESS_PROGRAMS_SOC_PROPERTIES g_prog_soc_prop[BCM_MAX_NUM_UNITS];
extern int   is_g_prog_soc_prop_initilized[BCM_MAX_NUM_UNITS];

extern int    is_db_in_use[SOC_DPP_DBAL_SW_TABLE_ID_VTT_LAST - SOC_DPP_DBAL_SW_TABLE_ID_VTT_FIRST + 1];
/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */
uint32 arad_pp_isem_access_programs_soc_properties_get(int unit);
uint32 arad_pp_isem_prog_programs_init(int unit);

uint32 arad_pp_vtt_1st_lookup_program_swap_sem(
    int unit,
    uint32 swap,
    ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION_TBL_DATA* tbl_data
);

void 
arad_pp_isem_access_deinit(int unit);
uint32
  arad_pp_isem_access_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pp_isem_access_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY             *isem_key,
    SOC_SAND_OUT ARAD_PP_ISEM_ACCESS_ENTRY           *isem_entry,
    SOC_SAND_OUT uint8                               *success
  );

uint32
  arad_pp_isem_access_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY             *isem_key,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_ENTRY           *isem_entry,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE            *success
  );

uint32
  arad_pp_isem_access_isem_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 *key_buffer,
    SOC_SAND_IN  uint32                                 *entry_buffer,
    SOC_SAND_IN  uint32                                 tables_access_id,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
  );

uint32 
    arad_pp_isem_access_isem_entry_remove_unsafe(int unit, uint32 key_buffer[ARAD_PP_ISEM_ACCESS_KEY_SIZE], uint32 tables_access_id);

uint32 
    arad_pp_isem_access_isem_entry_get_unsafe(int unit, uint32 key_buffer[ARAD_PP_ISEM_ACCESS_KEY_SIZE], uint32 entry_buffer[SOC_DPP_DEFS_MAX(ISEM_PAYLOAD_NOF_UINT32)], uint32 tbl_access_id, uint8* found);

uint32
  arad_pp_isem_access_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY             *isem_key
  );

uint32 arad_pp_isem_prefix_from_buffer(SOC_SAND_IN  int       unit,
                                       SOC_SAND_IN  uint32   *buffer,
                                       SOC_SAND_IN  uint32    lookup_num,
                                       SOC_SAND_IN  uint32    tables_access_id,
                                       SOC_SAND_OUT uint32   *prefix );

uint32
  arad_pp_isem_access_prefix_to_key_type(SOC_SAND_IN int                           unit,
                                         SOC_SAND_IN uint32                        isem_prefix,
                                         SOC_SAND_IN uint32                        lookup_num,
                                         SOC_SAND_IN uint32                        tables_access_id,
                                         SOC_SAND_IN uint32                        *buffer,
                                         SOC_SAND_OUT ARAD_PP_ISEM_ACCESS_KEY_TYPE *isem_key_in);

uint32
   arad_pp_isem_access_tcam_callback(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32  user_data
    );

uint32
  arad_pp_isem_access_key_from_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 *buffer,
    SOC_SAND_IN  uint32                                 lookup_num,
    SOC_SAND_IN  uint32                                 tables_access_id,
    SOC_SAND_OUT ARAD_PP_ISEM_ACCESS_KEY                *isem_key
  );

uint32
  arad_pp_isem_access_entry_from_buffer(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                    *buffer,
    SOC_SAND_OUT ARAD_PP_ISEM_ACCESS_ENTRY *isem_entry
  );

uint32
    arad_pp_isem_access_tcam_db_id_get(
      SOC_SAND_IN ARAD_PP_ISEM_ACCESS_KEY_TYPE key_type);


uint32
  arad_pp_isem_access_tcam_rewrite_entry(
     SOC_SAND_IN  int                        unit,
     SOC_SAND_IN  uint8                      entry_exists,
     SOC_SAND_IN  ARAD_TCAM_GLOBAL_LOCATION  *global_location,
     SOC_SAND_IN  ARAD_TCAM_LOCATION         *location
  );

uint32
  arad_pp_isem_access_key_to_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY                   *isem_key,
    SOC_SAND_IN  uint32                                 table_access_id,
    SOC_SAND_IN  uint32                                 insert_index, /* Inseration iteration */
    SOC_SAND_OUT uint32                                  *buffer
  );

uint32
  arad_pp_l2_lif_ac_key_to_sem_key_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                         *ac_key,
    SOC_SAND_OUT ARAD_PP_ISEM_ACCESS_KEY                       *isem_key
  );

uint32
  arad_pp_isem_access_sem_tables_get(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY                 *isem_key,
    SOC_SAND_OUT uint32                                  *nof_tables,
    SOC_SAND_OUT uint32                                  *tables_access_ids,
    SOC_SAND_OUT uint8                                   *is_duplicate_entry
  );

/* 
 * Retreive Enable / disable in rif key types (being set on init)
 */
uint32
  arad_pp_isem_access_prog_sel_in_rif_key_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT uint8                                 *with_in_rif
  );

/* 
 * Set different ethernet programs than default configuration. 
 */
uint32
  arad_pp_isem_access_prog_sel_ac_key_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 entry_ndx,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER           *qual_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE            *key_mapping
  );

uint32
  arad_pp_isem_access_prog_sel_ac_key_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 entry_ndx,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER           *qual_key,
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE            *key_mapping    
  );

void
  arad_pp_isem_access_print_all_programs_data(
    SOC_SAND_IN  int                                 unit
  );

char*
  arad_pp_isem_access_print_tt_program_data(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32 program_id,
    SOC_SAND_IN  uint32 to_print
  );

uint32 arad_pp_isem_access_print_vt_program_look_info(int unit, int prog_id);

uint32 arad_pp_isem_access_print_tt_program_look_info(int unit, int prog_id);

char* arad_pp_isem_access_print_vt_program_data(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32 program_id,
    SOC_SAND_IN  uint32 to_print
  );


void
  arad_pp_isem_access_program_sel_line_to_program_id(
    SOC_SAND_IN   int                 unit,
    SOC_SAND_IN   int                 line,    
    SOC_SAND_IN   int                 is_vt,/* if 1 give vt program if 0 give tt program*/
    SOC_SAND_OUT  uint8               *prog_id
  ) ;
/*********************************************************************
* NAME:
 *   arad_pp_isem_access_print_last_vtt_program_data
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Dump last VT&TT program invoked.
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   This API must be called during a continuous stream of
 *   the identical packets coming from the same source.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_isem_access_print_last_vtt_program_data(
    SOC_SAND_IN   int                 unit,
    SOC_SAND_IN   int                 core_id,
    SOC_SAND_IN   int                 to_print,
    SOC_SAND_OUT  int                 *prog_id_vt,
    SOC_SAND_OUT  int                 *prog_id_tt,
    SOC_SAND_OUT  int                 *num_of_progs_vt,
    SOC_SAND_OUT  int                 *num_of_progs_tt
  );


/*********************************************************************
* NAME:
 *   arad_pp_isem_access_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_isem_access module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_isem_access_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_isem_access_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_isem_access module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_isem_access_get_errs_ptr(void);

/* } */
#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_ISEM_ACCESS_INCLUDED__*/
#endif


