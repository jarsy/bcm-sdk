/* $Id: sysLib.c,v 1.25 2011/07/21 16:14:21 yshtil Exp $
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
 
/* sysLib.c - BCM 4702 system-dependent routines */

/*
modification history
--------------------
01e,24sep02,jmb   send XON to serial port.  
01d,06sep02,jmb   remove single PCI interrupt hack.  Use VxWorks shared PCI
                  interrupt handler.  Scan for secondary PCI bridges.
01c,15aug02,jmb   pciIntConnect for single PCI interrupt
01b,14aug02,jmb   stubbed out sysBusTas for vxMemProbe
01a,30jul02,jmb   created from sysLib.c,v 1.33 for HNBU 4710 eval board.  Merged
                  in code from Strata idtrp334 BSP.
*/

/*
DESCRIPTION
This library provides board-specific routines.


INCLUDE FILES: sysLib.h

SEE ALSO:
.pG "Configuration"
*/

/* includes */

#include "vxWorks.h"
#include "vxLib.h"
#include "config.h"
#include "version.h"
#include "ioLib.h"
#include "tyLib.h"
#include "sysLib.h"
#include "dosFsLib.h"
#include "string.h"
#include "intLib.h"
#include "muxLib.h"
#include "ipProto.h"
#include "ifLib.h"
#include "tickLib.h"
#include "cacheLib.h"
#include "sysSymTbl.h"
#include "symLib.h"
#include "rebootLib.h"
#include "stdio.h"
#include "hnbutypedefs.h"
#include "sbconfig.h"
#include "bcmenet47xx.h"
#include "vxbsp.h"
#include "sb2pci.h"
#include "drv/pci/pciConfigLib.h"
#include "vx_osl.h"
#include "vxbsp.h"
#include "sbextif.h"
#include "m48t59y.h"
#include "ds1743.h"
#include "netinet/in.h"         /* for sysBindFix() */
#include "sockLib.h"            /* for sysBindFix() */
#include "mem/byteNvRam.c"
#include "bcm4704.h"
#include "etc.h"

static bcmenetregs_t *timerregs = NULL;

IMPORT void     sysWbFlush ();
IMPORT ULONG    taskSRInit();
IMPORT int      sysSetCompare ();
IMPORT int      sysGetCompare ();
IMPORT int      sysGetPrid ();
IMPORT int      sysGetCount ();
IMPORT int      sysSetCount ();
IMPORT int      printf (const char *, ...);
IMPORT int      isprint (char);

IMPORT UCHAR    sysinbyte(ULONG address);
IMPORT USHORT   sysinword(ULONG address);
IMPORT ULONG    sysinlong( ULONG address);
IMPORT void     sysoutlong(ULONG address, ULONG datavalue);
IMPORT void     sysoutword(ULONG address, USHORT datavalue);
IMPORT void     sysoutbyte(ULONG address, UCHAR data);

IMPORT void     platform_init();
IMPORT uint32   get_sb_extif_tpsflag();

IMPORT UINT8    ffsMsbTbl[];            /* Msb high interrupt hash table */
IMPORT UINT8    ffsLsbTbl[];            /* Lsb high interrupt hash table */

IMPORT int              consoleFd;              /* klmod: fd of initial console device */
IMPORT int     pciMaxBus;

IMPORT STATUS usrBootLineCrack(char * bootString,BOOT_PARAMS *pParams);

/* globals */
int   sysCpu      = CPU;                /* system CPU type (MIPS_RC32364) */
char *sysMemTopAdr= NULL;               /* top of memory */
char *sysBootLine = BOOT_LINE_ADRS;     /* address of boot line */
char *sysExcMsg   = EXC_MSG_ADRS;       /* catastrophic message area */
int   sysProcNum;                       /* processor number of this CPU */
int   sysFlags;                         /* boot flags */
char  sysBootHost [BOOT_FIELD_LEN];     /* name of host from which we booted */
char  sysBootFile [BOOT_FIELD_LEN];     /* name of file from which we booted */
int   sysVectorIRQ0 =IV_IORQ0_VEC;      /* used by SAL for pci int connect */
int   sysMemTopDebug = 0;
ULONG sysSoftCompare = 0;               /* last target compare reg value */

/*
*  Since tying interrupt lines to the processor is board dependent sysHashOrder
*  is provided to select the prioritization of interrupt lines.  Setting the
*  #define INT_PRIO_MSB to TRUE prioritizes interrupts from 7-0, 7 being
*  highest priority, 0 being lowest.  Setting it to FALSE prioritizes
*  interrupts from 0-7.  See idts134.h for the definition of INT_PRIO_MSB.
*/

