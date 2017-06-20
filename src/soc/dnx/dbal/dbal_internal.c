/** \file dbal_internal.c
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * System wide Logical Table Manager internal functions.
 *
 * To be used for:
 *   Access of physical table
 *   Access of pure software tables
 *   Activation of access procedutes (dispatcher) which is
 *     equivalent to 'MBCM and arad_pp_dbal' on SDK6 for JR1.
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DBALDNX

#include "dbal_internal.h"
#include <soc/dnx/mdb.h>
#include <soc/dnx/dbal/dbal.h>
#include <shared/utilex/utilex_bitstream.h>
#include <shared/utilex/utilex_framework.h>


static dbal_mngr_info_t dbal_mngr = { {{0}} };

uint32 G_dbal_field_full_mask[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF
};

uint8
dbal_is_intiated(
    int unit)
{
    return dbal_mngr.is_intiated;
}

void
dbal_initiated_set(
    int unit)
{
    dbal_mngr.is_intiated = TRUE;
}

void
dbal_initiated_reset(
    int unit)
{
    dbal_mngr.is_intiated = FALSE;
}

shr_error_e
dbal_action_prints(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags,
    char *src_str)
{
    int bsl_severity;
    dbal_entry_handle_t *entry_handle = &dbal_mngr.entry_handles_pool[entry_handle_id];

    SHR_FUNC_INIT_VARS(unit);

    SHR_GET_SEVERITY_FOR_MODULE(bsl_severity);

    if (bsl_severity >= bslSeverityInfo)
    {
        DBAL_PRINT_FRAME_FOR_API_PRINTS(TRUE, bslSeverityInfo);
        LOG_CLI((BSL_META("Action %s: handle ID %d, table %s flags "), src_str, entry_handle_id,
                 entry_handle->table->table_name));
        if (flags == 0)
        {
            LOG_CLI((BSL_META("NONE")));
        }
        else
        {
            int iter;
            uint32 bit_set;
            for (iter = 0; iter < DBAL_COMMIT_NOF_OPCODES; iter++)
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field((uint32 *) & flags, iter, 1, &bit_set));
                if (bit_set)
                {
                    LOG_CLI((BSL_META("%s "), dbal_action_flags_to_string(unit, iter)));
                }
            }
        }
        LOG_CLI((BSL_META("\n")));

        SHR_IF_ERR_EXIT(dbal_entry_print(unit, entry_handle_id, 0));
        DBAL_PRINT_FRAME_FOR_API_PRINTS(FALSE, bslSeverityInfo);
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_action_validations(
    int unit,
    uint32 entry_handle_id,
    int is_value_fields_required)
{
    dbal_entry_handle_t *entry_handle = &dbal_mngr.entry_handles_pool[entry_handle_id];
    dbal_logical_table_t *table = entry_handle->table;

    SHR_FUNC_INIT_VARS(unit);

    if ((entry_handle->handle_status < DBAL_HANDLE_STATUS_IN_USE) || (entry_handle_id > DBAL_SW_NOF_ENTRY_HANDLES))
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Handle status is invalid, cannot perform action. handle_id=%d\n",
                     entry_handle_id);
    }

    if (entry_handle->error_info.error_exists)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cannot perform action. there is an issue with field %s table %s\n",
                     dbal_field_to_string(unit, entry_handle->error_info.field_id), table->table_name);
    }

    if (entry_handle->nof_key_fields != table->nof_key_fields)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Not all Key fields where set for table %s. total set %d requiered %d\n",
                     table->table_name, entry_handle->nof_key_fields, table->nof_key_fields);
    }

    if (is_value_fields_required)
    {
        if (entry_handle->nof_result_fields == 0)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "No value fields where set, cannot perform the action %s\n",
                         table->table_name);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_entry_handle_take_internal(
    int unit,
    dbal_tables_e table_id,
    uint32 * entry_handle_id)
{
    int iter;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_logical_table_reference_get(unit, table_id, &table));

    for (iter = 0; iter < DBAL_SW_NOF_ENTRY_HANDLES; iter++)
    {
        if (dbal_mngr.entry_handles_pool[iter].handle_status == DBAL_HANDLE_STATUS_AVAILABLE)
        {
            dbal_mngr.entry_handles_pool[iter].handle_status = DBAL_HANDLE_STATUS_IN_USE;
            *entry_handle_id = iter;
            dbal_mngr.entry_handles_pool[iter].table_id = table_id;
            dbal_mngr.entry_handles_pool[iter].table = table;
            dbal_mngr.entry_handles_pool[iter].nof_result_types = table->nof_result_types;
            dbal_mngr.entry_handles_pool[iter].cur_res_type = DBAL_RESULT_TYPE_NOT_INITIALIZED;
            if (table->core_mode == DBAL_CORE_ALL)
            {
                dbal_mngr.entry_handles_pool[iter].core_id = DBAL_CORE_ANY;
            }
            else
            {
                dbal_mngr.entry_handles_pool[iter].core_id = DBAL_CORE_NOT_INTIATED;
            }
            break;
        }
    }

    if (iter == DBAL_SW_NOF_ENTRY_HANDLES)
    {
        *entry_handle_id = -1;
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "dbal_entry_handle_take no handles available\n");
    }

exit:
    SHR_FUNC_EXIT;
}

void
dbal_entry_handle_release_internal(
    int unit,
    uint32 entry_handle_id)
{
    if (entry_handle_id >= DBAL_SW_NOF_ENTRY_HANDLES)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META("Illegal entry handle = %d \n"), entry_handle_id));
    }
    else
    {
        sal_memset(&(dbal_mngr.entry_handles_pool[entry_handle_id]), 0x0, sizeof(dbal_entry_handle_t));
    }
}

shr_error_e
dbal_entry_handle_get_internal(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_handle_t ** entry_handle)
{
    SHR_FUNC_INIT_VARS(unit);

    if (entry_handle_id >= DBAL_SW_NOF_ENTRY_HANDLES)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal entry handle = %d \n", entry_handle_id);
    }

    (*entry_handle) = &(dbal_mngr.entry_handles_pool[entry_handle_id]);

exit:
    SHR_FUNC_EXIT;
}

void
dbal_entry_handle_default_get_internal(
    int unit,
    dbal_entry_handle_t ** entry_handle)
{
    (*entry_handle) = &(dbal_mngr.entry_handle_for_default_entry);
}

/************************ Generic field functions ************************/

