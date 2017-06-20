/* configAll.h - default configuration header */

/* $Id: configAll.h,v 1.8 2011/07/21 16:14:11 yshtil Exp $
 * Copyright (c) 1994-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
15g,12oct05,mmi  add light power manager
15f,07oct05,pcm  added INCLUDE_HRFS_CHKDSK (SPR 113378)
15e,28sep05,tlu  Add IP6ATTACH_IFUNIT, IP6ATTACH_IFNAME, and IFCONFIG_OPTIONS 
		 for MIP6
15d,27sep05,vvv  removed INCLUDE_NET_MICRO_STACK (SPR #112929)
15f,26sep05,yvp  Removed ISR objects from the default build (SPR 112909).
15e,21sep05,pcm  added FD_DEV_NAME, FD_0_TYPE, and FD_1_TYPE
15d,13sep05,jln  Added INCLUDE_POSIX_PTHREAD_SCHEDULER,
                 INCLUDE_CUSTOM_SCHEDULER, and INCLUDE_VX_NATIVE_SCHEDULER
15c,13sep05,dlk  Add ICMPV4_TIMESTAMP_REQ_SUPPORT_CFG parameter.
13u,08sep05,niq  Add comments for QoS parameters
15c,13sep05,h_k  increased STACK_SAVE for SH. (SPR #112259)
15b,06sep05,kk   remove INCLUDE_OBJ_OWNERSHIP from the defaults (SPR# 11865)
15e,13sep05,dbt  Added CORE_DUMP_SKIP_TEXT_SECTION & CORE_DUMP_STACK_SIZE
                 parameters.
15d,12sep05,bpn  Added new shell component parameters SHELL_TASK_PRIORITY,
                 SHELL_TASK_NAME_BASE and SHELL_TASK_OPTIONS (SPR#106622).
15c,08sep05,dcc  removed INCLUDE_TAPEFS component (SPR# 111788).
15b,10sep05,dsk  Added new TIPC infrastructure components for sockets and buffers
15a,30aug05,md   Removed EDR_ERROR_INJECT_STUBS
14z,23aug05,xli  Add mip6 apps config parameters
14y,20aug05,dlk  Added IP_MAXFRAGPACKETS_CFG and IP_MAXFRAGSPERPACKET_CFG.
14x,18aug05,kch  Added l2config component.
14w,26jul05,tcr  add WINDVIEW components to excluded section
14t,15jul05,yvp  Added INCLUDE_EDR_STUB to default configuration.
14s,13jul05,jln  Added INCLUDE_POSIX_PTHREAD_SCHED
14r,09jul05,yvp  Added INCLUDE_TASK_CREATE_DELETE, TASK_RESTART, SEM_LIB
14q,24aug05,mmi  Add CPU PWR frameWork default defines
13h,17aug05,kk   updated RTP_SIGNAL_QUEUE_SIZE to 32 for POSIX
14p,15aug05,rhe  Added INCLUDE_FORMATTED_OUT_BASIC
14o,14aug05,niq  Add the QOS components
14n,12aug05,kch  Added INCLUDE_IFCLONE, INCLUDE_MUX_L2 and INCLUDE_SUBNET_VLAN
                 components. Also added MUX-L2 default configuration params.
14m,11aug05,ijm  changed DHCP6S_RAMDISK_SIZE parameter
14l,11aug05,wap  Add INCLUDE_END_POLLED_STATS.
14k,08aug05,wap  Add INCLUDE_NET_MICRO_STACK component (off by default)
14j,06aug05,jln  removed INCLUDE_POSIX_PTHREAD_SCHED
14i,04aug05,wap  Merged in changes from freeze10: add new components
                 INCLUDE_EDR_STUB, INCLUDE_POSIX_PTHREAD_SCHED,
                 INCLUDE_TASK_CREATE_DELETE, TASK_RESTART, SEM_LIB,
                 SEM_SHOW, INCLUDE_HOOKS, MAX_REBOOT_HOOKS and
                 INCLUDE_REBOOT_HOOKS.
14h,03aug05,vvv  combined INCLUDE_SCTPV4 and INCLUDE_SCTPV6 into single
                 component
14g,01aug05,bpn  Added WDB_REGS_SIZE parameter.
14f,01aug05,yvp  Added newly created components (sem, msgQ & wd create/delete).
14e,25jul05,dlk  Added INCLUDE_NETBUFADVLIB and INCLUDE_NETPOOLSHOW.
14d,25jul05,bpn  Added INCLUDE_WDB_RTP_CONTROL.
14c,20jul05,tlu  Added INCLUDE_MIP6_MN 
14b,20jul05,kch  added SCTP configuration parameters
14a,15jul05,yvp  Added INCLUDE_EDR_STUB to default configuration.
13z,13jul05,jln  Added INCLUDE_POSIX_PTHREAD_SCHED
13y,12jul05,ijm  added DHCPv6 authentication parameters
13x,09jul05,yvp  Added INCLUDE_TASK_CREATE_DELETE, TASK_RESTART, SEM_LIB
13t,09aug05,dbt  added FATAL_SYSTEM_CORE_DUMP_ENABLE &
                 KERNEL_APPL_CORE_DUMP_ENABLE parameters.
13s,15jul05,yvp  Added INCLUDE_EDR_STUB to default configuration.
13r,13jul05,jln  Added INCLUDE_POSIX_PTHREAD_SCHED
13q,09jul05,yvp  Added INCLUDE_TASK_CREATE_DELETE, TASK_RESTART, SEM_LIB
                 SEM_SHOW, INCLUDE_HOOKS and MAX_REBOOT_HOOKS.
13s,07jul05,yvp  Added INCLUDE_REBOOT_HOOKS to default configuration.
13w,07jul05,yvp  Added INCLUDE_REBOOT_HOOKS to default configuration.
13v,07jul05,vvv  added INCLUDE_MIPV6
13u,06jul05,kch  Added MLDR_IN_VS0 define.
13t,05jul05,dlk  Change NUM_SYS_2048 default to 0. Changed NUM_SYS_MBLKS
13p,07jul05,yvp  Added INCLUDE_REBOOT_HOOKS to default configuration.
13o,04aug05,dcc  added DOSFS_DEFAULT_CACHE_SIZE parameter.
13n,04aug05,jmp  removed CORE_DUMP_NUM_MAX parameter.
13m,03aug05,pcm  added DOSFS_DEFAULT_xxx and HRFS_DEFAULT_xxx macros
13l,03aug05,dbt  Added CORE_DUMP_MAX_HOOKS_NUM parameter.
13k,29jul05,jmp  added core dump memory filtering facility.
13j,28jul05,pcm  corrected name of INCLUDE_XBD_RAMDRV
13i,27jul05,jmp  updated some core dump parameters.
13h,25jul05,jmp  added core dump support.
                 and NUM_SYS_CLBLKS default to 20.
		 Add IPINTRQ_IFQ_MAXLEN_CFG and IP6INTRQ_IFQ_MAXLEN_CFG
		 for loopback usage.
13s,05jul05,dlk  Added parameters TCPV6_MTU_MIN_CFG, ICMPV6_MTU_MIN_CFG,
		 ICMPSRC_RESULT_DEFAULT_CFG, and ICMPV4_MTU_MIN_CFG.
13r,30jun05,wap  Add reference to INCLUDE_IFMEDIA
13q,28jun05,vvv  added IGNORE_LINKLOCAL
13p,24jun05,rp   added INCLUDE_ADDRCTL_POLICY
13o,24jun05,pcm  added INCLUDE_FS_EVENT_UTIL
13n,16jun05,pcm  added file system configurable parameters
13m,13jun05,pcm  added and removed new file system components
13l,08jun05,yvp  Added INCLUDE_BASE_KERNEL to the default configuration.
13k,04jun05,yvp  Added EDR_ERROR_INJECT_STUBS, INCLUDE_OBJ_CLASS_MGMT to the 
                 default configuration.
13j,31may05,dbt  Added support for TIPC WDB communication link.
		 Added support for WDB Agent Proxy Server.
13i,24may05,kch  Added IPV6CTL_PREFER_TEMPADDR_CFG, IPV6CTL_MCAST_PMTU_CFG,
                 ND6_MAXQUEUELEN_CFG and mobility configuration parameters.
13h,17may05,kch  Fixed default values for ND6_TEMPPLTIME_CFG and
                 ND6_TEMPVLTIME_CFG (SPR#109293)
13g,17may05,ebh  Add support for distributed SNS parameters for BSP builds
13f,16may05,scm  spr 108763 -Base 6 ARM offsets moved out of page 0
13e,29apr05,rp   added mldv2 defaults
13d,26apr05,bwa  changed SM_TIPC_INSTANCE to SM_TIPC_PKT_Q_LEN.
13c,22apr05,ebh  combined the SAL/SNS components/parameters
13b,20apr05,rp   merged from comp_wn_ipv6_mld_interim-dev
13a,20apr05,gls  added INCLUDE_OBJ_OWNERSHIP (SPR #106150)
12z,18apr05,bwa  SM_CPUS_MAX default value is now DEFAULT_CPUS_MAX (SPR107525).
12y,15apr05,wap  Add WDB_END_DEVICE_ADDRESS (SPR #82098)
12x,13apr05,yvp  removed INCLUDE_VXEVENTS from the included defaults
12w,12apr05,vvv  reduced RETRY_CFG to 2 (SPR #102903); added a few 
                 component descriptions
12v,29mar05,bwa  added TIPC, SM_TIPC and SNS_MP components/parameters.
12u,24mar05,jmt  SPR 107133: Fix problem with BOOT_CMD_STACK_SIZE being too 
                 small
12t,21mar05,pcm  added INCLUDE_TAPEFS to default list of excluded components
12s,03mar05,niq  Remove NET_GETOPT
12r,27feb05,dlk  Added SO_LINGER_INFINITE_CFG parameter.
12q,26feb05,dlk  Put INCLUDE_NET_SYSCTL in excluded network facilities section
                 for documentation purposes.
12p,25feb05,pcs  Udated to have distinct task stack overflow and underflow
12o,23feb05,wap  Don't turn on INCLUDE_NET_SYSCTL by default
                 sizes.
12n,10feb05,pch  SPR 102772: handle interrupt stack protection
12m,21feb05,ebh  update manually (zippy merge was not running)
12l,31jan05,niq  merge virtual stack support from medusa
12k,27jan05,vvv  fixed ADDIF_MASK default
12j,26jan05,vvv  removed IPv4-only FTP server
12i,20jan05,syy  Updated FTPD6 option to run dual server as default
12h,17jan05,bwa  added SNS_LISTEN_BACKLOG (SPR #102955).
12g,11jan05,vvv  merged from base6_itn6_networking-int (SPR #104035)
12f,17nov04,to   PPC970 also uses USER_HASH_TABLE_* macros
12e,31oct04,pes  Added CDF_OVERRIDE to SW_MMU_ENABLE definition for MIPS.
12d,27oct04,pes  Added conditional definition of SW_MMU_ENABLE for MIPS.
12c,26oct04,bpn  Changed path delimiter characters of VXE_PATH and shell
                 configuration variables.
12b,13oct04,ann  merged fixes for SPR #102522, 5x changes for PPP,
                 SPR# 101738, NBIO LOG changes.
12a,12oct04,md   added INCLUDE_EDR_SYSDBG_FLAG to included facilities
11z,30sep04,tam  renamed INCLUDE_EDR_KH, INCLUDE_KERNEL_HARDENING
11y,30sep04,elg  Add WDB_GOPHER_TAPE_LEN and WDB_GOPHER_TAPE_NB to configure
                 gopher tape sizes.
11x,29sep04,pcs  Add USER_HASH_TABLE_ADDRESS & USER_HASH_TABLE_SIZE macros for
                 PPC60X
11w,23sep04,jn   Initialize configuration parameter for system symbol table
11v,27sep04,dbt  Added several WDB parameters that used to be hardcode values
11u,17sep04,jmp  added PASSFS_CACHE parameter. added INCLUDE_PASSFS to
                 excluded components.
11t,09sep04,zl   fixing comments about VM_PAGE_SIZE (SPR #20950)
11s,03sep04,ans  Added INCLUDE_JOB_TASK
11r,02sep04,ans  Added Job facility parameters
11q,26aug04,md   added EDR_RECORD_SIZE define
11p,27aug04,gls  added SD_HOOK_TBL_SIZE
11m,08sep04,jyo  Fixing SPR 100119: Removing references to RT11.
13p,23sep04,ann  merge from networking: vxWorks 5x PPP equivalent changes, Appl
                 Log Util changes, restored DNS resolver params, changed 
                 INCLUDE_NET_SYSCTL_API to INCLUDE_NET_SYSCTL.  
13o,19sep04,sru  Change MIPS definitions of SM_ANCHOR_ADRS, BOOT_LINE_ADRS,
                 and EXC_MSG_ADRS to unconditionally place them in KSEG0.
13n,27aug04,pes  Back out AD-MMU changes until after beta.
13m,19aug04,pes  Enclose MIPS64 definition of TASK_USER_EXC_STACK_SIZE in
                 parentheses to ensure correct evaluation.
13l,19aug04,pes  Increase TASK_USER_EXC_STACK_SIZE for MIPS64 (only)
13k,17aug04,sru  Add EXC_PAGE_PHYS_ADRS for MIPS.
13j,24aug04,dbt  Added VM_PAGE_SIZE definition for VxSim.
13i,18aug04,bwa  increased the default number of DSI sockets and data buffers.
13h,29jul04,md   add default size of PM_RESERVED_MEM
13g,28jul04,md   removed INCLUDE_EDR_SYMBOLIC and INCLUDE_EDR_POLICIES
13f,17aug04,bwa  removed some DSI_xxx definitions that should've been private.
13e,31jul04,tam  added SW_MMU_ENABLED definition
13d,21jul04,elg  Increase WDB stack size (SPR #99322).
13c,14jun04,vvv  merged from base6_itn5_networking-dev (SPR #91581)
13b,25may04,elg  Add ability to start WDB before kernel.
13a,11jun04,dcc  added INCLUDE_POSIX_CLOCKS.
12z,09jun04,zl   removed references to vxVMI (INCLUDE_MMU_FULL).
12y,02jun04,ann  merged from /main/vdt/base6_itn5-int/9
12x,26may04,syy  Fix DHCPv6 define names
12w,25may04,ann  merged from /main/vdt/base6_itn5_kernel-dev.ot/4 to get the
                 code review changes.
12v,18may04,rp   moved define for INCLUDE_FTPD_SECURITY to usrDepend.c
12u,14may04,syy  Add more DHCPv6 related definitions
12t,07may04,vvv  fixed build warnings/errors (SPR #91088)
12s,06may04,vvv  fixed values of MBLKS and CLBLKS (SPR #89778)
12r,30apr04,vvv  added ARP_GRATUITUOUS_CFG (SPR #92689)
12q,28apr04,vvv  fixed MIB2_SYSTEM configuration (SPR #94509)
12p,03may04,kk   added INCLUDE_SHL_SHOW
12o,21apr04,cjj	 Added WIND_JOBS_MAX.  Cleaned up history version numbers 
		 back to 08y.
12n,02apr04,dms  Added INCLUDE_SHL to the excluded section.
12m,04mar04,ann  merged from version /main/vdt/base6_itn4-int/4.
12l,25feb04,dbt  Added declaration of INCLUDE_WDB_RTP and INCLUDE_WDB_RTP_BP.
12k,24feb04,bwa  added AF_LOCAL show routines to list of excluded components
12j,20feb04,vvv  modified for socket/sysctl separation from stack
12i,18feb04,asr  added INCLUDE_FTPD_ANONYMOUS_LOGIN and its parameters.
12h,10feb04,bpn  Added VXE_PATH configuration variable for the shell.
12g,27jan04,elg  Add default RTP initial task priority.
12f,26jan04,zl   added memEdr configuration
12e,21jan04,zhr  added INCLUDE_TELNET_CLIENT
12d,13jan04,pad  Set RTP_HOOK_TBL_SIZE's default value to 8 (SPR #92655).
12c,09jan04,p_r  moved SAL and SNS to usrDepend.c
12b,06jan04,rp   fixed SPR 92684
12a,08dec03,ann  added the VX_UNBREAKABLE option for tNetTask
11z,08dec03,p_r  changed SNS_PATHNAME and added COMP dependencies
11y,08dec03,m_s  added INCLUDE_SC_POSIX
11x,07dec03,aim  added INCLUDE_EDR_KH
11w,06dec03,niq  added RIP_IF_INIT and RIP_ROUTING_SOCKET_BUF_SIZE 
11v,22apr04,vvv  fixed warnings
11u,22apr04,rae  add INCLUDE_OLDROUTE and INCLUDE_IFINDEX
11t,22apr04,vvv  removed encapLib components (SPR #96452)
11s,21apr04,spm  added INCLUDE_SM_NET for shared memory setup
11r,21apr04,niq  Merge from base6_itn4_networking_perf-dev.al branch
11q,20apr04,xli  remove the unsupported DNS configuration parameters.   
11p,14apr04,syy  Added DHCP6 client definitions
11o,06apr04,jmp  increased usr task exception stack size for simulators.
11n,06apr04,yvp  Add INCLUDE_RTP_APPL into excluded facilities (SPR 95588).
11m,17mar04,bpn	 Modified SHELL_DEFAULT_CONFIG parameter (SPR#94900).
11l,26feb04,yvp	 Set SYSCALL_HOOK_TBL_SIZE default value to 8 (SPR #93621).
11k,06apr04,dlk  Added TCP_REASS_ENDPT_MAX_CFG.
11j,01apr04,dat  adding INCLUDE_MEMDRV, (93390) INCLUDE_PTYDRV TYLIB (6294)
11i,31mar04,dlk  Added TCP_REASS_GLOBAL_MAX_CFG (SPR #94914)
11h,31mar04,asr  enabled FTP security by default
11g,25mar04,asr  added timeout for ftpLib
11f,18mar04,dat  adding pool and set to defaults
11e,25feb04,dbt  Added declaration of INCLUDE_WDB_RTP and INCLUDE_WDB_RTP_BP.
11d,04mar04,ann  merged from version /main/vdt/base6_itn4-int/4.
11c,20feb04,vvv  modified for socket/sysctl separation from stack
11b,18feb04,asr  added INCLUDE_FTPD_ANONYMOUS_LOGIN and its parameters.
11a,21jan04,zhr  added INCLUDE_TELNET_CLIENT
                 SPR #75839/83022 fix (RIP merge from router stack)
10z,03dec03,zl   added RTP_HEAP_INIT_SIZE definition.
10y,02dec03,vvv  added INCLUDE_SC_SOCKLIB
10x,26nov03,rp   ppplite changes
10w,26nov03,p_r  added SAL components
10v,25nov03,ann  added the NFS components
10u,21nov03,bwa  added INCLUDE_UN and INCLUDE_UN_COMP to the list of excluded
                 network components
10t,20nov03,ppp  moved INCLUDE_INETLIB into the included section for
                 networking
10s,20nov03,vvv  added INCLUDE_SC_SYSCTL
10r,18nov03,bpn  Added SHELL_DEFAULT_CONFIG, SHELL_FIRST_CONFIG and 
                 SHELL_REMOTE_CONFIG parameters.
10q,17nov03,rhe  Added netBuf Pool files & intit routine
10p,17nov03,yvp  Added RTP_HOOK_TBL_SIZE & SYSCALL_HOOK_TBL_SIZE.
10o,04nov03,kk   added INCLUDE_SHARED_DATA
10n,31oct03,dat  adding posix dirlib component, sprs 22056/74198
10m,24oct03,tam  added KERNEL_HEAP_SIZE default definition
10l,21oct03,dat  added IO system call component
10k,26sep03,dbs  add ED&R policies
10j,22sep03,bpn  Added shell components INCLUDE_SHELL_VI_MODE and
                 INCLUDE_SHELL_INTERP_C. Set default line length to 256. Added
		 SHELL_CONFIG parameter and default value.
10i,19sep03,pad  Renamed TASK_OVERFLOW_SIZE and TASK_UNDERFLOW_SIZE into
                 TASK_STACK_OVERFLOW_SIZE and TASK_STACK_UNDERFLOW_SIZE.
10h,16sep03,vvv  added new components; cleaned up parameter definitions
10g,15sep03,nee  Merging SNTP and BOOTP changes from Ottawa branch
10f,15sep03,elg  BSP with serial connection as default WDB communication
                 interface do not build: WDB_TTY_ECHO missing.
10e,02sep03,kk   added TASK_USER_EXC_STACK_SIZE, TASK_OVERFLOW_SIZE, 
		 TASK_UNDERFLOW_SIZE
10d,03sep03,elg  Add WDB_END_DEVICE_NAME and WDB_END_DEVICE_UNIT default
                 definitions.
10c,26aug03,dbs  added ED&R features
10b,11aug03,niq  Merging from Accordion label ACCORDION_BASE6_MERGE_BASELINE
10a,22jul03,pad  Moved INCLUDE_RTP to the excluded component section since
                 we don't want to force the RTP support to be included always.
09z,09jul03,job  Added RTP support
09y,09jul03,md   added ROMFS support
09x,01jul03,nrj  added RTP support
09w,20jun03,ann  added comment against RIPNG, removed INCLUDE_ZLIB
09v,20jun03,syy  Increased the default number for NUM_SYS_1024 (SPR #88948)
09u,17jun03,vvv  added INCLUDE_NETMASK_GET (SPR #88786)
09t,16may03,vvv  added configuration parameter MIB2IF_LONG_COUNTERS
09s,16may03,ppp  changed SIZ_SYS_1024 from 1140 to 1144
09r,15may03,vvv  modified to exclude IPv6 components by default
09q,25apr03,vvv  unconditionally define network stack parameters; removed
		 INCLUDE_PPP; moved INCLUDE_MIB2_ALL and INCLUDE_MIB2_IPV6ALL
		 dependency completions to usrNetwork.c
09p,23apr03,vvv  updated network component list (SPR #83002); added FTP and
                 RSH as default components (SPR #87536)
09o,23apr03,vvv  updated from T2.2.1, ver08v (SPR #74633)
09n,22apr03,dbt  Added SIMLINUX support.
09m,17apr03,ann  added INCLUDE_FTP and INCLUDE_REMLIB
09l,10apr03,vvv  added FTP6 parameters (SPR #87544)
09k,10apr03,ann  added the Show routine components
09j,10apr03,ann  added TCPv6 and UDPv6 as default
09i,09apr03,to   added INCLUDE_OBJ_LIB to default configuration,
                 defined OBJ_LIBRARY_OPTIONS.
09h,09apr03,elg  Add WDB_COMM_PORT: it defines the default WDB UDP port.
09g,09apr03,bpn  Added SHELL_COMPATIBLE default value.
09f,03apr03,vvv  added new NETSTAT components (SPR #87352)
09e,27mar03,pai  merged SPR 74633 modifications into CP1
09d,26mar03,vvv  added configuration parameters for bootrom memory pools
		 (SPR #87113)
09c,20feb03,jmp  cleanup simulator code.
09b,20feb03,ann  accomodated the new network components due to split
09a,17feb03,bpn  Increased shell stack size to 0x10000, added new default
08z,10feb03,vvv  modified to exclude IPv6 components by default
08y,06feb03,vvv  added DHCPC_DISCOVER_RETRIES (SPR #83246)
08x,18oct02,hsh  remove MAXHOSTNAMELEN definition
08w,01oct02,ham  added MIB2_IP, MIB2_SYSTEM, MIB2_ALL and MIB2_IPV6ALL.
08v,01oct02,ham  changed the default parameter of ARP_MAX_ENTRIES_CFG(SPR#82324).
		 removed unnecessary components from ifdef NETWORK(SPR#82082).
08u,23aug02,kal  added RTSOL, RTADV, RIP and RIPNG
08t,14aug02,ppp  merged usrTor22Init.h into this file for the IPv6 components and 
		 parameters (SPR #80639)
08s,05jul02,hdn  added I80X86 CPU_FAMILY for 4KB VM_PAGE_SIZE
08r,22may02,sbs  Adding default system clock rate macro (SPR #1918)
08q,16may02,vvv  restored SLIP support
08p,30apr02,elr  Corrected default definition of LOGIN_PASSWORD
08o,27mar02,j_s  define WDB_COMM_TYPE to WDB_COMM_END (SPR #73338)
08n,13mar02,sn   include all available intrinsics by default
08m,13mar02,vvv  removed references to SLIP (SPR #72992)
08l,11mar02,sn   Wind Foundation classes no longer supported (SPR #73967)
08k,07mar02,vvv  fixed MIB startup sequence (SPR #72989)
08j,06feb02,sn   include intrinsics by default
08i,21jan02,sn   added INCLUDE_CTORS_DTORS to the default configuration
08h,07jan02,rbl  remove basic C++ again - diab does not have fine enough
                 granularity in its C++ components, so too much is being
                 pulled in. 
08g,10dec01,jlb  Change UGL to WindML
08f,10dec01,kab  SPR 71410 - stack overflow
08e,10dec01,sn   Turned on basic C++ by default; removed obsolete 
                 INCLUDE_CPLUS_TOOLS
08d,03dec01,g_h  Adding WDB_COMM_VTMD macro.
08c,13aug01,pch  add PPC440
08b,20nov01,bwa  Added INCLUDE_VXEVENTS to default configuration.
08a,08nov01,jn   switch ARM OMF from COFF to ELF
07z,31oct01,gls  merged in pthread code from AE
07y,06nov01,vvv  made NFS max. path length configurable (SPR #63551)
07x,01nov01,vvv  changed BOOTP_MAX_REQUESTS to 3
07w,29oct01,sn   Temporarily remove C++ from default configuration
07v,10oct01,pad  Switched i86 architecture to ELF OMF.
07u,09oct01,rae  merge from truestack ver 07s, base 07q
07t,08oct01,dee  Merge from ColdFire T2.1.0 release
07s,04oct01,jws  change SM_OBJ_MAX_TRIES to 5000 (SPR68418)
07s,03oct01,jkf  added dosFs2 specific defines to FALSE section.
07r,18sep01,pcm  added INCLUDE_EXC_SHOW and INCLUDE_LSTLIB
07q,07sep01,zl   excluded HW floating point support for SH7700.
07q,27jul01,to   define INCLUDE_SW_FP for ARM
07p,14oct99,jk   added INCLUDE_SOUND to excluded facilities.
07p,10nov00,s_m  PPC405 support
07q,25oct00,rec  increase stack save size for ARM
07p,21oct99,jgn  stop shared memory network & proxy ARP being defaults
07r,13apr01,mem  Disable proxy arp client by default.
07q,27oct00,spm  fixed DHCP startup update and mod history entry
07p,23oct00,niq  updated DHCP startup for latest version (from tor3_x branch)
07p,02mar00,zl   merged SH support into T2
07o,19mar99,dat  removed old driver support SPR 25877
07n,10mar99,drm  changing WINDMP to VXFUSION to reflect new product name.
07m,23feb99,sn   corrected comment for INCLUDE_CPLUS_STL
                 removed INCLUDE_CPLUS_BOOCH and INCLUDE_CPLUS_HEAP since
                 they're no-ops.
07l,04feb99,jmp  added INCLUDE_TSFS_BOOT & INCLUDE_TSFS_BOOT_VIO_CONSOLE to
                 the excluded facilities list (SPR# 24466).
07k,02feb99,tm   added PCI configuration type constants (PCI_CFG_*)
07j,28jan99,dat  removed INCLUDE_JAVA_RTX, according to simon
07i,27jan99,jco  merged from graphics2_0 branch
07h,19nov98,sn   added entries for C++ macros that are excluded by default
07h,01dec98,spm  added INCLUDE_ARP for automatic linking of API (SPR #7576)
07g,27oct98,ms   made INCLUDE_CPLUS the default
07f,25aug98,tpr  added PowerPC EC 603 support.
07o,01sep98,drm  added INCLUDE_WINDMP
07n,17apr98,nps  added INCLUDE_RBUFF.
07m,04mar97,jmb  must have HW_FP for HPSIM.
07l.28jan98,cth  removed WV_DEFAULT_ definitions
07k.18dec97,cth  reworked defines for WV 2.0, removed INCLUDE_INSTRUMENTATION
07j.20aug97,cth  added INCLUDE_WDB_TSFS option
07i,09mar98,sjw  added Java macros for conflict resolution
07h,26jun97,ics  added INCLUDE_HTTP (for Wind Web Server)
07g,02jun97,tcy  added support for WILLOWS RT
07o,04may98,cym  added SIMNT.
07n,29apr98,dbt  removed RDB referencies (no longer supported).
07m,19mar98,dbt  added INCLUDE_WDB_START_NOTIFY and INCLUDE_WDB_USER_EVENT.
07l,18mar98,jmb  HPSIM merge:  must have hardware FP
07k,11mar98,pdn  added WDB_COMM_PIPE (for VxSims)
07i,24nov97,jpd  changed VM_PAGE_SIZE for ARM family.
07f,06oct97,spm  added BSD43_COMPATIBLE define (optional binary compatibility).
07e,26aug97,spm  added defines for DHCP port selection
07d,15jul97,spm  added support for SNTP client and server
07c,03jul97,vin  added INCLUDE_ROUTE_SOCK
07b,03jul97,sgv  added INCLUDE_FTPD_SECURITY to fix spr #8602
07a,28apr97,spm  added DHCP_MAX_HOPS from dhcprLib.h
06w,02feb98,dbt  added INCLUDE_WDB_EVENTPOINTS to support WDB eventpoints.
06v,10dec97,spm  added INCLUDE_FTPD_SECURITY to support configurable password
                 authentication for FTP server (SPR #8602)
06u,04feb96,ms   added WDB support for NetROM 500 series.
06z,17apr97,vin  added INCLUDE_IP_FILTER.
06y,10apr97,jag  added support for DNS resolver.
06x,07arp97,vin  added INCLUDE_MCAST_ROUTING
06w,07apr97,spm  added support for DHCP server and relay agent
06v,07apr97,gnn  removed bogus RIP defaults
06u,14feb97,rjc  added OSPF_INCLUDE dfine for ospf routing
06t,29jan97,spm  added DHCPC_MIN_LEASE define and changed DHCPC_DEFAULT_LEASE.
06s,11jan97,vin  reorganized network related includes.
06r,04dec96,gnn  added RIP and DHCP defines.
06q,28aug96,gnn	 added WDB_COMM_END define.
06u,28nov96,cdp  added ARM support.
06t,03dec96,dbt  removed #ifdef INCLUDE_ANSI_ALL and #ifdef INCLUDE_MIB2_ALL
		 (moved in usrDepend.c).
06s,15nov96,jag  changed string associated with MIB2_SYS_DESCR (SPR# 7476)
06r,04oct96,elp  added INCLUDE_SYM_TBL_SYNC (SPR# 6775).
06q,09aug96,dbt  Moved the ifdef INCLUDE_POSIX_ALL to usrDepend.c (SPR #5524).
06p,29jul96,ms   reworked the fix for SPR #5603.
06o,24jul96,ism  hand-merged SIMSOLARIS.
06n,19jul96,dbt  Moved the ifdef INCLUDE_CONFIGURATION_5_2
		 to usrDepend.c (SPR #5603).
06m,11jul96,jmb  Eliminated alternative WDB macros names.  SPR #5707
06l,21jun96,jmb  long modhist -- deleted entries prior to 1994.  SPR #6528
06k,20jun96,tpr  added PowerPC 860 support.
06j,18jun96,dbt  removed line INCLUDE_ADA (spr 6645)
06i,10jun96,tam  changed STACK_RESIDENT to RAM_DST_ADRS for PPC
06j,16oct96.rjc  deleted snmp memory partition defines
06j,12aug96.rjc  changes for snmpdMemoryAlloc changes
06i,22apr96,kkk	 fixed typo on CPU_FAMILY for PPC (SPR# 6385)
06h,04apr96,rjc  Modifications for snmp v1 agent for 5.3
06g,07mar96,dat  WDB comm config can be specified on make command line
06f,05mar96,tpr  Added RESERVED for PowerPC.
06e,04dec95,vin  increased EVT_STACK_SIZE to 7000.
06d,06sep95,jag  deleted unnecessary STREAMS defines.
06c,01aug95,dzb  renamed STREAMS macros.  Added STREAMS_PROTO_INIT_RTN macro.
06b,28jul95,jag  Changed INCLUDE_STREAMS_LOG to INCLUDE_STREAMS_STRACE and
		 INCLUDE_STREAMS_STERR. Changed STREAMS_LOG_CONFIG_DIR to
		 STREAMS_STRERR_OUTPUT_DIR and STREAMS_STRACE_OUTPUT_DIR
06a,26jul95,dzb  adjusted default socket macro for STREAMS.
05z,25jul95,dzb  added INCLUDE_BSD_SOCKET and DEFAULT_XXX_SOCKET.
05y,18jul95,dzb  added PPP_CONNECT_DELAY.
05x,11jul95,dzb  modified INCLUDE_PPP inclusion.
05w,26jun95,dzb  removed PPP_OPT_USEHOSTNAME option.
05v,22jun95,sgv  Added STREAMS support
05u,21jun95,dzb  Added INCLUDE_PPP_CRYPT for unbundled crypt() support.
05t,20jun95,dzb  added PPP support.
06e,23sep95,tpr  changed VM_PAGE_SIZE for PPC family.
06d,19jun95,caf  additional PPC support.
06e,11oct95,ms   made 5.2 configuration easier to recreate (SPR #5134).
06d,10oct95,dat	 backward compatible BSP_VERSION and BSP_REV
06c,28sep95,dat	 new #define INCLUDE_WDB_ANNOUNCE
06b,21sep95,ms	 switched some WDB macros between here and usrWdb.c
06a,27jun95,ms	 renamed WDB_COMM_XXX macros
05z,21jun95,ms	 added INCLUDE_WDB_TTY_TEST and INCLUDE_WDB_EXIT_NOTIFY
05y,21jun95,tpr  added #define INCLUDE_WDB_MEM.
05x,13jun95,srh  Updated C++ support triggers.
05w,07jun95,ms	 WDB_STACK_SIZE is now CPU dependant.
05v,07jun95,p_m  added INCLUDE_FORMATTED_IO. suppressed spy from default
                 configuration.
05u,01jun95,ms	 all WDB macros start with WDB_ (usrWdb.c modhist has details).
05t,22may95,ms   added WDB agent support.
          + p_m  suppressed shell, symbol table and debug support from default
		 configuration.
05s,29mar95,kdl  added INCLUDE_GCC_FP.
05r,28mar95,kkk  added scalability MACROS, changed edata and end to arrays
		 (SPR #3917), added misc constants SM_PKTS_SIZE and
		 SM_CPUS_MAX (SPR #4130), added misc constant CONSOLE_BAUD_RATE
05q,24mar95,tpr  added #define USER_B_CACHE_ENABLE (SPR #4168).
05p,14mar95,caf  restored mips resident rom support (SPR #3856).
05o,18jan95,tmk  Added MC68060 case for HW_FP
06c,25may95,yao  changed to use _end for FREE_MEM_ADRS for PowerPC.
06b,12jan95,caf  added PPC floating point support.
06a,09jan95,yao  added PPC support.
05n,10dec94,kdl  Moved INCLUDE_POSIX_ALL to unincluded (SPR 3822).
05m,10dec94,caf  undid mod 05a, use _sdata for resident roms (SPR #3856).
05l,09dec94,jag  Added INCLUDE_MIB2_AT.
05k,17nov94,kdl  Added INCLUDE_NFS_SERVER (excluded); removed TRON references.
05j,13nov94,dzb  Moved INCLUDE_PING to excluded.
05i,11nov94,dzb  Added ZBUF_SOCK, TCP_DEBUG, and PING defines.
05h,11nov94,jag  cleanup of SNMP and MIB defines.
05g,04nov94,kdl	 initial merge cleanup.
05b,16jun94,caf  defined INCLUDE_ELF for MIPS, updated copyright notice.
04x,20jul94,ms   changed INCLUDE_HPPA_OUT to INCLUDE_SOM_COFF
05d,11apr94,jag  Removed conditions for definition of  NFS_GROUP and NFS_USER
		 ID for SNMP demo support.
05c,18feb94,elh  (SNMP VERSION) moved INCLUDE_POSIX_ALL to false section.
		 Added support for the mib2 library.
05f,25may94,kdl	 (POSIX VERSION) removed erroneous NFS def's.
05e,23mar94,smb	 (POSIX VERSION) removed PASSIVE_MODE
05d,15mar94,smb	 renamed tEvtTask parameters.
05c,15feb94,smb	 added define of WV_MODE
05b,12jan94,kdl	 (POSIX VERSION) turned off instrumentation, added
		 INCLUDE_POSIX_ALL; added INCLUDE_POSIX_SIGNALS; changed
		 INCLUDE_POSIX_MEM_MAN to INCLUDE_POSIX_MEM; added
		 NUM_SIGNAL_QUEUES.
*/

