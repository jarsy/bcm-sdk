/* m85xxTimer.c - PowerPC Book E timer library */

/* $Id: m85xxTimer.c,v 1.2 2011/07/21 16:14:13 yshtil Exp $
****************************************************************************
   This source and object code has been made available to you by IBM on an
   AS-IS basis.

   IT IS PROVIDED WITHOUT WARRANTY OF ANY KIND, INCLUDING THE WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE OR OF NONINFRINGEMENT
   OF THIRD PARTY RIGHTS.  IN NO EVENT SHALL IBM OR ITS LICENSORS BE LIABLE
   FOR INCIDENTAL, CONSEQUENTIAL OR PUNITIVE DAMAGES.  IBMS OR ITS LICENSORS
   DAMAGES FOR ANY CAUSE OF ACTION, WHETHER IN CONTRACT OR IN TORT, AT LAW OR
   AT EQUITY, SHALL BE LIMITED TO A MAXIMUM OF $1,000 PER LICENSE.  No license
   under IBM patents or patent applications is to be implied by the copyright
   license.

   Any user of this software should understand that neither IBM nor its
   licensors will be responsible for any consequences resulting from the use
   of this software.

   Any person who transfers this source code or any derivative work must
   include the IBM copyright notice, this paragraph, and the preceding two
   paragraphs in the transferred software.

   Any person who transfers this object code or any derivative work must
   include the IBM copyright notice in the transferred software.

   COPYRIGHT   I B M   CORPORATION 2000
   LICENSED MATERIAL  -  PROGRAM PROPERTY OF  I B M"

****************************************************************************
\NOMANUAL
*/

/* Copyright 1984-2004 Wind River Systems, Inc. */

/*
modification history
--------------------
01f,22oct04,dtr  Remove gnu compiler warning.
01e,01sep04,pch  fix masks in wdtTable
01d,31aug04,mdo  Documentation fixes for apigen
01c,05dec01,mcg  remove #include of ppc403Timer.c, used ppc440.h instead
01b,08nov01,jtp  TSR fields now cleared correctly
01a,24oct01,jtp  checked in. mcg ported this from ppc405Timer.c version 01h
*/

/*
DESCRIPTION
This library provides PowerPC Book E Timer routines. This library handles
the system clock, the auxiliary clock and timestamp functions. To
include the timestamp timer facility, the macro INCLUDE_TIMESTAMP must
be defined.

The macros SYS_CLK_RATE_MIN, SYS_CLK_RATE_MAX, AUX_CLK_RATE_MIN, and
AUX_CLK_RATE_MAX must be defined to provide parameter checking for
sysClkRateSet().

The BSP is responsible for connecting the interrupt handlers,
sysClkInt(), and sysAuxClkInt(), to the appropriate vectors.

The BSP must also initialize a global variable sysTimerClkFreq prior to using
this driver to indicate what frequency the timebase is running.  This value
is often calculated at run time, especially when an internal timer clock source
is being used.

INCLUDE FILES:

SEE ALSO:
.pG "Configuration"
*/

/* includes */
#include "vxWorks.h"
#include "vxLib.h"
#include "intLib.h"
#include "drv/timer/timestampDev.h"

/* local defines */

/* extern declarations */

IMPORT UINT32 sysTimerClkFreq;

/* typedefs */

typedef struct
    {
    unsigned long long fitPeriod;         /* Fixed Interval Timer periods */
    UINT32   fpMask;                      /* corresponding TCR mask       */
    } FIT_PERIOD;

typedef struct
    {
    unsigned long long wdtPeriod;         /* Watchdog Timer periods       */
    UINT32   fpMask;                      /* corresponding TCR mask       */
    } WDT_PERIOD;

/* Locals */

