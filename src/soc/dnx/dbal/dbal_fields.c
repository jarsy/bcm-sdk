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

#include "dbal_internal.h"
#include <shared/bsl.h>

static dbal_field_basic_info_t field_basic_info[DBAL_NOF_FIELDS];

/**************************************************Internal usage APIs *************************************************/
static shr_error_e
dbal_fields_encode_suffix(
    int unit,
    uint32 field_val,
    dbal_fields_e field_id,
    uint32 suffix_val,
    uint32 suffix_offset,
    uint32 * field_val_out)
{

    (*field_val_out) = (field_val >> suffix_offset) | (suffix_val);
    return 0;
}

static shr_error_e
dbal_fields_decode_suffix(
    int unit,
    uint32 field_val,
    dbal_fields_e field_id,
    uint32 suffix_offset,
    uint32 * field_val_out)
{
    (*field_val_out) = (field_val << suffix_offset);

    return 0;
}

static shr_error_e
dbal_fields_encode_prefix(
    int unit,
    uint32 field_val,
    dbal_fields_e field_id,
    uint32 prefix_val,
    uint32 prefix_length,
    uint32 * field_val_out)
{
    prefix_val = prefix_val << prefix_length;
    (*field_val_out) = (prefix_val) | (field_val);
    return 0;
}

static shr_error_e
dbal_fields_decode_prefix(
    int unit,
    uint32 field_val,
    dbal_fields_e field_id,
    uint32 prefix_length,
    uint32 * field_val_out)
{
    uint32 mask = 0;

    mask = (1 << prefix_length) - 1;
    (*field_val_out) = (field_val & mask);
    return 0;
}

static shr_error_e
dbal_fields_decode_enum(
    int unit,
    uint32 field_val,
    dbal_fields_e field_id,
    uint32 num_of_enum_values,
    uint32 field_size,
    uint32 * field_val_out)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < num_of_enum_values; ii++)
    {
        if (field_val == field_basic_info[field_id].enum_val_info[ii].value)
        {
            (*field_val_out) = ii;
            break;
        }
    }
    if (ii == num_of_enum_values)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, " Decoding failed\n");
    }

exit:
    SHR_FUNC_EXIT;

}

shr_error_e
dbal_fields_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_db_init_fields_set_default(unit, field_basic_info));
    SHR_IF_ERR_EXIT(dbal_db_init_fields(unit, DBAL_INIT_FLAGS_NONE, field_basic_info));
    SHR_IF_ERR_EXIT(dbal_db_init_fields_logical_validation(unit, field_basic_info));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_fields_deinit(
    int unit)
{
    int ii;
    dbal_field_basic_info_t *field_entry;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_FIELDS; ii++)
    {
        field_entry = &field_basic_info[ii];
        if (field_entry->nof_child_fields > 0)
        {
            SHR_FREE(field_entry->sub_field_info);
        }
        if (field_entry->nof_enum_values > 0)
        {
            SHR_FREE(field_entry->enum_val_info);
        }
    }
    SHR_FUNC_EXIT;
}

int
dbal_fields_is_instances_support(
    int unit,
    dbal_fields_e field_id)
{
    if (field_basic_info[field_id].instances_support == 1)
    {
        return 1;
    }

    return 0;
}

shr_error_e
dbal_fields_is_field_encoded(
    int unit,
    dbal_fields_e field_id)
{
    switch (field_basic_info[field_id].encode_info.encode_mode)
    {
        case DBAL_VALUE_FIELD_ENCODE_NONE:
            return 0;
            break;

        default:
            return 1;
            break;
    }

    return 0;
}

