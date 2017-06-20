/**
 * \file dbal_api.c
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * System wide Logical Table Manager.
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

extern uint32 G_dbal_field_full_mask[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

/************************************************** APIs IMPLEMENTATION*********************************************/

shr_error_e
dbal_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    if (!dbal_is_intiated(unit))
    {
        SHR_IF_ERR_EXIT(dbal_fields_init(unit));
        SHR_IF_ERR_EXIT(dbal_logical_tables_init(unit));
        SHR_IF_ERR_EXIT(dbal_physical_table_init(unit));

        dbal_initiated_set(unit);
        dbal_log_severity_set(unit, DNX_DBAL_LOGGER_TYPE_API, bslSeverityWarn);
        dbal_log_severity_set(unit, DNX_DBAL_LOGGER_TYPE_ACCESS, bslSeverityWarn);
        LOG_INFO(BSL_LOG_MODULE, (BSL_META("DBAL init performed\n")));
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "DBAL init is called but DBAL is already initiated\n");
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_deinit(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    if (dbal_is_intiated(unit))
    {
        SHR_IF_ERR_EXIT(dbal_physical_table_deinit(unit));
        SHR_IF_ERR_EXIT(dbal_logical_tables_deinit(unit));
        SHR_IF_ERR_EXIT(dbal_fields_deinit(unit));

        dbal_initiated_reset(unit);
        LOG_INFO(BSL_LOG_MODULE, (BSL_META("DBAL deinit performed\n")));
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "DBAL deinit is called but DBAL is not initiated\n");
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_log_severity_get(
    int unit,
    dnx_dbal_logger_type_e dbal_logger_type,
    bsl_severity_t * severity)
{
    SHR_FUNC_INIT_VARS(unit);

    if (dbal_logger_type == DNX_DBAL_LOGGER_TYPE_API)
    {
        SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, *severity);
    }
    else
    {
        SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALACCESSDNX, *severity);
    }

    SHR_FUNC_EXIT;
}

shr_error_e
dbal_log_severity_set(
    int unit,
    dnx_dbal_logger_type_e dbal_logger_type,
    int severity)
{
    SHR_FUNC_INIT_VARS(unit);

    if (dbal_logger_type == DNX_DBAL_LOGGER_TYPE_API)
    {
        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, severity);
    }
    else
    {
        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALACCESSDNX, severity);
    }

    SHR_FUNC_EXIT;
}

/******* Handle related operations **********/
shr_error_e
dbal_entry_handle_take(
    int unit,
    dbal_tables_e table_id,
    uint32 * entry_handle_id)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(dbal_entry_handle_take_internal(unit, table_id, entry_handle_id));
exit:
    SHR_FUNC_EXIT;
}

void
dbal_entry_handle_release(
    int unit,
    uint32 entry_handle_id)
{
    dbal_entry_handle_release_internal(unit, entry_handle_id);
}

shr_error_e
dbal_entry_handle_info_get(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_handle_t * entry_handle)
{
    dbal_entry_handle_t *local_entry_handle;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &local_entry_handle));

    (*entry_handle) = (*local_entry_handle);

exit:
    SHR_FUNC_EXIT;
}

/*********** Entry field operations - SET APIs ***********/
void
dbal_entry_key_field8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 field_val)
{
    uint32 field_val_u32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, G_dbal_field_full_mask);
}

void
dbal_entry_key_field16_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint16 field_val)
{
    uint32 field_val_u32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, G_dbal_field_full_mask);
}

void
dbal_entry_key_field32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 field_val)
{
    uint32 field_val_u32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, G_dbal_field_full_mask);
}

void
dbal_entry_key_field_arr8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 * field_val)
{
    uint32 field_val_u32[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    
    if (field_id == DBAL_FIELD_L2_MAC)
    {
        utilex_pp_mac_address_struct_to_long((utilex_pp_mac_address_t *) field_val, field_val_u32);
    }
    else
    {
        dbal_field_basic_info_t field_info;

        dbal_fields_field_info_get(unit, field_id, &field_info);
        utilex_U8_to_U32(field_val, BITS2BYTES(field_info.max_size), field_val_u32);
    }
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, G_dbal_field_full_mask);
}

