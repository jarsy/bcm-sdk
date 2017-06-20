/* bcm1250.h - Bcm1250 platform header */

/* Copyright 2005 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: bcm1250.h,v 1.3 2011/07/21 16:14:48 yshtil Exp $
********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

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
01i,28jul05,rlg  spr 104812 - added the KX bit to initial SR
01h,11apr05,kab  fix comments for apiGen (SPR 107842)
01g,03oct02,agf  fix CPU_CLOCK_RATE (SPR 81403)
01f,20jun02,pgh  Move the MIPS3_SD and MIPS3_LD definitions to bcm1250Lib.h.
01e,10may02,tlc  Add C++ header protection.
01d,31jan02,agf  replace __mips64 #ifdef with CPU==MIPS64
01c,20dec01,tlc  Remove UINT64 temporary typedef.
01b,07dec01,agf  add Broadcom copyright notice
01a,15nov01,agf  created.
*/

/*
 * This file contains I/O addresses and related constants for
 * VxWorks on the Broadcom BCM912500 evaluation board.  The
 * BCM1250 User's Manual (UM) contains full descriptions of
 * the following register uses.
 *
 * Note: in new code, definitions from ./bcm1250_*.h
 * should be used for register names and register fields.
 */

#ifndef	__INCbcm1250h
#define	__INCbcm1250h

