/* config.h - Wind River SBC8548 BSP configuration file */

/* $Id: config.h,v 1.25 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01d,06apr07,b_m  modify to use m85xxCCSR driver.
01c,27mar07,b_m  add MPC8548 rev.2 and TFFS define.
01b,27feb06,kds  removing INCLUDE_SHOW_ROUTINES
01a,30jan06,kds  Modify from cds8548/config.h/01d
*/

#ifndef	__INCconfigh
#define	__INCconfigh

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#define BSP_VER_1_1     1
#define BSP_VER_1_2     1
#define BSP_VERSION     "2.0"
#define BSP_REV         "/2"

/* for MPC8548 rev.2 silicon */
#define REV2_SILICON

#define SYS_MODEL           "Broadcom GTO"

#define SYS_MODEL_8548E     "Broadcom SBC8548E - Security Engine"
#define SYS_MODEL_8548      "Broadcom SBC8548"
#define SYS_MODEL_8547E     "Broadcom SBC8547E - Security Engine"
#define SYS_MODEL_8545E     "Broadcom SBC8545E - Security Engine"
#define SYS_MODEL_8545      "Broadcom SBC8545"
#define SYS_MODEL_8543E     "Broadcom SBC8543E - Security Engine"
#define SYS_MODEL_8543      "Broadcom SBCC8543"

#define SYS_MODEL_E500      "Freescale E500 : Unknown system version"
#define SYS_MODEL_UNKNOWN   "Freescale Unknown processor"

#undef INCLUDE_AUX_CLK

/* Define Clock Speed and source  */

#define	FREQ_33_MHZ	 33000000
#define	FREQ_66_MHZ	 66000000
#define	FREQ_100_MHZ	 99999999
#define	FREQ_133_MHZ	133333333
#define	FREQ_266_MHZ	266666666
#define FREQ_400_MHZ    400000000
#define	FREQ_333_MHZ	333333333
#define	FREQ_533_MHZ	533333333

#define	OSCILLATOR_FREQ	FREQ_66_MHZ

/*
 * DDR will work only for 400Mhz at the moment
 * These ratios are tested in bootrom for correct init
 */

#if (OSCILLATOR_FREQ == FREQ_33_MHZ)
#define PLAT_RATIO_533_MHZ 16 /* Unsupported */
#define PLAT_RATIO_400_MHZ 12
#define PLAT_RATIO_333_MHZ 8 /* Unsupported */
#endif
#if (OSCILLATOR_FREQ == FREQ_66_MHZ)
#define PLAT_RATIO_533_MHZ 8 /* Unsupported */
#define PLAT_RATIO_400_MHZ 6
#define PLAT_RATIO_333_MHZ 4 /* Unsupported */
#endif


#undef INCLUDE_DDR_ECC
#define DEFAULT_SYSCLKFREQ FREQ_400_MHZ

#define FORCE_DEFAULT_FREQ /* Use to force freq used with DUART/Timers etc */

/* This value is the 60x bus-assigned SDRAM Refresh Timer PSRT setting */

#define	LSRT_VALUE	0x20

/*
 * This value is the setting for the MPTPR[PTP] Refresh timer prescaler.
 * The value is dependent on the OSCILLATOR_FREQ value.  For other values
 * a conditionally compiled term must be created here for that OSCILLATOR_FREQ
 * value.
 *
 * BRGCLK_DIV_FACTOR
 * Baud Rate Generator division factor - 0 for division by 1
 *					 1 for division by 16
 */

#define	DIV_FACT_1	0
#define	DIV_FACT_16	1

/*
 * Assume 533MHz CCB with local bus clk ratio of 8
 */

#define	TPR			0x0000

#include "configSdkPre.h"   /* Configs for Broadcom SDK */
#include <configAll.h>
#include "configSdkPost.h"  /* Configs for Broadcom SDK */

#include "gto.h"

#define INCLUDE_I2C_DEV
/* Includes I2C support
 * MPC8548 has two I2C bus controller.
 * Bus 0 M41T81M
 * Bus 1 Dev 0x50 - EEPROM
 *       Dev 0x27 - Base Board ID (Rev 1 = 0x51)
 *       Dev xxx  - DDR SDRAM EEPROM 
 */
#define INCLUDE_NV_RAM

