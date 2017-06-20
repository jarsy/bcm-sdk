/* $Id: arad_pp_llp_mirror.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_LLP_MIRROR_INCLUDED__
/* { */
#define __ARAD_PP_LLP_MIRROR_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_llp_mirror.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_LLP_MIRROR_NOF_VID_MIRROR_INDICES (6)

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
  ARAD_PP_LLP_MIRROR_PORT_VLAN_ADD = ARAD_PP_PROC_DESC_BASE_LLP_MIRROR_FIRST,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_ADD_PRINT,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_ADD_UNSAFE,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_ADD_VERIFY,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_REMOVE,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_REMOVE_PRINT,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_REMOVE_UNSAFE,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_REMOVE_VERIFY,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_GET,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_GET_PRINT,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_GET_UNSAFE,
  ARAD_PP_LLP_MIRROR_PORT_VLAN_GET_VERIFY,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_SET,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_SET_PRINT,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_SET_UNSAFE,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_SET_VERIFY,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_GET,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_GET_PRINT,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_GET_VERIFY,
  ARAD_PP_LLP_MIRROR_PORT_DFLT_GET_UNSAFE,
  ARAD_PP_LLP_MIRROR_GET_PROCS_PTR,
  ARAD_PP_LLP_MIRROR_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_MIRROR_PROCEDURE_DESC_LAST
} ARAD_PP_LLP_MIRROR_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LLP_MIRROR_MIRROR_PROFILE_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LLP_MIRROR_FIRST,
  ARAD_PP_LLP_MIRROR_SUCCESS_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_MIRROR_TAGGED_DFLT_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_MIRROR_UNTAGGED_DFLT_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_VID_NDX_OUT_OF_RANGE_NO_ZERO_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_MIRROR_ERR_LAST
} ARAD_PP_LLP_MIRROR_ERR;

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
  arad_pp_llp_mirror_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_mirror_port_vlan_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set a mirroring for port and VLAN, so all incoming
 *   packets enter from the given port and identified with
 *   the given VID will be associated with Mirror command
 *   (Enables mirroring (copying) the packets to additional
 *   destination.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_IN  uint32                                  mirror_profile -
 *     Mirroring profile to associate with packets, the
 *     resolution of this mirroring occurs in Arad-TM device.
 *     Range 0-15.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
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
 *   assigned more than one mirror profile then: - in Arad:
 *   last assignment is considered. - in T20E: the
 *   snoop/mirror command with highest strength is
 *   considered.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_mirror_port_vlan_add_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  uint32                                  mirror_profile,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_llp_mirror_port_vlan_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_IN  uint32                                  mirror_profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_mirror_port_vlan_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a mirroring for port and VLAN
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_mirror_port_vlan_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx
  );

uint32
  arad_pp_llp_mirror_port_vlan_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_mirror_port_vlan_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the assigned mirroring profile for port and VLAN.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_OUT uint32                                  *mirror_profile -
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
  arad_pp_llp_mirror_port_vlan_get_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx,
    SOC_SAND_OUT uint32                                  *mirror_profile
  );

uint32
  arad_pp_llp_mirror_port_vlan_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           vid_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_mirror_port_dflt_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set default mirroring profiles for port
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
  arad_pp_llp_mirror_port_dflt_set_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO           *dflt_mirroring_info
  );

uint32
  arad_pp_llp_mirror_port_dflt_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO           *dflt_mirroring_info
  );

uint32
  arad_pp_llp_mirror_port_dflt_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_mirror_port_dflt_set_unsafe" API.
 *     Refer to "arad_pp_llp_mirror_port_dflt_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_llp_mirror_port_dflt_get_unsafe(
    SOC_SAND_IN  int                                 unit,
	SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO           *dflt_mirroring_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_mirror_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_llp_mirror module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_llp_mirror_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_llp_mirror_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_llp_mirror module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_llp_mirror_get_errs_ptr(void);

uint32
  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LLP_MIRROR_INCLUDED__*/
#endif

