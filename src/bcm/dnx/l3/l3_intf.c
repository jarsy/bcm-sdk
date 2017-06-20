/** \file l3_intf.c
 *  
 *  l3 interface for DNX.
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
#include <bcm_int/dnx/l3/l3.h>
#include <bcm_int/dnx/l3/l3_arp.h>
#include <bcm_int/dnx/l3/l3_fec.h>
#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/port/port_pp.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <shared/util.h>
#include <bcm/error.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <soc/dnx/dnx_data/dnx_data_l2.h>
#include <shared/utilex/utilex_bitstream.h>
#include <bcm_int/dnx/algo/l3/source_address_table_allocation.h>
#include <bcm_int/dnx/algo/template_mngr/template_mngr_api.h>
/*
 * }
 */

/*
 * DEFINES
 * {
 */

#define L3_INTF_MY_MAC_LSB_SIZE_IN_BITS          (10)
#define L3_INTF_MY_MAC_SECOND_BYTE_MASK UTILEX_BITS_MASK(L3_INTF_MY_MAC_LSB_SIZE_IN_BITS - SAL_UINT8_NOF_BITS - 1,0)
/*
 * }
 */

/*
 * MACROs
 * {
 */

#define L3_INTF_GET_MY_MAC_LSB(my_mac) ( ((my_mac[4] & L3_INTF_MY_MAC_SECOND_BYTE_MASK) << SAL_UINT8_NOF_BITS) |\
                                       my_mac[5])
#define L3_INTF_CLEAR_LSB_BITS(my_mac)

/** mask 10MSB from mac address */
#define L3_INTF_MAC_ADDR_MASK_10MSB(src, dst) \
    sal_memcpy(dst, src, L3_MAC_ADDR_SIZE_IN_BYTES); \
    dst[0] = 0; \
    dst[1] &= 0x3F

/** bits 1-6 are relevant so masking all the rest and shifting to right */
#define L3_INTF_MY_MAC_ETH_PREFIX(mymac) ((mymac >> 1) & 0x3F)

/*
 * }
 */

/**
* \brief
*  Update the following fields in DBAL_TABLE_RIF_BASIC: OUT_LIF FORWARDING_DOMAIN REMARK_PROFILE
* \par DIRECT INPUT:
*   \param [in] unit  -  The unit number.
*   \param [in] intf  -  The l3 interface structure.
* \par INDIRECT INPUT:
*   * \b *intf \n
*     See 'intf' in DIRECT INPUT above
* \par DIRECT OUTPUT:
*   shr_error_e - Non-zero in case of an error.
* \par INDIRECT OUTPUT:
*   * Write to HW OutRIF table
* \remark
*   * intf must be valid pointer
* \see
*   * dnx_l3_intf_create_egress()
*/
static shr_error_e
dnx_l3_intf_egress_outrif_info_set(
    int unit,
    bcm_l3_intf_t * intf)
{
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_RIF_BASIC, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF, INST_SINGLE, intf->l3a_vid);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_FORWARDING_DOMAIN, INST_SINGLE, intf->l3a_vid);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_REMARK_PROFILE, INST_SINGLE,
                                 intf->dscp_qos.qos_map_id);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
