/* configAll.h - default configuration header */

/*
 * Copyright (c) 1994-2012 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
modification history
--------------------
30y,10feb12,ljg  Adding BOOT_USB_POST_MS_BULKONLY_INIT_DELAY for IA USB nvram
                 support (WIND00322792)
19c,21jul11,zly  fix for WIND00201989:xbd priority inversion issue. 
                 Change default priority from 50 to 200. 
19b,21jul11,sye  Add direct BIO support for storage drivers. (WIND00217466)
19a,20jul11,zly  WIND00217466: add a direct mode for xbd.
18z,09sep10,ggz  Replace LOGIN_PASSWORD with sha256 hashed password
18y,23oct09,s_s  Added kprintf defines to support BSP build (WIND00186733)
18x,21sep09,v_r  Added shell session limitation system (def CQ:WIND00155318,
                 merged from 6.5 sustaining).
18w,15sep09,l_z  Remove unused macro INCLUDE_PPC_FPU, INCLUDE_I80387, 
                 INCLUDE_R3010, INCLUDE_SPARC_FPU and INCLUDE_MC68881. 
                 (WIND00153695)
18v,14aug09,jpb  Defect# 161788. Removed define of INCLUDE_SET.
18u,14aug09,hps  update dosfs cache name
18t,06aug09,hbh  Updated MIPC bus name.
18s,16jul09,jmp  Added INCLUDE_CORE_DUMP_RTP_FS component and
		 CORE_DUMP_RTP_FS_PATH parameter.
18r,15jun09,elp  modified default WDB MIPC bus
18s,07jun09,mdo  Make VFP the default for ARMARCH7
18r,28may09,mdo  Make vector FP the default for ARM
18q,05may09,rgo  Use SM_BASE_ADRS to build SM_ANCHOR_ADRS(WIND145934)
18p,24apr09,rlp  Removed private task option value (CQ:WIND00163553).
18o,24feb09,rlp  Removed unsupported INCLUDE_WDB_COMM_NETROM component.
18n,11nov08,rbc  WIND00143219 - Fix EXC_MSG_OFFSET for IA BSP's.
18m,24oct08,pgh  Add defines for USB boot.
18l,23oct08,zl   added VXEVENTS_OPTIONS configuration parameter.
18k,23sep08,jmp  Added the max number of tasks that can be spied
                 (CQ:WIND00106559).
18j,11sep08,v_r  Added tip library support (req CQ:WIND00114857).
18i,03sep08,dbt  Changed default MIPC bus name to plb_0.
18h,29aug08,zl   Added TASK_PRIORITY_SET_COMPATIBLE.
18g,25aug08,jpb  Renamed _WRS_VX_SMP to _WRS_CONFIG_SMP.
18f,29jul08,dbt  Added WDB & Proxy MIPC support.
		 Added new WDB Proxy parameters
18e,16jul08,jmp  Added INCLUDE_CORE_DUMP_RTP_COMPRESS_ZLIB and
                 INCLUDE_CORE_DUMP_RTP_COMPRESS_RLE.
18d,03jul08,rbc  Move the EXC_MSG_OFFSET for x86.
18c,23jun08,pad  Added RTP_OVERLAPPED_ADDRESS_SPACE macro definition.
18b,09jun08,pad  Added configuration macros RTP_CODE_REGION_START and
                    RTP_CODE_REGION_SIZE.
18a,22may08,jmp  Removed INCLUDE_CORE_DUMP_RAW_DEV: Deprecated.
                 Removed INCLUDE_CORE_DUMP_STREAM_DEV: No longer required for
		 core dump stream devices.
		 Removed INCLUDE_CORE_DUMP_UTIL & INCLUDE_CORE_DUMP_MEM_POOL:
		 Hidden components.
		 Added INCLUDE_CORE_DUMP_RTP (REQ CQ:WIND00005075).
17z,12mar08,dlk  Rename INCLUDE_SPINLOCK_FAST -> INCLUDE_SPINLOCK_ISR_ND.
17y,22apr08,dbt  Added WDB dynamic printf support.
17x,12dec07,dlk  Added INCLUDE_SPINLOCK_FAST.
17w,01nov07,kk   disable Power Management for PPC SMP for now (CQ:104713)
17v,16oct07,mfj  Fixing problem with WDB_POOL_SIZE (WIND00102855)
17u,09oct07,mze  fix typo
17t,03oct07,mze  fix for 106388 WDB_MODE can not be set to dual for AMP BSPs
17s,28sep07,kab  WIND00104310: Fix undefined MMU_PAGE_SIZE for MIPS
17r,06sep07,pmr  add VXIPI for AMP
17q,22aug07,act  io footprint work: add info on smaller IO configurations
17p,29aug07,pch  CQ102930: undef INCLUDE_TASK_VARS in SMP build
17o,27aug07,agf  udpate dosFs default params
17n,23aug07,jmg  Added VX_ENABLE_CPU_TIMEOUT.
17n,15aug07,mmi  extend power management support to PPC and SIMULATORS
17m,26jul07,v_r  Added the new shell history management components (req
		 CQ:WIND00005098).
17l,06jul07,tor  vxIpiLib dependency of SMP
17k,18jan07,dcc  fixed defect WIND0070704:"xbdBlkDevCreate creates a task with
		 hardcoded priority."
17h,01jun06,jlk  Added HRFS_DEFAULT_COMMIT_POLICY and HRFS_DEFAULT_COMMIT_PERIOD
16t,04apr07,zl   added INCLUDE_OBJ_INFO and INCLUDE_TASK_LIST.
16s,23mar07,h_k  added VX_DSP_TASK to WDB_SPAWN_OPTS.
17h,24feb07,jmt  Fix problem with ATA support in bootrom
17g,05feb07,gls  added SEM_RW_MAX_CONCURRENT_READERS
17f,10jan07,jmg  Added VX_GLOBAL_NO_STACK_FILL (Req. WIND00083667)
17e,21feb07,jmt  Remove INCLUDE_WDB from bootrom builds
17d,23jan07,jmt  Disable unneeded components for bootApp build
17c,06dec06,kch  Added kernel sysctl components to the if FALSE section
		 for reference.
17b,13dec06,gls  removed undef of INCLUDE_JOB_TASK
17a,07dec06,rlp  Modified the maximun number of WDB system mode breakpoint.
16z,22nov06,kk   temporarily exclude INCLUDE_JOB_TASK for SMP
16y,18nov06,pcs  Add ENABLE_ALL_CPUS for SMP.
16x,15nov06,dbt  Added support for INCLUDE_TLS component.
16w,15nov06,jmt  Defect 81462: Add BOOT_DISABLE_MMU_BEFORE_ENTRY default
16v,13nov06,jgm  Added INCLUDE_POSIX_TIMER_SHOW as part of defect WIND00039293
16u,30oct06,h_k  re-increased STACK_SAVE for SH, which was over written by a
		 merge. (CQ:70758)
16t,22sep06,mze  moved SM_MAX_PKTS here from smPktLib.h WIND00058841
16s,22sep06,slk  add INCLUDE_VXMEMPROBE_INIT
16r,24aug06,gls  changed value of MAX_ISR_JOBS (Defect #00059580)
16q,17jul06,ggz  Added INCLUDE_WVTIPC for TIPC WV instrumentation
16p,17jul06,mwv  updating DOSFS and HRFS default cache sizes (WIND00059268).
16p,15dec06,jmp  added new core dump components and parameters.
16o,12jul06,jon  adding ND6_MAX_PREFIXES_CFG UDP_STRICT_MCAST_MSHIP_CFG
16n,12jul06,bwa  added VxMP task-deferred ISR parameters.
16m,03jul06,kch  Moved coreip stack components and defines that are not
		 supported by IPNet to a new configAllCoreip.h. Added default
		 IPNET stack default build support for command line.
16l,23jun06,dlk  Remove networking components not applicable with IPNET stack.
		 Make INCLUDE_IPCOM a default.
16k,22jun06,jmp  added INCLUDE_CORE_DUMP_UTIL component.
		 added INCLUDE_CORE_DUMP_STREAM_DEV component.
16j,19jul06,kch  Include configAllNetwork.h unconditionally. Also moved
		 SNS component's defines to configNetParams.h.
16i,16jun06,zl   added INCLUDE_MEM_MGR_INFO, MEM_PART_DEFAULT_OPTIONS.
16h,12jun06,dlk  Added INCLUDE_NULLBUFPOOL.
16g,05jul06,jln  added thread SPORADIC server support
16r,04jul06,tcr  add INCLUDE_POSIX_TRACE
16e,12may06,dlk  Added UDP(6)_STRICT_MCAST_MSHIP_CFG (WIND00038260).
16d,21apr06,pcm  added INCLUDE_POSIX_ADVISORY_FILE_LOCKING
16c,26mar06,kch  Added ALTQ_USE_MACH_CLOCK to allow user to override the
		 machine dependent clock init and read routines (SPR#119511).
16b,15mar06,ebh  Change TIPC data buffer default for BSP builds
16a,09mar06,kch  Renamed ALTQ configuration parameters. Also removed unused
		 ALTQ_MACHCLK_USEPCC configuration parameter. (SPR#118834).
15z,02mar06,dlk  Remove INCLUDE_IPFLOW (SPR #104107)
15y,13feb06,rec  move coldfire offsets
15x,21feb06,mze  Revert max TIPC cluster size back to 65536 after problems found
15w,20feb06,mze  removing TIPC_DATA_00032, increase max TIPC cluster to 66076
15v,14feb06,kk   added INCLUDE_VX_TRADITIONAL_SCHEDULER
15u,13feb06,bwa  removed references to vxfusion.
15t,12feb06,jln  added INCLUDE_POSIX_THREAD_CPUTIME
15s,10feb06,jlk  Added IOS_POSIX_PSE52_MODE definition for BSP builds.
15r,09feb06,gls  added POSIX_SIGNAL_MODE
15q,03feb06,pcm  added INCLUDE_XBD_PART_LIB
15p,20jan06,pad  Added RTP_FD_NUM_MAX definition for BSP builds.
15o,13jan06,rlg  test spr 104154
15n,20dec05,pcm  added CDROM_COM_BUF_SIZE and various NFS cache parameters
15m,08dec05,jmp  added INCLUDE_CORE_DUMP_COMPRESS_RLE component.
		 added CORE_DUMP_CKSUM_ENABLE parameter.
15l,07dec05,pcm  added NFS3_CLIENT_CACHE_DEFAULT_NUM_LINES and
		 NFS3_CLIENT_CACHE_DEFAULT_LINE_SIZE
15k,24nov05,jmp  added CORE_DUMP_SKIP_USER_RESERVED_MEM parameter.
16m,17feb06,tlu  Add macro to stop WB from taking MIP6 values from configAll.h
16k,16feb06,xli  change the PPP_CLNT_SET_DEFAULTROUTE to FALSE
15l,05feb06,kch  Added ALTQ support.
15k,20dec05,dlk  Add INCLUDE_DNS_RESOLVER_DEBUG.
15j,28oct05,tlu  Added INCLUDE_MIP6_HA and configuration params for MIP6 HA
15i,26oct05,zl   added POSIX shared memory objects and memory mapped files.
15h,20jul05,rec  define SW_MMU_ENABLE TRUE for coldfire
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
12o,21apr04,cjj  Added WIND_JOBS_MAX.  Cleaned up history version numbers
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
11m,17mar04,bpn  Modified SHELL_DEFAULT_CONFIG parameter (SPR#94900).
11l,26feb04,yvp  Set SYSCALL_HOOK_TBL_SIZE default value to 8 (SPR #93621).
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
06q,28aug96,gnn  added WDB_COMM_END define.
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
06i,22apr96,kkk  fixed typo on CPU_FAMILY for PPC (SPR# 6385)
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
06d,10oct95,dat  backward compatible BSP_VERSION and BSP_REV
06c,28sep95,dat  new #define INCLUDE_WDB_ANNOUNCE
06b,21sep95,ms   switched some WDB macros between here and usrWdb.c
06a,27jun95,ms   renamed WDB_COMM_XXX macros
05z,21jun95,ms   added INCLUDE_WDB_TTY_TEST and INCLUDE_WDB_EXIT_NOTIFY
05y,21jun95,tpr  added #define INCLUDE_WDB_MEM.
05x,13jun95,srh  Updated C++ support triggers.
05w,07jun95,ms   WDB_STACK_SIZE is now CPU dependant.
05v,07jun95,p_m  added INCLUDE_FORMATTED_IO. suppressed spy from default
		 configuration.
05u,01jun95,ms   all WDB macros start with WDB_ (usrWdb.c modhist has details).
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
05g,04nov94,kdl  initial merge cleanup.
05b,16jun94,caf  defined INCLUDE_ELF for MIPS, updated copyright notice.
04x,20jul94,ms   changed INCLUDE_HPPA_OUT to INCLUDE_SOM_COFF
05d,11apr94,jag  Removed conditions for definition of  NFS_GROUP and NFS_USER
		 ID for SNMP demo support.
05c,18feb94,elh  (SNMP VERSION) moved INCLUDE_POSIX_ALL to false section.
		 Added support for the mib2 library.
05f,25may94,kdl  (POSIX VERSION) removed erroneous NFS def's.
05e,23mar94,smb  (POSIX VERSION) removed PASSIVE_MODE
05d,15mar94,smb  renamed tEvtTask parameters.
05c,15feb94,smb  added define of WV_MODE
05b,12jan94,kdl  (POSIX VERSION) turned off instrumentation, added
		 INCLUDE_POSIX_ALL; added INCLUDE_POSIX_SIGNALS; changed
		 INCLUDE_POSIX_MEM_MAN to INCLUDE_POSIX_MEM; added
		 NUM_SIGNAL_QUEUES.
*/

