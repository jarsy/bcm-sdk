/* $Id: sysLib.c,v 1.9 2011/07/21 16:14:58 yshtil Exp $
   Copyright 2001, Broadcom Corporation
   All Rights Reserved.

   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
   the contents of this file may not be disclosed to third parties, copied or
   duplicated in any form, in whole or in part, without the prior written
   permission of Broadcom Corporation.
 */

/* sysLib.c - BCM 56218 system-dependent routines */

/*
   modification history
   --------------------
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
#include "typedefs.h"
#include "vxbsp.h"
#include "drv/pci/pciConfigLib.h"
#include "vx_osl.h"
#include "vxbsp.h"
#include "netinet/in.h"         /* for sysBindFix() */
#include "sockLib.h"            /* for sysBindFix() */

extern void sysLedSet(unsigned char val);
extern int sysIsFlashProm(void);

#ifdef QUICK_TURN
STATUS sysNvRamGet
    (
    char *string,    /* where to copy non-volatile RAM    */
    int strLen,      /* maximum number of bytes to copy   */
    int offset       /* byte offset into non-volatile RAM */
    )
    {
        return (ERROR);
    }

STATUS sysNvRamSet
    (
    char *string,     /* string to be copied into non-volatile RAM */
    int strLen,       /* maximum number of bytes to copy           */
    int offset        /* byte offset into non-volatile RAM         */
    )
    {
        return (OK);
    }

#else
#include "flashFsLib.h"
#include "flashDrvLib.h"

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

        if (sysIsFlashProm())
            return (ERROR);

	if (offset >= FLASH_NVRAM_SIZE)
	    return (ERROR);

        /* now allocate buffers and read sectors */
        buf = malloc(FLASH_NVRAM_SIZE);
        if (buf == NULL)
            return (ERROR);

	if (flashBlkRead(FLASH_NVRAM_START_SECTOR, buf,
                         FLASH_NVRAM_SECTOR_OFFSET,
			 FLASH_NVRAM_SIZE) == ERROR) {
            free(buf);
	    return(ERROR);
	}

	memcpy(string, buf + offset, strLen);
	
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

        if (sysIsFlashProm())
            return (ERROR);

	if (offset >= FLASH_NVRAM_SIZE)
	    return (ERROR);

        /* now allocate buffers and read sectors */
        buf = malloc(FLASH_NVRAM_SIZE);
        if (buf == NULL)
            return (ERROR);

	if (flashBlkRead(FLASH_NVRAM_START_SECTOR, buf,
			 FLASH_NVRAM_SECTOR_OFFSET,
			 FLASH_NVRAM_SIZE) == ERROR) {
            free(buf);
	    return(ERROR);
	}

	memcpy(buf + offset, string, strLen);

	if (flashBlkWrite(FLASH_NVRAM_START_SECTOR, buf,
			 FLASH_NVRAM_SECTOR_OFFSET,
			 FLASH_NVRAM_SIZE) == ERROR) {
            free(buf);
	    return(ERROR);
	}
	flashSyncFilesystem();
	
        free(buf);
        return (OK);
    }

#endif /* QUICK_TURN */

#include "bcm56218.h"

IMPORT void     sysWbFlush ();
IMPORT ULONG    taskSRInit();
IMPORT int      sysSetCompare ();
IMPORT int      sysGetCompare ();
IMPORT int      sysGetPrid ();
IMPORT int      sysGetCount ();
IMPORT int      sysSetCount ();
IMPORT int      printf (const char *, ...);
IMPORT int      isprint (char);

IMPORT void     platform_init();

IMPORT UINT8    ffsMsbTbl[];            /* Msb high interrupt hash table */
IMPORT UINT8    ffsLsbTbl[];            /* Lsb high interrupt hash table */

IMPORT int              consoleFd;              /* klmod: fd of initial console device */

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
int   sysVectorIRQ0 = IV_IORQ3_VEC;      /* used by SAL for pci int connect */
int   sysMemTopDebug = 0;
ULONG sysSoftCompare = 0;               /* last target compare reg value */
int sysSoftCount31_0 = 0;                  /* last target count reg value */
ULONG sysSoftCount63_32 = 0;            /* last target count reg value */

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
LOCAL int   sysAuxClkTicksPerSecond = 0;       /* default aux timer rate    */
LOCAL int   sysAuxClkArg            = 0 ;        /*aux clock int routine arg  */
LOCAL FUNCPTR   sysAuxClkRoutine    = NULL ;     /*aux clock interpt routine  */
LOCAL int   auxclk_factor = 0;
LOCAL int   auxclk_downctr;
LOCAL void sysAuxClkInt (void);
LOCAL unsigned char sysEnetAddr[6];
LOCAL int sysEnetAddrInitialized = FALSE;

