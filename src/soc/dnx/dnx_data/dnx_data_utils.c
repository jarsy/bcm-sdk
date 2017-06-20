/*! \file dnx_data_utils.c
 * 
 * DEVICE DATA UTILS - Utilities function for DNX DATA
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
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DATA

/*
 * INCLUDE FILES:
 * {
 */
#include <soc/dnx/dnx_data/dnx_data.h>
#include <soc/dnx/dnx_data/dnx_data_internal.h>
#include <sal/appl/sal.h>

/*
 * }
 */

/*
 * FUNCTIONS:
 * {
 */

shr_error_e
dnx_data_utils_dump_verify(
    int unit,
    uint32 state_flags,
    uint32 data_flags,
    uint32 dump_flags,
    int *dump)
{
    uint32 data_types;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Assume dump 
     */
    *dump = 1;

    /*
     * Check if data is supported by device 
     */
    if ((data_flags & DNX_DATA_F_SUPPORTED) == 0)
    {
        *dump = 0;
    }

    /*
     * Check if to filter unchanged 
     */
    if ((dump_flags & DNX_DATA_F_CHANGED) && ((data_flags & DNX_DATA_F_CHANGED) == 0))
    {
        *dump = 0;
    }

    /*
     * Check if to filter non-property 
     */
    if ((dump_flags & DNX_DATA_F_PROPERTY) && ((data_flags & DNX_DATA_F_PROPERTY) == 0))
    {
        *dump = 0;
    }

    /*
     * Check if to filter according to data type (define, feature, numeric and table) 
     */
    data_types = dump_flags & DNX_DATA_FLAG_DATA_TYPES_MASK;
    if (data_types)
    {
        if ((data_flags & data_types) == 0)
        {
            *dump = 0;
        }
    }

/*exit:*/
    SHR_FUNC_EXIT;
}

const uint32 *
dnx_data_utils_generic_feature_data_get(
    int unit,
    int module_index,
    int submodule_index,
    char *data_name)
{
    int feature_index;
    dnx_data_feature_t *features;
    int nof_features;

    /*
     * Get feature array
     */
    features = _dnx_data[unit].modules[module_index].submodules[submodule_index].features;
    nof_features = _dnx_data[unit].modules[module_index].submodules[submodule_index].nof_features;

    /*
     * Iterate over features 
     */
    for (feature_index = 0; feature_index < nof_features; feature_index++)
    {
        if (!sal_strncasecmp(data_name, features[feature_index].name, strlen(data_name)))
        {
            /*
             * If feature found - return pointer to feature
             */
            return (uint32 *) & features[feature_index].data;
        }
    }

    /*
     * If not found return null
     */
    return NULL;
}

const uint32 *
dnx_data_utils_generic_define_data_get(
    int unit,
    int module_index,
    int submodule_index,
    char *data_name)
{
    int define_index;
    dnx_data_define_t *defines;
    int nof_defines;
    shr_error_e rv;

    /*
     * Get defines array
     */
    defines = _dnx_data[unit].modules[module_index].submodules[submodule_index].defines;
    nof_defines = _dnx_data[unit].modules[module_index].submodules[submodule_index].nof_defines;

    /*
     * Iterate over defines 
     */
    for (define_index = 0; define_index < nof_defines; define_index++)
    {
        if (!sal_strncasecmp(data_name, defines[define_index].name, strlen(data_name)))
        {
            /*
             * If define found - make sure define is supported 
             */
            rv = dnx_data_mgmt_access_verify(unit, _dnx_data[unit].state, defines[define_index].flags);
            if (rv != _SHR_E_NONE)
            {
                /*
                 * in case access denied 
                 */
                LOG_ERROR(BSL_LOG_MODULE,
                          (BSL_META_U(unit, "DNX DATA - define %s access denied for device %s\n"),
                           defines[define_index].name, _dnx_data[unit].name));
                return NULL;
            }

            /*
             * If supported - return pointer to data
             */
            return &defines[define_index].data;
        }
    }

    return NULL;
}

