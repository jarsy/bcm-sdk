/* usrConfig.c - user-defined system configuration library */

/* $Id: usrConfig.c,v 1.16 2011/07/21 16:14:11 yshtil Exp $
 * Copyright (c) 1984-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
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
source files are located in ../../src/config/usr[xxx].c and are #included into
this file below.  This file contains the bulk of the code a customer is
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
.tG "Getting Started, Cross-Development"
*/

#if VX_VERSION == 62

#include "vxWorks.h"			/* always first */
#include "config.h"			/* board support configuration header */
#include "usrConfig.h"			/* general configuration header */
#include "usrDepend.c"			/* include dependency rules */
#include "usrKernel.c"			/* kernel configuration */
#include "usrExtra.c"			/* conditionally included packages */

#ifdef INCLUDE_HTTP
#include <http/httpLib.h>		/* Wind Web Server interfaces */
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

#ifdef INCLUDE_RTP
#include <rtpLib.h>
#include <syscallTbl.h>
extern char * pRtpStartupField;

#ifdef INCLUDE_SHL
#include <shlLib.h>
extern STATUS shlLibInit (void);
extern STATUS shlScLibInit (void);
#endif /* INCLUDE_SHL */

#if defined (INCLUDE_ADR_SPACE_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
extern STATUS adrSpaceShellCmdInit (void);
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
extern void memRtpLibInit (UINT rtpHeapSize);
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

#ifdef INCLUDE_SC_SYSCTL
extern STATUS sysctlScLibInit (void);
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
#endif

#ifdef INCLUDE_POWER_MGMT_CPU_BSP_SUPPORT
void sysCpuPwrEnable (void);
#endif
#if (defined (INCLUDE_CPU_PWR_MGMT) && (CPU_FAMILY == I80X86))
#include <cpuPwrMgr.h>
#include <cpuPwrLib.h>
#include <private/cpuPwrLibP.h>
#endif

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

#if     defined(INCLUDE_LOADER)
#include <loadElfLib.h>
#endif

#if     defined(INCLUDE_NET_SYM_TBL)
#include <private/loadLibP.h>
#endif

#ifdef	INCLUDE_STANDALONE_SYM_TBL
#include <symbol.h>
#endif	/* INCLUDE_STANDALONE_SYM_TBL */

#ifdef INCLUDE_POOL
#include <poolLib.h>
#endif

#ifdef INCLUDE_SET
#include <setLib.h>
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
extern STATUS   excInit (int maxIsrJobs);
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
extern STATUS xbdBlkDevLibInit (void);
#endif

#ifdef INCLUDE_FS_MONITOR
extern STATUS fsMonitorInit (void);
#endif

#ifdef INCLUDE_FS_EVENT_UTIL
extern STATUS fsEventUtilLibInit (void);
#endif

#ifdef INCLUDE_HRFS_FORMAT
extern STATUS hrfsFormatLibInit (void);
#endif

#ifdef INCLUDE_DOSFS_MAIN
#include "usrDosfs.c"
#endif

#if defined (INCLUDE_HRFS) || defined (INCLUDE_HRFS_READONLY)
#include "usrHrfs.c"
#endif

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
#endif  /* (CPU_FAMILY == MIPS) */

#ifdef INCLUDE_FLASH
#ifdef	BROADCOM_BSP
IMPORT STATUS tffsPPCInit(int forceFormat);
IMPORT STATUS sysHasDOC();
#endif
#endif

/* global variables */

int		consoleFd;		/* fd of initial console device */
char		consoleName[20];	/* console device name, eg. "/tyCo/0" */
SYMTAB_ID	statSymTbl;		/* system error status symbol table id*/
SYMTAB_ID	standAloneSymTbl;	/* STANDALONE version symbol table id */
SYMTAB_ID	sysSymTbl;		/* system symbol table id */
BOOT_PARAMS	sysBootParams;		/* parameters from boot line */
int		sysStartType;		/* type of boot (WARM, COLD, etc) */
char *		memPoolEndAdrs;		/* end of the kernel memory pool */
#ifdef INCLUDE_PROTECT_TEXT
int		sysTextProtect = 1;
#else  /* INCLUDE_PROTECT_TEXT */
int		sysTextProtect = 0;
#endif /* INCLUDE_PROTECT_TEXT */

#if	(CPU_FAMILY == PPC)
IMPORT _RType taskMsrDefault;   /* from taskArchLib.c */
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

#ifdef BRINGUP
extern void sysSerialPrintStringNL(char* msg);
extern void sysLedDsply(char* msg);
#endif

void usrInit
    (
    int startType
    )
    {
#ifdef BRINGUP
    if ((SYS_REVID_GET()) == ID_CFM_1)
        bringupPrintRtn = sysSerialPrintStringNL;
    else
        bringupPrintRtn = sysLedDsply;
#else
    bringupPrintRtn = NULL;
#endif
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
    
#if (CPU_FAMILY == MIPS)
    palInit ();
#endif	/* (CPU_FAMILY == MIPS) */

#if	(CPU_FAMILY == SPARC)
    excWindowInit ();				/* SPARC window management */
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
    excVecInit ();				/* install exception vectors */
#endif  /* INCLUDE_EXC_HANDLING */

    sysHwInit ();				/* initialize system hardware */
 
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

    usrKernelInit ();				/* configure the Wind kernel */

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

    /* start the kernel specifying usrRoot as the root task */

    kernelInit ((FUNCPTR) usrRoot, ROOT_STACK_SIZE,
		(char *) MEM_POOL_START_ADRS, memPoolEndAdrs,
		ISR_STACK_SIZE, INT_LOCK_LEVEL);
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
#if defined (INCLUDE_CPU_PWR_MGMT) && (CPU_FAMILY == I80X86)
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
#endif
#endif

    /* Initialize the memory pool before initializing any other package.
     * The memory associated with the root task will be reclaimed at the
     * completion of its activities.
     */

#ifdef INCLUDE_MEM_ALLOT
    memAllotLibInit (pMemPoolStart, memPoolSize);      /* initialize pool */
#else  /* INCLUDE_MEM_ALLOT */
# ifdef INCLUDE_MEM_MGR_FULL
    memInit (pMemPoolStart, memPoolSize);	/* initialize memory pool */
# else
    memPartLibInit (pMemPoolStart, memPoolSize);/* initialize memory pool */
# endif /* INCLUDE_MEM_MGR_FULL */
#endif  /* INCLUDE_MEM_ALLOT */

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

#ifdef INCLUDE_USB
    usbdPciInit ();
#endif

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
                TASK_USER_EXEC_STACK_UNDERFLOW_SIZE);

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

