/* $Id: sand_hashtable.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/** \file utilex_hashtable.c
 *
 * All common utilities related to hash table.
 * Only for use by 'DNX' code!!! (This is due to the fact that this
 * utility uses a part of SWSTATE which is assigned to DNX only.)
 *
 * Note:
 * Since some compilers do not accept empty files ("ISO C forbids an empty source file")
 * then dummy code ia added for non-DNX compilation.
 *
 * Note that hash-table utilities use occupation-bitmap utilities
 * (utilex_occuation_bitmap.c,.h)
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDSWDNX_HASHDNX

/*************
* INCLUDES  *
*************/
/* { */

#include <shared/bsl.h>
#include <sal/core/libc.h>
/*
 * SW is included
 */
#include <shared/swstate/access/sw_state_access.h>
#include <shared/shrextend/shrextend_debug.h>

#include <shared/utilex/utilex_hashtable.h>
#include <shared/utilex/utilex_integer_arithmetic.h>

/* } */

/*************
* DEFINES   *
*************/
/* { */

/* } */

/*************
*  MACROS   *
*************/
/* { */

/* } */

/*************
* TYPE DEFS *
*************/
/* { */

 /*
  * } 
  */

 /*************
 * GLOBALS   *
 *************/
 /*
  * { 
  */

extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];
 /*
  * } 
  */
#ifdef BCM_DNX_SUPPORT
/* { */
 /*************
 * FUNCTIONS *
 *************/
 /*
  * { 
  */
shr_error_e
utilex_hash_table_info_clear(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table)
{
    UTILEX_HASH_TABLE_INIT_INFO info;
    uint32 hash_table_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    sal_memset((void *) &info, 0x0, (uint32) sizeof(UTILEX_HASH_TABLE_INIT_INFO));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.set(unit, hash_table_index, &info));
exit:
    SHR_FUNC_EXIT;
}

/************************************************************************/
/*  Internal functions                                                  */
/* {                                                                    */
/************************************************************************/

/**
 * \brief
 *   Get value of 'tmp_buf' pointer (See UTILEX_HASH_TABLE_T)
 *   from handle.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to get info on
 *   \param [in] tmp_buf_ptr_ptr -
 *     Pointer. This procedure loads pointed memory by the pointer
*      to the 'tmp_buf' internal workspace buffer (within SWSTATE memory).
 * \par INDIRECT INPUT
 *   SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *tmp_buf_ptr_ptr -\n
 *     See 'tmp_buf_ptr_ptr' in DIRECT INPUT above
 * \remark
 *   This procedure is exceptional. It is added here so we can use
 *   the buffer pointed by 'tmp_buf' as a work space whose address
 *   is passed to various utilities
 */
static shr_error_e
utilex_hash_table_get_tmp_data_ptr_from_handle(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint8 ** tmp_buf_ptr_ptr)
{
    uint32 hash_table_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    *tmp_buf_ptr_ptr = sw_state[unit]->dnx.shareddnx.sand.hash_table->hashs_array[hash_table_index]->hash_data.tmp_buf;
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *   Get value of 'tmp_buf2' pointer (See UTILEX_HASH_TABLE_T)
 *   from handle.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to get info on.
 *   \param [in] tmp_buf2_ptr_ptr -
 *     Pointer. This procedure loads pointed memory by the pointer
*      to the 'tmp_buf2' internal workspace buffer (within SWSTATE memory).
 * \par INDIRECT INPUT
 *   SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *tmp_buf2_ptr_ptr -\n
 *     See 'tmp_buf2_ptr_ptr' in DIRECT INPUT above
 * \remark
 *   This procedure is exceptional. It is added here so we can use
 *   the buffer pointed by 'tmp_buf2' as a work space whose address
 *   is passed to various utilities
 */
static shr_error_e
utilex_hash_table_get_tmp2_data_ptr_from_handle(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint8 ** tmp_buf2_ptr_ptr)
{
    uint32 hash_table_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    *tmp_buf2_ptr_ptr =
        sw_state[unit]->dnx.shareddnx.sand.hash_table->hashs_array[hash_table_index]->hash_data.tmp_buf2;
exit:
    SHR_FUNC_EXIT;
}
uint32
utilex_hash_table_get_illegal_hashtable_handle(
    void)
{
    return ((uint32) (-1));
}
#if (0)
/* { */
/*
 * The following two procedures are not used within this code but are left here
 * as demonstration of the types:
 *   UTILEX_HASH_MAP_SW_DB_ENTRY_GET
 *   UTILEX_HASH_MAP_SW_DB_ENTRY_SET
 */
uint32
utilex_hashtable_default_set_entry_fun(
    int prime_handle,
    uint32 sec_handle,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * data)
{
    sal_memcpy(buffer + (offset * len), data, len);
    return _SHR_E_NONE;
}

uint32
utilex_hashtable_default_get_entry_fun(
    int prime_handle,
    uint32 sec_handle,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * data)
{
    sal_memcpy(data, buffer + (offset * len), len);
    return _SHR_E_NONE;
}
/**
 * \brief
 *   Get element 'table_size' for hash table. This is the number
 *   of elements (members) assigned to this hash table.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to get info on.
 *   \param [in] table_size_ptr -
 *     Pointer. This procedure loads pointed memory by the info
 *     element 'table_size' (from within SWSTATE memory).
 * \par INDIRECT INPUT
 *   SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *table_size_ptr -\n
 *     See 'table_size_ptr' in DIRECT INPUT above
 * \remark
 *   None
 */
static shr_error_e
utilex_hash_table_get_table_size(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 * table_size_ptr)
{
    uint32 hash_table_index, table_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    *table_size_ptr = table_size;
exit:
    SHR_FUNC_EXIT;
}
/*********************************************************************
* NAME:
*     utilex_hash_table_simple_rehash
* FUNCTION:
*    Hash functions maps from key to hash value.
*    Simple and default implementation of a rehash function.
*    Returns the next entry in the hash table.
* INPUT:
*  int                            unit -
*   Identifier of the device to access.
*  UTILEX_HASH_TABLE_PTR        hash_table -
*   Handle to the hash table, needed so the hash function
*   can get the properties of the hash table to consider in the
*   calculations
*  UTILEX_HASH_TABLE_KEY        key -
*   key to hash
*  uint32                         seed -
*    Value to use in the hash calculation
*  uint32                        *hash_val -
*    The calculated hash value.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static shr_error_e
utilex_hash_table_simple_rehash(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 seed,
    uint32 * hash_val)
{
    uint32 hash_table_index, table_width;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(key, _SHR_E_PARAM, "key");
    SHR_NULL_CHECK(hash_val, _SHR_E_PARAM, "hash_val");

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    *hash_val = (seed + 1) % table_width;
exit:
    SHR_FUNC_EXIT;
}
/* } */
#endif

/**
 * \brief
 *   Hash function which maps from key to hash value.
 *   Simple and default implementation of a hash function.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle to the hash table, needed so the hash function
 *     can get the properties of the hash table to consider in the
 *     calculations
 *   \param [in] key -
 *     Key to find hash value for
 *   \param [in] seed -
 *     Seed value to use in the hash calculation. Different 'seed's will
 *     produce different hash values. Not used on this implementation.
 *   \param [in] hash_val -
 *     Pointer. This procedure loads pointed memory by the calculated
 *     hash value.
 * \par INDIRECT INPUT
 *   Details of hash table as extracted from SWSTATE structure corresponding
 *   to 'hash_table'.
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *hash_val -\n
 *     See 'hash_val' in DIRECT INPUT above
 * \remark
 *   None
 */
static shr_error_e
utilex_hash_table_simple_hash(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 seed,
    uint32 * hash_val)
{
    uint32 hash_table_index, indx, tmp, table_width, key_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(key, _SHR_E_PARAM, "key");
    SHR_NULL_CHECK(hash_val, _SHR_E_PARAM, "hash_val");
    tmp = 5381;
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));

    for (indx = 0; indx < key_size; ++indx)
    {
        tmp = ((tmp << 5) + tmp) ^ key[indx];
    }
    *hash_val = tmp % table_width;