/* NVRAM configuration */
#ifdef  INCLUDE_NV_RAM
#   undef  NV_RAM_SIZE
#define NV_RAM_SIZE             (16 * 1024)   /* 16 Kbytes = 128 Kbits */
#   define NV_RAM_ADRS 0
#   define NV_RAM_INTRVL        1
#   undef  NV_BOOT_OFFSET
#   define NV_BOOT_OFFSET       0x3830
#   define NV_MAC_ADRS_OFFSET   0x3800
#else
#   define NV_RAM_SIZE 0
#   define NV_RAM_ADRS 0
#endif  /* INCLUDE_NV_RAM */

/* Common Includes for VXBUS RIO and ETSEC */
#define INCLUDE_VXBUS

#undef INCLUDE_RAPIDIO_BUS

/*
 * RAPIDIO supports only point to point shared memory support
 * This includes TIPC and SM objects
 */

#ifdef INCLUDE_RAPIDIO_BUS
#define INCLUDE_SM_COMMON
#define INCLUDE_M85XX_RAPIDIO
#define INCLUDE_M85XX_CPU
#define DRV_RESOURCE_M85XXCCSR
#define INCLUDE_M85XX_RIO_SM_CFG
#define INCLUDE_VXBUS_SM_SUPPORT
#define RAPIDIO_BUS_STATIC_TABLE
#define VXBUS_TABLE_CONFIG
#endif


#ifdef INCLUDE_VXBUS
#define INCLUDE_VXBUS_INIT
#define INCLUDE_HWMEM_ALLOC
#define INCLUDE_VXBUS_ACTIVATE
#define INCLUDE_PARAM_SYS
#define INCLUDE_PLB_BUS
#define INCLUDE_VXBUS_SHOW
#define INCLUDE_MOT_ETSEC_HEND
#define INCLUDE_MII_BUS
#define INCLUDE_GENERICPHY
#undef INCLUDE_VSC82XXPHY
#define INCLUDE_END
#define INCLUDE_HEND_PARAM_SYS
#define HWMEM_POOL_SIZE 50000
#endif

/*
 * Other useful includes:
 * #define INCLUDE_IFMEDIA
 * #define INCLUDE_VXBUS_SHOW
 * #define INCLUDE_IFCONFIG
 * #define INCLUDE_NET_IF_SHOW
 * #define INCLUDE_SHOW_ROUTINES
 * #define INCLUDE_PCI_CFGSHOW
 * #define INCLUDE_ISR_OBJECTS
 */

/*
 * Need LINKHDR extension to support FCB in ETSEC to
 * reduce FCB insertion requiring it's own BD
 */

#undef MAX_LINKHDR_CFG
#define MAX_LINKHDR_CFG       32

#define WDT_RATE_MIN         (sysTimerClkFreq / (1 << 29))
#define WDT_RATE_MAX         (sysTimerClkFreq / (1 << 21))

#define DEFAULT_BOOT_LINE \
"motetsec(0,0)vaal:/home/labguy/boot/aung/tot/bcm.gto h=10.18.39.53 e=10.18.33.104 g=10.18.32.1 \
u=ntswboot pw=ntswboot+ f=0x7000"
#define INCLUDE_END

/* MMU and CACHE */

#define INCLUDE_MMU_BASIC
#define USER_I_MMU_ENABLE
#define USER_D_MMU_ENABLE

#undef E500_L1_PARITY_RECOVERY

#ifdef E500_L1_PARITY_RECOVERY

  /*
   * *** NOTE FOR PROJECT FACILITY USERS ***
   * Needs to use WRITETHROUGH, building with Project Facility must also
   * change USER_D_CACHE_MODE and USER_I_CACHE_MODE in Project Facility.
   */

# define CACHE_LIBRARY_MODE     CACHE_WRITETHROUGH
# define CAM_DRAM_CACHE_MODE    _MMU_TLB_ATTR_W
# define TLB_CACHE_MODE         VM_STATE_CACHEABLE_WRITETHROUGH

#else  /* E500_L1_PARITY_RECOVERY */

# define CACHE_LIBRARY_MODE     (CACHE_COPYBACK | CACHE_SNOOP_ENABLE)
# define CAM_DRAM_CACHE_MODE    _MMU_TLB_ATTR_M
# define TLB_CACHE_MODE         VM_STATE_CACHEABLE | VM_STATE_MEM_COHERENCY

#endif

