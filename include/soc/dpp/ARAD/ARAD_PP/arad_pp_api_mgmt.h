
/* $Id: arad_pp_api_mgmt.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_API_MGMT_INCLUDED__
/* { */
#define __ARAD_PP_API_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mgmt.h>


#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

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
*     arad_pp_mgmt_operation_mode_set
* TYPE:
*   PROC
* FUNCTION:
*     Set arad_pp device operation mode. This defines
*     configurations such as support for certain header types
*     etc.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *op_mode -
*     arad_pp device operation mode.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_operation_mode_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_MGMT_OPERATION_MODE *op_mode
  );

/*********************************************************************
* NAME:
*     arad_pp_mgmt_operation_mode_get
* TYPE:
*   PROC
* FUNCTION:
*     Set arad_pp device operation mode. This defines
*     configurations such as support for certain header types
*     etc.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_PP_MGMT_OPERATION_MODE *op_mode -
*     arad_pp device operation mode.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_operation_mode_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_PP_MGMT_OPERATION_MODE *op_mode
  );

/*********************************************************************
* NAME:
*     arad_pp_mgmt_device_close
* TYPE:
*   PROC
* FUNCTION:
*     Close the Device, and clean HW and SW.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to close.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_device_close(
    SOC_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_mgmt_elk_mode_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ELK interface mode.
 * INPUT:
 *   SOC_SAND_IN  int           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_MGMT_ELK_MODE elk_mode -
 *     The requested operation mode for the ELK interface.
 *   SOC_SAND_OUT uint32           *ingress_pkt_rate -
 *     The effective processing rate of the ingress device in
 *     packets per second.
 * REMARKS:
 *   1. The ELK's physical interface has to be configured
 *   before calling this function. See arad_nif_elk_set().2.
 *   Arad-B rev. B0 modes are not available on earlier
 *   revisions.3. The ingress device's rate is limited by the
 *   processing rate of the ELK interface, affecting all
 *   packets entering the device.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_elk_mode_set(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_ELK_MODE elk_mode,
    SOC_SAND_OUT uint32           *ingress_pkt_rate
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mgmt_elk_mode_set" API.
 *     Refer to "arad_pp_mgmt_elk_mode_set" API for details.
*********************************************************************/
uint32
  arad_pp_mgmt_elk_mode_get(
    SOC_SAND_IN  int           unit,
    SOC_SAND_OUT ARAD_PP_MGMT_ELK_MODE *elk_mode,
    SOC_SAND_OUT uint32           *ingress_pkt_rate
  );

/*********************************************************************
* NAME:
 *   arad_pp_mgmt_use_elk_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Determine whether the specified lookup is externalized
 *   or not.
 * INPUT:
 *   SOC_SAND_IN  int           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_MGMT_LKP_TYPE lkp_type -
 *     The lookup type in question.
 *   SOC_SAND_IN  uint8           use_elk -
 *     Whether to use the ELK for that lookup or not.
 * REMARKS:
 *   IPv4 multicast and IPv6 lookups can only be performed
 *   externally in Arad-B rev. B0 modes.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_use_elk_set(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  ARAD_PP_MGMT_LKP_TYPE lkp_type,
    SOC_SAND_IN  uint8           use_elk
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mgmt_use_elk_set" API.
 *     Refer to "arad_pp_mgmt_use_elk_set" API for details.
*********************************************************************/
uint32
  arad_pp_mgmt_use_elk_get(
    SOC_SAND_IN  int           unit,
    SOC_SAND_OUT ARAD_PP_MGMT_LKP_TYPE *lkp_type,
    SOC_SAND_OUT uint8           *use_elk
  );

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_API_MGMT_INCLUDED__*/
#endif


