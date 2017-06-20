
/* $Id: arad_pp_lif_ing_vlan_edit.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       soc_pb_pp_lif_ing_vlan_edit.h
*
* MODULE PREFIX:  soc_pb_pp
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

#ifndef __ARAD_PP_LIF_ING_VLAN_EDIT_INCLUDED__
/* { */
#define __ARAD_PP_LIF_ING_VLAN_EDIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_lif_ing_vlan_edit.h>

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
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_SET = ARAD_PP_PROC_DESC_BASE_LIF_ING_VLAN_EDIT_FIRST,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_SET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_SET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_SET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_GET,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_GET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_GET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_GET_UNSAFE,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_SET,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_SET_PRINT,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_SET_UNSAFE,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_SET_VERIFY,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_GET,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_GET_PRINT,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_GET_VERIFY,
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_GET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_SET,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_SET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_SET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_SET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_GET,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_GET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_GET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_STAG_GET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_SET,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_SET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_SET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_SET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_GET,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_GET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_GET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_CTAG_GET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_SET,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_SET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_SET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_SET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_GET,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_GET_PRINT,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_GET_VERIFY,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_MAP_UNTAGGED_GET_UNSAFE,
  ARAD_PP_LIF_ING_VLAN_EDIT_GET_PROCS_PTR,
  ARAD_PP_LIF_ING_VLAN_EDIT_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_DEI_TBL_SET,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_DEI_TBL_GET,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_ING_VLAN_EDIT_PROCEDURE_DESC_LAST
} ARAD_PP_LIF_ING_VLAN_EDIT_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_ID_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LIF_ING_VLAN_EDIT_FIRST,
  ARAD_PP_LIF_ING_VLAN_EDIT_COMMAND_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_TAG_PCP_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_OUT_PCP_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_TAG_UP_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_TC_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_DP_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_VID_SOURCE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_DEI_SOURCE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_TAG_FORMAT_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_EDIT_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_TAGS_TO_REMOVE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_TPID_PROFILE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_DEI_VID_SRC_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_INCONSISTANT_TBLS_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_DP_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_LIF_ING_VLAN_EDIT_PCP_DEI_OUT_OF_RANGE_ERR,
  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_ING_VLAN_EDIT_ERR_LAST
} ARAD_PP_LIF_ING_VLAN_EDIT_ERR;

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
  arad_pp_lif_ing_vlan_edit_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_command_id_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Ingress VLAN Edit command to perform over
 *   packets according to incoming VLAN tags format and
 *   required VLAN edit profile (set according to AC).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
 *   SOC_SAND_IN  uint32                                  command_id -
 *     Handle to be used to set the Edit command information.
 *     See soc_ppd_lif_ing_vlan_edit_command_info_set()
 * REMARKS:
 *   - In order to set the command configuration use
 *   soc_ppd_lif_ing_vlan_edit_command_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_command_id_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY       *command_key,
    SOC_SAND_IN  uint32                                  command_id
  );

uint32
  arad_pp_lif_ing_vlan_edit_command_id_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY       *command_key,
    SOC_SAND_IN  uint32                                  command_id
  );

uint32
  arad_pp_lif_ing_vlan_edit_command_id_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY       *command_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_ing_vlan_edit_command_id_set_unsafe" API.
 *     Refer to "arad_pp_lif_ing_vlan_edit_command_id_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_command_id_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY       *command_key,
    SOC_SAND_OUT uint32                                  *command_id
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_command_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Edit command to perform over packets.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  command_ndx -
 *     Command ID, set according
 *     soc_ppd_lif_ing_vlan_edit_command_id_set(). Range: 0 - 15.
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
  arad_pp_lif_ing_vlan_edit_command_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  command_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO      *command_info
  );

uint32
  arad_pp_lif_ing_vlan_edit_command_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  command_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO      *command_info
  );

