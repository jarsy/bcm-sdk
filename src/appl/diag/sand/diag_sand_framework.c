/**
 * \file diag_sand_framework.c
 *
 * Framework for sand shell commands development
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <sal/core/regex.h>
#include <sal/appl/sal.h>

#include <shared/utilex/utilex_integer_arithmetic.h>
#include <shared/shrextend/shrextend_debug.h>
#include <appl/diag/system.h>
#include <appl/diag/sand/diag_sand_framework.h>
#include <appl/diag/sand/diag_sand_utils.h>

#define BSL_LOG_MODULE BSL_LS_APPL_SHELL

sh_sand_control_t sys_ctr = { NULL, NULL};

#define MAX_WIDTH 256

void
sh_sand_print(char *string, int left_margin, int right_margin, int term_width)
{
    int token_size;
    int str_shift;
    int str_offset = 0;
    int str_size = strlen(string);

    token_size = term_width - left_margin - right_margin;

    do
    {
        /*
         * Print left margin
         */
        diag_sand_prt_char(left_margin, ' ');
        /*
         * Get shift where last white space in the token or first new line are situated
         */
        str_shift = utilex_str_get_shift(string + str_offset, token_size);
        /*
         * Print string up to this delimiter, print only specified number of characters(str_shift) from the string
         */
        LOG_CLI((BSL_META("%.*s\n"), str_shift, string + str_offset));
        /*
         * If delimiter is new line print one more new line
         */
        if(*(string + str_offset) == '\n')
        {
            LOG_CLI((BSL_META("\n")));
        }
        /*
         * Update current string offset taking into account delimiter
         */
        str_offset += str_shift;
    } while(str_offset < str_size); /* once current offset exceeds string length stop */
}

/**
 * Keeping all the tokens form general shell to be compliant :)
 */
static sh_sand_enum_t sand_bool_table[] = {
    {"False",  FALSE},
    {"True",   TRUE},
    {"Y",      TRUE},
    {"N",      FALSE},
    {"Yes",    TRUE},
    {"No",     FALSE},
    {"On",     TRUE},
    {"Off",    FALSE},
    {"Yep",    TRUE},
    {"Nope",   FALSE},
    {"1",      TRUE},
    {"0",      FALSE},
    {"OKay",   TRUE},
    {"YOUBET", TRUE},
    {"NOWay",  FALSE},
    {"YEAH",   TRUE},
    {"NOT",    FALSE},
    {NULL}
};

char *sh_sand_bool_str(
        int bool)
{
    if(bool == FALSE)
        return sand_bool_table[0].string;
    else
        return sand_bool_table[1].string;
}

static shr_error_e
sh_sand_get_value(
    sal_field_type_e type,
    char *source,
    sh_sand_param_u * target,
    void *ext_ptr)
{
    char *end_ptr;
    SHR_FUNC_INIT_VARS(NO_UNIT);

    switch (type)
    {
        case SAL_FIELD_TYPE_BOOL:
            if(source == NULL)
            { /* When boolean option appears on command line without value - meaning is TRUE */
                target->val_bool = TRUE;
            }
            else
            {
                int i_ind;
                for(i_ind = 0; sand_bool_table[i_ind].string != NULL; i_ind++)
                {
                    if(!sal_strcasecmp(sand_bool_table[i_ind].string, source))
                    {
                        target->val_bool = sand_bool_table[i_ind].value;
                        break;
                    }
                }
                if(sand_bool_table[i_ind].string == NULL)
                {
                    SHR_CLI_EXIT(_SHR_E_PARAM, "Boolean string:%s is not supported\n", source);
                }
            }
            break;
        case SAL_FIELD_TYPE_INT32:
            target->val_int32 = sal_strtol(source, &end_ptr, 0);
            if (end_ptr[0] != 0)
            {
                SHR_SET_CURRENT_ERR(_SHR_E_PARAM);
            }
            break;
        case SAL_FIELD_TYPE_UINT32:
            target->val_uint32 = sal_strtoul(source, &end_ptr, 0);
            if (end_ptr[0] != 0)
            {
                SHR_SET_CURRENT_ERR(_SHR_E_PARAM);
            }
            break;
        case SAL_FIELD_TYPE_IP4:
            if (parse_ipaddr(source, &target->ip4_addr) != _SHR_E_NONE)
            {
                SHR_SET_CURRENT_ERR(_SHR_E_PARAM);
            }
            break;
        case SAL_FIELD_TYPE_IP6:
            if (parse_ip6addr(source, target->ip6_addr) != _SHR_E_NONE)
            {
                SHR_SET_CURRENT_ERR(_SHR_E_PARAM);
            }
            break;
        case SAL_FIELD_TYPE_STR:
            sal_strncpy(target->val_str, source, SH_SAND_MAX_TOKEN_SIZE - 1);
            break;
        case SAL_FIELD_TYPE_MAC:
            if (parse_macaddr(source, target->mac_addr) != _SHR_E_NONE)
            {
                SHR_SET_CURRENT_ERR(_SHR_E_PARAM);
            }
            break;
        case SAL_FIELD_TYPE_ARRAY32:
            parse_long_integer(target->array_uint32, SH_SAND_MAX_ARRAY32_SIZE, source);
            break;
        case SAL_FIELD_TYPE_BITMAP:
            break;
        case SAL_FIELD_TYPE_ENUM:
            {
                sh_sand_enum_t *enum_entry = (sh_sand_enum_t *)ext_ptr;
                if(enum_entry == NULL)
                {
                    SHR_CLI_EXIT(_SHR_E_INTERNAL, "Bad enum option for:%s\n", source);
                }
                for(; enum_entry->string != NULL; enum_entry++)
                {
                    if(!sal_strcasecmp(enum_entry->string, source))
                    {
                        target->val_enum = enum_entry->value;
                        break;
                    }
                }
                if(enum_entry->string == NULL)
                {
                    SHR_CLI_EXIT(_SHR_E_PARAM, "Enum string:%s is not supported\n", source);
                }
            }
            break;
        case SAL_FIELD_TYPE_NONE:
        default:
            SHR_CLI_EXIT(_SHR_E_PARAM, "Unsupported parameter type:%d\n", type);
            break;
    }

exit:
    SHR_FUNC_EXIT;
}