exit:
    SHR_FUNC_EXIT;
}

static shr_error_e utilex_hash_table_find_entry(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint8 first_empty,
    uint8 alloc_by_index,
    uint32 * entry,
    uint8 * found,
    uint32 * prev,
    uint8 * first);

/************************************************************************/
/* }                                                                    */
/*  End of internals                                                    */
/************************************************************************/

shr_error_e
utilex_hash_table_init(
    int unit,
    uint32 max_nof_hashs)
{
    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.alloc(unit));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.hashs_array.ptr_alloc(unit, max_nof_hashs));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.max_nof_hashs.set(unit, max_nof_hashs));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.in_use.set(unit, 0));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.occupied_hashs.alloc_bitmap(unit, max_nof_hashs));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_clear_all_tmps(
    int unit)
{
    uint32 hash_table_index;
    uint32 max_buf_size, found, max_nof_hashs, key_size, data_size, ptr_size, in_use;
    uint8 bit_val;
    uint8 is_allocated;
    int32 offset;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.in_use.get(unit, &in_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.max_nof_hashs.get(unit, &max_nof_hashs));

    if (in_use >= max_nof_hashs)
    {
        /*
         * If number of occupied bitmap structures is beyond the
         * maximum then quit with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    /*
     * Find occupied hashs (a set bit in 'occupied_hashs') and, for each,
     * fill 'tmp' buffers by zeroes.
     *
     * Currently, 'tmp' buffers are:
     *   sw_state[_unit]->dpp.soc.sand.hash_table->hashs_array[hashs_array_ptr_arr_idx_0]->hash_data.tmp_buf
     *   sw_state[_unit]->dpp.soc.sand.hash_table->hashs_array[hashs_array_ptr_arr_idx_0]->hash_data.tmp_buf2
     */
    found = 0;
    offset = 0;
    for (hash_table_index = 0; hash_table_index < max_nof_hashs; hash_table_index++)
    {
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.occupied_hashs.bit_get(unit, hash_table_index, &bit_val));
        if (bit_val == 1)
        {
            /*
             * 'hash_table_index' is now the index of an occupied entry.
             */
            found++;
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf.is_allocated(unit, hash_table_index, &is_allocated));
            if (!is_allocated)
            {
                /*
                 * 'tmp_buf' buffer must be allocated, at this point.
                 */
                SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
                SHR_EXIT();
            }
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf2.is_allocated(unit, hash_table_index, &is_allocated));
            if (!is_allocated)
            {
                /*
                 * 'tmp_buf2' buffer must be allocated, at this point.
                 */
                SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
                SHR_EXIT();
            }
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size));
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
            max_buf_size = data_size;
            if (key_size > max_buf_size)
            {
                max_buf_size = key_size;
            }
            if (ptr_size > max_buf_size)
            {
                max_buf_size = ptr_size;
            }
            /*
             * Clear 'tmp_buf'
             */
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf.
                            memset(unit, hash_table_index, offset, max_buf_size, 0));
            /*
             * Clear 'tmp_buf2'
             */
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf2.
                            memset(unit, hash_table_index, offset, max_buf_size, 0));
            if (found >= in_use)
            {
                /*
                 * If all allocated entries have been treated. Quit.
                 */
                break;
            }
        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_create(
    int unit,
    UTILEX_HASH_TABLE_PTR * hash_table_ptr,
    UTILEX_HASH_TABLE_INIT_INFO init_info)
{
    UTILEX_HASH_TABLE_PTR hash_table;
    UTILEX_OCC_BM_INIT_INFO btmp_init_info;
    uint32 max_buf_size;
    uint32 found;
    uint32 hash_table_index, table_size, table_width, data_size, key_size, ptr_size, null_ptr, max_nof_hashs;
    uint32 in_use;
    uint8 bit_val;
    int32 offset;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_NULL_CHECK(hash_table_ptr, _SHR_E_PARAM, "hash_table_ptr");

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.in_use.get(unit, &in_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.max_nof_hashs.get(unit, &max_nof_hashs));

    if (in_use >= max_nof_hashs)
    {
        /*
         * If number of occupied bitmap structures is beyond the
         * maximum then quit with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    /*
     * Increment number of 'in_use' to cover the one we now intend to capture.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.in_use.set(unit, (in_use + 1)));
    /*
     * Find a free hash (a cleared bit in 'occupied_hashs'). At this point,
     * there must be one.
     */
    found = 0;
    for (hash_table_index = 0; hash_table_index < max_nof_hashs; hash_table_index++)
    {
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.occupied_hashs.bit_get(unit, hash_table_index, &bit_val));
        if (bit_val == 0)
        {
            /*
             * 'hash_table_index' is now the index of a free entry.
             */
            found = 1;
            break;
        }
    }
    if (!found)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_RESOURCE);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.occupied_hashs.bit_set(unit, hash_table_index));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.hashs_array.alloc(unit, hash_table_index));
    /*
     * Note that legal handles start at '1', not at '0'.
     */
    UTILEX_HASH_TABLE_CONVERT_HASHTABLE_INDEX_TO_HANDLE(hash_table, hash_table_index);
    /*
     * Set output of this procedure.
     */
    *hash_table_ptr = hash_table;

    utilex_hash_table_info_clear(unit, hash_table);
    /*
     * Load 'init' section of hash table as per input.
     * Make sure all callbacks are NULL - Currently, we only use defaults
     */
    if (init_info.get_entry_fun != NULL || init_info.set_entry_fun != NULL)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_RESOURCE);
        SHR_EXIT();
    }

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.set(unit, hash_table_index, &init_info));

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size));

    if (table_size == 0 || table_width == 0 || key_size == 0 || data_size == 0)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    /*
     * calculate the size of pointers (list head and next) according to table size.
     */
    ptr_size = (utilex_log2_round_up(table_size + 1) + (UTILEX_NOF_BITS_IN_BYTE - 1)) / UTILEX_NOF_BITS_IN_BYTE;
    /*
     * Set 'null_ptr' to all-ones (e.g., 255 in case of one byte).
     * Note that the maximal size is 32 bits!.
     */
    null_ptr = UTILEX_BITS_MASK((ptr_size * UTILEX_NOF_BITS_IN_BYTE - 1), 0);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.set(unit, hash_table_index, ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.null_ptr.set(unit, hash_table_index, null_ptr));
    /*
     * allocate buffer for keys
     */
    offset = 0;
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.keys.alloc(unit, hash_table_index, table_size * key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memset(unit, hash_table_index, offset, table_size * key_size, 0x00));
    /*
     * allocate buffer for next array (to build the linked list)
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.next.alloc(unit, hash_table_index, table_size * ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    next.memset(unit, hash_table_index, offset, table_size * ptr_size, 0xFF));
    /*
     * allocate buffer for lists_head (to build the linked list)
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.lists_head.alloc(unit, hash_table_index, table_width * ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    lists_head.memset(unit, hash_table_index, offset, table_width * ptr_size, 0xFF));

    max_buf_size = data_size;
    if (key_size > max_buf_size)
    {
        max_buf_size = key_size;
    }
    if (ptr_size > max_buf_size)
    {
        max_buf_size = ptr_size;
    }
    /*
     * allocate buffer for temporary (workspace) buffer (no. 1 )
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf.alloc(unit, hash_table_index, max_buf_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf.memset(unit, hash_table_index, offset, max_buf_size, 0x00));
    /*
     * allocate buffer for temporary (workspace) buffer (no. 2 )
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf2.alloc(unit, hash_table_index, max_buf_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf2.memset(unit, hash_table_index, offset, max_buf_size, 0x00));
    utilex_occ_bm_init_info_clear(&btmp_init_info);
    btmp_init_info.size = table_size;
    /*
     * initialize the data to be mapped to
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_create(unit, &btmp_init_info, &(memory_use)));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.set(unit, hash_table_index, memory_use));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_destroy(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table)
{
    uint32 hash_table_index;
    UTILEX_OCC_BM_PTR memory_use;
    uint8 bit_val;
    uint32 in_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);
    /*
     * First, mark this hash table as 'released'
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.in_use.get(unit, &in_use));

    if ((int) in_use < 0)
    {
        /*
         * If number of occupied hashtable structures goes below zero then quit
         * with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    /*
     * Decrement number of 'in_use' to cover the one we now intend to release.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.in_use.set(unit, (in_use - 1)));
    /*
     * Mark specific hash table as 'not occupied'
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.occupied_hashs.bit_get(unit, hash_table_index, &bit_val));
    if (bit_val == 0)
    {
        /*
         * If hash table structure is not indicated as 'occupied' then quit
         * with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.occupied_hashs.bit_clear(unit, hash_table_index));

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.lists_head.free(unit, hash_table_index));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.next.free(unit, hash_table_index));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.keys.free(unit, hash_table_index));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf.free(unit, hash_table_index));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf2.free(unit, hash_table_index));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_destroy(unit, memory_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS.hashs_array.free(unit, hash_table_index));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_entry_add(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 * data_indx,
    uint8 * success)
{
    uint8 found, first;
    uint32 key_size, entry_offset, prev_entry;
    uint32 hash_table_index;

    SHR_FUNC_INIT_VARS(unit);
    /*
     * This (*data_indx) is output.
     * Initialize only to make it possible to print it on exit,
     * even in case of error.
     */
    *data_indx = UTILEX_HASH_TABLE_NULL;
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(key, _SHR_E_PARAM, "key");
    SHR_NULL_CHECK(data_indx, _SHR_E_PARAM, "data_indx");
    SHR_NULL_CHECK(success, _SHR_E_PARAM, "success");
    /*
     * Check to see whether the entry exists
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_find_entry
                    (unit, hash_table, key, TRUE, FALSE, &entry_offset, &found, &prev_entry, &first));

    if (entry_offset == UTILEX_HASH_TABLE_NULL)
    {
        *success = FALSE;
        *data_indx = UTILEX_HASH_TABLE_NULL;
        goto exit;
    }
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    /*
     * Copy 'key_size' bytes from input 'key' buffer into 'UTILEX_HASH_TABLE_ACCESS_DATA.keys'
     * buffer at offset 'entry_offset * key_size'.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memwrite(unit, hash_table_index, key, key_size * entry_offset, key_size));
    *success = TRUE;
    *data_indx = entry_offset;

exit:
    LOG_DEBUG_EX(BSL_LOG_MODULE,
                 " Exit: Key[0,1] %d%d data_indx %d success %d\r\n",
                 (int) key[0], (int) key[1], (int) (*data_indx), (int) (*success));
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_entry_add_at_index(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 data_indx,
    uint8 * success)
{
    uint8 found, indx_in_use, first;
    uint32 entry_offset, prev_entry;
    uint32 old_index;
    uint32 hash_table_index, key_size;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(key, _SHR_E_PARAM, "key");
    SHR_NULL_CHECK(success, _SHR_E_PARAM, "success");

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    /*
     * check if this key already exist 
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup(unit, hash_table, key, &old_index, &found));

    if (found && old_index != data_indx)
    {
        *success = FALSE;
        goto exit;
    }
    if (found && old_index == data_indx)
    {
        /*
         * found in required index, done 
         */
        *success = TRUE;
        goto exit;
    }
    SHR_IF_ERR_EXIT(utilex_occ_bm_is_occupied(unit, memory_use, data_indx, &indx_in_use));
    if (indx_in_use && !found)
    {
        /*
         * index already in use for other usage 
         */
        *success = FALSE;
        goto exit;
    }
    entry_offset = data_indx;
    /*
     * check to see if the entry exists 
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_find_entry
                    (unit, hash_table, key, TRUE, TRUE, &entry_offset, &found, &prev_entry, &first));
    if (entry_offset == UTILEX_HASH_TABLE_NULL)
    {
        *success = FALSE;
        goto exit;
    }
    /*
     * Copy 'key_size' bytes from input 'key' buffer into 'UTILEX_HASH_TABLE_ACCESS_DATA.keys'
     * buffer at offset 'entry_offset * key_size'.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memwrite(unit, hash_table_index, key, key_size * entry_offset, key_size));
    *success = TRUE;
exit:
    SHR_FUNC_EXIT;
}
shr_error_e
utilex_hash_table_entry_remove(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key)
{
    uint8 found, first;
    uint32 entry_offset, prev_entry;
    uint32 null_ptr[1], hash_table_index, ptr_size;
    uint8 *next_entry;
    uint8 *tmp_buf_ptr;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(key, _SHR_E_PARAM, "key");
    /*
     * check to see whether the entry exists
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_find_entry
                    (unit, hash_table, key, FALSE, FALSE, &entry_offset, &found, &prev_entry, &first));
    /*
     * If the key to remove does not exist in the hash table then this operation has
     * no side effect.
     */
    if (!found)
    {
        goto exit;
    }
    /*
     * Remove node from linked list
     */
    /*
     * Get next pointer
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(utilex_hash_table_get_tmp_data_ptr_from_handle(unit, hash_table, &tmp_buf_ptr));
    /*
     * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.next' buffer at offset
     * 'entry_offset * ptr_size' into 'tmp_buf' buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    next.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * entry_offset, ptr_size));
    next_entry = tmp_buf_ptr;
    /*
     * if this is first node in the linked list then set head list
     */
    if (first)
    {
        /*
         * Copy 'ptr_size' bytes from 'next_entry' buffer into 'UTILEX_HASH_TABLE_ACCESS_DATA.lists_head'
         * buffer at offset 'prev_entry * ptr_size'.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        lists_head.memwrite(unit, hash_table_index, next_entry, ptr_size * prev_entry, ptr_size));
    }
    else
    {
        /*
         * Copy 'ptr_size' bytes from 'next_entry' buffer into 'UTILEX_HASH_TABLE_ACCESS_DATA.next'
         * buffer at offset 'prev_entry * ptr_size'.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        next.memwrite(unit, hash_table_index, next_entry, ptr_size * prev_entry, ptr_size));
    }
    /*
     * Now the entry 'entry_offset' is not in use. free it.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_occup_status_set(unit, memory_use, entry_offset, FALSE));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.null_ptr.get(unit, hash_table_index, null_ptr));
    SHR_IF_ERR_EXIT(utilex_U32_to_U8(null_ptr, ptr_size, tmp_buf_ptr));
    /*
     * Copy 'ptr_size' bytes from 'tmp_buf_ptr' (tmp_buf buffer) into 'UTILEX_HASH_TABLE_ACCESS_DATA.next'
     * buffer at offset 'entry_offset * ptr_size'.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    next.memwrite(unit, hash_table_index, tmp_buf_ptr, ptr_size * entry_offset, ptr_size));
exit:
    SHR_FUNC_EXIT;
}
shr_error_e
utilex_hash_table_entry_remove_by_index(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 data_indx)
{
    uint8 *cur_key;
    uint8 in_use;
    uint32 key_size, table_size, hash_table_index;
    UTILEX_OCC_BM_PTR memory_use;
    uint8 *tmp_buf2_ptr = NULL;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    /*
     * Check whether the entry 'entry_offset' is in use.
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_is_occupied(unit, memory_use, data_indx, &in_use));
    if (!in_use)
    {
        /*
         * If entry 'entry_offset' is not in use, quit.
         */
        goto exit;
    }
    SHR_IF_ERR_EXIT(utilex_hash_table_get_tmp2_data_ptr_from_handle(unit, hash_table, &tmp_buf2_ptr));
    /*
     * Copy 'key_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.keys' buffer at offset
     * 'data_indx * key_size' into 'tmp_buf2' buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memread(unit, hash_table_index, tmp_buf2_ptr, key_size * data_indx, key_size));
    cur_key = tmp_buf2_ptr;
    SHR_IF_ERR_EXIT(utilex_hash_table_entry_remove(unit, hash_table, cur_key));
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *   Lookup the hash table for the given key, using a few conditional
 *   parameters, and get information corresponding to that entry
 *   like: Is it on the table at all? Is it first on the table?
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to look for the key in.
 *   \param [in] key -
 *     Pointer to an array of uint8s containing the key to look for.
 *   \param [in] get_first_empty -
 *     Only meaningful if no such key was found on the specified hash
 *     table. In that case, always set '*found = FALSE;' plus:
 *     if 'get_first_empty' is not true, then
 *       this procedures loads the following pointers as indicated and quits:
 *         *entry = UTILEX_HASH_TABLE_NULL;
 *     else, behaviour depends on 'alloc_by_index':
 *       if (alloc_by_index is NOT true) then check whether there is
 *         an empty entry on the hash table. If there is, the load
 *         *entry by its identifying index. Otherwise, set '*entry = UTILEX_HASH_TABLE_NULL;'
 *       Otherwise, use *entry as input. Set the index contained
 *       in *entry (on the hash table) as 'being used' regardless on
 *       whether it is already occupied, and exit.
 *   \param [in] alloc_by_index -
 *     See 'get_first_empty' above.
 *   \param [in] entry -
 *     See 'get_first_empty' above. Memory pointed by 'entry' may be either
 *     input or output!
 *   \param [in] found -
 *     Pointer to memory to be loaded by this procedure by indication
 *     on whether the key was found in the hash table (TRUE) or not (FALSE)
 *   \param [in] prev_entry -
 *     Pointer to memory to be loaded by this procedure by value
 *     depending on *first (which is also INDIRECT OUTPUT).
 *     If '*first' is TRUE then '*prev'_entry is loaded by the hash value
 *       corresponding to 'key' specified above.
 *     Otherwise, it is loaded by the last 'next' value (index into OCC bitmap)
 *       which has been visited while searching for specified 'key'. If
 *       'key' has been found then this is its index but if it has not been
 *       found then this is the last valid 'next' index on the chain.
 *   \param [in] first -
 *     Pointer to memory to be loaded by this procedure by indication
 *     on whether the key was last searched for at the start of the table
 *     (*first set to TRUE) or not (*first set to FALSE)
 * \par INDIRECT INPUT
 *   SWSTATE system (For example, UTILEX_HASH_TABLE_ACCESS_DATA.lists_head,
 *   UTILEX_HASH_TABLE_ACCESS_DATA.next, etc.)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   \b *entry (See 'entry' on DIRECT INPUT above).            \n
 *   \b *found (See 'found' on DIRECT INPUT above).            \n
 *   \b *prev_entry (See 'prev_entry' on DIRECT INPUT above).  \n
 *   \b *first (See 'first' on DIRECT INPUT above).            \n
 */
