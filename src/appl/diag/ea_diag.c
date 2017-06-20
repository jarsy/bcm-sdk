/*
 * $Id: ea_diag.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * EA socdiag command list
 */
#include "appl/diag/diag.h"

char cmd_ea_clear_usage[] =
    "clear counters [PBMP] | stats [PBMP]\n\t"
    "counters - zeroes all or some internal packet counters\n\t"
    "stats - Clear the port based statistics.\n";

extern cmd_result_t cmd_ea_clear(int unit, args_t *a);

char cmd_ea_cos_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "cos <option> [args...]\n"
#else
    "cos show                            - Show current COS config\n\t"
    "cos qcount [Port=<port>] [count=#]  - Set strict queue count\n\t"
    "cos qsize [Port=<port>] [Queue=#] [Size=#]\n\t"
    "                                    - Set queue size\n\t"
    "cos qthreshold show [Port=<port>] [Queue=#]\n\t"
    "                                    - Show DBA report threshold\n\t"
    "cos qthreshold set  [Port=<port>] [Queue=#] [Threshold0=#]\n\t"
    "                    [Threshold1=#] [Threshold2=#] [Threshold3=#]\n\t"
    "                                    - Set DBA report threshold\n"
#endif
    ;
extern cmd_result_t cmd_ea_cos(int unit, args_t *a);

char if_ea_field_proc_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
"Parameters: <cmd> <opt>\n"
#else /* !COMPILER_STRING_CONST_LIMIT */
" Where <cmd> is:\n"
"\taction <add> <eid> [act] [p0] [p1]\n"
"\taction <get|remove> <eid> [act]\n"
"\tdetach\n"
"\tentry create <gid> [<eid>]\n"
"\tentry copy <src_eid> [<dst_eid>]\n"
"\tentry install|reinstall|remove|destroy <eid>\n"
"\tentry prio <eid> [highest|lowest|default|<priority>] (priority: 1-15)\n"
"\tentry inport <eid> [<port>] (port: 0 - 10)\n"
"\tgroup create <pri> [<gid>]\n"
"\tgroup destroy|get|set|status <gid>\n"
"\tinit\n"
"\trange create [<rid>] [<flags>] [<min>] [<max>]\n"
"\trange group create [<rid>] [<flags>] [<min>] [<max>] [<group>]\n"
"\trange get|destroy <rid>\n"
"\tqual <eid> <QUAL> data mask\n"
"\tqual <eid> get <QUAL>\n"
"\tqual <eid> delete <QUAL>\n"
"\tqual <eid> clear\n"
"\tqset add|set|clear|show [qualifier]\n"
"\tshow [group|entry] [id]\n"
#endif /* !COMPILER_STRING_CONST_LIMIT */
;
extern cmd_result_t if_ea_field_proc(int unit, args_t *a);

char if_ea_l2_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  l2 <option> [args...]\n"
#else
    "  l2 add [Port=<port>] [MACaddress=<mac>]\n\t"
    "        - Add incrementing L2 addresses associated with port\n\t"
    "  l2 replace [Port=<port>] [MACaddress=<mac>] [NewPort=<port>]\n\t"
    "        - Replace L2 addresses to new port\n\t"
    "  l2 del [MACaddress=<mac>] [Port=<port>]\n\t"
    "        - Delete incrementing L2 address(s)\n\t"
    "  l2 show [Port=<port>] \n\t"
    "        - Show L2 addresses associated with port(s)\n\t"
    "  l2 clear\n\t"
    "        - Remove all L2 entries\n\t"
    "  l2 learn control [Port=<port>] [Limit=<limit>] [Age=<age>]\n\t"
    "        - Configure or show the L2 Learn realted features\n\t"
    "  l2 dump\n\t"
    "        - Dump all entries in L2 table\n"
#endif
    ;

extern cmd_result_t if_ea_l2(int unit, args_t *a);

char if_ea_linkscan_usage[] =
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
extern cmd_result_t if_ea_linkscan(int unit, args_t *a);

char if_ea_port_usage[] =
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

char cmd_ea_mem_list_usage[] =
	"Usages:\n\t"
    "mem create pool <name> <entries> <blocksize> <number>\n\t"
    "mem free pool <p_idx>\n\t"
    "mem create blk <p_idx> <size>\n\t"
    "mem free blk <p_idx> <b_idx>\n\t"
    "mem create chain <p_idx> <c_idx> <size>\n\t"
    "mem append chain <p_idx> <c_old_idx> <c_new_idx>\n\t"
    "mem free chain <idx> <c_idx>\n\t"
    "mem show\n";

extern cmd_result_t cmd_ea_mem_list(int unit, args_t *a);

