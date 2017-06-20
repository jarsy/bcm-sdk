/*
 * $Id: dpp_dbal.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DBALACCESSDNX
#include <shared/bsl.h>

#include "dbal_internal.h"
#include <soc/dnx/dbal/dbal.h>
#include <soc/mem.h>
#include <soc/mcm/memregs.h>
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_bitstream.h>

/** ****************************************************
* \brief
* The function print (verbose level) an entry in SW table 
* (key+payload). 
* used for both hash and direct tables
*****************************************************/
static shr_error_e
dbal_sw_table_print_entry_from_sw_state(
    int unit,
    int entry_index,
    uint8 * payload,
    int res_length,
    uint8 is_set)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);
    LOG_CLI((BSL_META_U(unit, "DBAL SW Access. Entry %s SW state\n"), is_set ? "set in" : "get from"));
    LOG_CLI((BSL_META_U(unit, "Entry index : 0x%x08\n"), entry_index));
    LOG_CLI((BSL_META_U(unit, "Payload (%d bytes) : 0x"), res_length));
    for (ii = res_length - 1; ii >= 0; ii--)
    {
        LOG_CLI((BSL_META("%02x"), payload[ii]));
    }
    LOG_CLI((BSL_META_U(unit, "\n")));
    DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function set an entry represented ny entry_handle to a
