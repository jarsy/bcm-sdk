/* sysLib.c - Wind River SBC548 board system-dependent library */

/* $Id: sysLib.c,v 1.36 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,03apr07,b_m  add TFFS map to static tlb table and phyMemDesc table.
01a,31jan06,kds  Modified from cds8458/sysLib.c/01c
*/

/*
 * DESCRIPTION
 * This library provides board-specific routines for SBC8548.
 *
 * INCLUDE FILES:
 *
 * SEE ALSO:
 * .pG "Configuration"
*/

/* includes */
#define SYSLIB_CALL
#include <vxWorks.h>
#include <vme.h>
#include <memLib.h>
#include <cacheLib.h>
#include <sysLib.h>
#include "config.h"
#include "configNet.h"
#include <string.h>
#include <intLib.h>
#include <logLib.h>
#include <stdio.h>
#include <taskLib.h>
#include <vxLib.h>
#include <tyLib.h>
#include <sysSymTbl.h>
#include <arch/ppc/mmuE500Lib.h>
#include <arch/ppc/vxPpcLib.h>
#include <private/vmLibP.h>
#include <miiLib.h>


#ifdef INCLUDE_PCI
    #include <drv/pci/pciConfigLib.h>
    #include <drv/pci/pciIntLib.h>
    #include "mot85xxPci.h"
#endif /* INCLUDE_PCI */

#ifdef INCLUDE_I2C_DEV
#include "i2cPCF8547Ioport.h"
#include "i2cM41T81Clock.h"
#include "usrTime.c"
#endif

/* globals */

TLB_ENTRY_DESC sysStaticTlbDesc [] =
{
    /*
     * effAddr,  Unused,  realAddr, ts | size | attributes | permissions
     *
     * TLB #0.  Flash
     *
     * needed be first entry here
     */

    { FLASH0_BASE_ADRS, 0x0, FLASH0_BASE_ADRS, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256M |
        _MMU_TLB_IPROT | _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_I |
        _MMU_TLB_ATTR_G
    },

    /*
     * LOCAL MEMORY needed be second entry here  -
     * one TLB would be 256MB so use to get required 512MB
     */

    { 0x00000000, 0x0, 0x00000000, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256M |
        _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_M |
        CAM_DRAM_CACHE_MODE | _MMU_TLB_IPROT
    },
#if (LOCAL_MEM_SIZE > 0x10000000)
    { 0x10000000, 0x0, 0x10000000, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256M |
        _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_M |
        CAM_DRAM_CACHE_MODE | _MMU_TLB_IPROT
    },
#endif

    { CCSBAR, 0x0, CCSBAR, _MMU_TLB_TS_0 | _MMU_TLB_SZ_1M |
        _MMU_TLB_ATTR_I | _MMU_TLB_ATTR_G | _MMU_TLB_PERM_W | _MMU_TLB_IPROT
    }

#ifdef INCLUDE_L2_SRAM
    ,
    { L2SRAM_ADDR, 0x0, L2SRAM_ADDR, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256K |
        _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_I |
        _MMU_TLB_ATTR_G
    }

#endif /* INCLUDE_L2_SRAM */

    /* Assume PCI space contiguous and within 256MB */

#ifdef INCLUDE_PCI
    ,
    { PCI_MEM_ADRS, 0x0, PCI_MEM_ADRS, _MMU_TLB_TS_0 | PCI_MMU_TLB_SZ |
        _MMU_TLB_ATTR_I | _MMU_TLB_ATTR_G | _MMU_TLB_PERM_W
    }
#ifdef INCLUDE_CDS85XX_PCIEX
    ,
    { PCI_MEM_ADRS3, 0x0, PCI_MEM_ADRS3, _MMU_TLB_TS_0 |  PCI_MMU_TLB_SZ |
        _MMU_TLB_ATTR_I | _MMU_TLB_ATTR_G | _MMU_TLB_PERM_W
    }
#endif /* INCLUDE_CDS85XX_PCIEX */

#endif  /* INCLUDE_PCI */

};

int sysStaticTlbDescNumEnt = NELEMENTS (sysStaticTlbDesc);

#ifdef MMU_ASID_MAX     /* Base 6 MMU library in effect */

  /* macro to compose 64-bit PHYS_ADDRs */

# define PHYS_64BIT_ADDR(h, l)  (((PHYS_ADDR)(h) << 32) + (l))
#endif

/*
* sysPhysMemDesc[] is used to initialize the Page Table Entry (PTE) array
* used by the MMU to translate addresses with single page (4k) granularity.
* PTE memory space should not, in general, overlap BAT memory space but
* may be allowed if only Data or Instruction access is mapped via BAT.
*
* Address translations for local RAM, memory mapped PCI bus, the Board Control and
* Status registers, the MPC8260 Internal Memory Map, and local FLASH RAM are set here.
*
* PTEs are held, strangely enough, in a Page Table.  Page Table sizes are
* integer powers of two based on amount of memory to be mapped and a
* minimum size of 64 kbytes.  The MINIMUM recommended Page Table sizes
* for 32-bit PowerPCs are:
*
* Total mapped memory		Page Table size
* -------------------		---------------
*        8 Meg			     64 K
*       16 Meg			    128 K
*       32 Meg			    256 K
*       64 Meg			    512 K
*      128 Meg			      1 Meg
* 	.				.
* 	.				.
* 	.				.
*
* [Ref: chapter 7, PowerPC Microprocessor Family: The Programming Environments]
*
*/
PHYS_MEM_DESC sysPhysMemDesc [] =
{

    {
        /*
	 * Vector Table and Interrupt Stack
         * Must be sysPhysMemDesc [0] to allow adjustment in sysHwInit()
	 */

        (VIRT_ADDR) LOCAL_MEM_LOCAL_ADRS,
        (PHYS_ADDR) LOCAL_MEM_LOCAL_ADRS,
        LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | TLB_CACHE_MODE | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        /*
         * CCSBAR
        */
        (VIRT_ADDR) CCSBAR,
        (PHYS_ADDR) CCSBAR,
        0x00100000,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE |
        VM_STATE_MASK_MEM_COHERENCY | VM_STATE_MASK_GUARDED,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT |
        VM_STATE_MEM_COHERENCY | VM_STATE_GUARDED
    }

#ifdef INCLUDE_L2_SRAM
    ,
    {
        (VIRT_ADDR) L2SRAM_ADDR,
        (PHYS_ADDR) L2SRAM_ADDR,
        L2SRAM_WINDOW_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    }
#endif

#ifdef INCLUDE_PCI
    ,
    {
        (VIRT_ADDR) PCI_MEM_ADRS,
        (PHYS_ADDR) PCI_MEM_ADRS,
        PCI_MEM_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_MEMIO_ADRS,
        (PHYS_ADDR) PCI_MEMIO_ADRS,
        PCI_MEMIO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_IO_ADRS,
        (PHYS_ADDR) PCI_IO_ADRS,
        PCI_IO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }



#ifdef INCLUDE_CDS85XX_PCIEX
    ,
    {
        (VIRT_ADDR) PCI_MEM_ADRS3,
        (PHYS_ADDR) PCI_MEM_ADRS3,
        PCI_MEM_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_MEMIO_ADRS3,
        (PHYS_ADDR) PCI_MEMIO_ADRS3,
        PCI_MEMIO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_IO_ADRS3,
        (PHYS_ADDR) PCI_IO_ADRS3,
        PCI_IO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
#endif /* INCLUDE_CDS85XX_PCIEX */
#endif /* INCLUDE_PCI */

    ,
    { 
        (VIRT_ADDR) ALPHA_LED_BASE_ADRS,
        (PHYS_ADDR) ALPHA_LED_BASE_ADRS,
        ALPHA_LED_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) FLASH0_BASE_ADRS,
        (PHYS_ADDR) FLASH0_BASE_ADRS,
        FLASH0_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) FLASH1_BASE_ADRS,
        (PHYS_ADDR) FLASH1_BASE_ADRS,
        FLASH1_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }


};

int sysPhysMemDescNumEnt = NELEMENTS (sysPhysMemDesc);

/* Clock Ratio Tables */

#define MAX_VALUE_PLAT_RATIO 32
UINT32 platRatioTable[MAX_VALUE_PLAT_RATIO][2] =
{
    { 0, 0},
    { 0, 0},
    { 2, 0},
    { 3, 0},
    { 4, 0},
    { 5, 0},
    { 6, 0},
    { 7, 0},
    { 8, 0},
    { 9, 0},
    { 10, 0},
    { 0, 0},
    { 12, 0},
    { 0, 0},
    { 0, 0},
    { 0, 0},
    { 16, 0},
    { 0, 0},
    { 0, 0},
    { 0, 0},
    { 20, 0},
    { 0, 0}
};

#define MAX_VALUE_E500_RATIO 10
UINT32 e500RatioTable[MAX_VALUE_PLAT_RATIO][2] =
{
    { 0, 0},
    { 0, 0},
    { 1, 0},
    { 3, 1},
    { 2, 0},
    { 5, 1},
    { 3, 0},
    { 7, 1},
    { 4, 0},
    { 9, 1}
};

int      pci_autoconfig_done = FALSE;
UINT32   sysPciMode;
UINT32   sysPciSlotDeviceNumber;
int   sysBus      = BUS_TYPE_NONE;      /* system bus type (VME_BUS, etc) */
int   sysCpu      = CPU;                /* system CPU type (PPC8260) */
char *sysBootLine = BOOT_LINE_ADRS; /* address of boot line */
char *sysExcMsg   = EXC_MSG_ADRS;   /* catastrophic message area */
int   sysProcNum;           /* processor number of this CPU */
BOOL  sysVmeEnable = FALSE;     /* by default no VME */
UINT32  coreFreq;

