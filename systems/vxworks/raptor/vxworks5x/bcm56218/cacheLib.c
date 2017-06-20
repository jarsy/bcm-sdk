/*
    EXTERNAL SOURCE RELEASE on 12/03/2001 3.0 - Subject to change without notice.

*/
/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/* $Id: cacheLib.c,v 1.3 2007/10/08 22:31:08 iakramov Exp $ */

/* include files */
#include    <vxWorks.h>
#include    <cacheLib.h>
#include    <memLib.h>
#include    <stdlib.h>
#include    <errnoLib.h>
#include    <intLib.h>
#include    <config.h>

#define UNCACHED(_a) ((unsigned long)(_a) | 0xA0000000)

/* externals */
IMPORT void reset_caches(void);
IMPORT UINT32 readConfig(void);
IMPORT void EnableCache(void);
IMPORT void DisableCache(void);
IMPORT void sysWbFlush(void);
IMPORT void FlushAll_Dcache(void);
IMPORT void FlushAll_Icache(void);
IMPORT void Invalidate_Icache(void * base_addr, size_t bcount);
IMPORT void Invalidate_Dcache(void * base_addr, size_t bcount);
IMPORT void Clear_Dcache(void * base_addr, size_t bcount);
IMPORT void Flush_Dcache(void * base_addr, size_t bcount);

/* forward declarations */
LOCAL void *    cacheBcm47xxMalloc (size_t bytes);
LOCAL STATUS    cacheBcm47xxFree (void * pBuf);
LOCAL STATUS    cacheBcm47xxInvalidate (CACHE_TYPE cache, void * pVirtAdrs, 
                                     size_t bytes);
LOCAL STATUS    cacheBcm47xxFlush (CACHE_TYPE cache, void * pVirtAdrs, 
                                     size_t bytes);
LOCAL STATUS    cacheBcm47xxClear (CACHE_TYPE cache, void * pVirtAdrs, 
                                     size_t bytes);
LOCAL void * 	cacheBcm47xxPhysToVirt (void * address);
LOCAL void * 	cacheBcm47xxVirtToPhys (void * address);
LOCAL STATUS 	cacheBcm47xxTextUpdate (void * address, size_t bytes);

LOCAL STATUS    cacheBcm47xxEnable (CACHE_TYPE  cache);
LOCAL STATUS    cacheBcm47xxDisable (CACHE_TYPE  cache);

#ifdef  CACHE_MEM_POOL_RESERVED
LOCAL PART_ID cachePoolMemPartId;

/*
 * XXX this is broken if cacheBcm47xxMalloc is called before cachePoolCreate
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
    cacheBcm47xxFlush (DATA_CACHE, pBuffer, CACHEMEM_POOL_SIZE);
    cacheBcm47xxInvalidate (DATA_CACHE, pBuffer, CACHEMEM_POOL_SIZE);
    cachePoolMemPartId = memPartCreate (pBuffer, CACHEMEM_POOL_SIZE);

    return (OK);
}

#define internalMalloc(x) (cachePoolMemPartId ? \
			memPartAlloc(cachePoolMemPartId, (x)) : malloc(x))
#define internalFree(x) (cachePoolMemPartId ? \
			memPartFree(cachePoolMemPartId, (x)) : free(x))

#else

#define internalMalloc(x) malloc(x)
#define internalFree(x) free(x)

#endif  /* CACHE_MEM_POOL_RESERVED */


/**************************************************************************
*
* cacheBcm47xxLibInit - initialize the BCM47xx cache library
*
* This routine initializes the function pointers for the BCM47xx cache
* library.  The board support package can select this cache library 
* by calling this routine.
*
* RETURNS: OK.
*/

