/* $Id: arad_parser.c,v 1.43 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_HEADERS

#include <soc/mcm/memregs.h>
#include <soc/mem.h>
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <sal/compiler.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/arad_framework.h>
#include <soc/dpp/ARAD/arad_parser.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ptp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
/*#include <soc/dpp/ARAD/ARD_TM/ard_pmf_low_level.h>*/

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION            (0x0010) /* Base-Offset resolution */
#define ARAD_PARSER_PROG_MULTIPLIER                        (0x0040)
#define ARAD_PARSER_DONT_CARE                              (0x0000) /* When action is ARAD_PARSER_PROG_END we don't care abut the value of the macro select */
#define ARAD_PARSER_PFC_ERROR                              (0x0000) /* When action is ARAD_PARSER_PROG_END we don't care abut the value of the macro select */


#define ARAD_PARSER_PROG_FIN                               (0x0000) /* Execute instruction's command and finish */
#define ARAD_PARSER_PROG_END                               (0x007f) /* End and do not execute the instruction's command */

#define ARAD_PARSER_MACRO_SEL_0                            (0x0000) /* Trill | Ingress shaping */
#define ARAD_PARSER_MACRO_SEL_1                            (0x0001) /* Udp custom macro */
#define ARAD_PARSER_MACRO_SEL_2                            (0x0002) /* UDP | indicate where is the TM forwarding field */
#define ARAD_PARSER_MACRO_SEL_3                            (0x0003) /* additinal tag */
#define ARAD_PARSER_MACRO_SEL_TM_BASE                      (0x0004) /* Base TM custom macro - base set per PP-Port */
#define ARAD_PARSER_MACRO_SEL_ETH                          (0x0004)
#define ARAD_PARSER_MACRO_SEL_IPV4                         (0x0005)
#define ARAD_PARSER_MACRO_SEL_IPV6                         (0x0006)
#define ARAD_PARSER_MACRO_SEL_MPLS                         (0x0007)
/* parse udp header to check if there VXLAN */
#define ARAD_PARSER_MACRO_SEL_UDP_VX                       (ARAD_PARSER_MACRO_SEL_3) /* M3 */
/* parse udp header  */
#define ARAD_PARSER_MACRO_SEL_UDP                          (ARAD_PARSER_MACRO_SEL_1) /* M1 */
/* parse GTP header */
#define ARAD_PARSER_MACRO_SEL_GTP                           (ARAD_PARSER_MACRO_SEL_0)
/* parse ipv6 extension header */
#define ARAD_PARSER_MACRO_SEL_IPV6_EXT_1                   (ARAD_PARSER_MACRO_SEL_0)
#define ARAD_PARSER_MACRO_SEL_IPV6_EXT_2                   (ARAD_PARSER_MACRO_SEL_1)
#define ARAD_PARSER_MACRO_SEL_ADD_TPID                     (ARAD_PARSER_MACRO_SEL_1)

/*EoE*/
#define ARAD_PARSER_MACRO_SEL_EoE                          0x8 /* (ARAD_PARSER_MACRO_SEL_2)*/

#define ARAD_PARSER_PROG_TM_BASE                           (0x0001)
#define ARAD_PARSER_PROG_TM_BASE_START                     (ARAD_PARSER_PROG_TM_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_E_BASE                            (0x0003)
#define ARAD_PARSER_PROG_E_BASE_START                      (ARAD_PARSER_PROG_E_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPV4oE_BASE                       (0x0004)
#define ARAD_PARSER_PROG_IPV4oE_BASE_START                 (ARAD_PARSER_PROG_IPV4oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_MPLSoE_BASE                       (0x0005)
#define ARAD_PARSER_PROG_MPLSoE_BASE_START                 (ARAD_PARSER_PROG_MPLSoE_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPV6oE_BASE                       (0x0006)
#define ARAD_PARSER_PROG_IPV6oE_BASE_START                 (ARAD_PARSER_PROG_IPV6oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_TRILLoE_BASE                      (0x0007)
#define ARAD_PARSER_PROG_TRILLoE_BASE_START                (ARAD_PARSER_PROG_TRILLoE_BASE * ARAD_PARSER_PROG_MULTIPLIER)

/* base for IPv6oMPLS3oE */
#define ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE                 (9)
#define ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START           (ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)

/* base for MPLSoMPLS3oE: more than 3 MPLS labels */
#define ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                 (0x000a)
#define ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START           (ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)

#define ARAD_PARSER_PROG_IPoMPLS1oE_BASE                   (0x000b)
#define ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START             (ARAD_PARSER_PROG_IPoMPLS1oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPoMPLS2oE_BASE                   (0x000c)
#define ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START             (ARAD_PARSER_PROG_IPoMPLS2oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPoMPLS3oE_BASE                   (0x000d)
#define ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START             (ARAD_PARSER_PROG_IPoMPLS3oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IP4oIP4oE_BASE                    (0x000e)
#define ARAD_PARSER_PROG_IP4oIP4oE_BASE_START              (ARAD_PARSER_PROG_IP4oIP4oE_BASE  * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_FCoE1_BASE                        (0x000f)
#define ARAD_PARSER_PROG_FCoE1_BASE_START                  (ARAD_PARSER_PROG_FCoE1_BASE      * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_FCoE2_BASE                        (0x0010)
#define ARAD_PARSER_PROG_FCoE2_BASE_START                  (ARAD_PARSER_PROG_FCoE2_BASE      * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_FCoE3_BASE                        (0x0011)
#define ARAD_PARSER_PROG_FCoE3_BASE_START                  (ARAD_PARSER_PROG_FCoE3_BASE      * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_EoTRILoE_BASE                     (0x0012)
#define ARAD_PARSER_PROG_EoTRILoE_BASE_START               (ARAD_PARSER_PROG_EoTRILoE_BASE   * ARAD_PARSER_PROG_MULTIPLIER)

/* parse UDP (~again)*/
#define ARAD_PARSER_PROG_UDPoIPV4oE_BASE                   (19)
#define ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START             (ARAD_PARSER_PROG_UDPoIPV4oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)


/* limited GTP and FCOE are not expected to co-exist on same system */
#define ARAD_PARSER_PROG_GTP_UDPoIPV4oE_BASE               (ARAD_PARSER_PROG_FCoE1_BASE)
#define ARAD_PARSER_PROG_GTP_UDPoIPV4oE_BASE_START         (ARAD_PARSER_PROG_FCoE1_BASE_START)

#define ARAD_PARSER_PROG_GTP_UDPoIPV6oE_BASE               (ARAD_PARSER_PROG_FCoE2_BASE)
#define ARAD_PARSER_PROG_GTP_UDPoIPV6oE_BASE_START         (ARAD_PARSER_PROG_FCoE2_BASE_START)

#define ARAD_PARSER_PROG_UDPoIPV6oE_BASE                   (20)
#define ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START             (ARAD_PARSER_PROG_UDPoIPV6oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)

/* parse EthoUDP expect IPv4*/
#define ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE                 (21)
#define ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START           (ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)

/* Parse IPv6 extension header */
#define ARAD_PARSER_PROG_IPV6_EXT_1_BASE                   (22)
#define ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START             (ARAD_PARSER_PROG_IPV6_EXT_1_BASE      * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE               (23)
#define ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START         (ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE  * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE              (24)
#define ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START        (ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE              (25)
#define ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START        (ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE * ARAD_PARSER_PROG_MULTIPLIER)

#define ARAD_PARSER_PROG_ADD_TPID_BASE                     (26)
#define ARAD_PARSER_PROG_ADD_TPID_BASE_START               (ARAD_PARSER_PROG_ADD_TPID_BASE * ARAD_PARSER_PROG_MULTIPLIER)

#define ARAD_PARSER_PROG_RAW_MPLS_BASE                     (27)
#define ARAD_PARSER_PROG_RAW_MPLS_BASE_START               (ARAD_PARSER_PROG_RAW_MPLS_BASE * ARAD_PARSER_PROG_MULTIPLIER)

/* base for IPoETHoVxLANoUDPoIPoE */
#define ARAD_PARSER_PROG_IP_VXLAN_BASE                     (28)
#define ARAD_PARSER_PROG_IP_VXLAN_BASE_START               (ARAD_PARSER_PROG_IP_VXLAN_BASE * ARAD_PARSER_PROG_MULTIPLIER)

/* base for IPoMiM */
#define ARAD_PARSER_PROG_IP_MiM_BASE                       (29)
#define ARAD_PARSER_PROG_IP_MiM_BASE_START                 (ARAD_PARSER_PROG_IP_MiM_BASE * ARAD_PARSER_PROG_MULTIPLIER)

/* base for IPv6oMPLS1/2oE */
#define ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE                 (30)
#define ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START           (ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)
#define ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE                 (31)
#define ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START           (ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE * ARAD_PARSER_PROG_MULTIPLIER)

#define ARAD_PARSER_CUSTOM_MACRO_PROTO_1                   (0x0) /* Ingress shaping - start */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_2                   (0x1) /* Ingress shaping - end */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_3                   (0x2) /* OutLIF - start */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_4                   (0x3) /* OutLIF - end */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_5                   (0x4) /* MC-Flow-ID - start */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_6                   (0x5) /* MC-Flow-ID - end */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_7                   (0x6) /* BFD single hop */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_8                   (0x7) /* VXLAN */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_9                   (0x8) /* micro BFD : Used in Jericho only*/
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_10                  (0x9) /* PTP (1588) - start */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_11                  (0xa) /* PTP (1588) - end */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_12                  (0xb) /* UDP Tunnel Dummy or GTP store value 4*/
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_13                  (0xc) /* UDP Tunnel IPv4 dst port or GTP store value 6*/
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_14                  (0xd) /* FCoE VFT or UDP Tunnel IPv6 dst port or GTP port 2152 */
#define ARAD_PARSER_CUSTOM_MACRO_PROTO_15                  (0xe) /* FCoE encap or UDP Tunnel MPLS dst port or GTP port 2123 */

#define ARAD_PARSER_FLOW_ID_BITS                           (0x30000)

#define ARAD_PARSER_NOF_ETH_REPLICATIONS                   (1)
#define ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS       (3)


#define ARAD_ITMH_DEST_ISQ_FLOW_ID_PREFIX                  (0x4) /*3b'100*/
#define ARAD_ITMH_DEST_OUT_LIF_ID_PREFIX                   (0x5) /*3b'101*/
#define ARAD_ITMH_DEST_MC_FLOW_ID_PREFIX                   (0xD) /*4b'1101*/



/* Maps of first nibble after MPLS BOS label to next protocol value */

#define ARAD_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_5         (0x4) /* ipv4 */
#define ARAD_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_7         (0x6) /* ipv6 */

#define ARAD_PARSER_MPLS_NEXT_PROTOCOL_TCP                 (8)
#define ARAD_PARSER_MPLS_NEXT_PROTOCOL_UDP                 (9)
#define ARAD_PARSER_MPLS_NEXT_PROTOCOL_NOF                 (16)



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

typedef struct
{
  uint32 instruction_addr;
  uint32 macro_sel;
  uint32 next_addr_base;
  uint32 packet_format_code;
  uint32 parser_pmf_profile;
}ARAD_PARSERR_INFO;

/* Application Entry ID in custom
 * macro dynamic allocation array
 */
typedef enum
{
    ARAD_PARSER_DYNAMIC_MACRO_TRILL,
    ARAD_PARSER_DYNAMIC_MACRO_FCOE_1,
    ARAD_PARSER_DYNAMIC_MACRO_FCOE_2,
    ARAD_PARSER_DYNAMIC_MACRO_VXLAN,
    ARAD_PARSER_DYNAMIC_MACRO_UDP,
    ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_1,
    ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_2,
    ARAD_PARSER_DYNAMIC_MACRO_ADD_TPID,
    ARAD_PARSER_DYNAMIC_MACRO_NULL,
	ARAD_PARSER_DYNAMIC_MACRO_EoE,
    ARAD_PARSER_DYNAMIC_MACRO_GTP,
    ARAD_PARSER_DYNAMIC_MACRO_UDP_TUNNEL,

    /* always last */
    ARAD_PARSER_DYNAMIC_MACRO_COUNT

} ARAD_PARSER_DYNAMIC_MACRO_APP;

/* Available custom macros */
typedef enum
{
    ARAD_PARSER_CUSTOM_MACRO_0 = ARAD_PARSER_MACRO_SEL_0,
    ARAD_PARSER_CUSTOM_MACRO_1 = ARAD_PARSER_MACRO_SEL_1,
    ARAD_PARSER_CUSTOM_MACRO_2 = ARAD_PARSER_MACRO_SEL_2,
    ARAD_PARSER_CUSTOM_MACRO_3 = ARAD_PARSER_MACRO_SEL_3,

    /* always last */
    ARAD_PARSER_CUSTOM_MACRO_COUNT,
    ARAD_PARSER_CUSTOM_MACRO_INVALID = ARAD_PARSER_CUSTOM_MACRO_COUNT

} ARAD_PARSER_CUSTOM_MACRO;

/* Mapping table entry:
 * Static custom macros to Dynamic custom macro
 */
typedef struct
{
    uint32 instruction_addr;
    uint32 static_macro_sel;
    ARAD_PARSER_DYNAMIC_MACRO_APP app_id;
    uint32 include_prog_end;

} ARAD_PARSER_STATIC_TO_DYNAMIC_MACRO;

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* Parser Custom Macros - Dynamic Allocation
 * In Arad/Arad+ there are 4 custom macros that are dynamically allocated 
 * according to the supported applications (enabled by soc property).
 */

/* for each application, entry indicates allocated custom macros */
static ARAD_PARSER_CUSTOM_MACRO dynamic_macro[SOC_SAND_MAX_DEVICE][ARAD_PARSER_DYNAMIC_MACRO_COUNT];

static char *parser_static_macro[] =
{
    "Custom-Macro-0",
    "Custom-Macro-1",
    "Custom-Macro-2",
    "Custom-Macro-3",
    "Eth",
    "IPv4",
    "IPv6",
    "MPLS",
};

static char *parser_dynamic_macro[] =
{
    "TRILL",      /* [ARAD_PARSER_DYNAMIC_MACRO_TRILL]      = */
    "FCoE-1",     /* [ARAD_PARSER_DYNAMIC_MACRO_FCOE_1]     = */
    "FCoE-2",     /* [ARAD_PARSER_DYNAMIC_MACRO_FCOE_2]     = */
    "VxLAN",      /* [ARAD_PARSER_DYNAMIC_MACRO_VXLAN]      = */
    "UDP",        /* [ARAD_PARSER_DYNAMIC_MACRO_UDP]        = */
    "IPv6-Ext-1", /* [ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_1] = */
    "IPv6-Ext-2", /* [ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_2] = */
    "TPID",       /* [ARAD_PARSER_DYNAMIC_MACRO_ADD_TPID]   = */
    "NULL",       /* [ARAD_PARSER_DYNAMIC_MACRO_NULL]       = */
    "EoE",        /* [ARAD_PARSER_DYNAMIC_MACRO_EoE]        = */
    "GTP",        /* [ARAD_PARSER_DYNAMIC_MACRO_GTP]        = */
    "UDP-Tunnel"  /* [ARAD_PARSER_DYNAMIC_MACRO_UDP_TUNNEL] = */
};

/* custom macro bitmaps, for each bit indication if used */
static int arad_parser_custom_macro_used_bmp[SOC_SAND_MAX_DEVICE][1]; 

static ARAD_PARSER_STATIC_TO_DYNAMIC_MACRO 
    Arad_parser_macro_map[] =
{
/*  Program Base                                    Static Macro                        Dynamic Macro ID                        All Next Addresses */

    {ARAD_PARSER_PROG_RAW_MPLS_ADDR_START,          ARAD_PARSER_MACRO_SEL_0,            ARAD_PARSER_DYNAMIC_MACRO_NULL,         FALSE},
};

/* Mapping table from static custom macros to dynamic custom macros.
 * Arad_parserr_info_eth table contains for each instruction base which macro 
 * to operate and which is the next address base. 
 * The macro to operate is static, and in case it is a custom macro, its static 
 * value is not relevant anymore, and should be replaced with the newly 
 * dynamic allocated macro. The mapping is as follows: 
 * instruction base + static macro ->  entry ID in custom macro alloc table 
 * The entry ID in the dynamic macro table is constat and according to application. 
 * The last colums indicates if macro is operated even when next address base 
 * is empty (ARAD_PARSER_PROG_END).
 */
static ARAD_PARSER_STATIC_TO_DYNAMIC_MACRO 
    Arad_parser_macro_eth_map[] =
{
/*  Program Base                                    Static Macro                        Dynamic Macro ID                        All Next Addresses */

    {ARAD_PARSER_PROG_E_BASE_START,                 ARAD_PARSER_MACRO_SEL_0,            ARAD_PARSER_DYNAMIC_MACRO_TRILL,        FALSE},
    {ARAD_PARSER_PROG_E_BASE_START,                 ARAD_PARSER_MACRO_SEL_ADD_TPID,     ARAD_PARSER_DYNAMIC_MACRO_ADD_TPID,     FALSE},
    {ARAD_PARSER_PROG_E_BASE_START,                 ARAD_PARSER_MACRO_SEL_2,            ARAD_PARSER_DYNAMIC_MACRO_FCOE_2,       FALSE},
    {ARAD_PARSER_PROG_E_BASE_START,                 ARAD_PARSER_MACRO_SEL_3,            ARAD_PARSER_DYNAMIC_MACRO_VXLAN,        FALSE},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START,          ARAD_PARSER_MACRO_SEL_0,            ARAD_PARSER_DYNAMIC_MACRO_TRILL,        FALSE},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START,          ARAD_PARSER_MACRO_SEL_2,            ARAD_PARSER_DYNAMIC_MACRO_FCOE_2,       FALSE},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START,          ARAD_PARSER_MACRO_SEL_3,            ARAD_PARSER_DYNAMIC_MACRO_VXLAN,        FALSE},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START,            ARAD_PARSER_MACRO_SEL_UDP_VX,       ARAD_PARSER_DYNAMIC_MACRO_VXLAN,        FALSE},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START,         ARAD_PARSER_MACRO_SEL_1,            ARAD_PARSER_DYNAMIC_MACRO_UDP,          FALSE},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START,            ARAD_PARSER_MACRO_SEL_UDP,          ARAD_PARSER_DYNAMIC_MACRO_UDP,          FALSE},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START,            ARAD_PARSER_MACRO_SEL_IPV6_EXT_1,   ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_1,   FALSE},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START,        ARAD_PARSER_MACRO_SEL_IPV6_EXT_2,   ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_2,   TRUE},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START,    ARAD_PARSER_MACRO_SEL_UDP,          ARAD_PARSER_DYNAMIC_MACRO_UDP,          TRUE},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START,        ARAD_PARSER_MACRO_SEL_1,            ARAD_PARSER_DYNAMIC_MACRO_UDP,          FALSE},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START,        ARAD_PARSER_MACRO_SEL_1,            ARAD_PARSER_DYNAMIC_MACRO_UDP,          FALSE},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START,        ARAD_PARSER_MACRO_SEL_1,            ARAD_PARSER_DYNAMIC_MACRO_UDP,          FALSE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START,             ARAD_PARSER_MACRO_SEL_0,            ARAD_PARSER_DYNAMIC_MACRO_FCOE_1,       TRUE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START,             ARAD_PARSER_MACRO_SEL_0,            ARAD_PARSER_DYNAMIC_MACRO_FCOE_1,       TRUE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START,             ARAD_PARSER_MACRO_SEL_0,            ARAD_PARSER_DYNAMIC_MACRO_FCOE_1,       TRUE},
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START,        ARAD_PARSER_MACRO_SEL_UDP,          ARAD_PARSER_DYNAMIC_MACRO_UDP,          FALSE},
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START,        ARAD_PARSER_MACRO_SEL_UDP,          ARAD_PARSER_DYNAMIC_MACRO_UDP,          FALSE},
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START,        ARAD_PARSER_MACRO_SEL_GTP,          ARAD_PARSER_DYNAMIC_MACRO_GTP,          FALSE},
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START,        ARAD_PARSER_MACRO_SEL_GTP,          ARAD_PARSER_DYNAMIC_MACRO_GTP,          FALSE},
    {ARAD_PARSER_PROG_E_BASE_START,                 ARAD_PARSER_MACRO_SEL_EoE,          ARAD_PARSER_DYNAMIC_MACRO_EoE,          FALSE},
};


