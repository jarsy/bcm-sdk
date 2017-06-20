/* $Id: arad_pp_llp_filter.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_LLP_FILTER_INCLUDED__
/* { */
#define __ARAD_PP_LLP_FILTER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_llp_filter.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_SIZE                (8)

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
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_SET = ARAD_PP_PROC_DESC_BASE_LLP_FILTER_FIRST,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_SET_PRINT,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_SET_UNSAFE,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_SET_VERIFY,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_GET,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_GET_PRINT,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_GET_VERIFY,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_GET_UNSAFE,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_ADD,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_ADD_PRINT,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_ADD_UNSAFE,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_ADD_VERIFY,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_REMOVE,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_REMOVE_PRINT,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_REMOVE_UNSAFE,
  ARAD_PP_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_REMOVE_VERIFY,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_SET,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_SET_PRINT,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_SET_UNSAFE,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_SET_VERIFY,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_GET,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_GET_PRINT,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_GET_VERIFY,
  ARAD_PP_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_GET_UNSAFE,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_SET,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_SET_PRINT,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_SET_UNSAFE,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_SET_VERIFY,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_GET,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_GET_PRINT,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_GET_VERIFY,
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_GET_UNSAFE,
  ARAD_PP_LLP_FILTER_GET_PROCS_PTR,
  ARAD_PP_LLP_FILTER_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_FILTER_PROCEDURE_DESC_LAST
} ARAD_PP_LLP_FILTER_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LLP_FILTER_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LLP_FILTER_FIRST,
  ARAD_PP_LLP_FILTER_VLAN_FORMAT_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_FILTER_SUCCESS_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_FILTER_ACCEPT_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  SOC_PPC_PORTS_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_UNEXPECTED_TAG_FORMAT_VLAN_FORMAT_ERR,
  ARAD_PP_FILTER_VID_NOT_DESIGNATED_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_FILTER_ERR_LAST
} ARAD_PP_LLP_FILTER_ERR;

typedef struct
{
  uint32 arr[ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_SIZE];
} ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_REF_COUNT;

typedef struct ARAD_PP_LLP_FILTER_s
{
  ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_REF_COUNT ref_count;
} ARAD_PP_LLP_FILTER_t;
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
  arad_pp_llp_filter_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_filter_ingress_vlan_membership_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets ingress VLAN membership; which incoming local ports
 *   belong to the VLAN. Packets received on a port that is
 *   not a member of the VLAN the packet is classified to be
 *   filtered.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx -
 *     VLAN ID to set the membership for. Range: 0 - 4095.
 *   SOC_SAND_IN  SOC_PPC_PORT                                ports -
 *     SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE] - The VLAN membership:
 *     bitmap of VLAN member ports specific information, each
 *     member occupied one bit. Setting bit to 1 indicates port
 *     is member in the VLAN.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_filter_ingress_vlan_membership_set_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  uint32                                  ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  );

uint32
  arad_pp_llp_filter_ingress_vlan_membership_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  uint32                                  ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  );

uint32
  arad_pp_llp_filter_ingress_vlan_membership_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                             vid_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_filter_ingress_vlan_membership_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_llp_filter_ingress_vlan_membership_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_llp_filter_ingress_vlan_membership_get_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_OUT uint32                                  ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_filter_ingress_vlan_membership_port_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add a local port as a member in a VLAN.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port -
 *     the local port to add to the VLAN.
 * REMARKS:
 *   - If the local_port is already member of the vlan
 *   vid_ndx, then the function has no effect.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_filter_ingress_vlan_membership_port_add_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port
  );

uint32
  arad_pp_llp_filter_ingress_vlan_membership_port_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_filter_ingress_vlan_membership_port_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a local port from the VLAN membership.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port -
 *     The local port to add to the VLAN.
 * REMARKS:
 *   - If the local_port is not a member of the vlan vid_ndx,
 *   then the function has no effect.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_filter_ingress_vlan_membership_port_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port
  );

uint32
  arad_pp_llp_filter_ingress_vlan_membership_port_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_filter_ingress_acceptable_frames_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets acceptable frame type on incoming port.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  port_profile_ndx -
 *     Port Profile ID. Range 0 - 7. Set by
 *     soc_ppd_port_info_set().
 *   SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx -
 *     Acceptable frame types (S-tag, double Tags etc.). Use
 *     SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY to affect the
 *     status of all possible vlan Tag formats.
 *   SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile -
 *     Action profile according to which to
 *     process/drop/forward the packet.trap_code range:
 *     0-3.the CPU code attached with the packet is
 *     SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_0/1/2/3
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds (upon add). Operation may
 *     fail if there are no available action profiles pointers.
 * REMARKS:
 *   - use SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY as
 *   vlan_format_ndx to make a decision for ALL VLAN formats
 *   at once.- To set mapping from port to port profile use:
 *   1. soc_ppd_port_info_set(local_port_ndx, port_profile)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_filter_ingress_acceptable_frames_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_llp_filter_ingress_acceptable_frames_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile
  );

uint32
  arad_pp_llp_filter_ingress_acceptable_frames_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_filter_ingress_acceptable_frames_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_llp_filter_ingress_acceptable_frames_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_llp_filter_ingress_acceptable_frames_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT        vlan_format_ndx,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE                      *action_profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_filter_designated_vlan_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set a designated VLAN for a port. Incoming Trill packet
 *   will be checked if it has this T-VID; otherwise, packet
 *   will be dropped.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid -
 *     VID to set as designated VLAN of the Port. Range:
 *     0-4095.
 *   SOC_SAND_IN  uint8                                 accept -
 *     Accept or deny this frame type for this port type
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the set operation succeeded. Operation may fail
 *     if there are no available resources to support the given
 *     (new) VID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_filter_designated_vlan_set_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid,
    SOC_SAND_IN  uint8                                 accept,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_llp_filter_designated_vlan_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid,
    SOC_SAND_IN  uint8                                 accept
  );

uint32
  arad_pp_llp_filter_designated_vlan_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_filter_designated_vlan_set_unsafe" API.
 *     Refer to "arad_pp_llp_filter_designated_vlan_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_llp_filter_designated_vlan_get_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN   int                                core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_SAND_PP_VLAN_ID                           *vid,
    SOC_SAND_OUT uint8                                 *accept
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_filter_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_llp_filter module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_llp_filter_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_llp_filter_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_llp_filter module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_llp_filter_get_errs_ptr(void);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LLP_FILTER_INCLUDED__*/
#endif

