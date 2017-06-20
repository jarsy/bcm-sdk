/*
 * $Id: robo_diag.c,v 1.93 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * ROBO socdiag command list
 */
#include "appl/diag/diag.h"
       
char age_robo_usage[] =
    "Parameters:  [<seconds>]\n\t"
    "   Set the hardware age timer to the indicated number of seconds.\n\t"
    "   With no parameter, displays current value.\n\t"
    "   Setting to 0 disables hardware aging\n";


extern cmd_result_t cmd_robo_age(int unit, args_t *a);

char cmd_robo_auth_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "auth <option> [args...]\n"
#else
    "auth init\n\t"
    "       - Init auth function\n\t"
    "auth detach\n\t"
    "       - Detach auth function\n\t"
    "auth mac init\n\t"
    "       - Add switch mac address from config to all ports\n\t"
    "auth mac add  [PortBitMap = <pbmp>] [MACaddress=<address>]\n\t"
    "       - Add switch mac address\n\t"
    "auth mac del  [PortBitMap = <pbmp>] [MACaddress=<address>]\n\t"
    "       - Delete switch mac address\n\t"
    "auth mac clear  [PortBitMap = <pbmp>]\n\t"
    "       - Clear all switch mac addresses\n\t"
    "auth block [PortBitMap = <pbmp>] [IngressOnly=true|false]\n\t"
    "                [RxEapDrop=true|false]\n\t"
    "       - Block traffic for all directions or ingress direction only\n\t"
    "auth unblock [PortBitMap = <pbmp>] [RxEapDrop=true|false]\n\t"
    "       - All traffic allowed, ports in uncontrolled state\n\t"
    "auth enable [PortBitMap = <pbmp>] [LearnEnable=true|false] [AuthNum=#] \n\t"
    "                  [SaNum=<value>] [RxEapDrop=true|false]\n\t"
    "          # = 0: None\n\t"
    "            = 1: Static accept\n\t"
    "            = 2: Static reject\n\t"
    "            = 3: SA num (SaNum should be set)\n\t"
    "            = 4: SA match\n\t"
    "            = 5: Extend Mode(drop if SA is unknown or violated)\n\t"
    "            = 6: Simplfy Mode(trap to management port if SA is unknown or violated)\n\t"
#ifdef BCM_TB_SUPPORT
    "            = 7: SA movement Drop\n\t"
    "            = 8: SA movement CPUCOPY\n\t"
    "            = 9: SA unknown Drop\n\t"
    "            = 10: SA unknown CPUCOPY\n\t"
    "            = 11: SA overlimit Drop\n\t"
    "            = 12: SA overlimit CPUCOPY\n\t"
    "       - Authorized ports to allow traffic\n\t"
#endif
    "auth disable [PortBitMap = <pbmp>] [RxEapDrop=true|false]\n\t"
    "       - Put ports back in block state\n\t"
    "auth show\n\t"
    "       - Show ports access state\n"
#endif
    ;

extern cmd_result_t cmd_robo_auth(int unit, args_t *a);

char if_robo_bpdu_usage[] =
    "Usages:\n\t"
    "  bpdu add [Index=<n>] [MACaddress=<mac>] \n\t"
    "        - Add BPDU addresses.\n\t"
    "        - only Bcm5348 provide 3 BPDUs.\n\t"
    "  bpdu del [Index=<n>]\n\t"
    "        - Delete BPDU address.\n\t"
    "  bpdu show \n\t"
    "        - Show BPDU addresses.\n";

extern cmd_result_t if_robo_bpdu(int unit, args_t *a);

char cmd_robo_cablediag_usage[] =
    "Run Cable Diagnostics on a set of ports.\n"
    "Parameter: <portbitmap>\n";

extern cmd_result_t cmd_robo_cablediag(int unit, args_t *a);

char mem_robo_cache_usage[] =
    "Parameters [+<TABLE> | -<TABLE>]\n\t"
    "If no parameters are given, displays caching status.  Otherwise,\n\t"
    "turns software caching on (+) or off (-) for specified TABLE(s).\n\t"
    "Only VLAN and SEC_MAC available. SEC_MAC0, SEC_MAC1 and SEC_MAC2\n\t"
    "are all set to SEC_MAC\n";
extern cmd_result_t mem_robo_cache(int unit, args_t *a);


char cmd_robo_clear_usage[] =
    "clear counters [PBMP] | <TABLE> ... | DEV | stats [PBMP]\n\t"
    "counters - zeroes all or some internal packet counters\n\t"
    "<TABLE> - clears specified memory table\n\t"
    "        - set any of SEC_MAC0, SEC_MAC1 and SEC_MAC2 will\n\t"
    "          clear all of the three table.\n\t"
    "DEV - Call bcm_clear on the unit to reset to known state\n\t"
    "stats - Clear the port based statistics.\n";

extern cmd_result_t cmd_robo_clear(int unit, args_t *a);


char if_robo_combo_usage[] =
    "Parameters: <ports> [copper|fiber [<option>=<value>]]\n"
    "            <ports> watch [on|off]\n\t"
    "Display or update operating parameters of copper/fiber combo PHY.\n\t"
    "Note: the 'port' command operates on the currently active medium.\n\t"
    "Example: combo ge1 fiber autoneg_enable=1 autoneg_advert=1000,pause\n\t"
    "Watch subcommand enables/disables media change notifications\n";

extern cmd_result_t if_robo_combo(int unit, args_t *a);


char cmd_robo_cos_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "cos <option> [args...]\n"
#else
    "cos clear                  - Reset COS configuration to default\n\t"
    "cos config [Numcos=#]      - Set number of active COS queues\n\t"
    "cos map [Pri=#] [Queue=#]  - Map a priority 0-7 to a COS queue\n\t"   
    "cos show                   - Show current COS config\n\t"
    "cos strict                 - Set strict queue scheduling mode\n\t"
    "cos weight [W0=#] [W1=#] ... - Set weighted round-robin queue\n\t"
    "                               scheduling mode with the specified\n\t"
    "                               weights per queue\n\t"
    "cos drr [W0=#] [W1=#] ... - Set deficit round-robin queue\n\t"
    "                               scheduling mode with the specified\n\t"
    "                               weights per queue\n\t"
    "cos bandwidth [PortBitMap=<pbmp>] [Queue=#] [KbpsMIn=#]\n\t"
    "              [KbitsMaxBurst=#] [Flags=#]\n\t"
    "                           - Set cos bandwidth\n\t"
    "cos bandwidth_show         - Show current COS bandwidth config\n"
    "\n\t"
    "cos port map [PortBitMap=<pbmp>] [Pri=#] [Queue=#]\n\t"
    "                           - Map a port's priority 0-7 to a COS queue\n\t"
    "cos port show [PortBitMap=<pbmp>]\n\t"
    "                           - Show current port COS config\n\t"
    "cos dpcontrol [DlfDpValue=#] [DlfDpEn=#] [XoffDpEn=#]\n\t"
    "                           - Config DropPrecedence Control\n\t"
    "cos discard [Enable=true|false] [CapAvg=true|false]\n\t"
    "                           - Set discard (WRED) config\n\t"
    "cos discard_port [PortBitMap=<pbmp>] [Queue=#] [Color=green|yellow|red]\n\t"
    "             [DropSTart=#] [DropSLope=#] [AvgTime=#] [Gport=<GPORT ID>]\n\t"
    "                           - Set port discard (WRED) config\n\t"
    "cos discard_show [Gport=<GPORT ID>] [Queue=#]\n\t"
    "                           - Show current discard (WRED) config\n\t"
    "cos discard_gport [Enable=true|false] [CapAvg=true|false]\n\t"
    "                  [Gport=<GPORT ID>] [Queue=#] [Color=green|yellow|red]\n\t"
    "                  [DropSTart=#] [DropEnd=#] [DropProbability=#] [GAin=#]\n\t"
    "                           - Set GPORT discard (WRED) config\n\t"
    "cos discard_default [Enable=true|false]\n\t"
    "                  [DropSTart=#] [DropEnd=#] [DropProbability=#] [GAin=#]\n\t"
    "    - Set Default discard (WRED) config that is used for the IFP action\n\t"
    "cos bypass_wred [Enable=true|false] [PortBitMap=<pbmp>]\n\t"
    "                           - Config Bypass WRED Drop\n"
