/*
 * ! \file qos.c QOS procedures for DNX. Here add DESCRIPTION. 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_QOS
/*
 * Include files which are specifically for DNX. Final location.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
/*
 * }
 */
/*
 * Include files currently used for DNX. To be modified and moved to
 * final location.
 * {
 */
#include <soc/dnx/dbal/dbal.h>

/*
 * }
 */
/*
 * Include files.
 * {
 */
#include <shared/bslenum.h>
#include <bcm/types.h>
#include <bcm/qos.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
/*
 * }
 */


#define DNX_QOS_SUPPORTED_FLAGS_VALIDATE(flags)     if(flags & BCM_QOS_MAP_SUBPORT)                                    \
                                                    {                                                                  \
                                                        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal flag combination\n");    \
                                                    }

static shr_error_e
dnx_qos_map_create_verify(
    int unit,
    uint32 flags,
    int map_id)
{
    SHR_FUNC_INIT_VARS(unit);

    /** validate flags supported here with macro */

    DNX_QOS_SUPPORTED_FLAGS_VALIDATE(flags);

    /** validate map ID if needed */
    if (flags & BCM_QOS_MAP_WITH_ID)
    {
        if (map_id > 4096  )
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "illegal value for map_id\n");
        }
    }

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * ! \brief allocate resource pool for QOS 
 */
static shr_error_e
dnx_qos_resource_init(
    int unit)
{
    dnx_algo_res_create_data_t data;

    SHR_FUNC_INIT_VARS(unit);

    data.first_element = 0;

    data.nof_elements = 4096; 
    data.flags = 0;

    SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, BCM_CORE_ALL, "QOS", &data, NULL, NULL));

exit:
    SHR_FUNC_EXIT;
}

/*
 * ! \brief init for qos module. allocate resource pool \par DIRECT INPUT \param [in] unit - Relevant unit. \par
 * INDIRECT INPUT: * None \par DIRECT OUTPUT: \retval Negative in case of an error. See shr_error_e, for example: MAC
 * table is full \retval Zero in case of NO ERROR \par INDIRECT OUTPUT * None 
 */
int
bcm_dnx_qos_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_qos_resource_init(unit));

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * ! \brief create the map ID of the local device \par DIRECT INPUT \param [in] unit - Relevant unit.  \param [in]
 * flags - related flags BCM_QOS_MAP_XXX.  \param [in] map_id - ID of the MAP.  Pointer to module ID of the local
 * device to be filled by the function. \par INDIRECT INPUT: * None \par DIRECT OUTPUT: \retval Negative in case of an
 * error. See shr_error_e, for example: MAC table is full \retval Zero in case of NO ERROR \par INDIRECT OUTPUT * None 
 */
int
bcm_dnx_qos_map_create(
    int unit,
    uint32 flags,
    int *map_id)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_qos_map_create_verify(unit, flags, (*map_id)));

    

    /** when BCM_QOS_MAP_INGRESS it means allocating profile for PHB */
    if (flags & BCM_QOS_MAP_INGRESS)
    {
        if (flags & BCM_QOS_MAP_L2_VLAN_PCP)
        {
            SHR_IF_ERR_EXIT(dnx_algo_res_allocate(unit, BCM_CORE_ALL, "QOS", 0, NULL, map_id));
        }
    }

    /** when BCM_QOS_MAP_EGRESS it means allocating profile for [re]marking */
    if (flags & BCM_QOS_MAP_EGRESS)
    {
        *map_id = 0;
    }

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * ! \brief create the map ID of the local device \par DIRECT INPUT \param [in] unit - Relevant unit.  \param [in]
 * map_id - ID of the MAP.  Pointer to module ID of the local device to be filled by the function. \par INDIRECT
 * INPUT: * None \par DIRECT OUTPUT: \retval Negative in case of an error. See shr_error_e, for example: MAC table is
 * full \retval Zero in case of NO ERROR \par INDIRECT OUTPUT * None 
 */
int
bcm_dnx_qos_map_destroy(
    int unit,
    int map_id)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * bcm_qos_port_map_set 
 */
int
bcm_dnx_qos_port_map_set(
    int unit,
    bcm_gport_t port,
    int ing_map,
    int egr_map)
{
    return -1;
}

/*
 * bcm_qos_map_add 
 */
int
bcm_dnx_qos_map_add(
    int unit,
    uint32 flags,
    bcm_qos_map_t * map,
    int map_id)
{
    return -1;
}
