/* sysLib.h - system dependent routines header */

/* Copyright 1984-2001 Wind River Systems, Inc. */

/* $Id: sysLib.h,v 1.5 2011/07/21 16:14:08 yshtil Exp $
modification history
--------------------
03w,21may04,dtr  Fix SPR 96824.
03v,04feb04,elp  added VxSim sysNetIfConfig().
03u,19aug03,dbs  add ED&R system debug mode flag
03t,07may03,pes  PAL conditional compilation cleanup. Phase 2.
03s,24apr02,pai  Removed sysHwInit0() prototype.  A generic interface of this
                 name is not yet defined.
03r,15apr02,pai  added sysHwInit0() prototype (SPR 74951).
03q,10oct01,dat  SPR 70855, adding sysNanoDelay as an optional BSP API
03p,09jun98,dat  rework of optional BSP routine declarations
03o,14nov97,db   added functions forward declarations for PPC family.
03n,05nov96,hdn  moved NETIF structure from bootConfig.c.
03m,18sep96,dat  added sysBspRev, sysPhysMemTop, sysScsiConfig,
		 sysSerialHwInit, sysSerialHwInit2, sysSerialReset
03l,06aug96,dat  using #include timerDev.h to declare timer functions
03k,13jun96,hdn  added sysInLong() sysInLongString() sysOutLong()
		 sysOutLongString().
03j,15jun95,ms	 updated for new serial driver
03i,14jun95,hdn  added x86 specific prototypes.
03h,22may95,ms   added sysSerialDevGet() prototype.
03g,22sep92,rrr  added support for c++
03f,15sep92,jcf  cleaned up.
03e,31jul92,ccc  added tyCoDrv() and tyCoDevCreate() prototypes.
03d,27jul92,ccc  added prototypes to clean up os warnings.
03c,09rdc92,rdc  moved PHYS_MEM_DESC struct to vmLib.c.
03b,08rdc92,rdc  added PHYS_MEM_DESC struct.
03a,04jul92,jcf  cleaned up.
02b,26jun92,wmd  modified prototype for sysFaultTableInit().
02a,16jun92,ccc  removed function declarations for sysLib break-up.
01z,26may92,rrr  the tree shuffle
01y,23apr92,wmd  added sysFaultVecSet() prototype declaration for i960.
01x,21apr92,ccc  added SCSI declarations.
01w,16apr92,elh  added SYSFLG_PROXY.
01v,04apr92,elh  added SYSFLG_TFTP.
01u,18dec91,wmd  added mre ANSI prototypes for i960.
01t,04oct91,rrr  passed through the ansification filter
		  -fixed #else and #endif
		  -changed VOID to void
		  -changed ASMLANGUAGE to _ASMLANGUAGE
		  -changed copyright notice
01s,23oct90,shl  added a set of generic system dependent function prototypes.
01r,05oct90,dnw  added some ANSI prototypes.
01q,05oct90,shl  added copyright notice.
                 made #endif ANSI style.
01p,10aug90,dnw  added SYSFLG_BOOTP
01o,28jul90,dnw  removed BOOT_FIELD_LEN to bootLib.h
		 added declaration of sysBootParams
		 added include of bootLib.h
01m,20jun90,gae  changed start types to be bit fields and renamed types.
		 added import of sysModel.
01l,24apr90,shl  added SYSFLG_NO_SECURITY.
01k,23may89,dnw  added SYSFLG_NO_STARTUP_SCRIPT.
01j,02may89,dnw  added SYSFLG_VENDOR_{0,1,2,3}.
01i,15oct88,dnw  added SYSFLG_NO_AUTOBOOT, SYSFLG_QUICK_AUTOBOOT,
		   and BOOT_WARM_QUICK_AUTOBOOT.
01h,24mar88,ecs  added declaration of sysExcMsg.
01g,13nov87,jcf  changed names of boot types.
01f,29oct87,dnw  added SYSFLG_DEBUG.
01e,14oct87,dnw  added SYSFLG_NO_SYS_CONTROLLER.
01d,14jul87,dnw  added more system global varaibles.
		 deleted sysLocalToBusAdrs().
		 added system restart types.
01c,14feb87,dnw  added sysBus, sysCpu, sysLocalToBusAdrs
01b,18dec86,llk  added BOOT_FIELD_LEN.
01a,04aug84,dnw  written
*/

#ifndef __INCsysLibh
#define __INCsysLibh

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	_ASMLANGUAGE
#include "bootLib.h"
#include "ttyLib.h"
#include "sioLib.h"
#include "drv/timer/timerDev.h"

typedef struct  /* NETIF - network interface table */
    {
    char *ifName;  		     	/* interface name */
    FUNCPTR attachRtn;  		/* attach routine */
    char *arg1;				/* address */
    int arg2;				/* vector */
    int arg3;				/* level */
    int arg4;
    int arg5;
    int arg6;
    int arg7;
    int arg8;
    } NETIF;

