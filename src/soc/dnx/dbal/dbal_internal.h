/*
 * $Id: dbal)internal.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _DBAL_INTERNAL_H_INCLUDED__
#  define _DBAL_INTERNAL_H_INCLUDED__

#  include <soc/dnx/dbal/dbal_structures.h>
#  include <shared/shrextend/shrextend_debug.h>
#  include <shared/utilex/utilex_str.h>
#  include <shared/swstate/access/sw_state_access.h>
#  include <shared/utilex/utilex_hashtable.h>
#  include "dbal_string_mgmt.h"
#  include "dbal_db_init.h"

/**
 * \brief 
 * used for error indocations when working on fields, the error
 * svaed here will be used when performing action on the entry\n 
 */
#define DBAL_FIELD_ERR_HANDLE(string_to_save) LOG_ERROR(BSL_LOG_MODULE,(BSL_META_U(unit,"%s, field %s table %s\n"), \
                                              string_to_save, dbal_field_to_string(unit, field_id),                 \
                                              dbal_logical_table_to_string(unit, entry_handle->table_id)));         \
                                              entry_handle->error_info.field_id = field_id;                         \
                                              entry_handle->error_info.error_exists = 1;                            \
                                              return;

#define DBAL_FIELD_ERR_EXIT_HANDLE(string_to_save) LOG_ERROR(BSL_LOG_MODULE,(BSL_META_U(unit,"%s, field %s table %s\n"), \
                                              string_to_save, dbal_field_to_string(unit, field_id),                 \
                                              dbal_logical_table_to_string(unit, entry_handle->table_id)));         \
                                              entry_handle->error_info.field_id = field_id;                         \
                                              entry_handle->error_info.error_exists = 1;                            \
                                              SHR_ERR_EXIT(_SHR_E_INTERNAL,"");

#define DBAL_ENTRY_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, field_instance_id) \
    dbal_entry_handle_t *entry_handle;                                                            \
    if (dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle))                     \
    {                                                                                             \
        DBAL_FIELD_ERR_HANDLE("Illegal handle ID");                                               \
    }                                                                                             \
    if (field_instance_id != INST_SINGLE)                                                         \
    {                                                                                             \
      dbal_field_basic_info_t field_info;                                                         \
      dbal_fields_field_info_get(unit, field_id, &field_info);                                    \
      if (field_info.instances_support)                                                           \
      {                                                                                           \
          field_id = field_id + field_instance_id;                                                \
      }                                                                                           \
      else                                                                                        \
      {                                                                                           \
          DBAL_FIELD_ERR_HANDLE("Field does not support multiple instances");                     \
      }                                                                                           \
    }

#define DBAL_ENTRY_HANDLE_FIELD_OPERATION_VALIDATION(unit, entry_handle_id, field_id, field_instance_id) \
do                                                                                                \
{                                                                                                 \
    if (dbal_entry_handle_get_internal(unit, entry_handle_id, &entry_handle))                     \
    {                                                                                             \
        DBAL_FIELD_ERR_EXIT_HANDLE("Illegal handle ID");                                          \
    }                                                                                             \
    if (field_instance_id != INST_SINGLE)                                                         \
    {                                                                                             \
      dbal_field_basic_info_t field_info;                                                         \
      dbal_fields_field_info_get(unit, field_id, &field_info);                                    \
      if (field_info.instances_support)                                                           \
      {                                                                                           \
          field_id = field_id + field_instance_id;                                                \
      }                                                                                           \
      else                                                                                        \
      {                                                                                           \
          DBAL_FIELD_ERR_EXIT_HANDLE("Field does not support multiple instances");                \
      }                                                                                           \
    }                                                                                             \
}                                                                                                 \
while (0)

