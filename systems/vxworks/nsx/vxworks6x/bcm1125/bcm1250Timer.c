/* bcm1250Timer.c - bcm1250 timer library */

/* $Id: bcm1250Timer.c,v 1.5 2012/03/02 15:28:57 yaronm Exp $
 * Copyright (c) 2004-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
***********************************************************************
*/

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01e,01aug05,h_k  added #ifdef INCLUDE_AUX_CLK.
01d,15nov04,mdo  Documentation fixes for apigen
01c,18jul02,pgh  Use R4K library for sysClk.
01b,20jun02,pgh  Change path to bcm1250Lib.h.
01a,15nov01,agf  written
*/

/*
DESCRIPTION

This driver provides 3 main functions, system clock support, auxiliary
clock support. Timestamp timer support is currently UNSUPPORTED.  
If necessary, each function may be conditioned by a separate 
INCLUDE_ macro.  The timestamp function is always conditional upon 
the INCLUDE_TIMESTAMP macro.

(That feature is not currently supported by this driver.)

The macros SYS_CLK_RATE_MIN, SYS_CLK_RATE_MAX, AUX_CLK_RATE_MIN, and
AUX_CLK_RATE_MAX must be defined to provide parameter checking for the
sys[Aux]ClkRateSet() routines.

INCLUDE FILES:
timestampDev.h
*/

/* includes */

#include <vxWorks.h>
#include "config.h"
#include <intLib.h>
#include <sysLib.h>
#include "bcm1250IntLib.h"
#include "bcm1250Timer.h"
#include <drv/timer/timestampDev.h>
#include "bcm1250Lib.h"

/* defines */

#define	TIMER_FREQ	(bcm1250TimerFreq ? bcm1250TimerFreq : V_SCD_TIMER_FREQ)
LOCAL char *sysClkBase = NULL;
LOCAL FUNCPTR sysClkRoutine	= NULL; /* routine to call on clock tick */
LOCAL int sysClkArg		= (int)NULL; /* its argument */
LOCAL int sysClkRunning		= FALSE;
LOCAL int sysClkTicksPerSecond	= 60;

LOCAL char *sysAuxClkBase = NULL;
LOCAL FUNCPTR sysAuxClkRoutine	= NULL;
LOCAL int sysAuxClkArg		= (int)NULL;
LOCAL int sysAuxClkRunning	= FALSE;
LOCAL int sysAuxClkTicksPerSecond = 100;

LOCAL char *bcm1250TimerBase(int);
LOCAL int bcm1250TimerIntSource(int);

/*******************************************************************************
*
* bcm1250TimerBase - return a pointer to the SCD base address register of 
*                    the specified timer
*
* RETURNS: address of a timer's base register, NULL if a bogus timer is requested
*
*/
LOCAL char * bcm1250TimerBase
    (
    int unit
    )
    {

    switch (unit) 
        {
	case BCM1250_TIMER_0:
		return (char *)PHYS_TO_K1(A_SCD_TIMER_BASE(0));
	case BCM1250_TIMER_1:
		return (char *)PHYS_TO_K1(A_SCD_TIMER_BASE(1));
	case BCM1250_TIMER_2:
		return (char *)PHYS_TO_K1(A_SCD_TIMER_BASE(2));
	case BCM1250_TIMER_3:
		return (char *)PHYS_TO_K1(A_SCD_TIMER_BASE(3));

	}

    return NULL;
    }

/*******************************************************************************
*
* bcm1250TimerIntSource - provides the bcm1250 interrupt library's vector of
*                    the specified timer
*
* RETURNS: vector number or -1 if bogus timer is requested
*
*/
LOCAL int bcm1250TimerIntSource
    (
    int unit
    )
    {

    switch (unit) 
        {
        case BCM1250_TIMER_0:
            return ILVL_BCM1250_TIMER0;
        case BCM1250_TIMER_1:
            return ILVL_BCM1250_TIMER1;
        case BCM1250_TIMER_2:
            return ILVL_BCM1250_TIMER2;
        case BCM1250_TIMER_3:
            return ILVL_BCM1250_TIMER3;

	}

    return -1;
    }