/*
DESCRIPTION
This header contains the parameters that define the default
configuration for VxWorks.
*/

#ifndef	INCconfigAllh
#define	INCconfigAllh

#include "smLib.h"
#include "vme.h"
#include "iv.h"

/******************************************************************************/
/*                                                                            */
/*                      INCLUDED SOFTWARE FACILITIES                          */
/*                                                                            */
/******************************************************************************/
#if VX_VERSION == 62
#define INCLUDE_BASE_KERNEL     /* base kernel: support for obj ownership */
#define INCLUDE_CTORS_DTORS
#define INCLUDE_CACHE_SUPPORT	/* include cache support package */
#define INCLUDE_CONSTANT_RDY_Q	/* constant insert time ready queue */
#if 0
#define INCLUDE_EDR_STUB	/* ED&R error-injection stub */
#endif
#define INCLUDE_ENV_VARS	/* unix compatable environment variables */
#define INCLUDE_EXC_HANDLING	/* include basic exception handling */
#define INCLUDE_EXC_SHOW	/* include exception show routines */
#define INCLUDE_EXC_TASK	/* miscelaneous support task */
#define INCLUDE_JOB_TASK	/* task level job facility task */
#define INCLUDE_FLOATING_POINT	/* floating point I/O */
#define INCLUDE_FORMATTED_OUT_BASIC /* Basic formatted output routines */ 
#define INCLUDE_FORMATTED_IO	/* formatted I/O */
#define INCLUDE_GCC_FP		/* gcc floating point support libraries */
#define INCLUDE_HOOKS           /* allow adding/removing of hook functions */
#define INCLUDE_IO_SYSTEM	/* include I/O system */
#define INCLUDE_MEM_MGR_FULL	/* full featured memory manager */
#define INCLUDE_OBJ_CLASS_MGMT  /* include object and class management */
#define INCLUDE_OBJ_LIB		/* include object management */
#define INCLUDE_MSG_Q		/* include message queues */
#define INCLUDE_MSG_Q_CREATE_DELETE  /* message queue creation & deletion */
#define INCLUDE_NETWORK         /* network subsystem code */
#define INCLUDE_PIPES		/* pipe driver */
#define INCLUDE_POOL		/* pool allocator */
#define INCLUDE_REBOOT_HOOKS    /* support for hook callouts before reboots */
#define INCLUDE_SET		/* general set object lists */
#define INCLUDE_SELECT		/* select() facility */
#define INCLUDE_VX_NATIVE_SCHEDULER /* VxWorks native scheduler */
#define INCLUDE_SEM_LIB         /* generic support for all semaphore types */
#define INCLUDE_SEM_DELETE      /* support for semaphore deletion */
#define INCLUDE_SEM_BINARY	/* include binary semaphores */
#define INCLUDE_SEM_MUTEX	/* include mutex semaphores */
#define INCLUDE_SEM_COUNTING	/* include counting semaphores */
#define INCLUDE_SEM_BINARY_CREATE    /* binary semaphore creation */
#define INCLUDE_SEM_COUNTING_CREATE  /* counting semaphore creation */
#define INCLUDE_SEM_MUTEX_CREATE     /* mutex semaphore creation */
#define INCLUDE_SIGNALS		/* software signal library */
#define INCLUDE_STDIO		/* standard I/O */
#define INCLUDE_TASK_CREATE_DELETE /* dynamic creation & deletion of tasks */
#define INCLUDE_TASK_HOOKS	/* include kernel callouts */
#define INCLUDE_TASK_RESTART    /* Ability to restart & reset tasks */
#define INCLUDE_TASK_VARS	/* task variable package */
#define INCLUDE_TIMEX		/* timexLib for exec timing */
#define INCLUDE_TYLIB		/* serial driver support lib */
#define INCLUDE_TTY_DEV		/* attach serial drivers */
#define INCLUDE_WATCHDOGS	/* include watchdogs */
#define INCLUDE_WATCHDOGS_CREATE_DELETE   /* watchdog creation & deletion */
#define INCLUDE_WDB		/* WDB debug agent */
#define INCLUDE_ANSI_ALL        /* includes complete ANSI C library functions */
#define INCLUDE_ANSI_ASSERT	/* ANSI-C assert library functionality */
#define INCLUDE_ANSI_CTYPE	/* ANSI-C ctype library functionality */
#define INCLUDE_ANSI_LOCALE	/* ANSI-C locale library functionality */
#define INCLUDE_ANSI_MATH	/* ANSI-C math library functionality */
#define INCLUDE_ANSI_STDIO	/* ANSI-C stdio library functionality */
#define INCLUDE_ANSI_STDLIB	/* ANSI-C stdlib library functionality */
#define INCLUDE_ANSI_STRING	/* ANSI-C string library functionality */
#define INCLUDE_ANSI_TIME	/* ANSI-C time library functionality */
#if 0
#define	INCLUDE_EDR_SYSDBG_FLAG	/* include ED&R debug flag support */
#endif
#define INCLUDE_SHELL           /* interactive c-expression interpreter */
#define INCLUDE_SHELL_INTERP_C	/* C interpreter */
#define INCLUDE_SHELL_INTERP_CMD /* shell command interpreter */
#define INCLUDE_STANDALONE_SYM_TBL /* compiled-in symbol table */
#define INCLUDE_SYM_TBL         /* symbol table package */
#define INCLUDE_TASK_SHELL_CMD
#define INCLUDE_SYM_SHELL_CMD
#define INCLUDE_DEBUG_SHELL_CMD
#define INCLUDE_SHELL_VI_MODE
#define INCLUDE_DEBUG

#define INCLUDE_DOSFS           /* dosFs file system */
#define INCLUDE_DOSFS_MAIN      /* dosFsLib (2) */
#define INCLUDE_DOSFS_FAT       /* dosFs FAT12/16/32 FAT table handler */
#define INCLUDE_XBD             /* Extended block device */
#define INCLUDE_XBD_PART_LIB
#define INCLUDE_XBD_BLK_DEV
#define INCLUDE_FLASH
#define INCLUDE_FLASH_BOOT

#define INCLUDE_DOSFS_DIR_VFAT  /* Microsoft VFAT dirent handler */
#define INCLUDE_DOSFS_DIR_FIXED /* 8.3 & VxLongNames directory handler */
#define INCLUDE_DOSFS_FMT       /* dosFs2 file system formatting module */
#define INCLUDE_DOSFS_CHKDSK    /* file system integrity checking */
#define INCLUDE_DOSFS_CACHE     /* Dos FS caching layer */

#define DOSFS_DEFAULT_MAX_FILES          20
#define DOSFS_DEFAULT_CREATE_OPTIONS     0x04
#define DOSFS_DEFAULT_CACHE_SIZE	 0x8000 /* 32K */

#define INCLUDE_POSIX_ALL       /* include all available POSIX
				   functions */

#define INCLUDE_DEVICE_MANAGER  /* Device Manager */
#define INCLUDE_ERF             /* Event reporting framework */

/* for VxWorks native scheduler */

#ifdef INCLUDE_CONSTANT_RDY_Q
#define VX_NATIVE_SCHED_CONSTANT_RDY_Q  TRUE
#else
#define VX_NATIVE_SCHED_CONSTANT_RDY_Q  FALSE 
#endif

/* Toolchain specific included software facilities */
/* The following is for the benefit of the project facility */
#define INCLUDE_ALL_INTRINSICS

#ifdef INCLUDE_ALL_INTRINSICS
#ifdef _ARCH_SUPPORTS_GCC
#define INCLUDE_GNU_INTRINSICS
#endif
#ifdef _ARCH_SUPPORTS_DCC
#define INCLUDE_DIAB_INTRINSICS
#endif
#endif

/* CPU-SPECIFIC INCLUDED SOFTWARE FACILITIES */

/* include support for possibly existing floating point coprocessor */

#if	(CPU==MC68020 || CPU==MC68040 || CPU==MC68060 || CPU==CPU32)
#define INCLUDE_MC68881         /* MC68881/2 (68040) floating pt coprocessor */
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif	/* CPU==MC68020 || CPU==MC68040 || CPU==MC68060 || CPU==CPU32 */

#if	(CPU_FAMILY == SPARC) && defined(INCLUDE_FLOATING_POINT)
#define INCLUDE_SPARC_FPU       /* SPARC Floating-Point Unit */
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif	/* CPU_FAMILY == SPARC */

#if	(CPU_FAMILY==MIPS)
#define INCLUDE_R3010           /* R3010 float point co-processor */
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif	/* CPU_FAMILY==MIPS */

#if	((CPU_FAMILY==PPC) && (CPU!=PPC403) && (CPU!=PPC405) && (CPU!=PPC440) \
	&& (CPU!=PPCEC603) && (CPU!=PPC860))
#define INCLUDE_PPC_FPU         /* PowerPC floating point unit */
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif	/* (CPU_FAMILY==PPC) && (CPU!=PPC4xx) && (CPU!=PPCEC603) && (CPU!=PPC860) */

#if	(CPU==I960KB)
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif  /* I960KB */

#if     (CPU_FAMILY==I80X86)
#define INCLUDE_I80387          /* I80387 float point co-processor */
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* CPU_FAMILY==I80X86 */

#if	(CPU_FAMILY==SH)
#if 	(CPU==SH7750)
#define	INCLUDE_HW_FP
#else
#define	INCLUDE_SW_FP
#endif	/* CPU==SH7750 */
#endif	/* CPU_FAMILY==SH */

#if     (CPU_FAMILY==ARM)
#define INCLUDE_SW_FP
#endif	/* CPU_FAMILY==ARM */

/* define appropriate object module format for a given architecture */

#if	((CPU_FAMILY==MIPS) || (CPU_FAMILY==PPC) || (CPU_FAMILY==SH) || \
	 (CPU_FAMILY==SIMSPARCSOLARIS) || (CPU_FAMILY==COLDFIRE) || \
	 (CPU_FAMILY==I80X86) || (CPU_FAMILY==ARM) || (CPU_FAMILY==SIMNT) || \
	 (CPU==SIMLINUX))
#define INCLUDE_ELF             /* ELF object modules */
#else
#if	((CPU_FAMILY==I960) || (CPU_FAMILY==AM29XXX)) 
#define INCLUDE_COFF            /* COFF object modules */
#else
#define INCLUDE_AOUT            /* a.out object modules */
#endif
#endif


/* New CPU power management only available on Pentium */

#if (CPU_FAMILY == I80X86)

#define     INCLUDE_CPU_PWR_MGMT	/* Power mgmt framework */
#define     INCLUDE_CPU_LIGHT_PWR_MGR	/* Light manager included by default */

/* 
 * Next three lines will need to move to usrDepend.c.
 * Util power manager available on Pentium 4 only.
 */

#if ((CPU == PENTIUM4) && defined (INCLUDE_CPU_UTIL_PWR_MGR))
#undef      INCLUDE_CPU_LIGHT_PWR_MGR	/* Exclude light manager if util manager */
#endif					/* is included.  Only one mgr allowed */

/* Framework configuration.  Definitions should move to usrConfig.h */

#define     CPU_PWR_TASK_P_STATE      (cpuPwrP0State)
#define     CPU_PWR_ISR_P_STATE       (NULL)
#define     CPU_PWR_UTIL_PERIOD1      (300)
#define     CPU_PWR_UTIL_PERIOD2      (1000)
#define     CPU_PWR_HIGH_THRESHOLD    (100)
#define     CPU_PWR_LOW_THRESHOLD     (0)

/* 
 * Configuration for util power manager.  Definitions should move
 * to usrConfig.h
 */

#ifdef INCLUDE_CPU_UTIL_PWR_MGR
#define   CPU_PWR_UP_UTIL             (95)
#define   CPU_PWR_DOWN_UTIL           (95)
#endif

#endif /* CPU_FAMILY == I80X86 */

/******************************************************************************/
/*                                                                            */
/*                          EXCLUDED FACILITIES                               */
/*                                                                            */
/******************************************************************************/

#if 0

#define INCLUDE_CODETEST        /* CodeTEST target utilities library */
#define INCLUDE_CORE_DUMP	/* Kernel Core Dump */
#define INCLUDE_CORE_DUMP_COMPRESS /* Core Dump Compression */
#define INCLUDE_CORE_DUMP_SHOW	/* Core Dump Show routines */
#define INCLUDE_CORE_DUMP_MEM	/* Core Dump Memory storage device */
#define INCLUDE_CORE_DUMP_RAW_DEV/* Core Dump generic raw device storage */
				/* support */
#define INCLUDE_CORE_DUMP_MEM_FILTER /* Core Dump memory region filtering */
#define INCLUDE_CPLUS		/* include C++ support */
#define INCLUDE_CPLUS_IOSTREAMS	/* include basic iostreams classes */
#define INCLUDE_CPLUS_STL	/* include Standard Template Library core */
#define INCLUDE_CPLUS_STRING      /* include string class */
#define INCLUDE_CPLUS_STRING_IO   /* include i/o for string class */
#define INCLUDE_CPLUS_COMPLEX     /* include complex number class */
#define INCLUDE_CPLUS_COMPLEX_IO  /* include i/o for complex number class */
#define INCLUDE_CPLUS_IOSTREAMS_FULL  /* include all of iostreams */
#define INCLUDE_CONFIGURATION_5_2 /* pre-tornado tools */
#define INCLUDE_DEBUG           /* pre-tornado debugging */
#define INCLUDE_DEMO		/* include simple demo instead of shell */
#define INCLUDE_FS_EVENT_UTIL   /* File system event utility */
#define INCLUDE_FS_MONITOR      /* File system monitor */
#define INCLUDE_ERF             /* Event reporting framework */
#define INCLUDE_DEVICE_MANAGER  /* Device Manager */
#define INCLUDE_XBD             /* Extended block device */
#define INCLUDE_XBD_BLK_DEV     /* XBD-block device wrapper */
#define INCLUDE_XBD_RAMDRV      /* XBD Ram Drive */

#define INCLUDE_HRFS            /* Highly reliable file system */
#define INCLUDE_HRFS_READONLY   /* Read-only version of HRFS */
#define INCLUDE_HRFS_FORMAT     /* HRFS Formatter */
#define INCLUDE_HRFS_CHKDSK     /* HRFS Consistency Checker */
#define INCLUDE_RAWFS           /* rawFs file system */
#define INCLUDE_DOSFS           /* usrDosFsOld.c wrapper layer */
#define INCLUDE_DOSFS_MAIN      /* dosFsLib (2) */
#define INCLUDE_DOSFS_FAT       /* dosFs FAT12/16/32 FAT table handler */
#define INCLUDE_DOSFS_DIR_VFAT  /* Microsoft VFAT dirent handler */
#define INCLUDE_DOSFS_DIR_FIXED /* 8.3 & VxLongNames directory handler */
#define INCLUDE_DOSFS_FMT       /* dosFs2 file system formatting module */
#define INCLUDE_DOSFS_CHKDSK    /* file system integrity checking */
#define INCLUDE_DOSFS_CACHE     /* Dos FS caching layer */
#define INCLUDE_CBIO            /* CBIO API module */
#define INCLUDE_DISK_PART       /* disk partition handling code, fdisk... */
#define INCLUDE_DISK_UTIL       /* ls, cd, mkdir, xcopy, etc */
#define INCLUDE_TAR             /* tar utility */
#define INCLUDE_RAM_DISK        /* CBIO API ram disk driver */

#define INCLUDE_ISR_OBJECTS     /* Interrupt Service Routine Object Library */