LOCAL FIT_PERIOD fitTable[] =                   /* available FIT periods */
    {
	{ ((unsigned long long) 1 << 0), 0x0301e000 },	/* TBL[63] */
	{ ((unsigned long long) 1 << 1), 0x0201e000 },	/* TBL[62] */
	{ ((unsigned long long) 1 << 2), 0x0101e000 },	/* TBL[61] */
	{ ((unsigned long long) 1 << 3), 0x0001e000 },	/* TBL[60] */
	{ ((unsigned long long) 1 << 4), 0x0301c000 },	/* TBL[59] */
	{ ((unsigned long long) 1 << 5), 0x0201c000 },	/* TBL[58] */
	{ ((unsigned long long) 1 << 6), 0x0101c000 },	/* TBL[57] */
	{ ((unsigned long long) 1 << 7), 0x0001c000 },	/* TBL[56] */
	{ ((unsigned long long) 1 << 8), 0x0301a000 },	/* TBL[55] */
	{ ((unsigned long long) 1 << 9), 0x0201a000 },	/* TBL[54] */
	{ ((unsigned long long) 1 << 10), 0x0101a000 },	/* TBL[53] */
	{ ((unsigned long long) 1 << 11), 0x0001a000 },	/* TBL[52] */
	{ ((unsigned long long) 1 << 12), 0x03018000 },	/* TBL[51] */
	{ ((unsigned long long) 1 << 13), 0x02018000 },	/* TBL[50] */
	{ ((unsigned long long) 1 << 14), 0x01018000 },	/* TBL[49] */
	{ ((unsigned long long) 1 << 15), 0x00018000 },	/* TBL[48] */
	{ ((unsigned long long) 1 << 16), 0x03016000 },	/* TBL[47] */
	{ ((unsigned long long) 1 << 17), 0x02016000 },	/* TBL[46] */
	{ ((unsigned long long) 1 << 18), 0x01016000 },	/* TBL[45] */
	{ ((unsigned long long) 1 << 19), 0x00016000 },	/* TBL[44] */
	{ ((unsigned long long) 1 << 20), 0x03014000 },	/* TBL[43] */
	{ ((unsigned long long) 1 << 21), 0x02014000 },	/* TBL[42] */
	{ ((unsigned long long) 1 << 22), 0x01014000 },	/* TBL[41] */
	{ ((unsigned long long) 1 << 23), 0x00014000 },	/* TBL[40] */
	{ ((unsigned long long) 1 << 24), 0x03012000 },	/* TBL[39] */
	{ ((unsigned long long) 1 << 25), 0x02012000 },	/* TBL[38] */
	{ ((unsigned long long) 1 << 26), 0x01012000 },	/* TBL[37] */
	{ ((unsigned long long) 1 << 27), 0x00012000 },	/* TBL[36] */
	{ ((unsigned long long) 1 << 28), 0x03010000 },	/* TBL[35] */
	{ ((unsigned long long) 1 << 29), 0x02010000 },	/* TBL[34] */
	{ ((unsigned long long) 1 << 30), 0x01010000 },	/* TBL[33] */
	{ ((unsigned long long) 1 << 31), 0x00010000 },	/* TBL[32] */
	{ ((unsigned long long) 1 << 32), 0x0300e000 },	/* TBU[31] */
	{ ((unsigned long long) 1 << 33), 0x0200e000 },	/* TBU[30] */
	{ ((unsigned long long) 1 << 34), 0x0100e000 },	/* TBU[29] */
	{ ((unsigned long long) 1 << 35), 0x0000e000 },	/* TBU[28] */
	{ ((unsigned long long) 1 << 36), 0x0300c000 },	/* TBU[27] */
	{ ((unsigned long long) 1 << 37), 0x0200c000 },	/* TBU[26] */
	{ ((unsigned long long) 1 << 38), 0x0100c000 },	/* TBU[25] */
	{ ((unsigned long long) 1 << 39), 0x0000c000 },	/* TBU[24] */
	{ ((unsigned long long) 1 << 40), 0x0300a000 },	/* TBU[23] */
	{ ((unsigned long long) 1 << 41), 0x0200a000 },	/* TBU[22] */
	{ ((unsigned long long) 1 << 42), 0x0100a000 },	/* TBU[21] */
	{ ((unsigned long long) 1 << 43), 0x0000a000 },	/* TBU[20] */
	{ ((unsigned long long) 1 << 44), 0x03008000 },	/* TBU[19] */
	{ ((unsigned long long) 1 << 45), 0x02008000 },	/* TBU[18] */
	{ ((unsigned long long) 1 << 46), 0x01008000 },	/* TBU[17] */
	{ ((unsigned long long) 1 << 47), 0x00008000 },	/* TBU[16] */
	{ ((unsigned long long) 1 << 48), 0x03006000 },	/* TBU[15] */
	{ ((unsigned long long) 1 << 49), 0x02006000 },	/* TBU[14] */
	{ ((unsigned long long) 1 << 50), 0x01006000 },	/* TBU[13] */
	{ ((unsigned long long) 1 << 51), 0x00006000 },	/* TBU[12] */
	{ ((unsigned long long) 1 << 52), 0x03004000 },	/* TBU[11] */
	{ ((unsigned long long) 1 << 53), 0x02004000 },	/* TBU[10] */
	{ ((unsigned long long) 1 << 54), 0x01004000 },	/* TBU[9] */
	{ ((unsigned long long) 1 << 55), 0x00004000 },	/* TBU[8] */
	{ ((unsigned long long) 1 << 56), 0x03002000 },	/* TBU[7] */
	{ ((unsigned long long) 1 << 57), 0x02002000 },	/* TBU[6] */
	{ ((unsigned long long) 1 << 58), 0x01002000 },	/* TBU[5] */
	{ ((unsigned long long) 1 << 59), 0x00002000 },	/* TBU[4] */
	{ ((unsigned long long) 1 << 60), 0x03000000 },	/* TBU[3] */
	{ ((unsigned long long) 1 << 61), 0x02000000 },	/* TBU[2] */
	{ ((unsigned long long) 1 << 62), 0x01000000 },	/* TBU[1] */
	{ ((unsigned long long) 1 << 63), 0x00000000 }	/* TBU[0] */
    };

