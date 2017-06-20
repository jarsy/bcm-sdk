/* bcm1250PciLib.c - BCM12500 PCI/LDT configuration and access support */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* $Id: bcm1250PciLib.c,v 1.4 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,11dec01,agf  coding standard updates
01a,15nov01, agf  written
*/

/*
DESCRIPTION

This is the board-specific component of the BCM1250 PCI/LDT configuration
code.  Most of the work of autoconfiguration is performed by the generic
pciAutoConfigLib, which in turn uses pciConfigLib.

The code here provides four basic functions.

  It initializes the PCI and LDT host bridges prior to configuration.
  It provides functions to access the configuration registers as implemented
    in the BCM1250.
  It supplies callback hooks for the configuration algorithm itself,
    to exclude devices/buses and to perform any special pre- and
    post-processing required by a device as part of configuration..
  It describes how the board maps bus 0 device interrupts to BCM1250
    interrupt sources.

The bulk of the code is associated with the hook that initializes an
LDT chain and assigns UnitIDs prior to probing or enumeration of that
chain. Thereafter, the chain can treated as if it were an ordinary PCI bus.

USAGE

The public entry points provide the top level control of configuration
as documented in pciAutoConfigLib.

INCLUDE FILES: bcm1250PciLib.h
*/


/* includes */

#include "vxWorks.h"
#include "config.h"

#if defined(INCLUDE_PCI)

#include "drv/pci/pciConfigLib.h"
#include "pciAutoConfigLib.h"

typedef UINT64 uint64_t;

#include "bcm1250Lib.h"

/* defines */

/* Hyperspace addresses */

typedef UINT64 hsaddr_t;        /* 64-bit pointer type */

/* Configuration register access */

typedef UINT32 pcireg_t;        /* configuration register values */
typedef UINT32 pcitag_t;        /* encoded (bus, device, function) triples */

#define BCM1250_PCI_MAKE_TAG(b,d,f)                                     \
    (((b) << 16) | ((d) << 11) | ((f) << 8))
#define BCM1250_CFG_BASE                                                \
    MIPS_PHYS_TO_XKSEG_UNCACHED(PCI_CONF_ADRS + BIT_ENDIAN_OFFSET)

#define BCM1250_PCI_INT_BASE  56
#define BCM1250_PCI_LDT_INT_MAP(x) (x)

#if _BYTE_ORDER == _BIG_ENDIAN
/* This is for big-endian with a  match bits policy. */
#define BCM1250_CFG_ADDR(t, o, w)                                       \
    ((hsaddr_t)((BCM1250_CFG_BASE + (t) + (o)) ^ (4 - (w))))
#else /*_BYTE_ORDER == _LITTLE_ENDIAN*/
/* This is for little-endian, either policy. */
#define BCM1250_CFG_ADDR(t, o, w)                                       \
    ((hsaddr_t)(BCM1250_CFG_BASE + (t) + (o)))
#endif

#define SIBYTE_DEVICE(dev) ((dev) << 16 | 0x166D)
#define BCM1250_PCI_HOST_BRIDGE 0x0001
#define BCM1250_LDT_HOST_BRIDGE 0x0002

#define API_DEVICE(dev) ((dev) << 16 | 0x14D9)
#define API_STURGEON 0x0010

/* Sturgeon (LPB) specific registers */
#define LPB_CFG_READ                    0x60

#define LPB_CFG_INT_CTRL_BASE           0xA0

#define LPB_INT_CTRL_ENABLE             (1 << 15)
#define LPB_INT_CTRL_DESTMODE           (1 << 14)
#define LPB_INT_CTRL_DEST_SHIFT         6
#define LPB_INT_CTRL_DEST_MASK          (0xFF << LPB_INT_CTRL_DEST_SHIFT)
#define LPB_INT_CTRL_MSGTYPE_SHIFT      4
#define LPB_INT_CTRL_MSGTYPE_MASK       (0x3 << LPB_INT_CTRL_MSGTYPE_SHIFT)
#define LPB_INT_CTRL_POLARITY           (1 << 3)
#define LPB_INT_CTRL_TRIGGERMODE        (1 << 2)
#define LPB_INT_CTRL_VECTOR_SHIFT       0
#define LPB_INT_CTRL_VECTOR_MASK        (0x3 << 0)

#define LPB_CFG_INT_BLOCK1              0xC4

/* Host bridge (bus, dev, func) tuples */
#define BCM1250_PCI_BRIDGE  0, 0, 0
#define BCM1250_LDT_BRIDGE  0, 1, 0

/* PCI host bridge (PHB) extensions */

#define PHB_CFG_MAP_BASE         0x44
#define PHB_CFG_MAP_N_ENTRIES    16
#define PHB_CFG_SUBSYSSET        0x8C
#define PHB_CFG_READHOST         0x90

/* This macro is similar to ones in pciLocalBus.h */
#define PCI_REVISION(x)      ((x) & 0xFF)

/* LDT capability bits */

#define LDT_CMD_WARM_RESET      0x00010000

#define LDT_CTRL_LINK_FAIL      0x0010
#define LDT_CTRL_INIT_DONE      0x0020
#define LDT_CTRL_EOC            0x0040
#define LDT_CTRL_XMIT_OFF       0x0080


/* LDT host bridge (LHB) register extensions */

#define LHB_CFG_LINK_CMD        0x40
#define LHB_CFG_LINK_CTRL       0x44
#define LHB_CFG_LDT_FREQ        0x49

#define LHB_CFG_SRI_TXDEN       0x50
#define LHB_CFG_SRI_RXDEN       0x51
#define LHB_CFG_SRI_CMD         0x52
#define LHB_CFG_SRI_TXNUM       0x54
#define LHB_CFG_SRI_RXNUM       0x58
#define LHB_CFG_ERR_CTRL        0x68
#define LHB_CFG_SRI_CTRL        0x6c
#define LHB_CFG_TX_BUFCNT       0xc8

/* LHB Sri command fields */
#define LHB_SRI_CMD_SIPREADY        0x0001
#define LHB_SRI_CMD_SYNCPTRCTRL     0x0002
#define LHB_SRI_CMD_REDUCESYNCZERO  0x0004
#define LHB_SRI_CMD_DISSTARVECNT    0x0008
#define LHB_SRI_CMD_RXMARGIN_SHIFT  4
#define LHB_SRI_CMD_PLLCOMPAT       0x0200
#define LHB_SRI_CMD_TXOFFSET_SHIFT  12
#define LHB_SRI_CMD_LINKFREQDIRECT  0x8000

