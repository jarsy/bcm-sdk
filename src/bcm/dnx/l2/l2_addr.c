/** \file l2_addr.c
 * $Id$
 *
 * L2 procedures for DNX.
 *
 * This file contains functions for manipulating the MACT entries.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_L2
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
#include <bcm/l2.h>
/*
 * }
 */

/*
 * }
 */
/*
 * Function Declaration.
 * {
 */

/*
 * }
 */
/*
 * Defines.
 * {
 */
/** Strength of a dynamic entry in the MACT */
#define DYNAMIC_MACT_ENTRY_STRENGTH 1

/** Strength of a static entry in the MACT */
#define STATIC_MACT_ENTRY_STRENGTH 2

/**
 * ! \brief Verify l2addr parameter for BCM-API: bcm_dnx_l2_addr_add(). \n if l2addr.vid > 4K, make sure it's
 * allocated. \n Check l2addr.flags support. 
 */
static int
dnx_l2_addr_add_verify(
    int unit,
    bcm_l2_addr_t * l2addr)
{
    SHR_FUNC_INIT_VARS(unit);

    

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Implement the bcm_l2_addr_add in the case it adds to the FWD_MACT logical table
 */
static int
dnx_l2_addr_add_fwd_mact(
    int unit,
    bcm_l2_addr_t * l2addr)
{
    uint32 entry_handle_id;
    uint32 outLIF;
    uint32 destination;
    uint8 entry_strength;
    /** 1 for dynamic, 0 - static */
    /*uint8 is_dynamic;*/

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_l2_addr_add_verify(unit, l2addr));

    /*
     * GPORT MGMT missing. Currently assuming no GPORT encoding and no outLIF.
     */
    destination = l2addr->port;
    outLIF = 0;

    /*
     *  Set the entry's strength. Static entries should have a higher strength than dynamic.
     *  MACT payloads are updated when the new entry has a bigger strength than the previous.
     *  In order to give the static entries a priority over the dynamic, they should have a higher or equal strength.
     */
    if (l2addr->flags & BCM_L2_STATIC)
    {
        entry_strength = STATIC_MACT_ENTRY_STRENGTH;
        /*is_dynamic = 0;*/
    }
    else
    {
        entry_strength = DYNAMIC_MACT_ENTRY_STRENGTH;
        /*is_dynamic = 1;*/
    }

    /*
     * Write to MAC table 
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_MACT, &entry_handle_id));

    /** Key fields */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_FID, INST_SINGLE, l2addr->vid);
    dbal_entry_key_field_arr8_set(unit, entry_handle_id, DBAL_FIELD_L2_MAC, INST_SINGLE, l2addr->mac);

    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, DBAL_RESULT_TYPE_MACT_SINGLE_OUTLIF);
     
    if (l2addr->flags & BCM_L2_MCAST)
    {
      /** Destination is a multicast group */
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_MC_ID, INST_SINGLE, l2addr->l2mc_group);
    }
    else
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PORT_ID, INST_SINGLE, destination);
    }

    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_GLOB_OUT_LIF, INST_SINGLE, outLIF);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ENTRY_GROUPING, INST_SINGLE, l2addr->group);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_MAC_STRENGTH, INST_SINGLE, entry_strength);
    /*dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_AGING, INST_SINGLE, is_dynamic);*/

    /*
     * setting the entry with the default values 
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Create a MAC table entry
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] l2addr -
 *     The entry's information. \n
 *     l2addr.vid - Forwarding-ID \n
 *     l2addr.mac - MAC address \n
 *     l2addr.port - A gport to a system-destination (DSPA)
 *     l2addr.flags - can have the following flags concatenated: BCM_L2_STATIC, BCM_L2_MCAST, BCM_L2_TRUNK_MEMBER.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: MAC table is full
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Write to HW MAC table.
 */
int
bcm_dnx_l2_addr_add(
    int unit,
    bcm_l2_addr_t * l2addr)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_l2_addr_add_verify(unit, l2addr));

  /** Select the correct logical table that should be used for the add */
    SHR_IF_ERR_EXIT(dnx_l2_addr_add_fwd_mact(unit, l2addr));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Initialize the BCM L2 subsystem. 
 */
int
bcm_dnx_l2_init(
    int unit)
{
    return -1;
}

/*
 * Delete an L2 address entry from the specified device. 
 */
int
bcm_dnx_l2_addr_delete(
    int unit,
    bcm_mac_t mac,
    bcm_vlan_t vid)
{
    return -1;
}

/*
 * Check if an L2 entry is present in the L2 table. 
 */
int
bcm_dnx_l2_addr_get(
    int unit,
    bcm_mac_t mac_addr,
    bcm_vlan_t vid,
    bcm_l2_addr_t * l2addr)
{
    return -1;
}
