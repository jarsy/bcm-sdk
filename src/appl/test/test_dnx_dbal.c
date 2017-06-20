/*
 * ! \file diag_dnx_dbal.c add file description here 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DIAGDBALDNX

/*************
 * INCLUDES  *
 *************/
#include <appl/diag/test.h>
#include <appl/diag/diag.h>
#include <soc/drv.h>
#include <sal/appl/sal.h>
#include <shared/bsl.h>
#include <bcm/l2.h>

#if defined(BCM_DNX_SUPPORT)

#include <soc/dnx/dbal/dbal.h>
#include <appl/diag/sand/diag_sand_prt.h>
#include "src/soc/dnx/dbal/dbal_string_mgmt.h"
#include "src/soc/dnx/dbal/dbal_internal.h"
#include "src/appl/diag/dnx/dbal/diag_dnx_dbal_internal.h"

/** 
 * \brief Define the numbet of field updates made before commit 
 * in UPDATE_FIELD test case 
 */
#define MAX_NUM_UPDATE_BEFORE_COMMIT 5

/**
 * \brief This is a EMPTY TABLE IS for UT implemantation.
 * used to indicate n EMPTY table in test table list.
 */
#define DBAL_EMPTY_UT_TABLE_ID  (DBAL_NOF_TABLES+1)

/** 
 * \brief 
 * Typedef:    dbal_ut_modes_t
 * Purpose:    In DBAL UT we can keep table content 
 *             after test end or restore table content.
 */
typedef enum dbal_ut_modes_e
{
    DBAL_UT_KEEP_TABLE = 0,     /* keep table values at the end of the test */
    DBAL_UT_RESTORE_TABLE,      /* restore table values at the end of the test */
} dbal_ut_modes_t;

typedef cmd_result_t(
    *dbal_ut_func_t) (
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);

/** 
 * \brief Define the Maximum number of tables in 
 * one DBAL UT regresion test. 
 */
#define DBAL_MAX_NUM_UT_TABLES	3

/** 
 * \brief 
 * Typedef:    dbal_ut_t 
 * Purpose:    dtatabase definition for In DBAL UT 
 */
typedef struct dbal_ut_s
{
    char *test_name;            /* Test Name */
    dbal_ut_func_t test_func;
    dbal_tables_e table_id[DBAL_MAX_NUM_UT_TABLES];
    uint8 is_passed[DBAL_MAX_NUM_UT_TABLES];
    char *test_description;
} dbal_ut_t;

/*
 * external function declaration
 */
extern cmd_result_t dbal_logical_tables_test(
    int unit,
    char *input_strings[CMD_MAX_NOF_INPUTS]);

/*
 * Internal function declaration
 */
