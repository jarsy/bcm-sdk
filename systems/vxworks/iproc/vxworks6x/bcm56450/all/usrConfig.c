/* usrConfig.c - user-defined system configuration library */

/*
 * Copyright (c) 1984-2012 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
modification history
--------------------
30y,10feb12,ljg  Change initialization order of USB Bulk and Network 
                 (WIND00322792)
30x,10jan12,zl   initialize sysExcMsg string (WIND00318907).
30w,21jul11,sye  add ATA direct BIO support. (WIND00217466)
30v,20jul11,zly  WIND00217466: add a direct mode for xbd.
30u,24jun10,pmr  support extended vectors in bootrom (PPC)
30t,26feb10,kk   remove duplicate calls to envLibInit() (WIND00201819)
30s,18feb10,d_c  Defect WIND00200672 Fix WIND00200673 - add prototype for
                 vxCpuLibInit()
30r,26jan10,d_c  SMP M-N updates: Add call to vxCpuLibInit().
30q,08jan10,s_s  Calling envLibInit() soon after initializing the memory in
                 usrRoot() (defect CQ:WIND00194940)
30p,23oct09,s_s  added kprintf defines to support BSP build (WIND00186733)
30o,15oct09,hui  Updated call to usrHrfsInit() for remove commit policy stuff
30n,21sep09,v_r  Now including loginLibP.h (part of def CQ:WIND00155318 fix).
30m,08sep09,y_l  Fix compile error(usb macro definition error) (WIND00180571).
30l,14aug09,jpb  Defect# 161788. Removed setLib references.
30k,14aug09,hps  update dosfs cache name
30j,06aug09,zly  Added initialization code for TRFS(WIND00108098)
30i,28jul09,e_d  Removed the name detection for ATA device.(WIND00139792)
30h,02jul09,s_z  Remove GEN2 USB-Ethernet hardware related initialization 
                 entries.
30g,21apr09,jpb  Replaced setLib.h with setLibP.h.
30f,05mar09,s_z  Added USB GEN2 Usb-Ethernet class driver support
30g,17mar09,w_x  Added INCLUDE_SER_EMULATOR_INIT for USB CDC serial emulation
30f,05feb09,pgh  Remove unused bootapp components.
30e,16jan09,w_x  Added USB GEN2 storage class driver support
30d,13nov08,j_x  USB GEN2 class driver support
30c,29oct08,spw  Add INCLUDE_BOOT_DHCPC initialization
30b,24oct08,pgh  Change bootUsbFsLoadInit() API.
30a,02oct08,rec  add sysHw.c support
29z,30sep08,rec  add INCLUDE_USR_MPAPIC initialization
29y,25sep08,v_r  Decoupled tip from shell and network (def CQ:WIND00134830).
29x,23sep08,jmp  Added the max number of tasks that can be spied to
                 spyLibInit() (CQ:WIND00106559).
29w,19sep08,rec  remove EFK specific initialization
29v,11sep08,v_r  Added tip library support (req CQ:WIND00114857).
29u,08sep08,rbc  Fix BOOTAPP build issue.
29t,10jun08,tcr  update Windview dependencies for eventpoints
29s,28aug08,slk  addd SDA initialization for MIPS
29r,27aug08,jpb  Removed SMP specific code.  Not used in bsp builds.
29q,22aug08,rbc  Changes for passing EFI memory map and ACPI table pointer.
29p,20aug08,d_l  break default mac(ENET_DEFAULT) to three bytes.(WIND00121398)
29o,06aug08,rbc  Change INCLUDE_EFK_DEBUG to INCLUDE_EFI_ACPI_DEBUG
29n,29jul08,dbt  Updated WDB proxy support. Added WDB MIPC support.
29m,23jul08,scm  correct EFK DEBUG init...
29l,08jul08,rec  move ACPI initialization to earlier phase of bootup
29k,03jul08,scm  move efk initialization...
29j,02jul08,scm  modify acpi initialization...
29i,16jun08,rec  two-stage ACPI initialization
29h,09jun08,s_z  Reorder initialization of USB Keyboard (Defect WIND00119420)
29g,04jun08,jmp  Added INCLUDE_CORE_DUMP_RTP (REQ CQ:WIND00005075).
29f,03jun08,rbc  Adding UEFI Support.
29e,30may08,rec  add ACPI support
29d,04mar08,pgh  Move bootAddMemInit() before sysHwInit2().
29c,26feb08,v_r  Added the RTP debug command facility (req CQ:WIND00004496).
29b,09jan08,pcs  Move initialization of the edr lib to before usrTextProtect
                 (WIND00057363).
29a,14oct07,dlk  Added INCLUDE_VXBUS_END_QCTRL utility.
28z,28sep07,dbt  Added support for (hidden) INCLUDE_HASH component
                 (CQ:WIND00105203).
28y,04sep07,pch  CQ102745: kernelInit params now in a structure
28x,28aug07,act  io footprint work
28w,27aug07,agf  Update dosFsCacheLibInit argument list
28v,27aug07,ebh  Reorder initialization of SNS (Defect WIND00034610)
28u,12jul07,dcc  Added MIPS SDA initialization, although SDA is not enabled yet.
28t,15aug07,mmi  use _WRS_ARCH_HAS_CPU_PWR_MGMT around power management code
28s,23jul07,jrp  Moving usbInit under VXBUS conditional
28r,26jul07,v_r  Renamed INCLUDE_SHELL_SAVE_LOAD_HISTORY to
                 INCLUDE_SHELL_HISTORY_FILE.
28q,26jul07,v_r  Added shell persistent history facility init routine (req
                 CQ:WIND00005098).
28p,20jul07,kk   clean up unused code
28o,19jul07,jmp  added INCLUDE_VXDBG_CPU_CONTROL initialization
28n,17jul07,kk   clean up for SMP
28m,16jul07,jrp  Moving vxBus USB initializations to vxbUsrCmdLine.c
		 (CQ:WIND00099255).
28l,02jul07,tor  SMP route configured interrupts to each CPU as it starts
28k,19jun07,tor  add vxIpiLib support
28j,28may07,vik  added vxbUsrCmdLine.c
28i,18jan07,dcc  fixed defect WIND0070704:"xbdBlkDevCreate creates a task with
		 hardcoded priority."
28h,01jun06,jlk  Updated call to usrHrfsInit() for commit policy stuff
28g,09may07,v_r  Call the module hook library static initialization routine.
28f,30apr07,dcc  Added PPC SDA initialization in usrInit().
28e,01may07,kk   added timerShowInit() for INCLUDE_POSIX_TIMER_SHOW
28d,04apr07,v_r  Call the shell static initialization routine (req
                 CQ:WIND00045277).
		 Make use of already existing usrBanner() code.
28c,05feb07,jrp  Nortel Headset Integration
28b,02feb07,ami  vxbus related changes for usb initialization
28a,22mar07,jmt  Initialize usrBootLineInit for bootrom when INCLUDE_NETWORK
                 is not defined.
27z,28feb07,jmt  Add INCLUDE_BOOT_EDR_SUPPORT component
27y,24feb07,jmt  Fix problem with ATA support in bootroms
27x,05feb07,jrp  USB Headset Integration
27w,05mar07,kk   fix error msg to indicate called from usrRoot()
27v,23feb07,h_k  added INCLUDE_ISR_DEFER.
27u,12feb07,jmg  Added poolLibInit to usrRoot()if INCLUDE_POOL is defined.
27t,31jan07,pch  add SMP time base synchronization
27s,24jan07,jmg  Added VX_GLOBAL_NO_STACK_FILL to usrKernelInit()
                 (Req.WIND00083667)
27r,23jan07,scm  replaced VX_SMP_NUM_CPUS  with vxCpuConfiguredGet...
27q,02feb07,ami  vxbus related changes for usb initialization
27p,16feb07,jmt  Move INCLUDE_BOOT_ADDMEM earlier in init sequence
27o,08feb07,jmt  Add Filesystem boot support
27n,23jan07,jmt  Add BootApp USB components
27m,28nov06,h_k  added INCLUDE_BOOT_ETH_ADR_SET. (CQ:81271)
27l,08dec06,scm  add IA32 CPC support...
27k,28nov06,dbt  Added Cross Processor Calls initialization support on VxSim.
27j,27nov06,lei  activate all idle tasks in usrRoot
27i,24nov06,cjj  removed declaration of kernelCpuEnable
27h,18nov06,pcs  Add enabling of additional core for SMP.
27g,17nov06,jln  remove debug message
27f,15nov06,jmt  Defect 81462: Update BootApp init function
27e,10nov06,dbt  Added support for __thread variables (TLS library).
27d,09nov06,rlp  Fixed mistake in previous modification.
27c,09nov06,rlp  Renamed dbgRtpInit to vxdbgRtpLibInit (REQ00007655).
27b,08nov06,lei  activated idle task via kernelIdleTaskActivate() to
		 ensure that idle task has a valid MMU context.
27a,24oct06,jmt  Add INCLUDE_BOOT_ETH_MAC_HANDLER
26z,17oct06,gls  made maxIsrJobs argument a UINT (Defect #00059580)
26y,17oct06,lei  activated idle task on bootstrap core in usrRoot();
26x,04oct06,pes  Add cpcInit for MIPS.
26w,05sep06,tcr  add initialization for INCLUDE_POSIX_TRACE_TIMESTAMP
26v,05sep06,tcr  add initialization for INCLUDE_POSIX_TRACE_TIMESTAMP
26u,16aug06,jmt  Add bootApp components
26t,28jul06,lei  activated cpcInit for PPC.
26s,17jul06,ggz  added initialization of TIPC WV instrumentation
26r,15jul06,kch  Added configNetParams.h include statement. Also replaced
                 INCLUDE_NET_MICRO_STACK with INCLUDE_COMMON_NET.
26q,10jul06,mmi  moved cpcInit() from usrInit() to usrRoot()
26p,10jul06,gls  added sigeventLibInit()
26o,07jul06,pcs  update call to fn memRtpLibInit with 2nd parameter as
		 MEM_PART_DEFAULT_OPTIONS
26n,06jul06,jln  added thread SPORADIC scheduling support
26m,04jul06,tcr  add INCLUDE_POSIX_TRACE
26l,20jun06,mmi  add cpcInit() to init Cross-Process Call libraries
26k,16jun06,zl   added INCLUDE_MEM_MGR_INFO support.
26j,30may06,tkf  Remove usrBootLineGet() routine.
26i,19may06,tkf  Add usrBootLineGet() routine.
26h,05may06,tcr  Ensure rBuffMgr priority set before calling rBuffLibInit()
26g,19apr06,dcc  fixed SPR 120265
26f,07apr06,dgp  doc: fix SEE ALSO references for library
26e,23feb06,jlk  added initialization of POSIX advisory file locking component
26d,14feb06,jln  added initialization of thread CPU-time clock
26c,13feb06,bwa  removed references to vxfusion.
26b,10feb06,jlk  Updated iosInit() call to take the IOS_POSIX_PSE52_MODE
		 parameter.
26a,09feb06,gls  added posixMode parameter to sigInit()
25z,03feb06,md   SPR 117455, fix ED&R stub initialialisation
25y,20jan06,pad  Updated call to rtpLibInit() to pass the maximum number of
		 file descriptors.
25x,17jan06,bpn  Simplified the initialization of the system symbol table.
25w,10jan06,agf  moving symbols from sysLib to here so as not to generate
		 warning with sysComms in prj builds (SPR 114346)
25v,10jan06,zl   added INCLUDE_MAPPED_FILES_SHOW support.
25u,21dec05,rfr  Added INCLUDE_VRFS
25t,20dec05,pcm  added parameter to cdromFsInit() call
25s,26oct05,zl   added POSIX shared memory objects and memory mapped files.
25r,09oct05,mmi  added CPU power management initialization.
25q,28sep05,pcm  re-added usrFdConfig() as it was got lost
25p,21sep05,pcm  added calls to usrFdConfig() after fdDrv()
25o,15sep05,jln  Added taskPxLib initialization
25n,14sep05,ami  Initialization of USB Stack moved after the Initialization of
		 Network Stack (SPR #111676)
25m,08sep05,dcc  removed INCLUDE_TAPEFS component (SPR# 111788).
25l,08aug05,dcc  added dosFs cache initialization.
25k,16jul05,jln  Added INCLUDE_POSIX_PTHREAD_SCEHD initialization
25j,12jul05,mil  Added memAllotLibInit for minimal kernel.
25i,08jul05,mil  Conditionalized out memPartLibInit().
25h,04aug05,dcc  added an extra parameter to usrDosfsInit().
25g,04aug05,aeg  added inclusion of cdromFsLib.h
25f,03aug05,pcm  added parameters to usrDosfsInit()
25e,28jul05,pcm  simplified initialization of DosFs
25d,25jul05,jmp  added usrCoreDumpInit().
25c,19jul05,pcm  added INCLUDE_XBD_BLK_DEV
25e,24aug05,mmi  Add CPU Power Magmt. FrameWork
25m,15aug05,h_k  added INCLUDE_MMU_GLOBAL_MAP.
25l,05aug05,jln  removed configuration of POSIX scheduler
25k,04aug05,wap  Changes for microstack
25j,20jul05,pcm  added INCLUDE_XBD_BLK_DEV
25i,16jul05,jln  Added INCLUDE_POSIX_PTHREAD_SCEHD initialization
25h,12jul05,mil  Added memAllotLibInit for minimal kernel.
25g,08jul05,mil  Conditionalized out memPartLibInit().
24f,23jun05,jlk  Fixed up fs event lib
24e,23jun05,jlk  Added fsPathWaitLibInit(). Removed inclusion of copyright
		 header file
24d,17jun05,pcm  used configlette init routines for dosfs & hrfs
24c,13jun05,pcm  added new component init routines
25b,06jun05,dbt  Added support for Wind River Agent Proxy.
25a,16may05,ebh  update for distributed SNS services
24z,22apr05,dr   SPR# 108100 - added call to aimFppLibInit.
24y,31mar05,hch  change the INCLUDE_USB_AUDIO_DEMO typo
24x,23mar05,dcc  added cdromFsInit() to usrRoot().
24w,21mar05,pcm  added tapeFsInit() to usrRoot()
24v,18mar05,yvp  Scalability mods.
24u,15dec04,md   Moved ED&R stub init code to below clock init code
24t,25feb05,pcs  Modified to reflect the change to the prototype of fn.
		 rtpLibInit
24s,25feb05,h_k  removed prototype declaration for aimMmuShowInit(). (SPR
		 #106193)
24r,07feb05,zl   enable text protection early
24q,08feb05,pch  SPR 102772: add INCLUDE_PROTECT_INTERRUPT_STACK
24o,01feb05,gls  added usrKernelCoreInit() (SPR #102214)
24n,22nov04,zl   test KERNEL_HEAP_SIZE parameter against sysMemTop().
24m,15nov04,yvp  Changed prototype for usrRtpAppInitString().
24l,11nov04,job  Moved location of system call initialization routines
24k,01nov04,slk  SPR92598: Add _WRS_OSM_INIT define for call to excOsmInit
		 add MIPS support for excOsmInit.
24j,01nov04,slk  Remove MIPS from architectures calling excOsmInit
24i,29oct04,slk  SPR 92598: Add MIPS to valid CPUs that can call excOsmInit()
24h,28oct04,pch  SPR 92598: add excOsmInit (exception stack overflow)
		 add excIntNestLogInit (LOG_FRAMELESS and LOG_DISPATCH support)
24g,27oct04,rec  fix power management compilation warning
24f,26oct04,dtr  Remove gnu compiler warning for IMPORT aimMmuShowInit.
24e,26oct04,fr   added msgQOpenInit() and timerOpenInit() (SPR 101491)
24d,19oct04,zl   call usrTextProtect() instead of vmTextProtect().
24c,15oct04,pad  Swapped order of calls to rtpHookLibInit() and rtpLibInit():
		 rtpHookLibInit() must come first (SPR #102595).
24b,13oct04,dlk  Added NBIO_LOG.
24a,12oct04,rec  fix merge problem
23z,20sep04,ami  NET2280 tested on High Speed
23r,16aug04,pdg  Fix for long booting time of OHCI and EHCI
23q,09aug04,hch  added USB peripheral keyboard and mouse initialization
		 moved the pegasus driver initialization after usrNetInit
23p,04aug04,hch  add USB peripheral mass storage driver initialization
23o,29sep04,dtr  Add AIM MMU Show init. SPR 101412.
23n,21sep04,mig  Clear bss segment before cacheLibInit().
23m,23sep04,ann  merge from networking: Remove obsolete sysctl refs,
23y,06oct04,zl   corrected location of vmShowShellCmdInit() prototype
23x,02oct04,gls  sdLibInit() and sdShowInit() now declared in sdLibP.h
23w,01oct04,zl   added memory management shell commands.
23w,01oct04,aeg  moved msgQLibInit/msgQShowInit out of usrKernelInit.
23v,30sep04,tam  removed INCLUDE_EDR_KH; added INCLUDE_PROTECT_TASK_STACK and
		 INCLUDE_TASK_STACK_NO_EXEC
23u,23sep04,kk   added rtpShowShellInit() initialization
23x,29sep04,dbt  Make use of usrWdbBanner.c configlette. wdbConfig() is
		 replaced by usrWdbInit().
23w,23sep04,md   fix ED&R debug flag message
23v,23sep04,ann  merge from networking: Remove obsolete sysctl refs,
		 ftp login and security changes, SPR #100002.
23j,08sep04,jyo  Fixing SPR 100119: Removing references to RT11.
23u,24sep04,yvp  Fix SPR 99907: Stop calling ioGlobalStdSet in pre-kernel code.
23u,23sep04,jn   Add configuration parameter for system symbol table
23t,18sep04,jn   Add initialization routine for unldLib
23s,17sep04,jmp  made passFs cache configureable.
23r,09sep04,md   add carriage return to ED&R message
23q,13sep04,zl   corrected MM private headers.
23p,09sep04,bpn  Added shell command init routines.
23o,07sep04,yvp  rtpHookLibInit() prototype change.
23n,03sep04,ans  added INCLUDE_JOB_TASK and updated excInit () parameter list
23m,01sep04,jmp  removed simulator specific code.
23l,27aug04,gls  added SD_HOOK_TBL_SIZE parameter to sdLibInit()
23l,17sep04,sru  Add sysTextProtect global; undef MIPS vmTextProtect().
23k,24aug04,job  Changed call of posixScInit() to usrPosixScInit()
23j,28jul04,md   removed INCLUDE_EDR_SYMBOLIC, INCLUDE_SC_EDR; SPR78837
23j,15sep04,dtr  Remove usrSpeInit call with COPROCLIB.
23i,06jul04,dtr  Add in call to usrMmuOptimize and usrAimMmuConfig.
23h,07jul04,c_c  Get rid of romfs_image.h
23g,29jun04,job  INCLUDE_SYSCTL_HW inclusion dependent on INCLUDE_SYSCTL
23f,28jun04,job  changed INCLUDE_SYSCTL_API to INCLUDE_SYSCTL
23e,14jun04,job  Renamed usrSysctl2 to usrSysctl
23d,11jun04,dcc  added clockLibInit() call.
23c,09jun04,zl   removed references to vxVMI (INCLUDE_MMU_FULL).
23g,01jun04,mem  added call to palInit().
23t,14may04,syy  Remove DHCPv6 initilization code
23s,14may04,kk   added shlShellCmdInit() call
23r,08jun04,elg  Add ability to start WDB before kernel.
23q,02jun04,bpn  Register shell commands for the loader and the unloader.
23p,06may04,md   include cmd shell ED&R routines using INCLUDE_EDR_SHOW
23o,20may04,gls  renamed mmScLibInit to mmanScLibInit
23n,18may04,yvp  Added INCLUDE_RTP_APPL_INIT_CMD_SHELL_SCRIPT.
23m,18may04,bpn  Added call to usrRtpLibInit().
23l,06may04,md   include cmd shell ED&R routines using INCLUDE_EDR_SHOW
23k,03may04,kk   added INCLUDE_SHL_SHOW
23j,30apr04,fr   added edrShellCmdInit call
23i,02apr04,dms  Added INCLUDE_SHL.
23h,18mar04,md   added procfs component specific init calls
23g,14apr04,syy  Added DHCP6 client initialization
23f,07may04,yvp  Added support for various RTP Startup facility components.
23e,30apr04,kk  enable RTP_SHOW & SD_SHOW to be included standalone
23d,28apr04,zl   added memEdrRtpShowInit()
23c,27apr04,tam  fixed prototype of memRtpLibInit()
23b,27apr04,bpn  Replace loginPrompt() with loginPrompt2().
23a,21apr04,dat  move selectInit after iosInit, before tyLibInit (96540)
22z,14apr04,syy  Added DHCP6 client initialization
22y,14apr04,pch  SPR 95355: enable interrupts in root task after MMU is set up.
		 Fix warnings re constant conditionals involving NUM_TTY.
22x,09apr04,gls  added sdShowInit()
22w,06apr04,yp   reworking adrSpaceLib initialization
22v,06apr04,job  Pass TASK_STACK_OVERFLOW_SIZE and TASK_STACK_UNDERFLOW_SIZE
		 to rtpLibInit()
22u,06apr04,jn   Fix build warning in bsp builds.  Move loader related
		 includes here from usrConfig.h.
22t,01apr04,dat  adding INCLUDE_MEMDRV, (93390), INCLUDE_PTYDRV (6294)
22s,18mar04,dat  adding support for poolLib, poolShow, setLib
22r,10mar04,dbt  vi editing mode should be the default for the shell.
22q,23feb04,tcr  fix use of INCLUDE_TIMESTAMP
22p,23feb04,asr  Added protection around loginUserAdd
22o,20feb04,vvv  made changes for socket/sysctl separation from stack
22n,20feb04,yvp  Added RTP startup facility (INCLUDE_RTP_APPL).
22m,18feb04,ans  Added RTP_SIGNAL_QUEUE_SIZE for RTP initialization.
22l,11feb04,mil  Fixed shell not enabling SPE task option.
22k,18feb04,md   added INCLUDE_SC_EDR init code (ED&R syscall)
22j,03feb04,dat  chg init order for POSIX_AIO
22i,26jan04,elg  Add initialization of WDB hooks.
22h,26jan04,zl   added memEdrLib configuration.
22g,23jan04,syy  Eliminate compile warnings on wvNetDInit()
22f,16jan04,syy  Added wvNetDInit() to usrRoot()
22e,09jan04,als  correct pathname for SNS server RTP initialization (SPR#92606)
22d,08jan04,md   fix procfsFsDevCreate() prototype (SPR #92703)
22c,15dec03,dat  Copyright update to 2004
22b,08oct03,tcr  fix WindView timestamp inclusion for bsp builds
22a,04sep03,tcr  update WindView upload paths
21z,12dec03,yvp  Changed syscall.c to syscallTbl.h.
21y,09dec03,yvp  Added #include of syscall.c
21x,08dec03,m_s  included posix header files
21w,07dec03,md   moved procfs init code.
21v,05dec03,zl   added call of memRtpLibInit().
21u,04dec03,ans  Corrected #ifdef INCLUDE_POSIX_MMAN to #ifdef INCLUDE_SC_POSIX.
21t,02dec03,dbs  fix call to edrSystemDebugModeGet()
21s,24nov03,p_r  added SAL components
21r,18nov03,yp   code review changes
21q,12nov03,yp   adding adrSpaceShow
21p,04nov03,kk   added shared data initialization
21o,29oct03,gls  added pgMgrShowInit()
21n,23oct03,tam  fixed initialization of memPoolStartArs
21m,21oct03,dat  added IO system call component
21l,14oct03,gls  added INCLUDE_PG_MGR_LIB
21k,19nov03,cjj  Removed duplicate call to rtpLibInit().
21j,17nov03,yvp  Changed rtpLibInit() call to use new prototype.
21i,05nov03,cfc  USB2 Development Integration
21h,10oct03,md   Add new procfs init parameter
21g,08oct03,ans  Added timerLibInit.
21f,06oct03,dbs  add EDR KH support
21e,05oct03,kk   moved RTP initialize earlier, after pgPoolShowInit()
21d,25sep03,jkf  fixing comment error on endif
21c,22sep03,bpn  Modified shell initialization. Added command support for
		 RTP and DISK_UTIL components.
21b,17sep03,kk   moved RTP initialization earlier.
21a,16sep03,zl   added INCLUDE_PROCFS
20z,12sep03,dbs  move ED&R init as early as possible
20y,11sep03,dbs  move edr init diagnosis into configlette
20x,10sep03,dbs  use edrStub to inject boot event
20w,09sep03,zl   added INCLUDE_SC_MMAN initialization.
20v,22aug03,dcc  added INCLUDE_SC_KERNEL initialization.
20u,29aug03,dbs  add diagnostic if ED&R init fails
20t,26aug03,dbs  added ED&R features
20s,18aug03,job  Add RTP show routine support
20r,16aug03,yp  fixing pgPoolLib header
20q,11aug03,yp  adding page pool library support
20p,06aug03,kk   add RTP support
20o,24jul03,jeg  added VxSim virtualDisk support
20n,22jul03,jmp  modified to share usrPassFsInit() and usrDosDiskInit() code
		 with comps/src.
20m,02jul03,md   added INCLUDE_ROMFS
20l,27jun03,tam  enabled text & vector table write-protection with MMU_BASIC
20k,29may03,rec  Power Management
20j,14may03,jmp  initialized passFs directory to CWD instead of BSP dir.
20i,23apr03,jmp  made passFsInit code common to all simulators.
20h,22apr03,dbt  Added SIMLINUX support.
20g,10apr03,pes  PAL Coprocessor Abstraction
20f,09apr03,bpn  Modified the shell initialisation to use the configlet.
20e,31mar03,bpn  Fixed typo error.
20d,26mar03,jeg  updated to use passFs for simsolaris and simnt
20c,20mar03,bpn  Changed ledRegister() to ledModeRegister().
20b,17mar03,lcs  Modify ledLib initialization code.
20a,20feb03,jmp  cleanup simulator code.
19z,17feb03,bpn  Reworked shell initialization code.
19y,20jun03,t_m  added pccardlib
19x,12nov02,dbt  Reworked symbol table synchronization initialization code
		 (SPR #25509).
19w,15may02,sbs  changing default sys clock rate to a macro (SPR #1918)
19v,24apr02,j_s  make WDB Banner the same as in
		 target/config/comps/src/usrWdbBanner.c (SPR #76099)
		 updated 2001 to 2002
19u,19apr02,pch  SPR 74348: Enable PPC Machine Check exception ASAP
19t,05apr02,hbh  Fixed SPR# 30368 and improved ntPassFs initialization.
19s,04mar02,pch  SPR 73609
19r,11feb02,jkf  SPR#73173, adding INCLUDE_DOSFS_MAIN init section.
19q,21jan02,sn   added INCLUDE_CTORS_DTORS
19p,10dec01,jlb  Change UGL to WindML product
19o,07dec01,wef  add USB initialization code.
19n,07dec01,sn   added INCLUDE_CPLUS_DEMANGLER
19m,08nov01,jmp  removed useless routeAdd() for SIMNT.
19l,31oct01,gls  added pthread code
19k,27oct01,dat  Adding trap for misaligned data section SPR 9006
19j,21sep01,aeg  updated initialization sequence of selectLib (SPR #31319).
19i,17sep01,pcm  added excShowInit() (SPR 7333), and lstLibInit () (SPR 20698)
19h,15may01,pch  Move call of usrAltivecInit() from sysLib.c to usrConfig.c,
		 to fix an undefined reference when building bootrom.
19g,20jan99,dbs  change VXCOM to COM
19f,18dec98,dbs  add VXCOM as separate item
19e,06nov98,dbs  add DCOM init when INCLUDE_DCOM is set
19g,22feb01,scm  add fast dram support capabilities for XScale...
19f,16mar99,jpd  changed copyright yearfrom 1998 to 1999.
19e,13nov98,jpd  call usrMmuInit if INCLUDE_MMU_MPU defined; conditionally
		 add sysHwInit0() call before cacheLibInit(); added
		 INCLUDE_INITIAL_MEM_ALLOCATION
	    cdp	 make Thumb support for ARM CPUs dependent on ARM_THUMB.
19j,27feb01,hk   update INCLUDE_WDB_BANNER copyright notice to cover 2001.
19i,08jun00,csi  Merginf usrConfig.c for vxfusion
19h,19apr00,zl   updated copyright year to 2000.
19g,02mar00,zl   merged SH support into T2.
01g,07mar01,sn   simplify job of host val by using creationDate (which
		 is overridable) rather than __DATE__
19g,14oct99,jk   added conditional init code for sound driver support.
19i,30jul99,jkf  fixed SPR#4429. ideDrv, ataDrv, fdDrv, warn on ERROR.
19h,16mar99,jpd  changed copyright yearfrom 1998 to 1999.
19g,10mar99,drm  Changing WINDMP to VXFUSION to reflect new product name.
19f,13nov98,jpd  call usrMmuInit if INCLUDE_MMU_MPU defined; conditionally
		 add sysHwInit0() call before cacheLibInit(); added
		 INCLUDE_INITIAL_MEM_ALLOCATION
	    cdp	 make Thumb support for ARM CPUs dependent on ARM_THUMB.
19e,08oct98,rlp  added support for UGL and HTML.
19d,17sep98,ms   call cplusDemanglerInit if INCLUDE_CPLUS and INCLUDE_LOADER
19c,17sep98,cym  added code to calculate proper IP address for PCSIM.
19b,01sep98,drm  added code to initialize WindMP
19a,10aug98,cym  added code to inform tornado when the NT simulator is up.
18z,23apr98,yp   merged in TrueFFS support
18y,22apr98,elp  created group 1 module when standalone (fixed SPR# 20301).
18w,22apr98,hdn  added comment for tffsDrv().
18t,17apr98,nps  added support for rBuffShow.
18s,09mar98,sjw  merge in HTTP and JAVA changes for conflict resolution
18r,18dec97,cth  replaced windview, timestamp initialization with windviewConfig
18q,13nov97,cth  changed WV evtInstInit to wvLibInit2; removed connRtnSet,
		 evtTaskInit; added include wvLib.h, rBuffLib.h
18p,18aug97,nps  changed WV buffer initialization with the rBuff API.
18x,06mar98,ms   changed copyright year from 1996 to 1998.
18v,04feb98,jpd  fix unsigned/signed warning (usrRoot); updated copyright date.
18u,25feb98,jgn  removed call to excShowInit(), it is called by excInit()
		 if needed (SPR #20625)
18t,19jan98,hdn  changed a condition macro to include tffsDrv().
18p,06jan98,cym  added SIMNT as processor not to clear bss.
18s,23sep97,yp   added support for TFFS.
18r,13aug97,cdp  add Thumb (ARM7TDMI_T) support;
		 'fix' unsigned/signed comparison in usrRoot.
18q,26feb97,dat  added INCLUDE_USER_APPL and USER_APPL_INIT, spr 8071.
18p,05feb97,cdp  remove ARM test hook (_func_armStartup).
18o,12dec96,cdp  added _func_armStartup for ARM platforms.
18n,13dec96,elp	 changed syncLibInit() into symSyncLibInit().
18m,18nov96,dds  SPR 2529: return status for sysScsiInit is now checked.
18l,06nov96,ms   removed 17n hack to usrClock.
18k,01nov96,hdn  added support for PCMCIA.
18j,21oct96,hdn  changed paramerters for new LPT driver.
18i,04oct96,elp  added INCLUDE_SYM_TBL_SYNC (SPR# 6775).
18h,11jul96,tam  changed copyright year from 1995 to 1996.
18l,19jul96,hdn  added support for ATA driver.
18k,25jun96,jmb  Eliminated alternative WDB macro names (SPR #5707).
18j,25jun96,dbt  Initialized _func_selWakeupListInit before tyLib (SPR #3314).
18i,21jun96,jmb  update copyright. SPR #6364
18h,21jun96,jmb  long modhist -- deleted entries prior to 1994.  SPR #6528
18g,12mar96,tam  removed all #if (CPU == PPC403).
18p,06jun96,ism  bumped copyright to 1996.
18o,26jan96,ism  cleaned up vxsim/solaris version
18f,02oct95,kvk  added sysHwInit2() for PPC603.
18e,27sep95,tpr  removed #if (CPU_FAMILY != PPC) staff.
18n,07nov95,srh  fixed C++ support comments.
18m,29oct95,dat  fixed warnings about printf arguments
18l,11oct95,jdi  doc: changed .pG to .tG.
18k,10oct95,dat	 new BSP revision id. Added WDB Banner printing
18j,15jun95,ms	 updated for new serial drivers.
18i,09jun95,ms	 cleaned up console initialization
18h,30may95,p_m  added initialization of formatted I/O library.
		 replaced spyStop() by spyLibInit().
18g,22may95,p_m  no longer initialize object format specific libraries if
		 loader is not included.
18f,22may95,myz  modified new serial device initialization for x86 support
18e,22may95,ms   added some WDB agent support
18e,30oct95,ism  updated for SIMSPARCSOLARIS
18d,28mar95,kkk  added scalability support, changed edata and end to arrays
		 (SPR #3917), changed baud rate to be a macro in configAll.h
18g,25may95,yao  changed to bzero bss segment for PPC.
18f,04apr95,caf  added PPC cache support.
18e,03mar95,yao  added shsHwInit2 () for PPC403.
18d,24feb95,caf  added PPC support.
18c,20nov94,kdl  added hashLibInit() when using dosFs.
18b,09nov94,jds  additions for scsi backward compatability ; scsi[12]IfInit()
18a,04nov94,kdl	 merge cleanup.
17l,02aug94,tpr  added cacheEnable (BRANCH_CACHE) for the MC68060.
17m,15oct94,hdn  added LPT driver.
17o,20jul94,ms   changed INCLUDE_AOUT_HPPA to INCLUDE_SOM_COFF
17n,02may94,ms   added VxSim HP hack to usrClock().
17q,15mar94,smb  renamed tEvtTask parameters.
17p,15feb94,smb  defined WV_MODE, EVTBUFFER_ADDRESS for wvInstInit().
17o,12jan94,kdl  modified posix initialization; added queued signal init.
*/

