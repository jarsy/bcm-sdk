/* sysNet.c - system-dependent Network Library */

/* $Id: sysNet.c,v 1.4 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2005-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01h,23may07,b_m  move MAX_MAC_DEVS macro to config.h.
01g,18aug06,dtr  WIND61530 - Fix array overflow.
01f,03may06,dtr  SPR#120513 - check status of sysNetMacNVRamAddrGet.
01e,01may06,dtr  ETSEC now uses motetsec device name. SPR#102448.
01d,21feb06,dtr  SPR #117719 - Tidy up support for HEND and END.
01c,03feb06,wap  Add address get routine for HEND ETSEC devices
01b,27jan06,dtr  Tidy up.
01a,05jul05,dtr	 adopted from sbc8260/sysNet.c/01a
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
flash.  The RAM buffer contains eight entries, which is
more than currently needed by this board, but can be
considered as room for expansion in future boards using
a derivative of this BSP.  This RAM buffer is contained
in the array glbEnetAddr[][].
*/

#ifdef ETHERNET_MAC_HANDLER

#include <vxWorks.h>
#include "config.h"

/* locals */

/* defines */

#define MAC_OFFSET_MOTTSEC	0
#ifdef INCLUDE_MOT_ETSEC_HEND
const char *sysNetDevName[MAX_MAC_DEVS] = {"motetsec"};
#else
const char *sysNetDevName[MAX_MAC_DEVS] = {"mottsec"};
#endif
/* globals */

IMPORT int dynamicMacAddrGen
    (
    UINT8 * ifName,		/* interface name */
    int ifUnit,			/* interface unit */
    UINT8 * ifMacAddr,		/* address buffer */
    int ifMacAddrLen		/* length of buffer */
    );

/* locals */

LOCAL UINT8 glbEnetAddr[MAX_MAC_ADRS][MAC_ADRS_LEN + 2] = {
{ BRCM_ENET0, BRCM_ENET1, BRCM_ENET2, CUST_ENET3_0, CUST_ENET4, CUST_ENET5 },
{ BRCM_ENET0, BRCM_ENET1, BRCM_ENET2, CUST_ENET3_1, CUST_ENET4, CUST_ENET5 },
{ BRCM_ENET0, BRCM_ENET1, BRCM_ENET2, CUST_ENET3_2, CUST_ENET4, CUST_ENET5 },
{ BRCM_ENET0, BRCM_ENET1, BRCM_ENET2, CUST_ENET3_3, CUST_ENET4, CUST_ENET5 }
};

LOCAL UINT8 sysInvalidAddr[2][MAC_ADRS_LEN] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
};

/***********************************************************
*
* sysMacOffsetGet - Calculate table offset
*
*  This routine calculates which table entry corresponds to
*  the specified interface.
*
*  Two values are calculated and returned in the parameters
*  pointed to by ppEnet and pOffset.
*
* RETURNS: ERROR if the interface is not known; OK otherwise
*
* ERRNO: N/A
*/

STATUS sysMacOffsetGet
    (
    char *	ifName,		/* interface name */
    int		ifUnit,		/* interface unit */
    char **	ppEnet,		/* pointer to glbEnetAddr[][] entry */
    int *	pOffset		/* offset in NVRAM */
    )
    {

    /*
     * The address offsets into NVRAM and glbEnetAddr[] are:
     *
     *	mottsec0 - offset 0
     *
     */

    if ( (strcmp(ifName,"mottsec") == 0) || (strcmp(ifName,"motetsec") == 0) )
	{
	if ( ifUnit > 3 )
	    return(ERROR);

	*pOffset = ifUnit * (MAC_ADRS_LEN + 2); /* Round up to 32 bit word */
        *ppEnet  = (char*)glbEnetAddr[ifUnit + MAC_OFFSET_MOTTSEC];
	}
    else
    	return(ERROR);

    return(OK);
    }

STATUS usrBootLineCrack(char *bootString, BOOT_PARAMS *pParams);
/*
 * Extract MAC address from vxboot string
 */
