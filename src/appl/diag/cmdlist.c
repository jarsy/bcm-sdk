/*
 * $Id: cmdlist.c,v 1.358 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag command list
 */

#include <appl/diag/system.h>
#include <shared/bsl.h>
/*
 * The following are always turned off in portability mode
 *
 */
#ifdef NO_SAL_APPL

#undef INCLUDE_I2C

#endif /* NO_SAL_APPL */


#if defined(MOUSSE) || defined(BMW) || defined(IDTRP334) || defined(GTO) || \
    defined(MBZ) || defined(IDT438) || defined(NSX) || defined(ROBO_4704) || \
    defined(METROCORE) || defined(KEYSTONE)
#ifdef VXWORKS
#include <config.h>       /* For INCLUDE_G2XX */
#endif
#endif

#if defined(INCLUDE_BCMX_DIAG)
#include "bcmx/bcmx.h"
#endif

#if defined(BCM_SBX_SUPPORT)
#include <appl/diag/sbx/sbx.h>
#include <soc/sbx/sbx_drv.h>
#endif

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
#include <appl/diag/dcmn/cmdlist.h>
#endif

#if defined(BCM_DNX_SUPPORT)
#include <appl/diag/dnx/cmdlist.h>
#endif

#if defined(BCM_DNXF_SUPPORT)
#include <appl/diag/dnxf/cmdlist.h>
#endif

#if defined(BCM_EA_SUPPORT)
#include <appl/diag/ea/ea.h>
#if defined(INCLUDE_LIB_PIONEER_HOST)
#include <appl/pioneer_host/cmdlist.h>
#endif
#endif

#include <appl/diag/diag.h>
#include <appl/diag/cmdlist.h>

cmd_t *cur_cmd_list[SOC_MAX_NUM_DEVICES];
int cur_cmd_cnt[SOC_MAX_NUM_DEVICES];
int cur_mode[SOC_MAX_NUM_DEVICES];

cmd_t * dyn_cmd_list = NULL;
int     dyn_cmd_cnt = 0;

#if (defined(BCM_ESW_SUPPORT) || defined(BCM_SBX_SUPPORT))
static cmd_result_t (*_bcm_trunk_ref)(int unit, args_t *a) = _bcm_diag_trunk_show;
#endif

#if defined(INCLUDE_LIB_CPUDB) && defined(INCLUDE_LIB_CPUTRANS) && \
    defined(INCLUDE_LIB_DISCOVER) && defined(INCLUDE_LIB_STKTASK)
#define TKS_SUPPORT
#endif

/* This is the list of commands that can be executed independent of mode */
cmd_t bcm_cmd_common[] = {
    {"?",               sh_help_short,          sh_help_short_usage,
     "Display list of commands"},
    {"??",              sh_help,                sh_help_usage,
     "@help" },
#if defined(INCLUDE_LIB_AEDEV)
    {"AEDev",  cmd_aedev, cmd_aedev_usage, "AE Development Environment Utility."},
    {"SWDev",  cmd_swdev, cmd_swdev_usage, "SW Development Environment Utility."},
#endif
#if defined(INCLUDE_APIMODE)
    {"API",       cmd_api,      cmd_api_usage,
     "Control API command mode parsing."},
#endif
    {"ASSert",       cmd_assert,      cmd_assert_usage,
     "Assert"},
#if defined(BCM_ASYNC_SUPPORT)
    {"ASYNC",       cmd_async,      cmd_async_usage,
     "Control BCM API Async daemon."},
#endif
    {"Attach",          sh_attach,              sh_attach_usage,
     "Attach SOC device(s)" },
    {"BackGround",      sh_bg,                 sh_bg_usage,
     "Execute a command in the background."},
#if defined(INCLUDE_BCMX_DIAG)
    {"BCM",             cmd_mode_bcm,            shell_bcm_usage,
     "Set shell mode to BCM."},
    {"BCMX",            cmd_mode_bcmx,           shell_bcmx_usage,
     "Set shell mode to BCMX."},
#endif
#if defined(INCLUDE_BOARD)
    {"Board",           board_cmd,              board_cmd_usage,
     "New Board support."},
#endif
#if defined(BROADCOM_DEBUG)
    {"break",           sh_break,          sh_break_usage,        "place to hang a breakpoint" },
#endif
#if defined(BCM_CMICM_SUPPORT)
    {"BroadSync",       cmd_broadsync,          cmd_broadsync_usage,     "Manage Time API BroadSync endpoints"},
#endif
    {"CASE",            sh_case,                sh_case_usage,
     "Execute command based on string match"},
#ifndef NO_SAL_APPL
#ifndef NO_UNIX_ACCESS
    {"CD",              sh_cd,                  sh_cd_usage,
     "Change current working directory"},
#endif
#if defined(INCLUDE_LIB_CINT)
    {"cint",            cmd_cint,                cmd_cint_usage,
     "Enter the C interpreter",
    },
#endif
    {"CONFig",          sh_config,              sh_config_usage,
     "Configure Management interface"},
    {"CONSole",         sh_console,             sh_console_usage,
     "Control console options"},
#ifndef NO_UNIX_ACCESS
    {"CoPy",            sh_copy,                sh_copy_usage,
     "Copy a file"},
#endif
#endif /* NO_SAL_APPL */
#if defined(TKS_SUPPORT)
    {"CPUDB",           ct_cpudb,               ct_cpudb_usage,
     "Update the CPU database"},
    {"CTEcho",          ct_echo,                ct_echo_usage,
     "Send an echo request using CPUTRANS"},
    {"CTInstall",       ct_install,             ct_install_usage,
     "Set up transport pointers in CPU transports"},
    {"CTSetup",         ct_setup,               ct_setup_usage,
     "Modify the CPUTRANS setup"},
#endif
#ifndef NO_SAL_APPL
    {"DATE",            sh_date,                sh_date_usage,
     "Set or display current date"},
#endif
#if defined(TKS_SUPPORT)
    {"DBDump",          cmd_st_db_dump,         cmd_st_db_dump_usage,
     "Dump the current StackTask CPUDB"},
    {"DBParse",         cmd_cpudb_parse,        cmd_cpudb_parse_usage,
     "Parse a line of CPUDB dumped code"},
#endif
    {"DeBug",           sh_debug,               sh_debug_usage,
     "Enable/Disable debug output"},
    {"DELAY",           sh_delay,               sh_delay_usage,
     "Put CLI task in a busy-wait loop for some amount of time"},
#if defined(BCM_XGS_SUPPORT) || defined(BCM_ROBO_SUPPORT) || defined(BCM_SBX_SUPPORT)
    {"DEVice",          cmd_device,             cmd_device_usage,
     "Device add/remove"},
#endif /* BCM_[XGS ROBO SBX]_SUPPORT */
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"DIR",             sh_ls,                  sh_ls_usage,
     "@ls" },
#endif
    {"DISPatch",    cmd_dispatch,       cmd_dispatch_usage,
     "BCM Dispatch control."},
     {"Echo",            sh_echo,               sh_echo_usage,
     "Echo command line"},
#if !defined(NO_SAL_APPL) && !defined(NO_FILEIO) && !defined(NO_UNIX_ACCESS)
    {"EDline",          edline,                 edline_usage,
     "Edit file using ancient line editor"},
#endif
#ifndef NO_UNIX_ACCESS
    {"EXIT",            sh_exit,                sh_exit_usage,
     "Exit the current shell (and possibly reset)" },
#endif
    {"EXPR",            sh_expr,                sh_expr_usage,
     "Evaluate infix expression" },
#ifndef NO_SAL_APPL
    {"FLASHINIT",       sh_flashinit,           sh_flashinit_usage,
     "Initialize on board flash as a file system"},
    {"FLASHSYNC",       sh_flashsync,           sh_flashsync_usage,
     "Sync up on board flash with file system"},
#endif
    {"FOR",             sh_for,                 sh_for_usage,
     "Execute a series of commands in a loop"},
    {"Help",            sh_help,                sh_help_usage,
     "Print this list OR usage for a specific command" },
    {"HISTory",         sh_history,             sh_history_usage,
     "List command history"},
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
#ifdef VXWORKS
    {"HOST",            if_host,                if_host_usage,
     "Manipulate host name table"},
#endif
#endif
    {"IF",              sh_if,                  sh_if_usage,
     "Conditionally execute commands"},
#ifdef BCM_IPROC_SUPPORT
    {"IPROCRead",       iprocread_cmd,          iprocread_usage,
     "Read from IPROC Area"},
    {"IPROCWrite",      iprocwrite_cmd,         iprocwrite_usage,
     "Write to IPROC Area"},
#endif
    {"JOBS",            sh_jobs,                sh_jobs_usage,
     "List current background jobs"},
    {"KILL",            sh_kill,                sh_kill_usage,
     "Terminate a background job"},
/*
 * Change 'LED' clause: This was for BCM_PETRA_SUPPORT (Arad and Jericho
 * family)and was changed to BCM_SAND_SUPPORT (Jericho-2 family, DNX) to
 * enable using a similar configuration file (For DNX: dnx.doc)
 * In the future, this may be changed as per actual DNX board details.
 */
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SAND_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    {"LED",             ledproc_cmd,            ledproc_usage,
     "Control/Load LED processor"},
#endif
    {"LOCal",           var_local,              var_local_usage,
     "Create/Delete a variable in the local scope"},
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"LOG",             sh_log,                 sh_log_usage,
     "Enable/Disable logging and set log file"},
#endif
    {"LOOP",            sh_loop,                sh_loop_usage,
     "Execute a series of commands in a loop"},
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"LS",              sh_ls,                  sh_ls_usage,
     "List current directory"},
#endif
#ifdef INCLUDE_MACSEC
    {"MacSec",           sh_macsec,               sh_macsec_usage,
     "Set/Get MACSEC register/table and call MACSEC APIs"},
#endif /* INCLUDE_MACSEC */
#ifdef BCM_CMICM_SUPPORT
    {"MCSCmd",          mcscmd_cmd,             mcscmd_usage,           "Execute cmd on uC"},
    {"MCSDump",         mcsdump_cmd,            mcsdump_usage,          "Create MCS dumpfile"},
    {"MCSLoad",         mcsload_cmd,            mcsload_usage,          "Load hexfile to MCS memory"},
    {"MCSMsg",          mcsmsg_cmd,             mcsmsg_usage,           "Start/stop messaging with MCs"},
    {"MCSStatus",       mcsstatus_cmd,          mcsstatus_usage,        "Show MCS fault status"},
    {"MCSTimeStamp",    mcstimestamp_cmd,       mcstimestamp_usage,     "Print MCS timestamp data"},
#endif
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"MKDIR",           sh_mkdir,               sh_mkdir_usage,
     "Make a directory"},
#endif
#if defined(INCLUDE_BCMX_DIAG)
    {"MODE",            cmd_mode,               shell_mode_usage,
     "Set shell mode" },
#endif
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"MORe",            sh_more,                sh_more_usage,
     "Copy a file to the console"},
    {"MoVe",            sh_rename,              sh_rename_usage,
     "Rename a file on a file system"},
#ifdef INCLUDE_NFS
    {"NFS",             cmd_nfs,                cmd_nfs_usage,
     "NFS Control"},
#endif
#endif /* !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS) */
    {"NOEcho",          sh_noecho,              sh_noecho_usage,
     "Ignore command line"},
#ifndef NO_SAL_APPL
    {"Pause",           sh_pause,               sh_pause_usage,
     "Pause command processing and wait for input"},
#endif
#ifndef NO_SAL_APPL
#ifdef VXWORKS
    {"PING",            cmd_ping,               ping_usage,
     "Ping a host through OS stack"},
#endif
#endif
#ifdef INCLUDE_LIB_PIONEER_HOST
    {"PIOneer",         cmd_pioneer,            cmd_pioneer_usage,
     "Run a pioneer host command"},
#endif
#ifdef PORTMOD_DIAG
    {"PortMod",          cmd_portmod_diag,      portmod_diag_usage,    "Portmod Diagnostics"},
#endif
    {"PRINTENV",        var_display,            var_display_usage,
     "Display current variable list"},
    {"PROBE",           sh_probe,               sh_probe_usage,
     "Probe for available SOC units"},
#ifdef BCM_BPROF_STATS
    {"PROF",             sh_prof,              sh_prof_usage,           "Profile statistics info "},
#endif
#ifdef BCM_CMICM_SUPPORT
    {"PSCAN",           pscan_cmd,              pscan_usage,
     "Control uKernel port scanning."},
#endif
#ifdef INCLUDE_PTP
    {"PTP",             cmd_ptp,                cmd_ptp_usage,           "PTP stack configuration"},
#endif
#if defined(SOC_UKERNEL_DEBUG)
#if defined(BCM_CMICM_SUPPORT) || defined(BCM_IPROC_SUPPORT)
    {"UCDBG",           cmd_cmic_ucdebug,       cmd_cmic_ucdebug_usage,  "Ukernel Debug"},
