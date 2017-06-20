/* sysEpic.c - Driver for Embedded Programmable Interrupt Controller */

/* $Id: sysEpic.c,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,27jan06,dtr  Tidyup and minor changes.
01a,08oct05,dtr  Created based on cds85xx/01h.
*/

/*
DESCRIPTION
This module implements the Embedded Programmable Interrupt Controller (EPIC)
driver for the MPC8548.

The EPIC is an integrated interrupt controller in the 8548 which
provides following major capabilities:

  Support for twelve external interrupt sources and thirty-two internal
  interrupt sources

  Support for connection of external interrupt controller device e.g. 8259
  like as implemented on a WinBond chip
     
  12 external interrupt sources
  32 internal interrupt sources
  16 programmable interrupt priority levels
  Fully-nested interrupt delivery
  Spurious vector generation
  Route to critical interrupt destinations
  Route to external pin
  Configurable sense and polarity at initialization and runtime

The current implementation of this EPIC controller does not support
the following features or mode of operations:

  PIC global timers
  Inter-processor interrupts
  Messaging interrupts

EPIC features are customized by writing into general control registers
or into interrupt level specific registers (IVPRs).

This driver allows a basic interface to the EPIC such as initializing it,
setting interrupt vectors, priorities, level/edge sense and interrupt
polarities, as well as enabling and disabling specific interrupts.

This driver implements a complete interrupt architecture system, complete
with vector table.

Since interrupt vectors can be shared, this driver does provide for
overloading of interrupt routines (i.e. there is
a list of interrupt routines for each interrupt vector (level)).  To service
a vector requires that all connected interrupt routines be called in order
of their connection.

The following diagram shows an example of how interrupts can
be configured in this system.

    EPIC Vector table
0  |-------|  <-- external INT0 starts
   |-------|
   |-------|------------------------------+
    ....                                  |
   |-------|                              |
   |-------|                              |
12 |-------| <-- internal INT0 starts     |   (EPIC_MAX_EXT_IRQS) = 12
   |-------|                              |
   |-------|                              |
    ....                                  |
   |-------|                              |
   |-------|                              |
44 |-------| <-- global timer INT0 starts |   (EPIC_MAX_EXT_IRQS + \
   |-------|                              |    EPIC_MAX_IN_IRQS) = 44
   |-------|                              |
   |-------|                              |
48 |-------| <-- message INT0 starts      |   (EPIC_MAX_EXT_IRQS + \
   |-------|                              |    EPIC_MAX_IN_IRQS + \
   |-------|                              |    EPIC_MAX_GT_IRQS) = 48
   |-------|                              |
52 |-------| <-- IPI INT0 starts          |   (EPIC_MAX_EXT_IRQS + \
   |-------|                              |    EPIC_MAX_IN_IRQS + \
   |-------|                              |    EPIC_MAX_GT_IRQS + \
55 |-------|                              |    EPIC_MAX_MSG_IRQS) = 52
56 |-------|<-----------------------------+   sysVectorIRQ0 = 56 \
    ....         WinBond int handler      |   = 52 + EPIC_MAX_IPI_IRQS
   |-------|                              +-------------+
   |-------|                                            |
   |-------|                                 PCI slot 3 int handler
   |-------|
   |-------|<----- Cascaded 8259s
   |-------|
   |-------|
    ....
   |-------|
   |-------|
   |-------|
   |-------|
256|-------|

The driver is designed to put external interrupts at the beginning of the
vector table.  As a result, devices that route their interrupt to the EPIC
on the MPC8540 does not need to translate the vector number.  Therefore,
the macros IVEC_TO_INUM(x) and INUM_TO_IVEC(x) are not necessary.  For
some existing drivers, it may be necessary to use the following defines:
  #undef  INUM_TO_IVEC
  #define INUM_TO_IVEC(x) (x)
  #undef  IVEC_TO_INUM
  #define IVEC_TO_INUM(x) (x)

If there are other devices in the system capable of generating their own
vectors then we presume that an appropriate interrupt handler is created
and attached to the vector associated with the correct IRQ number.  That
interrupt handler would get a new vector directly from the device and then
call all of the handlers attached to that new vector.  Vector information is
stored in a linked list of INT_HANDLER_DESC structures. The sysIntTbl array
contains a pointer to the first entry for each vector.

INITIALIZATION

This driver is initialized from the BSP, usually as part of sysHwInit().
The first routine to be called is sysEpicInit(). The routine resets the
global configuration register and resets the epic registers to default
values.

The second routine to be called is sysEpicIntrInit().  This routine takes no
arguments. This routine allocates the vector table and initializes the
chips to a default state.  All individual interrupt sources are disabled.
Each has to be individually enabled by intEnable() before it will be
unmasked and allowed to generate an interrupt.

CRITICAL INTERRUPT

To enable the EPIC to handle also critical interrupt, or if a normal
interrupt is to be rerouted to the critical input pin, INCLUDE_EPIC_CRT_INTR
should be defined.  e.g. Define the following in config.h:
  #define INCLUDE_EPIC_CRT_INTR     /@ include critical interrupt support @/

The critical interrupt handler uses information from the summary registers
CISR0 and CISR1.  The EPIC does not manage critical interrupts and hence
Iack or EOI do not apply.  It was seen that the summary registers go
through a transient state before settling on the result.  This causes
spurious interrupts to be generated, and the vectors being called.
A typical behavior is the printout of "uninitialized PIC interrupt
vector 0xXX".  This is observed only when at least one source has been
routed to critical pin.

CUSTOMIZING THIS DRIVER

The BSP can change the default polarity and sensitivity for the external
interrupt and the internal interrupt independently.  They are:
  EPIC_EX_DFT_SENSE	/@ default to EPIC_SENSE_LVL @/
  EPIC_EX_DFT_POLAR     /@ default to EPIC_INT_ACT_HIGH @/
  EPIC_IN_DFT_POLAR     /@ default to EPIC_INT_ACT_HIGH @/
If any of the above is defined before the inclusion of sysEpic.h, such as
in config.h, the default will be overridden.  The available options are:
  EPIC_EX_DFT_SENSE     EPIC_SENSE_LVL, EPIC_SENSE_EDG
  EPIC_xx_DFT_POLAR     EPIC_INT_ACT_LOW, EPIC_INT_ACT_HIGH /@if level-sense @/
                        EPIC_INT_EDG_NEG, EPIC_INT_EDG_POS  /@if edge-sense @/

The macros CPU_INT_LOCK() and CPU_INT_UNLOCK provide the access
to the CPU level interrupt lock/unlock routines.  We presume that there
is a single interrupt line to the CPU.  By default these macros call
intLock() and intUnlock() respectively.

INCLUDE FILES:
*/

/* includes */

#include <vxWorks.h>
#include "config.h"
#include "sysEpic.h"

#include <sysLib.h>
#include <stdio.h>
#include <string.h>

#ifdef INCLUDE_WINDVIEW
#include <private/eventP.h>
#endif

/* defines */

#ifndef CPU_INT_LOCK
#   define CPU_INT_LOCK(pData) \
	(*pData = intLock ())
#endif