/*
DESCRIPTION
This header contains the parameters that define the default
configuration for VxWorks.
*/

#ifndef INCconfigAllh
#define INCconfigAllh

#include <vsbConfig.h>
#include <smLib.h>
#include <vme.h>
#include <iv.h>

/******************************************************************************/
/*                                                                            */
/*                      INCLUDED SOFTWARE FACILITIES                          */
/*                                                                            */
/******************************************************************************/

#define INCLUDE_BASE_KERNEL     /* base kernel: support for obj ownership */
#define INCLUDE_CTORS_DTORS
#define INCLUDE_CACHE_SUPPORT   /* include cache support package */
#define INCLUDE_CONSTANT_RDY_Q  /* constant insert time ready queue */
#define INCLUDE_EDR_STUB	/* ED&R error-injection stub */
#define INCLUDE_ENV_VARS	/* unix compatible environment variables */
#define INCLUDE_EXC_HANDLING	/* include basic exception handling */
#define INCLUDE_EXC_SHOW	/* include exception show routines */
#define INCLUDE_EXC_TASK	/* miscellaneous support task */
#define INCLUDE_JOB_TASK	/* task level job facility task */
#define INCLUDE_FLOATING_POINT	/* floating point I/O */
#define INCLUDE_FORMATTED_OUT_BASIC /* Basic formatted output routines */
#define INCLUDE_FORMATTED_IO	/* formatted I/O */
#define INCLUDE_HOOKS           /* allow adding/removing of hook functions */
#define	INCLUDE_IO_BASIC	/* basic I/O system */
#define	INCLUDE_IO_REMOVABLE	/* I/O device removability (eg USB unplug) */
#define	INCLUDE_IO_FILE_SYSTEM	/* I/O filesystem (uses REMOVABLE) */
#define	INCLUDE_IO_RTP		/* I/O from RTPs */
#define	INCLUDE_IO_POSIX	/* I/O POSIX features */
#define	INCLUDE_IO_MISC		/* I/O: obsolete features */
#define INCLUDE_IO_SYSTEM	/* = MISC+FILE_SYSTEM, plus RTP if RTPs on */
#define INCLUDE_LOGGING		/* logTask logging facility */
#define INCLUDE_MEM_MGR_FULL	/* full featured memory manager */
#define INCLUDE_MEM_MGR_INFO	/* memory manager info routines */
#define INCLUDE_OBJ_LIB         /* include object management */
#define INCLUDE_MSG_Q           /* include message queues */
#define INCLUDE_MSG_Q_CREATE_DELETE  /* message queue creation & deletion */
#define INCLUDE_NETWORK         /* network subsystem code */
#define INCLUDE_PIPES		/* pipe driver */
#define INCLUDE_POOL		/* pool allocator */
#define INCLUDE_REBOOT_HOOKS    /* support for hook callouts before reboots */
#define INCLUDE_SELECT		/* select() facility */
#define INCLUDE_VX_TRADITIONAL_SCHEDULER /* VxWorks traditional scheduler */
#define INCLUDE_SEM_LIB         /* generic support for all semaphore types */
#define INCLUDE_SEM_DELETE      /* support for semaphore deletion */
#define INCLUDE_SEM_BINARY	/* include binary semaphores */
#define INCLUDE_SEM_MUTEX	/* include mutex semaphores */
#define INCLUDE_SEM_COUNTING	/* include counting semaphores */
#define INCLUDE_SEM_BINARY_CREATE     /* binary semaphore creation */
#define INCLUDE_SEM_COUNTING_CREATE   /* counting semaphore creation */
#define INCLUDE_SEM_MUTEX_CREATE      /* mutex semaphore creation */
#define INCLUDE_SIGNALS		/* software signal library */
#define INCLUDE_STDIO		/* standard I/O */
#define INCLUDE_TASK_CREATE_DELETE /* dynamic creation & deletion of tasks */
#define INCLUDE_TASK_HOOKS	/* include kernel callouts */
#define INCLUDE_TASK_RESTART    /* Ability to restart & reset tasks */
#ifdef	_WRS_CONFIG_SMP
#undef	INCLUDE_TASK_VARS	/* task variables not supported in SMP */
#else	/* _WRS_CONFIG_SMP */
#define INCLUDE_TASK_VARS	/* task variable package */
#endif	/* _WRS_CONFIG_SMP */
#define INCLUDE_TIMEX		/* timexLib for exec timing */
#define INCLUDE_TYLIB		/* serial driver support lib */
#define INCLUDE_TTY_DEV		/* attach serial drivers */
#define INCLUDE_VXMEMPROBE_INIT	/* vxMemProbe initializer */
#define INCLUDE_WATCHDOGS	/* include watchdogs */
#define INCLUDE_WATCHDOGS_CREATE_DELETE   /* watchdog creation & deletion */
#define INCLUDE_WDB		/* WDB debug agent */
#define INCLUDE_ANSI_ALL        /* includes complete ANSI C library functions */
#define INCLUDE_ANSI_ASSERT     /* ANSI-C assert library functionality */
#define INCLUDE_ANSI_CTYPE      /* ANSI-C ctype library functionality */
#define INCLUDE_ANSI_LOCALE     /* ANSI-C locale library functionality */
#define INCLUDE_ANSI_MATH       /* ANSI-C math library functionality */
#define INCLUDE_ANSI_STDIO      /* ANSI-C stdio library functionality */
#define INCLUDE_ANSI_STDLIB     /* ANSI-C stdlib library functionality */
#define INCLUDE_ANSI_STRING     /* ANSI-C string library functionality */
#define INCLUDE_ANSI_TIME       /* ANSI-C time library functionality */
#define INCLUDE_EDR_SYSDBG_FLAG /* include ED&R debug flag support */
#ifdef _WRS_CONFIG_SMP
#define INCLUDE_SPINLOCK_ISR_ND	/* fast but non-fair ISR spin locks */
#endif /* _WRS_CONFIG_SMP */

