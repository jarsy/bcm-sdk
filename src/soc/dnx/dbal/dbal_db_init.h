/**
 * \file dbal_db_init.h
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Main functions for init the dbal fields and logical tables DB
 *
 */

#ifndef _DBAL_DB_INIT_INCLUDED__
#  define _DBAL_DB_INIT_INCLUDED__

/*************
 * INCLUDES   *
 *************/
#  include <soc/dnx/dbal/dbal_structures.h>
#  include <shared/dbx/dbx_xml.h>
#  include <shared/dbx/dbx_file.h>

/*************
 *  DEFINES  *
 *************/
/**
 * regular init procedue
 */
#  define DBAL_INIT_FLAGS_NONE        0x0

/**
 * xml parsing validation indication
 */
#  define DBAL_INIT_FLAGS_VALIDATION  0x1

/** 
 * identifier for dnx data reference 
 */
#  define DB_INIT_DNX_DATA_INDICATION                     "DNX_DATA"

/**
 * path to fields xml file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_DIR_PATH_FIELDS                         "dbal/fields"

/**
 * path to mdb tables xml file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_DIR_PATH_TABLES_MDB                     "dbal/tables_mdb"

/**
 * path to hard logic tables xml file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_DIR_PATH_TABLES_HARD_LOGIC              "dbal/tables_hard_logic"

/**
 * path to sw state tables xml file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_DIR_PATH_TABLES_SW                      "dbal/tables_sw_state"

/**
 * path to pemla tables xml file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_DIR_PATH_TABLES_PEMLA                   "dbal/tables_pemla"

/**
 * path to mdb tables xml validation file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_VALIDATION_TEST_PATH_FIELDS             "dbal/validation_test/fields"

/**
 * path to mdb tables xml validation file(s) (relative to DB 
 * path folder) 
 */
#  define DB_INIT_VALIDATION_TEST_PATH_TABLES_MDB         "dbal/validation_test/tables_mdb"

/**
 * path to hard logic tables xml validation file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_VALIDATION_TEST_PATH_TABLES_HARD_LOGIC  "dbal/validation_test/tables_hard_logic"

/**
 * path to sw state tables xml validation file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_VALIDATION_TEST_PATH_TABLES_SW          "dbal/validation_test/tables_sw_state"

/**
 * path to pemla tables xml validation file(s) 
 * (relative to DB path folder)
 */
#  define DB_INIT_VALIDATION_TEST_PATH_TABLES_PEMLA       "dbal/validation_test/tables_pemla"

#  define DBAL_DB_GET_TABLES_DIR_PATH(table_type,flags)                                           \
  (((flags & DBAL_INIT_FLAGS_VALIDATION) != 0)&&(table_type == DBAL_ACCESS_METHOD_HARD_LOGIC)) ?  \
    DB_INIT_VALIDATION_TEST_PATH_TABLES_HARD_LOGIC : (                                            \
  (((flags & DBAL_INIT_FLAGS_VALIDATION) == 0)&&(table_type == DBAL_ACCESS_METHOD_HARD_LOGIC)) ?  \
    DB_INIT_DIR_PATH_TABLES_HARD_LOGIC : (                                                        \
  (((flags & DBAL_INIT_FLAGS_VALIDATION) != 0)&&(table_type == DBAL_ACCESS_METHOD_PHY_TABLE))  ?  \
    DB_INIT_VALIDATION_TEST_PATH_TABLES_MDB : (                                                   \
  (((flags & DBAL_INIT_FLAGS_VALIDATION) == 0)&&(table_type == DBAL_ACCESS_METHOD_PHY_TABLE))  ?  \
    DB_INIT_DIR_PATH_TABLES_MDB : (                                                               \
  (((flags & DBAL_INIT_FLAGS_VALIDATION) != 0)&&(table_type == DBAL_ACCESS_METHOD_SW_ONLY))    ?  \
    DB_INIT_VALIDATION_TEST_PATH_TABLES_SW : (                                                    \
  (((flags & DBAL_INIT_FLAGS_VALIDATION) == 0)&&(table_type == DBAL_ACCESS_METHOD_SW_ONLY))    ?  \
    DB_INIT_DIR_PATH_TABLES_SW : (                                                                \
  (((flags & DBAL_INIT_FLAGS_VALIDATION) != 0)&&(table_type == DBAL_ACCESS_METHOD_PEMLA))      ?  \
    DB_INIT_VALIDATION_TEST_PATH_TABLES_PEMLA : DB_INIT_DIR_PATH_TABLES_PEMLA ))))))

/*************
 * TYPEDEFS  *
 *************/
/**
 * dnx_data parameters structure 
 * Used for dnx data reference from dbal XML files 
 */
