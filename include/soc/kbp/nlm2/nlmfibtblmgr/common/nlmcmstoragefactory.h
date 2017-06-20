/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 **************************************************************************************
 Copyright 2009-2012 Broadcom Corporation

 This program is the proprietary software of Broadcom Corporation and/or its licensors,
 and may only be used, duplicated, modified or distributed pursuant to the terms and 
 conditions of a separate, written license agreement executed between you and 
 Broadcom (an "Authorized License").Except as set forth in an Authorized License, 
 Broadcom grants no license (express or implied),right to use, or waiver of any kind 
 with respect to the Software, and Broadcom expressly reserves all rights in and to 
 the Software and all intellectual property rights therein.  
 IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY 
 WAY,AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization, constitutes the 
    valuable trade secrets of Broadcom, and you shall use all reasonable efforts to 
    protect the confidentiality thereof,and to use this information only in connection
    with your use of Broadcom integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH 
    ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER 
    EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM 
    SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, 
    NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. 
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS 
    BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES 
    WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE 
    THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF 
    OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
    ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************************
 */
 

/*@@NlmCmStorage Factory Module

   Summary
   This module creates a storage factory.

   Description
   <B>IMPORTANT READ THIS BEFORE USING THIS MODULE</B>

   This module is intended for creation of very large numbers of (hundreds of thousands or more)
   of similarly sized items that are all small (e.g., less than 128 bytes). It is not intended
   to be used as a factory that produces large allocations (it is not a general purpose
   malloc/free).
   
   The benefit of using this module is to reduce the storage overhead caused by malloc (most
   malloc implementations add an 8 byte overhead). However, before using this module, pay very
   close attention to alignment.
   
   Most systems require a minimum alignment of 4 bytes. However, other systems require an
   alignment of 8 bytes, depending on what is in the data structure that you are allocating the
   memory for. If you have items larger than 4-bytes long, then your alignment should be 8 bytes
   (for instance if your data structure contains doubles).

   And, when memory is required by this module, it is allocated in the chunk size specified
   during creation. If you are using this module for fixed size items, then this chunk size
   should be moderately-sized. If you are using this module for various sizes (but all small),
   then the chunk size should be small (about 1K bytes)

   * IMPORTANT: *
   Since most mallocs waste a lot of memory when we request a nice power of two number, it may
   be benificial to ask for a chunk size of something like (2^n - malloc-overhead). So this
   could be something like (1024-16) assuming that very few mallocs would go over 16 bytes of
   overhead.

   * BEFORE COMMITTING TO USING THIS MODULE, *
     * UNDERSTAND YOUR NEEDS
     * PROTOTYPE YOUR ALLOCATION PATTERN, AND
     * LOOK AT THE MEMORY SAVINGS YOU ARE ACTUALLY GETTING.

   UNLESS YOU ARE CREATING LARGE NUMBERS OF ITEMS, DO NOT USE THIS MODULE. See the member
   variables m_aTotalMemInUse and m_aClientMemInUse to get some idea of the wastage caused by
   this module. It should be on the order of (N * ChunkSize) if this number is a relatively
   small number compared to the number of items allocated (m_aNumItemsInUse) then you are okay.

   When memory is deallocated from this module, it remains in the free pool of this module and
   is not returned to the global malloc/free pool unless an entire block (all items that came from
   this block must have been freed) can be returned to the system. This is important to understand.

   * FAST MODE: *
   There is also a mode to allow for better performance but it uses 4 extra bytes per allocation.
   We have noticed that you can gain upto 5-10% in runtime by using the fast method but obviously,
   there is a memory footprint impact. Still, the impact is less than using the regular malloc/free

   * MAX ALLOC SIZE *
   Although this module is intended for large amount of small items, if you use it in
   a bad way, it adjusts to aocomodate but the overall performancec may degrade. To avoid
   this you can set the maximum block size this allocator should allocate. See the
   documentation for NlmCmStorageFactory__SetMaxAllocSize() for more info.
   Note that we do not keep stats for items that are allocated using the base-allocator

 */

#ifndef INCLUDED_NLMCMSTORAGEFACTORY_H
#define INCLUDED_NLMCMSTORAGEFACTORY_H

#include <nlmcmbasic.h>
#include <nlmcmarray_ptr.h>
#include <nlmcmrbtree_ptr.h>

