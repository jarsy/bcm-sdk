/* ppcDecTimer.c - PowerPC decrementer timer library */

/* Copyright 1984-1999 Wind River Systems, Inc. */

#include "copyright_wrs.h"

/* $Id: ppcDecTimer.c,v 1.2 2011/07/21 16:14:08 yshtil Exp $
modification history
--------------------
01r,13apr00,dat  removed sysHwInit2, other T3 fixes
01q,08mar99,jkf  fixed build warning, line 108 (t3 only)
01p,01apr99,jkf  sysTimestampEnable now starts the system clock rather
                 than returning error.  Timer now "stops" when disabled.
01o,03mar98,jgn  Re-enable interrupts during system clock tick processing
		 (SPR #10000)
01n,04nov96,tam  renamed MEMORY_BUS_SPEED to DEC_CLOCK_FREQ, BUS_CLK_TO_INC
		 to DEC_CLK_TO_INC and DEFAULT_BUS_SPEED to DEFAULT_DEC_CLK_FREQ
		 (spr #7423).
01m,29oct96,wlf  doc: cleanup.
01l,31jul96,dat  added PPC_TMR_RATE_SET_ADJUST
01k,22jul96,tam  cleanup. added timestamp support. Changed CPU_SPEED macro to 
		 MEMORY_BUS_SPEED.
01j,17jun96,tpr  optimized sysClkInt() assembly code.
01i,14may96,tam  fixed drift in sysClkInt. Changed DEFAULT_CPU_SPEED to 
		 33333333 (33.33 Mhz)
01h,11mar96,ms   fixed roundoff in sysClkInt.
01g,12feb96,tpr  reworked.
01f,23oct95,kvk  cleanup.
01e,22may95,caf  added conditional compile for PowerPC 601.
01d,02may95,caf  cleanup.
01c,27apr95,caf  removed vxDecEnable() and PPC60x_TIMER_INT_VECTOR,
		 changed name to ppcDecTimer.c.
01b,25jan95,vin	 cleanup.
01a,20jan95,kvk  written.
*/

/*
DESCRIPTION
This library provides PowerPC decrementer timer routines.
This library handles both the system clock and the auxiliary clock plus 
and timestamp functions.  However, the auxiliary clock functions have no 
effect.

The macro DEC_CLOCK_FREQ (frequency of the decrementer input clock) should 
be defined before using this module. The macro DEC_CLK_TO_INC (ratio between 
the number of decrementer input clock cycles and one counter increment) can 
be redefined prior to #including this file into sysLib.c.  The macros 
SYS_CLK_RATE_MIN and SYS_CLK_RATE_MAX must be defined to provide parameter 
checking for sysClkRateSet().

To include the timestamp timer facility, the macro INCLUDE_TIMESTAMP must be
defined. Note that changing the system clock rate will affect the timestamp
timer period, which can be read by calling sysTimestampPeriod().

If dynamic bus clock speed calculation is needed, the BSP can define 
the macro PPC_TMR_RATE_SET_ADJUST to be a call to the needed routine.

This macro, if defined, will be executed during each call to sysClkRateSet().
PPC_TMR_RATE_SET_ADJUST is usually not defined.

    e.g. Assuming sysClkRateAdjust can compute a correct value
    for sysDecClkFrequency.

    #define PPC_TMR_RATE_SET_ADJUST \
	sysClkRateAdjust (&sysDecClkFrequency)



INCLUDE FILES: ppcDecTimer.h
   
SEE ALSO:
.pG "Configuration"
*/

/* includes */

#include "arch/ppc/vxPpcLib.h"
#include "drv/timer/ppcDecTimer.h"
#include "drv/timer/timestampDev.h"

/* local defines */

#ifndef	DEC_CLK_TO_INC
#define	DEC_CLK_TO_INC		4		/* # bus clks per increment */
#endif

#define DEFAULT_DEC_CLK_FREQ	33333333	/* 33.33 Mhz default */

#if	(CPU == PPC601)
#   define	DEC_SHIFT 7		/* 7 low-order bits aren't used */
#else
#   define	DEC_SHIFT 0
#endif	/* CPU == PPC601 */

#ifndef CPU_INT_UNLOCK
#    define  CPU_INT_UNLOCK(x) (intUnlock(x))
#endif

#ifndef CPU_INT_LOCK
#    define  CPU_INT_LOCK(x) (*x = intLock())
#endif

/* extern declarations */

IMPORT STATUS	excIntConnect (VOIDFUNCPTR *, VOIDFUNCPTR);

