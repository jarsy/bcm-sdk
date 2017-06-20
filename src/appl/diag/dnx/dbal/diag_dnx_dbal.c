/** \file diag_dnx_dbal.c
 *
 * Main diagnostics for dbal applications All CLI commands, that are related to DBAL, are gathered in this file.
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
#include <soc/dnx/dbal/dbal.h>
#include <soc/dnx/dbal/dbal_structures.h>
#include "diag_dnx_dbal_internal.h"
#include <appl/diag/shell.h>
#include <appl/diag/cmdlist.h>
#include <appl/diag/sand/diag_sand_framework.h>
#include <appl/diag/sand/diag_sand_access.h>

#include "../diag_dnx_cmdlist.h"

extern sh_sand_man_t sh_dnx_data_man;
extern sh_sand_cmd_t sh_dnx_data_cmds[];
/*************
 * TYPEDEFS  *
 *************/
#define DNX_DBAL_LOGGER_OFF bslSeverityWarn
#define DNX_DBAL_LOGGER_NORMAL bslSeverityInfo
#define DNX_DBAL_LOGGER_HIGH bslSeverityVerbose
#define DNX_DBAL_LOGGER_PRINT (DNX_DBAL_LOGGER_HIGH + 1)

/*************
* FUNCTIONS *
*************/

/**
* \brief
* DBAL usage message according to all the diagnostic options
***********************/
char cmd_dnx_dbal_usage[] = "Database Abstraction Layer diagnostics:\n" "For DBAL Unit Test use tr 201 command\n";

 /*
  * LOCAL DIAG PACK:
  * {
  */
/**********************************************************************************************************************
 *  DBAL DIAGNOSTIC PACK:
 *  STRUCTURE:
 *  MAIN MENU CMD: define under sh_dnx_dbal_cmds
 *     TABLE - SUB MENU defined under sh_dnx_dbal_table_cmds
 *     ENTRY - SUB MENU defined under sh_dnx_dbal_entry_cmds
 *     FIELD - SUB MENU defined under sh_dnx_dbal_fields_cmds
 *     LABELSDUMP 
 *     LOGSEVERITY
 *     HANDLESSTATUS
 **********************************************************************************************************************/

/**********************************************************************************************************************
 *  DBAL DIAGNOSTIC PACK:
 *  TABLE SUB MENU function & infrastructure Definitions - START
 *  STURCTURE:
 *  1. cmd function definition
 *  2. cmd option and man table
 *  3. TABLE SUB MENU cmd table
 **********************************************************************************************************************/
static shr_error_e
dbal_tables_list_dump(
    int unit,
    dbal_labels_e label,
    sh_sand_control_t * sand_control)
{
    int ii, jj;
    CONST dbal_logical_table_t *table;

    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    if (label == DBAL_LABEL_NONE)
    {
        PRT_TITLE_SET("LOGICAL TABLES");
    }
    else
    {
        PRT_TITLE_SET("LOGICAL TABLES RELATED TO LABEL:%s", dbal_label_to_string(unit, label));
    }

    PRT_COLUMN_ADD("Table Name");
    PRT_COLUMN_ADD("Access Type");

    for (ii = 0; ii < DBAL_NOF_TABLES; ii++)
    {
        SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, ii, &table));
        if ((table->maturity_level == DBAL_MATURITY_LOW) || (table->is_table_valid == FALSE))
        {
            continue;
        }
        if (label == DBAL_LABEL_NONE)
        {
            PRT_ROW_ADD(PRT_ROW_SEP_NONE);
            PRT_CELL_SET("%s", table->table_name);
            PRT_CELL_SET("%s", dbal_access_method_to_string(unit, table->access_method));
        }
        else
        {
            for (jj = 0; jj < DBAL_MAX_NOF_ENTITY_LABEL_TYPES; jj++)
                if ((table->labels[jj] == label))
                {
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SET("%s", table->table_name);
                    PRT_CELL_SET("%s", dbal_access_method_to_string(unit, table->access_method));
                    break;
                }
        }
    }

    PRT_COMMITX;

exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

/**
* \brief
* Return the next table id that matches tonthe input string, (next refers to the curr_table_id) 
* if the subsstring is fully match to the table name  than is_full_match=1 
* use curr_table_id = -1 to start form the first existing table
*****************************************************/
static cmd_result_t
dbal_string_to_next_table_id_get(
    int unit,
    char *substr_match,
    dbal_tables_e curr_table_id,
    dbal_tables_e * table_id,
    int *is_full_match)
{
    int iter;
    CONST dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    *table_id = DBAL_NOF_TABLES;
    *is_full_match = 0;

    curr_table_id++;

    if ((substr_match == NULL) || (curr_table_id > DBAL_NOF_TABLES))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Illegal input substr_match not exists or table_id too big\n");
    }

    for (iter = curr_table_id; iter < DBAL_NOF_TABLES; iter++)
    {
        SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, iter, &table));

        if (!sal_strcasecmp(substr_match, table->table_name))
        {
            *is_full_match = 1;
            *table_id = iter;
            break;
        }

        if (strcaseindex(table->table_name, substr_match))
        {
            *table_id = iter;
            break;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief 
* dump logical tables. 
* input arg should be valid label_name / table_name. 
* dump all logical tables names that related to specific label_id, 
* if label_id = DBAL_LABEL_NONE dump all tables.
* if table_name match to more than one table name dump brief information of all matching tables 
* if full match valid table_name exist dump extended table information
*****************************************************/
static shr_error_e
cmd_dnx_dbal_tables_info(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    CONST dbal_logical_table_t *table;
    int is_full_match = 0;
    dbal_tables_e table_id;
    dbal_tables_e first_table_id = -1;
    dbal_labels_e label;
    char *table_name = NULL;
    char *label_name = NULL;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("table", table_name);
    SH_SAND_GET_STR("label", label_name);

    if (ISEMPTY(table_name))
    {
        if (ISEMPTY(label_name))
        {
            label = DBAL_LABEL_NONE;
        }
        else
        {
            SHR_IF_ERR_EXIT(dbal_label_string_to_id(unit, label_name, &label));
        }
        dbal_tables_list_dump(unit, label, sand_control);
    }
    else
    {
        dbal_string_to_next_table_id_get(unit, table_name, -1, &table_id, &is_full_match);
        if (table_id == DBAL_NOF_TABLES)
        {
            LOG_CLI((BSL_META("no matching tables found\n\n")));
        }

        if (is_full_match)
        {
            SHR_IF_ERR_EXIT(diag_dbal_logical_table_dump(unit, table_id, 1, sand_control));
            SHR_EXIT();
        }

        first_table_id = table_id;
        dbal_string_to_next_table_id_get(unit, table_name, table_id, &table_id, &is_full_match);
        if (table_id == DBAL_NOF_TABLES)
        {
            /*
             * only one table that match to string - print full table info 
             */
            SHR_IF_ERR_EXIT(diag_dbal_logical_table_dump(unit, first_table_id, 1, sand_control));
            SHR_EXIT();
        }
        else
        {
            /*
             * more than one table found, print the first table name 
             */
            SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, first_table_id, &table));
            LOG_CLI((BSL_META("\t%s \n"), table->table_name));
        }

        while (table_id != DBAL_NOF_TABLES)
        {
            SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
            LOG_CLI((BSL_META("\t%s \n"), table->table_name));

            dbal_string_to_next_table_id_get(unit, table_name, table_id, &table_id, &is_full_match);
        }
    }
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - clear a full DBAL tabl
 */
