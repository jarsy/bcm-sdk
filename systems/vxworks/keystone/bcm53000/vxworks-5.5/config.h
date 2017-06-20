/* Copyright 2001-2007 Wind River Systems, Inc. 
*
* The right to copy, distribute, modify or otherwise make use
* of this software may be licensed only pursuant to the terms
* of an applicable Wind River license agreement.
*/

/* $Id: config.h,v 1.5 2011/08/08 16:02:55 jimhuang Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */


#ifndef __INCconfigh
#define __INCconfigh

#ifdef __cplusplus
extern "C" {
#endif

/* BSP version/revision identification, before configAll.h */

#define BSP_VER_1_1 1
#define BSP_VER_1_2 1
#define BSP_VERSION "1.2"
#define BSP_REV     "0"   /* increment with each revision */

#undef  INCLUDE_PCI_TEST /* define used for testing PCIe during BSP validation */

/* define System Controller chip used on the board */

/* includes */
#include "configSdkPre.h"   /* Configs for Broadcom SDK */
#include <configAll.h>
#include "configSdkPost.h"  /* Configs for Broadcom SDK */
#include "keystone.h"
#include "bcm53000.h"

#undef  INCLUDE_HW_FP 
#define INCLUDE_SW_FP           /* include software floating point library */

/* #undef INCLUDE_SHELL */
/* #undef INCLUDE_WDB */        /* WDB debug agent */


/* Network driver configuration */
/******************************************************************************/
/*                                                                            */
/*              ENHANCED NETWORK DRIVER SUPPORT                               */
/*                                                                            */
/******************************************************************************/
#define INCLUDE_NET_SHOW
#define INCLUDE_END             

#define ETHERNET_MAC_HANDLER

#if defined(ETHERNET_MAC_HANDLER)
#define MAX_MAC_ADRS     2
#define MAC_ADRS_LEN     6

#define ENET_DEFAULT     0x18100000 /* BRCM fixed MAC addr; see WR_ENETx */

#ifndef MAX_MAC_DEVS
#define MAX_MAC_DEVS     2
#endif

#define BRCM_ENET0       0x00 /* BRCM specific portion of MAC (MSB->LSB) */
#define BRCM_ENET1       0x10
#define BRCM_ENET2       0x18

#define CUST_ENET3_0     0xA0    /* Customer portion of MAC address */
#define CUST_ENET3_1     0xA1
#define CUST_ENET4       0xAA
#define CUST_ENET5       0xA0
#endif


#define STANDALONE_NET

#define IP_MAX_UNITS 5
#define INCLUDE_ET0_END
#define ET0_IP_ADDRESS "192.168.2.1"
#define INCLUDE_ET1_END
#define ET1_IP_ADDRESS "192.168.2.2"

/* No Virtual ET or WL interface on BCM4704 CPCI */
#undef INCLUDE_ETV0_END
#undef ETV0_IP_ADDRESS
#undef INCLUDE_WL_END
#undef WL_IP_ADDRESS

#undef INCLUDE_EXC_SHOW


#define INCLUDE_PCI

#define  PCIE_CONTROLLER_0 0
#define  PCIE_CONTROLLER_1 1

/*
 * PCIE bus number arrangement.
 * Bus 0 : backplane bus
 * Bus 1~16 : PCIE controller 0
 * Bus 17~32 : PCIE controller 1
 */
#define  PCIE0_BUS_MIN   0
#define  PCIE0_BUS_MAX   16
#define  PCIE1_BUS_MIN   17
#define  PCIE1_BUS_MAX   32

/* constant needed for pciAutoConfig */
#define PCI_LAT_TIMER       0x80

/* Includes I2C support */
#define INCLUDE_I2C_DEV

/* MMU configuration
 *
 * MIPS_TLB_PAGE_SIZES is a bitmask that defines the supported MMU Page
 * Sizes for this BSP.  The following Page Sizes are supported:
 *   MMU_PAGE_MASK_8K,   MMU_PAGE_MASK_32K, MMU_PAGE_MASK_128K,
 *   MMU_PAGE_MASK_512K, MMU_PAGE_MASK_2M
 * VM_PAGE_SIZE must be set to the minimum page size define in
 * MIPS_TLB_PAGE_SIZES.
 */

#define MIPS_TLB_PAGE_SIZES        MMU_PAGE_MASK_8K
#undef VM_PAGE_SIZE
#define VM_PAGE_SIZE               TLB_8K_PAGE_SIZE
#define MMU_USE_EXC_TLB_PAGE       TRUE

/* CPU-specific facilities */

#define ROM_AUTO_CACHE_DETECT

#undef  USER_D_CACHE_MODE
#define USER_D_CACHE_MODE   CACHE_COPYBACK
#undef  USER_I_CACHE_MODE
#define USER_I_CACHE_MODE   CACHE_COPYBACK

#if !defined (ROM_AUTO_CACHE_DETECT)
#define ICACHE_SIZE     0x8000
#define ICACHE_LINE_SIZE    0x20
#define DCACHE_SIZE     0x8000
#define DCACHE_LINE_SIZE    0x20
#endif

#define SCACHE_SIZE     0x0
#define SCACHE_LINE_SIZE    0x0
#define TCACHE_SIZE     0x0
#define TCACHE_LINE_SIZE    0x0


/* miscellaneous definitions */

/* allocate 240 bytes in NVRAM for the bootline */

#undef  BOOT_LINE_SIZE
#define BOOT_LINE_SIZE      240 /* use last 240 bytes for bootline */

#ifdef NV_BOOT_OFFSET
#undef NV_BOOT_OFFSET
#endif
#define NV_BOOT_OFFSET      0
#define NV_EXT_OFFSET       128 /* beginning of extended NVRAM region  */

#define DEFAULT_BOOT_LINE \
"et(0,0)10.144.64.202:bcm.keystone h=10.144.64.22 e=10.144.2.126:ffffff00 g=10.144.2.1 u=tornado pw=tornado+"

#define INCLUDE_DOSFS           /* dosFs file system */
#define INCLUDE_FLASH_BOOT
#define INCLUDE_FLASH

/******************************************************************************/
/*                                                                            */
/*                          SERIAL SUPPORT                                    */
/*                                                                            */
/******************************************************************************/
#undef  NUM_TTY
#define NUM_TTY   num_TTY()

#ifdef CONSOLE_BAUD_RATE
#undef CONSOLE_BAUD_RATE                
#endif
#define CONSOLE_BAUD_RATE       9600          /* console baud rate */

/* system clock rate */
#define CPU_CLOCK_RATE      mipsclk 

#define SYS_CLK_RATE_MIN    1   /* minimum system clock rate */
#define SYS_CLK_RATE_MAX    5000    /* maximum system clock rate */

/* the low-end is limited by the 16 bit counter in the 8254 counter/timer
 * the high-end is limited mostly by interrupt service overhead
 */

#define	INCLUDE_AUX_CLK     /* Auxilliary clock support */
#define AUX_CLK_RATE_MIN    20  /* minimum auxiliary clock rate */
#define AUX_CLK_RATE_MAX    5000    /* maximum auxiliary clock rate */

#define SYS_MODEL "BCM53000 (MIPS74K)"

#define  INCLUDE_TIMESTAMP

/******************************************************************************/
/*                                                                            */
/*                       MEMORY CONSTANTS                                     */
/*                                                                            */
/******************************************************************************/
/* memory constants */
#define LOCAL_MEM_LOCAL_ADRS        0x80000000 
#define LOCAL_MEM_SIZE64              0x04000000  /* 64MB  memory */
#define LOCAL_MEM_SIZE128             0x08000000  /* 128MB  memory */
#define LOCAL_MEM_SIZE256             0x10000000  /* 256MB  memory */
#define LOCAL_MEM_SIZE      LOCAL_MEM_SIZE128  /* 128 MB */

#if 0
#define CACHE_MEM_POOL_RESERVED
#endif
#ifdef  CACHE_MEM_POOL_RESERVED
#define CACHEMEM_POOL_SIZE           (64<<20) /* 64MB */
#endif  /* CACHE_MEM_POOL_RESERVED */

#if 1
#define USER_RESERVED_MEM   (0x00200000)     /* number of reserved bytes */
#else
/* Reserve memory for CFE.  Currently reserving 0x87E7E000 and up */
#define USER_RESERVED_MEM   (0x00182000)     /* number of reserved bytes */
#endif

/*
 * The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_LOW_ADRS and
 * RAM_HIGH_ADRS are defined in config.h, and MakeSkel.
 * All definitions for these constants must be identical.
 */

#define ROM_TEXT_ADRS       0xbfc00000  /* base address of ROM */
#define ROM_BASE_ADRS       ROM_TEXT_ADRS   /* as the PF wants it. */
#define ROM_WARM_ADRS       (ROM_BASE_ADRS + 8) /* warm boot entry address */


#define ROM_SIZE                0x00200000      /* 512KB ROM space */
#define RAM_LOW_ADRS            0x80100000      /* RAM address for kernel/compressed code storage start */
#define RAM_HIGH_ADRS           0x83c00000      /* RAM address for ROM boot/RAM code start address */

#ifdef __cplusplus
}
#endif

