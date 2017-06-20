/*
 * $Id: alloc.c,v 1.24 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	alloc.c
 * Purpose:	Memory allocation
 */

#include <sal/core/alloc.h>

#include <sys/types.h>
#include <sal/types.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <vxWorks.h>
#include <cacheLib.h>
#include <intLib.h>
#include <sysLib.h>
#include <stdlib.h>
#include <memLib.h>

#ifdef BROADCOM_DEBUG

typedef struct block_s {
    uint32		start_sentinel;		/* value: 0xaaaaaaaa */
    char                *description;
    int                 size;
#if CPU==MIPS64
    int                 align;                 /* for Sibyte 8-byte alignment */
#endif
    struct block_s	*prev;
    struct block_s	*next;
    /* Variable user data; size S = (size + 3) / 4 words. */
    uint32		user_data[1];
    /* Then sentinel follows user data at user_data[S]; value: 0xbbbbbbbb */
} block_t;

static block_t *head = NULL;
#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_alloc_max;
static unsigned int _sal_alloc_curr;

#define SAL_ALLOC_RESOURCE_USAGE_INCR(a_curr, a_max, a_size, ilock)     \
        ilock = intLock();                                              \
        a_curr += (a_size);                                             \
        a_max = ((a_curr) > (a_max)) ? (a_curr) : (a_max);              \
        intUnlock(ilock)

#define SAL_ALLOC_RESOURCE_USAGE_DECR(a_curr, a_size, ilock)            \
        ilock = intLock();                                              \
        a_curr -= (a_size);                                             \
        intUnlock(ilock)

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
#endif
#endif /* BROADCOM_DEBUG */


void sal_get_alloc_counters(unsigned long	*alloc_bytes_count,unsigned long	*free_bytes_count)
{
    *alloc_bytes_count = 0;
    *free_bytes_count = 0;

}


/*
 * Function:
 *	sal_alloc
 * Purpose:
 *	Allocate general purpose system memory.
 * Parameters:
 *	sz - size of memory block to allocate.
 *	s - user description of memory block for debugging.
 * Returns:
 *	Pointer to allocated memory or NULL if out of memory.
 * Notes:
 *	Memory allocated by this routine is not guaranteed to be safe
 *	for hardware DMA read/write.
 */

void *
sal_alloc(unsigned int sz, char *s)
{
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    int		ilock;
#endif

    block_t	*p;
    unsigned int alloc_sz, size_words;
    int		il;

    assert(s != NULL);		/* Don't pass NULLs in here! */
    assert(s[0] != 0);		/* Don't pass empty strings in here! */

    size_words = (sz + 3) / 4;

    /* Check for wrap caused by bad input */
    alloc_sz = sizeof (block_t) + size_words * 4;
    if (alloc_sz < sz) {
        return NULL;
    }

    /*
     * Allocate space for block structure, user data, and the sentinel
     * at the end of the block (accounted for by user_data[1]).
     */
#if CPU==MIPS64
    if ((p = memalign(_CACHE_ALIGN_SIZE, alloc_sz)) == NULL) {
        return p;
    }
#else
    if ((p = malloc(alloc_sz)) == NULL) {
	return p;
    }
#endif

    p->start_sentinel = 0xaaaaaaaa;
    p->description = s;
    p->size = sz;
    p->user_data[size_words] = 0xbbbbbbbb;

    il = intLock();
    if (head != NULL) {
	head->prev = p;
    }
    p->prev = NULL;
    p->next = head;
    head = p;
    intUnlock(il);

#ifdef INCLUDE_BCM_SAL_PROFILE
    SAL_ALLOC_RESOURCE_USAGE_INCR(
        _sal_alloc_curr,
        _sal_alloc_max,
        (size_words * 4),
        ilock);
#endif

    return (void *)&p->user_data[0];

#else /* !BROADCOM_DEBUG */

#if CPU==MIPS64
    return memalign(_CACHE_ALIGN_SIZE, sz);
#else
    return malloc(sz);
#endif

#endif /* !BROADCOM_DEBUG */
}

/*
 * Function:
 *	sal_free
 * Purpose:
 *	Free memory block allocate by sal_alloc
 * Parameters:
 *	addr - Address returned by sal_alloc
 */

#ifdef BROADCOM_DEBUG

/* To do: use real data segment limits for bad pointer detection */

#define GOOD_PTR(p)						\
	(PTR_TO_INT(p) >= 0x00001000U &&			\
	 PTR_TO_INT(p) < 0xfffff000U)