LOCAL STATUS sysNetBootLineMacAddrGet
    (
    int		ifUnit,
    UINT8 *	macAddr,
    int		ifMacAddrLen
    )
    {
        char* bootLine;
        BOOT_PARAMS params;

        if ((ifUnit > 1) ||(macAddr == NULL) || (ifMacAddrLen < 6)) {
            return(ERROR);
        }
        bootLine = BOOT_LINE_ADRS;
        if (usrBootLineCrack(bootLine, &params) != OK) {
            return (ERROR);
        }
        if (params.other [0] != EOS)
            {
            int intArr[6];
            char *pStr;
            int ix, jx;

            /* extract enet MAC address from 'other' parameter */
            pStr = params.other;
            for(jx = 0; jx <=1; jx++) {
                if ((pStr = strstr(pStr, ";mac=")) != NULL)
                    {
                    if (sscanf(&pStr[5], "%02x:%02x:%02x:%02x:%02x:%02x",
                               &intArr[0], &intArr[1], &intArr[2], 
                               &intArr[3], &intArr[4], &intArr[5]) == 6)
                        {
                            if (ifUnit == jx) {
                                for (ix = 0; ix < 6; ix++)
                                {
                                    macAddr[5 - ix] = intArr[ix];
                                }
                                return (OK);
                            } else {
                                pStr += 5;
                            }
                        }
                    }
                }
            }
        return(ERROR);
    }

/***********************************************************
*
* sysNetMacNVRamAddrGet - Get interface MAC address
*
*  This routine gets the current MAC address from the
*  Non Volatile RAM, and store it in the ifMacAddr
*  buffer provided by the caller.
*
*  It is not required for the BSP to provide NVRAM to store
*  the MAC address.  Also, some interfaces do not allow
*  the MAC address to be set by software.  In either of
*  these cases, this routine simply returns ERROR.
*
*  Given a MAC address m0:m1:m2:m3:m4:m5, the byte order
*  of ifMacAddr is:
*	m0 @ ifMacAddr
*	m1 @ ifMacAddr + 1
*	m2 @ ifMacAddr + 2
*	m3 @ ifMacAddr + 3
*	m4 @ ifMacAddr + 4
*	m5 @ ifMacAddr + 5
*
* RETURNS: OK, if MAC address available, ERROR otherwise
*
* ERRNO: N/A
*/

STATUS sysNetMacNVRamAddrGet
    (
    char *	ifName,
    int		ifUnit,
    UINT8 *	ifMacAddr,
    int		ifMacAddrLen
    )
    {
    int   offset;
    char *pEnet;

    /* fetch address line & offset from glbEnetAddr[] table */

    if (sysMacOffsetGet(ifName, ifUnit, &pEnet, &offset) != OK)
        return(ERROR);

    if (sysNetBootLineMacAddrGet(ifUnit, pEnet, ifMacAddrLen) != OK)
    {
#if (NV_RAM_SIZE != NONE)
    /* get MAC address from NvRAM. */
    sysNvRamGet (pEnet, ifMacAddrLen, NV_MAC_ADRS_OFFSET+offset);
#endif /* (NV_RAM_SIZE != NONE) */
    }

    if ( memcmp(pEnet, sysInvalidAddr[0], MAC_ADRS_LEN) == 0 )
        return(ERROR);
    if ( memcmp(pEnet, sysInvalidAddr[1], MAC_ADRS_LEN) == 0 )
        return(ERROR);

    /* mac address in memory only */
    memcpy (ifMacAddr, pEnet, ifMacAddrLen);

    return (OK);
    }

/***********************************************************
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
* ERRNO: N/A
*/

STATUS sysNetMacAddrGet
    (
    char *	ifName,
    int		ifUnit,
    UINT8 *	ifMacAddr,
    int		ifMacAddrLen
    )
    {
    /*
     * None of our interfaces can be queried directly.
     * Return ERROR to indicate that we need to use
     * RAM/NVRAM instead.
     */
    return(ERROR);
    }

/***********************************************************
*
* sysNetMacAddrSet - Save interface MAC address
*
*  This routine saves the MAC address specified in
*  ifMacAddr to the appropriate location in NVRam (if
*  possible) and update the specified interface to use
*  the specified MAC address.
*
*  If the network interface MAC address cannot be set,
*  this routine returns ERROR.
*
* RETURNS: OK, if MAC address available, ERROR otherwise
*
* ERRNO: N/A
*/

STATUS sysNetMacAddrSet
    (
    char *	ifName,
    int		ifUnit,
    UINT8 *	ifMacAddr,
    int		ifMacAddrLen
    )
    {
    int   offset;
    char *pEnet;

    /* fetch address line & offset from glbEnetAddr[] table */

    if (sysMacOffsetGet(ifName, ifUnit, &pEnet, &offset) != OK)
        return(ERROR);

#if (NV_RAM_SIZE != NONE)
    /* check MAC address in NvRAM. */
    sysNvRamGet (pEnet, ifMacAddrLen, NV_MAC_ADRS_OFFSET+offset);
    if (0 == memcmp (ifMacAddr, pEnet, ifMacAddrLen))
	{
	/* same address so don't erase and rewrite flash */
	printf("Address unchanged\n");
	return (OK);
	}

    sysNvRamSet (ifMacAddr, ifMacAddrLen, NV_MAC_ADRS_OFFSET+offset);
#endif /* (NV_RAM_SIZE != NONE) */

    /* mac address in memory only */
    memcpy (ifMacAddr, pEnet, ifMacAddrLen);

    return (OK);
    }

