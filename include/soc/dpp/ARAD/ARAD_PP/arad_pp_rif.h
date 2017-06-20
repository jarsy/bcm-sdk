/* $Id: arad_pp_rif.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_RIF_INCLUDED__
/* { */
#define __ARAD_PP_RIF_INCLUDED__

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

#define ARAD_PP_RIF_COS_PROFILE_MAX              63
#define ARAD_PP_RIF_DEFAULT_FORWARD_PROFILE_MAX  3
#define ARAD_RIF_VSI_ASSIGNMENT_MODE_MAX         3
#define ARAD_RIF_PROTECTION_POINTER_MAX          ((1<<14)-1)
#define ARAD_RIF_TRAP_CODE_INDEX_MAX             7

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
  SOC_PPC_RIF_MPLS_LABELS_RANGE_SET = ARAD_PP_PROC_DESC_BASE_RIF_FIRST,
  SOC_PPC_RIF_MPLS_LABELS_RANGE_SET_PRINT,
  SOC_PPC_RIF_MPLS_LABELS_RANGE_SET_UNSAFE,
  SOC_PPC_RIF_MPLS_LABELS_RANGE_SET_VERIFY,
  SOC_PPC_RIF_MPLS_LABELS_RANGE_GET,
  SOC_PPC_RIF_MPLS_LABELS_RANGE_GET_PRINT,
  SOC_PPC_RIF_MPLS_LABELS_RANGE_GET_VERIFY,
  SOC_PPC_RIF_MPLS_LABELS_RANGE_GET_UNSAFE,
  ARAD_PP_RIF_MPLS_LABEL_MAP_ADD,
  ARAD_PP_RIF_MPLS_LABEL_MAP_ADD_PRINT,
  ARAD_PP_RIF_MPLS_LABEL_MAP_ADD_UNSAFE,
  ARAD_PP_RIF_MPLS_LABEL_MAP_ADD_INTERNAL_UNSAFE,
  ARAD_PP_RIF_MPLS_LABEL_MAP_ADD_VERIFY,
  ARAD_PP_RIF_MPLS_LABEL_MAP_REMOVE,
  ARAD_PP_RIF_MPLS_LABEL_MAP_REMOVE_PRINT,
  ARAD_PP_RIF_MPLS_LABEL_MAP_REMOVE_UNSAFE,
  ARAD_PP_RIF_MPLS_LABEL_MAP_REMOVE_VERIFY,
  ARAD_PP_RIF_MPLS_LABEL_MAP_GET,
  ARAD_PP_RIF_MPLS_LABEL_MAP_GET_PRINT,
  ARAD_PP_RIF_MPLS_LABEL_MAP_GET_UNSAFE,
  ARAD_PP_RIF_MPLS_LABEL_MAP_GET_INTERNAL_UNSAFE,
  ARAD_PP_RIF_MPLS_LABEL_MAP_GET_VERIFY,
  ARAD_PP_RIF_IP_TUNNEL_TERM_ADD,
  ARAD_PP_RIF_IP_TUNNEL_TERM_ADD_PRINT,
  ARAD_PP_RIF_IP_TUNNEL_TERM_ADD_UNSAFE,
  ARAD_PP_RIF_IP_TUNNEL_TERM_ADD_INTERNAL_UNSAFE,
  ARAD_PP_RIF_IP_TUNNEL_TERM_ADD_VERIFY,
  ARAD_PP_RIF_IP_TUNNEL_TERM_REMOVE,
  ARAD_PP_RIF_IP_TUNNEL_TERM_REMOVE_PRINT,
  ARAD_PP_RIF_IP_TUNNEL_TERM_REMOVE_UNSAFE,
  ARAD_PP_RIF_IP_TUNNEL_TERM_REMOVE_VERIFY,
  ARAD_PP_RIF_IP_TUNNEL_TERM_GET,
  ARAD_PP_RIF_IP_TUNNEL_TERM_GET_PRINT,
  ARAD_PP_RIF_IP_TUNNEL_TERM_GET_UNSAFE,
  ARAD_PP_RIF_IP_TUNNEL_TERM_GET_INTERNAL_UNSAFE,
  ARAD_PP_RIF_IP_TUNNEL_TERM_GET_VERIFY,
  ARAD_PP_RIF_VSID_MAP_SET,
  ARAD_PP_RIF_VSID_MAP_SET_PRINT,
  ARAD_PP_RIF_VSID_MAP_SET_UNSAFE,
  ARAD_PP_RIF_VSID_MAP_SET_VERIFY,
  ARAD_PP_RIF_VSID_MAP_GET,
  ARAD_PP_RIF_VSID_MAP_GET_PRINT,
  ARAD_PP_RIF_VSID_MAP_GET_VERIFY,
  ARAD_PP_RIF_VSID_MAP_GET_UNSAFE,
  SOC_PPC_RIF_INFO_SET,
  SOC_PPC_RIF_INFO_SET_PRINT,
  SOC_PPC_RIF_INFO_SET_UNSAFE,
  SOC_PPC_RIF_INFO_SET_VERIFY,
  SOC_PPC_RIF_INFO_GET,
  SOC_PPC_RIF_INFO_GET_PRINT,
  SOC_PPC_RIF_INFO_GET_VERIFY,
  SOC_PPC_RIF_INFO_GET_UNSAFE,
  ARAD_PP_RIF_TTL_SCOPE_SET,
  ARAD_PP_RIF_TTL_SCOPE_SET_PRINT,
  ARAD_PP_RIF_TTL_SCOPE_SET_UNSAFE,
  ARAD_PP_RIF_TTL_SCOPE_SET_VERIFY,
  ARAD_PP_RIF_TTL_SCOPE_GET,
  ARAD_PP_RIF_TTL_SCOPE_GET_PRINT,
  ARAD_PP_RIF_TTL_SCOPE_GET_VERIFY,
  ARAD_PP_RIF_TTL_SCOPE_GET_UNSAFE,
  ARAD_PP_RIF_GET_PROCS_PTR,
  ARAD_PP_RIF_GET_ERRS_PTR,
  ARAD_PP_GET_IN_RIF_PROFILE_FROM_VRF_UNSAFE,
  ARAD_PP_SET_IN_RIF_PROFILE_TO_VRF_UNSAFE,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_RIF_PROCEDURE_DESC_LAST
} ARAD_PP_RIF_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_RIF_SUCCESS_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_RIF_FIRST,
  ARAD_PP_RIF_DIP_KEY_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_FIRST_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_LAST_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_COS_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_TTL_SCOPE_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_ROUTING_ENABLERS_BM_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_MPLS_LABEL_INVALID_RANGE_ERR,
  ARAD_PP_RIF_VRF_NOT_FOUND_ERR,
  ARAD_PP_RIF_LIF_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_TPID_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_RIF_DEFAULT_FORWARD_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_RIF_SERVICE_TYPE_OUT_OF_RANGE_ERR,
  ARAD_RIF_VSI_ASSIGNMENT_MODE_OUT_OF_RANGE_ERR,
  ARAD_RIF_PROTECTION_POINTER_OUT_OF_RANGE_ERR,
  ARAD_RIF_MPLS_TERM_PROCESSING_TYPE_OUT_OF_RANGE_ERR,
  ARAD_RIF_MPLS_TERM_NEXT_PRTCL_OUT_OF_RANGE_ERR,
  ARAD_RIF_TRAP_CODE_INDEX_OUT_OF_RANGE_ERR,
  ARAD_RIF_LEARN_RECORD_OUT_OF_RANGE_ERR,
  ARAD_RIF_FORWARADING_CODE_OUT_OF_RANGE_ERR,

  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_RIF_MPLS_LABEL_KEY_ILLEGAL_ERR,
  ARAD_PP_RIF_IP_TUNNEL_TERM_KEY_ILLEGAL_ERR,



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_RIF_ERR_LAST
} ARAD_PP_RIF_ERR;