IMPORT void     mmuE500TlbDynamicInvalidate();
IMPORT void     mmuE500TlbStaticInvalidate();
IMPORT void mmuE500TlbStaticInit (int numDescs,
                                  TLB_ENTRY_DESC *pTlbDesc,
                                  BOOL cacheAllow);
IMPORT BOOL     mmuPpcIEnabled;
IMPORT BOOL     mmuPpcDEnabled;
IMPORT void     sysIvprSet(UINT32);

/* forward declarations */

void   sysUsDelay (UINT32);
void   sysLedClkRoutine (int arg);

#ifdef INCLUDE_L1_IPARITY_HDLR_INBSP
    #define _EXC_OFF_L1_PARITY 0x1500
IMPORT void jumpIParity();
IMPORT void sysIvor1Set(UINT32);
UINT32 instrParityCount = 0;
#endif  /* INCLUDE_L1_IPARITY_HDLR_INBSP */


#ifdef INCLUDE_PCI

STATUS sysPciSpecialCycle (int busNo, UINT32 message);
STATUS sysPciConfigRead   (int busNo, int deviceNo, int funcNo,
                           int offset, int width, void * pData);
STATUS sysPciConfigWrite  (int busNo, int deviceNo, int funcNo,
                           int offset, int width, ULONG data);

void   sysPciConfigEnable (int);

#endif /* INCLUDE_PCI */

/* 8260 Reset Configuration Table (From page 9-2 in Rev0 of 8260 book) */

#define END_OF_TABLE 0

UINT32 sysClkFreqGet(void);
UINT32 ppcE500ICACHE_LINE_NUM = (128 * 12);
UINT32 ppcE500DCACHE_LINE_NUM = (128 * 12);

UINT32 ppcE500CACHE_ALIGN_SIZE = 32;

#ifdef INCLUDE_PCI
LOCAL ULONG sysPciConfAddr = PCI_CFG_ADR_REG;   /* PCI Configuration Address */
LOCAL ULONG sysPciConfData = PCI_CFG_DATA_REG;  /* PCI Configuration Data */

#ifdef INCLUDE_GEI8254X_END
LOCAL int   sysPci1SysNum  = CDS85XX_PCI_1_BUS;

#ifdef  INCLUDE_CDS85XX_PCIEX
LOCAL int   sysPci3SysNum  = CDS85XX_PCIEX_BUS;
#endif /* INCLUDE_CDS85XX_PCIEX */
#endif /* INCLUDE_GEI8254X_END */

#include <pci/pciIntLib.c>           /* PCI int support */
#include <pci/pciConfigLib.c>        /* pci configuration library */
#if (defined(INCLUDE_PCI_CFGSHOW) && !defined(PRJ_BUILD))
#include <pci/pciConfigShow.c>
#endif /* (defined(INCLUDE_PCI_CFGSHOW) && !defined(PRJ_BUILD)) */
/* use pci auto config */
#include <pci/pciAutoConfigLib.c>    /* automatic PCI configuration */
#include <sysBusPci.c>               /* pciAutoConfig BSP support file */
#include <mot85xxPci.c>


#ifdef INCLUDE_GEI8254X_END
#include <sysGei8254xEnd.c>
#endif  /* INCLUDE_GEI_END */

#endif /* INCLUDE_PCI */

#include <m85xxTimer.c>
#include <sysMotI2c.c>
#include <sysMpc85xxI2c.c>

#ifdef INCLUDE_NV_RAM
    #include "i2c24LC128Eeprom.c"
    #include <mem/byteNvRam.c>      /* Generic NVRAM routines */
#else
    #include <mem/nullNvRam.c>
#endif /* INCLUDE_NV_RAM */

#ifdef INCLUDE_L1_IPARITY_HDLR
    #include <sysL1ICacheParity.c>
#endif

UINT32 inFullVxWorksImage=FALSE;

#define EXT_VEC_IRQ0            56
#define EXT_NUM_IRQ0            EXT_VEC_IRQ0
#define EXT_MAX_IRQS            200

STATUS  sysIntEnablePIC     (int intNum);   /* Enable i8259 or EPIC */
STATUS  sysCascadeIntEnable      (int intNum);
STATUS  sysCascadeIntDisable     (int intNum);
void    flashTest(VUINT16 *address,VUINT16 *buffer,VINT32 length);

UINT32   baudRateGenClk;

#include <sysEpic.c>


#ifdef INCLUDE_DUART
    #include <sysDuart.c>
#endif

#include <sysL2Cache.c>

#include <cmdLine.c>

#ifdef INCLUDE_VXBUS
IMPORT void hardWareInterFaceInit();
#endif /* INCLUDE_VXBUS */

#define WB_MAX_IRQS 256

#ifdef INCLUDE_SYSLED
#  include <sysLed.c>
#endif /* INCLUDE_SYSLED */

/* defines */

#define ZERO    0

/* needed to enable timer base */

#ifdef INCLUDE_PCI
    #define      M8260_DPPC_MASK	0x0C000000 /* bits 4 and 5 */
    #define      M8260_DPPC_VALUE	0x0C000000 /* bits (4,5) should be (1,0) */
#else
    #define      M8260_DPPC_MASK	0x0C000000 /* bits 4 and 5 */
    #define      M8260_DPPC_VALUE	0x08000000 /* bits (4,5) should be (1,0) */
#endif /*INCLUDE_PCI */

#define DELTA(a,b)                 (abs((int)a - (int)b))
#define HID0_MCP 0x80000000
#define HID1_ABE 0x00001000
#define HID1_ASTME 0x00002000
#define HID1_RXFE  0x00020000


#ifdef INCLUDE_VXBUS
#include <hwif/vxbus/vxBus.h>
#include <../src/hwif/h/busCtlr/m85xxRio.h>
#endif

#ifdef INCLUDE_MOT_TSEC_END
#include <sysNet.c>
#ifndef INCLUDE_VXBUS
#include <sysMotTsecEnd.c>
#endif /* INCLUDE_VXBUS */
#endif /* INCLUDE_MOT_TSEC_END */

#ifdef INCLUDE_VXBUS
#include <hwconf.c>
#endif

#ifdef INCLUDE_BRANCH_PREDICTION
IMPORT void disableBranchPrediction();
#endif

#ifdef INCLUDE_L2_SRAM
LOCAL void sysL2SramEnable(BOOL both);
#endif /* INCLUDE_L2_SRAM */

#ifdef INCLUDE_SPE
    #include <speLib.h>
IMPORT int       (* _func_speProbeRtn) () ;
#endif /* INCLUDE_SPE */

#ifdef INCLUDE_CACHE_SUPPORT
LOCAL void sysL1CacheQuery();
#endif

UINT32 sysTimerClkFreq = OSCILLATOR_FREQ;

IMPORT  void    sysL1Csr1Set(UINT32);
IMPORT  UINT    sysTimeBaseLGet(void);

LOCAL char * physTop = NULL;
LOCAL char * memTop = NULL;


#if     defined (INCLUDE_SPE)

/*******************************************************************************
*
* sysSpeProbe - Check if the CPU has SPE unit.
*
* This routine returns OK it the CPU has an SPE unit in it.
* Presently it assumes available.
*
* RETURNS: OK
*
* ERRNO: N/A
*/

int  sysSpeProbe (void)
    {
    ULONG regVal;
    int speUnitPresent = OK;

    /* The CPU type is indicated in the Processor Version Register (PVR) */

    regVal = 0;

    switch (regVal)
        {
        case 0:
        default:
            speUnitPresent = OK;
            break;
        }      /* switch  */

    return(speUnitPresent);
    }
#endif  /* INCLUDE_SPE */


/****************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the string.
*
* ERRNO: N/A
*/

char * sysModel (void)
    {
    UINT device;
    char* retChar = NULL;
    device = *M85XX_SVR(CCSBAR);

    switch(device & 0xffffff00)
	{
	case 0x80390000:
	    retChar = SYS_MODEL_8548E;
	    break;
	case 0x80310000:
	    retChar = SYS_MODEL_8548;
	    break;
	case 0x80390100:
	    retChar = SYS_MODEL_8547E;
	    break;
	case 0x80390200:
	    retChar = SYS_MODEL_8545E;
	    break;
	case 0x80310200:
	    retChar = SYS_MODEL_8545;
	    break;
	case 0x803A0000:
	    retChar = SYS_MODEL_8543E;
	    break;
	case 0x80320000:
	    retChar = SYS_MODEL_8543;
	    break;
	default:
	    retChar = SYS_MODEL_E500;
	    break;
	}


    device = *M85XX_PVR(CCSBAR);

    if ((device & 0xfff00000) != 0x80200000)
        retChar =SYS_MODEL_UNKNOWN;

    return(retChar);

    }

/******************************************************************************
*
* sysBspRev - return the BSP version with the revision eg 1.0/<x>
*
* This function returns a pointer to a BSP version with the revision.
* for eg. 1.0/<x>. BSP_REV defined in config.h is concatenated to
* BSP_VERSION and returned.
*
* RETURNS: A pointer to the BSP version/revision string.
*
* ERRNO: N/A
*/

char * sysBspRev (void)
    {
    return(BSP_VERSION BSP_REV);
    }

