/* sysNet.h - system-dependent Network Header File */

/* $Id: sysNet.h,v 1.3 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2005, 2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,23may07,b_m  move MAX_MAC_DEVS macro to config.h.
01a,14oct05,dtr	 created from ads834x/01c
*/


#ifndef  __INCsysNeth
#define  __INCsysNeth

#ifdef __cplusplus
    extern "C" {
#endif

#include <vxWorks.h>
#include <config.h>

extern const char *sysNetDevName[MAX_MAC_DEVS];

/* Prototypes */
int sysMacIndex2Dev (int index);

int sysMacIndex2Unit (int index);

STATUS sysMacOffsetGet (char *ifName, int ifUnit, char **ppEnet,
                              int *	pOffset);

STATUS sysNetMacNVRamAddrGet (char *ifName, int ifUnit, UINT8 *ifMacAddr,
                              int ifMacAddrLen);
STATUS sysNetMacAddrGet (char *ifName, int ifUnit, UINT8 *ifMacAddr,
                              int ifMacAddrLen);
STATUS sysNetMacAddrSet (char *ifName, int ifUnit, UINT8 *ifMacAddr,
                              int ifMacAddrLen);

#ifdef __cplusplus
    }
#endif

#endif   /* __INCsysNeth  */