#define INCLUDE_CACHE_SUPPORT
#define USER_D_CACHE_ENABLE
#undef  USER_D_CACHE_MODE
#define USER_D_CACHE_MODE  (CACHE_LIBRARY_MODE)
#define USER_I_CACHE_ENABLE
#undef  USER_I_CACHE_MODE
#define USER_I_CACHE_MODE (CACHE_LIBRARY_MODE)

#define  INCLUDE_L2_CACHE
#undef   INCLUDE_L2_SRAM

  /*
   * If E500_L1_PARITY_RECOVERY is not defined, use local BSP handler.
   * Works for L1 instr cache but not data cache.  Writethrough not needed.
   */

#ifdef E500_L1_PARITY_RECOVERY

# if defined(INCLUDE_CACHE_SUPPORT) && defined(USER_I_CACHE_ENABLE)
#   if (defined(_WRS_VXWORKS_MAJOR) && (_WRS_VXWORKS_MAJOR >= 6))
#     define INCLUDE_L1_IPARITY_HDLR              /* VxWorks 6.x */
#   else  /* _WRS_VXWORKS_MAJOR */
#     define INCLUDE_L1_IPARITY_HDLR_INBSP        /* VxWorks 5.5.x */
#   endif /* _WRS_VXWORKS_MAJOR */
# endif  /* INCLUDE_CACHE_SUPPORT && USER_I_CACHE_ENABLE */

#endif  /* E500_L1_PARITY_RECOVERY */

#define INCLUDE_BRANCH_PREDICTION

#if ((defined(INCLUDE_L2_CACHE)) && (defined(INCLUDE_L2_SRAM)))
#define L2_CACHE_SIZE      L2SIZ_256KB
#define L2_SRAM_SIZE       L2SIZ_256KB
#elif ((defined(INCLUDE_L2_CACHE)) && (!defined(INCLUDE_L2_SRAM)))
#define L2_CACHE_SIZE      L2SIZ_512KB
#define L2_SRAM_SIZE       0         /* Not Used */
#else
#define L2_SRAM_SIZE       L2SIZ_512KB
#define L2_CACHE_SIZE      0         /* Not Used */
#endif

#define L2SRAM_ADDR        0x7FF80000
#define L2SRAM_WINDOW_SIZE 0x80000

/* Disable Support for SPE 64bit registers */

#define INCLUDE_SPE

#define INCLUDE_SYS_HW_INIT_0		/* extended vectors */


/* TSEC is included */

#define INCLUDE_MOT_TSEC_END

#ifdef INCLUDE_MOT_TSEC_END
#define INCLUDE_PRIMARY_TSEC_END
#define INCLUDE_SECONDARY_TSEC_END
#undef INCLUDE_TERTIARY_TSEC_END
#undef INCLUDE_QUARTINARY_TSEC_END
#define INCLUDE_END
#endif /* INCLUDE_MOT_TSEC_END */

/* console is always DUART */

#define INCLUDE_DUART

/* Serial channel and TTY */

#if defined(INCLUDE_DUART)
#  undef  NUM_TTY
#  define NUM_TTY 1
#endif /* INCLUDE_DUART */

/* Clock rates */

#define	SYS_CLK_RATE_MIN	1	/* minimum system clock rate */
#define	SYS_CLK_RATE_MAX	8000	/* maximum system clock rate */
#define	AUX_CLK_RATE_MIN	((CCB_FREQ / 16) / (1 << FIT_DIVIDER_TAP_21))	/* minimum auxiliary clock rate */
#define	AUX_CLK_RATE_MAX	((CCB_FREQ / 16) / (1 << FIT_DIVIDER_TAP_10))	/* maximum auxiliary clock rate */
#define AUX_CLK_RATE        ((CCB_FREQ / 16) / (1 << FIT_DIVIDER_TAP_18))

/*
 * Although the support for point to point has been added it is untested
 * with Serial RIO. SImilar was tested on parallel RIO
 */

#ifdef INCLUDE_RAPIDIO_BUS

/* RapidIO information */

#define RAPIDIO_REG_BASE 0xc0000
#define RAPIDIO_BASE   (CCSBAR + RAPIDIO_REG_BASE)
#define RAPIDIO_ADRS   0xc0000000
#define RAPIDIO_SIZE   0x10000000
#define RAPIDIO_BUS_ADRS 0x80000000
#define RAPIDIO_BUS_SIZE 0x10000000
#endif

/* PCI support is available just not defined by default */
#define INCLUDE_PCI

