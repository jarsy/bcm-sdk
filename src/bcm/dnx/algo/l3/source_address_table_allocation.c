/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/** \file algo_l3.c
 *
 * Wrapper functions for utilex_multi_set.
 *
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_TEMPLATEMNGR
/**
* INCLUDE FILES:
* {
*/

#include <shared/shrextend/shrextend_debug.h>
#include <shared/swstate/sw_state_res_tag_bitmap.h>
#include <bcm_int/dnx/algo/template_mngr/multi_set_template.h>
#include <bcm_int/dnx/algo/template_mngr/template_mngr_api.h>
#include <bcm_int/dnx/algo/l3/source_address_table_allocation.h>
#include <soc/dnx/dnx_data/dnx_data_l3.h>
#include <soc/dnx/dnx_data/dnx_data_lif.h>
#include <soc/dnx/swstate/access/algo_l3_access.h>

/** } **/

/** 
 *  Source address table template advanced algorithm.
 *  
 *  The algorithm works as following:
 *  
 *  We hold a res tag bitmap that has the same ID range as the source address table template.
 *  When allocating an entry for the first time, we allocate it in the res tag bitmap, since it allows
 *   us to allocate the number of indexes we need, a functionality that doesn't exist in template manager.
 *   Then we use the ID allocated in the resource manager to save the entry in the template manager.
 *  
 *  {
 */

/**
 * The tags are used to identify the top and bottom halves of the table. 
 * We need to distinguish them because only the bottom half can hold mac addresses. 
 */
#define SOURCE_ADDRESS_BOTTOM_HALF_TAG  'b'

#define SOURCE_ADDRESS_TOP_HALF_TAG     't'

/**
 * Each address type takes a different number of entries in the table. This array maps
 *   between the type and the number of entries.
 *  
 *   source_address_type_mac    : 2
 *   source_address_type_ipv4   : 1
 *   source_address_type_ipv6   : 4
 */
static int address_type_to_nof_entries_map[source_address_type_count] = { 2, 1, 4 };


static void
dnx_algo_l3_source_address_table_entry_print(
    const void *data)
{
    source_address_entry_t *entry_ptr = (source_address_entry_t *) data;

    switch (entry_ptr->address_type)
    {
        case source_address_type_ipv4:
            LOG_CLI((BSL_META_U(0, "Address type: IPv4\n")));
            break;
        case source_address_type_ipv6:
            LOG_CLI((BSL_META_U(0, "Address type: IPv6\n")));
            break;
        case source_address_type_mac:
            LOG_CLI((BSL_META_U(0, "Address type: MAC\n")));
            break;
        default:
            LOG_CLI((BSL_META_U(0, "Invalid entry type.\n")));
    }
}

