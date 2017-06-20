/* $Id: sand_occupation_bitmap.c,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/** \file utilex_occupation_bitmap.c
 *
 * All common utilities related to occupation bitmap.
 * Only for use by 'DNX' code!!! (This is due to the fact that this
 * utility uses a part of SWSTATE which is assigned to DNX only.)
 *
 * Note:
 * Since some compilers do not accept empty files ("ISO C forbids an empty source file")
 * then dummy code ia added for non-DNX compilation.
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDSWDNX_OCCBMDNX

/*************
* INCLUDES  *
*************/
/* { */

#include <sal/core/libc.h>
/*
 * SW is included
 */
#include <shared/swstate/access/sw_state_access.h>
#include <shared/shrextend/shrextend_debug.h>

#include <shared/utilex/utilex_occupation_bitmap.h>
#include <shared/utilex/utilex_integer_arithmetic.h>

extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];

/* } */

#ifdef BCM_DNX_SUPPORT
/* { */

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

/* } */

/*************
* GLOBALS   *
*************/
/* { */

/**
 * \brief
 * Static global array to efficiently find the first bit set to one in a uint8
 * Index of array starts at '0' so, for example, \n
 *  * First entry is marked 'not meaningful' since '0' has no '1'-bits
 *  * First non-zero bit in 0x01 is '0'
 *  * First non-zero bit in 0x02 is '1'
 *  * First non-zero bit in 0x03 is '0'
 *  * First non-zero bit in 0x04 is '2'
 * Total number of meaningful entries is 256 (= number of bit combinations in a byte)
 */
static uint8 utilex_bitmap_val_to_first_one[UTILEX_OCC_BM_ARRAY_MAP_LEN] = {
    UTILEX_OCC_BM_NODE_IS_FULL,
    0,
    1, 0,
    2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1,
    0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1,
    0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0,
    1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2,
    0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/**
 * \brief
 * Static global array to find, efficiently, the first bit set to zero in a uint8
 * Index of array starts at '0' so, for example, \n
 *  * First zero bit in 0x00 is '0'
 *  * First zero bit in 0x01 is '1'
 *  * First zero bit in 0x02 is '0'
 *  * First zero bit in 0x03 is '2'
 *  * First zero bit in 0x04 is '0'
 *  * Last entry is marked 'not meaningful' since '255' has no '0'-bits
 * Total number of meaningful entries is 256 (= number of bit combinations in a byte)
 */
static uint8 utilex_bitmap_val_to_first_zero[UTILEX_OCC_BM_ARRAY_MAP_LEN] = {
    0,
    1, 0,
    2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1,
    0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1,
    0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0,
    1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2,
    0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    UTILEX_OCC_BM_NODE_IS_FULL
};

/**
 * \brief
 * Static global array to find, efficiently, the last bit set to zero in a uint8
 * (This array can be replaced with simple calculations but needed for high performances.)
 * Index of array starts at '0' so, for example, \n
 *  * First zero bit in 0x00 is '7'
 *  * First zero bit in 0x01 is '7'
 *  * First zero bit in 0x02 is '7'
 *  * First zero bit in 0x03 is '7'
 *  * First zero bit in 0x04 is '7'
 *  * Last entry is marked 'not meaningful' since '255' has no '0'-bits
 * Total number of meaningful entries is 256 (= number of bit combinations in a byte)
 */
static uint8 utilex_bitmap_val_to_last_zero[UTILEX_OCC_BM_ARRAY_MAP_LEN] = {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2,
    1, 1,
    0,
    UTILEX_OCC_BM_NODE_IS_FULL
};

/* } */

/*************
* FUNCTIONS *
*************/
shr_error_e
utilex_occ_bm_init(
    int unit,
    uint32 max_nof_dss)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.alloc(unit));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.ptr_alloc(unit, max_nof_dss));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.max_nof_dss.set(unit, max_nof_dss));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.in_use.set(unit, 0));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.occupied_dss.alloc_bitmap(unit, max_nof_dss));
exit:
    SHR_FUNC_EXIT;
}
#if (0)
/* { */
/*
 * Apparently, this is not requirted for DNX. We leave the code here but,
 * eventually, it should be remove.
 */
/*********************************************************************
* NAME:
*   utilex_occ_bm_get_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get bitmap structure pointer from handle.
* INPUT:
*   int                               unit -
*     Identifier of the device to access.
*   UTILEX_OCC_BM_PTR              bit_map -
*     Handle to bitmap to get pointer to.
* REMARKS:
*   This procedure is exceptional and is added here only for Arad (and
*   on Arad, for pat_tree. See utilex_pat_tree_create()).
*   Range of legal handles: 1 -> max_nof_dss
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_get_ptr_from_handle(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    UTILEX_OCC_BM_T ** bit_map_ptr_ptr)
{
    uint32 bit_map_index;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    *bit_map_ptr_ptr = sw_state[unit]->dnx.shareddnx.sand.occ_bitmap->dss_array[bit_map_index];
exit:
    SHR_FUNC_EXIT;
}
/* } */
#endif
/*********************************************************************
* NAME:
*     utilex_occ_bm_create
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Creates a new bitmap instance.
* INPUT:
*   int                               unit -
*     Identifier of the device to access.
*  UTILEX_OCC_BM_INIT_INFO          info -
*     information needed for the bitmap creation
*  UTILEX_OCC_BM_PTR              *bit_map -
*     Handle to the newly created bitmap.
* REMARKS:
*   Range of legal handles: 1 -> max_nof_dss
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_create(
    int unit,
    UTILEX_OCC_BM_INIT_INFO * init_info,
    UTILEX_OCC_BM_PTR * bit_map)
{
    uint32 found, cur_size, level_size, all_level_size, nof_levels, size;
    uint8 bit_val, init_val;
    uint32 indx, buffer_offset;
    uint32 in_use, max_nof_dss;
    uint32 bit_map_index;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    SHR_NULL_CHECK(bit_map, _SHR_E_INTERNAL, "bit_map");
    SHR_NULL_CHECK(init_info, _SHR_E_INTERNAL, "init_info");
    /*
     * Allocate the bitmap struct
     */
    size = init_info->size;
    if (init_info->init_val == FALSE)
    {
        init_val = 0X0;
    }
    else
    {
        init_val = 0XFF;
    }

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.in_use.get(unit, &in_use));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.max_nof_dss.get(unit, &max_nof_dss));

    if (in_use >= max_nof_dss)
    {
        /*
         * If number of occupied bitmap structures is beyond the maximum then quit
         * with error.
         */
        LOG_CLI((BSL_META_U(unit, "nof occupation bitmaps (%d) exceed the amount allowed per unit (%d). "), in_use,
                 max_nof_dss));
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    /*
     * Increment number of 'in_use' to cover the one we now intend to capture.
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.in_use.set(unit, (in_use + 1)));
    /*
     * Find a free ds (a cleared bit in 'occupied_dss'). At this point,
     * there must be one.
     */
    found = 0;
    for (bit_map_index = 0; bit_map_index < max_nof_dss; bit_map_index++)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.occupied_dss.bit_get(unit, bit_map_index, &bit_val));
        if (bit_val == 0)
        {
            /*
             * 'bit_map_index' is now the index of a free entry.
             */
            found = 1;
            break;
        }
    }
    if (!found)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.occupied_dss.bit_set(unit, bit_map_index));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.alloc(unit, bit_map_index));

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.init_val.set(unit, bit_map_index, init_info->init_val));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.set(unit, bit_map_index, FALSE));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.set(unit,
                                                                         bit_map_index, init_info->support_cache));

    cur_size = UTILEX_NOF_BITS_IN_CHAR; /*in bits */
    nof_levels = 1;
    level_size = UTILEX_NOF_BITS_IN_CHAR;       /*in bits */
    /*
     * calculate the number of levels and allocate the array of levels.
     */
    while (cur_size < size)
    {
        cur_size *= UTILEX_NOF_BITS_IN_CHAR;
        ++nof_levels;
    }
    /*
     * Note that 'alloc' also fills buffer with zero's
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.alloc(unit, bit_map_index, nof_levels));

    if (init_info->support_cache)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.set(unit, bit_map_index, TRUE));
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.alloc(unit, bit_map_index, nof_levels));
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.alloc(unit, bit_map_index, nof_levels));
    /*
     * now allocate the array for all levels.
     */
    all_level_size = 0;
    level_size = size;
    for (indx = nof_levels; indx > 0; --indx)
    {
        level_size = (level_size + (UTILEX_NOF_BITS_IN_CHAR - 1)) / UTILEX_NOF_BITS_IN_CHAR;    /* in chars */

        all_level_size += level_size;
    }
    if (init_info->support_cache)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.alloc(unit,
                                                                                     bit_map_index, all_level_size));
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_buffer.alloc(unit, bit_map_index, all_level_size));
    /*
     * Note that 'alloc' also fills buffer with zero's but 'init_val' may be different.
     */
    if (init_val)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.
                        levels_buffer.memset(unit, bit_map_index, 0, all_level_size, init_val));
    }
    /*
     * now set the offsets array for all levels.
     */
    level_size = size;
    buffer_offset = 0;
    for (indx = nof_levels; indx > 0; --indx)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.set(unit, bit_map_index, indx - 1, level_size));

        level_size = (level_size + (UTILEX_NOF_BITS_IN_CHAR - 1)) / UTILEX_NOF_BITS_IN_CHAR;    /* in chars */
        if (init_info->support_cache)
        {
            /*
             * Set offset within 'levels_cache_buffer' (was 'pointer' on original).
             */
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.set(unit,
                                                                                bit_map_index, indx - 1,
                                                                                buffer_offset));
        }
        /*
         * Set offset within 'levels_buffer' (was 'pointer' on original).
         */
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.set(unit, bit_map_index, indx - 1, buffer_offset));
        /*
         * adding this level size to the buffer offset
         */
        buffer_offset += level_size;

    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.set(unit, bit_map_index, nof_levels));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.size.set(unit, bit_map_index, size));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.buffer_size.set(unit, bit_map_index, all_level_size));
    /*
     * Note that legal handles start at '1', not at '0'.
     */
    UTILEX_OCC_BM_CONVERT_BITMAP_INDEX_TO_HANDLE((*bit_map), bit_map_index);
    /*
     * free_and_exit:
     * In theory, all allocated memory should be freed here but, for now,
     * no action is taken. In the future, calling api will conduct a rollback.
     */