void
dbal_entry_key_field_arr32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 * field_val)
{
    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val, G_dbal_field_full_mask);
}

/** masked APIs  */

void
dbal_entry_key_field8_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 field_val,
    uint8 field_mask)
{
    uint32 field_val_u32[1], field_mask_as_uint32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    field_mask_as_uint32[0] = field_mask;

    if (entry_handle->table->access_method != DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, field_mask_as_uint32);
}

void
dbal_entry_key_field16_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint16 field_val,
    uint16 field_mask)
{
    uint32 field_val_u32[1], field_mask_as_uint32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    field_mask_as_uint32[0] = field_mask;

    if (entry_handle->table->access_method != DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, field_mask_as_uint32);
}

void
dbal_entry_key_field32_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 field_val,
    uint32 field_mask)
{
    uint32 field_val_u32[1], field_mask_as_uint32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    field_mask_as_uint32[0] = field_mask;

    if (entry_handle->table->access_method != DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, field_mask_as_uint32);
}

void
dbal_entry_key_field_arr8_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 * field_val,
    uint8 * field_mask)
{
    uint32 field_val_u32[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };
    uint32 field_mask_as_uint32[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    if (entry_handle->table->access_method != DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    utilex_U8_to_U32(field_val, DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES, field_val_u32);
    utilex_U8_to_U32(field_mask, DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES, field_mask_as_uint32);
    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val_u32, field_mask_as_uint32);
}

void
dbal_entry_key_field_arr32_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 * field_val,
    uint32 * field_mask)
{
    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    if (entry_handle->table->access_method != DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    dbal_entry_key_field_set(unit, entry_handle_id, field_id, field_val, field_mask);
}

/** value APIs   */

void
dbal_entry_value_field8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 field_val)
{
    uint32 field_val_u32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    dbal_entry_value_field_set(unit, entry_handle_id, field_id, field_val_u32);
}

void
dbal_entry_value_field16_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint16 field_val)
{
    uint32 field_val_u32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    dbal_entry_value_field_set(unit, entry_handle_id, field_id, field_val_u32);
}

void
dbal_entry_value_field32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 field_val)
{
    uint32 field_val_u32[1];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    field_val_u32[0] = field_val;
    dbal_entry_value_field_set(unit, entry_handle_id, field_id, field_val_u32);
}

void
dbal_entry_value_field_arr8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 * field_val)
{
    uint32 field_val_u32[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    utilex_U8_to_U32(field_val, DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES, field_val_u32);

    dbal_entry_value_field_set(unit, entry_handle_id, field_id, field_val_u32);
}

void
dbal_entry_value_field_arr32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 * field_val)
{
    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }

    dbal_entry_value_field_set(unit, entry_handle_id, field_id, field_val);
}

/***********GET APIs ***********/
void
dbal_entry_value_field_arr8_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES])
{
    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }
    dbal_entry_value_field_get(unit, DBAL_FIELD_TYPE_ARRAY8, entry_handle_id, field_id, field_val, NULL);
}

void
dbal_entry_value_field32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 * field_val)
{
    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }
    dbal_entry_value_field_get(unit, DBAL_FIELD_TYPE_UINT32, entry_handle_id, field_id, field_val, NULL);
}

void
dbal_entry_value_field_arr32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS])
{
    DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    if (inst_id == INST_ALL)
    {
        DBAL_FIELD_ERR_HANDLE("Illegal use for mask APIs in table type");
    }
    dbal_entry_value_field_get(unit, DBAL_FIELD_TYPE_ARRAY32, entry_handle_id, field_id, field_val, NULL);
}

