/** \file l3.c
 *  
 *  l3 procedures for DNX.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_L3
/*
 * Include files.
 * {
 */
#include <soc/dnx/dbal/dbal.h>
#include <bcm/types.h>
#include <bcm_int/dnx/l3/l3_arp.h>
#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/port/port_pp.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <shared/util.h>
#include <bcm/error.h>
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

shr_error_e
dnx_l3_egress_create_arp_verify(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id)
{
    /*
     * uint32 ll_global_lif;
     */

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Supporting ARP only with EGRESS_ONLY flag
     */
    if (!_SHR_IS_FLAG_SET(flags, BCM_L3_EGRESS_ONLY))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "BCM_L3_EGRESS_ONLY flag should be set when configurting ARP.\r\n");
    }
    /*
     * For WITH_ID configuration - check ID is in valid range
     */
    if (_SHR_IS_FLAG_SET(flags, BCM_L3_WITH_ID))
    {
        /*
         * ll_global_lif = BCM_L3_ITF_VAL_GET(egr->encap_id);
         */
        
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_l3_egress_create_arp_allocate(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id,
    uint32 * ll_local_lif,
    uint32 * ll_global_lif)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Allocation with ID
     */
    if (egr->encap_id > 0)
    {
        *ll_global_lif = BCM_L3_ITF_VAL_GET(egr->encap_id);
    }

    
    *ll_local_lif = *ll_global_lif;

    /*
     * Choose entry size according to vlan parameter
     */
    if (egr->vlan)
    {
        /*
         * Allocate standard entry
         */
    }
    else
    {
        /*
         * Allocate small entry
         */
    }

    /*
     * Fill encap_id parameter
     */
    BCM_L3_ITF_SET(egr->encap_id, BCM_L3_ITF_TYPE_LIF, *ll_global_lif);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_l3_egress_create_arp_hw_write(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id,
    uint32 ll_local_lif)
{
    uint32 entry_handle_id = 0;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Write to EGRESS EEDB ARP table 
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ARP_BASIC, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF, INST_SINGLE, ll_local_lif);

    /*
     * Choose entry size according to vlan parameter
     */
    if (egr->vlan)
    {
        
        /*
         * dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_FORWARDING_DOMAIN, egr->vlan);
         */
    }

    dbal_entry_value_field_arr8_set(unit, entry_handle_id, DBAL_FIELD_ENCAP_DEST, INST_SINGLE, egr->mac_addr);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

    /*
     * Write to EGRESS EEDB IP/MPLS table 
     * (update only next pointer value that is pointing to ARP) 
     */
    
    if (egr->intf > 0)
    {
        /*
         * Interface might be LIF or RIF 
         */
        if (BCM_L3_ITF_TYPE_IS_LIF(egr->intf))
        {
            /*
             * Interface is OutLif, choose according to lif type which EEDB entry to edit 
             */
            /*
             * SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EEDB_OUT_TUNNELxxx, &entry_handle_id));
             * dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LIF, tunnel_local_lif);
             * dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_NEXT_PONTER, ll_local_lif);
             */
            SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
        }
        else
        {
            
        }
    }

exit:
    SHR_FUNC_EXIT;
}
