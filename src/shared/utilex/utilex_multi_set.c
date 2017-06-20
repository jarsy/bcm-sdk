/*
 * $Id: sand_multi_set.c,v 1.15 Broadcom SDK $
 $Copyright: (c) 2016 Broadcom.
 Broadcom Proprietary and Confidential. All rights reserved.$ 
 */
/** \file utilex_multi_set.c
 *
 * All common utilities related to multi-set of bits.
 *
 * A multi-set is essentially a hash table with control over
 * the number of duplications (reference count) both per member and
 * over the total number of duplications. 
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDSWDNX_UTILSDNX

/*************
* INCLUDES  *
*************/
/*
 * { 
 */
#include <shared/bsl.h>
#include <sal/core/libc.h>
#include <sal/appl/io.h>
#include <assert.h>
#include <shared/shrextend/shrextend_debug.h>
/*
 * SW STATE is included
 */
#include <shared/swstate/access/sw_state_access.h>
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_integer_arithmetic.h>
#include <shared/utilex/utilex_multi_set.h>
/*
 * } 
 */

extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];

/*************
* DEFINES   *
*************/
/*
 * { 
 */
#define UTILEX_MULTI_SET_HASH_WIDTH_FACTOR 1

/*
 * } 
 */

/*************
*  MACROS   *
*************/
/*
 * { 
 */
/*
 * } 
 */

/*************
* TYPE DEFS *
*************/
/*
 * { 
 */

 /*
  * } 
  */

 /*************
 * GLOBALS   *
 *************/
 /*
  * { 
  */

 /*
  * } 
  */

#ifdef BCM_DNX_SUPPORT
/*
 * { 
 */
 /*************
 * FUNCTIONS *
 *************/
 /*
  * { 
  */
/************************************************************************/
/*
 * Internal functions 
 */
/************************************************************************/

/*
 * Get handle to multi set which will be considered illegal
 * by all multi set utilities.
 * Legal values for 'handle' are 1 -> MAX_NOF_MULTIS or, to be more precise:
 * 1 -> max_nof_multis (The value of UTILEX_MULTI_SET_ACCESS.max_nof_multis.get(unit, &max_nof_multis))
 */
uint32
utilex_multi_set_get_illegal_multiset_handle(
    void)
{
    return ((uint32) (-1));
}
/*********************************************************************
* NAME:
*     utilex_multi_set_get_member_size
* TYPE:
*   PROC
* DATE:
*   Mar 26 2015
* FUNCTION:
*   Get element 'member_size' for multi set.
* INPUT:
*   int                           unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR           multi_set -
*     Handle to the multi set.
*   uint32                         *member_size_ptr -
*     This procedure loads pointed memory by the info element 'member_size'.
* REMARKS:
*   For external users nots aware of 'new sw state' structures.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_get_member_size(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    uint32 * member_size_ptr)
{
    uint32 multi_set_index, member_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.member_size.get(unit, multi_set_index, &member_size));
    *member_size_ptr = member_size;
exit:
    SHR_FUNC_EXIT;
}
/*
 * Copy 'len' bytes from indicated 'buffer' at offset 'offset * len'
 * into 'data'.
 */
shr_error_e
utilex_multi_set_default_get_entry(
    int unit,
    uint32 sec_hanlde,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * data)
{
    void *resptr;

    SHR_FUNC_INIT_VARS(unit);

    resptr = sal_memcpy(data, (buffer + (offset * len)), len);
    SHR_NULL_CHECK(resptr, _SHR_E_INTERNAL, "memcpy retval");
exit:
    SHR_FUNC_EXIT;
}
/*
 * Copy 'len' bytes from 'data' into indicated 'buffer' at
 * offset 'offset * len'.
 */