#endif
#endif
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"PWD",             sh_pwd,                 sh_pwd_usage,
     "Print platform dependent working directory"},
#endif
#ifndef NO_UNIX_ACCESS
    {"QUIT",            sh_exit,                sh_exit_usage,
     "@exit"},
#endif
#ifndef NO_SAL_APPL
    {"RCCache",         sh_rccache,             sh_rccache_usage,
     "Save contents of an rc file in memory"},
    {"RCLoad",          sh_rcload,              sh_rcload_usage,
     "Load commands from a file"},
#ifndef NO_UNIX_ACCESS
    {"REBOOT",          sh_reboot,              sh_reboot_usage,
     "Reboot the processor"},
    {"REName",          sh_rename,              sh_rename_usage,
     "@move" },
    {"RESET",           sh_reboot,              sh_reboot_usage,
     "@reboot"},
    {"RM",              sh_remove,              sh_remove_usage,
     "Remove a file from a file system"},
    {"RMDIR",           sh_rmdir,               sh_rmdir_usage,
     "Remove a directory"},
#endif
#endif  /* NO_SAL_APPL */

#if defined(BCM_RPC_SUPPORT)
    {"RPC",     cmd_rpc,        cmd_rpc_usage,
     "Control BCM API RPC daemon."},
#endif
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    {"SalProfile",      cmd_sal_profile,        cmd_sal_profile_usage,
     "Displays current SAL resource usage"},
#endif /* INCLUDE_BCM_SAL_PROFILE */
#endif /* BROADCOM_DEBUG */
#ifdef BCM_SAT_SUPPORT
    {"SAT",             cmd_sat,                cmd_sat_usage,
     "Service activation test"},
#endif /* BCM_SAT_SUPPORT */
#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"SAVE",            sh_write,               sh_write_usage,
     "Write data to a file"},
#endif
#if defined(BCM_SBX_SUPPORT) && defined(INCLUDE_BCMX_DIAG)
    {"SBX",             cmd_mode_sbx,           shell_sbx_usage,
     "Set shell mode to SBX."},
#endif
    {"SET",             sh_set,                 sh_set_usage,
     "Set various configuration options"},
    {"SETENV",          var_export,             var_export_usage,
     "Create/Delete a variable in the global scope" },

#if !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS)
    {"SHell",           sh_shell,               sh_shell_usage,
     "Invoke a system dependent shell"},
#endif /* !defined(NO_SAL_APPL) && !defined(NO_UNIX_ACCESS) */

    {"SLeep",           sh_sleep,               sh_sleep_usage,
     "Suspend the CLI task for specified amount of time"},
#ifndef NO_SAL_APPL
#ifdef INCLUDE_TCL
    {"TCL",             cmd_tcl,                tcl_usage,
     "Run a TCL shell or TCL script"},
#endif
#endif
#ifdef INCLUDE_PTP
    {"TDPLL",           cmd_tdpll,              cmd_tdpll_usage,
     "T-DPLL configuration and management"},
#endif
    {"TIME",            sh_time,                sh_time_usage,
     "Time the execution of one or more commands"},
#ifdef BSL_TRACE_INCLUDE
    {"TRaCe",           sh_trace,               sh_trace_usage,
     "Control console options"},
#endif
    {"Version",         sh_version,             sh_version_usage,
     "Print version and build information"},
};

int bcm_cmd_common_cnt = COUNTOF(bcm_cmd_common);

#if defined(BCM_ESW_SUPPORT)
/* This is the list of commands that can be executed only in ESW mode */
static cmd_t bcm_esw_cmd_list[] = {
/*                                                                                                                      */
/*    Command   |       Function            |   Usage String      |     Description (Short help)                        */
/*  ------------+---------------------------+---------------------+---------------------------------------------------- */
#ifdef INCLUDE_I2C
    {"ADC",             cmd_adc,                cmd_adc_usage,          "Show MAX127 A/D Conversions" },
#endif
    {"AGE",             cmd_esw_age,            cmd_age_esw_usage,           "Set ESW hardware age timer" },
#if defined(BCM_TOMAHAWK_SUPPORT)
    {"ASF",             if_esw_asf,             if_esw_asf_usage,       "ASF (cut-through) Mode" },
#endif
    {"Attach",          sh_attach,              sh_attach_usage,        "Attach SOC device(s)" },
    {"Auth",            cmd_esw_auth,           auth_esw_cmd_usage,     "Port-based network access control"},
#ifdef INCLUDE_AVS
    {"AVS",             cmd_avs,                cmd_avs_usage,          "AVS (Adaptive Voltage Scaling)" },
#endif
#ifdef INCLUDE_I2C
    {"BaseBoard",       cmd_bb,                 cmd_bb_usage,           "Configure baseboard system parameters."},
#endif
#ifdef BCM_KATANA_SUPPORT
#if defined(INCLUDE_L3)
#if defined(INCLUDE_BFD)
    {"BFD",             cmd_esw_bfd,            cmd_esw_bfd_usage,     "Manage BFD endpoints"},
#endif
#endif
#endif
    {"BIST",            cmd_bist,               bist_usage,             "Run on-chip memory built-in self tests"},
    {"BKPmon",          dbg_bkpmon,             bkpmon_usage,           ".Monitor incoming BKP Discard Messages" },
    {"BPDU",            if_esw_bpdu,            if_esw_bpdu_usage,          "Manage BPDU addresses" },
    {"BTiMeout",        cmd_btimeout,           btimeout_usage,         "Set BIST operation timeout in microseconds" },
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_TRIUMPH3_SUPPORT)
    {"BUFfer",          cmd_buffer,             cmd_buffer_usage,       "MMU config" },
#endif /* BCM_TRIDENT_SUPPORT || BCM_TRIUMPH3_SUPPORT */
    {"CABLEdiag",       cmd_esw_cablediag,      cmd_esw_cablediag_usage,"Run Cable Diagnotics" },
    {"CACHE",           mem_esw_cache,          mem_esw_cache_usage,    "Turn on/off software caching of tables" },
    {"CFAPINIT",        mem_cfapinit,           cfapinit_usage,         ".Init MMU cell free address table" },
#if defined(MBZ)
    {"CFMFailover",     if_cfm_failover,        if_cfm_failover_usage,  "Activate CFM failover" },
#endif
    {"CHecK",           cmd_check,              check_usage,            "Check a sorted memory table" },
    {"CLEAR",           cmd_esw_clear,          cmd_esw_clear_usage,    "Clear a memory table or counters" },
#ifdef INCLUDE_I2C
    {"CLOCKS",          cmd_clk,                cmd_clk_usage,          "Set core clock frequency." },
#endif
    {"COLOR",           cmd_color,              cmd_color_usage,        "Manage packet color"},
    {"COMBO",           if_esw_combo,           if_esw_combo_usage,     "Control combination copper/fiber ports"},
    {"COS",             cmd_esw_cos,            cmd_esw_cos_usage,      "Manage classes of service"},
    {"CounTeR",         cmd_esw_counter,        cmd_esw_counter_usage,  "Enable/disable counter collection"},
    {"CustomSTAT",         cmd_esw_custom_stat,        cmd_esw_custom_stat_usage,  "Enable/disable counter collection"},
#ifdef INCLUDE_I2C
    {"DAC",             cmd_dac,                cmd_dac_usage,          "Set DAC register"},
#endif
#ifdef BCM_DDR3_SUPPORT
    {"DDRMemRead",      cmd_ddr_mem_read,       cmd_ddr_mem_read_usage,  "Indirect reads to DDR"},
    {"DDRMemWrite",     cmd_ddr_mem_write,      cmd_ddr_mem_write_usage, "Indirect writes to DDR"},
    {"DDRPhyRead",      cmd_ddr_phy_read,       cmd_ddr_phy_read_usage,  "Read DDR40 phy registers"},
    {"DDRPhyTune",      cmd_ddr_phy_tune,       cmd_ddr_phy_tune_usage,  "Tune DDR40 phy registers"},
    {"DDRPhyWrite",     cmd_ddr_phy_write,      cmd_ddr_phy_write_usage, "Write DDR40 phy registers"},
#endif
    {"DELete",          mem_delete,             delete_usage,           "Delete entry by key from a sorted table" },
    {"DETach",          sh_detach,              sh_detach_usage,        "Detach SOC device(s)" },
    {"DMA",             cmd_esw_dma,            cmd_esw_dma_usage,      "DMA Facilities Interface" },
    {"DmaRomTest",      cmd_drt,                cmd_drt_usage,          "Simple test of the SOC DMA ROM API" },
    {"DMIRror",         if_dmirror,             if_dmirror_usage,       "Manage directed port mirroring"},
#ifdef BCM_CMICM_SUPPORT
    {"DPLL",            dpll_cmd,               dpll_usage,             "DPLL operations on SPI bus"},
#endif
    {"DSCP",            if_esw_dscp,            if_esw_dscp_usage,      "Map Diffserv Code Points" },
    {"DTAG",            if_esw_dtag,            if_esw_dtag_usage,      "Double Tagging" },
    {"Dump",            cmd_esw_dump,           cmd_esw_dump_usage,     "Dump an address space or registers" },
    {"EditReg",         cmd_esw_reg_edit,       cmd_esw_reg_edit_usage, "Edit each field of SOC internal register"},
    {"EGRess",          if_egress,              if_egress_usage,        "Manage source-based egress enabling"},
    {"EthernetAV",      cmd_esw_eav,            cmd_esw_eav_usage,      "Set/Display the Ethernet AV characteristics"},

#ifdef INCLUDE_EXAMPLES
    {"EXample",         cmd_example_exec,                cmd_example_exec_usage,   "Execute example codes "},
#endif

#ifdef BCM_TRIUMPH_SUPPORT
#ifndef NO_MEMTUNE
    {"EXTernalTuning",  cmd_extt,               cmd_extt_usage,         "External memory automatic tuning" },
    {"EXTernalTuning2", cmd_extt2,              cmd_extt2_usage,    "External memory automatic tuning 2" },
    {"EXTernalTuningSum",  cmd_extts,       cmd_extts_usage,        "External memory automatic tuning (summary)" },
#endif /* NO_MEMTUNE */
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef INCLUDE_FCMAP
    {"FCOE",             if_esw_fcoe,             if_esw_fcoe_usage, "FCoE NPV Service (Debug Only)"},
#endif

#ifdef BCM_FIELD_SUPPORT
    {"FieldProcessor",  if_esw_field_proc,       if_esw_field_proc_usage,"Manage Field Processor"},
#endif /* BCM_FIELD_SUPPORT */
#ifdef BCM_XGS5_SWITCH_PORT_SUPPORT
    {"FLEXport",        cmd_if_flexport,        cmd_if_flexport_usage,  "Flexport"},
#endif /* BCM_XGS5_SWITCH_PORT_SUPPORT */
    {"Getreg",          cmd_esw_reg_get,        cmd_esw_reg_get_usage,  "Get register" },
#ifdef BCM_KATANA_SUPPORT
    {"GlobalMeter",     cmd_esw_policer_global_meter, cmd_esw_policer_global_meter_usage,  "Global meter - policer commands"},
#endif
    {"GPORT",           if_esw_gport,           if_esw_gport_usage,     "Get a GPORT id" },
#ifdef BCM_XGS_SUPPORT
    {"H2HIGIG",         if_h2higig,             if_h2higig_usage,       "Convert hex words to higig info" },
#ifdef BCM_HIGIG2_SUPPORT
    {"H2HIGIG2",        if_h2higig2,            if_h2higig2_usage,      "Convert hex words to higig2 info" },
#endif
#endif
#ifdef BCM_XGS_SWITCH_SUPPORT
    {"HASH",            cmd_hash,               hash_usage,             "Get or set hardware hash modes" },
#endif
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_ENDURO_SUPPORT) || \
    defined(BCM_GREYHOUND2_SUPPORT)
    {"HashDestination", cmd_hash_destination,   hash_destination_usage, "Display Hash Destination" },
#endif
#ifdef INCLUDE_I2C
    {"HClksel",         cmd_hclksel,           cmd_hclksel_usage,       "Set I2C HClk (MUX for clock-chip-selects)" },
#endif
    {"HeaderMode",      cmd_hdr_mode,           cmd_hdr_mode_usage,     "Get or set packet tx header mode" },
#if defined(BCM_TRIDENT2_SUPPORT) || defined(BCM_TRIUMPH3_SUPPORT)
    {"HSP",             cmd_hsp,                cmd_hsp_usage,          "MMU HSP hierarchy" },
#endif /* BCM_TRIDENT2_SUPPORT */
#ifdef INCLUDE_I2C
    {"I2C",             cmd_i2c,                cmd_i2c_usage,          "Inter-Integrated Circuit (I2C) Bus commands"},
