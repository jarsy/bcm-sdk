/** \file diag_dnx_dbal_xml_parsing.c
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/*************
 * INCLUDES  *
 *************/
#include <sal/appl/sal.h>
#include <shared/bslnames.h>
#include <appl/diag/system.h>
#include <appl/diag/diag.h>
#include "diag_dnx_dbal_internal.h"

#define BSL_LOG_MODULE BSL_LS_SOCDNX_DIAGDBALDNX

/*************
 *  GLOBALS  *
 *************/
/**
 * fields expected DB for XML parsing validation test
 */
dbal_field_basic_info_t fields_data[DBAL_NOF_FIELDS];

/**
 * fields parsed DB for XML parsing validation test
 */
dbal_field_basic_info_t fields_parsed_info[DBAL_NOF_FIELDS];

/** 
 * logical tables expected DB for XML parsing validation test
 */
dbal_logical_tables_info_t tables_data;

/**
 * logical tables parsed DB for XML parsing validation test
 */
dbal_logical_tables_info_t tables_parsed_info;

/**
 * Global memory, use instead of allocation for expected 
 * fields/tables 
 */
/** C_VLAN field */
dbal_enum_decoding_info_t c_vlan_enum_info[4] = { {0}
};

/** MACT table */
dbal_table_field_info_t mact_key_fields[2] = { {0}
};
multi_res_info_t mact_res_types[1] = { {0}
};
dbal_table_field_info_t mact_res_fields[2] = { {0}
};

/** ING_VSI_INFO table */
dbal_table_field_info_t ing_vsi_key_fields[1] = { {0}
};
multi_res_info_t ing_vsi_res_types[1] = { {0}
};
dbal_table_field_info_t ing_vsi_res_fields[3] = { {0}
};

/** INGRESS_VLAN_BMP table */
dbal_table_field_info_t ing_vlan_key_fields[2] = { {0}
};
multi_res_info_t ing_vlan_res_types[1] = { {0}
};
dbal_table_field_info_t ing_vlan_res_fields[1] = { {0}
};
dbal_hl_mapping_multi_res_t ing_vlan_mult_direct_mapping = { {{0}
                                                              }
};
dbal_direct_l2p_field_info_t ing_vlan_memory_access[2] = { {0}
};

/** EGRESS_PORT table */
dbal_table_field_info_t eg_port_key_fields[2] = { {0}
};
multi_res_info_t eg_port_res_types[1] = { {0}
};
dbal_table_field_info_t eg_port_res_fields[2] = { {0}
};
dbal_hl_mapping_multi_res_t eg_port_mult_direct_mapping = { {{0}
                                                             }
};
dbal_direct_l2p_field_info_t eg_port_memory_access[3] = { {0}
};

/*************
 * FUNCTIONS *
 *************/
