/* $Id: ppd_api_llp_filter.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_llp_filter.h
*
* MODULE PREFIX:  soc_ppd_llp
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

#ifndef __SOC_PPD_API_LLP_FILTER_INCLUDED__
/* { */
#define __SOC_PPD_API_LLP_FILTER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_llp_filter.h>

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
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_SET = SOC_PPD_PROC_DESC_BASE_LLP_FILTER_FIRST,
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_SET_PRINT,
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_GET,
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_GET_PRINT,
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_ADD,
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_ADD_PRINT,
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_REMOVE,
  SOC_PPD_LLP_FILTER_INGRESS_VLAN_MEMBERSHIP_PORT_REMOVE_PRINT,
  SOC_PPD_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_SET,
  SOC_PPD_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_SET_PRINT,
  SOC_PPD_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_GET,
  SOC_PPD_LLP_FILTER_INGRESS_ACCEPTABLE_FRAMES_GET_PRINT,
  SOC_PPD_LLP_FILTER_DESIGNATED_VLAN_SET,
  SOC_PPD_LLP_FILTER_DESIGNATED_VLAN_SET_PRINT,
  SOC_PPD_LLP_FILTER_DESIGNATED_VLAN_GET,
  SOC_PPD_LLP_FILTER_DESIGNATED_VLAN_GET_PRINT,
  SOC_PPD_LLP_FILTER_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LLP_FILTER_PROCEDURE_DESC_LAST
} SOC_PPD_LLP_FILTER_PROCEDURE_DESC;

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
 *   soc_ppd_llp_filter_ingress_vlan_membership_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets ingress VLAN membership; which incoming local ports
 *   belong to the VLAN. Packets received on a port that is
 *   not a member of the VLAN the packet is classified to be
 *   filtered.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     VLAN ID to set the membership for. Range: 0 - 4095.
 *   SOC_SAND_IN  uint32                                ports -
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
  soc_ppd_llp_filter_ingress_vlan_membership_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_IN  uint32                                ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_filter_ingress_vlan_membership_set" API.
 *     Refer to "soc_ppd_llp_filter_ingress_vlan_membership_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_vlan_membership_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_OUT uint32                                ports[SOC_PPC_VLAN_MEMBERSHIP_BITMAP_SIZE]
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_filter_ingress_vlan_membership_port_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add a local port as a member in a VLAN.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
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
  soc_ppd_llp_filter_ingress_vlan_membership_port_add(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN   int                              core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_filter_ingress_vlan_membership_port_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a local port from the VLAN membership.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
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
  soc_ppd_llp_filter_ingress_vlan_membership_port_remove(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN   int                              core_id,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_filter_ingress_acceptable_frames_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets acceptable frame type on incoming port.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                port_profile_ndx -
 *     Port Profile ID. Range 0 - 7. Set by
 *     soc_ppd_port_info_set().
 *   SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT      vlan_format_ndx -
 *     Acceptable frame types (S-tag, double Tags etc.). Use
 *     SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT_ANY to affect the
 *     status of all possible vlan Tag formats.
 *   SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile -
 *     Action profile according to which to
 *     process/drop/forward the packet.trap_code range:
 *     0-3.the CPU code attached with the packet is
 *     SOC_PPC_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_0/1/2/3
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
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
  soc_ppd_llp_filter_ingress_acceptable_frames_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT      vlan_format_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_filter_ingress_acceptable_frames_set" API.
 *     Refer to "soc_ppd_llp_filter_ingress_acceptable_frames_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_llp_filter_ingress_acceptable_frames_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_ETHERNET_FRAME_VLAN_FORMAT      vlan_format_ndx,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE                      *action_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_filter_designated_vlan_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set a designated VLAN for a port. Incoming Trill packet
 *   will be checked if it has this T-VID; otherwise, packet
 *   will be dropped.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid -
 *     VID to set as designated VLAN of the Port. Range:
 *     0-4095.
 *   SOC_SAND_IN  uint8                               accept -
 *     Accept or deny this frame type for this port type
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the set operation succeeded. Operation may fail
 *     if there are no available resources to support the given
 *     (new) VID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_filter_designated_vlan_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN   int                              core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid,
    SOC_SAND_IN  uint8                               accept,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_filter_designated_vlan_set" API.
 *     Refer to "soc_ppd_llp_filter_designated_vlan_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_filter_designated_vlan_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN   int                              core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_SAND_PP_VLAN_ID                         *vid,
    SOC_SAND_OUT uint8                               *accept
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LLP_FILTER_INCLUDED__*/
#endif