#define INCLUDE_HTML		/* include HTML support */
#define INCLUDE_JAVA		/* Java virtual machine */
#define INCLUDE_KERNEL_HARDENING/* kernel hardening features: text segment    */
				/* write-protection, exception vector table   */
				/* write-protection, stack overflow &         */
				/* underflow detection, non-executable stacks */
#define INCLUDE_LSTLIB		/* include linked list library lstLib.c */
#define INCLUDE_SOUND           /* include support for sampled audio data */
#define INCLUDE_LOADER          /* object module loading */
#define INCLUDE_MEMDRV          /* memory disk driver */

/* INCLUDE_MMU_BASIC is defined by many bsp's in config.h */

#define INCLUDE_MMU_BASIC 	/* bundled mmu support */

#define INCLUDE_OBJ_OWNERSHIP   /* include object management ownership */

#define INCLUDE_POSIX_AIO       /* POSIX async I/O support */
#define INCLUDE_POSIX_AIO_SYSDRV/* POSIX async I/O system driver */
#define INCLUDE_POSIX_FTRUNC	/* POSIX ftruncate routine */
#define INCLUDE_POSIX_MEM	/* POSIX memory locking */
#define INCLUDE_POSIX_MQ        /* POSIX message queue support */
#define INCLUDE_POSIX_PTHREADS  /* POSIX pthreads support */
#define INCLUDE_POSIX_SCHED	/* POSIX scheduling */
#define INCLUDE_POSIX_SEM 	/* POSIX semaphores */
#define INCLUDE_POSIX_SIGNALS 	/* POSIX queued signals */
#define INCLUDE_POSIX_TIMERS 	/* POSIX timers */
#define INCLUDE_POSIX_CLOCKS	/* POSIX clocks */
#define INCLUDE_POSIX_DIRLIB 	/* POSIX directory utilities */
#define INCLUDE_SC_POSIX 	/* POSIX system calls */
#define INCLUDE_POSIX_PTHREAD_SCHEDULER /* POSIX scheduler for pthread in RTP */
#define INCLUDE_CUSTOM_SCHEDULER        /* user specified scheduler */
#define INLCUDE_SEM_INFO	
#define INLCUDE_SEM_SHOW	/* semaphore show component */
#define INCLUDE_PROTECT_TEXT	/* text segment write protection */
#define INCLUDE_PROTECT_VEC_TABLE /* vector table write protection  */
#define INCLUDE_PROTECT_TASK_STACK/* task stack protection (overflow & */
				  /* underflow detection)              */
#define INCLUDE_PTYDRV          /* pseudo terminal driver */
#define INCLUDE_RAMDRV          /* ram disk driver */
#define INCLUDE_RBUFF           /* ring of buffers library */
#define INCLUDE_RTP             /* Real Time Process (RTP) */
#define INCLUDE_RTP_APPL_USER           /* RTP Startup Facility - User Code */
#define INCLUDE_RTP_APPL_INIT_STRING    /* RTP Startup - RTP list string */
#define INCLUDE_RTP_APPL_INIT_BOOTLINE  /* RTP Startup - RTP list in bootline*/
#define INCLUDE_RTP_APPL_INIT_CMD_SHELL_SCRIPT /* RTP Startup Cmd Shell Script*/
#define INCLUDE_SC_IOS		/* Real Time Process, IO support */
#define INCLUDE_ROMFS		/* rom based file system */
#define INCLUDE_SECURITY	/* shell security for network access */
#define INCLUDE_SHARED_DATA	/* shared data regions */
#define INCLUDE_SHELL           /* interactive c-expression interpreter */
#define INCLUDE_SHELL_VI_MODE	/* vi editing mode for the shell */
#define INCLUDE_SHELL_EMACS_MODE /* emacs editing mode for the shell */
#define INCLUDE_SHELL_INTERP_C	/* C interpreter */
#define INCLUDE_SHELL_INTERP_CMD /* shell command interpreter */
#define INCLUDE_SHL		/* shared libraries */
#define INCLUDE_SHL_SHOW	/* shared libraries show component */
#define INCLUDE_SHOW_ROUTINES   /* show routines for system facilities*/
#define INCLUDE_SM_OBJ          /* shared memory objects (unbundled) */
#define INCLUDE_SPY             /* spyLib for task monitoring */
#define INCLUDE_STANDALONE_SYM_TBL /* compiled-in symbol table */
#define INCLUDE_STARTUP_SCRIPT  /* execute start-up script */
#define INCLUDE_STAT_SYM_TBL    /* create user-readable error status */
#define INCLUDE_SW_FP		/* software floating point emulation */
#define INCLUDE_SYM_TBL         /* symbol table package */
#define INCLUDE_TASK_STACK_NO_EXEC /* non-executable task stacks */
#define INCLUDE_WINDML		/* include WindML multimedia Library support */
#define INCLUDE_UNLOADER        /* object module unloading */
#define INCLUDE_VXEVENTS 	/* include VxWorks events */
#define INCLUDE_WILLOWS_RT      /* include Willows RT library */
#define INCLUDE_WDB_MDL_SYM_SYNC /* synchronize host and target symbol tables */
#define	INCLUDE_WDB_PROXY	/* WDB agent proxy */
#define INCLUDE_WDB_PROXY_UDP	/* UDP/IP WDB Agent Proxy backend */
#define INCLUDE_WDB_PROXY_TIPC	/* WDB Agent Proxy backend */
#define INCLUDE_WDB_RTP		/* WDB RTP support */
#define INCLUDE_WDB_RTP_BP	/* WDB RTP breakpoint support */
#define INCLUDE_WDB_RTP_CONTROL /* WDB RTP control support */
#define INCLUDE_WDB_TSFS	/* target-server file system */
#define INCLUDE_VXFUSION        /* WindMP (unbundled) */
#define INCLUDE_WINDVIEW	/* WindView target facilities */
#define INCLUDE_WINDVIEW_CLASS  /* WindView class instrumentation */
#define INCLUDE_WVUPLOAD_ALL	/* include all Windview upload-path types */
#define INCLUDE_WVUPLOAD_FILE	/* include file Windview upload-path */
#define INCLUDE_WVUPLOAD_SOCK	/* include socket Windview upload-path */
#define INCLUDE_WVUPLOAD_TSFSSOCK /* include tsfs sock Windview upload-path */
#define INCLUDE_TRIGGERING      /* triggering facility */
#define INCLUDE_TRIGGERING_SHOW /* triggering show routines */
#define INCLUDE_RBUFF           /* ring buffer for WindView */
#define INCLUDE_RBUFF_SHOW      /* ring buffer show routines */
#define INCLUDE_SEQ_TIMESTAMP   /* basic timestamp */
#define INCLUDE_SYS_TIMESTAMP   /* system-supplied timestamp */
#define INCLUDE_WVNETD          /* network instrumentation */

#define INCLUDE_ANSI_5_0	/* include only version 5.0 ANSI support */
#define INCLUDE_DELETE_5_0	/* define delete() function as in VxWorks 5.0 */

#define INCLUDE_TSFS_BOOT	/* Boot using Target Server File System */
#define INCLUDE_TSFS_BOOT_VIO_CONSOLE	/* use tgtsvr Console for TSFS_BOOT */

#define INCLUDE_EDR_PM		/* ED&R persistent memory */
#define INCLUDE_EDR_ERRLOG	/* ED&R error log */

#define INCLUDE_RTP_HOOKS	/* RTP create/delete hook support */
#define INCLUDE_SYSCALL_HOOKS	/* System call entry/exit/register hooks */

#define INCLUDE_MEM_EDR		/* memory manager ED&R */
#define INCLUDE_MEM_EDR_RTC	/* Run-time checker support for Diab */
#define INCLUDE_MEM_EDR_SHOW	/* memory manager ED&R show */
#define INCLUDE_MEM_EDR_RTP_SHOW  /* memory manager ED&R show for RTPs */

#define	INCLUDE_PASSFS		/* include VxSim pass-through filesystem */

#endif	/* FALSE */

/******************************************************************************/
/*                                                                            */
/*                  KERNEL SOFTWARE CONFIGURATION                             */
/*                                                                            */
/******************************************************************************/

#define USER_I_CACHE_MODE	CACHE_WRITETHROUGH  /* default mode */
#define USER_D_CACHE_MODE	CACHE_WRITETHROUGH  /* default mode */
#define USER_I_CACHE_ENABLE		    	    /* undef to leave disabled*/
#define USER_D_CACHE_ENABLE			    /* undef to leave disabled*/
#define USER_B_CACHE_ENABLE			    /* undef to leave disabled*/
#define USER_I_MMU_ENABLE			    /* undef to leave disabled*/
#define USER_D_MMU_ENABLE			    /* undef to leave disabled*/

#define SYM_TBL_HASH_SIZE_LOG2	8	/* 256 entry hash table symbol table */
#define STAT_TBL_HASH_SIZE_LOG2	6	/* 64 entry hash table for status */
#define MQ_HASH_SIZE		0	/* POSIX message queue hash table size
					 * 0 = default */
#define NUM_SIGNAL_QUEUES	16	/* POSIX queued signal count */

#define FREE_RAM_ADRS		(end)	/* start right after bss of VxWorks */

#define MAX_REBOOT_HOOKS	8	

#ifndef POSIX_PTHREAD_RR_TIMESLICE      /* timeslice for SCHED_RR pthread */
#define POSIX_PTHREAD_RR_TIMESLICE  10  /* default 10 ticks */
#endif /* POSIX_PTHREAD_RR_TIMESLICE */

/* object management parameters */

#define OBJ_LIBRARY_OPTIONS     0    

/* I/O system parameters */

#define SYS_CLK_RATE		60	/* default system clock rate */
#define NUM_DRIVERS		20	/* max 20 drivers in drvTable */
#define NUM_FILES		50	/* max 50 files open simultaneously */
#define NUM_DOSFS_FILES		20	/* max 20 dosFs files open */
#define NUM_RAWFS_FILES		5	/* max 5  rawFs files open */
#define MAX_LOG_MSGS		50      /* max 50 log msgs */
#define INIT_NBIO_MSGS		32	/* 32 * 128 bytes = 4096 bytes */

#define	NUM_TTY			2	/* number of tty channels */
#define	CONSOLE_TTY		0	/* console channel */

#define TYLIB_XOFF_PCNT		85	/* XOFF at 85% full */
#define TYLIB_XON_PCNT		50      /* XON at 50% full */
#define TYLIB_WRT_THRESHLD	20	/* wakeup at 20 charas in tx buff */

#define CONSOLE_BAUD_RATE	9600	/* console baud rate */

#ifdef	INCLUDE_SLIP
#define	SLIP_TTY		1	/* serial line IP channel */
#endif	/* INCLUDE_SLIP */

#define MAX_LIO_CALLS		0	/* max outstanding lio calls 0=default */
#define MAX_AIO_SYS_TASKS	0	/* max aio system tasks, 0 = default */
#define AIO_TASK_PRIORITY	0	/* aio system tasks prior, 0 = default */
#define AIO_TASK_STACK_SIZE	0	/* aio tasks stack size, 0 = default */

/* kernel and system task parameters by architecture */

#if	CPU_FAMILY==MC680X0
#define INT_LOCK_LEVEL  	0x7	/* 68k interrupt disable mask */
#define ROOT_STACK_SIZE		10000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x1000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		1000	/* size of ISR stack, in bytes */
#define TRAP_DEBUG		2	/* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif	/* CPU_FAMILY==MC680X0 */

#if	CPU_FAMILY==COLDFIRE
#define	INT_LOCK_LEVEL 		0x7	/* 68k interrupt disable mask */
#define ROOT_STACK_SIZE		10000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE		0x1000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		1000	/* size of ISR stack, in bytes */
#define TRAP_DEBUG		2	/* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS		((char *) LOCAL_MEM_LOCAL_ADRS)
#endif	/* CPU_FAMILY==COLDFIRE */

#if	CPU_FAMILY==SPARC
#define INT_LOCK_LEVEL  	15	/* SPARC interrupt disable level */
#define ROOT_STACK_SIZE		10000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		10000	/* size of ISR stack, in bytes */
#define VEC_BASE                (LOCAL_MEM_LOCAL_ADRS + 0x1000)
#define VEC_BASE_ADRS           ((char *) VEC_BASE)
#endif	/* CPU_FAMILY==SPARC */

#if CPU_FAMILY==SIMNT || CPU_FAMILY==SIMSPARCSOLARIS || CPU_FAMILY==SIMLINUX
#define INT_LOCK_LEVEL          0x1     /* interrupt disable mask */
#define ROOT_STACK_SIZE         20000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          50000   /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           0       /* dummy */
#endif  /* CPU_FAMILY == SIMxxx */

#if	CPU_FAMILY==I960
#define INT_LOCK_LEVEL  	0x1f	/* i960 interrupt disable mask */
#define ROOT_STACK_SIZE		20000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		1000	/* size of ISR stack, in bytes */
#define TRAP_DEBUG		0	/* n/a for the 80960 */
#define VEC_BASE_ADRS           NONE	/* base register not reconfigurable */
#endif	/* CPU_FAMILY==I960 */

#if	CPU_FAMILY==MIPS
#define INT_LOCK_LEVEL          0x1     /* R3K interrupt disable mask */
#define ROOT_STACK_SIZE         (20000) /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        (0x10000) /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	(0x2000)/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          (5000)  /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           ((char *) 0x0)  /* meaningless in R3k land */
#define VME_VECTORED            FALSE   /* use vectored VME interrupts */
#define TRAP_DEBUG              0       /* trap 0 - breakpoint trap */
#define EXC_PAGE_PHYS_ADRS      0x2000  /* 2nd page of RAM used by mapped OS */
#endif	/* CPU_FAMILY==MIPS */

#if	CPU_FAMILY==PPC
#define INT_LOCK_LEVEL          0x0     /* not used */
#define BOOT_CMD_STACK_SIZE	(11000) /* size of boot cmd stack, in bytes */
#define ROOT_STACK_SIZE         (24000) /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        (0x10000) /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          (5000)  /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           ((char *) 0x0)
#endif	/* CPU_FAMILY==PPC */

#if     CPU_FAMILY==I80X86
#define INT_LOCK_LEVEL          0x0     /* 80x86 interrupt disable mask */
#define ROOT_STACK_SIZE         10000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          1000    /* size of ISR stack, in bytes */
#define TRAP_DEBUG              0       /* not used */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif  /* CPU_FAMILY==I80X86 */

#if	CPU_FAMILY==AM29XXX
#define INT_LOCK_LEVEL  	0x0001	/* 29k all interrupts disable mask */
#define ROOT_STACK_SIZE		10000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		10000	/* size of ISR stack, in bytes */
#define TRAP_DEBUG		15	/* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif	/* CPU_FAMILY==AM29XXX */

#if	CPU_FAMILY==SH
#define	INT_LOCK_LEVEL		15
#define	ROOT_STACK_SIZE		10000
#define	SHELL_STACK_SIZE	0x10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x1000	/* size of WDB agents stack, in bytes */
#define	ISR_STACK_SIZE		1000
#define	VEC_BASE_ADRS		((char *) LOCAL_MEM_LOCAL_ADRS)
#endif	/* CPU_FAMILY==SH */

#if     CPU_FAMILY==ARM
#define INT_LOCK_LEVEL          0       /* interrupt disable mask - unused */
#define ROOT_STACK_SIZE         0x4000  /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define TRAP_DEBUG              0       /* not used */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
/*
 * NOTE: ISR_STACK_SIZE defined in config.h, not here - BSP interrupt
 * 	 structure dependent
 */
#endif  /* CPU_FAMILY==ARM */

#ifndef BOOT_CMD_STACK_SIZE
#define BOOT_CMD_STACK_SIZE	11000
#endif  /* BOOT_CMD_STACK_SIZE */

/* RTP initialization constants */

#define RTP_HOOK_TBL_SIZE     8
#define SYSCALL_HOOK_TBL_SIZE 8
#define RTP_SIGNAL_QUEUE_SIZE 32  	/* must be at least 32 for POSIX */

/* Shared Data initialization constants */

#define SD_HOOK_TBL_SIZE     8

/* WDB debug agent configuration */

#ifdef  INCLUDE_WDB

/* optional agent facilities */

#define INCLUDE_WDB_BANNER		/* print banner after agent starts */
#define INCLUDE_WDB_VIO			/* virtual I/O support */
#define	INCLUDE_WDB_TTY_TEST		/* test serial line communcation */
#define INCLUDE_WDB_START_NOTIFY	/* notify the host of task creation */
#define INCLUDE_WDB_USER_EVENT		/* user events handling */

/* core agent facilities - do not remove */

#define INCLUDE_WDB_CTXT		/* context control */
#define INCLUDE_WDB_FUNC_CALL		/* spawn function as separate task */
#define INCLUDE_WDB_DIRECT_CALL		/* call function in agents context */
#define INCLUDE_WDB_EVENTS		/* host async event notification */
#define INCLUDE_WDB_GOPHER		/* gopher info gathering */
#define INCLUDE_WDB_BP			/* breakpoint support */
#define INCLUDE_WDB_EXC_NOTIFY		/* notify host of exceptions */
#define INCLUDE_WDB_EXIT_NOTIFY		/* notify the host of task exit */
#define INCLUDE_WDB_REG			/* get/set hardware registers */
#define INCLUDE_WDB_EVENTPOINTS		/* eventpoints handling */
#define INCLUDE_WDB_MEM			/* optional memory services */

/* WDB agent initialization modes */

#define	WDB_POST_KERNEL_INIT	0	/* WDB starts after kernel */
#define	WDB_PRE_KERNEL_INIT	1	/* WDB starts before kernel */

#ifndef	WDB_INIT
#define	WDB_INIT	WDB_POST_KERNEL_INIT
#endif	/* WDB_INIT */

/* agent mode */

#define WDB_MODE        WDB_MODE_DUAL	/* WDB_MODE_[DUAL|TASK|EXTERN] */

/* agent communication paths */

#define WDB_COMM_NETWORK 	0	/* vxWorks network	- task mode */
#define WDB_COMM_SERIAL		1	/* raw serial		- bimodal   */
#define WDB_COMM_TYCODRV_5_2	2	/* older serial driver	- task mode */
#define WDB_COMM_ULIP		3	/* vxSim packet device	- bimodal   */
#define WDB_COMM_NETROM		4	/* netrom packet device	- bimodal   */
#define WDB_COMM_CUSTOM		5	/* custom packet device	- bimodal   */
#define WDB_COMM_END		6	/* END packet device 	- bimodal   */
#define WDB_COMM_PIPE           7       /* vxSim pipe device    - bimodal   */
#define WDB_COMM_VTMD           8       /* TMD packet device    - bimodal   */
#define WDB_COMM_TIPC           9       /* TIPC packet device   - task mode */

/* communication path configuration */

#ifndef WDB_COMM_TYPE
#define WDB_COMM_TYPE WDB_COMM_NETWORK /* default path is the network */
#endif

#ifndef	WDB_COMM_PORT
#define	WDB_COMM_PORT		0x4321	/* default WDB UDP port value */
#endif

#ifndef WDB_TTY_CHANNEL
#define WDB_TTY_CHANNEL		0	/* default Sio SERIAL channel */
#endif

#ifndef WDB_TTY_DEV_NAME
#define WDB_TTY_DEV_NAME    "/tyCo/0"	/* default TYCODRV_5_2 device name */
#endif

#ifndef WDB_TTY_BAUD
#define WDB_TTY_BAUD		9600 /* default baud rate */
#endif

#ifndef	WDB_TTY_ECHO
#define	WDB_TTY_ECHO		FALSE	/* default echoing debug flag */
#endif

#ifndef	WDB_END_DEVICE_NAME
#define	WDB_END_DEVICE_NAME	NULL
#endif	/* WDB_END_DEVICE_NAME */

#ifndef	WDB_END_DEVICE_UNIT
#define	WDB_END_DEVICE_UNIT	0
#endif	/* WDB_END_DEVICE_UNIT */

#ifndef WDB_END_DEVICE_ADDRESS
#define WDB_END_DEVICE_ADDRESS NULL
#endif /* WDB_END_DEVICE_ADDRESS */

#ifndef WDB_ULIP_DEV
#define WDB_ULIP_DEV    "/dev/ulip14"	/* default ULIP packet device */
#endif

#define	WDB_NETROM_TYPE		400	/* default is old 400 series */
#define	WDB_NETROM_WIDTH	1	/* width of a ROM word. This macro
					 * is not used for the 500 series. */
#define WDB_NETROM_INDEX	0	/* index into word of pod zero */
#define	WDB_NETROM_NUM_ACCESS	1	/* of pod zero per byte read */
#define	WDB_NETROM_ROMSIZE	ROM_SIZE /* size of a single ROM. If your
					 * board has N ROMs, this definition
					 * must be divided by N to be right */
#define WDB_NETROM_POLL_DELAY	2	/* # clock ticks to poll for input */

#define	WDB_TIPC_PORT_TYPE	70	/* WDB TIPC port type */
#define WDB_TIPC_PORT_INSTANCE	71	/* WDB TIPC port instance */

/* gopher configuration */

#define	WDB_GOPHER_TAPE_LEN	1400	/* maximal length of a gopher tape */
#define	WDB_GOPHER_TAPE_NB	10	/* maximal number of dynamically */
					/* allocated tapes */

/* miscelaneous agent constants */

#define WDB_MTU         	1500	/* max RPC message size */
#define WDB_POOL_SIZE 		((sysMemTop() - FREE_RAM_ADRS)/16)
					/* memory pool for host tools */
#define WDB_REGS_SIZE		512	/* temporary buffer size for coproc */
					/* registers access in system mode */
#define WDB_SPAWN_STACK_SIZE	0x5000	/* def. stack size for spawned task */
#define WDB_SPAWN_PRI		100	/* def. priority for spawned task */
#define WDB_SPAWN_OPTS		VX_FP_TASK /* def. options for spawned task */
#define	WDB_RTP_PRIORITY	200	/* def. priority for RTP initial task */
#define	WDB_RESTART_TIME	10	/* delay before error-restarting */
					/* agent */
#define	WDB_MAX_RESTARTS	5	/* max # agent restarts on error */
#define WDB_BP_MAX		50	/* max # of break points */
#define WDB_TASK_PRIORITY       3	/* priority of task agent */
#define	WDB_TASK_OPTIONS	VX_UNBREAKABLE | VX_FP_TASK
					/* WDB task options */



#endif  /* INCLUDE_WDB */

/* Wind River Agent Proxy configuration */

#define	WDB_PROXY_PORT			0x4444	/* TCP/IP port used by proxy */
#define WDB_PROXY_TASK_PRIORITY		55	/* Proxy Task priority */
#define WDB_PROXY_TASK_STACK_SIZE	10000	/* Proxy task stack size */
#define WDB_PROXY_TASK_OPTIONS		VX_SUPERVISOR_MODE | VX_UNBREAKABLE
						/* Proxy task options */

/* Shell default configuration */

#define SHELL_SECURE		FALSE
#define SHELL_START_AT_BOOT	TRUE
#define SHELL_COMPATIBLE	FALSE
#define SHELL_DEFAULT_CONFIG	\
"LINE_EDIT_MODE=,LINE_LENGTH=256,STRING_FREE=manual,INTERPRETER=,VXE_PATH=.;/romfs"
#define SHELL_FIRST_CONFIG	NULL
#define SHELL_REMOTE_CONFIG	NULL
#define SHELL_TASK_NAME_BASE	"tShell"
#define SHELL_TASK_PRIORITY	1
#define SHELL_TASK_OPTIONS	(VX_FP_TASK | VX_UNBREAKABLE | COPROCS_ALL | VX_PRIVATE_ENV)

/* defaults for tasks */

#ifndef	_WRS_ARCH_IS_SIMULATOR
#if (CPU_FAMILY==MIPS) && (_WRS_INT_REGISTER_SIZE==8)
#define TASK_USER_EXC_STACK_SIZE	(8192*2) /* usr task exc stack size */
#else
#define TASK_USER_EXC_STACK_SIZE	8192 /* usr task exc stack size */
#endif

#else
#define TASK_USER_EXC_STACK_SIZE	(8192*3)
#endif	/* _WRS_ARCH_IS_SIMULATOR */

/*
 * These define the default sizes, in bytes, of the overflow and underflow
 * guard regions for task stacks and the interrupt stack.  The values
 * defined here (or as overridden in config.h) are used only if the
 * corresponding protection capabilities are configured, and will be
 * rounded up to a multiple of VM_PAGE_SIZE.
 */

/*  user task exception stack overflow proection size */ 
#define TASK_USER_EXC_STACK_OVERFLOW_SIZE     4096 
/*user task execution stack overflow protection size      */
#define TASK_USER_EXEC_STACK_OVERFLOW_SIZE    4096 
/* user task execution stack  underflow protection size */
#define TASK_USER_EXEC_STACK_UNDERFLOW_SIZE   4096 
/* kernel task execution stack overflow protection size  */
#define TASK_KERNEL_EXEC_STACK_OVERFLOW_SIZE  4096 
/* kernel task execution stack underflow protection size  */ 
#define TASK_KERNEL_EXEC_STACK_UNDERFLOW_SIZE 4096 
#define INTERRUPT_STACK_OVERFLOW_SIZE	4096 /* stack overflow protection sz */
#define INTERRUPT_STACK_UNDERFLOW_SIZE	4096 /* stack underflow protection sz */

/* 
 * Default value for work queue size (in jobs).  MUST be a power of two
 * no greater than 0x10000.  Values below 64 may cause work queue panics.
 */
#define WIND_JOBS_MAX 64 

/* Job Facility configurable parameters */
#define MAX_ISR_JOBS 10
#define JOB_TASK_STACK_SIZE 8000

/* File system configurable parameters */

#define ERF_MAX_USR_CATEGORIES           16
#define ERF_MAX_USR_TYPES                16
#define DEVICE_MANAGER_MAX_NUM_DEVICES  255

#define DOSFS_DEFAULT_MAX_FILES          20
#define DOSFS_DEFAULT_CREATE_OPTIONS     0x04
#define DOSFS_DEFAULT_CACHE_SIZE	 0x8000 /* 32K */

#define HRFS_DEFAULT_MAX_BUFFERS          8
#define HRFS_DEFAULT_MAX_FILES           10

