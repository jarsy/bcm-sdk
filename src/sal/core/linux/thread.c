/*
 * $Id: thread.c,v 1.44 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 */

#include <lkm.h>
#include <sal/types.h>
#include <sal/core/libc.h>
#include <sal/core/thread.h>
#include <sal/core/time.h>
#include <linux/list.h>
#include <assert.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
#include <linux/sched/rt.h>
#endif

static DECLARE_COMPLETION(on_exit);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define KILL_THREAD(a, b, c)	send_sig((b), (a), (c))
#else
#define KILL_THREAD(a, b, c)	kill_proc((a)->pid, (b), (c))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
#define WQ_SLEEP(a, b)          wait_event_interruptible_timeout(a, NULL, b)
#else
#define WQ_SLEEP(a, b)          interruptible_sleep_on_timeout(&(a), b)
#endif


#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_thread_count_curr;
static unsigned int _sal_thread_count_max;
static unsigned int _sal_thread_stack_size_curr;
static unsigned int _sal_thread_stack_size_max;
static DEFINE_SPINLOCK(stat_lock);

#define SAL_THREAD_RESOURCE_USAGE_INCR(a_cnt, a_cnt_max, a_sz,          \
                                       a_sz_max, n_ssize, lock)         \
do  {   unsigned long flags;                                            \
        spin_lock_irqsave(&lock,flags);                                 \
        a_cnt++;                                                        \
        a_sz += (n_ssize);                                              \
        a_cnt_max = ((a_cnt) > (a_cnt_max)) ? (a_cnt) : (a_cnt_max);    \
        a_sz_max = ((a_sz) > (a_sz_max)) ? (a_sz) : (a_sz_max);         \
        spin_unlock_irqrestore(&lock,flags);                            \
    } while(0)

#define SAL_THREAD_RESOURCE_USAGE_DECR(a_count, a_ssize, n_ssize, lock) \
do  {   unsigned long flags;                                            \
        spin_lock_irqsave(&lock,flags);                                 \
        a_count--;                                                      \
        a_ssize -= (n_ssize);                                           \
        spin_unlock_irqrestore(&lock,flags);                            \
    } while(0)

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

#endif /* INCLUDE_BCM_SAL_PROFILE */
#endif /* BROADCOM_DEBUG */

#ifdef LKM_2_6
#include <linux/kthread.h>
#endif

typedef struct thread_ctrl_s {
    struct list_head list;
    struct task_struct *thread;
    struct semaphore start_sem;
    char name[16];
    void (*f)(void *);
    void *arg;
#ifndef LKM_2_6
    struct tq_struct tq;
#endif
    int prio;
} thread_ctrl_t;

static LIST_HEAD(thread_list);
static DEFINE_SPINLOCK(list_lock);

#define USECS_PER_JIFFY (SECOND_USEC / HZ)
#define USEC_TO_JIFFIES(usec) ((usec + (USECS_PER_JIFFY - 1)) / USECS_PER_JIFFY)

/* Transform 0..255 (0=highest) into -20..19 (-20=highest) */
#define SAL_TO_POSIX_NICE_PRIO(p) ((((p) * 39) / 255) - 20)

#ifndef SAL_THREAD_PRIO_HIGHEST 
#define SAL_THREAD_PRIO_HIGHEST 20
#endif
#ifndef SAL_THREAD_RT_PRIO_HIGHEST
#define SAL_THREAD_RT_PRIO_HIGHEST  90
#endif

#ifdef LKM_2_6

/* Transform 0-255 (0=highest) into SAL_THREAD_PRIO_HIGHEST-99 (SAL_THREAD_PRIO_HIGHEST=highest) */
#define SAL_TO_LINUX_2_6_RT_PRIO(p) (SAL_THREAD_PRIO_HIGHEST + (((p) * (99 - SAL_THREAD_PRIO_HIGHEST)) / 255))

#else

/* Transform 0-255 (0=highest) into 1-99 (99=highest) */
#define SAL_TO_POSIX_RT_PRIO(p) (99 - (((p) * 98) / 255))