#endif
    ;
extern cmd_result_t cmd_robo_cos(int unit, args_t *a);


char cmd_robo_counter_usage[] =
     "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "counter [options]\n"
#else
    "\nParameters: [on] [off] [sync] [Interval=<usec>]\n\t"
    "\t[PortBitMap=<pbm>] [MIBGroup=<MibGroup>]\n\t"
    "Starts the counter collection task running every <usec>\n\t"
    "microseconds.  The task tallies software counters based on hardware\n\t"
    "values and must run often enough to avoid counter overflow.\n\t"
    "If <interval> is 0, stops the task.  If <interval> is omitted, prints\n\t"
    "the current INTERVAL.  sync reads in all the counters to synchronize\n\t"
    "'show counters' for the first time, and must be used alone.\n\t"
    "for bcm5396 select MIB Group(0-2) <MibGroup> for different counters\n"
#endif
    ;

extern cmd_result_t cmd_robo_counter(int unit, args_t *a);

char if_robo_dscp_usage[] =
    "Usages:\n"
    "  dscp mode Mode=<mode>\n"
    "  dscp map [dscp [prio [cng]]]]\n"
    "        - map dscp to priority and color(optional)\n"
    "  dscp unmap IntPrio=<prio> Color=<Green|Yellow|Red|Preserve>"
    "  dscp=<dscp> \n"
    "        - unmap DSCP from IntPriority (TC) and Color (DP) \n"
    "Note: 1) Color=Preserve is for TB only.\n\t"
    "2) TB BX and VO dscp value can be or'd with BCM_DSCP_ECN(0x100) \n\t"
    "  to represent {DSCP(7:2):ECN(1:0)}\n";

extern cmd_result_t if_robo_dscp(int unit, args_t *a);

char if_robo_dtag_usage[] =
    "Usage:\n"
    "\tdtag show <pbmp>\n"
    "\tdtag mode <pbmp> none|internal|external\n"
    "\tdtag tpid <pbmp> hex-value\n";

extern cmd_result_t if_robo_dtag(int unit, args_t *a);


char cmd_robo_dump_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "DUMP [options]\n"
#else
    "DUMP [File=<name>] [Append=true|false] [raw] [hex] [all] [chg]\n\t"
    "        <TABLE>[.<COPYNO>] [<INDEX>] [<COUNT>]\n\t"
    "        [-filter <FIELD>=<VALUE>[,...]]\n\t"
    "      If raw is specified, show raw memory words instead of fields.\n\t"
    "      If hex is specified, show hex data only (for Expect parsing).\n\t"
    "      If all is specified, show even empty or invalid entries\n\t"
    "      If chg is specified, show only fields changed from defaults\n\t"
    "      (Use \"listmem\" command to show a list of valid tables)\n\t"
    "DUMP PCIC                     (PCI config space)\n\t"
    "DUMP PCIM [<START> [<COUNT>]] (CMIC PCI registers)\n\t"
    "DUMP SOC [ADDR | RVAL | DIFF] (All SOC registers)\n\t"
    "      ADDR shows only addresses, doesn't actually load.\n\t"
    "      RVAL shows reset defaults, doesn't actually load.\n\t"
    "      DIFF shows only regs not equal to their reset defaults.\n\t"
    "DUMP SOCMEM [DIFF] (All SOC memories)\n\t"
    "      DIFF shows only memories not equal to their reset defaults.\n\t"
    "DUMP MW [<START> [<COUNT>]]   (System memory, 32 bits)\n\t"
    "DUMP MH [<START> [<COUNT>]]   (System memory, 16 bits)\n\t"
    "DUMP MB [<START> [<COUNT>]]   (System memory, 8 bits)\n\t"
    "DUMP SA                       (ARL shadow table)\n\t"
    "DUMP DV ADDR                  (DMA vector)\n\t"
    "DUMP PHY [<PHYID>]            See also, the 'PHY' command.\n"
#endif
    ;
extern cmd_result_t cmd_robo_dump(int unit, args_t *a);

char cmd_robo_eav_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
"Parameters: <cmd> <opt>\n"
#else /* !COMPILER_STRING_CONST_LIMIT */
" Where <cmd> is:\n"
"\tcontrol set|get <ctrl_type> <parameter>\n"
"\tmac get|set <mac value>\n"
"\ttypes. (Describe the types of Ethernet AV CLI commands)\n"
"\tinit\n"
"\tport enable|disable <port number>\n"
"\tlink on|off <port number>\n"
"\tqueue set|get <port number> <queue_control_type> <param>\n"
"\twatch start|stop\n"
"\tstatus\n"
"\ttimestamp <port number>\n"
"\ttimesync set|get <time sync type> <p0> [<p1>]\n"
"\ttx <pbmp> <vlanid>\n"
"\tsrp set|get <mac value> <ethertype>\n"
#endif /* !COMPILER_STRING_CONST_LIMIT */
;
extern cmd_result_t cmd_robo_eav(int unit, args_t *a);

#ifdef INCLUDE_EAV_APPL
char cmd_robo_timesync_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
"Parameters: <cmd> <opt>\n"
#else /* !COMPILER_STRING_CONST_LIMIT */
" Where <cmd> is:\n\t"
" timesync start <interval>\n\t"
" timesync stop \n\t"
" timesync port enable|disable|master port\n\t"
" timesync status\n\t"
" timesync debug <port>\n"
#endif /* !COMPILER_STRING_CONST_LIMIT */
;
extern cmd_result_t cmd_robo_timesync(int unit, args_t *a);




char cmd_robo_discovery_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
"Parameters: <cmd> <opt>\n"
#else /* !COMPILER_STRING_CONST_LIMIT */
" Where <cmd> is:\n\t"
" discovery start <interval>\n\t"
" discovery stop \n\t"
" discovery forcesend port\n\t"
" discovery status\n\t"
" discovery debug <port>\n"
#endif /* !COMPILER_STRING_CONST_LIMIT */
;
extern cmd_result_t cmd_robo_discovery(int unit, args_t *a);


char cmd_robo_bandwidthreserve_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
"Parameters: <cmd> <opt>\n"
#else /* !COMPILER_STRING_CONST_LIMIT */
" Where <cmd> is:\n\t"
" bandwidthreserve start\n\t"
" bandwidthreserve stop\n\t"
" bandwidthreserve status\n\t"
#endif /* !COMPILER_STRING_CONST_LIMIT */
;
extern cmd_result_t cmd_robo_bandwidthreserve(int unit, args_t *a);
#endif

char cmd_robo_reg_edit_usage[] =
    "Parameters: <REGISTER>\n\t"
    "<REGISTER> is SOC register symbolic name.\n\t"
    "Loads a register and displays each field, providing an opportunity\n\t"
    "to modify each field.\n";
extern cmd_result_t cmd_robo_reg_edit(int unit, args_t *a);


char if_robo_field_proc_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
"Parameters: <cmd> <opt>\n"
#else /* !COMPILER_STRING_CONST_LIMIT */
" Where <cmd> is:\n"
"\taction <add|get|remove> <eid> [act] [p0] [p1]\n"
"\taction ports <add|get> <eid> <act> [<pbmp>]\n"
"\tcontrol <ctrl_num> [<status>]\n"
"\tdata create OffsetBase=<offset_base> offset=<offset> length=<length>\n"
"\t            [QualId=<qualid>] \n"
"\tdata destroy [all] or QualId=<qualid> \n"
"\tdetach\n"
"\tentry create <gid> [<eid>]\n"
"\tentry copy <src_eid> [<dst_eid>]\n"
"\tentry install|reinstall|remove|destroy <eid>\n"
"\tentry enable|disable <eid>\n"
"\tentry prio <eid> [highest|lowest|dontcare|default|<priority>]\n"
"\tentry oper <eid> [backup|restore|cleanup]\n"
"\tgroup create <pri> [<gid>] [mode]\n"
"\tgroup destroy|get|set <gid>\n"
"\tinit\n"
"\tlist actions|qualifiers\n"
"\trange create [<rid>] [<flags>] [<min>] [<max>]\n"
"\trange group create [<rid>] [<flags>] [<min>] [<max>] [<group>]\n"
"\trange get|destroy <rid>\n"
"\tresync\n"
"\tpolicer create PolId=<polcerid> mode=<mode> cbs=<cbs> \n"
"\t        cir=<cir> ebs=<ebs> eir=<eir> ColorBlind=<bool> \n" 
"\tpolicer set PolId=<polcerid> mode=<mode> cbs=<cbs> \n"
"\t        cir=<cir> ebs=<ebs> eir=<eir> ColorBlind=<bool> \n" 
"\tpolicer destroy [all] or PolId=<polcerid>] \n"
"\tpolicer attach entry=<eid> level=<level> PolId=<polcerid> \n"
"\tpolicer detach entry=<eid> level=<level> \n"
"\tstat create group=<group> type0=<stat_type0> type1=<stat_type1>\n"
"\t        type2=<stat_type2> typeX=<stat_typeX>\n"
"\tstat destroy StatId=<statid> \n"
"\tstat attach entry=<eid> [StatId=<statid>]* \n"
"\t         (*if not specified, last created StatId is used) \n"
"\tstat detach entry=<eid> StatId=<statid> \n"
"\tstat set StatId=<statid> type=<stat_type> val=<value>\n"
"\tstat get StatId=<statid> type=<stat_type>\n"
"\tqual <eid> <QUAL> [<udf_id*>/data_qual_id] data mask\n" 
"\t       (*required for QUAL=UserDefined or Data)\n"
"\tqual <eid> get <QUAL>\n"
"\tqual <eid> delete <QUAL>\n"
"\tqual <eid> clear\n"
"\tqset add|clear|show [qualifier]\n"
"\tshow [group|entry] [id]\n"
"\tthread off\n"
"\tstatus\n"
#endif /* !COMPILER_STRING_CONST_LIMIT */
;
extern cmd_result_t if_robo_field_proc(int unit, args_t *a);

char cmd_robo_reg_get_usage[] =
    "Parameters: [hex] [<REGTYPE>] <REGISTER|PAGE ADDRESS>\n\t"
    "If <REGTYPE> is not specified, it defaults to \"soc\".\n\t"
    "<REGISTER> is SOC register symbolic name.\n\t"
    "<PAGE> is SOC register page value.\n\t"
    "<ADDRESS> is SOC register address value.\n\t"
    "If hex is specified, dumps only a hex value (for Expect parsing).\n\t"
    "For a list of register types, use \"dump\".\n";

extern cmd_result_t cmd_robo_reg_get(int unit, args_t *a);

#ifdef IMP_SW_PROTECT
char if_robo_imp_protect_usage[] =
    "IMP Protect <min rate> <middle rate> <max rate>\n"
    "The granularity of the rate is Kbits per second\n"
    "With no arguments, displays the IMP Protect parameters.\n\t";

extern cmd_result_t if_robo_imp_protect(int unit, args_t *a);

#endif /* IMP_SW_PROTECT */

#ifdef INCLUDE_APS_DIAG_LIBS
char cmd_robo_aps_usage[] =
    "[slow]\n"
    "\treboot\n"
    "\tversion\n"
    "\tinitialize-flash <spi #> <dmu config> <spi speed>\n"
    "\tinstall-images <spi #> <ARM image file> <switch config file>\n"
    "\tinstall-arm-image <spi #> <ARM image file>\n"
    "\tinstall-arm-boot-image <spi #> <ARM boot image file>\n"
    "\tdownload-arm-image <ARM image file>\n"
    "\tinstall-switch-cfg <spi #> <Switch config file>\n"
    "\tinstall-avb-cfg <spi #> <Avb config file>\n"
    "\tflash-info <spi #>\n"
    "\tread-flash-header <spi #> <slot #>\n"
    "\terase-flash-sector <spi #> <sector address>\n"
    "\tread-flash <spi #> <start page> <end page>\n"
    "\twrite-flash <spi #> <filename>\n"
    "\tshell-cmd <command>\n"
    "\tpll-spread <spread param>\n"
    "\tset-power-state <default|ultra|deep|sleep|ref|half|full>\n"
    "\tget-power-state\n"
    "\tacd <port #> <command>\n"
    "\tvpd <spi #> <mac address in xxxxxxxxxxxx format> <serial number>\n";

extern cmd_result_t cmd_robo_aps(int unit, args_t *a);

#endif /* INCLUDE_APS_DIAG_LIBS */


char if_robo_ipg_usage[] =
    "Parameters: [PortBitMap=<pbmp>] [SPeed=10|100|1000]\n\t"
    "[FullDuplex=true|false]\n\t"
    "Get the IPG register values.\n\t"
    "If no args are given, displays all.\n";

extern cmd_result_t if_robo_ipg(int unit, args_t *a);

char if_robo_l2_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  l2 <option> [args...]\n"
#else    
    "  l2 add [PortBitMap=<pbmp>] [MACaddress=<mac>] [Vlanid=<id>]\n\t"
    "         [PRIority=<cosq>|<pri>] [STatic=true|false] [HIT=true|false]\n\t"
    "         [CPUmirror=true|false] [DiscardSource=true|false]\n\t"
    "         [DiscardDest]=true|false] [Replace=true|false]\n\t"
    "         [Pending=true|false] [VPort=<id>]\n\t"
    "        - Add incrementing L2 addresses associated with port(s)\n\t"
    "  l2 replace [Module=<n>] [Port=<port>] [MACaddress=<mac>]\n\t"
    "             [Vlanid=<id>] [Trunk=true|false] [TrunkGroupId=<id>]\n\t"
    "             [STatic=true|false] [STOnly=true|false]\n\t"
    "             [MCOnly=true|false] [MCast=true|false]\n\t"
    "             [Pending=true|false]\n\t"
    "             [NewModule=<n>] [NewPort=<port>] [NewTrunkGroupId=<id>]\n\t"
    "        - STOnly, MCast, MCOnly are flags for fast aging control.\n\t"
    "  l2 del [MACaddress=<mac>] [Count=<value>] [Vlanid=<id>]\n\t"
    "         [Pending=true|false] \n\t"
    "        - Delete incrementing L2 address(s)\n\t"
    "  l2 show [PortBitMap=<pbmp]\n\t"
    "        - Show L2 addresses associated with port(s)\n\t"
    "  l2 clear [Port=<port>] [MACaddress=<mac>]\n\t"
    "           [Vlanid=<id>] [TrunkGroupID=<id>] [Static=true|false]\n\t"
    "        - Remove all L2 entries on the given module, module/port,\n\t"
    "           VLAN, or trunk group ID\n\t"    
    "  l2 learn control set [PortBitMap=<pbmp>] [SALearn=<true|false>]\n\t"
    "           [CPU=<true|false] [FWD=<true|false]\n\t"
    "           [SwLearing=<true|false]\n\t"
    "        - Configuring the L2 Learn realted features: \n\t"
    "  l2 learn control show [PortBitMap=<pbmp>]\n\t"
    "        - show L2 learning control on the indicated ports \n\t"
    "  l2 dump\n\t"
    "        - Dump all entries in L2 table\n\t"
    "  l2 conflict [MACaddress=<mac>] [Vlanid=<id>]\n\t"
    "        - Dump all conflicting L2 entries (same hash bucket)\n\t"
    "  l2 watch [start | stop]\n\t"
    "        - Watch dynamic address insertions/deletions\n\t"
    "  l2 cache add [CacheIndex=<index>] [MACaddress=<mac>]\n\t"
    "           [PortBitMap=<pbmp>] [DestPorts==true|false]\n\t"
    "        - Add L2 cache entry\n\t"
    "  l2 cache del CacheIndex=<index> [Count=<value>]\n\t"
    "        - Delete L2 cache entry\n\t"
    "  l2 cache show\n\t"
    "        - Show L2 cache entries\n\t"
    "  l2 cache clear\n\t"
    "        - Delete all L2 cache entries\n"
