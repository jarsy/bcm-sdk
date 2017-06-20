/* $Id: sand_hashtable.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/** \file utilex_sorted_list.c
 *
 * Definitions and prototypes for all common utilities related to hash table.
 *
 * Note that hash-table utilities use occupation-bitmap utilities
 * (utilex_occuation_bitmap.c,.h)
 */
#ifndef UTILEX_HASHTABLE_H_INCLUDED
/* { */
#define UTILEX_HASHTABLE_H_INCLUDED

/*************
* INCLUDES  *
*************/
/* { */
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_occupation_bitmap.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */
/**
 * \brief
 * Maximal number of hash table entries to assign at init.
 * See utilex_hash_table_init(), utilex_hash_table_create()
 */
#define MAX_NOF_HASHS_FOR_DNX   (200 * SOC_DNX_DEFS_GET(unit, nof_cores))
#define UTILEX_HASH_TABLE_NULL  UTILEX_U32_MAX
/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define UTILEX_HASH_TABLE_ACCESS          sw_state_access[unit].dnx.shareddnx.sand.hash_table
#define UTILEX_HASH_TABLE_ACCESS_DATA     UTILEX_HASH_TABLE_ACCESS.hashs_array.hash_data
#define UTILEX_HASH_TABLE_ACCESS_INFO     UTILEX_HASH_TABLE_ACCESS.hashs_array.init_info
/**
 * \brief Set hash table iterator to the beginning of the hash table
 */
#define UTILEX_HASH_TABLE_ITER_SET_BEGIN(iter) ((*iter) = 0)
/**
 * \brief Check whether hash table iterator has reached the end of the hash table
 */
#define UTILEX_HASH_TABLE_ITER_IS_END(iter)    ((*iter) == UTILEX_U32_MAX)
/**
 * \brief Verify specific hash table index is marked as 'occupied'. If not, software goes to
 * exit with error code.
 * \par DIRECT INPUT:
 *   \param [in] _unit -
 *     Identifier of unit to handle
 *   \param [in] _hash_table_index -
 *     Index identifier of hash table (as derived from hash table handle).
 * \par INDIRECT INPUT:
 *   SWSTATE system
 * \par DIRECT OUTPUT:
 *   * None
 * \par INDIRECT OUTPUT:
 *   In case of error condition, goto exit with _SHR_E_MEMORY
 * \remark
 *   * 'exit' is assumed to be defined in the caller's scope.
 *   * SHR_FUNC_INIT_VARS(unit) is supposed to have been invoked at the beginning of the procedure
 */
#define UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(_unit,_hash_table_index) \
  { \
    uint8 bit_val ; \
    uint32 max_nof_hashs ; \
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.max_nof_hashs.get(_unit, &max_nof_hashs)) ; \
    if (_hash_table_index >= max_nof_hashs) \
    { \
      /* \
       * If hashtable handle is out of range then quit with error. \
       */ \
      bit_val = 0 ; \
    } \
    else \
    { \
      SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.occupied_hashs.bit_get(_unit, (int)_hash_table_index, &bit_val)) ; \
    } \
    if (bit_val == 0) \
    { \
      /* \
       * If hashtable structure is not indicated as 'occupied' then quit \
       * with error. \
       */ \
      SHR_SET_CURRENT_ERR(_SHR_E_MEMORY) ; \
      SHR_EXIT() ; \
    } \
  }
/**
 * \brief Verify specified unit has a legal value. If not, software goes to
 * exit with error code.
 * \par DIRECT INPUT:
 *   \param [in] _unit -
 *     Identifier of unit to handle
 * \par INDIRECT INPUT:
 *   SOC_MAX_NUM_DEVICES
 *   \ref shr_error_e
 * \par DIRECT OUTPUT:
 *   * None
 * \par INDIRECT OUTPUT:
 *   In case of error condition, goto exit with _SHR_E_PARAM
 * \remark
 *   * 'exit' is assumed to be defined in the caller's scope.
 *   * SHR_FUNC_INIT_VARS(unit) is supposed to have been invoked at the beginning of the procedure
 */
#define UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(_unit) \
  if ((_unit < 0) || (_unit >= SOC_MAX_NUM_DEVICES)) \
  { \
    /* \
     * If this is an illegal unit identifier, quit \
     * with error. \
     */ \
    SHR_SET_CURRENT_ERR(_SHR_E_PARAM) ; \
    SHR_EXIT() ; \
  }
