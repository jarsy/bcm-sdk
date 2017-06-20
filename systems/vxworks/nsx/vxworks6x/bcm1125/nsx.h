/* $Id: nsx.h,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * SiByte Board constants and defines
 *
 * Copyright 2004 Broadcom Corp.
 */

#ifndef _NSX_H_
#define _NSX_H_

#if 0
#define SYS_LED_ON()  (*((volatile UINT64 *) 0xb0061ab0) = 1)
#define SYS_LED_OFF() (*((volatile UINT64 *) 0xb0061ab8) = 1)
#endif

#define SYS_LED_ON()  (*((volatile UINT8 *) 0xb0061ab7) = 1)
#define SYS_LED_OFF() (*((volatile UINT8 *) 0xb0061abf) = 1)

#define NSX_LED_REG_BASE 0xbd0a0000
#define NSX_LED_REG(x) (*(volatile UINT8 *) (NSX_LED_REG_BASE + 8 * x))

/*
 * Set M_SYS_SYSTEM_RESET bit in SCD System Config register triggers reboot
 */
#define SYS_HARD_RESET() \
    { \
        UINT64 data; \
        data = SBREADCSR(A_SCD_SYSTEM_CFG) & ~M_SYS_SB_SOFTRES; \
        data |= M_SYS_SYSTEM_RESET; \
        SBWRITECSR(A_SCD_SYSTEM_CFG, data); \
    }

/* PCI-PCI support */

/* Generic PCI-PCI (P2P) Bridge configruation parameters */
#define P2P_CLR_STATUS           0xFFFF0000
#define P2P_SEC_BUS_RESET        (0x0040 << 16)
#define P2P_CLK_ENABLE           0x00       /* enable clocks on all slots */
#define P2P_PMC_DISABLE          0
#define P2P_PMC_ENABLE           6

/* DEC 21150 Specific Registers and Values */
#define DC21150_VENDOR_ID    0x1011
#define DC21150_DEVICE_ID    0x0022
#define PCI_CFG_DEC21150_CHIP_CTRL 0x40
#define PCI_CFG_DEC21150_DIAG_CTRL 0x41
#define PCI_CFG_DEC21150_ARB_CTRL  0x42
#define PCI_CFG_DEC21150_EVNT_DSBL 0x64
#define PCI_CFG_DEC21150_GPIO_DOUT 0x65
#define PCI_CFG_DEC21150_GPIO_CTRL 0x66
#define PCI_CFG_DEC21150_GPIO_DIN  0x67
#define PCI_CFG_DEC21150_SEC_CLK   0x68     /* secondary clock control reg */
#define PCI_CFG_DEC21150_SERR_STAT 0x6A

/* HINT (R) HB4 PCI-PCI Bridge (21150 clone) */
#define HINT_HB4_VENDOR_ID    0x3388
#define HINT_HB4_DEVICE_ID    0x0022

/* Pericom PCI-PCI Bridge (21150 clone) */
#define PERICOM_VENDOR_ID 0x12D8
#define PERICOM_DEVICE_ID 0x8150

#define PCI_CFG_DEC21150_SEC_CLK   0x68     /* secondary clock control reg */
#define PCI_CFG_DEC21150_SERR_STAT 0x6A

/* BCM1250 Architecture specific settings for PCI-PCI Bridge support */
#define P2P_NONPREF_MEM_BASE 0x41000000     /* PCI non-prefetch mem window */
#define P2P_NONPREF_MEM_SIZE 0x02000000     /* PCI non-prefetch size */
#define P2P_CACHE_LINE_SIZE  8              /* cache line size */
#define P2P_PRIM_LATENCY     0              /* latency */

/*
 * SiByte-related routines
 */

extern void  sysSerialPutc(int c);                   /* sysSerial.c */
extern int   sysSerialGetc(void);                    /* sysSerial.c */
extern void  sysSerialPrintString(char *s);          /* sysSerial.c */
extern void  sysSerialPrintHex(UINT32 val, int cr);  /* sysSerial.c */

int    sysTodGetSecond (void);
STATUS sysTodGet (int *pYr, int *pMon, int *pDay, int *pHr, int *pMn, int *pSec);
STATUS sysTodSet (int yr, int month, int day, int hour, int min, int sec);
STATUS sysTodInit (void);

#endif /* _NSX_H_ */

