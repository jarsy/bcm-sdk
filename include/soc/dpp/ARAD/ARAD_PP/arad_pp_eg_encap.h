
/* $Id: arad_pp_eg_encap.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_EG_ENCAP_INCLUDED__
/* { */
#define __ARAD_PP_EG_ENCAP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_eg_encap.h>
#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_IN_CUD_NOF_BITS 17
#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_OUT_CUD_NOF_BITS 21

#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB 19
#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_DSCP 2

#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_START 0
#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_NOF_BITS 2

#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_START \
        (ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_START + ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_NOF_BITS)
#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_NOF_BITS 11

#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_START \
        (ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_START + ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_NOF_BITS)
#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_NOF_BITS 4

#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_START 0
#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_NOF_BITS 4

#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_MEM_INDEX_START \
        (ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_START + ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_NOF_BITS)
#define ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_MEM_INDEX_NOF_BITS 13

#define ARAD_PP_EG_ENCAP_NOF_ENTRIES_PER_EEDB_BANK(_unit)       (2 * SOC_DPP_DEFS_GET(_unit, nof_eedb_lines_per_bank))
#define ARAD_PP_EG_ENCAP_NOF_ENTRIES_PER_HALF_EEDB_BANK(_unit)  (ARAD_PP_EG_ENCAP_NOF_ENTRIES_PER_EEDB_BANK(_unit)/2)
#define ARAD_PP_EG_ENCAP_NOF_TOP_BANKS_PER_EEDB_BANK            (4)
#define ARAD_PP_EG_ENCAP_NOF_REGULAR_EEDB_BANKS(_unit)          (SOC_DPP_DEFS_GET(_unit, eg_encap_nof_banks)    \
                                                                    - (SOC_DPP_DEFS_GET(unit, eg_encap_nof_top_banks)  \
                                                                       / ARAD_PP_EG_ENCAP_NOF_TOP_BANKS_PER_EEDB_BANK))
#define ARAD_PP_EG_ENCAP_NOF_ENTRIES_PER_TOP_BANK(unit)               (SOC_IS_QUX(unit) ? (256) : (1024))

#define ARAD_PP_EG_ENCAP_PROG_DATA_ENTRY_SIZE    (4)

#define ARAD_PP_EG_ENCAP_EEDB_SIZE(unit)          (((SOC_PPC_EG_ENCAP_BANK_NDX_MAX(unit)+1)*SOC_PPC_EG_ENCAP_NOF_BANKS(unit)))


#define ARAD_PP_EG_ENCAP_VERIFY_EEP_TYPE_COMPATIBLE_TO_ACCESS_TYPE(outlif,eep_type, access_type)   \
    if (!(access_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE) && (outlif != 0))                   \
    {                                                                               \
    switch (eep_type) {                                                             \
    case SOC_PPC_EG_ENCAP_EEP_TYPE_LIF_EEP:                                         \
      if ((access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_POP) &&           \
          (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_SWAP) &&          \
          (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_TUNNEL))            \
      {                                                                            \
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 560, exit);    \
      }                                                                            \
      break;                                                                       \
    case SOC_PPC_EG_ENCAP_EEP_TYPE_TUNNEL_EEP:                                     \
      if ((access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_TUNNEL) &&        \
          (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_IP))                   \
      {                                                                            \
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 570, exit);    \
      }                                                                            \
      break;                                                                        \
    case SOC_PPC_EG_ENCAP_EEP_TYPE_VSI:                                             \
      if (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_RIF)                 \
      {                                                                             \
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 580, exit);     \
      }                                                                             \
      break;                                                                        \
    case SOC_PPC_EG_ENCAP_EEP_TYPE_LL:                                              \
      if (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_LINK_LAYER )               \
      {           \
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 590, exit);     \
      }                                                                             \
      break;                                                                        \
    case SOC_PPC_EG_ENCAP_EEP_TYPE_DATA:                                              \
      if (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA && access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_IPV6 && access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_ARP_OVERLAY) {           \
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 595, exit);     \
      }                                                                             \
      break;                                                                        \
    case SOC_PPC_EG_ENCAP_EEP_TYPE_ROO_LL:                                            \
      if (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_ROO_LINK_LAYER)           \
      {                                                                              \
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 596, exit);  \
      }                                                                              \
      break;                                                                         \
    case SOC_PPC_EG_ENCAP_EEP_TYPE_TRILL:                                            \
      if (access_type != ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_TRILL)                    \
      {                                                                              \
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 597, exit);  \
      }                                                                              \
      break;                                                                         \
    default:                                                                        \
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR, 600, exit);       \
    }                                                                               \
    }                                                                               \
    else                                                                            \
    {                                                                               \
      ARAD_PP_DO_NOTHING_AND_EXIT;                                                  \
    }
