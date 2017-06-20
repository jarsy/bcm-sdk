/* $Id: ppd_api_diag.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_diag.h
*
* MODULE PREFIX:  soc_ppd_diag
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

#ifndef __SOC_PPD_API_DIAG_INCLUDED__
/* { */
#define __SOC_PPD_API_DIAG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_diag.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_trap_mgmt.h>
#include <soc/dpp/PPD/ppd_api_frwrd_mact.h>
#include <soc/dpp/PPD/ppd_api_frwrd_bmact.h>
#include <soc/dpp/PPD/ppd_api_frwrd_ipv4.h>
#include <soc/dpp/PPD/ppd_api_frwrd_ilm.h>
#include <soc/dpp/PPD/ppd_api_lif.h>
#include <soc/dpp/PPD/ppd_api_rif.h>
#include <soc/dpp/PPD/ppd_api_llp_sa_auth.h>
#include <soc/dpp/PPD/ppd_api_llp_parse.h>
#include <soc/dpp/PPD/ppd_api_mpls_term.h>
#include <soc/dpp/PPD/ppd_api_frwrd_ipv6.h>
#include <soc/dpp/PPD/ppd_api_frwrd_trill.h>
#include <soc/dpp/PPD/ppd_api_lif_ing_vlan_edit.h>
#include <soc/dpp/PPD/ppd_api_eg_encap.h>

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
  SOC_PPD_DIAG_SAMPLE_ENABLE_SET = SOC_PPD_PROC_DESC_BASE_DIAG_FIRST,
  SOC_PPD_DIAG_SAMPLE_ENABLE_SET_PRINT,
  SOC_PPD_DIAG_SAMPLE_ENABLE_GET,
  SOC_PPD_DIAG_SAMPLE_ENABLE_GET_PRINT,
  SOC_PPD_DIAG_MODE_INFO_SET,
  SOC_PPD_DIAG_MODE_INFO_SET_PRINT,
  SOC_PPD_DIAG_MODE_INFO_GET,
  SOC_PPD_DIAG_MODE_INFO_GET_PRINT,
  SOC_PPD_DIAG_PKT_TRACE_CLEAR,
  SOC_PPD_DIAG_PKT_TRACE_CLEAR_PRINT,
  SOC_PPD_DIAG_RECEIVED_PACKET_INFO_GET,
  SOC_PPD_DIAG_RECEIVED_PACKET_INFO_GET_PRINT,
  SOC_PPD_DIAG_PARSING_INFO_GET,
  SOC_PPD_DIAG_PARSING_INFO_GET_PRINT,
  SOC_PPD_DIAG_TERMINATION_INFO_GET,
  SOC_PPD_DIAG_TERMINATION_INFO_GET_PRINT,
  SOC_PPD_DIAG_FRWRD_LKUP_INFO_GET,
  SOC_PPD_DIAG_FRWRD_LKUP_INFO_GET_PRINT,
  SOC_PPD_DIAG_FRWRD_LPM_LKUP_GET,
  SOC_PPD_DIAG_FRWRD_LPM_LKUP_GET_PRINT,
  SOC_PPD_DIAG_TRAPS_INFO_GET,
  SOC_PPD_DIAG_TRAPS_INFO_GET_PRINT,
  SOC_PPD_DIAG_FRWRD_DECISION_TRACE_GET,
  SOC_PPD_DIAG_FRWRD_DECISION_TRACE_GET_PRINT,
  SOC_PPD_DIAG_LEARNING_INFO_GET,
  SOC_PPD_DIAG_LEARNING_INFO_GET_PRINT,
  SOC_PPD_DIAG_ING_VLAN_EDIT_INFO_GET,
  SOC_PPD_DIAG_ING_VLAN_EDIT_INFO_GET_PRINT,
  SOC_PPD_DIAG_PKT_ASSOCIATED_TM_INFO_GET,
  SOC_PPD_DIAG_PKT_ASSOCIATED_TM_INFO_GET_PRINT,
  SOC_PPD_DIAG_ENCAP_INFO_GET,
  SOC_PPD_DIAG_ENCAP_INFO_GET_PRINT,
  SOC_PPD_DIAG_EG_DROP_LOG_GET,
  SOC_PPD_DIAG_EG_DROP_LOG_GET_PRINT,
  SOC_PPD_DIAG_DB_LIF_LKUP_INFO_GET,
  SOC_PPD_DIAG_DB_LIF_LKUP_INFO_GET_PRINT,
  SOC_PPD_DIAG_DB_LEM_LKUP_INFO_GET,
  SOC_PPD_DIAG_DB_LEM_LKUP_INFO_GET_PRINT,
  SOC_PPD_DIAG_PKT_SEND,
  SOC_PPD_DIAG_PKT_SEND_PRINT,
  SOC_PPD_DIAG_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  SOC_PPD_DIAG_VSI_INFO_GET,

  /*
   * Last element. Do no touch.
   */
  SOC_PPD_DIAG_PROCEDURE_DESC_LAST
} SOC_PPD_DIAG_PROCEDURE_DESC;

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
 *   soc_ppd_diag_sample_enable_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable/disable diagnostic APIs.affects only APIs with
 *   type: need_sample
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                               enable -
 *     TRUE: diag APIs are enabled, FALSE diag APIs are
 *     disabled.
 * REMARKS:
 *   - when enabled will affect device power consuming
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_sample_enable_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint8                               enable
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_diag_sample_enable_set" API.
 *     Refer to "soc_ppd_diag_sample_enable_set" API for details.