static shr_error_e
utilex_hash_table_find_entry(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint8 get_first_empty,
    uint8 alloc_by_index,
    uint32 * entry,
    uint8 * found,
    uint32 * prev_entry,
    uint8 * first)
{
    uint8 *cur_key, *next;
    uint32 key_size, ptr_size, null_ptr, hash_val, ptr_long[1], next_node;
    uint8 not_found, found_new;
    uint32 hash_table_index;
    uint8 *tmp_buf_ptr;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);
    /*
     * Load a true pointer by zero value if the handle 'hash_table' is zero.
     * Some compilations do not allow an integer to be cast as pointer.
     */
    if (hash_table == 0)
    {
        next = (uint8 *) 0;
    }
    else
    {
        next = (uint8 *) (~0);
    }
    SHR_NULL_CHECK(next, _SHR_E_PARAM, "hash_table");
    SHR_NULL_CHECK(key, _SHR_E_PARAM, "key");
    SHR_NULL_CHECK(found, _SHR_E_PARAM, "found");

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.null_ptr.get(unit, hash_table_index, &null_ptr));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));

    *ptr_long = 0;
    *prev_entry = UTILEX_HASH_TABLE_NULL;
    *first = TRUE;
    /*
     * Hash the key to get list head
     */
    /*
     * Just to keep compiler happy on initialization...
     */
    hash_val = 0;
    SHR_IF_ERR_EXIT(utilex_hash_table_simple_hash(unit, hash_table, key, 0, &hash_val));
    *prev_entry = hash_val;
    /*
     * Read list head in two steps: (a) get temp. storage, (b) Copy
     * lists_head into this temporary buffer.
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_get_tmp_data_ptr_from_handle(unit, hash_table, &tmp_buf_ptr));
    /*
     * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.lists_head' buffer at offset
     * 'hash_val * ptr_size' into 'tmp_buf' buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    lists_head.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * hash_val, ptr_size));
    /*
     * Check whether the list head is null.
     */
    *ptr_long = 0;
    SHR_IF_ERR_EXIT(utilex_U8_to_U32(tmp_buf_ptr, ptr_size, ptr_long));
    if (*ptr_long == null_ptr)
    {
        if (get_first_empty)
        {
            if (alloc_by_index)
            {
                /*
                 * don't check if this index already in use 
                 */
                SHR_IF_ERR_EXIT(utilex_occ_bm_occup_status_set(unit, memory_use, *entry, TRUE));

                found_new = TRUE;

                *ptr_long = *entry;
            }
            else
            {
                SHR_IF_ERR_EXIT(utilex_occ_bm_alloc_next(unit, memory_use, ptr_long, &found_new));

            }
            if (!found_new)
            {
                *ptr_long = UTILEX_HASH_TABLE_NULL;
                *entry = UTILEX_HASH_TABLE_NULL;
            }
            else
            {
                SHR_IF_ERR_EXIT(utilex_U32_to_U8(ptr_long, ptr_size, tmp_buf_ptr));
                /*
                 * Copy 'ptr_size' bytes from 'tmp_buf' buffer into 'UTILEX_HASH_TABLE_ACCESS_DATA.lists_head'
                 * buffer at offset 'hash_val * ptr_size'.
                 */
                SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.lists_head.memwrite(unit,
                                                                                  hash_table_index, tmp_buf_ptr,
                                                                                  ptr_size * hash_val, ptr_size));
                *entry = *ptr_long;
            }
            *found = FALSE;
            goto exit;
        }
        *found = FALSE;
        *entry = UTILEX_HASH_TABLE_NULL;
        goto exit;
    }
    not_found = TRUE;

    while (*ptr_long != null_ptr)
    {
        /*
         * read keys
         */
        /*
         * Copy 'key_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.keys' buffer at offset
         * '*ptr_long * key_size' into 'tmp_buf' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        keys.memread(unit, hash_table_index, tmp_buf_ptr, key_size * (*ptr_long), key_size));
        cur_key = tmp_buf_ptr;
        not_found = (uint8) sal_memcmp(cur_key, key, (key_size * sizeof(UTILEX_HASH_TABLE_KEY)));
        if (not_found == FALSE)
        {
            /*
             * if key was found, quit
             */
            *found = TRUE;
            *entry = *ptr_long;
            goto exit;
        }
        /*
         * If wasn't found then look in the next node (next OCC place) on the list.
         */
        *first = FALSE;
        /*
         * Get next node (next OCC place)
         */
        /*
         * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.next' buffer at offset
         * '*ptr_long * ptr_size' into 'tmp_buf' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        next.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * (*ptr_long), ptr_size));
        next = tmp_buf_ptr;
        *prev_entry = *ptr_long;
        *ptr_long = 0;
        SHR_IF_ERR_EXIT(utilex_U8_to_U32(next, ptr_size, ptr_long));
    }
    /*
     * Didn't find anything, return 'not found'
     * but point the entry to the next available place if
     * 'get_first_empty' is set.
     */
    *found = FALSE;
    if (get_first_empty)
    {
        if (alloc_by_index)
        {
            /*
             * don't check if this index already in use 
             */
            SHR_IF_ERR_EXIT(utilex_occ_bm_occup_status_set(unit, memory_use, *entry, TRUE));
            found_new = TRUE;
            next_node = *entry;
        }
        else
        {
            SHR_IF_ERR_EXIT(utilex_occ_bm_alloc_next(unit, memory_use, &next_node, &found_new));
        }
        if (!found_new)
        {
            *entry = UTILEX_HASH_TABLE_NULL;
        }
        else
        {
            /*
             * We assume here that 'ptr_size' is smaller than sizeof(uint32) !!!
             */
            ptr_long[0] = next_node ;
            /*
             * For coverity, give utilex_U32_to_U8() first input which is an array
             * (and not directly, say, &next_node)
             */
            SHR_IF_ERR_EXIT(utilex_U32_to_U8(ptr_long, ptr_size, tmp_buf_ptr));
            /*
             * Copy 'ptr_size' bytes from 'tmp_buf' buffer into 'UTILEX_HASH_TABLE_ACCESS_DATA.next'
             * buffer at offset '(*prev_entry) * ptr_size'.
             */
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                            next.memwrite(unit, hash_table_index, tmp_buf_ptr, ptr_size * (*prev_entry), ptr_size));
            *entry = next_node;
        }
    }
    else
    {
        *entry = UTILEX_HASH_TABLE_NULL;
    }

