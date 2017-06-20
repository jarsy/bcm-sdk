/* Copyright 2002 Broadcom Corporation */

/* $Id: config.h,v 1.5 2011/07/21 16:14:55 yshtil Exp $
modification history
--------------------
01b,17sep02,jmb	 add TFFS 
01a,01aug02,jmb	 Written 
*/

/*
This file contains the configuration parameters for the
BCM 47xx
*/

#ifndef __INCconfigh
#define __INCconfigh


/* BSP version/revision identification, before configAll.h */
#define BSP_VER_1_1     1
#define BSP_VER_1_2	    1
#define BSP_VERSION     "1.0"
#define BSP_REV         "/0"    /* 0 for first revision */


/******************************************************************************/
/*                                                                            */
/*                        INCLUDE FILES                                       */
/*                                                                            */
/******************************************************************************/
    
#include "configAll.h"
#include "mbz.h"

#undef  INCLUDE_HW_FP
#define INCLUDE_SW_FP           /* include software floating point library */

#undef INCLUDE_SHELL

/******************************************************************************/
/*                                                                            */
/*              ENHANCED NETWORK DRIVER SUPPORT                               */
/*                                                                            */
/******************************************************************************/
#ifdef INCLUDE_NETWORK
#define INCLUDE_NET_SHOW
#define INCLUDE_END             

#define STANDALONE_NET

#define IP_MAX_UNITS 5
#undef INCLUDE_ET0_END
#define ET0_IP_ADDRESS "10.22.2.118"
#undef INCLUDE_ET1_END
#define ET1_IP_ADDRESS "192.168.2.1"
#endif /* INCLUDE_NETWORK */


/* No Virtual ET or WL interface on BCM4704 CPCI */
#undef INCLUDE_ETV0_END
#undef ETV0_IP_ADDRESS
#undef INCLUDE_WL_END
#undef WL_IP_ADDRESS

/******************************************************************************/
/*                                                                            */
/*                        BOOT PARAMETERS                                     */
/*                                                                            */
/******************************************************************************/
#define DEFAULT_BOOT_LINE \
"netdrv(0,0)10.18.39.6:latest/bcm.raptor h=10.18.39.6 e=10.18.53.157:ffffe000 g=10.18.32.1 u=tornado pw=tornado+ f=0x0000"

#define INCLUDE_DOSFS           /* dosFs file system */
#define INCLUDE_FLASH_BOOT
#define INCLUDE_FLASH

/******************************************************************************/
/*                                                                            */
/*                          CACHING SUPPORT                                   */
/*                                                                            */
/******************************************************************************/

#undef USER_I_CACHE_MODE
#undef USER_D_CACHE_MODE
 
#if 1
#define INCLUDE_CACHE_SUPPORT
#define USER_I_CACHE_MODE       CACHE_COPYBACK
#define USER_D_CACHE_MODE       CACHE_COPYBACK
#define USER_I_CACHE_ENABLE                         /* undef to leave disabled*/
#define USER_D_CACHE_ENABLE                         /* undef to leave disabled*/
#undef  USER_B_CACHE_ENABLE                         /* undef to leave disabled*/
#else
#undef INCLUDE_CACHE_SUPPORT
#define USER_I_CACHE_MODE       CACHE_DISABLED
#define USER_D_CACHE_MODE       CACHE_DISABLED
#undef USER_I_CACHE_ENABLE                         /* undef to leave disabled*/
#undef USER_D_CACHE_ENABLE                         /* undef to leave disabled*/
#undef  USER_B_CACHE_ENABLE                         /* undef to leave disabled*/
#endif

/* WDB agent config */
#undef SERIAL_SYS_MODE_DEBUG
#ifdef SERIAL_SYS_MODE_DEBUG
#undef  WDB_COMM_TYPE
#define WDB_COMM_TYPE    WDB_COMM_SERIAL
#undef  WDB_TTY_BAUD
#define WDB_TTY_BAUD     115200
#undef  WDB_TTY_CHANNEL
#define WDB_TTY_CHANNEL  0
#undef  WDB_TTY_DEV_NAME
#define WDB_TTY_DEV_NAME "/tyCo/0"
#endif /* SERIAL_SYS_MODE_DEBUG */

/******************************************************************************/
/*                                                                            */
/*                          SERIAL SUPPORT                                    */
/*                                                                            */
/******************************************************************************/
#undef  NUM_TTY
#define NUM_TTY                         2

