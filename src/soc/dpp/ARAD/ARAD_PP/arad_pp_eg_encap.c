#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_eg_encap.c,v 1.80 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS

#include <shared/bsl.h>
/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_ac.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap_access.h>
#include <soc/mem.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/arad_egr_prog_editor.h>
#include <soc/dpp/ARAD/arad_parser.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/JER/JER_PP/jer_pp_eg_encap.h>
#include <soc/dpp/JER/JER_PP/jer_pp_eg_encap_access.h>
#include <soc/dpp/QAX/QAX_PP/qax_pp_eg_encap_access.h>

#include <soc/dpp/PPC/ppc_api_eg_encap.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_MAX                        (SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD-1)
#define ARAD_PP_EG_ENCAP_DEPTH_MAX                               (SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD-1)
#define ARAD_PP_EG_ENCAP_PROFILE_NDX_MAX                         ((SOC_IS_JERICHO_B0_AND_ABOVE(unit)) ? (15) : (7))
#define ARAD_PP_EG_ENCAP_ENTRY_NDX_MAX                           (15)
#define ARAD_PP_EG_ENCAP_SRC_IP_MAX                              (15)
#define ARAD_PP_EG_ENCAP_IP_TNL_LIMIT_MIN                        (0) /* ((ll_limit + 1)) */
#define ARAD_PP_EG_ENCAP_SWAP_LABEL_MAX                          (1*1024*1024-1)
#define ARAD_PP_EG_ENCAP_LABEL_MAX                               (1*1024*1024-1)
#define ARAD_PP_EG_ENCAP_PUSH_PROFILE_MAX                        ((SOC_IS_JERICHO_B0_AND_ABOVE(unit)) ? (15) : (7))
#define ARAD_PP_EG_ENCAP_OUT_VSI_MAX                             (4*1024-1)
#define ARAD_PP_EG_ENCAP_TPID_PROFILE_MAX                        (3)
#define ARAD_PP_EG_ENCAP_TUNNEL_LABEL_MAX                        (SOC_SAND_U32_MAX)
#define ARAD_PP_EG_ENCAP_NOF_TUNNELS_MAX                         (2)
#define ARAD_PP_EG_ENCAP_DEST_MAX                                (SOC_SAND_U32_MAX)
#define ARAD_PP_EG_ENCAP_SRC_INDEX_MAX                           (15)
#define ARAD_PP_EG_ENCAP_TTL_INDEX_MAX                           (3)
#define ARAD_PP_EG_ENCAP_TOS_INDEX_MAX                           (15)
#define ARAD_PP_EG_ENCAP_TPID_INDEX_MAX                          (3)
#define ARAD_PP_EG_LL_ENCAP_REMARK_PROFILE_INDEX_MAX             ((SOC_IS_JERICHO(unit)) ? (7) : (3))
#define ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_MAX            (15)
#define ARAD_PP_EG_ENCAP_MODEL_MAX                               (SOC_SAND_PP_NOF_MPLS_TUNNEL_MODELS-1)
#define SOC_PPC_EG_ENCAP_EXP_MARK_MODE_MAX                       (SOC_PPC_NOF_EG_ENCAP_EXP_MARK_MODES-1)
#define ARAD_PP_EG_ENCAP_CW_MAX                                  (SOC_SAND_U32_MAX)
#define SOC_PPC_EG_ENCAP_ENTRY_TYPE_MAX                          (SOC_PPC_NOF_EG_ENCAP_ENTRY_TYPES_ARAD-1)
#define SOC_PPC_EG_ENCAP_ACCESS_PHASE_MAX(unit)                  (SOC_PPC_NOF_EG_ENCAP_ACCESS_PHASE_TYPES(unit)-1)
#define ARAD_PP_EG_ENCAP_ADD_ENTROPY_LABEL_MAX                   (1)
#define ARAD_PP_EG_ENCAP_PROTECTION_PASS_VAL_MAX                 (1)

#define ARAD_PP_EG_ENCAP_SIP_NOF_BITS                            (32)
#define ARAD_PP_EG_ENCAP_TTL_NOF_BITS                            (8)
#define ARAD_PP_EG_ENCAP_TOS_NOF_BITS                            (8)
#define ARAD_PP_EG_ENCAP_REMARK_NOF_BITS                         (4)
#define ARAD_PP_EG_ENCAP_NEXT_EEP_INVALID                        (SOC_PPC_EG_ENCAP_NEXT_EEP_INVALID)

#define ARAD_PP_EG_ENCAP_NOF_MIRRORS                             (16)

#define ARAD_PP_EG_ENCAP_ERSPAN_VERSION                          (1)
#define ARAD_PP_EG_ENCAP_VXLAN_UDP_SRC_PORT                      (0x0050)

#define ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_LSB           (0)
#define ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_NOF_BITS      (15)
#define ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_PATH_LSB              (14)
#define ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_PATH_NOF_BITS         (1)

/* } */
/*************
 * MACROS    *
 *************/
/* { */




/*if encap is of type ipv4, mpls or ll it must have even ndx*/
#define ARAD_PP_EG_ENCAP_NDX_OF_TYPE_VERIFY(ndx) \
    if (ndx % 2 != 0) {                                                     \
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_NDX_ILLEGAL_ERR, 550, exit); \
    }                                                                       \



#define ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry, flags, val)   \
    if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_DROP) {            \
        entry.drop = val;                                       \
    }

#define ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry, flags, val)   \
    if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_OUT_LIF_PROFILE) {            \
        entry.outlif_profile = val;                                       \
    }


#define ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry, flags, val)   \
    if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_DROP) {            \
        *val = entry.drop;                                       \
        goto exit;                                              \
    }

#define ARAD_PP_EG_ENCAP_GET_FIELD_NEXT_LIF(entry, flags, val)   \
    if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF) {            \
        *val = entry.next_outlif;                               \
        goto exit;                                              \
    }

#define ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry, flags, val)   \
    if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_OUT_LIF_PROFILE) {            \
        *val = entry.outlif_profile;                               \
        goto exit;                                              \
    }


/* 6LSBytes {Priority(3),Reserved(1),Direction(1),Truncated(1),SPAN-ID(10)} Directly Extracted from EES1[55:8]*/
/* Priority[15:13] Reserved[12:12] Direction[11:11] Truncated[10:10] span_id[9:0] */
#define ARAD_PP_EG_ENCAP_PRGE_MEM_ERSPAN_FORMAT_SET(__device_id, __prio, __trunc, __span_id, __data) \
    (__data)[1] = ( (ARAD_PP_EG_ENCAP_ERSPAN_VERSION << 28) | (__prio << 13) | (__trunc << 10) | __span_id ) ; \
     (__data)[0] = 0;

#define ARAD_PP_EG_ENCAP_PRGE_MEM_ERSPAN_FORMAT_SPAN_ID_GET(__span_id, __data) \
    (__span_id) = (((__data)[1]) & 0x03ff) ; 

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */


typedef enum {
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_NONE = 0,
  /*
   * Ether IP and VXLAN encapsulation 
   */
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_ETHER_IP = 1,

  #ifdef BCM_88660_A0     
  /*
   * VxLan encapsulation for arad+. It's the same value than EoIP.
   */
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_VXLAN = 1,
  #endif

  /*
   * Basic GRE (4B) encapsulation
   */
      SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_BASIC_GRE = 2,
  /*
   *  Inhance GRE (8B) encapsulation
   */
  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_ENHANCE_GRE = 3
} SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD; 


/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC SOC_PROCEDURE_DESC_ELEMENT
  Arad_pp_procedure_desc_element_eg_encap[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_RANGE_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_AC_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_AC_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_AC_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_AC_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_LL_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_REMOVE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_ENTRY_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_GLBL_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_GLBL_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_GLBL_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_GLBL_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_GLBL_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_EG_ENCAP_GLBL_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET_VERIFY),

  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_LL_ENTRY_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_TUNNEL_ENTRY_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_LIF_ENTRY_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_EG_ENCAP_DATA_ENTRY_GET_UNSAFE),

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_eg_encap[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'lif_eep_ndx' is out of range. \n\r "
    "The range is: No min - 64K.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_NEXT_EEP_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_NEXT_EEP_OUT_OF_RANGE_ERR",
    "The parameter 'next_eep' is out of range. \n\r "
    "The range is: No min - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'tunnel_eep_ndx' is out of range. \n\r "
    "The range is: No min - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR",
    "The parameter 'll_eep' is out of range. \n\r "
    "The range is: 0-64K.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_LL_EEP_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LL_EEP_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'll_eep_ndx' is out of range. \n\r "
    "The range is: 0 to range_info->ll_limit.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR,
    "SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'eep_type_ndx' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_EEP_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_EEP_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'eep_ndx' is out of range. \n\r "
    "The range is: 0 - 64K.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_DEPTH_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_DEPTH_OUT_OF_RANGE_ERR",
    "The parameter 'depth' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_NOF_ENTRIES_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_NOF_ENTRIES_OUT_OF_RANGE_ERR",
    "The parameter 'nof_entries' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_PROFILE_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_PROFILE_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'profile_ndx' is out of range. \n\r "
    "The range is: 0 - 7.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_NDX_ILLEGAL_ERR,
    "ARAD_PP_EG_ENCAP_NDX_ILLEGAL_ERR",
    "The parameter 'll_eep' is illegal. \n\r "
    "Must be even.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR,
    "ARAD_PP_EG_ENCAP_INCOMPATIBLE_TYPE_ERR",
    "The requested eep_type_ndx is not comaptible with \n\r"
    "the existing entry at eep_ndx. \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_ENTRY_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_ENTRY_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'entry_ndx' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_SRC_IP_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_SRC_IP_OUT_OF_RANGE_ERR",
    "The parameter 'src_ip' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_LL_LIMIT_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LL_LIMIT_OUT_OF_RANGE_ERR",
    "The parameter 'll_limit' is out of range. \n\r "
    "The range is: 0 - 4*1024.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_IP_TNL_LIMIT_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_IP_TNL_LIMIT_OUT_OF_RANGE_ERR",
    "The parameter 'ip_tnl_limit' is out of range. \n\r "
    "The range is: (ll_limit + 1) - (12K-1).\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_NOF_EG_ENCAP_ACCESS_PHASE_OUT_OF_RANGE_ERROR,
    "ARAD_PP_NOF_EG_ENCAP_ACCESS_PHASE_OUT_OF_RANGE_ERROR",
    "The parameter 'bank_access_phase' is out of range. \n\r "
    "The range is: 0-3.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_MPLS_TNL_LIMIT_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_MPLS_TNL_LIMIT_OUT_OF_RANGE_ERR",
    "The parameter 'mpls_tnl_limit' is out of range. \n\r "
    "The range is: ip_tnl_limit+1 - 12*1024.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_SWAP_LABEL_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_SWAP_LABEL_OUT_OF_RANGE_ERR",
    "The parameter 'swap_label' is out of range. \n\r "
    "The range is: 0 - 1*1024*1024.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_LABEL_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LABEL_OUT_OF_RANGE_ERR",
    "The parameter 'label' is out of range. \n\r "
    "The range is: 0 - 1*1024*1024.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_PUSH_PROFILE_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_PUSH_PROFILE_OUT_OF_RANGE_ERR",
    "The parameter 'push_profile' is out of range. \n\r "
    "The range is: 0 - 7.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_OUT_VSI_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_OUT_VSI_OUT_OF_RANGE_ERR",
    "The parameter 'out_vsi' is out of range. \n\r "
    "The range is: 0 - 4K-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_TPID_PROFILE_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_TPID_PROFILE_OUT_OF_RANGE_ERR",
    "The parameter 'tpid_profile' is out of range. \n\r "
    "The range is: 0 - 3.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_POP_TYPE_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_POP_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'pop_type' is out of range. \n\r "
    "Value should be an mpls pop command (Range: 8-14).\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_TUNNEL_LABEL_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_TUNNEL_LABEL_OUT_OF_RANGE_ERR",
    "The parameter 'tunnel_label' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_NOF_TUNNELS_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_NOF_TUNNELS_OUT_OF_RANGE_ERR",
    "The parameter 'nof_tunnels' is out of range. \n\r "
    "The range is: 0 - 2.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_ORIENTATION_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_ORIENTATION_OUT_OF_RANGE_ERR",
    "The parameter 'orientation' is out of range. \n\r "
    "The range is: SOC_SAND_PP_NOF_HUB_SPOKE_ORIENTATIONS - SOC_SAND_PP_NOF_HUB_SPOKE_ORIENTATIONS.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_DEST_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_DEST_OUT_OF_RANGE_ERR",
    "The parameter 'dest' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_SRC_INDEX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_SRC_INDEX_OUT_OF_RANGE_ERR",
    "The parameter 'src_index' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_TTL_INDEX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_TTL_INDEX_OUT_OF_RANGE_ERR",
    "The parameter 'ttl_index' is out of range. \n\r "
    "The range is: 0 - 3.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_TOS_INDEX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_TOS_INDEX_OUT_OF_RANGE_ERR",
    "The parameter 'tos_index' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_PCP_DEI_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_PCP_DEI_OUT_OF_RANGE_ERR",
    "The parameter 'pcp_dei' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U8_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_TPID_INDEX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_TPID_INDEX_OUT_OF_RANGE_ERR",
    "The parameter 'tpid_index' is out of range. \n\r "
    "The range is: 0 - 3.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_MODEL_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_MODEL_OUT_OF_RANGE_ERR",
    "The parameter 'model' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_PP_NOF_MPLS_TUNNEL_MODELS-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_EG_ENCAP_EXP_MARK_MODE_OUT_OF_RANGE_ERR,
    "SOC_PPC_EG_ENCAP_EXP_MARK_MODE_OUT_OF_RANGE_ERR",
    "The parameter 'exp_mark_mode' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_EG_ENCAP_EXP_MARK_MODES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_CW_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_CW_OUT_OF_RANGE_ERR",
    "The parameter 'cw' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_EG_ENCAP_ENTRY_TYPE_OUT_OF_RANGE_ERR,
    "SOC_PPC_EG_ENCAP_ENTRY_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'entry_type' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_EG_ENCAP_ENTRY_TYPES_ARAD-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */

  {
    ARAD_PP_EG_ENCAP_LIF_EEP_NDX_AC_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LIF_EEP_NDX_AC_OUT_OF_RANGE_ERR",
    "'lif_eep_ndx' is out of range. The range is: 0 to range_info->ll_limit,\n\r "
    "as configured using soc_ppd_eg_encap_range_info_set().\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_LIF_EEP_NDX_MPLS_TUNNEL_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LIF_EEP_NDX_MPLS_TUNNEL_OUT_OF_RANGE_ERR",
    "'lif_eep_ndx' is out of range. The range is: range_info->ip_tunnel_limit+1\n\r "
    "to 12K-1, as configured using soc_ppd_eg_encap_range_info_set().\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_LIF_EEP_AND_AC_ID_NOT_EQUAL_ERR,
    "ARAD_PP_EG_ENCAP_LIF_EEP_AND_AC_ID_NOT_EQUAL_ERR",
    "Out ac should be equal to lif_eep.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_LIF_EEP_NDX_PWE_MCAST_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_LIF_EEP_NDX_PWE_MCAST_OUT_OF_RANGE_ERR",
    "'lif_eep_ndx' is out of range. \n\r"
    "The range is: MAX(range_info->ip_tunnel_limit+1,8K) to 12K-1 .\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_IP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_IP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR",
    "'PARAM_NAME ' is out of range. \n\r"
    "The range is: 0-64K, \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_LL_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_LL_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR",
    "remark_profile_index is out of range. \n\r"
    "The range for ll encapsulation entry is: 0-3, \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR",
    "remark_profile_index is out of range. \n\r"
    "The range for vsi encapsulation entry is: 0-15, \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_EG_ENCAP_POP_INFO_PKT_FRWRD_TYPE_OUT_OF_RANGE_ERR,
    "SOC_PPC_EG_ENCAP_POP_INFO_PKT_FRWRD_TYPE_OUT_OF_RANGE_ERR",
    "pop_next_header is out of range. \n\r"
    "header must be of type ethernet/mpls/ipv4/ipv6. \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_ADD_ENTROPY_LABEL_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_ADD_ENTROPY_LABEL_OUT_OF_RANGE_ERR",
    "add_entropy_label is out of range. \n\r"
    "The range is: 0-1, \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_MIRROR_ID_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_MIRROR_ID_OUT_OF_RANGE_ERR",
    "mirror_index is out of range. \n\r"
    "The range is: 0-15, \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_MIRROR_VLAN_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_MIRROR_VLAN_OUT_OF_RANGE_ERR",
    "mirror_index is out of range. \n\r"
    "The range is: 0 - 4*1024-1, \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_EG_ENCAP_IS_ERSPAN_OUT_OF_RANGE_ERR,
    "ARAD_PP_EG_ENCAP_IS_ERSPAN_OUT_OF_RANGE_ERR",
    "is_erspan is out of range. \n\r"
    "The range is: 0 - 1, \n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
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

/* internal function declarations */

/* convert logical encapsulation mode (in struct SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO) to encapsulation mode hw (in struct ARAD_PP_EG_ENCAP_ACCESS_IP_TUNNEL_ENTRY_FORMAT) */
STATIC uint32 SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_TO_SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD( int unit, 
                                                                                                SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE encapsulation_mode, 
                                                                                                uint32* encapsulation_mode_hw) {
    uint32 res = SOC_SAND_OK;

    switch (encapsulation_mode) {
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_NONE) :
        *encapsulation_mode_hw = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_NONE;
    break;
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_ETHER_IP) :
        *encapsulation_mode_hw = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_ETHER_IP;
    break;
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_BASIC_GRE) :
        *encapsulation_mode_hw = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_BASIC_GRE;
    break;
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_ENHANCE_GRE) :
        *encapsulation_mode_hw = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_ENHANCE_GRE;
    break;
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_VXLAN) : 
        *encapsulation_mode_hw = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_VXLAN;
    break;
    default: 
        /* unsupported type */
        res = SOC_SAND_ERR;
        break;
    }
    return res;
}

/* convert encapsulation mode hw (in struct: ARAD_PP_EG_ENCAP_ACCESS_IP_TUNNEL_ENTRY_FORMAT) to logical encapsulation mode (in struct SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO) */
STATIC uint32 
SOC_PPC_EG_ENCAP_SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_TO_ENCAPSULATION_MODE(int unit, 
                                                                                  uint32 encapsulation_mode_hw, 
                                                                                  SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE* encapsulation_mode) {
    int vxlan_mode;
    uint32 res;

    res = SOC_SAND_OK;

    switch (encapsulation_mode_hw) {
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_NONE) :
        *encapsulation_mode = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_NONE;
    break;
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_ETHER_IP) : /* and SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_VXLAN */
        /* until arad, only EoIp is supported for this value, not vxlan */
        if (SOC_IS_ARAD_B1_AND_BELOW(unit)) { 
            *encapsulation_mode = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_ETHER_IP;
        }
        /* for arad+, encapsulation mode is shared for vxlan and ether ip, so need more info from the configuration */
        #ifdef BCM_88660_A0
        else {
            /* test soc property */
            vxlan_mode = SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & (SOC_DPP_IP_TUNNEL_TERM_DB_VXLAN);
            if (vxlan_mode) {
                *encapsulation_mode = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_VXLAN;
            } else {
                *encapsulation_mode = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_ETHER_IP;
            }
      }
      #endif
        
    break;
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_BASIC_GRE) :
        *encapsulation_mode = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_BASIC_GRE;
    break;
    case (SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_ENHANCE_GRE) :
        *encapsulation_mode = SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_ENHANCE_GRE;
    break;
    default: 
        /* unsupported type */
        return SOC_SAND_ERR;
        break;
    }

    return res;
}



STATIC uint32
  arad_pp_eg_encap_data_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  data_eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info,
    SOC_SAND_OUT uint8                                   *next_eep_valid, 
    SOC_SAND_OUT uint32                                    *next_eep
  );


STATIC soc_error_t 
    arad_pp_eg_encap_trill_entry_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info,
    SOC_SAND_OUT uint32                                  *next_eep
  ); 


uint32
  arad_pp_eg_encap_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
      reg_val;
  uint32
    res = SOC_SAND_OK;
  uint32 
    fld_val;

  uint64
    reg_val_64;
  uint64 
    fld_val_64;
  uint8 is_erspan_enable;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_PCID_LITE_SKIP(unit);


  res = arad_pp_eg_encap_access_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* set default ethenet type for snooped packet */
  reg_val = 0x88BE;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_IPV4_UNKNOWN_HEADER_CODE_CFGr, SOC_CORE_ALL, 0, IPV4_UNKNOWN_HEADER_CODE_ETHERNET_TYPEf,  reg_val));

   /* set special processing for snooped/mirrored packets */
  reg_val = 6;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_GLOBAL_CONFIGr, SOC_CORE_ALL, 0, TM_ACTION_IS_SNOOPf,  reg_val));

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_GLOBAL_CONFIGr, SOC_CORE_ALL, 0, TM_ACTION_IS_SNOOPf,  reg_val));
  

  /* > ARAD-B, if set then msb of tos index used for DF, thus only 8 profiles for TOS*/
  if(SOC_DPP_CONFIG(unit)->pp.ip_tunnel_defrag_set){
      reg_val = 1;
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_IPV4_TOS_TO_FLAGSr, SOC_CORE_ALL, 0, IPV4_TOS_TO_FLAGSf,  reg_val));
  }

  /* Configure bounce back properties.
     For Jericho, we use split horizon filter: configure orientation (aka frwrd group) at inLif and outLif */
  if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      if (SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & (SOC_DPP_IP_TUNNEL_TERM_DB_VXLAN | SOC_DPP_IP_TUNNEL_TERM_DB_NVGRE) ) {
          /* Configure bounce back properties */
          fld_val = 25; /* 22 (max-eth-header) + 3 */
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_BOUNCE_BACKr, SOC_CORE_ALL, 0, BOUNCE_BACK_THRESHOLDf,  fld_val)); 
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EGQ_BOUNCE_BACKr, SOC_CORE_ALL, 0, ENABLE_BOUNCE_BACK_FILTERf, &fld_val)); 
          fld_val |= 0x4; /* for IP */
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_BOUNCE_BACKr, SOC_CORE_ALL, 0, ENABLE_BOUNCE_BACK_FILTERf,  fld_val)); 
      }
  }
  #ifdef BCM_88660_A0
  /* configure vxlan */
  if (SOC_IS_ARADPLUS(unit) && SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & (SOC_DPP_IP_TUNNEL_TERM_DB_VXLAN)) {
      /* get current register value */
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  90,  exit, ARAD_REG_ACCESS_ERR,READ_EPNI_CFG_VXLAN_UDP_FIELDSr(unit, SOC_CORE_ALL, &reg_val_64));
      
      /* update register fields in variable */
      COMPILER_64_SET(fld_val_64, 0, 0x08);
      ARAD_FLD_TO_REG64(EPNI_CFG_VXLAN_UDP_FIELDSr, CFG_VXLAN_FLAGSf, fld_val_64 , reg_val_64, 100, exit);
      COMPILER_64_SET(fld_val_64, 0, 0x0);
      ARAD_FLD_TO_REG64(EPNI_CFG_VXLAN_UDP_FIELDSr, CFG_VXLAN_RESERVED_1f, fld_val_64, reg_val_64, 110, exit);
      COMPILER_64_SET(fld_val_64, 0, 0x0);
      ARAD_FLD_TO_REG64(EPNI_CFG_VXLAN_UDP_FIELDSr, CFG_VXLAN_RESERVED_2f, fld_val_64, reg_val_64, 120, exit);
      COMPILER_64_SET(fld_val_64, 0, ARAD_PP_EG_ENCAP_VXLAN_UDP_SRC_PORT);
      ARAD_FLD_TO_REG64(EPNI_CFG_VXLAN_UDP_FIELDSr, CFG_UDP_SRC_PORT_VXLANf, fld_val_64, reg_val_64, 130, exit);
      COMPILER_64_SET(fld_val_64, 0, ARAD_PP_VXLAN_DST_PORT);
      ARAD_FLD_TO_REG64(EPNI_CFG_VXLAN_UDP_FIELDSr, CFG_UDP_DEST_PORT_VXLANf, fld_val_64, reg_val_64, 140, exit);
      /* update register with variable */
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  150,  exit, ARAD_REG_ACCESS_ERR,WRITE_EPNI_CFG_VXLAN_UDP_FIELDSr(unit, SOC_CORE_ALL,  reg_val_64));
  }
  #endif

  is_erspan_enable = soc_property_get(unit, spn_BCM886XX_ERSPAN_TUNNEL_ENABLE, 0) ? 1 : 0;
  if (is_erspan_enable == 1) {
      /*only support Arad+/Jer_A0/QMX_A0*/
      if (SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 180, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_read(unit, EPNI_IPV4_SIZE_WITH_GRE_8_KEYr, SOC_CORE_ALL, 0, IPV4_SIZE_WITH_GRE_8_KEYf, &fld_val));
          fld_val += 8; /* add erspan length to build GRE8 encapsulation */
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 190, exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EPNI_IPV4_SIZE_WITH_GRE_8_KEYr, SOC_CORE_ALL, 0, IPV4_SIZE_WITH_GRE_8_KEYf, fld_val));
      }
  }

    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_init_unsafe()", 0, 0);
}

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
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_eg_encap_access_entry_init_unsafe(
          unit,
          lif_eep_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_init_unsafe()", lif_eep_ndx, 0);
}

