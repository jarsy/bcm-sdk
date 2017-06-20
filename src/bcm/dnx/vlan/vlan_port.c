/** \file vlan_port.c
 *  
 *  VLAN port procedures for DNX. Allows creation of
 *  VLAN-Port(AC) entities.
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
#include <bcm/vlan.h>
#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/port/port_pp.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <soc/dnx/dnx_data/dnx_data_l2.h>
#include <shared/util.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>

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
 * \brief - Verify function for dnx_vlan_port_create
 */
static shr_error_e
dnx_vlan_port_create_verify(
    int unit,
    bcm_vlan_port_t * vlan_port)
{

    SHR_FUNC_INIT_VARS(unit);

    /*
     * BCM_VLAN_PORT_VLAN_TRANSLATION flag is for egress only. 
     */
    if (!_SHR_IS_FLAG_SET(vlan_port->flags, BCM_VLAN_PORT_EGRESS_ONLY)
        && (_SHR_IS_FLAG_SET(vlan_port->flags, BCM_VLAN_PORT_VLAN_TRANSLATION)))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM,
                     "BCM_VLAN_PORT_VLAN_TRANSLATION flag is egress only configuration."
                     "Must set BCM_VLAN_PORT_EGRESS_ONLY flag.\r\n");
    }
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Configure VLAN PORT default ingress configuration 
 *          (ac inlif) per port 
 *  
 *          Called by bcm_dnx_vlan_port_create
 * parameters: bcm_vlan_port_t (in) 
 *             bcm_gport_t (in)
 */
static shr_error_e
bcm_dnx_vlan_port_create_ingress_match_port_default(
    int unit,
    bcm_vlan_port_t * vlan_port,
    bcm_gport_t local_in_lif)
{
    uint32 entry_handle_id;
    dnx_algo_gpm_gport_phy_info_t gport_info;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get Port + Core 
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get
                    (unit, vlan_port->port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE, &gport_info));

    /*
     * Ingress default action
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DEFAULT_LIF, INST_SINGLE, local_in_lif);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Configure PORT x VLAN x VLAN in ingress configuration. 
 *          ISEM lookup, result is ac inlif
 *  
 *          Called by bcm_dnx_vlan_port_create
 * parameters: bcm_vlan_port_t (in) 
 *             bcm_gport_t (in)
 */
