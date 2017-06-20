/**
 * \file dbal_db_init.c
 * $Id$
 *
 * Main functions for init the dbal fields and logical tables DB
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DBALDNX

/*************
 * INCLUDES  *
 *************/
#include <sal/appl/sal.h>
#include <shared/bsl.h>
#include <include/soc/dnx/dnx_data/dnx_data.h>
#include <include/shared/utilex/utilex_integer_arithmetic.h>
#include <soc/mcm/memregs.h>
#include <soc/dnx/mdb.h>
#include "dbal_internal.h"

/*************
 *  DEFINES  *
 *************/
#define DBAL_DB_INVALID         (-1)

/*************
 *  GLOBALS  *
 *************/
/**
 * Delimiters for labels list
 */
static char *label_delimiters = "\n\t ";

/**
 * Delimiters for dnx data indication
 */
static char *dnx_data_delimiters = ".";

/**
 * XML files list
 */
char *xml_files_list[DBAL_TABLES_NOF_OPEN_XMLS];

/*************
 * FUNCTIONS *
 *************/
/** ***********************************************************
* \brief
* The function init the xml files list and init it to EMPTY string 
* ************************************************************* */
static shr_error_e
dbal_db_file_list_init(
    int unit)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_TABLES_NOF_OPEN_XMLS; ii++)
    {
        SHR_ALLOC(xml_files_list[ii], RHFILE_MAX_SIZE, "xml_files_list", "%s%s%s\r\n", EMPTY, EMPTY, EMPTY);
        sal_strcpy(xml_files_list[ii], EMPTY);
    }

exit:
    SHR_FUNC_EXIT;
}

/** ***********************************************************
* \brief
* The function free the allocated memory for xml files list 
* ************************************************************* */
static void
dbal_db_file_list_deinit(
    int unit)
{
    int ii;
    for (ii = 0; ii < DBAL_TABLES_NOF_OPEN_XMLS; ii++)
    {
        SHR_FREE(xml_files_list[ii]);
    }
}

/** ***********************************************************
* \brief
* The function parse the read DNX DATA reference from xml to 
* dnx data components
* ************************************************************* */
static shr_error_e
dbal_db_parse_dnx_data(
    int unit,
    dbal_db_dnx_data_info_struct_t * dnx_data)
{
    char *dnx_data_label;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * first word is DNX_DATA - skip it
     */
    dnx_data_label = sal_strtok(dnx_data->full_map, dnx_data_delimiters);

    /*
     * second word is module
     */
    dnx_data_label = sal_strtok(NULL, dnx_data_delimiters);
    if (dnx_data_label != NULL)
    {
        sal_strncpy(dnx_data->module, dnx_data_label, sizeof(dnx_data->module) - 1);
        dnx_data->module[sizeof(dnx_data->module) - 1] = '\0';
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "dnx data do not include module:%s\n", dnx_data->full_map);
    }

    /*
     * third word is submodule
     */
    dnx_data_label = sal_strtok(NULL, dnx_data_delimiters);
    if (dnx_data_label != NULL)
    {
        sal_strncpy(dnx_data->submodule, dnx_data_label, sizeof(dnx_data->submodule) - 1);
        dnx_data->submodule[sizeof(dnx_data->submodule) - 1] = '\0';
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "dnx data do not include submodule:%s\n", dnx_data->full_map);
    }

    /*
     * fourth word is data
     */
    dnx_data_label = sal_strtok(NULL, dnx_data_delimiters);
    if (dnx_data_label != NULL)
    {
        sal_strncpy(dnx_data->data, dnx_data_label, sizeof(dnx_data->data) - 1);
        dnx_data->data[sizeof(dnx_data->data) - 1] = '\0';
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "dnx data do not include data:%s\n", dnx_data->full_map);
    }

    /*
     * fifth word (optional) member
     */
    dnx_data_label = sal_strtok(NULL, dnx_data_delimiters);
    if (dnx_data_label != NULL)
    {
        sal_strncpy(dnx_data->member, dnx_data_label, sizeof(dnx_data->member) - 1);
        dnx_data->member[sizeof(dnx_data->member) - 1] = '\0';
    }

exit:
    SHR_FUNC_EXIT;
}

/** ***********************************************************
* \brief
* The function gets a string read from XML and check if it has
* dnx data indication. 
* If it has - the indication will be parsed. 
* ************************************************************* */
static shr_error_e
dbal_db_get_dnx_data_indication(
    int unit,
    dbal_db_dnx_data_info_struct_t * dnx_data)
{
    SHR_FUNC_INIT_VARS(unit);

    if (sal_strstr(dnx_data->full_map, DB_INIT_DNX_DATA_INDICATION) != NULL)
    {
        dnx_data->indication = TRUE;
        SHR_IF_ERR_EXIT(dbal_db_parse_dnx_data(unit, dnx_data));
    }
    else
    {
        dnx_data->indication = FALSE;
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function calls the dnx_data general API
* ***************************************************** */
static shr_error_e
dbal_db_dnx_data_get_value(
    int unit,
    dbal_db_dnx_data_info_struct_t * dnx_data,
    uint32 * return_value)
{
    const uint32 *data_from_dnx;

    SHR_FUNC_INIT_VARS(unit);

    data_from_dnx = dnx_data_utils_generic_data_get(unit, dnx_data->module, dnx_data->submodule,
                                                    dnx_data->data, dnx_data->member);
    if (data_from_dnx == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "DNX_DATA data cannot be found\n"
                     "module: %s, submodule: %s, data: %s, member: %s\n",
                     dnx_data->module, dnx_data->submodule, dnx_data->data,
                     (dnx_data->member == NULL) ? "" : dnx_data->member);
    }
    *return_value = *data_from_dnx;

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function check if 2 have common field in its definition. 
* (fieldID and child fields) 
* ***************************************************** */
static shr_error_e
dbal_db_check_field_duplication(
    int unit,
    dbal_fields_e field_1,
    dbal_fields_e field_2,
    uint8 *is_duplicated)
{
    int ii, jj;
    dbal_field_basic_info_t * field_1_info;
    dbal_field_basic_info_t * field_2_info;

    SHR_FUNC_INIT_VARS(unit);
    
    SHR_IF_ERR_EXIT(dbal_fields_field_info_get_ptr(unit, field_1, &field_1_info));
    SHR_IF_ERR_EXIT(dbal_fields_field_info_get_ptr(unit, field_2, &field_2_info));

    /** check that the fields are different */
    if (field_1 == field_2)
    {
        *is_duplicated = TRUE;
        SHR_EXIT();
    }

    /** check that the field2 is not child field of field1 */
    for (ii = 0; ii < field_1_info->nof_child_fields; ii++)
    {
        if (field_1_info->sub_field_info[ii].sub_field_id == field_2)
        {
            *is_duplicated = TRUE;
            SHR_EXIT();
        }
    }

    /** check that the field1 is not child field of field2 */
    for (ii = 0; ii < field_2_info->nof_child_fields; ii++)
    {
        if (field_2_info->sub_field_info[ii].sub_field_id == field_1)
        {
            *is_duplicated = TRUE;
            SHR_EXIT();
        }
    }

    /** check that all child fields of field1 are not child
     *  field of field2 */
    for (ii = 0; ii < field_1_info->nof_child_fields; ii++)
    {
        for (jj = 0; jj < field_2_info->nof_child_fields; jj++)
        {
            if (field_1_info->sub_field_info[ii].sub_field_id == field_2_info->sub_field_info[jj].sub_field_id)
            {
                *is_duplicated = TRUE;
                SHR_EXIT();
            }
        }
    }

    *is_duplicated = FALSE;

exit:
    SHR_FUNC_EXIT;
}
/** ****************************************************
* \brief
* The function parse and adds to table offset parameters read 
* from XML 
* ***************************************************** */
static shr_error_e
dbal_db_init_condition_parsing(
    int unit,
    dbal_tables_e table_id,
    dbal_access_condition_info_t * entry_condition,
    table_db_access_condition_struct_t * condition_params)
{
    SHR_FUNC_INIT_VARS(unit);

    if (sal_strcmp(condition_params->type, EMPTY) != 0)
    {
        SHR_IF_ERR_EXIT(dbal_condition_string_to_id(unit, condition_params->type, &entry_condition->type));
        if (sal_strcmp(condition_params->field, EMPTY) != 0)
        {
            uint8 is_key;
            SHR_IF_ERR_EXIT(dbal_field_string_to_id(unit, condition_params->field, &entry_condition->field_id));
            /** validate that the field exists in table's key info */
            SHR_IF_ERR_EXIT(dbal_table_field_is_key(unit, table_id, entry_condition->field_id, &is_key));
            if (!is_key)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL,
                             "mapping condition uses an invalid field:%s for table:%s\n",
                             dbal_field_to_string(unit, entry_condition->field_id),
                             dbal_logical_table_to_string(unit, table_id));
            }
        }
        if (sal_strcmp(condition_params->enum_val, EMPTY) != 0)
        {
            int ii;
            char enum_name[DBAL_MAX_STRING_LENGTH];
            dbal_field_basic_info_t * field_info = NULL;
            SHR_IF_ERR_EXIT(dbal_fields_field_info_get_ptr(unit,entry_condition->field_id, &field_info));

            sal_strcpy(enum_name, dbal_field_to_string(unit, entry_condition->field_id));
            sal_strcat(enum_name, "_");
            sal_strcat(enum_name, condition_params->enum_val);

            for (ii = 0; ii < field_info->nof_enum_values; ii++)
            {
                if (sal_strcmp(field_info->enum_val_info[ii].name, enum_name) == 0)
                {
                    entry_condition->value = field_info->enum_val_info[ii].value;
                    break;
                }
            }
            if (ii == field_info->nof_enum_values)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL,
                             "Mapping condition set with enum, but enun doesn't exist in field db. field:%s, enum:%s\n",
                             dbal_field_to_string(unit, entry_condition->field_id), condition_params->enum_val);
            }
        }
        else
        {
            entry_condition->value = condition_params->value;
        }

    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function parse and adds to table offset parameters read 
* from XML 
* ***************************************************** */
static shr_error_e
dbal_db_init_offset_parsing(
    int unit,
    dbal_tables_e table_id,
    dbal_offset_encode_info_t * entry_offset,
    table_db_offset_in_hw_struct_t * offset_params)
{
    SHR_FUNC_INIT_VARS(unit);

    if (sal_strcmp(offset_params->type, EMPTY) != 0)
    {
        SHR_IF_ERR_EXIT(dbal_offset_encode_type_string_to_id(unit, offset_params->type, &entry_offset->encode_mode));
        entry_offset->input_param = offset_params->value;
        if (sal_strcmp(offset_params->field, EMPTY) != 0)
        {
            uint8 is_key;
            SHR_IF_ERR_EXIT(dbal_field_string_to_id(unit, offset_params->field, &entry_offset->field_id));

            /** validate that the field exists in table's key info */
            SHR_IF_ERR_EXIT(dbal_table_field_is_key(unit, table_id, entry_offset->field_id, &is_key));
            if (!is_key)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL,
                             "Offset caclulation uses an invalid field:%s for table:%s\n",
                             dbal_field_to_string(unit, entry_offset->field_id),
                             dbal_logical_table_to_string(unit, table_id));
            }
        }


    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* Allocates sw table memory according to table and entry sizes