static cmd_result_t test_dnx_dbal_update_field_before_commit(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_partial_access(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
cmd_result_t test_dnx_dbal_multiple_instance_field_test(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_rt_access(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_wrong_field_size(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_mact_entry_add_example(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_parent_field_mapping(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_mact_entry_add_example(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_xml_parsing_validation(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_wrong_field_access(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);
static cmd_result_t test_dnx_dbal_emun_mapping(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode);

dbal_ut_t dbal_ut_list[] = {
  /*********************************************************************************************************************/
    {"UPDATE_FIELD",                                                                              /** Test Name        */
     test_dnx_dbal_update_field_before_commit,                                                    /** Test Function    */
     {DBAL_TABLE_EGRESS_PORT, DBAL_TABLE_EGRESS_DEFAULT_AC_PROF, DBAL_TABLE_EGRESS_PORT},         /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Update table key/payload fields number of times before commit"},                            /** Test Description */
  /*********************************************************************************************************************/
    {"PARTIAL_ACCESS",                                                                            /** Test Name        */
     test_dnx_dbal_partial_access,                                                                /** Test Function    */
     {DBAL_TABLE_EGRESS_DEFAULT_AC_PROF, DBAL_TABLE_SW_STATE_SUB_FIELD_EXAMPLE, DBAL_NOF_TABLES}, /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Modify one payload field, check other not changed"},                                        /** Test Description */
  /*********************************************************************************************************************/
    {"SUB_FIELD",                                                                                 /** Test Name        */
     test_dnx_dbal_parent_field_mapping,                                                          /** Test Function    */
     {DBAL_TABLE_SW_STATE_SUB_FIELD_EXAMPLE, DBAL_NOF_TABLES, DBAL_NOF_TABLES},                   /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Access table with SUB FIELD field type"},                                                   /** Test Description */
  /*********************************************************************************************************************/
    {"MUL_INSTANCE",                                                                              /** Test Name        */
     test_dnx_dbal_multiple_instance_field_test,                                                  /** Test Function    */
     {DBAL_TABLE_EXAMPLE_TABLE_FOR_MUL_INSTANCES, DBAL_NOF_TABLES, DBAL_NOF_TABLES},              /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Access multiple instance type table"},                                                      /** Test Description */
  /*********************************************************************************************************************/
    {"RT_ACCESS",                                                                                 /** Test Name        */
     test_dnx_dbal_rt_access,                                                                     /** Test Function    */
     {DBAL_TABLE_SW_RESULT_TYPE_EXAMPLE, DBAL_NOF_TABLES, DBAL_NOF_TABLES},                       /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Access multiple result type table"},                                                        /** Test Description */
  /*********************************************************************************************************************/
    {"ENUM_MAPPING",                                                                              /** Test Name        */
     test_dnx_dbal_emun_mapping,                                                                  /** Test Function    */
     {DBAL_TABLE_SW_STATE_TABLE_EXAMPLE_1, DBAL_NOF_TABLES, DBAL_NOF_TABLES},                     /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Access table with ENUM field type "},                                                       /** Test Description */
  /*********************************************************************************************************************/
    {"WRONG_FIELD_SIZE",                                                                          /** Test Name        */
     test_dnx_dbal_wrong_field_size,                                                              /** Test Function    */
     {DBAL_TABLE_EGRESS_PORT, DBAL_NOF_TABLES, DBAL_NOF_TABLES},                                  /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Negative Test, access table with wrong field size"},                                        /** Test Description */
  /*********************************************************************************************************************/
    {"WRONG_FIELD_ACCESS",                                                                        /** Test Name        */
     test_dnx_dbal_wrong_field_access,                                                            /** Test Function    */
     {DBAL_TABLE_EGRESS_PORT, DBAL_NOF_TABLES, DBAL_NOF_TABLES},                                  /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Negative Test, access table with wrong field id"},                                          /** Test Description */
  /*********************************************************************************************************************/
    {"MACT_ENTRY",                                                                                /** Test Name        */
     test_dnx_dbal_mact_entry_add_example,                                                        /** Test Function    */
     {DBAL_TABLE_MACT, DBAL_NOF_TABLES, DBAL_NOF_TABLES},                                         /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "TBD - TestDescription"},                                                                    /** Test Description */
  /*********************************************************************************************************************/
    {"XML_PARSING_VALIDATION",                                                                    /** Test Name        */
     test_dnx_dbal_xml_parsing_validation,                                                        /** Test Function    */
     {DBAL_EMPTY_UT_TABLE_ID, DBAL_NOF_TABLES, DBAL_NOF_TABLES},                                  /** Test Tables      */
     {FALSE, FALSE, FALSE},                                                                       /** Test Status      */
     "Immitate the init procedure & compare to the expected values"},                             /** Test Description */
  /*********************************************************************************************************************/
};

/**
 * fields values for random values test
 */
uint32 key_field_val[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
uint32 field_val[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
/**
 * fields get values for random values test
 */
uint32 get_field_val[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

/*************
 * FUNCTIONS *
 *************/
/** 
 * \brief 
 * compare two fields array values, each field represented as array32. 
 * returnes 0 if two fields value equal  otherwise -1 
 */
static int
test_dnx_dbal_compare_fields_values(
    int unit,
    dbal_fields_e field_id,
    int length,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS],
    uint32 field_val_get[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS])
{
    int jj;
    uint32 bit, bit_get;

    for (jj = 0; jj < length; jj++)
    {
        bit = (field_val[jj / 32] >> (jj % 32)) & 0x1;
        bit_get = (field_val_get[jj / 32] >> (jj % 32)) & 0x1;
        if (bit != bit_get)
        {
            LOG_DEBUG(BSL_LOG_MODULE, (BSL_META_U(unit, "Comparison error: field=%s, bit=%d, expect=%d, get=%d\n"),
                                       dbal_field_to_string(unit, field_id), jj, bit, bit_get));
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Comparison error: field=%s, val=%d, expect_val=%d\n"),
                                       dbal_field_to_string(unit, field_id), field_val[jj / 32],
                                       field_val_get[jj / 32]));
            return -1;
        }
    }

    return 0;
}

/**
 * \brief 
 * This test sets the table key & payload fields number of times 
 * before commit.  read the all table and verify all payload 
 * fields have the correct value. 
 */
cmd_result_t
test_dnx_dbal_update_field_before_commit(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 entry_handle_wr = DBAL_SW_NOF_ENTRY_HANDLES;
    uint32 entry_handle_rd = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;
    dbal_field_type_e field_type;
    int field_index, update_index;
    uint32 reset_val = 0;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    /*
     * Get wr & rd table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_wr));
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_rd));

    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        for (update_index = 0; update_index < MAX_NUM_UPDATE_BEFORE_COMMIT; update_index++)
        {
            DIAG_DBAL_RANDOM_FIELD_VAL(key_field_val[field_index], table->keys_info[field_index].field_nof_bits, TRUE);
            dbal_entry_key_field_arr32_set(unit, entry_handle_wr, table->keys_info[field_index].field_id,
                                           INST_SINGLE, key_field_val[field_index]);
        }
    }

    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->multi_res_info[0].results_info[field_index].field_id, &field_type);
        for (update_index = 0; update_index < MAX_NUM_UPDATE_BEFORE_COMMIT; update_index++)
        {
            DIAG_DBAL_RANDOM_FIELD_VAL(field_val[field_index],
                                       table->multi_res_info[0].results_info[field_index].field_nof_bits, FALSE);
            dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                             table->multi_res_info[0].results_info[field_index].field_id, INST_SINGLE,
                                             field_val[field_index]);
        }
    }

    /*
     * Commit table values
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Read table fields
     */
    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        /*
         * restore key fields 
         */
        dbal_entry_key_field_arr32_set(unit, entry_handle_rd, table->keys_info[field_index].field_id,
                                       INST_SINGLE, key_field_val[field_index]);
    }

    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        /*
         * restore value fields 
         */
        dbal_entry_value_field_arr32_get(unit, entry_handle_rd,
                                         table->multi_res_info[0].results_info[field_index].field_id,
                                         INST_SINGLE, get_field_val[field_index]);
    }

    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * compare results 
     */
    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        if (test_dnx_dbal_compare_fields_values
            (unit, table->multi_res_info[0].results_info[field_index].field_id,
             table->multi_res_info[0].results_info[field_index].field_nof_bits,
             field_val[field_index], get_field_val[field_index]))
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "table %s wrong value\n", dbal_logical_table_to_string(unit, table_id));
        }
    }

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Finish \n")));

    /*
     * In case mode is restore we set the entry to 0
     */
    if (mode == DBAL_UT_RESTORE_TABLE)
    {
        for (field_index = 0; field_index < table->nof_key_fields; field_index++)
        {
            dbal_entry_key_field_arr32_set(unit, entry_handle_wr, table->keys_info[field_index].field_id,
                                           INST_SINGLE, key_field_val[field_index]);
        }

        for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
        {
            dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                             table->multi_res_info[0].results_info[field_index].field_id, INST_SINGLE,
                                             &reset_val);
        }
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));
    }

exit:
    dbal_entry_handle_release(unit, entry_handle_rd);
    dbal_entry_handle_release(unit, entry_handle_wr);
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * This test sets the table key & payload fields &commit the 
 * changes. Then it update only one payload field & commit. read
 * the all table and verify all payload fields have the correct 
 * value. 
 */
cmd_result_t
test_dnx_dbal_partial_access(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 entry_handle_wr = DBAL_SW_NOF_ENTRY_HANDLES;
    uint32 entry_handle_rd = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;
    dbal_field_type_e field_type;
    int field_index;
    uint32 reset_val = 0;

    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
    /*
     * Get wr & rd table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_wr));
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_rd));

    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->keys_info[field_index].field_id, &field_type);
        DIAG_DBAL_RANDOM_FIELD_VAL(key_field_val[field_index], table->keys_info[field_index].field_nof_bits, TRUE);
        dbal_entry_key_field_arr32_set(unit, entry_handle_wr, table->keys_info[field_index].field_id,
                                       INST_SINGLE, key_field_val[field_index]);
    }

    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->multi_res_info[0].results_info[field_index].field_id, &field_type);
        DIAG_DBAL_RANDOM_FIELD_VAL(field_val[field_index],
                                   table->multi_res_info[0].results_info[field_index].field_nof_bits, FALSE);
        dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                         table->multi_res_info[0].results_info[field_index].field_id,
                                         INST_SINGLE, field_val[field_index]);
    }

    /*
     * Commit table values
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_NORMAL));

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_wr));
    /*
     * Set key field for new handle
     */
    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->keys_info[field_index].field_id, &field_type);
        dbal_entry_key_field_arr32_set(unit, entry_handle_wr, table->keys_info[field_index].field_id,
                                       INST_SINGLE, key_field_val[field_index]);
    }

    /*
     * Update one field
     */
    field_index = 0;

    dbal_fields_type_get(unit, table->multi_res_info[0].results_info[field_index].field_id, &field_type);
    DIAG_DBAL_RANDOM_FIELD_VAL(field_val[table->nof_key_fields + field_index],
                               table->multi_res_info[0].results_info[field_index].field_nof_bits, FALSE);
    dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                     table->multi_res_info[0].results_info[field_index].field_id,
                                     INST_SINGLE, field_val[field_index]);

    /*
     * Commit table values
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Read table fields
     */
    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        /*
         * restore key fields 
         */
        dbal_entry_key_field_arr32_set(unit, entry_handle_rd, table->keys_info[field_index].field_id,
                                       INST_SINGLE, key_field_val[field_index]);
    }

    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->multi_res_info[0].results_info[field_index].field_id, &field_type);
        dbal_entry_value_field_arr32_get(unit, entry_handle_rd,
                                         table->multi_res_info[0].results_info[field_index].field_id,
                                         INST_SINGLE, get_field_val[field_index]);
    }
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * compare results 
     */
    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        if (test_dnx_dbal_compare_fields_values(unit,
                                                table->multi_res_info[0].results_info[field_index].field_id,
                                                table->multi_res_info[0].results_info[field_index].field_nof_bits,
                                                field_val[field_index], get_field_val[field_index]))
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "table %s wrong value\n", dbal_logical_table_to_string(unit, table_id));
        }
    }

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Finish \n")));

    /*
     * In case mode is restore we set the entry to 0
     */
    if (mode == DBAL_UT_RESTORE_TABLE)
    {
        for (field_index = 0; field_index < table->nof_key_fields; field_index++)
        {
            dbal_entry_key_field_arr32_set(unit, entry_handle_wr, table->keys_info[field_index].field_id,
                                           INST_SINGLE, key_field_val[field_index]);
        }
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

        for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
        {
            dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                             table->multi_res_info[0].results_info[field_index].field_id, INST_SINGLE,
                                             &reset_val);
        }
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));
    }

