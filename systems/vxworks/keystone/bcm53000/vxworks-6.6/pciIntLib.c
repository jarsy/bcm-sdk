/* pciIntLib.c - PCI Shared Interrupt support */

/* Copyright 1984-1998 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: pciIntLib.c,v 1.3 2011/07/21 16:14:28 yshtil Exp $
modification history
--------------------
01c,30sep98, tm  moved PCI_INT_RTN typedef here from private header (SPR 22544)
01b,04mar98, tm  augmented pciIntConnect/Disconnect to do intConnect also
01a,20feb98, tm  derived from pciIomapLib.c of ebsa285 BSP v01m
*/


/*
DESCRIPTION

This library is PCI Revision 2.1 compliant.

The functions addressed here include:

.IP "   -"
Initialize the library.
.IP "   -"
Connect a shared interrupt handler.
.IP "   -"
Disconnect a shared interrupt handler.
.IP "   -"
Master shared interrupt handler.
.LP

Shared PCI interrupt are supported by three functions: pciInt(),
pciIntConnect(), pciIntDisconnect().  pciIntConnect() adds the specified
interrupt handler to the link list and pciIntDisconnect() removes it from
the link list.  Master interrupt handler pciInt() executes these interrupt
handlers in the link list for a PCI interrupt.  Each interrupt handler must
check the device dependent interrupt status bit to determine the source of
the interrupt, since it simply execute all interrupt handlers in the link
list.  pciInt() should be attached by intConnect() function in the BSP 
initialization with its parameter. The parameter is an IRQ associated 
to the PCI interrupt.
*/

#include "vxWorks.h"
#include "config.h"
#include "dllLib.h"
#include "sysLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "intLib.h"
#include "drv/pci/pciIntLib.h"

/*
 * The following defines specify, by default, the maximum number of busses,
 * devices and functions allowed by the PCI 2.1 Specification.
 *
 * Any or all may be overriden by defining them in config.h.
 */

/* macros */

#ifndef INT_NUM_IRQ0
#   define INT_NUM_IRQ0 0
#endif /* INT_NUM_IRQ0 */

/*
 * Provide intConnect via a macro so that an alternate interrupt binding
 * mechanism can be specified
 *
 */

#ifndef PCI_INT_HANDLER_BIND
#define PCI_INT_HANDLER_BIND(vector, routine, param, pResult)          \
    {                                                                  \
    IMPORT STATUS intConnect();                                        \
    *pResult = intConnect ( (vector),(routine), (int)(param) );        \
    }
    
#endif /* PCI_INT_HANDLER_BIND */

/* typedefs */

typedef struct pciIntRtn
    {
    DL_NODE	node;		/* double link list */
    VOIDFUNCPTR	routine;	/* interrupt handler */
    int		parameter;	/* parameter of the handler */
    } PCI_INT_RTN;

/* globals */

DL_LIST	pciIntList[PCI_IRQ_LINES];	/* linked list of int handlers */

/* locals */

LOCAL STATUS pciIntLibInitStatus = NONE;

/*******************************************************************************
*
* pciIntLibInit - 
*
*
*/

STATUS pciIntLibInit ( void )
    {
    int ix;

    if (pciIntLibInitStatus != NONE)
	return (pciIntLibInitStatus);

    /* Initialize shared interrupt handler chains */

    for (ix = 0; ix < PCI_IRQ_LINES; ix++)
        dllInit (&pciIntList[ix]);

    return(pciIntLibInitStatus = OK);
    }

/*******************************************************************************
*
* pciInt - interrupt handler for shared PCI interrupt.
*
* This routine executes multiple interrupt handlers for a PCI interrupt.
* Each interrupt handler must check the device dependent interrupt status bit
* to determine the source of the interrupt, since it simply execute all
* interrupt handlers in the link list.
*
* RETURNS: N/A
*
*/

VOID pciInt
    (
    int irq		/* IRQ associated to the PCI interrupt */
    )
    {
    PCI_INT_RTN *pRtn;

    for (pRtn = (PCI_INT_RTN *)DLL_FIRST (&pciIntList[irq]); pRtn != NULL;
	 pRtn = (PCI_INT_RTN *)DLL_NEXT (&pRtn->node))
	(* pRtn->routine) (pRtn->parameter);
    }

/*******************************************************************************
*
* pciIntConnect - connect the interrupt handler to the PCI interrupt.
*
* This routine connects an interrupt handler to the PCI interrupt line(A - D).
* Link list is created if multiple handlers are assigned to the single PCI
* interrupt.
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be built.
*
*/

STATUS pciIntConnect
    (
    VOIDFUNCPTR *vector,        /* interrupt vector to attach to     */
    VOIDFUNCPTR routine,        /* routine to be called              */
    int parameter               /* parameter to be passed to routine */
    )
    {
    int irq = (IVEC_TO_INUM(vector)) - INT_NUM_IRQ0;
    PCI_INT_RTN *pRtn;
    int oldLevel;
    STATUS retStatus;

    if (pciIntLibInitStatus != OK)
	return (ERROR);

    /* If pciIntList[irq] is null, then bind the handler using intConnect */

    if ( DLL_EMPTY(&pciIntList[irq]) )
        {
        PCI_INT_HANDLER_BIND(vector, pciInt, irq , &retStatus )
	if ( retStatus == ERROR )
	    {
            return( ERROR );
	    }
        }

    if ((pRtn = (PCI_INT_RTN *)malloc (sizeof (PCI_INT_RTN))) == NULL)
        {
	/* intDisconnect here ??????????? */
        return (ERROR);
        }

    pRtn->routine   = routine;
    pRtn->parameter = parameter;

    oldLevel = intLock ();			/* LOCK INTERRUPT */
    dllAdd (&pciIntList[irq], &pRtn->node);
    intUnlock (oldLevel);			/* UNLOCK INTERRUPT */

    retStatus = ( (DLL_EMPTY(&pciIntList[irq])) ? ERROR : OK );
    return (retStatus);
    }


/*******************************************************************************
*
* pciIntDisconnect - disconnect the interrupt handler from the PCI interrupt.
*
* This routine disconnects the interrupt handler from the PCI interrupt line.
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be removed.
*
*/

STATUS pciIntDisconnect
    (
    VOIDFUNCPTR *vector,        /* interrupt vector to attach to     */
    VOIDFUNCPTR routine         /* routine to be called              */
    )
    {
    int irq = IVEC_TO_INUM ((int)vector) - INT_NUM_IRQ0;
    PCI_INT_RTN *pRtn;
    int oldLevel;
    STATUS retStatus = ERROR;

    if (pciIntLibInitStatus != OK)
	return (ERROR);

    for (pRtn = (PCI_INT_RTN *)DLL_FIRST (&pciIntList[irq]); pRtn != NULL;
	 pRtn = (PCI_INT_RTN *)DLL_NEXT (&pRtn->node))
	{
	if (pRtn->routine == routine)
	    {
	    oldLevel = intLock ();			/* LOCK INTERRUPT */
	    dllRemove (&pciIntList[irq], &pRtn->node);
	    intUnlock (oldLevel);			/* UNLOCK INTERRUPT */
	    free ((char *)pRtn);

	    /* If the last ISR was just removed, then do intDisconnect */

            if ( DLL_EMPTY(&pciIntList[irq]) )
	        {

                /* Get the address of the synthesized handler and free the 
		 * block of allocated memory
		 */

		/* intDisconnect(); */
		 
	        }
	    retStatus = OK;
	    }
	}

    return (retStatus);
    }

