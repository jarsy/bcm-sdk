/* $Id: utilex_sorted_list.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/** \file utilex_sorted_list.c
 *
 * All common utilities related to sorted list.
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDSWDNX_UTILSDNX

/*************
* INCLUDES  *
*************/
/* { */

#include <shared/bsl.h>
#include <sal/core/libc.h>
#include <sal/appl/io.h>
#include <assert.h>
#include <shared/shrextend/shrextend_debug.h>
/*
 * SW is included
 */
#include <shared/swstate/access/sw_state_access.h>
#include <shared/utilex/utilex_sorted_list.h>

/* } */

/*
 * SW is included
 */
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
#ifdef BCM_DNX_SUPPORT
/* { */

 /*************
 * GLOBALS   *
 *************/
 /*
  * { 
  */
/**
 * \brief
 * Root of 'sw atate' data block
 */
extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];

 /*
  * } 
  */
 /*************
 * FUNCTIONS *
 *************/
 /*
  * { 
  */

/************************************************************************/
/*  Internal functions                                                  */
/************************************************************************/
/* { */

/**
 * \brief
 *   Get value of 'tmp_data' pointer (See \ref UTILEX_SORTED_LIST_T)
 *   from handle.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to extract info from. Sorted list is
 *     assumed to have been created.
 *   \param [in] tmp_data_ptr_ptr -
 *     Pointer to 'pointer to tmp_data' \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by the pointer to the 'tmp_data'
 *     internal workspace buffer.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS and
 *     sw_state[unit]->dnx.shareddnx.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_data)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   See 'tmp_data_ptr_ptr' on DIRECT INPUT
 * \remarks:
 *   This procedure is exceptional. It is added here so we can use
 *   the buffer pointed by 'tmp_data' as a work space whose address
 *   is passed to variuos utilities.
 * \see
 *   * utilex_sorted_list_node_link_set()
 */
static shr_error_e
utilex_sorted_list_get_tmp_data_ptr_from_handle(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint8 ** tmp_data_ptr_ptr)
{
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    *tmp_data_ptr_ptr =
        sw_state[unit]->dnx.shareddnx.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_data;
exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief
 *   Get value of 'tmp_key' pointer (See \ref UTILEX_SORTED_LIST_T)
 *   from handle.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to extract info from. Sorted list is
 *     assumed to have been created.
 *   \param [in] tmp_key_ptr_ptr -
 *     Pointer to 'pointer to tmp_key' \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by the pointer to the 'tmp_key'
 *     internal workspace buffer.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, see \ref UTILEX_SORTED_LIST_ACCESS and
 *     sw_state[unit]->dnx.shareddnx.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_key)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   See 'tmp_data_ptr_ptr' on DIRECT INPUT
 * \remarks:
 *   This procedure is exceptional. It is added here so we can use
 *   the buffer pointed by 'tmp_key' as a work space whose address
 *   is passed to variuos utilities.
 * \see
 *   * utilex_sorted_list_node_link_set()
 */
static shr_error_e
utilex_sorted_list_get_tmp_key_ptr_from_handle(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint8 ** tmp_key_ptr_ptr)
{
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    *tmp_key_ptr_ptr =
        sw_state[unit]->dnx.shareddnx.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_key;
exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief
 *  Get the next valid entry (key and data) in the sorted list.
 *  Start traversing from the place pointed by the given iterator ('curr').
 *  Traverse either 'forward' (from smaller values to higher) or
 *  'backward ('not forward').
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to extract info from. Sorted list is
 *     assumed to have been created.
 *   \param [in] curr -
 *     Value of iterator (of type UTILEX_SORTED_LIST_ITER) on Sorted List
 *     to start the search at ('forward' or 'backward').
 *     Note that if 'curr' is at the top end of the table then, requesting,
 *     say, to get the next entry will result in loading 'null_ptr' into
 *     '*next_or_prev'. See below.
 *   \param [in] forward -
 *     Flag (of type uint8) indicating whether to search the Sorted List
 *     from smaller to larger values ('forward') or the other way round
 *     ('backward').
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     \b As \b output - \n
 *     If 'key' is not NULL and the next/prev iterator was found then this
 *     procedure loads *key with the 'key' corresponding to the found iterator.
 *   \param [in] data -
 *     Pointer to UTILEX_SORTED_LIST_DATA (array of uint8s). \n
 *     \b As \b output - \n
 *     If 'data' is not NULL and the next/prev iterator was found then this
 *     procedure loads *data with the 'data' corresponding to the found iterator.
 *   \param [in] next_or_prev -
 *     Pointer to UTILEX_SORTED_LIST_ITER \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by the next iterator ('larger' key), if
 *     'forward' is true, or by the previous iterator ('smaller' key) if 'forward' is
 *     false.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, the sorted list itself. See \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'key' on DIRECT INPUT
 *   * See \b 'data' on DIRECT INPUT \n
 *   * See \b 'next_or_prev' on DIRECT INPUT 
 * \remarks:
 *   * To start traversing the Sorted List from the beginning,
 *     use UTILEX_SORTED_LIST_ITER_BEGIN(unit,iter)
 *   * To check whether the iterator got to the end of the table,
 *     use UTILEX_SORTED_LIST_ITER_END(unit,iter)
 * \see
 *   * utilex_sorted_list_find_match_entry()
 */
static shr_error_e
utilex_sorted_list_get_next_aux(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER curr,
    uint8 forward,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    UTILEX_SORTED_LIST_ITER * next_or_prev)
{
    uint32 ptr_long;
    uint32 null_ptr, sorted_list_index;
    uint8 *tmp_data_ptr;
    uint32 ptr_size, key_size, data_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(next_or_prev, _SHR_E_INTERNAL, "next_or_prev");

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.null_ptr.get(unit, sorted_list_index, &null_ptr));
    /*
     * read next entry.
     */
    if ((!forward && curr == UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list))
        || ((forward && curr == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list))))
    {
        *next_or_prev = null_ptr;
        goto exit;
    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_tmp_data_ptr_from_handle(unit, sorted_list, &tmp_data_ptr));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
    if (forward)
    {
        /*
         * Copy 'ptr_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.next' buffer at offset
         * 'curr * ptr_size' into 'tmp_data' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.memread(unit, sorted_list_index,
                                                                    tmp_data_ptr, curr * ptr_size, ptr_size));
    }
    else
    {
        /*
         * Copy 'ptr_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.prev' buffer at offset
         * 'curr * ptr_size' into 'tmp_data' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.memread(unit, sorted_list_index,
                                                                    tmp_data_ptr, curr * ptr_size, ptr_size));
    }
    /*
     * Check whether the list node is null.
     */
    ptr_long = 0;
    utilex_U8_to_U32(tmp_data_ptr, ptr_size, &ptr_long);
    if (ptr_long == null_ptr)
    {
        *next_or_prev = null_ptr;
        goto exit;
    }
    if (ptr_long == UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list))
    {
        *next_or_prev = ptr_long;
        goto exit;
    }
    if (key != NULL)
    {
        /*
         * Read keys
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
        /*
         * Copy 'key_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.keys' buffer at offset
         * 'ptr_long * key_size' into input 'key' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memread(unit, sorted_list_index,
                                                                    key, ptr_long * key_size, key_size));
    }
    if (data != NULL)
    {
        /*
         * read data
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
        /*
         * Copy 'data_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.data' buffer at offset
         * 'ptr_long * data_size' into input 'data' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memread(unit, sorted_list_index,
                                                                    data, ptr_long * data_size, data_size));
    }
    *next_or_prev = ptr_long;
    goto exit;
exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief
 *  Connect two nodes (two iterators) on indicated Sorted List. The
 *  first (node1) points forward (using 'next') to the second (node2)
 *  and the second points backwards (using 'prev') to the first.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to extract info from. Sorted list is
 *     assumed to have been created.
 *   \param [in] node1 -
 *     Value of first iterator (of type UTILEX_SORTED_LIST_ITER = uint32)
 *     on Sorted List. This entry is set to point forward (using 'next')
 *     to the second entry (node2).
 *   \param [in] node2 -
 *     Value of second iterator (of type UTILEX_SORTED_LIST_ITER = uint32)
 *     on Sorted List. This entry is set to point backward (using 'prev')
 *     to the first entry (node2).
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, the sorted list itself. See \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remarks:
 *   * None
 * \see
 *   * utilex_sorted_list_node_alloc()
 */
static shr_error_e
utilex_sorted_list_node_link_set(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 node1,
    uint32 node2)
{
    uint32 sorted_list_index, ptr_size, node1_ptr[1], node2_ptr[2];
    uint8 *tmp_data_ptr;

    SHR_FUNC_INIT_VARS(unit);
    /*
     * to aviod coverity defect of array vs singleton 
     */
    *node1_ptr = node1;
    *node2_ptr = node2;
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(utilex_sorted_list_get_tmp_data_ptr_from_handle(unit, sorted_list, &tmp_data_ptr));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
    SHR_IF_ERR_EXIT(utilex_U32_to_U8(node2_ptr, ptr_size, tmp_data_ptr));
    /*
     * if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) 
     */
    {
        /*
         * Copy 'ptr_size' bytes from 'tmp_data_ptr' into
         * 'UTILEX_SORTED_LIST_ACCESS_DATA.next' buffer at offset 'ptr_size * node1'.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.memwrite(unit, sorted_list_index,
                                                                     tmp_data_ptr, node1 * ptr_size, ptr_size));
    }
    SHR_IF_ERR_EXIT(utilex_U32_to_U8(node1_ptr, ptr_size, tmp_data_ptr));
    /*
     * if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) 
     */
    {
        /*
         * Copy 'ptr_size' bytes from 'tmp_data_ptr' into
         * 'UTILEX_SORTED_LIST_ACCESS_DATA.prev' buffer at offset 'ptr_size * node2'.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.memwrite(unit, sorted_list_index,
                                                                     tmp_data_ptr, node2 * ptr_size, ptr_size));
    }
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Insert a new entry into the sorted list, if there is no free space,
 *  the operation returns an error.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to extract info from. Sorted list is
 *     assumed to have been created.
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     '*key' points to the key to attach to the new entry on specified Sorted List.
 *   \param [in] data -
 *     Pointer to UTILEX_SORTED_LIST_DATA (array of uint8s). \n
 *     '*data' points to the data to attach to the new entry on specified Sorted List.
 *   \param [in] prev_node -
 *     Value of iterator (of type UTILEX_SORTED_LIST_ITER = uint32), on Sorted List,
 *     to be set 'previous' to (smaller than) the newly added entry.
 *   \param [in] next_node -
 *     Value of iterator (of type UTILEX_SORTED_LIST_ITER = uint32), on Sorted List,
 *     to be set 'next' to (larger than) the newly added entry.
 *   \param [in] from_top_index_in_indices -
 *     Only meaningful if the 'indices' feature is enabled ('indices' element
 *     in UTILEX_SORTED_LIST_T is allocated). This is the index, in 'indices'
 *     array, after which the newly matched entry fits (and should be added).
 *     Note that this index is counted from the TOP of 'indices'. So, for
 *     example, when index=0 then entry should be added as LAST on 'indices'.
 *   \param [in] found -
 *     Pointer to uint8 \n
 *     \b As \b output - \n
 *     This procedure loads pointed memory by TRUE, if place was for the new entry
 *     to be added. Otherwise, FALSE is loaded.
 * \par INDIRECT INPUT
 *   * SWSTATE system (Specifically, the sorted list itself. See \ref UTILEX_SORTED_LIST_ACCESS)
 *   * See 'key' in DIRECT INPUT
 *   * See 'data' in DIRECT INPUT
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See 'found' in DIRECT INPUT
 * \remarks:
 *   * Caller is assumed to have verified that there is no such entry,
 *     already, on the list.
 * \see
 *   * utilex_sorted_list_find_match_entry()
 */
static shr_error_e
utilex_sorted_list_node_alloc(
    uint32 unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint32 prev_node,
    uint32 next_node,
    uint32 from_top_index_in_indices,
    uint8 * found)
{
    uint32 new_node_ptr;
    uint32 index_in_indices, sorted_list_index;
    uint32 key_size, data_size;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");
    SHR_NULL_CHECK(data, _SHR_E_INTERNAL, "data");
    SHR_NULL_CHECK(found, _SHR_E_INTERNAL, "found");
    /*
     * Check to see whether there is space for a new entry.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.get(unit, sorted_list_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_alloc_next(unit, memory_use, &new_node_ptr, found));
    if (*found == FALSE)
    {
        goto exit;
    }
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
        /*
         * Copy 'key_size' bytes from input 'keys' buffer into 'UTILEX_SORTED_LIST_ACCESS_DATA.keys'
         * buffer at offset 'key_size * new_node_ptr'.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memwrite(unit, sorted_list_index,
                                                                     key, new_node_ptr * key_size, key_size));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
        /*
         * Copy 'data_size' bytes from input 'data' buffer into 'UTILEX_SORTED_LIST_ACCESS_DATA.data'
         * buffer at offset 'data_size * new_node_ptr'.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memwrite(unit, sorted_list_index,
                                                                     data, new_node_ptr * data_size, data_size));
    }
    utilex_sorted_list_node_link_set(unit, sorted_list, prev_node, new_node_ptr);
    utilex_sorted_list_node_link_set(unit, sorted_list, new_node_ptr, next_node);
    {
        uint8 is_allocated;
        uint32 num_elements_on_indices;
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));

        if (is_allocated)
        {
            /*
             * Feature is enabled. Add element on 'indices' array at specified location.
             */
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit, sorted_list_index,
                                                                                       &num_elements_on_indices));

            if (num_elements_on_indices == 0)
            {
                /*
                 * If the 'indices' array is empty then, obviously, add the first element
                 * and ignore 'index_in_indices' (which should be '0').
                 */
                SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.set(unit, sorted_list_index, 0, new_node_ptr));

            }
            else
            {
                index_in_indices = num_elements_on_indices - from_top_index_in_indices - 1;
                /*
                 * Now 'index_in_indices' is index from the beginning of the array.
                 */
                if ((int) index_in_indices < -1)
                {
                    /*
                     * Error: System requires to write beyond current range of indices.
                     */
                    SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
                    SHR_EXIT();
                }
                else if ((int) index_in_indices < (int) (num_elements_on_indices - 1))
                {
                    /*
                     * We now need to make space (insert) 'new_node_ptr'. This requires
                     * Moving all elements BEYOND 'index_in_indices'. Note that when
                     * 'index_in_indices' is '-1' then the new element needs to be inserted as first.
                     */
                    uint32 tmp_index, tmp_on_indices;
                    uint32 num_to_copy;
                    num_to_copy = (num_elements_on_indices - 1 - index_in_indices);
                    tmp_index = num_elements_on_indices - 1;
                    while (num_to_copy)
                    {
                        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.get(unit, sorted_list_index,
                                                                                   tmp_index, &tmp_on_indices));

                        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.set(unit, sorted_list_index,
                                                                                   tmp_index + 1, tmp_on_indices));

                        num_to_copy--;
                        tmp_index--;
                    }
                }
                /*
                 * Note that if (index_in_indices == (num_elements_on_indices - 1)) then no 'move'
                 * is required.
                 */
                SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.set(unit, sorted_list_index,
                                                                           index_in_indices + 1, new_node_ptr));

            }
            num_elements_on_indices++;
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit, sorted_list_index,
                                                                                       num_elements_on_indices));

        }
    }