exit:
    dbal_entry_handle_release(unit, entry_handle_rd);
    dbal_entry_handle_release(unit, entry_handle_wr);
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * This test write a multiple instances field and read & compare
 * the results. then write & update a multiple instances field
 * and read & compare the results. 
 */
cmd_result_t
test_dnx_dbal_multiple_instance_field_test(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 entry_handle_id = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;
    uint32 counter_val = 0x0;
    uint8 instance_index;
    uint32 field_val[DBAL_NUM_OF_FIELD_INSTANCES][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
    uint32 get_field_val[DBAL_NUM_OF_FIELD_INSTANCES][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
    int nof_instances;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify table ID, This test is a special test that can run only 
     * run with DBAL_TABLE_EXAMPLE_TABLE_FOR_MUL_INSTANCES table
     */
    if (table_id != DBAL_TABLE_EXAMPLE_TABLE_FOR_MUL_INSTANCES)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "This test can not run with %s table\n",
                     dbal_logical_table_to_string(unit, table_id));
    }

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    /*
     * Get table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_id));

    /*
     * Set Key Fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, 15);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, 0);

    nof_instances = table->multi_res_info[0].results_info[0].nof_instances;

    /*
     * Set counter instance Field 
     */
    for (instance_index = 0; instance_index < nof_instances; instance_index++, counter_val++)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_COUNTER, instance_index, counter_val);
        field_val[instance_index][0] = counter_val;
    }

    /*
     * Commit table values
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Read table fields
     */
    for (instance_index = 0; instance_index < nof_instances; instance_index++)
    {
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_COUNTER, instance_index,
                                     get_field_val[instance_index]);
    }
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * compare results 
     */
    for (instance_index = 0; instance_index < nof_instances; instance_index++)
    {
        if (test_dnx_dbal_compare_fields_values
            (unit, DBAL_FIELD_COUNTER,
             table->multi_res_info[0].results_info[0].field_nof_bits,
             field_val[instance_index], get_field_val[instance_index]))
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "table %s wrong value\n", dbal_logical_table_to_string(unit, table_id));
        }
    }

    /*
     * Set counter instance Field 
     */
    counter_val = 0;
    for (instance_index = 0; instance_index < nof_instances; instance_index++, counter_val++)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_COUNTER, instance_index, counter_val);
        field_val[instance_index][0] = counter_val;
    }

    counter_val = 1;
    for (instance_index = 0; instance_index < nof_instances; instance_index++, counter_val++)
    {
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_COUNTER, instance_index, counter_val);
        field_val[instance_index][0] = counter_val;
    }

    /*
     * Commit table values
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Read table fields
     */
    for (instance_index = 0; instance_index < nof_instances; instance_index++)
    {
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_COUNTER, instance_index,
                                     get_field_val[instance_index]);
    }
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * compare results 
     */
    for (instance_index = 0; instance_index < nof_instances; instance_index++)
    {
        if (test_dnx_dbal_compare_fields_values
            (unit, DBAL_FIELD_COUNTER,
             table->multi_res_info[0].results_info[0].field_nof_bits,
             field_val[instance_index], get_field_val[instance_index]))
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "table %s wrong value\n", dbal_logical_table_to_string(unit, table_id));
        }
    }

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Finish \n")));