#if INT_PRIO_MSB == TRUE
#warning "INT_PRIO_MSB selccted. prioritizes interrupts from 7-0, 7 being  highest priority"
UINT8 * sysHashOrder = ffsMsbTbl;       /* interrupt prio., 7 = high 0 = low */
#else   /* INT_PRIO_MSB == TRUE */
#warning "INT_PRIO_LSB selccted. prioritizes interrupts from 0-7, 0 being  highest priority"
UINT8 * sysHashOrder = ffsLsbTbl;       /* interrupt prio., 0 = high 7 = low */
#endif  /* INT_PRIO_MSB == TRUE */

/* forward declarations */
STATUS sysEnetAddrGet (char *dev, int unit, unsigned char *pMac);

LOCAL void      sysClkInt ();
LOCAL int       sysSw0Ack ();
LOCAL int       sysSw1Ack ();

typedef struct 
    {
    ULONG       intCause;               /* cause of interrupt   */
    ULONG       bsrTableOffset;         /* index to BSR table   */
    ULONG       statusReg;              /* interrupt level      */
    ULONG       pad;                    /* pad for ease of use  */
    } PRIO_TABLE;

/*
*       intPrioTable is a board dependant structure that aids in the
*       processing of the 8 Rc32364 interrupt conditions.
*       It is used by excLib to determine the pending interrupt and
*       to call the user attached handler.  The present search algorithm
*       relies on a one to one mapping of Rc32364 interrupt lines and
*       table entries.
*
*       Each entry has 4 fields, the first (intCause) is the interrupt ID, 
*       second (bsrTableOffset) is the vector number, third is unused
*       last (pad) is the multiplex field.  When an interrupt is received
*       the handler maps the pending Rc32364 line to  its' corresponding
*       table entry (ie. Software interrupt 0 would map to the first entry
*       in the table).  At this point interrupts that are not pending are
*       enabled.  Next the multiplex field is read, if it is zero, field 
*       two is taken as the interrupt vector number in the BSR table, 
*       otherwise it is interpreted as a demultiplex function and called
*       with field 4 passed as it's parameter.  The job of the demultiplex
*       routine is to calculate the correct interrupt vector number and pass
*       it back to the handler.  The handler can then call the routine the
*       user has installed in the BSR table with intVecSet, or intConnect.
*
*       This table is critical to interrupt processing.  Do not alter
*       it's contents until you understand the consequences.  The routine
*       sysPrioUpdate can aid modification during runtime.
*/

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
VOIDFUNCPTR bringupPrintRtn = NULL;

/* select cache library */

IMPORT STATUS cacheBcm47xxLibInit(CACHE_MODE,CACHE_MODE);
FUNCPTR sysCacheLibInit = (FUNCPTR)cacheBcm47xxLibInit; 

/* locals */
LOCAL int   sysClkTicksPerSecond    = 100;      /* default sys timer rate    */
LOCAL int   sysClkArg               = 0;        /* clock int routine arg     */
LOCAL BOOL  sysClkConnected         = FALSE;    /* sys clock connect flag    */
LOCAL BOOL  sysClkRunning           = FALSE;    /* sys clock enabled flag    */
LOCAL int   sysClkProcessorTicks;               /* MIPS32 clock ticks        */
LOCAL FUNCPTR   sysClkRoutine       = NULL ;    /* clock interrupt routine   */
LOCAL char sysModelStr[200];

/* Commenting out sysAuxClkRoutines and defines as its not supported. However
   the auxiliary clock can be generated by using one of the three general 
   purpose timers of Rc32134 */                                           
LOCAL BOOL  sysAuxClkRunning        = FALSE;     /*sys aux clock enabled flag */
LOCAL BOOL  sysAuxClkConnected      = FALSE;     /*sys aux clock connect flag */
LOCAL int   sysAuxClkTicksPerSecond = 200;       /* default aux timer rate    */
LOCAL int   sysAuxClkArg            = 0 ;        /*aux clock int routine arg  */
LOCAL FUNCPTR   sysAuxClkRoutine    = NULL ;     /*aux clock interpt routine  */

LOCAL unsigned char sysEnetAddr[6];
LOCAL int sysEnetAddrInitialized = FALSE;

#ifdef INCLUDE_TIMESTAMP
LOCAL BOOL  sysTimestampRunning     = FALSE;   /* timestamp running flag */
#endif  /* INCLUDE_TIMESTAMP */

#ifdef INCLUDE_NETWORK
#ifdef INCLUDE_END
#if (defined(INCLUDE_ET0_END) || defined(INCLUDE_ET1_END))
#include "sysEtEnd.c"
#endif /* INCLUDE_ET_END */
#endif /* INCLUDE_END */
#endif  /* INCLUDE_NETWORK */

extern UINT32 get_sb_mips_clock();
int sysSlotIdGet();
int sysIsLM();

/*
 * Tell if a buffer contains a valid ansiTime time zone specification
 */
