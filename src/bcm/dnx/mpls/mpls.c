/** \file mpls.c General MPLS functionality for DNX. 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_MPLS
/*
 * Include files.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
#include <soc/dnx/dbal/dbal.h>
#include <bcm/types.h>
#include <bcm/mpls.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
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

/*
 * Verify functions
 * {
 */

 
static shr_error_e
dnx_mpls_tunnel_switch_ilm_add_verify(
    int unit,
    bcm_mpls_tunnel_switch_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    if (BCM_L3_ITF_TYPE_IS_LIF(info->egress_if))
    {
        /** Outlif  */
        if (!BCM_GPORT_IS_FORWARD_PORT(info->port))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM,
                         "info->port (0x%08X) is not valid since it is not a forwarding port. See BCM_GPORT_FORWARD_PORT_GET\r\n",
                         info->port);
        }
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM,
                     "info->egress_if (0x%08X) is invalid since L3 interface type is not LIF. See BCM_L3_ITF_TYPE_IS_LIF\r\n",
                     info->egress_if);
    }

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief  
*  Used by bcm_dnx_mpls_tunnel_switch_create to verify info struct.
* \par DIRECT INPUT: 
*   \param [in] unit  -  Relevant unit.
*   \param [in] info  -  A pointer to a struct containing relevant information for ilm configuration.
* \par INDIRECT INPUT: 
*   * \b *info \n
*     See 'info' in DIRECT INPUT above
* \par DIRECT OUTPUT: 
*   shr_error_e - Non-zero in case of an error.
* \par INDIRECT OUTPUT: 
*   * None
* \remark 
*   * None
* \see
*   * \ref bcm_dnx_mpls_tunnel_switch_create
*/
static shr_error_e
dnx_mpls_tunnel_switch_create_verify(
    int unit,
    bcm_mpls_tunnel_switch_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(info, _SHR_E_PARAM, "info");

    if (info->flags & (BCM_MPLS_SWITCH_LOOKUP_SECOND_LABEL | BCM_MPLS_SWITCH_COUNTED | BCM_MPLS_SWITCH_INT_PRI_SET
                       | BCM_MPLS_SWITCH_INT_PRI_MAP | BCM_MPLS_SWITCH_COLOR_MAP | BCM_MPLS_SWITCH_OUTER_EXP
                       | BCM_MPLS_SWITCH_OUTER_TTL | BCM_MPLS_SWITCH_INNER_EXP | BCM_MPLS_SWITCH_INNER_TTL |
                       BCM_MPLS_SWITCH_TTL_DECREMENT | BCM_MPLS_SWITCH_LOOKUP_L3_INGRESS_INTF | BCM_MPLS_SWITCH_DROP |
                       BCM_MPLS_SWITCH_P2MP | BCM_MPLS_SWITCH_SKIP_ETHERNET | BCM_MPLS_SWITCH_EXPECT_BOS |
                       BCM_MPLS_SWITCH_TRAP_TTL_0 | BCM_MPLS_SWITCH_TRAP_TTL_1 | BCM_MPLS_SWITCH_LOOKUP_INT_PRI |
                       BCM_MPLS_SWITCH_FRR | BCM_MPLS_SWITCH_ENCAP_SET | BCM_MPLS_SWITCH_NEXT_HEADER_L2 |
                       BCM_MPLS_SWITCH_NEXT_HEADER_IPV4 | BCM_MPLS_SWITCH_NEXT_HEADER_IPV6 |
                       BCM_MPLS_SWITCH_DIRECT_ATTACHED | BCM_MPLS_SWITCH_ENTROPY_ENABLE | BCM_MPLS_SWITCH_REPLACE |
                       BCM_MPLS_SWITCH_WITH_ID | BCM_MPLS_SWITCH_WIDE | BCM_MPLS_SWITCH_LOOKUP_NONE |
                       BCM_MPLS_SWITCH_EVPN_IML | BCM_MPLS_SWITCH_INGRESS_ECN_MAP |
                       BCM_MPLS_SWITCH_TUNNEL_TERM_ECN_MAP))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "one of the used flags is not supported - flags = 0x%08X", info->flags);
    }

    if ((info->action < BCM_MPLS_SWITCH_ACTION_SWAP) || (info->action > BCM_MPLS_SWITCH_ACTION_INVALID))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "action not valid = %d", info->action);
    }

    if (info->action != BCM_MPLS_SWITCH_ACTION_POP)
    {
        /** in case SWAP/NOP were requested verify that TTL_DECREMENT is set */
        if (((info->action == BCM_MPLS_SWITCH_ACTION_SWAP) || (info->action == BCM_MPLS_SWITCH_ACTION_NOP)) &&
            (!(info->flags & BCM_MPLS_SWITCH_TTL_DECREMENT)))
        {
            /** TTL always decemented (even if inherented/copied */
            SHR_ERR_EXIT(_SHR_E_PARAM,
                         "BCM_MPLS_SWITCH_TTL_DECREMENT must be set in case of ILM SWAP or action NOP. action = %d, flags = 0x%08X",
                         info->action, info->flags);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

 
static shr_error_e
dnx_mpls_tunnel_switch_term_add_verify(
    int unit,
    bcm_mpls_tunnel_switch_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * End of Verify functions
 * }
 */

/*
 * Inner functions
 * {
 */

/**
 * \brief
 * Create MPLS tunnels in the ingress for either termination (pop), forwarding (swap) or push.
 * \par DIRECT INPUT
 *    \param [in] unit -Relevant unit.
 *     \param [in] info -A pointer to a struct containing relevant information for the ILM entry.
 * \par INDIRECT INPUT:
 *   * \b *info \n
 *     See 'info' in DIRECT INPUT above
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See \ref shr_error_e, for example: MPLS label is out of range.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Write to MPLS-ILM DB - DBAL_TABLE_MPLS_FWD table.
 * \remark
 * MPLS-ILM is the forwarding table for MPLS application. It is used primarily for swap action \n
 * but supports also pointing to egress MPLS objects.
 */
static shr_error_e
dnx_mpls_tunnel_switch_ilm_add(
    int unit,
    bcm_mpls_tunnel_switch_t * info)
{
    uint32 entry_handle_id;
    uint32 forward_encap_id;
    dnx_algo_gpm_forward_info_t forward_info;

    SHR_FUNC_INIT_VARS(unit);

    /** Verification. */
    SHR_INVOKE_VERIFY_DNX(dnx_mpls_tunnel_switch_ilm_add_verify(unit, info));

    /** retrieve needed params from info */    
    BCM_FORWARD_ENCAP_ID_VAL_SET(forward_encap_id, BCM_FORWARD_ENCAP_ID_TYPE_OUTLIF, BCM_FORWARD_ENCAP_ID_OUTLIF_USAGE_MPLS_PORT, BCM_L3_ITF_VAL_GET(info->egress_if));
    SHR_IF_ERR_EXIT(algo_gpm_gport_and_encap_to_forward_information(unit, info->port, forward_encap_id, &forward_info));

    /** write to table */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_MPLS_FWD, &entry_handle_id));
    
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_T_ID, INST_SINGLE, 0);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_MPLS_LABEL, INST_SINGLE, info->label);
    
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                 DBAL_RESULT_TYPE_MPLS_FWD_FWD_DEST_OUTLIF);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DESTINATION, INST_SINGLE, forward_info.destination);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_GLOB_OUT_LIF, INST_SINGLE, forward_info.outlif);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}


static shr_error_e
dnx_mpls_tunnel_switch_term_add(
    int unit,
    bcm_mpls_tunnel_switch_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify the pointer 'info' is not NULL and exit with error if it is.
     */
    SHR_INVOKE_VERIFY_DNX(dnx_mpls_tunnel_switch_term_add_verify(unit, info));

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * End of Inner functions
 * }
 */

/*
 * APIs
 * {
 */


int
bcm_dnx_mpls_tunnel_switch_create(
    int unit,
    bcm_mpls_tunnel_switch_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verification of supported features.
     */
    SHR_INVOKE_VERIFY_DNX(dnx_mpls_tunnel_switch_create_verify(unit, info));

    /*
     * Execution
     */
    /** if action is not pop, then this is ILM action */
    if (info->action != BCM_MPLS_SWITCH_ACTION_POP)
    {
        SHR_IF_ERR_EXIT(dnx_mpls_tunnel_switch_ilm_add(unit, info));
    }
    else
    {
        /** else this is termination operation */
        SHR_IF_ERR_EXIT(dnx_mpls_tunnel_switch_term_add(unit, info));
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * End of APIs 
 * }
 */
