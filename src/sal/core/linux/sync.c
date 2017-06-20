/*
 * $Id: sync.c,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <sal/core/sync.h>
#include <sal/core/thread.h>
#include <sal/core/alloc.h>
#include <sal/core/time.h>
#include <sal/core/spl.h>

#include "lkm.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif

#include <assert.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_sem_count_curr;
static unsigned int _sal_sem_count_max;
static unsigned int _sal_mutex_count_curr;
static unsigned int _sal_mutex_count_max;
#define SAL_SEM_RESOURCE_USAGE_INCR(a_curr, a_max, ilock)               \
        a_curr++;                                                       \
        a_max = ((a_curr) > (a_max)) ? (a_curr) : (a_max)
    
#define SAL_SEM_RESOURCE_USAGE_DECR(a_curr, ilock)                      \
        a_curr--

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
        *sem_curr = _sal_sem_count_curr - _sal_mutex_count_curr;
    }
    if (sem_max != NULL) {
        *sem_max = _sal_sem_count_max - _sal_mutex_count_max;
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

#define USECS_PER_JIFFY (SECOND_USEC / HZ)
#define USEC_TO_JIFFIES(usec) ((usec + (USECS_PER_JIFFY - 1)) / USECS_PER_JIFFY)


/* See thread.c for details */
static INLINE void
thread_check_signals(void)
{
    if(signal_pending(current)) {
        sal_thread_exit(0);
    }
}

/* Emulate user mode system calls */
static INLINE void
sal_ret_from_syscall(void)
{
    if (!sal_no_sleep()) {
        schedule();
        thread_check_signals();
    }
}

/*
 * recursive_mutex_t
 *
 *   This is an abstract type built on the POSIX mutex that allows a
 *   mutex to be taken recursively by the same thread without deadlock.
 */

typedef struct recursive_mutex_s {
    sal_sem_t		sem;
    sal_thread_t	owner;
    int			recurse_count;
    char		*desc;
} recursive_mutex_t;

sal_mutex_t
sal_mutex_create(char *desc)
{
    recursive_mutex_t	*rm;

    if ((rm = sal_alloc(sizeof (recursive_mutex_t), desc)) == NULL) {
	return NULL;
    }

    rm->sem = sal_sem_create(desc, 1, 1);
    rm->owner = 0;
    rm->recurse_count = 0;
    rm->desc = desc;

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_INCR(
            _sal_mutex_count_curr,
            _sal_mutex_count_max,
            ilock);
#endif
#endif

    return (sal_mutex_t) rm;
}

void
sal_mutex_destroy(sal_mutex_t m)
{
    recursive_mutex_t	*rm = (recursive_mutex_t *) m;

    sal_sem_destroy(rm->sem);

    sal_free(rm);
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_DECR(
            _sal_mutex_count_curr,
            ilock);
#endif
#endif
}

int
#ifdef BROADCOM_DEBUG_MUTEX
sal_mutex_take_intern(sal_mutex_t m, int usec)
#else
sal_mutex_take(sal_mutex_t m, int usec)
#endif
{
    recursive_mutex_t	*rm = (recursive_mutex_t *) m;
    sal_thread_t	myself = sal_thread_self();
    int			err;

    if (rm->owner == myself) {
	rm->recurse_count++;
        sal_ret_from_syscall();
	return 0;
    }

    err = sal_sem_take(rm->sem, usec);

    if (err) {
    assert(usec != sal_mutex_FOREVER);
	return -1;
    }

    rm->owner = myself;

    return 0;
}

int
#ifdef BROADCOM_DEBUG_MUTEX
sal_mutex_give_intern(sal_mutex_t m)
#else
sal_mutex_give(sal_mutex_t m)
#endif
{
    recursive_mutex_t	*rm = (recursive_mutex_t *) m;
    int			err;

    if (rm->recurse_count > 0) {
	rm->recurse_count--;
        sal_ret_from_syscall();
	return 0;
    }

    rm->owner = 0;

    err = sal_sem_give(rm->sem);

    assert(err == 0);

    return err ? -1 : 0;
}

/*
 * sem_ctrl_t
 *
 *   The semaphore control type uses the binary property to implement
 *   timed semaphores with improved performance using wait queues.
 */

typedef struct sem_ctrl_s {
    struct semaphore    sem;
    int                 binary;
    int                 cnt;
    wait_queue_head_t   wq;
    atomic_t            wq_active;
} sem_ctrl_t;

