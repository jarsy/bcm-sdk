/* $Id: arad_pp_mpls_term.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_MPLS_TERM_INCLUDED__
/* { */
#define __ARAD_PP_MPLS_TERM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_mpls_term.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_MPLS_TERM_PROCESSING_TYPE_MAX              (SOC_PPC_NOF_MPLS_TERM_MODEL_TYPES-1)

#define ARAD_PP_MPLS_TERM_NOF_HEADERS_MIN                  (1)
#define ARAD_PP_MPLS_TERM_NOF_HEADERS_MAX                  (3)
#define ARAD_PP_MPLS_TERM_TTL_EXP_MAX                      (1)


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
  SOC_PPC_MPLS_TERM_LKUP_INFO_SET = ARAD_PP_PROC_DESC_BASE_MPLS_TERM_FIRST,
  SOC_PPC_MPLS_TERM_LKUP_INFO_SET_PRINT,
  SOC_PPC_MPLS_TERM_LKUP_INFO_SET_UNSAFE,
  SOC_PPC_MPLS_TERM_LKUP_INFO_SET_VERIFY,
  SOC_PPC_MPLS_TERM_LKUP_INFO_GET,
  SOC_PPC_MPLS_TERM_LKUP_INFO_GET_PRINT,
  SOC_PPC_MPLS_TERM_LKUP_INFO_GET_VERIFY,
  SOC_PPC_MPLS_TERM_LKUP_INFO_GET_UNSAFE,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_SET,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_SET_PRINT,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_SET_UNSAFE,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_SET_VERIFY,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_GET,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_GET_PRINT,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_GET_VERIFY,
  SOC_PPC_MPLS_TERM_LABEL_RANGE_GET_UNSAFE,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_SET,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_SET_PRINT,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_SET_UNSAFE,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_SET_VERIFY,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_GET,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_GET_PRINT,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_GET_VERIFY,
  ARAD_PP_MPLS_TERM_RANGE_TERMINATED_LABEL_GET_UNSAFE,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_SET,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_SET_PRINT,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_SET_UNSAFE,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_SET_VERIFY,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_GET,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_GET_PRINT,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_GET_VERIFY,
  ARAD_PP_MPLS_TERM_RESERVED_LABELS_GLOBAL_INFO_GET_UNSAFE,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_SET,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_SET_PRINT,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_SET_UNSAFE,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_SET_VERIFY,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_GET,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_GET_PRINT,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_GET_VERIFY,
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_GET_UNSAFE,
  ARAD_PP_MPLS_TERM_ENCOUNTERED_ENTRIES_GET_BLOCK,
  ARAD_PP_MPLS_TERM_ENCOUNTERED_ENTRIES_GET_BLOCK_PRINT,
  ARAD_PP_MPLS_TERM_ENCOUNTERED_ENTRIES_GET_BLOCK_UNSAFE,
  ARAD_PP_MPLS_TERM_ENCOUNTERED_ENTRIES_GET_BLOCK_VERIFY,
  SOC_PPC_MPLS_TERM_COS_INFO_SET,
  SOC_PPC_MPLS_TERM_COS_INFO_SET_PRINT,
  SOC_PPC_MPLS_TERM_COS_INFO_SET_UNSAFE,
  SOC_PPC_MPLS_TERM_COS_INFO_SET_VERIFY,
  SOC_PPC_MPLS_TERM_COS_INFO_GET,
  SOC_PPC_MPLS_TERM_COS_INFO_GET_PRINT,
  SOC_PPC_MPLS_TERM_COS_INFO_GET_VERIFY,
  SOC_PPC_MPLS_TERM_COS_INFO_GET_UNSAFE,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_SET,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_SET_PRINT,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_SET_UNSAFE,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_SET_VERIFY,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_GET,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_GET_PRINT,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_GET_VERIFY,
  ARAD_PP_MPLS_TERM_LABEL_TO_COS_INFO_GET_UNSAFE,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_SET,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_SET_print,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_SET_UNSAFE,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_SET_VERIFY,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_GET,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_GET_print,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_GET_UNSAFE,
  SOC_PPC_MPLS_TERM_PROFILE_INFO_GET_VERIFY,
  ARAD_PP_MPLS_TERM_ACTION_SET,
  ARAD_PP_MPLS_TERM_ACTION_SET_PRINT,
  ARAD_PP_MPLS_TERM_ACTION_SET_UNSAFE,
  ARAD_PP_MPLS_TERM_ACTION_SET_VERIFY,
  ARAD_PP_MPLS_TERM_ACTION_GET,
  ARAD_PP_MPLS_TERM_ACTION_GET_PRINT,
  ARAD_PP_MPLS_TERM_ACTION_GET_UNSAFE,
  ARAD_PP_MPLS_TERM_ACTION_GET_VERIFY,
  ARAD_PP_MPLS_TERM_GET_PROCS_PTR,
  ARAD_PP_MPLS_TERM_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
   ARAD_PP_IHP_MPLS_TUNNEL_TERMINATION_UPDATE_BASES,



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_MPLS_TERM_PROCEDURE_DESC_LAST
} ARAD_PP_MPLS_TERM_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_MPLS_TERM_RANGE_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_MPLS_TERM_FIRST,
  ARAD_PP_MPLS_TERM_IS_TERMINATED_LABEL_OUT_OF_RANGE_ERR,
  SOC_PPC_MPLS_TERM_KEY_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_PROCESSING_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_MODEL_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_MPLS_TERM_RSRVD_ACTION_STRENGTH_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_RSRVD_TRAP_CODE_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_SIMPLE_RANGE_TERM_NEXT_NOT_MPLS_ERR,
  ARAD_PP_MPLS_TERM_RANGE_EXCEED_BITMAP_SIZE_ERR,
  ARAD_PP_MPLS_TERM_RANGE_LAST_SMALLER_THAN_FIRST_ERR,
  ARAD_PP_MPLS_TERM_RANGE_NOT_KEEP_ORDER_ERR,
  ARAD_PP_MPLS_TERM_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_ACTION_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_NOF_HEADERS_OUT_OF_RANGE_ERR,
  ARAD_PP_MPLS_TERM_TTL_EXP_OUT_OF_RANGE_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_MPLS_TERM_ERR_LAST
} ARAD_PP_MPLS_TERM_ERR;

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
  arad_pp_mpls_term_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );
/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_lkup_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the lookup to perfrom for MPLS tunnel termination
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LKUP_INFO                 *lkup_info -
 *     Lookup type to perfrom for MPLS tunnel termination, may
 *     be tunnel, <tunnel,inRIF>
 * REMARKS:
 *   - Used for tunnel termination for both: MPLS label or
 *   PWE termination
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mpls_term_lkup_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LKUP_INFO                 *lkup_info
  );

uint32
  arad_pp_mpls_term_lkup_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LKUP_INFO                 *lkup_info
  );

uint32
  arad_pp_mpls_term_lkup_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mpls_term_lkup_info_set_unsafe" API.
 *     Refer to "arad_pp_mpls_term_lkup_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mpls_term_lkup_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LKUP_INFO                 *lkup_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_label_range_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the range of MPLS labels that may be used as
 *   tunnels, and enable terminating those tables
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                 range_ndx -
 *     There are 3 different ranges of labels. Range: 0-2.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO          *label_range_info -
 *     MPLS label ranges, of terminated MPLS tunnels. Separated
 *     ranges for pipe and uniform handling
 * REMARKS:
 *   - T20E: Range '0' should be configured to PIPE Range '1'
 *   should be configured to UNIFORM Range '2' is currently
 *   invalid- Arad-B: Tunnel outside the tunnel termination
 *   range may be terminated by calling to
 *   soc_ppd_rif_mpls_label_map_add()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mpls_term_label_range_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 range_ndx,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO          *label_range_info
  );

uint32
  arad_pp_mpls_term_label_range_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 range_ndx,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO          *label_range_info
  );

uint32
  arad_pp_mpls_term_label_range_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 range_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mpls_term_label_range_set_unsafe" API.
 *     Refer to "arad_pp_mpls_term_label_range_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_mpls_term_label_range_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 range_ndx,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO          *label_range_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_range_terminated_label_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable / Disable termination of each label in the MPLS
 *   tunnels range
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx -
 *     MPLS Label ID. Range: 0-2^20-1
 *   SOC_SAND_IN  uint8                                 is_terminated_label -
 *     TRUE: Label is terminated as tunnelFALSE: Label is not
 *     terminated, although it is in the termination range
 * REMARKS:
 *   - Return error if label is not in the range, configured
 *   by soc_ppd_mpls_term_label_range_set().- Arad-B: Tunnel
 *   outside the tunnel termination range may be terminated
 *   by calling to soc_ppd_rif_mpls_label_map_add().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mpls_term_range_terminated_label_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx,
    SOC_SAND_IN  uint8                                 is_terminated_label
  );

