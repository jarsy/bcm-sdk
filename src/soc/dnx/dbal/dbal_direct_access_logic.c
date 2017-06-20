/*
 * $Id: dbal_direct_access_logic.c,v 1.13 Broadcom SDK $
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
#include <soc/cmic.h>

#define DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS         (20)

#define DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES         (20*sizeof(uint32))

#define DBAL_MEM_READ_LOG( memory, mem_offset, block, mem_array_offset)                                             \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);                                                  \
        LOG_INFO_EX(BSL_LOG_MODULE, "Read preformed from memory %s, entry offset = %d block %d array_offset %d\n",  \
                    SOC_MEM_NAME(unit, memory), mem_offset, block, mem_array_offset);                               \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);                                                 \

#define DBAL_MEM_WRITE_LOG( memory, mem_offset, block, mem_array_offset)                                            \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);                                                  \
        LOG_INFO_EX(BSL_LOG_MODULE, "Write preformed to memory %s, entry offset = %d block %d array_offset %d\n",   \
                    SOC_MEM_NAME(unit, memory), mem_offset, block, mem_array_offset);                               \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);                                                 \

#define DBAL_MEM_WRITE_ARRAY_LOG( memory, mem_offset, block, num_elem)                                              \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);                                                  \
        LOG_INFO_EX(BSL_LOG_MODULE, "Write preformed to memory %s, entry offset = %d block %d array num elem %d\n", \
                    SOC_MEM_NAME(unit, memory), mem_offset, block, num_elem);                                       \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);                                                 \

#define DBAL_REG_READ_LOG( reg, reg_offset, block, reg_array_offset)                                                \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);                                                  \
        LOG_INFO_EX(BSL_LOG_MODULE, "Read preformed from register %s, entry offset %d block %d array offset %d\n",  \
                    SOC_REG_NAME(unit, reg), reg_offset, block, reg_array_offset);                                  \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);                                                 \

#define DBAL_REG_WRITE_LOG( reg, reg_offset, block, reg_array_offset)                                               \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityInfo);                                                  \
        LOG_INFO_EX(BSL_LOG_MODULE, "Write preformed to register %s, entry offset %d block %d array_offset %d\n",   \
                    SOC_REG_NAME(unit, reg), reg_offset, block, reg_array_offset);                                  \
        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityInfo);                                                 \

#define DBAL_DATA_LOG( data)                                            \
{                                                                       \
    int ii;                                                             \
    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "Data: ")));          \
    for (ii = 0; ii < DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS; ii++)  \
    {                                                                   \
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%d"), data[ii]));\
    }                                                                   \
    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));              \
}





shr_error_e
dbal_direct_condition_check(
    int unit,
    dbal_entry_handle_t * entry_handle,
    uint32 key,
    dbal_access_condition_info_t * access_condition,
    uint8 * is_passed)
{
    int cond_indx;
    uint32 compared_key;
    dbal_access_condition_info_t condition;
    uint8 is_condition_true = FALSE;
    uint32 is_found;

    SHR_FUNC_INIT_VARS(unit);

    for (cond_indx = 0; cond_indx < DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS; cond_indx++)
    {
        is_condition_true = FALSE;
        condition = access_condition[cond_indx];
        if (condition.field_id != DBAL_FIELD_EMPTY)
        {
            SHR_IF_ERR_EXIT(dbal_entry_key_field_from_handle_get(unit, entry_handle, condition.field_id, &compared_key,
                                                                 &is_found));
            if (!is_found)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal field %s not exists in table %s\n",
                             dbal_field_to_string(unit, condition.field_id), entry_handle->table->table_name);
            }
        }
        else
        {
            compared_key = key;
        }
        switch (condition.type)
        {
            case DBAL_CONDITION_BIGGER_THAN:
                if (compared_key > condition.value)
                {
                    is_condition_true = TRUE;
                }
                break;

            case DBAL_CONDITION_LOWER_THAN:
                if (compared_key < condition.value)
                {
                    is_condition_true = TRUE;
                }
                break;

            case DBAL_CONDITION_EQUAL_TO:
                if (compared_key == condition.value)
                {
                    is_condition_true = TRUE;
                }
                break;

            case DBAL_CONDITION_NONE:
                is_condition_true = TRUE;
                break;

            case DBAL_CONDITION_IS_EVEN:
                if (compared_key % 2 == 0)
                {
                    is_condition_true = TRUE;
                }
                break;

            case DBAL_CONDITION_IS_ODD:
                if (compared_key % 2 == 1)
                {
                    is_condition_true = TRUE;
                }
                break;

            default:
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal condition value %d\n", condition.type);
                break;
        }

        if (is_condition_true == FALSE)
        {
            *is_passed = 0;
            goto exit;
        }
    }

    *is_passed = 1;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_direct_offset_calculate(
    int unit,
    dbal_entry_handle_t * entry_handle,
    uint32 index,
    dbal_offset_encode_info_t * encode_info,
    uint32 * calc_index)
{
    dbal_logical_table_t *table = entry_handle->table;

    SHR_FUNC_INIT_VARS(unit);

    (*calc_index) = 0;

    /*
     * in this case we use specific field to be the key of the entry 
     */
    if (encode_info->field_id != DBAL_FIELD_EMPTY)
    {

        uint32 field_val = 0, is_found = 0;
        SHR_IF_ERR_EXIT(dbal_entry_key_field_from_handle_get(unit, entry_handle,
                                                             encode_info->field_id, &field_val, &is_found));
        if (is_found)
        {
            index = field_val;
        }
        else
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Encoding uses field %s that not added to table %s\n",
                         dbal_field_to_string(unit, encode_info->field_id), table->table_name);
        }

        if (encode_info->field_id == DBAL_FIELD_CORE_ID)
        {
            if ((entry_handle->core_id == DBAL_CORE_NOT_INTIATED) || (entry_handle->core_id == DBAL_CORE_ANY))
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Encoding uses CORE_ID, but field not added\n");
            }
            else
            {
                index = entry_handle->core_id;
            }
        }
    }

    switch (encode_info->encode_mode)
    {
        case DBAL_VALUE_OFFSET_ENCODE_NONE:
        case DBAL_VALUE_OFFSET_ENCODE_PARTIAL_KEY:
            (*calc_index) = index;
            break;

        case DBAL_VALUE_OFFSET_ENCODE_MODULO:
            (*calc_index) = index % encode_info->input_param;
            break;

        case DBAL_VALUE_OFFSET_ENCODE_DIVIDE:
            (*calc_index) = index / encode_info->input_param;
            break;

        case DBAL_VALUE_OFFSET_ENCODE_MULTIPLE:
            (*calc_index) = index * encode_info->input_param;
            break;

        case DBAL_VALUE_OFFSET_ENCODE_BOOL:
            if (index != 0)
            {
                (*calc_index) = 1;
            }
            break;

        case DBAL_VALUE_OFFSET_ENCODE_SUBTRACT:
            (*calc_index) = index - encode_info->input_param;
            break;

        case DBAL_VALUE_OFFSET_ENCODE_HARD_VALUE:
            (*calc_index) = encode_info->input_param;
            break;

        case DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP:
            (*calc_index) = (index % encode_info->input_param) * encode_info->internal_inparam;
            break;

        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal encode mode %d\n", encode_info->encode_mode);
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_direct_memory_block_calculate(
    int unit,
    dbal_entry_handle_t * entry_handle,
    uint32 core_id,
    soc_mem_t memory,
    dbal_offset_encode_info_t * block_index_info,
    int *block,
    int *num_of_blocks)
{
    uint32 calc_block = 0;

    SHR_FUNC_INIT_VARS(unit);

    if (block_index_info->encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
    {/** in this case a specific block should be set */
        SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, 0, block_index_info, &calc_block));

        (*block) = SOC_MEM_BLOCK_MIN(unit, memory) + calc_block;
        (*num_of_blocks) = 1;
    }
    else
    {
        if (core_id != DBAL_CORE_ANY)
        {/** in this case the the number of blocks depends on the number of cores X blocks per core */
            
            (*num_of_blocks) = (SOC_MEM_BLOCK_MAX(unit, memory) - SOC_MEM_BLOCK_MIN(unit, memory) + 1);
            (*num_of_blocks) = (*num_of_blocks) / 2;
            (*block) = SOC_MEM_BLOCK_MIN(unit, memory) + (core_id * (*num_of_blocks));
        }
        else
        {
            (*num_of_blocks) = 1;
            (*block) = MEM_BLOCK_ANY; /**just making sure that this is the value*/
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/** General description:
 *  this function reads from the HW according to the key and updated the entry handle buffer.
 *  only requested fields are received from HW (incase of get all fields is set all fields are requested).
 *  result_type_get: when NULL the function acts in the normal way, when using legal result type
 *  this function only tries to read only the result type from HW
 *   */
shr_error_e
dbal_direct_memory_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    int *result_type_get)
{
    int iter;
    soc_mem_t last_memory_used = INVALIDm;
    int last_mem_entry_offset = 0;
    int last_mem_array_offset_used = 0;
    uint32 mem_entry_offset = 0, field_offset = 0, mem_array_offset = 0, alias_offset;
    uint8 is_conditaion_pass = 0;
    uint32 data[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };
    uint32 key = entry_handle->phy_entry.key[0];    
    uint32 core_id = DBAL_CORE_ANY;
    dbal_logical_table_t *table = entry_handle->table;
    dbal_direct_l2p_info_t *l2p_direct_info;
    dbal_direct_l2p_field_info_t *curr_l2p_info;
    int block = MEM_BLOCK_ANY;
    int result_type = entry_handle->cur_res_type;

    SHR_FUNC_INIT_VARS(unit);

    if (result_type_get)
    {
        if (result_type != DBAL_RESULT_TYPE_NOT_INITIALIZED)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "illegal result type already found\n");
        }
        result_type = (*result_type_get);
    }

    if (result_type == DBAL_RESULT_TYPE_NOT_INITIALIZED)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "illegal result type\n");
    }

    l2p_direct_info = &(table->hl_mapping_multi_res[result_type].l2p_direct_info[DBAL_HL_ACCESS_MEMORY]);
    if (table->core_mode == DBAL_CORE_BY_INPUT)
    {
        if (entry_handle->core_id != DBAL_CORE_NOT_INTIATED)
        {
            core_id = entry_handle->core_id;
        }
    }

    for (iter = 0; iter < l2p_direct_info->num_of_access_fields; iter++)
    {
        int field_pos;
        curr_l2p_info = &l2p_direct_info->l2p_fields_info[iter];
        field_pos = curr_l2p_info->field_pos_in_interface;

        if (entry_handle->nof_result_types > 1)
        {
            /** multiple result retrieve the result type first */
            int table_iter;
            if (result_type_get)
            {
                if (curr_l2p_info->field_id != DBAL_FIELD_RESULT_TYPE)
                {
                    continue;
                }
            }
            for (table_iter = 0; table_iter < table->multi_res_info[result_type].nof_result_fields; table_iter++)
            {
                if (curr_l2p_info->field_id == table->multi_res_info[result_type].results_info[table_iter].field_id)
                {
                    field_pos = table_iter;
                    break;
                }
            }
            if (table_iter == table->multi_res_info[result_type].nof_result_fields)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "field not found in table %s\n",
                             dbal_field_to_string(unit, curr_l2p_info->field_id));
            }
        }
        else
        {
            if (!entry_handle->get_all_fields)
            {
                if (entry_handle->value_field_ids[field_pos] == DBAL_FIELD_EMPTY)
                {
                    continue;
                }
            }
        }

        SHR_IF_ERR_EXIT(dbal_direct_condition_check(unit, entry_handle, key, curr_l2p_info->mapping_condition,
                                                    &is_conditaion_pass));
        if (is_conditaion_pass)
        {
            if (curr_l2p_info->data_offset_info.encode_mode == DBAL_VALUE_OFFSET_ENCODE_NONE)
            /*
             * calculate the memory array offset no encoding to the array index means that offset is 0
             */
            mem_array_offset = 0;
            if (curr_l2p_info->array_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key,
                                                             &(curr_l2p_info->array_offset_info),
                                                             &mem_array_offset));
            }
            
            /*
             * mem entry offset calculate
             */
            SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key,
                                                         &(curr_l2p_info->entry_offset_info), &mem_entry_offset));

            /*
             * field offset calculate
             */
            field_offset = 0;
            if (curr_l2p_info->data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key,
                                                         &(curr_l2p_info->data_offset_info), &field_offset));
            }

            if ((last_memory_used == INVALIDm) || (last_memory_used != curr_l2p_info->memory) ||
                (last_mem_entry_offset != mem_entry_offset) || (last_mem_array_offset_used != mem_array_offset))
            {/** need to read new memory from HW */
                int num_of_blocks;
                soc_mem_t memory_to_read = (curr_l2p_info->alias_memory != INVALIDm)? curr_l2p_info->alias_memory:curr_l2p_info->memory;
                last_memory_used = curr_l2p_info->memory;
                last_mem_array_offset_used = mem_array_offset;
                last_mem_entry_offset = mem_entry_offset;
                sal_memset(data, 0x0, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);
                alias_offset = 0;

                SHR_IF_ERR_EXIT(dbal_direct_memory_block_calculate(unit, entry_handle, core_id, last_memory_used,
                                                                   &(curr_l2p_info->block_index_info), &block,
                                                                   &num_of_blocks));                

                if(curr_l2p_info->alias_memory != INVALIDm)
                {
                    if (curr_l2p_info->alias_data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
                    {                        
                        SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key, &(curr_l2p_info->alias_data_offset_info),
                                                             &alias_offset));
                    }
                }

                if (alias_offset != 0)
                {                            
                    uint32 data_for_alias[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };                            

                    SHR_IF_ERR_EXIT(soc_mem_array_read(unit, memory_to_read,
                                               last_mem_array_offset_used, block, mem_entry_offset, data_for_alias));
                    DBAL_MEM_READ_LOG(memory_to_read, mem_entry_offset, block, last_mem_array_offset_used);
                    DBAL_DATA_LOG(data_for_alias);
                    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "alias_offset = %d \n"), alias_offset));
                    
                    /**copy from alias data only the memory part that is needed */ 
                    SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(data_for_alias, alias_offset, 
                                                                   curr_l2p_info->alias_data_offset_info.internal_inparam, data));
                    DBAL_DATA_LOG(data);
                }
                else
                {
                    SHR_IF_ERR_EXIT(soc_mem_array_read(unit, memory_to_read,
                                               last_mem_array_offset_used, block, mem_entry_offset, data));
                    DBAL_MEM_READ_LOG(memory_to_read, mem_entry_offset, block, last_mem_array_offset_used);
                    DBAL_DATA_LOG(data);

                }
            }            

            {
                uint32 field_val[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS];
                sal_memset(field_val, 0x0, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);

                if (curr_l2p_info->hw_field != INVALIDf)
                {
                    soc_mem_field_get(unit, curr_l2p_info->memory, data, curr_l2p_info->hw_field, field_val);

                    /*
                     * this means that the field has offset, need to handle it
                     */
                    if (field_offset != 0)
                    {
                        uint32 tmp_field_val[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS];
                        sal_memset(tmp_field_val, 0x0, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);
                        SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val, field_offset,
                                                                       table->multi_res_info
                                                                       [result_type].results_info
                                                                       [field_pos].field_nof_bits, tmp_field_val));

                        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityVerbose);
                        LOG_VERBOSE_EX(BSL_LOG_MODULE,
                                       "Taking part of the field orig val %x, offset %d, received val %x%s\n",
                                       tmp_field_val[0],
                                       table->multi_res_info[result_type].results_info
                                       [field_pos].bits_offset_in_buffer, tmp_field_val[0], EMPTY);
                        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityVerbose);
                        sal_memcpy(field_val, tmp_field_val, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);
                    }

                    LOG_INFO_EX(BSL_LOG_MODULE, "read field %s, field offset %d value 0x%x %s\n",
                                SOC_FIELD_NAME(unit, curr_l2p_info->hw_field), field_offset, field_val[0], EMPTY);
                    SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(field_val,
                                                                   curr_l2p_info->offset_in_interface,
                                                                   curr_l2p_info->nof_bits_in_interface,
                                                                   entry_handle->phy_entry.payload));
                }
                else
                {
                    if (field_offset != 0)
                    {
                        uint32 data_aligend[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };
                        SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(data,
                                                                       field_offset,
                                                                       (curr_l2p_info->nof_bits_in_interface),
                                                                       data_aligend));

                        SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(data_aligend,
                                                                       (curr_l2p_info->offset_in_interface),
                                                                       (curr_l2p_info->nof_bits_in_interface),
                                                                       (uint32 *) (entry_handle->phy_entry.payload)));
                    }
                    else
                    {
                        SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(data,
                                                                       (curr_l2p_info->offset_in_interface),
                                                                       (curr_l2p_info->nof_bits_in_interface),
                                                                       (uint32 *) (entry_handle->phy_entry.payload)));
                    }
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/** actual access to HW, writing to memory. this method calculate the block and array indexes to HW memory*/
shr_error_e
dbal_direct_memory_write(
    int unit,
    dbal_entry_handle_t * entry_handle,
    dbal_direct_l2p_field_info_t * l2p_info,
    uint32 * data,
    uint32 core_id,
    int array_offset,
    int entry_offset)
{
    int block_iter, num_of_blocks;
    int block = MEM_BLOCK_ANY;
    int is_array_fill_needed = 0;
    soc_mem_t memory;

    SHR_FUNC_INIT_VARS(unit);

    if (!l2p_info)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "l2P not initialized\n");
    }
    
     memory = l2p_info->memory;   

     /** alias treatment */
     if (l2p_info->alias_memory != INVALIDm)
     {
         memory = l2p_info->alias_memory;         
     }

    /** if the array offset was not encoded (not specific array index requested to access)
     *  check if the memory is array if it is an array, access all the array */
    if (l2p_info->array_offset_info.encode_mode == DBAL_VALUE_OFFSET_ENCODE_NONE)
    {
        array_offset = 0;
        if (SOC_MEM_IS_ARRAY(unit, memory))
        {
            is_array_fill_needed = 1;
        }
    }

    SHR_IF_ERR_EXIT(dbal_direct_memory_block_calculate
                    (unit, entry_handle, core_id, memory, &(l2p_info->block_index_info), &block, &num_of_blocks));

    for (block_iter = 0; block_iter < num_of_blocks; block_iter++)
    {
        if (is_array_fill_needed)
        {
            int nof_elm = SOC_MEM_NUMELS(unit, memory) -1;
           
                                      
            SHR_IF_ERR_EXIT(soc_mem_array_fill_range(unit, 0, memory, 0, nof_elm, block, entry_offset, entry_offset, data));
            DBAL_MEM_WRITE_ARRAY_LOG(memory, entry_offset, block, nof_elm);
            DBAL_DATA_LOG(data);
        }
        else
        {
            SHR_IF_ERR_EXIT(soc_mem_array_write(unit, memory, array_offset, block, entry_offset, data));
            DBAL_MEM_WRITE_LOG(memory, entry_offset, block, array_offset);
            DBAL_DATA_LOG(data);
        }
        block++;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_direct_memory_set(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int iter;
    soc_mem_t last_memory_used = INVALIDm;
    int last_mem_offset_used = 0;
    int last_mem_array_offset_used = 0;
    uint32 mem_offset = 0, field_offset = 0, mem_array_offset = 0, alias_offset = 0;
    uint8 is_conditaion_pass = 0;
    uint32 field_data[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };
    uint32 data[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };
    uint32 data_for_alias[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };
    uint32 key = entry_handle->phy_entry.key[0];    
    uint32 core_id = DBAL_CORE_ANY;
    dbal_logical_table_t *table = entry_handle->table;
    dbal_direct_l2p_info_t *l2p_direct_info;
    dbal_direct_l2p_field_info_t *curr_l2p_info = NULL;
    dbal_direct_l2p_field_info_t *last_l2p_info = NULL;
    int block = MEM_BLOCK_ANY, num_of_blocks;
    int result_type = entry_handle->cur_res_type;

    SHR_FUNC_INIT_VARS(unit);

    l2p_direct_info = &(table->hl_mapping_multi_res[result_type].l2p_direct_info[DBAL_HL_ACCESS_MEMORY]);

    if (table->core_mode == DBAL_CORE_BY_INPUT)
    {
        if (entry_handle->core_id != DBAL_CORE_NOT_INTIATED)
        {
            core_id = entry_handle->core_id;
        }
    }

    for (iter = 0; iter < l2p_direct_info->num_of_access_fields; iter++)
    {
        int field_pos;
        last_l2p_info = curr_l2p_info;/** keeping the last valid memory info that was used*/
        field_pos = l2p_direct_info->l2p_fields_info[iter].field_pos_in_interface;
        if (entry_handle->value_field_ids[field_pos] == DBAL_FIELD_EMPTY)
        {/** field not requested */
            continue;
        }

        sal_memset(field_data, 0x0, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);

        SHR_IF_ERR_EXIT(dbal_direct_condition_check
                        (unit, entry_handle, key, l2p_direct_info->l2p_fields_info[iter].mapping_condition,
                         &is_conditaion_pass));
        if (is_conditaion_pass)
        {
            curr_l2p_info = &l2p_direct_info->l2p_fields_info[iter];
            /*
             * calculate the memory array offset 
             * incase no encoding to the array index can be one of two options: 
             * 1. the memory is not array and we need to afill in array index 0
             * 2. the memory is array and we need to fill all the array 
             * otherwise, regular encoding 
             */

            mem_array_offset = 0;
            if (curr_l2p_info->array_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key, &(curr_l2p_info->array_offset_info),
                                                             &mem_array_offset));                
            }
            
            /*
             * calculate the memory entry offset 
             */
            SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key, &(curr_l2p_info->entry_offset_info),
                                                         &mem_offset));

            /*
             * calculate the memory field offset 
             */ 
            field_offset = 0;           
            if (curr_l2p_info->data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key,
                                                         &(curr_l2p_info->data_offset_info), &field_offset));
            }

            /*
             * if the current memory is the same memory has the last no need to write yet
             */

            if ((last_memory_used != curr_l2p_info->memory) ||
                (last_mem_offset_used != mem_offset) || (last_mem_array_offset_used != mem_array_offset))
            {   
                soc_mem_t memory_to_read = (curr_l2p_info->alias_memory != INVALIDm)? curr_l2p_info->alias_memory:curr_l2p_info->memory;
                /*
                 * not the first memory, need to preform mem write to the prev memory 
                 */
                if (last_memory_used != INVALIDm)
                {
                    if (alias_offset > 0)
                    {
                        /* 
                         * if aliase offset exists we need to copy the data to the original data (data_for_alias)
                         * only the new part of the data in the correct offset
                         */ 
                        SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(data, alias_offset, 
                                                               curr_l2p_info->alias_data_offset_info.internal_inparam, data_for_alias));

                        SHR_IF_ERR_EXIT(dbal_direct_memory_write(unit, 
                                                                 entry_handle,                                                 
                                                                 curr_l2p_info,
                                                                 &(data_for_alias[0]),
                                                                 core_id, 
                                                                 last_mem_array_offset_used, 
                                                                 last_mem_offset_used));       
                    }
                    else
                    {
                        SHR_IF_ERR_EXIT(dbal_direct_memory_write(unit, 
                                                                 entry_handle, 
                                                                 last_l2p_info,
                                                                 &(data[0]),
                                                                 core_id, 
                                                                 last_mem_array_offset_used, 
                                                                 last_mem_offset_used));                    
                    }
                }                

                sal_memset(data, 0x0, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);

                /*
                 * updating the parameters of the current memory 
                 */
                last_memory_used = curr_l2p_info->memory;
                last_mem_offset_used = mem_offset;
                last_mem_array_offset_used = mem_array_offset;
                block = MEM_BLOCK_ANY;
                SHR_IF_ERR_EXIT(dbal_direct_memory_block_calculate(unit, entry_handle, core_id, last_memory_used,
                                                                   &(curr_l2p_info->block_index_info), &block,
                                                                   &num_of_blocks));

                SHR_IF_ERR_EXIT(soc_mem_array_read(unit, memory_to_read, mem_array_offset, block, mem_offset, data));
                DBAL_MEM_READ_LOG(memory_to_read, mem_offset, block, last_mem_array_offset_used);
                DBAL_DATA_LOG(data);

                alias_offset = 0;
                if(curr_l2p_info->alias_memory != INVALIDm)
                {
                    if (curr_l2p_info->alias_data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
                    {
                        SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key, &(curr_l2p_info->alias_data_offset_info),
                                                             &alias_offset));
                        if (alias_offset != 0)
                        {
                            LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "alias_offset = %d \n"), alias_offset));
                            sal_memcpy(data_for_alias, data, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);                            

                            /** copying the relevant part of the memory to begining of data */
                            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(data_for_alias, alias_offset, 
                                                                           curr_l2p_info->alias_data_offset_info.internal_inparam, data));
                            DBAL_DATA_LOG(data);
                        }
                    }                    
                }
            }

            /*
             * this part takes the field value from the buffer, taking only the reqiured bits 
             */
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(entry_handle->phy_entry.payload,
                                                           curr_l2p_info->offset_in_interface,
                                                           curr_l2p_info->nof_bits_in_interface, field_data));

            if (curr_l2p_info->hw_field != INVALIDf)
            {
                /*
                 * handling offset in field when field offset exists 
                 */
                if (curr_l2p_info->data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
                {
                    uint32 tmp_field_data[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };

                    soc_mem_field_get(unit, last_memory_used, data, curr_l2p_info->hw_field, tmp_field_data);
                    SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(field_data, field_offset,
                                                                   table->multi_res_info
                                                                   [result_type].results_info
                                                                   [curr_l2p_info->
                                                                    field_pos_in_interface].field_nof_bits,
                                                                   tmp_field_data));

                    sal_memcpy(field_data, tmp_field_data, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);
                }
                
                soc_mem_field_set(unit, last_memory_used, data, curr_l2p_info->hw_field, field_data);
            }
            else
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(field_data, field_offset,
                                                               table->multi_res_info
                                                               [result_type].results_info
                                                               [curr_l2p_info->field_pos_in_interface].field_nof_bits,
                                                               data));
            }
            DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityVerbose);
            LOG_VERBOSE_EX(BSL_LOG_MODULE, "write field %s, field offset %d, value 0x%x (%u) \n",
                           SOC_FIELD_NAME(unit, curr_l2p_info->hw_field), field_offset, field_data[0], field_data[0]);
            DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityVerbose);

        }
    }

    if (last_memory_used != 0)
    {
        if (alias_offset > 0)
        {
            /* 
             * if aliase offset exists we need to copy the data to the original data (data_for_alias)
             * only the new part of the data in the correct offset
             */
            SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(data, alias_offset, 
                                                               curr_l2p_info->alias_data_offset_info.internal_inparam, data_for_alias));

            SHR_IF_ERR_EXIT(dbal_direct_memory_write(unit, 
                                                     entry_handle,                                                 
                                                     curr_l2p_info,
                                                     &(data_for_alias[0]),
                                                     core_id, 
                                                     last_mem_array_offset_used, 
                                                     last_mem_offset_used));        
        }
        else
        {
            SHR_IF_ERR_EXIT(dbal_direct_memory_write(unit, 
                                                     entry_handle,                                                 
                                                     curr_l2p_info,
                                                     &(data[0]),
                                                     core_id, 
                                                     last_mem_array_offset_used, 
                                                     last_mem_offset_used));        
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/** General description:
 *  this function reads from the HW according to the key and updated the entry handle buffer.
 *  only requested fields are read from HW. incase of get all fields is set. all fields are requested.
 *  result_type_get: when NULL the function acts in the normal way, when this function called with legal result type
 *  this function only tries to read the result type from HW
 *   */
shr_error_e
dbal_direct_register_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    int *result_type_get)
{
    int iter;
    soc_reg_t last_register_used = INVALIDr;
    uint32 field_offset = 0, last_reg_array_offset = 0, reg_array_offset = 0;
    uint8 is_conditaion_pass = 0;
    uint32 data[SOC_REG_ABOVE_64_MAX_SIZE_U32] = { 0 };
    uint32 key = entry_handle->phy_entry.key[0];
    uint32 core_id = DBAL_CORE_ANY;
    int block = REG_PORT_ANY;
    dbal_logical_table_t *table = entry_handle->table;
    dbal_direct_l2p_info_t *l2p_direct_info;
    dbal_direct_l2p_field_info_t *curr_l2p_info;
    int result_type = entry_handle->cur_res_type;

    SHR_FUNC_INIT_VARS(unit);

    if (result_type_get)
    {
        /** using just one of the result types to get the result type  */
        if (result_type != DBAL_RESULT_TYPE_NOT_INITIALIZED)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "result type already found\n");
        }
        result_type = (*result_type_get);
    }
    if (result_type == DBAL_RESULT_TYPE_NOT_INITIALIZED)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "illegal result type\n");
    }

    l2p_direct_info = &table->hl_mapping_multi_res[result_type].l2p_direct_info[DBAL_HL_ACCESS_REGISTER];

    if (table->core_mode == DBAL_CORE_BY_INPUT)
    {
        if (entry_handle->core_id != DBAL_CORE_NOT_INTIATED)
        {
            core_id = entry_handle->core_id;
        }
        if (core_id != DBAL_CORE_ANY)
        {
            block = core_id;
        }
    }

    for (iter = 0; iter < l2p_direct_info->num_of_access_fields; iter++)
    {
        int field_pos;

        curr_l2p_info = &l2p_direct_info->l2p_fields_info[iter];
        field_pos = curr_l2p_info->field_pos_in_interface;

        if (entry_handle->nof_result_types > 1)
        {
            /** multiple result retrieve the result type first */
            int table_iter;
            if (result_type_get)
            {
                if (curr_l2p_info->field_id != DBAL_FIELD_RESULT_TYPE)
                {
                    continue;
                }
            }
            for (table_iter = 0; table_iter < table->multi_res_info[result_type].nof_result_fields; table_iter++)
            {
                if (curr_l2p_info->field_id == table->multi_res_info[result_type].results_info[table_iter].field_id)
                {
                    field_pos = table_iter;
                    break;
                }
            }
            if (table_iter == table->multi_res_info[result_type].nof_result_fields)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "field not found in table %s\n",
                             dbal_field_to_string(unit, curr_l2p_info->field_id));
            }
        }
        else
        {
            if (!entry_handle->get_all_fields)
            {
                if (entry_handle->value_field_ids[field_pos] == DBAL_FIELD_EMPTY)
                {
                    continue;
                }
            }
        }

        SHR_IF_ERR_EXIT(dbal_direct_condition_check(unit, entry_handle, key, curr_l2p_info->mapping_condition,
                                                    &is_conditaion_pass));
        if (is_conditaion_pass)
        {
            /** calculate the field offset */
            field_offset = 0;
            if (curr_l2p_info->data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key, 
                                                         &(curr_l2p_info->data_offset_info),&field_offset));
            }

            

            /** calculate the register array offset */

            reg_array_offset = 0;
            if(curr_l2p_info->array_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key,
                                                              &(curr_l2p_info->array_offset_info),&reg_array_offset));
            }
            
            if (last_register_used != curr_l2p_info->reg[0])
            {
                last_register_used = curr_l2p_info->reg[0];
                last_reg_array_offset = reg_array_offset;

                sal_memset(data, 0x0, SOC_REG_ABOVE_64_MAX_SIZE_U32 * sizeof(uint32));
                SHR_IF_ERR_EXIT(soc_reg_above_64_get(unit, last_register_used, block, last_reg_array_offset, data));
                DBAL_REG_READ_LOG(last_register_used, 0, block, last_reg_array_offset);
            }

            {
                uint32 field_val[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };

                if (curr_l2p_info->hw_field != INVALIDf)
                {
                    soc_reg_above_64_field_get(unit, last_register_used, data, curr_l2p_info->hw_field, field_val);

                    LOG_INFO_EX(BSL_LOG_MODULE, "read field %s value 0x%x %s%s\n",
                                SOC_FIELD_NAME(unit, curr_l2p_info->hw_field), field_val[0], EMPTY, EMPTY);

                    if (field_offset != 0)
                    {
                        uint32 tmp_field_val[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };

                        SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(field_val,
                                                                       field_offset,
                                                                       (uint32) (table->multi_res_info
                                                                                 [result_type].results_info
                                                                                 [field_pos].field_nof_bits),
                                                                       tmp_field_val));

                        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(TRUE, bslSeverityVerbose);
                        LOG_VERBOSE_EX(BSL_LOG_MODULE,
                                       "Taking part of the field orig val %x, offset =%d, received val = %x%s\n",
                                       tmp_field_val[0],
                                       table->multi_res_info[result_type].results_info[field_pos].bits_offset_in_buffer,
                                       tmp_field_val[0], EMPTY);
                        DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(FALSE, bslSeverityVerbose);

                        sal_memcpy(field_val, tmp_field_val, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);
                    }

                    SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(field_val,
                                                                   (curr_l2p_info->offset_in_interface),
                                                                   (curr_l2p_info->nof_bits_in_interface),
                                                                   (uint32 *) (entry_handle->phy_entry.payload)));
                }
                else
                {
                    if (field_offset != 0)/** data offset */
                    {
                        uint32 data_aligend[SOC_REG_ABOVE_64_MAX_SIZE_U32] = { 0 };
                        SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(data, field_offset,
                                                                       (curr_l2p_info->nof_bits_in_interface),
                                                                       data_aligend));

                        SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(data_aligend,
                                                                       (curr_l2p_info->offset_in_interface),
                                                                       (curr_l2p_info->nof_bits_in_interface),
                                                                       (uint32 *) (entry_handle->phy_entry.payload)));
                    }
                    else
                    {
                        SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(data,
                                                                       (curr_l2p_info->offset_in_interface),
                                                                       (curr_l2p_info->nof_bits_in_interface),
                                                                       (uint32 *) (entry_handle->phy_entry.payload)));
                    }
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_direct_register_set(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int iter;
    soc_reg_t last_register_used = INVALIDr;
    uint32 reg_array_offset = 0, last_reg_array_offset = 0, field_offset = 0;
    uint8 is_conditaion_pass = 0;
    uint32 field_data[DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_WORDS] = { 0 };
    uint32 data[SOC_REG_ABOVE_64_MAX_SIZE_U32] = { 0 };
    uint32 key = entry_handle->phy_entry.key[0];
    uint32 core_id;
    int block = REG_PORT_ANY;
    dbal_logical_table_t *table = entry_handle->table;
    dbal_direct_l2p_info_t *l2p_direct_info;
    dbal_direct_l2p_field_info_t *curr_l2p_info;
    int result_type = entry_handle->cur_res_type;

    SHR_FUNC_INIT_VARS(unit);

    l2p_direct_info = &table->hl_mapping_multi_res[result_type].l2p_direct_info[DBAL_HL_ACCESS_REGISTER];

    core_id = DBAL_CORE_ANY;
    if (table->core_mode == DBAL_CORE_BY_INPUT)
    {
        if (entry_handle->core_id != DBAL_CORE_NOT_INTIATED)
        {
            core_id = entry_handle->core_id;
        }

        if (core_id != DBAL_CORE_ANY)
        {
            block = core_id;
        }
    }

    for (iter = 0; iter < l2p_direct_info->num_of_access_fields; iter++)
    {
        int field_pos;
        curr_l2p_info = &l2p_direct_info->l2p_fields_info[iter];

        field_pos = curr_l2p_info->field_pos_in_interface;
        if (entry_handle->value_field_ids[field_pos] == DBAL_FIELD_EMPTY)
        {/** field not requested */
            continue;
        }

        sal_memset(field_data, 0x0, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);

        SHR_IF_ERR_EXIT(dbal_direct_condition_check(unit, entry_handle, key, curr_l2p_info->mapping_condition,
                                                    &is_conditaion_pass));
        if (is_conditaion_pass)
        {
            /** calculate the field offset */
            field_offset = 0;            
            if (curr_l2p_info->data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key,
                                                          &(curr_l2p_info->data_offset_info),&field_offset));
            }            

            /** calculate the register array offset */

            reg_array_offset = 0;
            if(curr_l2p_info->array_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
            {
                SHR_IF_ERR_EXIT(dbal_direct_offset_calculate(unit, entry_handle, key,
                                                              &(curr_l2p_info->array_offset_info),&reg_array_offset));
            }
            
            if (((last_register_used != curr_l2p_info->reg[0])) || (reg_array_offset != last_reg_array_offset))
            {
                if (last_register_used != INVALIDr)
                {
                    SHR_IF_ERR_EXIT(soc_reg_above_64_set(unit, last_register_used, block, last_reg_array_offset, data));
                    DBAL_REG_WRITE_LOG(last_register_used, 0, block, last_reg_array_offset);
                }

                sal_memset(data, 0x0, SOC_REG_ABOVE_64_MAX_SIZE_U32 * sizeof(uint32));
                last_register_used = curr_l2p_info->reg[0];
                last_reg_array_offset = reg_array_offset;
                SHR_IF_ERR_EXIT(soc_reg_above_64_get(unit, last_register_used, block, last_reg_array_offset, data));
                DBAL_REG_READ_LOG(last_register_used, 0, block, last_reg_array_offset);
            }

            /*
             * handling partial SW field use 
             */
            SHR_IF_ERR_EXIT(utilex_bitstream_get_any_field(entry_handle->phy_entry.payload,
                                                           curr_l2p_info->offset_in_interface,
                                                           curr_l2p_info->nof_bits_in_interface, field_data));

            if (curr_l2p_info->hw_field != INVALIDf)
            {
                if (curr_l2p_info->data_offset_info.encode_mode != DBAL_VALUE_OFFSET_ENCODE_NONE)
                {
                    uint32 field_val[SOC_REG_ABOVE_64_MAX_SIZE_U32] = { 0 };
                    /*
                     * if working on data (field not exists) no need to read the field data
                     */
                    if (curr_l2p_info->hw_field != INVALIDf)
                    {
                        soc_reg_above_64_field_get(unit, last_register_used, data, curr_l2p_info->hw_field, field_val);

                        SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(field_data, field_offset,
                                                                       table->multi_res_info
                                                                       [result_type].results_info
                                                                       [curr_l2p_info->
                                                                        field_pos_in_interface].field_nof_bits,
                                                                       field_val));

                        sal_memcpy(field_data, field_val, DBAL_DIRECT_MAX_MEMORY_LINE_SIZE_IN_BYTES);
                    }
                }

                soc_reg_above_64_field_set(unit, last_register_used, data, curr_l2p_info->hw_field, field_data);

            }
            else
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_set_any_field(field_data, field_offset,
                                                               table->multi_res_info
                                                               [result_type].results_info
                                                               [curr_l2p_info->field_pos_in_interface].field_nof_bits,
                                                               data));

            }

            LOG_INFO_EX(BSL_LOG_MODULE, "write field %s, field offset %d, value 0x%x (%u) \n",
                        SOC_FIELD_NAME(unit, curr_l2p_info->hw_field), field_offset, field_data[0], field_data[0]);
        }
    }

    if (last_register_used != INVALIDr)
    {
        SHR_IF_ERR_EXIT(soc_reg_above_64_set(unit, last_register_used, block, last_reg_array_offset, data));
        DBAL_REG_WRITE_LOG(last_register_used, 0, block, last_reg_array_offset);
    }

