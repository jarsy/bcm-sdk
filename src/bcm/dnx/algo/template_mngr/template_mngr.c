/** \file template_mngr.c
 *
 * Template management functions for DNX.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_TEMPLATEMNGR
/**
* INCLUDE FILES:
* {
*/

/*
 * Include files which are specifically for DNX. Final location.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
#include <shared/utilex/utilex_hashtable.h>
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
#include <shared/swstate/sw_state.h>
/*
 * }
 */

/**
 * }
 */

/*
 *  This is the prefix for the template manager's sw state, wrapped into a macro for easy access.
 */
#define TEMPLATE_MNGR_ACCESS sw_state_access[unit].dnx.bcmdnx.template_mngr

/*
 * Invalid template index, used to indicate that the template does not exist.
 */
#define DNX_ALGO_TEMPLATE_INVALID_INDEX -1

/* 
 * Number of supported templates in the system.
 */ 
#define DNX_ALGO_TEMPLATE_MNGR_NOF_TEMPLATES (100)

/*
 * Maximum length of a template's name.
 */
#define DNX_ALGO_TEMPLATE_MNGR_MAX_NAME_LENGTH   (100)

/*
 * Size of the key for hash tables. That's the size of string of maximum length.
 */
#define DNX_ALGO_TEMPLATE_HASH_SIZE      (sizeof(char) * DNX_ALGO_TEMPLATE_MNGR_MAX_NAME_LENGTH + 1)

/*
 * This array holds the advanced (or default) algorithm callback used by each template. 
 * It is duplicated per unit to avoid inconsistencies in case different units use a different
 *   set of templates.
 */
static dnx_algo_template_advanced_alogrithm_cb_t *Template_algorithm_cb_array[BCM_MAX_NUM_UNITS] = {0};

/*
 * This array holds the print callback used by each template. 
 * It is duplicated per unit to avoid inconsistencies in case different units use a different
 *   set of templates.
 */
static dnx_algo_template_print_data_cb *Template_print_data_cb_array[BCM_MAX_NUM_UNITS] = {0};

/*
 * Pool core is an array index. When templates are not duplicated per core, the first array entry (0)
 *   represents all cores. 
 * Since the input core for these templates is always _SHR_CORE_ALL, fix it to 0.
 */
#define DNX_ALGO_TEMPLATE_GET_POOL_CORE(_core_id) ((_core_id == _SHR_CORE_ALL) ? 0 : _core_id)


shr_error_e
dnx_algo_template_init(
    int unit)
{
    int nof_templates = DNX_ALGO_TEMPLATE_MNGR_NOF_TEMPLATES;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * First, allocate the algorithm and print callback array. 
     * This should be done regardless of warmboot status since it's a static array.
     */
    if (Template_algorithm_cb_array[unit] != NULL || Template_print_data_cb_array[unit] != NULL)
    {
        SHR_ERR_EXIT(BCM_E_INIT, "Template manager already initialized.");
    }

    /*
     * Allocate the algorithm array.
     */
    Template_algorithm_cb_array[unit] = (dnx_algo_template_advanced_alogrithm_cb_t *)
        sal_alloc(sizeof(dnx_algo_template_advanced_alogrithm_cb_t) * nof_templates,
                  "Template manager algorithm callback array");

    if (Template_algorithm_cb_array[unit] == NULL)
    {
        SHR_ERR_EXIT(BCM_E_MEMORY, "Failed to allocate Template_algorithm_cb_array[%d].", unit);
    }

    /*
     * Allocate the print array.
     */
    Template_print_data_cb_array[unit] = (dnx_algo_template_print_data_cb *)
        sal_alloc(sizeof(dnx_algo_template_print_data_cb) * nof_templates, "Template manager print callback array");

    if (Template_print_data_cb_array[unit] == NULL)
    {
        SHR_ERR_EXIT(BCM_E_MEMORY, "Failed to allocate Template_print_data_cb_array[%d].", unit);
    }

    if (!SOC_WARM_BOOT(unit))
    {
        UTILEX_HASH_TABLE_PTR htb_ptr;
        UTILEX_HASH_TABLE_INIT_INFO htb_init_info;

        /*
         * Intiailize module's SW state.
         */
        SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.ptr_alloc(unit, nof_templates));

        /*
         * Initialize name->template_index hash table and save the pointer.
         */

        sal_memset(&htb_init_info, 0, sizeof(htb_init_info));

        htb_init_info.data_size = sizeof(int);
        htb_init_info.key_size = DNX_ALGO_TEMPLATE_HASH_SIZE;
        htb_init_info.table_size = nof_templates;
        htb_init_info.table_width = nof_templates;
        SHR_IF_ERR_EXIT(utilex_hash_table_create(unit, &htb_ptr, htb_init_info));

        SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.hash_table_ptr.set(unit, htb_ptr));

        /*
         * Initialize the default template algorithm - multi_set_template:
         */
        SHR_IF_ERR_EXIT(multi_set_template_init(unit));
    }