void
dbal_entry_key_field_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    uint32 * field_val,
    uint32 * field_mask)
{
    dbal_entry_handle_t *entry_handle = &(dbal_mngr.entry_handles_pool[entry_handle_id]);
    int curr_field_index, num_of_bits = 0, is_field_updated = 0;
    dbal_logical_table_t *table = entry_handle->table;
    int is_sub_field = 0;
    uint32 valid_field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };
    uint32 *field_val_after_encoding;
    uint32 field_val_as_uint32;
    int is_sub_field_check = 0;
    dbal_fields_e parent_field_id = DBAL_FIELD_EMPTY;
    uint32 field_max_size;
    uint32 field_max_value;
    shr_error_e ret_val;

    if (entry_handle->handle_status < DBAL_HANDLE_STATUS_IN_USE)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "entry handle not taken %d\n"), entry_handle_id));
        DBAL_FIELD_ERR_HANDLE("entry handle not taken");
    }

    dbal_fields_is_sub_field(unit, field_id, &is_sub_field_check);

    for (curr_field_index = 0; curr_field_index < table->nof_key_fields; curr_field_index++)
    {
        if (table->keys_info[curr_field_index].field_id == field_id)
        {
            num_of_bits = table->keys_info[curr_field_index].field_nof_bits;
            if (entry_handle->key_field_ids[curr_field_index] != DBAL_FIELD_EMPTY)
            {
                is_field_updated = 1;
            }
            break;
        }
        if (is_sub_field_check)
        {
            int is_found = 0;
            dbal_fields_sub_field_match(unit, table->keys_info[curr_field_index].field_id, field_id, &is_found);
            if (is_found)
            {
                is_sub_field = 1;
                parent_field_id = table->keys_info[curr_field_index].field_id;
                num_of_bits = table->keys_info[curr_field_index].field_nof_bits;
                if (entry_handle->key_field_ids[curr_field_index] != DBAL_FIELD_EMPTY)
                {
                    is_field_updated = 1;
                }
                break;
            }
        }
    }

    if (num_of_bits == 0)
    {
        /** field not found or field is invlaid in table */
        DBAL_FIELD_ERR_HANDLE("KEY field not found in table or it is invalid");
    }

    dbal_fields_max_size_get(unit, field_id, &field_max_size);
    dbal_fields_max_value_get(unit, field_id, &field_max_value);

    if (num_of_bits >= field_max_size)
    {
        if (num_of_bits < 32)
        {
            /*
             * fields up to 31 bits length will be validated here
             */
            if (field_val[0] > field_max_value)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "field Illegal value. field max lvalue is 0x%x,"
                                                      "value is:0x%x\n"), field_max_value, field_val[0]));
                DBAL_FIELD_ERR_HANDLE("Illegal value for field");
            }
        }
        else
        {
            /*
             * fields larger than 31 bits will be validated here check that the value is within the valid range
             * according to nof_bits. The greater is neccesary for Child fields encoding validation 
             */
            int ii;
            int field_res_32b;
            int field_nof_32b;
            uint32 mask;
            int total_valid_length_of_input = num_of_bits + table->keys_info[curr_field_index].offset_in_logical_field;

            field_nof_32b = BITS2WORDS(total_valid_length_of_input);
            field_res_32b = (total_valid_length_of_input) % 32;

            if (field_res_32b == 0)
            {
                mask = 0xFFFFFFFF;
            }
            else
            {
                mask = (1 << field_res_32b) - 1;
            }
            if ((field_val[field_nof_32b - 1] & mask) != field_val[field_nof_32b - 1])
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "field Illegal value. legal length in bits %d value="),
                                           total_valid_length_of_input));
                for (ii = 0; ii < field_nof_32b; ii++)
                {
                    LOG_CLI((BSL_META_U(unit, "0x%08x "), field_val[ii]));
                }
                LOG_CLI((BSL_META_U(unit, "\n")));
                DBAL_FIELD_ERR_HANDLE("Illegal value for field");
            }
        }
    }

    /*
     * Copy the relevant part of the field 
     * Affect only if it has offset
     */
    ret_val = utilex_bitstream_get_any_field(field_val,
                                             table->keys_info[curr_field_index].offset_in_logical_field,
                                             table->keys_info[curr_field_index].field_nof_bits, valid_field_val);
    if (ret_val)
    {
        DBAL_FIELD_ERR_HANDLE("Copy from offset error for field");
    }

    field_val_after_encoding = valid_field_val;

    if ((dbal_fields_is_field_encoded(unit, field_id)) || is_sub_field)
    {
        ret_val = dbal_fields_field32_encode(unit, field_id, parent_field_id, valid_field_val[0], &field_val_as_uint32);
        if (ret_val)
        {
            DBAL_FIELD_ERR_HANDLE("encoding error for field");
        }
        field_val_after_encoding = &field_val_as_uint32;
    }

    /** incase of field is core id, save it directly in the handle  */
    if (field_id == DBAL_FIELD_CORE_ID)
    {
        uint32 core_val = 0;
        utilex_bitstream_set_any_field(field_val_after_encoding, 0,
                                       table->keys_info[curr_field_index].field_nof_bits, &(core_val));
        entry_handle->core_id = core_val;
    }
    else
    {
        utilex_bitstream_set_any_field(field_val_after_encoding,
                                       table->keys_info[curr_field_index].bits_offset_in_buffer,
                                       table->keys_info[curr_field_index].field_nof_bits,
                                       &(entry_handle->phy_entry.key[0]));
        if (field_mask)
        {
            utilex_bitstream_set_any_field(field_mask, table->keys_info[curr_field_index].bits_offset_in_buffer,
                                           table->keys_info[curr_field_index].field_nof_bits,
                                           &(entry_handle->phy_entry.k_mask[0]));
        }
        if (!is_field_updated)
        {
            entry_handle->phy_entry.key_size += table->keys_info[curr_field_index].field_nof_bits;
        }
    }

    entry_handle->key_field_ids[curr_field_index] = field_id;
    if (!is_field_updated)
    {
        entry_handle->num_of_fields++;
        entry_handle->nof_key_fields++;
    }
}

