/* $Id: ppd_api_frwrd_mact.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_mact.h
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

#ifndef __SOC_PPD_API_FRWRD_MACT_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_MACT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_frwrd_mact.h>

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
  SOC_PPD_FRWRD_MACT_ENTRY_ADD = SOC_PPD_PROC_DESC_BASE_FRWRD_MACT_FIRST,
  SOC_PPD_FRWRD_MACT_ENTRY_ADD_PRINT,
  SOC_PPD_FRWRD_MACT_ENTRY_REMOVE,
  SOC_PPD_FRWRD_MACT_ENTRY_REMOVE_PRINT,
  SOC_PPD_FRWRD_MACT_ENTRY_GET,
  SOC_PPD_FRWRD_MACT_ENTRY_GET_PRINT,
  SOC_PPD_FRWRD_MACT_ENTRY_PACK,
  SOC_PPD_FRWRD_MACT_ENTRY_PACK_PRINT,
  SOC_PPD_FRWRD_MACT_TRAVERSE,
  SOC_PPD_FRWRD_MACT_TRAVERSE_PRINT,
  SOC_PPD_FRWRD_MACT_TRAVERSE_BY_MAC_LIMIT_PER_TUNNEL,
  SOC_PPD_FRWRD_MACT_TRAVERSE_BY_MAC_LIMIT_PER_TUNNEL_PRINT,
  SOC_PPD_FRWRD_MACT_TRAVERSE_STATUS_GET,
  SOC_PPD_FRWRD_MACT_TRAVERSE_STATUS_GET_PRINT,
  SOC_PPD_FRWRD_MACT_GET_BLOCK,
  SOC_PPD_FRWRD_MACT_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_MACT_PRINT_BLOCK,
  SOC_PPD_FRWRD_MACT_PRINT_BLOCK_PRINT,
  SOC_PPD_FRWRD_MACT_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_MACT_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_MACT_PROCEDURE_DESC;

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
 *   soc_ppd_frwrd_mact_app_to_prefix_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get prefix value base application id.
 *
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                  app_id -
 *     application ID
 *   SOC_SAND_OUT uint32                                     *prefix -
 *     prefix value base application ID
 *     
 * REMARKS:
 *   - 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_app_to_prefix_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                        app_id,
    SOC_SAND_OUT uint32                        *prefix);

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_entry_pack
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Pack a L2 entry from PPC MAC entry to a buffer(CPU request request register format) .
 *
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                  insert -
 *     Describe whether it's add/delete a entry to/from MACT table.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type -
 *     Describe how this entry is added (insert/transplant).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key -
 *     The MACT key, e.g. (FID, MAC address).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value -
 *     The value of the MACT entry including forwarding and
 *     aging information.
 *   SOC_SAND_OUT uint32                                     *data -
 *     buffer store packed register value
 *   SOC_SAND_OUT uint32                                     *data_len -
 *     the length for register value
 *     
 * REMARKS:
 *   - The key can be (FID, DIP) for IPv4 compatible MC
 *   packets.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_entry_pack(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  uint8                                       insert,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value,
    SOC_SAND_OUT uint32                                      *data,
    SOC_SAND_OUT uint8                                       *data_len
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to the MAC table. Given a key, e.g. (FID,
 *   MAC), the packets associated with this FID and having
 *   this MAC address as DA will be processed and forwarded
 *   according to the given value.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type -
 *     Describe how this entry is added (insert/learn/refresh).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key -
 *     The MACT key, e.g. (FID, MAC address).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value -
 *     The value of the MACT entry including forwarding and
 *     aging information.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     If True, then the entry was inserted in the MAC Table
 *     correctly. Otherwise, an hardware error happened, for
 *     example if the MAC Table is full
 *     (SOC_SAND_FAILURE_OUT_OF_RESOURCES), or the limit per FID is
 *     reached (SOC_SAND_FAILURE_OUT_OF_RESOURCES_2), or an illegal
 *     process happened (e.g., an unknown FID, the request
 *     could not be sent, SOC_SAND_FAILURE_OUT_OF_RESOURCES_3). If
 *     a modification of a static entry is tried,
 *     SOC_SAND_FAILURE_REMOVE_ENTRY_FIRST is returned.
 * REMARKS:
 *   - The key can be (FID, DIP) for IPv4 compatible MC
 *   packets.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_entry_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_entry_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an entry from the MAC table according to the
 *   given Key.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key -
 *     The key (FID, MAC address) of the entry to delete.
 *   SOC_SAND_IN  int                               sw_only -
 *     if sw_only!=0 delete the entry only from the shadow DB in the SW; otherwise delete from the HW and SW DBs
 * REMARKS:
 *   - key can be (FID, DIP) for IPv4 compatible MC packets.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_entry_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  int                                sw_only
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an entry according to its key, e.g. (FID, MAC
 *   address).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key -
 *     The MACT key (FID, MAC address).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value -
 *     The value of the MACT entry including forwarding and
 *     aging information.
 *   SOC_SAND_OUT uint8                               *found -
 *     If TRUE, then the key exists in the MAC Table and the
 *     corresponding value is returned. Otherwise, the key does
 *     not exist and 'mac_entry_value' is not affected
 * REMARKS:
 *   - key can be (FID, DIP) for IPv4 compatible MC packets.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_entry_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value,
    SOC_SAND_OUT uint8                               *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_traverse
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the MACT entries. Compare each entry to a given
 *   rule, and for matching entries perform the given action.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule -
 *     Rule against which the MACT entries are compared.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION          *action -
 *     Action to perform for the matching matched entries
 *     (remove, modify, none).
 *   SOC_SAND_IN  uint8                               wait_till_finish -
 *     If TRUE, then the function returns only when the
 *     traverse operation is terminated; otherwise, it returns
 *     immediately.
 *   SOC_SAND_OUT uint32                                *nof_matched_entries -
 *     Number of entries in the MACT that matched the given
 *     rule. Valid only if wait_till_finish is TRUE.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_traverse(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION          *action,
    SOC_SAND_IN  uint8                               wait_till_finish,
    SOC_SAND_OUT uint32                                *nof_matched_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_traverse_by_mac_limit_per_tunnel
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the MACT entries when MAC limit per tunnel is enabled.
 *   Compare each entry to a given rule, and for matching entries
 *   perform the given action.
 *
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule -
 *     Rule against which the MACT entries are compared.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION          *action -
 *     Action to perform for the matching matched entries
 *     (remove, modify, none).
 *   SOC_SAND_OUT uint32                                *nof_matched_entries -
 *     Number of entries in the MACT that matched the given
 *     rule. Valid only if wait_till_finish is TRUE.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_traverse_by_mac_limit_per_tunnel(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION          *action,
    SOC_SAND_OUT uint32                                *nof_matched_entries
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_traverse_status_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the status of the traverse, including which
 *   action is performed and according to what rule, besides
 *   the expected time to finish the traverse and the number
 *   of matched entries if the traverse was finished.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO     *status -
 *     The status of the traverse operation and its possible
 *     output if the traverse was finished.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_traverse_status_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO     *status
  );


uint32
  soc_ppd_frwrd_mact_traverse_mode_info_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TRAVERSE_MODE_INFO  *mode
  );

uint32
  soc_ppd_frwrd_mact_traverse_mode_info_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_MODE_INFO  *mode
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the MAC Table entries (in specified range), and
 *   retrieve entries that match the given rule.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule -
 *     Rule to compare the MACT entries against.
 *   SOC_SAND_IN SOC_PPC_FRWRD_MACT_TABLE_TYPE               mact_type -
 *     Type of table to fetch the result from. HW or shadow SW
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Block range in the MACT.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys -
 *     Array to include MACT keys.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals -
 *     Array to include MACT entries values.
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of valid entries in mact_key and mact_val.
 * REMARKS:
 *   - mact_keys and mact_vals should point to an allocated
 *   memory of 'entries_to_act' entries. - To set the
 *   iterator to point to the start of the MAC table, use
 *   soc_sand_SAND_TABLE_BLOCK_RANGE_clear(). - To check if the
 *   returned iterator points to the end of the table, use
 *   soc_sand_SAND_TABLE_BLOCK_RANGE_is_end()- Example of use: To
 *   get the whole MACT table using this API:
 *   SOC_SAND_TABLE_BLOCK_RANGE block_range;
 *   SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE rule;
 *   SOC_PPC_FRWRD_MACT_ENTRY_KEY mac_keys[100];
 *   SOC_PPC_FRWRD_MACT_ENTRY_VALUE mac_vals[100];
 *   soc_sand_SAND_TABLE_BLOCK_RANGE_clear(&block_range);
 *   block_range.entries_to_act = 100;
 *   block_range.entries_to_scan = 16K; do{
 *   soc_ppd_frwrd_mact_get_block( &rule, &block_range,mac_keys,
 *   mac_vals,&nof_entries); // do something with the
 *   entries. }
 *   while(!soc_sand_SAND_TABLE_BLOCK_RANGE_is_end(&block_range))
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TABLE_TYPE               mact_type,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals,
    SOC_SAND_OUT uint32                                *nof_entries
  );

#if SOC_PPC_DEBUG_IS_LVL1
/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_mact_print_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print MAC Table entries in the table format.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys -
 *     Array to include MACT keys.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals -
 *     Array to include MACT entries values.
 *   SOC_SAND_IN  uint32                                nof_entries -
 *     Number of valid entries in mact_keys and mact_vals.
 *   SOC_SAND_IN  uint32                                flavors -
 *     Flavors how to print the MAC table
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_mact_print_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals,
    SOC_SAND_IN  uint32                                nof_entries,
    SOC_SAND_IN  uint32                                flavors
  );
#endif
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_MACT_INCLUDED__*/
#endif

