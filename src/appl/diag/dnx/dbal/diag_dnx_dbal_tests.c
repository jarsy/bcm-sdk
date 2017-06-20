/** \file diag_dnx_dbal_tests.c
 *
 * Main unitests for dbal applications.
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
#include <appl/diag/system.h>
#include <appl/diag/diag.h>
#include <bcm/l2.h>
#include <shared/utilex/utilex_bitstream.h>
#include "diag_dnx_dbal_internal.h"

/*************
 *  DEFINES  *
 *************/
/**
 * minimum number of entries that checked in logical table test
 */
#define DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE       (32)

/**
 * max size for entire key in words (32bits) 
 */
#define DIAG_DBAL_MAX_SIZE_FOR_TABLE_KEY_IN_WORDS (DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS*DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS)

/**
 * total max num of fields (result+key)
 */
#define DBAL_TABLE_MAX_NUM_OF_FIELDS              (DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS+DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS)

/*************
 * TYPEDEFS  *
 *************/
/**
 * all combination per table structure
 */
typedef struct
{
    /**
     * Number of fields in combination
     */
    int nof_fields;

    /**
     * Number of valid combinations
     */
    int nof_combinations;

    /**
     * The fields combinations
     */
    dbal_fields_e fields_id[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE][DBAL_TABLE_MAX_NUM_OF_FIELDS];

    /**
     * key indication per field per combination
     */
    uint8 fields_is_key[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE][DBAL_TABLE_MAX_NUM_OF_FIELDS];

    /**
     * field length [bits] per field per combination
     */
    int fields_bit_length[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE][DBAL_TABLE_MAX_NUM_OF_FIELDS];

    /**
     * field offset [bits] per field per combination
     */
    int fields_bit_offset[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE][DBAL_TABLE_MAX_NUM_OF_FIELDS];
} diag_dbal_ltt_fields_to_check;

/*************
 *  GLOBALS  *
 *************/

/**
 * fields combinations
 */
diag_dbal_ltt_fields_to_check fields_comb;

/**
 * fields values for random values test
 */
uint8 valid_key_value[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE];

/**
 * fields values for random values test
 */