exit:
    SHR_FUNC_EXIT;
}
/*********************************************************************
* NAME:
*     utilex_occ_bm_destroy
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Destroy an existing bitmap instance.
* INPUT:
*   int                 unit -
*     Identifier of the device to access.
*  UTILEX_OCC_BM_PTR bit_map -
*     Handle of the bitmap instance to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_destroy(
    int unit,
    UTILEX_OCC_BM_PTR bit_map)
{
    uint32 bit_map_index;
    uint8 support_cache;
    uint8 bit_val;
    uint32 in_use;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);
    /*
     * First, mark this bit map as 'released'
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.in_use.get(unit, &in_use));
    if ((int) in_use < 0)
    {
        /*
         * If number of occupied bitmap structures goes below zero then quit
         * with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    /*
     * Decrement number of 'in_use' to cover the one we now intend to release.
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.in_use.set(unit, (in_use - 1)));
    /*
     * Mark specific bit map as 'not occupied'
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.occupied_dss.bit_get(unit, bit_map_index, &bit_val));
    if (bit_val == 0)
    {
        /*
         * If bitmap structure is not indicated as 'occupied' then quit
         * with error.
         */
        SHR_SET_CURRENT_ERR(_SHR_E_MEMORY);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.occupied_dss.bit_clear(unit, bit_map_index));

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.free(unit, bit_map_index));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_buffer.free(unit, bit_map_index));

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));

    if (support_cache)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.free(unit, bit_map_index));
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.free(unit, bit_map_index));
    }

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.free(unit, bit_map_index));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.free(unit, bit_map_index));

exit:
    SHR_FUNC_EXIT;
}
/*********************************************************************
* NAME:
*     utilex_occ_bm_clear
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Clear the bitmap content without freeing the memory.
* INPUT:
*   int                 unit -
*     Identifier of the device to access.
*   UTILEX_OCC_BM_PTR bit_map -
*     Handle of the bitmap instance to clear
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_clear(
    int unit,
    UTILEX_OCC_BM_PTR bit_map)
{
    uint32 bit_map_index, level_indx, level_size, ii, buffer_offset;
    uint8 init_val;
    uint32 nof_levels;
    uint8 support_cache, cache_enabled;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.init_val.get(unit, bit_map_index, &init_val));
    if (init_val == FALSE)
    {
        init_val = 0X0;
    }
    else
    {
        init_val = 0XFF;
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.get(unit,
                                                                               bit_map_index, level_indx, &level_size));
            /*
             * Set 'level_size' to the number of bytes containing all bits of that level.
             */
            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            if (support_cache && cache_enabled)
            {
                /*
                 * Get offset within 'levels_cache_buffer' (was 'pointer' on original).
                 */
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.
                                levels_cache.get(unit, bit_map_index, level_indx, &buffer_offset));
                for (ii = 0; ii < level_size; ii++)
                {
                    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.
                                    set(unit, bit_map_index, buffer_offset + ii, init_val));
                }
            }
            else
            {
                /*
                 * Get offset within 'levels_buffer' (was 'pointer' on original).
                 */
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.
                                get(unit, bit_map_index, level_indx, &buffer_offset));
                for (ii = 0; ii < level_size; ii++)
                {
                    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_buffer.
                                    set(unit, bit_map_index, buffer_offset + ii, init_val));
                }
            }
        }
    }
