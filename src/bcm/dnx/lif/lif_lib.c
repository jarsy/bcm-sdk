/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
/** \file lif_lib.c
 * $Id$ 
 * 
 * This file contains the APIs required to allocate and destroy lifs.
 *  
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
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <bcm_int/dnx/algo/lif_mngr/lif_mngr.h>
#include <bcm_int/dnx/algo/lif_mngr/lif_mapping.h>
#include <bcm_int/dnx/lif/lif_lib.h>

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

/*
 * Verification fuction for lif_lib_lif_allocate. 
 * Note - it's possible to verify the global lif id if the WITH_ID flag was given, 
 *  but we currently skip it because it's checked in the resource manager anyway. 
 */
static shr_error_e
lif_lib_lif_allocate_verify(
    int unit,
    uint32 flags,
    int *global_lif,
    lif_lib_local_inlif_info_t * inlif_info,
    lif_lib_local_outlif_info_t * outlif_info)
{
    int is_ingress, is_egress;
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(global_lif, _SHR_E_PARAM, "global_lif");

    is_ingress = (inlif_info != NULL);
    is_egress = (outlif_info != NULL);

    if ((!is_ingress) && (!is_egress))
    {
        /*
         * A lif can be either an inlif or an outlif, or both (symmetric). If neither side is given as 
         *   input, then the lif has no meaning. 
         */
        SHR_ERR_EXIT(_SHR_E_PARAM, "At least one of inlif_info or outlif_info must be not NULL.");
    }

    if (_SHR_IS_FLAG_SET(flags, LIF_LIB_GLOBAL_LIF_WITH_ID)
        && ((is_ingress && !(LIF_LIB_GLOBAL_IN_LIF_IS_LEGAL(unit, *global_lif)))
            || (is_egress && !(LIF_LIB_GLOBAL_OUT_LIF_IS_LEGAL(unit, *global_lif)))))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "WITH_ID flag is set but input global lif 0x%08X is illegal (out of range).",
                     *global_lif);
    }

    /*
     * Verify inlif info.
     */
    if (is_ingress)
    {
        if (inlif_info->app_type < LIF_LIB_INGRESS_LIF_APP_TYPE_FIRST
            || inlif_info->app_type >= LIF_LIB_INGRESS_LIF_APP_TYPE_COUNT)
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Inlif app type %d is illegal (out of range).", (int) inlif_info->app_type);
        }
    }

    /*
     * Verify outlif info.
     */
    if (is_egress)
    {
        if (outlif_info->app_type < LIF_LIB_EGRESS_LIF_APP_TYPE_FIRST
            || outlif_info->app_type >= LIF_LIB_EGRESS_LIF_APP_TYPE_COUNT)
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Inlif app type %d is illegal (out of range).", (int) outlif_info->app_type);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
lif_lib_lif_allocate(
    int unit,
    uint32 flags,
    int *global_lif,
    lif_lib_local_inlif_info_t * inlif_info,
    lif_lib_local_outlif_info_t * outlif_info)
{
    int global_lif_tmp, inlif, outlif;
    int is_ingress, is_egress;
    uint32 global_lif_flags = 0;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify input.
     */
    SHR_INVOKE_VERIFY_DNX(lif_lib_lif_allocate_verify(unit, flags, global_lif, inlif_info, outlif_info));

    is_ingress = (inlif_info != NULL);
    is_egress = (outlif_info != NULL);

    /*
     * Steps 1,2: Allocate lifs, and create mapping. 
     * We use a temporary variable for global lif thourghout the function for code simplicity. 
     */
    global_lif_flags = (_SHR_IS_FLAG_SET(flags, LIF_LIB_GLOBAL_LIF_WITH_ID)) ? DNX_ALGO_RES_ALLOCATE_WITH_ID : 0;
    global_lif_tmp = *global_lif;
    SHR_IF_ERR_EXIT(dnx_algo_res_allocate(unit, _SHR_CORE_ALL, "Global lif", global_lif_flags, NULL, &global_lif_tmp));

    *global_lif = global_lif_tmp;

    /*
     * Allocate and map inlif.
     */
    if (is_ingress)
    {
        /*
         * Allocate.
         */
        SHR_IF_ERR_EXIT(dnx_algo_res_allocate(unit, _SHR_CORE_ALL, "Local inlif", 0, (void *) inlif_info, &inlif));
        /*
         * Create ingress mapping.
         */
        SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_create(unit, DNX_ALGO_LIF_INGRESS, global_lif_tmp, inlif));

        inlif_info->local_inlif = inlif;
    }

    /*
     * Allocate and map outlif.
     */
    if (is_egress)
    {
        /*
         * Allocate.
         */
        SHR_IF_ERR_EXIT(dnx_algo_res_allocate(unit, _SHR_CORE_ALL, "Local outlif", 0, (void *) outlif_info, &outlif));
        /*
         * Create egress mapping.
         */
        SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_create(unit, DNX_ALGO_LIF_EGRESS, global_lif_tmp, outlif));

        outlif_info->local_outlif = outlif;
    }

    /*
     * Steps 3,4: Change HW. 
     */
    if (is_egress)
    {
        

        
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Note that this function doesn't verify input. We expect the input to be varified by resource manager.
 */
shr_error_e
lif_lib_lif_deallocate(
    int unit,
    int global_lif,
    int local_inlif,
    int local_outlif)
{
    uint8 is_ingress, is_egress;
    SHR_FUNC_INIT_VARS(unit);

    is_ingress = (local_inlif != LIF_LIB_INVALID);
    is_egress = (local_outlif != LIF_LIB_INVALID);

    /*
     * Steps 1,2: Dellocate lifs, and remove mapping.
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_free(unit, _SHR_CORE_ALL, "Global lif", global_lif));

    /*
     * Deallocate and unmap inlif.
     */
    if (is_ingress)
    {
        /*
         * Deallocate.
         */
        SHR_IF_ERR_EXIT(dnx_algo_res_free(unit, _SHR_CORE_ALL, "Local inlif", local_inlif));
        /*
         * Remove ingress mapping.
         */
        SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_remove(unit, DNX_ALGO_LIF_INGRESS, global_lif));
    }

    /*
     * Deallocate and unmap outlif.
     */
    if (is_egress)
    {
        /*
         * Deallocate.
         */
        SHR_IF_ERR_EXIT(dnx_algo_res_free(unit, _SHR_CORE_ALL, "Local outlif", local_outlif));
        /*
         * Remove egress mapping.
         */
        SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_remove(unit, DNX_ALGO_LIF_EGRESS, global_lif));
    }

    /*
     * Steps 3,4: Change HW. 
     */
    if (is_egress)
    {
        

        
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * } 
 */
