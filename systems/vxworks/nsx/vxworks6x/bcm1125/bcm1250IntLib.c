/* bcm1250IntLib.c - BCM1250 Shared Interrupt support */

/* Copyright 2004 Wind River Systems, Inc. */

/* $Id: bcm1250IntLib.c,v 1.3 2011/07/21 16:14:48 yshtil Exp $
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
***********************************************************************
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
01f,15nov04,mdo  Documentation fixes for apigen
01e,23jun04,agf  function which returns core num moved from BSP to arch
01d,19jul02,pgh  Remove INT5 from interrupt mapper.
01c,20jun02,pgh  Change path to bcm1250Lib.h.
01b,06dec01,agf  improve comments for 'IntDisable'
01a,15nov01,agf  written.
*/


/*
DESCRIPTION
An interrupt source is one of the 64 (0..63) interrupt sources
recognized by the BCM1250 interrupt mapper.  Each source is the union
of the corresponding system source and the same-numbered LDT
interrupt.  LDT interrupts are assumed to be configured for
edge-triggering.

An interrupt line (aka vector) is one of the six external interrupts
(0..5) specified by the MIPS architecture.

Only a single handler is supported for each source, but the mapping
from sources to lines is arbitrary and controlled by the interrupt
mapper.  The vector-level handlers are provided here and demultiplex
to the corresponding source(s).

Shared BCM1250 interrupts are supported by five functions:
  bcm1250IntConnect(), bcm1250IntDisconnect(),
  bcm1250IntEnable(), bcm1250IntDisable(),
  bcm1250Int().

bcm1250IntConnect() adds the specified interrupt handler for a source
to the list for the line, and bcm1250IntDisconnect() removes it from
that list.

bcm1250IntEnable() enables the specified interrupt and
bcm1250IntDisable() disables it.  Initially, the interrupt for a
connected handler is disabled.

Master interrupt handler bcm1250Int() executes those interrupt handlers
associated with a given interrupt line.  bcm1250Int() should be
attached by the intConnect() function in the BSP initialization with
its parameter.   The parameter is an IRQ associated to the MIPS
vector number.

INCLUDE FILES
*/

#include "vxWorks.h"
#include "config.h"
#include "dllLib.h"
#include "sysLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "intLib.h"
#include "bcm1250IntLib.h"
#include "bcm1250Lib.h"

/* externals */

IMPORT int      palCoreNumGet ();

/* typedefs */

typedef struct bcm1250IntHandlerData
  {
  int           intrLine;       /* SB-1 hardware interrupt line (0-5) */
  int           intrSource;     /* bcm1250 source number (0-63) */
  VOIDFUNCPTR	routine;	/* interrupt handler */
  int		parameter;	/* parameter of the handler */
  } BCM1250_INT_HANDLER_DATA;

#define	BCM1250_NUM_INT_LINES	(IV_INT5_VEC - IV_INT0_VEC + 1)
#define	BCM1250_NUM_INT_SOURCES	R_IMR_INTERRUPT_MAP_COUNT

/* locals */

LOCAL STATUS bcm1250IntLibInitStatus = NONE;

LOCAL BCM1250_INT_HANDLER_DATA bcm1250IntHandlers[BCM1250_NUM_INT_SOURCES];

LOCAL volatile UINT64 *bcm1250IntMask;
LOCAL volatile UINT64 *bcm1250IntStatusBase;
LOCAL volatile UINT64 *bcm1250IntMapBase;
LOCAL volatile UINT64 *bcm1250LdtIntClear;

/* local forward declarations */

LOCAL void bcm1250Int(int intr_line);

/*******************************************************************************
*
* bcm1250IntLibInit - bcm1250 custom interrupt handler initialization
*
* This routine is called by sysHwInit2 prior to actually enabling the interrupt
* lines on an SB-1. This routine initializes the BCM1250 interrupt mapper of the
* 64 sources, initializes its own support data structures, and for vxWorks 
* connects the custom bcm1250 handler to service all the SB-1 interrupt lines.
*
* RETURNS:
* OK, or ERROR if the bcm1250 interrupt handler cannot be connected to all
* the interrupt lines.
*
* ERRNO
*/