uint32
  arad_pp_lif_ing_vlan_edit_command_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  command_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_ing_vlan_edit_command_info_set_unsafe" API.
 *     Refer to
 *     "arad_pp_lif_ing_vlan_edit_command_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_command_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  command_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO      *command_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_pcp_map_stag_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from outer S-tag PCP and DEI to the PCP and
 *   DEI values to be set in the transmitted packet's tag.
 *   This is the mapping to be used when the incoming packet
 *   is S-tagged (outer Tag).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  pcp_profile_ndx -
 *     The PCP profile is set according to AC. Range: 0 - 7.
 *   SOC_SAND_IN  uint8                                   tag_pcp_ndx -
 *     The PCP appears in the outer S-Tag of the packet. Range:
 *     0 - 7.
 *   SOC_SAND_IN  uint8                                 tag_dei_ndx -
 *     The DEI appears in the outer S-Tag of the packet. Range:
 *     0 - 1.
 *   SOC_SAND_IN  uint8                                   out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     presents the UP to set in the packet tag. Range: 0 - 7.
 *   SOC_SAND_IN  uint8                                 out_dei -
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
  arad_pp_lif_ing_vlan_edit_pcp_map_stag_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_pcp_ndx,
    SOC_SAND_IN  uint8                                 tag_dei_ndx,
    SOC_SAND_IN  uint8                                   out_pcp,
    SOC_SAND_IN  uint8                                 out_dei
  );

uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_stag_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_pcp_ndx,
    SOC_SAND_IN  uint8                                 tag_dei_ndx,
    SOC_SAND_IN  uint8                                   out_pcp,
    SOC_SAND_IN  uint8                                 out_dei
  );

uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_stag_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_pcp_ndx,
    SOC_SAND_IN  uint8                                 tag_dei_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_stag_set_unsafe" API.
 *     Refer to
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_stag_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_stag_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_pcp_ndx,
    SOC_SAND_IN  uint8                                 tag_dei_ndx,
    SOC_SAND_OUT uint8                                   *out_pcp,
    SOC_SAND_OUT uint8                                 *out_dei
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_pcp_map_ctag_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from outer C-tag UP to the PCP and DEI
 *   values to be set in the transmitted packet's tag. This
 *   is the mapping to be used when the incoming packet is
 *   C-tagged (outer Tag.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  pcp_profile_ndx -
 *     The PCP profile is set according to AC. Range: 0 - 7.
 *   SOC_SAND_IN  uint8                                   tag_up_ndx -
 *     The UP appears in the outer C-Tag of the packet. Range:
 *     0 - 7.
 *   SOC_SAND_IN  uint8                                   out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     presents the UP to set in the packet tag. Range: 0 - 7.
 *   SOC_SAND_IN  uint8                                 out_dei -
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
  arad_pp_lif_ing_vlan_edit_pcp_map_ctag_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_up_ndx,
    SOC_SAND_IN  uint8                                   out_pcp,
    SOC_SAND_IN  uint8                                 out_dei
  );

uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_ctag_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_up_ndx,
    SOC_SAND_IN  uint8                                   out_pcp,
    SOC_SAND_IN  uint8                                 out_dei
  );

uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_ctag_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_up_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_ctag_set_unsafe" API.
 *     Refer to
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_ctag_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_ctag_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tag_up_ndx,
    SOC_SAND_OUT uint8                                   *out_pcp,
    SOC_SAND_OUT uint8                                 *out_dei
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_pcp_map_untagged_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from COS parameters (DP and TC) to the PCP
 *   and DEI values to be set in the transmitted packet's
 *   tag. This is the mapping to be used when the incoming
 *   packet has no tags.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  pcp_profile_ndx -
 *     The PCP profile is set according to AC. Range: 0 - 7.
 *   SOC_SAND_IN  uint8                                   tc_ndx -
 *     Traffic Class. Calculated at the ingress. See COS
 *     module. Range: 0 - 7.
 *   SOC_SAND_IN  uint8                                   dp_ndx -
 *     Drop Precedence. Calculated at the ingress. See COS
 *     module. Range: 0 - 3.
 *   SOC_SAND_IN  uint8                                   out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     represents the UP to set in the packet C-tag. Range: 0 -
 *     7.
 *   SOC_SAND_IN  uint8                                 out_dei -
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
  arad_pp_lif_ing_vlan_edit_pcp_map_untagged_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tc_ndx,
    SOC_SAND_IN  uint8                                   dp_ndx,
    SOC_SAND_IN  uint8                                   out_pcp,
    SOC_SAND_IN  uint8                                 out_dei
  );

uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_untagged_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tc_ndx,
    SOC_SAND_IN  uint8                                   dp_ndx,
    SOC_SAND_IN  uint8                                   out_pcp,
    SOC_SAND_IN  uint8                                 out_dei
  );

uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_untagged_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tc_ndx,
    SOC_SAND_IN  uint8                                   dp_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_untagged_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_untagged_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_untagged_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pcp_profile_ndx,
    SOC_SAND_IN  uint8                                   tc_ndx,
    SOC_SAND_IN  uint8                                   dp_ndx,
    SOC_SAND_OUT uint8                                   *out_pcp,
    SOC_SAND_OUT uint8                                 *out_dei
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_pcp_map_dp_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from ingress DP to  PCP and DEI 
 *    to be set in the transmitted packet's tag.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                 dp_profile_ndx -
 *     The DP profile is set according to AC. Range: 0 - 3.
 *   SOC_SAND_IN  uint8                                   dp_ndx -
 *     The ingress DP of the packet. Range:
 *     0 - 3.
 *   SOC_SAND_IN  uint8                                 sel_pcp_dei -
 *     The PCP-DEI selected by IVE Range:
 *     0 - 15.
 *   SOC_SAND_IN  uint8                                   out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     presents the UP to set in the packet tag. Range: 0 - 7.
 *   SOC_SAND_IN  uint8                                 out_dei -
 *     The mapped DEI to set in the transmitted packet header.
 *     Range: 0 - 1.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_dp_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  dp_profile_ndx,
    SOC_SAND_IN  uint32                                   dp_ndx,
    SOC_SAND_IN  uint32                                sel_pcp_dei,
    SOC_SAND_IN  uint8                                   out_pcp,
    SOC_SAND_IN  uint8                                 out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_dp_set_unsafe" API.
 *     Refer to
 *     "arad_pp_lif_ing_vlan_edit_pcp_map_dp_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_lif_ing_vlan_edit_pcp_map_dp_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  dp_profile_ndx,
    SOC_SAND_IN  uint32                                   dp_ndx,
    SOC_SAND_IN  uint32                                 sel_pcp_dei,
    SOC_SAND_OUT uint8                                   *out_pcp,
    SOC_SAND_OUT uint8                                 *out_dei
  );


/*********************************************************************
*     Set mapping from IN-LIF profile to inner/outer DP profile 
 *     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
  arad_pp_lif_ing_vlan_edit_inner_global_info_set(
    SOC_SAND_IN int unit,
   SOC_SAND_IN uint32 in_lif_profile,
   SOC_SAND_IN int value
  );

uint32
  arad_pp_lif_ing_vlan_edit_outer_global_info_set(
    SOC_SAND_IN int unit,
   SOC_SAND_IN uint32 in_lif_profile,
   SOC_SAND_IN int value
  );

uint32
  arad_pp_lif_ing_vlan_edit_inner_global_info_get(
    SOC_SAND_IN int unit,
   SOC_SAND_IN uint32 in_lif_profile,
   SOC_SAND_OUT int *value
  );

uint32
  arad_pp_lif_ing_vlan_edit_outer_global_info_get(
    SOC_SAND_IN int unit,
   SOC_SAND_IN uint32 in_lif_profile,
   SOC_SAND_OUT int *value
  );


/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_lif_ing_vlan_edit module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_lif_ing_vlan_edit_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_lif_ing_vlan_edit_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_lif_ing_vlan_edit module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_lif_ing_vlan_edit_get_errs_ptr(void);

uint32
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO *info
  );

uint32
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY_verify(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY *info
  );

uint32
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LIF_ING_VLAN_EDIT_INCLUDED__*/
#endif