#define FD_DEV_NAME                      "/fd"
#define FD_0_TYPE                        0
#define FD_1_TYPE                        -1

/* default value of the kernel heap size: used only if the RTP support is in */

#define KERNEL_HEAP_SIZE 	(((sysMemTop() - FREE_RAM_ADRS) / 3) * 2)

/* RTP heap initial size: used only if the RTP support is in */

#define RTP_HEAP_INIT_SIZE	0x10000

/* memory ED&R configuration: used only if INCLUDE_MEM_EDR is defined */

#define MEDR_EXTENDED_ENABLE		FALSE	/* extended info (trace) */
#define MEDR_FREE_QUEUE_LEN		64	/* free queue length */
#define MEDR_FILL_FREE_ENABLE		FALSE	/* fill freed blocks */
#define MEDR_BLOCK_GUARD_ENABLE		FALSE	/* block guard zone */
#define MEDR_POOL_SIZE			0x100000 /* memory for database pool */

/* default persistent memory size for ED&R */

#ifndef	PM_RESERVED_MEM
#define PM_RESERVED_MEM			(6 * VM_PAGE_SIZE)
#endif
#ifndef	EDR_ERRLOG_SIZE
#define	EDR_ERRLOG_SIZE			(pmFreeSpace(EDR_PM_ARENA)/2)
#endif
#ifndef	EDR_RECORD_SIZE
#define	EDR_RECORD_SIZE			4096
#endif

/* core dump default configuration */

#define CORE_DUMP_REGION_SIZE		pmFreeSpace(pmDefaultArena)
						/* coredump storage mem size */
#define CORE_DUMP_COMPRESSION_LEVEL	9	/* coredump compression level */
#define CORE_DUMP_MEM_FILTER_MAX	10	/* Max # of mem region filter */
#define CORE_DUMP_MAX_HOOKS		10	/* Max # of core dump create */
						/* hooks */
#define FATAL_SYSTEM_CORE_DUMP_ENABLE	TRUE	/* Generate core dump for */
						/* fatal system exceptions */
#define KERNEL_APPL_CORE_DUMP_ENABLE	FALSE	/* Generate core dump for */
						/* kernel tasks exceptions */
#define CORE_DUMP_SKIP_TEXT_SECTION	TRUE	/* Remove VxWorks text */
						/* section from core dump */
#define CORE_DUMP_STACK_SIZE		8192	/* core dump stack size */


/******************************************************************************/
/*                                                                            */
/*                   "GENERIC" BOARD CONFIGURATION                            */
/*                                                                            */
/******************************************************************************/

/* device controller I/O addresses when included */

#define IO_ADRS_EI      ((char *) 0x000fff00)   /* 32A,32D i82596CA Ethernet */


/******************************************************************************/
/*                                                                            */
/*                   "MISCELLANEOUS" CONSTANTS                                */
/*                                                                            */
/******************************************************************************/

/* shared memory objects parameters (unbundled) */

#define SM_OBJ_MAX_TASK		40	/* max # of tasks using smObj */
#define SM_OBJ_MAX_SEM		60	/* max # of shared semaphores */
#define SM_OBJ_MAX_MSG_Q	10	/* max # of shared message queues */
#define SM_OBJ_MAX_MEM_PART	4	/* max # of shared memory partitions */
#define SM_OBJ_MAX_NAME		100	/* max # of shared objects names */
#define SM_OBJ_MAX_TRIES	5000	/* max # of tries to obtain lock */

/* shared memory network parameters  - defaults to values DEFAULT_PKTS_SIZE
 * and DEFAULT_CPUS_MAX in smPktLib.h respectively
 */

#define SM_PKTS_SIZE            0                /* shared memory packet size */
#define SM_CPUS_MAX             DEFAULT_CPUS_MAX /* SM max # of CPUs in sys   */

/* low memory layout */

#if     (CPU_FAMILY == I80X86)
#define GDT_BASE_OFFSET         0x1000
#define SM_ANCHOR_OFFSET        0x1100
#define BOOT_LINE_OFFSET        0x1200
#define EXC_MSG_OFFSET          0x1300
#elif   (CPU_FAMILY == PPC)
#define SM_ANCHOR_OFFSET        0x4100
#define BOOT_LINE_OFFSET        0x4200
#define EXC_MSG_OFFSET          0x4300
#elif	(CPU==SH7750 || CPU==SH7729 || CPU==SH7700)
#define SM_ANCHOR_OFFSET        0x1600	/* not used for SDRAM config. */
#define BOOT_LINE_OFFSET        0x1700
#define EXC_MSG_OFFSET          0x1800
#elif   (CPU_FAMILY == ARM)
#define SM_ANCHOR_OFFSET        0x1000
#define BOOT_LINE_OFFSET        0x1100
#define EXC_MSG_OFFSET          0x1200
#else
#define SM_ANCHOR_OFFSET        0x600
#define BOOT_LINE_OFFSET        0x700
#define EXC_MSG_OFFSET          0x800
#endif  /* (CPU_FAMILY == I80X86) */

/* The backplane driver onboard anchor at the following address */

#if (CPU_FAMILY==MIPS)
#define SM_ANCHOR_ADRS	((char *) \
                         KM_TO_K0((LOCAL_MEM_LOCAL_ADRS+SM_ANCHOR_OFFSET)))
#else /* CPU_FAMILY==MIPS */
#define SM_ANCHOR_ADRS	((char *) (LOCAL_MEM_LOCAL_ADRS+SM_ANCHOR_OFFSET))
#endif  /* CPU_FAMILY==MIPS */


/* The bootroms put the boot line at the following address */

#if (CPU_FAMILY==MIPS)
#define BOOT_LINE_ADRS	((char *) \
                         KM_TO_K0((LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET)))
#else /* CPU_FAMILY==MIPS */
#define BOOT_LINE_ADRS	((char *) (LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET))
#endif  /* CPU_FAMILY==MIPS */

#define	BOOT_LINE_SIZE	255	/* use 255 bytes for bootline */

/* The boot line is stored in non-volatile RAM at the following offset */

#define	NV_BOOT_OFFSET	0	/* store the boot line at start of NVRAM */


/* Messages from exceptions during exceptions go at the following address */

#if (CPU_FAMILY==MIPS)
#define EXC_MSG_ADRS	((char *) \
                         KM_TO_K0((LOCAL_MEM_LOCAL_ADRS+EXC_MSG_OFFSET)))
#else
#define EXC_MSG_ADRS	((char *) (LOCAL_MEM_LOCAL_ADRS+EXC_MSG_OFFSET))
#endif


/* Backplane H/W support */

#define	SM_TAS_TYPE	SM_TAS_HARD	/* hardware supports test-and-set */

/* VxSim pass-through filesystem constants */

#define PASSFS_CACHE	TRUE		/* Enable VxSim PassFs cache */

/* Resident ROMs constants */

#if     ((CPU_FAMILY==I960) || (CPU_FAMILY==ARM))
#define STACK_SAVE      512     	/* maximum stack used to preserve */
#else	/* (CPU_FAMILY==I960) || (CPU_FAMILY==ARM) */
#if	((CPU_FAMILY==SPARC) || (CPU_FAMILY==PPC))
#define	STACK_SAVE	0x1000
#else	/* (CPU_FAMILY==SPARC) || (CPU_FAMILY==PPC) */
#if	(CPU_FAMILY==SH)
#define	STACK_SAVE	0x80
#else	/* all other architecutes */
#define STACK_SAVE      0x40    	/* maximum stack used to preserve */
#endif	/* (CPU_FAMILY==SH) */
#endif	/* (CPU_FAMILY==SPARC) || (CPU_FAMILY==PPC) */
#endif	/* (CPU_FAMILY==I960)  || (CPU_FAMILY==ARM) */

#if     (CPU_FAMILY==SPARC)
#define RESERVED        0x2000		/* vector table base plus table size */
#else	/* 68000 or others */
#if	(CPU==MC68000)
#define RESERVED	0x400		/* avoid zeroing MC68302 vector table */
#else
#if	(CPU_FAMILY == PPC)
#define RESERVED	0x4400		/* avoid zeroing EXC_MSG */
#else	/* all other architectures */
#define RESERVED        0
#endif
#endif					/* mips cpp no elif */
#endif

#if     ((CPU_FAMILY == MIPS) || (CPU_FAMILY == PPC))
#define	STACK_RESIDENT	RAM_DST_ADRS
#else
#define	STACK_RESIDENT	_sdata
#endif

#if	(_STACK_DIR == _STACK_GROWS_DOWN)

#ifdef	ROM_RESIDENT
#define STACK_ADRS	STACK_RESIDENT
#else
#define STACK_ADRS	_romInit
#endif	/* ROM_RESIDENT */

#else	/* _STACK_DIR == _STACK_GROWS_UP */

#ifdef	ROM_RESIDENT
#define STACK_ADRS	(STACK_RESIDENT-STACK_SAVE)
#else
#define STACK_ADRS	(_romInit-STACK_SAVE)
#endif	/*  ROM_RESIDENT */

#endif	/* _STACK_DIR == _STACK_GROWS_UP */


/* Default Boot Parameters */

#define HOST_NAME_DEFAULT	"bootHost"	/* host name */
#define TARGET_NAME_DEFAULT	"vxTarget"	/* target name (tn) */
#define HOST_USER_DEFAULT	"target"	/* user (u) */
#define HOST_PASSWORD_DEFAULT	""		/* password */
#define SCRIPT_DEFAULT		""	 	/* startup script (s) */
#define OTHER_DEFAULT		"" 		/* other (o) */


/* Login security initial user name and password.
 * Use vxencrypt on host to find encrypted password.
 * Default password provided here is "password".
 */

#ifndef	LOGIN_USER_NAME
#define LOGIN_USER_NAME		"target"
#endif	/* LOGIN_USER_NAME */

#ifndef LOGIN_PASSWORD
#define LOGIN_PASSWORD		"bReb99RRed"	/* "password" */
#endif	/* LOGIN_PASSWORD */


/* install environment variable task create/delete hooks */

#ifdef  INCLUDE_ENV_VARS
#define	ENV_VAR_USE_HOOKS	TRUE
#endif	/* INCLUDE_ENV_VARS */

/* system symbol table configuration parameter */

#ifndef STORE_ABS_SYMBOLS
#define STORE_ABS_SYMBOLS       FALSE
#endif /* ndef STORE_ABS_SYMBOLS */

/*
 * Architecture specific VM page size. For more information see the 
 * respective Architecture Supplement.
 */

#if	((CPU_FAMILY == PPC) || (CPU_FAMILY == ARM) || (CPU_FAMILY == SH) || \
	 (CPU_FAMILY == I80X86))
#   define VM_PAGE_SIZE		4096
#elif	(CPU_FAMILY == MIPS)
#   define VM_PAGE_SIZE         (MMU_PAGE_SIZE)
#elif	((CPU_FAMILY ==SIMSPARCSOLARIS) || (CPU_FAMILY==SIMLINUX))
#   define VM_PAGE_SIZE		0x2000	/* default page size */
#elif	(CPU_FAMILY==SIMNT)
#   define VM_PAGE_SIZE		0x10000	/* default page size */
#else
#   define VM_PAGE_SIZE		8192
#endif	/* (CPU_FAMILY == PPC || ARM || SH || I80X86) */

#if (CPU_FAMILY != MIPS)
#define	SW_MMU_ENABLE	FALSE	/* software MMU simulation disabled by default*/
#else /* CPU_FAMILY != MIPS */
#ifndef CDF_OVERRIDE
#define	SW_MMU_ENABLE	TRUE	/* software MMU simulation enabled by default*/
#endif /* CDF_OVERRIDE */
#endif /* CPU_FAMILY != MIPS */

#if ((CPU == PPC603) || (CPU == PPCEC603) || (CPU == PPC604) || (CPU == PPC970))
    /* The below 2 macros are valid only for PPC60X and PPC970. */
#define USER_HASH_TABLE_ADDRESS      NULL   /* Let the OS decide the address. */
#define USER_HASH_TABLE_SIZE         0x0    /* Let the OS decide the size     */
#endif

/* VxWorks Network Configuration */

#ifdef INCLUDE_NETWORK
/******************************************************************************/
/*                                                                            */
/*                         INCLUDED NETWORK FACILITIES                        */
/*                                                                            */
/******************************************************************************/

#define  INCLUDE_IPATTACH         /* Attach boot END device to IPv4 */
#define  INCLUDE_NET_BOOT         /* Boot parameter process */
#define  INCLUDE_NETMASK_GET      /* network device netmask setup */
#define  INCLUDE_END              /* END interface support */
#define  INCLUDE_END_POLLED_STATS /* poll END device stats gathering */
#define  INCLUDE_MUX              /* MUX support */
#define  INCLUDE_IF               /* Interface Layer */
#define  INCLUDE_ETHERNET         /* Ethernet interface support */
#define  INCLUDE_ROUTE_STORAGE    /* Initialize storage for routing tables */
#define  INCLUDE_LOOPBACK         /* loopback interface */
#define  INCLUDE_MIB2_IF          /* MIB2 IF Counter Instrumentation */
#define  INCLUDE_IPV4             /* IPv4 */
#define  INCLUDE_TCPV4            /* TCP v4 */
#define  INCLUDE_UDPV4            /* UDP v4 */
#define  INCLUDE_ICMPV4           /* ICMP v4 */
#define  INCLUDE_RAWV4		  /* Include this for Raw sockets, ping etc */
#define  INCLUDE_NET_DAEMON       /* Network Daemon Support */
#define  INCLUDE_NET_INIT         /* Initialize tables for network stack */
#define  INCLUDE_DHCPC_LEASE_CLEAN /* DHCP client timestamp removal */
#define  INCLUDE_NET_BOOT_CONFIG   /* network boot device configuration */
#define  INCLUDE_ROUTE             /* Enable Radix/Routing function */
#define  INCLUDE_ROUTING_SOCKET    /* Routing Socket */
#define  INCLUDE_BOOT_LINE_INIT    /* The boot line routines */
#define  INCLUDE_NETPOOLSHOW   /* network pool show routines */
#define  INCLUDE_NETBUFPOOL    /* netBufLib Generic Pool */
#define  INCLUDE_LINKBUFPOOL   /* netBufLib Link Buf Pool Could be optional */
#define  INCLUDE_REMLIB        /* Remote Command Library */
#define  INCLUDE_FTP           /* FTP client */
#define  INCLUDE_NET_DRV       /* Allows access to file system on boot host */
#define  INCLUDE_NET_REM_IO    /* network remote I/O access */
#define  INCLUDE_HOST_TBL      /* host table support */
#define  INCLUDE_BOOTP         /* BOOTP client */
#define  INCLUDE_NET_HOST_SETUP /* assign local hostname to target */
#define  INCLUDE_INETLIB        /* inetLib */
#define  INCLUDE_NBIO_LOG       /* non-blocking message logging */

#define  INCLUDE_NFS
#undef  INCLUDE_NFS_CLIENT_ALL  /* Include All NFS clients */
#define  INCLUDE_NFS2_CLIENT     /* Incldue NFS v2 client */
#undef  INCLUDE_NFS3_CLIENT     /* Include NFS v3 client */
#undef  INCLUDE_NFS_MOUNT_ALL   /* Include Mount all exported FS */

/*
 * Only undefine INCLUDE_APPL_LOG_UTIL if you rebuild the stack with
 * logging completely disabled (LOG_ENABLE_MASK == 0).
 */
#define  INCLUDE_APPL_LOG_UTIL

/*
 * The following default network components are now defined in usrDepend.c and
 * bootConfig.c for scalability when network support is not included in the image.

#define  INCLUDE_BSD_SOCKET
#define  INCLUDE_SOCKLIB
#define  INCLUDE_DOMAIN_INIT
#define  INCLUDE_GTF
#define  INCLUDE_GTF_TIMER_START
#define  INCLUDE_NET_POOL
#define  INCLUDE_NETBUFLIB
 */


/* INCLUDED NETWORK DRIVERS */


/******************************************************************************/
/*                                                                            */
/*                         EXCLUDED NETWORK FACILITIES                        */
/*                                                                            */
/******************************************************************************/
#if 0

#define  INCLUDE_IFCLONE         /* Interface cloning */
#define  INCLUDE_MUX_L2          /* MUX Layer 2 suppport */
#define  INCLUDE_SUBNET_VLAN     /* Subnet-based VLAN suppport */
#define  INCLUDE_L2CONFIG        /* Layer 2 configuration utility */

#define  INCLUDE_IPV6           /* IPv6 support */
#define  INCLUDE_TCPV6          /* TCP v6 support */
#define  INCLUDE_UDPV6          /* UDP v6 support */
#define  INCLUDE_ICMPV6         /* ICMP v6 support */
#define  INCLUDE_IP6ATTACH      /* Attach boot END device to IPv6 */
#define  INCLUDE_RAWV6		/* Include this for Raw sockets, ping6 etc */
#define  INCLUDE_MROUTEV6       /* Multicast routing v6 */

#define  INCLUDE_MIPV6          /* Mobility for IPv6 */
#define  INCLUDE_MIP6_MN        /* IPv6 mobile node */
#define  MIP6_INIT_AT_BOOT TRUE /* init mip app at boot time */

#define  INCLUDE_NETBUFADVLIB   /* Additional network buffer pool features */

#define INCLUDE_SM_NET          /*
                                 * Setup shared memory network from boot
                                 * parameters. Requires SM END driver.
                                 */

#define  INCLUDE_UN             /* AF_LOCAL address family */
#define  INCLUDE_UN_COMP        /* AF_LOCAL/COMP protocol  */

#define  INCLUDE_IPFW
#define  INCLUDE_MROUTE         /* Multicast routing */
#define  INCLUDE_IGMP           /* IGMPv2 */
#define  INCLUDE_IPFLOW         
#define  INCLUDE_HOSTCACHE
#define  INCLUDE_IPV6_FW

#define  INCLUDE_ND             /* Neighbor Discovery */
#define  INCLUDE_MLD            /* Multicast Listener Discovery v1 */
#define  INCLUDE_MLD6_ROUTER    /* MLD Router */

#define  INCLUDE_GETADDRINFO	/* name to addr mapping */
#define  INCLUDE_GETSERVBYNAME   /* service by name */
#define  INCLUDE_GETNAMEINFO     /* addr to name mapping */
#define  INCLUDE_GETNETBYNAME    /* net by name */

#define  INCLUDE_MIB2_IPV6

#define  INCLUDE_NET_SYM_TBL     /* load symbol table from network */

#define  INCLUDE_POSIX_ALL       /* include all available POSIX functions */

/*
 * Include Wind Net PPP support to provide equivalent functionality
 * as the PPP implementation bundled with vxWorks 5.x
 */

#define INCLUDE_VXWORKS_5_X_EQUIV_PPP  /* include Point-to-Point Protocol */

#define  INCLUDE_SNMPD 	        /* SNMP Agent */
#define  INCLUDE_SNMPD_DEBUG	/* SNMP Agent debugging */

#define  INCLUDE_NET_SYSCTL	/* Network sysctl tree support */
#define  INCLUDE_SYSCTL_CLI     /* Command line Sysctl utility */
#define  INCLUDE_DNS_RESOLVER    /* DNS resolver */
#define  INCLUDE_NET_SHOW        /* IP show routines */
#define  INCLUDE_NET_IF_SHOW     /* Interface show routines */
#define  INCLUDE_NET_HOST_SHOW	 /* Host show routines */
#define  INCLUDE_NET_ROUTE_SHOW  /* Routing show routines */
#define  INCLUDE_ICMP_SHOW       /* ICMP show routines */
#define  INCLUDE_IGMP_SHOW       /* IGMP show routines */
#define  INCLUDE_TCP_SHOW        /* TCP show routines */
#define  INCLUDE_UDP_SHOW        /* UDP show routines */
#define  INCLUDE_UN_SHOW         /* AF_LOCAL show routine support */
#define  INCLUDE_UN_COMP_SHOW    /* AF_LOCAL/COMP show routines */
#define  INCLUDE_SNTPC           /* SNTP client */
#define  INCLUDE_SNTPS           /* SNTP server */
#define  INCLUDE_TCP_DEBUG       /* TCP debugging */
#define  INCLUDE_IFCONFIG        /* ifconfig command line/API */
#define  INCLUDE_IFCONFIG_PREFIX /* prefix component for ifconfig */
#define  INCLUDE_PING            
#define  INCLUDE_PING6
#define  INCLUDE_TELNET          /* Telnet server */
#define  INCLUDE_TELNET_CLIENT
#define  INCLUDE_RLOGIN          /* Rlogin client/server */
#define  INCLUDE_FTP6_SERVER     /* FTP v4/v6 server */
#define  INCLUDE_FTPD6_GUEST_LOGIN /* enable anonymous login for FTP server */
#define  INCLUDE_TFTP_SERVER
#define  INCLUDE_TFTP_CLIENT
#define  INCLUDE_NDP             /* ndp command-line utility */
#define  INCLUDE_ADDRCTL_POLICY  /* address selection command-line utility */
#define  INCLUDE_IFLIB           /* IPv4 interface address library */
#define  INCLUDE_IF6LIB          /* IPv6 interface address library */
#define  INCLUDE_IFMEDIA         /* SIOCGIFMEDIA/SIOCSIFMEDIA support */
#define  INCLUDE_ROUTECMD        /* routec command-line utility */
#define  INCLUDE_RTADV           /* router advertisement daemon */
#define  INCLUDE_RTSOL           /* router solicitation daemon */
#define  INCLUDE_RDISC           /* ICMP v4 router discovery server */
#define  INCLUDE_RIP             /* Routing Information Protocol v1/v2 */
#define  INCLUDE_RIPNG           /* Routing Information Protocol ng */
#define  INCLUDE_NETSTAT_IF
#define  INCLUDE_NETSTAT_ROUTE
#define  INCLUDE_NETSTAT_MROUTE
#define  INCLUDE_NETSTAT_MROUTEV6
#define  INCLUDE_NETSTAT_IPV4
#define  INCLUDE_NETSTAT_IPV6
#define  INCLUDE_NETSTAT_TCP
#define  INCLUDE_NETSTAT_UDP
#define  INCLUDE_NETSTAT_ICMPV4
#define  INCLUDE_NETSTAT_ICMPV6
#define  INCLUDE_NETSTAT_RAWV6
#define  INCLUDE_NETSTAT_IGMP
#define  INCLUDE_NETSTAT_SCTP
#define  INCLUDE_NETSTAT_UN_COMP
#define  INCLUDE_PREFIXCMD       /* prefix command utility to add IPv6 prefixes */
#define  INCLUDE_ICMPLIB
#define  INCLUDE_BPF
#define  INCLUDE_FTP6

/* DHCPv4 */

#define  INCLUDE_DHCPC
#define  INCLUDE_DHCP_CORE
#define  INCLUDE_DHCPS
#define  INCLUDE_DHCPS_SHARE
#define  INCLUDE_DHCPC_SHOW
#define  INCLUDE_DHCPR
#define  INCLUDE_DHCPC_LEASE_SAVE
#define  INCLUDE_DHCPC_BOOT
#define  INCLUDE_DHCPC_SHARE
#define  INCLUDE_DHCPC_LEASE_GET
#define  INCLUDE_DHCPC_LEASE_TEST

/* DHCPv6 runtime components */

#define  INCLUDE_DHCP6C         /* DHCPv6 client */
#define  INCLUDE_DHCP6C_SHOW    /* DHCPv6 client show routines */
#define  INCLUDE_DHCP6R         /* DHCPv6 relay */
#define  INCLUDE_DHCP6S         /* DHCPv6 server */

/* MIBs */

#define  INCLUDE_MIB2_ALL
#define  INCLUDE_MIB2_IPV6ALL
#define  INCLUDE_MIB2_IP
#define  INCLUDE_MIB2_UDP
#define  INCLUDE_MIB2_TCP
#define  INCLUDE_MIB2_ICMP
#define  INCLUDE_MIB2_SYSTEM
#define  INCLUDE_MIB2_UDPV6
#define  INCLUDE_MIB2_TCPV6
#define  INCLUDE_MIB2_ICMPV6
#define  INCLUDE_MIB2_MLD
#define  INCLUDE_MIB2_IFMGT

#define  INCLUDE_ARP_API
#define  INCLUDE_GIF
#define  INCLUDE_STF
#define  INCLUDE_IFNAME
#define  INCLUDE_GETIFADDRS
#define  INCLUDE_NET_MD5
#define  INCLUDE_NET_APPUTIL
#define  INCLUDE_SNPRINTF
#define  INCLUDE_STRICMP
#define  INCLUDE_STRNICMP
#define  INCLUDE_KERN_RANDOM
#define  INCLUDE_RPC
#define  INCLUDE_XDR_BOOL_T
#define  INCLUDE_XDR

#define  INCLUDE_NFS_SERVER_ALL  /* Include all NFS servers */
#define  INCLUDE_NFS2_SERVER     /* Include NFS v2 server */
#define  INCLUDE_NFS3_SERVER     /* Include NFS v3 server */
#define  INCLUDE_NFS_CLIENT_ALL  /* Include All NFS clients */
#define  INCLUDE_NFS2_CLIENT     /* Incldue NFS v2 client */
#define  INCLUDE_NFS3_CLIENT     /* Include NFS v3 client */
#define  INCLUDE_NFS_MOUNT_ALL   /* Include Mount all exported FS */

#define  INCLUDE_ADDIF
#define  INCLUDE_NET_FUNCBIND
#define  INCLUDE_PROXY_SERVER
#define  INCLUDE_TUNNELLIB
#define  INCLUDE_TUNNEL_MUX_REG
#define  INCLUDE_FASTUDP
#define  INCLUDE_FASTUDP6
#define  INCLUDE_ZBUF_SOCK
#define  INCLUDE_CIDR_MAPI
#define  INCLUDE_CIDR_METHODS

#define INCLUDE_OLDROUTE
#define INCLUDE_IFINDEX

#define  INCLUDE_RARP
#define  INCLUDE_FASTPATH

#define  INCLUDE_QOS_INGRESS    /* Ingress QOS  support */
#define  INCLUDE_QOS_INGRESS_HOOKS    /* Ingress QOS hooks */

#define  INCLUDE_SCTP           /* Stream Control Transmission Protocol */