exit:
    SHR_FUNC_EXIT;
}

/** this method uses the regular access function to resolve the result type in case of multiple result type tables
 *  it is called to the HW with the relevant result type when finished, the entry_handle->result_type should be updated */
shr_error_e
dbal_direct_result_type_resolution(
    int unit,
    dbal_entry_handle_t * entry_handle,
    int *result_type_resolved)
{
    int iter, ii;
    int curr_res_type = 0;
    uint32 field_value[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS] = { 0 };
    dbal_logical_table_t *table = entry_handle->table;
    dbal_table_field_info_t table_field_info;

    SHR_FUNC_INIT_VARS(unit);

    (*result_type_resolved) = 0;

    while (((*result_type_resolved) == 0) && (curr_res_type < table->nof_result_types))
    {
        for (iter = 0; iter < DBAL_NOF_HL_ACCESS_TYPES; iter++)
        {
            if (table->hl_mapping_multi_res[curr_res_type].l2p_direct_info[iter].num_of_access_fields != 0)
            {
                switch (iter)
                {
                    case DBAL_HL_ACCESS_MEMORY:
                        SHR_IF_ERR_EXIT(dbal_direct_memory_get(unit, entry_handle, &curr_res_type));
                        break;
                    case DBAL_HL_ACCESS_REGISTER:
                        SHR_IF_ERR_EXIT(dbal_direct_register_get(unit, entry_handle, &curr_res_type));
                        break;
                    case DBAL_HL_ACCESS_SW:
                         SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_get(unit, entry_handle, &curr_res_type));
                        break;
                    case DBAL_HL_ACCESS_PEMLA:
                    default:
                        continue;
                }
            }
        }
        SHR_IF_ERR_EXIT(dbal_table_field_info_get(unit, entry_handle->table_id,
                                                  DBAL_FIELD_RESULT_TYPE, 0, curr_res_type, &table_field_info));

        SHR_IF_ERR_EXIT(dbal_field_from_buffer_get(unit, &table_field_info, DBAL_FIELD_RESULT_TYPE,
                                                   entry_handle->phy_entry.payload, field_value));

        for (ii = 0; ii < entry_handle->table->nof_result_types; ii++)
        {
            if (entry_handle->table->multi_res_info[ii].result_type_hw_value == field_value[0])
            {
                /** result type found  */
                (*result_type_resolved) = 1;
                entry_handle->cur_res_type = ii;
                break;
            }
            else
            {
                if (entry_handle->table->unify_res_mapping)
                {/** in this case has to find the result type in the first iteration*/
                    SHR_EXIT();
                }
            }
        }
        curr_res_type++;
    }