static void
sh_sand_init_keywords(void)
{
    int i_key, i_ch, i_sh;

    for(i_key = 0; sh_sand_keywords[i_key].keyword != NULL; i_key++)
    {   /* Build shortcut */
        i_sh = 0;
        for(i_ch = 0; i_ch < sal_strlen(sh_sand_keywords[i_key].keyword); i_ch++)
        {
            if(isupper(sh_sand_keywords[i_key].keyword[i_ch]))
            {
                sh_sand_keywords[i_key].short_key[i_sh++] = sh_sand_keywords[i_key].keyword[i_ch];
            }
        }
        /*
         * Null terminate the string. If no capital - no shortcut was defined
         */
        sh_sand_keywords[i_key].short_key[i_sh] = 0;
    }
}

static shr_error_e
sh_sand_verify_keyword(
    char *keyword,
    char **short_key_p)
{
    int i_key;
    int str_len;

    SHR_FUNC_INIT_VARS(NO_UNIT);
    for(i_key = 0; sh_sand_keywords[i_key].keyword != NULL; i_key++)
    {
        if(!sal_strcasecmp(sh_sand_keywords[i_key].keyword, keyword))
        {
            break;
        }
        str_len = sal_strlen(keyword);
        if((keyword[str_len - 1] == 's') || (keyword[str_len - 1] == 'S'))
        {
            str_len--;
            if(!sal_strncasecmp(sh_sand_keywords[i_key].keyword, keyword, str_len))
            {
                break;
            }
        }
    }

    if(sh_sand_keywords[i_key].keyword == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_INIT, "Keyword:\"%s\" was not registered\n", keyword);
    }

    if(short_key_p != NULL)
    {
        *short_key_p = sh_sand_keywords[i_key].short_key;
    }

exit:
    SHR_FUNC_EXIT;
}

static sh_sand_arg_t*
sh_sand_get_option(rhlist_t *args_list, char *option_name)
{
    sh_sand_arg_t *sh_sand_arg;

    RHITERATOR(sh_sand_arg, args_list)
    {
        if (!sal_strcasecmp(option_name, RHNAME(sh_sand_arg))
            || ((sh_sand_arg->short_key != NULL) && (!sal_strcasecmp(option_name, sh_sand_arg->short_key))))
        {
            break;
        }
    }

    return sh_sand_arg;
}

