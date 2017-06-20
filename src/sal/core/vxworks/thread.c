/*
 * $Id: thread.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	thread.c
 * Purpose:	Defines SAL routines for VxWorks threads
 *
 * Thread Abstraction
 */

#include <sys/types.h>
#if VX_VERSION == 69
#include <types/vxWind.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <vxWorks.h>
#include <taskLib.h>
#include <tickLib.h>
#include <sysLib.h>
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
#include <intLib.h>
#endif
#endif
#include <sal/core/time.h>

#include <assert.h>
#include <sal/types.h>
#include <sal/core/thread.h>

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_thread_count_curr;
static unsigned int _sal_thread_count_max;
static unsigned int _sal_thread_stack_size_curr;
static unsigned int _sal_thread_stack_size_max;
#define SAL_THREAD_RESOURCE_USAGE_INCR(a_cnt, a_cnt_max, a_sz,      \
                                       a_sz_max, n_ssize, ilock)    \
    ilock = intLock();                                              \
    a_cnt++;                                                        \
    a_sz += (n_ssize);                                              \
    a_cnt_max = ((a_cnt) > (a_cnt_max)) ? (a_cnt) : (a_cnt_max);    \
    a_sz_max = ((a_sz) > (a_sz_max)) ? (a_sz) : (a_sz_max);         \
    intUnlock(ilock)

#define SAL_THREAD_RESOURCE_USAGE_DECR(a_count, a_ssize, n_ssize, ilock)\
        ilock = intLock();                                              \
        a_count--;                                                      \
        a_ssize -= (n_ssize);                                           \
        intUnlock(ilock)

/*
 * Function:
 *      sal_thread_resource_usage_get
 * Purpose:
 *      Provides count of active threads and stack allocation
 * Parameters:
 *      alloc_curr - Current memory usage.
 *      alloc_max - Memory usage high water mark
 */

void
sal_thread_resource_usage_get(unsigned int *sal_thread_count_curr,
                              unsigned int *sal_stack_size_curr,
                              unsigned int *sal_thread_count_max,
                              unsigned int *sal_stack_size_max)
{
    if (sal_thread_count_curr != NULL) {
        *sal_thread_count_curr = _sal_thread_count_curr;
    }
    if (sal_stack_size_curr != NULL) {
        *sal_stack_size_curr = _sal_thread_stack_size_curr;
    }
    if (sal_thread_count_max != NULL) {
        *sal_thread_count_max = _sal_thread_count_max;
    }
    if (sal_stack_size_max != NULL) {
        *sal_stack_size_max = _sal_thread_stack_size_max;
    }
}

#endif
#define DEFAULT_THREAD_OPTIONS VX_FP_TASK
#else
#define DEFAULT_THREAD_OPTIONS (VX_UNBREAKABLE | VX_FP_TASK)
#endif /* BROADCOM_DEBUG */

#ifdef VX_THREAD_OPT_UNBREAKABLE
#undef DEFAULT_THREAD_OPTIONS
#define DEFAULT_THREAD_OPTIONS (VX_UNBREAKABLE | VX_FP_TASK)
#endif /* VX_THREAD_OPT_UNBREAKABLE */

extern int _sal_usec_to_ticks(uint32 usec);	/* time.c */

/*
 * Function:
 *	sal_thread_create
 * Purpose:
 *	Abstraction for task creation
 * Parameters:
 *	name - name of task
 *	ss - stack size requested
 *	prio - scheduling prio (0 = highest, 255 = lowest)
 *	func - address of function to call
 *	arg - argument passed to func.
 * Returns:
 *	Thread ID
 */

sal_thread_t
sal_thread_create(char *name, int ss, int prio, void (f)(void *), void *arg)
{
    int		rv;
    sigset_t	new_mask, orig_mask;

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    int         ilock;
#endif
#endif

    if (prio == SAL_THREAD_PRIO_NO_PREEMPT) {
        prio = 0;
    }

#ifdef SAL_THREAD_PRIORITY
    prio = SAL_THREAD_PRIORITY;
#endif /* SAL_THREAD_PRIORITY */

    /* Make sure no child thread catches Control-C */

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGINT);
    sigprocmask(SIG_BLOCK, &new_mask, &orig_mask);
    rv = taskSpawn(name, prio, DEFAULT_THREAD_OPTIONS, ss, (FUNCPTR)f,
		   PTR_TO_INT(arg), 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sigprocmask(SIG_SETMASK, &orig_mask, NULL);

    if (rv == ERROR) {
	return (SAL_THREAD_ERROR);
    } else {
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_THREAD_RESOURCE_USAGE_INCR(
            _sal_thread_count_curr,
            _sal_thread_count_max,
            _sal_thread_stack_size_curr,
            _sal_thread_stack_size_max,
            ss,
            ilock);
#endif
#endif
	return ((sal_thread_t) INT_TO_PTR(rv));
    }
}