exit:
    dbal_entry_handle_release(unit, entry_handle_id);
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * This test verify the result type table functionality. 
 * it access table with multiple result types defined and access 
 * it in different ways using different result type values. 
 */
cmd_result_t
test_dnx_dbal_rt_access(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 field_val_int[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
    int rv;
    uint32 entry_handle_id = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;

    int bsl_dbal_severity, bsl_diag_severity;
    uint32 result_type;
    uint32 gport_val = 0x11;
    uint32 fec_val = 0x22;
    uint32 peer_gport_val = 0x33;
    uint32 vlan_domain_val = 0x44;
    uint32 esem_access_val = 0x5;
    uint32 tx_outer_tag_valid_val = 0x1;
    uint32 tx_outer_tag_vid_val = 0x77;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify table ID, This test is a special test that can run only 
     * run with DBAL_TABLE_SW_RESULT_TYPE_EXAMPLE table
     */
    if (table_id != DBAL_TABLE_SW_RESULT_TYPE_EXAMPLE)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "This test can not run with %s table\n",
                     dbal_logical_table_to_string(unit, table_id));
    }

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
    /*
     * Get table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_id));

    /*
     * Set Key Fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, 15);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, 0);

    /*
     * Set RT1 result type fields
     */
    result_type = DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT1;
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, result_type);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_GPORT_FOR_LEARNING, INST_SINGLE, gport_val);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_FEC, INST_SINGLE, fec_val);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PEER_GPORT, INST_SINGLE, peer_gport_val);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    rv = dbal_entry_get(unit, entry_handle_id, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE);

    if (rv)
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(rv, "entry get failed\n");
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT1))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_RESULT_TYPE), field_val_int[0],
                     DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT1);
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_GPORT_FOR_LEARNING, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != gport_val))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_GPORT_FOR_LEARNING), field_val_int[0], gport_val);
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_FEC, INST_SINGLE, field_val_int);
    if (rv || (field_val_int[0] != fec_val))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_FEC), field_val_int[0], fec_val);
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_PEER_GPORT, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != peer_gport_val))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_PEER_GPORT), field_val_int[0], peer_gport_val);
    }

     /** negative test, checking field that is not exists */
    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_ESEM_ACCESS_CMD, INST_SINGLE,
                                                 field_val_int);
    if (rv == 0)
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "negative test passed. field %s should not be found in entry,\n",
                     dbal_field_to_string(unit, DBAL_FIELD_ESEM_ACCESS_CMD));
    }

    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

    dbal_entry_handle_release(unit, entry_handle_id);

    /*
     * table handle result type cannot be changed
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_id));

    /*
     * Set Key Fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, 15);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, 0);

    result_type = DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT1;
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, result_type);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Set RT2 result type fields
     */
    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    /*
     * Try to change table handle result type to RT2 - this should fail
     */
    result_type = DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT2;
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, result_type);

    if (dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL,
                     "Negative test passed with no ERROR - change result type in same handle succedded\n");
    }

    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

    /*
     * get a new handle 
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_id));

    /*
     * Set Key Fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, 15);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, 0);

    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, result_type);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_DOMAIN, INST_SINGLE, vlan_domain_val);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ESEM_ACCESS_CMD, INST_SINGLE, esem_access_val);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VALID, INST_SINGLE,
                                 tx_outer_tag_valid_val);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VID, INST_SINGLE, tx_outer_tag_vid_val);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    rv = dbal_entry_get(unit, entry_handle_id, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE);

    if (rv)
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(rv, "entry get failed\n");
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT2))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_RESULT_TYPE), field_val_int[0],
                     DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT2);
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_VLAN_DOMAIN, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != vlan_domain_val))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_VLAN_DOMAIN), field_val_int[0], vlan_domain_val);
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_ESEM_ACCESS_CMD, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != esem_access_val))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_ESEM_ACCESS_CMD), field_val_int[0], esem_access_val);
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VALID, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != tx_outer_tag_valid_val))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_TX_OUTER_TAG_VALID), field_val_int[0],
                     tx_outer_tag_valid_val);
    }

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_TX_OUTER_TAG_VID, INST_SINGLE,
                                                 field_val_int);
    if (rv || (field_val_int[0] != tx_outer_tag_vid_val))
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: field_id[%s], get_field_val[%d] != expected_val[%d],\n",
                     dbal_field_to_string(unit, DBAL_FIELD_TX_OUTER_TAG_VID), field_val_int[0], tx_outer_tag_vid_val);
    }

    /** negative test, checking field that is not exists  */
    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    rv = dbal_entry_handle_value_field_arr32_get(unit, entry_handle_id, DBAL_FIELD_PEER_GPORT, INST_SINGLE,
                                                 field_val_int);
    if (rv == 0)
    {
        dbal_entry_handle_release(unit, entry_handle_id);
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "negative test passed. field %s should not be found in entry,\n",
                     dbal_field_to_string(unit, DBAL_FIELD_ESEM_ACCESS_CMD));
    }

    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

    dbal_entry_handle_release(unit, entry_handle_id);

    /*
     * Negative test - try to write to other result type field
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_id));

    /*
     * Set Key Fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_PP_PORT, INST_SINGLE, 15);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_CORE_ID, INST_SINGLE, 0);

    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    /*
     * dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
     * DBAL_RESULT_TYPE_SW_RESULT_TYPE_EXAMPLE_RT2);
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_GPORT_FOR_LEARNING, INST_SINGLE, gport_val);
    if (dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL,
                     "Negative test passed with no ERROR - access wrong field for current result type\n");
    }

    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * This is a negative test. This test set the key fields of the 
 * table correctly. * For payload fields it sets the fields with 
 * MAX values in length of field_nof_bits+2. then commit the 
 * changes, this test should FAIL. 
 */