#endif
#ifdef BCM_HERCULES_SUPPORT
    {"IBDump",          if_ibdump,              if_ibdump_usage,        "Display packets pending in the Ingress Buffer" },
#endif
#ifdef BCM_TRIUMPH3_SUPPORT
    {"IbodSync",        cmd_ibod_sync,          ibod_sync_usage,        "Enable/Disable IBOD sync process" },
#endif /* BCM_TRIUHMPH3_SUPPORT */
#if defined(VXWORKS)
#ifndef NO_SAL_APPL
    {"IFConfig",        cmd_if_config,      cmd_if_config_usage, "Configure a SOC port for TCP/IP"},
#endif
#endif
    {"INIT",            sh_init,                sh_init_usage,          "Initialize SOC and S/W"},
    {"Insert",          mem_insert,             insert_usage,           "Insert into a sorted table" },
    {"INTR",            cmd_intr,               cmd_intr_usage,         "Enable, disable, show interrupts" },
    {"IPFIX",           cmd_ipfix,              cmd_ipfix_usage,        "IPFIX"},
    {"IPG",             if_esw_ipg,             if_esw_ipg_usage,       "Set default IPG values for ports" },
#ifdef INCLUDE_L3
#if defined(BCM_XGS_SWITCH_SUPPORT)
    {"IPMC",            if_ipmc,                if_ipmc_usage,          "Manage IPMC (IP Multicast) addresses"},
#endif /* BCM_XGS_SWITCH_SUPPORT */
#endif /* INCLUDE_L3 */
#ifdef INCLUDE_KNET
    {"KNETctrl",        cmd_knet_ctrl,          cmd_knet_ctrl_usage,    "Manage kernel network functions"},
#endif
    {"L2",              if_esw_l2,              if_esw_l2_usage,         "Manage L2 (MAC) addresses"},
#ifdef BCM_XGS_SWITCH_SUPPORT
    {"L2MODE",          cmd_l2mode,             l2mode_usage,           "Change ARL handling mode" },
#endif
#ifdef INCLUDE_L3
    {"L3",              if_l3,                  if_l3_usage,            "Manage L3 (IP) addresses"},
#endif
#if defined(BCM_TOMAHAWK_SUPPORT)
    {"LATency",			if_esw_latency,			if_esw_latency_usage,	"Switch Latency Bypass Diagnostics"},
#endif
#ifdef INCLUDE_I2C
    {"LCDMSG",          cmd_lcdmsg,             cmd_lcdmsg_usage,       "Print message on Matrix Orbital LCD display (via I2C)"},
#endif
    {"LINKscan",        if_esw_linkscan,        if_esw_linkscan_usage,  "Configure/Display link scanning" },
    {"LISTmem",         cmd_esw_mem_list,       cmd_esw_mem_list_usage, "List the entry format for a given table" },
    {"Listreg",         cmd_esw_reg_list,       cmd_esw_reg_list_usage,  "List register fields"},
#ifdef BCM_HERCULES_SUPPORT
    {"LLAINIT",         mem_llainit,            llainit_usage,          ".Init MMU LLA pointers" },
#endif
#if defined(BCM_TRIUMPH3_SUPPORT) || defined(BCM_KATANA_SUPPORT)
    {"LLS",             cmd_lls,                cmd_lls_usage,          "MMU LLS hierarchy" },
#endif /* (BCM_TRIUMPH3_SUPPORT | BCM_KATANA_SUPPORT) */
#if defined(VXWORKS)
#if defined(BMW) || defined(NSX) || defined(METROCORE)
    {"LM",              lm_console,             lm_console_usage,       "LM connect"},
#endif
#endif
    {"LOOKup",          mem_lookup,             lookup_usage,           "Look up a table entry"},
    {"MCAST",           if_esw_mcast,           if_esw_mcast_usage,     "Manage multicast table"},
    {"MemFirst",        mem_first,              memfirst_usage,          "Displays first valid memory" },
    {"MemNext",         mem_next,               memnext_usage,          "Displays next valid memory" },
#ifdef INCLUDE_MEM_SCAN
    {"MemSCAN",         mem_scan,               memscan_usage,          "Turn on/off software memory error scanning" },
#endif
    {"MemWatch",        mem_watch,              memwatch_usage,          "Turn on/off memory snooping" },
#if defined(INCLUDE_L3) && defined(BCM_TRIUMPH2_SUPPORT)
    {"MIM",             if_tr2_mim,             if_tr2_mim_usage,
"Manage XGS4 Mac-in-MAC"},
#endif
    {"MIRror",          if_esw_mirror,          if_esw_mirror_usage,     "Manage port mirroring"},
#ifdef BCM_HERCULES_SUPPORT
    {"MmuConFiG",       if_mmu_cfg,             if_mmu_cfg_usage,       "Configure MMU mode" },
#endif
    {"MmuDeBuG",        mem_mmudebug,           mmudebug_usage,         ".Place SOC MMU in debug mode" },
    {"MODify",          cmd_esw_mem_modify,     cmd_esw_mem_modify_usage,"Modify table entry by field names" },
#ifdef BCM_XGS_SUPPORT
    {"ModMap",          cmd_modmap,             modmap_cmd_usage,       "MODID Remapping" },
#endif
    {"Modreg",          cmd_esw_reg_mod,        cmd_esw_reg_mod_usage,  "Read/modify/write register"},
#if defined(INCLUDE_L3) && defined(BCM_TRIUMPH_SUPPORT)
    {"MPLS",            if_tr_mpls,             if_tr_mpls_usage,
"Manage XGS4 MPLS"},
#endif
#ifdef BCM_CMICM_SUPPORT
    {"MSPI",            mspi_cmd,               mspi_usage,             "MasterSPI Read / Write"},
#endif
    {"MTiMeout",        cmd_mtimeout,           mtimeout_usage,         "Set MIIM operation timeout in usec" },
    {"MultiCast",       cmd_multicast,          cmd_multicast_usage,    "Manage multicast operation" },
#ifdef INCLUDE_I2C
    {"MUXsel",          cmd_muxsel,             cmd_muxsel_usage,       "Set I2C LPT state (MUX for clock-chip-selects)" },
#endif
#ifdef INCLUDE_I2C
    {"NVram",           cmd_nvram,              cmd_nvram_usage,        "Manipulate Nonvolatile memory"},
#endif
    {"OAM",             cmd_esw_oam,            cmd_esw_oam_usage,     "Manage OAM groups and endpoints"},
    {"PacketWatcher",   pw_command,             pw_usage,               "Monitor ports for packets"},
#if defined(MOUSSE) || defined(BMW) || defined(GTO)
    {"PANIC",           cmd_panic,              panic_usage,            "Set system panic mode"},
#endif
    {"PBMP",            cmd_esw_pbmp,           cmd_esw_pbmp_usage,     "Convert port bitmap string to hex"},
#ifdef INCLUDE_I2C
    {"PCIE",           cmd_pcie,              cmd_pcie_usage,        "R/W PCIE core registers"},
#endif
#ifdef PE_SDK
    {"PEAPP",           cmd_peapp,              cmd_peapp_usage,        "Port Extender Lib Commands"},
#endif /* PE_SDK */
#ifdef PE_MODE_CMD
    {"PEMODE",          cmd_pemode,             cmd_pemode_usage,       "Port Extender Mode"},
#endif /* PE_MODE_CMD */
    {"PHY",             if_esw_phy,             if_esw_phy_usage,       "Set/Display phy characteristics"},
#ifdef INCLUDE_I2C
#ifdef SHADOW_SVK
    {"PIO",             cmd_pio,                cmd_pio_usage,          "Set/Display parallel port."},
#endif
    {"POE",             cmd_poe,                cmd_poe_usage,          "Configure PowerOverEthernet controllers."},
    {"POESel",          cmd_poesel,             cmd_poesel_usage,       "Set I2C POE (MUX for poe-chip-selects)" },
#endif
    {"POP",             mem_pop,                pop_usage,              "Pop an entry from a FIFO" },
    {"PORT",            if_esw_port,            if_esw_port_usage,      "Set/Display port characteristics"},
#ifdef BCM_KATANA_SUPPORT
    {"PortPolicer",     if_esw_port_policer,    if_esw_port_policer_usage, "Display port policer status in table"},
#endif
    {"PortRate",        if_esw_port_rate,       if_esw_port_rate_usage, "Set/Display port rate metering characteristics"},
    {"PortSampRate",    if_port_samp_rate,      if_port_samp_rate_usage,"Set/Display sflow port sampling rate"},
    {"PortStat",        if_esw_port_stat,       if_esw_port_stat_usage, "Display port status in table"},
#ifdef INCLUDE_I2C
#ifdef BCM_ESW_SUPPORT
    {"PPDclk",          cmd_ppdclk,             cmd_ppdclk_usage,       "Show PPD clock delay" },
#endif
#endif
    {"PROBE",           sh_probe,               sh_probe_usage,         "Probe for available SOC units"},
    {"PUSH",            mem_push,               push_usage,             "Push an entry onto a FIFO" },
    {"PUTREG",          cmd_esw_reg_set,        cmd_esw_reg_set_usage,  "@setreg" },
    {"PVlan",           if_esw_pvlan,           if_esw_pvlan_usage,     "Port VLAN settings" },
    {"RATE",            cmd_esw_rate,           cmd_esw_rate_usage,     "Manage packet rate controls"},
    {"RateBw",          cmd_bw_rate,            cmd_bw_rate_usage,      "Set/Display port bandwidth rate metering characteristics"},
    {"RegCMp",          reg_cmp,                regcmp_usage,           "Test a register value"},
#ifdef INCLUDE_REGEX
    {"REgex",        cmd_regex,           cmd_regex_usage,     "Regex command" },
#endif
    {"RegWatch",        reg_watch,              regwatch_usage,          "Turn on/off register snooping" },
    {"REMove",          mem_remove,             remove_usage,           "Delete entry by index from a sorted table" },
    {"ResTest",     cmd_sh_restest,         cmd_sh_restest_usage,   "Tests for resource manager"},
    {"RX",              cmd_rx,                 cmd_rx_usage,           ".Receive some number of packets"},
    {"RXCfg",           cmd_esw_rx_cfg,         cmd_esw_rx_cfg_usage,   "Configure RX settings"},
    {"RXInit",          cmd_esw_rx_init,        cmd_esw_rx_init_usage,  "Call bcm_rx_init"},
    {"RXMon",           cmd_esw_rx_mon,         cmd_esw_rx_mon_usage,   "Register an RX handler to dump received packets"},
#if defined(INCLUDE_SIM8051)
    {"S51",             sim8051_ent,            sim8051_usage,          "Enter 8051 simulator" },
#endif
    {"SCHan",           cmd_schan,              schan_usage,            "Send raw S-Channel message, get response" },
    {"SEArch",          mem_search,             search_usage,           "Search a table for a byte pattern" },
#if defined(SER_TR_TEST_SUPPORT)
    {"SER",  cmd_esw_ser,   cmd_esw_ser_usage,     "Performs operations related to Soft Error Recovery"},
#endif /*defined(SER_TR_TEST_SUPPORT)*/
    {"Setreg",          cmd_esw_reg_set,        cmd_esw_reg_set_usage,  "Set register" },
    {"SHOW",            cmd_esw_show,           cmd_esw_show_usage,     "Show information on a subsystem" },
#ifdef PLISIM
    {"SHUTDOWN",        cmd_shutd,              NULL,                   "Tell plisim to shutdown" },
#endif
    {"SOC",             cmd_esw_soc,            cmd_esw_soc_usage,      "Print internal Driver control information"},
    {"SOCRES",          cmd_socres,             socres_usage,           ".SOC reset (SCP channel)" },
#ifdef BCM_TRIUMPH_SUPPORT
    {"SRAM",            cmd_sram,               cmd_sram_usage,         "External DDR2_SRAM test control" },
#endif
#ifdef BCM_SRAM_SCAN_SUPPORT
    {"SramSCAN",        sram_scan,              sramscan_usage,         "Turn on/off software SRAM error scanning" },
#endif
    {"STACKMode",       cmd_stk_mode,           cmd_stk_mode_usage,     "Set/get the stack mode"},
    {"StackPortGet",    cmd_stk_port_get,       cmd_stk_port_get_usage,  "Get stacking characteristics of a port"},
    {"StackPortSet",    cmd_stk_port_set,       cmd_stk_port_set_usage,  "Set stacking characteristics of a port"},
#ifdef PLISIM
    {"StartSim",        cmd_simstart,           NULL,                   "Tell plisim to activate" },
#endif
    {"STG",             if_esw_stg,             if_esw_stg_usage,       "Manage spanning tree groups" },
    {"STiMeout",        cmd_stimeout,           stimeout_usage,         "Set S-Channel timeout in microseconds" },
    {"STKMode",         cmd_stkmode,            cmd_stkmode_usage,       "Hardware Stacking Mode Control"},
