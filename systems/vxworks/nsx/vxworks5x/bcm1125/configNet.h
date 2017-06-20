/* configNet.h - network configuration header */

/* Copyright 2001 Wind River Systems, Inc. */

/* $Id: configNet.h,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,15nov01,agf  written
*/
 
#ifndef INCnetConfigh
#define INCnetConfigh

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
    { 0, DEC_LOAD_FUNC, SBE_LOAD_STRING, DEC_BUFF_LOAN, 0, FALSE},
    { 1, DEC_LOAD_FUNC, SBE_LOAD_STRING, DEC_BUFF_LOAN, 1, FALSE},
#else              /* on-chip devices */
#ifdef BCM1250
  #ifdef BCM1250_CPU_0
    { 1, SBE_LOAD_FUNC, SBE_LOAD_STRING, SBE_BUFF_LOAN, 1, FALSE},
  #else /* BCM1250_CPU_1 */
    { 1, SBE_LOAD_FUNC, SBE_LOAD_STRING, SBE_BUFF_LOAN, 1, FALSE},
  #if 0
    { 2, SBE_LOAD_FUNC, SBE_LOAD_STRING, SBE_BUFF_LOAN, 2, FALSE},
  #endif
  #endif
#else  /* BCM1125 */
    { 0, SBE_LOAD_FUNC, SBE_LOAD_STRING, SBE_BUFF_LOAN, NULL, FALSE},
#endif
#endif
    { 0, END_TBL_END, NULL, 0, NULL, FALSE},
};

#endif /* INCnetConfigh */