LOCAL WDT_PERIOD wdtTable[] =                   /* available WDT periods */
    {
	{ ((unsigned long long) 1 << 0),  0xc01e0000 },	/* TBL[63] */
	{ ((unsigned long long) 1 << 1),  0x801e0000 },	/* TBL[62] */
	{ ((unsigned long long) 1 << 2),  0x401e0000 },	/* TBL[61] */
	{ ((unsigned long long) 1 << 3),  0x001e0000 },	/* TBL[60] */
	{ ((unsigned long long) 1 << 4),  0xc01c0000 },	/* TBL[59] */
	{ ((unsigned long long) 1 << 5),  0x801c0000 },	/* TBL[58] */
	{ ((unsigned long long) 1 << 6),  0x401c0000 },	/* TBL[57] */
	{ ((unsigned long long) 1 << 7),  0x001c0000 },	/* TBL[56] */
	{ ((unsigned long long) 1 << 8),  0xc01a0000 },	/* TBL[55] */
	{ ((unsigned long long) 1 << 9),  0x801a0000 },	/* TBL[54] */
	{ ((unsigned long long) 1 << 10), 0x401a0000 },	/* TBL[53] */
	{ ((unsigned long long) 1 << 11), 0x001a0000 },	/* TBL[52] */
	{ ((unsigned long long) 1 << 12), 0xc0180000 },	/* TBL[51] */
	{ ((unsigned long long) 1 << 13), 0x80180000 },	/* TBL[50] */
	{ ((unsigned long long) 1 << 14), 0x40180000 },	/* TBL[49] */
	{ ((unsigned long long) 1 << 15), 0x00180000 },	/* TBL[48] */
	{ ((unsigned long long) 1 << 16), 0xc0160000 },	/* TBL[47] */
	{ ((unsigned long long) 1 << 17), 0x80160000 },	/* TBL[46] */
	{ ((unsigned long long) 1 << 18), 0x40160000 },	/* TBL[45] */
	{ ((unsigned long long) 1 << 19), 0x00160000 },	/* TBL[44] */
	{ ((unsigned long long) 1 << 20), 0xc0140000 },	/* TBL[43] */
	{ ((unsigned long long) 1 << 21), 0x80140000 },	/* TBL[42] */
	{ ((unsigned long long) 1 << 22), 0x40140000 },	/* TBL[41] */
	{ ((unsigned long long) 1 << 23), 0x00140000 },	/* TBL[40] */
	{ ((unsigned long long) 1 << 24), 0xc0120000 },	/* TBL[39] */
	{ ((unsigned long long) 1 << 25), 0x80120000 },	/* TBL[38] */
	{ ((unsigned long long) 1 << 26), 0x40120000 },	/* TBL[37] */
	{ ((unsigned long long) 1 << 27), 0x00120000 },	/* TBL[36] */
	{ ((unsigned long long) 1 << 28), 0xc0100000 },	/* TBL[35] */
	{ ((unsigned long long) 1 << 29), 0x80100000 },	/* TBL[34] */
	{ ((unsigned long long) 1 << 30), 0x40100000 },	/* TBL[33] */
	{ ((unsigned long long) 1 << 31), 0x00100000 },	/* TBL[32] */
	{ ((unsigned long long) 1 << 32), 0xc00e0000 },	/* TBU[31] */
	{ ((unsigned long long) 1 << 33), 0x800e0000 },	/* TBU[30] */
	{ ((unsigned long long) 1 << 34), 0x400e0000 },	/* TBU[29] */
	{ ((unsigned long long) 1 << 35), 0x000e0000 },	/* TBU[28] */
	{ ((unsigned long long) 1 << 36), 0xc00c0000 },	/* TBU[27] */
	{ ((unsigned long long) 1 << 37), 0x800c0000 },	/* TBU[26] */
	{ ((unsigned long long) 1 << 38), 0x400c0000 },	/* TBU[25] */
	{ ((unsigned long long) 1 << 39), 0x000c0000 },	/* TBU[24] */
	{ ((unsigned long long) 1 << 40), 0xc00a0000 },	/* TBU[23] */
	{ ((unsigned long long) 1 << 41), 0x800a0000 },	/* TBU[22] */
	{ ((unsigned long long) 1 << 42), 0x400a0000 },	/* TBU[21] */
	{ ((unsigned long long) 1 << 43), 0x000a0000 },	/* TBU[20] */
	{ ((unsigned long long) 1 << 44), 0xc0080000 },	/* TBU[19] */
	{ ((unsigned long long) 1 << 45), 0x80080000 },	/* TBU[18] */
	{ ((unsigned long long) 1 << 46), 0x40080000 },	/* TBU[17] */
	{ ((unsigned long long) 1 << 47), 0x00080000 },	/* TBU[16] */
	{ ((unsigned long long) 1 << 48), 0xc0060000 },	/* TBU[15] */
	{ ((unsigned long long) 1 << 49), 0x80060000 },	/* TBU[14] */
	{ ((unsigned long long) 1 << 50), 0x40060000 },	/* TBU[13] */
	{ ((unsigned long long) 1 << 51), 0x00060000 },	/* TBU[12] */
	{ ((unsigned long long) 1 << 52), 0xc0040000 },	/* TBU[11] */
	{ ((unsigned long long) 1 << 53), 0x80040000 },	/* TBU[10] */
	{ ((unsigned long long) 1 << 54), 0x40040000 },	/* TBU[9]  */
	{ ((unsigned long long) 1 << 55), 0x00040000 },	/* TBU[8]  */
	{ ((unsigned long long) 1 << 56), 0xc0020000 },	/* TBU[7]  */
	{ ((unsigned long long) 1 << 57), 0x80020000 },	/* TBU[6]  */
	{ ((unsigned long long) 1 << 58), 0x40020000 },	/* TBU[5]  */
	{ ((unsigned long long) 1 << 59), 0x00020000 },	/* TBU[4]  */
	{ ((unsigned long long) 1 << 60), 0xc0000000 },	/* TBU[3]  */
	{ ((unsigned long long) 1 << 61), 0x80000000 },	/* TBU[2]  */
	{ ((unsigned long long) 1 << 62), 0x40000000 },	/* TBU[1]  */
	{ ((unsigned long long) 1 << 63), 0x00000000 }	/* TBU[0] */
    };