static shr_error_e
cmd_dnx_dbal_table_clear(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dbal_tables_e table_id;
    char *table_name = NULL;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("table", table_name);

    if (ISEMPTY(table_name))
    {
        dbal_tables_list_dump(unit, DBAL_LABEL_NONE, sand_control);
        DIAG_DBAL_HEADER_DUMP("Example: dbal table Clear Table=<table_name>", "\0");
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(dbal_logical_table_string_to_id(unit, table_name, &table_id));

    SHR_IF_ERR_EXIT(dbal_table_clear(unit, table_id));
exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
* dump all entries that related to a specific table. 
* this function is using iterator to dump all the entries. 
* input parameter "table name" 
*******************/
static shr_error_e
cmd_dbal_tables_entries_dump(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    CONST dbal_logical_table_t *table;
    dbal_tables_e table_id = 0;
    int iter;
    char *table_name;
    dbal_iterator_info_t iterator_info;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("table", table_name);

    if (ISEMPTY(table_name))
    {
        dbal_tables_list_dump(unit, DBAL_LABEL_NONE, sand_control);
        DIAG_DBAL_HEADER_DUMP("Example: dbal table Entries Table=<table_name>", "\0");
        SHR_EXIT();
    }
    SHR_IF_ERR_EXIT(dbal_logical_table_string_to_id(unit, table_name, &table_id));

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    DIAG_DBAL_HEADER_DUMP("Dumping all entries related to table", table->table_name);

    SHR_IF_ERR_EXIT(dbal_table_iterator_init(unit, table_id,
                                             DBAL_ITER_MODE_GET_ALL_BUT_DEFAULT_ENTRIES, &iterator_info));

    /*
     * SHR_IF_ERR_EXIT(dbal_table_iterator_info_dump(unit,&iterator_info)); 
     */

    SHR_IF_ERR_EXIT(dbal_table_iterator_get_next(unit, &iterator_info));

    while (!iterator_info.is_end)
    {
        LOG_CLI((BSL_META("entry %-3d: KEY:: "), iterator_info.entries_counter));
        for (iter = 0; iter < iterator_info.nof_key_fields; iter++)
        {
            dbal_field_from_buffer_print(unit, iterator_info.keys_info[iter].field_id,
                                         table_id, iterator_info.keys_info[iter].field_val, NULL, 0, TRUE, FALSE);
            if (iter < iterator_info.nof_key_fields - 1)
            {
                LOG_CLI((BSL_META(",")));
            }
        }

        LOG_CLI((BSL_META("\n\t   VALUE:: ")));
        for (iter = 0; iter < iterator_info.nof_result_fields; iter++)
        {
            dbal_field_from_buffer_print(unit, iterator_info.results_info[iter].field_id,
                                         table_id, iterator_info.results_info[iter].field_val,
                                         NULL, iterator_info.entry_handle->cur_res_type, FALSE, FALSE);
            if (iter < iterator_info.nof_result_fields - 1)
            {
                LOG_CLI((BSL_META(",")));
            }
        }
        LOG_CLI((BSL_META("\n")));
        SHR_IF_ERR_EXIT(dbal_table_iterator_get_next(unit, &iterator_info));
    }

    SHR_IF_ERR_EXIT(dbal_table_iterator_destroy(unit, &iterator_info));

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
* Run logical test for specific/ALL table(s) according to input_strings[0], 
* if equal to  "ALL" all tests 
* if equal to specific table, run regression for specific table 
* input_strings[1] is the test mode. 
*  
*****************************************************/
cmd_result_t
dbal_logical_tables_test(
    int unit,
    char *input_strings[CMD_MAX_NOF_INPUTS])
{
    int res;
    uint32 flags = 0;
    dbal_tables_e table_id;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * second token is the test mode (default is 0)
     */
    flags = sal_ctoi(input_strings[1], 0);
    if (flags >= LTT_NOF_FLAGS)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Illegel LTT flags, flags=0x%x\n", flags);
    }

    /*
     * Run all tables
     */
    if (sal_strcasecmp(input_strings[0], "ALL") == 0)
    {
        int i, curr_res;
        res = 0;
        LOG_CLI((BSL_META("Runing tests per all existing logical tables...\n\n")));
        for (i = 0; i < DBAL_NOF_TABLES; i++)
        {
            curr_res = diag_dbal_test_logical_table(unit, i, flags | LTT_IS_REGRESSION);
            res |= (curr_res == CMD_FAIL);
            if (curr_res == CMD_OK)
            {
                if (flags & LTT_FULL_ITERATOR_TABLE_CLEAR_TEST)
                {
                    int counter = 0;
                    curr_res = diag_dbal_iterator_count_entries(unit, i, &counter);
                    res |= (curr_res == CMD_FAIL);
                    if (counter == 0)
                    {
                        LOG_CLI((BSL_META("\tTable %-40s - PASS, test pass and table clean\n"),
                                 dbal_logical_table_to_string(unit, i)));
                    }
                    else
                    {
                        LOG_CLI((BSL_META("\tTable %-40s - FAIL, test pass but found %d entries\n"),
                                 dbal_logical_table_to_string(unit, i), counter));
                    }
                }
                else
                {
                    LOG_CLI((BSL_META("\tTable %-40s - PASS\n"), dbal_logical_table_to_string(unit, i)));
                }
            }
            else if (curr_res == CMD_NOTIMPL)
            {
                if (!(flags & LTT_FULL_ITERATOR_TABLE_CLEAR_TEST))
                {
                    /** In full test, skip the NOT RUN print   */
                    LOG_CLI((BSL_META("\tTable %-40s - NOT_RUN\n"), dbal_logical_table_to_string(unit, i)));
                }
            }
            else
            {
                LOG_CLI((BSL_META("\tTable %-40s - FAIL\n"), dbal_logical_table_to_string(unit, i)));
            }
        }
        if (!res)
        {
            LOG_CLI((BSL_META("logical table test (all tables) - PASS\n")));
            SHR_EXIT();
        }
        else
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "logical table test (all tables) - FAIL\n");
        }
    }

    /*
     * first token should be the table name
     */
    res = dbal_logical_table_string_to_id(unit, input_strings[0], &table_id);
    if (res != CMD_OK)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Unknown table: %s\n", input_strings[0]);
    }

    res = diag_dbal_test_logical_table(unit, table_id, flags);
    if (res == CMD_OK)
    {
        if ((flags & LTT_TABLE_CLEAR_END_OF_TEST) || (flags & LTT_FULL_ITERATOR_TABLE_CLEAR_TEST))
        {
            int counter = -1;
            SHR_IF_ERR_EXIT(diag_dbal_iterator_count_entries(unit, table_id, &counter));
            if (counter == 0)
            {
                LOG_CLI((BSL_META("PASS, test pass and table clean\n")));
            }
            else
            {
                LOG_CLI((BSL_META("FAIL, found %d entries\n"), counter));
            }
        }
        else
        {
            LOG_CLI((BSL_META("PASS\n")));
        }
    }
    else
    {
        if (res == CMD_NOTIMPL)
        {
            LOG_CLI((BSL_META("Test configuration cannot be tested\n")));
        }
        else if (res == CMD_FAIL)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Test fail\n");
        }
    }

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
cmd_dnx_dbal_ltt(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    char *input_strings[5] = { "ALL", "0" };
    char *table_name = NULL;
    char *flags = NULL;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("table", table_name);
    SH_SAND_GET_STR("flags", flags);

    if (ISEMPTY(table_name))
    {
        dbal_tables_list_dump(unit, DBAL_LABEL_NONE, sand_control);
        DIAG_DBAL_HEADER_DUMP("Example: dbal table test Table=<table_name>/all Flags=<flags_value>", "\0");

        SHR_EXIT();
    }

    input_strings[0] = table_name;
    input_strings[1] = flags;
    dbal_logical_tables_test(unit, input_strings);

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_dbal_table_list_options[] = {
    {"TaBLe", SAL_FIELD_TYPE_STR, "dbal table name", ""},
    {"LaBeL", SAL_FIELD_TYPE_STR, "dbal label name", ""},
    {NULL}
};

static sh_sand_man_t dnx_dbal_table_list_man = {
    "dump all logical tables names that related to specific table name/label ,dump table information",
    "dump all logical tables names that related to specific table name/label ,dump table information",
    "dbal TaBLe List LaBeL=<label name>\n" "dbal TaBLe List TaBLe=<tabel name>",
    "dbal TaBLe List LaBeL=L3\n" "dbal TaBLe List TaBLe=INGRESS_PORT"
};

static sh_sand_option_t dnx_dbal_table_clear_options[] = {
    {"TaBLe", SAL_FIELD_TYPE_STR, "dbal table name", ""},
    {NULL}
};

static sh_sand_man_t dnx_dbal_table_clear_man = {
    "Clear a full table",
    "Clear a full table",
    "Synopsis",
    "dbal TaBLe CLear TaBLe=<tabel name>"
};

static sh_sand_option_t dnx_dbal_table_dump_options[] = {
    {"TaBLe", SAL_FIELD_TYPE_STR, "dbal table name", ""},
    {NULL}
};

static sh_sand_man_t dnx_dbal_table_dump_man = {
    "dump all table entries",
    "dump all table entries",
    "Synopsis",
    "dbal TaBLe DuMP TaBLe=<tabel name>"
};
static sh_sand_option_t dnx_dbal_ltt_options[] = {
    {"TaBLe", SAL_FIELD_TYPE_STR, "dbal table name", NULL},
    {"FLaGs", SAL_FIELD_TYPE_STR, "dbal ltt flags", "0"},
    {NULL}
};

static sh_sand_man_t dnx_dbal_ltt_man = {
    "DBAL logical table test - a basic set/get/iterator logical test per table",
    "Flags Usage:flags=<flags_value> (parsed as bitmap)\n"
        "Flags Options:\n"
        "\tFlags=0  - default. set/get/delte up to 32 entries\n"
        "\tFlags=1  - Perform iterator test\n"
        "\tFlags=2  - Clear table (instead of entry delete) at the end\n"
        "\tFlags=4  - Clear table at the beginning of the test\n"
        "\tFlags=8  - Regression mode (Low severity, only matured tables)\n"
        "\tFlags=16 - Iterator + Table Clear full test (In regression mode - only tables from list)\n"
        "\tFlags=32 - Keep entries in table, skip delete\n"
        "\tFlags=64 - Run test with maximum of three entries\n",
    "dbal TaBLe TeST TaBLe=<tabel name> FLaGs=<flags_value>",
    "dbal tbl tesl tbl=EXAMPLE_TABLE_FOR_HL_WITH_SW_FIELD flags=0x10"
};

sh_sand_cmd_t sh_dnx_dbal_table_cmds[] = {
  /********************************************************************************************************** 
   * CMD_NAME *     CMD_ACTION              * Next *        Options                   *       MAN           *
   *          *                             * Level*                                  *                     *
   *          *                             * CMD  *                                  *                     *
   **********************************************************************************************************/
    {"Info", cmd_dnx_dbal_tables_info, NULL, dnx_dbal_table_list_options, &dnx_dbal_table_list_man},
    {"CLear", cmd_dnx_dbal_table_clear, NULL, dnx_dbal_table_clear_options, &dnx_dbal_table_clear_man},
    {"DuMP", cmd_dbal_tables_entries_dump, NULL, dnx_dbal_table_dump_options, &dnx_dbal_table_dump_man},
    {"TeST", cmd_dnx_dbal_ltt, NULL, dnx_dbal_ltt_options, &dnx_dbal_ltt_man},
    {NULL}
};

/**********************************************************************************************************************
 *  DBAL DIAGNOSTIC PACK:
 *  ENTRY SUB MENU function & infrastructure Definitions - START
 *  STURCTURE:
 *  1. cmd function definition
 *  2. cmd option and man table
 *  3. ENTRY SUB MENU cmd table
 **********************************************************************************************************************/
/**
* \brief
* Print usgae example specific to requested table 
*****************************************************/
static shr_error_e
dnx_dbal_print_entry_help_line(
    int unit,
    char *table_name,
    char *cmd,
    uint8 key_field_only)
{
    CONST dbal_logical_table_t *table;
    dbal_field_basic_info_t *field_info;
    dbal_tables_e table_id;
    uint8 field_index;
    uint8 enum_val_index;
    uint8 result_index;
    uint8 enum_field_exist;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);
    dbal_logical_table_string_to_id(unit, table_name, &table_id);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    for (result_index = 0; result_index < table->nof_result_types; result_index++)
    {
        enum_field_exist = FALSE;
        if (table->nof_result_types == 1)
        {
            LOG_CLI((BSL_META("usage example: dbal ENTry %s TaBLe=%s "), cmd, table_name));
        }
        else
        {
            LOG_CLI((BSL_META("usage example for %s: dbal ENTry %s TaBLe=%s "),
                     table->multi_res_info[result_index].result_type_name, cmd, table_name));
        }

        /*
         * Print Key fields  
         */
        for (field_index = 0; field_index < table->nof_key_fields; field_index++)
        {
            LOG_CLI((BSL_META("%s=<val> "), dbal_field_to_string(unit, table->keys_info[field_index].field_id)));
        }
        if (key_field_only == FALSE)
        {
            /*
             * Print Result fields  
             */
            for (field_index = 0; field_index < table->multi_res_info[result_index].nof_result_fields; field_index++)
            {
                if (table->multi_res_info[result_index].results_info[field_index].field_id == DBAL_FIELD_RESULT_TYPE)
                {
                    LOG_CLI((BSL_META("%s=%d "),
                             dbal_field_to_string(unit,
                                                  table->multi_res_info[result_index].
                                                  results_info[field_index].field_id), result_index));
                }
                else
                {
                    LOG_CLI((BSL_META("%s=<val> "),
                             dbal_field_to_string(unit,
                                                  table->multi_res_info[result_index].
                                                  results_info[field_index].field_id)));
                }
            }

            LOG_CLI((BSL_META("\n\n")));

            /*
             * print enum fields values if exist
             */
            PRT_TITLE_SET("ENUM Fields");
            PRT_COLUMN_ADD("Field Name");
            PRT_COLUMN_ADD("ENUM Value");

            for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
            {
                SHR_IF_ERR_EXIT(dbal_fields_field_info_get_ptr
                                (unit, table->multi_res_info[0].results_info[field_index].field_id, &field_info));
                if (field_info->nof_enum_values > 0)
                {
                    enum_field_exist = TRUE;
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SET("%s",
                                 dbal_field_to_string(unit,
                                                      table->multi_res_info[0].results_info[field_index].field_id));
                    for (enum_val_index = 0; enum_val_index < field_info->nof_enum_values; enum_val_index++)
                    {
                        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                        PRT_CELL_SKIP(PRT_COLUMN_NUM - 1);
                        PRT_CELL_SET("%s", field_info->enum_val_info[enum_val_index].name);
                    }
                }
            }
            if (enum_field_exist)
            {
                PRT_COMMIT;
            }
            else
            {
                PRT_FREE;
            }

        }
    }
    LOG_CLI((BSL_META("\n")));

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_enum_t dbal_test_enum_table[DBAL_FIELD_MAX_NUM_OF_ENUM_VALUES];

static shr_error_e
dbal_builds_enum_sand_table(
    int unit,
    dbal_fields_e field_id)
{
    dbal_field_basic_info_t *field_info;
    int enum_entry_index = 0;
    int enum_val_index = 0;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_fields_field_info_get_ptr(unit, field_id, &field_info));
    for (enum_val_index = 0; enum_val_index < field_info->nof_enum_values; enum_val_index++)
    {
        dbal_test_enum_table[enum_entry_index].string = &(field_info->enum_val_info[enum_val_index].name[0]);
        dbal_test_enum_table[enum_entry_index].value = enum_val_index;
        enum_entry_index++;
    }

exit:
    SHR_FUNC_EXIT;
}