cmd_result_t
test_dnx_dbal_wrong_field_size(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 entry_handle_wr = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;
    dbal_field_type_e field_type;
    int field_index;
    int bsl_dbal_severity, bsl_diag_severity;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    /*
     * Get wr & rd table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_wr));

    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->keys_info[field_index].field_id, &field_type);
        DIAG_DBAL_RANDOM_FIELD_VAL(key_field_val[field_index], table->keys_info[field_index].field_nof_bits, TRUE);
        dbal_entry_key_field_arr32_set(unit, entry_handle_wr, table->keys_info[field_index].field_id,
                                       INST_SINGLE, key_field_val[field_index]);
    }

    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->multi_res_info[0].results_info[field_index].field_id, &field_type);
        DIAG_DBAL_MAX_FIELD_VAL(field_val[field_index],
                                (table->multi_res_info[0].results_info[field_index].field_nof_bits + 2), field_type);

        SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
        SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
        SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

        dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                         table->multi_res_info[0].results_info[field_index].field_id,
                                         INST_SINGLE, field_val[field_index]);

        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
        SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    }

    /*
     * Commit table values, This operation should fail as this is a negative test
     */
    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    if (dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_NORMAL) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Negative test passed with no ERROR\n");
    }

    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * This is a negative test. This test set the key fields of the 
 * table correctly. For payload fields it sets the fields with 
 * MAX values in length of  field_nof_bits+2. then commit the 
 * changes, this test should FAIL. 
 * *******************************************************
 */
cmd_result_t
test_dnx_dbal_wrong_field_access(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 entry_handle_wr = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;
    dbal_field_type_e field_type;
    int field_index;
    int bsl_dbal_severity, bsl_diag_severity;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    /*
     * Get wr & rd table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_wr));

    for (field_index = 0; field_index < table->nof_key_fields; field_index++)
    {
        DIAG_DBAL_RANDOM_FIELD_VAL(key_field_val[field_index], table->keys_info[field_index].field_nof_bits, TRUE);
        dbal_entry_key_field_arr32_set(unit, entry_handle_wr, table->keys_info[field_index].field_id,
                                       INST_SINGLE, key_field_val[field_index]);
    }

    for (field_index = 0; field_index < table->multi_res_info[0].nof_result_fields; field_index++)
    {
        dbal_fields_type_get(unit, table->multi_res_info[0].results_info[field_index].field_id, &field_type);
        DIAG_DBAL_MAX_FIELD_VAL(field_val[field_index],
                                (table->multi_res_info[0].results_info[field_index].field_nof_bits), field_type);
        dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                         table->multi_res_info[0].results_info[field_index].field_id,
                                         INST_SINGLE, field_val[field_index]);
    }

    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    DIAG_DBAL_MAX_FIELD_VAL(field_val[field_index],
                            (table->multi_res_info[0].results_info[field_index].field_nof_bits), field_type);
    dbal_entry_value_field_arr32_set(unit, entry_handle_wr,
                                     table->multi_res_info[0].results_info[field_index].field_id,
                                     INST_SINGLE, field_val[field_index]);

    /*
     * Commit table values, This operation should fail as this is a negative test
     */
    if (dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_NORMAL) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Negative test passed with no ERROR\n");
    }
    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
        SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * This test verify the enum mapping fields functionality. 
 * it access table with ENUM_TEST field and access it in  
 * different ways using different enum values including wrong 
 * values. 
 */
cmd_result_t
test_dnx_dbal_emun_mapping(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 entry_handle_wr = DBAL_SW_NOF_ENTRY_HANDLES;
    uint32 entry_handle_rd = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;

    uint32 destination_val = 0x22;
    uint32 get_destination_val;
    uint32 gport_val = 0x33;
    uint32 get_gport_val;
    uint32 enum_test_val = DBAL_ENUM_FVAL_ENUM_TEST_DROP;
    uint32 get_enum_test_val;
    dbal_field_basic_info_t field_info;
    int bsl_dbal_severity, bsl_diag_severity;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify table ID, This test is a special test that can run only 
     * run with DBAL_TABLE_SW_STATE_TABLE_EXAMPLE_1 table
     */
    if (table_id != DBAL_TABLE_SW_STATE_TABLE_EXAMPLE_1)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "This test can not run with %s table\n",
                     dbal_logical_table_to_string(unit, table_id));
    }

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
    /*
     * Get wr & rd table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_wr));
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_rd));

    /*
     * Set Key Fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_wr, DBAL_FIELD_IN_LIF, INST_SINGLE, 15);

    dbal_entry_key_field32_set(unit, entry_handle_rd, DBAL_FIELD_IN_LIF, INST_SINGLE, 15);

    /*
     * Set gport Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_GPORT_FOR_LEARNING, INST_SINGLE, gport_val);

    /*
     * Set destination Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_DESTINATION, INST_SINGLE, destination_val);

    /*
     * Set enum_test Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_ENUM_TEST, INST_SINGLE, enum_test_val);

    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Get gport Payload
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_GPORT_FOR_LEARNING, INST_SINGLE, &get_gport_val);
    /*
     * Get destination Payload
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_DESTINATION, INST_SINGLE, &get_destination_val);
    /*
     * Get enum_test Payload
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_ENUM_TEST, INST_SINGLE, &get_enum_test_val);

    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Check the GPORT_FOR_LEARNING val is as expected
     */
    if (get_gport_val != gport_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_gport_val[%d] != gport_val[%d],\n",
                     get_gport_val, gport_val);
    }

    /*
     * Check the DESTINATION val is as expected
     */
    if (get_destination_val != destination_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_destination_val[%d] != destination_val[%d],\n",
                     get_destination_val, destination_val);
    }

    /*
     * Check the ENUM_TEST val is as expected
     */
    if (get_enum_test_val != enum_test_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_enum_test_val[%d] != enum_test_val[%d],\n",
                     get_enum_test_val, enum_test_val);
    }

    /*
     * Set enum_test Payload
     */
    enum_test_val = DBAL_ENUM_FVAL_ENUM_TEST_SNOOP;
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_ENUM_TEST, INST_SINGLE, enum_test_val);

    enum_test_val = DBAL_ENUM_FVAL_ENUM_TEST_REDIRECT;
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_ENUM_TEST, INST_SINGLE, enum_test_val);

    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Get enum_test Payload
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_ENUM_TEST, INST_SINGLE, &get_enum_test_val);
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Check the ENUM_TEST val is as expected
     */
    if (get_enum_test_val != enum_test_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_enum_test_val[%d] != enum_test_val[%d]\n",
                     get_enum_test_val, enum_test_val);
    }

    /*
     * Set enum_test Payload with wrong value 
     */
    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, DBAL_FIELD_ENUM_TEST, &field_info));
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_ENUM_TEST, INST_SINGLE, field_info.nof_enum_values);
    if (dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Negative test passed with no ERROR\n");
    }
    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
        SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Finish \n")));