/* Maximal push profile value in mpls2 field in MPLS format entry in EEDB */
#define ARAD_PP_EG_ENCAP_MPLS2_MAX_PUSH_PROFILE_VALUE (7)
/* } */
/*************
 * MACROS    *
 *************/
/* { */
#define ARAD_PP_EG_ENCAP_NOF_RIF_BANKS_TYPE(rif_max_id)  ( rif_max_id / _BCM_DPP_AM_EGRESS_LIF_NOF_ENTRIES_PER_HALF_BANK(unit) )
#define ARAD_PP_EG_ENCAP_NOF_RIF_BANKS_PHASE(rif_max_id) ((rif_max_id / _BCM_DPP_AM_EGRESS_LIF_NOF_ENTRIES_PER_BANK(unit))*2  \
                                                        + (rif_max_id % _BCM_DPP_AM_EGRESS_LIF_NOF_ENTRIES_PER_BANK(unit)) ? 2 : 0)

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
  SOC_PPC_EG_ENCAP_RANGE_INFO_SET = ARAD_PP_PROC_DESC_BASE_EG_ENCAP_FIRST,
  SOC_PPC_EG_ENCAP_RANGE_INFO_SET_PRINT,
  SOC_PPC_EG_ENCAP_RANGE_INFO_SET_UNSAFE,
  SOC_PPC_EG_ENCAP_RANGE_INFO_SET_VERIFY,
  SOC_PPC_EG_ENCAP_RANGE_INFO_GET,
  SOC_PPC_EG_ENCAP_RANGE_INFO_GET_PRINT,
  SOC_PPC_EG_ENCAP_RANGE_INFO_GET_VERIFY,
  SOC_PPC_EG_ENCAP_RANGE_INFO_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_DATA_LIF_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_DATA_LIF_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_DATA_LIF_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_DATA_LIF_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_AC_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_AC_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_AC_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_AC_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_IPV6_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_IPV6_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV6_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_MIRROR_ENTRY_SET,
  ARAD_PP_EG_ENCAP_MIRROR_ENTRY_SET_UNSAFE,
  ARAD_PP_EG_ENCAP_MIRROR_ENTRY_SET_VERIFY,
  ARAD_PP_EG_ENCAP_MIRROR_ENTRY_GET,
  ARAD_PP_EG_ENCAP_MIRROR_ENTRY_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_PORT_ERSPAN_SET,
  ARAD_PP_EG_ENCAP_PORT_ERSPAN_SET_UNSAFE,
  ARAD_PP_EG_ENCAP_PORT_ERSPAN_GET,
  ARAD_PP_EG_ENCAP_PORT_ERSPAN_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_LL_ENTRY_ADD,
  ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_PRINT,
  ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_UNSAFE,
  ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_VERIFY,
  ARAD_PP_EG_ENCAP_ENTRY_REMOVE,
  ARAD_PP_EG_ENCAP_ENTRY_REMOVE_PRINT,
  ARAD_PP_EG_ENCAP_ENTRY_REMOVE_UNSAFE,
  ARAD_PP_EG_ENCAP_ENTRY_REMOVE_VERIFY,
  ARAD_PP_EG_ENCAP_ENTRY_GET,
  ARAD_PP_EG_ENCAP_ENTRY_GET_PRINT,
  ARAD_PP_EG_ENCAP_ENTRY_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_ENTRY_GET_VERIFY,
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET,
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET_UNSAFE,
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET_VERIFY,
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET,
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET_UNSAFE,
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET_VERIFY,
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET,
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET_VERIFY,
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET,
  ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET_UNSAFE,
  ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET_VERIFY,
  ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET,
  ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET_VERIFY,
  ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET_UNSAFE,
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET,
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET_UNSAFE,
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET_VERIFY,
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET,
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET_VERIFY,
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET_UNSAFE,
  SOC_PPC_EG_ENCAP_GLBL_INFO_SET,
  SOC_PPC_EG_ENCAP_GLBL_INFO_SET_UNSAFE,
  SOC_PPC_EG_ENCAP_GLBL_INFO_SET_VERIFY,
  SOC_PPC_EG_ENCAP_GLBL_INFO_GET,
  SOC_PPC_EG_ENCAP_GLBL_INFO_GET_VERIFY,
  SOC_PPC_EG_ENCAP_GLBL_INFO_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET_VERIFY,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_PRINT,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_VERIFY,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_PRINT,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_VERIFY,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET_PRINT,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET_VERIFY,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET_PRINT,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET_UNSAFE,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET_VERIFY,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_PRINT,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_VERIFY,
  ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_GET_PROCS_PTR,
  ARAD_PP_EG_ENCAP_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_INIT,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_INIT_UNSAFE,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_INIT_VERIFY,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET_VERIFY,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET_UNSAFE,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET_VERIFY,
  ARAD_PP_EG_ENCAP_LL_ENTRY_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_TUNNEL_ENTRY_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_LIF_ENTRY_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_VSI_ENTRY_GET_UNSAFE,
  ARAD_PP_EG_ENCAP_DATA_ENTRY_GET_UNSAFE,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_ENCAP_PROCEDURE_DESC_LAST
} ARAD_PP_EG_ENCAP_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_EG_ENCAP_FIRST,
  ARAD_PP_EG_ENCAP_NEXT_EEP_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_LL_EEP_NDX_OUT_OF_RANGE_ERR,
  SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_EEP_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_DEPTH_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_ENTRY_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_SRC_IP_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_LL_LIMIT_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_IP_TNL_LIMIT_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_MPLS_TNL_LIMIT_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_SWAP_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_PUSH_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_OUT_VSI_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_TPID_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_POP_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_TUNNEL_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_NOF_TUNNELS_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_ORIENTATION_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_DEST_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_SRC_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_TTL_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_TOS_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_PCP_DEI_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_TPID_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_MODEL_OUT_OF_RANGE_ERR,
  SOC_PPC_EG_ENCAP_EXP_MARK_MODE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_CW_OUT_OF_RANGE_ERR,
  SOC_PPC_EG_ENCAP_ENTRY_TYPE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_UNSUPPORTED,
  ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_LIF_EEP_NDX_AC_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_LIF_EEP_AND_AC_ID_NOT_EQUAL_ERR,
  ARAD_PP_EG_ENCAP_LIF_EEP_NDX_MPLS_TUNNEL_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_LIF_EEP_NDX_PWE_MCAST_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_IP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_NOF_EG_ENCAP_ACCESS_PHASE_OUT_OF_RANGE_ERROR,
  ARAD_PP_EG_LL_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR,
  SOC_PPC_EG_ENCAP_POP_INFO_PKT_FRWRD_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_ADD_ENTROPY_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR,
  ARAD_PP_EG_ENCAP_MIRROR_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_MIRROR_VLAN_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_IS_ERSPAN_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_PROTECTION_POINTER_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_PROTECTION_PASS_VAL_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_OUT_RIF_PROFILE_OUT_OF_RANGE_ERR,
  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_ENCAP_ERR_LAST
} ARAD_PP_EG_ENCAP_ERR;


