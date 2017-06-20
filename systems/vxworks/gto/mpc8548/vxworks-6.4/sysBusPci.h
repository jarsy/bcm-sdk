/* sysBusPci.h - Platform-specific PCI support constants header file */

/* $Id: sysBusPci.h,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,04jun05,dtr    Created from cds85xx/sysBusPci.h/01b.
*/

#ifndef __INCsysBusPcih
#define __INCsysBusPcih

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ASMLANGUAGE

#if (defined(__STDC__) || defined(__cplusplus)) 

STATUS sysPci1AutoconfigInclude
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    );

STATUS sysPci2AutoconfigInclude
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    );

STATUS sysPci3AutoconfigInclude
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    );

UCHAR sysPci1AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UCHAR pin				/* contents of PCI int pin register */
    );

UCHAR sysPci2AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UCHAR pin				/* contents of PCI int pin register */
    );

UCHAR sysPci3AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UCHAR pin				/* contents of PCI int pin register */
    );

void sysPciAutoconfigPreEnumBridgeInit
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    );

void sysPciAutoconfigPostEnumBridgeInit
    (
    PCI_SYSTEM * pSys,			/* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,			/* pointer to function in question */
    UINT devVend			/* deviceID/vendorID of device */
    );


#else	/* __STDC__ */

STATUS sysPci1AutoconfigInclude ();
STATUS sysPci2AutoconfigInclude ();
STATUS sysPci3AutoconfigInclude ();
UCHAR  sysPci1AutoconfigIntrAssign ();
UCHAR  sysPci2AutoconfigIntrAssign ();
UCHAR  sysPci3AutoconfigIntrAssign ();
void   sysPciAutoconfigPreEnumBridgeInit ();
void   sysPciAutoconfigPostEnumBridgeInit  ();

#endif	/* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCsysBusPcih */
