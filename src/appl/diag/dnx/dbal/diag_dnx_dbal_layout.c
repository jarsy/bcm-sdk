/** \file diag_dnx_dbal_layout.c
 *
 * Main DBAL layout functions.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DIAGDBALDNX

/*************
 * INCLUDES  *
 *************/
#include <sal/appl/sal.h>
#include <shared/bslnames.h>
#include <soc/mcm/memregs.h>
#include <appl/diag/system.h>
#include <appl/diag/diag.h>
#include "diag_dnx_dbal_internal.h"
#include <shared/utilex/utilex_bitstream.h>
#include <shared/utilex/utilex_framework.h>

void
diag_dbal_offset_encode_info_to_str(
    int unit,
    CONST dbal_offset_encode_info_t * encode_info,
    char *str)
{
    char *param_str, *encode_str;

    sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s", "");

    encode_str = dbal_offset_encode_type_to_string(unit, encode_info->encode_mode);

    param_str = dbal_offset_encode_conjunction_string(unit, encode_info->encode_mode);
    if (sal_strcasecmp(param_str, ""))
    {
        if (encode_info->encode_mode == DBAL_VALUE_OFFSET_ENCODE_PARTIAL_KEY)
        {
            sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s %s", encode_str,
                         dbal_field_to_string(unit, encode_info->field_id));
        }
        else
        {
            sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s %s %d", encode_str, param_str, encode_info->input_param);
        }
    }
    else
    {
        if (encode_info->encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
        {
            sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s ", encode_str);
        }
    }
}

STATIC void
diag_dbal_conditions_string_build(
    int unit,
    CONST dbal_access_condition_info_t condition[DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS],
    char *str)
{
    int ii, is_printed = 0, offset;

    sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s", "NONE");

    /** support params print for conditions */
    for (ii = 0; ii < DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS; ii++)
    {
        if (condition[ii].type != DBAL_CONDITION_NONE)
        {
            if (is_printed == 1)
            {
                offset = sal_snprintf(str, CMD_MAX_STRING_LENGTH, ", %s",
                                      dbal_condition_to_string(unit, condition[ii].type));
            }
            else
            {
                offset = sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s",
                                      dbal_condition_to_string(unit, condition[ii].type));
                is_printed = 1;
            }
            str += offset;
        }
    }
}

void
diag_dbal_lables_string_build(
    int unit,
    CONST dbal_labels_e labels[DBAL_MAX_NOF_ENTITY_LABEL_TYPES],
    char *str)
{
    int ii, is_label_printed = 0, offset = 0;

    sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s", " NONE");

    /** support params print for conditions */
    for (ii = 0; ii < DBAL_MAX_NOF_ENTITY_LABEL_TYPES; ii++)
    {
        if (labels[ii] != DBAL_LABEL_NONE)
        {
            if (is_label_printed == 1)
            {
                offset = sal_snprintf(str, CMD_MAX_STRING_LENGTH, ", %s", dbal_label_to_string(unit, labels[ii]));
            }
            else
            {
                offset = sal_snprintf(str, CMD_MAX_STRING_LENGTH, "%s", dbal_label_to_string(unit, labels[ii]));
                is_label_printed = 1;
            }
            str += offset;
        }
    }
}

STATIC shr_error_e
diag_dbal_hard_logic_direct_hw_info_dump(
    int unit,
    CONST dbal_direct_l2p_info_t * l2p_direct_info,
    sh_sand_control_t * sand_control)
{
    char *memory_name;
    char *alias_name = NULL;
    char *field_name;
    int jj, ii, to_print = 0, is_memory;
    char array_encode_info_string[CMD_MAX_STRING_LENGTH];
    char block_encode_info_string[CMD_MAX_STRING_LENGTH];
    char entry_encode_info_string[CMD_MAX_STRING_LENGTH];
    char field_encode_info_string[CMD_MAX_STRING_LENGTH];
    char alias_encode_info_string[CMD_MAX_STRING_LENGTH];
    char condition_info_string[CMD_MAX_STRING_LENGTH];
    uint8 has_alias;

    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_HL_ACCESS_TYPES; ii++)
    {
        if (ii == DBAL_HL_ACCESS_SW)
        {
            continue;
        }
        if (l2p_direct_info[ii].num_of_access_fields != 0)
        {
            to_print = 1;
            break;
        }
    }

    if (to_print)
    {
        PRT_TITLE_SET("Direct access logical to physical info");

        PRT_COLUMN_ADD("Field Name");
        PRT_COLUMN_ADD("bit size");
        PRT_COLUMN_ADD("offset");
        PRT_COLUMN_ADD("Mapped to Mem/Reg");
        PRT_COLUMN_ADD("Aliasing");
        PRT_COLUMN_ADD("Aliasing offset");
        PRT_COLUMN_ADD("HW field");
        PRT_COLUMN_ADD("Array offset");
        PRT_COLUMN_ADD("Block index");
        PRT_COLUMN_ADD("Entry offset");
        PRT_COLUMN_ADD("Field/data offset");
        PRT_COLUMN_ADD("Condition");

        for (ii = 0; ii < DBAL_NOF_HL_ACCESS_TYPES; ii++)
        {
            if (ii == DBAL_HL_ACCESS_SW)
            {
                continue;
            }
            for (jj = 0; jj < l2p_direct_info[ii].num_of_access_fields; jj++)
            {
                has_alias = FALSE;
                if (ii == DBAL_HL_ACCESS_MEMORY)
                {
                    is_memory = 1;
                    memory_name = SOC_MEM_NAME(unit, l2p_direct_info[ii].l2p_fields_info[jj].memory);
                    if (l2p_direct_info[ii].l2p_fields_info[jj].alias_memory != 0)
                    {
                        alias_name = SOC_MEM_NAME(unit, l2p_direct_info[ii].l2p_fields_info[jj].alias_memory);
                        diag_dbal_offset_encode_info_to_str(unit,
                                                            &(l2p_direct_info[ii].
                                                              l2p_fields_info[jj].alias_data_offset_info),
                                                            alias_encode_info_string);
                        has_alias = TRUE;
                    }
                }
                else if (ii == DBAL_HL_ACCESS_REGISTER)
                {
                    is_memory = 0;
                    memory_name = SOC_REG_NAME(unit, l2p_direct_info[ii].l2p_fields_info[jj].reg[0]);
                    if (l2p_direct_info[ii].l2p_fields_info[jj].alias_reg[0] != 0)
                    {
                        alias_name = SOC_REG_NAME(unit, l2p_direct_info[ii].l2p_fields_info[jj].alias_reg[0]);
                        diag_dbal_offset_encode_info_to_str(unit,
                                                            &(l2p_direct_info[ii].
                                                              l2p_fields_info[jj].alias_data_offset_info),
                                                            alias_encode_info_string);
                        has_alias = TRUE;
                    }
                }
                else
                {
                    LOG_ERROR_EX(BSL_LOG_MODULE, "\n access type not parsed %d %s %s %s\n", ii, EMPTY, EMPTY, EMPTY);
                    SHR_EXIT();
                }

                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET("%s", dbal_field_to_string(unit, l2p_direct_info[ii].l2p_fields_info[jj].field_id));
                PRT_CELL_SET("%d", l2p_direct_info[ii].l2p_fields_info[jj].nof_bits_in_interface);
                PRT_CELL_SET("%d", l2p_direct_info[ii].l2p_fields_info[jj].offset_in_interface);
                PRT_CELL_SET("%s%s", memory_name, is_memory ? "m" : "r");
                if (has_alias)
                {
                    PRT_CELL_SET("%s", alias_name);
                    PRT_CELL_SET("%s", alias_encode_info_string);
                }
                else
                {
                    PRT_CELL_SET("%s", "-");
                    PRT_CELL_SET("%s", "-");
                }
                if (l2p_direct_info[ii].l2p_fields_info[jj].hw_field != 0)
                {
                    field_name = SOC_FIELD_NAME(unit, l2p_direct_info[ii].l2p_fields_info[jj].hw_field);
                    PRT_CELL_SET("%s", field_name);
                }
                else
                {
                    PRT_CELL_SET("No Hw field");
                }

                diag_dbal_offset_encode_info_to_str(unit,
                                                    &(l2p_direct_info[ii].l2p_fields_info[jj].array_offset_info),
                                                    array_encode_info_string);
                PRT_CELL_SET("%s", array_encode_info_string);

                diag_dbal_offset_encode_info_to_str(unit,
                                                    &(l2p_direct_info[ii].l2p_fields_info[jj].block_index_info),
                                                    block_encode_info_string);
                PRT_CELL_SET("%s", block_encode_info_string);

                diag_dbal_offset_encode_info_to_str(unit, &(l2p_direct_info[ii].l2p_fields_info[jj].entry_offset_info),
                                                    entry_encode_info_string);
                if (!sal_strcasecmp(entry_encode_info_string, ""))
                {
                    sal_snprintf(entry_encode_info_string, CMD_MAX_STRING_LENGTH, "%s", "KEY");
                }
                PRT_CELL_SET("%s", entry_encode_info_string);

                diag_dbal_offset_encode_info_to_str(unit, &(l2p_direct_info[ii].l2p_fields_info[jj].data_offset_info),
                                                    field_encode_info_string);
                if (!sal_strcasecmp(field_encode_info_string, ""))
                {
                    sal_snprintf(field_encode_info_string, CMD_MAX_STRING_LENGTH, "%s", "NONE");
                }
                PRT_CELL_SET("%s", field_encode_info_string);

                diag_dbal_conditions_string_build(unit, (l2p_direct_info[ii].l2p_fields_info[jj].mapping_condition),
                                                  condition_info_string);
                PRT_CELL_SET("%s", condition_info_string);
            }
        }

        PRT_COMMITX;
        LOG_CLI((BSL_META("\n")));
    }

exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

STATIC shr_error_e
diag_dbal_hard_logic_direct_sw_info_dump(
    int unit,
    CONST dbal_logical_table_t * table,
    CONST dbal_direct_l2p_info_t * l2p_direct_info,
    sh_sand_control_t * sand_control)
{
    int jj;

    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    PRT_TITLE_SET("SW access logical to physical info");

    PRT_COLUMN_ADD("Field Name");
    PRT_COLUMN_ADD("size in SW buffer [Bytes]");
    PRT_COLUMN_ADD("offset in SW buffer [Bytes]");

    for (jj = 0; jj < l2p_direct_info[DBAL_ACCESS_METHOD_SW_ONLY].num_of_access_fields; jj++)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", dbal_field_to_string(unit,
                                                l2p_direct_info[DBAL_ACCESS_METHOD_SW_ONLY].
                                                l2p_fields_info[jj].field_id));
        PRT_CELL_SET("%d",
                     BITS2BYTES(table->
                                multi_res_info[0].results_info[l2p_direct_info[DBAL_ACCESS_METHOD_SW_ONLY].
                                                               l2p_fields_info[jj].field_pos_in_interface].
                                field_nof_bits));
        PRT_CELL_SET("%d",
                     table->multi_res_info[0].results_info[l2p_direct_info[DBAL_ACCESS_METHOD_SW_ONLY].
                                                           l2p_fields_info[jj].field_pos_in_interface].
                     bytes_offset_in_sw_buffer);
    }

    PRT_COMMIT;
    LOG_CLI((BSL_META("\n")));

exit:
    SHR_FUNC_EXIT;
}

STATIC shr_error_e
diag_dbal_table_key_fields_dump(
    int unit,
    CONST dbal_logical_table_t * table,
    sh_sand_control_t * sand_control)
{
    int ii;

    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    PRT_TITLE_SET("Key-fields");

    PRT_COLUMN_ADD("Field Name");
    PRT_COLUMN_ADD("Size");
    PRT_COLUMN_ADD("Buffer offset");

    /** Print Key fields */
    for (ii = 0; ii < table->nof_key_fields; ii++)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", dbal_field_to_string(unit, table->keys_info[ii].field_id));
        PRT_CELL_SET("%d", table->keys_info[ii].field_nof_bits);
        if (table->keys_info[ii].field_id == DBAL_FIELD_CORE_ID)
        {
            PRT_CELL_SET("%s", "-");
        }
        else
        {
            PRT_CELL_SET("%d", table->keys_info[ii].bits_offset_in_buffer);
        }
    }

    PRT_COMMIT;

exit:
    SHR_FUNC_EXIT;
}