#ifdef __cplusplus
extern "C" {
#endif

#include "vxWorks.h"

#ifndef MIPS64
#define	MIPS64	R4000
#endif

/* Mask Generation Macro */
#ifndef MSK
#define MSK(n)                    ((1 << (n)) - 1)
#endif

#define BUS		0

#ifdef _SIMULATOR_
#define CPU_CLOCK_RATE 50000  /* much more frequently if simulating */
#else
#define CPU_CLOCK_RATE 50000000
#endif

#define TARGET_BCM912500

/* define macro so drivers will call sysWbFlush() */

#define SYS_WB_FLUSH

/* reset default status register */

#define INITIAL_SR              (SR_CU0 | SR_CU1 | SR_BEV | SR_DE | SR_ERL)

/* task default status register */

/* Interrupts will be masked by the interrupt mapper until explicitly
 * enabled.
 * Run with Status.KX always set. 
 */
#define BCM1250_SR (SR_CU1 | SR_CU0 | SR_FR | SR_KX | SR_IMASK0 | SR_IE)

/* interrupt priority */

#define INT_PRIO_MSB    TRUE            /* interrupt priority msb highest */

/* Serial channels */

#define N_SIO_CHANNELS  3               /* 2 DUARTs and 1 JTAG */


/*
 * The BCM1250 interrupt system distinguishes 64 system interrupt sources
 * and a like number of LDT sources.  The interrupt mapper enables or
 * disables the various sources and also directs enabled sources to one
 * of the traditional MIPS interrupt vectors (IV0-IV5).  Thus attaching
 * an interrupt handler requires both an interrupt source (ILVL below) and
 * MIPS vector number (IV below).  This is non-standard for vxWorks.
 */

/* interrupt vectors */
#define IV_INT0_VEC		60	/* bcm1250 INT 0 */
#define IV_INT1_VEC		61	/* bcm1250 INT 1 */
#define IV_INT2_VEC		62	/* bcm1250 INT 2 */
#define IV_INT3_VEC		63	/* bcm1250 INT 3 */
#define IV_INT4_VEC		64	/* bcm1250 INT 4 */
#define IV_INT5_VEC		65	/* bcm1250 INT 5 */

/* bcm1250 interrupt level indexes */
#define ILVL_BCM1250_WATCHDOG0   0
#define ILVL_BCM1250_WATCHDOG1   1
#define ILVL_BCM1250_TIMER0      2
#define ILVL_BCM1250_TIMER1      3
#define ILVL_BCM1250_TIMER2      4
#define ILVL_BCM1250_TIMER3      5
#define ILVL_BCM1250_SMB0        6
#define ILVL_BCM1250_SMB1        7
#define ILVL_BCM1250_UART0       8
#define ILVL_BCM1250_UART1       9

#define ILVL_BCM1250_MAC0        19
#define ILVL_BCM1250_MAC1        20
#define ILVL_BCM1250_MAC2        21

#define ILVL_BCM1250_MBOX_INT_0	26
#define ILVL_BCM1250_MBOX_INT_1	27
#define ILVL_BCM1250_MBOX_INT_2	28
#define ILVL_BCM1250_MBOX_INT_3	29

#define ILVL_BCM1250_PCI_INTA    56
#define ILVL_BCM1250_PCI_INTB    57
#define ILVL_BCM1250_PCI_INTC    58
#define ILVL_BCM1250_PCI_INTD    59

#ifndef _ASMLANGUAGE
#ifdef __mips64

#define MIPS3_SD(addr, value) \
    (* (volatile unsigned long long *) (addr) = value)
#define MIPS3_LD(addr) \
    (*(volatile unsigned long long *) (addr))

#else

void mips3_sd( volatile unsigned long long *, unsigned long long );
unsigned long long mips3_ld( volatile unsigned long long * );
#define MIPS3_SD(addr, value) \
    mips3_sd((volatile unsigned long long *) (addr), (value))
#define MIPS3_LD(addr) \
    mips3_ld((volatile unsigned long long *) (addr))

#endif 

/* Hyperspace (64-bit virtual address) access routines */

#define MIPS_PHYS_TO_XKPHYS(cca,p)                                      \
    (((UINT64)1 << 63) | ((UINT64)(cca) << 59) | p)
#define MIPS_PHYS_TO_XKSEG_UNCACHED(p)	MIPS_PHYS_TO_XKPHYS(2,(p))
#define MIPS_PHYS_TO_XKSEG_CACHED(p)	MIPS_PHYS_TO_XKPHYS(5,(p))

UINT8  hs_read8 (UINT64 addr);
void   hs_write8 (UINT64 addr, UINT8 value);
UINT16 hs_read16 (UINT64 addr);
void   hs_write16 (UINT64 addr, UINT16 value);
UINT32 hs_read32 (UINT64 addr);
void   hs_write32 (UINT64 addr, UINT32 value);
UINT64 hs_read64 (UINT64 addr);
void   hs_write64 (UINT64 addr, UINT64 value);

#endif /* _ASMLANGUAGE */

/* misc hardware defines */

#define MAX_PHYS_MEMORY		0x4000000		/* 64 meg */
#define MIN_PHYS_MEMORY		0x0400000		/*  4 meg */

/* Miscellaneous */

#undef	TLB_ENTRIES
#define	TLB_ENTRIES	64

/* CP0_CONFIG masks */
#define CFG_K0_MASK        0x07
#define CFG_K0_EXCL_NOL2   0
#define CFG_K0_SHARED_NOL2 1
#define CFG_K0_UNCACHED    2
#define CFG_K0_CACHEABLE   3
#define CFG_K0_EXCL_COH    4
#define CFG_K0_COHERENT    5
#define CFG_K0_UNUSED      6
#define CFG_K0_ACCEL       7

/* CP0_CONFIG1 masks */
#define SB1_CONFIG1_DA_SHF            7
#define SB1_CONFIG1_DA_MSK            (MSK(3) << SB1_CONFIG1_DA_SHF)
#define SB1_CONFIG1_DL_SHF            10
#define SB1_CONFIG1_DL_MSK            (MSK(3) << SB1_CONFIG1_DL_SHF)
#define SB1_CONFIG1_DS_SHF            13
#define SB1_CONFIG1_DS_MSK            (MSK(3) << SB1_CONFIG1_DS_SHF)

#define SB1_CONFIG1_IA_SHF            16
#define SB1_CONFIG1_IA_MSK            (MSK(3) << SB1_CONFIG1_IA_SHF)
#define SB1_CONFIG1_IL_SHF            19
#define SB1_CONFIG1_IL_MSK            (MSK(3) << SB1_CONFIG1_IL_SHF)
#define SB1_CONFIG1_IS_SHF            22
#define SB1_CONFIG1_IS_MSK            (MSK(3) << SB1_CONFIG1_IS_SHF)


/* PCI and LDT definitions */

/*
 * Mapped memory and I/O space as seen by the SB-1250.
 * See UM, Figure 8-2.
 */
#define PCI_MEM_ADRS      0x41000000
#define PCI_MEM_SIZE      0x1F000000
#define PCI_IO_ADRS       0xDC000000
#define PCI_ISA_IO_SIZE   0x00008000
#define PCI_IO_SIZE       0x02000000

#define PCI_CONF_ADRS     0xDE000000
#define PCI_CONF_SIZE     0x01000000

/* Add the following to the above for "match bit lane" mode */
#define BIT_ENDIAN_OFFSET 0x20000000

#define PCI_LAT_TIMER     255


/*
 * These are dummy definitions for PCI access mechanisms 1 and 2.
 * BCM1250 accesses use mechanism 0 only.  Thus the following should
 * never be used, but they are required to avoid compile-time
 * undefined symbols.
 */

#define PCI_IN_BYTE(x)     (0)
#define PCI_IN_WORD(x)     (0)
#define PCI_IN_LONG(x)     (0)

#define PCI_OUT_BYTE(x,y)  ((void)0)
#define PCI_OUT_WORD(x,y)  ((void)0)
#define PCI_OUT_LONG(x,y)  ((void)0)


/*
 * There are 4 PCI interrupt lines (A-D) starting at PCI INTA.
 */

#define PCI_IRQ_LINES (ILVL_BCM1250_PCI_INTD - ILVL_BCM1250_PCI_INTA + 1)
#define INT_NUM_IRQ0  (ILVL_BCM1250_PCI_INTA)

/*
 * Map the PCI interrupt lines to MIPS interrupt INT0 (IP[2])
 */
#define PCI_INT_HANDLER_BIND(vector, routine, param, pResult)          \
    {                                                                  \
    IMPORT STATUS bcm1250IntConnect(int, int, VOIDFUNCPTR, int);        \
    IMPORT STATUS bcm1250IntEnable(int);                                \
    *pResult = bcm1250IntConnect ( IVEC_TO_INUM(vector), IV_INT0_VEC,   \
                                  (VOIDFUNCPTR)(routine),              \
                                  (int)(param) );                      \
    if (*pResult == OK)                                                \
        *pResult = bcm1250IntEnable( IVEC_TO_INUM(vector));             \
    }

#ifdef __cplusplus
}
#endif
#endif	/* __INCbcm1250h */
