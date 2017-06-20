/* $Id: ppd_api_lif_ing_vlan_edit.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_lif_ing_vlan_edit.h
*
* MODULE PREFIX:  soc_ppd_lif
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

#ifndef __SOC_PPD_API_LIF_ING_VLAN_EDIT_INCLUDED__
/* { */
#define __SOC_PPD_API_LIF_ING_VLAN_EDIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_lif_ing_vlan_edit.h>

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
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_ID_SET = SOC_PPD_PROC_DESC_BASE_LIF_ING_VLAN_EDIT_FIRST,
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_ID_SET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_ID_GET,
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_ID_GET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_INFO_SET,
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_INFO_SET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_INFO_GET,
  SOC_PPD_LIF_ING_VLAN_EDIT_COMMAND_INFO_GET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_SET,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_SET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_GET,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_GET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_SET,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_SET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_GET,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_GET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_SET,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_SET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_GET,
  SOC_PPD_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_GET_PRINT,
  SOC_PPD_LIF_ING_VLAN_EDIT_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LIF_ING_VLAN_EDIT_PROCEDURE_DESC_LAST
} SOC_PPD_LIF_ING_VLAN_EDIT_PROCEDURE_DESC;

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
 *   soc_ppd_lif_ing_vlan_edit_command_id_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Ingress VLAN Edit command to perform over
 *   packets according to incoming VLAN tags format and
 *   required VLAN edit profile (set according to AC).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY       *command_key -
 *     The Command key includes: - packet tag format: specifies
 *     which Tags are found on the incoming packet.- Edit
 *     profile: Up to 8 different editing profiles may be
 *     assigned to packets per given incoming tag format. The
 *     selected Edit Profile is an attribute of the In-AC. This
 *     allows for different tag structures to be imposed on
 *     packets by different In-ACs (e.g., One In-AC is set to
 *     always inject C-Tagged packets into the network,
 *     regardless if the incoming packet was tagged or not,
 *     while a different In-AC may be set to drop untagged
 *     packets)
 *   SOC_SAND_IN  uint32                                command_id -
 *     Handle to be used to set the Edit command information.
 *     See soc_ppd_lif_ing_vlan_edit_command_info_set()
 * REMARKS:
 *   - In order to set the command configuration use
 *   soc_ppd_lif_ing_vlan_edit_command_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_command_id_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY       *command_key,
    SOC_SAND_IN  uint32                                command_id
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lif_ing_vlan_edit_command_id_set" API.
 *     Refer to "soc_ppd_lif_ing_vlan_edit_command_id_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_command_id_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY       *command_key,
    SOC_SAND_OUT uint32                                *command_id
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_ing_vlan_edit_command_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Edit command to perform over packets.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                command_ndx -
 *     Command ID, set according
 *     soc_ppd_lif_ing_vlan_edit_command_id_set(). Range: 0 - 15(arad)/0-63(petra).
 *   SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO      *command_info -
 *     Edit command to perform on the packets (number of tags
 *     to remove, and tags to build) in order to apply the
 *     required tag structure identified by the Edit Profile
 *     per the given packet_tag_format.
 * REMARKS:
 *   - In order to set the command ID according to Tag Format
 *   and Vlan edit profile use
 *   soc_ppd_lif_ing_vlan_edit_command_id_set()- The commands
 *   should be set systemically with the same configuration.
 *   The command ID is determined at the ingress device
 *   However the edit command resolution occurs at the egress
 *   device. Thus egress devices should be synchronized to
 *   the command ID.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_command_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                command_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO      *command_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lif_ing_vlan_edit_command_info_set" API.
 *     Refer to "soc_ppd_lif_ing_vlan_edit_command_info_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_command_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                command_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO      *command_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_ing_vlan_edit_pcp_map_stag_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from outer S-tag PCP and DEI to the PCP and
 *   DEI values to be set in the transmitted packet's tag.
 *   This is the mapping to be used when the incoming packet
 *   is S-tagged (outer Tag).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pcp_profile_ndx -
 *     The PCP profile is set according to AC. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_pcp_ndx -
 *     The PCP appears in the outer S-Tag of the packet. Range:
 *     0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         tag_dei_ndx -
 *     The DEI appears in the outer S-Tag of the packet. Range:
 *     0 - 1.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     presents the UP to set in the packet tag. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei -
 *     The mapped DEI to set in the transmitted packet header.
 *     This is relevant when used for S-Tag. When the mapping
 *     used to build C tag, this value has to be set to 0.
 *     Range: 0 - 1.
 * REMARKS:
 *   - This mapping is used when the outer packet tag is
 *   S-tag.- When this mapping is used to build the C-tag,
 *   then the out-PCP is actually the UP, and the DEI has to
 *   be 0.- This value of PCP and DEI will be used when the
 *   source of PCP DEI is selected to be
 *   SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_NEW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_pcp_map_stag_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         tag_dei_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lif_ing_vlan_edit_pcp_map_stag_set" API.
 *     Refer to "soc_ppd_lif_ing_vlan_edit_pcp_map_stag_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_pcp_map_stag_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         tag_dei_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_ing_vlan_edit_pcp_map_ctag_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from outer C-tag UP to the PCP and DEI
 *   values to be set in the transmitted packet's tag. This
 *   is the mapping to be used when the incoming packet is
 *   C-tagged (outer Tag.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pcp_profile_ndx -
 *     The PCP profile is set according to AC. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_up_ndx -
 *     The UP appears in the outer C-Tag of the packet. Range:
 *     0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     presents the UP to set in the packet tag. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei -
 *     The mapped DEI to set in the transmitted packet header.
 *     This is relevant when used for S-Tag. When the mapping
 *     used to build C tag, this value has to be set to 0.
 *     Range: 0 - 1.
 * REMARKS:
 *   - This mapping is used when the outer packet tag is
 *   C-tag.- When this mapping is used to build the C-tag,
 *   then the out-PCP is actually the UP, and the DEI has to
 *   be 0.- This value of PCP and DEI will be used when the
 *   source of PCP DEI is selected to be
 *   SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_MAP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_pcp_map_ctag_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_up_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lif_ing_vlan_edit_pcp_map_ctag_set" API.
 *     Refer to "soc_ppd_lif_ing_vlan_edit_pcp_map_ctag_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_pcp_map_ctag_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_up_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_ing_vlan_edit_pcp_map_untagged_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from COS parameters (DP and TC) to the PCP
 *   and DEI values to be set in the transmitted packet's
 *   tag. This is the mapping to be used when the incoming
 *   packet has no tags.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pcp_profile_ndx -
 *     The PCP profile is set according to AC. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx -
 *     Traffic Class. Calculated at the ingress. See COS
 *     module. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx -
 *     Drop Precedence. Calculated at the ingress. See COS
 *     module. Range: 0 - 3.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     represents the UP to set in the packet C-tag. Range: 0 -
 *     7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei -
 *     The mapped DEI to set in the transmitted packet header.
 *     This is relevant when used for S-Tag. When the mapping
 *     used to build C tag, this value has to be set to 0.
 *     Range: 0 - 1.
 * REMARKS:
 *   - This mapping is used when the packet has no VLAN
 *   Tags.- When the Tag to build is C-Tag, then the out-PCP
 *   is actually the UP, and the DEI has to be 0.- This value
 *   of PCP and DEI will be used when the source of PCP DEI
 *   is selected to be
 *   SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_MAP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_pcp_map_untagged_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lif_ing_vlan_edit_pcp_map_untagged_set" API.
 *     Refer to "soc_ppd_lif_ing_vlan_edit_pcp_map_untagged_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_lif_ing_vlan_edit_pcp_map_untagged_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LIF_ING_VLAN_EDIT_INCLUDED__*/
#endif