/*
 * Parser-Program-Memory map:
 * 0x0000 - 0x0004    Program entry points, indexed by PP-Context - Arad_parser_info
 * 0x0005 - 0x000f    Unused
 * 0x0010 - 0x002f    TM program, after checking for destination info - Arad_parserr_info_tm
 * 0x0070 - 0x00af    Ethernet program - Arad_parserr_info_eth
 * 0x00b0 - 0x00ef    IPv4oE program - Arad_parserr_info_eth
 * 0x00f0 - 0x010d    MPLSoE program - Arad_parserr_info_eth
 * 0x0200 - 0x023f    TRILLoE program - Arad_parserr_info_eth
 * 0x0240 - 0x027f    IPV6oE program - Arad_parserr_info_eth
 * 0x0300 - 0x07ff    Unused
 */
static ARAD_PARSERR_INFO
  Arad_parserr_info[] =
{
  /* address                                      macro_sel                   next_address                          PFC                              PLC */
  {ARAD_PARSER_PROG_RAW_ADDR_START              , ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                  , ARAD_PARSER_PFC_RAW_AND_FTMH   , ARAD_PARSER_PLC_RAW}, /* RAW */
  {ARAD_PARSER_PROG_ETH_ADDR_START              , ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_E_BASE               , ARAD_PARSER_DONT_CARE          , ARAD_PARSER_DONT_CARE}, /* ETH */
  {ARAD_PARSER_PROG_TM_ADDR_START               , ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_TM_BASE              , ARAD_PARSER_DONT_CARE          , ARAD_PARSER_DONT_CARE}, /* TM */ 
  {ARAD_PARSER_PROG_FTMH_ADDR_START             , ARAD_PARSER_MACRO_SEL_3   , ARAD_PARSER_PROG_END                  , ARAD_PARSER_PFC_RAW_AND_FTMH   , ARAD_PARSER_PLC_FTMH}, /* FTMH  */
  {ARAD_PARSER_PROG_RAW_MPLS_ADDR_START         , ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_RAW_MPLS_BASE        , ARAD_PARSER_DONT_CARE          , ARAD_PARSER_DONT_CARE}, /* MPLS  */  
};

