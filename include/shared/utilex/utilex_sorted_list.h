/* $Id: sand_sorted_list.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/**
 * \file utilex_sorted_list.h
 *
 * Definitions and prototypes for all common utilities related
 * to sorted list.
 *
 * Note that sorted-list utilities use occupation-bitmap utilities
 * (utilex_occuation_bitmap.c,.h)
 */
#ifndef UTILEX_SORTED_LIST_H_INCLUDED
/* { */
#define UTILEX_SORTED_LIST_H_INCLUDED

/*************
* INCLUDES  *
*************/
/* { */
#include <shared/utilex/utilex_integer_arithmetic.h>
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_occupation_bitmap.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */
/**
 * \brief
 * If ENHANCED_SORTED_LIST_SEARCH_FOR_DNX is set to a non-zero value then
 * the 'indices' feature is enabled (See utilex_sorted_list_create()).
 * The 'indices' feature adds a direct table to each sorted (linked) list such
 * that it makes it possible to do binary search. 
 */
#define ENHANCED_SORTED_LIST_SEARCH_FOR_DNX 1
/**
 * \brief
 *   Maximal number of sorted list entries to assign at init.
 *   \see utilex_sorted_list_init(), utilex_sorted_list_create()
 */
#define MAX_NOF_LISTS_FOR_DNX           (200 * SOC_DNX_DEFS_GET(unit, nof_cores))
/**
 * \brief
 *   Indication of an empty sorted list.
 * \see
 *   utilex_sorted_list_entry_update()
 */
#define UTILEX_SORTED_LIST_NULL         UTILEX_U32_MAX
/**
 * \brief
 *   Maximal number of characters allowed to be used for printing the
 *   header of a sorted list table.
 * \see
 *   utilex_sorted_list_print()
 */
#define UTILEX_SORTED_LIST_HEADER_SIZE  160
/**
 * \brief
 * Access macros into sw state. Used to make the code more readable.
 */
#define UTILEX_SORTED_LIST_ACCESS          sw_state_access[unit].dnx.shareddnx.sand.sorted_list
#define UTILEX_SORTED_LIST_ACCESS_DATA     UTILEX_SORTED_LIST_ACCESS.lists_array.list_data
#define UTILEX_SORTED_LIST_ACCESS_INFO     UTILEX_SORTED_LIST_ACCESS.lists_array.init_info
/* } */

/*************
 * MACROS    *
 *************/
/* { */
/**
 * \brief
 *   Get value of sorted list iterator at the beginning of the sorted list
 *   Note that, in this system, the value of the 'beginning' iterator is selected as
 *   'the number of elements on the list' and the 'ending' iterator is selected
 *   as one larger than the 'beginning'.
 */
#define UTILEX_SORTED_LIST_ITER_BEGIN(unit,list) utilex_sorted_list_get_iter_begin_or_end(unit,list,1)
/**
 * \brief
 *   Get value of sorted list iterator at end of sorted list.
 *   Note that, in this system, the value of the 'beginning' iterator is selected as
 *   'the number of elements on the list' and the 'ending' iterator is selected
 *   as one larger than the 'beginning'.
 */
#define UTILEX_SORTED_LIST_ITER_END(unit,list)  utilex_sorted_list_get_iter_begin_or_end(unit,list,0)
/**
 * \brief
 * Get place of the head of the list into variable 'head_place' and
 *
 * \par DIRECT INPUT
 *   \param [in] sorted_list_index - \n
 *     Index of sorted list (starting from zero) to extract info \n
 *     from.
 *   \param [in] head_place - \n
 *     Variable to extract info into
 * \par INDIRECT INPUT
 *   * None
 * \par DIRECT OUTPUT
 *   * None
 * \par INDIRECT OUTPUT
 *   * &head_place - \n
 *       See 'head_place' above.
 * \remark
 *   * Caller is assumed to have declared the following: \n
 *       variable 'unit'  \n
 *       address 'exit'   \n
 *   * If operation fails then software goes to 'exit'.
 *
 */
#define UTILEX_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index, head_place) \
  SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&head_place)) ; \

/**
 * \brief
 * Get place of the tail of the list into variable 'tail_place'.
 *
 * \par DIRECT INPUT
 *   \param [in] sorted_list_index - \n
 *     Index of sorted list (starting from zero) to extract info \n
 *     from.
 *   \param [in] tail_place - \n
 *     Variable to extract info into
 * \par INDIRECT OUTPUT
 *   * &tail_place - \n
 *       See 'tail_place' above.
 * \remark
 *   * Caller is assumed to have declared the following: \n
 *       variable 'unit'  \n
 *       address 'exit'   \n
 *   * If operation fails then software goes to 'exit'.
 */
#define UTILEX_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index, tail_place) \
  SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit,sorted_list_index,&tail_place)) ; \
  tail_place += 1 ;
/**
 * \brief
 * Verify specific sorted list index is marked as 'occupied'. \n
 * If not, software goes to exit with error code.
 * 
 * Notes: \n
 *   'unit' is assumed to be defined in the caller's scope.  \n
 *   'exit' is assumed to be defined in the caller's scope.  \n
 */
#define UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(_sorted_list_index) \
  { \
    uint8 bit_val ; \
    uint32 max_nof_lists ; \
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists)) ; \
    if (_sorted_list_index >= max_nof_lists) \
    { \
      /* \
       * If sortedlist handle is out of range then quit with error. \
       */ \
      bit_val = 0 ; \
    } \
    else \
    { \
      SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, (int)_sorted_list_index, &bit_val)) ; \
    } \
    if (bit_val == 0) \
    { \
      /* \
       * If sortedlist structure is not indicated as 'occupied' then quit \
       * with error. \
       */ \
      SHR_IF_ERR_EXIT(_SHR_E_MEMORY) ; \
    } \
  }
/**
 * \brief
 * Verify specified unit has a legal value. If not, software goes to \n
 * exit with error code.
 * 
 * Notes: \n
 *   'exit' is assumed to be defined in the caller's scope.
 */
