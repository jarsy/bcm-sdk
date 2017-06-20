/* $Id: ppc_api_eg_vlan_edit.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_eg_vlan_edit.h
*
* MODULE PREFIX:  soc_ppc_eg
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

#ifndef __SOC_PPC_API_EG_VLAN_EDIT_INCLUDED__
/* { */
#define __SOC_PPC_API_EG_VLAN_EDIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/dpp_config_defs.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* ARAD specific values */

/* The Egress mapping is done from bits of edit profile and 4bits of tag format to a command IDs that comprises both */
#define SOC_PPC_NOF_EGRESS_TAG_FORMAT_BITS                      (4)
#define SOC_PPC_NOF_EGRESS_VLAN_EDIT_ACTION_MAPPINGS(unit)      (0x1 << ((SOC_DPP_DEFS_GET(unit, nof_eve_profile_bits)) + SOC_PPC_NOF_EGRESS_TAG_FORMAT_BITS))
#define SOC_PPC_NOF_EGRESS_VLAN_EDIT_ACTION_IDS(unit)           (SOC_PPC_NOF_EGRESS_VLAN_EDIT_ACTION_MAPPINGS(unit))

/* Two Egress actions are reserved for defaults: 0 for untagged packets and 1 for tagged packets */
#define SOC_PPC_NOF_EGRESS_VLAN_EDIT_RESERVED_ACTION_IDS_ARAD   (2)

/* Maximum values for all the devices */
#define SOC_PPC_MAX_NOF_EGRESS_VLAN_EDIT_ACTION_MAPPINGS        (0x1 << ((SOC_DPP_DEFS_MAX(NOF_EVE_PROFILE_BITS)) + SOC_PPC_NOF_EGRESS_TAG_FORMAT_BITS))
#define SOC_PPC_MAX_NOF_EGRESS_VLAN_EDIT_ACTION_IDS             (SOC_PPC_MAX_NOF_EGRESS_VLAN_EDIT_ACTION_MAPPINGS)

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
   *  Indicates that tag will not be generated. The outer tag
   *  cannot be empty. To transmit a packet with no tags, see
   *  soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set().
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_EMPTY = 0,
  /*
   *  The VID value is taken from the outer tag of the packet
   *  (considering the ingress editing effect).
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_OUTER_TAG = 1,
  /*
   *  The VID value is taken from the inner tag of the packet
   *  (considering the ingress editing effect)
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_INNER_TAG = 2,
  /*
   *  The VID value is taken from the edit information
   *  configured (vlan_tags[0]). See soc_ppd_eg_ac_info_set() In
   *  T20E: This can be the source only of the outer tag
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_EDIT_INFO_0 = 3,
  /*
   *  The VID value is taken from the edit information
   *  configured (vlan_tags[1]). See soc_ppd_eg_ac_info_set() In
   *  T20E: This can be the source only of the inner tag
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_EDIT_INFO_1 = 4,
  /*
   *  Soc_petra-B only. The VID value is taken from the configured
   *  link-layer encapsulation information. This VID may be
   *  used only if it valid. See soc_ppd_eg_encap_ll_entry_add().
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_ENCAP_INFO = 5,
  /*
   *  T20E only. The VID value is taken from the editing
   *  information configured by soc_ppd_eg_ac_port_cvid_info_set
   *  () This is relevant only for editing packets transmitted
   *  out from CEP port. can be use only the source of outer
   *  Tag
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_CEP_EDIT_INFO = 6,
  /*
   *  T20E only. This is relevant only for editing packets
   *  transmitted out from CEP port. In this case, the CVID
   *  value is set according to the PEP editing, which may be:
   *  1. The C-VID appears in the outer-tag, if exists.2. The
   *  C-VID appears in the inner-tag, if exists.3. The
   *  PEP-PVID, if none of the previous.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_PEP_EDIT = 7,
  /*
   *  Number of types in SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC in Soc_petraB
   */
  SOC_PPC_NOF_EG_VLAN_EDIT_TAG_VID_SRCS_PB = 8,
  /* 
   * Arad only. The VID value is equal to VSI 12 ls bits 
   */ 
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_VSI = SOC_PPC_NOF_EG_VLAN_EDIT_TAG_VID_SRCS_PB,
  /*
   *  Number of types in SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC in ARAD
   */
  SOC_PPC_NOF_EG_VLAN_EDIT_TAG_VID_SRCS_ARAD = 9
}SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC;

