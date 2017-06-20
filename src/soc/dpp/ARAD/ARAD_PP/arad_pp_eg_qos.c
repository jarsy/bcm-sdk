
#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_eg_qos.c,v 1.34 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/mem.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_qos.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/JER/jer_ingress_traffic_mgmt.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/drv.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_EG_QOS_DSCP_EXP_MAX                              (255)
#define ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX                           (255)
#define ARAD_PP_EG_QOS_EXP_MAP_PROFILE_MAX                       (3)
#define ARAD_PP_EG_QOS_PHP_TYPE_MAX                              (SOC_PPC_NOF_EG_QOS_UNIFORM_PHP_TYPES-1)
#define ARAD_PP_EG_QOS_DP_MAX                                    (3)
#define ARAD_PP_EG_QOS_IPV4_OUT_EXP_NOF_BITS                     (8)
#define ARAD_PP_EG_QOS_IPV6_OUT_EXP_NOF_BITS                     (8)
#define ARAD_PP_EPNI_REMARK_PROFILE_NOF_BITS                     (4)
#define ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS                    (8)
#define ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS                     (8)
#define ARAD_PP_EG_QOS_NOF_REMARK_PROFILES                       (16)
#define ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX                  (ARAD_PP_EG_QOS_NOF_REMARK_PROFILES-1)
#define ARAD_PP_EG_QOS_ETH_ECN_REMARK_PROFILE_INDEX_MAX          (3)
#define ARAD_PP_EG_QOS_ETH_REMARK_PROFILE_INDEX_MAX              (15)
#define ARAD_PP_EG_QOS_IP_REMARK_PROFILE_INDEX_MAX               (14)
#define ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS                        (3)
#define ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS                       (8)
#define ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_MIN                        (SOC_PPC_PKT_HDR_TYPE_ETH)
#define ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_MAX                        (SOC_PPC_PKT_HDR_TYPE_MPLS)
#define ARAD_PP_EG_ECN_CAPABLE_MAX                               (1)

#ifdef BCM_88660
/* For DSCP/EXP Marking { */
    #define ARAD_PP_EG_QOS_MAX_BY_BITS(bits)                     ((1 << (bits)) - 1)
    /* For a bitmap there is one bit for each value. Therefore the bitmap bits are (1 << bits). */
    #define ARAD_PP_EG_QOS_BITMAP_MAX_BY_BITS(bits)                        ARAD_PP_EG_QOS_MAX_BY_BITS(1 << bits)
    #define ARAD_PP_EG_QOS_INLIF_PROFILE_NOF_BITS(unit, dp_map_disabled)   (((SOC_IS_JERICHO_B0(unit) || SOC_IS_JERICHO_PLUS_A0(unit)) && (dp_map_disabled == 1)) ? 1 : 2)
    #define ARAD_PP_EG_QOS_INLIF_PROFILE_MAX(unit, dp_map_disabled)        ARAD_PP_EG_QOS_MAX_BY_BITS(ARAD_PP_EG_QOS_INLIF_PROFILE_NOF_BITS(unit, dp_map_disabled))
    #define ARAD_PP_EG_QOS_RESOLVED_DP_NOF_BITS(unit, dp_map_disabled)     (((SOC_IS_JERICHO_B0(unit) || SOC_IS_JERICHO_PLUS(unit)) && (dp_map_disabled == 1)) ? 2 : 1)
    #define ARAD_PP_EG_QOS_RESOLVED_DP_MAX(unit, dp_map_disabled)          ARAD_PP_EG_QOS_MAX_BY_BITS(ARAD_PP_EG_QOS_RESOLVED_DP_NOF_BITS(unit, dp_map_disabled))
    #define ARAD_PP_EG_QOS_DP_NOF_BITS                                     (2)
    #define ARAD_PP_EG_QOS_MARKING_PROFILE_NOF_BITS(unit, dp_map_disabled) (SOC_IS_JERICHO_PLUS(unit) ? ((dp_map_disabled == 1) ? 3 : 4) : 2)
    #define ARAD_PP_EG_QOS_MARKING_PROFILE_MAX(unit, dp_map_disabled)      ARAD_PP_EG_QOS_MAX_BY_BITS(ARAD_PP_EG_QOS_MARKING_PROFILE_NOF_BITS(unit, dp_map_disabled))
    #define ARAD_PP_EG_QOS_TC_NDX_NOF_BITS                                 (3)
    #define ARAD_PP_EG_QOS_IP_DSCP_NOF_BITS                                (8)
    #define ARAD_PP_EG_QOS_IP_DSCP_MAX                                     ARAD_PP_EG_QOS_MAX_BY_BITS(ARAD_PP_EG_QOS_IP_DSCP_NOF_BITS)
    #define ARAD_PP_EG_QOS_MPLS_EXP_NOF_BITS                               (3)
    #define ARAD_PP_EG_QOS_MPLS_EXP_MAX                                    ARAD_PP_EG_QOS_MAX_BY_BITS(ARAD_PP_EG_QOS_MPLS_EXP_NOF_BITS)
    #define ARAD_PP_EG_QOS_RESOLVED_DP_BITMAP_MAX                          ARAD_PP_EG_QOS_BITMAP_MAX_BY_BITS(ARAD_PP_EG_QOS_DP_NOF_BITS)
    #define ARAD_PP_EG_QOS_INLIF_PROFILE_BITMAP_MAX                        ARAD_PP_EG_QOS_BITMAP_MAX_BY_BITS(2)
/* } */
#endif /* BCM_88660 */

#define ARAD_PP_IP_TOS_MARKING_TABLE(unit)            (SOC_IS_JERICHO_PLUS(unit) ? EPNI_IP_TOS_MARKING_TABLEm : EPNI_IP_TOS_MARKINGm)

#define PP_ARAD_EPNI_IPV4_EXP_TO_TOS_MAP_REGS_KEY_ENTRY(profile,out_exp) \
  ((profile << 3) + out_exp)
  
#define PP_ARAD_EPNI_IPV6_EXP_TO_TC_MAP_REGS_KEY_ENTRY(profile,out_exp) \
  ((profile << 3) + out_exp)

/* ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA entry offset */
#define ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(is_ipv6, dp, profile, in_dscp_exp) \
  ((is_ipv6 << 12) + (dp << 10) + (profile << 6) + (in_dscp_exp >> 2))

/* ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA index offset */
#define ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_dscp_exp) \
  (in_dscp_exp & 3)

/* ARAD_PP_EPNI_EXP_REMARK_TBL_DATA entry offset */
#define ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(dp, profile, in_dscp_exp) \
  ((dp << 5) + (profile << 1) + ((in_dscp_exp & 0x4) >> 2))

/* ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA index offset */
#define ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_dscp_exp) \
  (in_dscp_exp & 3)


/* ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_DSCP entry offset */
#define ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_ENTRY_OFFSET(remark_profile, in_dscp_exp) \
  ((remark_profile << 6) + (in_dscp_exp >> 2))

/* ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_DSCP index offset */
#define ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_INDEX_OFFSET(in_dscp_exp) \
  (in_dscp_exp & 0x3)

/* ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_EXP entry offset */
#define ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_ENTRY_OFFSET(remark_profile, in_dscp_exp) \
  ((remark_profile << 5) + (in_dscp_exp >> 3))

/* ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_EXP index offset */
#define ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_INDEX_OFFSET(in_dscp_exp) \
  (in_dscp_exp & 0x7)

/* ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_TBL_DATA entry offset */
#define ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_KEY_ENTRY_OFFSET(remark_profile, in_dscp_exp) \
  ((remark_profile << 3) + (in_dscp_exp))

/* ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_TBL_DATA entry offset */
#define ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_KEY_ENTRY_OFFSET(remark_profile, in_dscp_exp) \
  ((remark_profile << 3) + (in_dscp_exp))

/* 
 * Encoding: 4d15, FTMH.ECN-Capable, Remark-Profile[1:0],DP[1:0]
 * ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_EXP last entry  
 */ 
#define ARAD_PP_EPNI_REMARK_ETH_TO_EXP_ECN_KEY_ENTRY_OFFSET(remark_profile, ecn_capable, dp) \
  ((15 << 5) + (ecn_capable << 4) + ((remark_profile & 0x3) << 2) + (dp))

/* 
 * Encoding: 4d15, FTMH.ECN-Capable, Remark-Profile[1:0],DP[1:0],DSCP-EXP[2](1b' MSB)  
 * ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_DSCP last entry  
 */ 
#define ARAD_PP_EPNI_REMARK_ETH_TO_DSCP_ECN_KEY_ENTRY_OFFSET(remark_profile, ecn_capable, dp, dscp_exp) \
  ((15 << 6) + (ecn_capable << 5) + ((remark_profile & 0x3) << 3) + (dp << 1) + (dscp_exp >> 2))

/* For LL-Arp format entry, remark-profile has only 3 bits from EEDB entry */
#define ARAD_PP_EPNI_REMARK_ETH_REMARK_PROFILE_MASK  0x7

/* Check whether the ethernet remark profile MSB is set */
#define ARAD_PP_EPNI_REMARK_ETH_REMARK_PROFILE_MSB(remark_profile) (remark_profile & 0x8)

/* 
 * Encoding: 4d15, FTMH.ECN-Capable, Remark-Profile[2:0],DP[1:0]
 * ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_EXP last entry  
 */ 
#define ARAD_PP_EPNI_REMARK_ETH_TO_EXP_KEY_ENTRY_OFFSET(remark_profile, dp) \
  ((15 << 5) + ((remark_profile & 0x7) << 2) + (dp))

/* 
 * Encoding: 4d15, Remark-Profile[2:0],DP[1:0],DSCP-EXP[2](1b' MSB)  
 * ARAD_PP_EPNI_REMARK_IPV4/IPV6_TO_DSCP last entry  
 */ 
#define ARAD_PP_EPNI_REMARK_ETH_TO_DSCP_KEY_ENTRY_OFFSET(remark_profile, dp, dscp_exp) \
  ((15 << 6) + ((remark_profile & 0x7) << 3) + (dp << 1) + (dscp_exp >> 2))



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

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC SOC_PROCEDURE_DESC_ELEMENT
  Arad_pp_procedure_desc_element_eg_qos[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PORT_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_QOS_PARAMS_REMARK_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_QOS_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_QOS_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_eg_qos[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_EG_QOS_DSCP_EXP_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_QOS_DSCP_EXP_OUT_OF_RANGE_ERR",
    "The parameter 'dscp_exp' is out of range. \n\r "
    "The range is: No min - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR",
    "The parameter 'in_dscp_exp' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_QOS_EXP_MAP_PROFILE_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_QOS_EXP_MAP_PROFILE_OUT_OF_RANGE_ERR",
    "The parameter 'exp_map_profile' is out of range. \n\r "
    "The range is: 0 - 3.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_QOS_PHP_TYPE_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_QOS_PHP_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'php_type' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_EG_QOS_UNIFORM_PHP_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_PKT_HDR_OUT_OF_RANGE_ERR,
    "ARAD_PP_PKT_HDR_OUT_OF_RANGE_ERR",
    "The parameter of type 'PKT_HDR_TYPE' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_PKT_HDR_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_REMARK_PROFILE_OUT_OF_RANGE_ERR,
    "ARAD_PP_REMARK_PROFILE_OUT_OF_RANGE_ERR",
    "The parameter of type 'remark_profile' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_OUT_OF_RANGE_ERR",
    "The parameter of type 'pkt_hdr_type' is out of range. \n\r "
    "Allowed types: ipv4, ipv6,mpls."
    "The range is: 2 - 4.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_ECN_CAPABLE_OUT_OF_RANGE_ERR,
    "ARAD_PP_ECN_CAPABLE_OUT_OF_RANGE_ERR",
    "The parameter of type 'ecn_capable' is out of range. \n\r "    
    "The range is: 0 - 1. Valid for ARAD-B0 and above\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
SOC_ERR_DESC_ELEMENT_DEF_LAST
};

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

STATIC int _arad_pp_eg_qos_init_unsafe_epni_dscp_remark_callback(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int copyno, 
    SOC_SAND_IN int array_index, 
    SOC_SAND_IN int index, 
    SOC_SAND_OUT uint32 *value, 
    SOC_SAND_IN int entry_sz, 
    SOC_SAND_IN void *opaque)
{
  /* Unrolls the following loop: */
  /*
  for (remark_profile = 0; remark_profile <= ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX; ++remark_profile) {
      for (in_dscp = 0; in_dscp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX; ++in_dscp) {
          for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
              for (routing_protocol = 0; routing_protocol < 2; ++routing_protocol) {

                  entry_offset = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(routing_protocol, dp, remark_profile, in_dscp);
                  // each entry_offset contains 4 dscp-exp-remark values
                  index = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_dscp) * ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS;

                  res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit, entry_offset, &ip_tbl_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

                  // in-DSCP-EXP = out-DSCP-EXP 
                  res = soc_sand_bitstream_set_any_field(&in_dscp, index, ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS, &ip_tbl_data.dscp_remark_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

                  res = arad_pp_epni_dscp_remark_tbl_set_unsafe(unit, entry_offset, &ip_tbl_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
              }
          }
      }
  }*/

  uint32 remark_profile = 0;
  uint32 in_dscp = 0;
  uint32 dp = 0;
  uint32 routing_protocol = 0;
  uint32 offset_in_entry = 0;
  /* The 2 dscp LSBs are offsets inside the entry.*/
  uint32 dscp_lsb_idx = 0;
  uint32 dscp_remark_data = 0;
  uint32 uindex = (uint32)index;

  *value = 0;
  SHR_BITCOPY_RANGE(&in_dscp, 2, &uindex, 0, 6);
  SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 6, 4);
  SHR_BITCOPY_RANGE(&dp, 0, &uindex, 10, 2);
  SHR_BITCOPY_RANGE(&routing_protocol, 0, &uindex, 12, 1);

  if (remark_profile <= ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX && in_dscp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX && dp <= ARAD_PP_EG_QOS_DP_MAX) {
    dscp_remark_data = 0;
    for (dscp_lsb_idx = 0; dscp_lsb_idx < 4; dscp_lsb_idx++) {
      offset_in_entry = dscp_lsb_idx * ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS;
      SHR_BITCOPY_RANGE(&dscp_remark_data, offset_in_entry, &in_dscp, 0, ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS);
      in_dscp++;
    }

    soc_mem_field32_set(unit, EPNI_DSCP_REMARKm, value, DSCP_REMARK_DATAf, dscp_remark_data);
  }

  return 0;
}