sal_sem_t
sal_sem_create(char *desc, int binary, int initial_count)
{
    sem_ctrl_t *s;

    if ((s = sal_alloc(sizeof(*s), desc)) != 0) {
	sema_init(&s->sem, initial_count);
        s->binary = binary;
        if (s->binary) {
            init_waitqueue_head(&s->wq);
        }
        atomic_set(&s->wq_active, 0);
    }

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_INCR(
            _sal_sem_count_curr,
            _sal_sem_count_max,
            ilock);
#endif
#endif

    return (sal_sem_t) s;
}

void
sal_sem_destroy(sal_sem_t b)
{
    sem_ctrl_t *s = (sem_ctrl_t *) b;

    if (s == NULL) {
	return;
    }

    /*
     * the linux kernel does not have a sema_destroy(s)
     */
    sal_free(s);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_DECR(
            _sal_sem_count_curr,
            ilock);
#endif
#endif
}

static int
_sal_sem_take(sal_sem_t b, int usec)
{
    sem_ctrl_t *s = (sem_ctrl_t *) b;
    int        err;

    if (usec == sal_sem_FOREVER && !in_interrupt()) {
        err = down_interruptible(&s->sem);
    } else {
        int time_wait = 1;
        int cnt;

        for (;;) {
            cnt = s->cnt;
            if (down_trylock(&s->sem) == 0) {
                err = 0;
                break;
            }

            if (s->binary) {

                /* Wait for event or timeout */

                if (time_wait > 1) {
                    err = -ETIMEDOUT;
                    break;
                }

                atomic_inc(&s->wq_active);

                err = wait_event_interruptible_timeout(s->wq, cnt != s->cnt, 
                                                       USEC_TO_JIFFIES(usec));
                atomic_dec(&s->wq_active);

                if (err < 0) {
                    break;
                }
                time_wait++;

            } else {

                /* Retry algorithm with exponential backoff */

                if (time_wait > usec) {
                    time_wait = usec;
                }

                sal_usleep(time_wait);
                
                usec -= time_wait;
            
                if (usec == 0) {
                    err = -ETIMEDOUT;
                    break;
                }

                if ((time_wait *= 2) > 100000) {
                    time_wait = 100000;
                }
            }
        }
    }

    sal_ret_from_syscall();

    return err;
}

int
sal_sem_take(sal_sem_t b, int usec)
{
    int err = 0;
    int retry;

    if ((usec < 0) && (usec != sal_sem_FOREVER)) {
        /* Return error if negative timeout is specified */
        return -1;
    }

    do {
        retry = 0;
        err = _sal_sem_take(b, usec);
        /* Retry if we got interrupted prematurely */
        if (err == -ERESTARTSYS || err == -EINTR) {
            retry = 1;
#ifdef LINUX_SAL_SEM_OVERRIDE
            /* Retry done from user-mode */
            return -2;
#endif
        }
    } while(retry);

    return err ? -1 : 0;
}

int
sal_sem_give(sal_sem_t b)
{
    sem_ctrl_t *s = (sem_ctrl_t *) b;
    int wq_active = atomic_read(&s->wq_active);

    if (s->binary) {
        if (down_trylock(&s->sem));
        up(&s->sem);
        s->cnt++;
        /*
         * At this point a binary semaphore may already have
         * been destroyed, so we need to make sure that we
         * do not access the semaphore anymore even if the
         * wake_up call is otherwise harmless.
         */
        if (wq_active) {
            wake_up_interruptible(&s->wq);
        }
    } else {
        up(&s->sem);
    }

    sal_ret_from_syscall();

    return 0;
}

/*
 * spinlock_ctrl_t
 *
 *   This is an abstract type built on the spinlock_t of Linux.
 */

typedef struct spinlock_ctrl_s {
    spinlock_t spinlock;
    unsigned long flags;
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
 *  The spinlock or NULL if creation failed
 */

sal_spinlock_t
sal_spinlock_create(char *desc)
{
    spinlock_ctrl_t sl = sal_alloc(sizeof(*sl), desc);

    if (sl != NULL) {
        spin_lock_init(&sl->spinlock);
        sl->flags = 0;
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
    spin_lock_irqsave(&sl->spinlock, sl->flags);
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
    spin_unlock_irqrestore(&sl->spinlock, sl->flags);
    return 0;
}
