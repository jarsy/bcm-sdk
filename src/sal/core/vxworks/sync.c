/*
 * $Id: sync.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	sync.c
 * Purpose:	Defines SAL routines for mutexes and semaphores
 *
 * Mutex and Binary Semaphore abstraction
 */

#include <sys/types.h>
#if VX_VERSION == 69
#include <types/vxWind.h>
#endif
#include <sal/types.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <vxWorks.h>
#include <semLib.h>
#include <sysLib.h>
#include <intLib.h>
#if VX_VERSION >= 66
#include <spinLockLib.h>
#endif

#include <assert.h>
#include <sal/core/sync.h>
#include <sal/core/spl.h>
#include <sal/core/thread.h>
#include <sal/core/alloc.h>

extern int _sal_usec_to_ticks(uint32 usec);	/* time.c */

#define SAL_INT_CONTEXT()	sal_int_context()

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_sem_count_curr;
static unsigned int _sal_sem_count_max;
static unsigned int _sal_mutex_count_curr;
static unsigned int _sal_mutex_count_max;
#define SAL_SEM_RESOURCE_USAGE_INCR(a_curr, a_max, ilock)               \
        ilock = intLock();                                              \
        a_curr++;                                                       \
        a_max = ((a_curr) > (a_max)) ? (a_curr) : (a_max);              \
        intUnlock(ilock)
    
#define SAL_SEM_RESOURCE_USAGE_DECR(a_curr, ilock)                      \
        ilock = intLock();                                              \
        a_curr--;                                                       \
        intUnlock(ilock)

/*
 * Function:
 *      sal_sem_resource_usage_get
 * Purpose:
 *      Provides count of active sem and maximum sem allocation
 * Parameters:
 *      sem_curr - Current semaphore allocation.
 *      sem_max - Maximum semaphore allocation.
 */

void
sal_sem_resource_usage_get(unsigned int *sem_curr, unsigned int *sem_max)
{
    if (sem_curr != NULL) {
        *sem_curr = _sal_sem_count_curr;
    }
    if (sem_max != NULL) {
        *sem_max = _sal_sem_count_max;
    }
}

/*
 * Function:
 *      sal_mutex_resource_usage_get
 * Purpose:
 *      Provides count of active mutex and maximum mutex allocation
 * Parameters:
 *      mutex_curr - Current mutex allocation.
 *      mutex_max - Maximum mutex allocation.
 */

void
sal_mutex_resource_usage_get(unsigned int *mutex_curr, unsigned int *mutex_max)
{
    if (mutex_curr != NULL) {
        *mutex_curr = _sal_mutex_count_curr;
    }
    if (mutex_max != NULL) {
        *mutex_max = _sal_mutex_count_max;
    }
}
#endif
#endif

/*
 * Keyboard interrupt protection
 *
 *   When a thread is running on a console, the user could Control-C
 *   while a mutex is held by the thread.  Control-C results in a signal
 *   that longjmp's somewhere else.  We prevent this from happening by
 *   blocking Control-C signals while any mutex is held.
 */

#ifndef NO_CONTROL_C
static int ctrl_c_depth = 0;
#endif

static void
ctrl_c_block(void)
{
    assert(!SAL_INT_CONTEXT());

#ifndef NO_CONTROL_C
    if (sal_thread_self() == sal_thread_main_get()) {
	if (ctrl_c_depth++ == 0) {
	    sigset_t set;
	    sigemptyset(&set);
	    sigaddset(&set, SIGINT);
	    sigprocmask(SIG_BLOCK, &set, NULL);
	}
    }
#endif
}

static void
ctrl_c_unblock(void)
{
    assert(!SAL_INT_CONTEXT());

#ifndef NO_CONTROL_C
    if (sal_thread_self() == sal_thread_main_get()) {
	assert(ctrl_c_depth > 0);
	if (--ctrl_c_depth == 0) {
	    sigset_t set;
	    sigemptyset(&set);
	    sigaddset(&set, SIGINT);
	    sigprocmask(SIG_UNBLOCK, &set, NULL);
	}
    }
#endif
}

/*
 * Mutex and semaphore abstraction
 */

sal_mutex_t
sal_mutex_create(char *desc)
{
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    int         ilock;
#endif
#endif

    SEM_ID		sem;

    assert(!SAL_INT_CONTEXT());

    sem = semMCreate(SEM_Q_PRIORITY |
		     SEM_DELETE_SAFE |
		     SEM_INVERSION_SAFE);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_INCR(
            _sal_mutex_count_curr,
            _sal_mutex_count_max,
            ilock);
#endif
#endif
    return (sal_mutex_t) sem;
}

void
sal_mutex_destroy(sal_mutex_t m)
{
    SEM_ID		sem = (SEM_ID) m;
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    int         ilock;
#endif
#endif

    assert(!SAL_INT_CONTEXT());
    assert(sem);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_DECR(
            _sal_mutex_count_curr,
            ilock);
#endif
#endif

    semDelete(sem);
}