#define LHB_ERR_CTRL_STATUS     0xff000000

#define PCI_CAPLISTPTR_REG       PCI_CFG_CAP_PTR

#define PCI_CAPLIST_PTR(cpr) ((cpr) & 0xff)
#define PCI_CAPLIST_NEXT(cr) (((cr) >> 8) & 0xff)
#define PCI_CAPLIST_CAP(cr)  ((cr) & 0xff)
#define PCI_CAP_LDT          8

#define PPB_BUS_REG              0x18
#define PPB_BRCTL_REG            0x3c
#define PPB_BRCTL_S_RESET        0x00400000

/* LDT utility functions, mostly for capabilities. */

#define LDT_PRIMARY    0
#define LDT_SECONDARY  1

#define LDTCAP_CMD_OFF    0

#define LDTCAP_CMD_TYPE_MASK                    0xe0000000
#define LDTCAP_CMD_TYPE_SHIFT                   (16+13)
#define LDTCAP_CMD_TYPE(cmd) \
            (((cmd) & LDTCAP_CMD_TYPE_MASK) >> LDTCAP_CMD_TYPE_SHIFT)
#define LDTCAP_CMD_TYPE_SLAVE                   0x0
#define LDTCAP_CMD_TYPE_HOST                    0x1

/* Slave/Pri commands */
#define LDTCAP_CMD_BASE_ID_MASK                 0x001f0000
#define LDTCAP_CMD_BASE_ID_SHIFT                (16+0)
#define LDTCAP_CMD_UNIT_CNT_MASK                0x03e00000
#define LDTCAP_CMD_UNIT_CNT_SHIFT               (16+5)
#define LDTCAP_CMD_UNIT_CNT(cmd) \
            (((cmd) & LDTCAP_CMD_UNIT_CNT_MASK) >> LDTCAP_CMD_UNIT_CNT_SHIFT)
#define LDTCAP_CMD_MASTER_HOST_MASK             0x04000000
#define LDTCAP_CMD_MASTER_HOST_SHIFT            (16+10)
#define LDTCAP_CMD_DEFAULT_DIR                  0x08000000

#define LDTCAP_LINK0_OFF  4

#define LDTCAP_LINK_CTRL_LKFAIL                 LDT_CTRL_LINK_FAIL
#define LDTCAP_LINK_CTRL_INIT                   LDT_CTRL_INIT_DONE
#define LDTCAP_LINK_CTRL_EOC                    LDT_CTRL_EOC
#define LDTCAP_LINK_CTRL_TXO                    LDT_CTRL_XMIT_OFF

/* Host/Sec commands */
#define LDTCAP_CMD_WARM_RESET                   0x00010000
#define LDTCAP_CMD_DBL_ENDED                    0x00020000
#define LDTCAP_CMD_DEV_ID_MASK                  0x007c0000
#define LDTCAP_CMD_DEV_ID_SHIFT                 2

#define PCI_BUSMAX      255
#define PCI_DEVMAX      31
#define PCI_FUNCMAX     7
#define PCI_REGMAX      255

#define pci_make_tag BCM1250_PCI_MAKE_TAG


/* globals */

/* locals */

LOCAL PCI_SYSTEM sysParams;

LOCAL int pciDeviceMode;

/* forward static declarations */

LOCAL STATUS pciRead (int bus, int dev, int func,
		      int reg, int size, void *pResult);
LOCAL STATUS pciWrite (int bus, int dev, int func,
		       int reg, int size, UINT32 data);

LOCAL void sysPciAutoconfigPreProbeBridgeInit (PCI_SYSTEM *pSys, PCI_LOC *pLoc,
					      UINT devVend);
LOCAL void sysPciAutoconfigPostProbeBridgeInit (PCI_SYSTEM *pSys, PCI_LOC *pLoc,
					       UINT devVend);
LOCAL STATUS sysPciAutoconfigInclude (PCI_SYSTEM *pSys, PCI_LOC *pLoc,
				      UINT devVend);
LOCAL STATUS sysPciAutoconfigIncludeSecondary (PCI_SYSTEM *pSys, PCI_LOC *pLoc,
					       UINT devVend);
LOCAL void sysPciAutoconfigPreEnumBridgeInit (PCI_SYSTEM *pSys, PCI_LOC *pLoc,
					      UINT devVend);
LOCAL void sysPciAutoconfigPostEnumBridgeInit (PCI_SYSTEM *pSys, PCI_LOC *pLoc,
					       UINT devVend);
LOCAL UCHAR sysPciAutoconfigIntrAssign (PCI_SYSTEM *pSys, PCI_LOC *pLoc,
					UCHAR pin);

LOCAL void sysPciHwInit (void);

LOCAL void ldtSecondaryInit (int bus, int device, int func, int bus2);

/* Exported functions */

/******************************************************************************
*
* sysPciConfig - Initialize the PCI configuration library.
*
* This routine selects PCI access mechanism 0 and supplies the read and
* write functions for use by the generic PCI configuration library.
*
* RETURNS: N/A
*/

void sysPciConfig (void)
    {
    /* Initialize the basic PCI access routines. */

    pciConfigLibInit (PCI_MECHANISM_0,
		      (ULONG) pciRead,
		      (ULONG) pciWrite,
		      (ULONG) NULL);       /* mySpecialRtn (not used) */
    }

/******************************************************************************
*
* sysHostBridgeInit - Initialize the host bridge(s) prior to configuration.
*
* This routine initializes the PCI and LDT host bridges in preparation
* for the standard (auto)configuration code.  It is a wrapper.
*
* RETURNS: N/A
*/

void sysHostBridgeInit (void)
    {
    /* Do PCI and LDT host bridge initialization */

    sysPciHwInit ();
    }

/******************************************************************************
*
* sysPciAutoConfig - Initialize and invoke PCI/LDT autoconfiguration.
*
* This routine fills in system parameters describing the BCM1250 PCI/LDT
* address space, attaches the customizing callback hooks, and invokes
* the generic autoconfiguration code.
*
* RETURNS: N/A
*/

