/*
 * $Id: dpp_dbal.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DBALDNX
#include <shared/bsl.h>
#include "dbal_internal.h"

static dbal_logical_tables_info_t logical_tables_info;

/**************************************************Internal usage APIs *************************************************/
shr_error_e
dbal_logical_tables_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_db_init_tables_set_default(unit, logical_tables_info.logical_tables));
    SHR_IF_ERR_EXIT(dbal_db_init_logical_tables(unit, DBAL_INIT_FLAGS_NONE, logical_tables_info.logical_tables,
                                                DBAL_ACCESS_METHOD_PHY_TABLE));
    SHR_IF_ERR_EXIT(dbal_db_init_logical_tables(unit, DBAL_INIT_FLAGS_NONE, logical_tables_info.logical_tables,
                                                DBAL_ACCESS_METHOD_HARD_LOGIC));
    SHR_IF_ERR_EXIT(dbal_db_init_logical_tables(unit, DBAL_INIT_FLAGS_NONE, logical_tables_info.logical_tables,
                                                DBAL_ACCESS_METHOD_SW_ONLY));
    SHR_IF_ERR_EXIT(dbal_db_init_logical_tables(unit, DBAL_INIT_FLAGS_NONE, logical_tables_info.logical_tables,
                                                DBAL_ACCESS_METHOD_PEMLA));
    SHR_IF_ERR_EXIT(dbal_db_init_tables_logical_validation(unit, logical_tables_info.logical_tables));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_logical_tables_deinit(
    int unit)
{
    int ii, jj, kk;
    dbal_logical_table_t *table_entry;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_TABLES; ii++)
    {
        table_entry = &logical_tables_info.logical_tables[ii];
        SHR_FREE(table_entry->keys_info);
        SHR_FREE(table_entry->multi_res_info->results_info);
        SHR_FREE(table_entry->multi_res_info);

        if (table_entry->access_method == DBAL_ACCESS_METHOD_HARD_LOGIC)
        {
            for (jj = 0; jj < table_entry->nof_result_types; jj++)
            {
                for (kk = 0; kk < DBAL_NOF_HL_ACCESS_TYPES; kk++)
                {
                    if (table_entry->hl_mapping_multi_res[jj].l2p_direct_info[kk].num_of_access_fields > 0)
                    {
                        SHR_FREE(table_entry->hl_mapping_multi_res[jj].l2p_direct_info[kk].l2p_fields_info);
                    }
                }
            }
            SHR_FREE(table_entry->hl_mapping_multi_res);
        }
        else if (table_entry->access_method == DBAL_ACCESS_METHOD_PEMLA)
        {
            SHR_FREE(table_entry->pemla_mapping.key_fields_mapping);
            SHR_FREE(table_entry->pemla_mapping.result_fields_mapping);
        }
    }
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_logical_table_reference_get(
    int unit,
    dbal_tables_e table_id,
    dbal_logical_table_t ** table)
{
    (*table) = &logical_tables_info.logical_tables[table_id];
    return 0;
}

shr_error_e
dbal_table_field_is_key(
    int unit,
    dbal_tables_e table_id,
    dbal_fields_e field_id,
    uint8 * is_key)
{
    int field_index;
    int is_sub_field;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    table = &logical_tables_info.logical_tables[table_id];
    SHR_IF_ERR_EXIT(dbal_fields_is_sub_field(unit, field_id, &is_sub_field));

    *is_key = FALSE;

    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        if (table->keys_info[field_index].field_id == field_id)
        {
            *is_key = TRUE;
            break;
        }
        if (is_sub_field)
        {
            int is_found = 0;
            dbal_fields_sub_field_match(unit, table->keys_info[field_index].field_id, field_id, &is_found);
            if (is_found)
            {
                *is_key = TRUE;
                break;
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_table_default_entry_get(
    int unit,
    dbal_tables_e table_id,
    uint32 * payload_buffer)
{
    SHR_FUNC_INIT_VARS(unit);

    
    sal_memset(payload_buffer, 0, DBAL_PHYSICAL_RES_SIZE_IN_WORDS);

    SHR_FUNC_EXIT;
}

/**************************************************General usage APIs (outside of dbal *************************************************/
shr_error_e
dbal_logical_table_get(
    int unit,
    dbal_tables_e table_id,
    CONST dbal_logical_table_t ** table)
{
    (*table) = &logical_tables_info.logical_tables[table_id];
    return 0;
}

shr_error_e
dbal_table_field_info_get(
    int unit,
    dbal_tables_e table_id,
    dbal_fields_e field_id,
    uint8 is_key,
    int result_type_idx,
    dbal_table_field_info_t * field_info)
{
    int iter;
    dbal_logical_table_t *table;
    int is_sub_field;
    int nof_fields;
    dbal_table_field_info_t *fields_db;

    SHR_FUNC_INIT_VARS(unit);

    field_info->field_id = DBAL_FIELD_EMPTY;
    table = &logical_tables_info.logical_tables[table_id];
    SHR_IF_ERR_EXIT(dbal_fields_is_sub_field(unit, field_id, &is_sub_field));

    if (is_key)
    {
        nof_fields = table->nof_key_fields;
        fields_db = table->keys_info;
    }
    else
    {
        nof_fields = table->multi_res_info[result_type_idx].nof_result_fields;
        fields_db = table->multi_res_info[result_type_idx].results_info;
    }

    for (iter = 0; iter < nof_fields; iter++)
    {
        if (fields_db[iter].field_id == field_id)
        {
            *field_info = fields_db[iter];
            break;
        }
        if (is_sub_field)
        {
            int is_found = 0;
            dbal_fields_sub_field_match(unit, fields_db[iter].field_id, field_id, &is_found);
            if (is_found)
            {
                *field_info = fields_db[iter];
                break;
            }
        }
    }

    if (field_info->field_id == DBAL_FIELD_EMPTY)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "field %s not found in table %-20s \n", dbal_field_to_string(unit, field_id),
                     table->table_name);
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e dbal_table_get_app_db_id(
    int unit,
    dbal_tables_e table_id,
    uint32 * app_db_id)
{
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    table = &logical_tables_info.logical_tables[table_id];

    if (table->access_method != DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "dbal_table_get_app_db_id valid only for MDB tables.called with table:%s \n",
                     table->table_name);
    }
    else
    {
        *app_db_id = table->app_id;
    }

exit:
    SHR_FUNC_EXIT;
}
