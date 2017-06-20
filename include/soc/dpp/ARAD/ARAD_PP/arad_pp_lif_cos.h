/* $Id: arad_pp_lif_cos.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_LIF_COS_INCLUDED__
/* { */
#define __ARAD_PP_LIF_COS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_lif_cos.h>


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
   * Auto generated. Do not edit following section {
   */
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_SET = ARAD_PP_PROC_DESC_BASE_LIF_COS_FIRST,
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_SET_PRINT,
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_SET_UNSAFE,
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_SET_VERIFY,
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_GET,
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_GET_PRINT,
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_GET_VERIFY,
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_GET_UNSAFE,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_SET,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_SET_PRINT,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_SET_UNSAFE,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_SET_VERIFY,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_GET,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_GET_PRINT,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_GET_VERIFY,
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_GET_UNSAFE,
  SOC_PPC_LIF_COS_PROFILE_INFO_SET,
  SOC_PPC_LIF_COS_PROFILE_INFO_SET_PRINT,
  SOC_PPC_LIF_COS_PROFILE_INFO_SET_UNSAFE,
  SOC_PPC_LIF_COS_PROFILE_INFO_SET_VERIFY,
  SOC_PPC_LIF_COS_PROFILE_INFO_GET,
  SOC_PPC_LIF_COS_PROFILE_INFO_GET_PRINT,
  SOC_PPC_LIF_COS_PROFILE_INFO_GET_VERIFY,
  SOC_PPC_LIF_COS_PROFILE_INFO_GET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_SET,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_SET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_SET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_SET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_GET,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_GET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_GET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_L2_INFO_GET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_SET,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_SET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_SET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_SET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_GET,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_GET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_GET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_IP_INFO_GET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_SET,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_SET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_SET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_SET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_GET,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_GET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_GET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_MPLS_LABEL_INFO_GET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_SET,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_SET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_SET_UNSAFE,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_SET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_GET,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_GET_PRINT,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_GET_VERIFY,
  ARAD_PP_LIF_COS_PROFILE_MAP_TC_DP_INFO_GET_UNSAFE,
  SOC_PPC_LIF_COS_OPCODE_TYPES_SET,
  SOC_PPC_LIF_COS_OPCODE_TYPES_SET_PRINT,
  SOC_PPC_LIF_COS_OPCODE_TYPES_SET_UNSAFE,
  SOC_PPC_LIF_COS_OPCODE_TYPES_SET_VERIFY,
  SOC_PPC_LIF_COS_OPCODE_TYPES_GET,
  SOC_PPC_LIF_COS_OPCODE_TYPES_GET_PRINT,
  SOC_PPC_LIF_COS_OPCODE_TYPES_GET_VERIFY,
  SOC_PPC_LIF_COS_OPCODE_TYPES_GET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_SET,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_SET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_SET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_SET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_GET,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_GET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_GET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_IPV6_TOS_MAP_GET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_SET,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_SET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_SET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_SET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_GET,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_GET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_GET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_IPV4_TOS_MAP_GET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_SET,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_SET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_SET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_SET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_GET,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_GET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_GET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_TC_DP_MAP_GET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_SET,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_SET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_SET_UNSAFE,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_SET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_GET,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_GET_PRINT,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_GET_VERIFY,
  ARAD_PP_LIF_COS_OPCODE_VLAN_TAG_MAP_GET_UNSAFE,
  ARAD_PP_LIF_COS_GET_PROCS_PTR,
  ARAD_PP_LIF_COS_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY_SET_UNSAFE,
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY_GET_UNSAFE,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_COS_PROCEDURE_DESC_LAST
} ARAD_PP_LIF_COS_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LIF_COS_PROFILE_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LIF_COS_FIRST,
  ARAD_PP_LIF_COS_COS_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_MAP_TBL_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_OPCODE_NDX_OUT_OF_RANGE_ERR,
  SOC_PPC_LIF_COS_OPCODE_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_TAG_TYPE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_MAP_TABLE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_IP_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_OUTER_TPID_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_AC_OFFSET_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_LIF_MAP_WHEN_IP_MPLS_MISMATCH_ERR,
  ARAD_PP_LIF_COS_MAP_FROM_TC_DP_DISABLED_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_COS_ERR_LAST
} ARAD_PP_LIF_COS_ERR;

