/* $Id: arad_pp_lif.h,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_LIF_INCLUDED__
/* { */
#define __ARAD_PP_LIF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_isem_access.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_LIF_PORT_PROFILE_NDX_MAX                          (7)
#define ARAD_PP_SW_DB_MULTI_SET_VLAN_COMPRESSION_RANGE_NOF_MEMBER (32)
#define ARAD_PP_SW_DB_MULTI_SET_LIF_AC_PROFILE_NOF_MEMBERS(unit)  (SOC_DPP_DEFS_GET(unit, nof_vtt_program_selection_lines) + 1)
#define ARAD_PP_LIF_LIF_PROFILE_MAX                              (15)

/* } */
/*************
 * MACROS    *
 *************/
/* { */
#define ARAD_PP_LIF_LOCAL_INLIF_HANDLE_TO_BASE_LIF_ID(lif_handle)             ( (lif_handle%2==0) ? (lif_handle/2) :  (lif_handle/2 + 1) )
#define ARAD_PP_LIF_LOCAL_INLIF_HANDLE_TO_ADDITIONAL_LIF_ID(lif_handle)       ( (lif_handle%2==0) ? (lif_handle/2  + 1) :  (lif_handle/2) )
#define ARAD_PP_LIF_LOCAL_INLIF_HANDLE_TO_ADDITIONAL_OFFSET(base_lif, ext_lif) ( (ext_lif>base_lif) ? TRUE : FALSE ) /* TRUE = MSHalf ; FALSE = LSHalf*/
#define ARAD_PP_LIF_LOCAL_INLIF_HANDLE_TO_SEM_RESULT_ID(base_lif, ext_lif)    (  SOC_SAND_MIN(base_lif,ext_lif) )

#define ARAD_PP_LIF_SEM_RESULT_ID_TO_LOCAL_INLIF_HANDLE(sem_result_id)    (sem_result_id*2 )

