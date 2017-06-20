/* wrSbc8548.h - Wind River SBC8548  board header */

/* $Id: gto.h,v 1.5 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,25apr07,b_m  add auxClock parameters.
01a,31jan06,kds  Modified from cds8548/cds8548.h/01b
*/

/*
This file contains I/O addresses and related constants for the
Broadcom SBC8548  board.
*/

#ifndef	INCgtoh
#define	INCgtoh

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#ifndef EIEIO_SYNC
#   define EIEIO_SYNC  _WRS_ASM (" eieio; sync")
#endif        /* EIEIO_SYNC */
#ifndef EIEIO
#   define EIEIO    _WRS_ASM (" eieio")
#endif        /* EIEIO */

#ifndef M8260ABBREVIATIONS
#define M8260ABBREVIATIONS

#ifdef  _ASMLANGUAGE
#define CAST(x)
#else /* _ASMLANGUAGE */
typedef volatile UCHAR VCHAR;   /* shorthand for volatile UCHAR */
typedef volatile INT32 VINT32; /* volatile unsigned word */
typedef volatile INT16 VINT16; /* volatile unsigned halfword */
typedef volatile INT8 VINT8;   /* volatile unsigned byte */
typedef volatile UINT32 VUINT32; /* volatile unsigned word */
typedef volatile UINT16 VUINT16; /* volatile unsigned halfword */
typedef volatile UINT8 VUINT8;   /* volatile unsigned byte */
#define CAST(x) (x)
#endif  /* _ASMLANGUAGE */

#endif /* M8260ABBREVIATIONS */

/* Base Address of Memory Mapped Registers */
#define CCSBAR  0xE0000000

/*
 * Timer clock divider ratios (in 2^n)
 */

#define FIT_DIVIDER_TAP_21 21
#define FIT_DIVIDER_TAP_20 20
#define FIT_DIVIDER_TAP_19 19
#define FIT_DIVIDER_TAP_18 18
#define FIT_DIVIDER_TAP_17 17
#define FIT_DIVIDER_TAP_16 16
#define FIT_DIVIDER_TAP_15 15
#define FIT_DIVIDER_TAP_14 14
#define FIT_DIVIDER_TAP_13 13
#define FIT_DIVIDER_TAP_12 12
#define FIT_DIVIDER_TAP_11 11
#define FIT_DIVIDER_TAP_10 10

#define CCB_FREQ            396000000

/* add PCI access macros */
#define PCI_MEMIO2LOCAL(x) \
         (((UINT32)x  - PCI_MEMIO_ADRS) + CPU_PCI_MEMIO_ADRS)

/* PCI IO memory adrs to CPU (60x bus) adrs */
#define PCI_IO2LOCAL(x) \
     (((UINT32)x  - PCI_IO_ADRS) + CPU_PCI_IO_ADRS)

#define PCI_MEM2LOCAL(x) \
         (((UINT32)x  - PCI_MEM_ADRS) + CPU_PCI_MEM_ADRS)

/* 60x bus adrs to PCI (non-prefetchable) memory address */

#define LOCAL2PCI_MEMIO(x) \
     ((int)(x) + PCI_MSTR_MEM_BUS)

#define ETHERNET_MAC_HANDLER

#define BRCM_ENET0  	    0x00  /* BRCM specific portion of MAC (MSB->LSB) */
#define BRCM_ENET1  	    0x10
#define BRCM_ENET2  	    0x18

#define CUST_ENET3_0    0xA0  /* Customer portion of MAC address */
#define CUST_ENET3_1    0xA1
#define CUST_ENET3_2    0xA2
#define CUST_ENET3_3    0xA3
#define CUST_ENET4 	0xAA
#define CUST_ENET5      0xA0

#define MAX_MAC_ADRS 4
#define MAC_ADRS_LEN 6

/* PCI defines begin */

#define PCI_AUTO_CONFIG_ADRS  0x4c00

