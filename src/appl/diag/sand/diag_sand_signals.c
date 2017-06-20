/*
 * $Id: diag_sand_dsig.c,v 1.00 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    diag_sand_dsig.c
 * Purpose:    Routines for handling debug and internal signals
 */

#include <soc/drv.h>

#include <sal/appl/sal.h>

#include <shared/bitop.h>
#include <shared/utilex/utilex_str.h>
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_integer_arithmetic.h>

#include <appl/diag/sand/diag_sand_dsig.h>
#include <appl/diag/sand/diag_sand_framework.h>
#include <appl/diag/sand/diag_sand_signals.h>

#include <soc/shared/sand_signals.h>

#include <bcm/types.h>

#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/dpp_config_defs.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_diag.h>
#endif

#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/dnx_data/dnx_data_device.h>
#endif

#define BSL_LOG_MODULE BSL_LS_APPL_SHELL

static sh_sand_man_t signal_get_man = {
    "List of signals with values",
    "List of signals filtered and shown using misc. filtering and showing options",
    "signal [name=<string>]\n",
    "signal name=TM_Cmd",
};

static sh_sand_enum_t order_enum_table[] = {
    {"big", 0},
    {"little", 1},
    {NULL}
};

static sh_sand_option_t signal_get_options[] = {
    {"name",  SAL_FIELD_TYPE_STR, "Signal name or its substring", ""},
    {"block", SAL_FIELD_TYPE_STR, "Show only signals from this block", ""},
    {"stage", SAL_FIELD_TYPE_STR, "Show only signals going to or coming from this stage", ""},
    {"to",    SAL_FIELD_TYPE_STR, "Show only signals going to this stage", ""},
    {"from",  SAL_FIELD_TYPE_STR, "Show only signals coming from this stage", ""},
    {"show",  SAL_FIELD_TYPE_STR, "Misc. options to control filtering/output", ""},
    {"order", SAL_FIELD_TYPE_ENUM,  "Print values in certain endian order <little/bug>", "big", (void *)order_enum_table},
    {"all",   SAL_FIELD_TYPE_BOOL, "Prints all signals ignoring other filtering input", "No"},
    {NULL}
};

static shr_error_e
sand_signal_expand_print(
    rhlist_t * field_list,
    int depth,
    prt_control_t *prt_ctr,
    int attr_offset,
    int flags)
{
    signal_output_t *field_output;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    RHITERATOR(field_output, field_list)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SKIP(attr_offset);

        if (!ISEMPTY(field_output->expansion) && sal_strcasecmp(RHNAME(field_output), field_output->expansion))
        {
            PRT_CELL_SET_SHIFT(depth, "%s(%s)", RHNAME(field_output), field_output->expansion);
        }
        else
        {
            PRT_CELL_SET_SHIFT(depth, "%s", RHNAME(field_output));
        }
        PRT_CELL_SET("%d", field_output->size);
        if (flags & SIGNALS_PRINT_VALUE)
        {
            PRT_CELL_SET("%s", field_output->print_value);
        }

        SHR_CLI_EXIT_IF_ERR(sand_signal_expand_print(field_output->field_list, depth + 1, prt_ctr, attr_offset, flags), "");
    }

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
signal_get_cb(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    match_t match_m;
    int core, core_num = 0;
    int attr_offset = 0, addr_offset = 0;
    int i_addr;
    rhlist_t *dsig_list = NULL;
    debug_signal_t *debug_signal;
    signal_output_t *signal_output;
    int all_flag, print_flags = 0;
    char *show_str;

    device_t *device;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    if ((device = sand_signal_device_get(unit)) == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_NOT_FOUND, "");
    }

#ifdef BCM_PETRA_SUPPORT
    if(SOC_IS_DPP(unit))
    {
        core_num = SOC_DPP_DEFS_GET(unit, nof_cores);
    }
#endif
#ifdef BCM_DNX_SUPPORT
    if(SOC_IS_DNX(unit))
    {
        core_num = dnx_data_device.general.nof_cores_get(unit);
    }
