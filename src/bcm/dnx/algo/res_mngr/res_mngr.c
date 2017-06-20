/** \file res_mngr.c
 *
 * Resource management functions for DNX.
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
/**
* INCLUDE FILES:
* {
*/

/*
 * Include files which are specifically for DNX. Final location.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/algo/res_mngr/simple_bitmap_wrap.h>
#include <shared/utilex/utilex_hashtable.h>
#include <soc/dnx/algo/res_mngr/res_mngr_internal.h>
#include <soc/dnx/dnx_data/dnx_data_device.h>
/*
 * }
 */

/*
 * Other include files. 
 * { 
 */
#include <shared/swstate/access/sw_state_access.h>
#include <bcm/types.h>
/*
 * }
 */

/**
 * }
 */

#define RES_MNGR_ACCESS sw_state_access[unit].dnx.bcmdnx.res_mngr

#define DNX_ALGO_RES_INVALID_INDEX -1

/*
 * Size of the key for hash tables. That's the size of string of maximum length.
 */
#define DNX_ALGO_RES_HASH_SIZE      (sizeof(char) * DNX_ALGO_RES_MNGR_MAX_NAME_LENGTH + 1)

dnx_algo_res_advanced_alogrithm_cb_t *res_algorithm_cb_array[BCM_MAX_NUM_UNITS];

#define DNX_ALGO_RES_GET_POOL_CORE(_core_id) ((_core_id == _SHR_CORE_ALL) ? 0 : _core_id)

shr_error_e
dnx_algo_res_init(
    int unit)
{
    int nof_resources = DNX_ALGO_RES_MNGR_NOF_RESOURCES;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * First, allocate the algorithm callback array. 
     * This should be done regardless of warmboot status since it's a static array.
     */
    if (res_algorithm_cb_array[unit] != NULL)
    {
        SHR_ERR_EXIT(BCM_E_INIT, "Resource manager already initialized.");
    }

    res_algorithm_cb_array[unit] = (dnx_algo_res_advanced_alogrithm_cb_t *)
        sal_alloc(sizeof(dnx_algo_res_advanced_alogrithm_cb_t) * nof_resources,
                  "Resource manager algorithm callback array");

    if (!SOC_WARM_BOOT(unit))
    {
        UTILEX_HASH_TABLE_PTR htb_ptr;
        UTILEX_HASH_TABLE_INIT_INFO htb_init_info;

        /*
         * Intiailize module's SW state.
         */
        SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.ptr_alloc(unit, nof_resources));

        /*
         * Initialize name->res_index hash table and save the pointer.
         */

        sal_memset(&htb_init_info, 0, sizeof(htb_init_info));

        htb_init_info.data_size = sizeof(int);
        htb_init_info.key_size = DNX_ALGO_RES_HASH_SIZE;
        htb_init_info.table_size = nof_resources;
        htb_init_info.table_width = nof_resources;
        SHR_IF_ERR_EXIT(utilex_hash_table_create(unit, &htb_ptr, htb_init_info));

        SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.hash_table_ptr.set(unit, htb_ptr));
    }
exit:
    if (SHR_FUNC_ERR() && res_algorithm_cb_array[unit] != NULL)
    {
        sal_free(res_algorithm_cb_array[unit]);
        res_algorithm_cb_array[unit] = NULL;
    }
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_res_deinit(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Free the static array. Everything else will be deinitialized with the sw state. 
     */
    if (res_algorithm_cb_array[unit] != NULL)
    {
        sal_free(res_algorithm_cb_array[unit]);
        res_algorithm_cb_array[unit] = NULL;
    }
    SHR_EXIT();
exit:
    SHR_FUNC_EXIT;
}

/*
 * Given a resource name and an empty pointer, returns the key for the name->res_index hash table. 
 *    dnx_algo_res_free_name_hash must be used after calling this function. 
 */
STATIC shr_error_e
dnx_algo_res_create_name_hash(
    dnx_algo_res_name_t name,
    dnx_algo_res_name_t * name_hash)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);

    *name_hash = (dnx_algo_res_name_t) sal_alloc(DNX_ALGO_RES_HASH_SIZE, "dnx_algo_res_create_name_hash");

    if (*name_hash == NULL)
    {
        SHR_ERR_EXIT(BCM_E_MEMORY, "Failed to allocate memory for resource manager hash");
    }

    sal_memset(*name_hash, 0, DNX_ALGO_RES_HASH_SIZE);

    sal_strcpy(*name_hash, name);