exit:
    SHR_FUNC_EXIT;
}

/** 
 *  get the entry from the HW. fill the entry handle buffer according to the requested key.
 *  in case that must_res_type_resolved==1  will return error if result type was not resolved   */
shr_error_e
dbal_direct_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    uint8 must_res_type_resolved)
{
    int iter, result_type_resolved = 0;
    uint8 has_configuration = FALSE;
    dbal_logical_table_t *table = entry_handle->table;

    SHR_FUNC_INIT_VARS(unit);

    if (entry_handle->phy_entry.key_size > 32)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Key is bigger than uint32 %d missing implementation\n",
                     entry_handle->phy_entry.key_size);
    }

    if (entry_handle->nof_result_types > 1)
    {
        /** multiple result resolve the result type first */
        SHR_IF_ERR_EXIT(dbal_direct_result_type_resolution(unit, entry_handle, &result_type_resolved));
        if (result_type_resolved == 0)
        {
            if (must_res_type_resolved == 1)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "result type not resolved!!! table %s\n", table->table_name);

            }
            SHR_EXIT();
        }
    }

    for (iter = 0; iter < DBAL_NOF_HL_ACCESS_TYPES; iter++)
    {
        if (table->hl_mapping_multi_res[entry_handle->cur_res_type].l2p_direct_info[iter].num_of_access_fields != 0)
        {
            has_configuration = TRUE;
            switch (iter)
            {
                case DBAL_HL_ACCESS_MEMORY:
                    SHR_IF_ERR_EXIT(dbal_direct_memory_get(unit, entry_handle, NULL));
                    break;
                case DBAL_HL_ACCESS_REGISTER:
                    SHR_IF_ERR_EXIT(dbal_direct_register_get(unit, entry_handle, NULL));
                    break;
                case DBAL_HL_ACCESS_SW:
                    SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_get(unit, entry_handle, NULL));
                    break;
                case DBAL_HL_ACCESS_PEMLA:
                default:
                    continue;
            }
        }
    }

    if (!has_configuration)
    {
        LOG_CLI((BSL_META("Missing configuration for logical 2 physical in table %s for direct access \n"),
                 table->table_name));
    }