typedef struct
{
    /** Indication if DNX_DATA referecnce is valid */
    uint8 indication;

    /** full mapping */
    char full_map[DBAL_MAX_STRING_LENGTH];

    /** dnx data module  */
    char module[DBAL_MAX_STRING_LENGTH];

    /** dnx data submodule  */
    char submodule[DBAL_MAX_STRING_LENGTH];

    /** dnx data data  */
    char data[DBAL_MAX_STRING_LENGTH];

    /** dnx data member */
    char member[DBAL_MAX_STRING_LENGTH];
} dbal_db_dnx_data_info_struct_t;

/**
 * child field structure 
 */
typedef struct
{
    /** child field name */
    char name[DBAL_MAX_STRING_LENGTH];

    /**  encode type  */
    char encode_type[DBAL_MAX_STRING_LENGTH];

    /**  encoding input param  */
    int encode_param1;

} dbal_db_child_field_info_struct_t;

/**
 * enum structure - read from XML
 */
typedef struct
{
    /** enum name */
    char name_from_interface[DBAL_MAX_STRING_LENGTH];

    /**  enum value */
    int value_from_mapping;

} dbal_db_enum_info_struct_t;

/**
 * field DB - read from XML  
 */
typedef struct
{
    /** field name */
    char name[DBAL_MAX_STRING_LENGTH];

    /** size [bits] */
    int size;

    /** size from DNX_DATA indication */
    dbal_db_dnx_data_info_struct_t size_dnx_data;

    /** field type */
    char type[DBAL_MAX_STRING_LENGTH];

    /** child fields list */
    dbal_db_child_field_info_struct_t childs[DBAL_FIELD_MAX_CHILDS_PER_FIELD];

        /** num of child fields */
    int nof_childs;

    /** field enum values */
    dbal_db_enum_info_struct_t enums[DBAL_FIELD_MAX_NUM_OF_ENUM_VALUES];

    /** num of enum values */
    int nof_enum_vals;

    /** field labels */
    char labels[DBAL_MAX_STRING_LENGTH];

    /** field max value */
    int max_value;

    /** field default value */
    int default_val;

    /** default value indication */
    uint8 default_val_valid;

    /** is this field support multiple instances */
    int instances_support;

    /** encode type */
    char encode_type[DBAL_MAX_STRING_LENGTH];

    /** encoding input param */
    int encode_param1;
} field_db_struct_t;

/**
 * field in table DB - read from XML
 */
typedef struct
{
    /** field name */
    char name[DBAL_MAX_STRING_LENGTH];

    /** field validity from DNX_DATA indication */
    dbal_db_dnx_data_info_struct_t valid_dnx_data;

    /** field in table size [bits]  */
    int size;

    /** is this field support multiple instances */
    int nof_instances;

    /** size from DNX_DATA indication */
    dbal_db_dnx_data_info_struct_t size_dnx_data;

    /** field offset in table [bits] */
    int offset;
} table_db_field_params_struct_t;

/**
 * Offset in HW structure - read from XML 
 */
typedef struct
{
    /** offset calculation type */
    char type[DBAL_MAX_STRING_LENGTH];

    /** field name - for encoding calculation */
    char field[DBAL_MAX_STRING_LENGTH];

    /** encoding calculation value  */
    int value;
} table_db_offset_in_hw_struct_t;

/**
 * Condition structure - read from XML
 */
typedef struct
{
    /** type - condition type */
    char type[DBAL_MAX_STRING_LENGTH];

    /** field - for condition calculation */
    char field[DBAL_MAX_STRING_LENGTH];

    /** value - for condition calculation */
    int value;

    /** use enum value of the field for value indication */
    char enum_val[DBAL_MAX_STRING_LENGTH];

} table_db_access_condition_struct_t;

/**
 * logical table access DB - read from XML 
 * hard logic tables only 
 */
typedef struct
{
    /** hard logic access type */
    dbal_hard_logic_access_types_e access_type;

    /** field name for access mapping */
    char access_field_name[DBAL_MAX_STRING_LENGTH];

    /** access name, field or register */
    char access_name[DBAL_MAX_STRING_LENGTH];

    /** size in access [bits] */
    int access_size;

    /** offset in access [bits] */
    int access_offset;

    /** mapping condition */
    table_db_access_condition_struct_t access_condition[DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS];

    /** array offset (array index in access) */
    table_db_offset_in_hw_struct_t array_offset;

    /** entry offset */
    table_db_offset_in_hw_struct_t entry_offset;

    table_db_offset_in_hw_struct_t block_index;

    /** data offset */
    table_db_offset_in_hw_struct_t data_offset;

    /** hw field name in access */
    char hw_field[DBAL_MAX_STRING_LENGTH];

    /** alias access name, memory or register */
    char alias_name[DBAL_MAX_STRING_LENGTH];

    /** data offset in aliased memory   */
    table_db_offset_in_hw_struct_t alias_data_offset;
} table_db_access_params_struct_t;