const uint32 *
dnx_data_utils_generic_table_data_get(
    int unit,
    int module_index,
    int submodule_index,
    char *data,
    char *member)
{
    int table_index, value_index;
    dnx_data_table_t *tables;
    int nof_tables;
    char *table_data;

    /*
     * 1. Get list of tables
     * 2. Look for the requested table
     * 3. Look for the requested member (value) in table
     * 4. return pointer to data
     */
    tables = _dnx_data[unit].modules[module_index].submodules[submodule_index].tables;
    nof_tables = _dnx_data[unit].modules[module_index].submodules[submodule_index].nof_tables;

    /*
     * Iterate over tables 
     */
    for (table_index = 0; table_index < nof_tables; table_index++)
    {
        if (!sal_strncasecmp(data, tables[table_index].name, strlen(data)))
        {
            /*
             * If table found - make sure table is supported by device
             */
            table_data = dnx_data_mgmt_table_data_get(unit, &tables[table_index], /*key 1 */ 0, /*key 2 */ 0);
            if (table_data == NULL)
            {
                /*
                 * table is not supported by the device - return null
                 */
                return NULL;
            }

            /*
             * iterate over values 
             */
            for (value_index = 0; value_index < tables[table_index].nof_values; value_index++)
            {
                if (!sal_strncasecmp(member, tables[table_index].values[value_index].name, strlen(member)))
                {
                    /*
                     * If value found - return pointer to data
                     */
                    return (const uint32 *) (table_data + tables[table_index].values[value_index].offset);
                }
            }

            return NULL;
        }
    }

    return NULL;
}

const uint32 *
dnx_data_utils_generic_module_data_get(
    int unit,
    int module_index,
    char *submodule,
    char *data,
    char *member)
{
    int submodule_index;
    const uint32 *res;

    /*
     * Get list of submodules
     */
    dnx_data_module_t *module = &_dnx_data[unit].modules[module_index];
    dnx_data_submodule_t *submodules = module->submodules;

    /*
     * Iterate over submodules 
     */
    for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
    {
        if (!sal_strncasecmp(submodule, submodules[submodule_index].name, strlen(submodule)))
        {

            /*
             * look for feature with that name 
             */
            res = dnx_data_utils_generic_feature_data_get(unit, module_index, submodule_index, data);
            if (res != NULL)
            {
                return res;
            }

            /*
             * look for define with that name 
             */
            res = dnx_data_utils_generic_define_data_get(unit, module_index, submodule_index, data);
            if (res != NULL)
            {
                return res;
            }

            /*
             * look for define with that name 
             */
            res = dnx_data_utils_generic_table_data_get(unit, module_index, submodule_index, data, member);
            if (res != NULL)
            {
                return res;
            }

            /*
             * required data wasn't found  (data and member) throw an error 
             */
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "DNX DATA - data wasn't found  %s, %s\n"), data, member != NULL ? member : ""));
            return NULL;
        }
    }

    /*
     * required data wasn't found (submodule) throwan error 
     */
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "DNX DATA - submodule wasn't found  %s\n"), submodule));
    return NULL;
}

const uint32 *
dnx_data_utils_generic_data_get(
    int unit,
    char *module,
    char *submodule,
    char *data,
    char *member)
{
    int module_index;

    /*
     * get modules array
     */
    dnx_data_module_t *modules = _dnx_data[unit].modules;

    /*
     * Iterate over modules 
     */
    for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
    {
        if (!sal_strncasecmp(module, modules[module_index].name, strlen(module)))
        {
            /*
             * If module found - look for requested data in that module
             */
            return dnx_data_utils_generic_module_data_get(unit, module_index, submodule, data, member);
        }
    }

    /*
     * required data wasn't found (module) throw print an error 
     */
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "DNX DATA - module wasn't found  %s\n"), module));
    return NULL;
}

char *
dnx_data_utils_generic_feature_data_str_get(
    int unit,
    int module_index,
    int submodule_index,
    char *data_name,
    char *buffer)
{
    int feature_index;
    dnx_data_feature_t *features;
    int nof_features;
    int val;

    /*
     * Get features array
     */
    features = _dnx_data[unit].modules[module_index].submodules[submodule_index].features;
    nof_features = _dnx_data[unit].modules[module_index].submodules[submodule_index].nof_features;

    /*
     * Iterate over features 
     */
    for (feature_index = 0; feature_index < nof_features; feature_index++)
    {
        if (!sal_strncasecmp(data_name, features[feature_index].name, strlen(data_name)))
        {
            /*
             * If found - convert value to string
             */
            val = features[feature_index].data;
            sal_sprintf(buffer, "%d", val);
            return buffer;

        }
    }

    return NULL;
}

