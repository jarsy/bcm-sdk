/*
 * ! \file algo_port.c Port Managment procedures for DNX.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_PORT

/*
 * Include files.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
#include <bcm/types.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <bcm_int/dnx/algo/algo_port.h>
#include <soc/dnx/dnx_data/dnx_data.h>
/*
 * }
 */

/*
 * See .h file
 */
shr_error_e
algo_port_pbmp_to_pp_pbmp(
    int unit,
    bcm_pbmp_t pbmp,
    bcm_pbmp_t * pbmp_pp_arr)
{
    int core_id;
    bcm_port_t port_i;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify the pointer 'pbmp_pp_arr' is not NULL and exit with error if it is.
     */
    SHR_NULL_CHECK(pbmp_pp_arr, _SHR_E_PARAM, "pbmp_pp_arr");

    /*
     * pbmp include internal pp ports over both of the cores. 
     * The below loop will go over each core and update the related HW pbmp, ubmp tables for the matched core 
     */
    for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
    {
        BCM_PBMP_CLEAR(pbmp_pp_arr[core_id]);
        /*
         * convert local port bmp(bitmap) to PP port bmp, required per core
         */
        BCM_PBMP_ITER(pbmp, port_i)
        {
            /*
             * Get Port + Core 
             */
            SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get
                            (unit, port_i, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE, &gport_info));
            /*
             * In case of match, add to pp-port bmp, otherwise continue (it is related to other core)
             */
            if ((gport_info.internal_core_id == core_id))
            {
                BCM_PBMP_PORT_ADD(pbmp_pp_arr[core_id], gport_info.internal_port_pp);
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
algo_port_pp_pbmp_to_local_pbmp(
    int unit,
    bcm_pbmp_t * pbmp_pp_arr,
    bcm_pbmp_t * pbmp)
{
    int core_id;
    bcm_port_t port_i;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify the pointer 'pbmp_pp_arr' is not NULL and exit with error if it is.
     */
    SHR_NULL_CHECK(pbmp_pp_arr, _SHR_E_PARAM, "pbmp_pp_arr");
    SHR_NULL_CHECK(pbmp, _SHR_E_PARAM, "pbmp");

    /*
     * The below loop will go over each core and update the related HW pbmp, ubmp tables for the matched core 
     */
    for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
    {
        
        BCM_PBMP_ITER(pbmp_pp_arr[core_id], port_i)
        {
            BCM_PBMP_PORT_ADD(*pbmp, port_i);
        }
    }

exit:
    SHR_FUNC_EXIT;
}
/*
 * See .h file
 */
shr_error_e
algo_port_pp_to_local_port(
    int unit,
    int core_id,
    bcm_port_t pp_port,
    bcm_gport_t * logical_port)
{

    SHR_FUNC_INIT_VARS(unit);
    SHR_NULL_CHECK(logical_port, _SHR_E_PARAM, "logical_port");
    /*
     * The function will map pp port to logical port.
     * The mapping is done based on the core id.
     */
    if (core_id == 0)
    {
        BCM_GPORT_LOCAL_SET(*logical_port, pp_port);
    }
    else
    {
        BCM_GPORT_LOCAL_SET(*logical_port, pp_port + 256);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
algo_local_port_to_pp_port(
    int unit,
    bcm_port_t logical_port,
    bcm_gport_t * pp_port,
    int *core_id)
{

    int core_id_internal;
    SHR_FUNC_INIT_VARS(unit);
    SHR_NULL_CHECK(pp_port, _SHR_E_PARAM, "pp_port");
    SHR_NULL_CHECK(core_id, _SHR_E_PARAM, "core_id");
    /*
     * The function will map logical port to pp port.
     * First we check if the logical port is local gport
     */
    if(BCM_GPORT_IS_LOCAL(logical_port))
    {
         *pp_port = BCM_GPORT_LOCAL_GET(logical_port);
    }
    else
    {
        if (logical_port < 256)
        {
            *pp_port = logical_port;
        }
        else
        {
            *pp_port = logical_port - 256;
        }
    }

   /*
    * The core_id mapping is based on the port.
    */
    if (*pp_port < 256)
    {
        core_id_internal = 0;
        *core_id = core_id_internal;
    }
    else
    {
        core_id_internal = 1;
        *core_id = core_id_internal;
    }

exit:
    SHR_FUNC_EXIT;
}