/* Bootrom components
 *
 * These components are included in default bootrom builds.
 * BSPs can disable these components in their config.h file.
 */

#ifdef BOOTAPP
/*
 * Included Bootrom Facilities
 */

#define	INCLUDE_BOOT_APP         /* Generic BootApp support */
#define	INCLUDE_BOOT_EXC_HANDLER /* BootApp Exception Handling support */
#define	INCLUDE_BOOT_FTP_LOADER  /* BootApp FTP Network support */
#define	INCLUDE_BOOT_INIT        /* BootApp Initialization */
#define	INCLUDE_BOOT_MEM_CMDS    /* BootApp Memory Shell Commands */
#define	INCLUDE_BOOT_NETWORK     /* BootApp Network support */
#define	INCLUDE_BOOT_RSH_LOADER  /* BootApp RSH Network support */
#define	INCLUDE_BOOT_SHELL       /* BootApp Shell */
#define	INCLUDE_BOOT_TFTP_LOADER /* BootApp TFTP Network support */
#define INCLUDE_ELF              /* ELF image support */
#define INCLUDE_NET_BOOT_CONFIG  /* network boot device configuration */
#define INCLUDE_POSIX_CLOCKS     /* POSIX clock support */

/*
 * Excluded Optional Bootrom Facilities
 */

#undef  INCLUDE_BOOT_MMU_BASIC   /* Bootrom MMU Basic support */
#undef  INCLUDE_BOOT_WDB         /* Bootrom WDB support */
#undef  INCLUDE_TSFS_BOOT        /* Boot using Target Server File System */

/* Bootrom disabled facilities
 *
 * These components are not included in default bootrom builds.
 * BSPs can enable these components in their config.h file.
 */

#undef INCLUDE_ANSI_ALL        /* complete ANSI C library functions */
#undef INCLUDE_ANSI_ASSERT     /* ANSI-C assert library functionality */
#undef INCLUDE_ANSI_LOCALE     /* ANSI-C locale library functionality */
#undef INCLUDE_ANSI_MATH       /* ANSI-C math library functionality */
#undef INCLUDE_ANSI_STDIO      /* ANSI-C stdio library functionality */
#undef INCLUDE_ANSI_STDLIB     /* ANSI-C stdlib library functionality */
#undef INCLUDE_CTORS_DTORS
#undef INCLUDE_EXC_SHOW
#undef INCLUDE_FLOATING_POINT  /* floating point I/O */
#undef INCLUDE_FORMATTED_IO	   /* formatted I/O */
#undef INCLUDE_IO_BASIC        /* basic I/O system */
#undef INCLUDE_IO_REMOVABLE    /* I/O device removability (eg USB unplug) */
#undef INCLUDE_IO_FILE_SYSTEM  /* I/O filesystem (uses REMOVABLE) */
#undef INCLUDE_IO_RTP          /* I/O from RTPs */
#undef INCLUDE_IO_POSIX        /* I/O POSIX features */
#undef INCLUDE_IO_MISC         /* I/O: obsolete features */
#undef INCLUDE_IO_SYSTEM       /* include I/O system */
#undef INCLUDE_MEM_MGR_INFO    /* memory manager info routines */
#undef INCLUDE_SIGNALS         /* software signal library */
#undef INCLUDE_STDIO           /* standard I/O */
#undef INCLUDE_TASK_VARS       /* task variable package */
#undef INCLUDE_TIMEX           /* timexLib for exec timing */
#undef INCLUDE_VXEVENTS        /* VxWorks events */
#endif  /* BOOTAPP */

/* for VxWorks traditional scheduler */

/*
 * VX_NATIVE_SCHED_CONSTANT_RDY_Q will be obsoleted and will be replaced
 * by VX_TRAD_SCHED_CONSTANT_RDY_Q.
 */

#ifdef INCLUDE_CONSTANT_RDY_Q
#define VX_TRAD_SCHED_CONSTANT_RDY_Q  TRUE
#define VX_NATIVE_SCHED_CONSTANT_RDY_Q  VX_TRAD_SCHED_CONSTANT_RDY_Q
#else
#define VX_TRAD_SCHED_CONSTANT_RDY_Q  FALSE
#define VX_NATIVE_SCHED_CONSTANT_RDY_Q  VX_TRAD_SCHED_CONSTANT_RDY_Q
#endif


/* CPU-SPECIFIC INCLUDED SOFTWARE FACILITIES */

/* include support for possibly existing floating point coprocessor */

#if     (CPU==MC68020 || CPU==MC68040 || CPU==MC68060 || CPU==CPU32)
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* CPU==MC68020 || CPU==MC68040 || CPU==MC68060 || CPU==CPU32 */

#if     (CPU_FAMILY == SPARC) && defined(INCLUDE_FLOATING_POINT)
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* CPU_FAMILY == SPARC */

#if     (CPU_FAMILY==MIPS)
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* CPU_FAMILY==MIPS */

#if     ((CPU_FAMILY==PPC) && (CPU!=PPC403) && (CPU!=PPC405) && (CPU!=PPC440) \
	&& (CPU!=PPCEC603) && (CPU!=PPC860))
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* (CPU_FAMILY==PPC) && (CPU!=PPC4xx) && (CPU!=PPCEC603) && (CPU!=PPC860) */

#if     (CPU==I960KB)
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* I960KB */

#if     (CPU_FAMILY==I80X86)
#define INCLUDE_HW_FP           /* potential hardware fp support */
#endif  /* CPU_FAMILY==I80X86 */

#if     (CPU_FAMILY==SH)
#if     (CPU==SH7750)
#define INCLUDE_HW_FP
#else
#define INCLUDE_SW_FP
#endif  /* CPU==SH7750 */
#endif  /* CPU_FAMILY==SH */

#if   (CPU_FAMILY==ARM)
#if   (CPU==ARMARCH7)
#define INCLUDE_VFP
#else
#define INCLUDE_SW_FP
#endif  /* (CPU==ARMARCH7) */
#endif  /* CPU_FAMILY==ARM */

/* define appropriate object module format for a given architecture */

#if     ((CPU_FAMILY==MIPS) || (CPU_FAMILY==PPC) || (CPU_FAMILY==SH) || \
	 (CPU_FAMILY==SIMSPARCSOLARIS) || (CPU_FAMILY==COLDFIRE) || \
	 (CPU_FAMILY==I80X86) || (CPU_FAMILY==ARM) || (CPU_FAMILY==SIMNT) || \
	 (CPU==SIMLINUX))
#define INCLUDE_ELF             /* ELF object modules */
#else
#if     ((CPU_FAMILY==I960) || (CPU_FAMILY==AM29XXX))
#define INCLUDE_COFF            /* COFF object modules */
#else
#define INCLUDE_AOUT            /* a.out object modules */
#endif
#endif

/*
 * Disabling power management for PPC SMP for now. For PPC, The 745x variant
 * appears to have issues with power management for SMP.  (CQ:104713)
 */

#if ((CPU_FAMILY == PPC) && defined (_WRS_CONFIG_SMP))
#undef _WRS_ARCH_HAS_CPU_PWR_MGMT
#endif /* CPU_FAMILY == PPC && defined _WRS_CONFIG_SMP */

/* include power management if supported by architecture */

#ifdef _WRS_ARCH_HAS_CPU_PWR_MGMT

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
#define     CPU_PWR_ISR_P_STATE       (0)
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

#endif /* _WRS_ARCH_HAS_CPU_PWR_MGMT */

/******************************************************************************/
/*                                                                            */
/*                          EXCLUDED FACILITIES                               */
/*                                                                            */
/******************************************************************************/

#if FALSE

#define INCLUDE_CODETEST        /* CodeTEST target utilities library */
#define INCLUDE_CORE_DUMP	/* Kernel Core Dump */
#define INCLUDE_CORE_DUMP_COMPRESS /* Zlib Kernel Core Dump Compression */
#define INCLUDE_CORE_DUMP_COMPRESS_RLE /* RLE Kernel Core Dump Compression */
#define INCLUDE_CORE_DUMP_RTP	/* RTP Core Dump */
#define	INCLUDE_CORE_DUMP_RTP_FS /* File System support for RTP Core Dump */
#define INCLUDE_CORE_DUMP_RTP_COMPRESS_ZLIB /* Zlib RTP Core Dump Compression */
#define INCLUDE_CORE_DUMP_RTP_COMPRESS_RLE /* RLE RTP Core Dump Compression */
#define INCLUDE_CORE_DUMP_SHOW	/* Core Dump Show routines */
#define INCLUDE_CORE_DUMP_MEM	/* Core Dump Memory storage device */
#define INCLUDE_CORE_DUMP_MEM_FILTER /* Core Dump memory region filtering */
#define INCLUDE_CPLUS           /* include C++ support */
#define INCLUDE_CPLUS_IOSTREAMS /* include basic iostreams classes */
#define INCLUDE_CPLUS_STL       /* include Standard Template Library core */
#define INCLUDE_CPLUS_STRING      /* include string class */
#define INCLUDE_CPLUS_STRING_IO   /* include i/o for string class */
#define INCLUDE_CPLUS_COMPLEX     /* include complex number class */
#define INCLUDE_CPLUS_COMPLEX_IO  /* include i/o for complex number class */
#define INCLUDE_CPLUS_IOSTREAMS_FULL  /* include all of iostreams */
#define INCLUDE_CONFIGURATION_5_2 /* pre-tornado tools */
#define INCLUDE_DEBUG           /* pre-tornado debugging */
#define INCLUDE_DEMO            /* include simple demo instead of shell */

#define INCLUDE_FS_EVENT_UTIL   /* File system event utility */
#define INCLUDE_FS_MONITOR      /* File system monitor */
#define INCLUDE_ERF             /* Event reporting framework */
#define INCLUDE_DEVICE_MANAGER  /* Device Manager */
#define INCLUDE_XBD             /* Extended block device */
#define INCLUDE_XBD_PART_LIB    /* XBD Partition Library */
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
#define INCLUDE_DOSFS_DIR_VFAT  /* Microsoft VFAT direct handler */
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