shr_error_e
dbal_entry_handle_value_field32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 * field_val)
{
    dbal_field_basic_info_t field_info;
    dbal_entry_handle_t *entry_handle;
    SHR_FUNC_INIT_VARS(unit);

    DBAL_ENTRY_HANDLE_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);

    SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, field_id, &field_info));
    if (field_info.max_size > SAL_UINT32_NOF_BITS)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "field size too big for uint32, use field_arr32_get\n");
    }
    SHR_IF_ERR_EXIT(dbal_entry_handle_value_field_get(unit, entry_handle_id, field_id, inst_id, field_val));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_entry_handle_value_field_arr8_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint8 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES])
{
    uint32 field_val_u32[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
    dbal_entry_handle_t *entry_handle;

    SHR_FUNC_INIT_VARS(unit);

    DBAL_ENTRY_HANDLE_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    SHR_IF_ERR_EXIT(dbal_entry_handle_value_field_get(unit, entry_handle_id, field_id, inst_id, field_val_u32));
    SHR_IF_ERR_EXIT(utilex_U32_to_U8(field_val_u32, DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES, field_val));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_entry_handle_value_field_arr32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS])
{
    dbal_entry_handle_t *entry_handle;

    SHR_FUNC_INIT_VARS(unit);

    DBAL_ENTRY_HANDLE_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, inst_id);
    SHR_IF_ERR_EXIT(dbal_entry_handle_value_field_get(unit, entry_handle_id, field_id, inst_id, field_val));

exit:
    SHR_FUNC_EXIT;
}

/******* Entry operations **********/
shr_error_e
dbal_entry_get(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags)
{
    dbal_entry_handle_t *entry_handle;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_action_validations(unit, entry_handle_id, !(flags & DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE)));

    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle));
    table = entry_handle->table;

    if (flags & DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE)
    {
        entry_handle->get_all_fields = TRUE;
        flags |= DBAL_COMMIT_KEEP_HANDLE;
    }

    if (entry_handle->nof_result_types > 1)
    {
        if (!(flags & DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE))
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "multiple result table has to use get_all_fields flag\n");
        }
    }

    switch (table->access_method)
    {
        case DBAL_ACCESS_METHOD_PHY_TABLE:
            entry_handle->phy_entry.payload_size = table->max_payload_size;
            entry_handle->phy_entry.key_size = table->key_size;
            SHR_IF_ERR_EXIT(dbal_phy_table_entry_get(unit, table->physical_db_id, table->app_id,
                                                     &(entry_handle->phy_entry)));
            break;

        case DBAL_ACCESS_METHOD_HARD_LOGIC:
            SHR_IF_ERR_EXIT(dbal_direct_entry_get(unit, entry_handle, TRUE));
            break;

        case DBAL_ACCESS_METHOD_SW_ONLY:
            SHR_IF_ERR_EXIT(dbal_sw_table_entry_get(unit, entry_handle));
            break;

        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal access method %d \n", table->access_method);
            break;
    }

    if (!(flags & DBAL_COMMIT_DISABLE_ACTION_PRINTS))
    {
        SHR_IF_ERR_EXIT(dbal_action_prints(unit, entry_handle_id, flags, "GET"));
    }

    if (flags & DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE)
    {
        if (entry_handle->nof_result_types > 1)
        {
            SHR_IF_ERR_EXIT(dbal_set_result_type_from_buffer(unit, entry_handle));
        }
        entry_handle->handle_status = DBAL_HANDLE_STATUS_ACTION_PREFORMED;
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_value_fields_parse(unit, entry_handle));
    }

exit:
    if (!(flags & DBAL_COMMIT_KEEP_HANDLE))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
    }
    SHR_FUNC_EXIT;
}