static shr_error_e
sh_sand_args_process(
    int unit,
    args_t * args,
    rhlist_t * args_list,
    int legacy_mode,
    sh_sand_cmd_t * sh_sand_cmd)
{
    sh_sand_arg_t *sh_sand_arg;
    char *cur_arg;
    int i;
    int variable_present = FALSE;
    char **tokens = NULL;
    uint32 realtokens = 0;
    char *option_value = NULL, *option_name = NULL;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    if (args_list == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_PARAM, "ERROR - input parameters problem\n");
    }

    if(legacy_mode == TRUE)
    {   /* No option processing for legacy commands */
        SHR_EXIT() ;
    }

    RHITERATOR(sh_sand_arg, args_list)
    {
        sh_sand_arg->present = FALSE;
    }

    /*
     * Create or clean dynamic List
     */
    if(sh_sand_cmd != NULL)
    {
        if(sh_sand_cmd->ctr.dyn_args_list == NULL)
        {
            if ((sh_sand_cmd->ctr.dyn_args_list = utilex_rhlist_create("Options", sizeof(sh_sand_arg_t), 1)) == NULL)
            {
                SHR_CLI_EXIT(_SHR_E_MEMORY, "ERROR - No memory for dyn args list\n");
            }
        }
        else /* dyn_args_list != NULL */
        {
            utilex_rhlist_clean(sh_sand_cmd->ctr.dyn_args_list);
        }
    }

    /*
     * Check all tokens left - supposedly opions
     */
    for (i = 0; i < ARG_CNT(args); i++)
    {
        if ((cur_arg = ARG_GET_WITH_INDEX(args, ARG_CUR_INDEX(args) + i)) == NULL)
        {
            SHR_CLI_EXIT(_SHR_E_PARAM, "No option for:%d\n", ARG_CUR_INDEX(args) + i);
        }
        /*
         * Analyze option
         */
        if((tokens = utilex_str_split(cur_arg, "=", 3, &realtokens)) == NULL)
        {
            SHR_CLI_EXIT(_SHR_E_MEMORY, "Inconsistent input command\n");
        }
        /* If there is no = in the token, for one occurrence we assume that it is "variable" option */
        option_name = tokens[0];
        switch(realtokens)
        {
            case 0:
                SHR_CLI_EXIT(_SHR_E_MEMORY, "No tokens in:%s\n", cur_arg);
                break;
            case 1:
                if ((sh_sand_arg = sh_sand_get_option(args_list, option_name)) == NULL)
                {   /* For one unknown option we can assume its name is variable */
                    if(variable_present == FALSE)
                    {
                        option_name = "variable";
                        if ((sh_sand_arg = utilex_rhlist_entry_get_by_sub(args_list, option_name)) == NULL)
                        {
                            SHR_CLI_EXIT(_SHR_E_PARAM, "Option:%s is not supported\n", tokens[0]);
                        }
                        else
                        {
                            variable_present = TRUE;
                            option_value = tokens[0];
                        }
                    }
                    else
                    {
                        SHR_CLI_EXIT(_SHR_E_PARAM, "Only one free variable is allowed\n");
                    }
                }
                else
                {   /* Find option, but only boolean are allowed to miss the value */
                    if (sh_sand_arg->type != SAL_FIELD_TYPE_BOOL)
                    {
                        SHR_CLI_EXIT(_SHR_E_PARAM, "Option:\"%s\" requires a value\n", option_name);
                    }
                    else
                    {   /* Boolean option with no explicit value */
                        option_value = NULL;
                    }
                }
                break;
            case 2: /* regular option with value*/
                /*
                 * Look for keyword in the args_list
                 */
                if ((sh_sand_arg = sh_sand_get_option(args_list, option_name)) == NULL)
                {
                    sal_field_type_e    type;
                    uint32              id;
                    rhhandle_t          void_arg;
                    void               *ext_ptr;

                    /*
                     * Check if the option is dynamic one
                     */
                    if((sh_sand_cmd != NULL) && (sh_sand_cmd->option_cb != NULL))
                    {
                        if(sh_sand_cmd->option_cb(unit, option_name, &type, &id, &ext_ptr) == _SHR_E_NONE)
                        {
                            if (utilex_rhlist_entry_add_tail(sh_sand_cmd->ctr.dyn_args_list,
                                                                    option_name, id, &void_arg) != _SHR_E_NONE)
                            {
                                SHR_CLI_EXIT(_SHR_E_MEMORY, "ERROR - Cannot add option:%s to dynamic list\n", option_name);
                            }
                            sh_sand_arg = void_arg;
                            sh_sand_arg->type = type;
                            sh_sand_arg->ext_ptr = ext_ptr;
                        }
                        else
                        {
                            SHR_CLI_EXIT(_SHR_E_MEMORY, "ERROR - Option:%s not recognized\n", option_name);
                        }
                    }
                    else
                    {
                        SHR_CLI_EXIT(_SHR_E_PARAM, "Option:%s is not supported\n", option_name);
                    }
                }
                option_value = tokens[1];
                break;
            case 3:
            default:
                SHR_CLI_EXIT(_SHR_E_PARAM, "More than 1 \"=\" in the input\n");
                break;
        }

        sh_sand_arg->present = TRUE;
        if (sh_sand_get_value(sh_sand_arg->type, option_value, &sh_sand_arg->param, sh_sand_arg->ext_ptr) != _SHR_E_NONE)
        {
            SHR_CLI_EXIT(_SHR_E_PARAM, "Illegal value:\"%s\" for option:\"%s\"\n", option_value, option_name);
        }

        utilex_str_split_free(tokens, realtokens);
        tokens = NULL;
    }

    RHITERATOR(sh_sand_arg, args_list)
    {
        if((sh_sand_arg->present == FALSE) && (sh_sand_arg->requested == TRUE))
        {
            SHR_CLI_EXIT(_SHR_E_PARAM, "Presence of option:\"%s\" is requested\n", RHNAME(sh_sand_arg));
        }
    }

exit:
    if(tokens != NULL)
    {   /* Means we left the loop on error */
        utilex_str_split_free(tokens, realtokens);
    }
    SHR_FUNC_EXIT;
}