typedef struct
{
  /* URPF mode for RIFs */
  PARSER_HINT_ARR uint8 *rif_urpf_mode;
} ARAD_PP_RIF_TO_LIF_GROUP_MAP;

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
  arad_pp_rif_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_mpls_labels_range_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Set the MPLS labels that may be mapped to Router
*   Interfaces
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_PPC_RIF_MPLS_LABELS_RANGE               *rif_labels_range -
*     First and Last MPLS Labels to be mapped to Router
*     Interfaces. Range Size: 0-896K
* REMARKS:
*   Not supported in Petra-B and in Arad.
*   Set the MPLS labels that may
*   be mapped to Router Interfaces.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_mpls_labels_range_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_MPLS_LABELS_RANGE               *rif_labels_range
  );

uint32
  arad_pp_rif_mpls_labels_range_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_MPLS_LABELS_RANGE               *rif_labels_range
  );

uint32
  arad_pp_rif_mpls_labels_range_get_verify(
    SOC_SAND_IN  int                                 unit
  );
/*********************************************************************
*   Sets the URPF mode to strict/loose
*   Not supported in Petra-B and in Arad.
*********************************************************************/
soc_error_t
 arad_pp_rif_global_urpf_mode_set(int unit, int index, int urpf_mode);
/*********************************************************************
*     Gets the configuration set by the
*     "arad_pp_rif_mpls_labels_range_set_unsafe" API.
*     Refer to "arad_pp_rif_mpls_labels_range_set_unsafe" API
*     for details.
*   Not supported in Petra-B and in Arad.
*********************************************************************/
uint32
  arad_pp_rif_mpls_labels_range_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_RIF_MPLS_LABELS_RANGE               *rif_labels_range
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_mpls_label_map_add_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Enable MPLS labels termination and setting the Router
*   interface according to the terminated MPLS label.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key -
*     The MPLS label, and optionally the VSID, to be
*     terminated and mapped to RIF
*   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
*     LIF table index
*   SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO                      *term_info -
*     MPLS Termination info including type of termination
*     (pipe/uniform), RIF
*   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
*     RIF attributes. RIF ID, ...
*   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
*     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
*     Exact Match table
* REMARKS:
*   T20E: The lif_index is ignored. Terminating the label via
*   'soc_ppd_rif_mpls_label_map_add()' enables:- Mapping to a
*   dedicated RIF, while range termination map to a global
*   RIF- Termination of labels outside the range-
*   Termination according to both MPLS label and the VSID,
*   and not only according to the MPLS label. The VSID was
*   assigned to the packet, according to the Link Layer
*   Ethernet header's AC.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_mpls_label_map_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO                      *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_rif_mpls_label_map_add_internal_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_IN  uint8                                 ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO                      *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_rif_mpls_label_map_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO                      *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_mpls_label_map_remove_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Remove MPLS label that was mapped to a RIF-Tunnel
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key -
*     The MPLS label, and optionally the VSID, to be
*     terminated and mapped to RIF
*   SOC_SAND_INOUT SOC_PPC_LIF_ID                              *lif_index -
*     LIF table index. Input when lookup type is NONE.
* REMARKS:
*   - Unbind the mapping of the MPLS label to the LIF table
*   from the SEM table- Invalidate the 'lif_index' entry in
*   the LIF table- The 'lif_index' is returned to the user
*   to enable management of the LIF table
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_mpls_label_map_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_INOUT SOC_PPC_LIF_ID                              *lif_index
  );