STATIC int _arad_pp_eg_qos_init_unsafe_epni_exp_remark_callback(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int copyno,
    SOC_SAND_IN int array_index,
    SOC_SAND_IN int index,
    SOC_SAND_OUT uint32 *value,
    SOC_SAND_IN int entry_sz,
    SOC_SAND_IN void *opaque)
{
  /* Unrolls the following loop: */
  /* MPLS */
/*  for (in_dscp = 0; in_dscp <= SOC_SAND_PP_MPLS_DSCP_MAX; ++in_dscp) {
      for (remark_profile = 0; remark_profile <= ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX; ++remark_profile) {
          for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {

              entry_offset = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(dp, remark_profile, in_dscp);
*/              /* each entry_offset contains 4 dscp-exp-remark values */
/*              index = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_dscp) * ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS;

              res = arad_pp_epni_exp_remark_tbl_get_unsafe(unit, entry_offset, &mpls_tbl_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

*/              /* in-DSCP-EXP = out-DSCP-EXP */
/*              res = soc_sand_bitstream_set_any_field(&in_dscp, index, ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS, &mpls_tbl_data.exp_remark_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

              res = arad_pp_epni_exp_remark_tbl_set_unsafe(unit, entry_offset, &mpls_tbl_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
          }
      }
  }
*/
  /* The 2 exp LSBs are offsets inside the entry.*/
  uint32 remark_profile = 0;
  uint32 in_exp = 0;
  uint32 offset_in_entry = 0;
  uint32 exp_lsb_idx = 0;
  uint32 exp_remark_data = 0;
  uint32 uindex = (uint32)index;

  *value = 0;

  SHR_BITCOPY_RANGE(&in_exp, 2, &uindex, 0, 1);
  SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 1, 4);

  if (remark_profile <= ARAD_PP_EG_QOS_NOF_REMARK_PROFILES && in_exp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX) {
    exp_remark_data = 0;
    for (exp_lsb_idx = 0; exp_lsb_idx < 4; exp_lsb_idx++) {
      offset_in_entry = exp_lsb_idx * ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS;
      SHR_BITCOPY_RANGE(&exp_remark_data, offset_in_entry, &in_exp, 0, ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS);
      in_exp++;
    }

    soc_mem_field32_set(unit, EPNI_EXP_REMARKm, value, EXP_REMARK_DATAf, exp_remark_data);
  }

  return 0;
}



STATIC int _arad_pp_eg_qos_init_unsafe_epni_remark_to_exp_callback(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int copyno,
    SOC_SAND_IN int array_index,
    SOC_SAND_IN int index,
    SOC_SAND_OUT uint32 *value,
    SOC_SAND_IN int entry_sz,
    SOC_SAND_IN void *opaque)
{

  /* Unrolls the following loop: */
/*     Remark encapsulation IPV4
    in_encap_qos_key.remark_profile = 0;

    in_encap_qos_key.pkt_hdr_type   = SOC_PPC_PKT_HDR_TYPE_IPV4;
    for (in_dscp = 0; in_dscp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX; ++in_dscp) {
      for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
        in_encap_qos_key.dp = dp;
        in_encap_qos_key.in_dscp_exp  = in_dscp;
        out_encap_qos_params.ip_dscp  = in_dscp;
        out_encap_qos_params.mpls_exp = in_dscp % (SOC_SAND_PP_MPLS_DSCP_MAX+1);
        res = arad_pp_eg_encap_qos_params_remark_set_unsafe(unit, &in_encap_qos_key, &out_encap_qos_params);
        SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);
      }
    }

    switch (in_encap_qos_key->pkt_hdr_type)
    {
    case (SOC_PPC_PKT_HDR_TYPE_IPV4):
       set remark ipv4 to exp table
      entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
      index = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
      index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

      res = arad_pp_epni_remark_ipv4_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_exp);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      tmp = out_encap_qos_params->mpls_exp;
      res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &ipv4_to_exp.dscp_exp_remark_data);
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      res = arad_pp_epni_remark_ipv4_to_exp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_exp);
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);*/



  /* The 3 exp LSBs are offsets inside the entry.*/
  uint32 remark_profile = 0;
  uint32 in_dscp_exp = 0;
  uint32 offset_in_entry = 0;

  uint32 lsb_idx = 0;
  uint32 exp_data = 0;
  uint32 mem = 0;
  uint32 field = 0;
  uint32 uindex = (uint32)index;
  uint32 num_of_values_per_entry = 0;
  const uint32 pkt_hdr_type = *((uint32 *)opaque);
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  *value = 0;


  switch(pkt_hdr_type)
  {
    case SOC_PPC_PKT_HDR_TYPE_IPV4:
      mem = EPNI_REMARK_IPV4_TO_EXPm;
      field = EXP_REMARK_DATAf;
      num_of_values_per_entry = 8;
      /*the 3 DSCP MSBs is priority*/
      SHR_BITCOPY_RANGE(&in_dscp_exp, 0, &uindex, 2, 3);
      SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 5, 4);
      break;
    case SOC_PPC_PKT_HDR_TYPE_IPV6:
      mem = EPNI_REMARK_IPV6_TO_EXPm;
      field = EXP_REMARK_DATAf;
      num_of_values_per_entry = 8;
      /*the 3 DSCP MSBs is priority*/
      SHR_BITCOPY_RANGE(&in_dscp_exp, 0, &uindex, 2, 3);
      SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 5, 4);
      break;
    case SOC_PPC_PKT_HDR_TYPE_MPLS:
      mem = EPNI_REMARK_MPLS_TO_EXPm;
      field = REMARK_MPLS_TO_EXPf;
      num_of_values_per_entry = 1;
      SHR_BITCOPY_RANGE(&in_dscp_exp, 0, &uindex, 0, 3);
      SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 3, 4);
      break;
    default:
      SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_pp_eg_qos_init_unsafe_epni_remark_to_exp_callback()", pkt_hdr_type , 0);
      break;
  }

  /* We configure only the first profile */
  if (remark_profile > 0 )
  {
    return 0;
  }

  if (in_dscp_exp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX ) {
    exp_data = 0;
    for (lsb_idx = 0; lsb_idx < num_of_values_per_entry; lsb_idx++) {
      offset_in_entry = lsb_idx * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;
      SHR_BITCOPY_RANGE(&exp_data, offset_in_entry, &in_dscp_exp, 0, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS);
      /*in_dscp_exp++;*/
    }

    soc_mem_field32_set(unit, mem, value, field, exp_data);
  }

  return 0;
}



STATIC int _arad_pp_eg_qos_init_unsafe_epni_remark_to_dscp_callback(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int copyno,
    SOC_SAND_IN int array_index,
    SOC_SAND_IN int index,
    SOC_SAND_OUT uint32 *value,
    SOC_SAND_IN int entry_sz,
    SOC_SAND_IN void *opaque)
{

  /* Unrolls the following loop: */
/*     Remark encapsulation IPV4
    in_encap_qos_key.remark_profile = 0;

    in_encap_qos_key.pkt_hdr_type   = SOC_PPC_PKT_HDR_TYPE_IPV4;
    for (in_dscp = 0; in_dscp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX; ++in_dscp) {
      for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
        in_encap_qos_key.dp = dp;
        in_encap_qos_key.in_dscp_exp  = in_dscp;
        out_encap_qos_params.ip_dscp  = in_dscp;
        out_encap_qos_params.mpls_exp = in_dscp % (SOC_SAND_PP_MPLS_DSCP_MAX+1);
        res = arad_pp_eg_encap_qos_params_remark_set_unsafe(unit, &in_encap_qos_key, &out_encap_qos_params);
        SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);
      }
    }

       set remark ipv4 to dscp table
      entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
      index = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
      index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

      res = arad_pp_epni_remark_ipv4_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_dscp);
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      tmp = out_encap_qos_params->ip_dscp;
      res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &ipv4_to_dscp.dscp_exp_remark_data);
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

      res = arad_pp_epni_remark_ipv4_to_dscp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_dscp);
      SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
     break;*/


  /* The 2 exp LSBs are offsets inside the entry.*/
  uint32 remark_profile = 0;
  uint32 in_dscp_exp = 0;
  uint32 offset_in_entry = 0;

  uint32 lsb_idx = 0;
  uint32 dscp_data = 0;
  uint32 mem = 0;
  uint32 field = 0;
  uint32 uindex = (uint32)index;
    uint32 num_of_values_per_entry = 0;
  const uint32 pkt_hdr_type = *((uint32 *)opaque);
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  *value = 0;

  switch(pkt_hdr_type)
  {
    case SOC_PPC_PKT_HDR_TYPE_IPV4:
      mem = EPNI_REMARK_IPV4_TO_DSCPm;
      field = EXP_REMARK_DATAf;
      num_of_values_per_entry = 4;
      SHR_BITCOPY_RANGE(&in_dscp_exp, 2, &uindex, 0, 6);
      SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 6, 4);
      break;
    case SOC_PPC_PKT_HDR_TYPE_IPV6:
      mem = EPNI_REMARK_IPV6_TO_DSCPm;
      field = EXP_REMARK_DATAf;
      num_of_values_per_entry = 4;
      SHR_BITCOPY_RANGE(&in_dscp_exp, 2, &uindex, 0, 6);
      SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 6, 4);
      break;
    case SOC_PPC_PKT_HDR_TYPE_MPLS:
      mem = EPNI_REMARK_MPLS_TO_DSCPm;
      field = REMARK_MPLS_TO_DSCPf;
      num_of_values_per_entry = 1;
      SHR_BITCOPY_RANGE(&in_dscp_exp, 0, &uindex, 0, 3);
      SHR_BITCOPY_RANGE(&remark_profile, 0, &uindex, 3, 4);
      break;
    default:
      SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_pp_eg_qos_init_unsafe_epni_remark_to_dscp_callback()", pkt_hdr_type , 0);
      break;
  }

  /* We configure only the first profile */
  if (remark_profile > 0 )
  {
    return 0;
  }
  if (in_dscp_exp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX ) {
    dscp_data = 0;
    for (lsb_idx = 0; lsb_idx < num_of_values_per_entry; lsb_idx++) {
      offset_in_entry = lsb_idx * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;
      SHR_BITCOPY_RANGE(&dscp_data, offset_in_entry, &in_dscp_exp, 0, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS);
      in_dscp_exp++;
    }

    soc_mem_field32_set(unit, mem, value, field, dscp_data);
  }

  return 0;
}