#ifdef INCLUDE_TIMESTAMP
LOCAL BOOL  sysTimestampRunning     = FALSE;   /* timestamp running flag */
#endif  /* INCLUDE_TIMESTAMP */

extern UINT32 get_sb_mips_clock();

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
    strcpy (sysModelStr, "BCM56218 (MIPS32)\n");
    strcat (sysModelStr, "Board: RAPTOR");
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

#define BCM56218_GPIO_INT_PINS      0x3
    /* Init GPIO */
    *((volatile unsigned int*)KSEG1ADDR(BCM56218_REG_GPIO + 0xc)) = 0xFFFFFFFF;
    /* Enable GPIO[1:0] for interrupts (Int. sense is Active low */
    *((volatile unsigned int*)KSEG1ADDR(BCM56218_REG_GPIO + 0x14)) = 
                                                    BCM56218_GPIO_INT_PINS;
    *((volatile unsigned int*)KSEG1ADDR(BCM56218_REG_GPIO + 0x10)) = 
                                                    BCM56218_GPIO_INT_PINS;
    *((volatile unsigned int*)KSEG1ADDR(BCM56218_REG_GPIO + 0x8)) = 
                                                    BCM56218_GPIO_INT_PINS;

    /* reset uart */
    *((volatile unsigned int*)KSEG1ADDR(0x18000114)) = 1;
    *((volatile unsigned int*)KSEG1ADDR(0x18000114)) = 3;
    /* set default task status register for BCM47xx */
    sr = BCM56218_SR;

    /* init status register but leave interrupts disabled */
    taskSRInit (sr);
    intSRSet (sr & ~SR_IE);

    BPRINT ("HWOK");
}


volatile int serialirqs = 0;
LOCAL void sharedInt0 (void)
{
    FUNCPTR  intVec;
    int      idx;

    sysLedDsply("SHIi");
    sysLedSet(0x8);
   
    if (!DEF_SBINTVEC)
        return;

    for (idx = IV_IORQ0_BIT0_VEC; idx <= IV_LAST_VEC; idx++) {
        intVec = intVecGet((FUNCPTR*)INUM_TO_IVEC(idx));
        if(intVec) (intVec)(idx);
    }
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
    volatile uint8 *ua = (volatile uint8 *)0xb8000402;
    static int war = 1, dummy;

    sysLedDsply("DMMY");

    if (war) {
        dummy = *ua;
        ua = (volatile uint8 *)0xb8000302;
        dummy = *ua;
        war = 0;
    }
    sysLedSet(0x4);
    return;
}