#ifdef  INCLUDE_PCI

#define PCI_MAX_BUS             32

#define EPIC_EX_DFT_POLAR	EPIC_INT_ACT_LOW

#define CDS85XX_PCI_1_BUS       1
#define CDS85XX_PCI_2_BUS       2
#define CDS85XX_PCIEX_BUS       3

/* PCI 2 not available on SBC8548 (single 64b PCI bus only) */

#undef  INCLUDE_CDS85XX_SECONDARY_PCI

/* define this MACRO to enable PCI Express */
#define  PCIEX_BUS_MIN   128
#define  PCIEX_BUS_MAX   255
#define  INCLUDE_CDS85XX_PCIEX

/*  cds85xx support dual PCI controllers */

/*

CPU Addr                                  PCI Addr ( PCI1 or PCI2)
PCI_LOCAL_MEM_BUS        -------------------------- PCI_MSTR_MEM_BUS
                         -                        -
                         -                        -
PCI_LOCAL_MEM_BUS +      -------------------------- PCI_MSTR_MEM_BUS +
PCI_LOCAL_MEM_SIZE       -                        - PCI_MSTR_MEM_SIZE
                         -                        -
                         -                        ----- PCI Bridge (for PCI1 only)
                         -                        -     configuration regs
                         -                        -
CPU_PCI_MEM_ADRS  (PCI1) -------------------------- PCI_MEM_ADRS
CPU_PCI_MEM_ADRS2 (PCI2) -                        - PCI_MEM_ADRS2
CPU_PCI_MEM_ADRS3(PCIEX) -------------------------- PCI_MEM_ADRS3
                         -                        -
CPU_PCI_MEMIO_ADRS       -------------------------- PCI_MEMIO_ADRS
CPU_PCI_MEMIO_ADRS2      -                        - PCI_MEMIO_ADRS2
CPU_PCI_MEMIO_ADRS3      -                        - PCI_MEMIO_ADRS3
                         -                        -
CPU_PCI_IO_ADRS  (PCI1)  -------------------------- PCI_IO_ADRS
CPU_PCI_IO_ADRS2 (PCI2)  -                        - PCI_IO_ADRS2
CPU_PCI_IO_ADRS3 (PCIEX) -                        - PCI_IO_ADRS3
                         -                        -
CPU_PCI_IO_ADRS  (PCI1)+ -------------------------- PCI_IO_ADRS +
CPU_PCI_IO_ADRS2 (PCI2)+                            PCI_IO_ADRS2 +
CPU_PCI_IO_ADRS2 (PCI3)                             PCI_IO_ADRS3
CPU_PCI_IO_SIZE          -                        - PCI_IO_SIZE
                         -                        -
*/

/* PCI based addresses */

#define PCI_LAW_BASE             0x50000000       /* PCI LAW window */
#define PCI_LAW_SIZE             0x10000000       /* PCI LAW size   */

#define PCI_MEM_SIZE             0x04000000        /* 64 MB */
#define PCI_MEMIO_SIZE           0x04000000        /* 64 MB */
#define PCI_IO_SIZE              0x04000000        /* 64 MB */

#define PCI_MEM_ADRS             (PCI_LAW_BASE)
#define PCI_MEMIO_ADRS           (PCI_MEM_ADRS   + PCI_MEM_SIZE)
#define PCI_IO_ADRS              (PCI_MEMIO_ADRS + PCI_MEMIO_SIZE)

#define PCIEX_LAW_BASE            0x60000000       /* PCI LAW window */
#define PCIEX_LAW_SIZE            0x10000000       /* PCI LAW size   */

#define PCI_MEM_ADRS3            (PCIEX_LAW_BASE)
#define PCI_MEMIO_ADRS3          (PCI_MEM_ADRS3   + PCI_MEM_SIZE)
#define PCI_IO_ADRS3             (PCI_MEMIO_ADRS3 + PCI_MEMIO_SIZE)

#if (PCI_LAW_SIZE == 0x10000000)
#define  PCI_LAW_WIN_SZ          LAWAR_SIZE_256MB
#endif

#if (PCI2_LAW_SIZE == 0x10000000)
#define  PCI2_LAW_WIN_SZ         LAWAR_SIZE_256MB
#endif

#if (PCIEX_LAW_SIZE == 0x10000000)
#define  PCIEX_LAW_WIN_SZ         LAWAR_SIZE_256MB
#endif