/**
 * Results set in table.
 * Union is implemented with multiple result sets
 */
typedef struct
{
    /** indication for result type is found in mapping */
    uint8 result_is_mapped;

    /** result type logical value (name) */
    char result_type_name[DBAL_MAX_STRING_LENGTH];

    /** result_type field value */
    int result_type_physical_value;

    /** number of result fields in table */
    int nof_res_fields;

    /** result fields array */
    table_db_field_params_struct_t result_fields[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS];
} table_db_results_field_set_struct_t;

/**
 * logical table full DB, read from XML
 */
typedef struct
{
    /** table name */
    char name[DBAL_MAX_STRING_LENGTH];

    /** maturity level */
    int maturity_level;

    /** table type */
    char type[DBAL_MAX_STRING_LENGTH];

    /** validity from DNX_DATA indication */
    dbal_db_dnx_data_info_struct_t valid_dnx_data;

    /** number of key fields in table */
    int nof_key_fields;

    /** key fields array */
    table_db_field_params_struct_t key_fields[DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS];

    /** number of results fields sets - union indication */
    int num_of_results_sets;

    /** results fields, support union */
    table_db_results_field_set_struct_t results_set[DBAL_MAX_NUMBER_OF_RESULT_TYPES];

    /** labels array */
    char labels[DBAL_MAX_STRING_LENGTH];

    /** core mode  */
    char core_mode[DBAL_MAX_STRING_LENGTH];

    /** table app id - MDB tables only */
    int app_db_id;

    /** table physical db - MDB tables only */
    char phy_db[DBAL_MAX_STRING_LENGTH];

    /** table physical db - MDB tables only */
    int sw_table_size;

    /** number of access mapping*/
    int nof_access[DBAL_MAX_NUMBER_OF_RESULT_TYPES];

    /** result type logical value (name) */
    char mapping_result_name[DBAL_MAX_STRING_LENGTH][DBAL_MAX_NUMBER_OF_RESULT_TYPES];

    /** access DB - hard logic tables only */
    table_db_access_params_struct_t access[DBAL_MAX_NUMBER_OF_RESULT_TYPES][DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS];

    /** pemla DB mapping ID */
    int pemla_db_id;

    /** pemla key fields ID mapping */
    uint32 pemla_key_mapping[DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS];

    /** pemla key fields ID mapping */
    uint32 pemla_result_mapping[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS];

} table_db_struct_t;

/*************
 * FUNCTIONS *
 *************/
/** ****************************************************
* \brief
* The function initializes dbal fields component with default values
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] field_info - \n
*      dbal tables info DB
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*****************************************************/
shr_error_e dbal_db_init_fields_set_default(
    int unit,
    dbal_field_basic_info_t * field_info);

/** ****************************************************
* \brief
* The function initializes dbal tables component with default values
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] table_info - \n
*      dbal tables info DB
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*****************************************************/
shr_error_e dbal_db_init_tables_set_default(
    int unit,
    dbal_logical_table_t * table_info);

/** ****************************************************
* \brief
* The function read the fields XML file(s) and fill the fields DB accordingly
* 
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] flags
*    \param [in] field_info - \n
*      dbal tables info DB
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*****************************************************/
shr_error_e dbal_db_init_fields(
    int unit,
    int flags,
    dbal_field_basic_info_t * field_info);

/** ****************************************************
* \brief
* The function read the MDB tables XML file(s) and fill
* the tables DB accordingly
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] flags
*    \param [in] table_info - \n
*       dbal tables info DB
*    \param [in] table_type - \n
*       refer to the type of tables that are initialized.
*       (HL,MDB,SW,PEMLA)
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*****************************************************/
shr_error_e dbal_db_init_logical_tables(
    int unit,
    int flags,
    dbal_logical_table_t * table_info,
    dbal_access_method_e table_type);

/** ****************************************************
* \brief
* The function run some logical validation on the fields DB 
* In addition it fills the DB with paraneters that not directly
* read from XML
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] fields_info - \n
*      dbal tables info DB
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      Error code
*****************************************************/
shr_error_e dbal_db_init_fields_logical_validation(
    int unit,
    dbal_field_basic_info_t * fields_info);

/******************************************************
 * \brief
 * The function run some logical validation on the tables DB
 * In addition it fills the DB with paraneters that not directly
 * read from XML
 *
 *  \par DIRECT INPUT:
 *    \param [in] unit
 *    \param [in] tables_info - \n
 *      dbal tables info DB
 *  \par DIRECT OUTPUT:
 *    shr_error_e - \n
 *      Error code
*****************************************************/
shr_error_e dbal_db_init_tables_logical_validation(
    int unit,
    dbal_logical_table_t * tables_info);

#endif /*_DBAL_DB_INIT_INCLUDED__*/
