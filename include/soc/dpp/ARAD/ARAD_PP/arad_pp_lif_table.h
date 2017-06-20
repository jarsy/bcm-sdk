
/* $Id: arad_pp_lif_table.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_LIF_TABLE_INCLUDED__
/* { */
#define __ARAD_PP_LIF_TABLE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_lif_table.h>
#include <soc/dpp/PPC/ppc_api_general.h>

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
  ARAD_PP_LIF_TABLE_GET_BLOCK = ARAD_PP_PROC_DESC_BASE_LIF_TABLE_FIRST,
  ARAD_PP_LIF_TABLE_GET_BLOCK_PRINT,
  ARAD_PP_LIF_TABLE_GET_BLOCK_UNSAFE,
  ARAD_PP_LIF_TABLE_GET_BLOCK_VERIFY,
  ARAD_PP_LIF_TABLE_ENTRY_GET,
  ARAD_PP_LIF_TABLE_ENTRY_GET_PRINT,
  ARAD_PP_LIF_TABLE_ENTRY_GET_UNSAFE,
  ARAD_PP_LIF_TABLE_ENTRY_GET_VERIFY,
  ARAD_PP_LIF_TABLE_ENTRY_UPDATE,
  ARAD_PP_LIF_TABLE_ENTRY_UPDATE_PRINT,
  ARAD_PP_LIF_TABLE_ENTRY_UPDATE_UNSAFE,
  ARAD_PP_LIF_TABLE_ENTRY_UPDATE_VERIFY,
  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO_GET,
  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO_GET_UNSAFE,
  ARAD_PP_LIF_TABLE_GET_PROCS_PTR,
  ARAD_PP_LIF_TABLE_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_TABLE_PROCEDURE_DESC_LAST
} ARAD_PP_LIF_TABLE_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LIF_TABLE_TYPE_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LIF_TABLE_FIRST,
  ARAD_PP_LIF_TABLE_ENTRIES_TYPE_BM_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_LIF_TABLE_ENTRY_UPDATE_TYPE_MISMATCH_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LIF_TABLE_ERR_LAST
} ARAD_PP_LIF_TABLE_ERR;

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
  arad_pp_lif_table_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );
 
/*********************************************************************
* NAME:
 *   arad_pp_lif_table_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the LIF Table entries (in specified range), and
 *   retrieve entries that match the given rule.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE         *rule -
 *     Rule to compare the LIF table entries against.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range -
 *     Block range in the MACT.
 *   SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO                      *entries_array -
 *     Logical interfaces info
 *   SOC_SAND_OUT uint32                                  *nof_entries -
 *     Number of valid entries in 'entries_array'.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_table_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE         *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range,
    SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO                      *entries_array,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_lif_table_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE         *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_table_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable updating LIF table entry attribute, without
 *   accessing the LIF KEY.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx -
 *     Lif entry index
 *   SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO                      *lif_entry_info -
 *     LIF entry attributes, according to LIF type.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_table_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO                      *lif_entry_info
  );

uint32
  arad_pp_lif_table_entry_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_table_entry_update_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable updating LIF table entry attribute, without
 *   accessing the LIF KEY.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx -
 *     Lif entry index
 *   SOC_SAND_IN  SOC_PPC_LIF_ENTRY_INFO                      *lif_entry_info -
 *     LIF entry attributes, according to LIF type.
 * REMARKS:
 *   Advanced function. Straitforward usage of the LIF table
 *   is adding or removing the interface according to its
 *   key.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_table_entry_update_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_INFO                      *lif_entry_info
  );

uint32
  arad_pp_lif_table_entry_update_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_INFO                      *lif_entry_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_table_entry_accessed_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   check if entry was accessed upon packet lookup
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx -
 *     Lif entry index
 *   SOC_SAND_IN  uint8                               clear_access_stat -
 *     clear on read
 *   SOC_SAND_OUT  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO       *accessed_info -
 *     was accessed
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_lif_table_entry_accessed_info_get_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx,
    SOC_SAND_IN  uint8                               clear_access_stat,
    SOC_SAND_OUT  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO       *accessed_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_lif_table_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_lif_table module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_lif_table_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_lif_table_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_lif_table module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_lif_table_get_errs_ptr(void);

uint32
  SOC_PPC_LIF_ENTRY_PER_TYPE_INFO_verify(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_PER_TYPE_INFO    *info,
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_TYPE             type
  );

uint32
  SOC_PPC_LIF_ENTRY_INFO_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_INFO *info
  );

uint32
  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE_verify(
    SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LIF_TABLE_INCLUDED__*/
#endif



