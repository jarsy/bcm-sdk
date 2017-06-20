/** \file diag_dnx_data.c
 * 
 * DEVICE DATA DIAG - diagnostic pack for module dnx_data
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
/*appl*/
#include <appl/diag/diag.h>
#include <appl/diag/system.h>
#include <appl/diag/sand/diag_sand_framework.h>
/*soc*/
#include <soc/dnx/dnx_data/dnx_data.h>
#include <soc/dnx/dnx_data/dnx_data_internal.h>
/*sal*/
#include <sal/appl/sal.h>
/*
 * }
 */

/*
 * LOCAL FUNCTIONs:
 * {
 */
/**
 * \brief - Dump dnx data 
 *          If specific data - will dump the requested data
 *          if "*" - will dump the all branch
 *          if NULL - will display list of supported data,
 * \par DIRECT INPUT:
 *   \param [in] unit -  Unit #
 *   \param [in] flags - data flags to get see DNX_DATA_DATA_F_* for details
 *   \param [in] module - module to get or "*" for all modules
 *   \param [in] submodule - submodule to get or "*" for all submodules
 *   \param [in] data - data to get or "*" for all data    
 *   
 * \par INDIRECT INPUT:
 *   * module global data - _dnx_data[unit]
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Output to screen - see brief
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e diag_dnx_data_dump(
    int unit,
    uint32 flags,
    char *module,
    char *submodule,
    char *data);

/**
 * \brief - Display info about dnx data
 *          If specific data - will display info the requested data
 *          if "*" - will display info the all branch
 *          if NULL - will display list of supported data,
 * \par DIRECT INPUT:
 *   \param [in] unit -  Unit #
 *   \param [in] flags - data flags to get see DNX_DATA_DATA_F_* for details
 *   \param [in] module - module to get or "*" for all modules
 *   \param [in] submodule - submodule to get or "*" for all submodules
 *   \param [in] data - data to get or "*" for all data    
 *   
 * \par INDIRECT INPUT:
 *   * module global data - _dnx_data[unit]
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Output to screen - see brief
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e diag_dnx_data_info(
    int unit,
    uint32 flags,
    char *module,
    char *submodule,
    char *data);

/**
 * \brief - Print property info
 *          If specific soc property - will display soc property info
 *          Otherwise will display list of soc property includes the requested soc property 
 * \par DIRECT INPUT:
 *   \param [in] unit -  Unit #
 *   \param [in] property - property name or substring of property name
 *
 * \par INDIRECT INPUT:
 *   * module global data - _dnx_data[unit]
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Output to screen - see brief
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e diag_dnx_data_property(
    int unit,
    char *property);

/**
 * }
 */

 /*
  * LOCAL DIAG PACK:
  * {
  */

static sh_sand_option_t dnx_data_dump_options[] = {
    {"changed",   SAL_FIELD_TYPE_BOOL, "dump changed only properties",                     "no"},
    {"variable",  SAL_FIELD_TYPE_STR,  "[<module_name>[.<submodule_name>[.<data_name>]]]", ""},
    {NULL}
};

/**
 * \brief - see definition on local function section (top of this file)
 */
static sh_sand_man_t dnx_data_dump_man = {
    "Dump data or get list of modules/submodules/data",
    "Please fill here full description for data dump",
    "data dump [changed] [property] [<data_type>] [<module_name>[.<submodule_name>[.<data_name>]]]'",
    "'data dump fabric.pipes.map'  - dump table 'map' in module 'fabric' and submodule 'pipes'\n"
    "'data dump fabric.pipes.*'    - dump all data in module 'fabric' and submodule 'pipes'\n"
    "'data dump fabric.pipes.'     - display list of all data in module 'fabric' and submodule 'pipes'\n"
    "'data dump fabric.'           - display list of all submodules in module 'fabric'\n"
    "'data dump chg *' will dump all data that changed by soc properties"
};

/**
 * \brief - parse args and call to diag_dnx_data_dump() with the requested data params
 */
