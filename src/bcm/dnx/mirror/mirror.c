/** \file mirror.c
 * $Id$
 *
 * General MIRROR functionality for DNX. \n
 * Note - Due to backward compatbility, this module is named "mirror" although mirror is \n 
 * just one application of snif. In General snif supports the following applications: \n
 * snoop, mirror, statistical sampling. 
 * Dedicated set of MIRROR APIs are distributed between mirror_*.c files: \n 
 * mirror.c, mirror_command.c 
 * 
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_MIRROR

/*
 * Include files.
 * {
 */
#include <shared/utilex/utilex_bitstream.h>
#include <shared/shrextend/shrextend_error.h>
#include <soc/dnx/dbal/dbal.h>
#include <soc/dnx/dnx_data/dnx_data_device.h>
#include <soc/dnx/dnx_data/dnx_data_snif.h>
#include <soc/dnx/legacy/cosq.h>
#include <bcm/mirror.h>
#include <bcm/types.h>
#include <bcm_int/dnx/mirror/mirror.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <bcm_int/dnx/algo/algo_port.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <bcm_int/dnx/algo/mirror/algo_mirror.h>
#include <bcm_int/dnx/legacy/gport_mgmt.h>
#include "mirror_profile.h"

/*
 * }
 */

/*
 * MACROs
 * {
 */

/*
 * }
 */