#define PPCACR_PRKM_MASK  0XF0
#define PCI_REQUEST_LEVEL 0x3

#define CLASS_OFFSET      0xB
#define CLASS_WIDTH       0x1
#define BRIDGE_CLASS_TYPE 0x6

#define PCICMD_ADRS       (PCI_CFG_BASE + 0x04)  /* PCI cmd reg */
#define PCICMD_VAL        0x00000006             /* PCI COMMAND Default value */
#define PCISTAT_ADRS      (PCI_CFG_BASE + 0x06)  /* PCI status reg */

#define NUM_PCI1_SLOTS          0x1
#define NUM_PCI2_SLOTS          0x0
#define NUM_PCIEX_SLOTS         0x1

#define PCI_XINT1_LVL           0x0            /* PCI XINT1 routed to IRQ2  */
#define PCI_XINT2_LVL           0x1            /* PCI XINT2 routed to IRQ3 */
#define PCI_XINT3_LVL           0x2            /* PCI XINT3 routed to IRQ4 */
#define PCI_XINT4_LVL           0x3            /* PCI XINT3 routed to IRQ5 */

#define PCI2_XINT1_LVL          0xb            /* PCI2 XINT1 routed to IRQ11  */

#define PCIEX_XINT1_LVL           0x0            /* PCIEX XINT1 routed to TBD */
#define PCIEX_XINT2_LVL           0x1            /* PCIEX XINT2 routed to TBD */
#define PCIEX_XINT3_LVL           0x2            /* PCIEX XINT3 routed to TBD */
#define PCIEX_XINT4_LVL           0x3            /* PCIEX XINT3 routed to TBD */

#define PCI_LAT_TIMER          0x40            /* latency timer value, 64 PCI clocks */

#define PCI_DEV_ID_85XX  0x000C1057            /* Id for Freescale 8555/8541 PCI 1*/

#define PCI_ARCADIA_BRIDGE_DEV_ID 0x051310E3   /* DEV ID for PCI bridge in Arcadia */

#define PCI1_DEV_ID      0x826010E3
#define PCI2_DEV_ID      0x826110E3
#define PCI3_DEV_ID      0x826210E3
#define PCI_DEV_ID_82XX  0x00031057  /* Id for MPC8266ADS-PCI board - Rev1 */

#define PCI_ID_I82559    0x12298086  /* Id for Intel 82559 */
#define PCI_ID_I82559ER  0x12098086  /* Id for Intel 82559 ER */

#define DELTA(a,b)                 (abs((int)a - (int)b))

#define BUS	0				/* bus-less board */


#define N_SIO_CHANNELS	 	2		/* No. serial I/O channels */
#define DEC_CLOCK_FREQ	OSCILLATOR_FREQ
#define SYS_CLK_FREQ    400000000
#define M85XX_I2C1_BASE 0x3000
#define M85XX_I2C2_BASE 0x3100

/* Local Access Windows Regster Offsets from CCSBAR */
/* LAWBARx
 * 0-11 Reserved - read 0
 * 12-31 Base address - Most significant 20 bits
 *
 * LAWARx
 * 0   Enable window
 * 1-7 Reserved
 * 8-11 Target interface - 0000 PCI/PCI-X
 *                       - 0001 -> 0011 Reserved
 *                       - 0100 Local Bus memory controller eg SDRAM/L2SRAM
 *                       - 0101 -> 1011 Reserved
 *                       - 1100 Rapid IO
 *                       - 1101 -> 1110 Reserved
 *                       - 1111 DDR SDRAM
 * 12-25 Reserved - read 0
 * 26-31 Size of Window  - min 001011 -> 4KBytes
 *                       step power of 2
 *                       - max 011110 -> 2 Gbytes
 */

/* Used for DDR SDRAM */

#define  M85XX_LAWBAR0(base)        (CAST(VUINT32 *)((base) + 0xc08))
#define  M85XX_LAWAR0(base)         (CAST(VUINT32 *)((base) + 0xc10))