typedef struct arad_pp_eg_encap_eedb_bank_info_s
{
    uint8   valid;         /*0 - not valid; 1- valid*/
    uint8   phase;         /*1-4*/
    uint16  app_bmp;         /*application allocated in this bank */
    uint8   type;            /*mapped or direct*/
    uint8   is_extended;     /*this bank has extension : TRUE/FALSE */
    uint8   ext_type;        /* extension type : protection/data , valid when is_extended=TRUE */
    uint32  force_flags;
    int     extension_bank;  /*number of extension bank (0-7)*/
    int     nof_free_entries;
} arad_pp_eg_encap_eedb_bank_info_t;

typedef struct arad_pp_eg_encap_eedb_top_bank_info_s
{
    uint8 extension_bank_owner;    /*0-21, -1 for unallocated*/
} arad_pp_eg_encap_eedb_top_bank_info_t;



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

uint32
  arad_pp_eg_encap_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Inits devision of the Egress Encapsulation Table entry
 *     This configuration only take effect the entry type is not ARAD_PP_EG_ENCAP_ACCESS_PREFIX_TYPE_OTHER and 
 *     ARAD_PP_EG_ENCAP_ACCESS_PREFIX_TYPE_NONE.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_entry_init_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_range_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets devision of the Egress Encapsulation Table between
 *   the different usages (Link layer/ IP tunnels/ MPLS
 *   tunnels).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO                 *range_info -
 *     Range information.
 * REMARKS:
 *   - T20E: not supported. Error will be returned if called.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_range_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO                 *range_info
  );

uint32
  arad_pp_eg_encap_range_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO                 *range_info
  );

uint32
  arad_pp_eg_encap_range_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_encap_range_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_encap_range_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_encap_range_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_RANGE_INFO                 *range_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_protection_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   SEgress Protection info into from an Out-LIF extension buffer.
 *   Used in Jericho for of various entries: AC, PWE, MPLS
 * INPUT:
 *   SOC_SAND_IN  int                                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                   out_lif_id -
 *     Out-LIF ID of the extension base entry.
 *   SOC_SAND_IN uint8                                  is_full_entry_extension -
 *     Indication whether the extended Out-LIF is of a full EEDB entry
 *   SOC_PPC_EG_ENCAP_PROTECTION_INFO                   protection_info
 *     The retrieved Protection info
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32 arad_pp_eg_encap_protection_info_set_unsafe(
    SOC_SAND_IN int                                 unit,
    SOC_SAND_IN int                                 out_lif_id,
    SOC_SAND_IN uint8                               is_full_entry_extension,
    SOC_SAND_IN SOC_PPC_EG_ENCAP_PROTECTION_INFO    *protection_info);

