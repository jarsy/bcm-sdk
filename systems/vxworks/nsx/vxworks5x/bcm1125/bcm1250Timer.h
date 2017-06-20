/* bcm1250Timer.h -- BCM1250 timer library header.  */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* $Id: bcm1250Timer.h,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,15nov01,agf  written.
*/

#ifndef __INCbcm1250TimerDevh
#define __INCbcm1250TimerDevh

#include "drv/timer/timerDev.h"

/* The following used in bcm1250TimerCfg.c. */
#define BCM1250_TIMER_0		0
#define BCM1250_TIMER_1		1
#define BCM1250_TIMER_2		2
#define BCM1250_TIMER_3		3

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	_ASMLANGUAGE

IMPORT	int	sysClkUnit;
IMPORT	int	sysClkIntVecNum;
IMPORT	int	sysAuxClkUnit;
IMPORT	int	sysAuxClkIntVecNum;

IMPORT  int	bcm1250TimerFreq;

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCbcm1250TimerDevh */
