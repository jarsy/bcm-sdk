/* $Id: ppd_api_trap_mgmt.h,v 1.24 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_trap_mgmt.h
*
* MODULE PREFIX:  soc_ppd_api_trap_mgmt
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

#ifndef __SOC_PPD_API_TRAP_MGMT_INCLUDED__
/* { */
#define __SOC_PPD_API_TRAP_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/pkt.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_trap_mgmt.h>


#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_frwrd_mact.h>

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
  SOC_PPD_TRAP_FRWRD_PROFILE_INFO_SET = SOC_PPD_PROC_DESC_BASE_TRAP_MGMT_FIRST,
  SOC_PPD_TRAP_FRWRD_PROFILE_INFO_SET_PRINT,
  SOC_PPD_TRAP_FRWRD_PROFILE_INFO_GET,
  SOC_PPD_TRAP_FRWRD_PROFILE_INFO_GET_PRINT,
  SOC_PPD_TRAP_SNOOP_PROFILE_INFO_SET,
  SOC_PPD_TRAP_SNOOP_PROFILE_INFO_SET_PRINT,
  SOC_PPD_TRAP_SNOOP_PROFILE_INFO_GET,
  SOC_PPD_TRAP_SNOOP_PROFILE_INFO_GET_PRINT,
  SOC_PPD_TRAP_TO_EG_ACTION_MAP_SET,
  SOC_PPD_TRAP_TO_EG_ACTION_MAP_SET_PRINT,
  SOC_PPD_TRAP_TO_EG_ACTION_MAP_GET,
  SOC_PPD_TRAP_TO_EG_ACTION_MAP_GET_PRINT,
  SOC_PPD_TRAP_EG_PROFILE_INFO_SET,
  SOC_PPD_TRAP_EG_PROFILE_INFO_SET_PRINT,
  SOC_PPD_TRAP_EG_PROFILE_INFO_GET,
  SOC_PPD_TRAP_EG_PROFILE_INFO_GET_PRINT,
  SOC_PPD_TRAP_MACT_EVENT_GET,
  SOC_PPD_TRAP_MACT_EVENT_GET_PRINT,
  SOC_PPD_TRAP_MACT_EVENT_PARSE,
  SOC_PPD_TRAP_MACT_EVENT_PARSE_PRINT,
  SOC_PPD_TRAP_PACKET_PARSE,
  SOC_PPD_TRAP_PACKET_PARSE_PRINT,
  SOC_PPD_TRAP_MGMT_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_TRAP_MGMT_PROCEDURE_DESC_LAST
} SOC_PPD_TRAP_MGMT_PROCEDURE_DESC;

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
 *   soc_ppd_trap_frwrd_profile_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set forwarding action profile information.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRAP_CODE                           trap_code_ndx -
 *     Trap code. Soc_petraB range: 0-255. T20E range: 0-255, only
 *     enumarators mentioned as supported in T20e.user can use
 *     values not in the enum for used defined values.
 *   SOC_SAND_IN  SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO      *profile_info -
 *     Information to set to the forwarding profile.
 *   SOC_SAND_IN int                                    core_id-
 *      For which core the info being set, in case of SOC_PPC_CORE_ANY same info will be set for all cores.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_trap_frwrd_profile_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_TRAP_CODE                           trap_code_ndx,
    SOC_SAND_IN  SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO      *profile_info,
    SOC_SAND_IN  int                                    core_id
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_trap_frwrd_profile_info_set" API.
 *     Refer to "soc_ppd_trap_frwrd_profile_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_trap_frwrd_profile_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_TRAP_CODE                           trap_code_ndx,
    SOC_SAND_OUT SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO      *profile_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_trap_snoop_profile_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set snoop action profile information.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRAP_CODE                           trap_code_ndx -
 *     Trap code. Soc_petraB range: 0-255. T20E range: 0-255, only
 *     enumarators mentioned as supported in T20E.user can use
 *     values not in the enum for used defined values.
 *   SOC_SAND_IN  SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO      *profile_info -
 *     Information to set to the snoop profile.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_trap_snoop_profile_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_TRAP_CODE                           trap_code_ndx,
    SOC_SAND_IN  SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO      *profile_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_trap_snoop_profile_info_set" API.
 *     Refer to "soc_ppd_trap_snoop_profile_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_trap_snoop_profile_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_TRAP_CODE                           trap_code_ndx,
    SOC_SAND_OUT SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO      *profile_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_trap_to_eg_action_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Maps egress trap type to egress action profile.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                      trap_type_bitmap_ndx -
 *     Trap Type bitmap, the cause for trapping/filtering the packets
 *     (for example MTU). Use SOC_PB_PP_TRAP_EG_TYPE for bit offsets.
 *   SOC_SAND_IN  uint32                                eg_action_profile -
 *     Egress action profile, to process/forward the packet
 *     according. To set the action pointed by this parameter
 *     use soc_ppd_trap_eg_profile_info_set(). Use
 *     SOC_PPC_TRAP_EG_NO_ACTION in order to bypass this trapping
 *     and then the packet will be processed/forwarded normal
 *     (as no trap was identified).
 *     Soc_petra-B: Range: 1 - 7.
 * REMARKS:
 *   - Soc_petra-B only. Error is returned if called for T20E.-
 *   In T20E: use soc_ppd_trap_snoop_profile_info_set() and
 *   soc_ppd_trap_frwrd_profile_info_set() to set action for Trap
 *   occurs in the T20E egress.- For part of the
 *   filters/traps (see SOC_PPC_EG_FILTER_PORT_ENABLE) user can
 *   set whether to perform the filter/trap per port. See
 *   soc_ppd_eg_filter_port_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_trap_to_eg_action_map_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                trap_type_bitmap_ndx,
    SOC_SAND_IN  uint32                                eg_action_profile
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_trap_to_eg_action_map_set" API.
 *     Refer to "soc_ppd_trap_to_eg_action_map_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_trap_to_eg_action_map_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                trap_type_bitmap_ndx,
    SOC_SAND_OUT uint32                                *eg_action_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_trap_eg_profile_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set egress action profile information.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                profile_ndx -
 *     Egress action profile. Range: 1 - 7.
 *   SOC_SAND_IN  SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO         *profile_info -
 *     Information to set to the egress profile.
 * REMARKS:
 *   - Soc_petra-B only. Error is returned if called for T20E.-
 *   In T20E: use soc_ppd_trap_snoop_profile_info_set() and
 *   soc_ppd_trap_frwrd_profile_info_set() to set action for Trap
 *   occurs in the T20E egress.- Use
 *   soc_ppd_trap_to_eg_action_map_set() to map trap (reason) to
 *   action profile
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_trap_eg_profile_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                profile_ndx,
    SOC_SAND_IN  SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO         *profile_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_trap_eg_profile_info_set" API.
 *     Refer to "soc_ppd_trap_eg_profile_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_trap_eg_profile_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                profile_ndx,
    SOC_SAND_OUT SOC_PPC_TRAP_EG_ACTION_PROFILE_INFO         *profile_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_trap_mact_event_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Read MACT event from the events FIFO into buffer.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT uint32                                buff -
 *     SOC_PPC_TRAP_EVENT_BUFF_MAX_SIZE]- Buffer to copy the Event
 *     to. Event is copied to buff starting from buff[0] lsb.
 *   SOC_SAND_OUT uint32                                *buff_len -
 *     the actual length of the returned buffer (in longs)
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_trap_mact_event_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT uint32                                buff[SOC_PPC_TRAP_EVENT_BUFF_MAX_SIZE],
    SOC_SAND_OUT uint32                                *buff_len
  );

