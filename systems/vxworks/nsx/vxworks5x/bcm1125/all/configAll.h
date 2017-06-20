/* configAll.h - default configuration header */

/* Copyright 1984 - 1999 Wind River Systems, Inc. */

/* $Id: configAll.h,v 1.3 2011/07/21 16:14:48 yshtil Exp $
modification history
--------------------
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




#define INCLUDE_CACHE_SUPPORT	/* include cache support package */
#define INCLUDE_CONSTANT_RDY_Q	/* constant insert time ready queue */
#define INCLUDE_CPLUS		/* include C++ support */
#define INCLUDE_CPLUS_IOSTREAMS	/* include basic iostreams classes */
#define INCLUDE_CPLUS_STL	/* include Standard Template Library core */
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

#if	((CPU_FAMILY==PPC) && (CPU!=PPC403) && (CPU!=PPC405) && (CPU!=PPCEC603) && (CPU!=PPC860))
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

#if	(CPU_FAMILY==SH) && defined(INCLUDE_FLOATING_POINT)
#define	INCLUDE_SW_FP
#if	(CPU==SH7700 || CPU==SH7750)
#define	INCLUDE_HW_FP		/* potential hardware fp support */
#endif	/* CPU==SH7700 */
#endif	/* CPU_FAMILY==SH */

#if	(CPU_FAMILY==SIMHPPA)
#define INCLUDE_HW_FP		/* potential hardware fp support */
#endif  /* SIMHPPA */

/* define appropriate object module format for a given architecture */

#if	((CPU_FAMILY==MIPS) || (CPU_FAMILY==PPC) || (CPU_FAMILY==SH) || \
	 (CPU_FAMILY==SIMSPARCSOLARIS))
#define INCLUDE_ELF             /* ELF object modules */
#else
#if	((CPU_FAMILY==I960) || (CPU_FAMILY==AM29XXX) || (CPU_FAMILY==ARM))
#define INCLUDE_COFF            /* COFF object modules */
#else
#if	(CPU_FAMILY==SIMHPPA)
#define	INCLUDE_SOM_COFF
#else	/* default */
#if	(CPU_FAMILY==SIMNT)
#define	INCLUDE_PECOFF
#else	/* default */
#define INCLUDE_AOUT            /* a.out object modules */
#endif
#endif
#endif
#endif


/******************************************************************************/
/*                                                                            */
/*                          EXCLUDED FACILITIES                               */
/*                                                                            */
/******************************************************************************/

#if 0
#define INCLUDE_CODETEST        /* CodeTEST target utilities library */
#define INCLUDE_CPLUS_VXW	/* include VxWorks wrapper classes */
#define INCLUDE_CPLUS_TOOLS	/* include Tools class library */
#define INCLUDE_CPLUS_STRING      /* include string class */
#define INCLUDE_CPLUS_STRING_IO   /* include i/o for string class */
#define INCLUDE_CPLUS_COMPLEX     /* include complex number class */
#define INCLUDE_CPLUS_COMPLEX_IO  /* include i/o for complex number class */
#define INCLUDE_CPLUS_IOSTREAMS_FULL  /* include all of iostreams */
#define INCLUDE_CONFIGURATION_5_2 /* pre-tornado tools */
#define INCLUDE_DEBUG           /* pre-tornado debugging */
#define INCLUDE_DEMO		/* include simple demo instead of shell */
#define INCLUDE_DOSFS           /* dosFs file system */
#define INCLUDE_HTML		/* include HTML support */
#define INCLUDE_JAVA		/* Java virtual machine */
#define INCLUDE_SOUND       /* include support for sampled audio data */
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
#define INCLUDE_RBUFF           /* ring of buffers library */
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
#define INCLUDE_UGL		/* include Universal Graphic Library support */
#define INCLUDE_UNLOADER        /* object module unloading */
#define INCLUDE_WILLOWS_RT      /* include Willows RT library */
#define INCLUDE_WDB_TSFS	/* target-server file system */
#define INCLUDE_VXFUSION        /* WindMP (unbundled) */
#define INCLUDE_WINDVIEW	/* WindView target facilities */
#define INCLUDE_WVUPLOAD_ALL	/* include all Windview upload-path types */
#define INCLUDE_WVUPLOAD_FILE	/* include file Windview upload-path */
#define INCLUDE_WVUPLOAD_SOCK	/* include socket Windview upload-path */
#define INCLUDE_WVUPLOAD_TSFSSOCK /* include tsfs sock Windview upload-path */


#define INCLUDE_ANSI_5_0	/* include only version 5.0 ANSI support */
#define INCLUDE_DELETE_5_0	/* define delete() function as in VxWorks 5.0 */

#define INCLUDE_TSFS_BOOT	/* Boot using Target Server File System */
#define INCLUDE_TSFS_BOOT_VIO_CONSOLE	/* use tgtsvr Console for TSFS_BOOT */

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

#if CPU==SIMNT
#define FREE_RAM_ADRS		simMemBlock
#else
#define FREE_RAM_ADRS		(end)	/* start right after bss of VxWorks */
#endif


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