exit:
    SHR_FUNC_EXIT;
}

/*
 * The following procedures are temporarily shaped to run on DNX.
 * However, on the final code, all definitions and procedures
 * will have to be removed from here and will have to be taken from
 * external files (e.g., jericho_2_tcam.h, jericho_2_tcam.c).
 * The procedure 'utilex_sorted_list_tcam_cmp_priority()' will
 * probably need to be moved to jericho_2_tcam.c.
 *
 * Note that we replaced, on all names, the word 'arad' by 'dnx' so,
 * for example, DNX_TCAM_DB_LIST_KEY_SIZE became DNX_TCAM_DB_LIST_KEY_SIZE.
 */
/*
 * {
 */
typedef struct
{
    /*
     *  First entry-id in the priority group
     */
    uint32 entry_id_first;
    /*
     *  Last entry-id in the priority group
     */
    uint32 entry_id_last;

} DNX_TCAM_PRIO_LOCATION;

#define DNX_TCAM_DB_LIST_KEY_SIZE                               (sizeof(uint32) /* for priority */)
#define DNX_TCAM_DB_LIST_DATA_SIZE                              (sizeof(DNX_TCAM_PRIO_LOCATION))

/**
 * \brief
 *  Clear a structure which describes a 'priority group' (set of entries
 *  with the same priority). See remarks.
 * \par DIRECT INPUT
 *   \param [in] info -
 *     Pointer to structure of type DNX_TCAM_PRIO_LOCATION.
 *     \b As \b output -\n
 *     This procedure clears this structure (which is the 'priority group').
 * \par INDIRECT INPUT
 *   * None
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See 'info' in DIRECT INPUT
 * \remarks:
 *   * 'Priority group' is loaded into 'data' of entries on Sorted List' so
 *     that even if two entries have the same priority, they may still differ
 *     by the 'priority group'.
 * \see
 *   * utilex_sorted_list_tcam_cmp_priority()
 */
static shr_error_e
dnx_tcam_prio_location_clear(
    DNX_TCAM_PRIO_LOCATION * info)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);

    SHR_NULL_CHECK(info, _SHR_E_INTERNAL, "info");
    sal_memset(info, 0x0, sizeof(*info));
exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief
 *  Convert 'data' (with format as on Sorted List') to a uint32 value.
 * \par DIRECT INPUT
 *   \param [in] data -
 *     Pointer to array of, at least four, uit8s.
 *     This procedure reads this array and converts the info to uint32.
 * \par INDIRECT INPUT
 *   * See 'data' in DIRECT INPUT
 * \par DIRECT OUTPUT
 *   \retval Value as converted from '*data'.
 * \par INDIRECT OUTPUT
 *   * None
 * \remarks:
 *   * None.
 * \see
 *   * utilex_sorted_list_tcam_cmp_priority()
 */
static uint32
dnx_tcam_db_prio_list_priority_value_decode(
    uint8 * data)
{
    uint32 value;

    utilex_U8_to_U32(data, sizeof(uint32), &value);

    return value;
}

/**
 * \brief
 * Compare two input buffers using custom metric:        \n
 * If the first (buffer1) is equal to the second (buffer2)       \n
 *                                        then return zero.      \n
 * If the first (buffer1) is larger than the second (buffer2)            \n
 *                                        then return a positive number. \n
 * If the first (buffer1) is smaller than the second (buffer2)           \n
 *                                        then return a negative number. \n
 * \par DIRECT INPUT:
 *   \param [in] buffer1 -
 *     Buffer of uint8s to compare contents of with 'buffer2'.
 *     Comparison is not necessarily straightforward. See 'size'
 *     below.
 *   \param [in] buffer2 -
 *     Buffer of uint8s to compare contents of with 'buffer1'
 *     Comparison is not necessarily straightforward. See 'size'
 *     below.
 *   \param [in] size -
 *     Number of bytes in each of the 'buffer's specified above.
 *     This number is also used for selection of the 'compare'
 *     algorithm: \n
 *     If 'size' is \ref DNX_TCAM_DB_LIST_KEY_SIZE then the
 *     bytes in each buffer are converted to uint32 and
 *     the results are compared. \n
 *     If 'size' is \ref DNX_TCAM_DB_LIST_DATA_SIZE then the
 *     bytes in each buffer are converted to structure of
 *     type DNX_TCAM_PRIO_LOCATION. This structure stands
 *     for a TCAM priority group. Two peiority groups ar
 *     consideref equal only if the entry ID of the first and
 *     the entry ID of the last are the same.
 * \par INDIRECT INPUT:
 *   SWSTATE system
 * \par DIRECT OUTPUT:
 *   * None
 * \par INDIRECT OUTPUT:
 *   In case of error condition, goto exit with _SHR_E_MEMORY
 * \remark
 *   Originally, this procedure was custom tailored to TCAM of ARAD.
 *   Its porting requires device-specific definitions (e.g., arad_tcam.h)
 *   and device-specific code (e.g., arad_tcam.c)  \n
 *   For now, TEMPORARILY, we add local definitions and tailor it
 *   to DNX.
 */
static int32
utilex_sorted_list_tcam_cmp_priority(
    uint8 * buffer1,
    uint8 * buffer2,
    uint32 size)
{
    uint32 prio_1, prio_2;
    DNX_TCAM_PRIO_LOCATION prio_location_1, prio_location_2;

    if ((buffer1 == NULL) || (buffer2 == NULL))
    {
        /*
         * Not supposed to compare NULL pointers 
         */
        assert(0);
    }
    /*
     * Key comparison 
     */
    if (size == DNX_TCAM_DB_LIST_KEY_SIZE)
    {
        prio_1 = dnx_tcam_db_prio_list_priority_value_decode(buffer1);
        prio_2 = dnx_tcam_db_prio_list_priority_value_decode(buffer2);
        return (((int32) prio_1) - ((int32) prio_2));
    }
    else if (size == DNX_TCAM_DB_LIST_DATA_SIZE)
    {   /* Data comparison */
        dnx_tcam_prio_location_clear(&prio_location_1);
        dnx_tcam_prio_location_clear(&prio_location_2);
        /*
         * buffer1 and buffer2 can't be null. It has been checked above 
         */
        /*
         * coverity[var_deref_model:FALSE] 
         */
        sal_memcpy(&prio_location_1, buffer1, sizeof(DNX_TCAM_PRIO_LOCATION));
        /*
         * coverity[var_deref_model:FALSE] 
         */
        sal_memcpy(&prio_location_2, buffer2, sizeof(DNX_TCAM_PRIO_LOCATION));
        return ((prio_location_1.entry_id_first !=
                 prio_location_2.entry_id_first)
                || (prio_location_1.entry_id_last != prio_location_2.entry_id_last)) ? 1 : 0;
    }
    else
    {
        /*
         * Unknown input formats 
         */
        assert(0);
        return 0;
    }
}

static shr_error_e
utilex_sorted_list_data_cmp(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 data_place,
    UTILEX_SORTED_LIST_DATA * const data,
    int32 * cmp_res_ptr)
{
    uint32 sorted_list_index, data_size;
    int32 cmp_res;
    UTILEX_SORTED_LIST_KEY_CMP cmp_key_fun;
    uint8 *tmp_data_ptr;
    utilex_sorted_list_cmp_func_type_e cmp_func_type;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit, sorted_list_index, &cmp_func_type));
    if (cmp_func_type == UTILEX_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
    {
        cmp_key_fun = utilex_sorted_list_tcam_cmp_priority;
    }
    else
    {
        cmp_key_fun = (UTILEX_SORTED_LIST_KEY_CMP) sal_memcmp;
    }
    {
        /*
         * read data: Copy 'data' to 'tmp_data'
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_tmp_data_ptr_from_handle(unit, sorted_list, &tmp_data_ptr));
        /*
         * Copy 'data_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.data' buffer at offset
         * 'data_size * data_place' into 'tmp_data' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memread(unit, sorted_list_index,
                                                                    tmp_data_ptr, data_place * data_size, data_size));
        /*
         * Now compare input 'data' buffer with data stored in sorted list using
         * indicated compare function.
         */
        cmp_res = cmp_key_fun(tmp_data_ptr, data, data_size * sizeof(uint8));
        *cmp_res_ptr = cmp_res;
    }
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *   Lookup indicated Sorted List for the specified key and return the data
 *   which has been inserted together with the given key.
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Identifier of the device to access.
 *   \param [in] sorted_list -
 *     Handle of the Sorted List to extract info from. Sorted list is
 *     assumed to have been created.
 *   \param [in] key -
 *     Pointer to UTILEX_SORTED_LIST_KEY (array of uint8s). \n
 *     '*key' points to the key to look for on Sorted List.
 *   \param [in] data -
 *     Pointer to UTILEX_SORTED_LIST_DATA (array of uint8s). \n
 *     '*data' points to the data to look for once 'key' was matched on Sorted List.
 *     If 'data' is set to NULL then this procedure ignores indicated data.
 *   \param [in] first_match -
 *     If 'first_match' is non zero then, when a matched key is found,
 *     program does not look further into '*data' to find a match there as well.
 *     In other words, '*data' plays no role in the search.
 *   \param [in] found -
 *     Pointer to boolean indicator \n
 *     \b As \b output - \n
 *     '*found' is loaded by TRUE only if match was found for both 'key' and 'data'.
 *     If 'first_match' is set then '*found' is loaded by TRUE if match was found
 *     for 'key' alone. See 'remarks' below.
 *   \param [in] prev_node -
 *     Pointer to UTILEX_SORTED_LIST_ITER \n
 *     \b As \b output - \n
 *     '*prev_node' is loaded by the iterator BELOW the one corresponding to 'key'
 *     (and, possibly, 'data'). If '*found' is TRUE then this is the interator
 *     just lower of the one specified by '*cur_node'
 *   \param [in] cur_node -
 *     Pointer to UTILEX_SORTED_LIST_ITER \n
 *     \b As \b output - \n
 *     If '*found' is FALSE, '*cur_node' is loaded by the iterator ABOVE the one
 *     corresponding to 'key' (and, possibly, 'data').
 *     If '*found' is TRUE then this is the interator which matches input 'key' (and,
 *     possibly, 'data').
 *   \param [in] index_in_indices -
 *     Pointer to uint32 \n
 *     \b As \b output - \n
 *     Only meaningful if the 'indices' feature is enabled (i.e., 'indices' element in
 *     UTILEX_SORTED_LIST_T is allocated).\n
 *     If '*found' is FALSE, this procedure loads pointed memory by index, in 'indices'
 *     array, after which the newly matched entry fits, and may be added.\n
 *     If '*found' is TRUE then the returned index is that of specified
 *     'key'. Note that, in this case, another entry with the same key is not acceptable.\n
 *     Note that this index (*index_in_indices) is counted from the TOP of 'indices'. So,
 *     for example, when index=0 then entry should be added as LAST on 'indices'.
 * \par INDIRECT INPUT
 *   * See \b 'key' in DIRECT INPUT
 *   * See \b 'data' in DIRECT INPUT
 *   * SWSTATE system (Specifically, the sorted list itself. See \ref UTILEX_SORTED_LIST_ACCESS)
 * \par DIRECT OUTPUT
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT
 *   * See \b 'found' on DIRECT INPUT
 *   * See \b 'prev_node' on DIRECT INPUT \n
 *   * See \b 'cur_node' on DIRECT INPUT 
 * \remarks:
 *   * If 'first_match' is FALSE and 'data' is NULL then this procedure will always
 *     return with '*found = FALSE' and with  '*prev_node', '*cur_node' loaded
 *     by the iterator below, above input key, respectively.
 *   * The search is done from top to bottom so, for example, if '*index_in_indices' is zero
 *     then an new entry may be added at the top (high values) of the list.
 * \see
 *   * utilex_sorted_list_entry_add()
 */
static shr_error_e
utilex_sorted_list_find_match_entry(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint8 first_match,
    uint8 * found,
    UTILEX_SORTED_LIST_ITER * prev_node,
    UTILEX_SORTED_LIST_ITER * cur_node,
    uint32 * index_in_indices)
{
    uint32 key_size, null_ptr, prev, curr, curr_index;
    UTILEX_SORTED_LIST_KEY_CMP cmp_key_fun;
    utilex_sorted_list_cmp_func_type_e cmp_func_type;
    int32 compare_res;
    uint32 sorted_list_index;
    uint8 *tmp_key_ptr;

    SHR_FUNC_INIT_VARS(unit);
    /*
     * Just make sure that, in case of error, an illegal value is returned.
     * Good practive though not necessary.
     */
    *index_in_indices = -2;
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");
    SHR_NULL_CHECK(found, _SHR_E_INTERNAL, "found");
    SHR_NULL_CHECK(prev_node, _SHR_E_INTERNAL, "prev_node");
    SHR_NULL_CHECK(cur_node, _SHR_E_INTERNAL, "cur_node");

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit, sorted_list_index, &cmp_func_type));

    if (cmp_func_type == UTILEX_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
    {
        cmp_key_fun = utilex_sorted_list_tcam_cmp_priority;
    }
    else
    {
        cmp_key_fun = (UTILEX_SORTED_LIST_KEY_CMP) sal_memcmp;
    }
    *found = FALSE;
    /*
     * Set 'prev' iterator to the end of the list (top, one beyond highest)
     */
    UTILEX_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index, prev);

    SHR_IF_ERR_EXIT(utilex_sorted_list_get_tmp_key_ptr_from_handle(unit, sorted_list, &tmp_key_ptr));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
    /*
     * 'curr_index' is the index into 'indices' array beyond which the new node is
     * to be added. Only meaningful if 'indices' feature is enabled.
     */
    curr_index = 0;
    do
    {
        /*
         * Read next entry. (This is inverted_order - so read previous)
         *
         * Scan the list from the end - faster in TCAM for sorted entries
         */
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux(unit, sorted_list, prev, FALSE, tmp_key_ptr, NULL, &curr));

        if (curr == UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list))
        {
            /*
             * If program has reached this place then it has reached the beginning of the
             * list and no match has been found. '*prev_node' is loaded with indication
             * of 'beginning of list' and '*cur_node' is loaded with iterator of first
             * entry.
             */
            *found = FALSE;
            *prev_node = UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list);
            *cur_node = prev;
            /*
             * We need to insert at the beginning of the array
             */
            *index_in_indices = curr_index;
            goto exit;
        }
        compare_res = cmp_key_fun(tmp_key_ptr, key, (key_size * sizeof(UTILEX_SORTED_LIST_KEY_TYPE)));
        /*
         * If key was found
         */
        if (compare_res == 0)
        {
            if (first_match)
            {
                *found = TRUE;
                prev = curr;
                SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux
                                (unit, sorted_list, prev, FALSE, tmp_key_ptr, NULL, &curr));
                *prev_node = curr;
                *cur_node = prev;
                *index_in_indices = curr_index;
                goto exit;
            }
            if (data != NULL)
            {
                SHR_IF_ERR_EXIT(utilex_sorted_list_data_cmp(unit, sorted_list, curr, data, &compare_res));
                if (compare_res)
                {
                    *found = TRUE;
                    prev = curr;
                    SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux
                                    (unit, sorted_list, prev, FALSE, tmp_key_ptr, NULL, &curr));
                    *prev_node = curr;
                    *cur_node = prev;
                    *index_in_indices = curr_index;
                    goto exit;
                }
            }
        }
        else if (compare_res < 0)
        {
            /*
             * Reach here if key on list is smaller than input key.
             */
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.null_ptr.get(unit, sorted_list_index, &null_ptr));

            *found = FALSE;
            *prev_node = (curr == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list)) ? null_ptr : curr;
            *cur_node = prev;
            *index_in_indices = curr_index;
            goto exit;
        }
        prev = curr;
        curr_index++;
    }
    while (!(*found));
exit:
    SHR_FUNC_EXIT;
}

/*
 * }
 */
#if (0)
/* { */
/*
 * The following procedures are not in use for current code. They are
 * left here only as examples for implementation.
 */