exit:
    SHR_FUNC_EXIT;
}
shr_error_e
utilex_hash_table_entry_lookup(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 * data_indx,
    uint8 * found)
{
    uint8 is_found, first;
    uint32 entry_offset, prev_entry;
    uint32 hash_table_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(key, _SHR_E_PARAM, "key");
    SHR_NULL_CHECK(data_indx, _SHR_E_PARAM, "data_indx");
    SHR_NULL_CHECK(found, _SHR_E_PARAM, "found");

    /*
     * Check to see whether the entry exists 
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_find_entry(unit,
                                                 hash_table, key, FALSE, FALSE, &entry_offset, &is_found, &prev_entry,
                                                 &first));

    /*
     * if the key to remove is not exist in the hash table then this operation has
     * no side effect.
     */
    if (!is_found)
    {
        *found = FALSE;
        *data_indx = UTILEX_HASH_TABLE_NULL;
        goto exit;
    }
    *found = TRUE;
    *data_indx = entry_offset;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_get_by_index(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 data_indx,
    UTILEX_HASH_TABLE_KEY * key,
    uint8 * found)
{
    uint32 table_size, key_size, hash_table_index = 0;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    if (data_indx > table_size)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    /*
     * check if the entry 'entry_offset' is in use.
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_is_occupied(unit, memory_use, data_indx, found));
    /*
     * check if the entry 'entry_offset' is in use.
     */
    if (!*found)
    {
        goto exit;
    }
    /*
     * Copy 'key_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.keys' buffer at offset
     * 'data_indx * key_size' into input 'key' buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memread(unit, hash_table_index, key, key_size * data_indx, key_size));
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*   utilex_hash_table_clear
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*   Clear the hash table content without setting free the memory
* INPUT:
*   int                        unit -
*     Identifier of the device to access.
*   UTILEX_HASH_TABLE_PTR    hash_table -
*     Handle of the hash table instance
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_hash_table_clear(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table)
{
    uint32 max_buf_size, data_size, ptr_size, table_width, table_size, key_size, hash_table_index;
    UTILEX_OCC_BM_PTR memory_use;
    int32 offset;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size));
    offset = 0;
    /*
     * Clear buffer for keys array
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memset(unit, hash_table_index, offset, table_size * key_size, 0x00));
    /*
     * clear buffer for next array (to build the linked list)
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    next.memset(unit, hash_table_index, offset, table_size * ptr_size, 0xFF));
    /*
     * clear buffer for lists_head (to build the linked list)
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    lists_head.memset(unit, hash_table_index, offset, table_width * ptr_size, 0xFF));
    max_buf_size = data_size;
    if (key_size > max_buf_size)
    {
        max_buf_size = key_size;
    }
    if (ptr_size > max_buf_size)
    {
        max_buf_size = ptr_size;
    }
    /*
     * Clear buffers for workspace
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf.memset(unit, hash_table_index, offset, max_buf_size, 0x00));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.tmp_buf2.memset(unit, hash_table_index, offset, max_buf_size, 0x00));
    /*
     * initialize the data to be mapped to 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_clear(unit, memory_use));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_get_next(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    UTILEX_HASH_TABLE_ITER * iter,
    UTILEX_HASH_TABLE_KEY * const key,
    uint32 * data_indx)
{
    uint32 indx;
    uint32 table_size, key_size, hash_table_index;
    uint8 occupied;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(iter, _SHR_E_PARAM, "iter");
    SHR_NULL_CHECK(data_indx, _SHR_E_PARAM, "data_indx");
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    /*
     * traverse the hash table head list.
     */
    for (indx = *iter; indx < table_size; ++indx)
    {
        SHR_IF_ERR_EXIT(utilex_occ_bm_is_occupied(unit, memory_use, indx, &occupied));
        if (occupied)
        {
            *data_indx = indx;
            *iter = indx + 1;
            /*
             * Copy 'key_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.keys' buffer at offset
             * 'indx * key_size' into input 'key' buffer.
             */
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                            keys.memread(unit, hash_table_index, key, key_size * indx, key_size));
            goto exit;
        }
    }
    *iter = UTILEX_U32_MAX;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_get_size_for_save(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 * size)
{
    uint32 table_width, table_size, key_size, ptr_size, bmp_size, total_size;
    uint32 hash_table_index;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    total_size = 0;

    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(size, _SHR_E_PARAM, "size");
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    /*
     * init info 
     */
    total_size += sizeof(UTILEX_HASH_TABLE_INIT_INFO);

    /*
     * DS data 
     */

    total_size += table_size * key_size;

    total_size += table_size * ptr_size;

    total_size += table_width * ptr_size;

    /*
     * initialize the data to be mapped to 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_get_size_for_save(unit, memory_use, &bmp_size));
    total_size += bmp_size;
    *size = total_size;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_save(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint8 * buffer,
    uint32 buffer_size_bytes,
    uint32 * actual_size_bytes)
{
    uint8 *cur_ptr;
    uint32 cur_size, table_size, key_size, ptr_size, table_width, total_size;
    uint32 hash_table_index;
    UTILEX_OCC_BM_PTR memory_use;
    int offset;

    SHR_FUNC_INIT_VARS(unit);
    /*
     * 'cur_ptr' is the the running pointer, into the input 'buffer' of the next free space.
     */
    cur_ptr = buffer;
    /*
     * 'total_size' is the running number of bytes required for the saving of the hash table.
     * It is increased along the flow of the procedure.
     * Before exit, it is loaded into '*actual_size_bytes' which is the final
     * number of bytes required for the saving.
     */
    total_size = 0;

    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_NULL_CHECK(buffer, _SHR_E_PARAM, "buffer");
    SHR_NULL_CHECK(actual_size_bytes, _SHR_E_PARAM, "actual_size_bytes");

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    offset = 0;
    /*
     * copy init info to 'cur_ptr' 
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.get(unit, hash_table_index, (UTILEX_HASH_TABLE_INIT_INFO *) cur_ptr));
    cur_ptr += sizeof(UTILEX_HASH_TABLE_INIT_INFO);

    /*
     * copy DS data to 'cur_ptr' 
     */
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memread(unit, hash_table_index, cur_ptr, offset, table_size * key_size));
    cur_ptr += (table_size * key_size);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    next.memread(unit, hash_table_index, cur_ptr, offset, table_size * ptr_size));
    cur_ptr += (table_size * ptr_size);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    lists_head.memread(unit, hash_table_index, cur_ptr, offset, table_width * ptr_size));
    cur_ptr += (table_width * ptr_size);
    total_size += (uint32) (cur_ptr - buffer);
    /*
     * initialize the data to be mapped to 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_save(unit, memory_use, cur_ptr, buffer_size_bytes - total_size, &cur_size));
    cur_ptr += cur_size;
    total_size += cur_size;
    *actual_size_bytes = total_size;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_load(
    int unit,
    uint8 ** buffer,
    UTILEX_HASH_MAP_SW_DB_ENTRY_SET set_entry_fun,
    UTILEX_HASH_MAP_SW_DB_ENTRY_GET get_entry_fun,
    UTILEX_HASH_MAP_HASH_FUN_CALL_BACK hash_function,
    UTILEX_HASH_MAP_HASH_FUN_CALL_BACK rehash_function,
    UTILEX_HASH_TABLE_PTR * hash_table_ptr)
{
    uint8 *cur_ptr;
    UTILEX_HASH_TABLE_PTR hash_table;
    uint32 hash_table_index, table_size, key_size, ptr_size, table_width;
    UTILEX_HASH_TABLE_INIT_INFO init_info;
    int offset;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_NULL_CHECK(buffer, _SHR_E_PARAM, "buffer");

    cur_ptr = (uint8 *) buffer[0];

    /*
     * copy init info from 'cur_ptr' 
     */
    sal_memcpy(&(init_info), cur_ptr, sizeof(UTILEX_HASH_TABLE_INIT_INFO));
    cur_ptr += sizeof(UTILEX_HASH_TABLE_INIT_INFO);
    init_info.hash_function = hash_function;
    init_info.rehash_function = rehash_function;
    init_info.set_entry_fun = set_entry_fun;
    init_info.get_entry_fun = get_entry_fun;

    /*
     * create DS 
     */

    /*
     * will not work!! (petra only code) 
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_create(unit, &hash_table, init_info));

    *hash_table_ptr = hash_table;
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.get(unit, hash_table_index, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    offset = 0;

    /*
     * copy DS data from 'cur_ptr' 
     */

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    keys.memwrite(unit, hash_table_index, cur_ptr, offset, table_size * key_size));
    cur_ptr += (table_size * key_size);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    next.memwrite(unit, hash_table_index, cur_ptr, offset, table_size * ptr_size));
    cur_ptr += (table_size * ptr_size);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                    lists_head.memwrite(unit, hash_table_index, cur_ptr, offset, table_width * ptr_size));
    cur_ptr += (table_width * ptr_size);

    /*
     * load bitmap from 'cur_ptr' -- Start by destroy 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_destroy(unit, memory_use));
    /*
     * load bitmap 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_load(unit, &cur_ptr, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.memory_use.set(unit, hash_table_index, memory_use));
    *buffer = cur_ptr;
exit:
    SHR_FUNC_EXIT;
}

#if UTILEX_DEBUG
/* { */

