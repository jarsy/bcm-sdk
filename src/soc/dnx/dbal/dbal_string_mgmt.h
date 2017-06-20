/**
 * \file dbal_string_mgmt.h
 * $Id$
 *
 * Main functions for dbal strings (names) conversion
 * basically string_to_id and to_string functions
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _DBAL_STRING_MGMT_INCLUDED__
#  define _DBAL_STRING_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
#  include <shared/shrextend/shrextend_debug.h>
#  include <soc/dnx/dbal/dbal_structures.h>

/*************
 * FUNCTIONS *
 *************/
/** ****************************************************
* \brief
* translate a string to field ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] field_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    \param [out] field_id - \n
*****************************************************/
shr_error_e dbal_field_string_to_id(
    int unit,
    char *str,
    dbal_fields_e * field_id);
/** ****************************************************
* \brief
* translate a string to logical table ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] log_table_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    log_table_id - \n
*****************************************************/
shr_error_e dbal_logical_table_string_to_id(
    int unit,
    char *str,
    dbal_tables_e * log_table_id);
/** ****************************************************
* \brief
* translate a string to logical table ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] hw_entity_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    log_table_id - \n
*****************************************************/
shr_error_e dbal_hw_entity_string_to_id(
    int unit,
    char *str,
    int *hw_entity_id);
/** ****************************************************
* \brief
* translate a string to physical table ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] phy_table_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    \param [out] phy_table_id - \n
*****************************************************/
shr_error_e dbal_physical_table_string_to_id(
    int unit,
    char *str,
    dbal_physical_tables_e * phy_table_id);
/** ****************************************************
* \brief
* translate a string to label ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] label_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    \param [out] label_id - \n
*****************************************************/
shr_error_e dbal_label_string_to_id(
    int unit,
    char *str,
    dbal_labels_e * label_id);
/** ****************************************************
* \brief
* translate a string to core mode ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] core_mode_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    \param [out] core_mode_id - \n
*****************************************************/
shr_error_e dbal_core_mode_string_to_id(
    int unit,
    char *str,
    dbal_core_mode_e * core_mode_id);
/** ****************************************************
* \brief
* translate a string to condition type ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] condition_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    \param [out] condition_id - \n
*****************************************************/
shr_error_e dbal_condition_string_to_id(
    int unit,
    char *str,
    dbal_condition_types_e * condition_id);
/** ****************************************************
* \brief
* translate a string to field type ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] field_type_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    \param [out] field_type_id - \n
*****************************************************/
shr_error_e dbal_field_type_string_to_id(
    int unit,
    char *str,
    dbal_field_type_e * field_type_id);
/** ****************************************************
* \brief
* translate a string to encode type ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] encode_type_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    \param [out] encode_type_id - \n
*****************************************************/
shr_error_e dbal_field_encode_type_string_to_id(
    int unit,
    char *str,
    dbal_value_field_encode_types_e * encode_type_id);
/** ****************************************************
* \brief
* translate a string to memory encode type ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] encode_type_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    mem_encode_type_id -
*****************************************************/
shr_error_e dbal_offset_encode_type_string_to_id(
    int unit,
    char *str,
    dbal_value_offset_encode_types_e * encode_type_id);
/** ****************************************************
* \brief
* translate a string to memory table type ID.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] str - \n
*      string to translate
*    \param [in] table_type_id - \n
*      pointer to output id
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*  \par INDIRECT OUTPUT:
*    table_type_id -
*****************************************************/
shr_error_e dbal_logical_table_type_string_to_id(
    int unit,
    char *str,
    dbal_table_type_e * table_type_id);
/** ****************************************************
* \brief
* translate field ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] field_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*****************************************************/
char *dbal_field_to_string(
    int unit,
    dbal_fields_e field_id);
/** ****************************************************
* \brief
* translate logical table ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] log_table_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_logical_table_to_string(
    int unit,
    dbal_tables_e log_table_id);
/** ****************************************************
* \brief
* translate physical table ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] phy_table_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_physical_table_to_string(
    int unit,
    dbal_physical_tables_e phy_table_id);
/** ****************************************************
* \brief
* translate label ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] label_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_label_to_string(
    int unit,
    dbal_labels_e label_id);
/** ****************************************************
* \brief
* translate HW entity ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] hw_entity_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*****************************************************/
char *dbal_hw_entity_to_string(
    int unit,
    int hw_entity_id);
/** ****************************************************
* \brief
* translate core mode ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] core_mode_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_core_mode_to_string(
    int unit,
    dbal_core_mode_e core_mode_id);
/** ****************************************************
* \brief
* translate conrition type ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] condition_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_condition_to_string(
    int unit,
    dbal_condition_types_e condition_id);
/** ****************************************************
* \brief
* translate field type ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] field_type_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_field_type_to_string(
    int unit,
    dbal_field_type_e field_type_id);
/** ****************************************************
* \brief
* translate encode type ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] encode_type_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_field_encode_type_to_string(
    int unit,
    dbal_value_field_encode_types_e encode_type_id);

/** ****************************************************
* \brief
* translate encode type ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] encode_type_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_offset_encode_type_to_string(
    int unit,
    dbal_value_offset_encode_types_e encode_type_id);

/** ****************************************************
* \brief
* function add conjunction for encoding print 
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] encode_type_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_offset_encode_conjunction_string(
    int unit,
    dbal_value_offset_encode_types_e encode_type_id);

/** ****************************************************
* \brief
* translate action flag ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] action_flag_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_action_flags_to_string(
    int unit,
    dbal_entry_action_flags_e action_flag_id);

/** ****************************************************
* \brief
* translate access method ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] access_method_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_access_method_to_string(
    int unit,
    dbal_access_method_e access_method_id);

/** ****************************************************
* \brief
* translate table type ID to string. In case of unknown id, a default unknown string will be returned
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] table_type_id - \n
*      ID to translate
*  \par INDIRECT INPUT:
*    None
*  \par DIRECT OUTPUT:
*    char * - \n
*      output string
*  \par INDIRECT OUTPUT:
*    None
*****************************************************/
char *dbal_logical_table_type_to_string(
    int unit,
    dbal_table_type_e table_type_id);

#endif