/*********************************************************************
 *     Get Egress Protection info from an Out-LIF extension buffer.
 *     Used in Jericho for of various entries: AC, PWE, MPLS
*********************************************************************/
uint32 arad_pp_eg_encap_protection_info_get_unsafe(
    SOC_SAND_IN int                                 unit,
    SOC_SAND_IN int                                 out_lif_id,
    SOC_SAND_IN uint8                               is_full_entry_extension,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PROTECTION_INFO   *protection_info);


uint32 arad_pp_eg_encap_protection_info_set_verify(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PROTECTION_INFO       *protection_info);


/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_null_lif_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set LIF Editing entry to be NULL Entry.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  lif_eep_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF (First
 *     encapsulation)
 *   SOC_SAND_IN  uint32                                  next_eep -
 *     Pointer to the next egress encapsulation table entry
 * REMARKS:
 *   - T20E Only API- Needed when the packet should only be
 *   encapsulated with Tunnel information and the EEI is NULL
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_null_lif_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  uint32                                  next_eep
  );

uint32
  arad_pp_eg_encap_null_lif_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  uint32                                  next_eep
  );

uint32
  arad_pp_eg_encap_lif_field_set_unsafe(
      SOC_SAND_IN  int                               unit,
      SOC_SAND_IN  uint32                                lif_eep_ndx,
      SOC_SAND_IN  uint32                                flags,
      SOC_SAND_IN  uint32                                val
  );

uint32
  arad_pp_eg_encap_lif_field_set_verify(
      SOC_SAND_IN  int                               unit,
      SOC_SAND_IN  uint32                                lif_eep_ndx,
      SOC_SAND_IN  uint32                                flags,
      SOC_SAND_IN  uint32                                val
  );


uint32
  arad_pp_eg_encap_lif_field_get_unsafe(
      SOC_SAND_IN  int                               unit,
      SOC_SAND_IN  uint32                                lif_eep_ndx,
      SOC_SAND_IN  uint32                                flags,
      SOC_SAND_OUT  uint32                               *val
  );

/*********************************************************************
*     Init CUD global extension.
*     
*********************************************************************/
uint32
  arad_pp_eg_encap_data_local_to_global_cud_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pp_eg_encap_data_local_to_global_cud_init_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Set CUD global extension.
*     
*********************************************************************/
uint32
  arad_pp_eg_encap_data_local_to_global_cud_set_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint32                                  in_cud,
    SOC_SAND_IN  uint32                                  out_cud
  );

uint32
  arad_pp_eg_encap_data_local_to_global_cud_set_verify(
     SOC_SAND_IN  int                                  unit,
     SOC_SAND_IN  uint32                                  in_cud,
     SOC_SAND_OUT uint32                                  out_cud
   );

/*********************************************************************
*     Get CUD global extension.
*     
*********************************************************************/
uint32
  arad_pp_eg_encap_data_local_to_global_cud_get_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint32                                  in_cud,
    SOC_SAND_OUT uint32*                                 out_cud
  );

uint32
  arad_pp_eg_encap_data_local_to_global_cud_get_verify(
     SOC_SAND_IN  int                                  unit,
     SOC_SAND_IN  uint32                                  in_cud,
     SOC_SAND_OUT uint32*                                 out_cud
   );
/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_data_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set LIF Editing entry to hold DATA entry.
 *   Needed on some cases, where further programming is needed.
 *   For example: IPV6 tunnels.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  lif_eep_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO              *data_info -
 *     Data entry information.
 *   SOC_SAND_IN  uint8                                  next_eep_valid,
 *       Is next_eep valid. 
 *   SOC_SAND_IN  uint32                                  next_eep -
 *     Pointer to Egress Encapsulation table entry.
 * REMARKS:
 *  
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_data_lif_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO              *data_info,
    SOC_SAND_IN  uint8                                   next_eep_valid,
    SOC_SAND_IN  uint32                                  next_eep
  );

uint32
  arad_pp_eg_encap_data_lif_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO              *data_info,
    SOC_SAND_IN  uint8                                   next_eep_valid,
    SOC_SAND_IN  uint32                                  next_eep
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_ac_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set LIF Editing entry to hold AC ID. Actually maps from
 *   CUD to AC.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  lif_eep_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF (First
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_AC_ID                               ac_id -
 *     The Out-AC ID, the packet is associated with
 * REMARKS:
 *   - In Arad-B: mapping has to maintain the encoding of
 *   the AC in the CUD.- Use this API to MAP CUD (from the TM
 *   header) to AC.- Mapping - Use LIF Editing Table.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_ac_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_AC_ID                               ac_id
  );

