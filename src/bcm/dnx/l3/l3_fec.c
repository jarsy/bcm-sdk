/** \file l3_fec.c
 *  
 *  l3 FEC procedures for DNX.
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
#include <bcm_int/dnx/l3/l3_fec.h>
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
dnx_l3_egress_create_fec_verify(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id)
{
    /*
     * uint32 fec_index;
     */

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Supporting FEC only with INGRESS_ONLY flag
     */
    if (!_SHR_IS_FLAG_SET(flags, BCM_L3_INGRESS_ONLY))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "BCM_L3_INGRESS_ONLY flag should be set when configurting FEC.\r\n");
    }
    /*
     * For WITH_ID configuration - check ID is in valid range
     */
    if (_SHR_IS_FLAG_SET(flags, BCM_L3_WITH_ID))
    {
        /*
         * fec_index = BCM_L3_ITF_VAL_GET(*if_id);
         */
        
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_l3_egress_create_fec_allocate(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id)
{
    uint32 fec_index = 0;

    SHR_FUNC_INIT_VARS(unit);

    if (_SHR_IS_FLAG_SET(flags, BCM_L3_WITH_ID))
    {
        fec_index = BCM_L3_ITF_VAL_GET(*if_id);
    }

    

    /*
     * Return Allocated ID inside if_id
     */
    BCM_L3_ITF_SET(*if_id, BCM_L3_ITF_TYPE_FEC, fec_index);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_l3_egress_create_fec_hw_write(
    int unit,
    uint32 flags,
    bcm_l3_egress_t * egr,
    bcm_if_t * if_id)
{
    uint32 entry_handle_id = 0;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Write to FEC table 
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_FEC, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_FEC, INST_SINGLE, BCM_L3_ITF_VAL_GET(*if_id));
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DESTINATION, INST_SINGLE, egr->port);
    /*
     * ARP
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_GLOB_OUT_LIF, INST_SINGLE, BCM_L3_ITF_VAL_GET(egr->encap_id));
    /*
     * OutRif  
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_GLOB_OUT_LIF_2ND, INST_SINGLE, BCM_L3_ITF_VAL_GET(egr->intf));
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}