uint32
  arad_pp_eg_qos_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    in_dscp;
  SOC_SAND_PP_DP
    dp;
  uint32
    res = SOC_SAND_OK,
/*    remark_profile,*/
/*    routing_protocol, */
/*    entry_offset,*/
    index,
    reg_val,
    tos,
    dscp_exp,
    offset,
    pkt_hdr_type;
  /*
  ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA
    ip_tbl_data;
  ARAD_PP_EPNI_EXP_REMARK_TBL_DATA
    mpls_tbl_data;
    */
  SOC_PPC_EG_QOS_PHP_REMARK_KEY
    php_remark;
  soc_reg_above_64_val_t reg_above64_val;
  SOC_PPC_EG_ENCAP_QOS_MAP_KEY
    in_encap_qos_key;
  SOC_PPC_EG_ENCAP_QOS_PARAMS
    out_encap_qos_params;
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_PCID_LITE_SKIP(unit);


  if (SOC_IS_ARAD_B0_AND_ABOVE(unit)) {

      /* 1:1 mapping of 2 bits LL remark profile to 4 bits */
      reg_val = (1 << 4) | (2 << 8) | (3 << 12);
      res = WRITE_EPNI_REMARK_PROFILE_CONFIGURATIONr(unit, REG_PORT_ANY, reg_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

      /* set mapping of Cfg-ECN-IP-Map[{FTMH.CNI, FTMH.ECN-Capable, IP-Header.ToS[1:0]}] -> Out-DSCP-EXP[1:0] for IPV4 and IPV6 forwarding */
      reg_val = 0;

      /* FTMH.CNI FTMH.ECN-Capable IP-Header.ToS[1:0] Out-DSCP-EXP[1:0]
            1        1                x                3
            0/1      0                x                x
            0        1                3                3
            0        1                0-2              1/2 */

      for (tos = 0; tos < 4; tos++) { /* run over all possible IP-Header.ToS[1:0] */

          /* case 1:
             {FTMH.CNI = 1, FTMH.ECN-Capable = 1, TOS[1:0]} -> 3 */
          offset = ((3 << 2) | tos)*2;
          reg_val |= (3 << offset);

          /* case 2-1:
             {FTMH.CNI = 0, FTMH.ECN-Capable = 0, TOS[1:0]} -> TOS[1:0] */
          offset = (tos & 0x3)*2;
          reg_val |= ((tos & 0x3) << offset);

          /* case 2-2:
             {FTMH.CNI = 1, FTMH.ECN-Capable = 0, TOS[1:0]} -> TOS[1:0] */
          offset = ((2 << 2) | tos)*2;
          reg_val |= ((tos & 0x3) << offset);

          /* cases 3,4: FTMH.CNI = 0, FTMH.ECN-Capable = 1 */
          offset = ((1 << 2) | tos)*2;
          if (tos == 3) {
              /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, TOS[1:0] = 3} -> 3 */
              reg_val |= (3 << offset); 
          }
          else if (tos == 0) {
              /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, TOS[1:0] = 0} -> 1 */
              reg_val |= (1 << offset); 
          }
          else { /* tos = 1/2 */
              /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, TOS[1:0] = 1/2} -> 1/2 */
              reg_val |= ((tos & 0x3) << offset); 
          }
      }
      res = WRITE_EPNI_ECN_IP_MAPr(unit, REG_PORT_ANY, reg_val); 
      SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);

      /* set mapping of Cfg-ECN-MPLS-Map[{FTMH.CNI, FTMH.ECN-Capable, Out-DSCP-EXP[2:0]] -> Out-DSCP-EXP[2:0] for MPLS forwarding */
      SOC_REG_ABOVE_64_CLEAR(reg_above64_val);

      /* check if MPLS ECN is in 1/2 bits mode */
      if (!(SOC_DPP_CONFIG(unit)->qos.ecn_mpls_one_bit_mode)) {

          /* FTMH.CNI  FTMH.ECN-Capable  Out-DSCP-EXP[2:0]  Out-DSCP-EXP[2:0] (new)
                1          1                  xxx                  x11
                1/0        0                  xxx                  xxx
                0          1                  x11                  x11
                0          1                  x10, x01, x00        x01/x10 */

          for (dscp_exp = 0; dscp_exp < 8; dscp_exp++) { /* run over all possible MPLS-out-DSCP-EXP[2:0] */

              /* case 1:
                 {FTMH.CNI = 1, FTMH.ECN-Capable = 1, DSCP-EXP[2:0]} -> x11 */
              offset = (((3 << 3) | dscp_exp)*3) % 32;
              index = (((3 << 3) | dscp_exp)*3) / 32;
              reg_above64_val[index] |= ((dscp_exp | 3) << offset);

              /* case 2-1:
                 {FTMH.CNI = 0, FTMH.ECN-Capable = 0, DSCP-EXP[2:0]} -> xxx */
              offset = (dscp_exp*3) % 32;
              index = (dscp_exp*3) / 32;
              reg_above64_val[index] |= (dscp_exp << offset);

              /* case 2-2:
                 {FTMH.CNI = 1, FTMH.ECN-Capable = 0, DSCP-EXP[2:0]} -> xxx */
              offset = (((2 << 2) | dscp_exp)*3) % 32;
              index = (((2 << 2) | dscp_exp)*3) / 32;
              reg_above64_val[index] |= (dscp_exp << offset);

              /* cases 3,4: FTMH.CNI = 0, FTMH.ECN-Capable = 1 */
              offset = (((1 << 2) | dscp_exp)*3) % 32;
              index = (((1 << 2) | dscp_exp)*3) / 32;
              if ((dscp_exp & 0x3) == 3) {
                  /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, DSCP-EXP[2:0] = x11} -> x11 */
                  reg_above64_val[index] |= (dscp_exp << offset);
              }
              else if ((dscp_exp & 0x3) == 0) {
                  /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, DSCP-EXP[2:0] = x00} -> x01 */
                  reg_above64_val[index] |= ((dscp_exp | 1) << offset); 
              }
              else { /* dscp = 1/2 */
                  /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, DSCP-EXP[2:0] = x01/x10} -> x01/x10 */
                  reg_above64_val[index] |= (dscp_exp << offset); 
              }
          }
      }
      else { /* 1 bit */

          /* FTMH.CNI  FTMH.ECN-Capable  Out-DSCP-EXP[2:0]  Out-DSCP-EXP[2:0] (new)
                1             1                xxx                 xx1
                0             1                xxx                 xxx */

          for (dscp_exp = 0; dscp_exp < 8; dscp_exp++) { /* run over all possible MPLS-out-DSCP-EXP[2:0] */

              /* case 1:
                 {FTMH.CNI = 1, FTMH.ECN-Capable = 1, DSCP-EXP[2:0]} -> xx1 */
              offset = (((3 << 3) | dscp_exp)*3) % 32;
              index = (((3 << 3) | dscp_exp)*3) / 32;
              reg_above64_val[index] |= ((dscp_exp | 1) << offset);

              /* case 2:
                  {FTMH.CNI = 0, FTMH.ECN-Capable = 1, DSCP-EXP[2:0]} -> xxx */
              offset = (((1 << 3) | dscp_exp)*3) % 32;
              index = (((1 << 3) | dscp_exp)*3) / 32;
              reg_above64_val[index] |= (dscp_exp << offset);
          }
      }
      res = WRITE_EPNI_ECN_MPLS_MAPr(unit, REG_PORT_ANY, reg_above64_val); 
      SOC_SAND_CHECK_FUNC_RESULT(res, 9, exit);
  }

  /* Instead of doing these (many) writes by loop, we use DMA. */
  /* Remark Forwarding */
  /* 1:1 mapping of in-DSCP-EXP to out-DSCP-EXP */
  /* IPV4/6 */
  
  /*
  for (remark_profile = 0; remark_profile <= ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX; ++remark_profile) {
      for (in_dscp = 0; in_dscp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX; ++in_dscp) {
          for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
              for (routing_protocol = 0; routing_protocol < 2; ++routing_protocol) {

                  entry_offset = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(routing_protocol, dp, remark_profile, in_dscp);
                  // each entry_offset contains 4 dscp-exp-remark values
                  index = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_dscp) * ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS;

                  res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit, entry_offset, &ip_tbl_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

                  // in-DSCP-EXP = out-DSCP-EXP 
                  res = soc_sand_bitstream_set_any_field(&in_dscp, index, ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS, &ip_tbl_data.dscp_remark_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

                  res = arad_pp_epni_dscp_remark_tbl_set_unsafe(unit, entry_offset, &ip_tbl_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
              }
          }
      }
  }*/
  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_DSCP_REMARKm, 0, MEM_BLOCK_ANY, -1, -1, 
                                                        _arad_pp_eg_qos_init_unsafe_epni_dscp_remark_callback, NULL);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 10, exit);
  }

  /* MPLS */
/*
  for (in_dscp = 0; in_dscp <= SOC_SAND_PP_MPLS_DSCP_MAX; ++in_dscp) {
      for (remark_profile = 0; remark_profile <= ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX; ++remark_profile) {
          for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {

              entry_offset = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(dp, remark_profile, in_dscp);
*/              /* each entry_offset contains 4 dscp-exp-remark values*/
/*              index = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_dscp) * ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS;

              res = arad_pp_epni_exp_remark_tbl_get_unsafe(unit, entry_offset, &mpls_tbl_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

*/              /* in-DSCP-EXP = out-DSCP-EXP*/
/*              res = soc_sand_bitstream_set_any_field(&in_dscp, index, ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS, &mpls_tbl_data.exp_remark_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

              res = arad_pp_epni_exp_remark_tbl_set_unsafe(unit, entry_offset, &mpls_tbl_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
          }
      }
  }
*/

  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_EXP_REMARKm, 0, MEM_BLOCK_ANY, -1, -1,
                                                              _arad_pp_eg_qos_init_unsafe_epni_exp_remark_callback, NULL);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 90, exit);
  }

  /* Remark encapsulation IPV4 */

