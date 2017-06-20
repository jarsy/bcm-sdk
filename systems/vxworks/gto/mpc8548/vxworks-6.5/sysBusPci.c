/* sysBusPci.c - Wind River SBC8548 platform-specific PCI bus support */

/* $Id: sysBusPci.c,v 1.3 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,03apr07,b_m  modify PCIe interrupt route.
01a,30jan06,kds  Modify from cds8548/sysBusPci.c/01a
*/

/*
DESCRIPTION
This is the  platform specific pciAutoConfigLib information.

INCLUDE FILES:
*/


/* includes */

#include <vxWorks.h>
#include <config.h>
#include <sysLib.h>

#include <drv/pci/pciConfigLib.h> 	
#include <drv/pci/pciIntLib.h> 	
#include <drv/pci/pciAutoConfigLib.h>	
#include <sysBusPci.h>

/* static file scope locals */

IMPORT void sysPciOutLong(UINT32*,UINT32);
IMPORT UINT32 sysPciInLong (UINT32*); 

LOCAL PCI_SYSTEM sysParams ;

#ifdef INCLUDE_CDS85XX_PCIEX
LOCAL PCI_SYSTEM sysParams3;
#endif /* INCLUDE_CDS85XX_PCIEX */

/* INT LINE TO IRQ assignment for SBC8548 board. */

LOCAL UCHAR sysPci1IntRoute [NUM_PCI1_SLOTS][4] =
    {{PCI_XINT1_LVL, PCI_XINT2_LVL, PCI_XINT3_LVL, PCI_XINT4_LVL}};

LOCAL UCHAR sysPci2IntRoute [4] = 
    {PCI2_XINT1_LVL, PCI_XINT2_LVL, PCI_XINT3_LVL, PCI_XINT4_LVL};

LOCAL UCHAR sysPci3IntRoute [NUM_PCIEX_SLOTS][4] =
    {{PCIEX_XINT1_LVL, PCIEX_XINT2_LVL, PCIEX_XINT3_LVL, PCIEX_XINT4_LVL}};

/*******************************************************************************
*
* sysPciAutoConfig - PCI autoconfig support routine
*
* This routine performs the PCI auto-configuration support function.
*
* RETURNS: N/A
*/

