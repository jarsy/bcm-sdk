/*
 * $Id: t3p1_cmds.c,v 1.1.2.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        t3p1_cmds.c
 * Purpose:     Caladan3 diag shell commands commands for ucode objects
 * Requires:
 *
 */
#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/cm.h>

#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_T3P1_SUPPORT)
#include <appl/diag/system.h>
#include <soc/types.h>
#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/t3p1/t3p1_int.h>
#include <soc/sbx/t3p1/t3p1_diags.h>
#include <soc/sbx/t3p1/t3p1_tmu_diags.h>
#include <soc/sbx/t3p1/t3p1_cop_diags.h>
#include <soc/sbx/t3p1/t3p1_cmu_diags.h>
#include <soc/sbx/t3p1/t3p1_ppe_diags.h>
#include <soc/sbx/caladan3/rce.h>
#include <bcm/error.h>

cmd_result_t
cmd_sbx_t3p1_get (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_print(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_set (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_set(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_delete (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_delete(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_global_get (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_print_globals(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_global_set (int unit, args_t *args)
{
    int rv;
    char *p;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_set_globals(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        p = ARG_GET(args);
        if (p && sal_strncasecmp(p,"higig_loop_enable", 
                     sal_strlen("higig_loop_enable")-1) == 0) {
            uint32 en = 0;
            soc_sbx_control_t *sbx;

            sbx = SOC_SBX_CONTROL(unit);
            p = ARG_GET(args);
            if (p)
                soc_sbx_t3p1_read(p, 2, &en);
            if (en) {   
                soc_sbx_caladan3_sws_pr_icc_program_sirius_header(unit, 1, 0, 0, 0);
            } else {
                if ((sal_strlen(sbx->uver_name) == 5) &&
                    (sal_strncasecmp(sbx->uver_name, "t3p1a", 5)==0)) {
                    soc_sbx_caladan3_sws_pr_icc_program_arad_header(unit, 0);
                } else {
                    soc_sbx_caladan3_sws_pr_icc_program_sirius_header(unit, 0, 0, 0, 0);
                }
            }
        }
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}
cmd_result_t
cmd_sbx_t3p1_mem_get (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_print_memories(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_mem_set (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_set_memories(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}
cmd_result_t
cmd_sbx_t3p1_constant_get (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_shell_print_constants(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}


cmd_result_t
cmd_sbx_t3p1_tmu_get (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_tmu_shell_print(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_tmu_set (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_tmu_shell_set(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_tmu_delete (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_tmu_shell_delete(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_ppe_get (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_ppe_shell_print(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_ppe_set (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_ppe_shell_set(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_cmu_get (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_cmu_shell_print(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_cmu_reset (int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_cmu_shell_reset(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_policer_add(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 7) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_policer_add(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_policer_remove(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_policer_remove(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}
 
cmd_result_t
cmd_sbx_t3p1_policer_remove_all(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_policer_remove_all(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_timer_add(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 2) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_timer_add(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}   

cmd_result_t
cmd_sbx_t3p1_timer_remove(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_timer_remove(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}   

cmd_result_t
cmd_sbx_t3p1_timer_remove_all(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_timer_remove_all(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_sequencer_add(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 2) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_sequencer_add(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}   

cmd_result_t
cmd_sbx_t3p1_sequencer_remove(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_sequencer_remove(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_sequencer_remove_all(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }
    
    rv = soc_sbx_t3p1_cop_shell_sequencer_remove_all(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {       
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_coherent_table_add(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 2) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_cop_shell_coherent_table_add(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}  

cmd_result_t
cmd_sbx_t3p1_coherent_table_remove(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_cop_shell_coherent_table_remove(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_coherent_table_remove_all(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 1) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_cop_shell_coherent_table_remove_all(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_coherent_table_get(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 2) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_cop_shell_coherent_table_get(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_t3p1_coherent_table_set(int unit, args_t *args)
{
    int rv;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) < 4) {
        return CMD_USAGE;
    }

    rv = soc_sbx_t3p1_cop_shell_coherent_table_set(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}


cmd_result_t 
_cmd_t3p1_util_constants(int unit, args_t *args)
{
    int i;

    if (ARG_CNT(args)) {
        int rv, value, nFound = 0;
        soc_sbx_t3p1_state_t *fe =
            (soc_sbx_t3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
        soc_sbx_t3p1_table_manager_t *tm = fe->tableinfo;
        
        value = parse_integer(ARG_GET(args));

        for (i = 0; i < SOC_SBX_T3P1_CONSTANT_MAX_ID; i++) {
            uint32 constVal;

            rv = soc_sbx_t3p1_constant_get(unit, tm->constants[i].name,
                                           &constVal);
            if (SOC_SUCCESS(rv)) {
                if (value == constVal) {
                    soc_sbx_t3p1_constant_shell_print(unit, i);
                    nFound++;
                }
            }
        }
        cli_out("Found %d constants with value %d\n", nFound, value);
        return CMD_OK;
    }

    for (i = 0; i < SOC_SBX_T3P1_CONSTANT_MAX_ID; i++) {
        soc_sbx_t3p1_constant_shell_print(unit, i);
    }

    return CMD_OK;
}

char cmd_sbx_t3p1_util_usage[] =
"Usage:\n"
"  32p1util help               - displays this messge\n"
"  t3p1util constants [val]             - displays constant names matching an integer value, all if no value supplied\n"
"  t3p1util route [add|delete] [params] - Add or delete Ipv4 routes\n"
"  t3p1util field [add|delete] [params] - Add or delete field entries\n"
"  t3p1util allocator                   - displays SDK table allocations\n";

cmd_result_t 
_cmd_t3p1_util_route(int unit, args_t *args)
{
    char            *subcmd;
    char            *cmd;
    uint32          c3_unit = 0;
    uint32          route_start = 0x01010000;
    uint32          route_mask = 0xffffff00;
    uint32          route_flags = 0;
    uint32          route_inc = 0x100;
    uint32          route_count = 10000;
    uint32          batch_size = 0;
    uint32          fte_count = 0;
    uint32          fte[8];
    bcm_l3_route_t  route;
    int             i;
    int             rc;
    sal_usecs_t     start_time, end_time;
    float           seconds;

    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: t3p1 cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "add") == 0) {

        while ((cmd = ARG_GET(args)) != NULL) {
            if (!sal_strncasecmp(cmd, "c3-unit", sal_strlen("c3-unit"))) {
                cmd += sal_strlen("c3-unit") + 1;
                c3_unit = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "route-start", sal_strlen("route-start"))) {
                cmd += sal_strlen("route-start") + 1;
                route_start = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "route-mask", sal_strlen("route-mask"))) {
                cmd += sal_strlen("route-mask") + 1;
                route_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "route-flags", sal_strlen("route-flags"))) {
                cmd += sal_strlen("route-flags") + 1;
                route_flags = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "route-inc", sal_strlen("route-inc"))) {
                cmd += sal_strlen("route-inc") + 1;
                route_inc = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "route-count", sal_strlen("route-count"))) {
                cmd += sal_strlen("route-count") + 1;
                route_count = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "batch-size", sal_strlen("batch-size"))) {
                cmd += sal_strlen("batch-size") + 1;
                batch_size = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "fte", sal_strlen("fte"))) {
                if (fte_count >= 8) {
                    cli_out("Too many fte entries, max is eight\n");
                    return CMD_FAIL;
                }
                cmd += sal_strlen("fte") + 1;
                fte[fte_count++] = strtoul(cmd, NULL, 0);
            }
        }

        cli_out("c3-unit: %u\n", c3_unit);
        cli_out("route-start: 0x%8.8x\n", route_start);
        cli_out("route-mask: 0x%8.8x\n", route_mask);
        cli_out("route-flags: 0x%8.8x\n", route_flags);
        cli_out("route-inc: %u\n", route_inc);
        cli_out("route-count: %u\n", route_count);
        cli_out("batch-size: %u\n", batch_size);
        cli_out("fte-count: %u\n", fte_count);

        if (fte_count == 0) {
            cli_out("Invalid operation. An FTE must be defined.\n");
            return CMD_FAIL;
        }
 
        for (i = 0; i < fte_count; i++) {
            cli_out("fte[%d]: 0x%x\n", i, fte[i]);
        }

        /* Enable batching if requested */
        rc = bcm_switch_control_set(c3_unit, bcmSwitchL3RouteCache, batch_size > 0 ? TRUE : FALSE);
        if (rc != BCM_E_NONE) {
            printf("bcm_switch_control_set, bcmSwitchL3RouteCache, failed %s\n",
                bcm_errmsg(rc));
        }

        /* Install/delete the routes */
        start_time = sal_time_usecs();
        for (i = 0; i < route_count; i++) {
            bcm_l3_route_t_init(&route);
            route.l3a_subnet = route_start;
            route.l3a_ip_mask = route_mask;
            route.l3a_flags = route_flags;
            route.l3a_intf = fte[i%fte_count];
            rc = bcm_l3_route_add(c3_unit, &route);
            if (rc != BCM_E_NONE) {

                cli_out("Failed to install route %d: %s\n", i, bcm_errmsg(rc));

                /* If batching flush last installed set of routes */
                /* coverity[dead_error_begin] */
                if ((batch_size != 0) && (i != 0)) {
                    rc = bcm_switch_control_set(c3_unit, bcmSwitchL3RouteCommit, TRUE);
                    if (rc != BCM_E_NONE) {
                        printf("bcm_switch_control_set, bcmSwitchL3RouteCommit, failed %s\n",
                            bcm_errmsg(rc));
                    }
                }

                end_time = sal_time_usecs();
                seconds = (float)(end_time - start_time) / 1000000.0;
                cli_out("Installed %u routes in %u micro-seconds, %f routes per second\n",
                        i, end_time-start_time,
                        (float)i / seconds);

                return CMD_FAIL;

            }

            /* If batching write routes */
            if (((batch_size != 0) && (i != 0) && (i % batch_size)) == 0) {
                rc = bcm_switch_control_set(c3_unit, bcmSwitchL3RouteCommit, TRUE);
                if (rc != BCM_E_NONE) {
                    printf("bcm_switch_control_set, bcmSwitchL3RouteCommit, failed %s\n",
                        bcm_errmsg(rc));
                }
            }

            route_start += route_inc;
        }
        /* If batching write last routes */
        if (batch_size != 0) {
            rc = bcm_switch_control_set(c3_unit, bcmSwitchL3RouteCommit, TRUE);
            if (rc != BCM_E_NONE) {
                printf("bcm_switch_control_set, bcmSwitchL3RouteCommit, failed %s\n",
                    bcm_errmsg(rc));
            }
        }

        end_time = sal_time_usecs();
        seconds = (float)(end_time - start_time) / 1000000.0;
        cli_out("Installed %u routes in %u micro-seconds, %f routes per second\n",
                route_count, end_time-start_time,
                (float)route_count / seconds);
    }


    return CMD_OK;

}

static void _mac_fill(uint8 *out, uint64 in)
{
    int     i;

    for (i = 0; i < 6; i++) {
        out[5-i] = COMPILER_64_LO(in) & 0xff;
        COMPILER_64_SHR(in, 8);
    }
}

static void _printk_mac(char *prefix, uint8 *mac, char *postfix)
{
    int     i;

    cli_out(prefix);
    for (i = 0; i < 6; i++) {
        if (i < 5) {
            cli_out("%2.2x:", mac[i]);
        } else {
            cli_out("%2.2x", mac[i]);
        }
    }
    cli_out(postfix);

}

cmd_result_t 
_cmd_t3p1_util_field(int unit, args_t *args)
{
    char                *subcmd;
    char                *cmd;
    uint32              c3_unit = 0;
    int                 modid = 0;
    uint32              groupid = 0;
    uint32              entryid_start = 0;
    uint32              entry_count = 0;
    int                 qualifyDmac = FALSE;
    uint64              dmacll = COMPILER_64_INIT(0,0);
    bcm_mac_t           dmac;
    uint64              dmac_maskll = COMPILER_64_INIT(0,0);
    bcm_mac_t           dmac_mask;
    uint32              dmac_inc = 0;
    int                 qualifySmac = FALSE;
    uint64              smacll = COMPILER_64_INIT(0,0);
    bcm_mac_t           smac;
    uint64              smac_maskll = COMPILER_64_INIT(0,0);
    bcm_mac_t           smac_mask;
    uint32              smac_inc = 0;
    int                 qualifyEtherType = FALSE;
    uint32              ethertype = 0;
    uint32              ethertype_mask = 0;
    int                 qualifyVlan = FALSE;
    uint32              vlan = 0;
    uint32              vlan_mask = 0;
    int                 qualifyDstIp = FALSE;
    uint32              dstip = 0;
    uint32              dstip_mask = 0;
    uint32              dstip_inc = 0;
    int                 qualifySrcIp = FALSE;
    uint32              srcip = 0;
    uint32              srcip_mask = 0;
    uint32              srcip_inc = 0;
    int                 qualifyIpProto = FALSE;
    uint32              ipproto = 0;
    uint32              ipproto_mask = 0;
    int                 qualifyL4DstPort = FALSE;
    uint32              l4dstport = 0;
    uint32              l4dstport_mask = 0;
    int                 qualifyL4SrcPort = FALSE;
    uint32              l4srcport = 0;
    uint32              l4srcport_mask = 0;
    int                 qualifyDscp = FALSE;
    uint32              dscp = 0;
    uint32              dscp_mask = 0;
    int                 qualifyInPort = FALSE;
    uint32              inport = 0;
    uint32              inport_mask = 0;
    int                 qualifyOutPort = FALSE;
    uint32              outport = 0;
    uint32              outport_mask = 0;
    int                 actionMirror = FALSE;
    uint32              mirror = 0;
    int                 actionDrop = FALSE;
    uint32              i;
    bcm_field_entry_t   entryId;
    int                 rc;

    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: t3p1 cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    _mac_fill((uint8*)&dmac, dmacll);
    _mac_fill((uint8*)&dmac_mask, dmac_maskll);
    _mac_fill((uint8*)&smac, smacll);
    _mac_fill((uint8*)&smac_mask, smac_maskll);

    if (sal_strcasecmp(subcmd, "add") == 0) {

        while ((cmd = ARG_GET(args)) != NULL) {
            if (!sal_strncasecmp(cmd, "c3-unit", sal_strlen("c3-unit"))) {
                cmd += sal_strlen("c3-unit") + 1;
                c3_unit = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "groupid", sal_strlen("groupid"))) {
                cmd += sal_strlen("groupid") + 1;
                groupid = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "entryid-start", sal_strlen("entryid-start"))) {
                cmd += sal_strlen("entryid-start") + 1;
                entryid_start = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "entry-count", sal_strlen("entry-count"))) {
                cmd += sal_strlen("entry-count") + 1;
                entry_count = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "dmac-mask", sal_strlen("dmac-mask"))) {
                cmd += sal_strlen("dmac-mask") + 1;
                dmac_maskll = parse_uint64(cmd);;
                _mac_fill((uint8*)&dmac_mask, dmac_maskll);
            } else if (!sal_strncasecmp(cmd, "dmac-inc", sal_strlen("dmac-inc"))) {
                cmd += sal_strlen("dmac-inc") + 1;
                dmac_inc = parse_integer(cmd);
            } else if (!sal_strncasecmp(cmd, "dmac", sal_strlen("dmac"))) {
                cmd += sal_strlen("dmac") + 1;
                dmacll = parse_uint64(cmd);
                _mac_fill((uint8*)&dmac, dmacll);
                qualifyDmac = TRUE;
            } else if (!sal_strncasecmp(cmd, "smac-mask", sal_strlen("smac-mask"))) {
                cmd += sal_strlen("smac-mask") + 1;
                smac_maskll = parse_uint64(cmd);
                _mac_fill((uint8*)&smac_mask, smac_maskll);
            } else if (!sal_strncasecmp(cmd, "smac-inc", sal_strlen("smac-inc"))) {
                cmd += sal_strlen("smac-inc") + 1;
                smac_inc = parse_integer(cmd);
            } else if (!sal_strncasecmp(cmd, "smac", sal_strlen("smac"))) {
                cmd += sal_strlen("smac") + 1;
                smacll = parse_uint64(cmd);
                _mac_fill((uint8*)&smac, smacll);
                qualifySmac = TRUE;
            } else if (!sal_strncasecmp(cmd, "ethertype-mask", sal_strlen("ethertype-mask"))) {
                cmd += sal_strlen("ethertype-mask") + 1;
                ethertype_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "ethertype", sal_strlen("ethertype"))) {
                cmd += sal_strlen("ethertype") + 1;
                ethertype = strtoul(cmd, NULL, 0);
                qualifyEtherType = TRUE;
            } else if (!sal_strncasecmp(cmd, "vlan-mask", sal_strlen("vlan-mask"))) {
                cmd += sal_strlen("vlan-mask") + 1;
                vlan_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "vlan", sal_strlen("vlan"))) {
                cmd += sal_strlen("vlan") + 1;
                vlan = strtoul(cmd, NULL, 0);
                qualifyVlan = TRUE;
            } else if (!sal_strncasecmp(cmd, "dstip-start", sal_strlen("dstip-start"))) {
                cmd += sal_strlen("dstip-start") + 1;
                dstip = strtoul(cmd, NULL, 0);
                qualifyDstIp = TRUE;
            } else if (!sal_strncasecmp(cmd, "dstip-mask", sal_strlen("dstip-mask"))) {
                cmd += sal_strlen("dstip-mask") + 1;
                dstip_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "dstip-inc", sal_strlen("dstip-inc"))) {
                cmd += sal_strlen("dstip-inc") + 1;
                dstip_inc = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "srcip-start", sal_strlen("srcip-start"))) {
                cmd += sal_strlen("srcip-start") + 1;
                srcip = strtoul(cmd, NULL, 0);
                qualifySrcIp = TRUE;
            } else if (!sal_strncasecmp(cmd, "srcip-mask", sal_strlen("srcip-mask"))) {
                cmd += sal_strlen("srcip-mask") + 1;
                srcip_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "srcip-inc", sal_strlen("srcip-inc"))) {
                cmd += sal_strlen("srcip-inc") + 1;
                srcip_inc = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "ipproto-mask", sal_strlen("ipproto-mask"))) {
                cmd += sal_strlen("ipproto-mask") + 1;
                ipproto_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "ipproto", sal_strlen("ipproto"))) {
                cmd += sal_strlen("ipproto") + 1;
                ipproto = strtoul(cmd, NULL, 0);
                qualifyIpProto = TRUE;
            } else if (!sal_strncasecmp(cmd, "l4dstport-mask", sal_strlen("l4dstport-mask"))) {
                cmd += sal_strlen("l4dstport-mask") + 1;
                l4dstport_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "l4dstport", sal_strlen("l4dstport"))) {
                cmd += sal_strlen("l4dstport") + 1;
                l4dstport = strtoul(cmd, NULL, 0);
                qualifyL4DstPort = TRUE;
            } else if (!sal_strncasecmp(cmd, "l4srcport-mask", sal_strlen("l4srcport-mask"))) {
                cmd += sal_strlen("l4srcport-mask") + 1;
                l4srcport_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "l4srcport", sal_strlen("l4srcport"))) {
                cmd += sal_strlen("l4srcport") + 1;
                l4srcport = strtoul(cmd, NULL, 0);
                qualifyL4SrcPort = TRUE;
            } else if (!sal_strncasecmp(cmd, "dscp-mask", sal_strlen("dscp-mask"))) {
                cmd += sal_strlen("dscp-mask") + 1;
                dscp_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "dscp", sal_strlen("dscp"))) {
                cmd += sal_strlen("dscp") + 1;
                dscp = strtoul(cmd, NULL, 0);
                qualifyDscp = TRUE;
            } else if (!sal_strncasecmp(cmd, "inport-mask", sal_strlen("inport-mask"))) {
                cmd += sal_strlen("inport-mask") + 1;
                inport_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "inport", sal_strlen("inport"))) {
                cmd += sal_strlen("inport") + 1;
                inport = strtoul(cmd, NULL, 0);
                qualifyInPort = TRUE;
            } else if (!sal_strncasecmp(cmd, "outport-mask", sal_strlen("outport-mask"))) {
                cmd += sal_strlen("outport-mask") + 1;
                outport_mask = strtoul(cmd, NULL, 0);
            } else if (!sal_strncasecmp(cmd, "outport", sal_strlen("outport"))) {
                cmd += sal_strlen("outport") + 1;
                outport = strtoul(cmd, NULL, 0);
                qualifyOutPort = TRUE;
            } else if (!sal_strncasecmp(cmd, "mirror", sal_strlen("mirror"))) {
                cmd += sal_strlen("mirror") + 1;
                mirror = strtoul(cmd, NULL, 0);
                actionMirror = TRUE;
            } else if (!sal_strncasecmp(cmd, "drop", sal_strlen("drop"))) {
                cmd += sal_strlen("drop") + 1;
                actionDrop = TRUE;
            }
        }

        cli_out("c3-unit:\t%u\n", c3_unit);
        cli_out("groupid:\t%u\n", groupid);
        cli_out("entryid-start:\t%u\n", entryid_start);
        cli_out("entry-count:\t%u\n", entry_count);
        _printk_mac("dmac:\t\t", (uint8*)&dmac, "\n");
        _printk_mac("dmac-mask:\t", (uint8*)&dmac_mask, "\n");
        cli_out("dmac-inc:\t%u\n", dmac_inc);
        _printk_mac("smac:\t\t", (uint8*)&smac, "\n");
        _printk_mac("smac-mask:\t", (uint8*)&smac_mask, "\n");
        cli_out("smac-inc:\t%u\n", smac_inc);
        cli_out("ethertype:\t%4.4x\n", ethertype);
        cli_out("ethertype-mask:\t0x%4.4x\n", ethertype_mask);
        cli_out("vlan:\t\t%u\n", vlan);
        cli_out("vlan-mask:\t0x%4.4x\n", vlan_mask);
        cli_out("dstip-start:\t0x%8.8x\n", dstip);
        cli_out("dstip-mask:\t0x%8.8x\n", dstip_mask);
        cli_out("dstip-inc:\t%u\n", dstip_inc);
        cli_out("srcip-start:\t0x%8.8x\n", srcip);
        cli_out("srcip-mask:\t0x%8.8x\n", srcip_mask);
        cli_out("srcip-inc:\t%u\n", srcip_inc);
        cli_out("ipproto:\t%u\n", ipproto);
        cli_out("ipproto-mask:\t0x%2.2x\n", ipproto_mask);
        cli_out("l4dstport:\t%u\n", l4dstport);
        cli_out("l4dstport-mask:\t0x%4.4x\n", l4dstport_mask);
        cli_out("l4srcport:\t%u\n", l4srcport);
        cli_out("l4srcport-mask:\t0x%4.4x\n", l4srcport_mask);
        cli_out("dscp:\t\t0x%2.2x\n", dscp);
        cli_out("dscp-mask:\t0x%2.2x\n", dscp_mask);
        cli_out("inport:\t\t0x%2.2x\n", inport);
        cli_out("inport-mask:\t0x%2.2x\n", inport_mask);
        cli_out("outport:\t0x%2.2x\n", outport);
        cli_out("outport-mask:\t0x%2.2x\n", outport_mask);
        cli_out("mirror:\t\t0x%8.8x\n", mirror);
        cli_out("drop:\t\t%s\n", actionDrop ? "True" : "False");

        entryId = entryid_start;
        for (i = 0; i < entry_count; i++) {

            /* Create entry */
            rc = bcm_field_entry_create_id(c3_unit, groupid, entryId);
            if (rc != BCM_E_NONE) {
                cli_out("Failed to create entry\n");
                return CMD_FAIL;
            }

            /*
             * Add qualifiers
             */

            /* Destinatin MAC */
            if (qualifyDmac) {
                _mac_fill((uint8*)&dmac, dmacll);
                /* _printk_mac("Qualify dmac ", (uint8*)&dmac, "\n"); */
                rc = bcm_field_qualify_DstMac(c3_unit, entryId, dmac, dmac_mask);
                if (rc != BCM_E_NONE) {
                    printf("field_setup: bcm_field_qualify_DstMac failed: %s\n", bcm_errmsg(rc));
                    return rc;
                }
                COMPILER_64_ADD_32(dmacll, dmac_inc);
            }

            /* Source MAC */
            if (qualifySmac) {
                _mac_fill((uint8*)&smac, smacll);
                /* _printk_mac("Qualify smac ", (uint8*)&smac, "\n"); */
                rc = bcm_field_qualify_SrcMac(c3_unit, entryId, smac, smac_mask);
                if (rc != BCM_E_NONE) {
                    printf("field_setup: bcm_field_qualify_SstMac failed: %s\n", bcm_errmsg(rc));
                    return rc;
                }
                COMPILER_64_ADD_32(smacll, smac_inc);
            }

            /* Ether type */
            if (qualifyEtherType) {
                rc = bcm_field_qualify_EtherType(c3_unit, entryId, ethertype, ethertype_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify ether type: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /* Vlan */
            if (qualifyVlan) {
                rc = bcm_field_qualify_OuterVlan(c3_unit, entryId, vlan, vlan_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify vlan: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /* Destination IP */
            if (qualifyDstIp) {
                /* cli_out("Qualify dstip 0x%8.8x\n", dstip); */
                rc = bcm_field_qualify_DstIp(c3_unit, entryId, dstip, dstip_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify dest IP: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
                dstip += dstip_inc;
            }

            /* Source IP */
            if (qualifySrcIp) {
                /* cli_out("Qualify srcip 0x%8.8x\n", srcip); */
                rc = bcm_field_qualify_SrcIp(c3_unit, entryId, srcip, srcip_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify src IP: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
                srcip += srcip_inc;
            }

            /* IP protocol */
            if (qualifyIpProto) {
                rc = bcm_field_qualify_IpProtocol(c3_unit, entryId, ipproto, ipproto_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify IP protocol: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /* L4 dest port */
            if (qualifyL4DstPort) {
                rc = bcm_field_qualify_L4DstPort(c3_unit, entryId, l4dstport, l4dstport_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify l4 dst port: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /* L4 src port */
            if (qualifyL4SrcPort) {
                rc = bcm_field_qualify_L4SrcPort(c3_unit, entryId, l4srcport, l4srcport_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify l4 src port: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /* TOS/DSCP */
            if (qualifyDscp) {
                rc = bcm_field_qualify_DSCP(c3_unit, entryId, dscp, dscp_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify dscp: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /* In port */
            if (qualifyInPort) {
                rc = bcm_field_qualify_InPort(c3_unit, entryId, inport, inport_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify in port: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /* Out port */
            if (qualifyOutPort) {
                rc = bcm_field_qualify_OutPort(c3_unit, entryId, outport, outport_mask);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to qualify out port: %s\n", bcm_errmsg(rc));
                    return CMD_FAIL;
                }
            }

            /*
             * Add action
             */

            /* Ingress mirror */
            if (actionMirror) {
                rc = bcm_field_action_add(c3_unit, entryId,
                    bcmFieldActionMirrorIngress,
                    modid, mirror);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to add mirror action\n");
                    return CMD_FAIL;
                }
            }

            /* Drop */
            if (actionDrop) {
                rc = bcm_field_action_add(c3_unit, entryId,
                    bcmFieldActionDrop,
                    modid, mirror);
                if (rc != BCM_E_NONE) {
                    cli_out("Failed to add drop action\n");
                    return CMD_FAIL;
                }
            }

            /* Install entry */
            rc = bcm_field_entry_install(c3_unit, entryId);
            if (rc != BCM_E_NONE) {
                cli_out("Failed to install entry\n");
                return CMD_FAIL;
            }

            /* Increment entry id */
            entryId++;
        }

    }

    return CMD_OK;

}

extern int soc_sbx_allocator_shell_print(int unit);

cmd_result_t
cmd_sbx_t3p1_util(int unit, args_t *a)
{
    char *cmd, *subcmd;
    cmd_result_t rv = CMD_OK;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an Caladan3.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    cmd = ARG_CMD(a);
    if (!sh_check_attached(cmd, unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_T3P1) {
        cli_out("ERROR: not running t3p1 microcode\n");
        return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "help") == 0) {
        rv = CMD_USAGE;
    }
    else if (sal_strcasecmp(subcmd, "constants") == 0) {
        return _cmd_t3p1_util_constants(unit, a);
    } else if (!sal_strcasecmp(subcmd, "allocator")) {
        return soc_sbx_allocator_shell_print(unit);
    } else if (sal_strcasecmp(subcmd, "route") == 0) {
        return _cmd_t3p1_util_route(unit, a);
    } else if (sal_strcasecmp(subcmd, "field") == 0) {
        return _cmd_t3p1_util_field(unit, a);
    } else {
        return CMD_USAGE;
    }

    return rv;
}

#define RCE_DATA_SIZE 64
static int
_show_rce_entry_qualify(int unit, int entryId, unsigned int qualIdx, int length)
{
    unsigned char data[RCE_DATA_SIZE] = {0};
    unsigned char mask[RCE_DATA_SIZE] = {0};
    int i = 0;
    int result = SOC_E_NONE;

    result = soc_c3_rce_entry_qualify_get(unit, entryId, qualIdx, data, mask);
    if (SOC_E_NONE == result) {
        cli_out("0x");
        for (i = length-1; i >= 0; i--) {
            cli_out("%02X", data[i]);
        }
        cli_out("/0x");
        for (i = length-1; i >= 0; i--) {
            cli_out("%02X", mask[i]);
        }
        cli_out("\n");
    }
    return result;
}

static int
_show_rce_entry_action(int unit, int entryId, int actionIdx)
{
    unsigned char data[RCE_DATA_SIZE] = {0};
    int i = 0, j = 0;
    int length = 0;
    soc_c3_rce_actiontable_desc_t *actionInfo = NULL;
    int result = SOC_E_NONE;

    if (actionIdx >= SOC_C3_RCE_PROGRAM_ACTION_MAX) {
        return SOC_E_PARAM;
    }

    result = soc_c3_rce_actiontable_info_get(unit, actionIdx, &actionInfo);
    if (SOC_E_NONE != result) {
        return SOC_E_PARAM;
    }
    cli_out("\tResult action %d (%s): ", actionIdx, actionInfo->tableName);
    for (i = 0; i < actionInfo->actionCount; i++) {
        sal_memset(data, 0, RCE_DATA_SIZE);
        result = soc_c3_rce_entry_action_get(unit, entryId, i, data);
        if (SOC_E_NONE != result) {
            continue;
        }
        length = (actionInfo->actFields[i].loc[0].numBits + 7) >> 3;
        cli_out("%s = 0x", actionInfo->actFields[i].actionName);
        for (j = length-1; j >= 0; j--) {
            cli_out("%02X", data[j]);
        }
        if (i < actionInfo->actionCount - 1) {
            cli_out(", ");
        }
    }
    cli_out("\n");

    soc_c3_rce_actiontable_info_free(unit, actionInfo);
    return result;
}

static int
_show_rce_entry(int unit, int programId, int groupId, int entryId, void *extra)
{
    soc_c3_rce_entry_desc_t *entryInfo = NULL;
    soc_c3_rce_group_desc_t *groupInfo = NULL;
    soc_c3_rce_program_desc_t *programInfo = NULL;
    int length = 0;
    int i = 0;
    int result = SOC_E_NONE;

    if ((0 > unit) || (SOC_MAX_NUM_DEVICES <= unit)) {
        return SOC_E_UNIT;
    }

    do {
        result = soc_c3_rce_entry_info_get(unit, entryId, &entryInfo);
        if (SOC_E_NONE != result) {
            break;
        }

        result = soc_c3_rce_group_info_get(unit, entryInfo->groupId, &groupInfo);
        if (SOC_E_NONE != result) {
            break;
        }

        if (programId != groupInfo->rceProgram) {
            break;
        }

        if (groupId != entryInfo->groupId) {
            break;
        }

        result = soc_c3_rce_program_info_get(unit, groupInfo->rceProgram, &programInfo);
        if (SOC_E_NONE != result) {
            break;
        }

        cli_out("Entry %d, group %d, priority %d\n",
                entryId,
                entryInfo->groupId,
                entryInfo->entryPriority);

        /* Iterate qualifier */
        for (i = 0; i < groupInfo->qualCount; i++) {
            cli_out("\tQualifier %d: %s = ", i, groupInfo->qualData[i]->qualName);
            switch (groupInfo->qualData[i]->qualType) {
                case socC3RCEQualType_prefix:
                case socC3RCEQualType_postfix:
                case socC3RCEQualType_masked:
                case socC3RCEQualType_exact:
                    length = (groupInfo->qualData[i]->param[1] - groupInfo->qualData[i]->param[0]) + 1; /* bits */
                    break;
                case socC3RCEQualType_prefix_sparse:
                case socC3RCEQualType_postfix_sparse:
                case socC3RCEQualType_masked_sparse:
                case socC3RCEQualType_exact_sparse:
                    length = groupInfo->qualData[i]->paramCount; /* this is in bits */
                    break;
                default:
                    /* should never reach here, so this is an error */
                    length = 0;
                    break;
            }
            length = (length + 7) >> 3; /* now it is bytes */
            _show_rce_entry_qualify(unit, entryId, i, length);
        }

        /* Iterate action */
        for (i = 0; i < SOC_C3_RCE_RESULT_REGISTER_COUNT; i++) {
            if ((1 << i) & groupInfo->resultLrp) {
                _show_rce_entry_action(unit, entryId, programInfo->actionTable[i]);
            }
        }
    }while (0);

    soc_c3_rce_entry_info_free(unit, entryInfo);
    soc_c3_rce_group_info_free(unit, groupInfo);
    soc_c3_rce_program_info_free(unit, programInfo);
    return result;
}

static int
_show_rce_group(int unit, int programId, int groupId, void *extras)
{
    soc_c3_rce_group_desc_t *groupInfo = NULL;
        
    int result = SOC_E_NONE;

    if ((0 > unit) || (SOC_MAX_NUM_DEVICES <= unit)) {
        return SOC_E_UNIT;
    }

    do {
        result = soc_c3_rce_group_info_get(unit,groupId, &groupInfo);
        if (SOC_E_NONE != result) {
            break;
        }
        if (programId != groupInfo->rceProgram) {
            break;
        }
        result = soc_c3_rce_entry_traverse(unit, groupId, _show_rce_entry, NULL);
    } while(0);

    soc_c3_rce_group_info_free(unit, groupInfo);
    return SOC_E_NONE;
}

static int
_show_rce_program(int unit, int programId, void *extras)
{
  int result  COMPILER_ATTRIBUTE((unused));

    if ((0 > unit) || (SOC_MAX_NUM_DEVICES <= unit)) {
        return SOC_E_UNIT;
    }

    result = soc_c3_rce_group_traverse(unit, programId, _show_rce_group, NULL);

    return SOC_E_NONE;
}

static int
_show_rce(int unit, void *extras)
{
  int result  COMPILER_ATTRIBUTE((unused));

    if ((0 > unit) || (SOC_MAX_NUM_DEVICES <= unit)) {
        return SOC_E_UNIT;
    }

    result = soc_c3_rce_program_traverse(unit, _show_rce_program, NULL);

    return SOC_E_NONE;
}


char soc_sbx_t3p1_rce_get_usage[] =
"\n"
"t3p1rceget all \n"
"t3p1rceget <programId> all\n"
"t3p1rceget <programId> <groupId> all \n"
"t3p1rceget <programId> <groupId> <entryId> \n"
;

cmd_result_t
cmd_sbx_t3p1_rce_get (int unit, args_t *args)
{
    int program_result = 0;
	int group_result = 0;
    int entry_result = 0;
    unsigned int programId = 0;
    unsigned int groupId = 0;
    unsigned int entryId = 0;
    int argc = ARG_CNT(args);
    char **argv = &_ARG_CUR(args);

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("This command will only work on an CALADAN3 chip.\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    if (argc > 0) {
        program_result = soc_sbx_t3p1_read(argv[0], -1, &programId);
    }

    if (argc > 1) {
        group_result = soc_sbx_t3p1_read(argv[1], -1, &groupId);
    }

    if (argc > 2) {
        entry_result = soc_sbx_t3p1_read(argv[2], -1, &entryId);
    }

    if (1 == argc) {
        if (!sal_strcasecmp(argv[0], "all")) {
            _show_rce(unit, NULL);
            ARG_DISCARD(args);
            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else if (2 == argc) {
    	if (0 != program_result) {
    	    return CMD_USAGE;
    	}
        if (!sal_strcasecmp(argv[1], "all")) {
            _show_rce_program(unit, programId, NULL);
            ARG_DISCARD(args);
            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else if (3 == argc) {
    	if (0 != program_result) {
    	    return CMD_USAGE;
    	}
    	if (0 != group_result) {
    	    return CMD_USAGE;
    	}
        if (!sal_strcasecmp(argv[2], "all")) {
            _show_rce_group(unit, programId, groupId, NULL);
            ARG_DISCARD(args);
            return CMD_OK;
        } else if (0 == entry_result) {
            _show_rce_entry(unit, programId, groupId, entryId, NULL);
            ARG_DISCARD(args);
            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else {
        return CMD_USAGE;
    }
}



#endif /* BCM_CALADAN3_SUPPORT */