/*
  in_encap_qos_key.remark_profile = 0; 

  in_encap_qos_key.pkt_hdr_type   = SOC_PPC_PKT_HDR_TYPE_IPV4;
  for (in_dscp = 0; in_dscp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX; ++in_dscp) {
    for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
      in_encap_qos_key.dp = dp;
      in_encap_qos_key.in_dscp_exp  = in_dscp;
      out_encap_qos_params.ip_dscp  = in_dscp;
      out_encap_qos_params.mpls_exp = in_dscp % (SOC_SAND_PP_MPLS_DSCP_MAX+1); 
      res = arad_pp_eg_encap_qos_params_remark_set_unsafe(unit, &in_encap_qos_key, &out_encap_qos_params);
      SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);
    }
  }
*/

  pkt_hdr_type = SOC_PPC_PKT_HDR_TYPE_IPV4;
  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_REMARK_IPV4_TO_EXPm, 0, MEM_BLOCK_ANY, -1, -1,
                                                              _arad_pp_eg_qos_init_unsafe_epni_remark_to_exp_callback, &pkt_hdr_type);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 350, exit);
  }

  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_REMARK_IPV4_TO_DSCPm, 0, MEM_BLOCK_ANY, -1, -1,
                                                              _arad_pp_eg_qos_init_unsafe_epni_remark_to_dscp_callback, &pkt_hdr_type);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 360, exit);
  }
 
  /* Remark encapsulation IPV6 */
  /*
  in_encap_qos_key.pkt_hdr_type = SOC_PPC_PKT_HDR_TYPE_IPV6;
  for (in_dscp = 0; in_dscp <= ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX; ++in_dscp) {
    for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
      in_encap_qos_key.dp = dp;
      in_encap_qos_key.in_dscp_exp  = in_dscp;
      out_encap_qos_params.ip_dscp  = in_dscp;
      out_encap_qos_params.mpls_exp = in_dscp % (SOC_SAND_PP_MPLS_DSCP_MAX+1); 
      res = arad_pp_eg_encap_qos_params_remark_set_unsafe(unit, &in_encap_qos_key, &out_encap_qos_params);
      SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);
    }
  }
  */

  pkt_hdr_type = SOC_PPC_PKT_HDR_TYPE_IPV6;
  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_REMARK_IPV6_TO_EXPm, 0, MEM_BLOCK_ANY, -1, -1,
                                                              _arad_pp_eg_qos_init_unsafe_epni_remark_to_exp_callback, &pkt_hdr_type);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 370, exit);
  }

  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_REMARK_IPV6_TO_DSCPm, 0, MEM_BLOCK_ANY, -1, -1,
                                                              _arad_pp_eg_qos_init_unsafe_epni_remark_to_dscp_callback, &pkt_hdr_type);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 380, exit);
  }

  /* Remark encapsulation MPLS */
  /*
  in_encap_qos_key.pkt_hdr_type   = SOC_PPC_PKT_HDR_TYPE_MPLS;
  for (in_dscp = 0; in_dscp <= SOC_SAND_PP_MPLS_DSCP_MAX; ++in_dscp) {
    for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
      in_encap_qos_key.dp           = dp;
      in_encap_qos_key.in_dscp_exp  = in_dscp;
      out_encap_qos_params.ip_dscp  = in_dscp;
      out_encap_qos_params.mpls_exp = in_dscp; 
      res = arad_pp_eg_encap_qos_params_remark_set_unsafe(unit, &in_encap_qos_key, &out_encap_qos_params);
      SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);
    }
  }
  */
  pkt_hdr_type = SOC_PPC_PKT_HDR_TYPE_MPLS;

  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_REMARK_MPLS_TO_EXPm, 0, MEM_BLOCK_ANY, -1, -1,
                                                              _arad_pp_eg_qos_init_unsafe_epni_remark_to_exp_callback, &pkt_hdr_type);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 390, exit);
  }

  res = arad_fill_table_with_variable_values_by_caching(unit, EPNI_REMARK_MPLS_TO_DSCPm, 0, MEM_BLOCK_ANY, -1, -1,
                                                              _arad_pp_eg_qos_init_unsafe_epni_remark_to_dscp_callback, &pkt_hdr_type);
  if (res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(0, 400, exit);
  }

  if (SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
    /* Remark encapsulation ETH */                                                                                           
    in_encap_qos_key.pkt_hdr_type = SOC_PPC_PKT_HDR_TYPE_ETH;
    in_encap_qos_key.remark_profile = 0; /* Default profile */
    for (in_dscp = 0; in_dscp <= SOC_SAND_PP_MPLS_EXP_MAX; ++in_dscp) {
        for (dp = 0; dp <= ARAD_PP_EG_QOS_DP_MAX; ++dp) {
            /* Set 1:1 mapping from in-dscp-exp to MPLS-EXP and IP-DSCP */
            in_encap_qos_key.dp = dp;
            in_encap_qos_key.in_dscp_exp  = in_dscp;
            out_encap_qos_params.ip_dscp  = in_dscp;
            out_encap_qos_params.mpls_exp = in_dscp % (SOC_SAND_PP_MPLS_DSCP_MAX+1);
            res = arad_pp_eg_encap_qos_params_remark_set_unsafe(unit, &in_encap_qos_key, &out_encap_qos_params);
            SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);
        }
    }
  }
  
  /* PHP */
  SOC_PPC_EG_QOS_PHP_REMARK_KEY_clear(&php_remark);
  for (in_dscp = 0; in_dscp <= SOC_SAND_PP_MPLS_DSCP_MAX; ++in_dscp) {
    for (php_remark.php_type = SOC_PPC_EG_QOS_UNIFORM_PHP_POP_INTO_IPV4; php_remark.php_type < SOC_PPC_NOF_EG_QOS_UNIFORM_PHP_TYPES; ++php_remark.php_type) {
      php_remark.exp_map_profile = 0;
      php_remark.exp = in_dscp;
       
      /* in-DSCP-EXP = out-DSCP-EXP */
      /* exp should mapt to dscp Precedence, the 3 Msb */
      res = arad_pp_eg_qos_params_php_remark_set_unsafe(unit, &php_remark, (in_dscp << 5));
      SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);      
    }
  }

  if (SOC_IS_JERICHO(unit)) {
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  100,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_REMARK_PROFILE_CONFIGURATIONr, SOC_CORE_ALL, 0, LINK_LAYER_REMARK_PROFILE_MAPf,  0x76543210));
  } else if (SOC_IS_ARAD_B0_AND_ABOVE(unit)){
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_REMARK_PROFILE_CONFIGURATIONr, SOC_CORE_ALL, 0, LINK_LAYER_REMARK_PROFILE_MAPf,  0x3210));
  }


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_init_unsafe()", 0, 0);
}

/*********************************************************************
*     Sets port information for egress QoS setting, including
 *     profiles used for QoS remarking.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_qos_port_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_PP_PCT_TBL_DATA
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PORT_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(port_qos_info);
  
  res = arad_pp_epni_pp_pct_tbl_get_unsafe(unit, core_id, local_port_ndx, &tbl_data);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  
  tbl_data.exp_map_profile  = port_qos_info->exp_map_profile;

  res = arad_pp_epni_pp_pct_tbl_set_unsafe(unit, core_id, local_port_ndx, &tbl_data);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

#ifdef BCM_88660
  if (SOC_IS_ARADPLUS(unit)) {
      res = WRITE_EPNI_PP_REMARK_PROFILEm(unit, MEM_BLOCK_ANY, local_port_ndx, (void*)&(port_qos_info->marking_profile));
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }
#endif /* BCM_88660 */
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_port_info_set_unsafe()", local_port_ndx, 0);
}

uint32
  arad_pp_eg_qos_port_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PORT_INFO_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(local_port_ndx, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_QOS_PORT_INFO, unit, port_qos_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_port_info_set_verify()", local_port_ndx, 0);
}

uint32
  arad_pp_eg_qos_port_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  )
{
  uint32
    res = SOC_SAND_OK;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PORT_INFO_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(local_port_ndx, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_port_info_get_verify()", local_port_ndx, 0);
}

/*********************************************************************
*     Sets port information for egress QoS setting, including
 *     profiles used for QoS remarking.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_qos_port_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_PP_PCT_TBL_DATA
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PORT_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(port_qos_info);

  SOC_PPC_EG_QOS_PORT_INFO_clear(port_qos_info);

  res = arad_pp_epni_pp_pct_tbl_get_unsafe(unit, core_id, local_port_ndx, &tbl_data);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  port_qos_info->exp_map_profile = tbl_data.exp_map_profile;

#ifdef BCM_88660
  if (SOC_IS_ARADPLUS(unit)) {
      res = READ_EPNI_PP_REMARK_PROFILEm(unit, MEM_BLOCK_ANY, local_port_ndx, &port_qos_info->marking_profile);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
#endif /* BCM_88660 */

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_port_info_get_unsafe()", local_port_ndx, 0);
}

/*********************************************************************
*     Sets how to remark QoS parameters upon PHP operation.
 *     When uniform pop performed the dscp_exp value is
 *     remarked.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_qos_params_php_remark_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_IN  uint32                                  dscp_exp
  )
{
  uint32
    index,
    key,
    tmp,
    res = SOC_SAND_OK;
  soc_reg_above_64_val_t ipv4_reg, ipv4_fld;
  soc_reg_above_64_val_t ipv6_reg, ipv6_fld;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(php_key);
  SOC_REG_ABOVE_64_CLEAR(ipv4_reg);
  SOC_REG_ABOVE_64_CLEAR(ipv4_fld);
  SOC_REG_ABOVE_64_CLEAR(ipv6_reg);
  SOC_REG_ABOVE_64_CLEAR(ipv6_fld);
  
  tmp = dscp_exp;

  switch ( php_key->php_type )
  {
  case SOC_PPC_EG_QOS_UNIFORM_PHP_POP_INTO_IPV4:
    key = PP_ARAD_EPNI_IPV4_EXP_TO_TOS_MAP_REGS_KEY_ENTRY(php_key->exp_map_profile,php_key->exp);
    index = key * ARAD_PP_EG_QOS_IPV4_OUT_EXP_NOF_BITS;

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_EPNI_IPV4_EXP_TO_TOS_MAPr(unit, REG_PORT_ANY, ipv4_reg));
    soc_reg_above_64_field_get(unit, EPNI_IPV4_EXP_TO_TOS_MAPr, ipv4_reg, IPV4_EXP_TO_TOS_MAPf, ipv4_fld);
    
    res = soc_sand_bitstream_set_any_field(&tmp, index, ARAD_PP_EG_QOS_IPV4_OUT_EXP_NOF_BITS, ipv4_fld);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    soc_reg_above_64_field_set(unit, EPNI_IPV4_EXP_TO_TOS_MAPr, ipv4_reg, IPV4_EXP_TO_TOS_MAPf, ipv4_fld);

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 50, exit, WRITE_EPNI_IPV4_EXP_TO_TOS_MAPr(unit, REG_PORT_ANY, ipv4_reg));
    break;
  case SOC_PPC_EG_QOS_UNIFORM_PHP_POP_INTO_IPV6:
    key = PP_ARAD_EPNI_IPV6_EXP_TO_TC_MAP_REGS_KEY_ENTRY(php_key->exp_map_profile,php_key->exp);
    index = key * ARAD_PP_EG_QOS_IPV6_OUT_EXP_NOF_BITS;

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, READ_EPNI_IPV6_EXP_TO_TC_MAPr(unit, REG_PORT_ANY, ipv6_reg));
    soc_reg_above_64_field_get(unit, EPNI_IPV6_EXP_TO_TC_MAPr, ipv6_reg, IPV6_EXP_TO_TC_MAPf, ipv6_fld);

    res = soc_sand_bitstream_set_any_field(&tmp, index, ARAD_PP_EG_QOS_IPV6_OUT_EXP_NOF_BITS, ipv6_fld);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    soc_reg_above_64_field_set(unit, EPNI_IPV6_EXP_TO_TC_MAPr, ipv6_reg, IPV6_EXP_TO_TC_MAPf, ipv6_fld);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, WRITE_EPNI_IPV6_EXP_TO_TC_MAPr(unit, REG_PORT_ANY, ipv6_reg));
    break;
  default:
    break;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_php_remark_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_qos_params_php_remark_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_IN  uint32                                  dscp_exp
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_QOS_PHP_REMARK_KEY, php_key, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(dscp_exp, ARAD_PP_EG_QOS_DSCP_EXP_MAX, ARAD_PP_EG_QOS_DSCP_EXP_OUT_OF_RANGE_ERR, 20, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_php_remark_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_qos_params_php_remark_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_QOS_PHP_REMARK_KEY, php_key, 10, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_php_remark_get_verify()", 0, 0);
}

/*********************************************************************
*     Sets how to remark QoS parameters upon PHP operation.
 *     When uniform pop performed the dscp_exp value is
 *     remarked.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_qos_params_php_remark_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_OUT uint32                                  *dscp_exp
  )
{
  uint32
    index,
    key,
    tmp = 0,
    res = SOC_SAND_OK;
  soc_reg_above_64_val_t ipv4_reg, ipv4_fld;
  soc_reg_above_64_val_t ipv6_reg, ipv6_fld;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(php_key);
  SOC_REG_ABOVE_64_CLEAR(ipv4_reg);
  SOC_REG_ABOVE_64_CLEAR(ipv4_fld);
  SOC_REG_ABOVE_64_CLEAR(ipv6_reg);
  SOC_REG_ABOVE_64_CLEAR(ipv6_fld);

  switch (php_key->php_type)
  {
  case SOC_PPC_EG_QOS_UNIFORM_PHP_POP_INTO_IPV4:
    key = PP_ARAD_EPNI_IPV4_EXP_TO_TOS_MAP_REGS_KEY_ENTRY(php_key->exp_map_profile,php_key->exp);
    index = key * ARAD_PP_EG_QOS_IPV4_OUT_EXP_NOF_BITS;
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 5, exit, READ_EPNI_IPV4_EXP_TO_TOS_MAPr(unit, REG_PORT_ANY, ipv4_reg));
    soc_reg_above_64_field_get(unit, EPNI_IPV4_EXP_TO_TOS_MAPr, ipv4_reg, IPV4_EXP_TO_TOS_MAPf, ipv4_fld);
    res = soc_sand_bitstream_get_any_field(ipv4_fld, index, (ARAD_PP_EG_QOS_IPV4_OUT_EXP_NOF_BITS), &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit); 
    break;

  case SOC_PPC_EG_QOS_UNIFORM_PHP_POP_INTO_IPV6:
    key = PP_ARAD_EPNI_IPV6_EXP_TO_TC_MAP_REGS_KEY_ENTRY(php_key->exp_map_profile,php_key->exp);
    index = key * ARAD_PP_EG_QOS_IPV6_OUT_EXP_NOF_BITS;
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 15, exit, READ_EPNI_IPV6_EXP_TO_TC_MAPr(unit, REG_PORT_ANY, ipv6_reg));
    soc_reg_above_64_field_get(unit, EPNI_IPV6_EXP_TO_TC_MAPr, ipv6_reg, IPV6_EXP_TO_TC_MAPf, ipv6_fld);
    res = soc_sand_bitstream_get_any_field(ipv6_fld, index, (ARAD_PP_EG_QOS_IPV6_OUT_EXP_NOF_BITS), &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit); 
    break;
  default:
       SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_QOS_PHP_TYPE_OUT_OF_RANGE_ERR, 10, exit);
    break;
  }

  *dscp_exp = tmp;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_php_remark_get_unsafe()", 0, 0);
}

#ifdef BCM_88660

/**
 * Copy @fields one by one into a uint32 and return it. 
 *  
 * @param fields The fields to be copied.
 * @param sizes The sizes of the fields (IN BITS).
 * @param num Number of fields. 
 *  
 * @return The copied register. 
 */