#define UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit) \
  if (((int)unit < 0) || ((int)unit >= SOC_MAX_NUM_DEVICES)) \
  { \
    /* \
     * If this is an illegal unit identifier, quit \
     * with error. \
     */ \
    SHR_SET_CURRENT_ERR(_SHR_E_UNIT) ; \
    SHR_EXIT() ; \
  }
/**
 * \brief
 * Convert input sorted list handle to index in 'occupied_lists' array.
 *
 * Indices go from 0 -> (occupied_lists - 1) \n
 * Handles go from 1 -> occupied_lists
 */
#define UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(_sorted_list_index,_handle) \
  (_sorted_list_index = _handle - 1)
/**
 * \brief
 * Convert input index in 'occupied_lists' array to sorted list handle.
 *
 * Indices go from 0 -> (occupied_lists - 1) \n
 * Handles go from 1 -> occupied_lists
 */
#define UTILEX_SORTED_LIST_CONVERT_SORTEDLIST_INDEX_TO_HANDLE(_handle,_sorted_list_index) \
  (_handle = _sorted_list_index + 1)
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
/**
 * \brief
 * The type assigned to the 'key' (on the 'key','data' pair)
 */
typedef uint8 UTILEX_SORTED_LIST_KEY_TYPE;
/**
 * \brief
 * The type assigned to the 'data' (on the 'key','data' pair)
 */
typedef uint8 UTILEX_SORTED_LIST_DATA_TYPE;

/**
 * \brief
 * The types of available 'compare' procedures.
 * \see utilex_sorted_list_data_cmp()
 */
typedef enum
{
  /**
   * Default 'compare' procedure (e.g., sal_memcmp())
   */
    UTILEX_SORTED_LIST_CMP_FUNC_TYPE_DEFAULT,
  /**
   * Special 'compare' procedure adapted for TCAM (e.g., utilex_sorted_list_tcam_cmp_priority())
   */
    UTILEX_SORTED_LIST_CMP_FUNC_TYPE_TCAM,
    UTILEX_SORTED_LIST_CMP_FUNC_NOF_TYPES
} utilex_sorted_list_cmp_func_type_e;

/**
 * \brief Type of sorted list key
 */
typedef uint8 UTILEX_SORTED_LIST_KEY;
/**
 * \brief Type of sorted list data
 */
typedef uint8 UTILEX_SORTED_LIST_DATA;
/*
 * \brief Type of iterator to use when running over sorted list.
 * Use this type to traverse the sorted list.
 */
typedef uint32 UTILEX_SORTED_LIST_ITER;

/**
 * \brief
 *   Typedef of procedure used to compare two keys (of elements on sorted list).
 * \par DIRECT INPUT
 *   \param [in]  buffer1 -
 *     Pointer to buffer of uint8s containing first sorted list key (of two)
 *     calculations
 *   \param [in]  buffer2 -
 *     Pointer to buffer of uint8s containing second sorted list key (of two)
 *     calculations
 *   \param [in]  size -
 *     Number of bytes on each key.
 * \par INDIRECT INPUT
 *   * None
 * \par DIRECT OUTPUT
 *   \retval Zero if the two keys are equal
 *   \retval Negative if the key on 'buffer1' is smaller than the key on 'buffer2'
 *   \retval Positive if the key on 'buffer1' is larger than the key on 'buffer2'
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   Built in algorithm, within this procedure, decides on the metric (for comparison)
 */
typedef int32(
    *UTILEX_SORTED_LIST_KEY_CMP) (
    uint8 * buffer1,
    uint8 * buffer2,
    uint32 size);

/**
 * \brief
 *   Typedef of procedure to to load sorted list entry information into
 *   a local SW DB on the device.
 *   See remarks!
 * \par DIRECT INPUT
 *   \param [in] prime_handle -
 *     Handle of the sorted list to identify sorted list instance
 *   \param [in] sec_handle -
 *     Secondary identifier of sorted list instance.
 *   \param [in] buffer -
 *     The memory "array" to write into (the info read from *data).
 *     \b As \b output - \n
 *     Entry info from '*data' is loaded into the address
 *     pointed by buffer+(offset*len)
 *   \param [in] offset -
 *     Offset of the entry in local memory "array".
 *   \param [in] len -
 *     Length in bytes (uint8) of the entry to load (into local memory).
 *   \param [in] data -
 *     Pointer to memory to read information which is to be written
 *     into buffer+(offset*len).
 * \par INDIRECT INPUT
 *   \b *data - \n
 *     See 'data' on DIRECT INPUT above.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *buffer -\n
 *     See 'buffer' on DIRECT INPUT above.
 * \remark
 *   No such procedure is implemented by current code.
 */
typedef uint32(
    *UTILEX_SORTED_LIST_SW_DB_ENTRY_SET) (
    int prime_handle,
    uint32 sec_handle,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * data);

/**
 * \brief
 *   Typedef of procedure to get sorted list entry information from a
 *   local SW DB on the device.
 *   See remarks!
 * \par DIRECT INPUT
 *   \param [in] prime_handle -
 *     Handle of the sorted list to identify sorted list instance
 *   \param [in] sec_handle -
 *     Secondary identifier of sorted list instance.
 *   \param [in] buffer -
 *     The memory "array" to read from (and write into *data).
 *   \param [in] offset -
 *     Offset of the entry in local memory "array".
 *   \param [in] len -
 *     Length in bytes (uint8) of the entry to read (from local memory).
 *   \param [in] data -
 *     Pointer to memory to write output into. \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory by information which
 *       is read from buffer+(offset*len).
 * \par INDIRECT INPUT
 *   \b *buffer - \n
 *     See 'buffer' in DIRECT INPUT above.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *data -\n
 *     See DIRECT INPUT above
 * \remark
 *   No such procedure is implemented by current code.
 */
typedef uint32(
    *UTILEX_SORTED_LIST_SW_DB_ENTRY_GET) (
    int prime_handle,
    uint32 sec_handle,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * data);

