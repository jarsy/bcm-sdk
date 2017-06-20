
/* $Id: arad_pp_frwrd_mact.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FRWRD_MACT_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_MACT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <soc/error.h>

#include <soc/dpp/PPC/ppc_api_frwrd_mact_mgmt.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define ARAD_PP_FRWRD_MACT_NOF_MS_IN_SEC                         (1000)
#define ARAD_PP_FRWRD_MACT_NOF_MICROSEC_IN_MS                    (1000)

#define ARAD_PP_MACT_NOF_FLUSH_ENTRIES                   (8)
#define ARAD_PP_L2_TRVRS_ITER_BLK_SIZE                   (130)

#define ARAD_PP_FRWRD_MACT_ILLEGAL_AGE_STATUS            (8)

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

/* Callback type for event handlers */
typedef int (*l2_event_fifo_interrupt_handler_cb_t)(int, uint32*) ;


typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FRWRD_MACT_ENTRY_ADD = ARAD_PP_PROC_DESC_BASE_FRWRD_MACT_FIRST,
  ARAD_PP_FRWRD_MACT_ENTRY_ADD_PRINT,
  ARAD_PP_FRWRD_MACT_ENTRY_ADD_UNSAFE,
  ARAD_PP_FRWRD_MACT_ENTRY_ADD_VERIFY,
  ARAD_PP_FRWRD_MACT_ENTRY_REMOVE,
  ARAD_PP_FRWRD_MACT_ENTRY_REMOVE_PRINT,
  ARAD_PP_FRWRD_MACT_ENTRY_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_MACT_ENTRY_REMOVE_VERIFY,
  ARAD_PP_FRWRD_MACT_ENTRY_GET,
  ARAD_PP_FRWRD_MACT_ENTRY_GET_PRINT,
  ARAD_PP_FRWRD_MACT_ENTRY_GET_UNSAFE,
  ARAD_PP_FRWRD_MACT_ENTRY_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_ENTRY_PACK,
  ARAD_PP_FRWRD_MACT_ENTRY_PACK_PRINT,
  ARAD_PP_FRWRD_MACT_ENTRY_PACK_UNSAFE,
  ARAD_PP_FRWRD_MACT_ENTRY_PACK_VERIFY,
  ARAD_PP_FRWRD_MACT_SW_ENTRY_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_MACT_TRAVERSE,
  ARAD_PP_FRWRD_MACT_TRAVERSE_PRINT,
  ARAD_PP_FRWRD_MACT_TRAVERSE_UNSAFE,
  ARAD_PP_FRWRD_MACT_TRAVERSE_VERIFY,
  ARAD_PP_FRWRD_MACT_TRAVERSE_BY_MAC_LIMIT_PER_TUNNEL_UNSAFE,
  ARAD_PP_FRWRD_MACT_TRAVERSE_STATUS_GET,
  ARAD_PP_FRWRD_MACT_TRAVERSE_STATUS_GET_PRINT,
  ARAD_PP_FRWRD_MACT_TRAVERSE_STATUS_GET_UNSAFE,
  ARAD_PP_FRWRD_MACT_TRAVERSE_STATUS_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_GET_BLOCK,
  ARAD_PP_FRWRD_MACT_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_MACT_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_MACT_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_MACT_PRINT_BLOCK,
  ARAD_PP_FRWRD_MACT_PRINT_BLOCK_PRINT,
  ARAD_PP_FRWRD_MACT_PRINT_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_MACT_PRINT_BLOCK_VERIFY,
  ARAD_PP_FRWRD_MACT_GET_PROCS_PTR,
  ARAD_PP_FRWRD_MACT_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_FRWRD_MACT_KEY_CONVERT,
  ARAD_PP_FRWRD_MACT_PAYLOAD_MASK_BUILD,
  ARAD_PP_FRWRD_MACT_KEY_PARSE,
  ARAD_PP_FRWRD_MACT_PAYLOAD_CONVERT,
  ARAD_PP_FRWRD_MACT_RULE_TO_KEY_CONVERT,
  ARAD_PP_FRWRD_MACT_PAYLOAD_BUILD,
  ARAD_PP_FRWRD_MACT_PAYLOAD_MASK_RETRIEVE,
  ARAD_PP_FRWRD_MACT_SW_TRAVERSE_UNSAFE,
  ARAD_PP_FRWRD_LEM_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_MACT_GET_BLOCK_BY_KEY_UNSAFE,
  ARAD_PP_FRWRD_MACT_SW_ENTRY_ADD_UNSAFE,
  ARAD_PP_FRWRD_MACT_CONVERT_KEY_VALUE_INTO_REQUEST_PAYLOAD,
  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_MACT_PROCEDURE_DESC_LAST
} ARAD_PP_FRWRD_MACT_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPC_FRWRD_MACT_ADD_TYPE_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FRWRD_MACT_FIRST,
  ARAD_PP_FRWRD_MACT_SUCCESS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_FLAVORS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_DIP_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_MACT_KEY_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_AGE_STATUS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_SEC_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_MILI_SEC_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_DIP_MASK_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_COMPARE_MASK_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_UPDATE_MASK_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_NOF_MATCHED_ENTRIES_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_BLOCK_ENTRIES_TO_ACT_OUT_OF_RANGE_ERR,
  ARAD_PP_BLOCK_ENTRIES_TO_SCAN_OUT_OF_RANGE_ERR,
  ARAD_PP_BLOCK_RANGE_ITER_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_ACTION_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_MACT_RETRIEVE_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_TABLE_TRAVERSE_WAITING_ERR,
  ARAD_PP_MACT_ACCESSED_UPDATE_ILLEGAL_ERR,
  ARAD_PP_FRWRD_MACT_FLUSH_OUT_OF_RESOURCE_ERR,
  ARAD_PP_FRWRD_MACT_UNKOWN_PREFIX_APP_ID_ERR,
  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_MACT_ERR_LAST
} ARAD_PP_FRWRD_MACT_ERR;