shr_error_e
utilex_multi_set_default_set_entry(
    int unit,
    uint32 sec_hanlde,
    uint8 * buffer,
    uint32 offset,
    uint32 len,
    uint8 * data)
{
    void *resptr;

    SHR_FUNC_INIT_VARS(unit);

    resptr = sal_memcpy((buffer + (offset * len)), data, len);
    SHR_NULL_CHECK(resptr, _SHR_E_INTERNAL, "memcpy retval");
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_member_add_internal
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add nof members to the mutli-set to either a specific index or a new index.
*  If member already exists then update the reference counter of this member.
* INPUT:
*   int                           unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR     multi_set -
*     The multi-set to add a member to.
*   UTILEX_MULTI_SET_KEY        key -
*     The member to add.
*   uint32                       *data_indx -
*     As output:
*       *data_indx is loaded by the place of the added member.
*     As input:
*       If *data_indx is set to UTILEX_U32_MAX then a new location
*       is automatically assigned for the newly added member (and
*       it is loaded into *data_indx as output)
*       Otherwise, this is the location identifying an existing
*       member which should have the same key.
*   uint32                        nof_additions -
*     Declare nof_additions to add the given key.
*     If given UTILEX_U32_MAX, the maximum number of entries will be added.
*     Note:
*       This input, when set to a non-zero value, is only meaningful when
*       'max_duplications' of input multi_set is larger than 1!
*   uint8                        *success -
*     Indicates whether the add operation succeeded. 
*     This may be FALSE,
*       a. If there was no more space available in the multi-set
*       b. If an attempt was make to load more instances (ref_count), on a member, than allowed.
*       c. If 'nof_additions' is zero
*       d. If number of instances per member (ref_count) is '1' and caller has pointed
*          a specific index (via *data_indx) and the same key already exists for enother index.
*       e. Trying to set a ref_count which is too high (larger than number of bits assigned,
*          larger than 'max_duplications' (maximal ref_count per member).
*          Note:
*            This error condition is only applied when 'max_duplications' of input multi_set
*            is larger than 1!
*     Note that if caller is trying to set ref_count larger than 'global_max' (maximal total
*     ref_counter), then an error is returned (_SHR_E_FULL).

* Remarks: 
*     Similar to  utilex_multi_set_member_add_at_index,
*     where nof_additions can be more than 1
*
*     Note that of all 'overflow' conditions, the only case where return value
*     is not OK is when trying to cross 'global_max'. In all other case,
*     *success is loaded by FALSE.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static shr_error_e
utilex_multi_set_member_add_internal(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    UTILEX_MULTI_SET_KEY * const key,
    uint32 * data_indx,
    uint32 nof_additions,
    uint8 * first_appear,
    uint8 * success)
{
    uint8 tmp_cnt[sizeof(uint32)];
    uint32 max_duplications, ref_count[1], counter_size, global_counter, global_max, found_index, adjusted_additions;
    uint8 exist, with_id;
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");
    SHR_NULL_CHECK(data_indx, _SHR_E_INTERNAL, "data_indx");
    SHR_NULL_CHECK(first_appear, _SHR_E_INTERNAL, "first_appear");
    SHR_NULL_CHECK(success, _SHR_E_INTERNAL, "success");
    /*
     * 'found_index' is loaded as indirect output below, so it does not need to be
     * initialized. However, Coverity requires initialization...
     */
    found_index = 0;
    with_id = (*data_indx != UTILEX_U32_MAX);
    if (nof_additions == 0)
    {
        *success = FALSE;
        goto exit;
    }
    /*
     * In case of singlton set, it's enough to just check if the entry exists. 
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    if (max_duplications <= 1)
    {
        SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup(unit, hash_table, key, &found_index, &exist));

        if (with_id)
        {
            /*
             * If caller has explicitly specified the index and entry (key) already exists
             * on a different index, return error. 
             */
            if (exist && found_index != *data_indx)
            {
                *first_appear = FALSE;
                *success = FALSE;
                goto exit;
            }
        }
        else
        {
            /*
             * If caller has NOT explicitly specified the index then keep derived
             * index in '*data_indx'. If this entry (key) already exists, then quit
             * with success (but set *first_appear = FALSE;). See just below.
             */
            *data_indx = found_index;
        }
        /*
         * Apparently, if 'exist' is non-zero then, necessarily, 'found_index'
         * is not UTILEX_HASH_TABLE_NULL.
         */
        if (exist)
        {
            *first_appear = FALSE;
            *success = TRUE;
            goto exit;
        }
    }
    /*
     * Reach this point if
     * a. Either it is not a singleton or
     * b. It is a singleton and it is not in the multi-set yet.
     *
     * If index was given by caller, add this member at at specified index.
     * Otherwise, add using input key and get corresponding index (into 'found_index'). 
     */
    if (with_id)
    {
        found_index = *data_indx;
        /*
         * Note: Procedure utilex_hash_table_entry_add_at_index() reports 'success' on trying to add
         * an existing entry.
         */
        SHR_IF_ERR_EXIT(utilex_hash_table_entry_add_at_index(unit, hash_table, key, found_index, success));
    }
    else
    {
        /*
         * Note: Procedure utilex_hash_table_entry_add() reports 'success' on trying to add
         * an existing entry.
         */
        SHR_IF_ERR_EXIT(utilex_hash_table_entry_add(unit, hash_table, key, &found_index, success));
    }
    /*
     * At this point, member has been added.
     *
     * If it is a singleton, and we didn't find it in the lookup (utilex_hash_table_entry_lookup),
     * then mark it as 'first appearance' and quit.
     * Note that *success is as loaded by 'utilex_hash_table_entry_add()'
     */
    /*
     * Save the index. 
     */
    *data_indx = found_index;
    if (max_duplications <= 1)
    {
        *first_appear = TRUE;
        goto exit;
    }
    /*
     * At this point, member is NOT a singleton and was already attempted to have been
     * added to the hashtable of the multiset.
     *
     * Apparently, if *success is FALSE then 'found_index' must be UTILEX_HASH_TABLE_NULL
     * and the other way round.
     */
    if (!(*success))
    {
        /*
         * Enter if *success is FALSE
         *
         * If we were given the index, we assume the user allocated the 
         * entry before the call. If we weren't given the index, we
         * assume the operation failed because the table is full.
         */
        *first_appear = (with_id) ? FALSE : TRUE;
        goto exit;
    }
    /*
     * At this point, member is NOT a singleton and was successfully
     * added to the hashtable of the multiset.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));
    /*
     * Copy 'counter_size' bytes from 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
     * '(*data_indx) * counter_size' into 'tmp_cnt' buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                    memread(unit, multi_set_index, tmp_cnt, (*data_indx) * counter_size, counter_size));
    *ref_count = 0;
    SHR_IF_ERR_EXIT(utilex_U8_to_U32(tmp_cnt, counter_size, ref_count));
    /*
     * Set if maximum entries is required. 
     */
    if (nof_additions == UTILEX_U32_MAX)
    {
        adjusted_additions = (max_duplications - (*ref_count));
    }
    else
    {
        adjusted_additions = nof_additions;
    }
    /*
     * Same value referenced more than initialized ed value times, return
     * operation fail.
     */
    if (((*ref_count) + adjusted_additions) > UTILEX_BITS_MASK((UTILEX_NOF_BITS_IN_BYTE * counter_size - 1), 0))
    {
        /*
         * Overflowing the counter. 
         */
        *success = FALSE;
        goto exit;
    }
    if (((*ref_count) + adjusted_additions) > max_duplications)
    {   /* Adding more entries than there are available */
        *success = FALSE;
        goto exit;
    }
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.global_counter.get(unit, multi_set_index, &global_counter));
    /*
     * Just to keep coverity happy...
     */
    global_max = 0;
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.global_max.get(unit, multi_set_index, &global_max));

    if ((int) global_max > 0)
    {
        if ((global_counter + adjusted_additions) > global_max)
        {
            /*
             * Adding more entries than the global counter allows. 
             */
            SHR_IF_ERR_EXIT(_SHR_E_FULL);
        }
    }
    if (*ref_count == 0)
    {
        *first_appear = TRUE;
    }
    else
    {
        *first_appear = FALSE;
    }
    *ref_count += adjusted_additions;

    /*
     * Update global counter. 
     */
    global_counter += adjusted_additions;
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter));

    SHR_IF_ERR_EXIT(utilex_U32_to_U8(ref_count, counter_size, tmp_cnt));

    /*
     * Copy 'counter_size' bytes from 'tmp_cnt' buffer into 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter'
     * buffer at offset '(*data_indx) * counter_size'.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                    memwrite(unit, multi_set_index, tmp_cnt, (*data_indx) * counter_size, counter_size));

exit:
    SHR_FUNC_EXIT;
}
/*********************************************************************
* NAME:
*   utilex_multi_set_init
* TYPE:
*   PROC
* DATE:
*   May 19 2015
* FUNCTION:
*   Initialize control structure for ALL multi set instances expected.
* INPUT:
*   int unit -
*     Identifier of the device to access.
*   uint32 max_nof_multis -
*     Maximal number of multi sets which can be sustained simultaneously.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_init(
    int unit,
    uint32 max_nof_multis)
{
    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.alloc(unit));
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.multis_array.ptr_alloc(unit, max_nof_multis));
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.max_nof_multis.set(unit, max_nof_multis));
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.in_use.set(unit, 0));
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.occupied_multis.alloc_bitmap(unit, max_nof_multis));
    /*
     * If it is required to run the unit test (utilex_multi_set_test_1(unit)) then
     * this is a good place to invoke it. 
     */