/* Not used */

#define  M85XX_LAWBAR1(base)        (CAST(VUINT32 *)((base) + 0xc28))
#define  M85XX_LAWAR1(base)         (CAST(VUINT32 *)((base) + 0xc30))

/* Used for Local Bus Alphanumeric LED */

#define  M85XX_LAWBAR2(base)        (CAST(VUINT32 *)((base) + 0xc48))
#define  M85XX_LAWAR2(base)         (CAST(VUINT32 *)((base) + 0xc50))

/* Used for Local Bus (0,1) FLASH */

#define  M85XX_LAWBAR3(base)        (CAST(VUINT32 *)((base) + 0xc68))
#define  M85XX_LAWAR3(base)         (CAST(VUINT32 *)((base) + 0xc70))

/* Used for PCI 1 */

#define  M85XX_LAWBAR4(base)        (CAST(VUINT32 *)((base) + 0xc88))
#define  M85XX_LAWAR4(base)         (CAST(VUINT32 *)((base) + 0xc90))

/* Used for PCI 2*/

#define  M85XX_LAWBAR5(base)        (CAST(VUINT32 *)((base) + 0xcA8))
#define  M85XX_LAWAR5(base)         (CAST(VUINT32 *)((base) + 0xcB0))

/* Used for PCI Express */

#define  M85XX_LAWBAR6(base)        (CAST(VUINT32 *)((base) + 0xcc8))
#define  M85XX_LAWAR6(base)         (CAST(VUINT32 *)((base) + 0xcd0))

/* Used for Serial RIO */

#define  M85XX_LAWBAR7(base)        (CAST(VUINT32 *)((base) + 0xce8))
#define  M85XX_LAWAR7(base)         (CAST(VUINT32 *)((base) + 0xcf0))

#define  LAWBAR_ADRS_SHIFT       12
#define  LAWAR_ENABLE            0x80000000
#define  LAWAR_TGTIF_PCI         0x00000000
#define  LAWAR_TGTIF_PCI2        0x00100000
#define  LAWAR_TGTIF_PCIEX       0x00200000
#define  LAWAR_TGTIF_LBC         0x00400000
#define  LAWAR_TGTIF_RAPIDIO     0x00C00000
#define  LAWAR_TGTIF_DDRSDRAM    0x00F00000

/* LAWAR SIZE Settings */

#define  LAWAR_SIZE_4KB     0x0000000B
#define  LAWAR_SIZE_8KB     0x0000000C
#define  LAWAR_SIZE_16KB    0x0000000D
#define  LAWAR_SIZE_32KB    0x0000000E
#define  LAWAR_SIZE_64KB    0x0000000F
#define  LAWAR_SIZE_128KB   0x00000010
#define  LAWAR_SIZE_256KB   0x00000011
#define  LAWAR_SIZE_512KB   0x00000012
#define  LAWAR_SIZE_1MB     0x00000013
#define  LAWAR_SIZE_2MB     0x00000014
#define  LAWAR_SIZE_4MB     0x00000015
#define  LAWAR_SIZE_8MB     0x00000016
#define  LAWAR_SIZE_16MB    0x00000017
#define  LAWAR_SIZE_32MB    0x00000018
#define  LAWAR_SIZE_64MB    0x00000019
#define  LAWAR_SIZE_128MB   0x0000001A
#define  LAWAR_SIZE_256MB   0x0000001B
#define  LAWAR_SIZE_512MB   0x0000001C
#define  LAWAR_SIZE_1GB     0x0000001D
#define  LAWAR_SIZE_2GB     0x0000001E