#ifndef CPU_INT_UNLOCK
#   define CPU_INT_UNLOCK(data) \
	(intUnlock (data))
#endif

/* externs */

IMPORT STATUS 	excIntConnect (VOIDFUNCPTR *, VOIDFUNCPTR);
IMPORT STATUS 	excIntCrtConnect (VOIDFUNCPTR *, VOIDFUNCPTR);


/* get the interrupt hook routines  prototypes*/

IMPORT STATUS	(*_func_intConnectRtn) (VOIDFUNCPTR *, VOIDFUNCPTR, int);
IMPORT int	(*_func_intEnableRtn)  (int);
IMPORT int	(*_func_intdisableRtn) (int);

/* globals */

INT_HANDLER_DESC * sysIntTbl [INTERRUPT_TABLESIZE]; /* system interrupt tbl */

/* locals */

/* forward declarations */

LOCAL int	epicIntEnable (ULONG srcAddr);
LOCAL int	epicIntDisable (ULONG srcAddr);
LOCAL int	epicVecOptionsSet (ULONG srcAddr, UINT32 mask, UINT32 options);
LOCAL UINT32	epicVecOptionsGet (ULONG srcAddr);
LOCAL void	sysEpicIntHandlerExec (int vector);
LOCAL void 	sysEpicIntHandler (void);
#ifdef INCLUDE_EPIC_CRT_INTR
LOCAL void 	sysEpicCrtIntHandler (void);
LOCAL int	epicCrtIntSet (ULONG srcAddr);
LOCAL int	epicCrtIntUnset (ULONG srcAddr);
LOCAL int	epicCrtIntGet (ULONG srcAddr);
#endif  /* INCLUDE_EPIC_CRT_INTR */
LOCAL int       epicSrcAddrCheck (ULONG srcAddr);

/*******************************************************************************
*
* sysEpicInit - initialize the epic controller
*
* This routine resets the global Configuration Register, thus it:
*  -  disables all interrupts
*  -  sets epic registers to reset values
*
* It then sets the EPIC operation mode to Mixed Mode (vs. Pass Through
* mode). At this point only mixed mode is supported which means the EPIC
* is not configured for the pass through mode.
*
* Only direct interrupt sources is supported by the PIC.  Serial 
* stype interrupt is not available.
*
* \NOMANUAL
* 
* RETURNS: N/A
*
* ERRNO: N/A
*/    

void sysEpicInit (void)
    {
    ULONG 	gcrVal;
    int 	irq;

    gcrVal = sysEpicRegRead (EPIC_GLOBAL_REG);

    gcrVal |= (EPIC_GCR_RESET);
    
    sysEpicRegWrite (EPIC_GLOBAL_REG, gcrVal);

    /* wait for the reset sequence to be completed */
    
    while (sysEpicRegRead (EPIC_GLOBAL_REG) & EPIC_GCR_RESET)
        {
        ; /* do nothing */
        }

    gcrVal = sysEpicRegRead (EPIC_GLOBAL_REG);

    gcrVal |= (EPIC_GCR_MODE_MIXED);	/* configure for mixed mode */
    
    sysEpicRegWrite (EPIC_GLOBAL_REG, gcrVal);

    /* Clear all pending interrupt */
    while (((USHORT) epicIntAck()) != (USHORT) 0xffff)
        {
        /* do nothing */
	}

    /* init all EIVPRs to sense = 1, polarity = defined, vec = 0, prio = 0 */
    for (irq = 0; irq < EPIC_MAX_EXT_IRQS; irq++)
        {
        epicIntDisable (EPIC_EX_VEC_REG(irq));

        epicIntSourceSet (EPIC_EX_VEC_REG(irq),
                          EPIC_EX_DFT_POLAR, EPIC_EX_DFT_SENSE, 0x0, 0x0);
        }
	
    /* init all IIVPRs to polarity = defined, vec = 0, prio = 0 */
    for (irq = 0; irq < EPIC_MAX_IN_IRQS; irq++)
        {
        epicIntDisable (EPIC_IN_VEC_REG(irq));

        epicIntSourceSet (EPIC_IN_VEC_REG(irq),
                          EPIC_IN_DFT_POLAR, 0x0, 0x0, 0x0);
        }

    /* init all GTVPRs to vec = 0, prio = 0 */
    for (irq = 0; irq < EPIC_MAX_GT_IRQS; irq++)
        {
        epicIntDisable (EPIC_GT_VEC_REG(irq));

        epicIntSourceSet (EPIC_GT_VEC_REG(irq),
                          0x0, 0x0, 0x0, 0x0);
        }

    /* init all MIVPRs to vec = 0, prio = 0 */
    for (irq = 0; irq < EPIC_MAX_MSG_IRQS; irq++)
        {
        epicIntDisable (EPIC_MSG_VEC_REG(irq));

        epicIntSourceSet (EPIC_MSG_VEC_REG(irq),
                          0x0, 0x0, 0x0, 0x0);
        }

    /* disable IPIs */
    for (irq = 0; irq < EPIC_MAX_IPI_IRQS; irq++)
        {
        epicIntDisable (EPIC_IPI_VEC_REG(irq));

        epicIntSourceSet (EPIC_IPI_VEC_REG(irq),
                          0x0, 0x0, 0x0, 0x0);
        }

    epicCurTaskPrioSet (EPIC_PRIORITY_MAX); /* set it to highest priority */
    }

/*******************************************************************************
*
* sysEpicIntrInit - initialize the interrupt table
*
* This function initializes the interrupt mechanism of the board.
*
* RETURNS: OK, always.
*
* ERRNO: N/A
*/

STATUS  sysEpicIntrInit (void)
    {
    int vector;
    int rc;

    /* initialize the interrupt table */

    for (vector = 0; vector < INTERRUPT_TABLESIZE; vector++)
        {
	sysIntTbl [vector] = NULL;
        }

    /*
     * connect the interrupt demultiplexer to the PowerPC external 
     * interrupt exception vector.
     * i. e.  put the address of this interrupt handler in
     * the PowerPC's only external interrupt exception vector
     * which is  _EXC_OFF_INTR = 0x500
     * Also connect critical input pin handler _EXC_OFF_CRTL = 0x100.
     */

    rc = excIntConnect ((VOIDFUNCPTR *) _EXC_OFF_INTR, sysEpicIntHandler);
#ifdef INCLUDE_EPIC_CRT_INTR
    rc = excIntCrtConnect ((VOIDFUNCPTR *) _EXC_OFF_CRTL, sysEpicCrtIntHandler);
#endif  /* INCLUDE_EPIC_CRT_INTR */

    /*  
     * set up the BSP specific interrupt routines
     * Attach the local routines to the VxWorks system calls
     *
     */

    _func_intConnectRtn  =  sysEpicIntConnect;
    _func_intEnableRtn   =  sysEpicIntEnable;
    _func_intDisableRtn  =  sysEpicIntDisable;

    epicCurTaskPrioSet (EPIC_PRIORITY_MIN); /* set it to lowest priority */
    
    return (rc);
    }