#define INCLUDE_SAL_CLIENT      /* Socket Application Library (Client) */
#define INCLUDE_SAL_SERVER      /* Socket Application Library (Server) */
#define INCLUDE_SNS             /* Socket Name Service */
#define INCLUDE_SNS_RTP         /* Socket Name Service (RTP daemon suport) */
#define INCLUDE_SNS_MP          /* Dist. Socket Name Service */
#define INCLUDE_SNS_MP_RTP      /* Dist. Socket Name Service (RTP daemon) */
#define INCLUDE_SNS_SHOW        /* Socket Name Service show routines */

#define INCLUDE_TIPC                    /* include TIPC protocol */
#define INCLUDE_TIPC_MEDIA_ETH          /* ethernet media */
#define INCLUDE_TIPC_MEDIA_SM           /* shared memory media */
#define INCLUDE_TIPC_CONFIG_STR         /* use static config string */
#define INCLUDE_TIPC_CONFIG_HOOK_BOOT   /* call boot params config routine */
#define INCLUDE_TIPC_CONFIG_HOOK_USER   /* call user config routine */
#define INCLUDE_TIPC_SHOW               /* include TIPC show routines */
#define INCLUDE_TIPC_MEMPOOL            /* include TIPC memory buffers */
#define INCLUDE_TIPC_SOCKET             /* include TIPC socket API */


/* SELECT_NET_ROUTE_SHOW_BUFFERING selection */
#if 0
#define  INCLUDE_NET_ROUTE_SHOW_NBIO  /* uses nbioLogLib */
#elif 1
#define   INCLUDE_NET_ROUTE_SHOW_SNPRINTF
#else
#define   INCLUDE_NET_ROUTE_SHOW_UNBUF
#endif

#define  INCLUDE_SYSCTL

/* The following components are available only with RTPs */

#define  INCLUDE_SYSCTL_HW
#define  INCLUDE_SC_SYSCTL
#define  INCLUDE_SC_SOCKLIB


/* The following components are available only with the MSP */

#define  INCLUDE_VIRTUAL_STACK
#define  INCLUDE_VIRTUAL_STACK_DEBUG
#define  INCLUDE_VIRTUAL_STACK_EXAMPLE
#define  INCLUDE_VIRTUAL_STACK_OLD_INTERFACE
#define  INCLUDE_VIRTUAL_STACK_SHOW_FUNCTIONS

/* INCLUDE_IFCONFIG */
#define IFCONFIG_ENTRY_1  NULL
#define IFCONFIG_ENTRY_2  NULL
#define IFCONFIG_ENTRY_3  NULL
#define IFCONFIG_ENTRY_4  NULL
#define IFCONFIG_ENTRY_5  NULL
#define IFCONFIG_ENTRY_6  NULL
#define IFCONFIG_ENTRY_7  NULL
#define IFCONFIG_ENTRY_8  NULL
#define IFCONFIG_ENTRY_9  NULL
#define IFCONFIG_ENTRY_10 NULL

/* INCLUDE_ROUTECMD */
#define ROUTE_ENTRY_1 NULL
#define ROUTE_ENTRY_2 NULL
#define ROUTE_ENTRY_3 NULL
#define ROUTE_ENTRY_4 NULL
#define ROUTE_ENTRY_5 NULL

/* INCLUDE_NDP */
#define NDP_ENTRY_1 NULL
#define NDP_ENTRY_2 NULL
#define NDP_ENTRY_3 NULL
#define NDP_ENTRY_4 NULL
#define NDP_ENTRY_5 NULL

/* INCLUDE_ADDRCTL_POLICY */
#define ADDRCTL_POLICY_ENTRY_1 NULL
#define ADDRCTL_POLICY_ENTRY_2 NULL
#define ADDRCTL_POLICY_ENTRY_3 NULL

/* INCLUDE_ARP_API */

#define ARP_ENTRY_1_HOST NULL
#define ARP_ENTRY_1_ADDR NULL
#define ARP_ENTRY_2_HOST NULL
#define ARP_ENTRY_2_ADDR NULL
#define ARP_ENTRY_3_HOST NULL
#define ARP_ENTRY_3_ADDR NULL
#define ARP_ENTRY_4_HOST NULL
#define ARP_ENTRY_4_ADDR NULL
#define ARP_ENTRY_5_HOST NULL
#define ARP_ENTRY_5_ADDR NULL

#endif  /* FALSE */

#endif	/* INCLUDE_NETWORK */

/* INCLUDE_GETADDRINFO */
#define IGNORE_LINKLOCAL  FALSE /* If this parameter is TRUE, getaddrinfo() */
                                /* will ignore link-local addresses unless */
				/* AI_ADDRCONFIG is set. */

#ifdef INCLUDE_MIB2_IF
/* Disable 64-bit counters by default */

#define MIB2IF_64BIT_COUNTERS 0
#endif /* INCLUDE_MIB2_IF */

/* SNMP configuration parameters */

#define SNMP_TRACE_LEVEL            0    /* Must be >= 0 and <= 3 with higher */
                                         /* values giving more info and 0     */
                                         /* giving no info                    */


/* INCLUDE_IP6ATTACH */
#define IFF_INET6_UP_CFG  1

/* INCLUDE_PROXY_SERVER */
#define PROXYD_MAIN_ADDRESS sysBootParams.ead
#define PROXYD_PROXY_ADDRESS sysBootParams.bad
#define CLNT_TBL_SZ_CFG 8
#define PORT_TBL_SZ_CFG 8
#define ARP_DEBUG_CFG FALSE
#define PROXY_ARP_VERBOSE_CFG FALSE
#define PROXY_BROADCAST_VERBOSE_CFG FALSE
#define PROXY_BROADCAST_FWD_CFG TRUE
#define ARP_REGISTER_CFG TRUE

/* INCLUDE_RARP */
#define RARP_DEBUG FALSE


/* NETWORK PARAMETERS */

/* INCLUDE_NET_INIT */
#undef  MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  256
#define MAX_LINKHDR_CFG 	16
#define MAX_PROTOHDR_CFG 	60

/* INCLUDE_IF */
#define IFQ_MAXLEN_CFG 50
#define IF_INDEXLIM_CFG   8
#define ND6_IFINFO_NUM_CFG		  8

/* INCLUDE_NET_DAEMON */
#define NET_JOB_NUM_CFG         85
#define NET_TASK_PRIORITY       50
#define NET_TASK_STACKSIZE      10000
#define NET_TASK_OPTIONS        VX_SUPERVISOR_MODE | VX_UNBREAKABLE

/* INCLUDE_ADDIF */
#define ADDIF_NUM               1              /* interface unit number */
#define ADDIF_NAME              "fei"          /* interface name */
#define ADDIF_ADDR              "192.168.1.1"  /* interface address (IPv4 or IPv6) */
#define ADDIF_MASK              0xffff0000     /* interface subnet mask (IPv4) */
                                               /* or prefix length (IPv6)      */

/* INCLUDE_VIRTUAL_STACK */
#define FTP6_SERVER_IN_VS0 	FALSE	
#define	TFTP_SERVER_IN_VS0 	FALSE
#define	RLOGIN_IN_VS0 		FALSE
#define	TELNET_SERVER_IN_VS0 	FALSE
#define TELNET_CLIENT_IN_VS0    FALSE
#define	DHCPR_IN_VS0 		FALSE
#define	DNS_CLIENT_IN_VS0 	FALSE
#define	DHCPS_IN_VS0 		FALSE
#define	RIP_IN_VS0 		FALSE
#define	RIPNG_IN_VS0 		FALSE
#define RDISC_IN_VS0 		FALSE
#define	RTADV_IN_VS0 		FALSE
#define	RTSOL_IN_VS0 		FALSE
#define	PROXY_ARP_SERVER_IN_VS0 FALSE
#define	NFS_SERVER_IN_VS0       FALSE
#define DHCP6S_IN_VS0           FALSE
#define DHCP6C_IN_VS0           FALSE
#define DHCP6R_IN_VS0           FALSE
#define SNTP_CLIENT_IN_VS0	FALSE
#define SNTP_SERVER_IN_VS0	FALSE
#define MLDR_IN_VS0     	FALSE

#define VS_DELTIMEO_CFG 	7000
#define VS_SOCKTIMEO_CFG 	1000
#define VS_APPLTIMEO_CFG	0

#define RPC_IN_VS0              FALSE

/* INCLUDE_GTF */
                                                /* gtfTimerManagerCreate( */
#define GTF_NBRTIMERUNIT_CFG            7       /*   nbrTimerUnit, */
#define GTF_BINPOWTIMERSLOT_CFG 9       /*   binPowTimerSlot, */
#define GTF_BINPOWGRANSCALE_CFG         2       /*   binPowGranScale, */
#define GTF_SIZETIMERFREEQUEUE_CFG 100  /*   sizeTimerFreeQueue, */
#define GTF_HZ_CFG                      10


/* INCLUDE_NET_POOL */
                                /* System pool */
#undef  NUM_SYS_MBLKS
#define NUM_SYS_MBLKS           20
#undef  NUM_SYS_CLBLKS
#define NUM_SYS_CLBLKS          20
#undef  PMA_SYSPOOL
#define PMA_SYSPOOL             NULL
#undef  PMS_SYSPOOL
#define PMS_SYSPOOL             0
#undef  SIZ_SYS_16
#define SIZ_SYS_16              20
#undef  NUM_SYS_16
#define NUM_SYS_16              250
#undef  PMA_SYS_16
#define PMA_SYS_16              NULL
#undef  PMS_SYS_16
#define PMS_SYS_16              0
#undef  SIZ_SYS_32
#define SIZ_SYS_32              44
#undef  NUM_SYS_32
#define NUM_SYS_32              200
#undef  PMA_SYS_32
#define PMA_SYS_32              NULL
#undef  PMS_SYS_32
#define PMS_SYS_32              0
#undef  SIZ_SYS_64
#define SIZ_SYS_64              96
#undef  NUM_SYS_64
#define NUM_SYS_64              100
#undef  PMA_SYS_64
#define PMA_SYS_64              NULL
#undef  PMS_SYS_64
#define PMS_SYS_64              0
#undef  SIZ_SYS_128
#define SIZ_SYS_128             172
#undef  NUM_SYS_128
#define NUM_SYS_128             150
#undef  PMA_SYS_128
#define PMA_SYS_128             NULL
#undef  PMS_SYS_128
#define PMS_SYS_128             0
#undef  SIZ_SYS_256
#define SIZ_SYS_256             292
#undef  NUM_SYS_256
#define NUM_SYS_256             100
#undef  PMA_SYS_256
#define PMA_SYS_256             NULL
#undef  PMS_SYS_256
#define PMS_SYS_256             0
#undef  SIZ_SYS_512
#define SIZ_SYS_512             664
#undef  NUM_SYS_512
#define NUM_SYS_512             50
#undef  PMA_SYS_512
#define PMA_SYS_512             NULL
#undef  PMS_SYS_512
#define PMS_SYS_512             0
#undef  SIZ_SYS_1024
#define SIZ_SYS_1024            1144
#undef  NUM_SYS_1024
#define NUM_SYS_1024            30
#undef  PMA_SYS_1024
#define PMA_SYS_1024            NULL
#undef  PMS_SYS_1024
#define PMS_SYS_1024            0
#undef  SIZ_SYS_2048
#define SIZ_SYS_2048            2048
#undef  NUM_SYS_2048
#define NUM_SYS_2048            0
#undef  PMA_SYS_2048
#define PMA_SYS_2048            NULL
#undef  PMS_SYS_2048
#define PMS_SYS_2048            0

                                /* Data pool */
#undef  NUM_DAT_MBLKS
#define NUM_DAT_MBLKS           NUM_DAT_CLBLKS
#undef  NUM_DAT_CLBLKS
#define NUM_DAT_CLBLKS          NUM_DAT_64 + NUM_DAT_128 + NUM_DAT_256 + \
                                NUM_DAT_512 + NUM_DAT_1024 + NUM_DAT_2048 + \
				NUM_DAT_4096 + NUM_DAT_8192 + NUM_DAT_16384 + \
				NUM_DAT_32768 + NUM_DAT_65536
#undef  PMA_DATPOOL
#define PMA_DATPOOL             NULL
#undef  PMS_DATPOOL
#define PMS_DATPOOL             0
#undef  NUM_DAT_64
#define NUM_DAT_64              100
#undef  PMA_DAT_64
#define PMA_DAT_64              NULL
#undef  PMS_DAT_64
#define PMS_DAT_64              0
#undef  NUM_DAT_128
#define NUM_DAT_128             400
#undef  PMA_DAT_128
#define PMA_DAT_128             NULL
#undef  PMS_DAT_128
#define PMS_DAT_128             0
#undef  NUM_DAT_256
#define NUM_DAT_256             40
#undef  PMA_DAT_256
#define PMA_DAT_256             NULL
#undef  PMS_DAT_256
#define PMS_DAT_256             0
#undef  NUM_DAT_512
#define NUM_DAT_512             40
#undef  PMA_DAT_512
#define PMA_DAT_512             NULL
#undef  PMS_DAT_512
#define PMS_DAT_512             0
#undef  NUM_DAT_1024
#define NUM_DAT_1024            25
#undef  PMA_DAT_1024
#define PMA_DAT_1024            NULL
#undef  PMS_DAT_1024
#define PMS_DAT_1024            0
#undef  NUM_DAT_2048
#define NUM_DAT_2048            50
#undef  PMA_DAT_2048
#define PMA_DAT_2048            NULL
#undef  PMS_DAT_2048
#define PMS_DAT_2048            0
#undef  NUM_DAT_4096 
#define NUM_DAT_4096            0 
#undef  PMA_DAT_4096
#define PMA_DAT_4096		0
#undef  PMS_DAT_4096  
#define PMS_DAT_4096	        0
#undef  NUM_DAT_8192
#define NUM_DAT_8192            0 
#undef  PMA_DAT_8192
#define PMA_DAT_8192            0 
#undef  PMS_DAT_8192
#define PMS_DAT_8192		0	
#undef  NUM_DAT_16384
#define NUM_DAT_16384		0
#undef  PMA_DAT_16384
#define PMA_DAT_16384    	0
#undef  PMS_DAT_16384
#define PMS_DAT_16384		0	
#undef  NUM_DAT_32768 
#define NUM_DAT_32768		0
#undef  PMA_DAT_32768
#define PMA_DAT_32768		0
#undef  PMS_DAT_32768
#define PMS_DAT_32768		0
#undef  NUM_DAT_65536
#define NUM_DAT_65536		0
#undef  PMA_DAT_65536
#define PMA_DAT_65536		0
#undef 	PMS_DAT_65536
#define PMS_DAT_65536		0

/* DSI buffer pools */

#define DSI_NUM_SOCKETS    200

#define DSI_DATA_32         50
#define DSI_DATA_64        100
#define DSI_DATA_128       200
#define DSI_DATA_256        40
#define DSI_DATA_512        40
#define DSI_DATA_1K         10
#define DSI_DATA_2K         10
#define DSI_DATA_4K         10
#define DSI_DATA_8K         10
#define DSI_DATA_16K         4
#define DSI_DATA_32K         0
#define DSI_DATA_64K         0

                                   
/* TIPC sockets and buffer pools */                                   

#define TIPC_NUM_SOCKETS    200

#define TIPC_DATA_00032        50
#define TIPC_DATA_00064       100
#define TIPC_DATA_00128       200
#define TIPC_DATA_00256        40
#define TIPC_DATA_00512        40
#define TIPC_DATA_01024        20
#define TIPC_DATA_02048        20
#define TIPC_DATA_04096         2
#define TIPC_DATA_08192         0
#define TIPC_DATA_16384         0
#define TIPC_DATA_32768         0
#define TIPC_DATA_65536         0

/* Bootrom configuration */

                                /* System pool */
#undef  NUM_SYS_MBLKS_MIN
#define NUM_SYS_MBLKS_MIN       (2 * NUM_SYS_CLBLKS_MIN)
#undef  NUM_SYS_CLBLKS_MIN
#define NUM_SYS_CLBLKS_MIN      40
#undef  NUM_SYS_16_MIN
#define NUM_SYS_16_MIN          60
#undef  NUM_SYS_32_MIN
#define NUM_SYS_32_MIN          50
#undef  NUM_SYS_64_MIN
#define NUM_SYS_64_MIN          25
#undef  NUM_SYS_128_MIN
#define NUM_SYS_128_MIN         40
#undef  NUM_SYS_256_MIN
#define NUM_SYS_256_MIN         25
#undef  NUM_SYS_512_MIN
#define NUM_SYS_512_MIN         15
#undef  NUM_SYS_1024_MIN
#define NUM_SYS_1024_MIN        10

                                /* Data pool */
#undef  NUM_DAT_MBLKS_MIN
#define NUM_DAT_MBLKS_MIN       200
#undef  NUM_DAT_CLBLKS_MIN
#define NUM_DAT_CLBLKS_MIN      NUM_DAT_64_MIN + NUM_DAT_128_MIN + \
				NUM_DAT_256_MIN + NUM_DAT_512_MIN + \
				NUM_DAT_1024_MIN + NUM_DAT_2048_MIN
#undef  NUM_DAT_64_MIN
#define NUM_DAT_64_MIN          50
#undef  NUM_DAT_128_MIN
#define NUM_DAT_128_MIN         200
#undef  NUM_DAT_256_MIN
#define NUM_DAT_256_MIN         20
#undef  NUM_DAT_512_MIN
#define NUM_DAT_512_MIN         20
#undef  NUM_DAT_1024_MIN
#define NUM_DAT_1024_MIN        25
#undef  NUM_DAT_2048_MIN
#define NUM_DAT_2048_MIN        50
#undef  NUM_DAT_4096_MIN
#define NUM_DAT_4096_MIN        0 
#undef  NUM_DAT_8192_MIN
#define NUM_DAT_8192_MIN        0 
#undef  NUM_DAT_16384_MIN
#define NUM_DAT_16384_MIN	0
#undef  NUM_DAT_32768_MIN 
#define NUM_DAT_32768_MIN	0
#undef  NUM_DAT_65536_MIN
#define NUM_DAT_65536_MIN	0

/* INCLUDE_MUX_L2 */
#define MUX_L2_NUM_PORTS_CFG   16   /* number of ports that the device has */
#define MUX_L2_MAX_VLANS_CFG   16   /* max number of 802.1Q VLANs supports */

/* INCLUDE_TUNNELLIB */
#define TUNNEL_PMTU_REFRESH_CFG 600

#if 0
/* INCLUDE_WDB_COMM_END */
#define WDB_END_MTU 		1500
#define WDB_END_DEVICE_NAME 	NULL
#define WDB_END_DEVICE_UNIT	NULL
#endif

/* INCLUDE_IPV4 */

#define IPPORT_RESERVED_CFG 1024
#define IPPORT_RESERVEDSTART_CFG 600
#define IPPORT_USERRESERVED_CFG 5000
#define IPPORT_HIFIRSTAUTO_CFG 49152
#define IPPORT_HILASTAUTO_CFG 65535
#define IPFORWARDING_CFG ipForwardCfg  /* ipForwardCfg is TRUE or FALSE depending */
				       /* on whether the stack was built as a host */
				       /* or a router. Replace ipForwardCfg with */
				       /* TRUE or FALSE to change the default */
				       /* configuration. */
#define IPSENDREDIRECTS_CFG TRUE
#define IP_DEFTTL_CFG 64
#define IP_DOSOURCEROUTE_CFG FALSE
#define IP_ACCEPTSOURCEROUTE_CFG FALSE
#define IP_CHECKINTERFACE_CFG FALSE
#define IPSTEALTH_CFG FALSE
#define RTEXPIRE_CFG (60*60)
#define RTMINEXPIRE_CFG 10
#define RTMAXCACHE_CFG 128
#define FASTFORWARDING_CFG FALSE
#define SUBNETS_ARE_LOCAL_CFG FALSE
#define FW_ENABLE_CFG FALSE
#define FW_ONE_PASS_CFG TRUE
#define FW_DEBUG_CFG FALSE
#define FW_VERBOSE_CFG FALSE
#define FW_VERBOSE_LIMIT_CFG 0
#define DYN_BUCKETS_CFG 256
#define DYN_MAX_CFG 1000
#define DYN_ACK_LIFETIME_CFG 300
#define DYN_SYN_LIFETIME_CFG 20
#define DYN_FIN_LIFETIME_CFG 20
#define DYN_RST_LIFETIME_CFG 5
#define DYN_UDP_LIFETIME_CFG 10
#define DYN_SHORT_LIFETIME_CFG 30
#define IPINTRQ_IFQ_MAXLEN_CFG 50
#define IP_MAXFRAGPACKETS_CFG 16
#define IP_MAXFRAGSPERPACKET_CFG 45
#define ICMPMASKREPL_CFG FALSE
#define ICMP_DROP_REDIRECT_CFG FALSE
#define ICMP_LOG_REDIRECT_CFG FALSE
#define ICMPLIM_CFG  200
#define ICMPBMCASTECHO_CFG TRUE
#define ICMPV4_MTU_MIN_CFG 296		/* Minimum PMTU for ICMPv4 */
#define ICMPV4_VALIDATION_REQ_CFG 1	/* Require needs-frag validation */
#define ICMPV4_TIMESTAMP_REQ_SUPPORT_CFG FALSE /* timestamp request support */
#define TCP_MSSDFLT_CFG 512
#define TCP_DO_RFC1323_CFG TRUE
#define TCP_DO_RFC1644_CFG FALSE
#define DO_TCPDRAIN_CFG TRUE
#define ICMP_MAY_RST_CFG TRUE
#define TCP_SEQ_GENSCHEME_CFG TRUE
#define TCP_SEQ_ISNSEED_INTERVAL 0
#define TCP_INFLIGHT_ENABLE FALSE
#define TCP_INFLIGHT_DEBUG FALSE
#define TCP_INFLIGHT_MIN 6144
#define TCP_INFLIGHT_MAX (65535 << 14)
#define TCP_KEEPINIT_CFG 75000
#define TCP_KEEPIDLE_CFG 7200000
#define TCP_KEEPINTVL_CFG 75000
#define TCP_KEEPCNT_CFG 8
#define TCP_DELACKTIME_CFG 100
#define TCP_MSL_CFG 30000
#define TCP_ALWAYS_KEEPALIVE_CFG FALSE
#define TCP_SENDSPACE_CFG (1024*16)
#define TCP_RECVSPACE_CFG (1024*16)
#define TCP_LOG_IN_VAIN_CFG FALSE
#define TCP_BLACKHOLE_CFG FALSE
#define TCP_DELACK_ENABLED_CFG TRUE
#define TCP_LQ_OVERFLOW_CFG TRUE
#define TCP_DROP_SYNFIN_CFG FALSE
#define TCP_REASS_GLOBAL_MAX_CFG 512
#define TCP_REASS_ENDPT_MAX_CFG (-1)
#define UDPCKSUM_CFG TRUE
#define UDP_LOG_IN_VAIN_CFG FALSE
#define UDP_BLACKHOLE_CFG FALSE
#define UDP_MAXDGRAM_CFG 9216
#define UDP_RECVSPACE_CFG 42080
#define RAW_MAXDGRAM_CFG 8192
#define RAW_RECVSPACE_CFG 8192
#define TCP_PATH_MTU_DISCOVERY_CFG TRUE
#define TCP_SLOWSTART_FLIGHTSIZE_CFG 1
#define TCP_LOCAL_SLOWSTART_FLIGHTSIZE_CFG 65535
#define TCP_DO_NEWRENO_CFG TRUE
#define ICMPERRORLEN_CFG       64
#define ARP_MAX_ENTRIES_CFG 0
#define ARP_PRUNE_INTVL_CFG  (5*60)
#define ARP_MAX_AGE_CFG  (20*60)
#define ARP_HOST_DOWN_TIME_CFG 20
#define ARP_MAXTRIES_CFG 5
#define ARP_USELOOPBACK_CFG TRUE
#define ARP_PROXYALL_CFG FALSE
#define ARP_GRATUITUOUS_CFG FALSE            /* send gratuituous ARP when */
					     /* link/inet is enabled */
#define LOG_ARP_WRONG_IFACE_CFG TRUE
#define IPV4_RT_DISP_TBL ptRibDispatchTable

/* INCLUDE_QOS_INGRESS */

/* 
 * The following parameter specifies the Job queue that will be used to 
 * schedule the deferred queue processing job.
 */

#define QOS_JOBQ      netJobQueueId  /* Ingress QoS Job Queue */


/*
 * The following parameter secifies the the priority at which the deferred 
 * queue processing job will be scheduled on the Job queue defined by QOS_JOBQ.
 * Valid values range from 0 to 31. For correct operation, this value should 
 * be less than the priority at which the driver receive routine executes.
 */

#define QOS_JOBQ_PRI  NET_TASK_QJOB_PRI - 1  /* Ingress QoS Job Queue priority*/


/* 
 * The following parameters specifies the the priority queue on which the packet
 * is deferred for processing, if the packet classification routine returns 
 * QOS_DEFER_PKT with -1 in the priority field. Valid values range from 0 to 31.
 */

#define QOS_DEFAULT_PRI		0  /* default priority at which to defer pkts*/

/* INCLUDE_SCTP */