/*********************************************************************
*     Sets devision of the Egress Encapsulation Table between
 *     the different usages (Link layer/ IP tunnels/ MPLS
 *     tunnels).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_range_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO                 *range_info
  )
{
  uint32
    bank_ndx,
    res = SOC_SAND_OK,
    tmp,
    nof_bits_per_bank;
  soc_reg_above_64_val_t 
      reg_val, 
      fld_val;
  soc_error_t
    rv = SOC_E_NONE;
 
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_RANGE_INFO_SET_UNSAFE);
  
  SOC_SAND_CHECK_NULL_INPUT(range_info);

  SOC_REG_ABOVE_64_CLEAR(reg_val);
  SOC_REG_ABOVE_64_CLEAR(fld_val);

  nof_bits_per_bank = SOC_DPP_DEFS_GET(unit, eg_encap_phase_nof_bits);

  for (bank_ndx=0; bank_ndx < SOC_PPC_EG_ENCAP_NOF_BANKS(unit); bank_ndx++) {
    if (range_info->bank_access_phase[bank_ndx] < SOC_PPC_NOF_EG_ENCAP_ACCESS_PHASE_TYPES(unit)) {
        tmp = range_info->bank_access_phase[bank_ndx];
        res = soc_sand_bitstream_set_any_field(&tmp, bank_ndx * nof_bits_per_bank, nof_bits_per_bank, fld_val);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20+bank_ndx, exit);
    }
  }
    
  soc_reg_above_64_field_set(unit, EPNI_EEDB_OUTLIF_ACCESSr, reg_val, EEDB_OUTLIF_ACCESSf, fld_val);
  SOC_DPP_ALLOW_WARMBOOT_WRITE(soc_reg_above_64_set(unit, EPNI_EEDB_OUTLIF_ACCESSr, REG_PORT_ANY, 0, reg_val), rv);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);

  if (SOC_IS_JERICHO_PLUS(unit)) {
      /* Both registers need to be set with the same value in QAX. */
      SOC_DPP_ALLOW_WARMBOOT_WRITE(soc_reg_above_64_set(unit, EDB_EEDB_OUTLIF_ACCESSr, REG_PORT_ANY, 0, reg_val), rv);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(rv, 40, exit);
  }

exit:
  SOC_DPP_WARMBOOT_RELEASE_HW_MUTEX(rv);
  if(rv != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_EGRESS,
              (BSL_META_U(unit,
                          " Failed while executing the macro SOC_DPP_WARMBOOT_RELEASE_HW_MUTEX.\n")));
  }
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_range_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_encap_range_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO                 *range_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_RANGE_INFO_SET_VERIFY);

  res = SOC_PPC_EG_ENCAP_RANGE_INFO_verify(unit, range_info);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_range_info_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_encap_range_info_get_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_RANGE_INFO_GET_VERIFY);

  /* Nothing to verify ... */
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_range_info_get_verify()", 0, 0);
}


/*********************************************************************
*     Sets devision of the Egress Encapsulation Table between
 *     the different usages (Link layer/ IP tunnels/ MPLS
 *     tunnels).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_range_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_RANGE_INFO                 *range_info
  )
{
  uint32
    ndx,
    res = SOC_SAND_OK;
  uint32
      nof_bits_per_bank;
  soc_reg_above_64_val_t reg_val, fld_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_RANGE_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(range_info);

  SOC_PPC_EG_ENCAP_RANGE_INFO_clear(range_info);

  SOC_REG_ABOVE_64_CLEAR(reg_val);
  SOC_REG_ABOVE_64_CLEAR(fld_val);

  SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg_above_64_get(unit, EPNI_EEDB_OUTLIF_ACCESSr, REG_PORT_ANY, 0, reg_val));

  nof_bits_per_bank = SOC_DPP_DEFS_GET(unit, eg_encap_phase_nof_bits);
 
  soc_reg_above_64_field_get(unit, EPNI_EEDB_OUTLIF_ACCESSr, reg_val, EEDB_OUTLIF_ACCESSf, fld_val); 

  for (ndx=0; ndx < SOC_PPC_EG_ENCAP_NOF_BANKS(unit); ndx++) {

      res = soc_sand_bitstream_get_any_field(fld_val, ndx*nof_bits_per_bank, nof_bits_per_bank, &range_info->bank_access_phase[ndx]);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20+ndx, exit);
  }
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_range_info_get_unsafe()", 0, 0);
}


/*********************************************************************
 *     Sets Egress Protection info.
 *     Used in Jericho as part of various entries: AC, PWE, MPLS
 *     Details: in the H file.
*********************************************************************/
uint32 arad_pp_eg_encap_protection_info_set_verify(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PROTECTION_INFO       *protection_info)
{
  uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_PROTECTION_INFO, unit, protection_info, 10, exit);
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_protection_info_set_verify()", 0, 0);
}


/*********************************************************************
 *     Set Egress Protection info into an Out-LIF extension buffer.
 *     Used in Jericho for of various entries: AC, PWE, MPLS
 *     Details: in the H file.
*********************************************************************/
uint32 arad_pp_eg_encap_protection_info_extension_buffer_set(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  uint8                                  is_full_entry_extension,
    SOC_SAND_IN  uint8                                  is_coupled,
    SOC_SAND_IN SOC_PPC_EG_ENCAP_PROTECTION_INFO        *protection_info, 
    SOC_SAND_OUT uint64                                 *protection_buffer)
{

    uint32 res = SOC_SAND_OK;
    uint32 data[sizeof(*protection_buffer)/sizeof(uint32)], tmp_val; 
    SOCDNX_INIT_FUNC_DEFS;

    soc_sand_os_memset(data, 0x0, sizeof(*protection_buffer));

    /* Set the Protection-Pointer to the buffer */
    res = soc_sand_bitstream_set_any_field(&(protection_info->protection_pointer), 
                                           ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_LSB, 
                                           ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_NOF_BITS, 
                                           data);
    SOCDNX_SAND_IF_ERR_EXIT(res);

    /* In case of a full entry in the EEDB in Coupled mode, the
       Protection-Pointer MSB stores Protection-Path value */
    if (is_coupled && is_full_entry_extension) {

        /* Override the Protection-Pointer MSB with Protection-Path value */
        tmp_val = SOC_SAND_NUM2BOOL(protection_info->protection_pass_value);
        res = soc_sand_bitstream_set_any_field(&tmp_val, 
                                               ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_PATH_LSB, 
                                               ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_PATH_NOF_BITS, 
                                               data);
        SOCDNX_SAND_IF_ERR_EXIT(res);
    }

    /* Format the buffer as a 64bit field */
    COMPILER_64_SET(*protection_buffer, data[1], data[0]);

exit:
  SOCDNX_FUNC_RETURN; 
}


/*********************************************************************
 *     Get Egress Protection info from an Out-LIF extension buffer.
 *     Used in Jericho for of various entries: AC, PWE, MPLS
 *     Details: in the H file.
*********************************************************************/
uint32 arad_pp_eg_encap_protection_info_extension_buffer_get(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  uint8                                  is_full_entry_extension,
    SOC_SAND_IN  uint8                                  is_coupled,
    SOC_SAND_IN int                                     out_lif_id,
    SOC_SAND_IN  uint64                                 protection_buffer,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PROTECTION_INFO       *protection_info)
{

    uint32 res = SOC_SAND_OK;
    uint32 data[sizeof(protection_buffer)/sizeof(uint32)], tmp_val;
    SOCDNX_INIT_FUNC_DEFS;

    soc_sand_os_memset(data, 0x0, sizeof(protection_buffer));

    /* Format the 64bit buffer to uint32 size fields */
    COMPILER_64_TO_32_LO(data[0], protection_buffer); 
    COMPILER_64_TO_32_HI(data[1], protection_buffer); 

    /* Get the Protection-Pointer ffrom the buffer */
    res = soc_sand_bitstream_get_any_field(data,
                                           ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_LSB,
                                           ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_NOF_BITS,
                                           &(protection_info->protection_pointer));
    SOCDNX_SAND_IF_ERR_EXIT(res);

    /* In case of a full entry in the EEDB in Coupled mode, the
       Protection-Pointer MSB stores Protection-Path value */
    if (is_coupled && is_full_entry_extension) {

        /* Override the Protection-Pointer MSB with Protection-Path value */
        res = soc_sand_bitstream_get_any_field(data,
                                               ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_PATH_LSB, 
                                               ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_PATH_NOF_BITS, 
                                               &tmp_val);
        SOCDNX_SAND_IF_ERR_EXIT(res);
        protection_info->protection_pass_value = SOC_SAND_BOOL2NUM(tmp_val);

        /* Make sure the retrieved Protection-Pointer MSB is reset since it
           stored the Protection-Path value */
        SHR_BITCLR(&(protection_info->protection_pointer),
                   (ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_NOF_BITS + ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_LSB - 1));
    } else {
        protection_info->protection_pass_value = 0;

        /* In half entry, Out-LIF ID LSB is also used to determine Egress Protection info */
        if (!is_full_entry_extension) {

            if (is_coupled) {
                /* In Coupled mode, the Out-LIF ID LSB is the Protection-Path value */
                protection_info->protection_pass_value = SOC_SAND_NUM2BOOL_INVERSE(out_lif_id % 2);
            } else {
                /* In De-Coupled mode, the Out-LIF ID LSB is the LSB of the Protection-Pointer */
                SHR_BITCLR(&(protection_info->protection_pointer), ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_LSB);
                protection_info->protection_pointer |= (out_lif_id % 2) << ARAD_PP_EG_ENCAP_ACCESS_PROTECTION_POINTER_LSB;
            }
        }
    }

exit:
  SOCDNX_FUNC_RETURN;
}


/*********************************************************************
 *     Set Egress Protection info into an Out-LIF extension buffer.
 *     Used in Jericho for of various entries: AC, PWE, MPLS
 *     Details: in the H file.
*********************************************************************/
uint32 arad_pp_eg_encap_protection_info_set_unsafe(
    SOC_SAND_IN int                                 unit,
    SOC_SAND_IN int                                 out_lif_id,
    SOC_SAND_IN uint8                               is_full_entry_extension,
    SOC_SAND_IN SOC_PPC_EG_ENCAP_PROTECTION_INFO    *protection_info)
{
    uint8 is_coupled, is_ingress = FALSE;
    uint32 res = SOC_SAND_OK;
    uint64 protection_buffer;
    SOCDNX_INIT_FUNC_DEFS

    if (protection_info->is_protection_valid) {

        /* Format the protection info to a HW protection extension buffer */
        is_coupled = SOC_DPP_IS_PROTECTION_EGRESS_COUPLED(unit);
        res = arad_pp_eg_encap_protection_info_extension_buffer_set(
                unit, is_full_entry_extension, is_coupled, protection_info, &protection_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(res);

        /* Set the extension buffer to the HW */
        res = arad_pp_lif_additional_data_set(unit, out_lif_id, is_ingress, protection_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(res);
    }

exit:
    SOCDNX_FUNC_RETURN; 
}


/*********************************************************************
 *     Get Egress Protection info into from an Out-LIF extension buffer.
 *     Used in Jericho for of various entries: AC, PWE, MPLS
 *     Details: in the H file.
*********************************************************************/
uint32 arad_pp_eg_encap_protection_info_get_unsafe(
    SOC_SAND_IN int                                 unit,
    SOC_SAND_IN int                                 out_lif_id,
    SOC_SAND_IN uint8                               is_full_entry_extension,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PROTECTION_INFO   *protection_info)
{
    uint8 is_coupled, is_ingress = FALSE, is_egress_wide_entry, ext_type;
    uint32 res = SOC_SAND_OK;
    uint64 protection_buffer;
    SOCDNX_INIT_FUNC_DEFS

    /* Check whether the Protection extension was allocated */
    res = arad_pp_lif_is_wide_entry(unit, out_lif_id, FALSE, &is_egress_wide_entry, &ext_type);
    SOCDNX_SAND_IF_ERR_EXIT(res);

    if (is_egress_wide_entry && (ext_type == 0)) {

        /* Get the extension buffer from the HW */
        res = arad_pp_lif_additional_data_get(unit, out_lif_id, is_ingress, &protection_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(res);

        /* Format the extension buffer to protection info */
        is_coupled = SOC_DPP_IS_PROTECTION_EGRESS_COUPLED(unit);
        res = arad_pp_eg_encap_protection_info_extension_buffer_get(
                unit, is_full_entry_extension, is_coupled, out_lif_id, protection_buffer, protection_info);
        SOCDNX_SAND_IF_ERR_EXIT(res);

        protection_info->is_protection_valid = TRUE;
    } else {
        SOC_PPC_EG_ENCAP_PROTECTION_INFO_clear(protection_info);
    }

exit:
    SOCDNX_FUNC_RETURN; 
}


/*********************************************************************
*     Set LIF Editing entry to be NULL Entry.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_null_lif_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_UNSAFE);

  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FEATURE_NOT_SUPPORTED_ERR, 10, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_null_lif_entry_add_unsafe()", lif_eep_ndx, 0);
}

uint32
  arad_pp_eg_encap_null_lif_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_VERIFY);

  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FEATURE_NOT_SUPPORTED_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_null_lif_entry_add_verify()", lif_eep_ndx, 0);
}



/*****************************************************************************
* Function:  arad_pp_eg_encap_lif_field_get_unsafe
* Purpose:   To get specific field int encap table
* Params:
* unit        - Device Number
* lif_eep_ndx -  Out-LIF-id which is index to the table
* flags       - Which value to read from table 
* val (OUT) -  value that read from encap table
* Return:    (uint32)
*******************************************************************************/
uint32  arad_pp_eg_encap_lif_field_get_unsafe(
      SOC_SAND_IN  int                               unit,
      SOC_SAND_IN  uint32                                lif_eep_ndx,
      SOC_SAND_IN  uint32                                flags,
      SOC_SAND_OUT  uint32                               *val 
  )
{
    uint32
     res = SOC_SAND_OK;
    ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
      eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_UNSAFE);

    SOC_SAND_CHECK_NULL_INPUT(val);

    res = arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, lif_eep_ndx, &eep_type);
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

    /* get outlif info */
    if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_AC) {
        ARAD_PP_EG_ENCAP_ACCESS_OUT_AC_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_out_ac_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 6, exit);
        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF) {
            /* out-ac has no next outlif */
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_ACCESS_ENTRY_TYPE_MISMATCH_ERR, 10, exit);
        }
        ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry,flags,val);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_TRILL) {
        ARAD_PP_EG_ENCAP_ACCESS_TRILL_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_trill_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        ARAD_PP_EG_ENCAP_GET_FIELD_NEXT_LIF(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry,flags,val);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_LINK_LAYER) {
        ARAD_PP_EG_ENCAP_ACCESS_LINK_LAYER_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_link_layer_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF){
            *val = entry.next_outlif_lsb;
        }
        ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry,flags,val);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_RIF) {
        ARAD_PP_EG_ENCAP_ACCESS_OUT_RIF_ENTRY_FORMAT
                entry;
        if (SOC_IS_JERICHO_PLUS(unit)) {
            /*RIF in EEDB in QAX point to QAX RIF table and does nto contain all entries, meaning Not all Fileds can be get/set*/
            if((flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_OUT_LIF_PROFILE) == FALSE &&
                (flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_DROP) == FALSE )
            { 
               SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_ACCESS_ENTRY_TYPE_MISMATCH_ERR, 10, exit);  
            }
            res = qax_pp_eg_encap_access_out_rif_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        
        } else { 
            res = arad_pp_eg_encap_access_out_rif_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
            ARAD_PP_EG_ENCAP_GET_FIELD_NEXT_LIF(entry,flags,val);
            
        }
        ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry,flags,val);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA || eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_IPV6) {
        ARAD_PP_EG_ENCAP_ACCESS_DATA_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_data_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        ARAD_PP_EG_ENCAP_GET_FIELD_NEXT_LIF(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry,flags,val);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_IP) {
        ARAD_PP_EG_ENCAP_ACCESS_IP_TUNNEL_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_ip_tunnel_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        ARAD_PP_EG_ENCAP_GET_FIELD_NEXT_LIF(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry,flags,val);
    }
    else if( eep_type ==  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_TUNNEL ||
        eep_type ==  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_SWAP ||
        eep_type ==  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_POP
      ) 
      {
        ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT  
            entry;
        res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        ARAD_PP_EG_ENCAP_GET_FIELD_NEXT_LIF(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_GET_FIELD_OUT_LIF_PROFILE(entry,flags,val);
    }

    else{
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_ACCESS_ENTRY_TYPE_MISMATCH_ERR, 10, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_next_outlif_update_unsafe()", lif_eep_ndx, 0);
}


/*****************************************************************************
* Function:  arad_pp_eg_encap_lif_field_set_unsafe
* Purpose:   Set a field in encap entry table
* Params:
* unit (IN)       - Device Number
* lif_eep_ndx (IN) - index to table which is the Out-LiF-id
* flags (IN)      - which entry to update
* val   (IN)      -  the value to update the table wich
* Return:    (uint32) ERROR type, NONE in case of success
* (Previously named soc_ppd_eg_encap_next_outlif_update_unsafe)
*******************************************************************************/
uint32 arad_pp_eg_encap_lif_field_set_unsafe(
      SOC_SAND_IN  int                               unit,
      SOC_SAND_IN  uint32                                lif_eep_ndx,
      SOC_SAND_IN  uint32                                flags,
      SOC_SAND_IN  uint32                                val
  )
{
    uint32
     res = SOC_SAND_OK;
    ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
      eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_UNSAFE);

    res = arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, lif_eep_ndx, &eep_type);
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

    /* get outlif info */
    if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_AC) {
        ARAD_PP_EG_ENCAP_ACCESS_OUT_AC_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_out_ac_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 6, exit);
        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF) {
            /* out-ac has no next outlif */
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_ACCESS_ENTRY_TYPE_MISMATCH_ERR, 10, exit);
        }
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);

        res = arad_pp_eg_encap_access_out_ac_entry_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);

    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_TRILL) {
        ARAD_PP_EG_ENCAP_ACCESS_TRILL_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_trill_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF) {
            entry.next_outlif = val;
            entry.next_outlif_valid = val != 0;
        }
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);
        
        res = arad_pp_eg_encap_access_trill_entry_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_LINK_LAYER) {
        ARAD_PP_EG_ENCAP_ACCESS_LINK_LAYER_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_link_layer_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF){
            entry.next_outlif_lsb = val;
            entry.next_outlif_valid = val != 0;
        }
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);
        
        res = arad_pp_eg_encap_access_link_layer_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_RIF) {
        ARAD_PP_EG_ENCAP_ACCESS_OUT_RIF_ENTRY_FORMAT
                entry;
        if (SOC_IS_JERICHO_PLUS(unit)) {
            /*RIF in EEDB in QAX point to QAX RIF table and does nto contain all entries, meaning Not all Fileds can be get/set*/
            if((flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_OUT_LIF_PROFILE) == FALSE &&
                (flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_DROP) == FALSE )
            { 
               SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_ACCESS_ENTRY_TYPE_MISMATCH_ERR, 10, exit);  
            }
            res = qax_pp_eg_encap_access_out_rif_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
            ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);
            ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
            res = qax_pp_eg_encap_access_out_rif_entry_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        } else {
            res = arad_pp_eg_encap_access_out_rif_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
            if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF){
                entry.next_outlif = val;
                entry.next_outlif_valid = val != 0;
            }
            ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
            ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);

            res = arad_pp_eg_encap_access_out_rif_entry_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA || eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_IPV6) {
        ARAD_PP_EG_ENCAP_ACCESS_DATA_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_data_entry_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF){
            entry.next_outlif = val;
            entry.next_outlif_valid = val != 0;
        }
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);
        
        res = arad_pp_eg_encap_access_data_entry_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }
    else if(eep_type == ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_IP) {
        ARAD_PP_EG_ENCAP_ACCESS_IP_TUNNEL_ENTRY_FORMAT
            entry;
        res = arad_pp_eg_encap_access_ip_tunnel_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF){
            entry.next_outlif = val;
        }
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);
        
        res = arad_pp_eg_encap_access_ip_tunnel_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }
    else if( eep_type ==  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_TUNNEL ||
        eep_type ==  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_SWAP ||
        eep_type ==  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_POP
      ) 
      {
        ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT  
            entry;
        res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_get_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(flags & SOC_PPC_EG_ENCAP_ENTRY_UPDATE_NEXT_LIF){
            entry.next_outlif = val;
        }
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_DROP(entry,flags,val);
        ARAD_PP_EG_ENCAP_UPDATE_FIELD_OUT_LIF_PROFILE(entry,flags,val);
        
        res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_set_unsafe(unit,lif_eep_ndx,&entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }

    else{
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_ACCESS_ENTRY_TYPE_MISMATCH_ERR, 10, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_next_outlif_update_unsafe()", lif_eep_ndx, 0);
}


uint32
  arad_pp_eg_encap_lif_field_set_verify(
      SOC_SAND_IN  int                               unit,
      SOC_SAND_IN  uint32                                lif_eep_ndx,
      SOC_SAND_IN  uint32                                flags,
      SOC_SAND_IN  uint32                                val
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_NULL_LIF_ENTRY_ADD_VERIFY);

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_next_outlif_update_verify()", lif_eep_ndx, 0);
}


uint32
  arad_pp_eg_encap_data_local_to_global_cud_init_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_INIT_VERIFY);

  if (!(SOC_IS_ARADPLUS(unit)) || 
      ((SOC_DPP_CONFIG(unit)->tm.mc_mode & DPP_MC_CUD_EXTENSION_MODE) == FALSE)) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_UNSUPPORTED, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_local_to_global_cud_init_verify()", 0, 0);
}

uint32
  arad_pp_eg_encap_data_local_to_global_cud_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK,
      eedb_mem_index = 0,
      eedb_array_index = 0,
      eedb_data_index = 0,
      in_out_cud;
  int
      dma_able, 
      epni_eedb_bank_buf_size,
      epni_eedb_bank_nof_elements,
      epni_eedb_bank_nof_bit_in_elment;
  uint32 
      flags=0;
  uint32 
      *epni_eedb_bank_buf;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_INIT_UNSAFE);

  epni_eedb_bank_nof_elements = 2048;
  epni_eedb_bank_nof_bit_in_elment = 96;/*80 bits in an element rounded up to 96*/
  epni_eedb_bank_buf_size = (epni_eedb_bank_nof_elements * (epni_eedb_bank_nof_bit_in_elment/32));


  dma_able = soc_mem_dmaable(unit, EPNI_EEDB_BANKm, 0);

  if (dma_able) {
     epni_eedb_bank_buf = (uint32 *)soc_cm_salloc(unit,epni_eedb_bank_buf_size*sizeof(uint32), "EEDB DMA buffer");
  } else {
      epni_eedb_bank_buf = (uint32 *)sal_alloc(epni_eedb_bank_buf_size*sizeof(uint32), "EEDB DMA buffer");
  }
  
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  for (eedb_array_index = 0; eedb_array_index < (1 << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_NOF_BITS) ; eedb_array_index++) 
  {
      res = soc_sand_os_memset(epni_eedb_bank_buf, 0x0, epni_eedb_bank_buf_size*sizeof(uint32));
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
      /* 
       * for each array index build a buffer, 
       * Where the 
       *     
       * CUD21[18:0] comes from EPNI_EEDB_BANKm array index: CUD17[16..13]] memory index: CUD[12..2], 
       * Each entry contains four 19 bit values selectable by CUD17[1..0].
       *  
       *  
       */

      for(eedb_mem_index = 0 ,in_out_cud = 0 ;
          eedb_mem_index < (1 << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_NOF_BITS) ;
          eedb_mem_index++, in_out_cud = 0) 
      {
          for(eedb_data_index = 0 ;
              eedb_data_index <  (1 << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_NOF_BITS) ;
              eedb_data_index++) 
          {
              /*build buffer*/
              /*build in-out cud*/
              in_out_cud  = eedb_data_index;
              in_out_cud |= (eedb_mem_index << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_START);
              in_out_cud |= (eedb_array_index << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_START);
              /*set in_out_cud in the relevant palce*/
              res = soc_sand_bitstream_set_any_field(&in_out_cud ,
                                                    (eedb_mem_index * epni_eedb_bank_nof_bit_in_elment) + (eedb_data_index*ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB) ,
                                                     ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB ,
                                                     epni_eedb_bank_buf);
              SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
          }
      }
      res = soc_mem_array_write_range(unit, flags, EPNI_EEDB_BANKm, eedb_array_index, MEM_BLOCK_ALL, 0, (epni_eedb_bank_nof_elements - 1),  epni_eedb_bank_buf);
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }

  /*
   * CUD21[20:19] comes from EPNI_DSCP_REMARKm index CUD17[16..4], 
   * Each entry contains sixteen two bit values selectable using CUD17[3..0].
   *  
   * For one to one cud mapping, we can set all of EPNI_DSCP_REMARKm to be zero, 
   * Since the incoming cud is never above 17 bit, So not action required
   *  
   */

