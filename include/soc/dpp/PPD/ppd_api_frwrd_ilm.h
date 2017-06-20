/* $Id: ppd_api_frwrd_ilm.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_ilm.h
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

#ifndef __SOC_PPD_API_FRWRD_ILM_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_ILM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ilm.h>

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
  SOC_PPD_FRWRD_ILM_GLBL_INFO_SET = SOC_PPD_PROC_DESC_BASE_FRWRD_ILM_FIRST,
  SOC_PPD_FRWRD_ILM_GLBL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_ILM_GLBL_INFO_GET,
  SOC_PPD_FRWRD_ILM_GLBL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_ILM_ADD,
  SOC_PPD_FRWRD_ILM_ADD_PRINT,
  SOC_PPD_FRWRD_ILM_GET,
  SOC_PPD_FRWRD_ILM_GET_PRINT,
  SOC_PPD_FRWRD_ILM_GET_BLOCK,
  SOC_PPD_FRWRD_ILM_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_ILM_REMOVE,
  SOC_PPD_FRWRD_ILM_REMOVE_PRINT,
  SOC_PPD_FRWRD_ILM_TABLE_CLEAR,
  SOC_PPD_FRWRD_ILM_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_ILM_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_ILM_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_ILM_PROCEDURE_DESC;

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
 *   soc_ppd_frwrd_ilm_glbl_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the ILM (ingress label
 *   mapping) (including ELSP and key building information)
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info -
 *     Global information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ilm_glbl_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_ilm_glbl_info_set" API.
 *     Refer to "soc_ppd_frwrd_ilm_glbl_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_ilm_glbl_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ilm_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add mapping from incoming label to destination and MPLS
 *   command.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key -
 *     Key to perform the forwarding lookup. May include
 *     in-label, EXP and in-port.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val -
 *     ILM Forwading decision. The destination to forward the
 *     packet to and the MPLS label command to perform. -
 *     Destination must be FEC pointer, or MC destination;
 *     error is returned otherwise.- EEI is MPLS command. MPLS
 *     command must be Pop (PHP application), or swap (LSR
 *     application), error will be returned if command is of
 *     type Push.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the ILM DB (LEM).
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ilm_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ilm_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the value (destination and MPLS command) the
 *   incoming label key is associated with.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key -
 *     Key to perform the forwarding lookup. May include
 *     in-label, EXP and in-port.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val -
 *     The destination to forward the packet to and the MPLS
 *     label command to perform. Valid only if found is TRUE.
 *   SOC_SAND_OUT uint8                               *found -
 *     Does the ilm_key exist in the ILM DB. If TRUE, then
 *     ilm_val is valid.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ilm_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val,
    SOC_SAND_OUT uint8                               *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ilm_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the block of entries from the ILM DB.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Defines block in the ILM database.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY                       *ilm_keys -
 *     Array to include ILM keys.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_vals -
 *     Array to include ILM values.
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of valid entries in ilm_key and ilm_val.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ilm_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY                       *ilm_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_vals,
    SOC_SAND_OUT uint32                                *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ilm_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove incoming label key from the ILM DB.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key -
 *     ILM key. May include in-label, EXP and in-port.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ilm_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ilm_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove all keys from the ILM DB.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ilm_table_clear(
    SOC_SAND_IN  int                               unit
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_ILM_INCLUDED__*/
#endif