static uint32 _arad_pp_eg_qos_fields_to_reg(uint32 fields[], uint32 sizes[], uint32 num)
{
  uint32 i;
  uint32 reg = 0;
  uint32 curr = 0;

  for (i = 0; i < num; i++) {
    SHR_BITCOPY_RANGE(&reg, curr, fields+i, 0, sizes[i]);
    curr += sizes[i];
  }

  return reg;
}

uint32
  arad_pp_eg_qos_params_marking_set_unsafe(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  )
{
  uint32
    res = SOC_SAND_OK;

  uint32 key = 0;
  uint32 val = 0;

  uint32 key_fields[4]; 
  uint32 key_sizes[4];
  uint32 key_field_num = 4;
  soc_mem_t mem;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  key_fields[0] = qos_key->marking_profile;
  key_fields[1] = qos_key->in_lif_profile;
  key_fields[2] = qos_key->resolved_dp_ndx;
  key_fields[3] = qos_key->tc_ndx;

  key_sizes[0] = ARAD_PP_EG_QOS_MARKING_PROFILE_NOF_BITS(unit, qos_key->dp_map_disabled);
  key_sizes[1] = ARAD_PP_EG_QOS_INLIF_PROFILE_NOF_BITS(unit, qos_key->dp_map_disabled);
  key_sizes[2] = ARAD_PP_EG_QOS_RESOLVED_DP_NOF_BITS(unit, qos_key->dp_map_disabled);
  key_sizes[3] = ARAD_PP_EG_QOS_TC_NDX_NOF_BITS;

  key = _arad_pp_eg_qos_fields_to_reg(key_fields, key_sizes, key_field_num);

  mem = ARAD_PP_IP_TOS_MARKING_TABLE(unit);
  soc_mem_field32_set(unit, mem, &val, IP_TOS_MARKING_DSCPf, qos_params->ip_dscp);
  soc_mem_field32_set(unit, mem, &val, IP_TOS_MARKING_EXPf, qos_params->mpls_exp);

  
  res = soc_mem_write(unit, mem, MEM_BLOCK_ANY, key, &val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_marking_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_qos_params_marking_set_verify(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(qos_key);
  SOC_SAND_CHECK_NULL_INPUT(qos_params);

  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_QOS_MARKING_KEY, unit, qos_key, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_QOS_MARKING_PARAMS, qos_params, 20, exit);
  
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_marking_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_qos_params_marking_get_unsafe(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_OUT SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  )
{
  uint32
    res = SOC_SAND_OK;

  uint32 key = 0;
  uint32 val = 0;

  uint32 key_fields[4];
  uint32 key_sizes[4];
  uint32 key_field_num = 4;
  soc_mem_t mem;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  key_fields[0] = qos_key->marking_profile;
  key_fields[1] = qos_key->in_lif_profile;
  key_fields[2] = qos_key->resolved_dp_ndx;
  key_fields[3] = qos_key->tc_ndx;

  key_sizes[0] = ARAD_PP_EG_QOS_MARKING_PROFILE_NOF_BITS(unit, qos_key->dp_map_disabled);
  key_sizes[1] = ARAD_PP_EG_QOS_INLIF_PROFILE_NOF_BITS(unit, qos_key->dp_map_disabled);
  key_sizes[2] = ARAD_PP_EG_QOS_RESOLVED_DP_NOF_BITS(unit, qos_key->dp_map_disabled);
  key_sizes[3] = ARAD_PP_EG_QOS_TC_NDX_NOF_BITS;

  key = _arad_pp_eg_qos_fields_to_reg(key_fields, key_sizes, key_field_num);

  mem = ARAD_PP_IP_TOS_MARKING_TABLE(unit);
  
  res = soc_mem_read(unit, mem, MEM_BLOCK_ANY, key, &val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  qos_params->ip_dscp = soc_mem_field32_get(unit, mem, &val, IP_TOS_MARKING_DSCPf);
  qos_params->mpls_exp = soc_mem_field32_get(unit, mem, &val, IP_TOS_MARKING_EXPf);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_marking_get_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_qos_params_marking_get_verify(
    SOC_SAND_IN     int unit,
    SOC_SAND_IN     SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_OUT    SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_VERIFY);

  SOC_SAND_CHECK_NULL_INPUT(qos_key);
  SOC_SAND_CHECK_NULL_INPUT(qos_params);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_QOS_MARKING_KEY, unit, qos_key, 10, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_marking_get_verify()", 0, 0);
}

uint32
  arad_pp_eg_qos_global_info_set_verify(
   SOC_SAND_IN int unit,
   SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;
 
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(info);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_QOS_GLOBAL_INFO, info, 10, exit);
   
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_global_info_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_qos_global_info_set_unsafe(
   SOC_SAND_IN int unit,
   SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info
  )
{
  uint32
      res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_CFG_ENABLE_DSCP_MARKINGr, SOC_CORE_ALL, 0, CFG_ENABLE_DSCP_MARKINGf,  info->in_lif_profile_bitmap));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_CFG_MARKING_DP_MAPr, SOC_CORE_ALL, 0, CFG_MARKING_DP_MAPf,  info->resolved_dp_bitmap));
  if (SOC_IS_JERICHO_B0(unit) || SOC_IS_QMX_B0(unit)) {
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_CFG_IN_LIF_PROFILE_MAPr, SOC_CORE_ALL, 0, CFG_IN_LIF_PROFILE_MAPf,  info->resolved_in_lif_profile_bitmap));
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_DP_MAP_FIX_ENABLEDr, SOC_CORE_ALL, 0, DP_MAP_FIX_ENABLEDf,  info->dp_map_mode));
  } else if (SOC_IS_JERICHO_PLUS(unit)) {
    uint32 dp_map_disable = ((info->dp_map_mode)? 0 : 1);
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_CFG_MARKING_DP_MAPr, SOC_CORE_ALL, 0, CFG_MARKING_DP_MAP_DISABLEf,  dp_map_disable));
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_global_info_set_unsafe()", 0, 0);
}

uint32 
  arad_pp_eg_qos_global_info_get_verify(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT SOC_PPC_EG_QOS_GLOBAL_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;
 
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(info);
   
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_global_info_get_verify()", 0, 0);
}

uint32 
  arad_pp_eg_qos_global_info_get_unsafe(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT SOC_PPC_EG_QOS_GLOBAL_INFO *info
  )
{
  uint32
      res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_CFG_ENABLE_DSCP_MARKINGr, SOC_CORE_ALL, 0, CFG_ENABLE_DSCP_MARKINGf, &info->in_lif_profile_bitmap));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_CFG_MARKING_DP_MAPr, SOC_CORE_ALL, 0, CFG_MARKING_DP_MAPf, &info->resolved_dp_bitmap));
  if (SOC_IS_JERICHO_B0(unit) || SOC_IS_QMX_B0(unit)) {
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_CFG_IN_LIF_PROFILE_MAPr, SOC_CORE_ALL, 0, CFG_IN_LIF_PROFILE_MAPf, &info->resolved_in_lif_profile_bitmap));
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_DP_MAP_FIX_ENABLEDr, SOC_CORE_ALL, 0, DP_MAP_FIX_ENABLEDf, &info->dp_map_mode));
  } else if (SOC_IS_JERICHO_PLUS(unit)) {
    uint32 dp_map_disable = 0;
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_CFG_MARKING_DP_MAPr, SOC_CORE_ALL, 0, CFG_MARKING_DP_MAP_DISABLEf,  &dp_map_disable));
	info->dp_map_mode = (dp_map_disable ? 0 : 1);
  }
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_global_info_get_unsafe()", 0, 0);
}

#endif