#endif	/* _ASMLANGUAGE */

/* system restart types */

#define	BOOT_NORMAL		0x00	/* normal reboot with countdown */
#define BOOT_NO_AUTOBOOT	0x01	/* no autoboot if set */
#define BOOT_CLEAR		0x02	/* clear memory if set */
#define BOOT_QUICK_AUTOBOOT	0x04	/* fast autoboot if set */

/* for backward compatibility */

#define BOOT_WARM_AUTOBOOT		BOOT_NORMAL
#define BOOT_WARM_NO_AUTOBOOT		BOOT_NO_AUTOBOOT
#define BOOT_WARM_QUICK_AUTOBOOT	BOOT_QUICK_AUTOBOOT
#define BOOT_COLD			BOOT_CLEAR


/* system configuration flags in sysFlags */

/* Some targets have system controllers that can be enabled in software.
 * By default, the system controller is enabled for processor 0.
 * When set this flag inhibits enabling the system controller even
 * for processor 0.
 */

#define SYSFLG_NO_SYS_CONTROLLER 0x01

/* System debug option:
 * load kernel symbol table with all symbols (not just globals)
 */

#define SYSFLG_DEBUG             0x02

#define SYSFLG_NO_AUTOBOOT	 0x04	/* Don't start autoboot sequence */
#define SYSFLG_QUICK_AUTOBOOT	 0x08	/* Immediate autoboot (no countdown) */
#define SYSFLG_NO_STARTUP_SCRIPT 0x10	/* Don't read startup script */
#define SYSFLG_NO_SECURITY	 0x20   /* Don't ask passwd on network login */
#define SYSFLG_AUTOCONFIG	 0x40   /* Use bootp or DHCP to get boot */
                                        /* parameters */
#define SYSFLG_TFTP		 0x80   /* Use tftp to get boot image */
#define SYSFLG_PROXY		 0x100	/* Use proxy arp */
#define SYSFLG_WDB		 0x200	/* Use WDB agent */
#define SYSFLG_SYS_MODE_DEBUG	 0x400	/* System is in 'debug' mode */
    
#define SYSFLG_VENDOR_0		 0x1000	/* vendor defined flag 0 */
#define SYSFLG_VENDOR_1		 0x2000	/* vendor defined flag 1 */
#define SYSFLG_VENDOR_2		 0x4000	/* vendor defined flag 2 */
#define SYSFLG_VENDOR_3		 0x8000	/* vendor defined flag 3 */


/* system parameters */

#ifndef	_ASMLANGUAGE

extern int 	sysBus;		/* system bus type (VME_BUS, MULTI_BUS, etc) */
extern int 	sysCpu;		/* system cpu type (MC680x0, SPARC, etc.) */
extern int 	sysProcNum;	/* processor number of this cpu */
extern char *	sysBootLine;	/* address of boot line */
extern char *	sysExcMsg;	/* address of exception message area */
extern int	sysFlags;	/* configuration flags */
extern BOOT_PARAMS sysBootParams; /* parameters from boot line */

/* obsolete - but remain for backward compatiblity */

extern char	sysBootHost[];	/* name of host from which system was booted */
extern char	sysBootFile[];	/* name of file from which system was booted */

/* function declarations */

#if defined(__STDC__) || defined(__cplusplus)

extern char *	sysModel (void);
extern char *	sysBspRev (void);
extern void 	sysHwInit (void);
extern void 	sysHwInit2 (void);
extern char *	sysMemTop (void);
extern char *	sysPhysMemTop (void);
extern STATUS 	sysToMonitor (int startType);
extern int 	sysProcNumGet (void);
extern void 	sysProcNumSet (int procNum);
extern BOOL 	sysBusTas (char *adrs);
extern STATUS 	sysNvRamGet (char *string, int strLen, int offset);
extern STATUS 	sysNvRamSet (char *string, int strLen, int offset);
extern STATUS 	sysScsiInit (void);
extern STATUS 	sysScsiConfig (void);
extern void	sysSerialHwInit (void);
extern void	sysSerialHwInit2 (void);
extern SIO_CHAN * sysSerialChanGet (int channel);
extern void	sysSerialReset (void);
extern STATUS	sysLocalToBusAdrs (int adrsSpace, char *localAdrs,
				   char **pBusAdrs);
extern STATUS	sysBusToLocalAdrs (int adrsSpace, char *busAdrs,
				   char **pLocalAdrs);
extern STATUS	sysIntDisable (int intLevel);
extern STATUS	sysIntEnable (int intLevel);
extern int	sysBusIntAck (int intLevel);
extern STATUS	sysBusIntGen (int level, int vector);
extern STATUS	sysMailboxConnect (FUNCPTR routine, int arg);
extern STATUS	sysMailboxEnable (char *mailboxAdrs);
extern int	tyCoDrv (void);
extern int	tyCoDevCreate (char *name, int channel, int rdBufSize,
			       int wrtBufSize);