void
dbal_entry_value_field_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    uint32 * field_val)
{
    dbal_entry_handle_t *entry_handle = &(dbal_mngr.entry_handles_pool[entry_handle_id]);
    int iter, num_of_bits = 0, is_field_updated = 0, field_index_in_table = 0;
    dbal_logical_table_t *table = entry_handle->table;
    dbal_fields_e parent_field_id = DBAL_FIELD_EMPTY;
    uint32 *field_val_after_encoding;
    uint32 valid_field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };
    uint32 field_val_as_uint32;
    int res_type_idx;
    int is_sub_field = 0;
    int is_sub_field_check = 0;
    uint32 field_max_size;
    uint32 field_max_value;
    shr_error_e ret_val;

    if (entry_handle->handle_status < DBAL_HANDLE_STATUS_IN_USE)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "entry handle not taken %d\n"), entry_handle_id));
        DBAL_FIELD_ERR_HANDLE("entry handle not taken");
    }

    /** multiple result types support  */
    if (entry_handle->nof_result_types > 1)
    {
        if ((entry_handle->cur_res_type == DBAL_RESULT_TYPE_NOT_INITIALIZED))
        {
            if (field_id != DBAL_FIELD_RESULT_TYPE)
            {
                DBAL_FIELD_ERR_HANDLE("for table with multiple result types need"
                                      " to configure entry type before setting any value field");
            }
            else
            {
                uint32 result_type_hw_value;

                if (field_val[0] >= entry_handle->nof_result_types)
                {
                    DBAL_FIELD_ERR_HANDLE("illegal result type");
                }
                entry_handle->cur_res_type = (int) field_val[0];
                res_type_idx = entry_handle->cur_res_type;
                result_type_hw_value = table->multi_res_info[field_val[0]].result_type_hw_value;

                for (iter = 0; iter < table->multi_res_info[res_type_idx].nof_result_fields; iter++)
                {
                    if (table->multi_res_info[res_type_idx].results_info[iter].field_id == DBAL_FIELD_RESULT_TYPE)
                    {
                        num_of_bits = table->multi_res_info[res_type_idx].results_info[iter].field_nof_bits;
                        field_index_in_table = iter;
                        break;
                    }
                }

                entry_handle->value_field_ids[field_index_in_table] = field_id;
                entry_handle->num_of_fields++;
                entry_handle->nof_result_fields++;

                utilex_bitstream_set_any_field(&result_type_hw_value,
                                               table->multi_res_info[res_type_idx].results_info
                                               [field_index_in_table].bits_offset_in_buffer,
                                               table->multi_res_info[res_type_idx].results_info
                                               [field_index_in_table].field_nof_bits,
                                               &(entry_handle->phy_entry.payload[0]));
                return;
            }
        }
        else
        {
            if (field_id == DBAL_FIELD_RESULT_TYPE)
            {
                /** this limitation can be changed if needed but than we need to check that no other field was updated */
                DBAL_FIELD_ERR_HANDLE("for table with multiple result types cannot re-set the result type");
            }
        }
    }
    else
    {
        entry_handle->cur_res_type = 0;
    }

    res_type_idx = entry_handle->cur_res_type;

    dbal_fields_is_sub_field(unit, field_id, &is_sub_field_check);

    for (iter = 0; iter < table->multi_res_info[res_type_idx].nof_result_fields; iter++)
    {
        if (table->multi_res_info[res_type_idx].results_info[iter].field_id == field_id)
        {
            num_of_bits = table->multi_res_info[res_type_idx].results_info[iter].field_nof_bits;
            field_index_in_table = iter;
            if (entry_handle->value_field_ids[field_index_in_table] != DBAL_FIELD_EMPTY)
            {
                is_field_updated = 1;
            }
            break;
        }
        if (is_sub_field_check)
        {
            int is_found = 0;
            dbal_fields_sub_field_match(unit,
                                        table->multi_res_info[res_type_idx].results_info[iter].field_id, field_id,
                                        &is_found);
            if (is_found)
            {
                is_sub_field = 1;
                parent_field_id = table->multi_res_info[res_type_idx].results_info[iter].field_id;
                num_of_bits = table->multi_res_info[res_type_idx].results_info[iter].field_nof_bits;
                field_index_in_table = iter;
                if (entry_handle->value_field_ids[field_index_in_table] != DBAL_FIELD_EMPTY)
                {
                    is_field_updated = 1;
                }
                break;
            }
        }
    }

    if (num_of_bits == 0)
    {
        DBAL_FIELD_ERR_HANDLE("Value field not found in table");
    }

    dbal_fields_max_size_get(unit, field_id, &field_max_size);
    dbal_fields_max_value_get(unit, field_id, &field_max_value);

    if (num_of_bits >= field_max_size)
    {
        if (num_of_bits < 32)
        {
            /*
             * fields up to 31 bits length will be validated here
             */
            if (field_val[0] > field_max_value)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "field Illegal value. field max value is 0x%x,"
                                                      "value is:0x%x\n"), field_max_value, field_val[0]));
                DBAL_FIELD_ERR_HANDLE("Illegal value for field");
            }
        }
        else
        {
            /*
             * fields larger than 31 bits will be validated here check that the value is within the valid range
             * according to nof_bits. The greater is neccesary for Child fields encoding validation 
             */
            int ii;
            int field_res_32b;
            int field_nof_32b;
            uint32 mask;
            int total_valid_length_of_input =
                num_of_bits +
                table->multi_res_info[res_type_idx].results_info[field_index_in_table].offset_in_logical_field;

            field_nof_32b = BITS2WORDS(total_valid_length_of_input);
            field_res_32b = (total_valid_length_of_input) % 32;

            if (field_res_32b == 0)
            {
                mask = 0xFFFFFFFF;
            }
            else
            {
                mask = (1 << field_res_32b) - 1;
            }
            if ((field_val[field_nof_32b - 1] & mask) != field_val[field_nof_32b - 1])
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "field Illegal value. legal length in bits %d value="),
                                           total_valid_length_of_input));
                for (ii = 0; ii < field_nof_32b; ii++)
                {
                    LOG_CLI((BSL_META_U(unit, "0x%08x "), field_val[ii]));
                }
                LOG_CLI((BSL_META_U(unit, "\n")));
                DBAL_FIELD_ERR_HANDLE("Illegal value for field");
            }
        }
    }

    /*
     * Copy the relevant part of the field 
     * Affect only if it has offset
     */
    ret_val =
        utilex_bitstream_get_any_field(field_val,
                                       table->multi_res_info[res_type_idx].results_info
                                       [field_index_in_table].offset_in_logical_field,
                                       table->multi_res_info[res_type_idx].results_info
                                       [field_index_in_table].field_nof_bits, valid_field_val);
    if (ret_val)
    {
        DBAL_FIELD_ERR_HANDLE("Copy from offset error for field");
    }

    field_val_after_encoding = valid_field_val;

    if ((dbal_fields_is_field_encoded(unit, field_id)) || is_sub_field)
    {
        ret_val = dbal_fields_field32_encode(unit, field_id, parent_field_id, field_val[0], &field_val_as_uint32);
        if (ret_val)
        {
            DBAL_FIELD_ERR_HANDLE("encoding error for field");
        }
        field_val_after_encoding = &field_val_as_uint32;
    }

    utilex_bitstream_set_any_field(field_val_after_encoding,
                                   table->multi_res_info[res_type_idx].
                                   results_info[field_index_in_table].bits_offset_in_buffer,
                                   table->multi_res_info[res_type_idx].
                                   results_info[field_index_in_table].field_nof_bits,
                                   &(entry_handle->phy_entry.payload[0]));
    if (!is_field_updated)
    {
        entry_handle->phy_entry.payload_size +=
            table->multi_res_info[res_type_idx].results_info[field_index_in_table].field_nof_bits;
        utilex_bitstream_set_any_field(G_dbal_field_full_mask,
                                       table->multi_res_info[res_type_idx].results_info
                                       [field_index_in_table].bits_offset_in_buffer,
                                       table->multi_res_info[res_type_idx].results_info
                                       [field_index_in_table].field_nof_bits, &(entry_handle->phy_entry.p_mask[0]));
    }

    entry_handle->value_field_ids[field_index_in_table] = field_id;
    if (!is_field_updated)
    {
        entry_handle->num_of_fields++;
        entry_handle->nof_result_fields++;
    }
}

