/**
 * \file dbal_string_mgmt.c
 * $Id$
 *
 * Main functions for dbal strings (names) conversion
 * basically string_to_id and to_string functions
 *
 * Strings mapping array - will be removed in two stages:
 *  1. when auto-coder will be integrated
 *  2. effiecient hashing function
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
#include <shared/utilex/utilex_str.h>
#include "dbal_string_mgmt.h"

/*************
 * STATICS   *
 *************/
/** 
 * defualt unknown string
 */
static char *unknown_string = "unknown_id";
extern char *dbal_field_strings[DBAL_NOF_FIELDS];
extern char *dbal_table_strings[DBAL_NOF_TABLES];
extern dbal_hw_entity_mapping dbal_hw_entities_strings[DBAL_NOF_HW_ENTITIES];

/** 
 * Strings mapping array for dbal pyisical tables:
 */
static char *dbal_physical_table_strings[DBAL_NOF_PHYSICAL_TABLES] = {
    "NONE",
    "TCAM",
    "LPM_PRIVATE",
    "LPM_PUBLIC",
    "ISEM_1",
    "INLIF_1",
    "IVSI",
    "ISEM_2",
    "INLIF_2",
    "ISEM_3",
    "INLIF_3",
    "LEM",
    "IOEM_0",
    "IOEM_1",
    "MAP",
    "FEC_1",
    "FEC_2",
    "FEC_3",
    "MC_ID",
    "GLEM_0",
    "GLEM_1",
    "EEDB_1",
    "EEDB_2",
    "EEDB_3",
    "EEDB_4",
    "EOEM_0",
    "EOEM_1",
    "ESEM",
    "EVSI",
    "EXEM_1",
    "EXEM_2",
    "EXEM_3",
    "EXEM_4",
    "RMEP"
};

/** 
 * Strings mapping array for dbal labels:
 */
static char *dbal_label_strings[DBAL_NOF_LABEL_TYPES] = {
    "NONE",
    "SYSTEM",
    "L2",
    "L3",
    "MPLS",
    "FCOE",
    "SNIF",
    "ECGM",
    "ICGM",
    "SCHEDULER",
    "ITM",
    "ETM",
    "STACK",
    "LAG",
    "MULTICAST",
    "FABRIC",
    "NIF",
    "FC",
    "CRPS",
    "MIRROR",
    "SNOOP",
    "OAM",
    "BFD",
    "PARSER",
    "VSWITCH",
    "PP_PORT",
    "VLAN_TRANSLATION",
    "VLAN",
    "L2_LEARNING",
    "VPLS",
    "VPWS",
    "EVPN",
    "TRILL",
    "L2GRE",
    "MIN",
    "VXLAN",
    "ROO",
    "HASHING",
    "QOS_PHB",
    "QOS_REMARKING",
    "STG",
    "L3_IF_RIF",
};

/** 
 * Strings mapping array for dbal core modes:
 */
static char *dbal_core_mode_strings[DBAL_NOF_CORE_MODE_TYPES] = {
    "NONE",
    "DPC",
    "SBC"
};

/** 
 * Strings mapping array for dbal condition types:
 */
static char *dbal_condition_strings[DBAL_NOF_CONDITION_TYPES] = {
    "NONE",
    "BIGGER_THAN",
    "LOWER_THAN",
    "EQUAL_TO",
    "IS_EVEN",
    "IS_ODD"
};

/**
 * Strings mapping array for dbal field types:
 */
static char *dbal_field_type_strings[DBAL_NOF_FIELD_TYPES] = {
    "NONE",
    "BOOL",
    "INT32",
    "UINT32",
    "IP",
    "ARRAY8",
    "ARRAY32",
    "BITMAP",
    "ENUM"
};

/**
 * Strings mapping array for dbal encode types: 
 * also need the update coreesponding parm1 
 * dbal_encode_type_param1_strings
 */
static char *dbal_field_encode_type_strings[DBAL_NOF_VALUE_FIELD_ENCODE_TYPES] = {
    "NONE",
    "BOOL",
    "PREFIX",
    "SUFFIX",
    "SUBTRACT",
    "HARD_VALUE",
    "ENUM"
};

/**
 * Strings mapping array for dbal encode types: 
 * also need the update coreesponding parm1 
 * dbal_encode_type_param1_strings
 */
static char *dbal_offset_encode_type_strings[DBAL_NOF_VALUE_OFFSET_ENCODE_TYPES] = {
    "NONE",
    "BOOL",
    "MODULO",
    "DIVIDE",
    "MULTIPLE",
    "SUBTRACT",
    "PARTIAL_KEY",
    "HARD_VALUE",
    "MODULO_FIELD"
};

/**
 * Strings mapping array for dbal encode parameter1: 
 * (string that representing the meaning of param1) 
 * if the  string is "" there is no parameters print at all 
 * if the string is " " will print the name only
 */
static char *dbal_offset_encode_type_conjunction_strings[DBAL_NOF_VALUE_OFFSET_ENCODE_TYPES] = {
    /*
     * NONE
     */
    "",

    /*
     * BOOL 
     */
    "",

    /*
     * MODULO 
     */
    "by",

    /*
     * DIVIDE 
     */
    "by",

    /*
     * MULTIPLE 
     */
    "by",

    /*
     * SUBTRACT 
     */
    "by",

    /*
     * PARTIAL_KEY 
     */
    " ",

    /*
     * HARD_VALUE 
     */
    " ",

    /*
     * MODULO_FIELD
     */
    "by"
};

/**
 * Strings mapping array for dbal action flags:
 */
static char *dbal_action_flag_strings[DBAL_COMMIT_NOF_OPCODES] = {
    "NORMAL",
    "KEEP_HANDLE",
    "OVERRUN_ENTRY",
    "GET_ALL_FIELDS"
};

/** 
 * Strings mapping array for dbal access types:
 */
static char *dbal_access_method_strings[DBAL_NOF_ACCESS_METHODS] = {
    "MDB Phy",
    "Hard Logic",
    "SW Only",
    "Pemla"
};

/** 
 * Strings mapping array for dbal table type:
 */
static char *dbal_logical_table_type_strings[DBAL_NOF_TABLE_TYPES] = {
    "NONE",
    "EM",
    "TCAM",
    "LPM",
    "DIRECT"
};

/*************
 * FUNCTIONS *
 *************/