exit:
    SHR_FUNC_EXIT;
}

/** 
 *  set an entry in HW. access the HW according to the key fields set and updates the value fields. access memories register and SW in
 *  this order */
shr_error_e
dbal_direct_entry_set(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int iter;
    uint8 has_configuration = FALSE;
    dbal_logical_table_t *table = entry_handle->table;

    SHR_FUNC_INIT_VARS(unit);

    if (entry_handle->phy_entry.key_size > 32)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Key is bigger than uint32 %d missing implementation\n",
                     entry_handle->phy_entry.key_size);
    }

    for (iter = 0; iter < DBAL_NOF_HL_ACCESS_TYPES; iter++)
    {
        if (table->hl_mapping_multi_res[entry_handle->cur_res_type].l2p_direct_info[iter].num_of_access_fields != 0)
        {
            has_configuration = TRUE;
            switch (iter)
            {
                case DBAL_HL_ACCESS_MEMORY:
                    SHR_IF_ERR_EXIT(dbal_direct_memory_set(unit, entry_handle));
                    break;
                case DBAL_HL_ACCESS_REGISTER:
                    SHR_IF_ERR_EXIT(dbal_direct_register_set(unit, entry_handle));
                    break;
                case DBAL_HL_ACCESS_SW:
                    SHR_IF_ERR_EXIT(dbal_sw_table_direct_entry_set(unit, entry_handle));
                case DBAL_HL_ACCESS_PEMLA:
                default:
                    continue;
            }
        }
    }

    if (!has_configuration)
    {
        LOG_CLI((BSL_META("Missing configuration for logical 2 physical in table %s for direct access \n"),
                 table->table_name));
    }