/**
 * \brief
 *   Typedef of procedure to print the contents of some general buffer.
 * \par DIRECT INPUT
 *   \param [in] buffer -
 *     Pointer to memory buffer to print.
 * \par INDIRECT INPUT
 *   \b *buffer - \n
 *     See 'buffer' in DIRECT INPUT above.
 * \par DIRECT OUTPUT
 *   * None
 * \par INDIRECT OUTPUT
 *   Printed memory
 * \remark
 *   Needed only in the print API (debug purposes)
 */
typedef void (
    *UTILEX_SORTED_LIST_PRINT_VAL) (
    uint8 * buffer);

/**
 * \brief Structure including the information user has to supply on sorted list creation
 */
typedef struct
{
  /**
   * \brief Handle of the sorted list to identify the sorted list instance.
   */
    int prime_handle;
  /**
   * \brief Handle of the sorted list to identify the sorted list instance.
   */
    uint32 sec_handle;
  /**
   * \brief Size of the sorted list. Maximal number of elements that can
   *   be inserted the sorted list
   */
    uint32 list_size;
  /**
   * \brief Size of the key (in bytes) to sort the list according to.
   */
    uint32 key_size;
  /**
   * \brief Size of the data (in bytes)
   */
    uint32 data_size;
  /**
   * \brief Type of 'compare' function to use (to decide on location
   *   within sorted list).
   *   * For TCAM entries, comparison is made by priority using \n
   *   utilex_sorted_list_tcam_cmp_priority().
   *   * For standard entries, comparison is made by priority using \n
   *   the standard sal_memcmp().
   *   * Comparison procedure should be of type \ref UTILEX_SORTED_LIST_KEY_CMP
   */
    utilex_sorted_list_cmp_func_type_e cmp_func_type;
  /**
   * \brief Procedure to read directly from memory image of sorted list into
   * local entry.
   * Not used on this code, for now.
   */
    UTILEX_SORTED_LIST_SW_DB_ENTRY_GET get_entry_fun;
  /**
   * \brief Procedure to write directly into memory image of sorted list from
   * local entry.
   * Not used on this code, for now.
   */
    UTILEX_SORTED_LIST_SW_DB_ENTRY_SET set_entry_fun;
} UTILEX_SORTED_LIST_INIT_INFO;

/**
 * \brief Structure for one sorted list element.
 */
typedef struct
{
  /**
   * Buffer to contain the full keys to sort the list by.
   */
    SW_STATE_BUFF *keys;
  /**
   * point to the next in the list.
   */
    SW_STATE_BUFF *next;
  /**
   * point to the prev in the list.
   */
    SW_STATE_BUFF *prev;
  /**
   * The data buffer to hold in the linked list.
   */
    SW_STATE_BUFF *data;
  /**
   * Array of all nodes on the linked list, ordered the same way
   * as the linked list itself.
   * To be used for fast direct access to elements on the linked
   * list (e.g., binary search)
   * If this pointer is not loaded then this linked list does not
   * have this feature enabled.
   */
    PARSER_HINT_ARR uint32 *indices;
  /**
   * Number of valid elements on 'indices' array.
   */
    uint32 num_elements_on_indices;
  /**
   * The size of the pointer in bytes.
   */
    uint32 ptr_size;
  /**
   * Handle of occupation bitmap: Available memory of the nodes,
   * for efficient manipulation.
   */
    UTILEX_OCC_BM_PTR memory_use;
  /**
   * Work space for data buffer. Altough it is part of sw state,
   * the data it contains has no value after warm boot.
   */
    SW_STATE_BUFF *tmp_data;
  /**
   * Work space for key buffer. Altough it is part of sw state,
   * the data it contains has no value after warm boot.
   */
    SW_STATE_BUFF *tmp_key;
  /**
   * Value used to indicate null pointer on this system.
   */
    uint32 null_ptr;

} UTILEX_SORTED_LIST_T;
/**
 * \brief
 *   Structure type for containing all info and data corresponding
 *   to one sorted list.
 * \see
 *   \b utilex_sw_state_sorted_list_t
 */
typedef struct
{
    UTILEX_SORTED_LIST_INIT_INFO init_info;
    UTILEX_SORTED_LIST_T list_data;
} UTILEX_SORTED_LIST_INFO;

/* } */
/**
 * \brief This is an identifier of an element of type UTILEX_SORTED_LIST_INFO
 *
 * Replace: typedef UTILEX_SORTED_LIST_INFO*  UTILEX_SORTED_LIST_PTR;
 * because the new software state does not use pointers, only handles.
 * So now, UTILEX_SORTED_LIST_PTR is just a handle to the 'sorted list'
 * structure (actually, index into 'lists_array' {of pointers})
 *
 * Note that the name is kept as is to minimize changes in current code.
 */
typedef uint32 UTILEX_SORTED_LIST_PTR;
/**
 * \brief Control Structure for all created sorted lists. Each list is pointed
 * by lists_array. See utilex_sorted_list_init()
 */
typedef struct
{
  /**
   * \brief Maximal number of sorted lists that may be opened simultaneously.
   */
    uint32 max_nof_lists;
  /**
   * \brief Number of sorted lists that are currently open.
   */
    uint32 in_use;
  /**
   * \brief Array of pointers, each pointing to one sorted list.
   *   On reset, they are all NULL
   */
    PARSER_HINT_ARR_PTR UTILEX_SORTED_LIST_INFO **lists_array;
  /**
   * \brief Bitmap indicating which sorted list (on lists_array[])
   *   is currently open
   */
    PARSER_HINT_ARR SHR_BITDCL *occupied_lists;
} utilex_sw_state_sorted_list_t;

/*************
* GLOBALS   *
*************/
/* { */

/* } */

/*************
* FUNCTIONS *
*************/
/* { */

/**
 * \brief
 *   Initialize control structure for ALL sorted list instances expected.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] max_nof_lists -
 *     Maximal number of sorted lists which can be sustained simultaneously.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Initialized sw_state_access[unit].dnx.shareddnx.sand.sorted_list: \n
 *   * Allocated array of pointers: 'lists_array' of type UTILEX_SORTED_LIST_INFO
 *   * Allocated bit map: 'occupied_lists' of type SHR_BITDCL
 *   * ...
 */