uint32
  arad_pp_mpls_term_range_terminated_label_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx,
    SOC_SAND_IN  uint8                                 is_terminated_label
  );

uint32
  arad_pp_mpls_term_range_terminated_label_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mpls_term_range_terminated_label_set_unsafe" API.
 *     Refer to
 *     "arad_pp_mpls_term_range_terminated_label_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_mpls_term_range_terminated_label_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx,
    SOC_SAND_OUT uint8                                 *is_terminated_label
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_reserved_labels_global_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Processing information for the MPLS reserved labels.
 *   MPLS Reserved labels are from 0 to 15.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
  arad_pp_mpls_term_reserved_labels_global_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *reserved_labels_info
  );

uint32
  arad_pp_mpls_term_reserved_labels_global_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *reserved_labels_info
  );

uint32
  arad_pp_mpls_term_reserved_labels_global_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mpls_term_reserved_labels_global_info_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_mpls_term_reserved_labels_global_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_mpls_term_reserved_labels_global_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *reserved_labels_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_reserved_label_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the per-reserved label processing information
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx -
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
  arad_pp_mpls_term_reserved_label_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO       *label_info
  );

uint32
  arad_pp_mpls_term_reserved_label_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO       *label_info
  );

uint32
  arad_pp_mpls_term_reserved_label_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mpls_term_reserved_label_info_set_unsafe" API.
 *     Refer to
 *     "arad_pp_mpls_term_reserved_label_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mpls_term_reserved_label_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        label_ndx,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO       *label_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_encountered_entries_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Diagnostic tool: Indicates the terminated MPLS label
 *   Ids.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        first_label_ndx -
 *     First MPLS label to scan
 *   SOC_SAND_INOUT uint32                                  *nof_encountered_labels -
 *     In: encountered_labels sizeOut: Number of encountered
 *     labels, stored in encountered_labels
 *   SOC_SAND_OUT uint32                                  *encountered_labels -
 *     Array of encountered labels. Each entry holds the label
 *     ID
 *   SOC_SAND_OUT SOC_SAND_PP_MPLS_LABEL                        *next_label_id -
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
  arad_pp_mpls_term_encountered_entries_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        first_label_ndx,
    SOC_SAND_INOUT uint32                                  *nof_encountered_labels,
    SOC_SAND_OUT uint32                                  *encountered_labels,
    SOC_SAND_OUT SOC_SAND_PP_MPLS_LABEL                        *next_label_id
  );

uint32
  arad_pp_mpls_term_encountered_entries_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_MPLS_LABEL                        first_label_ndx,
    SOC_SAND_INOUT uint32                                  *nof_encountered_labels
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_cos_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set information of resolving COS parameters whenever
 *   MPLS label is terminated.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_COS_INFO                  *term_cos_info -
 *     How to use terminated tunnel in the calculation of the
 *     COS parameters.
 * REMARKS:
 *   - Relevant only for T20E. Error is returned if called
 *   for Arad-B.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mpls_term_cos_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_COS_INFO                  *term_cos_info
  );

uint32
  arad_pp_mpls_term_cos_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_COS_INFO                  *term_cos_info
  );

uint32
  arad_pp_mpls_term_cos_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mpls_term_cos_info_set_unsafe" API.
 *     Refer to "arad_pp_mpls_term_cos_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_mpls_term_cos_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_COS_INFO                  *term_cos_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_label_to_cos_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mapping from terminated label fields (EXP) to COS
 *   parameters TC and DP.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key -
 *     SOC_SAND_IN SOC_PPC_MPLS_TERM_LABEL_COS_KEY *cos_key
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_VAL             *cos_val -
 *     SOC_SAND_IN SOC_PPC_MPLS_TERM_LABEL_COS_VAL *cos_val
 * REMARKS:
 *   - Relevant only for T20E. Error is returned if called
 *   for Arad-B.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mpls_term_label_to_cos_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_VAL             *cos_val
  );

uint32
  arad_pp_mpls_term_label_to_cos_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_VAL             *cos_val
  );