/**
 * \brief - Verify mirror destination profile attributes for BCM-API: bcm_dnx_mirror_destination_create()
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest - Mirror destination attributes
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_mirror_destination_create_verify(
    int unit,
    bcm_mirror_destination_t * mirror_dest)
{
    int action_profile_id;
    uint8 is_allocated;

    SHR_FUNC_INIT_VARS(unit);

    action_profile_id = 0;
    /*
     * verify valid flags  
     */
    if (mirror_dest->flags & ~(BCM_MIRROR_DEST_WITH_ID | BCM_MIRROR_DEST_REPLACE |
                               BCM_MIRROR_DEST_UPDATE_POLICER | BCM_MIRROR_DEST_DEST_MULTICAST |
                               BCM_MIRROR_DEST_UPDATE_COUNTER | BCM_MIRROR_DEST_UPDATE_COUNTER_1 |
                               BCM_MIRROR_DEST_UPDATE_COUNTER_2 | BCM_MIRROR_DEST_TUNNEL_IP_GRE |
                               BCM_MIRROR_DEST_TUNNEL_WITH_ENCAP_ID | BCM_MIRROR_DEST_IS_SNOOP |
                               BCM_MIRROR_DEST_TUNNEL_WITH_SPAN_ID | BCM_MIRROR_DEST_TUNNEL_RSPAN |
                               BCM_MIRROR_DEST_TUNNEL_L2 | BCM_MIRROR_DEST_EGRESS_ADD_ORIG_SYSTEM_HEADER |
                               BCM_MIRROR_DEST_EGRESS_TRAP_WITH_SYSTEM_HEADER))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "An unsupported bcm mirror destination flag was specified : 0x%lx",
                     (unsigned long) mirror_dest->flags);
    }

    /*
     * 1. Verify that BCM_MIRROR_DEST_REPLACE flag (modify existing mirror attributes) is given along with 
     *    BCM_MIRROR_DEST_WITH_ID(use specific ID for mirror command) flag. 
     * 2. Verify that the given profile is already allocated 
     */
    if (mirror_dest->flags & BCM_MIRROR_DEST_REPLACE)
    {
        if (!(mirror_dest->flags & BCM_MIRROR_DEST_WITH_ID))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "BCM_MIRROR_DEST_REPLACE flag has to be given along with"
                         "BCM_MIRROR_DEST_WITH_ID flag");
        }

        /** check whether the given snif profile is allocated, if not return an error */
        SHR_IF_ERR_EXIT(dnx_algo_mirror_profile_is_allocated(unit, mirror_dest->mirror_dest_id, &is_allocated));
        if (is_allocated == FALSE)
        {
            action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest->mirror_dest_id);

            SHR_ERR_EXIT(_SHR_E_PARAM, "BCM_MIRROR_DEST_REPLACE flag has to be used on existing profiles"
                         "Specified destination profile id ID: %d has not been allocated", action_profile_id);
        }
    }

    /*
     * verify profile id is in range
     */
    if (mirror_dest->flags & BCM_MIRROR_DEST_WITH_ID)
    {
        action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest->mirror_dest_id);

        if (action_profile_id > DNX_ALGO_MIRROR_INGRESS_PROFILE_MAX(unit))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Specified destination ID: %d is out of range", action_profile_id);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Verify mirror destination profile attributes for BCM-API: bcm_dnx_mirror_destination_destroy()
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest_id - gport of mirror profile
 *   
 * \par INDIRECT INPUT:
 *   * DNX data - SNIF module
 *   * SNIF resouce manager
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_mirror_destination_destroy_verify(
    int unit,
    bcm_gport_t mirror_dest_id)
{
    uint8 is_allocated;
    int action_profile_id;

    SHR_FUNC_INIT_VARS(unit);

    /** Verify valid gport type */
    if (BCM_GPORT_IS_MIRROR(mirror_dest_id) == FALSE)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Specified mirror destination has incorrect gport type: 0x%x", mirror_dest_id);
    }

    /** check whether the given snif profile is allocated, if not return an error */
    SHR_IF_ERR_EXIT(dnx_algo_mirror_profile_is_allocated(unit, mirror_dest_id, &is_allocated));
    if (is_allocated == FALSE)

    {
        action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest_id);
        SHR_ERR_EXIT(_SHR_E_PARAM, "The given profile: %d wasn't allocated", action_profile_id);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Verify mirror destination profile attributes for BCM-API: bcm_dnx_mirror_destination_get()
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest_id - gport of mirror profile
 *   \param [in] mirror_dest - Mirror destination attributes
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_mirror_destination_get_verify(
    int unit,
    bcm_gport_t mirror_dest_id,
    bcm_mirror_destination_t * mirror_dest)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(mirror_dest, _SHR_E_PARAM, "mirror_dest");

    /*
     * Same verify function as for bcm_dnx_mirror_destination_destroy() API, 
     * since both the get() and destroy() APIs expect the same input parameters - a valid mirror profile. 
     */
    SHR_IF_ERR_EXIT(dnx_mirror_destination_destroy_verify(unit, mirror_dest_id));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Allocate mirror profile and set its attributes
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID 
 *   \param [in] mirror_dest - Mirror profile attributes: \n
 *   * mirror_dest.mirror_dest_id - gport representing the command id. gport: BCM_GPORT_MIRROR \n
 *   * mirror_dest.gport - gport representing the destination of the command \n
 *   * mirror_dest.flags - the following flags are supported: \n
 *     * BCM_MIRROR_DEST_WITH_ID - allocate certain snif profile ID given in mirror_dest.gport \n
 *     * BCM_MIRROR_DEST_REPLACE - update already allocated profile \n
 *     * BCM_MIRROR_DEST_UPDATE_COUNTER - update counting attributes
 *     * BCM_MIRROR_DEST_IS_SNOOP - snif command is snoop (default is mirror)
 *   * mirror_dest.sample_rate_dividend - dividend of the probabiity to
 *     perfrom the snif command, the probability is calculated as following: \n
 *     mirror_dest.sample_rate_dividend/mirror_dest.sample_rate_divisor
 *   * mirror_dest.sample_rate_divisor - divisor of the probabiity to perfrom the snif \n
 *     command, the probability is calculated as following: \n
 *     mirror_dest.sample_rate_dividend/mirror_dest.sample_rate_divisor \n
 *   * mirror_dest.packet_copy_size - crop size in bytes (how many bytes to snif from the original packet)
 *   * mirror_dest.packet_control_updates - packet control parameters:  \n 
 *     *  valid - Used to specify which fields to use for header changes. Possible values will be named
 *                BCM_MIRROR_PKT_HEADER_UPDATE_*
 *     * color - drop precedence (DP)
 *     * prio - traiffc class (TC)
 *     * ecn_value - ECN capable and congestion encoding
 *     * trunk_hash_result - LAG load balancing key
 *     * in_port - ingress port
 *     * vsq - statistics VSQ pointer (VSQ-D)
 *          
 * \par INDIRECT INPUT:
 *   * Allocation manger of SNIF_INGRESS_MIRROR_PROFILES
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *  * This API creates and configures the SNIF profile. 3 SNIF applications are supported: \n
 *    Mirror (Ingress/Egress), Snoop(Ingress/Egress), Statistical sample(Ingress).
 *  * Upon creation of a SNIF rule, see \ref bcm_dnx_mirror_port_vlan_destination_add API for
 *    example, this profile should be mapped to the SNIF rule.
 *    
 * \see
 *   * None
 */