#if CPU_FAMILY==SIMNT || CPU_FAMILY==SIMSPARCSUNOS  || CPU_FAMILY==SIMHPPA || CPU_FAMILY==SIMSPARCSOLARIS
#define INT_LOCK_LEVEL          0x1     /* interrupt disable mask */
#define ROOT_STACK_SIZE         20000   /* size of root's stack, in bytes */
#define SHELL_STACK_SIZE        50000   /* size of shell's stack, in bytes */
#define WDB_STACK_SIZE	 	0x2000	/* size of WDB agents stack, in bytes */
#define ISR_STACK_SIZE          50000   /* size of ISR stack, in bytes */
#define VEC_BASE_ADRS           0       /* dummy */
#endif  /* CPU_FAMILY ==SIMNT || CPU_FAMILY==SIMSPARCSUNOS  || CPU_FAMILY==SIMHPPA || CPU_FAMILY==SIMSPARCSOLARIS */

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

#if	CPU_FAMILY==SH
#define	INT_LOCK_LEVEL		15
#define	ROOT_STACK_SIZE		10000
#define	SHELL_STACK_SIZE	10000	/* size of shell's stack, in bytes */
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
#define WDB_COMM_PIPE           7       /* vxSim pipe device    - bimodal   */

/* communication path configuration */

#ifndef WDB_COMM_TYPE
#define WDB_COMM_TYPE WDB_COMM_NETWORK	/* default path is the network */
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
#elif	(CPU==SH7750 || CPU==SH7729 || CPU==SH7700)
#define SM_ANCHOR_OFFSET        0x1600	/* not used for SDRAM config. */
#define BOOT_LINE_OFFSET        0x1700
#define EXC_MSG_OFFSET          0x1800
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

#if     ((CPU_FAMILY==I960) || (CPU_FAMILY==ARM))
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

#define HOST_NAME_DEFAULT	"bootHost"	/* host name */
#define TARGET_NAME_DEFAULT	"vxTarget"	/* target name (tn) */
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

#if	((CPU_FAMILY == PPC) || (CPU_FAMILY == ARM) || (CPU_FAMILY == SH))
#define VM_PAGE_SIZE		4096
#else 	/* ((CPU_FAMILY == PPC) || (CPU_FAMILY == ARM) || (CPU_FAMILY == SH)) */
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
#define INCLUDE_TFTP_CLIENT	/* tftp client */

#define INCLUDE_POSIX_ALL       /* include all available POSIX functions */
#endif

/* INCLUDED NETWORK DRIVERS */
#if 1 
#define INCLUDE_NFS             /* nfs package */
#define INCLUDE_FTP_SERVER      /* ftp server */
#define INCLUDE_NET_SHOW        /* network info and status facilities */
#endif


/******************************************************************************/
/*                                                                            */
/*                         EXCLUDED NETWORK FACILITIES                        */
/*                                                                            */
/******************************************************************************/
#if FALSE
#define INCLUDE_ARP             /* user interface to ARP table */
#define INCLUDE_BSD		/* netif driver support */
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
#define INCLUDE_PROXY_CLIENT	/* proxy arp client (Slave Board) */
#define INCLUDE_PROXY_DEFAULT_ADDR /* Use ethernet addr to generate bp addrs */
#define INCLUDE_PROXY_SERVER	/* proxy arp server (Master Board) */
#define INCLUDE_RIP             /* Routing Information Protocol RIP */
#define INCLUDE_RLOGIN          /* remote login */
#define INCLUDE_ROUTE_SOCK	/* include routing socket interface */
#define INCLUDE_RPC		/* rpc package */
#define INCLUDE_SLIP		/* include serial line interface */
#define INCLUDE_SM_NET		/* include backplane net interface */
#define INCLUDE_SM_SEQ_ADDR     /* shared memory network auto address setup */
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
#define DHCPC_MAX_MSGSIZE       590   /* Allows minimum message on Ethernet */
#endif

/* DHCP server parameters */

#ifdef INCLUDE_DHCPS
#define DHCPS_LEASE_HOOK 	NULL   /* Name of required storage routine */
#define DHCPS_ADDRESS_HOOK 	NULL   /* Name of optional storage routine */
#define DHCPS_DEFAULT_LEASE 	3600   /* default lease length (secs). */
#define DHCPS_MAX_LEASE 	3600   /* default value of max lease (secs). */
#define DHCPS_MAX_MSGSIZE 	590    /* Allows minimum message on Ethernet */
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

#ifdef INCLUDE_DNS_RESOLVER
#define RESOLVER_DOMAIN_SERVER  "10.16.64.11"      /* DNS server IP address */
#define RESOLVER_DOMAIN         "sj.broadcom.com"  /* Resolver domain */
#endif /* INCLUDE_DNS_RESOLVER */

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

/* add for vxWorks 5.5 */
#ifdef INCLUDE_BSD_SOCKET
#define SOMAXCONN_CFG 5
#endif

/* add for vxWorks 5.5 */
#if defined (INCLUDE_NFS_SERVER) || defined (INCLUDE_NFS)
#define NFS_MAXPATH             255             /* max. file path length */
#endif

#endif	/* INCconfigAllh */
