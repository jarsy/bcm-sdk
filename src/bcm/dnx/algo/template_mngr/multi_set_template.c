/** \file multi_set_template.c
 *
 * Wrapper functions for utilex_multi_set.
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
#include <soc/dnx/legacy/dnx_config_defs.h>

#define MULTI_SET_TEMPLATE_ACCESS sw_state_access[unit].dnx.bcmdnx.multi_set_template

shr_error_e
multi_set_template_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * The algo instance id is identical to the multi set ptr id.
     *  Therefore, allocate the wrapper sw state to mirror the number of multi sets.
     *  Allocate one extra since multi set PTRs start from one.
     */
    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.alloc(unit, MAX_NOF_MULTIS_FOR_DNX + 1));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
multi_set_template_create(
    int unit,
    dnx_algo_template_create_data_t * create_data,
    void *extra_arguments,
    int *algo_instance_id)
{
    UTILEX_MULTI_SET_PTR multi_set_ptr;
    UTILEX_MULTI_SET_INIT_INFO multi_set_init_info;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Init hash profile 
     */
    sal_memset(&multi_set_init_info, 0x0, sizeof(UTILEX_MULTI_SET_INIT_INFO));
    multi_set_init_info.max_duplications = create_data->max_references;
    multi_set_init_info.member_size = create_data->data_size;
    multi_set_init_info.nof_members = create_data->nof_profiles;

    SHR_IF_ERR_EXIT(utilex_multi_set_create(unit, &multi_set_ptr, multi_set_init_info));

    /*
     * Algo instance id is identical to the multi set ptr.
     */
    *algo_instance_id = multi_set_ptr;

    /*
     * Save the data that is required for wrapping the multi set.
     */
    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.data_size.
                    set(unit, *algo_instance_id, create_data->data_size));
    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.first_profile.
                    set(unit, *algo_instance_id, create_data->first_profile));

    /*
     * If template uses a default profile, allocate it.
     */
    if (_SHR_IS_FLAG_SET(create_data->flags, DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE)) {
        int nof_allocations = create_data->max_references;
        uint8 first_reference;
        SHR_IF_ERR_EXIT(multi_set_template_allocate(unit, *algo_instance_id, 
                                                    MULTI_SET_TEMPLATE_ALLOCATE_SEVERAL | DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID, 
                                                    create_data->default_data, &nof_allocations, &create_data->default_profile, &first_reference));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
multi_set_template_allocate(
    int unit,
    int algo_instance_id,
    uint32 flags,
    void *profile_data,
    void *extra_arguments,
    int *profile,
    uint8 * first_reference)
{
    uint8 add_success;
    int profile_alloc = 0, first_profile;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.first_profile.get(unit, algo_instance_id, &first_profile));

    if (_SHR_IS_FLAG_SET(flags, MULTI_SET_TEMPLATE_ALLOCATE_SEVERAL))
    {
        int *nof_allocations_ptr;
        if (!_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM,
                         "Can't use flag MULTI_SET_TEMPLATE_ALLOCATE_SEVERAL without DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID.");
        }
        SHR_NULL_CHECK(extra_arguments, _SHR_E_PARAM, "extra_arguments");

        nof_allocations_ptr = (int *) extra_arguments;

        profile_alloc = *profile - first_profile;

        SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index_nof_additions(unit,
                                                                           algo_instance_id,
                                                                           profile_data,
                                                                           profile_alloc,
                                                                           *nof_allocations_ptr,
                                                                           first_reference, &add_success));
    }
    else if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID))
    {
        profile_alloc = *profile - first_profile;
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index(unit,
                                                             algo_instance_id,
                                                             profile_data,
                                                             profile_alloc, first_reference, &add_success));
    }
    else
    {
        /*
         * Add new data 
         */
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,
                                                    algo_instance_id,
                                                    profile_data,
                                                    (uint32 *) & profile_alloc, first_reference, &add_success));
        *profile = profile_alloc + first_profile;
    }

    if (!add_success)
    {
        SHR_ERR_EXIT(_SHR_E_FULL, "Failed to allocate profile.");
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
multi_set_template_free(
    int unit,
    int algo_instance_id,
    int profile,
    uint8 * last_reference)
{
    int profile_alloc = 0, first_profile;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.first_profile.get(unit, algo_instance_id, &first_profile));

    /*
     * Remove old profile 
     */
    profile_alloc = profile - first_profile;
    SHR_IF_ERR_EXIT(utilex_multi_set_member_remove_by_index(unit, algo_instance_id, profile_alloc, last_reference));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
multi_set_template_exchange(
    int unit,
    int algo_instance_id,
    uint32 flags,
    const void *profile_data,
    int old_profile,
    const void *extra_arguments,
    int *new_profile,
    uint8 * first_reference,
    uint8 * last_reference)
{
    UTILEX_MULTI_SET_KEY *old_profile_data = NULL, *new_profile_old_data = NULL;
    uint32 old_profile_alloc = 0, new_profile_alloc = 0;
    int first_profile;
    uint32 ref_count_old;
    uint32 new_profile_old_ref_count;
    uint8 old_profile_freed = FALSE, new_profile_old_data_freed = FALSE;
    int data_size;
    uint8 add_success = TRUE;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.first_profile.get(unit, algo_instance_id, &first_profile));
    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.data_size.get(unit, algo_instance_id, &data_size));

    /*
     * First, we save the old data, to restore it in case the allocation fails, or in case of test.
     */
    old_profile_data = sal_alloc(data_size, "Data buffer old.");

    if (old_profile_data == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_MEMORY, "Failed to allocate pointer old_profile_data.");
    }

    old_profile_alloc = old_profile - first_profile;

    SHR_IF_ERR_EXIT(utilex_multi_set_get_by_index(unit,
                                                  algo_instance_id,
                                                  old_profile_alloc, old_profile_data, &ref_count_old));

    if ((ref_count_old == 0) && !_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_NOT_EXIST_OLD_PROFILE))
    {
        /*
         * User gave old profile that was empty
         */
        SHR_ERR_EXIT(_SHR_E_PARAM, "Given old_profile %d doesn't exist.", old_profile);
    }

    if (ref_count_old != 0)
    {
        /*
         *  Remove old profile if it exists.
         */
        SHR_IF_ERR_EXIT(utilex_multi_set_member_remove_by_index(unit, algo_instance_id, old_profile_alloc, last_reference));

        old_profile_freed = TRUE;
    }

    /*
     * WITH_ID means that the user would like to exchange the current profile for a specific new profile
     *   (as opposed to changing the profile to any new profile containing the supplied data).
     * If IGNORE_DATA is not specified, then the user would also like to change the value of the new template. 
     */
    if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID))
    {

        new_profile_alloc = *new_profile - first_profile;

        if (!_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA))
        {
            uint32 new_data_ref_count;
            uint32 old_profile_of_new_data;

            /*
             * Make sure that the new data does not exist already for another profile.
             * This check is actually made inside the insert by index as well. 
             * However no distinguishable error is returned for it.
             */
            SHR_IF_ERR_EXIT(utilex_multi_set_member_lookup(unit,
                                                           algo_instance_id,
                                                           (UTILEX_MULTI_SET_KEY *) profile_data,
                                                           &old_profile_of_new_data, &new_data_ref_count));

            if ((old_profile_of_new_data != new_profile_alloc) && (new_data_ref_count > 0))
            {
                SHR_ERR_EXIT(_SHR_E_EXISTS, "Given new data already exists on profile %d.",
                             old_profile_of_new_data + first_profile);
            }
        }

        /*
         *  Get the current data at index (and the ref count to it) and save it into the buffer in case of failure.
         */
        new_profile_old_data = sal_alloc(data_size, "Old data of new profile..");

        if (new_profile_old_data == NULL)
        {
            SHR_ERR_EXIT(_SHR_E_MEMORY, "Failed to allocate pointer new_profile_old_data.");
        }

        SHR_IF_ERR_EXIT(utilex_multi_set_get_by_index(unit,
                                                      algo_instance_id,
                                                      new_profile_alloc,
                                                      new_profile_old_data, &new_profile_old_ref_count));

        if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA) && (new_profile_old_ref_count == 0))
        {
            /*
             * WITH_ID and IGNORE_DATA are invalid if the profile does not exist - because we don't know where to get 
             * the new data from. 
             */
            SHR_ERR_EXIT(_SHR_E_NOT_FOUND,
                         "Can't use IGNORE_DATA and WITH_ID flags if there were no references to this profile.");
        }

        /*
         *  If we ignore the data or we have a new profile, then we need not change the existing profile value.
         */
        if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_EXCHANGE_IGNORE_DATA) || (new_profile_old_ref_count == 0))
        {
            /*
             * Add data by index. 
             */
            SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index(unit,
                                                                 algo_instance_id,
                                                                 new_profile_old_data,
                                                                 new_profile_alloc, first_reference, &add_success));
        }
        else    /* We need to change the current profile data. */
        {
            uint8 last_appear;

            /*
             *  Free all members using the old data.
             */
            SHR_IF_ERR_EXIT(utilex_multi_set_member_remove_by_index_multiple(unit,
                                                                             algo_instance_id,
                                                                             new_profile_alloc,
                                                                             new_profile_old_ref_count, &last_appear));

            new_profile_old_data_freed = TRUE;

            /*
             *  Add ref_count + 1 members using the new data at the required index.
             */
            SHR_IF_ERR_EXIT(utilex_multi_set_member_add_at_index_nof_additions(unit,
                                                                               algo_instance_id,
                                                                               (UTILEX_MULTI_SET_KEY *) profile_data,
                                                                               new_profile_alloc,
                                                                               new_profile_old_ref_count + 1,
                                                                               first_reference, &add_success));
        }
    }
    else        /* if (!WITH_ID) */
    {
        /*
         *  Add new data.
         */
        SHR_IF_ERR_EXIT(utilex_multi_set_member_add(unit,
                                                    algo_instance_id,
                                                    (UTILEX_MULTI_SET_KEY *) profile_data,
                                                    (uint32 *) & new_profile_alloc, first_reference, &add_success));

        *new_profile = new_profile_alloc + first_profile;
    }

    /*
     * If this is just an exchange test, then free the entries that were just allocated.
     */
    if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_EXCHANGE_TEST))
    {
        uint8 last_appear;
        if (new_profile_old_data_freed)
        {
            /*
             * Deallocate all references to this new data.
             */
            SHR_IF_ERR_EXIT(utilex_multi_set_member_remove_by_index_multiple(unit,
                                                                             algo_instance_id,
                                                                             new_profile_alloc,
                                                                             new_profile_old_ref_count + 1,
                                                                             &last_appear));
        }
        else
        {
            /*
             * Deallocate only one reference.
             */
            SHR_IF_ERR_EXIT(utilex_multi_set_member_remove_by_index(unit,
                                                                    algo_instance_id, new_profile_alloc, &last_appear));

        }
    }

