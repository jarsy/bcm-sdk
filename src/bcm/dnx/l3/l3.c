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
#include <bcm/l3.h>
#include <bcm_int/dnx/l3/l3_arp.h>
#include <bcm_int/dnx/l3/l3_fec.h>
#include <bcm_int/dnx/l3/l3.h>
#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/port/port_pp.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <shared/util.h>
#include <bcm/error.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <bcm_int/dnx/algo/template_mngr/template_mngr_api.h>
/*
 * }
 */

/*
 * DEFINEs
 * {
 */

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
 * See .h file
 */
shr_error_e
dnx_l3_module_init(
  int unit)
{
    uint32 entry_handle_id = DBAL_SW_NOF_ENTRY_HANDLES;
    uint32 context;
    uint32 all_ones[2] = {0xFFFFFFFF,0xFFFFFFFF};
    SHR_FUNC_INIT_VARS(unit);

    

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_CONTEXT_RES_ATTRIBUTES, &entry_handle_id));
    dbal_entry_value_field8_set(unit, entry_handle_id, DBAL_FIELD_MY_MAC_BASED_VSI_ENABLE, INST_SINGLE, 1);

    for(context = 0; context < 64; context++)
    {
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CONTEXT_PROFILE, INST_SINGLE, context);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));
    }

    /* 
     * Enablers vector init.
     */ 
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ENABLERS_VECTORS, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_ROUTING_ENABLERS_PROFILE,INST_SINGLE, 0);
    dbal_entry_value_field_arr32_set(unit, entry_handle_id, DBAL_FIELD_ENABLERS_VECTOR,INST_SINGLE, all_ones);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));


exit:

  dbal_entry_handle_release(unit, entry_handle_id);
  SHR_FUNC_EXIT;
}



shr_error_e
bcm_dnx_l3_egress_create(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id)
{
    uint32 ll_local_lif = 0;
    uint32 ll_global_lif = 0;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Currently Ingress + Egress is not supported, verifying each mode separatly
     */
    if (_SHR_IS_FLAG_SET(flags, BCM_L3_INGRESS_ONLY))
    {
        /*
         * Verify ingress (FEC)
         */
        SHR_INVOKE_VERIFY_DNX(dnx_l3_egress_create_fec_verify(unit, flags, egr, if_id));
    }
    else if (_SHR_IS_FLAG_SET(flags, BCM_L3_EGRESS_ONLY))
    {
        /*
         * Verify egress (ARP)
         */
        SHR_INVOKE_VERIFY_DNX(dnx_l3_egress_create_arp_verify(unit, flags, egr, if_id));
    }
    else
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Either BCM_L3_INGRESS_ONLY or BCM_L3_EGRESS_ONLY should be set.\r\n");
    }

    /*
     * SW Allocations
     */
    /*
     * DNX SW Algorithm, skip allocation of FEC/ARP, requires gport decoding in the future
     */
    if (_SHR_IS_FLAG_SET(flags, BCM_L3_INGRESS_ONLY))
    {
        /*
         * Allocate ingress (FEC)
         */
        SHR_IF_ERR_EXIT(dnx_l3_egress_create_fec_allocate(unit, flags, egr, if_id));
    }
    else if (_SHR_IS_FLAG_SET(flags, BCM_L3_EGRESS_ONLY))
    {
        /*
         * Allocate egress (ARP)
         */
        SHR_IF_ERR_EXIT(dnx_l3_egress_create_arp_allocate(unit, flags, egr, if_id, &ll_local_lif, &ll_global_lif));
    }

    /*
     * HW Configuration
     */
    /*
     * Write to HW tables
     */
    if (_SHR_IS_FLAG_SET(flags, BCM_L3_INGRESS_ONLY))
    {
        /*
         * Write to FEC table
         */
        SHR_IF_ERR_EXIT(dnx_l3_egress_create_fec_hw_write(unit, flags, egr, if_id));
    }
    else if (_SHR_IS_FLAG_SET(flags, BCM_L3_EGRESS_ONLY))
    {
        /*
         * Write ARP info the EEDB
         */
        SHR_IF_ERR_EXIT(dnx_l3_egress_create_arp_hw_write(unit, flags, egr, if_id, ll_local_lif));
    }

exit:
    SHR_FUNC_EXIT;
}




/* Destroy an Egress forwarding object. */
int bcm_dnx_l3_egress_destroy(
    int unit,
    bcm_if_t intf)
{
    return -1;
}

/* Get an Egress forwarding object. */
int bcm_dnx_l3_egress_get(
    int unit, 
    bcm_if_t intf, 
    bcm_l3_egress_t *egr)
{
    return -1;
}