/**
* \brief
* set the expected fields data for xml parsing validation test
*****************************************************/
static cmd_result_t
diag_dbal_set_expected_fields_data(
    dbal_field_basic_info_t * expected_fields)
{
    dbal_field_basic_info_t *field;

    /*
     * IPV4 field 
     */
    field = &expected_fields[DBAL_FIELD_IPV4];
    sal_strcpy(field->name, "IPV4");
    field->max_size = 32;
    field->type = DBAL_FIELD_TYPE_IP;
    field->labels[0] = DBAL_LABEL_L3;
    field->labels[1] = DBAL_LABEL_L2;
    field->instances_support = 0;
    /*
     * VRF field 
     */
    field = &expected_fields[DBAL_FIELD_VRF];
    sal_strcpy(field->name, "VRF");
    field->max_size = 17;
    field->type = DBAL_FIELD_TYPE_UINT32;
    field->labels[0] = DBAL_LABEL_L3;
    field->encode_info.encode_mode = DBAL_VALUE_FIELD_ENCODE_HARD_VALUE;
    field->encode_info.input_param = -8;
    field->instances_support = 0;

    /*
     * IPV6 field 
     */
    field = &expected_fields[DBAL_FIELD_IPV6];
    sal_strcpy(field->name, "IPV6");
    field->max_size = 128;
    field->type = DBAL_FIELD_TYPE_ARRAY8;
    field->labels[0] = DBAL_LABEL_L3;
    field->labels[1] = DBAL_LABEL_MPLS;
    field->default_value = 15;
    field->is_default_value_valid = TRUE;
    field->encode_info.encode_mode = DBAL_VALUE_FIELD_ENCODE_NONE;
    field->encode_info.input_param = 0;
    field->instances_support = 0;

    /*
     * CORE_ID field 
     */
    field = &expected_fields[DBAL_FIELD_CORE_ID];
    sal_strcpy(field->name, "CORE_ID");
    field->max_size = 1;
    field->type = DBAL_FIELD_TYPE_BOOL;
    field->labels[0] = DBAL_LABEL_L2;
    field->labels[1] = DBAL_LABEL_L3;
    field->labels[2] = DBAL_LABEL_MPLS;
    field->default_value = -1;
    field->is_default_value_valid = TRUE;
    field->instances_support = 0;

    /*
     * C_VLAN field 
     */
    field = &expected_fields[DBAL_FIELD_C_VLAN];
    sal_strcpy(field->name, "C_VLAN");
    field->max_size = 4;
    field->type = DBAL_FIELD_TYPE_ENUM;
    field->labels[0] = DBAL_LABEL_L2;
    field->nof_enum_values = 4;
    field->enum_val_info = &c_vlan_enum_info[0];
    sal_strcpy(field->enum_val_info[0].name, "C_VLAN_DEFAULT");
    sal_strcpy(field->enum_val_info[1].name, "C_VLAN_DROP");
    sal_strcpy(field->enum_val_info[2].name, "C_VLAN_SNOOP");
    sal_strcpy(field->enum_val_info[3].name, "C_VLAN_REDIRECT");
    field->enum_val_info[0].value = 0;
    field->enum_val_info[1].value = 5;
    field->enum_val_info[2].value = 14;
    field->enum_val_info[3].value = 3;
    field->encode_info.encode_mode = DBAL_VALUE_FIELD_ENCODE_ENUM;
    field->instances_support = 0;

    return CMD_OK;
}

