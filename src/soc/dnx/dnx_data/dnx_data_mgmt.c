/*! \file dnx_data_mgmt.c
 * 
 * DEVICE DATA MGMT - 
 * 
 * Device Data
 * SW component that maintains per device data
 * The data is static and won't be changed after device initialization.
 *     
 * Supported data types:
 *     - Define             - a 'uint32' or 'int' number (a max value for all devices is maintained)
 *     - feature            - 1 bit per each feature (supported/not supported) 
 *     - table              - the data is accessed with keys and/or can maintain multiple values and/or set by SoC property
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
dnx_data_mgmt_access_verify(
    int unit,
    uint32 state_flags,
    uint32 data_flags)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Check if data is supported by device
     */
    if ((data_flags & DNX_DATA_F_SUPPORTED) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "data not supported for device\n");
    }

    /*
     * Check if is appl init done and reading init only data
     */
    if ((state_flags & DNX_DATA_STATE_F_APPL_INIT_DONE) && (data_flags & DNX_DATA_F_INIT_ONLY))
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "reading init only data after init is not allowed\n");
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_data_mgmt_state_set(
    int unit,
    uint32 flags)
{
    SHR_FUNC_INIT_VARS(unit);

    _dnx_data[unit].state |= flags;

/*exit:*/
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_data_mgmt_deinit(
    int unit,
    dnx_data_t * unit_data)
{
    int module_index;
    dnx_data_module_t *module;

    int submodule_index;
    dnx_data_submodule_t *submodule;

    int table_index, feature_index, define_index;
    dnx_data_table_t *table;
    dnx_data_feature_t *feature;
    dnx_data_define_t *define;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * if unit data initilized 
     */
    if (unit_data->state != 0)
    {
        /*
         * Iterate over modules
         */
        for (module_index = 0; module_index < unit_data->nof_modules; module_index++)
        {
            module = &unit_data->modules[module_index];

            /*
             * iterate over submodules
             */
            for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
            {
                submodule = &module->submodules[submodule_index];

                /*
                 * iterate over features
                 */
                for (feature_index = 0; feature_index < submodule->nof_features; feature_index++)
                {
                    /*
                     * Free feature proprety mapping
                     */
                    feature = &submodule->features[feature_index];

                    if (feature->property.mapping != NULL)
                    {
                        sal_free(feature->property.mapping);
                        feature->property.mapping = NULL;
                    }
                }
                /*
                 * free feature array
                 */
                if (submodule->nof_features != 0 && submodule->features != NULL)
                {
                    sal_free(submodule->features);
                    submodule->features = NULL;

                }

                /*
                 * iterate over defines
                 */
                for (define_index = 0; define_index < submodule->nof_defines; define_index++)
                {
                    /*
                     * free define property mapping - numeric
                     */
                    define = &submodule->defines[define_index];

                    if (define->property.mapping != NULL)
                    {
                        sal_free(define->property.mapping);
                        define->property.mapping = NULL;
                    }
                }
                /*
                 * free defines array
                 */
                if (submodule->nof_defines != 0 && submodule->defines != NULL)
                {
                    sal_free(submodule->defines);
                    submodule->defines = NULL;
                }

                /*
                 * iterate over tables
                 */
                for (table_index = 0; table_index < submodule->nof_tables; table_index++)
                {
                    table = &submodule->tables[table_index];
                    /*
                     * free table data buffer 
                     */
                    if (table->data != NULL)
                    {
                        sal_free(table->data);
                        table->data = NULL;
                    }
                    /*
                     * free table propert mapping
                     */
                    if (table->property.mapping != NULL)
                    {
                        sal_free(table->property.mapping);
                        table->property.mapping = NULL;
                    }
                }
                /*
                 * free tables array
                 */
                if (submodule->nof_tables != 0 && submodule->tables != NULL)
                {
                    sal_free(submodule->tables);
                    submodule->tables = NULL;
                }
            }

            /*
             * Free submodules array
             */
            if (module->nof_submodules != 0 && module->submodules != NULL)
            {
                sal_free(module->submodules);
                module->submodules = NULL;
            }

        }
        /*
         * Free modules array
         */
        if (unit_data->nof_modules != 0 && unit_data->modules != NULL)
        {
            sal_free(unit_data->modules);
            unit_data->modules = NULL;
        }

        /*
         * reset global data
         */
        sal_memset(&_dnx_data[unit], 0, sizeof(dnx_data_t));
    }
/*exit:*/
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_data_mgmt_values_set(
    int unit)
{
    int module_index;
    dnx_data_module_t *module;

    int submodule_index;
    dnx_data_submodule_t *submodule;

    int data_index;
    SHR_FUNC_INIT_VARS(unit);

    if (_dnx_data[unit].state != 0)
    {

        /*
         * Set non-numeric defines in all modules
         */
        for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
        {
            module = &_dnx_data[unit].modules[module_index];

            for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
            {
                submodule = &module->submodules[submodule_index];

                /*
                 * Iterate over all non-numeric defines
                 */
                for (data_index = 0; data_index < submodule->nof_defines; data_index++)
                {

                    /*
                     * Filter unspported defines and numeric defines (will be set separately)
                     */
                    if (submodule->defines[data_index].flags & DNX_DATA_F_NUMERIC)
                    {
                        continue;
                    }
                    if (submodule->defines[data_index].set == NULL)
                    {
                        continue;
                    }

                    /*
                     * Set required values
                     */
                    SHR_IF_ERR_EXIT(submodule->defines[data_index].set(unit));
                }
            }
        }

        /*
         * Set features in all modules
         */
        for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
        {
            module = &_dnx_data[unit].modules[module_index];

            for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
            {
                submodule = &module->submodules[submodule_index];

                /*
                 * Call to set function 
                 */
                for (data_index = 0; data_index < submodule->nof_features; data_index++)
                {
                    if (submodule->features[data_index].set != NULL)
                    {
                        /*
                         * Set required values
                         */
                        SHR_IF_ERR_EXIT(submodule->features[data_index].set(unit));

                        /*
                         * Calc changed flag
                         */
                        SHR_IF_ERR_EXIT(dnx_data_mgmt_feature_changed_set(unit, &submodule->features[data_index]));
                    }
                }
            }
        }

        /*
         * Set non-numeric in all modules
         */
        for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
        {
            module = &_dnx_data[unit].modules[module_index];

            for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
            {
                submodule = &module->submodules[submodule_index];

                /*
                 * Iterate over all non-numeric defines
                 */
                for (data_index = 0; data_index < submodule->nof_defines; data_index++)
                {

                    /*
                     * Filter unspported defines and non numeric defines (already set)
                     */
                    if ((submodule->defines[data_index].flags & DNX_DATA_F_NUMERIC) == 0)
                    {
                        continue;
                    }
                    if (submodule->defines[data_index].set == NULL)
                    {
                        continue;
                    }

                    /*
                     * Set required values
                     */
                    SHR_IF_ERR_EXIT(submodule->defines[data_index].set(unit));

                    /*
                     * Calc changed flag
                     */
                    SHR_IF_ERR_EXIT(dnx_data_mgmt_define_changed_set(unit, &submodule->defines[data_index]));

                }
            }
        }

        /*
         * Set tables in all modules
         */
        for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
        {
            module = &_dnx_data[unit].modules[module_index];

            for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
            {
                submodule = &module->submodules[submodule_index];

                /*
                 * Call to set function 
                 */
                for (data_index = 0; data_index < submodule->nof_tables; data_index++)
                {
                    if (submodule->tables[data_index].set != NULL)
                    {
                        /*
                         * Set required values
                         */
                        SHR_IF_ERR_EXIT(submodule->tables[data_index].set(unit));

                        /*
                         * Calc changed flag
                         */
                        SHR_IF_ERR_EXIT(dnx_data_mgmt_table_changed_set(unit, &submodule->tables[data_index]));
                    }
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

uint32
dnx_data_mgmt_define_data_get(
    int unit,
    int module_index,
    int submodule_index,
    int define_index)
{
    shr_error_e rv;
    dnx_data_define_t *define;

    /*
     * get define struct
     */
    define = &_dnx_data[unit].modules[module_index].submodules[submodule_index].defines[define_index];

    /*
     * verify access
     */
    rv = dnx_data_mgmt_access_verify(unit, _dnx_data[unit].state, define->flags);
    if (rv != _SHR_E_NONE)
    {
        /*
         * in case access denied - error msg and assert
         */
        LOG_ERROR(BSL_LOG_MODULE,
                  (BSL_META_U(unit, "DNX DATA - define %s access denied for device %s\n"), define->name,
                   _dnx_data[unit].name));
        assert(0);
    }

    /*
     * return actual data
     */
    return define->data;

}

int
dnx_data_mgmt_feature_data_get(
    int unit,
    int module_index,
    int submodule_index,
    int feature_index)
{
    shr_error_e rv;
    dnx_data_feature_t *feature;

    /*
     * get feature struct
     */
    feature = &_dnx_data[unit].modules[module_index].submodules[submodule_index].features[feature_index];

    /*
     * verify access
     */
    rv = dnx_data_mgmt_access_verify(unit, _dnx_data[unit].state, feature->flags);
    if (rv == _SHR_E_NONE)
    {
        /*
         * return pointer to data
         */
        return feature->data;
    }

    /*
     * in case access denied - return 0
     */
    return 0;
}

dnx_data_table_t *
dnx_data_mgmt_table_get(
    int unit,
    int module_index,
    int submodule_index,
    int table_index)
{
    /*
     * get table struct
     */
    return &_dnx_data[unit].modules[module_index].submodules[submodule_index].tables[table_index];
}

char *
dnx_data_mgmt_table_data_get(
    int unit,
    dnx_data_table_t * table,
    int key0,
    int key1)
{
    shr_error_e rv;
    int key0_size, key1_size;

    /*
     * verify access
     */
    rv = dnx_data_mgmt_access_verify(unit, _dnx_data[unit].state, table->flags);
    if (rv == _SHR_E_NONE)
    {
        /*
         * get entry default values - last index will hold the default values
         */
        if (key0 == -1 && key1 == -1)
        {
            key0_size = table->keys[0].size != 0 ? table->keys[0].size : 1;
            key1_size = table->keys[1].size != 0 ? table->keys[1].size : 1;
            return table->data + key0_size * key1_size * table->size_of_values;
        }
        /*
         * Check key out of bound - in case key==0 assume that table without keys
         */
        if ((key0 >= 0 && key0 < table->keys[0].size) || key0 == 0)
        {
            if ((key1 >= 0 && key1 < table->keys[1].size) || key1 == 0)
            {
                /*
                 * return pointer to data 
                 * 2D buffer - line for each key1 
                 */
                return table->data + key0 * table->size_of_values + key1 * table->keys[0].size * table->size_of_values;
            }
            else
            {
                /*
                 * key out of bound - error msg and assert
                 */
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "DNX DATA - out of bound key - table %s\n"), table->name));
                assert(0);
            }
        }
        else
        {
            /*
             * key out of bound - error msg and assert
             */
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "DNX DATA - out of bound key - table %s\n"), table->name));
            assert(0);
        }
    }
    else
    {
        /*
         * in case access denied - error msg and assert
         */
        LOG_ERROR(BSL_LOG_MODULE,
                  (BSL_META_U(unit, "DNX DATA - table %s access denied for device %s\n"), table->name,
                   _dnx_data[unit].name));
        assert(0);
    }

    /*
     * dead code
     */
    return NULL;
}

shr_error_e
dnx_data_mgmt_property_read(
    int unit,
    dnx_data_property_t * property,
    int key,
    uint32 * value)
{
    int value_index, found = 0;
    uint32 val;
    char *val_str;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * read property according to property method
     */
    switch (property->method)
    {
        case dnx_data_property_method_enable:
        {
            /*
             * Enable method - see 'dnx_data_property_method_enable' for details 
             */
            /*
             * read
             */
            val = soc_property_get(unit, property->name, *value);
            /*
             * verify
             */
            if (val != 0 && val != 1 && val != *value)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM,
                             "[Enable read method] SoC property %s - only 0 or 1 are allowed - got %d \n",
                             property->name, val);
            }
            *value = val;
            break;
        }

        case dnx_data_property_method_port_enable:
        {
            /*
             * Port enable method - see 'dnx_data_property_method_port_enable' for details 
             */

            /*
             * read
             */
            val = soc_property_port_get(unit, key, property->name, *value);
            /*
             * verify
             */
            if (val != 0 && val != 1 && val != *value)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM,
                             "[Port enable read method] SoC property %s  port %d - only 0 or 1 are allowed - got %d \n",
                             property->name, key, val);
            }
            *value = val;
            break;
        }

        case dnx_data_property_method_suffix_enable:
        {
            /*
             * Suffix enable method - see 'dnx_data_property_method_suffix_enable' for details 
             */

            /*
             * read
             */
            val = soc_property_suffix_num_get(unit, key, property->name, property->suffix, *value);
            /*
             * verify
             */
            if (val != 0 && val != 1 && val != *value)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM,
                             "[Suffix enable read method] SoC property %s - suffix %s num %d - only 0 or 1 are allowed - got %d \n",
                             property->name, property->suffix, key, val);
            }
            *value = val;
            break;
        }

        case dnx_data_property_method_range:
        {
            /*
             * Range method - see 'dnx_data_property_method_range' for details 
             */

            /*
             * read
             */
            val = soc_property_get(unit, property->name, *value);
            /*
             * verify
             */
            if ((val < property->range_min || val > property->range_max) && (val != *value))
            {
                SHR_ERR_EXIT(_SHR_E_PARAM,
                             "[Range read method] SoC property %s - [%d - %d] is allowed - got %d \n",
                             property->name, property->range_min, property->range_max, val);
            }
            *value = val;
            break;
        }

        case dnx_data_property_method_port_range:
        {
            /*
             * Port range method - see 'dnx_data_property_method_port_range' for details 
             */

            /*
             * read
             */
            val = soc_property_port_get(unit, key, property->name, *value);
            /*
             * verify
             */
            if ((val < property->range_min || val > property->range_max) && (val != *value))
            {
                SHR_ERR_EXIT(_SHR_E_PARAM,
                             "[Range port read method] SoC property %s port %d - [%d - %d] is allowed - got %d \n",
                             property->name, key, property->range_min, property->range_max, val);
            }
            *value = val;
            break;
        }

        case dnx_data_property_method_suffix_range:
        {
            /*
             * Suffix range method - see 'dnx_data_property_method_suffix_range' for details 
             */

            /*
             * read
             */
            val = soc_property_suffix_num_get(unit, key, property->name, property->suffix, *value);
            /*
             * verify
             */
            if ((val < property->range_min || val > property->range_max) && (val != *value))
            {
                SHR_ERR_EXIT(_SHR_E_PARAM,
                             "[Range suffix read method] SoC property %s suffix %s num %d - [%d - %d] is allowed - got %d \n",
                             property->name, property->suffix, key, property->range_min, property->range_max, val);
            }
            *value = val;
            break;
        }

        case dnx_data_property_method_direct_map:
        {
            /*
             * Direct map method - see 'dnx_data_property_method_direct_map' for details 
             */

            /*
             * read
             */
            val_str = soc_property_get_str(unit, property->name);
            /*
             * verify
             */
            if (val_str == NULL)
            {
                /*
                 * Do nothing assume default
                 */
            }
            else
            {
                /*
                 * look for value_str in map 
                 * if found - set value accoding to map entry 
                 * otherwise  - throw an error 
                 */
                for (value_index = 0; value_index < property->nof_mapping; value_index++)
                {
                    if (!sal_strncasecmp(property->mapping[value_index].name, val_str, strlen(val_str)))
                    {
                        *value = property->mapping[value_index].val;;
                        found = 1;
                    }
                }
                if (!found)
                {
                    SHR_ERR_EXIT(_SHR_E_PARAM,
                                 "[Range direct map method] SoC property %s - %s is not a valid value\n"
                                 "Type 'dnx_data prpoerty %s' for more info (including the supported values)\n",
                                 property->name, val_str, property->name);

                }
            }
            break;
        }

        case dnx_data_property_method_port_direct_map:
        {
            /*
             * Port direct map method - see 'dnx_data_property_method_port_direct_map' for details 
             */

            /*
             * read
             */
            val_str = soc_property_port_get_str(unit, key, property->name);
            /*
             * verify
             */
            if (val_str == NULL)
            {
                /*
                 * Do nothing  - assume default
                 */
            }
            else
            {
                /*
                 * look for value_str in map 
                 * if found - set value accoding to map entry 
                 * otherwise  - throw an error 
                 */
                for (value_index = 0; value_index < property->nof_mapping; value_index++)
                {
                    if (!sal_strncasecmp(property->mapping[value_index].name, val_str, strlen(val_str)))
                    {
                        *value = property->mapping[value_index].val;
                        found = 1;
                    }
                }
                if (!found)
                {
                    SHR_ERR_EXIT(_SHR_E_PARAM,
                                 "[Range port direct map method] SoC property %s port %d - %s is not a valid value\n"
                                 "Type 'dnx_data prpoerty %s' for more info (including the supported values)\n",
                                 property->name, key, val_str, property->name);

                }
            }
            break;
        }

        case dnx_data_property_method_suffix_direct_map:
        {
            /*
             * Suffix direct map method - see 'dnx_data_property_method_suffix_direct_map' for details 
             */

            /*
             * read
             */
            val_str = soc_property_suffix_num_str_get(unit, key, property->name, property->suffix);
            /*
             * verify
             */
            if (val_str == NULL)
            {
                /*
                 * Do nothing  - assume default
                 */
            }
            else
            {
                /*
                 * look for value_str in map 
                 * if found - set value accoding to map entry 
                 * otherwise  - throw an error 
                 */
                for (value_index = 0; value_index < property->nof_mapping; value_index++)
                {
                    if (!sal_strncasecmp(property->mapping[value_index].name, val_str, strlen(val_str)))
                    {
                        *value = property->mapping[value_index].val;
                        found = 1;
                    }
                }
                if (!found)
                {
                    SHR_ERR_EXIT(_SHR_E_PARAM,
                                 "[Range suffix direct map method] SoC property %s suffix %s num %d- %s is not a valid value\n"
                                 "Type 'dnx_data prpoerty %s' for more info (including the supported values)\n",
                                 property->name, property->suffix, key, val_str, property->name);

                }
            }
            break;
        }

        default:
        {
            /*
             * unspported method - throw an error
             */

            SHR_ERR_EXIT(_SHR_E_PARAM, "SoC property read method not supported %d \n", property->method);
            break;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_data_mgmt_feature_changed_set(
    int unit,
    dnx_data_feature_t * feature)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * If value different from default value - set changed flag
     */
    if (feature->data != feature->default_data)
    {
        feature->flags |= DNX_DATA_F_CHANGED;
    }

    SHR_FUNC_EXIT;
}

shr_error_e
dnx_data_mgmt_define_changed_set(
    int unit,
    dnx_data_define_t * define)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * If value different from default value - set changed flag
     */
    if (define->data != define->default_data)
    {
        define->flags |= DNX_DATA_F_CHANGED;
    }

    SHR_FUNC_EXIT;
}

shr_error_e
dnx_data_mgmt_table_changed_set(
    int unit,
    dnx_data_table_t * table)
{
    char *default_val;
    char *val;
    int changed;
    int key0, key1;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get default data
     */
    default_val = dnx_data_mgmt_table_data_get(unit, table, -1, -1);

    /*
     * Iterate over all keys 
     * If one of them different from default - mark the table as changed (flag)
     */
    for (key0 = 0; key0 < table->keys[0].size || key0 == 0; key0++)
    {
        for (key1 = 0; key1 < table->keys[1].size || key1 == 0; key1++)
        {
            val = dnx_data_mgmt_table_data_get(unit, table, key0, key1);
            changed = sal_memcmp(default_val, val, table->size_of_values);
            if (changed)
            {
                table->flags |= DNX_DATA_F_CHANGED;
                SHR_EXIT();
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_data_mgmt_table_entry_changed_get(
    int unit,
    dnx_data_table_t * table,
    int key0,
    int key1,
    int *changed)
{
    char *default_val;
    char *val;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Get default data
     */
    default_val = dnx_data_mgmt_table_data_get(unit, table, -1, -1);

    /*
     * Get relevant entry
     */
    val = dnx_data_mgmt_table_data_get(unit, table, key0, key1);

    /*
     * Compare
     */
    *changed = sal_memcmp(default_val, val, table->size_of_values);

    SHR_FUNC_EXIT;
}

/*
 *}
 */