/* Local Bus Controller (LBC) Registers */
/* BRx 0-16 Base Address
 *     17-18 Extended Base Address
 *     19-20 Port Size - 00 reserved
 *                     - 01 8bit
 *                     - 10 16bit
 *                     - 11 32bit
 *     21-22 Data Error Correction
 *                     - 00 reserved
 *                     - 01 Normal parity
 *                     - 10 RMW parity generation (32-bit)
 *                     - 11 reserved
 *     23    Write Protect
 *     24-26 Machine Select = 000 GPCM
 *                          - 001->010 reserved
 *                          - 011 SDRAM
 *                          - 100->110 UPMA->UPMC
 *                          - 111 reserved
 *     28-29 Atomic Access  - 00 No atomic access
 *                          - 01 Read-after-write
 *                          - 10 Write-after-read
 *                          - 11 reserved
 *     31    Valid
 *
 * ORx for SDRAM
 *     0-16  Address mask
 *     17-18 Extended address mask
 *     19-21 Column address lines - 000->111 7->14
 *     23-25 Number of row address lines - 000->110 9->15
 *                                       - 111 Reserved
 *     26    Page mode select
 *     31    External address latch delay
 *
 * ORx for GPCM Mode
 *     0-16  Address mask
 *     17-18 Extended address mask
 *     19    Buffer Control Disable
 *     20    Chip select negation
 *     21-22 Address to chip select setup
 *     23    Extra Address to chip select setup
 *     24-27 Cycle length in Bus clocks - 0000->1111 0->15 wait states
 *     28    External address termination
 *     29    Timing relaxed
 *     30    Extended hold time for read access
 *     31    External address latch delay
 */


#define  M85XX_BR0(base)         (CAST(VUINT32 *)((base) + 0x5000))
#define  M85XX_OR0(base)         (CAST(VUINT32 *)((base) + 0x5004))
#define  M85XX_BR1(base)         (CAST(VUINT32 *)((base) + 0x5008))
#define  M85XX_OR1(base)         (CAST(VUINT32 *)((base) + 0x500c))
#define  M85XX_BR2(base)         (CAST(VUINT32 *)((base) + 0x5010))
#define  M85XX_OR2(base)         (CAST(VUINT32 *)((base) + 0x5014))
#define  M85XX_BR3(base)         (CAST(VUINT32 *)((base) + 0x5018))
#define  M85XX_OR3(base)         (CAST(VUINT32 *)((base) + 0x501c))
#define  M85XX_BR4(base)         (CAST(VUINT32 *)((base) + 0x5020))
#define  M85XX_OR4(base)         (CAST(VUINT32 *)((base) + 0x5024))
#define  M85XX_BR5(base)         (CAST(VUINT32 *)((base) + 0x5028))
#define  M85XX_OR5(base)         (CAST(VUINT32 *)((base) + 0x502C))
#define  M85XX_BR6(base)         (CAST(VUINT32 *)((base) + 0x5030))
#define  M85XX_OR6(base)         (CAST(VUINT32 *)((base) + 0x5034))
#define  M85XX_BR7(base)         (CAST(VUINT32 *)((base) + 0x5038))
#define  M85XX_OR7(base)         (CAST(VUINT32 *)((base) + 0x503C))

#define  M85XX_MAR(base)         (CAST(VUINT32 *)((base) + 0x5068))
#define  M85XX_MAMR(base)         (CAST(VUINT32 *)((base) + 0x5070))
#define  M85XX_MBMR(base)         (CAST(VUINT32 *)((base) + 0x5074))
#define  M85XX_MCMR(base)         (CAST(VUINT32 *)((base) + 0x5078))
#define  M85XX_MRTPR(base)         (CAST(VUINT32 *)((base) + 0x5084))
#define  MRTPR_PTP_MASK 0xff000000
#define  MRTPR_PTP_WRITE(x)   ( (x << 24) & MRTPR_PTP_MASK)
#define  M85XX_MDR(base)         (CAST(VUINT32 *)((base) + 0x5088))

#define  M85XX_LSDMR(base)         (CAST(VUINT32 *)((base) + 0x5094))
#define  LSDMR_RFEN     0x40000000 /* Refresh Enable */