exit:
    if (SHR_FUNC_ERR() && Template_algorithm_cb_array[unit] != NULL)
    {
        sal_free(Template_algorithm_cb_array[unit]);
        Template_algorithm_cb_array[unit] = NULL;
    }
    if (SHR_FUNC_ERR() && Template_print_data_cb_array[unit] != NULL)
    {
        sal_free(Template_print_data_cb_array[unit]);
        Template_print_data_cb_array[unit] = NULL;
    }

    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_template_deinit(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Free the static arrays. Everything else will be deinitialized with the sw state. 
     */
    if (Template_algorithm_cb_array[unit] != NULL)
    {
        sal_free(Template_algorithm_cb_array[unit]);
        Template_algorithm_cb_array[unit] = NULL;
    }
    if (Template_print_data_cb_array[unit] != NULL)
    {
        sal_free(Template_print_data_cb_array[unit]);
        Template_print_data_cb_array[unit] = NULL;
    }

    SHR_EXIT();
exit:
    SHR_FUNC_EXIT;
}

/*
 * Given a template name and an empty pointer, returns the key for the name->template_index hash table. 
 *    dnx_algo_template_free_name_hash must be used after calling this function.
 * name must be a pointer to a null terminated string of length <DNX_ALGO_TEMPLATE_MNGR_MAX_NAME_LENGTH                                                                                                .
 */
static shr_error_e
dnx_algo_template_create_name_hash(
    dnx_algo_template_name_t name,
    dnx_algo_template_name_t * name_hash)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);

    *name_hash =
        (dnx_algo_template_name_t) sal_alloc(DNX_ALGO_TEMPLATE_HASH_SIZE, "dnx_algo_template_create_name_hash");

    if (*name_hash == NULL)
    {
        SHR_ERR_EXIT(BCM_E_MEMORY, "Failed to allocate memory for template manager hash");
    }

    sal_memset(*name_hash, 0, DNX_ALGO_TEMPLATE_HASH_SIZE);

    sal_strcpy(*name_hash, name);

exit:
    SHR_FUNC_EXIT;
}

/*
 * Frees the key for the name->template_index hash table that was created above. 
 */
static shr_error_e
dnx_algo_template_free_name_hash(
    dnx_algo_template_name_t * name_hash)
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
 * Since dnx_algo_template_get_template_index is used to get the template index before the APIs invoke their verify function, 
 *   we need to verify it in a separate function.
 */
static shr_error_e
dnx_algo_template_get_template_index_verify(
   int unit,
   dnx_algo_template_name_t name)
{
    int name_len = 0;
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(name, BCM_E_PARAM, "name");

    name_len = sal_strlen(name);
    if (name_len > DNX_ALGO_TEMPLATE_MNGR_MAX_NAME_LENGTH)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Name %s is too long. Must be at most %d characters. Length is %d characters.", 
                     name, DNX_ALGO_TEMPLATE_MNGR_MAX_NAME_LENGTH, name_len);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Given a template name, returns the index for this template. See dnx_algo_template_create_template_index.
 * Returns DNX_ALGO_TEMPLATE_INVALID_INDEX for names that don't exist.
 */
