/* $Id: ppc_api_llp_parse.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_llp_parse.h
*
* MODULE PREFIX:  soc_ppc_llp
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

#ifndef __SOC_PPC_API_LLP_PARSE_INCLUDED__
/* { */
#define __SOC_PPC_API_LLP_PARSE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_trap_mgmt.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define SOC_PPC_TPID_PROFILE_NDX_MIN                             (0)
#define SOC_PPC_TPID_PROFILE_NDX_MAX                             (7)
#define SOC_PPC_TPID_PROFILE_ID_MIN                              (0)
#define SOC_PPC_TPID_PROFILE_ID_MAX                              (3)

/*     Number of global TPID values in device.                 */
#define  SOC_PPC_LLP_PARSE_NOF_TPID_VALS (5)


#define SOC_PPC_LLP_PARSE_FLAGS_OUTER_C   (1)
#define SOC_PPC_LLP_PARSE_FLAGS_INNER_C   (2)

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
   *  Indicates that none of the TPIDs assigned to the port
   *  was found on the packet.
   */
  SOC_PPC_LLP_PARSE_TPID_INDEX_NONE = 0,
  /*
   *  Indicates that tpid1 was found on the packet. tpid1 as
   *  set by soc_ppd_llp_parse_tpid_profile_info_set() for the
   *  relvant tpid profle
   */
  SOC_PPC_LLP_PARSE_TPID_INDEX_TPID1 = 1,
  /*
   *  Indicates that tpid2 was found on the packet. tpid2 as
   *  set by soc_ppd_llp_parse_tpid_profile_info_set()
   */
  SOC_PPC_LLP_PARSE_TPID_INDEX_TPID2 = 2,
  /*
   *  Indicates that ISID-TPID was found on the packet.
   *  ISID-TPID as set by soc_ppd_llp_parse_tpid_values_set()
   */
  SOC_PPC_LLP_PARSE_TPID_INDEX_ISID_TPID = 3,
  /*
   *  Number of types in SOC_PPC_LLP_PARSE_TPID_INDEX
   */
  SOC_PPC_NOF_LLP_PARSE_TPID_INDEXS = 4
}SOC_PPC_LLP_PARSE_TPID_INDEX;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Device TPID values. Used for Link-layer parsing to
   *  identify the VLAN tags on the packets. Also used to
   *  build the VLAN tags in egress. The last TPID in the
   *  array is used for ISID tag processing (ISID-TPID)Range:
   *  0 - 0xFFFF.
   */
  SOC_SAND_PP_TPID tpid_vals[SOC_PPC_LLP_PARSE_NOF_TPID_VALS];

} SOC_PPC_LLP_PARSE_TPID_VALUES;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Selects one of four global TPIDs. Used to parse the VLAN
   *  Tags in the ingress/egress and to construct the VLAN
   *  Tags in the egress. Range: 0 - 3.
   */
  uint8 index;
  /*
   *  T20E only. Is C-tag or S-Tag. S-tag composed of TPID,
   *  PCP, DEI and VID. C-tag composed of TPID, UP, CFI and
   *  VID. For Soc_petra-B has to be NONE and user may use
   *  soc_ppd_llp_parse_packet_format_info_set() to set the TPID
   *  type S, C ,etc...
   */
  SOC_SAND_PP_VLAN_TAG_TYPE type;

} SOC_PPC_LLP_PARSE_TPID_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  TPID-1 to identify VLAN tag in the packet's header. For
   *  Soc_petra-B only index is relevant.
   */
  SOC_PPC_LLP_PARSE_TPID_INFO tpid1;
  /*
   *  TPID-2 to identify VLAN tag in the packet's header. For
   *  Soc_petra-B only index is relevant.
   */
  SOC_PPC_LLP_PARSE_TPID_INFO tpid2;

} SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Indicates what is the outer TPID of the packet (tpid1,
   *  tpid2 or none). Range ...
   */
  SOC_PPC_LLP_PARSE_TPID_INDEX outer_tpid;
  /*
   *  Indicates what is the inner TPID of the packet (tpid1,
   *  tpid2 or none). Relevant only for double tagged packets
   *  (when outer-tpid is note none)
   */
  SOC_PPC_LLP_PARSE_TPID_INDEX inner_tpid;
  /*
   *  Set to TRUE to indicate that the outer Vlan-tag is
   *  priority-tag i.e. VLAN tag with VLAN ID equal to 0.
   */
  uint8 is_outer_prio;

} SOC_PPC_LLP_PARSE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR


   /* see SOC_PPC_LLP_PARSE_FLAGS */
   uint32 flags;
  /*
   *  Trap-code according to which to process/drop/forward the
   *  packet. Use trap code
   *  SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_DROP to drop
   *  packet. Use trap code
   *  SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_ACCEPT to accept
   *  packet.use SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_CUSTOM1
   *  and SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_CUSTOM2 for
   *  custom actions, to configure what action to perform use
   *  soc_ppd_trap_frwrd_profile_info_set()
   */
  SOC_PPC_TRAP_CODE action_trap_code;
  /*
   *  VLAN format of the packet, Identifies the VLAN tags
   *  structure on the packet. Used by vlan editing see
   *  soc_ppd_lif_ing_vlan_edit_command_id_set(). Besides affects
   *  the initial cos assignment:- when outer tag is of type
   *  'S' then the DP value consider the DEI from the tag.
   */
  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT tag_format;
  /*
   *  Should priority Tag considered as C-Tag or S-tag. Range:
   *  SOC_SAND_PP_VLAN_TAG_TYPE_NONE - SOC_SAND_PP_VLAN_TAG_TYPE_STAG.
   *  Relevant only when is_outer_prio = TRUE, otherwise
   *  should be set to SOC_SAND_PP_VLAN_TAG_TYPE_NONE.
   */
  SOC_SAND_PP_VLAN_TAG_TYPE priority_tag_type;
  /*
   *  Default Ingress VLAN edit command ID:Handle used to set
   *  the Vlan edit command information. See
   *  soc_ppd_lif_ing_vlan_edit_command_info_set()This is the
   *  default command packet will be edited according if AC
   *  lookup fails, otherwise the edit-command will be set
   *  according to AC.
   */
  uint32 dlft_edit_command_id;
  /*
   *  Default PCP profile, used during ingress VLAN editing
   *  phase, to set the PCP/UP of the edited VLAN headers. See
   *  soc_ppd_ing_vlan_edit_command_info_set (). This is the
   *  default PCP profile packet will be edited according if
   *  AC lookup fails, otherwise the pcp-profile will be set
   *  according to AC.
   */
  uint32 dflt_edit_pcp_profile;
  /* 
   *  Set initial-VID for the inner_tag and not outer_tag In case packet is S-tag.
   *  To do that API set incoming-vid-exist to FALSE     .
   *  Valid for ARAD only.                                                   .
   */
  uint8 initial_c_tag;

} SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO;


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
  SOC_PPC_LLP_PARSE_TPID_VALUES_clear(
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_VALUES *info
  );

void
  SOC_PPC_LLP_PARSE_TPID_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_INFO *info
  );

void
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO *info
  );

void
  SOC_PPC_LLP_PARSE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_INFO *info
  );

void
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LLP_PARSE_TPID_INDEX_to_string(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_INDEX enum_val
  );

void
  SOC_PPC_LLP_PARSE_TPID_VALUES_print(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES *info
  );

void
  SOC_PPC_LLP_PARSE_TPID_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_INFO *info
  );

void
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO *info
  );

void
  SOC_PPC_LLP_PARSE_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO *info
  );

void
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LLP_PARSE_INCLUDED__*/
#endif

