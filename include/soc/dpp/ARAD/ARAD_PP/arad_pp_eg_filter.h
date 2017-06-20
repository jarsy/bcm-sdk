
/* $Id: arad_pp_eg_filter.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_EG_FILTER_INCLUDED__
/* { */
#define __ARAD_PP_EG_FILTER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_port.h>
#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_eg_filter.h>
#include <soc/dpp/PPC/ppc_api_llp_parse.h>
#include <soc/dpp/PPC/ppc_api_llp_filter.h>


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
  SOC_PPC_EG_FILTER_PORT_INFO_SET = ARAD_PP_PROC_DESC_BASE_EG_FILTER_FIRST,
  SOC_PPC_EG_FILTER_PORT_INFO_SET_PRINT,
  SOC_PPC_EG_FILTER_PORT_INFO_SET_UNSAFE,
  SOC_PPC_EG_FILTER_PORT_INFO_SET_VERIFY,
  SOC_PPC_EG_FILTER_PORT_INFO_GET,
  SOC_PPC_EG_FILTER_PORT_INFO_GET_PRINT,
  SOC_PPC_EG_FILTER_PORT_INFO_GET_VERIFY,
  SOC_PPC_EG_FILTER_PORT_INFO_GET_UNSAFE,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_SET,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_SET_PRINT,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_SET_UNSAFE,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_SET_VERIFY,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_GET,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_GET_PRINT,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_GET_VERIFY,
  ARAD_PP_EG_FILTER_VSI_PORT_MEMBERSHIP_GET_UNSAFE,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_SET,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_SET_PRINT,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_SET_UNSAFE,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_SET_VERIFY,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_GET,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_GET_PRINT,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_GET_VERIFY,
  ARAD_PP_EG_FILTER_CVID_PORT_MEMBERSHIP_GET_UNSAFE,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_SET,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_SET_PRINT,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_SET_UNSAFE,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_SET_VERIFY,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_GET,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_GET_PRINT,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_GET_VERIFY,
  ARAD_PP_EG_FILTER_PORT_ACCEPTABLE_FRAMES_GET_UNSAFE,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_SET,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_SET_PRINT,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_SET_UNSAFE,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_SET_VERIFY,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_GET,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_GET_PRINT,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_GET_VERIFY,
  ARAD_PP_EG_FILTER_PEP_ACCEPTABLE_FRAMES_GET_UNSAFE,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_SET,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_SET_PRINT,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_SET_UNSAFE,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_SET_VERIFY,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_GET,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_GET_PRINT,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_GET_VERIFY,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_GET_UNSAFE,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_SET,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_SET_PRINT,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_SET_UNSAFE,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_SET_VERIFY,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_GET,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_GET_PRINT,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_GET_VERIFY,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_GET_UNSAFE,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_SET,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_SET_PRINT,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_SET_UNSAFE,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_SET_VERIFY,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_GET,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_GET_PRINT,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_GET_VERIFY,
  ARAD_PP_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_GET_UNSAFE,
  ARAD_PP_EG_FILTER_VSI_MEMBERSHIP_GET,
  ARAD_PP_EG_FILTER_VSI_MEMBERSHIP_GET_PRINT,
  ARAD_PP_EG_FILTER_VSI_MEMBERSHIP_GET_VERIFY,
  ARAD_PP_EG_FILTER_VSI_MEMBERSHIP_GET_UNSAFE,
  ARAD_PP_EG_FILTER_GET_PROCS_PTR,
  ARAD_PP_EG_FILTER_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_FILTER_PROCEDURE_DESC_LAST
} ARAD_PP_EG_FILTER_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_EG_FILTER_IS_MEMBER_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_EG_FILTER_FIRST,
  ARAD_PP_EG_FILTER_PORT_ACC_FRAMES_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_FILTER_VLAN_FORMAT_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_FILTER_ACCEPT_OUT_OF_RANGE_ERR,
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_FILTER_ORIENTATION_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_FILTER_FILTER_MASK_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_FILTER_ACCEPTABLE_FRAMES_PROFILE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_EG_PVLAN_FEATURE_DISABLED_ERR,
  ARAD_PP_EG_SPLIT_HORIZON_FEATURE_DISABLED_ERR,
  ARAD_PP_EG_FLTER_PORT_SPLIT_DISABLE_ERR,
  ARAD_PP_EG_FLTER_PORT_MTU_DISABLE_ERR,
  

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_FILTER_ERR_LAST
} ARAD_PP_EG_FILTER_ERR;

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
  arad_pp_eg_filter_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_port_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets out-port filtering information, including which
 *   filtering to perform on this specific out-port.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx -
 *     Local out-port ID.
 *   SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO          *port_info -
 *     Filtering information per port.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_filter_port_info_set_unsafe(
    SOC_SAND_IN  int                          unit,
	SOC_SAND_IN  int                          core_id,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO          *port_info
  );

