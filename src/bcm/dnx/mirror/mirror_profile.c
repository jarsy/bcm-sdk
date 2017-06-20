/** \file mirror_profile.c
 * $Id$
 *
 * Internal MIRROR profile functionality for DNX. \n
 * This file handles mirror profiles (allocation, creation, retrieval, destruction, etc..)
 * 
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_MIRROR

/*
 * Include files.
 * {
 */
#include <shared/utilex/utilex_bitstream.h>
#include <shared/shrextend/shrextend_error.h>
#include <soc/dnx/dbal/dbal.h>
#include <soc/dnx/dnx_data/dnx_data_device.h>
#include <soc/dnx/dnx_data/dnx_data_snif.h>
#include <soc/dnx/legacy/cosq.h>
#include <bcm/types.h>
#include <bcm_int/dnx/mirror/mirror.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <bcm_int/dnx/algo/algo_port.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>
#include <bcm_int/dnx/legacy/gport_mgmt.h>
#include "mirror_profile.h"

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
 * \brief - Roll back snif profile back to default values. 
 * By default all values expected to be 0. 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit -  Unit ID 
 *   \param [in] mirror_dest_id - gport of mirror profile
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Mirror related HW. See dbal tables: \n
 *     DBAL_TABLE_SNIF_COMMAND_TABLE, DBAL_TABLE_SNIF_COUNTERS_TABLE
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_mirror_profile_hw_defaults_set(
    int unit,
    bcm_gport_t mirror_dest_id)
{
    int action_profile_id, snif_type;
    uint32 entry_handle_id;

    SHR_FUNC_INIT_VARS(unit);

    action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest_id);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SNIF_COMMAND_TABLE, &entry_handle_id));

    /*
     * Clear HW(DBAL SNIF command table). 
     * Choose table key according to application type (mirror, snoop, sampling)
     */
    if (BCM_GPORT_IS_MIRROR_SNOOP(mirror_dest_id))
    {
        snif_type = DBAL_ENUM_FVAL_SNIF_TYPE_SNOOP;
    }
    else
    {
        snif_type = DBAL_ENUM_FVAL_SNIF_TYPE_MIRROR;
    }

    /*
     * key construction
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SNIF_COMMAND_ID, INST_SINGLE, action_profile_id);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SNIF_TYPE, INST_SINGLE, snif_type);

    SHR_IF_ERR_EXIT(dbal_entry_clear(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Read mirror attributes from HW (dbal)
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] action_profile_id - Mirror profile ID
 *   \param [in] mirror_dest - Mirror profile attributes
 *   
 * \par INDIRECT INPUT:
 *   * Mirror related HW, see dbal tables:
 *     
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Mirror related HW. See dbal tables: \n
 *     DBAL_TABLE_SNIF_COMMAND_TABLE, DBAL_TABLE_SNIF_COUNTERS_TABLE
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_mirror_profile_hw_get(
    int unit,
    int action_profile_id,
    bcm_mirror_destination_t * mirror_dest)
{
    uint32 entry_handle_id, array32[1];
    uint32 snif_type, dest, snif_probability, crop_enable, tc, tc_ow, dp, dp_ow, in_pp_port, in_pp_port_ow,
        st_vsq_ptr, st_vsq_ptr_ow, lag_lb_key, lag_lb_key_ow, admit_profile, admit_profile_ow, dest_val;
    dbal_table_field_info_t field_info;
    dbal_fields_e dbal_dest_type;
    DNX_TMC_DEST_INFO dest_info;

    SHR_FUNC_INIT_VARS(unit);

    entry_handle_id = 0;

    /*
     * Read values from HW(DBAL SNIF command table). 
     * Choose table key according to application type (mirror, snoop, sampling)
     */
    {
        if (mirror_dest->flags & BCM_MIRROR_DEST_IS_SNOOP)
        {
            snif_type = DBAL_ENUM_FVAL_SNIF_TYPE_SNOOP;
        }
        else
        {
            snif_type = DBAL_ENUM_FVAL_SNIF_TYPE_MIRROR;
        }

        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SNIF_COMMAND_TABLE, &entry_handle_id));
        /*
         * key construction
         */
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SNIF_COMMAND_ID, INST_SINGLE, action_profile_id);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SNIF_TYPE, INST_SINGLE, snif_type);

        /*
         * read all table values
         */
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_DESTINATION, INST_SINGLE, &dest);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_SNIF_PROBABILITY, INST_SINGLE,
                                     &snif_probability);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_CROP_ENABLE, INST_SINGLE, &crop_enable);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TC_OW, INST_SINGLE, &tc_ow);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_TC, INST_SINGLE, &tc);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_DP_OW, INST_SINGLE, &dp_ow);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_DP, INST_SINGLE, &dp);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_IN_PP_PORT_OW, INST_SINGLE, &in_pp_port_ow);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_IN_PP_PORT, INST_SINGLE, &in_pp_port);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_ST_VSQ_PTR_OW, INST_SINGLE, &st_vsq_ptr_ow);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_ST_VSQ_PTR, INST_SINGLE, &st_vsq_ptr);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_LAG_LB_KEY_OW, INST_SINGLE, &lag_lb_key_ow);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_LAG_LB_KEY, INST_SINGLE, &lag_lb_key);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_ADMIT_PROFILE_OW, INST_SINGLE,
                                     &admit_profile_ow);
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_ADMIT_PROFILE, INST_SINGLE, &admit_profile);

        SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }

    /*
     * After all values were read from HW, start filling the mirror_dest struct accordingly
     */
    {
        /*
         * Destination field. 
         * Decode destination field as it has sub fields
         */
        {
            SHR_IF_ERR_EXIT(dbal_fields_sub_field_info_get
                            (unit, DBAL_FIELD_DESTINATION, dest, &dbal_dest_type, &dest_val));
            DNX_TMC_DEST_INFO_clear(&dest_info);
            dest_info.dbal_type = dbal_dest_type;
            dest_info.id = dest_val;
            SHR_IF_ERR_EXIT(_bcm_dnx_gport_from_tm_dest_info(unit, &(mirror_dest->gport), &dest_info));
        }

        /*
         * Crop size field. 
         * Set crop size: 
         * Crop enable - only 256B are cropped 
         * Crop disable - whole packet is copied 
         */
        if (crop_enable)
        {
            mirror_dest->packet_copy_size = 256;
        }
        else
        {
            mirror_dest->packet_copy_size = 0;
        }

        /*
         * Probability field. 
         * Set divisor to maximal value for better precision. 
         */
        {
            /** dividend */
            mirror_dest->sample_rate_dividend = snif_probability ? snif_probability + 1 : 0;
            /** divisor - set to maximal value */
            SHR_IF_ERR_EXIT(dbal_table_field_info_get(unit, DBAL_TABLE_SNIF_COMMAND_TABLE, DBAL_FIELD_SNIF_PROBABILITY,
                                                      FALSE, 0, &field_info));
            SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(array32, 0, (field_info.field_nof_bits - 1)));
            mirror_dest->sample_rate_divisor = array32[0];
        }

        /*
         * Packet control fields. Each field has a corresponding override field. 
         * If the field is not overwritten, then the snif copy gets the value from the original packet (forward copy). 
         * Otherwise, if the field is overwritten, then the value should be indicated.                                                                                                          .
         */
        mirror_dest->packet_control_updates.valid = 0;
        /** TC field */
        if (tc_ow)
        {
            mirror_dest->packet_control_updates.prio = tc;
            mirror_dest->packet_control_updates.valid |= BCM_MIRROR_PKT_HEADER_UPDATE_PRIO;
        }
        /** DP field */
        if (dp_ow)
        {
            mirror_dest->packet_control_updates.color = dp;
            mirror_dest->packet_control_updates.valid |= BCM_MIRROR_PKT_HEADER_UPDATE_COLOR;
        }
        /** in pp port field */
        if (in_pp_port_ow)
        {
            mirror_dest->packet_control_updates.in_port = in_pp_port;
            mirror_dest->packet_control_updates.valid |= BCM_MIRROR_PKT_HEADER_UPDATE_IN_PORT;
        }
        /** statistics VSQ ptr field */
        if (st_vsq_ptr_ow)
        {
            mirror_dest->packet_control_updates.vsq = st_vsq_ptr;
            mirror_dest->packet_control_updates.valid |= BCM_MIRROR_PKT_HEADER_UPDATE_VSQ;
        }
        /** trunk(LAG) hash field */
        if (lag_lb_key_ow)
        {
            mirror_dest->packet_control_updates.trunk_hash_result = lag_lb_key;
            mirror_dest->packet_control_updates.valid |= BCM_MIRROR_PKT_HEADER_UPDATE_TRUNK_HASH_RESULT;
        }
        /*
         * Admit profile field
         * Admit profile 0 - ECN disabled 
         * Admit profile 1 - ECN enabled 
         */
        if (admit_profile_ow)
        {
            if (admit_profile == 1)
            {
                mirror_dest->packet_control_updates.ecn_value = 1;
            }
            else
            {
                mirror_dest->packet_control_updates.ecn_value = 0;
            }

            mirror_dest->packet_control_updates.valid |= BCM_MIRROR_PKT_HEADER_UPDATE_ECN_VALUE;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Write mirror attributes to HW (dbal)
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit ID
 *   \param [in] mirror_dest - Mirror profile attributes
 *   \param [in] action_profile_id - profile ID
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Mirror related HW. See dbal tables: \n
 *     DBAL_TABLE_SNIF_COMMAND_TABLE, DBAL_TABLE_SNIF_COUNTERS_TABLE
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_mirror_profile_hw_set(
    int unit,
    bcm_mirror_destination_t * mirror_dest,
    int action_profile_id)
{
    uint32 snif_probability, nof_bits;
    uint32 entry_handle_id;
    int snif_type;
    dbal_table_field_info_t field_info;
    DNX_TMC_DEST_INFO dest_info;

    SHR_FUNC_INIT_VARS(unit);

    entry_handle_id = 0;
    /*
     * Calculate HW field for snif probability
     */
    SHR_IF_ERR_EXIT(dbal_table_field_info_get(unit, DBAL_TABLE_SNIF_COMMAND_TABLE, DBAL_FIELD_SNIF_PROBABILITY,
                                              FALSE, 0, &field_info));
    nof_bits = field_info.field_nof_bits;
    SHR_IF_ERR_EXIT(dnx_algo_mirror_probability_get(unit, mirror_dest->sample_rate_dividend,
                                                    mirror_dest->sample_rate_divisor, nof_bits, &snif_probability));

    /*
     * Write to HW (DBAL SNIF command table).
     * Choose table key according to application type (mirror, snoop, sampling)
     */
    if (mirror_dest->flags & BCM_MIRROR_DEST_IS_SNOOP)
    {
        snif_type = DBAL_ENUM_FVAL_SNIF_TYPE_SNOOP;
    }
    else
    {
        snif_type = DBAL_ENUM_FVAL_SNIF_TYPE_MIRROR;
    }

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SNIF_COMMAND_TABLE, &entry_handle_id));
    /*
     * key construction
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SNIF_COMMAND_ID, INST_SINGLE, action_profile_id);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SNIF_TYPE, INST_SINGLE, snif_type);
    /*
     * destination field. 
     * Convert destination gport to dbal type (port, queue, multicast, etc..) and ID. 
     */
    DNX_TMC_DEST_INFO_clear(&dest_info);
    SHR_IF_ERR_EXIT(_bcm_dnx_gport_to_tm_dest_info(unit, mirror_dest->gport, &dest_info));
    dbal_entry_value_field32_set(unit, entry_handle_id, dest_info.dbal_type, INST_SINGLE, dest_info.id);
    /*
     * Probability field
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_SNIF_PROBABILITY, INST_SINGLE, snif_probability);
    /*
     * Crop enable field. The size should be given in bytes. 
     * 256 - Crop 256B of the packet (crop enable)
     * 0 - No cropping (crop disable)
     */
    switch (mirror_dest->packet_copy_size)
    {
        case 0:
        {
            dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_CROP_ENABLE, INST_SINGLE, FALSE);
            break;
        }
        case 256:
        {
            dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_CROP_ENABLE, INST_SINGLE, TRUE);
            break;
        }
        default:
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "cropping size=%d is invalid", mirror_dest->packet_copy_size);
            break;
        }
    }

    /*
     * Packet control fields. Each field has a corresponding override field. 
     * If the field is not overwritten, then the snif copy gets the value from the original packet (forward copy). 
     * Otherwise, if the field is overwritten, then the value should be indicated.                                                                                                          .
     */
    /*
     * TC field 
     */
    if (mirror_dest->packet_control_updates.valid & BCM_MIRROR_PKT_HEADER_UPDATE_PRIO)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TC_OW, INST_SINGLE, TRUE);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TC, INST_SINGLE,
                                     mirror_dest->packet_control_updates.prio);
    }
    else
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TC_OW, INST_SINGLE, FALSE);
    }
    /*
     * DP field 
     */
    if (mirror_dest->packet_control_updates.valid & BCM_MIRROR_PKT_HEADER_UPDATE_COLOR)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DP_OW, INST_SINGLE, TRUE);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DP, INST_SINGLE,
                                     mirror_dest->packet_control_updates.color);
    }
    else
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_DP_OW, INST_SINGLE, FALSE);
    }
    /*
     * in pp port field
     */
    if (mirror_dest->packet_control_updates.valid & BCM_MIRROR_PKT_HEADER_UPDATE_IN_PORT)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_PP_PORT_OW, INST_SINGLE, TRUE);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_PP_PORT, INST_SINGLE,
                                     mirror_dest->packet_control_updates.in_port);
    }
    else
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_PP_PORT_OW, INST_SINGLE, FALSE);
    }
    /*
     * statistics VSQ ptr field
     */
    if (mirror_dest->packet_control_updates.valid & BCM_MIRROR_PKT_HEADER_UPDATE_VSQ)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ST_VSQ_PTR_OW, INST_SINGLE, TRUE);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ST_VSQ_PTR, INST_SINGLE,
                                     mirror_dest->packet_control_updates.vsq);
    }
    else
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ST_VSQ_PTR_OW, INST_SINGLE, FALSE);
    }
    /*
     * trunk(LAG) hash field
     */
    if (mirror_dest->packet_control_updates.valid & BCM_MIRROR_PKT_HEADER_UPDATE_TRUNK_HASH_RESULT)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LAG_LB_KEY_OW, INST_SINGLE, TRUE);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LAG_LB_KEY, INST_SINGLE,
                                     mirror_dest->packet_control_updates.trunk_hash_result);
    }
    else
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LAG_LB_KEY_OW, INST_SINGLE, FALSE);
    }

    /*
     * Admit profile field
     * Admit profile 0 - ECN disabled 
     * Admit profile 1 - ECN enabled 
     */
    if (mirror_dest->packet_control_updates.valid & BCM_MIRROR_PKT_HEADER_UPDATE_ECN_VALUE)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ADMIT_PROFILE_OW, INST_SINGLE, TRUE);
        if (mirror_dest->packet_control_updates.ecn_value > 0)
        {
            dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ADMIT_PROFILE, INST_SINGLE, 1);
        }
        else
        {
            dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ADMIT_PROFILE, INST_SINGLE, 0);
        }
    }
    else
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ADMIT_PROFILE_OW, INST_SINGLE, FALSE);
    }

    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_mirror_profile_create(
    int unit,
    bcm_mirror_destination_t * mirror_dest)
{
    int action_profile_id;

    SHR_FUNC_INIT_VARS(unit);

    action_profile_id = 0;

    /*
     * If Change to existing profile is required, no allocation is needed. 
     * Otherwise, allocate a new profile (for both with id and without id options)
     */
    if (mirror_dest->flags & BCM_MIRROR_DEST_REPLACE)
    {
        action_profile_id = BCM_GPORT_MIRROR_GET(mirror_dest->mirror_dest_id);
    }
    else
    {
        /** Allocate snif profile */
        SHR_IF_ERR_EXIT(dnx_algo_mirror_profile_allocate(unit, mirror_dest, &action_profile_id));

        /** Set gport according to snif type and allocated ID(action_profile_id) */
        if (mirror_dest->flags & BCM_MIRROR_DEST_IS_SNOOP)
        {
            BCM_GPORT_MIRROR_SNOOP_SET(mirror_dest->mirror_dest_id, action_profile_id);
        }
        else
        {
            BCM_GPORT_MIRROR_MIRROR_SET(mirror_dest->mirror_dest_id, action_profile_id);
        }
    }

    /** Configure allocated profile in HW */
    SHR_IF_ERR_EXIT(dnx_mirror_profile_hw_set(unit, mirror_dest, action_profile_id));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_mirror_profile_get(
    int unit,
    bcm_gport_t mirror_dest_id,
    bcm_mirror_destination_t * mirror_dest)
{
    SHR_FUNC_INIT_VARS(unit);

    /** retrieve Mirror attributes from HW */
    SHR_IF_ERR_EXIT(dnx_mirror_profile_hw_get(unit, mirror_dest_id, mirror_dest));

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 */
shr_error_e
dnx_mirror_profile_destroy(
    int unit,
    bcm_gport_t mirror_dest_id)
{
    SHR_FUNC_INIT_VARS(unit);

    /** Roll back snif profile to default values */
    SHR_IF_ERR_EXIT(dnx_mirror_profile_hw_defaults_set(unit, mirror_dest_id));

    /** Release allocated resource manager */
    SHR_IF_ERR_EXIT(dnx_algo_mirror_profile_deallocate(unit, mirror_dest_id));

exit:
    SHR_FUNC_EXIT;
}
