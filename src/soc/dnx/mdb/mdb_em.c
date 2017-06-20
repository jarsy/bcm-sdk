/*
 * ! \file mdb_em.c $Id$ Contains all of the MDB Exact Match access functions provided to the DBAL.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/dnx/mdb.h>
#include <soc/dnx/dbal/dbal_structures.h>
#include <soc/dnx/dbal/dbal.h>
#include <shared/utilex/utilex_bitstream.h>
#include <shared/utilex/utilex_integer_arithmetic.h>
#include "../dbal/dbal_string_mgmt.h"
#include "mdb_internal.h"

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_MDBDNX
#include <shared/bsl.h>

typedef enum
{
    MDB_EM_CMD_DELETE,
    MDB_EM_CMD_INSERT,
    MDB_EM_CMD_REFRESH,
    MDB_EM_CMD_LEARN,
    MDB_EM_CMD_DEFRAG,
    MDB_EM_CMD_ACK,
    MDB_EM_CMD_MOVE,
    MDB_EM_CMD_LOOKUP,

    MDB_NOF_EM_CMD
} mdb_em_cmd_e;

/*
 * Due to the nature of hashing 100% HW utilization is unlikely
 */
#define MDB_EM_ENTRY_UTILIZATION   (0.95)

soc_mem_t mdb_em_dbal_to_mdb[DBAL_NOF_PHYSICAL_TABLES];
soc_mem_t mdb_em_dbal_to_mdb_tid_atr[DBAL_NOF_PHYSICAL_TABLES];
soc_reg_t mdb_em_dbal_to_mdb_entry_encoding[DBAL_NOF_PHYSICAL_TABLES];
mdb_em_entry_encoding_e mdb_em_app_id_to_entry_encoding[MDB_NOF_PHYSICAL_TABLES][MDB_NOF_APP_IDS];

