/* Copyright 2002 Broadcom Corporation */

/* $Id: mbz.h,v 1.14 2011/07/21 16:14:21 yshtil Exp $
modification history
--------------------
01c,21oct02,jmb  new TOD support, board IDs, bringup printing
01b,06sep02,jmb  added defined for HINT bridge and pciIntLib
01a,11aug02,jmb  Written
*/

#ifndef MBZ_H
#define MBZ_H


#include "bcm4704.h"

/*******************************************************************************
* Address space for external interface 
*******************************************************************************/

#ifndef KSEG1ADDR
#define KSEG1ADDR(_a) ((unsigned long)(_a) | 0xA0000000)
#endif

#ifndef KSEG1ADDRASM
#define KSEG1ADDRASM(_a) ((_a) | 0xA0000000)
#endif

#define EXTIF_BASE_ADDR          (KSEG1ADDR(BCM4704_EXTIF))
#define EXTIF_BASE_ADDR_ASM      (KSEG1ADDRASM(BCM4704_EXTIF))

/* Asynchronous interface (ISA Bus) */
#define AUX_IF_ADDR         0x1b000000

/* PCMCIA interface.  Used as an asynchronous interface, not PCMCIA */
#define    PCMCIAIF_BASE_ADDR   EXTIF_BASE_ADDR
#define    PCMCIAIF_BASE_ADDR_ASM   EXTIF_BASE_ADDR_ASM
#define    MBZ_RESET_ADDR    PCMCIAIF_BASE_ADDR
#define    MBZ_PLDREG_BASE   (PCMCIAIF_BASE_ADDR | 0x2000)
#define    MBZ_BOARDID_ADDR  (PCMCIAIF_BASE_ADDR | 0x4000)
#define    MBZ_BOARDID_ADDR_ASM  (PCMCIAIF_BASE_ADDR_ASM | 0x4000)
#define    MBZ_DOC_ADDR      (PCMCIAIF_BASE_ADDR | 0x6000)
#define    MBZ_LED_ADDR      (PCMCIAIF_BASE_ADDR | 0xc000)
#define    MBZ_LED_ADDR_ASM  (PCMCIAIF_BASE_ADDR_ASM | 0xc000)
#define    MBZ_NVRAM_ADDR    (PCMCIAIF_BASE_ADDR | 0xe000)

#define MBZ_DUART_CHAN0_ASM 0xb8000300
#define MBZ_DUART_CHAN1_ASM 0xb8000400

/* Byte swapped when running big-endian */
#ifdef  MIPSEL
#define SYS_PLD_READ(addr)   (* (volatile UINT8 *) ((addr)))
#else
#define SYS_PLD_READ(addr)   (* (volatile UINT8 *) ((addr)^3))
#endif

/* Board ID etc. */
#define SYS_REVID_GET()      SYS_PLD_READ(MBZ_BOARDID_ADDR_ASM)
#define SYS_SLOTID_GET()     SYS_PLD_READ(MBZ_PLDREG_BASE + 0x1)
#define SYS_ALTCONS_GET()    SYS_PLD_READ(MBZ_PLDREG_BASE + 0x7)

#define BOARD_ID_MBZ_1 0xfe /* CPCI with 1Mx8 AMD 29LV160B */ 
#define BOARD_ID_MBZ_2 0x80 /* CPCI with 1Mx16 Intel 28F320 */
#define BOARD_ID_LM_1  0x0e /* Line module: 1XBCM5671 + 2XBCM5690 */
#define BOARD_ID_LM_1_32MB  0x0e /*Line module: 1XBCM5671 + 2XBCM5690 32MB RAM*/
#define BOARD_ID_MBZ_5645_REF 0x7f /* BCM5645 with Turbo Stacking Serdes */
#define BOARD_ID_JAG_CPCI  0x11 /* 4704 CPCI */
#define BOARD_ID_JAG_CPCI2  0x15 /* 4704 CPCI */
#define BOARD_ID_LM_P6_CX4  0x12 /* Line module: 1XBCM5670 + 6XBCM5673 */
#define BOARD_ID_LM_P48  0x13   /* Line module: 1XBCM5675 + 4XBCM5695 */
#define BOARD_ID_LM_P6_CX4V2  0x16   /* Line module: 1XBCM5675 + 6XBCM5674 */
#define BOARD_ID_LM_FB_P48  0x17   /* Line module: 2XBCM56504 */
#define BOARD_ID_JAG_FB_MINI  0x19   /* 1xBCM56504 */
#define BOARD_ID_JAG_BCM56218_EB  0x1c   /* 1xBCM56218 in EM mode */

/*******************************************************************************/

/* 
* Reset function 
*    Implemented in PLD.  Read or write should trigger hard reset 
*/

#define SYS_HARD_RESET()   \
    { for (;;) { *(volatile unsigned char *)(MBZ_RESET_ADDR) = 0x80; \
                 *(volatile unsigned char *)(MBZ_RESET_ADDR ^ 3) = 0x80; \
               }}

/* 
* SERIAL PORT 
*    There is one serial connector on the back of the board
*    The second port is on the board.  It's generally not used
*/

#define N_UART_CHANNELS      2
#define COM1_ADR_INTERVAL    1
#define COM2_ADR_INTERVAL    1
/* #define COM1_FREQ            1851851*/
#define COM1_FREQ            1800000
#define COM2_FREQ            COM1_FREQ
#define COM1_BASE_ADR        0xb8000300
#define COM2_BASE_ADR        0xb8000400

