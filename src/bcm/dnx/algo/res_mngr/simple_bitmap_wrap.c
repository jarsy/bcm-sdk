/*! \file simple_bitmap_wrap.c
 *
 * Resource manager wrapper functions for simple bitmap.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_RESMNGR
/*!
* INCLUDE FILES:
* {
*/
/*
 * Include files which are specifically for DNX. Final location.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <shared/swstate/sw_state_res_bitmap.h>
/*
 * }
 */
#include <bcm/types.h>
#include <bcm/error.h>

/** 
 * }
 */

shr_error_e
dnx_algo_res_simple_bitmap_create(
    int unit,
    dnx_algo_res_create_data_t * create_data,
    void *extra_arguments,
    int *algo_instance_id)
{
    int rv;
    sw_state_res_bitmap_handle_t bitmap_handle;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(create_data, _SHR_E_PARAM, "create_data");
    SHR_NULL_CHECK(algo_instance_id, _SHR_E_PARAM, "algo_instance_id");

    rv = sw_state_res_bitmap_create(unit, &bitmap_handle, create_data->first_element, create_data->nof_elements);
    SHR_IF_ERR_EXIT(rv);

    *algo_instance_id = bitmap_handle;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_res_simple_bitmap_allocate(
    int unit,
    int algo_instance_id,
    uint32 flags,
    void *extra_arguments,
    int *element)
{
    uint32 internal_flags;
    uint8 with_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(element, _SHR_E_PARAM, "element");

    /*
     * Translate the allocation flags.
     * The input flags are of type DNX_ALGO_RES_ALLOCATE_*, but the sw_state_res_bitmap use a 
     * different set of flags.
     */
    with_id = _SHR_IS_FLAG_SET(flags, DNX_ALGO_RES_ALLOCATE_WITH_ID);
    internal_flags = 0;
    internal_flags |= (with_id) ? SW_STATE_RES_BITMAP_ALLOC_WITH_ID : 0;

    SHR_IF_ERR_EXIT(sw_state_res_bitmap_alloc(unit, algo_instance_id, internal_flags, 1, element));
    
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_res_simple_bitmap_is_allocated(
    int unit,
    int algo_instance_id,
    int element,
    uint8 * is_allocated)
{
    int rv;

    SHR_FUNC_INIT_VARS(unit);

    rv = sw_state_res_bitmap_check(unit, algo_instance_id, 1, element);
    if (rv == _SHR_E_NOT_FOUND)
    {
        *is_allocated = FALSE;
    }
    else if (rv == _SHR_E_EXISTS)
    {
        *is_allocated = TRUE;
    }
    else
    {
        SHR_IF_ERR_EXIT(rv);
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_res_simple_bitmap_free(
    int unit,
    int algo_instance_id,
    int element)
{
    int rv;

    SHR_FUNC_INIT_VARS(unit);

    rv = sw_state_res_bitmap_free(unit, algo_instance_id, 1, element);
    SHR_IF_ERR_EXIT(rv);

exit:
    SHR_FUNC_EXIT;
}