exit:
    SHR_FUNC_EXIT;
}
/*********************************************************************
* NAME:
*     utilex_multi_set_create
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     Creates a new Multi set instance.
* INPUT:
*   int                           unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR        *multi_set -
*     This procedure loads pointed memory by handle of the newly created
*     multi set.
*   UTILEX_MULTI_SET_INIT_INFO  init_info -
*     Information to use in order to create the mutli-set (size, duplications...)
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_create(
    int unit,
    UTILEX_MULTI_SET_PTR * multi_set_ptr,
    UTILEX_MULTI_SET_INIT_INFO init_info)
{
    UTILEX_HASH_TABLE_INIT_INFO hash_init;
    UTILEX_MULTI_SET_PTR multi_set;
    uint32 multi_set_index, max_duplications, found, max_nof_multis;
    uint32 in_use;
    uint8 bit_val;
    int prime_handle;
    uint32 member_size, sec_handle, nof_members;
    UTILEX_MULTISET_SW_DB_ENTRY_GET get_entry_fun;
    UTILEX_MULTISET_SW_DB_ENTRY_SET set_entry_fun;
    uint32 counter_size, global_counter;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_NULL_CHECK(multi_set_ptr, _SHR_E_INTERNAL, "multi_set_ptr");

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.in_use.get(unit, &in_use));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.max_nof_multis.get(unit, &max_nof_multis));

    if (in_use >= max_nof_multis)
    {
        /*
         * If number of occupied bitmap structures is beyond the
         * maximum then quit with error.
         */
        SHR_IF_ERR_EXIT(_SHR_E_MEMORY);
    }
    /*
     * Increment number of 'in_use' to cover the one we now intend to capture.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.in_use.set(unit, (in_use + 1)));

    /*
     * Find a free multi (a cleared bit in 'occupied_multis'). At this point,
     * there must be one.
     */
    found = 0;
    for (multi_set_index = 0; multi_set_index < max_nof_multis; multi_set_index++)
    {
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.occupied_multis.bit_get(unit, multi_set_index, &bit_val));

        if (bit_val == 0)
        {
            /*
             * 'multi_set_index' is now the index of a free entry.
             */
            found = 1;
            break;
        }
    }
    if (!found)
    {
        SHR_IF_ERR_EXIT(_SHR_E_MEMORY);
    }
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.occupied_multis.bit_set(unit, multi_set_index));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.multis_array.alloc(unit, multi_set_index));

    /*
     * Note that legal handles start at '1', not at '0'.
     */
    UTILEX_MULTI_SET_CONVERT_MULTISET_INDEX_TO_HANDLE(multi_set, multi_set_index);
    /*
     * Set output of this procedure.
     */
    *multi_set_ptr = multi_set;
    /*
     * Load 'init' section of multi set as per input.
     * Make sure all callbacks are NULL - Currently, we only use defaults
     */
    if (init_info.get_entry_fun != NULL || init_info.set_entry_fun != NULL)
    {
        SHR_IF_ERR_EXIT(_SHR_E_MEMORY);
    }
    if (init_info.nof_members == 0 || init_info.member_size == 0)
    {
        SHR_IF_ERR_EXIT(_SHR_E_MEMORY);
    }
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.set(unit, multi_set_index, &init_info));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    if (max_duplications == UTILEX_U32_MAX)
    {
        max_duplications = UTILEX_U32_MAX - 1;
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.set(unit, multi_set_index, max_duplications));

    }
    counter_size = 0;
    hash_table = utilex_hash_table_get_illegal_hashtable_handle();

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.set(unit, multi_set_index, counter_size));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.set(unit, multi_set_index, hash_table));

    /*
     * create hash-table.
     */
    sal_memset(&hash_init, 0x0, sizeof(UTILEX_HASH_TABLE_INIT_INFO));
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.member_size.get(unit, multi_set_index, &member_size));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.prime_handle.get(unit, multi_set_index, &prime_handle));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.sec_handle.get(unit, multi_set_index, &sec_handle));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.get_entry_fun.get(unit, multi_set_index, &get_entry_fun));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.set_entry_fun.get(unit, multi_set_index, &set_entry_fun));

    /*
     * Note: We take input 'init_info.global_max' as is unless it is zero (in
     * which case we ignore it).
     */

    hash_init.data_size = member_size;
    hash_init.key_size = member_size;
    hash_init.prime_handle = prime_handle;
    hash_init.sec_handle = sec_handle;
    hash_init.table_size = nof_members;
    hash_init.table_width = nof_members * UTILEX_MULTI_SET_HASH_WIDTH_FACTOR;
    hash_init.get_entry_fun = get_entry_fun;
    hash_init.set_entry_fun = set_entry_fun;

    global_counter = 0;
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter));

    SHR_IF_ERR_EXIT(utilex_hash_table_create(unit, &hash_table, hash_init));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.set(unit, multi_set_index, hash_table));

    /*
     * If there is need to manage the duplications, then allocate array to
     * manage the reference counter.
     */
    if (max_duplications > 1)
    {
        /*
         * calculate the size of pointers (list head and next) according to table size.
         */
        counter_size =
            (utilex_log2_round_up(max_duplications + 1) + (UTILEX_NOF_BITS_IN_BYTE - 1)) / UTILEX_NOF_BITS_IN_BYTE;
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.set(unit, multi_set_index, counter_size));

        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        alloc(unit, multi_set_index, nof_members * counter_size));

        /*
         * Fill 'ref_counter' by zeroes starting at offset '0'.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memset(unit, multi_set_index, 0, nof_members * counter_size, 0x00));

    }

exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_destroy
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*     free the mutli-set instance.
* INPUT:
*  int                        unit -
*     Identifier of the device to access.
*  UTILEX_MULTI_SET_PTR     multi_set -
*    Handle to the mutli-set to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_destroy(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set)
{
    uint32 multi_set_index;
    uint8 bit_val;
    uint32 in_use;
    UTILEX_HASH_TABLE_PTR hash_table;
    uint32 max_duplications, global_counter;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);
    /*
     * First, mark this multi set as 'released'
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.in_use.get(unit, &in_use));

    if ((int) in_use < 0)
    {
        /*
         * If number of occupied multiset structures goes below zero then quit
         * with error.
         */
        SHR_IF_ERR_EXIT(_SHR_E_MEMORY);
    }
    /*
     * Decrement number of 'in_use' to cover the one we now intend to release.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.in_use.set(unit, (in_use - 1)));

    /*
     * Mark specific multi set as 'not occupied'
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.occupied_multis.bit_get(unit, multi_set_index, &bit_val));

    if (bit_val == 0)
    {
        /*
         * If multi set structure is not indicated as 'occupied' then quit
         * with error.
         */
        SHR_IF_ERR_EXIT(_SHR_E_MEMORY);
    }
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.occupied_multis.bit_clear(unit, multi_set_index));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    SHR_IF_ERR_EXIT(utilex_hash_table_destroy(unit, hash_table));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    /*
     * if there is need to manage the duplications, then allocate array to
     * manage the reference counter.
     */
    if (max_duplications > 1)
    {
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.free(unit, multi_set_index));

    }
    global_counter = 0;
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS.multis_array.free(unit, multi_set_index));

exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_member_add
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   Add a member to the mutli-set, if already exist then
*   then update the occurrences of this member.
* INPUT:
*   int                           unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR        multi_set -
*    Handle to the mutli-set to add a member to.
*   UTILEX_MULTI_SET_KEY        key -
*     The member to add.
*   uint32                       *data_indx -
*     Index identifies the place of the added member.
*     the given key. Output only. This procedure initially
*     loads pointed memory by UTILEX_U32_MAX.
*   uint8                        *success -
*     Indicates whether the add operation succeeded.
*     This may be FALSE.
*     See list of options for failed 'success' on documentation of
*     utilex_multi_set_member_add_internal()
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_member_add(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    UTILEX_MULTI_SET_KEY * const key,
    uint32 * data_indx,
    uint8 * first_appear,
    uint8 * success)
{
    int nof_additions;
    uint32 multi_set_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    nof_additions = 1;
    /*
     * In this function, the data index is output only.
     * Set to UTILEX_U32_MAX to indicate that.
     */
    *data_indx = UTILEX_U32_MAX ;
    SHR_IF_ERR_EXIT(utilex_multi_set_member_add_internal(unit,
                                                         multi_set,
                                                         key, data_indx, nof_additions, first_appear, success));

exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_member_add_at_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add one member to the mutli-set to a specific index, if already exist then
*  then update the occurrences of this member.
* INPUT:
*   int                           unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR        multi_set -
*     The multi-set to add a member to.
*   UTILEX_MULTI_SET_KEY        key -
*     The member to add.
*   uint32                       *data_indx -
*     Index identifies the place of the added member.
*     The function assumes that it already exists. 
*   uint8                        *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* Remarks: 
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_member_add_at_index(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    UTILEX_MULTI_SET_KEY * const key,
    uint32 data_indx,
    uint8 * first_appear,
    uint8 * success)
{
    uint32 multi_set_index;
    const int nof_additions = 1;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index_nof_additions(unit,
                                                                       multi_set,
                                                                       key,
                                                                       data_indx, nof_additions, first_appear,
                                                                       success));

exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_member_add_at_index_nof_additions
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Add nof members to the mutli-set to a specific index, if already exist then
*  then update the occurrences of this member.
* INPUT:
*   int                           unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR        multi_set -
*     The multi-set to add a member to.
*   UTILEX_MULTI_SET_KEY        key -
*     The member to add.
*   uint32                       *data_indx -
*     Index identifies the place of the added member.
*     The function assumes that it already exists. 
*   uint32                        nof_additions -
*     Declare nof_additions to add the given key.
*     If given UTILEX_U32_MAX, the maximum number of entries will be added.
*   uint8                        *success -
*     whether the add operation success, this may be FALSE,
*     if there are no more space available in the multi-set
* Remarks: 
*     Similar to  utilex_multi_set_member_add_at_index,
*     where nof_additions can be more than 1
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_member_add_at_index_nof_additions(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    UTILEX_MULTI_SET_KEY * const key,
    uint32 data_indx,
    uint32 nof_additions,
    uint8 * first_appear,
    uint8 * success)
{
    uint32 multi_set_index;
    uint32 data_index_ptr;
    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    data_index_ptr = data_indx;
    SHR_IF_ERR_EXIT(utilex_multi_set_member_add_internal(unit,
                                                         multi_set,
                                                         key, &data_index_ptr, nof_additions, first_appear, success));

exit:
    SHR_FUNC_EXIT;
}


/*********************************************************************
* NAME:
*     utilex_multi_set_member_remove
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Remove a member from a mutli-set, if the member is not belong to the multi-set
*  the operation has no effect.
* INPUT:
*  int                           unit -
*     Identifier of the device to access.
*  UTILEX_MULTI_SET_PTR        multi_set -
*     The mutli-set instance.
*  UTILEX_MULTI_SET_KEY        *const key -
*     The key to remove from the mutli-set
*  uint32                       *data_indx -
*     where the key was found.
*  uint8                        *last_appear -
*     whether this was the last occurrence of this key in the multiset.
*     so no more occurrences of this key after this remove.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_member_remove(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    UTILEX_MULTI_SET_KEY * const key,
    uint32 * data_indx,
    uint8 * last_appear)
{
    uint8 found;
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup(unit, hash_table, key, data_indx, &found));

    if (!found)
    {
        *data_indx = UTILEX_MULTI_SET_NULL;
        *last_appear = FALSE;
        goto exit;
    }
    SHR_IF_ERR_EXIT(utilex_multi_set_member_remove_by_index_multiple(unit, multi_set, *data_indx, 1, last_appear));

exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_member_remove_by_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Remove a member from a mutli-set, if the member is not belong to the multi-set
*  the operation has no effect.
* INPUT:
*  int                         unit -
*    Identifier of the device to access.
*  UTILEX_MULTI_SET_PTR      multi_set -
*    The mutli-set instance.
*  uint32                      data_indx - 
*    The index of the entry to be deleted.
*  uint8                      *last_appear -
*    Will be set if it's the last appearance of the entry. 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_member_remove_by_index(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    uint32 data_indx,
    uint8 * last_appear)
{
    uint32 multi_set_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);
    SHR_IF_ERR_EXIT(utilex_multi_set_member_remove_by_index_multiple(unit, multi_set, data_indx, 1, last_appear));

exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_member_remove_by_index_multiple
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  Remove a member from a mutli-set, if the member is not belong to the multi-set
*  the operation has no effect.
* INPUT:
*  int                           unit -
*     Identifier of the device to access.
*  UTILEX_MULTI_SET_PTR        multi_set -
*     The mutli-set instance.
*  uint32                        data_indx - 
*    The index of the entry to be deleted.
*  uint32                        remove_amount -
*    How many references of the entry to be removed. 
*  uint8                        *last_appear -
*    Will be set if it's the last appearance of the entry. 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_member_remove_by_index_multiple(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    uint32 data_indx,
    uint32 remove_amount,
    uint8 * last_appear)
{
    uint32 max_duplications, counter_size, global_counter, ref_count, adjusted_remove_amount;
    uint8 tmp_cnt[sizeof(uint32)];
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_NULL_CHECK(last_appear, _SHR_E_INTERNAL, "last_appear");
    ref_count = 0;
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));

    if (max_duplications > 1)
    {
        /*
         * Copy 'counter_size' bytes from 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
         * 'data_indx * counter_size' into 'tmp_cnt' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memread(unit, multi_set_index, tmp_cnt, data_indx * counter_size, counter_size));

        utilex_U8_to_U32(tmp_cnt, counter_size, &ref_count);
        if (ref_count == 0)
        {
            *last_appear = TRUE;
            goto exit;
        }

        /*
         * Remove all entries 
         */
        if ((remove_amount > ref_count))
        {
            adjusted_remove_amount = ref_count;
        }
        else
        {
            adjusted_remove_amount = remove_amount;
        }
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.global_counter.get(unit, multi_set_index, &global_counter));

        global_counter -= adjusted_remove_amount;
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.global_counter.set(unit, multi_set_index, global_counter));

        ref_count -= adjusted_remove_amount;
        *last_appear = (ref_count == 0) ? TRUE : FALSE;
        SHR_IF_ERR_EXIT(utilex_U32_to_U8(&ref_count, counter_size, tmp_cnt));

        /*
         * Copy 'counter_size' bytes from 'tmp_cnt' buffer into 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter'
         * buffer at offset 'data_indx * counter_size'.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memwrite(unit, multi_set_index, tmp_cnt, data_indx * counter_size, counter_size));

    }
    if (*last_appear || max_duplications <= 1)
    {
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

        SHR_IF_ERR_EXIT(utilex_hash_table_entry_remove_by_index(unit, hash_table, data_indx));

    }
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*   utilex_multi_set_member_lookup
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   Lookup in the mutli-set for a member and return the occurrences/duplications of this
*   member in the multi-set, and the index identifying this member place.
*   the given key.
* INPUT:
*   int                         unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR      multi_set -
*     The mutli-set.
*   UTILEX_MULTI_SET_KEY      key -
*     The member to lookup
*   uint32                     *data_indx -
*     index identifying this member place.
*   uint32                     *ref_count -
*     the occurrences/duplications of this member in the multi-set
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_member_lookup(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    UTILEX_MULTI_SET_KEY * const key,
    uint32 * data_indx,
    uint32 * ref_count)
{
    uint8 tmp_cnt[sizeof(uint32)];
    uint8 found;
    uint32 ref_count_lcl[1];
    uint32 max_duplications, counter_size;
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");
    SHR_NULL_CHECK(data_indx, _SHR_E_INTERNAL, "data_indx");

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup(unit, hash_table, key, data_indx, &found));

    if (!found)
    {
        *ref_count = 0;
        goto exit;
    }
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    if (max_duplications > 1)
    {
        *ref_count_lcl = 0;
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));

        /*
         * Copy 'counter_size' bytes from 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
         * '(*data_indx) * counter_size' into 'tmp_cnt' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memread(unit, multi_set_index, tmp_cnt, (*data_indx) * counter_size, counter_size));

        SHR_IF_ERR_EXIT(utilex_U8_to_U32(tmp_cnt, counter_size, ref_count_lcl));

        *ref_count = *ref_count_lcl;
    }
    else
    {
        *ref_count = 1;
    }
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_get_next
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  get the next valid entry (key and data) in the multiset.
*  start traversing from the place pointed by the given iterator.
* INPUT:
*  int                           unit -
*     Identifier of the device to access.
*  UTILEX_MULTI_SET_PTR        multi_set -
*     The multiset.
*  UTILEX_MULTI_SET_ITER    *iter
*     iterator points to the entry to start traverse from.
*   UTILEX_MULTI_SET_KEY*     const key -
*     the multiset key returned
*   UTILEX_MULTI_SET_DATA     data -
*     the multiset data returned and associated with the key above.
* REMARKS:
*     - to start traverse the multiset from the beginning.
*       use UTILEX_MULTI_SET_ITER_BEGIN(iter)
*     - to check if the iterator get to the end of the table use.
*       use UTILEX_MULTI_SET_ITER_END(iter)
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_get_next(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    UTILEX_MULTI_SET_ITER * iter,
    UTILEX_MULTI_SET_KEY * key,
    uint32 * data_indx,
    uint32 * ref_count)
{
    uint32 counter_size, max_duplications, tmp_ref_count;
    uint8 tmp_cnt[sizeof(uint32)];
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);
    /*
     * traverse the mutli-set and print
     */
    if (UTILEX_HASH_TABLE_ITER_IS_END(iter))
    {
        goto exit;
    }
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    SHR_IF_ERR_EXIT(utilex_hash_table_get_next(unit, hash_table, iter, key, data_indx));

    if (UTILEX_HASH_TABLE_ITER_IS_END(iter))
    {
        goto exit;
    }
    tmp_ref_count = 1;
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    if (max_duplications > 1)
    {
        tmp_ref_count = 0;
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));

        /*
         * Copy 'counter_size' bytes from 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
         * '(*data_indx) * counter_size' into 'tmp_cnt' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memread(unit, multi_set_index, tmp_cnt, (*data_indx) * counter_size, counter_size));

        SHR_IF_ERR_EXIT(utilex_U8_to_U32(tmp_cnt, counter_size, &tmp_ref_count));

    }
    *ref_count = tmp_ref_count;
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_multi_set_get_by_index
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*  get the next valid entry (key and data) in the multiset.
*  start traversing from the place pointed by the given iterator.
* INPUT:
*  int                       unit -
*     Identifier of the device to access.
*  UTILEX_MULTI_SET_PTR    multi_set -
*     The multiset.
*  uint32                    data_indx -
*     where the key was found.
*   UTILEX_MULTI_SET_KEY* key -
*     the multiset key returned
*   UTILEX_MULTI_SET_DATA data -
*     the multiset data returned and associated with the key above.
* REMARKS:
*     - to start traverse the multiset from the beginning.
*       use UTILEX_MULTI_SET_ITER_BEGIN(iter)
*     - to check if the iterator get to the end of the table use.
*       use UTILEX_MULTI_SET_ITER_END(iter)
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_get_by_index(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    uint32 data_indx,
    UTILEX_MULTI_SET_KEY * key,
    uint32 * ref_count)
{
    uint32 counter_size, max_duplications, tmp_ref_count;
    uint8 found;
    uint8 tmp_cnt[sizeof(uint32)];
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    SHR_IF_ERR_EXIT(utilex_hash_table_get_by_index(unit, hash_table, data_indx, key, &found));

    if (!found)
    {
        *ref_count = 0;
        goto exit;
    }
    tmp_ref_count = 1;
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    if (max_duplications > 1)
    {
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));

        tmp_ref_count = 0;
        /*
         * Copy 'counter_size' bytes from 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
         * 'data_indx * counter_size' into 'tmp_cnt' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memread(unit, multi_set_index, tmp_cnt, data_indx * counter_size, counter_size));

        SHR_IF_ERR_EXIT(utilex_U8_to_U32(tmp_cnt, counter_size, &tmp_ref_count));

    }
    *ref_count = tmp_ref_count;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_multi_set_clear(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set)
{
    uint32 nof_members, counter_size, max_duplications;
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    /*
     * clear mutli-set.
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_clear(unit, hash_table));

    /*
     * if there is need to manage the duplications, then clear array to
     * manage the reference counter.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    if (max_duplications > 1)
    {
        /*
         * calculate the size of pointers (list head and next) according to table size.
         */
        counter_size =
            (utilex_log2_round_up(max_duplications + 1) + (UTILEX_NOF_BITS_IN_BYTE - 1)) / UTILEX_NOF_BITS_IN_BYTE;
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.set(unit, multi_set_index, counter_size));

        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members));

        /*
         * Fill 'ref_counter' buffer by zeroes
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memset(unit, multi_set_index, 0, nof_members * counter_size, 0x00));

    }

exit:
    SHR_FUNC_EXIT;
}

#ifdef UTILEX_DEBUG
/*
 * { 
 */