*  Update the following fields in DBAL_TABLE_EG_VSI_INFO: FIELD_VSI MY_MAC MY_MAC_PREFIX
* \par DIRECT INPUT:
*   \param [in] unit          -  The unit number.
*   \param [in] intf          -  The l3 interface structure.
*   \param [in] mymac_prefix  -  Mac prefix to be saved.
* \par INDIRECT INPUT:
*   * \b *intf \n
*     See 'intf' in DIRECT INPUT above
* \par DIRECT OUTPUT:
*   shr_error_e - Non-zero in case of an error.
* \par INDIRECT OUTPUT:
*   * None
* \remark
*   * intf must be valid pointer
* \see
*   * L3_INTF_GET_MY_MAC_LSB
*/
static shr_error_e
dnx_l3_intf_egress_vsi_info_set(
    int unit,
    bcm_l3_intf_t * intf,
    int mymac_prefix)
{
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EG_VSI_INFO, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, intf->l3a_vid);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_MY_MAC, INST_SINGLE,
                                 L3_INTF_GET_MY_MAC_LSB(intf->l3a_mac_addr));
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_MY_MAC_PREFIX, INST_SINGLE,
                                 L3_INTF_MY_MAC_ETH_PREFIX(mymac_prefix));
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
*  Write the source address MSBs to soruce_address table given the mymac index
* \par DIRECT INPUT:
*   \param [in] unit          -  The unit number.
*   \param [in] l3a_mac_addr  -  Requested MSB mac address to be written. (not shifted)
*   \param [in] mymac_prefix  -  MAC prefix to be saved to Index to the table (retrieve from the source_address template manager)
* \par INDIRECT INPUT:
*   * None
* \par DIRECT OUTPUT:
*   shr_error_e - Non-zero in case of an error.
* \par INDIRECT OUTPUT:
*   * Write to HW Egress Source address MSB table
* \remark
*   * intf must be valid pointer
* \see
*   * None
*/
static shr_error_e
dnx_l3_intf_egress_mymac_set(
    int unit,
    bcm_mac_t l3a_mac_addr,
    int mymac_prefix)
{
    uint32 entry_handle_id;
    bcm_mac_t mac_38_lsb;

    SHR_FUNC_INIT_VARS(unit);

    L3_INTF_MAC_ADDR_MASK_10MSB(l3a_mac_addr, mac_38_lsb);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SOURCE_ADDERSS_ETHERNET, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SOURCE_ADDRESS_INDEX, INST_SINGLE, mymac_prefix);
    dbal_entry_value_field_arr8_set(unit, entry_handle_id, DBAL_FIELD_SOURCE_ADDRESS, INST_SINGLE, mac_38_lsb);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Used by bcm_dnx_l3_intf_create() to verify the route entry add input
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] intf - The l3 interface structure to be verified.
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT INPUT
 *  See *intf above \n
 */
static shr_error_e
dnx_l3_intf_create_verify(
    int unit,
    bcm_l3_intf_t * intf)
{

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(intf, _SHR_E_PARAM, "l3_intf");

    /*
     * Verify the the VRF is in range.
     */
    if (intf->l3a_vrf >= dnx_data_l2.vsi.nof_vsis_get(unit))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "VRF %d is out of range, should be lower then %d.", intf->l3a_vrf,
                     dnx_data_l2.vsi.nof_vsis_get(unit));
    }
    /*
     * Verify that the MAC address isn't all zeros or an MC MAC address.
     */
    if (BCM_MAC_IS_ZERO(intf->l3a_mac_addr) || BCM_MAC_IS_MCAST(intf->l3a_mac_addr))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "MAC address can't be a Multicast MAC address or a 0 MAC address (%p).",
                     intf->l3a_mac_addr);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Add an entry into the MyMac table or increase the reference counter by on if the entry is
 *  already  exists in the template manager..
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] intf - The l3 interface structure.
 *  \param [in] my_mac_prefix_index - pointer to the received MyMac prefix index.
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT OUTPUT
 *   See *my_mac_prefix_index above \n
 * \par INDIRECT INPUT
 *  See *intf above \n
 */