/**
 * \brief Convert input hash table handle to index in 'occupied_hashs' array.
 * \par DIRECT INPUT:
 *   \param [in] _hash_table_index -
 *     Variable to contain index identifier of hash table
 *   \param [in] _handle -
 *     Handle, of hash table, to convert to index and load into _hash_table_index
 * \par INDIRECT INPUT:
 *   \ref shr_error_e
 * \par DIRECT OUTPUT:
 *   * None
 * \par INDIRECT OUTPUT:
 *   See _hash_table_index above
 * \remark
 *   * Indices go from 0 -> (occupied_hashs - 1)
 *   * Handles go from 1 -> occupied_hashs
 */
#define UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(_hash_table_index,_handle) (_hash_table_index = _handle - 1)
/**
 * \brief Convert input index in 'occupied_hashs' array to hash table handle.
 * \par DIRECT INPUT:
 *   \param [in] _handle -
 *     Variable to contain handle of hash table
 *   \param [in] _hash_table_index -
 *     Index identifier, of hash table, to convert to handle and load into _handle
 * \par INDIRECT INPUT:
 *   \ref shr_error_e
 * \par DIRECT OUTPUT:
 *   * None
 * \par INDIRECT OUTPUT:
 *   See _handle above
 * \remark
 *   * Indices go from 0 -> (occupied_hashs - 1)
 *   * Handles go from 1 -> occupied_hashs
 */
#define UTILEX_HASH_TABLE_CONVERT_HASHTABLE_INDEX_TO_HANDLE(_handle,_hash_table_index) (_handle = _hash_table_index + 1)
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
/**
 * \brief Type of the hash table key
 *
 */
typedef uint8 UTILEX_HASH_TABLE_KEY;
/**
 * \brief Type of the hash table data
 */
typedef uint8 *UTILEX_HASH_TABLE_DATA;
/*
 * the ADT of hash table, use this type to manipulate the hash table. 
 * just a workaround to allow cyclic reference from hash func to hash info and vice versa
 */
/* typedef struct UTILEX_HASH_TABLE_INFO_T *UTILEX_HASH_TABLE_PTR_WORKAROUND ; */
/**
 * \brief Iterator over the hash table, use this type to traverse the hash table.
 */
typedef uint32 UTILEX_HASH_TABLE_ITER;
/**
 * \brief This is an identifier of an element of type UTILEX_HASH_TABLE_INFO
 *
 * Replace: typedef UTILEX_HASH_TABLE_INFO* UTILEX_HASH_TABLE_PTR;
 * because the new software state does not use pointers, only handles.
 * So now, UTILEX_HASH_TABLE_PTR is just a handle to the 'hash table'
 * structure (actually, index into 'hashs_array' {of pointers})
 *
 * Note that the name is kept as is to minimize changes in current code.
 */
typedef uint32 UTILEX_HASH_TABLE_PTR;
/**
 * \brief
 *   Hash functions used to map from key to hash value (entry in the hash table)
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in]  hash_table -
 *     The hash table, needed so the hash function can get
 *     the properties of the hash table to consider in the
 *     calculations
 *   \param [in]  key -
 *     Pointer to array of bytes containing key to hash
 *   \param [in]  seed -
 *     value to use in the hash calculation
 *   \param [in]  hash_val -
 *     Pointer to load output. \n
 *     \b As \b output -
 *       This procedure loads pointed memory by the
 *       hash value calculated from input 'key'.
 * \par INDIRECT INPUT
 *   * None
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *hash_val -\n
 *     See DIRECT INPUT above.
 * \remark
 *   None.
 */
typedef uint32(
    *UTILEX_HASH_MAP_HASH_FUN_CALL_BACK) (
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 seed,
    uint32 * hash_val);
/**
 * \brief
 *   Call back to load entry information into a local SW DB on the device.
 * \par DIRECT INPUT
 *   \param [in] prime_handle -
 *     Handle of the hash table to identify hash table instance
 *     Currently, not used by this code.
 *   \param [in] sec_handle -
 *     Secondary identifier of hash table instance.
 *     Currently, not used by this code.
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
    *UTILEX_HASH_MAP_SW_DB_ENTRY_SET) (
    int prime_handle,
    uint32 sec_handle,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * data);
/**
 * \brief
 *   Call back to get entry information from a local SW DB on the device.
 * \par DIRECT INPUT
 *   \param [in] prime_handle -
 *     Handle of the hash table to identify hash table instance
 *     Currently, not used by this code.
 *   \param [in] sec_handle -
 *     Secondary identifier of hash table instance.
 *     Currently, not used by this code.
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
 *   None.
 */