/*
 *
 */
shr_error_e
utilex_multi_set_test_1(
    int unit)
{
    UTILEX_MULTI_SET_PTR multi_set ;
    UTILEX_MULTI_SET_INIT_INFO init_info ;
    uint32 ii, jj ;
    uint8 clear_on_print ;
    uint32 num_members_for_test ;
    uint32 test_num ;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    {
        test_num = 1 ;
        /*
         * Test no. 1:
         * 'max_duplications' is set greater than '1'
         * Add 'num_members_for_test' members. Do it to the maximum set for
         * ref_counter per member (max_duplications) and to the maximum count allowed for
         * the whole multi-set (global_max).
         */
        /*
         * Number of members to add to multiset for testing.
         */
        num_members_for_test = 4 ;
        /*
         * handle of the multiset to identify the multiset instance. Not used in this implementation.
         */
        init_info.prime_handle = 0 ;
        /*
         * handle of the multiset to identify the multiset instance. Not used in this implementation.
         */
        init_info.sec_handle = 0 ;
        /*
         * Number of different elements that can be inserted to the set.
         */
        init_info.nof_members = 10 ;
        /*
         * size of the member (in bytes)
         */
        init_info.member_size = 4 ;
        /*
         * The maximum duplications/occurrences of a value in the multi_set.
         * if 1  then the multi_set act as Set i.e.
         * no matter how many time an element was added
         * one remove operation will remove it from the set.
         * otherwise the member is not removed till
         * all the add operation reverted with remove operation
         * i.e. # removes = # add. if a value added more than this
         * number the operation fail.
         */
        init_info.max_duplications = 3 ;
        /*
         * The maximum number of duplications/occurrences of all values in the multi_set. 
         */
        init_info.global_max = num_members_for_test * init_info.max_duplications ;
        /*
         * Get/set procedures. Must be set to NULL in this implementation.
         */
        init_info.get_entry_fun = NULL ;
        init_info.set_entry_fun = NULL ;
        SHR_IF_ERR_EXIT(utilex_multi_set_create(unit,&multi_set,init_info)) ;
        for (jj = 0 ; jj < init_info.max_duplications ; jj++)
        {
            for (ii = 1 ; ii < (num_members_for_test + 1) ; ii++)
            {
                UTILEX_MULTI_SET_KEY key[4] ;
                uint32 data_indx ;
                uint8 first_appear ;
                uint8 success ;
                uint32 member_size ;
    
                SHR_IF_ERR_EXIT(utilex_multi_set_get_member_size(unit,multi_set,&member_size)) ;
                /*
                 * Member size must be the size of the key on the hash table.
                 */
                if (sizeof(key) != member_size)
                {
                    SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
                    SHR_EXIT() ;
                }
                /*
                 * *data_indx is output only so there is no need to load it.
                 */
                sal_memset(key,0,sizeof(key)) ;
                key[0] = 100 ;
                key[1] = ii ;
                SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success)) ;
                LOG_CLI((BSL_META_U(unit,
                    " %s(): %d : data_indx %d first_appear %d success %d\r\n"), __FUNCTION__,ii,data_indx,(int)first_appear,(int)success)) ;
            }
            clear_on_print = 0 ;
            SHR_IF_ERR_EXIT(utilex_multi_set_print(unit,multi_set,clear_on_print)) ;
        }
        SHR_IF_ERR_EXIT(utilex_multi_set_destroy(unit,multi_set)) ;
        LOG_CLI((BSL_META_U(unit," %s(): Test %d - OK!\r\n"), __FUNCTION__,test_num)) ;
    }
    {
        /*
         * Test no. 2:
         * 'max_duplications' is set greater than '1'
         * Add one member and increase ref_count to the maximum allowed.
         * Then, try to increase further and get error.
         */
        UTILEX_MULTI_SET_KEY key[4] ;
        UTILEX_MULTI_SET_KEY ref_key[4] ;
        uint32 ii ;
        uint32 data_indx ;
        uint8 first_appear ;
        uint8 success ;
        uint32 member_size ;
        uint32 nof_additions ;
        uint32 ref_count ;
        shr_error_e res ;

        test_num = 2 ;
        init_info.prime_handle = 0 ;
        init_info.sec_handle = 0 ;
        init_info.nof_members = 2 ;
        init_info.member_size = 4 ;
        init_info.max_duplications = 3 ;
        init_info.global_max = init_info.nof_members * init_info.max_duplications - 1 ;
        init_info.get_entry_fun = NULL ;
        init_info.set_entry_fun = NULL ;
        SHR_IF_ERR_EXIT(utilex_multi_set_create(unit,&multi_set,init_info)) ;
        SHR_IF_ERR_EXIT(utilex_multi_set_get_member_size(unit,multi_set,&member_size)) ;
        /*
         * Member size must be the size of the key on the hash table.
         */
        if (sizeof(key) != member_size)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            SHR_EXIT() ;
        }
        if (sizeof(ref_key) != member_size)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            SHR_EXIT() ;
        }
        sal_memset(key,0,sizeof(key)) ;
        key[0] = 64 ;
        key[1] = 10 ;
        key[2] = 9 ;
        key[3] = 8 ;
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success)) ;
        LOG_CLI((BSL_META_U(unit,
            " %s(): multi_set %d data_indx %d first_appear %d success %d\r\n"), __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Failed to add first member to multiset %d\r\n"), __FUNCTION__,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        /*
         * Increase ref_counter by '1' (by adding the same member again)
         */
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index(unit,multi_set,key,data_indx,&first_appear,&success)) ;
        LOG_CLI((BSL_META_U(unit,
            " %s(): multi_set %d data_indx %d first_appear %d success %d\r\n"),
            __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Failed to add second instance (ref_counter) to member data_indx (%d) on multiset %d\r\n"), __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        if (first_appear)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Add second instance to member %d on multiset %d but 'first_appear' is non-zero!\r\n"), __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        /*
         * Increase ref_counter to 'max_duplications' (by setting 'nof_additions' to UTILEX_U32_MAX)
         */
        nof_additions = UTILEX_U32_MAX ;
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index_nof_additions(unit,multi_set,key,data_indx,nof_additions,&first_appear,&success)) ;
        LOG_CLI((BSL_META_U(unit,
            " %s(): multi_set %d data_indx %d first_appear %d success %d\r\n"),
            __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Failed to add max instances (set ref_counter to max_duplications) to member data_indx (%d) on multiset %d\r\n"),
                __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        if (first_appear)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Add max instances to member %d on multiset %d but 'first_appear' is non-zero!\r\n"), __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        SHR_IF_ERR_EXIT(utilex_multi_set_get_by_index(unit,multi_set,data_indx,ref_key,&ref_count)) ;
        if (ref_count != init_info.max_duplications)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Member %d on multiset %d has ref_count %d which it should be %d!\r\n"),
                __FUNCTION__,data_indx,(int)multi_set,(int)ref_count,(int)(init_info.max_duplications))) ;
            SHR_EXIT() ;
        }
        if (sal_memcmp(key,ref_key,member_size) != 0)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Member %d on multiset %d has wrong key!\r\n"),
                __FUNCTION__,data_indx,(int)multi_set)) ;
            LOG_CLI((BSL_META_U(unit,"\r\n"))) ;
            LOG_CLI((BSL_META_U(unit,"Key found for member %d: 0x"),data_indx)) ;
            for (ii = 0 ; ii < member_size ; ii++)
            {
                LOG_CLI((BSL_META_U(unit,"%02X"),ref_key[ii])) ;
            }
            LOG_CLI((BSL_META_U(unit,"\r\n"))) ;
            SHR_EXIT() ;
        }
        /*
         * Now try to add one to reference counter (ref_count). Should fail on 'success'!
         */
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index(unit,multi_set,key,data_indx,&first_appear,&success)) ;
        if (success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Succeeded to add one to ref_counter although it should be alreday larger thabn max_duplications: member(%d) on multiset %d\r\n"),
                __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        /*
         * Now try to increase the total ref_counter beyond the maximum allowed.
         * Should fail on error on ret_val.
         */
        sal_memset(key,0,sizeof(key)) ;
        key[0] = 64 ;
        key[1] = 10 ;
        key[2] = 9 ;
        key[3] = 6 ;
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success)) ;
        LOG_CLI((BSL_META_U(unit,
            " %s(): multi_set %d data_indx %d first_appear %d success %d\r\n"), __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Failed to add second member to multiset %d\r\n"), __FUNCTION__,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        /*
         * Increase ref_counter to 'max_duplications' (by setting 'nof_additions' to UTILEX_U32_MAX).
         * This will make the global count of all ref_counters larger than max allowed (global_max).
         */
        nof_additions = UTILEX_U32_MAX ;
        res = utilex_multi_set_member_add_at_index_nof_additions(unit,multi_set,key,data_indx,nof_additions,&first_appear,&success) ;
        if (res == _SHR_E_NONE)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                " %s(): Succeeded to increase total ref_count to a value larger than max (%d)\r\n"), __FUNCTION__,(int)(init_info.global_max))) ;
            SHR_EXIT() ;
        }
        clear_on_print = 0 ;
        SHR_IF_ERR_EXIT(utilex_multi_set_print(unit,multi_set,clear_on_print)) ;
        SHR_IF_ERR_EXIT(utilex_multi_set_destroy(unit,multi_set)) ;
        LOG_CLI((BSL_META_U(unit," %s(): Test %d - OK!\r\n"), __FUNCTION__,test_num)) ;
    }
    {
        /*
         * Test no. 3:
         * 'max_duplications' is set to '1'
         * Add one member a few times. Should not fail even beyond 'max_duplications'.
         * Then, add new members over the number of assigned member count and get error.
         */
        UTILEX_MULTI_SET_KEY key[4] ;
        UTILEX_MULTI_SET_KEY ref_key[4] ;
        uint32 ii ;
        uint32 data_indx ;
        uint8 first_appear ;
        uint8 success ;
        uint32 member_size ;
        uint32 ref_count ;
        shr_error_e res ;

        test_num = 3 ;
        init_info.prime_handle = 0 ;
        init_info.sec_handle = 0 ;
        init_info.nof_members = 1 ;
        init_info.member_size = 4 ;
        init_info.max_duplications = 1 ;
        init_info.global_max = 0 ;
        init_info.get_entry_fun = NULL ;
        init_info.set_entry_fun = NULL ;
        SHR_IF_ERR_EXIT(utilex_multi_set_create(unit,&multi_set,init_info)) ;
        SHR_IF_ERR_EXIT(utilex_multi_set_get_member_size(unit,multi_set,&member_size)) ;
        /*
         * Member size must be the size of the key on the hash table.
         */
        if (sizeof(key) != member_size)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            SHR_EXIT() ;
        }
        if (sizeof(ref_key) != member_size)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            SHR_EXIT() ;
        }
        sal_memset(key,0,sizeof(key)) ;
        key[0] = 64 ;
        key[1] = 10 ;
        key[2] = 9 ;
        key[3] = 8 ;
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success)) ;
        LOG_CLI((BSL_META_U(unit,
            "%s(): multi_set %d data_indx %d first_appear %d success %d\r\n"), __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Failed to add first member to multiset %d\r\n"), __FUNCTION__,(int)multi_set)) ;
            SHR_EXIT() ;
	}
        /*
         * Add the same member again. Should succeed.
         */
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success)) ;
        LOG_CLI((BSL_META_U(unit,
            "%s(): multi_set %d data_indx %d first_appear %d success %d\r\n"),
            __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Failed to add the same member again. data_indx (%d) on multiset %d\r\n"), __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        if (first_appear)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Add same member %d on multiset %d but 'first_appear' is non-zero!\r\n"), __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        SHR_IF_ERR_EXIT(utilex_multi_set_get_by_index(unit,multi_set,data_indx,ref_key,&ref_count)) ;
        if (ref_count != init_info.max_duplications)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Member %d on multiset %d has ref_count %d which it should be %d!\r\n"),
                __FUNCTION__,data_indx,(int)multi_set,(int)ref_count,(int)(init_info.max_duplications))) ;
            SHR_EXIT() ;
        }
        if (sal_memcmp(key,ref_key,member_size) != 0)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Member %d on multiset %d has wrong key!\r\n"),
                __FUNCTION__,data_indx,(int)multi_set)) ;
            LOG_CLI((BSL_META_U(unit,"\r\n"))) ;
            LOG_CLI((BSL_META_U(unit,"Key found for member %d: 0x"),data_indx)) ;
            for (ii = 0 ; ii < member_size ; ii++)
            {
                LOG_CLI((BSL_META_U(unit,"%02X"),ref_key[ii])) ;
            }
            LOG_CLI((BSL_META_U(unit,"\r\n"))) ;
            SHR_EXIT() ;
        }
        /*
         * Now try to add one member. This is beyond assigned member count. Should fail on 'success'!
         *
         * Should NOT fail on error on ret_val.
         */
        sal_memset(key,0,sizeof(key)) ;
        key[0] = 64 ;
        key[1] = 10 ;
        key[2] = 9 ;
        key[3] = 6 ;
        res = utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success) ;
        LOG_CLI((BSL_META_U(unit,
            "%s(): multi_set %d data_indx %d first_appear %d success %d\r\n"),
             __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (res != _SHR_E_NONE)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Attempt to increase member count on multiset %d beyond max (%d) failed on ret val %d. NOT OK. Quit.\r\n"),
                __FUNCTION__,(int)multi_set,(int)(init_info.nof_members),(int)res)) ;
            SHR_EXIT() ;
        }
        if (success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Succedded to add member to multiset %d beyond max (%d). NOT OK. Quit.\r\n"),
                __FUNCTION__,(int)multi_set,(int)(init_info.nof_members))) ;
            SHR_EXIT() ;
        }
        clear_on_print = 0 ;
        SHR_IF_ERR_EXIT(utilex_multi_set_print(unit,multi_set,clear_on_print)) ;
        SHR_IF_ERR_EXIT(utilex_multi_set_destroy(unit,multi_set)) ;
        LOG_CLI((BSL_META_U(unit," %s(): Test %d - OK!\r\n"), __FUNCTION__,test_num)) ;
    }
    {
        /*
         * Test no. 4:
         * 'max_duplications' is set to '2'
         * Add one member a few times. Should fail after 'max_duplications'.
         */
        UTILEX_MULTI_SET_KEY key[4] ;
        UTILEX_MULTI_SET_KEY ref_key[4] ;
        uint32 ii ;
        uint32 data_indx ;
        uint8 first_appear ;
        uint8 success ;
        uint32 member_size ;
        uint32 ref_count ;
        uint32 loc_ref_count ;
        shr_error_e res ;

        test_num = 4 ;
        init_info.prime_handle = 0 ;
        init_info.sec_handle = 0 ;
        init_info.nof_members = 1 ;
        init_info.member_size = 4 ;
        init_info.max_duplications = 2 ;
        init_info.global_max = 0 ;
        init_info.get_entry_fun = NULL ;
        init_info.set_entry_fun = NULL ;
        SHR_IF_ERR_EXIT(utilex_multi_set_create(unit,&multi_set,init_info)) ;
        SHR_IF_ERR_EXIT(utilex_multi_set_get_member_size(unit,multi_set,&member_size)) ;
        /*
         * Member size must be the size of the key on the hash table.
         */
        if (sizeof(key) != member_size)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            SHR_EXIT() ;
        }
        if (sizeof(ref_key) != member_size)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            SHR_EXIT() ;
        }
        sal_memset(key,0,sizeof(key)) ;
        key[0] = 64 ;
        key[1] = 10 ;
        key[2] = 9 ;
        key[3] = 8 ;
        loc_ref_count = 0 ;
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success)) ;
        loc_ref_count++ ;
        LOG_CLI((BSL_META_U(unit,
            "%s(): multi_set %d data_indx %d first_appear %d success %d\r\n"), __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Failed to add first member to multiset %d\r\n"), __FUNCTION__,(int)multi_set)) ;
            SHR_EXIT() ;
	}
        /*
         * Add the same member again. Should succeed.
         */
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success)) ;
        loc_ref_count++ ;
        LOG_CLI((BSL_META_U(unit,
            "%s(): multi_set %d data_indx %d first_appear %d success %d\r\n"),
            __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (!success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Failed to add the same member again. data_indx (%d) on multiset %d\r\n"), __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        if (first_appear)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Add same member %d on multiset %d but 'first_appear' is non-zero!\r\n"), __FUNCTION__,data_indx,(int)multi_set)) ;
            SHR_EXIT() ;
        }
        SHR_IF_ERR_EXIT(utilex_multi_set_get_by_index(unit,multi_set,data_indx,ref_key,&ref_count)) ;
        if (ref_count != loc_ref_count)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Member %d on multiset %d has ref_count %d which it should be %d!\r\n"),
                __FUNCTION__,data_indx,(int)multi_set,(int)ref_count,(int)(loc_ref_count))) ;
            SHR_EXIT() ;
        }
        if (sal_memcmp(key,ref_key,member_size) != 0)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Member %d on multiset %d has wrong key!\r\n"),
                __FUNCTION__,data_indx,(int)multi_set)) ;
            LOG_CLI((BSL_META_U(unit,"\r\n"))) ;
            LOG_CLI((BSL_META_U(unit,"Key found for member %d: 0x"),data_indx)) ;
            for (ii = 0 ; ii < member_size ; ii++)
            {
                LOG_CLI((BSL_META_U(unit,"%02X"),ref_key[ii])) ;
            }
            LOG_CLI((BSL_META_U(unit,"\r\n"))) ;
            SHR_EXIT() ;
        }
        /*
         * Now try to add the same member again. This is beyond 'max_duplications'. Should fail on 'success'!
         *
         * Should NOT fail on error on ret_val.
         */
        res = utilex_multi_set_member_add(unit,multi_set,&key[0],&data_indx,&first_appear,&success) ;
        LOG_CLI((BSL_META_U(unit,
            "%s(): multi_set %d data_indx %d first_appear %d success %d\r\n"),
             __FUNCTION__,(int)multi_set,data_indx,(int)first_appear,(int)success)) ;
        if (res != _SHR_E_NONE)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Attempt to increase member ref count on multiset %d beyond max (%d) failed on ret val %d. NOT OK. Quit.\r\n"),
                __FUNCTION__,(int)multi_set,(int)(init_info.max_duplications),(int)res)) ;
            SHR_EXIT() ;
        }
        if (success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL) ;
            LOG_CLI((BSL_META_U(unit,
                "%s(): Succedded to add member ref count to multiset %d beyond max (%d). NOT OK. Quit.\r\n"),
                __FUNCTION__,(int)multi_set,(int)(init_info.max_duplications))) ;
            SHR_EXIT() ;
        }
        clear_on_print = 0 ;
        SHR_IF_ERR_EXIT(utilex_multi_set_print(unit,multi_set,clear_on_print)) ;
        SHR_IF_ERR_EXIT(utilex_multi_set_destroy(unit,multi_set)) ;
        LOG_CLI((BSL_META_U(unit," %s(): Test %d - OK!\r\n"), __FUNCTION__,test_num)) ;
    }