shr_error_e
dnx_algo_l3_source_address_table_create(
    int unit,
    dnx_algo_template_create_data_t * create_data,
    void *extra_arguments,
    int *algo_instance_id)
{
    sw_state_res_tag_bitmap_handle_t bitmap_handle;
    char tag;
    int half_bank;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * 1. Create a regular multi_set_template.
     */
    SHR_IF_ERR_EXIT(multi_set_template_create(unit, create_data, NULL, algo_instance_id));

    /*
     * 2. Create a res_tag_bitmap to manage the entries.
     *    The tag will distinguish the top half of the table (tag 't') from the bottom half (tag 'b').
     *    We need this distinction because only the bottom half can hold mac entries.
     */
    SHR_IF_ERR_EXIT(sw_state_res_tag_bitmap_create(unit,
                                                   &bitmap_handle,
                                                   create_data->first_profile,
                                                   create_data->nof_profiles,
                                                   create_data->nof_profiles / 2, sizeof(char)));

    /*
     * Set the tags.
     */
    half_bank = create_data->nof_profiles / 2;
    tag = SOURCE_ADDRESS_BOTTOM_HALF_TAG;
    SHR_IF_ERR_EXIT(sw_state_res_tag_bitmap_tag_set(unit, bitmap_handle, (void *) &tag,
                                                    create_data->first_profile, half_bank));

    tag = SOURCE_ADDRESS_TOP_HALF_TAG;
    SHR_IF_ERR_EXIT(sw_state_res_tag_bitmap_tag_set(unit, bitmap_handle, (void *) &tag,
                                                    create_data->first_profile + half_bank, half_bank));

    /* 
     * Save res tag bitmap handle in the sw state.
     */
    SHR_IF_ERR_EXIT(algo_l3_db.source_address_table_allocation.res_tag_bitmap_handle.set(unit, bitmap_handle));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Verification function for dnx_algo_l3_source_address_table_allocate.
 *   We assume that all pointers were already checked for NULL by the template manager, and that 
 *   the entry was checked for being in a legal range. 
 *  
 *  Therefore, we only verify that the address type is legal, and that it fits with the given profile if 
 *   WITH_ID flag was used. 
 */
shr_error_e
dnx_algo_l3_source_address_table_allocate_verify(
    int unit,
    uint32 flags,
    source_address_entry_t * profile_data,
    int profile)
{
    int nof_entries_to_allocate;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify profile data.
     */
    if ((profile_data->address_type == source_address_type_invalid)
        || (profile_data->address_type >= source_address_type_count))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Illegal address type. Was %d.", (int) profile_data->address_type);
    }

    /*
     * Verify given profile if WITH_ID. 
     * It should:
     *   A. Be a multiple of the number of entries this address type uses.
     *   B. If it's a mac entry, it can only be in the bottom half of the entries.
     */
    if (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID))
    {
        nof_entries_to_allocate = address_type_to_nof_entries_map[profile_data->address_type];

        if (profile % nof_entries_to_allocate != 0)
        {
            SHR_ERR_EXIT(_SHR_E_PARAM,
                         "Given profile is illegal for this address type. Needs to be a multiple of %d but was %d",
                         nof_entries_to_allocate, profile);
        }
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_l3_source_address_table_allocate(
    int unit,
    int algo_instance_id,
    uint32 flags,
    void *profile_data,
    void *extra_arguments,
    int *profile,
    uint8 * first_reference)
{
    source_address_entry_t *entry_ptr = (source_address_entry_t *) profile_data;
    int tmp_profile, rv;
    uint8 allocation_needed = TRUE, with_id;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_algo_l3_source_address_table_allocate_verify(unit, flags, entry_ptr, *profile));

    with_id = (_SHR_IS_FLAG_SET(flags, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID));

    /*
     * 1. Check if the data already exists or if it needs to be allocated.
     */
    rv = multi_set_template_profile_get(unit, algo_instance_id, profile_data, &tmp_profile);

    /*
     * If E_NOT_FOUND was returned, it means the data wasn't allocated yet and we need to allocate it. 
     * Otherwise, check for errors, and if none were returned, it means the data is already allocated. 
     * If WITH_ID, make sure the existing profile match the given profile.
     * Otherwise, save the profile we found and use it with the multi set template.
     */
    if (rv != _SHR_E_NOT_FOUND)
    {
        SHR_IF_ERR_EXIT(rv);
        if (with_id && (*profile != tmp_profile))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "This data already exists in profile %d. Given profile %d", tmp_profile,
                         *profile);
        }

        allocation_needed = FALSE;
        *profile = tmp_profile;
    }

    /*
     * 2. If data doesn't already exist, allocate in the res tag bitmap.
     */
    if (allocation_needed)
    {
        sw_state_res_tag_bitmap_handle_t bitmap_handle;
        uint8 tag, allocation_successful = FALSE;
        int nof_entries_to_allocate;
        uint32 res_tag_bitmap_flags;

        SHR_IF_ERR_EXIT(algo_l3_db.source_address_table_allocation.res_tag_bitmap_handle.get(unit, &bitmap_handle));
        nof_entries_to_allocate = address_type_to_nof_entries_map[entry_ptr->address_type];
        res_tag_bitmap_flags = with_id ? SW_STATE_RES_TAG_BITMAP_ALLOC_WITH_ID : 0;
        res_tag_bitmap_flags |= SW_STATE_RES_TAG_BITMAP_ALLOC_ALIGN_ZERO | SW_STATE_RES_TAG_BITMAP_ALWAYS_CHECK_TAG;

        if (entry_ptr->address_type != source_address_type_mac)
        {
            /*
             * In case of IP (not ethernet) the entire table can be used. First attempt to allocate in the top half, 
             *   so the bottom half will be reserved for mac. 
             */
            tag = SOURCE_ADDRESS_TOP_HALF_TAG;
            rv = sw_state_res_tag_bitmap_alloc_align_tag(unit, bitmap_handle, flags, nof_entries_to_allocate, 0,
                                                         &tag, nof_entries_to_allocate, profile);

            /*
             * If E_RESOURCE was returned, it means that allocation was unsuccessful. 
             * We'll attempt to allocate below.
             * Otherwise, check that no other error was returned, and set the the allocation 
             * succeeded.
             */
            if (rv != _SHR_E_RESOURCE)
            {
                SHR_IF_ERR_EXIT(rv);
                allocation_successful = TRUE;
            }
        }

        /*
         * Attempt to allocate in the bottom half if we didn't allocate in the top.
         */
        if (!allocation_successful)
        {
            tag = SOURCE_ADDRESS_BOTTOM_HALF_TAG;
            SHR_IF_ERR_EXIT(sw_state_res_tag_bitmap_alloc_align_tag
                            (unit, bitmap_handle, flags, nof_entries_to_allocate, 0, &tag,
                             nof_entries_to_allocate, profile));
        }
    }

    /*
     * 3. Use the allocated profile for the multi set template WITH_ID.
     */
    SHR_IF_ERR_EXIT(multi_set_template_allocate(unit, algo_instance_id, DNX_ALGO_TEMPLATE_ALLOCATE_WITH_ID,
                                                profile_data, NULL, profile, first_reference));

    /*
     * Sanity check: allocation_needed and first_reference should be the same.
     */
    if (allocation_needed != *first_reference)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Algorithm inconstistency. allocation_needed: %d, first_reference: %d",
                     allocation_needed, *first_reference);
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_l3_source_address_table_free(
    int unit,
    int algo_instance_id,
    int profile,
    uint8 * last_reference)
{
    int dummy_ref_count;
    source_address_entry_t entry_data;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get the entry data, in case we need to deallocate it from the resource manager.
     */
    SHR_IF_ERR_EXIT(multi_set_template_profile_data_get
                    (unit, algo_instance_id, profile, &dummy_ref_count, (void *) &entry_data));

    /*
     * Free from the multi set template.
     */
    SHR_IF_ERR_EXIT(multi_set_template_free(unit, algo_instance_id, profile, last_reference));

    /*
     * If last reference, free from the res tag bitmap as well.
     */
    if (*last_reference)
    {
        int nof_entries_to_deallocate;
        sw_state_res_tag_bitmap_handle_t bitmap_handle;

        SHR_IF_ERR_EXIT(algo_l3_db.source_address_table_allocation.res_tag_bitmap_handle.get(unit, &bitmap_handle));

        nof_entries_to_deallocate = address_type_to_nof_entries_map[entry_data.address_type];

        SHR_IF_ERR_EXIT(sw_state_res_tag_bitmap_free(unit, bitmap_handle, nof_entries_to_deallocate, profile));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_l3_source_address_table_exchange(
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
    SHR_FUNC_INIT_VARS(unit);
    SHR_EXIT();

    
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_l3_source_address_table_clear(
    int unit,
    int algo_instance_id)
{
    SHR_FUNC_INIT_VARS(unit);

    
    SHR_IF_ERR_EXIT(multi_set_template_clear(unit, algo_instance_id));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Initialize the source address table allocation algoirthm, implemented by tempalte manager.
 */
shr_error_e
dnx_algo_l3_source_address_table_init(
    int unit)
{
    dnx_algo_template_create_data_t source_address_template_data;

    dnx_algo_template_advanced_alogrithm_cb_t dnx_algo_l3_source_address_table_algo = {
        dnx_algo_l3_source_address_table_create,
        dnx_algo_l3_source_address_table_allocate,
        dnx_algo_l3_source_address_table_exchange,
        multi_set_template_profile_data_get,
        multi_set_template_profile_get,
        dnx_algo_l3_source_address_table_free,
        NULL,
        dnx_algo_l3_source_address_table_clear
    };

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Create template.
     */
    sal_memset(&source_address_template_data, 0, sizeof(source_address_template_data));

    source_address_template_data.flags = DNX_ALGO_TEMPLATE_CREATE_USE_ADVANCED_ALGORITHM;
    source_address_template_data.data_size = sizeof(source_address_entry_t);
    source_address_template_data.first_profile = 0;
    source_address_template_data.max_references = dnx_data_lif.out_lif.nof_local_out_lifs_get(unit);
    source_address_template_data.nof_profiles = dnx_data_l3.source_address.source_address_table_size_get(unit);
    source_address_template_data.print_cb = &dnx_algo_l3_source_address_table_entry_print;

    SHR_IF_ERR_EXIT(dnx_algo_template_create
                    (unit, _SHR_CORE_ALL, "Egress source address table", &source_address_template_data,
                     &dnx_algo_l3_source_address_table_algo, NULL));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e dnx_algo_l3_source_address_table_deinit(
    int unit){
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Sw state is deinitilzed in algo_l3.c
     */

    /*
     * Resource and template manager don't require deinitialization per instance.
     */

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}


/**
 * }
 */