uint32
  arad_pp_eg_encap_ac_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_AC_ID                               ac_id
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_swap_command_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set LIF Editing entry to hold MPLS LSR SWAP label.
 *   Needed for MPLS multicast services.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  lif_eep_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF (First
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO                  *swap_info -
 *     Swap label for LSR applications (typically for
 *     Multicast).
 *   SOC_SAND_IN  uint32                                  next_eep -
 *     Pointer to Egress Encapsulation table entry of type
 *     tunnel (second encapsulation), or link-layer
 * REMARKS:
 *   - In T20E, next_eep always points to Tunnels Editing
 *   Table.- The usage of this API for Multicast is to give a
 *   unique encapsulation for each copy.- If the user needs
 *   SWAP for Unicast packets, then the swap can be done in
 *   the ingress using the EEI.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_swap_command_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO                  *swap_info,
    SOC_SAND_IN  uint32                                  next_eep
  );

uint32
  arad_pp_eg_encap_swap_command_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO                  *swap_info,
    SOC_SAND_IN  uint32                                  next_eep
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_pwe_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set LIF Editing entry to hold PWE info (VC label and
 *   push profile).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  lif_eep_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF (First
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO                   *pwe_info -
 *     PWE info VC label and push profile.
 *   SOC_SAND_IN  uint32                                  next_eep -
 *     Pointer to the next egress encapsulation table entry
 * REMARKS:
 *   - In T20E: next_eep always points to Tunnels Editing
 *   Table.- The usage of this API for Multicast is to give
 *   special treatment for each copy.- In some applications,
 *   such as path protection in VPWS, the VC label can be set
 *   explicitely in the EEI in the ingress PP device, in
 *   which case the 1st encapsulation table is skipped.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_pwe_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO                   *pwe_info,
    SOC_SAND_IN  uint32                                  next_eep
  );

uint32
  arad_pp_eg_encap_pwe_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO                   *pwe_info,
    SOC_SAND_IN  uint32                                  next_eep
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_pop_command_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set LIF Editing entry to hold MPLS LSR POP command.
 *   Needed for MPLS multicast services.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  lif_eep_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF (First
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO                   *pop_info -
 *     POP information including type of POP, and information
 *     to process the inner Ethernet in case the POP is into
 *     Ethernet.
 *   SOC_SAND_IN  uint32                                  next_eep -
 *     Pointer to the next egress encapsulation table entry
 * REMARKS:
 *   - In T20E: next_eep always points to Tunnels Editing
 *   Table.- The usage of this API for Multicast is to give
 *   special treatment for each copy.- If the user needs to
 *   add a POP label for Unicast packet, then the POP command
 *   can be set in the EEI in the ingress.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_pop_command_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO                   *pop_info,
    SOC_SAND_IN  uint32                                  next_eep
  );

uint32
  arad_pp_eg_encap_pop_command_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO                   *pop_info,
    SOC_SAND_IN  uint32                                  next_eep
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_vsi_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set LIF Editing entry to hold out-VSI. Needed for
 *   IPv4/IPv6 Multicast Application, and MVR (Multicast VLAN
 *   Replication).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  lif_eep_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF (First
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                              out_vsi -
 *     The VSI to assign to the copy. For further processing.
 *   SOC_SAND_IN  uint32                                  next_eep -
 *     Pointer to the next egress encapsulation table entry
 * REMARKS:
 *   - IPv4 MC:- out_vsi: used as the out-RIF of the packet.-
 *   next_eep: points to LL including the MAC address.- MVR
 *   (multicast Vlan Replicaton):- out_vsi: used to set VID
 *   for each copy.- next_eep: has to be NULL.- In T20E:
 *   next_eep always points to Tunnels Editing Table. In this
 *   case, the Tunnel-Editing entry should do nothing and
 *   point to LL entry.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_vsi_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO             *vsi_info, 
    SOC_SAND_IN uint8                                  next_eep_valid, 
    SOC_SAND_IN  uint32                                  next_eep
  );

uint32
  arad_pp_eg_encap_vsi_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO            *vsi_info, 
    SOC_SAND_IN  uint8                                 next_eep_valid, 
    SOC_SAND_IN  uint32                                  next_eep
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_mpls_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add MPLS tunnels encapsulation entry to the Tunnels
 *   Editing Table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  tunnel_eep_ndx -
 *     Egress Encapsulation table entry, of type Tunnel (Second
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO            *mpls_encap_info -
 *     MPLS encapsulation information.
 *   SOC_SAND_IN  uint32                                  ll_eep -
 *     Points to next encapsulation (LL).
 * REMARKS:
 *   - In order to set entry with no encapsulation set
 *   'nof_tunnels' at 'mpls_encap_info' to zero.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_mpls_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO            *mpls_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  );