/******* Entry operations **********/
shr_error_e
dbal_entry_commit(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags)
{
    dbal_entry_handle_t *entry_handle;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_action_validations(unit, entry_handle_id, 1));
    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle));
    table = entry_handle->table;

    SHR_IF_ERR_EXIT(dbal_action_prints(unit, entry_handle_id, flags, "COMMIT"));

    
    if ((table->max_capacity) && (table->nof_entries + 1 > table->max_capacity))
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Logical table is full %s\n", table->table_name);
    }
    switch (table->access_method)
    {
        case DBAL_ACCESS_METHOD_PHY_TABLE:
            entry_handle->phy_entry.payload_size = table->max_payload_size;
            entry_handle->phy_entry.key_size = table->key_size;
            SHR_IF_ERR_EXIT(dbal_phy_table_entry_add(unit, table->physical_db_id, table->app_id,
                                                     &(entry_handle->phy_entry)));
            break;

        case DBAL_ACCESS_METHOD_HARD_LOGIC:
            SHR_IF_ERR_EXIT(dbal_direct_entry_set(unit, entry_handle));
            break;

        case DBAL_ACCESS_METHOD_SW_ONLY:
            SHR_IF_ERR_EXIT(dbal_sw_table_entry_set(unit, entry_handle));
            break;

        default:
            break;
    }

exit:
    if (!(flags & DBAL_COMMIT_KEEP_HANDLE))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
    }
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_entry_clear(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags)
{
    dbal_entry_handle_t *entry_handle;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_action_validations(unit, entry_handle_id, 0));
    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle));
    table = entry_handle->table;

    SHR_IF_ERR_EXIT(dbal_action_prints(unit, entry_handle_id, flags, "CLEAR"));

    switch (table->access_method)
    {
        case DBAL_ACCESS_METHOD_PHY_TABLE:
            entry_handle->phy_entry.payload_size = table->max_payload_size;
            entry_handle->phy_entry.key_size = table->key_size;
            SHR_IF_ERR_EXIT(dbal_phy_table_entry_delete(unit, table->physical_db_id, table->app_id,
                                                        &(entry_handle->phy_entry)));
            break;

        case DBAL_ACCESS_METHOD_HARD_LOGIC:
            SHR_IF_ERR_EXIT(dbal_direct_entry_set_default(unit, entry_handle));
            break;

        case DBAL_ACCESS_METHOD_SW_ONLY:
            SHR_IF_ERR_EXIT(dbal_sw_table_entry_clear(unit, entry_handle));
            break;

        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal access method %d\n", table->access_method);
            break;
    }

exit:
    if (!(flags & DBAL_COMMIT_KEEP_HANDLE))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
    }
    SHR_FUNC_EXIT;
}

/********* table operations *********/
shr_error_e
dbal_table_clear(
    int unit,
    dbal_tables_e table_id)
{
    uint32 entry_handle_id;
    dbal_entry_handle_t *entry_handle;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_id));
    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle));
    table = entry_handle->table;

    SHR_IF_ERR_EXIT(dbal_action_prints(unit, entry_handle_id, 0, "TABLE CLEAR"));

    switch (table->access_method)
    {
        case DBAL_ACCESS_METHOD_PHY_TABLE:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Table clear in not implemented for MDB yet...\n");
            break;

        case DBAL_ACCESS_METHOD_HARD_LOGIC:
            SHR_IF_ERR_EXIT(dbal_direct_table_clear(unit, entry_handle_id));
            break;

        case DBAL_ACCESS_METHOD_SW_ONLY:
            SHR_IF_ERR_EXIT(dbal_sw_table_clear(unit, entry_handle_id));
            break;

        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal access method %d\n", table->access_method);
            break;
    }

exit:
    dbal_entry_handle_release(unit, entry_handle_id);
    SHR_FUNC_EXIT;
}