#define PCI_MMU_TLB_SZ           _MMU_TLB_SZ_256M

#if (PCI_MEM_SIZE == 0x04000000)
#define PCI_MEM_SIZE_MASK        PCI_ATTR_WS_64M
#endif

#if (PCI_MEMIO_SIZE == 0x04000000)
#define PCI_MEMIO_SIZE_MASK      PCI_ATTR_WS_64M
#endif

#if (PCI_IO_SIZE == 0x04000000)
#define PCI_IO_SIZE_MASK         PCI_ATTR_WS_64M
#endif

#define PCI_BRIDGE_PIMMR_BASE_ADRS  0x40000000

/* just 1:1 mapping */

/* PCI 1 */

#define CPU_PCI_MEM_ADRS          PCI_MEM_ADRS
#define CPU_PCI_MEMIO_ADRS        PCI_MEMIO_ADRS
#define CPU_PCI_IO_ADRS           PCI_IO_ADRS

/* PCI 2 */

#define CPU_PCI_MEM_ADRS2         PCI_MEM_ADRS2
#define CPU_PCI_MEMIO_ADRS2       PCI_MEMIO_ADRS2
#define CPU_PCI_IO_ADRS2          PCI_IO_ADRS2

/* PCI Express */

#define CPU_PCI_MEM_ADRS3         PCI_MEM_ADRS3
#define CPU_PCI_MEMIO_ADRS3       PCI_MEMIO_ADRS3
#define CPU_PCI_IO_ADRS3          PCI_IO_ADRS3

#define CPU_PCI_MEM_SIZE          PCI_MEM_SIZE
#define CPU_PCI_MEMIO_SIZE        PCI_MEMIO_SIZE
#define CPU_PCI_IO_SIZE           PCI_IO_SIZE

/* CPU from PCI bus */

#define PCI_MSTR_MEM_BUS           0x00000000
#define PCI_MSTR_MEM_SIZE          0x40000000 /* 1G */

/* CPU Address that is visible from PCI */

#define PCI_LOCAL_MEM_BUS         0x00000000
#define PCI_LOCAL_MEM_SIZE        PCI_MSTR_MEM_SIZE
#define PCI_LOCAL_MEM_SIZE_MASK   PCI_ATTR_WS_1G

#define INCLUDE_PCI_AUTOCONF

#ifndef PCI_CFG_TYPE
#   ifdef INCLUDE_PCI_AUTOCONF
#      define PCI_CFG_TYPE PCI_CFG_AUTO
#   else
#      define PCI_CFG_TYPE PCI_CFG_FORCE
#   endif /* INCLUDE_PCI_AUTOCONF */
#endif /* PCI_CFG_TYPE */

#endif /* INCLUDE_PCI */

#ifdef INCLUDE_PCI

/* Intel 8254x support */
/* #define INCLUDE_GEI8254X_END*/
#ifdef INCLUDE_GEI8254X_END
 #define INCLUDE_PRIMARY_GEI_END
 #undef INCLUDE_SECONDARY_GEI_END
  #ifndef INCLUDE_END
     #define INCLUDE_END
  #endif  /* INCLUDE_END */
#endif /* INCLUDE_GEI8254X_END */

#endif /* INCLUDE_PCI */

/* Shared meory is set up for Serial RIO but is untested */

#if defined(INCLUDE_SM_COMMON)
#   define INCLUDE_SMEND
#   define INCLUDE_SM_NET
#   define INCLUDE_BSD
#   define INCLUDE_SM_NET_SHOW
#   define INCLUDE_NET_DRV
#   define INCLUDE_NET_LIB
#   ifdef  STANDALONE
#          define STANDALONE_NET
#   endif  /* STANDALONE */


/*
 * Changing SM_OFF_BOARD to TRUE also requires changing
 * SM_ANCHOR_ADRS and SM_MEM_ADRS appropriately.
 */

#define SM_OFF_BOARD    FALSE

#define INCLUDE_SM_SEQ_ADDR
#undef SM_TAS_TYPE
#define SM_TAS_TYPE SM_TAS_HARD

/*
 * INCLUDE_SM_NET and INCLUDE_SM_SEQ_ADDR are the shared memory backplane
 * driver and the auto address setup which are excluded by default.
 * To exclude them, uncomment the following lines:
 *
 * #define INCLUDE_SM_NET
 * #define INCLUDE_SM_SEQ_ADDR
 */

