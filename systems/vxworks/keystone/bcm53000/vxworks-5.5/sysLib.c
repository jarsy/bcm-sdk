/* Copyright (c) 2000-2007 Wind River Systems, Inc. 
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/* $Id: sysLib.c,v 1.6 2011/07/21 16:14:25 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */


/*
DESCRIPTION

This library provides board-specific routines.

INCLUDE FILES: sysLib.h

SEE ALSO:
.pG "Configuration"
*/

/* includes */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <siutils.h>
#include <vxbsp.h>

#include <vxWorks.h>
#include <cacheLib.h>

#include "config.h"

#include <bcmdevs.h>

#include <mipsinc.h>
#include <hndmips.h>
#include <hndcpu.h>
#include <bcm53000.h>


#include "sysLib.h"

#include "flashDrvLib.h"
#include "flashFsLib.h"

#include "vmLib.h"
#include "bcmnvram.h"

#ifdef INCLUDE_IPFTPS
/* for user authentication callback */
#include <ipcom_list.h>
#include <ipcom_sock.h>
#include <ipftps.h>
#endif /* INCLUDE_IPFTPS */

uint32 mipsclk=0;

static int32 wd_ms=0;
static chipcregs_t *cc = NULL;
static si_t *sih;

unsigned int boardtype;         /* broadcom board type (from bcmdevs.h) */

#ifndef INCLUDE_PCI_BUS
#include <drv/pci/pciConfigLib.h>
#endif  /* !INCLUDE_PCI_BUS */

#include <intLib.h>     /* SPR 102678 */

#if defined(INCLUDE_PCI) && !defined(INCLUDE_PCI_BUS)
IMPORT void     sysPciBusErrDisable();
IMPORT void     sysPciBusErrEnable();
UINT32    sysPciBusToLocal (int unit, UINT32 pciAdrs);
UINT32    sysLocalToPciBus (int unit, UINT32 localAdrs);
IMPORT STATUS pciIntConnect (VOIDFUNCPTR *, VOIDFUNCPTR, int);
IMPORT STATUS pciIntDisconnect2 (VOIDFUNCPTR *, VOIDFUNCPTR, int);
#endif /* INCLUDE_PCI */

#include "sysDelay.c"

#if defined(INCLUDE_PCI) && !defined(INCLUDE_PCI_BUS)
#include <drv/pci/pciConfigLib.h>
#include <drv/pci/pciIntLib.h>
#define PCI_SUBCLASS_00         0x00
#define PCI_SUBCLASS_01         0x01
#define PCI_SUBCLASS_04         0x04
#define PCI_SUBCLASS_07         0x07
#define PCI_CLASS_DISPLAY_CTLR  0x03
#define PCI_CLASS_BRIDGE_CTLR   0x06
#define PCI_SUBCLASS_HOST_PCI_BRIDGE    (PCI_SUBCLASS_00)
#define PCI_SUBCLASS_ISA_BRIDGE     (PCI_SUBCLASS_01)
#define PCI_SUBCLASS_P2P_BRIDGE     (PCI_SUBCLASS_04)
#define PCI_SUBCLASS_CARDBUS_BRIDGE (PCI_SUBCLASS_07)
#include <pciAutoConfigLib.c>    /* automatic PCI configuration */
#endif

#include <sysBusPci.c>               /* pciAutoConfig BSP support file */

#ifdef INCLUDE_HW_FP
#include <arch/mips/fppMipsLib.h>
#endif

#ifdef INCLUDE_I2C_DEV
#include "chipc_i2c.h"
#include "i2cM41T81Clock.h"
#include "usrTime.c"
#endif

#ifdef INCLUDE_PCI_TEST
#include "sysSil31xx.c"
#endif  /* INCLUDE_PCI_TEST */

/* externals */

IMPORT void     fpIntr ();
IMPORT void     sysWbFlush ();
IMPORT int      sysFpaAck ();
IMPORT int  sysAtlIntResolve(void);

IMPORT int      sysCompareSet ();
IMPORT int      sysCompareGet();
IMPORT UINT32   sysConfig1Get();
IMPORT void     mipsTlbClear();
IMPORT STATUS   pciConfigFixup();

IMPORT UINT8    ffsMsbTbl[];            /* Msb high interrupt hash table */
IMPORT UINT8    ffsLsbTbl[];            /* Lsb high interrupt hash table */


/* globals */
int   sysVectorIRQ0 =IV_IORQ0_VEC;      /* used by SAL for pci int connect */
int   sysMemTopDebug = 0;
UINT  sysStrayIntCount = 0;     /* Stray interrupt count */

/*
 * Since tying interrupt lines to the processor is board dependent 
 * sysHashOrder is provided to select the prioritization of interrupt lines.  
 * Setting the #define INT_PRIO_MSB to TRUE prioritizes interrupts from 7-0,
 * 7 being highest priority, 0 being lowest.  Setting it to FALSE prioritizes
 * interrupts from 0-7.  See vxbsp.h for the definition of INT_PRIO_MSB.
 */

#if     (INT_PRIO_MSB == TRUE)
UINT8 * sysHashOrder = ffsMsbTbl;       /* interrupt prio., 7 = high 0 = low*/
#else   /* INT_PRIO_MSB == TRUE */
UINT8 * sysHashOrder = ffsLsbTbl;       /* interrupt prio., 0 = high 7 = low*/
#endif  /* INT_PRIO_MSB == TRUE */


/* global */
ULONG sysSoftCompare = 0;               /* last target compare reg value */
int sysSoftCount31_0 = 0;                  /* last target count reg value */
ULONG sysSoftCount63_32 = 0;            /* last target count reg value */

LOCAL void      sysClkInt ();

LOCAL int   sysClkTicksPerSecond    = 100;      /* default sys timer rate    */
LOCAL int   sysClkArg               = 0;        /* clock int routine arg     */
LOCAL BOOL  sysClkConnected         = FALSE;    /* sys clock connect flag    */
LOCAL BOOL  sysClkRunning           = FALSE;    /* sys clock enabled flag    */
LOCAL int   sysClkProcessorTicks;               /* MIPS32 clock ticks        */
LOCAL FUNCPTR   sysClkRoutine       = NULL ;    /* clock interrupt routine   */

#if defined(AUXCLK) || defined(INCLUDE_AUX_CLK) /* not defined by default */
#include "gmac0_core.h"
static uint32 mptimer_id = 0; /* default use multipurpose timer0 */
static uint32 mptimer_int = CI_MP_CNT_0; /* default use multipurpose timer0 */

LOCAL BOOL  sysAuxClkRunning        = FALSE;     /*sys aux clock enabled flag */
LOCAL int   sysAuxClkTicksPerSecond = 200;       /* default aux timer rate    */
LOCAL BOOL  sysAuxClkConnected      = FALSE;     /*sys aux clock connect flag */
LOCAL int   sysAuxClkArg            = 0 ;        /*aux clock int routine arg  */
LOCAL FUNCPTR   sysAuxClkRoutine    = NULL ;     /*aux clock interpt routine  */
void set_auxtimer( uint32 base_addr );
void show_auxtimer( void );
void set_auxtimer_core(int core);
#endif

#ifdef  INCLUDE_TIMESTAMP
LOCAL BOOL  sysTimestampRunning     = FALSE;   /* timestamp running flag */
#endif  /* INCLUDE_TIMESTAMP */

#ifdef INCLUDE_END
#if (defined(INCLUDE_ET0_END) || defined(INCLUDE_ET1_END))
#include "sysEtEnd.c"
#include "sysNet.c"
#endif /* INCLUDE_ET_END */
#endif /* INCLUDE_END */

int Bcm53000HiMemSize;
uint32 Bcm53000HiMemAddr;

/* locals */

/*
 *  This table is critical to interrupt processing.  Do not alter its
 *  contents until you understand the consequences. Refer to the Tornado
 *  for MIPS Architecture supplement for instructions on its use.
 */

typedef struct
    {
    ULONG       intCause;               /* CAUSE IP bit of int source  */
    ULONG       bsrTableOffset;         /* index into BSR table        */
    ULONG       intMask;                /* interrupt mask              */
    ULONG       demux;                  /* demultiplex arg             */
    } PRIO_TABLE;

PRIO_TABLE intPrioTable[8] =
    {
    {CAUSE_SW1,(ULONG) IV_SWTRAP0_VEC, 0x0100, 0},      /* sw trap 0      */
    {CAUSE_SW2,(ULONG) IV_SWTRAP1_VEC, 0x0200, 0},      /* sw trap 1      */
    {CAUSE_IP3,(ULONG) INT_VEC_IORQ0 , 0x0400, 0},              
    {CAUSE_IP4,(ULONG) INT_VEC_IORQ1 , 0x0800, 0},      
    {CAUSE_IP5,(ULONG) INT_VEC_IORQ2 , 0x1000, 0},      
    {CAUSE_IP6,(ULONG) INT_VEC_IORQ3 , 0x2000, 0},      
    {CAUSE_IP7,(ULONG) INT_VEC_IORQ4 , 0x4000, 0},      
    {CAUSE_IP8,(ULONG) INT_VEC_IORQ5 , 0x8000, 0}       
    };

/* Included Generic MIPS Support code */
#define SYS_CLEAR_TLB             /* specialized version of this routine     */
#include <sysMipsLib.c>

#ifndef INCLUDE_SIO_UTILS
/* #include "sysSerial.c" */
#endif  /* !INCLUDE_SIO_UTILS */

#include <arch/mips/taskMipsLib.h>

/* The following definitions are for debug and development purpose */
#define EXCEPTION_DEBUG 0 /* Substitute kernel exception routine with debug routine. */

void sysPlatformInit(void);

/***************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the string "BCM53000 (MIPS74K)".
*/

char *sysModel (void)
    {
    return (SYS_MODEL);
    }

/***************************************************************************
*
* sysBspRev - return the BSP version with the revision eg 1.1/<x>
*
* This function returns a pointer to a BSP version with the revision.
* for eg. 1.1/<x>. BSP_REV is concatenated to BSP_VERSION to form the
* BSP identification string.
*
* RETURNS: A pointer to the BSP version/revision string.
*/

char * sysBspRev (void)
    {
    return (BSP_VERSION BSP_REV);
    }

/*******************************************************************************
*
* sysDelay - allow recovery time for port accesses
*
* This routine provides a brief delay used between accesses to the same serial
* port chip.
* 
* RETURNS: N/A
*/

void sysDelay (void)
    {
    sysUSecDelay(1000);
    }

/***************************************************************************
*
* sysHwInit - initialize the CPU board hardware
*
* This routine initializes various features of the board
* It is called from usrInit() in usrConfig.c.
*
* This routine initializes and turns off the timers.
*
* NOTE:
* This routine should not be called directly by the user.
* 
* RETURNS: N/A
*/

