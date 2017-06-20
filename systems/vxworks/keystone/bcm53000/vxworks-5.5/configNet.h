/* configNet.h - Network configuration header file */

/* Copyright 1984-2003 Wind River Systems, Inc. */

/* $Id: configNet.h,v 1.3 2011/07/21 16:14:24 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01e,19jun03,jmt   Modified to allow auto config using sysMipsPciEnum.c
01d,10may02,tlc  Add C++ header protection.
01c,16jul01,pes  Add CoE Copyright comment
01b,11apr01,pes  Initial Checkin/Development.
01a,03jan01,sru  created.
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