shr_error_e
utilex_sorted_list_default_entry_set(
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

shr_error_e
utilex_sorted_list_default_entry_get(
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
/* } */
#endif
/************************************************************************/
/*  End of internals                                                    */
/************************************************************************/
/* } */

shr_error_e
utilex_sorted_list_init(
    int unit,
    uint32 max_nof_lists)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.alloc(unit));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.lists_array.ptr_alloc(unit, max_nof_lists));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.max_nof_lists.set(unit, max_nof_lists));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.in_use.set(unit, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.occupied_lists.alloc_bitmap(unit, max_nof_lists));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_clear_all_tmps(
    int unit)
{
    uint32 sorted_list_index;
    uint32 found, max_nof_lists, tmp_size, key_size, data_size, ptr_size, in_use;
    uint8 bit_val;
    uint8 is_allocated;
    int32 offset;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.in_use.get(unit, &in_use));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists));
    if (in_use >= max_nof_lists)
    {
        /*
         * If number of occupied bitmap structures is beyond the
         * maximum then quit with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    /*
     * Find occupied lists (a set bit in 'occupied_lists') and, for each,
     * fill 'tmp' buffers by zeroes.
     *
     * Currently, 'tmp' buffers are:
     *   sw_state[unit]->dnx.shareddnx.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_key
     *   sw_state[unit]->dnx.shareddnx.sand.sorted_list->lists_array[sorted_list_index]->list_data.tmp_data
     */
    found = 0;
    offset = 0;
    for (sorted_list_index = 0; sorted_list_index < max_nof_lists; sorted_list_index++)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val));
        if (bit_val == 1)
        {
            /*
             * 'sorted_list_index' is now the index of an occupied entry.
             */
            found++;
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_key.is_allocated(unit,
                                                                                sorted_list_index, &is_allocated));
            if (!is_allocated)
            {
                /*
                 * 'tmp_key' buffer must be allocated, at this point.
                 */
                SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
                SHR_EXIT();
            }
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_data.is_allocated(unit,
                                                                                 sorted_list_index, &is_allocated));
            if (!is_allocated)
            {
                /*
                 * 'tmp_data' buffer must be allocated, at this point.
                 */
                SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
                SHR_EXIT();
            }
            /*
             * Clear 'tmp_key'
             */
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_key.memset(unit,
                                                                          sorted_list_index, offset, key_size, 0));
            /*
             * Clear 'tmp_data'
             */
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
            tmp_size = UTILEX_MAX(ptr_size, data_size);
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_data.memset(unit,
                                                                           sorted_list_index, offset, tmp_size, 0));

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
utilex_sorted_list_create(
    int unit,
    UTILEX_SORTED_LIST_PTR * sorted_list_ptr,
    UTILEX_SORTED_LIST_INIT_INFO init_info)
{
    UTILEX_OCC_BM_INIT_INFO btmp_init_info;
    uint32 tmp_size;
    uint32 sorted_list_index;
    uint32 found, data_size, list_size, key_size, null_ptr, ptr_size, max_nof_lists;
    uint32 in_use;
    uint8 bit_val;
    UTILEX_SORTED_LIST_PTR sorted_list;
    UTILEX_OCC_BM_PTR memory_use;
    int32 offset;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    SHR_NULL_CHECK(sorted_list_ptr, _SHR_E_INTERNAL, "sorted_list_ptr");

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.in_use.get(unit, &in_use));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists));
    if (in_use >= max_nof_lists)
    {
        /*
         * If number of occupied bitmap structures is beyond the
         * maximum then quit with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    /*
     * Increment number of 'in_use' to cover the one we now intend to capture.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.in_use.set(unit, (in_use + 1)));
    /*
     * Find a free list (a cleared bit in 'occupied_lists'). At this point,
     * there must be one.
     */
    found = 0;
    for (sorted_list_index = 0; sorted_list_index < max_nof_lists; sorted_list_index++)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val));
        if (bit_val == 0)
        {
            /*
             * 'sorted_list_index' is now the index of a free entry.
             */
            found = 1;
            break;
        }
    }
    if (!found)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.occupied_lists.bit_set(unit, sorted_list_index));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.lists_array.alloc(unit, sorted_list_index));
    /*
     * Note that legal handles start at '1', not at '0'.
     */
    UTILEX_SORTED_LIST_CONVERT_SORTEDLIST_INDEX_TO_HANDLE(sorted_list, sorted_list_index);
    /*
     * Set output of this procedure.
     */
    *sorted_list_ptr = sorted_list;

    utilex_sorted_list_info_clear(unit, sorted_list);
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.prime_handle.set(unit, sorted_list_index, init_info.prime_handle));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.sec_handle.set(unit, sorted_list_index, init_info.sec_handle));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.set(unit, sorted_list_index, init_info.list_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.set(unit, sorted_list_index, init_info.key_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.set(unit, sorted_list_index, init_info.data_size));
    if (init_info.get_entry_fun != NULL || init_info.set_entry_fun != NULL)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.get_entry_fun.set(unit, sorted_list_index, init_info.get_entry_fun));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.set_entry_fun.set(unit, sorted_list_index, init_info.set_entry_fun));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.cmp_func_type.set(unit, sorted_list_index, init_info.cmp_func_type));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
    if (list_size == 0 || key_size == 0 || data_size == 0)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    /*
     * calculate the size of pointers (list head and next) according to table size.
     */
    ptr_size = (utilex_log2_round_up(list_size + 2) + (UTILEX_NOF_BITS_IN_BYTE - 1)) / UTILEX_NOF_BITS_IN_BYTE;
    null_ptr = list_size + 1;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.set(unit, sorted_list_index, ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.null_ptr.set(unit, sorted_list_index, null_ptr));
    tmp_size = UTILEX_MAX(ptr_size, data_size);
    /*
     * Allocate the temps buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_data.alloc(unit, sorted_list_index, tmp_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_key.alloc(unit, sorted_list_index, key_size));
    /*
     * allocate buffer for keys
     */
    offset = 0;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.alloc(unit, sorted_list_index, list_size * key_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memset(unit,
                                                               sorted_list_index, offset, list_size * key_size, 0));
    /*
     * allocate buffer for next array (to build the linked list) one additional for head.
     * which is the last in the next pointers array.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.alloc(unit, sorted_list_index, (list_size + 2) * ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.memset(unit, sorted_list_index,
                                                               offset, (list_size + 2) * ptr_size, 0xFF));
    /*
     * allocate buffer for prev array (to build the linked list)
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.alloc(unit, sorted_list_index, (list_size + 2) * ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.memset(unit, sorted_list_index,
                                                               offset, (list_size + 2) * ptr_size, 0xFF));
    /*
     * allocate buffer for the data array
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.alloc(unit, sorted_list_index, list_size * data_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memset(unit, sorted_list_index,
                                                               offset, list_size * data_size, 0));
    {
        uint32 head_place, tail_place;
        /*
         * connect the head with the tail.
         */
        UTILEX_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index, head_place);
        UTILEX_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index, tail_place);
        SHR_IF_ERR_EXIT(utilex_sorted_list_node_link_set(unit, sorted_list, head_place, tail_place));
    }
    /*
     * initialize the data to be mapped to
     */
    utilex_occ_bm_init_info_clear(&btmp_init_info);
    btmp_init_info.size = list_size;
    /*
     * if(SOC_DPP_WB_ENGINE_VAR_NONE != init_info_ptr->wb_var_index) 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_create(unit, &btmp_init_info, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.set(unit, sorted_list_index, memory_use));

#if ENHANCED_SORTED_LIST_SEARCH_FOR_DNX
/* { */
    {
        /*
         * Initialize 'indices' array. Currently, we add this feature to all sorted lists!
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.alloc(unit, sorted_list_index, list_size));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit, sorted_list_index, 0));
    }
/* } */
#endif
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_clear(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list)
{
    uint32 sorted_list_index;
    uint32 data_size, list_size, key_size, ptr_size;
    UTILEX_OCC_BM_PTR memory_use;
    int32 offset;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    /*
     * if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index)  
     */
    {
        offset = 0;
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memset(unit, sorted_list_index,
                                                                   offset, list_size * key_size, 0x00));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.memset(unit, sorted_list_index,
                                                                   offset, (list_size + 2) * ptr_size, 0xFF));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.memset(unit, sorted_list_index,
                                                                   offset, (list_size + 2) * ptr_size, 0xFF));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memset(unit, sorted_list_index,
                                                                   offset, list_size * data_size, 0x00));

    }
    {
        /*
         * connect the head with the tail.
         */
        uint32 head_place, tail_place;
        UTILEX_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index, head_place);
        UTILEX_SORTED_LIST_TAIL_PLACE_GET(sorted_list_index, tail_place);
        SHR_IF_ERR_EXIT(utilex_sorted_list_node_link_set(unit, sorted_list, head_place, tail_place));
    }

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.get(unit, sorted_list_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_clear(unit, memory_use));
    {
        /*
         * Initialize 'indices' array. Currently, we add this feature to all sorted lists!
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit, sorted_list_index, 0));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_destroy(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list)
{
    uint32 sorted_list_index;
    UTILEX_OCC_BM_PTR memory_use;
    uint8 bit_val;
    uint32 in_use;
    uint8 is_allocated;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);
    /*
     * First, mark this sorted list as 'released'
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.in_use.get(unit, &in_use));
    if ((int) in_use < 0)
    {
        /*
         * If number of occupied sortedlist structures goes below zero then quit
         * with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    /*
     * Decrement number of 'in_use' to cover the one we now intend to release.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.in_use.set(unit, (in_use - 1)));
    /*
     * Mark specific sorted list as 'not occupied'
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, sorted_list_index, &bit_val));
    if (bit_val == 0)
    {
        /*
         * If sorted list structure is not indicated as 'occupied' then quit
         * with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.occupied_lists.bit_clear(unit, sorted_list_index));
    /*
     * Free the temps buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_data.free(unit, sorted_list_index));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_key.free(unit, sorted_list_index));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.free(unit, sorted_list_index));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.free(unit, sorted_list_index));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.free(unit, sorted_list_index));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.free(unit, sorted_list_index));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));
    if (is_allocated)
    {
        /*
         * Free 'indices' array if it has been allocated
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.free(unit, sorted_list_index));
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit, sorted_list_index, 0));
    }
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.get(unit, sorted_list_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_destroy(unit, memory_use));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS.lists_array.free(unit, sorted_list_index));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_entry_add(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint8 * success)
{
    uint8 found;
    uint32 curr_node, prev_node;
    uint32 from_top_index_in_indices;
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");
    SHR_NULL_CHECK(data, _SHR_E_INTERNAL, "data");
    SHR_NULL_CHECK(success, _SHR_E_INTERNAL, "success");

    SHR_IF_ERR_EXIT(utilex_sorted_list_find_match_entry(unit,
                                                        sorted_list,
                                                        key,
                                                        data, FALSE, &found, &prev_node, &curr_node,
                                                        &from_top_index_in_indices));
    if (found)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_EXISTS);
        SHR_EXIT();
    }
    /*
     * Allocate new node.
     */
    SHR_IF_ERR_EXIT(utilex_sorted_list_node_alloc(unit,
                                                  sorted_list, key, data, prev_node, curr_node,
                                                  from_top_index_in_indices, success));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_entry_add_by_iter(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER pos,
    uint8 before,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint8 * success)
{
    UTILEX_SORTED_LIST_ITER prev_node, next_node;
    UTILEX_SORTED_LIST_KEY_CMP cmp_key_fun;
    uint32 key_size;
    uint32 sorted_list_index;
    utilex_sorted_list_cmp_func_type_e cmp_func_type;
    /*
     * Initialize var: Just to make 'coverity' happy.
     */
    uint32 from_top_index_in_indices = 0;
    uint8 *tmp_key_ptr;
    uint8 is_allocated;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit, sorted_list_index, &cmp_func_type));

    if (cmp_func_type == UTILEX_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
    {
        cmp_key_fun = utilex_sorted_list_tcam_cmp_priority;
    }
    else
    {
        cmp_key_fun = (UTILEX_SORTED_LIST_KEY_CMP) sal_memcmp;
    }
    /*
     * Verify we are not adding after the end or before the beginning.
     */
    if ((before && pos == UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list)) ||
        ((!before && pos == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list))))
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    /*
     * Verify that this addition keeps the order.
     */
    prev_node = pos;
    next_node = pos;

    if (before)
    {
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux(unit, sorted_list, pos, FALSE, NULL, NULL, &prev_node));

    }
    else
    {
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux(unit, sorted_list, pos, TRUE, NULL, NULL, &next_node));

    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_tmp_key_ptr_from_handle(unit, sorted_list, &tmp_key_ptr));

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));

    if (prev_node != UTILEX_SORTED_LIST_ITER_END(unit, sorted_list)
        && prev_node != UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list))
    {
        SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, prev_node, tmp_key_ptr, NULL));

        if (cmp_key_fun(tmp_key_ptr, key, key_size) > 0)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
            SHR_EXIT();
        }
    }
    if (next_node != UTILEX_SORTED_LIST_ITER_END(unit, sorted_list)
        && next_node != UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list))
    {
        SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, next_node, tmp_key_ptr, NULL));

        if (cmp_key_fun(key, tmp_key_ptr, key_size) > 0)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
            SHR_EXIT();
        }
    }
    {
        uint32 num_elements_on_indices;
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));

        if (is_allocated)
        {
            /*
             * Feature is enabled. Find location of element on 'indices' array.
             */
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit, sorted_list_index,
                                                                                       &num_elements_on_indices));

            if ((next_node == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list))
                || (next_node == UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list)))
            {
                from_top_index_in_indices = 0;
            }
            else if (prev_node == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list)
                     || prev_node == UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list))
            {
                from_top_index_in_indices = num_elements_on_indices - 1;
            }
            else
            {
                /*
                 * Get the  index on 'indices' corresponding to previous element sorted list.
                 * Use it to add entry into 'indices'.
                 */
                SHR_IF_ERR_EXIT(utilex_sorted_list_get_index_from_iter
                                (unit, sorted_list, prev_node, &from_top_index_in_indices));

                from_top_index_in_indices = num_elements_on_indices - 1 - from_top_index_in_indices;
            }
        }
    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_node_alloc(unit,
                                                  sorted_list, key, data, prev_node, next_node,
                                                  from_top_index_in_indices, success));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_entry_update(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data)
{
    uint32 sorted_list_index;
    uint32 data_size, key_size, list_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(data, _SHR_E_INTERNAL, "data");
    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));

    if (iter == UTILEX_SORTED_LIST_NULL || iter >= list_size)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    /*
     * Check to see if the entry exists
     */
    if (data)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));

        /*
         * if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) 
         */
        {
            /*
             * Copy 'data_size' bytes from input 'data' buffer into 'UTILEX_SORTED_LIST_ACCESS_DATA.data'
             * buffer at offset 'data_size * iter'.
             */
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memwrite(unit, sorted_list_index,
                                                                         data, iter * data_size, data_size));

        }
    }
    if (key)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));

        /*
         * if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) 
         */
        {
            /*
             * Copy 'key_size' bytes from input 'key' buffer into 'UTILEX_SORTED_LIST_ACCESS_DATA.keys'
             * buffer at offset 'key_size * iter'.
             */
            SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memwrite(unit, sorted_list_index,
                                                                         key, iter * key_size, key_size));

        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_entry_get_data(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter,
    UTILEX_SORTED_LIST_DATA * const data)
{
    uint32 sorted_list_index;
    uint32 data_size, list_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(data, _SHR_E_INTERNAL, "data");

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));

    if (iter == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list) || iter >= list_size)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));

    /*
     * Copy 'data_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.data' buffer at offset
     * 'data_size * iter' into input 'data' buffer.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memread(unit, sorted_list_index,
                                                                data, iter * data_size, data_size));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_entry_remove_by_iter(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter)
{
    UTILEX_SORTED_LIST_ITER prev, next;
    uint32 sorted_list_index, null_ptr, ptr_size;
    UTILEX_OCC_BM_PTR memory_use;
    uint32 index_in_indices;
    uint8 is_indices_enabled;
    uint32 list_size;
    uint8 occupied;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_indices_enabled));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));
    /*
     * Make sure iterator is within range (not larger than initially assigned memory)
     */
    if (iter >= list_size)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    /*
     * Make sure iterator refers to to an allocated (occupied) entry. If not, quit with error.
     *
     * First load 'memory_use' to get handle of the corresponding bit map.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.get(unit, sorted_list_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_is_occupied(unit, memory_use, iter, &occupied));
    if (!occupied)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    if (is_indices_enabled)
    {
        /*
         * Get the corresponding index on 'indices' before removing from sorted list.
         * Use it, at end of procedure, to remove entry from 'indices'.
         */
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_index_from_iter(unit, sorted_list, iter, &index_in_indices));

    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux(unit, sorted_list, iter, TRUE, NULL, NULL, &next));
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux(unit, sorted_list, iter, FALSE, NULL, NULL, &prev));
    SHR_IF_ERR_EXIT(utilex_sorted_list_node_link_set(unit, sorted_list, prev, next));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.null_ptr.get(unit, sorted_list_index, &null_ptr));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
    /*
     * if(SOC_DPP_WB_ENGINE_VAR_NONE == init_info_ptr->wb_var_index) 
     */
    {
        /*
         * Copy 'ptr_size' bytes from '&null_ptr' (treated as a small buffer) into
         * 'UTILEX_SORTED_LIST_ACCESS_DATA.next' buffer at offset 'ptr_size * iter'.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.memwrite(unit, sorted_list_index,
                                                                     (uint8 *) & null_ptr, iter * ptr_size, ptr_size));
    }
    {
        /*
         * Copy 'ptr_size' bytes from '&null_ptr' (treated as a small buffer) into
         * 'UTILEX_SORTED_LIST_ACCESS_DATA.prev' buffer at offset 'ptr_size * iter'.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.memwrite(unit, sorted_list_index,
                                                                     (uint8 *) & null_ptr, iter * ptr_size, ptr_size));
    }
    /*
     * 'memory_use'  is already allocated at the beginning of this
     * procedure.
     */
    /*
     * Set flag indicating this entry (referred to by 'iter') is not occupied.
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_occup_status_set(unit, memory_use, iter, FALSE));
    if (is_indices_enabled)
    {
        uint32 ii, num_elements_on_indices, node_ptr;

        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit, sorted_list_index,
                                                                                   &num_elements_on_indices));
        if (index_in_indices < (num_elements_on_indices - 1))
        {
            for (ii = index_in_indices + 1; ii < num_elements_on_indices; ii++)
            {
                SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.get(unit, sorted_list_index, ii, &node_ptr));
                SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.set(unit, sorted_list_index, ii - 1, node_ptr));
            }
        }
        num_elements_on_indices -= 1;
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.set(unit, sorted_list_index,
                                                                                   num_elements_on_indices));
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_entry_lookup(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint8 * found,
    UTILEX_SORTED_LIST_ITER * iter)
{
    uint32 curr_node, prev_node;
    uint32 index_in_indices;
    uint32 sorted_list_index, null_ptr;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");
    SHR_NULL_CHECK(iter, _SHR_E_INTERNAL, "iter");
    /*
     * Check to see whether the entry exists
     */
    if (data != NULL)
    {
        SHR_IF_ERR_EXIT(utilex_sorted_list_find_match_entry(unit,
                                                            sorted_list,
                                                            key, data, FALSE, found, &prev_node, &curr_node,
                                                            &index_in_indices));
    }
    else
    {
        SHR_IF_ERR_EXIT(utilex_sorted_list_find_match_entry(unit,
                                                            sorted_list,
                                                            key, data, TRUE, found, &prev_node, &curr_node,
                                                            &index_in_indices));

        if (*found)
        {
            *iter = curr_node;
        }
        else
        {
            *iter = prev_node;
        }
        goto exit;
    }
    if (*found)
    {
        *iter = curr_node;
    }
    else
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.null_ptr.get(unit, sorted_list_index, &null_ptr));

        *iter = null_ptr;
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_entry_value(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data)
{
    uint32 sorted_list_index, key_size, data_size, list_size;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));

    if (iter == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list) || iter >= list_size)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    if (key != NULL)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
        /*
         * read keys
         *
         * Copy 'key_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.keys' buffer at offset
         * 'key_size * iter' into input 'key' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memread(unit, sorted_list_index,
                                                                    key, iter * key_size, key_size));
    }
    if (data != NULL)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
        /*
         * read data
         *
         * Copy 'data_size' bytes from 'UTILEX_SORTED_LIST_ACCESS_DATA.data' buffer at offset
         * 'data_size * iter' into input 'data' buffer.
         */
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memread(unit, sorted_list_index,
                                                                    data, iter * data_size, data_size));
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_get_next(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER * iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data)
{
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(data, _SHR_E_INTERNAL, "data");
    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");

    if (*iter == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list))
    {
        goto exit;
    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux(unit, sorted_list, *iter, TRUE, key, data, iter));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_get_prev(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER * iter,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data)
{
    uint32 head_place;
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(data, _SHR_E_INTERNAL, "data");
    SHR_NULL_CHECK(key, _SHR_E_INTERNAL, "key");

    UTILEX_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index, head_place);
    if (*iter == head_place)
    {
        goto exit;
    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux(unit, sorted_list, *iter, FALSE, key, data, iter));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_get_size_for_save(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 * size)
{
    uint32 bmp_size, total_size;
    uint32 tmp_size;
    uint32 sorted_list_index, ptr_size, key_size, list_size, data_size;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    total_size = 0;
    SHR_NULL_CHECK(size, _SHR_E_INTERNAL, "size");
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
    tmp_size = UTILEX_MAX(ptr_size, data_size);
    /*
     * init info 
     */
    total_size += sizeof(UTILEX_SORTED_LIST_INIT_INFO);
    /*
     * DS data 
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));
    total_size += sizeof(uint8) * tmp_size;
    total_size += sizeof(uint8) * key_size;
    total_size += sizeof(uint8) * list_size * key_size;
    total_size += sizeof(uint8) * (list_size + 2) * ptr_size;
    total_size += sizeof(uint8) * (list_size + 2) * ptr_size;
    total_size += sizeof(uint8) * list_size * data_size;
    /*
     * Initialize the data to be mapped to
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.get(unit, sorted_list_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_get_size_for_save(unit, memory_use, &bmp_size));
    total_size += bmp_size;
    *size = total_size;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_save(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint8 * buffer,
    uint32 buffer_size_bytes,
    uint32 * actual_size_bytes)
{
    uint8 *cur_ptr;
    uint32 tmp_size;
    uint32 cur_size, total_size;
    uint32 sorted_list_index, ptr_size, key_size, list_size, data_size;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_NULL_CHECK(buffer, _SHR_E_INTERNAL, "buffer");
    SHR_NULL_CHECK(actual_size_bytes, _SHR_E_INTERNAL, "actual_size_bytes");
    cur_ptr = buffer;
    total_size = 0;

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
    tmp_size = UTILEX_MAX(ptr_size, data_size);
    /*
     * copy init info structure (UTILEX_SORTED_LIST_INIT_INFO) into supplied (input) buffer area.
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.get
                    (unit, sorted_list_index, (UTILEX_SORTED_LIST_INIT_INFO *) cur_ptr));
    cur_ptr += sizeof(UTILEX_SORTED_LIST_INIT_INFO);
    /*
     * Copy DS data
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_data.memwrite(unit, sorted_list_index, cur_ptr, 0, tmp_size));
    cur_ptr += tmp_size;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_key.memwrite(unit, sorted_list_index, cur_ptr, 0, key_size));
    cur_ptr += key_size;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memwrite(unit, sorted_list_index,
                                                                 cur_ptr, 0, list_size * key_size));
    cur_ptr += (list_size * key_size);
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.memwrite(unit, sorted_list_index,
                                                                 cur_ptr, 0, (list_size + 2) * ptr_size));
    cur_ptr += ((list_size + 2) * ptr_size);
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.memwrite(unit, sorted_list_index,
                                                                 cur_ptr, 0, (list_size + 2) * ptr_size));
    cur_ptr += ((list_size + 2) * ptr_size);
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memwrite(unit, sorted_list_index,
                                                                 cur_ptr, 0, list_size * data_size));
    cur_ptr += (list_size * data_size);
    /*
     * Initialize the data to be mapped to
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.get(unit, sorted_list_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_save(unit, memory_use, cur_ptr, buffer_size_bytes - total_size, &cur_size));
    cur_ptr += cur_size;
    total_size += cur_size;

    *actual_size_bytes = total_size;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_load(
    int unit,
    uint8 ** buffer,
    UTILEX_SORTED_LIST_SW_DB_ENTRY_GET get_entry_fun,
    UTILEX_SORTED_LIST_SW_DB_ENTRY_SET set_entry_fun,
    UTILEX_SORTED_LIST_KEY_CMP cmp_key_fun,
    UTILEX_SORTED_LIST_PTR * sorted_list_ptr)
{
    UTILEX_SORTED_LIST_INIT_INFO local_init_info;
    uint8 *cur_ptr;
    uint32 tmp_size;
    UTILEX_SORTED_LIST_PTR sorted_list;
    uint32 sorted_list_index, ptr_size, key_size, list_size, data_size;
    UTILEX_OCC_BM_PTR memory_use;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);

    SHR_NULL_CHECK(buffer, _SHR_E_INTERNAL, "buffer");
    SHR_NULL_CHECK(sorted_list_ptr, _SHR_E_INTERNAL, "sorted_list_ptr");

    cur_ptr = buffer[0];
    /*
     * copy init info (structure UTILEX_SORTED_LIST_INIT_INFO) from supplied (input) buffer
     * area into local memory and create sorted list.
     */
    sal_memcpy(&local_init_info, cur_ptr, sizeof(UTILEX_SORTED_LIST_INIT_INFO));
    cur_ptr += sizeof(UTILEX_SORTED_LIST_INIT_INFO);
    local_init_info.get_entry_fun = get_entry_fun;
    local_init_info.set_entry_fun = set_entry_fun;
    /*
     * create DS - will not work!! (petra only code)
     */
    SHR_IF_ERR_EXIT(utilex_sorted_list_create(unit, sorted_list_ptr, local_init_info));
    sorted_list = *sorted_list_ptr;
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.get(unit, sorted_list_index, &ptr_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));
    tmp_size = UTILEX_MAX(ptr_size, data_size);
    /*
     * copy DS data 
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_data.memread(unit, sorted_list_index,
                                                                    (uint8 *) cur_ptr, 0, tmp_size));
    cur_ptr += tmp_size;

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.tmp_key.memread(unit, sorted_list_index,
                                                                   (uint8 *) cur_ptr, 0, key_size));
    cur_ptr += key_size;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.keys.memread(unit, sorted_list_index,
                                                                (uint8 *) cur_ptr, 0, list_size * key_size));
    cur_ptr += (list_size * key_size);
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.next.memread(unit, sorted_list_index,
                                                                (uint8 *) cur_ptr, 0, (list_size + 2) * ptr_size));
    cur_ptr += ((list_size + 2) * ptr_size);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.prev.memread(unit, sorted_list_index,
                                                                (uint8 *) cur_ptr, 0, (list_size + 2) * ptr_size));
    cur_ptr += ((list_size + 2) * ptr_size);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.data.memread(unit, sorted_list_index,
                                                                (uint8 *) cur_ptr, 0, list_size * data_size));
    cur_ptr += (list_size * data_size);

    /*
     * Destroy (release) bitmap 
     */
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.get(unit, sorted_list_index, &memory_use));
    SHR_IF_ERR_EXIT(utilex_occ_bm_destroy(unit, memory_use));
    /*
     * Load bitmap 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_load(unit, &cur_ptr, &memory_use));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.set(unit, sorted_list_index, memory_use));
    *buffer = cur_ptr;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_print(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    char table_header[UTILEX_SORTED_LIST_HEADER_SIZE],
    UTILEX_SORTED_LIST_PRINT_VAL print_key,
    UTILEX_SORTED_LIST_PRINT_VAL print_data)
{
    uint32 prev, curr;
    uint32 sorted_list_index;
    uint8 *tmp_data_ptr;
    uint8 *tmp_key_ptr;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);
    /*
     * traverse the sorted list head list.
     */
    LOG_CLI((BSL_META_U(unit, "  %s\n"), table_header));
    UTILEX_SORTED_LIST_HEAD_PLACE_GET(sorted_list_index, prev);
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_tmp_data_ptr_from_handle(unit, sorted_list, &tmp_data_ptr));
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_tmp_key_ptr_from_handle(unit, sorted_list, &tmp_key_ptr));
    do
    {
        /*
         * read next entry.
         */
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_next_aux
                        (unit, sorted_list, prev, TRUE, tmp_key_ptr, tmp_data_ptr, &curr));
        if (curr == UTILEX_SORTED_LIST_ITER_END(unit, sorted_list))
        {
            goto exit;
        }
        LOG_CLI((BSL_META_U(unit, "   %-10u"), curr));
        print_data(tmp_data_ptr);
        print_key(tmp_key_ptr);
        LOG_CLI((BSL_META_U(unit, "\n")));

        prev = curr;
    }
    while (curr != UTILEX_SORTED_LIST_ITER_END(unit, sorted_list));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_info_clear(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list)
{
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.get_entry_fun.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.set_entry_fun.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.prime_handle.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.sec_handle.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.cmp_func_type.set(unit, sorted_list_index,
                                                                     UTILEX_SORTED_LIST_CMP_FUNC_TYPE_DEFAULT));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.ptr_size.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.memory_use.set(unit, sorted_list_index, 0));
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.null_ptr.set(unit, sorted_list_index, 0));
exit:
    SHR_FUNC_EXIT;
}