exit:
  if (dma_able) {
      soc_cm_sfree(unit, epni_eedb_bank_buf);
  } else {
      sal_free(epni_eedb_bank_buf);
  }
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_local_to_global_cud_init_unsafe()", 0, 0);
}

/*********************************************************************
*     Set CUD global extension.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_data_local_to_global_cud_set_verify(
     SOC_SAND_IN  int                                  unit,
     SOC_SAND_IN  uint32                                  in_cud,
     SOC_SAND_OUT uint32                                  out_cud
   )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET_VERIFY);
    if (!(SOC_IS_ARADPLUS(unit)) || 
        ((SOC_DPP_CONFIG(unit)->tm.mc_mode & DPP_MC_CUD_EXTENSION_MODE) == FALSE)) {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_UNSUPPORTED, 5, exit);
    }
    SOC_SAND_ERR_IF_ABOVE_MAX(in_cud , (1 << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_IN_CUD_NOF_BITS) - 1, ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_OUT_OF_RANGE_ERR, 10, exit);
    SOC_SAND_ERR_IF_ABOVE_MAX(out_cud ,(1 << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_OUT_CUD_NOF_BITS) - 1, ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_OUT_OF_RANGE_ERR, 20, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_local_to_global_cud_set_verify()", in_cud, 0);
}

/*********************************************************************
*     Set CUD global extension.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_data_local_to_global_cud_set_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint32                                  in_cud,
    SOC_SAND_IN  uint32                                  out_cud
  )
{
    uint32 
        res = SOC_SAND_OK,
        eedb_mem_index = 0,
        eedb_array_index = 0,
        eedb_data_index = 0,
        eedb_data[ARAD_PP_EG_ENCAP_ACCESS_FORMAT_TBL_ENTRY_SIZE],
        dscp_data_index = 0,
        dscp_mem_index = 0,
        dscp_out_cud = 0;
    ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA
        dscp_tbl_data;
    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_SET_UNSAFE);
    /*
     * CUD21[18:0] comes from EPNI_EEDB_BANKm array index: CUD17[16..13]] memory index: CUD[12..2], 
     * Each entry contains four 19 bit values selectable by CUD17[1..0].
     *  
     * CUD21[20:19] comes from EPNI_DSCP_REMARKm index CUD17[16..4], 
     * Each entry contains sixteen two bit values selectable using CUD17[3..0].
     * 
     */
    /*eedb_mem_index = in_cud[12-2]; get bits 12-2*/
    res = soc_sand_bitstream_get_any_field(&in_cud , 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_START, 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_NOF_BITS, 
                                           &eedb_mem_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /*eedb_array_index = in_cud[16-13]; get bits 16-13*/
    res = soc_sand_bitstream_get_any_field(&in_cud, 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_START, 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_NOF_BITS, 
                                           &eedb_array_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

    /*eedb_data_index = in_cud[1-0]; get bits 1-0*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_START,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_NOF_BITS,
                                           &eedb_data_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 24, exit);
    

    res = READ_EDB_EEDB_BANKm(unit, eedb_array_index, MEM_BLOCK_ANY, eedb_mem_index, eedb_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

    /*write the new mapping data*/
    res = soc_sand_bitstream_set_any_field(&out_cud , (eedb_data_index*ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB) , ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB , eedb_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    /*set the new mapping data*/
    res = WRITE_EDB_EEDB_BANKm(unit, eedb_array_index, MEM_BLOCK_ANY, eedb_mem_index, eedb_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);


    /*dscp_mem_index = in_cud[16-4]; get bits 16-4*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_MEM_INDEX_START,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_MEM_INDEX_NOF_BITS,
                                           &dscp_mem_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    /*dscp_data_index = in_cud[3-0]; get bits 3-0*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_START,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_NOF_BITS,
                                           &dscp_data_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);


    res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit,dscp_mem_index,&dscp_tbl_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    /*write the new mapping data*/
    res = soc_sand_bitstream_get_any_field(&out_cud , ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB , ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_DSCP , &dscp_out_cud);
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

    res = soc_sand_bitstream_set_any_field(&dscp_out_cud , (dscp_data_index*ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_DSCP), ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_DSCP, &(dscp_tbl_data.dscp_remark_data));
    SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
    /*set the new mapping data*/
    res =  arad_pp_epni_dscp_remark_tbl_set_unsafe(unit,dscp_mem_index,&dscp_tbl_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);


exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_local_to_global_cud_set_unsafe()", in_cud, out_cud);

}

/*********************************************************************
*     Get CUD global extension.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_data_local_to_global_cud_get_verify(
     SOC_SAND_IN  int                                  unit,
     SOC_SAND_IN  uint32                                  in_cud,
     SOC_SAND_OUT uint32*                                 out_cud
   )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET_VERIFY);
    if (!(SOC_IS_ARADPLUS(unit)) || 
        ((SOC_DPP_CONFIG(unit)->tm.mc_mode & DPP_MC_CUD_EXTENSION_MODE) == FALSE)) {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_UNSUPPORTED, 5, exit);
    }
    SOC_SAND_ERR_IF_ABOVE_MAX(in_cud , (1 << ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_IN_CUD_NOF_BITS) - 1 , ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_OUT_OF_RANGE_ERR, 20, exit);

exit:    
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_local_to_global_cud_get_verify()", in_cud, 0);
}
/*********************************************************************
*     Get CUD global extension.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_data_local_to_global_cud_get_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint32                                  in_cud,
    SOC_SAND_OUT uint32*                                 out_cud
  )
{
    uint32 
        res = SOC_SAND_OK,
        eedb_mem_index = 0,
        eedb_array_index = 0,
        eedb_data_index = 0,
        eedb_data[ARAD_PP_EG_ENCAP_ACCESS_FORMAT_TBL_ENTRY_SIZE],
        dscp_data_index = 0,
        dscp_mem_index = 0,
        dscp_out_cud = 0,
        out_cud_lcl[1];
    ARAD_PP_EPNI_DSCP_REMARK_TBL_DATA
        dscp_tbl_data;        

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_GET_UNSAFE);
    /*
     * CUD21[18:0] comes from EPNI_EEDB_BANKm array index: CUD17[16..13]] memory index: CUD[12..2], 
     * Each entry contains four 19 bit values selectable by CUD17[1..0].
     *  
     * CUD21[20:19] comes from EPNI_DSCP_REMARKm index CUD17[16..4], 
     * Each entry contains sixteen two bit values selectable using CUD17[3..0].
     * 
     */
    *out_cud_lcl = 0x0;
    /*eedb_mem_index = in_cud[12-2]; get bits 12-2*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_START,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_MEM_INDEX_NOF_BITS,
                                           &eedb_mem_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /*eedb_array_index = in_cud[16-13]; get bits 16-13*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_START,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_ARRAY_INDEX_NOF_BITS,
                                           &eedb_array_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

    /*eedb_data_index = in_cud[1-0]; get bits 1-0*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_START, 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_EEDB_DATA_INDEX_NOF_BITS, 
                                           &eedb_data_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 24, exit);

    res = READ_EDB_EEDB_BANKm(unit, eedb_array_index, MEM_BLOCK_ANY, eedb_mem_index, eedb_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

    /*read mapping data*/
    res = soc_sand_bitstream_get_any_field(eedb_data, 
                                           (eedb_data_index * ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB), 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB,
                                           out_cud_lcl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    /*dscp_mem_index = in_cud[16-4]; get bits 16-4*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_MEM_INDEX_START, 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_MEM_INDEX_NOF_BITS,
                                           &dscp_mem_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    /*dscp_data_index = in_cud[3-0]; get bits 3-0*/
    res = soc_sand_bitstream_get_any_field(&in_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_START,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_DSCP_DATA_INDEX_NOF_BITS,
                                           &dscp_data_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    res = arad_pp_epni_dscp_remark_tbl_get_unsafe(unit,dscp_mem_index,&dscp_tbl_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    /*read mapping data*/
    res = soc_sand_bitstream_get_any_field(&(dscp_tbl_data.dscp_remark_data) , (dscp_data_index*ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_DSCP), ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_DSCP, &dscp_out_cud);
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

    res = soc_sand_bitstream_set_any_field(&dscp_out_cud,
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_EEDB, 
                                           ARAD_PP_EG_ENCAP_DATA_LOCAL_TO_GLOBAL_CUD_NOF_BITS_IN_DSCP, 
                                           out_cud_lcl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
    *out_cud = *out_cud_lcl;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_local_to_global_cud_get_unsafe()", in_cud, 0);

}

/*********************************************************************
*     Set LIF Editing entry to be DATA Entry.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_data_lif_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO              *data_info,
    SOC_SAND_IN  uint8                                   next_eep_valid,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK,
    ind;
  ARAD_PP_EG_ENCAP_ACCESS_DATA_ENTRY_FORMAT
    tbl_data;
  

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LIF_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(data_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  for (ind = 0; ind < SOC_PPC_EG_ENCAP_DATA_INFO_MAX_SIZE; ind++) {
    tbl_data.data[ind] = data_info->data_entry[ind];
  }
  
  tbl_data.next_outlif = next_eep;
  tbl_data.next_outlif_valid = SOC_SAND_BOOL2NUM(next_eep_valid);
  tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(data_info->oam_lif_set);
  tbl_data.drop = SOC_SAND_BOOL2NUM(data_info->drop);
  tbl_data.outlif_profile = data_info->outlif_profile;

  res = arad_pp_eg_encap_access_data_entry_format_tbl_set_unsafe(
          unit,
          lif_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_lif_entry_add_unsafe()", lif_eep_ndx, 0);
}

uint32
  arad_pp_eg_encap_data_lif_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO              *data_info,
    SOC_SAND_IN  uint8                                   next_eep_valid,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_LIF_ENTRY_ADD_VERIFY);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, lif_eep_ndx, ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_DATA_INFO, data_info, 20, exit);
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, next_eep, ARAD_PP_EG_ENCAP_NEXT_EEP_OUT_OF_RANGE_ERR, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_lif_entry_add_verify()", lif_eep_ndx, next_eep);
}

/*********************************************************************
*     Set LIF Editing entry to hold MPLS LSR SWAP label.
 *     Needed for MPLS multicast services.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_swap_command_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO                  *swap_info,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(swap_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  tbl_data.mpls1_command = ARAD_PP_EG_ENCAP_MPLS1_COMMAND_SWAP_val;
  tbl_data.mpls1_label = swap_info->swap_label;
  tbl_data.next_outlif = next_eep;
  tbl_data.next_vsi_lsb = swap_info->out_vsi;
  tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(swap_info->oam_lif_set);
  tbl_data.drop = SOC_SAND_BOOL2NUM(swap_info->drop);
  tbl_data.mpls_swap_remark_profile = swap_info->remark_profile;
  tbl_data.outlif_profile = swap_info->outlif_profile;

  res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_set_unsafe(
          unit,
          lif_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_swap_command_entry_add_unsafe()", lif_eep_ndx, 0);
}

uint32
  arad_pp_eg_encap_swap_command_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO                  *swap_info,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_SWAP_COMMAND_ENTRY_ADD_VERIFY);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, lif_eep_ndx, ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_SWAP_INFO, swap_info, 20, exit);
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, next_eep, ARAD_PP_EG_ENCAP_NEXT_EEP_OUT_OF_RANGE_ERR, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_swap_command_entry_add_verify()", lif_eep_ndx, 0);
}

/*********************************************************************
*     Set LIF Editing entry to hold PWE info (VC label and
 *     push profile).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_pwe_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO                   *pwe_info,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT
    tbl_data;
  uint8 is_full_entry_extension = TRUE;

  SOC_PPC_EG_ENCAP_ENTRY_INFO               encap_entry_info[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_MAX];
  uint32 next_eep_dummy[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_MAX];
  uint32 nof_entries;
  uint32 default_label_value = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(pwe_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  /* In Jericho B0 and above, default label value is configurable */
  default_label_value = SOC_IS_JERICHO_B0_AND_ABOVE(unit) ? soc_property_get(unit, spn_MPLS_ENCAP_INVALID_VALUE, 0) : 0;

  /* Check if this entry contains MPLS half entry, if so not delete it */
  res = arad_pp_eg_encap_entry_get_unsafe(unit, SOC_PPC_EG_ENCAP_EEP_TYPE_TUNNEL_EEP, lif_eep_ndx,
                                          2, encap_entry_info, next_eep_dummy, &nof_entries);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* If mpls_bind_pwe_with_mpls_one_call is unset, mpls label data is retained via fetching from the HW
     If mpls_bind_pwe_with_mpls_one_call is set, mpls label data was created via bcm_mpls_port_add, thus
     all information comes from the pwe_info struct */
  if (!soc_property_get(unit, spn_MPLS_BIND_PWE_WITH_MPLS_ONE_CALL, 0)) {
      if ((nof_entries >= 1) && (encap_entry_info[0].entry_type == SOC_PPC_EG_ENCAP_ENTRY_TYPE_MPLS_ENCAP) &&
          encap_entry_info[0].entry_val.mpls_encap_info.nof_tunnels > 1) {

          tbl_data.mpls2_label = encap_entry_info[0].entry_val.mpls_encap_info.tunnels[1].tunnel_label;
          tbl_data.mpls2_command = encap_entry_info[0].entry_val.mpls_encap_info.tunnels[1].push_profile;
      } else if (SOC_IS_JERICHO_B0_AND_ABOVE(unit)) {
          /* In Jericho B0 and above, default label value is configurable */
          tbl_data.mpls2_label = default_label_value;
      }
  } else { /* Fetch mpls label information from the pwe struct */
      if (pwe_info->egress_tunnel_label_info.nof_tunnels == 1) { /* This means that the user wishes to configure the mpls label (mpls2) in the PWE entry */
          tbl_data.mpls2_label = pwe_info->egress_tunnel_label_info.tunnels[0].tunnel_label;
          tbl_data.mpls2_command = SOC_PPC_MPLS_COMMAND_TYPE_PUSH + pwe_info->egress_tunnel_label_info.tunnels[0].push_profile;
      } else {
          tbl_data.mpls2_label = default_label_value;
          tbl_data.mpls2_command = 0;
      }
  }



 /* Push commands in HW implementations is 0 + push_profile */
  tbl_data.mpls1_command = ARAD_PP_EG_ENCAP_MPLS1_COMMAND_PUSH_val + pwe_info->push_profile;
  tbl_data.mpls1_label = pwe_info->label;
  tbl_data.next_outlif = next_eep;
  tbl_data.next_vsi_lsb = 0;
  tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(pwe_info->oam_lif_set);
  tbl_data.drop = SOC_SAND_BOOL2NUM(pwe_info->drop);
  tbl_data.outlif_profile = pwe_info->outlif_profile;
  
  res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_set_unsafe(
          unit,
          lif_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* Set the Protection extension info for Jericho */
  if (SOC_IS_JERICHO(unit)) {
      res = arad_pp_eg_encap_protection_info_set_unsafe(
                unit, lif_eep_ndx, is_full_entry_extension, &(pwe_info->protection_info));
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pwe_entry_add_unsafe()", lif_eep_ndx, 0);
}

uint32
  arad_pp_eg_encap_pwe_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO                   *pwe_info,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PWE_ENTRY_ADD_VERIFY);
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, lif_eep_ndx, ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_PWE_INFO, unit, pwe_info, 20, exit);
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, next_eep, ARAD_PP_EG_ENCAP_NEXT_EEP_OUT_OF_RANGE_ERR, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pwe_entry_add_verify()", lif_eep_ndx, next_eep);
}

/*********************************************************************
*     Set LIF Editing entry to hold MPLS LSR POP command.
 *     Needed for MPLS multicast services.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_pop_command_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO                   *pop_info,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(pop_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  tbl_data.model_is_pipe = pop_info->model == SOC_SAND_PP_MPLS_TUNNEL_MODEL_PIPE ? 0x1 : 0x0;
  tbl_data.upper_layer_protocol = pop_info->pop_next_header;
  tbl_data.mpls1_command = ARAD_PP_EG_ENCAP_MPLS1_COMMAND_POP_val;
  tbl_data.next_outlif = next_eep;
  tbl_data.next_vsi_lsb = pop_info->out_vsi;
  tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(pop_info->oam_lif_set);
  tbl_data.drop = SOC_SAND_BOOL2NUM(pop_info->drop);

  tbl_data.has_cw = SOC_SAND_BOOL2NUM(pop_info->ethernet_info.has_cw);
  tbl_data.tpid_profile = pop_info->ethernet_info.tpid_profile;
  
  res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_set_unsafe(
          unit,
          lif_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pop_command_entry_add_unsafe()", lif_eep_ndx, next_eep);
}

uint32
  arad_pp_eg_encap_pop_command_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO                   *pop_info,
    SOC_SAND_IN  uint32                                  next_eep
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_POP_COMMAND_ENTRY_ADD_VERIFY);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, lif_eep_ndx, ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_POP_INFO, pop_info, 20, exit);
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, next_eep, ARAD_PP_EG_ENCAP_NEXT_EEP_OUT_OF_RANGE_ERR, 30, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pop_command_entry_add_verify()", lif_eep_ndx, 0);
}


uint32
  arad_pp_eg_encap_vsi_entry_add_verify(
    SOC_SAND_IN int                              unit, 
    SOC_SAND_IN uint32                               lif_eep_ndx, 
    SOC_SAND_IN SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO      *vsi_info, 
    SOC_SAND_IN uint8                              next_eep_valid,
    SOC_SAND_IN uint32                               next_eep 
  )
{
  uint32
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_VERIFY);

  res = SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO_verify(unit, vsi_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, lif_eep_ndx, ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR, 30, exit);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, next_eep, ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR, 40, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_vsi_entry_add_verify()", lif_eep_ndx, 0);
}

/*********************************************************************
 *     Add out-RIF encapsulation entry to the Tunnels
 *     Editing Table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_vsi_entry_add_unsafe(
    SOC_SAND_IN int                              unit, 
    SOC_SAND_IN uint32                               lif_eep_ndx, 
    SOC_SAND_IN SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO          *vsi_info, 
    SOC_SAND_IN uint8                              next_eep_valid,                          
    SOC_SAND_IN uint32                               next_eep 
  )
{ 
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_OUT_RIF_ENTRY_FORMAT
    tbl_data;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_VSI_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(vsi_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));


  tbl_data.next_outlif_valid  = SOC_SAND_BOOL2NUM(next_eep_valid);
  tbl_data.next_outlif = (tbl_data.next_outlif_valid) ? next_eep:0;
  tbl_data.next_vsi_lsb = vsi_info->out_vsi;
  tbl_data.remark_profile = vsi_info->remark_profile;
  tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(vsi_info->oam_lif_set);
  tbl_data.drop = SOC_SAND_BOOL2NUM(vsi_info->drop);
      
  if (SOC_IS_JERICHO_PLUS(unit)) {
      tbl_data.outrif_profile_index = vsi_info->outrif_profile_index;
      res = qax_pp_eg_encap_access_out_rif_entry_format_tbl_set_unsafe(
              unit,
              lif_eep_ndx,
              &tbl_data
            );
  } else {
      res = arad_pp_eg_encap_access_out_rif_entry_format_tbl_set_unsafe(
              unit,
              lif_eep_ndx,
              &tbl_data
            );
  }
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_vsi_entry_add_unsafe()", lif_eep_ndx, 0);
}

/*********************************************************************
*     Add MPLS tunnels encapsulation entry to the Tunnels
 *     Editing Table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_mpls_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO            *mpls_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  )
{
  uint32
    res = SOC_SAND_OK, default_label_value = 0;
  ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT
    ee_entry_tbl_data;
  uint8 is_full_entry_extension = TRUE;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(mpls_encap_info);

  soc_sand_os_memset(&ee_entry_tbl_data, 0x0, sizeof(ee_entry_tbl_data));

  /* In Jericho B0 and above, default label value is configurable */
  default_label_value = SOC_IS_JERICHO_B0_AND_ABOVE(unit) ? soc_property_get(unit, spn_MPLS_ENCAP_INVALID_VALUE, 0) : 0;

  ee_entry_tbl_data.next_vsi_lsb = mpls_encap_info->out_vsi;
  ee_entry_tbl_data.next_outlif = ll_eep;
  ee_entry_tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(mpls_encap_info->oam_lif_set);
  ee_entry_tbl_data.drop = SOC_SAND_BOOL2NUM(mpls_encap_info->drop);
  ee_entry_tbl_data.outlif_profile = mpls_encap_info->outlif_profile;

  if (mpls_encap_info->nof_tunnels > 0)
  {
    /* 1st encapsulation */
    ee_entry_tbl_data.mpls1_command = ARAD_PP_EG_ENCAP_MPLS1_COMMAND_PUSH_val + mpls_encap_info->tunnels[0].push_profile; 
    ee_entry_tbl_data.mpls1_label = mpls_encap_info->tunnels[0].tunnel_label;
  }
  else
  {
    ee_entry_tbl_data.mpls1_command = 0;
    ee_entry_tbl_data.mpls1_label = 0;
  }

  if (mpls_encap_info->nof_tunnels > 1)
  {
    /* 2nd encapsulation */
    ee_entry_tbl_data.mpls2_command = SOC_PPC_MPLS_COMMAND_TYPE_PUSH + mpls_encap_info->tunnels[1].push_profile;
    ee_entry_tbl_data.mpls2_label = mpls_encap_info->tunnels[1].tunnel_label;
  }
  else /* no second encap */
  {
    ee_entry_tbl_data.mpls2_command = 0;
    ee_entry_tbl_data.mpls2_label = default_label_value;
  }

  res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_set_unsafe(
          unit,
          tunnel_eep_ndx,
          &ee_entry_tbl_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* Set the Protection extension info for Jericho */
  if (SOC_IS_JERICHO(unit)) {
      res = arad_pp_eg_encap_protection_info_set_unsafe(
                unit, tunnel_eep_ndx, is_full_entry_extension, &(mpls_encap_info->protection_info));
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_entry_add_unsafe()", tunnel_eep_ndx, ll_eep);
}

uint32
  arad_pp_eg_encap_mpls_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO            *mpls_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_MPLS_ENTRY_ADD_VERIFY);

  ARAD_PP_EG_ENCAP_NDX_OF_TYPE_VERIFY(tunnel_eep_ndx);
  
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, tunnel_eep_ndx, ARAD_PP_EG_ENCAP_IP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO, unit, mpls_encap_info, 20, exit);
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, ll_eep, ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR, 30, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_entry_add_verify()", tunnel_eep_ndx, 0);
}



/*********************************************************************
*     Add IPv4 tunnels encapsulation entry to the Egress
 *     Encapsulation Tunnels Editing Table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO            *ipv4_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_IP_TUNNEL_ENTRY_FORMAT
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(ipv4_encap_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  tbl_data.next_outlif = ll_eep;
  tbl_data.next_vsi_lsb = ipv4_encap_info->out_vsi;
  if (SOC_IS_JERICHO_B0_AND_ABOVE(unit) && (SOC_JER_PP_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_ENABLE == 1)) {
      tbl_data.encapsulation_mode = ipv4_encap_info->dest.encapsulation_mode_index; 
  } else {
      SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_TO_SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD(unit, ipv4_encap_info->dest.encapsulation_mode, &(tbl_data.encapsulation_mode));
  }
  tbl_data.ipv4_tos_index = ipv4_encap_info->dest.tos_index;
  tbl_data.ipv4_ttl_index = ipv4_encap_info->dest.ttl_index;
  tbl_data.ipv4_src_index = ipv4_encap_info->dest.src_index;
  tbl_data.ipv4_dst = ipv4_encap_info->dest.dest;
  tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(ipv4_encap_info->oam_lif_set);
  tbl_data.drop = SOC_SAND_BOOL2NUM(ipv4_encap_info->drop);
  tbl_data.outlif_profile = ipv4_encap_info->outlif_profile;

  res = arad_pp_eg_encap_access_ip_tunnel_format_tbl_set_unsafe(
          unit,
          tunnel_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_entry_add_unsafe()", tunnel_eep_ndx, 0);
}

uint32
  arad_pp_eg_encap_ipv4_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO            *ipv4_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_ENTRY_ADD_VERIFY);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, tunnel_eep_ndx, ARAD_PP_EG_ENCAP_IP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR, 5, exit);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO, ipv4_encap_info, 10, exit);

  ARAD_PP_EG_ENCAP_NDX_OF_TYPE_VERIFY(tunnel_eep_ndx);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, ll_eep, ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR, 20, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_entry_add_verify()", tunnel_eep_ndx, 0);
}


STATIC uint32
  arad_pp_eg_encap_ipv6_sip_entry_to_prge_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO       *ipv6_encap_info,
    SOC_SAND_OUT  soc_reg_above_64_val_t                 prge_data_entry
  )
{
    /* sip_data[0   +: 96]        = this.sip[127:32]; */
    SHR_BITCOPY_RANGE(prge_data_entry,0,ipv6_encap_info->tunnel.src.address,32,96);

    /* sip_data[96  +:  8]        = this.hop_limit;*/
    prge_data_entry[96/32] = ipv6_encap_info->tunnel.hop_limit << (96% 32);

    /* sip_data[104 +:  8]        = this.next_header;*/
    prge_data_entry[104/32] |= ipv6_encap_info->tunnel.next_header << (104 % 32);

    /* sip_data[112 +: 16]        = 16'b0; flow_label;*/
    prge_data_entry[112/32] |= ipv6_encap_info->tunnel.flow_label << (112 % 32);

    return SOC_SAND_OK;
}

