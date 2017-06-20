/* $Id: ppd_api_llp_mirror.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_llp_mirror.h
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

#ifndef __SOC_PPD_API_LLP_MIRROR_INCLUDED__
/* { */
#define __SOC_PPD_API_LLP_MIRROR_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_llp_mirror.h>

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
  SOC_PPD_LLP_MIRROR_PORT_VLAN_ADD = SOC_PPD_PROC_DESC_BASE_LLP_MIRROR_FIRST,
  SOC_PPD_LLP_MIRROR_PORT_VLAN_ADD_PRINT,
  SOC_PPD_LLP_MIRROR_PORT_VLAN_REMOVE,
  SOC_PPD_LLP_MIRROR_PORT_VLAN_REMOVE_PRINT,
  SOC_PPD_LLP_MIRROR_PORT_VLAN_GET,
  SOC_PPD_LLP_MIRROR_PORT_VLAN_GET_PRINT,
  SOC_PPD_LLP_MIRROR_PORT_DFLT_SET,
  SOC_PPD_LLP_MIRROR_PORT_DFLT_SET_PRINT,
  SOC_PPD_LLP_MIRROR_PORT_DFLT_GET,
  SOC_PPD_LLP_MIRROR_PORT_DFLT_GET_PRINT,
  SOC_PPD_LLP_MIRROR_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LLP_MIRROR_PROCEDURE_DESC_LAST
} SOC_PPD_LLP_MIRROR_PROCEDURE_DESC;

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
 *   soc_ppd_llp_mirror_port_vlan_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set a mirroring for port and VLAN, so all incoming
 *   packets enter from the given port and identified with
 *   the given VID will be associated with Mirror command
 *   (Enables mirroring (copying) the packets to additional
 *   destination.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_IN  uint32                                mirror_profile -
 *     Mirroring profile to associate with packets, the
 *     resolution of this mirroring occurs in Soc_petra-TM device.
 *     Range 0-15.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds (upon add). Add operation
 *     may fail if there are no suffiecient resources.
 *     Successful add is guaranteed for up to 6 different VIDs.
 * REMARKS:
 *   - The VID considered is the VID identified as a result
 *   of the ingress parsing, i.e., according to the tag
 *   structure of the incoming packet (and not according to
 *   the initial VID assignment), - In T20E, the mirror
 *   profile is identical to snoop profile. And consequnently
 *   the Snoop command/Profile is transmited to the TM
 *   device.- If during packet processing, the packet was
 *   assigned more than one mirror profile then: - in Soc_petra:
 *   last assignment is considered. - in T20E: the
 *   snoop/mirror command with highest strength is
 *   considered.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_vlan_add(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_IN  uint32                                mirror_profile,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_mirror_port_vlan_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a mirroring for port and VLAN
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_vlan_remove(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_mirror_port_vlan_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the assigned mirroring profile for port and VLAN.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_OUT uint32                                *mirror_profile -
 *     Mirroring profile associated with the port and VLAN.
 * REMARKS:
 *   - If no mirror profile was associated to the Port and
 *   VLAN (by soc_ppd_llp_trap_port_vlan_mirroring_add()), the
 *   default mirror profile is returned according to
 *   soc_ppd_llp_mirror_port_dflt_set().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_vlan_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_OUT uint32                                *mirror_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_mirror_port_dflt_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set default mirroring profiles for port
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO           *dflt_mirroring_info -
 *     Port default mirroring profiles for tagged and untagged
 *     packets.
 * REMARKS:
 *   - To assign a specific mirror profile for port x vlan
 *   use soc_ppd_llp_trap_port_vlan_mirroring_add().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_dflt_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO           *dflt_mirroring_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_mirror_port_dflt_set" API.
 *     Refer to "soc_ppd_llp_mirror_port_dflt_set" API for details.
*********************************************************************/
uint32
  soc_ppd_llp_mirror_port_dflt_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO           *dflt_mirroring_info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LLP_MIRROR_INCLUDED__*/
#endif