exit:
    if (SHR_FUNC_ERR())
    {
        LOG_CLI((BSL_META_U(unit,
            "%s(): Exit with error %s\r\n"), __FUNCTION__,shrextend_errmsg_get(SHR_GET_CURRENT_ERR())));
    }
    else
    {
        LOG_CLI((BSL_META_U(unit, "%s(): Exit with no error. All is OK.\r\n"), __FUNCTION__));
    }
    SHR_FUNC_EXIT;
}


/*********************************************************************
* NAME:
*     utilex_multi_set_print
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   Prints the mutli-set content, the members in the multi set,
*   the number of occurrences of   each member and the index.
* INPUT:
*  UTILEX_MULTI_SET_PTR       multi_set -
*     The mutli-set to print.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_print(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    uint8 clear_on_print)
{
    uint32 data_indx;
    uint32 print_header;
    UTILEX_HASH_TABLE_ITER iter;
    uint8 key[100];
    uint32 max_duplications, counter_size, ref_count, org_ref_count;
    uint8 tmp_cnt[sizeof(uint32)];
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));
    print_header = 1 ;
    /*
     * traverse the mutli-set and print
     */
    LOG_CLI((BSL_META_U(unit, "\r\n")));
    UTILEX_HASH_TABLE_ITER_SET_BEGIN(&iter);

    while (!UTILEX_HASH_TABLE_ITER_IS_END(&iter))
    {
        SHR_IF_ERR_EXIT(utilex_hash_table_get_next(unit, hash_table, &iter, key, &data_indx));

        if (UTILEX_HASH_TABLE_ITER_IS_END(&iter))
        {
            goto exit;
        }
        org_ref_count = 1;
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

        if (max_duplications > 1)
        {
            ref_count = 0;
            SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));
            /*
             * Copy 'counter_size' bytes from 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset
             * 'data_indx * counter_size' into 'tmp_cnt' buffer.
             */
            SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                            memread(unit, multi_set_index, tmp_cnt, data_indx * counter_size, counter_size));

            SHR_IF_ERR_EXIT(utilex_U8_to_U32(tmp_cnt, counter_size, &ref_count));

            org_ref_count = ref_count;
            if (clear_on_print)
            {
                ref_count = 0;
                SHR_IF_ERR_EXIT(utilex_U32_to_U8(&ref_count, counter_size, tmp_cnt));

                /*
                 * Copy 'counter_size' bytes from 'tmp_cnt' buffer into 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter'
                 * buffer at offset 'data_indx * counter_size'.
                 */
                SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                                memwrite(unit, multi_set_index, tmp_cnt, data_indx * counter_size, counter_size));
            }
        }
        if (org_ref_count)
        {
            uint32 member_size ;
            uint32 ii ;

            SHR_IF_ERR_EXIT(utilex_multi_set_get_member_size(unit,multi_set,&member_size)) ;
            if (print_header)
            {
                LOG_CLI((BSL_META_U(unit, "| %-8s|"), "Index"));
                LOG_CLI((BSL_META_U(unit, "  ")));
                for (ii = 0 ; ii < member_size ; ii++)
                {
                    LOG_CLI((BSL_META_U(unit, "%2s"), "  "));
                }
                LOG_CLI((BSL_META_U(unit, "| %-8s|\n\r"), "ref_cnt"));
                print_header = 0 ;
            }
            LOG_CLI((BSL_META_U(unit, "| %-8u|"), data_indx));
            LOG_CLI((BSL_META_U(unit, "0x")));
            for (ii = 0 ; ii < member_size ; ii++)
            {
                LOG_CLI((BSL_META_U(unit, "%02x"), key[member_size - ii - 1]));
            }
            LOG_CLI((BSL_META_U(unit, "| %-8u|\n\r"), org_ref_count));
        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_multi_set_get_size_for_save(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    uint32 * size)
{
    uint32 max_duplications, nof_members, counter_size, cur_size, total_size;
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    total_size = 0;

    SHR_NULL_CHECK(size, _SHR_E_INTERNAL, "size");

    cur_size = sizeof(UTILEX_HASH_TABLE_INIT_INFO);
    total_size += cur_size;

    /*
     * load hash table
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_get_size_for_save(unit, hash_table, &cur_size));

    total_size += cur_size;
    /*
     * If there is need to manage the duplications, then copy array to
     * manage the reference counter.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    if (max_duplications > 1)
    {
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));

        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members));

        total_size += (nof_members * counter_size);
    }
    *size = total_size;
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*   utilex_multi_set_save
* TYPE:
*   PROC
* DATE:
*   June 01 2015
* FUNCTION:
*   saves the given multiset in the given buffer
* INPUT:
*   int                     unit -
*     Identifier of the device to access.
*   UTILEX_MULTI_SET_PTR  multi_set -
*     The multiset to save.
*   uint8                  *buffer -
*     buffer to include the hast table
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by utilex_multi_set_get_size_for_save.
*   - call back functions are not saved.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_multi_set_save(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set,
    uint8 * buffer,
    uint32 buffer_size_bytes,
    uint32 * actual_size_bytes)
{
    uint8 *cur_ptr;
    uint32 max_duplications, nof_members, counter_size, cur_size, total_size;
    uint32 multi_set_index;
    UTILEX_HASH_TABLE_PTR hash_table;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));

    SHR_NULL_CHECK(actual_size_bytes, _SHR_E_INTERNAL, "actual_size_bytes");

    cur_ptr = (uint8 *) buffer;
    total_size = 0;

    /*
     * copy init info 
     */
    /*
     * Copy sizeof(UTILEX_MULTI_SET_INIT_INFO) bytes from 'UTILEX_MULTI_SET_ACCESS_INFO' buffer at offset
     * '0' into buffer pointed by 'cur_ptr'.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.get(unit, multi_set_index, (UTILEX_MULTI_SET_INIT_INFO *) cur_ptr));

    cur_ptr += sizeof(UTILEX_MULTI_SET_INIT_INFO);

    /*
     * copy DS data 
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_save(unit, hash_table, cur_ptr, buffer_size_bytes - total_size, &cur_size));

    cur_ptr += cur_size;
    total_size += cur_size;
    /*
     * if there is need to manage the duplications, then copy array to
     * manage the reference counter.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    if (max_duplications > 1)
    {
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));

        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members));

        /*
         * Copy 'nof_members * counter_size' bytes from 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer
         * at offset '0' into buffer pointed by 'cur_ptr'.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memread(unit, multi_set_index, cur_ptr, 0, nof_members * counter_size));

        cur_ptr += (nof_members * counter_size);
        total_size += (nof_members * counter_size);
    }

    *actual_size_bytes = total_size;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_multi_set_load(
    int unit,
    uint8 ** buffer,
    UTILEX_MULTISET_SW_DB_ENTRY_SET set_function,
    UTILEX_MULTISET_SW_DB_ENTRY_GET get_function,
    UTILEX_MULTI_SET_PTR * multi_set_ptr)
{
    uint8 *cur_ptr;
    uint32 multi_set_index;
    UTILEX_MULTI_SET_INIT_INFO init_info;
    UTILEX_HASH_TABLE_PTR hash_table;
    UTILEX_MULTI_SET_PTR multi_set;
    uint32 max_duplications, nof_members, counter_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_NULL_CHECK(buffer, _SHR_E_INTERNAL, "buffer");
    cur_ptr = (uint8 *) buffer[0];
    /*
     * copy init info 
     */
    sal_memcpy(&(init_info), cur_ptr, sizeof(UTILEX_MULTI_SET_INIT_INFO));
    cur_ptr += sizeof(UTILEX_MULTI_SET_INIT_INFO);
    init_info.set_entry_fun = set_function;
    init_info.get_entry_fun = get_function;
    /*
     * create DS - will not work!! (petra only code)
     */
    SHR_IF_ERR_EXIT(utilex_multi_set_create(unit, multi_set_ptr, init_info));
    multi_set = *multi_set_ptr;
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.hash_table.get(unit, multi_set_index, &hash_table));
    SHR_IF_ERR_EXIT(utilex_hash_table_destroy(unit, hash_table));
    /*
     * load hash table
     */
    SHR_IF_ERR_EXIT(utilex_hash_table_load(unit, &cur_ptr, set_function, get_function, NULL, NULL, &(hash_table)));
    /*
     * if there is need to manage the duplications, then copy array to
     * manage the reference counter.
     */
    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));
    if (max_duplications > 1)
    {
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members));
        /*
         * Copy 'nof_members * counter_size' bytes from buffer pointed by 'cur_ptr'
         * into 'UTILEX_MULTI_SET_ACCESS_DATA.ref_counter' buffer at offset '0'.
         */
        SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.ref_counter.
                        memwrite(unit, multi_set_index, cur_ptr, 0, nof_members * counter_size));
        cur_ptr += (nof_members * counter_size);
    }
    *buffer = cur_ptr;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_SAND_MULTI_SET_INFO_print(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set)
{
    uint32 multi_set_index;
    uint32 max_duplications, nof_members, member_size, sec_handle, counter_size;
    int prime_handle;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_MULTI_SET_CONVERT_HANDLE_TO_MULTISET_INDEX(multi_set_index, multi_set);
    UTILEX_MULTI_SET_VERIFY_MULTISET_IS_ACTIVE(multi_set_index);

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.max_duplications.get(unit, multi_set_index, &max_duplications));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.nof_members.get(unit, multi_set_index, &nof_members));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.member_size.get(unit, multi_set_index, &member_size));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_DATA.counter_size.get(unit, multi_set_index, &counter_size));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.prime_handle.get(unit, multi_set_index, &prime_handle));

    SHR_IF_ERR_EXIT(UTILEX_MULTI_SET_ACCESS_INFO.sec_handle.get(unit, multi_set_index, &sec_handle));

    LOG_CLI((BSL_META_U(unit, "init_info.max_duplications: %u\n"), max_duplications));
    LOG_CLI((BSL_META_U(unit, "init_info.nof_members: %u\n"), nof_members));
    LOG_CLI((BSL_META_U(unit, "init_info.member_size: %u\n"), member_size));
    LOG_CLI((BSL_META_U(unit, "multiset_data.counter_size: %u\n"), counter_size));
    LOG_CLI((BSL_META_U(unit, "init_info.prime_handle: %u\n"), prime_handle));
    LOG_CLI((BSL_META_U(unit, "init_info.sec_handle: %u\n"), sec_handle));

exit:
    SHR_FUNC_EXIT;
}
/*
 * } 
 */
#endif /* UTILEX_DEBUG */
/*
 * } 
 */
/*
 * FUNCTIONS 
 */
/*
 * } 
 */
#else
/*
 * { 
 */
/*
 * This is DUMMY code. It is only for compilers that do not accept empty files
 * and is never to be used.
 */
shr_error_e
utilex_multi_set_info_print(
    int unit,
    UTILEX_MULTI_SET_PTR multi_set)
{
    SHR_FUNC_INIT_VARS(unit);
    UTILEX_MULTI_SET_VERIFY_UNIT_IS_LEGAL(unit);
exit:
    SHR_FUNC_EXIT;
}
/*
 * } 
 */
#endif /* BCM_DNX_SUPPORT */