/* LSDMR OP - 000 Normal operation
 *          - 001 Auto Refresh  (Initialization)
 *          - 010 Self Refresh
 *          - 011 Mode Register Write (Initialization)
 *          - 100 Precharge Bank
 *          - 101 Precharge all banks (Initialization)
 *          - 110 Activate Bank
 *          - 111 Read/Write without valid transfer
 */

#define  LSDMR_OP_MASK  0x38000000
#define  LSDMR_OP_SHIFT(x) ((x << 27) & LSDMR_OP_MASK)

/* Bank Select Multiplexed address line - 000 lines 12:13
 *                                      - 001       13:14
 *                                      - 010       14:15
 *                                      - 011       15:16
 *                                      - 100       16:17
 *                                      - 101       17:18
 *                                      - 110       18:19
 *                                      - 111       19:20
 */

#define  LSDMR_BSMA_MASK 0x00E00000
#define  LSDMR_BSMA_SHIFT(x) ((x << 23) & LSDMR_BSMA_MASK)

/* RFCR Refresh recovery 000 - reserved
 *                       001->110 - 3->8 clocks
 *                       111 - 16 clocks
 */

#define  LSDMR_RFCR_MASK 0x00038000
#define  LSDMR_RFCR_SHIFT(x) ((x << 15) & LSDMR_RFCR_MASK)

/* Incomplete LSDMR definitions */

#define  M85XX_LURT(base)         (CAST(VUINT32 *)((base) + 0x50A0))
#define  M85XX_LSRT(base)         (CAST(VUINT32 *)((base) + 0x50A4))
#define  M85XX_LTESR(base)         (CAST(VUINT32 *)((base) + 0x50B0))
#define  M85XX_LTEDR(base)         (CAST(VUINT32 *)((base) + 0x50B4))
#define  M85XX_LTEIR(base)         (CAST(VUINT32 *)((base) + 0x50B8))
#define  M85XX_LTEATR(base)         (CAST(VUINT32 *)((base) + 0x50BC))
#define  M85XX_LTEAR(base)         (CAST(VUINT32 *)((base) + 0x50C0))

/* LBC Clock Configuration */
#define  M85XX_LBCR(base)         (CAST(VUINT32 *)((base) + 0x50D0))
#define  M85XX_LCRR(base)         (CAST(VUINT32 *)((base) + 0x50D4))

#define  M85XX_DCR0(base)         (CAST(VUINT32 *)((base) + 0xe0f1c))
#define  M85XX_DCR1(base)         (CAST(VUINT32 *)((base) + 0xe0f20))

/* ECM Registers */
#define ECM_OFFSET 0x1000
#define ECMBA (CCSBAR | ECM_OFFSET)

/* Offsets for DDR registers */
#define DDR_OFFSET 0x2000
#define DDRBA      (CCSBAR | DDR_OFFSET)

#define CS0_BNDS                  0x000
#define CS1_BNDS                  0x008
#define CS2_BNDS                  0x010
#define CS3_BNDS                  0x018
#define CS0_CONFIG                0x080
#define CS1_CONFIG                0x084
#define CS2_CONFIG                0x088
#define CS3_CONFIG                0x08C
#define EXTENDED_REF_REC          0x100
#define TIMING_CFG_0              0x104
#define TIMING_CFG_1              0x108
#define TIMING_CFG_2              0x10C
#define DDR_SDRAM_CFG             0x110
#define DDR_SDRAM_CFG_2           0x114
#define DDR_SDRAM_MODE_CFG        0x118
#define DDR_SDRAM_MODE_CFG_2      0x11c
#define DDR_SDRAM_MD_CNTL         0x120
#define DDR_SDRAM_INTERVAL        0x124
#define DDR_DATA_INIT             0x128
#define DDR_SDRAM_CLK_CTRL        0x130