/*******************************************************************************
*
* sysEpicIntEnable - enable a EPIC interrupt level
*
* This routine enables a specified EPIC interrupt level.
*
* \NOMANUAL
*
* RETURNS: OK, ERROR, or EPIC_INV_INTER_SOURCE.
*
* NOTE: To allow compatibility, sysEpicIntEnable() can be used only for
*       external and internal interrupts, and not GT, MSG, IPI.  It
*       assumes there are more internal IRQs than external.  It also
*       gives priority to external over internal given the same IRQ
*       is valid for both external and internal.
*
* ERRNO: N/A
*/

int sysEpicIntEnable
    (
    int vector
    )
    {
    if ((vector < 0)  || (vector >= EPIC_MAX_ALL_IRQS))
        return (ERROR);

    /* enable interrupt on EPIC */

    return epicIntEnable ( epicGetVecRegAdrs (vector) );
    }

/*******************************************************************************
*
* sysEpicIntDisable - disable a EPIC interrupt level
*
* This routine disables a specified EPIC interrupt level.
*
* \NOMANUAL
*
* RETURNS: OK, ERROR, or EPIC_INV_INTER_SOURCE.
*
* NOTE: To allow compatibility, sysEpicIntDisable() can be used only for
*       external and internal interrupts, and not GT, MSG, IPI.  It
*       assumes there are more internal IRQs than external.  It also
*       gives priority to external over internal given the same IRQ
*       is valid for both external and internal.
*
* ERRNO: N/A
*/

int  sysEpicIntDisable
    (
    int vector
    )
    {
    if ((vector < 0) || (vector >= EPIC_MAX_ALL_IRQS))
        return (ERROR);

    /* disable interrupt on EPIC */

    return epicIntDisable ( epicGetVecRegAdrs (vector) );
    }

/*******************************************************************************
*
* epicVecOptionsSet - set options for a vector
*
* <srcAddr> is the address offset of the Vector Priority register
* <mask> is one of EPIC_OPT_EN_MSK, EPIC_OPT_POLAR_MSK, EPIC_OPT_SENSE_MSK,
*                  EPIC_OPT_PRI_MSK, EPIC_OPT_EXPIN_MSK, EPIC_OPT_CRIT_MSK
* <options> is one of 
*   EPIC_OPT_EN_Y              interrupt enabled
*   EPIC_OPT_EN_N              interrupt disabled
*   EPIC_OPT_POLAR_ACT_LOW     polarity is active low
*   EPIC_OPT_POLAR_ACT_HIGH    polarity is active high
*   EPIC_OPT_POLAR_EDG_NEG     polarity is negative edge triggered
*   EPIC_OPT_POLAR_EDG_POS     polarity is positive edge triggered
*   EPIC_OPT_SENSE_EDG         edge sensitive
*   EPIC_OPT_SENSE_LVL         level sensitive
*   EPIC_OPT_PRI_VALUE(p)      set priority to p [0..15]
*   EPIC_OPT_EXPIN_OFF         external pin off
*   EPIC_OPT_EXPIN_ON          external pin on
*   EPIC_OPT_CRIT_OFF          route to critical pin off
*   EPIC_OPT_CRIT_ON           route to critical pin on
*
* \NOMANUAL
*
* RETURNS: OK or an error code, can be one of the following:
*          OK
*          ERROR
*          EPIC_VEC_OPTION_INV
*          EPIC_INV_INTER_SOURCE
*          EPIC_VEC_OPTION_NA
*          EPIC_VEC_HAS_NO_IDR
*          EPIC_INTER_IN_SERVICE
*
* ERRNO: N/A
*/

LOCAL int epicVecOptionsSet
    (
    ULONG       srcAddr,
    UINT32 	mask,
    UINT32 	options
    )
    {
    ULONG       vprVal;
    ULONG       idrVal;
    ULONG       vprFlag;
    ULONG       idrFlag;
    int         errCode;
    int         idrOffset = 0;              /* offset of IDR from VPR */

    if ((mask & ((0x3f30 << 16) | (0x3fff))) != 0)
        return (EPIC_VEC_OPTION_INV);

    errCode = epicSrcAddrCheck (srcAddr);

    if (errCode == EPIC_INV_INTER_SOURCE)
        {
        return (errCode);
        }

    vprFlag = mask & 0xffff0000;            /* upper sets upper VPR */
    idrFlag = (mask & 0x0000ffff) << 16;    /* lower sets upper IDR */

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            idrOffset = EPIC_EX_DEST_REG_VECREGOFF;
        break;

        case EPIC_IN_INTERRUPT:
            idrOffset = EPIC_IN_DEST_REG_VECREGOFF;
            if ((vprFlag & EPIC_OPT_SENSE_MSK) != 0)
                return (EPIC_VEC_OPTION_NA);
        break;

        case EPIC_GT_INTERRUPT:
            if (idrFlag != 0)
                return (EPIC_VEC_HAS_NO_IDR);
        break;

        case EPIC_MSG_INTERRUPT:
            idrOffset = EPIC_MSG_DEST_REG_VECREGOFF;
            if ((vprFlag & (EPIC_OPT_POLAR_MSK | EPIC_OPT_SENSE_MSK)) != 0)
                return (EPIC_VEC_OPTION_NA);
        break;

        case EPIC_IPI_INTERRUPT:
            if (idrFlag != 0)
                return (EPIC_VEC_HAS_NO_IDR);
        break;

        default:
            return (ERROR);
        }

    vprVal = sysEpicRegRead (srcAddr);

    if ((vprVal & EPIC_EIVPR_INTR_ACTIVE) != 0)
        return (EPIC_INTER_IN_SERVICE);

    if (vprFlag != 0)
        {
        vprVal &= ~(vprFlag);
        vprVal |= (options & 0xffff0000);
        sysEpicRegWrite (srcAddr, vprVal);
        }

    if ((idrFlag != 0) && (idrOffset != 0))
        {
        idrVal = sysEpicRegRead (srcAddr + idrOffset);
        idrVal &= ~(idrFlag);
        idrVal |= (options & 0xffff) << 16;
        sysEpicRegWrite (srcAddr + idrOffset, idrVal);
        }

    return OK;

    }

/*******************************************************************************
*
* epicVecOptionsGet - get options for a vector
*
* <srcAddr> is the address offset of the Vector Priority register
*
* Use one or more of the following masks to extract the returned value:
*   EPIC_OPT_EN_MSK
*   EPIC_OPT_POLAR_MSK
*   EPIC_OPT_SENSE_MSK
*   EPIC_OPT_PRI_MSK
*   EPIC_OPT_EXPIN_MSK
*   EPIC_OPT_CRIT_MSK
*
* \NOMANUAL
*
* RETURNS: OK, ERROR, or EPIC_INV_INTER_SOURCE
*
* ERRNO: N/A
*/