#endif
    ;

extern cmd_result_t if_robo_l2(int unit, args_t *a);

char if_robo_linkscan_usage[] =
    "Parameters: [SwPortBitMap=<pbmp>] [HwPortBitMap=<pbmp>]\n\t"
    "[Interval=<usec>] [FORCE=<pbmp>]\n"
#ifndef COMPILER_STRING_CONST_LIMIT    
    "\tWith no arguments, displays the linkscan status for all ports.\n\t"
    "Enables software linkscan on ports specified by SwPortBitMap.\n\t"
    "Enables hardware linkscan on ports specified by HwPortBitMap.\n\t"
    "Disables linkscan on all other ports.\n\t"
    "Interval specifies non-default linkscan interval for software.\n\t"
    "Note: With linkscan disabled, autonegotiated ports will NOT have\n\t"
    "the MACs updated with speed/duplex..\n\t"
    "FORCE=<pbmp> requests linkscan to update ports once even if link\n\t"
    "status did not change.\n"
#endif
    ;
extern cmd_result_t if_robo_linkscan(int unit, args_t *a);

char cmd_robo_mem_list_usage[] =
    "Parameters: <TABLE>\n\t"
    "If no parameters are given, displays a reference list of all\n\t"
    "memories and their attributes.\n\t"
    "If TABLE is given, displays the entry fields for that table.\n";

extern cmd_result_t cmd_robo_mem_list(int unit, args_t *a);

char cmd_robo_reg_list_usage[] =
    "1. Parameters: <REGISTER|PAGE ADDRESS> [<VALUE>]\n\t"
    "Lists all register fields in <REGISTER>\n\t"
    "<REGISTER> is SOC register symbolic name.\n\t"
    "<PAGE> is SOC register page value.\n\t"
    "<ADDRESS> is SOC register address value.\n\t"
    "2. Parameters: <SUBSTRING>\n\t"
    "Prints all SOC register names containing substring.\n\t"
    "3. Parameters: *\n\t"
    "Prints all SOC registers.\n";
extern cmd_result_t cmd_robo_reg_list(int unit, args_t *a);

char if_robo_mcast_usage[] =
    "Usages:\n\t"
    "  mcast add <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "                  <Index>=<val>\n\t"
    "  mcast delete <MACaddress>=<val> <Vlanid>=<val>\n\t"
    "  mcast join <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast leave <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast padd <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast premove <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast pget <MACaddress>=<val> <Vlanid>=<val>\n";

extern cmd_result_t if_robo_mcast(int unit, args_t *a);

char if_robo_mirror_usage[] =
    "Usages:\n"
    "  mirror Mode=<L2|Off>        -Mirror L2 or none\n"
    "  mirror MPortBitMap=<pbmp>   -Set the mirror destination port bitmap\n\t"
    "                       (single or multiple ports if chip supported)\n"
    "  mirror IngressBitMap=<pbmp> -PortBitMap with ingress mirroring enabled\n"
    "  mirror EgressBitMap=<pbmp>  -PortBitMap with egress mirroring enabled\n\n"
    "  mirror mtp dump_sw          -Dump mirror-to port software information\n"
    "  mirror mtp dump_hw          -Dump mirror-to port hardware information\n";
extern cmd_result_t if_robo_mirror(int unit, args_t *a);


char cmd_robo_mem_modify_usage[] =
    "Parameters: <TABLE> <ENTRY> <ENTRYCOUNT>\n\t"
    "        <FIELD>=<VALUE>[,...]\n\t"
    "Read/modify/write field(s) of a table entry(s).\n";

extern cmd_result_t cmd_robo_mem_modify(int unit, args_t *a);

char cmd_robo_reg_mod_usage[] =
    "Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
    "<REGISTER> is SOC register symbolic name.\n\t"
    "<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
    "Fields not specified in the list are left unaffected.\n";
extern cmd_result_t cmd_robo_reg_mod(int unit, args_t *a);

char cmd_robo_pbmp_usage[] =
    "Parameters: <pbmp>\n"
#ifndef COMPILER_STRING_CONST_LIMIT    
    "Converts a pbmp string into a hardware port bitmap.  A pbmp string\n\t"
    "is a single port, or a group of ports specified in a list using ','\n\t"
    "to separate them and '-' for ranges, e.g. 1-8,25,cpu.  Ports may be\n\t"
    "specified using the one-based port number (1-29) or port type and\n\t"
    "zero-based number (fe0-fe23,ge0-ge7).  'cpu' is equal to port 24,\n\t"
    "'fe' is all FE ports, 'ge' is all GE ports, 'e' is all ethernet\n\t"
    "ports, 'all' is all ports, and 'none' is no ports (0x0).\n\t"
    "A '~' may be used to exclude port previously given (e.g. e,~fe19)\n\t"
    "Acceptable strings and values also depend on the chip being used.\n\t"
    "A pbmp may also be given as a raw hex (0x) number, e.g. 0xbffffff.\n"
#endif
    ;
extern cmd_result_t cmd_robo_pbmp(int unit, args_t *a);
                     
char if_robo_phy_usage[] =
    "Parameters: [int] <ports> <regnum> [<value>]\n\t"
    "                 raw <mii-addr> <regnum> [<value>]\n"
    "            info\n"
    "            power <ports> [mode=mode_type [sleep_time=value] "
                               "[wake_time=value]]\n"
#ifndef COMPILER_STRING_CONST_LIMIT    
    "\tSet or display PHY registers.  If only <ports> is specified,\n\t"
    "then registers for those ports' PHYs are displayed. <ports> is a\n\t"
    "standard port bitmap, e.g. fe for all 10/100 ports, fe5-fe7 for\n\t"
    "three FE's, etc. (see \"help pbmp\").  If the int option is given,\n\t"
    "the intermediate PHY for the port is used instead of the outer PHY.\n\t"
    "In 'raw' mode, the direct mii-address can be specified, only\n\t"
    "'writing' is supported.\n"
    "Info     - display PHY device ID and attached PHY drivers\n\t" 
    "power    - set or display power mode for the PHY devices implemented\n\t"
    "           power control. mode_type has these values: auto_down,\n\t"
    "           full. sleep_time and wake_time only applies to\n\t"
    "           auto_down power mode\n" 
#endif
    ;
extern cmd_result_t if_robo_phy(int unit, args_t *a);

char if_robo_port_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
    "Usage: port <option> [args...]\n"
#else    
    "Parameters: <ports> [AutoNeg=on|off] [enable] [disable]\n\t"
    "[STP=Disable|Block|LIsten|LEarn|Forward] [detach] [probe] [attach]\n\t"
    "[LinkScan=on|off|hw|sw] [SPeed=10|100|1000] [FullDuplex=true|false]\n\t"
    "[TxPAUse=on|off RxPAUse=on|off] [DISCard=none|untag|all]\n\t"
    "[PRIOrity=<0-7>] [PHymaster=<Master|Slave|Auto>]\n\t"
    "[FrameMaxsize=<value>]\n\t"
    "If only <ports> is specified, characteristics for that port are\n\t"
    "displayed. <ports> is a standard port bitmap (see \"help pbmp\").\n\t"
    "If AutoNeg is on, SPeed and DUPlex are the ADVERTISED MAX values.\n\t"
    "If AutoNeg is off, SPeed and DUPlex are the FORCED values.\n\t"
    "SPeed of zero indicates maximum speed.\n\t"
    "LinkScan enables automatic scanning for link changes with updating\n\t"
    "of MAC registers.\n\t"
    "PAUse enables send/receive of pause frames in full duplex mode.\n\t"
    "PRIOrity sets the priority for untagged packets coming on this port.\n"