#define SCTP_SENDSPACE_CFG      (128 * 1024)
#ifdef INCLUDE_IPV6
#define SCTP_RECVSPACE_CFG      128 * (1024 + sizeof(struct sockaddr_in6))
#else
#define SCTP_RECVSPACE_CFG      128 * (1024 + sizeof(struct sockaddr_in))
#endif /* INCLUDE_IPV6 */
#define SCTP_AUTO_ASCONF_CFG            FALSE
#define SCTP_ECN_CFG                    TRUE
#define SCTP_ECN_NONCE_CFG              FALSE
#define SCTP_STRICT_SACKS_CFG           FALSE
#define SCTP_NO_CKSUM_ON_LOOPBACK_CFG   TRUE
#define SCTP_STRICT_INIT_CFG            TRUE
#define SCTP_PEER_CHUNK_OH_CFG          sizeof(struct mbuf)
#define SCTP_MAX_BURST_DEF_CFG          8
#define SCTP_MAX_CHUNKS_ON_QUEUE_CFG    512
#define SCTP_DELAYED_SACK_TIME_DEF_CFG  200   /* recv timer def = 200ms  */
#define SCTP_HEARTBEAT_INTERVAL_DEF_CFG 30000  /* 30 seconds + RTO (in ms) */
#define SCTP_DEF_PMTU_RAISE_SEC_CFG     600 /* 10 min between raise attempts */
#define SCTP_DEF_MAX_SHUTDOWN_SEC_CFG   180
#define SCTP_DEF_SECRET_LIFE_SEC_CFG    3600
#define SCTP_RTO_UPPER_BOUND_CFG        (60000)	/* 60 sec in ms */
#define SCTP_RTO_LOWER_BOUND_CFG        (1000)	/* 1 sec in ms */
#define SCTP_RTO_INITIAL_CFG            (3000)	/* 3 sec in ms */
#define SCTP_DEFAULT_COOKIE_LIFE_CFG    60  /* in seconds */
#define SCTP_DEF_MAX_INIT_CFG           8
#define SCTP_DEF_MAX_SEND_CFG           10
#define SCTP_OSTREAM_INITIAL_CFG        10
#define SCTP_DEBUG_CFG                  FALSE

/* INCLUDE_TCPV6 */
#define TCP_V6MSSDFLT_CFG                 1024
#define TCPV6_MTU_MIN_CFG		  0

/*
 * ipForwardCfg is TRUE or FALSE depending on whether the stack was built as a
 * host or a router. Replace ipForwardCfg with TRUE or FALSE to change the
 * default configuration.
 */
#define IPV6CTL_FORWARDING_CFG            ipForwardCfg
#define IPV6CTL_SENDREDIRECTS_CFG         TRUE
#define IPV6CTL_DEFHLIM_CFG               64
#define IPV6CTL_MAXFRAGPACKETS_CFG        16
#define IPV6CTL_ACCEPT_RTADV_CFG          FALSE
#define IPV6CTL_KEEPFAITH_CFG             FALSE
#define IPV6CTL_LOG_INTERVAL_CFG          5
#define IPV6CTL_HDRNESTLIMIT_CFG          50
#define IPV6CTL_DAD_COUNT_CFG             1
#define IPV6CTL_AUTO_FLOWLABEL_CFG        TRUE
#define IPV6CTL_DEFMCASTHLIM_CFG          1
#define IPV6CTL_GIF_HLIM_CFG              30
#define IPV6CTL_USE_DEPRECATED_CFG        TRUE
#define IPV6CTL_RR_PRUNE_CFG              5
#define ND6_USETEMPADDR_CFG           	  FALSE
#define ND6_TEMPPLTIME_CFG            	  86400   /* 1 day */
#define ND6_TEMPVLTIME_CFG            	  604800  /* 1 week */
#define IPV6CTL_PREFER_TEMPADDR_CFG       FALSE
#define IPV6CTL_MCAST_PMTU_CFG            FALSE
#define IPV6CTL_V6ONLY_CFG                FALSE
#define IPV6CTL_AUTO_LINKLOCAL_CFG        TRUE
#define ICMPV6CTL_REDIRACCEPT_CFG         FALSE
#define ICMPV6CTL_REDIRTIMEOUT_CFG        600
#define ND6_PRUNE_CFG           	  1
#define ND6_DELAY_CFG           	  5
#define ND6_UMAXTRIES_CFG       	  3
#define ND6_MMAXTRIES_CFG       	  3
#define ND6_USELOOPBACK_CFG     	  TRUE
#define ICMPV6CTL_NODEINFO_CFG            3
#define ICMPV6CTL_ERRPPSLIMIT_CFG         100
#define ICMPV6_MTU_MIN_CFG		  0
#define ND6_MAXNUDHINT_CFG      	  0
#define ND6_MAXQUEUELEN_CFG      	  1
#define ND6_DEBUG_CFG           	  FALSE
#define IPV6CTL_RTEXPIRE_CFG              3600
#define IPV6CTL_RTMINEXPIRE_CFG           10
#define IPV6CTL_RTMAXCACHE_CFG            128
#define IPV6CTL_RTTIMEOUT_CFG		  3600
#define IPV6_FW_ADD_CFG                   TRUE
#define IPV6_FW6_DEBUG_CFG                TRUE
#define IPV6_FW6_VERBOSE_CFG              FALSE
#define IPV6_FW6_VERBOSE_LIMIT_CFG        0
#define IPV6_RT_DISP_TBL 		  avlRibDispatchTable
#define IP6INTRQ_IFQ_MAXLEN_CFG		  50

/* INCLUDE_MIPV6 */
#define MIPV6CTL_DEBUG_CFG                FALSE
#define MIPV6CTL_USE_IPSEC_CFG            FALSE
#define MIPV6CTL_RR_HINT_PPSLIM_CFG       10

/* the following are applicable to MLDv2 only */
#define ICMPV6CTL_MLD_MAXSRCFILTER_CFG    128
#define ICMPV6CTL_MLD_SOMAXSRC_CFG        64
#define ICMPV6CTL_MLD_VERSION_CFG         0

/* INCLUDE_IPPROTO */
#define IP_MAX_UNITS      	32
#define NMBCLUSTERS_CFG  	64

/* INCLUDE_ROUTE_STORAGE */
#define ROUTE_DEFAULT_WEIGHT 100 

/* INCLUDE_REMLIB */
#define RSH_STDERR_SETUP_TIMEOUT -1

/*
 * INCLUDE_DNS_RESOLVER
 *
 * The values for the RESOLVER_DOMAIN and RESOLVER_DOMAIN_SERVER parameters
 * MUST be set to appropriate values when this component is included. 
 * It is very important that RESOLVER_DOMAIN_SERVER is set to the address of
 * a valid DNS server or it might result in long delays for applications. For
 * example, if you try to ping a host without specifying the PING_OPT_NOHOST
 * flag and if the server setting is incorrect or the server is down, ping
 * will appear to "hang" since it will try to resolve the IP address of the
 * peer to its hostname when the response is received. Since the server
 * will not respond, repeated attempts will be made with increasing timeouts
 * and ping will appear to be unresponsive during this time (15 seconds by
 * default).
 */
#define RESOLVER_DOMAIN_SERVER  "10.16.64.11"      /* DNS server IP address */
#define RESOLVER_DOMAIN         "sj.broadcom.com"  /* Resolver domain */
#define DNS_DEBUG FALSE
#define RES_TIMEOUT_CFG        RES_TIMEOUT
#define RETRY_CFG              2
#define RES_OPTIONS_CFG        RES_DEFAULT
#define NSCOUNT_CFG            1


/* INCLUDE_BSD_SOCKET */
#undef  MAXSOCKBUF_CFG
#define MAXSOCKBUF_CFG (SB_MAX) ? (SB_MAX) : (256*1024)
#undef  SOCKBUF_WASTE_FACTOR_CFG
#define SOCKBUF_WASTE_FACTOR_CFG 8
#undef  SOMAXCONN_CFG
#define SOMAXCONN_CFG (SOMAXCONN) ? (SOMAXCONN) : 128
#undef  SO_LINGER_INFINITE_CFG
#define SO_LINGER_INFINITE_CFG FALSE

/* INCLUDE_SNS     || INCLUDE_SNS_MP
|| INCLUDE_SNS_RTP || INCLUDE_SNS_MP_RTP */
#undef  SNS_PATHNAME
#define SNS_PATHNAME  "/romfs/snsServer.vxe"
#undef  SNS_PRIORITY
#define SNS_PRIORITY  50
#undef  SNS_STACK_SIZE
#define SNS_STACK_SIZE  20000
#undef  SNS_LISTEN_BACKLOG
#define SNS_LISTEN_BACKLOG  5
#undef  SNS_DISTRIBUTED_SERVER_TYPE
#define SNS_DISTRIBUTED_SERVER_TYPE 70
#undef  SNS_DISTRIBUTED_SERVER_INSTANCE
#define SNS_DISTRIBUTED_SERVER_INSTANCE 1

/* INCLUDE_TIPC_CONFIG_STR */
#ifdef INCLUDE_TIPC_CONFIG_STR
#define TIPC_CONFIG_STR ""
#endif

/* INCLUDE_TIPC_MEDIA_SM */

#define SM_TIPC_PKT_Q_LEN   0
#define SM_TIPC_SM_PKT_SIZE 0
#define SM_TIPC_NUM_BUF     30

/* INCLUDE_TELNET */

#define TELNETD_MAX_CLIENTS 1
#define TELNETD_TASKFLAG FALSE
#define TELNETD_PORT 23
#define TELNETD_PARSER_HOOK shellParserControl

/* INCLUDE_MIP6_MN */

#define MIP_DEBUG           /* Enable debug */
#define MIP_SECURE          /* Enable IPSec */
#define MN_IFCONFIG_OPTIONS "mip0 inet6 3ffe::9 prefixlen 64 home"
#define IP6ATTACH_IFUNIT 2
#define IP6ATTACH_IFNAME "motfcc"
#define IFCONFIG_OPTIONS "motfcc2 inet6 up"
#define MN_DAEMON_TASK_OPTIONS "-d -a 8005::3  -i mip0"
#define MN_MOVEMENT_DETECTION_TASK_OPTIONS "-p 5 -i mip0  -d -D  motfcc2"


/* INCLUDE_TFTP_SERVER */

#define TFTP_SERVER_STACKSIZE 12000
#define TFTP_SERVER_DIRECTORYNAMES "/tftpboot"
#define TFTP_SERVER_NOCONTROL FALSE
#define TFTP_SERVER_MAXCONNECTIONS 10


/* INCLUDE_FTP6_SERVER */
#define FTPD6_OPTIONS_STRING ""
#define FTPD6_LOGINRTN (FUNCPTR)loginUserVerify
#define FTPD6_SSIZE 0

/* INCLUDE_FTPD6_GUEST_LOGIN */
#define FTPD6_GUEST_1             "anonymous"
#define FTPD6_GUEST_2             "anonymous"
#define FTPD6_GUEST_3             "anonymous"
#define FTPD6_GUEST_ROOT_DIR      "/anon"
#define FTPD6_GUEST_UPLOAD_DIR    "/anon"
#define FTPD6_GUEST_DIR_VALIDATE  FALSE

/* INCLUDE_FTP */
#define FTP_TRANSIENT_MAX_RETRY_COUNT   100
#define FTP_TRANSIENT_RETRY_INTERVAL    0
#define FTP_TRANSIENT_FATAL             ftpTransientFatal
#define FTP_DEBUG_OPTIONS               0
#define FTP_TIMEOUT                     0

/* INCLUDE_FTP6 */
#define FTP6_REPLYTIMEOUT 10

/* BOOTP client parameters */

#ifdef INCLUDE_BOOTP
#define BOOTP_MAX_HDRSIZE       14    /* Maximum (link-level) Header Size */
#define BOOTP_MAX_REQUESTS      3     /* BOOTP client transmit limit      */
#endif

/* INCLUDE_DHCPS */

#define DHCPS_CPORT 68
#define DHCPS_SPORT 67
#define DHCP_MAX_HOPS 4
#define DHCPS_MAX_MSGSIZE 590
#define DHCPS_ADDRESS_HOOK NULL
#define DHCPS_DEFAULT_LEASE 3600
#define DHCPS_LEASE_HOOK NULL
#define DHCPS_MAX_LEASE 3600


/* INCLUDE_DHCPR */

#define DHCPS_SPORT 67
#define DHCPS_CPORT 68
#define DHCP_MAX_HOPS 4
#define DHCPS_MAX_MSGSIZE 590


/* INCLUDE_DHCPC */

#define DHCPC_CPORT 68
#define DHCPC_DEFAULT_LEASE 3600
#define DHCPC_MAX_LEASES 4
#define DHCPC_MAX_MSGSIZE 590
#define DHCPC_MIN_LEASE 30
#define DHCPC_OFFER_TIMEOUT 5
#define DHCPC_SPORT 67
#define DHCPC_DISCOVER_RETRIES	4     /* Max. number of DISCOVER retries */

/* INCLUDE_DHCP6C */

#define DHCP6C_UPORT          547       /* Upstream port to the server */
#define	DHCP6C_DPORT          546       /* Downstream port to the client */
#define DHCP6C_OPTION         1         /* DHCP6C_OPT_SEND_IA_ID. See dhcp6.h */
#define	DHCP6C_VERBOSE        0         /* 0 - 2 */
#define DHCP6C_UPLINK_IF      "motfcc0" /* name of the link to DHCP6 server */
#define DHCP6C_DOWNLINK_IF    "motscc0" /* name of the link to downstream */
/* #define DHCP6C_RUNATBOOT      FALSE     whether to run the task after boot */
#define DHCP6C_PRIORITY       80        /* task priority */
#define DHCP6C_SIZE           10000     /* task stack size */

/*
 * The following KEY definitions are used by the DHCPv6 client to
 * send authentication parameters to the DHCPv6 server, when the client
 * uses authentication. Authentication is optional. For more information
 * see reference entry for dhcp6c() and dhcp6cAuthInfoSet().
 */

/*
 * DHCPv6 administrative domain name, e.g. a domain name "example.com",
 * or a string: "dhcpv6Admin"
 */

#define DHCP6C_KEY_REALM_STRING       "dhcpv6Admin"

/*
 * DHCPv6 secret shared between the client and the server. Secret must be
 * base 64 encoded. To encode a secret, search for "base64 encoder" in the
 * internet, or use tools such as openssl to encode a random string.
 */

#define DHCP6C_KEY_SECRET_STRING      "5pvW2g48OHPvkYMJSw0vZA=="

/*
 * DHCPv6 key expiration time in seconds. This is a quoted string.
 * If the key never expires, specify "forever" or "0".
 * To specify a finite time: "600", "3600"...
 */


#define DHCP6C_KEY_EXPIRE_STRING      "forever"

/* INCLUDE_DHCP6S */

#define DHCP6S_UPORT          547       /* Upstream port to server */
#define	DHCP6S_DPORT          546       /* Downstream port to client */
/*
 * RAM disk size in bytes for server configuration file: Must be
 * even multiple of the blocksize (512 bytes): e.g., 512*32
 */
  
#define DHCP6S_RAMDISK_SIZE   512*32
#define DHCP6S_CONF_FILE_NAME "dhcp6s.conf" /* name of the cfg file */
#define	DHCP6S_VERBOSE        0         /* 0 - 2 */
#define DHCP6S_DOWNLINK_IF    "motscc0" /* name of the link to downstream */
/* #define DHCP6S_RUNATBOOT      FALSE     whether to run the task after boot */
#define DHCP6S_PRIORITY       80        /* task priority */
#define DHCP6S_SIZE           10000     /* task stack size */

/*
 * DHCPv6 server authentication parameters are specified in the server's
 * configuration file.  For more information, see reference entry for
 * dhcp6s().
 */

/* INCLUDE_DHCP6R */

#define DHCP6R_HOP_LIMIT      32        /* hop limit */
#define DHCP6R_UPORT          547       /* Upstream port to the svr */
#define	DHCP6R_DPORT          546       /* Downstream port to client */
#define	DHCP6R_VERBOSE        0         /* 0 - 2 */
#define DHCP6R_UPLINK_IF      "motfcc0" /* name of the link to DHCP6 server */
#define DHCP6R_DOWNLINK_IF    "motscc0" /* name of the link to downstream */
#define DHCP6R_SRC_ADDR       NULL      /* source addr string for RA */
#define DHCP6R_SERVER_ADDR    NULL      /* server addr string */
/* #define DHCP6R_RUNATBOOT      FALSE     whether to run the task after boot */
#define DHCP6R_PRIORITY       80        /* task priority */
#define DHCP6R_SIZE           10000     /* task stack size */


/* SNTP parameters */

#define SNTP_PORT 123              /* Parameter for server and client */

/* INCLUDE_SNTPS */
#define SNTPS_MODE SNTP_ACTIVE     /* SNTP_ACTIVE or SNTP_PASSIVE */
#define SNTPS_DSTADDR NULL         /* If NULL, uses subnet local broadcast. */
#define SNTPS_INTERVAL 64          /* Broadcast interval, in seconds. */
#define SNTPS_TIME_HOOK NULL       /* Name of required clock access routine. */


/* INCLUDE_RTADV */
#define RTADV_COMMAND ""


/* INCLUDE_RTSOL */
#define RTSOL_COMMAND ""

/* INCLUDE_RIP */
#define RIP_EXPIRE_TIME 180
#define RIP_GARBAGE_TIME 300
#define RIP_GATEWAY FALSE
#define RIP_MULTICAST FALSE
#define RIP_SUPPLIER FALSE
#define RIP_SUPPLY_INTERVAL 30
#define RIP_TIMER_RATE 1
#define RIP_VERSION 1
#define RIP_AUTH_TYPE 1
#define RIP_IF_INIT TRUE
#define RIP_ROUTING_SOCKET_BUF_SIZE 8192

/* INCLUDE_RDISC */
#define RDISC_PRIORITY 	128
#define RDISC_OPTIONS 	0
#define RDISC_STACKSIZE 20000

/* INCLUDE_RIPNG */
#define RIPNG_OPTIONS_STRING ""
#define RIPNG_PRIORITY 0


/* INCLUDE_MIB2_IF */
#define MIB2IF_TRAP_RTN NULL
#define MIB2IF_TRAP_ARG NULL
#define MIB2IF_LONG_COUNTERS FALSE

/* INCLUDE_MIB2_SYSTEM */
#define MIB2SYS_DESCR		NULL
#define MIB2SYS_CONTACT		NULL
#define MIB2SYS_LOCATION	NULL

/*
 * MIB2SYS_OID_LEN is the number of elements in the object id.
 * MIB2SYS_OID is the object id.  The default is "0.0" which
 * has the length of 2.
 */

#define MIB2SYS_OID_LEN	        2
#define MIB2SYS_OID	        {0, 0}


/* INCLUDE_MIB2_IP */
#define MIB2IP_MAX_ROUTE_TBL_SIZE      0

/* INCLUDE_MIB2_IPV6 */
#define MIB2IPV6_MAX_ROUTE_TBL_SIZE   0


/* INCLUDE_NFS* */
#define NFS_MAXFILENAME     40
#define NFS_MAXPATH         255
#define NFS_USER_ID		2001		/* dummy nfs user id */
#define NFS_GROUP_ID		100		/* dummy nfs user group id */


/* INCLUDE_BPF */
#define BPF_BUFSIZE_CFG 4096

/* INCLUDE_FASTUDP */
#define FASTUDP_DEBUG_CFG 0

/* INCLUDE_FASTUDP6 */
#define FASTUDP6_DEBUG_CFG 0

/* 
 * INCLUDE_FASTPATH
 *
 * Define the FF_IPV4_INIT_RTN and FF_IPV6_INIT_RTN  macros to
 * your fastpath module init routine. If you don't want to initialize
 * the module, set it to -1. Set the FF_IPV4_FIB_DISP_TBL and
 * FF_IPV6_FIB_DISP_TBL macros to the dispatch table for the FIB you want
 * to use with the fastpath module. You must define a valid dispatch
 * table if you set the FF_IPVX_INIT_RTN macro to other than -1
 */

#define FF_IPV4_INIT_RTN ipFFInit
#define FF_IPV6_INIT_RTN ipFFInit 
#define FF_IPV4_FIB_DISP_TBL ptRibDispatchTable
#define FF_IPV6_FIB_DISP_TBL avlRibDispatchTable

/* For INCLUDE_APPL_LOG_UTIL */

#define APPL_LOG_FD_CFG         STD_OUT
#define APPL_LOG_LEVEL_CFG      (_LOG_DEC_DFLT | _LOG_LVL_DFLT)
#define APPL_KERN_LOG_FD_CFG    KERN_LOG_FD
#define APPL_KERN_LOG_LEVEL_CFG (_LOG_DEC_DFLT | _LOG_LVL_DFLT)

/* For the INCLUDE_NET_ROUTE_SHOW_SNPRINTF */

#define ROUTE_SHOW_MEM	4096


/******************************************************************************/
/*                                                                            */
/*                     WIND NET PPP CONFIGURATION                             */
/*                                                                            */
/******************************************************************************/

/*
 * WindNet PPP requires Remote Access Framework libraries.
 *
 * Each PPP connection is a PPP stack created within a framework object.
 * A single framework object can support multiple PPP stacks, as many as
 * available memory will allow. 
 * A framework creates 2 tasks: a control and a data task.
 * A framework allocates system memory for use by its memory manager. 
 * PPP stack componens are configured using framework profile objects.
 * Wind Net PPP creates the following framework objects for VDT releases:
 *
 *  a default framework, pppSysFramework
 *
 *  a default serial profile, pppSysProfile, used to configure basic connection
 *  parameters for vxWorks 5.x equivalent PPP API, usrWindNetPPPInit.
 *
 * The Project Facility WindNet PPP Demo configuration, also uses
 * pppSysFramework, but it creates its own separate profiles. 
 *
 * Listed below are definitions used by Wind NET PPP to configure
 * pppSysFramework and pppSysProfile for vxWorks 5x Equivalent PPP.
 * 
 * PPPoE and Multilink parameters (both available in Platform releases only),
 * are used to create old profiles provided by earlier releases, PPP 1.x. 
 * 
 * Note: Wind Net PPP requires the inclusion of component INCLUDE_DNS_RESOLVER. 
 */

/* pppSysFramework default name */

#define PPP_SYSTEM_DEFAULT_FRAMEWORK_NAME      "SYS_PPP"

/* pppSysFramework data task (handles user data) priority */

#define PPP_DATA_TASK_PRIORITY                 85

/* pppSysFramework control task (handles PPP protocol packets) priority */

#define PPP_CONTROL_TASK_PRIORITY              80

/* memory size in bytes for control task job queue */

#define PPP_CONTROL_JOB_QUEUE_SIZE             2048

/* memory size in bytes for data task job queue */

#define PPP_DATA_JOB_QUEUE_SIZE                2048

/* Memory partition pppSysFramework will allocate from */

#define PPP_MEM_PART_ID                        memSysPartId

/* Network pool ID (NET_POOL_ID) for PPP data and control packets */

#define PPP_NET_POOL_ID                        _pNetDpool                       

/* Number of serial ports */
 
#define PPP_NUM_SIO_CHANNELS                   NUM_TTY


/******************************************************************************/
/*                     usrWindNetPPPInit                                      */
/******************************************************************************/

/* seconds to wait for usrWindNetPPPInit to establish link */

#define PPP_CONNECT_DELAY          30      /* maximum delay in seconds */

/* seconds to wait for ppp5xDelete to disconnect link */

#define PPP_DISCONNECT_DELAY       30      /* maximum delay in seconds */

/* print application callback status messages */

#define PPP_APPL_CALLBACKS_VERBOSE TRUE

/*
 *
 * Server mode:
 *
 * If TRUE, local side as server can be configured to be authenticator,
 * to assign addresses, and to wait passively for connections.
 *
 * If FALSE, local side as client, initiates connections, can be configured
 * to authenticate to server, and to accept addresses from server.
 */

#define PPP_SERVER_MODE         FALSE              /* TRUE = server */

/*
 * Enable silent restart for server: connection is terminated when client
 * authentication fails. Re-enable the port after link goes down.
 */

#define PPP_SILENT_RESTART    TRUE 

/* Seconds to wait after link goes down to re-enable port */

#define PPP_SILENT_RESTART_INTRVL    10       


/******************************************************************************/
/*                    SERIAL DEVICE                                           */
/******************************************************************************/

/*
 * PPP_STR_PEER_TYPE: 
 *
 * For null modem connections:
 *
 * Enter "REGULAR" for non-Windows peer, e.g. LINUX, vxWorks, Solaris.
 * Enter "WIN_CLIENT" for WINDOWS DUN peer
 * Enter "WIN_SERVER" for WINDOWS RAS peer
 *
 * For modem connections (Using external modem connected to serial port)
 *
 * Enter "MODEM", for modem connections.
 *
 * Wind Net PPP provides demo code to support external modem connections
 * in client mode only (dial out).   
 */

#define PPP_STR_PEER_TYPE        "REGULAR"    /* default PPP peer */

/*
 * If peer is "MODEM" number to dial e.g., "4481" "9825092"
 */

#define PPP_CLNT_PHONE_NUMBER_TO_DIAL   NULL   /* client mode only */

/* modem init string,e.g., "AT&F1S0=0S7=90E1V1&H1&I0&R2" */

#define PPP_CLNT_MODEM_INIT_STRING      NULL   /* client mode only */

/* COM port to use: COM1 = 0; COM2 = 1 */

#define PPP_TTY                 1       /* default PPP serial channel */

#define PPP_BAUDRATE            19200   /* default PPP baud rate */

/*
 * Asynchronous control character map, e.g. "00000000L",
 * If NULL, map is "ffffffffL"
 */

#define PPP_STR_ASYNCMAP   NULL         /* e.g., "ffffffffL" */

/*
 * Serial adapter packet queue.  Must be at least large enough to hold
 * fragments for a maximum sized IP packet
 */

#define SIO_MAX_SEND_QUEUE_SIZE         50


/******************************************************************************/
/*                           IPCP                                             */
/******************************************************************************/

/*
 * In server mode, RAS can be configured to assign peer an IP address
 * or to accept what the peer proposes.
 * PPP_RAS_IPCP_ACCEPT_REMOTE is tested only If RAS address assignment
 * is not enabled. 
 */

/* default PPP Remote Access Server (RAS) mode configuration parameters */

/* primary DNS address RAS will offer clients */

#define PPP_RAS_IPCP_PRIMARY_DNS              "10.0.0.40"  /* NULL, if none */

/* secondary DNS address RAS will offer clients */

#define PPP_RAS_IPCP_SECONDARY_DNS            "10.0.0.50"  /* NULL, if none */
#define PPP_RAS_IPCP_ASSIGNS_ADDRESSES        TRUE         
#define PPP_RAS_IPCP_ACCEPT_REMOTE            FALSE 

/* server local address for all connections  */

#define PPP_RAS_IPCP_LOCAL_ADDRESS_SPECIFY    "10.0.0.9"

/* Start address for RAS IP address pool */

#define PPP_RAS_IPCP_POOL_START_ADDR          "10.0.0.10"

/* Size of RAS IPv4 address pool */

#define PPP_RAS_IPCP_POOL_SIZE            20

/* PPP client mode configuration parameters */