int sysTimeZoneGood(char *tz)
{
    int                 i, colons = 0;

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


/*******************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the string "IDT S134".
*/

char *sysModel (void)
    {
    strcpy (sysModelStr, "BCM4704 (MIPS32)\n");
    strcat (sysModelStr, "Board: JAG");
    return (sysModelStr);
    }


/******************************************************************************
*
* sysBspRev - return the bsp version with the revision eg 1.1/<x>
*
* This function returns a pointer to a bsp version with the revision.
* for eg. 1.1/<x>. BSP_REV is concatanated to BSP_VERSION to form the
* BSP identification string.
*
* RETURNS: A pointer to the BSP version/revision string.
*/

char * sysBspRev (void)
    {
    return (BSP_VERSION BSP_REV);
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
    UINT8               N;
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

/*******************************************************************************
*
* sysHwInit - initialize the CPU board hardware
*
* This routine initializes various features of the IDT S364.
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
#if VX_VERSION == 542
#ifdef PATCH_INT_STUBS
#warning "=====#################################################======"
#warning "=====## Patching Exception/Interrupt Stubs          ##======"
#warning "=====## Will work only on T2.1.1/Vx5.4.2            ##======"
#warning "=====#################################################======"
    volatile UINT32 *patch_loc;
    extern int excStub();

    patch_loc = (volatile UINT32 *)excStub; /* excReturn */
    patch_loc = (volatile UINT32 *)(((UINT32)patch_loc | 0xa0000000) + 0x144 + 0x74);
    *(patch_loc + 1) = 0x24190002; /* li  $t9,2 */
    *(patch_loc + 4) = 0x40996000; /* mtc0    $t9,$12 */
    *(patch_loc + 0) = 0x40806000; /* mtc0    zero,$12 */
    cacheInvalidate(INSTRUCTION_CACHE, (void *)patch_loc, 20);

    patch_loc = (volatile UINT32 *)excStub; /* restoreVolatile */
    patch_loc = (volatile UINT32 *)(((UINT32)patch_loc | 0xa0000000) + 0x52c + 0x4);
    *(patch_loc + 1) = 0x24020002; /* li  $v0,2 */
    *(patch_loc + 4) = 0x40826000; /* mtc0    $v0,$12 */
    *(patch_loc + 0) = 0x40806000; /* mtc0    zero,$12 */
    cacheInvalidate(INSTRUCTION_CACHE, (void *)patch_loc, 20);
#else /* PATCH_INT_STUBS */
#warning "=====#################################################======"
#warning "=====## Patching Exception/Interrupt Stubs Skipped. ##======"
#warning "=====#################################################======"
#endif /* PATCH_INT_STUBS */
#endif /* 542 */

    /* set default task status register for BCM47xx */
    sr = BCM47XX_SR;

    /* init status register but leave interrupts disabled */
    taskSRInit (sr);
    intSRSet (sr & ~SR_IE);

    sysRevId = SYS_REVID_GET();
    switch(sysRevId) {
        case BOARD_ID_LM_P6_CX4:
        case BOARD_ID_JAG_CPCI:
        case BOARD_ID_JAG_CPCI2:
            /* Dallas TOD/NVRAM */
            sysTodFuncs.init = (FUNCPTR) ds1743_tod_init;
            sysTodFuncs.get = (FUNCPTR) ds1743_tod_get;
            sysTodFuncs.set = (FUNCPTR) ds1743_tod_set;
            sysTodFuncs.getSecond = (FUNCPTR) ds1743_tod_get_second;
            sysTodFuncs.watchdogArm = (FUNCPTR) NULL;
            break;
        case BOARD_ID_LM_P48:
        case BOARD_ID_LM_P6_CX4V2:
        case BOARD_ID_LM_FB_P48:
        case BOARD_ID_JAG_BCM56218_EB:
        default:
            break;
    }

    BPRINT ("HWOK");
    }


volatile int serialirqs = 0;
LOCAL void sharedInt0 (void)
{
    FUNCPTR  intVec;

    if (!DEF_SBINTVEC)
    return;

#if 0
    sysSerialInt(0);
    sysSerialInt(1);
#endif
    intVec = intVecGet((FUNCPTR*)INUM_TO_IVEC(IV_EXT_ALT1_VEC));
    if(intVec) (intVec)(0);
    intVec = intVecGet((FUNCPTR*)INUM_TO_IVEC(IV_EXT_ALT2_VEC));
    if(intVec) (intVec)(1);
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
* NOMANUAL
*/

static void dummyISR(int vec) {
    /* do nothing */
    sysLedDsply("DMMY");
    return;
}



void 
sysled(unsigned int val)
{
}

/* Detect HHB4 / DC21150 PCI-PCI bridge chips, and configure bus if
 * found. Note that this code should be run whenever we find a PCI
 * class code of type bridge. After we have run this procedure for the
 * first bridge, we then need to traverse any sub-bridges. This code
 * deals with only one PCI-PCI bridge and it's IO space for
 * sub-devices. See mousse.h for more details as well as the PCI-PCI
 * bridge specification.
 */
int sysPCIBridgeProbe(int instance,
                      int primary,
                      int secondary,
                      int subordinate)
{
    int BusNo, DevNo, FuncNo;
    UINT32 vendor;
    unsigned int buses = 0;
    /* Find DC21150 PCI-PCI bridge or
     * HINTCORP HB4 PCI-PCI Bridge
     */
    if ( (pciFindDevice(DC21150_VENDOR_ID,
                        DC21150_DEVICE_ID,
                        instance,
                        &BusNo, &DevNo, &FuncNo) != ERROR) ||
         (pciFindDevice(PERICOM_VENDOR_ID,
                        PERICOM_8150_DEV_ID,
                        instance,
                        &BusNo, &DevNo, &FuncNo) != ERROR) ||
         (pciFindDevice(HINT_HB4_VENDOR_ID,
                        HINT_HB4_DEVICE_ID,
                        instance,
                        &BusNo, &DevNo, &FuncNo) != ERROR)) {

        pciMaxBus++;

        if (pciConfigInLong(BusNo, DevNo, FuncNo,
                             PCI_CFG_VENDOR_ID, &vendor) != ERROR) {
            if ((((vendor >>  0) & 0x0000FFFF) == HINT_HB4_VENDOR_ID) &&
                (((vendor >> 16) & 0x0000FFFF) == HINT_HB4_DEVICE_ID)) {
                sysPciHostBridgeReadBurstDisable();
            }
        }

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
         * We need to blast all three values with a single write.
         */
        pciConfigOutLong(BusNo, DevNo, FuncNo,
                         PCI_CFG_PRIMARY_BUS, buses);

        /* Clear secondary status */
        pciConfigOutWord(BusNo,DevNo,FuncNo,
                         PCI_CFG_SEC_STATUS, 0xffff);

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

void sysHwInit2 (void)
{
    int sr,i;
    char xonString[3];
    extern STATUS pciIntLibInit ( void );

    sr = intSRGet();
    BPRINT ("PCI ");

#ifdef INCLUDE_PCI
    /* this routine is also responsible for resetting any devices that may
       currently be in an active state and sets up the interrupt mappings */

    pciConfigLibInit(0,0,0,0);

    /*
    * Probe for secondary bridge devices.  The 4702 supports 2 PCI devices
    * connected directly to the host PCI bus.
    * One or both of these devices may be PCI bridges to other buses.
    * Bridges on secondary buses is not supported.
    *
    * Args to the bridge probe function are:
    * (instance, primary busNo, secondary busNo, subordinate busNo)
    * For 4702, the bus numbering is non-standard.  HNBU software has
    * coopted bus number 0 for the silicon backplane, so PCI bus numbering
    * starts at 1.
    *
    */
#endif

    /* Start the Time of Day device */
    BPRINT ("TOD ");
    sysTodInit ((void *) MBZ_NVRAM_ADDR);

    BPRINT ("SER ");
    /* Setup the serial device descriptors. */
    sysSerialHwInit();

#if 1
    /*
    *  Send XON character in case power-off noise from UART has
    *  sent an XOFF
    */
    xonString[0] = (char) 0x11;
    xonString[1] = '\n';
    xonString[2] = '\0';

    sysSerialPrintString(xonString);
#endif
    
    /* Perform anything specific to this platform design */
    platform_init(); 

    if (SYS_REVID_GET() == BOARD_ID_JAG_BCM56218_EB) {
    } else {
        pciMaxBus = 1;

        /* 
         * Look for a secondary PCI bridge.  If we find one, look for
         * another.  This BSP only supports up to 2 secondary bridges
         * on the host bridge.
         */
        if (sysPCIBridgeProbe (0, 1, 2, 2) == OK)
            sysPCIBridgeProbe (1, 1, 3, 3);
        /* Initialize PCI shared interrupt handler */

        pciIntLibInit();
    }

    /*
    **  External UART interrupt init.
    **  BOARD_ID_MBZ_5645_REF uses internal UART.
    */
    
    /* connect dummy isr's to shared int vectors */
    for(i=IV_IORQ0_VEC; i<IV_LAST_VEC; i++)
      intConnect (INUM_TO_IVEC(i), dummyISR, i);

    /* Connect and enable "shared" mips int 0 handler */
    (void) intConnect (INUM_TO_IVEC(IV_IORQ0_VEC), sharedInt0, 0);
    sr |= INT_LVL_IORQ0;   

    /* connect and enable timer int */
    (void) intConnect (INUM_TO_IVEC(IV_RTIME_VEC), sysClkInt, 0);
    sr |= INT_LVL_TIMER;   

    sysSerialHwInit2();
    
    BPRINT("IRQs\n");
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
*/

char *sysPhysMemTop (void)
    {
  
  static char * memTop = NULL;
  UINT  memSize = LOCAL_MEM_SIZE;

    if (memTop == NULL)
	{
        switch(SYS_REVID_GET()) {
            case BOARD_ID_JAG_CPCI2:
            case BOARD_ID_LM_P6_CX4V2:
            case BOARD_ID_LM_FB_P48:
            case BOARD_ID_JAG_BCM56218_EB:
                memSize = LOCAL_MEM_SIZE128;
                break;
            default:
                /* Default 64 MB */
                memSize = LOCAL_MEM_SIZE;
                break;
	}
	memTop = (char *)(LOCAL_MEM_LOCAL_ADRS + memSize);
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
*/
char *sysMemTop (void)
{
    static char * memTop = NULL;

    if (memTop == NULL)
	memTop = sysPhysMemTop () - USER_RESERVED_MEM;

    return memTop;
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
extern UINT32 vxSpGet();
void
sysPrintCPURegs(char *a)
{
    int key;
    key = intLock ();
    sysSerialPrintString(a);
#define  PRINT(x) sysSerialPrintString(#x);sysSerialPrintHex(x(), 1)
    PRINT(sysGetCount);
    PRINT(sysGetCompare);
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

void
sysPrintCPURegsprintf(char *a)
{
    int key;
#define prstr(a) printf("%s", a)
#define prhex(a,b) printf("0x%08x%s", a,(b)?"\n":"")
    sysSerialPrintString(a);
#define  PRINT(x) prstr(#x);prhex(x(), 1)
    PRINT(sysGetCount);
    PRINT(sysGetCompare);
    PRINT(intSRGet);
    PRINT(intCRGet);
    key = intLock ();
    PRINT(ReadEPC);
    PRINT(ReadBVA);
    PRINT(ReadERRPC);
    intUnlock (key);
    prstr("SR = 0x");prhex(key, 1);
    prstr("=========\n");
#undef PRINT
}


/*******************************************************************************
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
#if 0
    if (sysToMonitorHook)
        (*sysToMonitorHook)();
#endif
    if (sysToMonitorBacktrace)
        sysBacktraceMips("\n--- Stack Trace ---\n", "", 1);

    if (sysToMonitorReboot) {
        FUNCPTR pRom = (FUNCPTR) (ROM_TEXT_ADRS + 8);
        intLock();
        /* no addition work necessary for the PRO100B */
        (*pRom) (startType);
    }

    return (OK);    /* in case we ever continue from rom monitor */
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
* SEE ALSO: sysProcNumSet()
*/

int sysProcNumGet (void)
    {
    return (sysProcNum);
    }

/*******************************************************************************
*
* sysProcNumSet - set the processor number
*
* This routine sets the processor number for this board.  Processor numbers
* should be unique on a single backplane.
*
* RETURNS: N/A
*
* SEE ALSO: sysProcNumGet()
*/

void sysProcNumSet
    (
    int procNum
    )
    {
    sysProcNum = procNum;
    }

/*
 * Write characters to AlphaNumeric display.
 * Note that the bytes for the first character is the last address.
 */
void
sysLedDsply(const char* msg)
{
    int k;

    k =  (SYS_REVID_GET() == BOARD_ID_JAG_CPCI2) ?  3 : 0;
    LED_REG(0 << k) = msg[3];
    LED_REG(1 << k) = msg[2];
    LED_REG(2 << k) = msg[1];
    LED_REG(3 << k) = msg[0];
}

/*******************************************************************************
*
* sysBusEid - get the value of the error ID register
*
* This routine returns the contents of the bus error status register
* (if one existed).
*
* NOTE:
* This routine must be provided on all Rc32364 board support packages.
*
* RETURNS: 0, always.
*/

USHORT sysBusEid (void)
    {
    return (0);
    }

/*******************************************************************************
*
* sysBusEar - get the access address of a bus error
*
* This routine returns the address of a bus error.
*
* NOTE:
* This routine must be provided on all Rc32364 board support packages.
* The IDT S134 cannot determine the address that caused
* a bus error.  It is possible to determine the source of a read bus error
* by interpreting the instruction stream that caused the bus error.
*
* RETURNS: -1, always.
*/

ULONG sysBusEar (void)
    {
    return (-1);
    }

/*******************************************************************************
*
* sysAutoAck - acknowledge the RC32364 interrupt condition 
*
* This routine acknowledges an RC32364 interrupt for a specified interrupt
* vector.
*
* NOTE:
* This routine must be provided on all RC32364 board support packages.
* Most interrupts are automatically acknowledged in the interrupt service
* routine.
*
* RETURNS: The result of the interrupt acknowledge cycle.
*/

int sysAutoAck
    (
    int vecNum          /* vector num of interrupt that bugs us */
    )
    {
    int result;
    result = 0;
    switch (vecNum)
	{
	case IV_RTIME_VEC:
	    sysSetCompare (0);
	    break;
	case IV_SWTRAP0_VEC:            /* software trap 0 */
	    return(result = sysSw0Ack ());
	    break;
	case IV_SWTRAP1_VEC:            /* software trap 1 */
	    return(result = sysSw1Ack ());
	    break;
	default:
	    return (-1);
	    break;
	}
    return (result);
    }

/*******************************************************************************
*
* sysSw0Gen - generate software interrupt 0
*
* This routine writes the RC32364 cause register to generate a software
* interrupt.
*
* RETURNS: N/A
*/

void sysSw0Gen (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg |= CAUSE_SW1;
    intCRSet (causeReg);
    }

/*******************************************************************************
*
* sysSw1Gen - generate software interrupt 1
*
* This routine writes the RC32364 cause register to generate a software
* interrupt.
*
* RETURNS: N/A
*/

void sysSw1Gen (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg |= CAUSE_SW2;
    intCRSet (causeReg);
    }

/*******************************************************************************
*
* sysSw0Ack - acknowledge software interrupt 0
*
* This routine writes the RC32364 cause register to acknowledge a software
* interrupt.
*
* NOTE:
* This routine is provided as a default interrupt service routine.
*
* RETURNS: N/A
*/

LOCAL int sysSw0Ack (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg &= ~CAUSE_SW1;
    intCRSet (causeReg);

    return(OK);
    }

/*******************************************************************************
*
* sysSw1Ack - acknowledge software interrupt 1 
*
* This routine writes the RC32364 cause register to acknowledge a software
* interrupt.
*
* NOTE:
* This routine is provided as a default interrupt service routine.
*
* RETURNS: N/A
*/

LOCAL int sysSw1Ack (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg &= ~CAUSE_SW2;
    intCRSet (causeReg);

    return(OK);
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
    
    sysClkProcessorTicks = (get_sb_mips_clock()/sysClkTicksPerSecond)/2;

    key = intLock ();
    sysSetCompare (0);
    sysSetCount (1);
    sysSetCompare (sysClkProcessorTicks);
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
    if (ticksPerSecond < SYS_CLK_RATE_MIN ||
	ticksPerSecond > SYS_CLK_RATE_MAX)
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
LOCAL void sysClkInt (void)
    {
    int key;
    ULONG delta;        /* time to when next intrerrupt should be */


    /* see sysClkDisable(). Clear interrupt and return. */
    if (!sysClkRunning)
	{
	sysSetCompare (sysSoftCompare);
	return;
	}
    key = intLock();
    /* use unsigned arithmetic to compensate for wrap-around */

    /* delta is how long ago the interrupt occured. */
    delta = sysGetCount() - sysSoftCompare;
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
    	sysSetCompare (sysSoftCompare);
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
    	sysSoftCompare = sysGetCount() + sysClkProcessorTicks;
    	sysSetCompare (sysSoftCompare);
	}
    intUnlock (key);

    if (sysClkRoutine != NULL)
	(*sysClkRoutine) (sysClkArg);

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
    sysSetCompare (0);
    sysSetCount (1);
    sysClkProcessorTicks = 0xffffff00;
    sysSetCompare (sysClkProcessorTicks);
    sysSoftCompare = sysClkProcessorTicks;
    sysClkRunning = FALSE;
    intUnlock (key);
    }


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

	intstatus = R_REG(&timerregs->intstatus) & I_TO;

    if (intstatus) {
        /* clear the timer interrupt */
    	W_REG(&timerregs->intstatus, intstatus);
        
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
    (void) intConnect (INUM_TO_IVEC(INT_VEC_IORQ3), sysAuxClkInt, 0);
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
        /* write a zero to disable the timer */
      W_REG( &timerregs->gptimer, 0);

      /* disable interrupts */
      W_REG( &timerregs->intmask, 0);
      
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
    uint32 clock,core;
    sbconfig_t *sbregs;

    if (timerregs == NULL) {
        printf("No core was selected to provide a timer\n");
        printf("Use set_auxtimer(base_addr) to select a core.\n");
        sysAuxClkRunning = FALSE ;
        return;
    }

    sbregs = (sbconfig_t*)((uint32)timerregs + SBCONFIGOFF);
    core = (sbregs->sbidhigh & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT;

    /* check if the core selected is the iLine core and set clock appropriately */
    if (core == SB_ILINE20) 
        clock = 64000000;
    else
        clock = get_sb_clock(); /* 100MHz bacplane */

    /* write to the timer */
    W_REG( &timerregs->gptimer, clock/sysAuxClkTicksPerSecond);
    /* enable timer interrupt */
    W_REG( &timerregs->intmask, I_TO);

    sysAuxClkRunning = TRUE ;
    }

void set_auxtimer( uint32 base_addr )
{
    timerregs = (bcmenetregs_t *)base_addr;
}

void show_auxtimer( void ) {
    printf("Aux timer core base address is 0x%x\n", (uint32)timerregs); 
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
     if( ticksPerSecond < AUX_CLK_RATE_MIN ||
	 ticksPerSecond > AUX_CLK_RATE_MAX)
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

/*
 * Reboot system.
 */

void sysReboot(void)
{
    SYS_HARD_RESET();

#if 0
    /* XXX: Watchdog */
    *(volatile UINT32 *)(0xb8000080) = 1;
#endif
}


#ifdef INCLUDE_TIMESTAMP
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

    return (get_sb_clock()/sysClkTicksPerSecond)/2;
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
    return (get_sb_clock()/2);
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
    return (sysGetCount() - (sysSoftCompare - sysClkProcessorTicks) + 1);
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
    return(sysGetCount() - (sysSoftCompare - sysClkProcessorTicks)+ 1);
    }

#endif  /* INCLUDE_TIMESTAMP */

/*******************************************************************************
*
* sysMaskVmeErr - mask the VMEbus error interrupt
*
* This routine is required for all RC32364 BSPs.  It has no effect on the
* S134.
*
* RETURNS: 0.
*
* NOMANUAL
*/

UINT8 sysMaskVmeErr (void)
    {
    return (0);
    }

/*******************************************************************************
*
* sysUnmaskVmeErr - unmask the VMEbus error interrupt
*
* This routine is required for all RC32364 BSPs.  It has no effect on the
* S134.
*
* RETURNS: 0.
*
* NOMANUAL
*/

UINT8 sysUnmaskVmeErr (void)
    {
    return (0);
    }

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
    unsigned char *pMac         /* where to write the mac address */
    )
    {
    int i;
    int allzeros;
    int allfs;
    
    if (dev == NULL)
        return (ERROR);
    
    if (strcmp ("phy", dev) == 0)
        {
        if (unit != 0 && unit != 1)
            return (ERROR);
        if (unit == 0) 
           * (int *) pMac = (sysIsLM() ? ET1_PHYADDR:
               (SYS_REVID_GET() == BOARD_ID_JAG_BCM56218_EB) ? 0 : ET0_PHYADDR);
        else
           * (int *) pMac = (sysIsLM() ? ET0_PHYADDR:
                    (((SYS_REVID_GET() == BOARD_ID_JAG_BCM56218_EB)) ? 
                    PHY_NOMDC : ET1_PHYADDR));
        return (OK);
        }

    if (strcmp ("mdc", dev) == 0)
        {
        if (unit != 0 && unit != 1)
            return (ERROR);
           * (int *) pMac = (unit == 0) ? ET0_MDCPORT : ET1_MDCPORT;
        return (OK);
        }

    pMac[0] = 0x02;
    pMac[1] = 0;
    pMac[2] = 0;
    pMac[3] = 0;
    pMac[4] = sysSlotIdGet();
    pMac[5] = unit;

    /* LM hack. Need flash NVRAM support */
    if ((SYS_REVID_GET() == BOARD_ID_LM_P48) ||
        (SYS_REVID_GET() == BOARD_ID_LM_FB_P48) ||
        (SYS_REVID_GET() == BOARD_ID_JAG_BCM56218_EB) ||
        (SYS_REVID_GET() == BOARD_ID_LM_P6_CX4V2)) {
        return OK;
    }

    if (sysEnetAddrInitialized)
        {
        bcopy(sysEnetAddr, pMac, 6);
        pMac[5] = pMac[5] + unit;
        return (OK);
        }

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
        else if (pMac[i] == (unsigned char) 0xff)
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
*  flashFsSync() is normally defined in flashFsLib, but MBZ uses
*  the DOC, not a flash array.
*/
STATUS flashFsSync(void)
    {
    return (OK);
    }

#endif /* INCLUDE_FLASH */

int sysPCIWlProbe(int instance,
                      int primary,
                      int secondary,
                      int subordinate)
{
    int BusNo, DevNo, FuncNo;

    sysSerialPrintString("sysPCIWlProbe");

    if ( (pciFindDevice(0x14e4,
                        0x4320,
                        instance,
                        &BusNo, &DevNo, &FuncNo) != ERROR)) {


#define PCI_WL_BAR0 0xa8200000

        /* Setup memory address space mapping */

        pciConfigOutLong(BusNo,DevNo,FuncNo,
                         PCI_CFG_BASE_ADDRESS_0,
                         PCI_WL_BAR0);

        pciConfigOutByte(BusNo,DevNo,FuncNo,
                         PCI_CFG_DEV_INT_LINE,
                         IV_IORQ1_VEC);


	pciConfigOutWord(BusNo,DevNo,FuncNo, PCI_CFG_COMMAND, 0x0117);

    	sysSerialPrintString("wl probe:OK");
        return OK;
    }
    	sysSerialPrintString("wl probe:ERROR");
    return ERROR;
}
/*
*  Stub required because we're using vxMemProbe
*/
BOOL sysBusTas(char * address)
    {
    return(TRUE);
    }

UINT8 sysBoardRev()
{
    return 0;
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



/* #define DEBUG 1*/
#ifdef DEBUG
#include "bcm4704.h"

/*
 *
 * Serial debugging print routines
 *
 * The following routines are for debugging, especially useful in early
 * boot ROM and ISR code.
 */
unsigned char pio_uart_inb(int addr);
void pio_uart_outb(int addr, unsigned char c);
void pio_uart_delay(void);
void pio_uart_putc(int c);
int pio_uart_getc(void);
void pio_uart_puts(char *s);
void pio_uart_puthex(unsigned int value, int cr);



/* Generic 4704/5365 parameters */
#ifdef _BCM95365K_
#define UART_BASE  INTERNAL_UART_COM1
#else
#endif
#define UART_BASE  INTERNAL_UART_COM2 /* COM1/2 swapped on K & P board */


#define UART_CLOCK 1850000
#define BASE_BAUD  9600

#define DIV_LO     ((UART_CLOCK/16)/BASE_BAUD)
#define DIV_HI     (DIV_LO>>8)	

/* 4704/5365 Internal UART register definitions */

#define UART_THR        0x00	/* Transmitter holding reg. */
#define UART_RDR        0x00	/* Receiver data reg.       */
#define UART_BRDL       0x00	/* Baud rate divisor (LSB)  */
#define UART_BRDH       0x01	/* Baud rate divisor (MSB)  */
#define UART_IER        0x01	/* Interrupt enable reg.    */
#define UART_IID        0x02	/* Interrupt ID reg.        */
#define UART_LCR        0x03	/* Line control reg.        */
#define UART_MDC        0x04	/* Modem control reg.       */
#define UART_LST        0x05	/* Line status reg.         */
#define UART_MSR        0x06	/* Modem status reg.        */

#ifdef  MIPSEL
#define UART_REG(reg) 		(UART_BASE + (reg))
#else
#define UART_REG(reg) 		(UART_BASE + (reg^3))
#endif

unsigned char
pio_uart_inb(int addr)
{
    return *(volatile unsigned char *) addr;
}

void
pio_uart_outb(int addr, unsigned char c)
{
    *(volatile unsigned char *) addr = c;
}

void
pio_uart_delay(void)
{
    volatile int i;
    for (i = 0; i < 0x10000; i++)
	;
}

void
pio_uart_putc(int c)
{
    int i = 10000;
    while (!(pio_uart_inb(UART_REG(UART_LST)) & 0x40) && i--)
        ;
    pio_uart_outb(UART_REG(UART_THR), c);
}

int
pio_uart_getc(void)
{
    while (!(pio_uart_inb(UART_REG(UART_LST)) & 0x01))
        ;
    return pio_uart_inb(UART_REG(UART_RDR));
}

void
pio_uart_puts(char *s)
{
    int c;

    pio_uart_outb(UART_REG(UART_LCR),  0x83); /* 0x80 */
    pio_uart_outb(UART_REG(UART_BRDL), (unsigned char)DIV_LO);
    pio_uart_outb(UART_REG(UART_BRDH), (unsigned char)DIV_HI);
    pio_uart_outb(UART_REG(UART_LCR),  0x03);
    pio_uart_outb(UART_REG(UART_MDC), 0xb);

    while ((c = *s++) != 0) {
        if (c == '\n')
            pio_uart_putc('\r');
        pio_uart_putc(c);
    }
    pio_uart_delay();	/* Allow last char to flush */
}

void
pio_uart_puthex(unsigned int value, int cr)
{
    int i;
    const char hex[] = "0123456789abcdef";

    pio_uart_outb(UART_REG(UART_LCR),  0x80); /* 0x83 */
    pio_uart_outb(UART_REG(UART_BRDL), (unsigned char)DIV_LO);
    pio_uart_outb(UART_REG(UART_BRDH), (unsigned char)DIV_HI);
    pio_uart_outb(UART_REG(UART_LCR),  0x03);
    /*     pio_uart_outb(UART_REG(UART_MDC), 0xb); */

    for (i = 0; i < 8; i++)
        pio_uart_putc(hex[value >> (28 - i * 4) & 0xf]);
    if (cr) {
	pio_uart_putc('\r');
	pio_uart_putc('\n');
    }

    pio_uart_delay();	/* Allow last char to flush */

}
#endif

void sys47xxClocks(int *core, int *sb, int *pci)
{
        *sb = 100;
        *pci = 33;
        *core = get_sb_mips_clock();
}

int
sysSlotIdGet()
{
    switch(SYS_REVID_GET()) {
        case BOARD_ID_LM_P6_CX4:
            return (SYS_SLOTID_GET() + 2);
        case BOARD_ID_LM_P48:
        case BOARD_ID_LM_P6_CX4V2:
        case BOARD_ID_LM_FB_P48:
            return (SYS_SLOTID_GET() + 3);
        case BOARD_ID_JAG_BCM56218_EB:
            return 0;
        default:
            return(2);
    }
}

int
sysIsLM()
{
    unsigned int id;

    id = SYS_REVID_GET();

    return(id == BOARD_ID_LM_P6_CX4) ||
    (id == BOARD_ID_LM_P6_CX4V2) ||
    (id == BOARD_ID_LM_FB_P48) ||
    (id == BOARD_ID_LM_P48);
}
