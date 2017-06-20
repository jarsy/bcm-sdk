/* $Id: ppd_api_mpls_term.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_mpls_term.h
*
* MODULE PREFIX:  soc_ppd_mpls
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

#ifndef __SOC_PPD_API_MPLS_TERM_INCLUDED__
/* { */
#define __SOC_PPD_API_MPLS_TERM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_mpls_term.h>

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
  SOC_PPD_MPLS_TERM_LKUP_INFO_SET = SOC_PPD_PROC_DESC_BASE_MPLS_TERM_FIRST,
  SOC_PPD_MPLS_TERM_LKUP_INFO_SET_PRINT,
  SOC_PPD_MPLS_TERM_LKUP_INFO_GET,
  SOC_PPD_MPLS_TERM_LKUP_INFO_GET_PRINT,
  SOC_PPD_MPLS_TERM_LABEL_RANGE_SET,
  SOC_PPD_MPLS_TERM_LABEL_RANGE_SET_PRINT,
  SOC_PPD_MPLS_TERM_LABEL_RANGE_GET,
  SOC_PPD_MPLS_TERM_LABEL_RANGE_GET_PRINT,
  SOC_PPD_MPLS_TERM_RANGE_TERMINATED_LABEL_SET,
  SOC_PPD_MPLS_TERM_RANGE_TERMINATED_LABEL_SET_PRINT,
  SOC_PPD_MPLS_TERM_RANGE_TERMINATED_LABEL_GET,
  SOC_PPD_MPLS_TERM_RANGE_TERMINATED_LABEL_GET_PRINT,
  SOC_PPD_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_SET,
  SOC_PPD_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_SET_PRINT,
  SOC_PPD_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_GET,
  SOC_PPD_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_GET_PRINT,
  SOC_PPD_MPLS_TERM_RESERVED_LABEL_INFO_SET,
  SOC_PPD_MPLS_TERM_RESERVED_LABEL_INFO_SET_PRINT,
  SOC_PPD_MPLS_TERM_RESERVED_LABEL_INFO_GET,
  SOC_PPD_MPLS_TERM_RESERVED_LABEL_INFO_GET_PRINT,
  SOC_PPD_MPLS_TERM_ENCOUNTERED_ENTRIES_GET_BLOCK,
  SOC_PPD_MPLS_TERM_ENCOUNTERED_ENTRIES_GET_BLOCK_PRINT,
  SOC_PPD_MPLS_TERM_COS_INFO_SET,
  SOC_PPD_MPLS_TERM_COS_INFO_SET_PRINT,
  SOC_PPD_MPLS_TERM_COS_INFO_GET,
  SOC_PPD_MPLS_TERM_COS_INFO_GET_PRINT,
  SOC_PPD_MPLS_TERM_LABEL_TO_COS_INFO_SET,
  SOC_PPD_MPLS_TERM_LABEL_TO_COS_INFO_SET_PRINT,
  SOC_PPD_MPLS_TERM_LABEL_TO_COS_INFO_GET,
  SOC_PPD_MPLS_TERM_LABEL_TO_COS_INFO_GET_PRINT,
  SOC_PPD_MPLS_TERM_PROFILE_INFO_GET,
  SOC_PPD_MPLS_TERM_PROFILE_INFO_GET_PRINT,
  SOC_PPD_MPLS_TERM_PROFILE_INFO_SET,
  SOC_PPD_MPLS_TERM_PROFILE_INFO_SET_PRINT,
  SOC_PPD_MPLS_TERM_ACTION_GET,
  SOC_PPD_MPLS_TERM_ACTION_GET_PRINT,
  SOC_PPD_MPLS_TERM_ACTION_SET,
  SOC_PPD_MPLS_TERM_ACTION_SET_PRINT,
  SOC_PPD_MPLS_TERM_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  SOC_PPD_MPLS_TERM_PROCEDURE_DESC_LAST
} SOC_PPD_MPLS_TERM_PROCEDURE_DESC;

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
 *   soc_ppd_mpls_term_lkup_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the lookup to perfrom for MPLS tunnel termination
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LKUP_INFO                 *lkup_info -
 *     Lookup type to perfrom for MPLS tunnel termination, may
 *     be tunnel, <tunnel,inRIF>
 * REMARKS:
 *   - Used for tunnel termination for both: MPLS label or
 *   PWE termination
 *   Not valid for ARAD. (being set on init sequence)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_lkup_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LKUP_INFO                 *lkup_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mpls_term_lkup_info_set" API.
 *     Refer to "soc_ppd_mpls_term_lkup_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_mpls_term_lkup_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LKUP_INFO                 *lkup_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_label_range_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the range of MPLS labels that may be used as
 *   tunnels, and enable terminating those tables
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               range_ndx -
 *     There are 3 different ranges of labels. Range: 0-2.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO          *label_range_info -
 *     MPLS label ranges, of terminated MPLS tunnels. Separated
 *     ranges for pipe and uniform handling
 * REMARKS:
 *   - Soc_petra-B: label can be terminated upon lookup or range.
 *     range termination has priority over lookup.
 *     if label is in on of the ranges, but the label is not
 *     set to be terminated then label will not be terminated at all
 *   - T20E: Range '0' should be configured to PIPE Range '1'
 *   should be configured to UNIFORM Range '2' is currently
 *   invalid- Soc_petra-B: Tunnel outside the tunnel termination
 *   range may be terminated by calling to
 *   soc_ppd_rif_mpls_label_map_add()
  * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_label_range_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               range_ndx,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO          *label_range_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mpls_term_label_range_set" API.
 *     Refer to "soc_ppd_mpls_term_label_range_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_mpls_term_label_range_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               range_ndx,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO          *label_range_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_range_terminated_label_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable / Disable termination of each label in the MPLS
 *   tunnels range
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      label_ndx -
 *     MPLS Label ID. Range: 0-2^20-1
 *   SOC_SAND_IN  uint8                               is_terminated_label -
 *     TRUE: Label is terminated as tunnelFALSE: Label is not
 *     terminated, although it is in the termination range
 * REMARKS:
 *   - Return error if label is not in the range, configured
 *   by soc_ppd_mpls_term_label_range_set().- Soc_petra-B: Tunnel
 *   outside the tunnel termination range may be terminated
 *   by calling to soc_ppd_rif_mpls_label_map_add().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_range_terminated_label_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      label_ndx,
    SOC_SAND_IN  uint8                               is_terminated_label
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mpls_term_range_terminated_label_set" API.
 *     Refer to "soc_ppd_mpls_term_range_terminated_label_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_mpls_term_range_terminated_label_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      label_ndx,
    SOC_SAND_OUT uint8                               *is_terminated_label
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_reserved_labels_global_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Processing information for the MPLS reserved labels.
 *   MPLS Reserved labels are from 0 to 15.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *reserved_labels_info -
 *     SOC_SAND_IN SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO
 *     *reserved_labels_info
 * REMARKS:
 *   - T20E only. The per reserved label processing
 *   information is configured by
 *   soc_ppd_mpls_term_reserved_label_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_reserved_labels_global_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *reserved_labels_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mpls_term_reserved_labels_global_info_set" API.
 *     Refer to "soc_ppd_mpls_term_reserved_labels_global_info_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_mpls_term_reserved_labels_global_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *reserved_labels_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_reserved_label_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the per-reserved label processing information
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      label_ndx -
 *     Reserved label IDRange: 0-15
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO       *label_info -
 *     Termination methods for MPLS labels 0-15
 * REMARKS:
 *   T20E: The global reserved labels processing information
 *   is configured by
 *   soc_ppd_mpls_term_reserved_labels_global_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_reserved_label_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      label_ndx,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO       *label_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mpls_term_reserved_label_info_set" API.
 *     Refer to "soc_ppd_mpls_term_reserved_label_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_mpls_term_reserved_label_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      label_ndx,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO       *label_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_encountered_entries_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Diagnostic tool: Indicates the terminated MPLS label
 *   Ids.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      first_label_ndx -
 *     First MPLS label to scan
 *   SOC_SAND_INOUT uint32                                *nof_encountered_labels -
 *     In: encountered_labels sizeOut: Number of encountered
 *     labels, stored in encountered_labels
 *   SOC_SAND_OUT uint32                                *encountered_labels -
 *     Array of encountered labels. Each entry holds the label
 *     ID
 *   SOC_SAND_OUT SOC_SAND_PP_MPLS_LABEL                      *next_label_id -
 *     When (out nof_encountered_lifs == in
 *     nof_encountered_lifs), there may be more encounters
 *     labels, and the user may recall the function with
 *     first_label_ndx = next_label_id
 * REMARKS:
 *   Returns the label Ids which are terminated, starting
 *   from first label by traversing the terminated range and
 *   adding to the list only those for which the valid bit is
 *   set
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_encountered_entries_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                      first_label_ndx,
    SOC_SAND_INOUT uint32                                *nof_encountered_labels,
    SOC_SAND_OUT uint32                                *encountered_labels,
    SOC_SAND_OUT SOC_SAND_PP_MPLS_LABEL                      *next_label_id
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_cos_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set information of resolving COS parameters whenever
 *   MPLS label is terminated.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_COS_INFO                  *term_cos_info -
 *     How to use terminated tunnel in the calculation of the
 *     COS parameters.
 * REMARKS:
 *   - Relevant only for T20E. Error is returned if called
 *   for Soc_petra-B.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_cos_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_COS_INFO                  *term_cos_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mpls_term_cos_info_set" API.
 *     Refer to "soc_ppd_mpls_term_cos_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_mpls_term_cos_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_COS_INFO                  *term_cos_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_label_to_cos_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from terminated label fields (EXP) to COS
 *   parameters TC and DP.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key -
 *     SOC_SAND_IN SOC_PPC_MPLS_TERM_LABEL_COS_KEY *cos_key
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_VAL             *cos_val -
 *     SOC_SAND_IN SOC_PPC_MPLS_TERM_LABEL_COS_VAL *cos_val
 * REMARKS:
 *   - Relevant only for T20E. Error is returned if called
 *   for Soc_petra-B.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_label_to_cos_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_VAL             *cos_val
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mpls_term_label_to_cos_info_set" API.
 *     Refer to "soc_ppd_mpls_term_label_to_cos_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_mpls_term_label_to_cos_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_COS_VAL             *cos_val
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_profile_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set termination profile attributes. Indicates how to process
 *   the terminated mpls-header
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                term_profile_ndx -
 *     Termination profile Table Entry.
 *     ARAD range: 0-7.
 *   SOC_SAND_IN SOC_PPC_MPLS_TERM_PROFILE_INFO               *term_profile_info -
 *     Termination profile related attributes.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_profile_info_set(
	SOC_SAND_IN  int                             	unit,
	SOC_SAND_IN  uint32 								term_profile_ndx, 
	SOC_SAND_IN  SOC_PPC_MPLS_TERM_PROFILE_INFO 			*term_profile_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_profile_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get termination profile attributes. Indicates how to process
 *   the terminated mpls-header
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                term_profile_ndx -
 *     Termination profile Table Entry.
 *     ARAD range: 0-7.
 *   SOC_SAND_OUT SOC_PPC_MPLS_TERM_PROFILE_INFO              *term_profile_info -
 *     Termination profile related attributes.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_profile_info_get(
	SOC_SAND_IN  int                             	unit,
	SOC_SAND_IN  uint32 								term_profile_ndx, 
	SOC_SAND_OUT SOC_PPC_MPLS_TERM_PROFILE_INFO 			*term_profile_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_action_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set termination action profile attributes. Used to set trap action
 *   for label 
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                action_profile_ndx -
 *     Action profile Table Entry.
 *     ARAD range: 0-7.
 *   SOC_SAND_IN  SOC_PPC_ACTION_PROFILE		              *action_profile_info -
 *     Action profile related attributes.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_action_set(
	  SOC_SAND_IN  int                             	unit,
	  SOC_SAND_IN  uint32                             	action_profile_ndx,
	  SOC_SAND_IN  SOC_PPC_ACTION_PROFILE 						*action_profile
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mpls_term_action_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get termination action profile attributes. Used to set trap action
 *   for label 
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                action_profile_ndx -
 *     Action profile Table Entry.
 *     ARAD range: 0-7.
 *   SOC_SAND_OUT SOC_PPC_ACTION_PROFILE		              *action_profile_info -
 *     Action profile related attributes.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mpls_term_action_get(
	  SOC_SAND_IN  int                             	unit,
	  SOC_SAND_IN  uint32                             	action_profile_ndx,
	  SOC_SAND_OUT SOC_PPC_ACTION_PROFILE 						*action_profile
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_MPLS_TERM_INCLUDED__*/
#endif

