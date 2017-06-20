/* $Id: ppd_api_eg_filter.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_eg_filter.h
*
* MODULE PREFIX:  soc_ppd_eg
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_PPD_API_EG_FILTER_INCLUDED__
/* { */
#define __SOC_PPD_API_EG_FILTER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_eg_filter.h>
#include <soc/dpp/PPD/ppd_api_llp_parse.h>
#include <soc/dpp/PPD/ppd_api_llp_filter.h>
#include <soc/dpp/PPD/ppd_api_general.h>

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
  SOC_PPD_EG_FILTER_PORT_INFO_SET = SOC_PPD_PROC_DESC_BASE_EG_FILTER_FIRST,
  SOC_PPD_EG_FILTER_PORT_INFO_SET_PRINT,
  SOC_PPD_EG_FILTER_PORT_INFO_GET,
  SOC_PPD_EG_FILTER_PORT_INFO_GET_PRINT,
  SOC_PPD_EG_FILTER_VSI_PORT_MEMBERSHIP_SET,
  SOC_PPD_EG_FILTER_VSI_PORT_MEMBERSHIP_SET_PRINT,
  SOC_PPD_EG_FILTER_VSI_PORT_MEMBERSHIP_GET,
  SOC_PPD_EG_FILTER_VSI_PORT_MEMBERSHIP_GET_PRINT,
  SOC_PPD_EG_FILTER_CVID_PORT_MEMBERSHIP_SET,
  SOC_PPD_EG_FILTER_CVID_PORT_MEMBERSHIP_SET_PRINT,
  SOC_PPD_EG_FILTER_CVID_PORT_MEMBERSHIP_GET,
  SOC_PPD_EG_FILTER_CVID_PORT_MEMBERSHIP_GET_PRINT,
  SOC_PPD_EG_FILTER_PORT_ACCEPTABLE_FRAMES_SET,
  SOC_PPD_EG_FILTER_PORT_ACCEPTABLE_FRAMES_SET_PRINT,
  SOC_PPD_EG_FILTER_PORT_ACCEPTABLE_FRAMES_GET,
  SOC_PPD_EG_FILTER_PORT_ACCEPTABLE_FRAMES_GET_PRINT,
  SOC_PPD_EG_FILTER_PEP_ACCEPTABLE_FRAMES_SET,
  SOC_PPD_EG_FILTER_PEP_ACCEPTABLE_FRAMES_SET_PRINT,
  SOC_PPD_EG_FILTER_PEP_ACCEPTABLE_FRAMES_GET,
  SOC_PPD_EG_FILTER_PEP_ACCEPTABLE_FRAMES_GET_PRINT,
  SOC_PPD_EG_FILTER_PVLAN_PORT_TYPE_SET,
  SOC_PPD_EG_FILTER_PVLAN_PORT_TYPE_SET_PRINT,
  SOC_PPD_EG_FILTER_PVLAN_PORT_TYPE_GET,
  SOC_PPD_EG_FILTER_PVLAN_PORT_TYPE_GET_PRINT,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_SET,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_SET_PRINT,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_GET,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_AC_ORIENTATION_GET_PRINT,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_SET,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_SET_PRINT,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_GET,
  SOC_PPD_EG_FILTER_SPLIT_HORIZON_OUT_LIF_ORIENTATION_GET_PRINT,
  SOC_PPD_EG_FILTER_VSI_MEMBERSHIP_GET,
  SOC_PPD_EG_FILTER_VSI_MEMBERSHIP_GET_PRINT,
  SOC_PPD_EG_FILTER_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_EG_FILTER_PROCEDURE_DESC_LAST
} SOC_PPD_EG_FILTER_PROCEDURE_DESC;

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

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets out-port filtering information, including which
 *   filtering to perform on this specific out-port.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx -
 *     Local out-port ID.
 *   SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO                 *port_info -
 *     Filtering information per port.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_filter_port_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO                 *port_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_port_info_set" API.
 *     Refer to "soc_ppd_eg_filter_port_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_port_info_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_FILTER_PORT_INFO                 *port_info
  );

#ifdef BCM_88660_A0
/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_global_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets global filtering information.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO         *global_info -
 *     Global filtering information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_filter_global_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_GLOBAL_INFO          *global_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_global_info_set" API.
 *     Refer to "soc_ppd_eg_filter_global_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_global_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_EG_FILTER_GLOBAL_INFO        *global_info
  );