STATIC uint32
  arad_pp_eg_encap_ipv6_sip_entry_from_prge_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO       *ipv6_encap_info,
    SOC_SAND_IN  soc_reg_above_64_val_t                 prge_data_entry
  )
{
    /* sip_data[0   +: 96]        = this.sip[127:32]; */
    SHR_BITCOPY_RANGE(ipv6_encap_info->tunnel.src.address,32,prge_data_entry,0,96);

    /* sip_data[96  +:  8]        = this.hop_limit;*/
    ipv6_encap_info->tunnel.hop_limit = (prge_data_entry[96/32] >> (96 % 32)) & 0xff;

    /* sip_data[104 +:  8]        = this.next_header;*/
    ipv6_encap_info->tunnel.next_header = (prge_data_entry[104/32] >> (104 % 32)) & 0xff;

    /* sip_data[112 +: 16]        = 16'b0; flow_label;*/
    ipv6_encap_info->tunnel.flow_label = (prge_data_entry[112/32] >> (112 % 32)) & 0xffff;

    return SOC_SAND_OK;
}

STATIC uint32
  arad_pp_eg_encap_ipv6_entry_to_data_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 sip_index,
    SOC_SAND_IN  uint32                                 dip_index,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO       *ipv6_encap_info,
    SOC_SAND_OUT  SOC_PPC_EG_ENCAP_DATA_INFO            *data_info
  )
{
    uint32
        entry_lsb = ARAD_PRGE_DATA_ENTRY_LSBS_IPV6_TUNNEL;
    /* eedb_data = {sip[31:0], sip_ptr, dip_ptr, 8'h3};*/

    /* lsb */
    SHR_BITCOPY_RANGE(data_info->data_entry,0,&entry_lsb,0,8);

    /* dip-ptr */
    SHR_BITCOPY_RANGE(data_info->data_entry,8,&dip_index,0,8);

    /* sip-ptr */
    SHR_BITCOPY_RANGE(data_info->data_entry,16,&sip_index,0,8);

    /* sip-lsb */
    SHR_BITCOPY_RANGE(data_info->data_entry,24,ipv6_encap_info->tunnel.src.address,0,32);

    data_info->oam_lif_set = 0;
    data_info->drop = 0;

    return SOC_SAND_OK;
}

STATIC uint32
  arad_pp_eg_encap_ipv6_entry_from_data_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT  uint32                                 *sip_index,
    SOC_SAND_OUT  uint32                                 *dip_index,
    SOC_SAND_OUT  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO       *ipv6_encap_info,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO               *data_info
  )
{
    uint32 dip_index_lcl[1] = {0};
    uint32 sip_index_lcl[1] = {0};

    /* eedb_data = {sip[31:0], sip_ptr, dip_ptr, 8'h3};*/
    
    /* dip-ptr */
    SHR_BITCOPY_RANGE(dip_index_lcl,0,data_info->data_entry,8,8);
    *dip_index = dip_index_lcl[0];

    /* sip-ptr */
    SHR_BITCOPY_RANGE(sip_index_lcl, 0, data_info->data_entry,16,8);
    *sip_index = sip_index_lcl[0];

    /* sip-lsb */
    SHR_BITCOPY_RANGE(ipv6_encap_info->tunnel.src.address, 0, data_info->data_entry,24,32);

    return SOC_SAND_OK;
}

/* construct 128b ERSPAN prge_data_entry from info in mirror_encap_info */
STATIC uint32
  arad_pp_eg_encap_mirror_erspan_data_to_prge_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO    *mirror_encap_info,
    SOC_SAND_OUT soc_reg_above_64_val_t                 prge_data_entry
  )
{
    soc_reg_above_64_val_t erspan_hdr;
    uint32 eth_hdr_size;

    soc_sand_os_memset(prge_data_entry, 0x0, sizeof(soc_reg_above_64_val_t));

    SOC_REG_ABOVE_64_CLEAR(erspan_hdr);
    
    /* construct ERSPAN header */
    ARAD_PP_EG_ENCAP_PRGE_MEM_ERSPAN_FORMAT_SET(unit, SOC_PPC_EG_ENCAP_ERSPAN_PRIO_VAL, SOC_PPC_EG_ENCAP_ERSPAN_TRUNC_VAL, mirror_encap_info->tunnel.erspan_id, erspan_hdr);

    /* tpid = 0 indicates there is no VLAN TAG */
    if(0 == mirror_encap_info->tunnel.tpid) 
    {
        eth_hdr_size = 14; /* SA(6) + DA(6) + ETH(2) */
    }
    else /* VLAN TAG exists */
    {
        eth_hdr_size = 18; /* SA(6) + DA(6) + VTAG(4) + ETH(2) */
    }

    /* prge_data = {EtherType (2bytes), {40{1'b0}}, EthHdrSize (1byte), ERSPAN(8bytes)} ;*/

    /* prge_data[0   +: 64]        = erspan_hdr 64'b; */
    SHR_BITCOPY_RANGE(prge_data_entry, 0, erspan_hdr, 0, 64);

    /* prge_data[64  +:  8]        = eth_hdr_size 8'b;*/
    prge_data_entry[64/32] |= eth_hdr_size << (64 % 32);

    /* prge_data[72  +: 40]        = 40'b0;*/
    SHR_BITCLR_RANGE(prge_data_entry, 72, 40);

    /* prge_data[112 +: 16]        = ether_type 16'b0;*/
    prge_data_entry[112/32] |= ((uint32)mirror_encap_info->tunnel.ether_type) << (112 % 32);


   return SOC_SAND_OK;
}

/* get ERSPAN info from 128b ERSPAN prge_data_entry into mirror_encap_info  */
STATIC uint32
  arad_pp_eg_encap_mirror_erspan_entry_from_prge_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO    *mirror_encap_info,
    SOC_SAND_IN  soc_reg_above_64_val_t                 prge_data_entry
  )
{
    soc_reg_above_64_val_t erspan_hdr;

    SOC_REG_ABOVE_64_CLEAR(erspan_hdr);

    /* prge_data = {EtherType (2bytes), {48{1'b0}}, ERSPAN(8bytes)} ;*/

    /* prge_data[0   +: 64]        = erspan_hdr 64'b; */
    SHR_BITCOPY_RANGE(erspan_hdr, 0, prge_data_entry, 0, 64);
    ARAD_PP_EG_ENCAP_PRGE_MEM_ERSPAN_FORMAT_SPAN_ID_GET(mirror_encap_info->tunnel.erspan_id, erspan_hdr);

    /* prge_data[64  +:  8]        = eth_hdr_size 8'b; */

    /* prge_data[72  +: 40]        = 40'b0;*/

    /* prge_data[112 +: 16]        = ether_type 16'b0; */
    mirror_encap_info->tunnel.ether_type = (prge_data_entry[112/32] >> (112 % 32)) & 0xffff;

    return SOC_SAND_OK;
}

/* construct 128b L2 prge_data_entry from info in mirror_encap_info */
STATIC uint32
  arad_pp_eg_encap_mirror_l2_data_to_prge_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO    *mirror_encap_info,
    SOC_SAND_OUT soc_reg_above_64_val_t                 prge_data_entry
  )
{
    uint32 res = SOC_SAND_OK;
    uint32 mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S] = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    soc_sand_os_memset(prge_data_entry, 0x0, sizeof(soc_reg_above_64_val_t));

    /* prge_data = {DA(6bytes), SA(6bytes), TAG(4bytes) or 32`b0} ;*/
 
    /* tpid = 0 indicates there is no VLAN TAG */
    if(0 == mirror_encap_info->tunnel.tpid) 
    {
        /* prge_data[0   +: 32]        = 32'b0; */
        prge_data_entry[ 0/32] |= 0;
    }
    else /* setting VLAN TAG */
    {
        /* prge_data[0   +: 12]        = vid 12'b;*/
        prge_data_entry[ 0/32] |= ((uint32)mirror_encap_info->tunnel.vid)  << (0 % 32);

        /* prge_data[12  +:  1]        = dei 1'b0; */
        prge_data_entry[12/32] |= 0 << (12 % 32);

        /* prge_data[13  +:  3]        = pcp 3'b; */
        prge_data_entry[13/32] |= ((uint32)mirror_encap_info->tunnel.pcp)  << (13 % 32);

        /* prge_data[16  +: 16]        = tpid 16'b; */
        prge_data_entry[16/32] |= ((uint32)mirror_encap_info->tunnel.tpid) << (16 % 32);

    }

    /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
    /* coverity[overrun-buffer-val : FALSE] */   
    res = soc_sand_pp_mac_address_struct_to_long(&(mirror_encap_info->tunnel.src), mac_in_longs);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* prge_data[32  +: 48]        = src mac 48'b; */
    SHR_BITCOPY_RANGE(prge_data_entry, 32, mac_in_longs, 0, 48);

    /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
    /* coverity[overrun-buffer-val : FALSE] */   
    res = soc_sand_pp_mac_address_struct_to_long(&(mirror_encap_info->tunnel.dest), mac_in_longs);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* prge_data[80  +: 48]        = dest mac 48'b; */
    SHR_BITCOPY_RANGE(prge_data_entry, 80, mac_in_longs, 0, 48);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mirror_l2_data_to_prge_buffer()", 0, 0);
}


