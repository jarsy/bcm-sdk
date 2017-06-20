/* bcm1250/config.h - BCM1250 configuration header */

/* Copyright 2002 Wind River Systems, Inc. */

/* $Id: config.h,v 1.6 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01e,12dec01,ros  remove INCLUDE_SM_OBJ and VXMP for shipment
01d,11dec01,agf  fix DEFAULT_BOOT_LINE
01c,10dec01,agf  minor tweaking
01b,06dec01,agf  remove extraneous component INCLUDE's
01a,15nov01,agf  written
*/

/*
This file contains the configuration parameters for the
Broadcom BCM1250-swarm evaluation board.
*/

#ifndef	__INCconfigh
#define	__INCconfigh

/* BSP version/revision identification, before configAll.h */
#define BSP_VER_1_1     1
#define BSP_VER_1_2     1
#define BSP_VERSION     "1.2"
#define BSP_REV         "/0"    /* 0 for first revision */

/******************************************************************************/
/*                        INCLUDE FILES                                       */
/******************************************************************************/
#include "configAll.h"
#include "bcm1250.h"

#define BCM1250_SHARED_SOURCE
#define BCM1250_CPU_0

#undef  _SIMULATOR_             /* define when running on the bcm1250 simulator */
#undef _SENTOSA_
#undef _RHONE_

/******************************************************************************/
/* Network driver configuration                                               */
/******************************************************************************/
#define INCLUDE_NETWORK
#define	INCLUDE_END
#define INCLUDE_NET_SHOW
#define STANDALONE_NET 

#if 0
#define INCLUDE_STANDALONE_NETWORK
#define INCLUDE_PING
#define INCLUDE_ARP
#define INCLUDE_TELNET

#define INCLUDE_SHOW_ROUTINES
#define INCLUDE_TIMESTAMP
#define INCLUDE_TFTP_CLIENT
#endif

/* optional PCI-based DEC 21x40 END driver */
/* note: this is normally INCLUDE_DC, but that causes problems here */
#undef  INCLUDE_DEC


/* WDB Agent configuration */

/* #undef INCLUDE_WDB*/

#ifdef INCLUDE_WDB
#undef	WDB_COMM_TYPE			/* disable default path */
#define	WDB_COMM_TYPE	WDB_COMM_END	/* set path to END */
#define WDB_MODE	WDB_MODE_DUAL	/* WDB_MODE_[DUAL|TASK|EXTERN] */
#endif

/* No VxWorks Shell */
#undef INCLUDE_SHELL

/* SCSI configuration */

#undef  INCLUDE_SCSI            /* SCSI support */
#undef  INCLUDE_SCSI2           /* SCSI2 support */


/* CPU-specific facilities */

#undef  USER_D_CACHE_MODE
#define USER_D_CACHE_MODE	CACHE_WRITETHROUGH
#undef  USER_I_CACHE_MODE
#define USER_I_CACHE_MODE	CACHE_WRITETHROUGH

#define INCLUDE_CACHE_SUPPORT 
#define USER_I_CACHE_ENABLE
#define USER_D_CACHE_ENABLE

/* PCI bus */
#define INCLUDE_PCI             /* only define PCI on primary core */

/* #define PCI_AUTO_DEBUG */

/* the following three drivers are not applicable for a busless board */
#undef  INCLUDE_EGL             /* remove EGL driver */
#undef  INCLUDE_ENP             /* remove ENP driver */
#undef  INCLUDE_SM_NET          /* remove SM driver */

/* Optional timestamp support for WindView - undefined by default */
#undef  INCLUDE_TIMESTAMP

/* override standard exception handlers */
#define INCLUDE_SYS_HW_INIT_0
#define SYS_HW_INIT_0 sysSetExcVecInitHook

#ifndef _ASMLANGUAGE
extern void sysSetExcVecInitHook (void);
extern void sysExcVecInit (void);
#endif

/* Algorithmics NVRAM */
#undef ALGCOMPAT


/******************************************************************************/
/*                        BOOT PARAMETERS                                     */
/******************************************************************************/

#if defined(BCM1250)

#define DEFAULT_BOOT_LINE \
    "sbe(1,0)10.16.64.72:/home/labguy/boot/jzhao/sdk5/bcm.nsx e=10.16.64.19:fffff000 h=10.16.64.72 g=10.16.64.1 u=tornado pw=tornado+ f=0 tn=SiByte1250"

#else

#define DEFAULT_BOOT_LINE \
    "sbe(0,0)10.18.39.6:/home/labguy/boot/jzhao/sdk5/bcm.nsx e=10.18.37.10:ffffe000 h=10.18.39.6 g=10.18.32.1 u=tornado pw=tornado+ f=0 tn=SiByte1125 o=flash"
