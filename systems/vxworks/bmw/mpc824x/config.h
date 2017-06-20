/* $Id: config.h,v 1.6 2011/07/21 16:14:08 yshtil Exp $
 * This file contains the configuration parameters for the BMW BSP.
 */

/*
modification history
--------------------
01b,30apr02,jmb turn on the caches
01a,18apr02,jmb derived from mousse config.h
*/



#ifndef INCconfigh
#define INCconfigh

/* BSP version/revision identification, before configAll.h */

#define BSP_VER_1_1     1
#define BSP_VERSION     "2.0"
#define BSP_REV         "/0"    /* 0 for first revision */
#define BSP_BOOTROM_REV "2.0"

#include "configAll.h"          /* Set the VxWorks default configuration */
#include "bmw.h"

/* NOTE: The subnetmask specified here must change with different net configs. 
 *  This subnet mask fffff800 indicates that the top two octets and part
 *  of the third octet are local.  so xxx.x.24-27 are local and the last
 *  octet can be 0-255.  The user will need to set the e= IP address and
 *  subnetmask for their particular configuration.
*/

/* M-Systems TFFS support */
#undef INCLUDE_TFFS            /* Don't use incomplete builtin code */


/*  Net  */
#define DEFAULT_BOOT_LINE \
"bc(0,0)borg:boot/bcm.bmw h=192.168.2.1 e=192.168.2.21:ffffff00 u=tornado pw=tornado+"

/*   UNIX ftp requires username and password
*  #define DEFAULT_BOOT_LINE \
	"dc(0,0)grinch:/home/maurie/vxWorks h=223.70.249.2 e=163.2.27.79:fffff800 tn=vxWorks u=maurie pw=simple g=163.2.31.254 f=0x0"
*/

/*   UNIX tftp requires only uesername and no password
 *    notice, the file must be in the local directry /tftpboot
 *    but the file name does not include this directory name, 
 *    tftp is anchored in /tftpboot, f=0x80 indicates tftp
 * #define DEFAULT_BOOT_LINE \
	"dc(0,0)grinch:vxWorks h=223.70.249.2 e=163.2.27.79:fffff800 tn=vxWorks u=maurie g=163.2.31.254 f=0x80"
*/



/* memory configuration

 Memory is specified in terms of the physical SDRAM architecture, such
 as the number of rows and columns of the memory devices, as well as
 the number of physical banks (chip selects). Due to the design of
 both the MPC8245 and the BMW subsystem the parameter limits are as
 follows:

      SDRAM memory rows: 12 - 13        (SPD parameter #3)
   SDRAM memory columns:  8 - 11        (SPD parameter #4)
   SDRAM physical banks:  1 - 2         (SPD parameter #5)

 The number of logical banks is fixed at 4.

 Configurations tested:

  Size   Rows   Columns  Banks
 ------ ------- ------- -------
  64MB       12       9       1
 128MB       12       9       2
 256MB       12      10       2
 
 */

#define SDRAM_ROWS      12
#define SDRAM_COLUMNS    9
#define SDRAM_BANKS      1

/*
  Calculate the size of a SDRAM physical bank in Megabytes. This size
  is num_logical_banks * memory_module_bit_width * 2^(rows+columns) bits.
  num_logical_banks is always 4 (2^2)
  memory_module_bit_width is always 64 (2^6)
  bits_per_byte is 8 (2^3)

  2^(rows+columns)*(2^2)*(2^6)/(2^20 * 2^3) = 2^(rows+columns-15)
  
*/

#define SDRAM_BANK_SIZE  (1<<(SDRAM_ROWS + SDRAM_COLUMNS - 15))

#define LOCAL_MEM_LOCAL_ADRS    0x0
#define LOCAL_MEM_SIZE          ((SDRAM_BANK_SIZE << 20) * SDRAM_BANKS)

#define USER_RESERVED_MEM       0

#define  LOCAL_MEM_AUTOSIZE                      /* run-time memory sizing */


/*
 * The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_HIGH_ADRS, and RAM_LOW_ADRS
 * are defined in config.h and Makefile.
 * All definitions for these constants must be identical.
 */
#define ROM_BASE_ADRS           0xfff00000      /* base address of ROM */
#define ROM_TEXT_ADRS           (ROM_BASE_ADRS+0x0100) /* with PC & SP */
#define ROM_WARM_ADRS           (ROM_TEXT_ADRS+0x0004) /* warm reboot entry */
#define ROM_SIZE                0x00080000      /* 512KB ROM space */
#define RAM_LOW_ADRS            0x00010000   /* RAM address for vxWorks */
#define RAM_HIGH_ADRS           0x02e00000   /* RAM address for bootrom */

/* Serial port configuration */

#define INCLUDE_SERIAL
#undef  NUM_TTY
#define NUM_TTY        N_SIO_CHANNELS  /* defined in bmw.h */
#define CONSOLE_BAUD_RATE	9600	/* console baud rate */