static shr_error_e
sh_sand_usage_leaf_tabular(
    sh_sand_cmd_t * sh_sand_cmd,
    sh_sand_control_t *sand_control)
{
    sh_sand_option_t *option;
    int item_col_id, option_col_id;
    int flag;
    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(NO_UNIT);

    if ((sh_sand_cmd == NULL) || (sh_sand_cmd->man == NULL))
    { /* On this stage there is no need to print message, all errors should be rectified by verify */
        SHR_EXIT();
    }

    PRT_TITLE_SET("Usage");

    PRT_COLUMN_ADDX(PRT_XML_CHILD, PRT_TITLE_ID, &item_col_id, "Item");
    PRT_COLUMN_ADDX(PRT_XML_CHILD, item_col_id,  &option_col_id, "Option");
    PRT_COLUMN_ADDX(PRT_XML_ATTRIBUTE, option_col_id, NULL, "Type");
    PRT_COLUMN_ADDX(PRT_XML_ATTRIBUTE, option_col_id, NULL, "Default");
    PRT_COLUMN_ADDX_FLEX(PRT_FLEX_ASCII, PRT_XML_ATTRIBUTE, option_col_id, NULL, "Description");

    if (sh_sand_cmd->man->synopsis != NULL)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
        PRT_CELL_SET("SYNOPSYS");
        PRT_CELL_SKIP(3);
        PRT_CELL_SET("%s", sh_sand_cmd->man->synopsis);
    }

    if (sh_sand_cmd->man->full != NULL)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
        PRT_CELL_SET("DESCRIPTION");
        PRT_CELL_SKIP(3);
        PRT_CELL_SET("%s", sh_sand_cmd->man->full);
    }

    if (sh_sand_cmd->options != NULL)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("OPTIONS");
        for (option = sh_sand_cmd->options; option->keyword != NULL; option++)
        {
            char *def_str;
            if(option->def == NULL)
            {
                def_str = "NONE";
            }
            else if(ISEMPTY(option->def))
            {
                def_str = "EMPTY";
            }
            else
            {
                def_str = option->def;
            }

            /* For the first option skip 1 cell, for all others allocate new row */
            if(option != sh_sand_cmd->options)
            {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SKIP(1);
            }
            PRT_CELL_SET("%s", option->keyword);
            PRT_CELL_SET("%s", sal_field_type_str(option->type));
            PRT_CELL_SET("%s", def_str);
            PRT_CELL_SET("%s", option->desc);
        }
        PRT_ROW_SET_MODE(PRT_ROW_SEP_UNDERSCORE);
    }

    /*
     * Show system options only when verbose option used
     */
    SH_SAND_GET_BOOL("verbose", flag);
    if(flag == TRUE)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
        PRT_CELL_SET("SYSTEM OPTIONS");
        for (option = sh_sand_sys_options; option->keyword != NULL; option++)
        {
            char *def_str;
            if(option->def == NULL)
            {
                def_str = "NONE";
            }
            else if(ISEMPTY(option->def))
            {
                def_str = "EMPTY";
            }
            else
            {
                def_str = option->def;
            }

            /* For the first option skip 1 cell, for all others allocate new row */
            if(option != sh_sand_sys_options)
            {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SKIP(1);
            }
            PRT_CELL_SET("%s", option->keyword);
            PRT_CELL_SET("%s", sal_field_type_str(option->type));
            PRT_CELL_SET("%s", def_str);
            PRT_CELL_SET("%s", option->desc);
        }
        PRT_ROW_SET_MODE(PRT_ROW_SEP_UNDERSCORE);
    }

    if (sh_sand_cmd->man->examples != NULL)
    {
        PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
        PRT_CELL_SET("EXAMPLES");
        PRT_CELL_SKIP(3);
        PRT_CELL_SET("%s", sh_sand_cmd->man->examples);
    }

    PRT_COMMITX;
exit:
    PRT_FREE;
    SHR_FUNC_EXIT;
}

static shr_error_e
sh_sand_usage_leaf(
    sh_sand_cmd_t * sh_sand_cmd,
    sh_sand_control_t *sand_control)
{
    int columns, margin;
    int flag;
    sh_sand_option_t *option;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    if ((sh_sand_cmd == NULL) || (sh_sand_cmd->man == NULL))
    { /* On this stage there is no need to print message, all errors should be rectified by verify */
        SHR_EXIT();
    }

    SH_SAND_GET_INT32("tabular", flag);
    if(flag == TRUE)
    {
        sh_sand_usage_leaf_tabular(sh_sand_cmd, sand_control);
        goto exit;
    }

    SH_SAND_GET_INT32("columns", columns);
    SH_SAND_GET_INT32("margin", margin);

    if (sh_sand_cmd->man->synopsis != NULL)
    {
        LOG_CLI((BSL_META("\nSYNOPSYS\n")));
        sh_sand_print(sh_sand_cmd->man->synopsis, margin, margin, columns);
    }

    if (sh_sand_cmd->man->full != NULL)
    {
        LOG_CLI((BSL_META("\nDESCRIPTION\n")));
        sh_sand_print(sh_sand_cmd->man->full, margin, margin, columns);
    }

    if (sh_sand_cmd->options != NULL)
    {
        LOG_CLI((BSL_META("\nOPTIONS\n")));
        diag_sand_prt_char(margin, ' ');
        LOG_CLI((BSL_META("<option_name> (<option_type>:<default_value>)\n\n")));
        for (option = sh_sand_cmd->options; option->keyword != NULL; option++)
        {
            char *def_str;

            if(option->def == NULL)
            {
                def_str = "NONE";
            }
            else if(ISEMPTY(option->def))
            {
                def_str = "EMPTY";
            }
            else
            {
                def_str = option->def;
            }

            diag_sand_prt_char(margin, ' ');
            LOG_CLI((BSL_META("%s (%s:%s)\n"), option->keyword, sal_field_type_str(option->type), def_str));
            if(option->desc)
            {
                sh_sand_print(option->desc, 2 * margin, margin, columns);
            }
        }
    }

    SH_SAND_GET_BOOL("verbose", flag);
    if(flag == TRUE)
    {
        LOG_CLI((BSL_META("\nSYSTEM OPTIONS\n")));
        for (option = sh_sand_sys_options; option->keyword != NULL; option++)
        {
            diag_sand_prt_char(margin, ' ');
            LOG_CLI((BSL_META("%s (%s:%s)\n"), option->keyword, sal_field_type_str(option->type),
                                                ((option->def == NULL) ? "NA" : option->def)));
            if(option->desc)
            {
                sh_sand_print(option->desc, 2 * margin, margin, columns);
            }
        }
    }

    if (sh_sand_cmd->man->examples != NULL)
    {
        LOG_CLI((BSL_META("\nEXAMPLES\n")));
        sh_sand_print(sh_sand_cmd->man->examples, margin, margin, columns);
    }
    LOG_CLI((BSL_META("\n")));

exit:
    SHR_FUNC_EXIT;
}

static void
sh_sand_usage_node(
    sh_sand_cmd_t * sh_sand_cmd_a,
    sh_sand_control_t * sand_control)
{
    sh_sand_cmd_t *sh_sand_cmd;

    for (sh_sand_cmd = sh_sand_cmd_a; (sh_sand_cmd != NULL) && (sh_sand_cmd->keyword != NULL); sh_sand_cmd++)
    {   /*
         * Output usage for specific node
         */
        if(sh_sand_cmd->action)
        {
            sh_sand_usage_leaf(sh_sand_cmd, sand_control);
        }
        /*
         *  If sh_sand_cmd->cmd is NULL recursion will end
         */
        sh_sand_usage_node(sh_sand_cmd->cmd, sand_control);
    }
    return;
}

void
sh_sand_cmd_get(
    args_t * args)
{
    int i;
    for (i = 0; i < ARG_CUR_INDEX(args); i++)
    {

    }
}

static void
sh_sand_help_leaf(
    sh_sand_cmd_t *sh_sand_cmd)
{
    if ((sh_sand_cmd != NULL) && (sh_sand_cmd->man != NULL) && (sh_sand_cmd->man->brief != NULL))
    {
        cli_out("\t%s - %s\n", sh_sand_cmd->keyword, sh_sand_cmd->man->brief);
    }
    return;
}

static void
sh_sand_help_branch(
    sh_sand_cmd_t * sh_sand_cmd_a)
{
    sh_sand_cmd_t *sh_sand_cmd;

    cli_out("Supported commands:\n");
    cli_out("\tusage - print usage of all underlying commands\n");
    cli_out("\tall   - perform all underlying commands with default option set\n");
    for (sh_sand_cmd = sh_sand_cmd_a; sh_sand_cmd && (sh_sand_cmd->keyword != NULL); sh_sand_cmd++)
    {
        sh_sand_help_leaf(sh_sand_cmd);
    }
    return;
}

static shr_error_e
sh_sand_sys_args_init(
    sh_sand_option_t *options,
    rhlist_t **sand_args_list_p)
{
    sh_sand_option_t *option;
    rhhandle_t void_arg;
    sh_sand_arg_t *sh_sand_arg;
    rhlist_t *sand_args_list = NULL;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    if(sand_args_list_p == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_INTERNAL, "ERROR - No pointer for args list provided\n");
    }
    /* If option == NULL, no command specific options provided, just system one will be relevant */
    if ((sand_args_list = utilex_rhlist_create("Options", sizeof(sh_sand_arg_t), 1)) == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_MEMORY, "ERROR - No memory for args list\n");
    }

    /*
     * Add all options to the list
     */
    for (option = options; option->keyword != NULL; option++)
    {
        if (utilex_rhlist_entry_add_tail(sand_args_list,
                                         option->keyword, RHID_TO_BE_GENERATED, &void_arg) != _SHR_E_NONE)
        {
            SHR_CLI_EXIT(_SHR_E_INIT, "Error - failed to add system option:\"%s\" to the list\n", option->keyword);
        }
        sh_sand_arg = void_arg;
        sh_sand_arg->type = option->type;
        sh_sand_arg->ext_ptr = option->ext_ptr;
        if (option->def != NULL)
        {
            if (sh_sand_get_value(sh_sand_arg->type, option->def, &sh_sand_arg->def, sh_sand_arg->ext_ptr) != _SHR_E_NONE)
            {
                SHR_CLI_EXIT(_SHR_E_PARAM, "Default Value:\"%s\" for option:\"%s\" does not match option definition\n",
                                                                                option->def, option->keyword);
            }
        }
        else
        {
            memset(&sh_sand_arg->def, 0, sizeof(sh_sand_param_u));
        }
    }