uint32
  arad_pp_eg_encap_mpls_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO            *mpls_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_ipv4_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv4 tunnels encapsulation entry to the Egress
 *   Encapsulation Tunnels Editing Table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  tunnel_eep_ndx -
 *     Egress Encapsulation table entry, of type Tunnel (Second
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO            *ipv4_encap_info -
 *     IPv4 encapsulation information.
 *   SOC_SAND_IN  uint32                                  ll_eep -
 *     Points to next encapsulation (LL). IP tunnel must point
 *     to a link-layer encapsulation entry and a link layer.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO            *ipv4_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  );

uint32
  arad_pp_eg_encap_ipv4_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO            *ipv4_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_ipv6_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPV6 tunnels encapsulation entry to the Egress
 *   Encapsulation Tunnels Editing Table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  tunnel_eep_ndx -
 *     Egress Encapsulation table entry, of type Tunnel (Second
 *     encapsulation)
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO            *ipv6_encap_info -
 *     IPV6 encapsulation information.
 *   SOC_SAND_IN  uint32                                  ll_eep -
 *     Points to next encapsulation (LL). IP tunnel must point
 *     to a link-layer encapsulation entry and a link layer.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv6_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO            *ipv6_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  );

uint32
  arad_pp_eg_encap_ipv6_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO            *ipv6_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_overlay_arp_data_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   build eedb entry of type data entry and insert it in eedb.
 *   insert eedb entry in eedb (HW) 
 *   build prge data for overlay arp data. 
 *   insert prge data in table (HW)
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
arad_pp_eg_encap_overlay_arp_data_entry_add_unsafe(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_IN  uint32                                 overlay_ll_eep_ndx, 
     SOC_SAND_INOUT  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *ll_encap_info
   );

uint32
arad_pp_eg_encap_overlay_arp_data_entry_add_verify(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_IN  uint32                                 overlay_ll_eep_ndx, 
     SOC_SAND_IN  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *ll_encap_info
   );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_mpls_default_ttl_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the default TTL value for PWE encapsulation.
 * INPUT:
 *   ttl_val - value to be set as default.
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_mpls_default_ttl_set(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_IN  uint8                                  ttl_val
   );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_mpls_default_ttl_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the default TTL value for PWE encapsulation.
 * OUTPUT:
 *   ttl_val - default TTL value for PWE encapsulation.
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_mpls_default_ttl_get(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_OUT uint8                                  *ttl_val
   );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_overlay_arp_data_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   get eedb entry. Check if it's a overlay arp data entry type 
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
arad_pp_eg_encap_overlay_arp_data_entry_get_unsafe(
   SOC_SAND_IN  int                                    unit,
   SOC_SAND_IN  uint32                                 overlay_ll_eep_ndx, 
   SOC_SAND_OUT SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *overlay_ll_encap_info

   );



/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_mirror_entry_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add Mirror encapsulation entry to the Egress
 *   Encapsulation Tunnels Editing Table.
 * INPUT:
 *   SOC_SAND_IN  int                                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      mirror_ndx -
 *     Egress Encapsulation table entry 0-15. each mirror entry occupies 2 entries:
 *       1. mirror_base + mirror_ndx
 *       2. mirror_base + mirror_ndx + 16
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO  *mirror_encap_info -
 *     Mirror encapsulation information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_mirror_entry_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              mirror_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *mirror_encap_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_mirror_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add Mirror encapsulation entry to the Egress
 *   Encapsulation Tunnels Editing Table.
 * INPUT:
 *   SOC_SAND_IN  int                                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      mirror_ndx -
 *     Egress Encapsulation table entry 0-15. each mirror entry occupies 2 entries:
 *       1. mirror_base + mirror_ndx
 *       2. mirror_base + mirror_ndx + 16
 *   SOC_SAND_OUT  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO  *mirror_encap_info -
 *     Mirror encapsulation information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_mirror_entry_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              mirror_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *mirror_encap_info
  );

uint32
  arad_pp_eg_encap_mirror_entry_set_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              mirror_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *mirror_encap_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_port_erspan_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   disable/enable ERSPAN per port,
 *   port can be used for ERSPAN or RSPAN/SPAN but not both .
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                local_port_ndx -
 *     pp port 
 *   SOC_SAND_IN  uint8                                  is_erspan -
 *     whether port is used for erspan (1) or not (0)
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_port_erspan_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_IN  uint8                                  is_erspan
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_port_erspan_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   get whether ERSPAN is disabled/enabled per port,
 *   port can be used for ERSPAN or RSPAN/SPAN but not both .
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                local_port_ndx -
 *     pp port 
 *   SOC_SAND_OUT  uint8                                  is_erspan -
 *     1 - port is used for ERSPAN. 0 - is not.
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_port_erspan_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_OUT uint8                                  *is_erspan
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_ll_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add LL encapsulation entry.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                  out_ac_valid,
 *       Is out_vid valid. Relevant ONLY for MIM
 *       tunnel and TRILL adjacent; otherwise, has to be FALSE.
 *   SOC_SAND_IN  uint32                                   out_ac_lsb,
 *      Next_outlif_lsb
 *   SOC_SAND_IN  uint32                                  ll_eep_ndx -
 *     Points to LL table.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO                    *ll_encap_info -
 *     LL encapsulation information, including DA-MAC.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_ll_entry_add_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                ll_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO                 *ll_encap_info
  );