exit:
    if (SHR_FUNC_ERR() || !add_success || _SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_EXCHANGE_TEST))
    {
        int rv;
        uint8 first_appear;
        /*
         *  Add new failed, or test. Restore old data.
         */
        if (!add_success)
        {
            SHR_SET_CURRENT_ERR(_SHR_E_FULL);
        }

        if (old_profile_freed)
        {

            rv = utilex_multi_set_member_add_at_index(unit,
                                                      algo_instance_id,
                                                      old_profile_data, old_profile_alloc, &first_appear, &add_success);

            if (SHR_FAILURE(rv) || !add_success)
            {
                /*
                 *  Can't restore data. Internal error
                 */
                SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
            }
        }

        if (new_profile_old_data_freed && !(SHR_FUNC_VAL_IS(_SHR_E_INTERNAL)))
        {
            rv = utilex_multi_set_member_add_at_index_nof_additions(unit,
                                                                    algo_instance_id,
                                                                    new_profile_old_data,
                                                                    new_profile_alloc,
                                                                    new_profile_old_ref_count,
                                                                    &first_appear, &add_success);
            if (SHR_FAILURE(rv) || !add_success)
            {
                /*
                 *  Can't restore data. Internal error
                 */
                SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
            }
        }
    }

    /*
     * Free pointers.
     */
    SHR_FREE(old_profile_data);
    SHR_FREE(new_profile_old_data);

    SHR_FUNC_EXIT;
}