exit:
    SHR_FUNC_EXIT;
}


shr_error_e
dbal_direct_entry_set_default(
    int unit,
    dbal_entry_handle_t * entry_handle)
{
    int iter;
    dbal_table_field_info_t *fields_info;
    int nof_result_types, res_type_idx;
    int nof_result_fields;
    dbal_entry_handle_t *default_entry_handle;

    SHR_FUNC_INIT_VARS(unit);

    dbal_entry_handle_default_get_internal(unit, &default_entry_handle);

    *default_entry_handle = *entry_handle;
    SHR_IF_ERR_EXIT(dbal_table_default_entry_get
                    (unit, entry_handle->table_id, default_entry_handle->phy_entry.payload));

    nof_result_types = entry_handle->table->nof_result_types;
    for (res_type_idx = 0; res_type_idx < nof_result_types; res_type_idx++)
    {
        nof_result_fields = entry_handle->table->multi_res_info[res_type_idx].nof_result_fields;
        fields_info = entry_handle->table->multi_res_info[res_type_idx].results_info;
        default_entry_handle->phy_entry.payload_size =
            entry_handle->table->multi_res_info[res_type_idx].entry_payload_size;
        for (iter = 0; iter < nof_result_fields; iter++)
        {
            default_entry_handle->value_field_ids[iter] = fields_info[iter].field_id;
        }
        default_entry_handle->nof_result_fields = nof_result_fields;
        default_entry_handle->num_of_fields = default_entry_handle->nof_key_fields + nof_result_fields;
        default_entry_handle->cur_res_type = res_type_idx;
        SHR_IF_ERR_EXIT(dbal_direct_entry_set(unit, default_entry_handle));
    }

exit:
    sal_memset(default_entry_handle, 0x0, sizeof(dbal_entry_handle_t));
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_direct_table_clear(
    int unit,
    uint32 entry_handle_id)
{
    int ii;
    int nof_key_fields = 0;
    int key_size = 0;

    uint32 max_entry_index[DBAL_PHYSICAL_KEY_SIZE_IN_WORDS] = { 0 };

    uint8 set_all_table = FALSE;
    uint8 is_valid_entry = FALSE;

    dbal_field_data_t *keys_info = NULL;
    dbal_entry_handle_t *entry_handle;
    dbal_logical_table_t *table;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle));
    table = entry_handle->table;

    nof_key_fields = table->nof_key_fields;

    SHR_ALLOC_SET_ZERO(keys_info, sizeof(dbal_field_data_t) * nof_key_fields, "table_clear_key_fields",
                       "%s%s%s\r\n", EMPTY, EMPTY, EMPTY);

    for (ii = 0; ii < nof_key_fields; ii++)
    {
        entry_handle->key_field_ids[ii] = table->keys_info[ii].field_id;
        if ((table->keys_info[ii].field_id == DBAL_FIELD_CORE_ID) && (table->core_mode == DBAL_CORE_BY_INPUT))
        {
            /*
             * set the iterator with the max core index 
             * than iterate it until it reached < 0
             */
            entry_handle->core_id = (1 << table->keys_info[ii].field_nof_bits) - 1;
        }
        else
        {
            key_size += table->keys_info[ii].field_nof_bits;
        }
    }

    entry_handle->nof_key_fields = table->nof_key_fields;
    entry_handle->num_of_fields = table->nof_key_fields;
    if (key_size != 0)
    {
        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(entry_handle->phy_entry.k_mask, 0, key_size - 1));
    }
    entry_handle->phy_entry.key_size = key_size;

    if (key_size < 32)
    {
        max_entry_index[0] = (1 << key_size) - 1;
    }

    else if (key_size == 32)
    {
        max_entry_index[0] = UTILEX_U32_MAX;
    }
    else
    {
        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(max_entry_index, 0, key_size - 1));
    }

    while (!set_all_table)
    {
        SHR_IF_ERR_EXIT(dbal_set_key_info_from_buffer(unit, entry_handle, keys_info, &is_valid_entry));
        if (is_valid_entry)
        {
            SHR_IF_ERR_EXIT(dbal_direct_entry_set_default(unit, entry_handle));
        }

        if (key_size <= 32)
        {
          /** key is up to 32b  */
            if (entry_handle->phy_entry.key[0] == max_entry_index[0])
            {
                entry_handle->core_id--;
                if (entry_handle->core_id < 0)
                {
                    set_all_table = TRUE;
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
        else
        {
          /** key is larger than 32b  */
            if (sal_memcmp(entry_handle->phy_entry.key, max_entry_index, sizeof(entry_handle->phy_entry.key)) == 0)
            {
                entry_handle->core_id--;
                if (entry_handle->core_id < 0)
                {
                    set_all_table = TRUE;
                }
                else
                {
                    sal_memset(entry_handle->phy_entry.key, 0, sizeof(entry_handle->phy_entry.key));
                }
            }
            else
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_add_one(entry_handle->phy_entry.key, BITS2WORDS(key_size)));
            }
        }
    }

exit:
    SHR_FREE(keys_info);
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_direct_table_iterator_init(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    uint32 key_size;

    SHR_FUNC_INIT_VARS(unit);

    key_size = iterator_info->entry_handle->phy_entry.key_size;

    iterator_info->direct_iterator.iterated_all_entries = FALSE;
    iterator_info->direct_iterator.key_size_in_words = BITS2WORDS(key_size);

    if (key_size < 32)
    {
        iterator_info->direct_iterator.max_num_of_iterations[0] = (1 << key_size) - 1;
    }
    else if (key_size == 32)
    {
        iterator_info->direct_iterator.max_num_of_iterations[0] = UTILEX_U32_MAX;
    }
    else
    {
        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(iterator_info->direct_iterator.max_num_of_iterations, 0,
                                                       key_size - 1));
    }

    SHR_IF_ERR_EXIT(dbal_table_default_entry_get
                    (unit, iterator_info->table_id, iterator_info->direct_iterator.default_entry));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_direct_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    int rv;
    uint8 is_valid_entry = FALSE;
    uint8 entry_found = FALSE;
    dbal_entry_handle_t *entry_handle;
    dbal_direct_table_iterator_t *direct_iterator;

    SHR_FUNC_INIT_VARS(unit);

    entry_handle = iterator_info->entry_handle;
    direct_iterator = &iterator_info->direct_iterator;

    while ((!entry_found) && (!direct_iterator->iterated_all_entries))
    {
        SHR_IF_ERR_EXIT(dbal_set_key_info_from_buffer(unit, entry_handle, iterator_info->keys_info, &is_valid_entry));
        if (is_valid_entry)
        {
            if (entry_handle->table->nof_result_types > 1)
            {
                iterator_info->entry_handle->get_all_fields = TRUE;
                iterator_info->entry_handle->cur_res_type = DBAL_RESULT_TYPE_NOT_INITIALIZED;
            }
            rv = dbal_direct_entry_get(unit, entry_handle, 0);

            if (rv == _SHR_E_NONE)
            {
                /*
                 * if the entry is not equals to the default entry it is returned 
                 */
                if (sal_memcmp(iterator_info->direct_iterator.default_entry,
                               entry_handle->phy_entry.payload, DBAL_PHYSICAL_RES_SIZE_IN_WORDS * sizeof(uint32)) != 0)
                {
                    entry_found = TRUE;
                }
            }
            else if (rv != _SHR_E_NOT_FOUND)
            {
                SHR_ERR_EXIT(rv, "direct entry get");
            }
        }

        /*
         * Check whether all entreis were iterated. 
         * set the next entry index to check
         */
        if (direct_iterator->key_size_in_words == 1)
        {
            /** key is up to 32b  */
            if (entry_handle->phy_entry.key[0] == direct_iterator->max_num_of_iterations[0])
            {
                entry_handle->core_id--;
                if (entry_handle->core_id < 0)
                {
                    direct_iterator->iterated_all_entries = TRUE;
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
        else
        {
            /** key is larger than 32b  */
            if (sal_memcmp(entry_handle->phy_entry.key, direct_iterator->max_num_of_iterations,
                           sizeof(entry_handle->phy_entry.key)) == 0)
            {
                entry_handle->core_id--;
                if (entry_handle->core_id < 0)
                {
                    direct_iterator->iterated_all_entries = TRUE;
                }
                else
                {
                    sal_memset(entry_handle->phy_entry.key, 0, sizeof(entry_handle->phy_entry.key));
                }
            }
            else
            {
                SHR_IF_ERR_EXIT(utilex_bitstream_add_one(entry_handle->phy_entry.key,
                                                         direct_iterator->key_size_in_words));
            }
        }
    }

    if (entry_found == FALSE)
    {
        iterator_info->is_end = TRUE;
    }

exit:
    SHR_FUNC_EXIT;
}