#define DBAL_PRINT_FRAME_FOR_ACCESS_PRINTS(is_start, sevirity)                          \
do                                                                                      \
{                                                                                       \
    bsl_severity_t access_svr;                                                          \
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALACCESSDNX, access_svr);      \
    if (access_svr >= sevirity)                                                         \
    {                                                                                   \
        LOG_CLI((BSL_META("----------------------------------------------------------------------------\n")));  \
        if (!is_start)                                                                  \
        {                                                                               \
            LOG_CLI((BSL_META("\n")));                                                  \
        }                                                                               \
    }                                                                                   \
}                                                                                       \
while (0)

#define DBAL_PRINT_FRAME_FOR_API_PRINTS(is_start, sevirity)                             \
do                                                                                      \
{                                                                                       \
    bsl_severity_t api_svr;                                                             \
    SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_SOCDNX_DBALDNX, api_svr);               \
    if (api_svr >= sevirity)                                                            \
    {                                                                                   \
        LOG_CLI((BSL_META("****************************************************************************\n")));  \
        if (!is_start)                                                                  \
        {                                                                               \
            LOG_CLI((BSL_META("\n")));                                                  \
        }                                                                               \
    }                                                                                   \
}                                                                                       \
while (0)

/**
 * \brief 
 * Path to the dbal sw tabes in sw state
 */
#define DBAL_SW_STATE_TABLES sw_state_access[unit].dnx.socdnx.dbal_db.sw_tables

/************** DBAL internal functionality (APIs from: dbal_internal.c) **************/
/** 
 * \brief returns 1 if dbal initiated, otherwise returns 0.
 */
uint8 dbal_is_intiated(
    int unit);

/** 
 * \brief indicates that dbal init preformed 
 */
void dbal_initiated_set(
    int unit);

/** 
 * \brief indicates that dbal deinit preformed 
 */
void dbal_initiated_reset(
    int unit);

/**
 *\brief prints the action preformed and entry information
 *  The prints are in INFO level, with respect to the DBAL API
 *  severity
 */
shr_error_e dbal_action_prints(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags,
    char *src_str);

/**
 *  \brief The function validate entry handle inforamtion before
 *         performing an action
 */
shr_error_e dbal_action_validations(
    int unit,
    uint32 entry_handle_id,
    int is_value_fields_required);

/**
 *\brief allocate SW entry handle from pool. this handle  is used to preform table actions 
 */
shr_error_e dbal_entry_handle_take_internal(
    int unit,
    dbal_tables_e table_id,
    uint32 * entry_handle_id);

/**
 *\brief return handle to pool. must be called after taking an handle. 
 *          entry APIs (commit/get/delete) will return the handle automaticlly (see also DBAL_COMMIT_KEEP_HANDLE)
 */
void dbal_entry_handle_release_internal(
    int unit,
    uint32 entry_handle_id);

/**
 *\brief get handle info.
 */
shr_error_e dbal_entry_handle_get_internal(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_handle_t ** entry_handle);

/**
 *\brief get default handle info.  
 * default handle use internally for set_default API 
 */
void dbal_entry_handle_default_get_internal(
    int unit,
    dbal_entry_handle_t ** entry_handle);

/**
 *\brief set a key field in the handle
 */
void dbal_entry_key_field_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    uint32 * field_val,
    uint32 * field_mask);

/**
 *\brief set a value field in the handle
 */
void dbal_entry_value_field_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    uint32 * field_val);

/**
 *\brief set a pointer to a value that will be updated after entry_get
 */
void dbal_entry_value_field_get(
    int unit,
    dbal_field_type_e field_type,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    void *field_val,
    void *field_mask);

/**
 *\brief copy the field value from entry buffer to given 
 *       pointer
 */
shr_error_e dbal_entry_handle_value_field_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int inst_id,
    uint32 * field_val);

/**
 * \brief prints entry handle buffer. 
 * entry print severity is INFO level, buffer print severity 
 * DEBUG level, prints all the key and value fields that was 
 * added to the handle. 
 */
shr_error_e dbal_entry_print(
    int unit,
    uint32 entry_handle_id,
    uint8 format);