shr_error_e
dbal_fields_field32_encode(
    int unit,
    dbal_fields_e field_id,
    dbal_fields_e parent_field_id,
    uint32 field_val,
    uint32 * field_value_out)
{
    dbal_value_field_encode_types_e encode_mode = field_basic_info[field_id].encode_info.encode_mode;
    uint32 in_param = field_basic_info[field_id].encode_info.input_param;
    int iter;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * using parent field encoding
     */
    if (parent_field_id != DBAL_FIELD_EMPTY)
    {
        for (iter = 0; iter < field_basic_info[parent_field_id].nof_child_fields; iter++)
        {
            if (field_basic_info[parent_field_id].sub_field_info[iter].sub_field_id == field_id)
            {
                encode_mode = field_basic_info[parent_field_id].sub_field_info[iter].encode_info.encode_mode;
                in_param = field_basic_info[parent_field_id].sub_field_info[iter].encode_info.input_param;
                break;
            }
        }

    }

    switch (encode_mode)
    {
        case DBAL_VALUE_FIELD_ENCODE_NONE:
            (*field_value_out) = field_val;
            break;

        case DBAL_VALUE_FIELD_ENCODE_BOOL:
            if (field_val == 0)
            {
                (*field_value_out) = 0;
            }
            else
            {
                (*field_value_out) = 1;
            }
            break;

        case DBAL_VALUE_FIELD_ENCODE_PREFIX:
            SHR_IF_ERR_EXIT(dbal_fields_encode_prefix(unit, field_val, field_id, in_param,
                                                      field_basic_info[field_id].max_size, field_value_out));
            break;

        case DBAL_VALUE_FIELD_ENCODE_SUFFIX:
            SHR_IF_ERR_EXIT(dbal_fields_encode_suffix(unit, field_val, field_id, in_param,
                                                      field_basic_info[field_id].max_size, field_value_out));
            break;

        case DBAL_VALUE_FIELD_ENCODE_SUBTRACT:
            (*field_value_out) = field_val - in_param;
            break;

        case DBAL_VALUE_FIELD_ENCODE_HARD_VALUE:
            if (field_val == 1)
            {
                (*field_value_out) = in_param;
            }
            else
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "Illegal input value for hard value field value %d\n", field_val);
            }
            break;

        case DBAL_VALUE_FIELD_ENCODE_ENUM:
            if (field_val >= field_basic_info[field_id].nof_enum_values)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "Illegal input value for enum field max value %d\n",
                             field_basic_info[field_id].nof_enum_values);
            }
            (*field_value_out) = field_basic_info[field_id].enum_val_info[field_val].value;
            break;

        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal encoding type for field %s\n", dbal_field_to_string(unit, field_id));
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_fields_field32_decode(
    int unit,
    dbal_fields_e field_id,
    dbal_fields_e parent_field_id,
    uint32 field_val,
    uint32 * field_value_out)
{

    dbal_value_field_encode_types_e encode_mode = field_basic_info[field_id].encode_info.encode_mode;
    uint32 in_param = field_basic_info[field_id].encode_info.input_param;
    int iter;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * using parent  field encoding
     */
    if (parent_field_id != DBAL_FIELD_EMPTY)
    {
        for (iter = 0; iter < field_basic_info[parent_field_id].nof_child_fields; iter++)
        {
            if (field_basic_info[parent_field_id].sub_field_info[iter].sub_field_id == field_id)
            {
                encode_mode = field_basic_info[parent_field_id].sub_field_info[iter].encode_info.encode_mode;
                in_param = field_basic_info[parent_field_id].sub_field_info[iter].encode_info.input_param;
                break;
            }
        }

    }

    switch (encode_mode)
    {
        case DBAL_VALUE_FIELD_ENCODE_NONE:
            (*field_value_out) = field_val;
            break;

        case DBAL_VALUE_FIELD_ENCODE_BOOL:
            if (field_val == 0)
            {
                (*field_value_out) = 0;
            }
            else
            {
                (*field_value_out) = 1;
            }
            break;
        case DBAL_VALUE_FIELD_ENCODE_PREFIX:
            SHR_IF_ERR_EXIT(dbal_fields_decode_prefix(unit, field_val, field_id,
                                                      field_basic_info[field_id].max_size, field_value_out));
            break;

        case DBAL_VALUE_FIELD_ENCODE_SUFFIX:
            SHR_IF_ERR_EXIT(dbal_fields_decode_suffix(unit, field_val, field_id,
                                                      field_basic_info[field_id].max_size, field_value_out));
            break;

        case DBAL_VALUE_FIELD_ENCODE_SUBTRACT:
            (*field_value_out) = field_val + in_param;
            break;

        case DBAL_VALUE_FIELD_ENCODE_HARD_VALUE:
            (*field_value_out) = in_param;
            break;

        case DBAL_VALUE_FIELD_ENCODE_ENUM:
            SHR_IF_ERR_EXIT(dbal_fields_decode_enum(unit, field_val, field_id,
                                                    field_basic_info[field_id].nof_enum_values,
                                                    field_basic_info[field_id].max_size, field_value_out));
            break;
        default:
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "Illegal encoding type for field %s\n", dbal_field_to_string(unit, field_id));
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_fields_field_info_get_ptr(
    int unit,
    dbal_fields_e field_id,
    dbal_field_basic_info_t ** field_info)
{
    (*field_info) = &field_basic_info[field_id];

    return 0;
}

shr_error_e
dbal_fields_is_sub_field(
    int unit,
    dbal_fields_e field_id,
    int *is_sub_field)
{
    (*is_sub_field) = 0;
    if (field_basic_info[field_id].parent_field_id[0] != DBAL_FIELD_EMPTY)
    {
        (*is_sub_field) = 1;
    }

    return 0;
}

shr_error_e
dbal_fields_sub_field_match(
    int unit,
    dbal_fields_e parent_field_id,
    dbal_fields_e sub_field_field_id,
    int *is_found)
{
    int iter;

    (*is_found) = 0;

    for (iter = 0; iter < field_basic_info[parent_field_id].nof_child_fields; iter++)
    {
        if (field_basic_info[parent_field_id].sub_field_info[iter].sub_field_id == sub_field_field_id)
        {
            (*is_found) = 1;
            return 0;
        }
    }
    return 0;
}

shr_error_e
dbal_fields_max_size_get(
    int unit,
    dbal_fields_e field_id,
    uint32 * field_size)
{
    (*field_size) = field_basic_info[field_id].max_size;

    return 0;
}

shr_error_e
dbal_fields_max_value_get(
    int unit,
    dbal_fields_e field_id,
    uint32 * max_value)
{
    (*max_value) = field_basic_info[field_id].max_value;

    return 0;
}

shr_error_e
dbal_fields_type_get(
    int unit,
    dbal_fields_e field_id,
    dbal_field_type_e * field_type)
{
    (*field_type) = field_basic_info[field_id].type;

    return 0;
}

shr_error_e
dbal_fields_encode_type_get(
    int unit,
    dbal_fields_e field_id,
    dbal_value_field_encode_types_e * encode_type)
{
    (*encode_type) = field_basic_info[field_id].encode_info.encode_mode;

    return 0;
}

/**************************************************General usage APIs (outside of dbal *************************************************/
shr_error_e
dbal_fields_sub_field_info_get(
    int unit,
    dbal_fields_e field_id,
    uint32 orig_val,
    dbal_fields_e * sub_field_id,
    uint32 * sub_field_val)
{
    int iter;
    uint32 decoded_value;
    uint32 re_encoded_value;
    dbal_fields_e curr_sub_field_id;

    SHR_FUNC_INIT_VARS(unit);

    (*sub_field_id) = DBAL_FIELD_EMPTY;

    for (iter = 0; iter < field_basic_info[field_id].nof_child_fields; iter++)
    {
        curr_sub_field_id = field_basic_info[field_id].sub_field_info[iter].sub_field_id;
        if (curr_sub_field_id != DBAL_FIELD_EMPTY)
        {
            SHR_IF_ERR_EXIT(dbal_fields_field32_decode(unit, curr_sub_field_id, field_id, orig_val, &decoded_value));
            SHR_IF_ERR_EXIT(dbal_fields_field32_encode(unit, curr_sub_field_id, field_id, decoded_value,
                                                       &re_encoded_value));
            if (re_encoded_value == orig_val)
            {
                (*sub_field_id) = curr_sub_field_id;
                (*sub_field_val) = decoded_value;
                break;
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_fields_field_info_get(
    int unit,
    dbal_fields_e field_id,
    dbal_field_basic_info_t * field_info)
{
    (*field_info) = field_basic_info[field_id];

    return 0;
}

shr_error_e
dbal_fields_parent_field32_value_set(
    int unit,
    dbal_fields_e parent_field_id,
    dbal_fields_e sub_field_id,
    uint32 sub_field_val,
    uint32 * parent_field_val)
{
    return dbal_fields_field32_encode(unit, sub_field_id, parent_field_id, sub_field_val, parent_field_val);
}