typedef struct
{
  uint8 map_from_tc_dp;
} ARAD_PP_SW_DB_LIF_COS;
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

soc_error_t
  arad_pp_lif_cos_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_ac_profile_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets COS information of AC COS Profile including (FORCE
 *   to const values, map field from the packet and select
 *   mapping table)
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      profile_ndx -
 *     AC COS Profile. Range: 1 - 15. Set by
 *     soc_ppd_l2_lif_ac_add().
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_AC_PROFILE_INFO                 *profile_info -
 *     COS Profile information.
 * REMARKS:
 *   - The profile is set per AC - see soc_ppd_l2_lif_ac_add/
 *   soc_ppd_l2_lif_ac_with_cos_add()- cos profile 0 used for
 *   NOP.- T20E Only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_ac_profile_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_AC_PROFILE_INFO                 *profile_info
  );

uint32
  arad_pp_lif_cos_ac_profile_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_AC_PROFILE_INFO                 *profile_info
  );

uint32
  arad_pp_lif_cos_ac_profile_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_ac_profile_info_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_ac_profile_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_ac_profile_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_AC_PROFILE_INFO                 *profile_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_pwe_profile_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets COS Profile information (FORCE to const values, map
 *   field from the packet and select mapping table)
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      profile_ndx -
 *     Profile. Range: 0 - 15.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PWE_PROFILE_INFO                *profile_info -
 *     COS Profile information.
 * REMARKS:
 *   - The profile is determined per PWE - see
 *   soc_ppd_l2_lif_pwe_add()- T20E Only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_pwe_profile_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PWE_PROFILE_INFO                *profile_info
  );

uint32
  arad_pp_lif_cos_pwe_profile_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PWE_PROFILE_INFO                *profile_info
  );

uint32
  arad_pp_lif_cos_pwe_profile_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_pwe_profile_info_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_pwe_profile_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_pwe_profile_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_PWE_PROFILE_INFO                *profile_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_profile_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set CoS Profile attributes. Incoming LIFs are mapped to
 *   CoS Profile. CoS Profile '0' defined to keep the previous
 *   settings. The other CoS profiles are configured by this
 *   function.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      cos_profile_ndx -
 *     CoS Profile ID. Range: 1 - 15.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_INFO                    *profile_info -
 *     CoS profile attributes
 * REMARKS:
 *   - Arad-B Only. T20E users should call
 *   soc_ppd_lif_cos_ac_profile_info_set()/soc_ppd_lif_cos_pwe_profile_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      cos_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_INFO                    *profile_info
  );

uint32
  arad_pp_lif_cos_profile_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      cos_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_INFO                    *profile_info
  );

