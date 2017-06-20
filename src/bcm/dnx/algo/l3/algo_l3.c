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
#include <shared/util.h>
#include <bcm_int/dnx/algo/template_mngr/template_mngr_api.h>
#include <bcm_int/dnx/algo/l3/source_address_table_allocation.h>
#include <bcm_int/dnx/l3/l3.h>
#include <soc/dnx/dnx_data/dnx_data_l2.h>
#include <soc/dnx/swstate/access/algo_l3_access.h>

#define L3_MYMAC_TABLE_SIZE 64

static void
dnx_algo_l3_print_ingress_mymac_prefix_entry_cb(
    const void *data)
{
    uint8 *mac = (uint8 *) data;
    LOG_CLI((BSL_META_U(0, "0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X"), mac[5], mac[4], mac[3], mac[2], mac[1],
             mac[0]));
    return;
}



static shr_error_e
dnx_algo_l3_ingress_mymac_prefix_entry_template_create(
   int unit)
{    
    dnx_algo_template_create_data_t data;

    SHR_FUNC_INIT_VARS(unit);


    /*
     * Set a template for the MyMac prefix table
     */

    sal_memset(&data, 0, sizeof(data));
    /*
     * The MyMac prefix table contains 64 entries, the first entry is for none MyMac termination VSIs so the
     * template manager will start from the second entry and will use the remaining 63 entries for terminating
     * MyMac prefixes.
     */
    data.data_size      = L3_MAC_ADDR_SIZE_IN_BYTES;
    data.first_profile  = 1;
    data.nof_profiles   = L3_MYMAC_TABLE_SIZE - 1;
    /*
     * Each VSI can point once to the MyMac prefixes table, so the maximal number of references is the VSIs number.
     */
    data.max_references = dnx_data_l2.vsi.nof_vsis_get(unit);
    data.print_cb = &dnx_algo_l3_print_ingress_mymac_prefix_entry_cb;

    SHR_IF_ERR_EXIT(dnx_algo_template_create(unit, _SHR_CORE_ALL, "Ingress MyMac prefix table", &data, NULL, NULL));

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Print an enablers vector entry.
 * \par DIRECT INPUT
 *  \param [in] data - the data to print
 */
static void
dnx_algo_l3_print_enablers_vector_entry_cb(
    const void *data)
{
    uint32* enablers_vector = (uint32 *)data;
    LOG_CLI((BSL_META_U(0, "0x%08X0x%08X"),enablers_vector[0],enablers_vector[1]));
    return;
}

/**
 * \brief
 * Create Template manager for L3 enablers_vector, with the following properties:
 * - entries : each entry is an enablers vector, more than one RIF profile can point to it.
 * - key : the key represents RIF-Profile
 * - the template has a default profile which will always exist in index 0.
 * - max references per profile is number of keys + 1,this is in order to always keep the default profile.
 *   this way no matter how many template exchanges we will do, the default profile won't reach 0 references.
 * - according to the user's flags BCM_L3_INGRESS_ROUTE_DISABLE* the profile will be decided and allocated
 *   (in case it doesnt exist already).
 * - a profile will be deleted when the API bcm_l3_intf_delete will be called and in case no more references
 *   exist for this profile
 * \par DIRECT_INPUT:
 *   \param [in] unit - the unit number
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_algo_l3_enableres_vector_template_create(
   int unit)
{
    dnx_algo_template_create_data_t data;
    uint32 all_ones[2] = {0xFFFFFFFF,0xFFFFFFFF};

    SHR_FUNC_INIT_VARS(unit);

    /*
     * enablers vector table contains 32 entries, first entry will be used as default entry with value:
     * 0xFFFFFFFFFFFFFFFF (can't be deleted), the rest of the entries will be assigned dynamically
     *
     */
    data.data_size = sizeof(uint32)*2;
    data.default_data = all_ones;
    data.default_profile = 0;
    data.first_profile = 0;
    data.flags = DNX_ALGO_TEMPLATE_CREATE_USE_DEFAULT_PROFILE;
    data.nof_profiles = NOF_ENTRIES_ENABLERS_VECTOR;
    
    data.max_references = NOF_ENTRIES_ENABLERS_VECTOR + 1;
    data.print_cb = &dnx_algo_l3_print_enablers_vector_entry_cb;

    SHR_IF_ERR_EXIT(dnx_algo_template_create(unit,_SHR_CORE_ALL,"enablers vector table", &data, NULL, NULL));

exit:
  SHR_FUNC_EXIT;
}

/** } **/

shr_error_e
dnx_algo_l3_init(
    int unit)
{
    uint8 is_init;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Init sw state
     */
    SHR_IF_ERR_EXIT(algo_l3_db.is_init(unit, &is_init));
    if (!is_init)
    {
        SHR_IF_ERR_EXIT(algo_l3_db.init(unit));
    }

    /*
     * Initialize the source address table template.
     */
    SHR_IF_ERR_EXIT(dnx_algo_l3_source_address_table_init(unit));

    /* 
     * Initialize ingress mymac prefix template.
     */
    SHR_IF_ERR_EXIT(dnx_algo_l3_ingress_mymac_prefix_entry_template_create(unit));

    /* 
     * Initialize l3 enablers vector template.
     */
    SHR_IF_ERR_EXIT(dnx_algo_l3_enableres_vector_template_create(unit));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_l3_deinit(
    int unit)
{
    uint8 is_init;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Init sw state
     */
    SHR_IF_ERR_EXIT(algo_l3_db.is_init(unit, &is_init));
    if (is_init)
    {
        SHR_IF_ERR_EXIT(algo_l3_db.deinit(unit));
    }

    /*
     * Resource and template manager don't require deinitialization per instance.
     */

    /* 
     * Deinitialize the source address table allocation algorithm.
     */
    SHR_IF_ERR_EXIT(dnx_algo_l3_source_address_table_deinit(unit));

exit:
    SHR_FUNC_EXIT;
}

/**
 * }
 */
