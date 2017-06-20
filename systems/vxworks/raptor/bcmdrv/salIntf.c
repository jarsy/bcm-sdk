/*
 * $Id: salIntf.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include "vxWorks.h"
#include "sysLib.h"
#include "intLib.h"
#include "salIntf.h"
#include "semLib.h"
#include "assert.h"
#include "systemInit.h"
#include "signal.h"
#include "taskLib.h"
#include "cacheLib.h"

/*
 * Mutex and semaphore abstraction
 */

sal_mutex_t 
sal_mutex_create(char *desc) 
{
    SEM_ID      sem;

    assert(!INT_CONTEXT());

    sem = semMCreate(SEM_Q_PRIORITY |
        SEM_DELETE_SAFE |
        SEM_INVERSION_SAFE);

    return (sal_mutex_t) sem;
}


void
sal_mutex_destroy(sal_mutex_t m) 
{
    SEM_ID      sem = (SEM_ID) m;

    assert(!INT_CONTEXT());
    assert(sem);

    semDelete(sem);
}


int _sal_usec_to_ticks( uint32 usec );

int 
sal_mutex_take(sal_mutex_t m, int usec) 
{
    SEM_ID      sem = (SEM_ID) m;
    int         ticks;

    assert(!INT_CONTEXT());
    assert(sem);

    /*    ctrl_c_block();*/

    if (usec == (uint32) sal_mutex_FOREVER) {
        ticks = WAIT_FOREVER;
    }
    else {
        ticks = _sal_usec_to_ticks(usec);
    }

    if (semTake(sem, ticks) != OK) {
        /*ctrl_c_unblock();*/
        return -1;
    }

    return 0;
}


int 
sal_mutex_give(sal_mutex_t m) 
{
    SEM_ID      sem = (SEM_ID) m;
    int         err;

    assert(!INT_CONTEXT());

    err = semGive(sem);

    /*    ctrl_c_unblock();*/

    return (err == OK) ? 0 : -1;
}


static int _int_locked = 0;

int 
sal_spl(int level) 
{
    int il;
    _int_locked--;
#if VX_VERSION == 64
    intUnlock(level);
    il = 0;
#else
    il = intUnlock(level);
#endif
    return il;
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
    int il;
    il = intLock();
    _int_locked++;
    return (il);
}


#define DEFAULT_THREAD_OPTIONS VX_FP_TASK
#define SAL_THREAD_ERROR    ((sal_thread_t) -1)

sal_thread_t 
sal_thread_create(char *name, int ss, int prio, void (f)(void *), void *arg) 
{
    int     rv;
    sigset_t    new_mask, orig_mask;

#ifdef SAL_THREAD_PRIORITY
    prio = SAL_THREAD_PRIORITY;
#endif

    /* Make sure no child thread catches Control-C */

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGINT);
    sigprocmask(SIG_BLOCK, &new_mask, &orig_mask);
    rv = taskSpawn(name, prio, DEFAULT_THREAD_OPTIONS, ss, (FUNCPTR)f,
        (uint32)(arg), 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sigprocmask(SIG_SETMASK, &orig_mask, NULL);

    if (rv == ERROR) {
        return (SAL_THREAD_ERROR);
    }
    else {
        return ((sal_thread_t) (rv));
    }
}


int 
sal_thread_destroy(sal_thread_t thread) 
{
    if (taskDelete((uint32)(thread)) == ERROR) {
        return -1;
    }

    return 0;
}


sal_sem_t
sal_sem_create(char *desc, int binary, int initial_count) 
{
    SEM_ID      b;

    assert(!INT_CONTEXT());

    if (binary) {
        b = semBCreate(SEM_Q_FIFO, initial_count);
    }
    else {
        b = semCCreate(SEM_Q_FIFO, initial_count);
    }

    return (sal_sem_t) b;
}


void
sal_sem_destroy(sal_sem_t b) 
{
    SEM_ID      sem = (SEM_ID) b;

    assert(sem);
    semDelete(sem);
}


int
sal_sem_take(sal_sem_t b, int usec) 
{
    SEM_ID      sem = (SEM_ID) b;
    int         ticks;

    assert(!INT_CONTEXT());
    assert(sem);

    if (usec == (uint32) sal_sem_FOREVER) {
        ticks = WAIT_FOREVER;
    }
    else {
        ticks = _sal_usec_to_ticks(usec);
    }

    return (semTake(sem, ticks) != OK) ? -1 : 0;
}


int
sal_sem_give(sal_sem_t b) 
{
    SEM_ID      sem = (SEM_ID) b;

    assert(sem);

    return (semGive(sem) != OK) ? -1 : 0;
}


void 
sal_thread_exit(int rc) 
{
    exit(rc);
}


void 
sal_dma_flush(void *addr, int len) 
{
    CACHE_DRV_FLUSH(&cacheDmaFuncs, addr, len);
}


void 
sal_dma_inval(void *addr, int len) 
{
    CACHE_DRV_INVALIDATE(&cacheDmaFuncs, addr, len);
}