uint32
  arad_pp_eg_filter_port_info_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO          *port_info
  );

uint32
  arad_pp_eg_filter_port_info_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_port_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_filter_port_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_filter_port_info_get_unsafe(
    SOC_SAND_IN  int                          unit,
	SOC_SAND_IN  int                          core_id,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_FILTER_PORT_INFO          *port_info
  );

#ifdef BCM_88660_A0
uint32
  arad_pp_eg_filter_global_info_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_GLOBAL_INFO   *global_info
  );

uint32
  arad_pp_eg_filter_global_info_set_verify(
    SOC_SAND_IN int                          unit,
    SOC_SAND_IN SOC_PPC_EG_FILTER_GLOBAL_INFO   *global_info
  );

uint32
  arad_pp_eg_filter_global_info_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_GLOBAL_INFO   *global_info
  );

uint32
  arad_pp_eg_filter_global_info_get_unsafe(
    SOC_SAND_OUT int                          unit,
    SOC_SAND_OUT SOC_PPC_EG_FILTER_GLOBAL_INFO   *global_info
  );
#endif /* BCM_88660_A0 */

/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_vsi_port_membership_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets egress VSI membership, which represents the
 *   Service/S-VLAN membership. Set whether outgoing local
 *   port belongs to the VSI. Packets transmitted out through
 *   a port that is not member of the packet's VSI are
 *   filtered.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx -
 *     SOC_SAND_IN SOC_PPC_VSI_ID vsid_ndx
 *   SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx -
 *     Local out-port ID.
 *   SOC_SAND_IN  uint8                                 is_member -
 *     If TRUE, then the port is member of the VSI; otherwise,
 *     the port is not member.
 * REMARKS:
 *   membership.- T20E: - In T20E, both memberships may be
 *   checked for CEP ports.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_filter_vsi_port_membership_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  int                          core_id,
    SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_IN  uint8                          is_member
  );

uint32
  arad_pp_eg_filter_vsi_port_membership_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_IN  uint8                          is_member
  );

uint32
  arad_pp_eg_filter_vsi_port_membership_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_vsi_port_membership_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_filter_vsi_port_membership_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_filter_vsi_port_membership_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  int                          core_id,
    SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_OUT uint8                          *is_member
  );