typedef uint32(
    *UTILEX_HASH_MAP_SW_DB_ENTRY_GET) (
    int prime_handle,
    uint32 sec_handle,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * const data);

/**
 * \brief
 *   Includes the information user has to supply for hash table creation
 * \see
 *   * \ref UTILEX_HASH_TABLE_INFO
 *   * \ref utilex_hash_table_init
 *   * \ref utilex_hash_table_create
 */
typedef struct
{
    /**
     * Handle of the hash table to identify the hash table instance
     */
    int prime_handle;
    /**
     * Handle of the hash table to identify the hash table instance
     */
    uint32 sec_handle;
    /**
     * Size of the hash table, in elements (maximal number of elements to insert the hash table)
     */
    uint32 table_size;
    /**
     * Number of unique results from the hash function.
     */
    uint32 table_width;
    /**
     * Size of the key (in bytes)
     */
    uint32 key_size;
    /**
     * Size of the data (in bytes)
     */
    uint32 data_size;
    /**
     * Hash functions used to map from key to hash value (number)
     * Set to NULL (or don't touch after clear) to use the default hash function.
     */
    UTILEX_HASH_MAP_HASH_FUN_CALL_BACK hash_function;
    /**
     * Hash function to use in case the hash function returns entry in use.
     * Set to NULL (or don't touch after clear) to use the default rehash function.
     */
    UTILEX_HASH_MAP_HASH_FUN_CALL_BACK rehash_function;
    /**
     * Procedure to read directly from memory image of hash table into
     * local entry image.
     * Not used on this code, for now.
     */
    UTILEX_HASH_MAP_SW_DB_ENTRY_GET get_entry_fun;
    /**
     * Procedure to write directly into memory image of hash table from
     * local entry image.
     * Not used on this code, for now.
     */
    UTILEX_HASH_MAP_SW_DB_ENTRY_SET set_entry_fun;
} UTILEX_HASH_TABLE_INIT_INFO;
/**
 * \brief
 * Structure at the root of each hash table, containg the table (lists_head),
 * the keys (keys) and 'next' elements for each lists_head entry which contains
 * more than one key (more than one hit).
 * All pointers refer to place on OCC (occupation bit map) table.
 * \see
 *   * \ref utilex_hash_table_entry_add()
 */
typedef struct
{
    /**
     * Array to include hash table entries. Index of this array is
     * the HASH VALUE of a specific key. Value is the place (index) on corresponding
     * OCC table (bit map).
     */
    SW_STATE_BUFF *lists_head;
    /**
     * Array of full keys. Index is 'place on OCC' (OCC stands for Occupation Bit Map)
     */
    SW_STATE_BUFF *keys;
    /**
     * Array of 'pointer's to the next in the list.
     * Index is 'place on OCC' (OCC stands for Occupation Bit Map)
     * Value (pointer) is also 'place on OCC' (of the 'next' match to
     * the original hash value).
     */
    SW_STATE_BUFF *next;
    /**
     * The size of one pointer in bytes.
     */
    uint32 ptr_size;
    /**
     * Mapping of hash table memory, for efficient manipulation.
     * Each hash table entry is represented by one bit.
     */
    UTILEX_OCC_BM_PTR memory_use;
    /**
     * Value assigned to a 'null pointer' (empty entry).
     */
    uint32 null_ptr;
    /**
     * Tmp buffers for copying of 'pointers' so they can be easily
     * tranformed into uint32.
     */
    SW_STATE_BUFF *tmp_buf;
    SW_STATE_BUFF *tmp_buf2;

} UTILEX_HASH_TABLE_T;

/**
 * \brief
 *   Structure type for containing all info and data corresponding
 *   to one hash table.
 * \see
 *   \b utilex_sw_state_hash_table_t
 */
typedef struct
{
    UTILEX_HASH_TABLE_INIT_INFO init_info;
    UTILEX_HASH_TABLE_T hash_data;
} UTILEX_HASH_TABLE_INFO;

/* } */

/**
 * \brief
 * Control Structure for all created hash tables. Each hash is pointed
 * by hashs_array. See \ref utilex_hash_table_init()
 */