STATUS bcm1250IntLibInit ( void )
    {
    int ix;
    int cpunum;

    if (bcm1250IntLibInitStatus != NONE)
	return (bcm1250IntLibInitStatus);

    /* Set CPU number, so that we talk to the correct interrupt controller. */
    cpunum = palCoreNumGet ();

    bcm1250IntMask =
      (void *)PHYS_TO_K1(A_IMR_REGISTER(cpunum, R_IMR_INTERRUPT_MASK));
    bcm1250IntStatusBase =
      (void *)PHYS_TO_K1(A_IMR_REGISTER(cpunum, R_IMR_INTERRUPT_STATUS_BASE));
    bcm1250LdtIntClear =
      (void *)PHYS_TO_K1(A_IMR_REGISTER(cpunum, R_IMR_LDT_INTERRUPT_CLR));
    bcm1250IntMapBase =
      (void *)PHYS_TO_K1(A_IMR_REGISTER(cpunum, R_IMR_INTERRUPT_MAP_BASE));

    /* Mask off all interrupts (they'll be enabled as needed later). */
    MIPS3_SD(bcm1250IntMask, 0xffffffffffffffffULL);
   
    /* Initialize each interrupt handler structure. */
    for (ix = 0; ix < BCM1250_NUM_INT_SOURCES; ix++)
	{
	bcm1250IntHandlers[ix].intrSource = -1;
	bcm1250IntHandlers[ix].routine = NULL;
	}

    /* Connect up all of the interrupts */
    for (ix = 0; ix < BCM1250_NUM_INT_LINES; ix++)
	if (intConnect (INUM_TO_IVEC(ix + IV_INT0_VEC), bcm1250Int,
			ix) != OK)
            return(bcm1250IntLibInitStatus = ERROR);

    return (bcm1250IntLibInitStatus = OK);
    }

/*******************************************************************************
*
* bcm1250Int - interrupt handler for shared BCM1250 interrupt.
*
* This routine executes multiple interrupt handlers for a BCM1250 CPU
* interrupt.  Prior to execution of the interrupt handler this routine
* checks the BCM1250 interrupt_status_n register to determine if source
* of the interrupt matches the interrupt handler in the link list.
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL VOID bcm1250Int
    (
    int intr_line		/* INT line that caused the interrupt */
    )
    {
    BCM1250_INT_HANDLER_DATA *pHand;
    UINT64 intStatus;
    UINT64 intBit;
    unsigned int intSource;

    /* If INT 5, make sure that count/compare interrupt is cleared. */
    if (intr_line == 5)
        {
#ifdef BCM1250_SB1_PROFILING
	IMPORT int sbprof_cpu_intr(void);
	if (sbprof_cpu_intr()) return;
#endif
	sysCompareSet(0);
	}

    intStatus = MIPS3_LD(&bcm1250IntStatusBase[intr_line]);

    /* Clear any of the interrupts that came from LDT */
    MIPS3_SD(bcm1250LdtIntClear, intStatus);
#if 0
    __asm__ ("beqz %0, .int_loop_end" : : "r"(intStatus));	
    __asm__ (".int_loop_start:  .word (0x1c << 26) | (0x1f & 16) << 21 | \
              (0x1f & 17) << 11 | 0x24");
    /*__asm__ (".int_loop_start:  dclz s1, s0");*/
        intSource = 63 - intSource; 
        intBit = 1ULL << intSource;
        pHand = &bcm1250IntHandlers[intSource];
        if (pHand->routine != NULL)
            (* pHand->routine) (pHand->parameter);
        intStatus ^= intBit;   /* clear the bit */        
    __asm__ ("bnez %0, .int_loop_start; .int_loop_end:" : :"r"(intStatus));	

#else
    intBit = 1ULL;
    for (intSource = 0; intStatus != 0; intSource++)
	{
	if ((intStatus & intBit) != 0)
	    {
	    pHand = &bcm1250IntHandlers[intSource];
	    if (pHand->routine != NULL)
	        (* pHand->routine) (pHand->parameter);
	    intStatus ^= intBit;   /* clear the bit */
	    }
	intBit <<= 1;
	}
#endif
    }

/*******************************************************************************
*
* bcm1250IntConnect - connect the interrupt handler to the BCM1250 interrupt.
*
* This routine maps one of the 64 bcm1250 interrupt sources to a SB-1 core 
* interrupt line, and establishes a handler for that interrupt.  Only one 
* interrupt handler may be established to a given bcm1250 interrupt source.
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be built.
*
* ERRNO
*/

STATUS bcm1250IntConnect
    (
    int intrSource,             /* interrupt level / bcm1250 source number */
    int intrVecNum,		/* interrupt number to attach to     */
    VOIDFUNCPTR routine,        /* routine to be called              */
    int parameter               /* parameter to be passed to routine */
    )
    {
    BCM1250_INT_HANDLER_DATA *pHand;
    STATUS retStatus;
    int intrLine, oldLevel;

    if (bcm1250IntLibInitStatus != OK)
        return (ERROR);

    if (intrSource < 0 || intrSource >= BCM1250_NUM_INT_SOURCES)
        return (ERROR);

    pHand = &bcm1250IntHandlers[intrSource];

    if(intrVecNum < IV_INT0_VEC || intrVecNum > IV_INT5_VEC)
        return(ERROR);

    intrLine = intrVecNum - IV_INT0_VEC;

    retStatus = OK;
    oldLevel = intLock ();		/* LOCK INTERRUPT */

    /* If handler function already is attached, that's an error. */
    if (pHand->routine != NULL)
        retStatus = ERROR;

    /* Set up the callback info and map the interrupt, but DO NOT enable it. */
    if (retStatus == OK)
        {
        pHand->intrLine  = intrLine;
        pHand->routine   = routine;
        pHand->parameter = parameter;

        MIPS3_SD(&bcm1250IntMapBase[intrSource], intrLine);
        }

    intUnlock (oldLevel);		/* UNLOCK INTERRUPT */

    return (retStatus);
    }