exit:
    if((sand_args_list != NULL) && !SHR_FUNC_VAL_IS(_SHR_E_NONE))
    {
        utilex_rhlist_free_all(sand_args_list);
    }
    else
    {
        if(sand_args_list_p != NULL)
        {
            *sand_args_list_p = sand_args_list;
        }
    }
    SHR_FUNC_EXIT;
}

static shr_error_e
sh_sand_args_init(
    sh_sand_option_t *options,
    rhlist_t **sand_args_list_p)
{
    sh_sand_option_t *option;
    rhhandle_t void_arg;
    sh_sand_arg_t *sh_sand_arg;
    rhlist_t *sand_args_list = NULL;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    if(sand_args_list_p == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_INTERNAL, "ERROR - No pointer for args list provided\n");
    }
    /* If option == NULL, no command specific options provided, just system one will be relevant */
    if ((sand_args_list = utilex_rhlist_create("Options", sizeof(sh_sand_arg_t), 1)) == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_MEMORY, "ERROR - No memory for args list\n");
    }

    for (option = options; option->keyword != NULL; option++)
    {
        sh_sand_option_t *cmp_option;

        for (cmp_option = sh_sand_sys_options; cmp_option->keyword != NULL; cmp_option++)
        {
            if (!sal_strcasecmp(option->keyword, cmp_option->keyword))
            {
                SHR_CLI_EXIT(_SHR_E_INIT, "Error - option:\"%s\" is system one\n", option->keyword);
            }
        }

        SHR_CLI_EXIT_IF_ERR(sh_sand_verify_keyword(option->keyword, &option->short_key), "");
        if(ISEMPTY(option->desc))
        {
            SHR_CLI_EXIT(_SHR_E_INIT, "Error - option:\"%s\" has no description\n", option->keyword);
        }
        if((option->type == SAL_FIELD_TYPE_ENUM) && (option->ext_ptr == NULL))
        {
            SHR_CLI_EXIT(_SHR_E_INIT, "Error - enum option:\"%s\" requests enum list to be provided\n", option->keyword);
        }

        if (utilex_rhlist_entry_add_tail(sand_args_list,
                                         option->keyword, RHID_TO_BE_GENERATED, &void_arg) != _SHR_E_NONE)
        {
            SHR_CLI_EXIT(_SHR_E_INIT, "Error - failed to add option:\"%s\" to the list\n", option->keyword);
        }
        sh_sand_arg = void_arg;
        sh_sand_arg->type = option->type;
        sh_sand_arg->ext_ptr = option->ext_ptr;
        sh_sand_arg->short_key = option->short_key;
        if (option->def != NULL)
        {
            if (sh_sand_get_value(sh_sand_arg->type, option->def, &sh_sand_arg->def, sh_sand_arg->ext_ptr) != _SHR_E_NONE)
            {
                SHR_CLI_EXIT(_SHR_E_PARAM, "Illegal Default value:\"%s\" for option:\"%s\"\n",
                                                                                option->def, option->keyword);
            }
        }
        else
        {   /* Absence of default means that option presence is requested */
            sh_sand_arg->requested = TRUE;
        }
    }
    /*
     * Add all system options to the list
     */
    for (option = sh_sand_sys_options; option->keyword != NULL; option++)
    {
        if (utilex_rhlist_entry_add_tail(sand_args_list,
                                         option->keyword, RHID_TO_BE_GENERATED, &void_arg) != _SHR_E_NONE)
        {
            SHR_CLI_EXIT(_SHR_E_INIT, "Error - failed to add system option:\"%s\" to the list\n", option->keyword);
        }
        sh_sand_arg = void_arg;
        sh_sand_arg->type = option->type;
        sh_sand_arg->ext_ptr = option->ext_ptr;
        if (option->def != NULL)
        {
            if (sh_sand_get_value(sh_sand_arg->type, option->def, &sh_sand_arg->def, sh_sand_arg->ext_ptr) != _SHR_E_NONE)
            {
                SHR_CLI_EXIT(_SHR_E_PARAM, "Illegal Default value:\"%s\" for option:\"%s\"\n",
                                                                                option->def, option->keyword);
            }
        }
        else
        {   /* Absence of default means that option presence is requested */
            sh_sand_arg->requested = TRUE;
        }
    }

