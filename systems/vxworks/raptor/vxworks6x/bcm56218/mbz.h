/* Copyright 2002 Broadcom Corporation */


#ifndef MBZ_H
#define MBZ_H

#include "bcm56218.h"

/*******************************************************************************
* Address space for external interface 
*******************************************************************************/

#ifndef KSEG1ADDR
#define KSEG1ADDR(_a) ((unsigned long)(_a) | 0xA0000000)
#endif

#ifndef KSEG1ADDRASM
#define KSEG1ADDRASM(_a) ((_a) | 0xA0000000)
#endif

/* $Id: mbz.h,v 1.5 2011/07/21 16:14:58 yshtil Exp $
* Reset function 
*/
#define SYS_HARD_RESET()   \
    { { *(volatile uint32 *)(ICS_SRESET_REG) = 0;   \
          *(volatile uint32 *)(ICS_SRESET_REG) = 1; \
            while(1);                               \
      }}

/* 
* SERIAL PORT 
*    There is one serial connector on the back of the board
*    The second port is on the board.  It's generally not used
*/

#define N_UART_CHANNELS      2
#define COM1_ADR_INTERVAL    1
#define COM2_ADR_INTERVAL    1

#ifdef QUICK_TURN
/* div = 15 for 585k db */
#define UART_REF_CLK_FREQ   2304000 /* (15 * CONSOLE_BAUD_RATE * 16) */
#else
#define UART_REF_CLK_FREQ   get_sb_clock()
#endif

#define COM1_BASE_ADR        INTERNAL_UART_COM1
#define COM2_BASE_ADR        INTERNAL_UART_COM2

#define COM1_INT_LVL         INT_LVL_IORQ1
#define COM1_INT_VEC         INT_VEC_IORQ1

#define COM2_INT_LVL         INT_LVL_IORQ2
#define COM2_INT_VEC         INT_VEC_IORQ2


/*
 *  NVRAM configuration
 */
#define NV_RAM_SIZE             0x8000
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

/* Defines for pciIntLib */
#define INT_NUM_IRQ0 65  /* IV_IORQ0_VEC */

/*
* This macro selects appropriate debug print routine when
* bootrom build with -DBRINGUP option.  Usually it prints
* 4-byte strings to either UART or serial port.
*/
#if 1
#define BPRINT(str)                                 \
        ((bringupPrintRtn == NULL) ? OK :           \
         (bringupPrintRtn) ((str) ))
#else
#define BPRINT(str)  
#endif

#ifdef MIPSEB
#define LED_DIGIT(n)  ((n)^3)
#else
#define LED_DIGIT(n)  (n)
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
extern int sysTodGetSecond (void);
extern STATUS sysTodGet (int *pYear, int *pMonth, int *pDay, int *pHour,
    int *pMin, int *pSec);
extern STATUS sysTodSet (int year, int month, int day, int hour, int min, int sec);
extern STATUS sysTodInit (UINT8 *addr);
extern STATUS sysTodWatchdogArm (int usec);
extern void sysLedDsply(const char* msg);

extern int sysIsFlashProm(void);
extern unsigned int sysGetSocRevId(void);
extern unsigned int sysGetSocDevId(void);
extern void sys_timestamp_get(UINT32 *up, UINT32 *low, UINT32 *freq);

#endif /* !_ASMLANGUAGE */

#endif /* MBZ_H */