sal_field_type_e dbal_field_type_to_sal_field_type[] = {
    SAL_FIELD_TYPE_NONE,    /** DBAL_FIELD_TYPE_NONE,    */
    SAL_FIELD_TYPE_BOOL,    /** DBAL_FIELD_TYPE_BOOL,    */
    SAL_FIELD_TYPE_INT32,   /** DBAL_FIELD_TYPE_INT32,   */
    SAL_FIELD_TYPE_UINT32,  /** DBAL_FIELD_TYPE_UINT32,  */
    SAL_FIELD_TYPE_IP4,     /** DBAL_FIELD_TYPE_IP,      */
    SAL_FIELD_TYPE_ARRAY32, /** DBAL_FIELD_TYPE_ARRAY8,  */
    SAL_FIELD_TYPE_ARRAY32, /** DBAL_FIELD_TYPE_ARRAY32, */
    SAL_FIELD_TYPE_BITMAP,  /** DBAL_FIELD_TYPE_BITMAP,  */
    SAL_FIELD_TYPE_ENUM,    /** DBAL_FIELD_TYPE_ENUM,    */
    SAL_FIELD_TYPE_MAX,     /** DBAL_NOF_FIELD_TYPES,    */
};

static shr_error_e
dbal_diag_cmd_field_get(
    int unit,
    char *keyword,
    sal_field_type_e * type_p,
    uint32 * id_p,
    void **ext_ptr_p)
{
    dbal_fields_e field_id;
    dbal_field_type_e field_type;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_field_string_to_id(unit, keyword, &field_id));
    SHR_IF_ERR_EXIT(dbal_fields_type_get(unit, field_id, &field_type));

    dbal_builds_enum_sand_table(unit, field_id);

    if (type_p != NULL)
    {
        *type_p = dbal_field_type_to_sal_field_type[field_type];
    }
    if (id_p != NULL)
    {
        *id_p = field_id;
    }
    if (ext_ptr_p != NULL)
    {   /* Fill here you pointer to sh_sand_enum_t */
        *ext_ptr_p = &dbal_test_enum_table[0];
    }
    SHR_EXIT();
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - commit a DBAL table entry
 */