#define INCLUDE_HTML            /* include HTML support */
#define INCLUDE_JAVA            /* Java virtual machine */
#define INCLUDE_KERNEL_HARDENING/* kernel hardening features: text segment    */
				/* write-protection, exception vector table   */
				/* write-protection, stack overflow &         */
				/* underflow detection, non-executable stacks */
#define INCLUDE_LSTLIB          /* include linked list library lstLib.c */
#define INCLUDE_SOUND           /* include support for sampled audio data */
#define INCLUDE_LOADER          /* object module loading */
#define INCLUDE_MEMDRV          /* memory disk driver */

/* INCLUDE_MMU_BASIC is defined by many bsp's in config.h */

#define INCLUDE_MMU_BASIC       /* bundled mmu support */

#define INCLUDE_OBJ_OWNERSHIP   /* include object management ownership */
#define INCLUDE_OBJ_INFO	/* include object info routines */

#define INCLUDE_POSIX_ADVISORY_FILE_LOCKING  /* POSIX advisory file locking */
#define INCLUDE_POSIX_AIO       /* POSIX async I/O support */
#define INCLUDE_POSIX_AIO_SYSDRV/* POSIX async I/O system driver */
#define INCLUDE_POSIX_FTRUNC	/* POSIX ftruncate routine */
#define INCLUDE_POSIX_MAPPED_FILES /* POSIX memory mapped files */
#define INCLUDE_POSIX_MEM	/* POSIX memory locking */
#define INCLUDE_POSIX_MQ        /* POSIX message queue support */
#define INCLUDE_POSIX_PTHREADS  /* POSIX pthreads support */
#define INCLUDE_POSIX_SCHED	/* POSIX scheduling */
#define INCLUDE_POSIX_SEM 	/* POSIX semaphores */
#define INCLUDE_POSIX_SIGNALS 	/* POSIX queued signals */
#define INCLUDE_POSIX_SHM 	/* POSIX shared memory objects */
#define INCLUDE_POSIX_TIMERS 	/* POSIX timers */
#define INCLUDE_POSIX_TIMER_SHOW /* show_timer() function */
#define INCLUDE_POSIX_CLOCKS	/* POSIX clocks */
#define INCLUDE_POSIX_THREAD_CPUTIME /* POSIX thread CPU-time in RTPs */
#define INCLUDE_POSIX_DIRLIB 	/* POSIX directory utilities */
#define INCLUDE_SC_POSIX 	/* POSIX system calls */
#define INCLUDE_POSIX_PTHREAD_SCHEDULER /* POSIX scheduler for pthread in RTP */
#define INCLUDE_PX_SCHED_SPORADIC_POLICY /* POSIX sporadic scheduling policy */
#define INCLUDE_CUSTOM_SCHEDULER        /* user specified scheduler */
#define INCLUDE_VX_NATIVE_SCHEDULER /* OBSOLETED: replace by
				       INCLUDE_VX_TRADITIONAL_SCHEDULER */
#define INLCUDE_SEM_INFO	
#define INCLUDE_SEM_READ_WRITE        /* include reader/writer semaphores */
#define INCLUDE_SEM_READ_WRITE_CREATE /* include reader/writer semaphores */
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
#define INCLUDE_SC_IOS          /* Real Time Process, IO support */
#define INCLUDE_ROMFS           /* rom based file system */
#define INCLUDE_SECURITY        /* shell security for network access */
#define INCLUDE_SHARED_DATA     /* shared data regions */
#define INCLUDE_SHELL           /* interactive c-expression interpreter */
#define INCLUDE_SHELL_VI_MODE   /* vi editing mode for the shell */
#define INCLUDE_SHELL_EMACS_MODE /* emacs editing mode for the shell */
#define INCLUDE_SHELL_INTERP_C  /* C interpreter */
#define INCLUDE_SHELL_INTERP_CMD /* shell command interpreter */
#define INCLUDE_SHELL_HISTORY_FILE /* to save and load the shell history */
#define INCLUDE_HISTORY_FILE_SHELL_CMD /* cmd interpreter history save/load */
#define INCLUDE_SHL             /* shared libraries */
#define INCLUDE_SHL_SHOW        /* shared libraries show component */
#define INCLUDE_SHOW_ROUTINES   /* show routines for system facilities*/
#define INCLUDE_SM_OBJ          /* shared memory objects (unbundled) */
#define INCLUDE_SPY             /* spyLib for task monitoring */
#define INCLUDE_TIP		/* tipLib for multiple serial line management */
#define INCLUDE_STANDALONE_SYM_TBL /* compiled-in symbol table */
#define INCLUDE_STARTUP_SCRIPT  /* execute start-up script */
#define INCLUDE_STAT_SYM_TBL    /* create user-readable error status */
#define INCLUDE_SW_FP           /* software floating point emulation */
#define INCLUDE_SYM_TBL         /* symbol table package */
#define INCLUDE_TASK_LIST	/* include task list management */
#define INCLUDE_TASK_STACK_NO_EXEC /* non-executable task stacks */
#define INCLUDE_WINDML          /* include WindML multimedia Library support */
#define INCLUDE_UNLOADER        /* object module unloading */
#define INCLUDE_VXEVENTS        /* include VxWorks events */
#define INCLUDE_WILLOWS_RT      /* include Willows RT library */
#define INCLUDE_WDB_DPRINTF	/* WDB dynamic printf support */
#define INCLUDE_WDB_MDL_SYM_SYNC /* synchronize host and target symbol tables */
#define	INCLUDE_WDB_PROXY	/* WDB agent proxy */
#define INCLUDE_WDB_PROXY_UDP	/* UDP/IP WDB Agent Proxy backend */
#define INCLUDE_WDB_PROXY_TIPC	/* WDB Agent Proxy backend */
#define INCLUDE_WDB_RTP		/* WDB RTP support */
#define INCLUDE_WDB_RTP_BP	/* WDB RTP breakpoint support */
#define INCLUDE_WDB_RTP_CONTROL /* WDB RTP control support */
#define INCLUDE_WDB_TSFS	/* target-server file system */
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
#define INCLUDE_WVTIPC          /* TIPC WV instrumentation */

#define INCLUDE_POSIX_TRACE     /* POSIX tracing in RTP */

#define INCLUDE_ANSI_5_0        /* include only version 5.0 ANSI support */
#define INCLUDE_DELETE_5_0      /* define delete() function as in VxWorks 5.0 */

#define INCLUDE_EDR_PM          /* ED&R persistent memory */
#define INCLUDE_EDR_ERRLOG      /* ED&R error log */

#define INCLUDE_RTP_HOOKS       /* RTP create/delete hook support */
#define INCLUDE_SYSCALL_HOOKS   /* System call entry/exit/register hooks */

#define INCLUDE_MEM_EDR         /* memory manager ED&R */
#define INCLUDE_MEM_EDR_RTC     /* Run-time checker support for Diab */
#define INCLUDE_MEM_EDR_SHOW    /* memory manager ED&R show */
#define INCLUDE_MEM_EDR_RTP_SHOW  /* memory manager ED&R show for RTPs */

#define INCLUDE_PASSFS          /* include VxSim pass-through filesystem */

#define	INCLUDE_TLS		/* thread local storage support */

#define  INCLUDE_SYSCTL         /* System control function */
#define  INCLUDE_SYSCTL_CLI     /* Command line Sysctl utility */

/* The following components are available only with RTPs */
#define  INCLUDE_SYSCTL_HW     /* System control hardware info registration */
#define  INCLUDE_SC_SYSCTL     /* System call support for sysctl() */

/* Kprintf facility related Components
 * INCLUDE_DEBUG_KWRITE_SIO & INCLUDE_DEBUG_KWRITE_USER are mutually
 * exclusive components. When INCLUDE_DEBUG_KWRITE_USER is defined, 
 * the user should provide a user output routine for kprintf() usage.
 * This routine should be defined for DEBUG_KWRITE_USR_RTN parameter
 * and a prototype should be provided for the same. For more information 
 * refer VxWorks Programmer's Guide
 */
#define  INCLUDE_DEBUG_KWRITE_SIO /* poll mode sio for kprintf & kputs */
#define  INCLUDE_DEBUG_KPRINTF    /* formatted kernel printf */
#define  INCLUDE_DEBUG_KPUTS      /* unformatted kernel printf */
#define  INCLUDE_DEBUG_KWRITE_USER /* user provided I/O for kprintf & kputs */

#endif  /* FALSE */

/******************************************************************************/
/*                                                                            */
/*                  KERNEL SOFTWARE CONFIGURATION                             */
/*                                                                            */
/******************************************************************************/

#define USER_I_CACHE_MODE       CACHE_WRITETHROUGH  /* default mode */
#define USER_D_CACHE_MODE       CACHE_WRITETHROUGH  /* default mode */
#define USER_I_CACHE_ENABLE                         /* undef to leave disabled*/
#define USER_D_CACHE_ENABLE                         /* undef to leave disabled*/
#define USER_B_CACHE_ENABLE                         /* undef to leave disabled*/
#define USER_I_MMU_ENABLE                           /* undef to leave disabled*/
#define USER_D_MMU_ENABLE                           /* undef to leave disabled*/

#define SYM_TBL_HASH_SIZE_LOG2  8       /* 256 entry hash table symbol table */
#define STAT_TBL_HASH_SIZE_LOG2 6       /* 64 entry hash table for status */
#define MQ_HASH_SIZE            0       /* POSIX message queue hash table size
					 * 0 = default */
#define NUM_SIGNAL_QUEUES       16      /* POSIX queued signal count */

#define FREE_RAM_ADRS           (end)   /* start right after bss of VxWorks */

#define MAX_REBOOT_HOOKS	  8	

#define SEM_RW_MAX_CONCURRENT_READERS 32  /* read/write semaphore maximum
					   * concurrent readers */

#ifndef POSIX_PTHREAD_RR_TIMESLICE      /* timeslice for SCHED_RR pthread */
#define POSIX_PTHREAD_RR_TIMESLICE  10  /* default 10 ticks */
#endif /* POSIX_PTHREAD_RR_TIMESLICE */

#define SHM_DEV_NAME		"/shm"	/* device name for POSIX shared */
					/* memory objects */
#ifndef CDF_OVERRIDE
#ifdef INCLUDE_RTP_POSIX_PSE52
#define POSIX_SIGNAL_MODE       TRUE
#else
#define POSIX_SIGNAL_MODE       FALSE
#endif /* INCLUDE_RTP_POSIX_PSE52 */
#endif /* ! CDF_OVERRIDE */

/* object management parameters */

#define OBJ_LIBRARY_OPTIONS     0

/* I/O system parameters */

#define SYS_CLK_RATE            60      /* default system clock rate */
#define NUM_DRIVERS             20      /* max 20 drivers in drvTable */
#define NUM_FILES               50      /* max 50 files open simultaneously */
#if !defined(CDF_OVERRIDE)
#ifdef INCLUDE_RTP_POSIX_PSE52
#define IOS_POSIX_PSE52_MODE	TRUE	/* Core I/O PSE52 mode is on */
#else
#define IOS_POSIX_PSE52_MODE    FALSE   /* Core I/O PSE52 mode is off */
#endif
#endif
#define NUM_DOSFS_FILES         20      /* max 20 dosFs files open */
#define NUM_RAWFS_FILES         5       /* max 5  rawFs files open */
#define MAX_LOG_MSGS            50      /* max 50 log msgs */
#define INIT_NBIO_MSGS          32      /* 32 * 128 bytes = 4096 bytes */

#define NUM_TTY                 2       /* number of tty channels */
#define CONSOLE_TTY             0       /* console channel */

#define TYLIB_XOFF_PCNT         85      /* XOFF at 85% full */
#define TYLIB_XON_PCNT          50      /* XON at 50% full */
#define TYLIB_WRT_THRESHLD      20      /* wakeup at 20 chars in tx buff */

#define CONSOLE_BAUD_RATE       9600    /* console baud rate */

#ifdef  INCLUDE_SLIP
#define SLIP_TTY                1       /* serial line IP channel */
#endif  /* INCLUDE_SLIP */

#define MAX_LIO_CALLS           0       /* max outstanding lio calls 0=default */
#define MAX_AIO_SYS_TASKS       0       /* max aio system tasks, 0 = default */
#define AIO_TASK_PRIORITY       0       /* aio system tasks prior, 0 = default */
#define AIO_TASK_STACK_SIZE     0       /* aio tasks stack size, 0 = default */

/* kernel and system task parameters by architecture */

#if     CPU_FAMILY==MC680X0
#define INT_LOCK_LEVEL          0x7     /* 68k interrupt disable mask */
#define ROOT_STACK_SIZE         10000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x1000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          1000    /* size of ISR stack, in bytes */
#define TRAP_DEBUG              2       /* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif  /* CPU_FAMILY==MC680X0 */

#if     CPU_FAMILY==COLDFIRE
#define INT_LOCK_LEVEL          0x7     /* 68k interrupt disable mask */
#define ROOT_STACK_SIZE         10000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x1000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          1000    /* size of ISR stack, in bytes */
#define TRAP_DEBUG              2       /* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif  /* CPU_FAMILY==COLDFIRE */

#if     CPU_FAMILY==SPARC
#define INT_LOCK_LEVEL          15      /* SPARC interrupt disable level */
#define ROOT_STACK_SIZE         10000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          10000   /* size of ISR stack, in bytes */
#define VEC_BASE                (LOCAL_MEM_LOCAL_ADRS + 0x1000)
#define VEC_BASE_ADRS           ((char *) VEC_BASE)
#endif  /* CPU_FAMILY==SPARC */

#if CPU_FAMILY==SIMNT || CPU_FAMILY==SIMSPARCSOLARIS || CPU_FAMILY==SIMLINUX
#define INT_LOCK_LEVEL          0x1     /* interrupt disable mask */
#define ROOT_STACK_SIZE         20000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          50000   /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           0       /* dummy */
#endif  /* CPU_FAMILY == SIMxxx */

#if     CPU_FAMILY==I960
#define INT_LOCK_LEVEL          0x1f    /* i960 interrupt disable mask */
#define ROOT_STACK_SIZE         20000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          1000    /* size of ISR stack, in bytes */
#define TRAP_DEBUG              0       /* n/a for the 80960 */
#define VEC_BASE_ADRS           NONE    /* base register not reconfigurable */
#endif  /* CPU_FAMILY==I960 */

#if     CPU_FAMILY==MIPS
#define INT_LOCK_LEVEL          0x1     /* R3K interrupt disable mask */
#define ROOT_STACK_SIZE         (20000) /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        (0x10000) /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          (0x2000)/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          (5000)  /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           ((char *) 0x0)  /* meaningless in R3k land */
#define VME_VECTORED            FALSE   /* use vectored VME interrupts */
#define TRAP_DEBUG              0       /* trap 0 - breakpoint trap */
#define EXC_PAGE_PHYS_ADRS      0x2000  /* 2nd page of RAM used by mapped OS */
#endif  /* CPU_FAMILY==MIPS */

#if     CPU_FAMILY==PPC
#define INT_LOCK_LEVEL          0x0     /* not used */
#define BOOT_CMD_STACK_SIZE     (11000) /* size of boot cmd stack, in bytes */
#define ROOT_STACK_SIZE         (24000) /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        (0x10000) /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          (5000)  /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           ((char *) 0x0)
#endif  /* CPU_FAMILY==PPC */

#if     CPU_FAMILY==I80X86
#define INT_LOCK_LEVEL          0x0     /* 80x86 interrupt disable mask */
#define ROOT_STACK_SIZE         10000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          1000    /* size of ISR stack, in bytes */
#define TRAP_DEBUG              0       /* not used */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif  /* CPU_FAMILY==I80X86 */

#if     CPU_FAMILY==AM29XXX
#define INT_LOCK_LEVEL          0x0001  /* 29k all interrupts disable mask */
#define ROOT_STACK_SIZE         10000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          10000   /* size of ISR stack, in bytes */
#define TRAP_DEBUG              15      /* trap 2 - breakpoint trap */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif  /* CPU_FAMILY==AM29XXX */

#if     CPU_FAMILY==SH
#define INT_LOCK_LEVEL          15
#define ROOT_STACK_SIZE         10000
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x1000  /* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          1000
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
#endif  /* CPU_FAMILY==SH */

#if     CPU_FAMILY==ARM
#define INT_LOCK_LEVEL          0       /* interrupt disable mask - unused */
#define ROOT_STACK_SIZE         0x4000  /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        0x10000 /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE          0x2000  /* size of WDB agents stack, in bytes */
#define TRAP_DEBUG              0       /* not used */
#define VEC_BASE_ADRS           ((char *) LOCAL_MEM_LOCAL_ADRS)
/*
 * NOTE: ISR_STACK_SIZE defined in config.h, not here - BSP interrupt
 *       structure dependent
 */
#endif  /* CPU_FAMILY==ARM */

#ifndef BOOT_CMD_STACK_SIZE
#define BOOT_CMD_STACK_SIZE     11000
#endif  /* BOOT_CMD_STACK_SIZE */

/* RTP initialization constants */

#ifndef CDF_OVERRIDE
#define RTP_HOOK_TBL_SIZE	8
#define SYSCALL_HOOK_TBL_SIZE	8
#define RTP_SIGNAL_QUEUE_SIZE	32  	/* must be at least 32 for POSIX */
#define RTP_FD_NUM_MAX		20	/* must be at least 20 for POSIX */
#define RTP_CODE_REGION_START	0	/* no RTP code region reserved by */
#define RTP_CODE_REGION_SIZE	0	/* default (applies to overlapped */
					/* virtual memory only). */

#define	RTP_OVERLAPPED_ADDRESS_SPACE	FALSE	/* Use flat virtual address
						   space for RTPs by default */

/* Shared Data initialization constants */

#define SD_HOOK_TBL_SIZE     8
#endif /* CDF_OVERRIDE */

/* WDB debug agent configuration */

#ifdef  INCLUDE_WDB

/* optional agent facilities */

#define INCLUDE_WDB_BANNER              /* print banner after agent starts */
#define INCLUDE_WDB_VIO                 /* virtual I/O support */
#define INCLUDE_WDB_TTY_TEST            /* test serial line communication */
#define INCLUDE_WDB_START_NOTIFY        /* notify the host of task creation */
#define INCLUDE_WDB_USER_EVENT          /* user events handling */

/* core agent facilities - do not remove */

#define INCLUDE_WDB_CTXT                /* context control */
#define INCLUDE_WDB_FUNC_CALL           /* spawn function as separate task */
#define INCLUDE_WDB_DIRECT_CALL         /* call function in agents context */
#define INCLUDE_WDB_EVENTS              /* host async event notification */
#define INCLUDE_WDB_GOPHER              /* gopher info gathering */
#define INCLUDE_WDB_BP                  /* breakpoint support */
#define INCLUDE_WDB_EXC_NOTIFY          /* notify host of exceptions */
#define INCLUDE_WDB_EXIT_NOTIFY         /* notify the host of task exit */
#define INCLUDE_WDB_REG                 /* get/set hardware registers */
#define INCLUDE_WDB_EVENTPOINTS         /* eventpoints handling */
#define INCLUDE_WDB_MEM                 /* optional memory services */

/* WDB agent initialization modes */

