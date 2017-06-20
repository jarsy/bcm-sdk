/* pciSupport.c
   Originally taken from pciIomapLib.c - Support for PCI drivers */

/* Copyright 2000 Broadcom Corporation */
/* Copyright 1984-1996 Wind River Systems, Inc. */
/* Copyright 1998 Ziatech Corporation */

/* $Id: pciSupport.c,v 1.2 2011/07/21 16:14:08 yshtil Exp $
modification history
--------------------

01sep00,detBRCM  Removed almost everything expect pciIntConnect,
                 pciIntDisconnect and pciInt handler.
28aug98,ziatech  fixed bug in Mechanism 2 that would detect devices
                 twice.  Device numbers above 0x0f now return 0xff's.
                 New device and class scanning algorithm (for both
                 Mechanisms) -- code detects PCI bridges and reads
                 the subordinate bus number to avoid scanning non-existent
                 busses.
24jul98,ziatech  removed debugging code.
26may98,ziatech  fixed mechanism 2 functions. various bug fixes.
01g,12jan97,hdn  changed variable name "vender" to "vendor".
01f,12jan97,hdn  changed variable name "class" to "classCodeReg".
01e,03dec96,hdn  added single/multi function check.
01d,16sep96,dat  made pciConfigMech global (for pciIomapShow).
01c,06aug96,hdn  added pciInt(), pciIntConnect() and pciIntDisconnect().
01b,14mar96,hdn  re-written.  changed parameters of the functions.
                 removed BIOS dependent codes.
01a,25feb95,bcs  written
*/


#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "intLib.h"
#include "sysLib.h"
#include "semLib.h"
#include "iv.h"
#include "dllLib.h"
#include "bmw.h"

/* number of IRQs mapped on PCI interrupt */
#define NUM_IRQ 16

typedef struct pciIntRtn {
    DL_NODE node;       /* double link list */
    VOIDFUNCPTR routine;    /* interrupt handler */
    int     parameter;  /* parameter of the handler */
} PCI_INT_RTN;

/* globals */
DL_LIST pciIntList[NUM_IRQ];  /* link list of int handlers */


/***************************************************************************
 *
 * pciInt - interrupt handler for shared PCI interrupt.
 *
 * Parameters:
 * int irq         IRQ associated to the PCI interrupt 
 *
 * This routine executes multiple interrupt handlers for a PCI interrupt.
 * Each interrupt handler must check the device dependent interrupt status bit
 * to determine the source of the interrupt, since it simply execute all
 * interrupt handlers in the link list.
 *
 * RETURNS: N/A
 *
 */

VOID
pciInt (int irq)
{
    PCI_INT_RTN *pRtn;

    for (pRtn = (PCI_INT_RTN *)DLL_FIRST (&pciIntList[irq]); pRtn != NULL;
         pRtn = (PCI_INT_RTN *)DLL_NEXT (&pRtn->node)) {
        (* pRtn->routine) (pRtn->parameter);
    }
}

/**************************************************************************
 *
 * pciIntConnect - connect the interrupt handler to the PCI interrupt.
 *
 * Parameters:
 *    VOIDFUNCPTR *vector,       interrupt vector to attach to    
 *    VOIDFUNCPTR routine,       routine to be called             
 *    int parameter              parameter to be passed to routine
 *
 * This routine connects an interrupt handler to the PCI interrupt line(A - D).
 * Link list is created if multiple handlers are assigned to the single PCI
 * interrupt.
 *
 * RETURNS:
 * OK, or ERROR if the interrupt handler cannot be built.
 *
 */

STATUS
pciIntConnect (VOIDFUNCPTR *vector, VOIDFUNCPTR routine, int parameter)
{
    static int alreadyConnected=FALSE;

    int irq = IVEC_TO_INUM ((int)vector) - sysVectorIRQ0;
    PCI_INT_RTN *pRtn;
    int oldLevel;

    pRtn = (PCI_INT_RTN *)malloc (sizeof (PCI_INT_RTN));
    if (pRtn == NULL) {
        return (ERROR);
    }

    pRtn->routine   = routine;
    pRtn->parameter = parameter;

    oldLevel = intLock ();                      /* LOCK INTERRUPT */
    dllAdd (&pciIntList[irq], &pRtn->node);
    intUnlock (oldLevel);                       /* UNLOCK INTERRUPT */

    if (!alreadyConnected) {
        if (intConnect(vector, (VOIDFUNCPTR) pciInt, irq) == ERROR) {
            return -1;
        }
        alreadyConnected=TRUE;
    }
    return (OK);
}


/**************************************************************************
 *
 * pciIntDisconnect - disconnect the interrupt handler from the PCI interrupt.
 *
 * Parameters:
 * VOIDFUNCPTR *vector,        interrupt vector to attach to
 * VOIDFUNCPTR routine         routine to be called             
 * This routine disconnects the interrupt handler from the PCI interrupt line.
 *
 * RETURNS:
 * OK, or ERROR if the interrupt handler cannot be removed.
 *
 */

STATUS
pciIntDisconnect
    (
    VOIDFUNCPTR *vector,        /* interrupt vector to attach to     */
    VOIDFUNCPTR routine         /* routine to be called              */
    )
{
    int irq = IVEC_TO_INUM ((int)vector) - INT_NUM_IRQ0;
    PCI_INT_RTN *pRtn;
    int oldLevel;

    for (pRtn = (PCI_INT_RTN *)DLL_FIRST (&pciIntList[irq]); pRtn != NULL;
         pRtn = (PCI_INT_RTN *)DLL_NEXT (&pRtn->node)) {
        if (pRtn->routine == routine) {
            oldLevel = intLock ();                      /* LOCK INTERRUPT */
            dllRemove (&pciIntList[irq], &pRtn->node);
            intUnlock (oldLevel);                       /* UNLOCK INTERRUPT */

            free ((char *)pRtn);
            return (OK);
        }
    }

    return (ERROR);
}