#ifndef _ASMLANGUAGE
IMPORT UINT32 rioHostAdrs;

#undef  SM_ANCHOR_ADRS
#define SM_ANCHOR_ADRS  ((sysProcNumGet() == 0) ? \
        ((char*) (LOCAL_MEM_LOCAL_ADRS + SM_ANCHOR_OFFSET)) : \
        ((char*) (rioHostAdrs + SM_ANCHOR_OFFSET)))

#endif

#define SM_INT_TYPE		SM_INT_BUS  /* or SM_INT_NONE */

#define SM_MEM_ADRS		0x00010000	/* Start of actual SM region */
#define SM_MEM_SIZE		0x00070000
#define SM_OBJ_MEM_ADRS		(SM_MEM_ADRS+SM_MEM_SIZE) /* SM Objects pool */
#define SM_OBJ_MEM_SIZE		0x00040000

#define SM_INT_ARG1   46 /*EPIC_SR_IN_DB1_INT_VEC*/
#define SM_INT_ARG2   46 /*EPIC_SR_IN_DB1_INT_VEC*/
#define SM_INT_ARG3   1


#define SM_TIPC_ADRS_DEFAULT    ((int) SM_OBJ_MEM_ADRS + SM_OBJ_MEM_SIZE)
#define SM_TIPC_ADRS            SM_TIPC_ADRS_DEFAULT
#define SM_TIPC_SIZE_DEFAULT    0x00020000
#define SM_TIPC_SIZE            SM_TIPC_SIZE_DEFAULT /* SM TIPC pool size  */

#undef IPFORWARDING_CFG
#define IPFORWARDING_CFG        TRUE

#endif /* defined(INCLUDE_SM_NET) */


/*
 * Set the Carrier Card's SW2[12]=00 for the following configuration,
 * the bootrom would be in the last 1 MB of flash0, and UBOOT would still
 * be in the flash 1.
 */

#define FLASH_WINDOW_SIZE               0x08000000
#define CDS85XX_FLASH_RESERVED_SIZE     0x00100000

/* LBC CS0 - flash 0 - 64MB, 16-bit flash - default for bootrom */

#define FLASH0_BASE_ADRS                 0xf8000000
#define FLASH0_ADRS_MASK                 0xf8000000
#define FLASH0_SIZE                      0x08000000

/* LBC CS1 - flash 1 - 64MB, 16-bit flash */

#define FLASH1_BASE_ADRS                 0xf0000000
#define FLASH1_ADRS_MASK                 0xf8000000
#define FLASH1_SIZE                      0x08000000


/* LBC CS2 - Alphanumeric LED Display */
#define ALPHA_LED_BASE_ADRS             0xe8000000
#define ALPHA_LED_ADRS_MASK             0xe8000000
#define ALPHA_LED_SIZE                  0x01000000

/* Memory addresses */

#define INCLUDE_DDR_SDRAM

/* NOTE this should match the LAWAR SIZE in romInit for the chosen SDRAM */

#define LOCAL_MEM_SIZE          0x20000000  /* 256/512 Mbyte memory available */
#define LOCAL_MEM_LOCAL_ADRS    0x00000000  /* Base of RAM */

#define DDR_SDRAM_LOCAL_ADRS       LOCAL_MEM_LOCAL_ADRS
#define DDR_SDRAM_LOCAL_SIZE       LOCAL_MEM_SIZE
#define DDR_SDRAM_LOCAL_ADRS_END   (DDR_SDRAM_LOCAL_ADRS + DDR_SDRAM_LOCAL_SIZE)

#ifdef INCLUDE_MMU_BASIC

#define INCLUDE_AIM_MMU_CONFIG
#define INCLUDE_MMU_OPTIMIZE

#define INCLUDE_AIM_MMU_SHOW

#define INCLUDE_AIM_MMU_MEM_POOL_CONFIG

/*
 * The following parameters are to configure initial memory usage for
 * page table and region tables and in event of running out the increment
 * of memory allocation and is specified as a number of MMU pages (4KByte
 * blocks).
 */

#define AIM_MMU_INIT_PT_NUM 0x40
#define AIM_MMU_INIT_PT_INCR 0x20
#define AIM_MMU_INIT_RT_NUM 0x10
#define AIM_MMU_INIT_RT_INCR 0x10

/* This write protects the page tables */

#define INCLUDE_AIM_MMU_PT_PROTECTION