#ifdef INCLUDE_RTP
#ifdef INCLUDE_PG_MGR_LIB
    pgMgrLibInit();    /* always returns OK */
#endif /* INCLUDE_PG_MGR_LIB */

#if	defined (INCLUDE_RTP_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    pgPoolShowInit ();				/* initialize pgPoolShow lib */
    pgMgrShowInit ();				/* initialize pgMgrShow lib */
#endif /* defined(INCLUDE_RTP_SHOW) */

#if	defined (INCLUDE_ADR_SPACE_SHOW) || defined (INCLUDE_SHOW_ROUTINES)
    adrSpaceShowInit ();		/* initialize adrSPaceShow lib */
#endif /* defined(INCLUDE_ADR_SPACE_SHOW) */
#endif /* INCLUDE_RTP */

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

#ifdef EDR_ERROR_INJECT_STUBS
    /* Now inject a 'boot' ED&R event - this takes place outside
     * the INCLUDE_EDR guards, as the stub is always included.
     */
    edrStubInit ();
#endif /* EDR_ERROR_INJECT_STUBS */

#ifdef INCLUDE_CORE_DUMP
    usrCoreDumpInit ();
#endif	/* INCLUDE_CORE_DUMP */

    /* set up system timer */

    sysClkConnect ((FUNCPTR) usrClock, 0);	/* connect clock ISR */
    sysClkRateSet (SYS_CLK_RATE);	/* set system clock rate */
    sysClkEnable ();				/* start it */

#ifdef INCLUDE_FAST_DRAM
/*
 * make use of data cache as fast DRAM,
 * establish parameters in config.h, MMU
 * must be initialed before data cache is
 * initialized as data ram...
 */
  cacheCreateInternalDataRAM((UINT32 *)FD_ORIGIN, FD_NUMLINES);
#endif

#ifdef 	INCLUDE_SET
    setLibInit ();
#endif

    /* initialize I/O system */

#ifdef  INCLUDE_IO_SYSTEM
    iosInit (NUM_DRIVERS, NUM_FILES, "/null");

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

#ifdef INCLUDE_TYLIB
    tyLibInit (TYLIB_XOFF_PCNT, TYLIB_XON_PCNT, TYLIB_WRT_THRESHLD);
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

    ioGlobalStdSet (STD_IN,  consoleFd);
    ioGlobalStdSet (STD_OUT, consoleFd);
    ioGlobalStdSet (STD_ERR, consoleFd);
#endif  /* INCLUDE_IO_SYSTEM */

#if defined (_WRS_PAL_COPROC_LIB)
    coprocLibInit();
#ifdef INCLUDE_HW_FP
    aimFppLibInit();        /* dummy function to include fppTaskRegsSet/Get */
#endif
#endif /* _WRS_PAL_COPROC_LIB */

    /* initialize symbol table facilities */

#ifdef	INCLUDE_SYM_TBL
    hashLibInit ();			/* initialize hash table package */
    symLibInit ();	                /* initialize symbol table package */
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

#ifdef	INCLUDE_SIGNALS
    sigInit ();				/* initialize signals */
#endif	/* INCLUDE_SIGNALS */

#ifdef	INCLUDE_POSIX_CLOCKS
    clockLibInit ();			/* initialize clock */
#endif	/* INCLUDE_POSIX_CLOCKS */

#ifdef	INCLUDE_POSIX_TIMERS
    timerLibInit ();			/* initialize timers */
#ifdef INCLUDE_OBJ_OPEN
    timerOpenInit ();
#endif
#endif	/* INCLUDE_POSIX_TIMERS */

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
#endif /* INCLUDE_POSIX_PTHREAD_SCHEDULERI */

    /* initialize POSIX message queues */

#ifdef INCLUDE_POSIX_MQ
    mqPxLibInit (MQ_HASH_SIZE);
#ifdef INCLUDE_SHOW_ROUTINES
    mqPxShowInit ();
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif  /* INCLUDE_POSIX_MQ */

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
    xbdBlkDevLibInit ();
#endif

#ifdef INCLUDE_FS_MONITOR
    fsMonitorInit ();
#endif

#ifdef INCLUDE_FS_EVENT_UTIL
    fsEventUtilLibInit ();
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
    dosFsCacheLibInit (DOSFS_DEFAULT_CACHE_SIZE);
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
    cdromFsInit();                      /* initialize the cdromFs library */
#endif

#ifdef	INCLUDE_MEMDRV
    memDrv ();				/* initialize memory disk driver */
#endif	/* INCLUDE_MEMDRV */

#ifdef	INCLUDE_PTYDRV
    ptyDrv ();				/* initialize pseudo-terminal driver */
#endif	/* INCLUDE_PTYDRV */

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

#ifdef  INCLUDE_FLASH
#ifdef	BROADCOM_BSP
    if (sysHasDOC()) {
        tffsPPCInit(0);
    }
#endif

    flashFsLibInit();
#endif  /* INCLUDE_FLASH */
    
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
    {                                   /* initialize hard disk driver */
    IMPORT ATA_RESOURCE ataResources[];
    ATA_RESOURCE *pAtaResource;
    for (ix = 0; ix < ATA_MAX_CTRLS; ix++)
        {
        pAtaResource = &ataResources[ix];
        if (pAtaResource->ctrlType == IDE_LOCAL)
            if ((ataDrv (ix, pAtaResource->drives, pAtaResource->intVector,
		   pAtaResource->intLevel, pAtaResource->configType,
                   pAtaResource->semTimeout, pAtaResource->wdgTimeout))
		== ERROR)
		{
#ifdef INCLUDE_STDIO
		printf ("ataDrv returned ERROR from usrRoot.\n");
#endif /* INCLUDE_STDIO */
		}
        }
    }

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
    memRtpLibInit (RTP_HEAP_INIT_SIZE);
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

#endif /* INCLUDE_RTP */

    /* initialize performance monitoring tools */

#ifdef	INCLUDE_SPY
    spyLibInit ();			/* install task cpu utilization tool */
#endif	/* INCLUDE_SPY */

#ifdef	INCLUDE_TIMEX
    timexInit ();			/* install function timing tool */
#endif	/* INCLUDE_TIMEX */

#ifdef  INCLUDE_ENV_VARS
    envLibInit (ENV_VAR_USE_HOOKS);	/* initialize environment variable */