exit:
    SHR_FUNC_EXIT;

}
/*********************************************************************
* NAME:
*     utilex_occ_bm_get_next_helper
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*   Find the next bit with specified val within specified bitmap handle.
* INPUT:
*   int                unit -
*     Identifier of the device to access.
*  UTILEX_OCC_BM_PTR bit_map -
*     The bitmap handle to perform the get operation on.
*   uint8 val -
*     bit value to look for 0/1
*   uint32 level_indx -
*     The level at which to start the search
*   uint32 char_indx -
*     The index of the byte, within the buffer of 'level_indx', at which to start
*     the search.
*   uint32  *place -
*     Start of the found chunk
*   uint8 * found -
*     If specified bit value was found then this procedure loads pointed
*     memory by a non-zero value. Otherwise, it is loaded by '0'. 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static shr_error_e
utilex_occ_bm_get_next_helper(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint8 val,
    uint32 level_indx,
    uint32 char_indx,
    uint32 * place,
    uint8 * found)
{
    uint32 cur_val, cur_bit, level_size;
    uint32 bit_map_index;
    uint32 nof_levels;
    uint8 cur_val_uint8;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    UTILEX_OCC_BM_ACTIVE_INST_GET(bit_map_index, level_indx, char_indx, cur_val_uint8);

    cur_val = cur_val_uint8;
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.get(unit, bit_map_index, level_indx, &level_size));
    /*
     * find the next one/zero in the uint8
     */
    if (val)
    {
        cur_bit = utilex_bitmap_val_to_first_one[cur_val];
    }
    else
    {
        cur_bit = utilex_bitmap_val_to_first_zero[cur_val];
    }
    /*
     * if bit was not found, or was found but after the level size, then return with found = FALSE;
     */
    if (cur_bit == UTILEX_OCC_BM_NODE_IS_FULL || cur_bit + UTILEX_NOF_BITS_IN_CHAR * char_indx >= level_size)
    {
        *place = 0;
        *found = FALSE;
        goto exit;
    }
    /*
     * Reached to the last level (the bit map) return.
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    if (level_indx == nof_levels - 1)
    {
        *place = cur_bit + UTILEX_NOF_BITS_IN_CHAR * char_indx;
        *found = TRUE;
        goto exit;
    }
    /*
     * else continue to the next level
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_get_next_helper(unit,
                                                  bit_map,
                                                  val,
                                                  level_indx + 1,
                                                  cur_bit + UTILEX_NOF_BITS_IN_CHAR * char_indx, place, found));
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*   utilex_occ_bm_calc_mask
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Creates a new bitmap instance.
* INPUT:
*  UTILEX_OCC_BM_PTR bit_map -
*     The bitmap to perform the get operation at.
*   uint8                    val -
*     bit value to look for 0/1
*   uint32  *place -
*     start of the found chunk
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
static shr_error_e
utilex_occ_bm_calc_mask(
    uint32 nof_levels,
    uint32 level_indx,
    uint32 char_indx,
    uint32 down_limit,
    uint32 up_limit,
    uint8 forward)
{
    uint32 range_start, range_end, btmp_start, btmp_end, start, end, mask_val, indx;

    range_start = down_limit;
    range_end = up_limit;

    for (indx = nof_levels - 1; indx > level_indx; --indx)
    {
        range_start = (range_start) / UTILEX_NOF_BITS_IN_CHAR;
        range_end = (range_end) / UTILEX_NOF_BITS_IN_CHAR;
    }

    btmp_start = char_indx * UTILEX_NOF_BITS_IN_CHAR;
    btmp_end = (char_indx + 1) * UTILEX_NOF_BITS_IN_CHAR - 1;

    if (range_start < btmp_start)
    {
        start = 0;
    }
    else if (range_start > btmp_end)
    {
        return UTILEX_U32_MAX;
    }
    else
    {
        start = range_start - btmp_start;
    }

    if (range_end > btmp_end)
    {
        end = UTILEX_NOF_BITS_IN_UINT32 - 1;
    }
    else if (range_end < btmp_start)
    {
        return UTILEX_U32_MAX;
    }
    else
    {
        end = range_end - btmp_start;
    }

    mask_val = 0;
    mask_val = UTILEX_ZERO_BITS_MASK(end, start);
    mask_val &= UTILEX_ZERO_BITS_MASK(UTILEX_NOF_BITS_IN_UINT32 - 1, UTILEX_NOF_BITS_IN_CHAR);

    return mask_val;
}

/*********************************************************************
* NAME:
*     utilex_occ_bm_get_next_range_helper
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*   Look for 'val' within specified range (down_limit <-> up_limit,
*   incl. edges) and indicate whether 'val' has been found or not
*   and where (if found).
* INPUT/OUTPUT:
*   int unit -
*     Identifier of the device to access.
*   UTILEX_OCC_BM_PTR bit_map -
*     The bitmap handle to perform the get operation on.
*   uint8 val -
*     Bit value to look for 0/1
*   uint32 level_indx -
*     'Level', within bitmap, to starts the search at. Note: In most
*     cases, start at level '0'. The following invocations will be
*     via recursion and will have other 'level_indx' values
*   uint32 char_indx -
*     Index of byte, within which, the range of bits to search is contained.
*     Note: In most cases, start at level '0'. The following invocations
*     will be via recursion and will have other 'char_indx' values
*   uint32 down_limit -
*     Index, starting from zero, of location of first bit in the range
*     to search, within 'bit_map'.
*   uint32 up_limit -
*     Index, starting from zero, of location of last bit in the range
*     to search, within 'bit_map'. Note that last bit may also be
*     the first bit.
*   uint8 forward -
*     Flag. If non-zero then this procedure searches starting from
*     'down_limit' towards higher indices. Otherwise, search is from
*     'up_limit' towards lower indices.
*   uint32 *place -
*     Location, in bit index, starting from '0', of matching bit, if
*     found.
*   uint8 *found -
*     This procedure loads pointed memory by a non-zero value if matching
*     bit was, indeed, found within specified range.
* REMARKS:
*   Note that this procedure is called recursively, for each level.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
static shr_error_e
utilex_occ_bm_get_next_range_helper(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint8 val,
    uint32 level_indx,
    uint32 char_indx,
    uint32 down_limit,
    uint32 up_limit,
    uint8 forward,
    uint32 * place,
    uint8 * found)
{
    uint32 cur_val, cur_bit, char_iter, level_size, mask_val, nof_levels;
    uint32 bit_map_index;
    uint8 cur_val_uint8;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.get(unit, bit_map_index, level_indx, &level_size));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));

    UTILEX_OCC_BM_ACTIVE_INST_GET(bit_map_index, level_indx, char_indx, cur_val_uint8);
    cur_val = cur_val_uint8;
    /*
     * Create a mask which has '0' within the range 'down_limit <-> up_limit' (including edges)
     * and '1' outside this range.
     * Note that there are zeroson all bits beyond the 8 LS bits (LS byte)
     * Note that it is possible for 'down_limit' to be equal to 'up_limit'.
     */
    mask_val = utilex_occ_bm_calc_mask(nof_levels, level_indx, char_indx, down_limit, up_limit, forward);
    if (val)
    {
        /*
         * Create a mask which has '1' within the range 'down_limit <-> up_limit' (including edges)
         * and '0' outside this range.
         */
        mask_val = ~mask_val;
        /*
         * Make sure all bits beyond the first 8 (LS byte) are zeroed
         */
        mask_val &= UTILEX_ZERO_BITS_MASK(UTILEX_NOF_BITS_IN_UINT32 - 1, UTILEX_NOF_BITS_IN_CHAR);
        if (level_indx == nof_levels - 1)
        {
            /*
             * If this is the last level,
             * create a value mask which has the original bitmap values within the range 'down_limit <-> up_limit'
             * (including edges) and '0' outside this range.
             */
            cur_val &= mask_val;
        }
        else
        {
            /*
             * If this is not yet the last level,
             * set a value mask which has all '1's within the range to indicate it is worth
             * keeping the search right to the last level.
             */
            cur_val = mask_val;
        }
    }
    else
    {
        /*
         * Create a value mask which has the original bitmap values within the
         * range 'down_limit <-> up_limit' (including edges) and '1' outside this range.
         */
        cur_val |= mask_val;
    }
    /*
     * At the most, there are 8 tries since 'cur_val' has, effectively only
     * 8 significant bits. (See 'cur_val_uint8' above.)
     */
    for (char_iter = 0; char_iter < UTILEX_NOF_BITS_IN_CHAR; ++char_iter)
    {
        if (forward)
        {
            /*
             * Find the next one/zero in the uint8
             */
            if (val)
            {
                cur_bit = utilex_bitmap_val_to_first_one[cur_val];
            }
            else
            {
                cur_bit = utilex_bitmap_val_to_first_zero[cur_val];
            }
        }
        else
        {
            if (val)
            {
                cur_bit = utilex_bitmap_val_to_last_zero[255 - cur_val];
            }
            else
            {
                cur_bit = utilex_bitmap_val_to_last_zero[cur_val];
            }
        }
        /*
         * if bit was not found, or was found but after the level size, then return with found = FALSE;
         */
        if (cur_bit == UTILEX_OCC_BM_NODE_IS_FULL || cur_bit + UTILEX_NOF_BITS_IN_CHAR * char_indx >= level_size)
        {
            *place = 0;
            *found = FALSE;
            goto exit;
        }
        /*
         * If procedure has managed to get to the last level (the bit map), then match
         * has been found. Return with corresponding location (bit index within bit_map).
         */
        if (level_indx == nof_levels - 1)
        {
            *place = cur_bit + UTILEX_NOF_BITS_IN_CHAR * char_indx;
            *found = TRUE;
            goto exit;
        }
        /*
         * This bit was tried. Don't try it again!
         */
        if (val)
        {
            cur_val &= UTILEX_RBIT(cur_bit);
        }
        else
        {
            cur_val |= UTILEX_BIT(cur_bit);
        }
        /*
         * else continue to the next level
         */
        SHR_IF_ERR_EXIT(utilex_occ_bm_get_next_range_helper(unit,
                                                            bit_map,
                                                            val,
                                                            level_indx + 1,
                                                            cur_bit + UTILEX_NOF_BITS_IN_CHAR * char_indx,
                                                            down_limit, up_limit, forward, place, found));
        if (*found)
        {
            goto exit;
        }
    }
    *place = 0;
    *found = FALSE;

exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_occ_bm_get_next
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Creates a new bitmap instance.
* INPUT:
*   int                unit -
*     Identifier of the device to access.
*  UTILEX_OCC_BM_PTR bit_map -
*     Handle of the bitmap to perform the get operation on.
*   uint32           *place -
*     the next place (starting from zero) with bit = 0.
*   uint8            *found -
*     whether a bit with zero was found in the bitmap
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_get_next(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 * place,
    uint8 * found)
{
    uint32 bit_map_index;
    uint32 size;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(utilex_occ_bm_get_next_helper(unit, bit_map, 0, 0, 0, place, found));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.size.get(unit, bit_map_index, &size));
    if (*place >= size)
    {
        *place = 0;
        *found = FALSE;
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_get_next_in_range(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 start,
    uint32 end,
    uint8 forward,
    uint32 * place,
    uint8 * found)
{
    uint32 bit_map_index;
    uint32 size;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(utilex_occ_bm_get_next_range_helper(unit, bit_map, 0, 0, 0, start, end, forward, place, found));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.size.get(unit, bit_map_index, &size));
    if (*place >= size)
    {
        *place = 0;
        *found = FALSE;
    }
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_occ_bm_alloc_next
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Creates a new bitmap instance.
* INPUT:
*   int                unit -
*     Identifier of the device to access.
*  UTILEX_OCC_BM_PTR bit_map -
*     Handle of the bitmap to perform the alloc_next operation on.
*   uint32           *place -
*     the allocated place.
*   uint8            *found-
*     whether a bit with zero was found in the bitmap
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_alloc_next(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 * place,
    uint8 * found)
{
    uint32 bit_map_index;
    uint32 size;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(utilex_occ_bm_get_next(unit, bit_map, place, found));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.size.get(unit, bit_map_index, &size));

    if (*place >= size)
    {
        *place = 0;
        *found = FALSE;
        goto exit;
    }

    SHR_IF_ERR_EXIT(utilex_occ_bm_occup_status_set(unit, bit_map, *place, 1));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_occup_status_set_helper(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 level_indx,
    uint32 char_indx,
    uint32 bit_indx,
    uint8 occupied)
{
    uint32 bit_map_index, old_val, new_val;
    uint8 new_val_uint8;
    uint8 old_val_uint8;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);
    /*
     * Make sure we keep type conversion (uint8/uint32) as per original code.
     */
    UTILEX_OCC_BM_ACTIVE_INST_GET(bit_map_index, level_indx, char_indx, old_val_uint8);
    old_val = old_val_uint8;
    new_val = old_val;

    SHR_IF_ERR_EXIT(utilex_set_field(&new_val, bit_indx, bit_indx, occupied));

    {
        /*
         * Convert type since all buffers are managed by bytes.
         */
        new_val_uint8 = new_val;
        UTILEX_OCC_BM_ACTIVE_INST_SET(bit_map_index, level_indx, char_indx, new_val_uint8);
    }
    /*
     * check whether there is need to update the higher levels
     */
    if (level_indx == 0)
    {
        goto exit;
    }

    if ((occupied && old_val != 0xFF && new_val == 0xFF) || (!occupied && old_val == 0xFF && new_val != 0xFF))
    {
        SHR_IF_ERR_EXIT(utilex_occ_bm_occup_status_set_helper(unit,
                                                              bit_map,
                                                              level_indx - 1,
                                                              char_indx / UTILEX_NOF_BITS_IN_CHAR,
                                                              char_indx % UTILEX_NOF_BITS_IN_CHAR, occupied));
    }
exit:
    SHR_FUNC_EXIT;
}

