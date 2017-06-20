/*
 * $Id: dpp_dbal.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _DBAL_H_INCLUDED__
#  define _DBAL_H_INCLUDED__

#  ifndef BCM_DNX_SUPPORT
#    error "This file is for use by DNX (JR2) family only!"
#  endif

#  include <soc/dnx/dbal/dbal_structures.h>

/**
 *\brief sets information related to a iterator rule can be called before dbal_table_iterator_get_next() 
 *
 * \par DIRECT INPUT:
 *   \param [in]  dbal_iterator - iterator info\n
 *   \param [in]  field_id      - the field ID  related to the condition \n
 *   \param [in]  field_val     - value to be compared\n
 *   \param [in]  condition     - condition (equal_to / bigger_than...)\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
#define DBAL_ITERATOR_RULE_SET(dbal_iterator, field_id, field_val, condition)       \
do                                                                                  \
{                                                                                   \
    dbal_iterator.rule.field_id = field_id;                                         \
    dbal_iterator.rule.field_val = field_val;                                       \
    dbal_iterator.rule.condition = condition;                                       \
}                                                                                   \
while (0)

/**************************************************DBAL APIs (from:dbal_api.c) *********************************************/
/**
 * \brief preform init to module. allocating SW_state, loading logical table and fields XMLs 
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
    shr_error_e dbal_init(
    int unit);


shr_error_e dbal_deinit(
    int unit);

/**
 *\brief return the severity of the logger for DBAL. 
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] dbal_logger_type -  dbal modul type
 *          API/ACCESS\n
 *   \param [in] severity -  pointer to the requested logger
 *          severity\n
 *  \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the sevirity pointer \n
 */
shr_error_e dbal_log_severity_get(
    int unit,
    dnx_dbal_logger_type_e dbal_logger_type,
    bsl_severity_t * severity);

/**
 *\brief sets the severity of the logger for DBAL. 
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] dbal_logger_type -  dbal module type
 *          API/ACESS\n
 *   \param [in] severity -  logger severity to set\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_log_severity_set(
    int unit,
    dnx_dbal_logger_type_e dbal_logger_type,
    int severity);

/**
 *\brief allocate SW entry handle from pool. this handle  is used to preform table actions.
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] table_id -  the requested table to allocate handle for\n
 *   \param [in] entry_handle_id -  pointer to the returned handle ID\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the handle pointer with the ID allocated\n
 */
shr_error_e dbal_entry_handle_take(
    int unit,
    dbal_tables_e table_id,
    uint32 * entry_handle_id);

/**
 *\brief return handle to pool. must be called after taking an handle. 
 *          entry APIs (commit/get/delete) will return the handle automaticlly (see also DBAL_COMMIT_KEEP_HANDLE)
 *
 * \par DIRECT INPUT:
 *   \param [in] unit -  \n
 *   \param [in] entry_handle_id -  handle ID to return\n
 */
void dbal_entry_handle_release(
    int unit,
    uint32 entry_handle_id);

/**
 *\brief get handle info.  used mainly for debug perpuses
 *
 * \par DIRECT INPUT:
 *   \param [in] unit -  \n
 *   \param [in] entry_handle_id -  the ID to get \n
 *   \param [in] entry_handle -  a pointer to the handle info returned\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the handle_info pointer \n
 */
shr_error_e dbal_entry_handle_info_get(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_handle_t * entry_handle);

/***********SET APIs ***********/

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_key_field8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 field_val);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_key_field16_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint16 field_val);

/**
 *\brief sets a field value in the entry. No error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_key_field32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 field_val);

/**
 *\brief sets a field value in the entry. No error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry. The number of bits taken from the array is according to the 
 * field definition in the table.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_key_field_arr8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 * field_val);

/**
 *\brief sets a field value in the entry. No error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry. The number of bits taken from the array is according to the 
 * field definition in the table.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_key_field_arr32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 * field_val);

/*
 * masked 
 */
/**
 *\brief sets a field value in the entry. No error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry. The number of bits taken from the field is according to the 
 * field size definitions in the table.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 *   \param [in]  field_mask - the mask to set \n
 */
void dbal_entry_key_field8_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 field_val,
    uint8 field_mask);

/**
 *\brief sets a field value in the entry. No error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 *   \param [in]  field_mask - the mask to set \n
 */
void dbal_entry_key_field16_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint16 field_val,
    uint16 field_mask);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 *   \param [in]  field_mask - the mask to set \n
 */
void dbal_entry_key_field32_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 field_val,
    uint32 field_mask);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 *   \param [in]  field_mask - the mask to set \n
 */
void dbal_entry_key_field_arr8_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 * field_val,
    uint8 * field_mask);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 *   \param [in]  field_mask - the mask to set \n
 */
void dbal_entry_key_field_arr32_masked_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 * field_val,
    uint32 * field_mask);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_value_field8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 field_val);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_value_field16_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint16 field_val);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_value_field32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 field_val);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_value_field_arr8_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 * field_val);