uint32
  arad_pp_lif_cos_profile_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      cos_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_profile_info_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_profile_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      cos_profile_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_INFO                    *profile_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_profile_map_l2_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from L2 VLAN Tag fields to DP and TC.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      map_tbl_ndx -
 *     Mapping table ID. Range: 1 - 15. Arad-B: The map table
 *     index is the CoS profile.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY          *map_key -
 *     L2 Key used for mapping, including (UP/PCP, DEI, matched
 *     TPID)
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value -
 *     The COS parameters (DP and TC) mapped to.
 * REMARKS:
 *   - T20E: The mapping table to use is determined according
 *   soc_ppd_lif_cos_ac_profile_info_set().- Arad-B: The mapping
 *   table is the CoS profile defined by the incoming LIF
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_l2_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY          *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_l2_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY          *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_l2_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY          *map_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_profile_map_l2_info_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_profile_map_l2_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_l2_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY          *map_key,
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_profile_map_ip_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from IP header fields (TOS / DSCP) to DP and
 *   TC.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      map_tbl_ndx -
 *     Mapping table ID. Range: 1 - 15. Arad-B: The map table
 *     index is the CoS profile.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY          *map_key -
 *     IP Key used for mapping, including (DSCP)
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value -
 *     The COS parameters (DP and TC) mapped to.
 * REMARKS:
 *   - T20E: The mapping table to used is determined
 *   according soc_ppd_lif_cos_ac_profile_info_set().- Arad-B:
 *   The mapping table is the CoS profile defined by the
 *   incoming LIF
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_ip_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY          *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_ip_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY          *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_ip_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY          *map_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_profile_map_ip_info_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_profile_map_ip_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_ip_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY          *map_key,
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_profile_map_mpls_label_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from label fields (EXP) to DP and TC.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      map_tbl_ndx -
 *     Mapping table ID. Range: 1 - 15. Arad-B: The map table
 *     index is the CoS profile.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY        *map_key -
 *     Label Key used for mapping, including (EXP)
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value -
 *     The COS parameters (DP and TC) mapped to.
 * REMARKS:
 *   - T20E: The mapping table to used is determined
 *   according soc_ppd_lif_cos_pwe_profile_info_set().- Arad-B:
 *   The mapping table is the CoS profile defined by the
 *   incoming LIF
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_mpls_label_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY        *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_mpls_label_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY        *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_mpls_label_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY        *map_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_profile_map_mpls_label_info_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_lif_cos_profile_map_mpls_label_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_mpls_label_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY        *map_key,
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_profile_map_tc_dp_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set remapping from TC and DP to TC and DP.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      map_tbl_ndx -
 *     Mapping table ID. Range: 1 - 15. The map table index is
 *     the CoS profile.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY       *map_key -
 *     TC and DP, calculated from previous processing stages
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value -
 *     The COS parameters (DP and TC) mapped to.
 * REMARKS:
 *   - Arad-B Only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_tc_dp_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY       *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_tc_dp_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY       *map_key,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

uint32
  arad_pp_lif_cos_profile_map_tc_dp_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY       *map_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_profile_map_tc_dp_info_set_unsafe" API.
 *     Refer to
 *     "arad_pp_lif_cos_profile_map_tc_dp_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_profile_map_tc_dp_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      map_tbl_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY       *map_key,
    SOC_SAND_OUT SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY           *map_value
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_opcode_types_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Opcode type. Set the mapping of Class of Service
 *   attributes to the AC-Offset.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      opcode_ndx -
 *     The ID of the configured Opcode. Range: Arad-B: 0 - 2;
 *     T20E: 0 - 6
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_TYPE                     opcode_type -
 *     The CoS attributes that will affect the AC-Offset
 *     setting.
 * REMARKS:
 *   - After setting the opcode type, one should call the
 *   APIs that map the chosen CoS attributes to AC-offset-
 *   After setting the Opcode type and mapping table(s)
 *   soc_ppd_l2_lif_ac_with_cos_add() may be called to add ACs
 *   according to the CoS attributes
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_types_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      opcode_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_TYPE                     opcode_type
  );

uint32
  arad_pp_lif_cos_opcode_types_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      opcode_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_TYPE                     opcode_type
  );

