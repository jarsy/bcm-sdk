/*
 * $Id: time.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	time.c
 * Purpose:	Time management
 */

#include <sys/types.h>
#if VX_VERSION == 69
#include <types/vxWind.h>
#endif
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef VXWORKS
#include <sysLib.h>
#endif

#include <assert.h>
#include <sal/core/time.h>
#include <sal/core/boot.h>
#include <sal/appl/vxworks/hal.h>
/*
 * Function:
 *	_sal_usec_to_ticks
 * Purpose:
 *	Internal routine to convert microseconds to VxWorks clock ticks.
 * Parameters:
 *	usec - time period in microseconds
 * Returns:
 *	ticks - VxWorks clock ticks
 * Notes:
 *	Rounds up: 0 usec maps to 0 ticks; >0 usec maps to >0 ticks.
 */

int
_sal_usec_to_ticks(uint32 usec)
{
    int		divisor;

    divisor = SECOND_USEC / sysClkRateGet();

    return (usec + divisor - 1) / divisor;
}

#ifdef COMPILER_HAS_DOUBLE

/*
 * Function:
 *	sal_time_double
 * Purpose:
 *	Returns the time since boot in seconds.
 * Returns:
 *	Double precision floating-point seconds.
 * Notes:
 *	Useful for benchmarking.
 *	Not safe for use in interrupt routines since it uses floating point.
 *	Made to be as accurate as possible on the given platform.
 */

double
sal_time_double(void)
{
      double       rate = 0.0;   
      UINT32       tbu, tbl, freq;   
     
    if (platform_info->f_timestamp_get && 
        (platform_info->f_timestamp_get(&tbu,&tbl,&freq) == OK) ) {
        rate = 1.0 / freq;   
        return rate * (tbu * 4294967296.0 + tbl);   
    } else {   
        /* Precision limited to the OS clock period (typically 0.01 sec) */
        struct timespec	tv;

        clock_gettime(CLOCK_REALTIME, &tv);

        return (tv.tv_sec + tv.tv_nsec * 0.000000001);
    }
}

#endif /* COMPILER_HAS_DOUBLE */

/*
 * Function:
 *	sal_time_usecs
 * Purpose:
 *	Returns the relative time in microseconds modulo 2^32.
 * Returns:
 *	Time in microseconds modulo 2^32
 * Notes:
 *	The precision is limited to the VxWorks clock period
 *	(typically 10000 usec.)
 */

sal_usecs_t
sal_time_usecs(void)
{
#if defined(COMPILER_HAS_LONGLONG)   
      /* provide more accurate microsecond timing using the PPC timebase */   
     
      UINT32              tbu, tbl,freq;   
      unsigned long long  tb;   
    
    if (platform_info->f_timestamp_get && 
        (platform_info->f_timestamp_get(&tbu,&tbl,&freq) == OK) ) {
         tb = tbu;   
         tb <<= 32;   
         tb |= tbl;   

        if (freq/SECOND_USEC) {
            return (tb / (freq/SECOND_USEC));
        } else if (freq/SECOND_MSEC) {
            return ((tb / (freq/SECOND_MSEC)) * MILLISECOND_USEC);
        } 
    }             
#endif     
    {
    struct timespec	ltv;

    clock_gettime(CLOCK_REALTIME, &ltv);

    return (ltv.tv_sec * 1000000 + ltv.tv_nsec / 1000);
    }
}

/*
 * Function:
 *	sal_time
 * Purpose:
 *	Return the current time in seconds since 00:00, Jan 1, 1970.
 * Returns:
 *	Time in seconds
 * Notes:
 *	This routine must be implemented so it is safe to call from
 *	an interrupt routine.  It is used for timestamping and other
 *	purposes.
 */

sal_time_t
sal_time(void)
{
    struct timespec	tv;

    clock_gettime(CLOCK_REALTIME, &tv);

    return tv.tv_sec;
}