static shr_error_e
bcm_dnx_vlan_port_create_ingress_match_port_vlan_vlan(
    int unit,
    bcm_vlan_port_t * vlan_port,
    bcm_gport_t local_in_lif)
{
    uint32 vlan_domain = 0;
    uint32 entry_handle_id;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get Vlan Domain from Port 
     */
    SHR_IF_ERR_EXIT(dnx_port_pp_vlan_domain_get(unit, vlan_port->port, &vlan_domain));
    /*
     * Ingress - Add AC lookup entry
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IN_AC_S_C_VLAN_DB, &entry_handle_id));
    
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_PORT, INST_SINGLE, vlan_domain);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_S_VLAN, INST_SINGLE, vlan_port->match_vlan);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_C_VLAN, INST_SINGLE, vlan_port->match_inner_vlan);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_LIF, INST_SINGLE, local_in_lif);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Configure PORT x VLAN in ingress configuration. 
 *          ISEM lookup, result is ac inlif
 *  
 *          Called by bcm_dnx_vlan_port_create
 * parameters: bcm_vlan_port_t (in) 
 *             bcm_gport_t (in)
 */
static shr_error_e
bcm_dnx_vlan_port_create_ingress_match_port_vlan(
    int unit,
    bcm_vlan_port_t * vlan_port,
    bcm_gport_t local_in_lif)
{
    uint32 vlan_domain = 0;
    uint32 entry_handle_id;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get Vlan Domain from Port 
     */
    SHR_IF_ERR_EXIT(dnx_port_pp_vlan_domain_get(unit, vlan_port->port, &vlan_domain));
    /*
     * Ingress - Add AC lookup entry
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IN_AC_S_VLAN_DB, &entry_handle_id));
    
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_PORT, INST_SINGLE, vlan_domain);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_S_VLAN, INST_SINGLE, vlan_port->match_vlan);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_LIF, INST_SINGLE, local_in_lif);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Configure VLAN PORT default egress configuration 
 *          (egress vlan encapsulation) per port by configuring
 *          tables that map the following: PORT ->
 *          ESEM_ACCESS_CMD ESEM_ACCESS_CMD -> AC_Profile
 *          AC_Profile-> AC info
 *  
 *          Called by bcm_dnx_vlan_port_create
 * parameters: bcm_vlan_port_t (in)
 */
static shr_error_e
bcm_dnx_vlan_port_create_egress_match_port_default(
    int unit,
    bcm_vlan_port_t * vlan_port)
{
    uint32 entry_handle_id;
    uint32 ac_profile = 0;

    SHR_FUNC_INIT_VARS(unit);

    
    vlan_port->vlan_port_id = ac_profile;

    /*
     * Write to EGRESS DEFAULT AC PROFILE table: AC_Profile -> AC info (Clean AC entry)
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_DEFAULT_AC_PROF, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_DEFAULT_AC_PROF, INST_SINGLE, ac_profile);
    
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ACTION_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_NWK_QOS_IDX, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LAYER_TYPE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_QOS_MODEL, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_1, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_2, INST_SINGLE, 0);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Configure VLAN PORT egress AC using ESEM 
 *       Mapping VLAN domain + VSI to egress vlan
 *  
 *          Called by bcm_dnx_vlan_port_create
 * parameters: bcm_vlan_port_t (in)
 */
static shr_error_e
bcm_dnx_vlan_port_create_egress_match_esem(
    int unit,
    bcm_vlan_port_t * vlan_port)
{
    uint32 entry_handle_id;
    uint32 vlan_domain = 0;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Add AC entry to ESEM 
     */
    SHR_IF_ERR_EXIT(dnx_port_pp_vlan_domain_get(unit, vlan_port->port, &vlan_domain));
    /*
     * Write to EGRESS ESEM OUT AC table (Clean AC entry)
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ESEM_OUT_AC, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_DOMAIN, INST_SINGLE, vlan_domain);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, vlan_port->vsi);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_1, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_2, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ACTION_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_NWK_QOS_IDX, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_REMARK_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_QOS_MODEL, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OAM_LIF_SET, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PROTECTION_PATH, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PROTECTION_POINTER, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_STAT_OBJECT_CMD, INST_SINGLE, 0);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Configure VLAN PORT egress EEDB 
 *       Mapping OutLif to egress AC information
 *  
 *          Called by bcm_dnx_vlan_port_create
 * parameters: bcm_vlan_port_t (in), 
 *             local_out_lif (in)
 */
static shr_error_e
bcm_dnx_vlan_port_create_egress_match_eedb(
    int unit,
    bcm_vlan_port_t * vlan_port,
    uint32 local_out_lif)
{
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EEDB_OUT_AC, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF, INST_SINGLE, local_out_lif);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_1, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_2, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ACTION_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF_PROFILE, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OAM_LIF_SET, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_NWK_QOS_IDX, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PROTECTION_PATH, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PROTECTION_POINTER, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_STAT_OBJECT_CMD, INST_SINGLE, 0);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_QOS_MODEL, INST_SINGLE, 0);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Configure SW state of gport to forward information 
 *       according to vlan_port struct.
 *  
 *          Called by bcm_dnx_vlan_port_create
 * parameters: bcm_vlan_port_t (in) 
 *             glob_out_lif (in) 
 */
static shr_error_e
vlan_port_gport_to_forward_information_set(
    int unit,
    bcm_vlan_port_t * vlan_port,
    uint32 glob_out_lif)
{
    uint32 entry_handle_id;
    uint8 is_ingress;
    uint32 result_type, destination;

    SHR_FUNC_INIT_VARS(unit);

    is_ingress = _SHR_IS_FLAG_SET(vlan_port->flags, BCM_VLAN_PORT_EGRESS_ONLY) ? FALSE : TRUE;

    /*
     * SW State should be filled only if ingress is configured because it represents 
     * the learning information relevant for this gport 
     */
    if (is_ingress)
    {
        /*
         * Currently this is the only type. Protection will be added
         */
        /*
         * Forwarding is done according to destination and Outlif
         */
        result_type = DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_OUTLIF;
        SHR_IF_ERR_EXIT(algo_gpm_encode_destination_field_from_gport(unit, vlan_port->port, &destination));

        /*
         * Fill destination (from Gport) info Forward Info table (SW state)
         */
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SW_STATE_GPORT_TO_FORWARDING_INFO, &entry_handle_id));
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LOGICAL_GPORT, INST_SINGLE,
                                   vlan_port->vlan_port_id);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, result_type);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DESTINATION, INST_SINGLE, destination);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF, INST_SINGLE, glob_out_lif);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