/*
DESCRIPTION
This library is the WRS-supplied configuration module for VxWorks.  It
contains the root task, the primary system initialization routine, the
network initialization routine, and the clock interrupt routine.

The include file config.h includes a number of system-dependent parameters used
in this file.

In an effort to simplify the presentation of the configuration of vxWorks,
this file has been split into smaller files.  These additional configuration
source files are located in '../../src/config/usr[xxx].c' and are #included
into this file below.  This file contains the bulk of the code a customer is
likely to customize.

The module usrDepend.c contains checks that guard against unsupported
configurations such as INCLUDE_NFS without INCLUDE_RPC.  The module
usrKernel.c contains the core initialization of the kernel which is rarely
customized, but provided for information.  The module usrNetwork.c now
contains all network initialization code.  Finally, the module usrExtra.c
contains the conditional inclusion of the optional packages selected in
configAll.h.

The source code necessary for the configuration selected is entirely
included in this file during compilation as part of a standard build in
the board support package.  No other make is necessary.

INCLUDE FILES:
config.h

SEE ALSO:
The VxWorks programmer guides.
*/

#include <vxWorks.h>			/* always first */
#include <config.h>			/* board support configuration header */
#include <usrConfig.h>			/* general configuration header */
#include <usrDepend.c>			/* include dependency rules */
#include <configNetParams.h>  /* header for network configuration parameters */
#include <usrKernel.c>			/* kernel configuration */
#include <usrExtra.c>			/* conditionally included packages */