#endif	/* INCLUDE_ENV_VARS */

    /* initialize object module loader and unloader */

#ifdef	INCLUDE_LOADER
    moduleLibInit ();			/* initialize module manager */
    loadLibInit (STORE_ABS_SYMBOLS);            /* Initialize loader */
    loadElfInit ();				/* use elf format */
#endif	/* INCLUDE_LOADER */

#ifdef INCLUDE_UNLOADER
    unldLibInit ();
#endif	/* INCLUDE_UNLOADER */

    

/*
 * Initialize network if network support is included. If network support is not
 * included call usrNetworkCommonInit() which initializes common components
 * like sockets, sysctl if support for these has been included.
 */

#if  defined(INCLUDE_NETWORK) || defined(INCLUDE_NET_MICRO_STACK)
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
#endif  /* INCLUDE_NETWORK || INCLUDE_NET_MICRO_STACK */

# ifdef	BROADCOM_BSP
    sysBindFix();			/* See sysLib.c */
# endif

#ifdef INCLUDE_PASSFS
    usrPassFsInit (PASSFS_CACHE);
#endif	/* INCLUDE_PASSFS */

/* initialize USB components */

#ifdef INCLUDE_USB_INIT
    usbInit (); 		/* USB Host Stack Initialization */
#endif

#ifdef INCLUDE_UHCI_INIT
    usrUsbHcdUhciAttach (); 	/* UHCI Initialization */
#endif

#ifdef INCLUDE_OHCI_INIT
    usrUsbHcdOhciAttach (); 	/* OHCI Initialization */
#endif

#ifdef INCLUDE_EHCI_INIT
    usrUsbHcdEhciAttach ();     /* EHCI Initialization */
#endif

#ifdef INCLUDE_USB_MOUSE_INIT
    usrUsbMseInit (); 		/* Mouse Driver Initialization */
#endif

#ifdef INCLUDE_USB_KEYBOARD_INIT
    usrUsbKbdInit (); 		/* Keyboard Driver Initialization */
#endif

#ifdef INCLUDE_USB_PRINTER_INIT
    usrUsbPrnInit (); 		/* Printer Driver Initialization */
#endif

#ifdef INCLUDE_USB_SPEAKER_INIT
    usrUsbSpkrInit (); 		/* Speaker Driver Initialization */
#endif

#ifdef INCLUDE_USB_AUDIO_DEMO
    usrUsbAudioDemo (); 	/* USB Audio Demo */
#endif

#ifdef INCLUDE_USB_MS_BULKONLY_INIT
    usrUsbBulkDevInit(); 	/* Bulk Driver Initialization */
#endif

#ifdef INCLUDE_USB_MS_CBI_INIT
    usrUsbCbiUfiDevInit (); 	/* CBI Driver Initialization */
#endif

#ifdef INCLUDE_USB_PEGASUS_END_INIT
    usrUsbPegasusEndInit ();    /* Pegasus Driver Initialization */
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

   /* initialize WindMP */

#ifdef INCLUDE_VXFUSION			/* unbundled VxFusion (distributed objects) */
    usrVxFusionInit (BOOT_LINE_ADRS);
#ifdef INCLUDE_SHOW_ROUTINES
    {
    extern void msgQDistShowInit();
    extern void distNameShowInit ();
    extern void distIfShowInit ();
    extern void msgQDistGrpShowInit ();

    msgQDistShowInit();
    distNameShowInit ();
    distIfShowInit ();
    msgQDistGrpShowInit ();
    }
#endif /* INCLUDE_SHOW_ROUTINES */
#endif /* INCLUDE_WINDMP */

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

#ifdef  INCLUDE_STANDALONE_SYM_TBL
# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("before symTblCreate()\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */

    usrStandaloneInit ();

# if defined(INCLUDE_LOGGING) && defined(INCLUDE_LOG_STARTUP)
    logMsg ("sysSymTbl complete.\n", 1,2,3,4,5,6);
    taskDelay (2);	/* allow time for message to be displayed */
# endif	/* INCLUDE_LOGGING && INCLUDE_LOG_STARTUP */
#endif	/* INCLUDE_STANDALONE_SYM_TBL */

#if (defined(INCLUDE_NET_SYM_TBL) || defined(INCLUDE_PASSFS_SYM_TBL))
    usrLoadSyms();
#endif	/* INCLUDE_NET_SYM_TBL || INCLUDE_PASSFS_SYM_TBL */

#ifdef  INCLUDE_STAT_SYM_TBL
    statSymTbl = symTblCreate (STAT_TBL_HASH_SIZE_LOG2, FALSE, memSysPartId);

    if (statSymTbl == NULL)
	{
	if (_func_printErr != NULL)
	    _func_printErr ("usrRoot: error creating the error status table\n");
        }
    else
	{
	/* fill in from builtin table*/
	for (ix = 0; (ULONG)ix < statTblSize; ix ++)
	    symTblAdd (statSymTbl, &(statTbl [ix]));
	}
#endif	/* INCLUDE_STAT_SYM_TBL */

#ifdef	INCLUDE_SYM_TBL_SYNC
    /* initialize wtx client to synchronize host and target symbol tables */

    symSyncLibInit ();
#endif /* INCLUDE_SYM_TBL_SYNC */

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

#ifdef   INCLUDE_RBUFF                  /* install rBuff support */
    rBuffLibInit();
#ifdef  INCLUDE_SHOW_ROUTINES
    rBuffShowInit ();                   /* install rBuff show routine */
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif /* INCLUDE_RBUFF */

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

#ifdef INCLUDE_WVNETD
    wvNetDInit();
#endif /* INCLUDE_WVNETD */

/* If WindView included, must have a timestamp of some sort */
#ifdef INCLUDE_SYS_TIMESTAMP
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

#if 0
    printLogo ();				/* print out the banner page */
#endif

    printf (" CPU: %s.  Processor #%d.\n", sysModel (), sysProcNumGet ());
    printf (" Memory Size: 0x%x.",
#ifdef	HITACHI_SH_KERNEL_ON_SDRAM
	     (UINT)(sysMemTop () - (char *)FREE_RAM_ADRS));
#else	/* HITACHI_SH_KERNEL_ON_SDRAM */
	     (UINT)(sysMemTop () - (char *)LOCAL_MEM_LOCAL_ADRS));