#endif

#ifdef CONFIG_TIMESYS
#define SET_RT_SCHED(task, prio) \
    rt_setscheduler(task, prio, SCHED_RR, task->sa.bound_cpu_mask)
#else
#define SET_RT_SCHED(task, prio) \
do { \
    task->policy = SCHED_RR; \
    task->rt_priority = prio; \
} while (0)
#endif

#ifdef LKM_2_6
#define SCHED_OTHER     SCHED_NORMAL
#endif

#ifdef MAX_USER_RT_PRIO
/* Assume 2.6 scheduler - Some Linux vendors have the 2.6 scheduler in 2.4 (MontaVista)
   This is a way of detecting it.
 */
#define SET_USER_SCHED(task, prio) \
do { \
    task->policy = SCHED_OTHER; \
    set_user_nice(task, prio); \
} while (0)
#define SAL_YIELD(task) \
    yield()
#else
/* Assume 2.4 scheduler */
#define SET_USER_SCHED(task, prio) \
do { \
    task->policy = SCHED_OTHER; \
    task->nice = prio; \
} while (0)
#define SAL_YIELD(task) \
do { \
    task->policy |= SCHED_YIELD; \
    schedule(); \
} while (0)
#endif

/*
 * Function: _find_thread
 *
 * Purpose:
 *    Find thread control structure in local list.
 * Parameters:
 *    t - task structure (thread ID) to look for.
 * Returns:
 *    Thread control structure
 */
STATIC thread_ctrl_t * 
_find_thread(struct task_struct *t)
{
    struct list_head *l;

    list_for_each(l, &thread_list) {
        thread_ctrl_t *ctrl = (thread_ctrl_t *)l;
        if (ctrl->thread == t) {
            return ctrl;
        }
    }
    return NULL;
}
	
/*
 * Function: thread_check_signals
 *
 * Purpose:
 *    Check for any pending sugnals.
 *    This must always be done manually in kernel mode.
 * Parameters:
 *    None
 * Returns:
 *    Nothing
 */
STATIC INLINE void
thread_check_signals(void)
{
    if(signal_pending(current)) {
        sal_thread_exit(0);
    }
}

/*
 * Function: thread_boot
 *
 * Purpose:
 *    Entry point for each new thread created.
 * Parameters:
 *    ctrl - information about thread being created
 * Returns:
 *    Nothing
 * Notes:
 *    Signals and other parameters are configured before jumping to
 *    the actual thread's main routine.
 */
STATIC void 
thread_boot(thread_ctrl_t *ctrl)
{
    lock_kernel();

#ifdef LKM_2_6
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
    daemonize(ctrl->name); 
#endif
    allow_signal(SIGTERM);
    allow_signal(SIGKILL);
#else
    daemonize();
    siginitsetinv(&current->blocked, (sigmask(SIGKILL)|sigmask(SIGTERM)));
#endif

    reparent_to_init();

#ifdef SAL_LINUX_RT_THREADS
#ifndef LKM_2_6
    SET_RT_SCHED(current, SAL_TO_POSIX_RT_PRIO(ctrl->prio));
#endif /* LKM_2_6 */
#else
    SET_USER_SCHED(current, SAL_TO_POSIX_NICE_PRIO(ctrl->prio));
#endif

    strcpy(current->comm, ctrl->name);
    unlock_kernel();
    ctrl->thread = current;

    smp_mb();
    /* Signal that thread initialization completed */
    up(&ctrl->start_sem);

    /* This is the real thread */
    ctrl->f(ctrl->arg);

    /* Clean up thread control */
    sal_thread_exit(0);
}

#ifndef LKM_2_6
/*
 * Function: thread_launcher
 *
 * Purpose:
 *    Ensure that new thread is properly launched from process context.
 * Parameters:
 *    data - pass-through pointer to private thread data
 * Returns:
 *    Nothing
 */
STATIC void
thread_launcher(void *data)
{
    kernel_thread((int (*)(void *))thread_boot, data, 0);
}
#endif /* !LKM_2_6 */