/*******************************************************************************
*
* sysClkInt - interrupt level processing for system clock
*
* This routine handles an auxiliary clock interrupt.  It acknowledges the
* interrupt and calls the routine installed by sysClkConnect().
*
* RETURNS: N/A
*
*/

void sysClkInt 
    (
    int timerNum
    )
    {
    int myLock;

    myLock = intLock();

    /* ack the interrupt */

    MIPS3_SD(sysClkBase + R_SCD_TIMER_CFG,
		M_SCD_TIMER_ENABLE | M_SCD_TIMER_MODE_CONTINUOUS);

#if defined(USE_SIBYTE_INTR)
    SOC_INTR_INVOKE();
#endif

    /* call system clock service routine */

    if (sysClkRoutine != NULL)
        (* sysClkRoutine) (sysClkArg);

    intUnlock(myLock);
    }


/*******************************************************************************
*
* sysClkConnect - connect a routine to the system clock interrupt
*
* This routine specifies the interrupt service routine to be called at each
* clock interrupt.  Normally, it is called from usrRoot() in usrConfig.c to 
* connect usrClock() to the system clock interrupt.
*
* RETURN: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), usrClock(), sysClkEnable()
*/

STATUS sysClkConnect
    (
    FUNCPTR routine,	/* routine to be called at each clock interrupt */
    int arg		/* argument with which to call routine */
    )
    {
    static BOOL beenHere = FALSE;

    if (!beenHere)
	{
	beenHere = TRUE;

	sysClkBase = bcm1250TimerBase(sysClkUnit);

	sysHwInit2 ();

	}

    sysClkRoutine   = NULL;
    sysClkArg	    = arg;
    sysClkRoutine   = routine;

    return (OK);
    }


/*******************************************************************************
*
* sysClkDisable - turn off system clock interrupts
*
* This routine disables system clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysClkEnable()
*/

void sysClkDisable (void)
    {
    if (sysClkRunning)
	{
	  MIPS3_SD(sysClkBase + R_SCD_TIMER_CFG, 0);
	
	sysClkRunning = FALSE;
	}
    }


/*******************************************************************************
*
* sysClkEnable - turn on system clock interrupts
*
* This routine enables system clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysClkConnect(), sysClkDisable(), sysClkRateSet()
*/

void sysClkEnable (void)
    {
    static int connected = FALSE;

    /* make sure the clock is diabled */
    MIPS3_SD(sysClkBase + R_SCD_TIMER_CFG, 0);

    if (!connected)
	{
	  
	  connected = TRUE;
	  
	  /* connect handler */
	  bcm1250IntConnect( bcm1250TimerIntSource(sysClkUnit),
			    sysClkIntVecNum, 
			    sysClkInt, 0);
	  
	  /* enable the interrupt by unmasking it in the bcm1250 int ctrl */
	  bcm1250IntEnable( bcm1250TimerIntSource(sysClkUnit));
	}

    if (!sysClkRunning)
	{
	  MIPS3_SD(sysClkBase + R_SCD_TIMER_INIT,
		   M_SCD_TIMER_INIT & (TIMER_FREQ / sysClkTicksPerSecond));
	  MIPS3_SD(sysClkBase + R_SCD_TIMER_CFG,
	    M_SCD_TIMER_ENABLE | M_SCD_TIMER_MODE_CONTINUOUS);
	  sysClkRunning = TRUE;
	}
    }


/*******************************************************************************
*
* sysClkRateGet - get the system clock rate
*
* This routine returns the system clock rate.
*
* RETURNS: The number of ticks per second of the system clock.
*
* SEE ALSO: sysClkEnable(), sysClkRateSet()
*/

int sysClkRateGet (void)
    {
    return (sysClkTicksPerSecond);
    }