char cmd_ea_reg_list_usage[] =
    "1. Parameters: <REGISTER|PAGE ADDRESS> [<VALUE>]\n\t"
    "Lists all register fields in <REGISTER>\n\t"
    "<REGISTER> is SOC register symbolic name.\n\t"
    "<PAGE> is SOC register page value.\n\t"
    "<ADDRESS> is SOC register address value.\n\t"
    "2. Parameters: <SUBSTRING>\n\t"
    "Prints all SOC register names containing substring.\n\t"
    "3. Parameters: *\n\t"
    "Prints all SOC registers.\n";
extern cmd_result_t cmd_ea_reg_list(int unit, args_t *a);

char if_ea_mcast_usage[] =
    "Usages:\n\t"
    "  mcast add <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "                  <Index>=<val>\n\t"
    "  mcast delete <MACaddress>=<val> <Vlanid>=<val>\n\t"
    "  mcast join <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast leave <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast padd <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast premove <MACaddress>=<val> <Vlanid>=<val> <PortBitMap>=<val>\n\t"
    "  mcast pget <MACaddress>=<val> <Vlanid>=<val>\n";

extern cmd_result_t if_ea_mcast(int unit, args_t *a);

extern cmd_result_t if_ea_port(int unit, args_t *a);

char cmd_ea_tx_usage[] =
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
#endif
    ;

char if_ea_port_rate_usage[] =
    "Set/Display port rate metering characteristics.\n"
    "Parameters: <pbm> [ingress|egress|queue_egress [arg1 arg2 <arg3>]]\n\t"
    "    If no parameters, show status of all ports\n\t"
    "    For Ingress or Egress: arg1 is rate, arg2 is max_burst\n\t"
#ifdef BCM_TB_SUPPORT
    "    For Egress: arg3 is the egress rate type : 0:kbps, 1:pps\n\t"
#endif
    "    For Queue_Egress: arg1 is queue id, arg2 is rate, arg3 is max_burst, \n\t"
    "    rate is in kilobits (1000 bits) per second\n\t"
    "    max_burst are in kilobits (1000 bits)\n";
extern cmd_result_t if_ea_port_rate(int unit, args_t *a);

char if_ea_port_stat_usage[] =
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
extern cmd_result_t if_ea_port_stat(int unit, args_t *a);

char if_ea_pvlan_usage[] =
    "Usages:\n\t"
    "  pvlan show <pbmp>\n\t"
    "        - Show PVLAN info for these ports.\n\t"
    "  pvlan set <pbmp> <vid>\n\t"
    "        - Set default VLAN tag for port(s)\n\t"
    "          Port bitmaps are read from the VTABLE entry for the VID.\n\t"
    "          <vid> must have been created and all ports in <pbmp> must\n\t"
    "          belong to that VLAN.\n";
extern cmd_result_t if_ea_pvlan(int unit, args_t *a);

char cmd_ea_rx_cfg_usage[] =
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
extern cmd_result_t cmd_ea_rx_cfg(int unit, args_t *a);
char cmd_ea_rx_init_usage[] =
    "RXInit <override-unit>\n"
    "    Call bcm_rx_init on the given override unit. Ignores\n"
    "    the current unit.\n";
extern cmd_result_t cmd_ea_rx_init(int unit, args_t *a);


extern cmd_result_t cmd_ea_tx(int unit, args_t *a);

char cmd_ea_tx_count_usage[] =
    "Parameters: None\n\t"
    "Print current request count and set count values for an active\n\t"
    "TXSTART command.\n";
extern cmd_result_t cmd_ea_tx_count(int unit, args_t *a);
extern cmd_result_t cmd_ea_tx_start(int unit, args_t *a);

char cmd_ea_tx_stop_usage[] =
    "Parameters: None\n\t"
    "Terminate a background TXSTART command that is currently running.\n\t"
    "This command only requests termination, the background thread\n\t"
    "looks for termination requests BETWEEN sending packets.\n";

char cmd_ea_show_usage[] =
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
extern cmd_result_t cmd_ea_show(int unit, args_t *a);

char cmd_ea_soc_usage[] =
#if defined(BCM_TK371X_SUPPORT)
    "Usages:\n\t"
    "soc debug <mask> \n\t"
    "soc dbgdump \n\t"
    "soc gpio read <flag>\n\t"
    "soc gpio write <flag> <mask> <value>\n\t"
    "soc nvserase\n\t"
    "soc reset\n\t"
	"soc upgrade <type> <name>	- (type:app;boot;pers)\n";
#else
    "Unsupported\n";
#endif    
extern cmd_result_t cmd_ea_soc(int unit, args_t *a);

char test_ea_domaen_usage[] =
	"Usage <ethernet number> \n"
	"	start\n";
extern cmd_result_t test_ea_domaen(int unit, args_t *a);

char if_ea_stg_usage[] =
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
extern cmd_result_t if_ea_stg(int unit, args_t *a);