void sysPciAutoConfig (void)
    {

    /* 32-bit Prefetchable Memory Space */
    /* Note: not currently supported, non-prefetchable memory space will
       be allocated instead.  The LDT host bridge does not support a
       prefetchable region in any case. */

    sysParams.pciMem32 = PCI_MEM_ADRS;
    sysParams.pciMem32Size = 0;

    /* 32-bit Non-prefetchable Memory Space */

    sysParams.pciMemIo32 = PCI_MEM_ADRS;
    sysParams.pciMemIo32Size = PCI_MEM_SIZE;

    /* 16-bit ISA I/O Space (exclude high-order bits) */

    sysParams.pciIo16 = (PCI_IO_ADRS + PCI_ISA_IO_SIZE) - PCI_IO_ADRS;
    sysParams.pciIo16Size = 0x10000 - PCI_ISA_IO_SIZE;

    /* 32-bit PCI I/O Space */

    sysParams.pciIo32 = (PCI_IO_ADRS + 0x10000) - PCI_IO_ADRS;
    sysParams.pciIo32Size = PCI_IO_SIZE - 0x10000;

    /* Configuration space parameters */

    sysParams.maxBus = 0;
    sysParams.cacheSize = ( _CACHE_ALIGN_SIZE / 4 );
    sysParams.maxLatency = PCI_LAT_TIMER;
    sysParams.includeRtn = sysPciAutoconfigInclude;
    sysParams.includeSecondaryRtn = sysPciAutoconfigIncludeSecondary;
    sysParams.bridgePreConfigInit = sysPciAutoconfigPreEnumBridgeInit;
    sysParams.bridgePostConfigInit = sysPciAutoconfigPostEnumBridgeInit;
    sysParams.bridgePreProbeInit = sysPciAutoconfigPreProbeBridgeInit;
    sysParams.bridgePostProbeInit = sysPciAutoconfigPostProbeBridgeInit;

    sysParams.autoIntRouting = TRUE;
    sysParams.intAssignRtn = sysPciAutoconfigIntrAssign;

    /*
     * Perform AutoConfig.
     * sysPciConfig and sysHostBridgeInit must precede this step.
     */

    pciAutoConfig (&sysParams);
    }


/* Functions that specialize pciConfigLib */

/******************************************************************************
*
* pciRead - Read a PCI configuration register.
*
* This routine reads selected bytes from the PCI configuration register
* identified by a (bus, device, function) tuple and register offset.
*
* Note that reads of 8 or 16 bits are done directly, as supported by
* the BCM1250, not by reading 32 bits followed by a shift/mask.
*
* RETURNS: Always returns OK
*/

LOCAL STATUS pciRead
    (
    int bus,
    int dev,
    int func,
    int reg,
    int size,
    void *pResult
    )
    {
    pcitag_t tag = BCM1250_PCI_MAKE_TAG (bus, dev, func);
    hsaddr_t addr = BCM1250_CFG_ADDR (tag, reg, size);

    switch (size)
        {
        case 1:
            *(UINT8  *) pResult = (pcireg_t) hs_read8  (addr);
            break;
        case 2:
            *(UINT16 *) pResult = (pcireg_t) hs_read16 (addr);
            break;
        case 4:
        default:
            *(UINT32 *) pResult = (pcireg_t) hs_read32 (addr);
            break;
        }
    return OK;
    }

/******************************************************************************
*
* pciWrite - Write a PCI configuration register.
*
* This routine writes selected bytes of a PCI configuration register
* identified by a (bus, device, function) tuple and register offset.
*
* Note that writes of 8 or 16 bits are done directly using byte enables,
* as supported by the BCM1250, not by read/modify/write of 32 bits.
*
* RETURNS: Always returns OK
*/

LOCAL STATUS pciWrite
    (
    int bus,
    int dev,
    int func,
    int reg,
    int size,
    UINT32 data
    )
    {
    pcitag_t tag = BCM1250_PCI_MAKE_TAG (bus, dev, func);
    hsaddr_t addr = BCM1250_CFG_ADDR (tag, reg, size);

    switch (size)
        {
        case 1:
            hs_write8 (addr, (UINT8) data);
            break;
        case 2:
            hs_write16 (addr, (UINT16) data);
            break;
        case 4:
        default:
            hs_write32 (addr, data);
            break;
        }
    return OK;
    }


/* Functions that specialize pciAutoConfigLib. */

/******************************************************************************
*
* sysPciAutoconfigInclude - Check if a PCI device is to be autoconfigured.
*
* This routine is called by the probe logic and returns OK if the
* device is to be included in the autoconfiguration.
*
* RETURNS: ERROR for the PCI host bridge, otherwise OK.
*/

LOCAL STATUS sysPciAutoconfigInclude
    (
     PCI_SYSTEM *pSys,		/* PCI_SYSTEM structure pointer */
     PCI_LOC *pLoc,		/* pointer to function in question */
     UINT devVend		/* deviceID/vendorID of device */
     )
    {
    /* Never configure the PCI host bridge. */
    if (devVend == SIBYTE_DEVICE(BCM1250_PCI_HOST_BRIDGE))
        return ERROR;

    /* In PCI Device Mode, skip all other bus 0 devices except
       the LDT host bridge, which the hardware defines to be device 1. */
    if (pciDeviceMode && (pLoc->bus == 0 && pLoc->device != 1))
        return ERROR;

    /* In a double-hosted system, never configure the LDT host bridge
        at the far end of the chain, which will have its DBL_ENDED bit
        set by sizing.. */
     if (devVend == SIBYTE_DEVICE(BCM1250_LDT_HOST_BRIDGE))
       {
       UINT32 cpr, lr;
       UINT32 offset;

       pciConfigInLong (pLoc->bus, pLoc->device, 0, PCI_CFG_CAP_PTR, &cpr);
       offset = cpr & 0xFC;
       pciConfigInLong (pLoc->bus, pLoc->device, 0, offset, &lr);
       if ((lr & 0x00020000 /*LDTCAP_CMD_DBL_ENDED*/) != 0)
           return ERROR;
       }

    return OK; /* Autoconfigure all other devices */
    }

/******************************************************************************
*
* sysPciAutoconfigIncludeSecondary - Check if a secondary bus is to be probed.
*
* This routine is called by the probe logic for bridge devices (those with
* PCI Type 1 headers) and returns OK if the secondary bus of that bridge
* is to be probed as part of the autoconfiguration.
*
* RETURNS: ERROR for the PCI host bridge, otherwise OK.
*/