exit:
    if((sand_args_list != NULL) && !SHR_FUNC_VAL_IS(_SHR_E_NONE))
    {
        utilex_rhlist_free_all(sand_args_list);
    }
    else
    {
        if(sand_args_list_p != NULL)
        {
            *sand_args_list_p = sand_args_list;
        }
    }
    SHR_FUNC_EXIT;
}

shr_error_e
sh_sand_act(
    int unit,
    args_t * args,
    sh_sand_cmd_t * sh_sand_cmd_a)
{
    char *cmd_name;
    sh_sand_cmd_t *sh_sand_cmd;

    SHR_FUNC_INIT_VARS(unit);

    if (sh_sand_cmd_a == NULL)
    {
        SHR_CLI_EXIT(_SHR_E_INTERNAL, "ERROR: No further command provided\n");
    }

    if(sys_ctr.stat_args_list == NULL)
    {   /* Use this case as a sign of the fact that global resources were not initialized/verified */
        SHR_CLI_EXIT_IF_ERR(sh_sand_sys_args_init(sh_sand_sys_options, &sys_ctr.stat_args_list),
                                                            "ERROR - problem with system options\n");
        sh_sand_init_keywords();
    }
    /*
     * Get command name
     */
    if ((cmd_name = ARG_CUR(args)) == NULL)
    {   /* if no command name - print help */
        sh_sand_help_branch(sh_sand_cmd_a);
        goto exit;
    }

    if (!sal_strncasecmp(cmd_name, "help", strlen(cmd_name)))
    {   /* Don't move arg index we may want to show entire branch */
        sh_sand_help_branch(sh_sand_cmd_a);
    }
    else if (!sal_strncasecmp(cmd_name, "usage", strlen(cmd_name)))
    {   /* Print usage for the tree, options may control the output, move args after usage keyword */
        ARG_NEXT(args);
        if(sh_sand_args_process(unit, args, sys_ctr.stat_args_list, FALSE, NULL) != _SHR_E_NONE)
        {
            SHR_CLI_EXIT(_SHR_E_INTERNAL, "Erroneous system options\n");
        }
        sh_sand_usage_node(sh_sand_cmd_a, &sys_ctr);
    }
    else if (!sal_strncasecmp(cmd_name, "all", strlen(cmd_name)))
    {
        /*
         * Iterate over command array invoking all commands. In this case we don't care about errors 
         */
        for (sh_sand_cmd = sh_sand_cmd_a; sh_sand_cmd->keyword != NULL; sh_sand_cmd++)
        {
            if (sh_sand_cmd->cmd != NULL)
            {
                sh_sand_act(unit, args, sh_sand_cmd->cmd);
            }
            else if (sh_sand_cmd->action != NULL)
            {
                if (sh_sand_cmd->ctr.stat_args_list == NULL)
                {
                    if (sh_sand_args_init(sh_sand_cmd->options, &sh_sand_cmd->ctr.stat_args_list) != _SHR_E_NONE)
                    {
                        SHR_CLI_EXIT(_SHR_E_INTERNAL, "Erroneous option list for:%s\n", sh_sand_cmd->keyword);
                    }
                }
                
                SHR_CLI_EXIT_IF_ERR(sh_sand_args_process(unit, args, sh_sand_cmd->ctr.stat_args_list, sh_sand_cmd->legacy_mode, sh_sand_cmd),
                        "ERROR - problem with command options for:%s \n", sh_sand_cmd->keyword);

                sh_sand_cmd->action(unit, args, &sh_sand_cmd->ctr);
                ARG_DISCARD(args);
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        /*
         * Now we can move after command word
         */
        ARG_NEXT(args);
        /*
         * Iterate over command array looking for command
         */
        for (sh_sand_cmd = sh_sand_cmd_a; sh_sand_cmd->keyword != NULL; sh_sand_cmd++)
        {
            /*
             * Temp for non-jr2 devices
             */
            if(sh_sand_cmd->short_key == NULL)
            {
                sh_sand_verify_keyword(sh_sand_cmd->keyword, &sh_sand_cmd->short_key);
            }

            /*
             * Call to the first command that is supported and match the cmd_name
             */
            if (!sal_strcasecmp(cmd_name, sh_sand_cmd->keyword)
                || ((sh_sand_cmd->short_key != NULL) && (!sal_strcasecmp(cmd_name, sh_sand_cmd->short_key))))
            {
                if (sh_sand_cmd->cmd != NULL)
                {
                    SHR_SET_CURRENT_ERR(sh_sand_act(unit, args, sh_sand_cmd->cmd));
                }
                else if (sh_sand_cmd->action != NULL) /* Currently either leaf or node, it may be both */
                {
                    char *next_cmd_name = ARG_CUR(args);
                    if ((next_cmd_name != NULL) && !sal_strncasecmp(next_cmd_name, "usage", strlen(next_cmd_name)))
                    {   /* Move after usage to see options */
                        ARG_NEXT(args);
                        if(sh_sand_args_process(unit, args, sys_ctr.stat_args_list, FALSE, NULL) != _SHR_E_NONE)
                        {
                            SHR_CLI_EXIT(_SHR_E_INTERNAL, "Erroneous system options for %s usage\n", sh_sand_cmd->keyword);
                        }
                        sh_sand_usage_leaf(sh_sand_cmd, &sys_ctr);
                    }
                    else if((next_cmd_name != NULL) && !sal_strncasecmp(next_cmd_name, "help", strlen(next_cmd_name)))
                    {   /* Just print help, help does nor require options, so ignore them*/
                        sh_sand_help_leaf(sh_sand_cmd);
                    }
                    else
                    {   /* Initialize arguments list if not already*/
                        if (sh_sand_cmd->ctr.stat_args_list == NULL)
                        {
                            if (sh_sand_args_init(sh_sand_cmd->options, &sh_sand_cmd->ctr.stat_args_list) != _SHR_E_NONE)
                            {
                                SHR_CLI_EXIT(_SHR_E_INTERNAL, "Erroneous option list for:%s\n", sh_sand_cmd->keyword);
                            }
                        }
                        /*
                         * Parse options
                         */
                        if ((SHR_SET_CURRENT_ERR(sh_sand_args_process(unit, args, sh_sand_cmd->ctr.stat_args_list, sh_sand_cmd->legacy_mode, sh_sand_cmd))) == _SHR_E_NONE)
                        {
                            SHR_SET_CURRENT_ERR(sh_sand_cmd->action(unit, args, &sh_sand_cmd->ctr));
                        }
                    }
                    ARG_DISCARD(args);
                    /*
                     * If usage error -> print usage info (if exist)
                     */
                    if (SHR_FUNC_VAL_IS(_SHR_E_PARAM))
                    {
                        sh_sand_usage_leaf(sh_sand_cmd, &sys_ctr);
                    }
                }
                else
                {
                    SHR_CLI_EXIT(_SHR_E_INTERNAL, "ERROR: command:%s had neither leaf nor branch\n", cmd_name);
                }
                goto exit;
            }
        }
        /*
         * command name not found - print usage
         */
        LOG_CLI((BSL_META("ERROR: command\"%s\" is not supported\n"), cmd_name));
        sh_sand_help_branch(sh_sand_cmd_a);
    }

exit:
    SHR_FUNC_EXIT;
}

static shr_error_e
sh_sand_verify_man(char *command, sh_sand_man_t *man)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);
    if((man == NULL) || (man->brief == NULL) || (man->full == NULL) || (man->examples == NULL) || (man->synopsis == NULL))
    {
        LOG_CLI((BSL_META("Bad \"usage\" for command:%s\n"), command));
        SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
    }

    SHR_FUNC_EXIT;
}

