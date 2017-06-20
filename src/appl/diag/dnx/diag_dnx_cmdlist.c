/*
 * $Id: cmdlist.c,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        cmdlist.c
 * Purpose:     List of commands available in DNX mode
 * Requires:
 */

#include <appl/diag/shell.h>
#include <appl/diag/cmdlist.h>
#include <appl/diag/sand/diag_sand_framework.h>
#include <appl/diag/sand/diag_sand_access.h>
#include <appl/diag/sand/diag_sand_signals.h>

#include "diag_dnx_cmdlist.h"

extern sh_sand_man_t sh_dnx_data_man;
extern sh_sand_cmd_t sh_dnx_data_cmds[];

extern sh_sand_man_t sh_dnx_dbal_man;
extern sh_sand_cmd_t sh_dnx_dbal_cmds[];

extern sh_sand_man_t sh_dnx_mdb_man;
extern sh_sand_cmd_t sh_dnx_mdb_cmds[];

sh_sand_cmd_t sh_dnx_commands[] = {
  /* Name    | Leaf Action | Junction Array Pointer | Options for Leaf | Usage */
    {"access", NULL,         sh_sand_access_cmds,     NULL,              &sh_sand_access_man},
    {"signal", NULL,         sh_sand_signal_cmds,     NULL,              &sh_sand_signal_man},
    {"data",   NULL,         sh_dnx_data_cmds   ,     NULL,              &sh_dnx_data_man},
    {"dbal",   NULL,         sh_dnx_dbal_cmds   ,     NULL,              &sh_dnx_dbal_man},
    {"mdb",    NULL,         sh_dnx_mdb_cmds    ,     NULL,              &sh_dnx_mdb_man},
    { NULL} /* This line should always stay as last one */
};

