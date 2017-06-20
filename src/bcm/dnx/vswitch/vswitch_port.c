/** \file vswitch_port.c
 *  
 *  Vswitch association with gport procedures for DNX. 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_VLAN
/*
 * Include files.
 * {
 */
#include <soc/dnx/dbal/dbal.h>
#include <bcm/types.h>
#include <bcm/vswitch.h>
#include <shared/shrextend/shrextend_debug.h>
#include <soc/dnx/dnx_data/dnx_data_l2.h>
#include <shared/util.h>
#include <bcm/error.h>

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
 * \brief - Verify function for bcm_dnx_vswitch_port_add 
 */
static shr_error_e
dnx_vswitch_port_add_verify(
    int unit,
    bcm_vlan_t  vsi,
    bcm_gport_t port)
{
    


    


    return 0;
}


/**
 * \brief
 * This API associated a gport with a VSI by updating the vsi 
 * attribute of an In-LIF. 
 * 
 * \par DIRECT INPUT
 *   \param [in] unit -
 *     Relevant unit.
 *   \param [in] vsi -
 *     The VSI that the In-LIF is associated with
 *   \param [in] port -
 *     A gport that represent an In-LIF that as is assciated
 *     with a VSI
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for
 *           example: invalid vsi.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * HW local In-LIF is modified with the VSI value and a few
 *     more initial values.
 */
shr_error_e bcm_dnx_vswitch_port_add(
    int unit, 
    bcm_vlan_t      vsi,
    bcm_gport_t     port)
{
    uint32 entry_handle_id;
    bcm_gport_t local_in_lif = port;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_vswitch_port_add_verify(unit, vsi, port));

    
    if (BCM_GPORT_IS_VLAN_PORT(port))
    {
        local_in_lif = BCM_GPORT_SUB_TYPE_LIF_VAL_GET(BCM_GPORT_VLAN_PORT_ID_GET(port));
    }

    



    
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IN_AC_INFO_DB, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_LIF, INST_SINGLE, local_in_lif);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, DBAL_RESULT_TYPE_IN_AC_INFO_DB_IN_LIF_FORMAT_AC_MP);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_FORWARDING_DOMAIN, INST_SINGLE, vsi);

    
    
    
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));


exit:
    SHR_FUNC_EXIT; 
}


shr_error_e bcm_dnx_vswitch_port_get(
    int unit,
    bcm_gport_t     port,
    bcm_vlan_t *    vsi)
{
    return -1;
}


shr_error_e bcm_dnx_vswitch_port_delete(
    int unit,
    bcm_vlan_t      vsi,
    bcm_gport_t     port)
{
    return -1;
}


shr_error_e bcm_dnx_vswitch_port_delete_all(
    int unit,
    bcm_vlan_t      vsi)
{
    return -1;
}


shr_error_e bcm_dnx_vswitch_port_traverse(
    int unit, 
    bcm_vlan_t      vsi, 
    bcm_vswitch_port_traverse_cb cb, 
    void            *user_data)
{
    return -1;
}