void sysPciAutoConfig (void)
    {
    void * pCookie;

    /* PCI 1 Auto configuration */

    sysPciConfigEnable (CDS85XX_PCI_1_BUS);

    /* 32-bit Prefetchable Memory Space */

    sysParams.pciMem32 	        = PCI_MEM_ADRS;
    sysParams.pciMem32Size 	= PCI_MEM_SIZE;

    /* 32-bit Non-prefetchable Memory Space */

    sysParams.pciMemIo32 	= PCI_MEMIO_ADRS;
    sysParams.pciMemIo32Size 	= PCI_MEMIO_SIZE;

    /* 32-bit PCI I/O Space */

    sysParams.pciIo32 		= PCI_IO_ADRS;
    sysParams.pciIo32Size 	= PCI_IO_SIZE;

    /* Configuration space parameters */

    sysParams.maxBus 		= 0;
    sysParams.cacheSize 	= ( _CACHE_ALIGN_SIZE / 4 );
    sysParams.maxLatency 	= PCI_LAT_TIMER;

    /*
     * Interrupt routing strategy
     * across PCI-to-PCI Bridges
     */

    sysParams.autoIntRouting 	= TRUE;

    /* Device inclusion and interrupt routing routines */

    sysParams.includeRtn 	= sysPci1AutoconfigInclude;
    sysParams.intAssignRtn 	= sysPci1AutoconfigIntrAssign;

    /*
     * PCI-to-PCI Bridge Pre-
     * and Post-enumeration init
     * routines
     */

    sysParams.bridgePreConfigInit = NULL;
			/* sysPciAutoconfigPreEnumBridgeInit; */
    sysParams.bridgePostConfigInit = NULL;
			/* sysPciAutoconfigPostEnumBridgeInit; */
    /*
     * Perform any needed PCI Host Bridge
     * Initialization that needs to be done
     * before pciAutoConfig is invoked here 
     * utilizing the information in the 
     * newly-populated sysParams structure. 
     */
 
    pCookie = pciAutoConfigLibInit (NULL);

    pciAutoCfgCtl (pCookie, PCI_PSYSTEM_STRUCT_COPY, &sysParams);
    pciAutoCfg (pCookie);

    /*
     * Perform any needed post-enumeration
     * PCI Host Bridge Initialization here
     * utilizing the information in the 
     * sysParams structure that has been 
     * updated as a result of the scan 
     * and configuration passes. 
     */

#ifdef INCLUDE_CDS85XX_PCIEX
    /* PCI 2 Auto configuration */

    sysPciConfigEnable (CDS85XX_PCIEX_BUS);

    /* 32-bit Prefetchable Memory Space */

    sysParams3.pciMem32 	= PCI_MEM_ADRS3;
    sysParams3.pciMem32Size 	= PCI_MEM_SIZE;

    /* 32-bit Non-prefetchable Memory Space */

    sysParams3.pciMemIo32 	= PCI_MEMIO_ADRS3;
    sysParams3.pciMemIo32Size 	= PCI_MEMIO_SIZE;

    /* 32-bit PCI I/O Space */

    sysParams3.pciIo32 		= PCI_IO_ADRS3;
    sysParams3.pciIo32Size 	= PCI_IO_SIZE;

    /* Configuration space parameters */

    sysParams3.maxBus 		= 0;
    sysParams3.cacheSize 	= ( _CACHE_ALIGN_SIZE / 4 );
    sysParams3.maxLatency 	= PCI_LAT_TIMER;

    /*
     * Interrupt routing strategy
     * across PCI-to-PCI Bridges
     */

    sysParams3.autoIntRouting 	= TRUE;

    /* Device inclusion and interrupt routing routines */

    sysParams3.includeRtn 	= sysPci3AutoconfigInclude;
    sysParams3.intAssignRtn 	= sysPci3AutoconfigIntrAssign;

    /*
     * PCI-to-PCI Bridge Pre-
     * and Post-enumeration init
     * routines
     */

    sysParams3.bridgePreConfigInit = NULL;
			/* sysPciAutoconfigPreEnumBridgeInit; */
    sysParams3.bridgePostConfigInit = NULL;
			/* sysPciAutoconfigPostEnumBridgeInit; */
    /*
     * Perform any needed PCI Host Bridge
     * Initialization that needs to be done
     * before pciAutoConfig is invoked here 
     * utilizing the information in the 
     * newly-populated sysParams structure. 
     */

    pCookie = pciAutoConfigLibInit (NULL);
    pciAutoCfgCtl (pCookie, PCI_PSYSTEM_STRUCT_COPY, &sysParams3);
    pciAutoCfg (pCookie);


#endif /* INCLUDE_CDS85XX_PCIEX */

    }

/*******************************************************************************
*
* sysPci1AutoconfigInclude - PCI 1 autoconfig support routine
*
* This routine performs the PCI 1 auto configuration support function. 
*
* RETURNS: OK or ERROR
*/

STATUS sysPci1AutoconfigInclude
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    )
    {
    
    /* 
     * Only support BUS 0 .. PCI_MAX_BUS -1;
     * Host controller itself (device number is 0) won't be configured;
     * Bridge on the Arcadia board (device number 17) won't be configured;
     */ 

    if ((pLoc->bus > PCI_MAX_BUS)                                     ||
        (pLoc->bus == 0 && pLoc->device == 0 && pLoc->function == 0)  ||
        (pLoc->bus == 0 && pLoc->device == 18 && pLoc->function == 0) ||
        (devVend == PCI_ARCADIA_BRIDGE_DEV_ID)) 

        return(ERROR);

    
    return (OK); /* Autoconfigure all devices */
    }

/*******************************************************************************
*
* sysPci2AutoconfigInclude - PCI 2 autoconfig support routine
*
* This routine peforms the PCI 2 auto configuration support function.
*
* RETURNS: OK or ERROR
*/

STATUS sysPci2AutoconfigInclude
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    )
    {
    
    /* 
     * Only support BUS 0;
     * Host controller itself (device number is 0) won't be configured;
     */ 

    if ((pLoc->bus > 0) ||
        (pLoc->bus == 0 && pLoc->device == 0 && pLoc->function == 0))  
        return(ERROR);

    return OK; /* Autoconfigure all devices */
    }