char *
dnx_data_utils_generic_define_data_str_get(
    int unit,
    int module_index,
    int submodule_index,
    char *data_name,
    char *buffer)
{
    int define_index;
    dnx_data_define_t *defines;
    int nof_defines;
    shr_error_e rv;

    /*
     * Get list of defines
     */
    defines = _dnx_data[unit].modules[module_index].submodules[submodule_index].defines;
    nof_defines = _dnx_data[unit].modules[module_index].submodules[submodule_index].nof_defines;

    /*
     * Iterate over defines 
     */
    for (define_index = 0; define_index < nof_defines; define_index++)
    {
        if (!sal_strncasecmp(data_name, defines[define_index].name, strlen(data_name)))
        {
            /*
             * Make sure that data is supported
             */
            rv = dnx_data_mgmt_access_verify(unit, _dnx_data[unit].state, defines[define_index].flags);
            if (rv != _SHR_E_NONE)
            {
                /*
                 * in case access denied - return string 'null'
                 */
                LOG_ERROR(BSL_LOG_MODULE,
                          (BSL_META_U(unit, "DNX DATA - define %s access denied for device %s\n"),
                           defines[define_index].name, _dnx_data[unit].name));
                sal_sprintf(buffer, "%s", "null");
            }

            /*
             * Convert value to string
             */
            sal_sprintf(buffer, "%d", defines[define_index].data);
            return buffer;
        }
    }

    return NULL;
}

char *
dnx_data_utils_generic_table_data_str_get(
    int unit,
    int module_index,
    int submodule_index,
    char *data,
    char *member,
    char *buffer)
{
    int table_index, value_index;
    dnx_data_table_t *tables;
    int nof_tables;
    shr_error_e rv;

    /*
     * 1. Get tables array
     * 2. Look for the requested table
     * 3. Look for the requested member
     * 4. get values as string
     */
    tables = _dnx_data[unit].modules[module_index].submodules[submodule_index].tables;
    nof_tables = _dnx_data[unit].modules[module_index].submodules[submodule_index].nof_tables;

    /*
     * Iterate over tables 
     */
    for (table_index = 0; table_index < nof_tables; table_index++)
    {
        if (!sal_strncasecmp(data, tables[table_index].name, strlen(data)))
        {
            if (tables[table_index].entry_get == NULL)
            {
                /*
                 * table is not supported by the device 
                 */
                return NULL;
            }

            /*
             * iterate over values 
             */
            for (value_index = 0; value_index < tables[table_index].nof_values; value_index++)
            {
                if (!sal_strncasecmp(member, tables[table_index].values[value_index].name, strlen(member)))
                {
                    /*
                     * If found - get values as string
                     */
                    rv = tables[table_index].entry_get(unit, buffer, 0, 0, value_index);
                    if (rv != _SHR_E_NONE)
                    {
                        return NULL;
                    }
                    return buffer;
                }
            }

            return NULL;
        }
    }

    return NULL;
}

char *
dnx_data_utils_generic_module_str_get(
    int unit,
    int module_index,
    char *submodule,
    char *data,
    char *member,
    char *buffer)
{
    int submodule_index;
    char *res;
    dnx_data_module_t *module = &_dnx_data[unit].modules[module_index];
    dnx_data_submodule_t *submodules = module->submodules;

    /*
     * Iterate over submodules 
     */
    for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
    {
        if (!sal_strncasecmp(submodule, submodules[submodule_index].name, strlen(submodule)))
        {

            /*
             * look for feature with that name 
             */
            res = dnx_data_utils_generic_feature_data_str_get(unit, module_index, submodule_index, data, buffer);
            if (res != NULL)
            {
                return res;
            }

            /*
             * look for define with that name 
             */
            res = dnx_data_utils_generic_define_data_str_get(unit, module_index, submodule_index, data, buffer);
            if (res != NULL)
            {
                return res;
            }

            /*
             * look for define with that name 
             */
            res = dnx_data_utils_generic_table_data_str_get(unit, module_index, submodule_index, data, member, buffer);
            if (res != NULL)
            {
                return res;
            }

            /*
             * required data wasn't found (data and member) throwan error 
             */
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "DNX DATA - data wasn't found  %s, %s\n"), data, member != NULL ? member : ""));
            return NULL;
        }
    }

    /*
     * required data wasn't found (submodule) throw an error 
     */
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "DNX DATA - submodule wasn't found  %s\n"), submodule));
    return NULL;
}

char *
dnx_data_utils_generic_data_str_get(
    int unit,
    char *module,
    char *submodule,
    char *data,
    char *member,
    char *buffer,
    int buffer_size)
{
    int module_index;

    dnx_data_module_t *modules = _dnx_data[unit].modules;

    /*
     * Iterate over modules 
     */
    for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
    {
        if (!sal_strncasecmp(module, modules[module_index].name, strlen(module)))
        {
            /*
             * If module found look for the data in that module
             */
            return dnx_data_utils_generic_module_str_get(unit, module_index, submodule, data, member, buffer);
        }
    }

    /*
     * required data wasn't found (module) throw print an error 
     */
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "DNX DATA - module wasn't found  %s\n"), module));
    return NULL;
}

/*!
 * }
 */