uint32
  arad_pp_lif_cos_opcode_types_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      opcode_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_opcode_types_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_opcode_types_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_types_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      opcode_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_OPCODE_TYPE                     *opcode_type
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_opcode_ipv6_tos_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Opcode according to IPv6 TOS field
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      opcode_ndx -
 *     The ID of the configured Opcode. Range: 1 - 3.
 *   SOC_SAND_IN  SOC_SAND_PP_IPV6_TC                           ipv6_tos_ndx -
 *     IPv6 TOS value, as arriving in the packet. Range: 0 -
 *     255.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info -
 *     The mapping rules according to the TOS value and Opcode
 *     ID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_ipv6_tos_map_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV6_TC                           ipv6_tos_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_ipv6_tos_map_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV6_TC                           ipv6_tos_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_ipv6_tos_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV6_TC                           ipv6_tos_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_opcode_ipv6_tos_map_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_opcode_ipv6_tos_map_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_ipv6_tos_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV6_TC                           ipv6_tos_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_opcode_ipv4_tos_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Opcode according to IPv4 TOS field
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  opcode_ndx -
 *     The ID of the configured Opcode. Range: 1 - 3.
 *   SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          ipv4_tos_ndx -
 *     IPv4 TOS value, as arriving in the packet. Range: 0 -
 *     255.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info -
 *     The mapping rules according to the TOS value and Opcode
 *     ID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_ipv4_tos_map_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          ipv4_tos_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_ipv4_tos_map_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          ipv4_tos_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_ipv4_tos_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          ipv4_tos_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_opcode_ipv4_tos_map_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_opcode_ipv4_tos_map_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_ipv4_tos_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          ipv4_tos_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_opcode_tc_dp_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Opcode according to Traffic Class and Drop
 *   Precedence
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  opcode_ndx -
 *     The ID of the configured Opcode. Range: 1 - 3.
 *   SOC_SAND_IN  SOC_SAND_PP_TC                                tc_ndx -
 *     Traffic Class. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DP                                dp_ndx -
 *     Drop Precedence. Range: 0 - 3.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info -
 *     The mapping rules according to the TC, DP and Opcode ID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_tc_dp_map_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                                tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                                dp_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_tc_dp_map_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                                tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                                dp_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_tc_dp_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                                tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                                dp_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_opcode_tc_dp_map_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_opcode_tc_dp_map_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_tc_dp_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                                tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                                dp_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_opcode_vlan_tag_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Opcode according to VLAN Tag
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  opcode_ndx -
 *     The ID of the configured Opcode. Range: 1 - 3.
 *   SOC_SAND_IN  uint8                                   tag_type_ndx -
 *     VLAN tag type S-TAG; C-TAG; I-TAG or none.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                            pcp_ndx -
 *     Priority Code Point. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                           dei_ndx -
 *     Drop Eligible Indicator. Range: 0 - 1.
 *   SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info -
 *     The mapping rules according to the TC, DP, and Opcode
 *     ID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_vlan_tag_map_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  uint8                                   tag_type_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                            pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                           dei_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_vlan_tag_map_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  uint8                                   tag_type_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                            pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                           dei_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

uint32
  arad_pp_lif_cos_opcode_vlan_tag_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  uint8                                   tag_type_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                            pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                           dei_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_cos_opcode_vlan_tag_map_set_unsafe" API.
 *     Refer to "arad_pp_lif_cos_opcode_vlan_tag_map_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_cos_opcode_vlan_tag_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  opcode_ndx,
    SOC_SAND_IN  uint8                                   tag_type_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                            pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                           dei_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_COS_OPCODE_ACTION_INFO          *action_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_lif_cos module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_lif_cos_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_lif_cos_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_lif_cos module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_lif_cos_get_errs_ptr(void);

uint32
  SOC_PPC_LIF_COS_AC_PROFILE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_AC_PROFILE_INFO *info
  );

uint32
  SOC_PPC_LIF_COS_PWE_PROFILE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PWE_PROFILE_INFO *info
  );

uint32
  SOC_PPC_LIF_COS_PROFILE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_INFO *info
  );

uint32
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_ENTRY *info
  );

uint32
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_IP_KEY *info
  );

uint32
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_MPLS_KEY *info
  );

uint32
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_L2_KEY *info
  );

uint32
  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_PROFILE_MAP_TBL_TC_DP_KEY *info
  );

uint32
  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LIF_COS_OPCODE_ACTION_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LIF_COS_INCLUDED__*/
#endif