void sysHwInit (void)
    {
    int sr;

    sr = BCM53000_SR;

#if 1
/* FIXME: QuickTurn always poll high USB interrupt(default intmask 0x10), disable IntMask 4 and 5 to prevent from the problem */
    /* Turn of any int used by our devices */
    *((volatile unsigned int *)(0xb8003024)) = 0x0;
    *((volatile unsigned int *)(0xb8003028)) = 0x80000000;
#endif

    /* set default task status register for usrRoot (usrRoot we reset this in sysHwInit2 */
    taskSRInit (BCM53000_SR);
    
    /* init status register but leave interrupts disabled */
    intSRSet (sr & ~SR_IE);
    }


/*******************************************************************************
*
* sharedInt0: checks to see which interrupts are connected to 0 and calls each
* in turn.
*
*******************************************************************************/

LOCAL void sharedInt0 (void)
{
    int intNum;
    FUNCPTR intVec;

    for (intNum = IV_IORQ0_BIT0_VEC; intNum <= IV_IORQ0_MAX_VEC; intNum++)
        if ((intVec = intVecGet((FUNCPTR *) INUM_TO_IVEC(intNum))))
            intVec();
}

/* Handle all possible external interface interrupt sources */
void sysExtInt (void)
{
    int intNum;
    FUNCPTR intVec;

    /* XXX Should have a vector for PCI INTB */
    for (intNum = IV_EXT_ALT1_VEC; intNum <= IV_PCMCIA; intNum++)
        if ((intVec = intVecGet((FUNCPTR *) INUM_TO_IVEC(intNum))))
            intVec();
}   

/*******************************************************************************
*
* PciInt: Allows muliple interrupts to share the PCI int line 
*
*******************************************************************************/

void sysPciInt (int id)
{
    int intNum;
    FUNCPTR intVec;

    /* XXX Should support up to 16 PCI INTA vectors */
    for (intNum = IV_PCI0_0; intNum <= IV_PCI1_MAX; intNum++)
        if ((intVec = intVecGet((FUNCPTR *) INUM_TO_IVEC(intNum))))
            intVec();
}


/***************************************************************************
*
* sysHwInit2 - additional system configuration and initialization
*
* This routine connects system interrupts and does any additional
* configuration necessary.
*
* RETURNS: N/A
*/

#if EXCEPTION_DEBUG /* Debug for QT exception */
void excExcHandleDBG
    (
    int     vecNum,     /* exception vector number */
    ESFMIPS *   pEsf,       /* pointer to exception stack frame */
    REG_SET *   pRegs       /* pointer to general regs on esf */
    );
#endif 

#ifndef BUILD_VXBOOT
#include "cfe_api.c"
#include "prom.c"
#endif

void sysHwInit2 (void)
{
    uint i;
    uint8 irq, flag;
    char xonString[3];

    /* Perform anything specific to this platform design : set global sih */
    sysPlatformInit(); 
    ASSERT(sih);

    /* Install TLB */
    tlb_map_init();
#ifdef INCLUDE_PCI
    /* this routine is also responsible for resetting any devices that may
       currently be in an active state and sets up the interrupt mappings */
    pciConfigLibInit(0,0,0,0);

#ifdef BROADCOM_BSP
    /* Small delay to allow PCI devices on bus 1 to be discovered */
    sysUSecDelay(20000);
#endif

#endif

#if EXCEPTION_DEBUG 
    for(i=1; i<=31; i++)
        intVecSet ((FUNCPTR *) INUM_TO_IVEC(i), (FUNCPTR) excExcHandleDBG);
    for(i=54; i<=59; i++)
        intVecSet ((FUNCPTR *) INUM_TO_IVEC(i), (FUNCPTR) excExcHandleDBG);
#endif

    /* connect dummy isr's to shared int vectors */
    for(i=IV_IORQ0_VEC; i<IV_LAST_VEC; i++)
        intVecSet ((FUNCPTR *) INUM_TO_IVEC(i), (FUNCPTR) NULL);

    /* Setup the serial device descriptors. */
    sysSerialHwInit(); 

    /*
    *  Send XON character in case power-off noise from UART has
    *  sent an XOFF
    */
    xonString[0] = (char) 0x11;
    xonString[1] = '\n';
    xonString[2] = '\0';

    sysSerialPrintString(xonString);
    
    pciIntLibInit();

    /* connect and enable timer int */
    (void) intConnect (INUM_TO_IVEC(IV_RTIME_VEC), sysClkInt, 0); 

    /* Connect and enable "shared" mips int 0 handler */
    (void) intConnect (INUM_TO_IVEC(IV_IORQ0_VEC), sharedInt0, 0);

    /* get the cc pointer */
    cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
    ASSERT(cc);
    
    /* connect the cc interrupt handler */
    pciConfigInByte(0, si_coreidx(sih), 0, PCI_CFG_DEV_INT_LINE, &irq);
    /* Figure out the correct vector for this interrupt */

    if (irq == 0) {
        /* Assign to shared interrupt vector */
        pciConfigInByte(0, si_coreidx(sih), 0, PCI_CFG_DEV_INT_PIN, &flag);
        irq = IV_IORQ0_BIT0_VEC + flag;
    } else {
        /* Assign to regular interrupt vector */
        irq += IV_IORQ0_VEC;
    }
    (void) intConnect(INUM_TO_IVEC(irq), sysExtInt, 0);

    /* connect serial channel interrupts */
    sysSerialHwInit2();

    pciConfigFixup();
    sysPciAutoConfig();

#ifndef BUILD_VXBOOT
    /* For using CFE API (mainly for getting env vars) */
    prom_init();
#endif

#ifdef INCLUDE_I2C_DEV
    if (ksi2c_init() == KSI2C_ERR_NONE) {
        ksi2c_open(0, KS_I2C_BUS_PIO, KS_I2C_SPEED_DEFAULT);
        m41t81_tod_init(); 
        usrTimeSync();
    }
#endif

#ifdef INCLUDE_PCI_TEST
      sysSil31xxPciInit();
#endif

    Bcm53000HiMemSize = BCM53000_HIMEM_SIZE-BCM53000_SDRAM_SZ;
    Bcm53000HiMemAddr = SI_TLB_SDRAM_VIRT_CACHED+BCM53000_SDRAM_SZ;

    intSRSet(BCM53000_SR);

    flashDrvLibInit();

}

/***************************************************************************
*
* sysPhysMemTop - get the address of the top of memory
*
* This routine returns the address of the first missing byte of memory, which
* indicates the top of memory.
*
* The current algorithm detects the top of memory by making memory
* probes with wraparound checks.  This is inherently a bit risky because
* there is a possibility that the probe will affect the executing code.
* This really needs to be written in assembler because it's not possible
* to predict how the compiler will generate the loads used to access memory
* which can lead to problems with stray values hanging around on the bus for
* non-existent memory.
*
* NOTE: Do not adjust LOCAL_MEM_SIZE to reserve memory for application
* use.  See sysMemTop() for more information on reserving memory.
*
* RETURNS: The address of the top of memory.
*/

