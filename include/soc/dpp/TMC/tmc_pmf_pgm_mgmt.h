/* $Id: tmc_pmf_pgm_mgmt.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_PMF_PGM_MGMT_INCLUDED__
/* { */
#define __SOC_TMC_PMF_PGM_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_u64.h>

#include <soc/dpp/TMC/tmc_api_general.h>
#include <soc/dpp/TMC/tmc_api_ports.h>

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
   *  Raw packet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_RAW = 0x1,
  /*
   *  Packet with first header FTMH (e.g., for stacking ports)
   *  and possibly with a PPH afterwards.
   */
  SOC_TMC_FP_PKT_HDR_TYPE_FTMH = 0x2,
  /*
   *  Only ITMH as parsed header
   */
  SOC_TMC_FP_PKT_HDR_TYPE_TM = 0x4,
  /*
   *  Ingress Shaping + ITMH headers
   */
  SOC_TMC_FP_PKT_HDR_TYPE_TM_IS = 0x8,
  /*
   *  ITMH + PPH headers
   */
  SOC_TMC_FP_PKT_HDR_TYPE_TM_PPH = 0x10,
  /*
   *  Ingress Shaping + ITMH + PPH headers
   */
  SOC_TMC_FP_PKT_HDR_TYPE_TM_IS_PPH = 0x20,
  /*
   *  Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_ETH = 0x40,
  /*
   *  MAC-in-MAC
   */
  SOC_TMC_FP_PKT_HDR_TYPE_ETH_ETH = 0x80,
  /*
   *  IPv4 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV4_ETH = 0x100,
  /*
   *  IPv6 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV6_ETH = 0x200,
  /*
   *  MPLS over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_MPLS1_ETH = 0x400,
  /*
   *  MPLS x 2 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_MPLS2_ETH = 0x800,
  /*
   *  MPLS x 3 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_MPLS3_ETH = 0x1000,
  /*
   *  Ethernet over MPLS over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_ETH_MPLS1_ETH = 0x2000,
  /*
   *  Ethernet over MPLS x 2 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_ETH_MPLS2_ETH = 0x4000,
  /*
   *  Ethernet over MPLS x 3 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_ETH_MPLS3_ETH = 0x8000,
  /*
   *  IPv4 over MPLS over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS1_ETH = 0x10000,
  /*
   *  IPv4 over MPLS x 2 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS2_ETH = 0x20000,
  /*
   *  IPv4 over MPLS x 3 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV4_MPLS3_ETH = 0x40000,
  /*
   *  IPv6 over MPLS over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS1_ETH = 0x80000,
  /*
   *  IPv6 over MPLS x 2 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS2_ETH = 0x100000,
  /*
   *  IPv6 over MPLS x 3 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV6_MPLS3_ETH = 0x200000,
  /*
   *  IPv4 over IPv4 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV4_IPV4_ETH = 0x400000,
  /*
   *  IPv6 over IPv4 over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_IPV6_IPV4_ETH = 0x800000,
  /*
   *  Ethernet over Trill over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_ETH_TRILL_ETH = 0x1000000,
  /*
   *  Ethernet over IP over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_ETH_IPV4_ETH = 0x2000000,
  /*
   *  Fiber Channel over Ethernet
   */
  SOC_TMC_FP_PKT_HDR_TYPE_FC_ETH = 0x4000000,
  /*
   *  Number of types in SOC_TMC_FP_PKT_HDR_TYPE
   */
  SOC_TMC_NOF_FP_PKT_HDR_TYPES = 25
}SOC_TMC_FP_PKT_HDR_TYPE;

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

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_PMF_PGM_MGMT_INCLUDED__*/
#endif