/**
 *\brief sets a field value in the entry. no error is returned from this function, all errors related to this field 
 * will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate \n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the value to set \n
 */
void dbal_entry_value_field_arr32_set(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 * field_val);

/***********GET APIs ***********/

/**
 *\brief used to get field info from the entry. in this API only setting the pointer that will used to retrive the value. 
 * the actual value is returned only after calling dbal_entry_get()
 * no error is returned from this function, all errors related to this field will be received when commit/get/delete the entry.
 * The array size should be at least the size of the requested field as defined in the table.
 * 
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate\n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in] field_val - the pointer that will be returned the value \n
 */
void dbal_entry_value_field8_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 * field_val);

/**
 *\brief used to get field info from the entry. in this API only setting the pointer that will used to retrive the value. 
 * the actual value is returned only after calling dbal_entry_get()
 * no error is returned from this function, all errors related to this field will be received when commit/get/delete the entry.
 * 
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate\n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in] field_val - the pointer that will be returned the value \n
 */
void dbal_entry_value_field16_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint16 * field_val);

/**
 *\brief used to get field info from the entry. in this API only setting the pointer that will used to retrive the value. 
 * the actual value is returned only after calling dbal_entry_get()
 * no error is returned from this function, all errors related to this field will be received when commit/get/delete the entry.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate\n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in] field_val - the pointer that will be returned the value \n
 */
void dbal_entry_value_field32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 * field_val);

/**
 *\brief used to get field info from the entry. in this API only setting the pointer that will used to retrive the value. 
 * the actual value is returned only after calling dbal_entry_get()
 * no error is returned from this function, all errors related to this field will be received when commit/get/delete the entry.
 * The array size should be at least the size of the requested field as defined in the table.
 * 
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate\n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in] field_val - the pointer that will be returned the value \n
 */
void dbal_entry_value_field_arr8_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES]);

/**
 *\brief used to get field info from the entry. in this API only setting the pointer that will used to retrive the value. 
 * the actual value is returned only after calling dbal_entry_get()
 * no error is returned from this function, all errors related to this field will be received when commit/get/delete the entry.
 * The array size should be at least the size of the requested field as defined in the table. 
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  entry_handle_id - the entry handle\n
 *   \param [in]  field_id  - the field to relate\n
 *   \param [in]  field_instance_id  - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in] field_val - the pointer that will be returned the value \n
 */
void dbal_entry_value_field_arr32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS]);

/**
 * \brief returns value fields from entry handle when using entry_get with flag DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit \n
 *   \param [in]  entry_handle_id - the handle to preform the action \n
 *   \param [in]  field_id - the field to relate \n
 *   \param [in]  field_instance_id - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the pointer that will be returned the value\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_entry_handle_value_field32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 * field_val);

/**
 * \brief returns value fields from entry handle when using entry_get with flag DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit \n
 *   \param [in]  entry_handle_id - the handle to preform the action \n
 *   \param [in]  field_id - the field to relate \n
 *   \param [in]  field_instance_id - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the pointer that will be returned the value\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_entry_handle_value_field_arr8_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint8 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES]);

/**
 * \brief returns value fields from entry handle when using entry_get with flag DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit \n
 *   \param [in]  entry_handle_id - the handle to preform the action \n
 *   \param [in]  field_id - the field to relate \n
 *   \param [in]  field_instance_id - for multiple instances fields, the instance number use INST_SINGLE when 
    no multiple insances in table. use INST_ALL to configure all fields instances \n
 *   \param [in]  field_val - the pointer that will be returned the value\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_entry_handle_value_field_arr32_get(
    int unit,
    uint32 entry_handle_id,
    dbal_fields_e field_id,
    int field_instance_id,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS]);

/**
 * \brief get entry from HW according to flags, update the pointers that was set to the handle by dbal_entry_value_field_XXX_get().
 * returns the information of the requested fields only.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit \n
 *   \param [in]  entry_handle_id - the handle to preform the action \n
 *   \param [in]  flags - requested flag, default value: DBAL_COMMIT_NORMAL\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_entry_get(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags);

/**
 * \brief set the entry to HW according to flags.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit \n
 *   \param [in]  entry_handle_id - the handle to preform the action \n
 *   \param [in]  flags - requested flag, default value: DBAL_COMMIT_NORMAL\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_entry_commit(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags);

/**
 * \brief clear the entry from HW according to flags.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit \n
 *   \param [in]  entry_handle_id - the handle to preform the action \n
 *   \param [in]  flags - requested flag, default value: DBAL_COMMIT_NORMAL\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_entry_clear(
    int unit,
    uint32 entry_handle_id,
    dbal_entry_action_flags_e flags);


shr_error_e dbal_table_clear(
    int unit,
    dbal_tables_e table_id);

/**
 *\brief init the iterator, must be called first. the iterator allow to run over all entries related to a specific table 
 *
 *IMPORTANT NOTE: when using the iterator no other operation allowed on the table.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit          -\n
 *   \param [in]  table_id      - the related table of the iterator\n
 *   \param [in]  mode          - which entries are valid for the iterator\n
 *   \param [in] iterator_info  - a pointer to the returned value the info of the iterator\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the iterator_info pointer \n
 */