char *sysPhysMemTop (void)
    {
#ifdef LOCAL_MEM_AUTOSIZE
#error
    struct toggle {
    UINT a, b, c, d;
    };

        UINT32 i;
    volatile struct toggle *pTest, *wTest;
    UINT32 adrsSpace;

#define PROBE_SIZE  MIN_PHYS_MEMORY

    struct toggle saves[(MAX_PHYS_MEMORY/PROBE_SIZE)+1][2];
    struct toggle cheq55, cheqAA;
    register int AA;
    int w, p, mp=0;
    UINT oldSr;
    int base;
    extern char end[];
    int finished=0;

    if (sysMemTopAdr == NULL || sysMemTopDebug) 
        {
    AA = 0xaaaaaaaa;
    cheqAA.a = cheqAA.b = cheqAA.c = cheqAA.d = 0xaaaaaaaa;
    cheq55.a = cheq55.b = cheq55.c = cheq55.d = 0x55555555;

    base = (((int)end & 0x1fffffff) + PROBE_SIZE - 1) & ~(PROBE_SIZE-1);
        
    /* disable parity errors */

    oldSr = intSRGet ();
    intSRSet (oldSr | SR_DE);

    if (sysMemTopDebug)
        printf ("Stashing copies: ");

    /* stash away copies of the locations to be tested */

    for (p = base; p <= MAX_PHYS_MEMORY; p += PROBE_SIZE)
        {
        pTest = ((volatile struct toggle *)(PHYS_TO_K1(p))) - 2;

        /* save two sets of 128 bits */

        if (sysMemTopDebug)
        printf ("0x%08x ", (unsigned int)pTest);

        saves[p/PROBE_SIZE][0].a = pTest[0].a;
        saves[p/PROBE_SIZE][0].b = pTest[0].b;
        saves[p/PROBE_SIZE][0].c = pTest[0].c;
        saves[p/PROBE_SIZE][0].d = pTest[0].d;
        saves[p/PROBE_SIZE][1].a = pTest[1].a;
        saves[p/PROBE_SIZE][1].b = pTest[1].b;
        saves[p/PROBE_SIZE][1].c = pTest[1].c;
        saves[p/PROBE_SIZE][1].d = pTest[1].d;
        }
    if (sysMemTopDebug)
        printf ("\n");

    if (sysMemTopDebug)
        printf ("Checking:\n");

    for (p = base; p <= MAX_PHYS_MEMORY; p += PROBE_SIZE)
        {
        pTest = ((volatile struct toggle *)(PHYS_TO_K1(p))) - 2;

        if (sysMemTopDebug)
                printf ("0x%08x(%08x): setting checkerboard\n",
                       p, (unsigned int)pTest);

            /* Store a checkerboard value in the first 128 bits of memory */

        pTest[0] = cheqAA;

        /* Store the inverse in the next 128 bits to trash the bus */

        pTest[1] = cheq55;

        /* check to see if the original value stayed there */

        finished = (pTest[0].a != AA || 
            pTest[0].b != AA || 
            pTest[0].c != AA || 
            pTest[0].d != AA);

        if (sysMemTopDebug && finished)
                printf ("0x%08x: checkerboard failed\n", p);

        if (finished)
        break;

        /* check to see if it wrapped */

        for (w = base; w < p; w += PROBE_SIZE)
        {
        wTest = ((volatile struct toggle *)(PHYS_TO_K1(w))) - 2;
        finished = (memcmp (&saves[w/PROBE_SIZE][0], (char *)&wTest[0], 
                   sizeof(struct toggle)) != 0) ||
                   (memcmp (&saves[w/PROBE_SIZE][1], (char *)&wTest[1],
                   sizeof(struct toggle)) != 0);
        if (sysMemTopDebug && finished)
            printf ("0x%08x: failed comparison at 0x%08x(%08x)\n",
                               p, w, (unsigned int)wTest);
        if (finished)
            break;
        }
        if (finished)
        break;
        if (sysMemTopDebug)
        printf ("0x%08x(%08x): restoring state\n", p, (unsigned int)pTest);
        pTest[0] = saves[p/PROBE_SIZE][0];
        pTest[1] = saves[p/PROBE_SIZE][1];
        mp = p;
        }

    for (p = base; p <= MAX_PHYS_MEMORY; p += PROBE_SIZE)
        {
        pTest = ((volatile struct toggle *)(PHYS_TO_K1(p))) - 2;

        /* restore two sets of 128 bits */

        pTest[0] = saves[p/PROBE_SIZE][0];
        pTest[1] = saves[p/PROBE_SIZE][1];
        }

    intSRSet (oldSr);

    if (!finished || mp == 0)
        {
        strcpy (EXC_MSG_ADRS, "No memory detected");
        sysToMonitor (BOOT_WARM_NO_AUTOBOOT);
        }

    /*
     * this calculation makes it unnecessary to know at compile-time
     * whether the kernel is mapped or not.
     */
    adrsSpace = (VIRT_ADDR)sysPhysMemTop & ~ADDRESS_SPACE_MASK;
    sysMemTopAdr = (char *)(adrsSpace +
                ((VIRT_ADDR)LOCAL_MEM_LOCAL_ADRS &
                 ADDRESS_SPACE_MASK) +
                mp);

    return (sysMemTopAdr);
#else /*LOCAL_MEM_AUTOSIZE*/

  static char * memTop = NULL;

    if (memTop == NULL)
    {
    memTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
    }
    return memTop;

#endif /*LOCAL_MEM_AUTOSIZE*/
    }

/***************************************************************************
*
* sysMemTop - get the address of the top of logical memory
*
* This routine returns the address of the first unusable byte of memory.
* VxWorks does not use any memory at or above this address.
*
* The user can reserve local memory from the board by declaring the
* macro USER_RESERVED_MEM with the amount of memory to reserve. This
* routine returns a pointer to the first byte of the reserved memory
* area.
*
* RETURNS: The address of the top of usable memory.
*/

char *sysMemTop (void)
    {
    static char * memTop = NULL;

    if (memTop == NULL)
        {
        memTop = sysPhysMemTop () - USER_RESERVED_MEM;

#ifdef INCLUDE_EDR_PM
        /* account for ED&R persistent memory */
        memTop = memTop - PM_RESERVED_MEM;
#endif
        }

    return memTop;
    }

/***************************************************************************
*
* sysCacheInit - initialize the MIPS cache library
*
* Initializes the generic cache library to work with the MIPS processor.
*
* RETURNS: OK or ERROR
*
* NOMANUAL
*/

#if defined (ROM_AUTO_CACHE_DETECT)
LOCAL int cacheSize[] = { 64, 128, 256, 512, 1024, 2048, 4096, 0 };
LOCAL int cacheLineSize[] = { 0, 4, 8, 16, 32, 64, 128, 0 };
LOCAL int cacheAssoc[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
#endif


LOCAL STATUS sysCacheInit
    (
    CACHE_MODE instMode,
    CACHE_MODE dataMode
    )
    {
#if defined (ROM_AUTO_CACHE_DETECT)

    UINT32 config1 = sysConfig1Get();
    int iCacheSize, iCacheLineSize, iCacheAssoc;
    int dCacheSize, dCacheLineSize, dCacheAssoc;
    
    iCacheSize =
      cacheSize[((config1 & CONF1_IS_MASK) >> CONF1_IS_SHIFT)];
    iCacheLineSize =
      cacheLineSize[((config1 & CONF1_IL_MASK) >> CONF1_IL_SHIFT)];
    iCacheAssoc =
      cacheAssoc[((config1 & CONF1_IA_MASK) >> CONF1_IA_SHIFT)];
    iCacheSize *= (iCacheLineSize * iCacheAssoc);

    dCacheSize =
      cacheSize[((config1 & CONF1_DS_MASK) >> CONF1_DS_SHIFT)];
    dCacheLineSize =
      cacheLineSize[((config1 & CONF1_DL_MASK) >> CONF1_DL_SHIFT)];
    dCacheAssoc =
      cacheAssoc[((config1 & CONF1_DA_MASK) >> CONF1_DA_SHIFT)];
    dCacheSize *= (dCacheLineSize * dCacheAssoc);

    return cache4kcLibInit(
	instMode,	/* I-cache mode */
	dataMode,	/* D-cache mode */
	iCacheSize,
	iCacheLineSize,
	dCacheSize,
	dCacheLineSize
	);
#else
	instMode,	/* I-cache mode */
	dataMode,	/* D-cache mode */
	ICACHE_SIZE,
	ICACHE_LINE_SIZE,
	DCACHE_SIZE,
	DCACHE_LINE_SIZE
	);
#endif /* AUTO_CACHE_DETECT */
    }


/***************************************************************************
*
* sysClearTlb - clear the translation lookaside buffer
*
* This routine clears the entries in the translation lookaside buffer (TLB)
* for the MIPS CPU.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void sysClearTlb (void)
    {
    mipsTlbClear();
    }


#ifdef BROADCOM_BSP
/******************************************************************************
*
* sysBacktraceMips - perform a stack trace of the caller with symbols
*               by Curt McDowell - csm@broadcom.com 03-20-01
*
* NOTE:
*
* This routine is specific to MIPS and GCC stack frame generation.
* Each function generated by the compiler must begin with a decrement
* of the stack pointer, then in the next few instructions, must save
* the return address on the stack.
*
*   27bdffd0    addiu       $sp,$sp,-48
*               ...
*   afbf0028    sw          $ra,40($sp)
*               ...
*/

/* Hack; some day fix to locate function start using symFindByValue */
#define SYSBT_MAX_FUNC_WORDS    4096

LOCAL void sysbt_addr2sym(char *symName, UINT addr)
{
    extern SYMTAB_ID    sysSymTbl;
    int                 symVal;
    SYM_TYPE            symType;

    if (symFindByValue(sysSymTbl,
                       (UINT) addr,
                       symName, &symVal, &symType) == ERROR)
        sprintf(symName, "0x%x", addr);
    else if (symVal < addr)
        sprintf(symName + strlen(symName),
                " + 0x%x", addr - symVal);
    else if (symVal > addr)
        sprintf(symName + strlen(symName),
                " - 0x%x", symVal - addr);
}

#define VALID_K0(addr) \
        (IS_KSEG0(addr) && K0_TO_PHYS(addr) < K0_TO_PHYS(sysPhysMemTop()))
#define VALID_K1(addr) \
        (IS_KSEG1(addr) && K1_TO_PHYS(addr) < K0_TO_PHYS(sysPhysMemTop()))
#define VALID_ADDR(addr) \
        (VALID_K0(addr) || VALID_K1(addr))
#define VALID_TEXT_ADDR(addr) \
        VALID_ADDR(addr)        /* some day fix for text range */
#define VALID_STACK_ADDR(addr) \
        VALID_ADDR(addr)        /* some day fix for stack range */

extern UINT         *vxSpGet(), *vxPcGet();         /* sysALib.s */

void sysBacktraceMips(char *pfx, char *sfx, int direct)
{
    UINT                *sp, *pc, *fn_start, *caller_sp, *ra;
    int                 limit, first = 1;
    char                buf[2048], symName[MAX_SYS_SYM_LEN+1], *s;
    int                 ra_off, sp_off;
    extern void         sysSerialPrintString(char *s);

    sp = vxSpGet();
    pc = vxPcGet();

    s = buf;

    strcpy(s, pfx);
    while (*s) s++;

    while (s < &buf[sizeof (buf) - 128]) {
        fn_start = pc;
        for (limit = 0; limit < SYSBT_MAX_FUNC_WORDS; limit++) {
            /* sw $ra,x($sp); x>=0 */
            if ((*--fn_start & 0xffff8000) == 0xafbf0000)
                break;
        }
        if (limit == SYSBT_MAX_FUNC_WORDS)
            break;
        ra_off = (*fn_start & 0x7fff);
        for (limit = 0; limit < SYSBT_MAX_FUNC_WORDS; limit++) {
            /* addiu $sp,$sp,-x; x<0 */
            if ((*--fn_start & 0xffff8000) == 0x27bd8000)
                break;
        }
        if (limit == SYSBT_MAX_FUNC_WORDS)
            break;
        sp_off = (int) (*fn_start | 0xffff0000);
        ra = (UINT *) sp[ra_off / 4];
        caller_sp = sp - (sp_off / 4);
        sysbt_addr2sym(symName, (UINT) pc);
        if (! first)
            sprintf(s,
                    "FUNC = 0x%x, PC = 0x%x (%s), SP = 0x%x\n",
                    (UINT) fn_start, (UINT) pc, symName, (UINT) sp);
        while (*s) s++;
        if (! VALID_TEXT_ADDR(ra)) {
            sprintf(s, "RA 0x%x out of range\n", (UINT) ra);
            while (*s) s++;
            break;
        }
        if (! VALID_STACK_ADDR(caller_sp)) {
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
        sysSerialPrintString(buf);      /* Atomically print giant string */
    else {
        fputs(buf, stdout);             /* Regular print giant string */
        fflush(stdout);
    }
}

#endif /* BROADCOM_BSP */

extern UINT32 ReadEPC();
extern UINT32 ReadBVA();
extern UINT32 ReadERRPC();
#ifndef BROADCOM_BSP
extern UINT32 vxSpGet();
#endif

void
sysPrintCPURegs(char *a)
{
    int key;
    key = intLock ();

    sysSerialPrintString(a);
#define  PRINT(x) sysSerialPrintString(#x);sysSerialPrintHex((UINT32) x(), 1)
    PRINT(sysCountGet);
    PRINT(sysCompareGet);
    PRINT(intCRGet);
    PRINT(ReadEPC);
    PRINT(ReadBVA);
    PRINT(ReadERRPC);
    PRINT(vxSpGet);
    sysSerialPrintString("SR = 0x");sysSerialPrintHex(key, 1);
    sysSerialPrintString("=========\n");
#undef PRINT
    intUnlock (key);
}

/***************************************************************************
*
* sysToMonitor - transfer control to the ROM monitor
*
* This routine transfers control to the ROM monitor.  Normally, it is called
* only by reboot()--which services ^X--and bus errors at interrupt level.
* However, in some circumstances, the user may wish to introduce a
* <startType> to enable special boot ROM facilities.
*
* RETURNS: Does not return.
*/

int sysToMonitorExcMessage = 0;
int sysToMonitorBacktrace = 1;
int sysToMonitorReboot = 1;
void (*sysToMonitorHook)(void);

int enable_sys_serial = 1;


STATUS sysToMonitor
    (
    int startType    /* parameter passed to ROM to tell it how to boot */
    )
    {
    enable_sys_serial = 1;
    sysPrintCPURegs("sysToMonitor\n");

    if (sysToMonitorHook)
        (*sysToMonitorHook)();

#ifdef BROADCOM_BSP
    if (sysToMonitorBacktrace)
        sysBacktraceMips("\n--- Stack Trace ---\n", "", 1);
#endif

    if (sysToMonitorReboot) {
#ifdef BROADCOM_BSP
        sysReboot();
#else
        FUNCPTR pRom = (FUNCPTR) (ROM_WARM_ADRS);
        intLock();
        /* no addition work necessary for the PRO100B */
        (*pRom) (startType);
#endif
    }

    return (OK);    /* in case we ever continue from rom monitor */
    }

/* routine to return the mips clock speed (discovered during init ) */
uint32 sysGetMIPSClk(void) {
    ASSERT(mipsclk);
    return(mipsclk);
}

/* routine to return the si clock speed (discovered during init ) */
uint32 sysGetSIClk(void) {
    ASSERT(mipsclk);
    return(mipsclk/4);
}

/* routine to return the ILP clock */
uint32 sysGetILPClk(void) {
    return(6250000); /* 6.25MHz */
}

void sysMipsDelay(uint32 us)
{
    uint32 n=0,x=0,y=0;
    ASSERT(mipsclk);

    n = (mipsclk/2000000)*us; /* mips count = MHz/2, us = 1/1000000 */
    x = readCount();
    y = readCount();

    /* check for wrap */
    if (x > (x+n)) 
        while (y >= x) 
            y = readCount();
        
    while (y < (x+n)) 
        y = readCount();
}

/*
 * Returns value to compensate and improve  sal_time_usecs timer precesion
 */
unsigned int sys_sal_time_usecs_comp(void)
{
    uint32 n, us, mclk_khz_mul10;
    int key;
    mclk_khz_mul10 = (mipsclk / 20000);
    key = intLock ();
    n = readCount() - (sysSoftCompare - sysClkProcessorTicks);
    intUnlock (key);
    us = n * 100 / mclk_khz_mul10;
    return (us);
}

void
sysPlatformInit()
{
    char *wd;

    sih = si_kattach(SI_OSH);
    ASSERT(sih);
    
    si_mips_init(sih, 0);

    /* Initialize NVRAM access */
    nvram_init((void *)sih);

    /* get broadcom board type */
    boardtype = sih->boardtype;

    /* Read the watchdog variable (set in ms) */
    wd = nvram_get("watchdog"); 
    if (wd) 
        wd_ms = bcm_atoi(wd) * WATCHDOG_CLOCK/1000;

    mipsclk = si_cpu_clock(sih);
}


void reset_watchdog(uint32 n)
{
        ASSERT(sih);
        si_watchdog(sih, n);
}

/*****************************************************************************
*
* sysClkEnable - turn on system clock interrupts
*
* This routine enables system clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysClkDisable(), sysClkRateSet()
*/

void sysClkEnable (void)
    {
    /* start the timer */
    int key;
    sysClkProcessorTicks = (sysGetMIPSClk()/sysClkTicksPerSecond)/2;

    key = intLock ();
    MTC0(C0_COMPARE, 0, 0);
    MTC0(C0_COUNT, 0, 1);
    MTC0(C0_COMPARE, 0, sysClkProcessorTicks);
    sysSoftCompare = sysClkProcessorTicks;
    sysClkRunning = TRUE;
    intUnlock (key);

    }

/*****************************************************************************
*
* sysClkRateGet - get the system clock rate
*
* This routine returns the interrupt rate of the system clock.
*
* RETURNS: The number of ticks per second of the system clock.
*
* SEE ALSO: sysClkRateSet()
*/

int sysClkRateGet (void)
    {
    return (sysClkTicksPerSecond);
    }

/*****************************************************************************
*
* sysClkRateSet - set the system clock rate
*
* This routine sets the interrupt rate of the system clock.  It does not
* enable system clock interrupts.  Normally, it is called by usrRoot() in
* usrConfig.c.
*
* NOTE
* The RC32364 internal timer is used to provide the system clock.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be
* set.
*
* SEE ALSO: sysClkDisable(), sysClkEnable(), sysClkRateGet()
*/

STATUS sysClkRateSet
    (
    int ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    if (ticksPerSecond < SYS_CLK_RATE_MIN || ticksPerSecond > SYS_CLK_RATE_MAX)
    return (ERROR);

    sysClkTicksPerSecond = ticksPerSecond;

    if (sysClkRunning)
    {
    sysClkDisable ();
    sysClkEnable ();
    }

    return (OK);
    }

/*****************************************************************************
*
* sysClkInt - handle a system clock interrupt
*
* This routine handles a system clock interrupt.  
*
* RETURNS: N/A
*/
extern int sys_clk_bp0;
extern int sys_clk_bp1;
extern int sys_clk_bp2;
extern int sys_clk_bp3;
extern int sys_sr_0;
extern int sys_sr_1;


LOCAL void sysClkInt (void)
{
    int key;
    ULONG delta;        /* time to when next interrupt should be */

    /* Reset the watchdog */
    reset_watchdog(wd_ms);
 
    /* see sysClkDisable(). Clear interrupt and return. */
    if (!sysClkRunning)
    {
    MTC0(C0_COMPARE, 0, sysSoftCompare);
    return;
    }

 /*sys_clk_bp0++;*/
/* sys_sr_0 = C0_SR_get();*/
    key = intLock();

    if (sysSoftCount31_0 < 0) {
        sysSoftCount31_0 = MFC0(C0_COUNT, 0);
        if (sysSoftCount31_0 > 0) {
            sysSoftCount63_32++;
        }
    } else {
        sysSoftCount31_0 = MFC0(C0_COUNT, 0);
    }

    /* use unsigned arithmetic to compensate for wrap-around */

    /* delta is how long ago the interrupt occured. */
    delta = MFC0(C0_COUNT, 0) - sysSoftCompare;

    if (delta < (sysClkProcessorTicks - 200))
    {
        /*
         * case 1 (normal): we haven't missed the next interrupt.
         *  The 200 above is an estimate of how far count will advance
         *  between reading it above and setting the compare register
         *  below (count should not pass the new compare value before
         *  we've set the register).  sysClkProcessorTicks is on
         *  the order of 500,000.
         */
        sysSoftCompare += sysClkProcessorTicks;
        MTC0(C0_COMPARE, 0, sysSoftCompare);
    }
    else if (delta < (16 * sysClkProcessorTicks))
    {
        /*
         * case 2 (missed a couple): we've missed some interrupts,
         *   don't set the compare register (so we'll take another
         *   interrupt as soon as we return).
         */
        sysSoftCompare += sysClkProcessorTicks;
    }
    else
    {
        /*
         * case 3 (missed a lot): set the next interrupt at
         *   sysClkProcessorTicks past the current time.
         */
        sysSoftCompare = MFC0(C0_COUNT, 0) + sysClkProcessorTicks;
        MTC0(C0_COMPARE, 0, sysSoftCompare);
    }

    intUnlock (key);

    if (sysClkRoutine != NULL) {
    (*sysClkRoutine) (sysClkArg);
    }
    }

/*****************************************************************************
*
* sysClkConnect - connect a routine to the system clock interrupt
*
* This routine specifies the interrupt handler to be called at each system
* clock interrupt.  It does not enable system clock interrupts.  Normally,
* it is called from usrRoot() in usrConfig.c to connect usrClock() to the
* system clock interrupt.  Other standard interrupt handlers are also set up
* at this time.
*
* RETURN: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), usrClock()
*/

STATUS sysClkConnect
    (
    FUNCPTR routine,    /* routine called at each system clock interrupt */
    int arg             /* argument with which to call routine           */
    )
    {
    if (sysClkConnected == FALSE)
    {
    sysHwInit2 ();

    sysClkConnected = TRUE;
    }

    sysClkRoutine   = routine;
    sysClkArg       = arg;

    return (OK);
    }

/*****************************************************************************
*
* sysClkDisable - turn off system clock interrupts
*
* This routine disables system clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysClkEnable(), sysClkRateSet()
*/

void sysClkDisable (void)
    {
    int key;
    
    /* we cannot disable interrupts so we do this instead... */

    key = intLock ();
    MTC0(C0_COMPARE, 0, 0);
    MTC0(C0_COUNT, 0, 1);
    sysClkProcessorTicks = 0xffffff00;
    MTC0(C0_COMPARE, 0, sysClkProcessorTicks);
    sysSoftCompare = sysClkProcessorTicks;
    sysClkRunning = FALSE;
    intUnlock (key);
    }

#if defined(AUXCLK) || defined(INCLUDE_AUX_CLK) /* AUX clock */
/*******************************************************************************
*
* sysAuxClkInt - interrupt level processing for auxiliary clock
*
* This routine handles the auxiliary clock interrupt.  It is attached to the
* clock interrupt vector by the routine sysAuxClkConnect().
* The appropriate routine is called and the interrupt is acknowleged.
*/

LOCAL void sysAuxClkInt (void)
{
    uint32 intstatus;
    uint32 temp;

    intstatus = R_REG(NULL, &cc->intstatus) & mptimer_int;
    if (intstatus) {
        /* clear the timer interrupt */
        temp = R_REG(NULL, &cc->MPcnt_status);
        intstatus >>= CI_MP_CNT_SIFT;
        temp &= ~intstatus;
        W_REG(NULL, &cc->MPcnt_status, temp);
        
        if (sysAuxClkRoutine != NULL)
        (*sysAuxClkRoutine) (sysAuxClkArg);     
    }
}

/*******************************************************************************
*
* sysAuxClkConnect - connect a routine to the auxiliary clock interrupt
*
* This routine specifies the interrupt service routine to be called at each
* auxiliary clock interrupt.  It does not enable auxiliary clock
* interrupts.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), sysAuxClkEnable()
*/

STATUS sysAuxClkConnect
    (
    FUNCPTR routine,    /*routine called at each aux clock interrupt   */    
    int arg             /*argument to auxiliary clock interrupt routine*/ 
    )
    {

    if (!sysAuxClkConnected)
        {
    sysAuxClkConnected = TRUE;
        (void) intConnect (INUM_TO_IVEC(IV_IORQ0_BIT1_VEC), sysAuxClkInt, 0);
        }

    sysAuxClkRoutine = routine;
    sysAuxClkArg = arg;

    return (OK);
    }

/*******************************************************************************
*
* sysAuxClkDisable - turn off auxiliary clock interrupts
*
* This routine disables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkEnable()
*/

void sysAuxClkDisable (void)
    {
      uint32 intstatus;

    /* write a zero to disable the timer */
      switch(mptimer_id) {
        case 3:
            W_REG(NULL,  &cc->MPcnt3, 0);
            break;        	
        case 2:
            W_REG(NULL,  &cc->MPcnt2, 0);
            break;        	
        case 1:
            W_REG(NULL,  &cc->MPcnt1, 0);
            break;        	
        case 0:
        default:
            W_REG(NULL,  &cc->MPcnt0, 0);
            break;        	
      }

    /* disable interrupts */
      intstatus = R_REG(NULL, &cc->intmask);
      intstatus &= ~mptimer_int;
      W_REG(NULL,  &cc->intmask, intstatus);
      
    sysAuxClkRunning = FALSE ;
    }

/*******************************************************************************
*
* sysAuxClkEnable - turn on auxiliary clock interrupts
*
* This routine enables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkConnect(), sysAuxClkDisable(), sysAuxClkRateSet()
*/

void sysAuxClkEnable (void)
    {
    uint32 intstatus;
    if (cc == NULL) {
        printf("No core was selected to provide a timer\n");
        printf("Use set_auxtimer(base_addr) to select a core.\n");
        sysAuxClkRunning = FALSE ;
        return;
        }

    /* write to the timer */
    switch(mptimer_id) {
        case 3:
            W_REG(NULL,  &cc->MPcnt3, sysGetILPClk()/sysAuxClkTicksPerSecond);
            break;        	
        case 2:
            W_REG(NULL,  &cc->MPcnt2, sysGetILPClk()/sysAuxClkTicksPerSecond);
            break;        	
        case 1:
            W_REG(NULL,  &cc->MPcnt1, sysGetILPClk()/sysAuxClkTicksPerSecond);
            break;        	
        case 0:
        default:
            W_REG(NULL,  &cc->MPcnt0, sysGetILPClk()/sysAuxClkTicksPerSecond);
            break;        	
    }

    /* enable timer interrupt */
    intstatus = R_REG(NULL, &cc->intmask);
    intstatus |= mptimer_int;
    W_REG(NULL,  &cc->intmask, intstatus);

    sysAuxClkRunning = TRUE ;
    }

void set_auxtimer( uint32 id )
{
    switch(id) {
        case 3:
            mptimer_int = CI_MP_CNT_3;
            break;        	
        case 2:
            mptimer_int = CI_MP_CNT_2;
            break;        	
        case 1:
            mptimer_int = CI_MP_CNT_1;
            break;        	
        case 0:
        default:
            mptimer_int = CI_MP_CNT_0;
            break;        	
    }
    mptimer_id = id;
}

void show_auxtimer( void ) {
    printf("Aux timer uses multipurpose timer#%d\n", mptimer_id); 
}

/*******************************************************************************
*
* sysAuxClkRateGet - get the auxiliary clock rate
*
* This routine returns the interrupt rate of the auxiliary clock.
*
* RETURNS: The number of ticks per second of the auxiliary clock.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateSet()
*/

int sysAuxClkRateGet (void)
    {
    return (sysAuxClkTicksPerSecond);
    }

/*******************************************************************************
*
* sysAuxClkRateSet - set the auxiliary clock rate
*
* This routine sets the interrupt rate of the auxiliary clock.
* It does not enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be set.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateGet()
*/

STATUS sysAuxClkRateSet
    (
    int ticksPerSecond     /* number of clock interrupts per second*/ 
    )
    {
     if( ticksPerSecond < AUX_CLK_RATE_MIN || ticksPerSecond > AUX_CLK_RATE_MAX)
       {
    return (ERROR);
       }
     sysAuxClkTicksPerSecond = ticksPerSecond ;
     
     if ( sysAuxClkRunning )
    {
      sysAuxClkDisable();
      sysAuxClkEnable();
    }
    
      return (OK);
    }

#endif

#ifdef  INCLUDE_TIMESTAMP
     /* Timestamp routines derived from src\drv\timer\mipsR4kTimer.c */

/*******************************************************************************
*
* sysTimestampConnect - connect a user routine to a timestamp timer interrupt
*
* This routine specifies the user interrupt routine to be called at each
* timestamp timer interrupt.
*
* RETURNS: ERROR, always.
*/

STATUS sysTimestampConnect
    (
    FUNCPTR routine,    /* routine called at each timestamp timer interrupt */
    int arg             /* argument with which to call routine */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysTimestampEnable - enable a timestamp timer interrupt
*
* This routine enables timestamp timer interrupts and resets the counter.
*
* RETURNS: OK, always.
*
* SEE ALSO: sysTimestampDisable()
*/

STATUS sysTimestampEnable (void)
   {
   if (sysTimestampRunning)
      {
      return (OK);
      }

   if (!sysClkRunning)
      return (ERROR);

   sysTimestampRunning = TRUE;

   return (OK);
   }

/*******************************************************************************
*
* sysTimestampDisable - disable a timestamp timer interrupt
*
* This routine disables timestamp timer interrupts.
*
* RETURNS: OK, always.
*
* SEE ALSO: sysTimestampEnable()
*/

STATUS sysTimestampDisable (void)
    {
    if (sysTimestampRunning)
        sysTimestampRunning = FALSE;

    return (OK);
    }

/*******************************************************************************
*
* sysTimestampPeriod - get the period of a timestamp timer
*
* This routine gets the period of the timestamp timer, in ticks.  The
* period, or terminal count, is the number of ticks to which the timestamp
* timer counts before rolling over and restarting the counting process.
*
* RETURNS: The period of the timestamp timer in counter ticks.
*/

UINT32 sysTimestampPeriod (void)
    {
    /*
     * The period of the timestamp depends on the clock rate of the on-chip
     * timer.
     */

    return (sysGetMIPSClk()/sysClkTicksPerSecond)/2;
    }

/*******************************************************************************
*
* sysTimestampFreq - get a timestamp timer clock frequency
*
* This routine gets the frequency of the timer clock, in ticks per
* second.  The rate of the timestamp timer is set explicitly by the
* hardware and typically cannot be altered.
*
* RETURNS: The timestamp timer clock frequency, in ticks per second.
*/

UINT32 sysTimestampFreq (void)
    {
    return (sysGetMIPSClk()/2);
    }

/*******************************************************************************
*
* sysTimestamp - get a timestamp timer tick count
*
* This routine returns the current value of the timestamp timer tick counter.
* The tick count can be converted to seconds by dividing it by the return of
* sysTimestampFreq().
*
* This routine should be called with interrupts locked.  If interrupts are
* not locked, sysTimestampLock() should be used instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestampFreq(), sysTimestampLock()
*/

UINT32 sysTimestamp (void)
    {
    return (MFC0(C0_COUNT, 0) - (sysSoftCompare - sysClkProcessorTicks) + 1);
    }

/*******************************************************************************
*
* sysTimestampLock - lock interrupts and get the timestamp timer tick count
*
* This routine locks interrupts when the tick counter must be stopped
* in order to read it or when two independent counters must be read.
* It then returns the current value of the timestamp timer tick
* counter.
*
* The tick count can be converted to seconds by dividing it by the return of
* sysTimestampFreq().
*
* If interrupts are already locked, sysTimestamp() should be
* used instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestampFreq(), sysTimestamp()
*/

UINT32 sysTimestampLock (void)
    {
    return(MFC0(C0_COUNT, 0) - (sysSoftCompare - sysClkProcessorTicks)+ 1);
    }

#endif  /* INCLUDE_TIMESTAMP */

/******************************************************************************
*
* sysNvRamGet - get the contents of non-volatile RAM (starting at sector 0)
*
* This routine copies the contents of non-volatile memory into a specified
* string.  The string is terminated with an EOS.
*
*
* RETURNS: ERROR.
*
* SEE ALSO: sysNvRamSet()
*/
STATUS sysNvRamGet
    (
    char *string,    /* where to copy non-volatile RAM    */
    int strLen,      /* maximum number of bytes to copy   */
    int offset       /* byte offset into non-volatile RAM */
    )
    {
         char *buf;
      
         if ((offset < 0) || (strLen < 0) || 
             (offset + strLen > FLASH_NVRAM_SIZE))    
             return (ERROR);     
      
         /* now allocate buffers and read sectors */     
         buf = malloc(FLASH_NVRAM_SIZE);     
         if (buf == NULL)     
             return (ERROR);     
      
         if (flashBlkRead(FLASH_NVRAM_START_SECTOR, buf,     
                          FLASH_SECTOR_SIZE - FLASH_NVRAM_SIZE,     
                          FLASH_NVRAM_SIZE) == ERROR) {     
             free(buf);     
             return(ERROR);     
         }     
      
         memcpy(string, buf + offset, strLen);     
         string[strLen] = 0;

         free(buf);  

        return (OK);
    }

/*******************************************************************************
*
* sysNvRamSet - write to non-volatile RAM
*
* This routine copies a specified string into non-volatile RAM.
* (starting at sector 0
*
*
* RETURNS: ERROR.
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysNvRamSet
    (
    char *string,     /* string to be copied into non-volatile RAM */
    int strLen,       /* maximum number of bytes to copy           */
    int offset        /* byte offset into non-volatile RAM         */
    )
    {
         char *buf;
      
         if ((offset < 0) || (strLen < 0) || 
             (offset + strLen > FLASH_NVRAM_SIZE))    
             return (ERROR);     
      
         /* now allocate buffers and read sectors */     
         buf = malloc(FLASH_NVRAM_SIZE);     
         if (buf == NULL)     
             return (ERROR);     
      
         if (flashBlkRead(FLASH_NVRAM_START_SECTOR, buf,     
                          FLASH_SECTOR_SIZE - FLASH_NVRAM_SIZE,     
                          FLASH_NVRAM_SIZE) == ERROR) {     
             free(buf);     
             return(ERROR);     
         }     
      
         memcpy(buf + offset, string, strLen);     
      
         if (flashBlkWrite(FLASH_NVRAM_START_SECTOR, buf,     
                          FLASH_SECTOR_SIZE - FLASH_NVRAM_SIZE,     
                          FLASH_NVRAM_SIZE) == ERROR) {     
             free(buf);     
             return(ERROR);     
         }     
         flashSyncFilesystem();     
              
         free(buf);     

        return (OK);
    }


/***************************************************************************
*
* sysAutoAck - acknowledge the MIPS interrupt condition 
*
* This routine acknowledges an MIPS interrupt for a specified interrupt
* vector.
*
* NOTE:
* This routine must be provided on all MIPS board support packages.
* Most interrupts are automatically acknowledged in the interrupt service
* routine.
*
* RETURNS: The result of the interrupt acknowledge cycle.
*/
int sysAutoAck
    (
    int vecNum      /* vector num of interrupt that bugs us */
    )
    {
    int result;

    result = 0;
    switch (vecNum)
    {
    case IV_RTIME_VEC:
        MTC0(C0_COMPARE, 0, 0);
        break;
    case IV_SWTRAP0_VEC:        /* software trap 0 */
        return(result = sysSw0Ack ());
        break;
    case IV_SWTRAP1_VEC:        /* software trap 1 */
        return(result = sysSw1Ack ());
        break;
    case IV_FPA_UNIMP_VEC:      /* unimplemented FPA oper*/
    case IV_FPA_INV_VEC:        /* invalid FPA operation*/
    case IV_FPA_DIV0_VEC:       /* FPA div by zero */
    case IV_FPA_OVF_VEC:        /* FPA overflow exception */
    case IV_FPA_UFL_VEC:        /* FPA underflow exception */
    case IV_FPA_PREC_VEC:       /* FPA inexact operation */
        return (result = sysFpaAck ());
        break;
    default:
        return (-1);
        break;
    }
    return (result);
    }


#ifdef INCLUDE_WINDML
#ifndef INCLUDE_VXBUS
#include "sysWindML.c"
#endif /* INCLUDE_VXBUS */
#endif /* INCLUDE_WINDML */
#ifdef  INCLUDE_SIO_UTILS
/******************************************************************************
*
* bspSerialChanGet - get a serial channel
*
* RETURNS: ERROR
*
* NOMANUAL
*/

SIO_CHAN * bspSerialChanGet
    (
    int channel     /* serial channel */
    )
    {
    return ((SIO_CHAN *) ERROR);
    }
#endif  /* INCLUDE_SIO_UTILS */

/*
 * Reboot system.
 */
void sysReboot(void)
{
#ifndef BUILD_VXBOOT
    int sys_reset_pin, cpu_reset_pin;
    char *sys_reset_str, *cpu_reset_str;
    sys_reset_pin = (sys_reset_str = getvar(NULL,"SYS_RESET_PIN")) ?
                    bcm_atoi(sys_reset_str) : 32; /* 32 for None */
    cpu_reset_pin = (cpu_reset_str = getvar(NULL,"CPU_RESET_PIN")) ?
                    bcm_atoi(cpu_reset_str) : 32; /* 32 for None */
#endif

    if (sih->chiprev > 0) { /* B0 */
        reset_watchdog(1);
    } else { /* A0 */
#ifndef BUILD_VXBOOT
        if (cpu_reset_pin != 32) {
            *(volatile uint32 *)(0xb8000064) = (1<<cpu_reset_pin) |
                                               (1<<sys_reset_pin);
            *(volatile uint32 *)(0xb8000068) = (1<<cpu_reset_pin) |
                                               (1<<sys_reset_pin);
            *(volatile uint32 *)(0xb8000064) = 0x00000000;
        } else
#endif
            /* Reboot system */
            SYS_HARD_RESET();
    }
}

#ifndef INCLUDE_FLASH
/*
*  This call is here because the xmodem code calls flashFsSync().
*  flashFsSync() is normally defined in flashFsLib, but MBZ uses
*  the DOC, not a flash array.
*/
STATUS flashFsSync(void)
    {
    return (OK);
    }

#endif /* INCLUDE_FLASH */

typedef struct _systodfunctions
    {
    FUNCPTR init;
    FUNCPTR get;
    FUNCPTR set;
    FUNCPTR getSecond;
    FUNCPTR watchdogArm;
    } SYSTODFUNCTIONS;

SYSTODFUNCTIONS sysTodFuncs;

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


void sysGetChipInfo(uint *id, uint *rev, uint *pkg)
{
    ASSERT(sih);
    *id = sih->chip;
    *rev = sih->chiprev;
    *pkg = sih->chippkg;
}

void sysGetClocks(int *core, int *mem, int *sb)
{
    uint32 tmp;

    *core = mipsclk;
    tmp = *((volatile unsigned int *)(0xb800f018));
    tmp &= 0xffff;
    if (tmp == 0x3011) {
        *mem = 300000000;
    } else if (tmp == 0x3511) {
        *mem = 333000000;
    } else {
        *mem = 0;
    }
    *sb = sysGetSIClk();
}

STATUS sysRtcGet (struct tm * time)
{
    int year, month, date, hour, min, sec;
    STATUS status;
 
    status = ERROR;

#ifdef INCLUDE_I2C_DEV
    status = m41t81_tod_get(&year, &month, &date, &hour, &min, &sec);

    if (status == 0) {
        time->tm_sec   = sec;
        time->tm_min   = min;
        time->tm_hour  = hour;
        time->tm_mday  = date;
        time->tm_mon   = month - 1;  /* POSIX needs 0-11 */ 
        time->tm_year  = year - 1900;
        time->tm_yday  = 0;          /* unknown */
        time->tm_isdst = 0;          /* unknown */
    }
#endif /* INCLUDE_I2C_DEV */
    return status;
}

#ifdef INCLUDE_IPFTPS
int usrAuthenticateCallback(Ipftps_session *session, char *password)
{
    if (strcmp(session->username, "anonymous") == 0) {
        /* Access restrictions apply for anonymous users */
        session->readonly = 1;
    } 

    /* Allow any user/password combination */
    return 0;
}
#endif /* INCLUDE_IPFTPS */

/*
 * Below section are debug and development tools for Keystone.
 */
#if 1 /* For debug purpose before UART is up */

#define LOG_BUF_LEN (16384)         /* This must be a power of two */
#define LOG_BUF_MASK (LOG_BUF_LEN -1)

static char real_log_buf[LOG_BUF_LEN];
static char *log_buf = real_log_buf;
#define LOG_BUF(idx) (log_buf[(idx) & LOG_BUF_MASK])

static unsigned long log_start;         /* Index into log_buf: next char to be read by syslog() */
static unsigned long con_start;         /* Index into log_buf: next char to be sent to consoles */
static unsigned long log_end;           /* Index into log_buf: most-recently-written-char + 1 */
static unsigned long logged_chars;      /* Number of chars produced since last read+clear operation */

#define default_message_loglevel 4

static void emit_log_char(char c)
{
    LOG_BUF(log_end) = c;
    log_end++;
    if (log_end - log_start > LOG_BUF_LEN)
        log_start = log_end - LOG_BUF_LEN;
    if (log_end - con_start > LOG_BUF_LEN)
        con_start = log_end - LOG_BUF_LEN;
    if (logged_chars < LOG_BUF_LEN)
        logged_chars++;
}

int
no_uart_dbg(const char *fmt, ...)
{
    va_list ap;
    int count;
    char buffer[1024];
    char *p;
    int log_level_unknown = 1;

    if (log_buf == real_log_buf)
        log_buf = (char *) KSEG1ADDR((char *) real_log_buf);

    va_start(ap, fmt);
    count = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

#if 1
    for (p = buffer; *p; p++) {
        if (log_level_unknown) {
            if (p[0] != '<' || p[1] < '0' || p[1] > '7' || p[2] != '>') {
                emit_log_char('<');
                emit_log_char(default_message_loglevel + '0');
                emit_log_char('>');
            }
            log_level_unknown = 0;
        }
        emit_log_char(*p);
        if (*p == '\n')
            log_level_unknown = 1;
    }
#else
    for (i = 0; i < count; i++) {
        putc(buffer[i]);

        /* XXX EFI environment requires CR\LF in a printf, etc.
         * so unless the string has \r\n, it will not execute CR
         * So force it!
         */
#ifdef EFI
        if (buffer[i] == '\n')
            putc('\r');
#endif
    }
#endif

    return count;
}
#else 
int
no_uart_dbg(const char *fmt, ...);
#endif 

#if EXCEPTION_DEBUG /* Debug for exception */
#include <esf.h>
#include <edrLib.h>
#include <excLib.h>
#include <private/kernelBaseLibP.h> /* IS_KERNEL_TASK() */
#include <private/kernelLibP.h>     /* KERNEL_ENTERED_ME() */
#include <arch/mips/palMipsLib.h>
#include <arch/mips/mmuMipsLib.h>
#include <arch/mips/vxCpuArchLib.h>
#include <private/windLibP.h>

IMPORT int         intCnt;

LOCAL int excGetInfoFromESFDBG
    (
    int vecNum,
    ESFMIPS *pEsf,
    EXC_INFO *pExcInfo 
    )
{
    pExcInfo->vecNum = vecNum;
    pExcInfo->valid  = EXC_VEC_NUM;

    if ((vecNum == IV_TLBMOD_VEC ) || (vecNum == IV_TLBL_VEC ) ||
        (vecNum == IV_TLBS_VEC ) || (vecNum == IV_ADEL_VEC ) ||
        (vecNum == IV_ADES_VEC ))
        {
    /* Its an address error , or tlb miss */

    pExcInfo->valid     |= EXC_EPC | EXC_STATUS_REG | EXC_ACCESS_ADDR |
                   EXC_CAUSE_REG ;
    pExcInfo->epc        = ((ESFMIPS *)pEsf)->esfRegs.pc;
    pExcInfo->statusReg  = ((ESFMIPS *)pEsf)->esfRegs.sr;
    pExcInfo->causeReg   = ((ESFMIPS *)pEsf)->cause;
    pExcInfo->badVa      = ((ESFMIPS *)pEsf)->badva;

    return (sizeof (ESFMIPS));
    }
    else if ( (vecNum == IV_IBUS_VEC ) || (vecNum == IV_DBUS_VEC ))
        {
    /* Its a bus error */

    pExcInfo->valid     |= EXC_EPC | EXC_STATUS_REG | 
                   EXC_CAUSE_REG | EXC_ERROR_ADDR ;

    pExcInfo->epc        = ((ESFMIPS *)pEsf)->esfRegs.pc;
    pExcInfo->statusReg  = ((ESFMIPS *)pEsf)->esfRegs.sr;
    pExcInfo->causeReg   = ((ESFMIPS *)pEsf)->cause;
    pExcInfo->ear        = sysBusEar();
    pExcInfo->eid        = sysBusEid();

    return (sizeof (ESFMIPS));
    }
    else if ((vecNum <= IV_FPA_PREC_VEC) && (vecNum >= IV_FPA_UNIMP_VEC))
        {
    /* Its a floating point error */

    pExcInfo->valid     |= EXC_EPC | EXC_STATUS_REG |
                   EXC_CAUSE_REG | EXC_FP_STATUS_REG ;
    pExcInfo->epc        = ((ESFMIPS *)pEsf)->esfRegs.pc;
    pExcInfo->statusReg  = ((ESFMIPS *)pEsf)->esfRegs.sr;
    pExcInfo->causeReg   = ((ESFMIPS *)pEsf)->cause;
    pExcInfo->fpcsr      = ((ESFMIPS *)pEsf)->fpcsr;

    return (sizeof (ESFMIPS));
    }
#ifdef EXC_BREAK_TYPE    
    else if (vecNum == IV_BP_VEC)
    {
    pExcInfo->valid    |= EXC_EPC | EXC_STATUS_REG | EXC_CAUSE_REG
                          | EXC_BREAK_TYPE;
    pExcInfo->epc       = ((ESFMIPS *)pEsf)->esfRegs.pc;
    pExcInfo->statusReg = ((ESFMIPS *)pEsf)->esfRegs.sr;
    pExcInfo->causeReg  = ((ESFMIPS *)pEsf)->cause;
    pExcInfo->breakType = excBreakTypeGet(pEsf);

    return (sizeof (ESFMIPS));
    }
#endif
    else
    {
    pExcInfo->valid    |= EXC_EPC | EXC_STATUS_REG | EXC_CAUSE_REG;
    pExcInfo->epc       = ((ESFMIPS *)pEsf)->esfRegs.pc;
    pExcInfo->statusReg = ((ESFMIPS *)pEsf)->esfRegs.sr;
    pExcInfo->causeReg  = ((ESFMIPS *)pEsf)->cause;

    return (sizeof (ESFMIPS));
    }
    }

void excExcHandleDBG
    (
    int     vecNum,     /* exception vector number */
    ESFMIPS *   pEsf,       /* pointer to exception stack frame */
    REG_SET *   pRegs       /* pointer to general regs on esf */
    )
{
    EXC_INFO    excInfo;
    int     esfSize;
    void *  faultAddr;
#ifdef  _WRS_VX_SMP
    WIND_TCB *  currentTask;
#endif
    char *  errStr = NULL;
{
    ULONG c0_cause = MFC0(C0_CAUSE, 0);
    ULONG c0_epc = MFC0(C0_EPC, 0);
    ULONG c0_baddr = MFC0(C0_BADVADDR, 0);
    no_uart_dbg("%d: exception handler, vecNum = %d, C0_CAUSE = 0x%x, CO_EPC = 0x%x, C0_BADVADDR = 0x%x\n", __LINE__, vecNum, c0_cause,c0_epc,c0_baddr);
#if 1
    sysSerialPrintHex(c0_cause, 1);
    sysSerialPrintHex(c0_epc, 1);
    sysSerialPrintHex(c0_baddr, 1);
#endif
    *((volatile unsigned int *)(0xb8000044)) = 0x87654321;
    *((volatile unsigned int *)(0xb8000048)) = c0_cause;
    *((volatile unsigned int *)(0xb800004c)) = c0_epc;
}
    /* restore old sr so it looks like an eret occured */ 
    pEsf->esfRegs.sr &= ~SR_EXL;

    esfSize = excGetInfoFromESFDBG (vecNum, pEsf, &excInfo);

    /* fix task stack pointer and registers */

    pRegs->sr    = pEsf->esfRegs.sr;

#ifdef WV_INSTRUMENTATION
    /* windview - level 3 event logging */
    EVT_CTX_1(EVENT_EXCEPTION, vecNum);
#endif /* WV_INSTRUMENTATION */

    /*
     * The _func_excBaseHook is provided for Wind River components to use
     * the exception mechanism and handle exceptions in their own way.
     * Currently the only user of this feature is objVerify().  The
     * Wind object management system installs a hook during system
     * initialization time, i.e. the hook is always present, to trap
     * accesses to non-existing or protected memory when a user supplies
     * a bad object identifier.
     *
     * The function(s) hooked into _func_excBaseHook should return a
     * non-zero value to indicate that the exception has been handled,
     * and thus excExcHandle() will just return.  A zero return value
     * indicates that normal exception handling should continue.
     *
     * If an additional Wind River subsystem wishes to hook into the
     * exception handling path, the _func_excBaseHook can be daisy
     * chained.  When the subsystem initialization function executes,
     * the existing FUNCPTR value  of _func_excBaseHook should be cached.  
     * Then during exception handling the cached FUNCPTR should be called
     * if the exception is not to be handled by the current hook.
     */

    if ((_func_excBaseHook != NULL) &&          /* user hook around? */
    ((* _func_excBaseHook) (vecNum, pEsf, pRegs, &excInfo)))
    return;                     /* user hook fixed it */

    /* If the exception happened at interrupt level, or in 
     * kernel state, the ED&R policy handler will most
     * likely reboot the system.
     */
    
    if (INT_CONTEXT () || KERNEL_ENTERED_ME () == TRUE)
    {
    EDR_INTERRUPT_INFO  intInfo;    /* information for system */
    int         key;

#ifdef  INCLUDE_FULL_EDR_STUBS
    /*
     * Error cases handled here are:
     *  INT_CONTEXT() && pEsf->_errno==S_excLib_INTERRUPT_STACK_OVERFLOW
     *      interrupt stack overflow
     *  INT_CONTEXT() && pEsf->_errno!=S_excLib_INTERRUPT_STACK_OVERFLOW
     *      other interrupt-level exceptions
     *  !INT_CONTEXT() && kernelState == TRUE
     *      exception while in kernel state
     */

#define KRN_STATE_MSG   "exception while in kernel state!"

# ifdef _WRS_VX_SMP
/* spaces between "#  !" are used for CPU number which is inserted below */
#define BAD_IDLE_MSG    "\nbad idle task state on CPU #  !"
    /*
     * errBuf must be large enough to contain the cat
     * of KRN_STATE_MSG and BAD_IDLE_MSG
     */
    char errBuf[80] = "";
# endif /* _WRS_VX_SMP */

    if (INT_CONTEXT ())
        {
        if (pEsf->_errno == S_excLib_INTERRUPT_STACK_OVERFLOW)
        errStr = "fatal interrupt stack overflow!";
        else
        errStr = "interrupt-level exception!";
        }
    else
        {
# ifdef _WRS_VX_SMP
        WIND_TCB *   idleTask;
        unsigned int cpuIndex;
        cpuset_t     cpusetCpu; /* CPU as a bit-field */
# endif /* _WRS_VX_SMP */

        errStr = KRN_STATE_MSG;

# ifdef _WRS_VX_SMP
        /*
         * If this CPU holds the kernel lock now, it must also have held
         * it when the exception occurred, thus no migration is possible
         * and we are still on the CPU where the exception occurred.
         * Check whether something bad has happened to the idle task as
         * this is perhaps the most likely cause of such an exception.
         */

        cpuIndex = _WRS_CPU_INDEX_GET ();
        idleTask = _WRS_KERNEL_CPU_GLOBAL_GET(cpuIndex, idleTaskId);
        CPUSET_ZERO (cpusetCpu);
        CPUSET_SET (cpusetCpu, cpuIndex); /* get CPU as a bit-field */
        if (idleTask->status != WIND_READY
          || idleTask->windSmpInfo.affinity != cpusetCpu)
        {
        int where;

        strcpy (errBuf, errStr);
        strcat (errBuf, BAD_IDLE_MSG);
        where = strlen(errBuf) - 3;

        /* store 2 digit number for all CPU numbers (leading zero) */
        errBuf[where] = ((char)(cpuIndex / 10)) | '0';
        cpuIndex %= 10;
        errBuf[where + 1] = (char)cpuIndex | '0';
        errStr = &errBuf[0];
        }
#undef  BAD_IDLE_MSG
# endif /* _WRS_VX_SMP */
        }
#undef  KRN_STATE_MSG
#endif  /* INCLUDE_FULL_EDR_STUBS */

    intInfo.vector      = vecNum;
    intInfo.pEsf        = (char *) pEsf;
    intInfo.pRegs       = pRegs;
    intInfo.pExcInfo    = &excInfo;
    intInfo.isException = TRUE;

    /*
     * Inject to ED&R, using EDR_INTERRUPT_FATAL_INJECT even for the
     * kernel-state case since all of these conditions
     * require a reboot.  Fake intCnt to cover the non-interrupt cases.
     */

    key = INT_CPU_LOCK ();
    _WRS_KERNEL_GLOBAL_ACCESS (intCnt)++;

    /* inject to ED&R */

    EDR_INTERRUPT_FATAL_INJECT(&intInfo,
                   pRegs,
                   &excInfo,
                   (void *) pRegs->pc,
                   errStr);

    _WRS_KERNEL_GLOBAL_ACCESS (intCnt)--;
    INT_CPU_UNLOCK(key);
    return;
    }

    /*
     * The system should be able to handle its own task-level exceptions,
     * provided that it has been configured to do so (INCLUDE_EXC_HANDLING)
     * and that enough of the infrastructure has been initialized.
     * Otherwise we should treat the exception as fatal.
     *
     * Normal task-level exception handling is appropriate if either
     *
     *   + excInit() has completed:  _func_jobExcAdd != NULL
     *   or
     *   + the I/O system is present and has been initialized
     *         _func_ioGlobalStdGet != NULL
     *      && (*_func_ioGlobalStdGet)(STD_ERR) != ERROR
     *
     * Since the following code needs to detect the fatal case rather
     * than the non-fatal case, it uses the inverse of the above.
     */

    if (_func_jobExcAdd == NULL
     && (_func_ioGlobalStdGet == (FUNCPTR)NULL
      || (*_func_ioGlobalStdGet)(STD_ERR) == ERROR))
    {
    EDR_INIT_INFO   initInfo;   /* information for system */

    initInfo.vector     = vecNum;
    initInfo.pEsf       = (char *) pEsf;
    initInfo.pRegs      = pRegs;
    initInfo.pExcInfo       = &excInfo;
    initInfo.isException    = TRUE;

    /* inject to ED&R */

#ifdef INCLUDE_FULL_EDR_STUBS
    errStr = "exception during initialization!";
#endif /* INCLUDE_FULL_EDR_STUBS */

    EDR_INIT_FATAL_INJECT (&initInfo,
                   pRegs,
                   &excInfo,
                   (void *) pRegs->pc,
                   errStr);
    return;
    }

    /*
     * The exception was caused by a task.  Fetch the Id once and store it
     * locally.  This makes little difference for UP, but for SMP it avoids
     * the overhead of multiple calls to taskIdSelf().
     */

#ifdef  _WRS_VX_SMP

    /* Exceptions that occur while holding a spinlock are fatal. */

    {
    int key;
    unsigned int cpuIndex;

    key = INT_CPU_LOCK ();          /* prevent migration */
    cpuIndex = _WRS_CPU_INDEX_GET ();
    currentTask = _WRS_KERNEL_CPU_GLOBAL_GET (cpuIndex, taskIdCurrent);

    if (_WRS_KERNEL_CPU_GLOBAL_GET (cpuIndex, spinLockIsrCnt) > 0)
#ifdef  INCLUDE_FULL_EDR_STUBS
        errStr = "exception while holding an ISR spinlock!";
#else   /* INCLUDE_FULL_EDR_STUBS */
        errStr = "I";
#endif  /* INCLUDE_FULL_EDR_STUBS */
    else if (_WRS_KERNEL_CPU_GLOBAL_GET (cpuIndex, spinLockTaskCnt) > 0)
#ifdef  INCLUDE_FULL_EDR_STUBS
        errStr = "exception while holding a task spinlock!";
#else   /* INCLUDE_FULL_EDR_STUBS */
        errStr = "T";
#endif  /* INCLUDE_FULL_EDR_STUBS */
    else
        errStr = NULL;

    INT_CPU_UNLOCK(key);            /* migration OK */

    if (errStr)
        {
        /*
         * Use EDR_INTERRUPT_FATAL_INJECT (EDR_INTERRUPT_INFO) because an
         * exception while holding one of these locks requires a reboot.
         * EDR_KERNEL_FATAL_INJECT (EDR_TASK_INFO) would only stop the task.
         */

        EDR_INTERRUPT_INFO  intInfo;    /* information for system */

        intInfo.vector  = vecNum;
        intInfo.pEsf    = (char *) pEsf;
        intInfo.pRegs   = pRegs;
        intInfo.pExcInfo    = &excInfo;
        intInfo.isException = TRUE;

        /*
         * edrInterruptFatalPolicyHandler() expects intCnt > 0, because
         * it would ordinarily be called from interrupt context.  Fake it,
         * and don't allow any actual interrupts while intCnt is bogus.
         */

        key = INT_CPU_LOCK ();
        _WRS_KERNEL_GLOBAL_ACCESS (intCnt)++;

        EDR_INTERRUPT_FATAL_INJECT (&intInfo,
                    pRegs,
                    &excInfo,
                    (void *) pRegs->pc,
                    errStr);

        _WRS_KERNEL_GLOBAL_ACCESS (intCnt)--;
        INT_CPU_UNLOCK(key);
        return;
        }
    }

    /*
     * There is one more fatal condition to check for:  an exception in the
     * context of an idle task requires a reboot because the idle task cannot
     * be stopped or suspended.  Idle tasks are supposed to have CPU affinity,
     * so if the exception did occur in an idle task we will still be on the
     * same CPU; thus we need only check whether currentTask matches our CPU's
     * idle task ID.  Granted this ignores the possibility of the idle task
     * having lost its affinity and then taken an exception, however this is
     * considered unlikely.
     *
     * By far the most likely cause for an exception in an idle task is a
     * problem in a task switch/swap hook, but since such hooks execute in
     * kernel state any exceptions in them will have been detected by the
     * (KERNEL_ENTERED_ME () == TRUE) test above.  An idle-task exception
     * detected here may have been caused by additions to the idleTaskEntry()
     * function, or by hardware problems.
     */

    if (currentTask == _WRS_KERNEL_GLOBAL_ACCESS(idleTaskId))
    {
    int key;
    EDR_INTERRUPT_INFO  intInfo;    /* information for system */

    intInfo.vector         = vecNum;
    intInfo.pEsf           = (char *) pEsf;
    intInfo.pRegs          = pRegs;
    intInfo.pExcInfo       = &excInfo;
    intInfo.isException    = TRUE;

#ifdef INCLUDE_FULL_EDR_STUBS
    errStr = "exception in idle task!";
#endif /* INCLUDE_FULL_EDR_STUBS */

    /* See above WRT intCnt, use of EDR_INTERRUPT_FATAL_INJECT(), etc. */

    key = INT_CPU_LOCK ();
    _WRS_KERNEL_GLOBAL_ACCESS (intCnt)++;

    EDR_INTERRUPT_FATAL_INJECT (&intInfo,
                    pRegs,
                    &excInfo,
                    (void *) pRegs->pc,
                    errStr);

    _WRS_KERNEL_GLOBAL_ACCESS (intCnt)--;
    INT_CPU_UNLOCK(key);
    return;
    }

#endif  /* _WRS_VX_SMP */


    /* The exception was caused by a task */

    _WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent)->pExcRegSet = pRegs; /* for taskRegs[GS]et */

#ifdef  INCLUDE_SHELL
    taskIdDefault ((int)_WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent)); /* update default tid */
#endif  /* INCLUDE_SHELL */

    bcopy ((char *) &excInfo, (char *) &(_WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent)->excInfo),
       sizeof (EXC_INFO));          /* copy in exc info */

    /* determine fault address based on flags in pExcInfo->valid field */
    if (excInfo.valid & EXC_ACCESS_ADDR)
    faultAddr = (void *)excInfo.badVa;
    else if (excInfo.valid & EXC_ERROR_ADDR)
    faultAddr = (void *)excInfo.ear;
    else if (excInfo.valid & EXC_EPC)
    faultAddr = (void *)excInfo.epc;
    else
    faultAddr = (void *)NULL;
        
    /* An explanation of ED&R interaction with signals:-
     *
     * We invoke the signal hook first, and then inject an
     * error. If the signal hook doesn't return, its because
     * someone has taken care of the problem, in which case its
     * okay for ED&R not to worry about it.
     *
     * This has the advantage of allowing us to merge the
     * error-inject call and the policy invocation into one,
     * potentially.
     */
     
#ifdef  INCLUDE_RTP
    if (IS_KERNEL_TASK (_WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent)))
