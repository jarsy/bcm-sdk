/*
 * $Id: spl.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        spl.c
 * Purpose:     Multi-processor safe recursive global lock
 */

#include <sal/core/spl.h>
#include "lkm.h"
#include <asm/hardirq.h>
#include <assert.h>
#include <linux/spinlock.h>
#include <asm/cache.h>

#ifdef LKM_2_6
#include <linux/preempt.h>
#else
#define preempt_disable()               do { } while (0)
#define preempt_enable()                do { } while (0)
#endif

/* The lock data structure is placed in it's own cacheline to prevent
   false sharing. */
 
struct __rec_spin_lock {
    spinlock_t spl_lock;
    unsigned long flags;
    struct task_struct *thread;
    int depth;
};

static struct _rec_spin_lock {
    spinlock_t spl_lock;
    unsigned long flags;
    struct task_struct *thread;
    int depth;
    char pad[ ((sizeof(struct __rec_spin_lock)+ L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1)) - sizeof(struct __rec_spin_lock)];
} __attribute__((aligned(L1_CACHE_BYTES))) rec_spin_lock = { __SPIN_LOCK_UNLOCKED(rec_spin_lock.spl_lock),0,NULL,0 };

/*
 * Function:
 *      sal_spl
 * Purpose:
 *      Set lock level. (MP safe)
 * Parameters:
 *      level - lock level to set.
 * Returns: always 0
 */

int
sal_spl(int level)
{
    assert (rec_spin_lock.thread == current);
    assert (rec_spin_lock.depth == level);

    if (likely(--rec_spin_lock.depth == 0)) {
        rec_spin_lock.thread = NULL;
        smp_wmb(); /* required on some systems */
        spin_unlock_irqrestore(&rec_spin_lock.spl_lock,rec_spin_lock.flags);
        preempt_enable();
    }
    return 0;
}

/*
 * Function:
 *      sal_splhi
 * Purpose:
 *      Increase the lock level. (MP safe)
 * Parameters:
 *      None
 * Returns:
 *      Value of new lock level.
 * Notes:
 *      Once the lock is taken interrupts are disabled and preemption
 *      cannot happen. This means only the thread holding the lock  
 *      can change rec_spin_lock.thread and rec_spin_lock.depth. 
 *      I.e. no mem barriers are required either.
 */

int
sal_splhi(void)
{
    if (likely(rec_spin_lock.thread != current)) {
        unsigned long flags;
        preempt_disable();
        spin_lock_irqsave(&rec_spin_lock.spl_lock,flags); 
        assert(rec_spin_lock.depth == 0);
        rec_spin_lock.flags = flags;
        rec_spin_lock.thread = current;
    }
    return ++rec_spin_lock.depth;
}

/*
 * Function:
 *      sal_int_locked
 * Purpose:
 *      Return TRUE if the global lock is taken.
 * Parameters:
 *      None
 * Returns:
 *      Boolean
 */

int
sal_int_locked(void)
{
    return rec_spin_lock.depth;
}

/*
 * Function:
 *      sal_int_context
 * Purpose:
 *      Return TRUE if running in interrupt context.
 * Parameters:
 *      None
 * Returns:
 *      Boolean
 */

int
sal_int_context(void)
{
    return in_interrupt();
}

/*
 * Function:
 *      sal_no_sleep
 * Purpose:
 *      Return TRUE if current context cannot sleep.
 * Parameters:
 *      None
 * Returns:
 *      Boolean
 * Notes:
 *      We also return TRUE if we hold the SPL lock because this
 *      also means that we cannot sleep.
 */

int
sal_no_sleep(void)
{
    if (rec_spin_lock.depth && rec_spin_lock.thread == current) {
        return 1;
    }
    return in_interrupt();
}