#endif	/* HITACHI_SH_KERNEL_ON_SDRAM */
    printf ("  BSP version " BSP_VERSION BSP_REV ".\n");
    printf (" Created: %s\n", creationDate);
    printf (" ED&R Policy Mode: %s\n",
#ifdef INCLUDE_EDR_SYSDBG_FLAG
	    edrIsDebugMode() ? "lab" : "deployed");
#else
	    "Permanently Deployed");
#endif
#if defined(INCLUDE_WDB) && defined(INCLUDE_WDB_BANNER)
    printf (" WDB Comm Type: %s\n", WDB_COMM_TYPE_STR);
    printf (" WDB: %s.\n",
	    ((wdbRunsExternal () || wdbRunsTasking ()) ?
		 "Ready" : "Agent configuration failed") );
#endif /*INCLUDE_WDB && INCLUDE_WDB_BANNER*/

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

#ifdef	INCLUDE_RTP

#ifdef INCLUDE_DEBUG
    dbgRtpInit ();		/* initialize process debugging */
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

#ifndef BROADCOM_BSP
    usrShell ();
#endif

    /* only include the simple demo if the shell is NOT included */

#else
#if defined(INCLUDE_DEMO)			/* create demo w/o shell */
    taskSpawn ("demo", 20, 0, 2000, (FUNCPTR)usrDemo, 0,0,0,0,0,0,0,0,0,0);
#endif						/* mips cpp no elif */

#endif	/* INCLUDE_SHELL */


#ifdef	INCLUDE_WDB_PROXY
    wrProxyLibInit (WDB_PROXY_PORT, WDB_PROXY_TASK_PRIORITY,
    			WDB_PROXY_TASK_STACK_SIZE, WDB_PROXY_TASK_OPTIONS);
#ifdef	INCLUDE_WDB_PROXY_UDP
    wrProxyUdpSockRegister ();
#endif	/* INCLUDE_WDB_PROXY_UDP */
#ifdef	INCLUDE_WDB_PROXY_TIPC
    wrProxyTipcSockRegister ();
#endif	/* INCLUDE_WDB_PROXY_TIPC */
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
#ifdef INCLUDE_MMU_OPTIMIZE
    usrMmuOptimize();
#endif /* INCLUDE_MMU_OPTIMIZE */
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


#else /* Not VX_VERSION 62 */

#include "vxWorks.h"			/* always first */
#include "config.h"			/* board support configuration header */
#include "sysLib.h"

#ifdef INCLUDE_FLASH
#ifdef	BROADCOM_BSP
IMPORT STATUS tffsPPCInit(int forceFormat);
IMPORT STATUS sysHasDOC();
#endif
#endif

#include "usrConfig.h"			/* general configuration header */
#include "usrDepend.c"			/* include dependency rules */
#include "usrKernel.c"			/* kernel configuration */
#if defined(BROADCOM_BSP) && defined(INCLUDE_NET_INIT)
#include "usrNetwork.c"			/* Attach network */
#endif

#include "usrExtra.c"			/* conditionally included packages */

/* global variables */

int		consoleFd;		/* fd of initial console device */
char		consoleName[20];	/* console device name, eg. "/tyCo/0" */
SYMTAB_ID	statSymTbl;		/* system error status symbol table id*/
SYMTAB_ID	standAloneSymTbl;	/* STANDALONE version symbol table id */
SYMTAB_ID	sysSymTbl;		/* system symbol table id */
BOOT_PARAMS	sysBootParams;		/* parameters from boot line */
int		sysStartType;		/* type of boot (WARM, COLD, etc) */

char *          bufferAddress;          /* windview: address of event buffer */

void		usrAuxClock (void);

#ifdef BRINGUP
extern void sysSerialPrintStringNL(char* msg);
extern void sysLedDsply(char* msg);
#endif

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
    int startType
    )
    {
#ifdef BRINGUP
    if ((SYS_REVID_GET()) == ID_CFM_1)
        bringupPrintRtn = sysSerialPrintStringNL;
    else
        bringupPrintRtn = sysLedDsply;
#else
    bringupPrintRtn = NULL;
#endif

#if	(CPU_FAMILY == SPARC)
    excWindowInit ();				/* SPARC window management */
#endif

    /* configure data and instruction cache if available and leave disabled */

#ifdef  INCLUDE_CACHE_SUPPORT
    cacheLibInit (USER_I_CACHE_MODE, USER_D_CACHE_MODE);
#endif  /* INCLUDE_CACHE_SUPPORT */

#if	CPU_FAMILY!=SIMSPARCSUNOS && CPU_FAMILY!=SIMHPPA && CPU_FAMILY!=SIMSPARCSOLARIS
    /* don't assume bss variables are zero before this call */

    bzero (edata, end - edata);		/* zero out bss variables */
#endif	/* CPU_FAMILY!=SIMSPARCSUNOS && CPU_FAMILY!=SIMHPPA && CPU_FAMILY!=SIMSPARCSOLARIS */

    sysStartType = startType;			/* save type of system start */

    intVecBaseSet ((FUNCPTR *) VEC_BASE_ADRS);	/* set vector base table */

#if (CPU_FAMILY == AM29XXX)
    excSpillFillInit ();			/* am29k stack cache managemt */
#endif

#ifdef  INCLUDE_EXC_HANDLING
    excVecInit ();				/* install exception vectors */
    excShowInit ();				/* install sysExcMsg handlers */
#endif  /* INCLUDE_EXC_HANDLING */

    sysHwInit ();				/* initialize system hardware */
    usrKernelInit ();				/* configure the Wind kernel */

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

    sysHwInit2();
    /* start the kernel specifying usrRoot as the root task */
    kernelInit ((FUNCPTR) usrRoot, ROOT_STACK_SIZE,
#ifdef  INCLUDE_WDB
                (char *) FREE_RAM_ADRS + WDB_POOL_SIZE,
#else
                (char *) FREE_RAM_ADRS,
#endif
		sysMemTop (), ISR_STACK_SIZE, INT_LOCK_LEVEL);
    }

/*
 * sysSetClock - set VxWorks clock from system realtime clock
 *
 *	The system clock is set from the TOD which contains UTC (GMT).
 *	The initial value of the TIMEZONE environment variable is then
 *	set from NVRAM.  The inverse process must be used to set the clock.
 *
 *	(This used to be in sysHwInit2, but mktime does
 *	not like to be used that early and crashes.)
 */

