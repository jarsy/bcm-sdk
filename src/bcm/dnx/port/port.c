/*
 ** \file port.c $Id$ PORT procedures for DNX. 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_PORT
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
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <bcm_int/dnx/port/port_pp.h>

/*
 * }
 */


/**
 * \brief - Verify Port, pclass and class_id parameters for 
 * BCM-API: bcm_dnx_port_class_get()
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *  Relevant unit.
 *   \param [in] port -
 *  Port - physical port
 *   \param [in] pclass -
 *  Typedef enum value for specific port configurations. See .h
 *   \param [in] class_id -
 *   Class id.
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
     None
 * \remark
     None
 * \see
     None
 */
static shr_error_e
dnx_port_class_get_verify(
    int unit,
    bcm_port_t port,
    bcm_port_class_t pclass,
    uint32 class_id)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Get port classification ID to aggregate a
 * group of ports for further processing such as Vlan
 * translation and field processing.
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   Relevant unit.
 *   \param [in] port -
 *   Port - physical port
 *   \param [in] pclass -
 *   Typedef enum value for specific port configurations. See .h
 *   file.
 *   \param [in] class_id -
 *   Class id.
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
 *   None
 * \par INDIRECT OUTPUT
     Gets the CLASS_ID value from the HW table
 * \remark
 *   In case of pclass=4, it gets the VLAN_MEMBERSHIP_IF value
 *   from the Ingress Port Table. Will be changed in the future
 *   with a new pClass Type.
 * \see
     None
 */
int
bcm_dnx_port_class_get(
    int unit,
    bcm_port_t port,
    bcm_port_class_t pclass,
    uint32 * class_id)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_port_class_get_verify(unit, port, pclass, *class_id));

    /*
     * Check pclass, for each class call appropriate function
     */
    /*
     * Note: Do not add code inside the case!!! Only through separate function!
     */
    switch (pclass)
    {
        case bcmPortClassId:
        {
            /*
             * Get Vlan Domain
             */
            SHR_IF_ERR_EXIT(dnx_port_pp_vlan_domain_get(unit, port, class_id));
            break;
        }
        case bcmPortClassIngress:
        {
            /*
             * So far we use the bcmPortClassIngress (#4) as a "dummy" enum value.
             * We get here the VLAN membership if value
             */
            SHR_IF_ERR_EXIT(dnx_port_pp_vlan_membership_if_get(unit, port, class_id));
            break;
        }

        default:
        {
            SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
            SHR_EXIT();
            break;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Verify Port, pclass and class_id parameters for
 * BCM-API: bcm_dnx_port_class_set()
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *  Relevant unit.
 *   \param [in] port -
 *  Port - physical port
 *   \param [in] pclass -
 *  Typedef enum value for specific port configurations. See .h
 *   \param [in] class_id -
 *   Class id.
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
     None
 * \remark
     None
 * \see
     None
 */
static shr_error_e
dnx_port_class_set_verify(
    int unit,
    bcm_port_t port,
    bcm_port_class_t pclass,
    uint32 class_id)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Set port classification ID to aggregate a
 * group of ports for further processing such as Vlan
 * translation and field processing.
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   Relevant unit.
 *   \param [in] port -
 *   Port - physical port
 *   \param [in] pclass -
 *   Typedef enum value for specific port configurations. See .h
 *   file
 *   \param [in] class_id -
 *   Class id.
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
 *   None
 * \par INDIRECT OUTPUT
     Set the CLASS ID value to the HW table
 * \remark
 *   In case of pclass=4, it sets the VLAN_MEMBERSHIP_IF value
 *   to the Ingress/Egress Port Table.s symetrically.
 * \see
     None
 */
int
bcm_dnx_port_class_set(
    int unit,
    bcm_port_t port,
    bcm_port_class_t pclass,
    uint32 class_id)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_port_class_set_verify(unit, port, pclass, class_id));

    /*
     * Check pclass, for each class call appropriate function
     */
    /*
     * Note: Do not add code inside the case!!! Only through separate function!
     */
    switch (pclass)
    {
        case bcmPortClassId:
        {
            /*
             * Set Vlan Domain
             */
            SHR_IF_ERR_EXIT(dnx_port_pp_vlan_domain_set(unit, port, class_id));
            break;
        }
        case bcmPortClassIngress:
        {
            /*
             * So far we use the bcmPortClassIngress (#4) as a "dummy" enum value.
             * We set here the VLAN membership if value
             */
            SHR_IF_ERR_EXIT(dnx_port_pp_vlan_membership_if_set(unit, port, class_id));
            break;
        }

        default:
        {
            SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
            SHR_EXIT();
            break;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Get the GPORT ID for the specified physical port
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] port -
 *     port - physical port
 *   \param [in] gport - Output param. GPORT ID of the given physical port.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: MAC table is full
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Write to HW MAC table.
 */
int
bcm_dnx_port_gport_get(
    int unit,
    bcm_port_t port,
    bcm_gport_t * gport)
{
    SHR_FUNC_INIT_VARS(unit);

    *gport = port | 0x8000000;

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
bcm_dnx_port_match_add(
    int unit,
    bcm_gport_t port,
    bcm_port_match_info_t * match_info)
{

    SHR_FUNC_INIT_VARS(unit);

    
    /*
     * Different meaning in ingress and egress
     */
    if (_SHR_IS_FLAG_SET(match_info->flags, BCM_PORT_MATCH_EGRESS_ONLY))
    {
        /*
         * Deside what to do according to match criteria
         */
        switch (match_info->match)
        {
            case BCM_PORT_MATCH_PORT:
            {
                /*
                 * MATCH_PORT criteria
                 */
                SHR_IF_ERR_EXIT(dnx_port_pp_egress_match_port_add(unit, port, match_info));
                break;
            }

            default:
            {
                SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
                SHR_EXIT();
                break;
            }
        }
    }
    else
    {
        SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL);
        SHR_EXIT();
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
bcm_dnx_port_match_delete(
    int unit,
    bcm_gport_t port,
    bcm_port_match_info_t * match_info)
{
    return -1;
}

shr_error_e
bcm_dnx_port_match_delete_all(
    int unit,
    bcm_gport_t port,
    bcm_port_match_info_t * match_info)
{
    return -1;
}

shr_error_e
bcm_dnx_port_match_multi_get(
    int unit,
    bcm_gport_t port,
    bcm_port_match_info_t * match_info)
{
    return -1;
}

shr_error_e
bcm_dnx_port_match_replace(
    int unit,
    bcm_gport_t port,
    bcm_port_match_info_t * match_info)
{
    return -1;
}

shr_error_e
bcm_dnx_port_match_set(
    int unit,
    bcm_gport_t port,
    bcm_port_match_info_t * match_info)
{
    return -1;
}
