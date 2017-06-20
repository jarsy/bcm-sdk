/* sysL2Cache.c - ARM Prime Cell PL310 L2 Cache Controller Routines */

/*
 * Copyright (c) 2010-2011 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01d,08dec11, ggm  various modification (WIND00240234):
                  1.update description;
                  2.change variable name;
                  3.use explicit cache sync operation;
                  4.remove L2_CACHE_SIZE because it is not used.
01c,06dec11, ggm  make the driver SMP-safe (WIND00233175).
01b,29jun10, my_  update for first release.
01a,28jun10, z_l  created.
*/

/*
DESCRIPTION
This file contains support routines for the Prime Cell L2 cache controller
(PL310). Some APIs for cache operation, such as flush/clear/invalidate,
are implemented in this file. These APIs are used in VxWorks arch library to
support L2 cache.

The following are atomic operations:

    Clean Line by PA or by Index/Way
    Invalidate Line by PA
    Clean and Invalidate Line by PA or by Index/Way
    Cache Sync

These operations stall the slave ports until they are complete. When these 
registers are read, bit [0], the C flag, indicates that a background  operation 
is in progress. So at the end of each operation, we wait for the background 
operation is complete.

All cache maintenance operations automatically perform a Cache Sync 
operation before starting. Before writing to any other register you must 
perform an explicit Cache Sync operation. So at the end of operation, we perform 
an explicit Cache Sync operation by writing 0 to Cache Sync register.
*/

#include <vxWorks.h>
#include "config.h"

/* locals */

LOCAL SPIN_LOCK_ISR_DECL(L2CacheLock, 0);

/* L2 Cache Controller Base Address */

/* register definitions */
 
#define L2_CACHE_ID              (L2_CTRL_BASE + 0x0)
#define L2_CACHE_TYPE            (L2_CTRL_BASE + 0x4)
#define L2_CONTROL               (L2_CTRL_BASE + 0x100)
#define L2_AUX_CONTROL           (L2_CTRL_BASE + 0x104)
#define L2_INT_MASK              (L2_CTRL_BASE + 0x214)
#define L2_INT_CLEAR             (L2_CTRL_BASE + 0x220)
 
#define L2_CACHE_SYNC            (L2_CTRL_BASE + 0x730)
#define L2_INVALIDATE_PA         (L2_CTRL_BASE + 0x770)
#define L2_INVALIDATE_WAY        (L2_CTRL_BASE + 0x77c)
#define L2_CLEAN_PA              (L2_CTRL_BASE + 0x7b0)
#define L2_CLEAN_WAY             (L2_CTRL_BASE + 0x7bc)
#define L2_CLEAN_INVALIDATE_PA   (L2_CTRL_BASE + 0x7f0)
#define L2_CLEAN_INVALIDATE_WAY  (L2_CTRL_BASE + 0x7fc)
 
#define L2_CONTROL_ENABLE        (0x1)
#define L2_WAY_ALL               (0xffff) /* 8-ways */
#define L2_OPERATION_IN_PROGRESS (0x1)
#define L2_INT_MASK_ALL          (0x0)
#define L2_INT_CLEAR_ALL         (0x1ff)
 
#define L2_CACHE_LINE_SIZE       (0x20)
#define L2_CACHE_LINE_MASK       (L2_CACHE_LINE_SIZE - 1)

#ifdef ARMBE8
#   define PL310_REGISTER_READ(reg) \
    vxbSwap32(*(volatile UINT32 *)(reg))
#else /* LE */
#   define PL310_REGISTER_READ(reg) \
    *(volatile UINT32 *)(reg)
#endif /* ARMBE8 */

#ifdef ARMBE8
#   define PL310_REGISTER_WRITE(reg, data)\
    *(volatile UINT32 *)(reg) = vxbSwap32(data)
#else /* LE */ 
#   define PL310_REGISTER_WRITE(reg, data)\
    *(volatile UINT32 *)(reg) = (data)
#endif /* ARMBE8 */