LOCAL STATUS sysPciAutoconfigIncludeSecondary
    (
    PCI_SYSTEM *pSys,		/* PCI_SYSTEM structure pointer */
    PCI_LOC *pLoc,		/* pointer to bridge function in question */
    UINT devVend		/* deviceID/vendorID of device */
    )
    {
    return OK;       /* Include all proper secondary buses */
    }

/******************************************************************************
*
* sysPciAutoconfigPreProbeBridgeInit -
*   Do any required fabric initialization for a secondary bus.
*
* This routine is called by the probe logic just prior to scanning a
* secondary bus and provides an opportunity to do initialization of
* the bus fabric.  It is a no-op except for the LDT host bridge,
* for which it does sizing and UnitID assignment for the LDT bus.
*
* RETURNS: N/A
*/

LOCAL void sysPciAutoconfigPreProbeBridgeInit
    (
    PCI_SYSTEM *pSys,		/* PCI_SYSTEM structure pointer */
    PCI_LOC *pLoc,		/* pointer to function in question */
    UINT devVend		/* deviceID/vendorID of device */
    )
    {
    /* XXX More general: see if LDT secondary capability exists. */
    if (devVend == SIBYTE_DEVICE(BCM1250_LDT_HOST_BRIDGE))
        ldtSecondaryInit (pLoc->bus, pLoc->device, pLoc->function, 1);
    }

/******************************************************************************
*
* sysPciAutoconfigPostProbeBridgeInit -
*   Do any required fabric or bridge finalization for a secondary bus.
*
* This routine is called by the probe logic just after scanning a
* secondary bus.  Always a noop.
*
* RETURNS: N/A
*/

LOCAL void sysPciAutoconfigPostProbeBridgeInit
    (
    PCI_SYSTEM *pSys,		/* PCI_SYSTEM structure pointer */
    PCI_LOC *pLoc,		/* pointer to function in question */
    UINT devVend		/* deviceID/vendorID of device */
    )
    {
    return;
    }

/******************************************************************************
*
* sysPciAutoconfigPreEnumBridgeInit -
*   Do any required initialization before enumerating a secondary bus.
*
* This routine is called by the enumeration (assignment) logic just
* before scanning the secondary bus of a bridge.  Always a noop.
*
* RETURNS: N/A
*/

LOCAL void sysPciAutoconfigPreEnumBridgeInit
    (
    PCI_SYSTEM *pSys,		/* PCI_SYSTEM structure pointer */
    PCI_LOC *pLoc,		/* pointer to function in question */
    UINT devVend		/* deviceID/vendorID of device */
    )
    {
    return;
    }

/******************************************************************************
*
* sysPciAutoconfigPostEnumBridgeInit -
*   Do any required finalization after enumerating a secondary bus.
*
* This routine is called by the enumeration (assignment) logic just
* after scanning the secondary bus of a bridge.  For an
* AP1011 (Sturgeon) LDT-to-PCI bridge, it assigns values to the
* Read Control and interrupt mapping registers.
*
* The Read Control register is initialized to enable prefetches for
* ReadLine and ReadMult commands (two lines each).  NOTE: This is
* a reasonable compromise setting but the value should be tuned to
* match the burst characteristics of the particular device(s) on
* the PCI bus and of the application.
*
* The interrupt mapping converts PCI intA - intD to LDT interrupts
* with the same source numbers.
*
* RETURNS: N/A
*/

LOCAL void sysPciAutoconfigPostEnumBridgeInit
    (
    PCI_SYSTEM *pSys,		/* PCI_SYSTEM structure pointer */
    PCI_LOC *pLoc,		/* pointer to function in question */
    UINT devVend		/* deviceID/vendorID of device */
    )
    {
    if (devVend == API_DEVICE(API_STURGEON))
	{
	UINT32 readParams;
	UINT32 intMap;
	UINT32 t, offset;

	/* set up READ register: Prefetch enable, 2 lines of fetch ahead */
	readParams = (2 << 5) | (2 << 2) | 0x3;
	pciConfigOutLong (pLoc->bus, pLoc->device, pLoc->function,
			  LPB_CFG_READ, readParams);

	/* Setup interrupt mapping for Block 1:
	     Enabled, Dest=Logical (CPU 0), Type=Fixed, Trigger=Edge */
	intMap = (LPB_INT_CTRL_ENABLE |
		  LPB_INT_CTRL_DESTMODE |                /* Logical */
		  (0x1 << LPB_INT_CTRL_DEST_SHIFT) |     /* CPU 0   */
		  (0x0 << LPB_INT_CTRL_MSGTYPE_SHIFT));  /* Fixed   */

	/* For the SWARM and CSWARMs, the LDT Host Bridge is assumed to
	   have the base interrupt mapping (by convention) and the
	   LDT device number is the effective "slot" number. */
	offset = pLoc->device;

	t = (intMap + offset);
	offset = (offset+1) % 4;
	t |= (intMap + offset) << 16;
	pciConfigOutLong (pLoc->bus, pLoc->device, pLoc->function,
			  LPB_CFG_INT_CTRL_BASE + 8, t);

	offset = (offset+1) % 4;
	t = (intMap + offset);
	offset = (offset+1) % 4;
	t |= (intMap + offset) << 16;
	pciConfigOutLong (pLoc->bus, pLoc->device, pLoc->function,
			  LPB_CFG_INT_CTRL_BASE + 12, t);

	pciConfigInLong (pLoc->bus, pLoc->device, pLoc->function,
			 LPB_CFG_INT_BLOCK1, &t);
	t &= 0xFFFFFF00;
	t |= (0x40 | (BCM1250_PCI_LDT_INT_MAP(BCM1250_PCI_INT_BASE) >> 2));
	pciConfigOutLong (pLoc->bus, pLoc->device, pLoc->function,
			  LPB_CFG_INT_BLOCK1, t);
	}
    return;
    }

/******************************************************************************
*
* sysPciAutoconfigIntrAssign - Assign interrupt source numbers.
*
* This routine computes the interrupt source number (PCI IntA - PCI IntD)
* for devices/functions as wired on the BCM912500 evaluation board.
* It should only be called for bus 0 devices.
*
* RETURNS: Source id for the BCM1250 interrupt mapper (56-59), or 0xFF if none.
*/