static shr_error_e
dnx_algo_template_get_template_index(
    int unit,
    dnx_algo_template_name_t name,
    int *template_index)
{
    uint8 found;
    uint32 index_get;
    UTILEX_HASH_TABLE_PTR htb_ptr;
    dnx_algo_template_name_t name_hash = NULL;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_get_template_index_verify(unit, name));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.hash_table_ptr.get(unit, &htb_ptr));

    SHR_IF_ERR_EXIT(dnx_algo_template_create_name_hash(name, &name_hash));

    SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup
                    (unit, htb_ptr, (UTILEX_HASH_TABLE_KEY *) name_hash, &index_get, &found));

    *template_index = (found) ? index_get : DNX_ALGO_TEMPLATE_INVALID_INDEX;

exit:
    dnx_algo_template_free_name_hash(&name_hash);
    SHR_FUNC_EXIT;
}

/*
 * Given a template name, creates a new template_index for it.
 *  
 * The template index is a handle used internally by the module to access the sw state 
 *   and the callback functions for the specific template. It is actually the index in these
 *   arrays.
 */
static shr_error_e
dnx_algo_template_create_template_index(
    int unit,
    dnx_algo_template_name_t name,
    int *template_index)
{
    uint8 success = FALSE;
    uint32 index_get;
    UTILEX_HASH_TABLE_PTR htb_ptr;
    dnx_algo_template_name_t name_hash = NULL;
    int used_count;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Save the name to the hash table and get the allocated id.
     */
    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.hash_table_ptr.get(unit, &htb_ptr));

    SHR_IF_ERR_EXIT(dnx_algo_template_create_name_hash(name, &name_hash));

    SHR_IF_ERR_EXIT(utilex_hash_table_entry_add
                    (unit, htb_ptr, (UTILEX_HASH_TABLE_KEY *) name_hash, &index_get, &success));

    if (!success)
    {
        SHR_ERR_EXIT(BCM_E_INTERNAL, "Failed to create template %s", name);
    }

    *template_index = index_get;

    /*
     * Update used templates count.
     */
    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.template_in_use_count.get(unit, &used_count));

    used_count++;
    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.template_in_use_count.set(unit, used_count));

exit:
    dnx_algo_template_free_name_hash(&name_hash);
    SHR_FUNC_EXIT;
}

/*
 * Verify input for dnx_algo_template_create.
 */