#if defined(TKS_SUPPORT)
    {"StkTask",         tks_stk_task,           tks_stk_task_usage,      "Stack task control" },
#endif
#ifdef SW_AUTONEG_SUPPORT    
    {"SW_AN", if_esw_swAutoneg, if_esw_swAutoneg_usage, "Enable/Disable SW AN Thread"},
#endif    
    {"SwitchControl",   cmd_switch_control,     cmd_switch_control_usage, "General switch control"},
#ifdef INCLUDE_I2C
#ifdef BCM_ESW_SUPPORT
    {"SYnth",           cmd_synth,              cmd_synth_usage,        "Show synthesizer frequency" },
#endif
#endif
#ifdef INCLUDE_TEST
    {"SystemSnake",     cmd_ss,                 cmd_ss_usage,           "Cycle packets through selected system"},
#endif
#ifdef BCM_TRIUMPH_SUPPORT
     {"TCAM",           cmd_tcam,               cmd_tcam_usage,           "TCAM control" },
#endif
#if defined(BCM_ESW_SUPPORT)
    {"TeCHSupport",     command_techsupport,    command_techsupport_usage,  "Collects information required to "
                                                                            "debug a given feature or subfeature" },
#endif
#ifdef INCLUDE_I2C
    {"TEMPerature",     cmd_temperature,        cmd_temperature_usage,  "Show environmental conditions"},
#endif
#ifdef INCLUDE_TEST
    {"TestClear",       test_clear,             test_clear_usage,       "Clear run statisistics for a test"},
    {"TestList",        test_print_list,        test_list_usage,        "List loaded tests and status"},
    {"TestMode",        test_mode,              test_mode_usage,        "Set global test run modes"},
    {"TestParameters",  test_parameters,        test_parameters_usage,  "Set test Parameters"},
    {"TestRun",         test_run,               test_run_usage,         "Run a specific or selected tests"},
    {"TestSelect",      test_select,            test_select_usage,      "Select tests for running"},
#endif
#ifdef INCLUDE_PTP
#ifdef PTP_KEYSTONE_STACK
    {"TOPLoad",         cmd_topload,            cmd_topload_usage,      "Load hexfile to ToP memory for PTP"},
    {"TOPInfo",         cmd_topinfo,            cmd_topinfo_usage,      "Print information about ToP Firmware"},
#endif
#endif
    {"TRUNK",           if_esw_trunk,           if_esw_trunk_usage,     "Manage port aggregation"},
#ifdef BCM_HERCULES15_SUPPORT
    {"TrunkPool",       cmd_trunkpool,          trunkpool_cmd_usage,    "Trunk pool table configuration"},
#endif
    {"TX",              cmd_esw_tx,             cmd_esw_tx_usage,       "Transmit one or more packets"},
    {"TXBeacon",        cmd_esw_txbeacon,       cmd_esw_txbeacon_usage, "txbeacon tests"},
    {"TXCount",         cmd_esw_tx_count,       cmd_esw_tx_count_usage, "Print current TX statistics"},
#ifdef PLISIM
    {"TXEN",            cmd_txen,               NULL,                   "Enable packet transmission" },
#endif
    {"TXSTArt",         cmd_esw_tx_start,       cmd_esw_tx_usage,       "Transmit one or more packets in background"},
    {"TXSTOp",          cmd_esw_tx_stop,        cmd_esw_tx_stop_usage,  "Terminate a previous \"txstart\" command"},
    {"VLAN",            if_esw_vlan,            if_esw_vlan_usage,      "Manage virtual LANs"},
    {"WARMBOOT",        sh_warmboot,            sh_warmboot_usage,      "Optionally boot warm"},
#if defined(INCLUDE_L3) && defined(BCM_TRIUMPH2_SUPPORT)
    {"WLAN",            if_tr2_wlan,            if_tr2_wlan_usage,
"Manage XGS4 WLAN"},
#endif
    {"Write",           cmd_esw_mem_write,      cmd_esw_mem_write_usage, "Write entry(s) into a table" },
#ifdef BCM_XGS3_SWITCH_SUPPORT
    {"XAUI",            cmd_xaui,                cmd_xaui_usage,         "Run XAUI BERT on specified port pair" },
#endif
#ifdef INCLUDE_I2C
    {"XClocks",         cmd_xclk,               cmd_xclk_usage,         "Configure clocks for PCI, SDRAM, Core clock"},
#endif
#ifdef INCLUDE_I2C
    {"XPoe",            cmd_xpoe,               cmd_xpoe_usage,         "Communication with PD63000 PowerOverEthernet MCU."},
#endif
#ifdef BCM_HERCULES_SUPPORT
    {"XQDump",          if_xqdump,              if_xqdump_usage,        "Display packets pending in the XQ" },
    {"XQErr",           if_xqerr,               if_xqerr_usage,         "Inject bit errors into packets pending in XQ" },
#endif
#ifdef BCM_EASY_RELOAD_SUPPORT
    {"XXReload",        cmd_xxreload,           cmd_xxreload_usage,     "\"Easy\" Reload control" },
#endif
#ifdef PHYMOD_EPIL_SUPPORT
    {"EPIL",        cmd_epil,           cmd_epil_usage,     "Epil experimental command" },
#endif
};

int bcm_esw_cmd_cnt = COUNTOF(bcm_esw_cmd_list);
#endif

#if defined(BCM_ROBO_SUPPORT)
/* This is the list of commands that can be executed only in ROBO mode */
static cmd_t bcm_robo_cmd_list[] = {
/*                                                                                                                      */
/*    Command   |       Function            |   Usage String      |     Description (Short help)                        */
/*  ------------+---------------------------+---------------------+---------------------------------------------------- */
    {"AGE",             cmd_robo_age,           age_robo_usage,             "Set ROBO hardware age timer" },
#ifdef INCLUDE_APS_DIAG_LIBS
    {"APS",             cmd_robo_aps,           cmd_robo_aps_usage,         "ARM Processor Subsystem Remote Management Command"},
#endif /* INCLUDE_APS_DIAG_LIBS */
    {"Attach",          sh_attach,              sh_attach_usage,            "Attach SOC device(s)" },
    {"Auth",            cmd_robo_auth,          cmd_robo_auth_usage,        "Port-based network access control"},
#ifdef INCLUDE_EAV_APPL
    {"BanDwidthReserve",         cmd_robo_bandwidthreserve,       cmd_robo_bandwidthreserve_usage,      "Reserve Bandwidth on an ethernet AV port"},
#endif
    {"BPDU",            if_robo_bpdu,           if_robo_bpdu_usage,         "Manage BPDU addresses" },
    {"CABLEdiag",       cmd_robo_cablediag,     cmd_robo_cablediag_usage,   "Run Cable Diagnotics" },
    {"CACHE",           mem_robo_cache,         mem_robo_cache_usage,       "Turn on/off software caching of tables" },
    {"CLEAR",           cmd_robo_clear,         cmd_robo_clear_usage,       "Clear a memory table or counters" },
    {"COLOR",           cmd_robo_color,              cmd_robo_color_usage,        "Manage packet color"},
    {"COMBO",           if_robo_combo,          if_robo_combo_usage,        "Control combination copper/fiber ports"},
    {"COS",             cmd_robo_cos,           cmd_robo_cos_usage,         "Manage classes of service"},
    {"CounTeR",         cmd_robo_counter,       cmd_robo_counter_usage,     "Enable/disable counter collection"},
    {"DETach",          sh_detach,              sh_detach_usage,            "Detach SOC device(s)" },
#ifdef INCLUDE_EAV_APPL
    {"DisCovery",         cmd_robo_discovery,       cmd_robo_discovery_usage,      "Start/Stop discovery thread for ethernet AV capable device"},
#endif
    {"DMIRror",         if_robo_dmirror,             if_robo_dmirror_usage,       "Manage directed port mirroring"},
    {"DOS",             if_dos_attack,          if_dos_attack_usage,        "DoS Attack" },
    {"DSCP",            if_robo_dscp,           if_robo_dscp_usage,         "Map Diffserv Code Points" },
    {"DTAG",            if_robo_dtag,           if_robo_dtag_usage,         "Double Tagging" },
    {"Dump",            cmd_robo_dump,          cmd_robo_dump_usage,        "Dump an address space or registers" },
    {"EditReg",         cmd_robo_reg_edit,      cmd_robo_reg_edit_usage,    "Edit each field of SOC internal register"},
    {"EGRess",          if_robo_egress,              if_robo_egress_usage,        "Manage source-based egress enabling"},
    {"EthernetAV",   cmd_robo_eav, cmd_robo_eav_usage, "Set/Display the Ethernet AV characteristics"},
#ifdef BCM_FIELD_SUPPORT
     {"FieldProcessor",  if_robo_field_proc,    if_robo_field_proc_usage,   "Manage Field Processor"},
#endif
    {"Getreg",          cmd_robo_reg_get,       cmd_robo_reg_get_usage,     "Get register"},
#if defined(VXWORKS)
    {"IFConfig",        cmd_if_config,     cmd_if_config_usage,   "Configure a SOC port for TCP/IP"},
#endif
#ifdef IMP_SW_PROTECT
    {"ImpProtect",            if_robo_imp_protect,           if_robo_imp_protect_usage,         "IMP Protect"},
#endif /* IMP_SW_PROTECT */
    {"INIT",            sh_init,                sh_init_usage,              "Initialize SOC and S/W"},
    {"IPG",             if_robo_ipg,            if_robo_ipg_usage,          "Set default IPG values for ports" },
#ifdef INCLUDE_KNET
    {"KNETctrl",        cmd_knet_ctrl,          cmd_knet_ctrl_usage,    "Manage kernel network functions"},
#endif
    {"L2",              if_robo_l2,             if_robo_l2_usage,           "Manage L2 (MAC) addresses"},
    {"L2MODE",          if_robo_l2mode,         if_robo_l2mode_usage,       "Change L2 Shadow handling mode" },
    {"LINKscan",        if_robo_linkscan,       if_robo_linkscan_usage,     "Configure/Display link scanning" },
    {"LISTmem",         cmd_robo_mem_list,      cmd_robo_mem_list_usage,    "List the entry format for a given table" },
    {"Listreg",         cmd_robo_reg_list,      cmd_robo_reg_list_usage,    "List register fields"},
    {"MCAST",           if_robo_mcast,          if_robo_mcast_usage,        "Manage multicast table"},
#ifdef BCM_TB_SUPPORT
    {"MCastREP",        if_robo_mcastrep,      if_robo_mcastrep_usage,    "Configure Multicast Replication"},
#endif  /* BCM_TB_SUPPORT */
    {"MIRror",          if_robo_mirror,         if_robo_mirror_usage,       "Manage port mirroring"},
    {"MODify",          cmd_robo_mem_modify,    cmd_robo_mem_modify_usage,  "Modify table entry by field names" },
    {"Modreg",          cmd_robo_reg_mod,       cmd_robo_reg_mod_usage,     "Read/modify/write register"},
    {"PacketWatcher",   pw_command,             pw_usage,                   "Monitor ports for packets"},
    {"PBMP",            cmd_robo_pbmp,          cmd_robo_pbmp_usage,        "Convert port bitmap string to hex"},
    {"PHY",             if_robo_phy,            if_robo_phy_usage,          "Set/Display phy characteristics"},
    {"PORT",            if_robo_port,           if_robo_port_usage,         "Set/Display port characteristics"},
    {"PortControl",     if_robo_port_control,   if_robo_port_control_usage, "Set/Display port control types"},
    {"PortCrossConnect",     if_robo_port_cross_connect,   if_robo_port_cross_connect_usage, "Set/Display port cross connect information"},
    {"PortRate",        if_robo_port_rate,      if_robo_port_rate_usage,    "Set/Display port rate metering characteristics"},
    {"PortSampRate",    if_robo_port_samp_rate,      if_robo_port_samp_rate_usage,"Set/Display sflow port sampling rate"},
    {"PortStat",        if_robo_port_stat,      if_robo_port_stat_usage,    "Display port status in table"},
    {"PROBE",           sh_probe,               sh_probe_usage,             "Probe for available SOC units"},
    {"PUTREG",          cmd_robo_reg_set,       cmd_robo_reg_set_usage,     "@setreg"},
    {"PVlan",           if_robo_pvlan,          if_robo_pvlan_usage,        "Port VLAN settings" },
    {"RATE",            cmd_robo_rate,          cmd_robo_rate_usage,        "Manage packet rate controls"},
    {"RXCfg",           cmd_robo_rx_cfg,        cmd_robo_rx_cfg_usage,      "Configure RX settings"},
    {"RXInit",          cmd_robo_rx_init,       cmd_robo_rx_init_usage,     "Call bcm_rx_init"},
    {"RXMon",           cmd_robo_rx_mon,        cmd_robo_rx_mon_usage,      "Register an RX handler to dump received packets"},
    {"Setreg",          cmd_robo_reg_set,       cmd_robo_reg_set_usage,     "Set register"},
    {"SHOW",            cmd_robo_show,          cmd_robo_show_usage,        "Show information on a subsystem"},
    {"SOC",             cmd_robo_soc,           cmd_robo_soc_usage,         "Print internal Driver control information"},
    {"STG",             if_robo_stg,            if_robo_stg_usage,          "Manage spanning tree groups" },
#ifdef BCM_TB_SUPPORT
    {"SubPort",         if_robo_subport,        if_robo_subport_usage,      "Manage subport configurations" },
#endif  /* BCM_TB_SUPPORT */
    {"SwitchControl",   cmd_robo_switch_control,     cmd_robo_switch_control_usage, "General switch control"},
#ifdef INCLUDE_TEST
    {"TestClear",       test_clear,             test_clear_usage,           "Clear run statisistics for a test"},
    {"TestList",        test_print_list,        test_list_usage,            "List loaded tests and status"},
    {"TestMode",        test_mode,              test_mode_usage,            "Set global test run modes"},
    {"TestParameters",  test_parameters,        test_parameters_usage,      "Set test Parameters"},
    {"TestRun",         test_run,               test_run_usage,             "Run a specific or selected tests"},
    {"TestSelect",      test_select,            test_select_usage,          "Select tests for running"},
#endif  /* INCLUDE_TEST */
#ifdef INCLUDE_EAV_APPL
    {"TiMeSync",         cmd_robo_timesync,       cmd_robo_timesync_usage,      "Start/Stop time sync thread for ethernet AV capable device"},
#endif
    {"TRUNK",           if_robo_trunk,          if_robo_trunk_usage,        "Manage port aggregation"},
    {"TX",              cmd_robo_tx,            cmd_robo_tx_usage,          "Transmit one or more packets"},
    {"TXCount",         cmd_robo_tx_count,      cmd_robo_tx_count_usage,    "Print current TX statistics"},
    {"TXSTArt",         cmd_robo_tx_start,      cmd_robo_tx_usage,          "Transmit one or more packets in background"},
    {"TXSTOp",          cmd_robo_tx_stop,       cmd_robo_tx_stop_usage,     "Terminate a previous \"txstart\" command"},
    {"VLAN",            if_robo_vlan,           if_robo_vlan_usage,         "Manage virtual LANs"},
    {"Write",           cmd_robo_mem_write,     cmd_robo_mem_write_usage,   "Write entry(s) into a table" },
};