static ARAD_PARSERR_INFO
  Arad_parserr_info_tm[] =
{
  /* TM, check for destination info extension { */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0002, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_IS                   , ARAD_PARSER_PLC_TM_IS}, /* Ingress shaping */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0003, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_IS                   , ARAD_PARSER_PLC_TM_IS}, /* Ingress shaping */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0004, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_IS                   , ARAD_PARSER_PLC_TM_IS}, /* Ingress shaping */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0006, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_OUT_LIF              , ARAD_PARSER_PLC_TM_OUT_LIF}, /* OutLIF */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0007, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_OUT_LIF              , ARAD_PARSER_PLC_TM_OUT_LIF}, /* OutLIF */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0008, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_OUT_LIF              , ARAD_PARSER_PLC_TM_OUT_LIF}, /* OutLIF */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000a, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_MC_FLOW              , ARAD_PARSER_PLC_TM_MC_FLOW}, /* MC-Flow-ID */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000b, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_MC_FLOW              , ARAD_PARSER_PLC_TM_MC_FLOW}, /* MC-Flow-ID */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000c, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_MC_FLOW              , ARAD_PARSER_PLC_TM_MC_FLOW}, /* MC-Flow-ID */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0012, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0013, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001A, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001B, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001C, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001D, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001E, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001F, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002A, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002B, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002C, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002D, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002E, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002F, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0032, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0033, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003A, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003B, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003C, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003D, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003E, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003F, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  /* } TM, check for destination info extension */
};
static ARAD_PARSERR_INFO
  Jericho_parserr_info_tm[] =
{
  /* TM, check for destination info extension { */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0002, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_OUT_LIF              , ARAD_PARSER_PLC_TM_OUT_LIF}, /* OutLIF */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0003, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_OUT_LIF              , ARAD_PARSER_PLC_TM_OUT_LIF}, /* OutLIF */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0004, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_MC_FLOW              , ARAD_PARSER_PLC_TM_MC_FLOW}, /* MC-Flow-ID */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0005, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_MC_FLOW              , ARAD_PARSER_PLC_TM_MC_FLOW}, /* MC-Flow-ID */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0006, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_IS                   , ARAD_PARSER_PLC_TM_IS}, /* Ingress shaping */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0007, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM_IS                   , ARAD_PARSER_PLC_TM_IS}, /* Ingress shaping */
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0008, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0012, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0013, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001A, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001B, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001C, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001D, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001E, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x001F, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002A, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002B, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002C, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002D, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002E, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x002F, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0032, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0033, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003A, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003B, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003C, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003D, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003E, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  {ARAD_PARSER_PROG_TM_BASE_START       + 0x003F, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_TM                      , ARAD_PARSER_PLC_TM},
  /* } TM, check for destination info extension */
};
static ARAD_PARSERR_INFO
  Arad_parserr_info_eth[] =
  {
    /* ETH { */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP}, /* Ethernet */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x000c, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x000d, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x000e, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x000f, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0010, ARAD_PARSER_MACRO_SEL_ADD_TPID , ARAD_PARSER_PROG_ADD_TPID_BASE , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0012, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0013, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0014, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0015, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0016, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0017, ARAD_PARSER_MACRO_SEL_EoE , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x001c, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP}, /* MPLS couplingoEth base - currently not supported */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x001d, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x001e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x001f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0020, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_TRILLoE_BASE , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP}, /* TrilloE base */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0021, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_TRILLoE_BASE , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0022, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_TRILLoE_BASE , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0023, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_TRILLoE_BASE , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0024, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_ETH       , ARAD_PARSER_PLC_PP}, /* EoE */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0025, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_ETH       , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0026, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_ETH       , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0027, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_IP_MiM_BASE  , ARAD_PARSER_PFC_ETH_ETH       , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START         /*48*/+ 0x0030, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FCoE1_BASE     , ARAD_PARSER_PFC_ETH  , ARAD_PARSER_PLC_PP},/*FCoE1 base*/
    {ARAD_PARSER_PROG_E_BASE_START         /*49*/+ 0x0031, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FCoE1_BASE     , ARAD_PARSER_PFC_ETH  , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START         /*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FCoE1_BASE     , ARAD_PARSER_PFC_ETH  , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START         /*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FCoE1_BASE     , ARAD_PARSER_PFC_ETH  , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0034, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPV4oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP}, /* IPV4oE base */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0035, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPV4oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0036, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPV4oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0037, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPV4oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0038, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPV6oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP}, /* IPV6oE base */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x0039, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPV6oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x003a, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPV6oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x003b, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPV6oE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x003c, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP}, /* MPLSoE base */
    {ARAD_PARSER_PROG_E_BASE_START        + 0x003d, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x003e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_E_BASE_START        + 0x003f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},

    /* ETH } */
    /**/
    /*  */
    /* IPv4oE { */                         /*  */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0004, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0005, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0012, ARAD_PARSER_MACRO_SEL_UDP_VX, ARAD_PARSER_PROG_UDPoIPV4oE_BASE  , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4}, /* UDPoIPV4oE, check if to do UDP or VXLAN */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0013, ARAD_PARSER_MACRO_SEL_UDP_VX, ARAD_PARSER_PROG_UDPoIPV4oE_BASE  , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4}, /* UDPoIPV4oE */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*26*/+ 0x001a, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IP4oIP4oE_BASE , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},/* IP4oIP4oE base */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*27*/+ 0x001b, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IP4oIP4oE_BASE , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x001c, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_IPV4_ETH , ARAD_PARSER_PLC_PP}, /* IPV6oIPV4oE */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x001d, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x001e, ARAD_PARSER_MACRO_SEL_MPLS , ARAD_PARSER_PROG_FIN         , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP}, /* MPLSoIPV4oE */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x001f, ARAD_PARSER_MACRO_SEL_MPLS , ARAD_PARSER_PROG_FIN         , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP}, /* MPLSoIPV4oE */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*36*/+ 0x0024, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*37*/+ 0x0025, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0032, ARAD_PARSER_MACRO_SEL_UDP_VX     , ARAD_PARSER_PROG_UDPoIPV4oE_BASE          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0033, ARAD_PARSER_MACRO_SEL_UDP_VX     , ARAD_PARSER_PROG_UDPoIPV4oE_BASE          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*58*/+ 0x003a, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IP4oIP4oE_BASE , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},/* IP4oIP4oE base */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*59*/+ 0x003b, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IP4oIP4oE_BASE , ARAD_PARSER_PFC_IPV4_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*60*/+ 0x003c, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_IPV4_ETH , ARAD_PARSER_PLC_PP},/* IP6oIP4oE */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START    /*61*/+ 0x003d, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x003e, ARAD_PARSER_MACRO_SEL_MPLS     , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP},/* MPLSoIPV4oE */
    {ARAD_PARSER_PROG_IPV4oE_BASE_START   + 0x003f, ARAD_PARSER_MACRO_SEL_MPLS     , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP},/* MPLSoIPV4oE */
    /* } IPv4oE */                         /*  */
    /*  */
    /* MPLSoE { */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0000, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP}, /* EoMPLS1oE */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0001, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP}, /* EoMPLS2oE */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0002, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP}, /* EoMPLS3oE */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0003, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP}, /* MPLS3+oE */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0007, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_GAL_GACH_BFD},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_GAL_GACH_BFD},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_GAL_GACH_BFD},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x000b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_GAL_GACH_BFD},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x000f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0012, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0013, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0017, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x001b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x001f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0023, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0027, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x002b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x002f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP}, /* MPLS3oE and more */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0032, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0033, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START    /*52*/+ 0x0034, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPoMPLS1oE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP}, /* MPLS1oE base */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START    /*53*/+ 0x0035, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPoMPLS2oE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP}, /* MPLS2oE base */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START    /*54*/+ 0x0036, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPoMPLS3oE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP}, /* MPLS3oE base */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x0037, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP}, /* MPLS3+oE*/
    {ARAD_PARSER_PROG_MPLSoE_BASE_START    /*56*/+ 0x0038, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE,  ARAD_PARSER_PFC_IPV6_MPLS1_ETH  , ARAD_PARSER_PLC_PP}, /* MPLS1oE base */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START    /*57*/+ 0x0039, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE,  ARAD_PARSER_PFC_IPV6_MPLS2_ETH  , ARAD_PARSER_PLC_PP}, /* MPLS2oE base */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START    /*58*/+ 0x003a, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE,  ARAD_PARSER_PFC_IPV6_MPLS3_ETH  , ARAD_PARSER_PLC_PP}, /* MPLS3oE base */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x003b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP}, /* MPLS3+oE */
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END                              , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoE_BASE_START   + 0x003f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE                , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    /* } MPLSoE */

    /*  */
    /* IPv6oE { */
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0002, ARAD_PARSER_MACRO_SEL_IPV6_EXT_1   , ARAD_PARSER_PROG_IPV6_EXT_1_BASE , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0003, ARAD_PARSER_MACRO_SEL_IPV6_EXT_1   , ARAD_PARSER_PROG_IPV6_EXT_1_BASE , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0012, ARAD_PARSER_MACRO_SEL_UDP     , ARAD_PARSER_PROG_UDPoIPV6oE_BASE , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0013, ARAD_PARSER_MACRO_SEL_UDP     , ARAD_PARSER_PROG_UDPoIPV6oE_BASE , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x001a, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV4_IPV6_ETH , ARAD_PARSER_PLC_PP}, /* IPV4oIPV6oE */
    {ARAD_PARSER_PROG_IPV6oE_BASE_START    /*27*/+ 0x001b, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_IPV6_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x001c, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_IPV6_ETH , ARAD_PARSER_PLC_PP}, /* IPV6oIPV6oE */
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x001d, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_IPV6_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0022, ARAD_PARSER_MACRO_SEL_IPV6_EXT_1   , ARAD_PARSER_PROG_IPV6_EXT_1_BASE  , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0023, ARAD_PARSER_MACRO_SEL_IPV6_EXT_1   , ARAD_PARSER_PROG_IPV6_EXT_1_BASE  , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0032, ARAD_PARSER_MACRO_SEL_UDP     , ARAD_PARSER_PROG_UDPoIPV6oE_BASE , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0033, ARAD_PARSER_MACRO_SEL_UDP     , ARAD_PARSER_PROG_UDPoIPV6oE_BASE , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START    /*58*/+ 0x003a, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_IPV6_ETH , ARAD_PARSER_PLC_PP}, /* IPV4oIPV6oE */
    {ARAD_PARSER_PROG_IPV6oE_BASE_START    /*59*/+ 0x003b, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_IPV6_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START    /*60*/+ 0x003c, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_IPV6_ETH , ARAD_PARSER_PLC_PP}, /* IPV6oIPV6oE */
    {ARAD_PARSER_PROG_IPV6oE_BASE_START    /*61*/+ 0x003d, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_IPV6_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6oE_BASE_START   + 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    /* } IPv6oE */
    /*  */
    /* ADDITIONAL TPID } */

    /* TRILLoE { */
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*00*/+ 0x0000, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE}, /*EoTrilloE */
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*01*/+ 0x0001, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*02*/+ 0x0002, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*03*/+ 0x0003, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*04*/+ 0x0004, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*05*/+ 0x0005, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*06*/+ 0x0006, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*07*/+ 0x0007, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*08*/+ 0x0008, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*09*/+ 0x0009, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*10*/+ 0x000a, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*11*/+ 0x000b, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*12*/+ 0x000c, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*13*/+ 0x000d, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*14*/+ 0x000e, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*15*/+ 0x000f, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*16*/+ 0x0010, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*17*/+ 0x0011, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*20*/+ 0x0014, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*21*/+ 0x0015, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*22*/+ 0x0016, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*23*/+ 0x0017, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*24*/+ 0x0018, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*25*/+ 0x0019, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*26*/+ 0x001a, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*27*/+ 0x001b, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*28*/+ 0x001c, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*29*/+ 0x001d, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*30*/+ 0x001e, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*31*/+ 0x001f, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*32*/+ 0x0020, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*33*/+ 0x0021, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*34*/+ 0x0022, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*35*/+ 0x0023, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*36*/+ 0x0024, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*37*/+ 0x0025, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*38*/+ 0x0026, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*39*/+ 0x0027, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*40*/+ 0x0028, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*41*/+ 0x0029, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*42*/+ 0x002a, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*43*/+ 0x002b, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*44*/+ 0x002c, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*45*/+ 0x002d, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*46*/+ 0x002e, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*47*/+ 0x002f, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*48*/+ 0x0030, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*49*/+ 0x0031, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*52*/+ 0x0034, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*53*/+ 0x0035, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*54*/+ 0x0036, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*55*/+ 0x0037, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*56*/+ 0x0038, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*57*/+ 0x0039, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*58*/+ 0x003a, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*59*/+ 0x003b, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*60*/+ 0x003c, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*61*/+ 0x003d, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*62*/+ 0x003e, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*63*/+ 0x003F, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*64*/+ 0x0040, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*65*/+ 0x0041, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*66*/+ 0x0042, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*67*/+ 0x0043, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*68*/+ 0x0044, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*69*/+ 0x0045, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*70*/+ 0x0046, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*71*/+ 0x0047, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*72*/+ 0x0048, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*73*/+ 0x0049, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*74*/+ 0x004A, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*75*/+ 0x004B, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*76*/+ 0x004C, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*77*/+ 0x004D, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*78*/+ 0x004E, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*79*/+ 0x004F, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*80*/+ 0x0050, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*81*/+ 0x0051, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*82*/+ 0x0052, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*83*/+ 0x0053, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*84*/+ 0x0054, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*85*/+ 0x0055, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*86*/+ 0x0056, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*87*/+ 0x0057, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*88*/+ 0x0058, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*89*/+ 0x0059, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*90*/+ 0x005A, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*91*/+ 0x005B, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*92*/+ 0x005C, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*93*/+ 0x005D, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*94*/+ 0x005E, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*95*/+ 0x005F, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*96*/+ 0x0060, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*97*/+ 0x0061, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*98*/+ 0x0062, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START   /*99*/+ 0x0063, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*100*/+ 0x0064, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*101*/+ 0x0065, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*102*/+ 0x0066, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*103*/+ 0x0067, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*104*/+ 0x0068, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*105*/+ 0x0069, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*106*/+ 0x006A, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*107*/+ 0x006B, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*108*/+ 0x006C, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*109*/+ 0x006D, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*110*/+ 0x006E, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*111*/+ 0x006F, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*112*/+ 0x0070, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*113*/+ 0x0071, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*114*/+ 0x0072, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*115*/+ 0x0073, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*116*/+ 0x0074, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*117*/+ 0x0075, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*118*/+ 0x0076, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*119*/+ 0x0077, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*120*/+ 0x0078, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*121*/+ 0x0079, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*122*/+ 0x007A, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*123*/+ 0x007B, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*124*/+ 0x007C, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*125*/+ 0x007D, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*126*/+ 0x007E, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_TRILLoE_BASE_START  /*127*/+ 0x007F, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_EoTRILoE_BASE  , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    /* } TRILLoE */                        /*  */
    /*  */

    /* IPv6oMPLS3oE { */                     /*  */
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS3oE_BASE_START/*63*/+ 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    /* } IPv6oMPLS3oE */                     /*  */

        /*  */
    /* MPLSoMPLS3oE { */
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},     /* MPLS4oE */
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5}, /* MPLS5oE */
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5}, /* MPLS6oE */
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5}, /* MPLS6oE (no BOS) */
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_GAL_GACH_BFD},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0012, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0013, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0032, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0033, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    {ARAD_PARSER_PROG_MPLSoMPLS3oE_BASE_START   + 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_MPLS_5},
    /* } MPLSoMPLS3oE */
    /* */
    /* IPoMPLS1oE { */                     /*  */
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS1oE_BASE_START/*63*/+ 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    /* } IPoMPLS1oE */                     /*  */
               /*  */
    /* IPoMPLS2oE { */                     /*  */
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS2oE_BASE_START/*63*/+ 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    /* } IPoMPLS2oE */                     /*  */
               /*  */
    /* IPoMPLS3oE { */                     /*  */
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPoMPLS3oE_BASE_START/*63*/+ 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    /* } IPoMPLS3oE */                     /*  */
               /*  */

    /* IP4oIP4oE { */                      /*  */
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP4oIP4oE_BASE_START /*63*/+ 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV4_IPV4_ETH , ARAD_PARSER_PLC_PP},
    /* } IP4oIP4oE */                      /*  */
               /*  */
    /* FCoE1 { */                          /*  */
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*00*/+ 0x0000, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE}, /* FCoE2 base */
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*01*/+ 0x0001, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*02*/+ 0x0002, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*03*/+ 0x0003, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*04*/+ 0x0004, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*05*/+ 0x0005, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*06*/+ 0x0006, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*07*/+ 0x0007, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*08*/+ 0x0008, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*09*/+ 0x0009, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*10*/+ 0x000a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*11*/+ 0x000b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*12*/+ 0x000c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*13*/+ 0x000d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*14*/+ 0x000e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*15*/+ 0x000f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*16*/+ 0x0010, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*17*/+ 0x0011, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*20*/+ 0x0014, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*21*/+ 0x0015, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*22*/+ 0x0016, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*23*/+ 0x0017, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*24*/+ 0x0018, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*25*/+ 0x0019, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*26*/+ 0x001a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*27*/+ 0x001b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*28*/+ 0x001c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*29*/+ 0x001d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*30*/+ 0x001e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*31*/+ 0x001f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*32*/+ 0x0020, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*33*/+ 0x0021, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*34*/+ 0x0022, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*35*/+ 0x0023, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*36*/+ 0x0024, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*37*/+ 0x0025, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*38*/+ 0x0026, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*39*/+ 0x0027, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*40*/+ 0x0028, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*41*/+ 0x0029, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*42*/+ 0x002a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*43*/+ 0x002b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*44*/+ 0x002c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*45*/+ 0x002d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*46*/+ 0x002e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*47*/+ 0x002f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*48*/+ 0x0030, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*49*/+ 0x0031, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*52*/+ 0x0034, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*53*/+ 0x0035, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*54*/+ 0x0036, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*55*/+ 0x0037, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*56*/+ 0x0038, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*57*/+ 0x0039, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*58*/+ 0x003a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*59*/+ 0x003b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*60*/+ 0x003c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*61*/+ 0x003d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*62*/+ 0x003e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    {ARAD_PARSER_PROG_FCoE1_BASE_START     /*63*/+ 0x003f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE2_BASE     , ARAD_PARSER_DONT_CARE                      , ARAD_PARSER_DONT_CARE},
    /* } FCoE1 */                          /*  */
               /*  */
    /* FCoE2 { */                          /*  */
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*00*/+ 0x0000, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE}, /* FCoE */
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*01*/+ 0x0001, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*02*/+ 0x0002, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*03*/+ 0x0003, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*04*/+ 0x0004, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*05*/+ 0x0005, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*06*/+ 0x0006, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*07*/+ 0x0007, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*08*/+ 0x0008, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*09*/+ 0x0009, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*10*/+ 0x000a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*11*/+ 0x000b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*12*/+ 0x000c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*13*/+ 0x000d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*14*/+ 0x000e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*15*/+ 0x000f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*16*/+ 0x0010, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*17*/+ 0x0011, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*20*/+ 0x0014, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*21*/+ 0x0015, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*22*/+ 0x0016, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*23*/+ 0x0017, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*24*/+ 0x0018, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*25*/+ 0x0019, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*26*/+ 0x001a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*27*/+ 0x001b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*28*/+ 0x001c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*29*/+ 0x001d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*30*/+ 0x001e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FCoE3_BASE     , ARAD_PARSER_DONT_CARE            , ARAD_PARSER_DONT_CARE}, /* FCoE3 base */
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*31*/+ 0x001f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*32*/+ 0x0020, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*33*/+ 0x0021, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*34*/+ 0x0022, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*35*/+ 0x0023, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*36*/+ 0x0024, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*37*/+ 0x0025, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*38*/+ 0x0026, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*39*/+ 0x0027, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*40*/+ 0x0028, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*41*/+ 0x0029, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*42*/+ 0x002a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*43*/+ 0x002b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*44*/+ 0x002c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*45*/+ 0x002d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*46*/+ 0x002e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*47*/+ 0x002f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*48*/+ 0x0030, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*49*/+ 0x0031, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*52*/+ 0x0034, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*53*/+ 0x0035, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*54*/+ 0x0036, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*55*/+ 0x0037, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*56*/+ 0x0038, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*57*/+ 0x0039, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*58*/+ 0x003a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*59*/+ 0x003b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*60*/+ 0x003c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE_VFT}, /* FCoE3 with VFT */
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*61*/+ 0x003d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*62*/+ 0x003e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE2_BASE_START     /*63*/+ 0x003f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_STD_ETH           , ARAD_PARSER_PLC_FCOE},
    /* } FCoE2 */                          /*  */
               /*  */
    /* FCoE3 { */                          /*  */
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*00*/+ 0x0000, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE}, /* Encap FCoE */
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*01*/+ 0x0001, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*02*/+ 0x0002, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*03*/+ 0x0003, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*04*/+ 0x0004, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*05*/+ 0x0005, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*06*/+ 0x0006, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*07*/+ 0x0007, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*08*/+ 0x0008, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*09*/+ 0x0009, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*10*/+ 0x000a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*11*/+ 0x000b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*12*/+ 0x000c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*13*/+ 0x000d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*14*/+ 0x000e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*15*/+ 0x000f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*16*/+ 0x0010, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*17*/+ 0x0011, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*20*/+ 0x0014, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*21*/+ 0x0015, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*22*/+ 0x0016, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*23*/+ 0x0017, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*24*/+ 0x0018, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*25*/+ 0x0019, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*26*/+ 0x001a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*27*/+ 0x001b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*28*/+ 0x001c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*29*/+ 0x001d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*30*/+ 0x001e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*31*/+ 0x001f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*32*/+ 0x0020, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*33*/+ 0x0021, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*34*/+ 0x0022, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*35*/+ 0x0023, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*36*/+ 0x0024, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*37*/+ 0x0025, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*38*/+ 0x0026, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*39*/+ 0x0027, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*40*/+ 0x0028, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*41*/+ 0x0029, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*42*/+ 0x002a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*43*/+ 0x002b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*44*/+ 0x002c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*45*/+ 0x002d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*46*/+ 0x002e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*47*/+ 0x002f, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*48*/+ 0x0030, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*49*/+ 0x0031, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*52*/+ 0x0034, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*53*/+ 0x0035, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*54*/+ 0x0036, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*55*/+ 0x0037, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*56*/+ 0x0038, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*57*/+ 0x0039, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*58*/+ 0x003a, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*59*/+ 0x003b, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*60*/+ 0x003c, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE_VFT}, /* Encap FCoE with VFT*/
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*61*/+ 0x003d, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*62*/+ 0x003e, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    {ARAD_PARSER_PROG_FCoE3_BASE_START     /*63*/+ 0x003F, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_FC_ENCAP_ETH     , ARAD_PARSER_PLC_FCOE},
    /* } FCoE3 */                          /* although the prsr.txt file initializes 128 rows, the next 64 row are not relevant  */

    /*  */
    /* EoTRILoE { */                       /*  */
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP}, /* EoTrilloE */
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*18*/+ 0x0012, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*19*/+ 0x0013, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*50*/+ 0x0032, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*51*/+ 0x0033, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_EoTRILoE_BASE_START  /*63*/+ 0x003F, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_ETH_TRILL_ETH , ARAD_PARSER_PLC_PP},/* EoTrilloE double TAG*/
    /* } EoTRILoE */                       /*  */
    /* UDP parsing{ */
    /* vxlan, continue to parse Eth After UDP, entries (2)xx,4(8),(1)0 ==> 0x10,0x30*/
    /* regular */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0000, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0001, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0002, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0003, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0004, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0005, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0006, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0007, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0008, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0009, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x000a, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x000b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x000c, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x000d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x000e, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x000f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    /* vxlan, continue to parse Eth After UDP, entries (2)xx,4(8),(1)0 ==> 0x10,0x30*/
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0010, ARAD_PARSER_MACRO_SEL_ETH/*M3*/ , ARAD_PARSER_PROG_IP_VXLAN_BASE , ARAD_PARSER_PFC_ETH_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* VxlanoUDP */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0011, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0012, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0013, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0014, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0015, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0016, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0017, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0018, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0019, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x001a, ARAD_PARSER_MACRO_SEL_IPV4      , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x001b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x001c, ARAD_PARSER_MACRO_SEL_IPV6      , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x001d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x001e, ARAD_PARSER_MACRO_SEL_MPLS      , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x001f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0020, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0021, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0022, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0023, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0024, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0025, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0026, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0027, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0028, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0029, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x002a, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x002b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x002c, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x002d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x002e, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x002f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    /* vxlan, continue to parse Eth After UDP, entries (2)xx,4(8),(1)0 ==> 0x10,0x30*/
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0030, ARAD_PARSER_MACRO_SEL_ETH/*M3*/ , ARAD_PARSER_PROG_IP_VXLAN_BASE , ARAD_PARSER_PFC_ETH_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* VxlanoUDP */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0031, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0032, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0033, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0034, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0035, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0036, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0037, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0038, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x0039, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x003a, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x003b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x003c, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x003d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x003e, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START     + 0x003f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_IPV4_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */

    /* vxlan, continue to parse Eth After UDP, entries (2)xx,4(8),(1)0 ==> 0x10,0x30*/
    /* Eth parsing after UDP not supported for IPv6 */
    /* regular */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0000, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0001, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0002, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0003, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0004, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0005, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0006, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0007, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0008, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0009, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x000a, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x000b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x000c, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x000d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x000e, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x000f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    /* vxlan, continue to parse Eth After UDP, entries (2)xx,4(8),(1)0 ==> 0x10,0x30*/
    /* Eth parsing after UDP not supported for IPv6 */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0010, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* VxlanoUDP */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0011, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0012, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0013, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0014, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0015, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0016, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0017, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0018, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0019, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x001a, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x001b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x001c, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x001d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x001e, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x001f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0020, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0021, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0022, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0023, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0024, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0025, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0026, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0027, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0028, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0029, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x002a, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x002b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x002c, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x002d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x002e, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x002f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    /* vxlan, continue to parse Eth After UDP, entries (2)xx,4(8),(1)0 ==> 0x10,0x30*/
    /* Eth parsing after UDP not supported for IPv6 */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0030, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* VxlanoUDP */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0031, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0032, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0033, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0034, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0035, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0036, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0037, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0038, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x0039, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x003a, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x003b, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x003c, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x003d, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x003e, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */
    {ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START     + 0x003f, ARAD_PARSER_MACRO_SEL_UDP/*M1*/ , ARAD_PARSER_PROG_END , ARAD_PARSER_PFC_IPV6_ETH           , ARAD_PARSER_PLC_PP_L4}, /* regular UD */

    /* } UDP parsing */


    /* EtheroVxlanoUDP parsing{ */
    /* vxlan, continue to parse Eth After UDP */
    /* regular */
    /* ipv4oEthoVxlanoUDP*/

    /* NONipv4oEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0000, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0001, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0002, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0003, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0004, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0005, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0006, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0007, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0008, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0009, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x000A, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x000B, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x000C, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x000D, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x000E, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x000F, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0010, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0011, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0012, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0013, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0014, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0015, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0016, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0017, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0018, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0019, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x001A, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x001B, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x001C, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x001D, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x001E, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x001F, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0020, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0021, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0022, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0023, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0024, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0025, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0026, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0027, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0028, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0029, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x002A, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x002B, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x002C, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x002D, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x002E, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x002F, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0030, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0031, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0032, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0033, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    /* {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0034, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP},  IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0034, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN  , ARAD_PARSER_PFC_ETH_IPV4_ETH              , ARAD_PARSER_PLC_VXLAN}, /* IPV4oE base */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0035, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0036, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0037, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0038, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x0039, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x003A, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x003B, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x003C, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x003D, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x003E, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    {ARAD_PARSER_PROG_EoUDPoIPV4oE_BASE_START + 0x003F, ARAD_PARSER_DONT_CARE , ARAD_PARSER_PROG_FIN , ARAD_PARSER_PFC_ERROR           , ARAD_PARSER_PLC_PP}, /* non-IPoEthoVxlanoUDP */
    /* } EtheroVxlanoUDP parsing */


    /* IPv6oE Extension Header { */
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0000, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0001, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0002, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0003, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0004, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0005, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0006, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0007, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0008, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0009, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x000a, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x000b, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x000c, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x000d, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x000e, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x000f, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0010, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0011, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0012, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0013, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0014, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0015, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0016, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0017, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0018, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0019, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x001a, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x001b, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x001c, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x001d, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x001e, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x001f, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0020, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0021, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0022, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0023, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0024, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0025, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0026, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0027, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0028, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0029, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x002a, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x002b, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x002c, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x002d, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x002e, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x002f, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0030, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0031, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0032, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0033, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0034, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0035, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0036, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0037, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0038, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x0039, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x003a, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x003b, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x003c, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x003d, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x003e, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_1_BASE_START   + 0x003f, ARAD_PARSER_MACRO_SEL_IPV6_EXT_2   , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    /* } IPv6oE Extension Header */

    /* UDPoIPv6oE Extension Header { */
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0000, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0001, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0002, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0003, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0004, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0005, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0006, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0007, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0008, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0009, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x000a, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x000b, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x000c, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x000d, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x000e, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x000f, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0010, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0011, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0012, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0013, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0014, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0015, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0016, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0017, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0018, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0019, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x001a, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x001b, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x001c, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x001d, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x001e, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x001f, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0020, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0021, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0022, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0023, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0024, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0025, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0026, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0027, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0028, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0029, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x002a, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x002b, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x002c, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x002d, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x002e, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x002f, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0030, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0031, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0032, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0033, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0034, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0035, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0036, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0037, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0038, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x0039, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x003a, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x003b, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x003c, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x003d, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x003e, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPV6_EXT_2_UDP_BASE_START   + 0x003f, ARAD_PARSER_MACRO_SEL_UDP   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP_L4},
    /* } UDPoIPv6oE Extension Header */

    /* IPv4oIPv6oE Extension Header { */
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0000, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0001, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0002, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0003, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0004, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0005, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0006, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0007, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0008, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0009, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x000a, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x000b, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x000c, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x000d, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x000e, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x000f, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0010, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0011, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0012, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0013, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0014, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0015, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0016, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0017, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0018, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0019, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x001a, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x001b, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x001c, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x001d, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x001e, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x001f, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0020, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0021, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0022, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0023, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0024, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0025, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0026, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0027, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0028, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0029, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x002a, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x002b, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x002c, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x002d, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x002e, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x002f, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0030, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0031, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0032, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0033, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0034, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0035, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0036, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0037, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0038, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x0039, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x003a, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x003b, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x003c, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x003d, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x003e, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV4_BASE_START   + 0x003f, ARAD_PARSER_MACRO_SEL_IPV4   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    /* } IPv4oIPv6oE Extension Header */

    /* IPv6oIPv6oE Extension Header { */
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0000, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0001, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0002, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0003, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0004, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0005, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0006, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0007, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0008, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0009, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x000a, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x000b, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x000c, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x000d, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x000e, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x000f, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0010, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0011, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0012, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0013, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0014, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0015, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0016, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0017, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0018, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0019, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x001a, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x001b, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x001c, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x001d, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x001e, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x001f, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0020, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0021, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0022, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0023, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0024, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0025, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0026, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0027, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0028, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0029, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x002a, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x002b, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x002c, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x002d, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x002e, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x002f, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0030, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0031, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0032, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0033, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0034, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0035, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0036, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0037, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0038, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x0039, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x003a, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x003b, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x003c, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x003d, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x003e, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPV6_EXT_2_IPV6_BASE_START   + 0x003f, ARAD_PARSER_MACRO_SEL_IPV6   , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_IPV6_ETH      , ARAD_PARSER_PLC_PP},
    /* } IPv6oIPv6oE Extension Header */


    /* ADDITIONAL TPID { */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP}, /* Ethernet */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x000e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_DONT_CARE         , ARAD_PARSER_DONT_CARE}, /* MPLS couplingoEth base - currently not supported */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0010, ARAD_PARSER_MACRO_SEL_0   , ARAD_PARSER_PROG_TRILLoE_BASE , ARAD_PARSER_DONT_CARE         , ARAD_PARSER_DONT_CARE}, /* TrilloE base */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0012, ARAD_PARSER_MACRO_SEL_ETH , ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_ETH       , ARAD_PARSER_PLC_PP}, /* EoE */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0013, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0018, ARAD_PARSER_MACRO_SEL_2   , ARAD_PARSER_PROG_FCoE1_BASE   , ARAD_PARSER_DONT_CARE         , ARAD_PARSER_DONT_CARE},/*FCoE1 base*/
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x001a, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_IPV4oE_BASE  , ARAD_PARSER_DONT_CARE         , ARAD_PARSER_DONT_CARE}, /* IPV4oE base */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x001c, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_IPV6oE_BASE  , ARAD_PARSER_DONT_CARE         , ARAD_PARSER_DONT_CARE}, /* IPV6oE base */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x001e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE  , ARAD_PARSER_DONT_CARE         , ARAD_PARSER_DONT_CARE}, /* MPLSoE base */
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0032, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0033, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_ADD_TPID_BASE_START        + 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH           , ARAD_PARSER_PLC_PP},

    /*  */
    /* Raw MPLS { */
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0000, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0001, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0002, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0003, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0004, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0005, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0006, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0007, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0008, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0009, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x000a, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x000b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x000c, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x000d, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x000e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x000f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0010, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0011, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0012, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0013, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0014, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0015, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0016, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0017, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0018, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0019, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x001a, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x001b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x001c, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x001d, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x001e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x001f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0020, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0021, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0022, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0023, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0024, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0025, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0026, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0027, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0028, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0029, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x002a, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x002b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x002c, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x002d, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x002e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x002f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0030, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0031, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0032, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0033, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0034, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0035, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0036, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0037, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0038, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x0039, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x003a, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x003b, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x003c, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x003d, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x003e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_RAW_MPLS_BASE_START  + 0x003f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_MPLSoE_BASE, ARAD_PARSER_PFC_MPLS3_ETH     , ARAD_PARSER_PLC_PP},
    /* } Raw MPLS */

    /* IPoETHoVxLAN {*/
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},  /* Ethernet */
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0012, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0013, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4}, /* MPLS couplingoEth base - currently not supported */
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4}, /* TrilloE base */
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4}, /* EoE */
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START         /*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END   , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},/*FCoE1 base*/
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START         /*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END   , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START         /*50*/+ 0x0032, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END   , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START         /*51*/+ 0x0033, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END   , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0034, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4}, /* IPV4oE base */
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0035, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0036, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0037, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0038, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4}, /* IPV6oE base */
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x0039, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x003a, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x003b, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x003c, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4}, /* MPLSoE base */
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x003d, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x003e, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IP_VXLAN_BASE_START        + 0x003f, ARAD_PARSER_MACRO_SEL_MPLS, ARAD_PARSER_PROG_FIN          , ARAD_PARSER_PFC_ETH_IPV4_ETH  , ARAD_PARSER_PLC_PP_L4},

    /* IPoMiM { */
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0000, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0001, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0002, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0003, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0004, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0005, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0006, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0007, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0008, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0009, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x000a, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x000b, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x000c, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x000d, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x000e, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x000f, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0010, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0011, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0012, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0013, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0014, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0015, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0016, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0017, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0018, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0019, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x001a, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x001b, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x001c, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x001d, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x001e, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x001f, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0020, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0021, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0022, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0023, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0024, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0025, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0026, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0027, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0028, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0029, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x002a, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x002b, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x002c, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x002d, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x002e, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x002f, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0030, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0031, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0032, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0033, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0034, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0035, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0036, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0037, ARAD_PARSER_MACRO_SEL_IPV4, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0038, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x0039, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x003a, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x003b, ARAD_PARSER_MACRO_SEL_IPV6, ARAD_PARSER_PROG_FIN, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x003c, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x003d, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x003e, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IP_MiM_BASE_START  + 0x003f, ARAD_PARSER_MACRO_SEL_ETH, ARAD_PARSER_PROG_END, ARAD_PARSER_PFC_ETH_ETH     , ARAD_PARSER_PLC_PP},
    /* } IPoMiM */

    /* IPv6oMPLS1oE { */                     /*  */
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS1oE_BASE_START/*63*/+ 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS1_ETH     , ARAD_PARSER_PLC_PP},
    /* } IPv6oMPLS1oE */                     /*  */

    /* IPv6oMPLS2oE { */                     /*  */
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*00*/+ 0x0000, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*01*/+ 0x0001, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*02*/+ 0x0002, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*03*/+ 0x0003, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*04*/+ 0x0004, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*05*/+ 0x0005, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*06*/+ 0x0006, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*07*/+ 0x0007, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*08*/+ 0x0008, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*09*/+ 0x0009, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*10*/+ 0x000a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*11*/+ 0x000b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*12*/+ 0x000c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*13*/+ 0x000d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*14*/+ 0x000e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*15*/+ 0x000f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*16*/+ 0x0010, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*17*/+ 0x0011, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*18*/+ 0x0012, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*19*/+ 0x0013, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*20*/+ 0x0014, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*21*/+ 0x0015, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*22*/+ 0x0016, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*23*/+ 0x0017, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*24*/+ 0x0018, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*25*/+ 0x0019, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*26*/+ 0x001a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*27*/+ 0x001b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*28*/+ 0x001c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*29*/+ 0x001d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*30*/+ 0x001e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*31*/+ 0x001f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*32*/+ 0x0020, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*33*/+ 0x0021, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*34*/+ 0x0022, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*35*/+ 0x0023, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*36*/+ 0x0024, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*37*/+ 0x0025, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*38*/+ 0x0026, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*39*/+ 0x0027, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*40*/+ 0x0028, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*41*/+ 0x0029, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*42*/+ 0x002a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*43*/+ 0x002b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*44*/+ 0x002c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*45*/+ 0x002d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*46*/+ 0x002e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*47*/+ 0x002f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*48*/+ 0x0030, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*49*/+ 0x0031, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*50*/+ 0x0032, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*51*/+ 0x0033, ARAD_PARSER_MACRO_SEL_1   , ARAD_PARSER_PROG_FIN            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP_L4},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*52*/+ 0x0034, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*53*/+ 0x0035, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*54*/+ 0x0036, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*55*/+ 0x0037, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*56*/+ 0x0038, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*57*/+ 0x0039, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*58*/+ 0x003a, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*59*/+ 0x003b, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*60*/+ 0x003c, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*61*/+ 0x003d, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*62*/+ 0x003e, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    {ARAD_PARSER_PROG_IPv6oMPLS2oE_BASE_START/*63*/+ 0x003f, ARAD_PARSER_DONT_CARE     , ARAD_PARSER_PROG_END            , ARAD_PARSER_PFC_IPV6_MPLS2_ETH     , ARAD_PARSER_PLC_PP},
    /* } IPv6oMPLS2oE */                     /*  */
  };