LOCAL UINT32 epicVecOptionsGet
    (
    ULONG       srcAddr
    )
    {
    ULONG       vprVal;
    ULONG       idrVal;
    int         errCode;
    int         idrOffset = 0;              /* offset of IDR from VPR */

    errCode = epicSrcAddrCheck (srcAddr);

    if (errCode == EPIC_INV_INTER_SOURCE)
        {
        return (errCode);
        }

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            idrOffset = EPIC_EX_DEST_REG_VECREGOFF;
        break;

        case EPIC_IN_INTERRUPT:
            idrOffset = EPIC_IN_DEST_REG_VECREGOFF;
        break;

        case EPIC_GT_INTERRUPT:
        break;

        case EPIC_MSG_INTERRUPT:
            idrOffset = EPIC_MSG_DEST_REG_VECREGOFF;
        break;

        case EPIC_IPI_INTERRUPT:
        break;

        default:
            return ((UINT32) ERROR);
        }

    vprVal = sysEpicRegRead (srcAddr);
    if (idrOffset != 0)
        idrVal = sysEpicRegRead (srcAddr + idrOffset);
    else
        idrVal = 0;

    return ((vprVal & 0xffff0000) | (idrVal >> 16));
    }

/*
*
* sysEpicVecOptionsSet - change options for a vector
*
*/

int sysEpicVecOptionsSet
    (
    int    vector,
    UINT32 mask,
    UINT32 options
    )
    {
    if ((vector < 0) || (vector >= EPIC_MAX_ALL_IRQS))
        return (ERROR);

    /* set vector options on EPIC */

    return epicVecOptionsSet ( epicGetVecRegAdrs (vector), mask, options );
    }

/*
*
* sysEpicVecOptionsGet - obtain options for a vector
*
*/

UINT32 sysEpicVecOptionsGet
    (
    int vector
    )
    {
    if ((vector < 0) || (vector >= EPIC_MAX_ALL_IRQS))
        return ((UINT32) ERROR);

    /* get vector options on EPIC */

    return epicVecOptionsGet ( epicGetVecRegAdrs (vector) );
    }

/*******************************************************************************
*
* sysEpicIntConnect - connect an interrupt handler to the system vector table
*
* This function connects an interrupt handler to the system vector table.
*
* RETURNS: OK or ERROR.
*
* ERRNO: N/A
*/

STATUS sysEpicIntConnect
    (
    VOIDFUNCPTR * 	vector,		/* interrupt vector to attach */
    VOIDFUNCPTR		routine,	/* routine to be called */
    int			parameter	/* parameter to be passed to routine */
    )
    {
    INT_HANDLER_DESC *	pNewHandler;
    INT_HANDLER_DESC *	pCurrHandler;
    int			intVal;
    BOOL		sharing = FALSE;

    if (((int)vector < 0)  || ((int) vector >= INTERRUPT_TABLESIZE)) 
	{
        return (ERROR);   /*  out of range  */
	}

    /* create a new interrupt handler */

    pNewHandler = malloc (sizeof (INT_HANDLER_DESC));

    /* check if the memory allocation succeed */

    if (pNewHandler == NULL)

	return (ERROR);

    /*  initialize the new handler  */

    pNewHandler->vec = routine;
    pNewHandler->arg = parameter;
    pNewHandler->next = NULL;

    /* install the handler in the system interrupt table  */

    intVal = intLock (); /* lock interrupts to prevent races */

    if (sysIntTbl [(int) vector] == NULL)
	{
        sysIntTbl [(int) vector] = pNewHandler;  /* single int. handler case */
	}
    else
	{
        pCurrHandler = sysIntTbl[(int) vector];/* multiple int. handler case */

        while (pCurrHandler->next != NULL)
            {
            pCurrHandler = pCurrHandler->next;
            }
        
        pCurrHandler->next = pNewHandler;

        sharing = TRUE;
	}

    if ((int)vector >= 0 && (int)vector < EPIC_MAX_ALL_IRQS)
        {

        /* EPIC IRQ set EPIC registers */

        if (!sharing)
            {
            if ((int) vector < EPIC_VEC_IN_IRQ0)        /* EX type */
                {
                epicIntSourceSet ( epicGetVecRegAdrs ((int) vector),
                               EPIC_EX_DFT_POLAR, EPIC_EX_DFT_SENSE,
                               EPIC_PRIORITY_DEFAULT, (int) vector);
                }
            else if ((int) vector < EPIC_VEC_GT_IRQ0)   /* IN type */
                {
                epicIntSourceSet ( epicGetVecRegAdrs ((int) vector),
                               EPIC_IN_DFT_POLAR, 0x0,
                               EPIC_PRIORITY_DEFAULT, (int) vector);
                }
            else                                        /* other types */
                {
                epicIntSourceSet ( epicGetVecRegAdrs ((int) vector),
                               0x0, 0x0,
                               EPIC_PRIORITY_DEFAULT, (int) vector);
                }
            }
        }

    intUnlock (intVal);

    return (OK);
    }

/*******************************************************************************
*
* sysEpicIntHandlerExec  - execute the handlers for a given vector
* 
* This routine executes all the handlers chained to a given vector.
* If a vector has no handlers attached to it, a logMsg is generated.
*
* \NOMANUAL
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysEpicIntHandlerExec
    (
    int vector
    )
    {    
    INT_HANDLER_DESC * pCurrHandler;

    /*  call each respective interrupt handler */	

    if ((pCurrHandler = sysIntTbl [vector]) == NULL)
        {
	logMsg ("uninitialized PIC interrupt vector 0x%x\r\n",
                vector, 0,0,0,0,0);
        }
    else
        {


	/* call Each respective chained interrupt handler  */
	while (pCurrHandler != NULL)
            {
  	    (*pCurrHandler->vec) (pCurrHandler->arg);
	    pCurrHandler = pCurrHandler->next;
            }

        }
    }

/*******************************************************************************
*
* sysEpicIntHandler - handles the EPIC interrupts to the CPU
*
* This routine handles interrupts originating from the embedded interrupt
* controller on the MPC8540 PowerPC processor.
* This handler is entered from the 0x500 exception.
*
* This routine is entered with CPU external interrupts enables.
*
* Since the EPIC is the primary interrupt controller this driver
* first initiates an Epic acknowledge call and reads the vector
* put out by the EPIC. Subsequent vectors have to be obtained if
* an external interrupt controller is connected to one of the
* epic handlers. 
*
* This routine then processes the interrupt by calling all the interrupt
* service routines chained to the vector.
*
* Finally, this routine re-arms the interrupt at the PIC by performing an 
* PIC EOI for both the EPIC and the 8259s.
*
* RETURNS:  N/A
*
* ERRNO: N/A
*/

LOCAL void  sysEpicIntHandler (void)
    {
    int	epicIntVec;
    int oldkey;

    epicIntVec = epicIntAck ();	/* clear int, return the vec for highest IRQ */

#ifdef INCLUDE_WINDVIEW
    WV_EVT_INT_ENT (epicIntVec);
#endif 

    /* loop till all interrupts are cleared */
    while (epicIntVec != 0xffff)
        {


        /* Allow external interrupts to the CPU. */

	CPU_INT_UNLOCK (_PPC_MSR_EE);
        sysEpicIntHandlerExec (epicIntVec);
	CPU_INT_LOCK (&oldkey);

        /*
         * Disable External Interrupts
         * External Interrupts will be re-enabled in the kernel's wrapper
         * of this Interrupt.
         */


        WRS_ASM ("sync");
        epicEOI();			/* signal end of interrupt on EPIC */
        WRS_ASM ("sync");

        epicIntVec = epicIntAck ();


        } /* while */

    }