shr_error_e
sh_sand_verify(
    int unit,
    sh_sand_cmd_t * sh_sand_cmd_a,
    char *command)
{
    sh_sand_cmd_t *sh_sand_cmd;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    if(sys_ctr.stat_args_list == NULL)
    {
        SHR_CLI_EXIT_IF_ERR(sh_sand_sys_args_init(sh_sand_sys_options, &sys_ctr.stat_args_list),
                                                    "ERROR - problem with system options\n");
        sh_sand_init_keywords();
    }
    /*
     * Iterate over command array 
     */
    for (sh_sand_cmd = sh_sand_cmd_a; sh_sand_cmd->keyword != NULL; sh_sand_cmd++)
    {   /* Verify that every command is either leaf or branch */
        /*
         * Keep previous command length 
         */
        int cmd_size = strlen(command);
        /*
         * Include present keyword 
         */
        if (cmd_size == 0)
            sal_strcpy(command, sh_sand_cmd->keyword);
        else
            sal_sprintf(command + cmd_size, " %s", sh_sand_cmd->keyword);
        /*
         * Obtain shortcut string
         * Meanwhile do not fail the run - just warn
         */
        sh_sand_verify_keyword(sh_sand_cmd->keyword, &sh_sand_cmd->short_key);

        if ((sh_sand_cmd->cmd == NULL) && (sh_sand_cmd->action == NULL))
        {
            LOG_CLI((BSL_META("ERROR - command:%s is neither leaf nor branch\n"), command));
            SHR_SET_CURRENT_ERR(_SHR_E_INTERNAL);
        }
        else
        {
            if (sh_sand_cmd->action)
            {   /* Command is leaf, verify that we have correct usage and options */
                SHR_CLI_EXIT_IF_ERR(sh_sand_verify_man(command, sh_sand_cmd->man), "");
                SHR_CLI_EXIT_IF_ERR(sh_sand_args_init(sh_sand_cmd->options, &sh_sand_cmd->ctr.stat_args_list),
                                                    "Erroneous option list for command:\"%s\"\n", command);
            }
            if (sh_sand_cmd->cmd)
            {   /* Command is branch got into recursive call */
                SHR_CLI_EXIT_IF_ERR(sh_sand_verify(unit, sh_sand_cmd->cmd, command), "");
            }
        }
        /*
         * Return previous command - cut off current keyword 
         */
        command[cmd_size] = 0;
    }

exit:
    SHR_FUNC_EXIT;
}