#define ARAD_PP_LIF_PROTECTION_POINTER_NOF_BITS_ARAD             (14)
#define ARAD_PP_LIF_PROTECTION_POINTER_MASK_ARAD                 (SOC_SAND_BITS_MASK((ARAD_PP_LIF_PROTECTION_POINTER_NOF_BITS_ARAD - 1), 0))
#define ARAD_PP_LIF_PROTECTION_POINTER_DEFAULT_VALUE             ((1 << ARAD_PP_LIF_PROTECTION_POINTER_NOF_BITS_ARAD) - (1))


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
  ARAD_PP_L2_LIF_PWE_MAP_RANGE_SET = ARAD_PP_PROC_DESC_BASE_LIF_FIRST,
  ARAD_PP_L2_LIF_PWE_MAP_RANGE_SET_UNSAFE,
  ARAD_PP_L2_LIF_PWE_MAP_RANGE_SET_VERIFY,
  ARAD_PP_L2_LIF_PWE_MAP_RANGE_GET,
  ARAD_PP_L2_LIF_PWE_MAP_RANGE_GET_VERIFY,
  ARAD_PP_L2_LIF_PWE_MAP_RANGE_GET_UNSAFE,
  ARAD_PP_L2_LIF_PWE_ADD,
  ARAD_PP_L2_LIF_PWE_ADD_UNSAFE,
  ARAD_PP_L2_LIF_PWE_ADD_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_PWE_ADD_VERIFY,
  ARAD_PP_L2_LIF_PWE_GET,
  ARAD_PP_L2_LIF_PWE_GET_UNSAFE,
  ARAD_PP_L2_LIF_PWE_GET_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_PWE_GET_VERIFY,
  ARAD_PP_L2_LIF_PWE_REMOVE,
  ARAD_PP_L2_LIF_PWE_REMOVE_UNSAFE,
  ARAD_PP_L2_LIF_PWE_REMOVE_VERIFY,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_ADD,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_ADD_UNSAFE,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_ADD_VERIFY,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_RANGE_VALID_SET_UNSAFE,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_RANGE_VALID_GET_UNSAFE,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_IS_VALID_RANGE,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_REMOVE,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_REMOVE_UNSAFE,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_REMOVE_VERIFY,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_GET,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_GET_UNSAFE,
  ARAD_PP_L2_LIF_VLAN_COMPRESSION_GET_VERIFY,
  ARAD_PP_L2_LIF_AC_MAP_KEY_SET,
  ARAD_PP_L2_LIF_AC_MAP_KEY_SET_UNSAFE,
  ARAD_PP_L2_LIF_AC_MAP_KEY_SET_VERIFY,
  ARAD_PP_L2_LIF_AC_MAP_KEY_GET,
  ARAD_PP_L2_LIF_AC_MAP_KEY_GET_VERIFY,
  ARAD_PP_L2_LIF_AC_MAP_KEY_GET_UNSAFE,
  SOC_PPC_L2_LIF_AC_MP_INFO_SET,
  SOC_PPC_L2_LIF_AC_MP_INFO_SET_UNSAFE,
  SOC_PPC_L2_LIF_AC_MP_INFO_SET_VERIFY,
  SOC_PPC_L2_LIF_AC_MP_INFO_GET,
  SOC_PPC_L2_LIF_AC_MP_INFO_GET_VERIFY,
  SOC_PPC_L2_LIF_AC_MP_INFO_GET_UNSAFE,
  ARAD_PP_L2_LIF_AC_ADD,
  ARAD_PP_L2_LIF_AC_ADD_UNSAFE,
  ARAD_PP_L2_LIF_AC_ADD_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_AC_ADD_VERIFY,
  ARAD_PP_L2_LIF_AC_GET,
  ARAD_PP_L2_LIF_AC_GET_UNSAFE,
  ARAD_PP_L2_LIF_AC_GET_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_AC_GET_VERIFY,
  ARAD_PP_L2_LIF_AC_WITH_COS_ADD,
  ARAD_PP_L2_LIF_AC_WITH_COS_ADD_UNSAFE,
  ARAD_PP_L2_LIF_AC_WITH_COS_ADD_VERIFY,
  ARAD_PP_L2_LIF_AC_WITH_COS_GET,
  ARAD_PP_L2_LIF_AC_WITH_COS_GET_UNSAFE,
  ARAD_PP_L2_LIF_AC_WITH_COS_GET_VERIFY,
  ARAD_PP_L2_LIF_AC_REMOVE,
  ARAD_PP_L2_LIF_AC_REMOVE_UNSAFE,
  ARAD_PP_L2_LIF_AC_REMOVE_VERIFY,
  ARAD_PP_L2_LIF_L2CP_TRAP_SET,
  ARAD_PP_L2_LIF_L2CP_TRAP_SET_UNSAFE,
  ARAD_PP_L2_LIF_L2CP_TRAP_SET_VERIFY,
  ARAD_PP_L2_LIF_L2CP_TRAP_GET,
  ARAD_PP_L2_LIF_L2CP_TRAP_GET_VERIFY,
  ARAD_PP_L2_LIF_L2CP_TRAP_GET_UNSAFE,
  ARAD_PP_L2_LIF_ISID_ADD,
  ARAD_PP_L2_LIF_ISID_ADD_UNSAFE,
  ARAD_PP_L2_LIF_ISID_ADD_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_ISID_ADD_VERIFY,
  ARAD_PP_L2_LIF_ISID_GET,
  ARAD_PP_L2_LIF_ISID_GET_UNSAFE,
  ARAD_PP_L2_LIF_ISID_GET_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_ISID_GET_VERIFY,
  ARAD_PP_L2_LIF_VSI_TO_ISID,
  ARAD_PP_L2_LIF_VSI_TO_ISID_UNSAFE,
  ARAD_PP_L2_LIF_VSI_TO_ISID_VERIFY,
  ARAD_PP_L2_LIF_ISID_REMOVE,
  ARAD_PP_L2_LIF_ISID_REMOVE_UNSAFE,
  ARAD_PP_L2_LIF_ISID_REMOVE_VERIFY,
  ARAD_PP_L2_LIF_GRE_ADD,
  ARAD_PP_L2_LIF_GRE_ADD_UNSAFE,
  ARAD_PP_L2_LIF_GRE_ADD_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_GRE_ADD_VERIFY,
  ARAD_PP_L2_LIF_GRE_GET,
  ARAD_PP_L2_LIF_GRE_GET_UNSAFE,
  ARAD_PP_L2_LIF_GRE_GET_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_GRE_GET_VERIFY,
  ARAD_PP_L2_LIF_GRE_REMOVE,
  ARAD_PP_L2_LIF_GRE_REMOVE_UNSAFE,
  ARAD_PP_L2_LIF_GRE_REMOVE_VERIFY,
  ARAD_PP_L2_LIF_VXLAN_ADD,
  ARAD_PP_L2_LIF_VXLAN_ADD_UNSAFE,
  ARAD_PP_L2_LIF_VXLAN_ADD_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_VXLAN_ADD_VERIFY,
  ARAD_PP_L2_LIF_VXLAN_GET,
  ARAD_PP_L2_LIF_VXLAN_GET_UNSAFE,
  ARAD_PP_L2_LIF_VXLAN_GET_INTERNAL_UNSAFE,
  ARAD_PP_L2_LIF_VXLAN_GET_VERIFY,
  ARAD_PP_L2_LIF_VXLAN_REMOVE,
  ARAD_PP_L2_LIF_VXLAN_REMOVE_UNSAFE,
  ARAD_PP_L2_LIF_VXLAN_REMOVE_VERIFY,
  ARAD_PP_L2_LIF_DEFAULT_FRWRD_INFO_SET,
  ARAD_PP_L2_LIF_DEFAULT_FRWRD_INFO_SET_UNSAFE,
  ARAD_PP_L2_LIF_DEFAULT_FRWRD_INFO_SET_VERIFY,
  ARAD_PP_L2_LIF_DEFAULT_FRWRD_INFO_GET,
  ARAD_PP_L2_LIF_DEFAULT_FRWRD_INFO_GET_UNSAFE,
  ARAD_PP_L2_LIF_DEFAULT_FRWRD_INFO_GET_VERIFY,
  ARAD_PP_LIF_GET_PROCS_PTR,
  ARAD_PP_LIF_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_PROCEDURE_DESC_LAST
} ARAD_PP_LIF_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LIF_SUCCESS_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LIF_FIRST,
  ARAD_PP_LIF_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_VLAN_FORMAT_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_KEY_MAPPING_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_KEY_MAPPING_ENTRY_EXISTS_ERR,
  ARAD_PP_LIF_KEY_MAPPING_NOT_SIMPLE_VLAN_TRANSLATION_ERR,
  ARAD_PP_LIF_HANDLE_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_DEFAULT_FRWD_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ORIENTATION_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_TPID_PROFILE_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_COS_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_SERVICE_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_VLAN_DOMAIN_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_VID_TAG_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_VLAN_RANGE_FIRST_OVER_LAST_ERR,
  ARAD_PP_LIF_VLAN_RANGE_VLAN_RANGE_ALREADY_DEFINED_ERR,
  ARAD_PP_LIF_KEY_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_RAW_KEY_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_LEARN_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_EDIT_PCP_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_L2CP_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_DA_MAC_ADDRESS_LSB_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_OPCODE_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_NOF_LIF_ENTRIES_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ISID_DOMAIN_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_LIF_VSI_IS_NOT_P2P_ERR,
  ARAD_PP_LIF_ILLEGAL_KEY_MAPPING_ERR,
  ARAD_PP_LIF_INNER_EXISTS_OUTER_NOT_ERR,
  ARAD_PP_LIF_VSI_ASSIGNMENT_MODE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_LIF_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PWE_TERM_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PWE_ACTION_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_DEFAULT_FRWRD_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PORT_DA_NOT_FOUND_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PROTECTION_POINTER_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PROTECTION_PASS_VAL_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PWE_MPLS_TUNNEL_MODEL_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_AC_P2P_DEFAULT_FRWRD_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_AC_MP_DEFAULT_FRWRD_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PWE_P2P_DEFAULT_FRWRD_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PWE_MP_DEFAULT_FRWRD_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_AC_P2P_ORIENTATION_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_PWE_P2P_ORIENTATION_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_DA_TYPE_OUT_OF_RANGE_ERR,  
  ARAD_PP_LIF_VC_LABEL_KEY_ILLEGAL_ERR,
  ARAD_PP_LIF_ADD_VSI_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_OFFSET_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_GRE_KEY_VPN_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_GRE_KEY_VSI_NOT_EQ_LIF_ERR,
  ARAD_PP_LIF_VXLAN_KEY_VPN_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_VXLAN_KEY_VSI_NOT_EQ_LIF_ERR,
  ARAD_PP_LIF_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_NETWORK_GROUP_OUT_OF_RANGE_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_ERR_LAST
} ARAD_PP_LIF_ERR;

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
  arad_pp_lif_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_pwe_map_range_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set MPLS labels that may be used as in-vc-labels
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_IN_VC_RANGE                      *in_vc_range -
 *     First and Last MPLS Labels to be used as ingress VC
 *     labels. T20E Range Size: 0-896K
 * REMARKS:
 *   - T20E: Set the MPLS labels that may be used as
 *   in-vc-labels - Arad-B: Ignored
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_pwe_map_range_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_IN_VC_RANGE                      *in_vc_range
  );

