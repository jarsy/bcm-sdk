/*
 * $Id: diag_sand_access.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        diag_sand_access.c
 * Purpose:     Diag shell direct access commands
 */

#include <shared/bsl.h>
#include <shared/utilex/utilex_framework.h>

#include <sal/appl/sal.h>
#include <soc/drv.h>
#include <soc/shared/access_pack.h>

#include <appl/diag/system.h>
#include <appl/diag/sand/diag_sand_reg.h>
#include <appl/diag/sand/diag_sand_prt.h>
#include <appl/diag/sand/diag_sand_framework.h>
#include <appl/diag/sand/diag_sand_access.h>

#ifdef BCM_DPP_SUPPORT
#include <soc/dpp/port_sw_db.h>
#endif

#ifdef BCM_IPROC_SUPPORT
#include <soc/iproc.h>
#endif

#define BSL_LOG_MODULE BSL_LS_APPL_SHELL

static shr_error_e
access_reg_read(
    int         unit,
    soc_reg_t   reg,
    soc_block_t block_id,
    int         port,
    int         reg_index,
    uint32      *value,
    uint32      *addr_p)
{
    int blk;
    int ret;
    uint8 acc_type;
    uint32 addr = 0;
    soc_reg_info_t * reginfo = &SOC_REG_INFO(unit, reg);

    SHR_FUNC_INIT_VARS(NO_UNIT);

    memset(value, 0, sizeof(uint32) * SOC_REG_ABOVE_64_MAX_SIZE_U32);

    switch(reginfo->regtype)
    {
        case soc_cpureg:
            addr = soc_reg_addr_get(unit, reg, block_id, reg_index, SOC_REG_ADDR_OPTION_NONE, &blk, &acc_type);
            *value = soc_pci_read(unit, addr);
            break;
#ifdef BCM_IPROC_SUPPORT
        case soc_iprocreg:
            block_id = CMIC_BLOCK(unit);
            addr = soc_reg_addr_get(unit, reg, block_id, reg_index, SOC_REG_ADDR_OPTION_NONE, &blk, &acc_type);
            soc_iproc_getreg(unit, addr, value);
            break;
#endif /* BCM_IPROC_SUPPORT */
#ifdef BCM_CMICM_SUPPORT
        case soc_mcsreg:
            blk = CMIC_BLOCK(unit);
            addr = soc_reg_addr_get(unit, reg, block_id, reg_index, SOC_REG_ADDR_OPTION_NONE, &blk, &acc_type);
            *value = soc_pci_mcs_read(unit, addr);
            break;
#endif /* BCM_CMICM_SUPPORT */
        case soc_genreg:
            addr = soc_reg_addr_get(unit, reg, SOC_BLOCK_PORT(unit, block_id), reg_index, SOC_REG_ADDR_OPTION_NONE, &blk, &acc_type);
            if ((ret = soc_reg_above_64_get(unit, reg, SOC_BLOCK_PORT(unit, block_id), reg_index, value)) < 0)
            {
                cli_out("ERROR: read from register %s(%d).%s failed: %s\n",
                        SOC_REG_NAME(unit, reg), reg_index, SOC_BLOCK_NAME(unit, block_id), soc_errmsg(ret));
                goto exit;
            }
            break;
        case soc_portreg:
            if((ret = soc_reg_above_64_get(unit, reg, port, reg_index, value)) < 0)
            {
                cli_out("ERROR: read from register %s failed: %s\n", SOC_REG_NAME(unit, reg), soc_errmsg(ret));
                goto exit;
            }
            break;
        case soc_customreg:
            if((ret = soc_custom_reg_above_64_get(unit, reg, port, reg_index, value)) < 0)
            {
                cli_out("ERROR: read from register %s failed: %s\n", SOC_REG_NAME(unit, reg), soc_errmsg(ret));
                goto exit;
            }
            break;
        default:
            goto exit;
            break;
    }

    if(addr_p != NULL)
    {
        *addr_p = addr;
    }
exit:
    SHR_FUNC_EXIT;
}