static shr_error_e
dnx_algo_template_create_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    dnx_algo_template_create_data_t * data,
    dnx_algo_template_advanced_alogrithm_cb_t * callbacks,
    int template_index)
{
    uint8 use_advanced_algorithm;
    int used_count;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(data, BCM_E_PARAM, "create_data");

    /*
     * Verify generic data.
     */
    if (data->first_profile < 0)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "First profile can't be negative. Got: %d", data->first_profile);
    }

    if (data->nof_profiles < 1)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Number of profiles must be at least 1. Got: %d", data->nof_profiles);
    }

    if (data->data_size < 1)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Data size must be at least 1. Got: %d", (int) data->data_size);
    }

    if (data->max_references < 1)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Max references must be at least 1. Got: %d", (int) data->max_references);
    }

    if (data->print_cb == NULL)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Must provide a print function callback");
    }

    if (core_id != BCM_CORE_ALL)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Core must be BCM_CORE_ALL. Given %d", core_id);
    }

    /*
     * Verify name. 
     *  If the name exists - we must be in warmboot, and only register callbacks.
     *  If the name doesn't exist - we can't be in warmboot mode, since creating new templates is forbidden in warmboot.
     */

    /*
     * Not found. 
     */
    if (template_index == DNX_ALGO_TEMPLATE_INVALID_INDEX)
    {
        if (SOC_WARM_BOOT(unit))
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Can't create new templates during warmboot: \"%s.\"", name);
        }
    }
    else
    {
        if (!SOC_WARM_BOOT(unit))
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Template name already exists: \"%s.\"", name);
        }
    }

    /*
     * Verify callbacks if advanced algorithm is in use.
     */
    use_advanced_algorithm = _SHR_IS_FLAG_SET(data->flags, DNX_ALGO_TEMPLATE_CREATE_USE_ADVANCED_ALGORITHM);

    if (use_advanced_algorithm)
    {
        if (callbacks == NULL)
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Callbacks struct is empty even though advanced algorithm was declared: \"%s.\"", name);
        }

        
        if (callbacks->allocate_cb == NULL || callbacks->create_cb == NULL
            || callbacks->free_cb == NULL || callbacks->profile_data_get_cb == NULL)
        {
            SHR_ERR_EXIT(BCM_E_PARAM,
                         "When using advanced algorithm, create_cb, allocate_cb, free_cb, and profile_data_get_cb "
                         "must be provided.\n"
                         "Pointers are: %p, %p, %p, %p",
                         callbacks->allocate_cb, callbacks->create_cb, callbacks->free_cb,
                         callbacks->profile_data_get_cb);
        }

        if (_SHR_IS_FLAG_SET(data->flags, DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE)
            && callbacks->exchange_cb == NULL)
        {
            SHR_ERR_EXIT(BCM_E_PARAM,
                         "When using advanced algorithm and default profile, exchange_cb must be provided.\n");
        }
    }
    else if (!use_advanced_algorithm && (callbacks != NULL))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Advanced algorithm is not in use, but callbacks struct is not NULL.");
    }

    /*
     *  Verify default profile if it's in use.
     */
    if (_SHR_IS_FLAG_SET(data->flags, DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE))
    {
        int max_profile;
        if (data->default_profile < data->first_profile)
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Default profile is too small. Must be at least %d but got: %d",
                         data->first_profile, data->default_profile);
        }

        max_profile = data->first_profile + data->nof_profiles - 1;
        if (data->default_profile > max_profile)
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "Default profile is too high. Must be at most %d but got: %d",
                         max_profile, data->default_profile);
        }

        if (data->default_data == NULL)
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "If default profile is in use, default data must be provided.");
        }
    }

    /*
     * Verify that there are enough free templates.
     */
    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.template_in_use_count.get(unit, &used_count));

    if (used_count >= DNX_ALGO_TEMPLATE_MNGR_NOF_TEMPLATES)
    {
        SHR_ERR_EXIT(BCM_E_FULL, "Can't create more templates. Maximum number of templates already created (%d).", 
                     DNX_ALGO_TEMPLATE_MNGR_NOF_TEMPLATES);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Register the callbacks for this template. If callbacks are not advanced 
 *   (indicated by passing NULL pointer) register the default callbacks.
 */
static shr_error_e
dnx_algo_template_register_callbacks(
    int unit,
    int template_index,
    dnx_algo_template_print_data_cb print_cb,
    dnx_algo_template_advanced_alogrithm_cb_t * callbacks)
{
    dnx_algo_template_advanced_alogrithm_cb_t *cb_array_ptr = &Template_algorithm_cb_array[unit][template_index];
    SHR_FUNC_INIT_VARS(unit);

    /*
     *  Save the callbacks for the mandatory functions.
     */
    if (callbacks != NULL)
    {
        /*
         * Enter if callback are ADVANCED.
         */
        cb_array_ptr->allocate_cb = callbacks->allocate_cb;
        cb_array_ptr->create_cb = callbacks->create_cb;
        cb_array_ptr->free_cb = callbacks->free_cb;
        cb_array_ptr->profile_data_get_cb = callbacks->profile_data_get_cb;

        
        if (callbacks->exchange_cb != NULL)
        {
            cb_array_ptr->exchange_cb = callbacks->exchange_cb;
        }

        if (callbacks->clear_cb != NULL)
        {
            cb_array_ptr->clear_cb = callbacks->clear_cb;
        }

        if (callbacks->profile_get_cb != NULL)
        {
            cb_array_ptr->profile_get_cb = callbacks->profile_get_cb;
        }

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
         * Register default functions - multi set template.
         */
        cb_array_ptr->allocate_cb = multi_set_template_allocate;
        cb_array_ptr->create_cb = multi_set_template_create;
        cb_array_ptr->free_cb = multi_set_template_free;
        cb_array_ptr->profile_data_get_cb = multi_set_template_profile_data_get;
        cb_array_ptr->exchange_cb = multi_set_template_exchange;
        cb_array_ptr->clear_cb = multi_set_template_clear;
        cb_array_ptr->profile_get_cb = multi_set_template_profile_get;
        
    }

    /*
     * Save the print callback.
     */
    Template_print_data_cb_array[unit][template_index] = print_cb;

    SHR_EXIT();
exit:
    SHR_FUNC_EXIT;
}

/*
 * Register the template's data upon creation. 
 * All data is considered to already be checked by dnx_algo_template_create_verify.
 */
static shr_error_e
dnx_algo_template_register_data(
    int unit,
    int core_id,
    int template_index,
    int algo_instance_id,
    dnx_algo_template_create_data_t * data)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * If this is the first core using this template, we also need to allocate it in the sw state.
     *  First core to use the template is always 0 (enforced by input validation).
     */
    if (core_id == 0)
    {
        uint8 is_allocated;
        SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.is_allocated(unit, template_index, &is_allocated));

        if (!is_allocated)
        {
            /*
             * If template is multi core template (indicated by DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE flag) 
             * then we need to allocate one pool per core. Otherwise, one is enough. 
             */
            int nof_cores_to_allocate;

            nof_cores_to_allocate = (_SHR_IS_FLAG_SET(data->flags, DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE))
                ? dnx_data_device.general.nof_cores_get(unit) : 1;

            SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.alloc(unit, template_index, nof_cores_to_allocate));
        }
    }

    /*
     * Simply save the data to the sw state.
     */
    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.flags.set(unit, template_index, core_id, data->flags));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.first_profile.
                    set(unit, template_index, core_id, data->first_profile));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.nof_profiles.
                    set(unit, template_index, core_id, data->nof_profiles));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.data_size.set(unit, template_index, core_id, data->data_size));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.default_profile.
                    set(unit, template_index, core_id, data->default_profile));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.max_references.
                    set(unit, template_index, core_id, data->max_references));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.algo_instance_id.
                    set(unit, template_index, core_id, algo_instance_id));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_template_create(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    dnx_algo_template_create_data_t * data,
    dnx_algo_template_advanced_alogrithm_cb_t * callbacks,
    void *extra_arguments)
{
    int template_index, algo_instance_id;
    int current_core, nof_cores_to_use;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * 0. Verify input and get template index.
     */
    SHR_IF_ERR_EXIT(dnx_algo_template_get_template_index(unit, name, &template_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_create_verify(unit, core_id, name, data, callbacks, template_index));

    /*
     * 1. Assign a template index if it's a new template. 
     *     In warmboot, the template already exists so we don't need to create a new one.
     */
    if (template_index == DNX_ALGO_TEMPLATE_INVALID_INDEX)
    {
        SHR_IF_ERR_EXIT(dnx_algo_template_create_template_index(unit, name, &template_index));
    }

    /*
     * 2. Register the callbacks in the callback array. 
     *    This should be done even if device is in warmboot.
     */
    SHR_IF_ERR_EXIT(dnx_algo_template_register_callbacks(unit, template_index, data->print_cb, callbacks));

    /*
     * If we're in warmboot, then we don't need to save anything else. 
     *   Everything will be automatically restored with the sw state.
     */
    if (!SOC_WARM_BOOT(unit))
    {
        /*
         * 3. Call the create function and register the template data. This should be done per core if it's a duplicate 
         *      per core template.
         *      If it's a separate per core template, this API will be called again and it will be created in that call.
         */

        nof_cores_to_use = (_SHR_IS_FLAG_SET(data->flags, DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE))
            ? dnx_data_device.general.nof_cores_get(unit) : 1;

        for (current_core = 0; current_core < nof_cores_to_use; current_core++)
        {
            /*
             * 3.1 Call the create function.
             */

            SHR_IF_ERR_EXIT(Template_algorithm_cb_array[unit][template_index].create_cb
                            (unit, data, extra_arguments, &algo_instance_id));

            /*
             * 3.2 Register the template data.
             */
            SHR_IF_ERR_EXIT(dnx_algo_template_register_data
                            (unit, current_core, template_index, algo_instance_id, data));
        }
    }
exit:
    SHR_FUNC_EXIT;
}

/*
 * Verify that a profile is within legal range of the template.
 */
static shr_error_e
dnx_algo_template_profile_verify(
    int unit,
    int pool_core,
    int template_index,
    int profile)
{

    int first_profile, nof_profiles;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get first profile and nof_profiles from the sw state.
     */
    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.first_profile.get(unit, template_index, pool_core, &first_profile));

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.nof_profiles.get(unit, template_index, pool_core, &nof_profiles));

    /*
     * Verify.
     */
    if (profile < first_profile)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Profile is too small. Profile is %d and minimum is %d ", profile, first_profile);
    }

    if (profile >= first_profile + nof_profiles)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Profile is too large. Profile is %d and maximum is %d ", profile, first_profile + nof_profiles);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verify that template is legal, and supports the given core_id. 
 */