static void
utilex_db_prio_list_data_encode(
    DNX_TCAM_PRIO_LOCATION * prio_location,
    uint8 * data)
{
    sal_memcpy(data, prio_location, sizeof(DNX_TCAM_PRIO_LOCATION));
}

STATIC shr_error_e
utilex_db_prio_list_priority_value_encode(
    uint32 priority,
    uint8 * data)
{
    uint32 priority_ptr[1];
    *priority_ptr = priority;
    return utilex_U32_to_U8(priority_ptr, sizeof(uint32), data);
}

shr_error_e
utilex_print_indices(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list)
{
    int ii;
    uint32 node_ptr, num_elements_on_indices;
    static int num_invocations = 0;
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    num_invocations++;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit, sorted_list_index,
                                                                               &num_elements_on_indices));

    sal_printf("%s(): Invocation %d Sorted_list %d - Display 'indices' of %d entries\r\n",
               __FUNCTION__, num_invocations, (int) sorted_list, num_elements_on_indices);
    for (ii = 0; ii < num_elements_on_indices; ii++)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.get(unit, sorted_list_index, ii, &node_ptr));

        sal_printf("index %02d value %02d\r\n", ii, node_ptr);
    }

exit:
    SHR_FUNC_EXIT;
}

static void
utilex_print_key(
    uint8 * buffer)
{
    sal_printf("Key: 0x%02X %02X %02X %02X\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
}

static void
utilex_print_data(
    uint8 * buffer)
{
    sal_printf("Data: 0x%02X %02X %02X %02X   %02X %02X %02X %02X\r\n",
               buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
}

shr_error_e
utilex_print_list(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list)
{
    static int num_invocations = 0;
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    num_invocations++;
    sal_printf("%s(): Invocation %d Sorted_list %d - \r\n", __FUNCTION__, num_invocations, (int) sorted_list);
    utilex_sorted_list_print(unit, sorted_list, "Sorted linked list", utilex_print_key, utilex_print_data);
exit:
    SHR_FUNC_EXIT;
}

#if UTILEX_DEBUG
/* { */
/*
 * See dnx_tcam_db_create_unsafe(), dnx_tcam_db_priority_list_entry_add()
 */
shr_error_e
utilex_sorted_list_test_1(
    int unit)
{
    uint32 num_elements, priority_profile, entry_id;
    uint32 prio, priority[2][10];
    uint32 max_nof_entries, active_nof_entries;
    UTILEX_SORTED_LIST_PTR priorities;
    UTILEX_SORTED_LIST_INIT_INFO priorities_init_info;
    uint8 key_buffer[DNX_TCAM_DB_LIST_KEY_SIZE], data_buffer[DNX_TCAM_DB_LIST_DATA_SIZE];
    uint8 entry_added;
    DNX_TCAM_PRIO_LOCATION prio_location;
    shr_error_e res;

    SHR_FUNC_INIT_VARS(unit);

    priority_profile = 1;
    max_nof_entries = sizeof(priority[0]) / sizeof(priority[0][0]);
    {
        /*
         * Select priorities for entries.
         */
        priority[0][0] = 16;
        priority[0][1] = 17;
        priority[0][2] = 15;
        priority[0][3] = 19;
        priority[0][4] = 18;
        priority[0][5] = 20;
        priority[0][6] = 21;
        priority[0][7] = 22;

        priority[1][0] = 44;
        priority[1][1] = 34;
        priority[1][2] = 30;
        priority[1][3] = 38;
        priority[1][4] = 40;
        priority[1][5] = 32;
        priority[1][6] = 42;
        priority[1][7] = 36;
    }
    /*
     * Initialize the entry priority list
     */
    sal_memset(&priorities_init_info, 0x0, sizeof(UTILEX_SORTED_LIST_INIT_INFO));
    priorities_init_info.prime_handle = unit;
    priorities_init_info.sec_handle = 0;
    priorities_init_info.list_size = max_nof_entries;
    priorities_init_info.key_size = DNX_TCAM_DB_LIST_KEY_SIZE * sizeof(uint8);
    priorities_init_info.data_size = DNX_TCAM_DB_LIST_DATA_SIZE * sizeof(uint8);
    priorities_init_info.get_entry_fun = NULL;
    priorities_init_info.set_entry_fun = NULL;
    priorities_init_info.cmp_func_type = UTILEX_SORTED_LIST_CMP_FUNC_TYPE_TCAM;
    SHR_IF_ERR_EXIT(utilex_sorted_list_create(unit, &priorities, priorities_init_info));

    active_nof_entries = max_nof_entries - 2;

    for (entry_id = 0; entry_id < active_nof_entries; entry_id++)
    {
        SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(priority[priority_profile][entry_id], key_buffer));
        dnx_tcam_prio_location_clear(&prio_location);
        prio_location.entry_id_first = entry_id;
        prio_location.entry_id_last = entry_id;
        utilex_db_prio_list_data_encode(&prio_location, data_buffer);

        SHR_IF_ERR_EXIT(utilex_sorted_list_entry_add(unit, priorities, key_buffer, data_buffer, &entry_added));

        utilex_print_indices(unit, priorities);
        utilex_print_list(unit, priorities);
    }
    if (1)
    {
        /*
         * Now exercise illegal remove
         */
        res = utilex_sorted_list_entry_remove_by_iter(unit, priorities, active_nof_entries + 1);
        if (res == _SHR_E_NONE)
        {
            sal_printf
                ("%s(): Succeeded in removing sorted list with handle %d and index %d. Should have failed - There is no such iterator\r\n",
                 __FUNCTION__, (int) priorities, (int) (active_nof_entries + 1));
            SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_INTERNAL,
                                     " Removing an illegal iterator (%d, handle %d) has succeeded!%s\r\n",
                                     (int) (active_nof_entries + 1), (int) priorities, EMPTY);
        }
        else
        {
            sal_printf
                ("%s(): Failed, as expected, trying to remove illegal sorted list entry (with handle %d, iterator %d). SW is OK!\r\n",
                 __FUNCTION__, (int) priorities, (int) (active_nof_entries + 1));
        }
    }
    do
    {
        if (1)
        {
            /*
             * Now exercise bisect. Use utilex_sorted_list_find_higher_eq_key()
             * This test will FAIL if 'indices' is not enabled (ENHANCED_SORTED_LIST_SEARCH_FOR_DNX is zero)
             */
            uint8 indices_is_enabled;
            uint8 found_equal;
            uint8 expected_found_equal;
            UTILEX_SORTED_LIST_ITER iter;
            UTILEX_SORTED_LIST_ITER expected_iter;

            SHR_IF_ERR_EXIT(utilex_sorted_list_is_indices_enabled(unit, priorities, &indices_is_enabled));
            if (!indices_is_enabled)
            {
                sal_printf("%s(): This test is only suited to operate with 'indices' enabled. Quit this test\r\n",
                           __FUNCTION__);
                break;
            }
            utilex_print_list(unit, priorities);
            prio = priority[priority_profile][1];
            /*
             * Results expected from utilex_sorted_list_find_higher_eq_key()
             */
            expected_found_equal = TRUE;
            expected_iter = 1;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_higher_eq_key(unit, priorities, key_buffer, &iter, &found_equal));
            if ((expected_iter == iter) && (expected_found_equal == found_equal))
            {
                sal_printf("%s(): Found matching element (%d, found_equal: %d) for priority %d\r\n",
                           __FUNCTION__, iter, found_equal, prio);
            }
            else
            {
                sal_printf
                    ("%s(): Results are not as expected (iter %d ; expected iter %d, found_equal %d ; expected found_equal %d). Quit! \r\n",
                     __FUNCTION__, iter, expected_iter, found_equal, expected_found_equal);
                SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_INTERNAL, " Unexpected search results. iter %d expected iter %d%s\r\n",
                                         iter, expected_iter, EMPTY);
            }
            prio = priority[priority_profile][4] + 1;
            expected_found_equal = FALSE;
            expected_iter = 6;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_higher_eq_key(unit, priorities, key_buffer, &iter, &found_equal));
            if ((expected_iter == iter) && (expected_found_equal == found_equal))
            {
                sal_printf("%s(): Found matching element (%d, found_equal: %d) for priority %d\r\n",
                           __FUNCTION__, iter, found_equal, prio);
            }
            else
            {
                sal_printf
                    ("%s(): Results are not as expected (iter %d ; expected iter %d, found_equal %d ; expected found_equal %d). Quit! \r\n",
                     __FUNCTION__, iter, expected_iter, found_equal, expected_found_equal);
                SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_INTERNAL, " Unexpected search results. iter %d expected iter %d%s\r\n",
                                         iter, expected_iter, EMPTY);
            }
            prio = priority[priority_profile][0];
            expected_found_equal = TRUE;
            expected_iter = 0;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_higher_eq_key(unit, priorities, key_buffer, &iter, &found_equal));
            if ((expected_iter == iter) && (expected_found_equal == found_equal))
            {
                sal_printf("%s(): Found matching element (%d, found_equal: %d) for priority %d\r\n",
                           __FUNCTION__, iter, found_equal, prio);
            }
            else
            {
                sal_printf
                    ("%s(): Results are not as expected (iter %d ; expected iter %d, found_equal %d ; expected found_equal %d). Quit! \r\n",
                     __FUNCTION__, iter, expected_iter, found_equal, expected_found_equal);
                SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_INTERNAL, " Unexpected search results. iter %d expected iter %d%s\r\n",
                                         iter, expected_iter, EMPTY);
            }
            prio = priority[priority_profile][2];
            /*
             * Results expected from utilex_sorted_list_find_higher_eq_key()
             */
            expected_found_equal = TRUE;
            expected_iter = 2;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_higher_eq_key(unit, priorities, key_buffer, &iter, &found_equal));
            if ((expected_iter == iter) && (expected_found_equal == found_equal))
            {
                sal_printf("%s(): Found matching element (%d, found_equal: %d) for priority %d\r\n",
                           __FUNCTION__, iter, found_equal, prio);
            }
            else
            {
                sal_printf
                    ("%s(): Results are not as expected (iter %d ; expected iter %d, found_equal %d ; expected found_equal %d). Quit! \r\n",
                     __FUNCTION__, iter, expected_iter, found_equal, expected_found_equal);
                SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_INTERNAL, " Unexpected search results. iter %d expected iter %d%s\r\n",
                                         iter, expected_iter, EMPTY);
            }
            prio = priority[priority_profile][0] + 1;
            /*
             * Results expected from utilex_sorted_list_find_higher_eq_key()
             */
            expected_found_equal = FALSE;
            expected_iter = max_nof_entries + 1;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_higher_eq_key(unit, priorities, key_buffer, &iter, &found_equal));
            sal_printf("%s(): Found matching element (%d, found_equal: %d) for priority %d\r\n",
                       __FUNCTION__, iter, found_equal, prio);
            prio = priority[priority_profile][2] - 1;
            /*
             * Results expected from utilex_sorted_list_find_higher_eq_key()
             */
            expected_found_equal = FALSE;
            expected_iter = 2;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_higher_eq_key(unit, priorities, key_buffer, &iter, &found_equal));
            if ((expected_iter == iter) && (expected_found_equal == found_equal))
            {
                sal_printf("%s(): Found matching element (%d, found_equal: %d) for priority %d\r\n",
                           __FUNCTION__, iter, found_equal, prio);
            }
            else
            {
                sal_printf
                    ("%s(): Results are not as expected (iter %d ; expected iter %d, found_equal %d ; expected found_equal %d). Quit! \r\n",
                     __FUNCTION__, iter, expected_iter, found_equal, expected_found_equal);
                SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_INTERNAL, " Unexpected search results. iter %d expected iter %d%s\r\n",
                                         iter, expected_iter, EMPTY);
            }
        }
    }
    while (0);
    do
    {
        if (0)
        {
            /*
             * Now exercise bisect. Use utilex_sorted_list_find_higher_eq_key()
             * This test will FAIL if 'indices' is not enabled (ENHANCED_SORTED_LIST_SEARCH_FOR_DNX is zero)
             */
            uint8 found_equal;
            uint8 indices_is_enabled;
            UTILEX_SORTED_LIST_ITER iter;
            uint32 index_on_indices;

            SHR_IF_ERR_EXIT(utilex_sorted_list_is_indices_enabled(unit, priorities, &indices_is_enabled));
            if (!indices_is_enabled)
            {
                sal_printf("%s(): This test is only suited to operate with 'indices' enabled. Quit this test\r\n",
                           __FUNCTION__);
                break;
            }
            prio = priority[priority_profile][1];
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));

            sal_printf
                ("%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
                 __FUNCTION__, iter, found_equal, prio, index_on_indices);
            prio = priority[priority_profile][4] + 1;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            sal_printf
                ("%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
                 __FUNCTION__, iter, found_equal, prio, index_on_indices);
            prio = priority[priority_profile][0];
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            sal_printf
                ("%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
                 __FUNCTION__, iter, found_equal, prio, index_on_indices);
            prio = priority[priority_profile][2];
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            sal_printf
                ("%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
                 __FUNCTION__, iter, found_equal, prio, index_on_indices);
            prio = priority[priority_profile][0] + 1;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            sal_printf
                ("%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
                 __FUNCTION__, iter, found_equal, prio, index_on_indices);
            prio = priority[priority_profile][2] - 1;
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            sal_printf
                ("%s(): Found matching element (%d found_equal %d) for priority %d index_on_indices %d\r\n",
                 __FUNCTION__, iter, found_equal, prio, index_on_indices);
            /*
             * Exercise removal of one element.
             * priority[priority_profile][1]
             */
            prio = priority[priority_profile][1];
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, priorities, &num_elements));
            sal_printf("%s(): Original num of element %d. Now, remove element %d\r\n", __FUNCTION__, num_elements,
                       iter);
            SHR_IF_ERR_EXIT(utilex_sorted_list_entry_remove_by_iter(unit, priorities, iter));
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, priorities, &num_elements));
            sal_printf("%s(): num of element %d\r\n", __FUNCTION__, num_elements);
            utilex_print_indices(unit, priorities);
            utilex_print_list(unit, priorities);
            /*
             * Exercise removal of one element.
             * priority[priority_profile][0]
             */
            prio = priority[priority_profile][0];
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, priorities, &num_elements));
            sal_printf("%s(): Original num of element %d. Now, remove element %d\r\n", __FUNCTION__, num_elements,
                       iter);
            SHR_IF_ERR_EXIT(utilex_sorted_list_entry_remove_by_iter(unit, priorities, iter));
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, priorities, &num_elements));
            sal_printf("%s(): num of element %d\r\n", __FUNCTION__, num_elements);
            utilex_print_indices(unit, priorities);
            utilex_print_list(unit, priorities);
            /*
             * Exercise removal of one element.
             * priority[priority_profile][2]
             */
            prio = priority[priority_profile][2];
            SHR_IF_ERR_EXIT(utilex_db_prio_list_priority_value_encode(prio, key_buffer));
            SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                            (unit, priorities, key_buffer, &iter, &index_on_indices, &found_equal));
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, priorities, &num_elements));
            sal_printf("%s(): Original num of element %d. Now, remove element %d\r\n", __FUNCTION__, num_elements,
                       iter);
            SHR_IF_ERR_EXIT(utilex_sorted_list_entry_remove_by_iter(unit, priorities, iter));
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, priorities, &num_elements));
            sal_printf("%s(): num of element %d\r\n", __FUNCTION__, num_elements);
            utilex_print_indices(unit, priorities);
            utilex_print_list(unit, priorities);
        }
    }
    while (0);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_info_print(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list)
{
    uint32 key_size, list_size, data_size, sec_handle, sorted_list_index;
    int prime_handle;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.data_size.get(unit, sorted_list_index, &data_size));

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.key_size.get(unit, sorted_list_index, &key_size));

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size));

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.prime_handle.get(unit, sorted_list_index, &prime_handle));

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.sec_handle.get(unit, sorted_list_index, &sec_handle));

    LOG_CLI((BSL_META_U(unit, "init_info.data_size   : %u\n"), data_size));
    LOG_CLI((BSL_META_U(unit, "init_info.key_size    : %u\n"), key_size));
    LOG_CLI((BSL_META_U(unit, "init_info.list_size   : %u\n"), list_size));
    LOG_CLI((BSL_META_U(unit, "init_info.prime_handle: %u\n"), prime_handle));
    LOG_CLI((BSL_META_U(unit, "init_info.sec_handle  : %u\n"), sec_handle));