LOCAL int       sysClkTicksPerSecond    = 60;   /* default ticks/second */
LOCAL FUNCPTR   sysClkRoutine           = NULL;
LOCAL int       sysClkArg               = (int)NULL;
LOCAL BOOL      sysClkConnectFirstTime  = TRUE;
LOCAL BOOL      sysClkRunning           = FALSE;

LOCAL int       sysAuxClkTicksPerSecond = 0;
LOCAL FUNCPTR   sysAuxClkRoutine        = NULL;
LOCAL int       sysAuxClkArg            = (int)NULL;
LOCAL BOOL      sysAuxClkRunning        = FALSE;

LOCAL int       sysWdtTicksPerSecond    = 0;
LOCAL FUNCPTR   sysWdtRoutine           = NULL;
LOCAL int       sysWdtArg               = (int)NULL;
LOCAL BOOL      sysWdtRunning           = FALSE;

LOCAL UINT32    decCountVal;                    /* DEC counter value */
LOCAL UINT32    fitPeriodMask = 0x00000000;     /* used to set TCR[FP]      */
LOCAL UINT32    wdtPeriodMask = 0x00000000;     /* used to set TCR[WP]      */

#ifdef  INCLUDE_TIMESTAMP
LOCAL BOOL      sysTimestampRunning = FALSE;    /* timestamp running flag */
#endif  /* INCLUDE_TIMESTAMP */


#define TCR_WP          (_PPC_TCR_WP_U    << 16)
#define TCR_WP_EXT      (0x001e  << 16)
#define TCR_WRC         (_PPC_TCR_WRC_U   << 16)
#define TCR_WIE         (_PPC_TCR_WIE_U   << 16)
#define TCR_DIE         (_PPC_TCR_DIE)
#define TCR_FP          (_PPC_TCR_FP_U    << 16)
#define TCR_FP_EXT      (0x0001e000)
#define TCR_FIE         (_PPC_TCR_FIE_U   << 16)
#define TCR_ARE         (_PPC_TCR_ARE_U   << 16)