/*
 * Function:
 *	sal_thread_destroy
 * Purpose:
 *	Abstraction for task deletion
 * Parameters:
 *	thread - thread ID
 * Returns:
 *	0 on success, -1 on failure
 * Notes:
 *	This routine is not generally used by Broadcom drivers because
 *	it's unsafe.  If a task is destroyed while holding a mutex or
 *	other resource, system operation becomes unpredictable.  Also,
 *	some RTOS's do not include kill routines.
 *
 *	Instead, Broadcom tasks are written so they can be notified via
 *	semaphore when it is time to exit, at which time they call
 *	sal_thread_exit().
 */

int
sal_thread_destroy(sal_thread_t thread)
{
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    TASK_DESC   taskdesc;

    if (taskInfoGet(PTR_TO_INT(thread), &taskdesc) != ERROR) {
        int         ilock;

        SAL_THREAD_RESOURCE_USAGE_DECR(
            _sal_thread_count_curr,
            _sal_thread_stack_size_curr,
            (taskdesc.td_pStackBase - taskdesc.td_pStackEnd),
            ilock);
    }
#endif
#endif
    if (taskDelete(PTR_TO_INT(thread)) == ERROR) {
	return -1;
    }

    return 0;
}

/*
 * Function:
 *	sal_thread_self
 * Purpose:
 *	Return thread ID of caller
 * Parameters:
 *	None
 * Returns:
 *	Thread ID
 */

sal_thread_t
sal_thread_self(void)
{
    return (sal_thread_t) INT_TO_PTR(taskIdSelf());
}

int
sal_thread_id_get(void)
{
    return taskIdSelf();
}

/*
 * Function:
 *	sal_thread_name
 * Purpose:
 *	Return name given to thread when it was created
 * Parameters:
 *	thread - thread ID
 *	thread_name - buffer to return thread name;
 *		gets empty string if not available
 *	thread_name_size - maximum size of buffer
 * Returns:
 *	NULL, if name not available
 *	thread_name, if name available
 */

char *
sal_thread_name(sal_thread_t thread, char *thread_name, int thread_name_size)
{
    char	*name = taskName(PTR_TO_INT(thread));

    if (name != NULL) {
	strncpy(thread_name, name, thread_name_size);
	thread_name[thread_name_size - 1] = 0;
	return thread_name;
    } else {
	thread_name[0] = 0;
	return NULL;
    }
}

/*
 * Function:
 *	sal_thread_exit
 * Purpose:
 *	Exit the calling thread
 * Parameters:
 *	rc - return code from thread.
 * Notes:
 *	Never returns.
 */

void
sal_thread_exit(int rc)
{
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    TASK_DESC   taskdesc;
    sal_thread_t thread;

    thread = sal_thread_self();
    if (taskInfoGet(PTR_TO_INT(thread), &taskdesc) != ERROR) {
        int         ilock;

        SAL_THREAD_RESOURCE_USAGE_DECR(
            _sal_thread_count_curr,
            _sal_thread_stack_size_curr,
            (taskdesc.td_pStackBase - taskdesc.td_pStackEnd),
            ilock);
    }
#endif
#endif
    exit(rc);
}

/*
 * Function:
 *	sal_thread_yield
 * Purpose:
 *	Yield the processor to other tasks.
 * Parameters:
 *	None
 */

void
sal_thread_yield(void)
{
    taskDelay(0);
}

/*
 * Function:
 *	sal_thread_main_set
 * Purpose:
 *	Set which thread is the main thread
 * Parameters:
 *	thread - thread ID
 * Notes:
 *	The main thread is the one that runs in the foreground on the
 *	console.  It prints normally, takes keyboard signals, etc.
 */

static sal_thread_t _sal_thread_main = 0;

void
sal_thread_main_set(sal_thread_t thread)
{
    _sal_thread_main = thread;
}