LOCAL UCHAR sysPciAutoconfigIntrAssign
    (
    PCI_SYSTEM *pSys,		/* PCI_SYSTEM structure pointer */
    PCI_LOC *pLoc,		/* pointer to function in question */
    UCHAR pin		/* contents of PCI int pin register */
    )
    {
    /* Since we set pSys->autIntRouting to TRUE, this should only be
       called for bus 0 devices. */
    UCHAR line;

    if (pLoc->bus == 0 && pin != 0 && pin <= 4)
        {
	switch (pLoc->device)
            {
	    case 0:
	        /* The PCI host bridge interrupt is used only in Device mode */
	        line = pin;
	        break;
	    case 1:
	        /* This is the LDT host bridge, and the interrupt lines
	           are only used to propagate to devices behind the
	           bridge.  We use the identity map, since there is an
	           immediate cascading with a Sturgeon.  That's also
	           consistent with CFE and with the IDSEL wiring below. */
	        line = pin;
	        break;
            /* case 5: case 6: case 7: */
	    case 14: case 15: case 16: case 17: case 18: case 19: case 20:
	        /* On SWARM and CSWARM, IDSEL0 is AD[11], but the AD lines
	           connected to IDSEL inputs are AD[16] (slot 0), AD[17]
                   (slot 1) and AD[18] (on-board USB). */
	        line = (((pLoc->device - 5) + (pin - 1)) % 4) + 1;
	        break;
	    default:
	        return (UCHAR)0xff;
	    }

	/* Now map to pci_inta .. pci_intd in the SCD interrupt bit vectors */
	return (UCHAR)(56 + (line - 1));
        }
    return (UCHAR)0xff;
    }


/* Auxiliary routines for bridge initialization. */

/******************************************************************************
*
* bcm1250PciHostBridgeInit - Initialize the PCI host bridge.
*
* This routine initializes the BCM1250 PCI host bridge to a state
* suitable for autoconfiguration.
*
* Note that the PCI host bridge has two, mostly disjoint, sets of
* configuration registers.  One is used in Host mode and is
* accessible from the ZBbus; the other is used in Device mode and is
* accessible from the PCI bus.  The MAP registers are shared but in
* Pass 1 are write-only, from the ZBbus side.  In pass 2, they are
* readable iff read_host is set.
*
* RETURNS: N/A
*/

LOCAL void bcm1250PciHostBridgeInit (void)
    {
    int i;
    UINT32 cr, t;

    /* reset the PCI busses */
    /* PCI is only reset at system reset */

    /* BCM1250: disable and clear the BAR0 MAP registers */
    for (i = 0; i < PHB_CFG_MAP_N_ENTRIES; i++)
        pciConfigOutLong (BCM1250_PCI_BRIDGE, PHB_CFG_MAP_BASE + 4*i, 0);

    /* Because they write to the ZBbus bank of configuration
       registers, some of the following initializations are noops in
       Device mode, but they do no harm. */

    /* PCI: enable bridge to PCI and PCI memory accesses, plus error
       handling */
    pciConfigOutWord (BCM1250_PCI_BRIDGE, PCI_CFG_COMMAND,
		      (PCI_CMD_MASTER_ENABLE | PCI_CMD_MEM_ENABLE
		       | PCI_CMD_WI_ENABLE
		       | PCI_CMD_SERR_ENABLE | PCI_CMD_PERR_ENABLE));

    /* PCI: clear errors */
    pciConfigOutWord (BCM1250_PCI_BRIDGE, PCI_CFG_STATUS,
		      (PCI_STATUS_DATA_PARITY_ERR | PCI_STATUS_ASSERT_SERR
		       | PCI_STATUS_MASTER_ABORT_RCV
		       | PCI_STATUS_TARGET_ABORT_RCV
		       | PCI_STATUS_TARGET_ABORT_GEN
		       | PCI_STATUS_PARITY_ERROR));

    /* PCI: push the writes */
    pciConfigInLong (BCM1250_PCI_BRIDGE, PCI_CFG_VENDOR_ID, &t);

    /* PCI: Set the subsystem id and enable external access (Device Mode) */
    pciConfigInLong (BCM1250_PCI_BRIDGE, (PCI_CFG_CLASS & 0xFC), &cr);
    if (PCI_REVISION(cr) >= 2)
        {
	UINT32 id;

	/* The following code copies the chip DeviceId/VendorId to the
           SubsystemId/SubsysVendor.  Change it to insert
           product-specific subsystem identifiers. */
	pciConfigInLong (BCM1250_PCI_BRIDGE, PCI_CFG_VENDOR_ID, &id);
	pciConfigOutLong (BCM1250_PCI_BRIDGE, PHB_CFG_SUBSYSSET, id);

	pciConfigOutLong (BCM1250_PCI_BRIDGE, PHB_CFG_READHOST, 0);
	pciConfigInLong(BCM1250_PCI_BRIDGE, PHB_CFG_READHOST, &t);  /* push */
	}
    }


/*
 * LDT host bridge initialization.  The following code is patterned
 * after ldtpcicfg.S from the Diag/soc directory. 
 */

/******************************************************************************
*
* lhbLinkReset - Reset the LDT link of the host bridge.
*
* This routine does a warm reset of the LDT fabric attached to the BCM1250
* LDT host bridge.
*
* RETURNS: N/A
*/

LOCAL void lhbLinkReset (void)
    {
    UINT32 prev, cmd;
    UINT32 brctrl;

    pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CMD, &prev);
    cmd = prev | LDT_CMD_WARM_RESET;
    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CMD, cmd);

    pciConfigInLong (BCM1250_LDT_BRIDGE, PPB_BRCTL_REG, &brctrl);
    brctrl |= PPB_BRCTL_S_RESET;
    pciConfigOutLong (BCM1250_LDT_BRIDGE, PPB_BRCTL_REG, brctrl);

    pciConfigInLong (BCM1250_LDT_BRIDGE, PPB_BRCTL_REG, &brctrl);
    brctrl &=~ PPB_BRCTL_S_RESET;
    pciConfigOutLong (BCM1250_LDT_BRIDGE, PPB_BRCTL_REG, brctrl);

    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CMD, prev);

    pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CMD, &cmd);  /* push */
    }