STATUS cacheBcm47xxLibInit ( 
    CACHE_MODE instMode, 
    CACHE_MODE dataMode )
{
    cacheLib.enableRtn     = cacheBcm47xxEnable;     /* cacheEnable() */
    cacheLib.disableRtn    = cacheBcm47xxDisable;    /* cacheDisable() */

    cacheLib.lockRtn       = NULL;                   /* cacheLock() */
    cacheLib.unlockRtn     = NULL;                   /* cacheUnlock() */

    cacheLib.flushRtn      = cacheBcm47xxFlush;      /* cacheFlush() */
    cacheLib.pipeFlushRtn  = NULL;                   /* cachePipeFlush() */
    cacheLib.textUpdateRtn = cacheBcm47xxTextUpdate; /* cacheTextUpdate() */

    cacheLib.invalidateRtn = cacheBcm47xxInvalidate; /* cacheInvalidate() */
    cacheLib.clearRtn      = cacheBcm47xxClear;      /* cacheClear() */

    cacheLib.dmaMallocRtn  = (FUNCPTR) cacheBcm47xxMalloc; /* cacheDmaMalloc() */
    cacheLib.dmaFreeRtn    = cacheBcm47xxFree;             /* cacheDmaFree() */

    cacheLib.dmaVirtToPhysRtn = (FUNCPTR) cacheBcm47xxVirtToPhys;
    cacheLib.dmaPhysToVirtRtn = (FUNCPTR) cacheBcm47xxPhysToVirt;

    /* Check if caches are already on - if not then init the caches */
    if ( (readConfig() & 0x3) == 0x2)
        reset_caches();
    
    cacheDataMode       = dataMode;     /* save dataMode for enable */
    cacheDataEnabled    = FALSE; 
    cacheMmuAvailable   = TRUE;         /* mmu support is provided */

    return (OK);
}

/**************************************************************************
*
* cacheBcm47xxEnable - enable the selected cache
*
* The instruction or data cache tags are invalidated and the cache is
* enabled.
*
* RETURNS: OK, or ERROR if cache or cache control is not supported.
*/

LOCAL STATUS cacheBcm47xxEnable ( CACHE_TYPE  cache )
{
    unsigned int   status;

    switch (cache)
    {
        case INSTRUCTION_CACHE:
        case DATA_CACHE:
                status = intLock ();        /* LOCK INTS */
                FlushAll_Dcache();             /* reset data cache */
                FlushAll_Icache();             /* reset i cache */
                EnableCache();             /* enable data cache */
                intUnlock (status);         /* UNLOCK INTS */
                cacheDataEnabled = TRUE;    /* d-cache is currently on */
                cacheFuncsSet ();           /* update cache func pointers */
                break;
        
        default:
            errnoSet (S_cacheLib_INVALID_CACHE);
            return (ERROR);
            break;
    }

    return (OK);
}


/**************************************************************************
*
* cacheDisable - disable the selected cache
*
* The instruction or data cache is invalidated and disabled.
*
* RETURNS: OK, or ERROR if cache or cache control is not supported.
*/

LOCAL STATUS cacheBcm47xxDisable ( CACHE_TYPE  cache )
{
    unsigned int   status;

    switch (cache)
    {
        case INSTRUCTION_CACHE:
        case DATA_CACHE:
            status = intLock ();        /* LOCK INTS */
            FlushAll_Dcache();             /* reset data cache */
            FlushAll_Icache();             /* reset data cache */
            DisableCache();            /* disable data cache */
            intUnlock (status);         /* UNLOCK INTS */
            cacheDataEnabled = FALSE;   /* d-cache is currently off */
            cacheFuncsSet ();           /* update cache func pointers */
            break;

        default:
            errnoSet (S_cacheLib_INVALID_CACHE);
            return (ERROR);
            break;
    }

    return (OK);
}


/**************************************************************************
*
* cacheBcm47xxMalloc - allocate a cache-safe buffer, if possible
*
* This function will attempt to return a pointer to a section of memory
* that will not experience any cache coherency problems.  It also sets
* the flush and invalidate function pointers to NULL or to the respective
* flush and invalidate routines.  Since the cache is write-through, the
* flush function pointer will always be NULL.
*
* RETURNS: pointer to non-cached buffer, or NULL
*/

LOCAL void * cacheBcm47xxMalloc ( size_t bytes )
{
    char * pBuffer;

    if ((pBuffer = (char *) internalMalloc (bytes)) == NULL)
        return ((void *) pBuffer);
    else {
        cacheBcm47xxFlush(DATA_CACHE, pBuffer, bytes);
        cacheBcm47xxInvalidate(DATA_CACHE, pBuffer, bytes);
        return ((void *) K0_TO_K1(pBuffer));
    }
}


