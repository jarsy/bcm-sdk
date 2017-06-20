/* $Id: ppd_api_eg_vlan_edit.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_eg_vlan_edit.h
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

#ifndef __SOC_PPD_API_EG_VLAN_EDIT_INCLUDED__
/* { */
#define __SOC_PPD_API_EG_VLAN_EDIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_eg_vlan_edit.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

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
  SOC_PPD_EG_VLAN_EDIT_PEP_INFO_SET = SOC_PPD_PROC_DESC_BASE_EG_VLAN_EDIT_FIRST,
  SOC_PPD_EG_VLAN_EDIT_PEP_INFO_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PEP_INFO_GET,
  SOC_PPD_EG_VLAN_EDIT_PEP_INFO_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_COMMAND_INFO_SET,
  SOC_PPD_EG_VLAN_EDIT_COMMAND_INFO_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_COMMAND_INFO_GET,
  SOC_PPD_EG_VLAN_EDIT_COMMAND_INFO_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PORT_VLAN_TRANSMIT_OUTER_TAG_SET,
  SOC_PPD_EG_VLAN_EDIT_PORT_VLAN_TRANSMIT_OUTER_TAG_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PORT_VLAN_TRANSMIT_OUTER_TAG_GET,
  SOC_PPD_EG_VLAN_EDIT_PORT_VLAN_TRANSMIT_OUTER_TAG_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_PROFILE_INFO_SET,
  SOC_PPD_EG_VLAN_EDIT_PCP_PROFILE_INFO_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_PROFILE_INFO_GET,
  SOC_PPD_EG_VLAN_EDIT_PCP_PROFILE_INFO_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_STAG_SET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_STAG_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_STAG_GET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_STAG_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_CTAG_SET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_CTAG_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_CTAG_GET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_CTAG_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_UNTAGGED_SET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_UNTAGGED_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_UNTAGGED_GET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_UNTAGGED_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_DSCP_EXP_GET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_DSCP_EXP_GET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_DSCP_EXP_SET,
  SOC_PPD_EG_VLAN_EDIT_PCP_MAP_DSCP_EXP_SET_PRINT,
  SOC_PPD_EG_VLAN_EDIT_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_EG_VLAN_EDIT_PROCEDURE_DESC_LAST
} SOC_PPD_EG_VLAN_EDIT_PROCEDURE_DESC;

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
 *   soc_ppd_eg_vlan_edit_pep_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the editing information for PEP port.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key -
 *     The key for this PEP editing, out-port x VSI.
 *   SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_PEP_INFO               *pep_info -
 *     The editing information for PEP port, including pep-pvid
 *     and UP, and profiles to set the VID and UP.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pep_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_PEP_INFO               *pep_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_vlan_edit_pep_info_set" API.
 *     Refer to "soc_ppd_eg_vlan_edit_pep_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pep_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_PEP_KEY                             *pep_key,
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_PEP_INFO               *pep_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_vlan_edit_command_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Edit command to perform over packets according
 *   to VLAN tags format and Edit profile.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY            *command_key -
 *     Command key including: - packet tag format: specifies
 *     which Tags found on the packet.- Edit profile: assigned
 *     to packet.
 *   SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO           *command_info -
 *     Edit command to perform on the packets, including number
 *     of tags to remove, and tags to build.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_command_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY            *command_key,
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO           *command_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_vlan_edit_command_info_set" API.
 *     Refer to "soc_ppd_eg_vlan_edit_command_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_command_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY            *command_key,
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO           *command_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   For port x VLAN, set whether to transmit packets from a
 *   specific port tagged or not, according to the outer-tag
 *   VID.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     The outer VID. Range: 0 - 4095.
 *   SOC_SAND_IN  uint8                               transmit_outer_tag -
 *     If TRUE, then transmit a packet with outer-tag. If FALSE,
 *     remove the outer tag from the packet header.
 * REMARKS:
 *   - It is ensured that packet has an outer tag either
 *   arriving on the packet or as a result of the former
 *   (egress) editing process.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_IN  uint8                               transmit_outer_tag
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set" API.
 *     Refer to
 *     "soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_OUT uint8                               *transmit_outer_tag
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_vlan_edit_pcp_profile_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   For PCP profile set the key used for mapping. When the
 *   Edit Command is set to determine the PCP-DEI value
 *   according to mapping, then the key used for mapping may
 *   be PCP-DEI, UP or DP and TC.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pcp_profile_ndx -
 *     The PCP profile is set according to Out-AC setting.
 *     Range: 0 - 7.
 *   SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY    key_mapping -
 *     How to build the key to be mapped to get out-PCP-DEI
 *     (either using packet Priority attributes (from VLAN TAGS
 *     UP/PCP/DEI) -if exist- or according to TC/DP)
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_profile_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY    key_mapping
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_vlan_edit_pcp_profile_info_set" API.
 *     Refer to "soc_ppd_eg_vlan_edit_pcp_profile_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_profile_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY    *key_mapping
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_vlan_edit_pcp_map_stag_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from outer S-tag PCP and DEI to the PCP and
 *   DEI values to be set in the transmitted packet's tag.
 *   This is the mapping to be used when the incoming packet
 *   is S-tagged (outer Tag) and pcp profile set to use
 *   packet's attribute.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pcp_profile_ndx -
 *     The PCP profile is set according to OUT-AC setting.
 *     Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_pcp_ndx -
 *     The PCP appears in the outer S-Tag of the packet. Range:
 *     0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         tag_dei_ndx -
 *     The DEI appears in the outer S-Tag of the packet. 
 *     Range: Soc_petraB: 0 - 1, ARAD: 0-3.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     presents the UP to set in the packet tag. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei -
 *     The mapped DEI to set in the transmitted packet header.
 *     This is relevant when used for S-Tag. When the mapping
 *     used to build C tag, this value has to be set to 0.
 *     Range: Soc_petraB: 0 - 1, ARAD: 0-3.
 * REMARKS:
 *   - This mapping is used when the outer packet tag is
 *   S-tag and pcp profile set to use packet's attribute.-
 *   When this mapping is used to build the C-tag, then the
 *   out-PCP is actually the UP, and the DEI has to be 0.-
 *   This value of PCP and DEI will be used when the source
 *   of PCP DEI is selected to be
 *   SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_MAP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_map_stag_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         tag_dei_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_vlan_edit_pcp_map_stag_set" API.
 *     Refer to "soc_ppd_eg_vlan_edit_pcp_map_stag_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_map_stag_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_pcp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         tag_dei_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_vlan_edit_pcp_map_ctag_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from outer C-tag UP to the PCP and DEI
 *   values to be set in the transmitted packet's tag. This
 *   is the mapping to be used when the incoming packet is
 *   C-tagged (outer Tag) and pcp profile set to use packet's
 *   attribute.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pcp_profile_ndx -
 *     The PCP profile is set according to OUT-AC setting.
 *     Range: 0 - 7.
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
 *     Range: Soc_petraB: 0 - 1, ARAD: 0-3.
 * REMARKS:
 *   - This mapping is used when the outer packet tag is
 *   C-tag and pcp profile set to use packet's attribute.-
 *   When this mapping is used to build the C-tag, then the
 *   out-PCP is actually the UP, and the DEI has to be 0.-
 *   This value of PCP and DEI will be used when the source
 *   of PCP DEI is selected to be
 *   SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_MAP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_map_ctag_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_up_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_vlan_edit_pcp_map_ctag_set" API.
 *     Refer to "soc_ppd_eg_vlan_edit_pcp_map_ctag_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_map_ctag_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          tag_up_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_vlan_edit_pcp_map_untagged_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from COS parameters (DP and TC) to the PCP
 *   and DEI values to be set in the transmitted packet's
 *   tag. This is the mapping to be used when the incoming
 *   packet has no tags or pcp profile is set to use TC and
 *   DP for the mapping.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pcp_profile_ndx -
 *     The PCP profile is set according to OUT-AC setting.
 *     Range: 0 - 7.
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
 *     Range: Soc_petraB: 0 - 1, ARAD: 0-3.
 * REMARKS:
 *   - This mapping is used when the packet has no Tags or
 *   pcp profile set to use TC and DP for the mapping.- When
 *   the Tag to build is C-Tag, then the out-PCP is actually
 *   the UP, and the DEI has to be 0.- This value of PCP and
 *   DEI will be used when the source of PCP DEI is selected
 *   to be SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_MAP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_map_untagged_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_vlan_edit_pcp_map_untagged_set" API.
 *     Refer to "soc_ppd_eg_vlan_edit_pcp_map_untagged_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_vlan_edit_pcp_map_untagged_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );


/*********************************************************************
* NAME:
 *   soc_ppd_eg_vlan_edit_pcp_map_dscp_exp_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from COS parameters to the PCP and DEI values
 *   to be set in the transmitted packet's tag.
 *   This is the mapping to be used when the incoming
 *   packet is set to use dscp and exp for the mapping.
 *
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PKT_HDR_TYPE                      pkt_type_ndx -
 *      The packet header type.
 *   SOC_SAND_IN  SOC_SAND_U*                                   dscp_exp_ndx -
 *     Dscp/exp. Calculated at the ingress. See COS
 *     module. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                            out_pcp -
 *     The mapped PCP to set in the transmitted packet header.
 *     When the mapping used to build C-tag, this value
 *     represents the UP to set in the packet C-tag. Range: 0 -
 *     7.
 *   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                           out_dei -
 *     The mapped DEI to set in the transmitted packet header.
 *     This is relevant when used for S-Tag. When the mapping
 *     used to build C tag, this value has to be set to 0.
 *     Range: Soc_petraB: 0 - 1, ARAD: 0-3.
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
  soc_ppd_eg_vlan_edit_pcp_map_dscp_exp_set( 
    SOC_SAND_IN int                                  unit, 
    SOC_SAND_IN SOC_PPC_PKT_FRWRD_TYPE                     pkt_type_ndx, 
    SOC_SAND_IN uint8                                    dscp_exp_ndx, 
    SOC_SAND_IN SOC_SAND_PP_PCP_UP                             out_pcp, 
    SOC_SAND_IN SOC_SAND_PP_DEI_CFI                            out_dei 
  );

/*********************************************************************
 * SOC_SAND_IN uint8 dscp_exp_ndx - 
 * The DSCP-EXP value, considering the egress remark, 
 * SOC_SAND_IN SOC_PPC_PKT_HDR_TYPE pkt_type_ndx, 
 * header type (Ethernet, IPv4, etc..) used for calculation/remarking dscp_exp 
 * generally L3 header on top of L2 header 
 * Refer to "soc_ppd_eg_vlan_edit_pcp_map_dscp_exp_set" API
 * for details.   
*********************************************************************/
uint32 
  soc_ppd_eg_vlan_edit_pcp_map_dscp_exp_get( 
    SOC_SAND_IN int                                  unit, 
    SOC_SAND_IN SOC_PPC_PKT_FRWRD_TYPE                     pkt_type_ndx, 
    SOC_SAND_IN uint8                                    dscp_exp_ndx, 
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                            *out_pcp, 
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                           *out_dei 
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_EG_VLAN_EDIT_INCLUDED__*/
#endif