/******************************************************************************
*
* lhbLinkReady - Wait for the LDT chain of the host bridge to become ready.
*
* This routine polls the secondary link of the BCM1250 LDT host bridge
* waiting for the fabric to assert InitDone or LinkFail.  It gives up
* after maxpoll iterations.
*
* RETURNS: 1 if any link error bits are set during polling, 0 otherwise.
*/

LOCAL int lhbLinkReady
    (
    int maxpoll
    )
    {
    UINT32 ctrl;
    int count;
    int linkerr;

    count = 0;
    linkerr = 0;
    ctrl = 0;

    while ((ctrl & (LDT_CTRL_INIT_DONE | LDT_CTRL_LINK_FAIL)) == 0
	   && count < maxpoll)
        {
	pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CTRL, &ctrl);
        count++;
        if ((ctrl & 0x190) != 0)
	    linkerr = 1;
        }

    return linkerr;
  }

/******************************************************************************
*
* bcm1250LdtHostBridgeInit - Initialize the LDT host bridge.
*
* This routine initializes the BCM1250 LDT host bridge, specifies the
* parameters for the outgoing LDT link, sets the ready bit (SipReady)
* and waits for InitDone or LinkFail on the link. It times out after two
* retries.  If the link cannot be initialized, it will be skipped
* in subsequent probing.
*
* RETURNS: N/A
*/

LOCAL void bcm1250LdtHostBridgeInit (void)
    {
    UINT16 sriCmd;
    UINT32 t;           /* used for reads to push writes */
    int i;
    int linkerr;
    int retry;

    /*
     * First set up System Reset Initialization registers (Table 8-12)
     * for frequency compatible (LDT 0.17) mode. This code is designed
     * to be run following a full reset and might need modification to
     * deal with a warm reset (SipReady already set?).
     */

    /* LDT 0.17 compatibility mode:
     *   SriCmd = (!LinkFreqDirect, LdtPLLCompat, DisStarveCnt,
     *           TxInitialOffset=5,
     *             RxMargin=2,
     *             !SipReady)
     *   SriRxDen = 0x10
     *   SriTxDen = 0x10
     *
     * Empirically, RxMargin is not critical with a 200 MHz LDT clock
     * but must be small (less than 15, and 0 seems reliable) with a
     * 400 MHz LDT clock.
     *
     * Setting DisStarveCnt is recommended for Pass 1 parts.
     */

    /* When booted by CFE, we should not reinitialize the fabric.
     * We assume that discovering SipReady already set means that
     * we have entered by that path and the System Reset Initialization
     * registers already have the desired settings.
     */

    pciConfigInWord (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_CMD, &sriCmd);

    /* Clear any pending error bits */
    pciConfigInWord (BCM1250_LDT_BRIDGE, LHB_CFG_ERR_CTRL, &t);
    pciConfigOutWord (BCM1250_LDT_BRIDGE, LHB_CFG_ERR_CTRL, 
                      t | LHB_ERR_CTRL_STATUS);
    
    if ((sriCmd & LHB_SRI_CMD_SIPREADY) != 0)
	{
	/* Just do a warm reset */
        /* XXX : Doing reset seems to sometimes hang the core */
	/* lhbLinkReset (); */
	return;
	}

    sriCmd = (LHB_SRI_CMD_PLLCOMPAT
	      | LHB_SRI_CMD_DISSTARVECNT
	      | (2 << LHB_SRI_CMD_RXMARGIN_SHIFT)
	      | (5 << LHB_SRI_CMD_TXOFFSET_SHIFT)
	      );
    pciConfigOutWord (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_CMD, sriCmd);

    /* Set the SRI frequency dividers */
    pciConfigOutByte (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_TXDEN, 0x10);
    pciConfigOutByte (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_RXDEN, 0x10);
    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_TXNUM, 0x0000ffff);
    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_RXNUM, 0x0000ffff);
    pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_RXNUM, &t);  /* push */

    /* Directed test: SPIN(10) here */
    for (i = 0; i < 10; i++)
        pciConfigInLong (BCM1250_LDT_BRIDGE, PCI_CFG_VENDOR_ID, &t);

    /* Set the Link Frequency register */
    /* BCM91250x boards are now set for 400 MHz by default */
    pciConfigOutByte (BCM1250_LDT_BRIDGE, LHB_CFG_LDT_FREQ, 0x02); 

    /* Set the Error Control register (some fatal interrupts). */
    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_ERR_CTRL, 0x00001209);

    /* Set the SRI Xmit Control register (4 packets). */
    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_CTRL, 0x00041515);

    /* Set the Tx buffer size (16 buffers each). */
    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_TX_BUFCNT, 0x00ffffff);

    /* Push the writes */
    pciConfigInLong (BCM1250_LDT_BRIDGE, PCI_CFG_VENDOR_ID, &t);

    /* Indicate SIP Ready */
    sriCmd |= LHB_SRI_CMD_SIPREADY;
    pciConfigOutWord (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_CMD, sriCmd);
    pciConfigInWord (BCM1250_LDT_BRIDGE, LHB_CFG_SRI_CMD, &sriCmd);  /* push */

    for (i = 0; i < 10; i++)
        pciConfigInLong (BCM1250_LDT_BRIDGE, PCI_CFG_VENDOR_ID, &t);

    retry = 0;
    for (;;)
        {
        /* wait for LinkFail or InitDone */
        linkerr = lhbLinkReady (1 << 20);   /* empirical delay */

        /* check and clear errors */
        pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_ERR_CTRL, &t);
        if ((t & LHB_ERR_CTRL_STATUS) != 0)
	    {
	    linkerr = 1;
	    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_ERR_CTRL, t);
	    }
        pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CTRL, &t);
        if ((t & 0x190) != 0)
	    {
	    linkerr = 1;
	    t |= 0x0f00;   /* Clear CrcErr bits too */
	    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CTRL, t);
	    }

        if (!linkerr || retry > 0)
	    break;

        /* Do an additional warm reset and retry. */
        retry++;
        lhbLinkReset ();
        }
 
    pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CTRL, &t);
 
    if ((t & LDT_CTRL_INIT_DONE) == 0)
        {
        /* Terminate the link */
        UINT32 lr;
 
        pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CTRL, &lr);
        lr |= LDT_CTRL_EOC;
        pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CTRL, lr);
        /* XXX API spec hints at needing a delay here. */
        lr |= LDT_CTRL_XMIT_OFF;
        pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_LINK_CTRL, lr);
        }
 
    /* Clear any pending error bits */
    pciConfigInLong (BCM1250_LDT_BRIDGE, LHB_CFG_ERR_CTRL, &t);
    pciConfigOutLong (BCM1250_LDT_BRIDGE, LHB_CFG_ERR_CTRL, t & 0xFF000000);    
    }