#endif  /* INCLUDE_RTP */
    {
    if (_func_sigExcKill != NULL)
        _func_sigExcKill((int) vecNum, vecNum, pRegs);
    }
#ifdef  INCLUDE_RTP
    else
    {
    /* Do signal raise and delivery for RTP task */

    /*
     * Make sure the exception didn't happen in the kernel code
     * that is after the system call
     * in such case we do not run user handlers.
     */

    if (_WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent)->excCnt == 1)
        {
        /* raise and deliver signal exception */

        if (_func_rtpSigExcKill != NULL)
        {
        int segViolation = 0;

        /* determine if an AdEL or AdES should be a SIGSEGV */
        /*
         * There are two vectors in _sigfaulttable for AdEL/AdES,
         * because AdEL/AdES can be either a SIGBUS (for
         * unaligned accesses) or SIGSEGV (if this is a
         * user-mode task attempting to access kernel space).
         * The entries for SIGSEGV are identified by their faultType
         * being 0x100 higher than the corresponding SIGBUS entries.
         *
         * We start by assuming that an AdEL/AdES exception is a
         * SIGBUS. (busErr = 0). If we detect that
         * this code was running in user space, and that the fault
         * address is above 0x80000000 (i.e., in kernel space), we
         * change this to SIGSEGV by setting busErr to 0x100.
         */
        if (((vecNum == IV_ADEL_VEC) || (vecNum == IV_ADES_VEC)) &&
            ((UINT32)excInfo.epc < KERNEL_SPACE_BOUNDARY) &&
            ((UINT32)faultAddr >= KERNEL_SPACE_BOUNDARY) )
            segViolation = SEG_ERR_OFF;

        (* _func_rtpSigExcKill) ((int) (vecNum + segViolation), vecNum,
                     pRegs, faultAddr);

        /*
         * If signal got delivered never here.... otherwise
         * a) It is possible that User mode stack is full and signal
         *    handler context for RTP task could not be carved. 
         *    In such case apply default policy.
         * b) It is possible that the sigaction associated with this
         *    exception signal is SIG_IGN. As per POSIX the behaviour
         *    onwards is undefined. We will continue and do the 
         *    deafult policy.
         */
        }

        } /* else exception in syscall, continue */
    }