* specific entry index in sw state table
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_index - \n
*    The index to insert to entry in sw state
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*  \par INDIRECT OUTPUT:
*  SW state dbal tables, an entry will
*    be added to the relevant table
*****************************************************/
static shr_error_e
dbal_sw_table_entry_set_in_index(
    int unit,
    int entry_index,
    dbal_entry_handle_t * entry_handle)
{
    int ii;
    dbal_table_field_info_t *table_field_info;
    int current_result_type = entry_handle->cur_res_type;
    int bsl_severity;

    SHR_FUNC_INIT_VARS(unit);

    SHR_GET_SEVERITY_FOR_MODULE(bsl_severity);

    for (ii = 0; ii < entry_handle->table->multi_res_info[current_result_type].nof_result_fields; ii++)
    {
        uint32 field_value[1];
        uint8 payload[DBAL_PHYSICAL_RES_SIZE_IN_WORDS];

        table_field_info = &(entry_handle->table->multi_res_info[current_result_type].results_info[ii]);

        if (!table_field_info->is_sw_field)
        {
            continue;
        }

        if (entry_handle->value_field_ids[ii] == DBAL_FIELD_EMPTY)
        {
     /** field not requested */
            continue;
        }

        SHR_IF_ERR_EXIT(utilex_bitstream_get_field
                        (entry_handle->phy_entry.payload, table_field_info->bits_offset_in_buffer,
                         table_field_info->field_nof_bits, field_value));
        SHR_IF_ERR_EXIT(utilex_U32_to_U8(field_value, BITS2BYTES(table_field_info->field_nof_bits), payload));

        SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.entries.entry_buffer.memwrite(unit,
                                                                           entry_handle->table_id,
                                                                           entry_index,
                                                                           payload,
                                                                           table_field_info->bytes_offset_in_sw_buffer,
                                                                           BITS2BYTES
                                                                           (table_field_info->field_nof_bits)));
    }

    if (bsl_severity >= bslSeverityInfo)
    {
        uint8 payload[DBAL_PHYSICAL_RES_SIZE_IN_WORDS];
        int res_length_bytes = entry_handle->table->sw_payload_length_bytes;
        SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.entries.entry_buffer.memread(unit, entry_handle->table_id, entry_index,
                                                                          payload, 0, res_length_bytes));
        SHR_IF_ERR_EXIT(dbal_sw_table_print_entry_from_sw_state(unit, entry_index, payload, res_length_bytes, TRUE));
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function return the information from specific entry index
* in sw state table to the dbal entry handle
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_index - \n
*    The index to get the information from in sw state
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*  \par INDIRECT OUTPUT:
*    \param [in] entry_handle - \n
*      will be updated with the entry information
*****************************************************/
static shr_error_e
dbal_sw_table_entry_get_in_index(
    int unit,
    int entry_index,
    dbal_entry_handle_t * entry_handle,
    int *res_type_get)
{
    int ii;
    int res_length_bytes;
    uint8 payload[DBAL_PHYSICAL_RES_SIZE_IN_WORDS];
    dbal_table_field_info_t *table_field_info;
    uint32 field_value;
    int result_type_index = 0;
    int bsl_severity;
    SHR_FUNC_INIT_VARS(unit);

    SHR_GET_SEVERITY_FOR_MODULE(bsl_severity);

    res_length_bytes = entry_handle->table->sw_payload_length_bytes;

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.entries.entry_buffer.memread(unit,
                                                                      entry_handle->table_id,
                                                                      entry_index, payload, 0, res_length_bytes));
    if (bsl_severity >= bslSeverityInfo)
    {
        SHR_IF_ERR_EXIT(dbal_sw_table_print_entry_from_sw_state(unit, entry_index, payload, res_length_bytes, FALSE));
    }

    if ((entry_handle->table->nof_result_types > 1) &&
        (entry_handle->table->multi_res_info[0].results_info[0].is_sw_field))
    {
        /*
         * if table has multiple results. read the RESULT_TYPE field and build the 
         * buffer accordingly. otherwise, fill the buffer according to 
         * the table results fields 
         */
        int result_type_size;
        int result_type_offset_in_sw_buffer;
        uint32 result_type_mask;

        result_type_size = entry_handle->table->multi_res_info[0].results_info[0].field_nof_bits;
        result_type_offset_in_sw_buffer =
            entry_handle->table->multi_res_info[0].results_info[0].bytes_offset_in_sw_buffer;
        result_type_mask = ((1 << result_type_size) - 1);

        SHR_IF_ERR_EXIT(utilex_U8_to_U32(&payload[result_type_offset_in_sw_buffer],
                                         BITS2BYTES(result_type_size), &field_value));
        result_type_index = field_value & result_type_mask;
    }
    else
    {
        result_type_index = entry_handle->cur_res_type;
    }

    if (res_type_get != NULL)
    {
        result_type_index = *res_type_get;
    }

    for (ii = 0; ii < entry_handle->table->multi_res_info[result_type_index].nof_result_fields; ii++)
    {
        table_field_info = &(entry_handle->table->multi_res_info[result_type_index].results_info[ii]);
        if (!entry_handle->get_all_fields)
        {
            /*
             * only if single result type per table - check which fields were requested 
             */
            if ((entry_handle->value_field_ids[ii] == DBAL_FIELD_EMPTY) || (!table_field_info->is_sw_field))
            {
                continue;
            }
        }

        SHR_IF_ERR_EXIT(utilex_U8_to_U32(&payload[table_field_info->bytes_offset_in_sw_buffer],
                                         BITS2BYTES(table_field_info->field_nof_bits), &field_value));

        SHR_IF_ERR_EXIT(utilex_bitstream_set_field(entry_handle->phy_entry.payload,
                                                   table_field_info->bits_offset_in_buffer,
                                                   table_field_info->field_nof_bits, field_value));
        entry_handle->phy_entry.payload_size += table_field_info->field_nof_bits;
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is used to add entry to a dbal sw state table of 
* type direct 
* It calculates the entry index in the table according the key 
* and calls dbal_sw_table_entry_set_in_index 
*  
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_direct_entry_set(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int key_length, key_mask, entry_index;

    SHR_FUNC_INIT_VARS(unit);

    key_length = entry_handle->phy_entry.key_size;
    key_mask = ((1 << key_length) - 1);

    entry_index = key_mask & entry_handle->phy_entry.key[0];
    if (entry_handle->core_id > DBAL_CORE_ANY)
    {
        entry_index |= (entry_handle->core_id << key_length);
    }

    SHR_IF_ERR_EXIT(dbal_sw_table_entry_set_in_index(unit, entry_index, entry_handle));

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is used to get and entry to a dbal sw state table
* of type direct 
* It calculates the entry index in the table according the key 
* and calls dbal_sw_table_entry_get_in_index 
*  
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_direct_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    int * res_type_get)
{
    int key_length, key_mask, entry_index;

    SHR_FUNC_INIT_VARS(unit);

    key_length = entry_handle->phy_entry.key_size;
    key_mask = ((1 << key_length) - 1);

    entry_index = key_mask & entry_handle->phy_entry.key[0];
    if (entry_handle->core_id > DBAL_CORE_ANY)
    {
        entry_index |= (entry_handle->core_id << key_length);
    }

    SHR_IF_ERR_EXIT(dbal_sw_table_entry_get_in_index(unit, entry_index, entry_handle, res_type_get));

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is the generic API to set a default entry (delete
* an entry) in dbal SW state tables. 
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_direct_entry_clear(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int key_length, key_mask, entry_index;
    uint8 default_payload[DBAL_PHYSICAL_RES_SIZE_IN_WORDS] = { 0 };
    int bsl_severity;

    SHR_FUNC_INIT_VARS(unit);

    key_length = entry_handle->phy_entry.key_size;
    key_mask = ((1 << key_length) - 1);

    entry_index = key_mask & entry_handle->phy_entry.key[0];
    if (entry_handle->core_id > DBAL_CORE_ANY)
    {
        entry_index |= (entry_handle->core_id << key_length);
    }

    SHR_GET_SEVERITY_FOR_MODULE(bsl_severity);
    if (bsl_severity >= bslSeverityInfo)
    {
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);
        LOG_CLI((BSL_META_U(unit, "DBAL SW Access (direct). Entry clear\n")));
        LOG_CLI((BSL_META_U(unit, "Entry index=0x%08x\n"), entry_index));
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);
    }

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.entries.entry_buffer.memwrite(unit,
                                                                       entry_handle->table_id,
                                                                       entry_index,
                                                                       default_payload,
                                                                       0,
                                                                       entry_handle->table->sw_payload_length_bytes));