/**
 * \brief prints physical entry buffer. 
 * Prints level is verbose - default. 
 * If a prints_severity is not null. prints severity will be 
 * according to input 
 */
shr_error_e dbal_phy_entry_print(
    int unit,
    dbal_physical_entry_t * entry,
    uint8 print_only_key,
    int prints_severity);

/**
 *\brief get key field from buffer, according to field ID
 */
shr_error_e dbal_entry_key_field_from_handle_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    dbal_fields_e field_id,
    uint32 * field_val_returned,
    uint32 * is_found);

/**
 *\brief set key fields in handle pointers with the 
 *       corresponding values from buffer. Used in iterator,
 *       when the buffer itself use as the keys for iterator
 *       perfoemance. there is a special treatment for core_id
 */
shr_error_e dbal_set_key_info_from_buffer(
    int unit,
    dbal_entry_handle_t * entry_handle,
    dbal_field_data_t * keys_info,
    uint8 * is_valid);

/** 
 * \brief Read result type from buffer. 
 * called after 'get all fields' action for multiple results tables, 
 * in order to retrive the result type value of the entry. 
 * After resolving the result type we can update the requested value fields
 */
shr_error_e dbal_set_result_type_from_buffer(
    int unit,
    dbal_entry_handle_t * entry_handle);

/** 
 * \brief Update the requested fields info in handle. 
 * In multiple results tables, the requested fields info is 
 * known after resolving the result type. 
 */
shr_error_e dbal_handle_update_requested_fields(
    int unit,
    dbal_entry_handle_t * entry_handle,
    dbal_field_data_t fields_array[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS]);

/**
 *\brief update values for all the requested payload fields from the handle 
 *(sets entry_handle->user_output_info[iter].returned_pointer with the parsed value).
 */
shr_error_e dbal_entry_handle_value_fields_parse(
    int unit,
    dbal_entry_handle_t * entry_handle);

/**
 *\brief get field value from buffer, handles encoding and sub field. the value received id the logical value.
 */
shr_error_e dbal_field_from_buffer_get(
    int unit,
    dbal_table_field_info_t * table_field_info,
    dbal_fields_e field_id,
    uint32 * buffer,
    uint32 * returned_field_value);

/**
 *\brief prints the field value according to field type. in case parent_field != DBAL_FIELD_EMPTY field is a subfield
 */
shr_error_e dbal_field_from_buffer_print(
    int unit,
    dbal_fields_e field_id,
    dbal_tables_e table_id,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES],
    uint32 field_mask[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES],
    int result_type_idx,
    uint8 is_key,
    uint8 is_full_buffer);

/************** Field internal APIs (from: dbal_fields.c) **************/
shr_error_e dbal_fields_init(
    int unit);

shr_error_e dbal_fields_deinit(
    int unit);

int dbal_fields_is_instances_support(
    int unit,
    dbal_fields_e field_id);

shr_error_e dbal_fields_is_field_encoded(
    int unit,
    dbal_fields_e field_id);

shr_error_e dbal_fields_field32_encode(
    int unit,
    dbal_fields_e field_id,
    dbal_fields_e parent_field_id,
    uint32 field_val,
    uint32 * field_value_out);

shr_error_e dbal_fields_field32_decode(
    int unit,
    dbal_fields_e field_id,
    dbal_fields_e parent_field_id,
    uint32 field_val,
    uint32 * field_value_out);

shr_error_e dbal_fields_field_info_get_ptr(
    int unit,
    dbal_fields_e field_id,
    dbal_field_basic_info_t ** field_info);

/**
 *\brief if field has parent fields returns is_sub_field = 1
 */
shr_error_e dbal_fields_is_sub_field(
    int unit,
    dbal_fields_e field_id,
    int *is_sub_field);

/**
 *\brief checks if a field is a sub field of an other field
 */
shr_error_e dbal_fields_sub_field_match(
    int unit,
    dbal_fields_e parent_field_id,
    dbal_fields_e sub_field_field_id,
    int *is_found);