/**
* \brief
* set the expected tables data for xml parsing validation test
*****************************************************/
static cmd_result_t
diag_dbal_set_expected_tables_data(
    dbal_logical_table_t * expected_tables)
{
    dbal_logical_table_t *table;

    /*
     * MACT table 
     */
    table = &expected_tables[DBAL_TABLE_MACT];
    sal_strcpy(table->table_name, "MACT");
    table->maturity_level = 1;
    table->is_table_valid = 1;
    table->access_method = DBAL_ACCESS_METHOD_PHY_TABLE;
    table->nof_key_fields = 2;
    table->table_type = DBAL_TABLE_TYPE_EM;

    table->keys_info = &mact_key_fields[0];
    table->keys_info[0].field_id = DBAL_FIELD_FID;
    table->keys_info[0].field_nof_bits = 0;
    table->keys_info[1].field_id = DBAL_FIELD_L2_MAC;
    table->keys_info[1].field_nof_bits = 38;
    table->keys_info[1].offset_in_logical_field = 10;

    table->nof_result_types = 1;
    table->multi_res_info = &mact_res_types[0];
    table->multi_res_info[0].nof_result_fields = 2;
    table->multi_res_info[0].results_info = &mact_res_fields[0];
    table->multi_res_info[0].results_info[0].field_id = DBAL_FIELD_DESTINATION;
    table->multi_res_info[0].results_info[0].field_nof_bits = 21;
    table->multi_res_info[0].results_info[1].field_id = DBAL_FIELD_OUT_LIF;
    table->multi_res_info[0].results_info[1].field_nof_bits = 20;

    table->labels[0] = DBAL_LABEL_L2;
    table->labels[1] = DBAL_LABEL_L3;
    table->core_mode = DBAL_CORE_ALL;
    table->physical_db_id = DBAL_PHYSICAL_TABLE_LEM;
    table->app_id = 15;

    /*
     * ING_VSI_INFO table 
     */
    table = &expected_tables[DBAL_TABLE_ING_VSI_INFO];
    sal_strcpy(table->table_name, "ING_VSI_INFO");
    table->maturity_level = 2;
    table->is_table_valid = 1;
    table->access_method = DBAL_ACCESS_METHOD_PHY_TABLE;
    table->table_type = DBAL_TABLE_TYPE_DIRECT;
    table->nof_key_fields = 1;
    table->keys_info = &ing_vsi_key_fields[0];
    table->keys_info[0].field_id = DBAL_FIELD_VSI;
    table->keys_info[0].field_nof_bits = 17;

    table->nof_result_types = 1;
    table->multi_res_info = &ing_vsi_res_types[0];
    table->multi_res_info[0].nof_result_fields = 3;
    table->multi_res_info[0].results_info = &ing_vsi_res_fields[0];
    table->multi_res_info[0].results_info[0].field_id = DBAL_FIELD_STP_TOPOLOGY_ID;
    table->multi_res_info[0].results_info[0].field_nof_bits = 8;
    table->multi_res_info[0].results_info[1].field_id = DBAL_FIELD_MY_MAC;
    table->multi_res_info[0].results_info[1].field_nof_bits = 10;
    table->multi_res_info[0].results_info[2].field_id = DBAL_FIELD_MY_MAC_PREFIX;
    table->multi_res_info[0].results_info[2].field_nof_bits = 6;

    table->labels[0] = DBAL_LABEL_MPLS;
    table->labels[1] = DBAL_LABEL_FCOE;
    table->core_mode = DBAL_CORE_BY_INPUT;
    table->physical_db_id = DBAL_PHYSICAL_TABLE_IVSI;
    table->app_id = -1;

    /*
     * INGRESS_VLAN_BMP table 
     */
    table = &expected_tables[DBAL_TABLE_INGRESS_VLAN_BMP];
    sal_strcpy(table->table_name, "INGRESS_VLAN_BMP");
    table->maturity_level = 2;
    table->is_table_valid = 1;
    table->access_method = DBAL_ACCESS_METHOD_HARD_LOGIC;
    table->table_type = DBAL_TABLE_TYPE_DIRECT;
    table->nof_key_fields = 2;
    table->keys_info = &ing_vlan_key_fields[0];
    table->keys_info[0].field_id = DBAL_FIELD_VLAN_ID;
    table->keys_info[0].field_nof_bits = 12;
    table->keys_info[1].field_id = DBAL_FIELD_CORE_ID;
    table->keys_info[1].field_nof_bits = 1;

    table->nof_result_types = 1;
    table->multi_res_info = &ing_vlan_res_types[0];
    table->multi_res_info[0].nof_result_fields = 1;
    table->multi_res_info[0].results_info = &ing_vlan_res_fields[0];
    table->multi_res_info[0].results_info[0].field_id = DBAL_FIELD_VLAN_MEMBER_DOMAIN_BMP;
    table->multi_res_info[0].results_info[0].field_nof_bits = 256;

    table->labels[0] = DBAL_LABEL_L2;
    table->labels[1] = DBAL_LABEL_L3;
    table->labels[2] = DBAL_LABEL_MPLS;
    table->core_mode = DBAL_CORE_BY_INPUT;
    table->hl_mapping_multi_res = &ing_vlan_mult_direct_mapping;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].num_of_access_fields = 2;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info = &ing_vlan_memory_access[0];
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].field_id =
        DBAL_FIELD_VLAN_MEMBER_DOMAIN_BMP;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].access_nof_bits = 128;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].memory =
        IPPA_VLAN_MEMBERSHIP_TABLEm;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].hw_field =
        VLAN_PORT_MEMBER_LINEf;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        array_offset_info.encode_mode = DBAL_VALUE_OFFSET_ENCODE_NONE;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        array_offset_info.input_param = 0;

    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].field_id =
        DBAL_FIELD_VLAN_MEMBER_DOMAIN_BMP;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].access_nof_bits = 128;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].access_offset = 128;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].memory =
        IPPA_VLAN_MEMBERSHIP_TABLEm;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].hw_field =
        VLAN_PORT_MEMBER_LINEf;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        array_offset_info.encode_mode = DBAL_VALUE_OFFSET_ENCODE_HARD_VALUE;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        array_offset_info.input_param = 1;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        entry_offset_info.encode_mode = DBAL_VALUE_OFFSET_ENCODE_DIVIDE;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        entry_offset_info.field_id = DBAL_FIELD_VLAN_ID;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        entry_offset_info.input_param = 3;

    /*
     * EGRESS_PORT table 
     */
    table = &expected_tables[DBAL_TABLE_EGRESS_PORT];
    sal_strcpy(table->table_name, "EGRESS_PORT");
    table->maturity_level = 2;
    table->is_table_valid = 1;
    table->access_method = DBAL_ACCESS_METHOD_HARD_LOGIC;
    table->table_type = DBAL_TABLE_TYPE_DIRECT;
    table->nof_key_fields = 2;
    table->keys_info = &eg_port_key_fields[0];
    table->keys_info[0].field_id = DBAL_FIELD_PP_PORT;
    table->keys_info[0].field_nof_bits = 8;
    table->keys_info[1].field_id = DBAL_FIELD_CORE_ID;
    table->keys_info[1].field_nof_bits = 1;

    table->nof_result_types = 1;
    table->multi_res_info = &eg_port_res_types[0];
    table->multi_res_info[0].nof_result_fields = 2;
    table->multi_res_info[0].results_info = &eg_port_res_fields[0];
    table->multi_res_info[0].results_info[0].field_id = DBAL_FIELD_VLAN_DOMAIN;
    table->multi_res_info[0].results_info[0].field_nof_bits = 9;
    table->multi_res_info[0].results_info[1].field_id = DBAL_FIELD_PORT_VID;
    table->multi_res_info[0].results_info[1].field_nof_bits = 12;

    table->labels[0] = DBAL_LABEL_L2;
    table->labels[1] = DBAL_LABEL_L3;
    table->labels[2] = DBAL_LABEL_MPLS;
    table->labels[3] = DBAL_LABEL_FCOE;
    table->core_mode = DBAL_CORE_ALL;
    table->hl_mapping_multi_res = &eg_port_mult_direct_mapping;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].num_of_access_fields = 3;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info = &eg_port_memory_access[0];
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].field_id =
        DBAL_FIELD_VLAN_DOMAIN;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].memory =
        ETPPA_PER_PORT_TABLEm;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].hw_field = VLAN_DOMAINf;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].mapping_condition[0].type =
        DBAL_CONDITION_BIGGER_THAN;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        mapping_condition[0].value = 14;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].mapping_condition[1].type =
        DBAL_CONDITION_LOWER_THAN;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        mapping_condition[1].value = 124;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        mapping_condition[1].field_id = DBAL_FIELD_PP_PORT;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        array_offset_info.encode_mode = DBAL_VALUE_OFFSET_ENCODE_MULTIPLE;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        array_offset_info.field_id = DBAL_FIELD_CORE_ID;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[0].
        array_offset_info.input_param = 4;

    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].field_id =
        DBAL_FIELD_VLAN_DOMAIN;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].memory =
        ETPPA_PER_PORT_TABLEm;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].hw_field = VLAN_DOMAINf;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].mapping_condition[0].type =
        DBAL_CONDITION_EQUAL_TO;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        mapping_condition[0].value = 14;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        entry_offset_info.encode_mode = DBAL_VALUE_OFFSET_ENCODE_HARD_VALUE;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[1].
        entry_offset_info.input_param = 1;

    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[2].field_id =
        DBAL_FIELD_VLAN_DOMAIN;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[2].memory =
        ETPPA_PER_PORT_TABLEm;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[2].mapping_condition[0].type =
        DBAL_CONDITION_LOWER_THAN;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[2].
        mapping_condition[0].value = 14;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[2].
        data_offset_info.encode_mode = DBAL_VALUE_OFFSET_ENCODE_PARTIAL_KEY;
    table->hl_mapping_multi_res[0].l2p_direct_info[DBAL_HL_ACCESS_MEMORY].l2p_fields_info[2].data_offset_info.field_id =
        DBAL_FIELD_PP_PORT;

    return CMD_OK;
}