typedef struct
{
    /**
     * \brief Maximal number of hash tables that may be opened simultaneously.
     */
    uint32 max_nof_hashs;
    /**
     * \brief Number of hash tables that are currently open.
     */
    uint32 in_use;
    /**
     * \brief Array of pointers, each pointing to one hash table.
     *   On reset, they are all NULL
     */
    PARSER_HINT_ARR_PTR UTILEX_HASH_TABLE_INFO **hashs_array;
    /**
     * \brief Bitmap indicating which hash table (on hashs_array[])
     *   is currently open
     */
    PARSER_HINT_ARR SHR_BITDCL *occupied_hashs;
} utilex_sw_state_hash_table_t;

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
 * \brief Get handle to hash table which will be considered illegal
 * by all hash table utilities.
 * Legal values for 'handle' are 1 -> MAX_NOF_HASHS_FOR_DNX or, to be more precise:
 * 1 -> max_nof_hashs (The value of UTILEX_HASH_TABLE_ACCESS.max_nof_hashs.get(unit, &max_nof_hashs))
 */
uint32 utilex_hash_table_get_illegal_hashtable_handle(
    void);

/**
 * \brief
 *   Initialize control structure for ALL hash table instances expected.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] max_nof_hashs -
 *     Maximal number of hash tables which can be sustained simultaneously.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Initialized sw_state_access[unit].dnx.shareddnx.sand.hash_table: \n
 *   * Allocated array of pointers: 'hashs_array'
 *   * Allocated array of bits: 'occupied_hashs'
 *   * ...
 */
shr_error_e utilex_hash_table_init(
    int unit,
    uint32 max_nof_hashs);
/**
 * \brief
 *   Fill all allocated 'tmp' (sand box) buffers by zeros.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf
 *   and UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf2)
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
 *   * utilex_hash_table_find_entry()
 */
shr_error_e utilex_hash_table_clear_all_tmps(
    int unit);
/**
 * \brief
 *   Creates a new Hash table instance.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Pointer to memory to load output into. \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory by handle of newly created
 *       hash table
 *   \param [in] init_info -
 *     Pointed memory contains setup parameters required for for the
 *     creation of the hash table. See \ref UTILEX_HASH_TABLE_INIT_INFO
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see \ref UTILEX_HASH_TABLE_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *hash_table \n
 *     See 'hash_table' in DIRECT INPUT above
 * \see
 *   * utilex_hash_table_init()
 */
shr_error_e utilex_hash_table_create(
    int unit,
    UTILEX_HASH_TABLE_PTR * hash_table,
    UTILEX_HASH_TABLE_INIT_INFO init_info);
/**
 * \brief
 *   Free the specified hash table instance.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the Hash table to destroy.
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see \ref UTILEX_HASH_TABLE_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   All resources of specified hash table are freed. Specifically,
 *   Root pointer at UTILEX_HASH_TABLE_ACCESS.hashs_array is cleared.
 * \see
 *   * utilex_hash_table_create()
 */
shr_error_e utilex_hash_table_destroy(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table);
/**
 * \brief
 *   Insert an entry into the hash table, if entry already exist then
 *   the operation succeeds and returned values refers to the entry
 *   which already exists.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the Hash table to add a key to.
 *   \param [in] key -
 *     The key to add into the hash table.
 *   \param [in] data_indx -
 *     Pointer to memory to load output into. \n
 *     \b As \b output - \n
 *       Pointed memory is to be loaded by index (identifier) of the newly added   \n
 *       key in the hash table. This is, essentially, the data carried by the key  \n
 *       If key already exists, pointed memory is loaded by its index (identifier) \n
 *       If '*success' is loaded by '0' then *data_indx is set to UTILEX_HASH_TABLE_NULL \n
 *   \param [in] success -
 *     Pointer to memory to load output into. \n
 *     \b As \b output - \n
 *       Pointed memory is to be loaded by              \n
 *       * TRUE if operation was completed to ssuccess (new key was added or aleady existing key was found)  \n
 *       * FALSE if operation has failed
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see \ref UTILEX_HASH_TABLE_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *data_indx -\n
 *     See 'data_indx' in DIRECT INPUT above.
 *   \b *success -\n
 *     See 'success' in DIRECT INPUT above.
 * \see
 *   * utilex_hash_table_entry_add_at_index()
 *   * utilex_hash_table_entry_remove()
 */
