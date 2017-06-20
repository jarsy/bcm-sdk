/* $Id: arad_pp_llp_parse.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_LLP_PARSE_INCLUDED__
/* { */
#define __ARAD_PP_LLP_PARSE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_parser.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_llp_parse.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Vlan compression defined indexes */
#define ARAD_PP_LLP_PARSE_VLAN_COMPRESSION_OUTER_COMP_NDX        (0)
#define ARAD_PP_LLP_PARSE_VLAN_COMPRESSION_INNER_COMP_NDX        (1)



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
  SOC_PPC_LLP_PARSE_TPID_VALUES_SET = ARAD_PP_PROC_DESC_BASE_LLP_PARSE_FIRST,
  SOC_PPC_LLP_PARSE_TPID_VALUES_SET_PRINT,
  SOC_PPC_LLP_PARSE_TPID_VALUES_SET_UNSAFE,
  SOC_PPC_LLP_PARSE_TPID_VALUES_SET_VERIFY,
  SOC_PPC_LLP_PARSE_TPID_VALUES_GET,
  SOC_PPC_LLP_PARSE_TPID_VALUES_GET_PRINT,
  SOC_PPC_LLP_PARSE_TPID_VALUES_GET_VERIFY,
  SOC_PPC_LLP_PARSE_TPID_VALUES_GET_UNSAFE,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_SET,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_SET_PRINT,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_SET_UNSAFE,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_SET_VERIFY,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_GET,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_GET_PRINT,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_GET_VERIFY,
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_GET_UNSAFE,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_SET,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_SET_PRINT,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_SET_UNSAFE,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_SET_VERIFY,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_GET,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_GET_PRINT,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_GET_VERIFY,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_TO_TPID_PROFILE_MAP_GET_UNSAFE,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_SET,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_SET_PRINT,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_SET_UNSAFE,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_SET_VERIFY,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_GET,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_GET_PRINT,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_GET_VERIFY,
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_GET_UNSAFE,
  ARAD_PP_LLP_PARSE_GET_PROCS_PTR,
  ARAD_PP_LLP_PARSE_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_PARSE_PROCEDURE_DESC_LAST
} ARAD_PP_LLP_PARSE_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LLP_PARSE_TPID_PROFILE_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LLP_PARSE_FIRST,
  ARAD_PP_LLP_PARSE_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_TPID_PROFILE_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_OUTER_TPID_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_INNER_TPID_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_TAG_FORMAT_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_PRIORITY_TAG_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_DLFT_EDIT_COMMAND_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_DFLT_EDIT_PCP_PROFILE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_LLP_PARSE_TYPE_ILLEGAL_ERR,
  ARAD_PP_LLP_PARSE_TRAP_CODE_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_PRIO_TAG_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LLP_PARSE_PRIORITY_TYPE_ILLEGAL_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LLP_PARSE_ERR_LAST
} ARAD_PP_LLP_PARSE_ERR;

typedef enum
{
  /*
  *  no TPID was found
  */
  ARAD_PP_LLP_PARSE_TAG_FORMAT_NONE = 0,
  /*
  *  TPID1 was found
  */
  ARAD_PP_LLP_PARSE_TAG_FORMAT_TPID1 = 1,
  /*
  *  TPID2 was found
  */
  ARAD_PP_LLP_PARSE_TAG_FORMAT_TPID2 = 2,
  /*
  *  TPID3 was found (for ITPID)
  */
  ARAD_PP_LLP_PARSE_TAG_FORMAT_TPID3 = 3,
  /*
  *  Number of types in ARAD_PP_LLP_PARSE_TAG_FORMAT
  */
  ARAD_PP_NOF_LLP_PARSE_TAG_FORMATS = 4
}ARAD_PP_LLP_PARSE_TAG_FORMAT;
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
  arad_pp_llp_parse_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_parse_tpid_values_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the global information for link layer parsing,
 *   including TPID values. Used in ingress to identify VLAN
 *   tags on incoming packets, and used in egress to
 *   construct VLAN tags on outgoing packets.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES                   *tpid_vals -
 *     The global information for link-layer parsing.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_parse_tpid_values_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES                   *tpid_vals
  );

uint32
  arad_pp_llp_parse_tpid_values_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES                   *tpid_vals
  );