/**
* \brief
* compare the expected fields DB and the parsed fields DB
* compare all field's structure elements (field by field)
* called only if memory compare has failed
*****************************************************/
static cmd_result_t
diag_dbal_compare_field_data(
    int unit,
    dbal_field_basic_info_t * expected_field,
    dbal_field_basic_info_t * parsed_field)
{
    int ii;

    if (sal_strcmp(expected_field->name, parsed_field->name) != 0)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected name. Parsed:%s ,Expected:%s \n"),
                                   parsed_field->name, expected_field->name));
        return CMD_FAIL;
    }
    if (expected_field->max_size != parsed_field->max_size)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected max_size. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->max_size, expected_field->max_size));
        return CMD_FAIL;
    }
    for (ii = 0; ii < DBAL_MAX_NOF_ENTITY_LABEL_TYPES; ii++)
    {
        if (expected_field->labels[ii] != parsed_field->labels[ii])
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected labels[%d]. Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_field->labels[ii], expected_field->labels[ii]));
            return CMD_FAIL;
        }
    }
    if (expected_field->type != parsed_field->type)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected type. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->type, expected_field->type));
        return CMD_FAIL;
    }
    if (expected_field->instances_support != parsed_field->instances_support)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected instances_support. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->instances_support, expected_field->instances_support));
        return CMD_FAIL;
    }
    for (ii = 0; ii < DBAL_FIELD_MAX_PARENTS_PER_FIELD; ii++)
    {
        if (expected_field->parent_field_id[ii] != parsed_field->parent_field_id[ii])
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected parent_field_id[%d]. Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_field->parent_field_id[ii], expected_field->parent_field_id[ii]));
            return CMD_FAIL;
        }
    }
    if (expected_field->default_value != parsed_field->default_value)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected default_value. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->default_value, expected_field->default_value));
        return CMD_FAIL;
    }
    if (expected_field->is_default_value_valid != parsed_field->is_default_value_valid)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected is_default_value_valid. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->is_default_value_valid, expected_field->is_default_value_valid));
        return CMD_FAIL;
    }
    if (expected_field->encode_info.encode_mode != parsed_field->encode_info.encode_mode)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected encode_type. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->encode_info.encode_mode, expected_field->encode_info.encode_mode));
        return CMD_FAIL;
    }
    if (expected_field->encode_info.input_param != parsed_field->encode_info.input_param)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected value_input_param1. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->encode_info.input_param, expected_field->encode_info.input_param));
        return CMD_FAIL;
    }
    if (expected_field->nof_child_fields != parsed_field->nof_child_fields)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected nof_child_fields. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->nof_child_fields, expected_field->nof_child_fields));
        return CMD_FAIL;
    }
    if (expected_field->nof_enum_values != parsed_field->nof_enum_values)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected nof_enum_values. Parsed=%d ,Expected=%d \n"),
                                   parsed_field->nof_enum_values, expected_field->nof_enum_values));
        return CMD_FAIL;
    }

    for (ii = 0; ii < expected_field->nof_child_fields; ii++)
    {
        if (expected_field->sub_field_info[ii].sub_field_id != parsed_field->sub_field_info[ii].sub_field_id)
        {
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "Unexpected sub_field_info[%d], sub_field_id. Parsed=%d ,Expected=%d \n"), ii,
                       parsed_field->sub_field_info[ii].sub_field_id, expected_field->sub_field_info[ii].sub_field_id));
            return CMD_FAIL;
        }
        if (expected_field->sub_field_info[ii].encode_info.encode_mode !=
            parsed_field->sub_field_info[ii].encode_info.encode_mode)
        {
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "Unexpected sub_field_info[%d], encode_mode. Parsed=%d ,Expected=%d \n"), ii,
                       parsed_field->sub_field_info[ii].encode_info.encode_mode,
                       expected_field->sub_field_info[ii].encode_info.encode_mode));
            return CMD_FAIL;
        }
        if (expected_field->sub_field_info[ii].encode_info.input_param !=
            parsed_field->sub_field_info[ii].encode_info.input_param)
        {
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "Unexpected sub_field_info[%d], input_param. Parsed=%d ,Expected=%d \n"), ii,
                       parsed_field->sub_field_info[ii].encode_info.input_param,
                       expected_field->sub_field_info[ii].encode_info.input_param));
            return CMD_FAIL;
        }
    }
    for (ii = 0; ii < expected_field->nof_enum_values; ii++)
    {
        if (expected_field->enum_val_info[ii].value != parsed_field->enum_val_info[ii].value)
        {
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "Unexpected enum_decoding_values[%d]. Parsed=%d ,Expected=%d \n"), ii,
                       parsed_field->enum_val_info[ii].value, expected_field->enum_val_info[ii].value));
            return CMD_FAIL;
        }
        if (sal_strcmp(expected_field->enum_val_info[ii].name, parsed_field->enum_val_info[ii].name) != 0)
        {
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "Unexpected enum_decoding_names[%d]. Parsed=%s ,Expected=%s \n"), ii,
                       parsed_field->enum_val_info[ii].name, expected_field->enum_val_info[ii].name));
            return CMD_FAIL;
        }
    }
    return CMD_OK;
}