/* client accepts IP address proposed by peer */

#define PPP_CLNT_IPCP_ACCEPT_LOCAL   TRUE   

/* client accepts peer's IP address for itself (server) */

#define PPP_CLNT_IPCP_ACCEPT_REMOTE  TRUE  

/* set peer address as default route */
 
#define PPP_CLNT_SET_DEFAULTROUTE  TRUE

/* client should retrieve primary and secondary DNS */

#define PPP_CLNT_GET_DNS TRUE

/* Enable Van Jacobson TCP header Compression */

#define  PPP_USE_VJC_LOCAL            FALSE
#define  PPP_USE_VJC_REMOTE           FALSE

/*
 * Maximum number of slots is MAX_STATES (defined by default to be 16
 * in sl_compress.h
 */

#define  PPP_VJC_LOCAL_MAX_SLOTS      16    /* max VJ compr header slots */
#define  PPP_VJC_REMOTE_MAX_SLOTS     16    /* max VJ compr header slots */

#define PPP_IPCP_MAX_TERMINATE      2       /* max IPCP term-reqs */
#define PPP_IPCP_TERM_REQ_INTERVAL  3       /* seconds between conf reqs */
#define PPP_IPCP_MAX_CONFIGURE      10      /* max IPCP conf-reqs */
#define PPP_IPCP_CONF_REQ_INTERVAL  3       /* seconds between conf requests */



/******************************************************************************/
/*                         AUTHENTICATION                                     */
/******************************************************************************/


#define PPP_USER_NAME1            "vxTarget"       /* client default username */ 
#define PPP_PASSWORD1             "vxTarget"       /* client default password */
#define PPP_USER_NAME2            "vxPPPClient"    /* default username2 */
#define PPP_PASSWORD2             "vxPPPClient"    /* default password2 */

/* RAS allows clients to authenticate with CHAP */

#define PPP_RAS_USE_CHAP_AUTH             TRUE
#define PPP_RAS_CHAP_CHALLENGE_INTERVAL   180  /* in seconds */

/* RAS allows clients to authenticate with PAP */

#define PPP_RAS_USE_PAP_AUTH        TRUE

/* client can use CHAP to authenticate to server */

#define PPP_CLNT_USE_CHAP_AUTH      TRUE

/* client can use PAP to authenticate to server */

#define PPP_CLNT_USE_PAP_AUTH       TRUE

#define PPP_PAP_MAX_AUTHREQ         8    /* max PAP auth-reqs */

#define PPP_PAP_AUTH_REQ_INTERVAL   5    /* seconds */


/******************************************************************************/
/*                              LCP                                           */
/******************************************************************************/

/*
 * Magic Number range 
 * start,end,step: e.g., "1,2147483647,0"
 * start should always be larger than 0
 * step of 0 means any number within the range is acceptable
 */

#define PPP_MAGIC_NUMBER_LOCAL_RANGE  "1,2147483647,0"
#define PPP_MAGIC_NUMBER_REMOTE_RANGE "1,2147483647,0"

/* Negotiate magic number for local side */

#define PPP_USE_LOCAL_MAGIC_NUMBER    FALSE

/* Enable Address and Control Field Compression */

#define  PPP_USE_ACFC    TRUE

/* Enable Protocol Field Compression */

#define  PPP_USE_PFC     TRUE

/* Echoes: if echoes fail, link is terminated */

#define PPP_LCP_SEND_ECHOES           TRUE
#define PPP_LCP_ECHO_INTERVAL         180  /* in seconds */
#define PPP_LCP_ECHO_MAX_RETRIES      5    /* maximum retries */

/*
 * SILENT mode: if TRUE, responds to configure requests,
 *              but does not initiate them
 *
 * PASSIVE_MODE for serial links does not need to be configured as it is
 * always enabled for either client or server mode: If no response to
 * configure requests is received, the local side waits in STOPPED state. 
 */

#define PPP_SILENT_MODE            FALSE  

#define PPP_MTU                     1500    /* Maximum Transmission Unit */
#define PPP_MRU                     1500    /* Maximum Receive Unit */
#define PPP_MIN_MTU                 1280    /* IPv6: 1280; IPv4: 576 */
#define PPP_MIN_MRU                 1280    /* IPv6: 1280; IPv4: 576 */
#define PPP_LCP_MAX_TERMINATE       2       /* max LCP term-reqs */
#define PPP_LCP_MAX_CONFIGURE       10      /* max LCP conf-reqs */
#define PPP_LCP_MAX_FAILURE         5       /* max conf-naks for LCP */
#define PPP_LCP_TERM_REQ_INTERVAL   3       /* seconds between term requests */
#define PPP_LCP_CONF_REQ_INTERVAL   3       /* seconds between conf requests */



/*
 * PROXY ARP:
 *
 * There is no longer a need to create ARP entries as PPP_OPT_PROXYARP
 * did for vxWorks 5.x PPP. In this release, PROXY ARP is implemented
 * by the network stack: To enable PROXY for all Ethernet network
 * interfaces, make ARP_PROXYALL_CFG TRUE.  To enable proxy only
 * for one or more Ethernet interfaces, see documentation for
 * ifProxyArpEnable().  It is also necessary to turn on IP
 * forwarding on the target doing the proxying, i.e., make network
 * stack parameter IPFORWARDING_CFG TRUE.
 * Adding a default route to the target doing the proxying and
 * enabling the server to provide the client DNS addressees will allow
 * the peer access outside the subnet.
 */

/*
 * To see debug messages, compile target/src/ppp directory with
 * ADDED_CFLAGS+="-DPPP_DEBUG", e.g.:
 *    target/src/ppp>make CPU=PPPC32 TOOL=diab rclean
 *    target/src/ppp>make CPU=PPPC32 TOOL=diab ADDED_CFLAGS+="-DPPP_DEBUG"  
 */


/******************************************************************************/
/*                                                                            */
/*       WIND NET PPP over Ethernet CONFIGURATION (PLATFORM RELEASES)         */
/*                                                                            */
/******************************************************************************/


/* Maximum number of services supported in AC (server) mode */

#define PPPOE_MAX_SERVICES       5

/* PPPOE_AC_MODE: 0x1 (server) PPPOE_HOST_MODE: 0x2 (client) */

#define PPPOE_OPERATING_MODE       0x2     /* server/client mode */
#define PPPOE_DEV_NAME             NULL    /* device name: e.g., "fei" */     
#define PPPOE_DEV_UNIT             NULL    /* unit number: e.g., "1" */ 
#define PPPOE_SERVICE_NAME         NULL    /* e.g., "silver" */ 
#define PPPOE_MTU                  1492    /* Maximum Transmission Unit */
#define PPPOE_MRU                  1492    /* Maximum Receive Unit */
#define WIN2K_PPPOE_MRU_WORKAROUND TRUE    /* Assume 1492 as maximum MRU/MTU */
                                           /* if peer won't negotiate MRU */
#define PPPOE_MIN_MTU              1280    /* IPv6: 1280; IPv4: 576 */
#define PPPOE_MIN_MRU              1280    /* IPv6: 1280; IPv4: 576 */



/******************************************************************************/
/*                                                                            */
/*       WIND NET MULTILINK PPP CONFIGURATION (PLATFORM RELEASES)             */
/*                                                                            */
/******************************************************************************/

/*
 * Multilink PPP creates a receive and a data task 
 */

#define MP_SEND_QUEUE_TASK_PRIORITY     80    /* MP send task priority */ 
#define MP_SEND_QUEUE_SIZE              2048  /* job queue size in bytes */

#define MP_RECEIVE_QUEUE_TASK_PRIORITY  85    /* MP receive task priority */
#define MP_RECEIVE_QUEUE_SIZE           2048  /* job queue size in bytes */

/* Local Max Receive Reconstruction Unit */

#define MLPPP_MRRU                      2000   

/* Enable Short Seq. no in mlppp header */

#define MLPPP_SHORT_SEQNO               TRUE  

/* Local End Doint Discriminator */

#define MLPPP_EID                       "1-WIND" 

/* If TRUE, enable bundle authentication */

#define MLPPP_BUNDLE_AUTH               FALSE  /* TRUE = enable */ 

/*
 * Static or dynamic bundle addition. If TRUE, all ports are added
 * statically to the bundle. Otherwise, all ports are added
 * dynamically to the bundle if multilink is negotiated with peer.
 */

#define MLPPP_STATIC_BUNDLE_ADDITION    TRUE


/******************************************************************************/
/*            WIND NET PPP CONFIGURATION PARAMETERS END                       */
/******************************************************************************/

/* PCI device configuration type constants */

#define PCI_CFG_FORCE 0
#define PCI_CFG_AUTO  1
#define PCI_CFG_NONE  2

/* for backward compatibility with old 1.0 BSPs */

#ifndef BSP_VERSION
#   define BSP_VERSION	"1.0"	/* old 1.0 style BSP */
#   define BSP_VER_1_0	TRUE
#endif

#ifndef BSP_REV
#   define BSP_REV	"/0"	/* old 1.0 style BSP */
#endif

#else /* VX_VERSION != 62 */

#define INCLUDE_CACHE_SUPPORT	/* include cache support package */
#define INCLUDE_CONSTANT_RDY_Q	/* constant insert time ready queue */
#define INCLUDE_ENV_VARS	/* unix compatable environment variables */
#define INCLUDE_EXC_HANDLING	/* include basic exception handling */
#define INCLUDE_EXC_TASK	/* miscelaneous support task */
#define INCLUDE_FLOATING_POINT	/* floating point I/O */
#define INCLUDE_FORMATTED_IO	/* formatted I/O */
#define INCLUDE_GCC_FP		/* gcc floating point support libraries */
#define INCLUDE_IO_SYSTEM	/* include I/O system */
#define INCLUDE_LOGGING		/* logTask logging facility */
#define INCLUDE_MEM_MGR_FULL	/* full featured memory manager */
#define INCLUDE_MSG_Q		/* include message queues */
#define INCLUDE_NETWORK         /* network subsystem code */
#define INCLUDE_PIPES		/* pipe driver */
#define INCLUDE_SELECT		/* select() facility */
#define INCLUDE_SEM_BINARY	/* include binary semaphores */
#define INCLUDE_SEM_MUTEX	/* include mutex semaphores */
#define INCLUDE_SEM_COUNTING	/* include counting semaphores */
#define INCLUDE_SIGNALS		/* software signal library */
#define INCLUDE_STDIO		/* standard I/O */
#define INCLUDE_TASK_HOOKS	/* include kernel callouts */
#define INCLUDE_TASK_VARS	/* task variable package */
#define INCLUDE_TIMEX		/* timexLib for exec timing */
#define INCLUDE_TTY_DEV		/* attach serial drivers */
#define INCLUDE_WATCHDOGS	/* include watchdogs */
#define INCLUDE_ANSI_ALL        /* includes complete ANSI C library functions */
#define INCLUDE_ANSI_ASSERT	/* ANSI-C assert library functionality */
#define INCLUDE_ANSI_CTYPE	/* ANSI-C ctype library functionality */
#define INCLUDE_ANSI_LOCALE	/* ANSI-C locale library functionality */
#define INCLUDE_ANSI_MATH	/* ANSI-C math library functionality */
#define INCLUDE_ANSI_STDIO	/* ANSI-C stdio library functionality */
#define INCLUDE_ANSI_STDLIB	/* ANSI-C stdlib library functionality */
#define INCLUDE_ANSI_STRING	/* ANSI-C string library functionality */
#define INCLUDE_ANSI_TIME	/* ANSI-C time library functionality */

/* CPU-SPECIFIC INCLUDED SOFTWARE FACILITIES */

/* include support for possibly existing floating point coprocessor */

#if	(CPU==MC68020 || CPU==MC68040 || CPU==MC68060 || CPU==CPU32)
#define INCLUDE_MC68881         /* MC68881/2 (68040) floating pt coprocessor */
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif	/* CPU==MC68020 || CPU==MC68040 || CPU==MC68060 || CPU==CPU32 */

#if	(CPU_FAMILY == SPARC) && defined(INCLUDE_FLOATING_POINT)
#define INCLUDE_SPARC_FPU       /* SPARC Floating-Point Unit */
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif	/* CPU_FAMILY == SPARC */

#if	(CPU_FAMILY==MIPS)
#define INCLUDE_R3010           /* R3010 float point co-processor */
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif	/* CPU_FAMILY==MIPS */

#if	((CPU_FAMILY==PPC) && (CPU!=PPC403) && (CPU!=PPC860))
#define INCLUDE_PPC_FPU         /* PowerPC floating point unit */
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif	/* (CPU_FAMILY==PPC) && (CPU!=PPC403) */

#if	(CPU==I960KB)
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif  /* I960KB */

#if     (CPU_FAMILY==I80X86)
#define INCLUDE_I80387          /* I80387 float point co-processor */
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* CPU_FAMILY==I80X86 */


/* define appropriate object module format for a given architecture */

#if	((CPU_FAMILY==MIPS) || (CPU_FAMILY==PPC) || (CPU_FAMILY==SIMSPARCSOLARIS))
#define INCLUDE_ELF             /* ELF object modules */
#else
#if	((CPU_FAMILY==I960) || (CPU_FAMILY==AM29XXX))
#define INCLUDE_COFF            /* COFF object modules */
#else
#if	(CPU_FAMILY==SIMHPPA)
#define	INCLUDE_SOM_COFF
#else	/* default */
#define INCLUDE_AOUT            /* a.out object modules */
#endif
#endif
#endif


/******************************************************************************/
/*                                                                            */
/*                          EXCLUDED FACILITIES                               */
/*                                                                            */
/******************************************************************************/

#define INCLUDE_DOSFS           /* dosFs file system */
#define INCLUDE_FLASH		/* BMW flash driver */
#define INCLUDE_FLASH_BOOT	/* BMW flash driver in boot ROM */

#define INCLUDE_WDB		/* WDB debug agent */

#if FALSE
#define INCLUDE_CODETEST	/* CodeTEST target utilities library */
#define INCLUDE_CPLUS		/* include C++ support */
#define INCLUDE_CPLUS_MIN	/* include minimal C++ support */
#define INCLUDE_CPLUS_IOSTREAMS	/* include iostreams classes */
#define INCLUDE_CPLUS_VXW	/* include VxWorks wrapper classes */
#define INCLUDE_CPLUS_TOOLS	/* include Tools class library */
#define INCLUDE_CPLUS_HEAP	/* include Heap class library */
#define INCLUDE_CPLUS_BOOCH	/* include Booch Components library */
#define INCLUDE_CONFIGURATION_5_2 /* pre-tornado tools */
#define INCLUDE_DEBUG           /* pre-tornado debugging */
#define INCLUDE_DEMO		/* include simple demo instead of shell */
#define INCLUDE_INSTRUMENTATION /* windView instrumentation */
#define INCLUDE_JAVA		/* Java virtual machine */
#define INCLUDE_JAVA_RTX	/* use RtX X libs for Java graphics */
#define INCLUDE_JAVA_VXWINDOWS	/* use VxWindows X libs for Java graphics */
#define INCLUDE_LOADER          /* object module loading */

/* INCLUDE_MMU_BASIC is defined by many bsp's in config.h. If INCLUDE_MMU_FULL
 * is defined in configAll.h and INCLUDE_MMU_BASIC is defined in config.h,
 * then INCLUDE_MMU_FULL will take precedence. 
 */

#define INCLUDE_MMU_BASIC 	/* bundled mmu support */
#define INCLUDE_MMU_FULL	/* unbundled mmu support */

#define INCLUDE_SYM_TBL_SYNC	/* synchronize host and target symbol tables */

#define INCLUDE_POSIX_AIO       /* POSIX async I/O support */
#define INCLUDE_POSIX_AIO_SYSDRV/* POSIX async I/O system driver */
#define INCLUDE_POSIX_FTRUNC	/* POSIX ftruncate routine */
#define INCLUDE_POSIX_MEM	/* POSIX memory locking */
#define INCLUDE_POSIX_MQ        /* POSIX message queue support */
#define INCLUDE_POSIX_SCHED	/* POSIX scheduling */
#define INCLUDE_POSIX_SEM 	/* POSIX semaphores */
#define INCLUDE_POSIX_SIGNALS 	/* POSIX queued signals */
#define INCLUDE_POSIX_TIMERS 	/* POSIX timers */
#define INCLUDE_PROTECT_TEXT	/* text segment write protection (unbundled) */
#define INCLUDE_PROTECT_VEC_TABLE /* vector table write protection (unbundled)*/
#define INCLUDE_RAMDRV          /* ram disk driver */
#define INCLUDE_RAWFS           /* rawFs file system */
#define INCLUDE_RT11FS		/* rt11Fs file system */
#define INCLUDE_SECURITY	/* shell security for network access */
#define INCLUDE_SHELL           /* interactive c-expression interpreter */
#define INCLUDE_SHOW_ROUTINES   /* show routines for system facilities*/
#define INCLUDE_SM_OBJ          /* shared memory objects (unbundled) */
#define INCLUDE_SPY             /* spyLib for task monitoring */
#define INCLUDE_STANDALONE_SYM_TBL /* compiled-in symbol table */
#define INCLUDE_STARTUP_SCRIPT  /* execute start-up script */
#define INCLUDE_STAT_SYM_TBL    /* create user-readable error status */
#define INCLUDE_SW_FP		/* software floating point emulation */
#define INCLUDE_SYM_TBL         /* symbol table package */
#define INCLUDE_UNLOADER        /* object module unloading */
#define INCLUDE_WILLOWS_RT      /* include Willows RT library */
#define INCLUDE_WINDVIEW	/* WindView command server */


#define INCLUDE_ANSI_5_0	/* include only version 5.0 ANSI support */
#define INCLUDE_BP_5_0		/* version 5.0 backplane driver */
#define INCLUDE_DELETE_5_0	/* define delete() function as in VxWorks 5.0 */

#endif	/* FALSE */

/******************************************************************************/
/*                                                                            */
/*                  KERNEL SOFTWARE CONFIGURATION                             */
/*                                                                            */
/******************************************************************************/

#define USER_I_CACHE_MODE	CACHE_WRITETHROUGH  /* default mode */
#define USER_D_CACHE_MODE	CACHE_WRITETHROUGH  /* default mode */
#define USER_I_CACHE_ENABLE		    	    /* undef to leave disabled*/
#define USER_D_CACHE_ENABLE			    /* undef to leave disabled*/
#define USER_B_CACHE_ENABLE			    /* undef to leave disabled*/
#define USER_I_MMU_ENABLE			    /* undef to leave disabled*/
#define USER_D_MMU_ENABLE			    /* undef to leave disabled*/

#define SYM_TBL_HASH_SIZE_LOG2	8	/* 256 entry hash table symbol table */
#define STAT_TBL_HASH_SIZE_LOG2	6	/* 64 entry hash table for status */
#define MQ_HASH_SIZE		0	/* POSIX message queue hash table size 
					 * 0 = default */
#define NUM_SIGNAL_QUEUES	16	/* POSIX queued signal count */

#define FREE_RAM_ADRS		(end)	/* start right after bss of VxWorks */


/* I/O system parameters */

#define NUM_DRIVERS		20	/* max 20 drivers in drvTable */
#define NUM_FILES		50	/* max 50 files open simultaneously */
#define NUM_DOSFS_FILES		20	/* max 20 dosFs files open */
#define NUM_RAWFS_FILES		5	/* max 5  rawFs files open */
#define NUM_RT11FS_FILES	5	/* max 5  rt11Fs files open */
#define MAX_LOG_MSGS		50      /* max 50 log msgs */

#define	NUM_TTY			2	/* number of tty channels */
#define	CONSOLE_TTY		0	/* console channel */

#define CONSOLE_BAUD_RATE	9600	/* console baud rate */

#ifdef	INCLUDE_SLIP
#define	SLIP_TTY		1	/* serial line IP channel */
#endif	/* INCLUDE_SLIP */

#define MAX_LIO_CALLS		0	/* max outstanding lio calls 0=default */
#define MAX_AIO_SYS_TASKS	0	/* max aio system tasks, 0 = default */
#define AIO_TASK_PRIORITY	0	/* aio system tasks prior, 0 = default */
#define AIO_TASK_STACK_SIZE	0	/* aio tasks stack size, 0 = default */


/* kernel and system task parameters by architecture */

#if	CPU_FAMILY==MC680X0
#define INT_LOCK_LEVEL  	0x7	/* 68k interrupt disable mask */
#define ROOT_STACK_SIZE		10000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	10000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x1000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		1000	/* size of ISR stack, in bytes */
#define TRAP_DEBUG		2	/* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif	/* CPU_FAMILY==MC680X0 */

#if	CPU_FAMILY==SPARC
#define INT_LOCK_LEVEL  	15	/* SPARC interrupt disable level */
#define ROOT_STACK_SIZE		10000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	50000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		10000	/* size of ISR stack, in bytes */
#define VEC_BASE                (LOCAL_MEM_LOCAL_ADRS + 0x1000)
#define VEC_BASE_ADRS           ((char *) VEC_BASE)
#endif	/* CPU_FAMILY==SPARC */

#if     CPU_FAMILY==SIMSPARCSUNOS  || CPU_FAMILY==SIMHPPA || CPU_FAMILY==SIMSPARCSOLARIS
#define INT_LOCK_LEVEL          0x1     /* interrupt disable mask */
#define ROOT_STACK_SIZE         20000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        50000   /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          50000   /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           0       /* dummy */
#endif  /* CPU_FAMILY==SIMSPARCSUNOS  || CPU_FAMILY==SIMHPPA || CPU_FAMILY==SIMSPARCSOLARIS */

#if	CPU_FAMILY==I960
#define INT_LOCK_LEVEL  	0x1f	/* i960 interrupt disable mask */
#define ROOT_STACK_SIZE		20000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	40000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		1000	/* size of ISR stack, in bytes */
#define TRAP_DEBUG		0	/* n/a for the 80960 */
#define VEC_BASE_ADRS           NONE	/* base register not reconfigurable */
#endif	/* CPU_FAMILY==I960 */

#if	CPU_FAMILY==MIPS
#define INT_LOCK_LEVEL          0x1     /* R3K interrupt disable mask */
#define ROOT_STACK_SIZE         (20000) /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        (20000) /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	(0x2000)/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          (5000)  /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           ((char *) 0x0)  /* meaningless in R3k land */
#define VME_VECTORED            FALSE   /* use vectored VME interrupts */
#define TRAP_DEBUG              0       /* trap 0 - breakpoint trap */
#endif	/* CPU_FAMILY==MIPS */

#if	CPU_FAMILY==PPC
#define INT_LOCK_LEVEL          0x0     /* not used */
#define ROOT_STACK_SIZE         (20000) /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        (20000) /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          (5000)  /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           ((char *) 0x0)
#endif	/* CPU_FAMILY==PPC */

#if     CPU_FAMILY==I80X86
#define INT_LOCK_LEVEL          0x0     /* 80x86 interrupt disable mask */
#define ROOT_STACK_SIZE         10000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        10000   /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x1000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          1000    /* size of ISR stack, in bytes */
#define TRAP_DEBUG              0       /* not used */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif  /* CPU_FAMILY==I80X86 */

#if	CPU_FAMILY==AM29XXX
#define INT_LOCK_LEVEL  	0x0001	/* 29k all interrupts disable mask */
#define ROOT_STACK_SIZE		10000	/* size of root's stack, in bytes */
#define SHELL_STACK_SIZE	40000	/* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE		10000	/* size of ISR stack, in bytes */
#define TRAP_DEBUG		15	/* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif	/* CPU_FAMILY==AM29XXX */

/* WDB debug agent configuration */

#ifdef  INCLUDE_WDB

/* optional agent facilities */

#define INCLUDE_WDB_BANNER		/* print banner after agent starts */
#define INCLUDE_WDB_VIO			/* virtual I/O support */
#define	INCLUDE_WDB_TTY_TEST		/* test serial line communcation */

/* core agent facilities - do not remove */

#define INCLUDE_WDB_CTXT		/* context control */
#define INCLUDE_WDB_FUNC_CALL		/* spawn function as separate task */
#define INCLUDE_WDB_DIRECT_CALL		/* call function in agents context */
#define INCLUDE_WDB_EVENTS		/* host async event notification */
#define INCLUDE_WDB_GOPHER		/* gopher info gathering */
#define INCLUDE_WDB_BP			/* breakpoint support */
#define INCLUDE_WDB_EXC_NOTIFY		/* notify host of exceptions */
#define INCLUDE_WDB_EXIT_NOTIFY		/* notify the host of task exit */
#define INCLUDE_WDB_MEM			/* optional memory services */
#define INCLUDE_WDB_REG			/* get/set hardware registers */

/* agent mode */

#define WDB_MODE        WDB_MODE_DUAL	/* WDB_MODE_[DUAL|TASK|EXTERN] */

/* agent communication paths */

#define WDB_COMM_NETWORK 	0	/* vxWorks network	- task mode */
#define WDB_COMM_SERIAL		1	/* raw serial		- bimodal   */
#define WDB_COMM_TYCODRV_5_2	2	/* older serial driver	- task mode */
#define WDB_COMM_ULIP		3	/* vxSim packet device	- bimodal   */
#define WDB_COMM_NETROM		4	/* netrom packet device	- bimodal   */
#define WDB_COMM_CUSTOM		5	/* custom packet device	- bimodal   */
#define WDB_COMM_END		6	/* END packet device 	- bimodal */

/* communication path configuration */

#ifndef WDB_COMM_TYPE
#define WDB_COMM_TYPE WDB_COMM_END 	/* default path is the end device */
#endif

#ifndef WDB_TTY_CHANNEL
#define WDB_TTY_CHANNEL		1	/* default Sio SERIAL channel */
#endif

#ifndef WDB_TTY_DEV_NAME
#define WDB_TTY_DEV_NAME    "/tyCo/1"	/* default TYCODRV_5_2 device name */
#endif

#ifndef WDB_TTY_BAUD
#define WDB_TTY_BAUD		9600	/* default baud rate */
#endif

#ifndef WDB_ULIP_DEV
#define WDB_ULIP_DEV    "/dev/ulip14"	/* default ULIP packet device */
#endif

#define	WDB_NETROM_TYPE		400	/* default is old 400 series */
#define	WDB_NETROM_WIDTH	1	/* width of a ROM word. This macro
					 * is not used for the 500 series. */