uint32
  arad_pp_rif_mpls_label_map_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_mpls_label_map_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Get MPLS label termination and Router interface info
*   according to the terminated MPLS label.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key -
*     The MPLS label, and optionally the VSID, to be
*     terminated and mapped to RIF
*   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
*     LIF table index
*   SOC_SAND_OUT SOC_PPC_MPLS_TERM_INFO                      *term_info -
*     MPLS Termination info including type of termination
*     (pipe/uniform), RIF-id>. The term_info.cos-profile is
*     ignored. Instead the rif_info.cos-profile is used.
*   SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info -
*     RIF attributes. RIF ID, ...
*   SOC_SAND_OUT uint8                                 *found -
*     Was key found
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_mpls_label_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_INFO                      *term_info,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_rif_mpls_label_map_get_internal_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_IN  uint8                                 ignore_key,
    SOC_SAND_OUT  SOC_PPC_LIF_ID                             *lif_index,
    SOC_SAND_OUT  SOC_PPC_MPLS_TERM_INFO                     *term_info,
    SOC_SAND_OUT  SOC_PPC_RIF_INFO                           *rif_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_rif_mpls_label_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_ip_tunnel_term_add_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Enable IP Tunnels termination and setting the Router
*   interface according to the terminated IP tunnel.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 flags -
*     Flags. Use to identify if lif_index is really a lif (for boundaries check)
*            or it's a learned FEC, or my_vtep_index
*   SOC_SAND_IN  uint32                                  dip_key -
*     Destination IP
*   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
*     Entry in the LIF table
*   SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO                    *term_info -
*     IP tunnel Termination info including type of termination
*     RIF-id, cos-profile.
*   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
*     RIF attributes. RIF ID, ...
*   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
*     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
*     Exact Match table
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_ip_tunnel_term_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                    flags,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                    *term_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO                    *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_rif_ip_tunnel_term_add_internal_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                *term_key,
    SOC_SAND_IN  uint8                                 ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO                    *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_rif_ip_tunnel_term_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                    flags,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                    *term_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO                    *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_ip_tunnel_term_remove_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Remove the IP Tunnel
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                  dip_key -
*     Destination IP
*   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
*     Entry in the LIF table
* REMARKS:
*   - Unbind the mapping of the IP Tunnel to the LIF table
*   from the SEM table- Invalidate the 'lif_index' entry in
*   the LIF table- The 'lif_index' is returned to the user
*   to enable management of the LIF table
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_ip_tunnel_term_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                    *term_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index
  );