STATIC shr_error_e
diag_dbal_table_result_fields_dump(
    int unit,
    CONST dbal_logical_table_t * table,
    int result_idx,
    sh_sand_control_t * sand_control)
{
    int ii;

    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    if (table->nof_result_types > 1)
    {
        PRT_TITLE_SET("result type=%s", table->multi_res_info[result_idx].result_type_name);
    }
    else
    {
        /** Title set is mandatory, set to empty srting  */
        PRT_TITLE_SET("");
    }

    PRT_COLUMN_ADD("Field Name");
    PRT_COLUMN_ADD("Size");
    PRT_COLUMN_ADD("Buffer offset");

    if (table->access_method == DBAL_ACCESS_METHOD_SW_ONLY)
    {
        PRT_COLUMN_ADD("SW buffer offset [bytes]");
    }

    /** Print Results fields */
    for (ii = 0; ii < table->multi_res_info[result_idx].nof_result_fields; ii++)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", dbal_field_to_string(unit, table->multi_res_info[result_idx].results_info[ii].field_id));
        PRT_CELL_SET("%d", table->multi_res_info[result_idx].results_info[ii].field_nof_bits);
        PRT_CELL_SET("%d", table->multi_res_info[result_idx].results_info[ii].bits_offset_in_buffer);

        if (table->access_method == DBAL_ACCESS_METHOD_SW_ONLY)
        {
            PRT_CELL_SET("%d", table->multi_res_info[result_idx].results_info[ii].bytes_offset_in_sw_buffer);
        }
    }
    if (table->multi_res_info[result_idx].zero_padding > 0)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", "Zero Padding (not a real field)");
        PRT_CELL_SET("%d", table->multi_res_info[result_idx].zero_padding);
        PRT_CELL_SET("%d", 0);
        if (table->access_method == DBAL_ACCESS_METHOD_SW_ONLY)
        {
            PRT_CELL_SET("%d", 0);
        }
    }

    PRT_COMMIT;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
