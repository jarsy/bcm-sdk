/* configNet.h - network configuration header */

/* Copyright 2005 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: configNet.h,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01d,28jul05,rlg  spr 109086 and 109090 compiler warnings cleanup
01c,11apr05,kab  fix comments for apiGen (SPR 107842)
01b,10may02,tlc  Add C++ header protection.
01a,15nov01,agf  written
*/
 
#ifndef INCnetConfigh
#define INCnetConfigh

#ifdef __cplusplus
extern "C" {
#endif

#include "vxWorks.h"
#include "end.h"
#include "config.h"

#define SBE_LOAD_FUNC   sysBcm1250MacEndLoad
#define SBE_BUFF_LOAN   0
#define SBE_LOAD_STRING ""

IMPORT END_OBJ* SBE_LOAD_FUNC (char*, void*);

#define DEC_LOAD_FUNC   sysDec21x40EndLoad
#define DEC_BUFF_LOAN   1

IMPORT END_OBJ* DEC_LOAD_FUNC (char*, void*);

END_TBL_ENTRY endDevTbl [] =
{
#ifdef INCLUDE_DEC  /* Tulip PCI devices */
    { 0, DEC_LOAD_FUNC, "", DEC_BUFF_LOAN, NULL, FALSE},
    { 1, DEC_LOAD_FUNC, "", DEC_BUFF_LOAN, NULL, FALSE},
#else              /* on-chip devices */
#ifdef BCM1250_CPU_0
    { 0, SBE_LOAD_FUNC, "", SBE_BUFF_LOAN, NULL, FALSE},
#else /* BCM1250_CPU_1 */
    { 0, SBE_LOAD_FUNC, "", SBE_BUFF_LOAN, NULL, FALSE},
    { 1, SBE_LOAD_FUNC, "", SBE_BUFF_LOAN, NULL, FALSE},
#endif
#endif
    { 0, END_TBL_END, NULL, 0, NULL, FALSE},
};

#ifdef __cplusplus
}
#endif
#endif /* INCnetConfigh */