static dpp_parser_plc_info_t dpp_parser_plc_info_tm[] =
{
    { DPP_PLC_TM,           ARAD_PARSER_PLC_TM,          FALSE, "TM"},
    {-1}
};

static dpp_parser_plc_info_t dpp_parser_plc_info_tm_is[] =
{
    { DPP_PLC_TM_IS,          ARAD_PARSER_PLC_TM_IS,       FALSE, "TM_IS"},
    {-1}
};

static dpp_parser_plc_info_t dpp_parser_plc_info_tm_mc_flow[] =
{
    { DPP_PLC_TM_MC_FLOW,     ARAD_PARSER_PLC_TM_MC_FLOW,     FALSE, "TM_MC_FLOW"},
    {-1}
};

static dpp_parser_plc_info_t dpp_parser_plc_info_tm_out_lif[] =
{
    { DPP_PLC_TM_OUT_LIF,     ARAD_PARSER_PLC_TM_OUT_LIF,     FALSE, "TM_OUT_LIF"},
    {-1}
};

static dpp_parser_plc_info_t dpp_parser_plc_info_raw_ftmh[] =
{
    { DPP_PLC_RAW,            ARAD_PARSER_PLC_RAW,            FALSE, "RAW"},
    { DPP_PLC_FTMH,           ARAD_PARSER_PLC_FTMH,           FALSE, "FTMH"},
    {-1}
};

static dpp_parser_plc_info_t dpp_parser_plc_info_pp[] =
{
/*  SW PLC                    HW PLC                          LB Set Name */
    { DPP_PLC_VXLAN,          ARAD_PARSER_PLC_VXLAN,          FALSE, "VXLAN"},
    { DPP_PLC_BFD_SINGLE_HOP, ARAD_PARSER_PLC_BFD_SINGLE_HOP, FALSE, "BFD_SINGLE_HOP"},
    { DPP_PLC_IP_UDP_GTP1,    ARAD_PARSER_PLC_IP_UDP_GTP1,    FALSE, "IP_UDP_GTP1"},
    { DPP_PLC_IP_UDP_GTP2,    ARAD_PARSER_PLC_IP_UDP_GTP2,    FALSE, "IP_UDP_GTP2"},
    { DPP_PLC_PP_L4,          ARAD_PARSER_PLC_PP_L4,          TRUE,  "PP_L4"},
    { DPP_PLC_PP,             ARAD_PARSER_PLC_PP,             FALSE, "PP"},
    {-1}
};

static dpp_parser_plc_info_t dpp_parser_plc_info_mpls[] =
{
/*  SW PLC                    HW PLC                          LB Set Name */
    { DPP_PLC_MPLS_5,         ARAD_PARSER_PLC_MPLS_5,         TRUE,  "MPLS_5"},
    { DPP_PLC_GAL_GACH_BFD,   ARAD_PARSER_PLC_GAL_GACH_BFD,   TRUE,  "GAL_GACH_BFD"},
    { DPP_PLC_PP_L4,          ARAD_PARSER_PLC_PP_L4,          TRUE,  "PP_L4"},
    { DPP_PLC_PP,             ARAD_PARSER_PLC_PP,             FALSE, "PP"},
    {-1}
};

static dpp_parser_plc_info_t dpp_parser_plc_info_fc[] =
{
/*  SW PLC                    HW PLC                          LB Set Name */
    { DPP_PLC_FC,             ARAD_PARSER_PLC_FCOE,           FALSE, "FCOE"},
    { DPP_PLC_FC_VFT,         ARAD_PARSER_PLC_FCOE_VFT,       TRUE,  "FCOE_VFT"},
    {-1}
};

dpp_parser_plc_profile_t dpp_parser_plc_profiles[] =
{
    { DPP_PLC_PROFILE_NA,           "None",             NULL},
    { DPP_PLC_PROFILE_TM,           "TM",               dpp_parser_plc_info_tm},
    { DPP_PLC_PROFILE_TM_IS,        "TM_IS",            dpp_parser_plc_info_tm_is},
    { DPP_PLC_PROFILE_TM_MC_FLOW,   "TM_MC_Flow",       dpp_parser_plc_info_tm_mc_flow},
    { DPP_PLC_PROFILE_TM_OUT_LIF,   "TM_Out_Lif",       dpp_parser_plc_info_tm_out_lif},
    { DPP_PLC_PROFILE_RAW_FTMH,     "RAW&FTMH",         dpp_parser_plc_info_raw_ftmh},
    { DPP_PLC_PROFILE_PP,           "Base PP",          dpp_parser_plc_info_pp},
    { DPP_PLC_PROFILE_MPLS,         "MPLS",             dpp_parser_plc_info_mpls},
    { DPP_PLC_PROFILE_FC,           "Fiber Channel",    dpp_parser_plc_info_fc},
    {-1}
};

dpp_parser_pfc_info_t dpp_parser_pfc_info[] =
{   /* SW PFC,                Name              HW PFC,                         IS_PROTO    L4_LOC, Sec,hdrtype, VTT,                            PMF                             PLC Profile*/
    { DPP_PFC_ETH,            "ETH",            ARAD_PARSER_PFC_ETH,            IS_ETH,           4, 0, 0x00001, ARAD_PARSER_PFC_ETH,            ARAD_PARSER_PFC_ETH,            DPP_PLC_PROFILE_PP},
    { DPP_PFC_ETH_ETH,        "ETHoETH",        ARAD_PARSER_PFC_ETH_ETH,        IS_ETH,           4, 0, 0x00011, ARAD_PARSER_PFC_ETH_ETH,        ARAD_PARSER_PFC_ETH_ETH,        DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV4_ETH,       "IPv4oETH",       ARAD_PARSER_PFC_IPV4_ETH,       IS_ETH | IS_IPV4, 3, 0, 0x00021, ARAD_PARSER_PFC_IPV4_ETH,       ARAD_PARSER_PFC_IPV4_ETH,       DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV6_ETH,       "IPv6oETH",       ARAD_PARSER_PFC_IPV6_ETH,       IS_ETH | IS_IPV6, 3, 0, 0x00031, ARAD_PARSER_PFC_IPV6_ETH,       ARAD_PARSER_PFC_IPV6_ETH,       DPP_PLC_PROFILE_PP},
    { DPP_PFC_MPLS1_ETH,      "MPLSx1oETH",     ARAD_PARSER_PFC_MPLS1_ETH,      IS_ETH | IS_MPLS, 4, 0, 0x00041, ARAD_PARSER_PFC_MPLS1_ETH,      ARAD_PARSER_PFC_MPLS1_ETH,      DPP_PLC_PROFILE_MPLS},
    { DPP_PFC_MPLS2_ETH,      "MPLSx2oETH",     ARAD_PARSER_PFC_MPLS2_ETH,      IS_ETH | IS_MPLS, 4, 0, 0x00441, ARAD_PARSER_PFC_MPLS2_ETH,      ARAD_PARSER_PFC_MPLS2_ETH,      DPP_PLC_PROFILE_MPLS},
    { DPP_PFC_MPLS3_ETH,      "MPLSx3oETH",     ARAD_PARSER_PFC_MPLS3_ETH,      IS_ETH | IS_MPLS, 4, 0, 0x04441, ARAD_PARSER_PFC_MPLS3_ETH,      ARAD_PARSER_PFC_MPLS3_ETH,      DPP_PLC_PROFILE_MPLS},
    { DPP_PFC_FC_STD_ETH,     "FCoETH",         ARAD_PARSER_PFC_FC_STD_ETH,     IS_ETH,           4, 0, 0x00061, ARAD_PARSER_PFC_FC_STD_ETH,     ARAD_PARSER_PFC_FC_STD_ETH,     DPP_PLC_PROFILE_FC},
    { DPP_PFC_FC_ENCAP_ETH,   "FCwithENCAPoETH",ARAD_PARSER_PFC_FC_ENCAP_ETH,   IS_ETH,           4, 0, 0x00051, ARAD_PARSER_PFC_FC_ENCAP_ETH,   ARAD_PARSER_PFC_FC_ENCAP_ETH,   DPP_PLC_PROFILE_FC},
    { DPP_PFC_ETH_IPV4_ETH,   "ETHoIPv4oETH",   ARAD_PARSER_PFC_ETH_IPV4_ETH,   IS_ETH | IS_IPV4, 4, 0, 0x00121, ARAD_PARSER_PFC_ETH_IPV4_ETH,   ARAD_PARSER_PFC_ETH_IPV4_ETH,   DPP_PLC_PROFILE_PP},
    { DPP_PFC_ETH_TRILL_ETH,  "ETHoTRILLoETH",  ARAD_PARSER_PFC_ETH_TRILL_ETH,  IS_ETH | IS_TRILL,4, 0, 0x00151, ARAD_PARSER_PFC_ETH_TRILL_ETH,  ARAD_PARSER_PFC_ETH_TRILL_ETH,  DPP_PLC_PROFILE_PP},
    { DPP_PFC_ETH_MPLS1_ETH,  "ETHoMPLSx1oETH", ARAD_PARSER_PFC_ETH_MPLS1_ETH,  IS_ETH | IS_MPLS, 4, 1, 0x00141, ARAD_PARSER_PFC_ETH_MPLS1_ETH,  ARAD_PARSER_PFC_ETH_MPLS1_ETH,  DPP_PLC_PROFILE_NA},
    { DPP_PFC_ETH_MPLS2_ETH,  "ETHoMPLSx2oETH", ARAD_PARSER_PFC_ETH_MPLS2_ETH,  IS_ETH | IS_MPLS, 4, 1, 0x01441, ARAD_PARSER_PFC_ETH_MPLS2_ETH,  ARAD_PARSER_PFC_ETH_MPLS2_ETH,  DPP_PLC_PROFILE_NA},
    { DPP_PFC_ETH_MPLS3_ETH,  "ETHoMPLSx3oETH", ARAD_PARSER_PFC_ETH_MPLS3_ETH,  IS_ETH | IS_MPLS, 4, 1, 0x14441, ARAD_PARSER_PFC_ETH_MPLS3_ETH,  ARAD_PARSER_PFC_ETH_MPLS3_ETH,  DPP_PLC_PROFILE_NA},
    { DPP_PFC_IPV4_IPV4_ETH,  "IPv4oIPv4oETH",  ARAD_PARSER_PFC_IPV4_IPV4_ETH,  IS_ETH | IS_IPV4, 4, 0, 0x00221, ARAD_PARSER_PFC_IPV4_IPV4_ETH,  ARAD_PARSER_PFC_IPV4_IPV4_ETH,  DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV4_IPV6_ETH,  "IPv4oIPv6oETH",  ARAD_PARSER_PFC_IPV4_IPV6_ETH,  IS_ETH | IS_IPV6, 4, 0, 0x00231, ARAD_PARSER_PFC_IPV4_IPV6_ETH,  ARAD_PARSER_PFC_IPV4_IPV6_ETH,  DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV4_MPLS1_ETH, "IPv4oMPLSx1oETH",ARAD_PARSER_PFC_IPV4_MPLS1_ETH, IS_ETH | IS_MPLS, 4, 0, 0x00241, ARAD_PARSER_PFC_IPV4_MPLS1_ETH, ARAD_PARSER_PFC_IPV4_MPLS1_ETH, DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV4_MPLS2_ETH, "IPv4oMPLSx2oETH",ARAD_PARSER_PFC_IPV4_MPLS2_ETH, IS_ETH | IS_MPLS, 4, 0, 0x02441, ARAD_PARSER_PFC_IPV4_MPLS2_ETH, ARAD_PARSER_PFC_IPV4_MPLS2_ETH, DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV4_MPLS3_ETH, "IPv4oMPLSx3oETH",ARAD_PARSER_PFC_IPV4_MPLS3_ETH, IS_ETH | IS_MPLS, 4, 0, 0x24441, ARAD_PARSER_PFC_IPV4_MPLS3_ETH, ARAD_PARSER_PFC_IPV4_MPLS3_ETH, DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV6_IPV4_ETH,  "IPv6oIPv4oETH",  ARAD_PARSER_PFC_IPV6_IPV4_ETH,  IS_ETH | IS_IPV4, 4, 0, 0x00321, ARAD_PARSER_PFC_IPV6_IPV4_ETH,  ARAD_PARSER_PFC_IPV6_IPV4_ETH,  DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV6_IPV6_ETH,  "IPv6oIPv6oETH",  ARAD_PARSER_PFC_IPV6_IPV6_ETH,  IS_ETH | IS_IPV6, 4, 0, 0x00331, ARAD_PARSER_PFC_IPV6_IPV6_ETH,  ARAD_PARSER_PFC_IPV6_IPV6_ETH,  DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV6_MPLS1_ETH, "IPv6oMPLSx1oETH",ARAD_PARSER_PFC_IPV6_MPLS1_ETH, IS_ETH | IS_MPLS, 4, 0, 0x00341, ARAD_PARSER_PFC_IPV6_MPLS1_ETH, ARAD_PARSER_PFC_IPV6_MPLS1_ETH, DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV6_MPLS2_ETH, "IPv6oMPLSx2oETH",ARAD_PARSER_PFC_IPV6_MPLS2_ETH, IS_ETH | IS_MPLS, 4, 0, 0x03441, ARAD_PARSER_PFC_IPV6_MPLS2_ETH, ARAD_PARSER_PFC_IPV6_MPLS2_ETH, DPP_PLC_PROFILE_PP},
    { DPP_PFC_IPV6_MPLS3_ETH, "IPv6oMPLSx3oETH",ARAD_PARSER_PFC_IPV6_MPLS3_ETH, IS_ETH | IS_MPLS, 4, 0, 0x34441, ARAD_PARSER_PFC_IPV6_MPLS3_ETH, ARAD_PARSER_PFC_IPV6_MPLS3_ETH, DPP_PLC_PROFILE_PP},
    { DPP_PFC_RAW_AND_FTMH,   "RAW_FTMH",       ARAD_PARSER_PFC_RAW_AND_FTMH,   IS_UD,            0, 0, 0,       ARAD_PARSER_PFC_RAW_AND_FTMH,   ARAD_PARSER_PFC_RAW_AND_FTMH,   DPP_PLC_PROFILE_RAW_FTMH},
    { DPP_PFC_TM,             "TM",             ARAD_PARSER_PFC_TM,             IS_UD,            0, 0, 0,       ARAD_PARSER_PFC_TM,             ARAD_PARSER_PFC_TM,             DPP_PLC_PROFILE_TM},
    { DPP_PFC_TM_IS,          "TM_IS",          ARAD_PARSER_PFC_TM_IS,          IS_UD,            0, 0, 0,       ARAD_PARSER_PFC_TM_IS,          ARAD_PARSER_PFC_TM_IS,          DPP_PLC_PROFILE_TM_IS},
    { DPP_PFC_TM_MC_FLOW,     "TM_MC_FLOW",     ARAD_PARSER_PFC_TM_MC_FLOW,     IS_UD,            0, 0, 0,       ARAD_PARSER_PFC_TM_MC_FLOW,     ARAD_PARSER_PFC_TM_MC_FLOW,     DPP_PLC_PROFILE_TM_MC_FLOW},
    { DPP_PFC_TM_OUT_LIF,     "TM_OUT_LIF",     ARAD_PARSER_PFC_TM_OUT_LIF,     IS_UD,            0, 0, 0,       ARAD_PARSER_PFC_TM_OUT_LIF,     ARAD_PARSER_PFC_TM_OUT_LIF,     DPP_PLC_PROFILE_TM_OUT_LIF},
    { -1}
};

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */
int dpp_parser_pfc_get_hw_by_sw(DPP_PFC_E pfc_sw, DPP_PFC_MASK_E pfc_sw_mask, uint32 *pfc_hw_p, uint32 *pfc_hw_mask_p)
{
    dpp_parser_pfc_info_t *pfc_info;
    uint32 pfc_hw = 0, pfc_hw_mask = 0;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->sw == pfc_sw)
        {
            pfc_hw = pfc_info->hw;
            break;
        }
    }

    if(pfc_info->sw == -1)
    {
        return SOC_SAND_ERR;
    }

    switch(pfc_sw_mask)
    {
        case DPP_PFC_MATCH_NA:
            break;
        case DPP_PFC_MATCH_ONE:
            pfc_hw_mask = ARAD_PARSER_PFC_PS_MATCH_ONE;
            break;
        case DPP_PFC_MATCH_ANY_ENCAP:
            pfc_hw_mask = ARAD_PARSER_PFC_PS_MATCH_ANY_ENCAP;
            break;
        default:
            return SOC_SAND_ERR;
    }

    if(pfc_hw_p != NULL)
        *pfc_hw_p = pfc_hw;

    if(pfc_hw_mask_p != NULL)
        *pfc_hw_mask_p = pfc_hw_mask;

    return SOC_SAND_OK;
}

int dpp_parser_pfc_get_vtt_by_sw(DPP_PFC_E pfc_sw, DPP_PFC_MASK_E pfc_sw_mask, uint32 *pfc_vtt_p, uint32 *pfc_vtt_mask_p)
{
    dpp_parser_pfc_info_t *pfc_info;
    uint32 pfc_vtt = 0, pfc_vtt_mask = 0;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->sw == pfc_sw)
        {
            pfc_vtt = pfc_info->vtt;
            break;
        }
    }

    if(pfc_info->sw == -1)
    {
        return SOC_SAND_ERR;
    }

    switch(pfc_sw_mask)
    {
        case DPP_PFC_MATCH_NA:
            break;
        case DPP_PFC_MATCH_ONE:
            pfc_vtt_mask = ARAD_PARSER_PFC_PS_MATCH_ONE;
            break;
        case DPP_PFC_MATCH_ANY_ENCAP:
            pfc_vtt_mask = ARAD_PARSER_PFC_PS_MATCH_ANY_ENCAP;
            break;
        default:
            return SOC_SAND_ERR;
    }

    if(pfc_vtt_p != NULL)
        *pfc_vtt_p = pfc_vtt;

    if(pfc_vtt_mask_p != NULL)
        *pfc_vtt_mask_p = pfc_vtt_mask;

    return SOC_SAND_OK;
}

int dpp_parser_pfc_get_acl_by_sw(DPP_PFC_E pfc_sw, DPP_PFC_MASK_E pfc_sw_mask, uint32 *pfc_acl_p, uint32 *pfc_acl_mask_p)
{
    dpp_parser_pfc_info_t *pfc_info;
    uint32 pfc_acl = 0, pfc_acl_mask = 0;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->sw == pfc_sw)
        {
            pfc_acl = pfc_info->pmf;
            break;
        }
    }

    if(pfc_info->sw == -1)
    {
        return SOC_SAND_ERR;
    }

    switch(pfc_sw_mask)
    {
        case DPP_PFC_MATCH_ONE:
            pfc_acl_mask = ARAD_PARSER_PFC_ACL_MATCH_ONE;
            break;
        case DPP_PFC_MATCH_ANY_MPLS:
            pfc_acl_mask = ARAD_PARSER_PFC_ACL_MATCH_ANY_MPLS;
            break;
        case DPP_PFC_MATCH_ANY_ENCAP:
            pfc_acl_mask = ARAD_PARSER_PFC_ACL_MATCH_ANY_ENCAP;
            break;
        default:
            return SOC_SAND_ERR;
    }

    if(pfc_acl_p != NULL)
        *pfc_acl_p = pfc_acl;

    if(pfc_acl_mask_p != NULL)
        *pfc_acl_mask_p = pfc_acl_mask;

    return SOC_SAND_OK;
}

int dpp_parser_pfc_get_sw_by_acl(uint32 pfc_acl, uint32 pfc_acl_mask, DPP_PFC_E *pfc_sw_p, DPP_PFC_MASK_E *pfc_sw_mask_p)
{
    dpp_parser_pfc_info_t *pfc_info;
    DPP_PFC_E pfc_sw = 0;
    DPP_PFC_MASK_E pfc_sw_mask = 0;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->pmf == pfc_acl)
        {
            pfc_sw = pfc_info->sw;
            break;
        }
    }

    if(pfc_info->sw == -1)
    {
        return SOC_SAND_ERR;
    }

    switch(pfc_acl)
    {
        case ARAD_PARSER_PFC_ACL_MATCH_ONE:
            pfc_sw_mask = DPP_PFC_MATCH_ONE;
            break;
        case ARAD_PARSER_PFC_ACL_MATCH_ANY_MPLS:
            pfc_sw_mask = DPP_PFC_MATCH_ANY_MPLS;
            break;
        case ARAD_PARSER_PFC_ACL_MATCH_ANY_ENCAP:
            pfc_sw_mask = DPP_PFC_MATCH_ANY_ENCAP;
            break;
        default:
            return SOC_SAND_ERR;
    }

    if(pfc_sw_p != NULL)
    {
        *pfc_sw_p = pfc_sw;
    }

    if(pfc_sw_mask_p != NULL)
    {
        *pfc_sw_mask_p = pfc_sw_mask;
    }

    return SOC_SAND_OK;
}

