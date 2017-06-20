/* $Id: configNet.h,v 1.2 2011/07/21 16:14:21 yshtil Exp $
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

#ifdef INCLUDE_IL_END

#define IL_LOAD_FUNC   il_load     /* driver external interface */
#define IL_LOAD_STRING ""
IMPORT END_OBJ* IL_LOAD_FUNC (char*, void*);

#endif /* INCLUDE_IL_END */

#if defined(INCLUDE_ET0_END) || defined(INCLUDE_ET1_END)

#ifdef BROADCOM_BSP
#define ET_LOAD_FUNC   sysEtEndLoad     /* driver external interface */
#define ET_VIRT_LOAD_FUNC   sysEtvEndLoad     /* driver external interface */
#define WL_LOAD_FUNC   sysWlEndLoad     /* driver external interface */
#else
#define ET_LOAD_FUNC   et_load     /* driver external interface */
#define ET_VIRT_LOAD_FUNC   etv_load    
#define WL_LOAD_FUNC   wl_load    
#endif

#define ET_LOAD_STRING ""
IMPORT END_OBJ* ET_LOAD_FUNC (char*, void*);
IMPORT END_OBJ* ET_VIRT_LOAD_FUNC (char*, void*);
IMPORT END_OBJ* WL_LOAD_FUNC (char*, void*);

#endif /* INCLUDE_ET_END */

#ifdef INCLUDE_ET_END

#define ET_LOAD_FUNC   et_load     /* driver external interface */
#define ET_VIRT_LOAD_FUNC   etv_load    
#define ET_LOAD_STRING ""
IMPORT END_OBJ* ET_LOAD_FUNC (char*, void*);

#endif /* INCLUDE_ET_END */

END_TBL_ENTRY endDevTbl [] =
    {
#ifdef INCLUDE_WL_END
    {0, WL_LOAD_FUNC, ET_LOAD_STRING, 0,
    NULL, FALSE},
#endif

#ifdef INCLUDE_IL_END
    {0, IL_LOAD_FUNC, IL_LOAD_STRING, 0,
    NULL, FALSE},
#endif /* INCLUDE_IL_END */

#ifdef INCLUDE_ET0_END
    {0, ET_LOAD_FUNC, ET_LOAD_STRING, 0,
    NULL, FALSE}, 
#endif

#ifdef INCLUDE_ETV0_END
    {0, ET_VIRT_LOAD_FUNC, ET_LOAD_STRING, 0,
    NULL, FALSE},
#endif

#ifdef INCLUDE_ET1_END
    {1, ET_LOAD_FUNC, ET_LOAD_STRING, 0,
    NULL, FALSE},
#endif /* INCLUDE_ET_END */

    {0, END_TBL_END, NULL, 0, NULL, FALSE},
    };


#endif /* INCconfigNeth */