int bcm_robo_cmd_cnt = COUNTOF(bcm_robo_cmd_list);

#endif /* BCM_ROBO_SUPPORT */

#if defined(BCM_EA_SUPPORT)
/* This is the list of commands that can be executed only in tk371x mode */
cmd_t bcm_ea_cmd_list[] = {
/*                                                                                                                      */
/*    Command   |       Function            |   Usage String      |     Description (Short help)                        */
/*  ------------+---------------------------+---------------------+---------------------------------------------------- */
    {"Attach",          sh_attach,              sh_attach_usage,            "Attach SOC device(s)" }, 
#if defined(BCM_TK371X_SUPPORT)	
    {"CLEAR",           cmd_ea_clear,           cmd_ea_clear_usage,         "Clear a memory table or counters" },
    {"COS",             cmd_ea_cos,             cmd_ea_cos_usage,           "Manage classes of service"},
#endif
    {"DETach",          sh_detach,              sh_detach_usage,            "Detach SOC device(s)" },
#if defined(BCM_TK371X_SUPPORT) && defined(BCM_FIELD_SUPPORT)   
    {"FieldProcessor",  if_ea_field_proc,       if_ea_field_proc_usage,     "Manage Field Processor"},
#endif
#if defined(VXWORKS)
    {"IFConfig",        cmd_if_config,          cmd_if_config_usage,        "Configure a SOC port for TCP/IP"},
#endif /* VXWORKS */
#if defined(BCM_TK371X_SUPPORT)  
    {"INIT",            sh_init,                sh_init_usage,              "Initialize SOC and S/W"},
    {"L2",              if_ea_l2,               if_ea_l2_usage,             "Manage L2 (MAC) addresses"},
    {"LINKscan",        if_ea_linkscan,         if_ea_linkscan_usage,       "Configure/Display link scanning" },
    {"MCAST",           if_ea_mcast,            if_ea_mcast_usage,          "Manage multicast table"},
    {"PORT",            if_ea_port,             if_ea_port_usage,           "Set/Display port characteristics"},
    {"PortControl",     if_ea_port_control,     if_ea_port_control_usage,   "Set/Display port control types"},
    {"PortPhyControl",	if_ea_port_phy_control, if_ea_port_phy_control_usage,"Set/Display port phy types"},
    {"PortStat",        if_ea_port_stat,        if_ea_port_stat_usage,      "Display port status in table"},
    {"PROBE",           sh_probe,               sh_probe_usage,             "Probe for available SOC units"},
    {"RXCfg",           cmd_ea_rx_cfg,          cmd_ea_rx_cfg_usage,      "Configure RX settings"},
    {"RXInit",          cmd_ea_rx_init,         cmd_ea_rx_init_usage,     "Call bcm_rx_init"},
    {"SHOW",            cmd_ea_show,            cmd_ea_show_usage,        "Show information on a subsystem"},
    {"SwitchControl",   cmd_ea_switch_control,  cmd_ea_switch_control_usage,    "General switch control"},
#endif  
    {"SOC",             cmd_ea_soc,             cmd_ea_soc_usage,         "Print internal Driver control information"},
#if defined(BCM_TK371X_SUPPORT)  
    {"SwitchControl",   cmd_ea_switch_control,  cmd_ea_switch_control_usage,    "General switch control"},
#endif
#ifdef BCM_PIONEER_SUPPORT
	{"MEM",				cmd_ea_mem_list,		cmd_ea_mem_list_usage,	   "Special Static Memory Management monitor"},
#endif
#ifdef INCLUDE_TEST
    {"TestClear",       test_clear,             test_clear_usage,           "Clear run statisistics for a test"},
    {"TestList",        test_print_list,        test_list_usage,            "List loaded tests and status"},
    {"TestMode",        test_mode,              test_mode_usage,            "Set global test run modes"},
    {"TestParameters",  test_parameters,        test_parameters_usage,      "Set test Parameters"},
    {"TestRun",         test_run,               test_run_usage,             "Run a specific or selected tests"},
    {"TestSelect",      test_select,            test_select_usage,          "Select tests for running"},
#endif  /* INCLUDE_TEST */
#if defined(BCM_TK371X_SUPPORT) 
    {"TX",              cmd_ea_tx,              cmd_ea_tx_usage,          "Transmit one or more packets"},
    {"TXCount",         cmd_ea_tx_count,        cmd_ea_tx_count_usage,    "Print current TX statistics"},
    {"TXSTArt",         cmd_ea_tx_start,        cmd_ea_tx_usage,          "Transmit one or more packets in background"},
    {"TXSTOp",          cmd_ea_tx_stop,         cmd_ea_tx_stop_usage,     "Terminate a previous \"txstart\" command"}, 
#endif	
};

int bcm_ea_cmd_cnt = COUNTOF(bcm_ea_cmd_list);

#endif /* BCM_EA_SUPPORT */

#ifdef BCM_SBX_SUPPORT
/* This is the list of commands that can be executed only in SBX mode */
extern char if_sbx_field_proc_usage[];
extern cmd_result_t if_sbx_field_proc(int unit, args_t *args);