shr_error_e utilex_hash_table_entry_add(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 * data_indx,
    uint8 * success);
/**
 * \brief
 *   Insert an entry into the hash table at specified index, entry
 *   must either already exist (with the same key) or specified index must
 *   be free.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the Hash table to add a key to.
 *   \param [in] key -
 *     The key to add into the hash table.
 *   \param [in] data_indx -
 *     If key is not in the table, this is the index (identifier)
 *     of the newly added entry. Otherwise, it must match index of
 *     existing  key.
 *   \param [in] success -
 *     Pointer to memory to load output into. \n
 *     \b As \b output - \n
 *       Pointed memory is to be loaded by              \n
 *       * TRUE if operation was completed to ssuccess (new key was added or aleady existing key was found)  \n
 *       * FALSE if operation has failed
 * \par INDIRECT INPUT
 *   SWSTATE system (Specifically, see \ref UTILEX_HASH_TABLE_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *success -\n
 *     See 'success' in DIRECT INPUT above
 * \see
 *   * utilex_hash_table_entry_add_at_index()
 *   * utilex_hash_table_entry_remove()
 */
shr_error_e utilex_hash_table_entry_add_at_index(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 data_indx,
    uint8 * success);
/**
 * \brief
 *   Remove an entry (one key) from a hash table, if the key does
 *   not exist then the operation has no effect (No error is
 *   reported).
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to remove entry from.
 *   \param [in] key -
 *     Pointer to an array of uint8s containing the key to remove.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Updated hash table (on SWSTATE memory):
 *     * UTILEX_HASH_TABLE_ACCESS_DATA.lists_head
 *     * UTILEX_HASH_TABLE_ACCESS_DATA.next
 *     * Updated Occupation Bitmap memory.
 */
shr_error_e utilex_hash_table_entry_remove(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key);
/**
 * \brief
 *   Remove an entry from a hash table given the index identifying
 *   this entry. if there is no entry on specified index then
 *   the operation has no effect (No error is reported).
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to remove entry from.
 *   \param [in] data_indx -
 *     Index of the entry to remove. (Index is on OCC bit map)
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Updated hash table (on SWSTATE memory):
 *     * UTILEX_HASH_TABLE_ACCESS_DATA.lists_head
 *     * UTILEX_HASH_TABLE_ACCESS_DATA.next
 *     * Updated Occupation Bitmap memory.
 */
shr_error_e utilex_hash_table_entry_remove_by_index(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 data_indx);
/**
 * \brief
 *   Lookup the hash table for the given key and return the index
 *   (unique) identifier of the given key.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to look for the key in.
 *   \param [in] key -
 *     Pointer to an array of uint8s containing the key to look for.
 *   \param [in] data_indx -
 *     Poniter to memory to be loaded by this procedure by the unique
 *     index id associated with the given key, valid only if
 *     'found' is true.
 *   \param [in] found -
 *     Pointer to memory to be loaded by this procedure by indication
 *     on whether the key was found in the hash table (TRUE) or not (FALSE)
 * \par INDIRECT INPUT
 *   SWSTATE system (For example, UTILEX_HASH_TABLE_ACCESS_DATA.lists_head,
 *   UTILEX_HASH_TABLE_ACCESS_DATA.next, etc.)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *data (See 'data' on DIRECT INPUT above).    \n
 *   \b *found (See 'found' on DIRECT INPUT above).  \n
 */
shr_error_e utilex_hash_table_entry_lookup(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 * data_indx,
    uint8 * found);

/**
 * \brief
 *   Get a key from a hash table given the index identifying
 *   this entry.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to remove entry from.
 *   \param [in] data_indx -
 *     Index of the key (entry) to find and retrieve. (Index is on OCC bit map)
 *   \param [in] key -
 *     Pointer to an array of uint8s to be loaded, by this procedure, by
 *     the key (in case it has been found).
 *   \param [in] found -
 *     Pointer to memory to be loaded by this procedure by indication
 *     on whether a key was found on 'data_indx' in the hash table
 *     (TRUE) or not (FALSE)
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *found (See 'data' on DIRECT INPUT above).
 *   \b *key (See 'key' on DIRECT INPUT above).
 */
shr_error_e utilex_hash_table_get_by_index(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 data_indx,
    UTILEX_HASH_TABLE_KEY * key,
    uint8 * found);