#ifdef INCLUDE_EPIC_CRT_INTR
/*
*
* epicCisr0Get - get critical interrupt summary
*
*/

UINT32 epicCisr0Get (void)
    {
    volatile unsigned long val;

    val = *(volatile unsigned long *) (CCSBAR | 0x41330);
    return val;
    }

/*
*
* epicCisr1Get - get critical interrupt summary
*
*/
UINT32 epicCisr1Get (void)
    {
    volatile unsigned long val;

    val = *(volatile unsigned long *) (CCSBAR | 0x41340);
    return val;
    }

/*
*
* sysEpicCrtIntHandler
*
*/

LOCAL void  sysEpicCrtIntHandler (void)
    {
    int oldkey;

    volatile unsigned long extSumm;
    volatile unsigned long intSumm;
    int i;

    while (((extSumm = (epicCisr0Get() & 0xfff)) != 0) ||
           ((intSumm = epicCisr1Get()) != 0))
        {
        /* service external first */
        for (i = 0; i < 12; i++)
            {
            if (((extSumm >> i) & 0x1) == 0x1)
                {
                /* CPU_INT_UNLOCK (_PPC_MSR_CE); */
                sysEpicIntHandlerExec (11 - i);
                /* CPU_INT_LOCK (&oldkey); */
                }
            }

        /* service internal */
        for (i = 0; i < 32; i++)
            {
            if (((intSumm >> i) & 0x1) == 0x1)
                {
                /* CPU_INT_UNLOCK (_PPC_MSR_CE); */
                sysEpicIntHandlerExec (EPIC_VEC_IN_IRQ0 + 31 - i);
                /* CPU_INT_LOCK (&oldkey); */
                }
            }

        }
    }
#endif  /* INCLUDE_EPIC_CRT_INTR */

/*******************************************************************************
*
* epicCurTaskPrioSet - set the priority of the current task.
*
* NOTES
*
* epicCurTaskPrioSet sets the priority of the Processor Current Task
* Priority register to the value of the prioNum parameter.  This function
* should be called after sysEpicInit() to lower the priority of the processor
* current task. Note that valid priority values are 0 through 15 (15 being
* the highest priority)
*
* \NOMANUAL
* 
* RETURNS: previous priority of the task.
*
* ERRNO: N/A
*/

int epicCurTaskPrioSet
    (
    int prioNum
    )
    {
    ULONG oldPrio;

    if ((prioNum < EPIC_PRIORITY_MIN) || (prioNum > EPIC_PRIORITY_MAX))
        {
        return ((int) EPIC_INV_PRIO_ERROR);
	}

    oldPrio = sysEpicRegRead (EPIC_CTASK_PRI_REG);

    sysEpicRegWrite (EPIC_CTASK_PRI_REG, prioNum);

    return (oldPrio);
    }

/*******************************************************************************
*
* epicIntEnable - enable an EPIC interrupt, given its IVPR
*
* This function clears the mask bit of an external, an internal or
* a Timer register to enable the interrupt.
*
* <srcAddr> is the address offset of the Vector Priority register  
*
* \NOMANUAL
* 
* RETURNS: OK or an error code if the IVPR passed in was invalid.
*
* ERRNO: N/A
*/

LOCAL int epicIntEnable
    (
    ULONG 	srcAddr
    )
    {
    ULONG 	srcVal;
    int 	errCode;
  
    errCode = epicSrcAddrCheck (srcAddr);

    if (errCode == EPIC_INV_INTER_SOURCE)
        {
        return (errCode);
        }

    srcVal = sysEpicRegRead (srcAddr);

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            srcVal &= ~(EPIC_EIVPR_INTR_MSK);   /* clear the mask bit */
        break;

        case EPIC_IN_INTERRUPT:
            srcVal &= ~(EPIC_IIVPR_INTR_MSK);   /* clear the mask bit */
        break;

        case EPIC_GT_INTERRUPT:
            srcVal &= ~(EPIC_GTVPR_INTR_MSK);   /* clear the mask bit */
        break;

        case EPIC_MSG_INTERRUPT:
            srcVal &= ~(EPIC_MIVPR_INTR_MSK);   /* clear the mask bit */
        break;

        case EPIC_IPI_INTERRUPT:
            srcVal &= ~(EPIC_IPIVPR_INTR_MSK);  /* clear the mask bit */
        break;

        default:
            return (ERROR);
        }

    sysEpicRegWrite (srcAddr, srcVal);

    return OK;

    }

/*******************************************************************************
*
* epicIntDisable - disable an EPIC interrupt, given its IVPR
*
* This function sets the mask bit of an external, an internal,
* a timer, a message, or an IPI register to disable the interrupt.
* <srcAddr> is the address offset of the Vector Priority register.
*
* \NOMANUAL
* 
* RETURNS: OK or an error code if the IVPR passed in was invalid.
*
* ERRNO: N/A
*/
               
LOCAL int epicIntDisable
    (
    ULONG 	srcAddr
    )
    {
    ULONG 	srcVal;
    int 	errCode;
  
    errCode = epicSrcAddrCheck (srcAddr);

    if (errCode == EPIC_INV_INTER_SOURCE)
        {
        return (errCode);
        }

    srcVal = sysEpicRegRead (srcAddr);

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            srcVal |= EPIC_EIVPR_INTR_MSK;	/* set the mask bit */
        break;

        case EPIC_IN_INTERRUPT:
            srcVal |= EPIC_IIVPR_INTR_MSK;	/* set the mask bit */
        break;

        case EPIC_GT_INTERRUPT:
            srcVal |= EPIC_GTVPR_INTR_MSK;	/* set the mask bit */
        break;

        case EPIC_MSG_INTERRUPT:
            srcVal |= EPIC_MIVPR_INTR_MSK;	/* set the mask bit */
        break;

        case EPIC_IPI_INTERRUPT:
            srcVal |= EPIC_IPIVPR_INTR_MSK;	/* set the mask bit */
        break;

        default:
            return (ERROR);
        }

    sysEpicRegWrite (srcAddr, srcVal);

    return OK;
    }

#ifdef INCLUDE_EPIC_CRT_INTR
/*
*
*/
LOCAL int epicCrtIntSet
    (
    ULONG 	srcAddr
    )
    {
    ULONG 	srcVal;
    int 	errCode = EPIC_EX_INTERRUPT;    /* same bits for all types */
  
    srcVal = sysEpicRegRead (srcAddr);

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            srcVal |= EPIC_EIDR_CRIT_INT;       /* set critical bit */
        break;

        case EPIC_IN_INTERRUPT:
            srcVal |= EPIC_IIDR_CRIT_INT;       /* set critical bit */
        break;

        case EPIC_GT_INTERRUPT:
            return (ERROR);                     /* not supported */
        break;

        case EPIC_MSG_INTERRUPT:
            srcVal |= EPIC_MIDR_CRIT_INT;       /* set critical bit */
        break;

        case EPIC_IPI_INTERRUPT:
            return (ERROR);                     /* not supported */
        break;

        default:
            return (ERROR);
        }

    sysEpicRegWrite (srcAddr, srcVal);

    return OK;
    }