/*********************************************************************
*     Remark QoS parameters, i.e. map in-dscp/exp and DP to
 *     out-dscp/exp in order to be set in outgoing packet
 *     forwarding headers.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_qos_params_remark_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  )
{
  ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA
    ip_tbl_data;
  ARAD_PP_EPNI_EXP_REMARK_TBL_DATA
    mpls_tbl_data;
  uint32
    tmp,
    index,
    entry_offset;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_REMARK_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(in_qos_key);
  SOC_SAND_CHECK_NULL_INPUT(out_qos_params);
  
  /* ipv4 */
  entry_offset = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(FALSE, in_qos_key->dp,in_qos_key->remark_profile, in_qos_key->in_dscp_exp);
  /* each entry_offset contains 4 dscp-exp-remark values */
  index = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_qos_key->in_dscp_exp) * ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS;

  res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit, entry_offset, &ip_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  tmp = out_qos_params->ipv4_tos;
  res = soc_sand_bitstream_set_any_field(&tmp , index, ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS, &ip_tbl_data.dscp_remark_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_epni_dscp_remark_tbl_set_unsafe(unit, entry_offset, &ip_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  
  /* ipv6 */
  entry_offset = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(TRUE, in_qos_key->dp,in_qos_key->remark_profile, in_qos_key->in_dscp_exp);
  /* each entry_offset contains 4 dscp-exp-remark values */
  index = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_qos_key->in_dscp_exp) * ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS;

  res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit, entry_offset, &ip_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  tmp = out_qos_params->ipv6_tc;
  res = soc_sand_bitstream_set_any_field(&tmp , index, ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS, &ip_tbl_data.dscp_remark_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  
  res = arad_pp_epni_dscp_remark_tbl_set_unsafe(unit, entry_offset, &ip_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  
  /* mpls */
  entry_offset = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(in_qos_key->dp,in_qos_key->remark_profile, in_qos_key->in_dscp_exp);
  /* each entry_offset contains 4 dscp-exp-remark values */
  index = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_qos_key->in_dscp_exp) * ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS;

  res = arad_pp_epni_exp_remark_tbl_get_unsafe(unit, entry_offset, &mpls_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

  tmp = out_qos_params->mpls_exp;
  res = soc_sand_bitstream_set_any_field(&tmp , index, ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS, &mpls_tbl_data.exp_remark_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  res = arad_pp_epni_exp_remark_tbl_set_unsafe(unit, entry_offset, &mpls_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_remark_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_qos_params_remark_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_REMARK_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_QOS_MAP_KEY, in_qos_key, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_QOS_PARAMS, out_qos_params, 20, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_remark_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_qos_params_remark_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_REMARK_GET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_QOS_MAP_KEY, in_qos_key, 10, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_remark_get_verify()", 0, 0);
}

/*********************************************************************
*     Remark QoS parameters, i.e. map in-dscp/exp and DP to
 *     out-dscp/exp in order to be set in outgoing packet
 *     headers.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_qos_params_remark_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_OUT SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  )
{
  ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA
    ip_tbl_data;
  ARAD_PP_EPNI_EXP_REMARK_TBL_DATA
    mpls_tbl_data;
  uint32
    tmp = 0,
    index,
    entry_offset;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_QOS_PARAMS_REMARK_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(in_qos_key);
  SOC_SAND_CHECK_NULL_INPUT(out_qos_params);

  SOC_PPC_EG_QOS_PARAMS_clear(out_qos_params);

  /* ipv4 */
  entry_offset = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(FALSE, in_qos_key->dp,in_qos_key->remark_profile, in_qos_key->in_dscp_exp);
  /* each entry_offset contains 4 dscp-exp-remark values */
  index = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_qos_key->in_dscp_exp) * ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS;

  res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit, entry_offset, &ip_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = soc_sand_bitstream_get_any_field(&ip_tbl_data.dscp_remark_data, index, ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS, &tmp);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  out_qos_params->ipv4_tos = (SOC_SAND_PP_IPV4_TOS)tmp;

   /* ipv6 */
  entry_offset = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(TRUE, in_qos_key->dp,in_qos_key->remark_profile, in_qos_key->in_dscp_exp);
  /* each entry_offset contains 4 dscp-exp-remark values */
  index = ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_qos_key->in_dscp_exp) * ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS;

  res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit, entry_offset, &ip_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = soc_sand_bitstream_get_any_field(&ip_tbl_data.dscp_remark_data, index, ARAD_PP_EPNI_DSCP_REMARK_TBL_NOF_BITS,&tmp);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  out_qos_params->ipv6_tc = (SOC_SAND_PP_IPV6_TC)tmp;
  
 /* mpls */
  entry_offset = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_ENTRY_OFFSET(in_qos_key->dp,in_qos_key->remark_profile, in_qos_key->in_dscp_exp);
  /* each entry_offset contains 4 dscp-exp-remark values */
  index = ARAD_PP_EPNI_EXP_REMARK_TBL_DATA_KEY_INDEX_OFFSET(in_qos_key->in_dscp_exp) * ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS;

  res = arad_pp_epni_exp_remark_tbl_get_unsafe(unit, entry_offset, &mpls_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

  res = soc_sand_bitstream_get_any_field(&mpls_tbl_data.exp_remark_data, index, ARAD_PP_EPNI_EXP_REMARK_TBL_NOF_BITS, &tmp);
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  out_qos_params->mpls_exp = (SOC_SAND_PP_MPLS_EXP)tmp;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_params_remark_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Remark QoS parameters, i.e. map in-dscp/exp, remark_profile and 
 *     header pkt type to out-dscp/exp in order to be set in outgoing
 *     packet encapsulated headers.
 *     Details: in the H file. (search for prototype)
 *    Valid only for ARAD.
 *********************************************************************/
uint32
  arad_pp_eg_encap_qos_params_remark_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params
  )
{
  uint32
    index,
    index_offset,
    entry_offset;
  uint32
    tmp,
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_TBL_DATA    
    mpls_to_dscp;
  ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_TBL_DATA    
    mpls_to_exp;
  ARAD_PP_EPNI_REMARK_IPV4_TO_DSCP_TBL_DATA    
    ipv4_to_dscp;
  ARAD_PP_EPNI_REMARK_IPV4_TO_EXP_TBL_DATA     
    ipv4_to_exp; 
  ARAD_PP_EPNI_REMARK_IPV6_TO_DSCP_TBL_DATA    
    ipv6_to_dscp;
  ARAD_PP_EPNI_REMARK_IPV6_TO_EXP_TBL_DATA     
    ipv6_to_exp;
  uint32
    ecn_capable, ecn_congestion,
    ecn_ip_enabled = 1, ecn_mpls_enabled = 1, ecn_is_enabled;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(in_encap_qos_key);
  SOC_SAND_CHECK_NULL_INPUT(out_encap_qos_params);

  if (SOC_IS_ARAD_B0_AND_ABOVE(unit)) 
  {
    if (in_encap_qos_key->pkt_hdr_type == SOC_PPC_PKT_HDR_TYPE_IPV4 ||
      in_encap_qos_key->pkt_hdr_type == SOC_PPC_PKT_HDR_TYPE_IPV6) 
    {
      /* Remark profile is limited to 14 because of ECN functionality */
      SOC_SAND_ERR_IF_ABOVE_MAX(in_encap_qos_key->remark_profile, ARAD_PP_EG_QOS_IP_REMARK_PROFILE_INDEX_MAX, ARAD_PP_REMARK_PROFILE_OUT_OF_RANGE_ERR, 10, exit);
    }
  }

  switch (in_encap_qos_key->pkt_hdr_type) 
  {
  case (SOC_PPC_PKT_HDR_TYPE_IPV4):
    /* set remark ipv4 to exp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

    res = arad_pp_epni_remark_ipv4_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    tmp = out_encap_qos_params->mpls_exp;
    res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &ipv4_to_exp.dscp_exp_remark_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    res = arad_pp_epni_remark_ipv4_to_exp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    
    /* set remark ipv4 to dscp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

    res = arad_pp_epni_remark_ipv4_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    tmp = out_encap_qos_params->ip_dscp;
    res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &ipv4_to_dscp.dscp_exp_remark_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    res = arad_pp_epni_remark_ipv4_to_dscp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
   break;
  
  case (SOC_PPC_PKT_HDR_TYPE_IPV6):
    /* set remark ipv6 to exp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

    res = arad_pp_epni_remark_ipv6_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    tmp = out_encap_qos_params->mpls_exp;
    res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &ipv6_to_exp.dscp_exp_remark_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    res = arad_pp_epni_remark_ipv6_to_exp_tbl_set_unsafe(unit, entry_offset, &ipv6_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

    
    /* set remark ipv6 to dscp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

    res = arad_pp_epni_remark_ipv6_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

    tmp = out_encap_qos_params->ip_dscp;
    res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &ipv6_to_dscp.dscp_exp_remark_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);

    res = arad_pp_epni_remark_ipv6_to_dscp_tbl_set_unsafe(unit, entry_offset, &ipv6_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);
   break;
  case (SOC_PPC_PKT_HDR_TYPE_MPLS):
    /* set remark mpls to exp table */
    entry_offset = ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index_offset = 0;

    res = arad_pp_epni_remark_mpls_to_exp_tbl_get_unsafe(unit, entry_offset, &mpls_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);

    tmp = out_encap_qos_params->mpls_exp;
    res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &mpls_to_exp.dscp_exp_remark_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);

    res = arad_pp_epni_remark_mpls_to_exp_tbl_set_unsafe(unit, entry_offset, &mpls_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);

    
    /* set remark mpls to dscp table */
    entry_offset = ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index_offset = 0;

    res = arad_pp_epni_remark_mpls_to_dscp_tbl_get_unsafe(unit, entry_offset, &mpls_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);

    tmp = out_encap_qos_params->ip_dscp;
    res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &mpls_to_dscp.dscp_exp_remark_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit);

    res = arad_pp_epni_remark_mpls_to_dscp_tbl_set_unsafe(unit, entry_offset, &mpls_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 190, exit);
   break;
  case (SOC_PPC_PKT_HDR_TYPE_ETH):
    /* set remark ethernet to tunnel valid for ARAD-B0 and above */
    if (SOC_IS_ARAD_B0_AND_ABOVE(unit))
    {
      /* If ECN is disabled by SOC property for IP, force FTMH.ECN-Capable=0 behaviour */
      if (!(SOC_DPP_CONFIG(unit)->qos.ecn_ip_enabled)) {
          ecn_ip_enabled = 0;
      }
      /* If ECN is disabled by SOC property for MPLS, force FTMH.ECN-Capable=0 behaviour */
      if (!(SOC_DPP_CONFIG(unit)->qos.ecn_mpls_enabled)) {
          ecn_mpls_enabled = 0;
      }
      
      /* When ECN is not enabled, Jericho support 16 remark profile.*/
      if (SOC_IS_JERICHO(unit)) {
          res = jer_itm_enable_ecn_get(unit, &ecn_is_enabled);
          SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

          if (!ecn_is_enabled) {
              /*set profile map first*/
              uint32 reg_val, fld_val;
              uint32 index, index_offset;

			  SOC_SAND_SOC_IF_ERROR_RETURN(res, 201, exit, READ_EPNI_REMARK_PROFILE_CONFIGURATIONr(unit, REG_PORT_ANY, &reg_val));
              fld_val = soc_reg_field_get(unit, EPNI_REMARK_PROFILE_CONFIGURATIONr, reg_val,  LINK_LAYER_REMARK_PROFILE_MAPf);

              index = in_encap_qos_key->remark_profile & ARAD_PP_EPNI_REMARK_ETH_REMARK_PROFILE_MASK;
              index_offset = index * ARAD_PP_EPNI_REMARK_PROFILE_NOF_BITS;
              res = soc_sand_bitstream_set_any_field(&in_encap_qos_key->remark_profile, index_offset, ARAD_PP_EPNI_REMARK_PROFILE_NOF_BITS, &fld_val);
              SOC_SAND_CHECK_FUNC_RESULT(res, 202, exit);

              soc_reg_field_set(unit, EPNI_REMARK_PROFILE_CONFIGURATIONr, &reg_val, LINK_LAYER_REMARK_PROFILE_MAPf, fld_val);
              SOC_SAND_SOC_IF_ERROR_RETURN(res, 203, exit, WRITE_EPNI_REMARK_PROFILE_CONFIGURATIONr(unit, REG_PORT_ANY, reg_val));

              if (ARAD_PP_EPNI_REMARK_ETH_REMARK_PROFILE_MSB(in_encap_qos_key->remark_profile)) {
                  /* set remark ethernet to dscp ip tunnel */
                  entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->dp, in_encap_qos_key->in_dscp_exp);
                  index_offset = 0;
                  
                  /* remark ipv4 table */
                  res = arad_pp_epni_remark_ipv4_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_dscp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 210, exit);

                  index = (in_encap_qos_key->in_dscp_exp & 0x3); /* 2b' LSB */ 
                  index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;                
                  tmp = out_encap_qos_params->ip_dscp;

                  res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &ipv4_to_dscp.dscp_exp_remark_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit);
  
                  res = arad_pp_epni_remark_ipv4_to_dscp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_dscp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 230, exit);

                  /* set remark ethernet to exp mpls tunnel */ 
                  entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->dp);
                  index_offset = 0;
                  
                  /* remark ipv4 table */
                  res = arad_pp_epni_remark_ipv4_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_exp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 240, exit);
                  
                  index = in_encap_qos_key->in_dscp_exp;    
                  index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;
                  tmp = out_encap_qos_params->mpls_exp;
                  
                  res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &ipv4_to_exp.dscp_exp_remark_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
                  
                  res = arad_pp_epni_remark_ipv4_to_exp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_exp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 260, exit);
              } else {
                  /* set remark ethernet to dscp ip tunnel */
                  entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->dp, in_encap_qos_key->in_dscp_exp);
                  index_offset = 0;
                  
                  /* remark ipv4 table */
                  res = arad_pp_epni_remark_ipv6_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_dscp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 210, exit);

                  index = (in_encap_qos_key->in_dscp_exp & 0x3); /* 2b' LSB */ 
                  index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;                
                  tmp = out_encap_qos_params->ip_dscp;

                  res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &ipv6_to_dscp.dscp_exp_remark_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit);
  
                  res = arad_pp_epni_remark_ipv6_to_dscp_tbl_set_unsafe(unit, entry_offset, &ipv6_to_dscp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 230, exit);

                  /* set remark ethernet to exp mpls tunnel */ 
                  entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->dp);
                  index_offset = 0;
                  
                  /* remark ipv4 table */
                  res = arad_pp_epni_remark_ipv6_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_exp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 240, exit);
                  
                  index = in_encap_qos_key->in_dscp_exp;    
                  index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;
                  tmp = out_encap_qos_params->mpls_exp;
                  
                  res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &ipv6_to_exp.dscp_exp_remark_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
                  
                  res = arad_pp_epni_remark_ipv6_to_exp_tbl_set_unsafe(unit, entry_offset, &ipv6_to_exp);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 260, exit);
              }
              goto exit;  
          }
      }

      /* run over all possibilities of ECN capable and congestion */
      for (ecn_capable = 0; ecn_capable < 2; ecn_capable++) {
          for (ecn_congestion = 0; ecn_congestion < 2; ecn_congestion++) {

              /* set remark ethernet to dscp ip tunnel */
              entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_DSCP_ECN_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, ecn_capable, in_encap_qos_key->dp, in_encap_qos_key->in_dscp_exp);
              index_offset = 0;

              /* For SOC property ip_ecn_mode = 1
                 FTMH.CNI FTMH.ECN-Capable IP-Header.ToS[1:0]  Out-DSCP-EXP[1:0]
                 1        1                x                   3
                 0/1      0                x                   x
                 0        1                3                   3
                 0        1                0-2                 1/2 */
                 
              /* For SOC propery ip_ecn_mode = 0
                 FTMH.CNI  FTMH.ECN-Capable  IP-Header.ToS[1:0]  Out-DSCP-EXP[1:0]
                 0/1       0/1               x                   x */

              if (ecn_congestion) 
              {
                /* remark ipv4 table */
                res = arad_pp_epni_remark_ipv4_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_dscp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 650, exit);

                index = (in_encap_qos_key->in_dscp_exp & 0x3); /* 2b' LSB */ 
                index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

                if ((ecn_capable == 1) && ecn_ip_enabled) {
                    /* case 1: {FTMH.CNI = 1, FTMH.ECN-Capable = 1, TOS[1:0]} -> 3 */
                    tmp = out_encap_qos_params->ip_dscp | 0x3;
                }
                else {
                    /* case 2-2: {FTMH.CNI = 1, FTMH.ECN-Capable = 0, TOS[1:0]} -> TOS[1:0] */
                    tmp = out_encap_qos_params->ip_dscp;
                }
                res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &ipv4_to_dscp.dscp_exp_remark_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 660, exit);

                res = arad_pp_epni_remark_ipv4_to_dscp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_dscp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 670, exit);
                
              }
              else
              {
                /* remark ipv6 table */
                res = arad_pp_epni_remark_ipv6_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_dscp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 710, exit);

                index = (in_encap_qos_key->in_dscp_exp & 0x3); /* 2b' LSB */ 
                index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

                if ((ecn_capable == 1) && ecn_ip_enabled) {
                    /* cases 3,4: FTMH.CNI = 0, FTMH.ECN-Capable = 1 */
                    if (out_encap_qos_params->ip_dscp == 0) {
                        /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, TOS[1:0] = 0} -> 1 */
                        tmp = out_encap_qos_params->ip_dscp | 0x1;
                    }
                    else { /* ip_dscp = 1-3 */
                        /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, TOS[1:0] = 1-3} -> 1-3 */
                        tmp = out_encap_qos_params->ip_dscp;
                    }
                }
                else {
                    /* case 2-1: {FTMH.CNI = 0, FTMH.ECN-Capable = 0, TOS[1:0]} -> TOS[1:0] */
                    tmp = out_encap_qos_params->ip_dscp;
                }
                res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &ipv6_to_dscp.dscp_exp_remark_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 720, exit);

                res = arad_pp_epni_remark_ipv6_to_dscp_tbl_set_unsafe(unit, entry_offset, &ipv6_to_dscp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 730, exit);
              }
             
              /* set remark ethernet to exp mpls tunnel */ 
              entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_EXP_ECN_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, ecn_capable, in_encap_qos_key->dp);
              index_offset = 0;

              /* For SOC property mpls_ecn_mode = 1
                 2 bits mode:
                 FTMH.CNI  FTMH.ECN-Capable  Out-DSCP-EXP[2:0]  Out-DSCP-EXP[2:0] (new)
                 1         1                 xxx                x11
                 1/0       0                 xxx                xxx
                 0         1                 x11                x11
                 0         1                 x10, x01, x00      x01/x10 */

              /* 1 bit mode:
                 FTMH.CNI  FTMH.ECN-Capable  Out-DSCP-EXP[2:0]  Out-DSCP-EXP[2:0] (new)
                    1          1                  xxx                      xx1
                    0          1                  xxx                      xxx */

              /* For SOC property mpls_ecn_mode = 0
                 1/2 bits mode:
                 FTMH.CNI  FTMH.ECN-Capable  Out-DSCP-EXP[2:0]  Out-DSCP-EXP[2:0] (new)
                 0/1       0/1               xxx                xxx  */
              if (ecn_congestion) 
              {
                /* remark ipv4 table */
                res = arad_pp_epni_remark_ipv4_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_exp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 920, exit);

                index = in_encap_qos_key->in_dscp_exp;    
                index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

                if (!(SOC_DPP_CONFIG(unit)->qos.ecn_mpls_one_bit_mode)) { /* ECN for MPLS is in 2 bits mode */
                    if (ecn_capable && ecn_mpls_enabled) {
                        /* case 1: {FTMH.CNI = 1, FTMH.ECN-Capable = 1, DSCP-EXP[2:0]} -> x11 */
                        tmp = out_encap_qos_params->mpls_exp | 0x3;
                    }
                    else {
                        /* case 2-2: {FTMH.CNI = 1, FTMH.ECN-Capable = 0, DSCP-EXP[2:0]} -> xxx */
                        tmp = out_encap_qos_params->mpls_exp;
                    }
                }
                else { /* 1 bit mode */
                    /* {FTMH.CNI = 1, FTMH.ECN-Capable = 1 (always), DSCP-EXP[2:0]} -> xx1 */
                     if (ecn_mpls_enabled) {
                        /* case 1: {FTMH.CNI = 1, FTMH.ECN-Capable = 1, DSCP-EXP[2:0]} -> x11 */
                        tmp = out_encap_qos_params->mpls_exp | 0x1;
                    }
                    else {
                        /* SOC property mpls_ecn_mode = 0 */
                        tmp = out_encap_qos_params->mpls_exp;
                    }
                }
                res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &ipv4_to_exp.dscp_exp_remark_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 930, exit);

                res = arad_pp_epni_remark_ipv4_to_exp_tbl_set_unsafe(unit, entry_offset, &ipv4_to_exp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 940, exit);
              }
              else
              {
                /* remark ipv6 table */
                res = arad_pp_epni_remark_ipv6_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_exp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 880, exit);

                index = in_encap_qos_key->in_dscp_exp;    
                index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

                if (!(SOC_DPP_CONFIG(unit)->qos.ecn_mpls_one_bit_mode)) { /* ECN for MPLS is in 2 bits mode */
                    if (ecn_capable && ecn_mpls_enabled) {
                        /* cases 3,4: FTMH.CNI = 0, FTMH.ECN-Capable = 1 */
                        if ((out_encap_qos_params->mpls_exp & 0x3) == 0) {
                            /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, DSCP-EXP[2:0] = x00} -> x01 */
                            tmp = out_encap_qos_params->mpls_exp | 0x1;
                        }
                        else { /* out_encap_qos_params->mpls_exp = 1-3 */
                            /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1, DSCP-EXP[2:0] = x10/x01/x11} -> x10/x01/x11 */
                            tmp = out_encap_qos_params->mpls_exp | 0x3;   
                        }
                    }
                    else {
                        /* case 2-1: {FTMH.CNI = 0, FTMH.ECN-Capable = 0, DSCP-EXP[2:0]} -> xxx */
                        tmp = out_encap_qos_params->mpls_exp;
                    }
                }
                else { /* 1 bit mode */
                    /* {FTMH.CNI = 0, FTMH.ECN-Capable = 1 (always), DSCP-EXP[2:0]} -> xxx */
                    tmp = out_encap_qos_params->mpls_exp;
                }
                res = soc_sand_bitstream_set_any_field(&tmp, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &ipv6_to_exp.dscp_exp_remark_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 890, exit);

                res = arad_pp_epni_remark_ipv6_to_exp_tbl_set_unsafe(unit, entry_offset, &ipv6_to_exp);
                SOC_SAND_CHECK_FUNC_RESULT(res, 900, exit);
              }
          }
      }
    }
    else
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_OUT_OF_RANGE_ERR, 1500, exit);
    }
   break;
  default:
   break;
  }
      
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_qos_params_remark_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_encap_qos_params_remark_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_IN SOC_PPC_EG_ENCAP_QOS_PARAMS                  *out_encap_qos_params
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_QOS_MAP_KEY, unit, in_encap_qos_key, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_QOS_PARAMS, out_encap_qos_params, 40, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_qos_params_remark_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_encap_qos_params_remark_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_GET_VERIFY);

  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_QOS_MAP_KEY, unit, in_encap_qos_key, 10, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_qos_params_remark_get_verify()", 0, 0);
}

