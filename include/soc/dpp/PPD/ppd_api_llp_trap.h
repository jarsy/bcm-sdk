/* $Id: ppd_api_llp_trap.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_llp_trap.h
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

#ifndef __SOC_PPD_API_LLP_TRAP_INCLUDED__
/* { */
#define __SOC_PPD_API_LLP_TRAP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_llp_trap.h>

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
  SOC_PPD_LLP_TRAP_PORT_INFO_SET = SOC_PPD_PROC_DESC_BASE_LLP_TRAP_FIRST,
  SOC_PPD_LLP_TRAP_PORT_INFO_SET_PRINT,
  SOC_PPD_LLP_TRAP_PORT_INFO_GET,
  SOC_PPD_LLP_TRAP_PORT_INFO_GET_PRINT,
  SOC_PPD_LLP_TRAP_ARP_INFO_SET,
  SOC_PPD_LLP_TRAP_ARP_INFO_SET_PRINT,
  SOC_PPD_LLP_TRAP_ARP_INFO_GET,
  SOC_PPD_LLP_TRAP_ARP_INFO_GET_PRINT,
  SOC_PPD_LLP_TRAP_RESERVED_MC_INFO_SET,
  SOC_PPD_LLP_TRAP_RESERVED_MC_INFO_SET_PRINT,
  SOC_PPD_LLP_TRAP_RESERVED_MC_INFO_GET,
  SOC_PPD_LLP_TRAP_RESERVED_MC_INFO_GET_PRINT,
  SOC_PPD_LLP_TRAP_PROG_TRAP_INFO_SET,
  SOC_PPD_LLP_TRAP_PROG_TRAP_INFO_SET_PRINT,
  SOC_PPD_LLP_TRAP_PROG_TRAP_INFO_GET,
  SOC_PPD_LLP_TRAP_PROG_TRAP_INFO_GET_PRINT,
  SOC_PPD_LLP_TRAP_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LLP_TRAP_PROCEDURE_DESC_LAST
} SOC_PPD_LLP_TRAP_PROCEDURE_DESC;

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
 *   soc_ppd_llp_trap_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets port information for Link Layer Traps, including
 *   which reserved Multicast profile and which Traps are
 *   enabled...
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_LLP_TRAP_PORT_INFO                  *port_info -
 *     Port information for Link-layer trapping.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_trap_port_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PORT_INFO                  *port_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_trap_port_info_set" API.
 *     Refer to "soc_ppd_llp_trap_port_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_llp_trap_port_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PORT_INFO                  *port_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_trap_arp_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets information for ARP trapping, including My-IP
 *   addresses (used to Trap ARP Requests)
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_TRAP_ARP_INFO                   *arp_info -
 *     ARP trap information
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_trap_arp_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_ARP_INFO                   *arp_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_trap_arp_info_set" API.
 *     Refer to "soc_ppd_llp_trap_arp_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_llp_trap_arp_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_ARP_INFO                   *arp_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_trap_reserved_mc_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets Trap information for IEEE reserved multicast
 *   (Ethernet Header. DA matches 01-80-c2-00-00-XX where XX =
 *   8'b00xx_xxxx.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY            *reserved_mc_key -
 *     Reserved Multicast key including Destination MAC address
 *     lsb (the msb are constant) and profile.
 *   SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *trap_action -
 *     Trap information including snoop/forwarding action.
 *     trap_code range: SOC_PPC_TRAP_CODE_RESERVED_MC_0-SOC_PPC_TRAP_CODE_RESERVED_MC_7.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_trap_reserved_mc_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY            *reserved_mc_key,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *trap_action
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_trap_reserved_mc_info_set" API.
 *     Refer to "soc_ppd_llp_trap_reserved_mc_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_trap_reserved_mc_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_RESERVED_MC_KEY            *reserved_mc_key,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE                      *trap_action
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_trap_prog_trap_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets a programmable trap, a trap that may be set to
 *   packets according to L2/L3/L4 attributes.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                prog_trap_ndx -
 *     Trap ID. Range: 0 - 3.
 *   SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER        *prog_trap_qual -
 *     L2/L3/L4 attributes of the packets including (packets
 *     MAC-DA, Ether-Type, IP protocol, L4 ports).
 * REMARKS:
 *   - User can specify per port which programmable traps are
 *   enabled on the port - See soc_ppd_llp_trap_port_info_set().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_trap_prog_trap_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                prog_trap_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER        *prog_trap_qual
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_trap_prog_trap_info_set" API.
 *     Refer to "soc_ppd_llp_trap_prog_trap_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_trap_prog_trap_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                prog_trap_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_TRAP_PROG_TRAP_QUALIFIER        *prog_trap_qual
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LLP_TRAP_INCLUDED__*/
#endif