/*********************************************************************
* NAME:
*     utilex_occ_bm_occup_status_set
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Set the occupation status a of sequence of bits.
* INPUT:
*   int                   unit -
*     Identifier of the device to access.
*  UTILEX_OCC_BM_PTR bit_map -
*     Handle of the bitmap to perform the set operation on.
*  uint32                 place -
*     bit to start the setting from
*  uint8                  occupied -
*     the status (occupied/unoccupied) to set for the give bits.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_occup_status_set(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 place,
    uint8 occupied)
{
    uint32 bit_map_index;
    uint32 nof_levels;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));

    SHR_IF_ERR_EXIT(utilex_occ_bm_occup_status_set_helper(unit,
                                                          bit_map,
                                                          nof_levels - 1,
                                                          place / UTILEX_NOF_BITS_IN_CHAR,
                                                          place % UTILEX_NOF_BITS_IN_CHAR, occupied));
exit:
    SHR_FUNC_EXIT;

}

/*********************************************************************
* NAME:
*     utilex_occ_bm_is_occupied
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*     Get the occupation status a of sequence of bits.
* INPUT:
*   int                unit -
*     Identifier of the device to access.
*  UTILEX_OCC_BM_PTR bit_map -
*     Handle of the bitmap to perform the get operation at.
*  uint32              place -
*     bit to get the status (occupied/unoccupied) for.
*  uint8             *occupied -
*     the status (occupied/unoccupied) of the given bit
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_bm_is_occupied(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 place,
    uint8 * occupied)
{
    uint32 bit_map_index, char_indx, val, bit_indx;
    uint32 nof_levels;
    uint8 val_uint8;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_NULL_CHECK(occupied, _SHR_E_INTERNAL, "occupied");
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    char_indx = place / UTILEX_NOF_BITS_IN_CHAR;
    UTILEX_OCC_BM_ACTIVE_INST_GET(bit_map_index, nof_levels - 1, char_indx, val_uint8);
    val = val_uint8;
    bit_indx = place % UTILEX_NOF_BITS_IN_CHAR;

    val = UTILEX_GET_BITS_RANGE(val, bit_indx, bit_indx);

    *occupied = UTILEX_NUM2BOOL(val);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_cache_set(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint8 cached)
{
    uint32 bit_map_index, level_indx, level_size, ii;
    uint8 tmp_uint8, support_cache, cache_enabled;
    uint32 nof_levels;
    uint32 levels_cache_offset, levels_offset;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));
    if (!support_cache && cached)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    /*
     * if not changing status skip it
     */
    if (cache_enabled == cached)
    {
        goto exit;
    }
    {
        cache_enabled = cached;
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.set(unit, bit_map_index, cache_enabled));
    }
    /*
     * if cache enabled, copy status to cache instance
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    if (cached)
    {
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                            get(unit, bit_map_index, level_indx, &level_size));
            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.
                            get(unit, bit_map_index, level_indx, &levels_cache_offset));
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.
                            get(unit, bit_map_index, level_indx, &levels_offset));
            for (ii = 0; ii < level_size; ii++)
            {
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_buffer.
                                get(unit, bit_map_index, levels_offset + ii, &tmp_uint8));
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.
                                set(unit, bit_map_index, levels_cache_offset + ii, tmp_uint8));
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_cache_commit(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 flags)
{
    uint32 bit_map_index, level_indx, level_size, ii;
    uint32 levels_cache_offset, levels_offset;
    uint8 support_cache;
    uint8 tmp_uint8, cache_enabled;
    uint32 nof_levels;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));
    if (!support_cache)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    /*
     * if cached disabled
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    if (!cache_enabled)
    {
        goto exit;
    }
    /*
     * if cache enabled, copy status to cache instance
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    {
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                            get(unit, bit_map_index, level_indx, &level_size));

            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.
                            get(unit, bit_map_index, level_indx, &levels_cache_offset));
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.
                            get(unit, bit_map_index, level_indx, &levels_offset));
            for (ii = 0; ii < level_size; ii++)
            {
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.
                                get(unit, bit_map_index, levels_cache_offset + ii, &tmp_uint8));
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_buffer.
                                set(unit, bit_map_index, levels_offset + ii, tmp_uint8));
            }
        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_cache_rollback(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 flags)
{
    uint32 bit_map_index, level_indx, level_size;
    uint8 support_cache;
    uint8 tmp_uint8, cache_enabled;
    uint32 nof_levels;
    uint32 ii, levels_cache_offset, levels_offset;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));
    if (!support_cache)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        SHR_EXIT();
    }
    /*
     * if cached disabled
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    if (!cache_enabled)
    {
        goto exit;
    }
    /*
     * if cache enabled, copy status to cache instance
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    {
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                            get(unit, bit_map_index, level_indx, &level_size));
            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.
                            get(unit, bit_map_index, level_indx, &levels_cache_offset));
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.
                            get(unit, bit_map_index, level_indx, &levels_offset));
            for (ii = 0; ii < level_size; ii++)
            {
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_buffer.
                                get(unit, bit_map_index, levels_offset + ii, &tmp_uint8));
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.
                                set(unit, bit_map_index, levels_cache_offset + ii, tmp_uint8));
            }
        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_get_size_for_save(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 * size)
{
    uint32 bit_map_index, level_indx, level_size;
    uint32 cur_size = 0;
    uint32 nof_levels;
    uint8 cache_enabled;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    cur_size += sizeof(uint8) * 2;
    cur_size += sizeof(uint32) * 1;

    /*
     * copy DS data 
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    for (level_indx = 0; level_indx < nof_levels; ++level_indx)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                        get(unit, bit_map_index, level_indx, &level_size));
        level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
        cur_size += level_size * sizeof(uint8);
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    if (cache_enabled)
    {
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                            get(unit, bit_map_index, level_indx, &level_size));
            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            cur_size += level_size * sizeof(uint8);
        }
    }
    *size = cur_size;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_save(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint8 * buffer,
    uint32 buffer_size_bytes,
    uint32 * actual_size_bytes)
{
    uint32 bit_map_index, level_indx, level_size;
    uint8 *cur_ptr = (uint8 *) buffer;
    uint32 ii, cur_size;
    uint8 init_val;
    uint32 size;
    uint8 support_cache;
    uint32 nof_levels;
    uint32 levels_offset, levels_cache_offset;
    uint8 cache_enabled;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    /*
     * copy init info 
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.init_val.get(unit, bit_map_index, &init_val));
    sal_memcpy(cur_ptr, &(init_val), sizeof(uint8));
    cur_ptr += sizeof(uint8);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.size.get(unit, bit_map_index, &size));
    sal_memcpy(cur_ptr, &(size), sizeof(uint32));
    cur_ptr += sizeof(uint32);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));
    sal_memcpy(cur_ptr, &(support_cache), sizeof(uint8));
    cur_ptr += sizeof(uint8);

    /*
     * copy DS data 
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    for (level_indx = 0; level_indx < nof_levels; ++level_indx)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                        get(unit, bit_map_index, level_indx, &level_size));
        level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
        cur_size = level_size * sizeof(uint8);
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.get(unit, bit_map_index, level_indx, &levels_offset));
        for (ii = 0; ii < cur_size; ii++)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.
                            levels_buffer.get(unit, bit_map_index, levels_offset + ii, cur_ptr));
            cur_ptr++;
        }
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    if (cache_enabled)
    {
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                            get(unit, bit_map_index, level_indx, &level_size));

            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            cur_size = level_size * sizeof(uint8);
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.
                            get(unit, bit_map_index, level_indx, &levels_cache_offset));
            for (ii = 0; ii < cur_size; ii++)
            {
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.
                                get(unit, bit_map_index, levels_cache_offset + ii, cur_ptr));
                cur_ptr++;
            }
        }
    }
    *actual_size_bytes = cur_ptr - buffer;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_load(
    int unit,
    uint8 ** buffer,
    UTILEX_OCC_BM_PTR * bit_map)
{
    UTILEX_OCC_BM_INIT_INFO bm_restored;
    uint32 level_indx, level_size;
    uint8 *cur_ptr = (uint8 *) buffer[0];
    uint32 cur_size;
    uint32 nof_levels;
    uint32 ii, bit_map_index, levels_offset, levels_cache_offset;
    uint8 cache_enabled;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);

    utilex_occ_bm_init_info_clear(&bm_restored);
    /*
     * copy init info 
     */
    sal_memcpy(&(bm_restored.init_val), cur_ptr, sizeof(uint8));
    cur_ptr += sizeof(uint8);

    sal_memcpy(&(bm_restored.size), cur_ptr, sizeof(uint32));
    cur_ptr += sizeof(uint32);

    sal_memcpy(&(bm_restored.support_cache), cur_ptr, sizeof(uint8));
    cur_ptr += sizeof(uint8);

    /*
     * create DS 
     */
    SHR_IF_ERR_EXIT(utilex_occ_bm_create(unit, &bm_restored, bit_map));

    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, (*bit_map));
    /*
     * fill DS info
     */
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    for (level_indx = 0; level_indx < nof_levels; ++level_indx)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                        get(unit, bit_map_index, level_indx, &level_size));
        level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
        cur_size = level_size * sizeof(uint8);
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.get(unit, bit_map_index, level_indx, &levels_offset));
        for (ii = 0; ii < cur_size; ii++)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.
                            levels_buffer.set(unit, bit_map_index, levels_offset + ii, *cur_ptr));
            cur_ptr++;
        }
    }
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    if (cache_enabled)
    {
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                            get(unit, bit_map_index, level_indx, &level_size));
            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            cur_size = level_size * sizeof(uint8);
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.
                            get(unit, bit_map_index, level_indx, &levels_cache_offset));
            for (ii = 0; ii < cur_size; ii++)
            {
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.
                                set(unit, bit_map_index, levels_cache_offset + ii, *cur_ptr));
                cur_ptr++;
            }
        }
    }
    *buffer = cur_ptr;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_init_info_clear(
    UTILEX_OCC_BM_INIT_INFO * info)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);
    SHR_NULL_CHECK(info, _SHR_E_INTERNAL, "info");

    sal_memset(info, 0x0, sizeof(UTILEX_OCC_BM_INIT_INFO));
    info->size = 0;
    info->init_val = FALSE;
    info->support_cache = FALSE;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_get_buffer_size(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint32 * buffer_size_p)
{
    uint32 bit_map_index, buffer_size;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.buffer_size.get(unit, bit_map_index, &buffer_size));
    *buffer_size_p = buffer_size;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_get_support_cache(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint8 * support_cache_p)
{
    uint32 bit_map_index;
    uint8 support_cache;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));
    *support_cache_p = support_cache;