/******************************************************************************
*
* sysPciHwInit - Reset and initialize the host bridges.
*
* This routine puts the BCM1250 PCI and LDT host bridges into initial
* states suitable for autoconfiguration.
*
* RETURNS: N/A
*/

LOCAL void sysPciHwInit (void)
{
    UINT64 systemCfg;

    /* stop the SB-1250 from servicing any further PCI or LDT requests */
    pciConfigOutWord (BCM1250_PCI_BRIDGE, PCI_CFG_COMMAND, 0);
    pciConfigOutWord (BCM1250_LDT_BRIDGE, PCI_CFG_COMMAND, 0);

    /* determine if we are strapped for PCI Device Mode */
    systemCfg = SBREADCSR(A_SCD_SYSTEM_CFG);
    pciDeviceMode = ((systemCfg & M_SYS_PCI_HOST) == 0);

    bcm1250PciHostBridgeInit ();
    bcm1250LdtHostBridgeInit ();
}


/* The code below this point deals with LDT fabric initialization.
 * The callback from the autoconfiguration code is ldtSecondaryInit,
 * which is invoked before initial traversal of any secondary bus that
 * is LDT-based.
 *
 * This code is more-or-less a transliteration of CFE code and will
 * evolve to become more compatible with WRS conventions for VxWorks
 * source code. 
 */

/******************************************************************************
*
* pci_conf_read - Read a PCI configuration register.
*
* This routine is a convenient interface to pciConfigInLong, with the
* (bus, device, function) tuple packed into a 32-bit tag as used for
* addressing in Type 1 configuration cycles.
*
* RETURNS: The 32-bit value of the configuration register.
*/

LOCAL pcireg_t pci_conf_read
    (
    pcitag_t tag,
    int reg
    )
    {
    int bus, device, func;
    pcireg_t cr;

    bus = (tag >> 16) & PCI_BUSMAX;
    device = (tag >> 11) & PCI_DEVMAX;
    func = (tag >> 8) & PCI_FUNCMAX;
    pciConfigInLong (bus, device, func, reg, &cr);
    return cr;
    }

/******************************************************************************
*
* pci_conf_write - Write a PCI configuration register.
*
* This routine is a convenient interface to pciConfigOutLong, as above.
*
* RETURNS: N/A
*/

LOCAL void pci_conf_write
    (
    pcitag_t tag,
    int reg,
    pcireg_t data
    )
    {
    int bus, device, func;

    bus = (tag >> 16) & PCI_BUSMAX;
    device = (tag >> 11) & PCI_DEVMAX;
    func = (tag >> 8) & PCI_FUNCMAX;
    pciConfigOutLong (bus, device, func, reg, data);
    }


/******************************************************************************
*
* pciFindLdtCapability - Find an LDT capability for a device.
*
* This routine finds an LDT capability block for a selected device/function
* and link type (primary/secondary).
*
* RETURNS: Offset of the capability block in configuration space, or
*          0 if no such capability found.
*/

LOCAL int pciFindLdtCapability
    (
    pcitag_t tag,
    int secondary
    )
    {
    pcireg_t cpr;
    pcireg_t cr;
    int offset, prev;
    int type;

    cpr = pci_conf_read (tag, PCI_CAPLISTPTR_REG);
    offset = PCI_CAPLIST_PTR(cpr) &~ 0x3;
    prev = 0;
  
    while (offset != 0 && offset != prev)
	{
	cr = pci_conf_read (tag, offset);
	if (PCI_CAPLIST_CAP(cr) == PCI_CAP_LDT)
	    {
	    type = LDTCAP_CMD_TYPE(cr);
	    if (secondary && type == LDTCAP_CMD_TYPE_HOST)
		return offset;
	    if (!secondary && type == LDTCAP_CMD_TYPE_SLAVE)
		return offset;
	    }
	prev = offset;
	offset = PCI_CAPLIST_NEXT(cr) &~ 0x3;
	}
    return 0;
    }

/******************************************************************************
*
* pciGetLdtLink - Get the value of a selected link control register.
*
* This routine gets the value of a Link Control/Config register
* given the capability offset and the index of the link.
*
* RETURNS: 32-bit value of the register.
*/

LOCAL pcireg_t pciGetLdtLink
    (
    pcitag_t tag,
    int offset,
    int index
    )
    {
    return pci_conf_read (tag, offset + LDTCAP_LINK0_OFF + index*4);
    }

/******************************************************************************
*
* pciSetLdtLink - Set the value of a selected link control register.
*
* This  routine sets the value of a Link Control/Config register given
* the capability offset and the index of the link.
*
* RETURNS: N/A
*/

LOCAL void pciSetLdtLink
    (
    pcitag_t tag,
    int offset,
    int index,
    pcireg_t lr
    )
    {
    pci_conf_write (tag, offset + LDTCAP_LINK0_OFF + index*4, lr);
    }


/* LDT bus initialization and sizing. */

/******************************************************************************
*
* ldtWaitReady - Wait for link fabric initialization to complete.
*
* This routine waits for initialization of the link selected by
* capability offset and link index to complete, or for a timeout.
*
* RETURNS: 0 on success, non-0 on failure.
*/

LOCAL int ldtWaitReady
    (
    pcitag_t tag,
    int offset,
    int index
    )
    {
    int i;
    pcireg_t lr;

    i = 0x10000;  /* empirical */
    do
	{
        if (--i == 0)
	    return 0;
        lr = pciGetLdtLink (tag, offset, index);
	}
	while ((lr & (LDTCAP_LINK_CTRL_INIT | LDTCAP_LINK_CTRL_LKFAIL)) == 0);

    return 1;
    }

/* LDT fabric initialization.  See LDT Spec, Section 13.3. */

/******************************************************************************
*
* ldtFabricInit - Initialize the links and unit ids of an LDT chain.
*
* This routine initializes an LDT chain by sizing the fabric and assigning
* unit IDs, after which the chain can be configured as if it were a PCI bus.
* Note that this code should only be run by the master in a double-hosted
* chain.
*
* RETURNS: 1 if the chain is double-hosted, otherwise 0.
*/