static int
access_reg_filter_no_read_fields(int unit, soc_reg_t reg)
{
    int i;
    uint32 disallowed_fields[] = { INDIRECT_COMMAND_COUNTf, INDIRECT_COMMAND_WR_DATAf, INTERRUPT_REGISTER_TESTf,
        ECC_INTERRUPT_REGISTER_TESTf, NUM_SOC_FIELD};

    for (i = 0; disallowed_fields[i] != NUM_SOC_FIELD; i++)
    {
        if (SOC_REG_FIELD_VALID(unit,reg,disallowed_fields[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

static shr_error_e
access_reg_properties_get(
    int unit,
    soc_reg_t reg,
    char *prop_str,
    void *property_h)
{
    shr_error_e rv;
    uint32 flags;

    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, reg);

    flags = reginfo->flags;

    sal_memset(prop_str, 0, PRT_COLUMN_WIDTH_BIG);
    sal_strcpy(prop_str + sal_strlen(prop_str), "reg:");
    sal_sprintf(prop_str + sal_strlen(prop_str), " %s", soc_regtypenames[reginfo->regtype]);
    if (flags & SOC_REG_FLAG_ARRAY)
    {
        sal_sprintf(prop_str + sal_strlen(prop_str), " array[%d]", reginfo->numels);
    }
    if (flags & SOC_REG_FLAG_RO)
    {
        sal_strcat(prop_str, " ro");
    }
    if (flags & SOC_REG_FLAG_WO)
    {
        sal_strcat(prop_str, " wo");
    }
    if (flags & SOC_REG_FLAG_COUNTER)
    {
        sal_strcat(prop_str, " counter");
    }
    if (flags & SOC_REG_FLAG_ED_CNTR)
    {
        sal_strcat(prop_str, " err/discard");
    }
    if(shr_access_reg_no_read_get(unit, reg))
    {
        sal_strcat(prop_str, " no_read");
    }
    else
    { /* Registers with "no_read" are automatically no_wb, no need to mention it implicitly */
        if(shr_access_reg_no_wb_get(unit, reg))
        {
            sal_strcat(prop_str, " no_wb");
        }
    }
    if ((property_h != NULL) && (diag_sand_compare(property_h, prop_str) == FALSE))
        rv = _SHR_E_NOT_FOUND;
    else
        rv = _SHR_E_NONE;
    return rv;
}

static shr_error_e
access_mem_properties_get(
    int unit,
    soc_mem_t mem,
    char *prop_str,
    void *property_h)
{
    shr_error_e rv;
    uint32 flags;
    soc_mem_info_t *mem_info = &SOC_MEM_INFO(unit, mem);

    flags = mem_info->flags;

    sal_memset(prop_str, 0, PRT_COLUMN_WIDTH_BIG);

    sal_sprintf(prop_str + sal_strlen(prop_str), "mem:[%d]", soc_mem_index_max(unit, mem) + 1);

    if (flags & SOC_MEM_FLAG_IS_ARRAY)
    {
        sal_strcat(prop_str, " array");
        if(SOC_MEM_ARRAY_INFOP(unit, mem) != NULL)
            sal_sprintf(prop_str + sal_strlen(prop_str), "[%d]", SOC_MEM_NUMELS(unit, mem));
    }
    if (flags & SOC_MEM_FLAG_READONLY)
    {
        sal_strcat(prop_str, " ro");
    }
    if (flags & SOC_MEM_FLAG_WRITEONLY)
    {
        sal_strcat(prop_str, " wo");
    }
    if (flags & SOC_MEM_FLAG_SIGNAL)
    {
        sal_strcat(prop_str, " signal");
    }
    if (flags & SOC_MEM_FLAG_DEBUG)
    {
        sal_strcat(prop_str, " debug");
    }
    if (flags & SOC_MEM_FLAG_CACHABLE)
    {
        sal_strcat(prop_str, " cachable");
    }
    if (flags & SOC_MEM_FLAG_HASHED)
    {
        sal_strcat(prop_str, " hashed");
    }
    if(shr_access_mem_no_read_get(unit, mem))
    {
        sal_strcat(prop_str, " no_read");
    }
    else
    { /* Registers with "no_read" are automatically no_wb, no need to mention it implicitly */
        if(shr_access_mem_no_wb_get(unit, mem))
        {
            sal_strcat(prop_str, " no_wb");
        }
    }

    if ((property_h != NULL) && (diag_sand_compare(property_h, prop_str) == FALSE))
        rv = _SHR_E_NOT_FOUND;
    else
        rv = _SHR_E_NONE;
    return rv;
}

static shr_error_e
access_reg_blocks_get(
    int unit,
    soc_reg_info_t * reginfo,
    char *block_str,
    char *match_n)
{
    shr_error_e rv;
    int i_bl;

    if (!ISEMPTY(match_n))
        rv = _SHR_E_NOT_FOUND;
    else
        rv = _SHR_E_NONE;

    sal_memset(block_str, 0, PRT_COLUMN_WIDTH_BIG);
    for (i_bl = 0; SOC_BLOCK_INFO(unit, i_bl).type >= 0; i_bl++)
    {
        if (!SOC_BLOCK_IS_TYPE(unit, i_bl, reginfo->block))
            continue;
        if (sal_strlen(block_str) != 0)
            sal_strcat(block_str, ",");

        sal_sprintf(block_str + sal_strlen(block_str), "%s", SOC_BLOCK_NAME(unit, i_bl));

        if (!SOC_INFO(unit).block_valid[i_bl])
            sal_strcat(block_str, "(dis)");
        if (!ISEMPTY(match_n) && sal_strcasestr(SOC_BLOCK_NAME(unit, i_bl), match_n))
            rv = _SHR_E_NONE;
    }
    return rv;
}

static shr_error_e
access_mem_blocks_get(
    int unit,
    soc_mem_t mem,
    char *block_str,
    char *match_n)
{
    shr_error_e rv;
    unsigned int i_bl;

    if (!ISEMPTY(match_n))
        rv = _SHR_E_NOT_FOUND;
    else
        rv = _SHR_E_NONE;

    sal_memset(block_str, 0, PRT_COLUMN_WIDTH_BIG);
    SOC_MEM_BLOCK_ITER(unit, mem, i_bl)
    {
        if (sal_strlen(block_str) != 0)
            sal_strcat(block_str, ",");

        sprintf(block_str + sal_strlen(block_str), "%s", SOC_BLOCK_NAME(unit, i_bl));

        if (!SOC_INFO(unit).block_valid[i_bl])
            sal_strcat(block_str, "(dis)");
        if (!ISEMPTY(match_n) && sal_strcasestr(SOC_BLOCK_NAME(unit, i_bl), match_n))
            rv = _SHR_E_NONE;
    }

    return rv;
}

static shr_error_e
access_obj_match(
        int dual_match,
        char *obj_name,
        soc_field_info_t * fields,
        int nFields,
        void *match_h,
        void *field_match_h,
        int *fld_match_list)
{
    shr_error_e rv = _SHR_E_NOT_FOUND;
    int i_fld;
    int reg_match, field_match = FALSE;

    reg_match = diag_sand_compare(match_h, obj_name);

    if((dual_match == FALSE) && (reg_match == TRUE))
    {
        for (i_fld = 0; i_fld < nFields; i_fld++)
        {
            fld_match_list[i_fld] = TRUE;
        }
        rv = _SHR_E_NONE;
    }
    else
    {
        for (i_fld = 0; i_fld < nFields; i_fld++)
        {
            if (diag_sand_compare(field_match_h, SOC_FIELD_NAME(unit, fields[i_fld].field)) == TRUE)
            {
                field_match = TRUE;
                fld_match_list[i_fld] = TRUE;
            }
            else
            {
                fld_match_list[i_fld] = FALSE;
            }
        }

        if(dual_match == TRUE)
        {
            if((reg_match == TRUE) && (field_match == TRUE))
            {
                rv = _SHR_E_NONE;
            }
        }
        else if(field_match == TRUE)
        {
            rv = _SHR_E_NONE;
        }
    }

    return rv;
}

static int
access_fields_get_ordered_list(
    soc_field_info_t * fields,
    int nFields,
    int *fld_index_l)
{
    int fldID = 0;
    int min_bit = 0;
    int i_fld;
    int fld_status[MAX_FIELDS_NUM];

    if ((fields == NULL) || (nFields == 0))
    {
        cli_out("No fields\n");
        return 0;
    }

    if (nFields > MAX_FIELDS_NUM)
    {
        cli_out("Number of field:%d exceeded maximum:%d\n", nFields, MAX_FIELDS_NUM);
        return 0;
    }

    for (i_fld = 0; i_fld < nFields; i_fld++)
        fld_status[i_fld] = FALSE;

    while (nFields != fldID)
    {
        int curID = -1;
        int fld_min_bit, cur_min_bit = UTILEX_I32_MAX;
        for (i_fld = 0; i_fld < nFields; i_fld++)
        {
            fld_min_bit = fields[i_fld].bp;

            /*
             * Check if we don't have gaps in bits i.e. next min bit equal to previous max + 1 
             */
            if ((fld_status[i_fld] == FALSE) && (fld_min_bit >= min_bit) && (fld_min_bit < cur_min_bit))
            {
                curID = i_fld;
                cur_min_bit = fld_min_bit;
            }
        }
        if ((cur_min_bit == UTILEX_I32_MAX) || (curID == -1))
        {
            cli_out("Inconsistent next field for:%d\n", fldID);
            return 0;
        }
        else
        {
            fld_index_l[fldID++] = curID;
            fld_status[curID] = TRUE;
            min_bit = cur_min_bit;
        }
    }
    /*
     * Next minimal bit is here actual the size of the entire entry 
     */
    return fields[fld_index_l[nFields - 1]].bp + fields[fld_index_l[nFields - 1]].len;
}

static int
access_fields_match_parse(
    char *match,
    char **field_match_p)
{
    int i_str;

    for(i_str = 0; i_str < sal_strlen(match); i_str++)
    {
        if(match[i_str] == '.')
        {
            match[i_str]    = 0;
            *field_match_p  = match + i_str + 1;
            return TRUE;
        }
    }

    *field_match_p  = match;
    return FALSE;
}

static sh_sand_man_t access_block_man = {
    "Lists blocks of device",
    "Block names allow subsequently to filter registers/memories per block",
    "ACCess blocks [name=<string>]\n",
    "acc blocks\n"
    "acc blocks name=SCH",
};

static sh_sand_option_t access_block_options[] = {
    {"name", SAL_FIELD_TYPE_STR, "Full block name or its substring, filtering blocks to be listed", ""},
    {NULL}
};

static shr_error_e
access_block_cmd(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    char *match_n;
    int i_bl;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(NO_UNIT);

    SH_SAND_GET_STR("name", match_n);

    PRT_TITLE_SET("BLOCK LIST");
    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADD("Block");
    PRT_COLUMN_ADD("ID");
    PRT_COLUMN_ADD("Type");
    PRT_COLUMN_ADD("Number");
    PRT_COLUMN_ADD("State");

    for (i_bl = 0; SOC_BLOCK_TYPE(unit, i_bl) >= 0; i_bl++)
    {
        if ((match_n) && sal_strcasestr(SOC_BLOCK_NAME(unit, i_bl), match_n) == NULL)
            continue;

        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%d", i_bl);
        PRT_CELL_SET("%s", SOC_BLOCK_NAME(unit, i_bl));
        PRT_CELL_SET("%d", SOC_BLOCK_TYPE(unit, i_bl));
        PRT_CELL_SET("%d", SOC_BLOCK_NUMBER(unit, i_bl));
        if (!SOC_INFO(unit).block_valid[i_bl])
            PRT_CELL_SET("disabled");
    }

    PRT_COMMITX;
exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

static sh_sand_man_t access_property_man = {
    "Lists register number per property",
    "Command scans all registers and prints the number per each priperty found",
    "ACCess property\n",
    "acc prp\n",
};

static sh_sand_option_t access_property_options[] = {
    {NULL}
};

static shr_error_e
access_property_cmd(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    int i_type;
    soc_reg_t reg;
    int reg_num[soc_invalidreg];
    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(NO_UNIT);

    memset(reg_num, 0, sizeof(int) * soc_invalidreg);
    PRT_TITLE_SET("REGISTER TYPES LIST");
    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADD("Type");
    PRT_COLUMN_ADD("Number");

    for (reg = 0; reg < NUM_SOC_REG; reg++)
    {
        if (!SOC_REG_IS_VALID(unit, reg))
            continue;

        reg_num[SOC_REG_INFO(unit, reg).regtype]++;
    }

    for(i_type = 0; i_type < soc_invalidreg; i_type++)
    {
        if(reg_num[i_type] == 0)
            continue;

        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", soc_regtypenames[i_type]);
        PRT_CELL_SET("%d", reg_num[i_type]);
    }

    PRT_COMMITX;
exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

static sh_sand_man_t access_list_man = {
    "Show registers/memories lists per filtering criteria",
    "Command allows to display or to save in XML file list of registers&memories filtered by one or more filters\n"
    "Entire list is very long and by default command without any option will return usage. If you want to have all, use"
    " \"all\" option."
    "Output includes register/memory name, property list, block list, reset value, size, address, optionally field list"
    "and description. Beware that sometimes output line exceeds screen width. In these case try either to increase your "
    "display size or apply more rigor filtering to decrease output volumes",
    "ACCess list [name=<regular expr>] [block=<substring>] [fields] [all] [desc] [property=<regular expr>]",
    "acc list name=RIP block=SCH\n"
    "acc list property=mem.*cachable block=PPDB_B0",
};

static sh_sand_option_t access_list_options[] = {
    {"name",        SAL_FIELD_TYPE_STR,  "Regular expression used to filter regs/mems and their fields", ""},
    {"block",       SAL_FIELD_TYPE_STR,  "Block name or any substring of it to filter regs/mems",        ""},
    {"all",         SAL_FIELD_TYPE_BOOL, "Ignore filters and print all regs/mems",                       "No"},
    {"field",      SAL_FIELD_TYPE_BOOL, "Include fields in regs/mems search and output",                "Yes"},
    {"desc",        SAL_FIELD_TYPE_BOOL, "Add description column in output",                             "No"},
    {"property",    SAL_FIELD_TYPE_STR,  "Name or name substring for regs/mems properties",              ""},
    {NULL}
};

static shr_error_e
access_list_cmd(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    char *block_n, *match_n, *property_n, *field_match_n;
    int fld_index_list[MAX_FIELDS_NUM];
    int fld_match_list[MAX_FIELDS_NUM];
    soc_reg_t reg;
    soc_mem_t mem;

    int desc_flag, field_flag, all_flag;

    int i_fld;
    soc_field_info_t *fld;
    soc_reg_above_64_val_t fld_value;
    int dual_match;
    void *match_h = NULL, *property_h = NULL, *field_match_h = NULL;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(NO_UNIT);
    /*
     * Get parameters 
     */
    SH_SAND_GET_STR("name", match_n);
    SH_SAND_GET_STR("block", block_n);
    SH_SAND_GET_STR("property", property_n);
    SH_SAND_GET_BOOL("field", field_flag);
    SH_SAND_GET_BOOL("desc", desc_flag);
    SH_SAND_GET_BOOL("all", all_flag);

    if(all_flag == TRUE)
    {
        match_n = block_n = property_n = "";
    }
    else
    {
        if(ISEMPTY(match_n) && ISEMPTY(block_n) && ISEMPTY(property_n))
        {
            SHR_CLI_EXIT(_SHR_E_PARAM, "Use \"all\" option to see the entire list or use filtering option\n");
        }
    }

    dual_match = access_fields_match_parse(match_n, &field_match_n);
    if((dual_match == TRUE) && (field_flag == FALSE))
    {
        SHR_CLI_EXIT(_SHR_E_PARAM, "Dotted format cannot be used with field option disabled\n");
    }
    SHR_IF_ERR_EXIT(diag_sand_compare_init(match_n,       &match_h));
    SHR_IF_ERR_EXIT(diag_sand_compare_init(field_match_n, &field_match_h));
    SHR_IF_ERR_EXIT(diag_sand_compare_init(property_n,    &property_h));

    PRT_TITLE_SET("ACCESS LIST");
    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADD("Object");
    PRT_COLUMN_ADD_FLEX(PRT_FLEX_ASCII, "Properties");
    PRT_COLUMN_ADD_FLEX(PRT_FLEX_ASCII, "Blocks");
    PRT_COLUMN_ADD("Bits");
    PRT_COLUMN_ADD("Reset");
    PRT_COLUMN_ADD("Address");

    if (desc_flag == TRUE)
    {
        PRT_COLUMN_ADD_FLEX(PRT_FLEX_ASCII, "Description");
    }

    for (reg = 0; reg < NUM_SOC_REG; reg++)
    {
        soc_reg_info_t *reginfo;
        int reg_size;
        uint32 reg_addr;
        soc_reg_above_64_val_t reg_value;
        char value_str[PRT_COLUMN_WIDTH_BIG];
        char prop_str[PRT_COLUMN_WIDTH_BIG];
        char block_str[PRT_COLUMN_WIDTH_BIG];

        if (!SOC_REG_IS_VALID(unit, reg))
            continue;

        reginfo = &SOC_REG_INFO(unit, reg);
        /* Filter per register and field names */
        if(access_obj_match(dual_match, SOC_REG_NAME(unit, reg), reginfo->fields, reginfo->nFields,
                                                              match_h, field_match_h, fld_match_list) != _SHR_E_NONE)
            continue;

        reg_addr = SOC_REG_BASE(unit, reg);
        /*
         * Fill properties 
         */
        if (access_reg_properties_get(unit, reg, prop_str, property_h) != _SHR_E_NONE)
            continue;
        /*
         * Fill blocks 
         */
        if (access_reg_blocks_get(unit, reginfo, block_str, block_n) != _SHR_E_NONE)
            continue;

        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", SOC_REG_NAME(unit, reg));
        PRT_CELL_SET("%s", prop_str);
        PRT_CELL_SET("%s", block_str);
        if ((reg_size = access_fields_get_ordered_list(reginfo->fields, reginfo->nFields, fld_index_list)) == 0)
        {
            cli_out("Problem with fields for register:%s\n", SOC_REG_NAME(unit, reg));
            continue;
        }

        PRT_CELL_SET("%02d:%02d", reginfo->fields[fld_index_list[0]].bp, reg_size - 1); /* Bits */
        SOC_REG_ABOVE_64_RST_VAL_GET(unit, reg, reg_value);
        diag_sand_value_to_str(reg_value, reg_size, value_str, PRT_COLUMN_WIDTH_BIG);
        PRT_CELL_SET("%s", value_str);  /* Reset */
        PRT_CELL_SET("%08x", reg_addr); /* Address */
        if (desc_flag == TRUE)
        {
            PRT_CELL_SET("%s", SOC_REG_DESC(unit, reg));
        }

        if(field_flag == TRUE)
        {
            for (i_fld = 0; i_fld < reginfo->nFields; i_fld++)
            {
                if(fld_match_list[fld_index_list[i_fld]] == FALSE)
                    continue;

                fld = &reginfo->fields[fld_index_list[i_fld]];
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET_SHIFT(1, "%s", SOC_FIELD_NAME(unit, fld->field));      /* Object */
                if (fld->flags & SOCF_RO)
                {
                    PRT_CELL_SET_SHIFT(1, "ro");    /* Properties */
                }
                else if (fld->flags & SOCF_WO)
                {
                    PRT_CELL_SET_SHIFT(1, "wo");    /* Properties */
                }
                else
                {
                    PRT_CELL_SET_SHIFT(1, "rw");    /* Properties */
                }
                PRT_CELL_SKIP(1);   /* Blocks */
                PRT_CELL_SET_SHIFT(1, "%02d:%02d", fld->bp, fld->bp + fld->len - 1);        /* Bits */

                SOC_REG_ABOVE_64_CLEAR(fld_value);
                SHR_BITCOPY_RANGE(fld_value, 0, reg_value, fld->bp, fld->len);
                diag_sand_value_to_str(fld_value, fld->len, value_str, PRT_COLUMN_WIDTH_BIG);
                PRT_CELL_SET_SHIFT(1, "%s", value_str);     /* Reset */
            }
        }
    }

    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        soc_mem_info_t *meminfo;
        int mem_size;
        uint32 mem_addr;
        uint8 acc_type;
        char prop_str[PRT_COLUMN_WIDTH_BIG];
        /* coverity[stack_use_overflow : FALSE] */
        char block_str[PRT_COLUMN_WIDTH_BIG];

        if (!SOC_MEM_IS_VALID(unit, mem))
            continue;

        meminfo = &SOC_MEM_INFO(unit, mem);
        /* Init to print all fields */
        for (i_fld = 0; i_fld < meminfo->nFields; i_fld++)
        {
            fld_match_list[i_fld] = TRUE;
        }

        /* Filter per memory and field names */
        if(access_obj_match(dual_match, SOC_MEM_NAME(unit, mem), meminfo->fields, meminfo->nFields,
                                                                    match_h, field_match_h, fld_match_list) != _SHR_E_NONE)
            continue;

        mem_addr = soc_mem_addr_get(unit, mem, 0, SOC_MEM_BLOCK_ANY(unit, mem), 0, &acc_type);
        /*
         * Fill properties 
         */
        if (access_mem_properties_get(unit, mem, prop_str, property_h) != _SHR_E_NONE)
            continue;
        /*
         * Fill blocks 
         */
        if (access_mem_blocks_get(unit, mem, block_str, block_n) != _SHR_E_NONE)
            continue;

        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", SOC_MEM_NAME(unit, mem));
        PRT_CELL_SET("%s", prop_str);
        PRT_CELL_SET("%s", block_str);

        if ((mem_size = access_fields_get_ordered_list(meminfo->fields, meminfo->nFields, fld_index_list)) == 0)
        {
            cli_out("Problem with field for memory:%s\n", SOC_MEM_NAME(unit, mem));
            continue;
        }

        PRT_CELL_SET("%02d:%02d", meminfo->fields[fld_index_list[0]].bp, mem_size - 1); /* Bits */

        PRT_CELL_SKIP(1);       /* Reset */
        PRT_CELL_SET("%08x", mem_addr); /* Address */
        if (desc_flag == TRUE)
        {
            PRT_CELL_SET("%s", SOC_MEM_DESC(unit, mem));
        }

        if (field_flag == TRUE)
        {
            for (i_fld = 0; i_fld < meminfo->nFields; i_fld++)
            {
                if(fld_match_list[fld_index_list[i_fld]] == FALSE)
                    continue;

                fld = &meminfo->fields[fld_index_list[i_fld]];
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET_SHIFT(1, "%s", SOC_FIELD_NAME(unit, fld->field));
                PRT_CELL_SKIP(2);   /* Properties/Blocks */
                PRT_CELL_SET_SHIFT(1, "%02d:%02d", fld->bp, fld->bp + fld->len - 1);
            }
        }
    }

    PRT_COMMITX;
exit:
    diag_sand_compare_close(match_h);
    diag_sand_compare_close(field_match_h);
    diag_sand_compare_close(property_h);
    PRT_FREE;
    SHR_FUNC_EXIT;
}
static sh_sand_enum_t sand_test_enum_table[] = {
    {"New", 1},
    {"Old", 2},
    {"All", 3},
    {"Global", 4},
    {NULL}
};

static sh_sand_man_t access_test_man = {
    "Test input options",
    "Print values for all options, each option has different type. Try to assign value to see how it will be accepted",
    "ACCess test [option=<value>] ... \n",
    "acc test ip4=10.0.0.1 bool=yes\n"
    "acc test id=0x200 mac=00:11:22:33:44:55",
};

static sh_sand_option_t access_test_options[] = {
    {"name", SAL_FIELD_TYPE_STR,     "Print the string used as input for this option",     ""},
    {"id",   SAL_FIELD_TYPE_UINT32,  "Print uint32 value used as input for this option",   "1"},
    {"bool", SAL_FIELD_TYPE_BOOL,    "Print boolean value used as input for this option",  "no"},
    {"ip4",  SAL_FIELD_TYPE_IP4,     "Print input or default ipv4 address",                "1.2.3.4"},
    {"ip6",  SAL_FIELD_TYPE_IP6,     "Print input or default ipv6 address",                "abcd:02:03:04:ef01:06:07:08"},
    {"mac",  SAL_FIELD_TYPE_MAC,     "Print input or default mac address",                 "07:00:00:00:00:08"},
    {"dump", SAL_FIELD_TYPE_ARRAY32, "Print value with arbitrary size as array of uint32", "0"},
    {"enum", SAL_FIELD_TYPE_ENUM,    "Print input or default enum",                        "global", (void *)sand_test_enum_table},
    {NULL}
};

static shr_error_e
access_test_option(
        int unit,
        char *keyword,
        sal_field_type_e *type_p,
        uint32 *id_p,
        void **ext_ptr_p)
{
    if(!sal_strcasecmp(keyword, "test"))
    {
        if(type_p != NULL)
            *type_p = SAL_FIELD_TYPE_ENUM;
        if(id_p != NULL)
            *id_p = 1000;
        if(ext_ptr_p != NULL)
        {
            *ext_ptr_p = (void *)sand_test_enum_table;
        }
        return _SHR_E_NONE;
    }
    else
        return _SHR_E_NOT_FOUND;
}

static shr_error_e
access_test_cmd(
    int unit,
    args_t * args,
    sh_sand_control_t *sand_control)
{
    char *match_n, *filename;
    int boolean;
    int value_int32;
    uint32 value_uint32, value_test = 0xFFFF;
    int enum_index;
    sal_ip_addr_t ip_addr;
    sal_mac_addr_t mac_addr;
    sal_ip6_addr_t ip6_addr;
    sh_sand_arg_t *sand_arg;
    int test_is_present;
    uint32 *array_uint32;
    char buffer[SH_SAND_MAX_ARRAY32_SIZE * 8 + 4];

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(NO_UNIT);

    PRT_TITLE_SET("framework test");
    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADD("Name");
    PRT_COLUMN_ADD("Type");
    PRT_COLUMN_ADD("Value");

    SH_SAND_GET_STR("file", filename);
    SH_SAND_GET_BOOL("bool", boolean);
    SH_SAND_GET_INT32("core", value_int32);
    SH_SAND_GET_STR("name", match_n);
    SH_SAND_GET_UINT32("id", value_uint32);
    SH_SAND_GET_ARRAY32("dump", array_uint32);
    SH_SAND_GET_IP4("ip4", ip_addr);
    SH_SAND_GET_IP6("ip6", ip6_addr);
    SH_SAND_GET_MAC("mac", mac_addr);
    SH_SAND_GET_ENUM("enum", enum_index);
    SH_SAND_GET_ENUM_DYN("test", value_test, test_is_present);

    SH_SAND_GET_ITERATOR(sand_arg)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
        PRT_CELL_SET("%s", SH_SAND_GET_NAME(sand_arg));
        PRT_CELL_SET("%s", sal_field_type_str(SH_SAND_GET_TYPE(sand_arg)));
        PRT_CELL_SET("%d", SH_SAND_GET_VALUE(sand_arg));
    }

    if(test_is_present)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
        PRT_CELL_SET("test"); PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_ENUM));
                              PRT_CELL_SET("%d", value_test);
    }
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("file"); PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_STR));
                          PRT_CELL_SET("%s", filename);
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("bool"); PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_BOOL));
                          PRT_CELL_SET("%s", sh_sand_bool_str(boolean));
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("core"); PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_INT32));
                          PRT_CELL_SET("%d(0x%08x)", value_int32, value_int32);
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("id");   PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_UINT32));
                          PRT_CELL_SET("%d(0x%08x)", value_uint32, value_uint32);
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("array32");   PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_ARRAY32));
    format_long_integer(buffer, array_uint32, SH_SAND_MAX_ARRAY32_SIZE);
                          PRT_CELL_SET("%s", buffer);
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("name"); PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_STR));
                          PRT_CELL_SET("%s", match_n);
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("ip4");  PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_IP4));
                          PRT_CELL_SET("%d.%d.%d.%d", (ip_addr >> 24) & 0xff, (ip_addr >> 16) & 0xff,
                                                      (ip_addr >> 8) & 0xff, ip_addr & 0xff);
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("ip6");  PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_IP6));
                          PRT_CELL_SET("%x:%x:%x:%x:%x:%x:%x:%x",
                            (((uint16) ip6_addr[0] << 8) | ip6_addr[1]), (((uint16) ip6_addr[2] << 8) | ip6_addr[3]),
                            (((uint16) ip6_addr[4] << 8) | ip6_addr[5]), (((uint16) ip6_addr[6] << 8) | ip6_addr[7]),
                            (((uint16) ip6_addr[8] << 8) | ip6_addr[9]), (((uint16) ip6_addr[10] << 8) | ip6_addr[11]),
                            (((uint16) ip6_addr[12] << 8) | ip6_addr[13]), (((uint16) ip6_addr[14] << 8) | ip6_addr[15]));
    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("mac");  PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_MAC));
                          PRT_CELL_SET("%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3],
                                                      mac_addr[4], mac_addr[5]);

    PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
    PRT_CELL_SET("enum");  PRT_CELL_SET("%s", sal_field_type_str(SAL_FIELD_TYPE_ENUM));
                           PRT_CELL_SET("%d", enum_index);
    PRT_COMMITX;
exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

static sh_sand_man_t access_read_man = {
    "Read registers/memories lists per filtering criteria",
    "Command allows to display or to save in XML file value of registers&memories filtered by one or more filters\n"
    "Entire list is very long and by default command without any option will return usage. If you want to have all, use"
    "\"all\" option."
    "Output includes register/memory name, block name, array index for arrayed register, optionally fields list."
    " Beware that sometimes output line exceeds screen width. In these case try either to increase your "
    "display size or apply more rigor filtering to decrease output volumes",
    "ACCess read [name=<regular expr>] [block=<substring>] [field] [all] [property=<regular expr>]",
    "acc read name=RIP block=SCH\n"
    "acc read property=mem.*cachable block=PPDB_B0\n"
    "acc read name=PPDB_B_LIF_TABLE_TRILL id=0 count=1 index=0",
};

static sh_sand_option_t access_read_options[] = {
    {"name",        SAL_FIELD_TYPE_STR,  "Regular expression used to filter regs/mems and their fields", ""},
    {"block",       SAL_FIELD_TYPE_STR,  "Block name or any substring of it to filter regs/mems",        ""},
    {"all",         SAL_FIELD_TYPE_BOOL, "Ignore filters and print all regs/mems",                       "No"},
    {"field",       SAL_FIELD_TYPE_BOOL, "Include fields in regs/mems search and output",                "No"},
    {"property",    SAL_FIELD_TYPE_STR,  "Name or name substring for regs/mems properties/n"
                                         "<reg/mem/array/ro/wo regtype name:<cpu/general/iproc/custom/port>", ""},
    {"index",       SAL_FIELD_TYPE_INT32,"Memory Array Index",                                           "-1"},
    {"id",          SAL_FIELD_TYPE_INT32,"Memory Entry Number",                                          "-1"},
    {"count",       SAL_FIELD_TYPE_INT32,"number of entries",                                            "0"},
    {"cache",       SAL_FIELD_TYPE_BOOL, "flag used to obtain non-cached memory value",                  "Yes"},
    {"over",        SAL_FIELD_TYPE_BOOL, "flag used to overwrite auxilary no read",                      "No"},
    {"wb",          SAL_FIELD_TYPE_BOOL, "used to read only warmboot meaningfull objects",               "No"},
    {NULL}
};

static shr_error_e
access_reg_print(
        prt_control_t *prt_ctr,
        int         unit,
        soc_reg_t   reg,
        char        *block_name,
        int         reg_index,
        uint32      address,
        soc_reg_above_64_val_t reg_value,
        int         field_flag,
        int         *fld_match_list
        )
{
    int reg_size;
    int fld_index_list[MAX_FIELDS_NUM];
    char value_str[PRT_COLUMN_WIDTH_BIG];
    soc_reg_info_t * reginfo = &SOC_REG_INFO(unit, reg);

    SHR_FUNC_INIT_VARS(NO_UNIT);

    if ((reg_size = access_fields_get_ordered_list(reginfo->fields, reginfo->nFields, fld_index_list)) == 0)
    {
        SHR_CLI_EXIT(_SHR_E_INTERNAL, "Problem with fields for register:%s\n", SOC_REG_NAME(unit, reg));
    }

    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
    PRT_CELL_SET("%s", SOC_REG_NAME(unit, reg));
    PRT_CELL_SET("%s", block_name);
    PRT_CELL_SET("%d", reg_index);
    diag_sand_value_to_str(reg_value, reg_size, value_str, PRT_COLUMN_WIDTH_BIG);
    PRT_CELL_SET("0x%08x", address);
    PRT_CELL_SET("%s", value_str);
    if(field_flag == TRUE)
    {
        int i_fld;
        soc_field_info_t *fld;
        soc_reg_above_64_val_t fld_value;
        for (i_fld = 0; i_fld < reginfo->nFields; i_fld++)
        {
            if(fld_match_list[fld_index_list[i_fld]] == FALSE)
                continue;
            fld = &reginfo->fields[fld_index_list[i_fld]];
            PRT_ROW_ADD(PRT_ROW_SEP_NONE);
            PRT_CELL_SET_SHIFT(1, "%s", SOC_FIELD_NAME(unit, fld->field));      /* Object */
            PRT_CELL_SKIP(3);   /* Block&Array Index&Address */
            SOC_REG_ABOVE_64_CLEAR(fld_value);
            SHR_BITCOPY_RANGE(fld_value, 0, reg_value, fld->bp, fld->len);
            diag_sand_value_to_str(fld_value, fld->len, value_str, PRT_COLUMN_WIDTH_BIG);
            PRT_CELL_SET_SHIFT(1, "%s", value_str);     /* Reset */
        }
    }

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
access_read_cmd(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    char *block_n, *match_n, *property_n, *field_match_n;
    int fld_index_list[MAX_FIELDS_NUM];
    int fld_match_list[MAX_FIELDS_NUM];
    soc_reg_t reg;
    soc_mem_t mem;

    int field_flag, all_flag;
    int object_col_id;
    void *match_h = NULL, *property_h = NULL, *field_match_h = NULL;
    int index, entry, count, wb;
    uint32 address;
    int cache, over;
    int dual_match;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(NO_UNIT);
    /*
     * Get parameters
     */
    SH_SAND_GET_STR("name", match_n);
    SH_SAND_GET_STR("block", block_n);
    SH_SAND_GET_STR("property", property_n);
    SH_SAND_GET_BOOL("field", field_flag);
    SH_SAND_GET_BOOL("all", all_flag);
    SH_SAND_GET_BOOL("index", index);
    SH_SAND_GET_BOOL("id", entry);
    SH_SAND_GET_BOOL("count", count);
    SH_SAND_GET_BOOL("cache", cache);
    SH_SAND_GET_BOOL("over", over);
    SH_SAND_GET_BOOL("wb", wb);

    if(all_flag == TRUE)
    {
        match_n = block_n = property_n = "";
    }
    else
    {
        if(ISEMPTY(match_n) && ISEMPTY(block_n) && ISEMPTY(property_n))
        {
            SHR_CLI_EXIT(_SHR_E_PARAM, "Use \"all\" too see the entire list or use filtering option\n");
        }
    }

    dual_match = access_fields_match_parse(match_n, &field_match_n);
    if(dual_match == TRUE)
    {
        field_flag = TRUE;
    }

    SHR_IF_ERR_EXIT(diag_sand_compare_init(match_n,       &match_h));
    SHR_IF_ERR_EXIT(diag_sand_compare_init(field_match_n, &field_match_h));
    SHR_IF_ERR_EXIT(diag_sand_compare_init(property_n,    &property_h));

    PRT_TITLE_SET("access read");
    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADDX(PRT_XML_CHILD,    PRT_TITLE_ID,  &object_col_id, "Object");
    PRT_COLUMN_ADDX(PRT_XML_ATTRIBUTE, object_col_id, NULL, "Block");
    PRT_COLUMN_ADDX(PRT_XML_ATTRIBUTE, object_col_id, NULL, "Index");
    PRT_COLUMN_ADDX(PRT_XML_ATTRIBUTE, object_col_id, NULL, "Address");
    PRT_COLUMN_ADDX(PRT_XML_ATTRIBUTE, object_col_id, NULL, "Value");

    for (reg = 0; reg < NUM_SOC_REG; reg++)
    {
        soc_reg_info_t *reginfo;
        soc_reg_above_64_val_t reg_value;
        int i_bl, i_ind;
        int numels;
        char prop_str[PRT_COLUMN_WIDTH_BIG];
        char block_str[PRT_COLUMN_WIDTH_BIG];
        int port;
        pbmp_t bm;

        if (!SOC_REG_IS_VALID(unit, reg))
            continue;

        if((over == FALSE) && shr_access_reg_no_read_get(unit, reg))
            continue;

        if((wb == TRUE) && shr_access_reg_no_wb_get(unit, reg))
            continue;

        /* Check that there is no forbidden field in register */
        if(access_reg_filter_no_read_fields(unit, reg))
            continue;

        reginfo = &SOC_REG_INFO(unit, reg);

        if(reginfo->flags & SOC_REG_FLAG_WO)
            continue;

        /* Filter per register and field names */
        if(access_obj_match(dual_match, SOC_REG_NAME(unit, reg), reginfo->fields, reginfo->nFields,
                                                                    match_h, field_match_h, fld_match_list) != _SHR_E_NONE)
            continue;
        /*
         * Filter on properties
         */
        if (access_reg_properties_get(unit, reg, prop_str, property_h) != _SHR_E_NONE)
            continue;
        /*
         * Filter on blocks
         */
        if (access_reg_blocks_get(unit, reginfo, block_str, block_n) != _SHR_E_NONE)
            continue;

        if((numels = SOC_REG_INFO(unit, reg).numels) == 0)
            numels = 1; /* Simulate index 0 for non arrayed registers, should not happen */

        switch(reginfo->regtype)
        {
            case soc_portreg:
                PBMP_ITER(PBMP_PORT_ALL(unit), port)
                {
    #ifdef BCM_PETRA_SUPPORT
                    int master_port;
                    if(SOC_IS_DPP(unit))
                    {
                        if(!IS_SFI_PORT(unit, port))
                        {
                            if(soc_port_sw_db_master_channel_get(unit, port, &master_port) < 0)
                            {
                                cli_out("Master get failed for:%s\n", SOC_REG_NAME(unit, reg));
                                continue;
                            }
                            if(port != master_port)
                                continue;
                        }
                    }
    #endif
                    if (SOC_REG_BLOCK_IN_LIST(unit, reg, SOC_PORT_TYPE(unit, port)))
                    {
                        if (!ISEMPTY(block_n) && (sal_strcasestr(SOC_PORT_NAME(unit, port), block_n) == NULL))
                            continue;
                        access_reg_read(unit, reg, 0, port, 0, reg_value, NULL);
                        access_reg_print(prt_ctr, unit, reg, SOC_PORT_NAME(unit, port), 0, SOC_REG_BASE(unit, reg), reg_value, field_flag, fld_match_list);
                    }
                }
                break;
            case soc_customreg:
                BCM_PBMP_ASSIGN(bm, SOC_INFO(unit).custom_reg_access.custom_port_pbmp);
                PBMP_ITER(bm, port)
                {
    #ifdef BCM_PETRA_SUPPORT
                    int master_port;
                    if(SOC_IS_DPP(unit))
                    {
                        if(!IS_SFI_PORT(unit, port))
                        {
                            if(soc_port_sw_db_master_channel_get(unit, port, &master_port) < 0)
                            {
                                cli_out("Master get failed for:%s\n", SOC_REG_NAME(unit, reg));
                                continue;
                            }
                            if(port != master_port)
                                continue;
                        }
                    }
    #endif
                    if (!SOC_PORT_VALID(unit, port))
                    {
                        continue;
                    }

                    if (!ISEMPTY(block_n) && (sal_strcasestr(SOC_PORT_NAME(unit, port), block_n) == NULL))
                        continue;

                    access_reg_read(unit, reg, 0, port, 0, reg_value, NULL);
                    access_reg_print(prt_ctr, unit, reg, SOC_PORT_NAME(unit, port), 0, SOC_REG_BASE(unit, reg), reg_value, field_flag, fld_match_list);
                }
                break;
            case soc_iprocreg:
                break;
            default:
                for (i_bl = 0; SOC_BLOCK_INFO(unit, i_bl).type >= 0; i_bl++)
                {   /*
                     * If block type dont match skip this block
                     */
                    if (!SOC_BLOCK_IS_TYPE(unit, i_bl, reginfo->block))
                        continue;
                    /*
                     * If block name filter was used and not matched skip this block
                     */
                    if (!ISEMPTY(block_n) && (sal_strcasestr(SOC_BLOCK_NAME(unit, i_bl), block_n) == NULL))
                        continue;
                    /*
                     * Non-arrayed registers has numels 1, so they will be index 0
                     */
                    for (i_ind = 0; i_ind < numels; i_ind++)
                    {
                        /*
                         * Put reg name for each block to keep consistency
                         */
                        access_reg_read(unit, reg, i_bl, 0, i_ind, reg_value, &address);
                        access_reg_print(prt_ctr, unit, reg, SOC_BLOCK_NAME(unit, i_bl), i_ind, address, reg_value, field_flag, fld_match_list);
                    }
                }
                break;
        }
    }

    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        soc_mem_info_t *meminfo;
        uint8 acc_type;
        int i_bl, i_ind, i_entry;
        int index_start, index_num;
        uint32 mem_value[SOC_MAX_MEM_WORDS];
        int soc_error;
        int mem_size;
        int cur_entry, cur_count;
        char value_str[PRT_COLUMN_WIDTH_BIG];
        char prop_str[PRT_COLUMN_WIDTH_BIG];
        char block_str[PRT_COLUMN_WIDTH_BIG];

        if (!SOC_MEM_IS_VALID(unit, mem))
            continue;

        /* Check via auxilary mcm db */
        if((over == FALSE) && shr_access_mem_no_read_get(unit, mem))
            continue;

        if((wb == TRUE) && shr_access_mem_no_wb_get(unit, mem))
            continue;

        meminfo = &SOC_MEM_INFO(unit, mem);

        if(meminfo->flags & SOC_MEM_FLAG_WRITEONLY)
           continue;

        /* Filter per memory and field names */
        if(access_obj_match(dual_match, SOC_MEM_NAME(unit, mem), meminfo->fields, meminfo->nFields,
                                                                    match_h, field_match_h, fld_match_list) != _SHR_E_NONE)
            continue;
        /*
         * Fill properties
         */
        if (access_mem_properties_get(unit, mem, prop_str, property_h) != _SHR_E_NONE)
            continue;
        /*
         * Fill blocks
         */
        if (access_mem_blocks_get(unit, mem, block_str, block_n) != _SHR_E_NONE)
            continue;

        if(entry == -1)
        {
            cur_entry = soc_mem_index_min(unit, mem);
        }
        else
        {
            if(entry > soc_mem_index_max(unit, mem))
            {
                SHR_CLI_EXIT(_SHR_E_PARAM, "Entry:%d is out of range for %s\n", entry, SOC_MEM_UFNAME(unit, mem));

            }
            else
            {
                cur_entry = entry;
            }
        }

        if(count == 0)
        {
            cur_count = soc_mem_index_max(unit, mem) + 1 - cur_entry;
        }
        else
        {
            if(cur_entry + count > (soc_mem_index_max(unit, mem) + 1))
            {
                SHR_CLI_EXIT(_SHR_E_PARAM, "Count:%d is out of range for %s\n", count, SOC_MEM_UFNAME(unit, mem));
            }
            else
            {
                cur_count = count;
            }
        }

        if((meminfo->flags & SOC_MEM_FLAG_IS_ARRAY) && (SOC_MEM_ARRAY_INFOP(unit, mem) != NULL))
        {
            if(index != -1)
            {
                index_start = index;
                index_num = 1;
            }
            else
            {
                index_start = 0;
                index_num = SOC_MEM_NUMELS(unit, mem);
            }
        }
        else
        {
            index_start = 0;
            index_num = 1;
        }

        if ((mem_size = access_fields_get_ordered_list(meminfo->fields, meminfo->nFields, fld_index_list)) == 0)
        {
            cli_out("Problem with field for memory:%s\n", SOC_MEM_NAME(unit, mem));
            continue;
        }

        SOC_MEM_BLOCK_ITER(unit, mem, i_bl)
        {
            /*
             * If block name filter was used and not matched skip this block
             */
            if (!ISEMPTY(block_n) && (sal_strcasestr(SOC_BLOCK_NAME(unit, i_bl), block_n) == NULL))
                continue;
            for(i_ind = index_start; i_ind < index_start + index_num; i_ind++)
            {
                for(i_entry = cur_entry; i_entry < cur_entry + cur_count; i_entry++)
                {
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SET("%s", SOC_MEM_NAME(unit, mem));
                    PRT_CELL_SET("%s", SOC_BLOCK_NAME(unit, i_bl));

                    if(meminfo->flags & SOC_MEM_FLAG_IS_ARRAY)
                    {
                        PRT_CELL_SET("%d(%d)", i_entry, i_ind);
                    }
                    else
                    {
                        PRT_CELL_SET("%d", i_entry);
                    }
                    address = soc_mem_addr_get(unit, mem, i_ind, i_bl, i_entry, &acc_type);

                    PRT_CELL_SET("%08x", address); /* Address */
                    if (cache == TRUE)
                    {
                        soc_error = soc_mem_array_read(unit, mem, i_ind, i_bl, i_entry, mem_value);
                    }
                    else
                    {
                        soc_error = soc_mem_array_read_flags(unit, mem, i_ind, i_bl, i_entry, mem_value, SOC_MEM_DONT_USE_CACHE);
                    }

                    if (soc_error < 0)
                    {
                        SHR_CLI_EXIT(_SHR_E_INTERNAL, "Read ERROR: table %s.%s[%d]: %s\n", SOC_MEM_UFNAME(unit, mem),
                                                                SOC_BLOCK_NAME(unit, i_bl), i_entry,  soc_errmsg(soc_error));
                    }
                    diag_sand_value_to_str(mem_value, mem_size, value_str, PRT_COLUMN_WIDTH_BIG);
                    PRT_CELL_SET("%s", value_str);

                    if (field_flag == TRUE)
                    {
                        soc_field_info_t *fld;
                        int i_fld;
                        soc_reg_above_64_val_t fld_value;
                        for (i_fld = 0; i_fld < meminfo->nFields; i_fld++)
                        {
                            if(fld_match_list[fld_index_list[i_fld]] == FALSE)
                                continue;

                            fld = &meminfo->fields[fld_index_list[i_fld]];
                            PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                            PRT_CELL_SET_SHIFT(1, "%s", SOC_FIELD_NAME(unit, fld->field));
                            PRT_CELL_SKIP(3);   /* Properties&Blocks&Index */
                            SOC_REG_ABOVE_64_CLEAR(fld_value);
                            SHR_BITCOPY_RANGE(fld_value, 0, mem_value, fld->bp, fld->len);
                            diag_sand_value_to_str(fld_value, fld->len, value_str, PRT_COLUMN_WIDTH_BIG);
                            PRT_CELL_SET_SHIFT(1, "%s", value_str);     /* Reset */
                        }
                    }
                }
            }
        }
    }

    PRT_COMMITX;
exit:
    diag_sand_compare_close(match_h);
    diag_sand_compare_close(field_match_h);
    diag_sand_compare_close(property_h);
    PRT_FREE;
    SHR_FUNC_EXIT;
}

sh_sand_man_t sh_sand_access_man = {
    cmd_sand_access_desc,
    NULL,
    NULL,
    NULL,
};

sh_sand_cmd_t sh_sand_access_cmds[] = {
    {"test",   access_test_cmd,  NULL, access_test_options,  &access_test_man, access_test_option},
    {"blocks", access_block_cmd, NULL, access_block_options, &access_block_man},
    {"list",   access_list_cmd,  NULL, access_list_options,  &access_list_man},
    {"read",   access_read_cmd,  NULL, access_read_options,  &access_read_man},
    {"property", access_property_cmd,  NULL, access_property_options,  &access_property_man},
    {NULL}
};

/*
 * This routine is for DPP/DFE only - For DNX/DNXF recursion starts from the top 
 */
cmd_result_t
cmd_sand_access(
    int unit,
    args_t * args)
{
    sh_sand_act(unit, args, sh_sand_access_cmds);
    /*
     * Always return OK - we provide all help & usage from inside framework
     */
    return CMD_OK;

}

/*
 * General shell style usage
 */
const char cmd_sand_access_usage[] = "Please use \"access(acc) list/read usage\" for help\n";

/*
 * General shell style description
 */
const char cmd_sand_access_desc[] = "List/Read/Modify registers&memories";