uint32
  arad_pp_eg_encap_ll_entry_add_verify(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                ll_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO                 *ll_encap_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_entry_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove entry from the encapsulation Table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                   eep_type_ndx -
 *     Encapsulation Table to select (LIFs/Tunnels/LL).
 *   SOC_SAND_IN  uint32                                  eep_ndx -
 *     Points into Editing Table according to 'eep_type_ndx'.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                   eep_type_ndx,
    SOC_SAND_IN  uint32                                  eep_ndx
  );

uint32
  arad_pp_eg_encap_entry_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                   eep_type_ndx,
    SOC_SAND_IN  uint32                                  eep_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_entry_type_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get entry type from the Egress encapsulation
 *   tables.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  eep_ndx -
 *     Points into EEDB Table.
 *   SOC_SAND_OUT  SOC_PPC_EG_ENCAP_ENTRY_TYPE            entry_type -
 *     Type of the encapsulation entry. 
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_entry_type_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_TYPE            *entry_type
  );

uint32
  arad_pp_eg_encap_entry_type_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  eep_ndx
  );
/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get entry information from the Egress encapsulation
 *   tables.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                   eep_type_ndx -
 *     Encapsulation Table to select (LIFs/Tunnels/LL).
 *   SOC_SAND_IN  uint32                                  eep_ndx -
 *     Points into Editing Table according to 'eep_type_ndx'.
 *   SOC_SAND_IN  uint32                                      depth -
 *     Number of encapsulation entries to get, see example of
 *     use in remarks.
 *   SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                     encap_entry_info -
 *     SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD]- To include egress
 *     encapsulation entries.
 *   SOC_SAND_OUT uint32                                      next_eep -
 *     SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD]- To include next Egress
 *     encapsulation pointer.
 *   SOC_SAND_OUT uint32                                      *nof_entries -
 *     Number of valid entries in next_eep and
 *     encap_entry_info.
 * REMARKS:
 *   - To get all egress encapsulations performed for a given
 *   out-lif, set eep_type_ndx =
 *   SOC_PPC_EG_ENCAP_EEP_TYPE_LIF_EEP, eep_ndx = out-lif, depth
 *   = SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_entry_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                       eep_type_ndx,
    SOC_SAND_IN  uint32                                      eep_ndx,
    SOC_SAND_IN  uint32                                      depth,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                     encap_entry_info[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD],
    SOC_SAND_OUT uint32                                      next_eep[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD],
    SOC_SAND_OUT uint32                                      *nof_entries
  );

uint32
  arad_pp_eg_encap_entry_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                       eep_type_ndx,
    SOC_SAND_IN  uint32                                      eep_ndx,
    SOC_SAND_IN  uint32                                      depth
  );




/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_entry_data_info_to_overlay_arp_encap_info_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Parse data info to get overlay arp encap info.
 *   tables.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                     encap_entry_info -
 *      To include egress encapsulation entries.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
arad_pp_eg_encap_entry_data_info_to_overlay_arp_encap_info_unsafe(
 SOC_SAND_IN  int                                    unit, 
   SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO          *exact_data_info
   );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_push_profile_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting the push profile info - specifying how to build
 *   the label header.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      profile_ndx -
 *     Profile ID. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO              *profile_info -
 *     Push profile information including TTL, EXP, has-CW, and
 *     if it is PIPE mode.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_push_profile_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO              *profile_info
  );

uint32
  arad_pp_eg_encap_push_profile_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO              *profile_info
  );

uint32
  arad_pp_eg_encap_push_profile_info_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_encap_push_profile_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_encap_push_profile_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_eg_encap_push_profile_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                      profile_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO              *profile_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_push_exp_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the EXP value of the pushed label as mapping of the
 *   TC and DP.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY                   *exp_key -
 *     The key (set of parameters) that is used to set the EXP.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_EXP                          exp -
 *     EXP to put in the pushed label.
 * REMARKS:
 *   - T20E only. Error is returned if called for Petra-B/ARAD.-
 *   Relevant when exp_mark_mode in push profile entry set to
 *   map EXP value from TC and DP
 *   (SOC_PPC_EG_ENCAP_EXP_MARK_MODE_FROM_PUSH_PROFILE)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_push_exp_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_EXP                          exp
  );

uint32
  arad_pp_eg_encap_push_exp_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_EXP                          exp
  );