shr_error_e dbal_fields_max_size_get(
    int unit,
    dbal_fields_e field_id,
    uint32 * field_size);

shr_error_e dbal_fields_max_value_get(
    int unit,
    dbal_fields_e field_id,
    uint32 * max_value);

shr_error_e dbal_fields_type_get(
    int unit,
    dbal_fields_e field_id,
    dbal_field_type_e * field_type);

shr_error_e dbal_fields_encode_type_get(
    int unit,
    dbal_fields_e field_id,
    dbal_value_field_encode_types_e * encode_type);

/************** Logical table APIs  (from: dbal_tables.c) **************/
shr_error_e dbal_logical_tables_init(
    int unit);

shr_error_e dbal_logical_tables_deinit(
    int unit);

shr_error_e dbal_logical_table_reference_get(
    int unit,
    dbal_tables_e table_id,
    dbal_logical_table_t ** table);

/**
 * \brief check if a given field is part of the table key fields
 * (directly or as child field)
 */
shr_error_e dbal_table_field_is_key(
    int unit,
    dbal_tables_e table_id,
    dbal_fields_e field_id,
    uint8 * is_key);

shr_error_e dbal_table_default_entry_get(
    int unit,
    dbal_tables_e table_id,
    uint32 * payload_buffer);

/************** Physical table APIs **************/
shr_error_e dbal_phy_table_entry_add(
    int unit,
    dbal_physical_tables_e physical_db_id,
    uint32 app_id,
    dbal_physical_entry_t * phy_entry);
shr_error_e dbal_phy_table_entry_get(
    int unit,
    dbal_physical_tables_e physical_db_id,
    uint32 app_id,
    dbal_physical_entry_t * phy_entry);
shr_error_e dbal_phy_table_entry_delete(
    int unit,
    dbal_physical_tables_e physical_db_id,
    uint32 app_id,
    dbal_physical_entry_t * phy_entry);
shr_error_e dbal_phy_table_iterator_init(
    int unit,
    dbal_iterator_info_t * iterator_info);
shr_error_e dbal_phy_table_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info);
shr_error_e dbal_phy_table_iterator_deinit(
    int unit,
    dbal_iterator_info_t * iterator_info);
/************** Direct access APIs **************/
shr_error_e dbal_direct_entry_set_default(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_direct_entry_set(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_direct_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    uint8 must_res_type_resolved);
shr_error_e dbal_direct_entry_delete(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_direct_table_clear(
    int unit,
    uint32 entry_handle_id);
shr_error_e dbal_direct_table_iterator_init(
    int unit,
    dbal_iterator_info_t * iterator_info);
shr_error_e dbal_direct_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info);
/************** sw state tables/fields APIs **************/
shr_error_e dbal_sw_table_direct_entry_set(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_direct_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle,
    int * res_type_get);
shr_error_e dbal_sw_table_direct_entry_clear(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_direct_iterator_init(
    int unit,
    dbal_tables_e table_id,
    uint32 key_size,
    dbal_sw_table_iterator_t * sw_iterator);
shr_error_e dbal_sw_table_direct_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info);
shr_error_e dbal_sw_table_hash_entry_add(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_hash_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_hash_entry_delete(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_hash_iterator_init(
    int unit,
    dbal_tables_e table_id,
    dbal_sw_table_iterator_t * sw_iterator);
shr_error_e dbal_sw_table_hash_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info);
shr_error_e dbal_sw_table_entry_set(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_entry_get(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_entry_clear(
    int unit,
    dbal_entry_handle_t * entry_handle);
shr_error_e dbal_sw_table_clear(
    int unit,
    uint32 entry_handle_id);
shr_error_e dbal_sw_table_iterator_init(
    int unit,
    dbal_iterator_info_t * iterator_info);
shr_error_e dbal_sw_table_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info);

shr_error_e dbal_iterator_init_handle_info(
    int unit,
    dbal_iterator_info_t * iterator_info);

#endif/*_DBAL_INCLUDED__*/