#ifndef BOOTAPP
#ifdef BROADCOM_BSP
#define INCLUDE_USER_APPL
#define USER_APPL_INIT { extern void vxSpawn(void); vxSpawn(); }
#define INCLUDE_SHELL
#define INCLUDE_SHELL_INTERP_C  /* C interpreter */
#define INCLUDE_SHELL_INTERP_CMD /* shell command interpreter */

#else
#define INCLUDE_SHELL
#define INCLUDE_SHELL_INTERP_C  /* C interpreter */
#define INCLUDE_SHELL_INTERP_CMD /* shell command interpreter */
#endif
#endif

#ifndef _ASMLANGUAGE
extern int num_TTY(void);
extern unsigned long readCount(void);
extern unsigned long readInstr(void);
extern void APPLICATION(void);   
#endif  /* _ASMLANGUAGE */

#endif  /* __INCconfigh */
#if defined(PRJ_BUILD)
#include "prjParams.h"
#endif /* defined PRJ_BUILD */

#if 1 /* For assembly debug */
#define JATRACE_REG1_ADDR   0xb8000048
#define JATRACE_REG2_ADDR   0xb8000044
#define JATRACE_LV(l,v)                        \
        li      v, JATRACE_SRC         ;    \
        li      l, JATRACE_REG1_ADDR   ;    \
        sw      v, 0(l)                ;    \
        li      v, __LINE__            ;    \
        li      l, JATRACE_REG2_ADDR   ;    \
        sw      v, 0(l)
#define JATRACE()   JATRACE_LV(k1,k0)
#define JATRACE1()   JATRACE_LV(a0,t0)
#define JATRACE2()   JATRACE_LV(s7,t9)
/* TO DEFINE: JATRACE_SRC */
#define JA_ASCII2INT(a, b, c, d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))
#endif 

#if 0
extern volatile unsigned int *plog;
#define JTRACE() do { *plog++ = __LINE__; } while(0)
#define JLOG(x)  do { *plog++ = (unsigned int)(x); } while(0)

#endif