uint32
  arad_pp_mpls_term_label_to_cos_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_mpls_term_label_to_cos_info_set_unsafe" API.
 *     Refer to "arad_pp_mpls_term_label_to_cos_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_mpls_term_label_to_cos_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY             *cos_key,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_LABEL_COS_VAL             *cos_val
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_profile_info_set_unsafe
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
 *   SOC_SAND_IN SOC_PPC_MPLS_TERM_PROFILE_INFO           *term_profile_info -
 *     Termination profile related attributes.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mpls_term_profile_info_set_unsafe(
	SOC_SAND_IN  int                        unit,
	SOC_SAND_IN  uint32 								        term_profile_ndx, 
	SOC_SAND_IN  SOC_PPC_MPLS_TERM_PROFILE_INFO 	*term_profile_info
  );

uint32
  arad_pp_mpls_term_profile_info_set_verify(
	SOC_SAND_IN  int                        unit,
	SOC_SAND_IN  uint32 								        term_profile_ndx, 
	SOC_SAND_IN  SOC_PPC_MPLS_TERM_PROFILE_INFO 	*term_profile_info
  );

uint32
  arad_pp_mpls_term_profile_info_get_verify(
	SOC_SAND_IN  int                        unit,
	SOC_SAND_IN  uint32 								        term_profile_ndx
  );

/*********************************************************************
*     Gets the termination profile attributes set by the
 *     "arad_pp_mpls_term_profile_info_set_unsafe" API.
 *     Refer to "arad_pp_mpls_term_profile_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_mpls_term_profile_info_get_unsafe(
	SOC_SAND_IN  int                        unit,
	SOC_SAND_IN  uint32 								        term_profile_ndx, 
	SOC_SAND_OUT SOC_PPC_MPLS_TERM_PROFILE_INFO 	*term_profile_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_action_set_unsafe
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
  arad_pp_mpls_term_action_set_unsafe(
	  SOC_SAND_IN  int                             	unit,
	  SOC_SAND_IN  uint32                             	action_profile_ndx,
	  SOC_SAND_IN  SOC_PPC_ACTION_PROFILE       					*action_profile
  );

uint32
  arad_pp_mpls_term_action_set_verify(
	  SOC_SAND_IN  int                             	unit,
	  SOC_SAND_IN  uint32                             	action_profile_ndx,
	  SOC_SAND_IN  SOC_PPC_ACTION_PROFILE      						*action_profile
  );

uint32
  arad_pp_mpls_term_action_get_verify(
	  SOC_SAND_IN  int                             	unit,
	  SOC_SAND_IN  uint32                             	action_profile_ndx
  );

/*********************************************************************
*     Gets the termination action profile attributes set by the
 *     "arad_pp_mpls_term_action_set_unsafe" API.
 *     Refer to "arad_pp_mpls_term_action_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_mpls_term_action_get_unsafe(
	  SOC_SAND_IN  int                             	unit,
	  SOC_SAND_IN  uint32                             	action_profile_ndx,
	  SOC_SAND_OUT SOC_PPC_ACTION_PROFILE      						*action_profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_mpls_termination_spacial_labels_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Init MPLS special labels termination mechanism (Explicit-NULL).
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   Explicit NULL is required to be done in new API and not on init becuase
 *   it requires TCAM initialization prior of configuration.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t arad_pp_mpls_termination_spacial_labels_init(int unit);

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_mpls_term module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_mpls_term_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_mpls_term_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_mpls_term module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_mpls_term_get_errs_ptr(void);

uint32
  SOC_PPC_MPLS_TERM_LABEL_RANGE_verify(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE *info
  );

uint32
  SOC_PPC_MPLS_TERM_LKUP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LKUP_INFO *info
  );

uint32
  SOC_PPC_MPLS_TERM_INFO_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO *info
  );

uint32
  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO_verify(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_RANGE_INFO *info
  );

uint32
  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO_verify(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABEL_INFO *info
  );

uint32
  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO_verify(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_RESERVED_LABELS_GLBL_INFO *info
  );

uint32
  SOC_PPC_MPLS_TERM_COS_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_COS_INFO *info
  );

uint32
  SOC_PPC_MPLS_TERM_LABEL_COS_KEY_verify(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_KEY *info
  );

uint32
  SOC_PPC_MPLS_TERM_LABEL_COS_VAL_verify(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_LABEL_COS_VAL *info
  );

uint32
  SOC_PPC_MPLS_TERM_PROFILE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_PROFILE_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_MPLS_TERM_INCLUDED__*/
#endif