/*
 * Function: sal_thread_create
 *
 * Purpose:
 *    Abstraction for task creation.
 * Parameters:
 *    name - name of task
 *    ss - stack size requested
 *    prio - scheduling prio (0 = highest, 255 = lowest)
 *    func - address of function to call
 *    arg - argument passed to func.
 * Returns:
 *    Thread ID
 * Notes:
 *    The stack size is fixed at the Linux kernel stacksize (8KB)
 */
sal_thread_t
sal_thread_create(char *name, int ss, int prio, void (f)(void *), void *arg)
{
    thread_ctrl_t *ctrl; 
    unsigned long flags;
#ifdef LKM_2_6
struct task_struct *thread;
    assert(!in_interrupt());
    if (in_interrupt()) {
         return NULL;
    }
#endif

    /* Allocate control structure */
    if ((ctrl = kmalloc(sizeof(thread_ctrl_t), GFP_ATOMIC)) == NULL) {
        return NULL;
    }
    memset(ctrl, 0, sizeof(thread_ctrl_t));
    /* Setup thread control structure */
    strncpy(ctrl->name, name ? name : current->comm, sizeof(ctrl->name)-1);
    ctrl->f = f;
    ctrl->arg = arg;
    ctrl->thread = NULL;
    ctrl->prio = prio;
    if (prio == SAL_THREAD_PRIO_NO_PREEMPT) {
        ctrl->prio = SAL_THREAD_RT_PRIO_HIGHEST;
    }
    init_MUTEX_LOCKED(&ctrl->start_sem);

    spin_lock_irqsave(&list_lock,flags);
    list_add(&ctrl->list, &thread_list);
    spin_unlock_irqrestore(&list_lock,flags);

#ifdef LKM_2_6 /* Use the new kthread interface. Manipulate prorities of the stopped thread. */
    thread=kthread_create((void *)thread_boot,(void *)ctrl,name);

    if (IS_ERR(thread)) {
        spin_lock_irqsave(&list_lock,flags);
        list_del(&ctrl->list);
        spin_unlock_irqrestore(&list_lock,flags);
        kfree(ctrl);
        return NULL;
    }

#ifdef SAL_LINUX_RT_THREADS
    SET_RT_SCHED(thread, SAL_TO_LINUX_2_6_RT_PRIO(ctrl->prio));
#endif

    if (prio == SAL_THREAD_PRIO_NO_PREEMPT) {
        thread->policy = SCHED_FIFO;
        thread->rt_priority = SAL_THREAD_RT_PRIO_HIGHEST;
    }

    smp_mb();
    wake_up_process(thread);

#else /* 2.4 Kernels */

    /* Launch new thread from task queue */
    ctrl->tq.sync = 0;
    INIT_LIST_HEAD(&ctrl->tq.list);
    ctrl->tq.routine = thread_launcher;
    ctrl->tq.data = ctrl;
    schedule_task(&ctrl->tq);

#endif

    /* Wait for task to initialize */
    down(&ctrl->start_sem);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_THREAD_RESOURCE_USAGE_INCR(
            _sal_thread_count_curr,
            _sal_thread_count_max,
            _sal_thread_stack_size_curr,
            _sal_thread_stack_size_max,
            0,
            stat_lock);
#endif
#endif

    return (sal_thread_t)ctrl->thread;
}

/*
 * Function: sal_thread_destroy
 *
 * Purpose:
 *    Abstraction for task deletion.
 * Parameters:
 *    thread - thread ID
 * Returns:
 *    Always 0
 * Notes:
 *    This routine is not generally used by Broadcom drivers because
 *    it's unsafe.  If a task is destroyed while holding a mutex or
 *    other resource, system operation becomes unpredictable.  Also,
 *    some RTOS's do not include kill routines.
 *
 *    Instead, Broadcom tasks are written so they can be notified via
 *    semaphore when it is time to exit, at which time they call
 *    sal_thread_exit().
 */