#endif  /* INCLUDE_RTP */

    /* Call the exception show routine if one has been installed */

    if (_func_excInfoShow != NULL)
    (*_func_excInfoShow) (&excInfo, TRUE, NULL, NULL);

    /* Invoke legacy exc-hook if installed, and if kernel task. */

    if (IS_KERNEL_TASK (_WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent)))
    {
    if (excExcepHook != NULL)
        (* excExcepHook) (_WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent), vecNum, pEsf);
    }
    
    /* Now record the exception, since no signal-handler took it. */

#ifdef  INCLUDE_RTP
    if (_WRS_IS_SUPV_EXC ())
#endif  /* INCLUDE_RTP */
    {
    EDR_TASK_INFO   taskInfo;   /* information for task handling */
    char *      errStr = NULL;

    taskInfo.taskId     = (int) _WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent);
    taskInfo.status     = 0;
    taskInfo.vector     = vecNum;
    taskInfo.pEsf       = (char *) pEsf;
    taskInfo.pRegs      = pRegs;
    taskInfo.pExcInfo   = &excInfo;
    taskInfo.isException    = TRUE;

#ifdef  INCLUDE_FULL_EDR_STUBS
    /* A fatal exception in a kernel task. if OSM error, print unique
     * error for this event
     */

    if (pEsf->_errno == S_excLib_EXCEPTION_STACK_OVERFLOW)
        errStr = "fatal exception stack overflow!";
    else {
        ULONG cause = MFC0(C0_CAUSE, 0);
        errStr = "QT DBG: fatal kernel task-level exception!";
        no_uart_dbg("%d: exception handler, vecNum = %d, C0_CAUSE = 0x%x\n", __LINE__, vecNum, cause);
    }