#ifdef INCLUDE_HTTP
#include <http/httpLib.h>		/* Wind Web Server interfaces */
#endif

#ifdef INCLUDE_VRFS
#include <vrfsLib.h>
#endif

#ifdef INCLUDE_ROMFS
#include <romfsLib.h>			/* read-only file system interfaces */
#include <romfsDrv.h>			/* romfs block device driver */

/*
 * ROM filesystem image will come from romfs_image.o object file, which defines
 * two symbols (see below):
 */

extern const char romfsImage[];               /* The file system */
extern const char romfsImageEnd[];    /* address of its end */

int romfsImageLen;    /* size of ROM fs */

#endif

/*
 * IO system implied requirements.  INCLUDE_IO_SYSTEM is the old name
 * for "give me everything".  We now have lower-level defines for omitting
 * various pieces, but you need only say (eg) "give me FILE_SYSTEM" to
 * get everything that FILE_SYSTEM requires too.
 *
 * INCLUDE_TTY (and INCLUDE_SELECT) used to be done with INCLUDE_IO_SYSTEM
 * but INCLUDE_IO_BASIC is all that is required.
 */
#if defined(INCLUDE_TTY) && !defined(INCLUDE_IO_BASIC)
# define INCLUDE_IO_BASIC
#endif
#if defined(INCLUDE_SELECT) && !defined(INCLUDE_IO_BASIC)
# define INCLUDE_IO_BASIC
#endif

#ifdef INCLUDE_IO_SYSTEM
# ifndef INCLUDE_IO_MISC
#  define INCLUDE_IO_MISC
# endif
# if defined(INCLUDE_RTP) && !defined(INCLUDE_IO_RTP)
#  define INCLUDE_IO_RTP
# endif
#endif /* INCLUDE_IO_SYSTEM */

#if defined(INCLUDE_IO_MISC) || defined(INCLUDE_IO_RTP)
# ifndef INCLUDE_IO_POSIX
#  define INCLUDE_IO_POSIX
# endif
#endif

#if defined(INCLUDE_IO_POSIX) && !defined(INCLUDE_IO_FILE_SYSTEM)
# define INCLUDE_IO_FILE_SYSTEM
#endif
#ifdef INCLUDE_IO_FILE_SYSTEM
# ifndef INCLUDE_IO_REMOVABLE
#  define INCLUDE_IO_REMOVABLE
# endif
# ifndef INCLUDE_SET
#  define INCLUDE_SET
# endif
#endif
#if defined(INCLUDE_IO_REMOVABLE) && !defined(INCLUDE_IO_BASIC)
# define INCLUDE_IO_BASIC
#endif

#if	defined(INCLUDE_TLS)
#include <private/tlsLibP.h>
#endif	/* INCLUDE_TLS */

#if	defined(INCLUDE_PG_POOL_LIB)
#include <private/pgPoolLibP.h>
#endif /* INCLUDE_PG_POOL_LIB */

#if	defined(INCLUDE_PG_POOL_VIRT_LIB)
#include <private/pgPoolVirtLibP.h>
#endif /* INCLUDE_PG_POOL_VIRT_LIB */
#if	defined(INCLUDE_PG_POOL_PHYS_LIB)
#include <private/pgPoolPhysLibP.h>
#endif /* INCLUDE_PG_POOL_PHYS_LIB */

#if	defined(INCLUDE_PG_MGR_LIB)
#include <private/pgMgrLibP.h>
#endif /* INCLUDE_PG_MGR_LIB */

#ifdef INCLUDE_FD
extern STATUS usrFdConfig (int, int, char *);
#endif

#ifdef INCLUDE_SIGEVENT
#include <private/sigeventP.h>
#endif

#ifdef INCLUDE_RTP
#include <rtpLib.h>
#include <syscallTbl.h>
extern char * pRtpStartupField;

#ifdef INCLUDE_SHL
#include <shlLib.h>
extern STATUS shlLibInit (void);
extern STATUS shlScLibInit (void);
#endif /* INCLUDE_SHL */

#ifdef INCLUDE_POSIX_MAPPED_FILES
extern mmanFdLibInit (void);
#endif

#ifdef INCLUDE_POSIX_SHM
extern STATUS shmFsLibInit (const char * devName);
#endif

#if defined (INCLUDE_ADR_SPACE_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
extern STATUS adrSpaceShellCmdInit (void);
#endif

#if defined (INCLUDE_MAPPED_FILES_SHOW)
extern STATUS mmanShowInit (void);
extern STATUS mmanShellCmdInit (void);
#endif

#include <rtpStartupFacility.c>		/* RTP Startup Facility Components */
#endif /* INCLUDE_RTP */

#if defined (INCLUDE_VM_SHOW) || \
    defined (INCLUDE_SHOW_ROUTINES) && defined (INCLUDE_MMU_BASIC)
extern STATUS vmShowShellCmdInit (void);
#endif

#ifdef INCLUDE_SHARED_DATA
#include <private/sdLibP.h>
extern STATUS sdShowShellCmdInit (void);
#endif /* INCLUDE_SHARED_DATA */

#ifdef INCLUDE_COM
extern int comLibInit ();
#endif

#ifdef INCLUDE_DCOM
extern int dcomLibInit ();
#endif

#ifdef INCLUDE_NBIO_LOG
#include <private/nbioLogLibP.h>
#endif

#ifdef INCLUDE_SC_IOS
#include <iosScLib.h>
#endif

#ifdef INCLUDE_MEM_RTP
extern void memRtpLibInit (UINT rtpHeapSize, UINT rtpHeapOptions);
#endif

#ifdef INCLUDE_SC_KERNEL
extern STATUS windScInit (void);
#endif

#ifdef INCLUDE_SC_MMAN
extern STATUS mmanScLibInit (void);
#endif

#ifdef INCLUDE_SC_POSIX
extern void usrPosixScInit (void);
#endif

#ifdef INCLUDE_POSIX_ADVISORY_FILE_LOCKING
#include "usrPosixAdvLock.c"
#endif

#ifdef INCLUDE_SC_SYSCTL
extern STATUS sysctlScLibInit (void);
#endif

#ifdef INCLUDE_SC_SD
extern STATUS sdScInit(void);
#endif

#ifdef INCLUDE_MEM_EDR_SHOW
extern STATUS memEdrShowInit (void);
extern STATUS memEdrShellCmdInit (void);
#endif

#ifdef INCLUDE_MEM_EDR_RTP_SHOW
extern STATUS memEdrRtpShowInit (void);
extern STATUS memEdrRtpShellCmdInit (void);
#endif

#ifdef INCLUDE_POSIX_CLOCKS
extern int clockLibInit (void);
#endif

#ifdef INCLUDE_POSIX_TIMERS
extern STATUS timerLibInit (void);
#ifdef INCLUDE_OBJ_OPEN
extern void timerOpenInit (void);
#endif
#if defined(INCLUDE_POSIX_TIMER_SHOW) ||defined (INCLUDE_SHOW_ROUTINES)
extern void timerShowInit(void);
#endif
#endif

#ifdef INCLUDE_POWER_MGMT_CPU_BSP_SUPPORT
void sysCpuPwrEnable (void);
#endif
#ifdef _WRS_ARCH_HAS_CPU_PWR_MGMT
#include <cpuPwrMgr.h>
#include <cpuPwrLib.h>
#include <private/cpuPwrLibP.h>
#endif

#ifdef INCLUDE_ACPI
#include <hwif/fw/acpiLib.h>
#endif /* INCLUDE_ACPI */

#ifdef INCLUDE_POSIX_SEM
#include <private/semPxLibP.h>
#ifdef INCLUDE_SHOW_ROUTINES
#include <semPxShow.h>
#endif
#endif

#ifdef INCLUDE_POSIX_MQ
#include <private/mqPxLibP.h>
#ifdef INCLUDE_SHOW_ROUTINES
#include <mqPxShow.h>
#endif
#endif

#ifdef INCLUDE_POSIX_THREAD_CPUTIME
#include <private/pxCpuTimeLibP.h>
#endif

#ifdef INCLUDE_SHELL
#include <private/shellLibP.h>
#ifdef INCLUDE_SHELL_HISTORY_FILE
#include <private/shellHistLibP.h>
#endif
#endif

#ifdef INCLUDE_SECURITY
#include <private/loginLibP.h>
#endif

#if     defined(INCLUDE_LOADER)
#include <loadElfLib.h>
#endif

#ifdef INCLUDE_RTP_DEBUG_COMMAND
#include <private/rtpDbgCmdLibP.h>
#endif

#ifdef INCLUDE_TIP
#include <private/tipLibP.h>
#endif

#if defined (INCLUDE_LOADER_HOOKS)
#include <private/moduleHookLibP.h>
#endif

#if     defined(INCLUDE_NET_SYM_TBL)
#include <private/loadLibP.h>
#endif

#ifdef	INCLUDE_STANDALONE_SYM_TBL
#include <symbol.h>
#endif	/* INCLUDE_STANDALONE_SYM_TBL */

#ifdef INCLUDE_POOL
#include <poolLib.h>
extern STATUS poolLibInit();
#endif

#ifdef INCLUDE_MEMDRV
#include <memDrv.h>
#endif

#ifdef INCLUDE_PTYDRV
#include <ptyDrv.h>
#endif

#ifdef INCLUDE_TYLIB
#include <tyLib.h>
#endif

#ifdef INCLUDE_EXC_TASK
extern STATUS   excInit (UINT maxIsrJobs);
#endif

#ifdef INCLUDE_JOB_TASK
extern STATUS jobLibInit (int jobTaskStackSize);
#endif

#ifdef INCLUDE_ERF
extern STATUS erfLibInit (UINT16 maxUserCat, UINT16 maxUserType);
#endif

#ifdef INCLUDE_DEVICE_MANAGER
extern STATUS devInit (uint16_t ndevs);
#endif

#ifdef INCLUDE_XBD
extern STATUS xbdInit (void);
#endif

#ifdef INCLUDE_XBD_BLK_DEV
extern STATUS xbdBlkDevLibInit (int xbdServiceTskPri, BOOL directMode);
#endif

#ifdef INCLUDE_FS_MONITOR
extern STATUS fsMonitorInit (void);
#endif

#ifdef INCLUDE_FS_EVENT_UTIL
extern STATUS fsEventUtilLibInit (void);
#endif

#ifdef INCLUDE_XBD_TRANS
extern STATUS xbdTransInit (void);
#endif

#ifdef INCLUDE_HRFS_FORMAT
extern STATUS hrfsFormatLibInit (void);
#endif

#ifdef INCLUDE_DOSFS_MAIN
#include "usrDosfs.c"
#endif

#ifdef INCLUDE_USR_MPAPIC
#include "usrMpapic.c"
#endif /* INCLUDE_USR_MPAPIC */

/* BSP hardware initialization that varies for bootApp builds */
#ifdef INCLUDE_SYS_HW
#include "sysHw.c"
#endif

#if defined (INCLUDE_HRFS) || defined (INCLUDE_HRFS_READONLY)
#include "usrHrfs.c"
#endif

#if defined(INCLUDE_VXBUS) && defined(INCLUDE_VXB_CMDLINE)
    #include "vxbUsrCmdLine.c"
#endif /* usrCmdLine.c */


/* include INCLUDE_BOOT_LINE_INIT configlet for bootroms with
 * Networking disabled.
 */
#if (defined(INCLUDE_BOOT_APP) && defined(INCLUDE_BOOT_LINE_INIT))
#if (!defined(INCLUDE_NETWORK) && !defined(INCLUDE_COMMON_NET))
extern int sysStartType;

#include "net/coreip/usrBootLine.c"
#endif   /* !INCLUDE_NETWORK && !INCLUDE_COMMON_NET */
#endif   /* INCLUDE_BOOT_APP && INCLUDE_BOOT_LINE_INIT */

#ifdef INCLUDE_CDROMFS
#include "cdromFsLib.h"
#endif /* INCLUDE_CDROMFS */

#if defined (WV_INSTRUMENTATION) && defined (INCLUDE_WVNETD)
#if defined(__STDC__) || defined(__cplusplus)
extern void	wvNetDInit (void);
#else	/* __STDC__ */
extern void	wvNetDInit ();
#endif	/* __STDC__ */
#endif /* WV_INSTRUMENTATION && INCLUDE_WVNETD */

#if defined (WV_INSTRUMENTATION) && defined (INCLUDE_WVTIPC)
#if defined(__STDC__) || defined(__cplusplus)
extern void	wvTipcInit (void);
#else	/* __STDC__ */
extern void	wvTipcInit ();
#endif	/* __STDC__ */
#endif /* WV_INSTRUMENTATION && INCLUDE_WVTIPC */

#ifdef INCLUDE_POSIX_TRACE
extern STATUS pxTraceStreamLibInit (void);
#endif /* INCLUDE_POSIX_TRACE */

#ifdef INCLUDE_POSIX_TRACE_TIMESTAMP
#include <wvTmrLib.h>
#endif /* INCLUDE_POSIX_TRACE_TIMESTAMP */

#ifdef	INCLUDE_WDB_PROXY
#include <wrProxyLib.h>
#endif	/* INCLUDE_WDB_PROXY */

/* defines - must be after include of usrDepend.c */

#ifdef	INCLUDE_WDB
#define FREE_MEM_START_ADRS (FREE_RAM_ADRS + WDB_POOL_SIZE)
#else
#define FREE_MEM_START_ADRS FREE_RAM_ADRS
#endif

#ifdef	INCLUDE_INITIAL_MEM_ALLOCATION
#define MEM_POOL_START_ADRS \
	    (ROUND_UP(FREE_MEM_START_ADRS, (INITIAL_MEM_ALIGNMENT)) + \
	    (INITIAL_MEM_SIZE))
#else	/* INCLUDE_INITIAL_MEM_ALLOCATION */
#define MEM_POOL_START_ADRS FREE_MEM_START_ADRS
#endif	/* INCLUDE_INITIAL_MEM_ALLOCATION */

#ifdef INCLUDE_UEFI_BOOT_SUPPORT
IMPORT VxWorksOSLoaderMemoryMapHeader *pSysUefiMemAddr;
IMPORT UINT32 *pSysUefiAcpiAddr;
IMPORT const char uefiComplianceInfoP[]; /* in text segment */
IMPORT UINT32 uefiMmuNotEnabled; /* MMU not enabled yet flag */
#endif

