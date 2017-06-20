/* sysNet.c - system-dependent network library */

/*
 * Copyright (c) 2008 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,14nov13,rab  Update for brcm_katana2.
01a,25feb08,x_s  Initial version.
*/

/*
DESCRIPTION
This module provides BSP functionality to support the
bootrom 'M' command to modify MAC addresses of on-board
network interfaces.

MAC adddress routines provided by the BSP in this file are:
    sysNetMacNVRamAddrGet()
    sysNetMacAddrGet()
    sysNetMacAddrSet()

This board provides storage in flash for the MAC addresses
of the motfcc and motscc interfaces.  This library also
implements a RAM buffer to represent the contents of the
flash. The RAM buffer contains eight entries, which is
more than currently needed by this board, but can be
considered as room for expansion in future boards using
a derivative of this BSP.  This RAM buffer is contained
in the array glbEnetAddr[][].
*/

#ifdef INCLUDE_END

#include <vxWorks.h>
#include "config.h"
#include <ctype.h>
#include <fioLib.h>
#include "sysNet.h"

/* define*/

/* globals */

unsigned char glbEnetAddr [MAX_MAC_DEVS][MAC_ADRS_LEN] =
    {
    { IPROC_ENET0, IPROC_ENET1, IPROC_ENET2, CUST_ENET3, CUST_ENET4, CUST_ENET5 }
    };

const char *sysNetDevName [MAX_MAC_DEVS] = {"et"};

LOCAL UINT8 sysInvalidAddr[2][MAC_ADRS_LEN] =
    {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    };

/*******************************************************************************
*
* sysMacIndex2Dev - convert index range to device string
*
* This routine converts an index range 0..MAX_MAC_ADRS-1
* to a device string index e.g. mottsec.
*
* RETURNS: index access device name in sysNetDevName
*
* ERRNO
*/

int sysMacIndex2Dev
    (
    int index
    )
    {
    return index;
    }

/*******************************************************************************
*
* sysMacIndex2Unit - convert index range to unit number
*
* This routine converts an index range 0..MAX_MAC_ADRS-1
* to a unit number.
*
* RETURNS: unit number of indexed device
*
* ERRNO
*/

int sysMacIndex2Unit
    (
    int index
    )
    {
    return index;
    }

/*******************************************************************************
*
* sysMacOffsetGet - Calculate table offset
*
* This routine calculates which table entry corresponds to
* the specified interface.
*
* Two values are calculated and returned in the parameters
* pointed to by ppEnet and pOffset.
*
* RETURNS: ERROR if the interface is not known; OK otherwise
*
* ERRNO
*/

STATUS sysMacOffsetGet
    (
    char *  ifName,     /* interface name */
    int     ifUnit,     /* interface unit */
    char ** ppEnet,     /* pointer to glbEnetAddr[][] entry */
    int *   pOffset     /* offset in NvRAM */
    )
    {

    int index;      /* index into device names: always 0 */

    if ( (ifUnit < 0) || (ifUnit > MAX_MAC_DEVS) )
        return (ERROR);

    index =ifUnit;  /* index into device names: always 0 */

    /* validate device name */

    if ( strcmp(ifName, sysNetDevName[index]) == 0 )
        {

        /* return MAC addr and NvRAM offset */

        *ppEnet  = (char*)glbEnetAddr[index];
        *pOffset = index * MAC_ADRS_LEN;

        return(OK);
        }

    /* device not found */

    return(ERROR);
    }


/*******************************************************************************
*
* sysNetMacNVRamAddrGet - Get interface MAC address
*
*  This routine gets the current MAC address from the
*  Non Volatile RAM, and stores it in the ifMacAddr
*  buffer provided by the caller.
*
*  It is not required for the BSP to provide NvRAM to store
*  the MAC address.  Also, some interfaces do not allow
*  the MAC address to be set by software.  In either of
*  these cases, this routine simply returns ERROR.
*
*  Given a MAC address m0:m1:m2:m3:m4:m5, the byte order
*  of ifMacAddr is:
*   m0 @ ifMacAddr
*   m1 @ ifMacAddr + 1
*   m2 @ ifMacAddr + 2
*   m3 @ ifMacAddr + 3
*   m4 @ ifMacAddr + 4
*   m5 @ ifMacAddr + 5
*
* RETURNS: OK, if MAC address available, ERROR otherwise
*
* ERRNO
*/

STATUS sysNetMacNVRamAddrGet
    (
    char *  ifName,
    int     ifUnit,
    UINT8 * ifMacAddr,
    int     ifMacAddrLen
    )
    {
    int    offset;
    char * pEnet;
    char   addr2[8];

    /* fetch address line & offset from glbEnetAddr[] table */

    if (sysMacOffsetGet(ifName, ifUnit, &pEnet, &offset) != OK)
        return(ERROR);

    /* copy to ifMacAddr*/

    memcpy (ifMacAddr, pEnet, ifMacAddrLen);

#if (NV_RAM_SIZE != NONE)

    /* get MAC address from NvRAM. */

    sysNvRamGet (addr2, ifMacAddrLen, NV_MAC_ADRS_OFFSET+offset);

#endif /* (NV_RAM_SIZE != NONE) */

    /* check MAC address is valid */

    if ( memcmp(addr2, sysInvalidAddr[0], MAC_ADRS_LEN) == 0 )
        return(ERROR);

    if ( memcmp(addr2, sysInvalidAddr[1], MAC_ADRS_LEN) == 0 )
        return(ERROR);

    /* copy MAC address to pointer for return */

    memcpy (ifMacAddr, addr2, ifMacAddrLen);

    return (OK);
    }

/*******************************************************************************
*
* sysNetMacAddrGet - Get interface MAC address
*
*  This routine gets the current MAC address from the
*  network interface, and stores it in the ifMacAddr
*  buffer provided by the caller.
*
*  If the network interface cannot be queried about the
*  MAC address, this routine returns ERROR.
*
* RETURNS: OK, if MAC address available, ERROR otherwise
*
* ERRNO
*/

STATUS sysNetMacAddrGet
    (
    char *  ifName,
    int     ifUnit,
    UINT8 * ifMacAddr,
    int     ifMacAddrLen
    )
    {
    /*
     * None of our interfaces can be queried directly.
     * Return ERROR to indicate that we need to use
     * RAM/NvRAM instead.
     */

    return(OK);
    }

/*******************************************************************************
*
* sysNetMacAddrSet - Save interface MAC address
*
*  This routine saves the MAC address specified in
*  ifMacAddr to the appropriate location in NvRAM (if
*  possible).
*
*  If the network interface MAC address cannot be set,
*  this routine returns ERROR.
*
* RETURNS: OK, if MAC address available, ERROR otherwise
*
* ERRNO
*/

STATUS sysNetMacAddrSet
    (
    char *  ifName,
    int     ifUnit,
    UINT8 * ifMacAddr,
    int     ifMacAddrLen
    )
    {
    int    offset;
    char * pEnet;

    /* fetch address line & offset from glbEnetAddr[] table */

    if (sysMacOffsetGet(ifName, ifUnit, &pEnet, &offset) != OK)
        {
        return(ERROR);
        }

#if (NV_RAM_SIZE != NONE)
    /* check MAC address in NvRAM */

    sysNvRamGet (pEnet, ifMacAddrLen, NV_MAC_ADRS_OFFSET+offset);
    if (memcmp (ifMacAddr, pEnet, ifMacAddrLen) == 0)
        {

        /* same address so don't erase and rewrite flash */

        return(OK);
        }

    sysNvRamSet ((char *)ifMacAddr, ifMacAddrLen, NV_MAC_ADRS_OFFSET+offset);
#endif /* (NV_RAM_SIZE != NONE) */

    /* copy MAC address to RAM buffer */

    memcpy (pEnet, ifMacAddr, ifMacAddrLen);

    return (OK);
    }


/*******************************************************************************
*
* sysEnetAddrGet - gets the 6 byte ethernet address
*
* This routine gets the 6 byte ethernet address used by the ethernet device.
*
* RETURNS: OK
*
* SEE ALSO: sysEnetAddrSet()
*/

STATUS sysEnetAddrGet
    (
    int     unit,
    UCHAR * addr     /* where address is returned in */
    )
    {
#if (NV_RAM_SIZE != NONE)
	UCHAR   addr2[8];
#endif
	
    memcpy (addr, &glbEnetAddr[unit][0], MAC_ADRS_LEN);

#if (NV_RAM_SIZE != NONE)

    if (sysNvRamGet ((char *)&addr2[0], MAC_ADRS_LEN, 
                     NV_MAC_ADRS_OFFSET + unit * MAC_ADRS_LEN) != OK)
        return (OK);

    /* check MAC address is valid */

    if ( memcmp(addr2, sysInvalidAddr[0], MAC_ADRS_LEN) == 0 )
        return(OK);

    if ( memcmp(addr2, sysInvalidAddr[1], MAC_ADRS_LEN) == 0 )
        return(OK);

    memcpy (addr, &addr2[0], MAC_ADRS_LEN);

#endif /* (NV_RAM_SIZE != NONE) */

    return (OK);
    }

#endif /* INCLUDE_END */