int
bcm_dnx_mirror_destination_create(
    int unit,
    bcm_mirror_destination_t * mirror_dest)
{
    SHR_FUNC_INIT_VARS(unit);

    /** Verify API attributes and ID */
    SHR_INVOKE_VERIFY_DNX(dnx_mirror_destination_create_verify(unit, mirror_dest));

    /** Allocate(if required) profile and configure its attributes in HW */
    SHR_IF_ERR_EXIT(dnx_mirror_profile_create(unit, mirror_dest));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Destroy(deallocate) an allocated mirror profile
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest_id - Gport of profile ID to be destroyed
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * Sniff allocation manager
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_destination_destroy(
    int unit,
    bcm_gport_t mirror_dest_id)
{
    SHR_FUNC_INIT_VARS(unit);

    /** Verify parameters */
    SHR_INVOKE_VERIFY_DNX(dnx_mirror_destination_destroy_verify(unit, mirror_dest_id));

    /** Deallocate profile and roll HW back to defaults */
    SHR_IF_ERR_EXIT(dnx_mirror_profile_destroy(unit, mirror_dest_id));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Retrieve mirror profile attributes.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest_id - Gport representing the mirror profile. 
 *   \param [in] mirror_dest - Mirror profile attributes. For more info see \ref bcm_dnx_mirror_destination_create.
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * mirror_dest - Pointer to mirror profile attributes after read from HW.
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_destination_get(
    int unit,
    bcm_gport_t mirror_dest_id,
    bcm_mirror_destination_t * mirror_dest)
{
    SHR_FUNC_INIT_VARS(unit);

    /** Verify parameters */
    SHR_INVOKE_VERIFY_DNX(dnx_mirror_destination_get_verify(unit, mirror_dest_id, mirror_dest));

    /** Retrieve mirror attributes */
    SHR_IF_ERR_EXIT(dnx_mirror_profile_get(unit, mirror_dest_id, mirror_dest));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] cb - 
 *   \param [in] user_data - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_destination_traverse(
    int unit,
    bcm_mirror_destination_traverse_cb cb,
    void *user_data)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] flags - 
 *   \param [in] mirror_dest - 
 *   \param [in] options - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_destination_add(
    int unit,
    bcm_port_t port,
    uint32 flags,
    bcm_gport_t mirror_dest,
    bcm_mirror_options_t options)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] flags - 
 *   \param [in] mirror_dest_size - 
 *   \param [in] mirror_dest - 
 *   \param [in] mirror_dest_count - 
 *   \param [in] options - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_destination_get(
    int unit,
    bcm_port_t port,
    uint32 flags,
    int mirror_dest_size,
    bcm_gport_t * mirror_dest,
    int *mirror_dest_count,
    bcm_mirror_options_t * options)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] vlan - 
 *   \param [in] flags - 
 *   \param [in] destid - 
 *   \param [in] options - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_vlan_destination_add(
    int unit,
    bcm_port_t port,
    bcm_vlan_t vlan,
    uint32 flags,
    bcm_gport_t destid,
    bcm_mirror_options_t options)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] vlan - 
 *   \param [in] flags - 
 *   \param [in] mirror_dest_size - 
 *   \param [in] destid - 
 *   \param [in] destcount - 
 *   \param [in] options - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_vlan_destination_get(
    int unit,
    bcm_port_t port,
    bcm_vlan_t vlan,
    uint32 flags,
    uint32 mirror_dest_size,
    bcm_gport_t * destid,
    uint32 * destcount,
    bcm_mirror_options_t * options)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] flags - 
 *   \param [in] mirror_dest_id - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_dest_delete(
    int unit,
    bcm_port_t port,
    uint32 flags,
    bcm_gport_t mirror_dest_id)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] flags - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_dest_delete_all(
    int unit,
    bcm_port_t port,
    uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] vlan - 
 *   \param [in] flags - 
 *   \param [in] destid - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_vlan_dest_delete(
    int unit,
    bcm_port_t port,
    bcm_vlan_t vlan,
    uint32 flags,
    bcm_gport_t destid)
{
    return BCM_E_UNAVAIL;
}

/**
 * \brief - 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - 
 *   \param [in] port - 
 *   \param [in] vlan - 
 *   \param [in] flags - 
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
bcm_dnx_mirror_port_vlan_dest_delete_all(
    int unit,
    bcm_port_t port,
    bcm_vlan_t vlan,
    uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * See .h file
 */
shr_error_e
dnx_mirror_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_mirror_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);

    SHR_FUNC_EXIT;
}
