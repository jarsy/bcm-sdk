/** \file algo_mirror.c
 *
 * Mirror related algorithm functions for DNX.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_MIRROR
/**
* INCLUDE FILES:
* {
*/

/*
 * Include files which are specifically for DNX. Final location.
 * {
 */
#include <soc/dnx/dnx_data/dnx_data_device.h>
#include <soc/dnx/dnx_data/dnx_data_snif.h>
#include <soc/dnx/dbal/dbal.h>
#include <bcm_int/dnx/algo/mirror/algo_mirror.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
/*
 * }
 */

/*
 * Other include files. 
 * { 
 */
#include <shared/swstate/access/sw_state_access.h>
#include <shared/utilex/utilex_bitstream.h>

/*
 * }
 */

/**
 * }
 */

/*
 * See .h file
 */
shr_error_e
dnx_algo_mirror_probability_get(
    int unit,
    uint32 dividend,
    uint32 divisor,
    int prob_nof_bits,
    uint32 * prob_field)
{
    uint32 prob_always, array32[1];
    uint64 val64, val64_divisor;

    SHR_FUNC_INIT_VARS(unit);
    *prob_field = 0;

    /*
     * Calculate always mirror probability by setting all bits to 1. 
     */
    SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(array32, 0, prob_nof_bits - 1));
    prob_always = array32[0];
    /*
     * 100% probability of mirror execution
     */
    if (dividend >= divisor)
    {
        *prob_field = prob_always;
    }
    else
    {
        /*
         * Calculate probability using 64 bit multiplication and division. 
         * The probability is calculated as following: 
         *  (2^prob_nof_bits-prob_field)/2^prob_nof_bits 
         */
        COMPILER_64_SET(val64, 0, dividend);
        COMPILER_64_SET(val64_divisor, 0, divisor);
        COMPILER_64_SHL(val64, prob_nof_bits);
        COMPILER_64_ADD_32(val64, divisor / 2);
        COMPILER_64_UDIV_64(val64, val64_divisor);
        COMPILER_64_TO_32_LO(*prob_field, val64);
        /*
         * Do not disable mirroring with non zero probability 
         */
        if (*prob_field <= 1)
        {
            if (dividend > 0)
            {
                *prob_field = 1;
            }
        }
        /*
         * Fix round up error
         */
        else if (*prob_field > prob_always)
        {
            *prob_field = prob_always;
        }
        else
        {
            (*prob_field)--;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_algo_mirror_profile_allocate(
    int unit,
    bcm_mirror_destination_t * mirror_dest,
    int *action_profile_id)
{
    int alloc_flags;
    dnx_algo_res_name_t res_alloc_name;

    SHR_FUNC_INIT_VARS(unit);

    alloc_flags = 0;

    /*
     * Get given profile ID 
     */
    if (mirror_dest->flags == BCM_MIRROR_DEST_WITH_ID)
    {
        *action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest->mirror_dest_id);
        alloc_flags = DNX_ALGO_RES_ALLOCATE_WITH_ID;
    }

    /*
     * allocate snif profile according to application (snoop, mirror, statistical sampling)  
     */
    if (mirror_dest->flags & BCM_MIRROR_DEST_IS_SNOOP)
    {
        res_alloc_name = DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_SNOOP;
    }
    else
    {
        res_alloc_name = DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_MIRROR;
    }

    /*
     * allocate SNIF profile
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_allocate(unit, BCM_CORE_ALL, res_alloc_name, alloc_flags, NULL, action_profile_id));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_algo_mirror_profile_deallocate(
    int unit,
    bcm_gport_t mirror_dest_id)
{
    int action_profile_id;
    dnx_algo_res_name_t res_alloc_name;

    SHR_FUNC_INIT_VARS(unit);

    action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest_id);

    /*
     * deallocate snif profile according to application (snoop, mirror, statistical sampling)  
     */
    if (BCM_GPORT_IS_MIRROR_SNOOP(mirror_dest_id))
    {
        /** snoop profile */
        res_alloc_name = DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_SNOOP;
    }
    else if (BCM_GPORT_IS_MIRROR_MIRROR(mirror_dest_id))
    {
        /** mirror profile */
        res_alloc_name = DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_MIRROR;
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "An unsupported gport type: 0x%x", mirror_dest_id);
    }

    /** free snif profile */
    SHR_IF_ERR_EXIT(dnx_algo_res_free(unit, BCM_CORE_ALL, res_alloc_name, action_profile_id));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_algo_mirror_profile_is_allocated(
    int unit,
    bcm_gport_t mirror_dest_id,
    uint8 * is_allocated)
{
    int action_profile_id;
    dnx_algo_res_name_t res_alloc_name;

    SHR_FUNC_INIT_VARS(unit);

    action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest_id);

    /*
     * deallocate snif profile according to application (snoop, mirror, statistical sampling)  
     */
    if (BCM_GPORT_IS_MIRROR_SNOOP(mirror_dest_id))
    {
        /** snoop profile */
        res_alloc_name = DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_SNOOP;
    }
    else if (BCM_GPORT_IS_MIRROR_MIRROR(mirror_dest_id))
    {
        /** mirror profile */
        res_alloc_name = DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_MIRROR;
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "An unsupported gport type: 0x%x", mirror_dest_id);
    }

    /** Check if given profile is already allocated */
    SHR_IF_ERR_EXIT(dnx_algo_res_is_allocated(unit, BCM_CORE_ALL, res_alloc_name, action_profile_id, is_allocated));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Initialize resource manager for mirror module.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Snif resource manager tables
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_algo_mirror_res_mngr_init(
    int unit)
{
    dnx_algo_res_create_data_t data;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Ingress Mirror resource manager  
     */
    {
        /*
         * First element should be 1 instead of 0 since profile 0 is special, profile 0 is reserved as default in HW 
         * and means don't mirror 
         */
        /** First profile id in mirror res mngr */
        data.first_element = DNX_ALGO_MIRROR_INGRESS_PROFILE_MIN;
        /** Number of mirror profiles is nof_profiles-1 since the first profile is resereved */
        data.nof_elements = dnx_data_snif.ingress.nof_profiles_get(unit) - DNX_ALGO_MIRROR_INGRESS_PROFILE_MIN;
        /** Currently sniffing isn't done per core */
        data.flags = 0;
        /*
         * data.desc = "SNIF - SNIF profiles allocated IDs";
         */
        SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, BCM_CORE_ALL, DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_MIRROR, &data,
                                            NULL, NULL));
    }

    /*
     * Ingress Snoop resource manager
     */
    {
        /** First profile id in snoop res mngr */
        data.first_element = 0;
        /** number of snoop profiles */
        data.nof_elements = dnx_data_snif.ingress.nof_profiles_get(unit);
        /** Currently sniffing isn't done per core */
        data.flags = 0;
        /*
         * data.desc = "SNIF - SNIF profiles allocated IDs";
         */
        SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, BCM_CORE_ALL, DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_SNOOP, &data,
                                            NULL, NULL));
    }

    /*
     * Ingress Statistical sampling resource manager
     */
    {
        /** First profile id in statistical sampling res mngr */
        data.first_element = 0;
        /** number of statistical sampling profiles*/
        data.nof_elements = dnx_data_snif.ingress.nof_profiles_get(unit);
        /** Currently sniffing isn't done per core */
        data.flags = 0;
        /*
         * data.desc = "SNIF - SNIF profiles allocated IDs";
         */
        SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, BCM_CORE_ALL, DNX_ALGO_SNIF_RES_MNGR_INGRESS_PROFILES_STAT_SAMPLING,
                                            &data, NULL, NULL));
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_algo_mirror_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Init mirror resource manager
     */
    SHR_IF_ERR_EXIT(dnx_algo_mirror_res_mngr_init(unit));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_algo_mirror_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);

    SHR_FUNC_EXIT;
}