* ***************************************************** */
static shr_error_e
dbal_db_allocate_sw_table(
    int unit,
    dbal_tables_e table_id,
    int num_of_entries,
    int result_length_in_bytes)
{
    int ii;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.entries.alloc(unit, table_id, num_of_entries));
    for (ii = 0; ii < num_of_entries; ii++)
    {
        SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.entries.entry_buffer.alloc(unit, table_id, ii, result_length_in_bytes));
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds a field read from XML to the dbal field DB. 
* Main functionality is translating the string from XML to 
* corresponding values 
* ***************************************************** */
static shr_error_e
dbal_db_init_field_add(
    int unit,
    field_db_struct_t * field_params,
    dbal_field_basic_info_t * field_info)
{
    dbal_field_basic_info_t *field_entry;
    dbal_fields_e field_id;
    int label_index;
    int ii;
    char *label_token;
    uint8 is_zero_mapping_exists = FALSE;
    int max_value_for_enum_mapping = 0;

    SHR_FUNC_INIT_VARS(unit);

    /** find the field id according to its name */
    SHR_IF_ERR_EXIT(dbal_field_string_to_id(unit, field_params->name, &field_id));
    field_entry = field_info + field_id;

    /** Name - Printing Name */
    sal_strcpy(field_entry->name, field_params->name);

    /** Size */
    if (field_params->size_dnx_data.indication == TRUE)
    {
        /** Need to be read from DNX_DATA */
        dbal_db_dnx_data_get_value(unit, &field_params->size_dnx_data, &field_entry->max_size);
    }
    else
    {
        field_entry->max_size = field_params->size;
    }

    /** instances_support */
    field_entry->instances_support = field_params->instances_support;

    /** Type */
    SHR_IF_ERR_EXIT(dbal_field_type_string_to_id(unit, field_params->type, &field_entry->type));

    /** Labels */
    label_index = 0;
    label_token = sal_strtok(field_params->labels, label_delimiters);
    while (label_token != NULL)
    {
        SHR_IF_ERR_EXIT(dbal_label_string_to_id(unit, label_token, &field_entry->labels[label_index]));
        label_token = sal_strtok(NULL, label_delimiters);
        label_index++;
    }

    /** Default Value */
    field_entry->default_value = field_params->default_val;
    field_entry->is_default_value_valid = field_params->default_val_valid;

    /** Max Value */
    if (field_params->max_value != DBAL_DB_INVALID)
    {
        field_entry->max_value = field_params->max_value;
    }
    else
    {
        if (field_entry->max_size > SAL_UINT32_MAX_BIT)
        {
            field_entry->max_value = 0;
        }
        else
        {
            field_entry->max_value = utilex_power_of_2(field_entry->max_size) - 1;
        }
    }

    /** Encoding */
    if (sal_strcmp(field_params->encode_type, EMPTY) != 0)
    {
        SHR_IF_ERR_EXIT(dbal_field_encode_type_string_to_id(unit, field_params->encode_type,
                                                            &field_entry->encode_info.encode_mode));
    }
    field_entry->encode_info.input_param = field_params->encode_param1;

    /** In case of Enum type - set also enum encoding */
    if (field_entry->type == DBAL_FIELD_TYPE_ENUM)
    {
        field_entry->encode_info.encode_mode = DBAL_VALUE_FIELD_ENCODE_ENUM;
    }

    /** encoding validation, at the moment encoding only supported for fields less that 32bit */
    if (field_entry->encode_info.encode_mode != DBAL_VALUE_FIELD_ENCODE_NONE)
    {
        if (field_entry->max_size > 32)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL,
                         "Illegal field (%s) encoding, not supported for field bigger than 32 bit %d\n",
                         field_entry->name, field_entry->encode_info.encode_mode);
        }
    }

    /** child fields */
    field_entry->nof_child_fields = field_params->nof_childs;
    if (field_entry->nof_child_fields > 0)
    {
        SHR_ALLOC_SET_ZERO(field_entry->sub_field_info, field_entry->nof_child_fields * sizeof(dbal_sub_field_info_t),
                           "sub field info allocation", "%s%s%s\r\n", field_entry->name, EMPTY, EMPTY);
    }

    for (ii = 0; ii < field_params->nof_childs; ii++)
    {
        SHR_IF_ERR_EXIT(dbal_field_string_to_id(unit,
                                                field_params->childs[ii].name,
                                                &field_entry->sub_field_info[ii].sub_field_id));

        SHR_IF_ERR_EXIT(dbal_field_encode_type_string_to_id(unit,
                                                            field_params->childs[ii].encode_type,
                                                            &field_entry->sub_field_info[ii].encode_info.encode_mode));

        field_entry->sub_field_info[ii].encode_info.input_param = field_params->childs[ii].encode_param1;
    }

    /** enums */
    field_entry->nof_enum_values = field_params->nof_enum_vals;
    if (field_entry->nof_enum_values > 0)
    {
        SHR_ALLOC_SET_ZERO(field_entry->enum_val_info, field_entry->nof_enum_values * sizeof(dbal_enum_decoding_info_t),
                           "enum info allocation", "%s%s%s\r\n", field_entry->name, EMPTY, EMPTY);
    }

    max_value_for_enum_mapping = ((1 << field_params->size) - 1);
    for (ii = 0; ii < field_params->nof_enum_vals; ii++)
    {
        sal_strcpy(field_entry->enum_val_info[ii].name, field_params->name);
        sal_strcat(field_entry->enum_val_info[ii].name, "_");
        sal_strcat(field_entry->enum_val_info[ii].name, field_params->enums[ii].name_from_interface);
        field_entry->enum_val_info[ii].value = field_params->enums[ii].value_from_mapping;

        if (field_params->enums[ii].value_from_mapping > max_value_for_enum_mapping)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL,
                         "Illegal value for HW mapping in enum. field size is %d, hw value is %d. field %s\n",
                         field_params->size, field_params->enums[ii].value_from_mapping, field_entry->name);
        }
        if (field_params->enums[ii].value_from_mapping == 0)
        {
            is_zero_mapping_exists = TRUE;
        }
    }

    if ((!is_zero_mapping_exists) && (field_params->nof_enum_vals > 0))
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal value for HW mapping in enum. zero value must be in use. field %s\n",
                     field_entry->name);
    }

    if (field_entry->instances_support)
    {
        /** update the other instances of the field with the same info */
        for (ii = 1; ii < DBAL_NUM_OF_FIELD_INSTANCES; ii++)
        {
            sal_memcpy(field_entry + ii, field_entry, sizeof(dbal_field_basic_info_t));
            sal_sprintf((field_entry + ii)->name, "%s_%02d", field_entry->name, ii);

        }
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds the key+result fields read from XML to the 
* dbal table DB. 
* Read by : dbal_db_init_table_add_interface_to_table
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_fields_to_table(
    int unit,
    dbal_table_field_info_t * fields_info,
    table_db_struct_t * table_params,
    uint8 is_key_fields,
    int result_set_idx)
{
    int ii;
    int num_of_fields;
    dbal_fields_e field_id;
    table_db_field_params_struct_t *fields_read_data;
    int field_index = 0;
    uint32 field_def_size = 0;

    SHR_FUNC_INIT_VARS(unit);

    if (is_key_fields)
    {
        num_of_fields = table_params->nof_key_fields;
        fields_read_data = table_params->key_fields;
    }
    else
    {
        num_of_fields = table_params->results_set[result_set_idx].nof_res_fields;
        fields_read_data = table_params->results_set[result_set_idx].result_fields;
    }

    for (ii = 0; ii < num_of_fields; ii++)
    {
        SHR_IF_ERR_EXIT(dbal_field_string_to_id(unit, fields_read_data[ii].name, &field_id));
        fields_info[field_index].field_id = field_id;

        /**  get field max size (default size) */
        SHR_IF_ERR_EXIT(dbal_fields_max_size_get(unit, field_id, &field_def_size));

        /** Read field offset */
        fields_info[field_index].offset_in_logical_field = fields_read_data[ii].offset;

        /** Read field size */
        if (fields_read_data[ii].size_dnx_data.indication == TRUE)
        {
            /** Need to be read from DNX_DATA */
            uint32 field_size = 0;
            dbal_db_dnx_data_get_value(unit, &fields_read_data[ii].size_dnx_data, &field_size);

            fields_info[field_index].field_nof_bits = field_size;
        }
        else
        {
            if (fields_read_data[ii].size != 0)
            {
                fields_info[field_index].field_nof_bits = fields_read_data[ii].size;
            }
            else
            {
                fields_info[field_index].field_nof_bits = field_def_size;
            }
        }

        /*
         * validate that field size and offset are no longer than field 
         * default size
         */
        if ((fields_info[field_index].offset_in_logical_field + fields_info[field_index].field_nof_bits) >
            field_def_size)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "field %s is longer than its default max size. offset=%d, size=%d\n",
                         dbal_field_to_string(unit, fields_info[field_index].field_id),
                         fields_info[field_index].offset_in_logical_field, fields_info[field_index].field_nof_bits);
        }

        if (fields_read_data[ii].valid_dnx_data.indication == TRUE)
        {
            /** Need to be read from DNX_DATA */
            uint32 valid_from_dnx_data = 0;
            dbal_db_dnx_data_get_value(unit, &fields_read_data[ii].valid_dnx_data, &valid_from_dnx_data);

            if (valid_from_dnx_data == 0)
            {
                fields_info[field_index].field_nof_bits = 0;
            }
        }

        /** multiple instances support */
        fields_info[field_index].nof_instances = fields_read_data[ii].nof_instances;
        if (fields_read_data[ii].nof_instances > 1)
        {
            int jj;
            if (is_key_fields)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "not supported multiple instances for key fields. field %s\n",
                             dbal_field_to_string(unit, fields_info[field_index].field_id));
            }
            if (!dbal_fields_is_instances_support(unit, fields_info[field_index].field_id))
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "field %s not supports mutiple instances \n",
                             dbal_field_to_string(unit, fields_info[field_index].field_id));
            }
            for (jj = 1; jj < fields_info[field_index].nof_instances; jj++)
            {
                sal_memcpy(&(fields_info[field_index + jj]), &(fields_info[field_index]),
                           sizeof(dbal_table_field_info_t));
                fields_info[field_index + jj].field_id = fields_info[field_index].field_id + jj;
            }
            field_index += fields_info[field_index].nof_instances;
        }
        else
        {
            field_index++;
        }
    }
exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds the table interface part read from XML to 
* the dbal table DB. 
* The interface is common to all logical tables type 
* Read by : dbal_db_init_logical_table_add
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_interface_to_table(
    int unit,
    dbal_logical_table_t * table_entry,
    table_db_struct_t * table_params)
{
    int label_index;
    int ii, jj;
    char *label_token;

    SHR_FUNC_INIT_VARS(unit);

    /** Name - Printing Name */
    sal_strcpy(table_entry->table_name, table_params->name);

    /** Maturity Level */
    table_entry->maturity_level = table_params->maturity_level;

    /** Type */
    SHR_IF_ERR_EXIT(dbal_logical_table_type_string_to_id(unit, table_params->type, &table_entry->table_type));

    /** Validity */
    if (table_params->valid_dnx_data.indication == TRUE)
    {
        /** Need to be read from DNX_DATA */
        dbal_db_dnx_data_get_value(unit, &table_params->valid_dnx_data, &table_entry->is_table_valid);
    }
    else
    {
        table_entry->is_table_valid = 1;
    }

    /** Labels */
    label_index = 0;
    label_token = sal_strtok(table_params->labels, label_delimiters);
    while (label_token != NULL)
    {
        SHR_IF_ERR_EXIT(dbal_label_string_to_id(unit, label_token, &table_entry->labels[label_index]));
        label_token = sal_strtok(NULL, label_delimiters);
        label_index++;
    }

    /** Key Fields */
    table_entry->nof_key_fields = table_params->nof_key_fields;

    SHR_ALLOC_SET_ZERO(table_entry->keys_info, table_entry->nof_key_fields * sizeof(dbal_table_field_info_t),
                       "key fields info allocation", "%s%s%s\r\n", table_entry->table_name, EMPTY, EMPTY);

    SHR_IF_ERR_EXIT(dbal_db_init_table_add_fields_to_table(unit, table_entry->keys_info, table_params, TRUE, 0));

    /** for multiple results fields */
    table_entry->nof_result_types = table_params->num_of_results_sets;

    SHR_ALLOC_SET_ZERO(table_entry->multi_res_info, table_entry->nof_result_types * sizeof(multi_res_info_t),
                       "multiple results info allocation", "%s%s%s\r\n", table_entry->table_name, EMPTY, EMPTY);

    for (ii = 0; ii < table_params->num_of_results_sets; ii++)
    {
        /** Result Fields */
        int nof_results_field = 0;

        for (jj = 0; jj < table_params->results_set[ii].nof_res_fields; jj++)
        {
            if (table_params->results_set[ii].result_fields[jj].nof_instances > 1)
            {
                nof_results_field += table_params->results_set[ii].result_fields[jj].nof_instances;
            }
            else
            {
                nof_results_field++;
            }
        }

        /** Total Num of Fields */
        table_entry->multi_res_info[ii].nof_result_fields = nof_results_field;
        table_entry->multi_res_info[ii].results_info = NULL;
        SHR_ALLOC_SET_ZERO(table_entry->multi_res_info[ii].results_info, nof_results_field * sizeof(multi_res_info_t),
                           "results fields info allocation", "%s%s%s\r\n", table_entry->table_name, EMPTY, EMPTY);

        SHR_IF_ERR_EXIT(dbal_db_init_table_add_fields_to_table
                        (unit, table_entry->multi_res_info[ii].results_info, table_params, FALSE, ii));

        /** Result type HW value */
        table_entry->multi_res_info[ii].result_type_hw_value = table_params->results_set[ii].result_is_mapped ?
            table_params->results_set[ii].result_type_physical_value : ii;

        /** Result type HW value */
        sal_strcpy(table_entry->multi_res_info[ii].result_type_name, table_params->results_set[ii].result_type_name);

    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds the table physical mapping part Memory and 
* register access types of hard logic tables
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_hl_memory_register(
    int unit,
    dbal_tables_e table_id,
    dbal_direct_l2p_field_info_t * access_entry,
    table_db_access_params_struct_t * access_params)
{
    int ii;
    SHR_FUNC_INIT_VARS(unit);

    access_entry->access_nof_bits = access_params->access_size;
    access_entry->access_offset = access_params->access_offset;

    /** HW parameters */
    if (access_params->access_type == DBAL_HL_ACCESS_MEMORY)
    {
        SHR_IF_ERR_EXIT(dbal_hw_entity_string_to_id(unit, access_params->access_name, &access_entry->memory));
        /** Alias Memory */
        if (sal_strcmp(access_params->alias_name, EMPTY) != 0)
        {
            SHR_IF_ERR_EXIT(dbal_hw_entity_string_to_id(unit, access_params->alias_name, &access_entry->alias_memory));
        }
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_hw_entity_string_to_id(unit, access_params->access_name, &access_entry->reg[0]));
        /** Alias Register */
        if (sal_strcmp(access_params->alias_name, EMPTY) != 0)
        {
            SHR_IF_ERR_EXIT(dbal_hw_entity_string_to_id(unit, access_params->alias_name, &access_entry->alias_reg[0]));
        }
    }

    if (sal_strcmp(access_params->hw_field, EMPTY) != 0)
    {
        SHR_IF_ERR_EXIT(dbal_hw_entity_string_to_id(unit, access_params->hw_field, &access_entry->hw_field));
    }

    /** condition */
    for (ii = 0; ii < DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS; ii++)
    {
        SHR_IF_ERR_EXIT(dbal_db_init_condition_parsing(unit, table_id, &access_entry->mapping_condition[ii],
                                                       &access_params->access_condition[ii]));
    }

    /** offsets (Array, Entry, Data, block) */
    SHR_IF_ERR_EXIT(dbal_db_init_offset_parsing(unit, table_id, &access_entry->array_offset_info,
                                                &access_params->array_offset));
    SHR_IF_ERR_EXIT(dbal_db_init_offset_parsing(unit, table_id, &access_entry->block_index_info,
                                                &access_params->block_index));
    SHR_IF_ERR_EXIT(dbal_db_init_offset_parsing(unit, table_id, &access_entry->entry_offset_info,
                                                &access_params->entry_offset));
    SHR_IF_ERR_EXIT(dbal_db_init_offset_parsing(unit, table_id, &access_entry->data_offset_info,
                                                &access_params->data_offset));
    SHR_IF_ERR_EXIT(dbal_db_init_offset_parsing(unit, table_id, &access_entry->array_offset_info,
                                                &access_params->array_offset));
    SHR_IF_ERR_EXIT(dbal_db_init_offset_parsing(unit, table_id, &access_entry->alias_data_offset_info,
                                                &access_params->alias_data_offset));

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds the table physical mapping part sw access
* types of hard logic tables 
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_hl_sw_access(
    int unit,
    dbal_logical_table_t * table_entry,
    table_db_struct_t * table_params,
    int sw_legth_bytes)
{
    int ii, key_length, num_of_entries;
    dbal_tables_e table_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_logical_table_string_to_id(unit, table_params->name, &table_id));

    /** calculate key length */
    key_length = 0;
    for (ii = 0; ii < table_params->nof_key_fields; ii++)
    {
        key_length += table_params->key_fields[ii].size;
    }
    if (key_length > DBAL_SW_DIRECT_TABLES_MAX_KEY_SIZE_BITS)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Key is too long for sw state tables. max key size is %d bits. table %s\n",
                     DBAL_SW_DIRECT_TABLES_MAX_KEY_SIZE_BITS, table_entry->table_name);
    }

    /** set to table the result length  */
    table_entry->sw_payload_length_bytes = sw_legth_bytes;

    /** set table type to direct */
    SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.set(unit, table_id, DBAL_SW_TABLE_DIRECT));

    /** calculate num of entries - 2^key_size */
    num_of_entries = utilex_power_of_2(key_length);

    /** allocate the table entries buffers */
    SHR_IF_ERR_EXIT(dbal_db_allocate_sw_table(unit, table_id, num_of_entries, sw_legth_bytes));

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function calculate the total size to allocate to sw part 
* of access payload 
* ***************************************************** */
static int
dbal_db_init_table_hl_sw_add_length(
    int unit,
    table_db_struct_t * table_params,
    table_db_access_params_struct_t * access_params)
{
    int ii, jj;
    int max_length = 0, current_result_length = 0;

    /** calculate max result length */
    for (ii = 0; ii < table_params->num_of_results_sets; ii++)
    {
        current_result_length = 0;
        for (jj = 0; jj < table_params->results_set[ii].nof_res_fields; jj++)
        {
            if (sal_strcmp(access_params->access_field_name, table_params->results_set[ii].result_fields[jj].name) == 0)
            {
                current_result_length = BITS2BYTES(table_params->results_set[ii].result_fields[jj].size);
                break;
            }
        }
        max_length = UTILEX_MAX(current_result_length, max_length);
    }
    return max_length;
}