static shr_error_e
cmd_dnx_dbal_entry_commit(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    CONST dbal_logical_table_t *table;
    uint32 entry_handle;
    int key_field_index = 0;
    int value_field_index = 0;
    uint32 key_field_val[DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
    uint32 value_field_val[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

    char *table_name;
    dbal_tables_e table_id;
    char *cmd = "CoMmit";
    sh_sand_arg_t *sand_arg;
    dbal_fields_e field_id;
    uint8 is_key;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get parameters 
     */
    SH_SAND_GET_STR("table", table_name);

    if (ISEMPTY(table_name))
    {
        dbal_tables_list_dump(unit, DBAL_LABEL_NONE, sand_control);
        DIAG_DBAL_HEADER_DUMP("Example: dbal Entry CoMmit TaBLe=<table_name>", "\0");
        SHR_EXIT();
    }

    dbal_logical_table_string_to_id(unit, table_name, &table_id);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
    /*
     * Get table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle));

    SH_SAND_GET_ITERATOR(sand_arg)
    {
        field_id = SH_SAND_GET_ID(sand_arg);
        SHR_IF_ERR_EXIT(dbal_table_field_is_key(unit, table_id, field_id, &is_key));
        if (is_key == TRUE)
        {
            /*
             * Get Key fields Params 
             */
            key_field_val[key_field_index][0] = SH_SAND_GET_VALUE(sand_arg);
            dbal_entry_key_field_arr32_set(unit, entry_handle, field_id, INST_SINGLE, key_field_val[key_field_index]);

            key_field_index++;
        }
        else
        {
            /*
             * Get Value fields Params 
             */
            value_field_val[value_field_index][0] = SH_SAND_GET_VALUE(sand_arg);
            dbal_entry_value_field_arr32_set(unit, entry_handle, field_id, INST_SINGLE,
                                             value_field_val[value_field_index]);

            value_field_index++;
        }
    }

    if (key_field_index != table->nof_key_fields)
    {
        LOG_CLI((BSL_META("All Key fields are mandatory please find available fields below:\n")));
        dbal_logical_table_string_to_id(unit, table_name, &table_id);
        SHR_IF_ERR_EXIT(diag_dbal_logical_table_dump(unit, table_id, 0, sand_control));
        dnx_dbal_print_entry_help_line(unit, table_name, cmd, FALSE);
        dbal_entry_handle_release(unit, entry_handle);
        SHR_EXIT();
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle, DBAL_COMMIT_NORMAL));
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - get a DBAL table entry
 */