#include <nlmcmexterncstart.h> /* this should be the final include */

typedef struct NlmCmStorageFactory
{
    NlmCmAllocator *	m_alloc_p ;

/*## Statistics maintained by this module */
    /* Total memory currently allocated from the system */
    nlm_u32		m_nTotalMemInUse ;

    /* Total memory currently allocated by the client of this module */
    nlm_u32		m_nClientMemInUse ;

    /* Peak memory usage by clients (peak value for m_nClientMemInUse) */
    nlm_u32		m_nClientPeakMemInUse ;

    /* Total number of alloc calls */
    nlm_u32		m_nNumAllocs ;

    /* Total number of dealloc calls */
    nlm_u32		m_nNumDeallocs ;

    /*
     * Total memory currently allocated by the client (including
     * any extra memory required for no-mans lands in debug mode).
     * Note that this does not include any overhead incurred by
     * the supplied allocator in the ctor() or any mnemory overhead
     * incurred by the leak detector.
     */
    nlm_u32		m_nClientMemInUseDbg ;
    nlm_u32		m_nClientPeakMemInUseDbg ;

    /* Number of currently active items allocated by the client */
    nlm_u32		m_nNumItemsInUse ;

    /* Number of items currently in the free pool */
    nlm_u32		m_nNumItemsInFreePool ;

    /* Amount of memory sitting various free pools */
    nlm_u32		m_nMemInFreePool ;
/*## End of statistics */

/*## Private items...please do not disturb */
    NlmCmArrayPtr	m_pvt_buckets ;
    NlmCmArrayPtr	m_pvt_buckets_save ;	/* Free blocks */

    /* The following rbtree is only used in the slow mode */
    NlmCmRBTreePtr	m_pvt_storage_tree_space ;
    NlmCmRBTreePtr *	m_pvt_storage_tree_p ;
    struct NlmCmRBTreeSFDbgInfo *
    /**/		m_pvt_debug_info_p ;	/* For Debug purposes only */
    nlm_u32		m_pvt_nChunkSize ;
    nlm_u32		m_pvt_nInvMask ;
    NlmBool		m_pvt_bUseFastFree ;
    nlm_u32		m_pvt_nAlign ;
    nlm_u32		m_pvt_nShiftAmt ;
    nlm_u32		m_pvt_nMask ;
    nlm_u32		m_pvt_headextra ;
    nlm_u32		m_pvt_tailextra ;
    nlm_u32		m_pvt_overhead ;
    nlm_u32		m_pvt_maxAllocSize ;	/* Largest allocation can be made with this allocator */
    nlm_u32		m_pvt_extraAllocSize ;
    struct NlmCmFactoryStorageInfo *
    /**/		m_pvt_fullBlocks_p ;

#ifdef NLMMT
    NlmCmSemaphore *	m_pvt_lock ;
#endif
} NlmCmStorageFactory ;

extern NlmCmStorageFactory *	NlmCmStorageFactory__ctor(
    NlmCmStorageFactory *self,		/* Ptr to self */
    NlmCmAllocator *alloc,		/* Ptr to allocator */
    nlm_u32 nChunkSize,		/* Allocate memory using these increments */
    nlm_u8 alignment,			/* # of bytes to align ptrs to (must be atleast 4) */
    NlmBool autopurge,			/* If non-zero, return mem. to system ASAP (obsoleted) */
    NlmBool useFastMethod,		/* Enable fast method for frees -- uses 4 extra bytes per alloc */
    NlmBool enableDebugInfo) ;		/* If non-zero, then enable the debug info */

extern void			NlmCmStorageFactory__dtor(
    NlmCmStorageFactory *self) ;

extern NlmCmStorageFactory *	NlmCmStorageFactory__create(
    NlmCmAllocator *,
    nlm_u32 nChunkSize,		/* Allocate memory using these increments */
    nlm_u8 alignment,			/* # of bytes to align ptrs to (must be atleast 4) */
    NlmBool autopurge,			/* If non-zero, return mem. to system ASAP */
    NlmBool useFastMethod,		/* Enable fast method for frees -- uses 4 extra bytes per alloc */
    NlmBool enableDebugInfo) ;		/* If non-zero, then enable the debug info */

extern void			NlmCmStorageFactory__destroy(
    NlmCmStorageFactory* self) ;