int
sal_mutex_take(sal_mutex_t m, int usec)
{
    SEM_ID		sem = (SEM_ID) m;
    int			ticks;

    assert(!SAL_INT_CONTEXT());
    assert(sem);

    ctrl_c_block();

    if (usec == sal_mutex_FOREVER) {
	ticks = WAIT_FOREVER;
    } else {
	ticks = _sal_usec_to_ticks(usec);
    }

    if (semTake(sem, ticks) != OK) {
	ctrl_c_unblock();
    assert(usec != sal_mutex_FOREVER);
	return -1;
    }

    return 0;
}

int
sal_mutex_give(sal_mutex_t m)
{
    SEM_ID		sem = (SEM_ID) m;
    int			err;

    assert(!SAL_INT_CONTEXT());

    err = semGive(sem);

    ctrl_c_unblock();

    assert(err == OK);

    return (err == OK) ? 0 : -1;
}

sal_sem_t
sal_sem_create(char *desc, int binary, int initial_count)
{
    SEM_ID		b;
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    int         ilock;
#endif
#endif

    assert(!SAL_INT_CONTEXT());

    if (binary) {
	b = semBCreate(SEM_Q_FIFO, initial_count);
    } else {
	b = semCCreate(SEM_Q_FIFO, initial_count);
    }

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_INCR(
            _sal_sem_count_curr,
            _sal_sem_count_max,
            ilock);
#endif
#endif

    return (sal_sem_t) b;
}

void
sal_sem_destroy(sal_sem_t b)
{
    SEM_ID		sem = (SEM_ID) b;
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    int         ilock;
#endif
#endif

    assert(sem);
    semDelete(sem);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_DECR(
            _sal_sem_count_curr,
            ilock);
#endif
#endif
}

int
sal_sem_take(sal_sem_t b, int usec)
{
    SEM_ID		sem = (SEM_ID) b;
    int			ticks;

    assert(!SAL_INT_CONTEXT());
    assert(sem);

    if ((usec < 0) && (usec != sal_sem_FOREVER)) {
        /* Return error if negative timeout is specified */
        return -1;
    }

    if (usec == sal_sem_FOREVER) {
	ticks = WAIT_FOREVER;
    } else {
	ticks = _sal_usec_to_ticks(usec);
    }

    return (semTake(sem, ticks) != OK) ? -1 : 0;
}

int
sal_sem_give(sal_sem_t b)
{
    SEM_ID		sem = (SEM_ID) b;

    assert(sem);

    return (semGive(sem) != OK) ? -1 : 0;
}

typedef struct spinlock_ctrl_s {
#if VX_VERSION >= 66
    spinlockIsr_t spinlock;
#else
    int il;
#endif
    char *desc;
} *spinlock_ctrl_t;

/*
 * Function:
 *  sal_spinlock_create
 * Purpose:
 *  Create a spinlock
 * Parameters:
 *  desc - spinlock description
 * Returns:
 *  The spinlock
 */

sal_spinlock_t
sal_spinlock_create(char *desc)
{
    spinlock_ctrl_t sl = sal_alloc(sizeof(*sl), desc);

    if (sl != NULL) {
#if VX_VERSION >= 66
        spinLockIsrInit(&sl->spinlock, 0);
#else
        sl->il = 0;
#endif
        sl->desc = desc;
    }
    return (sal_spinlock_t)sl;
}

/*
 * Function:
 *  sal_spinlock_destroy
 * Purpose:
 *  Destroy a spinlock
 * Parameters:
 *  lock - spinlock to destroy
 * Returns:
 *  0 on success
 */

int
sal_spinlock_destroy(sal_spinlock_t lock)
{
    spinlock_ctrl_t sl = (spinlock_ctrl_t)lock;

    assert(sl);
    sal_free(sl);
    return 0;
}

/*
 * Function:
 *  sal_spinlock_lock
 * Purpose:
 *  Obtains a spinlock
 * Parameters:
 *  lock - spninlock to obtain
 * Returns:
 *  0 on success
 */

int
sal_spinlock_lock(sal_spinlock_t lock)
{
    spinlock_ctrl_t sl = (spinlock_ctrl_t)lock;

    assert(sl);
#if VX_VERSION >= 66
    spinLockIsrTake(&sl->spinlock);
#else
    sl->il = intLock();
#endif
    return 0;
}

/*
 * Function:
 *  sal_spinlock_unlock
 * Purpose:
 *  Releases a spinlock
 * Parameters:
 *  lock - spinlock to release
 * Returns:
 *  0 on success
 */

int
sal_spinlock_unlock(sal_spinlock_t lock)
{
    spinlock_ctrl_t sl = (spinlock_ctrl_t)lock;

    assert(sl);
#if VX_VERSION >= 66
    spinLockIsrGive(&sl->spinlock);
#else
    intUnlock(sl->il);
#endif
    return 0;
}