#define GOOD_START(p)						\
	(p->start_sentinel == 0xaaaaaaaa)

#define GOOD_END(p)						\
	(p->user_data[(p->size + 3) / 4] == 0xbbbbbbbb)

#define GOOD_FIELD		0

#endif /* BROADCOM_DEBUG */

void 
sal_free(void *addr)
{
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    int		ilock;
#endif

    block_t	*p;
    int		il;
    int		size_words;

#ifdef SAL_FREE_NULL_IGNORE
    if (addr == NULL)
    {
        return;
    }
#endif

    assert(GOOD_PTR(addr));	/* Use macro to beautify assert message */

    p = (block_t *)
	((char *) addr - (int)(sizeof (*p) - sizeof (p->user_data)));

    assert(GOOD_START(p));	/* Use macro to beautify assert message */
    assert(GOOD_END(p));

    il = intLock();

    if (p == head) {
	if (p->prev != NULL) {
	    intUnlock(il);
	    assert(GOOD_FIELD);
	} else {
	    head = p->next;
	    if (head != NULL) {
		head->prev = NULL;
	    }
	    intUnlock(il);
	}
    } else {
	if (p->prev == NULL) {
	    intUnlock(il);
	    assert(GOOD_FIELD);
	} else {
	    p->prev->next = p->next;
	    if (p->next != NULL) {
		p->next->prev = p->prev;
	    }
	    intUnlock(il);
	}
    }

    size_words = (p->size + 3) / 4;

    /*
     * Detect redundant frees and memory being used after freeing
     * by filling entire block with 0xcc bytes.
     * (Similar to the way VxWorks fills unused memory with 0xee bytes).
     */

    memset(p, 0xcc, sizeof (block_t) + size_words * 4);

    free(p);
#ifdef INCLUDE_BCM_SAL_PROFILE
    SAL_ALLOC_RESOURCE_USAGE_DECR(
        _sal_alloc_curr,
        (size_words * 4),
        ilock);
#endif

#else /* !BROADCOM_DEBUG */

    free(addr);

#endif /* !BROADCOM_DEBUG */
}

#ifdef BROADCOM_DEBUG
/*
 * Function:
 *      sal_alloc_stat
 * Purpose:
 *      Dump the current allocations
 * Parameters:
 *      param - integer used to change behavior.
 * Notes:
 *      If the parameter is non-zero, the routine will not
 *      display successive entries with the same description.
 */

void 
sal_alloc_stat(void *param)
{
    block_t	*p;
    char	*last_desc;
    int		repeat;
    int		rep_count;
    int		tot_size;
    int		grand_tot;
    void	*datap;
    int		size;

    repeat = PTR_TO_INT(param);
    grand_tot = 0;
    if (!repeat) {	 /* Show every entry */
	for (p = head; p != NULL; p = p->next) {
	    grand_tot += p->size;
            printf("%8p: 0x%08x %s\n",
                   (void *)&p->user_data[0],
                   p->size,
                   p->description != NULL ? p->description : "???");
        }
    } else {	 /* Don't show repetitions */
	datap = NULL;
	size = 0;
	rep_count = 0;
	tot_size = 0;
	last_desc = "?";
	for (p = head; p != NULL; p = p->next) {
	    grand_tot += p->size;
            if (p->description == last_desc ||
		strcmp(p->description, last_desc) == 0) {
                rep_count++;
                tot_size += p->size;
            } else {
		if (rep_count > 0) {
		    printf("%8p: 0x%08x %s",
			   datap,
			   size,
			   last_desc);
		    if (rep_count > 1) {
			printf(", repeats %d times, total size 0x%x (%d)\n",
			       rep_count, tot_size, tot_size);
		    } else {
			printf("\n");
		    }
		}
		datap = &p->user_data[0];
		size = p->size;
		rep_count = 1;
                tot_size = p->size;
                last_desc = p->description;
            }
	}
	if (rep_count > 0) {
	    printf("%8p: 0x%08x %s",
		   datap,
		   size,
		   last_desc != NULL ? last_desc : "???");
	    if (rep_count > 1) {
		printf(", repeats %d times, total size 0x%x (%d)\n",
		       rep_count, tot_size, tot_size);
	    } else {
		printf("\n");
	    }
	}
    }
    printf("Grand total of %d bytes allocated\n", grand_tot);
}
#endif /* BROADCOM_DEBUG */