#endif
    ;
extern cmd_result_t if_robo_port(int unit, args_t *a);

char if_robo_port_rate_usage[] =
    "Set/Display port rate metering characteristics.\n"
    "Parameters: <pbm> [ingress|egress|queue_egress [arg1 arg2 <arg3>]]\n\t"
    "    If no parameters, show status of all ports\n\t"
    "    For Ingress or Egress: arg1 is rate, arg2 is max_burst\n\t"
    "    For Pause: arg1 is pause_thresh, arg2 is resume_thresh\n\t"
#ifdef BCM_TB_SUPPORT
    "    For Egress: arg3 is the egress rate type : 0:kbps, 1:pps\n\t"
#endif
    "    For Queue_Egress: arg1 is queue id, arg2 is rate, arg3 is max_burst, \n\t"
    "    rate is in kilobits (1000 bits) per second\n\t"
    "    max_burst are in kilobits (1000 bits)\n";
extern cmd_result_t if_robo_port_rate(int unit, args_t *a);

char if_robo_port_stat_usage[] =
    "Display info about port status in table format.\n"
    "    Link scan modes:\n"
    "        SW = software\n"
    "        HW = hardware\n"
    "    Learn operations (source lookup failure control):\n"
    "        F = SLF packets are forwarded\n"
    "        C = SLF packets are sent to the CPU\n"
    "        A = SLF packets are learned in L2 table\n"
    "    Pause:\n"
    "        TX = Switch will transmit pause packets\n"
    "        RX = Switch will obey pause packets\n";
extern cmd_result_t if_robo_port_stat(int unit, args_t *a);

char cmd_robo_reg_set_usage[] =
    "1. Parameters: [<REGTYPE>] <REGISTER|arg1 arg2> <VALUE>\n\t"
    "\tIf <REGTYPE> is not specified, it defaults to \"soc\".\n\t"
    "\t<REGISTER> is SOC register symbolic name.\n\t"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\tIf <REGTYPE> is SOC - <arg1> is SOC register page value.\n\t"
    "\t                      <arg2> is SOC register address value.\n\t"
    "\tIf <REGTYPE> is PHY - <arg1> is phy address value.\n\t"
    "\t                      <arg2> is register offset value.\n\t"
#endif
    "2. Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
    "\t<REGISTER> designates symbolic name.\n\t"
    "\t<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
    "\tfor example: L3_ENA=0,CPU_PRI=1.\n\t"
    "\tFields not specified in the list are set to zero.\n\t"
    "For a list of register types, use \"help dump\".\n";
extern cmd_result_t cmd_robo_reg_set(int unit, args_t *a);

char if_robo_pvlan_usage[] =
    "Usages:\n\t"
    "  pvlan show <pbmp>\n\t"
    "        - Show PVLAN info for these ports.\n\t"
    "  pvlan set <pbmp> <vid>\n\t"
    "        - Set default VLAN tag for port(s)\n\t"
    "          Port bitmaps are read from the VTABLE entry for the VID.\n\t"
    "          <vid> must have been created and all ports in <pbmp> must\n\t"
    "          belong to that VLAN.\n";
extern cmd_result_t if_robo_pvlan(int unit, args_t *a);

char cmd_robo_rate_usage[] =
    "Parameters:[PortBitMap=<pbm>] [Limit=<limit>]\n\t"
    "[Bcast=<1|0>] [Mcast=<1|0>] [Dlf=<1|0>]\n\t"
    "Enables the specified packet rate controls.\n\t"
    "  pbm       port(s) to set up or display\n\t"
    "  Limit     kilo bits per second\n\t"
    "  Bcast     Enable broadcast rate control\n\t"
    "  Mcast     Enable multicast rate control\n\t"
    "  Dlf       Enable DLF flooding rate control\n\t"
    "<limit> = 0, Bcast/Mcast/Dlf = 1: Disable Bcast/Mcast/Dlf suppression\n\t"
    "<limit> > 0, Bcast/Mcast/Dlf = 1: Enable Bcast/Mcast/Dlf suppression\n";
extern cmd_result_t cmd_robo_rate(int unit, args_t *a);

char cmd_robo_rx_cfg_usage[] =
    "Usages:\n\t"
    "rxcfg [<chan>] [options...]\n\t"
#ifndef COMPILER_STRING_CONST_LIMIT    
    "    With no options, displays current configuration\n\t"
    "    Global options include:\n\t"
    "        SPPS=<n>            Set system-wide packet per second limit\n\t"
    "                            Other options combined this are ignored\n\t"
    "        GPPS=<n>            Set global packet per second limit\n\t"
    "        PKTSIZE=<n>         Set maximum receive packet size\n\t"
    "        PPC=<n>             Set the number of pkts/chain\n\t"
    "        BURST=<n>           Set global packet burst size\n\t"
    "        COSPPS<n>=<r>       Set the per COS rate limiting\n\t"
    "        FREE=[T|F]          Should handler free buffers?\n\t"
    "    Channel specific options include:\n\t"
    "        CHAINS=<n>          Set the number of chains for the channel\n\t"
    "        PPS=<n>             Set packet per second for channel\n\t"
    "    Global options can be given w/o a channel.  Channel options\n\t"
    "    require that a channel be specified.\n\t"
    "    The channel's burst rate is #chains * pkts/chain\n\t"
#endif
    "rxcfg reason [reason code|packet type] [cosq number]\n\t"
    "    Get or set the mapping cosq number of the reason code\n\t"
    "        reason code    The reason code name to be set or get\n\t"
    "        packet type     The packet type name to be set or get\n\t"
    "            - the packet type name can be Switched/Nonunicast/Mirror\n\t"
    "        cosq number   The value of cosq to be mapped\n"
    ;
extern cmd_result_t cmd_robo_rx_cfg(int unit, args_t *a);

char cmd_robo_rx_init_usage[] =
    "RXInit <override-unit>\n"
    "    Call bcm_rx_init on the given override unit. Ignores\n"
    "    the current unit.\n";
extern cmd_result_t cmd_robo_rx_init(int unit, args_t *a);

char cmd_robo_rx_mon_usage[] =
    "Parameters [init|start|stop|show]\n"
#ifndef COMPILER_STRING_CONST_LIMIT    
    "With no parameters, show whether or not active.\n"
    "    init:    Initialize the RX API, but don't register handler\n"
    "    start:   Call RX start with local pkt dump routine\n"
    "             Modify the configuration with the rxcfg command\n"
    "    stop:    Call RX stop\n"
    "    show:    Call RX show\n"
#endif
    ;
extern cmd_result_t cmd_robo_rx_mon(int unit, args_t *a);

char cmd_robo_show_usage[] =
    "Usages:\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  show <args>\n"
#else
/*
    "  show Pci        - Probe and display function 0 of all busses/devices\n"
*/
    "  show CHips      - Show all driver-supported device IDs\n"
    "  show Counters [Changed] [Same] [Z] [NZ] [Hex] [Raw] [<reg> | <pbmp>]\n"
    "\tDisplay all counters, or only specified regs and/or ports\n"
    "\t  Changed - include counters that changed\n"
    "\t  Same    - include counters that did not change\n"
    "\t  Z       - include counters that are zero\n"
    "\t  Nz      - include counters that are not zero\n"
    "\t  All     - same as: Changed Same Z Nz\n"
    "\t  Hex     - display counters as 64-bit hex values\n"
    "\t  Raw     - display raw 64-bit hex values, no register name(s)\n"
    "\t  ErDisc  - Only show those counters marked with Error/Discard\n"
    "\tNOTES: If neither Changed or Same is specified, Change is used.\n"
    "\t       If neither Z or Nz is specified, Nz is used.\n"
    "\t       Use All to display counters regardless of value.\n"
    "  show Statistics [pbm] [all] - SNMP accumulated statistics,all shows 0s\n"
    "  show Errors            - logged error counts for certain errors\n"
    "  show params [<chip>]   - Chip parameters (chip id or current unit)\n"
    "  show unit [<unit>]     - Unit list or unit parameters\n"
    "  show features [all]    - Show enabled (or all) features for this unit\n"