int dpp_parser_pfc_get_pmf_by_sw(DPP_PFC_E pfc_sw, DPP_PFC_MASK_E pfc_sw_mask, uint32 *pfc_pmf_p, uint32 *pfc_pmf_mask_p)
{
    dpp_parser_pfc_info_t *pfc_info;
    uint32 pfc_pmf = 0, pfc_pmf_mask = 0;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->sw == pfc_sw)
        {
            pfc_pmf = pfc_info->pmf;
            break;
        }
    }

    if(pfc_info->sw == -1)
    {
        return SOC_SAND_ERR;
    }

    switch(pfc_sw_mask)
    {
        case DPP_PFC_MATCH_NA:
            break;
        case DPP_PFC_MATCH_ONE:
            pfc_pmf_mask = ARAD_PARSER_PFC_PS_MATCH_ONE;
            break;
        case DPP_PFC_MATCH_TYPE:
            pfc_pmf_mask = ARAD_PARSER_PFC_PS_MATCH_TYPE;
            break;
        default:
            return SOC_SAND_ERR;
    }

    if(pfc_pmf_p != NULL)
    {
        *pfc_pmf_p = pfc_pmf;
    }

    if(pfc_pmf_mask_p != NULL)
    {
        *pfc_pmf_mask_p = pfc_pmf_mask;
    }

    return SOC_SAND_OK;
}

int dpp_parser_pfc_get_sw_by_pmf(uint32 pfc_pmf, uint32 pfc_pmf_mask, DPP_PFC_E *pfc_sw_p, DPP_PFC_MASK_E *pfc_sw_mask_p)
{
    dpp_parser_pfc_info_t *pfc_info;
    DPP_PFC_E pfc_sw = 0;
    DPP_PFC_MASK_E pfc_sw_mask = 0;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->pmf == pfc_pmf)
        {
            pfc_sw = pfc_info->sw;
            break;
        }
    }

    if(pfc_info->sw == -1)
    {
        return SOC_SAND_ERR;
    }

    switch(pfc_pmf_mask)
    {
        case ARAD_PARSER_PFC_PS_MATCH_ONE:
            pfc_sw_mask = DPP_PFC_MATCH_ONE;
            break;
        case ARAD_PARSER_PFC_PS_MATCH_TYPE:
            pfc_sw_mask = DPP_PFC_MATCH_TYPE;
            break;
        default:
            return SOC_SAND_ERR;
    }

    if(pfc_sw_p != NULL)
    {
        *pfc_sw_p = pfc_sw;
    }

    if(pfc_sw_mask_p != NULL)
    {
        *pfc_sw_mask_p = pfc_sw_mask;
    }

    return SOC_SAND_OK;
}

char *dpp_parser_pfc_string_by_hw(int pfc_hw)
{
    dpp_parser_pfc_info_t *pfc_info;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->hw == pfc_hw)
        {
            return pfc_info->name;
        }
    }
    return "unknown";
}

char *dpp_parser_pfc_string_by_sw(DPP_PFC_E pfc_sw)
{
    dpp_parser_pfc_info_t *pfc_info;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->sw == pfc_sw)
        {
            return pfc_info->name;
        }
    }
    return "unknown";
}

dpp_parser_plc_profile_t *dpp_parser_plc_profile_by_pfc_info(dpp_parser_pfc_info_t *pfc_info)
{
    dpp_parser_plc_profile_t *plc_profile;
    for(plc_profile = dpp_parser_plc_profiles; plc_profile->id != -1; plc_profile++)
    {
        if(plc_profile->id == pfc_info->plc_profile_id)
            return plc_profile;
    }
    return NULL;
}

int dpp_parser_plc_hw_by_sw(DPP_PLC_PROFILE_E plc_profile_id, DPP_PLC_E plc_sw, DPP_PLC_MASK_E plc_sw_mask, uint32 *plc_hw_p, uint32 *plc_hw_mask_p)
{
    dpp_parser_plc_profile_t *plc_profile;
    uint32 plc_hw = 0, plc_hw_mask = 0;

    for(plc_profile = dpp_parser_plc_profiles; plc_profile->id != -1; plc_profile++)
    {
        if(plc_profile->id == plc_profile_id)
        {
            dpp_parser_plc_info_t *plc_info;
            for(plc_info = plc_profile->plc_info; plc_info->plc_sw != -1; plc_info++)
            {
                if(plc_info->plc_sw == plc_sw) {
                    plc_hw = plc_info->plc_hw;
                    break;
                }
            }
            if(plc_info->plc_sw == -1)
            {
                return SOC_SAND_ERR;
            }
            break;
        }
    }

    switch(plc_sw_mask)
    {
        case DPP_PLC_MATCH_NA:
            break;
        case DPP_PLC_MATCH_ONE:
            plc_hw_mask = ARAD_PARSER_PLC_MATCH_ONE;
            break;
        case DPP_PLC_MATCH_FCOE:
            plc_hw_mask = ARAD_PARSER_PLC_MATCH_ANY_FCOE;
            break;
        case DPP_PLC_MATCH_PP:
            plc_hw_mask = ARAD_PARSER_PLC_MATCH_ANY_FLP_PP;
            break;
        case DPP_PLC_MATCH_ANY:
            plc_hw_mask = ARAD_PARSER_PLC_MATCH_ANY;
            break;
        default:
            return SOC_SAND_ERR;
    }

    if(plc_hw_p != NULL)
    {
        *plc_hw_p = plc_hw;
    }

    if(plc_hw_mask_p != NULL)
    {
        *plc_hw_mask_p = plc_hw_mask;
    }

    return SOC_SAND_OK;
}

char *dpp_parser_plc_string_by_hw(int pfc_hw, int plc_hw)
{
    dpp_parser_pfc_info_t *pfc_info;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        dpp_parser_plc_profile_t *plc_profile;
        if(pfc_info->hw != pfc_hw)
            continue;

        for(plc_profile = dpp_parser_plc_profiles; plc_profile->id != -1; plc_profile++)
        {
            dpp_parser_plc_info_t *plc_info;
            if(plc_profile->id != pfc_info->plc_profile_id)
                continue;

            for(plc_info = plc_profile->plc_info; plc_info->plc_sw != -1; plc_info++)
            {
                if(plc_info->plc_hw == plc_hw)
                {
                    return plc_info->name;
                }
            }
            break;
        }
        break;
    }
    return "unknown";
}

uint32
dpp_parser_pfc_l4_location_init(int unit)
{
    uint32 res;
    dpp_parser_pfc_info_t *pfc_info;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        res = WRITE_IHB_PFC_INFOm(unit, MEM_BLOCK_ANY, pfc_info->hw, &pfc_info->l4_location);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_static_to_dynamic_macro()", 0, 0);
}

uint32
dpp_parser_pfc_map_init(int unit)
{
    uint32 res;
    dpp_parser_pfc_info_t *pfc_info;
    uint64 reg64_val;
    soc_reg_above_64_val_t pmf_pfc_map;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(pmf_pfc_map);
    COMPILER_64_ZERO(reg64_val);

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        /* Copy packet_format_code_ndx in bits [(6*packet_format_code_ndx+5):(6*packet_format_code_ndx)]*/
        SHR_BITCOPY_RANGE(pmf_pfc_map, (ARAD_PARSER_PFC_NOF_BITS * pfc_info->hw), &pfc_info->pmf, 0, ARAD_PARSER_PFC_NOF_BITS);
        if(pfc_info->is_proto & IS_ETH)
        {
            COMPILER_64_BITSET(reg64_val, pfc_info->hw);
        }
        /* 1x1 mapping VTT PacketFormatCode mapping */
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 138, exit, WRITE_IHP_VTT_PS_PACKET_FORMAT_CODE_MAP_TABLEm(unit, MEM_BLOCK_ANY, pfc_info->hw, &pfc_info->vtt));
    }
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 140, exit, WRITE_IHB_PMF_PFC_ACL_MAPPINGr(unit, REG_PORT_ANY, pmf_pfc_map));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 142, exit, WRITE_IHB_PMF_PFC_PS_MAPPINGr(unit,  REG_PORT_ANY, pmf_pfc_map));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 144, exit, WRITE_IHB_PMF_PFC_IS_ETHr(unit,      REG_PORT_ANY, reg64_val));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_static_to_dynamic_macro()", 0, 0);
}

SOC_PPC_PKT_HDR_TYPE
  dpp_parser_pfc_hdr_type_at_index(
    int    pfc_hw,
    uint32 hdr_index
  )
{
    SOC_PPC_PKT_HDR_TYPE hdr_type = 0;
    dpp_parser_pfc_info_t *pfc_info;

    for(pfc_info = dpp_parser_pfc_info; pfc_info->sw != -1; pfc_info++)
    {
        if(pfc_info->hw == pfc_hw)
        {
            hdr_type = SOC_SAND_GET_BITS_RANGE(pfc_info->hdr_type_map, 4*hdr_index + 3, 4*hdr_index);
            break;
        }
    }

  return hdr_type;
}

char *arad_parser_get_macro_str(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 macro_sel)
{
    int array_index;

    if(macro_sel >= 4)
        return parser_static_macro[macro_sel];

    for (array_index = 0; array_index < ARAD_PARSER_DYNAMIC_MACRO_COUNT; array_index++) {
        if(dynamic_macro[unit][array_index] == macro_sel)
            return parser_dynamic_macro[array_index];
    }

    return NULL;
}

static 
uint32
    arad_parser_static_to_dynamic_macro(
       SOC_SAND_IN   int                                    unit,
       SOC_SAND_IN   ARAD_PARSER_STATIC_TO_DYNAMIC_MACRO    static_to_dynamic_table[],
       SOC_SAND_OUT  uint32                                 tbl_size,
       SOC_SAND_IN   ARAD_PARSERR_INFO                      *info_entry,
       SOC_SAND_OUT  uint32                                 *macro_sel,
       SOC_SAND_OUT  uint32                                 *next_addr_base
    )
{
    uint32 
        index,
        app_index = ARAD_PARSER_DYNAMIC_MACRO_COUNT;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(info_entry);
    SOC_SAND_CHECK_NULL_INPUT(macro_sel);
    SOC_SAND_CHECK_NULL_INPUT(next_addr_base);

    /* Initialize macro and next address with original values */
    *macro_sel = info_entry->macro_sel;
    *next_addr_base = info_entry->next_addr_base;

    for (index = 0; index < tbl_size; index++) 
    {
        /* The mapping is performed according to the following conditions:
         * Cond 1: instruction address is in the correct range (exists in mapping table)
         * Cond 2: custom macro exists in mapping table 
         * Cond 3: next address is not END, unless it is specified in table that it's included 
         */
        if((info_entry->instruction_addr >= static_to_dynamic_table[index].instruction_addr) /* Cond 1 */
           && (info_entry->instruction_addr < static_to_dynamic_table[index].instruction_addr + ARAD_PARSER_PROG_MULTIPLIER) /* Cond 1 */
           && (info_entry->macro_sel == static_to_dynamic_table[index].static_macro_sel) /* Cond 2 */
           && ((info_entry->next_addr_base != ARAD_PARSER_PROG_END) /* Cond 3 */
               || static_to_dynamic_table[index].include_prog_end)) /* Cond 3 */
        {
            app_index = static_to_dynamic_table[index].app_id;

            /* Special case for UDPoIPV4 - operates VxLAN or UDP Tunnel macro first
             * and then continues to UDP custom macro at any case (good even
             * when it's not a VxLAN packet). 
             * If VxLAN and UDP tunnel are not enabled, then change this operate UDP macro
             * immediately after IPv4 (without the VxLAN or UDP tunnel macro).
             */
            if ((ARAD_PARSER_PROG_IPV4oE_BASE_START == static_to_dynamic_table[index].instruction_addr)
                && (ARAD_PARSER_MACRO_SEL_UDP_VX == static_to_dynamic_table[index].static_macro_sel)
                && (ARAD_PARSER_CUSTOM_MACRO_INVALID == dynamic_macro[unit][app_index]))
            {
                if (SOC_DPP_CONFIG(unit)->pp.udp_tunnel_enable) {
                    /* activate udp tunnel macro if soc property is enabled */
                    app_index = ARAD_PARSER_DYNAMIC_MACRO_UDP_TUNNEL;
                    *next_addr_base = ARAD_PARSER_PROG_UDPoIPV4oE_BASE;
                } else {
                    app_index = ARAD_PARSER_DYNAMIC_MACRO_UDP;
                    *next_addr_base = ARAD_PARSER_PROG_FIN;
                }
				/* bfd ipv4 single hop*/
	            if (SOC_DPP_CONFIG(unit)->pp.bfd_ipv4_single_hop || SOC_DPP_CONFIG(unit)->pp.limited_gtp_parsing ){
					app_index = ARAD_PARSER_DYNAMIC_MACRO_UDP;
	                *next_addr_base = ARAD_PARSER_PROG_UDPoIPV4oE_BASE;
	            }
			
            }
            else if ((ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START == static_to_dynamic_table[index].instruction_addr)
                    && (ARAD_PARSER_MACRO_SEL_UDP == static_to_dynamic_table[index].static_macro_sel)
                    && (ARAD_PARSER_CUSTOM_MACRO_INVALID == dynamic_macro[unit][ARAD_PARSER_DYNAMIC_MACRO_VXLAN])
                    && ((SOC_DPP_CONFIG(unit)->pp.bfd_ipv4_single_hop) || SOC_DPP_CONFIG(unit)->pp.limited_gtp_parsing) )
            {
                *next_addr_base = ARAD_PARSER_PROG_END;
            }

            /* EoE */
            else if ((ARAD_PARSER_PROG_E_BASE_START == static_to_dynamic_table[index].instruction_addr)
                    && (ARAD_PARSER_MACRO_SEL_EoE == static_to_dynamic_table[index].static_macro_sel)
                    && (ARAD_PARSER_CUSTOM_MACRO_INVALID == dynamic_macro[unit][ARAD_PARSER_DYNAMIC_MACRO_EoE])
                    && (SOC_DPP_CONFIG(unit)->pp.test2 == 0) )
            {
                *next_addr_base = ARAD_PARSER_PROG_END;
            }

            /* modify custom macro for found application ID */
            *macro_sel = dynamic_macro[unit][app_index];

            break;
        }
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_static_to_dynamic_macro()", 0, 0);
}

static 
uint32
    arad_parser_dynamic_macro_allocate(
       SOC_SAND_IN  int     unit,
       SOC_SAND_IN  uint32  app_entry,
       SOC_SAND_OUT uint32  *custom_macro
    )
{
    uint32  
        idx;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    *custom_macro = ARAD_PARSER_CUSTOM_MACRO_INVALID;

    /* iterate over all custom macros */
    for (idx = 0; idx < ARAD_PARSER_CUSTOM_MACRO_COUNT; idx++) {
        /* find a free custom macro */
        if (!SHR_BITGET(arad_parser_custom_macro_used_bmp[unit], idx)) 
        {
            /* Allocate the custom macro */
            SHR_BITSET(arad_parser_custom_macro_used_bmp[unit], idx);

            /* no need to continue searching */
            *custom_macro = idx;
            dynamic_macro[unit][app_entry] = *custom_macro;

            break;
        }
    }

    /* Allocation failure - no available macros found */
    if (*custom_macro == ARAD_PARSER_CUSTOM_MACRO_INVALID) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_ERR, 10, exit);
    }
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_dynamic_macro_allocate()", 0, 0);
}

static
uint32
    arad_parser_ipv6_extension_header_ip_protocol_set(
       SOC_SAND_IN int  unit
    )
{
    uint32 res = SOC_SAND_OK;
    uint32 entry_offset, entry_offset_start, entry_offset_max;
    ARAD_PP_IHP_PARSER_IP_PROTOCOLS_TBL_DATA
      ihp_parser_ip_protocols_tbl_data[ARAD_PARSER_IP_PROTO_NOF_ENTRIES];
	
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    sal_memset(ihp_parser_ip_protocols_tbl_data, 0x0, ARAD_PARSER_IP_PROTO_NOF_ENTRIES * sizeof(ARAD_PP_IHP_PARSER_IP_PROTOCOLS_TBL_DATA));

	if (SOC_IS_JERICHO_PLUS(unit))
	{
        /* For QAX and Jericho-Plus, the IPv6 macro supports parsing of up to one (1) IPv6-Extension-headers.
         * In order to enable that, the IP protocols should be configured with the relevant protocols. 
         * The macro compares the IP-Protocol in IPv6 header, and if matches to one of IPv6-Ext protocols 
         * it parses the extension header after the IPv6 header. 
         * The parsing of the extension header compares the next protocol to IP-Protocol table and updates 
         * IPv6 qualifier accordingly.
         * Also, it calculates the size according to field hdr-ext-length. The length is in 8-octet units, 
         * not including the first 8 octets (e.g. if hdr-ext-len==1, then extension-header size is 
         * 2 * 8-Octets = 16 bytes). The size of the extension header is added to IPv6 header length. 
         * Extension header is not considered a separate header in header offsets. 
         *  
         * Most of Ipv6 extension header protocols have the above format but not all. Only extensions with 
         * that format will be supported. 
         *  
         * According to http://www.iana.org/assignments/ipv6-parameters/ipv6-parameters.xhtml the following 
         * protocols are supported: Hop-by-Hop (0), Destination-Options (60), Routing (43), Fragment (44), 
         * Mobility (135), Host-Identity-Protocol (HIP) (139), Shim6 (140). 
         *  
         * IPv6 Extension headers that are NOT supported are: Encapsulating-Security-Payload (50), 
         * Authentication-Header (51), and two more (253) and (254) Use for experimentation and testing.
         *  
         */

		entry_offset_start = 0;
		entry_offset_max   = 6;

		/* Hop-by-Hop (0) */
		ihp_parser_ip_protocols_tbl_data[0].ip_protocol     = ARAD_PARSER_IP_PROTO_IPV6_EXT_HOP_BY_HOP;
		ihp_parser_ip_protocols_tbl_data[0].extension_valid = 1;

		/* Destination-Options (60) */
		ihp_parser_ip_protocols_tbl_data[1].ip_protocol     = ARAD_PARSER_IP_PROTO_IPV6_EXT_DEST_OPTIONS;
		ihp_parser_ip_protocols_tbl_data[1].extension_valid = 1;

		/* Routing (43) */
		ihp_parser_ip_protocols_tbl_data[2].ip_protocol     = ARAD_PARSER_IP_PROTO_IPV6_EXT_ROUTING;
		ihp_parser_ip_protocols_tbl_data[2].extension_valid = 1;

		/* Fragment (44) */
		ihp_parser_ip_protocols_tbl_data[3].ip_protocol     = ARAD_PARSER_IP_PROTO_IPV6_EXT_FRAGMENT;
		ihp_parser_ip_protocols_tbl_data[3].extension_valid = 1;

		/* Mobility (135) */
		ihp_parser_ip_protocols_tbl_data[4].ip_protocol     = ARAD_PARSER_IP_PROTO_IPV6_EXT_MOBILITY;
		ihp_parser_ip_protocols_tbl_data[4].extension_valid = 1;

		/* Host-Identity-Protocol (HIP) (139) */
		ihp_parser_ip_protocols_tbl_data[5].ip_protocol     = ARAD_PARSER_IP_PROTO_IPV6_EXT_HIP;
		ihp_parser_ip_protocols_tbl_data[5].extension_valid = 1;

		/* Shim6 (140) */
		ihp_parser_ip_protocols_tbl_data[6].ip_protocol     = ARAD_PARSER_IP_PROTO_IPV6_EXT_SHIM6;
		ihp_parser_ip_protocols_tbl_data[6].extension_valid = 1;

	}
	else
	{
        /* For Arad/jericho devices, the IPv6 extension header parsing is performed using a custom macro.
         * Only Hop-by-Hop extension header is supported.
         */
		entry_offset_start = 1;
		entry_offset_max   = 1;

		/* Hop-by-Hop (0) */
		ihp_parser_ip_protocols_tbl_data[1].ip_protocol = 0x0;
	}

	for(entry_offset = entry_offset_start; entry_offset <= entry_offset_max; entry_offset++)
	{
		res = arad_pp_ihp_parser_ip_protocols_tbl_set_unsafe(
				unit,
				entry_offset,
				&(ihp_parser_ip_protocols_tbl_data[entry_offset])
			  );
		SOC_SAND_CHECK_FUNC_RESULT(res, entry_offset * 10, exit);
	}

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_ipv6_extension_header_ip_protocol_set()", 0, 0);
}