#define TSR_ENW         (_PPC_TSR_ENW_U   << 16)
#define TSR_WIS         (_PPC_TSR_WIS_U   << 16)
#define TSR_WRS         (_PPC_TSR_WRS_U   << 16)
#define TSR_DIS         (_PPC_TSR_DIS)
#define TSR_FIS         (_PPC_TSR_FIS_U   << 16)


/***************************************************************************
*
* sysClkInt - clock interrupt handler
*
* This routine handles the clock interrupt on the PowerPC Book E
* architecture. It is attached to the Programmable Interval Timer vector
* by the routine sysClkConnect().
*
* RETURNS : N/A
*/

LOCAL void sysClkInt (void)
    {
    /* Acknowledge DEC interrupt */

    vxDecIntAck ();

    /* The DEC reloads itself automatically from DECAR */

    /* Execute the system clock routine */

    if (sysClkRoutine != NULL)
        (*(FUNCPTR) sysClkRoutine) (sysClkArg);

    }

/***************************************************************************
*
* sysClkConnect - connect a routine to the system clock interrupt
*
* This routine specifies the interrupt service routine to be called at
* each clock interrupt. Normally, it is called from usrRoot() in
* usrConfig.c to connect usrClock() to the system clock interrupt. It
* also connects the clock error interrupt service routine.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the
* interrupt.
*
* SEE ALSO: usrClock(), sysClkEnable()
*/

STATUS sysClkConnect
    (
    FUNCPTR     routine,        /* routine to connect */
    int         arg             /* argument with which to call the routine */
    )
    {

    if (sysClkConnectFirstTime)
        {
        sysHwInit2();
        sysClkConnectFirstTime = FALSE;
        }

    sysClkRoutine       = routine;
    sysClkArg           = arg;

    return (OK);
    }

/***************************************************************************
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
        /* clear the pending DEC interrupt */
        vxTsrSet (TSR_DIS);

        /* load the DEC counter and DECAR with interval value */
        vxDecarSet (decCountVal);
        vxDecSet (decCountVal);

        /* Enable the DEC interrupt & enable autoreload */
        vxTcrSet (vxTcrGet() | TCR_DIE | TCR_ARE);

	/* Enable the TBEN in HID0 */
	vxHid0Set (vxHid0Get() | _PPC_HID0_TBEN);

        /* set the running flag */

        sysClkRunning = TRUE;
        }
    }

/***************************************************************************
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
        /* disable the DEC interrupt and auto-reload capability */

        vxTcrSet (vxTcrGet() & ~ (TCR_DIE | TCR_ARE));

        /* clear the DEC counter and DECAR */

        vxDecSet (0);

        /* clear the pending DEC interrupt */

        vxTsrSet (TSR_DIS);

        /* reset the running flag */

        sysClkRunning = FALSE;
        }
    }

/***************************************************************************
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

/***************************************************************************
*
* sysClkRateSet - set the system clock rate
*
* This routine sets the interrupt rate of the system clock. It does not
* enable system clock interrupts. It is called by usrRoot() in
* usrConfig.c.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot
* be set.
*
* SEE ALSO: sysClkEnable(), sysClkRateGet()
*/

STATUS sysClkRateSet
    (
    int         ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    if (ticksPerSecond < SYS_CLK_RATE_MIN || ticksPerSecond > SYS_CLK_RATE_MAX)
        return (ERROR);

    /* save the clock speed */

    sysClkTicksPerSecond = ticksPerSecond;

    /*
     * compute the value to load in the decrementer. The new value will
     * be loaded into the decrementer after the end of the current period
     */

    decCountVal = sysTimerClkFreq / ticksPerSecond;

    /* Update the DEC interval  FIX 11/27/00 */

    vxDecarSet (decCountVal);
    vxDecSet (decCountVal);

    return (OK);
    }

/***************************************************************************
*
* sysAuxClkInt - auxiliary clock interrupt handler
*
* This routine handles the auxiliary clock interrupt on the PowerPC Book E
* architecture. It is attached to the Fix Interval Timer vector by the
* routine sysAuxClkConnect().
*
* RETURNS : N/A
*/