void sysSetClock(void)
{
    struct timespec 	tp;
    struct tm 		tm;
    static char		tzvar[74];
    extern int		sysTimeZoneGood(char *tz);	/* sysLib.c */

    /* Fetch GMT */

    if (sysTodGet (&tm.tm_year,		/* 00-99 */
		  &tm.tm_mon,		/* 01-12 */
		  &tm.tm_mday,		/* 01-31 */
		  &tm.tm_hour,		/* 00-23 */
		  &tm.tm_min,		/* 00-59 */
		  &tm.tm_sec) < 0)	/* 00-59 */
        return;

    /* Use mktime to convert GMT time to seconds since 1970 */

    putenv("TIMEZONE=GMT::0::");

    tm.tm_year -= 1900;		/* tm year is years since 1900 */
    tm.tm_mon -= 1;		/* tm month is 0-11 */

    tp.tv_sec  = mktime(&tm);
    tp.tv_nsec = 0;

    clock_settime(CLOCK_REALTIME, &tp);

    /* Set TIMEZONE variable according to NVRAM value, if valid */

    strcpy(tzvar, "TIMEZONE=");
    sysNvRamGet(tzvar + 9, 64, NV_OFF_TIMEZONE);
    if (sysTimeZoneGood(tzvar + 9))
	putenv(tzvar);
}

/*
 * dateTimeHook for dosFs, using current local time
 */