/******************************************************************************
*
* sysClkFreqGet - return Core Complex Bus clock freq
*
* This function returns the CCB clock freq.
*
* RETURNS: CCB clock freq
*
* ERRNO: N/A
*/

UINT32 sysClkFreqGet
(
void
)
    {
    UINT32  sysClkFreq;
    UINT32 e500Ratio,platRatio;

    platRatio = M85XX_PORPLLSR_PLAT_RATIO(CCSBAR);

#ifdef FORCE_DEFAULT_FREQ
    return(DEFAULT_SYSCLKFREQ);
#else

    if ((platRatio>MAX_VALUE_PLAT_RATIO)||(platRatioTable[platRatio][0]==0))
        return(DEFAULT_SYSCLKFREQ); /* A default value better than zero or -1 */

    sysClkFreq = ((UINT32)(OSCILLATOR_FREQ * platRatioTable[platRatio][0]))>>((UINT32)platRatioTable[platRatio][1]);

    e500Ratio = M85XX_PORPLLSR_E500_RATIO(CCSBAR);
    coreFreq = ((UINT32)(sysClkFreq * e500RatioTable[e500Ratio][0]))>>((UINT32)e500RatioTable[e500Ratio][1]);


    return(sysClkFreq);
#endif
    }

/******************************************************************************
*
* sysCpmFreqGet - Determines the CPM Operating Frequency
*
* This routine determines the CPM Operating Frequency.
*
* From page 9-2 Rev. 0  MPC8260  PowerQUICC II User's Manual
*
* RETURNS: CPM clock frequency for the current MOD_CK and MOD_CK_H settings
*
* ERRNO: N/A
*/

UINT32 sysCpmFreqGet (void)
    {
    UINT32 sysClkFreq = sysClkFreqGet();
    return(sysClkFreq);

    }

/******************************************************************************
*
* sysBaudClkFreq - Obtains frequency of the BRG_CLK in Hz
*
* From page 9-5 in Rev. 0 MPC8260 PowerQUICC II User's Manual
*
*     baud clock = 2*cpm_freq/2^2*(DFBRG+1) where DFBRG = 01
*                = 2*cpm_freq/16
*
* RETURNS: frequency of the BRG_CLK in Hz
*
* ERRNO: N/A
*/

UINT32 sysBaudClkFreq (void)
    {
    UINT32 cpmFreq = sysCpmFreqGet();

    return cpmFreq*2/16;
    }

/******************************************************************************
*
* sysHwMemInit - initialize and configure system memory.
*
* This routine is called before sysHwInit(). It performs memory auto-sizing
* and updates the system's physical regions table, `sysPhysRgnTbl'. It may
* include the code to do runtime configuration of extra memory controllers.
*
* NOTE: This routine should not be called directly by the user application.  It
* cannot be used to initialize interrupt vectors.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysHwMemInit (void)
    {

    /* Call sysPhysMemTop() to do memory autosizing if available */

    sysPhysMemTop ();

    }

/******************************************************************************
*
* sysHwInit - initialize the system hardware
*
* This routine initializes various feature of the MPC8260 ADS board. It sets up
* the control registers, initializes various devices if they are present.
*
* NOTE: This routine should not be called directly by the user.
*
* RETURNS: NA
*
* ERRNO: N/A
*/

void sysHwInit (void)
    {


#ifdef INCLUDE_RAPIDIO_BUS

    /* Errata not yet described - required for rapidIO TAS */

    *(UINT32*)(CCSBAR + 0x1010) = 0x01040004;
#endif

    sysIvprSet(0x0);

    /* Disable L1 Icache */

    sysL1Csr1Set(vxL1CSR1Get() & ~0x1);

    /* Check for architecture support for 36 bit physical addressing */

#if defined(PPC_e500v2)
    vxHid0Set(_PPC_HID0_MAS7EN|vxHid0Get());
#endif

    /* Enable machine check pin */

    vxHid0Set(HID0_MCP|vxHid0Get());

#ifdef E500_L1_PARITY_RECOVERY

    /* Enable Parity in L1 caches */

    vxL1CSR0Set(vxL1CSR0Get() | _PPC_L1CSR_CPE);
    vxL1CSR1Set(vxL1CSR1Get() | _PPC_L1CSR_CPE);
#endif  /* E500_L1_PARITY_RECOVERY */

    /* enable time base for delay use before DEC interrupt is setup */

    vxHid0Set(vxHid0Get() | _PPC_HID0_TBEN);

    sysTimerClkFreq = sysClkFreqGet()>>3 /* Clock div is 8 */;

#ifdef INCLUDE_AUX_CLK
    sysAuxClkRateSet(127);
#endif

#ifdef INCLUDE_CACHE_SUPPORT
    sysL1CacheQuery();
#endif /* INCLUDE_CACHE_SUPPORT */

    /* Initialise L2CTL register */

    vxL2CTLSet(0x28000000,M85XX_L2CTL(CCSBAR));

    /*
     * Need to setup static TLB entries for bootrom or any non-MMU
     * enabled images
     */
    mmuE500TlbDynamicInvalidate();
    mmuE500TlbStaticInvalidate();
    mmuE500TlbStaticInit(sysStaticTlbDescNumEnt, &sysStaticTlbDesc[0], TRUE);

#if (!defined(INCLUDE_MMU_BASIC) && !defined(INCLUDE_MMU_FULL))
    mmuPpcIEnabled=TRUE;
    mmuPpcDEnabled=TRUE;
#else /* !defined(INCLUDE_MMU_BASIC) && !defined(INCLUDE_MMU_FULL) */
    if (inFullVxWorksImage==FALSE)
        {
        mmuPpcIEnabled=TRUE;
        mmuPpcDEnabled=TRUE;
        }
    /* Enable I Cache if instruction mmu disabled */

#if (defined(USER_I_CACHE_ENABLE) && !defined(USER_I_MMU_ENABLE))
    mmuPpcIEnabled=TRUE;
#endif /* (defined(USER_I_CACHE_ENABLE) && !defined(USER_I_MMU_ENABLE)) */

#endif /* !defined(INCLUDE_MMU_BASIC) && !defined(INCLUDE_MMU_FULL) */


#if (defined(INCLUDE_L2_CACHE) && defined(INCLUDE_CACHE_SUPPORT))
    vxHid1Set(HID1_ABE); /* Address Broadcast enable */
    sysL2CacheInit();
#endif /* INCLUDE_L2_CACHE  && INCLUDE_CACHE_SUPPORT*/

    /* Machine check via RXFE for RIO */

    vxHid1Set(vxHid1Get()| HID1_ASTME | HID1_RXFE); /* Address Stream Enable */

    /* enable the flash window */

    *M85XX_LAWBAR3(CCSBAR) = FLASH1_BASE_ADRS >> LAWBAR_ADRS_SHIFT;
    *M85XX_LAWAR3(CCSBAR)  = LAWAR_ENABLE | LAWAR_TGTIF_LBC | LAWAR_SIZE_64MB;
    WRS_ASM("isync");

    /* Initialize the Embedded Programmable Interrupt Controller */

    sysEpicInit();


#ifdef INCLUDE_VXBUS
    hardWareInterFaceInit();
#endif /* INCLUDE_VXBUS */


#ifdef INCLUDE_DUART
    sysDuartHwInit ();
#endif


#ifdef INCLUDE_PCI

    /*
     * PCI slot device number is decoded differently between PCI mode and
     * PCI-X mode. So we read the PORDEVSR register to determine which mode
     * the system is in and assign the proper device number access the slot.
     */
    sysPciMode = M85XX_PORDEVSR_PCI_MODE(CCSBAR);
    if (sysPciMode == PORDEVSR_PCIX_MODE)
       {
      	   sysPciSlotDeviceNumber = 0x1; /* PCI-X */
       }
  	else
       {
   	   sysPciSlotDeviceNumber = 0x11; /* PCI */
       }
			/* config pci */

    if (pciConfigLibInit (PCI_MECHANISM_0,(ULONG) sysPciConfigRead,
                          (ULONG) sysPciConfigWrite,(ULONG) sysPciSpecialCycle) != OK)
        {
        sysToMonitor (BOOT_NO_AUTOBOOT);  /* BAIL */
        }

    /*  Initialize PCI interrupt library. */

    if ((pciIntLibInit ()) != OK)
        {
        sysToMonitor (BOOT_NO_AUTOBOOT);
        }
#if 0
    if ( *((char*)PCI_AUTO_CONFIG_ADRS) == FALSE )
#endif
        {
        mot85xxBridgeInit();
#ifdef INCLUDE_PCI_AUTOCONF
        sysPciAutoConfig();

        pci_autoconfig_done = TRUE;
#endif
        *((char*)PCI_AUTO_CONFIG_ADRS) = TRUE;
        }

#endif /* INCLUDE_PCI */


#ifdef E500_L1_PARITY_RECOVERY
    vxIvor1Set(_EXC_OFF_L1_PARITY);
#endif  /* E500_L1_PARITY_RECOVERY */
#ifdef INCLUDE_L1_IPARITY_HDLR
    installL1ICacheParityErrorRecovery();
#endif /* INCLUDE_L1_IPARITY_HDLR */

    /*
     * The power management mode is initialized here. Reduced power mode
     * is activated only when the kernel is idle (cf vxPowerDown).
     * Power management mode is selected via vxPowerModeSet().
     * DEFAULT_POWER_MGT_MODE is defined in config.h.
     */

#if defined(INCLUDE_L2_SRAM)
#if (defined(INCLUDE_L2_CACHE) && defined(INCLUDE_CACHE_SUPPORT))
    sysL2SramEnable(TRUE);
#elif (defined(INCLUDE_L2_SRAM))
    sysL2SramEnable(FALSE);
#endif
#endif

    CACHE_PIPE_FLUSH();

    vxPowerModeSet (DEFAULT_POWER_MGT_MODE);

    }

#ifdef INCLUDE_L2_SRAM
/*************************************************************************
*
* sysL2SramEnable - enables L2SRAM if L2SRAM only
*
* This routine enables L2SRAM if L2SRAM only or initializes blk
* size etc and leaves the rest to L2 cache code.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
LOCAL void sysL2SramEnable
(
BOOL both
)
    {
    volatile int l2CtlVal;

    /*
     * if INCLUDE_L2_CACHE and CACHE_SUPPORT
     * if ((L2_SRAM_SIZE + L2_CACHE_SIZE) > l2Siz)
     */

    /* Setup Windows for L2SRAM */

    *(M85XX_L2SRBAR0(CCSBAR)) = (UINT32)(L2SRAM_ADDR & M85XX_L2SRBAR_ADDR_MSK);

    /* Get present value */

    l2CtlVal = vxL2CTLGet(M85XX_L2CTL(CCSBAR));

    /* Disable L2CTL initially to allow changing of block size */

    l2CtlVal &=(~M85XX_L2CTL_L2E_MSK);
    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
    l2CtlVal &= ~M85XX_L2CTL_L2BLKSIZ_MSK;
    l2CtlVal &= ~M85XX_L2CTL_L2SRAM_MSK;

    if (both == TRUE)
        {
        /* Setup size of SRAM */

        l2CtlVal |= (L2SIZ_256KB << M85XX_L2CTL_L2BLKSIZ_BIT) |
                    (0x2 << M85XX_L2CTL_L2SRAM_BIT);
        }
    else
        {
        l2CtlVal |= (L2SIZ_512KB << M85XX_L2CTL_L2BLKSIZ_BIT) |
                    (0x1 << M85XX_L2CTL_L2SRAM_BIT);
        }

    /* Setup L2CTL for SRAM */

    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));

    if (both == FALSE)
        {
        /* This is done here so L2SRAM is set before enable */

        l2CtlVal |= M85XX_L2CTL_L2E_MSK; /* No cache so go ahead and enable */

        /* Enable L2CTL for SRAM */

        vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
        }

    }
#endif /* INCLUDE_L2_SRAM */

/**************************************************************************
*
* sysPhysMemTop - get the address of the top of physical memory
*
* This routine returns the address of the first missing byte of memory,
* which indicates the top of memory.
*
* RETURNS: The address of the top of physical memory.
*
* ERRNO: N/A
*
* SEE ALSO: sysMemTop()
*/

char * sysPhysMemTop (void)
    {

    if (physTop == NULL)
        {
        physTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
        }

    return(physTop) ;
    }

/***************************************************************************
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
*
* ERRNO: N/A
*/

char * sysMemTop (void)
    {

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

/**************************************************************************
*
* sysToMonitor - transfer control to the ROM monitor
*
* This routine transfers control to the ROM monitor.  Normally, it is called
* only by reboot()--which services ^X--and bus errors at interrupt level.
* However, in some circumstances, the user may wish to introduce a
* <startType> to enable special boot ROM facilities.
*
* RETURNS: Does not return.
*
* ERRNO: N/A
*/
int sysToMonitorExcMessage = 0;
int sysToMonitorBacktrace = 1;
int sysToMonitorReboot = 1;
void (*sysToMonitorHook)(void);

STATUS sysToMonitor
    (
    int startType   /* parameter passed to ROM to tell it how to boot */
    )
    {
#if 0
    FUNCPTR pRom = (FUNCPTR) (ROM_TEXT_ADRS + 4);   /* Warm reboot */
#endif

    if (sysToMonitorHook)
        (*sysToMonitorHook)();

    sysReboot();

#if 0
    intLock();


#ifdef INCLUDE_BRANCH_PREDICTION
    disableBranchPrediction();
#endif /* INCLUDE_BRANCH_PREDICTION */

#ifdef INCLUDE_CACHE_SUPPORT
    cacheDisable(INSTRUCTION_CACHE);
    cacheDisable(DATA_CACHE);
#endif
    sysClkDisable();


#ifdef INCLUDE_AUX_CLK
    sysAuxClkDisable();
#endif

    vxMsrSet(0);

    /* Clear unnecessary TLBs */

    mmuE500TlbDynamicInvalidate();
    mmuE500TlbStaticInvalidate();

    (*pRom) (startType);    /* jump to bootrom entry point */
#endif
    return(OK);    /* in case we ever continue from ROM monitor */
    }

/******************************************************************************
*
* sysHwInit2 - additional system configuration and initialization
*
* This routine connects system interrupts and does any additional
* configuration necessary.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysHwInit2 (void)
    {

#ifdef  INCLUDE_VXBUS
    vxbDevInit();
#endif /* INCLUDE_VXBUS */


    excIntConnect ((VOIDFUNCPTR *) _EXC_OFF_DECR,
                   (VOIDFUNCPTR) sysClkInt);

    sysClkRateSet(60);
    sysClkEnable();

#ifdef INCLUDE_AUX_CLK
    excIntConnect ((VOIDFUNCPTR *) _EXC_OFF_FIT, (VOIDFUNCPTR) sysAuxClkInt);
#endif

#  if defined(INCLUDE_AUX_CLK)

     /* initialize and start auxiliary clock support */

     sysAuxClkEnable ();

#  endif /* INCLUDE_AUX_CLK */


    /*
     * This was previously reqd for errata workaround #29, the workaround
     * has been replaced with patch for spr99776, so it now serves as an
     * example of implementing an l1 instruction parity handler
     */

#ifdef INCLUDE_L1_IPARITY_HDLR_INBSP
    memcpy((void*)_EXC_OFF_L1_PARITY, (void *)jumpIParity, sizeof(INSTR));
    cacheTextUpdate((void *)_EXC_OFF_L1_PARITY, sizeof(INSTR));
    sysIvor1Set(_EXC_OFF_L1_PARITY);
    cacheDisable(INSTRUCTION_CACHE);
    vxL1CSR1Set(vxL1CSR1Get() | _PPC_L1CSR_CPE);
    cacheEnable(INSTRUCTION_CACHE);
#endif  /* INCLUDE_L1_IPARITY_HDLR_INBSP */

    /* initialize the EPIC interrupts */

    sysEpicIntrInit ();

    /* initialize serial interrupts */

#if defined(INCLUDE_DUART)
    sysSerialHwInit2 ();
#endif /* INCLUDE_DUART */

#if     defined (INCLUDE_SPE)
    _func_speProbeRtn = sysSpeProbe;
#endif  /* INCLUDE_SPE */


#ifdef INCLUDE_PCI

#ifdef INCLUDE_GEI8254X_END
    sysPciConfigEnable (CDS85XX_PCI_1_BUS);
    pciConfigForeachFunc (0, FALSE, (PCI_FOREACH_FUNC) sys8254xDeviceCheck, (void *)&sysPci1SysNum);
#ifdef INCLUDE_CDS85XX_PCIEX
#ifdef INCLUDE_GEI8254X_END
    sysPciConfigEnable (CDS85XX_PCIEX_BUS);
    pciConfigForeachFunc (PCIEX_BUS_MIN + 1, FALSE, (PCI_FOREACH_FUNC) sys8254xDeviceCheck, (void *)&sysPci3SysNum);
#endif /* INCLUDE_GEI8254X_END */

#endif /* INCLUDE_CDS85XX_PCIEX */
#endif /* INCLUDE_GEI8254X_END */
#endif /* INCLUDE_PCI */

    /* Configure GPIO */
    * (VINT32 *) M85XX_GPOUTDR(CCSBAR) = 0x3;
    * (VINT32 *) M85XX_GPIOCR(CCSBAR) = (1 << 17) | ( 1 << 16) | (1 << 9);

#ifdef INCLUDE_SYSLED
    sysLedOn();
#endif 

#ifdef INCLUDE_I2C_DEV
    i2cDrvInit(0, MPC85XX_I2C_CONTROLLER);
    i2cDrvInit(1, MPC85XX_I2C_CONTROLLER);
    usrTimeSync();
#endif /* INCLUDE_I2C_DEV */

#ifdef  INCLUDE_VXBUS
    taskSpawn("devConnect",0,0,10000,vxbDevConnect,0,0,0,0,0,0,0,0,0,0);
#endif /* INCLUDE_VXBUS */

    }

/******************************************************************************
*
* sysProcNumGet - get the processor number
*
* This routine returns the processor number for the CPU board, which is
* set with sysProcNumSet().
*
* RETURNS: The processor number for the CPU board.
*
* ERRNO: N/A
*
* SEE ALSO: sysProcNumSet()
*/

int sysProcNumGet (void)
    {
    return(sysProcNum);
    }

/******************************************************************************
*
* sysProcNumSet - set the processor number
*
* This routine sets the processor number for the CPU board.  Processor numbers
* should be unique on a single backplane.
*
* Not applicable for the bus-less 8260Ads.
*
* RETURNS: N/A
*
* ERRNO: N/A
*
* SEE ALSO: sysProcNumGet()
*/

void sysProcNumSet
    (
    int     procNum         /* processor number */
    )
    {
    sysProcNum = procNum;
    }

/******************************************************************************
*
* sysLocalToBusAdrs - convert a local address to a bus address
*
* This routine gets the VMEbus address that accesses a specified local
* memory address.
*
* Not applicable for the 8260Ads
*
* RETURNS: ERROR, always.
*
* ERRNO: N/A
*
* SEE ALSO: sysBusToLocalAdrs()
*/

STATUS sysLocalToBusAdrs
    (
    int     adrsSpace,  /* bus address space where busAdrs resides */
    char *  localAdrs,  /* local address to convert */
    char ** pBusAdrs    /* where to return bus address */
    )
    {

    *pBusAdrs = localAdrs;

    return(OK);
    }

/******************************************************************************
*
* sysBusToLocalAdrs - convert a bus address to a local address
*
* This routine gets the local address that accesses a specified VMEbus
* physical memory address.
*
* Not applicable for the 8260Ads
*
* RETURNS: ERROR, always.
*
* ERRNO: N/A
*
* SEE ALSO: sysLocalToBusAdrs()
*/

STATUS sysBusToLocalAdrs
    (
    int     adrsSpace,  /* bus address space where busAdrs resides */
    char *  busAdrs,    /* bus address to convert */
    char ** pLocalAdrs  /* where to return local address */
    )
    {

    *pLocalAdrs = busAdrs;

    return(OK);
    }



#ifdef INCLUDE_PCI 	/* board level PCI routines */

/*******************************************************************************
* sysPciConfigEnable -  enable PCI 1 or PCI 2 bus configuration
*
* This function enables PCI 1 or PCI 2 bus configuration
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysPciConfigEnable
    (
    int pciHost
    )
    {
    int level;

    level = intLock ();

    if (pciHost == CDS85XX_PCI_2_BUS)
        {
        sysPciConfAddr = PCI2_CFG_ADR_REG;   /* PCI Configuration Address */
        sysPciConfData = PCI2_CFG_DATA_REG;  /* PCI Configuration Data */
        }
    else if (pciHost == CDS85XX_PCIEX_BUS)
        {
        sysPciConfAddr = PCIEX_CFG_ADR_REG;   /* PCI Configuration Address */
        sysPciConfData = PCIEX_CFG_DATA_REG;  /* PCI Configuration Data */
        }
    else /* default is for PCI 1 host */
        {
        sysPciConfAddr = PCI_CFG_ADR_REG;   /* PCI Configuration Address */
        sysPciConfData = PCI_CFG_DATA_REG;  /* PCI Configuration Data */
        }

    WRS_ASM("sync;eieio");

    intUnlock (level);
    }

/*******************************************************************************
*
* sysPciSpecialCycle - generate a special cycle with a message
*
* This routine generates a special cycle with a message.
*
* \NOMANUAL
*
* RETURNS: OK
*
* ERRNO: N/A
*/

STATUS sysPciSpecialCycle
    (
    int     busNo,
    UINT32  message
    )
    {
    int deviceNo    = 0x0000001f;
    int funcNo      = 0x00000007;

    if (pci_autoconfig_done) {
        if (busNo >= PCIEX_BUS_MIN) {
            busNo = busNo - PCIEX_BUS_MIN;
            sysPciConfigEnable(CDS85XX_PCIEX_BUS);
        } else {
            sysPciConfigEnable(CDS85XX_PCI_1_BUS);
        }
    }
 
    pciRegWrite ((UINT32 *)sysPciConfAddr,
                 (UINT32)pciConfigBdfPack (busNo, deviceNo, funcNo) |
                 0x80000000);

    PCI_OUT_LONG (sysPciConfData, message);

    return(OK);
    }

/*******************************************************************************
*
* sysPciConfigRead - read from the PCI configuration space
*
* This routine reads either a byte, word or a long word specified by
* the argument <width>, from the PCI configuration space
* This routine works around a problem in the hardware which hangs
* PCI bus if device no 12 is accessed from the PCI configuration space.
*
* \NOMANUAL
*
* RETURNS: OK, or ERROR if this library is not initialized
*
* ERRNO: N/A
*/

STATUS sysPciConfigRead
    (
    int busNo,    /* bus number */
    int deviceNo, /* device number */
    int funcNo,   /* function number */
    int offset,   /* offset into the configuration space */
    int width,    /* width to be read */
    void * pData /* data read from the offset */
    )
    {
    UINT8  retValByte = 0;
    UINT16 retValWord = 0;
    UINT32 retValLong = 0;
    STATUS retStat = ERROR;

    /* Set Default values of data */
    switch(width) {
        case 1:
            *((UINT8 *)pData) = 0xff;
            break;
        case 2:
            *((UINT16 *)pData) = 0xffff; 
            break;
        case 4:
            *((UINT32 *)pData) = 0xffffffff;
            break;
        default:
            return ERROR;
    }
 
#if defined(BCM_DEVICE_ON_PCIE) /* Skip device on PCI BUS */
    if  (0x1e == deviceNo) {
        return OK;
    }
#else
#endif
    if (pci_autoconfig_done) {
        if (busNo >= PCIEX_BUS_MIN) {
            if ((busNo > (PCIEX_BUS_MIN + PCI_MAX_BUS)) || (deviceNo > 31))  {
                return OK;
            }
            busNo = busNo - PCIEX_BUS_MIN;
            sysPciConfigEnable(CDS85XX_PCIEX_BUS);
        } else {
            sysPciConfigEnable(CDS85XX_PCI_1_BUS);
        }
    }
    
    /* This is for PCI Express */
    if ((sysPciConfAddr == PCIEX_CFG_ADR_REG) && (busNo > 0))
    {
         /* Read LTSSM state status register to make sure that the Link is ready */
         pciRegWrite ((UINT32 *)sysPciConfAddr,
                         (UINT32)pciConfigBdfPack (0, 0, 0) |
                         (0x404 & 0xfc) | 0x80000000 | ((0x404 & 0xf00) << 16));
          retValLong = PCI_IN_LONG (sysPciConfData);
          if (retValLong != 0x16) {
              /* Link training is not completed. Cannot read PCIE config 
               * space. */
              return OK;
          }
    }

    switch (width)
        {
        case 1: /* byte */
            pciRegWrite ((UINT32 *)sysPciConfAddr,
                         (UINT32)pciConfigBdfPack (busNo, deviceNo, funcNo) |
                         (offset & 0xffc) | 0x80000000 | ((offset & 0xf00) << 16)) ;

            retValByte = PCI_IN_BYTE (sysPciConfData + (offset & 0x3));
            *((UINT8 *)pData) = retValByte;
            retStat = OK;
            break;

        case 2: /* word */
            pciRegWrite ((UINT32 *)sysPciConfAddr,
                         (UINT32)pciConfigBdfPack (busNo, deviceNo, funcNo) |
                         (offset & 0xfc) | 0x80000000 | ((offset & 0xf00) << 16));

            retValWord = PCI_IN_WORD (sysPciConfData + (offset & 0x2));
            *((UINT16 *)pData) = retValWord;
            retStat = OK;
            break;

        case 4: /* long */
            pciRegWrite ((UINT32 *)sysPciConfAddr,
                         (UINT32)pciConfigBdfPack (busNo, deviceNo, funcNo) |
                         (offset & 0xfc) | 0x80000000 | ((offset & 0xf00) << 16));
            retValLong = PCI_IN_LONG (sysPciConfData);
            *((UINT32 *)pData) = retValLong;
            retStat = OK;
            break;

        default:
            retStat = ERROR;
            break;
        }

    return(retStat);
    }

/*******************************************************************************
*
* sysPciConfigWrite - write to the PCI configuration space
*
* This routine writes either a byte, word or a long word specified by
* the argument <width>, to the PCI configuration space
* This routine works around a problem in the hardware which hangs
* PCI bus if device no 12 is accessed from the PCI configuration space.
* Errata PCI-Express requires RMW to ensure always write 4byte aligned.
*
* \NOMANUAL
*
* RETURNS: OK, or ERROR if this library is not initialized
*
* ERRNO: N/A
*/

STATUS sysPciConfigWrite
    (
    int busNo,    /* bus number */
    int deviceNo, /* device number */
    int funcNo,   /* function number */
    int offset,   /* offset into the configuration space */
    int width,    /* width to write */
    ULONG data    /* data to write */
    )
    {

 /*
  * PCI-Express errata RMW workaround should work for all PCI
  * Requires always to do 4 byte read/write
  */

    ULONG data2,mask;
 
   /* Establish the required mask based on width and offset */

    switch(width)
	{
	case 1:
	    mask=0xff;
	    mask=~(mask << ((offset&0x3)*8));
	    break;
	case 2:
	    mask=0xffff;
	    mask=~(mask << ((offset&0x3)*8));
	    break;
	case 4:
	    mask=0;
	    break;
	default:
	    return(ERROR);
	}

    if (pci_autoconfig_done) {
        if (busNo >= PCIEX_BUS_MIN) {
            /* busNo = busNo - PCIEX_BUS_MIN;*/
            sysPciConfigEnable(CDS85XX_PCIEX_BUS);
        } else {
            sysPciConfigEnable(CDS85XX_PCI_1_BUS);
        }
    }

    sysPciConfigRead(busNo,deviceNo,funcNo,(offset & ~0x3), 4, &data2);

    if (pci_autoconfig_done) {
        if (busNo >= PCIEX_BUS_MIN) {
            busNo = busNo - PCIEX_BUS_MIN;
            sysPciConfigEnable(CDS85XX_PCIEX_BUS);
        } else {
            sysPciConfigEnable(CDS85XX_PCI_1_BUS);
        }
    }

    data2 &= mask;

    data2 |= data << ((offset&0x3)*8); /* Overwite only part of word reqd */

    data = data2;             /* final 32 bit value to write */
    width = 4;                /* always do 32 bit write */
    offset = (offset & ~0x3); /* align by 32 bits */

#if 0
    if ((busNo == 0) && (deviceNo == 0x1f))
        return(ERROR);
#endif

    switch (width)
        {
        case 1: /* byte */
            pciRegWrite ((UINT32 *)sysPciConfAddr,
                         (UINT32)pciConfigBdfPack (busNo, deviceNo, funcNo) |
                         (offset & 0xfc) | 0x80000000 | ((offset & 0xf00) << 16));
            PCI_OUT_BYTE ((sysPciConfData + (offset & 0x3)), data);
            break;

        case 2: /* word */
            pciRegWrite ((UINT32 *)sysPciConfAddr,
                         (UINT32)pciConfigBdfPack (busNo, deviceNo, funcNo) |
                         (offset & 0xfc) | 0x80000000 | ((offset & 0xf00) << 16));
            PCI_OUT_WORD ((sysPciConfData + (offset & 0x2)), data);
            break;

        case 4: /* long */
            pciRegWrite ((UINT32 *)sysPciConfAddr,
                         (UINT32)pciConfigBdfPack (busNo, deviceNo, funcNo) |
                         (offset & 0xfc) | 0x80000000 | ((offset & 0xf00) << 16));
            PCI_OUT_LONG (sysPciConfData, data);
            break;

        default:
            return(ERROR);

        }
    return(OK);
    }
#endif /* INCLUDE_PCI */

/******************************************************************************
*
* sysUsDelay - delay at least the specified amount of time (in microseconds)
*
* This routine will delay for at least the specified amount of time using the
* lower 32 bit "word" of the Time Base register as the timer.
*
* NOTE:  This routine will not relinquish the CPU; it is meant to perform a
* busy loop delay.  The minimum delay that this routine will provide is
* approximately 10 microseconds.  The maximum delay is approximately the
* size of UINT32; however, there is no roll-over compensation for the total
* delay time, so it is necessary to back off two times the system tick rate
* from the maximum.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysUsDelay
    (
    UINT32    delay        /* length of time in microsec to delay */
    )
    {
    register UINT baselineTickCount;
    register UINT curTickCount;
    register UINT terminalTickCount;
    register int actualRollover = 0;
    register int calcRollover = 0;
    UINT ticksToWait;
    UINT requestedDelay;
    UINT oneUsDelay;

    /* Exit if no delay count */

    if ((requestedDelay = delay) == 0)
        return;

    /*
     * Get the Time Base Lower register tick count, this will be used
     * as the baseline.
     */

    baselineTickCount = sysTimeBaseLGet();

    /*
     * Calculate number of ticks equal to 1 microsecond
     *
     * The Time Base register and the Decrementer count at the same rate:
     * once per 8 System Bus cycles.
     *
     * e.g., 199999999 cycles     1 tick      1 second            25 ticks
     *       ----------------  *  ------   *  --------         ~  --------
     *       second               8 cycles    1000000 microsec    microsec
     */

    /* add to round up before div to provide "at least" */

    oneUsDelay = ((sysTimerClkFreq + 1000000) / 1000000);

    /* Convert delay time into ticks */

    ticksToWait = requestedDelay * oneUsDelay;

    /* Compute when to stop */

    terminalTickCount = baselineTickCount + ticksToWait;

    /* Check for expected rollover */

    if (terminalTickCount < baselineTickCount)
        {
        calcRollover = 1;
        }

    do
        {

        /*
         * Get current Time Base Lower register count.
         * The Time Base counts UP from 0 to
         * all F's.
         */

        curTickCount = sysTimeBaseLGet();

        /* Check for actual rollover */

        if (curTickCount < baselineTickCount)
            {
            actualRollover = 1;
            }

        if (((curTickCount >= terminalTickCount)
             && (actualRollover == calcRollover)) ||
            ((curTickCount < terminalTickCount)
             && (actualRollover > calcRollover)))
            {

            /* Delay time met */

            break;
            }
        }
    while (TRUE); /* breaks above when delay time is met */
    }


void sysMsDelay
    (
    UINT      delay                   /* length of time in MS to delay */
    )
    {
    sysUsDelay ( (UINT32) delay * 1000 );
    }


/*********************************************************************
*
* sysDelay - Fixed 1ms delay.
*
* This routine consumes 1ms of time. It just calls sysMsDelay.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysDelay (void)
    {
    sysMsDelay (1);
    }

/***************************************************************************
*
* sysIntConnect - connect the BSP interrupt to the proper epic/i8259 handler.
*
* This routine checks the INT level and connects the proper routine.
* pciIntConnect() or intConnect().
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be built.
*
* ERRNO: N/A
*/

STATUS sysIntConnect
    (
    VOIDFUNCPTR *vector,        /* interrupt vector to attach to     */
    VOIDFUNCPTR routine,        /* routine to be called              */
    int parameter               /* parameter to be passed to routine */
    )
    {
    int tmpStat = ERROR;
    UINT32 read;

    if (((int) vector < 0) || ((int) vector >= EXT_VEC_IRQ0 + EXT_MAX_IRQS))
        {
        logMsg ("Error in sysIntConnect: out of range vector = %d.\n",
                (int)vector,2,3,4,5,6);

        return(ERROR);
        }

    if (vxMemProbe ((char *) routine, VX_READ, 4, (char *) &read) != OK)
        {
        logMsg ("Error in sysIntConnect: Cannot access routine.\n",
                1,2,3,4,5,6);
        return(ERROR);
        }

    if ((int) vector < EXT_VEC_IRQ0)
        {
        tmpStat = intConnect (vector, routine, parameter);
        }
    else
        {

        /* 
	 * call external int controller connect
         tmpStat = cascadeIntConnect (vector, routine, parameter);
	 */
        }

    if (tmpStat == ERROR)
        {
        logMsg ("Error in sysIntConnect: Connecting vector = %d.\n",
                (int)vector,2,3,4,5,6);
        }

    return(tmpStat);
    }


/*******************************************************************************
*
* sysIntEnable - enable an interrupt
*
* This function call is used to enable an interrupt.
*
* RETURNS: OK or ERROR if unable to enable interrupt.
*
* ERRNO: N/A
*/

STATUS sysIntEnable
    (
    int intNum
    )
    {
    int tmpStat = ERROR;
    if (((int) intNum < 0) || ((int) intNum >= EXT_NUM_IRQ0 + EXT_MAX_IRQS))
        {
        logMsg ("Error in sysIntEnable: Out of range intNum = %d.\n",
                (int)intNum,2,3,4,5,6);

        return(ERROR);
        }

    if ((int) intNum < EXT_NUM_IRQ0)
        {
        tmpStat = intEnable (intNum);
        }
    else
        {
        /* call external int controller connect */

        tmpStat = sysCascadeIntEnable (intNum - EXT_NUM_IRQ0);
        }

    if (tmpStat == ERROR)
        {
        logMsg ("Error in sysIntEnable: intNum = %d.\n",
                (int)intNum,2,3,4,5,6);
        }

    return(tmpStat);
    }

/****************************************************************************
*
* sysCascadeIntEnable - enable an external controller interrupt
*
* This function call is used to enable an interrupt outside of the MPC8540 PIC.
*
* RETURNS: OK or ERROR if unable to enable interrupt.
*
* ERRNO: N/A
*/

STATUS sysCascadeIntEnable
    (
    int intNum
    )
    {
    return(ERROR);
    }

/****************************************************************************
*
* sysIntDisable - disable an interrupt
*
* This function call is used to disable an interrupt.
*
* RETURNS: OK or ERROR if unable to disable interrupt.
*
* ERRNO: N/A
*/

STATUS sysIntDisable
    (
    int intNum
    )
    {
    int tmpStat = ERROR;
    if (((int) intNum < 0) || ((int) intNum >= EXT_NUM_IRQ0 + EXT_MAX_IRQS))
        {
        logMsg ("Error in sysIntDisable: Out of range intNum = %d.\n",
                (int)intNum,2,3,4,5,6);

        return(ERROR);
        }

    if ((int) intNum < EXT_NUM_IRQ0)
        {
        tmpStat = intDisable (intNum);
        }
    else
        {
        /* call external int controller connect */

        tmpStat = sysCascadeIntDisable (intNum - EXT_NUM_IRQ0);
        }


    if (tmpStat == ERROR)
        {
        logMsg ("Error in sysIntDisable: intNum = %d.\n",
                (int)intNum,2,3,4,5,6);
        }

    return(tmpStat);
    }

/**************************************************************************
*
* sysCascadeIntDisable - disable an external controller interrupt
*
* This function call is used to disable an interrupt outside of the MPC8540 PIC.
*
* RETURNS: OK or ERROR if unable to disable interrupt.
*
* ERRNO: N/A
*/

STATUS sysCascadeIntDisable
    (
    int intNum
    )
    {
    return(ERROR);
    }