#endif /* BCM_88660_A0 */

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_vsi_port_membership_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets egress VSI membership, which represents the
 *   Service/S-VLAN membership. Set whether outgoing local
 *   port belongs to the VSI. Packets transmitted out through
 *   a port that is not member of the packet's VSI are
 *   filtered.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx -
 *     SOC_SAND_IN SOC_PPC_VSI_ID vsid_ndx
 *   SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx -
 *     Local out-port ID.
 *   SOC_SAND_IN  uint8                               is_member -
 *     If TRUE, then the port is member of the VSI; otherwise,
 *     the port is not member.
 * REMARKS:
 *   - T20E: - In T20E, both memberships may be checked for
 *   CEP ports.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_filter_vsi_port_membership_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  uint8                               is_member
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_vsi_port_membership_set" API.
 *     Refer to "soc_ppd_eg_filter_vsi_port_membership_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_vsi_port_membership_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_OUT uint8                               *is_member
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_vsi_membership_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *     Gets egress VSI membership, which represents the
 *     Service/S-VLAN membership. Get all member outgoing local
 *     ports belongs to the VSI. 
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx -
 *     SOC_SAND_IN SOC_PPC_VSI_ID vsid_ndx
 *   SOC_SAND_IN  uint32                                ports -
 *     SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE] - The VLAN membership
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_filter_vsi_membership_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_VSI_ID                       vsid_ndx,
    SOC_SAND_OUT uint32                               ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_cvid_port_membership_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets egress CVID membership (relevant for CEP ports).
 *   Sets whether outgoing local port belongs to the CVID.
 *   Packets transmitted out through a port that is not
 *   member of the packet's CVID are filtered.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         cvid_ndx -
 *     CVID to set the membership for. Range: 0 - 4095. (4K)
 *   SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx -
 *     Local out-port ID.
 *   SOC_SAND_IN  uint8                               is_member -
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
  soc_ppd_eg_filter_cvid_port_membership_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         cvid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  uint8                               is_member
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_cvid_port_membership_set" API.
 *     Refer to "soc_ppd_eg_filter_cvid_port_membership_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_cvid_port_membership_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         cvid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_OUT uint8                               *is_member
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_port_acceptable_frames_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets acceptable frame type on outgoing port.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                          eg_acceptable_frames_port_profile -
 *     Egress Acceptable frames profile per port.
 *     Soc_petra-B: local out port is passed here, but effectively
 *     acceptable_frames_profile is taken according to configuration
 *     the soc_ppd_eg_filter_port_info_set
 *   SOC_SAND_IN  uint32                          llvp_port_profile -
 *     LLVP profile per port.
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *eg_prsr_out_key -
 *     Egress parsing output specifying what TPIDs exist on the
 *     packet
 *   SOC_SAND_IN  uint8                               accept -
 *     Accept or deny this frame type for this port type.
 * REMARKS:
 *     the check is performed on packet after ingress vlan editing
 *     and before egress vlan editing
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_filter_port_acceptable_frames_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               eg_acceptable_frames_port_profile,
    SOC_SAND_IN  uint32                               llvp_port_profile,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO               *eg_prsr_out_key,
    SOC_SAND_IN  uint8                                accept
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_port_acceptable_frames_set" API.
 *     Refer to "soc_ppd_eg_filter_port_acceptable_frames_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_port_acceptable_frames_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                out_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *eg_prsr_out_key,
    SOC_SAND_OUT uint8                               *accept
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_pep_acceptable_frames_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets acceptable frame type for PEP port.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key -
 *     PEP key identifier, out-port x VSI.
 *   SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT      vlan_format_ndx -
 *     Packet frame type (tags stack structure). Possible
 *     values: - SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_C_TAG, -
 *     SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_NONE-
 *     SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY (presents both
 *     C-Tagged and untagged packets)
 *   SOC_SAND_IN  uint8                               accept -
 *     Accept or deny this frame type for the given PEP.
 * REMARKS:
 *   - Soc_petra-B: not supported. Error will be returned if
 *   called.- use SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY as
 *   vlan_format_ndx to make a decision for ALL VLAN formats
 *   at once.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_filter_pep_acceptable_frames_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT      vlan_format_ndx,
    SOC_SAND_IN  uint8                               accept
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_pep_acceptable_frames_set" API.
 *     Refer to "soc_ppd_eg_filter_pep_acceptable_frames_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_pep_acceptable_frames_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT      vlan_format_ndx,
    SOC_SAND_OUT uint8                               *accept
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_pvlan_port_type_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Private VLAN (PVLAN) port type.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *src_sys_port_ndx -
 *     SOC_SAND_IN SOC_SAND_PP_SYS_PORT_ID *src_sys_port_ndx
 *   SOC_SAND_IN  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE           pvlan_port_type -
 *     PVLAN port Type (promiscuous, isolated, community)
 * REMARKS:
 *   - Each device has to know (locally) the type of every
 *   system port that may transmit a packet to it.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_filter_pvlan_port_type_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *src_sys_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE           pvlan_port_type
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_pvlan_port_type_set" API.
 *     Refer to "soc_ppd_eg_filter_pvlan_port_type_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_pvlan_port_type_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *src_sys_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE           *pvlan_port_type
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_split_horizon_out_ac_orientation_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the orientation of out-AC, hub or spoke.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx -
 *     The out-AC associated with packet.
 *   SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION       orientation -
 *     Out AC orientation Hub or Spoke. Packets forwarded from
 *     hub interface to hub interface will be filtered.
 * REMARKS:
 *   - T20E: not supported. Error will be returned if
 *   called.- In Soc_petra-B this setting will be used only if -
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
  soc_ppd_eg_filter_split_horizon_out_ac_orientation_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION           orientation
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_split_horizon_out_ac_orientation_set"
 *     API.
 *     Refer to
 *     "soc_ppd_eg_filter_split_horizon_out_ac_orientation_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_split_horizon_out_ac_orientation_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_OUT SOC_SAND_PP_HUB_SPOKE_ORIENTATION           *orientation
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_filter_split_horizon_out_lif_orientation_set_unsafe
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
 *   - T20E, Soc_petraB: not supported. Error will be returned if
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
  soc_ppd_eg_filter_split_horizon_out_lif_orientation_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               lif_eep_ndx,
    SOC_SAND_IN  SOC_SAND_PP_HUB_SPOKE_ORIENTATION           orientation
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_filter_split_horizon_out_lif_orientation_set"
 *     API.
 *     Refer to
 *     "soc_ppd_eg_filter_split_horizon_out_lif_orientation_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_eg_filter_split_horizon_out_lif_orientation_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               lif_eep_ndx,
    SOC_SAND_OUT SOC_SAND_PP_HUB_SPOKE_ORIENTATION           *orientation
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_EG_FILTER_INCLUDED__*/
#endif