#if	defined INCLUDE_PROTECT_TASK_STACK \
	|| defined INCLUDE_PROTECT_INTERRUPT_STACK
# if	defined(_WRS_OSM_INIT)
void excOsmInit (int, int);
# endif	/* defined(_WRS_OSM_INIT) */
# if	defined INCLUDE_PROTECT_INTERRUPT_STACK
void usrKernelIntStkProtect(void);
# endif	/* INCLUDE_PROTECT_INTERRUPT_STACK */
#endif	/* INCLUDE_PROTECT_TASK_STACK || INCLUDE_PROTECT_INTERRUPT_STACK */

#if	(CPU_FAMILY == PPC)
void excIntNestLogInit(void);
#endif	/* (CPU_FAMILY == PPC) */

#if	(CPU_FAMILY == MIPS)
void palInit (void);
IMPORT void vxSdaInit (void);
#endif  /* (CPU_FAMILY == MIPS) */


#ifdef INCLUDE_BOOT_APP
void bootApp (void);
#ifdef INCLUDE_BOOT_EXC_HANDLER
STATUS bootAppExcInit (void);
#endif
#if defined(INCLUDE_BOOT_INIT) || defined(INCLUDE_BOOT_DELAYED_INIT)
STATUS bootAppInit (BOOL delayedInit, UINT32 memBase, BOOL disableMmuBeforeEntry);
#endif
#ifdef INCLUDE_ELF
STATUS bootElfInit (void);
#endif
#ifdef INCLUDE_BOOT_SHELL
STATUS bootAppShellInit (void);
#ifdef INCLUDE_BOOT_MEM_CMDS
STATUS bootAppMemInit (UINT32 memBase);
#endif
#ifdef INCLUDE_BOOT_EDR_SUPPORT
STATUS bootAppEdrInit (void);
#endif
#endif /* INCLUDE_BOOT_SHELL */
#ifdef INCLUDE_BOOT_FILESYSTEMS
STATUS bootFsLoadInit (void);
#ifdef INCLUDE_BOOT_FD_LOADER
STATUS bootFdLoadInit (char * pDevName);
#endif
#ifdef INCLUDE_BOOT_USB_FS_LOADER
STATUS bootUsbFsLoadInit (int, int);
#endif
#ifdef INCLUDE_BOOT_TSFS_LOADER
STATUS bootTsfsLoadInit (void);
#endif
#endif /* INCLUDE_BOOT_FILESYSTEMS */

#ifdef INCLUDE_BOOT_DHCPC
STATUS bootDhcpcLoadInit (void);
#endif
#ifdef INCLUDE_BOOT_NETWORK
STATUS bootNetLoadInit (void);
#ifdef INCLUDE_BOOT_FTP_LOADER
STATUS bootFtpLoadInit (void);
#endif
#ifdef INCLUDE_BOOT_TFTP_LOADER
STATUS bootTftpLoadInit (void);
#endif
#ifdef INCLUDE_BOOT_RSH_LOADER
STATUS bootRshLoadInit (void);
#endif
#ifdef INCLUDE_BOOT_END
STATUS bootEndSupportInit (void);
#endif
#ifdef INCLUDE_BOOT_VXBUS
STATUS bootVxBusSupportInit (void);
#endif

#ifdef ETHERNET_MAC_HANDLER
#define INCLUDE_BOOT_ETH_MAC_HANDLER
#endif /* ETHERNET_MAC_HANDLER */
#ifdef ETHERNET_ADR_SET
#define INCLUDE_BOOT_ETH_ADR_SET
#endif  /* ETHERNET_ADR_SET */

#ifdef INCLUDE_BOOT_ETH_MAC_HANDLER
void bootEthMacHandlerInit (int maxMacAdrs, int maxMacDevs,
                            UINT8 defEnetAdrs0, UINT8 defEnetAdrs1, 
                            UINT8 defEnetAdrs2, int macAdrsLen);
#ifdef INCLUDE_BOOT_ETH_ADR_SET
#error "ETHERNET_MAC_HANDLER and ETHERNET_ADR_SET are mutually exclusive"
#endif  /* INCLUDE_BOOT_ETH_ADR_SET */
#elif  defined(INCLUDE_BOOT_ETH_ADR_SET)
void bootEthAdrSetInit (UINT8 defEnetAdrs0, UINT8 defEnetAdrs1, 
                        UINT8 defEnetAdrs2);
#endif /* INCLUDE_BOOT_ETH_MAC_HANDLER */
#endif /* INCLUDE_BOOT_NETWORK */

#ifdef INCLUDE_BOOT_ADDMEM
STATUS bootAddMemInit(UINT32 addr, UINT32 size);
#endif
#endif /* INCLUDE_BOOT_APP */

#ifdef INCLUDE_VXBUS_END_QCTRL
#include <hwif/util/vxbEndQctrl.h>
#endif /* INCLUDE_VXBUS_END_QCTRL */

#ifdef INCLUDE_VXCPULIB
IMPORT void vxCpuLibInit (void);
#endif /* INCLUDE_VXCPULIB */

/* global variables */

int		consoleFd;		/* fd of initial console device */
char		consoleName[20];	/* console device name, eg. "/tyCo/0" */
SYMTAB_ID	statSymTbl;		/* system error status symbol table id*/
SYMTAB_ID	standAloneSymTbl;	/* STANDALONE version symbol table id */
SYMTAB_ID	sysSymTbl;		/* system symbol table id */
BOOT_PARAMS	sysBootParams;		/* parameters from boot line */
int		sysStartType;		/* type of boot (WARM, COLD, etc) */
int		sysFlags;		/* boot flags */
char		sysBootHost [BOOT_FIELD_LEN];   /* name of boot host */
char		sysBootFile [BOOT_FIELD_LEN];   /* name of boot file */
char *		memPoolEndAdrs;		/* end of the kernel memory pool */
#ifdef INCLUDE_PROTECT_TEXT
int		sysTextProtect = 1;
#else  /* INCLUDE_PROTECT_TEXT */
int		sysTextProtect = 0;
#endif /* INCLUDE_PROTECT_TEXT */

#if	(CPU_FAMILY == PPC)
IMPORT _RType taskMsrDefault;   /* from taskArchLib.c */
IMPORT void vxSdaInit (void);
#endif	/* (CPU_FAMILY == PPC) */

extern char * startupScriptFieldSplit (char *);

/* Two magic cookies used to detect data section misalignment */

#define TRAP_VALUE_1	0x12348765
#define TRAP_VALUE_2	0x5a5ac3c3
LOCAL volatile UINT32	trapValue1	= TRAP_VALUE_1;
LOCAL volatile UINT32	trapValue2	= TRAP_VALUE_2;


/*******************************************************************************
*
* usrInit - user-defined system initialization routine
*
* This is the first C code executed after the system boots.  This routine is
* called by the assembly language start-up routine sysInit() which is in the
* sysALib module of the target-specific directory.  It is called with
* interrupts locked out.  The kernel is not multitasking at this point.
*
* This routine starts by clearing BSS; thus all variables are initialized to 0,
* as per the C specification.  It then initializes the hardware by calling
* sysHwInit(), sets up the interrupt/exception vectors, and starts kernel
* multitasking with usrRoot() as the root task.
*
* RETURNS: N/A
*
* SEE ALSO: kernelLib
*
* ARGSUSED0
*/

void usrInit
    (
#ifdef INCLUDE_UEFI_BOOT_SUPPORT
    int startType,
    VxWorksOSLoaderMemoryMapHeader *pUefiMemAddress,
    UINT32 *pUefiAcpiAddress
#else
    int startType
#endif
    )
    {

#if (CPU_FAMILY == PPC) || (CPU_FAMILY == MIPS)

    /* 
     * For PPC and MIPS, the call to vxSdaInit() must be the first operation 
     * in sysStart(). This is because vxSdaInit() sets the SDA registers
     * (r2 and r13 on PPC, gp on MIPS) to the SDA base values. No C code 
     * must be placed before this call.
     */

    _WRS_ASM ("");   /* code barrier to prevent compiler moving vxSdaInit() */
    vxSdaInit ();    /* this MUST be the first operation in usrInit() for PPC */
    _WRS_ASM ("");   /* code barrier to prevent compiler moving vxSdaInit() */

#endif	/* (CPU_FAMILY == PPC) || (CPU_FAMILY == MIPS)  */


    /*
     * This trap will catch improper loading of the data section.
     * We check the magic cookie values to make sure the data section is
     * in the expected memory location. We do not want
     * to proceed further if the data segment is not correct.
     *
     * It should be easy to detect entry into the trap using an ICE, JTAG,
     * or logic analyzer. Without the trap, the processor is likely to run
     * away out of control.
     *
     * Data section misalignment can occur when there is a change in tool
     * chain, build rules, compiler, host utilites,  etc.
     */

    while (trapValue1 != TRAP_VALUE_1 || trapValue2 != TRAP_VALUE_2)
	{
	/* infinite loop */;
	}

#ifndef INCLUDE_BOOT_APP
    /* 
     * initialize the exception message string for VxWorks images;
     * bootApp images must preserve the string so they can display it
     * when rebooting VxWorks due to non-recoverable errors.
     */

    sysExcMsg[0] = EOS;
#endif /* INCLUDE_BOOT_APP */

#if (CPU_FAMILY == MIPS)
    palInit ();
#endif	/* (CPU_FAMILY == MIPS) */

#if	(CPU_FAMILY == SPARC)
    excWindowInit ();				/* SPARC window management */
#endif

#ifdef INCLUDE_UEFI_BOOT_SUPPORT
    /*
     * For UEFI support, we will set the global pointer and then use it in the
     * SYS_HW_INIT_0() function below to set up the physical memory map table. 
     */
    pSysUefiMemAddr = pUefiMemAddress; /* store the pointer we received */
    pSysUefiAcpiAddr = pUefiAcpiAddress; /* store the pointer we received */
#endif

#ifdef INCLUDE_SYS_HW_INIT_0
    /*
     * Perform any BSP-specific initialisation that must be done before
     * cacheLibInit() is called and/or BSS is cleared.
     */

    SYS_HW_INIT_0 ();
#endif /* INCLUDE_SYS_HW_INIT_0 */

    /* configure data and instruction cache if available and leave disabled */

    /* don't assume bss variables are zero before this call */

    bzero (edata, end - edata);		/* zero out bss variables */

#ifdef  INCLUDE_CACHE_SUPPORT
    /*
     * SPR 73609:  If a cache is not to be enabled, don't require
     * its mode to be defined.  Instead, default it to disabled.
     */
# if (!defined(USER_D_CACHE_ENABLE) && !defined(USER_D_CACHE_MODE))
#  define USER_D_CACHE_MODE CACHE_DISABLED
# endif	/* !USER_D_CACHE_ENABLE && !USER_D_CACHE_MODE */
# if (!defined(USER_I_CACHE_ENABLE) && !defined(USER_I_CACHE_MODE))
#  define USER_I_CACHE_MODE CACHE_DISABLED
# endif	/* !USER_I_CACHE_ENABLE && !USER_I_CACHE_MODE */

    cacheLibInit (USER_I_CACHE_MODE, USER_D_CACHE_MODE);
#endif  /* INCLUDE_CACHE_SUPPORT */


    sysStartType = startType;			/* save type of system start */

    intVecBaseSet ((FUNCPTR *) VEC_BASE_ADRS);	/* set vector base table */

#if (CPU_FAMILY == AM29XXX)
    excSpillFillInit ();			/* am29k stack cache managemt */
#endif

#ifdef  INCLUDE_EXC_HANDLING
# if (CPU_FAMILY == PPC) && defined(INCLUDE_EXC_SHOW)
    /*
     * Do this ahead of excVecInit() to set up _func_excPanicHook, in case
     * the enabling of Machine Check there allows a pending one to occur.
     * excShowInit() will be called again later, harmlessly.
     */
    excShowInit ();
# endif  /* CPU_FAMILY == PPC && defined(INCLUDE_EXC_SHOW) */
# if (CPU_FAMILY == PPC) && defined(INCLUDE_EXC_EXTENDED_VECTORS)
    excExtendedVectors = TRUE;
# endif /* CPU_FAMILY == PPC && defined(INCLUDE_EXC_EXTENDED_VECTORS) */
    excVecInit ();				/* install exception vectors */
#endif  /* INCLUDE_EXC_HANDLING */

#ifdef INCLUDE_VXCPULIB
    vxCpuLibInit();		               /* init CPU identification data */
#endif /* INCLUDE_VXCPULIB */

    sysHwInit ();				/* initialize system hardware */

#ifdef  INCLUDE_DEBUG_KWRITE_SIO
    kwriteSioInit (CONSOLE_TTY, CONSOLE_BAUD_RATE);
#endif
#ifdef  INCLUDE_DEBUG_KWRITE_USER
    usrKwriteInit (DEBUG_KWRITE_USR_RTN);
#endif

#ifdef  INCLUDE_CACHE_SUPPORT
#ifdef 	USER_I_CACHE_ENABLE
    cacheEnable (INSTRUCTION_CACHE);		/* enable instruction cache */
#endif	/* USER_I_CACHE_ENABLE */

#ifdef	USER_D_CACHE_ENABLE
    cacheEnable (DATA_CACHE);			/* enable data cache */
#endif 	/* USER_D_CACHE_ENABLE */

#if (CPU == MC68060)
#ifdef 	USER_B_CACHE_ENABLE
    cacheEnable (BRANCH_CACHE);			/* enable branch cache */
#endif	/* USER_B_CACHE_ENABLE */
#endif	/* (CPU == MC68060) */
#endif  /* INCLUDE_CACHE_SUPPORT */

    usrKernelInit (VX_GLOBAL_NO_STACK_FILL);	/* configure the Wind kernel */

    /* configure memory manager ED&R library (pre-kernel part) */

#ifdef INCLUDE_MEM_EDR
    memEdrInit ();
#endif

    /* if wanted, start WDB before kernel */

#if	defined (INCLUDE_WDB) && (WDB_INIT == WDB_PRE_KERNEL_INIT)
    usrWdbInit ();
    wdbSystemSuspend ();
#endif	/* defined (INCLUDE_WDB) && (WDB_INIT == WDB_PRE_KERNEL_INIT) */

#ifdef INCLUDE_RTP
    memPoolEndAdrs = min ((char *) MEM_POOL_START_ADRS + KERNEL_HEAP_SIZE,
			  sysMemTop());
#else
    memPoolEndAdrs = sysMemTop();
#endif

#ifdef	INCLUDE_TLS
    /* Initialize Thread Local Storage library for __thread variables support */

    tlsLibInit (wrs_kernel_tls_vars_start,
    		(UINT32) wrs_kernel_tls_vars_size,
		wrs_kernel_tls_data_start,
		(UINT32) wrs_kernel_tls_data_size,
		(UINT32) wrs_kernel_tls_data_align);
#endif	/* INCLUDE_TLS */

    /* start the kernel specifying usrRoot as the root task */

	{
	_KERNEL_INIT_PARAMS	kIP;

	bfill((char *)&kIP, sizeof(kIP), 0);

	kIP.rootRtn = (FUNCPTR) usrRoot;
	kIP.rootMemSize = ROOT_STACK_SIZE;
	kIP.pMemPoolStart = MEM_POOL_START_ADRS;
	kIP.pMemPoolEnd = memPoolEndAdrs;
	kIP.intStackSize = ISR_STACK_SIZE;
	kIP.lockOutLevel = INT_LOCK_LEVEL;
	kIP.vmPageSize = VM_PAGE_SIZE;
#ifdef	_ARCH_SUPPORTS_PROTECT_INTERRUPT_STACK
# ifdef	INCLUDE_PROTECT_INTERRUPT_STACK
	/* else already zero, by bfill above */
	kIP.intStackOverflowSize = INTERRUPT_STACK_OVERFLOW_SIZE;
	kIP.intStackUnderflowSize = INTERRUPT_STACK_UNDERFLOW_SIZE;
# endif  /* INCLUDE_PROTECT_INTERRUPT_STACK */
#endif	/* _ARCH_SUPPORTS_PROTECT_INTERRUPT_STACK */

	kernelInit (_KERNEL_INIT_PARAMS_VN_AND_SIZE, &kIP);
	}
    }

/*******************************************************************************
*
* usrRoot - the root task
*
* This is the first task to run under the multitasking kernel.  It performs
* all final initialization and then starts other tasks.
*
* It initializes the I/O system, installs drivers, creates devices, and sets
* up the network, etc., as necessary for a particular configuration.  It
* may also create and load the system symbol table, if one is to be included.
* It may then load and spawn additional tasks as needed.  In the default
* configuration, it simply initializes the VxWorks shell.
*
* RETURNS: N/A
*/

