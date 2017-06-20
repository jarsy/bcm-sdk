/* $Id: ppd_api_eg_mirror.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_eg_mirror.h
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

#ifndef __SOC_PPD_API_EG_MIRROR_INCLUDED__
/* { */
#define __SOC_PPD_API_EG_MIRROR_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_eg_mirror.h>

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
  SOC_PPD_EG_MIRROR_PORT_VLAN_ADD = SOC_PPD_PROC_DESC_BASE_EG_MIRROR_FIRST,
  SOC_PPD_EG_MIRROR_PORT_VLAN_ADD_PRINT,
  SOC_PPD_EG_MIRROR_PORT_VLAN_REMOVE,
  SOC_PPD_EG_MIRROR_PORT_VLAN_REMOVE_PRINT,
  SOC_PPD_EG_MIRROR_PORT_VLAN_GET,
  SOC_PPD_EG_MIRROR_PORT_VLAN_GET_PRINT,
  SOC_PPD_EG_MIRROR_PORT_DFLT_SET,
  SOC_PPD_EG_MIRROR_PORT_DFLT_SET_PRINT,
  SOC_PPD_EG_MIRROR_PORT_DFLT_GET,
  SOC_PPD_EG_MIRROR_RECYCLE_COMMAND_TRAP_SET,
  SOC_PPD_EG_MIRROR_RECYCLE_COMMAND_TRAP_GET,  
  SOC_PPD_EG_MIRROR_PORT_DFLT_GET_PRINT,
  SOC_PPD_EG_MIRROR_PORT_INFO_GET,  
  SOC_PPD_EG_MIRROR_PORT_INFO_GET_PRINT,  
  SOC_PPD_EG_MIRROR_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_EG_MIRROR_PROCEDURE_DESC_LAST
} SOC_PPD_EG_MIRROR_PROCEDURE_DESC;

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
 *   soc_ppd_eg_mirror_port_vlan_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set outbound mirroring for out-port and VLAN, so all
 *   outgoing packets leave from the given port and with the
 *   given VID will be mirrored or not according to
 *   'enable_mirror'
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_IN  uint8                               enable_mirror -
 *     Soc_petra: TRUE packets will be mirrored. FALSE packet will not be mirrored.
 *     Arad: mirror profile number, 0 means no mirror.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds (upon add). Add operation
 *     may fail if there are no sufficient resources.
 *     Successful add is guaranteed for up to 7 different VIDs.
 * REMARKS:
 *   - Soc_petra-B only. Error is returned if called for T20E.-
 *   The VID considered is the outer VID set by the egress
 *   vlan editing i.e. the outer-VID the packet would has if
 *   the packet transmitted tagged according to
 *   soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set().-
 *   packet is mirrored only if - out-port x vlan was set
 *   enable mirror OR out-port default to enable mirroring -
 *   AND out-TM-port enables mirroring- The mirrored packet
 *   is mirrored to channel set according to out-TM-port. To
 *   see TM-port configuration see Soc_petra UM document
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_vlan_add(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_IN dpp_outbound_mirror_config_t        *config,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_mirror_port_vlan_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a mirroring for port and VLAN, upon this packet
 *   transmitted out this out_port_ndx and vid_ndx will be
 *   mirrored or not according to default configuration for
 *   out_port_ndx. see soc_ppd_eg_mirror_port_dflt_set()
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 * REMARKS:
 *   - Soc_petra-B only. Error is returned if called for T20E.-
 *   The VID considered is the outer VID set by the egress
 *   vlan editing i.e. the outer-VID the packet would has if
 *   the packet transmitted tagged according to
 *   soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_vlan_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_mirror_port_vlan_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the assigned mirroring profile for port and VLAN.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx -
 *     VLAN ID. Range: 0 - 4095.
 *   SOC_SAND_OUT uint8                               *enable_mirror -
 *     Whether mirroring is enabled on this port x vlan
 *     Not in Soc_petra, also returns the mirror profile (0 is disabled).
 * REMARKS:
 *   - Soc_petra-B only. Error is returned if called for T20E.-
 *   The VID considered is the outer VID set by the egress
 *   vlan editing i.e. the outer-VID the packet would has if
 *   the packet transmitted tagged according to
 *   soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set().- If
 *   no mirror profile was associated to the Port and VLAN
 *   (by soc_ppd_eg_port_vlan_mirror_add()), the default is
 *   returned according to soc_ppd_eg_mirror_port_dflt_set().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_vlan_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                out_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         vid_ndx,
    SOC_SAND_OUT dpp_outbound_mirror_config_t        *config
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_mirror_port_dflt_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set default mirroring profiles for port
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_EG_MIRROR_PORT_DFLT_INFO            *dflt_mirroring_info -
 *     Port default mirroring profiles for tagged and untagged
 *     packets.
 * REMARKS:
 *   - Soc_petra-B only. Error is returned if called for T20E.-
 *   To assign a specific mirror profile for port x vlan use
 *   soc_ppd_eg_port_vlan_mirror_add().- The VID considered is
 *   the outer VID set by the egress vlan editing i.e. the
 *   outer-VID the packet would has if the packet transmitted
 *   tagged according to
 *   soc_ppd_eg_vlan_edit_port_vlan_transmit_outer_tag_set().-
 *   packet is mirrored only if - out-port x vlan was set
 *   enable mirror OR out-port default to enable mirroring -
 *   AND out-TM-port enables mirroring- The mirrored packet
 *   is mirrored to channel set according to out-TM-port. To
 *   see TM-port configuration see Soc_petra UM document
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_dflt_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                      pp_port,
    SOC_SAND_IN  dpp_outbound_mirror_config_t       *config

  );