static shr_error_e
cmd_dnx_dbal_entry_get(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    CONST dbal_logical_table_t *table;
    uint32 entry_handle;
    int key_field_index = 0;
    int field_index = 0;
    dbal_field_data_t key_fields_array[DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS];
    dbal_field_data_t val_fields_array[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS];

    char *table_name;
    dbal_tables_e table_id;
    char *cmd = "Get";
    sh_sand_arg_t *sand_arg;
    dbal_fields_e field_id;
    uint8 is_key;
    int result_type_idx = 0;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get parameters 
     */
    SH_SAND_GET_STR("table", table_name);

    if (ISEMPTY(table_name))
    {
        dbal_tables_list_dump(unit, DBAL_LABEL_NONE, sand_control);
        DIAG_DBAL_HEADER_DUMP("Example: dbal ENTry Get TaBLe=<table_name>", "\0");
        SHR_EXIT();
    }

    dbal_logical_table_string_to_id(unit, table_name, &table_id);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
    /*
     * Get table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle));

    SH_SAND_GET_ITERATOR(sand_arg)
    {
        field_id = SH_SAND_GET_ID(sand_arg);
        SHR_IF_ERR_EXIT(dbal_table_field_is_key(unit, table_id, field_id, &is_key));
        if (is_key == TRUE)
        {
            key_fields_array[key_field_index].field_id = field_id;
            /*
             * Get Key fields Params 
             */
            key_fields_array[key_field_index].field_val[0] = SH_SAND_GET_VALUE(sand_arg);
            dbal_entry_key_field_arr32_set(unit, entry_handle, field_id, INST_SINGLE,
                                           key_fields_array[key_field_index].field_val);

            key_field_index++;
        }
    }

    if (key_field_index != table->nof_key_fields)
    {
        LOG_CLI((BSL_META("All Key fields are mandatory please find available fields below:\n")));
        dbal_logical_table_string_to_id(unit, table_name, &table_id);
        SHR_IF_ERR_EXIT(diag_dbal_logical_table_dump(unit, table_id, 0, sand_control));
        dnx_dbal_print_entry_help_line(unit, table_name, cmd, TRUE);
        dbal_entry_handle_release(unit, entry_handle);
        SHR_EXIT();
    }
    else
    {

        LOG_CLI((BSL_META("entry Get Table=%s "), table_name));
        /*
         * Print Key fields  
         */
        for (key_field_index = 0; key_field_index < table->nof_key_fields; key_field_index++)
        {
            dbal_field_from_buffer_print(unit, key_fields_array[key_field_index].field_id,
                                         table_id, key_fields_array[key_field_index].field_val,
                                         NULL, result_type_idx, TRUE, FALSE);
        }

        /*
         * Read Result fields  
         */
        if (table->nof_result_types == 1)
        {
            for (field_index = 0; field_index < table->multi_res_info[result_type_idx].nof_result_fields; field_index++)
            {
                field_id = table->multi_res_info[result_type_idx].results_info[field_index].field_id;
                val_fields_array[field_index].field_id = field_id;
                dbal_entry_value_field_arr32_get(unit, entry_handle, field_id,
                                                 INST_SINGLE, val_fields_array[field_index].field_val);
            }
            SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle, DBAL_COMMIT_NORMAL));
        }
        else
        { /** incase of multiple result type we cannot use the regular entry get sequence */
            int rv;
            rv = dbal_entry_get(unit, entry_handle, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE);
            if (rv)
            {
                dbal_entry_handle_release(unit, entry_handle);
                SHR_ERR_EXIT(rv, "entry get failed \n");
            }

            rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                                         val_fields_array[0].field_val);
            val_fields_array[0].field_id = DBAL_FIELD_RESULT_TYPE;
            result_type_idx = val_fields_array[0].field_val[0];

            if (rv)
            {
                dbal_entry_handle_release(unit, entry_handle);
                SHR_ERR_EXIT(rv, "entry get failed in mul result type\n");
            }

            for (field_index = 1; field_index < table->multi_res_info[result_type_idx].nof_result_fields; field_index++)
            {
                field_id = table->multi_res_info[result_type_idx].results_info[field_index].field_id;
                val_fields_array[field_index].field_id = field_id;
                rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle, field_id,
                                                             INST_SINGLE, val_fields_array[field_index].field_val);
                if (rv)
                {
                    dbal_entry_handle_release(unit, entry_handle);
                    SHR_ERR_EXIT(rv, "entry get failed in mul result type\n");
                }
            }
            dbal_entry_handle_release(unit, entry_handle);
        }
        /*
         * Print Result fields  
         */
        for (field_index = 0; field_index < table->multi_res_info[result_type_idx].nof_result_fields; field_index++)
        {
            dbal_field_from_buffer_print(unit, val_fields_array[field_index].field_id,
                                         table_id, val_fields_array[field_index].field_val,
                                         NULL, result_type_idx, FALSE, FALSE);
        }
    }
    LOG_CLI((BSL_META("\n")));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - clear a DBAL table entry
 */