uint32
  arad_pp_eg_filter_vsi_membership_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_vsi_membership_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_filter_vsi_membership_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_filter_vsi_membership_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  int                          core_id,
    SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx,    
    SOC_SAND_OUT uint32                               ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_default_port_membership_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets egress default membership for untagged packets
 *   Untagged packets transmitted out through
 *   a port that is not in the default membership list are
 *   filtered.
 * INPUT:
 *   SOC_SAND_IN  int           unit - Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT  out_port_ndx - Local out-port ID.
 *   SOC_SAND_IN  uint8         is_member - If TRUE,
 *     then the port is in the default membership list;
 *     otherwise, the port is not member.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_eg_filter_default_port_membership_set(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_PORT                 out_port_ndx,
    SOC_SAND_IN  uint8                        is_member
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_default_port_membership_set" API.
 *     Refer to
 *     "arad_pp_eg_filter_default_port_membership_set" API for
 *     details.
*********************************************************************/
soc_error_t
  arad_pp_eg_filter_default_port_membership_get(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_PORT                 out_port_ndx,
    SOC_SAND_OUT uint8                        *is_member
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_cvid_port_membership_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets egress CVID membership (relevant for CEP ports).
 *   Sets whether outgoing local port belongs to the CVID.
 *   Packets transmitted out through a port that is not
 *   member of the packet's CVID are filtered.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    cvid_ndx -
 *     CVID to set the membership for. Range: 0 - 4095. (4K)
 *   SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx -
 *     Local out-port ID.
 *   SOC_SAND_IN  uint8                          is_member -
 *     If TRUE, then the port is member of the CVID; otherwise,
 *     the port is not member.
 * REMARKS:
 *   - T20E only- Port type has to be CEP.- It is ensured
 *   that packet has outer tag (C-tag in this case), either
 *   arriving on the packet or as a result of the editing
 *   process.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_filter_cvid_port_membership_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    cvid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_IN  uint8                          is_member
  );

uint32
  arad_pp_eg_filter_cvid_port_membership_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    cvid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_IN  uint8                          is_member
  );

uint32
  arad_pp_eg_filter_cvid_port_membership_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    cvid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_cvid_port_membership_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_filter_cvid_port_membership_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_eg_filter_cvid_port_membership_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    cvid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                         out_port_ndx,
    SOC_SAND_OUT uint8                          *is_member
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_port_acceptable_frames_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets acceptable frame type on port profile.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                        eg_acceptable_frames_port_profile -
 *     Egress Acceptable frames profile per port.
 *   SOC_SAND_IN  uint32                        llvp_port_profile -
 *     LLVP profile per port.
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *eg_prsr_out_key -
 *     Egress parsing output specifying what TPIDs exist on the
 *     packet
 *   SOC_SAND_IN  uint8                                 accept -
 *     Accept or deny this frame type for this port type.
 * REMARKS:
 *   - T20E: not supported. Error will be returned if
 *   called.- For the profiles (0-3).- This actually refines
 *   the default and 'standard' frame types acceptance set
 *   according to port type.- Use
 *   SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY as
 *   vlan_format_ndx to make a decision for ALL VLAN formats
 *   port type.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_filter_port_acceptable_frames_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                          eg_acceptable_frames_port_profile,
    SOC_SAND_IN  uint32                          llvp_port_profile,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO          *eg_prsr_out_key,
    SOC_SAND_IN  uint8                           accept
  );

uint32
  arad_pp_eg_filter_port_acceptable_frames_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                          eg_acceptable_frames_port_profile,
    SOC_SAND_IN  uint32                          llvp_port_profile,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO          *eg_prsr_out_key,
    SOC_SAND_IN  uint8                           accept
  );

uint32
  arad_pp_eg_filter_port_acceptable_frames_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *eg_prsr_out_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_port_acceptable_frames_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_filter_port_acceptable_frames_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_eg_filter_port_acceptable_frames_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                           port_profile,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *eg_prsr_out_key,
    SOC_SAND_OUT uint8                                 *accept
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_pep_acceptable_frames_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets acceptable frame type for PEP port.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key -
 *     PEP key identifier, out-port x VSI.
 *   SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx -
 *     Packet frame type (tags stack structure). Possible
 *     values: - SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_C_TAG, -
 *     SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_NONE-
 *     SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY (presents both
 *     C-Tagged and untagged packets)
 *   SOC_SAND_IN  uint8                                 accept -
 *     Accept or deny this frame type for the given PEP.
 * REMARKS:
 *   - Arad-B: not supported. Error will be returned if
 *   called.- use SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY as
 *   vlan_format_ndx to make a decision for ALL VLAN formats
 *   at once.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_filter_pep_acceptable_frames_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx,
    SOC_SAND_IN  uint8                                 accept
  );

uint32
  arad_pp_eg_filter_pep_acceptable_frames_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx,
    SOC_SAND_IN  uint8                                 accept
  );

