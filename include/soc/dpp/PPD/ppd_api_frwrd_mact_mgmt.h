/* $Id: ppd_api_frwrd_mact_mgmt.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_mact_mgmt.h
*
* MODULE PREFIX:  soc_ppd_frwrd
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

#ifndef __SOC_PPD_API_FRWRD_MACT_MGMT_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_MACT_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_frwrd_mact_mgmt.h>

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
  SOC_PPD_FRWRD_MACT_OPER_MODE_INFO_SET = SOC_PPD_PROC_DESC_BASE_FRWRD_MACT_MGMT_FIRST,
  SOC_PPD_FRWRD_MACT_OPER_MODE_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_OPER_MODE_INFO_GET,
  SOC_PPD_FRWRD_MACT_OPER_MODE_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_AGING_INFO_SET,
  SOC_PPD_FRWRD_MACT_AGING_ONE_PASS_SET,
  SOC_PPD_FRWRD_MACT_AGING_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_AGING_INFO_GET,
  SOC_PPD_FRWRD_MACT_AGING_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_SET,
  SOC_PPD_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_GET,
  SOC_PPD_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_SET,
  SOC_PPD_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_SET_PRINT,
  SOC_PPD_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_GET,
  SOC_PPD_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_GET_PRINT,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_SET,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_GET,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_SET,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_GET,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_SET,
  SOC_PPD_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_GET,
  SOC_PPD_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_GET,
  SOC_PPD_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_PROFILE_SET,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_PROFILE_SET_PRINT,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_PROFILE_GET,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_PROFILE_GET_PRINT,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_INFO_SET,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_INFO_GET,
  SOC_PPD_FRWRD_MACT_EVENT_HANDLE_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_SET,
  SOC_PPD_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_GET,
  SOC_PPD_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_SET,
  SOC_PPD_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_GET,
  SOC_PPD_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_PORT_INFO_SET,
  SOC_PPD_FRWRD_MACT_PORT_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_PORT_INFO_GET,
  SOC_PPD_FRWRD_MACT_PORT_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_TRAP_INFO_SET,
  SOC_PPD_FRWRD_MACT_TRAP_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_TRAP_INFO_GET,
  SOC_PPD_FRWRD_MACT_TRAP_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_SET,
  SOC_PPD_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_SET_PRINT,
  SOC_PPD_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_GET,
  SOC_PPD_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_GET_PRINT,
  SOC_PPD_FRWRD_MACT_EVENT_GET,
  SOC_PPD_FRWRD_MACT_EVENT_GET_PRINT,
  SOC_PPD_FRWRD_MACT_EVENT_PARSE,
  SOC_PPD_FRWRD_MACT_EVENT_PARSE_PRINT,
  SOC_PPD_FRWRD_MACT_MGMT_GET_PROCS_PTR,
  SOC_PPD_FRWRD_MACT_MIM_INIT_SET,
  SOC_PPD_FRWRD_MACT_MIM_INIT_GET,
  SOC_PPD_FRWRD_MACT_ROUTED_LEARNING_SET,
  SOC_PPD_FRWRD_MACT_ROUTED_LEARNING_GET,
  /*
   * } Auto generated. Do not edit previous section.
   */

  SOC_PPD_FRWRD_MACT_LEARN_MSG_PARSE,
  SOC_PPD_FRWRD_MACT_LOOKUP_TYPE_SET,

  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_MACT_MGMT_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_MACT_MGMT_PROCEDURE_DESC;


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
 *   Configures the SA lookup type. Also configured 'opportunistic learning',
 *   which is affected by whether SA authentication is enabled.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_lookup_type_set(      
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_LOOKUP_TYPE         lookup_type
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_oper_mode_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the mode of the MACT, including - ingress vs.
 *   egress learning- how each device responds internally to
 *   events (learn/aged-out/refresh) - which events to inform
 *   other devices.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO           *oper_mode_info -
 *     MACT learning setting
 * REMARKS:
 *   - T20E supports only Egress independent and centralized
 *   learning modes
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_oper_mode_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO           *oper_mode_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_oper_mode_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_oper_mode_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_oper_mode_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_OPER_MODE_INFO           *oper_mode_info
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_fid_aging_profile_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map the mac-learn-profile to the fid_aging profile.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                           mac_learn_profile_ndx -
 *     MAC-learn-profile ID. Range: 0 - 3.
 *   SOC_SAND_IN  uint32                           fid_aging_profile -
 *     Profile used to set how MACT events are handled. 
 * REMARKS:
 *   - Soc_petra-B only. Error is return if called for T20E. -
 *   mac_learn_profile_ndx is set according to
 *   soc_ppd_vsi_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_fid_aging_profile_set(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  uint32       mac_learn_profile_ndx,
    SOC_SAND_IN  uint32       fid_aging_profile
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_fid_aging_profile_set" API.
 *     Refer to "soc_ppd_frwrd_mact_fid_aging_profile_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_fid_aging_profile_get(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  uint32       mac_learn_profile_ndx,
    SOC_SAND_OUT uint32      *fid_aging_profile
  );

/*********************************************************************
*     Set the configuration of 'fid_aging_profile'
*     with 'fid_ading_cycles
*********************************************************************/

uint32
  soc_ppd_frwrd_mact_aging_profile_config(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  uint32       fid_aging_profile,
    SOC_SAND_IN uint32        fid_aging_cycles
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_aging_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the aging info including enable aging and aging
 *   time.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_INFO               *aging_info -
 *     Whether to perform aging over the MAC entries and time
 *     of aging.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_aging_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_INFO               *aging_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_aging_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_aging_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_aging_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_AGING_INFO               *aging_info
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_aging_one_pass_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   perform one aging iteration
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO               *pass_info -
 *     Whether to perform aging over the MAC entries and time
 *     of aging.
 *   SOC_SAND_IN  SOC_SAND_SUCCESS_FAILURE               *success -
 *     success or fail
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_aging_one_pass_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO   *pass_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE             *success
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_aging_events_handle_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the device action upon events invoked by the aging
 *   process: - Whether the device deletes aged-out entries
 *   internally - Whether the device generates an event for
 *   aged-out entries - Whether the device generates an event
 *   for refreshed entries
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE            *aging_info -
 *     Device actions upon aging-related events.
 * REMARKS:
 *   - use soc_ppd_frwrd_mact_event_handle_info_set() to set how
 *   the OLP should distribute age-out and refresh events.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_aging_events_handle_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE            *aging_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_aging_events_handle_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_aging_events_handle_info_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_aging_events_handle_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE            *aging_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_fid_profile_to_fid_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Maps FID-Profile to FID, for shared learning.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                fid_profile_ndx -
 *     FID Profile. Range: 1 - 7. The 0 value is used when FID
 *     = VSI.
 *   SOC_SAND_IN  SOC_PPC_FID                                 fid -
 *     Filtering ID. Range: 0 - 16K-1.
 * REMARKS:
 *   - The FID-profile is an attribute of the VSI.- For VSIs
 *   with an FID-profile = 0, FID = VSI.- T20E: This API can
 *   be omitted since for all VSIs, FID = VSI.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_fid_profile_to_fid_map_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                fid_profile_ndx,
    SOC_SAND_IN  SOC_PPC_FID                                 fid
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_fid_profile_to_fid_map_set" API.
 *     Refer to "soc_ppd_frwrd_mact_fid_profile_to_fid_map_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_fid_profile_to_fid_map_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                fid_profile_ndx,
    SOC_SAND_OUT SOC_PPC_FID                                 *fid
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_mac_limit_glbl_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable the MAC limit feature, which limits per fid the
 *   maximum number of entries allowed to be in the MAC
 *   Table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO           *limit_info -
 *     Limitation settings, including if this feature is
 *     enabled and how to act when static entry tries to exceed
 *     the limit.
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_mac_limit_glbl_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO           *limit_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_mac_limit_glbl_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_mac_limit_glbl_info_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_mac_limit_glbl_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO           *limit_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_learn_profile_limit_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the limit information including the MAC-limit (i.e.,
 *   the maximum number of entries an FID can hold in the MAC
 *   Table), and the notification action if the configured
 *   limit is exceeded.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               mac_learn_profile_ndx -
 *     MAC-learn-profile ID. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO          *limit_info -
 *     Limit information including the maximum number of
 *     entries that can be learned/inserted; the action to
 *     perform when an entry is to be inserted/learned while
 *     exceeding the limitation.
 * REMARKS:
 *   - to set 'No limitation' for a specific profile set
 *   limit_info.is_limited = FALSE; - mac_learn_profile_ndx
 *   is set according to soc_ppd_vsi_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_learn_profile_limit_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                mac_learn_profile_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO          *limit_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_learn_profile_limit_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_learn_profile_limit_info_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_learn_profile_limit_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                mac_learn_profile_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO          *limit_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_mac_limit_exceeded_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the information if the MAC limitation is exceeded,
 *   i.e. when a MAC Table entry is tryied to be inserted and
 *   exceeds the limitation set per FID. This insertion can
 *   be triggered by CPU or after a packet learning.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO  *exceed_info -
 *     Information if the MAC limit was exceed, including the
 *     last FID which caused this limitation violation.
 * REMARKS:
 *   - Soc_petra-B only. Error is return if called for T20E.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_mac_limit_exceeded_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO  *exceed_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_event_handle_profile_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map the mac-learn-profile to the event-handle profile.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               mac_learn_profile_ndx -
 *     MAC-learn-profile ID. Range: 0 - 7.
 *   SOC_SAND_IN  uint32                               event_handle_profile -
 *     Profile used to set how MACT events are handled. See
 *     soc_ppd_frwrd_mact_event_handle_info_set(). Range: 0 - 1.
 * REMARKS:
 *   - Soc_petra-B only. Error is return if called for T20E. -
 *   mac_learn_profile_ndx is set according to
 *   soc_ppd_vsi_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_event_handle_profile_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                               mac_learn_profile_ndx,
    SOC_SAND_IN  uint32                               event_handle_profile
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_event_handle_profile_set" API.
 *     Refer to "soc_ppd_frwrd_mact_event_handle_profile_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_event_handle_profile_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                               mac_learn_profile_ndx,
    SOC_SAND_OUT uint32                               *event_handle_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_event_handle_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set how to handle an event according to the event key
 *   parameters (event-type,vsi-handle-profile,is-lag)
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY        *event_key -
 *     The key that identifies the event
 *     (event-type,vsi-handle-profile,is-lag).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO       *handle_info -
 *     How to handle the given events, including where (i.e.,
 *     which FIFO) to send these events.
 * REMARKS:
 *   - Soc_petra-B only. Error is return if called for T20E. - To
 *   set the FIFO configuration, use for the System learn
 *   FIFO soc_ppd_frwrd_mact_sys_learn_msgs_distribution_info_set
 *   and for the Shadow FIFO
 *   soc_ppd_frwrd_mact_shadow_msgs_distribution_info_set. - For
 *   the get API, a single event type must be set in the
 *   event-key parameter.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_event_handle_info_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY        *event_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO       *handle_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_event_handle_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_event_handle_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_event_handle_info_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY        *event_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO       *handle_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_learn_msgs_distribution_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set how to distribute the learn messages to other
 *   devices/CPU.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO          *distribution_info -
 *     Distribution information, in particular the header to
 *     associate with the learn messages.
 * REMARKS:
 *   - Soc_petra-B only. Error is return if called for T20E. -
 *   Soc_petra-B: both learn messages and shadow message must
 *   have the same external header, i.e. either both have an
 *   ITMH Header or none of them. - The get API returns also
 *   the EtherType if not inserted by the user.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_learn_msgs_distribution_info_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO          *distribution_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_learn_msgs_distribution_info_set" API.
 *     Refer to
 *     "soc_ppd_frwrd_mact_learn_msgs_distribution_info_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_learn_msgs_distribution_info_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO          *distribution_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_shadow_msgs_distribution_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set how to distribute the shadow messages to the other
 *   devices/CPU.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO          *distribution_info -
 *     Distribution information, in particular the header to
 *     associate with the shadow messages.
 * REMARKS:
 *   - Soc_petra-B only. Error is return if called for T20E. -
 *   Soc_petra-B: both learn messages and shadow message must
 *   have the same external header, i.e. either both have an
 *   ITMH Header or none of them. - The get API returns also
 *   the EtherType if not inserted by the user.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_shadow_msgs_distribution_info_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO          *distribution_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_shadow_msgs_distribution_info_set" API.
 *     Refer to
 *     "soc_ppd_frwrd_mact_shadow_msgs_distribution_info_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_shadow_msgs_distribution_info_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO          *distribution_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set per port MACT management information including which
 *   profile to activate when SA is known in this port.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                               local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_PORT_INFO               *port_info -
 *     Port information.
 * REMARKS:
 *   - Use soc_ppd_frwrd_mact_trap_info_set() to set the drop
 *   action for an SA MAC.- Soc_petra-B only, error when called
 *   over T20E device.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_port_info_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  SOC_PPC_PORT                               local_port_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_PORT_INFO               *port_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_port_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_port_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_port_info_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  int                              core_id,
    SOC_SAND_IN  SOC_PPC_PORT                               local_port_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_PORT_INFO               *port_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_trap_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   For each trap type, set the action profile. Different
 *   actions may be assigned to the same trap type according
 *   to the port-profile (4 possibilities).
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE               trap_type_ndx -
 *     Type of the Trap.
 *   SOC_SAND_IN  uint32                               port_profile_ndx -
 *     Per port profile, to enable the setting of different
 *     actions for the same trap type. To set this profile for
 *     a port, use soc_ppd_frwrd_mact_port_info_set().
 *   SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                     *action_profile -
 *     Trap information including snoop/forwarding action. The
 *     trap_code is necessarily 0.
 * REMARKS:
 *   - For the Trap is
 *   SOC_PPC_FRWRD_MACT_TRAP_TYPE_SAME_INTERFACE port_profile_ndx
 *   has to be 0, since there is port profile is not relevant
 *   for this Trap.- Use soc_ppd_frwrd_mact_port_info_set() to
 *   set the mapping from the port profile to the profile for
 *   action.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_trap_info_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE               trap_type_ndx,
    SOC_SAND_IN  uint32                               port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                     *action_profile
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_trap_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_trap_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_trap_info_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE               trap_type_ndx,
    SOC_SAND_IN  uint32                               port_profile_ndx,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE                     *action_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_ip_compatible_mc_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the information for bridging compatible Multicast
 *   MAC addresses.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO   *info -
 *     Compatible Multicast MAC information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_ip_compatible_mc_info_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO   *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_mact_ip_compatible_mc_info_set" API.
 *     Refer to "soc_ppd_frwrd_mact_ip_compatible_mc_info_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_ip_compatible_mc_info_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO   *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_event_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Read MACT event from the events FIFO into buffer.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf -
 *     Buffer to copy the Event to.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_event_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_event_parse
 * TYPE:
 *   PROC
 * FUNCTION:
 *   The MACT may report different events using the event
 *   FIFO (e.g., learn, age, transplant, and retrieve). This
 *   API Parses the event buffer into a meaningful structure.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf -
 *     Buffer includes MACT event
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_INFO              *mact_event -
 *     MACT Event parsed into structure
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_event_parse(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_INFO              *mact_event
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_learn_msg_parse
 * TYPE:
 *   PROC
 * FUNCTION:
 *   parse learning message given as buffer.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_LEARN_MSG            *learn_msg -
 *     learning message as bytes buffer
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO              *learn_events -
 *     learning events
 * REMARKS:
 *   Soc_petra-B:
 *   - Maximum number of events in an individual learn message is 8
 *   - Assuming packet received calling soc_petra_pkt_packet_recv
 *   - Assuming packet includes system header particularly FTMH header
 *   - Assuming called on local device
 *   - Calling soc_pb_pp_frwrd_mact_learn_msg_conf_get_unsafe/soc_pb_pp_frwrd_mact_learn_msg_parse_unsafe
 *   - To bypass above assumptions or to speed up performance user can call directly soc_pb_pp function/s
 *      1. call soc_pb_pp_frwrd_mact_learn_msg_conf_get_unsafe() once to get device configuration
 *      2. call soc_pb_pp_frwrd_mact_learn_msg_parse_unsafe() directly for each received packet
 *      - user may fill SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF differently instead of calling 
 *        soc_pb_pp_frwrd_mact_learn_msg_conf_get_unsafe
 * RETURNS:
 *   OK or ERROR indication.
 *   - error is return if passed packet/buffer is not learning message
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_learn_msg_parse(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_LEARN_MSG                 *learn_msg,
    SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO     *learn_events
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_mim_init_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   set whether mim init function has been called 
 *   set soc arad sw database that will be kept in WB.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                               mim_initialized-
 *     whether mim init function has been called 1/0
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_mim_init_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN   uint8                              mim_initialized
                                  );
/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_mim_init_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   get whether mim init function has been called 
 *   get it from the soc arad sw database 
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                               mim_initialized-
 *     whether mim init function has been called 1/0
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_mim_init_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT uint8                               *mim_initialized
                                  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_routed_learning_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   set routed packets MAC learning for different application types
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                              appFlags-
 *     indicates the L3 applications for which to enable/disable learning
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_routed_learning_set(
    SOC_SAND_IN  int                             unit, 
    SOC_SAND_IN  uint32                             appFlags
                                        );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_routed_learning_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   get routed packets MAC learning mode for different application types
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                              appFlags-
 *     indicates the L3 applications for which learning is enabled
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_routed_learning_get(
    SOC_SAND_IN  int                             unit, 
    SOC_SAND_OUT uint32                             *appFlags
                                        );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mac_limit_range_map_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the mapping information for the a specific mapped values
 *   range. The mapping information contanins the required bit
 *   manipulation for a mapped value in the range, in order to
 *   get the matching entry in the common MACT Limit table.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int8                               range_num -
 *     Range number of the mapped values.
 *     1 up to SOC_PPC_MAX_NOF_MACT_LIMIT_LIF_RANGES.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO *map_info -
 *     The mapping information per range.
 * REMARKS:
 *   - Arad+ only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_mac_limit_range_map_info_get(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_IN  int8                                           range_num,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO    *map_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mac_limit_mapping_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get all the MACT Limit mapping information.
 *   The information includes an entry pointer for invalid mapped
 *   values, range end values for mapped value various ranges.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO *map_info -
 *     General MACT limit mapping information.
 * REMARKS:
 *   - Arad+ only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_mac_limit_mapping_info_get(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO      *map_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_limit_mapped_val_to_table_index_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map a value to the common MACT limit table.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                          mapped_val -
 *     LIF number for mapping to the common limit table.
 *   SOC_SAND_OUT uint32                          *limit_tbl_idx -
 *     Index to the entry in the common limit table.
 *   SOC_SAND_OUT uint32                          *is_reserved -
 *     Flag indicating whether the resulting index points to
 *     the reserved entry for invalid mappings.
 * REMARKS:
 *   Applicable only for Arad+.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_limit_mapped_val_to_table_index_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 mapped_val,
    SOC_SAND_OUT uint32                                 *limit_tbl_idx,
    SOC_SAND_OUT uint32                                 *is_reserved
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_MACT_MGMT_INCLUDED__*/
#endif

