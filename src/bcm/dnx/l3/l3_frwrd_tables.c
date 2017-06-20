/*
 * ! \file l3_frwd_tables.c L3 forwarding tables procedures for DNX.
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
 * Include files currently used for DNX.
 * {
 */
#include <soc/dnx/dbal/dbal.h>
#include <soc/dnx/dnx_data/dnx_data_l2.h>
#include <soc/dnx/dnx_data/dnx_data_l3.h>
#include <shared/shrextend/shrextend_debug.h>

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
#include <bcm/l3.h>
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
 * Whether an entry is default or not, relevant only for route entries.
 */
#define L3_FRWRD_TABLES_IS_DEFAULT_ENTRY(ip_mask) (ip_mask == 0)

/*
 * }
 */

/**
 * \brief
 *  Verify the host entry add input.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] info - The l3 host structure to be verified.
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT INPUT
 *  See *info above \n
 */
static shr_error_e
dnx_l3_host_add_verify(
    int unit,
    bcm_l3_host_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify the the VRF is in range.
     */
    if (info->l3a_vrf >= dnx_data_l2.vsi.nof_vsis_get(unit))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "VRF %d is out of range, should be lower then %d.", info->l3a_vrf,
                     dnx_data_l2.vsi.nof_vsis_get(unit));
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Verify the route entry add input.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] info - The l3 route structure to be verified.
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT INPUT
 *  See *info above \n
 */
static shr_error_e
dnx_l3_route_add_verify(
    int unit,
    bcm_l3_route_t * info)
{
    
    uint32 fec_index;
    SHR_FUNC_INIT_VARS(unit);

    fec_index = BCM_L3_ITF_VAL_GET(info->l3a_intf);

    
    if (fec_index >= dnx_data_l3.fec.max_nof_fec_get(unit))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "FEC index %d is out of range, should be lower then %d.", fec_index,
                     dnx_data_l3.fec.max_nof_fec_get(unit));
    }

    /*
     * FECs that are used for default entry has a different range of values.
     */
    if (L3_FRWRD_TABLES_IS_DEFAULT_ENTRY(info->l3a_ip_mask)
        && (fec_index > dnx_data_l3.fec.max_fec_for_default_route_get(unit)))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "Default FEC index %d is out of range, should be lower then %d.", fec_index,
                     dnx_data_l3.fec.max_fec_for_default_route_get(unit));
    }

    /*
     * Verify the the VRF is in range.
     */
    if (info->l3a_vrf >= dnx_data_l2.vsi.nof_vsis_get(unit))
    {
        SHR_ERR_EXIT(BCM_E_PARAM, "VRF %d is out of range, should be lower then %d.", info->l3a_vrf,
                     dnx_data_l2.vsi.nof_vsis_get(unit));
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Adds/updates an IP host entry.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] info - L3 host information (key and payload) to be added/updated.
 * \par DIRECT OUTPUT:
 *   \retval Zero in case of NO ERROR.
 * \par INDIRECT INPUT
 *  See *info above \n
 */
int
bcm_dnx_l3_host_add(
    int unit,
    bcm_l3_host_t * info)
{
    uint32 entry_handle_id;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_l3_host_add_verify(unit, info));

    

    /*
     * Get an handle to the host table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IPV4_UNICAST_PRIVATE_HOST, &entry_handle_id));

    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VRF, INST_SINGLE, info->l3a_vrf);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IPV4, INST_SINGLE, info->l3a_ip_addr);

    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                 DBAL_RESULT_TYPE_IPV4_UNICAST_PRIVATE_HOST_DESTINATION_ONLY);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_FEC, INST_SINGLE, info->l3a_intf);

    /*
     * setting the entry with the default values
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Adds/updates an IP route entry.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] info - L3 route information (key and payload) to be added/updated.
 * \par DIRECT OUTPUT:
 *   \retval Zero in case of NO ERROR.
 * \par INDIRECT INPUT
 *  See *info above \n
 */
int
bcm_dnx_l3_route_add(
    int unit,
    bcm_l3_route_t * info)
{
    dbal_fields_e dest_type;
    uint32 entry_handle_id;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_l3_route_add_verify(unit, info));

    

    if (info->l3a_vrf > 0)
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IPv4_UNICAST_PRIVATE_LPM_FORWARD, &entry_handle_id));
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VRF, INST_SINGLE, info->l3a_vrf);
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IPv4_UNICAST_PUBLIC_LPM_FORWARD, &entry_handle_id));
    }

    
    dbal_entry_key_field32_masked_set(unit, entry_handle_id, DBAL_FIELD_IPV4, INST_SINGLE, info->l3a_subnet, info->l3a_ip_mask);

    /*
     * In case this is a default entry, the result type is a default FEC which is limited to a smaller range than
     * the regular FEC and have different encoding in the LPM table.
     */
    dest_type = L3_FRWRD_TABLES_IS_DEFAULT_ENTRY(info->l3a_ip_mask) ? DBAL_FIELD_FEC_DEFAULT : DBAL_FIELD_FEC;

    dbal_entry_value_field32_set(unit, entry_handle_id, dest_type, INST_SINGLE, info->l3a_intf);

    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}