/** ****************************************************
* \brief
* The function adds the table physical mapping part of hard 
* logic type tables read from XML to the dbal table DB. 
* Read by : dbal_db_init_logical_table_add
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_mapping_hard_logic(
    int unit,
    dbal_tables_e table_id,
    dbal_logical_table_t * table_entry,
    table_db_struct_t * table_params,
    int result_idx,
    int acc_map_idx)
{
    int ii, jj;
    int max_sw_length_bytes = 0;

    dbal_direct_l2p_field_info_t *access_entry;
    dbal_hard_logic_access_types_e access_type;
    dbal_hl_mapping_multi_res_t *hl_mapping;
    uint8 has_sw_fields = FALSE;

    CONST static dbal_direct_l2p_field_info_t hl_default_mapping = {
        DBAL_FIELD_EMPTY,   /** field_id */
        0,                  /** access_nof_bits*/
        0,                  /** access_offset*/
        0,                  /** field_pos_in_interface*/
        0,                  /** nof_bits_in_interface*/
        0,                  /** offset_in_interface*/
        INVALIDm,           /** memory*/
        {INVALIDr},         /** reg[DBAL_MAX_NUMBER_OF_REGISTERS]*/
        INVALIDf,           /** hw_field*/
        {0},                /** array_offset_info*/
        {0},                /** entry_offset_info*/
        {0},                /** data_offset_info*/
        {0},                /** block_index_info*/
        INVALIDm,           /** alias_memory*/
        {INVALIDr},         /** alias_reg[DBAL_MAX_NUMBER_OF_REGISTERS]*/
        {0},                /** alias_data_offset_info*/
        {{0}}               /** mapping_condition[DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS]*/
    };

    SHR_FUNC_INIT_VARS(unit);

    table_entry->access_method = DBAL_ACCESS_METHOD_HARD_LOGIC;
    hl_mapping = &table_entry->hl_mapping_multi_res[result_idx];

    /** access layer */
    for (ii = 0; ii < table_params->nof_access[acc_map_idx]; ii++)
    {
        /** count the num of access for each access type, for dynamic allocation */
        access_type = table_params->access[acc_map_idx][ii].access_type;
        if (sal_strcmp(table_params->access[acc_map_idx][ii].access_name, "NONE") == 0)
        {
            continue;
        }
        hl_mapping->l2p_direct_info[access_type].num_of_access_fields++;
    }

    for (ii = 0; ii < DBAL_NOF_HL_ACCESS_TYPES; ii++)
    {
        /** Allocate the dynamic memory according to nof access */
        hl_mapping->l2p_direct_info[ii].l2p_fields_info = NULL;
        if (hl_mapping->l2p_direct_info[ii].num_of_access_fields > 0)
        {
            SHR_ALLOC(hl_mapping->l2p_direct_info[ii].l2p_fields_info,
                      hl_mapping->l2p_direct_info[ii].num_of_access_fields * sizeof(dbal_direct_l2p_field_info_t),
                      "HL Access allocation", "%s%s%s\r\n", table_entry->table_name, EMPTY, EMPTY);

            for (jj = 0; jj < hl_mapping->l2p_direct_info[ii].num_of_access_fields; jj++)
            {
                hl_mapping->l2p_direct_info[ii].l2p_fields_info[jj] = hl_default_mapping;
            }
        }

        /** Set the num of access back to zero, will be updated below */
        hl_mapping->l2p_direct_info[ii].num_of_access_fields = 0;
    }

    for (ii = 0; ii < table_params->nof_access[acc_map_idx]; ii++)
    {
        /** access type */
        access_type = table_params->access[acc_map_idx][ii].access_type;

        /** In case of None memory, it means that the access block is missing */
        if (sal_strcmp(table_params->access[acc_map_idx][ii].access_name, "NONE") == 0)
        {
            continue;
        }

        access_entry = hl_mapping->l2p_direct_info[access_type].l2p_fields_info +
            hl_mapping->l2p_direct_info[access_type].num_of_access_fields;

        hl_mapping->l2p_direct_info[access_type].num_of_access_fields++;

        /** the associated dbal field */
        SHR_IF_ERR_EXIT(dbal_field_string_to_id(unit, table_params->access[acc_map_idx][ii].access_field_name,
                                                &access_entry->field_id));

        switch (access_type)
        {
            case DBAL_HL_ACCESS_MEMORY:
            case DBAL_HL_ACCESS_REGISTER:
                SHR_IF_ERR_EXIT(dbal_db_init_table_add_hl_memory_register(unit, table_id, access_entry, 
                                                                          &table_params->access[acc_map_idx][ii]));
                break;

            case DBAL_HL_ACCESS_SW:
                has_sw_fields = TRUE;
                max_sw_length_bytes += dbal_db_init_table_hl_sw_add_length(unit, table_params,
                                                                           &table_params->access[acc_map_idx][ii]);
                break;

            case DBAL_HL_ACCESS_PEMLA:
            default:
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "access type is not supported yet.table:%s\n", table_entry->table_name);
                break;
        }
    }

    if (has_sw_fields)
    {
        SHR_IF_ERR_EXIT(dbal_db_init_table_add_hl_sw_access(unit, table_entry, table_params, max_sw_length_bytes));
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds the table physical mapping part of MDB type 
* tables read from XML to the dbal table DB. Read by : 
* dbal_db_init_logical_table_add 
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_mapping_mdb(
    int unit,
    dbal_logical_table_t * table_entry,
    table_db_struct_t * table_params)
{
    SHR_FUNC_INIT_VARS(unit);

    table_entry->access_method = DBAL_ACCESS_METHOD_PHY_TABLE;
    if (table_params->app_db_id != DBAL_DB_INVALID)
    {
        table_entry->app_id = table_params->app_db_id;
    }
    else
    {
        table_entry->app_id = UTILEX_U32_MAX;
    }
    SHR_IF_ERR_EXIT(dbal_physical_table_string_to_id(unit, table_params->phy_db, &table_entry->physical_db_id));

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds the table's physical mapping part for tables
* of type SW only. 
* read from XML to the dbal table DB. 
* called by : dbal_db_init_logical_table_add
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_mapping_sw_state(
    int unit,
    dbal_logical_table_t * table_entry,
    dbal_tables_e table_id,
    table_db_struct_t * table_params)
{
    int ii, jj;
    uint32 num_of_entries = 0;
    int key_length = 0;
    int current_result_length = 0, max_result_length_bytes = 0;
    dbal_table_type_e table_type;
    UTILEX_HASH_TABLE_PTR hash_tbl_id;
    UTILEX_HASH_TABLE_INIT_INFO hash_tbl_init_info;

    SHR_FUNC_INIT_VARS(unit);

    table_entry->access_method = DBAL_ACCESS_METHOD_SW_ONLY;
    SHR_IF_ERR_EXIT(dbal_logical_table_type_string_to_id(unit, table_params->type, &table_type));

    /** calculate key length */
    key_length = 0;
    for (ii = 0; ii < table_entry->nof_key_fields; ii++)
    {
        key_length += table_entry->keys_info[ii].field_nof_bits;
    }

    if (table_type == DBAL_TABLE_TYPE_DIRECT)
    {
        if (key_length > DBAL_SW_DIRECT_TABLES_MAX_KEY_SIZE_BITS)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "key is too long.  max key size for sw state DIRECT is %d bits. table %s\n",
                         DBAL_SW_DIRECT_TABLES_MAX_KEY_SIZE_BITS, table_entry->table_name);
        }
    }
    else
    {
        if (BITS2BYTES(key_length) > DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "key is too long.  max key size for sw state HASH is %d bytes. table %s\n",
                         DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES, table_entry->table_name);
        }
    }

    /** calculate max result length */
    for (ii = 0; ii < table_entry->nof_result_types; ii++)
    {
        current_result_length = 0;
        for (jj = 0; jj < table_entry->multi_res_info[ii].nof_result_fields; jj++)
        {
            current_result_length += BITS2BYTES(table_entry->multi_res_info[ii].results_info[jj].field_nof_bits);
        }
        max_result_length_bytes = UTILEX_MAX(max_result_length_bytes, current_result_length);
    }

    table_entry->sw_payload_length_bytes = max_result_length_bytes;

    switch (table_type)
    {
        case DBAL_TABLE_TYPE_EM:
        case DBAL_TABLE_TYPE_LPM:
        case DBAL_TABLE_TYPE_TCAM:
            SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.set(unit, table_id, DBAL_SW_TABLE_HASH));
            num_of_entries = table_params->sw_table_size;

            hash_tbl_init_info.prime_handle = 0;
            hash_tbl_init_info.sec_handle = 0;
            hash_tbl_init_info.table_size = num_of_entries;
            hash_tbl_init_info.key_size = BITS2BYTES(key_length);
            hash_tbl_init_info.data_size = 1;
            hash_tbl_init_info.table_width = 1;
            hash_tbl_init_info.hash_function = NULL;
            hash_tbl_init_info.rehash_function = NULL;
            hash_tbl_init_info.get_entry_fun = NULL;
            hash_tbl_init_info.set_entry_fun = NULL;

            SHR_IF_ERR_EXIT(utilex_hash_table_create(unit, &hash_tbl_id, hash_tbl_init_info));

            SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.hash_table_id.set(unit, table_id, hash_tbl_id));
            break;

        case DBAL_TABLE_TYPE_DIRECT:
            SHR_IF_ERR_EXIT(DBAL_SW_STATE_TABLES.table_type.set(unit, table_id, DBAL_SW_TABLE_DIRECT));

            num_of_entries = utilex_power_of_2(key_length);
            break;

        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Unknown logical table type, %d. table %s\n", table_type,
                         table_entry->table_name);
            break;
    }

    /** allocate the table entries buffers */
    SHR_IF_ERR_EXIT(dbal_db_allocate_sw_table(unit, table_id, num_of_entries, max_result_length_bytes));

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds the table's physical mapping part for tables
* of type PEMLA. 
* read from XML to the dbal table DB. 
* called by : dbal_db_init_logical_table_add
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add_mapping_pemla(
    int unit,
    dbal_logical_table_t * table_entry,
    dbal_tables_e table_id,
    table_db_struct_t * table_params)
{
    int ii;
    int nof_key_fields, nof_result_fields;

    SHR_FUNC_INIT_VARS(unit);

    table_entry->access_method = DBAL_ACCESS_METHOD_PEMLA;

    table_entry->pemla_db_id = table_params->pemla_db_id;

    table_entry->pemla_mapping.key_fields_mapping = NULL;
    table_entry->pemla_mapping.result_fields_mapping = NULL;
    nof_key_fields = table_entry->nof_key_fields;
    nof_result_fields = table_entry->multi_res_info[0].nof_result_fields;

    SHR_ALLOC_SET_ZERO(table_entry->pemla_mapping.key_fields_mapping, nof_key_fields * sizeof(uint32),
                       "pemla key mapping allocation", "%s%s%s\r\n", table_entry->table_name, EMPTY, EMPTY);

    SHR_ALLOC_SET_ZERO(table_entry->pemla_mapping.result_fields_mapping, nof_result_fields * sizeof(uint32),
                       "pemla result mapping allocation", "%s%s%s\r\n", table_entry->table_name, EMPTY, EMPTY);

    for (ii = 0; ii < nof_key_fields; ii++)
    {
        table_entry->pemla_mapping.key_fields_mapping[ii] = table_params->pemla_key_mapping[ii];
    }

    for (ii = 0; ii < nof_result_fields; ii++)
    {
        table_entry->pemla_mapping.result_fields_mapping[ii] = table_params->pemla_result_mapping[ii];
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function adds a logical table read from XML to the dbal 
* tables DB. 
* Main functionality is translating the string from XML to 
* corresponding values 
* ***************************************************** */
static shr_error_e
dbal_db_init_table_add(
    int unit,
    table_db_struct_t * table_params,
    dbal_logical_table_t * table_info,
    dbal_access_method_e table_type)
{
    dbal_logical_table_t *table_entry;
    dbal_tables_e table_id;
    int ii, jj;

    SHR_FUNC_INIT_VARS(unit);

    /** find the field id according to its name */
    SHR_IF_ERR_EXIT(dbal_logical_table_string_to_id(unit, table_params->name, &table_id));
    table_entry = table_info + table_id;

    /** set interface data */
    SHR_IF_ERR_EXIT(dbal_db_init_table_add_interface_to_table(unit, table_entry, table_params));

    /** Core Mode */
    SHR_IF_ERR_EXIT(dbal_core_mode_string_to_id(unit, table_params->core_mode, &table_entry->core_mode));

    /** Physical mapping part */
    switch (table_type)
    {
        case DBAL_ACCESS_METHOD_HARD_LOGIC:

            SHR_ALLOC_SET_ZERO(table_entry->hl_mapping_multi_res,
                               table_entry->nof_result_types * sizeof(dbal_hl_mapping_multi_res_t),
                               "multiple result direct mapping allocation", "%s%s%s\r\n",
                               table_entry->table_name, EMPTY, EMPTY);

            for (ii = 0; ii < table_entry->nof_result_types; ii++)
            {
                if (table_entry->nof_result_types > 1)
                {
                    for (jj = 0; jj < table_entry->nof_result_types; jj++)
                    {
                        if (sal_strcmp(table_params->results_set[ii].result_type_name,
                                       table_params->mapping_result_name[jj]) == 0)
                        {
                            break;
                        }
                    }
                    if (jj == table_entry->nof_result_types)
                    {
                        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cannot find result type %s in result mapping names. table:%s\n",
                                     table_params->results_set[ii].result_type_name, table_entry->table_name);
                    }
                }
                else
                {
                    jj = 0;
                }
                SHR_IF_ERR_EXIT(dbal_db_init_table_add_mapping_hard_logic(unit, table_id, table_entry,
                                                                          table_params, ii, jj));
            }
            break;

        case DBAL_ACCESS_METHOD_PHY_TABLE:
            SHR_IF_ERR_EXIT(dbal_db_init_table_add_mapping_mdb(unit, table_entry, table_params));
            break;

        case DBAL_ACCESS_METHOD_SW_ONLY:
            SHR_IF_ERR_EXIT(dbal_db_init_table_add_mapping_sw_state(unit, table_entry, table_id, table_params));
            break;

        case DBAL_ACCESS_METHOD_PEMLA:
            SHR_IF_ERR_EXIT(dbal_db_init_table_add_mapping_pemla(unit, table_entry, table_id, table_params));
            break;
        default:
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function fills the interface part of the fields DB. 
* called by: dbal_db_init_fields
* ***************************************************** */
static shr_error_e
dbal_db_init_field_read_interface(
    int unit,
    void *field_interface,
    field_db_struct_t * field_params)
{
    int res;
    void *childList;
    void *enumList;

    SHR_FUNC_INIT_VARS(unit);

    RHDATA_GET_STR_STOP(field_interface, "Type", field_params->type);
    RHCONTENT_GET_STR_STOP(field_interface, "FieldLabels", field_params->labels);
    if (dbx_xml_child_get_content_int(field_interface, "DefaultValue", &field_params->default_val) != NULL)
    {
        field_params->default_val_valid = TRUE;
    }

    /*
     * the Interface may includes one of the followings: 
     * 1. child list (if this is a parent field 
     * 2. enum values 
     */
    childList = dbx_xml_child_get_first(field_interface, "ChildFields");
    if (childList != NULL)
    {
        int child_idx = 0;
        void *curChild;

        RHDATA_ITERATOR(curChild, childList, "ChildField")
        {
            RHDATA_GET_STR_STOP(curChild, "Name", field_params->childs[child_idx].name);
            child_idx++;
        }
        field_params->nof_childs = child_idx;
    }

    enumList = dbx_xml_child_get_first(field_interface, "EnumValues");
    if (enumList != NULL)
    {
        int enum_idx = 0;
        void *curEnum;

        RHDATA_ITERATOR(curEnum, enumList, "EnumVal")
        {
            RHDATA_GET_STR_STOP(curEnum, "Name", field_params->enums[enum_idx].name_from_interface);
            enum_idx++;
        }
        field_params->nof_enum_vals = enum_idx;
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function fills the physical mapping part of the fields DB.
* called by: dbal_db_init_fields
* ***************************************************** */
static shr_error_e
dbal_db_init_field_read_phy_mapping(
    int unit,
    void *field_phy,
    field_db_struct_t * field_params)
{
    int res;
    void *childList;
    void *enumList;

    SHR_FUNC_INIT_VARS(unit);

    RHCHDATA_GET_STR_DEF(field_phy, "Encoding", "EncodeType", field_params->encode_type, "");
    RHCHDATA_GET_INT_DEF(field_phy, "Encoding", "Value", field_params->encode_param1, 0);
    RHCONTENT_GET_INT_DEF(field_phy, "MaxValue", &field_params->max_value, DBAL_DB_INVALID);

    /*
     * the Phy mapping may includes one of the followings: 
     * 1. child list - encoding
     * 2. enum values - mapping
     */
    childList = dbx_xml_child_get_first(field_phy, "ChildFieldsEncoding");
    if (childList != NULL)
    {
        int child_idx = 0;
        void *curChild;
        char cur_child_name[DBAL_MAX_STRING_LENGTH];

        RHDATA_ITERATOR(curChild, childList, "ChildField")
        {
            RHDATA_GET_STR_STOP(curChild, "Name", cur_child_name);
            for (child_idx = 0; child_idx < field_params->nof_childs; child_idx++)
            {
                if (sal_strcmp(cur_child_name, field_params->childs[child_idx].name) == 0)
                {
                    RHDATA_GET_STR_STOP(curChild, "EncodeType", field_params->childs[child_idx].encode_type);
                    RHDATA_GET_INT_STOP(curChild, "Value", field_params->childs[child_idx].encode_param1);
                    break;
                }
            }
        }
    }

    enumList = dbx_xml_child_get_first(field_phy, "EnumValueMapping");
    if (enumList != NULL)
    {
        int enum_idx = 0;
        void *curEnum;
        char cur_enum_name[DBAL_MAX_STRING_LENGTH];

        RHDATA_ITERATOR(curEnum, enumList, "EnumVal")
        {
            RHDATA_GET_STR_STOP(curEnum, "Name", cur_enum_name);
            for (enum_idx = 0; enum_idx < field_params->nof_enum_vals; enum_idx++)
            {
                if (sal_strcmp(cur_enum_name, field_params->enums[enum_idx].name_from_interface) == 0)
                {
                    RHDATA_GET_INT_STOP(curEnum, "Value", field_params->enums[enum_idx].value_from_mapping);
                    break;
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function fills the field from table interface DB 
* called by: dbal_db_init_table_read_interface
* ***************************************************** */
static shr_error_e
dbal_db_init_table_read_field(
    int unit,
    void *cur_field_node,
    table_db_field_params_struct_t * field_db)
{
    int res;

    SHR_FUNC_INIT_VARS(unit);

    RHDATA_GET_STR_STOP(cur_field_node, "Name", field_db->name);

    RHDATA_GET_STR_DEF(cur_field_node, "Valid", field_db->valid_dnx_data.full_map, "");
    SHR_IF_ERR_EXIT(dbal_db_get_dnx_data_indication(unit, &field_db->valid_dnx_data));

    RHDATA_GET_STR_DEF(cur_field_node, "Size", field_db->size_dnx_data.full_map, "");
    SHR_IF_ERR_EXIT(dbal_db_get_dnx_data_indication(unit, &field_db->size_dnx_data));

    RHDATA_GET_INT_DEF(cur_field_node, "Size", field_db->size, 0);
    RHDATA_GET_INT_DEF(cur_field_node, "Offset", field_db->offset, 0);
    RHDATA_GET_INT_DEF(cur_field_node, "NofInstances", field_db->nof_instances, 0);

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function fills the interface part of tables DB 
* called by: 
* dbal_db_init_logical_tables_hard_logic 
* dbal_db_init_logical_tables_mdb
* ***************************************************** */
static shr_error_e
dbal_db_init_table_read_interface(
    int unit,
    void *interface_node,
    table_db_struct_t * table_param)
{
    int res, result_counter, field_counter;
    void *curKey, *multipleResult, *curResult, *curSub;

    SHR_FUNC_INIT_VARS(unit);

    RHDATA_GET_STR_STOP(interface_node, "Type", table_param->type);

    field_counter = 0;
    curKey = dbx_xml_child_get_first(interface_node, "Key");
    if (curKey == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cant find Key section in table:%s\n", table_param->name);
    }

    RHDATA_ITERATOR(curSub, curKey, "Field")
    {
        SHR_IF_ERR_EXIT(dbal_db_init_table_read_field(unit, curSub, &table_param->key_fields[field_counter]));
        field_counter++;
    }
    table_param->nof_key_fields = field_counter;

    result_counter = 0;
    curResult = dbx_xml_child_get_first(interface_node, "Result");
    if (curResult != NULL)
    {
        field_counter = 0;
        RHDATA_ITERATOR(curSub, curResult, "Field")
        {
            SHR_IF_ERR_EXIT(dbal_db_init_table_read_field(unit, curSub,
                                                          &table_param->results_set[result_counter].
                                                          result_fields[field_counter++]));
        }
        table_param->results_set[result_counter++].nof_res_fields = field_counter;
    }
    else
    {
        result_counter = 0;
        multipleResult = dbx_xml_child_get_first(interface_node, "MultipleResults");
        if (multipleResult == NULL)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cant find Result section (Result / MultipleResults) in table:%s\n",
                         table_param->name);
        }

        RHDATA_ITERATOR(curResult, multipleResult, "Result")
        {
            field_counter = 0;
            RHDATA_ITERATOR(curSub, curResult, "Field")
            {
                if (field_counter == 0)
                {
                    /** First field, read the result type name (value) */
                    RHDATA_GET_STR_STOP(curSub, "Value", table_param->results_set[result_counter].result_type_name);
                }
                SHR_IF_ERR_EXIT(dbal_db_init_table_read_field(unit, curSub,
                                                              &table_param->results_set[result_counter].result_fields
                                                              [field_counter++]));
            }
            table_param->results_set[result_counter++].nof_res_fields = field_counter;
        }
    }
    table_param->num_of_results_sets = result_counter;

    if ((result_counter > DBAL_MAX_NUMBER_OF_RESULT_TYPES) || (result_counter < DBAL_MIN_NUMBER_OF_RESULT_TYPES))
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal number of result types:%d, "
                     "minimum results sets is %d and maximum is %d."
                     "table %s\n",
                     result_counter, DBAL_MIN_NUMBER_OF_RESULT_TYPES, DBAL_MAX_NUMBER_OF_RESULT_TYPES,
                     table_param->name);
    }

    RHCONTENT_GET_STR_STOP(interface_node, "TableLabels", table_param->labels);

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function fills the field from table access DB 
* called by: dbal_db_init_logical_tables_hard_logic
* ***************************************************** */
static shr_error_e
dbal_db_init_table_read_access_field(
    int unit,
    void *field_node,
    char field_name[DBAL_MAX_STRING_LENGTH],
    int field_size,
    int field_offset,
    table_db_access_params_struct_t * access_params)
{
    int res;
    void *access_type_node, *condition_node, *alias_node;
    int condition_index;

    SHR_FUNC_INIT_VARS(unit);

    sal_strncpy(access_params->access_field_name, field_name, sizeof(access_params->access_field_name) - 1);
    access_params->access_field_name[sizeof(access_params->access_field_name) - 1] = '\0';

    access_params->access_size = field_size;
    access_params->access_offset = field_offset;

    condition_index = 0;
    condition_node = dbx_xml_child_get_first(field_node, "KeyCondition");
    while (condition_node != NULL)
    {
        RHDATA_GET_STR_DEF(condition_node, "Type", access_params->access_condition[condition_index].type, "");
        RHDATA_GET_STR_DEF(condition_node, "Field", access_params->access_condition[condition_index].field, "");
        RHDATA_GET_INT_DEF(condition_node, "Value", access_params->access_condition[condition_index].value, 0);
        RHDATA_GET_STR_DEF(condition_node, "Enum", access_params->access_condition[condition_index].enum_val, "");
        condition_node = dbx_xml_child_get_next(condition_node);
        condition_index++;
    }

    access_type_node = dbx_xml_child_get_first(field_node, "Register");
    if (access_type_node != NULL)
    {
        access_params->access_type = DBAL_HL_ACCESS_REGISTER;
    }
    else
    {
        access_type_node = dbx_xml_child_get_first(field_node, "Memory");
        if (access_type_node != NULL)
        {
            access_params->access_type = DBAL_HL_ACCESS_MEMORY;
        }
        else
        {
            access_type_node = dbx_xml_child_get_first(field_node, "SwState");
            if (access_type_node != NULL)
            {
                access_params->access_type = DBAL_HL_ACCESS_SW;
            }
            else
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cant find Memory/Register/SwState section in field:%s\n",
                             access_params->access_field_name);
            }
        }
    }

    if ((access_params->access_type == DBAL_HL_ACCESS_MEMORY) ||
        (access_params->access_type == DBAL_HL_ACCESS_REGISTER))
    {
        RHDATA_GET_STR_STOP(access_type_node, "Name", access_params->access_name);

        RHCHDATA_GET_STR_DEF(access_type_node, "ArrayOffset", "EncodeType", access_params->array_offset.type, "");
        RHCHDATA_GET_STR_DEF(access_type_node, "ArrayOffset", "Field", access_params->array_offset.field, "");
        RHCHDATA_GET_INT_DEF(access_type_node, "ArrayOffset", "Value", access_params->array_offset.value, 0);

        RHCHDATA_GET_STR_DEF(access_type_node, "EntryOffset", "EncodeType", access_params->entry_offset.type, "");
        RHCHDATA_GET_STR_DEF(access_type_node, "EntryOffset", "Field", access_params->entry_offset.field, "");
        RHCHDATA_GET_INT_DEF(access_type_node, "EntryOffset", "Value", access_params->entry_offset.value, 0);

        RHCHDATA_GET_STR_DEF(access_type_node, "BlockIndex", "EncodeType", access_params->block_index.type, "");
        RHCHDATA_GET_STR_DEF(access_type_node, "BlockIndex", "Field", access_params->block_index.field, "");
        RHCHDATA_GET_INT_DEF(access_type_node, "BlockIndex", "Value", access_params->block_index.value, 0);

        RHCHDATA_GET_STR_DEF(access_type_node, "DataOffset", "EncodeType", access_params->data_offset.type, "");
        RHCHDATA_GET_STR_DEF(access_type_node, "DataOffset", "Field", access_params->data_offset.field, "");
        RHCHDATA_GET_INT_DEF(access_type_node, "DataOffset", "Value", access_params->data_offset.value, 0);

        RHCHDATA_GET_STR_DEF(access_type_node, "HwField", "Name", access_params->hw_field, "");

        alias_node = dbx_xml_child_get_first(access_type_node, "Alias");
        if (alias_node != NULL)
        {
            RHDATA_GET_STR_STOP(alias_node, "Name", access_params->alias_name);
            RHCHDATA_GET_STR_DEF(alias_node, "DataOffset", "EncodeType", access_params->alias_data_offset.type, "");
            RHCHDATA_GET_STR_DEF(alias_node, "DataOffset", "Field", access_params->alias_data_offset.field, "");
            RHCHDATA_GET_INT_DEF(alias_node, "DataOffset", "Value", access_params->alias_data_offset.value, 0);
        }

    }
    else
    {
        sal_strcpy(access_params->access_name, "sw_state");
    }
exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function fills the logical to physical mapping information
* of hard logic tables type. 
* called by: dbal_db_init_logical_tables
* ***************************************************** */
static shr_error_e
dbal_db_init_table_logical_to_physical_hard_logic(
    int unit,
    void *physical_node,
    table_db_struct_t * table_param)
{
    int res;
    int ii, jj, kk;
    int access_counter, map_idx;
    void *FieldMapping, *curFieldMapping, *curAccessField;

    SHR_FUNC_INIT_VARS(unit);

    map_idx = 0;
    RHDATA_ITERATOR(FieldMapping, physical_node, "FieldMapping")
    {
        access_counter = 0;
        RHDATA_GET_STR_DEF(FieldMapping, "ResultType", table_param->mapping_result_name[map_idx], "");
        RHDATA_ITERATOR(curFieldMapping, FieldMapping, "Field")
        {
            char field_name[DBAL_MAX_STRING_LENGTH];
            char orig_field_name[DBAL_MAX_STRING_LENGTH];
            int field_instance;
            int field_size, field_offset;

            RHDATA_GET_STR_STOP(curFieldMapping, "Name", field_name);
            sal_strcpy(orig_field_name, field_name);
            RHDATA_GET_INT_DEF(curFieldMapping, "Size", field_size, 0);
            RHDATA_GET_INT_DEF(curFieldMapping, "Offset", field_offset, 0);

            RHDATA_ITERATOR(curAccessField, curFieldMapping, "Access")
            {
                RHDATA_GET_INT_DEF(curAccessField, "instanceIndex", field_instance, 0);
                if (field_instance != 0)
                {
                    sal_sprintf(field_name, "%s_%02d", orig_field_name, field_instance);
                }
                SHR_IF_ERR_EXIT(dbal_db_init_table_read_access_field(unit, curAccessField, field_name,
                                                                     field_size, field_offset,
                                                                     &table_param->access[map_idx][access_counter]));
                access_counter++;
            }
            RHDATA_ITERATOR(curAccessField, curFieldMapping, "SwAccess")
            {
                SHR_IF_ERR_EXIT(dbal_db_init_table_read_access_field(unit, curAccessField, field_name,
                                                                     field_size, field_offset,
                                                                     &table_param->access[map_idx][access_counter]));
                access_counter++;
            }
        }
        table_param->nof_access[map_idx] = access_counter;

        /*
         * re-arrange the fields in access 
         * purpose: fields of the same memory should be 'packed'
         */
        for (ii = 0; ii < access_counter; ii++)
        {
            table_db_access_params_struct_t temp_access;
            int same_memory_counter = 0;
            int same_memory_field_counter = 0;
            int cell_to_switch;

            for (jj = ii + 1; jj < access_counter; jj++)
            {
                cell_to_switch = 0;
                if (sal_strcmp
                    (table_param->access[map_idx][ii].access_name, table_param->access[map_idx][jj].access_name) == 0)
                {
                    if (sal_strcmp(table_param->access[map_idx][ii].hw_field, table_param->access[map_idx][jj].hw_field)
                        == 0)
                    {
                        /** found a field in the same memroy and same hw_field */
                        same_memory_field_counter++;
                        cell_to_switch = ii + same_memory_field_counter;
                    }
                    else
                    {
                        /*
                         * found a field in the same memroy but not the same hw_field 
                         * move it to right after the "same memory+same hw_field" packaged fields
                         */
                        same_memory_counter++;
                        cell_to_switch = ii + same_memory_field_counter + same_memory_counter;
                    }
                }

                if (cell_to_switch > jj)
                {
                    /** cell to switch cannot be larger than the current cell */
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "hard logic parsing, illegal state pf the packing algorithm\n");
                }

                if ((cell_to_switch == jj) || (cell_to_switch == 0))
                {
                    /*
                     * no switching 
                     * the cell to switch is the current one, or memory is not matched 
                     */
                    continue;
                }
                else
                {
                    /*
                     * move the current cell to the "cell to switch index" 
                     * all cells between will be kept in the same order.
                     */
                    int nof_copy_iterations = jj - cell_to_switch;
                    temp_access = table_param->access[map_idx][jj];
                    for (kk = 0; kk < nof_copy_iterations; kk++)
                    {
                        table_param->access[map_idx][jj - kk] = table_param->access[map_idx][jj - kk - 1];
                    }
                    table_param->access[map_idx][jj - nof_copy_iterations] = temp_access;
                }
            }
        }
        map_idx++;
    }

exit:
    SHR_FUNC_EXIT;
}

/** ****************************************************
* \brief
* The function fills the logical to physical mapping information
* of pemla tables type. 
* called by: dbal_db_init_logical_tables
* ***************************************************** */
static shr_error_e
dbal_db_init_table_logical_to_physical_pemla(
    int unit,
    void *physical_node,
    table_db_struct_t * table_param)
{
    int res;
    int ii;
    void *FieldMapping, *curFieldMapping;

    SHR_FUNC_INIT_VARS(unit);

    RHCONTENT_GET_INT_STOP(physical_node, "DbId", &table_param->pemla_db_id);

    FieldMapping = dbx_xml_child_get_first(physical_node, "FieldMapping");
    if (FieldMapping != NULL)
    {
        /** map according to mapping in xml */
        RHDATA_ITERATOR(curFieldMapping, FieldMapping, "Field")
        {
            char field_name[DBAL_MAX_STRING_LENGTH];
            int mapping_value;
            uint8 key_field, result_field;

            key_field = FALSE;
            result_field = FALSE;

            RHDATA_GET_STR_STOP(curFieldMapping, "Name", field_name);
            RHDATA_GET_INT_STOP(curFieldMapping, "HwValue", mapping_value);
            for (ii = 0; ii < table_param->nof_key_fields; ii++)
            {
                if (sal_strcmp(field_name, table_param->key_fields[ii].name) == 0)
                {
                    table_param->pemla_key_mapping[ii] = mapping_value;
                    key_field = TRUE;
                    break;
                }
            }
            if (!key_field)
            {
                for (ii = 0; ii < table_param->results_set[0].nof_res_fields; ii++)
                {
                    if (sal_strcmp(field_name, table_param->results_set[0].result_fields[ii].name) == 0)
                    {
                        table_param->pemla_result_mapping[ii] = mapping_value;
                        result_field = TRUE;
                        break;
                    }
                }
                if (!result_field)
                {
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "field %s was not found in key or result fields\n", field_name);
                }
            }
        }
    }
    else
    {
        /** If field mapping does not exists, map according to index */
        for (ii = 0; ii < table_param->nof_key_fields; ii++)
        {
            table_param->pemla_key_mapping[ii] = ii;
        }
        for (ii = 0; ii < table_param->results_set[0].nof_res_fields; ii++)
        {
            table_param->pemla_result_mapping[ii] = ii;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_db_init_fields(
    int unit,
    int flags,
    dbal_field_basic_info_t * field_info)
{
    int res = 0;
    int file_idx, nof_files;
    void *curTop, *cur, *curInterface, *curPhy;
    char **file_list_ptr;

    field_db_struct_t *cur_field_param = NULL;

    SHR_FUNC_INIT_VARS(unit);

    SHR_ALLOC(cur_field_param, sizeof(field_db_struct_t), "fields,cur_field_param", "%s%s%s\r\n", EMPTY, EMPTY, EMPTY);

    if (dbal_db_file_list_init(unit) != _SHR_E_NONE)
    {
        SHR_ERR_EXIT(_SHR_E_MEMORY, "Cant allocate file list array\n");
    }
    file_list_ptr = &xml_files_list[0];

    /** Read xml DB */
    if ((flags & DBAL_INIT_FLAGS_VALIDATION) == 0)
    {
        nof_files = dbx_file_get_xml_files_from_dir(SOC_CHIP_STRING(unit), DB_INIT_DIR_PATH_FIELDS, file_list_ptr);
    }
    else
    {
        nof_files = dbx_file_get_xml_files_from_dir(SOC_CHIP_STRING(unit),
                                                    DB_INIT_VALIDATION_TEST_PATH_FIELDS, file_list_ptr);
    }

    for (file_idx = 0; file_idx < nof_files; file_idx++)
    {
        char file_path[RHFILE_MAX_SIZE];
        char file_name[RHFILE_MAX_SIZE];

        if ((flags & DBAL_INIT_FLAGS_VALIDATION) == 0)
        {
            sal_strcpy(file_path, DB_INIT_DIR_PATH_FIELDS);
        }
        else
        {
            sal_strcpy(file_path, DB_INIT_VALIDATION_TEST_PATH_FIELDS);
        }
        sal_strcat(file_path, "/");

        sal_strncpy(file_name, xml_files_list[file_idx], sizeof(file_name) - 1);
        file_name[sizeof(file_name) - 1] = '\0';
        sal_strcat(file_path, file_name);

        curTop = dbx_file_get_xml_top(SOC_CHIP_STRING(unit), file_path, "FieldDbCatalog", 0);
        if (curTop == NULL)
        {
            SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "Cannot find dbal fields xml file: %s\n", file_path);
        }

        /** Run all over XML fields and add it to the fields table */
        RHDATA_ITERATOR(cur, curTop, "Field")
        {
            char instances_support[DBAL_MAX_STRING_LENGTH];
            sal_memset(cur_field_param, 0x0, sizeof(field_db_struct_t));

            RHDATA_GET_STR_STOP(cur, "Name", cur_field_param->name);

            RHDATA_GET_STR_STOP(cur, "Size", cur_field_param->size_dnx_data.full_map);
            SHR_IF_ERR_EXIT(dbal_db_get_dnx_data_indication(unit, &cur_field_param->size_dnx_data));

            RHDATA_GET_INT_STOP(cur, "Size", cur_field_param->size);

            RHDATA_GET_STR_DEF(cur, "InstanceSupport", instances_support, "");
            if (sal_strcmp(instances_support, "TRUE") == 0)
            {
                cur_field_param->instances_support = 1;
            }

            curInterface = dbx_xml_child_get_first(cur, "FieldDbInterface");
            if (curInterface == NULL)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cant find FieldDbInterface section in field:%s\n",
                             cur_field_param->name);
            }
            SHR_IF_ERR_EXIT(dbal_db_init_field_read_interface(unit, curInterface, cur_field_param));

            curPhy = dbx_xml_child_get_first(cur, "FieldToPhyDbMapping");
            if (curPhy == NULL)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cant find FieldToPhyDbMapping section in field:%s\n",
                             cur_field_param->name);
            }
            SHR_IF_ERR_EXIT(dbal_db_init_field_read_phy_mapping(unit, curPhy, cur_field_param));

            SHR_IF_ERR_EXIT(dbal_db_init_field_add(unit, cur_field_param, field_info));
        }
    }

exit:
    SHR_FREE(cur_field_param);
    dbal_db_file_list_deinit(unit);
    if (res)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META("XML PARSING ERROR\n")));
        SHR_SET_CURRENT_ERR(res);
    }
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_db_init_logical_tables(
    int unit,
    int flags,
    dbal_logical_table_t * table_info,
    dbal_access_method_e table_type)
{
    int file_idx, nof_files;
    int res;
    void *curTop, *cur, *curInterface, *curPhy, *multipleResultsTypeMapping, *curResultTypeMapping;
    char **file_list_ptr;
    char db_path[RHFILE_MAX_SIZE];

    table_db_struct_t *cur_table_param = NULL;

    SHR_FUNC_INIT_VARS(unit);

    SHR_ALLOC(cur_table_param, sizeof(table_db_struct_t),
              "logical_tables,cur_table_param", "%s%s%s\r\n", EMPTY, EMPTY, EMPTY);

    if (dbal_db_file_list_init(unit) != _SHR_E_NONE)
    {
        SHR_ERR_EXIT(_SHR_E_MEMORY, "Cant allocate file list array\n");
    }
    file_list_ptr = &xml_files_list[0];

    /** Read xml DB */
    sal_strcpy(db_path, DBAL_DB_GET_TABLES_DIR_PATH(table_type, flags));

    nof_files = dbx_file_get_xml_files_from_dir(SOC_CHIP_STRING(unit), db_path, file_list_ptr);

    for (file_idx = 0; file_idx < nof_files; file_idx++)
    {
        char file_path[RHFILE_MAX_SIZE];
        char file_name[RHFILE_MAX_SIZE];

        sal_strcpy(file_path, db_path);
        sal_strcat(file_path, "/");

        sal_strncpy(file_name, xml_files_list[file_idx], sizeof(file_name) - 1);
        file_name[sizeof(file_name) - 1] = '\0';

        sal_strcat(file_path, file_name);

        curTop = dbx_file_get_xml_top(SOC_CHIP_STRING(unit), file_path, "AppDbCatalog", 0);
        if (curTop == NULL)
        {
            SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "Cannot find dbal tables xml file: %s\n", file_path);
        }

        /** Run all over XML fields and add it to the fields table */
        RHDATA_ITERATOR(cur, curTop, "AppDB")
        {
            sal_memset(cur_table_param, 0x0, sizeof(table_db_struct_t));
            RHDATA_GET_STR_STOP(cur, "Name", cur_table_param->name);

            RHDATA_GET_STR_DEF(cur, "Valid", cur_table_param->valid_dnx_data.full_map, "");
            SHR_IF_ERR_EXIT(dbal_db_get_dnx_data_indication(unit, &cur_table_param->valid_dnx_data));

            RHCONTENT_GET_INT_DEF(cur, "MaturityLevel", &cur_table_param->maturity_level, 2);
            if (cur_table_param->maturity_level == DBAL_MATURITY_LOW)
            {
                continue;
            }

            curInterface = dbx_xml_child_get_first(cur, "AppDbInterface");
            if (curInterface == NULL)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cant find AppDbInterface section in table:%s\n", cur_table_param->name);
            }
            SHR_IF_ERR_EXIT(dbal_db_init_table_read_interface(unit, curInterface, cur_table_param));

            curPhy = dbx_xml_child_get_first(cur, "AppToPhyDbMapping");
            if (curPhy == NULL)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Cant find AppToPhyDbMapping section in table:%s\n",
                             cur_table_param->name);
            }

            RHCHDATA_GET_STR_STOP(curPhy, "CoreMode", "Mode", cur_table_param->core_mode);

            multipleResultsTypeMapping = dbx_xml_child_get_first(curPhy, "MultipleResultsMapping");
            if (multipleResultsTypeMapping != NULL)
            {
                RHDATA_ITERATOR(curResultTypeMapping, multipleResultsTypeMapping, "Result")
                {
                    int ii, hw_value;
                    char logical_value[DBAL_MAX_STRING_LENGTH];

                    RHDATA_GET_INT_STOP(curResultTypeMapping, "Value", hw_value);
                    RHDATA_GET_STR_STOP(curResultTypeMapping, "LogicalValue", logical_value);
                    for (ii = 0; ii < cur_table_param->num_of_results_sets; ii++)
                    {
                        if (sal_strcmp(logical_value, cur_table_param->results_set[ii].result_type_name) == 0)
                        {
                            if (cur_table_param->results_set[ii].result_is_mapped)
                            {
                                SHR_ERR_EXIT(_SHR_E_INTERNAL, "result type %s has multiple mapping. in table:%s\n",
                                             logical_value, cur_table_param->name);
                            }
                            else
                            {
                                cur_table_param->results_set[ii].result_is_mapped = TRUE;
                                cur_table_param->results_set[ii].result_type_physical_value = hw_value;
                                break;
                            }
                        }
                    }
                    if (ii == cur_table_param->num_of_results_sets)
                    {
                        SHR_ERR_EXIT(_SHR_E_INTERNAL,
                                     "result type mapping %s was not found in interface. in table:%s\n", logical_value,
                                     cur_table_param->name);
                    }
                }
            }
            switch (table_type)
            {
                case DBAL_ACCESS_METHOD_PHY_TABLE:
                    RHCONTENT_GET_STR_STOP(curPhy, "PhyDb", cur_table_param->phy_db);
                    RHCONTENT_GET_INT_DEF(curPhy, "AppDbId", &cur_table_param->app_db_id, DBAL_DB_INVALID);
                    break;
                case DBAL_ACCESS_METHOD_HARD_LOGIC:
                    SHR_IF_ERR_EXIT(dbal_db_init_table_logical_to_physical_hard_logic(unit, curPhy, cur_table_param));
                    break;
                case DBAL_ACCESS_METHOD_SW_ONLY:
                    RHCHDATA_GET_INT_DEF(curPhy, "TableProperties", "Size", cur_table_param->sw_table_size, 100);
                    break;
                case DBAL_ACCESS_METHOD_PEMLA:
                    SHR_IF_ERR_EXIT(dbal_db_init_table_logical_to_physical_pemla(unit, curPhy, cur_table_param));
                    break;
                default:
                    break;
            }

            SHR_IF_ERR_EXIT(dbal_db_init_table_add(unit, cur_table_param, table_info, table_type));
        }
    }

