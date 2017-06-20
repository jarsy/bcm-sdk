/* sysLib.c - BCM1250 system-dependent routines */

/* $Id: sysLib.c,v 1.6 2011/07/21 16:14:49 yshtil Exp $
 * Copyright (c) 1984-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

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
01p,23aug05,jmt  Componentize BSP for scalability
01o,03aug05,dr   Decoupled for scalability.
01n,16nov04,mdo  Documentation fixes for apigen
01m,07oct04,agf  remove use of LOCAL_MEM_LOCAL_ADRS_RUNT
01l,07oct04,agf  addt'l vxWorks 6.0 clean-up
01k,06oct04,pes  Change INCLUDE_MMU_BASIC to INCLUDE_MAPPED_KERNEL
01j,18aug04,md   PM_RESERVED_MEM is dependent on INCLUDE_EDR_PM
01i,03aug04,agf  change MMU include from FULL to BASIC
01h,23jun04,agf  remove exc vector init, handled by arch code
01k,17may04,agf  AIM AD-MMU support (kernel in KSEG2)
01j,03oct02,agf  changes for shared sentosa support
01i,18jul02,pgh  Use R4K library timer.
01h,25jun02,pgh  Add calls to enable L2 cache.
01g,20jun02,pgh  Change path to bcm1250Lib.h.
01f,13mar02,agf  remove cond compiles for obj module loader types,  SPR 73892
                 change SM_OBJ conditional compiles to SM_COMMON, SPR 74321
01e,21jan02,tlc  Remove specialization of sysBusTas() by removing SYS_BUS_TAS
                 macro.
01d,04jan02,agf  add nvRAM support supplied by Z.Chen
01d,17jan02,agf  make sysForceLink vars global so diab will not optimize them
                 out
01c,20dec01,agf  add references to symbols in bcm1250L2Cache.s and
                 bcm1250DramInit.s to make sure they are in the partial link
                 objects when building vxWorks_rom et al
01c,20dec01,tlc  Remove unecessary include files.
01b,07dec01,agf  remove vestigial #if 0 code from sysHwInit2
01a,15nov01,agf  written.
*/

/*
DESCRIPTION

This library provides board-specific routines for the Broadcom BCM1250-
swarm evaluation board.

INCLUDE FILES
*/

/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <version.h>
#include <ctype.h>
#include <cacheLib.h>
#include <fppLib.h>
#include <ioLib.h>
#include <intLib.h>
#include <sysLib.h>
#include "config.h"
#include "nsx.h"

#include <sysSymTbl.h>
#include "symLib.h"

#include <string.h>

#ifdef INCLUDE_PCI
#include "pciAutoConfigLib.h"
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciIntLib.h"
#include "bcm1250PciLib.h"
#endif /* INCLUDE_PCI */

#ifdef INCLUDE_SM_COMMON
#include <smLib.h>
#include <smUtilLib.h>

#include <bootLib.h>
#include <bootLoadLib.h>
#include <bootElfLib.h>
#include <fioLib.h>
#include <ftpLib.h>
#include <tftpLib.h>
#include <remLib.h>
#endif

#include <arch/mips/fppMipsLib.h>
#include "bcm1250DuartSio.h"
#include "bcm1250JTAGSio.h"
#include "bcm1250IntLib.h"
#include "bcm1250Lib.h"

#ifdef INCLUDE_NVRAM
#include "sysNvRam.h"
#endif  /* INCLUDE_NVRAM */
#if defined(BCM1250_SWARM)
#include "swarm.h"
#elif defined(BCM1250_SENTOSA)
#include "sentosa.h"
#endif  /* defined(BCM1250_SWARM) */

#if defined(INCLUDE_MAPPED_KERNEL)
#include "vmLib.h"
#endif

/*#define INCLUDE_VXMP_TESTS*/

#ifdef INCLUDE_VXMP_TESTS
#include "semSmLib.h"
#include "smNameLib.h"
#endif /* INCLUDE_VXMP_TESTS */

#include "m41t81clock.h"

#ifdef  INCLUDE_FLASH
#include "flashDrvLib.h"
#endif

/* defines */
#define SYS_BUS_TAS                   /* override of sysMipsLib.c */

#define RSHD            514     /* rshd service */

/* externals */

IMPORT void 	fpIntr ();
IMPORT int  	sysFpaAck ();

IMPORT int      taskSRInit();
IMPORT void     sysClearTlbEntry ();
IMPORT int      sysCompareSet ();
IMPORT int      sysPridGet ();
IMPORT void     sysAltGo ();

IMPORT int	palCoreNumGet ();

#if	(INT_PRIO_MSB == TRUE)
IMPORT UINT8    ffsMsbTbl[];            /* Msb high interrupt hash table */
#else	/* INT_PRIO_MSB == TRUE */
IMPORT UINT8    ffsLsbTbl[];            /* Lsb high interrupt hash table */
#endif	/* INT_PRIO_MSB == TRUE */

IMPORT int	sysConfigSet ();
IMPORT int	sysConfigGet ();
IMPORT int	sysConfig1Get ();

IMPORT void     sb1CacheExcVec (void *);
IMPORT void     excNormVecInit (void *);
IMPORT void     excTlbVecInit (void *);
IMPORT void     excXtlbVecInit (void *);
IMPORT void     excCacheVecInit (void *);
IMPORT STATUS   cacheSb1LibInit (CACHE_MODE, CACHE_MODE, \
                                UINT32, UINT32, UINT32, UINT32);

IMPORT int      bcm1250_dram_init (void);
IMPORT void     bcm1250_l2cache_init (void);