typedef struct
{
    uint32 flush_db_data[7];
} ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR;

typedef struct
{
  SOC_PPC_FRWRD_MACT_LEARNING_MODE            learning_mode;
  uint8                                       is_petra_a_compatible;
  ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR        flush_db_data_arr[8];
  uint32 flush_entry_use;
  uint8 traverse_mode;
  uint32 num_entries_in_dma_host_memory; /* This parameter does not mean that the host memory was allocated. When it is allocated this is the number of entries */
  uint32 size_of_learn_event;
  uint32 host_memory_allocated_bytes; /*How many bytes were allocated for the host memory. Need to allocate this memory again after warm boot */
} ARAD_PP_FWD_MACT;

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

/* tmp function until DMA for learning is implemented */
void arad_pp_frwrd_mact_handle_learn_event(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 *learn_event);

soc_error_t arad_pp_frwrd_mact_event_handler_callback_register(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  l2_event_fifo_interrupt_handler_cb_t     event_handler_cb);

uint32
  arad_pp_frwrd_mact_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_app_to_prefix_get_unsafe
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
  arad_pp_frwrd_mact_app_to_prefix_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                        app_id,
    SOC_SAND_OUT uint32                        *prefix);


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_entry_pack_unsafe
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
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE             add_type -
 *     Describe whether it's insert/transplant a entry to/from MACT table.
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
  arad_pp_frwrd_mact_entry_pack_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  uint8                                       insert,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value,
    SOC_SAND_OUT uint32                                      *data,
    SOC_SAND_OUT uint8                                       *data_len
  );

uint32
  arad_pp_frwrd_mact_entry_pack_verify(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to the MAC table. Given a key, e.g. (FID,
 *   MAC), the packets associated with this FID and having
 *   this MAC address as DA will be processed and forwarded
 *   according to the given value.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type -
 *     Describe how this entry is added (insert/learn/refresh).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key -
 *     The MACT key, e.g. (FID, MAC address).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value -
 *     The value of the MACT entry including forwarding and
 *     aging information.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
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
  arad_pp_frwrd_mact_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

uint32
  arad_pp_frwrd_mact_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_entry_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an entry from the MAC table according to the
 *   given Key.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
  arad_pp_frwrd_mact_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_IN  int                                sw_only
  );

uint32
  arad_pp_frwrd_mact_entry_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an entry according to its key, e.g. (FID, MAC
 *   address).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key -
 *     The MACT key (FID, MAC address).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value -
 *     The value of the MACT entry including forwarding and
 *     aging information.
 *   SOC_SAND_OUT uint8                                 *found -
 *     If TRUE, then the key exists in the MAC Table and the
 *     corresponding value is returned. Otherwise, the key does
 *     not exist and 'mac_entry_value' is not affected
 * REMARKS:
 *   - key can be (FID, DIP) for IPv4 compatible MC packets.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_frwrd_mact_entry_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_traverse_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the MACT entries. Compare each entry to a given
 *   rule, and for matching entries perform the given action.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule -
 *     Rule against which the MACT entries are compared.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION          *action -
 *     Action to perform for the matching matched entries
 *     (remove, modify, none).
 *   SOC_SAND_IN  uint8                                 wait_till_finish -
 *     If TRUE, then the function returns only when the
 *     traverse operation is terminated; otherwise, it returns
 *     immediately.
 *   SOC_SAND_OUT uint32                                  *nof_matched_entries -
 *     Number of entries in the MACT that matched the given
 *     rule. Valid only if wait_till_finish is TRUE.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_traverse_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION          *action,
    SOC_SAND_IN  uint8                                 wait_till_finish,
    SOC_SAND_OUT uint32                                  *nof_matched_entries
  );

uint32
  arad_pp_frwrd_mact_rule_to_key_convert(
      SOC_SAND_IN  int                             unit,
      SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE *rule,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY                  *rule_key,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY                  *rule_key_mask,
      SOC_SAND_IN  uint32                              mask_val
    );

uint32
  arad_pp_frwrd_mact_traverse_internal_unsafe(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *rule_key,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *rule_key_mask,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE  *rule_val,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION        *action,
    SOC_SAND_IN  uint8                                 wait_till_finish,
    SOC_SAND_OUT uint32                                  *nof_matched_entries
  );