static shr_error_e
dnx_l3_intf_add_my_mac(
    int unit,
    bcm_l3_intf_t * intf,
    uint32 * my_mac_prefix_index)
{
    uint8 first_reference = 0;
    uint32 entry_handle_id;
    bcm_mac_t prefix;

    SHR_FUNC_INIT_VARS(unit);

    sal_memcpy(prefix, intf->l3a_mac_addr, L3_MAC_ADDR_SIZE_IN_BYTES);

    /** Clear LSB bits from the prefix */
    prefix[5] = 0;
    prefix[4] &= ~L3_INTF_MY_MAC_SECOND_BYTE_MASK;

    SHR_IF_ERR_EXIT(dnx_algo_template_allocate
                    (unit, _SHR_CORE_ALL, "Ingress MyMac prefix table", 0, prefix, NULL, (int *) my_mac_prefix_index,
                     &first_reference));

    /*
     * In case this is the first time this profile is allocated, enter the new prefix into the prefix table.
     */
    if (first_reference)
    {

        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_MY_MAC_DA_PREFIXES, &entry_handle_id));

        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_MY_MAC_PREFIX, INST_SINGLE, *my_mac_prefix_index);

        dbal_entry_value_field_arr8_set(unit, entry_handle_id, DBAL_FIELD_L2_MAC, INST_SINGLE, prefix);

        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
*  Allocate entry for input mac address using template algorithm and get corresponding index (provided by the algorithm) and an indication on whether this mac address has already been allocated.
* \par DIRECT INPUT:
*   \param [in] unit                      -  The Unit number.
*   \param [in] l3a_mac_addr              -  Requested mac address to be checked.
*   \param [in] mymac_prefix              -  Valid pointer to mac address prefix provided by the algorithm.
*   \param [in] is_first_mymac_reference  -  Boolean. 1 - algorithm allocated new entry, 0 - mac address already allocated.
* \par INDIRECT INPUT:
*   * None
* \par DIRECT OUTPUT:
*   shr_error_e - Non-zero in case of an error.
* \par INDIRECT OUTPUT:
*   * mymac_prefix
*   * is_first_mymac_reference
* \remark
*   * None
* \see
*   * None
*/
static shr_error_e
dnx_l3_intf_egress_mymac_prefix_allocate(
    int unit,
    bcm_mac_t l3a_mac_addr,
    int *mymac_prefix,
    uint8 * is_first_mymac_reference)
{
    source_address_entry_t source_address_entry;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&source_address_entry, 0, sizeof(source_address_entry));

    source_address_entry.address_type = source_address_type_mac;
    sal_memcpy(source_address_entry.address.mac_address, l3a_mac_addr, sizeof(bcm_mac_t));

    SHR_IF_ERR_EXIT(dnx_algo_template_allocate
                    (unit, _SHR_CORE_ALL, "Egress source address table", 0, &source_address_entry, NULL, mymac_prefix,
                     is_first_mymac_reference));

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
*  Write L3 interface (egress part) to HW tables
* \par DIRECT INPUT:
*  \param [in] unit - The Unit number.
*  \param [in] intf - The L3 interface structure to be written.
* \par INDIRECT INPUT:
*   * \b *intf \n
*     See 'intf' in DIRECT INPUT above
* \par DIRECT OUTPUT:
*   shr_error_e - Non-zero in case of an error.
* \par INDIRECT OUTPUT:
*   * Following tables will be updated:
*   * DBAL_TABLE_EG_VSI_INFO,
*   * DBAL_TABLE_RIF_BASIC,
*   * DBAL_TABLE_SOURCE_ADDERSS_ETHERNET (if this is the first allocation)
* \remark
*   * intf must be valid pointer
* \see
*   * None
*/
static shr_error_e
dnx_l3_intf_create_egress(
    int unit,
    bcm_l3_intf_t * intf)
{
    int mymac_prefix;
    uint8 is_first_mymac_reference;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * SW allocations
     */
    SHR_IF_ERR_EXIT(dnx_l3_intf_egress_mymac_prefix_allocate
                    (unit, intf->l3a_mac_addr, &mymac_prefix, &is_first_mymac_reference));

    /*
     * writing to HW tables
     */
    if (is_first_mymac_reference)
    {
        /** Set prefix MSB MyMAC information in case it is the first time. */
        SHR_IF_ERR_EXIT(dnx_l3_intf_egress_mymac_set(unit, intf->l3a_mac_addr, mymac_prefix));
    }

    /** Add entry to RIF table */
    SHR_IF_ERR_EXIT(dnx_l3_intf_egress_outrif_info_set(unit, intf));

    /** Add entry to EG_VSI table */
    SHR_IF_ERR_EXIT(dnx_l3_intf_egress_vsi_info_set(unit, intf, mymac_prefix));

exit:
    SHR_FUNC_EXIT;

}