IMPORT STATUS   sysCacheInit( CACHE_MODE , CACHE_MODE );
LOCAL int cacheSize[] = { 64, 128, 256, 512, 1024, 2048, 4096, 0 };
LOCAL int cacheLineSize[] = { 0, 4, 8, 16, 32, 64, 128, 0 };
LOCAL int cacheAssoc[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
/* forward declarations */

#ifdef INCLUDE_SM_OBJ
STATUS	sysSmObjInit (void);
#endif

void sysPciTlbSet(void);
void sysFlashTlbSet(void);

/* globals */

int	sysMemTopDebug = 0;
int   	sysVectorIRQ0 = 0;      /* used by SAL for pci int connect */

/* globals */

/*
*  Since tying interrupt lines to the processor is board dependent sysHashOrder
*  is provided to select the prioritization of interrupt lines.  Setting the
*  #define INT_PRIO_MSB to TRUE prioritizes interrupts from 7-0, 7 being
*  highest priority, 0 being lowest.  Setting it to FALSE prioritizes
*  interrupts from 0-7.  See idts465.h for the definition of INT_PRIO_MSB.
*/

#if     (INT_PRIO_MSB == TRUE)
UINT8 * sysHashOrder = ffsMsbTbl;       /* interrupt prio., 7 = high 0 = low */
#else   /* INT_PRIO_MSB == TRUE */
UINT8 * sysHashOrder = ffsLsbTbl;       /* interrupt prio., 0 = high 7 = low */
#endif  /* INT_PRIO_MSB == TRUE */

/*
 *  This table is critical to interrupt processing.  Do not alter
 *  its contents until you understand the consequences. Refer to the
 *  Tornado for MIPS Architecture supplement for instructions
 *  on its use.
 */

typedef struct 
    {
    ULONG	intCause;		/* cause of interrupt	*/
    ULONG	bsrTableOffset; 	/* index to BSR table	*/
    ULONG	statusReg;		/* interrupt level	*/
    ULONG	pad;			/* pad for ease of use	*/
    } PRIO_TABLE;

PRIO_TABLE intPrioTable[8] = 
    {
    {CAUSE_SW1,(ULONG) IV_SWTRAP0_VEC, 0x0100, 0},  /* sw trap 0    */
    {CAUSE_SW2,(ULONG) IV_SWTRAP1_VEC, 0x0200, 0},  /* sw trap 1    */
    {CAUSE_IP3,(ULONG) IV_INT0_VEC, 0x0400, 0},     /* INT 0        */
    {CAUSE_IP4,(ULONG) IV_INT1_VEC, 0x0800, 0},     /* INT 1        */
    {CAUSE_IP5,(ULONG) IV_INT2_VEC, 0x1000, 0},     /* INT 2        */
    {CAUSE_IP6,(ULONG) IV_INT3_VEC, 0x2000, 0},     /* INT 3        */
    {CAUSE_IP7,(ULONG) IV_INT4_VEC, 0x4000, 0},     /* INT 4        */
    {CAUSE_IP8,(ULONG) IV_INT5_VEC, 0x8000, 0}      /* INT 5        */
    };


LOCAL unsigned char sysEnetAddr[6];
LOCAL int sysEnetAddrInitialized = FALSE;

#ifdef BROADCOM_BSP
#define PCI_MEMORY_SPACE    0x0040000000  /* must be 32Mb boundary */
#define PCI_TLB_PAGE_SIZE   0x0002000000  /* 32M */

#define TLB_FLASH_BASE_ADDRESS 0x1E000000  /* 32M boundary */
#define TLB_FLASH_MAP_SIZE     0x04000000  /* 64M */

#endif

/*
 * Virtual Memory definitions
 */

#if defined(INCLUDE_MAPPED_KERNEL)

#define VM_STATE_MASK_FOR_ALL \
	VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE
#define VM_STATE_FOR_IO \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT
#define VM_STATE_FOR_MEM_OS \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE
#define VM_STATE_FOR_MEM_APPLICATION \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE
#define VM_STATE_FOR_PCI \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT

/*
 * sysPhysMemDesc[] entries:
 * defines the memory space on the card
 */

PHYS_MEM_DESC sysPhysMemDesc [] =
    {

    /* RAM */
    {
    (VIRT_ADDR) LOCAL_MEM_LOCAL_ADRS, /* virtual Addr */
    (PHYS_ADDR) KM_TO_PHYS(LOCAL_MEM_LOCAL_ADRS), /* Physical Addr */  
    LOCAL_MEM_SIZE,             /* length */
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_MEM_OS
    }
    };

/* number Mmu entries to be mapped */

int sysPhysMemDescNumEnt = NELEMENTS(sysPhysMemDesc); 

#endif /* defined(INCLUDE_MAPPED_KERNEL) */


/* locals */

/* Included Generic MIPS Support code */
#include "sysMipsLib.c"

/* Included drivers */

#ifdef INCLUDE_PCI
#include "pci/pciConfigLib.c"	/* standard PCI config space access */
#include "pciAutoConfigLib.c"
#include "pci/pciIntLib.c"
#include "pci/pciConfigShow.c"
#endif /* INCLUDE_PCI */

#if defined(INCLUDE_TTY_DEV) || defined(INCLUDE_SIO_POLL)
/* serial driver */
#include "sysSerial.c"
#endif	/* INCLUDE_TTY_DEV || INCLUDE_SIO_POLL */

/* timer driver */
#if 0
#include "timer/mipsR4kTimer.c"
#else
#include "sysTimer.c"
#include "bcm1250Timer.h"
#endif

/* Additional Components */
#ifdef INCLUDE_NVRAM
#if (NV_RAM_SIZE == NONE)
#  include <mem/nullNvRam.c>
#else
/* nvRAM driver */
# include "x1240RtcEeprom.c"
# include "m24lv128Eeprom.c"
# include "sysNvRam.c"
#endif
#else
/******************************************************************************
*
* sysNvRamGet - get the contents of non-volatile RAM
*
* This routine copies the contents of non-volatile memory into a specified
* string.  The string will be terminated with an EOS.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysNvRamGet
    (
    char *string,    /* where to copy non-volatile RAM           */
    int  strLen,     /* maximum number of bytes to copy          */
    int  offset      /* byte offset into non-volatile RAM        */
    )
    {
#if 0
    char s[NV_RAM_SIZE];
    int key;
    int zz;

    if (strLen < 0 || offset < 0 || offset + strLen > NV_RAM_SIZE)
	return (ERROR);

    bzero (s, NV_RAM_SIZE);
    key = intLock ();
    zz = nvram_getenvb ("bootline", s, NV_RAM_SIZE);
    intUnlock (key);
    strncpy (string, s+offset, strLen);
    string[strLen] = EOS;

    return (OK);
#else
    return(ERROR);
#endif
    }

/*******************************************************************************
*
* sysNvRamSet - write to non-volatile RAM
*
* This routine copies a specified string into non-volatile RAM.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range.
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
#if 0
    char s[NV_RAM_SIZE+1];
    int key;

    if (strLen < 0 || offset < 0 || offset + strLen > NV_RAM_SIZE)
	return (ERROR);

    key = intLock ();
    sysNvRamGet (s, NV_RAM_SIZE, 0);
    strncpy (s+offset, string, strLen);

    nvram_setenvb ("bootline", s, NV_RAM_SIZE);
    intUnlock (key);

    return (OK);
#else
    return(ERROR);
#endif
    }



#endif  /* INCLUDE_NVRAM */