exit:
    SHR_FUNC_EXIT;
}

/*
 * Get handle to occ bitmap which will be considered illegal
 * by all occ bitmap utilities.
 * Legal values for 'handle' are 1 -> MAX_NOF_DSS_FOR_DNX or, to be more precise:
 * 1 -> max_nof_dss (The value of UTILEX_OCC_BITMAP_ACCESS.max_nof_dss.get(unit, &max_nof_dss))
 */
uint32
utilex_occ_bm_get_illegal_bitmap_handle(
    void)
{
    return ((uint32) (-1));
}
/*********************************************************************
* NAME:
*   utilex_occ_is_bitmap_active
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*   Get indication on whether specified bitmap handle is currently in use.
* INPUT:
*  int unit -
*    Identifier of the device to access.
*  UTILEX_OCC_BM_PTR bit_map -
*    Handle of the bitmap to perform the check on.
*  uint8 *in_use -
*    Return a non-zero value if bit map is in use.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
shr_error_e
utilex_occ_is_bitmap_active(
    int unit,
    UTILEX_OCC_BM_PTR bit_map,
    uint8 * in_use)
{
    uint32 bit_map_index;
    uint32 max_nof_dss;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    *in_use = 0;
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.max_nof_dss.get(unit, &max_nof_dss));
    if (bit_map_index < max_nof_dss)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.occupied_dss.bit_get(unit, (int) bit_map_index, in_use));
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_occ_bm_print(
    int unit,
    UTILEX_OCC_BM_PTR bit_map)
{
    uint32
        bit_map_index, char_indx, level_indx, level_size, levels_cache_offset, levels_offset, nof_levels, size,
        buffer_size;
    uint8 level_bits;
    uint8 init_val, support_cache, cache_enabled;

    SHR_FUNC_INIT_VARS(unit);

    UTILEX_OCC_BM_VERIFY_UNIT_IS_LEGAL(unit);
    UTILEX_OCC_BM_CONVERT_HANDLE_TO_BITMAP_INDEX(bit_map_index, bit_map);
    UTILEX_OCC_BM_VERIFY_BITMAP_IS_ACTIVE(bit_map_index);

    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.size.get(unit, bit_map_index, &size));
    LOG_CLI((BSL_META_U(unit, "size %u. "), size));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.init_val.get(unit, bit_map_index, &init_val));
    LOG_CLI((BSL_META_U(unit, "init_val %u. "), init_val));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.support_cache.get(unit, bit_map_index, &support_cache));
    LOG_CLI((BSL_META_U(unit, "support_cache %u. "), support_cache));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.cache_enabled.get(unit, bit_map_index, &cache_enabled));
    LOG_CLI((BSL_META_U(unit, "cache_enabled %u. "), cache_enabled));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.buffer_size.get(unit, bit_map_index, &buffer_size));
    LOG_CLI((BSL_META_U(unit, "buffer_size %u. "), buffer_size));
    LOG_CLI((BSL_META_U(unit, "\n")));
    SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.nof_levels.get(unit, bit_map_index, &nof_levels));
    LOG_CLI((BSL_META_U(unit, "nof_levels %u. "), nof_levels));
    LOG_CLI((BSL_META_U(unit, "Bare levels: ")));
    LOG_CLI((BSL_META_U(unit, "\n")));
    for (level_indx = 0; level_indx < nof_levels; ++level_indx)
    {
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                        get(unit, bit_map_index, level_indx, &level_size));
        level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
        LOG_CLI((BSL_META_U(unit, "level %u. level size %u:"), level_indx, level_size));
        SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels.get(unit, bit_map_index, level_indx, &levels_offset));
        for (char_indx = 0; char_indx < level_size; ++char_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_buffer.
                            get(unit, bit_map_index, levels_offset + char_indx, &level_bits));
            LOG_CLI((BSL_META_U(unit, "%02x  "), level_bits));
        }
        LOG_CLI((BSL_META_U(unit, "\n")));
    }
    if (cache_enabled)
    {
        LOG_CLI((BSL_META_U(unit, "Cache levels: ")));
        LOG_CLI((BSL_META_U(unit, "\n")));
        for (level_indx = 0; level_indx < nof_levels; ++level_indx)
        {
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_size.
                            get(unit, bit_map_index, level_indx, &level_size));
            level_size = (level_size + UTILEX_NOF_BITS_IN_CHAR - 1) / UTILEX_NOF_BITS_IN_CHAR;
            SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache.
                            get(unit, bit_map_index, level_indx, &levels_cache_offset));
            for (char_indx = 0; char_indx < level_size; char_indx++)
            {
                SHR_IF_ERR_EXIT(UTILEX_OCC_BITMAP_ACCESS.dss_array.levels_cache_buffer.
                                get(unit, bit_map_index, levels_cache_offset + char_indx, &level_bits));
                LOG_CLI((BSL_META_U(unit, "%02x  "), level_bits));
            }
            LOG_CLI((BSL_META_U(unit, "\n")));
        }
    }

    LOG_CLI((BSL_META_U(unit, "\n\n\n")));

exit:
    SHR_FUNC_EXIT;

}
#if UTILEX_DEBUG
/* { */