#define COM1_INT_VEC         IV_IORQ0_VEC
#define COM1_INT_LVL         INT_LVL_IORQ0

/* Fixme!  COM2 interrupt stuff not right */
#define COM2_INT_VEC         IV_IORQ0_VEC
#define COM2_INT_LVL         INT_LVL_IORQ0


/*
 *  NVRAM configuration
 *  NVRAM is implemented via the SGS Thomson M48T59Y
 *  64Kbit (8Kbx8) Timekeeper SRAM.
 *  This 8KB NVRAM also has a TOD. See m48t59y.{h,c} for more information.
 */
#define SYS_TOD_UNPROTECT()
#define SYS_TOD_PROTECT()
#define NV_RAM_SIZE             8176
#define NV_RAM_ADRS             MBZ_NVRAM_ADDR
#define NV_RAM_INTRVL           1
#define NV_RAM_WR_ENBL          SYS_TOD_UNPROTECT()
#define NV_RAM_WR_DSBL          SYS_TOD_PROTECT()
#define NV_OFF_BOOT0            0x0000  /* Boot string 0 (256b) */
#define NV_OFF_BOOT1            0x0100  /* Boot string 1 (256b) */
#define NV_OFF_BOOT2            0x0200  /* Boot string 2 (256b)*/
#define NV_OFF_MACADDR          0x0400  /* 21143 MAC address (6b) */
#define NV_OFF_ACTIVEBOOT       0x0406  /* Active boot string, 0 to 2 (1b) */
#define NV_OFF_UNUSED1          0x0407  /* Unused (1b) */
#define NV_OFF_BINDFIX          0x0408  /* See sysLib.c:sysBindFix() (1b) */
#define NV_OFF_UNUSED2          0x0409  /* Unused (7b) */
#define NV_OFF_TIMEZONE         0x0410  /* TIMEZONE env var (64b) */
#define NV_OFF__next_free       0x0450
#if 0
#define TOD_REG_BASE            (NV_RAM_ADRS | 0x1ff0)
#define SYS_TOD_PROTECT()
#define SYS_TOD_UNPROTECT()
#endif

/* LEDS */

/* Ethernet port parameters */

#define ET0_PHYADDR 2
#define ET0_MDCPORT 0

#define ET1_PHYADDR 1
#define ET1_MDCPORT 1


/* PCI */

/* Generic PCI-PCI (P2P) Bridge configruation parameters */
#define P2P_CLR_STATUS           0xFFFF0000
#define P2P_SEC_BUS_RESET        (0x0040 << 16)
#define P2P_CLK_ENABLE           0x00       /* enable clocks on all slots */
#define P2P_PMC_DISABLE          0
#define P2P_PMC_ENABLE           6

/* MPC8240 Architecture specific settings for PCI-PCI Bridge support */
/* See MPC8240UM Address Map B */
#define P2P_NONPREF_MEM_BASE     0xa8100000  /* PCI non-prefetch mem window */
#define P2P_NONPREF_MEM_SIZE     0x02000000  /* PCI non-prefetch size */
#define P2P_CACHE_LINE_SIZE      8           /* cache line size */
#define P2P_PRIM_LATENCY         0           /* latency */

#define DC21150_VENDOR_ID    0x1011
#define DC21150_DEVICE_ID    0x0022

/* HINT (R) HB4 PCI-PCI Bridge (21150 clone) */
#define HINT_HB4_VENDOR_ID    0x3388
#define HINT_HB4_DEVICE_ID    0x0022

/* Pericom PCI-PCI Bridge (21150 clone) */
#define PERICOM_VENDOR_ID    0x12D8
#define PERICOM_8150_DEV_ID  0x8150

#define PCI_CFG_DEC21150_SEC_CLK   0x68     /* secondary clock control reg */
#define PCI_CFG_DEC21150_SERR_STAT 0x6A

/* Defines for pciIntLib */
#define INT_NUM_IRQ0 65  /* IV_IORQ0_VEC */
#define PCI_IRQ_LINES 5  /* really only 1 line, but it's IRQ4 */

/*
* This macro selects appropriate debug print routine when
* bootrom build with -DBRINGUP option.  Usually it prints
* 4-byte strings to either UART or serial port.
*/
#if 1
#define DEBUG_H sysSerialPrintHex(__LINE__, 0); sysSerialPrintString(__FILE__)
#define BPRINT(str)  \
        ((bringupPrintRtn == NULL) ? OK :        \
         (bringupPrintRtn) ((str) ))
#else
#define BPRINT(str)  
#endif


#ifndef _ASMLANGUAGE

/*
 * MBZ-related routines
 */

extern VOIDFUNCPTR      bringupPrintRtn;                /* sysLib.c */
extern void     sysSerialPutc(int c);                   
extern int      sysSerialGetc(void);                   
extern void     sysSerialPrintString(char *s);        
extern void     sysSerialPrintStringNL(char *s);     
extern void     sysSerialPrintHex(UINT32 val, int cr); 
extern void     sysLedDisply(unsigned char *a);      
extern void     sys47xxClocks(int *core, int *sb, int *pci);
int sysTodGetSecond (void);
STATUS sysTodGet (int *pYear, int *pMonth, int *pDay, int *pHour,
    int *pMin, int *pSec);
STATUS sysTodSet (int year, int month, int day, int hour, int min, int sec);
STATUS sysTodInit (UINT8 *addr);
STATUS sysTodWatchdogArm (int usec);
void sysLedDsply(const char* msg);


#endif /* !_ASMLANGUAGE */

#endif /* MBZ_H */