void sysHwInit2 (void)
{
    int sr,i;
    char xonString[3];

    sr = intSRGet();

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

    /* connect dummy isr's to shared int vectors */
    for(i=IV_IORQ0_VEC; i <= IV_LAST_VEC; i++)
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


static UINT _bcm56218_find_mem_size(void)
{
    UINT  pattern = 0xdeaddada, bak0;
    UINT  size = 0x1000000; /* Min 16 MB */
    int ii;
    
    bak0 = *((UINT*)PHYS_TO_K1(LOCAL_MEM_LOCAL_ADRS + 0));

    for (ii = 0; ii < 4; ii++) {
        *((UINT*)PHYS_TO_K1(LOCAL_MEM_LOCAL_ADRS + 0)) = pattern;
        while (*((UINT*)PHYS_TO_K1(LOCAL_MEM_LOCAL_ADRS + size)) != pattern) {
            /* Check if the matched failed for 64MB, then memory can only be 128MB,
             * as ICS address space doesnt allow for more than 128 MB */
            if (size == 0x4000000) {
                size = 0x8000000;
                goto done;
            }
            size *= 2;
            ii = 0;
        }
        pattern++;
    }

done:
    *((UINT*)PHYS_TO_K1(LOCAL_MEM_LOCAL_ADRS + 0)) = bak0;
    return size;
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
    static UINT  memSize = LOCAL_MEM_SIZE;

    if (memTop == NULL)
    {
        memSize = _bcm56218_find_mem_size();
    }
    memTop = (char *)(LOCAL_MEM_LOCAL_ADRS + memSize);
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
extern UINT32 *vxSpGet();

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
/* #warning "Add system specific LED display code !!" */
}

void
sysLedSet(unsigned char val)
{
    *((volatile unsigned char *)KSEG1ADDR(LED_DIGIT(0x18000064))) = 
        ((~val) & 0xff);
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

    sysLedDsply("TMii");
   
    /* see sysClkDisable(). Clear interrupt and return. */
    if (!sysClkRunning)
    {
        sysSetCompare (sysSoftCompare);
        return;
    }
    key = intLock();
    if (sysSoftCount31_0 < 0) {
        sysSoftCount31_0 = sysGetCount();
        if (sysSoftCount31_0 > 0) {
            sysSoftCount63_32++;
        }
    } else {
        sysSoftCount31_0 = sysGetCount();
    }

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

    /* Call Aux clock routine */
    sysAuxClkInt();

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

uint32 auxClkIntCtr = 0;
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
    uint32  factor = 1;
    auxClkIntCtr++;
    sysLedDsply("T0Ii");
    sysLedSet(0x2);

    if ((auxclk_factor) && (sysAuxClkRoutine != NULL)) {
        if (--auxclk_downctr == 0) {
            (*sysAuxClkRoutine) (sysAuxClkArg);     
            auxclk_downctr = auxclk_factor;
        }
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
#if 0
    /* write a zero to disable the timer */
    W_REG(ICS_TIMER0_REG, 0);

    /* disable interrupts */
    W_REG(ICS_TIMER0_INT_MASK, 0);
#endif
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
    if (sysAuxClkTicksPerSecond) {
        auxclk_factor = sysAuxClkTicksPerSecond/sysClkTicksPerSecond;
        auxclk_downctr = auxclk_factor;
        sysAuxClkRunning = TRUE;
    }
}

void show_auxtimer( void ) {
    printf("Aux timer core base address is 0x%x\n", 
           (uint32)ICS_TIMER0_INT_STATUS); 
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
}

void sysWdReboot(void)
{
    *(volatile uint32 *) ICS_WATCHDOG_CTR_REG = 1;
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

    if (sysEnetAddrInitialized)
    {
        bcopy(sysEnetAddr, pMac, 6);
        pMac[5] = pMac[5] + unit;
        return (OK);
    }

    if (sysNvRamGet(pMac, 6, NV_OFF_MACADDR) == ERROR) {
        /* Put default MAC address. */
        pMac[0] = 0x02;
        pMac[1] = 0;
        pMac[2] = 0;
        pMac[3] = 0;
        pMac[4] = 0;
        pMac[5] = unit;

        return (ERROR);
    }

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

    printf("Error: Invalid MAC address. Please program valid mac address!!\n");
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
#include "bcm56218.h"

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



#ifdef QUICK_TURN
#define UART_BASE  INTERNAL_UART_COM1
#else
#define UART_BASE  INTERNAL_UART_COM2 /* COM1/2 swapped on K & P board */
#endif

#define UART_CLOCK UART_REF_CLK_FREQ
#define BASE_BAUD  CONSOLE_BAUD_RATE
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

#define UART_REG(reg) 		(UART_BASE + (reg))

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
    *sb = get_sb_clock();
    *pci = 0;
    *core = get_sb_mips_clock();
}

int
sysSlotIdGet()
{
    return 0;
}

int
sysIsLM()
{
    return 0;
}

int sysIsFlashProm()
{
    unsigned int *cap = (unsigned int *) 0xb8000004;

    if ((*cap) & 0x00000100) {
        return 1;
    }

    return 0;
}

unsigned int sysGetSocRevId()
{
    return ((*(volatile uint32 *) ICS_CHIP_ID_REG) >> 16) & 0xFF;
}

unsigned int sysGetSocDevId()
{
    return ((*(volatile uint32 *) ICS_CHIP_ID_REG) & 0xFFFF);
}

void sys_timestamp_get(uint32 *up, uint32 *low, uint32 *freq)
{
    int key, Count31_0;
    key = intLock();

    *up = sysSoftCount63_32;
    *low = Count31_0 = sysGetCount();
    if (sysSoftCount31_0 < 0) {
        if (Count31_0 > 0) {
            *up += 1;
        }
    }
    *freq = get_sb_clock();
    intUnlock (key);
}
