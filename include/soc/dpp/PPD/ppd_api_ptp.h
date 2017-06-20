/* $Id: ppd_api_ptp.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_PPD_API_PTP_INCLUDED__
/* { */
#define __SOC_PPD_API_PTP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPC/ppc_api_ptp.h>


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
  /*
  * } Auto generated. Do not edit previous section.
  */

  SOC_PPD_PTP_INIT = SOC_PPD_PROC_DESC_BASE_PTP_FIRST,
  SOC_PPD_PTP_PORT_SET,
  SOC_PPD_PTP_PORT_GET,
  /*
  * Last element. Do no touch.
  */
  SOC_PPD_PTP_PROCEDURE_DESC_LAST

} SOC_PPD_PTP_PROCEDURE_DESC;

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
 *   soc_ppd_ptp_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to initialize all the general 1588 
 *   registers/tables need for 1588 configuration.
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_ptp_init(
    SOC_SAND_IN  int                   unit
  );

/*********************************************************************
* NAME:
 *   soc_ppd_ptp_port_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   disable/enable 1588 per port,
 *   initialize 1588 general per port registers & tables .
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                local_port_ndx -
 *     pp port 
 *   SOC_SAND_IN  SOC_PPC_PTP_PORT_INFO                 *info -
 *     PTP (1588) port info
 *   SOC_SAND_IN  SOC_PPC_PTP_IN_PP_PORT_PROFILE        profile -
 *     the PTP (1588) profile index to set to port to.
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_ptp_port_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_IN  SOC_PPC_PTP_PORT_INFO                  *info,
    SOC_SAND_IN  SOC_PPC_PTP_IN_PP_PORT_PROFILE        profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_ptp_port_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   get 1588 status (enabled/disabled) per port
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                local_port_ndx -
 *     pp port 
 *   SOC_SAND_OUT SOC_PPC_PTP_PORT_INFO                 *profile -
 *     the PTP (1588) profile index of the port.
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_ptp_port_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_OUT SOC_PPC_PTP_IN_PP_PORT_PROFILE         *profile
  );


#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_PPD_INCLUDED__*/
#endif
