/* sysNet.h - system-dependent Network Header File */

/* $Id: sysNet.h,v 1.3 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,14oct05dtr	 created from ads834x/01c
*/


#ifndef  __INCsysNeth
#define  __INCsysNeth 

#ifdef __cplusplus
    extern "C" {
#endif

#include <vxWorks.h>
#include <config.h>

#define MAX_MAC_DEVS 2 /* two network devices (fcc, scc) */

extern const char *sysNetDevName[MAX_MAC_DEVS];

/* Prototypes */
#define ENET_DEFAULT         0x18100000 /* BRCM fixed MAC addr; see WR_ENETx */

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