LOCAL int ldtFabricInit
    (
    int bus
    )
    {
    int nextFreeId;
    int doubleEnded;
    int offset;
    int link;
    pcitag_t  tag;
    pcireg_t  cmd, lr;
    pcireg_t  id;
    int  i;

    nextFreeId = 1;
    doubleEnded = 0;

    for (;;)
        {
	tag = pci_make_tag (bus, 0, 0);

	offset = pciFindLdtCapability (tag, LDT_PRIMARY);
	if (offset == 0)
            {
	    /* There is no primary interface; we must have found a host. */
	    offset = pciFindLdtCapability (tag, LDT_SECONDARY);
	    if (offset != 0)
                {
		pciConfigInLong (bus, 0, 0, offset+LDTCAP_CMD_OFF, &lr);
		lr |= LDTCAP_CMD_DBL_ENDED;
		pciConfigOutLong (bus, 0, 0, offset+LDTCAP_CMD_OFF, lr);
		doubleEnded = 1;
	        }
	    break;
	    }

	/* Otherwise, we have the primary interface. */

	/* Rewrite the old value to set the master host bit.  XXX delay? */
	pciConfigInLong (bus, 0, 0, offset+LDTCAP_CMD_OFF, &cmd);
	pciConfigOutLong (bus, 0, 0, offset+LDTCAP_CMD_OFF, cmd);
	pciConfigInLong (bus, 0, 0, offset+LDTCAP_CMD_OFF, &cmd);

	cmd &= ~LDTCAP_CMD_BASE_ID_MASK;
	cmd |= (nextFreeId << LDTCAP_CMD_BASE_ID_SHIFT);
	pciConfigOutLong (bus, 0, 0, offset+LDTCAP_CMD_OFF, cmd);
	/* Unit id just changed */

	/* LDT configuration writes are effectively posted (pass 1).
	   Allow for some delay here in the most innocuous way possible
	   (a read with the new tag could hang). */
	for (i = 0; i < 10; i++)
	    pciConfigOutLong (bus, 0, 0, PCI_CFG_VENDOR_ID, 0);

	/* Update the tag */
	tag = pci_make_tag (bus, nextFreeId, 0);
	/* Read the id back, for good measure */
	pciConfigInLong (bus, nextFreeId, 0, PCI_CFG_VENDOR_ID, &id);

	nextFreeId += LDTCAP_CMD_UNIT_CNT(cmd);

	link = (cmd >> LDTCAP_CMD_MASTER_HOST_SHIFT) & 0x1; /* Upstream link */
	link ^= 1;                                          /* Downstream */

	ldtWaitReady (tag, offset, link);

	lr = pciGetLdtLink (tag, offset, link);
	if ((lr & LDTCAP_LINK_CTRL_INIT) == 0 || nextFreeId > 0x1f)
	    {
	    /* No downstream link or too many devices, set end of chain */
	    lr |= LDTCAP_LINK_CTRL_EOC;
	    pciSetLdtLink (tag, offset, link, lr);
	    lr |= LDTCAP_LINK_CTRL_TXO;
	    pciSetLdtLink (tag, offset, link, lr);
	    break;
	    }
        }

    return doubleEnded;
    }


/* LDT bridge and fabric initialization for a secondary bus. */

/******************************************************************************
*
* ldtSecondaryInit -
*    Top level control of bridge secondary and fabric initialization for LDT.
*
* This routine resets and then initializes the secondary of a bridge
* to LDT (currently only the BCM1250 host bridge).
*
* RETURNS: N/A
*/

LOCAL void ldtSecondaryInit
    (
    int bus,
    int device,
    int func,       /* the bridge */
    int bus2        /* its secondary bus number */
    )
    {
    int  offset;
    int  doubleEnded;
    pcireg_t brctl, cr, lr;
    pcireg_t t;
    pcitag_t tag = BCM1250_PCI_MAKE_TAG(bus, device, func);

    /* Attempt a Secondary Bus Reset. */
    pciConfigInLong (bus, device, func, PPB_BRCTL_REG, &brctl);
    brctl |= PPB_BRCTL_S_RESET;
    pciConfigOutLong (bus, device, func, PPB_BRCTL_REG, brctl);

    pciConfigInLong (bus, device, func, PPB_BRCTL_REG, &brctl);
    if ((brctl & PPB_BRCTL_S_RESET) != 0)
        {
        int  i;
        /* Bit can be written, assume soft reset is implemented. */
        brctl &=~ PPB_BRCTL_S_RESET;
	pciConfigOutLong (bus, device, func, PPB_BRCTL_REG, brctl);

        /* Add some delay (duration is a guess) */
        for (i = 0; i < 100; i++)
	    pciConfigInLong (bus, device, func, PPB_BRCTL_REG, &t);
        /* Alternatively, wait for LinkFail or InitDone. */
        }

    offset = pciFindLdtCapability (tag, LDT_SECONDARY);
    if (offset != 0)
        {
	int ready;

        ready = ldtWaitReady (tag, offset, 0);
	lr = pciGetLdtLink (tag, offset, 0);

	if (ready)
	    {
            if ((lr & LDTCAP_LINK_CTRL_INIT) != 0)
                {
                doubleEnded = ldtFabricInit (bus2);
                }
            else
                {
                lr |= LDTCAP_LINK_CTRL_EOC;
                pciSetLdtLink (tag, offset, 0, lr);
                lr |= LDTCAP_LINK_CTRL_TXO;
                pciSetLdtLink (tag, offset, 0, lr);
                doubleEnded = 0;
                }
            pciConfigInLong (bus, device, func, offset + LDTCAP_CMD_OFF, &cr);
            if (doubleEnded)
                cr |= LDTCAP_CMD_DBL_ENDED;
            else
                cr &=~ LDTCAP_CMD_DBL_ENDED;
            pciConfigOutLong (bus, device, func, offset + LDTCAP_CMD_OFF, cr);
	    }
	else
	    {
            lr |= LDTCAP_LINK_CTRL_EOC | LDTCAP_LINK_CTRL_TXO;
	    pciSetLdtLink (tag, offset, 0, lr);
	    }
        }
    }

#endif /*INCLUDE_PCI*/