uint32 key_field_val[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE]
    [DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

/**
 * fields get values for random values test
 */
uint32 result_field_val[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE]
    [DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

/**
 * fields get values for random values test
 */
uint32 result_field_val_get[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE]
    [DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

/**
 * iterated fields (all entries)
 */
uint32 iterator_parsed_fields[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

/**
 * found by iterator indication (for random entries values)
 */
uint8 iterator_found_indications[DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE];

/*************
 * FUNCTIONS *
 *************/
/**
* \brief 
* check if table is part of the full test (iterator and table clear) regression 
*****************************************************/
STATIC uint8
table_in_full_test_list(
    dbal_tables_e table_id)
{
    int ii, nof_tables;

    dbal_tables_e table_list[] = {
        /** Example tables all types */
        DBAL_TABLE_EXAMPLE_TABLE_FOR_REGISTER_USE,
        DBAL_TABLE_EXAMPLE_TABLE_FOR_EMPTY_KEY,
        DBAL_TABLE_EXAMPLE_TABLE_FOR_HL_WITH_SW_FIELD,
        DBAL_TABLE_EXAMPLE_TABLE_FOR_MUL_INSTANCES,
        DBAL_TABLE_SW_STATE_TABLE_EXAMPLE_1,
        DBAL_TABLE_SW_STATE_SUB_FIELD_EXAMPLE,
        DBAL_TABLE_SW_RESULT_TYPE_EXAMPLE,

        /** HardLogic */
        DBAL_TABLE_INGRESS_PORT,
        DBAL_TABLE_ESEM_ACCESS_CMD_PROP,
        DBAL_TABLE_SAME_INTERFACE_FILTER_TABLE,
        DBAL_TABLE_EGRESS_LLVP_CLASSIFICATION,
        DBAL_TABLE_SOURCE_ADDERSS_ETHERNET,
        DBAL_TABLE_IVEC,
        DBAL_TABLE_EVEC_ENCAP_STAGE,
        DBAL_TABLE_MY_MAC_DA_PREFIXES,
        DBAL_TABLE_CONTEXT_RES_ATTRIBUTES,
        DBAL_TABLE_ENABLERS_VECTORS,
        DBAL_TABLE_ROUTING_ENABLERS_PROFILE,
        DBAL_TABLE_SNIF_COUNTING_TABLE,
        DBAL_TABLE_EXAMPLE_FOR_HL_MULTIPLE_RESULT,
        DBAL_TABLE_EXAMPLE_FOR_HL_MULTIPLE_RESULT_WITH_REGISTER,

        /** MDB */

        /** SW */
        DBAL_TABLE_SW_STATE_GPORT_TO_FORWARDING_INFO,

        /** PEMLA */
        DBAL_TABLE_PEMLA_PARSER_TPID,
    };

    nof_tables = sizeof(table_list) / sizeof(dbal_tables_e);
    for (ii = 0; ii < nof_tables; ii++)
    {
        if (table_id == table_list[ii])
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
* \brief
* compare two fields array values, each field represented as array32. 
* returnes 0 if two fields value equal 
* otherwise -1 
* if compare_only_key==TRUE validate only key fields.
*  
*****************************************************/
STATIC int
compare_fields_values(
    int unit,
    CONST dbal_logical_table_t * table,
    uint32 field_val[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS],
    uint32 field_val_get[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS][DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS],
    int res_type_idx)
{
    int ii, jj;
    uint32 bit, bit_get;

    for (ii = 0; ii < table->multi_res_info[res_type_idx].nof_result_fields; ii++)
    {
        CONST dbal_table_field_info_t *res_info = table->multi_res_info[res_type_idx].results_info;

        for (jj = res_info[ii].offset_in_logical_field;
             jj < (res_info[ii].field_nof_bits + res_info[ii].offset_in_logical_field); jj++)
        {
            bit = (field_val[ii][jj / 32] >> (jj % 32)) & 0x1;
            bit_get = (field_val_get[ii][jj / 32] >> (jj % 32)) & 0x1;
            if (bit != bit_get)
            {
                LOG_DEBUG(BSL_LOG_MODULE,
                          (BSL_META_U(unit, "Comparison error: field=%s, bit=%d, expect=%d, get=%d\n"),
                           dbal_field_to_string(unit, res_info[ii].field_id), jj, bit, bit_get));
                return -1;
            }
        }
    }
    return 0;
}

/**
* \brief
* print out all table's field possibole combinations
*****************************************************/
static void
print_fields_combinations(
    int unit,
    diag_dbal_ltt_fields_to_check * field_combo)
{
    int ii, jj;
    for (ii = 0; ii < field_combo->nof_combinations; ii++)
    {
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%d)\t"), ii));
        for (jj = 0; jj < field_combo->nof_fields; jj++)
        {
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "field[%d]=%s "), jj,
                                      dbal_field_to_string(unit, field_combo->fields_id[ii][jj])));
        }
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
    }
}

/**
* \brief
* adding new list of fields to combinations structure. 
* this function will be called when a parent field is in the table, and will add its child fields 
*****************************************************/
static shr_error_e
diag_dbal_test_add_child_fields_to_combinations(
    int unit,
    diag_dbal_ltt_fields_to_check * field_combo,
    dbal_sub_field_info_t child_fields_info[DBAL_FIELD_MAX_CHILDS_PER_FIELD],
    int nof_chiled_fields,
    int field_offset_in_buffer,
    uint8 is_key)
{
    int ii, jj;
    int nof_combos;

    SHR_FUNC_INIT_VARS(unit);

    nof_combos = field_combo->nof_combinations;
    for (ii = 0; ii < nof_chiled_fields; ii++)
    {
        for (jj = 0; jj < nof_combos; jj++)
        {
            sal_memcpy(&field_combo->fields_id[ii * nof_combos + jj][0], &field_combo->fields_id[jj][0],
                       sizeof(int) * DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS);
            sal_memcpy(&field_combo->fields_is_key[ii * nof_combos + jj][0], &field_combo->fields_is_key[jj][0],
                       sizeof(int) * DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS);
            sal_memcpy(&field_combo->fields_bit_length[ii * nof_combos + jj][0], &field_combo->fields_bit_length[jj][0],
                       sizeof(int) * DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS);
            sal_memcpy(&field_combo->fields_bit_offset[ii * nof_combos + jj][0], &field_combo->fields_bit_offset[jj][0],
                       sizeof(int) * DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS);
        }
    }

    for (ii = 0; ii < nof_chiled_fields; ii++)
    {
        for (jj = 0; jj < nof_combos; jj++)
        {
            uint32 sub_field_length;

            SHR_IF_ERR_EXIT(dbal_fields_max_size_get(unit, child_fields_info[ii].sub_field_id, &sub_field_length));
            field_combo->fields_id[ii * nof_combos + jj][field_combo->nof_fields] = child_fields_info[ii].sub_field_id;
            field_combo->fields_is_key[ii * nof_combos + jj][field_combo->nof_fields] = is_key;
            field_combo->fields_bit_length[ii * nof_combos + jj][field_combo->nof_fields] = sub_field_length;
            field_combo->fields_bit_offset[ii * nof_combos + jj][field_combo->nof_fields] = field_offset_in_buffer;
        }
    }
    field_combo->nof_fields++;
    field_combo->nof_combinations *= nof_chiled_fields;

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
* Add field to the fields combination array.
***********************************************/
static shr_error_e
diag_dbal_test_add_field_to_combinations(
    int unit,
    dbal_tables_e table_id,
    CONST dbal_table_field_info_t * field_info,
    uint8 is_key)
{
    int ii;
    dbal_field_basic_info_t basic_field_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, field_info->field_id, &basic_field_info));
    if (basic_field_info.nof_child_fields != 0)
    {
        SHR_IF_ERR_EXIT(diag_dbal_test_add_child_fields_to_combinations(unit, &fields_comb,
                                                                        basic_field_info.sub_field_info,
                                                                        basic_field_info.nof_child_fields,
                                                                        field_info->offset_in_logical_field, is_key));
    }
    else
    {
        for (ii = 0; ii < fields_comb.nof_combinations; ii++)
        {
            fields_comb.fields_id[ii][fields_comb.nof_fields] = field_info->field_id;
            fields_comb.fields_is_key[ii][fields_comb.nof_fields] = is_key;
            fields_comb.fields_bit_length[ii][fields_comb.nof_fields] = field_info->field_nof_bits;
            fields_comb.fields_bit_offset[ii][fields_comb.nof_fields] = field_info->offset_in_logical_field;
        }
        fields_comb.nof_fields++;
    }
    if (fields_comb.nof_combinations > DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Exceeds the max num of fields combinations (%d)\n",
                     DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE);
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_fields_get_value_for_field(
    int unit,
    dbal_fields_e field_id,
    fields_values_types_e type_of_value,
    uint32 field_len,
    uint8 is_key,
    uint32 * field_value)
{
    dbal_field_basic_info_t field_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, field_id, &field_info));

    switch (field_info.type)
    {
        case DBAL_FIELD_TYPE_BOOL:
        case DBAL_FIELD_TYPE_INT32:
        case DBAL_FIELD_TYPE_IP:
        case DBAL_FIELD_TYPE_UINT32:
        case DBAL_FIELD_TYPE_ARRAY8:
        case DBAL_FIELD_TYPE_BITMAP:
        case DBAL_FIELD_TYPE_ARRAY32:
            if (type_of_value == MIN_FIELD_VAL)
            {
                DIAG_DBAL_MIN_FIELD_VAL(field_value, field_len, field_info.type);
            }
            else if (type_of_value == MAX_FIELD_VAL)
            {
                DIAG_DBAL_MAX_FIELD_VAL(field_value, field_len, field_info.type);
            }
            else
            {
                DIAG_DBAL_RANDOM_FIELD_VAL(field_value, field_len, is_key);
            }

            if (field_info.max_value != 0)
            {
                if (type_of_value == MAX_FIELD_VAL)
                {
                    field_value[0] = (field_value[0] > field_info.max_value) ? field_info.max_value : field_value[0];
                }
                else if (type_of_value == RANDOM_FIELD_VAL)
                {
                    field_value[0] %= (field_info.max_value + 1);
                }
            }

            if ((field_id == DBAL_FIELD_FEC) && ((field_value[0] & 0xc0000) == 0xc0000))
            {
                /*
                 * if fec field, and both bits 19th,20th are on - disable them
                 */
                field_value[0] &= 0x3ffff;
            }
            break;

        case DBAL_FIELD_TYPE_ENUM:
        {
            uint8 found_valid = FALSE;

            while (!found_valid)
            {
                field_value[0] = sal_rand() % field_info.nof_enum_values;
                if ((field_value[0] == 0) && (!is_key))
                {
                    field_value[0] = 1;
                }
                if (field_info.enum_val_info[field_value[0]].value < utilex_power_of_2(field_len))
                {
                    found_valid = TRUE;
                }
            }
            break;
        }

        case DBAL_FIELD_TYPE_NONE:
        case DBAL_NOF_FIELD_TYPES:
        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Unknown field type\n");
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
diag_dbal_iterator_count_entries(
    int unit,
    dbal_tables_e table_id,
    int *counter)
{
    dbal_iterator_info_t iterator_info;

    SHR_FUNC_INIT_VARS(unit);

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Counting entries\n")));

    SHR_IF_ERR_EXIT(dbal_table_iterator_init(unit, table_id, DBAL_ITER_MODE_GET_ALL_BUT_DEFAULT_ENTRIES,
                                             &iterator_info));
    while (!iterator_info.is_end)
    {
        SHR_IF_ERR_EXIT(dbal_table_iterator_get_next(unit, &iterator_info));
    }

    *counter = iterator_info.entries_counter;
    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Found %d entries\n"), *counter));
    SHR_IF_ERR_EXIT(dbal_table_iterator_destroy(unit, &iterator_info));

exit:
    SHR_FUNC_EXIT;
}

cmd_result_t
diag_dbal_test_logical_table(
    int unit,
    dbal_tables_e table_id,
    uint32 flags)
{
    int ii, jj;
    int key_size;
    int res_type_idx;
    int nof_entries;
    int keys_creation_counter;

    uint8 found_identical_keys = TRUE;
    uint8 keys_are_ready = FALSE;
    uint8 found_entry;

    CONST dbal_logical_table_t *table;
    uint32 entry_handle = DBAL_SW_NOF_ENTRY_HANDLES;
    dbal_iterator_info_t iterator_info;

    int invalid_entries_counter = 0;
    int mex_entries_in_test;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&key_field_val[0][0][0], 0, sizeof(key_field_val));
    sal_memset(&valid_key_value[0], TRUE, sizeof(valid_key_value));

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n*****logical_table_test*****\n "
                                         "Running tests for table %s, test flags 0x%x\n"),
                              dbal_logical_table_to_string(unit, table_id), flags));
    /*
     * init random seed value
     */
    sal_srand(sal_time_usecs());

    SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, table_id, &table));

    /*
     * in case of regression - run only with high maturity level
     */
    if ((flags & LTT_IS_REGRESSION) && (table->maturity_level != DBAL_MATURITY_HIGH))
    {
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Table maturity level is %d, test cannot run in regression\n"),
                                  table->maturity_level));
        return CMD_NOTIMPL;
    }

