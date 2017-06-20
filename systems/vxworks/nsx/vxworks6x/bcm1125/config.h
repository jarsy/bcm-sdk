/* bcm1250a/config.h - BCM1250-swarm configuration header */

/* $Id: config.h,v 1.7 2011/07/21 16:14:49 yshtil Exp $
 * Copyright (c) 2002-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01w,23aug05,jmt  Componentize BSP for scalability
01v,01aug05,h_k  added VXLAYER.
01u,31oct04,pes  General cleanup and removal of unneeded conditional code,
                 made possible by changing MIPS default of SW_MMU_ENABLE.
01t,28oct04,pes  The default setting of SW_MMU_ENABLE has been changed
                 for MIPS from FALSE to TRUE. This requires a change
		 in the logic to set it to FALSE if INCLUDE_MAPPED_KERNEL
		 is defined.
01s,07oct04,agf  addt'l vxWorks 6.0 clean-up
01r,06oct04,pes  Change INCLUDE_MMU_BASIC to INCLUDE_MAPPED_KERNEL. Add
                 suport for MMU-less RTPs.
01q,30aug04,j_b  remove INCLUDE_NETWORK dependencies (SPR #99747)
01q,30jul04,md   Use default PM_RESERVED_MEM 
01p,03aug04,agf  change MMU include from FULL to BASIC
01o,04may04,agf  port to Base6
01n,15nov02,agf  increment BSP_REV
01m,03oct02,agf  add ifdefs for selecting between cpu0 & cpu1 constants
01l,18jul02,agf  correct ROM_SIZE to be 2M
01k,17jul02,agf  update REV number for T2.2 FCS
01j,20jun02,pgh  Make the definition of NUM_TTY not dependent on bcm1250.h.
01i,10may02,tlc  Add C++ header protection.
01h,30apr02,agf  remove WDB_COMM overrides since WDB END has been fixed (SPR
                 73331)
01g,28mar02,tlc  Update revision information.
01f,13mar02,agf  increase SYS_CLK_RATE_MAX to 5000, SPR 74129
                 remove undefs of INCLUDE_SM_OBJ/NET, SPR 74321
                 move SM anchor address into cached space, SPR 74346
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

#ifdef __cplusplus
extern "C" {
#endif

/* BSP version/revision identification, before configAll.h */
#define BSP_VER_1_1     1
#define BSP_VER_1_2     1
#define BSP_VERSION     "2.0"   /* vxWorks 6.0 compatible */
#define BSP_REV         "/5"    /* 0 for first revision */

/* includes */
#include "configAll.h"
#include "bcm1250.h"
#if 0
#include "bcm1250CpuNum.h"
#include <arch/mips/mmuMipsLib.h>
#endif

#define BCM1250_SHARED_SOURCE
#define BCM1250_CPU_0

#undef  _SIMULATOR_             /* define when running on the bcm1250 simulator */
#undef _SENTOSA_
#undef _RHONE_


/* Network driver configuration */

#define INCLUDE_NETWORK
#define	INCLUDE_END
#define INCLUDE_NET_SHOW
#define STANDALONE_NET 

    
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


/* MMU configuration
 *
 * MIPS_TLB_PAGE_SIZES is a bitmask that defines the supported MMU Page
 * Sizes for this BSP.  The following Page Sizes are supported:
 *   MMU_PAGE_MASK_8K,   MMU_PAGE_MASK_32K, MMU_PAGE_MASK_128K,
 *   MMU_PAGE_MASK_512K, MMU_PAGE_MASK_2M
 * VM_PAGE_SIZE must be set to the minimum page size define in
 * MIPS_TLB_PAGE_SIZES.
 */
#if defined(INCLUDE_MAPPED_KERNEL)
#define MIPS_TLB_PAGE_SIZES	        MMU_PAGE_MASK_32M	
#undef  VM_PAGE_SIZE
#define VM_PAGE_SIZE			TLB_32M_PAGE_SIZE
#undef  INCLUDE_EXC_SHOW
#define MMU_USE_EXC_TLB_PAGE		TRUE

#define MMU_EXC_TLB_VIRT_BASE		0xffffe000
#ifdef  BCM1250_CPU_0
#define MMU_EXC_TLB_PAGE_BASE		0x00008000
#else   /* BCM1250_CPU_1 */
#define MMU_EXC_TLB_PAGE_BASE		0x00018000
#endif