shr_error_e dbal_table_iterator_init(
    int unit,
    dbal_tables_e table_id,
    dbal_iterator_mode_e mode,
    dbal_iterator_info_t * iterator_info);

/**
 *\brief return the next entry of the table. 
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in] iterator_info - in: the last entry info returned. out:next entry info\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the iterator_info pointer with the updated info (next entry found)\n
 */
shr_error_e dbal_table_iterator_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info);

/**
 *\brief destroy the iterator, must be called when finishing to work with the iterator.
 *
 * \par DIRECT INPUT:
 *   \param [in]  unit -\n
 *   \param [in]  iterator_info - the iterator that \n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 */
shr_error_e dbal_table_iterator_destroy(
    int unit,
    dbal_iterator_info_t * iterator_info);

/*************************Fields operations (from: dbal fields.c) *************************/
/**
 *\brief returns the decoded value and sub-field according to the encoded value added. 
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] field_id -  parent field ID\n
 *   \param [in] orig_val -  field orig value\n
 *   \param [in] sub_field_id -  a pointer to the requested sub field ID that feets to the encoding\n
 *   \param [in] sub_field_val -  a pointer to the requested the value decoded\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the sub_field_id and sub_field_value pointers \n
 */
shr_error_e dbal_fields_sub_field_info_get(
    int unit,
    dbal_fields_e field_id,
    uint32 orig_val,
    dbal_fields_e * sub_field_id,
    uint32 * sub_field_val);

/**
 *\brief returns the field info related to field ID. 
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] field_id -  requested field\n
 *   \param [in] field_info -  pointer to the requested field info\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the field_info pointer \n
 */
shr_error_e dbal_fields_field_info_get(
    int unit,
    dbal_fields_e field_id,
    dbal_field_basic_info_t * field_info);

/**
 * \brief Return encoded parent field value, given child field. 
 *        Encoding is applicable to field size up to 32 bit.
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] parent_field_id - field id to be encoded\n
 *   \param [in] sub_field_id -  sub field id\n
 *   \param [in] sub_field_val - sub field value to be encoded
 *          and encapsulated into parent_field_val\n
 *   \param [in] parent_field_val - pointer to the parent field
 *          id to be filled with encoded value\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the field_info pointer \n
 */
shr_error_e dbal_fields_parent_field32_value_set(
    int unit,
    dbal_fields_e parent_field_id,
    dbal_fields_e sub_field_id,
    uint32 sub_field_val,
    uint32 * parent_field_val);

/************************* Logical Table operations (from: dbal_tables.c) *************************/
/**
 * \brief return the logical table info pointer, error if table not found. 
 *  
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] table_id -  the table to look for\n
 *   \param [in] table -  a pointer to the requested table\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       return a pointer to the table\n
 */
shr_error_e dbal_logical_table_get(
    int unit,
    dbal_tables_e table_id,
    CONST dbal_logical_table_t ** table);

/**
 * \brief return table field info, error if field not related to table.
 * incase that the field is sub field in a table returns the parent field ID info.
 *
 * \par DIRECT INPUT:
 *   \param [in] unit
 *   \param [in] table_id -  the table to look for\n
 *   \param [in] field_id -  the field requested\n
 *   \param [in] is_key -  indication if the requested fireld is key or value field\n
 *   \param [in] result_type_idx -  valid in cases of value fields are requested. indicate the result type index\n
 *   \param [in] field_info -  pointer to the requested field info\n
 * \par DIRECT OUTPUT:
 *   Non-zero(shr_error_e) in case of an error
 * \par INDIRECT OUTPUT
 *       filled the field_info pointer \n
 */
shr_error_e dbal_table_field_info_get(
    int unit,
    dbal_tables_e table_id,
    dbal_fields_e field_id,
    uint8 is_key,
    int result_type_idx,
    dbal_table_field_info_t * field_info);

/**
 * \brief - Return the app db id that associates with the
 *        logical table. if table is not MDB table an error will
 *        be return.
 *        Notice, not all MDB tables have valid app db id, if
 *        invalid U32_MAX will be set
 *  
 * \par DIRECT_INPUT:
 *   \param [in] unit -
 *   \param [in] table_id - the table to look for\n
 *   \param [in] app_db_id - the physical app db id associate
 *          with the logical table
 * \par DIRECT OUTPUT:
 *   shr_error_e
 * \par INDIRECT OUTPUT
 *   \param [out] app_db_id - the physical app db id associate
 *          with the logical table
 */
shr_error_e dbal_table_get_app_db_id(
    int unit,
    dbal_tables_e table_id,
    uint32 * app_db_id);

#endif/*_DBAL_INCLUDED__*/