diag_dbal_logical_table_dump(
    int unit,
    dbal_tables_e table_id,
    int print_mode,             /* 0=partial, 1=full */
    sh_sand_control_t * sand_control)
{
    CONST dbal_logical_table_t *table;
    int ii;
    char str[CMD_MAX_STRING_LENGTH];

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    if (table->is_table_initiated == 0)
    {
        LOG_ERROR_EX(BSL_LOG_MODULE, "\n table is not initiated %s %s%s%s", table->table_name, EMPTY, EMPTY, EMPTY);
        SHR_EXIT();
    }

    DIAG_DBAL_HEADER_DUMP("Printing info for logical table", table->table_name);

    LOG_CLI((BSL_META("\tAccess type: %-10s "), dbal_access_method_to_string(unit, table->access_method)));

    if (table->access_method == DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        LOG_CLI((BSL_META("Name: %-20s"), dbal_physical_table_to_string(unit, table->physical_db_id)));
    }
    LOG_CLI((BSL_META("\n")));

    LOG_CLI((BSL_META("\tMaturity_level: %s \n"),
             (table->maturity_level == 1) ? "not running regression" : "regressions passed on this table"));

    diag_dbal_lables_string_build(unit, table->labels, str);
    LOG_CLI((BSL_META("\tTable Labels: %s\n"), str));

    LOG_CLI((BSL_META("\tCore mode: %s\n"), dbal_core_mode_to_string(unit, table->core_mode)));

    LOG_CLI((BSL_META("Table fields: \n")));
    SHR_IF_ERR_EXIT(diag_dbal_table_key_fields_dump(unit, table, sand_control));
    LOG_CLI((BSL_META("\n")));
    LOG_CLI((BSL_META("Result-fields: \n")));
    for (ii = 0; ii < table->nof_result_types; ii++)
    {
        SHR_IF_ERR_EXIT(diag_dbal_table_result_fields_dump(unit, table, ii, sand_control));
    }
    LOG_CLI((BSL_META("\n")));

    /*
     * LOG_CLI((BSL_META("\n\tTotal payload size: %d bits\n\n"), table->entry_payload_size));
     */

    if (print_mode == 1)
    {
        if (table->access_method == DBAL_ACCESS_METHOD_HARD_LOGIC)
        {
            for (ii = 0; ii < table->nof_result_types; ii++)
            {
                if (table->nof_result_types > 1)
                {
                    LOG_CLI((BSL_META("Direct mapping of result type %s\n"),
                             table->multi_res_info[ii].result_type_name));
                }
                if ((table->hl_mapping_multi_res[ii].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].num_of_access_fields > 0) ||
                    (table->hl_mapping_multi_res[ii].l2p_direct_info[DBAL_HL_ACCESS_REGISTER].num_of_access_fields > 0))
                {
                    LOG_CLI((BSL_META("HW Mapping:\n")));
                    SHR_IF_ERR_EXIT(diag_dbal_hard_logic_direct_hw_info_dump
                                    (unit, table->hl_mapping_multi_res[ii].l2p_direct_info, sand_control));
                    LOG_CLI((BSL_META("\n")));
                }

                if (table->hl_mapping_multi_res[ii].l2p_direct_info[DBAL_HL_ACCESS_SW].num_of_access_fields > 0)
                {
                    LOG_CLI((BSL_META("SW Mapping:\n")));
                    SHR_IF_ERR_EXIT(diag_dbal_hard_logic_direct_sw_info_dump
                                    (unit, table, table->hl_mapping_multi_res[ii].l2p_direct_info, sand_control));
                }
            }
        }
        else if (table->access_method == DBAL_ACCESS_METHOD_PEMLA)
        {
            LOG_CLI((BSL_META("PEMLA Mapping:\n")));
            LOG_CLI((BSL_META("DB Mapping: %d\n"), table->pemla_db_id));
            for (ii = 0; ii < table->nof_key_fields; ii++)
            {
                LOG_CLI((BSL_META("Key field %02d mapped to HW value %02d\n"), ii,
                         table->pemla_mapping.key_fields_mapping[ii]));
            }
            for (ii = 0; ii < table->multi_res_info[0].nof_result_fields; ii++)
            {
                LOG_CLI((BSL_META("Result field %02d mapped to HW value %02d\n"), ii,
                         table->pemla_mapping.result_fields_mapping[ii]));
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_table_iterator_info_dump(
    int unit,
    dbal_iterator_info_t * iterator_info)
{

    CONST dbal_logical_table_t *table = NULL;
    int iter;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, iterator_info->table_id, &table));

    DIAG_DBAL_SUBHEADER_DUMP("Dumping iterator information", "\0");

    LOG_CLI((BSL_META("table %s, num of key fields = %d, num of result fields = %d, is end %d\n"),
             table->table_name, iterator_info->nof_key_fields, iterator_info->nof_result_fields,
             iterator_info->is_end));

    LOG_CLI((BSL_META("max_num_of_iterations = 0x")));
    for (iter = iterator_info->direct_iterator.key_size_in_words - 1; iter > 0; iter--)
    {
        LOG_CLI((BSL_META("%08x"), iterator_info->direct_iterator.max_num_of_iterations[iter]));
    }
    LOG_CLI((BSL_META("%08x\n"), iterator_info->direct_iterator.max_num_of_iterations[0]));

    LOG_CLI((BSL_META("current key buffer = 0x")));
    for (iter = iterator_info->direct_iterator.key_size_in_words - 1; iter > 0; iter--)
    {
        LOG_CLI((BSL_META("%08x"), iterator_info->entry_handle->phy_entry.key[iter]));
    }
    LOG_CLI((BSL_META("%08x\n"), iterator_info->entry_handle->phy_entry.key[0]));

    LOG_CLI((BSL_META("core id to check = %d"), iterator_info->entry_handle->core_id));

    LOG_CLI((BSL_META("requested fields information \n")));

    LOG_CLI((BSL_META("\tkey fields: \n")));
    for (iter = 0; iter < iterator_info->nof_key_fields; iter++)
    {
        LOG_CLI((BSL_META("\t\tname %s pointer %p \n"),
                 dbal_field_to_string(unit, iterator_info->keys_info[iter].field_id),
                 iterator_info->keys_info[iter].field_val));
    }

    LOG_CLI((BSL_META("\tresult fields: \n")));
    for (iter = 0; iter < iterator_info->nof_result_fields; iter++)
    {
        LOG_CLI((BSL_META("\t\tname %s pointer %p \n"),
                 dbal_field_to_string(unit, iterator_info->results_info[iter].field_id),
                 iterator_info->results_info[iter].field_val));
    }

    LOG_CLI((BSL_META("\n")));

exit:
    SHR_FUNC_EXIT;
}