shr_error_e utilex_sorted_list_init(
    int unit,
    uint32 max_nof_lists);

/**
 * \brief
 *   Fill all allocated 'tmp' (sand box) buffers by zeros.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see UTILEX_SORTED_LIST_ACCESS_DATA.tmp_key
 *   and UTILEX_SORTED_LIST_ACCESS_DATA.tmp_data)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Cleared 'tmp' buffers in SWSTATE. See INDIRECT INPUT above.
 * \remark
 *   This procedure is to be used at init before 'diff'ing previous sw
 *   state buffer with current one. This ensures that such buffers are
 *   effectively not 'diff'ed.
 * \see
 *   * utilex_sorted_list_create()
 */
shr_error_e utilex_sorted_list_clear_all_tmps(
    int unit);
/**
 * \brief
 *   Creates a new Sorted List instance.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list_ptr -
 *     Pointer to memory to load output into. \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory by handle (identifier) of
 *       newly created sorted list.
 *   \param [in] init_info -
 *     Pointed memory contains setup parameters required for for the
 *     creation of the sorted list. See \ref UTILEX_SORTED_LIST_INIT_INFO
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *sorted_list_ptr \n
 *     See 'sorted_list_ptr' in DIRECT INPUT above
 * \see
 *   * utilex_sorted_list_init()
 */
shr_error_e utilex_sorted_list_create(
    int unit,
    UTILEX_SORTED_LIST_PTR * sorted_list_ptr,
    UTILEX_SORTED_LIST_INIT_INFO init_info);

/**
 * \brief
 *   Clear the indicated Sorted List's contents without releasing the memory.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle (identifier) of the Sorted List to clear contents of.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Cleared memory on SWSTATE buffer: Both directly on memory assigned
 *   to Sorted List and on corresponding occupation bit map.
 * \remarks
 *   The cleared memory (of Sorted List) is accessed by:     \n
 *   sw_state_access[unit].dnx.shareddnx.sand.sorted_list    \n
 *   The Occupation bit map is cleared using: utilex_occ_bm_clear().
 */
shr_error_e utilex_sorted_list_clear(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list);

/**
 * \brief
 *   Free the specified sorted list instance.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the sorted list to destroy.
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   All resources of specified sorted list are freed. Specifically,
 *   Root pointer at UTILEX_SORTED_LIST_ACCESS.lists_array is cleared.
 * \see
 *   * utilex_sorted_list_create()
 */
shr_error_e utilex_sorted_list_destroy(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list);

/**
 * \brief
 *   Insert a new entry into the sorted list. If entry already
 *   exists then the operation returns an error.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to add. Sorted list is assumed to have been
 *     created.
 *   \param [in] key -
 *     Pointer to key to add into the sorted list. The size of
 *     this array is per setup info specified upon creation of the
 *     Sorted List. See UTILEX_SORTED_LIST_INIT_INFO.key_size
 *   \param [in] data -
 *     Pointer to data to add into the sorted list. The size of
 *     this array is per setup info specified upon creation of the
 *     Sorted List. See UTILEX_SORTED_LIST_INIT_INFO.data_size
 *   \param [in] success -
 *     Pointer to memory to load output into. \n
 *     \b As \b output - \n
 *       Pointed memory is to be loaded by              \n
 *       * TRUE if operation was completed to success (new key was added)  \n
 *       * FALSE if operation has failed
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 *   * See 'key' in DIRECT INPUT above.
 *   * See 'data' in DIRECT INPUT above.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *success -\n
 *     See 'success' in DIRECT INPUT above.
 * \see
 *   * utilex_sorted_list_create()
 * \remarks
 *   To verify success, caller need to check that
 *     * Direct output is zero
 *     * '*success' is loaded by a non-zero value 
 */
shr_error_e utilex_sorted_list_entry_add(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint8 * success);

/**
 * \brief
*    Update 'data' and 'key' on entry pointed by 'iter' on specified 'sorted list'.
*    If any of 'data'/'key' is NULL then it is NOT updated.
*    It is the caller's responsibility to keep the order of the list so that
 *   modifying 'data'/'key' does not violate the order.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to update.
 *   \param [in] iter -
 *     Identifier of entry, on specified Sorted List, to update.
 *   \param [in] key -
 *     Pointer to key to use for updating the sorted list. The size of
 *     this array is per setup info specified upon creation of the
 *     Sorted List. See UTILEX_SORTED_LIST_INIT_INFO.key_size
 *   \param [in] data -
 *     Pointer to data to use for updating the sorted list. The size of
 *     this array is per setup info specified upon creation of the
 *     Sorted List. See UTILEX_SORTED_LIST_INIT_INFO.data_size
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 *   * See 'key' in DIRECT INPUT above.
 *   * See 'data' in DIRECT INPUT above.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Updated entry on Sorted List.
 * \see
 *   * utilex_sorted_list_create()
 */
shr_error_e utilex_sorted_list_entry_update(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data);

/**
 * \brief
 *   Insert an entry into specified Sorted List after/before the
 *   given iterator ('index', identifier).
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to update by inserting a new entry.
 *   \param [in] pos -
 *     Identifier of entry ('iterator'), on specified Sorted List, to update.
 *     See 'remarks' below.
 *   \param [in] before -
 *     Indication on whether to insert the new entry before (value: TRUE)
 *     or after (value: FALSE) the indicated iterator ('pos').
 *   \param [in] key -
 *     Pointer to key to use for the newly inserted entry on
 *     the sorted list. The size of this array is per setup info
 *     specified upon creation of the Sorted List.
 *     See UTILEX_SORTED_LIST_INIT_INFO.key_size
 *     See 'remarks' below.
 *   \param [in] data -
 *     Pointer to data to use for the newly inserted entry on
 *     the sorted list.  The size of this array is per setup info
 *     specified upon creation of the Sorted List.
 *     See UTILEX_SORTED_LIST_INIT_INFO.data_size
 *   \param [in] success -
 *     Pointer to memory to load output into. \n
 *     \b As \b output - \n
 *       Pointed memory is to be loaded by              \n
 *       * TRUE if operation was completed to success (new key was added)  \n
 *       * FALSE if operation has failed
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 *   * See 'key' in DIRECT INPUT above.
 *   * See 'data' in DIRECT INPUT above.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Newly inserted entry on Sorted List.
 * \see
 *   * utilex_sorted_list_entry_add()
 * \remarks
 *   * It is considered an error to try to add after the iterator that
 *     indicates 'end of list' or before the interator indicating
 *     'beginning of list'
 *   * Input 'key' must be such that inserting this new entry will
 *     be as per the order of the list (larger than the one before
 *     and smaller than the one after). Otherwise, an error is returned.
 *   * Operation may fail if there is no space on the list.
 *   * To verify success, caller need to check that
 *     * Direct output is zero
 *     * '*success' is loaded by a non-zero value 
 */
