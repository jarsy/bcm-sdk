/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/** \file lif_mngr.c
 *
 *  Lif algorithms initialization and deinitialization.
 */

#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_LIF

/*************
 * INCLUDES  *
 *************/
/*
 * { 
 */

#include <shared/shrextend/shrextend_debug.h>
#include <soc/dnx/dnx_data/dnx_data_lif.h>
#include <soc/dnx/swstate/access/lif_mngr_access.h>
#include <bcm_int/dnx/algo/lif_mngr/lif_mapping.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>

/*
 * } 
 */
/*************
 * DEFINES   *
 *************/
/*
 * { 
 */

/*
 * } 
 */
/*************
 * MACROS    *
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
/*************
 * FUNCTIONS *
 *************/
/*
 * { 
 */

shr_error_e
dnx_algo_global_lif_is_mapped(
    int unit,
    int global_lif,
    uint8 * is_mapped)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(is_mapped, _SHR_E_PARAM, "is_mapped");

    
    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_lif_mngr_init(
    int unit)
{
    dnx_algo_res_create_data_t inlif_data, outlif_data, global_lif_data;
    uint8 is_init;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&inlif_data, 0, sizeof(dnx_algo_res_create_data_t));
    sal_memset(&outlif_data, 0, sizeof(dnx_algo_res_create_data_t));
    sal_memset(&global_lif_data, 0, sizeof(dnx_algo_res_create_data_t));

    /*
     * Initialize lif mngr sw state. 
     */
    SHR_IF_ERR_EXIT(lif_mngr_access_is_init(unit, &is_init));

    if (!is_init)
    {
        SHR_IF_ERR_EXIT(lif_mngr_access_init(unit));
    }

    /*
     * Create global lif resource.
     */
    global_lif_data.first_element = 0;
    global_lif_data.nof_elements = dnx_data_lif.global_lif.nof_global_in_lifs_get(unit);
    global_lif_data.flags = 0;
    /*
     * data.desc = "Global lif allocation pool.";
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, _SHR_CORE_ALL, "Global lif", &global_lif_data, NULL, NULL));

    /*
     * Create local inlif resource.
     */
    inlif_data.first_element = 0;
    inlif_data.nof_elements = dnx_data_lif.in_lif.nof_local_in_lifs_get(unit);
    inlif_data.flags = 0;
    /*
     * data.desc = "Local inlif allocation pool.";
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, _SHR_CORE_ALL, "Local inlif", &inlif_data, NULL, NULL));

    /*
     * Create local outlif resource.
     */
    outlif_data.first_element = 0;
    outlif_data.nof_elements = dnx_data_lif.out_lif.nof_local_out_lifs_get(unit);
    outlif_data.flags = 0;
    /*
     * data.desc = "Local outlif allocation pool.";
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, _SHR_CORE_ALL, "Local outlif", &outlif_data, NULL, NULL));

    /*
     * Initialize the lif mapping module.
     */
    SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_init(unit));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_lif_mngr_deinit(
    int unit)
{
    uint8 is_init;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Deinitialize the lif mapping module.
     */
    SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_deinit(unit));

    /*
     * Initialize lif mngr sw state. 
     */
    SHR_IF_ERR_EXIT(lif_mngr_access_is_init(unit, &is_init));

    if (!is_init)
    {
        SHR_IF_ERR_EXIT(lif_mngr_access_deinit(unit));
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * } 
 */