exit:
    dbal_entry_handle_release(unit, entry_handle_rd);
    dbal_entry_handle_release(unit, entry_handle_wr);
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * This test verify the sub fields functionality. 
 * it access table with DESTINATION field and access it in  
 * different ways using different sub fields. 
 */
cmd_result_t
test_dnx_dbal_parent_field_mapping(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    uint32 entry_handle_wr = DBAL_SW_NOF_ENTRY_HANDLES;
    uint32 entry_handle_rd = DBAL_SW_NOF_ENTRY_HANDLES;
    CONST dbal_logical_table_t *table;
    uint32 get_destination_val;
    uint32 fec_val = 0x33;
    uint32 get_fec_val, get_fec_val_error;
    uint32 mc_id_val = 0x44;
    uint32 get_mc_id_val;
    uint32 port_id_val = 0x66;
    uint32 get_port_id_val;
    uint32 sub_field_val;
    dbal_fields_e sub_field_id;
    int bsl_dbal_severity, bsl_diag_severity;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify table ID, This test is a special test that can run only 
     * run with SW_STATE_SUB_FIELD_EXAMPLE table
     */
    if (table_id != DBAL_TABLE_SW_STATE_SUB_FIELD_EXAMPLE)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "This test can not run with %s table\n",
                     dbal_logical_table_to_string(unit, table_id));
    }

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));
    /*
     * Get wr & rd table handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_wr));
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle_rd));

    /*
     * Set Key Fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_wr, DBAL_FIELD_IN_LIF, INST_SINGLE, 15);
    dbal_entry_key_field32_set(unit, entry_handle_wr, DBAL_FIELD_VRF, INST_SINGLE, 2);

    dbal_entry_key_field32_set(unit, entry_handle_rd, DBAL_FIELD_IN_LIF, INST_SINGLE, 15);
    dbal_entry_key_field32_set(unit, entry_handle_rd, DBAL_FIELD_VRF, INST_SINGLE, 2);

    /*
     * Set fec Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_FEC, INST_SINGLE, fec_val);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Get fec Payload
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_FEC, INST_SINGLE, &get_fec_val);
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Set MC_ID Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_MC_ID, INST_SINGLE, mc_id_val);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Get fec Payload, This should fail as last set field id is DBAL_FIELD_MC_ID
     */
    SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
    SHR_SET_SEVERITY_FOR_MODULE(bslSeverityFatal);
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityFatal);

    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_FEC, INST_SINGLE, &get_fec_val_error);
    if (dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Negative test passed with no ERROR\n");
    }

    SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
        SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

    /*
     * Get MC_ID Payload
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_MC_ID, INST_SINGLE, &get_mc_id_val);
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Check the MC_ID val is as expected
     */
    if (get_mc_id_val != mc_id_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_mc_id_val[%d] != mc_id_val[%d],\n", get_mc_id_val,
                     mc_id_val);
    }

    /*
     * Check multiple set to field and correct behavior 
     * Set fec Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_FEC, INST_SINGLE, fec_val);

    /*
     * Set MC_ID Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_MC_ID, INST_SINGLE, mc_id_val);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Set PORT_ID Payload
     */
    dbal_entry_value_field32_set(unit, entry_handle_wr, DBAL_FIELD_PORT_ID, INST_SINGLE, port_id_val);
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_wr, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Get PORT_ID Payload - we set the field multiple times and check that we get the latest value when we read
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_PORT_ID, INST_SINGLE, &get_port_id_val);
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Get DESTINATION Payload
     */
    dbal_entry_value_field32_get(unit, entry_handle_rd, DBAL_FIELD_DESTINATION, INST_SINGLE, &get_destination_val);
    SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle_rd, DBAL_COMMIT_KEEP_HANDLE));

    /*
     * Check the FEC val is as expected
     */
    if (get_fec_val != fec_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_fec_val[%d] != fec_val[%d]\n", get_fec_val, fec_val);
    }

    /*
     * Check the PORT_ID val is as expected
     */
    if (get_port_id_val != port_id_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_port_id_val[%d] != port_id_val[%d]\n",
                     get_port_id_val, port_id_val);
    }

    /*
     * Check the DESTINATION field val not equal to PORT_ID val
     */
    if (get_destination_val == port_id_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: get_destination_val[%d] == port_id_val[%d],\n",
                     get_destination_val, port_id_val);
    }

    /*
     * Encode Destination field
     */
    SHR_IF_ERR_EXIT(dbal_fields_sub_field_info_get(unit, DBAL_FIELD_DESTINATION, get_destination_val,
                                                   &sub_field_id, &sub_field_val));

    /*
     * Check the ENCODED DESTINATION field val is as expected
     */
    if (sub_field_val != port_id_val)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: encoded get_destination_val[%d] != port_id_val[%d],\n",
                     sub_field_val, port_id_val);
    }

    /*
     * Check the ENCODED DESTINATION field ID is as expected
     */
    if (sub_field_id != DBAL_FIELD_PORT_ID)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Comparison error: encoded destination field id[%d] != DBAL_FIELD_PORT_ID,\n",
                     sub_field_id);
    }

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Finish \n")));

exit:
    dbal_entry_handle_release(unit, entry_handle_rd);
    dbal_entry_handle_release(unit, entry_handle_wr);
    SHR_FUNC_EXIT;
}