#define DDR_DATA_ERR_INJECT_HI    0xe00
#define DDR_DATA_ERR_INJECT_LO    0xe04
#define DDR_ECC_ERR_INJECT        0xe08
#define DDR_CAPTURE_DATA_HI       0xe20
#define DDR_CAPTURE_DATA_LO       0xe24
#define DDR_CAPTURE_ECC           0xe28
#define DDR_ERR_DETECT            0xe40
#define DDR_ERR_DISABLE           0xe44
#define DDR_ERR_INT_EN            0xe48
#define DDR_CAPTURE_ATTRIBUTES    0xe4c
#define DDR_CAPTURE_ADDRESS       0xe50
#define DDR_ERR_SBE               0xe58

#define DDR_IO_OVCR 0x90000000

/* PIC Base Address */

#define PIC_OFFSET 0x40000
#define PCIBA      (CCSBAR | PIC_OFFSET)

/* Global Function Registers */
/* PORPLL used to detect clocking ratio for CCB/CPM for serial devices */
/* Plat Ratio not working on board need to test!!!!*/

#define M85XX_PORPLLSR(base)           (CAST(VUINT32 *)((base) + 0xE0000))
#define M85XX_PORPLLSR_E500_RATIO_MASK 0x003f0000
#define M85XX_PORPLLSR_PLAT_RATIO_MASK 0x0000003e
#define M85XX_PORPLLSR_E500_RATIO(base) ((*M85XX_PORPLLSR(base) & M85XX_PORPLLSR_E500_RATIO_MASK)>>16)
#define M85XX_PORPLLSR_PLAT_RATIO(base) ((*M85XX_PORPLLSR(base) & M85XX_PORPLLSR_PLAT_RATIO_MASK)>>1)

#define M85XX_PORBMSR(base)            (CAST(VUINT32 *)((base) + 0xE0004))
#define M85XX_PORIMPSCR(base)          (CAST(VUINT32 *)((base) + 0xE0008))
#define M85XX_PORDEVSR(base)           (CAST(VUINT32 *)((base) + 0xE000C))
#define M85XX_PORDEVSR_PCI_MODE_MASK 0x00800000
#define M85XX_PORDEVSR_PCI_MODE(base) ((*M85XX_PORDEVSR(base) & M85XX_PORDEVSR_PCI_MODE_MASK)>>23)
#define PORDEVSR_PCIX_MODE 0
#define PORDEVSR_PCI_MODE 1
#define M85XX_PORDEVSR2(base)           (CAST(VUINT32 *)((base) + 0xE0014))

#define M85XX_DDRDLLCR(base)           (CAST(VUINT32 *)((base) + 0xE0E10))
#define M85XX_LBCDLLSR(base)           (CAST(VUINT32 *)((base) + 0xE0E20))

#define M85XX_DEVDISR(base)            (CAST(VUINT32 *)((base) + 0xE0070))
#define M85XX_DEVDISR_DDR  0x00010000
#define M85XX_PVR(base)                (CAST(VUINT32 *)((base) + 0xE00A0))
#define M85XX_SVR(base)                (CAST(VUINT32 *)((base) + 0xE00A4))

#define _PPC_BUCSR_FI 0x200            /* Invalidate branch cache */
#define _PPC_BUCSR_E 0x1               /* Enable branch prediction */

#define M85XX_GPIOCR(base)             (CAST(VUINT32 *)((base) + 0xE0030))
#define M85XX_GPOUTDR(base)            (CAST(VUINT32 *)((base) + 0xE0040))
#define M85XX_GPINDR(base)             (CAST(VUINT32 *)((base) + 0xE0050))

/* MOTETSEC Registers */
#define M85XX_MACCFG1(base)            (CAST(VUINT32 *)((base) + 0x24500))
#define M85XX_MACCFG2(base)            (CAST(VUINT32 *)((base) + 0x24504))
#define M85XX_ECNTRL(base)             (CAST(VUINT32 *)((base) + 0x24020))


#define BCM98548XMC_SYS_RESET           0x1
#define BCM98548XMC_CPU_RESET           0x2

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif /* INCgtoh */
