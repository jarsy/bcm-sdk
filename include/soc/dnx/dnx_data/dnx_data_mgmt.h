/*! \file dnx_data_mgmt.h
 * 
 * DEVICE DATA MGMT - functional procedures for DNX DATA
 * 
 * Device Data
 * SW component that maintains per device data
 * The data is static and won't be changed after device initialization.
 *     
 * Supported data types:
 *     - Define             - a 'uint32' number (a max value for all devices is maintained)
 *     - feature            - 1 bit per each feature (supported/not supported) - support soc properties 
 *     - table              - the data is accessed with keys and/or can maintain multiple values and/or set by soc property
 *     - numeric            - a 'uint32' number that support soc properties
 * 
 * User interface for DNX DATA component can be found in "dnx_data_if.h" and "dnx_data_if_#module#.h"
 * 
 * Adding the data is done via XMLs placed in "tools/autocoder/DeviceData/dnx/.." 
 * "How to" User Guide can be found in confluence. 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _DNX_DATA_MGMT_H_
/*{*/
#define _DNX_DATA_MGMT_H_

/**
* \brief This file is only used by DNX (JR2 family). Including it by
* software that is not specific to DNX is an error.
*/
#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/*
 * DEFINEs:
 * {
 */
/**
 * \brief - max number of keys supported in table
 */
#define DNX_DATA_MGMT_MAX_TABLE_KEYS                 (2)
/**
 * \brief - max number of values supported in table
 */
#define DNX_DATA_MGMT_MAX_TABLE_VALUES               (10)
/**
 * \brief - buffer size for value to string convert
 */
#define DNX_DATA_MGMT_MAX_TABLE_VALUE_LENGTH         (1000)

/*
 * DATA TYPES TO STR MACROS
 */
/**
 * \brief convert uint32/int arrays to string
 */
#define DNX_DATA_MGMT_ARR_STR(buffer, size, data)\
    do\
    {\
        int i, offset = 0;\
        for (i = 0; i < size; i++)\
        {\
            if (offset + 10 > DNX_DATA_MGMT_MAX_TABLE_VALUE_LENGTH) break; /* stop if buffer size is not big enough */\
            offset += sal_sprintf(buffer + offset, "%08x ", data[i]);\
        }\
    } while(0)
/**
 * \brief convert pbmp to string
 */
#define DNX_DATA_MGMT_PBMP_STR(buffer, data)\
    do\
    {\
        int i, offset = 0;\
        for (i = 0; i < _SHR_PBMP_WORD_MAX; i++)\
        {\
            if (offset + 10 > DNX_DATA_MGMT_MAX_TABLE_VALUE_LENGTH) break;/* stop if buffer size is not big enough */\
            offset += sal_sprintf(buffer + offset, "%08x ", _SHR_PBMP_WORD_GET(data, i));\
        }\
    } while(0)

/*
 * }
 */

/*
 * TYPEDEFs:
 * {
 */

/** 
 * \brief 
 *  SoC porperty reader type -
 *  The SoC proeprty read method will be determined according to this enum.
 */
typedef enum
{
  /** 
   * value will not modified by SoC property 
   */
    dnx_data_property_method_invalid = 0,

  /** 
   * Only values 0 and 1 are allowed - 
   * Reading numeric value using soc_property_get 
   */
    dnx_data_property_method_enable = 1,
  /** 
   * Only values 0 and 1 are allowed - 
   * Reading numeric value using soc_property_port_get 
   * Table index used as port 
   */
    dnx_data_property_method_port_enable = 2,
  /** 
   * Only values 0 and 1 are allowed - 
   * Reading numeric value using soc_property_suffix_num_get
   * Table index used as num (if exist) 
   */
    dnx_data_property_method_suffix_enable = 3,

  /** 
   * Values from range_min to range_max are allowed 
   * Reading numeric value using soc_property_get 
   */
    dnx_data_property_method_range = 4,
  /** 
   * Values from range_min to range_max are allowed 
   * Reading numeric value using soc_property_port_get 
   * Table index used as port 
   */
    dnx_data_property_method_port_range = 5,
  /** 
   * Values from range_min to range_max are allowed 
   * Reading numeric value using soc_property_suffix_num_get
   * Table index used as num (if exist) 
   */
    dnx_data_property_method_suffix_range = 6,

  /** 
   * Every allowed string should be specified and the mapping to uint32 value 
   * Reading numeric value using soc_property_get_str 
   */
    dnx_data_property_method_direct_map = 7,
  /** 
   * Every allowed string should be specified and the mapping to uint32 value 
   * Reading numeric value using soc_property_port_get_str 
   * Table index used as port 
   */
    dnx_data_property_method_port_direct_map = 8,
  /** 
   * Every allowed string should be specified and the mapping to uint32 value 
   * Reading numeric value using soc_property_suffix_num_str_get
   * Table index used as num (if exist) 
   */
    dnx_data_property_method_suffix_direct_map = 9,

  /** 
   * Read SoC proerty throgh dedicate custom function 
   */
    dnx_data_property_method_custom = 10
} dnx_data_property_method_e;

 /**
 * \brief 
 *  In case property mode is direct mapping -
 *  Each struct holds mapping from string to 'uint32' val
 */
typedef struct
{
    /**
     * property string to map from
     */
    char *name;

    /**
     * property value to map to
     */
    uint32 val;
} dnx_data_property_map_t;

 /** 
 * \brief 
 *  Struct that holds data about table property reader.
 */
typedef struct
{
    /** 
     * SoC property name 
     */
    char *name;

    /** 
     * Property description 
     */
    char *doc;

    /** 
     * Read method - see dnx_data_property_method_e for more info 
     */
    dnx_data_property_method_e method;

    /** 
     * Read method name (used for diag only)
     */
    char *method_str;

    /** 
     * Whether the property is static or dynamic 
     */
    uint32 is_static;

    /** 
     * Suffix - relevant when DNX_DATA_PROPERTY_F_SUFFIX_NUM set 
     */
    char *suffix;

    /** 
     * Allowed int values range  - relevant when method set to dnx_data_property_method_range 
     */
    int range_min;
    int range_max;

    /** 
     * Allowed string values and mapping to uint32  - relevant when method set to dnx_data_property_method_direct_map 
     */
    dnx_data_property_map_t *mapping;
    int nof_mapping;
} dnx_data_property_t;

/**
 * \brief Get entry value as string
 * 
 * \par DIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] key1, key2 - key indexs - (0 if the index is not used)
 *   \param [in] value_index - index of the required value.
 * \par INDIRECT OUTPUT: 
 *   \param [out] buffer - buffer for string 
 */
typedef shr_error_e(
    *dnx_data_table_entry_str_get_f) (
    int unit,
    char *buffer,
    int key1,
    int key2,
    int value_index);

/**
 * \brief Pointer to function (per device) which set values for data
 * 
 * \par DIRECT INPUT:
 *   \param [in] unit - Unit #
 * \par INDIRECT OUTPUT:
 *    * none
 * \par DIRECT OUTPUT:
 *     shr_error_e
 */
typedef shr_error_e(
    *dnx_data_value_set_f) (
    int unit);

/**
 * \brief 
 *  Struct that holds data about table key.
 */
typedef struct
{
    /**
     * Values field name
     */
    char *name;

    /**
     * Values field type name
     */
    char *type;

    /**
     * Values field description
     */
    char *doc;

    /**
     * Values default value - stored as strind (used for diagnostic)
     */
    char *default_val;

    /**
     * Offset that value was stored in table structure 
     */
    int offset;
} dnx_data_value_t;

/**
 * \brief Struct that holds data about table key.
 */
typedef struct
{
    /**
     * Key name
     */
    char *name;

    /**
     * Key Description
     */
    char *doc;

    /** 
     * Number of indexes required 
     */
    int size;
} dnx_data_key_t;

/**
 * \brief Struct that holds table data
 */
typedef struct
{
    /**
     * Data flags - see DNX_DATA_F_# 
     */
    uint32 flags;

    /**
     * Table name
     */
    char *name;

    /**
     * Table description
     */
    char *doc;

    /**
     * Number of table key fields 
     */
    uint32 nof_keys;

    /**
     * Number of keys
     */
    dnx_data_key_t keys[DNX_DATA_MGMT_MAX_TABLE_KEYS];

    /**
     * Number of value fields
     */
    uint32 nof_values;

    /**
     * Array of values fields
     */
    dnx_data_value_t values[DNX_DATA_MGMT_MAX_TABLE_VALUES];

    /** 
     * size of table data structure - used for data buffer format 
     */
    uint32 size_of_values;

    /**
     * data buffer to store all the data 
     */
    char *data;

    /**
     * Pointer to function to get values as string
     */
    dnx_data_table_entry_str_get_f entry_get;

    /**
     * Pointer to function that set the value  
     */
    dnx_data_value_set_f set;

    /**
     *property info  
     */
    dnx_data_property_t property;
} dnx_data_table_t;

/**
 * \brief Struct that holds feature data
 */
typedef struct
{
    /**
     *Data flags - see DNX_DATA_F_#  
     */
    uint32 flags;

    /** 
     * feature name 
     */
    char *name;
    /**
     * doc about the feature
     */
    char *doc;
    /**
     * actual data - stores as uint32
     */
    uint32 data;

        /**
     * default data - stores as uint32
     */
    uint32 default_data;

    /**
     * Pointer to function that set the value  
     */
    dnx_data_value_set_f set;

    /**
     *property info  
     */
    dnx_data_property_t property;
} dnx_data_feature_t;

/**
 * \brief  Struct that holds define data
 */
typedef struct
{
    /**
     *Data flags - see DNX_DATA_F_#  
     */
    uint32 flags;

    /** 
     * define name 
     */
    char *name;

    /**
     * doc about the feature
     */
    char *doc;

    /**
     * actual data - stores as uint32
     */
    uint32 data;

        /**
     * default data - stores as uint32
     */
    uint32 default_data;

    /**
     * Pointer to function that set the value  
     */
    dnx_data_value_set_f set;

    /**
     *property info  
     */
    dnx_data_property_t property;
} dnx_data_define_t;

/**
 * \brief  Struct that holds submodule data
 */
typedef struct
{
    /**
     * Submodule name 
     */
    char *name;

    /**
     * Submodule description
     */
    char *doc;

    /**
     * Number of features in submodule
     */
    uint32 nof_features;

    /**
     * Features array
     */
    dnx_data_feature_t *features;

    /**
     * Number of defines in submodule
     */
    uint32 nof_defines;
    /**
     * Defines array
     */
    dnx_data_define_t *defines;

    /**
     * Number of tables in submodule
     */
    uint32 nof_tables;

    /**
     * Tables array
     */
    dnx_data_table_t *tables;
} dnx_data_submodule_t;

/**
 * \brief 
 *  Struct that holds module data
 */
typedef struct
{
    /**
     * Module name
     */
    char *name;
    /**
     * Submodule array
     */
    dnx_data_submodule_t *submodules;
    /**
     * Number of submodules
     */
    uint32 nof_submodules;
} dnx_data_module_t;

/**
 * \brief 
 *  Struct that holds device data
 */
typedef struct
{
    /**
     * Device name
     */
    char *name;

    /**
     * Modules array
     */
    dnx_data_module_t *modules;
    /**
     * Number if modules
     */
    uint32 nof_modules;

    /** 
     * Component state flags - see DNX_DATA_STATE_F_# 
     */
    uint32 state;
} dnx_data_t;

/**
 * }
 */

/*
 * FUNCTIONS:
 * {
 */
/**
* \brief - DNX DATA access verfier module
 * Verify get access - 
 *  - check whether data is supported for device
 *  - check whether trying to access dynamic SoC property after init
* \par DIRECT_INPUT:
*   \param [in] unit - 
*   \param [in] state_flags - module state flags see DNX_DATA_STATE_F_# 
*   \param [in] data_flags - data flags see DNX_DATA_F_#
*   
* \par INDIRECT INPUT:
*   * None
* \par DIRECT OUTPUT:
*   see shr_error_e 
* \par INDIRECT OUTPUT
*   * None
* \remark
*   * None
* \see
*   * None
*/
shr_error_e dnx_data_mgmt_access_verify(
    int unit,
    uint32 state_flags,
    uint32 data_flags);

/**
* \brief - component state modify
*          Update DNX DATA module about the state.
* \par DIRECT_INPUT:
*   \param [in] unit - unit #
*   \param [in] flags - state flags to set see DNX_DATA_STATE_F_* for details
*   
* \par INDIRECT INPUT:
*   * None
* \par DIRECT OUTPUT:
*   see shr_error_e 
* \par INDIRECT OUTPUT
*   * module global data - _dnx_data[unit].state_flags
* \remark
*   * None
* \see
*   * None
*/
shr_error_e dnx_data_mgmt_state_set(
    int unit,
    uint32 flags);

/**
* \brief - Deinit data structure - dnx_data 
* 
* \par DIRECT_INPUT:
*   \param [in] unit - unit #
*   \param [in] unit_data - pointer to module global data
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
shr_error_e dnx_data_mgmt_deinit(
    int unit,
    dnx_data_t * unit_data);

/**
 * \brief - get feature data - 1 bit enable / disable
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit # 
 *   \param [in] module_index - see enum dnx_data_module_e
 *   \param [in] submodule_index - see enum dnx_data_#module#_submodule_e
 *   \param [in] feature_index - see enum dnx_data_#module#_#submodule#_feature_e
 *   
 * \par INDIRECT INPUT:
 *   * module global data - _dnx_data[unit]
 * \par DIRECT OUTPUT:
 *   int  - feature data
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int dnx_data_mgmt_feature_data_get(
    int unit,
    int module_index,
    int submodule_index,
    int feature_index);

uint32 dnx_data_mgmt_define_data_get(
    int unit,
    int module_index,
    int submodule_index,
    int define_index);

/**
 * \brief - get table strut from module data
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] module_index - see enum dnx_data_module_e
 *   \param [in] submodule_index - see enum dnx_data_#module#_submodule_e
 *   \param [in] table_index - see enum dnx_data_#module#_#submodule#_table_e
 *   
 * \par INDIRECT INPUT:
 *   * module global data - _dnx_data[unit]
 * \par DIRECT OUTPUT:
 *   dnx_data_table_t* - table struct
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
dnx_data_table_t *dnx_data_mgmt_table_get(
    int unit,
    int module_index,
    int submodule_index,
    int table_index);

/**
 * \brief - get pointer to relevant entry in table
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] table - pointer to table structure in database
 *   \param [in] key1 - key1 index - 0 if no index
 *   \param [in] key2 - key2 index - 0 if no index
 *   
 * \par INDIRECT INPUT:
 *   * _dnx_data[unit] global structure
 * \par DIRECT OUTPUT:
 *   char* - general pointer to data (should be case according to table data structure)
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
char *dnx_data_mgmt_table_data_get(
    int unit,
    dnx_data_table_t * table,
    int key1,
    int key2);

/**
 * \brief Read soc property according to info in property structure
 * 
 * \par DIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] property - property info
 *   \param [in] key - to read per key the key index - otherwise -1
 *   \param [in] value - default value in case property wasn't set
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   \param [out] value - property data
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_data_mgmt_property_read(
    int unit,
    dnx_data_property_t * property,
    int key,
    uint32 * value);

/**
 * \brief Check if define changed - and set the proper flag.
 *  
 * \par DIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] define - pointer to define structure in database
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_data_mgmt_define_changed_set(
    int unit,
    dnx_data_define_t * define);

/**
 * \brief Check if feature changed - and set the proper flag.
 *  
 * \par DIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] feature - pointer to feature structure in database
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_data_mgmt_feature_changed_set(
    int unit,
    dnx_data_feature_t * feature);

/**
 * \brief Check if there are entries different from deafult
 *        If there are the table will marked as changed.
 *  
 * \par DIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] table - pointer to table structure in database
 *  
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_data_mgmt_table_changed_set(
    int unit,
    dnx_data_table_t * table);

/**
 * \brief - Set data values (all modules) 
 *          Order of setting:
 *          - defines
 *          - features
 *          - numerics
 *          - tables
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
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
shr_error_e dnx_data_mgmt_values_set(
    int unit);

/**
 * \brief Check if entry changed (different from default)
 *  
 * \par DIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] table - pointer to table structure in database
 *   \param [in] key0 - key0 entry index
 *   \param [in] key1 - key1 entry index
 *  
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   \param [in] changed - 1 iff entry different from default
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_data_mgmt_table_entry_changed_get(
    int unit,
    dnx_data_table_t * table,
    int key0,
    int key1,
    int *changed);
/*
 * }
 */
#endif /*_DNX_DATA_MGMT_H_*/