/**************************************************************************
*
* cacheBcm47xxFree - free the buffer acquired by cacheMalloc ()
*
* This routine restores the non-cached buffer to its original state
* and does whatever else is necessary to undo the allocate function.
*
* RETURNS: OK, or ERROR if not able to undo cacheMalloc() operation
*/

LOCAL STATUS cacheBcm47xxFree ( void * pBuf )
{
    internalFree ((void *)K1_TO_K0(pBuf));
    return (OK);
}

/**************************************************************************
*
* cacheBcm47xxInvalidate - invalidate all or some entries in a cache
*
* This routine invalidates all or some of the entries in the
* specified cache.
*
* RETURNS: OK, or ERROR if cache or cache control is not supported.
*/

LOCAL STATUS cacheBcm47xxInvalidate ( CACHE_TYPE cache, void *pVirtAdrs,
                                         size_t  bytes )
{                                                           
    switch (cache)
    {
    case DATA_CACHE:
        Invalidate_Dcache (pVirtAdrs, bytes);
        break;

    case INSTRUCTION_CACHE:
        Invalidate_Icache (pVirtAdrs, bytes);
        break;

    default:
        errno = S_cacheLib_INVALID_CACHE;
        return (ERROR);
        break;
    }
    
    return (OK);
}

/**************************************************************************
*
* cacheBcm47xxFlush - invalidate all or some entries in a cache
*
* This routine flushes all or some of the entries in the
* specified cache.
*
* RETURNS: OK, or ERROR if cache or cache control is not supported.
*/

LOCAL STATUS cacheBcm47xxFlush ( CACHE_TYPE cache, void *pVirtAdrs,
                                         size_t  bytes )
{
    switch (cache)
    {
    case DATA_CACHE:
        Flush_Dcache (pVirtAdrs, bytes);
        break;

    default:
        errno = S_cacheLib_INVALID_CACHE;
        return (ERROR);
        break;
    }
    return (OK);
}


/**************************************************************************
*
* cacheBcm47xxClear - invalidate and flush all or some entries in a cache
*
* This routine invalidates and flushes all or some of the entries in the
* specified cache.
*
* RETURNS: OK, or ERROR if cache or cache control is not supported.
*/
LOCAL STATUS cacheBcm47xxClear ( CACHE_TYPE cache, void *pVirtAdrs,
                                         size_t  bytes )
{
    switch (cache)
    {
    case DATA_CACHE:
        Clear_Dcache (pVirtAdrs, bytes);
        break;

    default:
        errno = S_cacheLib_INVALID_CACHE;
        return (ERROR);
        break;
    }
    return (OK);
}

/**************************************************************************
*
* cacheBcm47xxVirtToPhys - virtual-to-physical address translation
*
* This routine may be attached to the CACHE_DRV structure virtToPhysRtn
* function pointer by cacheBcm47xxMalloc().  This implies that the virtual
* memory library is not installed, and that the "cache-safe" buffer has
* been created through the use of the R3900 K1 segment.
*
* NOMANUAL
*/

LOCAL void * cacheBcm47xxVirtToPhys ( void * address )
{
    return ((void *) K1_TO_PHYS(address));
}


/**************************************************************************
*
* cacheBcm47xxPhysToVirt - physical-to-virtual address translation
*
* This routine may be attached to the CACHE_DRV structure physToVirtRtn
* function pointer by cacheBcm47xxMalloc().  This implies that the virtual
* memory library is not installed, and that the "cache-safe" buffer has
* been created through the use of the BCM47xx K1 segment.
*
* NOMANUAL
*/

LOCAL void * cacheBcm47xxPhysToVirt ( void * address )
{
    return ((void *) PHYS_TO_K1(address));
}


/**************************************************************************
*
* cacheBcm47xxTextUpdate - invalidate updated text section
*
* This routine invalidates the specified text section so that
* the correct updated text is executed.
*
* NOMANUAL
*/

LOCAL STATUS cacheBcm47xxTextUpdate ( void * address, size_t bytes )
{
    FlushAll_Dcache();
    return (cacheBcm47xxInvalidate (INSTRUCTION_CACHE, address, bytes));
}