/*******************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the string "Broadcom BCM1250".
*
* ERRNO
*/

char *sysModel (void)
    {
    return ("Broadcom BCM1250");
    }


/******************************************************************************
*
* sysBspRev - return the bsp version with the revision eg 1.1/<x>
*
* This function returns a pointer to a BSP version with the revision.
* for eg. 1.1/<x>. BSP_REV is concatenated to BSP_VERSION to form the
* BSP identification string.
*
* RETURNS: A pointer to the BSP version/revision string.
*
* ERRNO
*/

char * sysBspRev (void)
    {
    return (BSP_VERSION BSP_REV);
    }

/*******************************************************************************
*
* sysCpuNumGet - get the BCM1250 CPU number
*
* This routine gets the CPU number for this CPU from the PRID register.
*
* RETURNS: CPU number
*/

int sysCpuNumGet (void)
    {
    return ((sysPridGet () >> 25) & 0x7);
    }

/**********************************************************************
 * Detect HHB4 / DC21150 PCI-PCI bridge chips, and configure bus if
 * found. Note that this code should be run whenever we find a PCI
 * class code of type bridge. After we have run this procedure for the
 * first bridge, we then need to traverse any sub-bridges. This code
 * deals with only one PCI-PCI bridge and it's IO space for
 * sub-devices. See mousse.h for more details as well as the PCI-PCI
 * bridge specification.
 ***********************************************************************/