static
uint32
arad_parser_fcoe_programs_set(SOC_SAND_IN int unit)
{
    uint32 res = SOC_SAND_OK;
    ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PARAMETERS_TBL_DATA
        ihp_parser_custom_macro_parameters_tbl_data;
    ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_DATA
      ihp_parser_custom_macro_protocols_tbl_data;
    uint32
        tbl_data,
        custom_macro;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_FCOE_1, &custom_macro);
    SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

    /* fcoe 1st macro */                                                                                /*comments (taken from verilog file): */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 0;                             /*start from vft (if exist)                                                                                                                                                                                        */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 0;                             /*                                                                                                                                                                                                                 */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 24;                            /*compare 8 msb                                                                                                                                                                                                    */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 3;                             /*compare all 32 bits                                                                                                                                                                                              */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0x6000;                        /*compare entries 13,14                                                                                                                                                                                            */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 28/* b11100 */;                /*compare to next address offset bits 4:0                                                                                                                                                                          */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 0;                             /*if (par_custom_macro_param_entry.Data.CstmConditionSelect == next_address_offset[4:0]) macro_size = par_custom_macro_param_entry.Data.CstmShiftA; else macro_size = par_custom_macro_param_entry.Data.CstmShiftB;*/
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 8;                             /*VFT size                                                                                                                                                                                                         */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 24;                            /*Standard size 24B                                                                                                                                                                                                */
    ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0;                             /*                                                                                                                                                                                                                 */
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 15;                            /*we need Protocol value in bits 10:7 of the qualfier to set new next-protocol                                                                                                                                     */
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0;                             /*Not relevant                                                                                                                                                                                                     */
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0x18/* {4'h0, 2'h3, 3'h0} */;  /*Not relevant                                                                                                                                                                                                     */
    ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0x7ff;                         /*Not relevant                                                                                                                                                                                                     */

    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(unit,
                                                                    custom_macro,
                                                                    &ihp_parser_custom_macro_parameters_tbl_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);


    res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_FCOE_2, &custom_macro);
    SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

    /* fcoe 2nd macro */                                                                  /*comments (taken from verilog file): */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 1;               /*start after version + reserved                                                                                      */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 0;               /*                                                                                                                    */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 0;               /*                                                                                                                    */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 4;               /*compare to eth values                                                                                               */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0x6000;          /*compare all values                                                                                                  */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;               /*we don't care, size is always 4B                                                                                    */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 1;               /*Size is data4B << par_custom_macro_param_entry.Data.CstmShiftA[3:0]) + par_custom_macro_param_entry.Data.CstmShiftB */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 16 /* b10000 */; /*bit 4 is set                                                                                                        */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 14;              /*size is always 14B                                                                                                  */
    ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 1;               /*invisible stage!                                                                                                    */
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 14;              /*we need Protocol value in bits 10:7 of the qualfier to set new next-protocol                                        */
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0;               /*                                                                                                                    */
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0;               /*reset all                                                                                                           */
    ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0;               /*don't change values of qualifier                                                                                    */

    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(unit,
                                                                    custom_macro,
                                                                    &ihp_parser_custom_macro_parameters_tbl_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);


    /* protocols for FCoe */

    /* VFT */
    ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0x50000000;
    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_14,
            &ihp_parser_custom_macro_protocols_tbl_data
         );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* encapsulation */
    ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0x52000000;
    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_15,
            &ihp_parser_custom_macro_protocols_tbl_data
         );
    SOC_SAND_CHECK_FUNC_RESULT(res, 31, exit);

    /* fill memory with the entry in data */
    tbl_data = 1;
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 32, exit, WRITE_IHP_PARSER_CUSTOM_MACRO_WORD_MAPm(unit, MEM_BLOCK_ANY,0x50, &tbl_data));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_fcoe_programs_set()", 0, 0);
}

STATIC int _arad_packet_format_tbl_eth_callback(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int copyno,
    SOC_SAND_IN int array_index,
    SOC_SAND_IN int index,
    SOC_SAND_OUT uint32 *value,
    SOC_SAND_IN int entry_sz,
    SOC_SAND_IN void *opaque)
{
    uint32 packet_format_code = 0;
    uint32 parser_pmf_profile = 0;
    uint32 instr_address = 0;
    uint32 uindex = (uint32)index;
    uint32 *arad_parser_packet_format_tbl_eth_indx_p = (uint32*)opaque;
    uint32 arad_parser_packet_format_tbl_eth_indx = *arad_parser_packet_format_tbl_eth_indx_p;

    instr_address = Arad_parserr_info_eth[arad_parser_packet_format_tbl_eth_indx].instruction_addr;

    if (uindex == instr_address)
    {
        packet_format_code = Arad_parserr_info_eth[arad_parser_packet_format_tbl_eth_indx].packet_format_code;
        parser_pmf_profile = Arad_parserr_info_eth[arad_parser_packet_format_tbl_eth_indx].parser_pmf_profile;

        soc_mem_field32_set(unit, IHP_PACKET_FORMAT_TABLEm, value, PACKET_FORMAT_CODEf, packet_format_code);
        soc_mem_field32_set(unit, IHP_PACKET_FORMAT_TABLEm, value, PARSER_LEAF_CONTEXTf, parser_pmf_profile);

        (*arad_parser_packet_format_tbl_eth_indx_p)++;
    }

    return 0;
}

STATIC int _arad_parser_program_tbl_eth_callback(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int copyno,
    SOC_SAND_IN int array_index,
    SOC_SAND_IN int index,
    SOC_SAND_OUT uint32 *value,
    SOC_SAND_IN int entry_sz,
    SOC_SAND_IN void *opaque)
{
    uint32 macro_sel = 0;
    uint32 next_addr_base = 0;
    uint32 instr_address = 0;
    uint32 uindex = (uint32)index;
    uint32 *arad_parser_parser_program_tbl_eth_indx_p = (uint32*)opaque;
    uint32 arad_parser_parser_program_tbl_eth_indx = *arad_parser_parser_program_tbl_eth_indx_p;

    instr_address = Arad_parserr_info_eth[arad_parser_parser_program_tbl_eth_indx].instruction_addr;
    /* In case the row index from the callback is equal to the address the Arad_parserr_info_eth table:
     * update the IHP_PARSER_PROGRAM table.
     * Otherwise - skip to the next hop*/
    if (uindex == instr_address)
    {
        arad_parser_static_to_dynamic_macro(
              unit,
              Arad_parser_macro_eth_map,
              (sizeof(Arad_parser_macro_eth_map) / sizeof(ARAD_PARSER_STATIC_TO_DYNAMIC_MACRO)),
              &(Arad_parserr_info_eth[arad_parser_parser_program_tbl_eth_indx]),
              &macro_sel,
              &next_addr_base);

        if ((next_addr_base != ARAD_PARSER_PROG_FIN) && (next_addr_base != ARAD_PARSER_PROG_END))
        {
          next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);
        }

        soc_mem_field32_set(unit, IHP_PARSER_PROGRAMm, value, NEXT_ADDR_BASEf, next_addr_base);
        soc_mem_field32_set(unit, IHP_PARSER_PROGRAMm, value, MACRO_SELf, macro_sel);

        (*arad_parser_parser_program_tbl_eth_indx_p)++;
    }
    return 0;
}

uint32
  arad_parser_init(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK,
      custom_macro,
      tbl_data,
      tm_second_iter = 0,
    idx,
    instr_addr,
    array_index,
    eth_rep_ndx,
    table_line,
    reg_val,
    addtional_tpid_enable,
    macro_sel,
    next_addr_base,
    non_first_fragments_enable;
  ARAD_PP_IHP_PARSER_PROGRAM_TBL_DATA
    ihp_parser_program_tbl_data;
  ARAD_PP_IHP_PACKET_FORMAT_TABLE_TBL_DATA
    ihp_packet_format_table_tbl_data;
  ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PARAMETERS_TBL_DATA
    ihp_parser_custom_macro_parameters_tbl_data;
  ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_DATA
    ihp_parser_custom_macro_protocols_tbl_data;
  ARAD_PP_IHP_PARSER_ETH_PROTOCOLS_TBL_DATA
    ihp_parser_eth_protocols_tbl_data;

  int instr_addr_index = 0;
  int application = 0;
  int application_start;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PARSER_INIT); 

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, ENABLE_DATA_PATHf,    0x1));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 20, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, MAX_HEADER_STACKf,    0x5));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 20, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, IP_NEXT_PROTOCOL_GRE_ETHERNETf,    0x1));

  if (SOC_IS_JERICHO(unit)) {
      non_first_fragments_enable = (soc_property_get(unit, spn_FIELD_IP_FIRST_FRAGMENT_PARSED, TRUE)) ? 0x1 : 0x0;

      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 05, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, FORCE_BUBBLESf,  0));
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 30, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, ENABLE_TCAM_BUBBLESf, 0x1));
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 40, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, ENABLE_MACT_BUBBLESf, 0x1));
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 40, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, NON_FIRST_FRAGMENTS_ENABLEf, non_first_fragments_enable));
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 40, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, ADDITIONAL_TPID_ENABLEf, 0x1));
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 20, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, LLC_CHECK_LSBf, 0x1));
  }
  else {
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 20, exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, FIELD_6_6f, 0x1));
  }

  /* Init custom macros before dynamic allocation begins */
  arad_parser_custom_macro_used_bmp[unit][0] = 0;
  for (array_index = 0; array_index < ARAD_PARSER_DYNAMIC_MACRO_COUNT; array_index++) {
      dynamic_macro[unit][array_index] = ARAD_PARSER_CUSTOM_MACRO_INVALID;
  }
#ifdef PLISIM
  if ((!SAL_BOOT_PLISIM))