exit:
    SHR_FUNC_EXIT;
}

/*
 * Frees the key for the name->res_index hash table that was created above.
 */
STATIC shr_error_e
dnx_algo_res_free_name_hash(
    dnx_algo_res_name_t * name_hash)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);

    if (*name_hash != NULL)
    {
        sal_free(*name_hash);
        *name_hash = NULL;
    }

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * Given a resource name, returns the index for this resource. 
 * Returns DNX_ALGO_RES_INVALID_INDEX for not existing names.
 */
STATIC shr_error_e
dnx_algo_res_get_res_index(
    int unit,
    dnx_algo_res_name_t name,
    int *res_index)
{
    uint8 found;
    uint32 index_get;
    UTILEX_HASH_TABLE_PTR htb_ptr;
    dnx_algo_res_name_t name_hash = NULL;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.hash_table_ptr.get(unit, &htb_ptr));

    SHR_IF_ERR_EXIT(dnx_algo_res_create_name_hash(name, &name_hash));

    SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup
                    (unit, htb_ptr, (UTILEX_HASH_TABLE_KEY *) name_hash, &index_get, &found));

    *res_index = (found) ? index_get : DNX_ALGO_RES_INVALID_INDEX;

exit:
    dnx_algo_res_free_name_hash(&name_hash);
    SHR_FUNC_EXIT;
}

/*
 * Given a resource name, creates a new res_index for it. 
 */
STATIC shr_error_e
dnx_algo_res_create_res_index(
    int unit,
    dnx_algo_res_name_t name,
    int *res_index)
{
    uint8 success = FALSE;
    uint32 index_get = 0;
    UTILEX_HASH_TABLE_PTR htb_ptr;
    dnx_algo_res_name_t name_hash = NULL;
    int used_count;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Save the name to the hash table and get the allocated id.
     */
    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.hash_table_ptr.get(unit, &htb_ptr));

    SHR_IF_ERR_EXIT(dnx_algo_res_create_name_hash(name, &name_hash));

    SHR_IF_ERR_EXIT(utilex_hash_table_entry_add
                    (unit, htb_ptr, (UTILEX_HASH_TABLE_KEY *) name_hash, &index_get, &success));

    if (!success)
    {
        SHR_ERR_EXIT(BCM_E_INTERNAL, "Failed to create resource %s", name);
    }

    *res_index = index_get;

    /*
     * Updated used resources count.
     */
    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.res_in_use_count.get(unit, &used_count));

    used_count++;
    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.res_in_use_count.set(unit, used_count));

exit:
    dnx_algo_res_free_name_hash(&name_hash);
    SHR_FUNC_EXIT;
}

/*
 * Verify input for dnx_algo_res_create.
 */