cmd_result_t
cmd_dnx_invoke(int unit, args_t *args)
{
    /* Take args back to first command */
    ARG_PREV(args);
    sh_sand_act(unit, args, sh_dnx_commands);
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
cmd_dnx_all(int unit, args_t *args)
{
    /*
     * Start from highest point in the tree
     */
    sh_sand_act(unit, args, sh_dnx_commands);
    /*
     * Always return OK - we provide all help & usage from inside framework
     */
    return CMD_OK;
}

cmd_result_t
cmd_dnx_verify(int unit)
{
    cmd_result_t result;
    char *command = NULL;
    command = sal_alloc(ARGS_BUFFER, "cmd_dnx_verify.command");
    if(command == NULL) {
        cli_out("Memory allocation failure\n");
        return CMD_FAIL;
    }
    command[0] = 0;

    result =  diag_sand_error_get(sh_sand_verify(unit, sh_dnx_commands, command));
    sal_free(command);
    return result;
}

const char cmd_dnx_usage[] =
        "Allows to invoke any combination of commands, get and search in usage for all dnx specific commands\n";

cmd_t bcm_dnx_cmd_list[] = {

    {"ACCess",      cmd_dnx_invoke,      cmd_sand_access_usage,   cmd_sand_access_desc},
    {"Attach",      sh_attach,           sh_attach_usage,         "Attach SOC device(s)" }, /* Init */
#if defined (BCM_PETRA_SUPPORT)
    {"CLEAR",       cmd_dpp_clear,       cmd_dpp_clear_usage,     "Clear a memory table or counters" }, /* TM */
    {"COSQ",        cmd_dpp_cosq,        cmd_dpp_cosq_usage,      "Set/Get cosq Parameters" }, /* TM */
    {"CounTeR",     cmd_dpp_counter,     cmd_dpp_counter_usage,   "Enable/disable counter collection"}, /* Counters */
    {"CounTeRProc", cmd_dpp_ctr_proc,    cmd_dpp_ctr_proc_usage,  "Counter processor diagnostics"}, /* Counters */
#if defined(__DUNE_WRX_BCM_CPU__)
    {"cpu_i2c",     cmd_dpp_cpu_i2c,     cmd_dpp_cpu_i2c_usage,   "Read/Write I2C via cpu" }, /* Access */
    {"cpu_regs",    cmd_dpp_cpu_regs,    cmd_dpp_cpu_regs_usage,  "Read/Write function to cpu regs" }, /* Access */
#endif
#endif /* BCM_PETRA_SUPPORT */
    {"DBAL",        cmd_dnx_invoke,      cmd_dnx_dbal_usage,      "DataBase Abstraction Layer diagnostics"}, /* Infra */
    {"DEInit",      sh_deinit,           sh_deinit_usage,         "Deinit SW modules" }, /* Init */
    {"DETach",      sh_detach,           sh_detach_usage,         "Detach SOC device(s)" },  /* Init */
    {"DEviceReset", sh_device_reset,     sh_device_reset_usage,   "Perform different device reset modes/actions." }, /* TM */
#if defined (BCM_PETRA_SUPPORT)
    {"DIAG",        cmd_dpp_diag,        cmd_dpp_diag_usage,      "Display diagnostic information" }, /* PP */
    {"DMA",         cmd_dpp_dma,         cmd_dpp_dma_usage,       "DMA Facilities Interface" }, /* Access */
#endif /* BCM_PETRA_SUPPORT */
    {"DNX",         cmd_dnx_all,         cmd_dnx_usage,           "Invocation point for dnx specific commands"}, /* Infra */
    {"dnx_DATA",    cmd_dnx_invoke,      cmd_dnx_data_usage,      "DNX DATA diagnostic pack"}, /* Infra */
#if defined (BCM_PETRA_SUPPORT)
    {"DRAMBuf",     cmd_arad_dram_buf,   cmd_arad_dram_buf_usage, "Manage and get information on dram buffers"}, /* TM */
    {"DramMmuIndAccess", cmd_arad_dram_mmu_ind_access, cmd_arad_dram_mmu_ind_access_usage, "Perform MMU indirect reading and writing"}, /* TM */
#endif /* BCM_PETRA_SUPPORT */
    {"Dump",        cmd_sand_mem_get,    cmd_sand_mem_get_usage,      "Dump an address space or registers" }, /* Access Back */
#if defined (BCM_PETRA_SUPPORT)
    {"export",      cmd_dpp_export,      cmd_dpp_export_usage_str,"Data Export commands" }, /* Arrakis */
    {"Fabric",      diag_dnx_fabric_diag_pack,           diag_dnx_fabric_diag_pack_usage_str, "DNX fabric diagnostic pack" }, /* TM */
    {"Fc",          cmd_dpp_fc,          cmd_dpp_fc_usage,        "Show Flow-control status"}, /* TM */
#endif /* BCM_PETRA_SUPPORT */
    {"Getreg",      cmd_sand_reg_get,     cmd_sand_reg_get_usage,   "Get register" }, /* Access Back */
#if defined (BCM_PETRA_SUPPORT)
    {"GPort",       cmd_dpp_gport,       cmd_dpp_gport_usage,     "Show the current queue gports set up in the system"}, /* SW Utils or Ports */
    {"Gtimer",      cmd_dpp_gtimer,      cmd_dpp_gtimer_usage,    "Manage gtimer"}, /* Counters */
#endif /* BCM_PETRA_SUPPORT */
#ifdef INCLUDE_I2C
    {"I2C",         cmd_i2c,             cmd_i2c_usage,           "Inter-Integrated Circuit (I2C) Bus commands"}, /* Access*/
#endif  
    {"INIT",        sh_init,             sh_init_usage,           "Initialize SOC and S/W"}, /* Init */
    {"init_dnx",    cmd_dnx_init_dnx,    appl_dnx_init_usage,    "Initialize/deinitialize DNX S/W"}, /* Init */
#if defined (BCM_PETRA_SUPPORT)
    {"intr",        cmd_dpp_intr,        cmd_dpp_intr_usage,      "Interrupt Controll" }, /* Access */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    {"KBP",         cmd_dpp_kbp,         cmd_dpp_kbp_usage,       "Perform Access to KBP data"}, /* PP */
#endif   
    {"L2",          cmd_dpp_l2,          cmd_dpp_l2_usage,        "Manage L2 (MAC) addresses"}, /* Back BCM */
    {"L3",          cmd_dpp_l3,          cmd_dpp_l3_usage,        "Manage L3 "}, /* Back BCM */
#endif /* BCM_PETRA_SUPPORT */
    {"LINKscan",    if_esw_linkscan,     if_esw_linkscan_usage,   "Configure/Display link scanning" }, /* Common */
    {"LISTmem",     cmd_sand_mem_list,    cmd_sand_mem_list_usage,  "List the entry format for a given table" }, /* Access Back*/
    {"Listreg",     cmd_sand_reg_list,    cmd_sand_reg_list_usage,  "List register fields"}, /* Access Back */
    {"MDB",         cmd_dnx_invoke,       cmd_dnx_usage,   "Modular DataBase diagnostics"}, /* Infra under DBAL */
    {"MODify",      cmd_sand_mem_modify,  cmd_sand_mem_modify_usage,"Modify table entry by field names" }, /* Access Back */
    {"Modreg",      cmd_sand_reg_modify,  cmd_sand_reg_modify_usage,"Register Read/Modify/Write" },/* Access Back */
    {"PacketWatcher",   pw_command,             pw_usage,                   "Monitor ports for packets"}, /* Common */
#if defined (BCM_PETRA_SUPPORT)
    {"PBMP",        cmd_dpp_pbmp,             cmd_dpp_pbmp_usage,     "Convert port bitmap string to hex"}, /* Back BCM */
    {"PCIC",        cmd_pcic_access,      cmd_pcic_access_usage,        "Access to PCI configuration space"  }, /* Access */
#if defined(__DUNE_WRX_BCM_CPU__)
    {"pcie",        cmd_dpp_pcie_reg,         cmd_dpp_pcie_reg_usage,    "Read/Write form devices via pcie" }, /* Access */
#endif
    {"PHY",         if_dpp_phy,             if_dpp_phy_usage,            "Set/Display phy characteristics"}, /* Back  */
    {"PORT",        if_dpp_port,              if_dpp_port_usage,      "Set/Display port characteristics"}, /* Back BCM */
#endif /* BCM_PETRA_SUPPORT */
#ifdef PORTMOD_DIAG
    {"PortMod", cmd_portmod_diag, portmod_diag_usage, "portmod diagnostics"}, /* Back */
#endif
#if defined (BCM_PETRA_SUPPORT)
    {"PortStat",    if_dpp_port_stat,         if_dpp_port_stat_usage,     "Display port status in table"}, /* Back */
#endif /* BCM_PETRA_SUPPORT */
    {"PROBE",       sh_probe,                 sh_probe_usage,     "Probe for available SOC units"}, /* Common */
#if defined (BCM_PETRA_SUPPORT)
    {"REINIT",      sh_reinit,              sh_init_usage,     "ReInitialize SOC and S/W"}, /* Init */
#endif /* BCM_PETRA_SUPPORT */
    {"Setreg",      cmd_sand_reg_set,          cmd_sand_reg_set_usage,     "Set register" }, /* Access Common */
    {"SIGnal",      cmd_dnx_invoke,            cmd_sand_signal_usage,   cmd_sand_signal_desc},
#if defined (BCM_PETRA_SUPPORT)
    {"SHOW",        cmd_dpp_show,             cmd_dpp_show_usage,     "Show information on a subsystem" }, /* Back */
    {"SOC",         cmd_dpp_soc,              cmd_dpp_soc_usage,     "Print internal driver control information"}, /* TBD */
    {"STG",         if_dpp_stg,               if_dpp_stg_usage,      "Manage spanning tree groups"}, /* Back BCM */
#endif /* BCM_PETRA_SUPPORT */
    {"STKMode",     cmd_stkmode,              cmd_stkmode_usage,     "Hardware Stacking Mode Control"}, /* TM */
#if defined (BCM_PETRA_SUPPORT)
    {"SwitchControl", cmd_dpp_switch_control, cmd_dpp_switch_control_usage,  "General switch control"}, /* TBD */
    {"Tdm",           cmd_dpp_tdm,              cmd_dpp_tdm_usage,     "Manage tdm"}, /* TM */
#endif /* BCM_PETRA_SUPPORT */
#ifdef INCLUDE_TEST
    {"TestClear",   test_clear,               test_clear_usage,     "Clear run statistics for a test"},
    {"TestList",    test_print_list,          test_list_usage,     "List loaded tests and status"},
    {"TestMode",    test_mode,                test_mode_usage,     "Set global test run modes"},
    {"TestParameters", test_parameters,       test_parameters_usage,     "Set test Parameters"},
    {"TestRun",     test_run,                 test_run_usage,     "Run a specific or selected tests"},
    {"TestSelect",  test_select,              test_select_usage,     "Select tests for running"},
#endif
#if defined (BCM_PETRA_SUPPORT)
#if defined(__DUNE_WRX_BCM_CPU__)
    {"TimeMeasurement",          cmd_dpp_time_measure,               cmd_dpp_time_measure_usage, "Time Measurement"}, /* Access */
#endif
#endif
    {"Tx",          cmd_dnx_tx,               cmd_dnx_tx_usage,     "Transmit packets"}, /* Common Back */
#if defined (BCM_PETRA_SUPPORT)
    {"Vlan",        cmd_dpp_vlan,             cmd_dpp_vlan_usage,     "Manage VLANs"}, /* Back BCM */
#endif /* BCM_PETRA_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT
    {"WARMBOOT",        sh_warmboot,            sh_warmboot_usage,    "Optionally boot warm"}, /* Common */
#endif /*BCM_WARM_BOOT_SUPPORT*/
    {"Write",       cmd_sand_mem_write,        cmd_sand_mem_write_usage,     "Write entry(s) into a table" }, /* Access Back */
};

int bcm_dnx_cmd_cnt = COUNTOF(bcm_dnx_cmd_list);