uint32
  arad_pp_l2_lif_pwe_map_range_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_IN_VC_RANGE                      *in_vc_range
  );

uint32
  arad_pp_l2_lif_pwe_map_range_get_verify(
    SOC_SAND_IN  int                                     unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_l2_lif_pwe_map_range_set_unsafe" API.
 *     Refer to "arad_pp_l2_lif_pwe_map_range_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_l2_lif_pwe_map_range_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_PPC_L2_LIF_IN_VC_RANGE                      *in_vc_range
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_pwe_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable an in VC label as a PWE handle, and set the PWE
 *   attributes
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label -
 *     The In-MPLS label that is attached to the PWE
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
 *     LIF table index
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_INFO                     *pwe_info -
 *     PWE attributes. VSID, Out-VC, ...
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
 *     Exact Match table
 * REMARKS:
 *   T20E: The lif_index is ignored. Must be set to
 *   T20E_LIF_DEFAULT_PWE_LIF. Arad-B: In multipoint services
 *   (pwe_info.service_type=MP), the default forwarding is
 *   set per VSI and NOT per PWE interface using this API.
 *   When Adding a PWE to the system, the user is expected
 *   to:- Find the Egress Editing pointer (EEP) that points
 *   to the PWE's tunnel and Link-Layer encapsulation.- Add a
 *   FEC, with the PWEs TM destination and the EEP that
 *   points to the tunnel. This FEC is used for learning.-
 *   For multipoint services, add the PWE to the multicast
 *   group that commits the flooding of the unknown MAC
 *   addresses in the VSI.- Call this function, when the
 *   above FEC is part of the learning information.
 *   ARAD: in case device is bring up with mpls indexed support
 *   in_vc_label consist both the label value and label index.
 *   to set in_vc_label call SOC_PPC_VC_LABEL_INDEXED_SET.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_pwe_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO        *pwe_additional_info,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_INFO                     *pwe_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_l2_lif_pwe_add_internal_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label,
    SOC_SAND_IN  uint8                                 ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO        *pwe_additional_info,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_INFO                     *pwe_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_l2_lif_pwe_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_INFO                     *pwe_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_pwe_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get PWE attributes and the LIF-index according to
 *   in_vc_label.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label -
 *     The In-MPLS label that is attached to the PWE
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              lif_index -
 *     The index of the PWE in the LIF table
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_PWE_INFO                     *pwe_info -
 *     PWE incoming attributes. VSID, Out-VC, ...
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: The entry was found, 'pwe_info' is validFALSE: The
 *     entry was not found, 'pwe_info' is invalid
 * REMARKS:
 *   T20E: The lif_index is ignored. Arad-B: In multipoint
 *   services, the default forwarding is according to the
 *   VSI. When Adding a PWE to the system, the user is
 *   expected to:- Find the Egress Editing pointer (EEP) that
 *   points to the PWE's tunnel and Link-Layer
 *   encapsulation.- Add a FEC, with the PWEs TM destination
 *   and the EEP that points to the tunnel. This FEC is used
 *   for learning.- For multipoint services, add the PWE to
 *   the multicast group that commits the flooding of the
 *   unknown MAC addresses in the VSI.- Call this function,
 *   when the above FEC is part of the learning
 *   information. In the T20E driver, the AC set API
 *   configures the VSI attributes as well.
 *   ARAD: in case device is bring up with mpls indexed support
 *   in_vc_label consist both the label value and label index.
 *   to set in_vc_label call SOC_PPC_VC_LABEL_INDEXED_SET.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_pwe_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO            *pwe_additional_info,
    SOC_SAND_OUT SOC_PPC_L2_LIF_PWE_INFO                     *pwe_info,
    SOC_SAND_OUT uint8                                 *found
  );


/*
 * get PWE according to key or lif-id
 */
uint32
  arad_pp_l2_lif_pwe_get_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                            in_vc_label,
    SOC_SAND_IN  uint8                                     ignore_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                                  *lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO              *pwe_additional_info,
    SOC_SAND_OUT SOC_PPC_L2_LIF_PWE_INFO                         *pwe_info,
    SOC_SAND_OUT uint8                                     *found
  );