#define WDB_POST_KERNEL_INIT    0       /* WDB starts after kernel */
#define WDB_PRE_KERNEL_INIT     1       /* WDB starts before kernel */

#ifndef WDB_INIT
#define WDB_INIT        WDB_POST_KERNEL_INIT
#endif  /* WDB_INIT */

/* agent mode */

#ifdef _WRS_VX_AMP        /* in AMP mode INCLUDE_WDB_SYS results in project errors */
    #define WDB_MODE        WDB_MODE_TASK
#else
    #define WDB_MODE        WDB_MODE_DUAL
#endif
/* #define WDB_MODE        WDB_MODE_DUAL   this was expanded in tcl code to INCLUDE_WDB_SYS & INCLUDE_WDB_TASK*/ /* WDB_MODE_[DUAL|TASK|EXTERN] */

/* agent communication paths */

#define WDB_COMM_NETWORK        0       /* vxWorks network      - task mode */
#define WDB_COMM_SERIAL         1       /* raw serial           - bimodal   */
#define WDB_COMM_TYCODRV_5_2    2       /* older serial driver  - task mode */
#define WDB_COMM_ULIP           3       /* vxSim packet device  - bimodal   */
#define WDB_COMM_NETROM         4       /* netrom packet device - bimodal   */
#define WDB_COMM_CUSTOM         5       /* custom packet device - bimodal   */
#define WDB_COMM_END            6       /* END packet device    - bimodal   */
#define WDB_COMM_PIPE           7       /* vxSim pipe device    - bimodal   */
#define WDB_COMM_VTMD           8       /* TMD packet device    - bimodal   */
#define WDB_COMM_TIPC           9       /* TIPC packet device   - task mode */
#define WDB_COMM_MIPC          10       /* MIPC packet device   - task mode */

/* communication path configuration */

#ifndef WDB_COMM_TYPE
#define WDB_COMM_TYPE WDB_COMM_END      /* default path is the network */
#endif

#ifndef WDB_COMM_PORT
#define WDB_COMM_PORT           0x4321  /* default WDB UDP port value */
#endif

#ifndef WDB_TTY_CHANNEL
#define WDB_TTY_CHANNEL         1       /* default Sio SERIAL channel */
#endif

#ifndef WDB_TTY_DEV_NAME
#define WDB_TTY_DEV_NAME    "/tyCo/1"   /* default TYCODRV_5_2 device name */
#endif

#ifndef WDB_TTY_BAUD
#define WDB_TTY_BAUD            9600    /* default baud rate */
#endif

#ifndef WDB_TTY_ECHO
#define WDB_TTY_ECHO            FALSE   /* default echoing debug flag */
#endif

#ifndef WDB_END_DEVICE_NAME
#define WDB_END_DEVICE_NAME     NULL
#endif  /* WDB_END_DEVICE_NAME */

#ifndef WDB_END_DEVICE_UNIT
#define WDB_END_DEVICE_UNIT     0
#endif  /* WDB_END_DEVICE_UNIT */

#ifndef WDB_END_DEVICE_ADDRESS
#define WDB_END_DEVICE_ADDRESS NULL
#endif /* WDB_END_DEVICE_ADDRESS */

#ifndef WDB_ULIP_DEV
#define WDB_ULIP_DEV    "/dev/ulip14"   /* default ULIP packet device */
#endif

#define WDB_TIPC_PORT_TYPE      70      /* WDB TIPC port type */
#define WDB_TIPC_PORT_INSTANCE  71      /* WDB TIPC port instance */

#ifndef WDB_MIPC_PORT
#define WDB_MIPC_PORT		2	/* default WDB MIPC port value */
#endif

#ifndef WDB_MIPC_BUS_NAME
#define WDB_MIPC_BUS_NAME	"main"	/* default WDB MIPC bus name */
#endif

/* gopher configuration */

#define WDB_GOPHER_TAPE_LEN     1400    /* maximal length of a gopher tape */
#define WDB_GOPHER_TAPE_NB      10      /* maximal number of dynamically */
					/* allocated tapes */

/* miscellaneous agent constants */

#define WDB_MTU         	1500	/* max RPC message size */
#define WDB_POOL_SIZE 		((unsigned int) (sysMemTop() - FREE_RAM_ADRS)/16)
					/* memory pool for host tools */
#define WDB_REGS_SIZE		512	/* temporary buffer size for coproc */
					/* registers access in system mode */
#define WDB_SPAWN_STACK_SIZE	0x5000	/* def. stack size for spawned task */
#define WDB_SPAWN_PRI		100	/* def. priority for spawned task */
#define	WDB_RTP_PRIORITY	200	/* def. priority for RTP initial task */
#define	WDB_RESTART_TIME	10	/* delay before error-restarting */
					/* agent */
#define	WDB_MAX_RESTARTS	5	/* max # agent restarts on error */
#define WDB_BP_MAX		25	/* max # of break points */
#define WDB_TASK_PRIORITY       3	/* priority of task agent */

#define	WDB_DPRINTF_MAX_MSGS	100	/* max # of pending dynamic printf */
					/* messages */
#define	WDB_DPRINTF_MAX_MSG_LENGTH	100	/* max size of dynamic printf */
					/* message */
#ifdef	VX_DSP_TASK
#define WDB_SPAWN_OPTS		VX_FP_TASK | VX_DSP_TASK
					/* def. options for spawned task */
#define	WDB_TASK_OPTIONS	VX_UNBREAKABLE | VX_FP_TASK | VX_DSP_TASK
					/* WDB task options */
#else
#define WDB_SPAWN_OPTS		VX_FP_TASK /* def. options for spawned task */
#define	WDB_TASK_OPTIONS	VX_UNBREAKABLE | VX_FP_TASK
					/* WDB task options */
#endif	/* VX_DSP_TASK */



#ifdef BOOTAPP
/* Disable WDB by default for Bootroms.
 * This can be added back into the bootroms by defining INCLUDE_BOOT_WDB
 */
#undef  INCLUDE_WDB             /* WDB debug agent */
#undef  INCLUDE_WDB_BANNER		/* print banner after agent starts */
#undef 	INCLUDE_WDB_TTY_TEST		/* test serial line communication */
#undef  INCLUDE_WDB_START_NOTIFY	/* notify the host of task creation */
#undef  INCLUDE_WDB_USER_EVENT		/* user events handling */
#undef  INCLUDE_WDB_CTXT		/* context control */
#undef  INCLUDE_WDB_FUNC_CALL		/* spawn function as separate task */
#undef  INCLUDE_WDB_GOPHER		/* gopher info gathering */
#undef  INCLUDE_WDB_EXIT_NOTIFY		/* notify the host of task exit */
#undef  INCLUDE_WDB_REG			/* get/set hardware registers */
#undef  INCLUDE_WDB_EVENTPOINTS		/* eventpoints handling */
#undef  INCLUDE_WDB_MEM			/* optional memory services */
#undef  INCLUDE_WDB_BP			/* breakpoint support */
#endif  /* BOOTAPP */

#endif  /* INCLUDE_WDB */

/* Wind River Agent Proxy configuration */

#define WDB_PROXY_PORT                  0x4444  /* TCP/IP port used by proxy */
#define WDB_PROXY_TASK_PRIORITY         55      /* Proxy Task priority */
#define WDB_PROXY_TASK_STACK_SIZE       10000   /* Proxy task stack size */
#define WDB_PROXY_TASK_OPTIONS          VX_UNBREAKABLE
						/* Proxy task options */
#define	WDB_PROXY_HAS_CONTROL_PORT	TRUE	/* Proxy control port support */
#define	WDB_PROXY_INIT_STRING		NULL	/* Proxy init string */

/* Shell default configuration */

#define SHELL_SECURE            FALSE /* enable console login/password prompt */
#define SHELL_START_AT_BOOT     TRUE  /* whether the shell starts at boot */
#define SHELL_COMPATIBLE        FALSE /* 5.x-compatible shell behavior */

/* Configuration inherited by all shell sessions */

#define SHELL_DEFAULT_CONFIG    \
"LINE_EDIT_MODE=,LINE_LENGTH=256,STRING_FREE=manual,INTERPRETER=,VXE_PATH=.;/romfs"

/* Configuration of the first shell session to start */

#define SHELL_FIRST_CONFIG      NULL

/* Configuration string of remote shell sessions (telnet, rlogin, etc) */

#define SHELL_REMOTE_CONFIG     NULL

#define SHELL_TASK_NAME_BASE	"tShell" /* root name of all shell tasks */
#define SHELL_TASK_PRIORITY	1	 /* VxWorks shell task priority */

/* Default options of the shell tasks */

#define SHELL_TASK_OPTIONS	(VX_FP_TASK | VX_UNBREAKABLE | COPROCS_ALL | VX_PRIVATE_ENV)

/* Maximum number of concurrent shell sessions (-1 means unlimited) */

#define SHELL_MAX_SESSIONS	-1

/* Tip utility default configuration */

#define TIP_CONFIG_STRING	""
#define TIP_ESCAPE_CHARACTER	"~"

/* defaults for tasks */

#define VX_GLOBAL_NO_STACK_FILL	FALSE

#ifndef _WRS_ARCH_IS_SIMULATOR
#if (CPU_FAMILY==MIPS) && (_WRS_INT_REGISTER_SIZE==8)
#define TASK_USER_EXC_STACK_SIZE        (8192*2) /* usr task exc stack size */
#else
#define TASK_USER_EXC_STACK_SIZE        8192 /* usr task exc stack size */
#endif

#else
#define TASK_USER_EXC_STACK_SIZE        (8192*3)
#endif  /* _WRS_ARCH_IS_SIMULATOR */

/*
 * These define the default sizes, in bytes, of the overflow and underflow
 * guard regions for task stacks and the interrupt stack.  The values
 * defined here (or as overridden in config.h) are used only if the
 * corresponding protection capabilities are configured, and will be
 * rounded up to a multiple of VM_PAGE_SIZE.
 */