#ifdef CMODEL_SERVER_MODE
    if (table->access_method == DBAL_ACCESS_METHOD_HARD_LOGIC)
    {
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Hard logic tables cannot be ran on C-MODEL yet...\n")));
        return CMD_NOTIMPL;
    }
#else
    if (table->access_method == DBAL_ACCESS_METHOD_PHY_TABLE)
    {
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "MDB tables should be ran only on C-MODEL\n")));
        return CMD_NOTIMPL;
    }
#endif

    /*
     * Set flags according to dependencies flgs
     */
    if (flags & LTT_FULL_ITERATOR_TABLE_CLEAR_TEST)
    {
        flags |= LTT_RUN_ITERATOR;
        flags |= LTT_TABLE_CLEAR_END_OF_TEST;
        flags |= LTT_TABLE_CLEAR_STT_OF_TEST;
        if (flags & LTT_IS_REGRESSION)
        {
            if (!table_in_full_test_list(table_id))
            {
                return CMD_NOTIMPL;
            }
        }
    }

    if (flags & LTT_RUN_ITERATOR)
    {
        flags |= LTT_TABLE_CLEAR_STT_OF_TEST;
    }

    if (flags & LTT_RUN__TEST_WITH_3_ENTRIES)
    {
        mex_entries_in_test = 3;
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Running test with up to 3 entries\n")));
    }
    else
    {
        mex_entries_in_test = DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE;
    }

    /*
     * set the valid keys for the table 
     * run this part until we have different 32 keys 
     */
    keys_creation_counter = 0;
    while (!keys_are_ready)
    {
        LOG_DEBUG(BSL_LOG_MODULE, (BSL_META_U(unit, "create key buffer: iteration:%d\n"), ++keys_creation_counter));
        key_size = 0;
        for (ii = 0; ii < table->nof_key_fields; ii++)
        {
            key_size += table->keys_info[ii].field_nof_bits;
        }

        if (key_size < 8)
        {
            /*
             * if key is less than 8 bits, fill all table, with all valid 
             * keys values (or up to 32 different values)
             */
            int total_bits_before_field = 0;
            nof_entries = UTILEX_MIN(mex_entries_in_test, (1 << key_size));
            for (jj = 0; jj < table->nof_key_fields; jj++)
            {
                uint32 cur_field_val = 0;
                int field_nof_values = utilex_power_of_2(table->keys_info[jj].field_nof_bits);
                dbal_field_basic_info_t field_info;

                SHR_IF_ERR_EXIT(dbal_fields_field_info_get(unit, table->keys_info[jj].field_id, &field_info));

                if (field_nof_values <= 0)
                {
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "Something went wrong. field_nof_values cannot be zero\n");
                }

                for (ii = 0; ii < nof_entries; ii++)
                {
                    cur_field_val = ((ii >> total_bits_before_field) % field_nof_values);
                    key_field_val[ii][jj][0] = cur_field_val;
                    if ((cur_field_val > field_info.max_value) ||
                        ((field_info.type == DBAL_FIELD_TYPE_ENUM) &&
                         ((cur_field_val >= field_info.nof_enum_values) ||
                          (field_info.enum_val_info[cur_field_val].value >= field_nof_values))))
                    {
                        /*
                         * When field is enum, the entry is invalid if (at least one): 
                         * 1. the field value is equal or greater then num of enum vals 
                         * 2. the hw value is equal or greater then the field max value by range 
                         * If field has max_value indication, its value cannot be larger than it 
                         */
                        valid_key_value[ii] = FALSE;
                        invalid_entries_counter++;
                    }
                }
                total_bits_before_field += table->keys_info[jj].field_nof_bits;
            }
        }
        else
        {
            /*
             * for 6bit and larger key size, check 32 entris. 
             * min entry, max entry, 30 random entries
             */
            nof_entries = mex_entries_in_test;
            for (ii = 0; ii < nof_entries; ii++)
            {
                for (jj = 0; jj < table->nof_key_fields; jj++)
                {
                    if (ii == 0)
                    {
                        SHR_IF_ERR_EXIT(dbal_fields_get_value_for_field(unit, table->keys_info[jj].field_id,
                                                                        MIN_FIELD_VAL,
                                                                        table->keys_info[jj].field_nof_bits +
                                                                        table->keys_info[jj].offset_in_logical_field,
                                                                        TRUE, key_field_val[ii][jj]));
                    }
                    else if (ii == (nof_entries - 1))
                    {

                        SHR_IF_ERR_EXIT(dbal_fields_get_value_for_field(unit, table->keys_info[jj].field_id,
                                                                        MAX_FIELD_VAL,
                                                                        table->keys_info[jj].field_nof_bits +
                                                                        table->keys_info[jj].offset_in_logical_field,
                                                                        TRUE, key_field_val[ii][jj]));
                    }
                    else
                    {
                        SHR_IF_ERR_EXIT(dbal_fields_get_value_for_field(unit, table->keys_info[jj].field_id,
                                                                        RANDOM_FIELD_VAL,
                                                                        table->keys_info[jj].field_nof_bits +
                                                                        table->keys_info[jj].offset_in_logical_field,
                                                                        TRUE, key_field_val[ii][jj]));
                    }
                    if (table->access_method == DBAL_ACCESS_METHOD_PHY_TABLE)
                    {
                        /*
                         * In case of direct MDB table, the entries are limied to max 
                         * capacity 
                         */
                        dbal_physical_table_def_t *physical_table;
                        SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, table->physical_db_id, &physical_table));
                        if (table->table_type == DBAL_TABLE_TYPE_DIRECT)
                        {
                            /*
                             * There is an assumption here that for direct table, key is lower than 32b
                             */
                            int basic_payload_len = 0;
                            int payload_size = 0;
                            int index_factor = 0;
                            key_field_val[ii][jj][0] = key_field_val[ii][jj][0] % physical_table->max_capacity;

                            /*
                             * This API should set in basic_payload_len the length in bits
                             */
                            SHR_IF_ERR_EXIT(mdb_direct_table_get_basic_size(unit, table->physical_db_id,
                                                                            &basic_payload_len));
                            /*
                             * Compute the max payload length of the table
                             */
                            payload_size = table->max_payload_size;

                            /*
                             * translate key to valid value, according to the payload size
                             * and basic payload size ratio
                             */
                            index_factor = (payload_size + basic_payload_len - 1) / (basic_payload_len);
                            key_field_val[ii][jj][0] /= index_factor;
                            key_field_val[ii][jj][0] *= index_factor;
                        }
                    }
                }
            }
        }

        for (ii = 0; ii < nof_entries; ii++)
        {
            /*
             * validate that there are no entries duplication
             */
            found_identical_keys = FALSE;
            for (jj = ii + 1; jj < nof_entries; jj++)
            {
                if (sal_memcmp(key_field_val[ii], key_field_val[jj],
                               (sizeof(key_field_val) / DIAG_DBAL_MIN_NOF_ENTRIES_PER_TABLE)) == 0)
                {
                    found_identical_keys = TRUE;
                    break;
                }
            }
            if (found_identical_keys)
            {
                int bsl_diag_severity;
                SHR_GET_SEVERITY_FOR_MODULE(bsl_diag_severity);

                if (bsl_diag_severity >= bslSeverityDebug)
                {
                    int aa, bb;
                    for (aa = 0; aa < nof_entries; aa++)
                    {
                        LOG_CLI((BSL_META_U(unit, "Entry %02d:"), aa));
                        for (bb = 0; bb < table->nof_key_fields; bb++)
                        {
                            LOG_CLI((BSL_META_U(unit, " 0x%08x"), key_field_val[aa][bb][0]));
                        }
                        LOG_CLI((BSL_META_U(unit, "\n")));
                    }
                    LOG_CLI((BSL_META_U(unit, "\n")));
                }

                break;
            }
        }
        if (ii == nof_entries)
        {
            keys_are_ready = TRUE;
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Keys are ready, %d different key buffers\n"), nof_entries));
        }
    }

    if (flags & LTT_TABLE_CLEAR_STT_OF_TEST)
    {
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Clear table before running the test\n")));
        SHR_IF_ERR_EXIT(dbal_table_clear(unit, table_id));
    }

    for (res_type_idx = 0; res_type_idx < table->nof_result_types; res_type_idx++)
    {
        int num_of_value_fields = table->multi_res_info[res_type_idx].nof_result_fields;
        CONST dbal_table_field_info_t *results_info = table->multi_res_info[res_type_idx].results_info;

        sal_memset(&fields_comb, 0, sizeof(diag_dbal_ltt_fields_to_check));
        sal_memset(&result_field_val[0][0][0], 0, sizeof(result_field_val));
        sal_memset(&result_field_val_get[0][0][0], 0, sizeof(result_field_val_get));
        sal_memset(&iterator_parsed_fields[0][0], 0, sizeof(iterator_parsed_fields));
        sal_memset(&iterator_found_indications[0], 0, sizeof(iterator_found_indications));

        fields_comb.nof_fields = 0;
        fields_comb.nof_combinations = 1;

        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Start test for result type %d\n"), res_type_idx));

        /*
         * arrange all fields possible combinations 
         * The combinations are derived from child fields 
         * first key fields, than result fields 
         */
        for (jj = 0; jj < table->nof_key_fields; jj++)
        {
            SHR_IF_ERR_EXIT(diag_dbal_test_add_field_to_combinations(unit, table_id, &table->keys_info[jj], TRUE));
        }
        for (jj = 0; jj < num_of_value_fields; jj++)
        {
            SHR_IF_ERR_EXIT(diag_dbal_test_add_field_to_combinations(unit, table_id, &results_info[jj], FALSE));
        }

        /*
         * If there is more than one combination - print out all combinations
         */
        if (fields_comb.nof_combinations > 1)
        {
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Checking %d fields combinations:\n"),
                                      fields_comb.nof_combinations));
            print_fields_combinations(unit, &fields_comb);
        }

        /*
         * add all entries to the table. 
         * keys- according to prior selection 
         * values - randomaly, except maximun result value with maximum key value 
         */
        for (ii = 0; ii < nof_entries; ii++)
        {
            int comb_index = ii % fields_comb.nof_combinations;

            if (!valid_key_value[ii])
            {
                continue;
            }
            SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle));

            for (jj = 0; jj < fields_comb.nof_fields; jj++)
            {
                if (fields_comb.fields_is_key[comb_index][jj])
                {
                    dbal_entry_key_field_arr32_set(unit, entry_handle, fields_comb.fields_id[comb_index][jj],
                                                   INST_SINGLE, key_field_val[ii][jj]);
                }
                else
                {
                    uint8 max_value = FALSE;
                    int res_idx = jj - table->nof_key_fields;
                    if (ii == (nof_entries - 1))
                    {
                        max_value = TRUE;
                    }
                    SHR_IF_ERR_EXIT(dbal_fields_get_value_for_field(unit, fields_comb.fields_id[comb_index][jj],
                                                                    max_value ? MAX_FIELD_VAL : RANDOM_FIELD_VAL,
                                                                    fields_comb.fields_bit_length[comb_index][jj] +
                                                                    fields_comb.fields_bit_offset[comb_index][jj],
                                                                    FALSE, result_field_val[ii][res_idx]));
                    /*
                     * If it is a multi results type table, set the RESULT_TYPE 
                     * field to the result type indx 
                     */
                    if ((table->nof_result_types > 1) &&
                        (fields_comb.fields_id[comb_index][jj] == DBAL_FIELD_RESULT_TYPE))
                    {
                        result_field_val[ii][res_idx][0] = res_type_idx;
                    }
                    dbal_entry_value_field_arr32_set(unit, entry_handle, fields_comb.fields_id[comb_index][jj],
                                                     INST_SINGLE, result_field_val[ii][res_idx]);
                }
            }
            SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle, DBAL_COMMIT_NORMAL));
        }

        /*
         * get all entries, using the inserted keys 
         * compare to the corresponding result values 
         */
        for (ii = 0; ii < nof_entries; ii++)
        {
            int comb_index = ii % fields_comb.nof_combinations;

            if (!valid_key_value[ii])
            {
                continue;
            }

            SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle));

            for (jj = 0; jj < fields_comb.nof_fields; jj++)
            {
                int res_idx = jj - table->nof_key_fields;

                if (fields_comb.fields_is_key[comb_index][jj])
                {
                    dbal_entry_key_field_arr32_set(unit, entry_handle, fields_comb.fields_id[comb_index][jj],
                                                   INST_SINGLE, key_field_val[ii][jj]);
                }
                else
                {
                    /*
                     * If the table is a single result type table: use regular procedure
                     * If table has multiple result types, use GET ALL FIELDS flags and parse fields 
                     */
                    if (table->nof_result_types == 1)
                    {
                        dbal_entry_value_field_arr32_get(unit, entry_handle, fields_comb.fields_id[comb_index][jj],
                                                         INST_SINGLE, result_field_val_get[ii][res_idx]);
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            if (table->nof_result_types == 1)
            {
                SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle, DBAL_COMMIT_NORMAL));
            }
            else
            {
                SHR_IF_ERR_EXIT(dbal_entry_get(unit, entry_handle, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE));

                for (jj = 0; jj < fields_comb.nof_fields; jj++)
                {
                    int res_idx = jj - table->nof_key_fields;

                    if (fields_comb.fields_is_key[comb_index][jj])
                    {
                        continue;
                    }
                    else
                    {
                        SHR_IF_ERR_EXIT(dbal_entry_handle_value_field32_get
                                        (unit, entry_handle, fields_comb.fields_id[comb_index][jj], INST_SINGLE,
                                         result_field_val_get[ii][res_idx]));
                    }
                }
                dbal_entry_handle_release(unit, entry_handle);
            }

            /*
             * compare results
             */
            if (compare_fields_values(unit, table, result_field_val[ii], result_field_val_get[ii], res_type_idx) != 0)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Entry %d was not found\n", ii);
            }
        }
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Finish with min/max/random values basic test\n")));

        /*
         * In case of LTT_RUN_ITERATOR is set, continue to iterator test  
         */
        if (flags & LTT_RUN_ITERATOR)
        {
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Starting iterator test\n")));

            SHR_IF_ERR_EXIT(dbal_table_iterator_init(unit, table_id, DBAL_ITER_MODE_GET_ALL_BUT_DEFAULT_ENTRIES,
                                                     &iterator_info));
            /*
             * iterator main loop 
             * get next entry and find the match entry in result buffer 
             */
            SHR_IF_ERR_EXIT(dbal_table_iterator_get_next(unit, &iterator_info));
            while (!iterator_info.is_end)
            {
                found_entry = FALSE;
                for (ii = 0; ii < nof_entries; ii++)
                {
                    int comb_index = ii % fields_comb.nof_combinations;

                    /** if there are sub-fields here, parse the result*/
                    dbal_fields_e sub_field_id;
                    uint32 sub_field_value[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];
                    for (jj = 0; jj < fields_comb.nof_fields; jj++)
                    {
                        if (fields_comb.fields_is_key[comb_index][jj])
                        {
                            continue;
                        }
                        else
                        {
                            int res_idx = jj - table->nof_key_fields;
                            sub_field_id = DBAL_FIELD_EMPTY;
                            sal_memset(sub_field_value, 0, sizeof(sub_field_value));
                            SHR_IF_ERR_EXIT(dbal_fields_sub_field_info_get(unit,
                                                                           results_info[res_idx].field_id,
                                                                           iterator_info.
                                                                           results_info[res_idx].field_val[0],
                                                                           &sub_field_id, sub_field_value));
                            if (sub_field_id != DBAL_FIELD_EMPTY)
                            {
                                sal_memcpy(iterator_parsed_fields[res_idx], sub_field_value,
                                           DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS * sizeof(uint32));
                            }
                            else
                            {
                                sal_memcpy(iterator_parsed_fields[res_idx],
                                           iterator_info.results_info[res_idx].field_val,
                                           DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS * sizeof(uint32));
                            }
                        }
                    }

                    if (compare_fields_values(unit, table, result_field_val[ii], iterator_parsed_fields,
                                              res_type_idx) == 0)
                    {
                        if (iterator_found_indications[ii] == TRUE)
                        {
                            continue;
                        }
                        LOG_DEBUG(BSL_LOG_MODULE, (BSL_META_U(unit, "Iterate #%d match to entry #%d\n"),
                                                   iterator_info.entries_counter, ii));
                        iterator_found_indications[ii] = TRUE;
                        found_entry = TRUE;
                        break;
                    }
                }

                if (found_entry == FALSE)
                {
                    SHR_IF_ERR_EXIT(dbal_table_iterator_destroy(unit, &iterator_info));
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "Iterated entry #%d was not found in entries DB of the test\n",
                                 iterator_info.entries_counter);
                }
                SHR_IF_ERR_EXIT(dbal_table_iterator_get_next(unit, &iterator_info));
            }

            if (iterator_info.entries_counter != (nof_entries - invalid_entries_counter))
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Wrong counting of entries in table."
                             "iterator count=%d, num of entries=%d, invalid_entries_counter=%d\n",
                             iterator_info.entries_counter, nof_entries, invalid_entries_counter);
            }
            else
            {
                LOG_INFO(BSL_LOG_MODULE,
                         (BSL_META_U(unit, "Iterator found %d entries\n"), iterator_info.entries_counter));
            }
            SHR_IF_ERR_EXIT(dbal_table_iterator_destroy(unit, &iterator_info));

            /*
             * check that all entries are found as expected 
             */
            for (ii = 0; ii < nof_entries; ii++)
            {
                if (!valid_key_value[ii])
                {
                    continue;
                }

                if (iterator_found_indications[ii] == FALSE)
                {
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "Entry #%d, was not found by iterator\n", ii);
                }
            }
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Done with iterator test\n")));
        }

        if (flags & LTT_DO_NOT_REMOVE_ENTRIES)
        {
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Keep entries in table, skip the delete part\n")));
        }
        else
        {
            /*
             * delete all entries 
             */
            if (flags & LTT_TABLE_CLEAR_END_OF_TEST)
            {
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Clear table\n")));
                SHR_IF_ERR_EXIT(dbal_table_clear(unit, table_id));
            }
            else
            {
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Delete entries\n")));

                for (ii = 0; ii < nof_entries; ii++)
                {
                    if (!valid_key_value[ii])
                    {
                        continue;
                    }
                    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, table_id, &entry_handle));

                    for (jj = 0; jj < table->nof_key_fields; jj++)
                    {
                        dbal_entry_key_field_arr32_set(unit, entry_handle, table->keys_info[jj].field_id,
                                                       INST_SINGLE, key_field_val[ii][jj]);
                    }
                    SHR_IF_ERR_EXIT(dbal_entry_clear(unit, entry_handle, DBAL_COMMIT_NORMAL));
                }
            }
        }
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Logical test is done for result type %d\n"), res_type_idx));
    }
    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Logical test is done for table\n")));

exit:
    if (SHR_FUNC_ERR())
    {
        dbal_entry_handle_release(unit, entry_handle);

        LOG_INFO(BSL_LOG_MODULE,
                 (BSL_META_U(unit, "Error in LTT. table %s\n"), dbal_logical_table_to_string(unit, table_id)));

    }
    SHR_FUNC_EXIT;
}