#endif

    sal_memset(&match_m, 0, sizeof(match_t));

    match_m.flags = SIGNALS_MATCH_EXPAND;

    SH_SAND_GET_STR("name",   match_m.name);
    SH_SAND_GET_STR("block",  match_m.block);
    SH_SAND_GET_STR("from",   match_m.from);
    SH_SAND_GET_STR("to",     match_m.to);
    SH_SAND_GET_STR("stage",  match_m.stage);
    SH_SAND_GET_INT32("core", core);
    SH_SAND_GET_BOOL("all",   all_flag);
    SH_SAND_GET_ENUM("order", match_m.output_order);

    if(all_flag == TRUE)
    {
        match_m.name  = NULL;
        match_m.from  = NULL;
        match_m.to    = NULL;
        match_m.stage = NULL;
    }
    else if(ISEMPTY(match_m.name) && ISEMPTY(match_m.from) && ISEMPTY(match_m.to) && ISEMPTY(match_m.stage))
    {
        SHR_CLI_EXIT(_SHR_E_PARAM, "Please specify some filtering criteria ar use \"all\" to show all signals\n");
    }

    SH_SAND_GET_STR("show",   show_str);

    if(!ISEMPTY(show_str))
    {   /* Show options parsing */
        char **tokens;
        uint32 realtokens = 0;
        int i_token;

        if ((tokens = utilex_str_split(show_str, ",", 6, &realtokens)) == NULL)
        {
            SHR_CLI_EXIT(_SHR_E_INTERNAL, "Problem parsing show string\n");
        }

        for (i_token = 0; i_token < realtokens; i_token++)
        {
            if (!sal_strcasecmp(tokens[i_token], "detail"))
                print_flags |= SIGNALS_PRINT_DETAIL;
            else if (!sal_strcasecmp(tokens[i_token], "hw"))
                print_flags |= SIGNALS_PRINT_HW;
            else if (!sal_strcasecmp(tokens[i_token], "source"))
                match_m.flags |= SIGNALS_MATCH_HW;
            else if (!sal_strcasecmp(tokens[i_token], "perm"))
                match_m.flags |= SIGNALS_MATCH_PERM;
            else if (!sal_strcasecmp(tokens[i_token], "noexpand"))
                match_m.flags &= ~SIGNALS_MATCH_EXPAND;
            else if (!sal_strcasecmp(tokens[i_token], "exact"))
                match_m.flags |= SIGNALS_MATCH_EXACT;
            else if (!sal_strcasecmp(tokens[i_token], "nocond"))
                match_m.flags |= SIGNALS_MATCH_NOCOND;
            else if (!sal_strcasecmp(tokens[i_token], "lucky"))
                match_m.flags |= SIGNALS_MATCH_ONCE;
            else
            {
                utilex_str_split_free(tokens, realtokens);
                SHR_CLI_EXIT(_SHR_E_PARAM, "Unknown show option:%s\n", tokens[i_token]);
            }
        }

        utilex_str_split_free(tokens, realtokens);
    }

    if ((dsig_list = utilex_rhlist_create("prt_print", sizeof(signal_output_t), 0)) == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_MEMORY, "Failed to create signal list\n");
    }

    if (core == -1)
    {
        if (core_num > 1)
        {
            print_flags |= SIGNALS_PRINT_CORE;
        }
        for (core = 0; core < core_num; core++)
        {
            sand_signal_list_get(device, unit, core, &match_m, dsig_list);
        }
    }
    else
    {
        if ((core < 0) || (core >= core_num))
        {
            SHR_CLI_EXIT(_SHR_E_PARAM, "Illegal core ID:%d for device\n", core);
        }
        sand_signal_list_get(device, unit, core, &match_m, dsig_list);
    }

    PRT_TITLE_SET("Signals");

    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADD("Block");
    PRT_COLUMN_ADD("From");
    PRT_COLUMN_ADD("To");

    attr_offset = PRT_COLUMN_NUM;

    PRT_COLUMN_ADD("Attribute");
    PRT_COLUMN_ADD("Size");
    PRT_COLUMN_ADD_FLEX(PRT_FLEX_BINARY, "Value");

    if (print_flags & SIGNALS_PRINT_DETAIL)
    {
        PRT_COLUMN_ADD("Perm");
        PRT_COLUMN_ADD("HW");
        addr_offset = PRT_COLUMN_NUM;
        PRT_COLUMN_ADD("High");
        PRT_COLUMN_ADD("Low");
        PRT_COLUMN_ADD("MSB");
        PRT_COLUMN_ADD("LSB");
    }
    else if (print_flags & SIGNALS_PRINT_HW)
    {
        PRT_COLUMN_ADD("HW");
    }

    RHITERATOR(signal_output, dsig_list)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        debug_signal = signal_output->debug_signal;
        if (print_flags & SIGNALS_PRINT_CORE)
        {
            PRT_CELL_SET("%s:%d", debug_signal->block_n, signal_output->core);
        }
        else
        {
            PRT_CELL_SET("%s", debug_signal->block_n);
        }
        PRT_CELL_SET("%s", debug_signal->from);
        PRT_CELL_SET("%s", debug_signal->to);
        if (!ISEMPTY(debug_signal->expansion) && sal_strcasecmp(debug_signal->attribute, debug_signal->expansion))
        {
            PRT_CELL_SET("%s(%s)", debug_signal->attribute, debug_signal->expansion);
        }
        else
        {
            PRT_CELL_SET("%s", debug_signal->attribute);
        }
        PRT_CELL_SET("%d", debug_signal->size);
        PRT_CELL_SET("%s", signal_output->print_value);

        if (print_flags & SIGNALS_PRINT_DETAIL)
        {
            PRT_CELL_SET("%d", debug_signal->perm);
            PRT_CELL_SET("%s", debug_signal->hw);
            for (i_addr = 0; i_addr < debug_signal->range_num; i_addr++)
            {
                if (i_addr != 0)
                {
                    PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                    PRT_CELL_SKIP(addr_offset);
                }
                PRT_CELL_SET("%d", debug_signal->address[i_addr].high);
                PRT_CELL_SET("%d", debug_signal->address[i_addr].low);
                PRT_CELL_SET("%d", debug_signal->address[i_addr].msb);
                PRT_CELL_SET("%d", debug_signal->address[i_addr].lsb);
            }
        }
        else if (print_flags & SIGNALS_PRINT_HW)
        {
            PRT_CELL_SET("%s", debug_signal->hw);
        }

        SHR_CLI_EXIT_IF_ERR(sand_signal_expand_print(signal_output->field_list, 1, prt_ctr, attr_offset, print_flags), "");
    }

    PRT_COMMITX;