/*
*
*/
LOCAL int epicCrtIntUnset
    (
    ULONG 	srcAddr
    )
    {
    ULONG 	srcVal;
    int 	errCode = EPIC_EX_INTERRUPT;    /* same bits for all types */
  
    srcVal = sysEpicRegRead (srcAddr);

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            srcVal &= ~(EPIC_EIDR_CRIT_INT);    /* set critical bit */
        break;

        case EPIC_IN_INTERRUPT:
            srcVal &= ~(EPIC_IIDR_CRIT_INT);    /* set critical bit */
        break;

        case EPIC_GT_INTERRUPT:
            return (ERROR);                     /* not supported */
        break;

        case EPIC_MSG_INTERRUPT:
            srcVal &= ~(EPIC_MIDR_CRIT_INT);    /* set critical bit */
        break;

        case EPIC_IPI_INTERRUPT:
            return (ERROR);                     /* not supported */
        break;

        default:
            return (ERROR);
        }

    sysEpicRegWrite (srcAddr, srcVal);

    return OK;
    }

/*
*
*/
LOCAL int epicCrtIntGet
    (
    ULONG 	srcAddr
    )
    {
    ULONG 	srcVal;
    int 	errCode = EPIC_EX_INTERRUPT;    /* same bits for all types */
  
    srcVal = sysEpicRegRead (srcAddr);

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            srcVal = (srcVal >> 30) & 1;
        break;
        }

    return srcVal;
}

/*
*
*/
int  sysEpicCrtIntSet
    (
    int vector
    )
    {

    if ((vector < 0)  || (vector >= EPIC_MAX_ALL_IRQS))
        return (ERROR);

    /* enable critical bit on EPIC */

    return epicCrtIntSet ( epicGetDestRegAdrs (vector) );
    }

/*
*
*/
int  sysEpicCrtIntUnset
    (
    int vector
    )
    {

    if ((vector < 0)  || (vector >= EPIC_MAX_ALL_IRQS))
        return (ERROR);

    /* disable critical bit on EPIC */

    return epicCrtIntUnset ( epicGetDestRegAdrs (vector) );
    }

/*
*
*/
int  sysEpicCrtIntGet
    (
    int vector
    )
    {

    if ((vector < 0)  || (vector >= EPIC_MAX_ALL_IRQS))
        return (ERROR);

    /* obtain critical bit on EPIC */

    return epicCrtIntGet ( epicGetDestRegAdrs (vector) );
    }
#endif  /* INCLUDE_EPIC_CRT_INTR */

/*******************************************************************************
*
* epicIntAck - read the IACK register and return vector
* 
* NOTES
*
* epicIntAck reads the Interrupt acknowledge register and return
* the vector number of the highest pending interrupt.
*
* \NOMANUAL
* 
* RETURNS: the vector number of the highest priority pending interrupt.
*
* ERRNO: N/A
*/

int epicIntAck(void)
    {
    int eumbVal;

    eumbVal = sysEpicRegRead (EPIC_INT_ACK_REG);
    WRS_ASM ("isync;sync");
    return eumbVal;

    }

/*******************************************************************************
*
* epic EOI 0 signal end of interrupt on the EPIC
*
* NOTES
*
* epicEOI writes 0x0 to the EOI register to signal end of interrupt.
* This function is usually called after an interrupt routine is served.
*
* \NOMANUAL
* 
* RETURNS: N/A
*
* ERRNO: N/A
*/

void epicEOI(void)
    {
    sysEpicRegWrite (EPIC_EOI_REG, 0x0);
    }

/*******************************************************************************
*
* epicGetVecRegAdrs - translate a vector to vector reg address
*
* This routine translates a vector to vector reg address.
*
* \NOMANUAL
*
* RETURNS: vector register address of the corresponding vector type
*/

ULONG epicGetVecRegAdrs
    (
    int vector
    )
    {
    if ((vector < EPIC_VEC_EXT_IRQ0) || (vector >= EPIC_VEC_CTRL_EXT))
        return ((ULONG) ERROR);

    if (vector < EPIC_VEC_IN_IRQ0)    /* type EXT */
        {
        return (EPIC_EX_VEC_REG (vector - EPIC_VEC_EXT_IRQ0));
        }

    if (vector < EPIC_VEC_GT_IRQ0)    /* type IN */
        {
        return (EPIC_IN_VEC_REG (vector - EPIC_VEC_IN_IRQ0));
        }

    if (vector < EPIC_VEC_MSG_IRQ0)    /* type GT */
        {
        return (EPIC_GT_VEC_REG (vector - EPIC_VEC_GT_IRQ0));
        }

    if (vector < EPIC_VEC_IPI_IRQ0)    /* type MSG */
        {
        return (EPIC_MSG_VEC_REG (vector - EPIC_VEC_MSG_IRQ0));
        }

    if (vector < EPIC_VEC_CTRL_EXT)    /* type IPI */
        {
        return (EPIC_IPI_VEC_REG (vector - EPIC_VEC_IPI_IRQ0));
        }

    /* should not reach here */
    return ((ULONG) ERROR);
    }

/*******************************************************************************
*
* epicGetDestRegAdrs - translate a vector to destination reg address
*
* This routine translates a vector to destination reg address
*
* \NOMANUAL
*
* RETURNS: the destination register address of the corresponding vector type
*
* ERRNO: N/A
*/

ULONG epicGetDestRegAdrs
    (
    int vector
    )
    {
    if ((vector < EPIC_VEC_EXT_IRQ0) || (vector >= EPIC_VEC_CTRL_EXT))
        return ((ULONG) ERROR);

    if (vector < EPIC_VEC_IN_IRQ0)    /* type EXT */
        {
        return (EPIC_EX_DEST_REG (vector - EPIC_VEC_EXT_IRQ0));
        }

    if (vector < EPIC_VEC_GT_IRQ0)    /* type IN */
        {
        return (EPIC_IN_DEST_REG (vector - EPIC_VEC_IN_IRQ0));
        }

    if (vector < EPIC_VEC_MSG_IRQ0)    /* type GT */
        {
        return ((ULONG) ERROR);
        }

    if (vector < EPIC_VEC_IPI_IRQ0)    /* type MSG */
        {
        return (EPIC_MSG_DEST_REG (vector - EPIC_VEC_MSG_IRQ0));
        }

    if (vector < EPIC_VEC_CTRL_EXT)    /* type IPI */
        {
        return ((ULONG) ERROR);
        }

    /* should not reach here */
    return ((ULONG) ERROR);
    }

/*******************************************************************************
*
* epicSrcAddrCheck - check source of the VPR address
*
* This routine checks if the VPR address passed in is external, internal, 
* timer, message, or IPI.
*
* \NOMANUAL
* 
* RETURNS: EPIC_IN_INTERRUPT for internal interrupt sources, or
*          EPIC_EX_INTERRUPT for external ones, or
*          EPIC_GT_INTERRUPT for global timer ones, or
*          EPIC_MSG_INTERRUPT for message ones, or
*          EPIC_IPI_INTERRUPT for IPI ones, or
*          EPIC_INV_INTER_SOURCE if an invalid address was passed.
*
* ERRNO: N/A
*/

