/* usrTime.c - associate POSIX clock, ANSI time and a Real-Time Clock device */

/* Copyright 1997 Wind River Systems Inc. */

/* $Id: usrTime.c,v 1.3 2011/07/21 16:14:17 yshtil Exp $
modification history
--------------------
01c,01may97,leo  derived from older RTC driver
*/

/*
DESCRIPTION

This module should be included from usrConfig.c and called somewhere from
usrRoot() in order to associate and synchronize between POSIX clock of the
kernel and a Real-Time Clock chip found on some boards.

INTERNAL
Due to the way the POSIX clock is currently implementedm it was not really
feasible to perform gradual clock adjustment, i.e. slightly modify the clock
resolution to compensate for inaccuracy of the system clock frequency.
*/


/* includes */
#include "vxWorks.h"
#include "stdio.h"
#include "time.h"
#include "timers.h"
#include "sysLib.h"
#include "taskLib.h"
#include "stdlib.h"
#include "logLib.h"
#include "private/timerLibP.h"

/* globals */
BOOL usrTimeDebug = FALSE ;

/* forward declaration - should be moved to sysLib.h someday */
extern STATUS sysRtcGet ( struct tm *tm);
extern STATUS sysRtcSet ( const struct tm *tm);

/******************************************************************************
*
* usrTimeSync - synchronize the system clock with an RTC
*
* This function should be called upon system initialization, after
* sysClkEnable(), or any time after that repeatedly.
*
* NOTE:
* If accurate time keeping is needed, every so often the clock resolution
* can be subtly changed to reflect possible inaccuracy of the System
* Clock interrupt generator, and keep the wall clock in sync with the RTC.
*
*/
STATUS usrTimeSync (void)
    {
    struct timespec tv ;
    struct tm tm ;

    if (sysRtcGet( &tm ) == OK) {

        tv.tv_sec = mktime( &tm );
        tv.tv_nsec = 0;
        clock_settime( CLOCK_REALTIME, &tv );

        tv.tv_sec = 0;
        tv.tv_nsec = 1000000000 / sysClkRateGet() ;
        clock_setres( CLOCK_REALTIME, &tv );
    }

    return OK ;
    }

#if 0
/******************************************************************************
*
* date - set or display current time and date
*
* A simple date and time manipulator similar to "date" command on Unix.
* If the argument is 0 (or NULL), it will display the date and time
* according to the Real Time Chip. If the argument is -1, this command
* will display the date and time according to VxWorks  REALTIME_CLOCK.
* Otherwise the argument can be a string of the following form
* to set the RTC and system date and time manually:
*
*    "YYMMDDhhmmss"
*
* where "YY" stands for last two digits of the year, and so forth.
*
* RETURNS: N/A
*
* SEE ALSO
*
* ansiTime(1), clockLib(1)
*/
void date
    (
    const char *str
    )
    {
    struct tm tm ;
    struct timespec tv ;
    char buf[60];
    size_t i = sizeof(buf);
    time_t t ;


    if ((int)str == -1)
	{
	t = time (NULL);
	localtime_r( &t, &tm);
	asctime_r( &tm, buf, &i );
	printf("%s", buf );
	}
    else if (str != NULL)
	{
	/* Set the time */

	tm.tm_sec	= (str[10] -'0')*10 + str[11] - '0' ;
	tm.tm_min	= (str[8] -'0')*10 + str[9] - '0' ;
	tm.tm_hour	= (str[6] -'0')*10 + str[7] - '0' ;
	tm.tm_mday	= (str[4] -'0')*10 + str[5] - '0' ;
	tm.tm_mon	= (str[2] -'0')*10 + str[3] - '0' - 1; /* Jan == 0 */
	tm.tm_year	= (str[0] -'0')*10 + str[1] - '0' ;
	tm.tm_wday = 0 ;
	tm.tm_yday = 0 ;
	tm.tm_isdst = 0 ;

	/* correction for y2k */
	if (tm.tm_year < 80 )
	    tm.tm_year += 100 ;

	tv.tv_sec = mktime( &tm );
	tv.tv_nsec = 0;
	sysRtcSet( &tm );
	clock_settime( CLOCK_REALTIME, &tv );
	asctime_r( &tm, buf, &i );
	printf("%s", buf );
	}
    else
	{
	/* Print the date and time of the RTC */
	sysRtcGet( &tm );
	asctime_r( &tm, buf, &i );
	printf("%s", buf );
	}
#ifdef	LOCAL_DEBUG
    printf("	tm.tm_sec %d\n",	tm.tm_sec);
    printf("	tm.tm_min %d\n",	tm.tm_min);
    printf("	tm.tm_hour %d\n",	tm.tm_hour);
    printf("	tm.tm_mday %d\n",	tm.tm_mday);
    printf("	tm.tm_mon %d\n",	tm.tm_mon);
    printf("	tm.tm_year %d\n",	tm.tm_year);
    printf("	tm.tm_wday %d\n",	tm.tm_wday);
    printf("	tm.tm_yday %d\n",	tm.tm_yday);
    printf("	tm.tm_isdst %d\n",	tm.tm_isdst);
#endif
    }