shr_error_e utilex_sorted_list_entry_add_by_iter(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER pos,
    uint8 before,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint8 * success);

/**
 * \brief
 *   Remove an existing entry from the sorted list. If entry does
 *   not exist then the operation returns an error.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to add. Sorted list is assumed to have been
 *     created.
 *   \param [in] iter -
 *     Identifier of entry ('iterator'), on specified Sorted List, to remove.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Removed entry
 * \see
 *   * utilex_sorted_list_entry_add_by_iter()
 */
shr_error_e utilex_sorted_list_entry_remove_by_iter(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter);
/**
 * \brief
 *   Lookup  the Sorted List for the given key plus data and return the iterator
 *   identifying this data and key.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     '*key' points to the key to look for on Sorted List.
 *   \param [in] data -
 *     Pointer to UTILEX_SORTED_LIST_DATA (array of uint8s). \n
 *     '*data' points to the data to look for once 'key' was matched on Sorted List.
 *     If 'data' is set to NULL then this procedure ignores indicated data.
 *   \param [in] found -
 *     Pointer to boolean indicator \n
 *     \b As \b output - \n
 *     '*found' is loaded by TRUE only if match was found for both 'key' and 'data'.
 *     If 'data' is set to NULL then '*found' is loaded by TRUE if match was found
 *     for 'key' alone. See 'remarks' below.
 *   \param [in] iter -
 *     Pointer to UTILEX_SORTED_LIST_ITER \n
 *     \b As \b output - \n
 *     If 'data' is NOT NULL: \n
 *       If '*found' is TRUE, this procedure loads pointed memory by iterator pointing
 *        to the given 'data plus key' in the sorted list. 
 *       Otherwise, '*iter' is loaded by 'nul_ptr'.
 *        See sw_state_access[unit].dnx.shareddnx.sand.sorted_list.lists_array.list_data.null_ptr
 *     If 'data' is NULL: \n
 *       If '*found' is TRUE, this procedure loads pointed memory by iterator pointing
 *        to the given 'key' in the sorted list. 
 *       Otherwise, '*iter' is loaded by iterator pointingto the element just 'lower' (previous)
 *        to the given 'key' in the sorted list.
 * \par INDIRECT INPUT
 *   * See \b 'key' in DIRECT INPUT
 *   * See \b 'data' in DIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'found' on DIRECT INPUT
 *   * See \b 'iter' on DIRECT INPUT
 * \see
 *   * utilex_sorted_list_find_match_entry()
 */
shr_error_e utilex_sorted_list_entry_lookup(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint8 * found,
    UTILEX_SORTED_LIST_ITER * iter);

/**
 * \brief
 *   Lookup  the Sorted List for the given iterator (index identifier) and
 *   return the corresponding key and data (if pointer is not NULL)
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] iter -
 *     Iterator (index identifier) of an entry on sorted list. Must be smaller than
 *     list_size.
 *     See sw_state_access[unit].dnx.shareddnx.sand.sorted_list.lists_array.init_info.list_size
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     \b As \b output - \n
 *     This procedure loads '*key' by the key corresponding to 'iter' on Sorted List.
 *     If 'key' is set to NULL then this procedure ignores this input.
 *   \param [in] data -
 *     Pointer to UTILEX_SORTED_LIST_DATA (array of uint8s). \n
 *     \b As \b output - \n
 *     This procedure loads '*data' by the data corresponding to 'iter' on Sorted List.
 *     If 'data' is set to NULL then this procedure ignores this input.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'key' in DIRECT INPUT
 *   * See \b 'data' in DIRECT INPUT
 * \remarks
 *   If 'iter' does not point to a valid entry, DIRECT OUTPUT return value is _SHR_E_UNAVAIL
 * \see
 *   * utilex_sorted_list_find_higher_eq_key()
 */
shr_error_e utilex_sorted_list_entry_value(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data);

/**
 * \brief
 *   Get 'key plus data' of the entry just next (higher) to the one
 *   specified by '*iter'
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] iter -
 *     Pointer of type UTILEX_SORTED_LIST_ITER.
 *     Pointer to iterator (index identifier) of an entry on sorted list to get
 *     the 'next' of. If this iterator indicates 'end of list' then procedure
 *     returns without error and without loading 'key' or 'data'.
 *     See UTILEX_SORTED_LIST_ITER_END
 *     See sw_state_access[unit].dnx.shareddnx.sand.sorted_list.lists_array.init_info.list_size
 *     Note that, currently, end of list is indicated by 'list_size' plus '1'. See
 *     utilex_sorted_list_get_iter_begin_or_end().
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     \b As \b output - \n
 *     This procedure loads '*key' by the key corresponding to the 'next' (higher) 
 *     iterator on Sorted List. If 'iter' indicates 'end of list' then nothing is loaded.
 *     If 'key' is set to NULL then this procedure ignores this input.
 *   \param [in] data -
 *     Pointer to UTILEX_SORTED_LIST_DATA (array of uint8s). \n
 *     \b As \b output - \n
 *     This procedure loads '*data' by the data corresponding to the 'next' (higher) 
 *     iterator on Sorted List. If 'iter' indicates 'end of list' then nothing is loaded.
 *     If 'data' is set to NULL then this procedure ignores this input.
 * \par INDIRECT INPUT
 *   * See 'iter' on DIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'key' in DIRECT INPUT
 *   * See \b 'data' in DIRECT INPUT
 * \see
 *   * utilex_sorted_list_get_next_aux()
 */