#endif
  {
      /* Zero the MacroWordMap table */
      tbl_data = 0;
      res = arad_fill_table_with_entry(unit, IHP_PARSER_CUSTOM_MACRO_WORD_MAPm, MEM_BLOCK_ANY, &tbl_data); /* fill memory with the entry in data */
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  }

  /* TM programs custom macros { */
      if (SOC_IS_JERICHO(unit)
          && !soc_property_get(unit, spn_ITMH_ARAD_MODE_ENABLE, 0)
          && !soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "itmh_jericho_parse_disable", 0)) 
      {
          /* Jericho mode */
          /* 
           * Configure custom protocols that will be used by custom macro
           */

          /* Extension + Outlif */
          ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0x8 << 5; 
          res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                  unit,
                  ARAD_PARSER_CUSTOM_MACRO_PROTO_1,
                  &ihp_parser_custom_macro_protocols_tbl_data
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

          /* Extension + Multicast-ID */
          ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0x9 << 5; 
          res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                  unit,
                  ARAD_PARSER_CUSTOM_MACRO_PROTO_2,
                  &ihp_parser_custom_macro_protocols_tbl_data
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

          /* Extension + ISQ-Flow-ID */
          ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0xa << 5; 
          res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                  unit,
                  ARAD_PARSER_CUSTOM_MACRO_PROTO_3,
                  &ihp_parser_custom_macro_protocols_tbl_data
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

          /* Extension + Default */
          ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0xf << 5; 
          res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                  unit,
                  ARAD_PARSER_CUSTOM_MACRO_PROTO_4,
                  &ihp_parser_custom_macro_protocols_tbl_data
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

          /*
           *  Configure the macro (macro 0) to identify whether the field ITMH_BASE_ EXTENSION_EXISTS is set.
           *  This is done by comparing the ITMH LSB bit 1.
           */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select = 1;         /* take 3 LSB bytes of ITMH header + 1 MSB of next header*/
          ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left = 23;          /* mask all bits except extension bit indication and type of extension if exists */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right = 5;          /* none */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode = 0x3;   /* Compare 32 bits */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask = 0xf;   /* Compare to protocol 1,2,3,4 */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select = 0;    /* Doesn't matter, size is always 4 bytes */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel = 0;           /* Take compare result */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a = 4;             /* Size is 4 bytes if compare is true */
          ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b = 4;             /* Size is 4 bytes if compare is false */
          ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0;   /* separate header */
          ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask = 0;        /*  */
          ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask = 0;       /*  */
          ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift =  0x1F/*-1*/;/*  */
          ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask = 0;    /*  */
      }
      else
      {
        /* ARAD mode */
        /*
         *  Configure the macro to identify whether the destination is of type with destination info ext.
         *  (macro 0). This is done by comparing the ITMH destination (including the destination type) to
         *  to custom protocols 0, 1, 3, 4, 5, 6, which are set according to the destination info ranges.
         */
        /* By default ingress shaping is off */
        ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select = 0;
        ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left = 12; /* all bits upto destination*/
        ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right = 0;
        ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode = 0x3; /* Compare 32 bits */
        ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask = 0x3F; /* Use protocols 0,1,2,3,4,5 */
        ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select = 3; /* Between protocol 1 and 2 */
        ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel = 0; /* take compare result */
        ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a = 4;
        ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b = 4;
        ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0;
        ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask = 0;
        ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask = 0;
        ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift =  0x1F/*-1*/;
        ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask = 0;
      }

    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
          unit,
          ARAD_PARSER_MACRO_SEL_TM_BASE + ARAD_PARSER_MACRO_SEL_1,
          &ihp_parser_custom_macro_parameters_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    /* 
     * Macro1 is used to set destination info extension offset size
     * , relevant only if destination info extension exists.
     */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right = 0;
    ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left = 0;
    ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select = 0;
    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask = 0x0; 
    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode = 0x0;
    ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select = 3; /* Between protocol 1 and 2 */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel = 0; /* take compare result */
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a = 3;
    ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b = 3;
    ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0;
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask = 0;
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask = 0;
    ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift =  0x1F/*-1*/;
    ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask = 0;

    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
          unit,
          ARAD_PARSER_MACRO_SEL_TM_BASE + ARAD_PARSER_MACRO_SEL_2,
          &ihp_parser_custom_macro_parameters_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);   
  
  /* TM programs custom macros } */

  if(SOC_DPP_CONFIG(unit)->trill.mode)
  {
      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_TRILL, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);   

      /*
       *  Configure the a macro to identify TRILL present
       *  (macro 3). only the shift matters.
       */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a = 0x14; /* bit 4 is set */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b = 6; /* we don't care. size is always 4B */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask = 0x7fff; /* compare all values */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode = 4; /* compare to eth values */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select = 0; /* we don't care, size is always 4B */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right = 6;
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left = 21;
      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select = 30; /* Go back 2 bytes */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel = 1; /* we don't care, size is always 4B */
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask = 0x780; /* change only the next-protocol values */
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0; /* Invisible stage */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift = 14; /* we need protocol value in bits 10:7 of the qualifier to set new next-protocol */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask = 0;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask = 0x780; /* reset all but next-protocol */

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
              unit,
              custom_macro,
              &ihp_parser_custom_macro_parameters_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);
  }

  /* Configure VxLAN custom macros only if enabled */
  if(SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & SOC_DPP_IP_TUNNEL_TERM_DB_VXLAN || SOC_DPP_CONFIG(unit)->pp.custom_feature_vxlan_port_termination)
  {
      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_VXLAN, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

      /* M3 */
      /* udp_invisible for VXLAN {*/
      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_VXLAN_DST_PORT;
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
              ARAD_PARSER_CUSTOM_MACRO_PROTO_8,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 151, exit);
     
      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_get_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 152, exit);

      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select = 0;                 /* from start of UDP */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left = 16;                  /* mask UDP Source port */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right = 0;                  /* look at UDP dest port */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode = 1;             /* Compare 16 lsb of selected data to 16 lsb of comparator values */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask = 0x80;      /* compare to entry 7 */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select = 16; /* (7+1)*2 */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel = 0;                   /* jump A if match */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a = 16;                  /* UDP+VXLAN */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b = 0;                  /* nothing */
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 1;           /* invisible stage */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift = 0;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask = 0;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask = 0;
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask = 0;            /* don't touch IP qualifier */

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 153, exit);

      /* udp_invisible for VXLAN } */
  }

  /* Configure UDP Tunnel custom macros only if enabled */
  if(SOC_DPP_CONFIG(unit)->pp.udp_tunnel_enable)
  {
      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_UDP_TUNNEL, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0x10000;   /* indicates non udp tunnel */
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
              ARAD_PARSER_CUSTOM_MACRO_PROTO_12,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 251, exit);

      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_UDP_IPV4_DST_PORT;
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
              ARAD_PARSER_CUSTOM_MACRO_PROTO_13,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 252, exit);

            ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_UDP_IPV6_DST_PORT;
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
              ARAD_PARSER_CUSTOM_MACRO_PROTO_14,
              &ihp_parser_custom_macro_protocols_tbl_data
            );

      SOC_SAND_CHECK_FUNC_RESULT(res, 253, exit);
            ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_UDP_MPLS_DST_PORT;
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
              ARAD_PARSER_CUSTOM_MACRO_PROTO_15,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 254, exit);

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_get_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 255, exit);

      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select = 0;             /* from start of UDP */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left = 16;              /* mask UDP Source port */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right = 0;              /* look at UDP dest port */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode = 3;         /* Compare 32 lsb of selected data to 32 lsb of comparator values */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask = 0x7800;    /* compare to entry 15, 14, 13 and 12 */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select = 23;       /* compare to not udp */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel = 0;               /* jump A if match */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a = 0;                 /* not UDP tunnel */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b = 8;                 /* UDP Tunnel */
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 1;       /* invisible stage */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift = 0xe;            /* take range-or-protocol */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask = 0;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask = 0x780;        /* change only the next-protocol values */
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask = 0x780;     /* change only the next-protocol values */

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 256, exit);
  }

  /* UDP Custom Macro is usually configured, unless it is speciffically
   * requested to be disabled with soc property custom_feature_udp_parse_disable 
   * (in case there is a shortage of custom macros).
   */
  if(!soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "udp_parse_disable", 0))
  {
      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_UDP, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

      /* udp custom macro {*/

      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 0;           /* take VID + eth-type (TPID is eth-type of previous header) */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 16;          /* mask VID data */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 1;           /* Compare 16 lsb of selected data to 16 lsb of comparator values */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0x600;       /* compare to entries 8 and 9 */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;           /* we don't care, size is always 4B */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 0;           /* we don't care, size is always 4B */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 8;           /* bit 4 is set */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 8;           /* we don't care, size is always 4B */
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0;           /* invisible stage */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 0xf;         /* we need Protocol value in bits 10:7 of the qualfier to set new next-protocol */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0x780;       /* reset all but next-protocol */
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0x780;       /* change only the next-protocol values */


      if ((SOC_DPP_CONFIG(unit)->pp.bfd_ipv4_single_hop) || (SOC_DPP_CONFIG(unit)->pp.bfd_ipv6_enable == SOC_DPP_ARAD_BFD_IPV6_SUPPORT_WITH_LEM)) {
          ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   |= (1 << ARAD_PARSER_CUSTOM_MACRO_PROTO_7);       /* add bfd ipv4 single hop macro protocol */
          if (SOC_IS_JERICHO(unit)) {
              ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   |= (1 << ARAD_PARSER_CUSTOM_MACRO_PROTO_9);       /* add micro bfd macro protocol */
          }
      }

      if (SOC_DPP_CONFIG(unit)->pp.limited_gtp_parsing) {
              ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   |= (1 << ARAD_PARSER_CUSTOM_MACRO_PROTO_14);
              ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   |= (1 << ARAD_PARSER_CUSTOM_MACRO_PROTO_15);
      }

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 154, exit);

      /* udp custom macro } */
  }


  if (SOC_DPP_CONFIG(unit)->pp.limited_gtp_parsing) {

      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_GTP, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 12;  /* 12 bytes size of GTP header, speculate according to first nibble after */      
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 0;   /* take 4 msbs */         
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 28;  /* zero 28 lsbs */        
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 3;   /* compare all 32 bits */       
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0x0;
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 0;
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 12;
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 12;
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 0xf;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0;
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0x780;
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0x780;


      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   |= (1 << ARAD_PARSER_CUSTOM_MACRO_PROTO_12);
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   |= (1 << ARAD_PARSER_CUSTOM_MACRO_PROTO_13);

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 154, exit);

  }


  /* Configure IPv6 Extension Header custom macros only if enabled */
  if(SOC_DPP_CONFIG(unit)->pp.ipv6_ext_header)
  {
      /* Configure relevant IP Protocols.
       * For Arad/Jericho - Currently only type 0 (Hop-by-Hop) is supported. 
       * For QAX/Jericho-Plus - Several extension headers supported. Look inside function.
       */
      res = arad_parser_ipv6_extension_header_ip_protocol_set(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

	  if (!SOC_IS_JERICHO_PLUS(unit))
	  {
		  /* First custom macro - get header size {*/

		  res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_1, &custom_macro);
		  SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

		  ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 29;          /* take next header */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 24;          /* mask anything else */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 5;           /* Compare 8 lsb to IP protocol */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0x7fff;      /* Compare all values */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 1;           /* don't advance header size or header qualifier */
		  ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0;           /* */

		  res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
				unit,
				custom_macro,
				&ihp_parser_custom_macro_parameters_tbl_data
			  );
		  SOC_SAND_CHECK_FUNC_RESULT(res, 154, exit);

		  /* First custom macro - get header size } */

		  /* Second custom macro - get header size {*/

		  res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_IPV6_EXT_2, &custom_macro);
		  SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);
		  
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 30;          /* take header size */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 24;          /* mask anything else */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 3;           /* Compare all 32 bits */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 1;           /* Use masked header shifted by Shift-A + Shift-B */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 3;           /* 0b00011 - shift 3 bit left (0x10011 would be shift right) */
		  ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 8;           /* Add a constant of 8 bytes */
		  ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 1;           /* Don't advance header qualifier and add header size to IPv6 header size */
		  ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0;           /* */
		  ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0;           /* */

		  res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
				unit,
				custom_macro,
				&ihp_parser_custom_macro_parameters_tbl_data
			  );
		  SOC_SAND_CHECK_FUNC_RESULT(res, 154, exit);
	  }
  }

  /* Configure Additional TPID custom macros only if enabled */
  addtional_tpid_enable = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "additional_tpid", 0);
  if (addtional_tpid_enable)
  {
      /* Configure relevant Ethertype:
       * Need to configure ethertype 0x88a8 - actualy this value is 
       * an additional (global) TPID. In case the outer-tag equals 
       * a global TPID, it is not recognized by the ethernet macro. 
       * The ethernet macro will then parse it as an ethertype and 
       * compare it to ethernet protocols. Then it should hit the new 
       * ethernet protocol we add. It's added as a user-defined 
       * ethernet protocol, and if there's a match, the following 
       * custom macro is ran, which sets the outer-tag to be TPID-2 
       * and calculates the next protocol. 
       *  
       * Ethernet macro will stop after outer-tag, thinking that 
       * it is ethertype:
       *  
       *    +-----+------+------------+------+-------------+
       *    | DA  |  SA  | Outer-Tag  | VID  |  Ethertype  |
       *    +-----+------+------------+------+-------------+
       *  
       * Note: 
       * This macro supports SINGLE-TAG PACKETS ONLY.
       */
      /* 0x88a8 = Additional TPID value - not real protocol, used as WA */
      ihp_parser_eth_protocols_tbl_data.eth_type_protocol = ARAD_PARSER_ETHER_PROTO_4_ADD_TPID_TYPE;
      ihp_parser_eth_protocols_tbl_data.eth_sap_protocol = 0;      
      res = arad_pp_ihp_parser_eth_protocols_tbl_set_unsafe(
              unit,
              ARAD_PARSER_ETHER_PROTO_4_ADD_TPID,
              &ihp_parser_eth_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 172, exit);

      /* {*/

      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_ADD_TPID, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 173, exit);

      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 0;           /* take next header */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 16;          /* mask anything else */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 4;           /* Compare 16 lsb to ethernet values */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0x7fff;      /* Compare all values */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 0;           /* If condition holds use shift-A else shift-B */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 4;           /* add 4 bytes to previous header size */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 4;           /* add 4 bytes to previous header size */
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 1;           /* don't advance header size or header qualifier */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 14;          /* change protocol to be new ethertype */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0x10;        /* set Outer-Tag bits 4:3 of the Ethernet-Tag-Format to 2 0b10 (TPID 2) */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0x7f7;       /* set Outer-Tag bits 4:3 of the Ethernet-Tag-Format to 2 0b10 (TPID 2) */
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0x798;       /* Allow to change protocol (bits 10:7) and outer-tag (bits 4:3) */

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 174, exit);

      /* } */
  }

  if(soc_property_get(unit, spn_PORT_RAW_MPLS_ENABLE, 0))
  {
      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_NULL, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

      /* NULL custom macro - advance header counter, but keep the offset {*/

      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0x780;       /*  */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0;           /*  */
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0;           /*  */

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 154, exit);

      /* NULL custom macro } */
  }

  if(SOC_DPP_CONFIG(unit)->pp.test2 == 1)
  {
      res = arad_parser_dynamic_macro_allocate(unit, ARAD_PARSER_DYNAMIC_MACRO_EoE, &custom_macro);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);

      /* New custom macro to identify EoE format; EoE-TPID = 0xE0E0 and 0xE0EC as two protocols. 
	  A custom macro will then process the TTL and EID */

      ihp_parser_custom_macro_parameters_tbl_data.cstm_word_select       = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_left         = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_mask_right        = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mode   = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask   = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_condition_select  = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_sel         = 0;           /* If condition holds use shift-A else shift-B */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_a           = 2;           /* add 2 bytes to previous header size */
      ihp_parser_custom_macro_parameters_tbl_data.cstm_shift_b           = 2;           /* add 2 bytes to previous header size */
      ihp_parser_custom_macro_parameters_tbl_data.augment_previous_stage = 1;           /* don't advance header size or header qualifier */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_shift        = 0;           /* */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_or_mask      = 0x480;       /* set packet format qualifier to mac in mac */
      ihp_parser_custom_macro_parameters_tbl_data.qualifier_and_mask     = 0x7f;       /* set packet format qualifier to mac in mac */
      ihp_parser_custom_macro_parameters_tbl_data.header_qualifier_mask  = 0x780;            /* */

      res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
            unit,
            custom_macro,
            &ihp_parser_custom_macro_parameters_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 154, exit);

      
  }

  

  /* Configure FCoE custom macros only if enabled */
  if(SOC_DPP_CONFIG(unit)->pp.fcoe_enable)
  {
      res = arad_parser_fcoe_programs_set(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 155, exit);
  }

  idx = 0;
  for (table_line = 0; table_line < (sizeof(Arad_parserr_info) + (2 * sizeof(Arad_parserr_info_tm))) / sizeof(ARAD_PARSERR_INFO); ++table_line)
  {
   if (table_line == sizeof(Arad_parserr_info) / sizeof(ARAD_PARSERR_INFO))
    {
      idx = 0;
    }

    /*
     *    First instructions
     */
    if (table_line < sizeof(Arad_parserr_info) / sizeof(ARAD_PARSERR_INFO))
    {
      arad_parser_static_to_dynamic_macro(
         unit,
         Arad_parser_macro_map,
         (sizeof(Arad_parser_macro_map) / sizeof(ARAD_PARSER_STATIC_TO_DYNAMIC_MACRO)),
         &(Arad_parserr_info[idx]),
         &macro_sel,
         &next_addr_base
         );

      instr_addr = Arad_parserr_info[idx].instruction_addr;

      ihp_parser_program_tbl_data.macro_sel = macro_sel;
      ihp_parser_program_tbl_data.next_addr_base = next_addr_base;

      if ((ihp_parser_program_tbl_data.next_addr_base != ARAD_PARSER_PROG_FIN) 
       && (ihp_parser_program_tbl_data.next_addr_base != ARAD_PARSER_PROG_END)) {
          ihp_parser_program_tbl_data.next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);
      }

      ihp_packet_format_table_tbl_data.packet_format_code = Arad_parserr_info[idx].packet_format_code;
      ihp_packet_format_table_tbl_data.parser_leaf_context = Arad_parserr_info[idx].parser_pmf_profile;
    }
    else
    {
      /*
       *    TM
       */
        /* Replicate twice the TM table: the TM custom macro output is in 7b and can take all the range */
        if (table_line == ((sizeof(Arad_parserr_info) + sizeof(Arad_parserr_info_tm)) / sizeof(ARAD_PARSERR_INFO))) {
            tm_second_iter = 1;
            idx = 0;
        }

        /* For Jericho ITMH format, different value for parser program memory
         */
        if (SOC_IS_JERICHO(unit)
            && !soc_property_get(unit, spn_ITMH_ARAD_MODE_ENABLE, 0)
            && !soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "itmh_jericho_parse_disable", 0)) 
        {
            instr_addr = Jericho_parserr_info_tm[idx].instruction_addr + (tm_second_iter * ARAD_PARSER_PROG_MULTIPLIER);
            ihp_parser_program_tbl_data.macro_sel = Jericho_parserr_info_tm[idx].macro_sel;
            ihp_parser_program_tbl_data.next_addr_base = Jericho_parserr_info_tm[idx].next_addr_base;
            if ((ihp_parser_program_tbl_data.next_addr_base != ARAD_PARSER_PROG_FIN) 
             && (ihp_parser_program_tbl_data.next_addr_base != ARAD_PARSER_PROG_END)) {
                ihp_parser_program_tbl_data.next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);
            }

            ihp_packet_format_table_tbl_data.packet_format_code = Jericho_parserr_info_tm[idx].packet_format_code;
            ihp_packet_format_table_tbl_data.parser_leaf_context = Jericho_parserr_info_tm[idx].parser_pmf_profile;
        }
        else
        {
          instr_addr = Arad_parserr_info_tm[idx].instruction_addr + (tm_second_iter * ARAD_PARSER_PROG_MULTIPLIER);
          ihp_parser_program_tbl_data.macro_sel = Arad_parserr_info_tm[idx].macro_sel;
          ihp_parser_program_tbl_data.next_addr_base = Arad_parserr_info_tm[idx].next_addr_base;
          if ((ihp_parser_program_tbl_data.next_addr_base != ARAD_PARSER_PROG_FIN) 
           && (ihp_parser_program_tbl_data.next_addr_base != ARAD_PARSER_PROG_END)) {
              ihp_parser_program_tbl_data.next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);
          }
          
          ihp_packet_format_table_tbl_data.packet_format_code = Arad_parserr_info_tm[idx].packet_format_code;
          ihp_packet_format_table_tbl_data.parser_leaf_context = Arad_parserr_info_tm[idx].parser_pmf_profile;
        }
    }

    for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index)
    {
      /* Parser program Replications */
      res = arad_pp_ihp_parser_program_tbl_set_unsafe(
              unit,
              array_index,
              instr_addr,
              &ihp_parser_program_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }
    
    res = arad_pp_ihp_packet_format_table_tbl_set_unsafe(
            unit,
            instr_addr,
            &ihp_packet_format_table_tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    idx ++;
  }

  /*
   * Ethernet replications
   */
  idx = 0;
  for (eth_rep_ndx = 0; eth_rep_ndx < ARAD_PARSER_NOF_ETH_REPLICATIONS; ++eth_rep_ndx)
  {
      uint32 arad_parser_packet_format_tbl_eth_indx = 0;

      for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index)
      {
          uint32 arad_parser_parser_program_tbl_eth_indx = 0;
          res = arad_fill_table_with_variable_values_by_caching(unit, IHP_PARSER_PROGRAMm, array_index, MEM_BLOCK_ANY, ARAD_PARSER_PROG_E_BASE_START, -1,
                  _arad_parser_program_tbl_eth_callback, &arad_parser_parser_program_tbl_eth_indx);
          if (res != SOC_E_NONE) {
            SOC_SAND_SET_ERROR_CODE(0, 350, exit);
          }
      }
      res = arad_fill_table_with_variable_values_by_caching(unit, IHP_PACKET_FORMAT_TABLEm, 0, MEM_BLOCK_ANY, ARAD_PARSER_PROG_E_BASE_START, -1,
              _arad_packet_format_tbl_eth_callback, &arad_parser_packet_format_tbl_eth_indx);
      if (res != SOC_E_NONE) {
        SOC_SAND_SET_ERROR_CODE(0, 350, exit);
      }
  }

  /* PTP (1588) {*/

  /* Configure PTP (1588) udp ports */
  ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_PTP_UDP_ENCAP_PORT1;
  res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
          unit,
          ARAD_PARSER_CUSTOM_MACRO_PROTO_10,
          &ihp_parser_custom_macro_protocols_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 155, exit);
 
  ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_PTP_UDP_ENCAP_PORT2;
  res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
          unit,
          ARAD_PARSER_CUSTOM_MACRO_PROTO_11,
          &ihp_parser_custom_macro_protocols_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 156, exit);

  /*bfd single hop*/
  ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_BFD_SINGLE_HOP_UDP_PORT;
  res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
          unit,
          ARAD_PARSER_CUSTOM_MACRO_PROTO_7,
          &ihp_parser_custom_macro_protocols_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 156, exit);

  ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_BFD_MICRO_UDP_PORT;
  res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
          unit,
          ARAD_PARSER_CUSTOM_MACRO_PROTO_9,
          &ihp_parser_custom_macro_protocols_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 156, exit);

  if (SOC_DPP_CONFIG(unit)->pp.limited_gtp_parsing) {
      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_GTP_U_UDP_PORT; 
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
               ARAD_PARSER_CUSTOM_MACRO_PROTO_14,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 156, exit);

      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ARAD_PP_GTP_C_UDP_PORT;
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
               ARAD_PARSER_CUSTOM_MACRO_PROTO_15,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 156, exit);

      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0x40000000; /* speculative parsing of IPV4 */ 
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
               ARAD_PARSER_CUSTOM_MACRO_PROTO_12,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 156, exit);

      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = 0x60000000; /* speculative parsing of IPV4 */
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
              unit,
               ARAD_PARSER_CUSTOM_MACRO_PROTO_13,
              &ihp_parser_custom_macro_protocols_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 156, exit);

  }

  /* PTP (1588) }*/

 /* 
   * Write the encoded ranges to the custom macro protocols table for destination info extension in case of ARAD header 
   * (OutLIF and MC-FLOW-ID) 
   */
      if (SOC_IS_ARADPLUS_AND_BELOW(unit)
          || soc_property_get(unit, spn_ITMH_ARAD_MODE_ENABLE, 0)
          || soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "itmh_jericho_parse_disable", 0))
      {

        /* OUTLIF */
        ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ((ARAD_ITMH_DEST_OUT_LIF_ID_PREFIX << 17) | 0x0);
        res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                unit,
                ARAD_PARSER_CUSTOM_MACRO_PROTO_3,
                &ihp_parser_custom_macro_protocols_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);
        ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ((ARAD_ITMH_DEST_OUT_LIF_ID_PREFIX << 17) | 0x1ffff);
        res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                unit,
                ARAD_PARSER_CUSTOM_MACRO_PROTO_4,
                &ihp_parser_custom_macro_protocols_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 162, exit);
        /* MC-FLOW-ID */
        ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ((ARAD_ITMH_DEST_MC_FLOW_ID_PREFIX << 16) | 0x0);
        res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                unit,
                ARAD_PARSER_CUSTOM_MACRO_PROTO_5,
                &ihp_parser_custom_macro_protocols_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 164, exit);
        ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ((ARAD_ITMH_DEST_MC_FLOW_ID_PREFIX << 16) | 0xffff);
        res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
                unit,
                ARAD_PARSER_CUSTOM_MACRO_PROTO_6,
                &ihp_parser_custom_macro_protocols_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 166, exit);

        /* In case of Arad, the Ingress Shaping always enabled */
        res = arad_parser_ingress_shape_state_set(
                unit,
                TRUE,
                0, /* q_low */
                0x1FFFF /*q_high*/
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 168, exit);
      }

  /* Update Custom Ether-types */
  /* Coupling */
  ihp_parser_eth_protocols_tbl_data.eth_type_protocol     = ARAD_PARSER_ETHER_PROTO_7_MPLS_MC_ETHER_TYPE;
  ihp_parser_eth_protocols_tbl_data.eth_sap_protocol      = ARAD_PARSER_ETHER_PROTO_7_MPLS_MC_ETHER_TYPE;
  res = arad_pp_ihp_parser_eth_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_ETHER_PROTO_7_MPLS_MC,
            &ihp_parser_eth_protocols_tbl_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);  


  /* EoE */
  ihp_parser_eth_protocols_tbl_data.eth_type_protocol     = ARAD_PARSER_ETHER_PROTO_3_EoE_TPID_TYPE;
  ihp_parser_eth_protocols_tbl_data.eth_sap_protocol      = ARAD_PARSER_ETHER_PROTO_3_EoE_TPID_TYPE; 
  res = arad_pp_ihp_parser_eth_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_ETHER_PROTO_3_EoE,
            &ihp_parser_eth_protocols_tbl_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit); 
  
  ihp_parser_eth_protocols_tbl_data.eth_type_protocol     = ARAD_PARSER_ETHER_PROTO_5_EoE_TPID_TYPE;
  ihp_parser_eth_protocols_tbl_data.eth_sap_protocol      = ARAD_PARSER_ETHER_PROTO_5_EoE_TPID_TYPE; 
  res = arad_pp_ihp_parser_eth_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_ETHER_PROTO_5_EoE,
            &ihp_parser_eth_protocols_tbl_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);  


  /* PTP (1588) {*/

  /* 1588 Ether-type */
  res = arad_pp_ihp_parser_eth_protocols_tbl_get_unsafe(unit,
                                                        ARAD_PARSER_ETHER_PROTO_6_1588,
                                                        &ihp_parser_eth_protocols_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 210, exit);

  ihp_parser_eth_protocols_tbl_data.eth_type_protocol = ARAD_PARSER_ETHER_PROTO_6_1588_ETHER_TYPE;

  res = arad_pp_ihp_parser_eth_protocols_tbl_set_unsafe(unit,
                                                        ARAD_PARSER_ETHER_PROTO_6_1588,
                                                        &ihp_parser_eth_protocols_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit);

  /* PTP (1588) }*/


  /* PTP (1588) {*/

  /* mapping between the value of protocol encapsulator with mpls nibble value and the protocol enumerator */

  /* when 'mpls encapsulated' protocol nibble is 4 the protocol enum is 13 */
  soc_IHP_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_MAPm_field32_set(unit, &tbl_data, NEXT_PROTOCOLf, ARAD_PP_L3_NEXT_PRTCL_NDX_IPV4);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 230, exit, WRITE_IHP_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_MAPm(unit, 
                                                                                                  MEM_BLOCK_ANY, 
                                                                                                  /* entry index */ ARAD_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_5, 
                                                                                                  &tbl_data));

  /* when 'mpls encapsulated' protocol nibble is 6 the protocol enum is 14 */
  soc_IHP_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_MAPm_field32_set(unit, &tbl_data, NEXT_PROTOCOLf, ARAD_PP_L3_NEXT_PRTCL_NDX_IPV6);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 230, exit, WRITE_IHP_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_MAPm(unit, 
                                                                                                  MEM_BLOCK_ANY, 
                                                                                                  /* entry index */ ARAD_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_7,
                                                                                                  &tbl_data));

  /* PTP (1588) }*/


  /*bfd ipv4 single hop*/
  if (SOC_DPP_CONFIG(unit)->pp.bfd_ipv4_single_hop){

      instr_addr = ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_7 + 1) << 1); /* PROTOCOL_7  (BFD single hop) << 1*/
      res = arad_pp_ihp_packet_format_table_tbl_get_unsafe(unit,
                                                           instr_addr,
                                                           &ihp_packet_format_table_tbl_data);
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      ihp_packet_format_table_tbl_data.parser_leaf_context = ARAD_PARSER_PLC_BFD_SINGLE_HOP;

      res = arad_pp_ihp_packet_format_table_tbl_set_unsafe(
          unit,
          instr_addr,
          &ihp_packet_format_table_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);


      if (SOC_IS_JERICHO(unit)) {
          instr_addr = ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_9 + 1) << 1); /* PROTOCOL_9  (micro BFD) << 1*/
          res = arad_pp_ihp_packet_format_table_tbl_get_unsafe(unit,
                                                               instr_addr,
                                                               &ihp_packet_format_table_tbl_data);
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

          ihp_packet_format_table_tbl_data.parser_leaf_context = ARAD_PARSER_PLC_BFD_SINGLE_HOP;

          res = arad_pp_ihp_packet_format_table_tbl_set_unsafe(
              unit,
              instr_addr,
              &ihp_packet_format_table_tbl_data
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      }
  }

  if ((SOC_DPP_CONFIG(unit)->pp.bfd_ipv6_enable == SOC_DPP_ARAD_BFD_IPV6_SUPPORT_WITH_LEM)) {
      instr_addr = ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_7 + 1) << 1); /* PROTOCOL_9  (micro BFD) << 1*/
      res = arad_pp_ihp_packet_format_table_tbl_get_unsafe(unit,
                                                           instr_addr,
                                                           &ihp_packet_format_table_tbl_data);
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      ihp_packet_format_table_tbl_data.parser_leaf_context = ARAD_PARSER_PLC_BFD_SINGLE_HOP;

      res = arad_pp_ihp_packet_format_table_tbl_set_unsafe(
          unit,
          instr_addr,
          &ihp_packet_format_table_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      if (SOC_IS_JERICHO(unit)) {
          instr_addr = ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_9 + 1) << 1); /* PROTOCOL_9  (micro BFD) << 1*/
          res = arad_pp_ihp_packet_format_table_tbl_get_unsafe(unit,
                                                               instr_addr,
                                                               &ihp_packet_format_table_tbl_data);
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

          ihp_packet_format_table_tbl_data.parser_leaf_context = ARAD_PARSER_PLC_BFD_SINGLE_HOP;

          res = arad_pp_ihp_packet_format_table_tbl_set_unsafe(
              unit,
              instr_addr,
              &ihp_packet_format_table_tbl_data
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      }

  }

  if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "mpls_ipv4_parse_disable", 0)) {
      /* All instruction addresses for MPLS protocol */
      uint32 mpls_ipv4_instr_addr[4] = {    ARAD_PARSER_PROG_IPV4oE_BASE_START + 0X1e, 
                                            ARAD_PARSER_PROG_IPV4oE_BASE_START + 0X1f,
                                            ARAD_PARSER_PROG_IPV4oE_BASE_START + 0X3e, 
                                            ARAD_PARSER_PROG_IPV4oE_BASE_START + 0X3f   };
      uint32 instr_addr_index;

      /* Don't run MPLS MACRO */
      ihp_parser_program_tbl_data.macro_sel = ARAD_PARSER_DONT_CARE;
      ihp_parser_program_tbl_data.next_addr_base = ARAD_PARSER_PROG_END;

      /* Update all relevant parser program instructions (and replications) */
      for (instr_addr_index = 0; instr_addr_index < 4 ; instr_addr_index++) { 
          for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index)
          {
              /* Parser program Replications */
              res = arad_pp_ihp_parser_program_tbl_set_unsafe(
                      unit,
                      array_index,
                      mpls_ipv4_instr_addr[instr_addr_index],
                      &ihp_parser_program_tbl_data
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
          }
      }
  }

  if (SOC_IS_JERICHO(unit)) {
      uint32 next_protocol, is_ipv4, entry;

      /* Configure Second stage parsing (0-8 are UserDefined) */
      for (next_protocol=0; next_protocol<ARAD_PARSER_MPLS_NEXT_PROTOCOL_NOF; next_protocol++) {

          /* IPv4/6 Next protocol size */
          for (is_ipv4=0; is_ipv4<=1; is_ipv4++) {
              entry = (is_ipv4<<4) | next_protocol;
              
              SOC_SAND_SOC_IF_ERROR_RETURN(res, 230, exit, READ_IHP_VTT_IP_PACKET_FORMAT_ATTRIBUTES_TABLEm(unit, MEM_BLOCK_ANY, entry, &tbl_data));
              if (next_protocol==ARAD_PARSER_MPLS_NEXT_PROTOCOL_TCP || next_protocol==ARAD_PARSER_MPLS_NEXT_PROTOCOL_UDP) { 
                  reg_val = 8; /* Size of TCP and UDP headers */
              }
              else {
                  reg_val = 0; /* Not supported */
              }
              soc_IHP_VTT_IP_PACKET_FORMAT_ATTRIBUTES_TABLEm_field_set(unit, &tbl_data, NEXT_PROTOCOL_LENGTHf, &reg_val);

              if (next_protocol==ARAD_PARSER_MPLS_NEXT_PROTOCOL_TCP || next_protocol==ARAD_PARSER_MPLS_NEXT_PROTOCOL_UDP) {
                  reg_val = ARAD_PARSER_PLC_PP_L4;
              }
              else {
                  reg_val = ARAD_PARSER_PLC_PP;
              }
              soc_IHP_VTT_IP_PACKET_FORMAT_ATTRIBUTES_TABLEm_field_set(unit, &tbl_data, PARSER_LEAF_CONTEXTf, &reg_val);

              reg_val = is_ipv4 ? ARAD_PARSER_PFC_IPV4_MPLS1_ETH : ARAD_PARSER_PFC_IPV6_MPLS1_ETH;
              soc_IHP_VTT_IP_PACKET_FORMAT_ATTRIBUTES_TABLEm_field_set(unit, &tbl_data, PACKET_FORMAT_CODEf, &reg_val);

              SOC_SAND_SOC_IF_ERROR_RETURN(res, 230, exit, WRITE_IHP_VTT_IP_PACKET_FORMAT_ATTRIBUTES_TABLEm(unit, MEM_BLOCK_ANY, entry, &tbl_data));
          }
          /* Ethernet Next protocol size */
          SOC_SAND_SOC_IF_ERROR_RETURN(res, 230, exit, READ_IHP_VTT_ETH_PACKET_FORMAT_ATTRIBUTES_TABLEm(unit, MEM_BLOCK_ANY, next_protocol, &tbl_data));
          if (next_protocol==ARAD_PARSER_MPLS_NEXT_PROTOCOL_TCP || next_protocol==ARAD_PARSER_MPLS_NEXT_PROTOCOL_UDP) {
              reg_val = ARAD_PARSER_PLC_PP_L4;
          }
          else {
              reg_val = ARAD_PARSER_PLC_PP;
          }
          soc_IHP_VTT_ETH_PACKET_FORMAT_ATTRIBUTES_TABLEm_field_set(unit, &tbl_data, PARSER_LEAF_CONTEXTf, &reg_val);

          reg_val = ARAD_PARSER_PFC_ETH_MPLS1_ETH;
          soc_IHP_VTT_ETH_PACKET_FORMAT_ATTRIBUTES_TABLEm_field_set(unit, &tbl_data, PACKET_FORMAT_CODEf, &reg_val);

          reg_val=0;
          soc_IHP_VTT_ETH_PACKET_FORMAT_ATTRIBUTES_TABLEm_field_set(unit, &tbl_data, NEXT_PROTOCOL_LENGTHf, &reg_val);
          SOC_SAND_SOC_IF_ERROR_RETURN(res, 230, exit, WRITE_IHP_VTT_ETH_PACKET_FORMAT_ATTRIBUTES_TABLEm(unit, MEM_BLOCK_ANY, next_protocol, &tbl_data));

      }

  }

  if (SOC_DPP_CONFIG(unit)->pp.limited_gtp_parsing) {

      if (SOC_DPP_CONFIG(unit)->pp.fcoe_enable) {
          /* mutually exclusive feature, using same resources */
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_ERR, 77, exit);
      }

      if (SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & SOC_DPP_IP_TUNNEL_TERM_DB_VXLAN) {
          /* curently we don't support both vxlan and limited gtp parsing on same device */
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_ERR, 78, exit);
      }

      if (SOC_DPP_CONFIG(unit)->pp.udp_tunnel_enable) {
          /* mutually exclusive feature, using same resources */
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_ERR, 79, exit);
      }

      instr_addr = ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_14 + 1) << 1); 

      ihp_parser_program_tbl_data.macro_sel = dynamic_macro[unit][ARAD_PARSER_DYNAMIC_MACRO_GTP]/* something to do with parsing GTP */;
      ihp_parser_program_tbl_data.next_addr_base = ARAD_PARSER_PROG_GTP_UDPoIPV4oE_BASE;
      ihp_parser_program_tbl_data.next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);

      for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index) {

          /* Parser program Replications */
          res = arad_pp_ihp_parser_program_tbl_set_unsafe(
              unit,
              array_index,
              instr_addr,
              &ihp_parser_program_tbl_data
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
      }

      instr_addr = ARAD_PARSER_PROG_UDPoIPV4oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_15 + 1) << 1);
      ihp_parser_program_tbl_data.macro_sel = dynamic_macro[unit][ARAD_PARSER_DYNAMIC_MACRO_GTP]/* something to do with parsing GTP */;
      ihp_parser_program_tbl_data.next_addr_base = ARAD_PARSER_PROG_GTP_UDPoIPV4oE_BASE;
      ihp_parser_program_tbl_data.next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);

      for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index) {

          /* Parser program Replications */
          res = arad_pp_ihp_parser_program_tbl_set_unsafe(
              unit,
              array_index,
              instr_addr,
              &ihp_parser_program_tbl_data
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
      }

      instr_addr = ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_14 + 1) << 1);
      ihp_parser_program_tbl_data.macro_sel = dynamic_macro[unit][ARAD_PARSER_DYNAMIC_MACRO_GTP]/* something to do with parsing GTP */;
      ihp_parser_program_tbl_data.next_addr_base = ARAD_PARSER_PROG_GTP_UDPoIPV6oE_BASE;
      ihp_parser_program_tbl_data.next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);

      for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index) {

          /* Parser program Replications */
          res = arad_pp_ihp_parser_program_tbl_set_unsafe(
              unit,
              array_index,
              instr_addr,
              &ihp_parser_program_tbl_data
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
      }

      instr_addr = ARAD_PARSER_PROG_UDPoIPV6oE_BASE_START + ((ARAD_PARSER_CUSTOM_MACRO_PROTO_15 + 1) << 1);
      ihp_parser_program_tbl_data.macro_sel = dynamic_macro[unit][ARAD_PARSER_DYNAMIC_MACRO_GTP]/* something to do with parsing GTP */;
      ihp_parser_program_tbl_data.next_addr_base = ARAD_PARSER_PROG_GTP_UDPoIPV6oE_BASE;
      ihp_parser_program_tbl_data.next_addr_base *= (ARAD_PARSER_PROG_MULTIPLIER / ARAD_PARSER_PROG_BASE_OFFSET_RESOLUTION);

      for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index) {

          /* Parser program Replications */
          res = arad_pp_ihp_parser_program_tbl_set_unsafe(
              unit,
              array_index,
              instr_addr,
              &ihp_parser_program_tbl_data
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
      }

      /* create ARAD_PARSER_PROG_GTP_UDPoIPV4oE_BASE_START, ARAD_PARSER_PROG_GTP_UDPoIPV6oE_BASE_START
         tables
      */
      for (application = 0; application < 2; application++) {


          ihp_parser_program_tbl_data.macro_sel = ARAD_PARSER_DONT_CARE;
          ihp_parser_program_tbl_data.next_addr_base = ARAD_PARSER_PROG_END;

          application_start = application ? ARAD_PARSER_PROG_GTP_UDPoIPV6oE_BASE_START : ARAD_PARSER_PROG_GTP_UDPoIPV4oE_BASE_START;

          for (array_index = 0; array_index < ARAD_PARSER_NOF_PARSER_PROGRAMS_REPLICATIONS; ++array_index) {
              for (instr_addr_index = 0; instr_addr_index < 64; instr_addr_index++) {

                  /* Parser program Replications */
                  res = arad_pp_ihp_parser_program_tbl_set_unsafe(
                      unit,
                      array_index,
                      application_start + instr_addr_index,
                      &ihp_parser_program_tbl_data
                      );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
              }
          }

          for (instr_addr_index = 0; instr_addr_index < 64; instr_addr_index++) {

              ihp_packet_format_table_tbl_data.parser_leaf_context = ARAD_PARSER_PLC_PP_L4;
              ihp_packet_format_table_tbl_data.packet_format_code = application ? ARAD_PARSER_PFC_IPV6_ETH : ARAD_PARSER_PFC_IPV4_ETH;

              if (instr_addr_index == ((ARAD_PARSER_CUSTOM_MACRO_PROTO_12 + 1) << 1)) {
                  ihp_packet_format_table_tbl_data.parser_leaf_context = ARAD_PARSER_PLC_IP_UDP_GTP1;
              }

              if (instr_addr_index == ((ARAD_PARSER_CUSTOM_MACRO_PROTO_13 + 1) << 1)) {
                  ihp_packet_format_table_tbl_data.parser_leaf_context = ARAD_PARSER_PLC_IP_UDP_GTP2;
              }

              res = arad_pp_ihp_packet_format_table_tbl_set_unsafe(
                  unit,
                  application_start + instr_addr_index,
                  &ihp_packet_format_table_tbl_data
                  );
              SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
          }

      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_init()", 0, 0);

}


/* use to update cfg attributes of the vxlan program, e.g. upd-dest port */
uint32
arad_parser_vxlan_program_info_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN uint16 udp_dest_port
 )
{
    ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_DATA
      ihp_parser_custom_macro_protocols_tbl_data;
    uint32 res = SOC_SAND_OK;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = udp_dest_port;
    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_8,
            &ihp_parser_custom_macro_protocols_tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_vxlan_program_info_set()", 0, 0);
}