/* Locals */
LOCAL int 	sysClkTicksPerSecond 	= 60;	  /* default 60 ticks/second */
LOCAL FUNCPTR	sysClkRoutine		= NULL;
LOCAL int	sysClkArg		= 0;
LOCAL int       decCountVal		= 10000000;	/* default dec value */
LOCAL BOOL	sysClkRunning 		= FALSE;
LOCAL int 	sysDecClkFrequency	= DEFAULT_DEC_CLK_FREQ/DEC_CLK_TO_INC;


#ifdef	INCLUDE_TIMESTAMP
LOCAL BOOL	sysTimestampRunning  	= FALSE;   /* timestamp running flag */
LOCAL volatile int decTimerOffValue	= 0;
#endif	/* INCLUDE_TIMESTAMP */

/*******************************************************************************
*
* sysClkInt - clock interrupt handler
*
* This routine handles the clock interrupt on the PowerPC architecture. It is
* attached to the decrementer vector by the routine sysClkConnect().
*
* RETURNS : N/A
*/

LOCAL void sysClkInt (void)
    {
    /*
     * The PowerPC decrementer doesn't reload the value by itself. The reload
     * need to be performed in this handler. The reload value should be 
     * adjusted each time because the time spent between the exception
     * generation and the moment the register is reloaded changes.
     * By reading the decrementer we obtain the time spent between the 
     * two events and can adjust the value to reload. This is done in assembly
     * language in order to minimize time spent between reading and writing
     * to the decrementer register in order to maximize system clock accuracy.
     */

__asm__ ("
    mfdec   	3	
sysClkIntLoop:
    add. 	3, %0, 3
    ble		sysClkIntLoop		/* check if we missed tick(s) */
    mtdec	3"
    :					/* no output operands  */
    : "r" (decCountVal)			/* input operand, %0 = decCountVal */
    : "3", "cc"/* side-effects: r3 clobbered, 'condition code' is clobbered */
    );

    /* Unlock interrupts during decrementer processing */

    CPU_INT_UNLOCK (_PPC_MSR_EE);

    /* execute the system clock routine */

    if (sysClkRunning && (sysClkRoutine != NULL))
	(*(FUNCPTR) sysClkRoutine) (sysClkArg);

    }

/*******************************************************************************
*
* sysClkConnect - connect a routine to the system clock interrupt
*
* This routine specifies the interrupt service routine to be called at each
* clock interrupt.  Normally, it is called from usrRoot() in usrConfig.c to
* connect usrClock() to the system clock interrupt.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), usrClock(), sysClkEnable()
*/
    
STATUS sysClkConnect
    (
    FUNCPTR 	routine,	/* routine to connect */
    int 	arg		/* argument for the routine */
    )
    {
#ifdef	DEC_CLOCK_FREQ 
    sysDecClkFrequency = DEC_CLOCK_FREQ / DEC_CLK_TO_INC;
#endif	/* DEC_CLOCK_FREQ */

    /* connect the routine to the decrementer exception */

    excIntConnect ((VOIDFUNCPTR *) _EXC_OFF_DECR, (VOIDFUNCPTR) sysClkInt);
    
    sysClkRoutine	= routine;
    sysClkArg		= arg;

    return (OK);
    }

/******************************************************************************
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
    if (!sysClkRunning)
	{
	sysClkRunning = TRUE;
	vxDecSet (decCountVal);
	}
    }

/******************************************************************************
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
	sysClkRunning = FALSE;
    }

/******************************************************************************
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

/******************************************************************************
*
* sysClkRateSet - set the system clock rate
*
* This routine sets the interrupt rate of the system clock.  It is called
* by usrRoot() in usrConfig.c.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be set.
*
* SEE ALSO: sysClkEnable(), sysClkRateGet()
*/
   
STATUS sysClkRateSet
    (
    int 	ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    if (ticksPerSecond < SYS_CLK_RATE_MIN || ticksPerSecond > SYS_CLK_RATE_MAX)
        return (ERROR);

    /* save the clock speed */

    sysClkTicksPerSecond = ticksPerSecond;

    /* Calibrate the clock, if needed. */

#ifdef PPC_TMR_RATE_SET_ADJUST
    PPC_TMR_RATE_SET_ADJUST;
#endif

    /* 
     * compute the value to load in the decrementer. The new value will be
     * load in the decrementer after the end of the current period
     */

    decCountVal = sysDecClkFrequency / ticksPerSecond;

    return (OK);
    }


#ifdef  INCLUDE_TIMESTAMP

/*******************************************************************************
*
* sysTimestampConnect - connect a user routine to a timestamp timer interrupt
*
* This routine specifies the user interrupt routine to be called at each
* timestamp timer interrupt.  
*
* NOTE: This routine has no effect, since the CPU decrementer has no
* timestamp timer interrupt.
*
* RETURNS: ERROR, always.
*/

STATUS sysTimestampConnect
    (
    FUNCPTR routine,    /* routine called at each timestamp timer interrupt */
    int arg             /* argument with which to call routine */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysTimestampEnable - enable a timestamp timer interrupt
*
* This routine enables timestamp timer interrupts and resets the counter.
*
* NOTE: This routine has no effect, since the CPU decrementer has no
* timestamp timer interrupt.
*
* RETURNS: OK, always.
*
* SEE ALSO: sysTimestampDisable()
*/

STATUS sysTimestampEnable (void)
   {
   if (sysTimestampRunning)
      {
      return (OK);
      }

   if (!sysClkRunning)          /* don't have any auxiliary clock ! */
	{
	sysClkEnable();		/* jkf changed from return ERROR */
	}

   sysTimestampRunning = TRUE;

   return (OK);
   }

/*******************************************************************************
*
* sysTimestampDisable - disable a timestamp timer interrupt
*
* This routine disables timestamp timer interrupts.
*
* NOTE: This routine has no effect, since the CPU decrementer has no
* timestamp timer interrupt.
*
* RETURNS: OK, always.
*
* SEE ALSO: sysTimestampEnable()
*/

STATUS sysTimestampDisable (void)
    {
    if (sysTimestampRunning)
        sysTimestampRunning = FALSE;
    decTimerOffValue = vxDecGet ();    
    return (OK);
    }

/*******************************************************************************
*
* sysTimestampPeriod - get the period of a timestamp timer 
*
* This routine gets the period of the timestamp timer, in ticks.  The
* period, or terminal count, is the number of ticks to which the timestamp
* timer counts before rolling over and restarting the counting process.
*
* RETURNS: The period of the timestamp timer in counter ticks.
*/

UINT32 sysTimestampPeriod (void)
    {
    /*
     * The period of the timestamp depends on the clock rate of the on-chip
     * timer (ie the Decrementer reload value).
     */
    
    return (decCountVal);
    }

/*******************************************************************************
*
* sysTimestampFreq - get a timestamp timer clock frequency
*
* This routine gets the frequency of the timer clock, in ticks per 
* second.  The rate of the timestamp timer is set explicitly by the 
* hardware and typically cannot be altered.
*
* NOTE: Because the PowerPC decrementer clock serves as the timestamp timer,
* the decrementer clock frequency is also the timestamp timer frequency.
*
* RETURNS: The timestamp timer clock frequency, in ticks per second.
*/

UINT32 sysTimestampFreq (void)
    {
    return (sysDecClkFrequency);
    }

/*******************************************************************************
*
* sysTimestamp - get a timestamp timer tick count
*
* This routine returns the current value of the timestamp timer tick counter.
* The tick count can be converted to seconds by dividing it by the return of
* sysTimestampFreq().
*
* This routine should be called with interrupts locked.  If interrupts are
* not locked, sysTimestampLock() should be used instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestampFreq(), sysTimestampLock()
*/

UINT32 sysTimestamp (void)
    {
    if (sysTimestampRunning)
	return (decCountVal - (INT32) vxDecGet());
    else
	return (decTimerOffValue);
    }

/*******************************************************************************
*
* sysTimestampLock - lock interrupts and get the timestamp timer tick count
*
* This routine locks interrupts when the tick counter must be stopped 
* in order to read it or when two independent counters must be read.  
* It then returns the current value of the timestamp timer tick
* counter.
* 
* The tick count can be converted to seconds by dividing it by the return of
* sysTimestampFreq().
*
* If interrupts are already locked, sysTimestamp() should be
* used instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestampFreq(), sysTimestamp()
*/

UINT32 sysTimestampLock (void)
    {
    UINT32 currentDecValue;
    int oldLevel;

    oldLevel = intLock ();                              /* LOCK INTERRUPT */
    currentDecValue = vxDecGet();
    intUnlock (oldLevel);                               /* UNLOCK INTERRUPT */

    if (sysTimestampRunning)
	return (decCountVal - currentDecValue);
    else
	return (decTimerOffValue);
    }

#endif  /* INCLUDE_TIMESTAMP */