uint32
  arad_pp_llp_parse_tpid_values_get_verify(
    SOC_SAND_IN  int                                     unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_parse_tpid_values_set_unsafe" API.
 *     Refer to "arad_pp_llp_parse_tpid_values_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_llp_parse_tpid_values_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_VALUES                   *tpid_vals
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_parse_tpid_profile_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the TPID profile selection of two TPIDs from the
 *   Global TPIDs.
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      tpid_profile_ndx -
 *     TPID Profile ID. Range: AradB: 0 - 3. T20E: 0 - 7.
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO         *tpid_profile_info -
 *     TPID profile information.
 * REMARKS:
 *   - set TPID profile (combination of TPID values), to be
 *   used in ingress/egress parsing and editing.- in Arad-B
 *   Mapping a tpid-profile to a local-port is: using
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
  arad_pp_llp_parse_tpid_profile_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tpid_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO         *tpid_profile_info
  );

uint32
  arad_pp_llp_parse_tpid_profile_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tpid_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO         *tpid_profile_info
  );

uint32
  arad_pp_llp_parse_tpid_profile_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tpid_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_parse_tpid_profile_info_set_unsafe" API.
 *     Refer to "arad_pp_llp_parse_tpid_profile_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_llp_parse_tpid_profile_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  tpid_profile_ndx,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO         *tpid_profile_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_parse_port_profile_to_tpid_profile_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Maps from Port profile to TPID Profile.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  port_profile_ndx -
 *     Port Profile ID. Range 0 - 7. set by
 *     soc_ppd_port_info_set().
 *   SOC_SAND_IN  uint32                                  tpid_profile_id -
 *     TPID Profile ID. Used for packet parsing/editing. Range:
 *     AradB: 0 - 3. T20E: 0 - 7.
 * REMARKS:
 *   - T20E only. In Arad use
 *   soc_ppd_port_info_set(local_port_ndx, tpid_profile)- TPID
 *   profile is used to select TPID values.- Assign a
 *   port-profile to the local-port using: 1.
 *   soc_ppd_port_info_set(local_port_ndx, port_profile)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_parse_port_profile_to_tpid_profile_map_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint32                                  tpid_profile_id
  );

uint32
  arad_pp_llp_parse_port_profile_to_tpid_profile_map_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  uint32                                  tpid_profile_id
  );

uint32
  arad_pp_llp_parse_port_profile_to_tpid_profile_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_parse_port_profile_to_tpid_profile_map_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_llp_parse_port_profile_to_tpid_profile_map_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_llp_parse_port_profile_to_tpid_profile_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_OUT uint32                                  *tpid_profile_id
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_parse_packet_format_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given the port profile and the parsing information
 *   determine: - Whether this packet format accepted or
 *   denied. - The tag structure of the packet, i.e. what
 *   vlan tags exist on the packet (S-tag, S-C-tag, etc...).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  port_profile_ndx -
 *     Port Profile ID. Range 0 - 7. Set by
 *     soc_ppd_port_info_set().
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key -
 *     Parsing information specifying what TPIDs exist on the
 *     packet
 *   SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info -
 *     Packet format information, including whether this format
 *     is acceptable or not, and what vlan tag structure to
 *     assign to this packet
 * REMARKS:
 *   - Arad-B only.- Assign a port-profile to the local-port
 *   using: 1. soc_ppd_port_info_set(local_port_ndx,
 *   port_profile)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_llp_parse_packet_format_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  );
soc_error_t
  arad_pp_llp_parse_packet_format_eg_info_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  );
soc_error_t
  arad_pp_llp_parse_packet_format_eg_info_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  );

uint32
  arad_pp_llp_parse_packet_format_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  );

uint32
  arad_pp_llp_parse_packet_format_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_llp_parse_packet_format_info_set_unsafe" API.
 *     Refer to "arad_pp_llp_parse_packet_format_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_llp_parse_packet_format_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO                      *parse_key,
    SOC_SAND_OUT SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO        *format_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_llp_parse_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_llp_parse module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_llp_parse_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_llp_parse_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_llp_parse module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_llp_parse_get_errs_ptr(void);

uint32
  SOC_PPC_LLP_PARSE_TPID_VALUES_verify(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_VALUES *info
  );

uint32
  SOC_PPC_LLP_PARSE_TPID_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_INFO *info
  );

uint32
  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_TPID_PROFILE_INFO *info
  );

uint32
  SOC_PPC_LLP_PARSE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_INFO *info
  );

uint32
  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LLP_PARSE_PACKET_FORMAT_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LLP_PARSE_INCLUDED__*/
#endif