/*  user task exception stack overflow protection size */
#define TASK_USER_EXC_STACK_OVERFLOW_SIZE     4096
/*user task execution stack overflow protection size      */
#define TASK_USER_EXEC_STACK_OVERFLOW_SIZE    4096
/* user task execution stack  underflow protection size */
#define TASK_USER_EXEC_STACK_UNDERFLOW_SIZE   4096
/* kernel task execution stack overflow protection size  */
#define TASK_KERNEL_EXEC_STACK_OVERFLOW_SIZE  4096
/* kernel task execution stack underflow protection size  */
#define TASK_KERNEL_EXEC_STACK_UNDERFLOW_SIZE 4096
#define INTERRUPT_STACK_OVERFLOW_SIZE   4096 /* stack overflow protection sz */
#define INTERRUPT_STACK_UNDERFLOW_SIZE  4096 /* stack underflow protection sz */

/*
 * To ease migration for applications that expect the old (incorrect)
 * behaviour for priority changes (including priority inheritance), the
 * TASK_PRIORITY_SET_COMPATIBLE configuration parameter is provided.
 * With the old behaviour (parameter set TRUE), a running task may
 * be preempted when it's priority is lowered to a level at which other
 * ready tasks are waiting. This configuration option will be
 * removed in a future.
 */

#define TASK_PRIORITY_SET_COMPATIBLE	FALSE

/*
 * VxWorks Events configuration options. For available options see the
 * eventLib reference guide.
 */

#define VXEVENTS_OPTIONS		EVENTS_NONINTERRUPTIBLE

/*
 * Default value for work queue size (in jobs).  MUST be a power of two
 * no greater than 0x10000.  Values below 64 may cause work queue panics.
 */
#define WIND_JOBS_MAX 64

/* Job Facility configurable parameters */
#define MAX_ISR_JOBS 16
#define JOB_TASK_STACK_SIZE 8000

#ifdef _WRS_CONFIG_SMP
  /* Enable all additional cores on an SMP based system. */
  #define ENABLE_ALL_CPUS                  TRUE

  /* Timeout for enabling CPUs in seconds */

  #define VX_ENABLE_CPU_TIMEOUT		   5

  /* include vxIpiLib for SMP, required by cpcLib. */
  #define INCLUDE_VXIPI
#endif /* _WRS_CONFIG_SMP */

#ifdef _WRS_VX_AMP
  #define INCLUDE_VXIPI
#endif /* _WRS_VX_AMP */

/* File system configurable parameters */

#define ERF_MAX_USR_CATEGORIES           16
#define ERF_MAX_USR_TYPES                16
#define DEVICE_MANAGER_MAX_NUM_DEVICES  255

#define DOSFS_DEFAULT_MAX_FILES          20
#define DOSFS_DEFAULT_CREATE_OPTIONS     0x04
#define DOSFS_DEFAULT_DATA_DIR_CACHE_SIZE           0x800000
#define DOSFS_DEFAULT_FAT_CACHE_SIZE                0x100000
#define DOSFS_CACHE_BACKGROUND_FLUSH_TASK_ENABLE    FALSE

#define HRFS_DEFAULT_MAX_BUFFERS         16
#define HRFS_DEFAULT_MAX_FILES           10
#define	HRFS_DEFAULT_COMMIT_POLICY       FS_COMMIT_POLICY_AUTO
#define HRFS_DEFAULT_COMMIT_PERIOD       5000

#define XBD_BLK_DEV_TASK_PRIORITY        200
#define XBD_BLK_DEV_DIRECT_BIO_HANDLING  FALSE

#define CDROM_COM_BUF_SIZE               3

#define FD_DEV_NAME                      "/fd"
#define FD_0_TYPE                        0
#define FD_1_TYPE                        -1

#define FS_NAMES_ATA_PRIMARY_MASTER      "/ata0a"
#define FS_NAMES_ATA_PRIMARY_SLAVE       ""
#define FS_NAMES_ATA_SECONDARY_MASTER    ""
#define FS_NAMES_ATA_SECONDARY_SLAVE     ""

/* direct BIO support for storage drivers */

#define ATA_DIRECT_BIO_HANDLING          FALSE
#define AHCI_DIRECT_BIO_HANDLING         FALSE
#define ICH_DIRECT_BIO_HANDLING          FALSE
#define FSLSATA_DIRECT_BIO_HANDLING      FALSE
#define FSLSDHC_DIRECT_BIO_HANDLING      FALSE
#define SI31XX_DIRECT_BIO_HANDLING       FALSE

#define TFFS_DRIVE_NUMBER                0
#define TFFS_REMOVABLE                   FALSE
#define TFFS_MOUNT_POINT                 "/tffs0"

#define BOOT_USB_OPEN_ATTEMPTS          10
#define BOOT_USB_ATTEMPT_DELAY          60
#define BOOT_USB_POST_MS_BULKONLY_INIT_DELAY 2

/* default value of the kernel heap size: used only if the RTP support is in */

#define KERNEL_HEAP_SIZE		((unsigned int)(sysMemTop() - FREE_RAM_ADRS) / 3 * 2)

/* default memory partition options in the kernel with INCLUDE_MEM_MGR_FULL */

#define MEM_PART_DEFAULT_OPTIONS	(MEM_ALLOC_ERROR_LOG_FLAG      | \
					 MEM_ALLOC_ERROR_EDR_WARN_FLAG | \
					 MEM_BLOCK_ERROR_LOG_FLAG      | \
					 MEM_BLOCK_ERROR_EDR_WARN_FLAG | \
					 MEM_BLOCK_ERROR_SUSPEND_FLAG  | \
					 MEM_BLOCK_CHECK)

/* RTP heap initial size: used only if the RTP support is in */

#define RTP_HEAP_INIT_SIZE		0x10000

/* memory ED&R configuration: used only if INCLUDE_MEM_EDR is defined */

#define MEDR_EXTENDED_ENABLE            FALSE   /* extended info (trace) */
#define MEDR_FREE_QUEUE_LEN             64      /* free queue length */
#define MEDR_FILL_FREE_ENABLE           FALSE   /* fill freed blocks */
#define MEDR_BLOCK_GUARD_ENABLE         FALSE   /* block guard zone */
#define MEDR_POOL_SIZE                  0x100000 /* memory for database pool */

/* default persistent memory size for ED&R */

#ifndef PM_RESERVED_MEM
#define PM_RESERVED_MEM                 (6 * VM_PAGE_SIZE)
#endif
#ifndef EDR_ERRLOG_SIZE
#define EDR_ERRLOG_SIZE                 (pmFreeSpace(EDR_PM_ARENA)/2)
#endif
#ifndef EDR_RECORD_SIZE
#define EDR_RECORD_SIZE                 4096
#endif

/* core dump default configuration */

#define CORE_DUMP_REGION_SIZE		pmFreeSpace(pmDefaultArena)
						/* coredump storage mem size */
#define CORE_DUMP_COMPRESSION_LEVEL	9	/* ZLib Kernel Core Dump */
						/* compression level */
#define CORE_DUMP_RTP_ZLIB_COMPRESSION_LEVEL 9	/* ZLib RTP Core Dump */
						/* compression level */
#define CORE_DUMP_MEM_FILTER_MAX	10	/* Max # of mem region filter */
#define CORE_DUMP_MAX_HOOKS		10	/* Max # of core dump create */
						/* hooks */
#define FATAL_SYSTEM_CORE_DUMP_ENABLE	TRUE	/* Generate core dump for */
						/* fatal system exceptions */
#define KERNEL_APPL_CORE_DUMP_ENABLE	FALSE	/* Generate core dump for */
						/* kernel tasks exceptions */
#define CORE_DUMP_CKSUM_ENABLE		FALSE	/* Compute core dump checksum */
#define CORE_DUMP_SKIP_TEXT_SECTION	TRUE	/* Remove VxWorks text */
						/* section from core dump */
#define CORE_DUMP_SKIP_USER_RESERVED_MEM FALSE	/* Remove User Reserved Mem */
						/* from core dump */
#define CORE_DUMP_STACK_SIZE		8192	/* core dump stack size */
#define CORE_DUMP_MEM_POOL_PAGE_SIZE	0x8000	/* core dump memory pool */
						/* pages size */
#define CORE_DUMP_MEM_POOL_PROTECTION_ENABLE	TRUE
						/* Protect core dump pool if */
						/* virtual memory is enabled */
#define CORE_DUMP_RTP_FS_PATH	"/tgtsvr"	/* RTP Core Dump Storage Path */

/* Spy default configuration */

#define SPY_TASKS_MAX		200	/* Max # of tasks that can be spied */

/* user kwrite routine config parameter - default NULL */

#define DEBUG_KWRITE_USR_RTN          NULL

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

#define SM_OBJ_MAX_TASK         40      /* max # of tasks using smObj */
#define SM_OBJ_MAX_SEM          60      /* max # of shared semaphores */
#define SM_OBJ_MAX_MSG_Q        10      /* max # of shared message queues */
#define SM_OBJ_MAX_MEM_PART     4       /* max # of shared memory partitions */
#define SM_OBJ_MAX_NAME         100     /* max # of shared objects names */
#define SM_OBJ_MAX_TRIES        5000    /* max # of tries to obtain lock */
#define SM_OBJ_TASK_PRIORITY    40      /* tSmObjTask priority */
#define SM_OBJ_TASK_OPTIONS     0       /* tSmObjTask task options */

/* shared memory network parameters  - defaults to values DEFAULT_PKTS_SIZE
 * and DEFAULT_CPUS_MAX in smPktLib.h respectively
 */

#define SM_PKTS_SIZE            0          /* shared memory packet size */
#define SM_MAX_PKTS           200          /* shared memory max input packets */
#define SM_CPUS_MAX             DEFAULT_CPUS_MAX /* SM max # of CPUs in sys   */