/* For all architectures, except X86, the arguments are unsigned.  */
#if defined (_WRS_SIGNED_IO_ARGS)
#define _WRS_IOLONG	int
#define _WRS_IOSHORT	short
#define _WRS_IOCHAR	char
#else /* _WRS_SIGNED_IOARGS */
#define _WRS_IOLONG	ULONG
#define _WRS_IOSHORT	UINT16
#define _WRS_IOCHAR	UCHAR
#endif	/* _WRS_SIGNED_IO_ARGS */


extern UCHAR	sysInByte		(_WRS_IOLONG port);
extern USHORT	sysInWord		(_WRS_IOLONG port);
extern ULONG	sysInLong		(_WRS_IOLONG port);
extern void	sysInWordString		(_WRS_IOLONG port, _WRS_IOSHORT *pData, int count);
extern void	sysInLongString		(_WRS_IOLONG port, _WRS_IOLONG *pData, int count);
extern void	sysOutByte		(_WRS_IOLONG port, _WRS_IOCHAR data);
extern void	sysOutWord		(_WRS_IOLONG port, _WRS_IOSHORT data);
extern void	sysOutLong		(_WRS_IOLONG port, _WRS_IOLONG data);
extern void	sysOutWordString	(_WRS_IOLONG port, _WRS_IOSHORT *pData, int count);
extern void	sysOutLongString	(_WRS_IOLONG port, _WRS_IOLONG *pData, int count);


/* optional support routines used by several architectures */

extern void	sysNanoDelay		(UINT32 nanoseconds);
extern void	sysReboot		(void);
extern void	sysDelay		(void);
extern void	sysWait			(void);
extern STATUS	sysIntDisablePIC	(int intLevel);
extern STATUS	sysIntEnablePIC		(int intLevel);
extern UINT8   pldRegRead(int);
extern void pldRegWrite(int,UINT8);
extern int sysIsCFM(void);
extern int sysHasDOC(void);
extern int sysSlotIdGet(void);

/* i960 special routines */

extern void 	sysFaultTableInit	(void (*func)(void));
extern UINT32 	sysFaultVecSet		(INSTR *vector, UINT32 faultNo, 
					 UINT32 type);
extern void 	sysExcInfoPrint		(UINT32 type);

/* X86 special routines */

extern void	sysLoadGdt		(char *sysGdtr);

/* VxSim special routines */
#ifdef _WRS_ARCH_IS_SIMULATOR
extern void	sysNetIfConfig		(void);
#endif /* _WRS_ARCH_IS_SIMULATOR */

#else	/* __STDC__ */

extern char *	sysModel ();
extern void 	sysHwInit ();
extern void 	sysHwInit2 ();
extern char *	sysMemTop ();
extern STATUS 	sysToMonitor ();
extern int 	sysProcNumGet ();
extern void 	sysProcNumSet ();
extern BOOL 	sysBusTas ();
extern STATUS 	sysNvRamGet ();
extern STATUS 	sysNvRamSet ();
extern STATUS 	sysScsiInit ();
extern SIO_CHAN *sysSerialChanGet ();
extern STATUS   sysLocalToBusAdrs ();
extern STATUS   sysBusToLocalAdrs ();
extern STATUS   sysIntDisable ();
extern STATUS   sysIntEnable ();
extern int      sysBusIntAck ();
extern STATUS   sysBusIntGen ();
extern STATUS   sysMailboxConnect ();
extern STATUS	sysMailboxEnable ();
extern int	tyCoDrv ();
extern int	tyCoDevCreate ();
extern void sysLedDsply(char* msg);

#if	CPU_FAMILY == I960
extern void 	sysFaultTableInit	();
extern UINT32 	sysFaultVecSet		();
extern void 	sysExcInfoPrint		();
#endif	/* CPU_FAMILY == I960 */

#if defined (_WRS_SYS_IO)
extern UCHAR	sysInByte		();
extern USHORT	sysInWord		();
extern ULONG	sysInLong		();
extern void	sysInWordString		();
extern void	sysInLongString		();
extern void	sysOutByte		();
extern void	sysOutWord		();
extern void	sysOutLong		();
extern void	sysOutWordString	();
extern void	sysOutLongString	();
extern void	sysReboot		();
extern void	sysDelay		();
extern void	sysWait			();
extern void	sysLoadGdt		();
extern STATUS	sysIntDisablePIC	();
extern STATUS	sysIntEnablePIC		();
#endif /* _WRS_SYS_IO */

/* VxSim special routines */
#ifdef _WRS_ARCH_IS_SIMULATOR
extern void	sysNetIfConfig		();
#endif /* _WRS_ARCH_IS_SIMULATOR */

#endif	/* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCsysLibh */
