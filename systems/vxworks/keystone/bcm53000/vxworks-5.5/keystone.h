#ifndef KEYSTONE_H
#define KEYSTONE_H

#ifndef _LANGUAGE_ASSEMBLY
#include "flashFsLib.h"  /* for NV_RAM_SIZE */
#endif

/*******************************************************************************
* Address space for external interface 
*******************************************************************************/

#ifndef KSEG1ADDR
#define KSEG1ADDR(_a) ((unsigned long)(_a) | 0xA0000000)
#endif

#ifndef KSEG1ADDRASM
#define KSEG1ADDRASM(_a) ((_a) | 0xA0000000)
#endif

/* $Id: keystone.h,v 1.4 2011/07/21 16:14:25 yshtil Exp $
* Reset function 
*    Implemented in PLD.  Read or write should trigger hard reset 
*/

/* Using the GPIO pins wired to CPU/board reset to achieve system reboot */
/* BCM953003C using GPIO 11 and 12 */
/* BCM953001R24M using GPIO 7 */
#define SYS_HARD_RESET()   \
    {                                                   \
      char *boardtype = getvar(NULL,"boardtype");       \
      if (boardtype &&                                  \
          !strncmp(boardtype,"bcm953003c",strlen("bcm953003c"))) {\
        *(volatile uint32 *)(0xb8000064) = 0x00001800;  \
        *(volatile uint32 *)(0xb8000068) = 0x0000f800;  \
        *(volatile uint32 *)(0xb8000064) = 0x00000000;  \
      } else {                                          \
        *(volatile uint32 *)(0xb8000064) = 0x00000080;  \
        *(volatile uint32 *)(0xb8000068) = 0x00000080;  \
        *(volatile uint32 *)(0xb8000064) = 0x00000000;  \
      }                                                 \
    }

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
 */
#define SYS_TOD_UNPROTECT()
#define SYS_TOD_PROTECT()
#define NV_RAM_SIZE             FLASH_NVRAM_SIZE
#define NV_RAM_ADRS             (KSEG1ADDR(0x1c080000))
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

#define ET0_PHYADDR 9
#define ET0_MDCPORT 0

#define ET1_PHYADDR 7
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
/* Cover all of the interrupt vectors with pciIntLib, 
   as vxWorks does not support interrupt sharing by default */
#define PCI_IRQ_LINES 25

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
void sysLedDsply(char* msg);
void sysGetClocks(int *core, int *mem, int *sb);


#endif /* !_ASMLANGUAGE */

#define BUS			BUS_TYPE_PCI	/* no off-board bus interface */

#endif /* KEYSTONE_H */