/*
 * Cache configuration
 *
 * Note that when MMU is enabled, cache modes are controlled by
 * the MMU table entries in sysPhysMemDesc[], not the cache mode
 * macros defined here.
 */

/* define these to turn on icaches */
#define  INCLUDE_CACHE_SUPPORT   /* cacheLib support */
#define  USER_I_CACHE_ENABLE	
#undef USER_I_CACHE_MODE
#define  USER_I_CACHE_MODE  	CACHE_COPYBACK

#define USER_D_CACHE_ENABLE
#undef  USER_D_CACHE_MODE
#define USER_D_CACHE_MODE       CACHE_COPYBACK 


/* MMU configuration */

#define MMU_DEBUG         /*  print out the sysPhysMemDesc tables */
#define  INCLUDE_MMU_BASIC
#undef  INCLUDE_MMU_FULL        /* no MMU support */

#undef  INCLUDE_SHELL
#undef	INCLUDE_ANSI_ASSERT
  
#undef  INCLUDE_EX
#undef  INCLUDE_ENP
#undef  INCLUDE_SM_NET


/* VME configuration */

/* PCI configuration */

#define  INCLUDE_PCI

/* Generic PCI configuration parameters */
#define  PCI_MAX_BUS             2      /* MPC107, optionally DC21150 */
#define	INCLUDE_END			/* END network driver please */

#if defined(INCLUDE_END) 
#undef	INCLUDE_END_DEC_21X4X
#define	IP_MAX_UNITS	32		/* 32 Per SOC device */
#endif

#define  INCLUDE_NETWORK		/* Network required */
#define  INCLUDE_NET_INIT		/* Init network */
#define  INCLUDE_NET_SHOW

/* Load Ramix Quad Enet driver */
/* #define INCLUDE_RAMIX_PMC */

/* --------------------------------------------------------------------- */
/* Broadcom definitions                                                  */
/* --------------------------------------------------------------------- */

/*
 * VX_VERSION allows this BSP to be compiled under Tornado 1/VxWorks 5.3.1
 * (531) or Tornado 2/VxWorks 5.4 (54).  A default is provide in case the
 * Makefile does not provide the definition.
 */

#ifndef VX_VERSION
#define VX_VERSION 531
#endif

#define ENET_DEFAULT                   0x04e11000

#ifdef BROADCOM_BSP

/* NOTE: Keep these in sync with sal/pci.h */

#define PCI_SOC_MBAR0	0x81100000
#define PCI_SOC_MEM_WINSZ	0x10000
#define PCI_SOC_MBAR_PRI_BUS(unit)	(PCI_SOC_MBAR0 - (unit) * PCI_SOC_MEM_WINSZ)
#define PCI_SOC_MBAR_SEC_BUS(unit)	(PCI_SOC_MBAR0 + (unit) * PCI_SOC_MEM_WINSZ)

/* Start the Orion SOC diagnostics shell */

#define INCLUDE_USER_APPL
#define USER_APPL_INIT { extern void vxSpawn(void); vxSpawn(); }

#endif /* BROADCOM_BSP */

/*  
 *  NVRAM configuration
 *  NVRAM is implemented via the SGS Thomson M48T59Y
 *  64Kbit (8Kbx8) Timekeeper SRAM.
 *  This 8KB NVRAM also has a TOD. See m48t59y.{h,c} for more information.
 */

#define NV_RAM_SIZE		8176
#define NV_RAM_ADRS     	TOD_NVRAM_BASE
#define NV_RAM_INTRVL   	1
#define NV_RAM_WR_ENBL		SYS_TOD_UNPROTECT()
#define NV_RAM_WR_DSBL		SYS_TOD_PROTECT()

#define NV_OFF_BOOT0		0x0000	/* Boot string 0 (256b) */
#define NV_OFF_BOOT1		0x0100	/* Boot string 1 (256b) */
#define NV_OFF_BOOT2		0x0200	/* Boot string 2 (256b)*/
#define NV_OFF_MACADDR		0x0400	/* 21143 MAC address (6b) */
#define NV_OFF_ACTIVEBOOT	0x0406	/* Active boot string, 0 to 2 (1b) */
#define NV_OFF_UNUSED1		0x0407	/* Unused (1b) */
#define NV_OFF_BINDFIX		0x0408	/* See sysLib.c:sysBindFix() (1b) */
#define NV_OFF_UNUSED2		0x0409	/* Unused (7b) */
#define NV_OFF_TIMEZONE		0x0410	/* TIMEZONE env var (64b) */
#define NV_OFF__next_free	0x0450

/*  needed to start ethernet for vxWorks.st standalone version */

#define  STANDALONE_NET

#if 0
#define INCLUDE_INSTRUMENTATION
#define INCLUDE_WINDVIEW
#endif

#ifndef BOOTCONFIG
#define INCLUDE_SHOW_ROUTINES
#define INCLUDE_TIMESTAMP
#define INCLUDE_TFTP_CLIENT /* For bootflags=0x80 and Windows Host */

#define INCLUDE_PING
#endif

#endif  /* INCconfigh */