/******************************************************************************
*
* usrTimeAdj - routinely adjust system time with the Real-Time Clock
*
*/
STATUS usrTimeAdj( int interval )
    {
    static time_t lastFreq, lastTime, lastPeriod ;
    FAST time_t freq, t, t1;
    long  dif;
    struct tm tm ;
    struct timespec tv ;

    if( interval == 0)
	interval = 120 ;

    freq = sysClkRateGet();

    FOREVER
	{
	/* sleep until the next iteration */
	taskDelay( interval * freq );

	taskLock() ;
	sysRtcGet( &tm );
	freq = sysClkRateGet();
	t = time (NULL);
	taskUnlock();

	if( t == ERROR )
	    {
	    char buf[48]; int i=sizeof(buf);
	    asctime_r( &tm, buf, &i );
	    logMsg("mktime error %s", (int) buf, 0,0,0,0,0);
	    }

	t1 = mktime( &tm );
	dif = t1 - t;		/* how many seconds skew */

	/* logMsg("t=%d t1=%d dif=%d\n", t, t1, dif, 0,0,0); */

	/* if there is a serious difference, do a step adjustment */
	if ((abs(dif) > 1) || (freq != lastFreq) )
	    {
	    if(usrTimeDebug)
		{
		char buf[48]; int i=sizeof(buf);
		asctime_r( &tm, buf, &i );
		logMsg("syncing clock %s", (int) buf, 0,0,0,0,0);
		}
	    usrTimeSync();
	    /* update statics for next iteration */
	    lastFreq	= freq ;
	    lastPeriod 	= tv.tv_nsec = 1000000000 / freq ;
	    lastTime	= time(NULL) ;
	    }
#ifdef	__XXX__
	else if( dif != 0)
	    {
	    FAST time_t tmp ;
	    /* otherwise, gradually adjust clock to RTC speed */
	    tv.tv_sec = 0;

	    /* this expression must be carefully ordered, not to
	     * get involved with 64-bit arithmetic which is
	     * incomplete in the current version's library.
	     */
	    tmp = lastPeriod / ((t - lastTime)*freq);
	    tv.tv_nsec += tmp * dif ;

	    clock_setres( CLOCK_REALTIME, &tv );

	    if(usrTimeDebug)
		logMsg("dif %d old period %d ns, new period %d ns, interval %d\n",
			dif, lastPeriod, tv.tv_nsec, t - lastTime, 0, 0 );

	    /* update statics for next iteration */
	    lastFreq	= freq ;
	    lastPeriod 	= tv.tv_nsec ;
	    lastTime	= t ;
	    }
#endif	/*__XXX__*/

	} /* FOREVER */
    }

/******************************************************************************
*
* usrTimeInit - init system time and Real-Time Clock
*
*/
void usrTimeInit(void)
    {

    usrTimeSync();

    taskSpawn("tTimeSync", 255, 0, 4096, usrTimeAdj, 60,
		0,0,0,0,0,0,0,0,0);
    }
#endif
/* End Of File */