/*******************************************************************************
*
* sysPci3AutoconfigInclude - PCI Express autoconfig support routine
*
* This routine performs the PCI Express auto configuration support function.
*
* RETURNS: OK or ERROR
*/

STATUS sysPci3AutoconfigInclude
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    )
    {
    
    /* 
     * Only support BUS 0;
     * Host controller itself (device number is 0) won't be configured;
     */ 

    if ((pLoc->bus > 2) ||
        (pLoc->bus == 0 && pLoc->device == 0 && pLoc->function == 0) ||
	(pLoc->device > 0))  
        return(ERROR);

    return OK; /* Autoconfigure all devices */
    }

/*******************************************************************************
*
* sysPci1AutoconfigIntrAssign - PCI 1 autoconfig support routine
*
* This routine peforms the PCI 1 auto configuration interrupt assignment
* support function.
*
* RETURNS: PCI interrupt line number given pin mask
*/

UCHAR sysPci1AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UCHAR pin				/* contents of PCI int pin register */
    )
    {
    UCHAR tmpChar = 0xff;

    /* 
     * Ensure this is a reasonable value for bus zero.
     * If OK, return INT level, else we return 0xff.
     */
    if (((pin > 0) && (pin < 5))       				&& 
	(((pLoc->device) - sysPciSlotDeviceNumber) < NUM_PCI1_SLOTS) 	&&
	(((pLoc->device) - sysPciSlotDeviceNumber) >= 0))
	{
	tmpChar = 
	    sysPci1IntRoute [((pLoc->device) - sysPciSlotDeviceNumber)][(pin-1)];
	}

    /* return the value to be assigned to the pin */

    return (tmpChar);
    }

/*******************************************************************************
*
* sysPci2AutoconfigIntrAssign - PCI 2 autoconfig support routine
*
* This routine peforms the PCI 2 auto configuration interrupt assignment
* support function.
*
* RETURNS: PCI interrupt line number given pin mask
*/

UCHAR sysPci2AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UCHAR pin				/* contents of PCI int pin register */
    )
    {
    UCHAR tmpChar = 0xff;

    /* 
     * Ensure this is a reasonable value for bus zero.
     * If OK, return INT level, else we return 0xff.
     */
    if ((pin > 0) && (pin < 5))	
	tmpChar = sysPci2IntRoute [(pin-1)];

    /* return the value to be assigned to the pin */

    return (tmpChar);
    }

/*******************************************************************************
*
* sysPci3AutoconfigIntrAssign - PCI Express autoconfig support routine
*
* This routine peforms the PCI Express auto configuration interrupt assignment
* support function.
*
* RETURNS: PCI interrupt line number given pin mask
*/

UCHAR sysPci3AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UCHAR pin				/* contents of PCI int pin register */
    )
    {
    UCHAR tmpChar = 0xff;

    /* 
     * Ensure this is a reasonable value for bus zero.
     * If OK, return INT level, else we return 0xff.
     */
    if (((pin > 0) && (pin < 5))       				&& 
	(((pLoc->device) - sysPciSlotDeviceNumber) < NUM_PCIEX_SLOTS) 	&&
	(((pLoc->device) - sysPciSlotDeviceNumber) >= 0))
	{
	tmpChar = 
	    sysPci3IntRoute [((pLoc->device) - sysPciSlotDeviceNumber)][(pin-1)];
	}

    /* return the value to be assigned to the pin */

    return (tmpChar);
    }

/*******************************************************************************
*
* sysPciAutoconfigPreEnumBridgeInit - PCI autoconfig support routine
*
* This routine performs the PCI bridge pre-initilaization and enumeration 
* support function.
*
* RETURNS: N/A
*/


void sysPciAutoconfigPreEnumBridgeInit
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    )
    {
    return;
    }


/*******************************************************************************
*
* sysPciAutoconfigPostEnumBridgeInit - PCI autoconfig support routine
*
* This routine performs the PCI bridge post-initialization and enumeration
* support function.
*
* RETURNS: N/A
*/

void sysPciAutoconfigPostEnumBridgeInit
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    )
    {
    return;
    }