int
sal_thread_destroy(sal_thread_t thread)
{
    unsigned long flags;

    spin_lock_irqsave(&list_lock,flags);
    if (_find_thread((struct task_struct *)thread)) {
        KILL_THREAD(((struct task_struct *)thread),SIGKILL, 1);
    }
    spin_unlock_irqrestore(&list_lock,flags);
    wait_for_completion(&on_exit);

    return 0;
}

/*
 * Function: sal_thread_self
 *
 * Purpose:
 *    Return thread ID of caller.
 * Parameters:
 *    None
 * Returns:
 *    Thread ID
 */
sal_thread_t
sal_thread_self(void)
{
    return (sal_thread_t)current;
}

int
sal_thread_id_get(void)
{
    return current->pid;
}

/*
 * Function: sal_thread_name
 *
 * Purpose:
 *    Return name given to thread when it was created.
 * Parameters:
 *    thread - thread ID
 *    thread_name - buffer to return thread name
 *    thread_name_size - maximum size of buffer
 * Returns:
 *    thread_name
 * Notes:
 *    If the specified thread does not exist, an empty string is
 *    loaded into thread_name, and NULL is returned.
 */
char *
sal_thread_name(sal_thread_t thread, char *thread_name, int thread_name_size)
{	
    thread_ctrl_t *ctrl;
    unsigned long flags;

    spin_lock_irqsave(&list_lock,flags);
    ctrl = _find_thread((struct task_struct *)thread);
    if (ctrl) {
        strncpy(thread_name, ctrl->name, thread_name_size);
        spin_unlock_irqrestore(&list_lock,flags);
        thread_name[thread_name_size - 1] = 0;
        return thread_name;
    }
    else {
        spin_unlock_irqrestore(&list_lock,flags);
        thread_name[0] = 0;
        return NULL;
    }
}

/*
 * Function: sal_thread_exit
 *
 * Purpose:
 *    Exit the calling thread
 * Parameters:
 *    rc - return code from thread.
 * Notes:
 *    Never returns.
 */
