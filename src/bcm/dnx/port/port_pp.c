/** \file port_pp.c
 * $Id$
 *
 * PP Port procedures for DNX.
 *
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
#include <shared/bslenum.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <bcm_int/dnx/port/port_pp.h>
#include <soc/dnx/dbal/dbal.h>
#include <bcm_int/dnx_dispatch.h>
/*
 * }
 */

/**
 * \brief
 * Verify vid parameter for BCM-API: bcm_dnx_port_untagged_vlan_set().
 */
static int
dnx_port_untagged_vlan_set_verify(
    int unit,
    bcm_port_t port,
    bcm_vlan_t vid)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Check vid < 4K 
     */

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
dnx_port_pp_vlan_domain_set_verify(
    int unit,
    bcm_port_t port,
    uint32 vlan_domain)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_pp_vlan_domain_set(
    int unit,
    bcm_port_t port,
    uint32 vlan_domain)
{
    uint32 entry_handle_id;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_port_pp_vlan_domain_set_verify(unit, port, vlan_domain));

    /*
     * Get Port + Core 
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get(unit, port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE, &gport_info));

    /*
     * Write to INGRESS PORT table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_PORT, &entry_handle_id));

    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);

    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_DOMAIN, INST_SINGLE, vlan_domain);

    /*
     * setting the entry with the default values 
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

    /*
     * Write to EGRESS PORT table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_PORT, &entry_handle_id));

    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);

    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_DOMAIN, INST_SINGLE, vlan_domain);

    /*
     * setting the entry with the default values
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *   Verify Port
 *   parameter for BCM-API: bcm_dnx_port_class_get()
 */
static shr_error_e
dnx_port_pp_vlan_membership_if_get_verify(
    int unit,
    bcm_port_t port)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Get the VLAN Membership-IF from local port.
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Relevant unit.
 *   \param [in] port - port - physical port
 *   \param [in] vlan_mem_if - Vlan membership interface
 *
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *  Read from Ingress Port table.
 * \remark
 *  We assume that VLAN-membership-IF mapping from local port is
 *  symmetric.Because of that, it is enough to get the
 *  information from Ingress Port table.
 * \see
     None
 */
int
dnx_port_pp_vlan_membership_if_get(
    int unit,
    bcm_port_t port,
    uint32 * vlan_mem_if)
{
    uint32 entry_handle_id;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_port_pp_vlan_membership_if_get_verify(unit, port));
    /*
     * Get Port + Core
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get(unit, port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE, &gport_info));

    /*
     * Reading from Ingress PORT_TABLE table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
    dbal_entry_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_VLAN_MEMBERSHIP_IF, INST_SINGLE,
                                     vlan_mem_if);

    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *   Verify Port
 *   parameter for BCM-API: bcm_dnx_port_class_set()
 */
static shr_error_e
dnx_port_pp_vlan_membership_if_set_verify(
    int unit,
    bcm_port_t port)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Set the VLAN Membership-IF per local port.
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   Relevant unit.
 *   \param [in] port -
 *   Port - physical port
 *   \param [in] vlan_mem_if -
 *   VLAN-membership-if
 * \par INDIRECT INPUT:
     None
 * \par DIRECT OUTPUT:
    None
 * \par INDIRECT OUTPUT
     Write to Ingress/Egress PORT TABLE
 * \remark
 *  We assume that VLAN-membership-IF mapping per local port is
 *  symmetric. Because of that, we need to set the appropriate
 *  information to Ingress and Egress Port tables.
 * \see
     See .h file
 */
int
dnx_port_pp_vlan_membership_if_set(
    int unit,
    bcm_port_t port,
    uint32 vlan_mem_if)
{
    uint32 entry_handle_id;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_port_pp_vlan_membership_if_set_verify(unit, port));

    /*
     * Get Port + Core
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get(unit, port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE, &gport_info));

    /*
     * Write to Ingress PORT_TABLE table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_MEMBERSHIP_IF, INST_SINGLE, vlan_mem_if);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

    /*
     * Write to Egress PORT_TABLE table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_MEMBERSHIP_IF, INST_SINGLE, vlan_mem_if);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
dnx_port_pp_vlan_domain_get_verify(
    int unit,
    bcm_port_t port)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * Return Egress VLAN domain 
 */
shr_error_e
dnx_port_pp_vlan_domain_get(
    int unit,
    bcm_port_t port,
    uint32 * vlan_domain)
{
    uint32 entry_handle_id;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_port_pp_vlan_domain_get_verify(unit, port));

    /*
     * Get Port + Core 
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get(unit, port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE, &gport_info));

    /*
     * Read EGRESS PORT table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_PORT, &entry_handle_id));

    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);

    dbal_entry_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_VLAN_DOMAIN, INST_SINGLE, vlan_domain);

    /*
     * getting the entry with the default values 
     */
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * This API determines VLANs according to Port-VID (PVID) for untag cases.
 * 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] port -
 *     A gport indicating physical port
 *   \param [in] vid -
 *     VLAN Identifier.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: invalid vid.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Update HW Ingress Port table
 */
int
bcm_dnx_port_untagged_vlan_set(
    int unit,
    bcm_port_t port,
    bcm_vlan_t vid)
{
    uint32 entry_handle_id;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_port_untagged_vlan_set_verify(unit, port, vid));

    /*
     * Get Port + Core 
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get(unit, port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE, &gport_info));

    /*
     * Write to INGRESS_PORT table 
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PORT_VID, INST_SINGLE, vid);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_pp_egress_match_port_add(
    int unit,
    bcm_gport_t ac_profile_gport,
    bcm_port_match_info_t * match_info)
{
    uint32 entry_handle_id;
    uint32 esem_access_command = 1;
    dnx_algo_gpm_gport_phy_info_t gport_info;
    uint32 ac_profile;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * This code is handling Default port match on egress. 
     * bcm_vlan_port_create allocates AC_PROFILE, returns allocated id 
     * bcm_port_match_add will get allocate ac_profile as parameter (through gport) and 
     * 1. Allocate ESEM_ACC_CMND
     * 2. Connect Port->ESEM_ACC_CMND->AC_PROFILE 
     */
    
    ac_profile = ac_profile_gport;
    
    /*
     * 2. Connect Port->ESEM_ACC_CMND->AC_PROFILE 
     */

    /*
     * Get physical port info
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get
                    (unit, match_info->port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_LOCAL_IS_MANDATORY, &gport_info));
    /*
     * Write ESEM_ACCESS_CMD to EGRESS PP PORT table: PORT -> ESEM_ACCESS_CMD
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, gport_info.internal_port_pp);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, gport_info.internal_core_id);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ESEM_ACCESS_CMD, INST_SINGLE, esem_access_command);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ESEM_ACCESS_CMD_PROP, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_ESEM_ACCESS_CMD, INST_SINGLE, esem_access_command);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DEFAULT_AC_PROF, INST_SINGLE, ac_profile);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ESEM_ACCESS_VALID, INST_SINGLE, 1);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
int
dnx_pp_port_init(
    int unit)
{

    bcm_port_t port;
    uint32 class_id;

    SHR_FUNC_INIT_VARS(unit);

    
    for (port = 0; port < 512; port++)
    {
        /*
         * Set default 1:1 mapping for VLAN membership according to Port x VLAN
         * Set the VLAN-membership-if to be equal to the port for each port to mimic JR1 mapping
         */
        class_id = (uint32) port;
        SHR_IF_ERR_EXIT(bcm_dnx_port_class_set(unit, port, bcmPortClassIngress, class_id));
    }

exit:
    SHR_FUNC_EXIT;

}