/**
 * \brief
 *  Write L3 interface (ingress part) to HW tables
 * \par DIRECT INPUT
 *  \param [in] unit - The Unit number.
 *  \param [in] intf - The L3 interface structure to be written.
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT INPUT
 *  See *intf above \n
 */
int
bcm_dnx_l3_intf_create_ingress(
    int unit,
    bcm_l3_intf_t * intf)
{

    uint32 entry_handle_id, profile_to_write = 0, my_mac_prefix_index;
    int default_rif_profile = 0, rv;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * check input flags
     */
    if ((intf->l3a_flags & BCM_L3_WITH_ID) && (intf->l3a_intf_id != intf->l3a_vid))
    {
        /*
         * l3a_intf_id must be equal to l3a_vid in case BCM_L3_WITH_ID is set
         */
        SHR_ERR_EXIT(_SHR_E_PARAM, "BCM_L3_WITH_ID is set but intf->l3a_intf_id != intf->l3a_vid\n");
    }
    else
    {
        intf->l3a_intf_id = intf->l3a_vid;
    }

    /*
     * get the current VSI_profile from VSI table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, intf->l3a_vid);
    rv = dbal_entry_get(unit, entry_handle_id, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE);

    if (rv)
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(rv, "entry get failed\n");
    }
    rv = dbal_entry_handle_value_field32_get(unit, entry_handle_id, DBAL_FIELD_VSI_PROFILE, INST_SINGLE,
                                             &profile_to_write);
    dbal_entry_handle_release(unit, entry_handle_id);
    if (rv)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "field VSI_PROFILE didnot found!!\n");
    }

    /*
     * change only 5 LSB bits which are RIF profile
     * assume default profile will always exist in template manager and will never be deleted
     */
    profile_to_write = CALCULATE_VSI_PROFILE(profile_to_write, default_rif_profile);

    /*
     * set new VSI profile (only VRF and 5 LSB bit of VSI_PROFILE may have been changed)
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, intf->l3a_vid);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                 DBAL_RESULT_TYPE_ING_VSI_INFO_BASIC_FORMAT);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VRF, INST_SINGLE, intf->l3a_vrf);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI_PROFILE, INST_SINGLE, profile_to_write);

    /*
     * MYMAC HW config
     */
    SHR_IF_ERR_EXIT(dnx_l3_intf_add_my_mac(unit, intf, &my_mac_prefix_index));
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_MY_MAC_PREFIX, INST_SINGLE, my_mac_prefix_index);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_MY_MAC, INST_SINGLE,
                                 L3_INTF_GET_MY_MAC_LSB(intf->l3a_mac_addr));

    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Create a new L3 interface.
 */
int
bcm_dnx_l3_intf_create(
    int unit,
    bcm_l3_intf_t * intf)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_l3_intf_create_verify(unit, intf));

    /*
     * invalid flag 
     */
    if ((intf->l3a_flags & BCM_L3_WITH_ID) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "flag BCM_L3_WITH_ID must be set");
    }

    /*
     * Write to HW ingress RIF tables
     */
    SHR_IF_ERR_EXIT(bcm_dnx_l3_intf_create_ingress(unit, intf));

    /*
     * Write to HW egress RIF tables
     */
    SHR_IF_ERR_EXIT(dnx_l3_intf_create_egress(unit, intf));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Disables bits in Enablers vector according to user flags
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] routing_enablers_vector - the Enablers Vector to modify.
 *  \param [in] ing_intf - the ingress intreface.
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT INPUT
 *  See *routing_enablers_vector above
 *  See *ing_intf above \n
 */