#endif  /* INCLUDE_FULL_EDR_STUBS */

    EDR_KERNEL_FATAL_INJECT (&taskInfo,
                 pRegs,
                 &excInfo,
                 (void *) pRegs->pc,
                 errStr);
    }
#if defined (INCLUDE_RTP) && defined (INCLUDE_FULL_EDR_STUBS)
    else
    {
    EDR_RTP_INFO    rtpInfo;     /* information for rtp handling */

    rtpInfo.rtpId       = _WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent)->rtpId;
    rtpInfo.taskId      = (int) _WRS_KERNEL_GLOBAL_ACCESS(taskIdCurrent);
    rtpInfo.options     = 0;
    rtpInfo.status      = 0;
    rtpInfo.vector      = vecNum;
    rtpInfo.pEsf        = (char *) pEsf;
    rtpInfo.pRegs       = pRegs;
    rtpInfo.pExcInfo    = &excInfo;
    rtpInfo.isException = TRUE;

    /* A fatal exception in an RTP task. */
        
    EDR_RTP_FATAL_INJECT (&rtpInfo,
                  pRegs,
                  &excInfo,
                  (void *) pRegs->pc,
                  "fatal RTP exception!");
    }
#endif  /* INCLUDE_RTP && INCLUDE_FULL_EDR_STUBS */
    }

#define SYSLIB_SYSCALL()                        \
({                              \
    __asm__ __volatile__("                  \
    syscall");                      \
})
#endif

void sys_timestamp_get(uint32 *up, uint32 *low, uint32 *freq)
{
    int key, Count31_0;
    key = intLock();

    *up = sysSoftCount63_32;
    *low = Count31_0 = MFC0(C0_COUNT, 0);
    if (sysSoftCount31_0 < 0) {
        if (Count31_0 > 0) {
            *up += 1;
        }
    }
    *freq = sysGetMIPSClk() / 2;
    intUnlock (key);
}

#ifdef intUnlock
#undef intUnlock
#endif
extern int 	intUnlock (int oldSR);

int intUnlockSafe(int il)
{
    intUnlock(il);
    __asm__ __volatile__(".word 0x00000c0");
    __asm__ __volatile__("nop");
    return(0);
}

