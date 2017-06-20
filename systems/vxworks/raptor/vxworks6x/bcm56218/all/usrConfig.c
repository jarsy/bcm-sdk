/* usrConfig.c - user-defined system configuration library */

/* Copyright 1984-2001 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: usrConfig.c,v 1.4 2011/07/21 16:15:00 yshtil Exp $
modification history
--------------------
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

#include "vxWorks.h"			/* always first */
#include "config.h"			/* board support configuration header */

#ifdef BROADCOM_BSP
IMPORT UINT8 sysBoardRev();
extern void sysSetClock(void);
extern void sysBindFix(void);
#endif /* BROADCOM_BSP */

#include <usrConfig.h>			/* general configuration header */
#include <usrDepend.c>			/* include dependency rules */
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

#ifdef INCLUDE_POSIX_THREAD_CPUTIME
#include <private/pxCpuTimeLibP.h>
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
    int startType
    )
    {
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
    adrSpaceShowInit ();			/* initialize adrSpaceShow */
#endif /* defined(INCLUDE_ADR_SPACE_SHOW) */

#if defined (INCLUDE_MAPPED_FILES_SHOW)
    mmanShowInit ();				/* initialize mmanShow lib */
#endif
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

    /* Now inject a 'boot' ED&R event - this takes place outside
     * the INCLUDE_EDR guards, as the stub is always included.
     */
    edrStubInit ();

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
    iosInit (NUM_DRIVERS, NUM_FILES, "/null", IOS_POSIX_PSE52_MODE);

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
    sigInit (POSIX_SIGNAL_MODE);        /* initialize signals */
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
#ifdef BROADCOM_BSP
#else
    tffsDrv ();				/* it should be after pcmciaInit() */
#endif
#endif	/* INCLUDE_TFFS */

#ifdef	INCLUDE_FLASH
#ifndef QUICK_TURN
    flashFsLibInit();
#endif
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
    spyLibInit ();			/* install task cpu utilization tool */
#endif	/* INCLUDE_SPY */

#ifdef	INCLUDE_TIMEX
    timexInit ();			/* install function timing tool */
#endif	/* INCLUDE_TIMEX */

#ifdef  INCLUDE_ENV_VARS
    envLibInit (ENV_VAR_USE_HOOKS);	/* initialize environment variable */
#endif	/* INCLUDE_ENV_VARS */

#ifdef BROADCOM_BSP
    /*
     * Set clock after envLib is initialized because it sets the
     * TIMEZONE environment variable according to NVRAM.
     */

    sysSetClock();
#endif
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

    printLogo ();				/* print out the banner page */

    printf (" CPU: %s.  Processor #%d.\n", sysModel (), sysProcNumGet ());
    printf (" Memory Size: 0x%x.",
#ifdef	HITACHI_SH_KERNEL_ON_SDRAM
	     (UINT)(sysMemTop () - (char *)FREE_RAM_ADRS));
#else	/* HITACHI_SH_KERNEL_ON_SDRAM */
	     (UINT)(sysMemTop () - (char *)LOCAL_MEM_LOCAL_ADRS));
#endif	/* HITACHI_SH_KERNEL_ON_SDRAM */
    printf ("  BSP version " BSP_VERSION BSP_REV ".\n");
#ifdef BROADCOM_BSP
    printf ("       Board ID: %d\n", sysBoardRev());
#endif
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


#ifdef BROADCOM_BSP
/*
 * sysSetClock - set VxWorks clock from system realtime clock
 *
 *      The system clock is set from the TOD which contains UTC (GMT).
 *      The initial value of the TIMEZONE environment variable is then
 *      set from NVRAM.  The inverse process must be used to set the clock.
 *
 *      (This used to be in sysHwInit2, but mktime does
 *      not like to be used that early and crashes.)
 */


void sysSetClock(void)
{
    struct timespec     tp;
    struct tm           tm;
    static char         tzvar[74];
    extern int          sysTimeZoneGood(char *tz);      /* sysLib.c */

    /* Fetch GMT */

    if (sysTodGet(&tm.tm_year,                /* 00-99 */
                  &tm.tm_mon,                 /* 01-12 */
                  &tm.tm_mday,                /* 01-31 */
                  &tm.tm_hour,                /* 00-23 */
                  &tm.tm_min,                 /* 00-59 */
                  &tm.tm_sec) < 0)            /* 00-59 */
        return;

    /* Use mktime to convert GMT time to seconds since 1970 */

    putenv("TIMEZONE=GMT::0::");

    tm.tm_year -= 1900;         /* tm year is years since 1900 */
    tm.tm_mon -= 1;             /* tm month is 0-11 */

    tp.tv_sec  = mktime(&tm);
    tp.tv_nsec = 0;

    clock_settime(CLOCK_REALTIME, &tp);

    /* Set TIMEZONE variable according to NVRAM value, if valid */

    strcpy(tzvar, "TIMEZONE=");
    sysNvRamGet(tzvar + 9, 64, NV_OFF_TIMEZONE);
    if (sysTimeZoneGood(tzvar + 9))
        putenv(tzvar);
}

#endif /* BROADCOM_BSP */
