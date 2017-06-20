/* sysBusPci.c - PCI Autoconfig support */

/* Copyright (c) 1999,2001,2003,2005 Wind River Systems, Inc. All Rights Reserved */

/* $Id: sysBusPci.c,v 1.3 2011/07/21 16:14:25 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
DESCRIPTION

*/

/* includes */

#include "vxWorks.h"
#include "logLib.h"
#include "taskLib.h"
#include "config.h"

#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciAutoConfigLib.h"

#include <hndsoc.h>

/* defines */

/* typedefs */

/* globals */

PCI_SYSTEM sysParams0;
PCI_SYSTEM sysParams1;

#define UNUSED  0xff


static UCHAR intLine0 [][4] =
    {
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 0 */
    { IV_PCI0_0, IV_PCI0_1, IV_PCI0_2, IV_PCI0_MAX},  /* dev number 1 */
};

static UCHAR intLine1 [][4] =
    {
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 0 */
    { IV_PCI1_0, IV_PCI1_1, IV_PCI1_2, IV_PCI1_MAX},  /* dev number 1 */
};

/* locals */

/* forward declarations */

LOCAL UCHAR sysPci0AutoConfigIntAsgn ( PCI_SYSTEM * pSys, PCI_LOC * pFunc,
    UCHAR intPin );
LOCAL UCHAR sysPci1AutoConfigIntAsgn ( PCI_SYSTEM * pSys, PCI_LOC * pFunc,
    UCHAR intPin );
LOCAL STATUS sysPci0AutoConfigInclude ( PCI_SYSTEM *pSys, PCI_LOC *pciLoc,
    UINT devVend );
LOCAL STATUS sysPci1AutoConfigInclude ( PCI_SYSTEM *pSys, PCI_LOC *pciLoc,
    UINT devVend );


/* subroutines */

/******************************************************************************
*
* sysPciAutoConfigInclude - Determine if function is to be autoConfigured
*
* This function is called with PCI bus, device, function, and vendor 
* information.  It returns an indication of whether or not the particular
* function should be included in the automatic configuration process.
* This capability is useful if it is desired that a particular function
* NOT be automatically configured.  Of course, if the device is not
* included in automatic configuration, it will be unusable unless the
* user's code made provisions to configure the function outside of the
* the automatic process.
*
* RETURNS: TRUE if function is to be included in automatic configuration,
* FALSE otherwise.
*/
#if 1
uint8 pci_hbslot = 0;
#else
extern uint8 pci_hbslot;
#endif

LOCAL STATUS sysPci0AutoConfigInclude
    (
    PCI_SYSTEM *pSys,       /* input: AutoConfig system information */
    PCI_LOC *pciLoc,        /* input: PCI address of this function */
    UINT     devVend        /* input: Device/vendor ID number      */
    )
    {
    /* 
     * Excluded: 
     * Bus 0: backplane
     * Bus 1, Dev 0: Host Controller
     */ 
    if ((pciLoc->bus == 0) || 
    	((pciLoc->bus == 1) && (pciLoc->device == pci_hbslot)))  
        return(ERROR);

    return (OK);
    }

LOCAL STATUS sysPci1AutoConfigInclude
    (
    PCI_SYSTEM *pSys,       /* input: AutoConfig system information */
    PCI_LOC *pciLoc,        /* input: PCI address of this function */
    UINT     devVend        /* input: Device/vendor ID number      */
    )
    {
    /* 
     * Excluded: 
     * Bus = PCIE1_BUS_MIN(first bus number of the 2nd PCIe controller)
     * Dev 0: Host Controller
     */ 

    if ((pciLoc->bus == PCIE1_BUS_MIN) && (pciLoc->device == pci_hbslot))  
        return(ERROR);

    return (OK);
    }

/******************************************************************************
*
* sysPciAutoConfigIntAssign - Assign the "interrupt line" value
*
* RETURNS: "interrupt line" value.
*
*/