/* Allocate memory of a given size. Size of 0 also returns a pointer */
extern void *			NlmCmStorageFactory__allocate(
    NlmCmStorageFactory* self, nlm_u32 sz) ;

/* Deallocate the given memory. Passing in NULL is okay. */
extern void			NlmCmStorageFactory__deallocate(
    NlmCmStorageFactory* self, void *) ;

/* NlmCmStorageFactory__SetMaxAllocSize
 *
 * The function sets the maximum allocation size that should be handled by
 * this allocator
 *
 * There are two major modes for Allocator:
 *
 * Fast mode:
 *     In the fast mode, When an allocation of size > max-size is made, the call
 *     is silently diverted to the base allocator provided during creation
 *
 * Slow mode:
 *     In this mode, the storage factor will resize and adjust but in debug mode,
 *     it will assert.
 *
 * So, in both cases, it will work but it may not be that desirable in the slow mode
 *
 * By default, there is no max-size
 *
 * Note: This function must be called right after construction (no allocations
 * should have been made)
 *
 * Returns the previous maxsize
 */
extern nlm_u32			NlmCmStorageFactory__SetMaxAllocSize(
    NlmCmStorageFactory* self, nlm_u32 sz) ;


/* Summary
   Given a 'ptr' allocated from this factory, determine its size.
   
   Description
   Note that this requires a tree walk and the performance depends on the number of chunks
   (not items but chunks) currently active.

   Returns
   0 if the the 'ptr' does not belong to this factory

   Also sometimes, it may return a proper size even if the ptr has been deallocated from
   the factory (the size determination is done comparing the ptr with the various chunks
   address ranges.

   Note:
   The returned size is always a multiple of 4 (or whatever the factory's alignment is set to).
 */
extern nlm_u32			NlmCmStorageFactory__GetSizeOf(
    NlmCmStorageFactory* self,
    void *ptr) ;

/* The following is depracated */
#define				NlmCmStorageFactory__Purge(self)		(void)self

#define				NlmCmStorageFactory__GetAlloc(self)	(self)->m_alloc_p

/* Summary
  The following will check all the memory leaks. This function
  will do nothing if debug info is not enabled during the creation
  of the factory. It also does not do anything if there are no
  leaks. If the input 'fp' is NULL, it uses stdout. Use maxleaks
  to limit the number of leaks printed (use 0 for default)

  If sortedByAllocCount is set to false, it will print the leaks
  sorted by address and sorted by allocate count otherwise. Note
  that it is expensive to sort by allocate count whereas sorting
  by address is pretty much free.

  Returns: The number of leaks actually detected.
*/
extern nlm_u32			NlmCmStorageFactory__PrintLeaks(
    NlmCmStorageFactory* self,
    NlmCmFile *fp,
    nlm_u32 maxleaks,
    NlmBool sortedByAllocCount) ;

/* Returns number of memory leaks (objects that have not yet been freed) */
extern nlm_u32			NlmCmStorageFactory__GetNumLeaks(
    NlmCmStorageFactory* self) ;

/* Summary
   Calls NlmCmBasic__DebugBreak() on the nth item is allocated or deallocated.
   Using -1 to disable this feature. To use this feature, 'enableDebugInfo'
   must be true during the creation of the factory (which is forced true in
   debug builds)

   Returns the last setting
*/
extern nlm_32			NlmCmStorageFactory__SetBreakAllocNth(
    NlmCmStorageFactory* self,
    nlm_32 nth) ;
/* Summary
   Calls NlmCmBasic__DebugBreak() on when the given ptr is allocated or
   deallocated. Using 0 to disable this feature. To use this feature,
   'enableDebugInfo' must be true during the creation of the factory
   (which is forced true in debug builds)

   Returns the last setting
*/
extern void *			NlmCmStorageFactory__SetBreakAllocPtr(
    NlmCmStorageFactory* self,
    void *ptr) ;

/* Summary
   Disables debug mode and frees up any memory that was dedicated
   to help debug. Once disabled, you cannot re-enable debug mode
*/
extern void			NlmCmStorageFactory__DisableDebugInfo(
    NlmCmStorageFactory* self) ;


/* Returns to the system any large chunks that can are fully deallocated. */
extern NlmErrNum_t		NlmCmStorageFactory__Verify(void) ;

extern void NlmCmStorageFactory__PrintInfo(NlmCmStorageFactory *self, FILE *fp);

#include <nlmcmexterncend.h>
#endif