/* low memory layout */

#if     (CPU_FAMILY == I80X86)
#define GDT_BASE_OFFSET         0x1000
#define SM_ANCHOR_OFFSET        0x1100
#define BOOT_LINE_OFFSET        0x1200
#define EXC_MSG_OFFSET          0x1300
#define MPAPIC_DATA_OFFSET      0x2000
#elif   (CPU_FAMILY == PPC)
#define SM_ANCHOR_OFFSET        0x4100
#define BOOT_LINE_OFFSET        0x4200
#define EXC_MSG_OFFSET          0x4300
#elif   (CPU==SH7750 || CPU==SH7729 || CPU==SH7700)
#define SM_ANCHOR_OFFSET        0x1600  /* not used for SDRAM config. */
#define BOOT_LINE_OFFSET        0x1700
#define EXC_MSG_OFFSET          0x1800
#elif   (CPU_FAMILY == ARM)
#define SM_ANCHOR_OFFSET        0x1000
#define BOOT_LINE_OFFSET        0x1100
#define EXC_MSG_OFFSET          0x1200
#elif   (CPU_FAMILY == COLDFIRE)
#define SM_ANCHOR_OFFSET        0x1000
#define BOOT_LINE_OFFSET        0x1100
#define EXC_MSG_OFFSET          0x1200
#else
#define SM_ANCHOR_OFFSET        0x600
#define BOOT_LINE_OFFSET        0x700
#define EXC_MSG_OFFSET          0x800
#endif  /* (CPU_FAMILY == I80X86) */

/* The backplane driver onboard anchor at the following address */
#define SM_BASE_ADRS LOCAL_MEM_LOCAL_ADRS

#if (CPU_FAMILY==MIPS)
#define SM_ANCHOR_ADRS  ((char *) \
			 KM_TO_K0((SM_BASE_ADRS + SM_ANCHOR_OFFSET)))
#else /* CPU_FAMILY==MIPS */
#define SM_ANCHOR_ADRS  ((char *) (SM_BASE_ADRS + SM_ANCHOR_OFFSET))
#endif  /* CPU_FAMILY==MIPS */


/* The bootroms put the boot line at the following address */

#if (CPU_FAMILY==MIPS)
#define BOOT_LINE_ADRS  ((char *) \
			 KM_TO_K0((LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET)))
#else /* CPU_FAMILY==MIPS */
#define BOOT_LINE_ADRS  ((char *) (LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET))
#endif  /* CPU_FAMILY==MIPS */

#define BOOT_LINE_SIZE  255     /* use 255 bytes for bootline */

/* The boot line is stored in non-volatile RAM at the following offset */

#define NV_BOOT_OFFSET  0       /* store the boot line at start of NVRAM */


/* Messages from exceptions during exceptions go at the following address */

#if (CPU_FAMILY==MIPS)
#define EXC_MSG_ADRS    ((char *) \
			 KM_TO_K0((LOCAL_MEM_LOCAL_ADRS+EXC_MSG_OFFSET)))
#else
#define EXC_MSG_ADRS    ((char *) (LOCAL_MEM_LOCAL_ADRS+EXC_MSG_OFFSET))
#endif

/* BootApp defaults */

#define BOOT_DISABLE_MMU_BEFORE_ENTRY  TRUE /* disable MMU before entry */

/* Backplane H/W support */

#define SM_TAS_TYPE     SM_TAS_HARD     /* hardware supports test-and-set */

/* VxSim pass-through filesystem constants */

#define PASSFS_CACHE    TRUE            /* Enable VxSim PassFs cache */

/* Resident ROMs constants */

#if     ((CPU_FAMILY==I960) || (CPU_FAMILY==ARM) || (CPU_FAMILY==COLDFIRE))
#define STACK_SAVE      512             /* maximum stack used to preserve */
#else   /* sparc or others */
#if     ((CPU_FAMILY==SPARC) || (CPU_FAMILY==PPC))
#define STACK_SAVE      0x1000
#else	/* (CPU_FAMILY==SPARC) || (CPU_FAMILY==PPC) */
#if	(CPU_FAMILY==SH)
#define	STACK_SAVE	0x80
#else   /* all other architectures */
#define STACK_SAVE      0x40            /* maximum stack used to preserve */
#endif	/* (CPU_FAMILY==SH) */
#endif                                  /* mips cpp no elif */
#endif

#if     (CPU_FAMILY==SPARC)
#define RESERVED        0x2000          /* vector table base plus table size */
#else   /* 68000 or others */
#if     (CPU==MC68000)
#define RESERVED        0x400           /* avoid zeroing MC68302 vector table */
#else
#if     (CPU_FAMILY == PPC)
#define RESERVED        0x4400          /* avoid zeroing EXC_MSG */
#else   /* all other architectures */
#define RESERVED        0
#endif
#endif                                  /* mips cpp no elif */
#endif

#if     ((CPU_FAMILY == MIPS) || (CPU_FAMILY == PPC))
#define STACK_RESIDENT  RAM_DST_ADRS
#else
#define STACK_RESIDENT  _sdata
#endif

#if     (_STACK_DIR == _STACK_GROWS_DOWN)

#ifdef  ROM_RESIDENT
#define STACK_ADRS      STACK_RESIDENT
#else
#define STACK_ADRS      _romInit
#endif  /* ROM_RESIDENT */

#else   /* _STACK_DIR == _STACK_GROWS_UP */

#ifdef  ROM_RESIDENT
#define STACK_ADRS      (STACK_RESIDENT-STACK_SAVE)
#else
#define STACK_ADRS      (_romInit-STACK_SAVE)
#endif  /*  ROM_RESIDENT */

#endif  /* _STACK_DIR == _STACK_GROWS_UP */


/* Default Boot Parameters */

#define HOST_NAME_DEFAULT       "bootHost"      /* host name */
#define TARGET_NAME_DEFAULT     "vxTarget"      /* target name (tn) */
#define HOST_USER_DEFAULT       "target"        /* user (u) */
#define HOST_PASSWORD_DEFAULT   ""              /* password */
#define SCRIPT_DEFAULT          ""              /* startup script (s) */
#define OTHER_DEFAULT           ""              /* other (o) */


/* Login security initial user name and password.
 * Use vxencrypt on host to find encrypted password.
 * Default password provided here is "password".
 */

#ifndef LOGIN_USER_NAME
#define LOGIN_USER_NAME         "target"
#endif  /* LOGIN_USER_NAME */

#ifndef LOGIN_PASSWORD
/*"password"*/
#define LOGIN_PASSWORD "XohImNooBHFR0OVvjcYpJ3NgPQ1qq73WKhHvch0VQtg="
#endif  /* LOGIN_PASSWORD */


/* install environment variable task create/delete hooks */

#ifdef  INCLUDE_ENV_VARS
#define ENV_VAR_USE_HOOKS       TRUE
#endif  /* INCLUDE_ENV_VARS */

/* system symbol table configuration parameter */

#ifndef STORE_ABS_SYMBOLS
#define STORE_ABS_SYMBOLS       FALSE
#endif /* ndef STORE_ABS_SYMBOLS */

/*
 * Architecture specific VM page size. For more information see the
 * respective Architecture Supplement.
 */

#if     ((CPU_FAMILY == PPC) || (CPU_FAMILY == ARM) || (CPU_FAMILY == SH) || \
	 (CPU_FAMILY == I80X86))
#   define VM_PAGE_SIZE         4096
#elif   (CPU_FAMILY == MIPS)
#   ifdef MMU_PAGE_SIZE
#       define VM_PAGE_SIZE	(MMU_PAGE_SIZE)
#   else
#       define VM_PAGE_SIZE	0x2000
#   endif /* MMU_PAGE_SIZE */
#elif   ((CPU_FAMILY ==SIMSPARCSOLARIS) || (CPU_FAMILY==SIMLINUX))
#   define VM_PAGE_SIZE         0x2000  /* default page size */
#elif   (CPU_FAMILY==SIMNT)
#   define VM_PAGE_SIZE         0x10000 /* default page size */
#else
#   define VM_PAGE_SIZE         8192
#endif  /* (CPU_FAMILY == PPC || ARM || SH || I80X86) */

#if (CPU_FAMILY != MIPS) && (CPU_FAMILY != COLDFIRE)
#define SW_MMU_ENABLE   FALSE   /* software MMU simulation disabled by default*/
#else /* CPU_FAMILY != MIPS */
#ifndef CDF_OVERRIDE
#define SW_MMU_ENABLE   TRUE    /* software MMU simulation enabled by default*/
#endif /* CDF_OVERRIDE */
#endif /* CPU_FAMILY != MIPS */

#if ((CPU == PPC603) || (CPU == PPCEC603) || (CPU == PPC604) || (CPU == PPC970))
    /* The below 2 macros are valid only for PPC60X and PPC970. */
#define USER_HASH_TABLE_ADDRESS      NULL   /* Let the OS decide the address. */
#define USER_HASH_TABLE_SIZE         0x0    /* Let the OS decide the size     */
#endif

/******************************************************************************/
/*                                                                            */
/*                         INCLUDED NETWORK FACILITIES                        */
/*                                                                            */
/******************************************************************************/
#include "configAllNetwork.h"   /* VxWorks network configuration header */

/* PCI device configuration type constants */

#define PCI_CFG_FORCE 0
#define PCI_CFG_AUTO  1
#define PCI_CFG_NONE  2

/* for backward compatibility with old 1.0 BSPs */

#ifndef BSP_VERSION
#   define BSP_VERSION  "1.0"   /* old 1.0 style BSP */
#   define BSP_VER_1_0  TRUE
#endif

#ifndef BSP_REV
#   define BSP_REV      "/0"    /* old 1.0 style BSP */
#endif

#endif  /* INCconfigAllh */