cmd_t bcm_sbx_cmd_list[] = {
/*                                                                                                                      */
/*    Command   |       Function            |   Usage String      |     Description (Short help)                        */
/*  ------------+---------------------------+---------------------+---------------------------------------------------- */
    {"AFL",         cmd_sbx_afl,            cmd_sbx_afl_usage,          "Aligned block free list manipulation" },
#if defined(BCM_CALADAN3_SUPPORT)
    {"AGE",         cmd_sbx_age,            cmd_sbx_age_usage,          "Set L2 age timer"},
#endif /* BCM_CALADAN3_SUPPORT */
    {"Attach",      sh_attach,              sh_attach_usage,            "Attach SOC device(s)" },
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
#ifdef INCLUDE_I2C
    {"BaseBoard",       cmd_bb,                 cmd_bb_usage,           "Configure baseboard system parameters."},
#endif
#endif
#if !defined(INCLUDE_BOARD)
    {"Board",       cmd_sbx_board,          cmd_sbx_board_usage,        "Board specific config"},
#endif
#ifdef BCM_CALADAN3_SUPPORT
    {"C3Debug", cmd_sbx_caladan3_debugger, cmd_sbx_caladan3_debugger_usage, "Caladan3 debugger interface"},
    {"C3Monitor", cmd_sbx_caladan3_bgnd_monitor, cmd_sbx_caladan3_bgnd_monitor_usage, "Caladan3 bkgnd monitor"},
#endif
    {"CLEAR",       cmd_sbx_clear,          cmd_sbx_clear_usage,        "Clear a memory table or counters" },
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    {"CounTeR",     cmd_sbx_counter,        cmd_sbx_counter_usage,      "Enable/disable counter collection"},
#endif
#if defined(INCLUDE_I2C) && defined(BCM_CALADAN3_SVK)
    {"CpuI2c",          cmd_cpu_i2c,        cmd_cpu_i2c_usage,          "I2C Bus commands from CPU"},
#endif
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    {"DDRMemRead",  cmd_sbx_ddr_mem_read,  cmd_sbx_ddr_mem_read_usage,  "Indirect reads to DDR"},
    {"DDRMemWrite", cmd_sbx_ddr_mem_write, cmd_sbx_ddr_mem_write_usage, "Indirect writes to DDR"},
    {"DDRPhyRead",  cmd_sbx_ddr_phy_read,  cmd_sbx_ddr_phy_read_usage,  "Read DDR phy registers"},
    {"DDRPhyTune",  cmd_sbx_ddr_phy_tune,  cmd_sbx_ddr_phy_tune_usage,  "Tune DDR phy registers"},
    {"DDRPhyTuneAuto",  cmd_sbx_ddr_phy_tune_auto,  cmd_sbx_ddr_phy_tune_auto_usage,  "Auto Tune DDR phy registers"},
    {"DDRPhyWrite", cmd_sbx_ddr_phy_write,  cmd_sbx_ddr_phy_write_usage,  "Write DDR phy registers"},
#endif /* BCM_SIRIUS_SUPPORT || BCM_CALADAN3_SUPPORT */
#ifdef BCM_CMICM_SUPPORT
    {"DPLL",            dpll_cmd,               dpll_usage,             "DPLL operations on SPI bus"},
#endif
    {"Dump",        cmd_sbx_dump,           cmd_sbx_dump_usage,         "Dump misc. info" },
    {"Fabric",      cmd_sbx_fabric,         cmd_sbx_fabric_usage,       "Fabric misc.commands" },
    {"Field",       cmd_sbx_field,          cmd_sbx_field_usage,        "Field debugging access" },
#ifdef BCM_FIELD_SUPPORT
    {"FieldProcessor",if_sbx_field_proc,    if_sbx_field_proc_usage,    "Manage Field Processor"},
#endif /* BCM_FIELD_SUPPORT */
    {"FL",          cmd_sbx_fl,             cmd_sbx_fl_usage,           "Free list manipulation" },
#ifdef BCM_CALADAN3_SUPPORT
    {"FlowControlStatus", cmd_sbx_caladan3_flow_control_status, cmd_sbx_caladan3_flow_control_status_usage, "Flow control status" },
#endif
    {"Fo_count",    cmd_sbx_failover_count, cmd_sbx_failover_count_usage, "Count failover events" },
    {"ForceModmap", cmd_sbx_forcemodmap,    cmd_sbx_forcemodmap_usage,  "Force modmap for qe2000" },
#ifdef BCM_CALADAN3_SUPPORT
#ifdef BCM_CALADAN3_G3P1_SUPPORT

    {"G3P1CmuGet",  cmd_sbx_g3p1_cmu_get,    soc_sbx_g3p1_cmu_get_usage,  "CMU Counters Get" },
    {"G3P1CmuReset",cmd_sbx_g3p1_cmu_reset,  soc_sbx_g3p1_cmu_reset_usage,  "CMU Counters Reset" }, 
    
    {"G3P1CohAdd", cmd_sbx_g3p1_coherent_table_add, soc_sbx_g3p1_coherent_table_add_usage, "Coherent Table Add" },
    {"G3P1CohDel", cmd_sbx_g3p1_coherent_table_remove, soc_sbx_g3p1_coherent_table_remove_usage, "Coherent Table Remove" },
    {"G3P1CohDelAll", cmd_sbx_g3p1_coherent_table_remove_all, soc_sbx_g3p1_coherent_table_remove_all_usage, "Coherent Table Remove all in group" },
    {"G3p1CohGet", cmd_sbx_g3p1_coherent_table_get, soc_sbx_g3p1_coherent_table_get_usage, "Coherent Table Get" },
    {"G3p1CohSet", cmd_sbx_g3p1_coherent_table_set, soc_sbx_g3p1_coherent_table_set_usage, "Coherent Table Set" },

    {"G3P1ConstGet",  cmd_sbx_g3p1_constant_get,    soc_sbx_g3p1_constant_usage,  "Constant Value Get" },
    {"G3P1Delete",  cmd_sbx_g3p1_delete,    soc_sbx_g3p1_delete_usage,  "Table Delete" },
    {"G3P1Get",  cmd_sbx_g3p1_get,    soc_sbx_g3p1_get_usage,  "Table Get" },
    {"G3P1GlobalGet",  cmd_sbx_g3p1_global_get,    soc_sbx_g3p1_global_get_usage,  "Global Value Get" },
    {"G3P1GlobalSet",  cmd_sbx_g3p1_global_set,    soc_sbx_g3p1_global_set_usage,  "Global Value Set" },
    {"G3P1MemGet",  cmd_sbx_g3p1_mem_get,    soc_sbx_g3p1_mem_get_usage,  "Memory Get" },
    {"G3P1MemSet",  cmd_sbx_g3p1_mem_set,    soc_sbx_g3p1_mem_set_usage,  "Memory Set" },
    
    {"G3P1PolAdd", cmd_sbx_g3p1_policer_add, soc_sbx_g3p1_policer_add_usage, "Policer Add" },
    {"G3P1PolDel", cmd_sbx_g3p1_policer_remove, soc_sbx_g3p1_policer_remove_usage, "Policer Remove" },
    {"G3P1PolDelAll", cmd_sbx_g3p1_policer_remove_all, soc_sbx_g3p1_policer_remove_all_usage, "Policer Remove all in group" },
     
    {"G3P1PpeGet",  cmd_sbx_g3p1_ppe_get,    soc_sbx_g3p1_ppe_get_usage,  "PPE Table Get" },
    {"G3P1PpeSet",  cmd_sbx_g3p1_ppe_set,    soc_sbx_g3p1_ppe_set_usage,  "PPE Table Set" },
	{"G3P1PtableGet",  cmd_sbx_g3p1_ptable_get,    soc_sbx_g3p1_ptable_get_usage,  "Property Table Get" },
    {"G3P1PtableSet",  cmd_sbx_g3p1_ptable_set,    soc_sbx_g3p1_ptable_set_usage,  "Property Table Set" },

    {"G3P1RceGet",  cmd_sbx_g3p1_rce_get,    soc_sbx_g3p1_rce_get_usage,  "RCE Rule Get" },
       
    {"G3P1SeqAdd", cmd_sbx_g3p1_sequencer_add, soc_sbx_g3p1_sequencer_add_usage, "Sequencer Add" },
    {"G3P1SeqDel", cmd_sbx_g3p1_sequencer_remove, soc_sbx_g3p1_sequencer_remove_usage, "Sequencer Remove" },
    {"G3P1SeqDelAll", cmd_sbx_g3p1_sequencer_remove_all, soc_sbx_g3p1_sequencer_remove_all_usage, "Sequencer Remove all in group" },      
    
    {"G3P1Set",  cmd_sbx_g3p1_set,    soc_sbx_g3p1_set_usage,  "Table Set" },

    {"G3P1TapsSearch", cmd_sbx_g3p1_taps_prefix_search, soc_sbx_g3p1_taps_prefix_search_usage, "Search a specific prefix"},        

    {"G3P1TimeAdd", cmd_sbx_g3p1_timer_add, soc_sbx_g3p1_timer_add_usage, "Timer Add" },
    {"G3P1TimeDel", cmd_sbx_g3p1_timer_remove, soc_sbx_g3p1_timer_remove_usage, "Timer Remove" },
    {"G3P1TimeDelAll", cmd_sbx_g3p1_timer_remove_all, soc_sbx_g3p1_timer_remove_all_usage, "Timer Remove all in group" },

    {"G3P1TmuDelete",  cmd_sbx_g3p1_tmu_delete,    soc_sbx_g3p1_tmu_delete_usage,  "TMU Table Delete" },
    {"G3P1TmuGet",  cmd_sbx_g3p1_tmu_get,    soc_sbx_g3p1_tmu_get_usage,  "TMU Table Get" },
    {"G3P1TmuSet",  cmd_sbx_g3p1_tmu_set,    soc_sbx_g3p1_tmu_set_usage,  "TMU Table Set" },        
    
    {"G3P1Util",    cmd_sbx_g3p1_util,      cmd_sbx_g3p1_util_usage,    "G3P1 microcode utilities" },
#endif

    {"G3Util",      cmd_sbx_g3_util,       cmd_sbx_g3_util_usage,     "G3 Utilities" },

#endif /* Caladan3 support */

    {"GetCompRing", cmd_sbx_completion_ring_get, cmd_sbx_completion_ring_get_usage,  "Completion ring read" },
    {"Getreg",      cmd_sbx_reg_get,        cmd_sbx_reg_get_usage,      "Register Read" },
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    {"GetState",    cmd_sbx_get_state,      cmd_sbx_get_state_usage,    "Get internal BCM state information" },
#endif
#endif
    {"GetTxRing",   cmd_sbx_tx_ring_get,    cmd_sbx_tx_ring_get_usage,  "TX ring read" },
#if defined(BCM_CALADAN3_SUPPORT)
    {"HeaderCapture", cmd_sbx_caladan_hc,    cmd_sbx_caladan_hc_usage,    "Enable header capture"},
    {"HeaderDump",  cmd_sbx_caladan_hd,      cmd_sbx_caladan_hd_usage,    "Dump captured headers"},
#endif
#ifdef INCLUDE_I2C
    {"I2C",             cmd_i2c,            cmd_i2c_usage,              "Inter-Integrated Circuit (I2C) Bus commands"},
#endif
    {"INIT",        sh_init,                sh_init_usage,              "Initialize SOC and S/W"},
    {"IPMC",        cmd_sbx_ipmc,           cmd_sbx_ipmc_usage,         "Manage IPMC "},
#if defined(BCM_CALADAN3_SUPPORT)
    {"L2",          cmd_sbx_l2,             cmd_sbx_l2_usage,           "Manage L2 (MAC) addresses"},
    {"L2Cache",     cmd_sbx_l2_cache,       cmd_sbx_l2_cache_usage,     "Manage L2 (MAC) Cache addresses"},
    {"L3",          cmd_sbx_l3,             cmd_sbx_l3_usage,           "Manage L3 "},
    {"LEARN",       cmd_sbx_learn,          cmd_sbx_learn_usage,        "Control Learning"},
#endif
    {"LINKscan",    if_esw_linkscan,        if_esw_linkscan_usage,      "Configure/Display link scanning" },
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) 
    {"LISTmem",     cmd_sbx_mem_list,       cmd_sbx_mem_list_usage, "List the entry format for a given table" },
#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) */
    {"Listreg",     cmd_sbx_reg_list,       cmd_sbx_reg_list_usage,     "List Registers" },
#if defined(BCM_CALADAN3_SUPPORT) 
    {"LPM",         cmd_sbx_lpm,             cmd_sbx_lpm_usage,          "Instrument IPV4 LPM"},
    {"MAC",         cmd_sbx_mac,             cmd_sbx_mac_usage,          "Instrument MAC EM"},    
#endif /* BCM_CALADAN3_SUPPORT */
    {"MCAST",       cmd_sbx_mcast,          cmd_sbx_mcast_usage,        "Manipulate L2 multicast"},
    {"MCEnableSi", cmd_sbx_mcenablesiports,cmd_sbx_mcenablesiports_usage, "Enable SI serdes (if there is a BM9600)"},
    {"MCFABinit",    cmd_sbx_mcfabinit_config,cmd_sbx_mcfabinit_config_usage, "Run Metrocore System fabric card configuration"},
    {"MCFPGA",      cmd_sbx_mcfpga_rw,      cmd_sbx_mcfpga_rw_usage,    "R/W access to FPGA on Metrocor"},
    {"MCInit",      cmd_sbx_mcinit_config,  cmd_sbx_mcinit_config_usage, "Run Metrocore System card configuration"},
    {"MCLCInit",    cmd_sbx_mclcinit_config,cmd_sbx_mclcinit_config_usage, "Run Metrocore System line card configuration"},
    {"MCLCSalinit",    cmd_sbx_mclcstandalone_config,cmd_sbx_mclcstandalone_config_usage, "Run Metrocore System configuration for standalone line card"},
    {"MCPBinit",    cmd_sbx_mcpbinit_config,cmd_sbx_mcpbinit_config_usage, "Run Metrocore PizzaBox configuration"},
    {"MCRemoveAll",cmd_sbx_mcremoveall,   cmd_sbx_mcremoveall_usage, "RemoveAllFlows"},
#ifdef BCM_CALADAN3_SUPPORT
    {"MCSDump",         mcsdump_cmd,            mcsdump_usage,          "Create MCS dumpfile"},
    {"MCSLoad",         mcsload_cmd,            mcsload_usage,          "Load hexfile to MCS memory"},
    {"MCSMsg",          mcsmsg_cmd,             mcsmsg_usage,           "Start/stop messaging with MCs"},
#endif
    {"MCState",    cmd_sbx_mcstate_config,cmd_sbx_mcstate_config_usage, "Returns zero if all sbx units succeded through init bcm"},
    {"MCTune",    cmd_sbx_mctune,cmd_sbx_mctune_usage, "Tune hypercore serdes"},
#if defined(BCM_BM9600_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    {"MdioSiRead", cmd_sbx_mdio_serdes_read, cmd_sbx_mdio_serdes_read_usage, "Read BM9600 mdio registers"},
    {"MdioSiWrite", cmd_sbx_mdio_serdes_write, cmd_sbx_mdio_serdes_write_usage, "Write BM9600 mdio registers"},
#endif
    {"MemSet",      cmd_sbx_mem_set,       cmd_sbx_mem_set_usage,      "Set a field in QE2000 memory"},
    {"MemSHow",     cmd_sbx_mem_show,      cmd_sbx_mem_show_usage,     "Read/display QE2000 memory"},
#if defined(BCM_CALADAN3_SUPPORT)
	{"MeterMonitor", cmd_sbx_caladan3_meter_monitor, cmd_sbx_caladan3_meter_monitor_usage, "Manage meter monitor counters"},
#endif
    {"MIm",         cmd_sbx_mim,           cmd_sbx_mim_usage,          "Access mim configurations" },
    {"MIm_test",    cmd_sbx_mim_test,      cmd_sbx_mim_test_usage,     "Sample mim configuration"},
    {"MIRror",      cmd_sbx_mirror,        cmd_sbx_mirror_usage,       "Manage port mirroring"},
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    {"MODify",      cmd_sbx_mem_modify,     cmd_sbx_mem_modify_usage,  "Modify table entry by field names" },
#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) */
    {"Modreg",      cmd_sbx_reg_modify,     cmd_sbx_reg_modify_usage,      "Register Read/Modify/Write" },
    {"MPLS",        cmd_sbx_mpls,           cmd_sbx_mpls_usage,         "Manage mpls"},
#ifdef BCM_SIRIUS_SUPPORT
    {"MultiCast",   cmd_sbx_multicast,      cmd_sbx_multicast_usage,    "Manage multicast operation"},
#endif /* BCM_SIRIUS_SUPPORT */
#ifdef INCLUDE_I2C
    {"MUXsel",          cmd_muxsel,             cmd_muxsel_usage,       "Set I2C LPT state (MUX for clock-chip-selects)" },
#endif
#ifdef BCM_QE2000_SUPPORT
    {"MVTGet",  cmd_sbx_qe2000_mvtget,     cmd_sbx_qe2000_mvtget_usage,   "get MVT entry"},
    {"MVTSet",  cmd_sbx_qe2000_mvtset,     cmd_sbx_qe2000_mvtset_usage,   "set MVT entry"},
#endif
#if defined(BCM_CALADAN3_SUPPORT)
    {"NicConfig",       cmd_sbx_caladan_nic_config,           cmd_sbx_caladan_nic_config_usage,         "Configure PCI CPU NIC port"},
    {"OAM",     cmd_sbx_oam,                cmd_sbx_oam_usage,     "Manage OAM groups and endpoints"},
#endif
#ifdef BCM_CALADAN3_SUPPORT
    {"OcmAllocDump", cmd_sbx_caladan3_ocm_dump_allocator, cmd_sbx_caladan3_ocm_allocator_dump_usage, "Dumps OCM memory allocation scheme"},
#endif
#ifdef BCM_SIRIUS_SUPPORT
    {"PacketWatcher",   pw_command,             pw_usage,                   "Monitor ports for packets"},