void
sal_thread_exit(int rc)
{
    thread_ctrl_t *ctrl;
    unsigned long flags;

    spin_lock_irqsave(&list_lock,flags);
    ctrl = _find_thread(current);

    if(!ctrl) {
        spin_unlock_irqrestore(&list_lock,flags);
	/* We are not a kernel thread. 
	 * Just return so the process can leave the system call. 
	 */
	return; 
    }

    list_del(&ctrl->list);
    spin_unlock_irqrestore(&list_lock,flags);
    kfree(ctrl);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_THREAD_RESOURCE_USAGE_DECR(
            _sal_thread_count_curr,
            _sal_thread_stack_size_curr,
            0,
            stat_lock);
#endif
#endif

    /* If main thread then kill all other threads */
    if (sal_thread_self() == sal_thread_main_get()) {
        struct list_head *l;
        int i = 0;
        spin_lock_irqsave(&list_lock,flags);
        list_for_each(l, &thread_list) {
            KILL_THREAD(((thread_ctrl_t *)l)->thread, SIGKILL, 1);
            i++;
        }
        spin_unlock_irqrestore(&list_lock,flags);
        while (i-- > 0)
            wait_for_completion(&on_exit);
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
    /* Let kevent reap zombie */
    kill_proc(2, SIGCHLD, 1);
#endif

    /* do_exit() is not an exported symbol */
    complete_and_exit(&on_exit, rc);
}

/*
 * Function: sal_thread_yield
 *
 * Purpose:
 *    Yield the processor to other tasks.
 * Parameters:
 *    None
 * Returns:
 *    Nothing
 */
void
sal_thread_yield(void)
{
    if (!in_interrupt()) {
        SAL_YIELD(current);
        thread_check_signals();
    }
}

/*
 * Function: sal_sleep
 *
 * Purpose:
 *    Suspend calling thread for a specified number of seconds.
 * Parameters:
 *    sec - number of seconds to suspend
 * Returns:
 *    Nothing
 * Notes:
 *    Other tasks are free to run while the caller is suspended.
 *    Should never be called from interrupt context.
 */
void
sal_sleep(int sec)
{
    wait_queue_head_t queue;

    assert(!in_interrupt());
    init_waitqueue_head(&queue);
    WQ_SLEEP(queue, sec * HZ);
    thread_check_signals();
#ifndef LKM_2_6
    mb();
#endif
}

/*
 * Function: sal_usleep
 *
 * Purpose:
 *    Suspend calling thread for a specified number of microseconds.
 * Parameters:
 *    usec - number of microseconds to suspend
 * Returns:
 *    Nothing
 * Notes:
 *    Since the actual delay period for the WQ_SLEEP function depends
 *    on the Linux system tick (usually 1/100 sec), we simply yield
 *    the processor until the specified number of usecs have passed,
 *    whenever the delay period is less than a system tick.  Should
 *    only be called from interrupt context with small values.
 */
void
sal_usleep(uint32 usec)
{
    sal_usecs_t start_usec;
    wait_queue_head_t queue;

    if (in_interrupt()) {
        assert(usec < 1000);
        start_usec = sal_time_usecs();
        while ((sal_time_usecs() - start_usec) < usec)
            ;
    }
    else {
        if (usec <= SECOND_USEC / HZ) {
            start_usec = sal_time_usecs();
            do {
                sal_thread_yield();
            } while ((sal_time_usecs() - start_usec) < usec);
        } else {
            init_waitqueue_head(&queue);
            WQ_SLEEP(queue, USEC_TO_JIFFIES(usec));
            thread_check_signals();
        }
    }
#ifndef LKM_2_6
    mb();
#endif
}

static sal_thread_t _sal_main_thread = NULL;

/*
 * Function: sal_thread_main_set
 *
 * Purpose:
 *    Set which thread is the main thread
 * Parameters:
 *    thread - thread ID
 * Returns:
 *    Nothing
 * Notes:
 *    The main thread is the one that runs in the foreground on the
 *    console. It prints normally, takes keyboard signals, etc. When
 *    the main thread exits, all other threads will be terminated.
 */
void
sal_thread_main_set(sal_thread_t thread)
{
    _sal_main_thread = thread;
    smp_mb();
}
    
/*
 * Function: sal_thread_main_get
 *
 * Purpose:
 *    Return which thread is the main thread.
 * Returns:
 *    Thread ID
 * Notes:
 *    See sal_thread_main_set().
 */
sal_thread_t
sal_thread_main_get(void)
{
    return _sal_main_thread;
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

void
sal_udelay(uint32 usec)
{
    static volatile int _sal_udelay_counter;
    static uint32 loops = 0;
    uint32 ix, iy;

    if (loops == 0 || usec == 0) {      /* Need calibration? */
        uint32 max_loops;
        int start = 0, stop = 0;
        int mpt = USECS_PER_JIFFY;      /* usec/tick */

        for (loops = 1; loops < 0x1000 && stop == start; loops <<= 1) {
            /* Wait for clock turn over */
            for (stop = start = jiffies; start == stop; start = jiffies) {
                /* Empty */
            }
            sal_udelay(mpt);    /* Single recursion */
            stop = jiffies;
        }

        max_loops = loops / 2;  /* Loop above overshoots */

        start = stop = 0;

        if (loops < 4) {
            loops = 4;
        }

        for (loops /= 4; loops < max_loops && stop == start; loops++) {
            /* Wait for clock turn over */
            for (stop = start = jiffies; start == stop; start = jiffies) {
                /* Empty */
            }
            sal_udelay(mpt);    /* Single recursion */
            stop = jiffies;
        }
    }
   
    for (iy = 0; iy < usec; iy++) {
        for (ix = 0; ix < loops; ix++) {
            _sal_udelay_counter++;      /* Prevent optimizations */
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
    return FALSE;   
}

void *
sal_tls_key_get(sal_tls_key_t *key)
{   
    return NULL;    
}

int 
sal_tls_key_delete(sal_tls_key_t *key)
{
    return FALSE;   
}