STATIC shr_error_e
dnx_algo_res_create_verify(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    dnx_algo_res_create_data_t * data,
    dnx_algo_res_advanced_alogrithm_cb_t * callbacks,
    int res_index)
{
    uint8 use_advanced_algorithm;
    int used_count;
    int max_name_length = DNX_ALGO_RES_MNGR_MAX_NAME_LENGTH;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(data, BCM_E_PARAM, "create_data");

    /*
     * Verify generic data.
     */

    if (sal_strlen(name) > max_name_length)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Name %s is too long. Must be at most %d characters", name, max_name_length);
    }

    if (data->first_element < 0)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "First element must be at least 0. Got: %d", data->first_element);
    }

    if (data->nof_elements < 1)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Number of elements must be at least 1. Got: %d", data->nof_elements);
    }

    if (core_id != BCM_CORE_ALL)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Core must be BCM_CORE_ALL.");
    }

    /*
     * Verify name. 
     *  If the name exists - we must be in warmboot, and only register callbacks.
     *  If the name doesn't exist - legal except for the above cases.
     */

    /*
     * Not found. 
     */
    if (res_index == DNX_ALGO_RES_INVALID_INDEX)
    {
        if (SOC_WARM_BOOT(unit))
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Can't create new resources during warmboot: \"%s.\"", name);
        }
    }
    else
    {
        if (!SOC_WARM_BOOT(unit))
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Resource already exists: \"%s.\"", name);
        }
    }

    /*
     * Verify callbacks if advanced algorithm is in use.
     */
    use_advanced_algorithm = _SHR_IS_FLAG_SET(data->flags, DNX_ALGO_RES_CREATE_USE_ADVANCED_ALGORITHM);

    if (use_advanced_algorithm)
    {
        if (callbacks == NULL)
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Callbacks struct is empty even though advanced algorithm was declared.");
        }

        if (callbacks->allocate_cb == NULL
            || callbacks->create_cb == NULL || callbacks->free_cb == NULL || callbacks->is_allocated_cb == NULL)
        {
            SHR_ERR_EXIT(BCM_E_PARAM,
                         "When using advanced algorithm, create_cb, allocate_cb, free_cb and is_allocated_cb"
                         "must be provided.\n"
                         "Pointers are: %p, %p, %p, %p",
                         callbacks->allocate_cb, callbacks->create_cb, callbacks->free_cb, callbacks->is_allocated_cb);
        }
    }
    else if (!use_advanced_algorithm && (callbacks != NULL))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Callbacks struct must be null if advanced algorithm is not used.");
    }

    /*
     * Verify that there are enough free resources.
     */
    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.res_in_use_count.get(unit, &used_count));

    if (used_count >= DNX_ALGO_RES_MNGR_NOF_RESOURCES)
    {
        SHR_ERR_EXIT(BCM_E_FULL, "Can't create more resources. Maximum number of resources already created.");
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Register the callbacks for this resource. If callbacks are not advanced 
 *   (indicated by passing NULL pointer) register the default callbacks.
 */
STATIC shr_error_e
dnx_algo_res_create_register_callbacks(
    int unit,
    int res_index,
    dnx_algo_res_advanced_alogrithm_cb_t * callbacks)
{
    dnx_algo_res_advanced_alogrithm_cb_t *cb_array_ptr = &res_algorithm_cb_array[unit][res_index];
    SHR_FUNC_INIT_VARS(unit);

    /*
     *  Save the callbacks for the mandatory functions.
     */
    if (callbacks != NULL)
    {
        cb_array_ptr->allocate_cb = callbacks->allocate_cb;
        cb_array_ptr->create_cb = callbacks->create_cb;
        cb_array_ptr->free_cb = callbacks->free_cb;
        cb_array_ptr->is_allocated_cb = callbacks->is_allocated_cb;

        

        if (callbacks->dump_cb != NULL)
        {
            cb_array_ptr->dump_cb = callbacks->dump_cb;
        }

        if (callbacks->dump_in_range_cb != NULL)
        {
            cb_array_ptr->dump_in_range_cb = callbacks->dump_in_range_cb;
        }

        if (callbacks->free_all_cb != NULL)
        {
            cb_array_ptr->free_all_cb = callbacks->free_all_cb;
        }

        if (callbacks->iterate_cb != NULL)
        {
            cb_array_ptr->iterate_cb = callbacks->iterate_cb;
        }
    }
    else
    {
        /*
         * Register default functions - simple bitmap.
         */
        cb_array_ptr->allocate_cb = dnx_algo_res_simple_bitmap_allocate;
        cb_array_ptr->create_cb = dnx_algo_res_simple_bitmap_create;
        cb_array_ptr->free_cb = dnx_algo_res_simple_bitmap_free;
        cb_array_ptr->is_allocated_cb = dnx_algo_res_simple_bitmap_is_allocated;
        
    }

    SHR_EXIT();
exit:
    SHR_FUNC_EXIT;
}

/*
 * Register the resource's data upon creation.
 */
STATIC shr_error_e
dnx_algo_res_create_register_data(
    int unit,
    int core_id,
    int res_index,
    int internal_res_index,
    dnx_algo_res_create_data_t * data)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * If this is the first core using this resource, we also need to allocate it in the sw state.
     *  First core to use the resource is always 0 (enforced by input validation).
     */
    if (core_id == 0)
    {
        uint8 is_allocated;
        SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.is_allocated(unit, res_index, &is_allocated));

        if (!is_allocated)
        {
            /*
             * If resource is multi core resource (indicated by DNX_ALGO_RES_CREATE_DUPLICATE_PER_CORE flag) 
             * then we need to allocate one pool per core. Otherwise, one is enough. 
             */
            int nof_cores_to_allocate = (_SHR_IS_FLAG_SET(data->flags, DNX_ALGO_RES_CREATE_DUPLICATE_PER_CORE))
                ? dnx_data_device.general.nof_cores_get(unit) : 1;

            SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.alloc(unit, res_index, nof_cores_to_allocate));
        }
    }

    /*
     * Simply save the data to the sw state.
     */
    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.flags.set(unit, res_index, core_id, data->flags));

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.first_element.set(unit, res_index, core_id, data->first_element));

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.nof_elements.set(unit, res_index, core_id, data->nof_elements));

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.algo_instance_id.set(unit, res_index, core_id, internal_res_index));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_res_create(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    dnx_algo_res_create_data_t * data,
    dnx_algo_res_advanced_alogrithm_cb_t * callbacks,
    void *extra_arguments)
{
    int res_index, internal_res_index;
    int current_core, nof_cores_to_use;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * 0. Verify input and get res index.
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_get_res_index(unit, name, &res_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_res_create_verify(unit, core_id, name, data, callbacks, res_index));

    /*
     * 1. Assign a resource index if it's a new resource. 
     *     In warmboot, the resource already exists so we don't need to create a new one.
     */
    if (res_index == DNX_ALGO_RES_INVALID_INDEX)
    {
        SHR_IF_ERR_EXIT(dnx_algo_res_create_res_index(unit, name, &res_index));
    }

    /*
     * 2. Register the callbacks in the callback array. 
     *    This should be done even if device is in warmboot.
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_create_register_callbacks(unit, res_index, callbacks));

    /*
     * If we're in warmboot, then we don't need to save anything else. 
     *   Everything will be automatically restored with the sw state.
     */
    if (!SOC_WARM_BOOT(unit))
    {
        /*
         * 3. Call the create function and register the resource data. This should be done per core if it's a duplicate 
         *      per core resource.
         *      If it's a separate per core resource, this API will be called again and it will be created in that call.
         */

        nof_cores_to_use = (_SHR_IS_FLAG_SET(data->flags, DNX_ALGO_RES_CREATE_DUPLICATE_PER_CORE))
            ? dnx_data_device.general.nof_cores_get(unit) : 1;

        for (current_core = 0; current_core < nof_cores_to_use; current_core++)
        {
            /*
             * 3.1 Call the create function.
             */

            SHR_IF_ERR_EXIT(res_algorithm_cb_array[unit][res_index].create_cb
                            (unit, data, extra_arguments, &internal_res_index));

            /*
             * 3.2 Register the resource data.
             */
            SHR_IF_ERR_EXIT(dnx_algo_res_create_register_data(unit, current_core, res_index, internal_res_index, data));
        }
    }
exit:
    SHR_FUNC_EXIT;
}