#define WDB_NETROM_INDEX	0	/* index into word of pod zero */
#define	WDB_NETROM_NUM_ACCESS	1	/* of pod zero per byte read */
#define	WDB_NETROM_ROMSIZE	ROM_SIZE /* size of a single ROM. If your
					 * board has N ROMs, this definition
					 * must be divided by N to be right */
#define WDB_NETROM_POLL_DELAY	2	/* # clock ticks to poll for input */

/* miscelaneous agent constants */

#define WDB_MTU         	1500	/* max RPC message size */
#define WDB_POOL_SIZE 		((sysMemTop() - FREE_RAM_ADRS)/16) /* memory pool for host tools */
#define WDB_SPAWN_STACK_SIZE	0x5000	/* default stack size of spawned task */

#endif  /* INCLUDE_WDB */

/* WindView event task and buffer parameters */

#define EVT_STACK_SIZE		7000
#define WV_EVT_STACK		EVT_STACK_SIZE
#define EVT_PRIORITY		0
#define WV_EVT_PRIORITY		EVT_PRIORITY
#define EVTBUFFER_SIZE		20000
#define EVTBUFFER_ADDRESS	(char *)NULL

/* WindView command server task parameters */

#define WV_SERVER_STACK		10000
#define WV_SERVER_PRIORITY	100

/* WindView event task */


/* WindView event collection mode 
 *
 * The WindView collection mode can be one of the following,
 *	CONTINUOUS_MODE		- continuous collection and display of events 
 *	POST_MORTEM_MODE	- collection of events only
 */

#define WV_MODE			CONTINUOUS_MODE

/* Java component changes - begin */

/* Note: these macros have no effect if INCLUDE_JAVA is not defined */
#define INCLUDE_JAVA_AGENT		/* Java debug agent for -debug */
#define INCLUDE_JAVA_AWT		/* Motif java.awt peer class */
#define INCLUDE_JAVA_JPEG		/* JPEG image decoding */
#define INCLUDE_JAVA_MATH		/* java.math.BigInteger math support */
#define INCLUDE_JAVA_MMEDIA		/* Audio support */
#define INCLUDE_JAVA_NET		/* java.net support */
#define INCLUDE_JAVA_SYSRESOURCE	/* system resource protocol handler */
#define INCLUDE_JAVA_ZIP		/* java.util.zip support */
#define INCLUDE_JAVA_USER		/* include user native code */

/* Java component changes - end */

/******************************************************************************/
/*                                                                            */
/*                   "GENERIC" BOARD CONFIGURATION                            */
/*                                                                            */
/******************************************************************************/

/* device controller I/O addresses when included */

#define IO_ADRS_EI      ((char *) 0x000fff00)   /* 32A,32D i82596CA Ethernet */
#define IO_ADRS_EX	((char *) 0x00ff0000)	/* 24A,32D Excelan Ethernet */
#define IO_ADRS_ENP	((char *) 0x00de0000)	/* 24A,32D CMC Ethernet */
#define IO_ADRS_EGL	((char *) 0x00004000)	/* 16A,16D Interphase Enet */

#define IO_AM_EX	VME_AM_STD_SUP_DATA	/* Excelan address modifier */
#define IO_AM_EX_MASTER	VME_AM_STD_SUP_DATA	/* Excellan AM for DMA access */
#define IO_AM_ENP	VME_AM_STD_SUP_DATA	/* CMC address modifier */

/* device controller interrupt vectors when included */

#define INT_VEC_ENP		192	/* CMC Ethernet controller*/
#define INT_VEC_EX		193	/* Excelan Ethernet controller*/
#define INT_VEC_EGL		200	/* Interphase Ethernet controller*/

/* device controller interrupt levels when included */

#define INT_LVL_EGL		5	/* Interphase Ethernet controller */
#define INT_LVL_EX		2	/* Excelan Ethernet controller */
#define INT_LVL_ENP		3	/* CMC Ethernet controller */


/******************************************************************************/
/*                                                                            */
/*                   "MISCELLANEOUS" CONSTANTS                                */
/*                                                                            */
/******************************************************************************/

/* shared memory objects parameters (unbundled) */

#define SM_OBJ_MAX_TASK		40	/* max # of tasks using smObj */
#define SM_OBJ_MAX_SEM		60	/* max # of shared semaphores */
#define SM_OBJ_MAX_MSG_Q	10	/* max # of shared message queues */
#define SM_OBJ_MAX_MEM_PART	4	/* max # of shared memory partitions */
#define SM_OBJ_MAX_NAME		100	/* max # of shared objects names */
#define SM_OBJ_MAX_TRIES	100	/* max # of tries to obtain lock */

/* shared memory network parameters  - defaults to values DEFAULT_PKTS_SIZE
 * and DEFAULT_CPUS_MAX in smPktLib.h respectively
 */

#define SM_PKTS_SIZE            0       /* shared memory packet size */
#define SM_CPUS_MAX             0       /* max # of cpus for shared network */

/* low memory layout */

#if     (CPU_FAMILY == I80X86)
#define GDT_BASE_OFFSET         0x800
#define SM_ANCHOR_OFFSET        0x1100
#define BOOT_LINE_OFFSET        0x1200
#define EXC_MSG_OFFSET          0x1300
#elif   (CPU_FAMILY == PPC)
#define SM_ANCHOR_OFFSET        0x4100
#define BOOT_LINE_OFFSET        0x4200
#define EXC_MSG_OFFSET          0x4300
#else
#define SM_ANCHOR_OFFSET        0x600
#define BOOT_LINE_OFFSET        0x700
#define EXC_MSG_OFFSET          0x800
#endif  /* (CPU_FAMILY == I80X86) */

/* The backplane driver onboard anchor at the following address */

#define SM_ANCHOR_ADRS	((char *) (LOCAL_MEM_LOCAL_ADRS+SM_ANCHOR_OFFSET))


/* The bootroms put the boot line at the following address */

#define BOOT_LINE_ADRS	((char *) (LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET))
#define	BOOT_LINE_SIZE	255	/* use 255 bytes for bootline */

/* The boot line is stored in non-volatile RAM at the following offset */

#define	NV_BOOT_OFFSET	0	/* store the boot line at start of NVRAM */


/* Messages from exceptions during exceptions go at the following address */

#define EXC_MSG_ADRS	((char *) (LOCAL_MEM_LOCAL_ADRS+EXC_MSG_OFFSET))


/* Backplane H/W support */

#define	SM_TAS_TYPE	SM_TAS_HARD	/* hardware supports test-and-set */


/* Resident ROMs constants */

#if     (CPU_FAMILY==I960)
#define STACK_SAVE      512     	/* maximum stack used to preserve */
#else	/* sparc or others */
#if	((CPU_FAMILY==SPARC) || (CPU_FAMILY==PPC))
#define	STACK_SAVE	0x1000
#else	/* all other architecutes */
#define STACK_SAVE      0x40    	/* maximum stack used to preserve */
#endif					/* mips cpp no elif */
#endif

#if     (CPU_FAMILY==SPARC)
#define RESERVED        0x2000		/* vector table base plus table size */
#else	/* 68000 or others */
#if	(CPU==MC68000)
#define RESERVED	0x400		/* avoid zeroing MC68302 vector table */
#else	
#if	(CPU_FAMILY == PPC)
#define RESERVED	0x4400		/* avoid zeroing EXC_MSG */
#else	/* all other architectures */
#define RESERVED        0
#endif
#endif					/* mips cpp no elif */
#endif

#if     ((CPU_FAMILY == MIPS) || (CPU_FAMILY == PPC))
#define	STACK_RESIDENT	RAM_DST_ADRS
#else
#define	STACK_RESIDENT	_sdata
#endif

#if	(_STACK_DIR == _STACK_GROWS_DOWN)

#ifdef	ROM_RESIDENT
#define STACK_ADRS	STACK_RESIDENT
#else
#define STACK_ADRS	_romInit
#endif	/* ROM_RESIDENT */

#else	/* _STACK_DIR == _STACK_GROWS_UP */

#ifdef	ROM_RESIDENT
#define STACK_ADRS	(STACK_RESIDENT-STACK_SAVE)
#else
#define STACK_ADRS	(_romInit-STACK_SAVE)
#endif	/*  ROM_RESIDENT */

#endif	/* _STACK_DIR == _STACK_GROWS_UP */


/* Default Boot Parameters */

#define HOST_NAME_DEFAULT	"igor"	        /* host name */
#define TARGET_NAME_DEFAULT	"kahlua"	/* target name (tn) */
#define HOST_USER_DEFAULT	"tornado"	/* user (u) */
#define HOST_PASSWORD_DEFAULT	"tornado+"	/* password */
#define SCRIPT_DEFAULT		""	 	/* startup script (s) */
#define OTHER_DEFAULT		"" 		/* other (o) */


/* Login security initial user name and password.
 * Use vxencrypt on host to find encrypted password.
 * Default password provided here is "password".
 */

#ifdef	INCLUDE_SECURITY
#define LOGIN_USER_NAME		"target"
#define LOGIN_PASSWORD		"bReb99RRed"	/* "password" */
#endif	/* INCLUDE_SECURITY */


/* install environment variable task create/delete hooks */

#ifdef  INCLUDE_ENV_VARS
#define	ENV_VAR_USE_HOOKS	TRUE
#endif	/* INCLUDE_ENV_VARS */

/*
 * default page size for MMU is 8k.
 * 68040 will also work with 4k page size
 * powerPC is only 8k
 */

#if	(CPU_FAMILY == PPC)
#define VM_PAGE_SIZE		4096
#else 	/* (CPU_FAMILY != PPC) */
#define VM_PAGE_SIZE		8192
#endif	/* (CPU_FAMILY != PPC) */

/* vxWorks Network Configuration */
#ifdef INCLUDE_NETWORK
/******************************************************************************/
/*                                                                            */
/*                         INCLUDED NETWORK FACILITIES                        */
/*                                                                            */
/******************************************************************************/
#define INCLUDE_BSD_SOCKET	/* include the BSD socket library */
#define INCLUDE_ICMP		/* include icmp code */
#define INCLUDE_IGMP		/* include igmp code */
#define INCLUDE_UDP		/* include udp code */
#define INCLUDE_TCP		/* include tcp code */
#define INCLUDE_NET_INIT        /* network subsystem initialization */
#define INCLUDE_NET_REM_IO	/* network remote file i/o driver */

#if 0
#define INCLUDE_BOOTP		/* bootp */
#define INCLUDE_PROXY_CLIENT	/* proxy arp client (Slave Board) */
#define INCLUDE_TFTP_CLIENT	/* tftp client */
#endif
#define INCLUDE_POSIX_ALL       /* include all available POSIX
				   functions */


/* INCLUDED NETWORK DRIVERS */
#if CPU_FAMILY==MIPS
#define INCLUDE_EGL             /* include Interphase Ethernet interface */
#else
#define INCLUDE_EX		/* include Excelan Ethernet interface */
#endif	/* CPU_FAMILY==MIPS */

#define INCLUDE_NFS             /* nfs package */
#define INCLUDE_FTP_SERVER	/* ftp server */
#define INCLUDE_NET_SHOW        /* network info and status facilities */

/******************************************************************************/
/*                                                                            */
/*                         EXCLUDED NETWORK FACILITIES                        */
/*                                                                            */
/******************************************************************************/
#if FALSE
#define INCLUDE_DHCPC           /* DHCP client */
#define INCLUDE_DHCPS           /* DHCP server */
#define INCLUDE_DHCPR           /* DHCP relay agent */
#define INCLUDE_DNS_DEBUG       /* DNS resolver debug mode */
#define INCLUDE_DNS_RESOLVER	/* DNS resolver */
#define INCLUDE_IP_FILTER	/* IP filter library */
#define INCLUDE_FTP_SERVER	/* ftp server */
#define INCLUDE_FTPD_SECURITY   /* password security for ftp sessions */
#define INCLUDE_HTTP            /* http server (Wind Web Server) */
#define INCLUDE_MCAST_ROUTING	/* include multicast routing in the code */
#define INCLUDE_MIB2_ALL        /* All of MIB 2 */
#define INCLUDE_MIB2_SYSTEM	/* the system group */
#define INCLUDE_MIB2_TCP        /* the TCP group */
#define INCLUDE_MIB2_ICMP	/* the ICMP group */
#define INCLUDE_MIB2_UDP        /* the UDP group */
#define INCLUDE_MIB2_IF		/* the interfaces group */
#define INCLUDE_MIB2_AT         /* the AT group */
#define INCLUDE_MIB2_IP		/* the IP group */
#define INCLUDE_NET_SHOW        /* network info and status facilities */
#define INCLUDE_NET_SYM_TBL     /* load symbol table from network */
#define INCLUDE_NFS             /* nfs package */
#define INCLUDE_NFS_SERVER      /* nfs server */
#define INCLUDE_NFS_MOUNT_ALL	/* automatically mount all NFS file systems */
#define INCLUDE_OSPF            /* open shortest path first routing protocol */
#define INCLUDE_PING		/* ping() utility */
#define INCLUDE_POSIX_ALL       /* include all available POSIX functions */
#define INCLUDE_PPP		/* include Point-to-Point Procotol */
#define INCLUDE_PROXY_DEFAULT_ADDR /* Use ethernet addr to generate bp addrs */
#define INCLUDE_PROXY_SERVER	/* proxy arp server (Master Board) */
#define INCLUDE_RDB             /* remote debugging package */
#define INCLUDE_RIP             /* Routing Information Protocol RIP */
#define INCLUDE_RLOGIN          /* remote login */
#define INCLUDE_ROUTE_SOCK	/* include routing socket interface */
#define INCLUDE_RPC		/* rpc package */
#define INCLUDE_SLIP		/* include serial line interface */
#define INCLUDE_SNMPD 	        /* SNMP Agent */
#define INCLUDE_SNMPD_DEBUG	/* SNMP Agent debugging */
#define INCLUDE_SNTPC           /* SNTP client */
#define INCLUDE_SNTPS           /* SNTP server */
#define	INCLUDE_TCP_DEBUG	/* TCP debug facility */
#define INCLUDE_TELNET          /* telnet-style remote login */
#define INCLUDE_TFTP_SERVER	/* tftp server */
#define	INCLUDE_ZBUF_SOCK	/* zbuf socket interface */

#define INCLUDE_STREAMS_ALL	   /* Complete Streams subsystem */
#define INCLUDE_STREAMS		   /* CORE Streams (stream head) */
#define INCLUDE_STREAMS_SOCKET	   /* Streams Socket layer and socket library */
#define INCLUDE_STREAMS_TLI	   /* Streams TLI layer and TLI library */
#define INCLUDE_STREAMS_AUTOPUSH   /* SVR4 autopush mechanism */
#define INCLUDE_STREAMS_DLPI	   /* Streams DLPI to mbuf compatible driver */
#define INCLUDE_STREAMS_DEBUG      /* Streams Debugging facility */
#define INCLUDE_STREAMS_STRACE	   /* Streams trace utility */
#define INCLUDE_STREAMS_STRERR	   /* Streams error log utility */

#define DEFAULT_STREAMS_SOCKET	   /* default to Streams socket for AF_INET */
/* network drivers */
#define INCLUDE_LN		/* include AMD LANCE interface */
#define INCLUDE_LNSGI		/* include AMD LANCE interface for SGI VIP10 */
#define INCLUDE_MED		/* include Matrix network interface*/
#define INCLUDE_NIC		/* include National NIC interface */
#endif	/* FALSE */

#endif	/* INCLUDE_NETWORK */

#ifdef INCLUDE_BSD_SOCKET
    /* 
     * By default, set the sockets API for compatibility with BSD 4.3 
     * applications.
     */
#define BSD43_COMPATIBLE
#endif

#ifdef INCLUDE_NFS
/* Default NFS parameters - constants may be changed here, variables
 * may be changed in usrConfig.c at the point where NFS is included.
 */

#define NFS_USER_ID		2001		/* dummy nfs user id */
#define NFS_GROUP_ID		100		/* dummy nfs user group id */
#endif /* INCLUDE_NFS */

/* STREAMS Configuration Parameters */

/* the following define the STREAMS default configuration parameters */
 
#define STREAMS_MEM_MAX		102400	/* max STREAMS memory usage */
#define STREAMS_MEM_PART_ADDR	0x0	/* mem partition address, NULL=alloc */
#define STREAMS_MEM_PART_SIZE	0x0	/* mem partition size, NULL=max */
#define STREAMS_PUSH_MAX	9	/* max pushable modules */
#define STREAMS_MSGSZ_MAX	4096	/* max data message size */
#define STREAMS_CTLSZ_MAX	1024	/* max control message size */

#define	STREAMS_PROTO_INIT_RTN	NULL	/* streams protocol init routine */
 
/* The "/tmp" directory for storing log files will work only if the
 * communication protocol between the host and target is "ftp" or "rsh" .
 * If "nfs" is being used then the directory has to be set to the 
 * directory exported by the host system.
 */

#define STREAMS_STRERR_OUTPUT_DIR  "/tmp"   /* Directory for strerr utility */
#define STREAMS_STRACE_OUTPUT_DIR  "/tmp"   /* Directory for strace file */

/* SNMP configuration parameters */
/* MIB-2 Variable defaults - see RFC 1213 for complete description */
 
#define MIB2_SYS_DESCR                  "VxWorks SNMPv1/v2c Agent"
#define MIB2_SYS_CONTACT                "Wind River Systems"
#define MIB2_SYS_LOCATION               "Planet Earth"

/* MIB2_SYS__OBJID_LEN is the number of elements in the object id
 * MIB_2_SYS_OBJID is the object id.  The default is "0.0" which
 * has the length of 2
 */
 
#define MIB2_SYS_OBJID_LEN              2
#define MIB2_SYS_OBJID                  { 0, 0 }

#define SNMP_TRACE_LEVEL            0    /* Must be >= 0 and <= 3 with higher */
                                         /* values giving more info and 0     */
                                         /* giving no info                    */ 




#define PPP_TTY				1	/* default PPP serial channel */
#define PPP_OPTIONS_STRUCT			/* use PPP options macros */
#define	PPP_OPTIONS_FILE		NULL	/* use PPP options file */
#define	PPP_CONNECT_DELAY		15	/* delay to establish link */

#undef	INCLUDE_PPP_CRYPT			/* include PPP crypt support */

/* PPP options flags - set to 1 to turn on option */

#define	PPP_OPT_NO_ALL			0	/* Don't allow any options */
#define	PPP_OPT_PASSIVE_MODE		0	/* Set passive mode */
#define	PPP_OPT_SILENT_MODE		0	/* Set silent mode */
#define	PPP_OPT_DEFAULTROUTE		0	/* Add default route */
#define	PPP_OPT_PROXYARP		0	/* Add proxy ARP entry */
#define	PPP_OPT_IPCP_ACCEPT_LOCAL	0	/* Acpt peer's IP addr for us */
#define	PPP_OPT_IPCP_ACCEPT_REMOTE	0	/* Acpt peer's IP addr for it */
#define	PPP_OPT_NO_IP			0	/* Disable IP addr negot. */
#define	PPP_OPT_NO_ACC			0	/* Disable addr/control compr */
#define	PPP_OPT_NO_PC			0	/* Disable proto field compr */
#define	PPP_OPT_NO_VJ			0	/* Disable VJ compression */
#define	PPP_OPT_NO_VJCCOMP		0	/* Disable VJ conct-ID compr */
#define	PPP_OPT_NO_ASYNCMAP		0	/* Disable async map negot. */
#define	PPP_OPT_NO_MN			0	/* Disable magic num negot. */
#define	PPP_OPT_NO_MRU			0	/* Disable MRU negotiation */
#define	PPP_OPT_NO_PAP			0	/* Don't allow PAP auth */
#define	PPP_OPT_NO_CHAP			0	/* Don't allow CHAP auth */
#define	PPP_OPT_REQUIRE_PAP		0	/* Require PAP auth */
#define	PPP_OPT_REQUIRE_CHAP		0	/* Require CHAP auth */
#define	PPP_OPT_LOGIN			0	/* Use login dbase for PAP */
#define	PPP_OPT_DEBUG			0	/* Enable daemon debug mode */
#define	PPP_OPT_DRIVER_DEBUG		0	/* Enable driver debug mode */

/* PPP options strings - set to desired string to turn on option */

#define	PPP_STR_ASYNCMAP		NULL	/* desired async map */
#define	PPP_STR_ESCAPE_CHARS		NULL	/* chars to escape on xmits */
#define	PPP_STR_VJ_MAX_SLOTS		NULL	/* max VJ compr header slots */
#define	PPP_STR_NETMASK			NULL	/* netmask value */
#define	PPP_STR_MRU			NULL	/* MRU value for negotiation */
#define	PPP_STR_MTU			NULL	/* MTU value for negotiation */
#define	PPP_STR_LCP_ECHO_FAILURE	NULL	/* max LCP echo failures */
#define	PPP_STR_LCP_ECHO_INTERVAL	NULL	/* time for LCP echo requests */
#define	PPP_STR_LCP_RESTART		NULL	/* timeout for LCP */
#define	PPP_STR_LCP_MAX_TERMINATE	NULL	/* max LCP term-reqs */
#define	PPP_STR_LCP_MAX_CONFIGURE	NULL	/* max LCP conf-reqs */
#define	PPP_STR_LCP_MAX_FAILURE		NULL	/* max conf-naks for LCP */
#define	PPP_STR_IPCP_RESTART		NULL	/* timeout for IPCP */
#define	PPP_STR_IPCP_MAX_TERMINATE	NULL	/* max IPCP term-reqs */
#define	PPP_STR_IPCP_MAX_CONFIGURE	NULL	/* max IPCP conf-reqs */
#define	PPP_STR_IPCP_MAX_FAILURE	NULL	/* max # conf-naks for IPCP */
#define	PPP_STR_LOCAL_AUTH_NAME		NULL	/* local name for auth */
#define	PPP_STR_REMOTE_AUTH_NAME	NULL	/* remote name for auth */
#define	PPP_STR_PAP_FILE		NULL	/* PAP secrets file */
#define	PPP_STR_PAP_USER_NAME		NULL	/* username for PAP peer auth */
#define	PPP_STR_PAP_PASSWD		NULL	/* password for PAP peer auth */
#define	PPP_STR_PAP_RESTART		NULL	/* timeout for PAP */
#define	PPP_STR_PAP_MAX_AUTHREQ		NULL	/* max PAP auth-reqs */
#define	PPP_STR_CHAP_FILE		NULL	/* CHAP secrets file */
#define	PPP_STR_CHAP_RESTART		NULL	/* timeout for CHAP */
#define	PPP_STR_CHAP_INTERVAL		NULL	/* CHAP rechallenge interval */
#define	PPP_STR_CHAP_MAX_CHALLENGE	NULL	/* max CHAP challenges */

/* DHCP client parameters */

#ifdef INCLUDE_DHCPC
#define DHCPC_SPORT 		67    /* Port monitored by DHCP servers. */
#define DHCPC_CPORT 		68    /* Port monitored by DHCP clients. */
#define DHCPC_MAX_LEASES 	4     /* Max. number of simultaneous leases */
#define DHCPC_OFFER_TIMEOUT 	5     /* Seconds to wait for multiple offers */
#define DHCPC_DEFAULT_LEASE 	3600  /* Desired lease length in seconds */
#define DHCPC_MIN_LEASE 	30    /* Minimum allowable lease length */
#endif

/* DHCP server parameters */

#ifdef INCLUDE_DHCPS
#define DHCPS_LEASE_HOOK 	NULL   /* Name of required storage routine */
#define DHCPS_ADDRESS_HOOK 	NULL   /* Name of optional storage routine */
#define DHCPS_DEFAULT_LEASE 	3600   /* default lease length (secs). */
#define DHCPS_MAX_LEASE 	3600   /* default value of max lease (secs). */
#endif

/* DHCP server and relay agent parameters */

#if defined (INCLUDE_DHCPS) || defined (INCLUDE_DHCPR)
#define DHCP_MAX_HOPS           4       /* Hops before discard, up to 16. */
#define DHCPS_SPORT 		67 	/* Port monitored by DHCP servers */
#define DHCPS_CPORT 		68 	/* Port monitored by DHCP clients */
#endif

/* SNTP parameters */

#if defined (INCLUDE_SNTPC) || defined (INCLUDE_SNTPS)
#define SNTP_PORT 123

#ifdef INCLUDE_SNTPS
#define SNTPS_MODE SNTP_ACTIVE     /* SNTP_ACTIVE or SNTP_PASSIVE */
#define SNTPS_DSTADDR NULL         /* If NULL, uses subnet local broadcast. */
#define SNTPS_INTERVAL 64          /* Broadcast interval, in seconds. */
#define SNTPS_TIME_HOOK NULL       /* Name of required clock access routine. */
#endif    /* SNTP server. */
#endif /* SNTP server or client. */

/* DNS resolver parameters; You must change these defaults to your config. */
#if 0/* Disable for performance and to prevent hostAdd() errors. */
#define INCLUDE_DNS_DEBUG
#define INCLUDE_DNS_RESOLVER
#ifdef INCLUDE_DNS_RESOLVER    
#define RESOLVER_DOMAIN_SERVER  "192.168.1.1"    /* DNS server IP address */
#define RESOLVER_DOMAIN         "mavericknetworks.com"  /* Resolver domain */
#endif /* INCLUDE_DNS_RESOLVER */
#endif 

/* for backward compatibility with old 1.0 BSPs */
#ifndef BSP_VERSION
#   define BSP_VERSION	"1.0"	/* old 1.0 style BSP */
#   define BSP_VER_1_0	TRUE
#endif

#ifndef BSP_REV
#   define BSP_REV	"/0"	/* old 1.0 style BSP */
#endif

/* add for vxWorks 5.5 */
#ifdef INCLUDE_BSD_SOCKET
#define SOMAXCONN_CFG 5
#endif

/* add for vxWorks 5.5 */
#if defined (INCLUDE_NFS_SERVER) || defined (INCLUDE_NFS)
#define NFS_MAXPATH             255             /* max. file path length */
#endif
#endif /* VX_VERSION == 62 */
                                
#endif	/* INCconfigAllh */