uint32
  arad_pp_rif_ip_tunnel_term_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                 *term_key
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_ip_tunnel_term_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Get IP Tunnels termination and Router interface info
*   according to the terminated IP tunnel.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                  dip_key -
*     Destination IP
*   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
*     Entry in the LIF table
*   SOC_SAND_OUT SOC_PPC_RIF_IP_TERM_INFO                    *term_info -
*     IP tunnel Termination info including type of termination
*     RIF-id. The term_info.cos-profile is ignored. Instead
*     the rif_info.cos-profile is used.
*   SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info -
*     RIF attributes. RIF ID, ...
*   SOC_SAND_OUT uint8                                 *found -
*     Was key found
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_ip_tunnel_term_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                *term_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_RIF_IP_TERM_INFO                    *term_info,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT uint8                                 *found
  );


uint32
  arad_pp_rif_ip_tunnel_term_get_internal_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                *term_key,
    SOC_SAND_IN  uint8                                 ignore_key,
    SOC_SAND_OUT  SOC_PPC_LIF_ID                             *lif_index,
    SOC_SAND_OUT  SOC_PPC_RIF_IP_TERM_INFO                   *term_info,
    SOC_SAND_OUT  SOC_PPC_RIF_INFO                           *rif_info,
    SOC_SAND_OUT uint8                                 *found
  );


uint32
  arad_pp_rif_ip_tunnel_term_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY                *term_key
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_vsid_map_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Remove the IP Tunnel
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx -
*     VSID. Equal to the RIF-ID
*   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
*     RIF attributes. in this case RIF_ID has to be equal to
*     vsid_ndx
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_vsid_map_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

uint32
  arad_pp_rif_vsid_map_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

uint32
  arad_pp_rif_vsid_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
*     "arad_pp_rif_vsid_map_set_unsafe" API.
*     Refer to "arad_pp_rif_vsid_map_set_unsafe" API for
*     details.
*********************************************************************/
uint32
  arad_pp_rif_vsid_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_info_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Set the Router Interface according to the VSID.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx -
*     Router Interface ID. Range: 0 - 4K-1.
*   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
*     RIF attributes. CoS-Profile relvant only if the RIF-ID
*     used as VSI-RIF.
* REMARKS:
*   - use this API to set attributes of RIF obtained upon
*   MPLS tunnel termination by range, or reserved values.-
*   can be used also to update RIF attributes of RIF set
*   according to IP/MPLS tunnel termination or VSI.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

uint32
  arad_pp_rif_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

uint32
  arad_pp_rif_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
*     "arad_pp_rif_info_set_unsafe" API.
*     Refer to "arad_pp_rif_info_set_unsafe" API for details.
*********************************************************************/
uint32
  arad_pp_rif_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_ttl_scope_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Set TTL value for TTL-scope.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                 ttl_scope_ndx -
*     TTL scope index. Range: 0-7. set according to RIF. See
*     SOC_PPC_RIF_INFO.
*   SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl_val -
*     TTL value. Range: 0 -255.
* REMARKS:
*   - When packet is routed (IP/MPLS routing) to this RIF
*   then packet's TTL is compared againsy the TTL if it less
*   or equal then packet is filter. Range: 0 - 7. use
*   soc_ppd_trap_eg_profile_info_set(SOC_PPC_TRAP_EG_TYPE_TTL_SCOPE,
*   eg_trap_info) to set how to handle packets match
*   condition of this filter.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_rif_ttl_scope_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 ttl_scope_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl_val
  );

uint32
  arad_pp_rif_ttl_scope_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 ttl_scope_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IP_TTL                            ttl_val
  );

uint32
  arad_pp_rif_ttl_scope_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 ttl_scope_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
*     "arad_pp_rif_ttl_scope_set_unsafe" API.
*     Refer to "arad_pp_rif_ttl_scope_set_unsafe" API for
*     details.
*********************************************************************/
uint32
  arad_pp_rif_ttl_scope_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 ttl_scope_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IP_TTL                            *ttl_val
  );




