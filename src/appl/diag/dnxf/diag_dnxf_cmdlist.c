/*
 * $Id: cmdlist.c,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        cmdlist.c
 * Purpose:     List of commands available in DPP mode
 * Requires:
 */

#include <appl/diag/shell.h>
#include <appl/diag/cmdlist.h>
#include <appl/diag/sand/diag_sand_framework.h>
#include <appl/diag/sand/diag_sand_access.h>

#include "diag_dnxf_cmdlist.h"

sh_sand_cmd_t sh_dnxf_commands[] = {
    {"access", NULL,         sh_sand_access_cmds, NULL, &sh_sand_access_man},
    { NULL} /* This line should always stay as last one */
};

cmd_result_t
cmd_dnxf_invoke(int unit, args_t *args)
{
    /* Take args back to first command */
    ARG_PREV(args);
    sh_sand_act(unit, args, sh_dnxf_commands);
    /*
     * Always return OK - we provide all help & usage from inside framework
     */
    return CMD_OK;
}

/**
 * \brief Routine allowing to treat all new commands in unified way.
 * All commands may be invoked through dnx - e.g. "dnx access list..."
 */
cmd_result_t
cmd_dnxf_all(int unit, args_t *args)
{
    /*
     * Start from highest point in the tree
     */
    sh_sand_act(unit, args, sh_dnxf_commands);
    /*
     * Always return OK - we provide all help & usage from inside framework
     */
    return CMD_OK;
}

cmd_result_t
cmd_dnxf_verify(int unit)
{
    cmd_result_t result;
    char *command = NULL;
    command = sal_alloc(ARGS_BUFFER, "cmd_dnx_verify.command");
    if(command == NULL) {
        cli_out("Memory allocation failure\n");
        return CMD_FAIL;
    }
    command[0] = 0;

    result =  diag_sand_error_get(sh_sand_verify(unit, sh_dnxf_commands, command));
    sal_free(command);
    return result;
}

const char cmd_dnxf_usage[] =
        "Allows to invoke any combination of commands, get and search in usage for all dnx specific commands\n";