/***********************************************************
*
* sysMacIndex2Unit - convert index range to unit number
*
* This routine converts an index range 0..MAX_MAC_ADRS-1
* to a unit number.
*
* RETURNS: unit number of indexed device
*
* ERRNO: N/A
*/

int sysMacIndex2Unit
    (
    int index
    )
    {
    return (index);   /* remaining three are motfcc (0,1,2) */
    }

/***********************************************************
*
* sysMacIndex2Dev - convert index range to device string
*
* This routine converts an index range 0..MAX_MAC_ADRS-1
* to a device string index e.g. motfcc.
*
* RETURNS: index access device name in sysNetDevName
*
* ERRNO: N/A
*/

int sysMacIndex2Dev
    (
    int index
    )
    {
    return (0);      /* remaining are motfcc */
    }
#ifdef ENET_MAC_DEBUG

/***********************************************************
*
* sysNetMacAddrClear - clear MAC address in FLASH/NVRAM
*
*  This routine clears the storage locations in NVRAM
*  reserved for the MAC address of the specified interface.
*
*  This is useful for debugging the 'M' command.  To use
*  this, boot vxWorks and run sysNetMacAddrClear() from
*  the shell.  After running this command, you can test
*  the bootrom 'M' command under conditions similar to a
*  new, unprogrammed board.
*
* RETURNS: OK, if MAC address available, ERROR otherwise
*/

int sysNetMacAddrClear
    (
    char *	ifName,
    int		ifUnit
    )
    {
    UINT8 	ifMacAddr[MAC_ADRS_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    int   offset;
    char *pEnet;

    if (sysMacOffsetGet(ifName, ifUnit, &pEnet, &offset) != OK)
        return(ERROR);

    sysNvRamSet (ifMacAddr, MAC_ADRS_LEN, NV_MAC_ADRS_OFFSET+offset);
    memcpy (ifMacAddr, pEnet, MAC_ADRS_LEN);
    return(0);
    }
#endif /* ENET_MAC_DEBUG */

#ifdef INCLUDE_MOT_ETSEC_HEND
/***********************************************************************
*
* sysMotEtsecEnetAddrGet - get the hardware Ethernet address
*
* This routine provides the six byte Ethernet hardware address that will be
* used by each individual TSEC device unit.  This routine must copy
* the six byte address to the space provided by <addr>.
*
* RETURNS: OK, or ERROR if the Ethernet address cannot be returned.
*/
STATUS sysMotEtsecEnetAddrGet
    (
    int     unit,
    UCHAR * pAddr
    )
    {

    return(sysNetMacNVRamAddrGet ("motetsec",unit,pAddr,MAC_ADRS_LEN));

    }
#else
#ifdef INCLUDE_MOT_TSEC_HEND
IMPORT STATUS    sysEnetAddrGet (UINT32, UCHAR *);
/***********************************************************************
*
* sysEnetAddrGet - get the hardware Ethernet address
*
* This routine provides the six byte Ethernet hardware address that will be
* used by each individual TSEC device unit.  This routine must copy
* the six byte address to the space provided by <addr>.
*
* RETURNS: OK, or ERROR if the Ethernet address cannot be returned.
*/
STATUS sysEnetAddrGet
    (
    UINT32     unit,
    UCHAR * pAddr
    )
    {

    return(sysNetMacNVRamAddrGet ("mottsec",unit,pAddr,MAC_ADRS_LEN));

    }
#else
/***********************************************************************
*
* sysMotTsecEnetAddrGet - get the hardware Ethernet address
*
* This routine provides the six byte Ethernet hardware address that will be
* used by each individual TSEC device unit.  This routine must copy
* the six byte address to the space provided by <addr>.
*
* RETURNS: OK, or ERROR if the Ethernet address cannot be returned.
*/
STATUS sysMotTsecEnetAddrGet
    (
    int     unit,
    UCHAR * pAddr
    )
    {

    return(sysNetMacNVRamAddrGet ("mottsec",unit,pAddr,MAC_ADRS_LEN));

    }
#endif
#endif

#endif /* ETHERNET_MAC_HANDLER */