/**
* \brief
* compare the expected tables DB and the parsed tables DB
* compare all table's structure elements (field by field)
* called only if memory compare has failed
*****************************************************/
static cmd_result_t
diag_dbal_compare_table_data(
    int unit,
    dbal_logical_table_t * expected_table,
    dbal_logical_table_t * parsed_table)
{
    int ii, jj;

    if (sal_strcmp(expected_table->table_name, parsed_table->table_name) != 0)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected table_name. Parsed:%s ,Expected:%s \n"),
                                   parsed_table->table_name, expected_table->table_name));
        return CMD_FAIL;
    }
    if (expected_table->is_table_initiated != parsed_table->is_table_initiated)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected is_table_initiated. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->is_table_initiated, expected_table->is_table_initiated));
        return CMD_FAIL;
    }
    if (expected_table->is_table_valid != parsed_table->is_table_valid)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected is_table_valid. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->is_table_valid, expected_table->is_table_valid));
        return CMD_FAIL;
    }
    for (ii = 0; ii < DBAL_MAX_NOF_ENTITY_LABEL_TYPES; ii++)
    {
        if (expected_table->labels[ii] != parsed_table->labels[ii])
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected labels[%d]. Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_table->labels[ii], expected_table->labels[ii]));
            return CMD_FAIL;
        }
    }
    if (expected_table->maturity_level != parsed_table->maturity_level)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected maturity_level. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->maturity_level, expected_table->maturity_level));
        return CMD_FAIL;
    }
    if (expected_table->table_type != parsed_table->table_type)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected table_type. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->table_type, expected_table->table_type));
        return CMD_FAIL;
    }
    if (expected_table->nof_entries != parsed_table->nof_entries)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected nof_entries. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->nof_entries, expected_table->nof_entries));
        return CMD_FAIL;
    }
    if (expected_table->min_index != parsed_table->min_index)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected min_index. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->min_index, expected_table->min_index));
        return CMD_FAIL;
    }
    if (expected_table->max_capacity != parsed_table->max_capacity)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected max_capacity. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->max_capacity, expected_table->max_capacity));
        return CMD_FAIL;
    }
    if (expected_table->access_method != parsed_table->access_method)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected access_method. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->access_method, expected_table->access_method));
        return CMD_FAIL;
    }
    if (expected_table->core_mode != parsed_table->core_mode)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected core_mode. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->core_mode, expected_table->core_mode));
        return CMD_FAIL;
    }
    if (expected_table->physical_db_id != parsed_table->physical_db_id)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected physical_db_id. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->physical_db_id, expected_table->physical_db_id));
        return CMD_FAIL;
    }
    if (expected_table->app_id != parsed_table->app_id)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected app_id. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->app_id, expected_table->app_id));
        return CMD_FAIL;
    }
    if (expected_table->max_payload_size != parsed_table->max_payload_size)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected max_payload_size. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->max_payload_size, expected_table->max_payload_size));
        return CMD_FAIL;
    }
    if (expected_table->max_nof_result_fields != parsed_table->max_nof_result_fields)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected max_nof_result_fields. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->max_nof_result_fields, expected_table->max_nof_result_fields));
        return CMD_FAIL;
    }
    if (expected_table->sw_payload_length_bytes != parsed_table->sw_payload_length_bytes)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected sw_payload_length_bytes. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->sw_payload_length_bytes, expected_table->sw_payload_length_bytes));
        return CMD_FAIL;
    }
    if (expected_table->key_size != parsed_table->key_size)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected key_size. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->key_size, expected_table->key_size));
        return CMD_FAIL;
    }
    if (expected_table->nof_key_fields != parsed_table->nof_key_fields)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected nof_key_fields. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->nof_key_fields, expected_table->nof_key_fields));
        return CMD_FAIL;
    }
    for (ii = 0; ii < expected_table->nof_key_fields; ii++)
    {
        if (sal_memcmp(&expected_table->keys_info[ii], &parsed_table->keys_info[ii],
                       sizeof(dbal_table_field_info_t)) != 0)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected keys_info[%d].\n"), ii));
            return CMD_FAIL;
        }
    }
    if (expected_table->nof_result_types != parsed_table->nof_result_types)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected nof_result_types. Parsed=%d ,Expected=%d \n"),
                                   parsed_table->nof_result_types, expected_table->nof_result_types));
        return CMD_FAIL;
    }
    for (ii = 0; ii < expected_table->nof_result_types; ii++)
    {
        if (sal_strcmp(expected_table->multi_res_info[ii].result_type_name,
                       parsed_table->multi_res_info[ii].result_type_name) != 0)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected multi_res_info[%d].result_type_name. "
                                                  "Parsed=%s ,Expected=%s \n"),
                                       ii, parsed_table->multi_res_info[ii].result_type_name,
                                       expected_table->multi_res_info[ii].result_type_name));
            return CMD_FAIL;
        }
        if (expected_table->multi_res_info[ii].zero_padding != parsed_table->multi_res_info[ii].zero_padding)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected multi_res_info[%d].zero_padding. "
                                                  "Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_table->multi_res_info[ii].zero_padding,
                                       expected_table->multi_res_info[ii].zero_padding));
            return CMD_FAIL;
        }
        if (expected_table->multi_res_info[ii].sw_payload_length_bytes !=
            parsed_table->multi_res_info[ii].sw_payload_length_bytes)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected multi_res_info[%d] "
                                                  "sw_payload_length_bytes. Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_table->multi_res_info[ii].sw_payload_length_bytes,
                                       expected_table->multi_res_info[ii].sw_payload_length_bytes));
            return CMD_FAIL;
        }
        if (expected_table->multi_res_info[ii].result_type_hw_value !=
            parsed_table->multi_res_info[ii].result_type_hw_value)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected multi_res_info[%d] "
                                                  "result_type_hw_value. Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_table->multi_res_info[ii].result_type_hw_value,
                                       expected_table->multi_res_info[ii].result_type_hw_value));
            return CMD_FAIL;
        }
        if (expected_table->multi_res_info[ii].entry_payload_size !=
            parsed_table->multi_res_info[ii].entry_payload_size)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected multi_res_info[%d] "
                                                  "entry_payload_size. Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_table->multi_res_info[ii].entry_payload_size,
                                       expected_table->multi_res_info[ii].entry_payload_size));
            return CMD_FAIL;
        }
        if (expected_table->multi_res_info[ii].nof_result_fields != parsed_table->multi_res_info[ii].nof_result_fields)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected multi_res_info[%d] "
                                                  "nof_result_fields. Parsed=%d ,Expected=%d \n"),
                                       ii, parsed_table->multi_res_info[ii].nof_result_fields,
                                       expected_table->multi_res_info[ii].nof_result_fields));
            return CMD_FAIL;
        }
        for (jj = 0; jj < expected_table->multi_res_info[ii].nof_result_fields; jj++)
        {
            if (sal_memcmp(&expected_table->multi_res_info[ii].results_info[jj],
                           &parsed_table->multi_res_info[ii].results_info[jj], sizeof(dbal_table_field_info_t)) != 0)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Unexpected multi_res_info[%d]. "
                                                      "results_info[%d] \n"), ii, jj));
                return CMD_FAIL;
            }
        }
    }

    if (expected_table->access_method == DBAL_ACCESS_METHOD_HARD_LOGIC)
    {
        for (ii = 0; ii < expected_table->nof_result_types; ii++)
        {
            for (jj = 0; jj < DBAL_NOF_HL_ACCESS_TYPES; jj++)
            {
                int kk;
                dbal_direct_l2p_info_t *access_expected, *access_parsed;
                access_expected = &expected_table->hl_mapping_multi_res[ii].l2p_direct_info[jj];
                access_parsed = &parsed_table->hl_mapping_multi_res[ii].l2p_direct_info[jj];
                if (access_expected->num_of_access_fields != access_parsed->num_of_access_fields)
                {
                    LOG_ERROR(BSL_LOG_MODULE,
                              (BSL_META_U
                               (unit,
                                "Unexpected result type=%d  access type=%d, nof fields.Parsed=%d ,Expected=%d \n"), ii,
                               jj, access_parsed->num_of_access_fields, access_expected->num_of_access_fields));
                    return CMD_FAIL;
                }
                for (kk = 0; kk < access_expected->num_of_access_fields; kk++)
                {
                    if (sal_memcmp(&access_expected->l2p_fields_info[kk], &access_parsed->l2p_fields_info[kk],
                                   sizeof(dbal_direct_l2p_info_t)) != 0)
                    {
                        LOG_ERROR(BSL_LOG_MODULE,
                                  (BSL_META_U(unit, "Unexpected HL result type=%d access type %d access num %d \n"), ii,
                                   jj, kk));
                        return CMD_FAIL;
                    }
                }
            }
        }
    }
    return CMD_OK;
}

