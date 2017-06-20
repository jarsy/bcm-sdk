/* $Id: ppd_api_llp_parse.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_llp_parse.h
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

#ifndef __SOC_PPD_API_LLP_PARSE_INCLUDED__
/* { */
#define __SOC_PPD_API_LLP_PARSE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_llp_parse.h>

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
  SOC_PPD_LLP_PARSE_TPID_VALUES_SET = SOC_PPD_PROC_DESC_BASE_LLP_PARSE_FIRST,
  SOC_PPD_LLP_PARSE_TPID_VALUES_SET_PRINT,
  SOC_PPD_LLP_PARSE_TPID_VALUES_GET,
  SOC_PPD_LLP_PARSE_TPID_VALUES_GET_PRINT,
  SOC_PPD_LLP_PARSE_TPID_PROFILE_INFO_SET,
  SOC_PPD_LLP_PARSE_TPID_PROFILE_INFO_SET_PRINT,
  SOC_PPD_LLP_PARSE_TPID_PROFILE_INFO_GET,
  SOC_PPD_LLP_PARSE_TPID_PROFILE_INFO_GET_PRINT,
  SOC_PPD_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_SET,
  SOC_PPD_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_SET_PRINT,
  SOC_PPD_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_GET,
  SOC_PPD_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_GET_PRINT,
  SOC_PPD_LLP_PARSE_PACKET_FORMAT_INFO_SET,
  SOC_PPD_LLP_PARSE_PACKET_FORMAT_INFO_SET_PRINT,
  SOC_PPD_LLP_PARSE_PACKET_FORMAT_INFO_GET,
  SOC_PPD_LLP_PARSE_PACKET_FORMAT_INFO_GET_PRINT,
  SOC_PPD_LLP_PARSE_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LLP_PARSE_PROCEDURE_DESC_LAST
} SOC_PPD_LLP_PARSE_PROCEDURE_DESC;

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
 *   soc_ppd_llp_parse_tpid_values_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the global information for link layer parsing,
 *   including TPID values. Used in ingress to identify VLAN
 *   tags on incoming packets, and used in egress to
 *   construct VLAN tags on outgoing packets.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES               *tpid_vals -
 *     The global information for link-layer parsing.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_values_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES               *tpid_vals
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_parse_tpid_values_set" API.
 *     Refer to "soc_ppd_llp_parse_tpid_values_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_values_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_VALUES               *tpid_vals
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_parse_tpid_profile_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the TPID profile selection of two TPIDs from the
 *   Global TPIDs.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                tpid_profile_ndx -
 *     TPID Profile ID. Range: Soc_petraB: 0 - 3. T20E: 0 - 7.
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO         *tpid_profile_info -
 *     TPID profile information.
 * REMARKS:
 *   - set TPID profile (combination of TPID values), to be
 *   used in ingress/egress parsing and editing.- in Soc_petra-B
 *   Assigning a tpid-profile to a local-port is: using
 *   soc_ppd_port_info_set(local_port_ndx, tpid_profile)- in T20E
 *   Mapping a tpid-profile to a local-port is a two-stage
 *   process: 1. Assign a port-profile to the local-port
 *   using: soc_ppd_port_info_set(local_port_ndx, port_profile)
 *   2. Map assigned port-profile to a tpid-profile using:
 *   soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set(port_profile_ndx,
 *   tpid_profile)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_profile_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                tpid_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO         *tpid_profile_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_parse_tpid_profile_info_set" API.
 *     Refer to "soc_ppd_llp_parse_tpid_profile_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_parse_tpid_profile_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                tpid_profile_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO         *tpid_profile_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Maps from Port profile to TPID Profile.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                port_profile_ndx -
 *     Port Profile ID. Range 0 - 7. set by
 *     soc_ppd_port_info_set().
 *   SOC_SAND_IN  uint32                                tpid_profile_id -
 *     TPID Profile ID. Used for packet parsing/editing. Range:
 *     Soc_petraB: 0 - 3. T20E: 0 - 7.
 * REMARKS:
 *   - T20E only. In Soc_petra use
 *   soc_ppd_port_info_set(local_port_ndx, tpid_profile)- TPID
 *   profile is used to select TPID values.- Assign a
 *   port-profile to the local-port using: 1.
 *   soc_ppd_port_info_set(local_port_ndx, port_profile)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_IN  uint32                                tpid_profile_id
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set"
 *     API.
 *     Refer to
 *     "soc_ppd_llp_parse_port_profile_to_tpid_profile_map_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_llp_parse_port_profile_to_tpid_profile_map_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_OUT uint32                                *tpid_profile_id
  );

/*********************************************************************
* NAME:
 *   soc_ppd_llp_parse_packet_format_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given the port profile and the parsing information
 *   determine: - Whether this packet format accepted or
 *   denied. - The tag structure of the packet, i.e. what
 *   vlan tags exist on the packet (S-tag, S-C-tag, etc...).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access. Range 0- 3.
 *   SOC_SAND_IN  uint32                          eg_acceptable_frames_port_profile -
 *     Egress Acceptable frames profile per port.
 *   SOC_SAND_IN  uint32                          llvp_port_profile -
 *     LLVP profile per port. Range 0-7.
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key -
 *     Parsing information specifying what TPIDs exist on the
 *     packet
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info -
 *     Packet format information, including whether this format
 *     is acceptable or not, and what vlan tag structure to
 *     assign to this packet
 * REMARKS:
 *   - Soc_petra-B only.- Assign a port-profile to the local-port
 *   using: 1. soc_ppd_port_info_set(local_port_ndx,
 *   port_profile)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_llp_parse_packet_format_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_llp_parse_packet_format_info_set" API.
 *     Refer to "soc_ppd_llp_parse_packet_format_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_llp_parse_packet_format_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LLP_PARSE_INCLUDED__*/
#endif

