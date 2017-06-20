/*
 * $Id: alloc.c,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	alloc.c
 * Purpose:	Defines sal routines for memory allocation
 */
#include <lkm.h>
#include <sal/core/alloc.h>
#include <sal/types.h>
#include <linux/vmalloc.h>
#include <assert.h>

#ifdef BROADCOM_DEBUG
#define SAL_ALLOC_DEBUG
#endif /* BROADCOM_DEBUG */

#define FROM_KMALLOC 1
#define FROM_VMALLOC 2
#define FROM_KMALLOC_GIANT 3

extern void* kmalloc_giant(int sz);
extern void  kfree_giant(void* ptr);

typedef struct _alloc_info {
#ifdef SAL_ALLOC_DEBUG
    uint32 start_sentinel;
    uint32 size;
    struct list_head list;
    char *name;
#endif
    unsigned long src;
    uint32 data[1];
} alloc_info_t;

#ifdef SAL_ALLOC_DEBUG

static LIST_HEAD(_sal_alloc_info);
static DEFINE_SPINLOCK(_ai_lock);

#define AI_LOCK() spin_lock_irqsave(&_ai_lock, flags)
#define AI_UNLOCK() spin_unlock_irqrestore(&_ai_lock, flags)

#define START_SENTINEL 0xaaaaaaaa
#define END_SENTINEL 0xbbbbbbbb

#ifndef PTRS_ARE_64BITS
#define GOOD_PTR(p) \
	(PTR_TO_INT(p) >= 0x80000000U && \
	 PTR_TO_INT(p) < 0xfffff000U)
#else
#define GOOD_PTR(p) ((p) != NULL)      /* Temporary */
#endif

#define GOOD_START(p) \
	(p->start_sentinel == START_SENTINEL)

#define GOOD_END(p) \
	(p->data[(p->size + 3) / 4] == END_SENTINEL)

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_alloc_max;
static unsigned int _sal_alloc_curr;

#define SAL_ALLOC_RESOURCE_USAGE_INCR(a_curr, a_max, a_size, ilock)     \
        a_curr += (a_size);                                             \
        a_max = ((a_curr) > (a_max)) ? (a_curr) : (a_max)

#define SAL_ALLOC_RESOURCE_USAGE_DECR(a_curr, a_size, ilock)            \
        a_curr -= (a_size)                                              \

/*
 * Function:
 *      sal_alloc_resource_usage_get
 * Purpose:
 *      Provides Current/Maximum memory allocation.
 * Parameters:
 *      alloc_curr - Current memory usage.
 *      alloc_max - Memory usage high water mark
 */

void 
sal_alloc_resource_usage_get(uint32 *alloc_curr, uint32 *alloc_max)
{
    if (alloc_curr != NULL) {
        *alloc_curr = _sal_alloc_curr;
    }
    if (alloc_max != NULL) {
        *alloc_max = _sal_alloc_max;
    }
}
#endif /* INCLUDE_BCM_SAL_PROFILE */
#endif /* BROADCOM_DEBUG */
#endif /* SAL_ALLOC_DEBUG */

void sal_get_alloc_counters(unsigned long	*alloc_bytes_count,unsigned long	*free_bytes_count)
{
    *alloc_bytes_count = 0;
    *free_bytes_count = 0;

}


/*
 * Function:
 *    sal_alloc
 * Purpose:
 *    Allocate general purpose system memory.
 * Parameters:
 *    sz - size of memory block to allocate
 *    s - optional user description of memory block for debugging
 * Returns:
 *    Pointer to memory block
 * Notes:
 *    Memory allocated by this routine is not guaranteed to be safe
 *    for hardware DMA read/write.
 */
void *
sal_alloc(unsigned int sz, char *s)
{
    unsigned int alloc_sz, szw;
    alloc_info_t *ai;

    szw = (sz + 3) / 4;

    /* Check for wrap caused by bad input */
    alloc_sz = sizeof(alloc_info_t) + 4 * szw;
    if (alloc_sz < sz) {
        return NULL;
    }

    /*
     * If a memory allocation is too large to be handled 
     * by kmalloc(), we fall back to vmalloc.
     */
    if ((ai = kmalloc(alloc_sz, GFP_ATOMIC)) != NULL) {
        ai->src = FROM_KMALLOC;
    } else if ((ai = vmalloc(alloc_sz)) != NULL) {
        ai->src = FROM_VMALLOC;
    } else if ((ai = kmalloc_giant(alloc_sz)) != NULL) {
        ai->src = FROM_KMALLOC_GIANT;
    }

    if (ai) {
#ifdef SAL_ALLOC_DEBUG
        unsigned long flags;
        ai->size = sz;
        ai->name = s ? s : "unknown";
        ai->start_sentinel = START_SENTINEL;
        ai->data[szw] = END_SENTINEL;
        AI_LOCK();
        list_add(&ai->list, &_sal_alloc_info);
        AI_UNLOCK();
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    SAL_ALLOC_RESOURCE_USAGE_INCR(
        _sal_alloc_curr,
        _sal_alloc_max,
        (sz),
        ilock);
#endif /* INCLUDE_BCM_SAL_PROFILE */
#endif /* BROADCOM_DEBUG */
#endif /* SAL_ALLOC_DEBUG */
        return ai->data;
    }
    return NULL;
}

/*
 * Function:
 *    sal_free
 * Purpose:
 *    Free memory block allocated by sal_alloc.
 * Parameters:
 *    addr - Address returned by sal_alloc
 */
void 
sal_free(void *addr)
{
    alloc_info_t *ai;
#ifdef SAL_ALLOC_DEBUG
    struct list_head *pos;
    unsigned long flags;

#ifdef SAL_FREE_NULL_IGNORE
    if (addr == NULL)
    {
        return;
    }
#endif

    assert(GOOD_PTR(addr));

    ai = NULL;
    AI_LOCK();
    list_for_each(pos, &_sal_alloc_info) {
        ai = list_entry(pos, alloc_info_t, list);
        if (addr == ai->data) {

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_ALLOC_RESOURCE_USAGE_DECR(
            _sal_alloc_curr,
            (ai->size),
            ilock);
#endif
#endif /* BROADCOM_DEBUG */

            list_del(&ai->list);
            break;
        }
    }
    AI_UNLOCK();

    assert(GOOD_PTR(ai));
    assert(GOOD_START(ai));
    assert(GOOD_END(ai));
#else
    ai = addr - (sizeof(*ai) - sizeof(ai->data));
#endif
    switch (ai->src) {
    case FROM_VMALLOC:
        vfree(ai);
        break;
    case FROM_KMALLOC:
        kfree(ai);
        break;
    case FROM_KMALLOC_GIANT:
        kfree_giant(ai);
        break;
    default:
        printk(KERN_ERR "%s: Unknown source: %lu\n", FUNCTION_NAME(), ai->src);
        break;
    }
}

/*
 * Function:
 *    sal_alloc_purge
 * Purpose:
 *    Free all memory blocks allocated by sal_alloc.
 * Parameters:
 *    print_func - optional callback function
 * Returns:
 *    Nothing
 * Notes:
 *    This function is useful to track down memory leaks that occur 
 *    when unloading kernel modules, which may not always shut down 
 *    all threads gracefully. As opposed to user mode tasks, the
 *    kernel mode tasks are responsible for cleaning up all memory
 *    before exiting.
 *    If memory leaks are not a real problem, e.g. if the system is 
 *    always rebooted after modules are unloaded, this function may 
 *    still be helpful during development as it eliminates the need
 *    for rebooting the system between firmware changes.
 *    If a callback function is installed, care must be taken when
 *    printing the user description string (s) as the referenced
 *    memory may belong to a kernel module that has been unloaded 
 *    already. Basically, if the application consists of multiple
 *    kernel modules (not counting the BDE and proxy modules), the
 *    user description string should not be referenced.
 */
void
sal_alloc_purge(void (*print_func)(void *ptr, unsigned int size, char *s))
{  
#ifdef SAL_ALLOC_DEBUG
    struct list_head *pos, *tmp;
    alloc_info_t *ai;

    /* No need to use AI_LOCK here */
    list_for_each_safe(pos, tmp, &_sal_alloc_info) {
        ai = list_entry(pos, alloc_info_t, list);
        if (print_func) {
            print_func(ai->data, ai->size, ai->name);
        }
        sal_free(ai->data);
    }
#endif
}

/*
 * Function:
 *      sal_alloc_stat
 * Purpose:
 *      Dump the current allocations
 * Parameters:
 *      param - integer used to change behavior. (Ignored)
 * Notes: (Not Implemented)
 *      If the parameter is non-zero, the routine will not
 *      display successive entries with the same description.
 */

void 
sal_alloc_stat(void *param)
{  
#ifdef SAL_ALLOC_DEBUG
    struct list_head *pos, *tmp;
    alloc_info_t *ai;
    unsigned int        grand_tot;
    grand_tot = 0;

    /* No need to use AI_LOCK here */
    list_for_each_safe(pos, tmp, &_sal_alloc_info) {
        ai = list_entry(pos, alloc_info_t, list);
        printk("%8p: 0x%08x %s\n",
               (void *)ai->data,
               ai->size,
               ai->name != NULL ? ai->name : "???");
        grand_tot += ai->size;
    }
    printk("Grand total of %d bytes allocated\n", grand_tot);
#endif
}
