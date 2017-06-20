/*
 * $Id: spl.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	spl.c
 * Purpose:	Interrupt Blocking
 */

#include <sys/types.h>
#include <signal.h>

#include <assert.h>
#include <sal/core/sync.h>

#ifdef SAL_SPL_NO_PREEMPT
#include <pthread.h>
#endif

static sal_mutex_t	spl_mutex;
static int		spl_level;

/*
 * Function:
 *	sal_spl_init
 * Purpose:
 *	Initialize the synchronization portion of the SAL
 * Returns:
 *	0 on success, -1 on failure
 */

int
sal_spl_init(void)
{
    /* Initialize spl */

    spl_mutex = sal_mutex_create("spl mutex");
    spl_level = 0;

    return 0;
}

/*
 * Function:
 *	sal_spl
 * Purpose:
 *	Set interrupt level.
 * Parameters:
 *	level - interrupt level to set.
 * Returns:
 *	Value of interrupt level in effect prior to the call.
 * Notes:
 *	Used most often to restore interrupts blocked by sal_splhi.
 */

int
sal_spl(int level)
{
#ifdef SAL_SPL_NO_PREEMPT
    struct sched_param param;
    int policy;

    if (pthread_getschedparam(pthread_self(), &policy, &param) == 0) {
        /* Interrupt thread uses SCHED_RR and should be left alone */
        if (policy != SCHED_RR) {
            /* setting sched_priority to 0 will only change the real time priority
               of the thread. For a non real time thread it should be 0. This is
               very Linux specific.
             */  
            param.sched_priority = 0;
            pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
        }
    }
#endif
    assert(level == spl_level);
    spl_level--;
    sal_mutex_give(spl_mutex);
    return 0;
}

/*
 * Function:
 *	sal_splhi
 * Purpose:
 *	Set interrupt mask to highest level.
 * Parameters:
 *	None
 * Returns:
 *	Value of interrupt level in effect prior to the call.
 */

int
sal_splhi(void)
{
#ifdef SAL_SPL_NO_PREEMPT
    struct sched_param param;
    int policy;

    if (pthread_getschedparam(pthread_self(), &policy, &param) == 0) {
        /* Interrupt thread uses SCHED_RR and should be left alone */
        if (policy != SCHED_RR) {
            param.sched_priority = 90;
            pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
        }
    }
#endif
    sal_mutex_take(spl_mutex, sal_mutex_FOREVER);
    return ++spl_level;
}

/*
 * Function:
 *	sal_int_context
 * Purpose:
 *	Return TRUE if running in interrupt context.
 * Parameters:
 *	None
 * Returns:
 *	Boolean
 */

int
sal_int_context(void)
{
    /* Must be provided by the virtual interrupt controller */
    extern int intr_int_context() __attribute__((weak)); 
    
    if (intr_int_context) {
        return intr_int_context();
    } else {
        return 0;
    }
}

/*
 * Function:
 *	sal_no_sleep
 * Purpose:
 *	Return TRUE if current context cannot sleep.
 * Parameters:
 *	None
 * Returns:
 *	Boolean
 */

int
sal_no_sleep(void)
{
    return 0;
}