cmd_result_t
test_dnx_dbal_mact_entry_add_example(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{

    SHR_FUNC_INIT_VARS(unit);

    SHR_EXIT();

  /** TBD test related to MDB should be added here */

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - dnx dbal unit test - use as dbal regression 
 * This function is the unitest dispacher for DBAL unitest 
 * scenarios. The dispacher support the following functionality:
 * DBAL UT - flags and information of DBAL Unit Test 
 * Usage: tr 201 [option], when option is one of: 
 *  [List] - Display the list of all availiable tests"
 *  [All] - run all tests"
 *  [Test_Id=<TestIndex>] - run a specific test" *
 *  [Table_Name=<DbalTableName>] - run the specific test on this
 *  specific Dbal Table"
 *
 */
int
test_dnx_dbal_unit_test(
    int unit,
    args_t * args,
    void *ptr)
{
    uint32 cmd_list = FALSE;
    uint32 cmd_all = FALSE;
    uint32 cmd_usage = FALSE;
    int nof_tests, table_index, test_index;
    uint8 test_id;
    int test_mode = DBAL_UT_RESTORE_TABLE;
    parse_table_t pt;
    uint32 default_value = 0xFFFFFFFF;
    int res, nof_arg;
    char *iterate_table = NULL;
    dbal_tables_e table_id;
    int bsl_dbal_severity, bsl_diag_severity;
    int pass_counter = 0;
    int num_of_run_tests = 0;

    PRT_INIT_VARS;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get parameters 
     */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Test_Id", PQ_DFL | PQ_INT8, (void *) INT_TO_PTR(default_value), &test_id, NULL);
    parse_table_add(&pt, "TI", PQ_DFL | PQ_INT8, (void *) INT_TO_PTR(default_value), &test_id, NULL);
    parse_table_add(&pt, "List", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_list, NULL);
    parse_table_add(&pt, "L", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_list, NULL);
    parse_table_add(&pt, "All", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_all, NULL);
    parse_table_add(&pt, "A", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_all, NULL);
    parse_table_add(&pt, "Table_Name", PQ_NO_EQ_OPT | PQ_STRING, (void *) "NONE", (void *) &iterate_table, NULL);
    parse_table_add(&pt, "TN", PQ_NO_EQ_OPT | PQ_STRING, (void *) "NONE", (void *) &iterate_table, NULL);
    parse_table_add(&pt, "Usage", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_usage, NULL);
    parse_table_add(&pt, "U", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_usage, NULL);
    parse_table_add(&pt, "?", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_usage, NULL);

    nof_arg = parse_arg_eq(args, &pt);
    nof_tests = sizeof(dbal_ut_list) / sizeof(dbal_ut_t);

    if (0 >= nof_arg)
    {
        cmd_all = TRUE;
    }
    if (cmd_usage == TRUE)
    {
        cli_out("\nDBAL UT - flags and information of DBAL Unit Test"
                "\nUsage: tr 201 [option], when option is one of:"
                "\n\t\t[List]                           - Display the list of all availiable tests"
                "\n\t\t[All]                            - run all tests"
                "\n\t\t[Test_Id=<TestIndex>]            - run a specific test"
                "\n\t\t\t[Table_Name=<DbalTableName>]   - run the specific test on this specific Dbal Table" "\n\n");
        return CMD_OK;
    }
    else if (cmd_list == TRUE)
    {
        /*
         * print the unitests list.
         */
        PRT_TITLE_SET("DBAL unitest list");
        PRT_COLUMN_ADD("Test Index");
        PRT_COLUMN_ADD("Test Name");
        PRT_COLUMN_ADD("Table 1");
        PRT_COLUMN_ADD("Table 2");
        PRT_COLUMN_ADD("Table 3");
        PRT_COLUMN_ADD("Description");
        for (test_index = 0; test_index < nof_tests; test_index++)
        {
            PRT_ROW_ADD(PRT_ROW_SEP_NONE);
            PRT_CELL_SET("%d", test_index);
            PRT_CELL_SET("%s", dbal_ut_list[test_index].test_name);
            if (dbal_ut_list[test_index].table_id[0] < DBAL_NOF_TABLES)
            {
                PRT_CELL_SET("%s", dbal_logical_table_to_string(unit, dbal_ut_list[test_index].table_id[0]));
            }
            else
            {
                PRT_CELL_SET("EMPTY");
            }
            if (dbal_ut_list[test_index].table_id[1] < DBAL_NOF_TABLES)
            {
                PRT_CELL_SET("%s", dbal_logical_table_to_string(unit, dbal_ut_list[test_index].table_id[1]));
            }
            else
            {
                PRT_CELL_SET("EMPTY");
            }
            if (dbal_ut_list[test_index].table_id[2] < DBAL_NOF_TABLES)
            {
                PRT_CELL_SET("%s", dbal_logical_table_to_string(unit, dbal_ut_list[test_index].table_id[2]));
            }
            else
            {
                PRT_CELL_SET("EMPTY");
            }

            PRT_CELL_SET("%s", dbal_ut_list[test_index].test_description);
        }
        PRT_COMMIT;
        LOG_CLI((BSL_META("\n")));
    }
    else if (cmd_all == TRUE)
    {
        /*
         * Run all test as a regression 
         */
        SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
        SHR_SET_SEVERITY_FOR_MODULE(bslSeverityError);

        SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
            SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityError);

        for (test_index = 0; test_index < nof_tests; test_index++)
        {
            for (table_index = 0; table_index < DBAL_MAX_NUM_UT_TABLES; table_index++)
            {
                if ((dbal_ut_list[test_index].table_id[table_index] < DBAL_NOF_TABLES) ||
                    (dbal_ut_list[test_index].table_id[table_index] == DBAL_EMPTY_UT_TABLE_ID))
                {
                    num_of_run_tests++;
                    res =
                        dbal_ut_list[test_index].test_func(unit, dbal_ut_list[test_index].table_id[table_index],
                                                           test_mode);
                    if (res == CMD_OK)
                    {
                        dbal_ut_list[test_index].is_passed[table_index] = TRUE;
                        pass_counter++;
                    }
                }
            }
        }

        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity)
            SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

        /*
         * Print run summery
         */
        if (pass_counter == num_of_run_tests)
        {
            PRT_TITLE_SET("DBAL REGRESSION PASSED: Results - PASS-%d, FAIL-%d",
                          pass_counter, num_of_run_tests - pass_counter);
        }
        else
        {
            PRT_TITLE_SET("DBAL REGRESSION FAILED: Results - PASS-%d, FAIL-%d",
                          pass_counter, num_of_run_tests - pass_counter);
        }

        PRT_COLUMN_ADD("Index");
        PRT_COLUMN_ADD("Test Name");
        PRT_COLUMN_ADD("Table Name");
        PRT_COLUMN_ADD("Result");
        PRT_COLUMN_ADD("Description");
        for (test_index = 0; test_index < nof_tests; test_index++)
        {
            for (table_index = 0; table_index < DBAL_MAX_NUM_UT_TABLES; table_index++)
            {
                if ((dbal_ut_list[test_index].table_id[table_index] < DBAL_NOF_TABLES) ||
                    (dbal_ut_list[test_index].table_id[table_index] == DBAL_EMPTY_UT_TABLE_ID))
                {
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SET("%d", test_index);
                    PRT_CELL_SET("%s", dbal_ut_list[test_index].test_name);
                    if (dbal_ut_list[test_index].table_id[table_index] < DBAL_NOF_TABLES)
                    {
                        PRT_CELL_SET("%s",
                                     dbal_logical_table_to_string(unit,
                                                                  dbal_ut_list[test_index].table_id[table_index]));
                    }
                    else
                    {
                        PRT_CELL_SET("EMPTY");
                    }
                    PRT_CELL_SET("%s", dbal_ut_list[test_index].is_passed[table_index] == TRUE ? "PASS" : "FAIL");
                    PRT_CELL_SET("%s", dbal_ut_list[test_index].test_description);
                }
            }
        }

        PRT_COMMIT;

        if (pass_counter < num_of_run_tests)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "\n");
        }
    }
    else if (test_id < nof_tests)
    {
        if (sal_strcmp(iterate_table, "NONE") != 0)
        {
            SHR_IF_ERR_EXIT_WITH_LOG(dbal_logical_table_string_to_id(unit, iterate_table, &table_id),
                                     "unknown table name %s %s %s\n", iterate_table, EMPTY, EMPTY);
            LOG_CLI((BSL_META("DBAL unitest: runing test_id %d, table_id %d:\n"), test_id, table_id));
            dbal_ut_list[test_id].test_func(unit, table_id, test_mode);
        }
        else
        {
            for (table_index = 0; table_index < DBAL_MAX_NUM_UT_TABLES; table_index++)
            {
                if ((dbal_ut_list[test_id].table_id[table_index] < DBAL_NOF_TABLES) ||
                    (dbal_ut_list[test_id].table_id[table_index] == DBAL_EMPTY_UT_TABLE_ID))
                {
                    LOG_CLI((BSL_META("DBAL unitest: runing test_id %d, table_id %d:\n"), test_id,
                             dbal_ut_list[test_id].table_id[table_index]));
                    res = dbal_ut_list[test_id].test_func(unit, dbal_ut_list[test_id].table_id[table_index], test_mode);
                    if (res != CMD_OK)
                    {
                        SHR_ERR_EXIT(_SHR_E_INTERNAL, "\n");
                    }
                }
            }
        }
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "unknown test id %d. \n", test_id);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * Run XML parsing validation test 
 * 1. immitate the init procedue (with validation flag) 
 * 2. compare to the expected values 
 */