typedef enum
{
  /*
   *  Use when no VLAN tag is generated. In T20E, use also for
   *  inner tag of CEP port (which should not be exist).
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_NONE = 0,
  /*
   *  The PCP (UP) and DEI values are set according to
   *  mapping. See PCP Mapping Setting
   *  (soc_ppd_eg_vlan_edit_pcp_map_stag_set(),
   *  soc_ppd_eg_vlan_edit_pcp_map_ctag_set() and
   *  soc_ppd_eg_vlan_edit_pcp_map_untagged_set())
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_MAP = 1,
  /*
   *  The PCP (UP) and DEI values are taken from the outer
   *  tag.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_OUTER_TAG = 2,
  /*
   *  The PCP (UP) and DEI values are taken from the inner
   *  tag.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_INNER_TAG = 3,
  /*
   *  T20E only. The PCP (UP) and DEI values are taken from the
   *  editing information configured by soc_ppd_eg_ac_info_set()
   *  (vlan_tags[0]). T20E: this refers to outer Tag-PCP-DEI
   *  i.e. this can be the source of the PCP-DEI of outer-tag
   *  only.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_EDIT_INFO_0 = 4,
  /*
   *  T20E only. The PCP (UP) and DEI values are taken from the
   *  editing information configured by soc_ppd_eg_ac_info_set()
   *  (vlan_tags[1]). T20E: this refers to inner Tag-PCP-DEI
   *  i.e. this can be the source of the PCP-DEI of inner-tag
   *  only.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_EDIT_INFO_1 = 5,
  /*
   *  T20E only. The PCP (UP) and DEI values are taken from
   *  the editing information configured by
   *  soc_ppd_eg_ac_port_cvid_info_set () This is relevant only
   *  for editing packets transmitted out from CEP port. Can
   *  be used only the source of outer Tag
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_CEP_EDIT_INFO = 6,
  /*
   *  T20E only. This is relevant only for editing packets
   *  transmitted out from the CEP port. In this case, the
   *  PCP/UP value will be set according to the PEP editing,
   *  which may be: 1. The UP appears in the outer-tag, if
   *  exists.2. The UP appears in the inner-tag, if exists.3.
   *  The PEP-UP, if none of the previous.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_PEP_EDIT = 7,
  /*
   *  Number of types in SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC
   */
  SOC_PPC_NOF_EG_VLAN_EDIT_TAG_PCP_DEI_SRCS = 8
}SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC;