static shr_error_e
dnx_algo_template_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int template_index)
{
    uint32 flags;
    uint8 multi_core;
    int pool_core;
    SHR_FUNC_INIT_VARS(unit);

    if (template_index == DNX_ALGO_TEMPLATE_INVALID_INDEX)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Template doesn't exist: \"%s.\"", name);
    }

    /* 
     * Use the default pool_core.
     */
    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(_SHR_CORE_ALL);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.flags.get(unit, template_index, pool_core, &flags));

    /*
     * Multi core templates must be called separately per core. 
     *   Otherwise, must be called with _SHR_CORE_ALL. 
     */
    multi_core = (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_CREATE_DUPLICATE_PER_CORE));

    if (multi_core && core_id == _SHR_CORE_ALL)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Template must be used one core a time: \"%s.\"", name);
    }

    if (!multi_core && core_id != _SHR_CORE_ALL)
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Template must be used with _SHR_CORE_ALL: \"%s.\"", name);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verify input for dnx_algo_template_allocate.
 */
static shr_error_e
dnx_algo_template_allocate_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    uint32 flags,
    void *profile_data,
    int *profile,
    uint8 * first_reference,
    int template_index)
{
    int pool_core;
    uint32 template_flags;
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(profile_data, BCM_E_PARAM, "profile_data");
    SHR_NULL_CHECK(profile, BCM_E_PARAM, "profile");
    SHR_NULL_CHECK(first_reference, BCM_E_PARAM, "first_reference")
        SHR_IF_ERR_EXIT(dnx_algo_template_verify(unit, core_id, name, template_index));

    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.flags.get(unit, template_index, pool_core, &template_flags));

    if (_SHR_IS_FLAG_SET(template_flags, DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE))
    {
        SHR_ERR_EXIT(BCM_E_UNAVAIL, "Can't use allocate function when default profile is used: \"%s.\"", name);
    }

    if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID))
    {
        SHR_IF_ERR_EXIT(dnx_algo_template_profile_verify(unit, pool_core, template_index, *profile));
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 *  See description in include/bcm_int/dnx/algo/template_mngr_api.h.
 */
shr_error_e
dnx_algo_template_allocate(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    uint32 flags,
    void *profile_data,
    void *extra_arguments,
    int *profile,
    uint8 * first_reference)
{
    int algo_instance_id, template_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get template_index and verify input.
    */
    SHR_IF_ERR_EXIT(dnx_algo_template_get_template_index(unit, name, &template_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_allocate_verify(unit, core_id, name, flags, profile_data,
                                                            profile, first_reference, template_index));

    /*
     * Fetch the algo_instance id and call the function from the callback.
    */
    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.algo_instance_id.
                    get(unit, template_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(Template_algorithm_cb_array[unit][template_index].allocate_cb
                    (unit, algo_instance_id, flags, profile_data, extra_arguments, profile, first_reference));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Common verification function for dnx_algo_template_profile_data_get and dnx_algo_template_free.
 */
static shr_error_e
dnx_algo_template_and_profile_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int profile,
    int template_index)
{
    int pool_core;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify template. 
     */
    SHR_IF_ERR_EXIT(dnx_algo_template_verify(unit, core_id, name, template_index));

    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    /*
     * Verify profile.
     */
    SHR_IF_ERR_EXIT(dnx_algo_template_profile_verify(unit, pool_core, template_index, profile));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verification for dnx_algo_template_profile_data_get. 
 */
static shr_error_e
dnx_algo_template_profile_data_get_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int profile,
    int *ref_count,
    void *profile_data,
    int template_index)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(ref_count, BCM_E_PARAM, "ref_count");
    SHR_NULL_CHECK(profile_data, BCM_E_PARAM, "profile_data");

    SHR_IF_ERR_EXIT(dnx_algo_template_and_profile_verify(unit, core_id, name, profile, template_index));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See description in include/bcm_int/dnx/algo/template_mngr_api.h. 
 */
shr_error_e
dnx_algo_template_profile_data_get(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int profile,
    int *ref_count,
    void *profile_data)
{
    int algo_instance_id, template_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_template_get_template_index(unit, name, &template_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_profile_data_get_verify
                          (unit, core_id, name, profile, ref_count, profile_data, template_index));

    /*
     * Fetch the algo_instance id and call the function from the callback.
    */
    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.algo_instance_id.
                    get(unit, template_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(Template_algorithm_cb_array[unit][template_index].
                    profile_data_get_cb(unit, algo_instance_id, profile, ref_count, profile_data));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verification for dnx_algo_template_free. 
 */
static shr_error_e
dnx_algo_template_free_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int profile,
    uint8 * last_reference,
    int template_index)
{
    int pool_core;
    uint32 template_flags;
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(last_reference, BCM_E_PARAM, "last_reference");

    SHR_IF_ERR_EXIT(dnx_algo_template_and_profile_verify(unit, core_id, name, profile, template_index));

    /*
     * Verify that free operation is allowed on this template (not allowed when there's 
     *   a default profile) 
     */
    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);
    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.flags.get(unit, template_index, pool_core, &template_flags));

    if (_SHR_IS_FLAG_SET(template_flags, DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE))
    {
        SHR_ERR_EXIT(BCM_E_UNAVAIL, "Can't use allocate function when default profile is used: \"%s.\"", name);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * See description in include/bcm_int/dnx/algo/template_mngr_api.h. 
 */
shr_error_e
dnx_algo_template_free(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    int profile,
    uint8 * last_reference)
{
    int algo_instance_id, template_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_template_get_template_index(unit, name, &template_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_free_verify(unit, core_id, name, profile, last_reference, template_index));

    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.algo_instance_id.
                    get(unit, template_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(Template_algorithm_cb_array[unit][template_index].
                    free_cb(unit, algo_instance_id, profile, last_reference));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Verification for dnx_algo_template_exchange.
 */
static shr_error_e
dnx_algo_template_exchange_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    uint32 flags,
    const void *profile_data,
    int old_profile,
    int *new_profile,
    uint8 * first_reference,
    uint8 * last_reference,
    int template_index)
{
    int pool_core;
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(new_profile, BCM_E_PARAM, "new_profile");
    SHR_NULL_CHECK(first_reference, BCM_E_PARAM, "last_reference");
    SHR_NULL_CHECK(last_reference, BCM_E_PARAM, "last_reference");

    SHR_IF_ERR_EXIT(dnx_algo_template_verify(unit, core_id, name, template_index));

    /*
     * Check flags. 
     * Full explanation is in include/bcm_int/dnx/algo/template_mngr_api.h.  
     */
    if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA))
    {
        if (!_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID))
        {
            SHR_ERR_EXIT(BCM_E_PARAM, "When using DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA flag, "
                         "DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID must also be set.");
        }
    }
    else
    {
        /*
         * If ignore data flag is not set, then profile_data can't be NULL. 
         */
        SHR_NULL_CHECK(profile_data, BCM_E_PARAM, "profile_data");
    }


    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    /*
     * Verify new_profile if WITH_ID.
     */
    if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID))
    {
        SHR_IF_ERR_EXIT(dnx_algo_template_profile_verify(unit, pool_core, template_index, *new_profile));
    }

    /*
     * Verify old profile.
     */
    SHR_IF_ERR_EXIT(dnx_algo_template_profile_verify(unit, pool_core, template_index, old_profile));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See description in include/bcm_int/dnx/algo/template_mngr_api.h.
 */
shr_error_e
dnx_algo_template_exchange(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    uint32 flags,
    const void *profile_data,
    int old_profile,
    const void *extra_arguments,
    int *new_profile,
    uint8 * first_reference,
    uint8 * last_reference)
{
    int algo_instance_id, template_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_template_get_template_index(unit, name, &template_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_exchange_verify(unit, core_id, name, flags, profile_data, old_profile,
                                                            new_profile, first_reference, last_reference, template_index));

    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.algo_instance_id.
                    get(unit, template_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(Template_algorithm_cb_array[unit][template_index].
                    exchange_cb(unit, algo_instance_id, flags, profile_data, old_profile, extra_arguments, new_profile,
                                first_reference, last_reference));

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
dnx_algo_template_profile_get_verify(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    const void *profile_data,
    int *profile,
    int template_index)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(profile_data, BCM_E_PARAM, "profile_data");
    SHR_NULL_CHECK(profile, BCM_E_PARAM, "profile");

    SHR_IF_ERR_EXIT(dnx_algo_template_verify(unit, core_id, name, template_index));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_template_profile_get(
    int unit,
    int core_id,
    dnx_algo_template_name_t name,
    const void *profile_data,
    int *profile)
{
    int algo_instance_id, template_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_template_get_template_index(unit, name, &template_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_profile_get_verify
                          (unit, core_id, name, profile_data, profile, template_index));

    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.algo_instance_id.
                    get(unit, template_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(Template_algorithm_cb_array[unit][template_index].
                    profile_get_cb(unit, algo_instance_id, profile_data, profile));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_template_clear(
    int unit,
    int core_id,
    dnx_algo_template_name_t name)
{
    int algo_instance_id, template_index, pool_core;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_template_get_template_index(unit, name, &template_index));

    SHR_INVOKE_VERIFY_DNX(dnx_algo_template_verify(unit, core_id, name, template_index));

    pool_core = DNX_ALGO_TEMPLATE_GET_POOL_CORE(core_id);

    SHR_IF_ERR_EXIT(TEMPLATE_MNGR_ACCESS.pools_data.algo_instance_id.
                    get(unit, template_index, pool_core, &algo_instance_id));

    SHR_IF_ERR_EXIT(Template_algorithm_cb_array[unit][template_index].clear_cb(unit, algo_instance_id));

exit:
    SHR_FUNC_EXIT;
}