shr_error_e utilex_sorted_list_get_next(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER * iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data);

/**
 * \brief
 *   Get 'key plus data' of the entry just previous (lower) to the one
 *   specified by '*iter'
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] iter -
 *     Pointer of type UTILEX_SORTED_LIST_ITER.
 *     Pointer to iterator (index identifier) of an entry on sorted list to get
 *     the 'prev' of. If this iterator indicates 'beginning of list' then procedure
 *     returns without error and without loading 'key' or 'data'.
 *     See UTILEX_SORTED_LIST_ITER_BEGIN
 *     See sw_state_access[unit].dnx.shareddnx.sand.sorted_list.lists_array.init_info.list_size
 *     Note that, currently, beginning of list is indicated by 'list_size'. See
 *     utilex_sorted_list_get_iter_begin_or_end().
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     \b As \b output - \n
 *     This procedure loads '*key' by the key corresponding to the 'prev' (lower) 
 *     iterator on Sorted List. If 'iter' indicates 'beginning of list' then nothing is loaded.
 *     If 'key' is set to NULL then this procedure ignores this input.
 *   \param [in] data -
 *     Pointer to UTILEX_SORTED_LIST_DATA (array of uint8s). \n
 *     \b As \b output - \n
 *     This procedure loads '*data' by the data corresponding to the 'prev' (lower) 
 *     iterator on Sorted List. If 'iter' indicates 'beginning of list' then nothing is loaded.
 *     If 'data' is set to NULL then this procedure ignores this input.
 * \par INDIRECT INPUT
 *   * See 'iter' on DIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'key' in DIRECT INPUT
 *   * See \b 'data' in DIRECT INPUT
 * \see
 *   * utilex_sorted_list_get_next_aux()
 */
shr_error_e utilex_sorted_list_get_prev(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER * iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data);

/**
 * \brief
 *   Returns the size, in bytes, of the memory buffer needed for loading
 *   the sorted list into. 'size' includes all info required for
 *   reconstruction of that Sorted List.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to get the size of. Sorted list is assumed
 *     to have been created.
 *   \param [in] size -
 *     Pointer to uint32. \n
 *     \b As \b output - \n
 *     This procedure loads '*size' by the number of bytes occupied by specified
 *     Sorted List
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'size' in DIRECT INPUT
 * \see
 *   * utilex_sorted_list_save()
 *   * utilex_occ_bm_get_size_for_save()
 */
shr_error_e utilex_sorted_list_get_size_for_save(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 * size);

/**
 * \brief
 *   Save specified Sorted List into specified memory buffer.
 *   Saving includes all info required for reconstruction of that Sorted List.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to get the size of. Sorted list is assumed
 *     to have been created.
 *   \param [in] buffer -
 *     Pointer to uint8. \n
 *     \b As \b output - \n
 *     This procedure loads '*buffer' by all info required for future construction
 *     of specified Sorted List
 *   \param [in] buffer_size_bytes -
 *     Number of bytes actually available on *buffer. Must be larger or equal to
 *     the size actually required.
 *   \param [in] actual_size_bytes -
 *     Pointer to uint32. \n
 *     \b As \b output - \n
 *     This procedure loads '*actual_size_bytes' by the number of bytes actually
 *     required for the save operation of specified Sorted List
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'buffer' in DIRECT INPUT
 *   * See \b 'actual_size_bytes' in DIRECT INPUT
 * \see
 *   * utilex_sorted_list_load()
 */
shr_error_e utilex_sorted_list_save(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint8 * buffer,
    uint32 buffer_size_bytes,
    uint32 * actual_size_bytes);

/**
 * \brief
 *   Load specified Sorted List from specified memory buffer.
 *   Loading includes all info required for reconstruction of that Sorted List.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] buffer -
 *     Pointer to pointer to a variable of type uint8. \n
 *     Caller loads pointed memory by the address of the buffer from which
 *     this procedure is to load all Sorted List info.
 *     The size of the buffer has to be at least the value returned
 *     by utilex_sorted_list_get_size_for_save().
 *     \b As \b output - \n
 *     This procedure loads '*buffer' by pointer into original input buffer
 *     at the location just beyond that which was used to load sorted list info.
 *     Just to clarify, this pointer may be used by this procedure to further
 *     get information for this buffer.
 *   \param [in] get_entry_fun -
 *     Procedure of type UTILEX_SORTED_LIST_SW_DB_ENTRY_GET.
 *     Procedure to get sorted list entry information from a local memory on the device.
 *   \param [in] set_entry_fun -
 *     Procedure of type UTILEX_SORTED_LIST_SW_DB_ENTRY_SET.
 *     Procedure to set sorted list entry information into a local memory on the device.
 *   \param [in] cmp_key_fun -
 *     Procedure of type UTILEX_SORTED_LIST_KEY_CMP.
 *     procedure to use to compare two keys (of elements on sorted list).
 *   \param [in] sorted_list_ptr -
 *     Pointer to UTILEX_SORTED_LIST_PTR
 *     \b As \b output - \n
 *     This procedure loads pointed memory by handle of the newly created Sorted List.
 * \par INDIRECT INPUT
 *   * See \b 'buffer' in DIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'buffer' in DIRECT INPUT
 *   * See \b 'sorted_list_ptr' in DIRECT INPUT
 * \see
 *   * utilex_sorted_list_save()
 *   * utilex_sorted_list_get_size_for_save()
 * \remarks
 *   The procedures, on direct input, are specified since their addresses can not be 
 *   stored on the 'save' operation due to the fact that addresses may change at the
 *   'load' platform compared to those on the 'save' platform.
 */
shr_error_e utilex_sorted_list_load(
    int unit,
    uint8 ** buffer,
    UTILEX_SORTED_LIST_SW_DB_ENTRY_GET get_entry_fun,
    UTILEX_SORTED_LIST_SW_DB_ENTRY_SET set_entry_fun,
    UTILEX_SORTED_LIST_KEY_CMP cmp_key_fun,
    UTILEX_SORTED_LIST_PTR * sorted_list_ptr);

