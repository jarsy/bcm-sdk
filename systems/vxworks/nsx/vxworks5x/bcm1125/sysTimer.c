/* sysTimer.c - bcm1250 timer configuration module */

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

/* $Id: sysTimer.c,v 1.3 2011/07/21 16:14:44 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01c,07dec01,agf  update DESCRIPTION field
01b,07dec01,agf  add Broadcom copyright notice
01a,09nov01,zmm  Add Ethernet address support.
*/

/*
DESCRIPTION
This file initilizes variables that configure the bcm1250 SCD general
timer number and interrupt vector number for the system clock and aux
clock.
*/

#include "vxWorks.h"
#include "config.h"

#include "bcm1250Timer.h"

#ifndef BCM1250_SHARED_SOURCE

int     sysClkUnit = BCM1250_TIMER_0;
int     sysClkIntVecNum = IV_INT5_VEC;
int     sysAuxClkUnit = BCM1250_TIMER_1;
int     sysAuxClkIntVecNum = IV_INT5_VEC;

#else /* BCM1250_SHARED_SOURCE */

#if defined(BCM1250_CPU_0)
int     sysClkUnit = BCM1250_TIMER_0;
int     sysClkIntVecNum = IV_INT5_VEC;
int     sysAuxClkUnit = BCM1250_TIMER_1;
int     sysAuxClkIntVecNum = IV_INT5_VEC;
#elif defined(BCM1250_CPU_1)
int     sysClkUnit = BCM1250_TIMER_2;
int     sysClkIntVecNum = IV_INT5_VEC;
int     sysAuxClkUnit = BCM1250_TIMER_3;
int     sysAuxClkIntVecNum = IV_INT5_VEC;
#else
#error "sysTimer: either BCM1250_CPU_0 or BCM1250_CPU_1 must be defined"
#endif

#endif /* BCM1250_SHARED_SOURCE */


/*
  Timer frequency.  Use '0' on real hardware.  For simulation, it should
  be approximately the number of times the timer decrements per second.
  This will be dependent on the actual CPU speed of the host running the
  simulator.

  10000 ends up with time in VxWorks passing at a fraction of real time
  (say, 10-30%, depending on host system), but doesn't cause the kernel
  to spend _too_ much time handling interrupts.
*/
#ifdef _SIMULATOR_ 
int	bcm1250TimerFreq = 10000;
#else
int	bcm1250TimerFreq = 0;
#endif
