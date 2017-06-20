/* $Id: cacheLib.c,v 1.2 Broadcom SDK $ 
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/* include files */
#include    <vxWorks.h>
#include    <cacheLib.h>
#include    <memLib.h>
#include    <stdlib.h>
#include    <errnoLib.h>
#include    <intLib.h>
#include    <config.h>

#define UNCACHED(_a) ((unsigned long)(_a) | 0xA0000000)

/* forward declarations */
LOCAL void *    cacheBcm53000Malloc (size_t bytes);
LOCAL STATUS    cacheBcm53000Free (void * pBuf);

#ifdef  CACHE_MEM_POOL_RESERVED
LOCAL PART_ID cachePoolMemPartId;

/*
 * XXX this is broken if cacheBcm53000Malloc is called before cachePoolCreate
 * changes cachePoolMemPartId and then memory is freed - an allocation from
 * one pool will be added to the free list of the other.
 */
int cachePoolCreate ()
{
    char *pBuffer;

    pBuffer = malloc (CACHEMEM_POOL_SIZE);
    if (pBuffer == NULL) {
	/* give up; leave MemPartId as NULL so that internalMalloc
	 * falls back to malloc().
	 */
	return (ERROR);
    }

    cacheLib.flushRtn(DATA_CACHE, pBuffer, CACHEMEM_POOL_SIZE);
    cacheLib.invalidateRtn(DATA_CACHE, pBuffer, CACHEMEM_POOL_SIZE);

    cachePoolMemPartId = memPartCreate (pBuffer, CACHEMEM_POOL_SIZE);

    return (OK);
}

#define internalMalloc(x) (cachePoolMemPartId ? \
			memPartAlignedAlloc(cachePoolMemPartId, (x), 32) : malloc(x))
#define internalFree(x) (cachePoolMemPartId ? \
			memPartFree(cachePoolMemPartId, (x)) : free(x))

#else

#define internalMalloc(x) malloc(x)
#define internalFree(x) free(x)

#endif  /* CACHE_MEM_POOL_RESERVED */


/**************************************************************************
*
* cacheBcm53000LibUpdate - update cache library
*
* This routine updates the function pointers,
* dmaMallocRtn and dmaFreeRtn, for the BCM53000 cache
* library if DMA support is enabled.
*
* RETURNS: OK.
*/

int cacheBcm53000LibUpdate ()
{
    cacheLib.dmaMallocRtn  = (FUNCPTR) cacheBcm53000Malloc; /* cacheDmaMalloc() */
    cacheLib.dmaFreeRtn    = cacheBcm53000Free;             /* cacheDmaFree() */

    cacheFuncsSet ();                           /* update cache func ptrs */
    return (OK);
}


/**************************************************************************
*
* cacheBcm53000Malloc - allocate a cache-safe buffer, if possible
*
* This function will attempt to return a pointer to a section of memory
* that will not experience any cache coherency problems.  It also sets
* the flush and invalidate function pointers to NULL or to the respective
* flush and invalidate routines.  Since the cache is write-through, the
* flush function pointer will always be NULL.
*
* RETURNS: pointer to non-cached buffer, or NULL
*/

#define IS_UNMAPPED(_x)        (((unsigned)(_x) & 0x80000000) == 0x80000000)
#define MAPPED_CACHED_TO_UNCACHED(_x)    ((unsigned)(_x) | 0x60000000)
#define MAPPED_UNCACHED_TO_CACHED(_x)    ((unsigned)(_x) & 0x5fffffff)

LOCAL void * cacheBcm53000Malloc ( size_t bytes )
{
    char * pBuffer;

    if ((pBuffer = (char *) internalMalloc (bytes)) == NULL)
        return ((void *) pBuffer);
    else {
        cacheLib.flushRtn(DATA_CACHE, pBuffer, bytes);
        cacheLib.invalidateRtn(DATA_CACHE, pBuffer, bytes);

        if (IS_UNMAPPED(pBuffer)) {
            return ((void *) K0_TO_K1(pBuffer));
        } else {
            return ((void *) MAPPED_CACHED_TO_UNCACHED(pBuffer));
        }
    }
}


/**************************************************************************
*
* cacheBcm53000Free - free the buffer acquired by cacheMalloc ()
*
* This routine restores the non-cached buffer to its original state
* and does whatever else is necessary to undo the allocate function.
*
* RETURNS: OK, or ERROR if not able to undo cacheMalloc() operation
*/

LOCAL STATUS cacheBcm53000Free ( void * pBuf )
{
    if (IS_UNMAPPED(pBuf)) {
        internalFree ((void *)K1_TO_K0(pBuf));
    } else {
        internalFree ((void *)MAPPED_UNCACHED_TO_CACHED(pBuf));
    }
    return (OK);
}

