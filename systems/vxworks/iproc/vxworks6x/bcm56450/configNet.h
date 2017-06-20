/* configNet.h - network configuration header */

/*
 * Copyright (c) 2010 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01a,17may10, my_  created from arm_pbxa9 rev 01a
*/

#ifndef __INCconfigNeth
#define __INCconfigNeth

#ifdef __cplusplus
extern "C" {
#endif

#include <vxWorks.h>
#include <end.h>

#ifdef INCLUDE_END

END_TBL_ENTRY endDevTbl [] =
    {
    /* End of table marker */

    { 0, END_TBL_END, NULL, 0, NULL, FALSE}
    };

#endif /* INCLUDE_END */

#ifdef __cplusplus
}
#endif

#endif /* __INCconfigNeth */

