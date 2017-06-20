/* pciAutoConfigLib.h - PCI bus automatic resource allocation facility */

/* Copyright 1997,1998,1999 Motorola, Inc. All Rights Reserved */
/* Copyright 1984-2001 Wind River Systems, Inc. */

/* $Id: pciAutoConfigLib.h,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01f,07may99,tm   Added pciAutoAddrAlign to perform alignment checking
                 pciAutoFuncDis-/En-able, pciAutoRegConfig public (SPR 26484)
01e,30mar99,dat  SPR 25312, changed class to pciClass
01d,29jul98,scb  added roll call capability
01c,29jul98,tm   code review changes / added _DEV_DISPLAY attribute
01b,24jul98,tm   added int routing offset to PCI_LOC structure 
                 added autoIntRouting boolean to PCI_SYSTEM structure
		 added protos for pciAutoBusNumberSet, pciAutoDevReset
01a,19feb98,dat  written, from Motorola.
*/

#ifndef __INCpciAutoConfigLibh
#define __INCpciAutoConfigLibh

#ifdef __cplusplus
extern "C" {
#endif

/* PCI_LOC attributes */

#define PCI_AUTO_ATTR_DEV_EXCLUDE	(UINT8)(0x01) /* Exclude this device */
#define PCI_AUTO_ATTR_DEV_DISPLAY	(UINT8)(0x02) /* (S)VGA disp device  */
#define PCI_AUTO_ATTR_DEV_PREFETCH	(UINT8)(0x04) /* Device requests PF  */

#define PCI_AUTO_ATTR_BUS_PREFETCH	(UINT8)(0x08) /* Bridge implements   */
#define PCI_AUTO_ATTR_BUS_PCI		(UINT8)(0x10) /* PCI-PCI Bridge      */
#define PCI_AUTO_ATTR_BUS_HOST		(UINT8)(0x20) /* PCI Host Bridge     */
#define PCI_AUTO_ATTR_BUS_ISA		(UINT8)(0x40) /* PCI-ISA Bridge      */
#define PCI_AUTO_ATTR_BUS_4GB_IO	(UINT8)(0x80) /* 4G/64K IO Adresses  */

#ifndef _ASMLANGUAGE

typedef struct /* PCI_LOC, a standard bus location */
    {
    UINT8 bus;
    UINT8 device;
    UINT8 function;
    UINT8 attribute;
    UINT8 offset;       /* interrupt routing for this device */
    } PCI_LOC;

/* PCI identification structure */

typedef struct
    {
    PCI_LOC loc;
    UINT devVend;
    } PCI_ID;

typedef struct pciSystem /* PCI_SYSTEM, auto configuration info */
    {
    UINT pciMem32;		/* 32 bit prefetchable memory location */
    UINT pciMem32Size;		/* 32 bit prefetchable memory size */
    UINT pciMemIo32;		/* 32 bit non-prefetchable memory location */
    UINT pciMemIo32Size;	/* 32 bit non-prefetchable memory size */
    UINT pciIo32;		/* 32 bit io location */
    UINT pciIo32Size;		/* 32 bit io size */
    UINT pciIo16;		/* 16 bit io location */
    UINT pciIo16Size;		/* 16 bit io size */
    int maxBus;			/* Highest subbus number */
    int cacheSize;		/* cache line size */
    UINT maxLatency;		/* max latency */
    BOOL autoIntRouting;        /* automatic routing strategy */
    STATUS (* includeRtn)	/* returns OK to include */
	   (
	   struct pciSystem * pSystem,
	   PCI_LOC * pLoc,
	   UINT devVend
	   );
    UCHAR  (* intAssignRtn)	/* returns int line, given int pin */
	   (
	   struct pciSystem * pSystem,
	   PCI_LOC * pLoc,
	   UCHAR pin
	   );
    void (* bridgePreConfigInit) /* bridge pre-enumeration initialization */
	   (
	   struct pciSystem * pSystem,
	   PCI_LOC * pLoc,
	   UINT devVend
	   );
    void (* bridgePostConfigInit)/* bridge post-enumeration initialization */
	   (
	   struct pciSystem * pSystem,
	   PCI_LOC * pLoc,
	   UINT devVend
	   );
    STATUS (* pciRollcallRtn) ();  /* Roll call check */
    STATUS (* includeSecondaryRtn) /* returns OK to include a secondary bus */
	   (
	   struct pciSystem * pSystem,
	   PCI_LOC * pLoc,
	   UINT devVend
	   );
    void (* bridgePreProbeInit) /* bridge pre-probe initialization */
	   (
	   struct pciSystem * pSystem,
	   PCI_LOC * pLoc,
	   UINT devVend
	   );
    void (* bridgePostProbeInit)/* bridge post-probe initialization */
	   (
	   struct pciSystem * pSystem,
	   PCI_LOC * pLoc,
	   UINT devVend
	   );
    } PCI_SYSTEM;


void pciAutoConfig (PCI_SYSTEM *);
STATUS pciAutoGetNextClass ( PCI_SYSTEM *pSystem, PCI_LOC *pciFunc,
                             UINT *index, UINT pciClass, UINT mask);
STATUS pciAutoBusNumberSet ( PCI_LOC * pPciLoc, UINT primary, UINT secondary,
			    UINT subordinate);
STATUS pciAutoDevReset ( PCI_LOC * pPciLoc);
STATUS pciAutoAddrAlign ( UINT32 base, UINT32 limit, UINT32 reqSize,
			  UINT32 *pAlignedBase );
void pciAutoFuncEnable ( PCI_SYSTEM * pSystem, PCI_LOC * pFunc);
void pciAutoFuncDisable ( PCI_LOC *pPciFunc);
UINT pciAutoRegConfig ( PCI_SYSTEM * pSystem, PCI_LOC *pPciFunc,
    		UINT baseAddr, UINT nSize, UINT addrInfo);

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCpciAutoConfigLibh */
