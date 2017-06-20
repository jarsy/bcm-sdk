/*
 * ! \file mdb_diag.c Contains all of the MDB diag commands
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_MDBDNX
#include <shared/bsl.h>

/*************
 * INCLUDES  *
 *************/
#include <soc/dnx/mdb.h>
#include "../../../../../soc/dnx/dbal/dbal_string_mgmt.h"

#include <shared/bsl.h>
#include <sal/appl/sal.h>
#include <shared/bslnames.h>
#include <soc/mcm/memregs.h>
#include <soc/dpp/SAND/Management/sand_low_level.h>
#include <bcm_int/dpp/error.h>
#include <appl/diag/system.h>
#include <appl/diag/diag.h>

#include <appl/diag/sand/diag_sand_prt.h>
#include <appl/diag/sand/diag_sand_framework.h>
#include <shared/utilex/utilex_bitstream.h>

/*************
 * DEFINES   *
 *************/

#define CMD_MAX_STRING_LENGTH   30
#define CMD_MAX_NOF_INPUTS      5

typedef cmd_result_t(
    *CMD_DIAG_FUNC) (
    int unit,
    char *params[CMD_MAX_NOF_INPUTS]);

typedef struct
{
    char comand_name[CMD_MAX_STRING_LENGTH];
    char comand_name_short[CMD_MAX_STRING_LENGTH / 4];
    int num_of_mandatory_params;
    int num_of_optional_params;
    CMD_DIAG_FUNC diag_func;

} cmd_string_func_t;

typedef struct
{
    CMD_DIAG_FUNC diag_func;
    char *inputs[CMD_MAX_NOF_INPUTS];
    uint8 is_passed;
    char *test_description;

} regression_test_list_t;

/*************
* FUNCTIONS *
*************/

cmd_result_t
diag_mdb_test_physical_table(
    int unit,
    dbal_physical_tables_e table_id,
    mdb_test_mode_e mode)
{
    mdb_db_type_e mdb_db_type = mdb_get_db_type(table_id);

    SHR_FUNC_INIT_VARS(unit);

    LOG_CLI((BSL_META("*****diag_mdb_test_physical_table*****\n" "table=%s(%d) ,test_mode=%d\n"),
             dbal_physical_table_to_string(unit, table_id), table_id, mode));

    if (mdb_db_type == MDB_DB_TCAM)
    {
        LOG_CLI((BSL_META("TCAM test not yet implemented.\n")));
    }
    else if (mdb_db_type == MDB_DB_KAPS)
    {
        LOG_CLI((BSL_META("Physical table %d associated with LPM table.\n"), table_id));
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
        if (TRUE)
        {
            SHR_IF_ERR_EXIT(mdb_lpm_test(unit, table_id, mode));
        }
        else
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
        {
            SHR_ERR_EXIT(_SHR_E_DISABLED, "Compilation does not include KBPSDK flags\n");
        }
    }
    else if (mdb_db_type == MDB_DB_EM)
    {
        LOG_CLI((BSL_META("Physical table %d associated with EM.\n"), table_id));
        SHR_IF_ERR_EXIT(mdb_em_test(unit, table_id, mode));
    }
    else if (mdb_db_type == MDB_DB_DIRECT)
    {
        LOG_CLI((BSL_META("Physical table %d associated with direct table.\n"), table_id));
        SHR_IF_ERR_EXIT(mdb_direct_table_test(unit, table_id, mode));
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Physical table %d not associated with TCAM/LPM/EM/Direct\n", table_id);
    }

    LOG_CLI((BSL_META("MDB Table test passed!\n")));

    /*
     * sal_srand(sal_time_usecs());
     */

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_mdb_test_options[] = {
    {"Table", SAL_FIELD_TYPE_STR, "DBAL physical table name", ""},
    {"MoDe", SAL_FIELD_TYPE_INT32, "The test mode: 0-fast (tests ~1/10 of the capacity), 1-full", "0"},
    {"all", SAL_FIELD_TYPE_BOOL, "Ignore Table and run the test on all table", "No"},
    {NULL}
};

static sh_sand_man_t dnx_mdb_test_man = {
    "Test the MDB physical table operation.",
    "Test the different MDB physical tables by adding random entries, verifying the entries through get and deleting them. "
        "Running with \"all\" option tests all tables and ignores the table parameter. "
        "Mode allows toggling between a short test(0) and a long test(1)",
    "dnx mdb test [all] [table=DBAL_PHYSICAL_TABLE_NAME] [MoDe=0/1]",
    "dnx mdb test all\n" "dnx mdb test table=LEM mode=1"
};

/**
* \brief
* Change the severity of the MDB module,
* if no parameter than dump the current severity
*****************************************************/
static shr_error_e
cmd_mdb_test(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    int all;
    int mode;
    char *table_name = NULL;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("Table", table_name);
    SH_SAND_GET_INT32("MoDe", mode);
    SH_SAND_GET_BOOL("all", all);

    if (mode > MDB_NOF_TEST_MODES)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "The mode is out of range.\n");
    }

    /*
     * Ignore table and run the test on all tables
     */
    if (all != 0)
    {
        int ii;
        for (ii = DBAL_PHYSICAL_TABLE_NONE + 1; ii < DBAL_NOF_PHYSICAL_TABLES; ii++)
        {
            SHR_IF_ERR_CONT(diag_mdb_test_physical_table(unit, ii, mode));
        }
    }
    else
    {
        if (ISEMPTY(table_name))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "The Table parameter must be specified if not running on all.\n");
        }
        else
        {
            dbal_physical_tables_e dbal_physical_table_id;
            SHR_IF_ERR_EXIT(dbal_physical_table_string_to_id(unit, table_name, &dbal_physical_table_id));

            SHR_IF_ERR_EXIT(diag_mdb_test_physical_table(unit, dbal_physical_table_id, mode));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_man_t dnx_mdb_info_man = {
    "Dump all DBAL Physical tables names or print information for a specific table",
    "Prints all of the DBAL physical table names if not given a table name, "
        "otherwise prints: table type [Direct, MDB, LPM, TCAM], Address resolution, capacity "
        "allocated clusters {mem, block, array_index, offset within bucket).",
    "dnx mdb info [table=DBAL_PHYSICAL_TABLE_NAME]",
    "dnx mdb info table=ISEM_1"
};

static sh_sand_option_t dnx_mdb_info_options[] = {
    {"Table", SAL_FIELD_TYPE_STR, "DBAL physical table name", ""},
    {NULL}
};

static shr_error_e
mdb_diag_single_table_info_print(
    int unit,
    sh_sand_control_t * sand_control,
    char *table_name,
    mdb_db_type_e mdb_db_type,
    uint32 max_capacity,
    mdb_dbal_physical_table_access_info_t * table_access_info)
{
    int ii;
    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Print table header
     */
    PRT_TITLE_SET("DBAL PHYSICAL TABLE");

    PRT_COLUMN_ADD("Table Name");
    PRT_COLUMN_ADD("Type");
    PRT_COLUMN_ADD("Entry capacity");
    PRT_COLUMN_ADD("#Clusters");
    PRT_COLUMN_ADD("Address Resolution[bits]");
    PRT_COLUMN_ADD("Row size[bits]");

    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
    PRT_CELL_SET("%s", table_name);
    PRT_CELL_SET("%s", mdb_get_db_type_str(mdb_db_type));
    PRT_CELL_SET("%d", max_capacity);
    PRT_CELL_SET("%d", table_access_info->nof_clusters);
    if (mdb_db_type == MDB_DB_DIRECT)
    {
        PRT_CELL_SET("%d", table_access_info->direct_access_resolution);
        PRT_CELL_SET("%d", table_access_info->row_width);
    }
    if (mdb_db_type == MDB_DB_EM)
    {
        PRT_CELL_SET("%d", 0);
        PRT_CELL_SET("%d", table_access_info->row_width);
    }

    PRT_COMMITX;

    /*
     * Print cluster info
     */
    PRT_TITLE_SET("DBAL PHYSICAL TABLE - CLUSTER INFO");

    PRT_COLUMN_ADD("Cluster ID");
    PRT_COLUMN_ADD("Memory");
    PRT_COLUMN_ADD("Memory Array Index[0-1]");
    PRT_COLUMN_ADD("Memory Cluster Offset[0-3]");
    PRT_COLUMN_ADD("Block");
    PRT_COLUMN_ADD("Block Instance");

    for (ii = 0; ii < table_access_info->nof_clusters; ii++)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%d", table_access_info->cluster_info_array[ii].cluster_address);
        PRT_CELL_SET("%s", SOC_MEM_NAME(unit, table_access_info->cluster_info_array[ii].macro_mem));
        PRT_CELL_SET("%d", table_access_info->cluster_info_array[ii].bucket_index);
        PRT_CELL_SET("%d", table_access_info->cluster_info_array[ii].bucket_offset);
        if (table_access_info->cluster_info_array[ii].blk == DHC_BLOCK(unit, 0))
        {
            PRT_CELL_SET("%s", "DHC_BLOCK");
        }
        else if (table_access_info->cluster_info_array[ii].blk == DDHB_BLOCK(unit, 0))
        {
            PRT_CELL_SET("%s", "DDHB_BLOCK");
        }
        else
        {
            PRT_CELL_SET("%s", "");
        }
        PRT_CELL_SET("%d", table_access_info->cluster_info_array[ii].blk_id);
    }

    PRT_COMMITX;

exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

/**
* \brief
* dump physical table information.
* See dnx_mdb_info_man for the different options.
*****************************************************/
shr_error_e
cmd_dnx_mdb_info_print(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dbal_physical_tables_e dbal_physical_table_id;
    dbal_physical_table_def_t *dbal_physical_table;
    mdb_dbal_physical_table_access_info_t table_access_info;
    mdb_physical_tables_e mdb_physical_table;
    char *table_name = NULL;
    int ii;

    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("table", table_name);

    if (ISEMPTY(table_name))
    {
        /*
         * Print table names
         */

        PRT_TITLE_SET("DBAL PHYSICAL TABLES");

        PRT_COLUMN_ADD("Table Name");
        PRT_COLUMN_ADD("Type");
        PRT_COLUMN_ADD("Entry capacity");
        PRT_COLUMN_ADD("#Clusters");
        PRT_COLUMN_ADD("Address Resolution[bits]");
        PRT_COLUMN_ADD("Row size[bits]");

        for (ii = 1; ii < DBAL_NOF_PHYSICAL_TABLES; ii++)
        {
            SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, ii, &dbal_physical_table));
            mdb_physical_table = mdb_get_physical_table(ii);
            SHR_IF_ERR_EXIT(mdb_get_db_access_info(unit, mdb_physical_table, &table_access_info));

            PRT_ROW_ADD(PRT_ROW_SEP_NONE);
            PRT_CELL_SET("%s", dbal_physical_table_to_string(unit, ii));
            PRT_CELL_SET("%s", mdb_get_db_type_str(mdb_get_db_type(ii)));
            PRT_CELL_SET("%d", dbal_physical_table->max_capacity);
            PRT_CELL_SET("%d", table_access_info.nof_clusters);
            if (mdb_get_db_type(ii) == MDB_DB_DIRECT)
            {
                PRT_CELL_SET("%d", table_access_info.direct_access_resolution);
                PRT_CELL_SET("%d", table_access_info.row_width);
            }
            if (mdb_get_db_type(ii) == MDB_DB_EM)
            {
                PRT_CELL_SET("%d", 0);
                PRT_CELL_SET("%d", table_access_info.row_width);
            }
        }

        PRT_COMMITX;
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_physical_table_string_to_id(unit, table_name, &dbal_physical_table_id));
        SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, dbal_physical_table_id, &dbal_physical_table));
        mdb_physical_table = mdb_get_physical_table(dbal_physical_table_id);

        /*
         * LPM Requires special treatment as it is split between small KAPS, KAPS BB and KAPS ADS
         */
        if (dbal_physical_table_id == DBAL_PHYSICAL_TABLE_LPM_PRIVATE)
        {
            SHR_IF_ERR_EXIT(mdb_get_db_access_info(unit, MDB_PHYSICAL_TABLE_KAPS_1, &table_access_info));
            SHR_IF_ERR_EXIT(mdb_diag_single_table_info_print
                            (unit, sand_control, "MDB_PHYSICAL_TABLE_KAPS_1", mdb_get_db_type(dbal_physical_table_id),
                             dbal_physical_table->max_capacity, &table_access_info));

            SHR_IF_ERR_EXIT(mdb_get_db_access_info(unit, MDB_PHYSICAL_TABLE_ADS_1, &table_access_info));
            SHR_IF_ERR_EXIT(mdb_diag_single_table_info_print
                            (unit, sand_control, "MDB_PHYSICAL_TABLE_ADS_1", mdb_get_db_type(dbal_physical_table_id),
                             dbal_physical_table->max_capacity, &table_access_info));
        }
        else if (dbal_physical_table_id == DBAL_PHYSICAL_TABLE_LPM_PUBLIC)
        {
            SHR_IF_ERR_EXIT(mdb_get_db_access_info(unit, MDB_PHYSICAL_TABLE_KAPS_2, &table_access_info));
            SHR_IF_ERR_EXIT(mdb_diag_single_table_info_print
                            (unit, sand_control, "MDB_PHYSICAL_TABLE_KAPS_2", mdb_get_db_type(dbal_physical_table_id),
                             dbal_physical_table->max_capacity, &table_access_info));

            SHR_IF_ERR_EXIT(mdb_get_db_access_info(unit, MDB_PHYSICAL_TABLE_ADS_2, &table_access_info));
            SHR_IF_ERR_EXIT(mdb_diag_single_table_info_print
                            (unit, sand_control, "MDB_PHYSICAL_TABLE_ADS_2", mdb_get_db_type(dbal_physical_table_id),
                             dbal_physical_table->max_capacity, &table_access_info));
        }
        else
        {
            SHR_IF_ERR_EXIT(mdb_get_db_access_info(unit, mdb_physical_table, &table_access_info));

            SHR_IF_ERR_EXIT(mdb_diag_single_table_info_print
                            (unit, sand_control, dbal_physical_table->physical_name,
                             mdb_get_db_type(dbal_physical_table_id), dbal_physical_table->max_capacity,
                             &table_access_info));

            /*
             * Print information related to APP ID for EM tables
             */
            if (mdb_get_db_type(dbal_physical_table_id) == MDB_DB_EM)
            {
                PRT_TITLE_SET("Associated APP IDs");

                PRT_COLUMN_ADD("APP ID");
                PRT_COLUMN_ADD("Key size");
                PRT_COLUMN_ADD("Entry size");

                for (ii = 0; ii < MDB_NOF_APP_IDS; ii++)
                {
                    uint32 key_size;
                    uint32 entry_size;
                    SHR_IF_ERR_EXIT(mdb_em_get_key_size(unit, dbal_physical_table_id, ii, &key_size));
                    SHR_IF_ERR_EXIT(mdb_em_get_entry_size(unit, dbal_physical_table_id, ii, &entry_size));

                    if (key_size != 0)
                    {
                        /*
                         * Only print valid APP IDs
                         */
                        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                        PRT_CELL_SET("%d", ii);
                        PRT_CELL_SET("%d", key_size);
                        PRT_CELL_SET("%d", entry_size);
                    }
                }

                PRT_COMMITX;
            }
        }
    }

exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

/*
 * Dump all the entries in private or public DB associated with a specific APP ID
 */
static shr_error_e
mdb_diag_lpm_dump(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    sh_sand_control_t * sand_control)
{
    dbal_physical_entry_iterator_t physical_entry_iterator;
    dbal_physical_entry_t entry;
    uint8 is_end = FALSE;
    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&physical_entry_iterator, 0x0, sizeof(physical_entry_iterator));

    /*
     * Print table header
     */
    if (dbal_physical_table_id == DBAL_PHYSICAL_TABLE_LPM_PRIVATE)
    {
        PRT_TITLE_SET("MDB LPM PRIVATE");
    }
    else if (dbal_physical_table_id == DBAL_PHYSICAL_TABLE_LPM_PUBLIC)
    {
        PRT_TITLE_SET("MDB LPM PUBLIC");
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "LPM dump operation must be done on an LPM table.\n");
    }

    PRT_COLUMN_ADD("Prefix Length");
    PRT_COLUMN_ADD("Prefix");
    PRT_COLUMN_ADD("Payload");

    SHR_IF_ERR_EXIT(mdb_lpm_iterator_init(unit, dbal_physical_table_id, app_id, &physical_entry_iterator));

    SHR_IF_ERR_EXIT(mdb_lpm_iterator_get_next
                    (unit, dbal_physical_table_id, app_id, &physical_entry_iterator, &entry, &is_end));
    while (is_end == FALSE)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%d", mdb_lpm_calculate_prefix_length(entry.k_mask));
        PRT_CELL_SET("0x%08x%08x%08x%08x%08x", entry.key[4], entry.key[3], entry.key[2], entry.key[1], entry.key[0]);
        PRT_CELL_SET("0x%05x", entry.payload[0]);

        SHR_IF_ERR_EXIT(mdb_lpm_iterator_get_next
                        (unit, dbal_physical_table_id, app_id, &physical_entry_iterator, &entry, &is_end));
    }

    PRT_COMMITX;

exit:
    PRT_FREE;
    SHR_IF_ERR_CONT(mdb_lpm_iterator_deinit(unit, dbal_physical_table_id, app_id, &physical_entry_iterator));
    SHR_FUNC_EXIT;
}

static sh_sand_man_t dnx_mdb_lpm_man = {
    "Perform different operation on the LPM (KAPS) table.",
    "Directly perform an operation on the LPM private or public tables. "
        "Specify the 160bit key (including 6bit prefix!) using key, the prefix_length and the payload in case of add. "
        "Note: the LPM is aligned to the MSB, for example if the key is only 16bits=0x1234, "
        "then the input is key=0x1234000000000000000000000000000000000000, note the prefix in this case is 0x12 >> 2 = 4.",
    "dnx mdb lpm add/get/search/delete/dump [table=LPM_PRIVATE/PUBLIC] [key=0xVALUE] [prefix_length=0-160] [payload=0xVALUE] [app_id=VALUE]",
    "dnx mdb LPM add table=LPM_PRIVATE key=0x1234567812345678123456781234560000000000 prefix_length=120 payload=0xabcd\n"
        "dnx mdb LPM get table=LPM_PRIVATE key=0x1234567812345678123456781234560000000000 prefix_length=120\n"
        "dnx mdb LPM search table=LPM_PRIVATE key=0x1234567812345678123456781234560000000000\n"
        "dnx mdb LPM delete table=LPM_PRIVATE key=0x1234567812345678123456781234560000000000 prefix_length=120\n"
        "dnx mdb LPM dump table=LPM_PRIVATE"
};

static sh_sand_option_t dnx_mdb_lpm_options[] = {
    {"ADD", SAL_FIELD_TYPE_BOOL, "Perform an add operation", "no"},
    {"app_id", SAL_FIELD_TYPE_UINT32, "The APP ID", "0"},
    {"DeLeTe", SAL_FIELD_TYPE_BOOL, "Perform a delete operation", "no"},
    {"dump", SAL_FIELD_TYPE_BOOL, "Perform a dump operation", "no"},
    {"Get", SAL_FIELD_TYPE_BOOL, "Perform a get operation", "no"},
    {"payload", SAL_FIELD_TYPE_UINT32, "The 20bit KAPS payload", "0"},
    {"prefix_length", SAL_FIELD_TYPE_INT32, "The prefix length, 0-160", "0"},
    {"key", SAL_FIELD_TYPE_ARRAY32, "The 160bit key", "0"},
    {"search", SAL_FIELD_TYPE_BOOL, "Perform a search operation", "no"},
    {"Table", SAL_FIELD_TYPE_STR, "DBAL physical table name", ""},
    {NULL}
};

/**
* \brief
* Perform different operation on the LPM (KAPS) table
* See dnx_mdb_lpm_man for the different options.
*****************************************************/
shr_error_e
cmd_dnx_mdb_lpm(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dbal_physical_tables_e dbal_physical_table_id;
    dbal_physical_entry_t entry;
    int add_cmd, get_cmd, delete_cmd, dump_cmd, search_cmd;
    char *table_name = NULL;
    int prefix_length;
    uint32 app_id;
    uint32 *array_uint32;
    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&entry, 0x0, sizeof(entry));

    SH_SAND_GET_STR("Table", table_name);
    SH_SAND_GET_BOOL("ADD", add_cmd);
    SH_SAND_GET_BOOL("Get", get_cmd);
    SH_SAND_GET_BOOL("DeLeTe", delete_cmd);
    SH_SAND_GET_BOOL("dump", dump_cmd);
    SH_SAND_GET_BOOL("search", search_cmd);

    SH_SAND_GET_ARRAY32("key", array_uint32);
    sal_memcpy(entry.key, array_uint32, MDB_KAPS_KEY_WIDTH_IN_UINT32 * sizeof(entry.key[0]));
    SH_SAND_GET_INT32("prefix_length", prefix_length);
    SH_SAND_GET_INT32("payload", entry.payload[0]);
    SH_SAND_GET_UINT32("app_id", app_id);

    if (prefix_length > MDB_KAPS_KEY_WIDTH_IN_BITS)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "The prefix_length must be smaller than %d bits.\n", MDB_KAPS_KEY_WIDTH_IN_BITS);
    }
    else
    {
        SHR_IF_ERR_EXIT(mdb_lpm_prefix_len_to_mask(unit, prefix_length, &entry));
    }

    if (ISEMPTY(table_name))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "The Table parameter must be specified.\n");
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_physical_table_string_to_id(unit, table_name, &dbal_physical_table_id));

        if (add_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_lpm_entry_add(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Add operation completed successfully on LPM table.\n")));
        }
        else if (get_cmd != 0)
        {
            shr_error_e return_error;
            return_error = mdb_lpm_entry_get(unit, dbal_physical_table_id, app_id, &entry);
            if (return_error == _SHR_E_NOT_FOUND)
            {
                LOG_CLI((BSL_META("Get operation failed, entry not found in LPM table.\n")));
            }
            else if (return_error != _SHR_E_NONE)
            {
                SHR_ERR_EXIT(return_error, "Get operation failed with unexpected error in LPM table.\n");
            }
            else
            {
                LOG_CLI((BSL_META("Get operation completed successfully on LPM table.\n")));
                LOG_CLI((BSL_META("The associated payload is: 0x%x\n"), entry.payload[0]));
            }
        }
        else if (delete_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_lpm_entry_delete(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Delete operation completed successfully on LPM table.\n")));
        }
        else if (dump_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_diag_lpm_dump(unit, dbal_physical_table_id, app_id, sand_control));
        }
        else if (search_cmd != 0)
        {
            int core;

            PRT_TITLE_SET("MDB LPM PUBLIC");
            PRT_COLUMN_ADD("Core");
            PRT_COLUMN_ADD("Prefix Length");
            PRT_COLUMN_ADD("Prefix");
            PRT_COLUMN_ADD("Payload");

            for (core = 0; core < MDB_CURRENT_NOF_CORES; core++)
            {
                SHR_IF_ERR_EXIT(mdb_lpm_entry_search(unit, core, dbal_physical_table_id, app_id, &entry));

                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET("%d", core);
                PRT_CELL_SET("%d", mdb_lpm_calculate_prefix_length(entry.k_mask));
                PRT_CELL_SET("0x%08x%08x%08x%08x%08x", entry.key[4], entry.key[3], entry.key[2], entry.key[1],
                             entry.key[0]);
                PRT_CELL_SET("0x%05x", entry.payload[0]);
            }
            PRT_COMMITX;
        }
        else
        {
            LOG_CLI((BSL_META("An operation must be specified.\n")));
        }
    }

exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

static sh_sand_man_t dnx_mdb_direct_man = {
    "Perform different operation on the direct tables.",
    "Directly perform an operation on the direct tables. "
        "Specify the index (key) using key, the payload_size and the payload in case of add. "
        "If payload_size is not specified(=0) then it is assumed to be the table default. "
        "Payload mask is assumed to be full.",
    "dnx mdb direct add/get/delete/dump [table=DBAL_PHYSICAL_TABLE_NAME] [key=0xVALUE] [payload_size=0-180] [payload=0xVALUE]",
    "dnx mdb direct add table=IVSI key=100 payload_size=90 payload=0x12345678abcdef01030507\n"
        "dnx mdb direct get table=IVSI key=100 payload_size=90\n"
        "dnx mdb direct delete table=IVSI key=100 payload_size=90\n" "dnx mdb direct dump table=IVSI"
};

static sh_sand_option_t dnx_mdb_direct_options[] = {
    {"Table", SAL_FIELD_TYPE_STR, "DBAL physical table name", ""},
    {"ADD", SAL_FIELD_TYPE_BOOL, "Perform an add operation", "no"},
    {"Get", SAL_FIELD_TYPE_BOOL, "Perform a get operation", "no"},
    {"DeLeTe", SAL_FIELD_TYPE_BOOL, "Perform a delete operation", "no"},
    {"Dump", SAL_FIELD_TYPE_BOOL, "Perform a delete operation", "no"},
    {"key", SAL_FIELD_TYPE_UINT32, "The direct entry index", "0"},
    {"payload_size", SAL_FIELD_TYPE_UINT32, "The payload size", "0"},
    {"payload", SAL_FIELD_TYPE_ARRAY32, "Up to 180bits of the payload", "0"},
    {NULL}
};

/*
 * Dump all the entries in a specific direct table
 */
static shr_error_e
mdb_diag_direct_table_dump(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    sh_sand_control_t * sand_control)
{
    dbal_physical_entry_iterator_t physical_entry_iterator;
    dbal_physical_entry_t entry;
    dbal_physical_table_def_t *dbal_physical_table;
    uint8 is_end = FALSE;
    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&physical_entry_iterator, 0x0, sizeof(physical_entry_iterator));

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, dbal_physical_table_id, &dbal_physical_table));

    /*
     * Print table header
     */
    PRT_TITLE_SET("Entries in: %s", dbal_physical_table->physical_name);

    PRT_COLUMN_ADD("Key");
    PRT_COLUMN_ADD("Payload Size");
    PRT_COLUMN_ADD("Payload");

    SHR_IF_ERR_EXIT(mdb_direct_table_iterator_init(unit, dbal_physical_table_id, app_id, &physical_entry_iterator));

    SHR_IF_ERR_EXIT(mdb_direct_table_iterator_get_next
                    (unit, dbal_physical_table_id, app_id, &physical_entry_iterator, &entry, &is_end));
    while (is_end == FALSE)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%d", entry.key[0]);
        PRT_CELL_SET("%d", entry.payload_size);

        if (entry.payload_size <= SAL_UINT32_NOF_BITS)
        {
            PRT_CELL_SET("0x%08x", entry.payload[0]);
        }
        else if (entry.payload_size <= 2 * SAL_UINT32_NOF_BITS)
        {
            PRT_CELL_SET("0x%08x%08x", entry.payload[1], entry.payload[0]);
        }
        else if (entry.payload_size <= 3 * SAL_UINT32_NOF_BITS)
        {
            PRT_CELL_SET("0x%08x%08x%08x", entry.payload[2], entry.payload[1], entry.payload[0]);
        }
        else if (entry.payload_size <= 4 * SAL_UINT32_NOF_BITS)
        {
            PRT_CELL_SET("0x%08x%08x%08x%08x", entry.payload[3], entry.payload[2], entry.payload[1], entry.payload[0]);
        }
        else if (entry.payload_size <= 5 * SAL_UINT32_NOF_BITS)
        {
            PRT_CELL_SET("0x%08x%08x%08x%08x%08x", entry.payload[4], entry.payload[3], entry.payload[2],
                         entry.payload[1], entry.payload[0]);
        }
        else if (entry.payload_size <= 6 * SAL_UINT32_NOF_BITS)
        {
            PRT_CELL_SET("0x%08x%08x%08x%08x%08x%08x", entry.payload[5], entry.payload[4], entry.payload[3],
                         entry.payload[2], entry.payload[1], entry.payload[0]);
        }
        else
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Unexpected payload size, larger than 192 bits.\n");
        }

        SHR_IF_ERR_EXIT(mdb_direct_table_iterator_get_next
                        (unit, dbal_physical_table_id, app_id, &physical_entry_iterator, &entry, &is_end));
    }

    PRT_COMMITX;

exit:
    PRT_FREE;
    SHR_IF_ERR_CONT(mdb_direct_table_iterator_deinit(unit, dbal_physical_table_id, app_id, &physical_entry_iterator));
    SHR_FUNC_EXIT;
}

/**
* \brief
* Perform different operation on the direct tables
* See dnx_mdb_direct_man for the different options.
*****************************************************/
shr_error_e
cmd_dnx_mdb_direct(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dbal_physical_tables_e dbal_physical_table_id;
    dbal_physical_entry_t entry;
    int add_cmd, get_cmd, delete_cmd, dump_cmd;
    char *table_name = NULL;
    /*
     * app_id is unused in direct tables
     */
    uint32 app_id = 0;
    uint32 *array_uint32;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&entry, 0x0, sizeof(entry));

    SH_SAND_GET_STR("Table", table_name);
    SH_SAND_GET_BOOL("ADD", add_cmd);
    SH_SAND_GET_BOOL("Get", get_cmd);
    SH_SAND_GET_BOOL("Delete", delete_cmd);
    SH_SAND_GET_BOOL("dump", dump_cmd);

    SH_SAND_GET_INT32("key", entry.key[0]);

    SH_SAND_GET_INT32("payload_size", entry.payload_size);
    SH_SAND_GET_ARRAY32("payload", array_uint32);
    sal_memcpy(entry.payload, array_uint32, MDB_MAX_DIRECT_PAYLOAD_SIZE_IN_UINT32 * sizeof(entry.payload[0]));

    if (ISEMPTY(table_name))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "The Table parameter must be specified.\n");
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_physical_table_string_to_id(unit, table_name, &dbal_physical_table_id));

        /*
         * Use the table default payload size if not specified
         */
        if (entry.payload_size == 0)
        {
            int basic_size;
            SHR_IF_ERR_EXIT(mdb_direct_table_get_basic_size(unit, dbal_physical_table_id, &basic_size));
            entry.payload_size = (uint8) basic_size;
        }

        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(entry.p_mask, 0, entry.payload_size));

        if (add_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_direct_table_entry_add(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Add operation completed successfully on direct table.\n")));
        }
        else if (get_cmd != 0)
        {
            int ii;
            SHR_IF_ERR_EXIT(mdb_direct_table_entry_get(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Get operation completed successfully on direct table.\n")));
            for (ii = 0; ii < BITS2WORDS(entry.payload_size); ii++)
            {
                LOG_CLI((BSL_META("entry.payload[%d]: 0x%08x.\n"), BITS2WORDS(entry.payload_size) - 1 - ii,
                         entry.payload[BITS2WORDS(entry.payload_size) - 1 - ii]));
            }
        }
        else if (delete_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_direct_table_entry_delete(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Delete operation completed successfully on direct table.\n")));
        }
        else if (dump_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_diag_direct_table_dump(unit, dbal_physical_table_id, app_id, sand_control));
        }
        else
        {
            LOG_CLI((BSL_META("An operation must be specified.\n")));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_man_t dnx_mdb_em_man = {
    "Perform different operation on the EM table.",
    "Directly perform an operation on the EM tables. "
        "Specify up to 160bits for the key using key, specify the key_size, the payload_size and the payload in case of add. "
        "If key_size is unspecified, the default is assumed. " "Payload mask is assumed to be full.",
    "dnx mdb em add/get/delete [table=DBAL_PHYSICAL_TABLE_NAME] [key_size=0-160] [key=0xVALUE] [payload_size=1-180] [payload=0xVALUE] [app_id=VALUE]",
    "dnx mdb em add table=ISEM_1 key=0x12345678 key_size=32 payload_size=16 payload=0xabcd app_id=0\n"
        "dnx mdb em add table=LEM key=0x123456789abc key_size=49 payload_size=48 payload=0xabcdef0123 app_id=0\n"
        "dnx mdb em get table=ISEM_1 key=0x12345678 key_size=32 payload_size=16 app_id=0\n"
        "dnx mdb em delete table=ISEM_1 key=0x12345678 key_size=32 app_id=0"
};

static sh_sand_option_t dnx_mdb_em_options[] = {
    {"Table", SAL_FIELD_TYPE_STR, "DBAL physical table name", ""},
    {"ADD", SAL_FIELD_TYPE_BOOL, "Perform an add operation", "no"},
    {"Get", SAL_FIELD_TYPE_BOOL, "Perform a get operation", "no"},
    {"Delete", SAL_FIELD_TYPE_BOOL, "Perform a delete operation", "no"},
    {"key_size", SAL_FIELD_TYPE_UINT32, "The key size in bits, 0 means default", "0"},
    {"key", SAL_FIELD_TYPE_ARRAY32, "The 160bit key", "0"},
    {"payload_size", SAL_FIELD_TYPE_UINT32, "The payload size in bits", "0"},
    {"payload", SAL_FIELD_TYPE_ARRAY32, "Up to 180bits of the payload", "0"},
    {"app_id", SAL_FIELD_TYPE_UINT32, "The APP ID", "0"},
    {NULL}
};

/**
* \brief
* Perform different operation on the LPM (KAPS) table
* See dnx_mdb_lpm_man for the different options.
*****************************************************/
shr_error_e
cmd_dnx_mdb_em(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    dbal_physical_tables_e dbal_physical_table_id;
    dbal_physical_entry_t entry;
    int add_cmd, get_cmd, delete_cmd;
    char *table_name = NULL;
    uint32 app_id;
    uint32 *array_uint32;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&entry, 0x0, sizeof(entry));

    SH_SAND_GET_STR("Table", table_name);
    SH_SAND_GET_BOOL("ADD", add_cmd);
    SH_SAND_GET_BOOL("Get", get_cmd);
    SH_SAND_GET_BOOL("Delete", delete_cmd);

    SH_SAND_GET_UINT32("key_size", entry.key_size);
    SH_SAND_GET_ARRAY32("key", array_uint32);
    sal_memcpy(entry.key, array_uint32, MDB_MAX_EM_KEY_SIZE_IN_UINT32 * sizeof(entry.key[0]));

    SH_SAND_GET_INT32("payload_size", entry.payload_size);
    SH_SAND_GET_ARRAY32("payload", array_uint32);
    sal_memcpy(entry.payload, array_uint32, MDB_MAX_EM_KEY_PAYLOAD_IN_UINT32 * sizeof(entry.payload[0]));

    SH_SAND_GET_UINT32("app_id", app_id);

    if (ISEMPTY(table_name))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "The Table parameter must be specified.\n");
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_physical_table_string_to_id(unit, table_name, &dbal_physical_table_id));

        /*
         * If no key_size is given attempt to retrieve the default
         */
        if (entry.key_size == 0)
        {
            SHR_IF_ERR_EXIT(mdb_em_get_key_size(unit, dbal_physical_table_id, app_id, &entry.key_size));
            /*
             * If default key_size is 0
             */
            if (entry.key_size == 0)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "This app_id is not associated with this table.\n");
            }
        }

        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(entry.p_mask, 0, entry.payload_size));

        if (add_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_em_entry_add(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Add operation completed successfully on EM table.\n")));
        }
        else if (get_cmd != 0)
        {
            int ii;
            SHR_IF_ERR_EXIT(mdb_em_entry_get(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Get operation completed successfully on EM table.\n")));
            for (ii = 0; ii < BITS2WORDS(entry.payload_size); ii++)
            {
                LOG_CLI((BSL_META("entry.payload[%d]: 0x%08x.\n"), BITS2WORDS(entry.payload_size) - 1 - ii,
                         entry.payload[BITS2WORDS(entry.payload_size) - 1 - ii]));
            }
        }
        else if (delete_cmd != 0)
        {
            SHR_IF_ERR_EXIT(mdb_em_entry_delete(unit, dbal_physical_table_id, app_id, &entry));
            LOG_CLI((BSL_META("Delete operation completed successfully on EM table.\n")));
        }
        else
        {
            LOG_CLI((BSL_META("An operation must be specified.\n")));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_mdb_log_severity_options[] = {
    {"Severity", SAL_FIELD_TYPE_UINT32, "MDB Log Severity", "-1"},
    {NULL}
};

static sh_sand_man_t dnx_mdb_log_severity_man = {
    "Change or print the severity threshold of the logger.",
    "The different severity thresholds are - 0: Off, 1: Fatal, 2: Error, 3: Warn, 4: Info, 5: Verbose, 6:Debug. Pass Severity=-1 to print current severity.",
    "dnx mdb logger [Severity=Level/-1]",
    "dnx mdb logger Severity=5"
};

/**
* \brief
* Change the severity of the MDB module,
* if no parameter than dump the current severity
*****************************************************/
static shr_error_e
cmd_mdb_print_mode_set(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    int val;
    bsl_severity_t severity = bslSeverityOff;

    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_INT32("Severity", val);

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    LOG_CLI((BSL_META("Current MDB log severity: %s (%d)\n"), bsl_severity2str(severity), severity));

    if (val != -1)
    {
        if ((val > bslSeverityDebug) || (val < bslSeverityOff))
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Severity out of bounds %d\n", val);
        }
        else
        {
            SHR_SET_SEVERITY_FOR_MODULE(val);
            LOG_CLI((BSL_META("Set MDB log severity: %s (%d)\n"), bsl_severity2str(val), val));
            if (val > bslSeverityInfo)
            {
                LOG_CLI((BSL_META("MDB detailed prints enabled, every MDB action will print input/output.\n")));
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

sh_sand_man_t sh_dnx_mdb_man = {
    "MDB diagnostic commands",
    "The dispatcher for the different MDB diagnostics commands, which allow printing different information regarding the table, manipulating the LPM/Direct/EM tables and changing the MDB logging level.",
    "dnx mdb [command]",
    "dnx mdb info\n" "dnx mdb lpm\n" "dnx mdb logger"
};

/**
 * \brief DNX MDB diagnostic pack
 * List of the supported commands, pointer to command function and command usage function.
 * This is the entry point for MDB diagnostic commands
 */
sh_sand_cmd_t sh_dnx_mdb_cmds[] = {
   /*************************************************************************************************************************************
    *   CMD_NAME    *     CMD_ACTION            * Next                    *        Options                 *         MAN                *
    *               *                           * Level                   *                                *                            *
    *               *                           * CMD                     *                                *                            *
    *************************************************************************************************************************************/
    {"Info", cmd_dnx_mdb_info_print, NULL, dnx_mdb_info_options, &dnx_mdb_info_man}
    ,
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    {"LPM", cmd_dnx_mdb_lpm, NULL, dnx_mdb_lpm_options, &dnx_mdb_lpm_man}
    ,
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
    {"Direct", cmd_dnx_mdb_direct, NULL, dnx_mdb_direct_options, &dnx_mdb_direct_man}
    ,
    {"EM", cmd_dnx_mdb_em, NULL, dnx_mdb_em_options, &dnx_mdb_em_man}
    ,
    {"LoGger", cmd_mdb_print_mode_set, NULL, dnx_mdb_log_severity_options, &dnx_mdb_log_severity_man}
    ,
    {"TeST", cmd_mdb_test, NULL, dnx_mdb_test_options, &dnx_mdb_test_man}
    ,
    {NULL}
};

/*
 * General shell style usage
 */
const char cmd_mdb_usage[] = "Please use \"mdb usage\" for help\n";