#ifdef INCLUDE_CACHE_SUPPORT
/***********************************************************************
*
* sysL1CacheQuery - configure L1 cache size and alignment
*
* Populates L1 cache size and alignment from configuration registers.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL1CacheQuery(void)
    {
    UINT32 temp;
    UINT32 align;
    UINT32 cachesize;

    temp = vxL1CFG0Get();

    cachesize = (temp & 0xFF) << 10;

    align = (temp >> 23) & 0x3;


    switch (align)
        {
        case 0:
            ppcE500CACHE_ALIGN_SIZE=32;
            break;
        case 1:
            ppcE500CACHE_ALIGN_SIZE=64;
            break;
        default:
            ppcE500CACHE_ALIGN_SIZE=32;
            break;
        }

    ppcE500DCACHE_LINE_NUM = (cachesize / ppcE500CACHE_ALIGN_SIZE);
    ppcE500ICACHE_LINE_NUM = (cachesize / ppcE500CACHE_ALIGN_SIZE);

    /*
     * The core manual suggests for a 32 byte cache line and 8 lines per set
     * we actually need 12 unique address loads to flush the set.
     * The number of lines to flush should be ( 3/2 * cache lines )
     */

    ppcE500DCACHE_LINE_NUM = (3*ppcE500DCACHE_LINE_NUM)>>1;
    ppcE500ICACHE_LINE_NUM = (3*ppcE500ICACHE_LINE_NUM)>>1;

    }

#endif /* INCLUDE_CACHE_SUPPORT */

/***************************************************************************
*
* saveExcMsg - write exception message to save area for catastrophic error
*
* The message will be displayed upon rebooting with a bootrom.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void saveExcMsg
    (
    char *errorMsg
    )
    {
    strcpy ((char *)EXC_MSG_OFFSET, errorMsg);
    }


void chipErrataCpu29Print(void)
    {
    saveExcMsg("Silicon fault detected, possible machine state corruption.\nSystem rebooted to limit damage.");
    }



/***************************************************************************
*
* vxImmrGet - get the CPM DPRAM base address
*
* This routine returns the CPM DP Ram base address for CPM device drivers.
*
* RETURNS:
*
* ERRNO: N/A
*/

UINT32 vxImmrGet(void)
    {
    return(CCSBAR + 0x80000);
    }

/***************************************************************************
*
* sysGetPeripheralBase   - Provides CCSRBAR address fro security engine
*                          drivers.
*
* RETURNS:
*
* ERRNO: N/A
*/
UINT32 sysGetPeripheralBase()
    {
    return(CCSBAR);
    }

#if defined (_GNU_TOOL)
void sysIntHandler (void)
    {
    }

void vxDecInt (void)
    {
    }

int excRtnTbl = 0;
#endif  /* _GNU_TOOL */

#ifdef INCLUDE_SHOW_ROUTINES

/***************************************************************************
*
* coreLbcShow - Show routine for local bus controller
*
* This routine shows the local bus controller registers.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void coreLbcShow(void)
    {
    VINT32 tmp, tmp2;

    tmp = * (VINT32 *) M85XX_BR0(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR0(CCSBAR);
    printf("Local bus BR0 = 0x%x\tOR0 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR1(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR1(CCSBAR);
    printf("Local bus BR1 = 0x%x\tOR1 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR2(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR2(CCSBAR);
    printf("Local bus BR2 = 0x%x\tOR2 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR3(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR3(CCSBAR);
    printf("Local bus BR3 = 0x%x\tOR3 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR4(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR4(CCSBAR);
    printf("Local bus BR4 = 0x%x\tOR4 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR5(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR5(CCSBAR);
    printf("Local bus BR5 = 0x%x\tOR5 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR6(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR6(CCSBAR);
    printf("Local bus BR6 = 0x%x\tOR6 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR7(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR7(CCSBAR);
    printf("Local bus BR7 = 0x%x\tOR7 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_LBCR(CCSBAR);
    printf("Local bus LBCR = 0x%x\n", tmp);

    tmp = * (VINT32 *) M85XX_LCRR(CCSBAR);
    printf("Local bus LCRR = 0x%x\n", tmp);
    }

    #define xbit0(x, n)    ((x & (1 << (31 - n))) >> (31 - n))  /* 0..31 */
    #define xbit32(x, n)   ((x & (1 << (63 - n))) >> (63 - n))  /* 32..63 */
    #define onoff0(x, n)   xbit0(x, n) ? "ON", "OFF"
    #define onoff32(x, n)  xbit32(x, n) ? "ON", "OFF"

/***************************************************************************
*
* coreShow - Show routine for core registers
*
* This routine shows the core registers.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void coreShow(void)
    {
    VUINT32 tmp, tmp2;

    tmp = vxMsrGet();
    printf("MSR - 0x%x\n", tmp);
    printf("      UCLE-%x SPE-%x WE-%x CE-%x EE-%x PR-%x ME-%x\n",
           xbit32(tmp,37), xbit32(tmp,38), xbit32(tmp,45), xbit32(tmp,46),
           xbit32(tmp,48), xbit32(tmp,49), xbit32(tmp,51));
    printf("      UBLE-%x DE-%x IS-%x DS-%x PMM-%x\n",
           xbit32(tmp,53), xbit32(tmp,54), xbit32(tmp,58), xbit32(tmp,59),
           xbit32(tmp,61));
    tmp = vxHid0Get();
    tmp2 = vxHid1Get();
    printf("HID0 = 0x%x, HID1 = 0x%x\n", tmp, tmp2);
    tmp = vxL1CSR0Get();
    printf("L1CSR0: cache is %s - 0x%x\n", tmp&1?"ON":"OFF", tmp);
    tmp = vxL1CSR1Get();
    printf("L1CSR1: Icache is %s - 0x%x\n", tmp&1?"ON":"OFF", tmp);
    tmp = vxL1CFG0Get();
    tmp2 = vxL1CFG1Get();
    printf("L1CFG0 = 0x%x, L1CFG1 = 0x%x\n", tmp, tmp2);
    tmp = *(VUINT32 *) (CCSBAR + 0x20000);
    printf("L2CTL - 0x%x\n", tmp);
    printf("        l2 is %s\n", tmp&0x80000000?"ON":"OFF");
    printf("        l2siz-%x l2blksz-%x l2do-%x l2io-%x\n",
           (xbit0(tmp,2)<<1)|xbit0(tmp,3), (xbit0(tmp,4)<<1)|xbit0(tmp,5),
           xbit0(tmp,9), xbit0(tmp,10));
    printf("        l2pmextdis-%x l2intdis-%x l2sram-%x\n",
           xbit0(tmp,11), xbit0(tmp,12),
           (xbit0(tmp,13)<<2)|(xbit0(tmp,14)<<1)|xbit0(tmp,15));
    tmp = *(VUINT32 *) (CCSBAR + 0x20100);
    tmp2 = *(VUINT32 *) (CCSBAR + 0x20108);
    printf("L2SRBAR0 - 0x%x\n", tmp);
    printf("L2SRBAR1 - 0x%x\n", tmp2);

    printf("Core Freq = %3d Hz\n",coreFreq);
    printf("CCB Freq = %3d Hz\n",sysClkFreqGet());
    printf("PCI Freq = %3d Hz\n",OSCILLATOR_FREQ);
    printf("CPM Freq = %3d Hz\n",sysClkFreqGet());

    }

#endif

int sysVectorIRQ0 = 0;
void
sysReboot(void)
{
    int level;

     level = intLock();
     * (VINT32 *) M85XX_GPOUTDR(CCSBAR) = ~0x0;
     * (VINT32 *) M85XX_GPIOCR(CCSBAR) = 0x200;
     * (VINT32 *) M85XX_GPOUTDR(CCSBAR) &= ~(BCM98548XMC_SYS_RESET);
     sysMsDelay(500); /* 500 ms delay */
     * (VINT32 *) M85XX_GPOUTDR(CCSBAR) &=
                         ~(BCM98548XMC_SYS_RESET| BCM98548XMC_CPU_RESET);
 
     for(;;);
}

UINT8 sysBoardRev()
{
    UINT8 board_id;

    board_id = 0;

#ifdef INCLUDE_I2C_DEV
    if (pcf8574_boardid_get(&board_id) == 0) {
        return board_id; 
    }
#endif

    return 0;
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

void
pcieShow()
{
int min_bus =129; int max_bus = 129;
#define P_MIN_BUS       min_bus
#define P_MAX_BUS       max_bus
#define P_MAX_DEV       31
#define P_MAX_FUNC      0
#define P_MAX_OFFSET    (0x400 -1)
#define FOR_EACH_LOOP(curr_id, min_id, max_id, incr_id) \
        for(curr_id = min_id; curr_id <= max_id; curr_id += incr_id)

    int bus, devid, func, offset, val, dev_max;

    printf("BS DV FN OF VALUE\n");
    FOR_EACH_LOOP(bus, P_MIN_BUS, P_MAX_BUS, 1) {
        dev_max = (bus < PCIEX_BUS_MIN) ? P_MAX_DEV : 0;
        FOR_EACH_LOOP(devid, 0, dev_max, 1) {
            FOR_EACH_LOOP(func, 0, P_MAX_FUNC, 1) {
                FOR_EACH_LOOP(offset, 0, P_MAX_OFFSET, 4) {
                    pciConfigInLong(bus, devid, func, offset, &val);
                    if (val == 0xffffffff) {
                        break;
                    }
                    printf("%02d %02d %02d %02X %08X\n",
                                bus, devid, func, offset, val);
                }
            }
        }
    }
}

void sysPcieConfigShow()
{
    int busNo;
    int devNo;
    int funcNo;
    int data;
    int offset;

    busNo  = PCIEX_BUS_MIN + 0;
    devNo  = 0;
    funcNo = 0;

    sysPciConfigEnable (CDS85XX_PCIEX_BUS);
    for (offset = 0; offset <= 0x7c; offset = offset + 4) {
       if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
           printf("offset 0x%08x = 0x%08x\n", offset, data);
       } else {
           printf("sysPciConfigRead error.\n");
           break;
       }
    }

    printf("PCI-E Extended Configuration Space\n");
    for (offset = 0x100; offset <= 0x134; offset = offset + 4) {
       if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
           printf("offset 0x%08x = 0x%08x\n", offset, data);
       } else {
           printf("sysPciConfigRead error.\n");
           break;
       }
    }

    offset = 0x404;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
    offset = 0x440;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
    offset = 0x450;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
    offset = 0x454;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
    offset = 0x478;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
    offset = 0x4b0;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
    offset = 0x590;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
    offset = 0x5a0;
    if (sysPciConfigRead(busNo, devNo, funcNo, offset, 4, &data)== OK) {
       printf("offset 0x%08x = 0x%08x\n", offset, data);
    } else {
       printf("sysPciConfigRead error offset 0x%08x.\n", offset);
    }
}