#ifdef  BCM_TB_SUPPORT
    "  show VlanMapping    - Show last hit IVM/EVM entry IDs (TB BX and VO only)\n"
#endif /* BCM_TB_SUPPORT */
#if defined(VXWORKS)
    "  show ip                - IP statistics\n"
    "  show icmp              - ICMP statistics\n"
    "  show arp               - ARP statistics\n"
    "  show udp               - UDP statistics\n"
    "  show tcp               - TCP statistics\n"
    "  show mux               - MUX protocol stack backplane\n"
    "  show routes            - IP routing table\n"
    "  show hosts             - IP host table\n"
#endif /* VXWORKS */
#endif
    ;
extern cmd_result_t cmd_robo_show(int unit, args_t *a);


char cmd_robo_soc_usage[] =
    "Parameters: [<unit #>] ... \n\t"
    "Print internal SOC driver control information IF compiled as a \n\t"
    "debug version. If not compiled as a debug version, a warning is\n\t"
    "printed and the command completes successfully with no further\n\t"
    "output\n";
extern cmd_result_t cmd_robo_soc(int unit, args_t *a);

char if_robo_stg_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  stg <option> [args...]\n"
#else    
    "  stg create [<id>]            - Create a STG; optionally specify ID\n\t"
    "  stg destroy <id>             - Destroy a STG\n\t"
    "  stg show [<id>]              - List STG(s)\n\t"
    "  stg add <id> <vlan_id> [...]     - Add VLAN(s) to a STG\n\t"
    "  stg remove <id> <vlan_id> [...]  - Remove VLAN(s) from a STG\n\t"
    "  stg clear <id>          - Remove ALL VLAN(s) from a STG\n\t"
    "  stg stp                      - Get span tree state, all ports/STGs\n\t"
    "  stg stp <id>                 - Get span tree state of ports in STG\n\t"
    "  stg stp <id> <pbmp> <state>  - Set span tree state of ports in STG\n\t"
    "                                 (disable/block/listen/learn/forward)\n\t"
    "  stg default [<id>]           - Show or set the default STG\n"
#endif
    ;
extern cmd_result_t if_robo_stg(int unit, args_t *a);

char if_robo_trunk_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  trunk <option> [args...]\n"
#else    
    "  trunk init\n\t"
    "        - Initialize trunking function\n\t"
    "  trunk deinit\n\t"
    "        - Deinitialize trunking function\n\t"
    "  trunk add <Id=val> <Rtag=val> <Pbmp=val>\n\t"
    "        - Add ports to a trunk\n\t"
    "  trunk remove <Id=val> <Pbmp=val>\n\t"
    "        - Remove ports from a trunk\n\t"
    "  trunk show [<Id=val>]\n\t"
    "        - Display trunk information\n\t"
    "  trunk psc <Id=val> <Rtag=val>\n\t"
    "        - Change Rtag(for testing ONLY)\n\t"
    "  trunk mcast <Id=val> <Mac=val> <Vlan=val>\n\t"
    "        - Join multicast to a trunk\n"
#endif
    ;
extern cmd_result_t if_robo_trunk(int unit, args_t *a);

char cmd_robo_tx_usage[] =
    "Parameters: <Count> [options]\n"
#ifndef COMPILER_STRING_CONST_LIMIT    
    "  Transmit the specified number of packets, if the contents of the\n"
    "  packets is important, they may be specified using options.\n"
    "  Supported options are:\n"
    "      PortBitMap=<pbmp>   - Specify port bitmap packet is sent to.\n"
    "      UntagBitMap=<pbmp>  - Specify untag bitmap used for DMA.\n"
    "      Length=<value>      - Specify the total length of the packet,\n"
    "                            including header, possible tag, and CRC.\n"
    "      VLantag=<value>     - Specify the VLAN tag used, only the low\n"
    "                            order 16-bits are used (VLANID=0 for none)\n"
    "      Pattern=<value>     - Specify 32-bit data pattern used.\n"
    "      PatternInc=<value>  - Value by which each word of the data\n"
    "                            pattern is incremented\n"
    "      PerPortSrcMac=[0|1] - Associate specific (different) src macs\n"
    "                            with each source port.\n"
    "      SourceMac=<value>   - Source MAC address in packet\n"
    "      SourceMacInc=<val>  - Source MAC increment\n"
    "      DestMac=<value>     - Destination MAC address in packet.\n"
    "      DestMacInc=<vale>   - Destination MAC increment.\n"
    "      CallbackFunction=[0|1] - Register tx callback function.\n"
#ifdef BCM_TB_SUPPORT
    "  - Thunderbolt only:\n"
    "      TrafficClass=<val>  - Traffic Class (0-15)\n"
    "      McastGroupID=<val>  - Multicast Group ID. \n"
    "                            Take effect when PortBitMap=0x0 (assigned NULL).\n"
    "      DropPrecedence=<val>- Drop Precendence.(0:green/1:yellow/2:red/3:preserve)\n"
    "      FlowID=<value>      - Flow ID\n"
    "      FilTeRs=<value>     - Enable Filter(s) with bitmap\n"
    "                            0x01: LAG filter;         0x02: Tag filter\n"
    "                            0x04: Port Mask filter;   0x08: STP filter\n"
    "                            0x10: EAP filter;         0x20: Ingress VLAN filter\n"
    "                            0x40: Egress VLAN filter; 0x80: SA filter\n"
    "      Note: CPU loopback can be done by PortBitMap=cpu.\n"
#endif /* BCM_TB_SUPPORT */
#ifdef BCM_POLAR_SUPPORT
    "  - Polar only:\n"
    "      TrafficClass=<val>  - Traffic Class (0-7)\n"
#endif /* BCM_POLAR_SUPPORT */
#endif
    ;

extern cmd_result_t cmd_robo_tx(int unit, args_t *a);


char cmd_robo_tx_count_usage[] =
    "Parameters: None\n\t"
    "Print current request count and set count values for an active\n\t"
    "TXSTART command.\n";
extern cmd_result_t cmd_robo_tx_count(int unit, args_t *a);
extern cmd_result_t cmd_robo_tx_start(int unit, args_t *a);

char cmd_robo_tx_stop_usage[] =
    "Parameters: None\n\t"
    "Terminate a background TXSTART command that is currently running.\n\t"
    "This command only requests termination, the background thread\n\t"
    "looks for termination requests BETWEEN sending packets.\n";
extern cmd_result_t cmd_robo_tx_stop(int unit, args_t *a);

char if_robo_vlan_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  vlan <option> [args...]\n"
#else    
    "  vlan create <id> [PortBitMap=<pbmp> UntagBitMap=<pbmp>]\n\t"
    "                                       - Create a VLAN\n\t"
    "  vlan destroy <id>                    - Destroy a VLAN\n\t"
    "  vlan clear                           - Destroy all VLANs\n\t"
    "  vlan add <id> [PortBitMap=<pbmp> UntagBitMap=<pbmp>\n\t"
    "                                       - Add port(s) to a VLAN\n\t"
    "  vlan remove <id> [PortBitMap=<pbmp>] - Remove ports from a VLAN\n\t"
    "  vlan MulticastFlood <id>  [Mode]     - Multicast flood setting\n\t"
    "  vlan show                            - Display all VLANs\n\t"
    "  vlan default [<id>]                  - Show or set the default VLAN\n\t"