shr_error_e
multi_set_template_profile_data_get(
    int unit,
    int algo_instance_id,
    int profile,
    int *ref_count,
    void *data)
{
    int profile_alloc;
    int first_profile;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.first_profile.get(unit, algo_instance_id, &first_profile));

    profile_alloc = profile - first_profile;

    SHR_IF_ERR_EXIT(utilex_multi_set_get_by_index(unit, algo_instance_id, profile_alloc, data, (uint32 *) ref_count));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
multi_set_template_profile_get(
    int unit,
    int algo_instance_id,
    const void *data,
    int *profile)
{
    uint32 profile_alloc;
    uint32 ref_count;
    int first_profile;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(MULTI_SET_TEMPLATE_ACCESS.pools_data.first_profile.get(unit, algo_instance_id, &first_profile));

    SHR_IF_ERR_EXIT(utilex_multi_set_member_lookup(unit,
                                                   algo_instance_id,
                                                   (UTILEX_MULTI_SET_KEY *) data, &profile_alloc, &ref_count));

    if (ref_count == 0)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_NOT_FOUND);
    }
    else
    {
        *profile = profile_alloc + first_profile;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
multi_set_template_clear(
    int unit,
    int algo_instance_id)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(utilex_multi_set_clear(unit, algo_instance_id));

exit:
    SHR_FUNC_EXIT;
}
