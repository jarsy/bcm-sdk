/** \file vlan.c
 * $Id$
 *
 * General VLAN functionality for DNX.
 * Dedicated set of VLAN APIs are distributed between vlan_*.c files: \n
 * vlan_port.c - BCM_VLAN_PORT (Attachment-Circuit) functionality.
 * 
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
#include <bcm_int/dnx/port/port_pp.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <bcm_int/dnx/algo/algo_port.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <bcm_int/dnx/vlan/vlan.h>
#include <soc/dnx/dnx_data/dnx_data_l2.h>
#include <soc/dnx/dnx_data/dnx_data_device.h>
#include <bcm_int/dnx_dispatch.h>

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
 * \brief
 *   Verify vlanId parameter for BCM-API: bcm_dnx_vlan_create()
 */
static shr_error_e
dnx_vlan_create_verify(
    int unit,
    bcm_vlan_t vid)
{
    SHR_FUNC_INIT_VARS(unit);

    BCM_DNX_VLAN_CHK_ID(unit, vid);

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *   Verify vlanId parameter for BCM-API: bcm_dnx_vlan_destroy()
 */
static shr_error_e
dnx_vlan_destroy_verify(
    int unit,
    bcm_vlan_t vid)
{
    SHR_FUNC_INIT_VARS(unit);

    BCM_DNX_VLAN_CHK_ID(unit, vid);

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_vlan_algo_res_init(
    int unit)
{
    dnx_algo_res_create_data_t data;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * VSI resource management - Allocate 
     */
    data.first_element = 1;
    /*
     * Element 0 cannot be used, vlan 0 is not a valid value - by protocol, nof is minus 1
     */
    data.nof_elements = dnx_data_l2.vsi.nof_vsis_get(unit) - 1;
    data.flags = 0;
    /*
     * data.desc = "VSI - L2 Forwarding Domain allocated IDs";
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_create(unit, BCM_CORE_ALL, "VSI", &data, NULL, NULL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Set VSI table entry to its default values 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit. 
 *   \param [in] vid -
 *     The incoming VSI ID, must be in the range of 0-4K
 *     VSI - Virtual Switching Instance is a generalization of the VLAN concept used primarily in advanced bridging 
 *     application. A VSI interconnects Logical Interfaces (LIFs).
 *     VSI is a logical partition of the MAC table and a flooding domain (comprising its member interfaces).
 *     For more information about VSI , see the Programmer's
 *     Guide PP document.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Write to HW VSI table.
 * \remark 
 *  
 */
static shr_error_e
bcm_dnx_vlan_table_default_set(
    int unit,
    bcm_vlan_t vid)
{
    uint32 entry_handle_id;
    int vsi;

    SHR_FUNC_INIT_VARS(unit);

    vsi = vid;

    /*
     * Write to HW VSI table 
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_ING_VSI_INFO, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VSI, INST_SINGLE, vsi);
    /*
     *    * DBAL Field STP topology ID has convertion based on HW device.
     *    * See DBAL Field STP topology ID logical to physical convertion for full details.
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                 DBAL_RESULT_TYPE_ING_VSI_INFO_BASIC_FORMAT);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_STP_TOPOLOGY_ID, INST_SINGLE, BCM_STG_DEFAULT);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_MY_MAC_PREFIX, INST_SINGLE, 0);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Create a VSI and set its default values 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit. 
 *   \param [in] vid -
 *     The incoming VSI ID, must be in the range of 0-4K.
 *     VSI ID 0 is invalid value.
 *     VSI - Virtual Switching Instance is a generalization of the VLAN concept used primarily in advanced bridging 
 *     application. A VSI interconnects Logical Interfaces (LIFs).
 *     VSI is a logical partition of the MAC table and a flooding domain (comprising its member interfaces).
 *     For more information about VSI , see the PG PP document.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: VSI already created or input VSI range value is incorrect
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Write to HW VSI table, Update Allocation-MNGR of VSI ID.
 * \remark
 *  API is avaiable only for the first 4K VSIs, in case a VSI larger than 4K then use bcm_vswitch_create. \n
 *  In addition, note that by default VSI is added to the default STG ID (Spanning Tree Group). \n
 *  STG attribute used to filter incoming and outgoing packets according to Port, VSI and STP-state. \n
 *  For more information see bcm_stg_xxx APIs.
 */
int
bcm_dnx_vlan_create(
    int unit,
    bcm_vlan_t vid)
{
    int vsi;

    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_vlan_create_verify(unit, vid));

    vsi = vid;
    /*
     * DNX SW Algorithm, allocate VSI.
     * dnx_algo_res_allocate() will return with error if resource for specified VSI has already been allocated.
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_allocate(unit, BCM_CORE_ALL, "VSI", DNX_ALGO_RES_ALLOCATE_WITH_ID, NULL, &vsi));

    /*
     * Write to HW VSI table 
     */
    SHR_IF_ERR_EXIT(bcm_dnx_vlan_table_default_set(unit, vsi));

    /*
     * For now skip STG addition and multicast removal
     */

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - TX Tag get functionality , given core_id and pp port return if TX tag is valid and its vid
 *
 */
static shr_error_e
dnx_vlan_tx_tag_get(
    int unit,
    int core_id,
    int pp_port,
    uint32 * tx_tag_vid,
    uint32 * tx_tag_valid)
{
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, pp_port);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, core_id);

    dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VALID, INST_SINGLE, tx_tag_valid);
    dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VID, INST_SINGLE, tx_tag_vid);
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *   Verify vlanId parameter for BCM-API: bcm_dnx_vlan_port_add()
 */
static shr_error_e
dnx_vlan_port_add_verify(
    int unit,
    bcm_vlan_t vid,
    bcm_pbmp_t * pbmp_pp_arr,
    bcm_pbmp_t * ubmp_pp_arr)
{
    uint32 tx_tag_vid;
    uint32 tx_tag_valid;
    int core_id;
    int pp_port_i;

    SHR_FUNC_INIT_VARS(unit);

    BCM_DNX_VLAN_CHK_ID(unit, vid);

    /*
     * Verify TX tag functionality for certain VID x UBMP
     * Illegal: Port is already legal untag on VID X and now required also on VID Y
     */
    for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
    {
        BCM_PBMP_ITER(pbmp_pp_arr[core_id], pp_port_i)
        {
            SHR_IF_ERR_EXIT(dnx_vlan_tx_tag_get(unit, core_id, pp_port_i, &tx_tag_vid, &tx_tag_valid));

            if (tx_tag_valid && BCM_PBMP_MEMBER(ubmp_pp_arr[core_id], pp_port_i) && tx_tag_vid != vid)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "Local pp port 0x%x core-id %d is already tagged on vid 0x%x \r\n",
                             pp_port_i, core_id, tx_tag_vid);
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - VLAN membership set functionality
 * Given vid , set the VLAN membership IF bitmap
 * The Procedure will commit to HW(DBAL) to VLAN membership table.
 * The functionality is assumed to be symmetric between Ingress and Egress.
 */
static shr_error_e
dnx_vlan_membership_set(
    int unit,
    int vid,
    bcm_pbmp_t vlan_membership_if_pbmp)
{
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);

    LOG_INFO_EX(BSL_LOG_MODULE, "VLAN Membership: vid %d  bitmap 0 0x%x bitmap 1 0x%x %s\n", vid,
                vlan_membership_if_pbmp.pbits[0], vlan_membership_if_pbmp.pbits[1], EMPTY);

    /*
     * Write to Ingress VLAN_BMP table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_VLAN_BMP, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_ID, INST_SINGLE, vid);
    dbal_entry_value_field_arr32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_MEMBER_DOMAIN_BMP, INST_SINGLE,
                                     vlan_membership_if_pbmp.pbits);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

    /*
     * Write to Egress VLAN_BMP table
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_VLAN_BMP, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_ID, INST_SINGLE, vid);
    dbal_entry_value_field_arr32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_MEMBER_DOMAIN_BMP, INST_SINGLE,
                                     vlan_membership_if_pbmp.pbits);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - TX Tag functionality
 * Given core_id and pp_port, set the Egress TX TAG / unTAG fucntioanlity.
 * TX Tag/Untag fucntionality will decide if to keep the Outer Tag or to remove it when going out to the out-port interface.
 * The functionality is done according to out-port (i.e. core-id and pp_port).
 * The HW has two fields, 
 * tx_tag_valid - to do the functionality or not 
 * tx_tag_vid - which VID to use in case it is invalid.
 */
static shr_error_e
dnx_vlan_tx_tag_set(
    int unit,
    int core_id,
    int pp_port,
    uint32 tx_tag_vid,
    uint32 tx_tag_valid)
{
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);

    LOG_INFO_EX(BSL_LOG_MODULE, "TX TAG : core_id %d  out_pp_port 0x%x tx_untag_valid %d tx_untag_vid 0x%x \n", core_id,
                pp_port, tx_tag_valid, tx_tag_vid);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_PORT, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, pp_port);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, core_id);

    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VALID, INST_SINGLE, tx_tag_valid);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VID, INST_SINGLE, tx_tag_vid);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - This procedure converts an array of pbmp of pp ports
 *        per core to TX tag valid per PP port
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] pbmp_pp_arr - Input param. Given the array of
 *          pbmp of pp ports per core.
 *   \param [in] tx_tag_pp_arr - Output param. Indicates which
 *          pp ports the functionality of TX tag is valid.
 *   
 * \par INDIRECT INPUT:
 * \par DIRECT OUTPUT:
 *   shr_error_e Error handling 
 * \par INDIRECT OUTPUT
 *   * tx_tag_pp_arr See DIRECT_INPUT.
 *   * Clears valid TX tag from HW(DBAL) Egress-Port table
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_vlan_convert_pp_pbmp_to_tx_tag_valid_pbmp(
    int unit,
    bcm_pbmp_t * pbmp_pp_arr,
    bcm_pbmp_t * tx_tag_pp_arr)
{
    int core_id;
    int pp_port;
    uint32 tx_tag_valid;
    uint32 tx_tag_vid;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(pbmp_pp_arr, _SHR_E_PARAM, "pbmp_pp_arr");
    SHR_NULL_CHECK(tx_tag_pp_arr, _SHR_E_PARAM, "tx_tag_pp_arr");

    for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
    {
        BCM_PBMP_CLEAR(tx_tag_pp_arr[core_id]);
        /*
         * convert pp port bmp(bitmap) to tx_tag pp bmp, required per core
         */
        BCM_PBMP_ITER(pbmp_pp_arr[core_id], pp_port)
        {
            SHR_IF_ERR_EXIT(dnx_vlan_tx_tag_get(unit, core_id, pp_port, &tx_tag_vid, &tx_tag_valid));
            if (tx_tag_valid)
            {
                BCM_PBMP_PORT_ADD(tx_tag_pp_arr[core_id], pp_port);
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Convert an array of pbmp of pp port for each core to VLAN Membership IF bitmap \n
 * Input: pbmp_pp_arr - Array of PP port bitmap per core \n
 * Output: VLAN membership interface bitmap (shared between cores) \n
 * Note: VLAN Membership IF bitmap is shared between cores and not per core
 *
 */
static shr_error_e
dnx_vlan_pbmp_pp_to_vlan_mem_if_bmp(
    int unit,
    bcm_pbmp_t * pbmp_pp_arr,
    bcm_pbmp_t * vlan_mem_if_bmp)
{
    int vlan_mem_if;
    uint32 class_id;
    bcm_gport_t local_port_i;
    bcm_port_t pp_port_i;
    int core_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(pbmp_pp_arr, _SHR_E_PARAM, "pbmp_pp_arr");
    SHR_NULL_CHECK(vlan_mem_if_bmp, _SHR_E_PARAM, "vlan_mem_if_bmp");

    BCM_PBMP_CLEAR(*vlan_mem_if_bmp);
    /*
     * The below loop will go over each core and update the related HW vlan_mem_domain
     */

    for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
    {
        BCM_PBMP_ITER(pbmp_pp_arr[core_id], pp_port_i)
        {
            SHR_IF_ERR_EXIT(algo_port_pp_to_local_port(unit, core_id, pp_port_i, &local_port_i));
            SHR_IF_ERR_EXIT(bcm_dnx_port_class_get(unit, local_port_i, bcmPortClassIngress, &class_id));
            vlan_mem_if = class_id;
            BCM_PBMP_PORT_ADD(*vlan_mem_if_bmp, vlan_mem_if);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * This API sets VLAN port membership using VLAN ID and a set of ports. \n
 * The input pbmp will also be updated to be the egress Multicast VLAN group ID (TBD). \n
 * In addition, HW is set to either keep tag or to remove it when packets go out of specified ports.
 * This option is selected based on input 'ubmp'.
 * 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit. 
 *   \param [in] vid -
 *     VLAN Identifier.
 *   \param [in] pbmp -
 *     logical ports bitmap. Assume symmetric (Ingress & Egress) configuration 
 *   \param [in] ubmp -
 *     logical ports bitmap that should be untagged when outgoing packet is transmitted with vid
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: ubmp invalid cases.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Update HW Ingress and Egress VLAN-Port membership table
 *   * Update HW Egress outgoing port untag functionality.
 *   * Update HW Egress VLAN Multicast member (TBD)
 * \remark
 *   1. Unlike previous devices, outgoing port untag functionality is more limited. \n
 *   In previous DPP devices, configuration could allow for any Out-Port x VLAN to be untagged. \n
 *   In DNX devices, configuration allow only one VLAN per port to be untagged. \n
 *   Example: \n
 *      In DPP devices, port 13 can set VLAN 15 , VLAN 16 to be set untagged \n
 *      In DNX devices, it is not possible. Port 13 can set VLAN 15 or VLAN 16 to be untagged but not both.
 *   2. VLAN membership table is accessed not according to port but according to port mapping "VLAN membership IF". \n
 *   VLAN membership IF provides a namespace to VLAN membership from a port either physical (local port) or logical (LIF gport for example: PWE) \n
 *   This API only configure Simple-Bridge application and so relates only to the physical part of VLAN membership.
 *   For Logical port part see \ref bcm_vlan_gport_add.
 *   VLAN membership IF is updated according to \ref bcm_port_class_set. \n
 *   This API only assume it gets the information that was configurd before. \n
 *   Note: In case VLAN membership IF mapping is updated there is no automatic update of the VLAN membership table. This is the responsibility of the user. 
 */
shr_error_e
bcm_dnx_vlan_port_add(
    int unit,
    bcm_vlan_t vid,
    bcm_pbmp_t pbmp,
    bcm_pbmp_t ubmp)
{
    int core_id;
    bcm_port_t pp_port_i;
    bcm_pbmp_t vlan_mem_if_bmp;
    bcm_pbmp_t pbmp_pp_arr[DNX_DATA_MAX_DEVICE_GENERAL_NOF_CORES];
    bcm_pbmp_t ubmp_pp_arr[DNX_DATA_MAX_DEVICE_GENERAL_NOF_CORES];
    bcm_pbmp_t tx_tag_pp_arr[DNX_DATA_MAX_DEVICE_GENERAL_NOF_CORES];

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Retrieve information before verify:
     * Local pp ports according to pbmp, ubmp input params
     * Existance of TX-Tag on pp ports which are indicated on ubmp
     */
    /*
     * Map pbmp, ubmp to local pp port pbmp
     */
    SHR_IF_ERR_EXIT(algo_port_pbmp_to_pp_pbmp(unit, pbmp, pbmp_pp_arr));
    SHR_IF_ERR_EXIT(algo_port_pbmp_to_pp_pbmp(unit, ubmp, ubmp_pp_arr));
    /*
     * Retrieve TX tag valid local pp pbmp
     */
    SHR_IF_ERR_EXIT(dnx_vlan_convert_pp_pbmp_to_tx_tag_valid_pbmp(unit, pbmp_pp_arr, tx_tag_pp_arr));

    /*
     * Verify is done after convert of bitmaps in order to validate better that the ubmp is provided as expected.
     * See more in the verify function.
     */
    SHR_INVOKE_VERIFY_DNX(dnx_vlan_port_add_verify(unit, vid, ubmp_pp_arr, tx_tag_pp_arr));

    /*
     * Set VLAN membership functionality
     * Convert to VLAN membership IF and commit
     * For VLAN membership IF informaiton , see remark number 2 in Procedure documentation.
     */
    SHR_IF_ERR_EXIT(dnx_vlan_pbmp_pp_to_vlan_mem_if_bmp(unit, pbmp_pp_arr, &vlan_mem_if_bmp));
    SHR_IF_ERR_EXIT(dnx_vlan_membership_set(unit, vid, vlan_mem_if_bmp));

    /*
     * Set Tx Tag functionality
     */
    {
        for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
        {
            BCM_PBMP_ITER(pbmp_pp_arr[core_id], pp_port_i)
            {
                /*
                 * Disable TX tag in case port was TX-tag valid and now it is not.
                 * BCM_PBMP_MEMBER(tx_tag_pp_arr[core_id], pp_port_i) indicates whether port 'pp_port_i' was 'TX-tag valid'. 
                 * BCM_PBMP_MEMBER(ubmp_pp_arr[core_id], pp_port_i) indicates whether port is now set to 'NOT TX-tag valid'.
                 */
                if (BCM_PBMP_MEMBER(tx_tag_pp_arr[core_id], pp_port_i)
                    && !BCM_PBMP_MEMBER(ubmp_pp_arr[core_id], pp_port_i))
                {
                    SHR_IF_ERR_EXIT(dnx_vlan_tx_tag_set(unit, core_id, pp_port_i, 0, FALSE));
                }
                /*
                 * Enable TX tag in case port is ubmp (not matter if it was or currently is)
                 */
                if (BCM_PBMP_MEMBER(ubmp_pp_arr[core_id], pp_port_i))
                {
                    SHR_IF_ERR_EXIT(dnx_vlan_tx_tag_set(unit, core_id, pp_port_i, vid, TRUE));
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - VLAN membership get functionality
 * Given vid, get the VLAN membership IF bitmap 
 * The Procedure will get information from HW(DBAL) VLAN membership table.
 * The functionality is assumed to be symmetric between Ingress and Egress, so we get information from Ingress.
 */
static shr_error_e
dnx_vlan_membership_get(
    int unit,
    int vid,
    bcm_pbmp_t * vlan_membership_if_pbmp)
{
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);
    SHR_NULL_CHECK(vlan_membership_if_pbmp, _SHR_E_PARAM, "vlan_membership_if_pbmp");
    BCM_PBMP_CLEAR(*vlan_membership_if_pbmp);
    /*
     * Get from Ingress VLAN_BMP table information for the vlan membership if pbmp
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_VLAN_BMP, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_ID, INST_SINGLE, vid);
    dbal_entry_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_VLAN_MEMBER_DOMAIN_BMP, INST_SINGLE,
                                     vlan_membership_if_pbmp->pbits);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Convert an array of vlan membership bitmap IF to pbmp of pp port for each core \n
 * Input: VLAN membership interface bitmap (shared between cores) \n pbmp_pp_arr - Array of PP port bitmap per core \n
 * Output:  pbmp_pp_arr - Array of PP port bitmap per core \n
 * Note: VLAN Membership IF bitmap is shared between cores and not per core
 * 
 */
#ifdef port_class_get
static shr_error_e
dnx_vlan_mem_if_bmp_to_pbmp_pp(
    int unit,
    bcm_pbmp_t vlan_mem_if_bmp,
    bcm_pbmp_t * pbmp_pp_arr)
{
    int port_i;
    uint32 class_id;
    int vlan_member;
    /*
     * bitmap of the ports associated with the VLAN membership IF
     */
    bcm_pbmp_t vlan_mem_bmp;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(pbmp_pp_arr, _SHR_E_PARAM, "pbmp_pp_arr");

    BCM_PBMP_CLEAR(*pbmp_pp_arr);
    BCM_PBMP_CLEAR(vlan_mem_bmp);
    
    for(port_i = 0; port_i < 512; port_i++)
    {
        /*
         *  The below loop will go over each element of the vlan_mem_domain and update pbmp_pp_arr per core
         */
        BCM_PBMP_ITER(vlan_mem_if_bmp, vlan_member)
        {
            SHR_IF_ERR_EXIT(bcm_dnx_port_class_get(unit, port_i, bcmPortClassIngress, &class_id));
            if (vlan_member == class_id)
                BCM_PBMP_PORT_ADD(vlan_mem_bmp, port_i);
        }
    }
    /*
     * Convert the vlan_mem_bmp to array of physical pp ports
     */
    SHR_IF_ERR_EXIT(algo_port_pbmp_to_pp_pbmp(unit, vlan_mem_bmp, pbmp_pp_arr));

exit:
    SHR_FUNC_EXIT;
}
#endif

/**
 * \brief - This procedure converts an array of pbmp of pp ports
 *        per core to TX tag valid per PP port
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] vid - Input param Cheks if the tx_tag_vid matches the given vid
 *   \param [in] pbmp_pp_arr - Input param. Given the array of
 *          pbmp of pp ports per core.
 *   \param [in] ubmp_pp_arr - Output param. Indicates which
 *          pp ports the functionality of TX tag is valid .
 *   
 * \par INDIRECT INPUT:
 * \par DIRECT OUTPUT:
 *   shr_error_e Error handling 
 * \par INDIRECT OUTPUT
 *   * ubmp_pp_arr See DIRECT_INPUT.
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_vlan_convert_pp_pbmp_to_ubmp(
    int unit,
    bcm_vlan_t vid,
    bcm_pbmp_t * pbmp_pp_arr,
    bcm_pbmp_t * ubmp_pp_arr)
{
    int core_id;
    int pp_port;
    uint32 tx_tag_valid;
    uint32 tx_tag_vid;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(pbmp_pp_arr, _SHR_E_PARAM, "pbmp_pp_arr");
    SHR_NULL_CHECK(ubmp_pp_arr, _SHR_E_PARAM, "ubmp_pp_arr");

    for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
    {
        BCM_PBMP_CLEAR(ubmp_pp_arr[core_id]);
        /*
         * convert pp port bmp(bitmap) to tx_tag pp bmp, required per core
         */
        BCM_PBMP_ITER(pbmp_pp_arr[core_id], pp_port)
        {
            SHR_IF_ERR_EXIT(dnx_vlan_tx_tag_get(unit, core_id, pp_port, &tx_tag_vid, &tx_tag_valid));
            if (tx_tag_valid & (vid == tx_tag_vid))
            {
                BCM_PBMP_PORT_ADD(ubmp_pp_arr[core_id], pp_port);
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * This API removes ports from the VLAN port membership \n
 * the removal is done based on the incoming vid and pbmp \n
 * The pbmp contains the ports, to be removed. \n
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit. 
 *   \param [in] vid -
 *     VLAN Identifier.
 *   \param [in] pbmp -
 *     logical ports bitmap, containing the relevant ports to be removed.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: pbmp invalid cases.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * The egress transmit is updated to be untagged for all removed ports.
 *   * The vlan membership if is updated according to the removed ports.
 * \remark
 *    vlan_mem_if_bmp is decided according to Ingress table since it is symmetric between Ingress & Egress.
 *    The removal is done for both tagged and untagged functionality.
 *    The removal is symmetric of the VLAN port membership for both Ingress and Egress.
 */
shr_error_e
bcm_dnx_vlan_port_remove(
    int unit,
    bcm_vlan_t vid,
    bcm_pbmp_t pbmp)
{

    bcm_pbmp_t vlan_mem_if_bmp;
    bcm_pbmp_t vlan_mem_if_bmp_pbmp;
    bcm_pbmp_t vlan_mem_if_bmp_neg;
    bcm_pbmp_t pbmp_pp_arr[DNX_DATA_MAX_DEVICE_GENERAL_NOF_CORES];
    bcm_pbmp_t ubmp_pp_arr[DNX_DATA_MAX_DEVICE_GENERAL_NOF_CORES];
    int pp_port_i;
    int core_id;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Retrieve information of the ingress vlan membership interface pbmp, based on vid
     */
    SHR_IF_ERR_EXIT(dnx_vlan_membership_get(unit, vid, &vlan_mem_if_bmp));

    /*
     * Map the incoming pbmp to local pp port pbmp and convert it to vlan membership bitmap
     */
    SHR_IF_ERR_EXIT(algo_port_pbmp_to_pp_pbmp(unit, pbmp, pbmp_pp_arr));
    SHR_IF_ERR_EXIT(dnx_vlan_pbmp_pp_to_vlan_mem_if_bmp(unit, pbmp_pp_arr, &vlan_mem_if_bmp_pbmp));
    
    /*
     * Retrieve information of the physical ports, associated to the vlan membership, which have tx tag enabled
     * The function will return array of physical ubmp per core.
     */
    SHR_IF_ERR_EXIT(dnx_vlan_convert_pp_pbmp_to_ubmp(unit, vid, pbmp_pp_arr, ubmp_pp_arr));

    /*
     * Get the negative of the input pbmp, perform logical AND with the vlan membership pbmp.
     * This will reside in pbmp, containing only the ports, which should be set on the device.
     */

    BCM_PBMP_NEGATE(vlan_mem_if_bmp_neg, vlan_mem_if_bmp_pbmp); 
    BCM_PBMP_AND(vlan_mem_if_bmp, vlan_mem_if_bmp_neg);

    /*
    * Update the vlan membership for both ingress and egress vlan membership table.
    */
    SHR_IF_ERR_EXIT(dnx_vlan_membership_set(unit, vid, vlan_mem_if_bmp));

    /*
     * Set egress transmit to be untagged for all removed ports, for the given VID, if the tx tag was set to be valid for them.
     */
    for (core_id = 0; core_id < dnx_data_device.general.nof_cores_get(unit); core_id++)
    {
        BCM_PBMP_ITER(ubmp_pp_arr[core_id], pp_port_i)
        {
            SHR_IF_ERR_EXIT(dnx_vlan_tx_tag_set(unit, core_id, pp_port_i, 0, FALSE));
        }
    }

exit:
  SHR_FUNC_EXIT;
}

/**
 * \brief
 * This API gets VLAN port membership using VLAN ID \n
 * Based on the input vid the API gets information for the vlan membership \n
 * From the vlan membership if pbmp get the pbmp and the ubmp.
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit. 
 *   \param [in] vid -
 *     VLAN Identifier.
 *   \param [in] pbmp -
 *     logical ports bitmap. Assume symmetric (Ingress & Egress) configuration 
 *   \param [in] ubmp -
 *     logical ports bitmap that should be untagged when outgoing packet is transmitted with vid
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: ubmp invalid cases.
 *   \retval Zero in case of NO ERROR
 *   \retval pbmp - port bitmap of the membered ports 
 *   \retval ubmp - port bitmap of the membered ports, which has tx_tag valid
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *    pbmp is decided according to Ingress table since it is symmetric between Ingress & Egress.
 */
shr_error_e
bcm_dnx_vlan_port_get(
    int unit,
    bcm_vlan_t vid,
    bcm_pbmp_t * pbmp,
    bcm_pbmp_t * ubmp)
{
    bcm_pbmp_t vlan_mem_if_bmp;
    bcm_pbmp_t pbmp_pp_arr[DNX_DATA_MAX_DEVICE_GENERAL_NOF_CORES];
    bcm_pbmp_t ubmp_pp_arr[DNX_DATA_MAX_DEVICE_GENERAL_NOF_CORES];

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Retrieve information of the vlan membership if port bitmap based on given vid
     */
    SHR_IF_ERR_EXIT(dnx_vlan_membership_get(unit, vid, &vlan_mem_if_bmp));

    /*
     * Retrieve information of the physical ports, associated to the vlan membership
     * The function will return array of physical pbmp per core.
     */
/*    SHR_IF_ERR_EXIT(dnx_vlan_mem_if_bmp_to_pbmp_pp(unit, vlan_mem_if_bmp, pbmp_pp_arr)); */

    /*
     * Retrieve information of the physical ports, associated to the vlan membership, which have tx tag enabled
     * The function will return array of physical ubmp per core.
     */
    SHR_IF_ERR_EXIT(dnx_vlan_convert_pp_pbmp_to_ubmp(unit, vid, pbmp_pp_arr, ubmp_pp_arr));

    /*
     * Map pbmp_pp_arr and ubmp_pp_arr to pbmp and ubmp
     */
    SHR_IF_ERR_EXIT(algo_port_pp_pbmp_to_local_pbmp(unit, pbmp_pp_arr, pbmp));

    SHR_IF_ERR_EXIT(algo_port_pp_pbmp_to_local_pbmp(unit, ubmp_pp_arr, ubmp));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Destroy an allocated VSI 
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit. 
 *   \param [in] vid -
 *     The incoming VSI ID, must be in the range of 0-4K.
 *     VSI ID 0 is invalid parameter and VSI ID 1 is default VSI.
 *     VSI - Virtual Switching Instance is a generalization of the VLAN concept used primarily in advanced bridging 
 *     application. A VSI interconnects Logical Interfaces (LIFs).
 *     VSI is a logical partition of the MAC table and a flooding domain (comprising its member interfaces).
 *     For more information about VSI , see the PG PP document.
 * \par INDIRECT INPUT: 
*   * Allocation-MNGR of VSI ID.
 * \par DIRECT OUTPUT: 
 *  Negative in case of an error. See shr_error_e, for
 *           example: VSI was not created or input VSI range
 *           value is incorrect
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Write to HW VSI table, allocation-MNGR of VSI ID.
 * \remark 
 *  
 */
int
bcm_dnx_vlan_destroy(
    int unit,
    bcm_vlan_t vid)
{

    int vsi;
    uint8 is_allocated;
    bcm_pbmp_t pbmp;
    bcm_pbmp_t ubmp;
    SHR_FUNC_INIT_VARS(unit);

    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_CLEAR(ubmp);

    SHR_INVOKE_VERIFY_DNX(dnx_vlan_destroy_verify(unit, vid));
    vsi = vid;

    /*
     * Check that the vsi was allocated:
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_is_allocated(unit, BCM_CORE_ALL, "VSI", vsi, &is_allocated));

    if (is_allocated == FALSE)
    {
        SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "vsi %d doesn't exist\n", vsi);
    }

   /*
    * Clear vlan membership
    */
    SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_get(unit, vid, &pbmp, &ubmp));
    SHR_IF_ERR_EXIT(bcm_dnx_vlan_port_remove(unit, vid, pbmp));

    /*
     * Write back defaults to HW VSI table 
     */
    SHR_IF_ERR_EXIT(bcm_dnx_vlan_table_default_set(unit, vsi));

    /*
     * DNX SW Algorithm, de-allocate VSI 
     */
    SHR_IF_ERR_EXIT(dnx_algo_res_free(unit, BCM_CORE_ALL, "VSI", vsi));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Destroy all allocated VSIs
 *
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit. 
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Write to HW VSI table, allocation-MNGR of all but default
 *     VSI IDs.
 * \remark 
 *   * None
 * \see
 *   * bcm_dnx_vlan_destroy
 *  
 */
int
bcm_dnx_vlan_destroy_all(
    int unit)
{

    int vsi;
    int default_vsi;
    uint8 is_allocated;

    SHR_FUNC_INIT_VARS(unit);

    
    default_vsi = BCM_VLAN_DEFAULT;

    for (vsi = BCM_VLAN_MIN; vsi < BCM_VLAN_COUNT; vsi++)
    {
        if (BCM_VLAN_VALID(vsi))
        {
            /*
             * Check whether the vsi was allocated: 
             */
            SHR_IF_ERR_EXIT(dnx_algo_res_is_allocated(unit, BCM_CORE_ALL, "VSI", vsi, &is_allocated));

            if ((is_allocated == TRUE) && (vsi != default_vsi))
            {
                SHR_IF_ERR_EXIT(bcm_dnx_vlan_destroy(unit, vsi));
            }
        }
        else
        {
            continue;
        }
    }

exit:
    SHR_FUNC_EXIT;
}
int
bcm_dnx_vlan_default_get(
    int unit, 
    bcm_vlan_t *vid_ptr)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_ERR_EXIT(_SHR_E_UNAVAIL, "bcm_dnx_vlan_default_get not supported ");

exit:
  SHR_FUNC_EXIT;
}

int
bcm_dnx_vlan_default_set(
    int unit, 
    bcm_vlan_t vid)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_ERR_EXIT(_SHR_E_UNAVAIL, "bcm_dnx_vlan_default_set not supported ");

exit:
  SHR_FUNC_EXIT;
}