shr_error_e
mdb_em_get_entry_size(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    uint32 * entry_size)
{
    mdb_physical_tables_e mdb_physical_table = mdb_direct_dbal_to_mdb[dbal_physical_table_id];
    uint32 entry_size_factor = (utilex_power_of_2(mdb_em_app_id_to_entry_encoding[mdb_physical_table][app_id]));

    SHR_FUNC_INIT_VARS(unit);

    if (app_id >= MDB_NOF_APP_IDS)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Error. APP ID %d is not valid.\n", app_id);
    }

    if (mdb_get_db_type(dbal_physical_table_id) != MDB_DB_EM)
    {
        SHR_ERR_EXIT(_SHR_E_BADID,
                     "Error. dbal_physical_table %d is not associated with an exact match memory.\n",
                     dbal_physical_table_id);
    }

    if (mdb_em_app_id_to_entry_encoding[mdb_physical_table][app_id] >= MDB_EM_ENTRY_ENCODING_EMPTY)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Error. Invalid physical table and app id combination, encoding: %d.\n",
                     mdb_em_app_id_to_entry_encoding[mdb_physical_table][app_id]);
    }

    if (entry_size_factor == 0)
    {
        *entry_size = 0;
    }
    else
    {
        *entry_size = mdb_direct_table_to_row_width[mdb_physical_table] * 1 / entry_size_factor;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_em_get_key_size(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    uint32 * key_size)
{

    SHR_FUNC_INIT_VARS(unit);

    if (app_id >= MDB_NOF_APP_IDS)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Error. APP ID %d is not valid.\n", app_id);
    }

    if (mdb_get_db_type(dbal_physical_table_id) != MDB_DB_EM)
    {
        SHR_ERR_EXIT(_SHR_E_BADID,
                     "Error. dbal_physical_table %d is not associated with an exact match memory.\n",
                     dbal_physical_table_id);
    }

    /*
     * If no logical table is associated with this table, return 0
     */
    if (mdb_em_dbal_to_mdb_tid_atr[dbal_physical_table_id] == INVALIDm)
    {
        *key_size = 0;
    }
    else
    {
        SHR_IF_ERR_EXIT(soc_mem_read
                        (unit, mdb_em_dbal_to_mdb_tid_atr[dbal_physical_table_id], SOC_BLOCK_ANY, app_id, key_size));
    }

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
mdb_em_entry_extract_payload(
    int unit,
    soc_mem_t mem,
    soc_reg_above_64_val_t data,
    uint32 payload_size,
    uint32 * payload)
{
    soc_field_info_t *field_info;
    uint32 entry_size;
    soc_reg_above_64_val_t entry;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Find the size of the entry for this memory
     */
    SOC_FIND_FIELD(ENTRYf, SOC_MEM_INFO(unit, mem).fields, SOC_MEM_INFO(unit, mem).nFields, field_info);
    entry_size = field_info->len;

    soc_mem_field_get(unit, mem, data, ENTRYf, entry);

    /*
     * ENTRY format is {Payload in MSB, Zeros, Key in LSB, zero_buffer}
     */
    SHR_BITCOPY_RANGE(payload, 0, entry, entry_size - payload_size, payload_size);

    SHR_FUNC_EXIT;
}

static shr_error_e
mdb_em_entry_prepare_data(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    uint32 command,
    soc_mem_t mem,
    uint32 * key,
    uint32 key_size,
    uint32 * payload,
    uint32 payload_size,
    soc_reg_above_64_val_t data)
{
    soc_field_info_t *field_info;
    uint32 entry_size;
    uint32 temp_total_size;
    uint32 zero_buffer = 0;
    soc_reg_above_64_val_t entry;
    mdb_physical_tables_e mdb_physical_table = mdb_direct_dbal_to_mdb[dbal_physical_table_id];

    SHR_FUNC_INIT_VARS(unit);

    if (mdb_physical_table == MDB_NOF_PHYSICAL_TABLES)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM,
                     "Error. DBAL physical table %d is not associated with an MDB table.\n", dbal_physical_table_id);
    }

    /*
     * Find the size of the entry for this memory
     */
    SOC_FIND_FIELD(ENTRYf, SOC_MEM_INFO(unit, mem).fields, SOC_MEM_INFO(unit, mem).nFields, field_info);
    entry_size = field_info->len;

    if (key_size + payload_size > entry_size)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Error. key_size(%d) + payload_size(%d) > entry_size(%d).\n", key_size, payload_size,
                     entry_size);
    }

    sal_memset(entry, 0x0, sizeof(entry));

    soc_mem_field_set(unit, mem, data, COMMANDf, &command);
    soc_mem_field_set(unit, mem, data, TIDf, &app_id);

    /*
     * Calculate the entry total size according to the entry_encoding
     */
    SHR_IF_ERR_EXIT(mdb_em_get_entry_size(unit, dbal_physical_table_id, app_id, &temp_total_size));

    /*
     * zero_buffer is the amount necessary to round the payload encoding to 240/120
     */
    zero_buffer = mdb_direct_table_to_row_width[mdb_physical_table] - temp_total_size;

    /*
     * Make sure zero_buffer fits in the entry
     */
    while (key_size + payload_size + zero_buffer > entry_size)
    {
        /*
         * SHR_ERR_EXIT(_SHR_E_PARAM, "Error. key_size(%d) + payload_size(%d) + zero_buffer(%d) > entry_size(%d).\n",
         * key_size, payload_size, zero_buffer, entry_size);
         */
        temp_total_size *= 2;
        zero_buffer = mdb_direct_table_to_row_width[mdb_physical_table] - temp_total_size;
    }

    /*
     * ENTRY format is {Payload in MSB, Zeros, Key in LSB, zero_buffer}
     */
    if (payload != NULL)
    {
        SHR_BITCOPY_RANGE(entry, entry_size - payload_size, payload, 0, payload_size);
    }
    SHR_BITCOPY_RANGE(entry, zero_buffer, key, 0, key_size);

    soc_mem_field_set(unit, mem, data, COMMANDf, &command);
    soc_mem_field_set(unit, mem, data, TIDf, &app_id);
    soc_mem_field_set(unit, mem, data, ENTRYf, entry);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_em_entry_add(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    soc_reg_above_64_val_t data;
    int blk = MDB_BLOCK(unit);
    int is_masked;
    bsl_severity_t severity;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(data, 0x0, sizeof(data));

    if (mdb_em_dbal_to_mdb[dbal_physical_table_id] == INVALIDm)
    {
        SHR_ERR_EXIT(_SHR_E_BADID,
                     "Error. dbal_physical_table %d is not associated with an exact match memory.\n",
                     dbal_physical_table_id);
    }

    SHR_IF_ERR_EXIT(mdb_direct_is_payload_masked(unit, entry, &is_masked));

    if (is_masked == 1)
    {
        /*
         * if the payload is partially masked we will need to read the entry, zero the valid payload bits and use OR to copy over only the valid bits
         */
        dbal_physical_entry_t get_entry;

        sal_memcpy(&get_entry, entry, sizeof(dbal_physical_entry_t));

        SHR_IF_ERR_EXIT(mdb_em_entry_get(unit, dbal_physical_table_id, app_id, &get_entry));

        /*
         * zero the valid payload bits
         */
        SHR_BITREMOVE_RANGE(get_entry.payload, entry->p_mask, 0, entry->payload_size, get_entry.payload);

        /*
         * Use OR to insert the masked payload
         */
        SHR_BITOR_RANGE(get_entry.payload, entry->payload, 0, entry->payload_size, get_entry.payload);

        SHR_IF_ERR_EXIT(mdb_em_entry_prepare_data
                        (unit, dbal_physical_table_id, app_id, (uint32) MDB_EM_CMD_INSERT,
                         mdb_em_dbal_to_mdb[dbal_physical_table_id], entry->key, entry->key_size, get_entry.payload,
                         entry->payload_size, data));
    }
    else
    {
        /*
         * If the payload is fully valid, we can simply copy the payload as is to the appropriate offset
         */
        SHR_IF_ERR_EXIT(mdb_em_entry_prepare_data
                        (unit, dbal_physical_table_id, app_id, (uint32) MDB_EM_CMD_INSERT,
                         mdb_em_dbal_to_mdb[dbal_physical_table_id], entry->key, entry->key_size, entry->payload,
                         entry->payload_size, data));
    }

    SHR_IF_ERR_EXIT(soc_mem_write(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id], blk, 0 /* index */ , data));

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        uint32 data_offset;
        uint32 print_index;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_em_entry_add: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U
                     (unit, "entry->key_size: %d. entry->payload_size: %d, physical_table: %s, app_id: %d.\n"),
                     entry->key_size, entry->payload_size, dbal_physical_table_to_string(unit, dbal_physical_table_id),
                     app_id));
        for (data_offset = 0; data_offset < BITS2WORDS(entry->key_size); data_offset++)
        {
            print_index = BITS2WORDS(entry->key_size) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->key[%d]: %08X.\n"), print_index, entry->key[print_index]));
        }
        for (data_offset = 0; data_offset < BITS2WORDS(entry->payload_size); data_offset++)
        {
            print_index = BITS2WORDS(entry->payload_size) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->payload[%d]: %08X.\n"), print_index, entry->payload[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U(unit, "Written to memory: %s.\n"),
                     SOC_MEM_NAME(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id])));
        for (data_offset = 0;
             data_offset < BYTES2WORDS(SOC_MEM_INFO(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id]).bytes);
             data_offset++)
        {
            print_index =
                BYTES2WORDS(SOC_MEM_INFO(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id]).bytes) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "Written data[%d]: %08X.\n"), print_index, data[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_em_entry_add: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_em_entry_get(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    soc_reg_above_64_val_t write_data, read_data;
    int blk = MDB_BLOCK(unit);
    bsl_severity_t severity;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(write_data, 0x0, sizeof(write_data));

    if (mdb_em_dbal_to_mdb[dbal_physical_table_id] == INVALIDm)
    {
        SHR_ERR_EXIT(_SHR_E_BADID,
                     "Error. dbal_physical_table %d is not associated with an exact match memory.\n",
                     dbal_physical_table_id);
    }

    /*
     * Issue a lookup command
     * The command is located at MDB_EM_CMD_OFFSET, the app id at MDB_EM_APP_OFFSET, entry key at MDB_EM_ENTRY_OFFSET, entry payload at MDB_EM_ENTRY_OFFSET + key_size
     */
    SHR_IF_ERR_EXIT(mdb_em_entry_prepare_data
                    (unit, dbal_physical_table_id, app_id, (uint32) MDB_EM_CMD_DELETE,
                     mdb_em_dbal_to_mdb[dbal_physical_table_id], entry->key, entry->key_size, NULL, entry->payload_size,
                     write_data));
    SHR_IF_ERR_EXIT(soc_mem_write(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id], blk, 0 /* index */ , write_data));

    /*
     * Read the lookup response
     */
    SHR_IF_ERR_EXIT(soc_mem_read(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id], blk, 0 /* index */ , read_data));

    SHR_IF_ERR_EXIT(mdb_em_entry_extract_payload
                    (unit, mdb_em_dbal_to_mdb[dbal_physical_table_id], read_data, entry->payload_size, entry->payload));

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        uint32 data_offset;
        uint32 print_index;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_em_entry_add: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U
                     (unit, "entry->key_size: %d. entry->payload_size: %d, physical_table: %s, app_id: %d.\n"),
                     entry->key_size, entry->payload_size, dbal_physical_table_to_string(unit, dbal_physical_table_id),
                     app_id));
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U(unit, "Read from memory: %s.\n"),
                     SOC_MEM_NAME(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id])));
        for (data_offset = 0;
             data_offset < BYTES2WORDS(SOC_MEM_INFO(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id]).bytes);
             data_offset++)
        {
            print_index =
                BYTES2WORDS(SOC_MEM_INFO(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id]).bytes) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "read_data[%d]: %08X.\n"), print_index, read_data[print_index]));
        }
        for (data_offset = 0; data_offset < BITS2WORDS(entry->key_size); data_offset++)
        {
            print_index = BITS2WORDS(entry->key_size) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->key[%d]: %08X.\n"), print_index, entry->key[print_index]));
        }
        for (data_offset = 0; data_offset < BITS2WORDS(entry->payload_size); data_offset++)
        {
            print_index = BITS2WORDS(entry->payload_size) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->payload[%d]: %08X.\n"), print_index, entry->payload[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_em_entry_add: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_em_entry_delete(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    soc_reg_above_64_val_t data;
    int blk = MDB_BLOCK(unit);
    bsl_severity_t severity;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(data, 0x0, sizeof(data));

    if (mdb_em_dbal_to_mdb[dbal_physical_table_id] == INVALIDm)
    {
        SHR_ERR_EXIT(_SHR_E_BADID,
                     "Error. dbal_physical_table %d is not associated with an exact match register.\n",
                     dbal_physical_table_id);
    }

    SHR_IF_ERR_EXIT(mdb_em_entry_prepare_data
                    (unit, dbal_physical_table_id, app_id, (uint32) MDB_EM_CMD_DELETE,
                     mdb_em_dbal_to_mdb[dbal_physical_table_id], entry->key, entry->key_size, entry->payload,
                     entry->payload_size, data));

    SHR_IF_ERR_EXIT(soc_mem_write(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id], blk, 0 /* index */ , data));

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        uint32 data_offset;
        uint32 print_index;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_em_entry_delete: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U
                     (unit, "entry->key_size: %d. entry->payload_size: %d, physical_table: %s, app_id: %d.\n"),
                     entry->key_size, entry->payload_size, dbal_physical_table_to_string(unit, dbal_physical_table_id),
                     app_id));
        for (data_offset = 0; data_offset < BITS2WORDS(entry->key_size); data_offset++)
        {
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->key[%d]: %08X.\n"), data_offset, entry->key[data_offset]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U(unit, "Deleted from memory: %s.\n"),
                     SOC_MEM_NAME(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id])));
        for (data_offset = 0;
             data_offset < (BYTES2WORDS(SOC_MEM_INFO(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id]).bytes));
             data_offset++)
        {
            print_index =
                BYTES2WORDS(SOC_MEM_INFO(unit, mdb_em_dbal_to_mdb[dbal_physical_table_id]).bytes) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "Written data[%d]: %08X.\n"), print_index, data[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_em_entry_delete: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_em_test(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    mdb_test_mode_e mode)
{
    dbal_physical_table_def_t *dbal_physical_table;
    dbal_physical_entry_t entry, entry_duplicate;
    int max_entries;
    int entry_counter;
    int uint32_counter;
    uint32 app_id;
    dbal_tables_e dbal_logical_table_id;
    CONST dbal_logical_table_t *dbal_logical_table = NULL;

    SHR_FUNC_INIT_VARS(unit);

    if (mdb_get_db_type(dbal_physical_table_id) != MDB_DB_EM)
    {
        SHR_ERR_EXIT(_SHR_E_BADID, "%s is not associated with an MDB EM table.\n",
                     dbal_physical_table_to_string(unit, dbal_physical_table_id));
    }

    /*
     * Find a DBAL logical table that uses this EM physical table and use its app id
     */
    for (dbal_logical_table_id = 0; dbal_logical_table_id < DBAL_NOF_TABLES; dbal_logical_table_id++)
    {
        SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, dbal_logical_table_id, &dbal_logical_table));
        if (dbal_logical_table->physical_db_id == dbal_physical_table_id)
        {
            break;
        }
    }

    if (dbal_logical_table_id == DBAL_NOF_TABLES)
    {
        LOG_INFO(BSL_LOG_MODULE,
                 (BSL_META_U
                  (unit,
                   "DBAL physical table %d is not associated with a DBAL logical table, skipping table test.\n"),
                  dbal_physical_table_id));
    }
    else
    {
        app_id = dbal_logical_table->app_id;

        sal_memset(&entry, 0x0, sizeof(entry));

        SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, dbal_physical_table_id, &dbal_physical_table));

        entry.payload_size = dbal_logical_table->max_payload_size;
        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(entry.p_mask, 0, entry.payload_size));
        entry.key_size = dbal_logical_table->key_size;

        /*
         * Assumes that each EM entry fits in a single 120bit row, need to adjust for different number of entries per
         * row
         */
        max_entries = dbal_physical_table->max_capacity;

        /*
         * Iterate on max number of entries, (should be multiplied by 2 since half the entries are deleted to verify
         * delete functionality, but CModel fails)
         */
        entry_counter = 0;
        while (entry_counter < max_entries)
        {
            /*
             * Fill the entry key with random content
             */
            for (uint32_counter = 0;
                 uint32_counter < ((entry.key_size + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS); uint32_counter++)
            {
                entry.key[uint32_counter] = sal_rand();
            }

            /*
             * Zero redundant bits
             */
            if ((((entry.key_size + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) !=
                entry.key_size)
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_reset_bit_range
                                (entry.key, entry.key_size,
                                 (((entry.key_size + SAL_UINT32_NOF_BITS -
                                    1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) - 1));
            }

            /*
             * Copy the entry to the duplicate entry, to be used later to verify the get function
             */
            sal_memcpy(&entry_duplicate, &entry, sizeof(entry));

            /*
             * Fill the entry payload with random content
             */
            for (uint32_counter = 0;
                 uint32_counter < ((entry.payload_size + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS);
                 uint32_counter++)
            {
                entry.payload[uint32_counter] = sal_rand();
            }

            /*
             * Zero redundant bits
             */
            if ((((entry.payload_size + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) !=
                entry.payload_size)
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_reset_bit_range
                                (entry.payload, entry.payload_size,
                                 (((entry.payload_size + SAL_UINT32_NOF_BITS -
                                    1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) - 1));
            }

            SHR_IF_ERR_EXIT(mdb_em_entry_add(unit, dbal_physical_table_id, app_id, &entry));

            SHR_IF_ERR_EXIT(mdb_em_entry_get(unit, dbal_physical_table_id, app_id, &entry_duplicate));

            /*
             * Xor between the read data and the written data, expect the output to be all zeros
             */
            SHR_IF_ERR_EXIT(utilex_bitstream_xor
                            (entry.payload, entry_duplicate.payload, DBAL_PHYSICAL_RES_SIZE_IN_WORDS));

            if (utilex_bitstream_have_one_in_range(entry.payload, 0 /* start_place */ , entry.payload_size - 1))
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_xor
                                (entry.payload, entry_duplicate.payload, DBAL_PHYSICAL_RES_SIZE_IN_WORDS));

                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Written data:\n 0x")));
                for (uint32_counter = ((entry.payload_size + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) - 1;
                     uint32_counter >= 0; uint32_counter--)
                {
                    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%08X"), entry.payload[uint32_counter]));
                }
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));

                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Read data:\n 0x")));
                for (uint32_counter = ((entry.payload_size + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) - 1;
                     uint32_counter >= 0; uint32_counter--)
                {
                    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%08X"), entry_duplicate.payload[uint32_counter]));
                }
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));

                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "entry_counter: %d\n"), entry_counter));

                SHR_ERR_EXIT(_SHR_E_FAIL, "Test failed, read data is not equal to written data.\n");
            }

            /*
             * Only delete half the entries, to also allow max capacity tests
             */
            if (entry_counter % 2 == 0)
            {
                /*
                 * Delete the entry
                 */
                SHR_IF_ERR_EXIT(mdb_em_entry_delete(unit, dbal_physical_table_id, app_id, &entry_duplicate));

                SHR_IF_ERR_EXIT(mdb_em_entry_get(unit, dbal_physical_table_id, app_id, &entry_duplicate));

                /*
                 * Verify the entry was deleted
                 */
                if (utilex_bitstream_have_one_in_range
                    (entry_duplicate.payload, 0 /* start_place */ , entry.payload_size - 1))
                {
                    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Read data:\n 0x")));
                    for (uint32_counter = ((entry.payload_size + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) - 1;
                         uint32_counter >= 0; uint32_counter--)
                    {
                        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%08X"), entry_duplicate.payload[uint32_counter]));
                    }
                    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));

                    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "entry_counter: %d\n"), entry_counter));

                    SHR_ERR_EXIT(_SHR_E_FAIL, "Test failed, read data is not zero after entry delete.\n");
                }
            }
            if (mode == MDB_TEST_FULL)
            {
                entry_counter++;
            }
            else
            {
                entry_counter += sal_rand() % MDB_TEST_BRIEF_FACTOR;
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}