uint32
  arad_pp_l2_lif_pwe_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_pwe_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remvoe the in-VC-label
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label -
 *     The In-MPLS label that is attached to the PWE
 *   SOC_SAND_IN  uint8                                remove_from_sem -
 *     Is set when entry should be removed from both lif table and SEM
 *   SOC_SAND_IN  uint8                                is_gal -
 *     Indication if lif is GAL entry   
 *   SOC_SAND_OUT uint32                                  *lif_index -
 *     LIF table index
 * REMARKS:
 *   - Unbind the mapping of the In-VC-Label to the LIF table
 *   from the SEM table- Invalidate the 'lif_index' entry in
 *   the LIF table- The 'lif_index' is returned to the user
 *   to enable management of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_pwe_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label,
    SOC_SAND_IN  uint8                                remove_from_sem,
    SOC_SAND_IN  uint8                                is_gal,
    SOC_SAND_OUT uint32                                  *lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_ADDITIONAL_INFO        *pwe_additional_info
  );

uint32
  arad_pp_l2_lif_pwe_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        in_vc_label,
    SOC_SAND_IN  uint8                                remove_from_sem
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_vlan_compression_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable VLAN compression when adding Attachment Circuits.
 *   Enables defining an AC according to VLAN domain and a
 *   range of VLANs, and not only according to port*VLAN
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY                   *vlan_range_key -
 *     VLAN Range Key. When AC is added with the first VLAN in
 *     the range, the AC is defined for all the VLAN in the
 *     range. When the user adds an AC with VID in the middle of
 *     the range, the driver returns ERROR.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
 *     VLAN ranges table
 * REMARKS:
 *   - Arad-B Only. In the T20E, there are no VLAN ranges,
 *   and an AC is specified per port * VLAN [* C-VLAN]- In
 *   the device, there is a global ranges table, and per port
 *   there is an option to use or not use the range. The
 *   driver manages the global table internally; therefore,
 *   the driver actually does the following:Upon Enable: Add
 *   the VLAN range mapping, unless it already exists, and
 *   enable for the ports in the VLAN domain. Upon Disable:
 *   Remove the VLAN range mapping, unless utilized by other
 *   ports, and the ports mapping in the VLAN domain.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_vlan_compression_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY                   *vlan_range_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_l2_lif_vlan_compression_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY                   *vlan_range_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_vlan_compression_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a VLAN range from the compressed VLAN ranges
 *   database
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY                   *vlan_range_key -
 *     VLAN Range Key.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_vlan_compression_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY                   *vlan_range_key
  );

uint32
  arad_pp_l2_lif_vlan_compression_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY                   *vlan_range_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_vlan_compression_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get all compressed VLAN ranges of a port
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                 local_port_ndx -
 *     Identifier of the port to access.
 *   SOC_SAND_OUT  SOC_PPC_L2_PORT_VLAN_RANGE_INFO       *vlan_range_info -
 *     VLAN Range Info.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_vlan_compression_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  int                                        core_id,
    SOC_SAND_IN  uint32                                     local_port_ndx,
    SOC_SAND_OUT SOC_PPC_L2_PORT_VLAN_RANGE_INFO            *vlan_range_info
  );

uint32
  arad_pp_l2_lif_vlan_compression_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  uint32                                     local_port_ndx,
    SOC_SAND_OUT SOC_PPC_L2_PORT_VLAN_RANGE_INFO            *vlan_range_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_ac_map_key_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the packets attributes (in-port, VIDs) to consider
 *   when associating an incoming packet to in-AC.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER             *qual_key -
 *     The qualifier used to associate the packet into
 *     AC-key Includes port-profile and output from packet parsing
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE              key_mapping -
 *     AC map key, can be any of the following:- port- port x
 *     Vlan (outer)- port x Vlan x Vlan
 * REMARKS:
 *   - use SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY as
 *   vlan_format_ndx to make a decision for ALL VLAN formats
 *   at once.- In order to assign a port profile to a local
 *   port, use: 1. ARAD_PP_port_info_set(local_port_ndx,
 *   port_profile)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_map_key_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER             *qual_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE              key_mapping
  );

uint32
  arad_pp_l2_lif_ac_map_key_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER                 *qual_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE                  key_mapping
  );

uint32
  arad_pp_l2_lif_ac_map_key_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER                 *qual_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_l2_lif_ac_map_key_set" API.
 *     Refer to "arad_pp_l2_lif_ac_map_key_set" API for details.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_map_key_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER             *qual_key,
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_MAP_KEY_TYPE              *key_mapping
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_ac_mp_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an incoming Attachment-Circuit (port x VLAN x VLAN)
 *   and MD-level, and to determine the action to perform. If
 *   the MP is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                       lif_ndx -
 *     LIF table index
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MP_INFO            *info -
 *     MP configuration information.
 * REMARKS:
 *   Should only be used if the MP is down MEP or MIP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_mp_info_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                       lif_ndx,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MP_INFO            *info
  );

uint32
  arad_pp_l2_lif_ac_mp_info_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                       lif_ndx,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MP_INFO            *info
  );

uint32
  arad_pp_l2_lif_ac_mp_info_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                       lif_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_l2_lif_ac_mp_info_set_unsafe" API.
 *     Refer to "arad_pp_l2_lif_ac_mp_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_mp_info_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                       lif_ndx,
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_MP_INFO            *info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_ac_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable an Attachment Circuit, and set the AC attributes.
 *   Bind the ac-key to the LIF, and set the LIF attributes
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key -
 *     AC key identifies the AC and how to map a packet to it
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
 *     Logical Interfaces table index. The LIF index is also the
 *     AC ID that the packet is learnt with.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_INFO                      *ac_info -
 *     AC Attributes. VSID, Ingress Editing attributes, ...
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space (in the
 *     Exact Match table)
 * REMARKS:
 *   - T20E: The VLAN domain is equal to the port ID- T20E:
 *   The concept of VLAN ranges is not applicable- Arad-B:
 *   In multipoint services, the default forwarding is
 *   according to the VSI- QinQ CEP port: o The VSI is the
 *   S-VIDo 'vid_ndx' is the C-VID tago This API registers
 *   C-VID * port to service o The egress editing module will
 *   map the VSI to the appended S-VIDo No need in AC
 *   learning, because the S-VID is the service, and the
 *   S-VID * port is sufficient for the editing.o Ingress
 *   editing may be used to append / replace / remove C-VID-
 *   MIM: This function is valid for customer facing ACs. In
 *   this case, the VSI holds the I-SID as the EEI- VPLS: The
 *   resolved VSI is the core VSI, and may be updated
 *   according to PWE- LIF index: Should be equal to
 *   'out_ac_ndx' when configuring the Out-AC via
 *   soc_ppd_eg_ac_info_set()When Adding an AC to the system, the
 *   user is expected to:- Set the Out-AC Egress editing.-
 *   For multipoint services, when egress editing is
 *   according to AC, add the AC as a member in the flooding
 *   multicast ID. Otherwise, add the source port- Call this
 *   function
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_INFO                      *ac_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );


uint32
  arad_pp_l2_lif_ac_add_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                           *ac_key,
    SOC_SAND_IN  uint8                                     ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                                  lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_INFO                          *ac_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success
  );


uint32
  arad_pp_l2_lif_ac_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_INFO                      *ac_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_ac_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an Attachment Circuit, according to AC key.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key -
 *     AC key identifies the AC and how to map a packet to it
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     The Logical Interfaces table index that the AC was found
 *     in.
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_AC_INFO                      *ac_info -
 *     AC Attributes. VSID, Ingress Editing attributes, ...
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: The entry was found, 'ac_info' is validFALSE: The
 *     entry was not found, 'ac_info' is invalid
 * REMARKS:
 *   Get the AC info and index, according to the AC Key
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_INFO                      *ac_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_l2_lif_ac_get_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                           *ac_key,
    SOC_SAND_IN  uint8                                     ignore_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                                  *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_INFO                          *ac_info,
    SOC_SAND_OUT uint8                                     *found
  );

uint32
  arad_pp_l2_lif_ac_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_ac_with_cos_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add Attachment Circuit (AC) group. Enable defining ACs,
 *   according to Quality of Service attributes, on top of
 *   the usual port * VID [*VID] identification
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key -
 *     AC key that identifies the ACs group.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              base_lif_index -
 *     Logical Interfaces table base index. The AC occupies
 *     entries base_lif_index to (base_lif_index + ACs-group
 *     size - 1) in the Logical Interfaces table
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_GROUP_INFO                *acs_group_info -
 *     ACs Group Attributes. Contain an array of ACs, and
 *     pointer to the mapping attributes between the QoS
 *     information and the ACs in the group.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
 *     Exact Match table
 * REMARKS:
 *   - See remarks of soc_ppd_l2_lif_ac_add()- Map VLAN Domain
 *   and VID(s) to group of AC Logical interfaces, and set
 *   ACs information. The QoS attribute type and mapping is
 *   according to the opcode_id. - 'nof_lif_entries' is only
 *   used for driver validation. The Number of LIF entries is
 *   derived from the 'opcode_id'.- The opcode types are
 *   configured via the API soc_ppd_lif_cos_opcode_types_set
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_with_cos_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              base_lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_GROUP_INFO                *acs_group_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_l2_lif_ac_with_cos_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              base_lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_GROUP_INFO                *acs_group_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_ac_with_cos_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get Attachment Circuit (AC) group attributes
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key -
 *     AC key identifies the ACs group and how to map a packet
 *     to it. Out of the ACs group, the specific AC is
 *     according to the QoS params and the Op-code.
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *base_lif_index -
 *     Logical Interfaces table base index. The LIF index of the
 *     first AC in the ACs array.
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_AC_GROUP_INFO                *acs_group_info -
 *     ACs Group Attributes. Contain an array of ACs, and
 *     pointer to the mapping attributes between the QoS
 *     information and the ACs in the group.
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: The entry was found-'acs_group_info' is
 *     validFALSE: The entry was not found-'acs_group_info' is
 *     invalid
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_with_cos_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *base_lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_GROUP_INFO                *acs_group_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_l2_lif_ac_with_cos_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_ac_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an Attachment Circuit
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key -
 *     AC key identifies
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Logical Interfaces table index
 * REMARKS:
 *   - Unbind the mapping of the AC to the LIF table from the
 *   SEM table- Invalidate the 'lif_index' entry in the LIF
 *   table- The 'lif_index' is returned to the user to enable
 *   management of the LIF table- Revert both
 *   soc_ppd_l2_lif_ac_with_cos_add() and soc_ppd_l2_lif_ac_add(). In
 *   this case, the driver invalidate all the lif table
 *   entries of the LIF group, and the returned 'lif_index'
 *   is the lif_base_index
 *   - For ARAD, in case lif_index = NULL, then remove only key without
 *   any modification on the LIF settings.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_ac_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index
  );

uint32
  arad_pp_l2_lif_ac_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY                       *ac_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_l2cp_trap_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets Trap information for Layer 2 control protocol
 *   frames. Packet is an MEF layer 2 control protocol
 *   service frame When DA matches 01-80-c2-00-00-XX where XX
 *   = 8'b00xx_xxxx.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_KEY                     *l2cp_key -
 *     Reserved Multicast key including Destination MAC address
 *     lsb (the msb are constant) and l2cp profile (set
 *     according to in-AC see soc_ppd_l2_lif_ac_add()).
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE             handle_type -
 *     How to handle the frame with the l2cp_key, drop,
 *     peer(trap), normal or tunneled (transparent)
 * REMARKS:
 *   - Arad-B only.- This applied only for L2 AC LIF.- in
 *   case L2CP packet to be dropped/trapped(peer) the packet
 *   is assigned Trap code
 *   SOC_PPC_TRAP_CODE_L2CP_DROP/SOC_PPC_TRAP_CODE_L2CP_PEER. - In
 *   order to set the forwarding/snooping action use
 *   soc_ppd_trap_frwrd_profile_info_set()/soc_ppd_trap_snoop_profile_info_set
 *   with the above trap codes
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_l2cp_trap_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_KEY                     *l2cp_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE             handle_type
  );

uint32
  arad_pp_l2_lif_l2cp_trap_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_KEY                     *l2cp_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE             handle_type
  );