*********************************************************************/
uint32
  soc_ppd_diag_sample_enable_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT uint8                               *enable
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_mode_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the mode configuration for diag module, including
 *   diag-flavor
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_MODE_INFO                      *mode_info -
 *     Mode of diagnsotcis
 * REMARKS:
 *   Diag Type: All-Packets
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_mode_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_DIAG_MODE_INFO                      *mode_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_diag_mode_info_set" API.
 *     Refer to "soc_ppd_diag_mode_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_diag_mode_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_DIAG_MODE_INFO                      *mode_info
  );

/*********************************************************************
 * NAME:
 *   soc_ppd_diag_vsi_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the VSI information of the last packet.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_VSI_INFO           *vsi_info -
 *     Information regarding the Last packet VSI
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_vsi_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_VSI_INFO                      *vsi_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_pkt_trace_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the trace of transmitted packet, so next trace
 *   info will relate to next packets to transmit
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                pkt_trace -
 *     Packet traces type as encoded by SOC_PPC_DIAG_PKT_TRACE.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_pkt_trace_clear(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  uint32                                pkt_trace
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_received_packet_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the header of last received packet entered the
 *   device and the its association to TM/system/PP ports.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_RECEIVED_PACKET_INFO           *rcvd_pkt_info -
 *     Information regards Last received packet
 * REMARKS:
 *   Diag Type: Last-Packet, need-sampleif sample is disabled
 *   then: last-packet, clear-on-read
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_received_packet_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_RECEIVED_PACKET_INFO           *rcvd_pkt_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_parsing_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns network headers qualified on packet upon
 *   parsing, including packet format, L2 headers fields,...
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_PARSING_INFO                   *pars_info -
 *     Information obtained from parsing including L2 headers,
 *     packet format,...
 * REMARKS:
 *   Diag Type: Last Packet, need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_parsing_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_PARSING_INFO                   *pars_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_termination_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns information obtained by termination including
 *   terminated headers
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_TERM_INFO                      *term_info -
 *     Includes terminated headers,
 * REMARKS:
 *   Diag Type: Last Packet, need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_termination_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_TERM_INFO                      *term_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_frwrd_lkup_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the forwarding lookup performed including:
 *   forwarding type (bridging, routing, ILM, ...), the key
 *   used for the lookup and the result of the lookup
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_LKUP_INFO                *frwrd_info -
 *     forwarding lookup information including key and result
 * REMARKS:
 *   Diag Type: Last Packet, need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_frwrd_lkup_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_LKUP_INFO                *frwrd_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_frwrd_lpm_lkup_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Simulate IP lookup in the device tables and return
 *   FEC-pointer
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_KEY              *lpm_key -
 *     forwarding lookup information including key and result
 *   SOC_SAND_OUT uint32                           *fec_ptr -
 *     FEC pointer
 *   SOC_SAND_OUT uint8                          *found -
 *     Was key found
 * REMARKS:
 *   Diag Type: lookup a key, don't need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_frwrd_lpm_lkup_get(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  int                          core_id,
    SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_KEY              *lpm_key,
    SOC_SAND_OUT uint32                           *fec_ptr,
    SOC_SAND_OUT uint8                          *found,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT            *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_traps_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns information regard packet trapping/snooping,
 *   including which traps/snoops were fulfilled, which
 *   trap/snoop was committed, and whether packet was
 *   forwarded/processed according to trap or according to
 *   normal packet processing flow.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_TRAPS_INFO                *traps_info -
 *     Information regarding the trapping
 * REMARKS:
 *   Diag Type: Last Packet, Clear-on-read. When called after
 *   injecting more than one packet then 'trap_stack' will
 *   hold the status for all injected packets from last call.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_traps_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_TRAPS_INFO                     *traps_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

uint32
  soc_ppd_diag_frwrd_decision_get(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                core_id,
    SOC_SAND_OUT uint32                             *frwrd_trace_info
  );
/*********************************************************************
* NAME:
 *   soc_ppd_diag_frwrd_decision_trace_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Return the trace (changes) for forwarding decision for
 *   last packet in several phases in processing
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO      *frwrd_trace_info -
 *     Forwarding decision in several phases in the processing
 * REMARKS:
 *   Diag Type: Last Packet, Need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_frwrd_decision_trace_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO      *frwrd_trace_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_learning_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the information to be learned for the incoming
 *   packet. This is the information that the processing
 *   determine to be learned, the MACT supposed to learn this
 *   information.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEARN_INFO                     *learn_info -
 *     Learning information including key and value
 *     <destination and additional info (AC, EEP, MPLS command
 *     etc...)>
 * REMARKS:
 *   Diag Type: Last-Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_learning_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_LEARN_INFO                     *learn_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val    
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_ing_vlan_edit_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the result of ingress vlan editing,
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_VLAN_EDIT_RES                  *vec_res -
 *     Vlan edit command result, removed tags and build tags
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_ing_vlan_edit_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_VLAN_EDIT_RES                  *vec_res,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val    
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_pkt_associated_tm_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   The Traffic management information associated with the
 *   packet including meter, DP, TC, etc...
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_PKT_TM_INFO                    *pkt_tm_info -
 *     Traffic management information associated with the
 *     packet
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_pkt_associated_tm_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_PKT_TM_INFO                    *pkt_tm_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val

  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_encap_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the encapsulation and editing information applied to
 *   last packet
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_ENCAP_INFO                     *encap_info -
 *     SOC_SAND_OUT SOC_PPC_DIAG_ENCAP_INFO *encap_info
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_encap_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_ENCAP_INFO                     *encap_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_eg_drop_log_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the reason for packet discard
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_EG_DROP_LOG_INFO               *eg_drop_log -
 *     Egress drop log, reason why packets were dropped.
 * REMARKS:
 *   Diag Type: all Packets since last clear.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_eg_drop_log_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_DIAG_EG_DROP_LOG_INFO               *eg_drop_log
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_db_lif_lkup_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the lookup key and result used in the LIF DB
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info -
 *     The specific use of the DB, for example the lookup
 *     number
 *   SOC_SAND_OUT SOC_PPC_DIAG_LIF_LKUP_INFO                  *lkup_info -
 *     Lookup information, key and result
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_db_lif_lkup_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info,
    SOC_SAND_OUT SOC_PPC_DIAG_LIF_LKUP_INFO                  *lkup_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_diag_db_lem_lkup_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the lookup key and result used in the LEM DB
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info -
 *     The specific use of the DB, for example the lookup
 *     number
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEM_LKUP_TYPE                  *type -
 *     Lookup type
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEM_KEY                        *key -
 *     Lookup key
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEM_VALUE                      *val -
 *     Lookup result
 *   SOC_SAND_OUT uint8                               *valid -
 *     Is lookup result valid.
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_diag_db_lem_lkup_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info,
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_LKUP_TYPE                  *type,
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_KEY                        *key,
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_VALUE                      *val,
    SOC_SAND_OUT uint8                               *valid
  );


/*********************************************************************
* NAME:
 *   soc_ppd_diag_egress_vlan_edit_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Return the egress vlan edit command informations
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO           *prm_vec_res -
 *     
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
  soc_ppd_diag_egress_vlan_edit_info_get( 
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO           *prm_vec_res,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val    
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_DIAG_INCLUDED__*/
#endif