exit:
    if(dsig_list != NULL)
    {
        sand_signal_list_free(dsig_list);
    }
    PRT_FREE;
    SHR_FUNC_EXIT;
}

static sh_sand_man_t signal_struct_man = {
    "Signal structures",
    "List of parsing capabilites for certian signals",
    "signal struct [name=<string>]\n",
    "signal struct name=TM_Cmd",
};

static sh_sand_option_t signal_struct_options[] = {
    {"name", SAL_FIELD_TYPE_STR, "Struct name or its substring", ""},
    {NULL}
};

static shr_error_e
signal_struct_cb(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    char *match_n;
    int i_str, i_field;
    device_t *device;
    sigstruct_t *cur_sigstruct;
    sigstruct_field_t *cur_sigstruct_field;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    if ((device = sand_signal_device_get(unit)) == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_NOT_FOUND, "");
    }

    SH_SAND_GET_STR("name", match_n);

    PRT_TITLE_SET("Parsed Structures");

    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADD("Signal");
    PRT_COLUMN_ADD("Size");
    PRT_COLUMN_ADD("Field");
    PRT_COLUMN_ADD("Start");
    PRT_COLUMN_ADD("Size");
    PRT_COLUMN_ADD("Condition");

    for (i_str = 0; i_str < device->sigstruct_num; i_str++)
    {
        cur_sigstruct = &device->sigstructs[i_str];
        if (!ISEMPTY(match_n) && (sal_strcasestr(cur_sigstruct->name, match_n) == NULL))
            continue;

        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", cur_sigstruct->name);
        PRT_CELL_SET("%d", cur_sigstruct->size);

        for (i_field = 0; i_field < cur_sigstruct->field_num; i_field++)
        {
            cur_sigstruct_field = &cur_sigstruct->fields[i_field];
            if (i_field != 0)
            {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SKIP(2);
            }

            PRT_CELL_SET("%s", cur_sigstruct_field->name);
            PRT_CELL_SET("%d", cur_sigstruct_field->start_bit);
            PRT_CELL_SET("%d", cur_sigstruct_field->size);
            PRT_CELL_SET("%s", cur_sigstruct_field->cond_attribute);
        }
    }

    PRT_COMMITX;
exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

static sh_sand_man_t signal_stage_man = {
    "Lists stages of the pipeline",
    "List of stages serves as a facilitator to signal filtering tool",
    "signal stages [name=<string>]\n",
    "signal stage block=IRPP\n"
    "signal stage name=parser",
};

static sh_sand_option_t signal_stage_options[] = {
    {"name", SAL_FIELD_TYPE_STR, "Full stage name or its substring", ""},
    {"block", SAL_FIELD_TYPE_STR, "Full block name or its substring", ""},
    {NULL}
};