static char * e500_ratio[] = {"reserved", "reserved", "1:1", "3:2",
                             "2:1", "5:2", "3:1", "7:2", "4:1", "9:2",
                             "reserved", "reserved", "reserved", "reserved",
                             "reserved", "reserved" };
static char * tsec_protocol[] = {"FIFO", "MII", "GMII", "TBI"};                        
static char * io_sel[] = {"reserved", "reserved", "reserved",
                          "x4 RIO 2.5Gbps, x4 PCI-E", 
                          "x4 RIO 1.25Gbps, x4 PCI-E", 
                          "x4 RIO 3.125Gbps", 
                          "x4 RIO 1.25Gbps", 
                          "x8 PCI-E"};
void sysPorStatShow()
{
    UINT reg;
    int  field; 
    
    reg = *M85XX_PORPLLSR(CCSBAR);
    field = (reg >> 16) & 0xf;
    printf("e500_ratio = %s\n", e500_ratio[field]);
    field = (reg >> 15) & 1;
    printf("PCI1_clk_sel = %s\n", field ? "SYS_CLK" : "PCI1_CLK");
    field = (reg >> 14) & 1;
    printf("PCI2_clk_sel = %s\n", field ? "SYS_CLK" : "PCI2_CLK");
    field = (reg >> 1) & 0xf;
    printf("Plat_Ratio = %d : 1\n", field);

    reg = *M85XX_PORBMSR(CCSBAR);
    field = (reg >> 31) & 1;
    printf("BCFG = %s\n", field ? "CPU immediate boot" : "CPU wait to boot");

    reg = *M85XX_PORDEVSR(CCSBAR);
    field = (reg >> 31) & 1;
    printf("eTSEC1 and eTSEC2 in %s interface mode\n", 
           field ? "full" : "reduced");
    field = (reg >> 30) & 1;
    printf("eTSEC3 and eTSEC4 in %s interface mode\n",
           field ? "full" : "reduced");
    field = (reg >> 24) & 3;
    printf("eTSEC1 operates in %s mode\n", tsec_protocol[field]); 
    field = (reg >> 12) & 3;
    printf("eTSEC2 operates in %s mode\n", tsec_protocol[field]); 
    field = (reg >> 10) & 3;
    printf("eTSEC3 operates in %s mode\n", tsec_protocol[field]); 
    field = (reg >> 8) & 3;
    printf("eTSEC4 operates in %s mode\n", tsec_protocol[field]); 
    field = (reg >> 19) & 0xf;
    printf("%s\n", io_sel[field]);

    reg = *M85XX_PORDEVSR2(CCSBAR);
    field = (reg >> 2) & 1;
    printf("Serdes %s\n", field ? "enabled" : "disabled");
}

void extIsr(int id) {
    logMsg("Interrupt %d\n", id, 0, 0, 0, 0, 0);
}

void installExtInt()
{

    sysEpicIntConnect((VOIDFUNCPTR *)(EXT_VEC_IRQ0 + 4), (VOIDFUNCPTR)extIsr, 4);
    sysEpicIntEnable((EXT_VEC_IRQ0 + 4));

    sysEpicIntConnect((VOIDFUNCPTR *)(EXT_VEC_IRQ0 + 5), (VOIDFUNCPTR)extIsr, 5);
    sysEpicIntEnable((EXT_VEC_IRQ0 + 5));

    sysEpicIntConnect((VOIDFUNCPTR *)(EXT_VEC_IRQ0 + 6), (VOIDFUNCPTR)extIsr, 6);
    sysEpicIntEnable((EXT_VEC_IRQ0 + 6));

    sysEpicIntConnect((VOIDFUNCPTR *)(EXT_VEC_IRQ0 + 7), (VOIDFUNCPTR)extIsr, 7);
    sysEpicIntEnable((EXT_VEC_IRQ0 + 7));

    sysEpicIntConnect((VOIDFUNCPTR *)(EXT_VEC_IRQ0 + 10), (VOIDFUNCPTR)extIsr, 10);
    sysEpicIntEnable((EXT_VEC_IRQ0 + 10));

    sysEpicIntConnect((VOIDFUNCPTR *)(EXT_VEC_IRQ0 + 11), (VOIDFUNCPTR)extIsr, 11);
    sysEpicIntEnable((EXT_VEC_IRQ0 + 11));
}

void gpoutSet(int pin, int val)
{
    int  key;
    UINT reg;

    key = intLock();

    reg = *M85XX_GPOUTDR(CCSBAR);

    if (val) {
        reg |= (0x80000000 >> pin);
    } else {
        reg &= ~(0x80000000 >> pin);
    } 

    *M85XX_GPOUTDR(CCSBAR) = reg;
    intUnlock(key);

}

void gpinGet()
{
    UINT reg;

    reg = *M85XX_GPINDR(CCSBAR);

    printf("PMC_EREADY = %d\n", (reg & ( 1 << 16)) ? 1 : 0); 
    printf("~P4_PRST  = %d\n", (reg & ( 1 << 17)) ? 1 : 0); 
    printf("~XMC_PRST = %d\n", (reg & ( 1 << 18)) ? 1 : 0); 
    printf("FLASH_SEL = %d\n", (reg & ( 1 << 19)) ? 1 : 0); 
    printf("XMC_ROOT0 = %d\n", (reg & ( 1 << 20)) ? 1 : 0); 
    printf("PMC_MONARCH = %d\n", (reg & ( 1 << 21)) ? 1 : 0); 
    printf("GPIO0 = %d\n", (reg & ( 1 << 22)) ? 1 : 0); 
    printf("GPIN5 = %d\n", (reg & ( 1 << 23)) ? 1 : 0); 
 

}

void autoconfig_done()
{
   printf("pci_autoconfig_done = %d\n", pci_autoconfig_done);
}

void pcieLinkPoll()
{
    int poll_count;
    int data;
    for(poll_count=1; poll_count <=1000; poll_count++) {
        if (sysPciConfigRead(128, 0, 0, 0x404, 4, &data)== OK) {
            if (data != 0x16) {
                printf("Link down on %d poll\n", poll_count);
            }
        } else {
            printf("(sysPciConfigRead(128, 0, 0, 0x404, 4, &data) Failed\n");
        }
    }
}

#ifdef INCLUDE_SYS_HW_INIT_0
IMPORT BOOL excExtendedVectors;
/***************************************************************************
*
* SYS_HW_INIT_0 - test for extended vectors
*
* This routine is used for testing extended vectors.
*
* RETURNS: N/A
*
* ERRNO
*/

void SYS_HW_INIT_0 (void)
    {
    excExtendedVectors = TRUE;
    }
#endif  /* INCLUDE_SYS_HW_INIT_0 */


#ifdef INCLUDE_FTP6_SERVER
int brcmUserVerify(char *name, char *passwd)
{
    /* allow all users */
    return 0;
}
#endif /* INCLUDE_FTP6_SERVER */

#include <symLib.h>
/******************************************************************************
*
* sysBacktracePpc - perform a stack trace of the caller with symbols
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

    rc = symFindByValue(sysSymTbl,
		       (UINT) addr,
		       symName, &symVal, &symType);
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
    char               symName[MAX_SYS_SYM_LEN+1];


    sp = vxSpGet();
    pc = vxPcGet();

    s = buf;

    strcpy(s, pfx);
    while (*s) s++;

    while (s < &buf[sizeof (buf) - 128]) {

	fn_start = pc;

	for (limit = 0; limit < SYSBT_MAX_FUNC_WORDS; limit++) {
	    /* mfspr r0, LR */
	    /* mflr    r0 */
	    if (*fn_start-- == 0x7c0802a6)
		break;
	}

	if (limit == SYSBT_MAX_FUNC_WORDS)  break;

        if ((*fn_start & 0xffff8000) != 0x94218000) /* stwu r1,x(r1); x<0 */
                fn_start++;

	fn_pro = fn_start;

	for (limit = 0; limit < SYSBT_MAX_FUNC_WORDS; limit++)
	    if ((*++fn_pro & 0xffff8000) == 0x90010000) /* stw r0,y(r1); y>0 */
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
	logMsg("%s", buf, 2, 3, 4, 5, 6);  /* Atomically print giant string */
    else {
	fputs(buf, stdout);		/* Regular print giant string */
	fflush(stdout);
    }
}