exit:
    SHR_FUNC_EXIT;
}

/* } */
#endif
/*
 * Utilities related to the 'indices' feature.
 */
/* { */
shr_error_e
utilex_sorted_list_find_higher_eq_key(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_ITER * iter,
    uint8 * found_equal)
{
    uint32 sorted_list_index, matched, index_on_indices, top_index_on_indices, bottom_index_on_indices;
    uint8 is_allocated;
    uint32 num_elements;
    UTILEX_SORTED_LIST_ITER local_iter;
    UTILEX_SORTED_LIST_KEY element_key[DNX_TCAM_DB_LIST_KEY_SIZE];
    UTILEX_SORTED_LIST_KEY_CMP cmp_key_fun;
    utilex_sorted_list_cmp_func_type_e cmp_func_type;
    int32 compare_res;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    *iter = UTILEX_SORTED_LIST_ITER_END(unit, sorted_list);
    *found_equal = 0;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));

    if (!is_allocated)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit, sorted_list_index, &cmp_func_type));
    if (cmp_func_type == UTILEX_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
    {
        cmp_key_fun = utilex_sorted_list_tcam_cmp_priority;
    }
    else
    {
        cmp_key_fun = (UTILEX_SORTED_LIST_KEY_CMP) sal_memcmp;
    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, sorted_list, &num_elements));

    if (num_elements == 0)
    {
        /*
         * List is empty. Return a value of UTILEX_SORTED_LIST_ITER_END
         */
        goto exit;
    }
    /*
     * If input priority is higher than highest on list then return
     * a value of UTILEX_SORTED_LIST_ITER_END
     */
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, num_elements - 1, &local_iter));
    SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));
    compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
    /*
     * Comparison procedure does: compare_res=key-element_key
     */
    if (compare_res > 0)
    {
        /*
         * List has no higher element. Return a value of UTILEX_SORTED_LIST_ITER_END
         */
        goto exit;
    }
    /*
     * If input priority is lower than lowest on list then point to the lowest
     */
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, 0, &local_iter));

    SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));

    compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
    if (compare_res <= 0)
    {
        /*
         * First element is already higher than input priority (key).
         */
        if (compare_res == 0)
        {
            *found_equal = 1;
        }
        *iter = local_iter;
        goto exit;
    }
    /*
     * Input key is within range of the sorted list. Solution MUST be found or there is some error.
     */
    matched = 0;
    top_index_on_indices = num_elements - 1;
    bottom_index_on_indices = 0;
    while (!matched)
    {
        index_on_indices = (top_index_on_indices - bottom_index_on_indices) / 2;
        if (index_on_indices == 0)
        {
            /*
             * We have one or two elements to check. Start by checking the lower one:
             */
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices
                            (unit, sorted_list, bottom_index_on_indices, &local_iter));
            SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));
            compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
            if (compare_res <= 0)
            {
                matched = 1;
                if (compare_res == 0)
                {
                    *found_equal = 1;
                }
                *iter = local_iter;
                goto exit;
            }
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices
                            (unit, sorted_list, top_index_on_indices, &local_iter));
            SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));
            compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
            if (compare_res <= 0)
            {
                matched = 2;
                if (compare_res == 0)
                {
                    *found_equal = 1;
                }
                *iter = local_iter;
                goto exit;
            }
            /*
             * Failed to find matching element. Error!
             */
            SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
            SHR_EXIT();
        }
        index_on_indices += bottom_index_on_indices;
        /*
         * There are more-than-one elements to be inspected.
         */
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, index_on_indices, &local_iter));
        SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));
        /*
         * Comparison procedure does: compare_res=key-element_key
         */
        compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
        if (compare_res <= 0)
        {
            /*
             * Found an element which is higher than input. Bisect the lower half.
             */
            top_index_on_indices = index_on_indices;
            continue;
        }
        else
        {
            /*
             * Middle element which is lower than input. Bisect the upper half.
             */
            bottom_index_on_indices = index_on_indices + 1;
            continue;
        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_find_lower_eq_key(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_ITER * iter,
    uint32 * index_p,
    uint8 * found_equal)
{
    uint32 sorted_list_index, matched, index_on_indices, top_index_on_indices, bottom_index_on_indices;
    uint8 is_allocated;
    uint32 num_elements;
    UTILEX_SORTED_LIST_ITER local_iter;
    UTILEX_SORTED_LIST_KEY element_key[DNX_TCAM_DB_LIST_KEY_SIZE];
    UTILEX_SORTED_LIST_KEY_CMP cmp_key_fun;
    utilex_sorted_list_cmp_func_type_e cmp_func_type;
    int32 compare_res;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    *iter = UTILEX_SORTED_LIST_ITER_BEGIN(unit, sorted_list);
    *index_p = -1;
    *found_equal = 0;
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));

    if (!is_allocated)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_INFO.cmp_func_type.get(unit, sorted_list_index, &cmp_func_type));

    if (cmp_func_type == UTILEX_SORTED_LIST_CMP_FUNC_TYPE_TCAM)
    {
        cmp_key_fun = utilex_sorted_list_tcam_cmp_priority;
    }
    else
    {
        cmp_key_fun = (UTILEX_SORTED_LIST_KEY_CMP) sal_memcmp;
    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_num_elements(unit, sorted_list, &num_elements));

    if (num_elements == 0)
    {
        /*
         * List is empty. Return a value of UTILEX_SORTED_LIST_ITER_BEGIN
         */
        goto exit;
    }
    /*
     * If input priority is lower than lowest on list then return
     * a value of UTILEX_SORTED_LIST_ITER_BEGIN
     */
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, 0, &local_iter));

    SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));

    compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
    /*
     * Comparison procedure does: compare_res=key-element_key
     */
    if (compare_res < 0)
    {
        /*
         * List has no lower element. Return a value of UTILEX_SORTED_LIST_ITER_BEGIN
         */
        goto exit;
    }
    /*
     * If input priority is higher than highest on list then point to the highest
     */
    index_on_indices = num_elements - 1;
    SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, index_on_indices, &local_iter));

    SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));

    compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
    /*
     * Comparison procedure does: compare_res=key-element_key
     */
    if (compare_res >= 0)
    {
        /*
         * Last element is lower than input priority (key).
         */
        if (compare_res == 0)
        {
            *found_equal = 1;
        }
        *index_p = index_on_indices;
        *iter = local_iter;
        goto exit;
    }
    /*
     * Input key is within range of the sorted list. Solution MUST be found or there is some error.
     */
    matched = 0;
    top_index_on_indices = num_elements - 1;
    bottom_index_on_indices = 0;
    while (!matched)
    {
        index_on_indices = (top_index_on_indices - bottom_index_on_indices) / 2;
        if (index_on_indices == 0)
        {
            /*
             * We have one or two elements to check. Start by checking the upper one:
             */
            index_on_indices = top_index_on_indices;
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, index_on_indices, &local_iter));

            SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));

            compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
            /*
             * Comparison procedure does: compare_res=key-element_key
             */
            if (compare_res >= 0)
            {
                matched = 1;
                if (compare_res == 0)
                {
                    *found_equal = 1;
                }
                *index_p = index_on_indices;
                *iter = local_iter;
                goto exit;
            }
            index_on_indices = bottom_index_on_indices;
            SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, index_on_indices, &local_iter));

            SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));

            compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
            if (compare_res >= 0)
            {
                matched = 2;
                if (compare_res == 0)
                {
                    *found_equal = 1;
                }
                *index_p = index_on_indices;
                *iter = local_iter;
                goto exit;
            }
            /*
             * Failed to find matching element. Error!
             */
            SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
            SHR_EXIT();
        }
        index_on_indices += bottom_index_on_indices;
        /*
         * There are more-than-one elements to be inspected.
         */
        SHR_IF_ERR_EXIT(utilex_sorted_list_get_iter_from_indices(unit, sorted_list, index_on_indices, &local_iter));

        SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, local_iter, element_key, NULL));

        /*
         * Comparison procedure does: compare_res=key-element_key
         */
        compare_res = cmp_key_fun(key, element_key, sizeof(element_key));
        if (compare_res >= 0)
        {
            /*
             * Found an element which is lower than input. Bisect the upper half.
             */
            bottom_index_on_indices = index_on_indices;
            continue;
        }
        else
        {
            /*
             * Found an element which is higher than input. Bisect the lower half.
             */
            top_index_on_indices = index_on_indices - 1;
            continue;
        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_get_num_elements(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 * num_elements)
{
    uint32 sorted_list_index;
    uint8 is_allocated;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));

    if (is_allocated)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.num_elements_on_indices.get(unit,
                                                                                   sorted_list_index, num_elements));

    }
    else
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_is_indices_enabled(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint8 * is_enabled)
{
    uint32 sorted_list_index;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, is_enabled));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_get_iter_from_indices(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    uint32 index_in_indices,
    UTILEX_SORTED_LIST_ITER * iter)
{
    uint32 sorted_list_index;
    uint8 is_allocated;

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));
    if (is_allocated)
    {
        SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.get(unit, sorted_list_index, index_in_indices, iter));
    }
    else
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_sorted_list_get_index_from_iter(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_ITER iter,
    uint32 * index_in_indices)
{
    uint32 sorted_list_index;
    uint8 is_allocated;
    uint8 found_equal;
    UTILEX_SORTED_LIST_ITER local_iter;
    UTILEX_SORTED_LIST_KEY element_key[DNX_TCAM_DB_LIST_KEY_SIZE];

    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    UTILEX_SORTED_LIST_VERIFY_SORTEDLIST_IS_ACTIVE(sorted_list_index);

    SHR_IF_ERR_EXIT(UTILEX_SORTED_LIST_ACCESS_DATA.indices.is_allocated(unit, sorted_list_index, &is_allocated));

    if (!is_allocated)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(utilex_sorted_list_entry_value(unit, sorted_list, iter, element_key, NULL));
    SHR_IF_ERR_EXIT(utilex_sorted_list_find_lower_eq_key
                    (unit, sorted_list, element_key, &local_iter, index_in_indices, &found_equal));
    if (!found_equal)
    {
        *index_in_indices = -1;
    }
exit:
    SHR_FUNC_EXIT;
}