static shr_error_e
cmd_dnx_dbal_entry_clear(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    CONST dbal_logical_table_t *table;
    uint32 entry_handle;
    int key_field_index = 0;
    dbal_field_data_t key_fields_array[DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS];

    char *table_name;
    dbal_tables_e table_id;
    char *cmd = "CLear";
    sh_sand_arg_t *sand_arg;
    dbal_fields_e field_id;
    uint8 is_key;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get parameters 
     */
    SH_SAND_GET_STR("table", table_name);

    if (ISEMPTY(table_name))
    {
        dbal_tables_list_dump(unit, DBAL_LABEL_NONE, sand_control);
        DIAG_DBAL_HEADER_DUMP("Example: dbal ENTry CLear TaBLe=<table_name>", "\0");
        SHR_EXIT();
    }

    dbal_logical_table_string_to_id(unit, table_name, &table_id);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
    /*
     * Get table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle));

    SH_SAND_GET_ITERATOR(sand_arg)
    {
        field_id = SH_SAND_GET_ID(sand_arg);
        key_fields_array[key_field_index].field_id = field_id;
        SHR_IF_ERR_EXIT(dbal_table_field_is_key(unit, table_id, field_id, &is_key));
        if (is_key == TRUE)
        {
            /*
             * Get Key fields Params 
             */
            key_fields_array[key_field_index].field_val[0] = SH_SAND_GET_VALUE(sand_arg);
            dbal_entry_key_field_arr32_set(unit, entry_handle, field_id, INST_SINGLE,
                                           key_fields_array[key_field_index].field_val);

            key_field_index++;
        }
    }

    if (key_field_index != table->nof_key_fields)
    {
        LOG_CLI((BSL_META("All Key fields are mandatory please find available fields below:\n")));
        dbal_logical_table_string_to_id(unit, table_name, &table_id);
        SHR_IF_ERR_EXIT(diag_dbal_logical_table_dump(unit, table_id, 0, sand_control));
        dnx_dbal_print_entry_help_line(unit, table_name, cmd, TRUE);
        dbal_entry_handle_release(unit, entry_handle);
        SHR_EXIT();
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_entry_clear(unit, entry_handle, DBAL_COMMIT_NORMAL));
    }

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_dbal_entry_commit_options[] = {
    {"TaBLe", SAL_FIELD_TYPE_STR, "dbal table name", ""},
    {NULL}
};

static sh_sand_man_t dnx_dbal_entry_commit_man = {
    "Set and commit a table entry",
    "Set and commit a table entry",
    "Synopsis",
    "dbal Entry Commit Table=<table_name>"
};

static sh_sand_option_t dnx_dbal_entry_get_options[] = {
    {"TaBLe", SAL_FIELD_TYPE_STR, "dbal table name", ""},
    {NULL}
};

static sh_sand_man_t dnx_dbal_entry_get_man = {
    "Get a table Entry",
    "Full",
    "Synopsis",
    "dbal Entry Get Table=<table_name>"
};

static sh_sand_option_t dnx_dbal_entry_clear_options[] = {
    {"TaBLe", SAL_FIELD_TYPE_STR, "dbal table name", ""},
    {NULL}
};

static sh_sand_man_t dnx_dbal_entry_clear_man = {
    "Clear a table entry",
    "Clear a table entry",
    "Synopsis",
    "dbal Entry CLear Table=<table_name>"
};

sh_sand_cmd_t sh_dnx_dbal_entry_cmds[] = {
   /****************************************************************************************************************** 
    * CMD    *     CMD_ACTION           * Next *        Options               *            MAN             * CB      *
    * NAME   *                          * Level*                              *                            *         *
    *        *                          * CMD  *                              *                            *         *
    ******************************************************************************************************************/
    {"CoMmit", cmd_dnx_dbal_entry_commit, NULL, dnx_dbal_entry_commit_options, &dnx_dbal_entry_commit_man,
     dbal_diag_cmd_field_get},
    {"Get", cmd_dnx_dbal_entry_get, NULL, dnx_dbal_entry_get_options, &dnx_dbal_entry_get_man, dbal_diag_cmd_field_get},
    {"CLear", cmd_dnx_dbal_entry_clear, NULL, dnx_dbal_entry_clear_options, &dnx_dbal_entry_clear_man,
     dbal_diag_cmd_field_get},
    {NULL}
};

/**********************************************************************************************************************
 *  DBAL DIAGNOSTIC PACK:
 *  FIELD SUB MENU function & infrastructure Definitions - START
 *  STURCTURE:
 *  1. cmd function definition
 *  2. cmd option and man table
 *  3. FIELD SUB MENU cmd table
 **********************************************************************************************************************/

/**
* \brief 
* receives input_strings[0] = label name
* dump all fields information that related to specific label, 
* if label == DBAL_NOF_LABEL_TYPES or DBAL_LABEL_NONE 
* dump all fields. 
*****************************************************/
static shr_error_e
cmd_dnx_dbal_fields_dump(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dbal_labels_e label;
    dbal_field_basic_info_t field_info;
    int ii, jj, to_print = 0;
    char str[CMD_MAX_STRING_LENGTH];
    char *label_name;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("label", label_name);

    if (ISEMPTY(label_name))
    {
        label = DBAL_LABEL_NONE;
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_label_string_to_id(unit, label_name, &label));
    }

    if ((label == DBAL_NOF_LABEL_TYPES) || (label == DBAL_LABEL_NONE))
    {
        PRT_TITLE_SET("FIELDS INFORMATION");
    }
    else
    {
        PRT_TITLE_SET("FIELDS INFORMATION RELATED TO LABEL:%s", dbal_label_to_string(unit, label));
    }

    PRT_COLUMN_ADD("Field Name");
    PRT_COLUMN_ADD("Bit size");
    PRT_COLUMN_ADD("Encode Info");
    PRT_COLUMN_ADD("Parent field");
    PRT_COLUMN_ADD("Default");
    PRT_COLUMN_ADD("Labels");
    PRT_COLUMN_ADD("Printable");
    PRT_COLUMN_ADD("Additional info");

    for (ii = DBAL_FIELD_EMPTY + 1; ii < DBAL_NOF_FIELDS; ii++)
    {
        if (label == DBAL_LABEL_NONE)
        {
            to_print = 1;
            SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, ii, &field_info));
        }
        else
        {
            SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, ii, &field_info));
            for (jj = 0; jj < DBAL_MAX_NOF_ENTITY_LABEL_TYPES; jj++)
                if ((field_info.labels[jj] == label) || (field_info.labels[jj] == DBAL_NOF_LABEL_TYPES))
                {
                    to_print = 1;
                    break;
                }
        }

        if (to_print)
        {
            to_print = 0;
            PRT_ROW_ADD(PRT_ROW_SEP_NONE);
            PRT_CELL_SET("%s", field_info.name);
            PRT_CELL_SET("%d", field_info.max_size);

            if ((field_info.encode_info.encode_mode == DBAL_VALUE_FIELD_ENCODE_HARD_VALUE) ||
                (field_info.encode_info.encode_mode == DBAL_VALUE_FIELD_ENCODE_SUBTRACT) ||
                (field_info.encode_info.encode_mode == DBAL_VALUE_FIELD_ENCODE_SUFFIX) ||
                (field_info.encode_info.encode_mode == DBAL_VALUE_FIELD_ENCODE_PREFIX))
            {
                PRT_CELL_SET("%s %d", dbal_field_encode_type_to_string(unit, field_info.encode_info.encode_mode),
                             field_info.encode_info.input_param);
            }
            else
            {
                PRT_CELL_SET("%s", dbal_field_encode_type_to_string(unit, field_info.encode_info.encode_mode));
            }

            PRT_CELL_SET("%s", (field_info.parent_field_id[0] == DBAL_NOF_FIELDS) ? " - " :
                         dbal_field_to_string(unit, field_info.parent_field_id[0]));
            if (field_info.is_default_value_valid == 0)
            {
                PRT_CELL_SET("%s", " - ");
            }
            else
            {
                PRT_CELL_SET("%d", field_info.default_value);
            }
            diag_dbal_lables_string_build(unit, (CONST dbal_labels_e *) (field_info.labels), str);
            PRT_CELL_SET("%s", str);
            PRT_CELL_SET("%s", dbal_field_type_to_string(unit, field_info.type));

            for (jj = 0; jj < field_info.nof_enum_values; jj++)
            {
                if (jj == 0)
                {
                    PRT_CELL_SET("ENUM values:");
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SKIP(PRT_COLUMN_NUM - 1);
                }
                PRT_CELL_SET("%s Value=%d", field_info.enum_val_info[jj].name, field_info.enum_val_info[jj].value);
                if (jj != (field_info.nof_enum_values - 1))
                {
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SKIP(PRT_COLUMN_NUM - 1);
                }
            }

            for (jj = 0; jj < field_info.nof_child_fields; jj++)
            {
                if (jj == 0)
                {
                    PRT_CELL_SET("Sub fields:");
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SKIP(PRT_COLUMN_NUM - 1);
                }
                PRT_CELL_SET("%s", dbal_field_to_string(unit, field_info.sub_field_info[jj].sub_field_id));

                if (jj != (field_info.nof_child_fields - 1))
                {
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SKIP(PRT_COLUMN_NUM - 1);
                }
            }

            if (field_info.instances_support == 1)
            {
                PRT_CELL_SET("Instance-able");
                ii = ii + DBAL_NUM_OF_FIELD_INSTANCES - 1;
            }
        }
    }

    PRT_COMMITX;

    LOG_CLI((BSL_META("\n")));

