/* sysLib.c - generic PPC  system-dependent library 
 * Copyright 1984-1997 Wind River Systems, Inc. 
 * Copyright 1996,1999 Motorola, Inc. 


modification history
--------------------
01n,25oct02,jmb Some test code for CFM
01m,09oct02,jmb More interrupt types for CFM
01l,09oct02,jmb Preliminary interrupt handling code for chassis system
01k,28jun02,jmb Routines to help sal determine CPU and PCI clk speeds.
01j,28jun02,jmb Recognize two PLL jumper settings as special cases, indicating
                that the PCI clock is 50 MHZ. (WhiteKnight)
01i,13jun02,jmb map RCS3 for second DOC and add a stub to get rid of flash*.
01h,07jun02,jmb restore sysToMonitor hooks and add a hack to make ctrl-\ work
01g,29may02,jmb add componentized include of sysBcm570xEnd.c
01f,27may02,jmb add routine to fetch ethernet address from NVRAM
01e,08may02,jmb add microsecond delay routine
01d,01may02,jmb add routine to figure out ram clock speed from PLL
01c,30apr02,jmb Get rid of dependence on L2 cache code
01b,30apr02,jmb Get rid of dependence on DEC ethernet driver and
                expand ethernet address space for BCM 5701
01a,22apr02,jmb Ported from mousse to bmw
*/

/* $Id: sysLib.c,v 1.13 2011/07/21 16:14:08 yshtil Exp $
DESCRIPTION
This library provides board-specific routines.

INCLUDE FILES: sysLib.h

SEE ALSO:
.pG "Configuration"
*/

/* includes */

#include "stdio.h"
#include "ctype.h"
#include "vxWorks.h"
#include "memLib.h"
#include "cacheLib.h"
#include "sysLib.h"
#include "config.h"
#include "string.h"
#include "time.h"
#include "intLib.h"
#include "logLib.h"
#include "taskLib.h"
#include "vxLib.h"
#include "tyLib.h"
#include "sysSymTbl.h"
#include "symLib.h"
#include "rebootLib.h"
#include "arch/ppc/mmu603Lib.h"
#include "arch/ppc/vxPpcLib.h"
#include "private/vmLibP.h"
#include "netinet/in.h"		/* for sysBindFix() */
#include "sockLib.h"		/* for sysBindFix() */
#include "mpc107.h"
#include "bmw.h"
#include "ds1743.h"				/* NVRAM/TOD support */
#include "m48t59y.h"				/* NVRAM/TOD support */
/* #include "mpc824x_i2c.h" */

/* prototype   */

extern ULONG sysPVRReadSys();
ULONG sys107RegRead( ULONG regNum );
void  sys107RegWrite( ULONG regNum, ULONG regVal );
ULONG sysEUMBBARRead(ULONG regNum);
void  sysEUMBBARWrite(ULONG regNum, ULONG regVal);

/* defines */

#define ZERO	0

#define SYS_MODEL	"BMW"

/* PCI Slot information */

/* Macro to swap a 16 bit value from big to little endian 
 * and vice versa  
 */

#define BYTE_SWAP_16_BIT(x)    ( (((x) & 0x00ff) << 8) | ( (x) >> 8) )

#ifndef SYNC
# define SYNC  __asm__(" sync")
#endif  /* SYNC */ 

/* globals */
int sysRamClkGet();
int sysRamClk = 0;

/*
 * sysBatDesc[] is used to initialize the block address translation (BAT)
 * registers within the PowerPC 603/604 MMU.  BAT hits take precedence
 * over Page Table Entry (PTE) hits and are faster.  Overlap of memory
 * coverage by BATs and PTEs is permitted in cases where either the IBATs
 * or the DBATs do not provide the necessary mapping (PTEs apply to both
 * instruction AND data space, without distinction).
 *
 * The primary means of memory control for VxWorks is the MMU PTE support
 * provided by vmLib and cacheLib.  Use of BAT registers will conflict
 * with vmLib support.  User's may use BAT registers for i/o mapping and
 * other purposes but are cautioned that conflicts with cacheing and mapping
 * through vmLib may arise.  Be aware that memory spaces mapped through a BAT
 * are not mapped by a PTE and any vmLib() or cacheLib() operations on such
 * areas will not be effective, nor will they report any error conditions.
 *
 * Note: BAT registers CANNOT be disabled - they are always active.
 * For example, setting them all to zero will yield four identical data
 * and instruction memory spaces starting at local address zero, each 128KB
 * in size, and each set as write-back and cache-enabled.  Hence, the BAT regs
 * MUST be configured carefully.
 *
 * With this in mind, it is recommended that the BAT registers be used
 * to map LARGE memory areas external to the processor if possible.
 * If not possible, map sections of high RAM and/or PROM space where
 * fine grained control of memory access is not needed.  This has the
 * beneficial effects of reducing PTE table size (8 bytes per 4k page)
 * and increasing the speed of access to the largest possible memory space.
 * Use the PTE table only for memory which needs fine grained (4KB pages)
 * control or which is too small to be mapped by the BAT regs.
 *
 * The BAT configuration for 4xx/6xx-based PPC boards is as follows:
 * All BATs point to PROM/FLASH memory so that end customer may configure
 * them as required.
 *
 * [Ref: chapter 7, PowerPC Microprocessor Family: The Programming Environments]
 */

#define KAHLUA_UNUSED_MEM	0x40000000