/*****************************************************************************
*
* bcm1250IntDisconnect - disconnect the interrupt handler
*
* This routine disconnects the interrupt handler from the BCM1250 
* interrupt line.
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be removed.
*
* ERRNO
*/

STATUS bcm1250IntDisconnect
    (
    int intrSource
    )
    {
    BCM1250_INT_HANDLER_DATA *pHand;
    UINT64 imr;
    int oldLevel;
    STATUS retStatus;

    if (bcm1250IntLibInitStatus != OK)
        return (ERROR);

    if (intrSource < 0 || intrSource >= BCM1250_NUM_INT_SOURCES)
        return (ERROR);

    pHand = &bcm1250IntHandlers[intrSource];

    retStatus = OK;
    oldLevel = intLock ();		/* LOCK INTERRUPT */

    /* If no handler function is attached, that's an error. */
    if (pHand->routine == NULL)
        retStatus = ERROR;

    /* Disable the interrupt if it's currently enabled. */
    if (retStatus == OK && pHand->intrSource != -1)
        {
        pHand->intrSource = -1;

        imr = MIPS3_LD(bcm1250IntMask);
        imr |= (1ULL << intrSource);
        MIPS3_SD(bcm1250IntMask, imr);
        }

    /* Remove the handler function. */
    pHand->routine = NULL;

    intUnlock (oldLevel);		/* UNLOCK INTERRUPT */

    return(retStatus);
    }


/*****************************************************************************
*
* bcm1250IntEnable - enable an external interrupt source
*
* This routine enables an external interrupt source on the SB-1
*
* RETURNS:
* OK, or ERROR if the interrupt source cannot be enabled.
*
* ERRNO
*/

STATUS bcm1250IntEnable
    (
    int intrSource
    )
    {
    BCM1250_INT_HANDLER_DATA *pHand;
    UINT64 imr;
    int oldLevel;
    STATUS retStatus;

    if (bcm1250IntLibInitStatus != OK)
        return (ERROR);

    if (intrSource < 0 || intrSource >= BCM1250_NUM_INT_SOURCES)
        return (ERROR);

    pHand = &bcm1250IntHandlers[intrSource];

    retStatus = OK;
    oldLevel = intLock ();		/* LOCK INTERRUPT */

    /* If no handler function is attached, that's an error. */
    if (pHand->routine == NULL)
        retStatus = ERROR;

    /* Enable the interrupt if it's not currently enabled. */
    if (retStatus == OK && pHand->intrSource == -1)
        {
        pHand->intrSource = intrSource;

        imr = MIPS3_LD(bcm1250IntMask);
        imr &= ~ (1ULL << intrSource);
        MIPS3_SD(bcm1250IntMask, imr);
        }

    intUnlock (oldLevel);		/* UNLOCK INTERRUPT */

    return (retStatus);
    }


/*****************************************************************************
*
* bcm1250IntDisable - disable an external interrupt source
*
* This routine disables an external interrupt source on the SB-1.
* RETURNS:
* OK, or ERROR if the interrupt source is an invalid source number, or
*   the source does not currently have a routine attached.
*
* ERRNO
*/

STATUS bcm1250IntDisable
    (
    int intrSource
    )
    {
    BCM1250_INT_HANDLER_DATA *pHand;
    UINT64 imr;
    int oldLevel;
    STATUS retStatus;

    if (bcm1250IntLibInitStatus != OK)
        return (ERROR);

    if (intrSource < 0 || intrSource >= BCM1250_NUM_INT_SOURCES)
        return (ERROR);

    pHand = &bcm1250IntHandlers[intrSource];

    retStatus = OK;
    oldLevel = intLock ();		/* LOCK INTERRUPT */

    /* If no handler function is attached, that's an error. */
    if (pHand->routine == NULL)
        retStatus = ERROR;

    /* Disable the interrupt if it's currently enabled. */
    if (retStatus == OK && pHand->intrSource != -1)
        {
        pHand->intrSource = -1;

        imr = MIPS3_LD(bcm1250IntMask);
        imr |= (1ULL << intrSource);
        MIPS3_SD(bcm1250IntMask, imr);
        }
 
    intUnlock (oldLevel);		/* UNLOCK INTERRUPT */

    return (retStatus);
    }