/*
 * Function:
 *	sal_thread_main_get
 * Purpose:
 *	Return which thread is the main thread
 * Returns:
 *	Thread ID
 * Notes:
 *	See sal_thread_main_set().
 */

sal_thread_t
sal_thread_main_get(void)
{
    return _sal_thread_main;
}

/*
 * Function:
 *	sal_sleep
 * Purpose:
 *	Suspend calling thread for a specified number of seconds.
 * Parameters:
 *	sec - number of seconds to suspend
 * Notes:
 *	Other tasks are free to run while the caller is suspended.
 */

void
sal_sleep(int sec)
{
    taskDelay(sec * sysClkRateGet());
}

/*
 * Function:
 *	sal_usleep
 * Purpose:
 *	Suspend calling thread for a specified number of microseconds.
 * Parameters:
 *	usec - number of microseconds to suspend
 * Notes:
 *	The actual delay period depends on the resolution of the
 *	VxWorks taskDelay() routine, whose precision is limited to the
 *	the period of the scheduler tick, generally 1/60 or 1/100 sec.
 *	Other tasks are free to run while the caller is suspended.
 */

void
sal_usleep(uint32 usec)
{
    sal_usecs_t t_start, t_curr,delta_time;
    int ticks;
    if (!usec) {
        return;
    }

    t_start = sal_time_usecs();

    ticks = _sal_usec_to_ticks(usec) - 1;

    /* if the sleep time is less than 1ms, simply spin it out since it probably wastes 
     * more cpu cycles to do a task context switch. 
     */
    if (!ticks) {
        if (usec >= 1000) {
            ticks = 1;
        }
    }
     
    if (ticks) {
        taskDelay(ticks);
    }

    while (TRUE) {
       t_curr = sal_time_usecs();
    
       /* check timestamp wraparound */
       if (t_curr < t_start) {
           delta_time = t_curr + (SAL_USECS_TIMESTAMP_ROLLOVER - t_start);
       } else {
           delta_time = t_curr - t_start;
       }
       if (delta_time >= usec) {
           break;
       }
       taskDelay(0);
    } 
}

/*
 * Function:
 *	sal_udelay
 * Purpose:
 *	Spin wait for an approximate number of microseconds
 * Parameters:
 *	usec - number of microseconds
 * Notes:
 *	MUST be called once before normal use so it can self-calibrate.
 *	Code for self-calibrating delay loop is courtesy of
 *	Geoffrey Espin, the comp.os.vxworks Usenet group, and
 *	JA Borkhuis (http://www.xs4all.nl/~borkhuis).
 */
volatile int _sal_udelay_counter;

void
sal_udelay(uint32 usec)
{
#if defined(BCM_ICS) && defined(QUICK_TURN)
    static int		loops = 200;
#else
    static int		loops = 0;
#endif
    int			ix;
    uint32		iy;

    if (loops == 0 || usec == 0) {		/* Need calibration? */
        int	max_loops;
        int	start = 0, stop = 0;
        int	mpt = 1000000 / sysClkRateGet();	/* usec/tick */

        for (loops = 1; loops < 0x1000 && stop == start; loops <<= 1) {
            /* Wait for clock turn over */

            for (stop = start = tickGet();
		 start == stop;
		 start = tickGet()) {
		/* Empty */
	    }

	    sal_udelay(mpt);	/* Single recursion */
            stop = tickGet();
        }

        max_loops = loops / 2;	/* Loop above overshoots */

        start = stop = 0;

        if (loops < 4) {
            loops = 4;
	}

        for (loops /= 4; loops < max_loops && stop == start; loops++) {
            /* Wait for clock turn over */

            for (stop = start = tickGet();
		 start == stop;
		 start = tickGet()) {
		/* Empty */
	    }

	    sal_udelay(mpt);	/* Single recursion */
            stop = tickGet();
        }
    }
   
    for (iy = 0; iy < usec; iy++) {
        for (ix = 0; ix < loops; ix++) {
	    _sal_udelay_counter++;	/* Prevent optimizations */
	}
    }
}

sal_tls_key_t * 
sal_tls_key_create(void (*destructor)(void *))
{
	if (destructor) {
		;
	}
    return NULL;
}

int 
sal_tls_key_set(sal_tls_key_t *key, void *val)
{   
    return 0;   
}

void *
sal_tls_key_get(sal_tls_key_t *key)
{   
    return NULL;    
}

int 
sal_tls_key_delete(sal_tls_key_t *key)
{
    return 0;   
}