typedef enum
{
  /*
   *  When the Edit Command is set to determine the PCP-DEI
   *  value according to mapping, then the key used for
   *  mapping is TC and DP as calculated.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY_TC_DP = 0,
  /*
   *  When the Edit Command is set to determine the PCP-DEI
   *  value according to mapping, then the key used for
   *  mapping is packets attributes, which depends on Tags
   *  exist on the packet:1. Packet's PCP-DEI, if packet has
   *  S-tag.2. Packets UP if packet has C-tag. 3. TC and DP if
   *  there is no C/S Tags on the packet
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY_PCP = 1,
  /*
   *  Number of types in SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY in Soc_petraB
   */
  SOC_PPC_NOF_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEYS_PB = 2,
  /* 
   * Arad only. 
   * When the Edit Command is set to determine the PCP-DEI 
   * value according to mapping, then the key used for 
   * mapping is DSCP-EXP as calculated at ingress 
   * DSCP-EXP value considered after "DSCP Remark" phase 
   */ 
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY_DSCP_EXP = SOC_PPC_NOF_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEYS_PB, 
  /*
   *  Number of types in SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY
   */
  SOC_PPC_NOF_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEYS_ARAD = 3

}SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  PEP-PVID. C-VLAN ID to assign for untagged packet.
   *  Toward the C-Component. Range: 0 - 4095.
   */
  SOC_SAND_PP_VLAN_ID pvid;
  /*
   *  User priority to set in the packet header. By
   *  soc_ppd_eg_vlan_edit_command_info_set(), the user can set
   *  the UP/PCP of the packet to this value, or to map it
   *  according to COS attributes (see pcp_profile). Range: 0
   *  - 7.
   */
  SOC_SAND_PP_PCP_UP up;
  /*
   *  Profile used with C-TAG for egress C-TAG editing. Used
   *  to give different treatment for packets in the same
   *  C-Component, depending to the PEP they pass from.
   *  Relevant only for: T20E. For Soc_petra-B has to be
   *  zero. Range: 0 - 3.
   */
  uint32 pep_edit_profile;

} SOC_PPC_EG_VLAN_EDIT_PEP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Packet frame type (VLAN tags stack). If ingress editing
   *  was performed over the packet, then this is the internal
   *  tag format (packet format after ingress editing).
   *  Otherwise, this is the packet incoming tag Format.
   */
  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT tag_format;
  /*
   *  Profile according to which to edit the VLAN tags. This
   *  profile is set according to the port setting.Range:
   *  Up till Jericho:      0 - 15
   *  Jericho and above:    0 - 31.
   */ 
  uint32 edit_profile;

} SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Select one of the four global TPIDs configured by
   *  soc_ppd_llp_parse_tpid_value_set() as the TPID of the tag of
   *  the packet (if exists)
   */
  uint32 tpid_index;
  /*
   *  The source to get the VID value from.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC vid_source;
  /*
   *  The source to get the PCP-DEI/UP value from. If
   *  vid_source is empty, then this value is not relevant.
   */
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC pcp_dei_source;

} SOC_PPC_EG_VLAN_EDIT_COMMAND_TAG_BUILD_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Number of VLAN tags to remove from the incoming packet
   *  (after the ingress editing, if present)0 - none.1 - to
   *  remove outer tag.2 - to remove outer and inner tags. In
   *  the editing process, first these tags are removed and
   *  then the inner tag and outer tag are built according to
   *  the fields below. Range: 0 - 2.
   */
  uint8 tags_to_remove;
  /*
   *  Information regard building/editing the Inner Tag.
   */
  SOC_PPC_EG_VLAN_EDIT_COMMAND_TAG_BUILD_INFO inner_tag;
  /*
   *  Information regard building/editing the external Tag.
   */
  SOC_PPC_EG_VLAN_EDIT_COMMAND_TAG_BUILD_INFO outer_tag;
  /*
   *  T20E only. Whether this command is used to edit packets
   *  outgoing from CEP port.
   */
  uint8 cep_editing;
  /*
   *  If True, enable ETAG support in EVE
   */
  uint8 is_extender;
  /*
   *  Membership Check Type (in VLAN Mode, 1=Tagged-packet,  0=Un-Tagged-Packet)
   *  Relevant for Jericho_Plus and above.
   */
  uint32 packet_is_tagged_after_eve;

} SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO;


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

void
  SOC_PPC_EG_VLAN_EDIT_PEP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_PEP_INFO *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_COMMAND_TAG_BUILD_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_COMMAND_TAG_BUILD_INFO *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC_to_string(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_TAG_VID_SRC enum_val
  );

const char*
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC_to_string(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_SRC enum_val
  );

const char*
  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY_to_string(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_TAG_PCP_DEI_MAP_KEY enum_val
  );

void
  SOC_PPC_EG_VLAN_EDIT_PEP_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_PEP_INFO *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY_print(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_KEY *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_COMMAND_TAG_BUILD_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_TAG_BUILD_INFO *info
  );

void
  SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_COMMAND_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_EG_VLAN_EDIT_INCLUDED__*/
#endif

