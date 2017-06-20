/* $Id: arad_pp_ptp.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_PTP_INCLUDED__
/* { */
#define __ARAD_PP_PTP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_ptp.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_PTP_UDP_ENCAP_PORT1 319
#define ARAD_PP_PTP_UDP_ENCAP_PORT2 320
#define ARAD_PP_BFD_SINGLE_HOP_UDP_PORT 3784
#define ARAD_PP_BFD_MICRO_UDP_PORT 6784
#define ARAD_PP_GTP_U_UDP_PORT 2152
#define ARAD_PP_GTP_C_UDP_PORT 2123

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
  ARAD_PP_PTP_INIT = ARAD_PP_PROC_DESC_BASE_PTP_FIRST,
  ARAD_PP_PTP_INIT_UNSAFE,
  ARAD_PP_PTP_PORT_SET,
  ARAD_PP_PTP_PORT_SET_UNSAFE,
  ARAD_PP_PTP_PORT_GET,
  ARAD_PP_PTP_PORT_GET_UNSAFE,
  ARAD_PP_PTP_PORT_SET_ACTION_PROFILE,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_PTP_PROCEDURE_DESC_LAST
} ARAD_PP_PTP_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_PTP_ACTION_PROFILE_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_PTP_FIRST,

  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_PTP_ERR_LAST
} ARAD_PP_PTP_ERR;

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
 *   arad_pp_ptp_init_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure ARAD device to support PTP (1588) protocol.
 *   initialize 1588 general (not per port) registers & tables .
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ptp_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_ptp_set_unsafe
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
  arad_pp_ptp_port_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_IN  SOC_PPC_PTP_PORT_INFO                  *info,
    SOC_SAND_IN  SOC_PPC_PTP_IN_PP_PORT_PROFILE         profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_ptp_get_unsafe
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
  arad_pp_ptp_port_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_OUT SOC_PPC_PTP_IN_PP_PORT_PROFILE         *profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_ptp_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_ptp, arad_pp_ptp modules.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
 arad_pp_ptp_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_ptp_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_ptp module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
arad_pp_ptp_get_errs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_ptp_port_p2p_delay_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Update path delay in p2p delay mchanism.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                  local_port_ndx -
 *     pp port 
 *   SOC_SAND_IN  int                           value -
 *     path delay value
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_ptp_p2p_delay_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_PORT                   local_port_ndx,
    SOC_SAND_IN  int                            value
  );

/*********************************************************************
* NAME:
 *   arad_pp_ptp_port_p2p_delay_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Read path delay in p2p delay mchanism.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                  local_port_ndx -
 *     pp port 
 *   SOC_SAND_OUT  int*                         value -
 *     path delay value
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_ptp_p2p_delay_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_PORT                   local_port_ndx,
    SOC_SAND_OUT int*                           value
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LAG_INCLUDED__*/
#endif


