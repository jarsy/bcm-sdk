/* $Id: configNet.h,v 1.3 2011/07/21 16:14:55 yshtil Exp $
    EXTERNAL SOURCE RELEASE on 12/03/2001 3.0 - Subject to change without notice.

*/
/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
    

*/
/* configNet.h - Network configuration header file */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,30dec99,alp  Added Support for SPR#29948
01a,20aug99,vnk  written for idt by Teamf1 Inc.
*/

#ifndef INCconfigNeth
#define INCconfigNeth

#include "vxWorks.h"
#include "end.h"

#ifdef BCM_END_DRV
IMPORT END_OBJ* NetdrvLoad(char* initString, void *ap);
#endif

END_TBL_ENTRY endDevTbl [] =
    {
#ifdef BCM_END_DRV
    { 0, NetdrvLoad, "netdrv!", 0, NULL, FALSE},
#endif
    {0, END_TBL_END, NULL, 0, NULL, FALSE},
    };


#endif /* INCconfigNeth */