void dateTimeHook(DOS_DATE_TIME *pDateTime)
{
    time_t		now;
    struct tm		tm;

    time(&now);
    localtime_r(&now, &tm);

    pDateTime->dosdt_year = tm.tm_year + 1900;
    pDateTime->dosdt_month = tm.tm_mon + 1;	/* 01-12 */
    pDateTime->dosdt_day = tm.tm_mday;		/* 01-31 */
    pDateTime->dosdt_hour = tm.tm_hour;		/* 00-23 */
    pDateTime->dosdt_minute = tm.tm_min;	/* 00-59 */
    pDateTime->dosdt_second = tm.tm_sec;	/* 00-59 */
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
    char tyName [20];
    int  ix;

    /* Initialize the memory pool before initializing any other package.
     * The memory associated with the root task will be reclaimed at the
     * completion of its activities.
     */
#ifdef INCLUDE_MEM_MGR_FULL
    memInit (pMemPoolStart, memPoolSize);	/* initialize memory pool */
#else
    memPartLibInit (pMemPoolStart, memPoolSize);/* initialize memory pool */
#endif /* INCLUDE_MEM_MGR_FULL */

#ifdef	INCLUDE_SHOW_ROUTINES 
    memShowInit ();				/* initialize memShow routine */
#endif	/* INCLUDE_SHOW_ROUTINES */

#if	defined(INCLUDE_MMU_BASIC) || defined(INCLUDE_MMU_FULL)
    usrMmuInit ();				/* initialize the mmu */
#endif	/* defined(INCLUDE_MMU_BASIC) || defined(INCLUDE_MMU_FULL) */

    /* set up system timer */
    sysClkConnect ((FUNCPTR) usrClock, 0);	/* connect clock ISR */
    sysClkRateSet (100);			/* set system clock rate */
    sysClkEnable ();				/* start it */
#if !defined(INCLUDE_SPY)
    /* Connect the load meter if not using spyLib */
    sysAuxClkConnect ((FUNCPTR) usrAuxClock, 0);
    sysAuxClkRateSet (67);			/* rel. prime to sysClk */
    sysAuxClkEnable ();
#endif

#if VX_VERSION == 55
    /*
     * The select library needs to be initialized before the tyLib module
     * since the _func_selWakeupListInit FUNCPTR is required (SPR #3314).
     * The installation of the select task delete hooks is performed
     * later in usrRoot() after NFS and RPC have been initialized.
     */

#ifdef  INCLUDE_SELECT
    selectInit (NUM_FILES);
#endif	/* INCLUDE_SELECT */
#else
    /* 
     * select should be initialized after NFS and RPC for proper delete
     * hook order but _func_selWakeupListInit should be set 
     * before tyLib module (SPR #3314).
     */

#ifdef  INCLUDE_SELECT
    _func_selWakeupListInit     = (FUNCPTR) selWakeupListInit;
#endif	/* INCLUDE_SELECT */
#endif /* VX_VERSION == 55 */

    /* initialize I/O system */

#ifdef  INCLUDE_IO_SYSTEM
    iosInit (NUM_DRIVERS, NUM_FILES, "/null");

    consoleFd = NONE;                           /* assume no console device */
#ifdef  INCLUDE_TYCODRV_5_2
#ifdef  INCLUDE_TTY_DEV
    if (NUM_TTY > 0)
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
    if (NUM_TTY > 0)
	{
	ttyDrv();				/* install console driver */

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

    ioGlobalStdSet (STD_IN,  consoleFd);
    ioGlobalStdSet (STD_OUT, consoleFd);
    ioGlobalStdSet (STD_ERR, consoleFd);

#endif  /* INCLUDE_IO_SYSTEM */

    /* initialize symbol table facilities */

#ifdef	INCLUDE_SYM_TBL
    hashLibInit ();			/* initialize hash table package */
    symLibInit ();			/* initialize symbol table package */
#ifdef 	INCLUDE_SHOW_ROUTINES 
    symShowInit ();			/* initialize symbol table show */
#endif	/* INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_SYM_TBL */


    /* initialize exception handling */

#if     defined(INCLUDE_EXC_HANDLING) && defined(INCLUDE_EXC_TASK)
    excInit ();				/* initialize exception handling */
#endif  /* defined(INCLUDE_EXC_HANDLING) && defined(INCLUDE_EXC_TASK) */

#ifdef	INCLUDE_LOGGING
    logInit (consoleFd, MAX_LOG_MSGS);	/* initialize logging */
#endif	/* INCLUDE_LOGGING */

#ifdef	INCLUDE_SIGNALS
    sigInit ();				/* initialize signals */
#endif	/* INCLUDE_SIGNALS */

    /* initialize debugging */

#ifdef	INCLUDE_DEBUG
    dbgInit ();				/* initialize debugging */
#endif	/* INCLUDE_DEBUG */


    /* initialize pipe driver */

#ifdef	INCLUDE_PIPES
    pipeDrv ();				/* install pipe driver */
#endif	/* INCLUDE_PIPES */


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

    /* initialize POSIX message queues */

#ifdef INCLUDE_POSIX_MQ
    mqPxLibInit (MQ_HASH_SIZE);		
#ifdef INCLUDE_SHOW_ROUTINES
    mqPxShowInit ();
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif  /* INCLUDE_POSIX_MQ */

    /* initialize POSIX async I/O support */

#ifdef INCLUDE_POSIX_AIO
    aioPxLibInit (MAX_LIO_CALLS);
#ifdef INCLUDE_POSIX_AIO_SYSDRV
    aioSysInit (MAX_AIO_SYS_TASKS, AIO_TASK_PRIORITY, AIO_TASK_STACK_SIZE);
#endif  /* INCLUDE_POSIX_AIO_SYSDRV */
#endif  /* INCLUDE_POSIX_AIO */

    /* initialize filesystems and disk drivers */

#ifdef	INCLUDE_DOSFS
    hashLibInit ();			/* initialize hash table package */
    dosFsInit (NUM_DOSFS_FILES); 	/* init dosFs filesystem */
    dosFsDateTimeInstall((FUNCPTR) dateTimeHook);
#endif	/* INCLUDE_DOSFS */

#ifdef	INCLUDE_RAWFS
    rawFsInit (NUM_RAWFS_FILES); 	/* init rawFs filesystem */
#endif	/* INCLUDE_RAWFS */

#ifdef	INCLUDE_RT11FS
    rt11FsInit (NUM_RT11FS_FILES); 	/* init rt11Fs filesystem */
#endif	/* INCLUDE_RT11FS */

#ifdef	INCLUDE_RAMDRV
    ramDrv ();				/* initialize ram disk driver */
#endif	/* INCLUDE_RAMDRV */

#ifdef	INCLUDE_FLASH
#ifdef	BROADCOM_BSP
    if (sysHasDOC()) {
        tffsPPCInit(0);
    }
#endif
#endif	/* INCLUDE_FLASH */

#ifdef  INCLUDE_FLASH
    flashFsLibInit();
#endif  /* INCLUDE_FLASH */

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
        printf ("sysScsiInit() Failed, SCSI system not initialized\n");
        }

#endif	/* INCLUDE_SCSI */

#ifdef  INCLUDE_FD
    fdDrv (FD_INT_VEC, FD_INT_LVL);     /* initialize floppy disk driver */
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
    ideDrv (IDE_INT_VEC, IDE_INT_LVL, IDE_CONFIG); /* init IDE disk driver */
#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA
    {                                   /* initialize hard disk driver */
    IMPORT ATA_RESOURCE ataResources[];
    ATA_RESOURCE *pAtaResource;

    for (ix = 0; ix < ATA_MAX_CTRLS; ix++)
        {
        pAtaResource = &ataResources[ix];
        if (pAtaResource->ctrlType == IDE_LOCAL)
            ataDrv (ix, pAtaResource->drives, pAtaResource->intVector,
                    pAtaResource->intLevel, pAtaResource->configType,
                    pAtaResource->semTimeout, pAtaResource->wdgTimeout);
        }
    }
#ifdef  INCLUDE_SHOW_ROUTINES
    ataShowInit ();                     /* install ATA/IDE show routine */
#endif  /* INCLUDE_SHOW_ROUTINES */
#endif  /* INCLUDE_ATA */

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

#ifdef  INCLUDE_FORMATTED_IO
    fioLibInit ();			/* initialize formatted I/O */
#endif  /* INCLUDE_FORMATTED_IO */

    /* initialize floating point facilities */

#ifdef	INCLUDE_FLOATING_POINT
    floatInit ();			/* initialize floating point I/O */
#endif	/* INCLUDE_FLOATING_POINT */

    /* install software floating point emulation (if applicable) */

#ifdef	INCLUDE_SW_FP
    mathSoftInit ();			/* use software emulation for fp math */
#endif	/* INCLUDE_SW_FP */

    /* install hardware floating point support (if applicable) */

#ifdef	INCLUDE_HW_FP
    mathHardInit (); 			/* do fppInit() & install hw fp math */

#ifdef	INCLUDE_SHOW_ROUTINES 
    fppShowInit ();			/* install hardware fp show routine */
#endif	/* INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_HW_FP */

    /* initialize performance monitoring tools */

#ifdef	INCLUDE_SPY
    spyLibInit ();			/* install task cpu utilization tool */
#endif	/* INCLUDE_SPY */

#ifdef	INCLUDE_TIMEX
    timexInit ();			/* install function timing tool */
#endif	/* INCLUDE_TIMEX */

#ifdef  INCLUDE_ENV_VARS
    envLibInit (ENV_VAR_USE_HOOKS);	/* initialize environment variable */
#endif	/* INCLUDE_ENV_VARS */

    /*
     * Set clock after envLib is initialized because it sets the
     * TIMEZONE environment variable according to NVRAM.
     */

    sysSetClock();

    /* initialize object module loader */

#ifdef	INCLUDE_LOADER
    moduleLibInit ();			/* initialize module manager */

#if	defined(INCLUDE_AOUT)
    loadAoutInit ();				/* use a.out format */
#else	/* coff or ecoff */
#if	defined(INCLUDE_ECOFF)
    loadEcoffInit ();				/* use ecoff format */
#else	/* ecoff */
#if	defined(INCLUDE_COFF)
    loadCoffInit ();				/* use coff format */
#else   /* coff */
#if	defined(INCLUDE_ELF)
    loadElfInit ();				/* use elf format */
#else
#if	defined(INCLUDE_SOM_COFF)
    loadSomCoffInit ();
#endif
#endif
#endif
#endif
#endif

#endif	/* INCLUDE_LOADER */

    /* initialize wtx client to synchronize host and target symbol tables */
#ifdef	INCLUDE_SYM_TBL_SYNC
    symSyncLibInit ();
#endif /* INCLUDE_SYM_TBL_SYNC */

    /* initialize network */

#ifdef  INCLUDE_NET_INIT
    usrBootLineInit (sysStartType);	/* crack the bootline */
    usrNetInit (BOOT_LINE_ADRS);	/* initialize network support */
# ifdef	BROADCOM_BSP
    sysBindFix();			/* See sysLib.c */
# endif
#endif	/* INCLUDE_NET_INIT */

#ifdef	INCLUDE_PASSFS
    {
    extern STATUS passFsInit ();
    extern void *passFsDevInit ();
    char passName [256];

    if (passFsInit (1) == OK)
	{
	extern char vxsim_hostname[];
	extern char vxsim_cwd[];

	sprintf (passName, "%s:", vxsim_hostname);
	if (passFsDevInit (passName) == NULL)
	    {
	    printf ("passFsDevInit failed for <%s>\n", passName);
	    }
	else
	    {
	    sprintf (passName, "%s:%s", vxsim_hostname, vxsim_cwd);
	    ioDefPathSet (passName);
	    }
	}
    else
	printf ("passFsInit failed\n");
    }
#endif	/* INCLUDE_PASSFS */

#ifdef	INCLUDE_DOS_DISK
    {
    char unixName [80];
    extern void unixDrv ();
    extern void unixDiskInit ();
    extern char *u_progname;  /* home of executable */
    char *pLastSlash;

    unixDrv ();

    pLastSlash = strrchr (u_progname, '/');
    pLastSlash = (pLastSlash == NULL) ? u_progname : (pLastSlash + 1);
    sprintf (unixName, "/tmp/%s%d.dos", pLastSlash, sysProcNumGet());
    unixDiskInit (unixName, "A:", 0);
    }
#endif	/* INCLUDE_DOS_DISK */

    /* initialize shared memory objects */

#ifdef INCLUDE_SM_OBJ			/* unbundled shared memory objects */
    usrSmObjInit (BOOT_LINE_ADRS);
#endif /* INCLUDE_SM_OBJ */

    /* write protect text segment & vector table only after bpattach () */

#ifdef	INCLUDE_MMU_FULL		/* unbundled mmu product */
#ifdef	INCLUDE_PROTECT_TEXT
    if (vmTextProtect () != OK)
	printf ("\nError protecting text segment. errno = %x\n", errno);
#endif	/* INCLUDE_PROTECT_TEXT */

#ifdef	INCLUDE_PROTECT_VEC_TABLE
    if (intVecTableWriteProtect () != OK)
	printf ("\nError protecting vector table. errno = %x\n", errno);
#endif	/* INCLUDE_PROTECT_VEC_TABLE */
#endif	/* INCLUDE_MMU_FULL */

#if VX_VERSION == 55
    /* install select hook only after NFS/RPC for proper delete hook order */

#ifdef	INCLUDE_SELECT
    selTaskDeleteHookAdd ();
#endif	/* INCLUDE_SELECT */
#else
    /* initialize select only after NFS and RPC for proper delete hook order */

#ifdef	INCLUDE_SELECT
    selectInit ();
#endif	/* INCLUDE_SELECT */
#endif /* VX_VERSION == 55 */

    /* create system and status symbol tables */

#ifdef  INCLUDE_STANDALONE_SYM_TBL
    sysSymTbl = symTblCreate (SYM_TBL_HASH_SIZE_LOG2, TRUE, memSysPartId);

#if 0
    printf ("\nAdding %ld symbols for standalone.\n", standTblSize);
#endif

    for (ix = 0; ix < standTblSize; ix++)	/* fill in from built in table*/
	symTblAdd (sysSymTbl, &(standTbl[ix]));
#endif	/* INCLUDE_STANDALONE_SYM_TBL */

#ifdef  INCLUDE_NET_SYM_TBL
    sysSymTbl = symTblCreate (SYM_TBL_HASH_SIZE_LOG2, TRUE, memSysPartId);

    netLoadSymTbl ();				/* fill in table from host */
#endif	/* INCLUDE_NET_SYM_TBL */

#ifdef  INCLUDE_STAT_SYM_TBL
    statSymTbl = symTblCreate (STAT_TBL_HASH_SIZE_LOG2, FALSE, memSysPartId);

    for (ix = 0; ix < statTblSize; ix ++)	/* fill in from built in table*/
	symTblAdd (statSymTbl, &(statTbl [ix]));
#endif	/* INCLUDE_STAT_SYM_TBL */


    /* initialize C++ support library */

#if	defined (INCLUDE_CPLUS) && defined (INCLUDE_CPLUS_MIN)
#error	Define only one of INCLUDE_CPLUS or INCLUDE_CPLUS_MIN, not both
#endif

#ifdef	INCLUDE_CPLUS			/* all standard C++ runtime support */
    cplusLibInit ();
#endif

#ifdef	INCLUDE_CPLUS_MIN		/* minimal C++ runtime support */
    cplusLibMinInit ();
#endif

    /* initialize windview support */

#ifdef  INCLUDE_INSTRUMENTATION

    /* initialize timer support for windview */

#ifdef  INCLUDE_TIMESTAMP
#ifdef  INCLUDE_USER_TIMESTAMP
    wvTmrRegister ((UINTFUNCPTR) USER_TIMESTAMP,
    		   (UINTFUNCPTR) USER_TIMESTAMPLOCK,
    		   (FUNCPTR)     USER_TIMEENABLE,
    		   (FUNCPTR)     USER_TIMEDISABLE,
    		   (FUNCPTR)     USER_TIMECONNECT,
    		   (UINTFUNCPTR) USER_TIMEPERIOD,
    		   (UINTFUNCPTR) USER_TIMEFREQ);
#else	/* INCLUDE_USER_TIMESTAMP */
    wvTmrRegister (sysTimestamp,
    		   sysTimestampLock,
    		   sysTimestampEnable,
    		   sysTimestampDisable,
    		   sysTimestampConnect,
    		   sysTimestampPeriod,
    		   sysTimestampFreq);
#endif  /* INCLUDE_USER_TIMESTAMP */
#else	/* INCLUDE_TIMESTAMP */
    wvTmrRegister (seqStamp,
    		   seqStampLock,
    		   seqEnable,
    		   seqDisable,
    		   seqConnect,
    		   seqPeriod,
    		   seqFreq);
#endif  /* INCLUDE_TIMESTAMP */


    /* initialize WindView host target connection */

    connRtnSet (evtSockInit, evtSockClose, evtSockError, evtSockDataTransfer);

    /* set the characteristics of the event task - tEvtTask */

    wvEvtTaskInit (WV_EVT_STACK, WV_EVT_PRIORITY);		

    /* initialize windview instrumentation buffers and tasks */

    bufferAddress = wvInstInit (EVTBUFFER_ADDRESS, EVTBUFFER_SIZE, WV_MODE);

    /* initialize WindView command server */

#ifdef INCLUDE_WINDVIEW
    wvServerInit (WV_SERVER_STACK, WV_SERVER_PRIORITY);
#endif /* INCLUDE_WINDVIEW */

#endif  /* INCLUDE_INSTRUMENTATION */

    /* initialize the WDB debug agent */

#ifdef  INCLUDE_WDB
    wdbConfig();

#ifdef	INCLUDE_WDB_BANNER
#ifndef INCLUDE_SHELL
    /* short banner, like the bootPrintLogo banner */
    printf ("\n\n");
    printf ("%17s%s",     "","VxWorks\n\n");
    printf ("Copyright 1984-1996  Wind River Systems, Inc.\n\n");
    printf ("            CPU: %s\n", sysModel ());
    printf ("        VxWorks: " VXWORKS_VERSION "\n");
    printf ("    BSP version: " BSP_VERSION BSP_REV "\n");
    printf ("  Creation date: %s\n", __DATE__);
    printf ("            WDB: %s.\n\n",
	    ((wdbRunsExternal () || wdbRunsTasking ()) ?
		 "Ready" : "Agent configuration failed") );
#endif /*INCLUDE_SHELL*/

#endif /*INCLUDE_WDB_BANNER*/

#endif  /* INCLUDE_WDB */


#if !defined(BROADCOM_BSP)

    /* initialize interactive shell */

#ifdef  INCLUDE_SHELL
#ifdef	INCLUDE_SECURITY			/* include shell security */
    if ((sysFlags & SYSFLG_NO_SECURITY) == 0)
	{
        loginInit ();				/* initialize login table */
        shellLoginInstall (loginPrompt, NULL);	/* install security program */

	/* add additional users here as required */

        loginUserAdd (LOGIN_USER_NAME, LOGIN_PASSWORD);	
	}
#endif	/* INCLUDE_SECURITY */

#if 0
    printLogo ();				/* print out the banner page */
#endif

    printf ("                               ");
    printf ("CPU: %s.  Processor #%d.\n", sysModel (), sysProcNumGet ());
    printf ("                              ");
    printf ("Memory Size: 0x%x.", (UINT)(sysMemTop () - (char *)LOCAL_MEM_LOCAL_ADRS));
    printf ("  BSP version " BSP_VERSION BSP_REV ".");
#if defined(INCLUDE_WDB) && defined(INCLUDE_WDB_BANNER)
    printf ("\n                             ");
    printf ("WDB: %s.",
	    ((wdbRunsExternal () || wdbRunsTasking ()) ?
		 "Ready" : "Agent configuration failed") );
#endif /*INCLUDE_WDB && INCLUDE_WDB_BANNER*/
    printf ("\n\n");

#ifdef	INCLUDE_STARTUP_SCRIPT			/* run a startup script */
    if (sysBootParams.startupScript [0] != EOS)
	usrStartupScript (sysBootParams.startupScript);
#endif	/* INCLUDE_STARTUP_SCRIPT */

    shellInit (SHELL_STACK_SIZE, TRUE);		/* create the shell */


    /* only include the simple demo if the shell is NOT included */

#else
#if defined(INCLUDE_DEMO)			/* create demo w/o shell */
    taskSpawn ("demo", 20, 0, 2000, (FUNCPTR)usrDemo, 0,0,0,0,0,0,0,0,0,0);
#endif						/* mips cpp no elif */

#endif	/* INCLUDE_SHELL */

#endif /* BROADCOM_BSP */

#ifdef INCLUDE_USER_APPL
    /* Startup the user's application */

    USER_APPL_INIT;	/* must be a valid C statement or block */
#endif
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

#if !defined(INCLUDE_SPY)
/*
 * The aux clock is used to update the load factor.
 *
 * Only use this if spyLib is not being used,
 * as spyLib also uses the aux clock.
 *
 * A smoothing function is used:
 *	update = last * FACTOR + current * (1 - FACTOR)
 *		0.0 <= (update, last, FACTOR, current) <= 1.0
 *
 * The decay over 10 seconds for a clock rate of auxHz is thus:
 *	decay = FACTOR^(auxHz * 10)
 */

#define LOAD_FPADJ	16384			/* fixed point for 1.0 */
#define LOAD_FPFACTOR	16269			/* fixed point for 0.993 */

#define LOAD_FPFPIDLE	0			/* round toward 0 */
#define LOAD_FPFPBUSY	(LOAD_FPADJ * (LOAD_FPADJ - LOAD_FPFACTOR) + \
			 LOAD_FPADJ - 1)	/* round toward 1 */