/*********************************************************************
*     Remark QoS parameters, i.e. map in-dscp/exp, remark_profile and 
 *     header pkt type to out-dscp/exp in order to be set in outgoing
 *     packet encapsulated headers.
 *     Details: in the H file. (search for prototype)
 *    Valid only for ARAD.
 *********************************************************************/
uint32
  arad_pp_eg_encap_qos_params_remark_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params
  )
{
  uint32
    index,
    index_offset,
    entry_offset;
  uint32
    tmp,
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_TBL_DATA    
    mpls_to_dscp;
  ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_TBL_DATA    
    mpls_to_exp;
  ARAD_PP_EPNI_REMARK_IPV4_TO_DSCP_TBL_DATA    
    ipv4_to_dscp;
  ARAD_PP_EPNI_REMARK_IPV4_TO_EXP_TBL_DATA     
    ipv4_to_exp; 
  ARAD_PP_EPNI_REMARK_IPV6_TO_DSCP_TBL_DATA    
    ipv6_to_dscp;
  ARAD_PP_EPNI_REMARK_IPV6_TO_EXP_TBL_DATA     
    ipv6_to_exp;


  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(in_encap_qos_key);
  SOC_SAND_CHECK_NULL_INPUT(out_encap_qos_params);

  switch (in_encap_qos_key->pkt_hdr_type) 
  {
  case (SOC_PPC_PKT_HDR_TYPE_IPV4):
    /* get remark ipv4 to exp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

    res = arad_pp_epni_remark_ipv4_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = soc_sand_bitstream_get_any_field(&ipv4_to_exp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    out_encap_qos_params->mpls_exp = (SOC_SAND_PP_MPLS_EXP)tmp;
    /* get remark ipv4 to dscp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

    res = arad_pp_epni_remark_ipv4_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv4_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    res = soc_sand_bitstream_get_any_field(&ipv4_to_dscp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    out_encap_qos_params->ip_dscp = (SOC_SAND_PP_IPV4_TOS)tmp;
   break;
  case (SOC_PPC_PKT_HDR_TYPE_IPV6):
    /* get remark ipv6 to exp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_EXP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

    res = arad_pp_epni_remark_ipv6_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    res = soc_sand_bitstream_get_any_field(&ipv6_to_exp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    out_encap_qos_params->mpls_exp = (SOC_SAND_PP_MPLS_EXP)tmp;
    /* get remark ipv6 to dscp table */
    entry_offset = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index = ARAD_PP_EPNI_REMARK_IP_TO_DSCP_KEY_INDEX_OFFSET(in_encap_qos_key->in_dscp_exp);
    index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

    res = arad_pp_epni_remark_ipv6_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

    res = soc_sand_bitstream_get_any_field(&ipv6_to_dscp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);
    out_encap_qos_params->ip_dscp = (SOC_SAND_PP_IPV4_TOS)tmp;
   break;
  
  case (SOC_PPC_PKT_HDR_TYPE_MPLS):
    /* get remark mpls to exp table */
    entry_offset = ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index_offset = 0;

    res = arad_pp_epni_remark_mpls_to_exp_tbl_get_unsafe(unit, entry_offset, &mpls_to_exp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);

    res = soc_sand_bitstream_get_any_field(&mpls_to_exp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);

    out_encap_qos_params->mpls_exp = (SOC_SAND_PP_MPLS_EXP)tmp;
    /* get remark mpls to dscp table */
    entry_offset = ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, in_encap_qos_key->in_dscp_exp);
    index_offset = 0;

    res = arad_pp_epni_remark_mpls_to_dscp_tbl_get_unsafe(unit, entry_offset, &mpls_to_dscp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);

    res = soc_sand_bitstream_get_any_field(&mpls_to_dscp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &tmp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit);
    out_encap_qos_params->ip_dscp = (SOC_SAND_PP_IPV4_TOS)tmp;
   break;
  case (SOC_PPC_PKT_HDR_TYPE_ETH):
    if (SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
        /* get remark eth to mpls exp table for ECN-Capable=0 */
        entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_EXP_ECN_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, 0, in_encap_qos_key->dp);
        index = in_encap_qos_key->in_dscp_exp;
        index_offset = index * ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS;

        res = arad_pp_epni_remark_ipv6_to_exp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_exp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        res = soc_sand_bitstream_get_any_field(&ipv6_to_exp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_EXP_NOF_BITS, &tmp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

        out_encap_qos_params->mpls_exp = (SOC_SAND_PP_MPLS_EXP)tmp;

        /* get remark eth to dscp table for ECN-Capable=0*/
        entry_offset = ARAD_PP_EPNI_REMARK_ETH_TO_DSCP_ECN_KEY_ENTRY_OFFSET(in_encap_qos_key->remark_profile, 0, in_encap_qos_key->dp, in_encap_qos_key->in_dscp_exp);
        index = (in_encap_qos_key->in_dscp_exp & 0x3); /* 2b' LSB */
        index_offset = index * ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS;

        res = arad_pp_epni_remark_ipv6_to_dscp_tbl_get_unsafe(unit, entry_offset, &ipv6_to_dscp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

        res = soc_sand_bitstream_get_any_field(&ipv6_to_dscp.dscp_exp_remark_data, index_offset, ARAD_PP_EG_ENCAP_OUT_DSCP_NOF_BITS, &tmp);
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

        out_encap_qos_params->ip_dscp = (SOC_SAND_PP_IPV4_TOS)tmp;
    }
   break;
  default:
   break;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_qos_params_remark_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_eg_qos module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_eg_qos_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_eg_qos;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_eg_qos module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_eg_qos_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_eg_qos;
}

uint32
    SOC_PPC_EG_QOS_MAP_KEY_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->in_dscp_exp, ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX, ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->dp, ARAD_PP_EG_QOS_DP_MAX, SOC_SAND_PP_DP_OUT_OF_RANGE_ERR, 11, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->remark_profile, ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX, ARAD_PP_REMARK_PROFILE_OUT_OF_RANGE_ERR, 12, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_MAP_KEY_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_QOS_MAP_KEY_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY *info
  )
{
  uint32 res = SOC_SAND_OK;
  uint32 remark_profile_max = ARAD_PP_EG_QOS_ETH_ECN_REMARK_PROFILE_INDEX_MAX;
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->in_dscp_exp, ARAD_PP_EG_QOS_IN_DSCP_EXP_MAX, ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->remark_profile, ARAD_PP_EG_QOS_REMARK_PROFILE_INDEX_MAX, ARAD_PP_REMARK_PROFILE_OUT_OF_RANGE_ERR, 11, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->pkt_hdr_type, ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_MAX, ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_BELOW_MIN(info->pkt_hdr_type, ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_MIN, ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_OUT_OF_RANGE_ERR, 30, exit);  
  if (info->pkt_hdr_type == SOC_PPC_PKT_HDR_TYPE_MPLS) {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->in_dscp_exp, SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 40, exit);
  }
  if (info->pkt_hdr_type == SOC_PPC_PKT_HDR_TYPE_ETH) 
  {    
    SOC_SAND_ERR_IF_ABOVE_MAX(info->dp, ARAD_PP_EG_QOS_DP_MAX, SOC_SAND_PP_DP_OUT_OF_RANGE_ERR, 50, exit);
    /*SOC_SAND_ERR_IF_ABOVE_MAX(info->ecn_capable, ARAD_PP_EG_ECN_CAPABLE_MAX, ARAD_PP_ECN_CAPABLE_OUT_OF_RANGE_ERR, 60, exit);*/
    /* Remark profile is limited to 0-3 for Ethernet remark */
    
    /* When ECN is not enabled, Jericho support 16 remark profile.*/
    if (SOC_IS_JERICHO(unit)) {
        uint32 ecn_is_enabled;

        res = jer_itm_enable_ecn_get(unit, &ecn_is_enabled);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);
        if (!ecn_is_enabled) {
            remark_profile_max = ARAD_PP_EG_QOS_ETH_REMARK_PROFILE_INDEX_MAX;
        }        
    }
    SOC_SAND_ERR_IF_ABOVE_MAX(info->remark_profile, remark_profile_max, ARAD_PP_REMARK_PROFILE_OUT_OF_RANGE_ERR, 70, exit);
    /* In-dscp-exp is limited to 0-7 for Ethernet remark */
    SOC_SAND_ERR_IF_ABOVE_MAX(info->in_dscp_exp, SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 80, exit);
  }
  else /* In case no Ethernet header type dp and ecn capable are not used */
  {
    SOC_SAND_ERR_IF_ABOVE_MAX(info->dp, 0, SOC_SAND_PP_DP_OUT_OF_RANGE_ERR, 90, exit);
    /*SOC_SAND_ERR_IF_ABOVE_MAX(info->ecn_capable, 0, ARAD_PP_ECN_CAPABLE_OUT_OF_RANGE_ERR, 100, exit);*/
  }
  
  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_QOS_MAP_KEY_verify()",0,0);
}