/**
 * \brief
 *   Print contents of specified Sorted List - all valid entries
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to print the contents of. Sorted list is assumed
 *     to have been created.
 *   \param [in] table_header -
 *     NULL terminated array of chars to print as header of the printout.
 *   \param [in] print_key -
 *     Pointer to procedure of type UTILEX_SORTED_LIST_PRINT_VAL to be used
 *     for printing 'key' of each entry.
 *   \param [in] print_data -
 *     Pointer to procedure of type UTILEX_SORTED_LIST_PRINT_VAL to be used
 *     for printing 'data' of each entry.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * Printed contents of Sorted List
 * \see
 *   * utilex_print_list()
 */
shr_error_e utilex_sorted_list_print(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    char table_header[UTILEX_SORTED_LIST_HEADER_SIZE],
    UTILEX_SORTED_LIST_PRINT_VAL print_key,
    UTILEX_SORTED_LIST_PRINT_VAL print_data);
/**
 * \brief
 *   Print 'indices' array corresponding to specified sorted list.
 *   The 'indices' array is an array of pointers to the sorted list,
 *   which is ordered by value (key/data). (The sorted list itself
 *   is a linked list).
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to print the contents of its
 *     'indices' array. Sorted list is assumed to have been created.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see UTILEX_SORTED_LIST_ACCESS_DATA.indices)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * Printed contents of 'indices' array of Sorted List
 * \see
 *   * utilex_sorted_list_test_1()
 */
shr_error_e utilex_print_indices(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list);
/**
 * \brief
 *   Print contents of specified Sorted List - all valid entries
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to print the contents of. Sorted list is assumed
 *     to have been created.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * Printed contents of Sorted List
 * \see
 *   * utilex_sorted_list_print()
 *   * utilex_sorted_list_test_1()
 */
shr_error_e utilex_print_list(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list);

/**
 * \brief
 *   Print the 'info' structure of the specified sorted list.
 *   The 'info' structure contains 'meta data' regarding that
 *   Sorted List.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to print the 'info' of.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS_INFO)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * Printed contents of 'info' structure of Sorted List
 */
shr_error_e utilex_sorted_list_info_print(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list);

#ifdef UTILEX_DEBUG
/* { */
/**
 * \brief
 *   Clear the 'info' structure of the specified sorted list.
 *   The 'info' structure contains 'meta data' regarding that
 *   Sorted List.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to print the 'info' of.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS_INFO)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * Printed contents of 'info' structure of Sorted List
 */
shr_error_e utilex_sorted_list_info_clear(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list);

/*
 * 
 */
/**
 * \brief
 *   Fill sorted list and print its contents. Do various operations
 *   (remove, find, ...). Monitor results and print them.
 *   Debug exercise. Use for debug only to verify operation of package.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS_INFO)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * Executed exercises
 */
shr_error_e utilex_sorted_list_test_1(
    int unit);
/* } */
#endif

/**
 * \brief
 *   Get iterator related to either 'beginning' or 'end' of the
 *   specified sorted list.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to get indicated 'iterator' for.
 *   \param [in] get_begin -
 *     If non-zero then get iterator indicating the beginning of
 *     the list. Otherwise get iterator indicating end of list
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS_INFO)
 * \par DIRECT OUTPUT
 *   \retval Iterator at beginning or end fo list (as per 'get_begin'), if all is OK
 *   \retval -1 if error is encountered
 * \par INDIRECT OUTPUT
 *   * None
 * \see
 *   * UTILEX_SORTED_LIST_ITER_BEGIN
 *   * UTILEX_SORTED_LIST_ITER_END
 * \remarks
 *   Iterator values indicating 'begin' or 'end' are outside the range
 *   of valid indices (beyond list_size).
 */
uint32 utilex_sorted_list_get_iter_begin_or_end(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    int get_begin);

/*
 * Utilities related to the 'indices' feature.
 */
/* { */

/**
 * \brief
 *   Get location, on array of available linked list elements, of the first
 *   element with priority (key) higher than or equal to specified on input.
 *   (Only available when 'indices' feature is enabled on specified
 *   Sorted List). See 'remarks' below.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     Key to use as reference. Assumed to be an array of ARAD_TCAM_DB_LIST_KEY_SIZE
 *     characters. (Actually, ARAD_TCAM_DB_LIST_KEY_SIZE is assumed to be sizeof(uint32))
 *   \param [in] iter -
 *     Pointer to UTILEX_SORTED_LIST_ITER \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory by index (location), on array of
 *       available linked list elements for this linked list, of element with
 *       higher/eq priority.
 *       If input 'key' is strictly higher than highest element on sorted
 *       list (i.e. last valid element) then return: \n
 *       UTILEX_SORTED_LIST_ITER_END(unit,sorted_list) \n
 *       If sorted list is empty then return: \n
 *       UTILEX_SORTED_LIST_ITER_END(unit,sorted_list)
 *   \param [in] found_equal -
 *     Pointer to boolean indicator \n
 *     \b As \b output - \n
 *     If matching element was found and its priority was exactly equal
 *     to that of input 'key' then this procedure loads pointed memory
 *     by a non-zero value. Otherwise, it is loaded by zero.
 * \par INDIRECT INPUT
 *   * See \b 'key' in DIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'iter' on DIRECT INPUT
 *   * See \b 'found_equal' on DIRECT INPUT
 * \see
 *   * utilex_sorted_list_test_1()
 * \remarks
 *   Key is converted to priority using utilex_db_prio_list_priority_value_decode(). \n
 *   This procedure is only valid when 'indices' feature is enabled
 *   on specified sorted list.
 */
shr_error_e utilex_sorted_list_find_higher_eq_key(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_ITER * iter,
    uint8 * found_equal);