/*
 * Verify that an element is within legal range of the resource.
 */
STATIC shr_error_e
dnx_algo_res_element_verify(
    int unit,
    int pool_core,
    int res_index,
    int element)
{

    int first_element, nof_elements;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get first element and nof_elements from the sw state.
     */
    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.first_element.get(unit, res_index, pool_core, &first_element));

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.nof_elements.get(unit, res_index, pool_core, &nof_elements));

    /*
     * Verify.
     */
    if (element < first_element)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Element is too low. Element is %d and minimum is %d ", element, first_element);
    }

    if (element >= first_element + nof_elements)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Element is too high. Element is %d and minimum is %d ", element, first_element);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verify that an element is legal, and supports the given core_id.
 */
STATIC shr_error_e
dnx_algo_res_verify(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int res_index)
{
    uint32 flags;
    uint8 multi_core;
    int pool_core = 0;
    SHR_FUNC_INIT_VARS(unit);

    if (res_index == -1)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Resource doesn't exist: \"%s.\"", name);
    }

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.flags.get(unit, res_index, pool_core, &flags));

    /*
     * Multi core resources must be called separately per core. 
     *   Otherwise, must be called with _SHR_CORE_ALL. 
     */
    multi_core = (_SHR_IS_FLAG_SET(flags, DNX_ALGO_RES_CREATE_DUPLICATE_PER_CORE));

    if (multi_core && core_id == _SHR_CORE_ALL)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Resource must be used one core a time: \"%s.\"", name);
    }

    if (!multi_core && core_id != _SHR_CORE_ALL)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Resource must be used with _SHR_CORE_ALL: \"%s.\"", name);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verify input for allocate function.
 */