shr_error_e
dbal_field_string_to_id(
    int unit,
    char *str,
    dbal_fields_e * field_id)
{
    dbal_fields_e ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_FIELDS; ii++)
    {
        if (sal_strcasecmp(dbal_field_strings[ii], str) == 0)
        {
            *field_id = ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_field_string_to_id: " "field not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_logical_table_string_to_id(
    int unit,
    char *str,
    dbal_tables_e * log_table_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_TABLES; ii++)
    {
        if (sal_strcasecmp(dbal_table_strings[ii], str) == 0)
        {
            *log_table_id = (dbal_tables_e) ii;
            SHR_EXIT();
        }
    }
    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_logical_table_string_to_id: " "log table not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_hw_entity_string_to_id(
    int unit,
    char *str,
    int *hw_entity_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_HW_ENTITIES; ii++)
    {
        if (sal_strcasecmp(dbal_hw_entities_strings[ii].hw_entity_name, str) == 0)
        {
            *hw_entity_id = dbal_hw_entities_strings[ii].hw_entity_val;
            SHR_EXIT();
        }
    }
    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_hw_entities_string_to_id: " "hw entity not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_physical_table_string_to_id(
    int unit,
    char *str,
    dbal_physical_tables_e * phy_table_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_PHYSICAL_TABLES; ii++)
    {
        if (sal_strcasecmp(dbal_physical_table_strings[ii], str) == 0)
        {
            *phy_table_id = (dbal_physical_tables_e) ii;
            SHR_EXIT();
        }
    }
    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_physical_table_string_to_id: " "phy table not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_label_string_to_id(
    int unit,
    char *str,
    dbal_labels_e * label_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_LABEL_TYPES; ii++)
    {
        if (sal_strcasecmp(dbal_label_strings[ii], str) == 0)
        {
            *label_id = (dbal_labels_e) ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_label_string_to_id: " "label not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_core_mode_string_to_id(
    int unit,
    char *str,
    dbal_core_mode_e * core_mode_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_CORE_MODE_TYPES; ii++)
    {
        if (sal_strcasecmp(dbal_core_mode_strings[ii], str) == 0)
        {
            *core_mode_id = (dbal_core_mode_e) ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_core_mode_string_to_id: " "core mode not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_condition_string_to_id(
    int unit,
    char *str,
    dbal_condition_types_e * condition_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_CONDITION_TYPES; ii++)
    {
        if (sal_strcasecmp(dbal_condition_strings[ii], str) == 0)
        {
            *condition_id = (dbal_condition_types_e) ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_condition_string_to_id: " "condition type not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_field_type_string_to_id(
    int unit,
    char *str,
    dbal_field_type_e * field_type_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_FIELD_TYPES; ii++)
    {
        if (sal_strcasecmp(dbal_field_type_strings[ii], str) == 0)
        {
            *field_type_id = (dbal_field_type_e) ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_field_type_string_to_id: " "field type not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_field_encode_type_string_to_id(
    int unit,
    char *str,
    dbal_value_field_encode_types_e * encode_type_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_VALUE_FIELD_ENCODE_TYPES; ii++)
    {
        if (sal_strcasecmp(dbal_field_encode_type_strings[ii], str) == 0)
        {
            *encode_type_id = (dbal_value_field_encode_types_e) ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_field_encode_type_string_to_id: " "encode type not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_offset_encode_type_string_to_id(
    int unit,
    char *str,
    dbal_value_offset_encode_types_e * encode_type_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_VALUE_OFFSET_ENCODE_TYPES; ii++)
    {
        if (sal_strcasecmp(dbal_offset_encode_type_strings[ii], str) == 0)
        {
            *encode_type_id = (dbal_value_offset_encode_types_e) ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_offset_encode_type_string_to_id: " "encode type not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_logical_table_type_string_to_id(
    int unit,
    char *str,
    dbal_table_type_e * table_type_id)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_TABLE_TYPES; ii++)
    {
        if (sal_strcasecmp(dbal_logical_table_type_strings[ii], str) == 0)
        {
            *table_type_id = (dbal_table_type_e) ii;
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "dbal_logical_table_type_string_to_id: " "table type not found (%s)\n", str);
exit:
    SHR_FUNC_EXIT;
}

char *
dbal_field_to_string(
    int unit,
    dbal_fields_e field_id)
{
    if (field_id < DBAL_NOF_FIELDS)
    {
        return dbal_field_strings[field_id];
    }
    return unknown_string;
}

char *
dbal_logical_table_to_string(
    int unit,
    dbal_tables_e log_table_id)
{
    if (log_table_id < DBAL_NOF_TABLES)
    {
        return dbal_table_strings[log_table_id];
    }
    return unknown_string;
}

char *
dbal_hw_entity_to_string(
    int unit,
    int hw_entity_id)
{
    int ii;

    for (ii = 0; ii < DBAL_NOF_HW_ENTITIES; ii++)
    {
        if (dbal_hw_entities_strings[ii].hw_entity_val == hw_entity_id)
        {
            return dbal_hw_entities_strings[ii].hw_entity_name;
        }
    }
    return unknown_string;
}

char *
dbal_physical_table_to_string(
    int unit,
    dbal_physical_tables_e phy_table_id)
{
    if (phy_table_id < DBAL_NOF_PHYSICAL_TABLES)
    {
        return dbal_physical_table_strings[phy_table_id];
    }
    return unknown_string;
}

char *
dbal_label_to_string(
    int unit,
    dbal_labels_e label_id)
{
    if (label_id < DBAL_NOF_LABEL_TYPES)
    {
        return dbal_label_strings[label_id];
    }

    return unknown_string;
}

char *
dbal_core_mode_to_string(
    int unit,
    dbal_core_mode_e core_mode_id)
{
    if (core_mode_id < DBAL_NOF_CORE_MODE_TYPES)
    {
        return dbal_core_mode_strings[core_mode_id];
    }
    return unknown_string;
}

char *
dbal_condition_to_string(
    int unit,
    dbal_condition_types_e condition_id)
{
    if (condition_id < DBAL_NOF_CONDITION_TYPES)
    {
        return dbal_condition_strings[condition_id];
    }
    return unknown_string;
}

char *
dbal_field_type_to_string(
    int unit,
    dbal_field_type_e field_type_id)
{
    if (field_type_id < DBAL_NOF_FIELD_TYPES)
    {
        return dbal_field_type_strings[field_type_id];
    }
    return unknown_string;
}

char *
dbal_field_encode_type_to_string(
    int unit,
    dbal_value_field_encode_types_e encode_type_id)
{
    if (encode_type_id < DBAL_NOF_VALUE_FIELD_ENCODE_TYPES)
    {
        return dbal_field_encode_type_strings[encode_type_id];
    }
    return unknown_string;
}

char *
dbal_offset_encode_type_to_string(
    int unit,
    dbal_value_offset_encode_types_e encode_type_id)
{
    if (encode_type_id < DBAL_NOF_VALUE_OFFSET_ENCODE_TYPES)
    {
        return dbal_offset_encode_type_strings[encode_type_id];
    }
    return unknown_string;
}

char *
dbal_offset_encode_conjunction_string(
    int unit,
    dbal_value_offset_encode_types_e encode_type_id)
{
    if (encode_type_id < DBAL_NOF_VALUE_OFFSET_ENCODE_TYPES)
    {
        return dbal_offset_encode_type_conjunction_strings[encode_type_id];
    }
    return unknown_string;
}

char *
dbal_action_flags_to_string(
    int unit,
    dbal_entry_action_flags_e action_flag_id)
{
    if (action_flag_id < DBAL_COMMIT_NOF_OPCODES)
    {
        return dbal_action_flag_strings[action_flag_id];
    }
    return unknown_string;
}

char *
dbal_access_method_to_string(
    int unit,
    dbal_access_method_e access_method_id)
{
    if (access_method_id < DBAL_NOF_ACCESS_METHODS)
    {
        return dbal_access_method_strings[access_method_id];
    }
    return unknown_string;
}

char *
dbal_logical_table_type_to_string(
    int unit,
    dbal_table_type_e table_type_id)
{
    if (table_type_id < DBAL_NOF_TABLE_TYPES)
    {
        return dbal_logical_table_type_strings[table_type_id];
    }
    return unknown_string;
}