LOCAL int epicSrcAddrCheck
    (
    ULONG srcAddr
    )
    {
    switch (srcAddr)
        {
	case EPIC_EX_INT0_VEC_REG:
	case EPIC_EX_INT1_VEC_REG:
	case EPIC_EX_INT2_VEC_REG:
	case EPIC_EX_INT3_VEC_REG:
	case EPIC_EX_INT4_VEC_REG:
	case EPIC_EX_INT5_VEC_REG:
	case EPIC_EX_INT6_VEC_REG:
	case EPIC_EX_INT7_VEC_REG:
	case EPIC_EX_INT8_VEC_REG:
	case EPIC_EX_INT9_VEC_REG:
	case EPIC_EX_INT10_VEC_REG:
	case EPIC_EX_INT11_VEC_REG:
            return (EPIC_EX_INTERRUPT);
	break;

	case EPIC_IN_INT0_VEC_REG:
	case EPIC_IN_INT1_VEC_REG:
	case EPIC_IN_INT2_VEC_REG:
	case EPIC_IN_INT3_VEC_REG:
	case EPIC_IN_INT4_VEC_REG:
	case EPIC_IN_INT5_VEC_REG:
	case EPIC_IN_INT6_VEC_REG:
	case EPIC_IN_INT7_VEC_REG:
	case EPIC_IN_INT8_VEC_REG:
	case EPIC_IN_INT9_VEC_REG:
	case EPIC_IN_INT10_VEC_REG:
	case EPIC_IN_INT11_VEC_REG:
	case EPIC_IN_INT12_VEC_REG:
	case EPIC_IN_INT13_VEC_REG:
	case EPIC_IN_INT14_VEC_REG:
	case EPIC_IN_INT15_VEC_REG:
	case EPIC_IN_INT16_VEC_REG:
	case EPIC_IN_INT17_VEC_REG:
	case EPIC_IN_INT18_VEC_REG:
	case EPIC_IN_INT19_VEC_REG:
	case EPIC_IN_INT20_VEC_REG:
	case EPIC_IN_INT21_VEC_REG:
	case EPIC_IN_INT22_VEC_REG:
	case EPIC_IN_INT23_VEC_REG:
	case EPIC_IN_INT24_VEC_REG:
	case EPIC_IN_INT25_VEC_REG:
	case EPIC_IN_INT26_VEC_REG:
	case EPIC_IN_INT27_VEC_REG:
	case EPIC_IN_INT28_VEC_REG:
	case EPIC_IN_INT29_VEC_REG:
	case EPIC_IN_INT30_VEC_REG:
	case EPIC_IN_INT31_VEC_REG:
	case EPIC_IN_INT32_VEC_REG:
	case EPIC_IN_INT33_VEC_REG:
	case EPIC_IN_INT34_VEC_REG:
	case EPIC_IN_INT35_VEC_REG:
	case EPIC_IN_INT36_VEC_REG:
	case EPIC_IN_INT37_VEC_REG:
	case EPIC_IN_INT38_VEC_REG:
	case EPIC_IN_INT39_VEC_REG:
	case EPIC_IN_INT40_VEC_REG:
	case EPIC_IN_INT41_VEC_REG:
	case EPIC_IN_INT42_VEC_REG:
	case EPIC_IN_INT43_VEC_REG:
	case EPIC_IN_INT44_VEC_REG:
	case EPIC_IN_INT45_VEC_REG:
	case EPIC_IN_INT46_VEC_REG:
	case EPIC_IN_INT47_VEC_REG:
            return (EPIC_IN_INTERRUPT);
	break;

	case EPIC_TM0_VEC_REG:
	case EPIC_TM1_VEC_REG:
	case EPIC_TM2_VEC_REG:
	case EPIC_TM3_VEC_REG:
            return (EPIC_GT_INTERRUPT);
	break;

	case EPIC_MSG_INT0_VEC_REG:
	case EPIC_MSG_INT1_VEC_REG:
	case EPIC_MSG_INT2_VEC_REG:
	case EPIC_MSG_INT3_VEC_REG:
            return (EPIC_MSG_INTERRUPT);
	break;

	case EPIC_IPI_0_VEC_REG:
	case EPIC_IPI_1_VEC_REG:
	case EPIC_IPI_2_VEC_REG:
	case EPIC_IPI_3_VEC_REG:
            return (EPIC_IPI_INTERRUPT);
	break;

        default:
            return (EPIC_INV_INTER_SOURCE);

        }
    }

/*******************************************************************************
*
* epicIntSourceSet - set interrupt parameters for an interrupt register
*
* This function sets the interrupt parameters for:
*     External vector/priority register (EIVPR)
*     Internal vector/priority register (IIVPR)
*     Global Timer vector/priority register (GTVPR)
*     Message vector/priority register (MIVPR)
*     IPI vector/priority register (IPIVPR)
* The interrupt parameters can only be set when the current source is not
* in-request or in-service, which is determined by the Activity bit.  This
* routine reads the Activity bit; if the value of this bit is 1 (in-request
* or in-service), it returns error; otherwise, it sets the value of the
* parameters to the register offset defined in srcAddr.
*
* inputs:  srcAddr:   Address Offset of the source interrupt register.  This
*		      routine assumes that the srcAddr passed in is an valid
*		      Source Vector Priority address.
*          polarity: Use by external & internal interrupts only.
*                    1 -  Enable active high or positive edge
*                    0 -  Enable active low or negative edge
*          sense:    Use by external interrupts only.
*                    1 -  Enable level sensitive interrupts
*                    0 -  Enable edge sensitive interrupts
*          priority: valid number 0 to 15   
*          vector:   valid number 0 - 65535 (16 bits)
*
* Errors:  EPIC_INV_INTER_SOURCE Invalid Source Address,
*          EPIC_INTER_IN_SERVICE Interrupt currently in service
*          ERROR Unknown type
*
* \NOMANUAL
* 
* RETURNS: OK or one of errors above
*
* ERRNO: N/A
*/