#define R4K_MMU_CACHEABLE		3
#define R4K_MMU_UNCACHEABLE		2
#define R4K_MMU_CACHE_COPYBACK		3
#define R4K_MMU_CACHE_WRITETHROUGH	1
#define R4K_MMU_CACHE_COHERENCY		5
#endif

#ifndef MIPSSB1_N_TLB_ENTRIES
#define MIPSSB1_N_TLB_ENTRIES		64
#endif
#define MIPS_N_TLB_ENTRIES		MIPSSB1_N_TLB_ENTRIES


/* PCI bus */
#define INCLUDE_PCI             /* only define PCI on primary core */


/* the following three drivers are not applicable for a busless board */
#undef  INCLUDE_EGL             /* remove EGL driver */
#undef  INCLUDE_ENP             /* remove ENP driver */
#undef  INCLUDE_SM_NET          /* remove SM driver */

/* Optional timestamp support for WindView - undefined by default */
#undef  INCLUDE_TIMESTAMP


/* Algorithmics NVRAM */
#undef ALGCOMPAT


/* miscellaneous definitions */
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

#ifdef INCLUDE_DOSFS
#define INCLUDE_DOSFS_FAT       /* dosFs FAT12/16/32 FAT table handler */
#define INCLUDE_DOSFS_DIR_VFAT  /* Microsoft VFAT dirent handler */
#define INCLUDE_DOSFS_DIR_FIXED /* 8.3 & VxLongNames directory handler */
#define INCLUDE_DOSFS_FMT       /* dosFs2 file system formatting module */
#define INCLUDE_DOSFS_CHKDSK    /* file system integrity checking */
#define INCLUDE_CBIO            /* CBIO API module */
#define INCLUDE_DISK_PART       /* disk partition handling code, fdisk... */
#endif
/******************************************************************************/
/*                                                                            */
/*                          SERIAL SUPPORT                                    */
/*                                                                            */
/******************************************************************************/
#undef  NUM_TTY
#define NUM_TTY     3       /* 2 DUARTs and 1 JTAG */

#undef  CONSOLE_TTY
#define CONSOLE_TTY 0

/* assign UART A, UART B and JTAG */
#define INCLUDE_BCM1250_UART_CHAN_A
#undef  INCLUDE_BCM1250_UART_CHAN_B
#undef  INCLUDE_BCM1250_JTAG_CHAN_A
#define BCM1250_UART_CHAN_A_IDX                 0       /* tty 0 */

#ifndef BCM1250_UART_DEFAULT_BAUD
#ifdef BCM1250
#define BCM1250_UART_DEFAULT_BAUD  115200
#else
#define BCM1250_UART_DEFAULT_BAUD  9600
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
/* allocate 240 bytes in NVRAM for the bootline */
/* #define INCLUDE_NVRAM */

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

/*
 * mapped kernels require that INCLUDE_MMU_BASIC is defined and
 * SW_MMU_ENABLE is set to FALSE.
 */
#if defined (INCLUDE_MAPPED_KERNEL)
#if !defined (INCLUDE_MMU_BASIC)
#define INCLUDE_MMU_BASIC
#endif /* INCLUDE_MMU_BASIC */
#undef SW_MMU_ENABLE
#define SW_MMU_ENABLE FALSE
#endif /* defined INCLUDE_MAPPED_KERNEL */

#ifdef __cplusplus
}
#endif
#endif	/* __INCconfigh */

#if defined(PRJ_BUILD)
#include "prjParams.h"
#endif /* defined PRJ_BUILD */

#if 1
#define LED_ADDR   (char*)0xBD0A0000
#define BPRINT(str)                     \
    {                                   \
            *(LED_ADDR+24) = str[0];    \
            *(LED_ADDR+16) = str[1];    \
            *(LED_ADDR+8) =  str[2];    \
            *(LED_ADDR+0) =  str[3];    \
    }


#else
#define BPRINT(str)
#endif


/*
 * EXC_PAGE_PHYS_ADRS is the PHYS_ADR for the mmu exception data
 * This is normally defined in configAll.h, but must be redefined
 * here so multicore kernels will not overlap. Also, it must be
 * done after the prj facility attempts to set it.
 */
#ifdef  BCM1250_CPU_0
#undef  EXC_PAGE_PHYS_ADRS
#define EXC_PAGE_PHYS_ADRS           0x00002000
#else
#undef  EXC_PAGE_PHYS_ADRS
#define EXC_PAGE_PHYS_ADRS           0x00012000
#endif /* BCM1250_CPU_0 */

