/* $Id: jer2_jer2_jer2_tmc_pmf_pgm_mgmt.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_PMF_PGM_MGMT_INCLUDED__
/* { */
#define __DNX_TMC_PMF_PGM_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ports.h>

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

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  The PMF Program addition / removal is triggered by the
   *  PP-Port.
   */
  DNX_TMC_PMF_PGM_MGMT_SOURCE_PP_PORT = 0,
  /*
   *  The PMF Program addition / removal is triggered by the
   *  FP.
   */
  DNX_TMC_PMF_PGM_MGMT_SOURCE_FP = 1,
  /*
   *  Number of types in DNX_TMC_PMF_PGM_MGMT_SOURCE
   */
  DNX_TMC_PMF_NOF_PGM_MGMT_SOURCES = 2
}DNX_TMC_PMF_PGM_MGMT_SOURCE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then the configuration must be set. If False,
   *  only verify there is enough resource for the new
   *  configuration.
   */
  uint8 is_to_set;
  /*
   *  If True, then the Database is created. Otherwise, the
   *  Database is deleted.
   */
  uint8 is_addition;
  /* 
   * If True, second iteration (in case of failure) 
   */
  uint8 is_2nd_iter;

} DNX_TMC_PMF_PGM_MGMT_INFO;

typedef enum
{
  /*
   *  Raw packet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_RAW = 0x1,
  /*
   *  Packet with first header FTMH (e.g., for stacking ports)
   *  and possibly with a PPH afterwards.
   */
  DNX_TMC_FP_PKT_HDR_TYPE_FTMH = 0x2,
  /*
   *  Only ITMH as parsed header
   */
  DNX_TMC_FP_PKT_HDR_TYPE_TM = 0x4,
  /*
   *  Ingress Shaping + ITMH headers
   */
  DNX_TMC_FP_PKT_HDR_TYPE_TM_IS = 0x8,
  /*
   *  ITMH + PPH headers
   */
  DNX_TMC_FP_PKT_HDR_TYPE_TM_PPH = 0x10,
  /*
   *  Ingress Shaping + ITMH + PPH headers
   */
  DNX_TMC_FP_PKT_HDR_TYPE_TM_IS_PPH = 0x20,
  /*
   *  Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_ETH = 0x40,
  /*
   *  MAC-in-MAC
   */
  DNX_TMC_FP_PKT_HDR_TYPE_ETH_ETH = 0x80,
  /*
   *  IPv4 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV4_ETH = 0x100,
  /*
   *  IPv6 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV6_ETH = 0x200,
  /*
   *  MPLS over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_MPLS1_ETH = 0x400,
  /*
   *  MPLS x 2 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_MPLS2_ETH = 0x800,
  /*
   *  MPLS x 3 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_MPLS3_ETH = 0x1000,
  /*
   *  Ethernet over MPLS over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_ETH_MPLS1_ETH = 0x2000,
  /*
   *  Ethernet over MPLS x 2 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_ETH_MPLS2_ETH = 0x4000,
  /*
   *  Ethernet over MPLS x 3 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_ETH_MPLS3_ETH = 0x8000,
  /*
   *  IPv4 over MPLS over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS1_ETH = 0x10000,
  /*
   *  IPv4 over MPLS x 2 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS2_ETH = 0x20000,
  /*
   *  IPv4 over MPLS x 3 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS3_ETH = 0x40000,
  /*
   *  IPv6 over MPLS over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS1_ETH = 0x80000,
  /*
   *  IPv6 over MPLS x 2 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS2_ETH = 0x100000,
  /*
   *  IPv6 over MPLS x 3 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS3_ETH = 0x200000,
  /*
   *  IPv4 over IPv4 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV4_IPV4_ETH = 0x400000,
  /*
   *  IPv6 over IPv4 over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_IPV6_IPV4_ETH = 0x800000,
  /*
   *  Ethernet over Trill over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_ETH_TRILL_ETH = 0x1000000,
  /*
   *  Ethernet over IP over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_ETH_IPV4_ETH = 0x2000000,
  /*
   *  Fiber Channel over Ethernet
   */
  DNX_TMC_FP_PKT_HDR_TYPE_FC_ETH = 0x4000000,
  /*
   *  Number of types in DNX_TMC_FP_PKT_HDR_TYPE
   */
  DNX_TMC_NOF_FP_PKT_HDR_TYPES = 25
}DNX_TMC_FP_PKT_HDR_TYPE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  PP-Port index. Range: 0 - 63.
   */
  uint32 pp_port_ndx;
  /*
   *  Packet-Format-Group (relevant if Header type is
   *  Ethernet) given by the FP. Range: 0 - 7.
   */
  uint32 pfg_ndx;

} DNX_TMC_PMF_PGM_MGMT_NDX;


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
  DNX_TMC_PMF_PGM_MGMT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PMF_PGM_MGMT_INFO *info
  );

void
  DNX_TMC_PMF_PGM_MGMT_NDX_clear(
    DNX_SAND_OUT DNX_TMC_PMF_PGM_MGMT_NDX *info
  );

#if DNX_TMC_DEBUG_IS_LVL1
const char*
  DNX_TMC_FP_PKT_HDR_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_FP_PKT_HDR_TYPE enum_val
  );

const char*
  DNX_TMC_PMF_PGM_MGMT_SOURCE_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_PGM_MGMT_SOURCE enum_val
  );

void
  DNX_TMC_PMF_PGM_MGMT_INFO_print(
    DNX_SAND_IN  DNX_TMC_PMF_PGM_MGMT_INFO *info
  );

void
  DNX_TMC_PMF_PGM_MGMT_NDX_print(
    DNX_SAND_IN  DNX_TMC_PMF_PGM_MGMT_NDX *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_PMF_PGM_MGMT_INCLUDED__*/
#endif