shr_error_e
utilex_hash_table_get_num_active(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table,
    uint32 * num_active_ptr)
{
    uint32 indx;
    uint32 ptr_long;
    uint8 *list_head, *next;
    uint32 table_width, ptr_size, null_ptr, hash_table_index;
    uint8 *tmp_buf_ptr;
    uint32 num_active;

    SHR_FUNC_INIT_VARS(unit);
    num_active = 0;
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.null_ptr.get(unit, hash_table_index, &null_ptr));
    SHR_IF_ERR_EXIT(utilex_hash_table_get_tmp_data_ptr_from_handle(unit, hash_table, &tmp_buf_ptr));
    /*
     * traverse the hash table head list.
     */
    for (indx = 0; indx < table_width; ++indx)
    {
        /*
         * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.lists_head' buffer at offset
         * 'indx * ptr_size' into 'tmp_buf' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        lists_head.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * indx, ptr_size));
        list_head = tmp_buf_ptr;
        ptr_long = 0;
        SHR_IF_ERR_EXIT(utilex_U8_to_U32(list_head, ptr_size, &ptr_long));
        if (ptr_long == null_ptr)
        {
            continue;
        }
        while (ptr_long != null_ptr)
        {
            num_active++;
            /*
             * get next node (next OCC place)
             */
            /*
             * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.next' buffer at offset
             * 'ptr_long * ptr_size' into 'tmp_buf' buffer.
             */
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                            next.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * ptr_long, ptr_size));
            next = tmp_buf_ptr;
            ptr_long = 0;
            SHR_IF_ERR_EXIT(utilex_U8_to_U32(next, ptr_size, &ptr_long));
        }
    }