uint32
  arad_pp_l2_lif_l2cp_trap_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_KEY                     *l2cp_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_l2_lif_l2cp_trap_set_unsafe" API.
 *     Refer to "arad_pp_l2_lif_l2cp_trap_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_l2_lif_l2cp_trap_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_KEY                     *l2cp_key,
    SOC_SAND_OUT SOC_PPC_L2_LIF_L2CP_HANDLE_TYPE             *handle_type
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_isid_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map an I-SID to VSI and vice versa. Set I-SID related
 *   attributes. Relevant when the packet arrives from the
 *   PBP
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                          vsi_ndx -
 *     System VSID.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key -
 *     I-SID, as taken from the I-TAG, and optionally I-SID
 *     domain.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
 *     Logical Interfaces Table Entry. I-SID mapping to VSI is
 *     done in the LIF table. Arad-B Range: 0-16K.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_INFO                        *isid_info -
 *     VSID and I-SID related attributes
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in
 *     Exact Match
 * REMARKS:
 *   The mapping from the VSI to I-SID is done in the VSID
 *   table. The mapping from the I-SID to VSI is done in the
 *   LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_isid_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                          vsi_ndx,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_INFO                    *isid_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_l2_lif_isid_add_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                         vsi_ndx,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                    *isid_key,
    SOC_SAND_IN  uint8                                      ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                             lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_INFO                   *isid_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                   *success
  );

uint32
  arad_pp_l2_lif_isid_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                          vsi_ndx,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_INFO                    *isid_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_isid_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map an I-SID to VSI and vice versa. Set I-SID related
 *   attributes. Relevant when the packet arrives from the
 *   PBP
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key -
 *     I-SID, as taken from the I-TAG, and optionally I-SID
 *     domain.
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Logical Interfaces Table Entry
 *   SOC_SAND_OUT SOC_PPC_SYS_VSI_ID                          *vsi_index -
 *     System VSID.
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_ISID_INFO                    *isid_info -
 *     VSID and I-SID related attributes
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: ISID was foundFALSE: ISID was not found
 * REMARKS:
 *   The ISID mapping information is taken from the LIF
 *   table.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_isid_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_SYS_VSI_ID                          *vsi_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_ISID_INFO                    *isid_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_l2_lif_isid_get_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                    *isid_key,
    SOC_SAND_IN  uint8                                      ignore_key, 
    SOC_SAND_OUT SOC_PPC_LIF_ID                             *lif_index,
    SOC_SAND_OUT SOC_PPC_SYS_VSI_ID                         *vsi_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_ISID_INFO                   *isid_info,
    SOC_SAND_OUT uint8                                      *found
  );

uint32
  arad_pp_l2_lif_isid_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_vsi_to_isid_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the I-SID related to the VSI
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                          vsi_ndx -
 *     System VSID.
 *   SOC_SAND_OUT SOC_SAND_PP_ISID                              *isid_id -
 *     I-SID related to the VSID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_vsi_to_isid_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                              vsi_ndx,
    SOC_SAND_OUT SOC_SAND_PP_ISID                                  *isid_id
  );

uint32
  arad_pp_l2_lif_vsi_to_isid_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                              vsi_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_isid_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Unbind an I-SID from VSI and vice versa
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_SYS_VSI_ID                          vsi_ndx -
 *     System VSID.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key -
 *     I-SID, as taken from the I-TAG, and optionally I-SID
 *     domain.
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Logical Interfaces Table Entry. I-SID mapping to VSI is
 *     done in the LIF table. Arad-B Range: 0-16KT20E Range:
 *     0-128K
 * REMARKS:
 *   - Unbind the mapping from the VSI to the I-SID in the
 *   VSI table- Unbind the mapping of the ISID-key to the LIF
 *   table from the SEM table- Invalidate the 'lif_index'
 *   entry in the LIF table- The 'lif_index' is returned to
 *   the user to enable management of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_isid_remove_unsafe(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                     *isid_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index
  );

uint32
  arad_pp_l2_lif_isid_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY                   *isid_key
  );


/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_gre_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map an GRE-KEY to VSI and vice versa. Set GRE-KEY related
 *   attributes. Relevant when the packet arrives from the
 *   PBP
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key -
 *     GRE-KEY, as taken from the I-TAG, and optionally GRE-KEY
 *     domain.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
 *     Logical Interfaces Table Entry. GRE-KEY mapping to VSI is
 *     done in the LIF table. Arad-B Range: 0-16K.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_INFO                        *gre_info -
 *     VSID and GRE-KEY related attributes
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in
 *     Exact Match
 * REMARKS:
 *   The mapping from the VSI to GRE-KEY is done in the VSID
 *   table. The mapping from the GRE-KEY to VSI is done in the
 *   LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_gre_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_INFO                    *gre_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_l2_lif_gre_add_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                    *gre_key,
    SOC_SAND_IN  uint8                                      ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                             lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_INFO                   *gre_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                   *success
  );

uint32
  arad_pp_l2_lif_gre_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_INFO                    *gre_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_gre_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map an GRE-KEY to VSI and vice versa. Set GRE-KEY related
 *   attributes. Relevant when the packet arrives from the
 *   PBP
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key -
 *     GRE-KEY, as taken from the I-TAG, and optionally GRE-KEY
 *     domain.
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Logical Interfaces Table Entry
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_GRE_INFO                    *gre_info -
 *     VSID and GRE-KEY related attributes
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: GRE was foundFALSE: GRE was not found
 * REMARKS:
 *   The GRE mapping information is taken from the LIF
 *   table.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_gre_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_GRE_INFO                    *gre_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_l2_lif_gre_get_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                    *gre_key,
    SOC_SAND_IN  uint8                                      ignore_key, 
    SOC_SAND_OUT SOC_PPC_LIF_ID                             *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_GRE_INFO                   *gre_info,
    SOC_SAND_OUT uint8                                      *found
  );