STATIC shr_error_e
dnx_algo_res_allocate_verify(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    uint32 flags,
    int *element,
    int res_index)
{
    int pool_core;
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(element, BCM_E_PARAM, "element");

    SHR_IF_ERR_EXIT(dnx_algo_res_verify(unit, core_id, name, res_index));

    pool_core = DNX_ALGO_RES_GET_POOL_CORE(core_id);

    if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_RES_ALLOCATE_WITH_ID))
    {
        SHR_IF_ERR_EXIT(dnx_algo_res_element_verify(unit, pool_core, res_index, *element));
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 *  See description in .h file.
 */
shr_error_e
dnx_algo_res_allocate(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    uint32 flags,
    void *extra_arguemnts,
    int *element)
{
    int algo_instance_id, res_index, pool_core;
    int rv;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_res_get_res_index(unit, name, &res_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_res_allocate_verify(unit, core_id, name, flags, element, res_index));

    /*
     * Pool core is an array index. Since the array index for single core resources is always 0, set it so.
     */
    pool_core = DNX_ALGO_RES_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.algo_instance_id.get(unit, res_index, pool_core, &algo_instance_id));

    rv = res_algorithm_cb_array[unit][res_index].allocate_cb
                    (unit, algo_instance_id, flags, extra_arguemnts, element);

    if ((rv == _SHR_E_RESOURCE) && (_SHR_IS_FLAG_SET(flags, DNX_ALGO_RES_ALLOCATE_WITH_ID))) {
        /*
         * Some advanced algorithms may use res_bitmap as an internal implementation. 
         * It returns E_RESOURCE when an element WITH_ID already exists. This error 
         * code is irrelevant, so replace it with E_EXISTS. 
         */
        SHR_IF_ERR_EXIT(_SHR_E_EXISTS);
    }
    SHR_IF_ERR_EXIT(rv);

exit:
    SHR_FUNC_EXIT;
}

/*
 * Common verification function for is_allocated and _delete, since they check similar inputs.
 */
STATIC shr_error_e
dnx_algo_res_get_delete_verify(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int element,
    int res_index)
{
    int pool_core;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify resource. 
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_verify(unit, core_id, name, res_index));

    pool_core = DNX_ALGO_RES_GET_POOL_CORE(core_id);

    /*
     * Verify element.
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_element_verify(unit, pool_core, res_index, element));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verification for _is_allocated API. 
 */
shr_error_e
dnx_algo_res_get_verify(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int element,
    uint8 * is_allocated,
    int res_index)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(is_allocated, BCM_E_PARAM, "is_allocated");

    SHR_IF_ERR_EXIT(dnx_algo_res_get_delete_verify(unit, core_id, name, element, res_index));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See description in .h file. 
 */
shr_error_e
dnx_algo_res_is_allocated(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int element,
    uint8 * is_allocated)
{
    int algo_instance_id, res_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_res_get_res_index(unit, name, &res_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_res_get_verify(unit, core_id, name, element, is_allocated, res_index));

    pool_core = DNX_ALGO_RES_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.algo_instance_id.get(unit, res_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(res_algorithm_cb_array[unit][res_index].
                    is_allocated_cb(unit, algo_instance_id, element, is_allocated));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See description in .h file. 
 */
shr_error_e
dnx_algo_res_free(
    int unit,
    int core_id,
    dnx_algo_res_name_t name,
    int element)
{
    int algo_instance_id, res_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_res_get_res_index(unit, name, &res_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_res_get_delete_verify(unit, core_id, name, element, res_index));

    pool_core = DNX_ALGO_RES_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(RES_MNGR_ACCESS.pools_data.algo_instance_id.get(unit, res_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(res_algorithm_cb_array[unit][res_index].free_cb(unit, algo_instance_id, element));

exit:
    SHR_FUNC_EXIT;
}