uint32
  arad_pp_eg_filter_pep_acceptable_frames_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_pep_acceptable_frames_set_unsafe" API.
 *     Refer to
 *     "arad_pp_eg_filter_pep_acceptable_frames_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_eg_filter_pep_acceptable_frames_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx,
    SOC_SAND_OUT uint8                                 *accept
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_pvlan_port_type_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Private VLAN (PVLAN) port type.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                           phy_sys_port_ndx -
 *     Physical System port. Range: 0 - 4095.
 *   SOC_SAND_IN  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE    pvlan_port_type -
 *     PVLAN port Type (promiscuous, isolated, community)
 * REMARKS:
 *   - Each device has to know (locally) the type of every
 *   system port that may transmit a packet to it.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_filter_pvlan_port_type_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE           pvlan_port_type
  );

uint32
  arad_pp_eg_filter_pvlan_port_type_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE           pvlan_port_type
  );

uint32
  arad_pp_eg_filter_pvlan_port_type_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_pvlan_port_type_set_unsafe" API.
 *     Refer to "arad_pp_eg_filter_pvlan_port_type_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_eg_filter_pvlan_port_type_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                       *src_sys_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE    *pvlan_port_type
  );

/*********************************************************************
* Not supported for arad. 
* see arad_pp_eg_filter_split_horizon_out_lif_orientation_set_unsafe 
* API for details. 
*********************************************************************/
uint32
  arad_pp_eg_filter_split_horizon_out_ac_orientation_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION  orientation
  );

uint32
  arad_pp_eg_filter_split_horizon_out_ac_orientation_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION  orientation
  );

uint32
  arad_pp_eg_filter_split_horizon_out_ac_orientation_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx
  );

/*********************************************************************
* Not supported for arad. 
* see arad_pp_eg_filter_split_horizon_out_lif_orientation_get_unsafe 
* API for details. 
*********************************************************************/
uint32
  arad_pp_eg_filter_split_horizon_out_ac_orientation_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_OUT SOC_SAND_PP_HUB_SPOKE_ORIENTATION  *orientation
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_split_horizon_out_lif_orientation_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the orientation of out-AC, hub or spoke.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                        lif_ee_ndx -
 *     Egress Encapsulation table entry, of type Out-LIF (First
 *     encapsulation)
 *   SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION  orientation -
 *     Out AC orientation Hub or Spoke. Packets forwarded from
 *     hub interface to hub interface will be filtered.
 * REMARKS:
 *   - T20E, PetraB: not supported. Error will be returned if
 *   called.-
 *   In Arad this setting will be used only if -
 *   Auxiliary DB allocated for this purpose. - the EEI
 *   associated with the packet has out-AC.- Orientation of
 *   incoming interface set according to
 *   soc_ppd_l2_lif_pwe_add()/ soc_ppd_l2_lif_ac_add()- To set
 *   orientation of tunnel, use
 *   soc_ppd_eg_encap_mpls_encap_entry_add()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_filter_split_horizon_out_lif_orientation_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        lif_eep_ndx,
    SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION      orientation
  );

uint32
  arad_pp_eg_filter_split_horizon_out_lif_orientation_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        lif_eep_ndx,
    SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION      orientation
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_filter_split_horizon_out_lif_orientation_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_eg_filter_split_horizon_out_lif_orientation_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_eg_filter_split_horizon_out_lif_orientation_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                           lif_eep_ndx,
    SOC_SAND_OUT SOC_SAND_PP_HUB_SPOKE_ORIENTATION      *orientation
  );

uint32
  arad_pp_eg_filter_split_horizon_out_lif_orientation_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                           eep_ndx
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_eg_filter module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_eg_filter_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_eg_filter_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_eg_filter module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_eg_filter_get_errs_ptr(void);

uint32
  SOC_PPC_EG_FILTER_PORT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO *info
  );

#ifdef BCM_88660_A0
uint32
  ARAD_PP_EG_FILTER_GLOBAL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_FILTER_GLOBAL_INFO *info
  );
#endif /* BCM_88660_A0 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_EG_FILTER_INCLUDED__*/
#endif