/* } */

uint32
utilex_sorted_list_get_iter_begin_or_end(
    int unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    int get_begin)
{
    uint32 res;
    uint32 sorted_list_index;
    uint32 list_size;
    int _func_unit = unit;

    list_size = (uint32) (-1);
    if ((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES))
    {
        /*
         * If this is an illegal unit identifier, quit
         * with error.
         */
        goto exit;
    }
    UTILEX_SORTED_LIST_CONVERT_HANDLE_TO_SORTEDLIST_INDEX(sorted_list_index, sorted_list);
    {
        uint8 bit_val;
        uint32 max_nof_lists;

        res = UTILEX_SORTED_LIST_ACCESS.max_nof_lists.get(unit, &max_nof_lists);
        if (res != _SHR_E_NONE)
        {
            LOG_ERROR_EX(BSL_LOG_MODULE,
                         " Error '%s' indicated ; Location 1 %s%s%s\r\n", shrextend_errmsg_get(res), EMPTY, EMPTY,
                         EMPTY);
            goto exit;
        }
        if (sorted_list_index >= max_nof_lists)
        {
            /*
             * If sortedlist handle is out of range then quit with error.
             */
            bit_val = 0;
        }
        else
        {
            res = UTILEX_SORTED_LIST_ACCESS.occupied_lists.bit_get(unit, (int) sorted_list_index, &bit_val);
            if (res != SOC_E_NONE)
            {
                LOG_ERROR_EX(BSL_LOG_MODULE,
                             " Error '%s' indicated ; Location 2 %s%s%s\r\n", shrextend_errmsg_get(res), EMPTY, EMPTY,
                             EMPTY);
                goto exit;
            }
        }
        if (bit_val == 0)
        {
            /*
             * If sortedlist structure is not indicated as 'occupied' then quit
             * with error.
             */
            goto exit;
        }
    }
    res = UTILEX_SORTED_LIST_ACCESS_INFO.list_size.get(unit, sorted_list_index, &list_size);
    if (res != SOC_E_NONE)
    {
        LOG_ERROR_EX(BSL_LOG_MODULE,
                     " Error '%s' indicated ; Location 3 %s%s%s\r\n", shrextend_errmsg_get(res), EMPTY, EMPTY, EMPTY);
        goto exit;
    }
    if (get_begin == 0)
    {
        list_size += 1;
    }
exit:
    return (list_size);
}

/*
 * }
 */
/* } */
#else
/* { */
/*
 * This is DUMMY code. It is only for compilers that do not accept empty files
 * and is never to be used.
 */
shr_error_e
utilex_sorted_list_node_alloc(
    uint32 unit,
    UTILEX_SORTED_LIST_PTR sorted_list,
    UTILEX_SORTED_LIST_KEY * const key,
    UTILEX_SORTED_LIST_DATA * const data,
    uint32 prev_node,
    uint32 next_node,
    uint32 from_top_index_in_indices,
    uint8 * found)
{
    SHR_FUNC_INIT_VARS(unit);
    UTILEX_SORTED_LIST_VERIFY_UNIT_IS_LEGAL(unit);
exit:
    SHR_FUNC_EXIT;

}

/* } */
#endif /* BCM_DNX_SUPPORT */