exit:
    SHR_FUNC_EXIT;
}

/** **********************************************
 * \brief - Init the iterator info structure, according to the
 *        table parameters. 
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   \param [in] table_id - dbal table id
 *   \param [in] key_size - table key_size
 *   \param [in] sw_iterator - sw table iterator info
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 ***************************************************/
shr_error_e
dbal_sw_table_direct_iterator_init(
    int unit,
    dbal_tables_e table_id,
    uint32 key_size,
    dbal_sw_table_iterator_t * sw_iterator)
{
    SHR_FUNC_INIT_VARS(unit);

    sw_iterator->direct_iterated_all_entries = FALSE;
    sw_iterator->direct_max_index = (1 << key_size) - 1;
    SHR_IF_ERR_EXIT(dbal_table_default_entry_get(unit, table_id, sw_iterator->direct_default_entry));

exit:
    SHR_FUNC_EXIT;
}

/** **********************************************
 * \brief - get the next entry in direct table, relative to the
 *        entry that exist in iterator info
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit
 *   \param [in] iterator_info
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 ***************************************************/
shr_error_e
dbal_sw_table_direct_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    uint8 is_valid_entry = FALSE;
    uint8 entry_found = FALSE;
    dbal_entry_handle_t *entry_handle;
    dbal_sw_table_iterator_t *sw_iterator;

    SHR_FUNC_INIT_VARS(unit);

    entry_handle = iterator_info->entry_handle;
    sw_iterator = &iterator_info->sw_iterator;

    while ((!entry_found) && (!sw_iterator->direct_iterated_all_entries))
    {
        SHR_IF_ERR_EXIT(dbal_set_key_info_from_buffer(unit, entry_handle, iterator_info->keys_info, &is_valid_entry));
        if (is_valid_entry)
        {
            iterator_info->entry_handle->get_all_fields = TRUE;
            SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_get(unit, iterator_info->entry_handle, NULL));

            /*
             * if the entry is not equals to the default entry it is returned 
             */
            if (sal_memcmp(iterator_info->sw_iterator.direct_default_entry,
                           entry_handle->phy_entry.payload, DBAL_PHYSICAL_RES_SIZE_IN_WORDS * sizeof(uint32)) != 0)
            {
                entry_found = TRUE;
            }
        }

        /*
         * Check whether all entreis were iterated. 
         * set the next entry index to check
         */
        if (entry_handle->phy_entry.key[0] == sw_iterator->direct_max_index)
        {
            entry_handle->core_id--;
            if (entry_handle->core_id < 0)
            {
                sw_iterator->direct_iterated_all_entries = TRUE;
            }
            else
            {
                entry_handle->phy_entry.key[0] = 0;
            }
        }
        else
        {
            entry_handle->phy_entry.key[0]++;
        }
    }

    if (entry_found == FALSE)
    {
        iterator_info->is_end = TRUE;
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is used to add entry to a dbal sw state table of 
* type hash 
* It gets the entry index in the table according the hash table 
* (adds the key to hash table) and calls 
* dbal_sw_table_entry_set_in_index 
*  
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_hash_entry_add(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    uint32 data_indx = UTILEX_U32_MAX;
    uint8 success = FALSE;
    uint32 key_val[DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_WORDS] = { 0 };
    UTILEX_HASH_TABLE_PTR hash_table_id = 0;
    UTILEX_HASH_TABLE_KEY key[DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES] = { 0 };

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field
                    (entry_handle->phy_entry.key, 0, entry_handle->phy_entry.key_size, key_val));

    if (entry_handle->core_id > DBAL_CORE_ANY)
    {
        int key_length = entry_handle->phy_entry.key_size;
        SHR_IF_ERR_EXIT(utilex_bitstream_set_field(key_val, key_length, DBAL_CORE_SIZE_IN_BITS, entry_handle->core_id));
    }
    SHR_IF_ERR_EXIT(utilex_U32_to_U8(key_val, BITS2BYTES(entry_handle->phy_entry.key_size), key));

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.hash_table_id.get(unit, entry_handle->table_id, &hash_table_id));
    SHR_IF_ERR_EXIT(utilex_hash_table_entry_add(unit, hash_table_id, key, &data_indx, &success));

    if (!success)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "cant add entry to hash sw table hash table id = %d. table %s\n",
                     hash_table_id, entry_handle->table->table_name);
    }

    SHR_IF_ERR_EXIT(dbal_sw_table_entry_set_in_index(unit, data_indx, entry_handle));

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is used to get entry to a dbal sw state table of 
* type hash 
* It gets the entry index in the table according the hash table 
* (lookup with the key in the hash table) and calls 
* dbal_sw_table_entry_get_in_index 
*  
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_hash_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    uint32 data_indx;
    uint8 found;
    uint32 key_val[DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_WORDS] = { 0 };
    UTILEX_HASH_TABLE_PTR hash_table_id;
    UTILEX_HASH_TABLE_KEY key[DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES] = { 0 };

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field
                    (entry_handle->phy_entry.key, 0, entry_handle->phy_entry.key_size, key_val));

    if (entry_handle->core_id > DBAL_CORE_ANY)
    {
        int key_length = entry_handle->phy_entry.key_size;
        SHR_IF_ERR_EXIT(utilex_bitstream_set_field(key_val, key_length, DBAL_CORE_SIZE_IN_BITS, entry_handle->core_id));
    }
    SHR_IF_ERR_EXIT(utilex_U32_to_U8(key_val, BITS2BYTES(entry_handle->phy_entry.key_size), key));

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.hash_table_id.get(unit, entry_handle->table_id, &hash_table_id));
    SHR_IF_ERR_EXIT(utilex_hash_table_entry_lookup(unit, hash_table_id, key, &data_indx, &found));

    if (!found)
    {
        LOG_WARN(BSL_LOG_MODULE, (BSL_META_U(unit, "entry was not found in sw hash table. hash_table_id=%d\n"),
                                  hash_table_id));
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_sw_table_entry_get_in_index(unit, data_indx, entry_handle, NULL));
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is used to delete entry to a dbal sw state table 
* of type hash 
* It remove the key from the hash table
*  
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_hash_entry_delete(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    uint32 key_val[DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_WORDS] = { 0 };
    UTILEX_HASH_TABLE_PTR hash_table_id;
    UTILEX_HASH_TABLE_KEY key[DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES] = { 0 };
    int bsl_severity;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field
                    (entry_handle->phy_entry.key, 0, entry_handle->phy_entry.key_size, key_val));

    if (entry_handle->core_id > DBAL_CORE_ANY)
    {
        int key_length = entry_handle->phy_entry.key_size;
        SHR_IF_ERR_EXIT(utilex_bitstream_set_field(key_val, key_length, DBAL_CORE_SIZE_IN_BITS, entry_handle->core_id));
    }
    SHR_IF_ERR_EXIT(utilex_U32_to_U8(key_val, BITS2BYTES(entry_handle->phy_entry.key_size), key));

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.hash_table_id.get(unit, entry_handle->table_id, &hash_table_id));

    SHR_GET_SEVERITY_FOR_MODULE(bsl_severity);
    if (bsl_severity >= bslSeverityInfo)
    {
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);
        LOG_CLI((BSL_META_U(unit, "DBAL SW Access (hash). Entry clear\n")));
        if (entry_handle->phy_entry.key_size > 32)
        {
            LOG_CLI((BSL_META_U(unit, "hash key=0x%02x%02x%02x%02x%02x%02x%02x%02x\n"),
                     key[7], key[6], key[5], key[4], key[3], key[2], key[1], key[0]));
        }
        else
        {
            LOG_CLI((BSL_META_U(unit, "hash key=0x%02x%02x%02x%02x\n"), key[3], key[2], key[1], key[0]));
        }
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);
    }
    SHR_IF_ERR_EXIT(utilex_hash_table_entry_remove(unit, hash_table_id, key));