/**
 * \brief
 *   Get location, on array of available linked list elements, of the first
 *   element with priority (key) lower than or equal to specified on input.
 *   (Only available when 'indices' feature is enabled on specified
 *   Sorted List). See 'remarks' below.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     Key to use as reference. Assumed to be an array of ARAD_TCAM_DB_LIST_KEY_SIZE
 *     characters. (Actually, ARAD_TCAM_DB_LIST_KEY_SIZE is assumed to be sizeof(uint32))
 *   \param [in] iter -
 *     Pointer to UTILEX_SORTED_LIST_ITER \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory by index (location), on array of
 *       available linked list elements for this linked list, of element with
 *       lower/eq priority.
 *       If input 'key' is strictly lower than lowest element on sorted
 *       list (i.e. first valid element) then return: \n
 *       UTILEX_SORTED_LIST_ITER_BEGIN(unit,sorted_list) \n
 *       If sorted list is empty then return: \n
 *       UTILEX_SORTED_LIST_ITER_BEGIN(unit,sorted_list)
 *   \param [in] index_p -
 *     Pointer to uint32 \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by index, on array containing
 *     pointers (indices) of available linked list elements, arranged by
 *     priority. \n
 *     See 'iter' above. \n
 *     If there is no element on sorted list which matches required
 *     criterion (e.g./, input key is strictly lower than lowest
 *     element on sorted) then return '-1'. \n
 *     Note: In this latter case, '*iter' will be:
 *       UTILEX_SORTED_LIST_ITER_BEGIN((unit,sorted_list)
 *   \param [in] found_equal -
 *     Pointer to boolean indicator \n
 *     \b As \b output - \n
 *     If matching element was found and its priority was exactly equal
 *     to that of input 'key' then this procedure loads pointed memory
 *     by a non-zero value. Otherwise, it is loaded by zero.
 * \par INDIRECT INPUT
 *   * See \b 'key' in DIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'iter' on DIRECT INPUT
 *   * See \b 'index_p' on DIRECT INPUT
 *   * See \b 'found_equal' on DIRECT INPUT
 * \see
 *   * utilex_sorted_list_get_index_from_iter()
 *   * utilex_sorted_list_test_1()
 * \remarks
 *   * Key is converted to priority using utilex_db_prio_list_priority_value_decode().
 *   * This procedure is only valid when 'indices' feature is enabled
 *   on specified sorted list.
 *   * Key/priority comparison is done by a procedure as indicated on
 *     SORTED_LIST_ACCESS_INFO.cmp_func_type
 */
shr_error_e utilex_sorted_list_find_lower_eq_key(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_ITER * iter,
    uint32 * index_p,
    uint8 * found_equal);

/**
 * \brief
 *   Get current number of elements on specified sorted list. (This is
 *   also the number of elements on 'indices' array)
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] num_elements -
 *     Pointer to uint32 \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by the number of valid elements
 *     which are currently on the list. \n
 *     Only meaningful if the 'indices' feature is enabled ('indices'
 *     element in UTILEX_SORTED_LIST_T is allocated). 
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'num_elements' on DIRECT INPUT
 * \see
 *   * utilex_sorted_list_test_1()
 *   * utilex_sorted_list_find_higher_eq_key()
 */
shr_error_e utilex_sorted_list_get_num_elements(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 * num_elements);

/**
 * \brief
 *   Get indication on whether the 'indices' feature is enabled for this
 *   sorted list.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] is_enabled -
 *     Pointer to uint8 \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by a non-zero value if 'indices'
 *     feature is enabled. Otherwise, zero is loaded
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'is_enabled' on DIRECT INPUT
 * \see
 *   * utilex_sorted_list_test_1()
 */
shr_error_e utilex_sorted_list_is_indices_enabled(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint8 * is_enabled);

/**
 * \brief
 *   Given index within 'indices' (which is the ordinal location of
 *   a sorted linked list element, starting from low key value), get
 *   the corresponding location on array of available linked list
 *   elements.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] index_in_indices -
 *     Index on 'indices' array to use to get the corresponding index
 *     on array of available link-list elements. \n
 *     Only meaningful if the 'indices' feature is enabled ('indices'
 *     element in UTILEX_SORTED_LIST_T is allocated). \n
 *     Note that this index is counted from the BOTTOM of 'indices'.
 *     So, for example, when index=0 then returned entry refers to LOWEST key.
 *   \param [in] iter -
 *     Pointer to UTILEX_SORTED_LIST_ITER \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by index (location) of
 *     the entry corresponding to 'index_in_indices' on array of available
 *     linked list elements for this sorted linked list
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'iter' on DIRECT INPUT
 * \see
 *   * utilex_sorted_list_find_higher_eq_key()
 *   * utilex_sorted_list_find_lower_eq_key()
 */
shr_error_e utilex_sorted_list_get_iter_from_indices(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 index_in_indices,
    UTILEX_SORTED_LIST_ITER * iter);

/**
 * \brief
 *   Given index on array of available linked list elements for this
 *   linked list, get index within 'indices' (which is the ordinal
 *   location of a sorted linked list element, starting from low key
 *   value).
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to look up. Sorted list is assumed to have been
 *     created.
 *   \param [in] iter -
 *     Index (location) of the entry (to find a match for on 'indices')
 *     on array of available linked list elements for this sorted linked list.
 *   \param [in] index_in_indices -
 *     Pointer to uint32 \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by index, on 'indices' array, to
 *     corresponding to 'iter'. \n
 *     If no matching index is found, return '-1' \n
 *     Only meaningful if the 'indices' feature is enabled ('indices'
 *     element in UTILEX_SORTED_LIST_T is allocated).
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'iter' on DIRECT INPUT
 * \see
 *   * utilex_sorted_list_entry_add_by_iter()
 *   * utilex_sorted_list_entry_remove_by_iter()
 * \remarks:
 *   Use utilex_sorted_list_entry_value() to get 'key' or 'data'
 *   of corresponding linked list element.
 */
shr_error_e utilex_sorted_list_get_index_from_iter(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter,
    uint32 * index_in_indices);
/* } */
/* } FUNCTIONS */
/* } UTILEX_SORTED_LIST_H_INCLUDED*/
#endif