bcm_dnx_vlan_port_create(
    int unit,
    bcm_vlan_port_t * vlan_port)
{
    uint32 entry_handle_id;
    bcm_gport_t glob_in_lif, glob_out_lif, local_in_lif, local_out_lif;
    uint8 is_ingress, is_egress;

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_vlan_port_create_verify(unit, vlan_port));

    is_ingress = _SHR_IS_FLAG_SET(vlan_port->flags, BCM_VLAN_PORT_EGRESS_ONLY) ? FALSE : TRUE;
    is_egress = _SHR_IS_FLAG_SET(vlan_port->flags, BCM_VLAN_PORT_INGRESS_ONLY) ? FALSE : TRUE;

    /*
     * SW Allocations
     */
    /*
     * DNX SW Algorithm, skip allocation of LIF, requires gport decoding in the future 
     * In the future add here lif allocation algorithm call
     */
    if (BCM_GPORT_IS_VLAN_PORT(vlan_port->vlan_port_id))
    {
        glob_in_lif = glob_out_lif = local_in_lif = local_out_lif =
            BCM_GPORT_SUB_TYPE_LIF_VAL_GET(BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->vlan_port_id));
    }
    else
    {
        glob_in_lif = glob_out_lif = local_in_lif = local_out_lif = vlan_port->vlan_port_id;
    }

    /*
     * SW State
     */
    SHR_IF_ERR_EXIT(vlan_port_gport_to_forward_information_set(unit, vlan_port, glob_out_lif));

    /*
     * Map Local In-LIF to Global In-LIF  
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IN_AC_INFO_DB, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_LIF, INST_SINGLE, local_in_lif);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                 DBAL_RESULT_TYPE_IN_AC_INFO_DB_IN_LIF_FORMAT_AC_MP);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_GLOB_IN_LIF, INST_SINGLE, glob_in_lif);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_FORWARDING_DOMAIN, INST_SINGLE, vlan_port->vsi);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    /*
     * Match criteia can be either PORT (default configuration per port) 
     * or other (per VLAN for example) 
     * Relevant for both ingress and egress 
     */
    switch (vlan_port->criteria)
    {
        case BCM_VLAN_PORT_MATCH_PORT:
        {
            if (is_ingress)
            {
                /*
                 * Ingress default action
                 */
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_create_ingress_match_port_default(unit, vlan_port, local_in_lif));

            }
            if (is_egress)
            {
                /*
                 * Egress default match
                 */
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_create_egress_match_port_default(unit, vlan_port));
            }
            break;
        }
        case BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED:
        {
            /*
             * Three configuration modes: 1. Symmetric (InLif + Outlif EEDB) 2. Ingress only (InLif) 3. Egress only
             * (Outlif) 
             */
            if (is_ingress)
            {
                /*
                 * case 1/2: Symmetric/Ingress only 
                 */
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_create_ingress_match_port_vlan_vlan(unit, vlan_port, local_in_lif));
            }

            if (is_egress)
            {
                /*
                 * case 2/3: Symmetric/Egress only 
                 */
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_create_egress_match_eedb(unit, vlan_port, local_out_lif));
            }
            break;
        }
        case BCM_VLAN_PORT_MATCH_PORT_VLAN:
        default:
        {
            /*
             * Four configuration modes: 
             * 1. Symmetric (InLif + OutLif EEDB)
             * 2. Ingress only (InLif)
             * 3. Egress only (ESEM) 
             * 4. Egress only (OutLif)
             */
            if (is_ingress)
            {
                /*
                 * Case 1/2 above: Symmetric/Ingress only
                 */
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_create_ingress_match_port_vlan(unit, vlan_port, local_in_lif));
            }
            /*
             * Cases 1/3/4 above: Symmetric/Egress only
             */
            if (is_egress && _SHR_IS_FLAG_SET(vlan_port->flags, BCM_VLAN_PORT_VLAN_TRANSLATION))
            {
                /*
                 * Case 3.  Egress only (ESEM) 
                 */
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_create_egress_match_esem(unit, vlan_port));
            }
            else if (is_egress && !_SHR_IS_FLAG_SET(vlan_port->flags, BCM_VLAN_PORT_VLAN_TRANSLATION))
            {
                /*
                 * Case 1: Symmetric (InLif + OutLif EEDB)
                 * Case 4. Egress only (OutLif)  
                 */
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_create_egress_match_eedb(unit, vlan_port, local_out_lif));
            }
            break;
        }
    }

exit:
    SHR_FUNC_EXIT;
}