/*****************************************************
*NAME
*  utilex_occ_bm_test_1
*TYPE:
*  PROC
*DATE:
*  15-Feb-2008
*FUNCTION:
* Verification that a MC-Group that requires more than available will fail to open
* 1.  Open many large groups (8*2K Ingress) and (7*2K Egress) MC-Groups.
*     And open one more Egress MC-Group with 1 entry (MC-ID = 16K-2K = 14K).
* 2.  Then try and open an Ingress MC-Group with 2K entries (MC-ID = 1).
*     Expect INSUFFICIENT MEMORY value to be TRUE.
* 3.  Delete the Egress group with 1 entry (MC-ID = 14K).
* 4.  Open an Ingress MC-Group with 2K entries (MC-ID = 1). Expect success.
*
*INPUT:
*  UTILEX_DIRECT:
*    uint32 silent -
*    Indicator.
*    1 - Do not print debuging info.
*    0 - Print various debuging info.
*  None.
*OUTPUT:
*  UTILEX_DIRECT:
*    uint32 -
*    Indicator.
*    1 - Test pass.
*    0 - Test fail.
*  NON
*REMARKS:
*
*SEE ALSO:
*****************************************************/
static uint32
utilex_occ_bm_test_1(
    int unit,
    uint8 silent)
{
    int32 pass = TRUE;
    uint32 indx, place, num_bits;
    uint8 found;
    UTILEX_OCC_BM_PTR bitmap;
    UTILEX_OCC_BM_INIT_INFO btmp_init_info;
    shr_error_e utilex_ret;

    num_bits = 64;
    utilex_ret = utilex_occ_bm_init_info_clear(&btmp_init_info);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    btmp_init_info.size = num_bits;

    utilex_ret = utilex_occ_bm_create(unit, &btmp_init_info, &bitmap);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }

    for (indx = 0; indx < (num_bits + 2); ++indx)
    {
        utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
        if (indx >= num_bits)
        {
            /*
             * At this point, the bitmap is full. We should get a zero 'found'
             * and, then, loop.
             */
            if (!silent)
            {
                LOG_CLI((BSL_META_U(unit,
                                    "utilex_occ_bm_test_1: Trying to find '0' on a full bitmap. indx %d num_bits %d found %d\r\n"),
                         (int) indx, (int) num_bits, (int) found));
            }
            if (found)
            {
                if (!silent)
                {
                    LOG_CLI((BSL_META_U(unit,
                                        "Failed in utilex_occ_bm_get_next: Got non-zero 'found' on a full bitmap. indx %d num_bits %d found %d\r\n"),
                             (int) indx, (int) num_bits, (int) found));
                }
                pass = FALSE;
                goto exit;
            }
            continue;
        }
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        if (indx == 19)
        {
            /*
             * At this point, 19 bits have bit set (from BIT0 to BIT18)
             * so the image of the bit map is:
             * 0xFF 0xFF 0x07
             */
            uint32 local_place, start, end, ii;
            uint8 local_found;
            uint8 val;
            uint8 forward[2];
            forward[0] = TRUE;
            forward[1] = FALSE;
            for (ii = 0; ii < 2; ii++)
            {
                start = 14;
                end = 21;
                utilex_ret =
                    utilex_occ_bm_get_next_in_range(unit, bitmap, start, end, forward[ii], &local_place, &local_found);
                if (SHR_FAILURE(utilex_ret))
                {
                    goto exit;
                }
                if (ii == 0)
                {
                    /*
                     * This is a forward search so the first '0' is the one following the
                     * last bit that was set (BIT18)
                     */
                    if (!(local_found && (local_place == 19)))
                    {
                        if (!silent)
                        {
                            LOG_CLI((BSL_META_U(unit,
                                                "Failed in utilex_occ_bm_destroy: Found should be TRUE and place should be 19\r\n"
                                                "==> local_found %d local_place %d\r\n"),
                                     (int) local_found, (int) local_place));
                        }
                        pass = FALSE;
                        goto exit;
                    }
                }
                else
                {
                    /*
                     * This is a backward search so the first '0' is the one at the
                     * end of the range (BIT21)
                     */
                    if (!(local_found && (local_place == 21)))
                    {
                        if (!silent)
                        {
                            LOG_CLI((BSL_META_U(unit,
                                                "Failed in utilex_occ_bm_destroy: Found should be TRUE and place should be 21\r\n"
                                                "==> local_found %d local_place %d\r\n"),
                                     (int) local_found, (int) local_place));
                        }
                        pass = FALSE;
                        goto exit;
                    }
                }
                start = 15;
                end = 21;
                val = 1;
                utilex_ret =
                    utilex_occ_bm_get_next_range_helper(unit, bitmap, val, 0, 0, start, end, forward[ii], &local_place,
                                                        &local_found);
                if (SHR_FAILURE(utilex_ret))
                {
                    goto exit;
                }
                if (ii == 0)
                {
                    /*
                     * This is a forward search so the first '1' is the first
                     * bit in the range (BIT15)
                     */
                    if (!(local_found && (local_place == 15)))
                    {
                        if (!silent)
                        {
                            LOG_CLI((BSL_META_U(unit,
                                                "Failed in utilex_occ_bm_destroy: Found should be TRUE and place should be 17\r\n"
                                                "==> local_found %d local_place %d\r\n"),
                                     (int) local_found, (int) local_place));
                        }
                        pass = FALSE;
                        goto exit;
                    }
                }
                else
                {
                    /*
                     * This is a backward search so the first '1' is the
                     * last bit that was set (BIT18)
                     */
                    if (!(local_found && (local_place == 18)))
                    {
                        if (!silent)
                        {
                            LOG_CLI((BSL_META_U(unit,
                                                "Failed in utilex_occ_bm_destroy: Found should be TRUE and place should be 19\r\n"
                                                "==> local_found %d local_place %d\r\n"),
                                     (int) local_found, (int) local_place));
                        }
                        pass = FALSE;
                        goto exit;
                    }
                }
            }

        }

        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
            if (found)
            {
                utilex_occ_bm_print(unit, bitmap);
            }
        }

        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
    }
exit:
    if (bitmap)
    {
        utilex_ret = utilex_occ_bm_destroy(unit, bitmap);

        if (SHR_FAILURE(utilex_ret))
        {
            LOG_CLI((BSL_META_U(unit, "Failed in utilex_occ_bm_destroy\n")));
            pass = FALSE;
        }
    }
    if (!silent)
    {
        LOG_CLI((BSL_META_U(unit, "\n\r ********** utilex_occ_bm_test_1 : END. pass %d **********\n"), (int) pass));
    }
    return pass;
}

static uint32
utilex_occ_bm_test_2(
    int unit,
    uint8 silent)
{
    int32 pass = TRUE;
    uint32 indx, place;
    uint32 expected_places[5];
    uint8 found;
    UTILEX_OCC_BM_PTR bitmap;
    UTILEX_OCC_BM_INIT_INFO btmp_init_info;
    shr_error_e utilex_ret;

    utilex_ret = utilex_occ_bm_init_info_clear(&btmp_init_info);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    btmp_init_info.size = 64;

    utilex_ret = utilex_occ_bm_create(unit, &btmp_init_info, &bitmap);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }

    for (indx = 0; indx < 66; ++indx)
    {
        utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
            if (found)
            {
                utilex_occ_bm_print(unit, bitmap);
            }
        }

        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
    }
    /*
     * free one bit on 20
     */
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 20, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    /*
     * get next free place (has to be 20)
     */
    utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    if (!silent)
    {
        LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
        if (found)
        {
            utilex_occ_bm_print(unit, bitmap);
        }
    }

    if (place != 20)
    {
        pass = FALSE;
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u instead of 20\n"), place));
        }
    }

    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    /*
     * free 4 bits on 63, 0, 15, 32
     */
    expected_places[0] = 0;
    expected_places[1] = 15;
    expected_places[2] = 32;
    expected_places[3] = 63;
    expected_places[4] = 0;

    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 63, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 0, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 15, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 32, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }

    /*
     * allocate 5 bits 4 should be 63, 0, 15, 32 and one not found.
     */
    for (indx = 0; indx < 5; ++indx)
    {
        utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
            if (found)
            {
                utilex_occ_bm_print(unit, bitmap);
            }
        }
        if (place != expected_places[indx])
        {
            pass = FALSE;
            if (!silent)
            {
                LOG_CLI((BSL_META_U(unit, "found %u instead of %u\n"), place, expected_places[indx]));
            }
        }

        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
    }