void
dbal_entry_value_field_get(
    int unit,
    dbal_field_type_e field_type,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    void *field_val,
    void *field_mask)
{
    dbal_entry_handle_t *entry_handle = &(dbal_mngr.entry_handles_pool[entry_handle_id]);
    dbal_logical_table_t *table = entry_handle->table;
    int iter, field_index_in_table = 0, is_field_updated = 0;
    int is_sub_field_check = 0;
    int is_found = 0, res_type = 0;

    if (entry_handle->handle_status < DBAL_HANDLE_STATUS_IN_USE)
    {
        DBAL_FIELD_ERR_HANDLE("entry handle not taken");
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "entry handle not taken %d\n"), entry_handle_id));
        return;
    }

    if (table->nof_result_types > 1)
    {
        DBAL_FIELD_ERR_HANDLE("cannot set value field to multiple result types table use handle_value_field");
    }

    entry_handle->cur_res_type = 0;

    dbal_fields_is_sub_field(unit, field_id, &is_sub_field_check);

    for (iter = 0; iter < table->multi_res_info[res_type].nof_result_fields; iter++)
    {
        if (table->multi_res_info[res_type].results_info[iter].field_id == field_id)
        {
            field_index_in_table = iter;
            is_found = 1;
            if (entry_handle->value_field_ids[field_index_in_table] != DBAL_FIELD_EMPTY)
            {
                is_field_updated = 1;
            }
            break;
        }
        if (is_sub_field_check)
        {
            dbal_fields_sub_field_match(unit,
                                        table->multi_res_info[res_type].results_info[iter].field_id,
                                        field_id, &is_found);
            if (is_found)
            {
                field_index_in_table = iter;
                if (entry_handle->value_field_ids[field_index_in_table] != DBAL_FIELD_EMPTY)
                {
                    is_field_updated = 1;
                }
                break;
            }
        }
    }

    if (is_found == 0)
    {
        DBAL_FIELD_ERR_HANDLE("VALUE field not found in table");
    }

    if ((field_type == DBAL_FIELD_TYPE_UINT32)
        && (table->multi_res_info[res_type].results_info[field_index_in_table].field_nof_bits > 32))
    {
        DBAL_FIELD_ERR_HANDLE("field size too big for uint32");
    }

    entry_handle->value_field_ids[field_index_in_table] = field_id;
    entry_handle->user_output_info[field_index_in_table].returned_pointer = field_val;
    entry_handle->user_output_info[field_index_in_table].type = field_type;

    if (!is_field_updated)
    {
        entry_handle->num_of_fields++;
        entry_handle->nof_result_fields++;
    }
}