/* 
 * The following five function pointers are defined and used in VxWorks ARM 
 * arch library. To support L2 cache, customer BSP should initialize them with
 * concrete implementation. 
 */
 
IMPORT void (*_pSysL2CacheEnable)(CACHE_TYPE cacheType);
IMPORT void (*_pSysL2CacheDisable)(CACHE_TYPE cacheType);
IMPORT void (*_pSysL2CacheInvFunc)(CACHE_TYPE cacheType, void * start, void * end);
IMPORT void (*_pSysL2CacheFlush)(CACHE_TYPE cacheType, void * start, void * end);
IMPORT void (*_pSysL2CacheClear)(CACHE_TYPE cacheType, void * start, void * end);

/*******************************************************************************
*
* sysL2Invalidate - invalidate L2 Cache
*
* This routine invalidates L2 cache ranging from 'pMemStart' to 'pMemEnd'
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL2Invalidate
    (
    CACHE_TYPE cacheType,
    void * pMemStart,
    void * pMemEnd
    )
    {
    int phyAddr; 
    int tempSize;
    int invalidateSize;
 
    if ((UINT32)pMemEnd == ENTIRE_CACHE) /* invalidate the whole L2 cache */
        {
        SPIN_LOCK_ISR_TAKE(&L2CacheLock);

        PL310_REGISTER_WRITE(L2_INVALIDATE_WAY, L2_WAY_ALL);

        /* poll state until the background invalidate operation is complete */
 
        while (PL310_REGISTER_READ(L2_INVALIDATE_WAY) & L2_WAY_ALL);

        SPIN_LOCK_ISR_GIVE(&L2CacheLock);

        return;
        }
    else
        {
        if ((UINT32)pMemEnd > (UINT32)pMemStart) /* check the parameters */
            invalidateSize = (UINT32)pMemEnd - (UINT32)pMemStart;
        else
            return;
        }
 
    /* use the physical address for L2 cache operation */
    
    phyAddr = mmuVirtToPhys((VIRT_ADDR)pMemStart);
    phyAddr &= ~L2_CACHE_LINE_MASK;
 
    SPIN_LOCK_ISR_TAKE(&L2CacheLock);

    for (tempSize = 0; tempSize < invalidateSize; tempSize += L2_CACHE_LINE_SIZE)
        {
        /* invalidate operation */
        
        PL310_REGISTER_WRITE(L2_INVALIDATE_PA, (phyAddr + tempSize));
        
        /* wait for completion */

        while (PL310_REGISTER_READ(L2_INVALIDATE_PA) & L2_OPERATION_IN_PROGRESS);
        }
 
    /* check the boundary cache line */
    
    if (((L2_CACHE_LINE_SIZE - ((UINT32)pMemStart % L2_CACHE_LINE_SIZE)) + 
        invalidateSize % L2_CACHE_LINE_SIZE) > L2_CACHE_LINE_SIZE)
        {
        /* invalidate operation */
        
        PL310_REGISTER_WRITE(L2_INVALIDATE_PA, (phyAddr + tempSize)); 
    
        /* wait for completion */
    
        while (PL310_REGISTER_READ(L2_INVALIDATE_PA) & L2_OPERATION_IN_PROGRESS);
        }
 
    /* cache sync */
     
    PL310_REGISTER_WRITE(L2_CACHE_SYNC, 0);

    /* wait for completion */

    while (PL310_REGISTER_READ(L2_CACHE_SYNC) & L2_OPERATION_IN_PROGRESS);

    SPIN_LOCK_ISR_GIVE(&L2CacheLock);
    }

/*******************************************************************************
*
* sysL2Flush - flush L2 Cache
*
* This routine flushes L2 cache ranging from 'pMemStart' to 'pMemEnd'
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL2Flush
    (
    CACHE_TYPE cacheType,
    void * pMemStart,
    void * pMemEnd
    )
    {
    int phyAddr;
    int tempSize;
    int flushSize;
 
    if ((UINT32)pMemEnd == ENTIRE_CACHE) /* flush the whole L2 cache */
        {
        SPIN_LOCK_ISR_TAKE(&L2CacheLock);

        PL310_REGISTER_WRITE(L2_CLEAN_WAY, L2_WAY_ALL);
 
        /* poll state until the background flush operation  is complete */
        
        while (PL310_REGISTER_READ(L2_CLEAN_WAY) & L2_WAY_ALL);

        SPIN_LOCK_ISR_GIVE(&L2CacheLock);

        return;
        }
    else
        {
        if ((UINT32)pMemEnd > (UINT32)pMemStart) /* check the parameters */
            flushSize = (UINT32)pMemEnd - (UINT32)pMemStart;
        else
            return;
        }
 
    /* use the physical address for L2 cache operation */
    
    phyAddr = mmuVirtToPhys((VIRT_ADDR)pMemStart);
    phyAddr &= ~L2_CACHE_LINE_MASK;

    SPIN_LOCK_ISR_TAKE(&L2CacheLock); 

    for (tempSize = 0; tempSize < flushSize; tempSize += L2_CACHE_LINE_SIZE)
        {
        /* flush operation */
        
        PL310_REGISTER_WRITE(L2_CLEAN_PA, (phyAddr + tempSize)); 

        /* wait for completion */

        while (PL310_REGISTER_READ(L2_CLEAN_PA) & L2_OPERATION_IN_PROGRESS);     
        }
 
    /* check the boundary cache line */
    
    if (((L2_CACHE_LINE_SIZE - ((UINT32)pMemStart % L2_CACHE_LINE_SIZE)) + 
        flushSize % L2_CACHE_LINE_SIZE) > L2_CACHE_LINE_SIZE)
        {
        /* flush operation */
        
        PL310_REGISTER_WRITE(L2_CLEAN_PA, (phyAddr + tempSize));

        /* wait for completion */

        while (PL310_REGISTER_READ(L2_CLEAN_PA) & L2_OPERATION_IN_PROGRESS);             
        }
 
    /* cache sync */
     
    PL310_REGISTER_WRITE(L2_CACHE_SYNC, 0);

    /* wait for completion */

    while (PL310_REGISTER_READ(L2_CACHE_SYNC) & L2_OPERATION_IN_PROGRESS);

    SPIN_LOCK_ISR_GIVE(&L2CacheLock);
    }
 
/*******************************************************************************
*
* sysL2Clear - clear L2 Cache
*
* This routine clears L2 cache ranging from 'pMemStart' to 'pMemEnd'
*
* NOTE Clear operation is flush + invalidate
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL2Clear
    (
    CACHE_TYPE cacheType,
    void * pMemStart,
    void * pMemEnd
    )
    {
    int phyAddr;
    int tempSize;
    int clearSize;
 
    if ((UINT32)pMemEnd == ENTIRE_CACHE) /* clear the whole L2 cache */
        {
        SPIN_LOCK_ISR_TAKE(&L2CacheLock);

        PL310_REGISTER_WRITE(L2_CLEAN_INVALIDATE_WAY, L2_WAY_ALL);
 
        /* poll state until the background clear operation  is complete */
        
        while (PL310_REGISTER_READ(L2_CLEAN_INVALIDATE_WAY) & L2_WAY_ALL);
        
        SPIN_LOCK_ISR_GIVE(&L2CacheLock);

        return;
        }
    else
        {
        if ((UINT32)pMemEnd > (UINT32)pMemStart) /* check the parameters */
            clearSize = (UINT32)pMemEnd - (UINT32)pMemStart;
        else
            return;
        }
 
    /* use the physical address for L2 cache operation */
    
    phyAddr = mmuVirtToPhys((VIRT_ADDR)pMemStart);
    phyAddr &= ~L2_CACHE_LINE_MASK;
 
    SPIN_LOCK_ISR_TAKE(&L2CacheLock);

    for (tempSize = 0; tempSize < clearSize; tempSize += L2_CACHE_LINE_SIZE)
        {
        /* clear operation */
         
        PL310_REGISTER_WRITE(L2_CLEAN_INVALIDATE_PA, (phyAddr + tempSize));

        /* wait for completion */

        while (PL310_REGISTER_READ(L2_CLEAN_INVALIDATE_PA) & L2_OPERATION_IN_PROGRESS);              
        }
 
    /* check the boundary cache line */
    
    if (((L2_CACHE_LINE_SIZE - ((UINT32)pMemStart % L2_CACHE_LINE_SIZE)) + 
        clearSize % L2_CACHE_LINE_SIZE) > L2_CACHE_LINE_SIZE)
        {
        /* clear operation */
         
        PL310_REGISTER_WRITE(L2_CLEAN_INVALIDATE_PA, (phyAddr + tempSize));

        /* wait for completion */

        while (PL310_REGISTER_READ(L2_CLEAN_INVALIDATE_PA) & L2_OPERATION_IN_PROGRESS);      
        }
 
    /* cache sync */
     
    PL310_REGISTER_WRITE(L2_CACHE_SYNC, 0);

    /* wait for completion */

    while (PL310_REGISTER_READ(L2_CACHE_SYNC) & L2_OPERATION_IN_PROGRESS);

    SPIN_LOCK_ISR_GIVE(&L2CacheLock);
    }