cmd_result_t
test_dnx_dbal_xml_parsing_validation(
    int unit,
    dbal_tables_e table_id,
    dbal_ut_modes_t mode)
{
    return diag_dbal_test_xml_parsing(unit);
}

/**
 * \brief - dnx dbal logical test - per table logical test 
 * DBAL Logical Table Test - flags and information of DBAL 
 * Logical Table Test" 
 * Usage: tr 202 [option], 
 * when option is one of:" 
 * [All] - run all tests" 
 */
int
test_dnx_dbal_logical_test(
    int unit,
    args_t * args,
    void *ptr)
{
    int res = 0;
    char *input_strings[5] = { "ALL", "0" };
    int bsl_dbal_severity, bsl_diag_severity;
    uint32 cmd_all = FALSE;
    uint32 cmd_usage = FALSE;
    int nof_arg = 0;
    char *ltt_flags = NULL;
    parse_table_t pt;

    SHR_FUNC_INIT_VARS(unit);
    /*
     * Get parameters 
     */

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "All", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_all, NULL);
    parse_table_add(&pt, "A", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_all, NULL);
    parse_table_add(&pt, "Flags", PQ_DFL | PQ_STRING, &ltt_flags, &ltt_flags, NULL);
    parse_table_add(&pt, "F", PQ_DFL | PQ_STRING, &ltt_flags, &ltt_flags, NULL);
    parse_table_add(&pt, "Usage", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_usage, NULL);
    parse_table_add(&pt, "U", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_usage, NULL);
    parse_table_add(&pt, "?", PQ_DFL | PQ_NO_EQ_OPT | PQ_BOOL, 0, (void *) &cmd_usage, NULL);

    nof_arg = parse_arg_eq(args, &pt);

    if (0 >= nof_arg)
    {
        cmd_all = TRUE;
    }
    if (ltt_flags)
    {
        input_strings[1] = ltt_flags;
    }

    if (cmd_usage == TRUE)
    {
        cli_out("\nDBAL Logical Table Test - flags and information of DBAL Logical Table Test"
                "\nUsage: tr 202 [option], when option is one of:"
                "\n\t\t[All]                            - run all tests"
                "\n\t\t[Mode]                           - 0-basic, 1-advanced" "\n\n");
        return CMD_OK;
    }
    else if (cmd_all == TRUE)
    {
        /*
         * Run test as a regression 
         */
        SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);
        SHR_SET_SEVERITY_FOR_MODULE(bslSeverityWarn);

        SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bslSeverityWarn);

        res = dbal_logical_tables_test(unit, input_strings);

        SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, bsl_dbal_severity);
        SHR_SET_SEVERITY_FOR_MODULE(bsl_diag_severity);

        if (res == CMD_OK)
        {
            LOG_CLI((BSL_META("***********************************************************\n"
                              "******** DBAL LOGICAL TABLE TEST REGRESSION PASSED ********\n"
                              "***********************************************************\n")));
        }
        else
        {
            LOG_CLI((BSL_META("***********************************************************\n"
                              "******** DBAL LOGICAL TABLE TEST REGRESSION FAILED ********\n"
                              "***********************************************************\n\n")));
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "DBAL regression failure!\n");
        }
    }

exit:
    SHR_FUNC_EXIT;
}

#endif /* defined(BCM_DNX_SUPPORT) */