/* get L2 info from 128b L2 prge_data_entry into mirror_encap_info  */
STATIC uint32
  arad_pp_eg_encap_mirror_l2_entry_from_prge_buffer(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO    *mirror_encap_info,
    SOC_SAND_IN  soc_reg_above_64_val_t                 prge_data_entry
  )
{
    uint32 res = SOC_SAND_OK;
    uint32 mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S] = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* prge_data = {DA(6bytes), SA(6bytes), TAG(4bytes) or 32`b0} ;*/
 
    /* prge_data[0   +: 12]        = vid 12'b;*/
    mirror_encap_info->tunnel.vid  = (prge_data_entry[ 0/32] >> ( 0 % 32)) & 0xfff;

    /* prge_data[12  +:  1]        = dei 1'b0; */

    /* prge_data[13  +:  3]        = pcp 3'b; */
    mirror_encap_info->tunnel.pcp  = (prge_data_entry[13/32] >> (13 % 32)) & 0x7;

    /* prge_data[16  +: 32]        = tpid 16'b; */
    mirror_encap_info->tunnel.tpid = (prge_data_entry[16/32] >> (16 % 32)) & 0xffff;

    /* prge_data[32  +: 48]        = src mac 48'b; */
    SHR_BITCOPY_RANGE(mac_in_longs, 0, prge_data_entry, 32, 48);

    /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
    /* coverity[overrun-buffer-val : FALSE] */   
    res = soc_sand_pp_mac_address_long_to_struct(mac_in_longs, &(mirror_encap_info->tunnel.src));
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* prge_data[80  +: 48]        = dest mac 48'b; */
    SHR_BITCOPY_RANGE(mac_in_longs, 0, prge_data_entry, 80, 48);

    /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
    /* coverity[overrun-buffer-val : FALSE] */   
    res = soc_sand_pp_mac_address_long_to_struct(mac_in_longs, &(mirror_encap_info->tunnel.dest));
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mirror_l2_entry_from_prge_buffer()", 0, 0);
 }

/* build prge data buffer for overlay arp
 * prge-data(128b):(if 1tag)   MyMAC[99:52] TPID[51:36] PCP-DEI[35:32] VID[31:20] ETHERTYPE[19:4] SIZE-OF-HEADER-WITHOUT-DA[3:0]
 * prge-data(128b): (untagged) MyMAC[67:20]                                       ETHERTYPE[19:4] SIZE-OF-HEADER-WITHOUT-DA[3:0] */
STATIC uint32
arad_pp_eg_encap_overlay_arp_data_to_prge_buffer(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *overlay_arp_info, 
    SOC_SAND_OUT soc_reg_above_64_val_t                 prge_data_entry
   ) 
{
    uint32 size_of_header_without_da;  /* in byte */
    uint32 is_ethernet_1_tag;

    uint32 start_index;
    uint32 field_length;
    uint32 field_value;

    uint32 field_mac[2];


    /* clear output */
    soc_sand_os_memset(prge_data_entry, 0x0, sizeof(soc_reg_above_64_val_t));

    start_index = 0;

    /* check if ethernet is untagged or 1 tag */
    is_ethernet_1_tag = (overlay_arp_info->outer_tpid != 0);

    /* size of header without da (4b) */
    if (is_ethernet_1_tag) {
        size_of_header_without_da = 12; /*  SA(6) + VLAN(4) + ETH(2) */
    } else {
        size_of_header_without_da = 8; /*  SA(6) + ETH(2) */
    }
    field_length = 4;
    field_value = size_of_header_without_da;
    SHR_BITCOPY_RANGE(prge_data_entry, start_index, &field_value, 0, field_length);

    start_index += field_length;

    /* ethertype (16b) */
    field_length = 16;
    field_value = overlay_arp_info->ether_type;
    SHR_BITCOPY_RANGE(prge_data_entry, start_index, &field_value, 0, field_length);

    start_index += field_length;

    if (is_ethernet_1_tag) {
        /* vid (12b)  */
        field_length = 12;
        field_value = overlay_arp_info->out_vid;
        SHR_BITCOPY_RANGE(prge_data_entry, start_index, &field_value, 0, field_length);

        start_index += field_length;

        /* PCP-DEI (4b) */
        field_length = 4;
        field_value = overlay_arp_info->pcp_dei;
        SHR_BITCOPY_RANGE(prge_data_entry, start_index, &field_value, 0, field_length);

        start_index += field_length;

        /* TPID (16b) */
        field_length = 16;
        field_value = overlay_arp_info->outer_tpid;
        SHR_BITCOPY_RANGE(prge_data_entry, start_index, &field_value, 0, field_length);

        start_index += field_length;
    }

    /* My-MAC (48b) */
    field_length = 48;


    soc_sand_pp_mac_address_struct_to_long(&overlay_arp_info->src_mac, field_mac);


    SHR_BITCOPY_RANGE(prge_data_entry, start_index, field_mac, 0, field_length);

    return SOC_SAND_OK;
}


/* Fill prge ovrelay arp info from prge buffer 
 * prge-data(128b):(if 1tag)   MyMAC[99:52] TPID[51:36] PCP-DEI[35:32] VID[31:20] ETHERTYPE[19:4] SIZE-OF-HEADER-WITHOUT-DA[3:0]
 * prge-data(128b): (untagged) MyMAC[67:20]                                       ETHERTYPE[19:4] SIZE-OF-HEADER-WITHOUT-DA[3:0] */
STATIC uint32
arad_pp_eg_encap_overlay_arp_data_from_prge_buffer(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *overlay_arp_info, 
    SOC_SAND_IN soc_reg_above_64_val_t                 prge_data_entry
   ) 
{
    uint32 field_mac[2];

    uint32 size_of_header_without_da = 0;  /* in byte */
    uint32 is_ethernet_1_tag = 0;

    uint32 start_index = 0;
    uint32 field_length = 0;
    uint32 field_value = 0; 
    

    start_index = 0;

    /* get size of header without da (4b) */
    field_length = 4;
    SHR_BITCOPY_RANGE(&size_of_header_without_da, 0, prge_data_entry, start_index, field_length);

    start_index += field_length;

    /*  check if ethernet is untagged or 1 tag */
    is_ethernet_1_tag = (size_of_header_without_da == 12); /*  SA(6) + VLAN(4) + ETH(2) */

    /* ethertype (16b) */
    field_length = 16;
    SHR_BITCOPY_RANGE(&field_value, 0, prge_data_entry, start_index, field_length);
    overlay_arp_info->ether_type = (uint16) field_value; 
    start_index += field_length;


    if (is_ethernet_1_tag) {
        /* vid (12b)  */
        field_length = 12;
        SHR_BITCOPY_RANGE(&overlay_arp_info->out_vid, 0, prge_data_entry, start_index, field_length);

        start_index += field_length;

        /* PCP-DEI (4b) */
        field_value = 0;
        field_length = 4;
        SHR_BITCOPY_RANGE(&field_value, 0, prge_data_entry, start_index, field_length);
        overlay_arp_info->pcp_dei = (uint8) field_value; 

        start_index += field_length;

        /* TPID (16b) */
        field_value = 0;
        field_length = 16;
        SHR_BITCOPY_RANGE(&field_value, 0, prge_data_entry, start_index, field_length);
        overlay_arp_info->outer_tpid = (uint16) field_value;

        start_index += field_length;
    }

    /* My-MAC (48b) */
    field_length = 48;
    SHR_BITCOPY_RANGE(field_mac, 0, prge_data_entry, start_index, field_length);
    soc_sand_pp_mac_address_long_to_struct(field_mac, &overlay_arp_info->src_mac);

    return SOC_SAND_OK;
}




/* Build data entry for overlay arp eedb entry 
 * data_entry format: DA[55:8] LL-VSI-Pointer[7:4] Reserved[3:2] entry_type[1:0]
 * ll-vsi-pointer: from 0 to 15. Prge data index in reserved range in prge data. 
 */ 
STATIC uint32
arad_pp_eg_encap_overlay_arp_data_to_data_entry_buffer(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *overlay_arp_info, 
    SOC_SAND_OUT  SOC_PPC_EG_ENCAP_DATA_INFO            *data_info
    ) 
{
    uint32 field_value; 
    uint32 field_mac[2];

    /* entry type (2b) */
    field_value = ARAD_PRGE_DATA_ENTRY_LSBS_ROO_VXLAN;
    SHR_BITCOPY_RANGE(data_info->data_entry, 0, &field_value, 0, 2);
    /* reserved (2b) */

    /* ll-vsi-pointer (4b) */
    field_value = overlay_arp_info->ll_vsi_pointer;
    SHR_BITCOPY_RANGE(data_info->data_entry, 4, &field_value, 0, 4);

    /* da (48b) */
    soc_sand_pp_mac_address_struct_to_long(&overlay_arp_info->dest_mac, field_mac);

    SHR_BITCOPY_RANGE(data_info->data_entry, 8, field_mac, 0, 48);

    return SOC_SAND_OK;

}

/* 
 * Fill overlay arp info according to data entry
 * data_entry format: DA[55:8] LL-VSI-Pointer[7:4] Reserved[3:2] entry_type[1:0]
 */ 
static uint32 
arad_pp_eg_encap_overlay_arp_data_from_data_entry_buffer(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *overlay_arp_info, 
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO              *data_info
   ) 
{
    uint32 field_mac[2];
    uint32 ll_vsi_pointer_lcl[1] = {0};

    /* ll-vsi-pointer (4b) */
    SHR_BITCOPY_RANGE(ll_vsi_pointer_lcl, 0, data_info->data_entry, 4, 4);
    overlay_arp_info->ll_vsi_pointer = (uint8) ll_vsi_pointer_lcl[0];

    /* da (48b) */
    SHR_BITCOPY_RANGE(field_mac, 0, data_info->data_entry, 8, 48);
    soc_sand_pp_mac_address_long_to_struct(field_mac, &overlay_arp_info->dest_mac);

    return SOC_SAND_OK;
}



/*
 * given data entry, 
 * returns the usage/exact entry type 
 * inpute: entry-data 
 * output: entry-type 
 * assumnig: given data is indeed of entry type/ 
 */
STATIC SOC_PPC_EG_ENCAP_ENTRY_TYPE
  arad_pp_eg_encap_data_entry_exact_type_get_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO          *tbl_data
  )
{
    int data_entry_lsbs = tbl_data->data_entry[0] & 0x3; 
    switch (data_entry_lsbs) 
    {
    case ARAD_PRGE_DATA_ENTRY_LSBS_IPV6_TUNNEL:
        return SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV6_ENCAP;
        break;
    case ARAD_PRGE_DATA_ENTRY_LSBS_ROO_VXLAN: 
        return SOC_PPC_EG_ENCAP_ENTRY_TYPE_OVERLAY_ARP_ENCAP; 
        break;
    default: 
        return SOC_PPC_EG_ENCAP_ENTRY_TYPE_DATA;
        break;
    }
}


  
/*
 * given data entry, 
 * returns ipv6 info
 * or returns overlay arp info from prge data
 * assumnig: given data is indeed of entry type of ipv6 format 
 */
STATIC uint32
  arad_pp_eg_encap_data_entry_data_exact_info_get_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO                  *data_info,
    SOC_SAND_OUT  SOC_PPC_EG_ENCAP_ENTRY_INFO                *exact_data_info
  )
{
    uint32
      res = SOC_SAND_OK;
    SOC_PPC_EG_ENCAP_ENTRY_TYPE encap_entry_type; 

    uint32 sip_index=0;
    uint32 dip_index=0;
    ARAD_PP_EPNI_PRGE_DATA_TBL_DATA prge_data_sip, prge_data_dip;
    SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO ipv6_encap_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);


    encap_entry_type = arad_pp_eg_encap_data_entry_exact_type_get_unsafe(unit,data_info);

    switch (encap_entry_type) 
    { 
    case (SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV6_ENCAP): 
        SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO_clear(&ipv6_encap_info);

        /* get data entry */
        res = arad_pp_eg_encap_ipv6_entry_from_data_buffer(unit,&sip_index,&dip_index,&ipv6_encap_info,data_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        /* get SIP entry */
        res = arad_pp_epni_prge_data_tbl_get_unsafe(unit,sip_index,&prge_data_sip);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        /* get DIP entry */
        res = arad_pp_epni_prge_data_tbl_get_unsafe(unit,dip_index,&prge_data_dip);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        /* parse SIP*/
        arad_pp_eg_encap_ipv6_sip_entry_from_prge_buffer(unit,&ipv6_encap_info,prge_data_sip.prge_data_entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

        /* parse DIP, copy address it from PRGE into ipv6 info  */
        soc_sand_os_memcpy(ipv6_encap_info.tunnel.dest.address,
                           prge_data_dip.prge_data_entry,
                           SOC_SAND_PP_IPV6_ADDRESS_NOF_UINT32S * sizeof(uint32));
                                      
        exact_data_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV6_ENCAP;
        soc_sand_os_memcpy(&exact_data_info->entry_val.ipv6_encap_info,&ipv6_encap_info,sizeof(SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO));

        break;

    default: 
        exact_data_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_DATA;
        break;
    }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_entry_data_exact_info_get_unsafe()", 0, 0);
}






/*
 *  given pointer to EEDB entry.
 *  if entry type is IPv6 returns pointers to PORGE data.
 *  if entry type is Data but not IPv6 return found - 0.
 *  if entry type is not DATA returns ERROR.
 */

uint32
  arad_pp_eg_encap_ipv6_entry_get_references(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_OUT  uint32                                 *sip_index,
    SOC_SAND_OUT  uint32                                 *dip_index,
    SOC_SAND_OUT  uint8                                  *found
  )
{
    uint32
      res = SOC_SAND_OK;
    SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO       
        ipv6_encap_info;
    SOC_PPC_EG_ENCAP_ENTRY_INFO
        encap_entry_info[1];
    uint8
        next_eep_valid;
    uint32
        next_eep[1];

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* get data entry */
    res = arad_pp_eg_encap_data_entry_get_unsafe(
            unit,
            tunnel_eep_ndx,
            encap_entry_info,
            &next_eep_valid, /* Dummy pointer */
            next_eep
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* arrive here then this is indeed data entry */

    /* check if it's IPv6 entry*/
    if(arad_pp_eg_encap_data_entry_exact_type_get_unsafe(unit,&(encap_entry_info->entry_val.data_info)) == SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV6_ENCAP) {

        /* this is IPv6 entry */
        *found  = 1;

        SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO_clear(&ipv6_encap_info);       

        /* parse data entry to SIP and DIP indexes*/
        res = arad_pp_eg_encap_ipv6_entry_from_data_buffer(unit,sip_index,dip_index,&ipv6_encap_info,&encap_entry_info[0].entry_val.data_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }
    else{
        *found  = 0;
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv6_entry_get_references()", tunnel_eep_ndx, 0);
}

/*********************************************************************
*     Add IPV6 tunnels encapsulation entry to the Egress
 *     Encapsulation Tunnels Editing Table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv6_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO        *ipv6_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep 
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_PRGE_DATA_TBL_DATA
      prge_data_dip,
      prge_data_sip;
  SOC_SAND_SUCCESS_FAILURE
      add_dip_success,
      add_sip_success;
  uint32
      dip_index,sip_index,
      old_dip_index,old_sip_index;
  uint8
      dip_first_appear,
      sip_first_appear,
      dip_last_appear,
      sip_last_appear,
      found;
  SOC_PPC_EG_ENCAP_DATA_INFO
      data_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV6_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(ipv6_encap_info);

  soc_sand_os_memset(&prge_data_dip, 0x0, sizeof(prge_data_dip));
  soc_sand_os_memset(&prge_data_sip, 0x0, sizeof(prge_data_sip));

  SOC_PPC_EG_ENCAP_DATA_INFO_clear(&data_info);

  /* template management */
  /* below profile management add before remove, for code simplicity, is not full optimize, as it can fail
     while 1 profile can be resued, but is ok as there are 256 profiles.
   */
  /* get old profile used if any */
  res = arad_pp_eg_encap_ipv6_entry_get_references(unit,tunnel_eep_ndx,&old_sip_index,&old_dip_index,&found);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* allocate two profiles */
  /* build DIP buffer */
  soc_sand_os_memcpy(&prge_data_dip.prge_data_entry, ipv6_encap_info->tunnel.dest.address, sizeof(uint32) * SOC_SAND_PP_IPV6_ADDRESS_NOF_UINT32S);

  /* allocate place for DIP */
  res = arad_sw_db_multiset_add(
        unit,
		ARAD_SW_DB_CORE_ANY,
        ARAD_PP_SW_DB_MULTI_SET_EG_ENCAP_PROG_DATA_ENTRY,
        prge_data_dip.prge_data_entry,
        &dip_index,
        &dip_first_appear,
        &add_dip_success
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  if(add_dip_success != SOC_SAND_SUCCESS) {
      goto exit;
  }

  /* build SIP buffer */
  res = arad_pp_eg_encap_ipv6_sip_entry_to_prge_buffer(unit,ipv6_encap_info,prge_data_sip.prge_data_entry);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  /* allocate place for SIP */
  res = arad_sw_db_multiset_add(
        unit,
		ARAD_SW_DB_CORE_ANY,
        ARAD_PP_SW_DB_MULTI_SET_EG_ENCAP_PROG_DATA_ENTRY,
        prge_data_sip.prge_data_entry,
        &sip_index,
        &sip_first_appear,
        &add_sip_success
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  if(add_sip_success != SOC_SAND_SUCCESS) {

      /* free above DIP profile */
      res = arad_sw_db_multiset_remove_by_index(
            unit,
            ARAD_SW_DB_CORE_ANY,
            ARAD_PP_SW_DB_MULTI_SET_EG_ENCAP_PROG_DATA_ENTRY,
            dip_index,
            &dip_last_appear
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
      goto exit;
  }

  /* if we get here, addition success */

  /* remove old profiles if exist*/
  if(found) {
      ARAD_PP_EPNI_PRGE_DATA_TBL_DATA prge_reset_data;
      soc_sand_os_memset(&prge_reset_data, 0x0, sizeof(prge_reset_data));

      res = arad_sw_db_multiset_remove_by_index(
            unit,
            ARAD_SW_DB_CORE_ANY,
            ARAD_PP_SW_DB_MULTI_SET_EG_ENCAP_PROG_DATA_ENTRY,
            old_dip_index,
            &dip_last_appear
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

      if (dip_last_appear) {
          SOC_REG_ABOVE_64_CLEAR(&prge_reset_data.prge_data_entry);
          res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, old_dip_index, &prge_reset_data);
          SOC_SAND_CHECK_FUNC_RESULT(res, 81, exit);
      }

      res = arad_sw_db_multiset_remove_by_index(
            unit,
            ARAD_SW_DB_CORE_ANY,
            ARAD_PP_SW_DB_MULTI_SET_EG_ENCAP_PROG_DATA_ENTRY,
            old_sip_index,
            &sip_last_appear
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

      if (sip_last_appear) {
          SOC_REG_ABOVE_64_CLEAR(&prge_reset_data.prge_data_entry);
          res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, old_sip_index, &prge_reset_data);
          SOC_SAND_CHECK_FUNC_RESULT(res, 91, exit);
      }
  }

  /* update HW tables */

  /* write to PRGE data if needed */
  if(dip_first_appear) {
      /* set prge data DIP */
      res = arad_pp_epni_prge_data_tbl_set_unsafe(unit,dip_index,&prge_data_dip);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
  }

  if(sip_first_appear) {
      /* set prge data SIP  */
      res = arad_pp_epni_prge_data_tbl_set_unsafe(unit,sip_index,&prge_data_sip);
      SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
  }
  
  /* write to data entry */
  res = arad_pp_eg_encap_ipv6_entry_to_data_buffer(unit,sip_index,dip_index,ipv6_encap_info,&data_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);

  res = arad_pp_eg_encap_data_lif_entry_add_unsafe(unit,tunnel_eep_ndx,&data_info,ll_eep!=0,ll_eep);
  SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv6_entry_add_unsafe()", tunnel_eep_ndx, 0);
}

uint32
  arad_pp_eg_encap_ipv6_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tunnel_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO            *ipv6_encap_info,
    SOC_SAND_IN  uint32                                  ll_eep
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV6_ENTRY_ADD_VERIFY);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, tunnel_eep_ndx, ARAD_PP_EG_ENCAP_IP_TUNNEL_EEP_NDX_OUT_OF_RANGE_ERR, 5, exit);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO, ipv6_encap_info, 10, exit);

  ARAD_PP_EG_ENCAP_NDX_OF_TYPE_VERIFY(tunnel_eep_ndx);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, ll_eep, ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR, 20, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv6_entry_add_verify()", tunnel_eep_ndx, ll_eep);
}


/*********************************************************************
 *     Add Mirror ERSPAN encapsulation 2 entries (L2 & ERSPAN headers) to prge memory.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_mirror_entry_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              mirror_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *mirror_encap_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_PRGE_DATA_TBL_DATA
      prge_data_erspan,
      prge_data_l2;
  uint32 prge_tbl_mirror_base;
  ARAD_PP_EPNI_ISID_TABLE_TBL_DATA
      isid_tbl_data;


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_MIRROR_ENTRY_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(mirror_encap_info);


  /* build erspan entry */
  res = arad_pp_eg_encap_mirror_erspan_data_to_prge_buffer(unit, mirror_encap_info, prge_data_erspan.prge_data_entry);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* build l2 entry */
  res = arad_pp_eg_encap_mirror_l2_data_to_prge_buffer(unit, mirror_encap_info, prge_data_l2.prge_data_entry);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* get the offset */
  res = arad_sw_db_eg_encap_prge_tbl_nof_dynamic_entries_get(unit, &prge_tbl_mirror_base);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /* set prge data erspan */
  res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, prge_tbl_mirror_base + mirror_ndx, &prge_data_erspan);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /* set prge data l2 */
  res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, prge_tbl_mirror_base + mirror_ndx + ARAD_PP_EG_ENCAP_NOF_MIRRORS, &prge_data_l2);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  /* ERSPAN multicast we allocate the ISID table 16 first entries */
  isid_tbl_data.isid = mirror_encap_info->tunnel.encap_id;
  res = arad_pp_epni_isid_table_tbl_set_unsafe(unit, mirror_encap_info->tunnel.encap_id, &isid_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mirror_entry_set_unsafe()", mirror_ndx, 0);
}


uint32
  arad_pp_eg_encap_mirror_entry_set_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              mirror_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *mirror_encap_info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_MIRROR_ENTRY_SET_VERIFY);


  SOC_SAND_ERR_IF_ABOVE_MAX(mirror_ndx, DPP_MIRROR_ACTION_NDX_MAX, ARAD_PP_EG_ENCAP_MIRROR_ID_OUT_OF_RANGE_ERR, 5, exit);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO, mirror_encap_info, 20, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mirror_entry_set_verify()", mirror_ndx, 0);
}



/*********************************************************************
 *     Get Mirror ERSPAN encapsulation 2 entries (L2 & ERSPAN headers) data from prge memory.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_mirror_entry_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              mirror_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *mirror_encap_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_PRGE_DATA_TBL_DATA
      prge_data_erspan,
      prge_data_l2;
  uint32 prge_tbl_mirror_base;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_MIRROR_ENTRY_GET_UNSAFE);

  /* get the offset */
  res = arad_sw_db_eg_encap_prge_tbl_nof_dynamic_entries_get(unit, &prge_tbl_mirror_base);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* get prge data erspan */
  res = arad_pp_epni_prge_data_tbl_get_unsafe(unit, prge_tbl_mirror_base + mirror_ndx, &prge_data_erspan);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* get prge data l2 */
  res = arad_pp_epni_prge_data_tbl_get_unsafe(unit, prge_tbl_mirror_base + mirror_ndx + ARAD_PP_EG_ENCAP_NOF_MIRRORS, &prge_data_l2);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /* get erspan entry */
  res = arad_pp_eg_encap_mirror_erspan_entry_from_prge_buffer(unit, mirror_encap_info, prge_data_erspan.prge_data_entry);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /* get l2 entry */
  res = arad_pp_eg_encap_mirror_l2_entry_from_prge_buffer(unit, mirror_encap_info, prge_data_l2.prge_data_entry);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mirror_entry_get_unsafe()", mirror_ndx, 0);
}


/*********************************************************************
*     Set whether ERSPAN is disabled/enabled per port.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_port_erspan_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_IN  uint8                                  is_erspan
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_PP_PCT_TBL_DATA
    pp_pct_tbl_data;
  int set_table = 0;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PORT_ERSPAN_SET_UNSAFE);


  res = arad_pp_epni_pp_pct_tbl_get_unsafe(unit, core_id, local_port_ndx, &pp_pct_tbl_data);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit); 

                
  if(1 == is_erspan) {
      /* read table content: if (SPAN) -> RSPAN, if (XGS SPAN) -> XGS RSPAN*/
      if(ARAD_PRGE_PP_SELECT_PP_COPY_HEADER_SPAN == pp_pct_tbl_data.prge_profile) {
          pp_pct_tbl_data.prge_profile = ARAD_PRGE_PP_SELECT_PP_COPY_HEADER_ERSPAN;
          set_table = 1;
      }
      else if(ARAD_PRGE_PP_SELECT_XGS_PE_FROM_FTMH_SPAN == pp_pct_tbl_data.prge_profile) {
          pp_pct_tbl_data.prge_profile = ARAD_PRGE_PP_SELECT_XGS_PE_FROM_FTMH_ERSPAN;
          set_table = 1;
      }
  }
  else {
      /* read table content: if (RSPAN) -> SPAN, if (XGS RSPAN) -> XGS SPAN*/
      if(ARAD_PRGE_PP_SELECT_PP_COPY_HEADER_ERSPAN == pp_pct_tbl_data.prge_profile) {
          pp_pct_tbl_data.prge_profile = ARAD_PRGE_PP_SELECT_PP_COPY_HEADER_SPAN;
          set_table = 1;
      }
      else if(ARAD_PRGE_PP_SELECT_XGS_PE_FROM_FTMH_ERSPAN == pp_pct_tbl_data.prge_profile) {
          pp_pct_tbl_data.prge_profile = ARAD_PRGE_PP_SELECT_XGS_PE_FROM_FTMH_SPAN;
          set_table = 1;
      }
  }

  if(1 == set_table) {
      res = arad_pp_epni_pp_pct_tbl_set_unsafe(unit, core_id, local_port_ndx, &pp_pct_tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mirror_entry_get_unsafe()", local_port_ndx, is_erspan);
}

/*********************************************************************
*     Get whether ERSPAN is disabled/enabled per port.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_port_erspan_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_OUT uint8                                  *is_erspan
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EPNI_PP_PCT_TBL_DATA
    pp_pct_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PORT_ERSPAN_GET_UNSAFE);

  res = arad_pp_epni_pp_pct_tbl_get_unsafe(unit, core_id, local_port_ndx, &pp_pct_tbl_data);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit); 

  if(ARAD_PRGE_PP_SELECT_PP_COPY_HEADER_ERSPAN   == pp_pct_tbl_data.prge_profile ||
     ARAD_PRGE_PP_SELECT_XGS_PE_FROM_FTMH_ERSPAN == pp_pct_tbl_data.prge_profile) {
      *is_erspan = 1;
  }
  else {
      *is_erspan = 0;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mirror_entry_get_unsafe()", local_port_ndx, 0);
}


/*********************************************************************
*     Add LL encapsulation entry.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ll_entry_add_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                ll_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO                 *ll_encap_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_LINK_LAYER_ENTRY_FORMAT
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_UNSAFE);
  
  SOC_SAND_CHECK_NULL_INPUT(ll_encap_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_struct_to_long(
          &ll_encap_info->dest_mac,
          tbl_data.dest_mac
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  tbl_data.vid = ll_encap_info->out_vid;
  tbl_data.vid_valid = ll_encap_info->out_vid_valid;
  tbl_data.next_outlif_lsb = ll_encap_info->out_ac_lsb;
  tbl_data.next_outlif_valid = ll_encap_info->out_ac_valid;
  tbl_data.remark_profile = ll_encap_info->ll_remark_profile;
  tbl_data.oam_lif_set = SOC_SAND_BOOL2NUM(ll_encap_info->oam_lif_set);
  tbl_data.drop = SOC_SAND_BOOL2NUM(ll_encap_info->drop);
  if (SOC_IS_JERICHO(unit)) {
      tbl_data.outlif_profile = ll_encap_info->outlif_profile; 
  }
  if (SOC_IS_JERICHO_PLUS(unit) && !SOC_IS_QAX_A0(unit)) {
      tbl_data.native_ll = ll_encap_info->native_ll;
  }
  res = arad_pp_eg_encap_access_link_layer_format_tbl_set_unsafe(
          unit,
          ll_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ll_entry_add_unsafe()", ll_eep_ndx, 0);
}


uint32
  arad_pp_eg_encap_ll_entry_add_verify(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                ll_eep_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO                  *ll_encap_info
  )
{
  uint32
    res = SOC_SAND_OK;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_LL_ENTRY_ADD_VERIFY);

  ARAD_PP_EG_ENCAP_NDX_OF_TYPE_VERIFY(ll_eep_ndx);

  res = SOC_PPC_EG_ENCAP_LL_INFO_verify(unit, ll_encap_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, ll_eep_ndx, ARAD_PP_EG_ENCAP_LL_EEP_OUT_OF_RANGE_ERR, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ll_entry_add_verify()", ll_eep_ndx, 0);
}


/*********************************************************************
*     Remove entry from the encapsulation Table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                   eep_type_ndx,
    SOC_SAND_IN  uint32                                  eep_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_PPC_EG_ENCAP_ENTRY_INFO encap_entry_info[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD];
  uint32 next_eep[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD];
  uint32 nof_entries;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_ENTRY_REMOVE_UNSAFE);

  res = arad_pp_eg_encap_entry_get_unsafe(unit, eep_type_ndx, eep_ndx, 1, encap_entry_info, next_eep, &nof_entries);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* in overlay arp eedb entries in arad+, eedb entry contains an index to prge data.
     Remove entry in prge data. */
  if (SOC_IS_ARADPLUS_A0(unit)) {
      if (encap_entry_info[0].entry_type == SOC_PPC_EG_ENCAP_ENTRY_TYPE_OVERLAY_ARP_ENCAP) {
          uint32  prge_data_relative_index;
          uint32 prge_data_base_index;
          uint32 prge_data_absolute_index;
          uint8 prge_data_last_appear; 
          ARAD_PP_EPNI_PRGE_DATA_TBL_DATA prge_data;

          /* get relative index */
          prge_data_relative_index = encap_entry_info[0].entry_val.overlay_arp_encap_info.ll_vsi_pointer;

          /* get base index */
          res  = arad_sw_db_eg_encap_prge_tbl_overlay_arp_entries_base_index_get(unit, &prge_data_base_index);
          SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit); 

          /* absolute prge data index of overlay arp */
          prge_data_absolute_index = prge_data_base_index + prge_data_relative_index;

          /* remove prge entry SW */
          res = arad_sw_db_multiset_remove_by_index(unit, 
                                                    ARAD_SW_DB_CORE_ANY,
                                                    ARAD_PP_SW_DB_MULTI_SET_ENTRY_OVERLAY_ARP_PROG_DATA_ENTRY, 
                                                    prge_data_relative_index ,
                                                    &prge_data_last_appear); 
          SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit); 

          /* remove prge entry HW */
          SOC_REG_ABOVE_64_CLEAR(&prge_data.prge_data_entry);
          res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, prge_data_absolute_index, &prge_data); 
          SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit); 
      }
  } 
  /* in ROO LL eedb entries in jericho,
     eedb entry contains a 1/4 entry and an additional table entry (CfgEtherTypeIndex table) */
  else if (SOC_IS_JERICHO(unit)) {

      if (encap_entry_info[0].entry_type == SOC_PPC_EG_ENCAP_ENTRY_TYPE_ROO_LL_ENCAP) {
        uint64 additional_data;

          /* get additional entry that contain tpids, so we can find the nof tags. */
          SOC_PPC_EG_ENCAP_ETHER_TYPE_INDEX_INFO eth_type_index_entry;
          soc_sand_os_memset(
               &eth_type_index_entry, 0x0, sizeof(SOC_PPC_EG_ENCAP_ETHER_TYPE_INDEX_INFO)); 
          soc_jer_eg_encap_ether_type_index_get(
             unit, encap_entry_info[0].entry_val.overlay_arp_encap_info.eth_type_index, &eth_type_index_entry); 

          COMPILER_64_SET(additional_data,0,0);

            /* nof tags */
            if (eth_type_index_entry.tpid_0 != 0) encap_entry_info[0].entry_val.overlay_arp_encap_info.nof_tags++;
            if (eth_type_index_entry.tpid_1 != 0) encap_entry_info[0].entry_val.overlay_arp_encap_info.nof_tags++;

          /* remove 1/4 entry */
          res = arad_pp_lif_additional_data_set(unit, eep_ndx, 0, additional_data); 
          SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);
      }
  }

  /* Clear PRGE data when removing IPv6 tunnel in arad_plus and above devices */
  if (SOC_IS_ARADPLUS(unit)) {

      if (encap_entry_info[0].entry_type == SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV6_ENCAP) {

          uint8 prge_data_last_appear, found;
          uint32 old_dip_index, old_sip_index;
          ARAD_PP_EPNI_PRGE_DATA_TBL_DATA prge_reset_data;

          soc_sand_os_memset(&prge_reset_data, 0x0, sizeof(prge_reset_data));

          res = arad_pp_eg_encap_ipv6_entry_get_references(unit,eep_ndx,&old_sip_index,&old_dip_index,&found);
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

          if (found) {
              /* remove prge entry SW */
              res = arad_sw_db_multiset_remove_by_index(unit,
                                                        ARAD_SW_DB_CORE_ANY,
                                                        ARAD_PP_SW_DB_MULTI_SET_EG_ENCAP_PROG_DATA_ENTRY,
                                                        old_sip_index,
                                                        &prge_data_last_appear);
              SOC_SAND_CHECK_FUNC_RESULT(res, 31, exit);

              /* remove prge entry HW */
              if (prge_data_last_appear) {
                  SOC_REG_ABOVE_64_CLEAR(&prge_reset_data.prge_data_entry);
                  res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, old_sip_index, &prge_reset_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 32, exit);
              }

              /* remove prge entry SW */
              res = arad_sw_db_multiset_remove_by_index(unit,
                                                        ARAD_SW_DB_CORE_ANY,
                                                        ARAD_PP_SW_DB_MULTI_SET_EG_ENCAP_PROG_DATA_ENTRY,
                                                        old_dip_index,
                                                        &prge_data_last_appear);
              SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);

              /* remove prge entry HW */
              if (prge_data_last_appear) {
                  SOC_REG_ABOVE_64_CLEAR(&prge_reset_data.prge_data_entry);
                  res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, old_dip_index, &prge_reset_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
              }
          }
      }
  }

  res = arad_pp_eg_encap_access_entry_init_unsafe(
          unit,
          eep_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_remove_unsafe()", eep_type_ndx, eep_ndx);
}


uint32
  arad_pp_eg_encap_entry_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                   eep_type_ndx,
    SOC_SAND_IN  uint32                                  eep_ndx
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_ENTRY_REMOVE_VERIFY);
  SOC_SAND_ERR_IF_ABOVE_MAX(eep_type_ndx, SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_MAX, SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, eep_ndx, ARAD_PP_EG_ENCAP_EEP_NDX_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_remove_verify()", 0, eep_ndx);
}

/*********************************************************************
 *     Get LL encapsulation entry.
 *********************************************************************/
STATIC uint32
  arad_pp_eg_encap_ll_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  ll_eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_LINK_LAYER_ENTRY_FORMAT
    tbl_data;
  SOC_PPC_EG_ENCAP_EEP_TYPE
    eep_type_ndx = SOC_PPC_EG_ENCAP_EEP_TYPE_LL;
  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
    cur_eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_LL_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(encap_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  SOC_PPC_EG_ENCAP_ENTRY_INFO_clear(encap_info);

  res = arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, ll_eep_ndx, &cur_eep_type);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  ARAD_PP_EG_ENCAP_VERIFY_EEP_TYPE_COMPATIBLE_TO_ACCESS_TYPE(ll_eep_ndx,eep_type_ndx, cur_eep_type);

  res = arad_pp_eg_encap_access_link_layer_format_tbl_get_unsafe(
          unit,
          ll_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_long_to_struct(
          tbl_data.dest_mac,
          &encap_info->entry_val.ll_info.dest_mac
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_LL_ENCAP;
  encap_info->entry_val.ll_info.out_vid = tbl_data.vid;
  encap_info->entry_val.ll_info.out_vid_valid = SOC_SAND_NUM2BOOL(tbl_data.vid_valid);
  encap_info->entry_val.ll_info.ll_remark_profile = tbl_data.remark_profile;
  encap_info->entry_val.ll_info.oam_lif_set = SOC_SAND_BOOL2NUM(tbl_data.oam_lif_set);
  encap_info->entry_val.ll_info.drop = SOC_SAND_BOOL2NUM(tbl_data.drop);
  encap_info->entry_val.ll_info.out_ac_valid = SOC_SAND_NUM2BOOL(tbl_data.next_outlif_valid);
  if (encap_info->entry_val.ll_info.out_ac_valid)
  {
      encap_info->entry_val.ll_info.out_ac_lsb = tbl_data.next_outlif_lsb;    
  }
  if (SOC_IS_JERICHO(unit)) {
      encap_info->entry_val.ll_info.outlif_profile = tbl_data.outlif_profile; 
  }
  if (SOC_IS_JERICHO_PLUS(unit) && !SOC_IS_QAX_A0(unit)) {
      encap_info->entry_val.ll_info.native_ll = tbl_data.native_ll; 
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ll_entry_get_unsafe()", ll_eep_ndx, 0);
}

  

/*********************************************************************
 *     Get DATA encapsulation entry.
 *********************************************************************/
STATIC uint32
  arad_pp_eg_encap_data_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  data_eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info,
    SOC_SAND_OUT uint8                                   *next_eep_valid, 
    SOC_SAND_OUT uint32                                    *next_eep
  )
{
  uint32
    res = SOC_SAND_OK,
    ind;
  ARAD_PP_EG_ENCAP_ACCESS_DATA_ENTRY_FORMAT
    tbl_data;
  SOC_PPC_EG_ENCAP_EEP_TYPE
    eep_type_ndx = SOC_PPC_EG_ENCAP_EEP_TYPE_DATA;
  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
    cur_eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_DATA_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(encap_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  SOC_PPC_EG_ENCAP_ENTRY_INFO_clear(encap_info);

  arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, data_eep_ndx, &cur_eep_type);
  ARAD_PP_EG_ENCAP_VERIFY_EEP_TYPE_COMPATIBLE_TO_ACCESS_TYPE(data_eep_ndx,eep_type_ndx, cur_eep_type); 

  res = arad_pp_eg_encap_access_data_entry_format_tbl_get_unsafe(
          unit,
          data_eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  for (ind = 0; ind < SOC_PPC_EG_ENCAP_DATA_INFO_MAX_SIZE; ind++) {
    encap_info->entry_val.data_info.data_entry[ind] = tbl_data.data[ind];
  }

  encap_info->entry_val.data_info.oam_lif_set = SOC_SAND_BOOL2NUM(tbl_data.oam_lif_set);
  encap_info->entry_val.data_info.drop = SOC_SAND_BOOL2NUM(tbl_data.drop);

  *next_eep_valid = SOC_SAND_NUM2BOOL(tbl_data.next_outlif_valid);
  if (*next_eep_valid)
  {
      *next_eep = tbl_data.next_outlif;   
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_data_entry_get_unsafe()", data_eep_ndx, 0);
}

/*********************************************************************
 *     Get RIF encapsulation entry.
 *********************************************************************/
STATIC uint32
  arad_pp_eg_encap_vsi_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lif_eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info,
    SOC_SAND_OUT uint8                                   *next_eep_valid, 
    SOC_SAND_OUT uint32                                    *next_eep
  )
{ 
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_ACCESS_OUT_RIF_ENTRY_FORMAT
    tbl_data;
  SOC_PPC_EG_ENCAP_EEP_TYPE
    eep_type_ndx = SOC_PPC_EG_ENCAP_EEP_TYPE_VSI;
  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
    cur_eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_VSI_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(encap_info);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  SOC_PPC_EG_ENCAP_ENTRY_INFO_clear(encap_info);

  arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, lif_eep_ndx, &cur_eep_type);
  ARAD_PP_EG_ENCAP_VERIFY_EEP_TYPE_COMPATIBLE_TO_ACCESS_TYPE(lif_eep_ndx,eep_type_ndx, cur_eep_type); 

  if (SOC_IS_JERICHO_PLUS(unit)) {
      res = qax_pp_eg_encap_access_out_rif_entry_format_tbl_get_unsafe(
              unit,
              lif_eep_ndx,
              &tbl_data
            );
  } else {
      res = arad_pp_eg_encap_access_out_rif_entry_format_tbl_get_unsafe(
              unit,
              lif_eep_ndx,
              &tbl_data
            );
  }
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_VSI;
  encap_info->entry_val.vsi_info.out_vsi = tbl_data.next_vsi_lsb;
  encap_info->entry_val.vsi_info.remark_profile = tbl_data.remark_profile;
  encap_info->entry_val.vsi_info.oam_lif_set = SOC_SAND_BOOL2NUM(tbl_data.oam_lif_set);
  encap_info->entry_val.vsi_info.drop = SOC_SAND_BOOL2NUM(tbl_data.drop);
  if (SOC_IS_JERICHO(unit)) {
      encap_info->entry_val.vsi_info.outlif_profile = tbl_data.outlif_profile;
  }
  if (SOC_IS_JERICHO_PLUS(unit)) {
      encap_info->entry_val.vsi_info.outrif_profile_index = tbl_data.outrif_profile_index;
  }

  *next_eep_valid = SOC_SAND_NUM2BOOL(tbl_data.next_outlif_valid);
  if (*next_eep_valid)
  {
      *next_eep = tbl_data.next_outlif;   
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_vsi_entry_get_unsafe()", lif_eep_ndx, 0);
}

/*********************************************************************
 *     Get LIF encapsulation entry.
 *********************************************************************/
STATIC uint32
  arad_pp_eg_encap_lif_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info,
    SOC_SAND_OUT uint32                                  *next_eep
  )
{
  uint32
    res;
  ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT
    tbl_data;
  SOC_PPC_EG_ENCAP_EEP_TYPE
    eep_type_ndx = SOC_PPC_EG_ENCAP_EEP_TYPE_LIF_EEP;
  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
    cur_eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;
  uint8 is_full_entry_extension = TRUE;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_LIF_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(encap_info);
  SOC_SAND_CHECK_NULL_INPUT(next_eep);

  soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

  arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, eep_ndx, &cur_eep_type);
  ARAD_PP_EG_ENCAP_VERIFY_EEP_TYPE_COMPATIBLE_TO_ACCESS_TYPE(eep_ndx,eep_type_ndx, cur_eep_type); 

  res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_get_unsafe(
          unit,
          eep_ndx,
          &tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (tbl_data.mpls1_command == ARAD_PP_EG_ENCAP_MPLS1_COMMAND_SWAP_val)
  {
    /* SWAP */
    encap_info->entry_val.swap_info.swap_label = tbl_data.mpls1_label;
    encap_info->entry_val.swap_info.oam_lif_set = tbl_data.oam_lif_set;
    encap_info->entry_val.swap_info.drop = tbl_data.drop;
    encap_info->entry_val.swap_info.out_vsi = tbl_data.next_vsi_lsb;
    encap_info->entry_val.swap_info.outlif_profile = SOC_IS_JERICHO(unit) ? tbl_data.outlif_profile : 0;
    encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_SWAP_CMND;
    encap_info->entry_val.swap_info.remark_profile = tbl_data.mpls_swap_remark_profile;
  }
  else if (tbl_data.mpls1_command == ARAD_PP_EG_ENCAP_MPLS1_COMMAND_POP_val)
  {
    /* POP */
    encap_info->entry_val.pop_info.oam_lif_set = tbl_data.oam_lif_set;
    encap_info->entry_val.pop_info.drop = tbl_data.drop;
    encap_info->entry_val.pop_info.pop_type = SOC_PPC_MPLS_COMMAND_TYPE_POP;
    encap_info->entry_val.pop_info.out_vsi = tbl_data.next_vsi_lsb;
    encap_info->entry_val.pop_info.ethernet_info.has_cw = SOC_SAND_NUM2BOOL(tbl_data.has_cw);
    encap_info->entry_val.pop_info.pop_next_header = tbl_data.upper_layer_protocol;
    encap_info->entry_val.pop_info.ethernet_info.tpid_profile = tbl_data.tpid_profile;
    encap_info->entry_val.pop_info.model = (tbl_data.model_is_pipe )? SOC_SAND_PP_MPLS_TUNNEL_MODEL_PIPE : SOC_SAND_PP_MPLS_TUNNEL_MODEL_UNIFORM;
    encap_info->entry_val.pop_info.outlif_profile = SOC_IS_JERICHO(unit) ? tbl_data.outlif_profile : 0;
    encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_POP_CMND;
  }
  else
  {
    /* PWE */
    uint32 default_label_value = 0;
    default_label_value = SOC_IS_JERICHO_B0_AND_ABOVE(unit) ? soc_property_get(unit, spn_MPLS_ENCAP_INVALID_VALUE, 0) : 0;

    /* Push commands in HW implementations is 0 + push_profile */
    encap_info->entry_val.pwe_info.push_profile = tbl_data.mpls1_command - ARAD_PP_EG_ENCAP_MPLS1_COMMAND_PUSH_val;
    encap_info->entry_val.pwe_info.label = tbl_data.mpls1_label;
    encap_info->entry_val.pwe_info.oam_lif_set = tbl_data.oam_lif_set;
    encap_info->entry_val.pwe_info.drop = tbl_data.drop;
    encap_info->entry_val.pwe_info.outlif_profile = SOC_IS_JERICHO(unit) ? tbl_data.outlif_profile : 0;
    /* MPLS information (mpls2) may be configured via bcm_mpls_port_add only if this soc property is set */ 
    if (soc_property_get(unit, spn_MPLS_BIND_PWE_WITH_MPLS_ONE_CALL, 0)) {
        encap_info->entry_val.pwe_info.egress_tunnel_label_info.tunnels[0].tunnel_label = (tbl_data.mpls2_label == default_label_value) ? 0 : tbl_data.mpls2_label;
        encap_info->entry_val.pwe_info.egress_tunnel_label_info.tunnels[0].push_profile = tbl_data.mpls2_command;
        encap_info->entry_val.pwe_info.egress_tunnel_label_info.nof_tunnels = tbl_data.mpls2_label == default_label_value ? 0 : 1;
    }
    encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_PWE;


    /* Retrieve the Protection extension info for Jericho */
    SOC_PPC_EG_ENCAP_PROTECTION_INFO_clear(&(encap_info->entry_val.pwe_info.protection_info));
    if (SOC_IS_JERICHO(unit)) {
        res = arad_pp_eg_encap_protection_info_get_unsafe(
                unit, eep_ndx, is_full_entry_extension, &(encap_info->entry_val.pwe_info.protection_info));
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
  }

  *next_eep = tbl_data.next_outlif;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_tunnel_entry_get_unsafe()", eep_ndx, 0);
}

/*********************************************************************
 *     Get tunnel encapsulation entry.
 *********************************************************************/
STATIC uint32
  arad_pp_eg_encap_tunnel_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info,
    SOC_SAND_OUT uint32                                  *next_eep
  )
{
  uint32
    res = SOC_SAND_OK, default_label_value = 0;
  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
    tbl_data_entry_type;
  ARAD_PP_EG_ENCAP_ACCESS_MPLS_TUNNEL_ENTRY_FORMAT
    ee_entry_tbl_data;
  ARAD_PP_EG_ENCAP_ACCESS_IP_TUNNEL_ENTRY_FORMAT
    ip_tunnel_format_tbl_data;
  uint8 is_full_entry_extension = TRUE;
  uint32 push_command_upper_limit_val = 0;
  uint32 push_profile = 0;


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_TUNNEL_ENTRY_GET_UNSAFE);

  soc_sand_os_memset(&tbl_data_entry_type, 0x0, sizeof(tbl_data_entry_type));
  soc_sand_os_memset(&ip_tunnel_format_tbl_data, 0x0, sizeof(ip_tunnel_format_tbl_data));

  SOC_PPC_EG_ENCAP_ENTRY_INFO_clear(encap_info);

  arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, eep_ndx, &tbl_data_entry_type);

  switch(tbl_data_entry_type)
  {
    case (ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_IP):
        /* IP Tunnel */
        res = arad_pp_eg_encap_access_ip_tunnel_format_tbl_get_unsafe(
                unit,
                eep_ndx,
                &ip_tunnel_format_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        encap_info->entry_val.ipv4_encap_info.dest.dest = ip_tunnel_format_tbl_data.ipv4_dst;
        encap_info->entry_val.ipv4_encap_info.dest.src_index = ip_tunnel_format_tbl_data.ipv4_src_index;
        encap_info->entry_val.ipv4_encap_info.dest.tos_index = (uint8)ip_tunnel_format_tbl_data.ipv4_tos_index;
        encap_info->entry_val.ipv4_encap_info.dest.ttl_index = (uint8)ip_tunnel_format_tbl_data.ipv4_ttl_index;

        if (SOC_IS_JERICHO_B0_AND_ABOVE(unit)&& (SOC_JER_PP_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_ENABLE == 1)) {
            encap_info->entry_val.ipv4_encap_info.dest.encapsulation_mode_index = ip_tunnel_format_tbl_data.encapsulation_mode; 
        } else {
            SOC_PPC_EG_ENCAP_SOC_PPC_EG_ENCAP_ENCAPSULATION_MODE_HW_FLD_TO_ENCAPSULATION_MODE(unit, 
                                                                                              ip_tunnel_format_tbl_data.encapsulation_mode, 
                                                                                              &(encap_info->entry_val.ipv4_encap_info.dest.encapsulation_mode));
        }
        encap_info->entry_val.ipv4_encap_info.out_vsi = ip_tunnel_format_tbl_data.next_vsi_lsb;
        encap_info->entry_val.ipv4_encap_info.oam_lif_set = SOC_SAND_BOOL2NUM(ip_tunnel_format_tbl_data.oam_lif_set);
        encap_info->entry_val.ipv4_encap_info.drop = SOC_SAND_BOOL2NUM(ip_tunnel_format_tbl_data.drop);
        encap_info->entry_val.ipv4_encap_info.outlif_profile = SOC_IS_JERICHO(unit) ? ip_tunnel_format_tbl_data.outlif_profile : 0;

        *next_eep = ip_tunnel_format_tbl_data.next_outlif;

        encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV4_ENCAP;
        break;
   case (ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_TUNNEL):
        /* MPLS tunnel */
		/* In Jericho B0 and above, default label value is configurable */
        default_label_value = SOC_IS_JERICHO_B0_AND_ABOVE(unit) ? soc_property_get(unit, spn_MPLS_ENCAP_INVALID_VALUE, 0) : 0;

        push_command_upper_limit_val = SOC_IS_JERICHO_B0_AND_ABOVE(unit) ? ARAD_PP_EG_ENCAP_MPLS1_COMMAND_PUSH_JER_B0_UPPER_LIMIT_val : ARAD_PP_EG_ENCAP_MPLS1_COMMAND_PUSH_UPPER_LIMIT_val;

        res = arad_pp_eg_encap_access_mpls_tunnel_format_tbl_get_unsafe(
                unit,
                eep_ndx,
                &ee_entry_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_MPLS_ENCAP;
        encap_info->entry_val.mpls_encap_info.out_vsi = ee_entry_tbl_data.next_vsi_lsb;
        encap_info->entry_val.mpls_encap_info.oam_lif_set = SOC_SAND_BOOL2NUM(ee_entry_tbl_data.oam_lif_set);
        encap_info->entry_val.mpls_encap_info.drop = SOC_SAND_BOOL2NUM(ee_entry_tbl_data.drop);
        encap_info->entry_val.mpls_encap_info.outlif_profile = SOC_IS_JERICHO(unit) ? ee_entry_tbl_data.outlif_profile : 0 ;

        *next_eep = ee_entry_tbl_data.next_outlif;

        encap_info->entry_val.mpls_encap_info.nof_tunnels = 0;

        /* 1st encapsulation */
        encap_info->entry_val.mpls_encap_info.tunnels[0].push_profile =
          ee_entry_tbl_data.mpls1_command - ARAD_PP_EG_ENCAP_MPLS1_COMMAND_PUSH_val;
        encap_info->entry_val.mpls_encap_info.tunnels[0].tunnel_label =
          ee_entry_tbl_data.mpls1_label;

        /* 2nd encapsulation */
        encap_info->entry_val.mpls_encap_info.tunnels[1].push_profile =
          ee_entry_tbl_data.mpls2_command - ARAD_PP_EG_ENCAP_MPLS1_COMMAND_PUSH_val;
        encap_info->entry_val.mpls_encap_info.tunnels[1].tunnel_label =
          ee_entry_tbl_data.mpls2_label == default_label_value ? 0 : ee_entry_tbl_data.mpls2_label;

        push_profile = encap_info->entry_val.mpls_encap_info.tunnels[0].push_profile;

        /* Nof tunnels */
        if (push_profile <= push_command_upper_limit_val && push_profile != ARAD_PP_EG_ENCAP_MPLS1_COMMAND_POP_val && \
            push_profile != ARAD_PP_EG_ENCAP_MPLS1_COMMAND_SWAP_val)
        {
          ++encap_info->entry_val.mpls_encap_info.nof_tunnels;
        }

        push_profile = encap_info->entry_val.mpls_encap_info.tunnels[1].push_profile;

        if ((push_profile <= push_command_upper_limit_val && push_profile != ARAD_PP_EG_ENCAP_MPLS1_COMMAND_POP_val \
              && push_profile != ARAD_PP_EG_ENCAP_MPLS1_COMMAND_SWAP_val)  && (ee_entry_tbl_data.mpls2_label != default_label_value))
        {
          ++encap_info->entry_val.mpls_encap_info.nof_tunnels;
        }
        
        /* Retrieve the Protection extension info for Jericho */
        SOC_PPC_EG_ENCAP_PROTECTION_INFO_clear(&(encap_info->entry_val.mpls_encap_info.protection_info));
        if (SOC_IS_JERICHO(unit)) {
            res = arad_pp_eg_encap_protection_info_get_unsafe(
                    unit, eep_ndx, is_full_entry_extension, &(encap_info->entry_val.mpls_encap_info.protection_info));
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
        }

        break;
  case (ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE):
      ARAD_PP_DO_NOTHING_AND_EXIT;
      break;
  default: 
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_ACCESS_ENTRY_TYPE_MISMATCH_ERR, 10, exit);
  }
 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_tunnel_entry_get_unsafe()", eep_ndx, 0);
}

STATIC uint32
  arad_pp_eg_encap_eedb_type_to_entry_type(
    SOC_SAND_IN  int                             unit, 
    SOC_SAND_IN  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE  eep_type,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_TYPE        *entry_type
  )
{
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  switch(eep_type) {
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE:
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_ISID:
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_TRILL:
    *entry_type =  SOC_PPC_EG_ENCAP_ENTRY_TYPE_NULL;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_PWE;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_IPV6:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV6_ENCAP;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_DATA;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_LINK_LAYER:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_LL_ENCAP;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_SWAP:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_SWAP_CMND;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_TUNNEL:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_MPLS_ENCAP;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_IP:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_IPV4_ENCAP;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_RIF:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_VSI;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_AC:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_AC;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_POP:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_POP_CMND;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_ARP_OVERLAY: 
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_OVERLAY_ARP_ENCAP;
    break;
  case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_ROO_LINK_LAYER:
    *entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_ROO_LL_ENCAP; 
    break;
  default:
    SOC_SAND_SET_ERROR_CODE(SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR, 40, exit);
  }
   
  ARAD_DO_NOTHING_AND_EXIT;
 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_type_get_unsafe()", 0, eep_type);
}
  

/*********************************************************************
*     Get entry type from the Egress encapsulation
 *     tables.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_entry_type_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_TYPE            *entry_type
  )
{
  uint32
   res = SOC_SAND_OK;
  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
    eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET_UNSAFE);
 
  res = arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, eep_ndx, &eep_type);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  res = arad_pp_eg_encap_eedb_type_to_entry_type(
          unit, 
          eep_type,
          entry_type
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_type_get_unsafe()", 0, eep_ndx);
}


uint32
  arad_pp_eg_encap_entry_type_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  eep_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_ENTRY_TYPE_GET_VERIFY);

  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, eep_ndx, ARAD_PP_EG_ENCAP_EEP_NDX_OUT_OF_RANGE_ERR, 20, exit);
 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_type_get_verify()", 0, eep_ndx);
}


/*********************************************************************
*     Get entry information from the Egress encapsulation
 *     tables.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                 eep_type_ndx,
    SOC_SAND_IN  uint32                                  eep_ndx,
    SOC_SAND_IN  uint32                                  depth,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO               encap_entry_info[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD],
    SOC_SAND_OUT uint32                                  next_eep[SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD],
    SOC_SAND_OUT uint32                                  *nof_entries
  )
{
  uint32
   res = SOC_SAND_OK,
   depth_i,
   cur_eep_ndx;
  ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
    cur_eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;
  uint8
    ll_out_ac_valid;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(nof_entries);
 
  SOC_SAND_ERR_IF_ABOVE_MAX(depth, SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD - 1, ARAD_PP_EG_ENCAP_DEPTH_OUT_OF_RANGE_ERR, 5, exit);

  /* Eep entry to get on current iteration */
  cur_eep_ndx = eep_ndx;
  *nof_entries = 0; 

  for (depth_i = 0; depth_i <= depth; ++depth_i)
  {
    SOC_PPC_EG_ENCAP_ENTRY_INFO_clear(&encap_entry_info[depth_i]);
    next_eep[depth_i] = ARAD_PP_EG_ENCAP_NEXT_EEP_INVALID;
    if (cur_eep_ndx == ARAD_PP_EG_ENCAP_NEXT_EEP_INVALID) {
        break;
    } else {
      arad_pp_eg_encap_access_key_prefix_type_get_unsafe(unit, cur_eep_ndx, &cur_eep_type);

      switch (cur_eep_type)
      {
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_POP:    
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_SWAP:    
        res = arad_pp_eg_encap_lif_entry_get_unsafe(
                  unit,
                  cur_eep_ndx,
                  &encap_entry_info[depth_i],
                  &next_eep[depth_i]
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
          break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_MPLS_TUNNEL:
          /* 
                  *In case depth > 0, the first entry might be PWE, the second entry could be MPLS tunnel. 
                  *So only check the first enry if it's PWE or not.
                  */
          if ((eep_type_ndx == SOC_PPC_EG_ENCAP_EEP_TYPE_LIF_EEP) && (depth_i == 0)) {
              res = arad_pp_eg_encap_lif_entry_get_unsafe(
                        unit,
                        cur_eep_ndx,
                        &encap_entry_info[depth_i],
                        &next_eep[depth_i]
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
          } else {
              res = arad_pp_eg_encap_tunnel_entry_get_unsafe(
                        unit,
                        cur_eep_ndx,
                        &encap_entry_info[depth_i],
                        &next_eep[depth_i]
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
          }
          break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_IP: 
        res = arad_pp_eg_encap_tunnel_entry_get_unsafe(
                  unit,
                  cur_eep_ndx,
                  &encap_entry_info[depth_i],
                  &next_eep[depth_i]
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_RIF: 
        res = arad_pp_eg_encap_vsi_entry_get_unsafe(
                unit,
                cur_eep_ndx,
                &encap_entry_info[depth_i],
                &ll_out_ac_valid,
                &next_eep[depth_i]
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_LINK_LAYER:
        res = arad_pp_eg_encap_ll_entry_get_unsafe(
                unit,
                cur_eep_ndx,
                &encap_entry_info[depth_i]
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
        
        if (encap_entry_info[depth_i].entry_val.ll_info.out_ac_valid)
        {
          next_eep[depth_i] = ARAD_PP_EG_ENCAP_NEXT_EEP_INVALID;
        }
        else
        {
          next_eep[depth_i] = encap_entry_info[depth_i].entry_val.ll_info.out_ac_lsb;
        }
        break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA:
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_IPV6:
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_DATA_ARP_OVERLAY:
        res = arad_pp_eg_encap_data_entry_get_unsafe(
                unit,
                cur_eep_ndx,
                &encap_entry_info[depth_i],
                &ll_out_ac_valid, /* Dummy pointer */
                &next_eep[depth_i]
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

        /* check if can get more specific information */
        res = arad_pp_eg_encap_data_entry_data_exact_info_get_unsafe(unit,&(encap_entry_info[depth_i].entry_val.data_info),&encap_entry_info[depth_i]);
        SOC_SAND_CHECK_FUNC_RESULT(res, 83, exit);
        break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_TRILL:
        res = arad_pp_eg_encap_trill_entry_get(
                unit,
                cur_eep_ndx,
                &encap_entry_info[depth_i],
                &next_eep[depth_i]
           ); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 84, exit);
        break; 
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_OUT_AC:
        encap_entry_info[depth_i].entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_AC;
        res = arad_pp_eg_ac_info_get_unsafe(unit, cur_eep_ndx, &(encap_entry_info[depth_i].entry_val.out_ac));
        SOC_SAND_CHECK_FUNC_RESULT(res, 86, exit);
        break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_ROO_LINK_LAYER:
        if (SOC_IS_JERICHO(unit)) {
            res = soc_jer_eg_encap_roo_ll_entry_get(unit, cur_eep_ndx, &encap_entry_info[depth_i]);
            SOC_SAND_CHECK_FUNC_RESULT(res, 91, exit);                                             
        }
        break; 
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_ISID:
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FEATURE_NOT_SUPPORTED_ERR, 90, exit);
		  break;
      case ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE:
        ARAD_PP_DO_NOTHING_AND_EXIT;
        break;
      default:
        SOC_SAND_SET_ERROR_CODE(SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR, 100, exit);
        break;
      }
    }

    if (depth_i != SOC_PPC_NOF_EG_ENCAP_EEP_TYPES_ARAD)
    {
      /* If not done, update current eep for next iteration */
      cur_eep_ndx = next_eep[depth_i];      
    }

    ++(*nof_entries);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_get_unsafe()", 0, eep_ndx);
}

uint32
  arad_pp_eg_encap_entry_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_EEP_TYPE                   eep_type_ndx,
    SOC_SAND_IN  uint32                                  eep_ndx,
    SOC_SAND_IN  uint32                                  depth
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_ENTRY_GET_VERIFY);

  if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      SOC_SAND_ERR_IF_ABOVE_MAX(eep_type_ndx, SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_MAX, SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  } 
  /* for jericho */
  else {
      SOC_SAND_ERR_IF_ABOVE_MAX(eep_type_ndx, SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_MAX, SOC_PPC_EG_ENCAP_EEP_TYPE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  }
  ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, eep_ndx, ARAD_PP_EG_ENCAP_EEP_NDX_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(depth, ARAD_PP_EG_ENCAP_DEPTH_MAX, ARAD_PP_EG_ENCAP_DEPTH_OUT_OF_RANGE_ERR, 30, exit);
 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_get_verify()", 0, eep_ndx);
}




/*********************************************************************
*     Parse data info to get overlay arp encap info.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
arad_pp_eg_encap_entry_data_info_to_overlay_arp_encap_info_unsafe(
 SOC_SAND_IN  int                                    unit, 
   SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO          *exact_data_info
   )
{
    uint32
       res = SOC_SAND_OK; 
    
    uint32 prge_data_relative_index; /* relative_index: index received from multiset.  */
    uint32 prge_data_base_index;     /* base_index is not static, depends on application. */
    uint32 prge_data_absolute_index; /* absolute_index = base_index + relative_index */
    ARAD_PP_EPNI_PRGE_DATA_TBL_DATA prge_data; /* prge data. Pointed by data_entry. Contains additional info for overlay arp */
    SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO overlay_arp_encap_info; 

    SOC_PPC_EG_ENCAP_DATA_INFO *data_info; 

    

    SOC_SAND_INIT_ERROR_DEFINITIONS(0); 

    data_info = &(exact_data_info->entry_val.data_info); 

    SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO_clear(&overlay_arp_encap_info);

    /* buffer to encap info */
    arad_pp_eg_encap_overlay_arp_data_from_data_entry_buffer(unit, 
                                                             &overlay_arp_encap_info, 
                                                             data_info);
    
    /* relative prge data index for overlay arp entries  */
    prge_data_relative_index = overlay_arp_encap_info.ll_vsi_pointer;

    /* get prge data base index for overlay arp entries */
    res  = arad_sw_db_eg_encap_prge_tbl_overlay_arp_entries_base_index_get(unit, &prge_data_base_index);
    SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

    /* absolute prge data index of overlay arp */
    prge_data_absolute_index = prge_data_base_index + prge_data_relative_index;

    /* get prge data */
    res = arad_pp_epni_prge_data_tbl_get_unsafe(unit, prge_data_absolute_index, &prge_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);

    /* get data from prge data */
    res = arad_pp_eg_encap_overlay_arp_data_from_prge_buffer(unit, &overlay_arp_encap_info, prge_data.prge_data_entry);

    /* update entry type */
    exact_data_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_OVERLAY_ARP_ENCAP;

    soc_sand_os_memcpy(&exact_data_info->entry_val.overlay_arp_encap_info, 
                       &overlay_arp_encap_info, 
                       sizeof(SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO));



    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_entry_data_info_to_overlay_arp_encap_info_unsafe()", 0, 0); 
    
}



#define ARAD_PP_EPNI_MPLS_CMD_TABLE_ENTRY_SIZE                      (1)

/*********************************************************************
*     Setting the push profile info - specifying how to build
 *     the label header.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_push_profile_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  profile_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO          *profile_info
  )
{
  uint32
    mpls_is_pipe,
    res = SOC_SAND_OK;
  uint32
    data[ARAD_PP_EPNI_MPLS_CMD_TABLE_ENTRY_SIZE];
  

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(profile_info);
 

  res = soc_mem_read(
          unit,
          EPNI_MPLS_CMD_PROFILEm,
          MEM_BLOCK_ANY,
          profile_ndx,
          data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, CWf, profile_info->has_cw);
  soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, EXPf, profile_info->exp);
  soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, TTLf, profile_info->ttl);
  soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, REMARK_PROFILEf, profile_info->remark_profile);

  mpls_is_pipe = profile_info->model == SOC_SAND_PP_MPLS_TUNNEL_MODEL_PIPE ? 0x1 : 0x0;

  if (!SOC_IS_JERICHO_PLUS(unit)) {
      
      soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, MODELf, mpls_is_pipe);
      soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, ADD_ENTROPY_LABELf, profile_info->add_entropy_label);
  } else { 
      /* In QAX and above, ttl and exp inheritance models are configured seperately */
      uint32 is_exp_set, is_ttl_set;
      uint32 reserved_label_profile_1, reserved_label_profile_2;

      is_ttl_set = (profile_info->ttl_model == SOC_SAND_PP_MPLS_TUNNEL_MODEL_SET) ? 1 : 0;
      is_exp_set = (profile_info->exp_model == SOC_SAND_PP_MPLS_TUNNEL_MODEL_SET) ? 1 : 0;

      soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, TTL_MODELf, is_ttl_set);
      soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, EXP_MODELf, is_exp_set);

      /* In QAX and above, each push profile holds three pointers to the reserved label profile table.
         reserved_label_profile_n, 1 <= n <=3, represents the nth label in the special label stack that is
         associated with the mpls entry that uses this push profile */
      reserved_label_profile_1 = (profile_info->add_entropy_label) ? 1 : 0;
      reserved_label_profile_2 = (profile_info->add_entropy_label_indicator) ? 2 : 0;

      soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, RESERVED_MPLS_PROFILE_1f, reserved_label_profile_1);
      soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, RESERVED_MPLS_PROFILE_2f, reserved_label_profile_2);
      soc_mem_field32_set(unit, EPNI_MPLS_CMD_PROFILEm, data, RESERVED_MPLS_PROFILE_3f, 0);
  }
 
  res = soc_mem_write(
          unit,
          EPNI_MPLS_CMD_PROFILEm,
          MEM_BLOCK_ANY,
          profile_ndx,
          data
        );
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_profile_info_set_unsafe()", profile_ndx, 0);
}

uint32
  arad_pp_eg_encap_push_profile_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  profile_ndx,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO          *profile_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(profile_ndx, ARAD_PP_EG_ENCAP_PROFILE_NDX_MAX, ARAD_PP_EG_ENCAP_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  if (SOC_IS_JERICHO_PLUS(unit) && profile_info->add_entropy_label_indicator && !(profile_info->add_entropy_label)) {
      SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Error: ELI cannot be set without setting EL\n")));
  }
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO,unit, profile_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_profile_info_set_verify()", profile_ndx, 0);
}

uint32
  arad_pp_eg_encap_push_profile_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  profile_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(profile_ndx, ARAD_PP_EG_ENCAP_PROFILE_NDX_MAX, ARAD_PP_EG_ENCAP_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_profile_info_get_verify()", profile_ndx, 0);
}

/*********************************************************************
*     Setting the push profile info - specifying how to build
 *     the label header.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_push_profile_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  profile_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO          *profile_info
  )
{
   uint32
    mpls_is_pipe,
    res = SOC_SAND_OK;
  uint32
    data[ARAD_PP_EPNI_MPLS_CMD_TABLE_ENTRY_SIZE];
  

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(profile_info);

  res = soc_mem_read(
          unit,
          EPNI_MPLS_CMD_PROFILEm,
          MEM_BLOCK_ANY,
          profile_ndx,
          data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  

  profile_info->has_cw            = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, CWf);
  profile_info->exp               = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, EXPf);
  profile_info->ttl               = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, TTLf);
  profile_info->remark_profile    = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, REMARK_PROFILEf);

  if (!SOC_IS_JERICHO_PLUS(unit)){
      mpls_is_pipe                    = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, MODELf);
      profile_info->add_entropy_label = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, ADD_ENTROPY_LABELf);  
      profile_info->model  = mpls_is_pipe ? SOC_SAND_PP_MPLS_TUNNEL_MODEL_PIPE : SOC_SAND_PP_MPLS_TUNNEL_MODEL_UNIFORM;
  } else {
      /* In QAX and above, ttl and exp inheritance models are configured seperately */           
      uint32 is_exp_set, is_ttl_set;
      uint32 reserved_label_profile_1, reserved_label_profile_2;

      is_ttl_set = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, TTL_MODELf);
      is_exp_set = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, EXP_MODELf);

      profile_info->ttl_model = is_ttl_set ? SOC_SAND_PP_MPLS_TUNNEL_MODEL_SET : SOC_SAND_PP_MPLS_TUNNEL_MODEL_COPY;
      profile_info->exp_model = is_exp_set ? SOC_SAND_PP_MPLS_TUNNEL_MODEL_SET : SOC_SAND_PP_MPLS_TUNNEL_MODEL_COPY;

      reserved_label_profile_1 = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, RESERVED_MPLS_PROFILE_1f);
      reserved_label_profile_2 = soc_mem_field32_get(unit, EPNI_MPLS_CMD_PROFILEm, data, RESERVED_MPLS_PROFILE_2f);

      profile_info->add_entropy_label = (reserved_label_profile_1 == 1) ? 1 : 0;
      profile_info->add_entropy_label_indicator = (reserved_label_profile_2 == 2) ? 1 : 0;


  }

  profile_info->exp_mark_mode = SOC_PPC_EG_ENCAP_EXP_MARK_MODE_FROM_PUSH_PROFILE;
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_profile_info_get_unsafe()", profile_ndx, 0);
}


/*********************************************************************
*     Set the EXP value of the pushed label as mapping of the
 *     TC and DP.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_push_exp_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_EXP                          exp
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(exp_key);

  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FEATURE_NOT_SUPPORTED_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_exp_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_encap_push_exp_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_EXP                          exp
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_SET_VERIFY);

  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FEATURE_NOT_SUPPORTED_ERR, 10, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_exp_info_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_encap_push_exp_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET_VERIFY);

  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FEATURE_NOT_SUPPORTED_ERR, 10, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_exp_info_get_verify()", 0, 0);
}

/*********************************************************************
*     Set the EXP value of the pushed label as mapping of the
 *     TC and DP.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_push_exp_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY               *exp_key,
    SOC_SAND_OUT SOC_SAND_PP_MPLS_EXP                          *exp
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_PUSH_EXP_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(exp_key);
  SOC_SAND_CHECK_NULL_INPUT(exp);

  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FEATURE_NOT_SUPPORTED_ERR, 10, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_push_exp_info_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Set Global information for PWE Encapsulation.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_pwe_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO              *glbl_info
  )
{
  uint32 res;
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_MPLS_CONTROL_WORDr, SOC_CORE_ALL, 0, MPLS_CONTROL_WORDf,  glbl_info->cw));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pwe_glbl_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_encap_pwe_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO              *glbl_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO, glbl_info, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pwe_glbl_info_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_encap_pwe_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pwe_glbl_info_get_verify()", 0, 0);
}

/*********************************************************************
*     Set Global information for PWE Encapsulation.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_pwe_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_PWE_GLBL_INFO              *glbl_info
  )
{
  uint32 res;
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_clear(glbl_info);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_MPLS_CONTROL_WORDr, SOC_CORE_ALL, 0, MPLS_CONTROL_WORDf, &glbl_info->cw));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_pwe_glbl_info_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Set Global information for Encapsulation.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_GLBL_INFO              *glbl_info
  )
{

  uint32 res;
  /* used only for vxlan in arad+ */
  #ifdef BCM_88660_A0 
    uint64 field64;
  #endif

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_GLBL_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_GRE_ETHERNET_TYPE_ETHERNETr, SOC_CORE_ALL, 0, GRE_ETHERNET_TYPE_ETHERNETf,  (uint32)glbl_info->l2_gre_prtcl_type));
  /* Set CUSTOM as Ethernet to eliminate the chance for undefined CUSTOM to hit */
  /*SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  15,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_GRE_ETHERNET_TYPE_CUSTOMr, SOC_CORE_ALL, 0, GRE_ETHERNET_TYPE_CUSTOMf,  (uint32)glbl_info->l2_gre_prtcl_type)); */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  25,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_IPV4_UNKNOWN_HEADER_CODE_CFGr, SOC_CORE_ALL, 0, IPV4_UNKNOWN_HEADER_CODE_ETHERNET_TYPEf,  (uint32)glbl_info->unkown_gre_prtcl_type));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_IPV4_UNKNOWN_HEADER_CODE_CFGr, SOC_CORE_ALL, 0, IPV4_UNKNOWN_HEADER_CODE_PROTOCOLf,  (uint32)glbl_info->unkown_ip_next_prtcl_type));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_ETHERNET_IPr, SOC_CORE_ALL, 0, ETHERNET_IPf,  (uint32)glbl_info->eth_ip_val));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_ETHERTYPE_DEFAULTr, SOC_CORE_ALL, 0, ETHERTYPE_DEFAULTf,  (uint32)glbl_info->unkown_ll_ether_type));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_ENTROPY_LABEL_MSBSr, SOC_CORE_ALL, 0, ENTROPY_LABEL_MSBSf,  (uint32)glbl_info->mpls_entry_label_msbs));

  /* if vxlan enabled */
  if (SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & SOC_DPP_IP_TUNNEL_TERM_DB_VXLAN) {
      /* for version under arad+, use egress program */
      if (SOC_IS_ARAD_B1_AND_BELOW(unit)) { 
          /* set Vxlan UDP at egress program */
          res = arad_egr_prog_vxlan_program_info_set(
                  unit,
                  glbl_info->vxlan_udp_dest_port
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
      }
      #ifdef BCM_88660_A0
      /* for arad+, use configuration */
      else {
          COMPILER_64_SET(field64, 0, (uint32) glbl_info->vxlan_udp_dest_port);
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_modify(unit, EPNI_CFG_VXLAN_UDP_FIELDSr, SOC_CORE_ALL, 0, CFG_UDP_DEST_PORT_VXLANf,  field64));
      }
      #endif

      /* set Vxlan UDP port at parser */
      res = arad_parser_vxlan_program_info_set(
              unit,
              glbl_info->vxlan_udp_dest_port
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_glbl_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_encap_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_GLBL_INFO              *glbl_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_GLBL_INFO_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_GLBL_INFO, glbl_info, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_glbl_info_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_encap_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_GLBL_INFO_GET_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_glbl_info_get_verify()", 0, 0);
}

/*********************************************************************
*     Set Global information for Encapsulation.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_GLBL_INFO              *glbl_info
  )
{
  uint32 res,
      field32;

  /* used only for vxlan in arad+ */
  #ifdef BCM_88660_A0 
    uint64 field64;
  #endif

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_EG_ENCAP_GLBL_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  SOC_PPC_EG_ENCAP_GLBL_INFO_clear(glbl_info);
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHP_GRE_ETHERNET_TYPE_ETHERNETr, SOC_CORE_ALL, 0, GRE_ETHERNET_TYPE_ETHERNETf, &field32));
  glbl_info->l2_gre_prtcl_type = field32;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_IPV4_UNKNOWN_HEADER_CODE_CFGr, SOC_CORE_ALL, 0, IPV4_UNKNOWN_HEADER_CODE_ETHERNET_TYPEf, &field32));
  glbl_info->unkown_gre_prtcl_type = field32;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_IPV4_UNKNOWN_HEADER_CODE_CFGr, SOC_CORE_ALL, 0, IPV4_UNKNOWN_HEADER_CODE_PROTOCOLf, &field32));
  glbl_info->unkown_ip_next_prtcl_type = field32;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_ETHERNET_IPr, SOC_CORE_ALL, 0, ETHERNET_IPf, &field32));
  glbl_info->eth_ip_val = field32;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_ETHERTYPE_DEFAULTr, SOC_CORE_ALL, 0, ETHERTYPE_DEFAULTf, &field32));
  glbl_info->unkown_ll_ether_type = field32;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_ENTROPY_LABEL_MSBSr, SOC_CORE_ALL, 0, ENTROPY_LABEL_MSBSf, &field32));
  glbl_info->mpls_entry_label_msbs = field32;

  /* get Vxlan UDP port */
  if (SOC_IS_ARAD_B1_AND_BELOW(unit)) {
      res = arad_parser_vxlan_program_info_get(
              unit,
              &glbl_info->vxlan_udp_dest_port
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }
  #ifdef BCM_88660_A0
  /* for arad+, use configuration */
  else {
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, EPNI_CFG_VXLAN_UDP_FIELDSr, SOC_CORE_ALL, 0, CFG_UDP_DEST_PORT_VXLANf, &field64));
      COMPILER_64_TO_32_LO(glbl_info->vxlan_udp_dest_port, field64);
  }
  #endif

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_glbl_info_get_unsafe()", 0, 0);
}


/*********************************************************************
*     Set source IP address for IPv4 Tunneling.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  uint32                                  src_ip
  )
{
  uint32
    entry_offset,
    res = SOC_SAND_OK;
  soc_reg_above_64_val_t
    reg_val;
 
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET_UNSAFE);
  
	SOC_REG_ABOVE_64_CLEAR(&reg_val);

  res = READ_EPNI_IPV4_SIPr(unit, REG_PORT_ANY, reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  entry_offset = entry_ndx * ARAD_PP_EG_ENCAP_SIP_NOF_BITS;

  res = soc_sand_bitstream_set_any_field(
          &src_ip,
          entry_offset,
          ARAD_PP_EG_ENCAP_SIP_NOF_BITS,
          reg_val
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, WRITE_EPNI_IPV4_SIPr(unit, REG_PORT_ANY, reg_val));
#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_EPNI_IPV4_SIP_ROUTINGr(unit, REG_PORT_ANY, reg_val));
  }
#endif /* BCM_88660_A0 */


  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  uint32                                  src_ip
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_EG_ENCAP_ENTRY_NDX_MAX, ARAD_PP_EG_ENCAP_ENTRY_NDX_OUT_OF_RANGE_ERR, 10, exit);
  /* SOC_SAND_ERR_IF_ABOVE_MAX(src_ip, ARAD_PP_EG_ENCAP_SRC_IP_MAX, ARAD_PP_EG_ENCAP_SRC_IP_OUT_OF_RANGE_ERR, 20, exit); */
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_set_verify()", entry_ndx, 0);
}

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_EG_ENCAP_ENTRY_NDX_MAX, ARAD_PP_EG_ENCAP_ENTRY_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_get_verify()", entry_ndx, 0);
}

/*********************************************************************
*     Set source IP address for IPv4 Tunneling.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT uint32                                  *src_ip
  )
{
  uint32
    entry_offset,
    res = SOC_SAND_OK;
  soc_reg_above_64_val_t
    fld_val,
    reg_val;
  

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_SRC_IP_GET_UNSAFE);

  SOC_REG_ABOVE_64_CLEAR(fld_val);
  SOC_REG_ABOVE_64_CLEAR(reg_val);

  res = READ_EPNI_IPV4_SIPr(unit, REG_PORT_ANY, reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  ARAD_FLD_FROM_REG_ABOVE_64(EPNI_IPV4_SIPr, IPV4_SIPf, fld_val, reg_val, 15, exit);
  
  
  entry_offset = entry_ndx * ARAD_PP_EG_ENCAP_SIP_NOF_BITS;

  res = soc_sand_bitstream_get_any_field(
          fld_val,
          entry_offset,
          ARAD_PP_EG_ENCAP_SIP_NOF_BITS,
          src_ip
        );

  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_src_ip_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Set TTL for IPv4 Tunneling.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl
  )
{
  uint32
    res = SOC_SAND_OK,
    tmp,
    fld_offset,
    fld_val;
 
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_UNSAFE);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_IPV4_TTLr, SOC_CORE_ALL, 0, IPV4_TTLf, &fld_val));
  
  fld_offset = entry_ndx * ARAD_PP_EG_ENCAP_TTL_NOF_BITS;

  tmp = SOC_SAND_PP_TTL_VAL_GET(ttl);
  res = soc_sand_bitstream_set_any_field(
          &tmp,
          fld_offset,
          ARAD_PP_EG_ENCAP_TTL_NOF_BITS,
          &fld_val
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_IPV4_TTLr, SOC_CORE_ALL, 0, IPV4_TTLf,  fld_val));

  /* write if pipe */
  res = READ_EPNI_IPV4_TTL_MODELr(unit, REG_PORT_ANY, &fld_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);

  tmp = SOC_SAND_PP_TOS_IS_UNIFORM_GET(ttl);

  /* write if pipe to hardware */
  tmp = !tmp;
  res = soc_sand_bitstream_set_any_field(
          &tmp,
          entry_ndx,
          1,
          &fld_val
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = WRITE_EPNI_IPV4_TTL_MODELr(unit, REG_PORT_ANY, fld_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_unsafe()", entry_ndx, 0);
}

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_EG_ENCAP_TTL_INDEX_MAX, ARAD_PP_EG_ENCAP_TTL_INDEX_OUT_OF_RANGE_ERR, 10, exit);
  /* SOC_SAND_ERR_IF_ABOVE_MAX(ttl, SOC_SAND_PP_IP_TTL_MAX, SOC_SAND_PP_IP_TTL_OUT_OF_RANGE_ERR, 20, exit); */
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_verify()", entry_ndx, 0);
}

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_EG_ENCAP_TTL_INDEX_MAX, ARAD_PP_EG_ENCAP_TTL_INDEX_OUT_OF_RANGE_ERR, 10, exit);
  
  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_get_verify()", entry_ndx, 0);
}


/*********************************************************************
*     Set TTL for IPv4 Tunneling.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IP_TTL                            *ttl
  )
{
  uint32
    res = SOC_SAND_OK,
    tmp,
    ttl_val,
    fld_offset,
    fld_val;
    
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(ttl);
  
  fld_offset = entry_ndx * ARAD_PP_EG_ENCAP_TTL_NOF_BITS;

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_IPV4_TTLr, SOC_CORE_ALL, 0, IPV4_TTLf, &fld_val));
  
  fld_offset = entry_ndx * ARAD_PP_EG_ENCAP_TTL_NOF_BITS;

  res = soc_sand_bitstream_get_any_field(
          &fld_val,
          fld_offset,
          ARAD_PP_EG_ENCAP_TTL_NOF_BITS,
          &tmp
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  ttl_val = tmp & 255;

  /* get if uniform */
  res = READ_EPNI_IPV4_TTL_MODELr(unit, REG_PORT_ANY, &fld_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  tmp = (fld_val & (1 << entry_ndx));
  tmp = !tmp;

  SOC_SAND_PP_TTL_SET(*ttl,ttl_val,tmp);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_ttl_set_unsafe()", entry_ndx, 0);
}

/*********************************************************************
*     Set TOS for IPv4 Tunneling.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          tos
  )
{
  uint32
    res = SOC_SAND_OK,
    tmp,
    fld_offset;
  soc_reg_above_64_val_t
    reg_val;
  uint32
    reg_32_val;  
  uint64
      reg_64_val;
  int
      is_uniform;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TTL_SET_UNSAFE);

  SOC_REG_ABOVE_64_CLEAR(reg_val);

  is_uniform = SOC_SAND_PP_TOS_IS_UNIFORM_GET(tos);

  if (!is_uniform){
      res = READ_EPNI_IPV4_TOSr(unit, REG_PORT_ANY, reg_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);


      fld_offset = entry_ndx * ARAD_PP_EG_ENCAP_TOS_NOF_BITS;

      tmp = SOC_SAND_PP_TOS_VAL_GET(tos);

      res = soc_sand_bitstream_set_any_field(
              &tmp,
              fld_offset,
              ARAD_PP_EG_ENCAP_TTL_NOF_BITS,
              reg_val
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      res = WRITE_EPNI_IPV4_TOSr(unit, REG_PORT_ANY, reg_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  } else {

      res = READ_EPNI_IPV4_TUNNEL_REMARKr(unit, REG_PORT_ANY, &reg_64_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);

      fld_offset = entry_ndx * ARAD_PP_EG_ENCAP_REMARK_NOF_BITS;

      tmp = SOC_SAND_PP_TOS_VAL_GET(tos);

      res = soc_sand_bitstream_set_any_field(
          &tmp,
          fld_offset,
          ARAD_PP_EG_ENCAP_REMARK_NOF_BITS,
          reg_val
      );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      COMPILER_64_SET(reg_64_val, reg_val[1], reg_val[0]);

      res = WRITE_EPNI_IPV4_TUNNEL_REMARKr(unit, REG_PORT_ANY, reg_64_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);
  }

  /* write if pipe */
  res = READ_EPNI_IPV4_TOS_MODELr(unit, REG_PORT_ANY, &reg_32_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  tmp = SOC_SAND_PP_TOS_IS_UNIFORM_GET(tos);

  /* write if pipe to hardware */
  tmp = !tmp;
  res = soc_sand_bitstream_set_any_field(
      &tmp,
      entry_ndx,
      1,
      &reg_32_val
  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

  res = WRITE_EPNI_IPV4_TOS_MODELr(unit, REG_PORT_ANY, reg_32_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_unsafe()", entry_ndx, 0);
}


uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_TOS                          tos
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_EG_ENCAP_TOS_INDEX_MAX, ARAD_PP_EG_ENCAP_TOS_INDEX_OUT_OF_RANGE_ERR, 10, exit);
  /* SOC_SAND_ERR_IF_ABOVE_MAX(tos, SOC_SAND_PP_IPV4_TOS_MAX, SOC_SAND_PP_IPV4_TOS_OUT_OF_RANGE_ERR, 20, exit); */


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_verify()", entry_ndx, 0);
}

uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_ndx, ARAD_PP_EG_ENCAP_TOS_INDEX_MAX, ARAD_PP_EG_ENCAP_TOS_INDEX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_tos_get_verify()", entry_ndx, 0);
}

/*********************************************************************
*     Set TOS for IPv4 Tunneling.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_ipv4_tunnel_glbl_tos_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  entry_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IPV4_TOS                          *tos
  )
{
  uint32
    res = SOC_SAND_OK,
    tmp=0,
    tos_val,
    fld_offset;
  soc_reg_above_64_val_t
    reg_val;
  uint32
    reg_32_val;  
  uint64
      reg_64_val;
  int
      is_uniform;

    
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_EG_ENCAP_IPV4_TUNNEL_GLBL_TOS_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(tos);
  
  SOC_REG_ABOVE_64_CLEAR(reg_val);

  /* get if uniform */
  res = READ_EPNI_IPV4_TOS_MODELr(unit, REG_PORT_ANY, &reg_32_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  tmp = (reg_32_val & (1 << entry_ndx));
  tmp = !tmp;

  is_uniform = tmp;
 
  if (!is_uniform)
  {
      res = READ_EPNI_IPV4_TOSr(unit, REG_PORT_ANY, reg_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

      fld_offset = entry_ndx * ARAD_PP_EG_ENCAP_TOS_NOF_BITS;
  } else {
      res = READ_EPNI_IPV4_TUNNEL_REMARKr(unit, REG_PORT_ANY, &reg_64_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      COMPILER_64_TO_32_LO(reg_val[0], reg_64_val);
      COMPILER_64_TO_32_LO(reg_val[1], reg_64_val);

      fld_offset = entry_ndx * ARAD_PP_EG_ENCAP_REMARK_NOF_BITS;
  }

  res = soc_sand_bitstream_get_any_field(
          reg_val,
          fld_offset,
          ARAD_PP_EG_ENCAP_TTL_NOF_BITS,
          &tmp
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  tos_val = tmp & 255;

  SOC_SAND_PP_TOS_SET(*tos,tos_val,is_uniform);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_ipv4_tunnel_glbl_tos_set_unsafe()", entry_ndx, 0);
}

/*********************************************************************
*     Set MPLS PIPE mode to do copy EXP (1) or set EXP (0).
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint8                                is_exp_copy
  )
{
    uint32
      res = SOC_SAND_OK;
    uint32
      reg_val;  

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 88, exit, READ_EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r(unit, REG_PORT_ANY, &reg_val));
    soc_reg_field_set(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r, &reg_val, CFG_MPLS_PIPE_FIX_ENABLEf, is_exp_copy);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 88, exit, WRITE_EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r(unit, REG_PORT_ANY, reg_val));

  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set_unsafe()", is_exp_copy, 0);
}

uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set_verify(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint8                                is_exp_copy
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    if (SOC_IS_ARAD_B1_AND_BELOW(unit)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG(" Error: Unsupported device\n")));
    }

    if ((is_exp_copy!=0) && (is_exp_copy!=1)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG(" Error: is_exp_copy must be 0 or 1 (set or unset)\n")));
    }

  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_set_verify()", 0, 0);
}

uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_get_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_OUT uint8                                *is_exp_copy
  )
{
    uint32
      res = SOC_SAND_OK;
    uint32
      reg_val;  

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 88, exit, READ_EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r(unit, REG_PORT_ANY, &reg_val));
    *is_exp_copy = soc_reg_field_get(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r, reg_val, CFG_MPLS_PIPE_FIX_ENABLEf);

  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_get_unsafe()", 0, 0);
}

uint32
  arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_get_verify(
    SOC_SAND_IN  int                                  unit
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    if (SOC_IS_ARAD_B1_AND_BELOW(unit)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG(" Error: Unsupported device\n")));
    }

  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_pipe_mode_is_exp_copy_get_verify()", 0, 0);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_eg_encap module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_eg_encap_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_eg_encap;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_eg_encap module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_eg_encap_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_eg_encap;
}

uint32
  SOC_PPC_EG_ENCAP_RANGE_INFO_verify(
    int unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_RANGE_INFO *info
  )
{
  uint32
      ndx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->ll_limit, (16*1024-1), ARAD_PP_EG_ENCAP_LL_LIMIT_OUT_OF_RANGE_ERR, 10, exit);
  /* ARAD_PP_EG_ENCAP_IP_TNL_LIMIT_MIN may be changed and be more thean 0 */
  /* coverity[unsigned_compare : FALSE] */
  SOC_SAND_ERR_IF_OUT_OF_RANGE(info->ip_tnl_limit, ARAD_PP_EG_ENCAP_IP_TNL_LIMIT_MIN, (16*1024-1), ARAD_PP_EG_ENCAP_IP_TNL_LIMIT_OUT_OF_RANGE_ERR, 20, exit);

  for (ndx=0; ndx<SOC_DPP_DEFS_MAX(EG_ENCAP_NOF_BANKS); ndx++) {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->bank_access_phase[ndx], SOC_PPC_EG_ENCAP_ACCESS_PHASE_MAX(unit), ARAD_PP_NOF_EG_ENCAP_ACCESS_PHASE_OUT_OF_RANGE_ERROR,
                            30+ndx, exit);
  }
  
  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_RANGE_INFO_verify()",0,0);
}

uint32 SOC_PPC_EG_ENCAP_PROTECTION_INFO_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PROTECTION_INFO *info)
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    if (info->is_protection_valid) {
        SOC_SAND_ERR_IF_ABOVE_MAX(info->protection_pointer, SOC_DPP_DEFS_GET(unit, nof_failover_egress_ids),
                                  ARAD_PP_EG_ENCAP_PROTECTION_POINTER_OUT_OF_RANGE_ERR, 10, exit);
        SOC_SAND_ERR_IF_ABOVE_MAX(info->protection_pass_value, ARAD_PP_EG_ENCAP_PROTECTION_PASS_VAL_MAX,
                                  ARAD_PP_EG_ENCAP_PROTECTION_PASS_VAL_OUT_OF_RANGE_ERR, 20, exit);
    }

    SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_PROTECTION_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_SWAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_SWAP_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->swap_label, ARAD_PP_EG_ENCAP_SWAP_LABEL_MAX, ARAD_PP_EG_ENCAP_SWAP_LABEL_OUT_OF_RANGE_ERR, 10, exit);


  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_SWAP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_DATA_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_DATA_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_DATA_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_PWE_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_INFO *info
  )
{
  uint32 res = SOC_SAND_OK;
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->label, ARAD_PP_EG_ENCAP_LABEL_MAX, ARAD_PP_EG_ENCAP_LABEL_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->push_profile, ARAD_PP_EG_ENCAP_PUSH_PROFILE_MAX, ARAD_PP_EG_ENCAP_PUSH_PROFILE_OUT_OF_RANGE_ERR, 11, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_PROTECTION_INFO, unit, &(info->protection_info), 20, exit);
  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_PWE_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->tpid_profile, ARAD_PP_EG_ENCAP_TPID_PROFILE_MAX, ARAD_PP_EG_ENCAP_TPID_PROFILE_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_POP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_POP_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  if (info->pop_type != SOC_PPC_MPLS_COMMAND_TYPE_POP) {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_EG_ENCAP_POP_TYPE_OUT_OF_RANGE_ERR, 10, exit); 
  }
  SOC_SAND_ERR_IF_ABOVE_MAX(info->model, ARAD_PP_EG_ENCAP_MODEL_MAX, ARAD_PP_EG_ENCAP_MODEL_OUT_OF_RANGE_ERR, 11,exit );
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_POP_INTO_ETH_INFO, &(info->ethernet_info), 12, exit);
  SOC_SAND_ERR_IF_OUT_OF_RANGE(info->pop_next_header, SOC_PPC_PKT_FRWRD_TYPE_BRIDGE, SOC_PPC_PKT_FRWRD_TYPE_MPLS, SOC_PPC_EG_ENCAP_POP_INFO_PKT_FRWRD_TYPE_OUT_OF_RANGE_ERR, 13, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->out_vsi, ARAD_PP_EG_ENCAP_OUT_VSI_MAX, ARAD_PP_EG_ENCAP_OUT_VSI_OUT_OF_RANGE_ERR, 10, exit);
  
  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_POP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->tunnel_label, ARAD_PP_EG_ENCAP_TUNNEL_LABEL_MAX, ARAD_PP_EG_ENCAP_TUNNEL_LABEL_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->push_profile, ARAD_PP_EG_ENCAP_PUSH_PROFILE_MAX, ARAD_PP_EG_ENCAP_PUSH_PROFILE_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < SOC_PPC_EG_ENCAP_MPLS_MAX_NOF_TUNNELS; ++ind)
  {
    ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_MPLS_TUNNEL_INFO, unit, &(info->tunnels[ind]), 10, exit);
  }
  SOC_SAND_ERR_IF_ABOVE_MAX(info->nof_tunnels, ARAD_PP_EG_ENCAP_NOF_TUNNELS_MAX, ARAD_PP_EG_ENCAP_NOF_TUNNELS_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_ERR_IF_NOT_EQUALS_VALUE(
          info->orientation, SOC_SAND_PP_NOF_HUB_SPOKE_ORIENTATIONS,
          ARAD_PP_EG_ENCAP_ORIENTATION_OUT_OF_RANGE_ERR, 12, exit
        );  
  SOC_SAND_ERR_IF_ABOVE_MAX(info->out_vsi, SOC_DPP_DEFS_GET(unit, nof_out_vsi) - 1, ARAD_PP_EG_ENCAP_OUT_VSI_OUT_OF_RANGE_ERR, 20, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_PROTECTION_INFO, unit, &(info->protection_info), 30, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  /* Nothing to verify in dest */
  /* SOC_SAND_ERR_IF_ABOVE_MAX(info->dest, ARAD_PP_EG_ENCAP_DEST_MAX, ARAD_PP_EG_ENCAP_DEST_OUT_OF_RANGE_ERR, 10, exit); */
  SOC_SAND_ERR_IF_ABOVE_MAX(info->src_index, ARAD_PP_EG_ENCAP_SRC_INDEX_MAX, ARAD_PP_EG_ENCAP_SRC_INDEX_OUT_OF_RANGE_ERR, 11, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->ttl_index, ARAD_PP_EG_ENCAP_TTL_INDEX_MAX, ARAD_PP_EG_ENCAP_TTL_INDEX_OUT_OF_RANGE_ERR, 12, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->tos_index, ARAD_PP_EG_ENCAP_TOS_INDEX_MAX, ARAD_PP_EG_ENCAP_TOS_INDEX_OUT_OF_RANGE_ERR, 13, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_IPV4_TUNNEL_INFO, &(info->dest), 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->out_vsi, ARAD_PP_EG_ENCAP_OUT_VSI_MAX, ARAD_PP_EG_ENCAP_OUT_VSI_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  /* Nothing to verify */

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_IPV6_TUNNEL_INFO, &(info->tunnel), 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_IPV6_ENCAP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->vid, SOC_SAND_PP_VLAN_ID_MAX, ARAD_PP_EG_ENCAP_MIRROR_VLAN_OUT_OF_RANGE_ERR, 11, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->encap_id, ARAD_PP_EG_ENCAP_NOF_MIRRORS, SOC_SAND_VALUE_ABOVE_MAX_ERR, 15, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_MIRROR_TUNNEL_INFO, &(info->tunnel), 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_MIRROR_ENCAP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_LL_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_LL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->out_vid, SOC_DPP_DEFS_GET(unit, nof_out_vsi) - 1, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 12, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->tpid_index, ARAD_PP_EG_ENCAP_TPID_INDEX_MAX, ARAD_PP_EG_ENCAP_TPID_INDEX_OUT_OF_RANGE_ERR, 15, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->ll_remark_profile, ARAD_PP_EG_LL_ENCAP_REMARK_PROFILE_INDEX_MAX, ARAD_PP_EG_LL_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR, 20, exit);
  if (info->out_ac_valid) {
      ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, info->out_ac_lsb, SOC_PPC_AC_ID_OUT_OF_RANGE_ERR, 30, exit); 
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_LL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO_verify(
    SOC_SAND_IN    int unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_NOF(info->out_vsi, SOC_DPP_CONFIG(unit)->l3.nof_rifs, ARAD_PP_EG_ENCAP_OUT_VSI_OUT_OF_RANGE_ERR, 10, exit);

  if (SOC_IS_JERICHO_PLUS(unit)) {
      SOC_SAND_ERR_IF_ABOVE_NOF(info->outrif_profile_index, SOC_DPP_CONFIG(unit)->l3.nof_rif_profiles, ARAD_PP_EG_ENCAP_OUT_RIF_PROFILE_OUT_OF_RANGE_ERR, 15, exit);
  }

  SOC_SAND_ERR_IF_ABOVE_MAX(info->remark_profile, ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_MAX, ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR, 20, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_VSI_ENCAP_INFO_verify()",0,0);
}



uint32
  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_verify(
     SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO *info
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

  /* In QAX and above, ttl and exp inheritance models are configured seperately */
  if (SOC_IS_JERICHO_PLUS(unit)) {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->exp_model, ARAD_PP_EG_ENCAP_MODEL_MAX, ARAD_PP_EG_ENCAP_MODEL_OUT_OF_RANGE_ERR, 10, exit);
      SOC_SAND_ERR_IF_ABOVE_MAX(info->ttl_model, ARAD_PP_EG_ENCAP_MODEL_MAX, ARAD_PP_EG_ENCAP_MODEL_OUT_OF_RANGE_ERR, 10, exit);
  } else {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->model, ARAD_PP_EG_ENCAP_MODEL_MAX, ARAD_PP_EG_ENCAP_MODEL_OUT_OF_RANGE_ERR, 10, exit);
  }
  /* SOC_SAND_ERR_IF_ABOVE_MAX(info->ttl, SOC_SAND_PP_IP_TTL_MAX, SOC_SAND_PP_IP_TTL_OUT_OF_RANGE_ERR, 12, exit); */
  SOC_SAND_ERR_IF_ABOVE_MAX(info->exp_mark_mode, SOC_PPC_EG_ENCAP_EXP_MARK_MODE_MAX, SOC_PPC_EG_ENCAP_EXP_MARK_MODE_OUT_OF_RANGE_ERR, 13, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->exp, SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 14, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->add_entropy_label, ARAD_PP_EG_ENCAP_ADD_ENTROPY_LABEL_MAX, ARAD_PP_EG_ENCAP_ADD_ENTROPY_LABEL_OUT_OF_RANGE_ERR, 15, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->remark_profile, ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_MAX, ARAD_PP_EG_VSI_ENCAP_REMARK_PROFILE_INDEX_OUT_OF_RANGE_ERR, 15, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_PUSH_PROFILE_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY_verify(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PUSH_EXP_KEY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->push_profile, ARAD_PP_EG_ENCAP_PUSH_PROFILE_MAX, ARAD_PP_EG_ENCAP_PUSH_PROFILE_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->tc, SOC_SAND_PP_TC_MAX, SOC_SAND_PP_TC_OUT_OF_RANGE_ERR, 11, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->dp, SOC_SAND_PP_DP_MAX, SOC_SAND_PP_DP_OUT_OF_RANGE_ERR, 12, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_PUSH_EXP_KEY_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_PWE_GLBL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->cw, ARAD_PP_EG_ENCAP_CW_MAX, ARAD_PP_EG_ENCAP_CW_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_PWE_GLBL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_GLBL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_GLBL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_ENTRY_VALUE_verify(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENTRY_VALUE *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->vsi, ARAD_PP_VSI_ID_MAX, SOC_PPC_VSI_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_AC_INFO, unit, &(info->out_ac),  11, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_SWAP_INFO, &(info->swap_info), 12, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_PWE_INFO, unit, &(info->pwe_info), 13, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_POP_INFO, &(info->pop_info), 14, exit);
  ARAD_PP_STRUCT_VERIFY_UNIT(SOC_PPC_EG_ENCAP_MPLS_ENCAP_INFO, unit, &(info->mpls_encap_info), 15, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_IPV4_ENCAP_INFO, &(info->ipv4_encap_info), 16, exit);
  res = SOC_PPC_EG_ENCAP_LL_INFO_verify(unit, &(info->ll_info));
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_ENTRY_VALUE_verify()",0,0);
}

uint32
  SOC_PPC_EG_ENCAP_ENTRY_INFO_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_ENTRY_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->entry_type, SOC_PPC_EG_ENCAP_ENTRY_TYPE_MAX, SOC_PPC_EG_ENCAP_ENTRY_TYPE_OUT_OF_RANGE_ERR, 10, exit);
  res = SOC_PPC_EG_ENCAP_ENTRY_VALUE_verify(unit, &(info->entry_val));
  SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_ENTRY_INFO_verify()",0,0);
}


/* Number of my nicknames: my nicknames index (index 0) + number of my virtual nicknames
   Use SOC_DPP_DEFINES instead. */
#define ARAD_PP_EG_ENCAP_NOF_MY_NICK_NAMES 4

uint32
   SOC_PPC_EG_ENCAP_TRILL_INFO_verify(
      SOC_SAND_IN int                          unit, 
      SOC_SAND_IN SOC_PPC_EG_ENCAP_TRILL_INFO *info) {
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0); 

    SOC_SAND_ERR_IF_ABOVE_NOF(info->my_nickname_index, 4, SOC_SAND_VALUE_ABOVE_MAX_ERR, 10, exit);
    /* no need to check nickname (nickname type is uint16, and its length is 16b) */
    SOC_SAND_ERR_IF_ABOVE_MAX(info->m, (1 << SOC_SAND_PP_TRILL_M_NOF_BITS) -1, SOC_SAND_VALUE_ABOVE_MAX_ERR, 12, exit);
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_TRILL_INFO_verify()", 0, 0); 
}


/*********************************************************************
*     Add arp overlay encapsulation entry to eedb 
 *     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
arad_pp_eg_encap_overlay_arp_data_entry_add_unsafe(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_IN  uint32                                 overlay_ll_eep_ndx, 
     SOC_SAND_INOUT  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *ll_encap_info
   ) 
{   
  uint32 
      res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(ll_encap_info);

  /* overlay arp eedb entry changes between arad+ and jericho
   * 
   * in arad+: To build the arp entry, we use a generic eedb entry: data-entry.
   *           Data entry contains the DA and a pointer to another table: prge-data. 
   *           format of eedb entry: ... data-entry[72:17] ...
   *           data-entry(56b): DA [55:8] LL-VSI-Pointer[7:4] Reserved[3:2] Entry-Type[1:0]
   *           LL-VSI-Pointer(4b) is a relative index from prge-data. Prge-data has a reserved range for overlay arp. 
   *           Format of prge data for overlay arp:
   *           prge-data(128b):(if 1tag)   MyMAC[99:52] TPID[51:36] PCP-DEI[35:32] VID[31:20] ETHERTYPE[19:4] SIZE-OF-HEADER-WITHOUT-DA[3:0]
   *           prge-data(128b): (untagged) MyMAC[67:20]                                       ETHERTYPE[19:4] SIZE-OF-HEADER-WITHOUT-DA[3:0]
   */
  if (SOC_IS_ARADPLUS(unit)) {

      SOC_PPC_EG_ENCAP_DATA_INFO data_info;  /* eedb entry of type "data_entry" */

      ARAD_PP_EPNI_PRGE_DATA_TBL_DATA prge_data;
      uint32 prge_data_relative_index; /* relative_index: index received from multiset.  */
      uint32 prge_data_base_index;     /* base_index is not static, depends on application. */
      uint32 prge_data_absolute_index; /* absolute_index = base_index + relative_index */
      uint8  prge_data_first_appear;
      SOC_SAND_SUCCESS_FAILURE  prge_data_success;

      SOC_PPC_EG_ENCAP_DATA_INFO_clear(&data_info);

      /* build prge data buffer */
      res = arad_pp_eg_encap_overlay_arp_data_to_prge_buffer(
         unit, 
         ll_encap_info,   
         prge_data.prge_data_entry);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      /* allocate prge data entry in sw
         Get a prge index. index relative to prge data table*/
      res  = arad_sw_db_multiset_add(
         unit, 
		 ARAD_SW_DB_CORE_ANY,
         ARAD_PP_SW_DB_MULTI_SET_ENTRY_OVERLAY_ARP_PROG_DATA_ENTRY, 
         prge_data.prge_data_entry,
         &prge_data_relative_index,    
         &prge_data_first_appear,
         &prge_data_success
      );
      SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);

      /* get prge data base index for overlay arp entries */
      res  = arad_sw_db_eg_encap_prge_tbl_overlay_arp_entries_base_index_get(unit, &prge_data_base_index);
      SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

      /* absolute prge data index of overlay arp */
      prge_data_absolute_index = prge_data_base_index + prge_data_relative_index;

      /* add prge data in prge data table (HW) */
      res = arad_pp_epni_prge_data_tbl_set_unsafe(unit, prge_data_absolute_index, &prge_data);
      SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);

      /* update encap info */
      ll_encap_info->ll_vsi_pointer = prge_data_relative_index;

      /* build data entry */
      res = arad_pp_eg_encap_overlay_arp_data_to_data_entry_buffer(unit, ll_encap_info, &data_info);
      SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);

      /* write in eedb (HW) */
      res = arad_pp_eg_encap_data_lif_entry_add_unsafe(unit, overlay_ll_eep_ndx, &data_info, FALSE, 0);
      SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);
  } 

  exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_overlay_arp_data_entry_add_unsafe()",0,0);

}



uint32 
SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO_verify(SOC_SAND_IN SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *info) { 
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0); 
    SOC_SAND_CHECK_NULL_INPUT(info); 
    
    SOC_SAND_MAGIC_NUM_VERIFY(info); 
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO_verify()", 0, 0); 
}

uint32
  arad_pp_eg_encap_overlay_arp_data_entry_add_verify(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_IN  uint32                                 overlay_ll_eep_ndx, 
     SOC_SAND_IN  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *ll_encap_info
   ) 
{
  uint32
    res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, overlay_ll_eep_ndx, ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR, 10, exit);
    ARAD_PP_STRUCT_VERIFY(SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO, ll_encap_info, 20, exit);


    ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_overlay_arp_data_entry_add_verify()",0,0);
}

uint32
  arad_pp_eg_encap_mpls_default_ttl_set(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_IN  uint8                                  ttl_val
   ) 
{
  uint32
      res = SOC_SAND_OK,
      data;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = READ_EPNI_PP_CONFIGr(unit, REG_PORT_ANY, &data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    soc_reg_field_set(unit, EPNI_PP_CONFIGr, &data, DEFAULT_TTLf, ttl_val);

    res = WRITE_EPNI_PP_CONFIGr(unit, SOC_CORE_ALL, data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_default_ttl_set()",0,0);
}

uint32
  arad_pp_eg_encap_mpls_default_ttl_get(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_OUT uint8                                  *ttl_val
   ) 
{
  uint32
      res = SOC_SAND_OK,
      data;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = READ_EPNI_PP_CONFIGr(unit, REG_PORT_ANY, &data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    *ttl_val = (uint8) soc_reg_field_get(unit, EPNI_PP_CONFIGr, data, DEFAULT_TTLf);

    ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_mpls_default_ttl_get()",0,0);
}

STATIC soc_error_t 
    arad_pp_eg_encap_trill_entry_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  eep_ndx,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO                 *encap_info,
    SOC_SAND_OUT uint32                                  *next_eep){

    uint32 res = SOC_SAND_OK; 


    ARAD_PP_EG_ENCAP_ACCESS_TRILL_ENTRY_FORMAT tbl_data; 

    SOC_PPC_EG_ENCAP_EEP_TYPE                  
        eep_type_ndx = SOC_PPC_EG_ENCAP_EEP_TYPE_TRILL;
    ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE
        cur_eep_type = ARAD_PP_EG_ENCAP_EEDB_ACCESS_TYPE_NONE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0); 

    SOC_SAND_CHECK_NULL_INPUT(encap_info);
    SOC_PPC_EG_ENCAP_ENTRY_INFO_clear(encap_info); 
    soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

    /* 1. get the eedb entry of type Trill
     * 2. convert from access layer struct to soc layer struct
     * 3. set next_eep is valid
     */

    res = arad_pp_eg_encap_access_key_prefix_type_get_unsafe(       
       unit, eep_ndx, &cur_eep_type);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    ARAD_PP_EG_ENCAP_VERIFY_EEP_TYPE_COMPATIBLE_TO_ACCESS_TYPE(
       eep_ndx, eep_type_ndx, cur_eep_type); 

    /* 1. get the eedb entry of type Trill */
    res = arad_pp_eg_encap_access_trill_entry_format_tbl_get_unsafe(
       unit, eep_ndx, &tbl_data); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* set eg encap info type */
    encap_info->entry_type = SOC_PPC_EG_ENCAP_ENTRY_TYPE_TRILL_ENCAP; 

    /* 2. convert from access layer struct to soc layer struct */
    encap_info->entry_val.trill_encap_info.m = tbl_data.m; 
    encap_info->entry_val.trill_encap_info.my_nickname_index = tbl_data.my_nickname_index; 
    encap_info->entry_val.trill_encap_info.nick = tbl_data.nick; 
    encap_info->entry_val.trill_encap_info.outlif_profile = tbl_data.outlif_profile; 
    encap_info->entry_val.trill_encap_info.ll_eep_ndx = tbl_data.next_outlif;
    encap_info->entry_val.trill_encap_info.ll_eep_is_valid = tbl_data.next_outlif_valid;
    encap_info->entry_val.trill_encap_info.remark_profile = tbl_data.remark_profile;

    /* set next_eep is valid */
    if (tbl_data.next_outlif_valid) {
        *next_eep = tbl_data.next_outlif; 
    }  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_encap_trill_entry_get()",0,0);
}

soc_error_t
    arad_pp_eg_trill_entry_set(
       SOC_SAND_IN int                          unit, 
       SOC_SAND_IN uint32                       trill_eep_ndx, 
       SOC_SAND_IN SOC_PPC_EG_ENCAP_TRILL_INFO* trill_encap_info
       ) {

    uint32 res = SOC_SAND_OK; 
    ARAD_PP_EG_ENCAP_ACCESS_TRILL_ENTRY_FORMAT tbl_data; 

    SOC_SAND_INIT_ERROR_DEFINITIONS(0); 

    soc_sand_os_memset(&tbl_data, 0x0, sizeof(tbl_data));

    /* check input */
    ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, trill_eep_ndx, ARAD_PP_EG_ENCAP_LIF_EEP_NDX_OUT_OF_RANGE_ERR, 10, exit);
    res = SOC_PPC_EG_ENCAP_TRILL_INFO_verify(unit, trill_encap_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);

    /* conversion from soc layer struct to access layer struct */
    tbl_data.my_nickname_index = trill_encap_info->my_nickname_index; 
    tbl_data.m = trill_encap_info->m; 
    tbl_data.nick = trill_encap_info->nick; 
    tbl_data.outlif_profile = trill_encap_info->outlif_profile; 
    tbl_data.next_outlif = trill_encap_info->ll_eep_ndx;
    tbl_data.next_outlif_valid = trill_encap_info->ll_eep_is_valid;
    tbl_data.remark_profile = trill_encap_info->remark_profile;

    /* set to HW: call access layer */
    res = arad_pp_eg_encap_access_trill_entry_format_tbl_set_unsafe(unit, trill_eep_ndx, &tbl_data); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_eg_trill_entry_set()", 0, 0); 
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */


