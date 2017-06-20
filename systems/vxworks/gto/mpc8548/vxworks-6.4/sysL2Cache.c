/* sysL2Cache.c -  L2 cache routines */

/* $Id: sysL2Cache.c,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01c,30aug06,dtr  Support for rev2 silicon changes.
                 No support for 36 phys with L2.
01b,27jan06,dtr  Tidy up - coding standards.
01a,11oct05,dtr  Created from cds85xx/01f
*/

/*
DESCRIPTION 
L2 cache support routines including populating hooks for cache 
library access.

INCLUDE FILES: sysL2Cache.h
*/

#include <vxWorks.h>
#include "config.h"
#include <cacheLib.h>
#include "sysL2Cache.h"

/* L2 Cache Routines */

IMPORT VOIDFUNCPTR _pSysL2CacheInvFunc;
IMPORT VOIDFUNCPTR _pSysL2CacheEnable;
IMPORT VOIDFUNCPTR _pSysL2CacheDisable;
IMPORT VOIDFUNCPTR _pSysL2CacheFlush;
IMPORT UINT32 vxL2CTLGet(VUINT32 *);
IMPORT void vxL2CTLSet(UINT32,VUINT32*);
IMPORT char * cachePpcReadOrigin;

LOCAL void sysL2CacheInvFunc();
LOCAL void sysL2CacheEnable();
LOCAL void sysL2CacheDisable();
LOCAL void sysL2CacheFlush(int cacheType);

IMPORT void sysCacheFlush();

STATUS sysL2ExtWriteBufferAlloc (char *adrs,UINT size,BOOL lock);

/* 512/64  == (Min Cache Size / L2 Cache Line Size) */
int l2CacheLineNum = ((4096 + 8192) * L2_CACHE_SIZE); 


/******************************************************************************
*
* sysL2CacheInit - L2 cache initialization
*
* L2 cache initialization routine of blk size etc if only L2 cache
* else expect L2 setup by sysL2SramEnable. Attaches hooks into kernel 
* cache code.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/  
void sysL2CacheInit
    (
    )
    {
    volatile UINT32 l2CtlVal;
    int n;

    /* Get L2 cache parameters */
    l2CtlVal = vxL2CTLGet(M85XX_L2CTL(CCSBAR));

#ifndef INCLUDE_L2_SRAM
    vxL2CTLSet(l2CtlVal&(~M85XX_L2CTL_L2E_MSK),M85XX_L2CTL(CCSBAR));
    l2CtlVal = (L2_CACHE_SIZE << M85XX_L2CTL_L2BLKSIZ_BIT);
     /* Set L2CTL */
    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
#endif

    _pSysL2CacheInvFunc = (VOIDFUNCPTR)sysL2CacheInvFunc;
    _pSysL2CacheEnable = sysL2CacheEnable;
    _pSysL2CacheDisable = sysL2CacheDisable;
    _pSysL2CacheFlush = sysL2CacheFlush;

    /* Clean external write region registers */
    for ( n=0 ; n < NO_L2_EXT_WRITE_REGIONS ; n++)
        {
        *M85XX_L2CEWARn(CCSBAR,n) = 0;
        *M85XX_L2CEWCRn(CCSBAR,n) = 0;
        }

    }


/******************************************************************************
*
* sysL2CacheInvFunc - Invalidates L2 cache
*
* This routine Invalidates the L2 Cache.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/  

LOCAL void sysL2CacheInvFunc
    (
    )
    {
    volatile UINT32 l2CtlVal;

    /* Get present value */
    l2CtlVal = vxL2CTLGet(M85XX_L2CTL(CCSBAR));
    /* Invalidate entire L2 */
    l2CtlVal |=  M85XX_L2CTL_L2I_MSK;
    /* Set L2CTL */
    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
    while (vxL2CTLGet(M85XX_L2CTL(CCSBAR)) & M85XX_L2CTL_L2I_MSK);

    }

/******************************************************************************
*
* sysL2CacheEnable - Enables L2 cache
*
* This routine Enables the L2 cache.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/  
LOCAL void sysL2CacheEnable
    (
    int cacheType
    )
    {
    volatile UINT32 l2CtlVal;
    int key;

    key = intLock();

    l2CtlVal = vxL2CTLGet(M85XX_L2CTL(CCSBAR));

    l2CtlVal |= M85XX_L2CTL_L2E_MSK ;
     /* Set L2CTL */
    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
    intUnlock(key);
    
    }

/******************************************************************************
*
* sysL2CacheDisable - Disables L2 cache
*
* This routine Disables the L2 cache.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/  
LOCAL void sysL2CacheDisable
    (
    int cacheType
    )
    {
    volatile UINT32 l2CtlVal;
    int key;

    key = intLock();
    /* Set L2E to disabled */
    l2CtlVal = vxL2CTLGet(M85XX_L2CTL(CCSBAR));
    l2CtlVal &= ~((UINT32)M85XX_L2CTL_L2E_MSK);
    /* Set L2CTL */
    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
    intUnlock(key);
    }

/******************************************************************************
*
* sysL2CacheDisables - Flushes L1 and L2 cache lines
*
* This routine Flushes L1 and L2 cache lines.  Required to make sure L2
* is actually flushed. Uses cachePpcReadOrigin as a safe region which 
* equates to start of text section.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/  
LOCAL void sysL2CacheFlush
    (
    int cacheType
    )
    {
    int noOfCacheLines,key;

    key = intLock();
    noOfCacheLines = (ppcE500DCACHE_LINE_NUM + l2CacheLineNum );

    sysCacheFlush(noOfCacheLines,cachePpcReadOrigin, ppcE500CACHE_ALIGN_SIZE);

    intUnlock(key);

    }

/************************************************************************
*
* sysL2ExtWriteBufferAlloc - get an L2 address to write
*
* Establishes address which can be written direct to L2 externally. 
* Optionally lock into L2.  Requires power of 2 size and address 
* alignment >= 256 bytes.
*
* RETURNS: OK, or ERROR
*
* ERRNO: N/A
*/  
STATUS sysL2ExtWriteBufferAlloc
    (
    char *adrs,
    UINT size,
    BOOL lock
    )
    {
    int n;

    if (((UINT)ROUND_DOWN((UINT)adrs,size) != (UINT)adrs) ||
        ((UINT)ROUND_DOWN((UINT)adrs,0x100) != (UINT)adrs))
        {
        printf ("l2 external write region setup failed - check alignment\n"); 
        return(ERROR);
        }

    n = 0;

    while ( (*M85XX_L2CEWCRn(CCSBAR,n) & M85XX_L2CEWCR_E_MSK) == 
            M85XX_L2CEWCR_E_MSK)
        {
        n++;
        if ( n >= NO_L2_EXT_WRITE_REGIONS )
            break;
        }

    if ( n >= NO_L2_EXT_WRITE_REGIONS )
        {
        printf ("l2 external write region setup failed-no more available\n"); 
        return(ERROR);
        }


#ifndef REV2_SILICON
    *M85XX_L2CEWARn(CCSBAR,n) = (((UINT)adrs >> 4)& M85XX_L2CEWAR_ADDR_MSK);
#else
    *M85XX_L2CEWARn(CCSBAR,n) = ((UINT)adrs & M85XX_L2CEWAR_ADDR_MSK);
#endif

    *M85XX_L2CEWCRn(CCSBAR,n) =  M85XX_L2CEWCR_E_MSK  | 
                                 ((~(size - 1)) >> 8) | 
                                 (lock << M85XX_L2CEWCR_LOCK_BIT);
    
    return(OK);
    }