uint32
  SOC_PPC_EG_QOS_PARAMS_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  /* info->ipv4_tos and info->ipv6_tc are always in range (< 255) since they are uint8 */
  SOC_SAND_ERR_IF_ABOVE_MAX(info->mpls_exp, SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 12, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_PARAMS_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_QOS_PARAMS_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->mpls_exp, SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 12, exit);
  /* info->ip_dscp is always in range (< 255) since it is uint8 */
  
  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_PARAMS_verify()",0,0);
}

uint32
  SOC_PPC_EG_QOS_PHP_REMARK_KEY_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->exp_map_profile, ARAD_PP_EG_QOS_EXP_MAP_PROFILE_MAX, ARAD_PP_EG_QOS_EXP_MAP_PROFILE_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->php_type, ARAD_PP_EG_QOS_PHP_TYPE_MAX, ARAD_PP_EG_QOS_PHP_TYPE_OUT_OF_RANGE_ERR, 11, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->exp, SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 12, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_PHP_REMARK_KEY_verify()",0,0);
}

uint32
  SOC_PPC_EG_QOS_PORT_INFO_verify(
    SOC_SAND_IN int unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->exp_map_profile, ARAD_PP_EG_QOS_EXP_MAP_PROFILE_MAX, ARAD_PP_EG_QOS_EXP_MAP_PROFILE_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->marking_profile, ARAD_PP_EG_QOS_MARKING_PROFILE_MAX(unit, 0), ARAD_PP_EG_QOS_PHP_TYPE_OUT_OF_RANGE_ERR, 20, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_PORT_INFO_verify()",0,0);
}

#ifdef BCM_88660
uint32
  SOC_PPC_EG_QOS_MARKING_KEY_verify(
    SOC_SAND_IN int unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MARKING_KEY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->resolved_dp_ndx, ARAD_PP_EG_QOS_RESOLVED_DP_MAX(unit, info->dp_map_disabled), ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->tc_ndx, ARAD_PP_TC_MAX_VAL, ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->in_lif_profile, ARAD_PP_EG_QOS_INLIF_PROFILE_MAX(unit, info->dp_map_disabled), ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 30, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->marking_profile, ARAD_PP_EG_QOS_MARKING_PROFILE_MAX(unit, info->dp_map_disabled), ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 40, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_MARKING_KEY_verify()",0,0);
}
uint32
  SOC_PPC_EG_QOS_MARKING_PARAMS_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_MARKING_PARAMS *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->ip_dscp, ARAD_PP_EG_QOS_IP_DSCP_MAX, ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->mpls_exp, ARAD_PP_EG_QOS_MPLS_EXP_MAX, ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_MARKING_PARAMS_verify()",0,0);
}
uint32 SOC_PPC_EG_QOS_GLOBAL_INFO_verify(SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info)
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_MAGIC_NUM_VERIFY(info);
  
  SOC_SAND_ERR_IF_ABOVE_MAX(info->resolved_dp_bitmap, ARAD_PP_EG_QOS_RESOLVED_DP_BITMAP_MAX, ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->in_lif_profile_bitmap, ARAD_PP_EG_QOS_INLIF_PROFILE_BITMAP_MAX, ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_QOS_GLOBAL_INFO_verify()",0,0);
}
#endif /* BCM_88660 */

uint32
arad_pp_port_eg_ttl_inheritance_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN uint64 outlif_profiles
  )
{
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_modify(unit, EPNI_CFG_OUTLIF_PROFILE_ALLOW_TTL_INHERITANCEr, SOC_CORE_ALL, 0, CFG_OUTLIF_PROFILE_ALLOW_TTL_INHERITANCEf, outlif_profiles));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_ppd_eg_ttl_inheritance_set()", 0, 0);
}


uint32
arad_pp_port_eg_ttl_inheritance_get(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT uint64 *outlif_profiles
  )
{
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, EPNI_CFG_OUTLIF_PROFILE_ALLOW_TTL_INHERITANCEr, SOC_CORE_DEFAULT, 0, CFG_OUTLIF_PROFILE_ALLOW_TTL_INHERITANCEf, outlif_profiles));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_ppd_eg_ttl_inheritance_get()", 0, 0);

}


uint32
arad_pp_port_eg_qos_inheritance_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN uint64 outlif_profiles
  )
{
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_modify(unit, EPNI_CFG_OUTLIF_PROFILE_ALLOW_QOS_INHERITANCEr, SOC_CORE_ALL, 0, CFG_OUTLIF_PROFILE_ALLOW_QOS_INHERITANCEf, outlif_profiles));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_ppd_eg_qos_inheritance_set()", 0, 0);
}


uint32 
arad_pp_port_eg_qos_inheritance_get(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT uint64 *outlif_profiles
  )
{
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,	exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, EPNI_CFG_OUTLIF_PROFILE_ALLOW_QOS_INHERITANCEr, SOC_CORE_DEFAULT, 0, CFG_OUTLIF_PROFILE_ALLOW_QOS_INHERITANCEf, outlif_profiles));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_ppd_eg_qos_inheritance_get()", 0, 0);
}
uint32
  arad_pp_port_eg_qos_marking_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN bcm_port_t port,
   SOC_SAND_IN int enable
  )
{
  uint32 res = SOC_SAND_OK;
  soc_reg_above_64_val_t reg_val, fld_val, mask;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  SOC_REG_ABOVE_64_CLEAR(fld_val);
  SOC_REG_ABOVE_64_CLEAR(mask);
 
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg_above_64_get(unit, EPNI_CFG_PORT_ENABLE_DSCP_MARKINGr, REG_PORT_ANY, 0, reg_val));
  soc_reg_above_64_field_get(unit, EPNI_CFG_PORT_ENABLE_DSCP_MARKINGr, reg_val, CFG_PORT_ENABLE_DSCP_MARKINGf, fld_val); 

  SOC_REG_ABOVE_64_CREATE_MASK(mask, 1, port);
  if (enable == 0) {
      SOC_REG_ABOVE_64_NOT(mask);
      SOC_REG_ABOVE_64_AND(fld_val, mask);
  } else {
      SOC_REG_ABOVE_64_OR(fld_val, mask);
  }
  soc_reg_above_64_field_set(unit, EPNI_CFG_PORT_ENABLE_DSCP_MARKINGr, reg_val, CFG_PORT_ENABLE_DSCP_MARKINGf, fld_val);
 
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_set(unit, EPNI_CFG_PORT_ENABLE_DSCP_MARKINGr, REG_PORT_ANY, 0, reg_val));
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_global_info_set_unsafe()", 0, 0);
}


uint32 
  arad_pp_port_eg_qos_marking_get(
   SOC_SAND_IN int unit,
   SOC_SAND_IN bcm_port_t port,
   SOC_SAND_OUT int *enable
  )
{
  uint32 res = SOC_SAND_OK;
  soc_reg_above_64_val_t reg_val, fld_val, mask;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_REG_ABOVE_64_CLEAR(reg_val);
  SOC_REG_ABOVE_64_CLEAR(fld_val);
  SOC_REG_ABOVE_64_CLEAR(mask);

  SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg_above_64_get(unit, EPNI_CFG_PORT_ENABLE_DSCP_MARKINGr, REG_PORT_ANY, 0, reg_val));
  soc_reg_above_64_field_get(unit, EPNI_CFG_PORT_ENABLE_DSCP_MARKINGr, reg_val, CFG_PORT_ENABLE_DSCP_MARKINGf, fld_val); 

   SOC_REG_ABOVE_64_CREATE_MASK(mask, 1, port);
  SOC_REG_ABOVE_64_AND(fld_val, mask);
  *enable = (SOC_REG_ABOVE_64_IS_ZERO(fld_val)) ? 0 : 1;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_qos_global_info_get_unsafe()", 0, 0);
}
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88650_A0) */