#ifdef CONSOLE_BAUD_RATE_OVERRIDE
#undef CONSOLE_BAUD_RATE
#define CONSOLE_BAUD_RATE       CONSOLE_BAUD_RATE_OVERRIDE
#else
#define CONSOLE_BAUD_RATE       9600          /* console baud rate */
#endif
 

/******************************************************************************/
/*                                                                            */
/*                          TIMER SUPPORT                                     */
/*                                                                            */
/******************************************************************************/
#define SYS_CLK_RATE_MIN        1           /* minimum system clock rate */
#define SYS_CLK_RATE_MAX        3600        /* maximum system clock rate */
#define AUX_CLK_RATE_MIN        1           /* minimum auxiliary clock rate */
#define AUX_CLK_RATE_MAX        10000       /* maximum auxiliary clock rate */

/******************************************************************************/
/*                                                                            */
/*                       MEMORY CONSTANTS                                     */
/*                                                                            */
/******************************************************************************/
#define LOCAL_MEM_LOCAL_ADRS        0x80000000 
#define LOCAL_MEM_SIZE32              0x02000000  /* 64MB  memory */
#define LOCAL_MEM_SIZE128             0x08000000  /* 128MB  memory */
#define LOCAL_MEM_SIZE256             0x10000000  /* 256MB  memory */
#define LOCAL_MEM_SIZE                  LOCAL_MEM_SIZE128

#undef CACHE_MEM_POOL_RESERVED
#ifdef  CACHE_MEM_POOL_RESERVED
/* Configuration constants for bcm4704/bcm5836 "high memory" support.
 * CACHEMEM_POOL_SIZE
 *     This much memory is withdrawn from the low 128MB region and
 *     reserved for DMA mappings (through the cacheLib interface).
 *     Depending on application requirements, this value may need to
 *     be adjusted up or down.  The minimum value will be system-dependent
 *     but is likely in the 1MB - 8MB range; the maximum value is
 *     slightly less than 128MB (or the amount of physical memory
 *     available, if less than 128MB).
 *
 *     If too low a value is configured, DMA allocations through
 *     cacheDmaMalloc will fail.
 *
 *     If too high a value is configured, the system will fail to allocate
 *     memory and will disable highmem.  This will result in only the
 *     low 128M being used (the system will fall back to the non-highmem
 *     behavior).
 *
 *     If 128M or less physical memory is installed, the system will
 *     detect that and will fall back to the non-highmem behavior.
 */
#define CACHEMEM_POOL_SIZE           (64<<20)
#endif  /* CACHE_MEM_POOL_RESERVED */

/* User reserved memory, See sysMemTop */
#define USER_RESERVED_MEM	0

/*
 * The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_LOW_ADRS and
 * RAM_HIGH_ADRS are defined in config.h, and MakeSkel.
 * All definitions for these constants must be identical.
 */
#define ROM_TEXT_ADRS           0xbfc00000      /* base addresss of ROM */
#define ROM_BASE_ADRS           ROM_TEXT_ADRS  
#define ROM_SIZE                0x00080000      /* 512KB ROM space */
#define RAM_LOW_ADRS            0x80008000      /* RAM address for kernel/compressed code storage start */
#define RAM_HIGH_ADRS           0x82c00000      /* RAM address for ROM boot/RAM code start address */

/* Board-specific options */
#if defined(BCM56218) || defined(BCM56214) || defined(BCM56218P48)

#undef SDRAM_MODE

#ifdef QUICK_TURN
#define DDR16MX16X1
#else
#define DDR64MX16X1
#endif

#elif defined(BCM56018) || defined(BCM56014)

#define SDRAM_MODE

#define SDRM32X16X1

#endif



/******************************************************************************/
/*                                                                            */
/*                        APPLICATION ENTRY POINT                             */
/*                                                                            */
/******************************************************************************/ 

/* Start the Orion SOC diagnostics shell */

#define INCLUDE_USER_APPL
#define USER_APPL_INIT { extern void vxSpawn(void); vxSpawn(); }
             
#ifndef _ASMLANGUAGE
extern unsigned long readCount(void);
extern unsigned long readInstr(void);
extern void APPLICATION(void);   
#endif  /* _ASMLANGUAGE */
              
#endif  /* __INCconfigh */

#if defined(PRJ_BUILD)
#include "prjParams.h"
#endif

/* end of config.h */
