/* sysDelay.c - BSP microsecond delay routine. */

/* Copyright 1984-2003 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: sysDelay.c,v 1.2 2011/07/21 16:14:28 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01c,27feb03,jmt  modified to code review comments
01b,27jun02,d_c  Add polled timer functions
01a,05mar02,d_c  Make common to all IDT BSPs.
*/

/*
DESCRIPTION

This file contains a routine to implement short delays (on the order
of microseconds), as well as functions for implementing polled
timeouts using the CP0 timer.

It is assumed that this file is included by sysLib.c.

*/

/* includes */

#include "vxWorks.h"
#include "config.h"
#include "sysLib.h"

IMPORT int      sysCountGet ();		/* define in sysALib.s of BSP */

/* defines */

/* Number of CPO timer counts in a microsecond. */

#define COUNTS_PER_USEC  (CPU_CLOCK_RATE / 1000000)

/* typedefs */

/* globals */

/* locals */

/* forward declarations */


/***************************************************************************
*
* sysUSecDelay - delay for at least a given number of microseconds;
*
* This function polls the CP0 count register for a given number of
* microseconds. It is used to implement very short delays.
*
* Note: The maximum possible delay is 2^32 / (CPU_CLOCK_RATE/1000000)
* microseconds. The minimum possible delay is the time it takes
* to execute this function with uSecs == 0, which is somewhat more
* than zero!
*
* If interrupts are not disabled, it is possible that you will
* delay quite a bit more than the requested time, if a number of interrupts
* occur just before the count register reaches the expiration value.
*
* There is a remote possibility that the count register will get
* reset by the timer module (timer/mipsR4kTimer.c). This could happen
* if anything pre-empts this delay loop and calls sysClkEnable or
* sysClkDisable .
*
* The CP0 Count registers increments at 1/2 the maximum instruction
* issue rate, (which == the CPU_CLOCK_RATE in this case). So the
* the increase in the count register per uSec is computed as:
* (cpu clock rate in cycles/second) / (1000000 uSecs in a second)
*
* RETURNS: N/A
*/

void sysUSecDelay
    (
    UINT32 uSecs		/* microseconds to delay */
    )
    {
    UINT32 expiredCount;	/* normalized count when delay expired */
    UINT32 normalize;		/* value used to normalize read count */
    UINT32 count;		/* the count read from CP0 */

    expiredCount = uSecs * COUNTS_PER_USEC;
    normalize = (UINT32) sysCountGet ();
    count = 0;

    while (count < expiredCount)
	{
	count = ((UINT32) sysCountGet ()) - normalize;
	}
    return;
    }

/***************************************************************************
*
* sysUSecTimoutStart - start a polled timeout
*
* This routine reads and returns the current value of the CP0 count
* register. This return value is used in subsequent calls to
* sysUSecTimeoutExpired to determine when a given number of
* microseconds have elapsed.
*
* RETURNS: startCount, the current value of the CP0 count register
*
* SEE ALSO: sysUSecTimoutExpired()
*/

UINT32 sysUSecTimoutStart (void)
    {
    return (UINT32) sysCountGet ();
    }


/***************************************************************************
*
* sysUSecTimoutExpired - return TRUE when timeout has expired
*
* This routine returns TRUE when at least <uSecs> microseconds have
* elapsed since the corresponding call to sysUSecTimeoutStart().
*
* RETURNS: TRUE when timeout as expired, FALSE othewise
*
* SEE ALSO: sysUSecTimeoutStart()
*/

BOOL sysUSecTimoutExpired
    (
    UINT32  startCount,     /* starting count from sysUSecTimeoutStart() */
    UINT32  uSecs           /* timeout in microseconds */
    )
    {
    UINT32  expiredCount;   /* normalized count when delay expired */
    BOOL    expired;        /* TRUE when timeout has expired */

    expiredCount = uSecs * COUNTS_PER_USEC;

    expired = ((( (UINT32) sysCountGet () ) - startCount) > expiredCount) ?
                 TRUE : FALSE;

    return expired;
    }