void sysAuxClkInt (void)
    {
    vxFitIntAck ();             /* acknowledge FIT interrupt */

    /* program TCR with the FIT period */

    vxTcrSet ((vxTcrGet() & ~(TCR_FP|TCR_FP_EXT)) | fitPeriodMask);

    /* execute the system clock routine */

    if (sysAuxClkRoutine != NULL)
        (*(FUNCPTR) sysAuxClkRoutine) (sysAuxClkArg);

    }
/***************************************************************************
*
* sysAuxClkConnect - connect a routine to the auxiliary clock interrupt
*
* This routine specifies the interrupt service routine to be called at
* each auxiliary clock interrupt. It does not enable auxiliary clock
* interrupts.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the
* interrupt.
*
* SEE ALSO: excIntConnectTimer(), sysAuxClkEnable()
*/

STATUS sysAuxClkConnect
    (
    FUNCPTR     routine, /* routine called at each aux. clock interrupt */
    int         arg      /* argument to auxiliary clock interrupt */
    )
    {

    sysAuxClkRoutine    = routine;
    sysAuxClkArg        = arg;

    return (OK);
    }

/***************************************************************************
*
* sysAuxClkEnable - turn on auxiliary clock interrupts
*
* This routine enables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkConnect(), sysAuxClkDisable(), sysAuxClkRateSet()
*/

void sysAuxClkEnable (void)
    {
    if (!sysAuxClkRunning)
        {
        /* clear the pending FIT interrupt */

        vxTsrSet (TSR_FIS);

        /* program TCR with the FIT period */

        vxTcrSet ((vxTcrGet() & ~(TCR_FP|TCR_FP_EXT)) | fitPeriodMask);

        /* Enable the FIT interrupt */

        vxTcrSet (vxTcrGet() | TCR_FIE);

        /* set the running flag */

        sysAuxClkRunning = TRUE;
        }
    }

/***************************************************************************
*
* sysAuxClkDisable - turn off auxiliary clock interrupts
*
* This routine disables auxiliary clock interrupts.
*
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkEnable()
*/

void sysAuxClkDisable (void)
    {
    if (sysAuxClkRunning)
        {
        /* disable the FIT interrupt */

        vxTcrSet (vxTcrGet() & (~TCR_FIE));

        /* clear the pending FIT interrupt */

        vxTsrSet (TSR_FIS);

        /* reset the running flag */

        sysAuxClkRunning = FALSE;
        }
    }

/***************************************************************************
*
* sysAuxClkRateGet - get the auxiliary clock rate
*
* This routine returns the interrupt rate of the auxiliary clock.
*
* RETURNS: The number of ticks per second of the auxiliary clock.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateSet()
*/

int sysAuxClkRateGet (void)
    {
    return (sysAuxClkTicksPerSecond);
    }

/***************************************************************************
*
* sysAuxClkRateSet - set the auxiliary clock rate
*
* This routine sets the interrupt rate of the auxiliary clock. It does
* not enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot
* be set.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateGet()
*/

STATUS sysAuxClkRateSet
    (
    int         ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    int ix;
    int jx = 0;
    unsigned long long fitPeriod;

    /*
     * compute the FIT period.  The closest value to <ticksPerSecond>
     * is being used.
     */

    if (ticksPerSecond < AUX_CLK_RATE_MIN || ticksPerSecond > AUX_CLK_RATE_MAX)
        return (ERROR);

    fitPeriod = (sysTimerClkFreq >> 1) / ticksPerSecond;

    /* get the closest value to ticksPerSecond supported by the FIT */

    for (ix = 0; ix < (int) NELEMENTS (fitTable); ix++)
        {
        if (fitPeriod <= fitTable [ix].fitPeriod)
            {
            if (ix != 0)
                if ( fitPeriod <
                     ((fitTable [ix].fitPeriod + fitTable [ix-1].fitPeriod)/2))
                    jx = ix-1;
                else
                    jx = ix;
            else
                jx = ix;
            break;
            }
        if (ix == NELEMENTS (fitTable) - 1)
            jx = ix;
        }
    fitPeriod = fitTable [jx].fitPeriod;        /* actual period of the FIT */
    fitPeriodMask = fitTable [jx].fpMask;       /* Mask to program TCR with */

    /* save the clock speed */

    sysAuxClkTicksPerSecond = (sysTimerClkFreq >> 1) / fitPeriod;

    return (OK);
    }

/***************************************************************************
*
* sysWdtInt - watchdog interrupt handler.
*
* This routine handles the watchdog interrupt on the PowerPC Book E
* architecture. It is attached to the watchdog timer vector by the
* routine sysWdtConnect().
*
* RETURNS : N/A
*/