#endif /* BCM_SIRIUS_SUPPORT */
    {"PBMP",        cmd_sbx_pbmp,           cmd_sbx_pbmp_usage,         "Convert port bitmap string to hex"},
    {"PCICRead",    cmd_sbx_pcic_read,      cmd_sbx_pcic_read_usage,     "Read PCI config space "},
    {"PCICWrite",   cmd_sbx_pcic_write,     cmd_sbx_pcic_write_usage,    "Write PCI config space "},
    {"PHY",         if_esw_phy,             if_esw_phy_usage,       "Set/Display phy characteristics"},
#if defined(INCLUDE_I2C) && defined(BCM_CALADAN3_SVK)
    {"PIO",             cmd_pio,                cmd_pio_usage,          "Set/Display parallel port."},
#endif
    {"PORT",        cmd_sbx_port,           cmd_sbx_port_usage,         "Set/Display port characteristics"},
#ifdef BCM_CALADAN3_SUPPORT
    {"PortMapDump", cmd_sbx_caladan3_port_map_dump, cmd_sbx_caladan3_port_map_dump_usage, "Dump port mapping"},
#endif
    {"PortRate",    cmd_sbx_port_rate,      cmd_sbx_port_rate_usage,    "Set/Display port egress shaper rate"},
    {"PortStat",    cmd_sbx_port_stat,      cmd_sbx_port_stat_usage,    "Display port status in table"},
#ifdef BCM_CALADAN3_SUPPORT
    {"PPEArm", cmd_sbx_caladan3_ppe_arm, cmd_sbx_caladan3_ppe_arm_usage, "Arm PPE for TCAM trace"},
    {"PPEDump", cmd_sbx_caladan3_ppe_dump, cmd_sbx_caladan3_ppe_dump_usage, "Dump PPE TCAM trace"},
#endif
    {"PrintCounts", cmd_sbx_print_counts,   cmd_sbx_print_counts_usage,         "Print count registers"},
#ifdef BCM_CALADAN3_SUPPORT
    {"PrintCountseX", cmd_sbx_print_counts_ex, cmd_sbx_print_counts_ex_usage,   "Print count registers extended"},
#endif    
    {"PrintErrors", cmd_sbx_print_errors,   cmd_sbx_print_errors_usage,         "Print error registers"},
    {"PrintInfo",   cmd_sbx_print_info,     cmd_sbx_print_info_usage, "Print string-searched registers"},
    {"PROBE",       sh_probe,               sh_probe_usage,             "Probe for available SOC units"},
    {"PVLAN",       cmd_sbx_pvlan,          cmd_sbx_pvlan_usage,        "Port VLAN settings" },
#ifdef BCM_QE2000_SUPPORT
    {"QSINdirect",  cmd_sbx_qe2000_qsindirect,     cmd_sbx_qe2000_qsindirect_usage,   "Get/Set QE-2000 queues indirect memory"},
#endif
#if defined(BCM_QE2000_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    {"QueueInfo",   cmd_sbx_qinfo_get,      cmd_sbx_qinfo_get_usage,    "Get queue configuration info"},
#endif
#ifdef BCM_CALADAN3_SUPPORT
    {"Reconfig", cmd_sbx_caladan3_reconfig, cmd_sbx_caladan3_reconfig_usage, "Caladan3 sws reconfig"},
#endif
#ifdef BCM_CALADAN3_SUPPORT
    {"RegionMapDump", cmd_sbx_caladan3_tmu_region_map_dump, cmd_sbx_caladan3_region_map_dump_usage, "Dump TMU region mapping"},
#endif
#if !defined(BCM_CALADAN3_SUPPORT)
    {"RX",          cmd_sbx_rx,             cmd_sbx_rx_usage,           "Receive a packet"},
#endif
    {"RXInit",      cmd_sbx_rx_init,        cmd_sbx_rx_init_usage,      "Initialize BCM RX thread"},
    {"RXStop",      cmd_sbx_rx_stop,        cmd_sbx_rx_stop_usage,      "Stop the BCM RX thread"},
#if !defined(BCM_CALADAN3_SUPPORT)
    {"SBX_RX",      cmd_soc_sbx_rx,         cmd_soc_sbx_rx_usage,       "Receive a packet from the SOC layer"},
    {"SBX_TX",      cmd_soc_sbx_tx,         cmd_soc_sbx_tx_usage,       "Inject a packet at the SOC layer"},
#endif
    {"Setreg",      cmd_sbx_reg_set,        cmd_sbx_reg_set_usage,      "Register Write" },
    {"SHOW",        cmd_sbx_show,           cmd_sbx_show_usage,         "Show information on a subsystem" },
    {"SOC",         cmd_sbx_soc,            cmd_sbx_soc_usage,          "Print internal driver control information"},
#if defined(BCM_BM9600_SUPPORT)
    {"SotPolicing",   cmd_sbx_sot_policing, cmd_sbx_sot_policing_usage,
     "Check crossbar channels for policing error condition, disable/enable ports in error"},
#endif
    {"STAble",      cmd_sbx_stable,         cmd_sbx_stable_usage,       "Configure the stable for warm boot"},
    {"STKMode",         cmd_stkmode,            cmd_stkmode_usage,       "Hardware Stacking Mode Control"},
    {"SwitchControl",   cmd_sbx_switch_control,     cmd_sbx_switch_control_usage, "General switch control"},
#ifdef BCM_CALADAN3_SUPPORT
    {"SwsIccGet",    cmd_sbx_caladan3_sws_icc_get,     cmd_sbx_caladan3_sws_get_usage,     "Get Tcam Icc rule"},
    {"SwsIccSet",    cmd_sbx_caladan3_sws_icc_set,     cmd_sbx_caladan3_sws_set_usage,     "Set Tcam Icc rule"},
    {"SwsParamDump", cmd_sbx_caladan3_sws_param_dump, cmd_sbx_caladan3_sws_param_dump_usage, "Dump SWS parameters"},
#endif
#ifdef BCM_CALADAN3_T3P1_SUPPORT
    {"T3p1CmuGet",  cmd_sbx_t3p1_cmu_get,    soc_sbx_t3p1_cmu_get_usage,  "CMU Counters Get" },
    {"T3p1CmuReset",cmd_sbx_t3p1_cmu_reset,  soc_sbx_t3p1_cmu_reset_usage,  "CMU Counters Reset" },

    {"T3p1CohAdd", cmd_sbx_t3p1_coherent_table_add, soc_sbx_t3p1_coherent_table_add_usage, "Coherent Table Add" },
    {"T3p1CohDel", cmd_sbx_t3p1_coherent_table_remove, soc_sbx_t3p1_coherent_table_remove_usage, "Coherent Table Remove" },
    {"T3p1CohDelAll", cmd_sbx_t3p1_coherent_table_remove_all, soc_sbx_t3p1_coherent_table_remove_all_usage, "Coherent Table Remove all in group" },
    {"T3p1CohGet", cmd_sbx_t3p1_coherent_table_get, soc_sbx_t3p1_coherent_table_get_usage, "Coherent Table Get" },
    {"T3p1CohSet", cmd_sbx_t3p1_coherent_table_set, soc_sbx_t3p1_coherent_table_set_usage, "Coherent Table Set" },

    {"T3p1ConstGet",  cmd_sbx_t3p1_constant_get,    soc_sbx_t3p1_constant_usage,  "Constant Value Get" },
    {"T3p1Delete",  cmd_sbx_t3p1_delete,    soc_sbx_t3p1_delete_usage,  "Table Delete" },
    {"T3p1Get",  cmd_sbx_t3p1_get,    soc_sbx_t3p1_get_usage,  "Table Get" },
    {"T3p1GlobalGet",  cmd_sbx_t3p1_global_get,    soc_sbx_t3p1_global_get_usage,  "Global Value Get" },
    {"T3p1GlobalSet",  cmd_sbx_t3p1_global_set,    soc_sbx_t3p1_global_set_usage,  "Global Value Set" },
    {"T3p1MemGet",  cmd_sbx_t3p1_mem_get,    soc_sbx_t3p1_mem_get_usage,  "Memory Get" },
    {"T3p1MemSet",  cmd_sbx_t3p1_mem_set,    soc_sbx_t3p1_mem_set_usage,  "Memory Set" },

    {"T3p1PolAdd", cmd_sbx_t3p1_policer_add, soc_sbx_t3p1_policer_add_usage, "Policer Add" },
    {"T3p1PolDel", cmd_sbx_t3p1_policer_remove, soc_sbx_t3p1_policer_remove_usage, "Policer Remove" },
    {"T3p1PolDelAll", cmd_sbx_t3p1_policer_remove_all, soc_sbx_t3p1_policer_remove_all_usage, "Policer Remove all in group" },

    {"T3p1PpeGet",  cmd_sbx_t3p1_ppe_get,    soc_sbx_t3p1_ppe_get_usage,  "PPE Table Get" },
    {"T3p1PpeSet",  cmd_sbx_t3p1_ppe_set,    soc_sbx_t3p1_ppe_set_usage,  "PPE Table Set" },

    {"T3p1RceGet",  cmd_sbx_t3p1_rce_get,    soc_sbx_t3p1_rce_get_usage,  "RCE Rule Get" },

    {"T3p1SeqAdd", cmd_sbx_t3p1_sequencer_add, soc_sbx_t3p1_sequencer_add_usage, "Sequencer Add" },
    {"T3p1SeqDel", cmd_sbx_t3p1_sequencer_remove, soc_sbx_t3p1_sequencer_remove_usage, "Sequencer Remove" },
    {"T3p1SeqDelAll", cmd_sbx_t3p1_sequencer_remove_all, soc_sbx_t3p1_sequencer_remove_all_usage, "Sequencer Remove all in group" },

    {"T3p1Set",  cmd_sbx_t3p1_set,    soc_sbx_t3p1_set_usage,  "Table Set" },

    {"T3p1TimeAdd", cmd_sbx_t3p1_timer_add, soc_sbx_t3p1_timer_add_usage, "Timer Add" },
    {"T3p1TimeDel", cmd_sbx_t3p1_timer_remove, soc_sbx_t3p1_timer_remove_usage, "Timer Remove" },
    {"T3p1TimeDelAll", cmd_sbx_t3p1_timer_remove_all, soc_sbx_t3p1_timer_remove_all_usage, "Timer Remove all in group" },

    {"T3p1TmuDelete",  cmd_sbx_t3p1_tmu_delete,    soc_sbx_t3p1_tmu_delete_usage,  "TMU Table Delete" },
    {"T3p1TmuGet",  cmd_sbx_t3p1_tmu_get,    soc_sbx_t3p1_tmu_get_usage,  "TMU Table Get" },
    {"T3p1TmuSet",  cmd_sbx_t3p1_tmu_set,    soc_sbx_t3p1_tmu_set_usage,  "TMU Table Set" },
    {"T3p1Util",    cmd_sbx_t3p1_util,      cmd_sbx_t3p1_util_usage,    "T3p1 microcode utilities" },
#endif

#ifdef BCM_CALADAN3_SUPPORT
    {"Taps", cmd_sbx_caladan3_taps_configure, cmd_sbx_caladan3_taps_configure_usage, "Caladan3 TAPS Configure Inteface"},
    {"Tcam", cmd_sbx_caladan3_tcam_access, cmd_sbx_caladan3_tcam_access_usage, "Caladan3 Tcam Inteface"},
#endif
#ifdef INCLUDE_I2C
    {"TEMPerature",     cmd_temperature,        cmd_temperature_usage,  "Show board temperature"},
#endif
#ifdef INCLUDE_TEST
    {"TestClear",       test_clear,             test_clear_usage,       "Clear run statisistics for a test"},
    {"TestList",        test_print_list,        test_list_usage,        "List loaded tests and status"},
    {"TestMode",        test_mode,              test_mode_usage,        "Set global test run modes"},
    {"TestParameters",  test_parameters,        test_parameters_usage,  "Set test Parameters"},
    {"TestRun",         test_run,               test_run_usage,         "Run a specific or selected tests"},
    {"TestSelect",      test_select,            test_select_usage,      "Select tests for running"},
#endif
#ifdef BCM_CALADAN3_SUPPORT
    {"TmuAllocDump", cmd_sbx_caladan3_tmu_dump_allocator, cmd_sbx_caladan3_tmu_allocator_dump_usage, "Dumps TMU memory allocation scheme"},
#endif

    {"TX",          cmd_sbx_tx,             cmd_sbx_tx_usage,           "Transmit one or more packets"},
    {"VERIFYreg",   cmd_sbx_reg_verify,     cmd_sbx_reg_verify_usage,   "Write a device register and read it back"},
    {"VLAN",        cmd_sbx_vlan,           cmd_sbx_vlan_usage,         "VLAN (q) manipulation"},
    {"WARMBOOT",    sh_warmboot,            sh_warmboot_usage,          "Optionally boot warm"},
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    {"Write",           cmd_sbx_mem_write,      cmd_sbx_mem_write_usage, "Write entry(s) into a table" },
#endif /* BCM_SIRIUS_SUPPORT || BCM_CALADAN3_SUPPORT */
#if (defined(BCM_BME3200_SUPPORT) || defined(BCM_BM9600_SUPPORT))
    {"XBtestcnt",   cmd_sbx_xb_test_cnt,    cmd_sbx_xb_test_cnt_usage,  "Count of TS headers on xbar port"},
