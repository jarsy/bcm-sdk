/* $Id: ppc_api_lif_ing_vlan_edit.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_lif_ing_vlan_edit.h
*
* MODULE PREFIX:  soc_ppc_lif
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

#ifndef __SOC_PPC_API_LIF_ING_VLAN_EDIT_INCLUDED__
/* { */
#define __SOC_PPC_API_LIF_ING_VLAN_EDIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* ARAD specific values */
#define SOC_PPC_NOF_INGRESS_VLAN_EDIT_ACTION_IDS_ARAD           (64)
#define SOC_PPC_NOF_INGRESS_VLAN_EDIT_RESERVED_ACTION_IDS_ARAD  (4)

/* Maximum values for all the devices */
#define SOC_PPC_NOF_INGRESS_VLAN_EDIT_ACTION_IDS                (SOC_PPC_NOF_INGRESS_VLAN_EDIT_ACTION_IDS_ARAD)

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
   *  Indicates that tag will not be generated.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC_EMPTY = 0,
  /*
   *  The VID value is taken from the outer tag of the packet.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC_OUTER_TAG = 1,
  /*
   *  The VID value is taken from the inner tag of the packet.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC_INNER_TAG = 2,
  /*
   *  The VID value is taken from the edit information
   *  configured set to the AC. See soc_ppd_l2_lif_ac_add() - vid
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC_AC_EDIT_INFO = 3,
  /*
   *  Number of types in SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC for Soc_petraB
   */
  SOC_PPC_NOF_LIF_ING_VLAN_EDIT_TAG_VID_SRCS_PB = 4,
  /*
   *  The VID value is taken from the edit information
   *  configured set to the AC. See soc_ppd_l2_lif_ac_add() - vid2
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC_AC_EDIT_INFO_2 = SOC_PPC_NOF_LIF_ING_VLAN_EDIT_TAG_VID_SRCS_PB,
  /*
   *  Number of types in SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC for ARAD
   */
  SOC_PPC_NOF_LIF_ING_VLAN_EDIT_TAG_VID_SRCS_ARAD = 5


}SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC;

typedef enum
{
  /*
   *  Use when no VLAN tag is generated.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_NONE = 0,
  /*
   *  The PCP_DEI value is taken from the outer tag of the
   *  packet.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_OUTER_TAG = 1,
  /*
   *  The PCP_DEI value is taken from the inner tag of the
   *  packet.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_INNER_TAG = 2,
  /*
   *  The PCP (UP) and DEI values are set according to
   *  mapping. See PCP Mapping Setting
   *  (soc_ppd_lif_ing_vlan_edit_pcp_map_stag_set(),
   *  soc_ppd_lif_ing_vlan_edit_pcp_map_ctag_set() and
   *  soc_ppd_lif_ing_vlan_edit_pcp_map_untagged_set()).
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_NEW = 3,
  /*
   *  Number of types in SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC
   */
  SOC_PPC_NOF_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRCS = 4
}SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC;

typedef enum
{
  /*
   *  Use Global configured TPIDs for VLAN Editing.
   */
  SOC_PPC_VLAN_EDIT_TAG_TPID_SRC_GLB_0 = 0,
  SOC_PPC_VLAN_EDIT_TAG_TPID_SRC_GLB_1 = 1,
  SOC_PPC_VLAN_EDIT_TAG_TPID_SRC_GLB_2 = 2,
  SOC_PPC_VLAN_EDIT_TAG_TPID_SRC_GLB_3 = 3,
  /*
   *  Use ingress packet's Outer tpid.
   *  It is available in egress on devices BCM8847X and later only.
   */
  SOC_PPC_VLAN_EDIT_TAG_TPID_SRC_OUTER = 4,
  /*
   *  Use ingress packet's inner tpid.
   *  It is available in egress on devices BCM8847X and later only.
   */
  SOC_PPC_VLAN_EDIT_TAG_TPID_SRC_INNER = 5
}SOC_PPC_VLAN_EDIT_TAG_TPID_SRC;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * Selects one of the global TPID values configured by
   * soc_ppd_llp_parse_tpid_values_set(). Used to build the VLAN
   * tag. Range: 0 - 3.
   */
  uint32 tpid_index;
  /*
   *  The source to get the VID value from.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC vid_source;
  /*
   *  The source to get the PCP-DEI/UP value from. If
   *  vid_source is empty, then this value is not relevant.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC pcp_dei_source;

} SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VLAN frame type (VLAN tags structure) of incoming
   *  packet.
   */
  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT tag_format;
  /*
   *  Profile according to which to edit the VLAN tags. This
   *  profile is set according to in-AC. Range: 0 - 7.
   */
  uint32 edit_profile;

} SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY;

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
   *  Select one of the four global TPID profiles configured
   *  by soc_ppd_llp_parse_tpid_profile_set(). can select only
   *  from TPID profile 0 to TPID profile 3, Besides TPID1 is
   *  considered as outer TPID, TPID2 is considered as Inner
   *  TPID. Range 0-3.
   */
  uint32 tpid_profile;
  /*
   *  Information regard building/editing the Inner Tag.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO inner_tag;
  /*
   *  Information regard building/editing the external Tag.
   */
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO outer_tag;

} SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO;


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
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO *info
  );

void
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY_clear(
    SOC_SAND_OUT SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY *info
  );

void
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC_to_string(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_VID_SRC enum_val
  );

const char*
  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC_to_string(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_TAG_PCP_DEI_SRC enum_val
  );

void
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_TAG_BUILD_INFO *info
  );

void
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY_print(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_KEY *info
  );

void
  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO_print(
    SOC_SAND_IN  SOC_PPC_LIF_ING_VLAN_EDIT_COMMAND_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LIF_ING_VLAN_EDIT_INCLUDED__*/
#endif