LOCAL void sysWdtInt (void)
    {
    /* acknowledge WDT interrupt */

    vxTsrSet (TSR_WIS);

    /* execute the watchdog  clock routine */

    if (sysWdtRoutine != NULL)
        (*(FUNCPTR) sysWdtRoutine) (sysWdtArg);

    }

/***************************************************************************
*
* sysWdtConnect - connect a routine to the watchdog interrupt
*
* This routine specifies the interrupt service routine to be called at
* each watchdog interrupt. It does not enable watchdog interrupts.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the
* watchdog
* interrupt.
*
* SEE ALSO: excIntCrtConnect(), sysWdtEnable()
*/

STATUS sysWdtConnect
    (
    FUNCPTR     routine, /* routine called at each watchdog interrupt */
    int         arg      /* argument with which to call routine        */
    )
    {
    /* connect the routine to the WDT vector */

    excIntCrtConnect ((VOIDFUNCPTR *) _EXC_OFF_WD, (VOIDFUNCPTR) sysWdtInt);

    sysWdtRoutine    = routine;
    sysWdtArg        = arg;

    return (OK);
    }

/***************************************************************************
*
* sysWdtEnable - turn on watchdog interrupts
*
* This routine enables watchdog interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysWdtConnect(), sysWdtDisable(), sysWdtRateSet()
*/

void sysWdtEnable (void)
    {
    if (!sysWdtRunning)
        {
        /* clear the pending WDT interrupt */

        vxTsrSet (TSR_WIS);

        /* program TCR with the WDT period */

        vxTcrSet ((vxTcrGet() & ~(TCR_WP|TCR_WP_EXT)) | wdtPeriodMask);

        /* Enable the WDT interrupt */

        vxTcrSet (vxTcrGet() | TCR_WIE);

        /* Enable critical interrupt */

        vxMsrSet (vxMsrGet() | _PPC_MSR_CE);

        /* set the running flag */

        sysWdtRunning = TRUE;
        }
    }

/***************************************************************************
*
* sysWdtDisable - turn off watchdog interrupts
*
* This routine disables watchdog interrupts. This routine does not
* disable critical interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysWdtEnable()
*/

void sysWdtDisable (void)
    {
    if (sysWdtRunning)
        {
        /* disable the WDT interrupt */

        vxTcrSet (vxTcrGet() & ~ (TCR_WIE));

        /* clear the pending WDT interrupt */

        vxTsrSet (TSR_WIS);

        /* reset the running flag */

        sysWdtRunning = FALSE;
        }
    }

/***************************************************************************
*
* sysWdtRateGet - get the watchdog timer rate
*
* This routine returns the interrupt rate of the watchdog timer.
*
* RETURNS: The number of watchdog interrupts per second.
*
* SEE ALSO: sysWdtEnable(), sysWdtRateSet()
*/

int sysWdtRateGet (void)
    {
    return (sysWdtTicksPerSecond);
    }

/***************************************************************************
*
* sysWdtRateSet - set the watchdog timer rate
*
* This routine sets the interrupt rate of the watchdog timer. It does
* not enable watchdog interrupts.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot
* be set.
*
* SEE ALSO: sysWdtEnable(), sysWdtRateGet()
*/

STATUS sysWdtRateSet
    (
    int         ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    int ix;
    int jx = 0;
    unsigned long long wdtPeriod;

    /*
     * compute the WDT period.
     * only 4 values are possible (cf wdtTable[]). The closest value to
     * <ticksPerSecond> is being used.
     */

    if ((ticksPerSecond < (int) WDT_RATE_MIN) ||
        (ticksPerSecond > (int) WDT_RATE_MAX))
        return (ERROR);

    wdtPeriod = (sysTimerClkFreq >> 1) / ticksPerSecond;

    /* get the closest value to ticksPerSecond supported by the WDT */

    for (ix = 0; ix < (int) NELEMENTS (wdtTable); ix++)
        {
        if (wdtPeriod <= wdtTable [ix].wdtPeriod)
            {
            if (ix != 0)
                if ( wdtPeriod <
                     ((wdtTable [ix].wdtPeriod + wdtTable [ix-1].wdtPeriod)/2))
                    jx = ix-1;
                else
                    jx = ix;
            else
                jx = ix;
            break;
            }
        if (ix == NELEMENTS (wdtTable) - 1)
            jx = ix;
        }
    wdtPeriod = wdtTable [jx].wdtPeriod;        /* actual period of the WDT */
    wdtPeriodMask = wdtTable [jx].fpMask;       /* Mask to program TCR with */

    /* save the clock speed */

    sysWdtTicksPerSecond = (sysTimerClkFreq >> 1) / wdtPeriod;

    return (OK);
    }