/*
 * INCLUDE_LOCK_TEXT_SECTION Requires use of CAM entries which can
 * possibly be already used in sysStaticTlbDesc. Also the larger the
 * page size that can be used the less CAM entries required so aligning
 * RAM_LOW_ADRS on a large page size boundary is usually necessary.
 * Locking the text section should increase performance because no
 * Instruction TLB misses are taken within the text section.
 */

#define INCLUDE_LOCK_TEXT_SECTION
#undef INCLUDE_LOCK_TEXT_SECTION
#undef INCLUDE_PAGE_SIZE_OPTIMIZATION /* Not available for 85XX */

#endif /* INCLUDE_MMU_BASIC */

/*
 * Default power management mode - selected via vxPowerModeSet() in
 * sysHwInit().
 */

#define DEFAULT_POWER_MGT_MODE  VX_POWER_MODE_DISABLE

#define INCLUDE_CTORS_DTORS

/*
 * Using software FP support. Athough task based 32 bit HW_FP is allowed
 * no optimised libraries are available. The kernel doesn't execute any hw
 * fp instructions
 */

#undef  INCLUDE_HW_FP
#undef  INCLUDE_PPC_FPU
#define INCLUDE_SW_FP

/*
 * The constants ROM_XXX_ADRS, ROM_SIZE, and RAM_XXX_ADRS are defined
 * in both config.h and Makefile.  All definitions for these constants
 * must be identical.
 */

#ifndef RAM_LOW_ADRS
#define	RAM_LOW_ADRS		0x00100000
#endif  /* RAM_LOW_ADRS */

#ifndef RAM_HIGH_ADRS
#define	RAM_HIGH_ADRS		0x08000000
#endif  /* RAM_HIGH_ADRS */

#define ROM_TEXT_ADRS 0xFFF00100
#define ROM_BASE_ADRS 0xFFF00000
#define ROM_SIZE      0x00100000

#define USER_RESERVED_MEM 0x000000

#define INCLUDE_SYSLED

#ifdef INCLUDE_END

#   define ETHERNET_MAC_HANDLER

#   define BRCM_ENET0       0x00 /* BRCM specific portion of MAC (MSB->LSB) */
#   define BRCM_ENET1       0x10
#   define BRCM_ENET2       0x18

#   define CUST_ENET3_0     0xA0    /* Customer portion of MAC address */
#   define CUST_ENET3_1     0xA1
#   define CUST_ENET3_2     0xA2
#   define CUST_ENET3_3     0xA3
#   define CUST_ENET4       0xAA
#   define CUST_ENET5       0xA0

#   define MAX_MAC_ADRS     4
#   define MAC_ADRS_LEN     6

#endif

/* add TFFS as boot device */
#undef INCLUDE_TFFS

#ifdef INCLUDE_TFFS
#   define INCLUDE_TFFS_MOUNT
#   define INCLUDE_TFFS_SHOW
#endif

#ifdef INCLUDE_TFFS
#   ifndef INCLUDE_DOSFS
#       define INCLUDE_DOSFS        /* file system to be used */
#   endif   /* INCLUDE_DOSFS */
#endif  /* INCLUDE_TFFS */

#define INCLUDE_FLASH
#ifdef INCLUDE_FLASH
#define INCLUDE_FLASH_BOOT
#define INCLUDE_DOSFS
#endif

/* Use DOS File System */
#ifdef INCLUDE_DOSFS
#   define  INCLUDE_DOSFS_MAIN
#   define  INCLUDE_DOSFS_CACHE
#   define  INCLUDE_DOSFS_FAT
#	define	INCLUDE_DOSFS_FMT
#	define	INCLUDE_DOSFS_DIR_FIXED
#   define  INCLUDE_DOSFS_DIR_VFAT
#   define  INCLUDE_DISK_UTIL
#endif  /* INCLUDE_DOSFS */

#if !defined(PRJ_BUILD)
/* configuration for bootrom */
#   define INCLUDE_ERF
#   define INCLUDE_XBD
#   define INCLUDE_XBD_BLK_DEV
#   define INCLUDE_FS_MONITOR
#   define INCLUDE_FS_EVENT_UTIL
#   define INCLUDE_DEVICE_MANAGER
#endif

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif  /* INCconfigh */

#if defined(PRJ_BUILD)
    #include "prjParams.h"
#endif /* PRJ_BUILD */