uint32
arad_parser_vxlan_program_info_get(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT uint16 *udp_dest_port
 )
{
    ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_DATA
      ihp_parser_custom_macro_protocols_tbl_data;
    uint32 res = SOC_SAND_OK;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_get_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_8,
            &ihp_parser_custom_macro_protocols_tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    *udp_dest_port = (uint16)ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_vxlan_program_get()", 0, 0);
}


uint32
  arad_parser_ingress_shape_state_set(
    SOC_SAND_IN int                                 unit,
    SOC_SAND_IN uint8                                 enable,
    SOC_SAND_IN uint32                                  q_low,
    SOC_SAND_IN uint32                                  q_high
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PARAMETERS_TBL_DATA
    ihp_parser_custom_macro_parameters_tbl_data;
  ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_DATA
    ihp_parser_custom_macro_protocols_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PARSER_INGRESS_SHAPE_STATE_SET);

    /* ARAD mode */
    /*
     * Write the encoded ingress shaping queue prefix to the custom macro protocols table
     */   
    ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ((ARAD_ITMH_DEST_ISQ_FLOW_ID_PREFIX << 17) | q_low); 
    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_1,
            &ihp_parser_custom_macro_protocols_tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = ((ARAD_ITMH_DEST_ISQ_FLOW_ID_PREFIX << 17) | q_high);
    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_2,
            &ihp_parser_custom_macro_protocols_tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    /* Exit for Arad: always enabled */
    ARAD_DO_NOTHING_AND_EXIT;
  /* TM custom protocols for ingress shaping }*/

  if (enable)
  {
    /* Mask also Ingress shaping protocol in custom macro 0 (IS present)*/
    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_get_unsafe(
          unit,
          ARAD_PARSER_MACRO_SEL_TM_BASE + ARAD_PARSER_MACRO_SEL_0,
          &ihp_parser_custom_macro_parameters_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask |= 0x3; /* Use also protocols 1,2 */

    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
          unit,
          ARAD_PARSER_MACRO_SEL_TM_BASE + ARAD_PARSER_MACRO_SEL_0,
          &ihp_parser_custom_macro_parameters_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);  
  }
  else
  {
    /* Remove Ingress shaping protocols in custom macro 0 (IS present)*/
    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_get_unsafe(
          unit,
          ARAD_PARSER_MACRO_SEL_TM_BASE + ARAD_PARSER_MACRO_SEL_0,
          &ihp_parser_custom_macro_parameters_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    ihp_parser_custom_macro_parameters_tbl_data.cstm_comparator_mask &= 0xFFFFFFFC; /* Remove protocols 1,2 */

    res = arad_pp_ihp_parser_custom_macro_parameters_tbl_set_unsafe(
          unit,
          ARAD_PARSER_MACRO_SEL_TM_BASE + ARAD_PARSER_MACRO_SEL_0,
          &ihp_parser_custom_macro_parameters_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 65, exit);     
  }


 
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_ingress_shaping_enable_set()", enable, 0);
}

int
  arad_parser_nof_bytes_to_remove_set(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN int         core,
    SOC_SAND_IN uint32      tm_port,
    SOC_SAND_IN uint32      nof_bytes_to_skip
  )
{
    uint32 reg_val;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_IHP_PTC_INFOm(unit, IHP_BLOCK(unit, core), tm_port, &reg_val));
    soc_IHP_PTC_INFOm_field32_set(unit, &reg_val, PTC_OUTER_HEADER_STARTf, nof_bytes_to_skip);
    SOCDNX_IF_ERR_EXIT(WRITE_IHP_PTC_INFOm(unit, IHP_BLOCK(unit, core), tm_port, &reg_val));

exit:
    SOCDNX_FUNC_RETURN;
}

int
  arad_parser_nof_bytes_to_remove_get(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN int         core,
    SOC_SAND_IN uint32      tm_port,
    SOC_SAND_OUT uint32     *nof_bytes_to_skip
  )
{
    uint32 reg_val;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_IHP_PTC_INFOm(unit, IHP_BLOCK(unit, core), tm_port, &reg_val));
    *nof_bytes_to_skip = soc_IHP_PTC_INFOm_field32_get(unit, &reg_val, PTC_OUTER_HEADER_STARTf);

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
  arad_parser_pp_port_nof_bytes_to_remove_set(
    SOC_SAND_IN int      unit,
    SOC_SAND_IN int         core,
    SOC_SAND_IN uint32      pp_port_ndx,
    SOC_SAND_IN uint32      nof_bytes_to_skip
  )
{
  uint32
    res = SOC_SAND_OK,
    reg_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHP_PP_PORT_INFOm(unit, IHP_BLOCK(unit, core), pp_port_ndx, &reg_val));
  soc_IHP_PP_PORT_INFOm_field32_set(unit, &reg_val, PP_PORT_OUTER_HEADER_STARTf, nof_bytes_to_skip);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHP_PP_PORT_INFOm(unit, IHP_BLOCK(unit, core), pp_port_ndx, &reg_val));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_pp_port_nof_bytes_to_remove_set()", pp_port_ndx, 0);
}

/*
 * arad_parser_udp_tunnel_dst_port_set
 * the function is called from bcm_petra_switch_control_set
 * to change udp tunnel destination ports during runtime
 */
uint32
  arad_parser_udp_tunnel_dst_port_set(
    SOC_SAND_IN int                                      unit,
    SOC_SAND_IN ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE   udp_dst_port_type,
    SOC_SAND_IN int                                      udp_dst_port_val
  )
{
  uint32
    res = SOC_SAND_OK;

  ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_DATA
    ihp_parser_custom_macro_protocols_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  switch (udp_dst_port_type)
  {
  case ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_IPV4:
    ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = udp_dst_port_val;   /*ipv4*/
    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
          unit,
          ARAD_PARSER_CUSTOM_MACRO_PROTO_13,
          &ihp_parser_custom_macro_protocols_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 252, exit);
    break;
  case ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_IPV6:
      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = udp_dst_port_val;   /*ipv6*/
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_14,
            &ihp_parser_custom_macro_protocols_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 253, exit);
    break;
  case ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_MPLS:
      ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol = udp_dst_port_val;   /*MPLS*/
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_set_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_15,
            &ihp_parser_custom_macro_protocols_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 254, exit);
    break;
  default:
     res = BCM_E_PARAM;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_udp_tunnel_dst_port_set()", 0, 0);
}


/*
 * arad_parser_udp_tunnel_dst_port_get
 * the function is called from bcm_petra_switch_control_get
 * to read udp tunnel destination ports during runtime
 */
uint32
arad_parser_udp_tunnel_dst_port_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE  udp_dst_port_type,
    SOC_SAND_OUT int                                     *udp_dst_port_val
  )
{
  uint32
    res = SOC_SAND_OK;

  ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_DATA
    ihp_parser_custom_macro_protocols_tbl_data;

 SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  switch (udp_dst_port_type)
  {
  case ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_IPV4:
    res = arad_pp_ihp_parser_custom_macro_protocols_tbl_get_unsafe(
          unit,
          ARAD_PARSER_CUSTOM_MACRO_PROTO_13, /*ipv4*/
          &ihp_parser_custom_macro_protocols_tbl_data
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 252, exit);
    break;
  case ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_IPV6:
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_get_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_14,  /*ipv6*/
            &ihp_parser_custom_macro_protocols_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 253, exit);
    break;
  case ARAD_PARSER_UDP_TUNNEL_NEXT_PRTCL_TYPE_MPLS:
      res = arad_pp_ihp_parser_custom_macro_protocols_tbl_get_unsafe(
            unit,
            ARAD_PARSER_CUSTOM_MACRO_PROTO_15,  /*MPLS*/
            &ihp_parser_custom_macro_protocols_tbl_data
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 254, exit);
    break;
  default:
     res = BCM_E_PARAM;
  }

  *udp_dst_port_val = (uint16)ihp_parser_custom_macro_protocols_tbl_data.cstm_protocol;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_parser_udp_tunnel_dst_port_get()", 0, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>