static shr_error_e
cmd_dnx_data_dump(
    int unit,
    args_t * args,
    sh_sand_control_t *sand_control)
{
    char *module = NULL;
    char *submodule = NULL;
    char *data = NULL;
    uint32 flags = 0;
    uint32 nof_tokens = 0;
    char **datatokens = NULL;
    char *arg;

    SHR_FUNC_INIT_VARS(NO_UNIT);
    /*
     * parse params
     */
    while (((arg = ARG_GET(args)) != NULL))
    {
        if (!sal_strncasecmp(arg, "chg", strlen(arg)))
        {
            /*
             * Only property data can be marked as changed
             */
            flags |= DNX_DATA_F_CHANGED;
            flags |= DNX_DATA_F_PROPERTY;
            flags |= DNX_DATA_F_NUMERIC;
            flags |= DNX_DATA_F_FEATURE;
            flags |= DNX_DATA_F_TABLE;
        }
        else if (!sal_strncasecmp(arg, "property", strlen(arg)))
        {
            /*
             * Only numeric, feature and table support property
             */
            flags |= DNX_DATA_F_PROPERTY;
            flags |= DNX_DATA_F_NUMERIC;
            flags |= DNX_DATA_F_FEATURE;
            flags |= DNX_DATA_F_TABLE;
        }
        else if (!sal_strncasecmp(arg, "feature", strlen(arg)))
        {
            flags |= DNX_DATA_F_FEATURE;
        }
        else if (!sal_strncasecmp(arg, "define", strlen(arg)))
        {
            flags |= DNX_DATA_F_DEFINE;
        }
        else if (!sal_strncasecmp(arg, "table", strlen(arg)))
        {
            flags |= DNX_DATA_F_TABLE;
        }
        else if (!sal_strncasecmp(arg, "numeric", strlen(arg)))
        {
            flags |= DNX_DATA_F_NUMERIC;
        }
        else
        {
            /*
             * Free resources
             */
            if (datatokens != NULL)
            {
                utilex_str_split_free(datatokens, nof_tokens);
            }
            /*
             * Expected format - <module>.<submodule>.<data>
             */
            datatokens = utilex_str_split(arg, ".", 3, &nof_tokens);
            if (nof_tokens > 0)
            {
                module = datatokens[0];
            }
            if (nof_tokens > 1)
            {
                submodule = datatokens[1];
            }
            if (nof_tokens > 2)
            {
                data = datatokens[2];
            }
            if (nof_tokens > 3)
            {
                SHR_CLI_EXIT(_SHR_E_PARAM, ": data dump - data format not supported - %s\n", arg);
            }
            /*
             * set empty token to null
             */
            if (module != NULL && module[0] == 0)
            {
                module = NULL;
            }
            if (submodule != NULL && submodule[0] == 0)
            {
                submodule = NULL;
            }
            if (data != NULL && data[0] == 0)
            {
                data = NULL;
            }
            /*
             * Set '*' along the chain
             */
            if (module != NULL && !sal_strncasecmp(module, "*", strlen("*")))
            {
                submodule = "*";
                data = "*";
            }
            if (submodule != NULL && !sal_strncasecmp(submodule, "*", strlen("*")))
            {
                data = "*";
            }
        }
    }

    /*
     * Call to dnx data dump with the relevant params
     */
    SHR_CLI_EXIT_IF_ERR(diag_dnx_data_dump(unit, flags, module, submodule, data),
                                    "ERROR: diag dnx data dump - command failed\n");
exit:
/*
 * Free resources
 */
    if (datatokens != NULL)
    {
        utilex_str_split_free(datatokens, nof_tokens);
    }
    SHR_FUNC_EXIT;
}

/**
 * \brief - see definition on local function section (top of this file)
 */
static sh_sand_option_t dnx_data_info_options[] = {
    {"changed",   SAL_FIELD_TYPE_BOOL, "dump changed only properties",                     "no"},
    {"variable",  SAL_FIELD_TYPE_STR, "[<module_name>[.<submodule_name>[.<data_name>]]]", ""},
    {NULL}
};

/**
 * \brief - see definition on local function section (top of this file)
 */
static sh_sand_man_t dnx_data_info_man = {
    "Get list of modules/submodules/data or full description id data specified",
    "if '*' is specified will display info about the all branch",
    "data info [changed] [property] [<data_type>] [<module_name>[.<submodule_name>[.<data_name>]]]",
    "'data info fabric.pipes.map' will display info about table 'map' in module 'fabric' and submodule 'pipes'\n"
    "'data info fabric.pipes.*' will display info about all data in module 'fabric' and submodule 'pipes'\n"
    "'data info fabric.' will display list of all submodules in module 'fabric'\n"
    "'data info chg *' will display info about all data that changed by soc properties"
};

/**
 * \brief - parse args and call to diag_dnx_data_info() with the requested data params
 */
static shr_error_e
cmd_dnx_data_info(
    int unit,
    args_t * args,
    sh_sand_control_t *sand_control)
{
    char *module = NULL;
    char *submodule = NULL;
    char *data = NULL;
    uint32 flags = 0;
    uint32 nof_tokens = 0;
    char **datatokens = NULL;
    char *arg;

    SHR_FUNC_INIT_VARS(NO_UNIT);
    /*
     * parse params
     */
    while (((arg = ARG_GET(args)) != NULL))
    {
        if (!sal_strncasecmp(arg, "chg", strlen(arg)))
        {
            /*
             * Only property data can be marked as changed
             */
            flags |= DNX_DATA_F_CHANGED;
            flags |= DNX_DATA_F_PROPERTY;
            flags |= DNX_DATA_F_NUMERIC;
            flags |= DNX_DATA_F_FEATURE;
            flags |= DNX_DATA_F_TABLE;
        }
        else if (!sal_strncasecmp(arg, "property", strlen(arg)))
        {
            /*
             * Only numeric, feature and table support property
             */
            flags |= DNX_DATA_F_PROPERTY;
            flags |= DNX_DATA_F_NUMERIC;
            flags |= DNX_DATA_F_FEATURE;
            flags |= DNX_DATA_F_TABLE;
        }
        else if (!sal_strncasecmp(arg, "feature", strlen(arg)))
        {
            flags |= DNX_DATA_F_FEATURE;
        }
        else if (!sal_strncasecmp(arg, "define", strlen(arg)))
        {
            flags |= DNX_DATA_F_DEFINE;
        }
        else if (!sal_strncasecmp(arg, "table", strlen(arg)))
        {
            flags |= DNX_DATA_F_TABLE;
        }
        else if (!sal_strncasecmp(arg, "numeric", strlen(arg)))
        {
            flags |= DNX_DATA_F_NUMERIC;
        }
        else
        {
            /*
             * Free resources
             */
            if (datatokens != NULL)
            {
                utilex_str_split_free(datatokens, nof_tokens);
            }
            /*
             * Expected format - <module>.<submodule>.<data>
             */
            datatokens = utilex_str_split(arg, ".", 3, &nof_tokens);
            if (nof_tokens > 0)
            {
                module = datatokens[0];
            }
            if (nof_tokens > 1)
            {
                submodule = datatokens[1];
            }
            if (nof_tokens > 2)
            {
                data = datatokens[2];
            }
            if (nof_tokens > 3)
            {
                SHR_CLI_EXIT(_SHR_E_PARAM, ": data info - data format not supported - %s\n", arg);
            }
            /*
             * set empty token to null
             */
            if (module != NULL && module[0] == 0)
            {
                module = NULL;
            }
            if (submodule != NULL && submodule[0] == 0)
            {
                submodule = NULL;
            }
            if (data != NULL && data[0] == 0)
            {
                data = NULL;
            }
            /*
             * Set '*' along the chain
             */
            if (module != NULL && !sal_strncasecmp(module, "*", strlen("*")))
            {
                submodule = "*";
                data = "*";
            }
            if (submodule != NULL && !sal_strncasecmp(submodule, "*", strlen("*")))
            {
                data = "*";
            }
        }
    }

    SHR_CLI_EXIT_IF_ERR(diag_dnx_data_info(unit, flags, module, submodule, data),
                                    "ERROR: diag dnx data info - command failed\n");
exit:
/*
 * Free resources
 */
    if (datatokens != NULL)
    {
        utilex_str_split_free(datatokens, nof_tokens);
    }
    SHR_FUNC_EXIT;
}