#define LOAD_MINHZ	1		/* LED blink freq at load = 0.0 */
#define LOAD_MAXHZ	10 		/* LED blink freq at load = 1.0 */

UINT32 	sysLoadVal = 0;
int 	sysLoadShow = 0;

void usrShowLoad(void)
{
    int loadPercent = sysLoadVal * 100 / LOAD_FPADJ;

    logMsg("Load: %d.%d%d\n",
	   loadPercent / 100,
	   (loadPercent % 100) / 10,
	   loadPercent % 10,
	   0, 0, 0);
}

void usrAuxClock ()
{
    static int firstTime = 1;
    static int showTime = 0;
    static int ledState = 0;
    static int FPshowLedTime = 0;
    static int FPauxClkPeriod = 0;
    int ledToggleHz, FPledTogglePeriod;

    if (firstTime) {
	FPauxClkPeriod = LOAD_FPADJ / sysAuxClkRateGet();
	firstTime = 0;
    }

    sysLoadVal =
	((LOAD_FPFACTOR * sysLoadVal +
	  (kernelIsIdle ? LOAD_FPFPIDLE : LOAD_FPFPBUSY)) / LOAD_FPADJ);

    if (sysLoadShow && ++showTime >= sysAuxClkRateGet() * sysLoadShow) {
	usrShowLoad();
	showTime = 0;
    }

    /*
     * As the system load varies from 0.0 to 1.0, the LED blink period
     * varies from LOAD_MINHZ to LOAD_MAXHZ.  The factor of 2 comes in
     * because the LED is being toggled (1/2 period).
     */

    ledToggleHz = (2 * LOAD_MINHZ +
		   2 * sysLoadVal * (LOAD_MAXHZ - LOAD_MINHZ) / LOAD_FPADJ);
    FPledTogglePeriod = LOAD_FPADJ / ledToggleHz;
    FPshowLedTime += FPauxClkPeriod;

    if (FPshowLedTime >= FPledTogglePeriod) {
	FPshowLedTime -= FPledTogglePeriod;
	if (ledState) {
	    ledState = 0;
	    SYS_LED_OFF();
	} else {
	    ledState = 1;
	    SYS_LED_ON();
	}
    }
}
#endif

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

#endif	/* VX_VERSION */