exit:
    SHR_FUNC_EXIT;
}

/** **********************************************
 * \brief - Init the iterator info structure, according to the
 *        table parameters. 
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   \param [in] table_id - dbal table id
 *   \param [in] sw_hash_iterator - hash table iterator info
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 ***************************************************/
shr_error_e
dbal_sw_table_hash_iterator_init(
    int unit,
    dbal_tables_e table_id,
    dbal_sw_table_iterator_t * sw_iterator)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.hash_table_id.get(unit, table_id, &sw_iterator->hash_table_id));
    sw_iterator->hash_entry_index = 0;

exit:
    SHR_FUNC_EXIT;
}

/** **********************************************
 * \brief - get the next entry in hash table, relative to the
 *        entry that exist in iterator info
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit
 *   \param [in] iterator_info
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 ***************************************************/
shr_error_e
dbal_sw_table_hash_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    uint32 data_indx;
    UTILEX_HASH_TABLE_KEY key[DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES];

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(utilex_hash_table_get_next(unit, iterator_info->sw_iterator.hash_table_id,
                                               &iterator_info->sw_iterator.hash_entry_index, key, &data_indx));

    if (UTILEX_HASH_TABLE_ITER_IS_END(&iterator_info->sw_iterator.hash_entry_index))
    {
        iterator_info->is_end = TRUE;
        SHR_EXIT();
    }
    else
    {
        uint8 is_valid;
        SHR_IF_ERR_EXIT(utilex_U8_to_U32(key, DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES,
                                         iterator_info->entry_handle->phy_entry.key));
        SHR_IF_ERR_EXIT(dbal_set_key_info_from_buffer(unit, iterator_info->entry_handle, iterator_info->keys_info,
                                                      &is_valid));
        if (!is_valid)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "SW Hash iterator found invalid entry\n");
        }

        iterator_info->entry_handle->get_all_fields = TRUE;
        SHR_IF_ERR_EXIT(dbal_sw_table_hash_entry_get(unit, iterator_info->entry_handle));
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is the generic API to set an  entry in dbal SW 
* state tables. 
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_entry_set(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    dbal_sw_state_table_type_e table_type;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.get(unit, entry_handle->table_id, &table_type));
    switch (table_type)
    {
        case DBAL_SW_TABLE_DIRECT:
            SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_set(unit, entry_handle));
            break;
        case DBAL_SW_TABLE_HASH:
            SHR_IF_ERR_EXIT(dbal_sw_table_hash_entry_add(unit, entry_handle));
            break;
        default:
            break;
    }
exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is the generic API to get an entry in dbal SW
* state tables.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    dbal_sw_state_table_type_e table_type;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.get(unit, entry_handle->table_id, &table_type));
    switch (table_type)
    {
        case DBAL_SW_TABLE_DIRECT:
            SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_get(unit, entry_handle, NULL));
            break;
        case DBAL_SW_TABLE_HASH:
            SHR_IF_ERR_EXIT(dbal_sw_table_hash_entry_get(unit, entry_handle));
            break;
        default:
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function is the generic API to delete an entry in dbal SW
* state tables.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] entry_handle - \n
*      dbal entry handle - holds the entry information
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*    Error code
*****************************************************/
shr_error_e
dbal_sw_table_entry_clear(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    dbal_sw_state_table_type_e table_type;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.get(unit, entry_handle->table_id, &table_type));
    switch (table_type)
    {
        case DBAL_SW_TABLE_DIRECT:
            SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_clear(unit, entry_handle));
            break;

        case DBAL_SW_TABLE_HASH:
            SHR_IF_ERR_EXIT(dbal_sw_table_hash_entry_delete(unit, entry_handle));
            break;

        default:
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - clear a sw table. direct or hash
 * clear direct table mean to set all entries to zero
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   \param [in] entry_handle_id -
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e
dbal_sw_table_clear(
    int unit,
    uint32 entry_handle_id)
{
    dbal_sw_state_table_type_e table_type;
    UTILEX_HASH_TABLE_PTR hash_table_id;
    dbal_entry_handle_t *entry_handle;
    uint32 entry_index, max_entry_index;
    uint8 default_payload[DBAL_PHYSICAL_RES_SIZE_IN_WORDS] = { 0 };

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle));

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.get(unit, entry_handle->table_id, &table_type));
    switch (table_type)
    {
        case DBAL_SW_TABLE_DIRECT:
            max_entry_index = utilex_power_of_2(entry_handle->table->key_size);
            for (entry_index = 0; entry_index < max_entry_index; entry_index++)
            {
                SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.entries.entry_buffer.memwrite(unit,
                                                                                   entry_handle->table_id,
                                                                                   entry_index,
                                                                                   default_payload,
                                                                                   0,
                                                                                   entry_handle->table->
                                                                                   sw_payload_length_bytes));
            }
            break;

        case DBAL_SW_TABLE_HASH:
            SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.hash_table_id.get(unit, entry_handle->table_id, &hash_table_id));
            SHR_IF_ERR_EXIT(utilex_hash_table_clear(unit, hash_table_id));
            break;

        default:
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - iterator init. sw tables shell.
 * calls iterator init of direct/hash according to tablt type
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   \param [in] iterator_info -
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e
dbal_sw_table_iterator_init(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    dbal_sw_state_table_type_e sw_table_type;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.get(unit, iterator_info->table_id, &sw_table_type));

    switch (sw_table_type)
    {
        case DBAL_SW_TABLE_DIRECT:
            SHR_IF_ERR_EXIT(dbal_sw_table_direct_iterator_init(unit, iterator_info->table_id,
                                                               iterator_info->entry_handle->phy_entry.key_size,
                                                               &iterator_info->sw_iterator));
            break;

        case DBAL_SW_TABLE_HASH:
            SHR_IF_ERR_EXIT(dbal_sw_table_hash_iterator_init(unit, iterator_info->table_id,
                                                             &iterator_info->sw_iterator));
            break;

        default:
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - iterator get next. sw tables shell
 * calls iterator get next of direct/hash according to tablt
 * type
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   \param [in] iterator_info -
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e
dbal_sw_table_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    dbal_sw_state_table_type_e sw_table_type;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.get(unit, iterator_info->entry_handle->table_id, &sw_table_type));

    switch (sw_table_type)
    {
        case DBAL_SW_TABLE_DIRECT:
            SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_get_next(unit, iterator_info));
            break;

        case DBAL_SW_TABLE_HASH:
            SHR_IF_ERR_EXIT(dbal_sw_table_hash_entry_get_next(unit, iterator_info));
            break;

        default:
            break;
    }
exit:
    SHR_FUNC_EXIT;
}