shr_error_e
dbal_entry_handle_value_field_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 * field_val)
{
    dbal_entry_handle_t *entry_handle;
    dbal_table_field_info_t table_field_info;

    SHR_FUNC_INIT_VARS(unit);

    DBAL_ENTRY_HANDLE_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);

    if (entry_handle->get_all_fields == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "no available fields\n");
    }

    if (entry_handle->handle_status != DBAL_HANDLE_STATUS_ACTION_PREFORMED)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "field get not preformed on the handle buffer empty\n");
    }

    /*
     *  incase of result type we can easily retun it from the
     *  handle, also if we will take it from buffer we need to use
     *  mapping to the HW value to the enum value like in
     *  dbal_set_result_type_from_buffer
     */
    if (field_id == DBAL_FIELD_RESULT_TYPE)
    {
        (*field_val) = entry_handle->cur_res_type;
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_table_field_info_get(unit, entry_handle->table_id, field_id, 0, entry_handle->cur_res_type,
                                                  &table_field_info));
        SHR_IF_ERR_EXIT(dbal_field_from_buffer_get(unit, &table_field_info, field_id, entry_handle->phy_entry.payload,
                                                   field_val));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_entry_print(
    int unit,
    uint32 entry_handle_id,
    uint8 format)
{
    int iter;
    dbal_entry_handle_t *entry_handle = &(dbal_mngr.entry_handles_pool[entry_handle_id]);
    dbal_logical_table_t *table = entry_handle->table;
    int bsl_severity;
    uint32 *mask = NULL;
    int nof_fields_printed = 0;

    SHR_FUNC_INIT_VARS(unit);

    if (table->access_method == DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        if ((table->physical_db_id == DBAL_PHYSICAL_TABLE_TCAM) ||
            (table->physical_db_id == DBAL_PHYSICAL_TABLE_LPM_PRIVATE) ||
            (table->physical_db_id == DBAL_PHYSICAL_TABLE_LPM_PUBLIC))
        {
            mask = entry_handle->phy_entry.k_mask;
        }
    }

    SHR_GET_SEVERITY_FOR_MODULE(bsl_severity);

    /** buffer is printed with low severity */
    SHR_IF_ERR_EXIT(dbal_phy_entry_print(unit, &entry_handle->phy_entry, FALSE, bslSeverityDebug));

    if (bsl_severity >= bslSeverityInfo)
    {
        LOG_CLI((BSL_META("KEY: ")));
        for (iter = 0; iter < table->nof_key_fields; iter++)
        {
            if (entry_handle->key_field_ids[iter] == DBAL_FIELD_EMPTY)
            {
                continue;
            }

            if ((entry_handle->key_field_ids[iter] == DBAL_FIELD_CORE_ID) && table->core_mode == DBAL_CORE_BY_INPUT)
            {
                /** in this case the core ID will be used as a seperate parameter and not part of the key */
                LOG_CLI((BSL_META("%s "), dbal_field_to_string(unit, entry_handle->key_field_ids[iter])));
                LOG_CLI((BSL_META("%d"), entry_handle->core_id));
            }
            else
            {
                SHR_IF_ERR_EXIT(dbal_field_from_buffer_print(unit, entry_handle->key_field_ids[iter],
                                                             entry_handle->table_id, entry_handle->phy_entry.key, mask,
                                                             0, TRUE, TRUE));
            }

            if (nof_fields_printed < (entry_handle->nof_key_fields - 1))
            {
                LOG_CLI((BSL_META(", ")));
            }
            nof_fields_printed++;
        }

        nof_fields_printed = 0;
        if (entry_handle->nof_result_fields)
        {
            LOG_CLI((BSL_META("\nRESULT: ")));
            for (iter = 0; iter < table->multi_res_info[entry_handle->cur_res_type].nof_result_fields; iter++)
            {
                if (entry_handle->value_field_ids[iter] == DBAL_FIELD_EMPTY)
                {
                    continue;
                }

                SHR_IF_ERR_EXIT(dbal_field_from_buffer_print(unit, entry_handle->value_field_ids[iter],
                                                             entry_handle->table_id, entry_handle->phy_entry.payload,
                                                             NULL, entry_handle->cur_res_type, FALSE, TRUE));

                if (nof_fields_printed < (entry_handle->nof_result_fields - 1))
                {
                    LOG_CLI((BSL_META(", ")));
                }
                nof_fields_printed++;
            }
        }
        LOG_CLI((BSL_META("\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_phy_entry_print(
    int unit,
    dbal_physical_entry_t * entry,
    uint8 print_only_key,
    int prints_severity)
{
    int ii;
    int key_size_in_words = BITS2WORDS(entry->key_size);
    int payload_size_in_words = BITS2WORDS(entry->payload_size);
    int bsl_severity;
    int severity_of_prints;

    SHR_FUNC_INIT_VARS(unit);

    SHR_GET_SEVERITY_FOR_MODULE(bsl_severity);

    if (prints_severity != -1)
    {
        severity_of_prints = prints_severity;
    }
    else
    {
        severity_of_prints = bslSeverityInfo;
    }

    if (bsl_severity >= severity_of_prints)
    {
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, severity_of_prints);
        LOG_CLI((BSL_META("Physical entry.\n")));

        LOG_CLI((BSL_META("Key(%3d bits): 0x"), entry->key_size));
        for (ii = key_size_in_words - 1; ii >= 0; ii--)
        {
            LOG_CLI((BSL_META("%08x"), entry->key[ii]));
        }
        LOG_CLI((BSL_META("\n")));
        LOG_CLI((BSL_META("Key Mask     : 0x")));
        for (ii = key_size_in_words - 1; ii >= 0; ii--)
        {
            LOG_CLI((BSL_META("%08x"), entry->k_mask[ii]));
        }
        LOG_CLI((BSL_META("\n")));

        if (!print_only_key)
        {
            LOG_CLI((BSL_META("Payload(%3d bits): 0x"), entry->payload_size));
            for (ii = payload_size_in_words - 1; ii >= 0; ii--)
            {
                LOG_CLI((BSL_META("%08x"), entry->payload[ii]));
            }
            LOG_CLI((BSL_META("\n")));
            LOG_CLI((BSL_META("Payload Mask     : 0x")));
            for (ii = payload_size_in_words - 1; ii >= 0; ii--)
            {
                LOG_CLI((BSL_META("%08x"), entry->p_mask[ii]));
            }
            LOG_CLI((BSL_META("\n")));
        }
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, severity_of_prints);
    }
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_entry_key_field_from_handle_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    dbal_fields_e field_id,
    uint32 * field_val_returned,
    uint32 * is_found)
{
    uint32 field_val, field_value_after_encode;
    int iter;
    dbal_logical_table_t *table = entry_handle->table;
    int field_offset_in_buffer, field_nof_bits;

    SHR_FUNC_INIT_VARS(unit);

    for (iter = 0; iter < DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS; iter++)
    {
        if (entry_handle->key_field_ids[iter] == field_id)
        {
            break;
        }
    }

    if (iter == DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS)
    {
        *is_found = 0;
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "field %s not exists in buffer %s \n",
                     dbal_field_to_string(unit, field_id), table->table_name);
    }

    field_offset_in_buffer = table->keys_info[iter].bits_offset_in_buffer;
    field_nof_bits = table->keys_info[iter].field_nof_bits;

    if (field_nof_bits > 32)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "field %s bigger than 32bit \n", dbal_field_to_string(unit, field_id));
    }

    if (field_id == DBAL_FIELD_CORE_ID)
    {
        if (entry_handle->core_id != DBAL_CORE_NOT_INTIATED)
        {
            (*field_val_returned) = entry_handle->core_id;
        }
        else
        {
            /** could not happen, if the field found it should be initiated */
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Requested field core ID not found");
        }
    }
    else
    {
        SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(entry_handle->phy_entry.key, field_offset_in_buffer,
                                                       field_nof_bits, &field_val));

        if (dbal_fields_is_field_encoded(unit, field_id))
        {
            /*
             * subfield decoding is not needed here 
             */
            SHR_IF_ERR_EXIT(dbal_fields_field32_decode(unit, entry_handle->key_field_ids[iter], 0, field_val,
                                                       &field_value_after_encode));
            (*field_val_returned) = field_value_after_encode;
        }
        else
        {
            (*field_val_returned) = field_val;
        }
    }

    *is_found = 1;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_set_key_info_from_buffer(
    int unit,
    dbal_entry_handle_t * entry_handle,
    dbal_field_data_t * keys_info,
    uint8 * is_valid)
{
    int ii, jj;
    dbal_logical_table_t *table;
    dbal_field_basic_info_t field_info;

    SHR_FUNC_INIT_VARS(unit);

    table = entry_handle->table;
    *is_valid = TRUE;

    for (ii = 0; ii < table->nof_key_fields; ii++)
    {
        /*
         * copy the field value from buffer to the iterator info 
         */
        keys_info[ii].field_id = table->keys_info[ii].field_id;
        if (table->keys_info[ii].field_id == DBAL_FIELD_CORE_ID)
        {
            if ((entry_handle->core_id > DBAL_CORE_ANY) && (entry_handle->core_id < (1 << DBAL_CORE_SIZE_IN_BITS)))
            {
                keys_info[ii].field_val[0] = entry_handle->core_id;
            }
            else
            {
                *is_valid = FALSE;
                SHR_EXIT();
            }
        }
        else
        {
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(entry_handle->phy_entry.key,
                                                           table->keys_info[ii].bits_offset_in_buffer,
                                                           table->keys_info[ii].field_nof_bits,
                                                           keys_info[ii].field_val));
        }
        /*
         * If field is of type ENUM, translate it to its enum val. 
         * If not exists, the entry is not valid 
         */
        SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, table->keys_info[ii].field_id, &field_info));
        if (field_info.type == DBAL_FIELD_TYPE_ENUM)
        {
            for (jj = 0; jj < field_info.nof_enum_values; jj++)
            {
                if (keys_info[ii].field_val[0] == field_info.enum_val_info[jj].value)
                {
                    keys_info[ii].field_val[0] = jj;
                    break;
                }
            }
            if (jj == field_info.nof_enum_values)
            {
                *is_valid = FALSE;
                SHR_EXIT();
            }
            else
            {
                *is_valid = TRUE;
            }
        }
        else if ((field_info.max_value != 0) && (keys_info[ii].field_val[0] > field_info.max_value))
        {
            *is_valid = FALSE;
            SHR_EXIT();
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_set_result_type_from_buffer(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int result_type_size, ii;
    int result_type_offset_in_buffer;
    uint32 result_type_mask;
    uint32 result_hw_val;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    table = entry_handle->table;
    result_type_size = table->multi_res_info[0].results_info[0].field_nof_bits;
    result_type_offset_in_buffer = table->multi_res_info[0].results_info[0].bits_offset_in_buffer;
    result_type_mask = ((1 << result_type_size) - 1);

    if (result_type_size > 32)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "result type bigger than 32bit not supported \n");
    }

    SHR_IF_ERR_EXIT(utilex_bitstream_get_field(entry_handle->phy_entry.payload, result_type_offset_in_buffer,
                                               result_type_size, &result_hw_val));

    result_hw_val &= result_type_mask;

    for (ii = 0; ii < entry_handle->table->nof_result_types; ii++)
    {
        if (entry_handle->table->multi_res_info[ii].result_type_hw_value == result_hw_val)
        {
            /** result type found  */
            entry_handle->cur_res_type = ii;
            break;
        }
    }

    if (ii == entry_handle->table->nof_result_types)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "result type HW value is not found in SW mapping\n");
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_handle_update_requested_fields(
    int unit,
    dbal_entry_handle_t * entry_handle,
    dbal_field_data_t fields_array[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS])
{
    int ii;
    uint32 result_idx;
    dbal_logical_table_t *table = entry_handle->table;

    SHR_FUNC_INIT_VARS(unit);

    result_idx = entry_handle->cur_res_type;

    if (result_idx == DBAL_RESULT_TYPE_NOT_INITIALIZED)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "result type not initialized \n");
    }

    for (ii = 0; ii < table->multi_res_info[result_idx].nof_result_fields; ii++)
    {
        entry_handle->value_field_ids[ii] = table->multi_res_info[result_idx].results_info[ii].field_id;
        entry_handle->user_output_info[ii].returned_pointer = fields_array[ii].field_val;
        entry_handle->user_output_info[ii].type = DBAL_FIELD_TYPE_UINT32;
        fields_array[ii].field_id = entry_handle->value_field_ids[ii];
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_entry_handle_value_fields_parse(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int iter;
    dbal_logical_table_t *table = entry_handle->table;
    dbal_table_field_info_t *table_field_info;

    SHR_FUNC_INIT_VARS(unit);

    for (iter = 0; iter < table->multi_res_info[entry_handle->cur_res_type].nof_result_fields; iter++)
    {
        if (entry_handle->value_field_ids[iter] == DBAL_FIELD_EMPTY)
        {
            continue;
        }

        if (entry_handle->value_field_ids[iter] == DBAL_FIELD_RESULT_TYPE)
        {
            uint32 *field_val = (uint32 *) entry_handle->user_output_info[iter].returned_pointer;
            field_val[0] = entry_handle->cur_res_type;
            continue;
        }

        table_field_info = &table->multi_res_info[entry_handle->cur_res_type].results_info[iter];

        switch (entry_handle->user_output_info[iter].type)
        {
            case DBAL_FIELD_TYPE_UINT32:
            case DBAL_FIELD_TYPE_ARRAY32:
            case DBAL_FIELD_TYPE_INT32:

                SHR_IF_ERR_EXIT(dbal_field_from_buffer_get(unit, table_field_info, entry_handle->value_field_ids[iter],
                                                           entry_handle->phy_entry.payload,
                                                           (uint32 *) entry_handle->
                                                           user_output_info[iter].returned_pointer));
                break;

            case DBAL_FIELD_TYPE_ARRAY8:
            {
                uint32 field_array32_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };
                SHR_IF_ERR_EXIT(dbal_field_from_buffer_get(unit, table_field_info, entry_handle->value_field_ids[iter],
                                                           entry_handle->phy_entry.payload, field_array32_val));
                SHR_IF_ERR_EXIT(utilex_U32_to_U8(field_array32_val, DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES,
                                                 ((uint8 *) entry_handle->user_output_info[iter].returned_pointer)));
                break;
            }

            /** this field type is not used when using the get field functionality */
            case DBAL_FIELD_TYPE_BOOL:
            case DBAL_FIELD_TYPE_IP:
            case DBAL_FIELD_TYPE_NONE:
            case DBAL_FIELD_TYPE_BITMAP:
            case DBAL_FIELD_TYPE_ENUM:
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Wrong type %s for field %s %d \n",
                             dbal_field_to_string(unit, entry_handle->value_field_ids[iter]),
                             dbal_field_type_to_string(unit, entry_handle->user_output_info[iter].type),
                             entry_handle->user_output_info[iter].type);
                break;

            default:
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal type %s %d\n",
                             dbal_field_type_to_string(unit, entry_handle->user_output_info[iter].type),
                             entry_handle->user_output_info[iter].type);
                break;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_field_from_buffer_get(
    int unit,
    dbal_table_field_info_t * table_field_info,
    dbal_fields_e field_id,
    uint32 * buffer,
    uint32 * returned_field_value)
{
    int field_offset_in_buffer, field_nof_bits, field_logical_offset;
    int is_sub_field = 0;
    dbal_fields_e parent_field_id = DBAL_FIELD_EMPTY;
    uint32 field_value[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };
    uint32 field_value_after_offset[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };
    uint32 field_value_after_encode;

    SHR_FUNC_INIT_VARS(unit);

    (*returned_field_value) = 0;

    if (field_id != table_field_info->field_id)
    {
        parent_field_id = table_field_info->field_id;
        is_sub_field = 1;
    }

    field_offset_in_buffer = table_field_info->bits_offset_in_buffer;
    field_nof_bits = table_field_info->field_nof_bits;
    field_logical_offset = table_field_info->offset_in_logical_field;

    SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(buffer, field_offset_in_buffer, field_nof_bits, field_value));

    if ((dbal_fields_is_field_encoded(unit, table_field_info->field_id)) || (is_sub_field))
    {
        SHR_IF_ERR_EXIT(dbal_fields_field32_decode(unit, field_id, parent_field_id, field_value[0],
                                                   &field_value_after_encode));
        if (is_sub_field)
        {
            uint32 verify_field_val;
            SHR_IF_ERR_EXIT(dbal_fields_field32_encode(unit, field_id, parent_field_id, field_value_after_encode,
                                                       &verify_field_val));
            if (verify_field_val != field_value[0])
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL,
                             " Subfield requested %s not match to entry field type field requested = %s\n",
                             dbal_field_to_string(unit, field_id),
                             dbal_field_to_string(unit, table_field_info->field_id));
            }
        }
        utilex_bitstream_set_any_field(&field_value_after_encode, field_logical_offset, field_nof_bits,
                                       field_value_after_offset);
    }
    else
    {
        utilex_bitstream_set_any_field(field_value, field_logical_offset, field_nof_bits, field_value_after_offset);
    }

    sal_memcpy(returned_field_value, field_value_after_offset, BITS2BYTES(field_nof_bits + field_logical_offset));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_field_from_buffer_print(
    int unit,
    dbal_fields_e field_id,
    dbal_tables_e table_id,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES],
    uint32 field_mask[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES],
    int result_type_idx,
    uint8 is_key,
    uint8 is_full_buffer)
{
    uint32 val_as_uint32 = 0;
    uint32 decoded_field_value = 0;
    int iter;
    dbal_field_basic_info_t field_info;
    int offset, size;
    dbal_fields_e parent_field = DBAL_FIELD_EMPTY;
    dbal_table_field_info_t table_field_info = { 0 };
    CONST dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, field_id, &field_info));
    SHR_IF_ERR_EXIT(dbal_table_field_info_get(unit, table_id, field_id, is_key, result_type_idx, &table_field_info));
    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    if (is_full_buffer)
    {
        offset = table_field_info.bits_offset_in_buffer;
    }
    else
    {
        offset = 0;
    }
    size = table_field_info.field_nof_bits;

    if (table_field_info.field_id != field_id)
    {
        parent_field = table_field_info.field_id;
    }

    LOG_CLI((BSL_META("%s "), field_info.name));

    switch (field_info.type)
    {
        case DBAL_FIELD_TYPE_UINT32:
        case DBAL_FIELD_TYPE_INT32:

            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, size, &val_as_uint32));
            if ((dbal_fields_is_field_encoded(unit, field_id)) || (parent_field != DBAL_FIELD_EMPTY))
            {
                SHR_IF_ERR_EXIT(dbal_fields_field32_decode(unit, field_id, parent_field,
                                                           val_as_uint32, &decoded_field_value));
                LOG_CLI((BSL_META("encoded 0x%x decoded 0x%x "), val_as_uint32, decoded_field_value));
            }
            else
            {
                dbal_fields_e sub_field_id;
                uint32 sub_field_val;

                if (field_info.type == DBAL_FIELD_TYPE_UINT32)
                {
                    if (field_id == DBAL_FIELD_RESULT_TYPE)
                    {
                        LOG_CLI((BSL_META("%s "), table->multi_res_info[result_type_idx].result_type_name));
                    }
                    else
                    {
                        LOG_CLI((BSL_META("0x%x "), val_as_uint32));
                    }
                }
                else
                {
                    LOG_CLI((BSL_META("%d "), val_as_uint32));
                }

                if (field_info.nof_child_fields != 0)
                {
                    SHR_IF_ERR_EXIT(dbal_fields_sub_field_info_get(unit, field_id, val_as_uint32,
                                                                   &sub_field_id, &sub_field_val));
                    if (sub_field_id != DBAL_FIELD_EMPTY)
                    {
                        LOG_CLI((BSL_META("encoded sub-field %s 0x%x "), dbal_field_to_string(unit, sub_field_id),
                                 sub_field_val));
                    }
                }

            }
            break;

        case DBAL_FIELD_TYPE_IP:
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, size, &val_as_uint32));
            LOG_CLI((BSL_META("%d.%d.%d.%d (0x%x) "), UTILEX_GET_BYTE_0(val_as_uint32),
                     UTILEX_GET_BYTE_1(val_as_uint32), UTILEX_GET_BYTE_2(val_as_uint32),
                     UTILEX_GET_BYTE_3(val_as_uint32), val_as_uint32));
            break;

        case DBAL_FIELD_TYPE_BITMAP:
            LOG_CLI((BSL_META("(set bits):")));
            {
                int is_first_print = 1;
                int is_bit_set = 0;
                int first_bit_in_sequence = 0;
                for (iter = 0; iter < size; iter++)
                {
                    SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, 1, &val_as_uint32));
                    offset = offset + 1;
                    if (val_as_uint32)
                    {
                        if (!is_bit_set)
                        {
                            first_bit_in_sequence = iter;
                            is_bit_set = 1;
                        }
                    }
                    else
                    {
                        if (is_bit_set)
                        {
                            if (first_bit_in_sequence == iter - 1)
                            {
                                if (is_first_print)
                                {
                                    LOG_CLI((BSL_META(" %d"), iter - 1));
                                }
                                else
                                {
                                    LOG_CLI((BSL_META(", %d"), iter - 1));
                                }
                            }
                            else
                            {
                                if (is_first_print)
                                {
                                    LOG_CLI((BSL_META(" %d-%d"), first_bit_in_sequence, iter - 1));
                                }
                                else
                                {
                                    LOG_CLI((BSL_META(", %d-%d"), first_bit_in_sequence, iter - 1));
                                }
                            }
                            is_first_print = 0;
                        }
                        first_bit_in_sequence = 0;
                        is_bit_set = 0;
                    }
                }
                if (is_bit_set)
                {
                    if (first_bit_in_sequence == (iter - 1))
                    {
                        LOG_CLI((BSL_META("%d"), iter - 1));
                    }
                    else
                    {
                        LOG_CLI((BSL_META("%d-%d"), first_bit_in_sequence, iter - 1));
                    }
                }
                if (is_first_print == 1)
                {
                    LOG_CLI((BSL_META(" none")));
                }
            }
            break;

        case DBAL_FIELD_TYPE_BOOL:
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, size, &val_as_uint32));
            if (val_as_uint32 == 0)
            {
                LOG_CLI((BSL_META("False ")));
            }
            else
            {
                LOG_CLI((BSL_META("True ")));
            }
            break;

        case DBAL_FIELD_TYPE_ARRAY8:
            for (iter = 0; iter < BITS2BYTES(size) - 1; iter++)
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, 8, &val_as_uint32));
                offset = offset + 8;
                LOG_CLI((BSL_META("0x%x:"), val_as_uint32));
            }
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, 8, &val_as_uint32));
            LOG_CLI((BSL_META("0x%x "), val_as_uint32));
            break;

        case DBAL_FIELD_TYPE_ARRAY32:
            for (iter = 0; iter < BITS2WORDS(size) - 1; iter++)
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, 32, &val_as_uint32));
                offset = offset + 32;
                LOG_CLI((BSL_META("0x%x:"), val_as_uint32));
            }
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, 32, &val_as_uint32));
            LOG_CLI((BSL_META("0x%x "), val_as_uint32));
            break;

        case DBAL_FIELD_TYPE_ENUM:
        {
            uint32 enum_val;
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, offset, size, &val_as_uint32));
            SHR_IF_ERR_EXIT(dbal_fields_field32_decode(unit, field_id, DBAL_FIELD_EMPTY, val_as_uint32, &enum_val));
            LOG_CLI((BSL_META("%s, hw value 0x%x"), field_info.enum_val_info[enum_val].name, val_as_uint32));
            break;
        }
        default:
            break;
    }

    /** mask print  */
    if (field_mask)
    {
        offset = table_field_info.bits_offset_in_buffer;
        LOG_CLI((BSL_META("mask: ")));
        for (iter = 0; iter < BITS2WORDS(size) - 1; iter++)
        {
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_mask, offset, 32, &val_as_uint32));
            offset = offset + 32;
            LOG_CLI((BSL_META("0x%x "), val_as_uint32));
        }
        SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_mask, offset, size, &val_as_uint32));
        LOG_CLI((BSL_META("0x%x "), val_as_uint32));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_iterator_init_handle_info(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    int ii;
    uint32 key_size = 0;
    dbal_logical_table_t *table;
    dbal_entry_handle_t *entry_handle;

    SHR_FUNC_INIT_VARS(unit);

    entry_handle = iterator_info->entry_handle;
    table = entry_handle->table;

    for (ii = 0; ii < table->nof_key_fields; ii++)
    {
        /** set key fields on both iterator info and handle  */
        iterator_info->keys_info[ii].field_id = table->keys_info[ii].field_id;
        entry_handle->key_field_ids[ii] = table->keys_info[ii].field_id;

        if ((table->keys_info[ii].field_id == DBAL_FIELD_CORE_ID) && (table->core_mode == DBAL_CORE_BY_INPUT))
        {
            /** set the iterator with the max core index, than iterate until it is negative */
            entry_handle->core_id = (1 << table->keys_info[ii].field_nof_bits) - 1;
        }
        else
        {
            key_size += table->keys_info[ii].field_nof_bits;
        }
    }
    entry_handle->nof_key_fields = table->nof_key_fields;
    entry_handle->num_of_fields = table->nof_key_fields;
    entry_handle->phy_entry.key_size = key_size;

    if (key_size != 0)
    {
        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(entry_handle->phy_entry.k_mask, 0, key_size - 1));
    }

    if (table->nof_result_types == 1)
    {
        int nof_result_fields = table->multi_res_info[0].nof_result_fields;
        dbal_table_field_info_t *results_info = table->multi_res_info[0].results_info;
        entry_handle->cur_res_type = 0;
        entry_handle->phy_entry.payload_size = table->max_payload_size;

        for (ii = 0; ii < nof_result_fields; ii++)
        {
            iterator_info->results_info[ii].field_id = results_info[ii].field_id;
            entry_handle->value_field_ids[ii] = results_info[ii].field_id;

            entry_handle->user_output_info[ii].returned_pointer = iterator_info->results_info[ii].field_val;
            entry_handle->user_output_info[ii].type = DBAL_FIELD_TYPE_UINT32;
        }
        entry_handle->nof_result_fields = nof_result_fields;
        entry_handle->num_of_fields += nof_result_fields;
    }
    else
    {
        entry_handle->phy_entry.payload_size = table->max_payload_size;
    }

exit:
    SHR_FUNC_EXIT;
}