static shr_error_e
signal_stage_cb(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    char *match_n, *block_n;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    SH_SAND_GET_STR("name", match_n);
    SH_SAND_GET_STR("block", block_n);

    PRT_TITLE_SET("SIGNAL STAGES");

    PRT_COLUMN_ADD("Block");
    PRT_COLUMN_ADD("Stage");

#ifdef CMODEL_SERVER_MODE
    PRT_COLUMN_ADD("ID");
#endif

    {
        int i_bl, i_st;
        pp_block_t *cur_pp_block;
        pp_stage_t *cur_pp_stage;
        device_t *device;
        if ((device = sand_signal_device_get(unit)) == NULL)
        {
            SHR_CLI_EXIT(_SHR_E_NOT_FOUND, "");
        }

        for (i_bl = 0; i_bl < device->block_num; i_bl++)
        {
            cur_pp_block = &device->pp_blocks[i_bl];

            if (!ISEMPTY(block_n) && (sal_strcasestr(cur_pp_block->name, block_n) == NULL))
                continue;

            for (i_st = 0; i_st < cur_pp_block->stage_num; i_st++)
            {
                cur_pp_stage = &cur_pp_block->stages[i_st];

                if (!ISEMPTY(match_n) && (sal_strcasestr(cur_pp_stage->name, match_n) == NULL))
                    continue;

                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET("%s", cur_pp_block->name);
                PRT_CELL_SET("%s", cur_pp_stage->name);
#ifdef CMODEL_SERVER_MODE
                PRT_CELL_SET("%d", cur_pp_stage->id);
#endif
            }
        }
    }

    PRT_COMMITX;
exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

static sh_sand_man_t signal_param_man = {
    "List of parameters",
    "List of parameters - names for values of certain signals",
    "signal param [name=<string>]\n",
    "signal param name=FHEI_Code",
};

static sh_sand_option_t signal_param_options[] = {
    {"name", SAL_FIELD_TYPE_STR, "Parameter name or its substring", ""},
    {NULL}
};

static shr_error_e
signal_param_cb(
    int unit,
    args_t * args,
    sh_sand_control_t * sand_control)
{
    char *match_n;
    int i_par, i_val;
    device_t *device;
    sigparam_t *cur_sigparam;
    sigparam_value_t *cur_sigparam_value;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    if ((device = sand_signal_device_get(unit)) == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_NOT_FOUND, "");
    }

    SH_SAND_GET_STR("name", match_n);

    PRT_TITLE_SET("Parameters Decoding");
    /*
     * Prepare header, pay attention to put header items and content in the same order
     */
    PRT_COLUMN_ADD("Signal");
    PRT_COLUMN_ADD("Size");
    PRT_COLUMN_ADD("Value Name");
    PRT_COLUMN_ADD("Value");

    for (i_par = 0; i_par < device->sigparam_num; i_par++)
    {
        cur_sigparam = &device->sigparams[i_par];
        if (!ISEMPTY(match_n) && (sal_strcasestr(cur_sigparam->name, match_n) == NULL))
            continue;

        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("%s", cur_sigparam->name);
        PRT_CELL_SET("%d", cur_sigparam->size);

        for (i_val = 0; i_val < cur_sigparam->value_num; i_val++)
        {
            cur_sigparam_value = &cur_sigparam->values[i_val];
            if (i_val != 0)
            {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SKIP(2);
            }

            PRT_CELL_SET("%s", cur_sigparam_value->name);
            PRT_CELL_SET("%d", cur_sigparam_value->value);
        }
    }


    PRT_COMMITX;
exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

sh_sand_cmd_t sh_sand_signal_cmds[] = {
    {"get",        signal_get_cb,    NULL, signal_get_options,    &signal_get_man},
    {"structure",  signal_struct_cb, NULL, signal_struct_options, &signal_struct_man},
    {"stage",      signal_stage_cb,  NULL, signal_stage_options,  &signal_stage_man},
    {"parameter",  signal_param_cb,  NULL, signal_param_options,  &signal_param_man},
    {NULL}
};

sh_sand_man_t sh_sand_signal_man = {
    cmd_sand_signal_desc,
    NULL,
    NULL,
    NULL,
};

cmd_result_t
cmd_sand_signal(
    int unit,
    args_t * args)
{
    sh_sand_act(unit, args, sh_sand_signal_cmds);
    /*
     * Always return OK - we provide all help & usage from inside framework
     */
    return CMD_OK;

}

const char cmd_sand_signal_usage[] = "Please use \"signal(sig) usage\" for help\n";
/*
 * General shell style description
 */
const char cmd_sand_signal_desc[] = "Present device signals per filtering criteria";