STATIC int
dnx_l3_routing_enablers_vector_config(
    int unit,
    uint32 * routing_enablers_vector,
    bcm_l3_ingress_t * ing_intf)
{
    
    uint32 temp_routing_enablers_vector = 0xFFFFFFFF;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(ing_intf, _SHR_E_PARAM, "NULL input - ing_intf");

    if (ing_intf->flags & BCM_L3_INGRESS_ROUTE_DISABLE_IP4_UCAST)
    {
        temp_routing_enablers_vector &= ~IPV4_ENABLER_OFFSET;
    }
    if (ing_intf->flags & BCM_L3_INGRESS_ROUTE_DISABLE_IP6_UCAST)
    {
        temp_routing_enablers_vector &= ~IPV6_ENABLER_OFFSET;
    }
    if (ing_intf->flags & BCM_L3_INGRESS_ROUTE_DISABLE_IP6_MCAST)
    {
        temp_routing_enablers_vector &= ~IPV6_MC_ENABLER_OFFSET;
    }
    if (ing_intf->flags & BCM_L3_INGRESS_ROUTE_DISABLE_IP4_MCAST)
    {
        temp_routing_enablers_vector &= ~IPV4_MC_ENABLER_OFFSET;
    }
    if (ing_intf->flags & BCM_L3_INGRESS_ROUTE_DISABLE_MPLS)
    {
        temp_routing_enablers_vector &= ~MPLS_ENABLER_OFFSET;
    }

    routing_enablers_vector[1] = temp_routing_enablers_vector;

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  Creates L3 Ingress Interface.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] ing_intf - the ingress intreface.
 *  \param [in] intf_id - allocated interface ID.
 * \par DIRECT OUTPUT:
 *   Non-zero in case of an error.
 * \par INDIRECT INPUT
 *  See *ing_intf above
 *  See *intf_id above \n
 */