/*********************************************************************
* NAME:
 *   soc_ppd_trap_mact_event_parse
 * TYPE:
 *   PROC
 * FUNCTION:
 *   The MACT may report different events using the event
 *   FIFO (e.g., learn, age, transplant, and retrieve). This
 *   API Parses the event buffer into a meaningful structure.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                buff -
 *     SOC_PPC_TRAP_EVENT_BUFF_MAX_SIZE]- Buffer includes MACT
 *     event
 *   SOC_SAND_IN  uint32                                buff_len -
 *     the actual length of the given buffer (in longs)
 *   SOC_SAND_OUT SOC_PPC_TRAP_MACT_EVENT_INFO                *mact_event -
 *     MACT Event parsed into structure
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_trap_mact_event_parse(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                buff[SOC_PPC_TRAP_EVENT_BUFF_MAX_SIZE],
    SOC_SAND_IN  uint32                                buff_len,
    SOC_SAND_OUT SOC_PPC_TRAP_MACT_EVENT_INFO                *mact_event
  );

/*********************************************************************
* NAME:
 *   soc_ppd_trap_packet_parse
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Parse a packet received as buffer, identifying the
 *   reason of trapping (if any), the source system port, and
 *   pointer to the packet payload and additional
 *   information.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                 *buff -
 *     buffer includes the packet
 *   SOC_SAND_IN  uint32                                buff_len -
 *     The size of supplied 'buff' In longs
 *   SOC_SAND_OUT SOC_PPC_TRAP_PACKET_INFO                    *packet_info -
 *     Information retrieved by parsing the packet.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_trap_packet_parse(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint8                                 *buff,
    SOC_SAND_IN  uint32                                buff_len,
    SOC_SAND_OUT SOC_PPC_TRAP_PACKET_INFO              *packet_info,
    SOC_SAND_OUT soc_pkt_t                              *dnx_pkt
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_TRAP_MGMT_INCLUDED__*/
#endif