#endif /* BCM_BME3200/9600_SUPPORT */
#ifdef BCM_EASY_RELOAD_SUPPORT
    {"XXReload",        cmd_xxreload,       cmd_xxreload_usage,         "\"Easy\" Reload control" },
#endif
    {"XXSocreload",  cmd_sbx_xxsocreload,   cmd_sbx_xxsocreload_usage,  "Allows reload mode to be set in sbx.soc"},
};

int bcm_sbx_cmd_cnt = COUNTOF(bcm_sbx_cmd_list);

#endif /* BCM_SBX_SUPPORT */

void
cmdlist_init(void)
{
    int         i;
    int         unit;

#if defined(BCM_ESW_SUPPORT)
    for (i = 1; i < bcm_esw_cmd_cnt; i++) {
        if (sal_strcasecmp(bcm_esw_cmd_list[i].c_cmd,
        bcm_esw_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm esw command %s not alphabetized\n",
                    bcm_esw_cmd_list[i].c_cmd);
        }
    }
#endif

    for (i = 1; i < bcm_cmd_common_cnt; i++) {
        if (sal_strcasecmp(bcm_cmd_common[i].c_cmd,
                       bcm_cmd_common[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm common command %s not alphabetized\n",
                    bcm_cmd_common[i].c_cmd);
        }
    }

#if defined(BCM_ROBO_SUPPORT)
    for (i = 1; i < bcm_robo_cmd_cnt; i++) {
        if (sal_strcasecmp(bcm_robo_cmd_list[i].c_cmd,
        bcm_robo_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm robo command %s not alphabetized\n",
                    bcm_robo_cmd_list[i].c_cmd);
        }
    }
#endif

#if defined(BCM_EA_SUPPORT)
    for (i = 1; i < bcm_ea_cmd_cnt; i++) {
        if (sal_strcasecmp(bcm_ea_cmd_list[i].c_cmd,
        bcm_ea_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm ethernet access command %s not alphabetized\n",
                    bcm_ea_cmd_list[i].c_cmd);
        }
    }
#endif

#if defined(INCLUDE_BCMX_DIAG)
    for (i = 1; i < bcmx_cmd_cnt; i++) {
        if (sal_strcasecmp(bcmx_cmd_list[i].c_cmd,
               bcmx_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcmx command %s not alphabetized\n",
                    bcmx_cmd_list[i].c_cmd);
        }
    }
#endif


#if defined(BCM_SBX_SUPPORT)
    for (i = 1; i < bcm_sbx_cmd_cnt; i++) {
        if (sal_strcasecmp(bcm_sbx_cmd_list[i].c_cmd,
        bcm_sbx_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm sbx command %s not alphabetized\n",
                    bcm_sbx_cmd_list[i].c_cmd);
        }
    }
#endif

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
    for (i = 1; i < bcm_dpp_cmd_cnt; i++) {
        if (sal_strcasecmp(bcm_dpp_cmd_list[i].c_cmd,
        bcm_dpp_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm dpp command %s not alphabetized\n",
                    bcm_dpp_cmd_list[i].c_cmd);
        }
    }
#endif

#if defined(BCM_DNX_SUPPORT)
    for (i = 1; i < bcm_dnx_cmd_cnt; i++) {
        if (sal_strcasecmp(bcm_dnx_cmd_list[i].c_cmd,
        bcm_dnx_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm dnx command %s not alphabetized\n",
                    bcm_dnx_cmd_list[i].c_cmd);
        }
    }
#endif

#if defined(BCM_DNXF_SUPPORT)
    for (i = 1; i < bcm_dnxf_cmd_cnt; i++) {
        if (sal_strcasecmp(bcm_dnxf_cmd_list[i].c_cmd,
        bcm_dnxf_cmd_list[i - 1].c_cmd) <= 0) {
            cli_out("WARNING: bcm dnxf command %s not alphabetized\n",
                    bcm_dnxf_cmd_list[i].c_cmd);
        }
    }
#endif
    for(i = 0; i < SOC_MAX_NUM_DEVICES; i++) {
        cur_cmd_list[i] = NULL;
        cur_cmd_cnt[i] = 0;
    }

    for (i = 0; i < soc_ndev; i++) {
        unit = SOC_NDEV_IDX2DEV(i);
#if defined(BCM_SBX_SUPPORT)
        if(SOC_IS_SBX(unit)) {
            command_mode_set(unit, SBX_CMD_MODE);
        }
#endif

#if defined(BCM_DNXF_SUPPORT)
        if(SOC_IS_DNXF(unit) ) {
            command_mode_set(unit, DNXF_CMD_MODE);
        }
#endif /* BCM_DNXF_SUPPORT */

#if defined(BCM_DNX_SUPPORT)
        if(SOC_IS_DNX(unit) ) {
            command_mode_set(unit, DNX_CMD_MODE);
        }
#endif /* BCM_DNX_SUPPORT */

#if defined(BCM_DNXF_SUPPORT)
        if(SOC_IS_DNXF(unit) ) {
            command_mode_set(unit, DNXF_CMD_MODE);
        }
#endif /* BCM_DNXF_SUPPORT */

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
        if(SOC_IS_DPP(unit) || SOC_IS_DFE(unit)) {
            command_mode_set(unit, DPP_CMD_MODE);
        }
#endif /* BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT */

#if defined(BCM_ESW_SUPPORT)
        if(SOC_IS_ESW(unit)) {
            command_mode_set(unit, ESW_CMD_MODE);
        }
#endif /* BCM_ESW_SUPPORT */
#if defined(BCM_ROBO_SUPPORT)
        if(SOC_IS_ROBO(unit)) {
            command_mode_set(unit, ROBO_CMD_MODE);
        }
#endif /* BCM_ROBO_SUPPORT */
    }

#if (defined(BCM_ESW_SUPPORT) || defined(BCM_SBX_SUPPORT))
    _bcm_trunk_ref = NULL;
#endif
}

/*
 * Add a dynamic command to the shell
 */
int
cmdlist_add(cmd_t* cmd)
{

    /*
     * Allow the dynamic command list to be cleared by passing NULL
     */
    if(cmd == NULL) {
        dyn_cmd_cnt = 0;
        return CMD_OK;
    }

    /*
     * Allow the dynamic command memory to be allocated when needed.
     * Also allows it to be called prior to diag_shell invokation if necessary.
     */

    if(dyn_cmd_list == NULL) {
        dyn_cmd_list = sal_alloc(sizeof(cmd_t)*BCM_SHELL_MAX_DYNAMIC_CMDS, "DYN CMD LIST");
        if(dyn_cmd_list == NULL) {
            return CMD_FAIL;
        }
        else {
            dyn_cmd_cnt = 0;
        }
    }

    /*
     * Attempt to add the command
     */
    if(dyn_cmd_cnt >= BCM_SHELL_MAX_DYNAMIC_CMDS) {
        /* Full */
        return CMD_FAIL;
    }
    else {
        dyn_cmd_list[dyn_cmd_cnt] = *cmd;
        dyn_cmd_cnt++;
        return CMD_OK;
    }
}

int
cmdlist_remove(cmd_t* cmd)
{
    cmd_t* p;
    int i;

    /*
     * Find this command in the array
     */
    for(i = 0, p = dyn_cmd_list; i < dyn_cmd_cnt; i++, p++) {
        if(p->c_f == cmd->c_f) {

            /*
             * This command should be removed.
             */
            if(i == dyn_cmd_cnt - 1) {
                /* This command is the last entry */
                dyn_cmd_cnt--;
            }
            else {
                /* Swap the last command into this slot */
                dyn_cmd_list[i] = dyn_cmd_list[dyn_cmd_cnt-1];
                dyn_cmd_cnt--;
            }
            return CMD_OK;
        }
    }

    /* Not found in the list */
    return CMD_NFND;
}

void
command_mode_set(int unit, int mode)
{
    if((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        cli_out("WARNING: unit:%d in command_mode_set is out of range\n", unit);
        return;
    }

    if (mode == cur_mode[unit]) {
        return;
    }

    switch (mode) {
#if defined(INCLUDE_BCMX_DIAG)
    case BCMX_CMD_MODE:
        cur_cmd_list[unit] = bcmx_cmd_list;
        cur_cmd_cnt[unit] = bcmx_cmd_cnt;
        break;
#endif
#if defined(BCM_ROBO_SUPPORT)
    case ROBO_CMD_MODE:
        cur_cmd_list[unit] = bcm_robo_cmd_list;
        cur_cmd_cnt[unit] = bcm_robo_cmd_cnt;
        break;
#endif
#if defined(BCM_EA_SUPPORT)
    case EA_CMD_MODE:
        cur_cmd_list[unit] = bcm_ea_cmd_list;
        cur_cmd_cnt[unit] = bcm_ea_cmd_cnt;
        break;
#endif
#if defined(BCM_SBX_SUPPORT)
    case SBX_CMD_MODE:
        cur_cmd_list[unit] = bcm_sbx_cmd_list;
        cur_cmd_cnt[unit] = bcm_sbx_cmd_cnt;
        break;
#endif

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
    case DPP_CMD_MODE:
        cur_cmd_list[unit] = bcm_dpp_cmd_list;
        cur_cmd_cnt[unit] = bcm_dpp_cmd_cnt;
        break;
#endif

#if defined(BCM_DNX_SUPPORT)
    case DNX_CMD_MODE:
        cur_cmd_list[unit] = bcm_dnx_cmd_list;
        cur_cmd_cnt[unit] = bcm_dnx_cmd_cnt;
        break;
#endif

#if defined(BCM_DNXF_SUPPORT)
    case DNXF_CMD_MODE:
        cur_cmd_list[unit] = bcm_dnxf_cmd_list;
        cur_cmd_cnt[unit] = bcm_dnxf_cmd_cnt;
        break;
#endif

#if defined(BCM_ESW_SUPPORT)
    case ESW_CMD_MODE:
    default:   /* BCM is default command mode */
        cur_cmd_list[unit] = bcm_esw_cmd_list;
        cur_cmd_cnt[unit] = bcm_esw_cmd_cnt;
        break;
#endif
    }
    cur_mode[unit] = mode;
}

int
command_mode_get(int unit)
{
    if((unit < 0) || (unit >= SOC_MAX_NUM_DEVICES)) {
        cli_out("WARNING: unit:%d in command_mode_get is out of range\n", unit);
        return ILLEGAL_CMD_MODE;
    }

    return cur_mode[unit];
}

/*
 * Function:
 *     subcommand_execute
 * Purpose:
 *     Parse subcommand and execute function defined in given command list
 *     table.
 *
 *     This function provides an alternate way of parsing subcommands using
 *     currently defined parse functions and typedefs.  It is not limited
 *     to parsing subcommands of a command, it can be called from a subcommand
 *     as long as the correct table is given.
 *
 * Params:
 *     unit     - Unit number (passed-in to the initial command function)
 *     args     - Pointer to the argument structure
 *                Current argument MUST point to the subcommand to be parsed
 *     cmd_list - Table of subcommands for a given command
 *     cmd_cnt  - Number of subcommands in table
 */
cmd_result_t
subcommand_execute(int unit, args_t *args, cmd_t *cmd_list, int cmd_cnt)
{
    char            *initial_cmd = ARG_CMD(args);
    char            *key;
    cmd_t           *cmd;
    char            *param;
    cmd_result_t    rv = CMD_OK;

    if ((key = ARG_GET(args)) == NULL) {
        sal_printf("%s:  Subcommand required\n", initial_cmd);
        return CMD_USAGE;
    }

    cmd = (cmd_t *) parse_lookup(key, cmd_list, sizeof(cmd_t), cmd_cnt);

    if (cmd == NULL)
    {
        sal_printf("%s: Unknown subcommand %s\n", initial_cmd, key);
        return CMD_USAGE;
    }

    /* If next param is "?", just display help for command */
    param = ARG_CUR(args);
    if (param != NULL) {
        if (sal_strcmp(param, "?") == 0)
        {
            ARG_GET(args);    /* Consume "?" */
            if ((cmd->c_usage != NULL) &&
                (soc_property_get(unit, spn_HELP_CLI_ENABLE, 1))) {
                sal_printf("Usage:  %s\n", cmd->c_usage);
            }
            if ((cmd->c_help != NULL) &&
                soc_property_get(unit, spn_HELP_CLI_ENABLE, 1)) {
                sal_printf("Help :  %s\n", cmd->c_help);
            }
            return rv;
        }
    }

    rv = cmd->c_f(unit, args);

    return rv;
}