void usrRoot
    (
    char *	pMemPoolStart,		/* start of system memory partition */
    unsigned	memPoolSize		/* initial size of mem pool */
    )
    {
#if defined(INCLUDE_TTY_DEV) || defined(INCLUDE_PC_CONSOLE) || \
    defined(INCLUDE_STAT_SYM_TBL) || defined(INCLUDE_ATA)
    int  ix;
#if defined(INCLUDE_TTY_DEV) || defined(INCLUDE_PC_CONSOLE)
    char tyName [20];
#endif
#endif  /* INCLUDE_{TTY_DEV,PC_CONSOLE,STAT_SYM_TBL,ATA} */

    /* initialize core kernel facilities */

    usrKernelCoreInit();

    /* New CPU power management only available on Pentium */
#ifdef INCLUDE_CPU_PWR_MGMT 
    cpuPwrFrameWorkInit
	(
	CPU_PWR_TASK_P_STATE,
	CPU_PWR_ISR_P_STATE,
	CPU_PWR_UTIL_PERIOD1,
	CPU_PWR_UTIL_PERIOD2,
	CPU_PWR_HIGH_THRESHOLD,
	CPU_PWR_LOW_THRESHOLD
	);
#ifdef INCLUDE_CPU_UTIL_PWR_MGR
    cpuPwrUtilMgrInit (CPU_PWR_UP_UTIL, CPU_PWR_DOWN_UTIL);
#else
    cpuPwrLightMgrInit ();
#endif /* INCLUDE_CPU_UTIL_PWR_MGR */
#endif /* INCLUDE_CPU_PWR_MGMT */

    /* Initialize the memory pool before initializing any other package.
     * The memory associated with the root task will be reclaimed at the
     * completion of its activities.
     */

#ifdef INCLUDE_MEM_ALLOT
    /* initialize memory manager */

    memAllotLibInit (pMemPoolStart, memPoolSize);
#else  /* INCLUDE_MEM_ALLOT */
# ifdef INCLUDE_MEM_MGR_FULL
    memInit (pMemPoolStart, memPoolSize, MEM_PART_DEFAULT_OPTIONS);
# else
    memPartLibInit (pMemPoolStart, memPoolSize);
# endif /* INCLUDE_MEM_MGR_FULL */

# ifdef INCLUDE_MEM_MGR_INFO
    memInfoInit();
# endif /* INCLUDE_MEM_MGR_INFO */

#endif  /* INCLUDE_MEM_ALLOT */

#ifdef INCLUDE_POOL
    poolLibInit();
#endif

    /* configure memory manager ED&R library (post kernel init part) */

#ifdef INCLUDE_MEM_EDR
    memEdrInit2 ();
#ifdef INCLUDE_MEM_EDR_SHOW
    memEdrShowInit ();
#endif
#endif

    /* memEdrRtpShow is dependent on RTP support, but not on MEM_EDR */

#ifdef INCLUDE_RTP
#ifdef INCLUDE_MEM_EDR_RTP_SHOW
    memEdrRtpShowInit ();
#endif
#endif

#ifdef	INCLUDE_SHOW_ROUTINES
    memShowInit ();				/* initialize memShow routine */
#endif	/* INCLUDE_SHOW_ROUTINES */


#ifdef INCLUDE_USB_TARG
    usrUsbTargPciInit ();
#endif

    /* Initialize sysctl support */

#ifdef INCLUDE_SYSCTL
    usrSysctlInit();                            /* System control function */

#ifdef INCLUDE_SYSCTL_HW
    usrHwSysctlInit();                          /* System control h/w access */
#endif /* INCLUDE_SYSCTL_HW */

#endif /* INCLUDE_SYSCTL */

    /*
     * Initialization of RTP should be early. Memory allocations
     * may vary depending on whether RTP is initialized or not.
     */

/*
 * RTP initialization is up here because the MM code relies on it. This needs
 * to be moved once the dependency is removed.
 */

#ifdef  INCLUDE_RTP

#ifdef        INCLUDE_RTP_HOOKS
    rtpHookLibInit (RTP_HOOK_TBL_SIZE);
#endif        /* INCLUDE_RTP_HOOKS */

    rtpLibInit (RTP_HOOK_TBL_SIZE,
		SYSCALL_HOOK_TBL_SIZE,
		RTP_SIGNAL_QUEUE_SIZE,
		TASK_USER_EXC_STACK_OVERFLOW_SIZE,
		TASK_USER_EXEC_STACK_OVERFLOW_SIZE,
		TASK_USER_EXEC_STACK_UNDERFLOW_SIZE,
		RTP_FD_NUM_MAX
		);

#ifdef        INCLUDE_SYSCALL_HOOKS
    syscallHookLibInit ();
#endif        /* INCLUDE_RTP_HOOKS */

#if defined (INCLUDE_RTP_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    rtpShowInit ();                             /* initialize rtpShow routine */
#endif  /* INCLUDE_SHOW_ROUTINES */

#ifdef INCLUDE_SHARED_DATA
    sdLibInit (SD_HOOK_TBL_SIZE);		/* initialize shared data */
#if defined(INCLUDE_SHARED_DATA_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    sdShowInit ();                             /* initialize sdShow routine */
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif /* INCLUDE_SHARED_DATA */

#ifdef	INCLUDE_SHL
    shlLibInit ();				/* initialize shared library */
#if defined (INCLUDE_SHL_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    shlShowInit();				/* initialize SHL show ruotines */
#endif /* INCLUDE_SHL_SHOW || INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_SHL */

#endif  /* INCLUDE_RTP */

    /* setup the page pool library */

    /*
     * The MMU intialization code relies on the address space allocator being
     * initialized. The address space allocator relies on the page pool
     * libraries being initialized. Therefore this order needs to be
     * preserved.
     */

#ifdef INCLUDE_PG_POOL_LIB
    pgPoolLibInit();    /* always returns OK */
#endif /* INCLUDE_PG_POOL_LIB */
#ifdef INCLUDE_PG_POOL_VIRT_LIB
    pgPoolVirtLibInit();        /* always returns OK */
#endif /* INCLUDE_PG_POOL_VIRT_LIB */
#ifdef INCLUDE_PG_POOL_PHYS_LIB
    pgPoolPhysLibInit();        /* always returns OK */
#endif /* INCLUDE_PG_POOL_PHYS_LIB */

#if    defined(INCLUDE_MMU_BASIC) || defined(INCLUDE_MMU_GLOBAL_MAP)
#ifdef INCLUDE_AIM_MMU_CONFIG
    usrAimMmuConfig();
#endif /* INCLUDE_AIM_MMU_CONFIG */
    usrMmuInit ((VIRT_ADDR) pMemPoolStart, memPoolSize);/* initialize the mmu */
#endif	/* defined(INCLUDE_MMU_BASIC) || defined(INCLUDE_MMU_GLOBAL_MAP) */

#ifdef INCLUDE_UEFI_BOOT_SUPPORT
	uefiMmuNotEnabled=0; /* MMU enabled; no more UEFI operations on the physMemDesc table */
#endif

    /*
     * initialize environment variable. putenv() and getenv() now uses 
     * semaphores for thread saftey. So this library needs to be initialized 
     * before it is being called by any other function. (defect CQ:WIND00194940)
     */

#ifdef  INCLUDE_ENV_VARS
    envLibInit (ENV_VAR_USE_HOOKS);     /* initialize environment variable */
#endif  /* INCLUDE_ENV_VARS */

/*
 * ED&R initialization needs to be as early as possible in the boot
 * sequence, in order to catch early exceptions. It is here,
 * immediately after the MMU is initialised, since it uses the
 * services of vmLib.
 */

#ifdef INCLUDE_EDR_PM
    usrPmInit ();
#endif /* INCLUDE_EDR_PM */

#ifdef INCLUDE_EDR_ERRLOG
    /* initialize the ED&R subsystem */
    usrEdrInit ();

#ifdef INCLUDE_EDR_SHOW
    edrShowInit ();
#endif /* INCLUDE_EDR_SHOW */

#endif /* INCLUDE_EDR_ERRLOG */

#ifdef INCLUDE_EDR_SYSDBG_FLAG
    edrSystemDebugModeInit ();
#endif /* INCLUDE_EDR_SYSDBG_FLAG */

#ifdef INCLUDE_BOOT_EDR_SUPPORT
    edrSystemDebugModeSet (FALSE);  /* always run in deployed mode */
#endif

#ifndef INCLUDE_BOOT_APP
    /* Now inject a 'boot' ED&R event - this takes place outside
     * the INCLUDE_EDR guards, as the stub is always included.
     *
     * Do not inject this event for a bootrom build
     */
    edrStubInit ();
#endif

#if    defined(INCLUDE_MMU_BASIC)
#ifdef	INCLUDE_PROTECT_TEXT
    usrTextProtect ();
#endif	/* INCLUDE_PROTECT_TEXT */

    /*
     * For arch which provide an OSM initialization function, it must be
     * called if guard regions are being defined for either task stacks or
     * the interrupt stack.  If the interrupt stack is protected and task
     * stacks are not, pass the task stack overflow size parameter as zero.
     */

#if	defined INCLUDE_PROTECT_TASK_STACK \
	|| defined INCLUDE_PROTECT_INTERRUPT_STACK
# if	defined(_WRS_OSM_INIT)
#  if	defined INCLUDE_PROTECT_TASK_STACK
    excOsmInit (TASK_USER_EXC_STACK_OVERFLOW_SIZE, VM_PAGE_SIZE);
#  else  /* INCLUDE_PROTECT_TASK_STACK */
    excOsmInit (0, VM_PAGE_SIZE);
#  endif /* INCLUDE_PROTECT_TASK_STACK */
# endif	/* defined(_WRS_OSM_INIT) */
#endif	/* INCLUDE_PROTECT_TASK_STACK || INCLUDE_PROTECT_INTERRUPT_STACK */

#if	defined INCLUDE_PROTECT_INTERRUPT_STACK
    /* Apply protection to interrupt stack guard regions */
    usrKernelIntStkProtect();
#endif	/* INCLUDE_PROTECT_INTERRUPT_STACK */

#if	defined INCLUDE_PROTECT_TASK_STACK
    taskStackGuardPageEnable ();
#endif	/* INCLUDE_PROTECT_TASK_STACK */

#ifdef	INCLUDE_TASK_STACK_NO_EXEC
    taskStackNoExecEnable ();
#endif	/* INCLUDE_TASK_STACK_NO_EXEC */

#ifdef INCLUDE_AIM_MMU_SHOW
    aimMmuShowInit();
#endif
#endif	/* defined(INCLUDE_MMU_BASIC) */

#if	(CPU_FAMILY == PPC)
    excIntNestLogInit();			/* frameless nesting logger */
    /* Enable interrupts immediately after MMU */
    vxMsrSet(vxMsrGet() | taskMsrDefault);
#endif	/* (CPU_FAMILY == PPC) */

    /*
     * initialize signals earlier since idle task entry function has
     * a dependency on "sigInit()"
     */

#ifdef	INCLUDE_SIGNALS
    sigInit (POSIX_SIGNAL_MODE);        /* initialize signals */
#endif	/* INCLUDE_SIGNALS */

    /*
     * idle task must be activated:
     *
     * %before% the root task goes to a NON WIND_READY state
     *
     * %after%
     * "sigInit ()" - initializes the signal facilities
     * "usrMmuInit ()" - initializes the MMU
     * "usrKernelIdleTaskStkProtect()" - Apply protection to idle task
     *                                   exception stack guard regions.
     *                                   <not implemented yet>
     */

#ifdef INCLUDE_RTP
#ifdef INCLUDE_PG_MGR_LIB
    pgMgrLibInit();    /* always returns OK */
#endif /* INCLUDE_PG_MGR_LIB */

#if	defined (INCLUDE_RTP_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    pgPoolShowInit ();				/* initialize pgPoolShow lib */
    pgMgrShowInit ();				/* initialize pgMgrShow lib */
#endif /* defined(INCLUDE_RTP_SHOW) */

#if	defined (INCLUDE_ADR_SPACE_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    adrSpaceShowInit ();			/* initialize adrSpaceShow */
#endif /* defined(INCLUDE_ADR_SPACE_SHOW) */

#if defined (INCLUDE_MAPPED_FILES_SHOW)
    mmanShowInit ();				/* initialize mmanShow lib */
#endif
#endif /* INCLUDE_RTP */

#if defined(INCLUDE_CORE_DUMP) || defined(INCLUDE_CORE_DUMP_RTP)
    usrCoreDumpInit ();
#endif	/* INCLUDE_CORE_DUMP */

#if defined(INCLUDE_BOOT_APP) && defined(INCLUDE_BOOT_ADDMEM)
    bootAddMemInit(ADDED_BOOTMEM_ADDR, ADDED_BOOTMEM_SIZE);
#endif  /* INCLUDE_BOOT_ADDMEM defined */

    /* set up system timer */

    sysClkConnect ((FUNCPTR) usrClock, 0);	/* connect clock ISR */
    sysClkRateSet (SYS_CLK_RATE);	/* set system clock rate */
    sysClkEnable ();				/* start it */

#ifdef INCLUDE_VXIPI
    vxIpiLibInit ();
#endif /* INCLUDE_VXIPI */

#ifdef INCLUDE_VXDBG_CPU_CONTROL
    vxdbgCpuLibInit ();
#endif /* INCLUDE_VXDBG_CPU_CONTROL */

#ifdef INCLUDE_FAST_DRAM
/*
 * make use of data cache as fast DRAM,
 * establish parameters in config.h, MMU
 * must be initialed before data cache is
 * initialized as data ram...
 */
  cacheCreateInternalDataRAM((UINT32 *)FD_ORIGIN, FD_NUMLINES);
#endif

    /* initialize I/O system */

#ifdef  INCLUDE_IO_BASIC
    iosInit (NUM_DRIVERS, NUM_FILES, "/null");
#endif /* INCLUDE_IO_BASIC */

#ifdef	INCLUDE_IO_FILE_SYSTEM
    iosPathLibInit ();
#endif

#ifdef	INCLUDE_IO_REMOVABLE
    iosRmvLibInit ();
#endif

#ifdef	INCLUDE_IO_POSIX
    iosPxLibInit (IOS_POSIX_PSE52_MODE);
#endif

#ifdef	INCLUDE_IO_RTP
    iosRtpLibInit ();
#endif

    consoleFd = NONE;                           /* assume no console device */

    /*
     * The select library needs to be initialized before the tyLib module
     * since the _func_selWakeupListInit FUNCPTR is required (SPR #3314).
     * The installation of the select task delete hooks is performed
     * later in usrRoot() after NFS and RPC have been initialized.
     */
#ifdef  INCLUDE_SELECT
    selectInit (NUM_FILES);
#endif	/* INCLUDE_SELECT */

    /* initialize process file system */

#ifdef 	INCLUDE_PROCFS
    {
    extern int procfsFsDevCreate (char *, int);
    extern int rtpProcfsInit (void);
    extern int edrProcfsInit (void);

    procfsFsDevCreate ("/proc", 5);

#ifdef	INCLUDE_RTP
    rtpProcfsInit();	/* install RTP specific entries */
#endif /* INCLUDE_RTP */

#ifdef	INCLUDE_EDR_ERRLOG
    edrProcfsInit();	/* install ED&R specific entries */
#endif /* INCLUDE_EDR_ERRLOG */
    }
#endif /* INCLUDE_PROCFS */

#ifdef INCLUDE_ISR_DEFER
    usrIsrDeferInit ();
#endif	/* INCLUDE_ISR_DEFER */

#ifdef INCLUDE_TYLIB
    tyLibInit (TYLIB_XOFF_PCNT, TYLIB_XON_PCNT, TYLIB_WRT_THRESHLD);
#endif

#ifdef INCLUDE_SER_EMULATOR_INIT
    usbTargSerInit(); /* USB device CDC serial driver initialization */
#endif

#ifdef  INCLUDE_TYCODRV_5_2
#ifdef  INCLUDE_TTY_DEV
    if ((NUM_TTY > 0))
	{
	tyCoDrv ();				/* install console driver */

	for (ix = 0; ix < NUM_TTY; ix++)	/* create serial devices */
	    {
	    sprintf (tyName, "%s%d", "/tyCo/", ix);

	    (void) tyCoDevCreate (tyName, ix, 512, 512);

	    if (ix == CONSOLE_TTY)
		strcpy (consoleName, tyName);	/* store console name */
	    }

	consoleFd = open (consoleName, O_RDWR, 0);

	/* set baud rate */

	(void) ioctl (consoleFd, FIOBAUDRATE, CONSOLE_BAUD_RATE);
	(void) ioctl (consoleFd, FIOSETOPTIONS, OPT_TERMINAL);
	}
#endif	/* INCLUDE_TTY_DEV */

#else   /* !INCLUDE_TYCODRV_5_2 */
#ifdef  INCLUDE_TTY_DEV

    ttyDrv ();				/* install console driver */

    if ((NUM_TTY > 0))
	{
	for (ix = 0; ix < NUM_TTY; ix++)	/* create serial devices */
	    {
#if     (defined(INCLUDE_WDB) && (WDB_COMM_TYPE == WDB_COMM_SERIAL))
	    if (ix == WDB_TTY_CHANNEL)		/* don't use WDBs channel */
		continue;
#endif
	    sprintf (tyName, "%s%d", "/tyCo/", ix);
	    (void) ttyDevCreate (tyName, sysSerialChanGet(ix), 512, 512);

	    if (ix == CONSOLE_TTY)		/* init the tty console */
		{
		strcpy (consoleName, tyName);
		consoleFd = open (consoleName, O_RDWR, 0);
		(void) ioctl (consoleFd, FIOBAUDRATE, CONSOLE_BAUD_RATE);
		(void) ioctl (consoleFd, FIOSETOPTIONS, OPT_TERMINAL);
		}
	    }
	}
#endif  /* INCLUDE_TTY_DEV */

#ifdef INCLUDE_PC_CONSOLE
    pcConDrv ();
    for (ix = 0; ix < N_VIRTUAL_CONSOLES; ix++)
	{
	sprintf (tyName, "%s%d", "/pcConsole/", ix);
	(void) pcConDevCreate (tyName,ix, 512, 512);
	if (ix == PC_CONSOLE)		/* init the console device */
	    {
	    strcpy (consoleName, tyName);
	    consoleFd = open (consoleName, O_RDWR, 0);
	    (void) ioctl (consoleFd, FIOBAUDRATE, CONSOLE_BAUD_RATE);
	    (void) ioctl (consoleFd, FIOSETOPTIONS, OPT_TERMINAL);
	    }
	}
#endif	/* INCLUDE_PC_CONSOLE */

#endif  /* !INCLUDE_TYCODRV_5_2 */

#ifdef INCLUDE_IO_BASIC
    /*
     * It's actually safe to call these with consoleFd==NONE (it will
     * just return an error), but it seems more appropriate to check.
     */
    if (consoleFd != NONE)
	{
	ioGlobalStdSet (STD_IN,  consoleFd);
	ioGlobalStdSet (STD_OUT, consoleFd);
	ioGlobalStdSet (STD_ERR, consoleFd);
	}
#endif  /* INCLUDE_IO_BASIC */

#if defined (_WRS_PAL_COPROC_LIB)
    coprocLibInit();
#ifdef INCLUDE_HW_FP
    aimFppLibInit();        /* dummy function to include fppTaskRegsSet/Get */
#endif
#endif /* _WRS_PAL_COPROC_LIB */

#ifdef INCLUDE_ACPI
    acpiLibKernelInit (); /* pentium specific ACPI initialzation (part-2) */
#endif

#ifdef	INCLUDE_HASH
    hashLibInit ();			/* initialize hash table package */
#endif	/* INCLUDE_HASH */

    /* initialize symbol table facilities */

#ifdef	INCLUDE_SYM_TBL
    symLibInit ();			/* initialize symbol table package */
#ifdef 	INCLUDE_SHOW_ROUTINES
    symShowInit ();			/* initialize symbol table show */
#endif	/* INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_SYM_TBL */

    /* initialize exception handling */

#if     defined(INCLUDE_EXC_HANDLING) && defined(INCLUDE_EXC_TASK)
#ifdef	INCLUDE_EXC_SHOW
    excShowInit ();
#endif
    /* initialize exception handling */
    excInit (MAX_ISR_JOBS);
#endif  /* defined(INCLUDE_EXC_HANDLING) && defined(INCLUDE_EXC_TASK) */

#if defined(INCLUDE_JOB_TASK)
    jobLibInit (JOB_TASK_STACK_SIZE);
#endif /* INCLUDE_JOB_TASK */

#ifdef	INCLUDE_LSTLIB
    lstLibInit ();
#endif

#ifdef	INCLUDE_LOGGING
    logInit (consoleFd, MAX_LOG_MSGS);	/* initialize logging */
# ifdef	INCLUDE_LOG_STARTUP
    logMsg ("logging started to %s [%d], queue size %d\n",
		consoleName, consoleFd, MAX_LOG_MSGS, 4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOG_STARTUP */
#endif	/* INCLUDE_LOGGING */

#ifdef INCLUDE_NBIO_LOG
    nbioLogInit (consoleFd, INIT_NBIO_MSGS);
#endif /* INCLUDE_NBIO_LOG */

#ifdef	INCLUDE_POSIX_CLOCKS
    clockLibInit ();			/* initialize clock */
#endif	/* INCLUDE_POSIX_CLOCKS */

#ifdef	INCLUDE_POSIX_TIMERS
    timerLibInit ();			/* initialize timers */
#ifdef INCLUDE_OBJ_OPEN
    timerOpenInit ();
#endif
#if defined(INCLUDE_POSIX_TIMER_SHOW) || defined(INCLUDE_SHOW_ROUTINES)
    timerShowInit();
#endif
#endif	/* INCLUDE_POSIX_TIMERS */


#ifdef INCLUDE_POSIX_SHM
    shmFsLibInit (SHM_DEV_NAME);	/* initialize POSIX shared memory */
					/* objects option */
#endif

#ifdef INCLUDE_POSIX_MAPPED_FILES
    mmanFdLibInit();			/* initialize POSIX memory mapped */
					/* files option */
#endif

#ifdef INCLUDE_POSIX_ADVISORY_FILE_LOCKING
    usrPosixAdvLockInit();
#endif

    /* initialize debugging */

#ifdef	INCLUDE_DEBUG
    dbgInit ();				/* initialize debugging */
#endif	/* INCLUDE_DEBUG */


    /* initialize pipe driver */

#ifdef	INCLUDE_PIPES
    pipeDrv ();				/* install pipe driver */
#endif	/* INCLUDE_PIPES */

    /* initialize POSIX async I/O support */

#ifdef INCLUDE_POSIX_AIO
    aioPxLibInit (MAX_LIO_CALLS);
#ifdef INCLUDE_POSIX_AIO_SYSDRV
    aioSysInit (MAX_AIO_SYS_TASKS, AIO_TASK_PRIORITY, AIO_TASK_STACK_SIZE);
#endif  /* INCLUDE_POSIX_AIO_SYSDRV */
#endif  /* INCLUDE_POSIX_AIO */

    /* initialize standard I/O package */

#ifdef	INCLUDE_STDIO
    stdioInit ();			/* initialize standard I/O library */
#ifdef  INCLUDE_SHOW_ROUTINES
    stdioShowInit ();
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_STDIO */

    /* initialize POSIX queued signals */

#if defined(INCLUDE_POSIX_SIGNALS) && defined(INCLUDE_SIGNALS)
    sigqueueInit (NUM_SIGNAL_QUEUES); /* initialize queued signals */
#endif

    /* initialize POSIX semaphores */

#ifdef  INCLUDE_POSIX_SEM
    semPxLibInit ();
#ifdef INCLUDE_SHOW_ROUTINES
    semPxShowInit ();
#endif  /* INCLUDE_SHOW_POUTINES */
#endif  /* INCLUDE_POSIX_SEM */

    /* initialize POSIX threads */

#ifdef INCLUDE_POSIX_PTHREADS
    pthreadLibInit ();
#endif  /* INCLUDE_POSIX_PTHREADS */

    /* initialize POSIX scheduler interface to pthread */

#ifdef INCLUDE_POSIX_PTHREAD_SCHEDULER
    taskPxLibInit ();
#ifdef INCLUDE_PX_SCHED_SPORADIC_POLICY
    pxSSThreadLibInit ();
#endif /* INCLUDE_PX_SCHED_SPORADIC_POLICY */
#endif /* INCLUDE_POSIX_PTHREAD_SCHEDULER */

    /* initialize Signal Events */

#ifdef INCLUDE_SIGEVENT
    sigeventLibInit ();
#endif  /* INCLUDE_SIGEVENT */

    /* initialize POSIX message queues */

#ifdef INCLUDE_POSIX_MQ
    mqPxLibInit (MQ_HASH_SIZE);
#ifdef INCLUDE_SHOW_ROUTINES
    mqPxShowInit ();
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif  /* INCLUDE_POSIX_MQ */

#ifdef INCLUDE_POSIX_THREAD_CPUTIME
    pxCpuTimeLibInit();
#endif

#ifdef INCLUDE_ERF
    erfLibInit (ERF_MAX_USR_CATEGORIES, ERF_MAX_USR_TYPES);
#endif

#ifdef INCLUDE_DEVICE_MANAGER
    devInit (DEVICE_MANAGER_MAX_NUM_DEVICES);
#endif

#ifdef INCLUDE_XBD
    xbdInit ();
#endif

#ifdef INCLUDE_XBD_BLK_DEV
    xbdBlkDevLibInit (XBD_BLK_DEV_TASK_PRIORITY,
                      XBD_BLK_DEV_DIRECT_BIO_HANDLING);
#endif

#ifdef INCLUDE_FS_MONITOR
    fsMonitorInit ();
#endif

#ifdef INCLUDE_FS_EVENT_UTIL
    fsEventUtilLibInit ();
#endif

#ifdef INCLUDE_XBD_TRANS
    xbdTransInit ();
#endif

#if defined(INCLUDE_HRFS) || defined (INCLUDE_HRFS_READONLY)
    usrHrfsInit (HRFS_DEFAULT_MAX_BUFFERS, HRFS_DEFAULT_MAX_FILES);
#endif

#ifdef INCLUDE_CBIO /* init main CBIO module, cbioLib() */
    cbioLibInit();
#endif /* INCLUDE_CBIO */

    /* initialize filesystems and disk drivers */

#ifdef INCLUDE_DOSFS_MAIN  /* dosFs2 file system initialization */
    usrDosfsInit (DOSFS_DEFAULT_MAX_FILES, DOSFS_DEFAULT_CREATE_OPTIONS);
#endif /* INCLUDE_DOSFS_MAIN */

#ifdef INCLUDE_DOSFS_CACHE
    dosFsCacheLibInit (DOSFS_DEFAULT_DATA_DIR_CACHE_SIZE,
                       DOSFS_DEFAULT_FAT_CACHE_SIZE,
                       DOSFS_CACHE_BACKGROUND_FLUSH_TASK_ENABLE);
#endif

#ifdef	INCLUDE_RAWFS
    rawFsInit (NUM_RAWFS_FILES); 	/* init rawFs filesystem */
#endif	/* INCLUDE_RAWFS */

#ifdef INCLUDE_HRFS_FORMAT
    hrfsFormatLibInit ();
#endif

#ifdef	INCLUDE_RAMDRV
    ramDrv ();				/* initialize ram disk driver */
#endif	/* INCLUDE_RAMDRV */

#ifdef INCLUDE_CDROMFS
    cdromFsInit (CDROM_COM_BUF_SIZE);   /* initialize the cdromFs library */
#endif

#ifdef	INCLUDE_MEMDRV
    memDrv ();				/* initialize memory disk driver */
#endif	/* INCLUDE_MEMDRV */

#ifdef	INCLUDE_PTYDRV
    ptyDrv ();				/* initialize pseudo-terminal driver */
#endif	/* INCLUDE_PTYDRV */

#ifdef INCLUDE_VRFS
    vrfsInit();
    vrfsDevCreate();
#endif

#ifdef	INCLUDE_ROMFS
    romfsImageLen = romfsImageEnd-romfsImage;

    if (romfsImageLen != 0)		/* initialize ROMFS file system */
	{
	BLK_DEV *pBlkDev;
	void *pVolDesc;

	pBlkDev=(BLK_DEV *)romfsBlkDevCreate((char *)&romfsImage,
					  romfsImageLen, 0);
	if (pBlkDev == NULL)
	    {
	    printErr ("Error creating romfs block device!\n");
	    }
	else
	    {
	    pVolDesc = (void *)romfsFsDevCreate ("/romfs", pBlkDev);
	    }
	}
#endif /* INCLUDE_ROMFS */

    /* Initialize PCCARD services */

#ifdef INCLUDE_PCCARD
    csInit (CS_PCCARD_STACK);
#ifdef INCLUDE_PCCARD_SHOW
    csShowInit ();
#endif /* INCLUDE_PCCARD_SHOW */
#endif /* INCLUDE_PCCARD */

#ifdef	INCLUDE_SCSI

    /*
     * initialize either the SCSI1 or SCSI2 interface; initialize SCSI2 when
     * the SCSI2 interface is available.
     */

#ifndef INCLUDE_SCSI2
    scsi1IfInit ();
#else
    scsi2IfInit ();
#endif

    /* initialize SCSI controller */
    if (sysScsiInit () == OK)
	{
	usrScsiConfig ();			/* configure SCSI peripherals */
	}
    else
	{
#ifdef INCLUDE_STDIO
	printf ("sysScsiInit() Failed, SCSI system not initialized\n");
#endif  /* INCLUDE_STDIO */
	}

#endif	/* INCLUDE_SCSI */

#ifdef  INCLUDE_FD     /* initialize floppy disk driver */
    if ((fdDrv (FD_INT_VEC, FD_INT_LVL)) == ERROR)
    	{
#ifdef INCLUDE_STDIO
	printf ("fdDrv returned ERROR from usrRoot.\n");
#endif /* INCLUDE_STDIO */
	}
    usrFdConfig (0, FD_0_TYPE, FD_DEV_NAME "0");
    usrFdConfig (1, FD_1_TYPE, FD_DEV_NAME "1");
#endif /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
    /* init IDE disk driver */
    if ((ideDrv (IDE_INT_VEC, IDE_INT_LVL, IDE_CONFIG)) == ERROR)
	{
#ifdef INCLUDE_STDIO
	printf ("ideDrv returned ERROR from usrRoot.\n");
#endif /* INCLUDE_STDIO */
	}

#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA
    usrAtaInit ();
    usrAtaConfig (0, 0, FS_NAMES_ATA_PRIMARY_MASTER, ATA_DIRECT_BIO_HANDLING);
    usrAtaConfig (0, 1, FS_NAMES_ATA_PRIMARY_SLAVE, ATA_DIRECT_BIO_HANDLING);
    usrAtaConfig (1, 0, FS_NAMES_ATA_SECONDARY_MASTER, ATA_DIRECT_BIO_HANDLING);
    usrAtaConfig (1, 1, FS_NAMES_ATA_SECONDARY_SLAVE, ATA_DIRECT_BIO_HANDLING);

#ifdef  INCLUDE_SHOW_ROUTINES
    ataShowInit ();                     /* install ATA/IDE show routine */
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif  /* INCLUDE_ATA */

#ifdef INCLUDE_PCCARD
    csClientLoad (CS_ATA_ENABLER);
#endif /* INCLUDE_PCCARD */

#ifdef  INCLUDE_LPT
    {
    IMPORT LPT_RESOURCE lptResources[];
    lptDrv (LPT_CHANNELS, &lptResources[0]); /* init LPT parallel driver */
    }
#endif  /* INCLUDE_LPT */

#ifdef  INCLUDE_PCMCIA
#ifdef  INCLUDE_SHOW_ROUTINES
    pcmciaShowInit ();			/* install PCMCIA show routines */
#endif  /* INCLUDE_SHOW_ROUTINES */
    pcmciaInit ();			/* init PCMCIA Lib */
#endif  /* INCLUDE_PCMCIA */

#ifdef	INCLUDE_TFFS
    tffsDrv ();				/* it should be after pcmciaInit() */
#endif	/* INCLUDE_TFFS */

#ifdef	INCLUDE_TFFS_MOUNT
    usrTffsConfig (TFFS_DRIVE_NUMBER, TFFS_REMOVABLE, TFFS_MOUNT_POINT );
#endif	/* INCLUDE_TFFS_MOUNT */

#ifdef	INCLUDE_FLASH
    flashFsLibInit();
#endif	/* INCLUDE_FLASH */

#ifdef  INCLUDE_FORMATTED_IO
    fioLibInit ();			/* initialize formatted I/O */
#endif  /* INCLUDE_FORMATTED_IO */

    /* initialize floating point facilities */

#ifdef	INCLUDE_FLOATING_POINT
    floatInit ();			/* initialize floating point I/O */
#endif	/* INCLUDE_FLOATING_POINT */

    /* install software floating point emulation (if applicable) */

#ifdef	INCLUDE_SW_FP
    mathSoftInit ();		/* use software emulation for fp math */
#endif	/* INCLUDE_SW_FP */

#if defined (_WRS_PAL_COPROC_LIB) && defined(INCLUDE_SHOW_ROUTINES)
    coprocShowInit();
#endif /* _WRS_PAL_COPROC_LIB && INCLUDE_SHOW_ROUTINES */

#if !defined (_WRS_PAL_COPROC_LIB)
    /* install hardware floating point support (if applicable) */

#ifdef	INCLUDE_HW_FP
    mathHardInit (); 		/* do fppInit() & install hw fp math */

#ifdef	INCLUDE_SHOW_ROUTINES
    fppShowInit ();			/* install hardware fp show routine */
#endif	/* INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_HW_FP */

    /* install dsp support (if applicable) */

#ifdef	INCLUDE_DSP
    usrDspInit (); 			/* do dspInit() */
#endif	/* INCLUDE_DSP */

    /* initialize AltiVec library (if applicable) */

#ifdef	INCLUDE_ALTIVEC
    usrAltivecInit ();
#endif	/* INCLUDE_ALTIVEC */

    /* initialize Spe library (if applicable) */

#ifdef	INCLUDE_SPE
    usrSpeInit ();
#endif	/* INCLUDE_SPE */

#endif /* _WRS_PAL_COPROC_LIB */

   /* Initialize system calls */

#ifdef INCLUDE_RTP

#ifdef INCLUDE_SC_POSIX
    usrPosixScInit ();
#endif

#ifdef INCLUDE_MEM_RTP
    memRtpLibInit (RTP_HEAP_INIT_SIZE, MEM_PART_DEFAULT_OPTIONS);
#endif

#ifdef INCLUDE_SC_KERNEL
    windScInit ();
#endif

#ifdef INCLUDE_SC_MMAN
    mmanScLibInit ();
#endif

#ifdef INCLUDE_SHL
    shlScLibInit ();				/* initialize SL syscalls */
#endif /* INCLUDE_SHL */

#ifdef INCLUDE_SC_IOS
    iosScLibInit ();
#endif

#ifdef  INCLUDE_SC_SYSCTL
    sysctlScLibInit();                          /* Syscall support for sysctl */
#endif /* INCLUDE_SC_SYSCTL */

#ifdef INCLUDE_SC_SD
    sdScInit();
#endif

#endif /* INCLUDE_RTP */

    /* initialize performance monitoring tools */

#ifdef	INCLUDE_SPY
    spyLibInit (SPY_TASKS_MAX);		/* install task cpu utilization tool */
#endif	/* INCLUDE_SPY */

#ifdef	INCLUDE_TIMEX
    timexInit ();			/* install function timing tool */
#endif	/* INCLUDE_TIMEX */

    /* initialize object module loader and unloader */

#ifdef	INCLUDE_LOADER
    moduleLibInit ();			/* initialize module manager */
    loadLibInit (STORE_ABS_SYMBOLS);            /* Initialize loader */
    loadElfInit ();				/* use elf format */
#endif	/* INCLUDE_LOADER */

#ifdef INCLUDE_UNLOADER
    unldLibInit ();
#endif	/* INCLUDE_UNLOADER */

#ifdef INCLUDE_LOADER_HOOKS
    moduleHookLibInit ();
#endif /* INCLUDE_LOADER_HOOKS */

#if defined(INCLUDE_LOADER) && defined(INCLUDE_TLS)
#ifdef	INCLUDE_TLS_LOADER_SUPPORT
    /* Initialize thread local storage support for DKMs */

    tlsLoadLibInit ();
#endif	/* INCLUDE_TLS_LOADER_SUPPORT */
#endif /* INCLUDE_LOADER && INCLUDE_TLS */

#ifdef INCLUDE_USB_MS_BULKONLY_INIT
    usrUsbBulkDevInit (); 	/* Bulk Driver Initialization */

#if (defined(BOOT_USB_POST_MS_BULKONLY_INIT_DELAY))
    /* Allow time for USB discovery */
    taskDelay ( BOOT_USB_POST_MS_BULKONLY_INIT_DELAY );
#endif
#endif /* INCLUDE_USB_MS_BULKONLY_INIT */

#ifdef INCLUDE_USB_MS_CBI_INIT
    usrUsbCbiUfiDevInit ();     /* CBI Driver Initialization */
#if (defined(BOOT_USB_POST_MS_BULKONLY_INIT_DELAY))
    /* Allow time for USB discovery */
    taskDelay ( BOOT_USB_POST_MS_BULKONLY_INIT_DELAY );
#endif
#endif

#ifdef INCLUDE_USB_GEN2_STORAGE_INIT
    usrUsb2MscInit ();      /* GEN2 Storage Driver Initialization */

#if (defined(BOOT_USB_POST_MS_BULKONLY_INIT_DELAY))
    /* Allow time for USB discovery */
    taskDelay ( BOOT_USB_POST_MS_BULKONLY_INIT_DELAY );
#endif
#endif

/* if networking is not included as part of the bootrom, still
 * initialize the INCLUDE_BOOT_LINE_INIT component if included.
 */
#if (defined(INCLUDE_BOOT_APP) && defined(INCLUDE_BOOT_LINE_INIT))
#if (!defined(INCLUDE_NETWORK) && !defined(INCLUDE_COMMON_NET))
    usrBootLineInit (sysStartType);	/* crack the bootline */
#endif   /* !INCLUDE_NETWORK && !INCLUDE_COMMON_NET */
#endif   /* INCLUDE_BOOT_APP && INCLUDE_BOOT_LINE_INIT */

#ifdef INCLUDE_VXBUS_END_QCTRL
    /* The init routine only requires semaphores, but the utility
     * isn't ready to use until the network daemons have been initialized.
     */
    vxbEndQctrlInit ();
#endif /* INCLUDE_VXBUS_END_QCTRL */

/*
 * Initialize network if network support is included. If network support is not
 * included call usrNetworkCommonInit() which initializes common components
 * like sockets, sysctl if support for these has been included.
 */

#if  defined(INCLUDE_NETWORK) || defined(INCLUDE_COMMON_NET)
#ifdef  INCLUDE_NET_INIT
#ifdef	INCLUDE_BOOT_LINE_INIT
    usrBootLineInit (sysStartType);	/* crack the bootline */
#endif /* INCLUDE_BOOT_LINE_INIT */
# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("before usrNetInit()\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */
    usrNetInit (BOOT_LINE_ADRS);	/* initialize network support */
# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("after usrNetInit()\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */
#endif	/* INCLUDE_NET_INIT */
#else   /* INCLUDE_NETWORK */
    usrNetworkCommonInit ();
#endif  /* INCLUDE_NETWORK || INCLUDE_COMMON_NET */

#ifdef INCLUDE_PASSFS
    usrPassFsInit (PASSFS_CACHE);
#endif	/* INCLUDE_PASSFS */

/* initialize USB components */

#ifdef INCLUDE_USB_MOUSE_INIT
    usrUsbMseInit ();       /* Mouse Driver Initialization */
#endif

#ifdef INCLUDE_USB_PRINTER_INIT
    usrUsbPrnInit ();       /* Printer Driver Initialization */
#endif

#ifdef INCLUDE_USB_SPEAKER_INIT
    usrUsbSpkrInit (); 		/* Speaker Driver Initialization */
    #ifdef INCLUDE_USB_HEADSET_DEMO
        usbBrcmAudioStart();
    #endif
#endif

#ifdef INCLUDE_USB_AUDIO_DEMO
    usrUsbAudioDemo (); 	/* USB Audio Demo */
#endif

#ifdef INCLUDE_USB_PEGASUS_END_INIT
    usrUsbPegasusEndInit ();    /* Pegasus Driver Initialization */
#endif

#ifdef INCLUDE_USB_GEN2_HELPER
    usrUsb2HelperInit();    /* GEN2 Helper Initialization */
#endif

#ifdef INCLUDE_USB_GEN2_PRINTER_INIT
    usrUsb2PrnInit ();      /* GEN2 Printer Driver Initialization */
#endif

#ifdef INCLUDE_USB_GEN2_MOUSE_INIT
    usrUsb2MseInit ();      /* GEN2 Mouse Driver Initialization */
#endif

#ifdef INCLUDE_USB_GEN2_KEYBOARD_INIT
    usrUsb2KbdInit ();      /* GEN2 Keyboard Driver Initialization */
#endif

#ifdef INCLUDE_USB_GEN2_END_INIT
    usrUsb2EndInit ();         /* GEN2 General Usb-Ethernet Driver Initialization */
#endif



#ifdef INCLUDE_PHILIPS1582
    /* The following is to workaround the PCI interrupt issue with ISP1582 */
    sysIsp1582PciDisconnect ();
#endif

#ifdef INCLUDE_MS_EMULATOR_INIT

    /* Peripheral Mass Storage driver initialization */
    usbTargMsInit();

#endif

#ifdef INCLUDE_KBD_EMULATOR_INIT
    usrUsbTargKbdInit ();        /* Peripheral Keyboard driver initialization */
#endif

#ifdef INCLUDE_PRN_EMULATOR_INIT
    usrUsbTargPrnInit ();        /* Peripheral Printer driver initialization */
#endif

#ifdef	INCLUDE_DOS_DISK
    usrDosDiskInit ();
#endif	/* INCLUDE_DOS_DISK */

#ifdef INCLUDE_VIRTUAL_DISK
    virtualDiskInit ();
#endif /* INCLUDE_VIRTUAL_DISK */

    /* initialize shared memory objects */

#ifdef INCLUDE_SM_OBJ			/* unbundled shared memory objects */
    usrSmObjInit (BOOT_LINE_ADRS);
#endif /* INCLUDE_SM_OBJ */

    /*
     * write protect vector table only after shared memory has been
     * initialized. The anchor may shares a page with it.
     */

#if	defined(INCLUDE_MMU_BASIC)
#ifdef	INCLUDE_PROTECT_VEC_TABLE
    intVecTableWriteProtect ();
#endif	/* INCLUDE_PROTECT_VEC_TABLE */
#endif	/* INCLUDE_MMU_BASIC */

    /* install select hook only after NFS/RPC for proper delete hook order */

#ifdef	INCLUDE_SELECT
    selTaskDeleteHookAdd ();
#endif	/* INCLUDE_SELECT */

    /* create system and status symbol tables */

#if defined(INCLUDE_SYM_TBL_INIT)
# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("before symTblCreate()\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */
    usrSysSymTblInit ();
# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("sysSymTbl initialization complete.\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */
#endif	/* INCLUDE_SYM_TBL_INIT */

#ifdef  INCLUDE_STANDALONE_SYM_TBL
    usrStandaloneInit ();
#elif (defined(INCLUDE_NET_SYM_TBL) || defined(INCLUDE_PASSFS_SYM_TBL))
    usrLoadSyms ();
#endif	/* INCLUDE_NET_SYM_TBL || INCLUDE_PASSFS_SYM_TBL */

#ifdef  INCLUDE_STAT_SYM_TBL
    usrStatTblInit ();
#endif	/* INCLUDE_STAT_SYM_TBL */

    /* initialize C++ support library */

#if	defined (INCLUDE_CPLUS) && defined (INCLUDE_CPLUS_MIN)
#error	Define only one of INCLUDE_CPLUS or INCLUDE_CPLUS_MIN, not both
#endif

#if	defined (INCLUDE_CPLUS) || defined (INCLUDE_CPLUS_MIN)
#ifndef INCLUDE_CTORS_DTORS
#define INCLUDE_CTORS_DTORS
#endif
#endif

#ifdef  INCLUDE_CTORS_DTORS
  /*
   * call compiler generated init functions (usually - but not necessarily -
   * C++ related)
   */
   cplusCtorsLink ();
#endif

#ifdef	INCLUDE_CPLUS			/* all standard C++ runtime support */
    cplusLibInit ();
#endif

#ifdef	INCLUDE_CPLUS_MIN		/* minimal C++ runtime support */
    cplusLibMinInit ();
#endif

#ifdef INCLUDE_CPLUS_DEMANGLER
    cplusDemanglerInit ();
#endif

    /* initialize COM/DCOM runtime support */
#ifdef INCLUDE_COM
    comLibInit ();
#endif
#ifdef INCLUDE_DCOM
    dcomLibInit ();
#endif

    /* initialize Wind Web Server */

#ifdef INCLUDE_HTTP
    httpd ();
#endif /* INCLUDE_HTTP */

#ifdef INCLUDE_EVENTPOINT
    eventPointLibInit ();
#endif

#ifdef INCLUDE_EVENTPOINT_STUB
    eventPointStubLibInit ();    
#endif

#ifdef INCLUDE_DATACOLLECTOR
    dataCollectorLibInit ();
#endif

#ifdef INCLUDE_SV_DATACOLLECTOR
    svDataCollectorLibInit ();
#endif

#ifdef INCLUDE_TRIG_DATACOLLECTOR
    trgDataCollectorLibInit ();
#endif

#ifdef INCLUDE_WINDVIEW
#ifdef INCLUDE_WVUPLOAD_SOCK
    wvSockUploadPathLibInit ();
#endif /* INCLUDE_WVUPLOAD_SOCK */

#ifdef INCLUDE_WVUPLOAD_FILE
    wvFileUploadPathLibInit ();
#endif /* INCLUDE_WVUPLOAD_FILE */

#ifdef INCLUDE_WVUPLOAD_TSFSSOCK
    wvTsfsUploadPathLibInit ();
#endif /* INCLUDE_WVUPLOAD_TSFSSOCK */

    windviewConfig ();

#ifdef   INCLUDE_RBUFF                  /* install rBuff support */
    rBuffLibInit();
#ifdef  INCLUDE_SHOW_ROUTINES
    rBuffShowInit ();                   /* install rBuff show routine */
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif /* INCLUDE_RBUFF */

#ifdef INCLUDE_WVNETD
    wvNetDInit();
#endif /* INCLUDE_WVNETD */

#if defined (WV_INSTRUMENTATION) && defined (INCLUDE_WVTIPC)
    wvTipcInit();
#endif /* INCLUDE_WVTIPC */

/* If WindView included, must have a timestamp of some sort */
#if defined (INCLUDE_SYS_TIMESTAMP) || defined (INCLUDE_POSIX_TRACE)
    wvTmrRegister ((UINTFUNCPTR) sysTimestamp,
		   (UINTFUNCPTR) sysTimestampLock,
		   (FUNCPTR)     sysTimestampEnable,
		   (FUNCPTR)     sysTimestampDisable,
		   (FUNCPTR)     sysTimestampConnect,
		   (UINTFUNCPTR) sysTimestampPeriod,
		   (UINTFUNCPTR) sysTimestampFreq);
#else /* INCLUDE_SYS_TIMESTAMP */
    wvTmrRegister ((UINTFUNCPTR) seqStamp,
		   (UINTFUNCPTR) seqStampLock,
		   (FUNCPTR)     seqEnable,
		   (FUNCPTR)     seqDisable,
		   (FUNCPTR)     seqConnect,
		   (UINTFUNCPTR) seqPeriod,
		   (UINTFUNCPTR) seqFreq);
#endif /* INCLUDE_SYS_TIMESTAMP */

#endif /* INCLUDE_WINDVIEW */

#ifdef INCLUDE_POSIX_TRACE
    traceTmrRegister ((UINTFUNCPTR) sysTimestamp,
		      (FUNCPTR)sysTimestampEnable,
		      (FUNCPTR)sysTimestampDisable,
		      (FUNCPTR)sysTimestampConnect,
		      (UINTFUNCPTR)sysTimestampPeriod,
		      (UINTFUNCPTR)sysTimestampFreq);

    pxTraceStreamLibInit ();
#endif /* INCLUDE_POSIX_TRACE */

    /* Include Sysctl command line support */

#ifdef INCLUDE_SYSCTL_CLI
    sysctlLibInit();                               /* Command line SYSCTL */
#endif /* INCLUDE_SYSCTL_CLI */

    /* initialize the WDB debug agent */

#if	defined (INCLUDE_WDB) && (WDB_INIT == WDB_POST_KERNEL_INIT)
# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("before usrWdbInit()\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */
    usrWdbInit();
# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("after usrWdbInit()\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */

#ifdef	INCLUDE_WDB_BANNER
#ifndef INCLUDE_SHELL
    usrWdbBanner ();
#endif /* INCLUDE_SHELL */
#endif /* INCLUDE_WDB_BANNER */

#endif  /* defined (INCLUDE_WDB) && (WDB_INIT == WDB_POST_KERNEL_INIT) */

/* initialize SNS */

#ifdef INCLUDE_SNS
# undef INCLUDE_SNS_RTP
# undef INCLUDE_SNS_MP
# undef INCLUDE_SNS_MP_RTP
    salSnsTaskInit ();
#endif

#ifdef INCLUDE_SNS_RTP
# undef INCLUDE_SNS_MP
# undef INCLUDE_SNS_MP_RTP
    salSnsRtpInit ();
#endif

#ifdef INCLUDE_SNS_MP
# undef INCLUDE_SNS_MP_RTP
    salSnsTaskInit ();
#endif

#ifdef INCLUDE_SNS_MP_RTP
    salSnsRtpInit ();
#endif

#ifdef INCLUDE_TIP
    usrTipInit (TIP_CONFIG_STRING, TIP_ESCAPE_CHARACTER);
#endif

    /* initialize interactive shell */

#ifdef  INCLUDE_SHELL
#ifdef	INCLUDE_SECURITY			/* include shell security */

    /* initialize login table */
    loginInit ();

    /* add additional users here as required */
#if defined(LOGIN_USER_NAME) && defined(LOGIN_PASSWORD)
    loginUserAdd (LOGIN_USER_NAME, LOGIN_PASSWORD);
#endif

    if ((sysFlags & SYSFLG_NO_SECURITY) == 0)
	{
	/* install security program */
	shellLoginInstall (loginPrompt2, 0);
	}
#endif	/* INCLUDE_SECURITY */

    /*
     * Statically initialize the shell core. Error message is handled by
     * shellLibInit().
     */

    shellLibInit ();

#ifdef INCLUDE_SHELL_HISTORY_FILE
    /* 
     * Statically initialize the shell history saving/loading facility. The
     * error message is handled by shellHistLibInit().
     */

    shellHistLibInit ();
#endif

    /*
     * Print out the shell banner. It also acts as the WDB banner in case both
     * INCLUDE_SHELL_BANNER and INCLUDE_WDB_BANNER are defined.
     */
#if !defined(BROADCOM_BSP)
    usrBanner ();
#endif

#if (!defined(INCLUDE_SHELL_EMACS_MODE) && !defined(INCLUDE_SHELL_VI_MODE)) || defined (INCLUDE_SHELL_VI_MODE)
    /*
     * Editing mode similar to the vi editing mode - this will be the
     * default mode
     */

    ledModeRegister (viLedLibInit);
#endif

#ifdef	INCLUDE_SHELL_EMACS_MODE
    /* Editing mode similar to the emacs editing mode */

    ledModeRegister (emacsLedLibInit);
#endif

#if (!defined(INCLUDE_SHELL_INTERP_CMD) && !defined(INCLUDE_SHELL_INTERP_C)) || defined (INCLUDE_SHELL_INTERP_C)
    /* The C interpreter for the target-resident shell */

    shellInterpRegister (shellInterpCInit);
#endif

#ifdef	INCLUDE_SHELL_INTERP_CMD
    /* The command interpreter for the target-resident shell */

    shellInterpRegister (shellInterpCmdInit);

#ifdef INCLUDE_DISK_UTIL
    /* Add the shell commands for the file system support */

    usrFsShellCmdInit ();
#endif

#if defined (INCLUDE_VM_SHOW) || \
    defined (INCLUDE_SHOW_ROUTINES) && defined (INCLUDE_MMU_BASIC)
    vmShowShellCmdInit ();	/* initialize shell commands for vmShow */
#endif

#ifdef INCLUDE_HISTORY_FILE_SHELL_CMD
    /* Add the history management commands */

    histShellCmdInit ();

#endif

#ifdef	INCLUDE_RTP

#ifdef INCLUDE_DEBUG
    vxdbgRtpLibInit ();		/* initialize process debugging */
#endif	/* INCLUDE_DEBUG */

    /* Add the shell commands for the RTP */

    rtpShellCmdInit ();		/* initialize shell commands for RTPs */
#ifdef INCLUDE_SHOW_ROUTINES
    {
    extern STATUS rtpShowShellCmdInit(void);
    rtpShowShellCmdInit();	/* initialize shell commands for RTP info */
    }
#endif /* INCLUDE_RTP_SHOW */
#ifdef INCLUDE_SHL
    shlShellCmdInit ();		/* initialize shell commands for SHLs */
#endif /* INCLUDE_SHL */
#if defined (INCLUDE_SHARED_DATA_SHOW) || \
    defined (INCLUDE_SHARED_DATA) && defined (INCLUDE_SHOW_ROUTINES)
    sdShowShellCmdInit ();	/* initialize shell commands for SDs */
#endif /* INCLUDE_SHARED_DATA_SHOW */
#if defined (INCLUDE_ADR_SPACE_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    adrSpaceShellCmdInit();	/* initialize shell commands for adrSpaceShow */
#endif /* INCLUDE_ADR_SPACE_SHOW */
#if defined (INCLUDE_MAPPED_FILES_SHOW)
    mmanShowShellCmdInit();		/* initialize shell commands for mmanShow */
#endif /* INCLUDE_MAPPED_FILES_SHOW */
#endif /* INCLUDE_RTP */

#ifdef	INCLUDE_EDR_SHOW
    /* Add the shell commands for the EDR */

    edrShellCmdInit ();
#endif

#ifdef INCLUDE_MEM_EDR_SHOW
    memEdrShellCmdInit();	/* initialize shell commands for memEdrLib */
#endif

#ifdef INCLUDE_MEM_EDR_RTP_SHOW
    memEdrRtpShellCmdInit();	/* initialize shell commands for memEdrLib */
#endif

#if defined (INCLUDE_RTP) && defined (INCLUDE_RTP_DEBUG_COMMAND)
    rtpDbgCmdLibInit ();	/* initialize the RTP debug command facility */
#endif

#ifdef INCLUDE_LOADER
    /* Add the shell commands for the loader */

    moduleShellCmdInit ();
#endif

#ifdef INCLUDE_UNLOADER
    /* Add the shell commands for the unloader */

    unloadShellCmdInit ();
#endif

#ifdef INCLUDE_DEBUG
    /* Add the shell commands for the debug */

    dbgShellCmdInit ();
#endif

#ifdef INCLUDE_SYM_TBL
    /* Add the shell commands for the symbols */

    symShellCmdInit ();
#endif

    /* Add the shell commands for the tasks */

    taskShellCmdInit ();

#endif	/* INCLUDE_SHELL_INTERP_CMD */

#ifdef	INCLUDE_RTP
    usrRtpLibInit ();
#endif

#ifdef	INCLUDE_STARTUP_SCRIPT			/* run a startup script */
    usrStartupScript (startupScriptFieldSplit (sysBootParams.startupScript));
#endif	/* INCLUDE_STARTUP_SCRIPT */

#if !defined(BROADCOM_BSP)
    usrShell ();
#endif

    /* only include the simple demo if the shell is NOT included */

#else
#if defined(INCLUDE_DEMO)			/* create demo w/o shell */
    taskSpawn ("demo", 20, 0, 2000, (FUNCPTR)usrDemo, 0,0,0,0,0,0,0,0,0,0);
#endif						/* mips cpp no elif */

#endif	/* INCLUDE_SHELL */

#ifdef	INCLUDE_WDB_PROXY
#ifdef	INCLUDE_WDB_PROXY_UDP
    wrProxyUdpSockRegister ();
#endif	/* INCLUDE_WDB_PROXY_UDP */
#ifdef	INCLUDE_WDB_PROXY_TIPC
    wrProxyTipcSockRegister ();
#endif	/* INCLUDE_WDB_PROXY_TIPC */
#ifdef	INCLUDE_WDB_PROXY_MIPC
    wrProxyMipcSockRegister ();
#endif	/* INCLUDE_WDB_PROXY_TIPC */
    wrProxyLibInit (WDB_PROXY_PORT, WDB_PROXY_INIT_STRING,
    			WDB_PROXY_HAS_CONTROL_PORT, WDB_PROXY_TASK_PRIORITY,
    			WDB_PROXY_TASK_STACK_SIZE, WDB_PROXY_TASK_OPTIONS);
#endif	/* INCLUDE_WDB_PROXY */

#ifdef  INCLUDE_WINDML
    usrWindMlInit ();
#endif  /* INCLUDE_WINDML */

#if defined (INCLUDE_SOUND) && defined(INCLUDE_SB16)
    sb16Drv ();            /* install sound driver SB16 */

    sb16DevCreate ("/sound", 0x220, 5, 1, 5);
#endif /* INCLUDE_SOUND && INCLUDE_SB16 */

#if defined(INCLUDE_JAVA)
    javaConfig ();
#endif	/* INCLUDE_JAVA */

#ifdef INCLUDE_HTML
    usrHtmlInit ();
#endif /* INCLUDE_HTML */


#ifdef INCLUDE_POWER_MGMT_CPU_BSP_SUPPORT
    /* turn on power management */

    sysOneShotInit ();
    sysCpuPwrEnable ();
#endif


    /* BOOTAPP starts just before USER_APPL, will not return */

#ifdef INCLUDE_BOOT_APP
#ifdef INCLUDE_BOOT_EXC_HANDLER
    bootAppExcInit();
#endif
#if defined(INCLUDE_BOOT_INIT)
    bootAppInit (FALSE, LOCAL_MEM_LOCAL_ADRS, BOOT_DISABLE_MMU_BEFORE_ENTRY);
#elif defined(INCLUDE_BOOT_DELAYED_INIT)
    bootAppInit (TRUE, LOCAL_MEM_LOCAL_ADRS, BOOT_DISABLE_MMU_BEFORE_ENTRY);
#endif
#ifdef INCLUDE_ELF
    bootElfInit ();
#endif
#ifdef INCLUDE_BOOT_SHELL
    bootAppShellInit ();
#ifdef INCLUDE_BOOT_MEM_CMDS
    bootAppMemInit (LOCAL_MEM_LOCAL_ADRS);
#endif
#ifdef INCLUDE_BOOT_EDR_SUPPORT
    bootAppEdrInit ();
#endif
#endif /* INCLUDE_BOOT_SHELL */

/* initialize filesystems boot components */
#ifdef INCLUDE_BOOT_FILESYSTEMS
    bootFsLoadInit ();
#ifdef INCLUDE_BOOT_FD_LOADER
    bootFdLoadInit (FD_DEV_NAME);
#endif
#ifdef INCLUDE_BOOT_USB_FS_LOADER
    bootUsbFsLoadInit (BOOT_USB_OPEN_ATTEMPTS, BOOT_USB_ATTEMPT_DELAY);
#endif
#ifdef INCLUDE_BOOT_TSFS_LOADER
    bootTsfsLoadInit ();
#endif
#endif /* INCLUDE_BOOT_FILESYSTEMS */

#ifdef INCLUDE_UEFI_BOOT_SUPPORT
    bootUefiInit ();
#endif

/* initialize Network boot components after Filesystems */
#ifdef INCLUDE_BOOT_DHCPC
    bootDhcpcLoadInit ();
#endif
#ifdef INCLUDE_BOOT_NETWORK
    bootNetLoadInit ();
#ifdef INCLUDE_BOOT_FTP_LOADER
    bootFtpLoadInit ();
#endif
#ifdef INCLUDE_BOOT_TFTP_LOADER
    bootTftpLoadInit ();
#endif
#ifdef INCLUDE_BOOT_RSH_LOADER
    bootRshLoadInit ();
#endif
#ifdef INCLUDE_BOOT_END
    bootEndSupportInit ();
#endif
#ifdef INCLUDE_BOOT_VXBUS
    bootVxBusSupportInit ();
#endif
#ifdef INCLUDE_BOOT_ETH_MAC_HANDLER
    bootEthMacHandlerInit (MAX_MAC_ADRS,  MAX_MAC_DEVS,
                           ENET_DEFAULT0, ENET_DEFAULT1, 
                           ENET_DEFAULT2, MAC_ADRS_LEN);
#endif
#ifdef INCLUDE_BOOT_ETH_ADR_SET
    bootEthAdrSetInit (ENET_DEFAULT0, ENET_DEFAULT1, ENET_DEFAULT2);
#endif
#endif /* INCLUDE_BOOT_NETWORK */
    bootApp();
#endif /* INCLUDE_BOOT_APP */

     /* Initialize the usb keyboard after shell Initialization */

#ifdef INCLUDE_USB_KEYBOARD_INIT
    usrUsbKbdInit (); 		/* Keyboard Driver Initialization */
#endif /* INCLUDE_USB_KEYBOARD_INIT */

/*  usrMmuOptimize needs to be called before the additional CPU's are enabled.*/
#ifdef INCLUDE_MMU_OPTIMIZE
    usrMmuOptimize();
#endif /* INCLUDE_MMU_OPTIMIZE */

/* USER_APPL is the second last initialization step */

#ifdef INCLUDE_USER_APPL
    /* Startup the user's application */

    USER_APPL_INIT;	/* must be a valid C statement or block */
#endif

    /* The RTP Startup Facility is always the last one to initialize */

#ifdef INCLUDE_RTP

#ifdef INCLUDE_RTP_APPL_USER
    usrRtpAppInit ();
#endif /* INCLUDE_RTP_APPL_USER */

#ifdef INCLUDE_RTP_APPL_INIT_STRING
    usrRtpAppInitString ();
#endif /* INCLUDE_RTP_APPL_INIT_STRING */

#ifdef INCLUDE_RTP_APPL_INIT_BOOTLINE
    usrRtpAppInitBootline ();
#endif /* INCLUDE_RTP_APPL_INIT_BOOTLINE */

#ifdef INCLUDE_RTP_APPL_INIT_CMD_SHELL_SCRIPT
#ifndef RTP_APPL_CMD_SCRIPT_FILE
#define RTP_APPL_CMD_SCRIPT_FILE NULL
#endif
    usrRtpAppInitCmdShellScriptRun (RTP_APPL_CMD_SCRIPT_FILE);
#endif /* INCLUDE_RTP_APPL_INIT_CMD_SHELL_SCRIPT */

#endif /* INCLUDE_RTP */
    }

/*******************************************************************************
*
* usrClock - user-defined system clock interrupt routine
*
* This routine is called at interrupt level on each clock interrupt.
* It is installed by usrRoot() with a sysClkConnect() call.
* It calls all the other packages that need to know about clock ticks,
* including the kernel itself.
*
* If the application needs anything to happen at the system clock interrupt
* level, it can be added to this routine.
*
* RETURNS: N/A
*/

void usrClock ()

    {
    tickAnnounce ();	/* announce system tick to kernel */
    }

#ifdef	INCLUDE_DEMO

/********************************************************************************
* usrDemo - example application without shell
*
* This routine is spawned as a task at the end of usrRoot(), if INCLUDE_DEMO
* is defined, and INCLUDE_SHELL is NOT defined in configAll.h or config.h.
* It is intended to indicate how a shell-less application can be linked,
* loaded, and ROMed.
*
* NOMANUAL
*/

void usrDemo (void)

    {
    char string [40];

    printf ("VxWorks (for %s) version %s.\n", sysModel (), vxWorksVersion);
    printf ("Kernel: %s.\n", kernelVersion ());
    printf ("Made on %s.\n", creationDate);

#ifdef INCLUDE_UEFI_BOOT_SUPPORT
	printf ("This image is %s.\n", uefiComplianceInfoP );
	printf ("UEFI Data block located at 0x%X\n", pSysUefiMemAddr );

	/* check for UEFI memory map data block not valid */
	if (pSysUefiMemAddr == NULL)
	{
		printf ("WARNING - UEFI Data block pointer at 0x%X is NULL\n", &pSysUefiMemAddr );
	}
	else if (pSysUefiMemAddr->HeaderSize != VXWORKS_OS_MEMORY_HEADER_8BYTE_SIZE)
	{
		printf ("WARNING - UEFI data block at 0x%X has invalid header size of %u\n", 
				pSysUefiMemAddr, pSysUefiMemAddr->HeaderSize );
	}
    else if (pSysUefiMemAddr->PhysMemoryMapBuffer != (EFI_PHYSICAL_ADDRESS) (int)pSysUefiMemAddr)
	{
		printf ("WARNING - UEFI data block at 0x%X, but table value is 0x%lX", 
				pSysUefiMemAddr, pSysUefiMemAddr->PhysMemoryMapBuffer );
	}
#endif

    FOREVER
	{
	printf ("\nThis is a test.  Type something: ");
	fioRdString (STD_IN, string, sizeof (string));
	printf ("\nYou typed \"%s\".\n", string);

	if (strcmp (string, "0") == 0)
	    memShow (0);

	if (strcmp (string, "1") == 0)
	    memShow (1);
	}
    }

#endif	/* INCLUDE_DEMO */