/*********************************************************************
 * NAME:
 *   arad_pp_rif_native_routing_vlan_tags_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *        set native vsi compensation per rif
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_RIF_ID                               intf_id -
 *     RIF id. 
 *   SOC_SAND_IN  uint8                          native_routing_vlan_tags -
 *     Number of tags in native ethernet. Range: 0 - 2.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
arad_pp_rif_native_routing_vlan_tags_set_unsafe(
   SOC_SAND_IN  int                                    unit, 
   SOC_SAND_IN  SOC_PPC_RIF_ID                         intf_id, 
   SOC_SAND_IN  uint8                                  native_routing_vlan_tags
   );

uint32
arad_pp_rif_native_routing_vlan_tags_set_verify(
   SOC_SAND_IN  int                                    unit, 
   SOC_SAND_IN  SOC_PPC_RIF_ID                         intf_id, 
   SOC_SAND_IN  uint8                                  native_routing_vlan_tags
   );

uint32
arad_pp_rif_native_routing_vlan_tags_get_unsafe(
   SOC_SAND_IN  int                                    unit, 
   SOC_SAND_IN  SOC_PPC_RIF_ID                         intf_id, 
   SOC_SAND_OUT  uint8                                  *native_routing_vlan_tags
   );

uint32
arad_pp_rif_native_routing_vlan_tags_get_verify(
   SOC_SAND_IN  int                                    unit, 
   SOC_SAND_IN  SOC_PPC_RIF_ID                         intf_id, 
   SOC_SAND_IN  uint8                                  *native_routing_vlan_tags
   );

uint32
  arad_pp_l2_rif_mpls_key_parse_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY              *isem_key,
    SOC_SAND_OUT SOC_PPC_MPLS_LABEL_RIF_KEY           *rif_key
  );

uint32
  arad_pp_rif_ip_tunnel_key_parse_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  ARAD_PP_ISEM_ACCESS_KEY              *isem_key,
    SOC_SAND_OUT SOC_PPC_RIF_IP_TERM_KEY              *term_key
  );

/*********************************************************************
* NAME:
*   arad_pp_rif_get_procs_ptr
* TYPE:
*   PROC
* FUNCTION:
*   Get the pointer to the list of procedures of the
*   arad_pp_api_rif module.
* INPUT:
* REMARKS:
*
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_rif_get_procs_ptr(void);

/*********************************************************************
* NAME:
*   arad_pp_rif_get_errs_ptr
* TYPE:
*   PROC
* FUNCTION:
*   Get the pointer to the list of errors of the
*   arad_pp_api_rif module.
* INPUT:
* REMARKS:
*
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_rif_get_errs_ptr(void);

uint32
  SOC_PPC_RIF_MPLS_LABELS_RANGE_verify(
    SOC_SAND_IN  SOC_PPC_RIF_MPLS_LABELS_RANGE *info
  );

uint32
  SOC_PPC_MPLS_LABEL_RIF_KEY_verify(
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY *info
  );

uint32
  SOC_PPC_RIF_INFO_verify(
    SOC_SAND_IN  int              unit,
    SOC_SAND_IN  SOC_PPC_RIF_INFO *info
  );

uint32
  SOC_PPC_RIF_IP_TERM_INFO_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO *info
  );

/**********************************************************************************
*  Get the l_3_vpn_default_routing/InRifProfile bit for a given VRF by iterating  * 
*  over all the InRifs in IHP_IN_RIF_CONFIG_TABLE till the wanted VRF is found.   *
*  This determines What to do when an IP address is not found.                    *
*  Returns ARAD_PP_RIF_VRF_NOT_FOUND_ERR if the VRF is not found in the table.    *
***********************************************************************************/
uint32
  arad_pp_get_in_rif_profile_from_vrf_internal_unsafe(
    SOC_SAND_IN  int   unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID  vrf_id,
    SOC_SAND_OUT uint8   *out_is_in_rif_profile
  );

/**********************************************************************************
*  Set the l_3_vpn_default_routing/InRifProfile bit for a given VRF by iterating  * 
*  over all the InRifs in IHP_IN_RIF_CONFIG_TABLE and setting the entries of all  *
*  matching VRFs.                                                                 *
*  Returns ARAD_PP_RIF_VRF_NOT_FOUND_ERR if the VRF is not found in the table.    *
***********************************************************************************/
uint32
  arad_pp_set_in_rif_profile_to_vrf_internal_unsafe(
    SOC_SAND_IN  int   unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID  vrf_id,
    SOC_SAND_IN  uint8   is_in_rif_profile
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_RIF_INCLUDED__*/
#endif