/*******************************************************************************
*
* sysClkRateSet - set the system clock rate
*
* This routine sets the interrupt rate of the system clock.
* It is called by usrRoot() in usrConfig.c.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be set.
*
* SEE ALSO: sysClkEnable(), sysClkRateGet()
*/

STATUS sysClkRateSet
    (
    int ticksPerSecond	    /* number of clock interrupts per second */
    )
    {

    /*  return ERROR if rate is not supported */

    if (ticksPerSecond < SYS_CLK_RATE_MIN || ticksPerSecond > SYS_CLK_RATE_MAX)
	return (ERROR);

    sysClkTicksPerSecond = ticksPerSecond;

    if (sysClkRunning)
	{
	sysClkDisable ();
	sysClkEnable ();
	}

    return (OK);
    }


#ifdef	INCLUDE_AUX_CLK
/* locals */

LOCAL char *sysAuxClkBase = NULL;
LOCAL FUNCPTR sysAuxClkRoutine	= NULL;
LOCAL int sysAuxClkArg		= (int)NULL;
LOCAL int sysAuxClkRunning	= FALSE;
LOCAL int sysAuxClkTicksPerSecond = 100;

LOCAL char *bcm1250TimerBase(int);
LOCAL int bcm1250TimerIntSource(int);


/*******************************************************************************
*
* sysAuxClkInt - handle an auxiliary clock interrupt
*
* This routine handles an auxiliary clock interrupt.  It acknowledges the
* interrupt and calls the routine installed by sysAuxClkConnect().
*
* RETURNS: N/A
*
* ERRNO
*/

void sysAuxClkInt (void)
    {
      int myLock;

      myLock = intLock();
      /* ack the interrupt */
      MIPS3_SD(sysAuxClkBase + R_SCD_TIMER_CFG,
	M_SCD_TIMER_ENABLE | M_SCD_TIMER_MODE_CONTINUOUS);

      /* call auxiliary clock service routine */

      if (sysAuxClkRoutine != NULL)
	(*sysAuxClkRoutine) (sysAuxClkArg);
      
      intUnlock(myLock);
    }

/*******************************************************************************
*
* sysAuxClkConnect - connect a routine to the auxiliary clock interrupt
*
* This routine specifies the interrupt service routine to be called at each
* auxiliary clock interrupt.  It does not enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* ERRNO
*
* SEE ALSO: intConnect(), sysAuxClkEnable()
*/

STATUS sysAuxClkConnect
    (
    FUNCPTR routine,    /* routine called at each aux clock interrupt */
    int arg             /* argument to auxiliary clock interrupt routine */
    )
    {
    sysAuxClkBase = bcm1250TimerBase(sysAuxClkUnit);

    sysAuxClkRoutine	= NULL;
    sysAuxClkArg	= arg;
    sysAuxClkRoutine	= routine;

    return (OK);
    }

/*******************************************************************************
*
* sysAuxClkDisable - turn off auxiliary clock interrupts
*
* This routine disables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* ERRNO
*
* SEE ALSO: sysAuxClkEnable()
*/

void sysAuxClkDisable (void)
    {
    if (sysAuxClkRunning)
        {
          MIPS3_SD(sysAuxClkBase + R_SCD_TIMER_CFG, 0);

	  sysAuxClkRunning = FALSE;
        }
    }

/*******************************************************************************
*
* sysAuxClkEnable - turn on auxiliary clock interrupts
*
* This routine enables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* ERRNO
*
* SEE ALSO: sysAuxClkConnect(), sysAuxClkDisable(), sysAuxClkRateSet()
*/

void sysAuxClkEnable (void)
    {
    static int connected = FALSE;

    /* make sure the clock is disabled */
    MIPS3_SD(sysAuxClkBase + R_SCD_TIMER_CFG, 0);

    if (!connected)
	{
	/*  connect sysAuxClkInt to appropriate interrupt */
	connected = TRUE;

	bcm1250IntConnect( bcm1250TimerIntSource(sysAuxClkUnit),
			  sysAuxClkIntVecNum, 
			  sysAuxClkInt, 0);
	
	/* enable the interrupt by unmasking it in the bcm1250 int ctrl */
	bcm1250IntEnable( bcm1250TimerIntSource(sysAuxClkUnit));
	}

    if (!sysAuxClkRunning)
        {
	/*
	 *  start auxiliary timer interrupts at a
	 * at a frequency of sysAuxClkTicksPerSecond.
	 */
	  MIPS3_SD(sysAuxClkBase + R_SCD_TIMER_INIT,
		   M_SCD_TIMER_INIT & (TIMER_FREQ / sysAuxClkTicksPerSecond));
	  MIPS3_SD(sysAuxClkBase + R_SCD_TIMER_CFG,
	    M_SCD_TIMER_ENABLE | M_SCD_TIMER_MODE_CONTINUOUS);

	  sysAuxClkRunning = TRUE;
	}
    }

/*******************************************************************************
*
* sysAuxClkRateGet - get the auxiliary clock rate
*
* This routine returns the interrupt rate of the auxiliary clock.
*
* RETURNS: The number of ticks per second of the auxiliary clock.
*
* ERRNO
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateSet()
*/

int sysAuxClkRateGet (void)
    {
    return (sysAuxClkTicksPerSecond);
    }

/*******************************************************************************
*
* sysAuxClkRateSet - set the auxiliary clock rate
*
* This routine sets the interrupt rate of the auxiliary clock.  It does not
* enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be set.
*
* ERRNO
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateGet()
*/

STATUS sysAuxClkRateSet
    (
    int ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    /*  return ERROR if rate is not supported */

    if (ticksPerSecond < AUX_CLK_RATE_MIN || ticksPerSecond > AUX_CLK_RATE_MAX)
        return(ERROR);

    sysAuxClkTicksPerSecond = ticksPerSecond;

    if (sysAuxClkRunning)
	{
	sysAuxClkDisable ();
	sysAuxClkEnable ();
	}

    return(OK);
    }

/*******************************************************************************
*
* bcm1250TimerBase - get the address of the timer base register  
*
* This routine returns a pointer to the SCD base address register of 
* the specified timer.
*
* RETURNS: address of a timer's base register, NULL if a bogus timer is requested
*
* ERRNO
*/
LOCAL char * bcm1250TimerBase
    (
    int unit
    )
    {

    switch (unit) 
        {
	case BCM1250_TIMER_0:
            return(char *)PHYS_TO_K1(A_SCD_TIMER_BASE(0));
	case BCM1250_TIMER_1:
            return(char *)PHYS_TO_K1(A_SCD_TIMER_BASE(1));
	case BCM1250_TIMER_2:
            return(char *)PHYS_TO_K1(A_SCD_TIMER_BASE(2));
	case BCM1250_TIMER_3:
            return(char *)PHYS_TO_K1(A_SCD_TIMER_BASE(3));

	}

    return NULL;
    }

/*******************************************************************************
*
* bcm1250TimerIntSource - get the vector of the specified timer
*
* This routine provides the bcm1250 interrupt library's vector of
*                    the specified timer
*
* RETURNS: vector number or -1 if bogus timer is requested
*
* ERRNO
*/
LOCAL int bcm1250TimerIntSource
    (
    int unit
    )
    {

    switch (unit) 
        {
        case BCM1250_TIMER_0:
            return ILVL_BCM1250_TIMER0;
        case BCM1250_TIMER_1:
            return ILVL_BCM1250_TIMER1;
        case BCM1250_TIMER_2:
            return ILVL_BCM1250_TIMER2;
        case BCM1250_TIMER_3:
            return ILVL_BCM1250_TIMER3;

	}

    return -1;
    }
#endif	/* INCLUDE_AUX_CLK */