#endif

/* allocate 240 bytes in NVRAM for the bootline */
#undef BOOT_LINE_SIZE
#define	BOOT_LINE_SIZE	240	/* use 240 bytes for bootline */
#define NV_RAM_SIZE BOOT_LINE_SIZE

/* Add in DOSFS for Broadcom */
#define INCLUDE_DOSFS           /* dosFs file system */
#define INCLUDE_FLASH
#define INCLUDE_FLASH_BOOT

/******************************************************************************/
/*                                                                            */
/*                          SERIAL SUPPORT                                    */
/*                                                                            */
/******************************************************************************/
#undef  NUM_TTY
#define NUM_TTY			N_SIO_CHANNELS

#undef  CONSOLE_TTY
#define CONSOLE_TTY 0

/* assign UART A, exclude UART B and JTAG */
#define INCLUDE_BCM1250_UART_CHAN_A
#undef  INCLUDE_BCM1250_UART_CHAN_B
#undef  INCLUDE_BCM1250_JTAG_CHAN_A
#define BCM1250_UART_CHAN_A_IDX                 0       /* tty 0 */

#ifndef BCM1250_UART_DEFAULT_BAUD
#ifdef BCM1250
#define BCM1250_UART_DEFAULT_BAUD 115200
#else
#define BCM1250_UART_DEFAULT_BAUD 9600
#endif
#endif
#undef CONSOLE_BAUD_RATE

#define CONSOLE_BAUD_RATE       BCM1250_UART_DEFAULT_BAUD

#define SYS_CLK_RATE_MIN  1	/* minimum system clock rate */
#define SYS_CLK_RATE_MAX  3600	/* maximum system clock rate */
#define AUX_CLK_RATE_MIN  1	/* minimum auxiliary clock rate */
#define AUX_CLK_RATE_MAX  10000	/* maximum auxiliary clock rate */

/**********************************************************/
/*             MEMORY CONSTANTS                           */
/**********************************************************/
#define LOCAL_MEM_LOCAL_ADRS		0x80000000
#define LOCAL_MEM_SIZE			0x08000000   /* 128M memory */
#define USER_RESERVED_MEM       	0x00000000   /* number of reserved bytes */

#define LOCAL_MEM_LOCAL_ADRS_RUNT	0x80000000
#define LOCAL_MEM_SIZE_RUNT		0x01000000   /* lower 16MB memory for cpu0 run-time kernel */

/*
 * The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_LOW_ADRS and
 * RAM_HIGH_ADRS are defined in config.h, and MakeSkel.
 * All definitions for these constants must be identical.
 */

#define ROM_TEXT_ADRS		0xbfc00000      /* base address of ROM */
#define ROM_BASE_ADRS           ROM_TEXT_ADRS
#define ROM_SIZE                0x00100000      /* 1MB ROM space */
#define RAM_LOW_ADRS		0x80100000      /* RAM address for kernel */
#define RAM_HIGH_ADRS		0x84000000      /* RAM address for ROM boot */


/******************************************************************************/
/* APPLICATION ENTRY POINT - Start the Orion SOC diagnostics shell            */
/******************************************************************************/

#define INCLUDE_USER_APPL
#define USER_APPL_INIT { extern void vxSpawn(void); vxSpawn(); }


/* Shared memory configuration */
#undef  INCLUDE_SM_OBJ
#undef  INCLUDE_VXMP_TESTS
/*
#define  INCLUDE_SM_OBJ
#define  INCLUDE_VXMP_TESTS
*/
#define SM_TAS_TYPE 		SM_TAS_HARD
#define SM_INT_TYPE		SM_INT_USER_1
#define SM_INT_ARG1		0x1   /* bit to use to set/clear the mbox int */
#define SM_INT_ARG2		0     /* not used */
#define SM_INT_ARG3		0     /* not used by this BSP */
#define SM_OBJ_MEM_ADRS 	NONE
#define SM_OBJ_MEM_SIZE 	0x80000  /* sh. mem Objects pool size 512K */

#undef	SM_ANCHOR_ADRS
#define SM_ANCHOR_ADRS  ((char *) 0x80000600)   /* on-board anchor adrs */
#define SM_MEM_ADRS     NONE            /* NONE = allocate sh. mem from pool */
#define SM_MEM_SIZE     0x80000                 /* 512K */
#define SM_OFF_BOARD    FALSE

#define IP_MAX_UNITS 4

#endif	/* __INCconfigh */
#if defined(PRJ_BUILD)
#include "prjParams.h"
#endif