#ifdef  INCLUDE_TIMESTAMP

/***************************************************************************
*
* sysTimestampConnect - connect a user routine to a timestamp timer interrupt
*
* This routine specifies the user interrupt routine to be called at each
* timestamp timer interrupt. In this case, however, the timestamp timer
* interrupt is not used since the on-chip timer is also used by the
* system clock.
*
* RETURNS: OK, or ERROR if sysTimestampInt() interrupt handler is not
* used.
*/

STATUS sysTimestampConnect
    (
    FUNCPTR     routine, /* routine called at each timestamp timer interrupt */
    int         arg      /* argument with which to call routine */
    )
    {
    return (ERROR);
    }

/***************************************************************************
*
* sysTimestampEnable - initialize and enable a timestamp timer
*
* This routine disables the timestamp timer interrupt and initializes
* the counter registers. If the timestamp timer is already running, this
* routine merely resets the timer counter.
*
* This routine does not initialize the timer rate. The rate of the
* timestamp timer should be set explicitly in the BSP by sysHwInit().
*
* RETURNS: OK, or ERROR if the timestamp timer cannot be enabled.
*/

STATUS sysTimestampEnable (void)
   {
   if (sysTimestampRunning)
      {
      return (OK);
      }

   if (!sysClkRunning)
      return (ERROR);

   sysTimestampRunning = TRUE;

   return (OK);
   }

/*******************************************************************************
*
* sysTimestampDisable - disable a timestamp timer
*
* This routine disables the timestamp timer. Interrupts are not
* disabled; however, because the tick counter does not increment after
* the timestamp timer is disabled, interrupts no longer are generated.
*
* RETURNS: OK, or ERROR if the timestamp timer cannot be disabled.
*/

STATUS sysTimestampDisable (void)
    {
    if (sysTimestampRunning)
        sysTimestampRunning = FALSE;

    return (OK);
    }

/*******************************************************************************
*
* sysTimestampPeriod - get a timestamp timer period
*
* This routine specifies the period of the timestamp timer, in ticks.
* The period, or terminal count, is the number of ticks to which the
* timestamp timer counts before rolling over and restarting the counting
* process.
*
* RETURNS: The period of the timestamp timer in counter ticks.
*/

UINT32 sysTimestampPeriod (void)
    {
    /*
     * The period of the timestamp depends on the clock rate of the
     * on-chip timer (ie the Decrementer reload value).
     */

    return (decCountVal);
    }

/*******************************************************************************
*
* sysTimestampFreq - get a timestamp timer clock frequency
*
* This routine specifies the frequency of the timer clock, in ticks per
* second. The rate of the timestamp timer should be set explicitly in
* the BSP by sysHwInit().
*
* RETURNS: The timestamp timer clock frequency, in ticks per second.
*/

UINT32 sysTimestampFreq (void)
    {
    return (sysTimerClkFreq);
    }

/*******************************************************************************
*
* sysTimestamp - get a timestamp timer tick count
*
* This routine returns the current value of the timestamp timer tick
* counter. The tick count can be converted to seconds by dividing by the
* return of sysTimestampFreq().
*
* This routine should be called with interrupts locked. If interrupts
* are not locked, sysTimestampLock() should be used instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestampFreq(), sysTimestampLock()
*/

UINT32 sysTimestamp (void)
    {
    return (decCountVal - (UINT32) vxDecGet());
    }

/***************************************************************************
*
* sysTimestampLock - lock an interrupt and get a timestamp timer tick count
*
* This routine locks interrupts when stop the tick counter must be
* stopped in order to read it or when two independent counters must be
* read. It then returns the current value of the timestamp timer tick
* counter.
*
* The tick count can be converted to seconds by dividing by the return
* of sysTimestampFreq().
*
* If interrupts are already locked, sysTimestamp() should be used
* instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestamp()
*/

UINT32 sysTimestampLock (void)
    {
    UINT32 currentDecValue;
    int oldLevel;

    oldLevel = intLock ();                              /* LOCK INTERRUPT */
    currentDecValue = vxDecGet();
    intUnlock (oldLevel);                               /* UNLOCK INTERRUPT */

    return (decCountVal - currentDecValue);
    }

#endif  /* INCLUDE_TIMESTAMP */