/*******************************************************************************
*
* sysL2Enable - enable L2 Cache
*
* This routine enables the external L2 cache connected to Cortex A9 processor
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL2Enable
    (
    CACHE_TYPE cacheType
    )
    {
    SPIN_LOCK_ISR_TAKE(&L2CacheLock);

    PL310_REGISTER_WRITE(L2_CONTROL, L2_CONTROL_ENABLE); /* enable L2 cache */

    SPIN_LOCK_ISR_GIVE(&L2CacheLock);
    }

/*******************************************************************************
*
* sysL2Disable - disable L2 Cache
*
* This routine disables the external L2 cache connected to Cortex A9 processor
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL2Disable
    (
    CACHE_TYPE cacheType
    )
    {
    SPIN_LOCK_ISR_TAKE(&L2CacheLock);

    PL310_REGISTER_WRITE(L2_CONTROL, 0x0); /* disable L2 cache */

    SPIN_LOCK_ISR_GIVE(&L2CacheLock);
    }

/*******************************************************************************
*
* sysL2CacheInit - init L2 cache support
*
* This routine initializes L2 cache support for Cortex A9
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL2CacheInit(void)
    {
    UINT32 val;

    PL310_REGISTER_WRITE(L2_CONTROL, 0x0); /* disable L2 cache */
 
    val = PL310_REGISTER_READ(L2_AUX_CONTROL);
    val &=  ~(0x000F0000);
    val |= 0x0A130000;
    PL310_REGISTER_WRITE(L2_AUX_CONTROL,val); /* 16-way associativity. way-size is 16KB */

    /*
     * We use the default configuration, so here we do not need use the 
     * auxiliary control register to configure associativity, way size and 
     * latency of RAM access.
     */
 
    /* invalidate the whole L2 cache through way operation */
    
    PL310_REGISTER_WRITE(L2_INVALIDATE_WAY, L2_WAY_ALL);
 
    /* poll state until the background invalidate operation is complete */
    
    while (PL310_REGISTER_READ(L2_INVALIDATE_WAY) & L2_WAY_ALL);
 
    /* clear all pending interrupts */
    
    PL310_REGISTER_WRITE(L2_INT_CLEAR, L2_INT_CLEAR_ALL);
 
    /* mask all interrupts */
 
    PL310_REGISTER_WRITE(L2_INT_MASK, L2_INT_MASK_ALL);
 
    _pSysL2CacheInvFunc  = sysL2Invalidate;
    _pSysL2CacheEnable   = sysL2Enable;
    _pSysL2CacheDisable  = sysL2Disable;
    _pSysL2CacheFlush    = sysL2Flush;
    _pSysL2CacheClear    = sysL2Clear;
 
    }

