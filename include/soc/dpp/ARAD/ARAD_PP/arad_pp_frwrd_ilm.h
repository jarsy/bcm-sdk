/* $Id: arad_pp_frwrd_ilm.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ARAD/ARAD_PP/include/arad_pp_frwrd_ilm.h
*
* MODULE PREFIX:  arad_pp
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

#ifndef __ARAD_PP_FRWRD_ILM_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_ILM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>

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
  SOC_PPC_FRWRD_ILM_GLBL_INFO_SET = ARAD_PP_PROC_DESC_BASE_FRWRD_ILM_FIRST,
  SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_PRINT,
  SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_ILM_GLBL_INFO_GET,
  SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_PRINT,
  SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_ILM_ADD,
  ARAD_PP_FRWRD_ILM_ADD_PRINT,
  ARAD_PP_FRWRD_ILM_ADD_UNSAFE,
  ARAD_PP_FRWRD_ILM_ADD_VERIFY,
  ARAD_PP_FRWRD_ILM_GET,
  ARAD_PP_FRWRD_ILM_GET_PRINT,
  ARAD_PP_FRWRD_ILM_GET_UNSAFE,
  ARAD_PP_FRWRD_ILM_GET_VERIFY,
  ARAD_PP_FRWRD_ILM_GET_BLOCK,
  ARAD_PP_FRWRD_ILM_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_ILM_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_ILM_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_ILM_REMOVE,
  ARAD_PP_FRWRD_ILM_REMOVE_PRINT,
  ARAD_PP_FRWRD_ILM_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_ILM_REMOVE_VERIFY,
  ARAD_PP_FRWRD_ILM_TABLE_CLEAR,
  ARAD_PP_FRWRD_ILM_TABLE_CLEAR_PRINT,
  ARAD_PP_FRWRD_ILM_TABLE_CLEAR_UNSAFE,
  ARAD_PP_FRWRD_ILM_TABLE_CLEAR_VERIFY,
  ARAD_PP_FRWRD_ILM_GET_PROCS_PTR,
  ARAD_PP_FRWRD_ILM_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_ILM_PROCEDURE_DESC_LAST
} ARAD_PP_FRWRD_ILM_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FRWRD_ILM_SUCCESS_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FRWRD_ILM_FIRST,
  /*
   * } Auto generated. Do not edit previous section.
   */

   SOC_PPC_FRWRD_ILM_KEY_INPORT_NOT_MASKED_ERR,
   SOC_PPC_FRWRD_ILM_KEY_INRIF_NOT_MASKED_ERR,
   ARAD_PP_FRWRD_ILM_EEI_NOT_MPLS_ERR,
   SOC_PPC_FRWRD_ILM_KEY_MAPPED_EXP_NOT_ZERO_ERR,
   SOC_PPC_FRWRD_ILM_KEY_MASK_NOT_SUPPORTED_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_ILM_ERR_LAST
} ARAD_PP_FRWRD_ILM_ERR;

typedef struct
{
  /*
  * whether to mask InRIF in ILM key
  */
  uint8 mask_inrif;

  /*
   * whether to mask port in ILM key
   */
  uint8 mask_port;

}  ARAD_PP_ILM_INFO;

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
  arad_pp_frwrd_ilm_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_glbl_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the ILM (ingress label
 *   mapping) (including ELSP and key building information)
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info -
 *     Global information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  );

uint32
  arad_pp_frwrd_ilm_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  );

uint32
  arad_pp_frwrd_ilm_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_ilm_glbl_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_ilm_glbl_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add mapping from incoming label to destination and MPLS
 *   command.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the ILM DB (LEM).
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_ilm_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the value (destination and MPLS command) the
 *   incoming label key is associated with.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key -
 *     Key to perform the forwarding lookup. May include
 *     in-label, EXP and in-port.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val -
 *     The destination to forward the packet to and the MPLS
 *     label command to perform. Valid only if found is TRUE.
 *   SOC_SAND_OUT uint8                                 *found -
 *     Does the ilm_key exist in the ILM DB. If TRUE, then
 *     ilm_val is valid.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_frwrd_ilm_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the block of entries from the ILM DB.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range -
 *     Defines block in the ILM database.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY                       *ilm_keys -
 *     Array to include ILM keys.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_vals -
 *     Array to include ILM values.
 *   SOC_SAND_OUT uint32                                  *nof_entries -
 *     Number of valid entries in ilm_key and ilm_val.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY                       *ilm_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO                 *ilm_vals,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_ilm_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove incoming label key from the ILM DB.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key -
 *     ILM key. May include in-label, EXP and in-port.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
  );

uint32
  arad_pp_frwrd_ilm_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_table_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove all keys from the ILM DB.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_table_clear_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pp_frwrd_ilm_table_clear_verify(
    SOC_SAND_IN  int                                 unit
  );

/* parse lem access key for IpV4 host address
*/
  void
    arad_pp_frwrd_ilm_lem_key_parse(
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY *key,
      SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
    );

  void
    arad_pp_frwrd_ilm_lem_payload_parse(
      SOC_SAND_IN int                                      unit,
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_PAYLOAD *payload,
      SOC_SAND_OUT  SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val
    );

void
    arad_pp_frwrd_ilm_lem_key_build(
       SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY  *ilm_key,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY *key
    );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_ilm module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_ilm_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ilm_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_frwrd_ilm module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_ilm_get_errs_ptr(void);

uint32
  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO *info
  );

uint32
  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO *info
  );

uint32
  SOC_PPC_FRWRD_ILM_KEY_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY *info
  );

uint32
  SOC_PPC_FRWRD_ILM_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_ILM_INCLUDED__*/
#endif