uint32
  arad_pp_l2_lif_gre_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_gre_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Unbind an GRE-KEY from VSI and vice versa
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key -
 *     GRE-KEY, as taken from the I-TAG, and optionally GRE-KEY
 *     domain.
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Logical Interfaces Table Entry. GRE-KEY mapping to VSI is
 *     done in the LIF table. Arad-B Range: 0-16KT20E Range:
 *     0-128K
 * REMARKS:
 *   - Unbind the mapping from the VSI to the GRE-KEY in the
 *   VSI table- Unbind the mapping of the GRE-key to the LIF
 *   table from the SEM table- Invalidate the 'lif_index'
 *   entry in the LIF table- The 'lif_index' is returned to
 *   the user to enable management of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_gre_remove_unsafe(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                     *gre_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index
  );

uint32
  arad_pp_l2_lif_gre_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY                   *gre_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_vxlan_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map an VXLAN-KEY to VSI and vice versa. Set VXLAN-KEY related
 *   attributes. Relevant when the packet arrives from the
 *   PBP
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key -
 *     VXLAN-KEY, as taken from the I-TAG, and optionally VXLAN-KEY
 *     domain.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
 *     Logical Interfaces Table Entry. VXLAN-KEY mapping to VSI is
 *     done in the LIF table. Arad-B Range: 0-16K.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_INFO                        *vxlan_info -
 *     VSID and VXLAN-KEY related attributes
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                          *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in
 *     Exact Match
 * REMARKS:
 *   The mapping from the VSI to VXLAN-KEY is done in the VSID
 *   table. The mapping from the VXLAN-KEY to VSI is done in the
 *   LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_vxlan_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_INFO                    *vxlan_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_l2_lif_vxlan_add_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                    *vxlan_key,
    SOC_SAND_IN  uint8                                      ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                             lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_INFO                   *vxlan_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                   *success
  );

uint32
  arad_pp_l2_lif_vxlan_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_INFO                    *vxlan_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_vxlan_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map an VXLAN-KEY to VSI and vice versa. Set VXLAN-KEY related
 *   attributes. Relevant when the packet arrives from the
 *   PBP
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key -
 *     VXLAN-KEY, as taken from the I-TAG, and optionally VXLAN-KEY
 *     domain.
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Logical Interfaces Table Entry
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_VXLAN_INFO                    *vxlan_info -
 *     VSID and VXLAN-KEY related attributes
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: VXLAN was foundFALSE: VXLAN was not found
 * REMARKS:
 *   The VXLAN mapping information is taken from the LIF
 *   table.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_vxlan_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_VXLAN_INFO                    *vxlan_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_l2_lif_vxlan_get_internal_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                    *vxlan_key,
    SOC_SAND_IN  uint8                                      ignore_key, 
    SOC_SAND_OUT SOC_PPC_LIF_ID                             *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_VXLAN_INFO                   *vxlan_info,
    SOC_SAND_OUT uint8                                      *found
  );

uint32
  arad_pp_l2_lif_vxlan_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_vxlan_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Unbind an VXLAN-KEY from VSI and vice versa
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key -
 *     VXLAN-KEY, as taken from the I-TAG, and optionally VXLAN-KEY
 *     domain.
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Logical Interfaces Table Entry. VXLAN-KEY mapping to VSI is
 *     done in the LIF table. Arad-B Range: 0-16KT20E Range:
 *     0-128K
 * REMARKS:
 *   - Unbind the mapping from the VSI to the VXLAN-KEY in the
 *   VSI table- Unbind the mapping of the VXLAN-key to the LIF
 *   table from the SEM table- Invalidate the 'lif_index'
 *   entry in the LIF table- The 'lif_index' is returned to
 *   the user to enable management of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_vxlan_remove_unsafe(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                     *vxlan_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index
  );

uint32
  arad_pp_l2_lif_vxlan_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY                   *vxlan_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_default_frwrd_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the action info (forwarding/snooping) to assign
 *   for packets upon failure lookup in the MACT (MACT
 *   default forwarding).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY           *dflt_frwrd_key -
 *     Set of parameters that default forwarding may be
 *     assigned to. Includes Port-DA-not-found profile,
 *     LIF-default-profile and DA-type (UC/MC/BC).
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION        *action_info -
 *     Default Action information including snoop/forwarding action and add_vsi base.
 * REMARKS:
 *  Used to set default
 *   forwarding for LIF/Profile.- By arad_pp_port_info_set() and
 *   arad_pp_l2_lif_xx_info_set() the user
 *   supplies the default forwarding profiles for the LIF and Port; 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_l2_lif_default_frwrd_info_set_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY        *dflt_frwrd_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION     *action_info
  );

uint32
  arad_pp_l2_lif_default_frwrd_info_set_verify(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY        *dflt_frwrd_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION     *action_info
  );