cmd_t bcm_dnxf_cmd_list[] = {

  {"ACCess",      cmd_dnxf_invoke,      cmd_sand_access_usage,   cmd_sand_access_desc},
  {"Attach", sh_attach, sh_attach_usage, "Attach SOC device(s)"},
#if defined(BCM_DFE_SUPPORT)
  {"AVS", cmd_avs, cmd_avs_usage, "AVS - get AVS (Adjustable Voltage Scaling) value"},
  {"CLEAR", cmd_dpp_clear, cmd_dpp_clear_usage, "Clear a memory table or counters"},
  {"CounTeR", cmd_dpp_counter, cmd_dpp_counter_usage, "Enable/disable counter collection"},
#if defined(__DUNE_WRX_BCM_CPU__)
  {"cpu_i2c", cmd_dpp_cpu_i2c, cmd_dpp_cpu_i2c_usage, "Read/Write I2C via cpu"},
  {"cpu_regs", cmd_dpp_cpu_regs, cmd_dpp_cpu_regs_usage, "Read/Write function to cpu regs"},
#endif
#endif /* BCM_DFE_SUPPORT */
  {"DEInit", sh_deinit, sh_deinit_usage, "Deinit SW modules"},
  {"DETach", sh_detach, sh_detach_usage, "Detach SOC device(s)"},
#if defined(BCM_DFE_SUPPORT)
  {"DIAG", cmd_dpp_diag, cmd_dpp_diag_usage, "Display diagnostic information"},
  {"DMA",  cmd_dpp_dma, cmd_dpp_dma_usage, "DMA Facilities Interface"},
#endif /* BCM_DFE_SUPPORT */
  {"DNX",  cmd_dnxf_all,   cmd_dnxf_usage, "Invocation point for dnxf specific commands"}, /* Infra */
  {"Dump", cmd_sand_mem_get, cmd_sand_mem_get_usage, "Dump an address space or registers"},
#if defined(BCM_DFE_SUPPORT)
  {"Fabric", diag_dnx_fabric_diag_pack, diag_dnx_fabric_diag_pack_usage_str, "DNX fabric diagnostic pack"},
#endif /* BCM_DFE_SUPPORT */
  {"Getreg", cmd_sand_reg_get, cmd_sand_reg_get_usage, "Get register"},
#if defined(BCM_DFE_SUPPORT)
  {"GPort", cmd_dpp_gport, cmd_dpp_gport_usage, "Show the current queue gports set up in the system"},
#endif /* BCM_DFE_SUPPORT */
#ifdef INCLUDE_I2C
  {"I2C", cmd_i2c, cmd_i2c_usage, "Inter-Integrated Circuit (I2C) Bus commands"},
#endif
  {"INIT", sh_init, sh_init_usage, "Initialize SOC and S/W"},
#if defined(BCM_DFE_SUPPORT)
  {"init_dnx", cmd_init_dnx, appl_dcmn_init_usage, "Initialize/deinitialize DNX S/W"},
  {"intr", cmd_dpp_intr, cmd_dpp_intr_usage, "Interrupt Controling"},
#endif /* BCM_DFE_SUPPORT */
  {"LINKscan", if_esw_linkscan, if_esw_linkscan_usage, "Configure/Display link scanning"},
  {"LISTmem",     cmd_sand_mem_list,    cmd_sand_mem_list_usage,  "List the entry format for a given table" },
  {"Listreg",     cmd_sand_reg_list,    cmd_sand_reg_list_usage,  "List register fields"},
  {"MODify",      cmd_sand_mem_modify,  cmd_sand_mem_modify_usage,"Modify table entry by field names" },
  {"Modreg",      cmd_sand_reg_modify,  cmd_sand_reg_modify_usage,"Register Read/Modify/Write" },
  {"PacketWatcher", pw_command, pw_usage, "Monitor ports for packets"},
#if defined(BCM_DFE_SUPPORT)
  {"PBMP", cmd_dpp_pbmp, cmd_dpp_pbmp_usage, "Convert port bitmap string to hex"},
  {"PCIC", cmd_pcic_access, cmd_pcic_access_usage, "Access to PCI configuration space"},
#if defined(__DUNE_WRX_BCM_CPU__)
  {"pcie", cmd_dpp_pcie_reg, cmd_dpp_pcie_reg_usage, "Read/Write form devices via pcie"},
#endif
  {"PHY", if_dpp_phy, if_dpp_phy_usage, "Set/Display phy characteristics"},
  {"PORT", if_dpp_port, if_dpp_port_usage, "Set/Display port characteristics"},
#endif /* BCM_DFE_SUPPORT */
#ifdef PORTMOD_DIAG
  {"PortMod", cmd_portmod_diag, portmod_diag_usage, "portmod diagnostics"},
#endif
#if defined(BCM_DFE_SUPPORT)
  {"PortStat", if_dpp_port_stat, if_dpp_port_stat_usage, "Display port status in table"},
#endif /* BCM_DFE_SUPPORT */
  {"PROBE", sh_probe, sh_probe_usage, "Probe for available SOC units"},
  {"Setreg",      cmd_sand_reg_set,          cmd_sand_reg_set_usage,     "Set register" },
#if defined(BCM_DFE_SUPPORT)
  {"SHOW", cmd_dpp_show, cmd_dpp_show_usage, "Show information on a subsystem"},
  {"SOC", cmd_dpp_soc, cmd_dpp_soc_usage, "Print internal driver control information"},
#endif /* BCM_DFE_SUPPORT */
  {"STKMode", cmd_stkmode, cmd_stkmode_usage, "Hardware Stacking Mode Control"},
#if defined(BCM_DFE_SUPPORT)
  {"SwitchControl", cmd_dpp_switch_control, cmd_dpp_switch_control_usage, "General switch control"},
#endif /* BCM_DFE_SUPPORT */
#ifdef INCLUDE_TEST
  {"TestClear", test_clear, test_clear_usage,   "Clear run statisistics for a test"},
  {"TestList", test_print_list, test_list_usage, "List loaded tests and status"},
  {"TestMode", test_mode, test_mode_usage, "Set global test run modes"},
  {"TestParameters", test_parameters, test_parameters_usage, "Set test Parameters"},
  {"TestRun", test_run, test_run_usage, "Run a specific or selected tests"},
  {"TestSelect", test_select, test_select_usage, "Select tests for running"},
#endif
#ifdef BCM_WARM_BOOT_SUPPORT
  {"WARMBOOT", sh_warmboot, sh_warmboot_usage, "Optionally boot warm"},
#endif /*BCM_WARM_BOOT_SUPPORT */
  {"Write",       cmd_sand_mem_write,        cmd_sand_mem_write_usage,     "Write entry(s) into a table" },
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_EASY_RELOAD_WB_COMPAT_SUPPORT)
  {"XXReload", cmd_xxreload, cmd_xxreload_usage, "\"Easy\" Reload control"}
  ,
#endif /*defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_EASY_RELOAD_WB_COMPAT_SUPPORT) */
};

int bcm_dnxf_cmd_cnt = COUNTOF(bcm_dnxf_cmd_list);
