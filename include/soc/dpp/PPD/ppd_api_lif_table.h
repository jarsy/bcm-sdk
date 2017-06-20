/* $Id: ppd_api_lif_table.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_lif_table.h
*
* MODULE PREFIX:  soc_ppd_lif
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

#ifndef __SOC_PPD_API_LIF_TABLE_INCLUDED__
/* { */
#define __SOC_PPD_API_LIF_TABLE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_lif_table.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_lif.h>
#include <soc/dpp/PPD/ppd_api_rif.h>

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
  SOC_PPD_LIF_TABLE_GET_BLOCK = SOC_PPD_PROC_DESC_BASE_LIF_TABLE_FIRST,
  SOC_PPD_LIF_TABLE_GET_BLOCK_PRINT,
  SOC_PPD_LIF_TABLE_ENTRY_GET,
  SOC_PPD_LIF_TABLE_ENTRY_GET_PRINT,
  SOC_PPD_LIF_TABLE_ENTRY_UPDATE,
  SOC_PPD_LIF_TABLE_ENTRY_ACCESSED_INFO_GET,
  SOC_PPD_LIF_TABLE_PRINT_BLOCK,
  SOC_PPD_LIF_TABLE_ENTRY_UPDATE_PRINT,
  SOC_PPD_LIF_TABLE_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LIF_TABLE_PROCEDURE_DESC_LAST
} SOC_PPD_LIF_TABLE_PROCEDURE_DESC;


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
 *   soc_ppd_lif_table_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the LIF Table entries (in specified range), and
 *   retrieve entries that match the given rule.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE         *rule -
 *     Rule to compare the LIF table entries against.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Block range in the MACT.
 *   SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO                      *entries_array -
 *     Logical interfaces info
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of valid entries in 'entries_array'.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lif_table_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE         *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
    SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO                      *entries_array,
    SOC_SAND_OUT uint32                                *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_table_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable updating LIF table entry attribute, without
 *   accessing the LIF KEY.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
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
  soc_ppd_lif_table_entry_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx,
    SOC_SAND_OUT SOC_PPC_LIF_ENTRY_INFO                      *lif_entry_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_table_entry_update
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable updating LIF table entry attribute, without
 *   accessing the LIF KEY.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
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
  soc_ppd_lif_table_entry_update(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx,
    SOC_SAND_IN  SOC_PPC_LIF_ENTRY_INFO                      *lif_entry_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lif_table_entry_accessed_info_get
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
  soc_ppd_lif_table_entry_accessed_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_ndx,
    SOC_SAND_IN  uint8                               clear_access_stat,
    SOC_SAND_OUT  SOC_PPC_LIF_TABLE_ENTRY_ACCESSED_INFO       *accessed_info
  );


uint32
  soc_ppd_lif_table_print_block(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  SOC_PPC_LIF_TBL_TRAVERSE_MATCH_RULE            *rule,
    SOC_SAND_IN SOC_SAND_TABLE_BLOCK_RANGE                        *block_range,
    SOC_SAND_IN SOC_PPC_LIF_ENTRY_INFO                          *entries_array,
    SOC_SAND_IN uint32                                      nof_entries
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LIF_TABLE_INCLUDED__*/
#endif