uint32
  arad_pp_frwrd_mact_traverse_by_mac_limit_per_tunnel_internal_unsafe(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *rule_key,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *rule_key_mask,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE  *rule_val,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION        *action,
    SOC_SAND_OUT uint32                                  *nof_matched_entries
  );

uint32
  arad_pp_frwrd_mact_traverse_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION          *action,
    SOC_SAND_IN  uint8                                 wait_till_finish
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_traverse_status_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the status of the traverse, including which
 *   action is performed and according to what rule, besides
 *   the expected time to finish the traverse and the number
 *   of matched entries if the traverse was finished.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
  arad_pp_frwrd_mact_traverse_status_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO     *status
  );

uint32
  arad_pp_frwrd_mact_traverse_mode_info_set_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TRAVERSE_MODE_INFO  *mode
  );

uint32
  arad_pp_frwrd_mact_traverse_mode_info_get_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_TRAVERSE_MODE_INFO  *mode
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the MAC Table entries (in specified range), and
 *   retrieve entries that match the given rule.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule -
 *     Rule to compare the MACT entries against.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range -
 *     Block range in the MACT.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys -
 *     Array to include MACT keys.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals -
 *     Array to include MACT entries values.
 *   SOC_SAND_OUT uint32                                  *nof_entries -
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
  arad_pp_frwrd_mact_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_get_sw_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the SW shadow of the MAC Table entries (in specified range), and
 *   retrieve entries that match the given rule.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule -
 *     Rule to compare the MACT entries against.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range -
 *     Block range in the MACT.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys -
 *     Array to include MACT keys.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals -
 *     Array to include MACT entries values.
 *   SOC_SAND_OUT uint32                                  *nof_entries -
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
  arad_pp_frwrd_mact_get_sw_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mact_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mact_vals,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_mact_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE      *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range
  );

uint32
  arad_pp_frwrd_mact_key_parse(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY       *key,
      SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_KEY *mac_entry_key
    );

uint32
  arad_pp_frwrd_mact_payload_convert(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD     *payload,
      SOC_SAND_OUT SOC_PPC_FRWRD_MACT_ENTRY_VALUE *mac_entry_value
  );


#define ARAD_PP_FRWRD_LEM_GET_BLOCK_ACCESSED_ONLY (0x1)
#define ARAD_PP_FRWRD_LEM_GET_BLOCK_CLEAR_ACCESSED (0x2)
#define ARAD_PP_FRWRD_LEM_GET_BLOCK_KBP_HOST_INDICATION (0x4)


uint32
  arad_pp_frwrd_lem_get_block_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *key,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *key_mask,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE   *value_rule,
    SOC_SAND_OUT uint32                                  access_only,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE            *block_range,
    SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_KEY                 *read_keys,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD              *read_vals,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_lem_traverse_internal_unsafe(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *key,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                  *key_mask,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE  *value_rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION        *action,
    SOC_SAND_IN  uint8                                 wait_till_finish,
    SOC_SAND_OUT uint32                                  *nof_matched_entries
  );

uint32
  arad_pp_frwrd_lem_traverse_by_mac_limit_per_tunnel_internal_unsafe(
    SOC_SAND_IN  int                                        unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                       *rule_key,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY                       *rule_key_mask,
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE  *rule_val,
    SOC_SAND_INOUT    SOC_SAND_TABLE_BLOCK_RANGE              *block_range,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION           *action,
    SOC_SAND_OUT uint32                                       *nof_matched_entries 
  );

uint32
  arad_pp_frwrd_mact_key_convert(
      SOC_SAND_IN  int                          unit,
      SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY *mac_entry_key,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY       *key
    );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_mact module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_mact_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_frwrd_mact module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_mact_get_errs_ptr(void);

uint32
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR_verify(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR *info
  );

uint32
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC_verify(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC *info
  );

uint32
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL *info
  );

uint32
  SOC_PPC_FRWRD_MACT_ENTRY_KEY_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY *info
  );

uint32
  SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE *info
  );

uint32
  SOC_PPC_FRWRD_MACT_TIME_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TIME *info
  );

uint32
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC_verify(
    SOC_SAND_IN  int                                            unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_MAC *info
  );

uint32
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC_verify(
    SOC_SAND_IN  int                                                unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_KEY_RULE_IPV4_MC *info
  );

uint32
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE *info
  );

uint32
  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE_verify(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE *info
  );

uint32
  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION *info
  );

uint32
  SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAVERSE_STATUS_INFO *info
  );

uint32
arad_pp_lem_dbal_access_mact_entry_add_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY      *mac_entry_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE    *mac_entry_value,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE          *success
  );

uint32
  arad_pp_lem_dbal_access_mact_entry_get_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY      *mac_entry_key,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD        *payload,
    SOC_SAND_OUT uint8                             *found
  );

uint32
  arad_pp_lem_dbal_access_mact_entry_remove_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY      *mac_entry_key
  );
/* } */

/***************************************************************************
 *      Reset the flush machine's rules, current_index and pause registers
****************************************************************************/
void arad_pp_frwrd_mact_clear_flush_operation(SOC_SAND_IN  int unit);


#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_MACT_INCLUDED__*/
#endif