#ifdef INCLUDE_PCI
int sysPCIBridgeProbe(int instance,
                      int primary,
                      int secondary,
                      int subordinate)
{
    int BusNo, DevNo, FuncNo;
    unsigned int buses = 0;

    /* Find DC21150 PCI-PCI bridge or HINTCORP HB4 PCI-PCI Bridge */
    if ( (pciFindDevice(PERICOM_VENDOR_ID,
                        PERICOM_DEVICE_ID,
                        instance,
                        &BusNo, &DevNo, &FuncNo) != ERROR) ||

         (pciFindDevice(DC21150_VENDOR_ID,
                        DC21150_DEVICE_ID,
                        instance,
                        &BusNo, &DevNo, &FuncNo) != ERROR) ||

         (pciFindDevice(HINT_HB4_VENDOR_ID,
                        HINT_HB4_DEVICE_ID,
                        instance,
                        &BusNo, &DevNo, &FuncNo) != ERROR) ) {

        /* Disable device */
        pciConfigOutWord(BusNo,DevNo,FuncNo, PCI_CFG_COMMAND, 0x0000);
        pciConfigOutWord(BusNo,DevNo,FuncNo, PCI_CFG_STATUS, 0xffff);

        /* Reset secondary bus */
        pciConfigOutWord(BusNo, DevNo, FuncNo,
                         PCI_CFG_BRIDGE_CONTROL,
                         0x0040);

        /* Setup topology info */
        buses = (buses & 0xff000000)
            | ((unsigned int)(primary)     <<  0)
            | ((unsigned int)(secondary)   <<  8)
            | ((unsigned int)(subordinate) << 16);
        /*
         * We need to blast all three values with a single write.
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
#endif


/*******************************************************************************
*
* sysHwInit - initialize the CPU board hardware
*
* This routine initializes various features of the BCM1250
* It is called from usrInit() in usrConfig.c.
*
* This routine initializes and turns off the timers.
*
* NOTE:
* This routine should not be called directly by the user.
* 
* RETURNS: N/A
*
* ERRNO
*/
void sysHwInit (void)
    {
    int sr;

    sr = BCM1250_SR;

#ifdef BROADCOM_BSP
    /*
     * Hacking to set taskSrDefault to BCM1250_SR.
     * There seems to be a bug in libarch.a for bcm1250 architecture.
     */
    extern ULONG taskSrDefault, taskSrUserDefault, _mipsSrImask;
    taskSrDefault = taskSrUserDefault = sr;
    _mipsSrImask = SR_IMASK0;

    /*
     * Another hack to overwrite the default MIPS cache routines
     * as per suggestion by windriver.
     */
    sysCacheInit(USER_I_CACHE_MODE, USER_D_CACHE_MODE);
#else
    /* init status register but leave interrupts disabled */
    taskSRInit (sr);
#endif
    intSRSet (sr & ~SR_IE);

    /* set the Processor number based on the CPU ID */
    sysProcNumSet (palCoreNumGet());

    /* make sure there is a valid boot string if running a secondary cpu */
    if (palCoreNumGet () != 0)    
	{
        strcpy (sysBootLine, DEFAULT_BOOT_LINE);
	}

#ifdef	INCLUDE_HW_FP
    /* initialize floating pt unit */
    if (fppProbe () == OK)
	{
	fppInitialize ();
	intVecSet ((FUNCPTR *)INUM_TO_IVEC (IV_FPE_VEC), (FUNCPTR) fpIntr);
	}
#endif	/* INCLUDE_HW_FP */

BPRINT("X3  ");
#if defined(INCLUDE_TTY_DEV) || defined(INCLUDE_SIO_POLL)
    /* Setup the serial device descriptors. */
    sysSerialHwInit ();
#endif	/* INCLUDE_TTY_DEV || INCLUDE_SIO_POLL */

BPRINT("X4  ");
    /* map TLB for PCI memory space */
    sysPciTlbSet();
    sysFlashTlbSet();
    
BPRINT("X5  ");
#if defined (INCLUDE_PCI)
    /* Initialize the PCI and LDT here (following sysLib for sdb4122eagle). */
    sysPciConfig ();
    pciIntLibInit ();

    /* Initialize the host bridges. */

BPRINT("X6  ");
    sysHostBridgeInit ();

#endif /*INCLUDE_PCI*/

BPRINT("X7  ");
    /* ensure the timers are quiescent */
    MIPS3_SD(PHYS_TO_K1(A_SCD_TIMER_REGISTER(sysClkUnit,    R_SCD_TIMER_CFG)), 0);

    MIPS3_SD(PHYS_TO_K1(A_SCD_TIMER_REGISTER(sysAuxClkUnit, R_SCD_TIMER_CFG)), 0);

BPRINT("X8  ");
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
* ERRNO
*/
void sysHwInit2 (void)
    {
    /* initialize the bcm1250 Interrupt Mapper handler */

    BPRINT("HIN2");
    bcm1250IntLibInit();

#if defined(INCLUDE_TTY_DEV) || defined(INCLUDE_SIO_POLL)
    /* connect serial channel interrupts */

    sysSerialHwInit2();
#endif	/* INCLUDE_TTY_DEV || INCLUDE_SIO_POLL */

#ifdef INCLUDE_PCI
    /* Size the LDT fabric; configure PCI and LDT */
    /* sysPciAutoConfig (); */

    /* Device-specific PCI initialization might go here. */
#endif /*INCLUDE_PCI*/

#ifdef INCLUDE_SM_OBJ
    (void) sysSmObjInit ();
#endif
    }


/***************************************************************************
*
* sysCacheInit - initialize the MIPS cache library
*
* This routine sets the six global cache size variables indicating the sizes
* and line sizes of the primary data, primary instruction, and secondary
* caches, and then calls the cacheInit routine.
*
* RETURNS: OK or ERROR
*
* NOMANUAL
*/

STATUS sysCacheInit
    (
    CACHE_MODE instMode,
    CACHE_MODE dataMode
    )
    {

    int config1 = sysConfig1Get();
    int iCacheSize, iCacheLineSize, iCacheAssoc;
    int dCacheSize, dCacheLineSize, dCacheAssoc;
   
    iCacheSize =
      cacheSize[((config1 & SB1_CONFIG1_IS_MSK) >> SB1_CONFIG1_IS_SHF)];
    iCacheLineSize =
      cacheLineSize[((config1 & SB1_CONFIG1_IL_MSK) >> SB1_CONFIG1_IL_SHF)];
    iCacheAssoc =
      cacheAssoc[((config1 & SB1_CONFIG1_IA_MSK) >> SB1_CONFIG1_IA_SHF)];
    iCacheSize *= (iCacheLineSize * iCacheAssoc);

    dCacheSize =
      cacheSize[((config1 & SB1_CONFIG1_DS_MSK) >> SB1_CONFIG1_DS_SHF)];
    dCacheLineSize =
      cacheLineSize[((config1 & SB1_CONFIG1_DL_MSK) >> SB1_CONFIG1_DL_SHF)];
    dCacheAssoc =
      cacheAssoc[((config1 & SB1_CONFIG1_DA_MSK) >> SB1_CONFIG1_DA_SHF)];
    dCacheSize *= (dCacheLineSize * dCacheAssoc);

    return cacheSb1LibInit(
        instMode,       /* I-cache mode */
        dataMode,       /* D-cache mode */
        iCacheSize,
        iCacheLineSize,
        dCacheSize,
        dCacheLineSize
        );

    }
/*******************************************************************************
*
* sysPhysMemTop - get the address of the top of memory
*
* This routine returns the address of the first missing byte of memory, which
* indicates the top of memory.
*
* NOTE: Do not adjust LOCAL_MEM_SIZE to reserve memory for application
* use.  See sysMemTop() for more information on reserving memory.
*
* RETURNS: The address of the top of memory.
*
* ERRNO
*/

char *sysPhysMemTop (void)
    {
    static char * memTop = NULL;

    if (memTop == NULL)
	{
	memTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
	}
    return memTop;
    }

/*******************************************************************************
*
* sysMemTop - get the address of the top of logical memory
*
* This routine returns the address of the first unusable byte of memory.
* VxWorks will not use any memory at or above this address.
*
* The user can reserve local memory from the board by declaring the
* macro USER_RESERVED_MEM with the amount of memory to reserve. This
* routine will return a pointer to the first byte of the reserved memory
* area.
*
* RETURNS: The address of the top of usable memory.
*
* ERRNO
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

/*******************************************************************************
*
* sysAutoAck - acknowledge the R4000 interrupt condition 
*
* This routine acknowledges an R4000 interrupt for a specified interrupt
* vector.
*
* NOTE:
* This routine must be provided on all R4000 board support packages.
* Most interrupts are automatically acknowledged in the interrupt service
* routine.
*
* RETURNS: The result of the interrupt acknowledge cycle.
*
* ERRNO
*/

int sysAutoAck
    (
    int vecNum		/* vector num of interrupt that bugs us */
    )
    {
    int result;

    result = 0;
    switch (vecNum)
	{
    case IV_INT5_VEC:
        sysCompareSet (0);		/* reset count/compare interrupt */
        break;
	case IV_SWTRAP0_VEC:		/* software trap 0 */
	    return(result = sysSw0Ack ());
	    break;

	case IV_SWTRAP1_VEC:		/* software trap 1 */
	    return(result = sysSw1Ack ());
	    break;

	case IV_FPA_UNIMP_VEC:		/* unimplemented FPA oper*/
	case IV_FPA_INV_VEC:		/* invalid FPA operation*/
	case IV_FPA_DIV0_VEC:		/* FPA div by zero */
	case IV_FPA_OVF_VEC:		/* FPA overflow exception */
	case IV_FPA_UFL_VEC:		/* FPA underflow exception */
	case IV_FPA_PREC_VEC:		/* FPA inexact operation */
            return(result = sysFpaAck ());
	    break;

	default:
            return(-1);
        break;
	}
    return(result);
    }

#ifdef	INCLUDE_SHOW_ROUTINES
/*******************************************************************************
*
* sysDisplayMem - display memory
*
* Display contents of memory, starting at adrs.  Memory is displayed in
* words.  The number of words displayed defaults to 64.  If
* nwords is non-zero, that number of words is printed, rounded up to
* the nearest number of full lines.  That number then becomes the default.
*
* RETURNS: OK always
*
* ERRNO
*/

STATUS sysDisplayMem
    (
    FAST char *adrs,	/* address to display */
    int	       nwords	/* number of words to print. */
    )			/* If 0, print 64 or last specified. */
    {
    static char *last_adrs;
    static int dNbytes = 128;
    char ascii [17];
    FAST int nbytes;
    FAST int byte;

    ascii [16] = EOS;			/* put an EOS on the string */

    nbytes = 2 * nwords;

    if (nbytes == 0)
	nbytes = dNbytes;	/* no count specified: use current byte count */
    else
	dNbytes = nbytes;	/* change current byte count */

    if (adrs == 0)
	adrs = last_adrs;	/* no address specified: use last address */

    adrs = (char *) ((int) adrs & ~1);	/* round adrs down to word boundary */


    /* print leading spaces on first line */

    bfill ((char *) ascii, 16, '.');

    printf ("%06x:  ", (int) adrs & ~0xf);

    for (byte = 0; byte < ((int) adrs & 0xf); byte++)
	{
	printf ("  ");
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	ascii[byte] = ' ';
	}

    /* print out all the words */
    while (nbytes-- > 0)
	{
	if (byte == 16)
	    {
	    /* end of line:
	     *   print out ascii format values and address of next line */

	    printf ("  *%16s*\n%06x:  ", ascii, (int) adrs);

	    bfill ((char *) ascii, 16, '.');	/* clear out ascii buffer */
	    byte = 0;				/* reset word count */
	    }

	printf ("%02x", *adrs & 0x000000ff);
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	if ( *adrs == ' ' || 
             ( isascii (*adrs) && isprint ((int)(*adrs)) )
           )
	    ascii[byte] = *adrs;

	adrs++;
	byte++;
	}

    /* print remainder of last line */
    for (; byte < 16; byte++)
	{
	printf ("  ");
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	ascii[byte] = ' ';
	}

    printf ("  *%16s*\n", ascii);	/* print out ascii format values */
    last_adrs = adrs;

    return(OK);
    }

/*******************************************************************************
*
* showBits - debugging function to show bits of a 64-bit quantity
* 
* This routine displays the bits of the supplied 64-bit value. 
*
* RETURNS: N/A
*
* ERRNO
*/

void showBits
    (
    long long *ptr
    )
    {
    unsigned long long mask = ((long long) 1 << 63);
    int count = 0;

    while (mask)
	{
	putchar ((*ptr & mask) ? '1' : '-');
	mask >>= 1;
	count++;
	if (!(count & 3))
	    putchar ('.');
	}
    putchar ('\n');
    }
#endif	/* INCLUDE_SHOW_ROUTINES */


#ifdef INCLUDE_SM_COMMON
/*******************************************************************************
*
* sysMailboxConnect - specifies interrupt routine fro each mailbox interruption.
*
* This routine specifies the interrupt service routine to be called at each
* mailbox interrupt.
*
* RETURNS: ERROR, since there is no mailbox facility.
*
* ERRNO
*
* SEE ALSO: sysMailboxEnable()
*/

STATUS sysMailboxConnect
    (
    FUNCPTR routine,    /* routine called at each mailbox interrupt */
    int     arg         /* argument with which to call routine      */
    )
    {
    return (ERROR);
    }

/******************************************************************************
*
* sysMailboxEnable - enable the mailbox interrupt
*
* This routine enables the mailbox interrupt.
*
* RETURNS: ERROR, since there is no mailbox facility.
*
* ERRNO
*
* SEE ALSO: sysMailboxConnect()
*/

STATUS sysMailboxEnable
    (
    char *mailboxAdrs           /* mailbox address */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysSmIntGen - Called when we should interrupt the other CPU.
* 
* This routine is used by the SM layer to interrupt another CPU
* in the system.
*
* RETURNS: OK, always
*
* ERRNO
*/

STATUS sysSmIntGen
    (
    int arg1,	/* from SM_INT_ARG1 - the mailbox bit to set */
    int arg2,	/* from SM_INT_ARG2 - unused */
    int arv3	/* from SM_INT_ARG3 - unused */
    )
    {
    unsigned long long *pMboxSet = (unsigned long long *)
      (K1BASE | A_IMR_REGISTER(sysProcNum ^ 1,  R_IMR_MAILBOX_SET_CPU));

    /* set the mailbox interrupt */

    *pMboxSet = arg1;
    return OK;
    }

/*******************************************************************************
*
* sysSmInt - Called when we are interrupted by another CPU.
* 
* sysSmInt - Called when we are interrupted by the other CPU.  This
* function simply passes the call along to the SM layer.  The provided
* parameter was registered when we connected to the interrupt, and is
* the bit value to use to clear this mailbox interrupt. 
*
* RETURNS: N/A
*
* ERRNO
*/

void sysSmInt
    (
    int parameter
    )
    {
    unsigned long long *pMboxClear = (unsigned long long *)
      (K1BASE | A_IMR_REGISTER(sysProcNum,  R_IMR_MAILBOX_CLR_CPU));

    /* give the interrupt to the SM layer */

    smUtilIntRoutine ();

    /* clear the mailbox interrupt */
    
    *pMboxClear = parameter;
    }

#ifdef INCLUDE_SM_OBJ
/*******************************************************************************
*
* sysSmObjInit - Initialize the SM layer.
*
* This function is called from sysHwInit2, this function sets up the
* BSP for use with SM objects.  We must accomplish two tasks.  First,
* we must hook in our interrupt-generation routine (sysSmObjInterrupt), 
* and second, we must register as a receiver for interrupts from the 
* other CPU.
*
* RETURNS: N/A
*
* ERRNO
*/

STATUS sysSmObjInit (void)
   {
   STATUS status;

    /* provide our interrupt-generation code to the SM layer. The SM
     * layer knows to call this function because of the SM_INT_TYPE
     * definition in config.h. 
     */
       
    smUtilUser1Rtn = (FUNCPTR) sysSmIntGen;

    /* 
     * register our interrupt receiver - use first bit of the mailbox,
     * so we need to connect to mbox_int_3 
     */

    status = bcm1250IntConnect(ILVL_BCM1250_MBOX_INT_3,  /* bits 15:0 */
			      IV_INT4_VEC,	  /* somewhat arbitrary */
			      (VOIDFUNCPTR) sysSmInt,
			      SM_INT_ARG1);
    printf("status 0 %d\n",status);
    if (status != OK)
	return status;

    status = bcm1250IntEnable (ILVL_BCM1250_MBOX_INT_3);
    printf("status 1 %d\n",status);

    return status;
    }
#endif /* INCLUDE_SM_OBJ */

#ifdef BCM1250_CPU_0
#ifdef	INCLUDE_BOOT_LINE_INIT
/*******************************************************************************
*
* sysBootSec - this function loads cpu1's runtime kernel and starts it running
*
* This function can be called from the target shell or made to run automatically
* after finishes booting via the INCLUDE_DEMO functionality. It loads the 
* run-time kernel for the secondary cpu from across the network, then passes
* the entry address to the secondary cpu via the mailbox register.
*
* 5 parameters are required to load the kernel image from across the network:
*    hostName - computer which will be serving the kernel image file
*    fileName - file name of the kernel image
*    hostType - protocol the host uses to serve the image file (rsh/TFTP/FTP)
*    usrName  - user name required to login to the host
*    passwd   - password required to login to the host
*
* Except for the fileName, which can optionally be supplied when this function
* is called, all the other parameters are picked up from cpu0's BOOT_LINE.
*
* RETURNS: OK, or ERROR
*
* ERRNO
*/

STATUS sysBootSec
    (
    char * kernelFile
    )
    {
    BOOT_PARAMS params;
    FUNCPTR entry;
    int fd;
    int errFd;          /* for receiving standard error messages from Unix */
    char command [100];
    BOOL bootFtp = FALSE;
    BOOL bootRsh = FALSE;


    /* initialize object module loader */

    bootElfInit ();              /* MIPS only uses elf format */


    /* use cpu0's boot string as a starting point */

    bootStringToStruct (BOOT_LINE_ADRS, &params);

    bootFtp = (params.passwd[0] != EOS);

    /*
     * if a kernelFile is specified use it, 
     * otherwise append a '1' to the filename of cpu0's BOOT_LINE
     */

    if (kernelFile != EOS)
        {
        sprintf(params.bootFile, "%s", kernelFile) ;
        }
    else
        {
        sprintf(params.bootFile, "%s1", params.bootFile) ;
        }

    /*
     * begin the kernel image download process
     */

    printf ("Secondary kernel image is %s\n", params.bootFile);
    printf ("Loading kernel... ");

#ifdef INCLUDE_TFTP_CLIENT
    if (params.flags & SYSFLG_TFTP)         /* use tftp to get image */
       {
       if (tftpXfer (params.had, 0, params.bootFile, "get", "binary", 
                      &fd, &errFd) == ERROR)
           return (ERROR);
       }
    else
#endif
       {
       if (bootFtp)
           {

           if (ftpXfer (params.had, params.usr, params.passwd, "", "RETR %s",
                        "", params.bootFile, &errFd, &fd) == ERROR)
               return (ERROR);
           }
       else
           {
           bootRsh = TRUE;
           sprintf (command, "cat %s", params.bootFile);

           fd = rcmd (params.had, RSHD, params.usr, params.usr, 
                      command, &errFd);
           if (fd == ERROR)
               return (ERROR);
           }
       }

    if (bootLoadModule (fd, &entry) != OK)
        goto readErr;

    printf("Starting cpu1 at 0x%x\n\n", (int)entry);


#ifdef INCLUDE_TFTP_CLIENT
    /*
     * Successful TFTP transfers don't need any cleanup. The tftpXfer()
     * routine closes all file descriptors once the data has been
     * retrieved from the remote host.
     */

    if (params.flags & SYSFLG_TFTP)
        {
        sysAltGo (entry);
        return (OK);
        }
#endif


    if (bootRsh == FALSE)
        {

        /* Empty the Data Socket before close. PC FTP server hangs otherwise */

        while ((read (fd, command, sizeof (command))) > 0);

        if (bootFtp)
            (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
        }

    close (fd);
    close (errFd);
    sysAltGo (entry);
    return (OK);

readErr:
    /* check standard error on Unix */

    if (bootRsh == FALSE)
        {

        /* Empty the Data Socket before close. PC FTP server hangs otherwise */

        while ((read (fd, command, sizeof (command))) > 0);

        if (bootFtp)
            {
            (void) ftpReplyGet (errFd, FALSE); /* error message on std. err */
            (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
            }
        }
    else
        {
        char buf [100];
        int errBytesRecv = fioRead (errFd, buf, sizeof (buf));

        if (errBytesRecv > 0)
            {
            /* print error message on standard error fd */

            buf [errBytesRecv] = EOS;
            printf ("\n%s:%s: %s\n", params.had, params.bootFile, buf);
            }
        }

    close (fd);
    close (errFd);

    return (ERROR);
    }
#endif	/* INCLUDE_BOOT_LINE_INIT */
#endif /* BCM1250_CPU_0 */


/*------------------------------------------------------------------------*/

#ifdef INCLUDE_VXMP_TESTS

#define SEM_NAME "mySharedSem"
#define DELAY_TICKS 200

/*******************************************************************************
*
* semTask1 - User-callable test function for shared semaphores.
*
* This routine implements the "first CPU" part of the shared semaphore
* test.  It is designed to run in parallel with semTask2.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* SEE ALSO: semTask2
*/

STATUS semTask1 (void)
    {
    int thisTick;
    SEM_ID semSmId;

    /* created shared semaphore */

    if ((semSmId = semBSmCreate (SEM_Q_FIFO, SEM_FULL)) == NULL)
	{
	printf( "semTask1: can't create shared semaphore\n");
	return ERROR;
	}

    /* add object to name database */

    if (smNameAdd (SEM_NAME, semSmId, T_SM_SEM_B) == ERROR)
	{
	printf ("semTask1: can't name the semaphore\n");
	return ERROR;
	}

    /* grab shared semaphore and hold it for a while */

    semTake (semSmId, WAIT_FOREVER);

    /* normally do something useful */

    printf( "Task1 has the shared semaphore\n");
    for (thisTick = 0; thisTick < DELAY_TICKS; thisTick++)
	{
	printf( "taskDelay(tick %d of %d)\n", thisTick, DELAY_TICKS);
	taskDelay (1);
	}

    printf( "Task1 has released the shared semaphore\n");
    semGive (semSmId);

    printf( "Task1 killing time\n");
    for (thisTick = 0; thisTick < DELAY_TICKS / 2; thisTick++)
	{
	printf( "taskDelay(tick %d of %d)\n", thisTick, DELAY_TICKS / 2);
	taskDelay (1);
	}


    printf( "Task1 retaking shared semaphore\n");

    /* grab shared semaphore and hold it for a while */

    semTake (semSmId, WAIT_FOREVER);

    printf( "Task1 now has the shared semaphore\n");

    return OK;
    }

/*******************************************************************************
*
* semTask2 - User-callable test function for shared semaphores.
*
* This routine implements the "second CPU" part of the shared semaphore
* test.  It is designed to run in parallel with semTask1.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* SEE ALSO: semTask1
*/

STATUS semTask2 (void)
    {
    STATUS s;
    SEM_ID semSmId;
    int objType;
    int thisTick;

    /* find object in name database */
    
    printf( "semTask2: starting\n");

    s = smNameFind (SEM_NAME, (void **) &semSmId, &objType, WAIT_FOREVER);
    if (s == ERROR)
	{
	printf ("semTask2: can't find %s\n", SEM_NAME);
	return ERROR;
	}

    /* take the shared semaphore */

    printf ("semTask2 is now going to take the shared semaphore\n");
    semTake (semSmId, WAIT_FOREVER);

    /* normally do something useful */

    printf ("semTask2 got the shared semaphore\n");

    printf( "semTask2 killing time\n");
    for (thisTick = 0; thisTick < DELAY_TICKS; thisTick++)
	{
	printf( "taskDelay(tick %d of %d)\n", thisTick, DELAY_TICKS);
	taskDelay (1);
	}

    /* release the shared semaphore */

    semGive (semSmId);

    printf ("semTask2 has released the shared semaphore.\n");

    return OK;
    }

#endif /* INCLUDE_VXMP_TESTS */

#endif /* INCLUDE_SM_COMMON */

/******************************************************************************
 * *
 * * sysBacktraceMips - perform a stack trace of the caller with symbols
 * *               by Curt McDowell - csm@broadcom.com 03-20-01
 * *
 * * NOTE:
 * *
 * * This routine is specific to MIPS and GCC stack frame generation.
 * * Each function generated by the compiler must begin with a decrement
 * * of the stack pointer, then in the next few instructions, must save
 * * the return address on the stack.
 * *
 * *   27bdffd0    addiu       $sp,$sp,-48
 * *               ...
 * *   afbf0028    sw          $ra,40($sp)
 * *               ...
 * */

/* Hack; some day fix to locate function start using symFindByValue */
#define SYSBT_MAX_FUNC_WORDS    4096

LOCAL void sysbt_addr2sym(char *symName, UINT addr)
{
    extern SYMTAB_ID	sysSymTbl;
    UINT		symVal;
    SYM_TYPE		symType;
    int                 rc;

#if VX_VERSION==55
    char                **psymName;
    
    rc = symByValueFind(sysSymTbl,
		       (UINT) addr,
		       psymName, &symVal, &symType);
    if (rc != ERROR)
        strcpy(symName, *psymName);        
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

void sysBacktraceMips(char *pfx, char *sfx, int direct)
{
    UINT                *sp, *pc, *fn_start, *caller_sp, *ra;
    extern UINT         *vxSpGet(), *vxPcGet();         /* sysALib.s */
    int                 limit, first = 1;
    char                buf[2048], symName[MAX_SYS_SYM_LEN+1], *s;
    int                 ra_off, sp_off;

    sp = vxSpGet();
    pc = vxPcGet();

    s = buf;

    if (pfx) {
        strcpy(s, pfx);
    }
    while (*s) s++;

    while (s < &buf[sizeof (buf) - 128]) {
        fn_start = pc;
        for (limit = 0; limit < SYSBT_MAX_FUNC_WORDS; limit++) {
            /* sd $ra,x($sp); x>=0 */
            if ((*--fn_start & 0xffff8000) == 0xffbf0000)
                break;
        }
        if (limit == SYSBT_MAX_FUNC_WORDS)
            break;
        ra_off = (*fn_start & 0x7fff) + 4;
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
        *symName = 0;
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

    if (pfx) {
        strcpy(s, sfx);
    }

    if (direct)
        sysSerialPrintString(buf);      /* Atomically print giant string */
    else {
        fputs(buf, stdout);             /* Regular print giant string */
        fflush(stdout);
    }
}

/*******************************************************************************
 * *
 * * sysToMonitor - transfer control to the ROM monitor
 * *
 * * This routine transfers control to the ROM monitor.  Normally, it is called
 * * only by reboot()--which services ^X--and bus errors at interrupt level.
 * * However, in some circumstances, the user may wish to introduce a
 * * <startType> to enable special boot ROM facilities.
 * *
 * * RETURNS: Does not return.
 * */

int sysToMonitorExcMessage = 0;
int sysToMonitorBacktrace = 1;
int sysToMonitorReboot = 1;
void (*sysToMonitorHook)(void);
#if 0
STATUS sysToMonitor
    (
    int startType    /* parameter passed to ROM to tell it how to boot */
    )
    {
    UINT64 data;
    FUNCPTR pRom = (FUNCPTR) (ROM_TEXT_ADRS + 8);

    data = SBREADCSR(A_SCD_SYSTEM_CFG) & ~M_SYS_SB_SOFTRES;
    data |= M_SYS_SYSTEM_RESET; 
    SBWRITECSR(A_SCD_SYSTEM_CFG, data);

    if (sysToMonitorReboot) {
        (*pRom) (startType);
    }

    return (OK);    /* in case we ever continue from rom monitor */
    }

#else
STATUS sysToMonitor
    (
    int startType    /* parameter passed to ROM to tell it how to boot */
    )
    {
    UINT64 data;

    if (sysToMonitorHook)
        (*sysToMonitorHook)();

    if (sysToMonitorBacktrace)
        sysBacktraceMips("\n--- Stack Trace ---\n", "", 1);

    data = SBREADCSR(A_SCD_SYSTEM_CFG) & ~M_SYS_SB_SOFTRES;
    data |= M_SYS_SYSTEM_RESET; 
    SBWRITECSR(A_SCD_SYSTEM_CFG, data);

    if (sysToMonitorReboot) {
        FUNCPTR pRom = (FUNCPTR) (ROM_TEXT_ADRS + 8);
        intLock();

        sysWbFlush();   /* Flush write buffers */

        (*pRom) (startType);

        return (OK);    /* in case we ever continue from rom monitor */
    }

    return (OK);
    }
#endif

/******************************************************************************
*
* sysEnetAddrSet - set the ethernet address
*
* This routine is used to override the default base MAC address.
*
*/

STATUS sysEnetAddrSet
    (
    unsigned char *pMac /* new mac address */
    )
    {
    bcopy(pMac, sysEnetAddr, 6);
    sysEnetAddrInitialized = TRUE;

    return (OK);
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
    char *dev,          /* name of device, e.g. "bc" */
    int unit,           /* unit number */
    unsigned char *pMac /* where to write the mac address */
    )
    {
    if (sysEnetAddrInitialized)
        {
        bcopy(sysEnetAddr, pMac, 6);
        pMac[5] = pMac[5] + unit;
        return (OK);
        }
    return (ERROR);
    }

/*******************************************************************************
 * *
 * * sysReboot - Reboots the system
 * *
 * *
 * * RETURNS: Does not return.
 * */
void sysReboot(void)
{
        UINT64 data;
    sysSerialPrintString("SYS_HARD_RESET\n");
/*
    SYS_HARD_RESET();
*/

        data = SBREADCSR(A_SCD_SYSTEM_CFG);
        data |= M_SYS_SYSTEM_RESET;
        SBWRITECSR(A_SCD_SYSTEM_CFG, data);
}

/*******************************************************************************
 * *
 * */
int sysTodGetSecond (void)
{
    return 0;
}

STATUS sysTodInit (void)
{
    return (m41t81_tod_init());
}

STATUS sysTodGet (
    int *pYear,
    int *pMonth,
    int *pDay,
    int *pHour,
    int *pMin,
    int *pSec)
{
    return (m41t81_tod_get(pYear, pMonth, pDay, pHour, pMin, pSec));
}

STATUS sysTodSet (
    int year,
    int month,
    int day,
    int hour,
    int min,
    int sec)
{
    return (m41t81_tod_set(year, month, day, hour, min, sec));
}

/***********************************************************
 *
 * TLB related functions
 *
 ***********************************************************/
static void
SetTlb (unsigned long long virt, unsigned long long phy0,
        unsigned long long phy1, int idx)
{
    unsigned long long temp;

    /* cc = 2 (uncacheed); d = 1; v = 1; */
    __asm__ __volatile__ (
         "li      %3, 0xff000000 \n"
         "and     %0, %0, %3 \n"
         "dmtc0   %0, $10 \n"
         "addu    %1, %1, 0x17 \n"
         "dmtc0   %1, $2 \n"
         "addu    %2, %2, 0x17 \n"
         "dmtc0   %2, $3 \n"
         "li      %3, 0xfff \n"
         "dsll    %3, %3, 13 \n"
         "dmtc0   %3, $5 \n"
         "dmtc0   %7, $0 \n"
         "tlbwi \n"
         : "=r" (virt), "=r" (phy0), "=r" (phy1), "=r" (temp)
         : "0" (virt), "1" (phy0), "2" (phy1), "r" (idx)
    );
}

static void
setTlbs(unsigned long long virt, unsigned long long phy,
        unsigned int nEntry, int idx)
{
    int i, shift;
    unsigned long long phy1;

    shift = 6;  /* (PFN bit 0 <=> addr bit 12) - (6 bits of cc, d, v, 0) */
    phy1 = phy + 0x1000000;

    for (i = idx ; i < (idx + nEntry); i++) {
        /* printf("setTlbs i:%d virt:%16llX phy:%16llX "
               "(%16llX) phy1:%16llX (%16llX)\n",
                i, virt, phy, (phy >> shift), phy1, (phy1 >> shift));
         */
        SetTlb(virt, (phy >> shift), (phy1 >> shift), i);
        virt += 0x2000000; /* 16 megbytes */
        phy  += 0x2000000;
        phy1 += 0x2000000;
    }
}

/*
 * size must be multiple of 32MB
 */
static void
mem_map(unsigned int virt, unsigned int phy, unsigned int size)
{
    unsigned long long virtAddr, phyAddr;
    int nEntry, mask;
    unsigned long long valuehi, valuelo0, valuelo1;
    int idx;
    unsigned int pmask = 0x03ffe000;
    static int low = 0, done = 1;

    if (!done) {
        __asm__ __volatile__ (
            "     mtc0  %0, $5  \n"
            : : "r" (pmask)
        );

        done = 1;
    }
    /*
    printf ("mem_map(virt=0x%x, phy=0x%x, size=%d)\n", virt, phy, size);
     */

    virtAddr = (unsigned long long) virt;
    phyAddr  = (unsigned long long) phy;

    nEntry = (size >> 24) / 2; /* suppose size is multiple of 32MB */

    setTlbs(virtAddr, phyAddr, nEntry, low);
#if 0
    for (idx = low ; idx < (low + nEntry); idx++) {
        __asm__ __volatile__ (
            "     mtc0 %4, $0 \n"
            "     tlbr \n"
            "     dmfc0 %0, $10 \n"
            "     dmfc0 %1, $2  \n"
            "     dmfc0 %2, $3  \n"
            "     mfc0  %3, $5  \n"
            : "=r" (valuehi), "=r" (valuelo0), "=r" (valuelo1), "=r" (mask)
            : "r" (idx)
        );

        /* printf("Entry %2d  %016llX %016llx %016llx %08x\n",
                idx, valuehi, valuelo0, valuelo1, mask); */
    }
#endif
    low += nEntry;
}

#if 0
void bcmtlbdump(void)
{
    unsigned long long virtAddr, phyAddr;
    int nEntry, mask;
    unsigned long long valuehi, valuelo0, valuelo1;
    int idx;

    for (idx = 0; idx < MIPSSB1_N_TLB_ENTRIES; idx++) {
        __asm__ __volatile__ (
            "     mtc0 %4, $0 \n"
            "     tlbr \n"
            "     dmfc0 %0, $10 \n"
            "     dmfc0 %1, $2  \n"
            "     dmfc0 %2, $3  \n"
            "     mfc0  %3, $5  \n"
            : "=r" (valuehi), "=r" (valuelo0), "=r" (valuelo1), "=r" (mask)
            : "r" (idx)
        );

        printf("Entry %2d  %016llX %016llx %016llx %08x\n",
                idx, valuehi, valuelo0, valuelo1, mask);
    }
}
#endif

/*
 * MMU mapping for PCI_MEMORY_SPACE
 */
void
sysPciTlbSet(void)
{
    /*
     * Virtual 0x40000000-0x41ffffff to Physical 0x40000000 - 0x41ffffff
     * Note the PCI base address for StrataXGS is 0x41000000, which should
     * be covered by this mapping.
     */
    mem_map(PCI_MEMORY_SPACE, PCI_MEMORY_SPACE, PCI_TLB_PAGE_SIZE);
}

void
sysFlashTlbSet(void)
{
    /*
     * Map virtual 1EC00000 - 20C00000 to phy 1EC00000 - 20C00000
     * for flash device (see flashDrvLib.h or rhone.h)
     */
    mem_map(TLB_FLASH_BASE_ADDRESS, TLB_FLASH_BASE_ADDRESS, TLB_FLASH_MAP_SIZE);
}

/*
 * CFM board related support functions
 */
/*
 * Check if board ID matches CFM Board ID
 */
int
sysIsCFM()
{
    return (0);
}

/*
 * Get CFM slot ID.
 */
int
sysSlotIdGet()
{
    if (sysIsCFM()) {
        return(0);
    } else {
        return(0);
    }
}