exit:
    PRT_FREE SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_dbal_fields_dump_options[] = {
    {"LaBeL", SAL_FIELD_TYPE_STR, "dbal label name", ""},
    {NULL}
};

static sh_sand_man_t dnx_dbal_fields_dump_man = {
    "dump all available fields",
    "dump all available fields",
    "Synopsis",
    "dbal FieLD Dump LaBeL=<label name>"
};

sh_sand_cmd_t sh_dnx_dbal_fields_cmds[] = {
  /************************************************************************************************************************ 
   *   CMD_NAME    *     CMD_ACTION           * Next  *        Options                    *            MAN                *
   *               *                          * Level *                                   *                               *
   *               *                          * CMD   *                                   *                               *
   ************************************************************************************************************************/
    {"DuMP", cmd_dnx_dbal_fields_dump, NULL, dnx_dbal_fields_dump_options, &dnx_dbal_fields_dump_man},
    {NULL}
};

/**********************************************************************************************************************
 *  DBAL DIAGNOSTIC PACK:
 *  LABELS SUB MENU function & infrastructure Definitions - START
 *  STURCTURE:
 *  1. cmd function definition
 *  2. cmd option and man table
 *  3. FIELD SUB MENU cmd table
 **********************************************************************************************************************/

/**
 * \brief
 * dump all dbal labels 
 * no input parameter is used 
 *******************/