uint32
  arad_pp_l2_lif_default_frwrd_info_get_verify(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY        *dflt_frwrd_key    
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_l2_lif_default_frwrd_info_set" API.
 *     Refer to "arad_pp_l2_lif_default_frwrd_info_set" API for details.
*********************************************************************/
uint32
  arad_pp_l2_lif_default_frwrd_info_get_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY        *dflt_frwrd_key,
    SOC_SAND_OUT SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION     *action_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_extender_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map Name-Space x E-CID x C-VID to a Port-Extender In-LIF and VSI.
 * INPUT:
 *   SOC_SAND_IN  int                                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_KEY           *extender_key -
 *     Port Extender key - ame-Space x E-CID x C-VID
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                        lif_index -
 *     Logical Interfaces Table Entry. Port-Extender
 *     mapping to VSI is done in the LIF table.
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_INFO          *extender_info -
 *     VSID and I-SID related attributes
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is
 *     no space in Exact Match
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_pp_l2_lif_extender_add_unsafe(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_KEY        *extender_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                     lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_INFO       *extender_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE           *success);


uint32 arad_pp_l2_lif_extender_add_internal_unsafe(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_KEY        *extender_key,
    SOC_SAND_IN  uint8                              ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                     lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_INFO       *extender_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE           *success);


uint32 arad_pp_l2_lif_extender_add_verify(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_KEY        *extender_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                     lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_INFO       *extender_info);


/*********************************************************************
* NAME:
 *   arad_pp_l2_lif_extender_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get a Port Extender HW info, according to Port Extender HW key.
 * INPUT:
 *   int                               unit -
 *     Identifier of the device to access.
 *   SOC_PPC_L2_LIF_EXTENDER_KEY       *extender_key -
 *     Port Extender key identifies the Port Extender and how to map a packet to it
 *   SOC_PPC_LIF_ID                    *lif_index -
 *     The Logical Interfaces table index that the Port Extender was found  in.
 *   SOC_PPC_L2_LIF_EXTENDER_INFO      *extender_info -
 *     Port Extender Attributes. VSID, Ingress Editing attributes, etc
 *   uint8                             *found -
 *     TRUE:  The entry was found, 'extender_info' is valid
 *     FALSE: The entry was not found, 'extender_info' is invalid
 * REMARKS:
 *   Get the Port Extender info and index, according to the HW Key
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_pp_l2_lif_extender_get(
    int                                unit,
    SOC_PPC_L2_LIF_EXTENDER_KEY        *extender_key,
    SOC_PPC_LIF_ID                     *lif_index,
    SOC_PPC_L2_LIF_EXTENDER_INFO       *extender_info,
    uint8                              *found);

soc_error_t arad_pp_l2_lif_extender_remove(
    int                                     unit,
    SOC_PPC_L2_LIF_EXTENDER_KEY             *extender_key,
    int                                     *lif_index);




uint32
  arad_pp_l2_lif_ac_key_parse_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY             *isem_key,
    SOC_SAND_OUT SOC_PPC_L2_LIF_AC_KEY               *ac_key
  );

uint32
  arad_pp_l2_lif_pwe_key_parse_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY              *isem_key,
    SOC_SAND_OUT SOC_SAND_PP_MPLS_LABEL               *pwe
  );

uint32
  arad_pp_l2_lif_isid_key_parse_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY              *isem_key,
    SOC_SAND_OUT SOC_PPC_L2_LIF_ISID_KEY              *isid_key
  );

soc_error_t
arad_pp_lif_additional_data_set(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  int lif_id, 
        SOC_SAND_IN  uint8 is_ingress,
        SOC_SAND_IN  uint64 data
   );


soc_error_t
arad_pp_lif_additional_data_get(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  int lif_id, 
        SOC_SAND_IN  uint8 is_ingress,
        SOC_SAND_OUT uint64 *data
   );

 
int 
arad_pp_lif_index_to_sem_result( 
    int                    unit,
    SOC_PPC_LIF_ID         base_lif_id,
    SOC_PPC_LIF_ID         ext_lif_id,
    uint32                 *sem_result
   ); 

uint32 
arad_pp_sem_result_to_lif_index( 
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 sem_result
   );

soc_error_t
arad_pp_lif_is_wide_entry(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  int lif_id, 
        SOC_SAND_IN  uint8 is_ingress,
        SOC_SAND_OUT uint8 *is_wide_entry,
        SOC_SAND_OUT uint8 *ext_type);

void
arad_pp_l2_lif_convert_protection_fields_to_hw(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  uint32 protection_pointer,
        SOC_SAND_IN  uint8  protection_pass_value,
        SOC_SAND_OUT uint32 *hw_protection_pointer,
        SOC_SAND_OUT uint32 *hw_protection_path);


void
arad_pp_l2_lif_convert_protection_fields_from_hw(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  uint32 hw_protection_pointer,
        SOC_SAND_IN  uint32 hw_protection_path,
        SOC_SAND_OUT uint32 *protection_pointer,
        SOC_SAND_OUT uint8  *protection_pass_value);

/*********************************************************************
* NAME:
 *   arad_pp_lif_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_lif module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_lif_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_lif_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_lif module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_lif_get_errs_ptr(void);

uint32
  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY_QUALIFIER *info
  );

uint32
  SOC_PPC_L2_LIF_IN_VC_RANGE_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_IN_VC_RANGE *info
  );

uint32
  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FORWARDING_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_PWE_LEARN_RECORD_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_LEARN_RECORD *info
  );

uint32
  SOC_PPC_L2_LIF_PWE_INFO_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_PWE_INFO *info
  );

uint32
  SOC_PPC_L2_VLAN_RANGE_KEY_verify(
    SOC_SAND_IN  SOC_PPC_L2_VLAN_RANGE_KEY *info
  );

uint32
  SOC_PPC_L2_LIF_AC_KEY_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_KEY *info
  );

uint32
  SOC_PPC_L2_LIF_AC_LEARN_RECORD_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_LEARN_RECORD *info
  );

uint32
  SOC_PPC_L2_LIF_AC_ING_EDIT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_ING_EDIT_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_AC_INFO_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_AC_MP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_MP_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_L2CP_KEY_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_L2CP_KEY *info
  );

uint32
  SOC_PPC_L2_LIF_AC_GROUP_INFO_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_AC_GROUP_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_ISID_KEY_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_KEY *info
  );

uint32
  SOC_PPC_L2_LIF_ISID_INFO_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_ISID_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_TRILL_INFO_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_GRE_KEY_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_KEY *info
  );

uint32
  SOC_PPC_L2_LIF_GRE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_GRE_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_VXLAN_KEY_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_KEY *info
  );

uint32
  SOC_PPC_L2_LIF_VXLAN_INFO_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_VXLAN_INFO *info
  );

uint32
  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_KEY *info
  );

uint32
  SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION_verify(
    SOC_SAND_IN  SOC_PPC_L2_LIF_DEFAULT_FRWRD_ACTION *info
  );

uint32 SOC_PPC_L2_LIF_EXTENDER_KEY_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_KEY *info);

uint32 SOC_PPC_L2_LIF_EXTENDER_INFO_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_L2_LIF_EXTENDER_INFO *info);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LIF_INCLUDED__*/
#endif