#ifdef BCM_PROTOCOL2V_SUPPORT
    "  vlan protocol [enable|disable] PortBitMap=<pbmp>\n\t"
    "  vlan protocol add PortBitMap=<pbmp> Frame=<N> Ether=<N>\n\t"
    "        VLan=<vlanid> Prio=<prio>\n\t"
    "  vlan protocol delete PortBitMap=<pbmp> Frame=<N> Ether=<N>\n\t"
    "  vlan protocol clear PortBitMap=<pbmp>\n\t"
#endif  /* BCM_PROTOCOL2V_SUPPORT */
#ifdef BCM_MAC2V_SUPPORT
    "  vlan mac [enable|disable] PortBitMap=<pbmp>\n\t"
    "  vlan mac add MACaddress=<address> VLan=<vlanid> Prio=<prio>\n\t"
    "  vlan mac delete MACaddress=<address>\n\t"
    "  vlan mac clear\n\t"
    "  vlan mac show MACaddress=<address>\n\t"
#endif  /* BCM_MAC2V_SUPPORT */
#ifdef BCM_V2V_SUPPORT
    "  vlan translate [enable|disable] PortBitMap=<pbmp>\n\t"
    "  vlan translate add Port=<port> OldVLan=<vlanid> NewVLan=<vlanid> dtag=<T|F>\n\t"
    "  vlan translate get Port=<port> OldVLan=<vlanid> \n\t"
    "  vlan translate show              - Shows all vlan translations \n\t"
    "  vlan translate delete Port=<port> OldVLan=<vlanid>\n\t"
    "  vlan translate clear\n\t"
    "  vlan translate range add Port=<port> OldVLanHi=<vlanid> \n\t"
    "        OldVlanLo=<vlanid> NewVLan=<vlanid> Prio=<prio> Cng=<cng>\n\t"
    "  vlan translate range get Port=<port> OldVLanHi=<vlanid>\n\t"
    "        OldVlanLo=<vlanid>\n\t"
    "  vlan translate range delete Port=<port> OldVLanHi=<vlanid>\n\t"
    "        OldVlanLo=<vlanid>\n\t"
    "  vlan translate range clear\n\t"
    "  vlan translate range show\n\t"
    "  vlan translate egress add Port=<port> OldVLan=<vlanid> NewVLan=<vlanid>\n\t"
    "        Prio=<prio> Cng=<cng>\n\t"
    "  vlan translate egress get Port=<port> OldVLan=<vlanid> \n\t"
    "  vlan translate egress delete Port=<port> OldVLan=<vlanid>\n\t"
    "  vlan translate egress clear\n\t"
    "  vlan translate dtag add Port=<port> OldVLan=<vlanid> NewVLan=<vlanid>\n\t"
    "        Prio=<prio> Cng=<cng>\n\t"
    "  vlan translate dtag get Port=<port> OldVLan=<vlanid> \n\t"
    "  vlan translate dtag show     - Shows all double tagged vlans \n\t"
    "  vlan translate dtag delete Port=<port> OldVLan=<vlanid>\n\t"
    "  vlan translate dtag clear\n\t"
    "  vlan translate dtag range add Port=<port> OldVLanHi=<vlanid> \n\t"
    "        OldVlanLo=<vlanid> NewVLan=<vlanid> Prio=<prio> Cng=<cng>\n\t"
    "  vlan translate dtag range get Port=<port> OldVLanHi=<vlanid>\n\t"
    "        OldVlanLo=<vlanid>\n\t"
    "  vlan translate dtag range delete Port=<port> OldVLanHi=<vlanid>\n\t"
    "        OldVlanLo=<vlanid>\n\t"
    "  vlan translate dtag range clear\n\t"
    "  vlan translate dtag range show\n\t"
    "  vlan action port default add Port=<port> OuterVlan=<vlanid>\n\t"
    "        InnerVlan=<vlanid> Prio=<prio> DtOuter=<action>\n\t"
    "        DtOuterPrio=<action> DtInner=<action> DtInnerPrio=<action>\n\t"
    "        OtOuter=<action> OtOuterPrio=<action> OtInner=<action>\n\t"
    "        OtOuter=<action> ItOuter=<action> ItInner=<action>\n\t"
    "        ItInnerPrio=<action> UtOuter=<action> UtInner=<action>\n\t"
    "        OuterPcp=<action> InnerPcp=<action>\n\t"
    "        - Add port default VLAN tag with actions.\n\t"
    "          Valid vlan tag actions are { Add, Delete, Replace, None }\n\t"
    "          Valid vlan pcp actions are { Mapped, IngressInnerPcp, IngressOuterPcp, PortDefault, None }\n\t"
    "  vlan action port default get Port=<port>\n\t"
    "        - Get port default VLAN tag actions\n\t"
    "  vlan action port default delete Port=<port>\n\t"
    "        - Delete port default VLAN tag actions\n\t"
    "  vlan action port egress default add Port=<port> OuterVlan=<vlanid>\n\t"
    "        InnerVlan=<vlanid> Prio=<prio> DtOuter=<action>\n\t"
    "        DtOuterPrio=<action> DtInner=<action> DtInnerPrio=<action>\n\t"
    "        OtOuter=<action> OtOuterPrio=<action> OtInner=<action>\n\t"
    "        OtOuter=<action> ItOuter=<action> ItInner=<action>\n\t"
    "        ItInnerPrio=<action> UtOuter=<action> UtInner=<action>\n\t"
    "        OuterPcp=<action> InnerPcp=<action>\n\t"
    "        - Add port egress VLAN tag with actions.\n\t"
    "          Valid vlan tag actions are { Add, Delete, Replace, None }\n\t"
    "          Valid vlan pcp actions are { Mapped, IngressInnerPcp, IngressOuterPcp, PortDefault, None }\n\t"
    "  vlan action port egress default get Port=<port>\n\t"
    "        - Get port egress VLAN tag actions\n\t"
    "  vlan action port egress default delete Port=<port>\n\t"
    "        - Delete port egress VLAN tag actions\n\t"
    "  vlan action protocol add PortBitMap=<pbmp> Frame=<N> Ether=<N>\n\t"
    "        OuterVlan=<vlanid> InnerVlan=<vlanid> Prio=<prio> \n\t"
    "        DtOuter=<action> DtOuterPrio=<action> DtInner=<action> \n\t"
    "        DtInnerPrio=<action> OtOuter=<action> OtOuterPrio=<action>\n\t"
    "        OtInner=<action> OtOuter=<action> ItOuter=<action>\n\t"
    "        ItInner=<action> ItInnerPrio=<action> UtOuter=<action> \n\t"
    "        UtInner=<action>\n\t"
    "        OuterPcp=<action> InnerPcp=<action>\n\t"
    "        - Add protocol-based VLAN tag with actions.\n\t"
    "          Valid actions are { Add, Delete, Replace, None }\n\t"
    "          Valid vlan pcp actions are { Mapped, IngressInnerPcp, IngressOuterPcp, PortDefault, None }\n\t"
    "  vlan action protocol delete PortBitMap=<pbmp> Frame=<N> Ether=<N>\n\t"
    "        - Delete VLAN port protocol actions.\n\t"
    "  vlan action protocol get PortBitMap=<pbmp> Frame=<N> Ether=<N>\n\t"
    "        - Get VLAN port protocol actions.\n\t"
    "  vlan action protocol show\n\t"
    "        - Show all VLAN port protocol actions.\n\t"
    "  vlan action protocol clear PortBitMap=<pbmp>\n\t"
    "        - Delete all VLAN port protocol actions.\n\t"
    "  vlan action translate add Port=<port> KeyType=<key> \n\t"
    "        OldOuterVlan=<vlanid> OldInnerVlan=<vlanid> \n\t"
    "        OuterVlan=<vlanid> InnerVlan=<vlanid> Prio=<prio> \n\t"
    "        DtOuter=<action> DtOuterPrio=<action> DtInner=<action>\n\t"
    "        DtInnerPrio=<action> OtOuter=<action> OtOuterPrio=<action>\n\t"
    "        OtInner=<action> OtOuter=<action> ItOuter=<action>\n\t"
    "        ItInner=<action> ItInnerPrio=<action> UtOuter=<action>\n\t"
    "        UtInner=<action>\n\t"
    "        OuterPcp=<action> InnerPcp=<action>\n\t"
    "        - Add VLAN tag translation with actions.\n\t"
    "          Valid vlan tag actions are { Add, Delete, Replace, None }\n\t"
    "          Valid key types are { Double, Outer, Inner, OuterTag, \n\t"
    "          InnerTag, PortDouble, PortOuter }\n\t"
    "          Valid vlan pcp actions are { Mapped, IngressInnerPcp, IngressOuterPcp, PortDefault, None }\n\t"
    "  vlan action translate delete Port=<port> KeyType=<key> \n\t"
    "        OldOuterVlan=<vlanid> OldInnerVlan=<vlanid>\n\t"
    "        - Delete VLAN tag translation actions.\n\t"
    "  vlan action translate get Port=<port> KeyType=<key> \n\t"
    "        OldOuterVlan=<vlanid> OldInnerVlan=<vlanid>\n\t"
    "        - Get VLAN tag translation actions.\n\t"
    "  vlan action translate show\n\t"
    "        - Show all VLAN tag translation actions.\n\t"
    "  vlan action translate range add Port=<port> \n\t"
    "        OuterVlanLo=<vlanid> OuterVlanHi=<vlanid> \n\t"
    "        InnerVlanLo=<vlanid> InnerVlanHi=<vlanid>\n\t"
    "        OuterVlan=<vlanid> InnerVlan=<vlanid> Prio=<prio> \n\t"
    "        DtOuter=<action> DtOuterPrio=<action> DtInner=<action>\n\t"
    "        DtInnerPrio=<action> OtOuter=<action> OtOuterPrio=<action>\n\t"
    "        OtInner=<action> OtOuter=<action> ItOuter=<action>\n\t"
    "        ItInner=<action> ItInnerPrio=<action> UtOuter=<action>\n\t"
    "        UtInner=<action>\n\t"
    "        OuterPcp=<action> InnerPcp=<action>\n\t"
    "        - Add VLAN tag range translation with actions.\n\t"
    "          Valid vlan tag actions are { Add, Delete, Replace, None }\n\t"
    "          Valid vlan pcp actions are { Mapped, IngressInnerPcp, IngressOuterPcp, PortDefault, None }\n\t"
    "  vlan action translate range delete Port=<port> \n\t"
    "        OuterVlanLo=<vlanid> OuterVlanHi=<vlanid> \n\t"
    "        InnerVlanLo=<vlanid> InnerVlanHi=<vlanid>\n\t"
    "        - Delete VLAN tag translation range actions.\n\t"
    "  vlan action translate range get Port=<port> \n\t"
    "        OuterVlanLo=<vlanid> OuterVlanHi=<vlanid> \n\t"
    "        InnerVlanLo=<vlanid> InnerVlanHi=<vlanid>\n\t"
    "        - Get VLAN tag translation range actions.\n\t"
    "  vlan action translate range clear\n\t"
    "        - Delete all VLAN tag translation range actions.\n\t"
    "  vlan action translate range show\n\t"
    "        - Show all VLAN tag translation range actions.\n\t"
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT)
    "  vlan action translate egress add PortClass=<class> Special=<0 | 1>\n\t"
    "        OldOuterVlan=<vlanid> OuterVlan=<vlanid> InnerVlan=<vlanid>\n\t"
    "        DtOuter=<action> DtInner=<action>\n\t"
    "        - Special at 1 to add bcm53115's EgressVidRemark(EVR) table\n\t"
    "           and the OldOuterVlan will be ignored.\n\t"
    "        - Special at 0 to create bcm53115's VLAN translation \n\t"
    "           solution with mapping or transparent mode.\n\t"
    "        - PortClass indicats the EVR entry id when Special=1 and\n\t"
    "           indicats local port id when Special=0.\n\t"
    "        - OldOuterVlan will be ignored when Special=1.\n\t"
    "        - Valid actions are { Add, Delete, Replace, None }\n\t"
    "  vlan action translate egress delete PortClass=<class> Special=<0 | 1>\n\t"
    "        OldOuterVlan=<vlanid> OuterVlan=<vlanid>\n\t"
    "        - Delete VLAN tag egress translation actions.\n\t"
    "        - Special at 1 to delete bcm53115's EgressVidRemark(EVR) table\n\t"
    "           and the OldOuterVlan will be ignored.\n\t"
    "        - Special at 0 to destroy the bcm53115's created VLAN translation.\n\t"
    "        - PortClass indicats the EVR entry id when Special=1 and\n\t"
    "           indicats local port id when Special=0.\n\t"
    "        - OldOuterVlan and OuterVlan will be ignored when Special=1.\n\t"
    "  vlan action translate egress get PortClass=<class> Special=<0 | 1>\n\t"
    "        OldOuterVlan=<vlanid> OuterVlan=<vlanid>\n\t"
    "        - Get VLAN tag egress translation actions.\n\t"
    "        - Special at 1 to get bcm53115's EgressVidRemark(EVR) table\n\t"
    "           and the OldOuterVlan will be ignored.\n\t"
    "        - Special at 0 to get the bcm53115's created VLAN translation.\n\t"
    "        - PortClass indicats the EVR entry id when Special=1 and\n\t"
    "           indicats local port id when Special=0.\n\t"
    "        - OldOuterVlan and OuterVlan will be ignored when Special=1.\n\t"
    "  vlan action translate egress show\n\t"
    "        - Show all directly added bcm53115's EVR table entries.\n"
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT */
#endif  /* BCM_V2V_SUPPORT */
    "  vlan control <name> <value>\n\t"
    "  vlan port <pbmp> <name> <value>\n\t"
    "  vlan crossconnect add OuterVLan=<vlanid> InnerVLann=<vlanid> Port1=<port> Port2=<port>\n\t"
    "  vlan crossconnect delete OuterVLan=<vlanid> InnerVLann=<vlanid>\n\t"
    "  vlan crossconnect clear\n\t"
    "  vlan crossconnect show\n\t"
    "  vlan block add <id> [KnownMcast=<pbmp>] [UnknownMcast=<pbmp>] [UnknownUcast=<pbmp>] [Bcast=<pbmp>]\n\t"
    "  vlan block get <id>\n\t"
    "  vlan policer create PolId=<policerid> cbs=<cbs> cir=<cir>\n\t"
    "  vlan policer destory PolId=<policerid>\n\t"
    "  vlan policer attach Port=<port> Vlan=<vlanid> PolId=<policerid>\n\t"
    "  vlan policer detach Port=<port> Vlan=<vlanid>\n\t"    
#endif
    "  vlan <id> <name>=<vlaue>         - Set/Get per VLAN property\n\t"
    "  vlan dlf set vlan=<vid> bcast=<mgid> mlf=<mgid> ulf=<mgid> - Set MGIDs of DLF configuration for each packet types\n\t"
    "  vlan dlf delete vlan=<vid> - Delete MGIDs of DLF configuration for all packet types\n\t"
    "  vlan dlf show vlan=<vid> - Show DLF configuration for all packet types\n"
    ;
extern cmd_result_t if_robo_vlan(int unit, args_t *a);


char cmd_robo_mem_write_usage[] =
    "Parameters: <TABLE> <ENTRY> <ENTRYCOUNT>\n\t"
    "        { <DW0> .. <DWN> | <FIELD>=<VALUE>[,...] }\n\t"
    "Number of <DW> must be a multiple of table entry size.\n\t"
    "Writes entry(s) into table index(es).\n";
extern cmd_result_t cmd_robo_mem_write(int unit, args_t *a);