static shr_error_e
cmd_dbal_labels_dump(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dbal_labels_e label;

    SHR_FUNC_INIT_VARS(unit);

    DIAG_DBAL_HEADER_DUMP("Existing labels", "\0");

    for (label = 0; label < DBAL_NOF_LABEL_TYPES; label++)
    {
        LOG_CLI((BSL_META("\t%s \n"), dbal_label_to_string(unit, label)));
    }
    SHR_EXIT();
exit:
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_dbal_lables_dump_options[] = {
    {NULL}
};

static sh_sand_man_t dnx_dbal_lables_dump_man = {
    "dump all available labels",
    "dump all available labels",
    "Synopsis",
    "dbal LaBeL DuMP"
};

sh_sand_cmd_t sh_dnx_dbal_labels_cmds[] = {
  /******************************************************************************************************************** 
   *   CMD_NAME    *     CMD_ACTION       * Next  *        Options                    *            MAN                *
   *               *                      * Level *                                   *                               *
   *               *                      * CMD   *                                   *                               *
   ********************************************************************************************************************/
    {"DuMP", cmd_dbal_labels_dump, NULL, dnx_dbal_lables_dump_options, &dnx_dbal_lables_dump_man},
    {NULL}
};

/**********************************************************************************************************************
 *  DBAL DIAGNOSTIC PACK:
 *  HANDLES SUB MENU function & infrastructure Definitions - START
 *  STURCTURE:
 *  1. cmd function definition
 *  2. cmd option and man table
 *  3. FIELD SUB MENU cmd table
 **********************************************************************************************************************/

/**
* \brief
* dump information of DBAL entries handles.
* if id arg = valid handle ID than prints also 
* extended information about the requested handle ID.
*  
********************************************************/
static shr_error_e
cmd_dbal_handles_info_dump(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    int iter, free_handle_counter = 0, handle_id = 0;
    dbal_entry_handle_t *entry_handle;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_INT32("ID", handle_id);

    for (iter = 0; iter < DBAL_SW_NOF_ENTRY_HANDLES; iter++)
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, iter, &entry_handle));
        if (entry_handle->handle_status > DBAL_HANDLE_STATUS_AVAILABLE)
        {
            LOG_CLI((BSL_META("Handle %d is been used by table %s\n"), iter, entry_handle->table->table_name));
        }
        else
        {
            free_handle_counter++;
        }
    }
    LOG_CLI((BSL_META("Total available handles %d of %d handles \n"), free_handle_counter, DBAL_SW_NOF_ENTRY_HANDLES));

    if ((handle_id >= DBAL_SW_NOF_ENTRY_HANDLES) || (handle_id < -1))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Illegal input handle_id out of range %d \n", handle_id);
    }

    if (handle_id != -1)
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, handle_id, &entry_handle));
        if (entry_handle->handle_status != DBAL_HANDLE_STATUS_AVAILABLE)
        {
            LOG_CLI((BSL_META("handle id=%d, status=%d, core=%d, error indication=%d, num of fields=%d, table:%s\n"),
                     handle_id, entry_handle->handle_status, entry_handle->core_id,
                     entry_handle->error_info.error_exists, entry_handle->num_of_fields,
                     entry_handle->table->table_name));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_dbal_handles_status_options[] = {
    {"ID", SAL_FIELD_TYPE_INT32, "handle ID", "-1"},
    {NULL}
};

static sh_sand_man_t dnx_dbal_handles_status_man = {
    "dump entry handle status and basic info",
    "dump entry handle status and basic info",
    "Synopsis",
    "dbal ENTry StaTuS"
};

sh_sand_cmd_t sh_dnx_dbal_handles_cmds[] = {
  /******************************************************************************************************************* 
   * CMD_NAME *     CMD_ACTION             * Next  *        Options                  *            MAN                *
   *          *                            * Level *                                 *                               *
   *          *                            * CMD   *                                 *                               *
   *******************************************************************************************************************/
    {"StaTuS", cmd_dbal_handles_info_dump, NULL, dnx_dbal_handles_status_options, &dnx_dbal_handles_status_man},
    {NULL}
};

/**********************************************************************************************************************
 *  DBAL DIAGNOSTIC PACK:
 *  DBAL MAIN MENU function & infrastructure Definitions - START
 *  STURCTURE:
 *  1. cmd function definition
 *  2. cmd option and man table
 *  3. DBAL MAIN MENU cmd table
 **********************************************************************************************************************/

/**
* \brief
* Change the severity of the DBAL module, 
* if no parameter than dump the current severity 
*****************************************************/
static shr_error_e
cmd_dbal_print_mode_set(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dnx_dbal_logger_type_e dbal_logger_type;
    bsl_severity_t severity = bslSeverityOff;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_ENUM("Type", dbal_logger_type);
    SH_SAND_GET_ENUM("Severity", severity);

    if (dbal_logger_type == DNX_DBAL_LOGGER_TYPE_LAST)
    {
        DIAG_DBAL_HEADER_DUMP("Example: dbal LoGger Type=<API/ACCESS> Severity=<OFF/NORMAL/HIGH/INFO>", "\0");
    }
    else
    {
        if (severity == DNX_DBAL_LOGGER_PRINT)
        {
            dbal_log_severity_get(unit, dbal_logger_type, &severity);
            LOG_CLI((BSL_META("Log severity: %s (%d)\n"), bsl_severity2str(severity), severity));
        }
        else
        {
            dbal_log_severity_set(unit, dbal_logger_type, severity);
            LOG_CLI((BSL_META("Log severity: %s (%d)\n"), bsl_severity2str(severity), severity));
            if (severity != DNX_DBAL_LOGGER_OFF)
            {
                LOG_CLI((BSL_META("Action prints enabled [after each main dbal action print will be ecxecuted]\n")));
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_dbal_table_options[] = {
    {NULL}
};

static sh_sand_man_t dnx_dbal_table_man = {
    "table operation menu",
    "table operation menu",
    "dbal TaBLe",
    "dbal TaBLe"
};

static sh_sand_option_t dnx_dbal_entry_options[] = {
    {NULL}
};

static sh_sand_man_t dnx_dbal_entry_man = {
    "entry operation menu",
    "entry operation menu",
    "dbal ENTry",
    "dbal ENTry"
};

static sh_sand_option_t dnx_dbal_fields_options[] = {
    {NULL}
};

static sh_sand_man_t dnx_dbal_fields_man = {
    "field operation menu",
    "field operation menu",
    "dbal FieLD",
    "dbal FieLD"
};

static sh_sand_option_t dnx_dbal_lables_options[] = {
    {NULL}
};

static sh_sand_man_t dnx_dbal_lables_man = {
    "labels operation menu",
    "labels operation menu",
    "dbal TaBLe",
    "dbal TaBLe"
};

static sh_sand_enum_t sand_test_dbal_logger_enum_table[] = {
    {"OFF", DNX_DBAL_LOGGER_OFF},
    {"NORMAL", DNX_DBAL_LOGGER_NORMAL},
    {"HIGH", DNX_DBAL_LOGGER_HIGH},
    {"INFO", DNX_DBAL_LOGGER_PRINT},
    {"API", DNX_DBAL_LOGGER_TYPE_API},
    {"ACCESS", DNX_DBAL_LOGGER_TYPE_ACCESS},
    {"EMPTY", DNX_DBAL_LOGGER_TYPE_LAST},
    {NULL}
};

static sh_sand_option_t dnx_dbal_log_severity_options[] = {
    {"Type", SAL_FIELD_TYPE_ENUM, "dbal log Severity", "EMPTY", (void *) sand_test_dbal_logger_enum_table},
    {"SeVerity", SAL_FIELD_TYPE_ENUM, "dbal log Severity", "INFO", (void *) sand_test_dbal_logger_enum_table},
    {NULL}
};

static sh_sand_man_t dnx_dbal_log_severity_man = {
    "change the severity of the logger",
    "change the severity of the logger, Off=Warning, Normal=Info, High=Verbose, no input:dump current severity",
    "dbal LoGger Type=<dbal_log_type>SeVerity=<severity level>",
    "dbal LoGger Type=API SeVerity=Normal" "dbal LoGger Type=ACCESS SeVerity=High"
};

static sh_sand_option_t dnx_dbal_handles_options[] = {
    {NULL}
};

static sh_sand_man_t dnx_dbal_handles_man = {
    "handles operation menu",
    "handles operation menu",
    "Synopsis",
    "dbal HanDLe"
};

sh_sand_man_t sh_dnx_dbal_man = {
    "dbal diagnostic commands",
    "dbal diagnostic commands",
    "dbal",
    "dbal"
};

/**
 * \brief DNX DBAL diagnostic pack
 * List of the supported commands, pointer to command function and command usage function. 
 * This is the entry point for DBAL diagnostic commands 
 */
sh_sand_cmd_t sh_dnx_dbal_cmds[] = {
   /******************************************************************************************************************************** 
    * CMD_NAME *     CMD_ACTION            * Next                    *        Options                 *         MAN                *
    *          *                           * Level                   *                                *                            *
    *          *                           * CMD                     *                                *                            *
    ********************************************************************************************************************************/
    {"TaBLe", NULL, sh_dnx_dbal_table_cmds, dnx_dbal_table_options, &dnx_dbal_table_man},
    {"ENTry", NULL, sh_dnx_dbal_entry_cmds, dnx_dbal_entry_options, &dnx_dbal_entry_man},
    {"FieLD", NULL, sh_dnx_dbal_fields_cmds, dnx_dbal_fields_options, &dnx_dbal_fields_man},
    {"LaBeL", NULL, sh_dnx_dbal_labels_cmds, dnx_dbal_lables_options, &dnx_dbal_lables_man},
    {"LoGger", cmd_dbal_print_mode_set, NULL, dnx_dbal_log_severity_options, &dnx_dbal_log_severity_man},
    {"HanDLe", NULL, sh_dnx_dbal_handles_cmds, dnx_dbal_handles_options, &dnx_dbal_handles_man},
    {NULL}
};