uint32
  arad_pp_eg_encap_push_exp_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_encap_push_exp_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_encap_push_exp_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_eg_encap_push_exp_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key,
    SOC_SAND_OUT SOC_SAND_PP_MPLS_EXP                          *exp
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_pwe_glbl_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Global information for PWE Encapsulation.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO              *glbl_info -
 *     Global in
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_pwe_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO              *glbl_info
  );

uint32
  arad_pp_eg_encap_pwe_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO              *glbl_info
  );

uint32
  arad_pp_eg_encap_pwe_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_encap_pwe_glbl_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_encap_pwe_glbl_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_eg_encap_pwe_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PWE_GLBL_INFO              *glbl_info
  );

uint32
  arad_pp_eg_encap_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_GLBL_INFO              *glbl_info
  );
uint32
  arad_pp_eg_encap_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_GLBL_INFO              *glbl_info
  );
uint32
  arad_pp_eg_encap_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );
uint32
  arad_pp_eg_encap_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_GLBL_INFO              *glbl_info
  );
/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set source IP address for IPv4 Tunneling.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  entry_ndx -
 *     Entry pointed by IPv4 Tunnel - see
 *     soc_ppd_eg_encap_ipv4_encap_entry_add(). Range: 0 - 15.
 *   SOC_SAND_IN  uint32                                  src_ip -
 *     Source IP address.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  uint32                                  src_ip
  );

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  uint32                                  src_ip
  );

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT uint32                                  *src_ip
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set TTL for IPv4 Tunneling.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  entry_ndx -
 *     Entry pointed by IPv4 Tunnel. See
 *     soc_ppd_eg_encap_ipv4_encap_entry_add(). Range: 0 - 3.
 *   SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl -
 *     TTL value to set in the IPV4 tunnel header. Range: 0 -
 *     63.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl
  );

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl
  );

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IP_TTL                            *ttl
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set TOS for IPv4 Tunneling.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  entry_ndx -
 *     Entry pointed by IPv4 Tunnel. See
 *     soc_ppd_eg_encap_ipv4_encap_entry_add(). Range: 0 - 15.
 *   SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          tos -
 *     TOS value to set in the IPV4 tunnel header. Range: 0 -
 *     255.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          tos
  );

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          tos
  );

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IPV4_TOS                          *tos
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set MPLS PIPE mode to do copy EXP (1) or set EXP (0).
 * INPUT:
 *   SOC_SAND_IN  int                                  unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                is_exp_copy -
 *     Pipe mode - do copy EXP (1) or set EXP (0).
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint8                                is_exp_copy
  );

uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set_verify(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint8                                is_exp_copy
  );

/*********************************************************************
 *     Gets the configuration set by the
 *     "arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set" API.
 *     Refer to "arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_get_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_OUT uint8                                *is_exp_copy
  );

uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_get_verify(
    SOC_SAND_IN  int                                  unit
  );



/*********************************************************************
* NAME:
 *   arad_pp_eg_trill_entry_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add trill tunnel encapsulation entry 
 * INPUT:
 *   SOC_SAND_IN  int                                  unit 
 *     Identifier of the device to access.
 *   SOC_SAND_IN uint32                       trill_eep_ndx 
 *      Egress Encapsulation table entry
 *   SOC_SAND_IN SOC_PPC_EG_ENCAP_TRILL_INFO* trill_encap_info 
 *      Trill encapsulation information. 
 *   SOC_SAND_IN uint32                       ll_eep_ndx 
 *      Points to next encapsulation (LL). 
 *   SOC_SAND_IN uint8                        ll_eep_is_valid 
 *      Indicate the next encapsulation is valid
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
    arad_pp_eg_trill_entry_set(
       SOC_SAND_IN int                          unit, 
       SOC_SAND_IN uint32                       trill_eep_ndx, 
       SOC_SAND_IN SOC_PPC_EG_ENCAP_TRILL_INFO* trill_encap_info
  ); 



/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_eg_encap module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_eg_encap_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_eg_encap module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_eg_encap_get_errs_ptr(void);

uint32
  SOC_PPC_EG_ENCAP_RANGE_INFO_verify(
     int unit, 
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO *info
  );

uint32 SOC_PPC_EG_ENCAP_PROTECTION_INFO_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PROTECTION_INFO *info);

uint32
  SOC_PPC_EG_ENCAP_SWAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_PWE_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO *info
  );

uint32 
  SOC_PPC_EG_ENCAP_DATA_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_POP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_LL_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY_verify(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY *info
  );

uint32
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_GLBL_INFO *info
  );
uint32
  SOC_PPC_EG_ENCAP_ENTRY_VALUE_verify(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENTRY_VALUE *info
  );

uint32
  SOC_PPC_EG_ENCAP_ENTRY_INFO_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENTRY_INFO *info
  );

uint32
  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *info
  );




/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_EG_ENCAP_INCLUDED__*/
#endif