static sh_sand_option_t dnx_data_property_options[] = {
    {"variable",  SAL_FIELD_TYPE_STR,  "Filter soc properties by this substring", NULL},
    {NULL}
};

static sh_sand_man_t dnx_data_property_man = {
    "Get list of soc properties contains substring <property> or info about soc property named <property>",
    "Please fill full description here",
    "data property [name]",
    "Please provide at least one example"
};

/**
 * \brief - parse args and call to diag_dnx_data_property() with the requested data params
 */
static shr_error_e
cmd_dnx_data_property(
    int unit,
    args_t * args,
    sh_sand_control_t *sand_control)
{
    char *property;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    SH_SAND_GET_STR("variable", property);

    /*
     * Call to dnx data dump with the relevant params
     */
    SHR_CLI_EXIT_IF_ERR(diag_dnx_data_property(unit, property),
                                    "ERROR: diag dnx data info - command failed\n");

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print property info according to the input structure 
 */
static shr_error_e
diag_dnx_data_property_info(
    int unit,
    char *module,
    char *submodule,
    char *data_type,
    char *data,
    dnx_data_property_t * property)
{
    int map_index;
    char *prefix;
    SHR_FUNC_INIT_VARS(unit);

    prefix = module == NULL ? "\t" : "";
    if (module != NULL)
        cli_out("--------------\n");
    cli_out("%sPROPERTY:\n", prefix);
    cli_out("%s\tNAME: '%s'\n", prefix, property->name);
    if (module != NULL)
        cli_out("\t%s: '%s.%s.%s'\n", data_type, module, submodule, data);
    cli_out("%s\tMETHOD: '%s'\n", prefix, property->method_str);

    switch (property->method)
    {
        case dnx_data_property_method_range:
        case dnx_data_property_method_port_range:
        case dnx_data_property_method_suffix_range:
            cli_out("%s\tRANGE: [%d - %d]\n", prefix, property->range_min, property->range_max);
            break;
        case dnx_data_property_method_direct_map:
        case dnx_data_property_method_port_direct_map:
        case dnx_data_property_method_suffix_direct_map:
            cli_out("%s\tMAPPING: \n", prefix);
            for (map_index = 0; map_index < property->nof_mapping; map_index++)
            {
                cli_out("%s\t\t\t '%s' -> '%d' \n", prefix, property->mapping[map_index].name,
                        property->mapping[map_index].val);
            }
            break;
        default:
            /*
             * do nothing
             */
            break;
    }
    cli_out("%s\tDOC: \n'\n%s\n'\n", prefix, property->doc);
    if (module != NULL)
        cli_out("--------------\n");

    SHR_FUNC_EXIT;
}

/**
 * \brief - print list of defines according to flags
 */
static shr_error_e
diag_dnx_data_define_list(
    int unit,
    dnx_data_define_t * defines,
    int nof_defines,
    uint32 flags)
{
    int define_index;
    int dump, count;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Dump table info
     */
    if (flags & DNX_DATA_F_NUMERIC)
    {
        cli_out("LIST OF NUMERICS:\n");
    }
    else
    {
        cli_out("LIST OF DEFINES:\n");
    }

    cli_out("-\n");
    /*
     * Iterate over defines
     */
    count = 0;
    for (define_index = 0; define_index < nof_defines; define_index++)
    {

        /*
         * Check if data is supported
         */
        rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, defines[define_index].flags, flags, &dump);
        SHR_IF_ERR_EXIT(rv);

        if (dump)
        {
            cli_out("%-30s", defines[define_index].name);
            count++;
            if (count % 3 == 0)
                cli_out("\n");
        }
    }
    cli_out("\n");
    cli_out("-\n");

exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief - print info about the define named 'data' 
 *          if 'data' == '*" print info of all defines according to flags. 
 */
static shr_error_e
diag_dnx_data_define_info(
    int unit,
    dnx_data_define_t * defines,
    int nof_defines,
    uint32 flags,
    char *data)
{
    int define_index;
    int dump;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    if (data == NULL)   /*list of defnies mode */
    {
        SHR_IF_ERR_EXIT(diag_dnx_data_define_list(unit, defines, nof_defines, flags));
    }
    else
    {
        for (define_index = 0; define_index < nof_defines; define_index++)
        {
            if (!sal_strncasecmp(data, defines[define_index].name, strlen(data)) ||
                !sal_strncasecmp(data, "*", strlen("*")))
            {
                /*
                 * Check if data is supported/match flags
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, defines[define_index].flags, flags, &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)       /*if supproted/match flags */
                {
                    cli_out("-\n");
                    if (defines[define_index].flags & DNX_DATA_F_NUMERIC)
                    {
                        cli_out("NUMERIC: '%s'\n", defines[define_index].name);
                    }
                    else
                    {
                        cli_out("DEFINE: '%s'\n", defines[define_index].name);
                    }
                    cli_out("\tDOC: '%s'\n", defines[define_index].doc);
                    if (defines[define_index].flags & DNX_DATA_F_INIT_ONLY)
                    {
                        cli_out("\tINIT_ONLY\n");
                    }

                    if (defines[define_index].property.name != NULL)
                    {
                        SHR_IF_ERR_EXIT(diag_dnx_data_property_info(unit, NULL, NULL,
                                                                    NULL, NULL, &defines[define_index].property));
                    }
                    cli_out("-\n");
                }
            }
        }
    }

    cli_out("\n");

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print list of features according to flags
 */
static shr_error_e
diag_dnx_data_feature_list(
    int unit,
    dnx_data_feature_t * features,
    int nof_features,
    uint32 flags)
{
    int feature_index;
    int dump, count;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * title 
     */
    cli_out("LIST OF FEATURES:\n");
    cli_out("-\n");
    /*
     * Iterate over features
     */
    count = 0;
    for (feature_index = 0; feature_index < nof_features; feature_index++)
    {

        /*
         * Check if data is supported / match flags
         */
        rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, features[feature_index].flags, flags, &dump);
        SHR_IF_ERR_EXIT(rv);

        if (dump)       /* if supported / requested flags */
        {
            cli_out("%-30s", features[feature_index].name);
            count++;
            if (count % 3 == 0)
                cli_out("\n");
        }
    }
    cli_out("\n");
    cli_out("-\n");

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print info about the feature named 'data' 
 *          if 'data' == '*" print info of all features according to flags. 
 */
static shr_error_e
diag_dnx_data_feature_info(
    int unit,
    dnx_data_feature_t * features,
    int nof_features,
    uint32 flags,
    char *data)
{
    int feature_index;
    int dump;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    if (data == NULL)   /*list of features mode */
    {
        SHR_IF_ERR_EXIT(diag_dnx_data_feature_list(unit, features, nof_features, flags));
    }
    else
    {
        for (feature_index = 0; feature_index < nof_features; feature_index++)
        {
            if (!sal_strncasecmp(data, features[feature_index].name, strlen(data)) ||
                !sal_strncasecmp(data, "*", strlen("*")))
            {
                /*
                 * Check if data is supported / match flags
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, features[feature_index].flags, flags,
                                                &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)       /* if supported / match flags */
                {
                    cli_out("-\n");
                    cli_out("FEATURE: '%s'\n", features[feature_index].name);
                    cli_out("\tDOC: '%s'\n", features[feature_index].doc);
                    if (features[feature_index].flags & DNX_DATA_F_INIT_ONLY)
                    {
                        cli_out("\tINIT_ONLY\n");
                    }
                    if (features[feature_index].property.name != NULL)
                    {
                        SHR_IF_ERR_EXIT(diag_dnx_data_property_info(unit, NULL, NULL,
                                                                    NULL, NULL, &features[feature_index].property));
                    }
                    cli_out("-\n");
                }
            }
        }
    }

    cli_out("\n");

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - display table info or dump data according to info_mode      
 */
static shr_error_e
diag_dnx_data_table_dump(
    int unit,
    dnx_data_table_t * table,
    uint32 dump_flags,
    int info_mode)
{
    int key1_index, key2_index, value_index, key_index;
    int key1_size, key2_size;
    char buffer[DNX_DATA_MGMT_MAX_TABLE_VALUE_LENGTH];
    int changed;
    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    cli_out("-\n");
    if (info_mode)
    {
        /*
         * title
         */
        cli_out("TABLE: '%s'\n", table->name);

        /*
         * Display table info
         */
        cli_out("\tDOC: '%s'\n", table->doc);
        if (table->flags & DNX_DATA_F_INIT_ONLY)
        {
            cli_out("\tINIT_ONLY\n");
        }

        /*
         * Dump keys info
         */
        if (table->nof_keys != 0)
        {
            cli_out("\tKEYS:\n");

            for (key_index = 0; key_index < table->nof_keys; key_index++)
            {
                cli_out("\t\t-\n");
                cli_out("\t\tNAME: '%s'\n", table->keys[key_index].name);
                cli_out("\t\tDOC: '%s'\n", table->keys[key_index].doc);
                cli_out("\t\tSIZE: '%d'\n", table->keys[key_index].size);
                cli_out("\t\t-\n");
            }
        }

        /*
         * Dump values info
         */
        cli_out("\tVALUES:\n");
        for (value_index = 0; value_index < table->nof_values; value_index++)
        {
            cli_out("\t\t-\n");
            cli_out("\t\tNAME: '%s'\n", table->values[value_index].name);
            cli_out("\t\tTYPE: '%s'\n", table->values[value_index].type);
            cli_out("\t\tDOC: '%s'\n", table->values[value_index].doc);
            cli_out("\t\tDEFAULT: '%s'\n", table->values[value_index].default_val);
            cli_out("\t\t-\n");
        }

        /*
         * Dump property info
         */
        if (table->property.name != NULL)
        {
            SHR_IF_ERR_EXIT(diag_dnx_data_property_info(unit, NULL, NULL, NULL, NULL, &table->property));
        }
        cli_out("-\n");

    }
    else
    {
        /*
         * Dump table
         */
        /*
         * Get keys size
         */
        key1_size = 0;
        key2_size = 0;
        if (table->nof_keys > 0)
        {
            key1_size = table->keys[0].size;
            if (table->nof_keys > 1)
            {
                key2_size = table->keys[1].size;
            }
        }

        /*
         * define rows for table
         */
        PRT_TITLE_SET("TABLE: '%s'", table->name);
        for (key_index = 0; key_index < table->nof_keys; key_index++)
        {
            PRT_COLUMN_ADD("%s", table->keys[key_index].name);
        }
        PRT_COLUMN_ADD("#");
        for (value_index = 0; value_index < table->nof_values; value_index++)
        {
            PRT_COLUMN_ADD_FLEX(PRT_FLEX_ASCII, "%s", table->values[value_index].name);
        }
        /*
         * Iterate entries and print keys and values
         */
        for (key1_index = 0; key1_index < key1_size || (key1_index == 0); key1_index++)
        {
            for (key2_index = 0; (key2_index < key2_size) || (key2_index == 0); key2_index++)
            {
                /*
                 * Filter changed entries according to dump flags
                 */
                changed = 1;
                if (dump_flags & DNX_DATA_F_CHANGED)
                {
                    SHR_IF_ERR_EXIT(dnx_data_mgmt_table_entry_changed_get
                                    (unit, table, key1_index, key2_index, &changed));
                }
                if (!changed)
                {
                    continue;
                }

                /*
                 *  Print entry
                 */
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                /*
                 * Entry keys
                 */
                if (table->nof_keys == 1)
                {
                    PRT_CELL_SET("%d", key1_index);
                }
                else if (table->nof_keys == 2)
                {
                    PRT_CELL_SET("%d", key1_index);
                    PRT_CELL_SET("%d", key2_index);
                }
                PRT_CELL_SET("#");
                /*
                 * Entry values
                 */
                for (value_index = 0; value_index < table->nof_values; value_index++)
                {
                    /*
                     * Get value as string
                     */
                    table->entry_get(unit, buffer, key1_index, key2_index, value_index);
                    PRT_CELL_SET("%s", buffer);
                }

            }
        }
        PRT_COMMIT;
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print list of tables according to flags
 */
static shr_error_e
diag_dnx_data_table_list(
    int unit,
    dnx_data_table_t * tables,
    int nof_tables,
    uint32 flags)
{
    int table_index;
    int dump, count;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Display table list
     */
    cli_out("LIST OF TABLES:\n");
    cli_out("-\n");
    /*
     * Iterate over tables
     */
    count = 0;
    for (table_index = 0; table_index < nof_tables; table_index++)
    {

        /*
         * Check if data is supported / match flags
         */
        rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, tables[table_index].flags, flags, &dump);
        SHR_IF_ERR_EXIT(rv);

        if (dump)       /*if supported / match flags */
        {
            /*
             * Print name
             */
            cli_out("%-30s", tables[table_index].name);
            count++;
            if (count % 3 == 0)
                cli_out("\n");
        }
    }
    cli_out("\n");
    cli_out("-\n");

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print info about the table named 'data' 
 *          if 'data' == '*" print info of all tables according to flags. 
 */
static shr_error_e
diag_dnx_data_table_info(
    int unit,
    dnx_data_table_t * tables,
    int nof_tables,
    uint32 flags,
    char *data)
{
    int table_index;
    int dump;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Dump table info
     */
    if (data == NULL)   /*list of tables mode */
    {
        /*
         *  Display list of tables
         */
        SHR_IF_ERR_EXIT(diag_dnx_data_table_list(unit, tables, nof_tables, flags));
    }
    else
    {
        for (table_index = 0; table_index < nof_tables; table_index++)
        {
            if (!sal_strncasecmp(data, tables[table_index].name, strlen(data)) ||
                !sal_strncasecmp(data, "*", strlen("*")))
            {
                /*
                 * Check if data is supported / match flags
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, tables[table_index].flags, flags, &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)
                {
                    /*
                     * Display table info
                     */
                    SHR_IF_ERR_EXIT(diag_dnx_data_table_dump(unit, &tables[table_index], flags, 1));
                }       /*Dump table */
            }   /*if selected table */
        }
    }

    cli_out("\n");

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print list of submodules
 */
static shr_error_e
diag_dnx_data_submodule_list(
    int unit,
    dnx_data_module_t * module)
{
    int count = 0;
    int submodule_index;
    dnx_data_submodule_t *submodules = module->submodules;

    SHR_FUNC_INIT_VARS(unit);

    cli_out("LIST OF SUBMODULES:\n");
    cli_out("---\n");
    /*
     * Iterate over submodules
     */
    for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
    {
        /*
         * Print name
         */
        cli_out("%-30s", submodules[submodule_index].name);
        count++;
        if (count % 3 == 0)
            cli_out("\n");
    }
    cli_out("\n");

    SHR_FUNC_EXIT;
}

/**
 * \brief - print info about the submodule named 'submodule' 
 *          if 'submodule' == "*" print info of all submodules according to flags. 
 */
static shr_error_e
diag_dnx_data_module_info(
    int unit,
    dnx_data_module_t * module,
    uint32 flags,
    char *submodule,
    char *data)
{
    int submodule_index;
    dnx_data_submodule_t *submodules = module->submodules;

    SHR_FUNC_INIT_VARS(unit);

    if (submodule == NULL)      /*list of submodules mode */
    {
        SHR_IF_ERR_EXIT(diag_dnx_data_submodule_list(unit, module));
    }
    else
    {
        /*
         * Iterate over submodules
         */
        for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
        {
            if (!sal_strncasecmp(submodule, submodules[submodule_index].name, strlen(submodule)) ||
                !sal_strncasecmp(submodule, "*", strlen("*")))
            {
                cli_out("SUBMODULE: %s \n", submodules[submodule_index].name);
                cli_out("---:\n");
                cli_out("%s \n\n", submodules[submodule_index].doc);

                /*
                 * Defines info
                 */
                if ((flags & DNX_DATA_F_DEFINE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_define_info(unit, submodules[submodule_index].defines,
                                                              submodules[submodule_index].nof_defines,
                                                              flags | DNX_DATA_F_DEFINE, data));
                }

                /*
                 * Numeric info
                 */
                if ((flags & DNX_DATA_F_DEFINE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_define_info(unit, submodules[submodule_index].defines,
                                                              submodules[submodule_index].nof_defines,
                                                              flags | DNX_DATA_F_NUMERIC, data));
                }

                /*
                 * Feature info
                 */
                if ((flags & DNX_DATA_F_FEATURE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_feature_info(unit, submodules[submodule_index].features,
                                                               submodules[submodule_index].nof_features, flags, data));
                }

                /*
                 * Table info
                 */
                if ((flags & DNX_DATA_F_TABLE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_table_info(unit, submodules[submodule_index].tables,
                                                             submodules[submodule_index].nof_tables, flags, data));
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print list of modules
 */
static shr_error_e
diag_dnx_data_list(
    int unit)
{
    int module_index;
    int count = 0;
    dnx_data_module_t *modules = _dnx_data[unit].modules;
    SHR_FUNC_INIT_VARS(unit);

    cli_out("LIST OF MODULES:\n");
    cli_out("----------------\n");
    /*
     * Iterate over modules
     */
    for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
    {
        /*
         * Print name
         */
        cli_out("%-30s", modules[module_index].name);
        count++;
        if (count % 3 == 0)
            cli_out("\n");
    }
    cli_out("\n");

    SHR_FUNC_EXIT;
}

/**
 * \brief - see definition on local function section (top of this file)
 */
static shr_error_e
diag_dnx_data_info(
    int unit,
    uint32 flags,
    char *module,
    char *submodule,
    char *data)
{
    int module_index;

    dnx_data_module_t *modules = _dnx_data[unit].modules;
    SHR_FUNC_INIT_VARS(unit);

    cli_out("DNX DATA INFO:\n");
    cli_out("==============\n");

    if (module == NULL) /*list of modules mode */
    {
        /*
         * Display module list
         */
        SHR_IF_ERR_EXIT(diag_dnx_data_list(unit));
    }
    else
    {
        /*
         * Iterate over modules
         */
        for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
        {
            if (!sal_strncasecmp(module, modules[module_index].name, strlen(module)) ||
                !sal_strncasecmp(module, "*", strlen("*")))
            {
                cli_out("MODULE %s \n", modules[module_index].name);
                cli_out("---------------:\n");
                SHR_IF_ERR_EXIT(diag_dnx_data_module_info(unit, &modules[module_index], flags, submodule, data));
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print data of define named 'data' 
 *          if 'data' == '*" print data of all defines according to flags. 
 */
static shr_error_e
diag_dnx_data_defines_dump(
    int unit,
    dnx_data_define_t * defines,
    int nof_defines,
    uint32 flags,
    char *data)
{
    int define_index;
    int dump;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    if (data == NULL)   /*defines list mode */
    {
        /*
         * Display numeric / define list
         */
        SHR_IF_ERR_EXIT(diag_dnx_data_define_list(unit, defines, nof_defines, flags));
    }
    else
    {
        /*
         * Iterate over defines
         */
        for (define_index = 0; define_index < nof_defines; define_index++)
        {
            if (!sal_strncasecmp(data, "*", strlen("*")) ||
                !sal_strncasecmp(data, defines[define_index].name, strlen(data)))
            {
                /*
                 * Check if data is supported / match flags
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, defines[define_index].flags, flags, &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)       /* if supported / match flags */
                {
                    /*
                     * Dump define / numeric 
                     */
                    cli_out("---\n");
                    if (flags & DNX_DATA_F_NUMERIC)
                    {
                        cli_out("NUMERIC: '%s'        VALUE: '%d'\n", defines[define_index].name,
                                defines[define_index].data);
                    }
                    else
                    {
                        cli_out("DEFINE: '%s'        VALUE: '%d'\n", defines[define_index].name,
                                defines[define_index].data);
                    }
                    cli_out("---\n");
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print data of feature named 'data' 
 *          if 'data' == '*" print data of all features according to flags. 
 */
static shr_error_e
diag_dnx_data_features_dump(
    int unit,
    dnx_data_feature_t * features,
    int nof_features,
    uint32 flags,
    char *data)
{
    int feature_index;
    int dump;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    if (data == NULL)   /*features list mode */
    {
        /*
         * Display list of features
         */
        SHR_IF_ERR_EXIT(diag_dnx_data_feature_list(unit, features, nof_features, flags));
    }
    else
    {
        /*
         * Iterate over features
         */
        for (feature_index = 0; feature_index < nof_features; feature_index++)
        {
            if (!sal_strncasecmp(data, "*", strlen("*")) ||
                !sal_strncasecmp(data, features[feature_index].name, strlen(data)))
            {
                /*
                 * Check if data is supported / match flags
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, features[feature_index].flags, flags,
                                                &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)       /* if supported / match flags */
                {
                    /*
                     * Dump data
                     */
                    cli_out("---\n");
                    cli_out("FEATURE: '%s' %s\n", features[feature_index].name,
                            features[feature_index].data == 0 ? " - disabled" : "");
                    cli_out("---\n");
                }
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print data of table named 'data' 
 *          if 'data' == '*" print data of all tables according to flags. 
 */
static shr_error_e
diag_dnx_data_tables_dump(
    int unit,
    dnx_data_table_t * tables,
    int nof_tables,
    uint32 flags,
    char *data)
{
    int table_index;
    int dump;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    if (data == NULL)   /*tables list mode */
    {
        /*
         * Display list of tables
         */
        SHR_IF_ERR_EXIT(diag_dnx_data_table_list(unit, tables, nof_tables, flags));
    }
    else
    {
        /*
         * Iterate over tables
         */
        for (table_index = 0; table_index < nof_tables; table_index++)
        {
            if (!sal_strncasecmp(data, "*", strlen("*")) ||
                !sal_strncasecmp(data, tables[table_index].name, strlen(data)))
            {
                /*
                 * Check if data is supported / match flags
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, tables[table_index].flags, flags, &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)       /* if supported / match flags */
                {
                    /*
                     * Dump data
                     */
                    SHR_IF_ERR_EXIT(diag_dnx_data_table_dump(unit, &tables[table_index], flags, 0));
                }       /*Dump table */
            }   /*if selected table */
        }       /*tables iterate */
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - print data of submodule named 'submodule' 
 *          if 'submodule' == '*" print data of all submodules according to flags. 
 */
static shr_error_e
diag_dnx_data_module_dump(
    int unit,
    dnx_data_module_t * module,
    uint32 flags,
    char *submodule,
    char *data)
{
    int submodule_index;
    dnx_data_submodule_t *submodules = module->submodules;
    SHR_FUNC_INIT_VARS(unit);

    if (submodule == NULL)      /*submodule list mode */
    {
        /*
         * Display list of submodules
         */
        SHR_IF_ERR_EXIT(diag_dnx_data_submodule_list(unit, module));
    }
    else
    {
        /*
         * Iterate over submodules
         */
        for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
        {
            if (!sal_strncasecmp(submodule, "*", strlen("*")) ||
                !sal_strncasecmp(submodule, submodules[submodule_index].name, strlen(submodule)))
            {
                cli_out("SUBMODULE: %s \n", submodules[submodule_index].name);
                cli_out("----------------\n");

                /*
                 * Defines
                 */
                if ((flags & DNX_DATA_F_DEFINE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_defines_dump(unit, submodules[submodule_index].defines,
                                                               submodules[submodule_index].nof_defines,
                                                               flags | DNX_DATA_F_DEFINE, data));
                }

                /*
                 * Numerics
                 */
                if ((flags & DNX_DATA_F_DEFINE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_defines_dump(unit, submodules[submodule_index].defines,
                                                               submodules[submodule_index].nof_defines,
                                                               flags | DNX_DATA_F_NUMERIC, data));
                }

                /*
                 * Features
                 */
                if ((flags & DNX_DATA_F_FEATURE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_features_dump(unit, submodules[submodule_index].features,
                                                                submodules[submodule_index].nof_features, flags, data));
                }

                /*
                 * Tables
                 */
                if ((flags & DNX_DATA_F_TABLE) || flags == 0)
                {
                    SHR_IF_ERR_EXIT(diag_dnx_data_tables_dump(unit, submodules[submodule_index].tables,
                                                              submodules[submodule_index].nof_tables, flags, data));
                }

                cli_out("----------------\n\n");
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief - see definition on local function section (top of this file)
 */
shr_error_e
diag_dnx_data_dump(
    int unit,
    uint32 flags,
    char *module,
    char *submodule,
    char *data)
{
    int module_index;

    dnx_data_module_t *modules = _dnx_data[unit].modules;
    SHR_FUNC_INIT_VARS(unit);

    cli_out("DNX DATA DUMP:\n");
    cli_out("==============\n");
    cli_out("==============\n\n");

    if (module == NULL) /*module list mode */
    {
        /*
         * Display list of modules
         */
        SHR_IF_ERR_EXIT(diag_dnx_data_list(unit));
    }
    else
    {
        /*
         * Iterate over modules
         */
        for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
        {
            if (!sal_strncasecmp(module, "*", strlen("*")) ||
                !sal_strncasecmp(module, modules[module_index].name, strlen(module)))
            {
                /*
                 * Dump module
                 */
                cli_out("MODULE: %s \n", modules[module_index].name);
                cli_out("================\n");
                SHR_IF_ERR_EXIT(diag_dnx_data_module_dump(unit, &modules[module_index], flags, submodule, data));
                cli_out("================\n\n");
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - see definition on local function section (top of this file)
 */
static shr_error_e
diag_dnx_data_property(
    int unit,
    char *property)
{
    int module_index, submodule_index, table_index, feature_index, define_index;

    dnx_data_module_t *modules = _dnx_data[unit].modules;
    dnx_data_module_t *module;
    dnx_data_submodule_t *submodule;
    dnx_data_table_t *table;
    dnx_data_feature_t *feature;
    dnx_data_define_t *define;
    int dump;
    shr_error_e rv;
    SHR_FUNC_INIT_VARS(unit);

    cli_out("DNX DATA PROPERTY:\n");
    cli_out("==============\n");
    cli_out("==============\n\n");

    /*
     * Iterate over modules
     */
    for (module_index = 0; module_index < _dnx_data[unit].nof_modules; module_index++)
    {
        module = &modules[module_index];

        /*
         * iterate of submodules
         */
        for (submodule_index = 0; submodule_index < module->nof_submodules; submodule_index++)
        {
            submodule = &module->submodules[submodule_index];

            /*
             * iterate of tables
             */
            for (table_index = 0; table_index < submodule->nof_tables; table_index++)
            {
                table = &submodule->tables[table_index];
                /*
                 * Check if data is supported
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, table->flags, 0, &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)
                {
                    /*
                     * check if data loaded be SoC property
                     */
                    if (table->property.name != NULL)
                    {
                        /*
                         *check if identical
                         */
                        if (!sal_strncasecmp(table->property.name, property, strlen(table->property.name)))
                        {
                            SHR_IF_ERR_EXIT(diag_dnx_data_property_info(unit, module->name, submodule->name,
                                                                        "TABLE", table->name, &table->property));
                        }
                        else if (strcaseindex(table->property.name, property) != NULL)  /*check if substring */
                        {
                            cli_out("PROPERTY: '%s'\n", table->property.name);
                        }
                    }
                }       /*Dump table */
            }   /*table iterator */

            /*
             * iterate of features
             */
            for (feature_index = 0; feature_index < submodule->nof_features; feature_index++)
            {
                feature = &submodule->features[feature_index];
                /*
                 * Check if data is supported
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, feature->flags, 0, &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)
                {
                    /*
                     * check if data loaded be SoC property
                     */
                    if (feature->property.name != NULL)
                    {
                        /*
                         * check if identical
                         */
                        if (!sal_strncasecmp(feature->property.name, property, strlen(feature->property.name)))
                        {
                            SHR_IF_ERR_EXIT(diag_dnx_data_property_info(unit, module->name, submodule->name,
                                                                        "FEATURE", feature->name, &feature->property));
                        }
                        /*
                         * check if substring
                         */
                        else if (strcaseindex(feature->property.name, property) != NULL)
                        {
                            cli_out("PROPERTY: '%s'\n", feature->property.name);
                        }
                    }
                }       /*Dump feature */
            }   /*feature iterator */

            /*
             * iterate of numerics
             */
            for (define_index = 0; define_index < submodule->nof_defines; define_index++)
            {
                define = &submodule->defines[define_index];
                /*
                 * Check if data is supported
                 */
                rv = dnx_data_utils_dump_verify(unit, _dnx_data[unit].state, define->flags, 0, &dump);
                SHR_IF_ERR_EXIT(rv);

                if (dump)
                {
                    /*
                     * check if data loaded be SoC property
                     */
                    if (define->property.name != NULL)
                    {
                        /*
                         * check if identical
                         */
                        if (!sal_strncasecmp(define->property.name, property, strlen(define->property.name)))
                        {
                            SHR_IF_ERR_EXIT(diag_dnx_data_property_info(unit, module->name, submodule->name,
                                                                        "NUMERIC", define->name, &define->property));
                        }
                        /*
                         * check if substring
                         */
                        else if (strcaseindex(define->property.name, property) != NULL)
                        {
                            cli_out("PROPERTY: '%s'\n", define->property.name);
                        }
                    }
                }       /*Dump numeric */
            }   /*numeric iterator */

        }       /*submodule iterator */
    }   /*module iterator */

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief DNX DATA diagnostic pack
 * List of the supported commands, pointer to command function and command usage function.
 */
sh_sand_cmd_t sh_dnx_data_cmds[] = {
    /*keyword,   action,                command, options,                   man                   */
    {"dump",     cmd_dnx_data_dump,     NULL,    dnx_data_dump_options,     &dnx_data_dump_man, NULL, TRUE},
    {"info",     cmd_dnx_data_info,     NULL,    dnx_data_info_options,     &dnx_data_info_man, NULL, TRUE},
    {"property", cmd_dnx_data_property, NULL,    dnx_data_property_options, &dnx_data_property_man},
    {NULL}
};

sh_sand_man_t sh_dnx_data_man = {
    "Misc facilities for displaying dnx data information",
    NULL,
    NULL,
    NULL,
};

const char cmd_dnx_data_usage[] = "Display dnx data information";

/*
 * }
 */