int
bcm_dnx_l3_ingress_create(
    int unit,
    bcm_l3_ingress_t * ing_intf,
    bcm_if_t * intf_id)
{
    uint32 entry_handle_id, old_profile = 0, profile_to_write = 0, routing_enablers_vector[2] =
        { 0xFFFFFFFF, 0xFFFFFFFF };
    uint32 all_zeroes[2] = { 0, 0 };
    int new_profile = 0, MSB_counter = 0, rv;
    uint8 is_first = 0, is_last = 0;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * disable the proper bits in enablers vector according to user's flags
     */
    dnx_l3_routing_enablers_vector_config(unit, routing_enablers_vector, ing_intf);

    /*
     * get old VSI_profile
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, *intf_id);
    rv = dbal_entry_get(unit, entry_handle_id, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE);
    if (rv)
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(rv, "entry get failed\n");
    }
    rv = dbal_entry_handle_value_field32_get(unit, entry_handle_id, DBAL_FIELD_VSI_PROFILE, INST_SINGLE,
                                             &profile_to_write);
    dbal_entry_handle_release(unit, entry_handle_id);
    if (rv)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "field VSI_PROFILE didnot found!!\n");
    }

    /*
     * get only 5 LSB bits of VSI_profile and perform the template profile exchange
     */
    old_profile = old_profile & 0x1F;
    SHR_IF_ERR_EXIT(dnx_algo_template_exchange(unit, _SHR_CORE_ALL, "enablers vector table",
                                               0, routing_enablers_vector, old_profile, NULL, &new_profile, &is_first,
                                               &is_last));

    /*
     * in case the old profile had only one pointer, need to delete it from HW
     */
    if (is_last)
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ENABLERS_VECTORS, &entry_handle_id));
        dbal_entry_key_field32_set(unit, entry_handle_id,
                                   DBAL_FIELD_ROUTING_ENABLERS_PROFILE, INST_SINGLE, old_profile);
        dbal_entry_value_field_arr32_set(unit, entry_handle_id, DBAL_FIELD_ENABLERS_VECTOR, INST_SINGLE, all_zeroes);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

        /*
         * write to table ROUTING_ENABLERS_PROFILE 32 times
         */
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ROUTING_ENABLERS_PROFILE, &entry_handle_id));
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ROUTING_ENABLERS_PROFILE, INST_SINGLE, 0);
        for (MSB_counter = 0; MSB_counter < NOF_ROUTING_PROFILES_DUPLICATES; MSB_counter++)
        {
            dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_RIF_PROFILE,
                                       INST_SINGLE, old_profile | MSB_counter << NOF_ENABLERS_PROFILE_BITS);
            _func_rv = dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE);
            if (_func_rv != _SHR_E_NONE)
            {
                dbal_entry_handle_release(unit, entry_handle_id);
                SHR_ERR_EXIT(_SHR_E_PARAM, "failed to write to table ROUTING_ENABLERS_PROFILE\n");
            }
        }
        dbal_entry_handle_release(unit, entry_handle_id);
    }
    /*
     * in case the new profile is used for the first time, need to write it to HW
     */
    if (is_first)
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ENABLERS_VECTORS, &entry_handle_id));
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_ROUTING_ENABLERS_PROFILE,
                                   INST_SINGLE, new_profile);
        dbal_entry_value_field_arr32_set(unit, entry_handle_id, DBAL_FIELD_ENABLERS_VECTOR,
                                         INST_SINGLE, routing_enablers_vector);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

        /*
         * write to table ROUTING_ENABLERS_PROFILE 32 times
         */
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ROUTING_ENABLERS_PROFILE, &entry_handle_id));
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ROUTING_ENABLERS_PROFILE,
                                     INST_SINGLE, new_profile);
        for (MSB_counter = 0; MSB_counter < NOF_ROUTING_PROFILES_DUPLICATES; MSB_counter++)
        {
            dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_RIF_PROFILE,
                                       INST_SINGLE, new_profile | MSB_counter << NOF_ENABLERS_PROFILE_BITS);
            _func_rv = dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE);
            if (_func_rv != _SHR_E_NONE)
            {
                dbal_entry_handle_release(unit, entry_handle_id);
                SHR_ERR_EXIT(_SHR_E_PARAM, "failed to write to table ROUTING_ENABLERS_PROFILE\n");
            }
        }
        dbal_entry_handle_release(unit, entry_handle_id);
    }

    /*
     * get the current VSI_profile in VSI table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, *intf_id);
    rv = dbal_entry_get(unit, entry_handle_id, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE);
    if (rv)
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(rv, "entry get failed\n");
    }
    rv = dbal_entry_handle_value_field32_get(unit, entry_handle_id, DBAL_FIELD_VSI_PROFILE, INST_SINGLE,
                                             &profile_to_write);
    dbal_entry_handle_release(unit, entry_handle_id);
    if (rv)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "field VSI_PROFILE didnot found!!\n");
    }

    /*
     * change only 5 LSB bits which are RIF profile
     */
    profile_to_write = (profile_to_write & 0x3E0) | (new_profile & 0x1F);

    /*
     * set new VSI profile (only VRF and 5 LSB bit of VSI_PROFILE may have been changed)
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, *intf_id);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                 DBAL_RESULT_TYPE_ING_VSI_INFO_BASIC_FORMAT);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VRF, INST_SINGLE, ing_intf->vrf);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI_PROFILE, INST_SINGLE, profile_to_write);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Delete an L3 interface.
 */
int
bcm_dnx_l3_intf_delete(
    int unit,
    bcm_l3_intf_t * intf)
{
    return -1;
}

/*
 * Given the L3 interface number, return the interface information. 
 */
int
bcm_dnx_l3_intf_get(
    int unit,
    bcm_l3_intf_t * intf)
{
    return -1;
}