exit:
    *num_active_ptr = num_active;
    SHR_FUNC_EXIT;
}
shr_error_e
utilex_hash_table_print(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table)
{
    uint32 indx;
    uint32 ptr_long, print_indx;
    uint8 *list_head, *cur_key, *next;
    uint32 table_width, ptr_size, null_ptr, key_size, hash_table_index;
    uint8 *tmp_buf_ptr;
    char *next_text;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.null_ptr.get(unit, hash_table_index, &null_ptr));
    SHR_IF_ERR_EXIT(utilex_hash_table_get_tmp_data_ptr_from_handle(unit, hash_table, &tmp_buf_ptr));

    /*
     * traverse the hash table head list.
     */
    for (indx = 0; indx < table_width; ++indx)
    {
        /*
         * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.lists_head' buffer at offset
         * 'indx * ptr_size' into 'tmp_buf' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        lists_head.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * indx, ptr_size));
        list_head = tmp_buf_ptr;
        ptr_long = 0;
        next_text = "OCC place";

        SHR_IF_ERR_EXIT(utilex_U8_to_U32(list_head, ptr_size, &ptr_long));

        if (ptr_long == null_ptr)
        {
            continue;
        }
        LOG_CLI((BSL_META_U(unit, " entry %2u:  "), indx));

        while (ptr_long != null_ptr)
        {
            /*
             * read keys
             */
            /*
             * Copy 'key_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.keys' buffer at offset
             * 'ptr_long * key_size' into 'tmp_buf' buffer.
             */
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                            keys.memread(unit, hash_table_index, tmp_buf_ptr, key_size * ptr_long, key_size));
            cur_key = tmp_buf_ptr;

            LOG_CLI((BSL_META_U(unit, "(Key: 0x")));
            for (print_indx = 0; print_indx < key_size; ++print_indx)
            {
                LOG_CLI((BSL_META_U(unit, "%02x"), cur_key[key_size - print_indx - 1]));
            }
            LOG_CLI((BSL_META_U(unit, "--> %s: %d)"), next_text, (int) (ptr_long)));
            LOG_CLI((BSL_META_U(unit, "\t")));
            /*
             * get next node (next OCC place)
             */
            /*
             * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.next' buffer at offset
             * 'ptr_long * ptr_size' into 'tmp_buf' buffer.
             */
            SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                            next.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * ptr_long, ptr_size));
            next = tmp_buf_ptr;
            ptr_long = 0;
            SHR_IF_ERR_EXIT(utilex_U8_to_U32(next, ptr_size, &ptr_long));

            if (ptr_long == null_ptr)
            {
                LOG_CLI((BSL_META_U(unit, "next node (next OCC place)--> NONE!)")));
            }
            else
            {
                LOG_CLI((BSL_META_U(unit, "next node (next OCC place)--> %u)"), ptr_long));
            }
            next_text = "OCC place";
        }
        LOG_CLI((BSL_META_U(unit, "\n")));
    }
exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief
 *   Prints the 'next' and 'key' auxiliary tables corresponding to
 *   the specified hash table. All entries are printed, including
 *   empty ones.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] hash_table -
 *     Handle of the hash table to print auxiliary tables for.
 * \par INDIRECT INPUT
 *   SWSTATE system
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   Printed auxiliary tables of specified hash table.
 * \remarks
 *   LOG_CLI is used for the printing.
 */
static shr_error_e
utilex_next_and_key_table_print(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table)
{
    uint32 indx;
    uint32 ptr_long, print_indx;
    uint8 *cur_key, *next;
    uint32 table_width, ptr_size, key_size, hash_table_index;
    uint8 *tmp_buf_ptr;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.ptr_size.get(unit, hash_table_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));
    SHR_IF_ERR_EXIT(utilex_hash_table_get_tmp_data_ptr_from_handle(unit, hash_table, &tmp_buf_ptr));
    /*
     * Traverse the 'next' table and the 'keys' table.
     */
    for (indx = 0; indx < table_width; ++indx)
    {
        /*
         * Copy 'ptr_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.next' buffer at offset
         * 'indx * ptr_size' into 'tmp_buf' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        next.memread(unit, hash_table_index, tmp_buf_ptr, ptr_size * indx, ptr_size));
        next = tmp_buf_ptr;
        ptr_long = 0;
        SHR_IF_ERR_EXIT(utilex_U8_to_U32(next, ptr_size, &ptr_long));
        LOG_CLI((BSL_META_U(unit, " entry %2u: Pointer Value %3d "), indx, (int) ptr_long));
        /*
         * read key
         */
        /*
         * Copy 'key_size' bytes from 'UTILEX_HASH_TABLE_ACCESS_DATA.keys' buffer at offset
         * 'indx * key_size' into 'tmp_buf' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_DATA.
                        keys.memread(unit, hash_table_index, tmp_buf_ptr, key_size * indx, key_size));
        cur_key = tmp_buf_ptr;
        LOG_CLI((BSL_META_U(unit, "Key 0x")));
        for (print_indx = 0; print_indx < key_size; ++print_indx)
        {
            LOG_CLI((BSL_META_U(unit, "%02x"), cur_key[key_size - print_indx - 1]));
        }
        LOG_CLI((BSL_META_U(unit, "\n")));
    }
    LOG_CLI((BSL_META_U(unit, "\n")));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_hash_table_info_print(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table)
{
    uint32 data_size, key_size, table_size, table_width, hash_table_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);

    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.data_size.get(unit, hash_table_index, &data_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.key_size.get(unit, hash_table_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_size.get(unit, hash_table_index, &table_size));
    SHR_IF_ERR_EXIT(UTILEX_HASH_TABLE_ACCESS_INFO.table_width.get(unit, hash_table_index, &table_width));

    LOG_CLI((BSL_META_U(unit, "init_info.data_size  : %u\n"), data_size));
    LOG_CLI((BSL_META_U(unit, "init_info.key_size   : %u\n"), key_size));
    LOG_CLI((BSL_META_U(unit, "init_info.table_size : %u\n"), table_size));
    LOG_CLI((BSL_META_U(unit, "init_info.table_width: %u\n"), table_width));
exit:
    SHR_FUNC_EXIT;
}
shr_error_e
utilex_hash_table_test_1(
    int unit)
{
    int ii;
    uint32 hash_table_index;
    UTILEX_HASH_TABLE_PTR hash_table;
    UTILEX_HASH_TABLE_INIT_INFO hash_init;
    int prime_handle;
    uint32 member_size, sec_handle, nof_members, active_nof_members, num_active;
    UTILEX_HASH_MAP_SW_DB_ENTRY_GET get_entry_fun;
    UTILEX_HASH_MAP_SW_DB_ENTRY_SET set_entry_fun;
    UTILEX_HASH_TABLE_KEY hash_table_key[2];
    uint32 data_indx, last_data_indx;
#if (0)
    uint32 first_data_indx;
#endif
    uint8 success;
    uint8 found;
    shr_error_e err;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
    member_size = 2;
    prime_handle = 100;
    sec_handle = 200;
    /*
     * 'active_nof_members' is the number of hash table enries which
     * are going to be set active (by loading keys). It must be smaller
     * than 'nof_members' which is the total number of available
     * enries on this table.
     */
    active_nof_members = 10;
    nof_members = 15;
    get_entry_fun = NULL;
    set_entry_fun = NULL;

    hash_init.data_size = member_size;
    hash_init.key_size = member_size;
    hash_init.prime_handle = prime_handle;
    hash_init.sec_handle = sec_handle;
    hash_init.table_size = nof_members;
    hash_init.table_width = nof_members * 1;
    hash_init.get_entry_fun = get_entry_fun;
    hash_init.set_entry_fun = set_entry_fun;
    SHR_IF_ERR_EXIT(utilex_hash_table_create(unit, &hash_table, hash_init));
    UTILEX_HASH_TABLE_CONVERT_HANDLE_TO_HASHTABLE_INDEX(hash_table_index, hash_table);
    UTILEX_HASH_TABLE_VERIFY_HASHTABLE_IS_ACTIVE(unit, hash_table_index);
    SHR_IF_ERR_EXIT(utilex_hash_table_info_print(unit, hash_table));
    /*
     * Build a hash table with 'nof_members' elements. Each with a key of 'member_size'
     * bytes.
     * With current hash, there should be two entries with 'next' elements (i.e.,
     * First hit is not necessarily final).
     */
    hash_table_key[0] = (UTILEX_HASH_TABLE_KEY) 1;
    for (ii = 1; ii < sizeof(hash_table_key) - 1; ii++)
    {
        hash_table_key[ii] = hash_table_key[ii - 1] + 1;
    }
    hash_table_key[ii] = 0;
    for (ii = 0; ii < active_nof_members; ii++)
    {
        SHR_IF_ERR_EXIT(utilex_hash_table_entry_add(unit, hash_table,
                                                    (UTILEX_HASH_TABLE_KEY *) hash_table_key, &data_indx, &success));
        if (!success)
        {
            SHR_IF_ERR_EXIT(_SHR_E_INTERNAL);
        }
#if (0)
        if (ii == 0)
        {
            first_data_indx = data_indx;
        }
#endif
        hash_table_key[sizeof(hash_table_key) - 1] = hash_table_key[sizeof(hash_table_key) - 1] + 1;
    }
    LOG_CLI((BSL_META_U(unit,
                        " Display table (lists_head) with %d elements (index is hash val)\r\n"
                        " ===============================================================\r\n"), (int) nof_members));
    SHR_IF_ERR_EXIT(utilex_hash_table_print(unit, hash_table));
    LOG_CLI((BSL_META_U(unit, " \r\n" " ---------------------------------------------------------------\r\n")));
    LOG_CLI((BSL_META_U(unit,
                        " Display table (next and keys) with %d elements (index is OCC place)\r\n"
                        " ===================================================================\r\n"),
             (int) nof_members));
    SHR_IF_ERR_EXIT(utilex_next_and_key_table_print(unit, hash_table));
    LOG_CLI((BSL_META_U(unit, " \r\n" " --------------------------------------------------------------------\r\n")));
    /*
     * At this point, there should be 'active_nof_members' keys in the table.
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_get_num_active(unit, hash_table, &num_active));
    if (num_active != active_nof_members)
    {
        SHR_IF_ERR_EXIT(_SHR_E_INTERNAL);
    }
    /*
     * Set 'hash_table_key' to be the last key added.
     */
    hash_table_key[sizeof(hash_table_key) - 1] = hash_table_key[sizeof(hash_table_key) - 1] - 1;
    /*
     * Try to add the last key again. Should succeed.
     */
    err = utilex_hash_table_entry_add(unit, hash_table,
                                      (UTILEX_HASH_TABLE_KEY *) hash_table_key, &last_data_indx, &success);
    if ((err != _SHR_E_NONE) || (success != TRUE))
    {
        SHR_IF_ERR_EXIT(_SHR_E_INTERNAL);
    }
    /*
     * Try to add the last at an offset different from its reported offset.
     * Should fail: Get error (success != TRUE).
     */
    err = utilex_hash_table_entry_add_at_index(unit, hash_table,
                                               (UTILEX_HASH_TABLE_KEY *) hash_table_key, (last_data_indx - 1),
                                               &success);
    if ((err == _SHR_E_NONE) && success)
    {
        SHR_IF_ERR_EXIT(_SHR_E_INTERNAL);
    }
    /*
     * Look for the last key. It should be found.
     */
    LOG_CLI((BSL_META_U(unit, " Look for the last key\r\n" " =====================\r\n")));
    last_data_indx = data_indx;
    SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup(unit, hash_table,
                                                   (UTILEX_HASH_TABLE_KEY *) hash_table_key, &data_indx, &found));
    if (!found)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_UNAVAIL,
                                 " Key starting with %d%d was not found! %s\r\n",
                                 hash_table_key[0], hash_table_key[1], EMPTY);
    }
    else
    {
        LOG_CLI((BSL_META_U(unit, " OK. Last key was found\r\n")));
        if (last_data_indx != data_indx)
        {
            SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_UNAVAIL,
                                     " Data index of last key (%d) is not as expected (%d)! %s\r\n",
                                     data_indx, last_data_indx, EMPTY);
        }
        else
        {
            LOG_CLI((BSL_META_U(unit,
                                " OK. Data index of last key (%d) is as expected (%d)\r\n"), data_indx,
                     last_data_indx));
        }
    }
    LOG_CLI((BSL_META_U(unit, " ------------------------------\r\n")));
    /*
     * Remove all 'active_nof_members' elements except last.
     */
    hash_table_key[0] = (UTILEX_HASH_TABLE_KEY) 1;
    for (ii = 1; ii < sizeof(hash_table_key) - 1; ii++)
    {
        hash_table_key[ii] = hash_table_key[ii - 1] + 1;
    }
    hash_table_key[ii] = 0;
    for (ii = 0; ii < (active_nof_members - 1); ii++)
    {
        SHR_IF_ERR_EXIT(utilex_hash_table_entry_remove(unit, hash_table, (UTILEX_HASH_TABLE_KEY *) hash_table_key));
        hash_table_key[sizeof(hash_table_key) - 1] = hash_table_key[sizeof(hash_table_key) - 1] + 1;
    }
    LOG_CLI((BSL_META_U(unit, " Display table with %d elements\r\n" " ==============================\r\n"), (int) 1));
    SHR_IF_ERR_EXIT(utilex_hash_table_print(unit, hash_table));
    LOG_CLI((BSL_META_U(unit, " \r\n" " ------------------------------\r\n")));
    SHR_IF_ERR_EXIT(utilex_hash_table_destroy(unit, hash_table));
exit:
    if (SHR_FUNC_ERR())
    {
        LOG_CLI((BSL_META_U(unit,
                            " %s(): Exit with error %s\r\n"), __FUNCTION__,
                 shrextend_errmsg_get(SHR_GET_CURRENT_ERR())));
    }
    else
    {
        LOG_CLI((BSL_META_U(unit, " %s(): Exit with no error. All is OK.\r\n"), __FUNCTION__));
    }
    SHR_FUNC_EXIT;
}

/* }*/
#endif /* UTILEX_DEBUG */
/* } */
#else
/* { */
/*
 * This is DUMMY code. It is only for compilers that do not accept empty files
 * and is never to be used.
 */
shr_error_e
utilex_hash_table_info_clear(
    int unit,
    UTILEX_HASH_TABLE_PTR hash_table)
{
    SHR_FUNC_INIT_VARS(unit);
    UTILEX_HASH_TABLE_VERIFY_UNIT_IS_LEGAL(unit);
exit:
    SHR_FUNC_EXIT;
}
/* } */

/* } */
#endif