STATUS epicIntSourceSet
    (
    ULONG 	srcAddr,     
    int 	polarity,
    int 	sense,
    int 	priority,
    int 	vector
    )
    {
    ULONG 	srcVal;
    ULONG	errCode;
  
    errCode = epicSrcAddrCheck(srcAddr);

    if (errCode == EPIC_INV_INTER_SOURCE)
        {
        return (errCode);
        }

    srcVal = sysEpicRegRead(srcAddr);

    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            if (srcVal & EPIC_EIVPR_INTR_ACTIVE)
                {
                return (EPIC_INTER_IN_SERVICE);
                }
            /* mask off current settings */
            srcVal &= ~(EPIC_EIVPR_INTR_POLARITY |
                        EPIC_EIVPR_INTR_SENSE |
                        EPIC_EIVPR_PRIORITY_MSK  |
                        EPIC_EIVPR_VECTOR_MSK);
            /* set new values */
            srcVal |= (EPIC_EIVPR_POLARITY (polarity) |
                       EPIC_EIVPR_SENS (sense) |
                       EPIC_EIVPR_PRIORITY (priority) |
                       EPIC_EIVPR_VECTOR (vector));
        break;

        case EPIC_IN_INTERRUPT:
            if (srcVal & EPIC_IIVPR_INTR_ACTIVE)
                {
                return (EPIC_INTER_IN_SERVICE);
                }
            /* mask off current settings */
            srcVal &= ~(EPIC_IIVPR_INTR_POLARITY |
                        EPIC_IIVPR_PRIORITY_MSK  |
                        EPIC_IIVPR_VECTOR_MSK);
            /* set new values */
            srcVal |= (EPIC_IIVPR_POLARITY (polarity) |
                       EPIC_IIVPR_PRIORITY (priority) |
                       EPIC_IIVPR_VECTOR (vector));
        break;

        case EPIC_GT_INTERRUPT:
            if (srcVal & EPIC_GTVPR_INTR_ACTIVE)
                {
                return (EPIC_INTER_IN_SERVICE);
                }
            /* mask off current settings */
            srcVal &= ~(EPIC_GTVPR_PRIORITY_MSK  |
                        EPIC_GTVPR_VECTOR_MSK);
            /* set new values */
            srcVal |= (EPIC_GTVPR_PRIORITY (priority) |
                       EPIC_GTVPR_VECTOR (vector));
        break;

        case EPIC_MSG_INTERRUPT:
            if (srcVal & EPIC_MIVPR_INTR_ACTIVE)
                {
                return (EPIC_INTER_IN_SERVICE);
                }
            /* mask off current settings */
            srcVal &= ~(EPIC_MIVPR_PRIORITY_MSK  |
                        EPIC_MIVPR_VECTOR_MSK);
            /* set new values */
            srcVal |= (EPIC_MIVPR_PRIORITY (priority) |
                       EPIC_MIVPR_VECTOR (vector));
        break;

        case EPIC_IPI_INTERRUPT:
            if (srcVal & EPIC_IPIVPR_INTR_ACTIVE)
                {
                return (EPIC_INTER_IN_SERVICE);
                }
            /* mask off current settings */
            srcVal &= ~(EPIC_IPIVPR_PRIORITY_MSK  |
                        EPIC_IPIVPR_VECTOR_MSK);
            /* set new values */
            srcVal |= (EPIC_IPIVPR_PRIORITY (priority) |
                       EPIC_IPIVPR_VECTOR (vector));
        break;

        default:
            return (ERROR);
        }

    sysEpicRegWrite(srcAddr, srcVal);

    return (OK);
    }


/*******************************************************************************
*
* epicIntSourceGet - retrieve information of an EPIC source vector register.
*
* This function retrieves information of an epic source vector register.
* The information includes the Enable bit, the polarity, sense bits, the
* interrupt priority and interrupt vector number.
* Input:  srcAddr   - address of the source vector register
* Output: enable    - whether the interrupt is enabled
*          polarity - interrupt polarity (high or low)
*          sense    - interrupt sense (level or edge)
*          Priority - interrupt priority
*          Vector   - interrupt vector number
*
* \NOMANUAL
* 
* RETURNS: OK or ERROR or EPIC_INV_INTER_SOURCE
*
* ERRNO: N/A
*/

STATUS epicIntSourceGet
    (
    ULONG 	srcAddr,
    int *	pEnableMask,
    int *	pPolarity,
    int *	pSense,
    int *	pPriority,
    int *	pVector
    )
    {
    ULONG 	srcVal;
    int 	errCode;
  
    errCode = epicSrcAddrCheck (srcAddr);

    if (errCode == EPIC_INV_INTER_SOURCE)
        {
        return errCode;
        }

    srcVal = sysEpicRegRead(srcAddr);
    
    switch (errCode)
        {
        case EPIC_EX_INTERRUPT:
            *pEnableMask  = (srcVal & EPIC_EIVPR_INTR_MSK)      >> 31;
            *pPolarity    = (srcVal & EPIC_EIVPR_INTR_POLARITY) >> 23;
            *pSense       = (srcVal & EPIC_EIVPR_INTR_SENSE)    >> 22;
            *pPriority    = (srcVal & EPIC_EIVPR_PRIORITY_MSK)  >> 16;
            *pVector      = (srcVal & EPIC_EIVPR_VECTOR_MSK);
        break;

        case EPIC_IN_INTERRUPT:
            *pEnableMask  = (srcVal & EPIC_IIVPR_INTR_MSK)      >> 31;
            *pPolarity    = (srcVal & EPIC_IIVPR_INTR_POLARITY) >> 23;
            *pPriority    = (srcVal & EPIC_IIVPR_PRIORITY_MSK)  >> 16;
            *pVector      = (srcVal & EPIC_IIVPR_VECTOR_MSK);
        break;

        case EPIC_GT_INTERRUPT:
            *pEnableMask  = (srcVal & EPIC_GTVPR_INTR_MSK)      >> 31;
            *pPriority    = (srcVal & EPIC_GTVPR_PRIORITY_MSK)  >> 16;
            *pVector      = (srcVal & EPIC_GTVPR_VECTOR_MSK);
        break;

        case EPIC_MSG_INTERRUPT:
            *pEnableMask  = (srcVal & EPIC_MIVPR_INTR_MSK)      >> 31;
            *pPriority    = (srcVal & EPIC_MIVPR_PRIORITY_MSK)  >> 16;
            *pVector      = (srcVal & EPIC_MIVPR_VECTOR_MSK);
        break;

        case EPIC_IPI_INTERRUPT:
            *pEnableMask  = (srcVal & EPIC_IPIVPR_INTR_MSK)     >> 31;
            *pPriority    = (srcVal & EPIC_IPIVPR_PRIORITY_MSK) >> 16;
            *pVector      = (srcVal & EPIC_IPIVPR_VECTOR_MSK);
        break;

        default:
            return (ERROR);
        }

    return (OK);
    }

/*******************************************************************************
*
* sysEpicRegRead - read a register from the EPIC address space
*
* This function reads a register from the EPIC address space. The
* register number <regNum> is added to the offset of the EPIC base address
*
* \NOMANUAL
*
* RETURNS:  the 32 bit little endian value of the register.
*
* ERRNO: N/A
*/

UINT32 sysEpicRegRead
    (
    ULONG regNum
    )
    {
    UINT32 temp;

    WRS_ASM("isync;sync");
    temp = *(UINT32 *) ((UINT32)CCSBAR + (UINT32)regNum) ;
    return (temp);
    }

/*******************************************************************************
*
* sysEpicRegWrite -  write a register to the EPIC address space
*
* This function writes a register to the EPIC address space. The
* register number <regNum> is added to the offset of the EPIC base address
* and the resulting address is loaded with <regVal>
*
* \NOMANUAL
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysEpicRegWrite
    (
    ULONG regNum,
    UINT32 regVal
    )
    {

    *(UINT32 *) (CCSBAR + regNum) = regVal;
    WRS_ASM("eieio;sync");
    return ;
    }