/**
* \brief
* Run XML parsing validation 
* 1. immitate the init procedue (with validation flag) 
* 2. compare to the expected values 
*****************************************************/
cmd_result_t
diag_dbal_test_xml_parsing(
    int unit)
{
    int ii;
    int res = CMD_OK;

    /*
     * Fields - setting hard-coded values
     */
    res = dbal_db_init_fields_set_default(unit, fields_data);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_init_fields_set_default res=%d\n"), res));
        return res;
    }

    res = diag_dbal_set_expected_fields_data(fields_data);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_set_expected_fields_data, res=%d\n"), res));
        return res;
    }

    /*
     * Fields reading
     */
    res = dbal_db_init_fields_set_default(unit, fields_parsed_info);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_init_fields_set_default, res=%d\n"), res));
        return res;
    }

    res = dbal_db_init_fields(unit, DBAL_INIT_FLAGS_VALIDATION, fields_parsed_info);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_init_fields, res=%d\n"), res));
        return res;
    }

    /*
     * Fields comparison
     */
    for (ii = 0; ii < DBAL_NOF_FIELDS; ii++)
    {
        if (sal_memcmp(&fields_data[ii], &fields_parsed_info[ii], sizeof(dbal_field_basic_info_t)) != 0)
        {
            res = diag_dbal_compare_field_data(unit, &fields_data[ii], &fields_parsed_info[ii]);
            if (res != CMD_OK)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_compare_field_data, field=%s\n"),
                                           dbal_field_to_string(unit, ii)));
                return res;
            }
        }
    }
    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "XML Parsing Validation: Fields - PASSED\n")));

    /*
     * Tables - setting hard-coded values
     */
    res = dbal_db_init_tables_set_default(unit, tables_data.logical_tables);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_init_logical_tables_set_default, res=%d\n"), res));
        return res;
    }

    res = diag_dbal_set_expected_tables_data(tables_data.logical_tables);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_set_expected_tables_data, res=%d\n"), res));
        return res;
    }

    /*
     * Tables reading
     */
    res = dbal_db_init_tables_set_default(unit, tables_parsed_info.logical_tables);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_init_logical_tables_set_default, res=%d\n"), res));
        return res;
    }

    res = dbal_db_init_logical_tables(unit, DBAL_INIT_FLAGS_VALIDATION,
                                      tables_parsed_info.logical_tables, DBAL_ACCESS_METHOD_PHY_TABLE);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_init_mdb_logical_tables, res=%d\n"), res));
        return res;
    }

    res = dbal_db_init_logical_tables(unit, DBAL_INIT_FLAGS_VALIDATION, tables_parsed_info.logical_tables,
                                      DBAL_ACCESS_METHOD_HARD_LOGIC);
    if (res != CMD_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "dbal_db_init_hard_logic_logical_tables, res=%d\n"), res));
        return res;
    }

    /*
     * Tables comparison
     */
    for (ii = 0; ii < DBAL_NOF_TABLES; ii++)
    {
        if (sal_memcmp(&tables_data.logical_tables[ii], &tables_parsed_info.logical_tables[ii],
                       sizeof(dbal_logical_table_t)) != 0)
        {
            res = diag_dbal_compare_table_data(unit, &tables_data.logical_tables[ii],
                                               &tables_parsed_info.logical_tables[ii]);
            if (res != CMD_OK)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Error. dbal_db_compare_table_data, table=%s\n"),
                                           dbal_logical_table_to_string(unit, ii)));
                return res;
            }
        }
    }
    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "XML Parsing Validation: Tables - PASSED\n")));

    return res;
}