/*********************************************************************
* Get the default mirroring profile for port
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_dflt_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT  dpp_outbound_mirror_config_t       *config
  );


uint32
  soc_ppd_eg_mirror_port_appl_set(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  SOC_PPC_PORT  local_port_ndx,
    SOC_SAND_IN  uint8     appl_mirroring_info
  );

/*********************************************************************
* Check if mirroring for a port by other (than mirroring) applications is enabled
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_appl_get(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  SOC_PPC_PORT  local_port_ndx,
    SOC_SAND_OUT uint8     *appl_mirroring_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_mirror_port_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Retreive mirror port information.
 * INPUT:
 *   SOC_SAND_IN  int    unit - Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT  local_port_ndx - Local port ID.
 *   SOC_SAND_IN  SOC_PPC_EG_MIRROR_PORT_INFO info - Port information
 * REMARKS:
 *   not supported in Soc_petra
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_mirror_port_info_get(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  int    core_id,
    SOC_SAND_IN  SOC_PPC_PORT  local_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_MIRROR_PORT_INFO *info
  );


/*********************************************************************
* NAME:
 *   arad_pp_eg_mirror_recycle_command_trap_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting trap code field of recycle command table in given entry.
 *   Used for trapping outbound mirroring packets.
 *   Setting FORWARD_STRENGTH to 7.
 *   Mirrored copy will be disabled.
 * INPUT:
 *   SOC_SAND_IN  int    unit - Identifier of the device to access.
 *   SOC_SAND_IN  uint32    recycle_command - Index in the table.
 *   SOC_SAND_IN  uint32    trap_code - internal trap code
  *  SOC_SAND_IN  uint32        forward_strengh, 
 *   SOC_SAND_IN  uint32    snoop_strength - snoop strength 
 * REMARKS:
 *   not supported in Soc_petra
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_mirror_recycle_command_trap_set(
    SOC_SAND_IN  int        unit,      /* Identifier of the device to access */
    SOC_SAND_IN  uint32        recycle_command, /* Equal to mirror profile */
    SOC_SAND_IN  uint32        trap_code, /* PPD - not HW code */
    SOC_SAND_IN  uint32        snoop_strength,
    SOC_SAND_IN  uint32        forward_strengh 
  );

uint32
  soc_ppd_eg_mirror_recycle_command_trap_get(
    SOC_SAND_IN  int        unit,      /* Identifier of the device to access */
    SOC_SAND_IN  uint32        recycle_command, /* Equal to mirror profile */
    SOC_SAND_OUT  uint32       *trap_code,
    SOC_SAND_OUT  uint32       *snoop_strength
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_EG_MIRROR_INCLUDED__*/
#endif