/******** iterator functions ********/
shr_error_e
dbal_table_iterator_init(
    int unit,
    dbal_tables_e table_id,
    dbal_iterator_mode_e mode,
    dbal_iterator_info_t * iterator_info)
{
    uint32 entry_handle_id;
    int alloc_key_fields;
    int alloc_val_fields;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_id));

    sal_memset(iterator_info, 0x0, sizeof(dbal_iterator_info_t));

    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &(iterator_info->entry_handle)));
    alloc_key_fields = iterator_info->entry_handle->table->nof_key_fields;
    alloc_val_fields = iterator_info->entry_handle->table->max_nof_result_fields;

    iterator_info->entry_handle_id = entry_handle_id;
    iterator_info->table_id = table_id;
    iterator_info->mode = mode;
    iterator_info->is_end = FALSE;
    iterator_info->nof_key_fields = alloc_key_fields;
    iterator_info->nof_result_fields = alloc_val_fields;

    iterator_info->keys_info = NULL;
    iterator_info->results_info = NULL;

    SHR_ALLOC_SET_ZERO(iterator_info->keys_info, sizeof(dbal_field_data_t) * alloc_key_fields,
                       "iterator_key_fields", "%s%s%s\r\n", EMPTY, EMPTY, EMPTY);
    SHR_ALLOC_SET_ZERO(iterator_info->results_info, sizeof(dbal_field_data_t) * alloc_val_fields,
                       "iterator_value_fields", "%s%s%s\r\n", EMPTY, EMPTY, EMPTY);

    SHR_IF_ERR_EXIT(dbal_iterator_init_handle_info(unit, iterator_info));

    /**  set additional parameters per table type  */
    switch (iterator_info->entry_handle->table->access_method)
    {
        case DBAL_ACCESS_METHOD_PHY_TABLE:
            SHR_IF_ERR_EXIT(dbal_phy_table_iterator_init(unit, iterator_info));
            break;

        case DBAL_ACCESS_METHOD_HARD_LOGIC:
            SHR_IF_ERR_EXIT(dbal_direct_table_iterator_init(unit, iterator_info));
            break;

        case DBAL_ACCESS_METHOD_SW_ONLY:
            SHR_IF_ERR_EXIT(dbal_sw_table_iterator_init(unit, iterator_info));
            break;

        default:
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_table_iterator_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    SHR_FUNC_INIT_VARS(unit);

    switch (iterator_info->entry_handle->table->access_method)
    {
        case DBAL_ACCESS_METHOD_PHY_TABLE:
            SHR_IF_ERR_EXIT(dbal_phy_table_entry_get_next(unit, iterator_info));
            break;

        case DBAL_ACCESS_METHOD_HARD_LOGIC:
            SHR_IF_ERR_EXIT(dbal_direct_entry_get_next(unit, iterator_info));
            break;

        case DBAL_ACCESS_METHOD_SW_ONLY:
            SHR_IF_ERR_EXIT(dbal_sw_table_entry_get_next(unit, iterator_info));
            break;

        default:
            break;
    }

    if (!iterator_info->is_end)
    {
        if (iterator_info->entry_handle->table->nof_result_types > 1)
        {
            SHR_IF_ERR_EXIT(dbal_set_result_type_from_buffer(unit, iterator_info->entry_handle));
            SHR_IF_ERR_EXIT(dbal_handle_update_requested_fields(unit, iterator_info->entry_handle,
                                                                iterator_info->results_info));
        }
        SHR_IF_ERR_EXIT(dbal_entry_handle_value_fields_parse(unit, iterator_info->entry_handle));
        iterator_info->entries_counter++;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_table_iterator_destroy(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    SHR_FUNC_INIT_VARS(unit);

    switch (iterator_info->entry_handle->table->access_method)
    {
        case DBAL_ACCESS_METHOD_PHY_TABLE:
            SHR_IF_ERR_EXIT(dbal_phy_table_iterator_deinit(unit, iterator_info));
            break;

        case DBAL_ACCESS_METHOD_HARD_LOGIC:
        case DBAL_ACCESS_METHOD_SW_ONLY:
        default:
            break;
    }

    SHR_FREE(iterator_info->keys_info);
    SHR_FREE(iterator_info->results_info);
    dbal_entry_handle_release_internal(unit, iterator_info->entry_handle_id);
    sal_memset(iterator_info, 0x0, sizeof(dbal_iterator_info_t));

exit:
    SHR_FUNC_EXIT;
}