UINT32 sysBatDesc [2 * (_MMU_NUM_IBAT + _MMU_NUM_DBAT)] =
    {
    /* I BAT 0 */
    ((KAHLUA_UNUSED_MEM & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_1M |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((KAHLUA_UNUSED_MEM & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT),

    /* I BAT 1 */
    ((KAHLUA_UNUSED_MEM & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_1M |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((KAHLUA_UNUSED_MEM & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT),

    /* I BAT 2 */
    ((KAHLUA_UNUSED_MEM & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_1M |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((KAHLUA_UNUSED_MEM & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT),

    /* I BAT 3 */
    ((KAHLUA_UNUSED_MEM & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_1M |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((KAHLUA_UNUSED_MEM & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT),

    /* D BAT 0 */
    ((PCI_SPECIAL_BASE & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_16M |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((PCI_SPECIAL_BASE & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT | _MMU_LBAT_GUARDED | _MMU_LBAT_MEM_COHERENT),

    /* D BAT 1 */
    ((FLASH1_START & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_16M |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((FLASH1_START & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT | _MMU_LBAT_GUARDED | _MMU_LBAT_MEM_COHERENT),

    /* D BAT 2 */
    ((XROM_BASE_ADDR & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_128K |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((XROM_BASE_ADDR & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT | _MMU_LBAT_GUARDED | _MMU_LBAT_MEM_COHERENT),

    /* D BAT 3 */
    ((FLASH2_START & _MMU_UBAT_BEPI_MASK) | _MMU_UBAT_BL_32M |
    _MMU_UBAT_VS | _MMU_UBAT_VP),
    ((FLASH2_START & _MMU_LBAT_BRPN_MASK) | _MMU_LBAT_PP_RW |
    _MMU_LBAT_CACHE_INHIBIT | _MMU_LBAT_GUARDED | _MMU_LBAT_MEM_COHERENT),
    };


/*
 * sysPhysMemDesc[] is used to initialize the Page Table Entry (PTE) array
 * used by the MMU to translate addresses with single page (4k) granularity.
 * PTE memory space should not, in general, overlap BAT memory space but
 * may be allowed if only Data or Instruction access is mapped via BAT.
 *
 * All entries in this table both addresses and lengths must be page aligned.
 *
 * PTEs are held in a Page Table.  Page table sizes are integer powers
 * of two based on amount of memory to be mapped and a minimum size of
 * 64 kbytes.  The MINIMUM recommended page table sizes for 32-bit
 * PowerPCs are:
 *
 * Total mapped memory		Page Table size
 * -------------------		---------------
 *        8 Meg			     64 K
 *       16 Meg			    128 K
 *       32 Meg			    256 K
 *       64 Meg			    512 K
 *      128 Meg			      1 Meg
 * 	...				...
 *
 * [Ref: chapter 7, PowerPC Microprocessor Family: The Programming Env.]
 */

#define PCI_STATE_MASK		(VM_STATE_MASK_VALID | \
				 VM_STATE_MASK_WRITABLE | \
				 VM_STATE_MASK_CACHEABLE | \
				 VM_STATE_MASK_MEM_COHERENCY | \
				 VM_STATE_MASK_GUARDED)

#define PCI_STATE_VAL		(VM_STATE_VALID | \
				 VM_STATE_WRITABLE | \
				 VM_STATE_CACHEABLE_NOT | \
				 VM_STATE_MEM_COHERENCY | \
				 VM_STATE_GUARDED)

/*
 * Note: 0xfe000000 through the end of memory (includes all of port X space)
 *       are covered by two IBATs.  See sysBatDesc[] above.
 */

PHYS_MEM_DESC sysPhysMemDesc [] =
    {

    /* Vector Table and Interrupt Stack */
    {
    (void *) LOCAL_MEM_LOCAL_ADRS,
    (void *) LOCAL_MEM_LOCAL_ADRS,
    RAM_LOW_ADRS,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },

    {

    /* Local DRAM - MUST be second entry in table, see sysHwInit
       sysPhysMemTop() will update this entry if the macro
       LOCAL_MEM_AUTOSIZE is defined */
    (void *) RAM_LOW_ADRS,
    (void *) RAM_LOW_ADRS,
    LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE -  RAM_LOW_ADRS, 
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE
    },

    {
    /* Embedded Utilities Memory Block */
    (void *) EUMBBAR_VAL,
    (void *) EUMBBAR_VAL,
    EUMBSIZE,
    PCI_STATE_MASK, PCI_STATE_VAL,
    },

    /* Network device */
    {
    (void *) PCI_ENET_MEMADDR,
    (void *) PCI_ENET_MEMADDR,
    0x20000,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 0 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(8),
    (void *) PCI_SOC_MBAR_PRI_BUS(8),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 1 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(1),
    (void *) PCI_SOC_MBAR_PRI_BUS(1),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 2 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(2),
    (void *) PCI_SOC_MBAR_PRI_BUS(2),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 3 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(3),
    (void *) PCI_SOC_MBAR_PRI_BUS(3),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 4 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(4),
    (void *) PCI_SOC_MBAR_PRI_BUS(4),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 5 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(5),
    (void *) PCI_SOC_MBAR_PRI_BUS(5),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 6 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(6),
    (void *) PCI_SOC_MBAR_PRI_BUS(6),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 7 */
    {
    (void *) PCI_SOC_MBAR_PRI_BUS(7),
    (void *) PCI_SOC_MBAR_PRI_BUS(7),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 0 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(0),
    (void *) PCI_SOC_MBAR_SEC_BUS(0),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 1 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(1),
    (void *) PCI_SOC_MBAR_SEC_BUS(1),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 2 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(2),
    (void *) PCI_SOC_MBAR_SEC_BUS(2),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 3 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(3),
    (void *) PCI_SOC_MBAR_SEC_BUS(3),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 4 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(4),
    (void *) PCI_SOC_MBAR_SEC_BUS(4),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 5 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(5),
    (void *) PCI_SOC_MBAR_SEC_BUS(5),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 6 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(6),
    (void *) PCI_SOC_MBAR_SEC_BUS(6),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* BCM56xx Unit 7 */
    {
    (void *) PCI_SOC_MBAR_SEC_BUS(7),
    (void *) PCI_SOC_MBAR_SEC_BUS(7),
    PCI_SOC_MEM_WINSZ,
    PCI_STATE_MASK, PCI_STATE_VAL
    },

    /* PCI-PCI Bridge space */
    {
    (void *) P2P_PREF_MEM_BASE,
    (void *) P2P_PREF_MEM_BASE,
    0x8000,
    PCI_STATE_MASK, PCI_STATE_VAL
    },
    
    };

int sysPhysMemDescNumEnt = NELEMENTS (sysPhysMemDesc);

int   sysCpu      = CPU;                /* system CPU type (MC680x0) */
char *sysBootLine = BOOT_LINE_ADRS;	/* address of boot line */
char *sysExcMsg   = EXC_MSG_ADRS;	/* catastrophic message area */
int   sysProcNum = 0;			/* processor number of this CPU */
int   sysFlags;				/* boot flags */
char  sysBootHost [BOOT_FIELD_LEN];	/* name of host from which we booted */
char  sysBootFile [BOOT_FIELD_LEN];	/* name of file from which we booted */
int   sysVectorIRQ0 = INT_VEC_IRQ0;

unsigned long  sysPciConfAddr;
unsigned long  sysPciConfData;
VOIDFUNCPTR bringupPrintRtn = NULL;

#define P33MHZ  33000000
#define P50MHZ  50000000
#define P66MHZ  66000000

typedef struct _pllmult 
    {
    unsigned int pcifreq;
    unsigned int pciToMem;
    unsigned int memToCpu;
    } PLLMULT;

PLLMULT pllMult [] = {
    {P33MHZ, 6, 5},     /* 0x00 - 00000b - 0  */
    {P33MHZ, 6, 6},     /* 0x01 - 00001b - 1  */
    {P33MHZ, 2, 9},     /* 0x02 - 00010b - 2  */
    {P33MHZ, 2, 4},     /* 0x03 - 00011b - 3  */
    {P33MHZ, 4,4},      /* 0x04 - 00100b - 4  */
    {P33MHZ, 2, 5},     /* 0x05 - 00101b - 5  */
    {P33MHZ, -1, -1},   /* 0x06 - 00110b - 6  */
    {P33MHZ, 2,6},      /* 0x07 - 00111b - 7  */
    {P33MHZ, 2, 6},     /* 0x08 - 01000b - 8  */
    {P66MHZ, 4, 4},     /* 0x09 - 01001b - 9  */
    {P33MHZ, 4, 9},     /* 0x0a - 01010b - 10 */
    {P33MHZ, 3, 6},     /* 0x0b - 01011b - 11 */
    {P33MHZ, 4,5},      /* 0x0c - 01100b - 12 */
    {P33MHZ, 3, 7},     /* 0x0d - 01101b - 13 */
    {P33MHZ, 4, 6},     /* 0x0e - 01110b - 14 */
    {P33MHZ, -1,-1},    /* 0x0f - 01111b - 15 */
    {P33MHZ, 6, 4},     /* 0x10 - 10000b - 16 */
    {P33MHZ, -1, -1},   /* 0x11 - 10001b - 17 */
    {P33MHZ, 3, 4},     /* 0x12 - 10010b - 18 */
    {P33MHZ, -1, -1},   /* 0x13 - 10011b - 19 */
    {P33MHZ, 4,7},      /* 0x14 - 10100b - 20 */
    {P33MHZ, 5, 8},     /* 0x15 - 10101b - 21 */
    {P33MHZ, 4, 8},     /* 0x16 - 10110b - 22 */
    {P33MHZ, -1,-1},    /* 0x17 - 10111b - 23 */
    {P33MHZ, 5, 6},     /* 0x18 - 11000b - 24 */
    {P50MHZ, 4, 5},     /* 0x19 - 11001b - 25 */
    {P33MHZ, 2, 8},     /* 0x1a - 11010b - 26 */
    {P33MHZ, 4, 6},     /* 0x1b - 11011b - 27 */
    {P66MHZ, 3,6},      /* 0x1c - 11100b - 28 */
    {P66MHZ, 3, 6},     /* 0x1d - 11101b - 29 */
    {P33MHZ, -1, -1},   /* 0x1e - 11110b - 30 */
    {P33MHZ, -1,-1},    /* 0x1f - 11111b - 31 */
    };

typedef struct _systodfunctions 
    {
    FUNCPTR init;
    FUNCPTR get;
    FUNCPTR set;
    FUNCPTR getSecond;
    FUNCPTR watchdogArm;
    } SYSTODFUNCTIONS;

SYSTODFUNCTIONS sysTodFuncs;

int sysRevId;

/* locals */

LOCAL char sysModelStr[200];

/* forward declarations */

char *  sysPhysMemTop (void);
STATUS sysEnetAddrGet (char *dev, int unit, unsigned char *pMac);


/* BSP DRIVERS */

#include "../../src/drv/pci/pciConfigLib.c"
#include "mem/byteNvRam.c"

/* See ppcDecTimer.c for more info on decrement timer */
void sysClkRateAdjust(int *freq) { *freq = (int) sysMeasureTimebase(); }
#define PPC_TMR_RATE_SET_ADJUST		sysClkRateAdjust(&sysDecClkFrequency)
#include "../../src/drv/timer/ppcDecTimer.c"

#ifdef INCLUDE_NETWORK
#ifdef INCLUDE_END
#ifdef INCLUDE_BCM570XEND
#include "sysBcm570xEnd.c"
#endif /* INCLUDE_BCM570XEND */
#endif /* INCLUDE_END */
#endif  /* INCLUDE_NETWORK */

/*
 * Aux clock routines utilizing EPIC in MPC8240
 */

FUNCPTR 	sysAuxClkRoutine;
int 		sysAuxClkArg;
int 		sysAuxClkTicksPerSecond;
int 		sysAuxClkFrequency;

void sysAuxClkInt(void)
{
    if (sysAuxClkRoutine)
	(*(FUNCPTR) sysAuxClkRoutine) (sysAuxClkArg);
}

STATUS sysAuxClkConnect(FUNCPTR routine, int arg)
{
    sysAuxClkRoutine = routine;
    sysAuxClkArg = arg;

    return intConnect(INUM_TO_IVEC(EPIC_VECTOR_TM0), sysAuxClkInt, 0);
}

STATUS sysAuxClkRateSet(int ticksPerSecond)
{
    UINT32		reg;

    if (ticksPerSecond < AUX_CLK_RATE_MIN || ticksPerSecond > AUX_CLK_RATE_MAX)
        return ERROR;

    sysAuxClkTicksPerSecond = ticksPerSecond;
    sysAuxClkFrequency = sysMeasureTimebase() / 2;

    /*
     * Set clock rate to default just in case, which is 1/8 of the timebase
     * divider input frequency.  The PPC timebase counter itself operates
     * at 1/4 the divider input frequency (twice the aux clock).
     */

    reg = sysEUMBBARRead(EPIC_INT_CONF_REG);
    reg &= ~0x70000000;
    reg |=  0x40000000;
    sysEUMBBARWrite(EPIC_INT_CONF_REG, reg);

    sysEUMBBARWrite(EPIC_TM0_BASE_COUNT_REG,
		    sysAuxClkFrequency / sysAuxClkTicksPerSecond);

    return OK;
}

int sysAuxClkRateGet(void)
{
    return sysAuxClkTicksPerSecond;
}

void sysAuxClkEnable(void)
{
    intEnable(EPIC_VECTOR_TM0);
}

void sysAuxClkDisable(void)
{
    intDisable(EPIC_VECTOR_TM0);
}

/*
 * Reboot system.
 */

void sysReboot(void)
{
    /* 
    * Force unlock of interrupts.
    * This hack makes ctrl-\ reboot work
    */
    intUnlock (intLock() | 0x00008000);
    SYS_HARD_RESET();
}

/*
 * Tell if a buffer contains a valid ansiTime time zone specification
 */

int sysTimeZoneGood(char *tz)
{
    int			i, colons = 0;

    for (i = 0; i < 64; i++) {
	if (tz[i] == 0 || tz[i] == 10 || tz[i] == 13) {
	    tz[i] = 0;
	    break;
	}
	if (tz[i] < 32 || (tz[i] & 0x80))
	    return 0;
	if (tz[i] == ':')
	    colons++;
    }

    return (i < 64 && colons == 4);
}

/******************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.  The returned string
* depends on the board model and CPU version being used, for example,
* Motorola YellowKnife
*
* RETURNS: A pointer to the string.
*/


char * sysModel (void)
    {
     ULONG regVal;
     unsigned char temp;
     unsigned char pllVal;

     /* Get the model type from the PVR */

     regVal = sysPVRReadSys();

     switch (regVal)
       {
	  case CPU_TYPE_603:
	  case CPU_TYPE_603E:
	  case CPU_TYPE_603P:
	     strcpy(sysModelStr, "MPC603\n");
	     break;

	  case CPU_TYPE_604:
	  case CPU_TYPE_604E:
	  case CPU_TYPE_604R:
	     strcpy(sysModelStr, "MPC604\n");
	     break;

	  case CPU_TYPE_8240:
	     strcpy(sysModelStr, "MPC8240 - KAHLUA\n");
	     break;

	  case CPU_TYPE_8245:
	     strcpy(sysModelStr, "MPC8245 - KAHLUA2\n");
	     break;

	  default:
              strcpy (sysModelStr, "unknown");
              break;
	 }	/* switch  */

     strcat(sysModelStr, "Board: ");

     * (unsigned int *) sysPciConfAddr = LONGSWAP (PLL_CFG_ADR_X);
     temp = * ((unsigned char *) sysPciConfData + PLL_CFG_ADR_SHIFT);
     pllVal = temp >> 3;

     switch (sysRevId)
        {
        case ID_LANCELOT:
            strcat(sysModelStr, "Lancelot");
            break;
        case ID_WHITE_KNIGHT:   /* BMW1 has same ID */
            if (pllVal == 0x19 || pllVal == 0x1c)
                strcat(sysModelStr, "WhiteKnight");
            else
                strcat(sysModelStr, SYS_MODEL);
            break;
        default:
            strcat(sysModelStr, SYS_MODEL);
        }

     sprintf(sysModelStr + strlen(sysModelStr),
	     ", PLD rev: 0x%x, Map B (CHRP) %dMB",
             SYS_REVID_GET(), ((unsigned int)sysPhysMemTop())>>20);

     return (sysModelStr);
    }


/*****************************************************************************
*
* sysBspRev - return the BSP version and revision number
*
* This routine returns a pointer to a BSP version and revision number, for
* example, 1.1/0. BSP_REV is concatenated to BSP_VERSION and returned.
*
* RETURNS: A pointer to the BSP version/revision string.
*/

char * sysBspRev (void)
    {
    return (BSP_VERSION BSP_REV);
    }


void
sysNetHwInit()
{
}

STATUS sysLanIntEnable(int intLevel)
{
    intEnable(intLevel);
    return OK;
}

STATUS sysLanIntDisable(int intLevel)
{
    intDisable(intLevel);
    return OK;
}

/*****************************************************************************
*
* Each time VxWorks is rebooted, it starts assigning system-assigned
* port numbers beginning at 1024.  Thus, if vxWorks is rebooted two or
* more times in succession, the same port number will be re-used.
*
* This behavior causes problems when vxWorks is being booted from a
* remote FTP server (particularly one running Solaris), because port
* 1024 goes into a TIME_WAIT state on the server and cannot be reused
* until it times out, typically in 2-4 minutes.
*
* This hack reduces the likelihood of this happening by "wasting"
* a different number of system-assigned port numbers for each boot.
*/

void sysBindFix(void)
{
    UINT8		N;

    sysNvRamGet((char *) &N, 1, NV_OFF_BINDFIX);
    N -= 16;
    sysNvRamSet((char *) &N, 1, NV_OFF_BINDFIX);

    /* This is quite fast even when N=255 */

    while (N--) {
	int s;
	struct sockaddr_in saddr;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = 0;
	saddr.sin_family = AF_INET;
	s = socket(AF_INET, SOCK_STREAM, 0);
	bind(s, (struct sockaddr *) &saddr, sizeof(saddr));
	close(s);
    }
}

#ifdef INCLUDE_RAMIX_4PORT_PMC    
/*
 * Check for RAMIX CP610 PMC Adapters
 * The CP610 is a DC21150 PCI-PCI Bridge with
 * four (4) DC21143 PCI 10/100 Ethernet devices
 * on board.
 */
int sysRamixPMCProbe(void)
    {
    int i, devs=0;
    int BusNo, DevNo, FuncNo;
    int devConfigCommand = (PCI_CMD_IO_ENABLE  |
                           PCI_CMD_MEM_ENABLE |
                           PCI_CMD_MASTER_ENABLE);
     
    for ( i = 0; i < MAX_NUM_DEC_CHIP; i++) 
        {
        if ( pciFindDevice(DC21143_VENDOR_ID, DC21143_DEVICE_ID, i,
                         &BusNo, &DevNo, &FuncNo) != ERROR) 
            {
            if(BusNo > 0)
                 {
                 devs++;
                 /* Found DEC 21143 Ethernet Device (usually on-board) */
                 pciDevConfig(BusNo, DevNo, FuncNo,
                            P2P_4K_PCI_MEM_ADDR(i),
                            P2P_4K_PCI_MEM_ADDR(i),
                            devConfigCommand);
                  pciConfigOutByte(BusNo,DevNo,FuncNo,
                                PCI_CFG_DEV_INT_LINE, BMW_IRQ_CPCI);
                  }
            }
         }
     return devs;
     }
#endif

/* Detect HHB4 / DC21150 PCI-PCI bridge chips, and configure bus if
 * found. Note that this code should be run whenever we find a PCI
 * class code of type bridge. After we have run this procedure for the
 * first bridge, we then need to traverse any sub-bridges. This code
 * deals with only one PCI-PCI bridge and it's IO space for
 * sub-devices. See bmw.h for more details as well as the PCI-PCI
 * bridge specification.
 */
int sysPCIBridgeProbe(int instance,
		      int primary,
		      int secondary,
		      int subordinate)
{
    int BusNo, DevNo, FuncNo;
    unsigned int buses = 0;

    /* Find DC21150 PCI-PCI bridge or
     * HINTCORP HB4 PCI-PCI Bridge
     */
    if ( (pciFindDevice(DC21150_VENDOR_ID,
			DC21150_DEVICE_ID,
			instance,
			&BusNo, &DevNo, &FuncNo) != ERROR) ||
         (pciFindDevice(PERICOM_VENDOR_ID,
            PERICOM_DEVICE_ID,
            instance,
            &BusNo, &DevNo, &FuncNo) != ERROR) ||
         (pciFindDevice(HINT_HB4_VENDOR_ID,
			HINT_HB4_DEVICE_ID,
			instance,
			&BusNo, &DevNo, &FuncNo) != ERROR)) {
	
	/* Disable device */
	pciConfigOutWord(BusNo,DevNo,FuncNo, PCI_CFG_COMMAND, 0x0000);
	pciConfigOutWord(BusNo,DevNo,FuncNo, PCI_CFG_STATUS, 0xffff);
	
	/* Reset secondary bus */
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_BRIDGE_CONTROL,
			 0x0040);
	
	/* Setup topology info */
	buses = (buses & 0xff000000)
	    | ((unsigned int)(primary)     <<  0)
	    | ((unsigned int)(secondary)   <<  8)
	    | ((unsigned int)(subordinate) << 16);
	/*
	 * linux, -We need to blast all three values with a single write.
	 */
	pciConfigOutLong(BusNo, DevNo, FuncNo,
			 PCI_CFG_PRIMARY_BUS, buses);
	
	/* Clear secondary status */
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_SEC_STATUS, 0xffff);
#if 0	
	/* Setup IO address space mapping */
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_IO_BASE,
			 ((P2P_IO_BASE & 0x0000F000) >> 8) |
			 ((P2P_IO_BASE + P2P_IO_SIZE - 1) & 0x0000F000));
#endif	
	/* Setup memory address space mapping */
	pciConfigOutWord(BusNo,DevNo,FuncNo,		    
			 PCI_CFG_MEM_BASE,
			 ((P2P_NONPREF_MEM_BASE & 0xFFF00000) >> 16));
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_MEM_LIMIT,
			 ((P2P_NONPREF_MEM_BASE +
			   P2P_NONPREF_MEM_SIZE - 1) & 0xFFF00000) >> 16);
#if 0	
	/* Setup prefetchable memory address space mapping */	    
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_PRE_MEM_BASE,
			 ((P2P_PREF_MEM_BASE & 0xFFF00000) >> 16));
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_PRE_MEM_LIMIT,
			 ((P2P_PREF_MEM_BASE +
			   P2P_PREF_MEM_SIZE - 1) &  0xFFF00000) >> 16);
	/* only < 4GB space */
	pciConfigOutLong(BusNo,DevNo,FuncNo,	    
			 PCI_CFG_PRE_MEM_BASE_U,
			 P2P_PREF_HI32_BASE);
	pciConfigOutLong(BusNo,DevNo,FuncNo,	    
			 PCI_CFG_PRE_MEM_LIMIT_U,
			 P2P_PREF_HI32_BASE);

	/* Finish setting up IO space */
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_IO_BASE_U,
			 ((P2P_IO_BASE & 0xFFFF0000) >> 16));
	
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_IO_LIMIT_U,
			 ((P2P_IO_BASE +
			   P2P_IO_SIZE - 1) & 0xFFFF0000) >> 16);
#endif
	/* Clear bridge control */
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_BRIDGE_CONTROL,0x0000);
	
	/* Enable PCI clocks on remote end */
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_DEC21150_SEC_CLK,
			 P2P_CLK_ENABLE);

	/* Clear status */
	pciConfigOutByte(BusNo,DevNo,FuncNo,
			 PCI_CFG_DEC21150_SERR_STAT, 0xff);
	
	/* Clear status */
	pciConfigOutWord(BusNo,DevNo,FuncNo,
			 PCI_CFG_STATUS, 0xffff);
	
	/* Re-enable config space */
	pciConfigOutWord(BusNo,DevNo,FuncNo, PCI_CFG_COMMAND,
			 P2P_PMC_ENABLE);
			     
	return OK;
    }    
    return ERROR;
}


/*****************************************************************************
*
* sysHwInit - initialize the system hardware
*
* This routine initializes various features of the board.
* It is the first board-specific C code executed, and runs with
* interrupts masked in the processor.
* This routine resets all devices to a quiescent state, typically
* by calling driver initialization routines.
*
* NOTE: Because this routine will be called from sysToMonitor, it must
* shutdown all potential DMA master devices.  If a device is doing DMA
* at reboot time, the DMA could interfere with the reboot. For devices
* on non-local busses, this is easy if the bus reset or sysFail line can
* be asserted.
*
* NOTE: This routine should not be called directly by the user application.
*
* RETURNS: N/A
*/

void sysHwInit (void)
{
    int mechanism = 1;			/* PCI mechanism - My 6/98 */
    ULONG retVal;

    sysRevId = SYS_REVID_GET();
#if 0
    cacheLib.flushRtn = NULL;		/* no flush necessary */
    cacheLib.invalidateRtn = NULL;	/* no invalidate necessary */
#endif
    sysPciConfAddr = CHRP_REG_ADDR;
    sysPciConfData = CHRP_REG_DATA;

    retVal = sys107RegRead(BMC_BASE);   /* Read Device & Vendor Ids */

    /* Compare with expected value */
    if ( (retVal != VEN_DEV_ID) && (retVal != KAHLUA_ID) )
        if ( (retVal != VEN_DEV_ID) && (retVal != KAHLUA2_ID) )
	    for (;;)
	    ;    
    /*
     * Initialize the EUMBBAR (Embedded Util Mem Block Base Addr Reg).
     * This is necessary before the EPIC, DMA ctlr, I2C ctlr, etc. can
     * be accessed.
     */

    sys107RegWrite(EUMBBAR_REG, EUMBBAR_VAL);

    /* Enable RCS2 with RCS0 timing parameters 
    *  This is enabled in newer bootroms, however, it's duplicated here
    *  in case board is using old bootroms
    */ 

    sys107RegWrite(XROM_CFG1_ADR, 0x84000000);

    /* Map RCS2 at 0x7c000000, 64 Kbytes */

    sys107RegWrite (XROM_CFG3_ADR, (XROM_BASE_ADDR & 0x0ffff000) | 4);

    /* Initialize TOD functions */

    if (sysRevId != ID_BMW_1 && sysRevId != ID_WHITE_KNIGHT)
        {
        sysTodFuncs.init = (FUNCPTR) ds1743_tod_init;
        sysTodFuncs.get = (FUNCPTR) ds1743_tod_get;
        sysTodFuncs.set = (FUNCPTR) ds1743_tod_set;
        sysTodFuncs.getSecond = (FUNCPTR) ds1743_tod_get_second;
        sysTodFuncs.watchdogArm = (FUNCPTR) NULL;
        }
    else
        {
        sysTodFuncs.init = (FUNCPTR) m48_tod_init;
        sysTodFuncs.get = (FUNCPTR) m48_tod_get;
        sysTodFuncs.set = (FUNCPTR) m48_tod_set;
        sysTodFuncs.getSecond =  (FUNCPTR) m48_tod_get_second;
        sysTodFuncs.watchdogArm = (FUNCPTR) m48_watchdog_arm;
        }

    /* Enable RCS3 with RCS0 timing parameters */

    sys107RegWrite(XROM_CFG2_ADR, 0x84000000);

    /* Map RCS3 for the 32MB flash.  Region size 32MB (size of flash window) */
    sys107RegWrite (XROM_CFG4_ADR, (FLASH2_START & 0x0ffff000) | 0xd);

    /*
     * Initialize EPIC 
     */

    epic_init();

    BPRINT("TOD ");

    sysTodInit ((void *) TOD_BASE);

    sysRamClk = sysRamClkGet();

    BPRINT("SER ");
#ifdef INCLUDE_SERIAL
    sysSerialHwInit ();		/* serial devices */
#endif

    BPRINT("PCI ");
#ifdef INCLUDE_PCI    
    /* Initialize PCI, before the network, so that images work
     * correctly with PCI devices.
     */
    pciConfigLibInit (mechanism, sysPciConfAddr, sysPciConfData, 0x0);

    /* Probe for bus1: DC-21150/HB4 PCI-PCI bridge */

        sysPCIBridgeProbe(0, 0, 1, 1);

#ifdef INCLUDE_RAMIX_4PORT_PMC    
    /* Probe for 21143 chips off aux bus */
    if (sysRevId != ID_LANCELOT && sysRevId != ID_CFM_1)
        sysRamixPMCProbe();
#endif /* INCLUDE_RAMIX_4PORT_PMC */
    
#endif
    
    BPRINT("NET ");
#ifdef INCLUDE_NETWORK
    sysNetHwInit(); 		/* network interface */
#endif

    /* mpc824x_i2c_init(0x54, 100000, MPC824X_I2C_MODE_INTR); */

}

/*
 * sysMeasureTimebase - Measure decrementer/timebase frequency
 *
 *   Performs the same measurement loop in two trials to minimize
 *   I-cache effects, as well as to line up the measurement on an
 *   even TOD second.  Locks out interrupts for accuracy.  Takes
 *   an average of 1.5 seconds to perform the measurement the first
 *   time it is called.  After that, it keeps returning the same value.
 *   Returns the decrementer/timebase frequency in Hz.
 */

UINT32 sysMeasureTimebase(void)
{
    static UINT32	measured = 0;

    BPRINT("CLK!");

#ifdef TOD_NONE
    measured = DEC_CLOCK_FREQ;
#endif

    if (! measured) {
	int		il, loop, sec;
	UINT32		t1, t2, dummy;

	il = intLock();

	vxTimeBaseGet(&dummy, &t2);	/* Get routine in the I-cache */

	for (loop = 0; loop < 2; loop++) {
	    sec = sysTodGetSecond ();
	    while (sysTodGetSecond () == sec)
		;
	    t1 = t2;
	    vxTimeBaseGet(&dummy, &t2);
	}

	measured = t2 - t1;    /* TBL overflow does not matter here */

	intUnlock(il);
    }

    return measured;
}

/*******************************************************************************
*
* sysHwInit2 - initialize additional system hardware
*
* This routine connects system interrupt vectors and configures any
* required features not configured by sysHwInit. It is called from usrRoot()
* in usrConfig.c after the multitasking kernel has started. NOT!
*
* RETURNS: N/A
*/

void 
sysHwInit2 (void)
{
    static int	initialized;		/* must protect against double call! */

    if (initialized)
	return;

    initialized = TRUE;

    /*	sysPciIntRoute();   set up PCI INTA,B,C,d -> IRQ9,10,11,12 */

#ifdef INCLUDE_SERIAL
    sysSerialHwInit2();		/* connect serial interrupts */
#endif

#ifdef INCLUDE_NETWORK
     /* sysNetHwInit2 ();	 */
#endif

#ifdef INCLUDE_PCI
    /* TODO - any secondary PCI setup */
#endif

    /* L2 Cache setup */

#if	defined(INCLUDE_CACHE_SUPPORT) && defined(INCLUDE_CACHE_L2)
#  ifdef USER_L2_CACHE_ENABLE
    sysL2CacheInit ();
#  else
    sysL2CacheDisable ();
#  endif
#endif /*(INCLUDE_CACHE_SUPPORT) && (INCLUDE_CACHE_L2)*/

#ifdef USER_750_L2BACK			/* MPC750 L2 Back cache - My 9/98 */
    {
	ULONG regVal; 
	regVal = sysL2CRRead();		/* Read content of L2CR register  */
	sysL2BackGlobalInv();
	sysL2BackEnable(regVal);	/* Enable L2 Backside cache */
    }
#endif

}



/*******************************************************************************
*
* sysPhysMemTop - get the address of the top of physical memory
*
* This routine returns the address of the first missing byte of memory,
* which indicates the top of memory.
*
* Normally, the user specifies the amount of physical memory with the
* macro LOCAL_MEM_SIZE in config.h.  BSPs that support run-time
* memory sizing do so only if the macro LOCAL_MEM_AUTOSIZE is defined.
* If not defined, then LOCAL_MEM_SIZE is assumed to be, and must be, the
* true size of physical memory.
*
* NOTE: Do no adjust LOCAL_MEM_SIZE to reserve memory for application
* use.  See sysMemTop() for more information on reserving memory.
*
* RETURNS: The address of the top of physical memory.
*
* SEE ALSO: sysMemTop()
*/

char * sysPhysMemTop (void)
    {
    static char * physTop = NULL;

    if (physTop == NULL)
	{
#ifdef LOCAL_MEM_AUTOSIZE

#define POW_2_20 (1024*1024)

        int mber, megs;

        /* Determine installed memory by inspecting the memory controller */
        mber = sys107RegRead(MEM_EN_ADR);
        switch (mber) {
        case 0x01:
            megs = (sys107RegRead(MEM_END1_ADR) & 0xff) + 1;
            physTop = (char *)(LOCAL_MEM_LOCAL_ADRS + (megs*POW_2_20));
            break;
        case 0x03:
            megs = ((sys107RegRead(MEM_END1_ADR)>>8) & 0xff) + 1;
            physTop = (char *)(LOCAL_MEM_LOCAL_ADRS + (megs*POW_2_20));
            break;
        default:
            /* MEM_EN_ADR makes no sense, so just assume a default */
            printf("MEM_EN_ADDR error(%02x)\n", mber);
            physTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
            break;
        }

        /* Update memory description table */
        sysPhysMemDesc[1].len = (UINT32)physTop +
            LOCAL_MEM_LOCAL_ADRS - RAM_LOW_ADRS;


#else
	/* Don't do autosizing, if size is given */

	physTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);

#endif /* LOCAL_MEM_AUTOSIZE */
	}

    return physTop;
    }


/*******************************************************************************
*
* sysMemTop - get the address of the top of VxWorks memory
*
* This routine returns a pointer to the first byte of memory not
* controlled or used by VxWorks.
*
* The user can reserve memory space by defining the macro USER_RESERVED_MEM
* in config.h.  This routine returns the address of the reserved memory
* area.  The value of USER_RESERVED_MEM is in bytes.
*
* RETURNS: The address of the top of VxWorks memory.
*/

char * sysMemTop (void)
    {
    static char * memTop = NULL;

    if (memTop == NULL)
	{
	memTop = sysPhysMemTop () - USER_RESERVED_MEM;
	}

    return memTop;
    }

/******************************************************************************
*
* sysBacktracePpc - perform a stack trace of the caller with symbols
* 		by Curt McDowell - csm@broadcom.com 11/15/99
*
* NOTE:
* 
* This routine is specific to PowerPC and GCC stack frame generation.
* The frame save code must be generated as follows at the beginning of
* each routine because the routine searches for the instructions:
*
*   9421ffe8    stwu        r1,-X(r1)	-Create stack frame, save sp
*   7c0802a6    mfspr       r0,LR	-Must be next instruction
*   93a1000c    stw         r29,12(r1)	-Must save temps next
*   93c10010    stw         r30,16(r1)
*   93e10014    stw         r31,20(r1)
*   9001001c    stw         r0,X+4(r1)	-Must save LR last (between frames)
*/

#define SYSBT_MAX_FUNC_WORDS	4096

LOCAL void sysbt_addr2sym(char *symName, UINT addr)
{
    extern SYMTAB_ID	sysSymTbl;
    UINT		symVal;
    SYM_TYPE		symType;
    int                 rc;

#if VX_VERSION==55 || VX_VERSION==62
    char                *symNameTmp;
    
    rc = symByValueFind(sysSymTbl,
		       (UINT) addr,
		       &symNameTmp, &symVal, &symType);
    if (rc != ERROR)
        strcpy(symName, symNameTmp);
#else
    rc = symFindByValue(sysSymTbl,
		       (UINT) addr,
		       symName, &symVal, &symType);
#endif
    if (rc == ERROR)
        sprintf(symName, "0x%x", addr);
    else if (symVal < addr)
	sprintf(symName + strlen(symName),
		" + 0x%x", addr - symVal);
    else if (symVal > addr)
	sprintf(symName + strlen(symName),
		" - 0x%x", symVal - addr);
}

void sysBacktracePpc(char *pfx, char *sfx, int direct)
{
    UINT		*sp, *pc, *fn_start, *fn_pro, *caller_sp, *ra;
    extern UINT		*vxSpGet(), *vxPcGet();		/* sysALib.s */
    int			limit, first = 1;
    char		buf[2048], *s;
    char		symName[MAX_SYS_SYM_LEN+1];

    sp = vxSpGet();
    pc = vxPcGet();

    s = buf;

    strcpy(s, pfx);
    while (*s) s++;

    while (s < &buf[sizeof (buf) - 128]) {
	fn_start = pc;
	for (limit = 0; limit < SYSBT_MAX_FUNC_WORDS; limit++) {
	    /* mfspr r0, LR */
	    if (*fn_start-- == 0x7c0802a6)
		break;
	}
	if (limit == SYSBT_MAX_FUNC_WORDS ||
	    (*fn_start & 0xffff8000) != 0x94218000) /* stwu r1,x(r1); x<0 */
	    break;
	fn_pro = fn_start;
	for (limit = 0; limit < SYSBT_MAX_FUNC_WORDS; limit++)
	    if ((*++fn_pro & 0xffff8000) == 0x90010000)	/* stw r0,y(r1); y>0 */
		break;
	if (limit == SYSBT_MAX_FUNC_WORDS)
	    break;
	ra = (UINT *) sp[(*fn_pro & 0x0000ffff) / 4];
	caller_sp = (UINT *) sp[0];
	sysbt_addr2sym(symName, (UINT) pc);
	if (! first)
	    sprintf(s,
		    "FUNC = 0x%x, PC = 0x%x (%s), SP = 0x%x\n",
		    (UINT) fn_start, (UINT) pc, symName, (UINT) sp);
	while (*s) s++;
	if ((UINT) ra < 0x100 ||
	    (UINT) ra >= (UINT) sysPhysMemTop()) {
	    sprintf(s, "RA 0x%x out of range\n", (UINT) ra);
	    while (*s) s++;
	    break;
	}
	if ((UINT) caller_sp < 0x100 ||
	    (UINT) caller_sp >= (UINT) sysPhysMemTop()) {
	    sprintf(s, "Caller SP 0x%x out of range\n", (UINT) caller_sp);
	    while (*s) s++;
	    break;
	}
	pc = ra;
	sp = caller_sp;
	first = 0;
    }

    strcpy(s, sfx);

    if (direct)
	sysSerialPrintString(buf); 	/* Atomically print giant string */
    else {
	fputs(buf, stdout);		/* Regular print giant string */
	fflush(stdout);
    }
}

/******************************************************************************
 *
 * sysDumpExcMsg
 *
 * Print readable portion of exception message as a single atomic string
 */

void sysDumpExcMsg(char *pfx, char *sfx)
{
    char		buf[1024], *s = buf;
    char		*msg = EXC_MSG_ADRS + 2;
    int			msglen = sysExcMsg - msg;

    strcpy(s, pfx);
    while (*s) s++;

    if (msglen > 0) {
	memcpy(s, msg, msglen);
	s += msglen;
    }

    strcpy(s, sfx);

    sysSerialPrintString(buf);	/* Atomically print one giant string */
}

/******************************************************************************
*
* sysToMonitor - transfer control to the ROM monitor
*
* This routine transfers control to the ROM monitor.  Normally, it is called
* only by reboot()--which services ^X--and by bus errors at interrupt level.
* However, in some circumstances, the user may wish to introduce a
* <startType> to enable special boot ROM facilities.
*
* The entry point for a warm boot is defined by the macro ROM_WARM_ADRS
* in config.h.  We do an absolute jump to this address to enter the
* ROM code.
*
* RETURNS: Does not return.
*/

int sysToMonitorExcMessage = 1;
int sysToMonitorBacktrace = 1;
int sysToMonitorReboot = 1;
void (*sysToMonitorHook)(void);

STATUS sysToMonitor
    (
    int startType    /* parameter passed to ROM to tell it how to boot */
    )
    {
#if 0
    FUNCPTR pRom = (FUNCPTR) (ROM_WARM_ADRS);

    intLock ();			/* disable interrupts */

    cacheDisable (INSTRUCTION_CACHE);  	/* Disable the Instruction Cache */
    cacheDisable (DATA_CACHE);   	/* Disable the Data Cache */

# if     (CPU == PPC604)
    vxHid0Set (vxHid0Get () & ~_PPC_HID0_SIED);	/* Enable Serial Instr Exec */
# endif  /* (CPU == PPC604) */
    printf("Calling sysHwInit...");
    sysHwInit ();		/* disable all sub systems to a quiet state */

    printf("Calling sysHwInit...");
    vxMsrSet (0);		/* Clear the MSR */
    printf("Jumping to Warm boot location...");
    (*pRom) (startType);	/* jump to romInit.s */
#endif

    if (sysToMonitorHook)
	(*sysToMonitorHook)();

    if (sysToMonitorExcMessage)
	sysDumpExcMsg("\n--- Exception Message ---\n", "");

    if (sysToMonitorBacktrace)
	sysBacktracePpc("\n--- Stack Trace ---\n", "", 1);

    if (sysToMonitorReboot)
	sysReboot();

    return (OK);		/* in case we continue from ROM monitor */
    }

/******************************************************************************
*
* sysProcNumGet - get the processor number
*
* This routine returns the processor number for the CPU board, which is
* set with sysProcNumSet().
*
* RETURNS: The processor number for this CPU board.
*
* SEE ALSO: sysProcNumSet()
*/

int sysProcNumGet (void)
    {
    return (sysProcNum); 
    }

/******************************************************************************
*
* sysProcNumSet - set the processor number
*
* This routine sets the processor number for the CPU board.  Processor numbers
* should be unique on a single backplane.
*
* For bus systems, it is assumes that processor 0 is the bus master and
* exports its memory to the bus.
*
* RETURNS: N/A
*
* SEE ALSO: sysProcNumGet()
*/

void sysProcNumSet
    (
    int procNum			/* processor number */
    )
    {
    sysProcNum = procNum;

    if (procNum == 0)
        {

#ifdef INCLUDE_PCI
	/* TODO - Enable/Initialize the interface as bus slave */
#endif
	}
    }


/************************************************************************ 
 *  This procedure reads a 32-bit address MPC107 register, and returns   
 *  a 32 bit value.  It swaps the address to little endian before        
 *  writing it to config address, and swaps the value to big endian      
 *  before returning to the caller.     
 */

ULONG sys107RegRead(regNum)
ULONG regNum;
{
    ULONG temp;

    /* swap the addr. to little endian */
    *(ULONG *)sysPciConfAddr = LONGSWAP(regNum);
    temp = *(ULONG *)sysPciConfData;
    return ( LONGSWAP(temp));		      /* swap the data upon return */
}

/************************************************************************
 *  This procedure writes a 32-bit address MPC107 register, and returns   
 *  a 32 bit value.  It swaps the address to little endian before        
 *  writing it to config address, and swaps the value to big endian      
 *  before returning to the caller.     
 */

void sys107RegWrite(regNum, regVal)
ULONG regNum;
ULONG regVal;
{
    /* swap the addr. to little endian */
    *(ULONG *)sysPciConfAddr = LONGSWAP(regNum);
    *(ULONG *)sysPciConfData = LONGSWAP(regVal);
    return;
}


/**********************************************************************
 *  Write a word (16 bits) to a memory location after the value
 *  has been byte swapped (big to little endian or vice versa)    
 */

void sysOutWord
    (
    ULONG       address,       /* I/O address to write to */
    UINT16        data            /* data to write */
    )
    {
    *(UINT16 *)address = BYTE_SWAP_16_BIT(data);  
    SYNC;
    }                                                


/**********************************************************************
 *  Write a long word (32 bits) to a memory location after the value
 *  has been byte swapped (big to little endian or vice versa)      
 */

void sysOutLong
    (
    ULONG address,        /* I/O address to write to */
    ULONG    data            /* data to write */
    )
    {
   
    *(ULONG *)address = LONGSWAP(data); 
    SYNC;
    }                                                


/*********************************************************************
 *  Read a word (16 bits) from a memory location, and byte swap the 
 *  value before returning to the caller.
 */

USHORT sysInWord
    (
    ULONG       address         /* I/O address to read from */
    )
{
  USHORT retVal;
  retVal = BYTE_SWAP_16_BIT(*(USHORT *)address);
  return (retVal);
}


/*******************************************************************
 *  Read a long word (32 bits) from a memory location, and byte 
 *  swap the value before returning to the caller.     
 */

ULONG sysInLong
    (
    ULONG       address         /* I/O address to read from */
    )
{
    ULONG retVal;
    retVal = LONGSWAP(*(ULONG *)address);
    return (retVal);
}


/*******************************************************************
 *  Read a register in the Embedded Utilities Memory Block address
 *  space.  
 *  Input: regNum - register number + utility base address.  Example, 
 *         the base address of EPIC is 0x40000, the register number
 *	   being passed is 0x40000+the address of the target register.
 *	   (See epic.h for register addresses).
 *  Output:  The 32 bit little endian value of the register.
 */

ULONG sysEUMBBARRead(regNum)
ULONG regNum;
{
    ULONG temp;
    __asm__("sync"); /* MPC8240UM: 10.4.8 - issue sync after EUMBBAR r/w */
    temp = *(ULONG *) (EUMBBAR_VAL + regNum) ;
    temp = LONGSWAP(temp);

    return (temp);
}

	
/*******************************************************************
 *  Write a value to a register in the Embedded Utilities Memory 
 *  Block address space.
 *  Input: regNum - register number + utility base address.  Example, 
 *                  the base address of EPIC is 0x40000, the register 
 *	            number is 0x40000+the address of the target register.
 *	            (See epic.h for register addresses).
 *         regVal - value to be written to the register.
 */

void sysEUMBBARWrite(regNum, regVal)
ULONG regNum;
ULONG regVal;
{
    *(ULONG *) (EUMBBAR_VAL + regNum) = LONGSWAP(regVal);
    __asm__("sync");/* MPC8240UM: 10.4.8 - issue sync after EUMBBAR r/w */
    return;
}


/*
 * Write characters to LCD display.
 * Note that the bytes for the first character is the last address.
 */
void
sysLedDsply(char* msg)
{
    LED_REG(0) = msg[3];
    LED_REG(1) = msg[2];
    LED_REG(2) = msg[1];
    LED_REG(3) = msg[0];
}

/******************************************************************************
*
* sysRamClkGet - compute the RAM clock speed
*
* This routine returns the RAM clock speed for a given PCI clock by reading
* the PLL configuration register.
*
* RETURNS: Ram clock speed in Hz.
*
* SEE ALSO: sysProcNumSet()
*/
int sysRamClkGet()
    {

    unsigned char temp;
    unsigned char pllVal;
    int factor;
    int ramClk;
    int pciFreq;

    /*
    * Read the PLL configuration register. It's a byte wide register
    * that's not on a 4-byte boundary, thus the address shift.
    */

    * (unsigned int *) sysPciConfAddr = LONGSWAP (PLL_CFG_ADR_X);
    temp = * ((unsigned char *) sysPciConfData + PLL_CFG_ADR_SHIFT);
    pllVal = temp >> 3;
    factor = pllMult[pllVal].pciToMem;
    pciFreq = pllMult[pllVal].pcifreq;

    if (factor == -1)
        return (-1);

    /*
    * Now compute the ram clock, "factor" is double the multiplier.
    * Since some of the original multipliers are odd multiples of
    * .5, the double multiple is used to avoid the floating point unit.
    */
   
    ramClk = pciFreq * (factor >> 1);
    if (factor & 0x1)   /* take care of the .5 multipliers */
       ramClk += (pciFreq >> 1);

    return (ramClk);
    
    }

/******************************************************************************
*
* sysPciToRamClkGet - get the double multiple for PCI to RAM clock
*
* This routine returns the ratio of RAM to PCI clock speed times 2.
* The value is determined by reading the PLL configuration register.
*
* RETURNS: ratio * 2
*
*/
int sysPciToRamClkGet()
    {

    unsigned char temp;
    unsigned char pllVal;
    int factor;

    /*
    * Read the PLL configuration register. It's a byte wide register
    * that's not on a 4-byte boundary, thus the address shift.
    */

    * (unsigned int *) sysPciConfAddr = LONGSWAP (PLL_CFG_ADR_X);
    temp = * ((unsigned char *) sysPciConfData + PLL_CFG_ADR_SHIFT);
    pllVal = temp >> 3;
    factor = pllMult[pllVal].pciToMem;
    return (factor);
    }

/******************************************************************************
*
* sysRamToCpuClkGet - get the double multiple for RAM to CPU clock
*
* This routine returns the ratio of CPU to RAM clock speed times 2.
* The value is determined by reading the PLL configuration register.
*
* RETURNS: ratio * 2
*
*/
int sysRamToCpuClkGet()
    {

    unsigned char temp;
    unsigned char pllVal;
    int factor;

    /*
    * Read the PLL configuration register. It's a byte wide register
    * that's not on a 4-byte boundary, thus the address shift.
    */

    * (unsigned int *) sysPciConfAddr = LONGSWAP (PLL_CFG_ADR_X);
    temp = * ((unsigned char *) sysPciConfData + PLL_CFG_ADR_SHIFT);
    pllVal = temp >> 3;
    factor = pllMult[pllVal].memToCpu;
    return (factor);
    }

/******************************************************************************
*
* sysUsecDelay - delay for the specified amount of time (microseconds)
*
* This routine will delay for the specified amount of time by counting
* decrementer ticks.
*
* This routine is not dependent on a particular rollover value for
* the decrementer, it should work no matter what the rollover
* value is.
*
* A small amount of count may be lost at the rollover point resulting in
* the sysMsDelay() causing a slightly longer delay than requested.
*
* This routine will produce incorrect results if the delay time requested
* requires a count larger than 0xffffffff to hold the 
* elapsed tick count.  For a system bus speed of 67 MHZ this amounts to
* about 258 seconds.
*
* RETURNS: N/A
*/

#define DELTA(a,b)                   (abs((int)a - (int)b))

void sysUsecDelay
    (
    UINT        delay                   /* length of time in microsecs to delay */
    )
    {
    register UINT oldval;               /* decrementer value */
    register UINT newval;               /* decrementer value */
    register UINT totalDelta;           /* Dec. delta for entire delay period */
    register UINT decElapsed;           /* cumulative decrementer ticks */

    /*
     * Calculate delta of decrementer ticks for desired elapsed time.
     * The DEC register changes every 4 system logic (SDRAM) clocks. 
     */

    totalDelta = ((sysRamClk / 4) / 1000000) * delay;

    /*
     * Now keep grabbing decrementer value and incrementing "decElapsed" until
     * we hit the desired delay value.  Compensate for the fact that we may
     * read the decrementer at 0xffffffff before the interrupt service
     * routine has a chance to set in the rollover value.
     */

    decElapsed = 0;

    oldval = vxDecGet ();

    while (decElapsed < totalDelta)
        {
        newval = vxDecGet ();
        if ( DELTA(oldval,newval) < 1000 )
            decElapsed += DELTA(oldval,newval);  /* no rollover */
        else
            if (newval > oldval)
                decElapsed += abs((int)oldval);  /* rollover */
        oldval = newval;
        }
    }

/******************************************************************************
*
* sysEnetAddrGet - read the ethernet address from NVRAM
*
* This routine returns the system ethernet address from flash.
* If more than one ethernet driver is installed, the ethernet address
* will be the unit number added to the system ethernet address (unit 0).
*
* RETURNS: OK if flash read successful and enet addr isn't garbage
*          ERROR if flash read fails or enet addr is all 0's or all f's
*
*/
STATUS sysEnetAddrGet
    (
    char *dev, 		/* name of device, e.g. "bc" */
    int unit,		/* unit number */
    unsigned char *pMac		/* where to write the mac address */
    )
{
    int i;
    int allzeros;
    int allfs;

    if (sysNvRamGet(pMac, 6, NV_OFF_MACADDR) == ERROR)
        return (ERROR);

    /*
     * Check what we got from NVRAM.  If it looks fishy, return ERROR 
     */

    allzeros = 1;
    allfs = 1;
    for (i = 0; i < 6; i++)
        {
        if (pMac[i] == '\0')
            {
            if (allzeros)
                continue;
            else
                goto goodmac;
            }
        else if (pMac[i] == (char) 0xff)
            {
            if (allfs)
                continue;
            else
                goto goodmac;
            }
        else
            goto goodmac;
        }

    return (ERROR);

goodmac:

    pMac[5] = pMac[5] + unit;
    return (OK);
}

#ifndef INCLUDE_FLASH
/*
*  This call is here because the xmodem code calls flashFsSync().
*  flashFsSync() is normally defined in flashFsLib, but BMW uses
*  the DOC, not a flash array.
*/
STATUS flashFsSync(void)
    {
    return (OK);
    }
#endif

UINT8 sysBoardRev()
{
    return (SYS_REVID_GET());
}

int sysTodGetSecond (void)
    {
    if (sysTodFuncs.getSecond == NULL)
        return -1;
    else
        return (sysTodFuncs.getSecond ());
    }

STATUS sysTodGet
    (
    int *pYear,
    int *pMonth,
    int *pDay,
    int *pHour,
    int *pMin,
    int *pSec 
    )
    {
    if (sysTodFuncs.get == NULL)
        return (ERROR);
    else
        return (sysTodFuncs.get (pYear, pMonth, pDay, pHour, pMin, pSec));
    }

STATUS sysTodSet
    (
    int year,
    int month,
    int day,
    int hour,
    int min,
    int sec 
    )
    {
    if (sysTodFuncs.set == NULL)
        return (ERROR);
    else
        return (sysTodFuncs.set (year, month, day, hour, min, sec));
    }

STATUS sysTodInit (UINT8 *addr)
    {
    if (sysTodFuncs.init == NULL)
        return (ERROR);
    else
        return (sysTodFuncs.init (addr));
    }

STATUS sysTodWatchdogArm
    (
    int usec
    )
    {
    if (sysTodFuncs.watchdogArm == NULL)
        return (ERROR);
    else
        return (sysTodFuncs.watchdogArm (usec));
    }

/*****************************************************************************
*
* sysSerialPrintStringNL - print string and add newline
* 
* This routine should only be used as alternative to sysLedDsply, as
* part of the BPRINT() mechanism for debugging bootroms or board bringup.
*
* Returns:  N/A
*/

void sysSerialPrintStringNL
    (
    char *s
    )
    {
    sysSerialPrintString (s);
    sysSerialPrintString ("\n\r");
    }

void pldRegWrite 
    (
    int reg, 
    UINT8 value
    )
    {
    PLD_REG(reg) = value;
    }

UINT8 pldRegRead 
    (
    int reg 
    )
    {
    UINT8 val;
    val = PLD_REG(reg);
    return (val);
    }

UINT8 pldRegDump 
    (
    int reg 
    )
    {
    UINT8 val;
    val = pldRegRead (reg);
    printf("Reg = 0x%x, val = 0x%x\n", reg, val);
    return (val);
    }

WDOG_ID buttonWd;
WDOG_ID fanWd;
WDOG_ID powerWd;
WDOG_ID temperatureWd;

int buttonEnable(int dummy)
    {
    pldRegWrite (CFM_REG_INTMASK, 
        pldRegRead (CFM_REG_INTMASK) | dummy);
    return 0;
    }

UINT8 lmPresenceVec;
UINT8 lmPresenceVecNew;
UINT8 cfmPresenceVec;
UINT8 cfmPresenceVecNew;

void pldInt (void)
    {
    UINT8 mode;
    UINT8 cause;
    UINT8 mask;
    intDisable (EPIC_VECTOR_EXT0);    /* No nested interrupts */

    cause = pldRegRead (CFM_REG_INTCAUSE);
    mask = pldRegRead (CFM_REG_INTMASK);
    cause = cause & mask;
    pldRegWrite (CFM_REG_INTACK, cause);
    if (cause & CFM_INTMASK_BUTTON)
        {
        /* 
        * Disable button interrupt and spawn watchdog to reenable 
        * the interrupt in .25 second
        */
        pldRegWrite (CFM_REG_INTMASK, 
            pldRegRead (CFM_REG_INTMASK) & ~CFM_INTMASK_BUTTON);
        wdStart (buttonWd, sysClkRateGet() >> 2, buttonEnable, 
            CFM_INTMASK_BUTTON);

        /* Read and change the LM LED mode indicator on the CFM */
        mode = PLD_REG (CFM_REG_LM_LED_MODE);

        if (mode == 0x80)
           mode = 0x02;
        else
           mode <<= 2;

        pldRegWrite (CFM_REG_LM_LED_MODE, mode);

        /* 
        * At this point can do a semGive so that resource manager task will 
        * send a message to each active line module informing it to update
        * its LED display
        */
#if 0
        semGive (lmLedSem);
#endif
        }

    if (cause & CFM_INTMASK_POWER)
        {
        /* Mask the interrupt and re-enable it in 1 minute */

        pldRegWrite (CFM_REG_INTMASK, 
            pldRegRead (CFM_REG_INTMASK) & ~CFM_INTMASK_POWER);
        wdStart (powerWd, 60*sysClkRateGet(), buttonEnable, CFM_INTMASK_POWER);
        logMsg ("POWER ALERT!  Check power supply immediately\n",0,0,0,0,0,0);
        }

    if (cause & CFM_INTMASK_FAN)
        {
        /* Mask the interrupt and re-enable it in 1 minute */
        pldRegWrite (CFM_REG_INTMASK, 
            pldRegRead (CFM_REG_INTMASK) & ~CFM_INTMASK_FAN);
        wdStart (fanWd, 60*sysClkRateGet(), buttonEnable, CFM_INTMASK_FAN);
        logMsg ("FAN ALERT!  Check air supply immediately\n",0,0,0,0,0,0);
        }

    if (cause & CFM_INTMASK_TEMPERATURE)
        {
        /* Mask the interrupt and re-enable it in 1 minute */
        pldRegWrite (CFM_REG_INTMASK, 
            pldRegRead (CFM_REG_INTMASK) & ~CFM_INTMASK_TEMPERATURE);
        wdStart (temperatureWd, 60*sysClkRateGet(), buttonEnable,
            CFM_INTMASK_TEMPERATURE);
        logMsg ("TEMPERATURE ALERT!  Insert thermometer immediately\n",
            0,0,0,0,0,0);
        }

#define MAX_LMS 8
    if (cause & CFM_INTMASK_LM_PRESENCE)
        {
        int i;
        UINT8 vec;
        UINT8 vecNew;
sysSerialPrintString("int: LM presence\n");
        lmPresenceVecNew = pldRegRead (CFM_REG_LM_PRESENCE);

        if (lmPresenceVecNew == lmPresenceVec)
            return;  /* spurious interrupt */

        vec = lmPresenceVec;
        vecNew = lmPresenceVecNew;

        for (i = 0; i < MAX_LMS; i++ )
            {
            if ((vecNew & 0x1) != (vec & 0x1))
                if (vecNew & 0x1)
                    {
                    /* 
                    * A Line module has been removed.
                    * Pull the reset line low for the empty slot.
                    */
                    pldRegWrite (CFM_REG_LM_RESET, 
                        pldRegRead (CFM_REG_LM_RESET) & ~(1<<i));
                    /* 
                    * At this point, a resource manager should
                    * be notified of the event
                    */
                    }
                else
                    {
                    /* 
                    * A new line module was inserted.  At this point
                    * a resource manager should be notified so that
                    * it can decide whether release the new LM from reset.
                    * The code below shows how to take the
                    * new module out of reset.
                    */
#if 0
                    pldRegWrite (CFM_REG_LM_RESET,
                        pldRegRead (CFM_REG_LM_RESET) | (1<<i));
#endif
                    }
            vec >>= 1;
            vecNew >>= 1;
            }

        lmPresenceVec = lmPresenceVecNew;
        }

    if (cause & CFM_INTMASK_CFM_PRESENCE)
        {
sysSerialPrintString("int: CFM presence\n");
        cfmPresenceVecNew = pldRegRead (CFM_REG_CFM_PRESENCE) & 0x1;

        if (cfmPresenceVecNew == cfmPresenceVec)
            return;  /* spurious interrupt */

        cfmPresenceVec = cfmPresenceVecNew;

        if (cfmPresenceVec == 0x1)
            {
            /* A CFM has been removed.  Pull the reset line of the empty slot */
            pldRegWrite (CFM_REG_CFM_RESET, 0);
            sysSerialPrintString ("reset CFM\n");
            }
        else
            {
            /* 
            * A new CFM was inserted.  At this point
            * a resource manager should be notified so that
            * it can decide whether or not to allow the new CFM to boot.
            */
#if 0
            /* Example:  releases CFE reset line */
            pldRegWrite (CFM_REG_CFM_RESET, 0x1);
#endif
            sysSerialPrintString ("new CFM present\n");
            }
        }

    intEnable (EPIC_VECTOR_EXT0);
    }

/*******************************************************************************
*
*   pldIntConnect
*
*   Connect an interrupt handler to IRQ0.  IRQ0 comes from the FPGA on
*   the chassis system.  Devices on that interrupt are:
*
*   -- Push-button for line module LED display
*   -- Fan (failure interrupt)
*   -- Power supply (failure)
*   -- Temperature 
*   -- Line module presence (one interrupt for all modules)
*   -- CFM presence
*
*   The 6 interrupt types are maskable.
*   When a PLD interrupt occurs, the interrupting device in found
*   in the PLD's cause register.
*/
STATUS pldIntConnect (void)
    {
    if (intConnect ((VOIDFUNCPTR *)INUM_TO_IVEC (EPIC_VECTOR_EXT0),
             (VOIDFUNCPTR)pldInt, 0) != OK)
        {
        printf("pldInterruptConnect failed\n");
        return (ERROR);
        }

    /*
    * For now I'm putting the master check here, but it belongs
    * somewhere else... Jimmy
    */
    if ((pldRegRead (CFM_REG_CFM_RESET) & 0x2) == 0)   /* We are the master */
      {
      pldRegWrite (CFM_REG_CFM_RESET, pldRegRead (CFM_REG_CFM_RESET) | 0x1);
      }

    buttonWd = wdCreate();
    fanWd = wdCreate();
    powerWd = wdCreate();
    temperatureWd = wdCreate();

    /* Enable all devices that interrupt thru PLD  */

    buttonEnable (0x7f);

    /* Enable IRQ from PLD */

    pldRegWrite (CFM_REG_INTMASK, 
        pldRegRead (CFM_REG_INTMASK) | CFM_INTMASK_IRQ0);

    /* Enable interrupt at processor */
    intEnable (EPIC_VECTOR_EXT0);
    return (OK);
    }

int
sysIsCFM()
{
    return (sysRevId == ID_CFM_1);
}

int
sysHasDOC()
{
    return (sysRevId != ID_BCM56215R26T);
}

int
sysSlotIdGet()
{
    if (sysIsCFM()) {
        return((pldRegRead(CFM_REG_CFM_RESET) & CFM_RESET_SLOT_ID)?1:0);
    } else {
        return(0);
    }
}

#if 0
void lmLedModeInit()
    {
    /* Read LM LED mode */
    int mode = pldRegRead (0x4);
    }
#endif