/**
 * \brief
 *   Clear the indicated Hash table's contents without releasing the memory.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to clear contents of.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Cleared memory on SWSTATE buffer: Both directly on memory assigned
 *   to hash table and on corresponding occupation bit map.
 * \remarks
 *   The cleared memory (of hash table) is accessed by:     \n
 *   sw_state_access[unit].dnx.shareddnx.sand.hash_table    \n
 *   The Occupation bit map is cleared using: utilex_occ_bm_clear().
 */
shr_error_e utilex_hash_table_clear(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table);

/**
 * \brief
 *   Get the next valid entry (key and data) in the hash table.
 *   Start traversing from the place pointed by the given iterator.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to search.
 *   \param [in] iter -
 *     Pointer to index into the table: 'interator'. \n
 *     \b As \b input: \n
 *         Iterator points to the entry to start traverse from. \n
 *     \b As \b output: \n
 *         This is the index into the table which is the one following
 *         the valid entry (valid key) which was found. If no valid
 *         entry was found, *iterator is loaded by UTILEX_U32_MAX.
 *         Note that *iter may be loaded by 'table_size' (namely, one
 *         index following the last on the hash table!).
 *         See \ref UTILEX_HASH_TABLE_INIT_INFO
 *   \param [in] key -
 *     Pointer. This procedure loads pointed memory by the 'key' found
 *     on the 'next' valid entry.That is, this is the key at entry
 *     whose index is *data_indx. This output is NOT valid if *iter is
 *     loaded by UTILEX_U32_MAX. \n
 *     Note:  The number of bytes in a key is 'key_size. See \ref UTILEX_HASH_TABLE_INIT_INFO
 *   \param [in] data_indx -
 *     Pointer. This procedure loads pointed memory by the 'index' found
 *     on the 'next' valid entry. That is, this is the index at entry
 *     whose key is *key. This output is NOT valid if *iter is
 *     loaded by UTILEX_U32_MAX
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *iter - \n
 *     See 'iter' on DIRECT INPUT
 *   \b *key - \n
 *     See 'key' on DIRECT INPUT
 *   \b *data_indx - \n
 *     See 'data_indx' on DIRECT INPUT
 * \remarks
 */
shr_error_e utilex_hash_table_get_next(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_ITER * iter,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 * data_indx);

/**
 * \brief
 *   Returns the size of the buffer needed to return the hash table
 *   as buffer. This buffer contain all information required to
 *   reconstruct the full hash table. This capability is used
 *   for storing hash table in external memory and for retrieving it.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to check.
 *   \param [in] size -
 *     Pointer. This procedure loads pointed memory by the size of
 *     memory buffer required to store all info corresponding tp
 *     specified hash table.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *size - \n
 *     See 'size' on DIRECT INPUT
 * \see
 *    utilex_hash_table_save()
 */
shr_error_e utilex_hash_table_get_size_for_save(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 * size);

/**
 * \brief
 *   Saves the given hash table in the given buffer
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to save.
 *   \param [in] buffer -
 *     Pointer to output buffer. \n 
 *     \b As \b output - \n
 *       This procedure loads pointed memory by the all info
 *       corresponding to specified hash table.
 *   \param [in] buffer_size_bytes -
 *     Maximal number of bytes the caller assigns for the saving.
 *     This procedure ASSUMES that 'buffer_size_bytes' is larger than
 *     the actual number of bytes required for the saving (which is loaded
 *     into '*actual_size_bytes').
 *   \param [in] actual_size_bytes -
 *     Pointer to be loaded by this procedure. \n
 *     \b As \b output - \n
 *       This procedure loads pointed memory by the actual number of bytes
 *       that were actually used for the saving. Make sure to verify it
 *       is smaller than 'buffer_size_bytes'.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * \b *buffer (See DIRECT INPUT above)
 *   * \b *actual_size_bytes (See DIRECT INPUT above)
 * \see
 *    utilex_hash_table_get_size_for_save()
 */
shr_error_e utilex_hash_table_save(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint8 * buffer,
    uint32 buffer_size_bytes,
    uint32 * actual_size_bytes);

