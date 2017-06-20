/*
 * $Id: time.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	time.c
 * Purpose:	Time management
 */

#include <sal/core/time.h>
#include "lkm.h"
#include <linux/time.h>

/* Check if system has getrawmonotonic() */
#ifndef LINUX_HAS_RAW_MONOTONIC
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
#define LINUX_HAS_RAW_MONOTONIC
#endif
#endif

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
 *	Made to be as accurate as possible on the given platform.
 */

double
sal_time_double(void)
{
    return 0;
}
#endif

/*
 * Function:
 *	sal_time_usecs
 * Purpose:
 *	Returns the relative time in microseconds modulo 2^32.
 * Returns:
 *	Time in microseconds modulo 2^32
 * Notes:
 *	The precision is limited to the Unix clock period
 *	(typically 10000 usec.)
 *	
 */

sal_usecs_t
sal_time_usecs(void)
{
#ifdef LINUX_HAS_RAW_MONOTONIC
    /* Use monotonic clock if available */
    struct timespec lts;
    getrawmonotonic(&lts);
    return (lts.tv_sec * SECOND_USEC + lts.tv_nsec / 1000);
#else
    /* Use RTC if monotonic clock unavailable */
    struct timeval ltv;
    do_gettimeofday(&ltv);
    return (ltv.tv_sec * SECOND_USEC + ltv.tv_usec);
#endif
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
    struct timeval ltv;
    do_gettimeofday(&ltv);
    return ltv.tv_sec;
}