LOCAL UCHAR sysPci0AutoConfigIntAsgn
    ( 
    PCI_SYSTEM * pSys,      /* input: AutoConfig system information */
    PCI_LOC * pFunc,
    UCHAR intPin        /* input: interrupt pin number */
    )
    {
    UCHAR irqValue = 0xff;    /* Calculated value                */

    irqValue = intLine0 [(pFunc->device)][intPin - 1];

    /* TODO - add any non-standard interrupt assignments here */

    PCI_AUTO_DEBUG_MSG("intAssign called for device [%d %d %d] IRQ: %d\n",
        pFunc->bus, pFunc->device, pFunc->function,
        irqValue, 0, 0 );

    return (irqValue);
    }

LOCAL UCHAR sysPci1AutoConfigIntAsgn
    ( 
    PCI_SYSTEM * pSys,      /* input: AutoConfig system information */
    PCI_LOC * pFunc,
    UCHAR intPin        /* input: interrupt pin number */
    )
    {
    UCHAR irqValue = 0xff;    /* Calculated value                */

    irqValue = intLine1 [(pFunc->device)][intPin - 1];

    /* TODO - add any non-standard interrupt assignments here */

    PCI_AUTO_DEBUG_MSG("intAssign called for device [%d %d %d] IRQ: %d\n",
        pFunc->bus, pFunc->device, pFunc->function,
        irqValue, 0, 0 );

    return (irqValue);
    }

#ifndef INCLUDE_PCI_BUS 
/*******************************************************************************
*
* sysPciAutoConfig - PCI autoConfig support routine
*
* This routine instantiates the PCI_SYSTEM structure needed to configure
* the system. This consists of assigning address ranges to each category
* of PCI system resource: Prefetchable and Non-Prefetchable 32-bit Memory, and
* 16- and 32-bit I/O. Global values for the Cache Line Size and Maximum
* Latency are also specified. Finally, the four supplemental routines for 
* device inclusion/exclusion, interrupt assignment, and pre- and
* post-enumeration bridge initialization are specified. 
*
* RETURNS: N/A
*/

void sysPciAutoConfig (void)
    {
    sysPciConfigEnable(PCIE_CONTROLLER_0);


    /* 32-bit Non-prefetchable Memory Space */
    sysParams0.pciMemIo32 = SI_PCI0_MEM;
    sysParams0.pciMemIo32Size = SI_PCI_MEM_SZ;

 
    /* 32-bit PCI I/O Space: 32K at 32K offset */
    sysParams0.pciIo32 = 0;
    sysParams0.pciIo32Size = 0;


    /* Configuration space parameters */
 
    sysParams0.cacheSize = (_CACHE_ALIGN_SIZE/4);
    sysParams0.maxLatency = PCI_LAT_TIMER;
    sysParams0.autoIntRouting = TRUE;
    sysParams0.includeRtn = sysPci0AutoConfigInclude;
    sysParams0.intAssignRtn = sysPci0AutoConfigIntAsgn;
    sysParams0.bridgePreConfigInit = NULL;
    sysParams0.bridgePostConfigInit = NULL;
/*    sysParams.autoIntRouting = TRUE;*/
 
    /* Perform AutoConfig */
 
    pciAutoConfig (&sysParams0);

    sysPciConfigEnable(PCIE_CONTROLLER_1);
    /* 32-bit Non-prefetchable Memory Space */
    sysParams1.pciMemIo32 = SI_PCI1_MEM;
    sysParams1.pciMemIo32Size = SI_PCI_MEM_SZ;

 
    /* 32-bit PCI I/O Space: 32K at 32K offset */
    sysParams1.pciIo32 = 0;
    sysParams1.pciIo32Size = 0;


    /* Configuration space parameters */
 
    sysParams1.cacheSize = (_CACHE_ALIGN_SIZE/4);
    sysParams1.maxLatency = PCI_LAT_TIMER;
    sysParams1.autoIntRouting = TRUE;
    sysParams1.includeRtn = sysPci1AutoConfigInclude;
    sysParams1.intAssignRtn = sysPci1AutoConfigIntAsgn;
    sysParams1.bridgePreConfigInit = NULL;
    sysParams1.bridgePostConfigInit = NULL;
/*    sysParams.autoIntRouting = TRUE;*/
 
    /* Perform AutoConfig */
 
    pciAutoConfig (&sysParams1);

    return;
    }
#endif