/**
 * \brief
 *   Load from the given buffer, assumed to have been used as storage
*     area for utilex_hash_table_save(), into a newly created hash table.
 *    Newly cteated hash table is on SWSTATE memory.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] buffer -
 *     Pointer to an array of pointers to memory buffers (each is an
 *     array of uit8s. Note that only the first element (of array
 *     of pointers) is used! Note that the buffer pointed by the
 *     first element must be large enough as indicated by
 *     utilex_hash_table_get_size_for_save()\n
 *     \b As \b output - \n
 *       First array element is loaded by the pointer to the next free byte
 *       on the original input buffer.
 *   \param [in] set_entry_fun -
 *     Pointer to function to use to load entry information into a local
 *     SW DB on the device. NOT USED by this procedure. MUST be NULL.
 *   \param [in] get_entry_fun -
 *     Pointer to function to use to get entry information from a local
 *     SW DB on the device. NOT USED by this procedure. MUST be NULL.
 *   \param [in] hash_function -
 *     Pointer to hash function to use on created hash table if default
 *     hash function is not used. \n
 *     NOT USED by this procedure. IGNORED.
 *   \param [in] rehash_function -
 *     Pointer to rehash function to use on created hash table if default
 *     hash function is not used. \n
 *     NOT USED by this procedure. IGNORED.
 *   \param [in] hash_table_ptr -
 *     Pointer to be loaded by output. \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by the handle to the newly created
 *     hash table.
 * \par INDIRECT INPUT
 *   * SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * \b *hash_table_ptr (See DIRECT INPUT above)
 *   * \b *buffer (See DIRECT INPUT above)
 * \see
 *    utilex_hash_table_get_size_for_save()
 */
shr_error_e utilex_hash_table_load(
    int unit,
    uint8 ** buffer,
    UTILEX_HASH_MAP_SW_DB_ENTRY_SET set_entry_fun,
    UTILEX_HASH_MAP_SW_DB_ENTRY_GET get_entry_fun,
    UTILEX_HASH_MAP_HASH_FUN_CALL_BACK hash_function,
    UTILEX_HASH_MAP_HASH_FUN_CALL_BACK rehash_function,
    UTILEX_HASH_TABLE_PTR * hash_table_ptr);

/**
 * \brief
 *   Clear the 'info' section of a hash table. Used as a necessary
 *   step in the creation of a new table.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to remove entry from.
 * \par INDIRECT INPUT
 *   SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Erased entry (corresponding to hash_table) in
 *   sw_state_access[unit].dnx.shareddnx.sand.hash_table.hashs_array.init_info[]
 */
shr_error_e utilex_hash_table_info_clear(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table);

#ifdef UTILEX_DEBUG
/* { */
/**
 * \brief
 *   Get the number of active entries on specified hash table.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to get num entries on.
 *   \param [in] num_active_ptr -
 *     Pointer to be loaded by output.\n
 *     \b As \b Output - \n
 *       This procedure loads pointed memory by the number of active
 *       entries on specified hash table.
 * \par INDIRECT INPUT
 *   SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *num_active_ptr - See 'num_active_ptr' in DIRECT INPUT above.
 */
shr_error_e utilex_hash_table_get_num_active(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 * num_active_ptr);

/**
 * \brief
 *   Prints the hash table contents, all active entries, NOT including
 *   empty entries.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to print.
 * \par INDIRECT INPUT
 *   SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Printed hash table.
 * \remarks
 *   LOG_CLI is used for the printing.
 */
shr_error_e utilex_hash_table_print(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table);

/**
 * \brief
 *   Prints the control (info) data corresponding to specified hash table.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to print.
 * \par INDIRECT INPUT
 *   SWSTATE system (UTILEX_HASH_TABLE_ACCESS_INFO).
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Printed 'info' structure. Only relevant entries are printed
 *   (and not, say, elements which are not used by current code).
 * \remarks
 *   LOG_CLI is used for the printing.
 */
shr_error_e utilex_hash_table_info_print(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table);

/**
 * \brief
 *   Test hash table, Create, fill, verify number of active, 
 *   verify actions that should fail, delete, etc.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 * \par INDIRECT INPUT
 *   SWSTATE system corresponding to hash tables (UTILEX_HASH_TABLE_ACCESS_DATA,
 *   UTILEX_HASH_TABLE_ACCESS_INFO)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Printed hash table.
 * \remarks
 *   * LOG_CLI is used for the printing.
 *   * In the end, newly created hash table is destroyed.
 */
shr_error_e utilex_hash_table_test_1(
    int unit);
/* } */
#endif /* UTILEX_DEBUG */

/* } */

/* } UTILEX_HASHTABLE_H_INCLUDED*/
#endif
