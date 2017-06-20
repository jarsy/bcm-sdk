/* $Id: ppd_api_port.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_port.h
*
* MODULE PREFIX:  soc_ppd_port
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

#ifndef __SOC_PPD_API_PORT_INCLUDED__
/* { */
#define __SOC_PPD_API_PORT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_port.h>

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
  SOC_PPD_PORT_INFO_SET = SOC_PPD_PROC_DESC_BASE_PORT_FIRST,
  SOC_PPD_PORT_INFO_SET_PRINT,
  SOC_PPD_PORT_INFO_GET,
  SOC_PPD_PORT_INFO_GET_PRINT,
  SOC_PPD_PORT_STP_STATE_SET,
  SOC_PPD_PORT_STP_STATE_SET_PRINT,
  SOC_PPD_PORT_STP_STATE_GET,
  SOC_PPD_PORT_STP_STATE_GET_PRINT,
  SOC_PPD_PORT_LOCAL_PORT_TO_SYS_PHY_MAP_SET,
  SOC_PPD_PORT_LOCAL_PORT_TO_SYS_PHY_MAP_SET_PRINT,
  SOC_PPD_PORT_LOCAL_PORT_TO_SYS_PHY_MAP_GET,
  SOC_PPD_PORT_LOCAL_PORT_TO_SYS_PHY_MAP_GET_PRINT,
  SOC_PPD_PORT_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_PORT_PROCEDURE_DESC_LAST
} SOC_PPD_PORT_PROCEDURE_DESC;

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
 *   soc_ppd_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set main attributes of the port. Generally, these
 *   attributes identify the port and may have use in more
 *   than one module.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_PORT_INFO                           *port_info -
 *     Port attributes, including main attributes of the port.
 * REMARKS:
 *   - In order to configure the port default AC use
 *   soc_ppd_l2_lif_ac_add() while ignoring the VIDs i.e. set
 *   VIDs to SOC_PPC_LIF_IGNORE_INNER_VID and
 *   SOC_PPC_LIF_IGNORE_OUTER_VID.- Note that not all port
 *   attributes are included in this configuration. Some
 *   attributes that are used by a single module are
 *   configured by APIs in that module. Typically the API
 *   name is soc_ppd_<module_name>_port_info_set.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_port_info_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_PORT_INFO                           *port_info
  );

/*********************************************************************
*     Gets the configuration set by the "soc_ppd_port_info_set"
 *     API.
 *     Refer to "soc_ppd_port_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_port_info_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_PORT_INFO                           *port_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_port_stp_state_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the STP state of a port in a specific topology ID.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  uint32                                topology_id_ndx -
 *     Topology ID. Set using soc_ppd_vsi_info_set(). Range: 0 -
 *     63.
 *   SOC_SAND_IN  SOC_PPC_PORT_STP_STATE                      stp_state -
 *     STP state of the port (discard/learn/forward)
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_port_stp_state_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  uint32                                topology_id_ndx,
    SOC_SAND_IN  SOC_PPC_PORT_STP_STATE                      stp_state
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_port_stp_state_set" API.
 *     Refer to "soc_ppd_port_stp_state_set" API for details.
*********************************************************************/
uint32
  soc_ppd_port_stp_state_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  uint32                                topology_id_ndx,
    SOC_SAND_OUT SOC_PPC_PORT_STP_STATE                      *stp_state
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_PORT_INCLUDED__*/
#endif