exit:
    SHR_FREE(cur_table_param);
    dbal_db_file_list_deinit(unit);
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_db_init_fields_set_default(
    int unit,
    dbal_field_basic_info_t * field_info)
{
    int ii;

    CONST static dbal_field_basic_info_t field_default = {
        "",                             /** name */
        0,                              /** max_size */
        DBAL_NOF_FIELD_TYPES,           /** type */
        0,                              /** instances_support */
        {DBAL_LABEL_NONE},              /** labels */
        0,                              /** default_value_valid */
        0,                              /** default_value */
        0,                              /** max_value */
        {DBAL_FIELD_EMPTY},             /** parent fields */
        0,                              /** num of child fields */
        NULL,                           /** sub_fields_info */
        0,                              /** num of enum vals */
        NULL,                           /** enum_val_info */
        {0}                             /** encode_info */
    };

    SHR_FUNC_INIT_VARS(unit);

    /** Initialize all fields */
    for (ii = 0; ii < DBAL_NOF_FIELDS; ii++)
    {
        field_info[ii] = field_default;
    }
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_db_init_tables_set_default(
    int unit,
    dbal_logical_table_t * table_info)
{
    int ii;

    CONST static dbal_logical_table_t table_default = {
        "",                         /** table_name */
        0,                          /** is_table_valid */
        0,                          /** is_table_initiated */
        DBAL_MATURITY_LOW,          /** matuity level */
        {DBAL_LABEL_NONE},          /** labels */
        DBAL_TABLE_TYPE_NONE,       /** table_type */
        0,                          /** nof_entries */
        0,                          /** min_index */
        0,                          /** max_capacity */
        0,                          /** nof_key_fields */
        NULL,                       /** keys_info */
        0,                          /** key_size */
        0,                          /** nof_result_types */
        NULL,                       /** multi_res_info */
        0,                          /** sw_payload_length_bytes */
        0,                          /** max_payload_size */
        0,                          /** max_nof_result_fields */
        DBAL_CORE_NONE,             /** core_mode */
        DBAL_NOF_ACCESS_METHODS,    /** access_method */
        0,                          /** app_id */
        DBAL_PHYSICAL_TABLE_NONE,   /** physical_db_id */
        0,                          /** result_types_unify */
        NULL,                       /** l2p_direct_info */
        0,                          /** pemla_db_id */
        {NULL}                      /** pemla_mapping */
    };

    SHR_FUNC_INIT_VARS(unit);

    /** Initialize all tables */
    for (ii = 0; ii < DBAL_NOF_TABLES; ii++)
    {
        table_info[ii] = table_default;
    }
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_db_init_fields_logical_validation(
    int unit,
    dbal_field_basic_info_t * fields_info)
{
    int ii, jj, iter;
    int max_field_size;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_FIELDS; ii++)
    {
        /** validate max size of field */
        switch (fields_info[ii].type)
        {
            case DBAL_FIELD_TYPE_ARRAY32:
            case DBAL_FIELD_TYPE_ARRAY8:
            case DBAL_FIELD_TYPE_BITMAP:
                max_field_size = BYTES2BITS(DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES);
                break;
            case DBAL_FIELD_TYPE_BOOL:
                max_field_size = 1;
                break;
            case DBAL_FIELD_TYPE_INT32:
            case DBAL_FIELD_TYPE_UINT32:
            case DBAL_FIELD_TYPE_IP:
                max_field_size = 32;
                break;
            case DBAL_FIELD_TYPE_ENUM:
                /** DBAL_FIELD_MAX_NUM_OF_ENUM_VALUES in bits */
                max_field_size = utilex_msb_bit_on(DBAL_FIELD_MAX_NUM_OF_ENUM_VALUES) + 1;
                break;
            case DBAL_FIELD_TYPE_NONE:
            case DBAL_NOF_FIELD_TYPES:
            default:
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Unknown field type, field: %s\n", dbal_field_to_string(unit, ii));
        }
        if (fields_info[ii].max_size > max_field_size)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Field size for field %s is too long %d, max size of is %d bits\n",
                         fields_info[ii].name, fields_info[ii].max_size, max_field_size);
        }

        /** if a field has children, update it as a parent for all its child fields */
        for (iter = 0; iter < fields_info[ii].nof_child_fields; iter++)
        {
            for (jj = 0; jj < DBAL_FIELD_MAX_PARENTS_PER_FIELD; jj++)
            {
                if (fields_info[fields_info[ii].sub_field_info[iter].sub_field_id].parent_field_id[jj] ==
                    DBAL_FIELD_EMPTY)
                {
                    fields_info[fields_info[ii].sub_field_info[iter].sub_field_id].parent_field_id[jj] = ii;
                    break;
                }
            }
            if (jj == DBAL_FIELD_MAX_PARENTS_PER_FIELD)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Field %s exceed the max number of parent fields (%d)\n",
                             dbal_field_to_string(unit, fields_info[ii].sub_field_info[iter].sub_field_id),
                             DBAL_FIELD_MAX_PARENTS_PER_FIELD);
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_db_init_tables_logical_validation(
    int unit,
    dbal_logical_table_t * tables_info)
{
    int ii, jj, kk, qq;
    int bits_offset_in_key, bits_offset_in_payload;
    int bytes_offset_in_sw_payload;
    int max_payload_size, max_nof_result_fields;
    uint8 is_all_core_mode, is_core_in_key;
    uint32 key_size;
    dbal_table_field_info_t *field_info;
    dbal_logical_table_t *table = NULL;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_TABLES; ii++)
    {
        table = &tables_info[ii];
        is_all_core_mode = FALSE;
        is_core_in_key = FALSE;

        /**  check that table is not already initiated */
        if (table->is_table_initiated == 1)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Init alredy preformed on table %s\n", table->table_name);
        }

        /**  check that table is not already initiated */
        if (table->is_table_valid == 0)
        {
            continue;
        }

        bits_offset_in_key = 0;
        bits_offset_in_payload = 0;
        bytes_offset_in_sw_payload = 0;

        /** Validate that the core indication are valid */
        if (table->core_mode == DBAL_CORE_ALL)
        {
            is_all_core_mode = TRUE;
        }
        else if (table->core_mode == DBAL_CORE_BY_INPUT)
        {
            is_all_core_mode = FALSE;
        }
        else
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Invalid core mode for table %s\n", table->table_name);
        }

        /** For all fields, set it offset in table's key and payload */
        bits_offset_in_key = 0;
        key_size = 0;
        for (jj = 0; jj < table->nof_key_fields; jj++)
        {
            field_info = &(table->keys_info[jj]);
            if (field_info->field_id == DBAL_FIELD_EMPTY)
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Invalid field ID in table %s \n", table->table_name);
            }

            field_info->is_field_encoded = dbal_fields_is_field_encoded(unit, field_info->field_id);

            if (field_info->field_id == DBAL_FIELD_CORE_ID)
            {
                is_core_in_key |= TRUE;
                if (field_info->field_nof_bits != DBAL_CORE_SIZE_IN_BITS)
                {
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "Field core ID has to be 1 bit field. table %s\n", table->table_name);
                }
            }
            else
            {
                field_info->bits_offset_in_buffer = bits_offset_in_key;
                bits_offset_in_key += field_info->field_nof_bits;
            }

            /** Check that any field is not repeated in the table  */
            for (kk = jj + 1; kk < table->nof_key_fields; kk++)
            {
                uint8 is_duplication = FALSE;
                SHR_IF_ERR_EXIT(dbal_db_check_field_duplication(unit, field_info->field_id,
                                                                table->keys_info[kk].field_id, &is_duplication));
                if (is_duplication)
                {
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "The field %s is appear more than once in the table %s\n",
                                 dbal_field_to_string(unit, field_info->field_id), table->table_name);
                }
            }

            key_size += field_info->field_nof_bits;
        }

        /** for MDB tables we need to invert the order of the buffer */
        if (table->access_method == DBAL_ACCESS_METHOD_PHY_TABLE)
        {
            int curr_buffer_size = 0;
            if (table->table_type == DBAL_TABLE_TYPE_LPM)
            {
                /*
                 * LPM (KAPS) table's key is aligned to MSB, with fixed key size 
                 * of MDB_KAPS_KEY_WIDTH_IN_BITS (aligned to MSB - KAPS prefix length)
                 */
                for (kk = 0; kk < table->nof_key_fields; kk++)
                {
                    curr_buffer_size += table->keys_info[kk].field_nof_bits;
                    table->keys_info[kk].bits_offset_in_buffer =
                        MDB_KAPS_KEY_WIDTH_IN_BITS - (MDB_KAPS_KEY_PREFIX_LENGTH + curr_buffer_size);
                }
                table->key_size = MDB_KAPS_KEY_WIDTH_IN_BITS;
            }
            else
            {
                for (kk = table->nof_key_fields - 1; kk >= 0; kk--)
                {
                    table->keys_info[kk].bits_offset_in_buffer = curr_buffer_size;
                    curr_buffer_size += table->keys_info[kk].field_nof_bits;
                }

                table->key_size = key_size;
            }
            if ((key_size > SAL_UINT32_NOF_BITS) && (table->table_type == DBAL_TABLE_TYPE_DIRECT))
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "MDB tables's key of type direct is limited to 32 bit. table %s\n",
                             table->table_name);
            }
        }
        else
        {
            table->key_size = key_size;
        }

        /*
         * For all result types, do the same as for the single result type 
         * For all fields, set it offset in table's key and payload 
         */
        for (qq = 0; qq < table->nof_result_types; qq++)
        {
            multi_res_info_t *result_type_set = &(table->multi_res_info[qq]);

            bits_offset_in_payload = 0;
            bytes_offset_in_sw_payload = 0;

            for (jj = 0; jj < result_type_set->nof_result_fields; jj++)
            {
                field_info = &(result_type_set->results_info[jj]);

                if (field_info->field_id == DBAL_FIELD_EMPTY)
                {
                    SHR_ERR_EXIT(_SHR_E_INTERNAL, "Invalid field ID in table %s, result type %s \n",
                                 table->table_name, result_type_set->result_type_name);
                }

                field_info->is_field_encoded = dbal_fields_is_field_encoded(unit, field_info->field_id);

                result_type_set->entry_payload_size += result_type_set->results_info[jj].field_nof_bits;
                result_type_set->results_info[jj].bits_offset_in_buffer = bits_offset_in_payload;
                bits_offset_in_payload += result_type_set->results_info[jj].field_nof_bits;
                if (table->access_method == DBAL_ACCESS_METHOD_SW_ONLY)
                {
                    result_type_set->results_info[jj].is_sw_field = TRUE;
                    result_type_set->results_info[jj].bytes_offset_in_sw_buffer = bytes_offset_in_sw_payload;
                    bytes_offset_in_sw_payload += BITS2BYTES(result_type_set->results_info[jj].field_nof_bits);
                }

                /*
                 * Check that any field is not repeated in the table  
                 */
                for (kk = jj + 1; kk < result_type_set->nof_result_fields; kk++)
                {
                    uint8 is_duplication = FALSE;
                    SHR_IF_ERR_EXIT(dbal_db_check_field_duplication(unit, field_info->field_id,
                                                                    table->multi_res_info[qq].results_info[kk].field_id,
                                                                    &is_duplication));
                    if (is_duplication)
                    {
                        SHR_ERR_EXIT(_SHR_E_INTERNAL,
                                     "The field %s is appear more than once in the table %s, result type %s\n",
                                     dbal_field_to_string(unit, result_type_set->results_info[kk].field_id),
                                     table->table_name, result_type_set->result_type_name);
                    }
                }
            }
        }

        max_payload_size = 0;
        max_nof_result_fields = 0;
        for (qq = 0; qq < table->nof_result_types; qq++)
        {
            multi_res_info_t *result_type_set = &(table->multi_res_info[qq]);
            max_payload_size = UTILEX_MAX(max_payload_size, result_type_set->entry_payload_size);
            max_nof_result_fields = UTILEX_MAX(max_nof_result_fields, result_type_set->nof_result_fields);
        }
        table->max_payload_size = max_payload_size;
        table->max_nof_result_fields = max_nof_result_fields;

        for (qq = 0; qq < table->nof_result_types; qq++)
        {
            multi_res_info_t *result_type_set = &(table->multi_res_info[qq]);
            result_type_set->zero_padding = max_payload_size - result_type_set->entry_payload_size;
        }

        for (qq = 0; qq < table->nof_result_types; qq++)
        {
            multi_res_info_t *result_type_set = &(table->multi_res_info[qq]);

            /** for MDB tables we need to invert the order of the buffer */
            if (table->access_method == DBAL_ACCESS_METHOD_PHY_TABLE)
            {
                int curr_buffer_size = result_type_set->zero_padding;
                for (kk = result_type_set->nof_result_fields - 1; kk >= 0; kk--)
                {
                    result_type_set->results_info[kk].bits_offset_in_buffer = curr_buffer_size;
                    curr_buffer_size += result_type_set->results_info[kk].field_nof_bits;
                }
            }
        }

        if (is_all_core_mode == is_core_in_key)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Mismatch in core mode and core indication in key in table %s\n",
                         table->table_name);
        }

        /*
         * Copy interface parameter to the access corresponding fields 
         * Do it for all result types 
         */
        if (table->access_method == DBAL_ACCESS_METHOD_HARD_LOGIC)
        {
            for (qq = 0; qq < table->nof_result_types; qq++)
            {
                int field_in_table = 0;
                multi_res_info_t *result_type_set = &(table->multi_res_info[qq]);
                bytes_offset_in_sw_payload = 0;

                for (kk = 0; kk < DBAL_NOF_HL_ACCESS_TYPES; kk++)
                {
                    for (jj = 0; jj < table->hl_mapping_multi_res[qq].l2p_direct_info[kk].num_of_access_fields; jj++)
                    {
                        /** look for the corresponding field in the table and set position */
                        for (field_in_table = 0; field_in_table < result_type_set->nof_result_fields; field_in_table++)
                        {
                            if ((result_type_set->results_info[field_in_table].field_id ==
                                 table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].field_id))
                            {
                                break;
                            }
                        }
                        if (field_in_table == result_type_set->nof_result_fields)
                        {
                            SHR_ERR_EXIT(_SHR_E_INTERNAL,
                                         "field %s in L2P not found in table %s as result field, result type %s\n",
                                         dbal_field_to_string(unit,
                                                              table->hl_mapping_multi_res[qq].
                                                              l2p_direct_info[kk].l2p_fields_info[jj].field_id),
                                         table->table_name, result_type_set->result_type_name);
                        }

                        table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].field_pos_in_interface =
                            field_in_table;

                        table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].nof_bits_in_interface =
                            (table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].access_nof_bits !=
                             0) ? table->hl_mapping_multi_res[qq].l2p_direct_info[kk].
                            l2p_fields_info[jj].access_nof_bits : result_type_set->results_info[field_in_table].
                            field_nof_bits;

                        table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].offset_in_interface =
                            table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].access_offset +
                            result_type_set->results_info[field_in_table].bits_offset_in_buffer;

                        /** check that the field is mapped to a valid memory/register */
                        switch (kk)
                        {
                            case DBAL_HL_ACCESS_MEMORY:
                                if (!SOC_MEM_IS_VALID
                                    (unit,
                                     table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].memory))
                                {
                                    SHR_ERR_EXIT(_SHR_E_INTERNAL,
                                                 "The field %s, table %s mapped to invalid memory, result type %s\n",
                                                 dbal_field_to_string(unit,
                                                                      result_type_set->results_info[kk].field_id),
                                                 table->table_name, result_type_set->result_type_name);
                                }
                                break;
                            case DBAL_HL_ACCESS_REGISTER:
                                if (!SOC_REG_IS_VALID
                                    (unit,
                                     table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].reg[0]))
                                {
                                    SHR_ERR_EXIT(_SHR_E_INTERNAL,
                                                 "The field %s, table %s mapped to invalid register, result type %s\n",
                                                 dbal_field_to_string(unit,
                                                                      result_type_set->results_info[kk].field_id),
                                                 table->table_name, result_type_set->result_type_name);
                                }
                                break;
                            case DBAL_HL_ACCESS_SW:
                                result_type_set->results_info[field_in_table].is_sw_field = TRUE;
                                result_type_set->results_info[field_in_table].bytes_offset_in_sw_buffer =
                                    bytes_offset_in_sw_payload;
                                bytes_offset_in_sw_payload +=
                                    BITS2BYTES(result_type_set->results_info[field_in_table].field_nof_bits);
                                break;
                            case DBAL_HL_ACCESS_PEMLA:
                            default:
                                SHR_ERR_EXIT(_SHR_E_INTERNAL, "Invalid access type, %d, for hard-logic table: %s\n",
                                             kk, table->table_name);
                                break;
                        }

                        /** offset internal parameters init */
                        if (table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                            array_offset_info.encode_mode == DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP)
                        {
                            table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                                array_offset_info.internal_inparam =
                                result_type_set->results_info[field_in_table].field_nof_bits;
                        }

                        if (table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                            entry_offset_info.encode_mode == DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP)
                        {
                            table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                                entry_offset_info.internal_inparam =
                                result_type_set->results_info[field_in_table].field_nof_bits;
                        }

                        if (table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                            data_offset_info.encode_mode == DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP)
                        {
                            table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                                data_offset_info.internal_inparam =
                                result_type_set->results_info[field_in_table].field_nof_bits;
                        }

                        if (table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                            block_index_info.encode_mode == DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP)
                        {
                            table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].
                                block_index_info.internal_inparam =
                                result_type_set->results_info[field_in_table].field_nof_bits;
                        }

                        if (table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].alias_data_offset_info.encode_mode 
                            == DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP)
                        {
                            if (table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].memory != INVALIDm )
                            {
                                int mem_size = SOC_MEM_BYTES(unit,table->hl_mapping_multi_res[qq].l2p_direct_info[kk].
                                                             l2p_fields_info[jj].memory);
                                mem_size = BYTES2BITS(mem_size);
                                table->hl_mapping_multi_res[qq].l2p_direct_info[kk].l2p_fields_info[jj].alias_data_offset_info.internal_inparam
                                    = mem_size;                                
                            }
                            else
                            {                                
                                SHR_ERR_EXIT(_SHR_E_INTERNAL, "not supported aliasing with offset to registers\n");
                            }

                        }                        
                    }
                }
            }
        }
        table->is_table_initiated = 1;
    }

exit:
    SHR_FUNC_EXIT;
}