exit:
    if (bitmap)
    {
        utilex_ret = utilex_occ_bm_destroy(unit, bitmap);

        if (SHR_FAILURE(utilex_ret))
        {
            LOG_CLI((BSL_META_U(unit, "Failed in utilex_occ_bm_destroy\n")));
            pass = FALSE;
        }
    }
    if (!silent)
    {
        LOG_CLI((BSL_META_U(unit, "\n\r ********** utilex_occ_bm_test_2 : END **********\n")));
    }
    return pass;
}

static uint32
utilex_occ_bm_test_3(
    int unit,
    uint8 silent)
{
    int32 pass = TRUE;
    uint32 indx, place;
    uint32 expected_places[5];
    uint8 found;
    UTILEX_OCC_BM_PTR bitmap;
    UTILEX_OCC_BM_INIT_INFO btmp_init_info;
    shr_error_e utilex_ret;

    utilex_ret = utilex_occ_bm_init_info_clear(&btmp_init_info);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    btmp_init_info.size = 700;

    utilex_ret = utilex_occ_bm_create(unit, &btmp_init_info, &bitmap);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }

    for (indx = 0; indx < 702; ++indx)
    {
        utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
            if (found)
            {
                utilex_occ_bm_print(unit, bitmap);
            }
        }
        if ((indx < 700 && place != indx) || (indx >= 700 && found))
        {
            pass = FALSE;
            if (!silent)
            {
                LOG_CLI((BSL_META_U(unit, "found %u instead of %u\n"), place, indx));
            }
        }

        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
    }
    /*
     * free one bit on 20
     */
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 200, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    /*
     * get next free place (has to be 200)
     */
    utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    if (!silent)
    {
        LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
        if (found)
        {
            utilex_occ_bm_print(unit, bitmap);
        }
    }

    if (place != 200)
    {
        pass = FALSE;
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u instead of 200\n"), place));
        }
    }

    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    /*
     * free 4 bits on 630, 0, 150, 320
     */
    expected_places[0] = 0;
    expected_places[1] = 150;
    expected_places[2] = 320;
    expected_places[3] = 630;
    expected_places[4] = 0;

    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 630, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 0, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 150, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, 320, FALSE);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }

    /*
     * allocate 5 bits 4 should be 630, 0, 150, 320 and one not found.
     */
    for (indx = 0; indx < 5; ++indx)
    {
        utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
            if (found)
            {
                utilex_occ_bm_print(unit, bitmap);
            }
        }
        if (place != expected_places[indx])
        {
            pass = FALSE;
            if (!silent)
            {
                LOG_CLI((BSL_META_U(unit, "found %u instead of %u\n"), place, expected_places[indx]));
            }
        }

        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
    }

exit:
    if (bitmap)
    {
        utilex_ret = utilex_occ_bm_destroy(unit, bitmap);

        if (SHR_FAILURE(utilex_ret))
        {
            LOG_CLI((BSL_META_U(unit, "Failed in utilex_occ_bm_destroy\n")));
            pass = FALSE;
        }
    }
    if (!silent)
    {
        LOG_CLI((BSL_META_U(unit, "\n\r ********** utilex_occ_bm_test_3 : END **********\n")));
    }
    return pass;
}

static uint32
utilex_occ_bm_test_4(
    int unit,
    uint8 silent)
{
    int32 pass = TRUE;
    uint32 indx, place, free_place;
    uint8 found;
    UTILEX_OCC_BM_PTR bitmap;
    UTILEX_OCC_BM_INIT_INFO btmp_init_info;
    shr_error_e utilex_ret;

    utilex_ret = utilex_occ_bm_init_info_clear(&btmp_init_info);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }
    btmp_init_info.size = 700;

    utilex_ret = utilex_occ_bm_create(unit, &btmp_init_info, &bitmap);
    if (SHR_FAILURE(utilex_ret))
    {
        goto exit;
    }

    for (indx = 0; indx < 702; ++indx)
    {
        utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
            if (found)
            {
                utilex_occ_bm_print(unit, bitmap);
            }
        }
        if ((indx < 700 && place != indx) || (indx >= 700 && found))
        {
            pass = FALSE;
            if (!silent)
            {
                LOG_CLI((BSL_META_U(unit, "found %u instead of %u\n"), place, indx));
            }
        }

        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
    }
    for (indx = 0; indx < 700; ++indx)
    {
        /*
         * free one bit one by one
         */
        free_place = indx;
        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, free_place, FALSE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        /*
         * get next free place (has to be free_place)
         */
        utilex_ret = utilex_occ_bm_get_next(unit, bitmap, &place, &found);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
        if (!silent)
        {
            LOG_CLI((BSL_META_U(unit, "found %u \n"), place));
            if (found)
            {
                utilex_occ_bm_print(unit, bitmap);
            }
        }

        if (place != free_place)
        {
            pass = FALSE;
            if (!silent)
            {
                LOG_CLI((BSL_META_U(unit, "found %u instead of %u\n"), place, free_place));
            }
        }

        utilex_ret = utilex_occ_bm_occup_status_set(unit, bitmap, place, TRUE);
        if (SHR_FAILURE(utilex_ret))
        {
            goto exit;
        }
    }
exit:
    if (bitmap)
    {
        utilex_ret = utilex_occ_bm_destroy(unit, bitmap);

        if (SHR_FAILURE(utilex_ret))
        {
            LOG_CLI((BSL_META_U(unit, "Failed in utilex_occ_bm_destroy\n")));
            pass = FALSE;
        }
    }
    if (!silent)
    {
        LOG_CLI((BSL_META_U(unit, "\n\r ********** utilex_occ_bm_test_4 : END **********\n")));
    }
    return pass;
}

uint32
utilex_occ_bm_tests(
    int unit,
    uint8 silent)
{
    uint32 utilex_pass;
    uint32 fail;

    fail = FALSE;

    utilex_pass = utilex_occ_bm_test_1(unit, silent);
    if (!utilex_pass)
    {
        fail = TRUE;
    }
    utilex_pass = utilex_occ_bm_test_2(unit, silent);
    if (!utilex_pass)
    {
        fail = TRUE;
    }
    utilex_pass = utilex_occ_bm_test_3(unit, silent);
    if (!utilex_pass)
    {
        fail = TRUE;
    }
    utilex_pass = utilex_occ_bm_test_4(unit, silent);
    if (!utilex_pass)
    {
        fail = TRUE;
    }
    if (!silent)
    {
        LOG_CLI((BSL_META_U(unit, "\n\r ********** utilex_occ_bm_tests : END. Fail: %d **********\n"), fail));
    }
    /*
     * Return a non-zero value on success and zero on failure.
     */
    return (!fail);
}

/* } */
#endif /* UTILEX_DEBUG */

/* } */
#else
/* { */
/*
 * This is dummy code. It is only for compilers that do not accept empty files
 * and is never to be used.
 */
shr_error_e
utilex_occ_bm_init_info_clear(
    UTILEX_OCC_BM_INIT_INFO * info)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);
    SHR_NULL_CHECK(info, _SHR_E_INTERNAL, "info");

    sal_memset(info, 0x0, sizeof(UTILEX_OCC_BM_INIT_INFO));
    info->size = 0;
    info->init_val = FALSE;
    info->support_cache = FALSE;

exit:
    SHR_FUNC_EXIT;
}
/* } */
#endif
