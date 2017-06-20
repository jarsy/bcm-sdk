/*
 *
 * ===========================================================
 * ==  sbQe2000ElibVlanMem.c - elib VLAN Memory Management  ==
 * ===========================================================
 *
 * WORKING REVISION: $Id: sbQe2000ElibVlanMem.c,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbQe2000ElibVlanMem.c
 *
 * ABSTRACT:
 *
 *     elib public API for VLAN Memory Management
 *
 *     The memory management scheme works as follows:
 *
 *     Structure:
 *         AVL Tree:
 *           Each node in the tree describes a free block of memory.
 *           Said node has the number of lines that contained in the block,
 *           as well as the starting address (within the BF memory) of the
 *           block.
 *         Doubly Linked List:
 *           The nodes within the aforementioned tree have previous and next
 *           pointers to allow for a DLL.  This DLL is used to coalesce
 *           adjacent free blocks, reducing fragmentation, and allowing for
 *           larger blocks to be allocated.
 *
 *     Initialization:
 *           We start with a single node avl tree.  This is deemed the 'free'
 *           block tree.  The node has the entire Switch Context Record Memory.
 *           (15K lines, each line is 64 bits).  We also start with this node
 *           as the head of our doubly linked list.
 *
 *     Allocation:
 *           We execute a best fit search of the free tree.
 *           The node returned to us is removed from the tree.  If the node
 *           returned to us is larger than our need, we split off the
 *           unused portion and add said portion back into the tree.
 *           We mark the allocated portion used, insert the remainder
 *           into the DLL (the allocated node was already in the DLL).
 *           We then return the BF memory address to the user.
 *
 *     ReAlloc:
 *           If the realloc size is identical, we simply return the same
 *           address to the user.
 *           Otherwise, we free the associated block, (with possible
 *           coalescence), and allocate a fresh block.
 *           One item of note, this implementation of realloc does not
 *           copy the original contents of the memory to the new location.
 *
 *     Free:
 *           Find the node (within the DLL) associated with this block of
 *           memory.  If the previous block is free also, coalesce the two
 *           blocks into one.  If the next block is free, coalesce that block
 *           into this one.  Mark the memory as free in the DLL, and add it to
 *           the free tree.
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     21-June-2005
 *     Rewritten to use AVL free tree on:
 *     27-July-2005
 *
 */
#include <shared/bsl.h>

#include "sbWrappers.h"
#include "sbQe2000Elib.h"
#include "sbTypes.h"
#include "sbQe2000ElibMem.h"
#include "sbQe2000ElibContext.h"
#include "sbQe2000ElibZf.h"
#include <soc/error.h>

#define BF_MEM_USED (0x80000000)
#define BF_VRT_MEM_SIZE (15 * 1024)

/*
 * Internally used functions
 */
static sbQe2000ElibBfMemAvl_pst sbQe2000ElibVlanMemAllocDLLSearch(SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulAddr);
static int sbQe2000ElibVlanMemAllocDLLInsert(SB_QE2000_ELIB_CONTEXT_PST pEp, sbQe2000ElibBfMemAvl_pst pBfMemAvl);
static int sbQe2000ElibVlanMemAllocDLLRemove(SB_QE2000_ELIB_CONTEXT_PST pEp, sbQe2000ElibBfMemAvl_pst pBfMemAvl);
static int sbQe2000ElibBfAvlCompare(void *unit, shr_avl_datum_t *a, shr_avl_datum_t *b);
static int sbQe2000ElibVlanMemAvlRemove(SB_QE2000_ELIB_CONTEXT_PST pEp, sbQe2000ElibBfMemAvl_pst pBfMemAvl);
static void sbQe2000ElibVlanMemListTree(shr_avl_entry_t *a,int m);


/*
 * Doubly Linked List functions
 */

static sbQe2000ElibBfMemAvl_pst sbQe2000ElibVlanMemAllocDLLSearch(SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulAddr)
{
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;
    uint32 ulCurrAddr;

    SB_ASSERT(pEp);

    pBfMemAvl = pEp->pBfMemList;

    while(NULL != pBfMemAvl)
    {
        ulCurrAddr = pBfMemAvl->ulAddr & ~BF_MEM_USED;
        if(ulCurrAddr == ulAddr)
        {
            return( pBfMemAvl );
        }

        pBfMemAvl = pBfMemAvl->pNext;
    }

    /*
     * If we got here, we didn't find the appropriate entry
     */
    return( NULL );
}

static int sbQe2000ElibVlanMemAllocDLLInsert(SB_QE2000_ELIB_CONTEXT_PST pEp, sbQe2000ElibBfMemAvl_pst pBfMemAvl)
{
    sbQe2000ElibBfMemAvl_pst pBfMemAvlCurr;
    uint32 ulCurrAddr;
    uint32 ulInsertAddr;

    SB_ASSERT(pEp);
    SB_ASSERT(pBfMemAvl);

    pBfMemAvlCurr = pEp->pBfMemList;

    /*
     * Search for the first entry that has an address that is greater
     * than our insert address.  Once found, back track one entry and
     * insert the new one before the found
     * entry.  If we don't find said entry, insert it at the tail.
     * We assume we cannot have the same address twice in the DLL.
     */
    ulInsertAddr = pBfMemAvl->ulAddr & ~BF_MEM_USED;
    while(NULL != pBfMemAvlCurr)
    {
        ulCurrAddr = pBfMemAvlCurr->ulAddr & ~BF_MEM_USED;

        if(ulCurrAddr > ulInsertAddr)
        {
            pBfMemAvlCurr = pBfMemAvlCurr->pPrev;
            break;
        }

        if(NULL == pBfMemAvlCurr->pNext)
        {
            break;
        }
        pBfMemAvlCurr = pBfMemAvlCurr->pNext;
    }

    /*
     * Insert after the entry we found.
     * Since this is an ordered list, and the head is initialized
     * We need not worry about inserting at the head.
     */
    pBfMemAvl->pPrev = pBfMemAvlCurr;
    pBfMemAvl->pNext = pBfMemAvlCurr->pNext;
    pBfMemAvlCurr->pNext = pBfMemAvl;
    if(NULL != pBfMemAvl->pNext)
    {
        pBfMemAvl->pNext->pPrev = pBfMemAvl;
    }

    return( 0 );
}

static int sbQe2000ElibVlanMemAllocDLLRemove(SB_QE2000_ELIB_CONTEXT_PST pEp, sbQe2000ElibBfMemAvl_pst pBfMemAvl)
{
    sbQe2000ElibBfMemAvl_pst pBfMemAvlPrev;
    sbQe2000ElibBfMemAvl_pst pBfMemAvlNext;

    SB_ASSERT(pEp);
    SB_ASSERT(pBfMemAvl);

    pBfMemAvlPrev = pBfMemAvl->pPrev;
    pBfMemAvlNext = pBfMemAvl->pNext;

    if(NULL != pBfMemAvlPrev)
    {
        pBfMemAvlPrev->pNext = pBfMemAvlNext;
    }

    if(NULL != pBfMemAvlNext)
    {
        pBfMemAvlNext->pPrev = pBfMemAvlPrev;
    }

    return( 0 );

}

static int sbQe2000ElibBfAvlCompare(void *user_data, shr_avl_datum_t *a, 
                                    shr_avl_datum_t *b)
{
    int size_diff;

    COMPILER_REFERENCE(user_data);

    size_diff = (((sbQe2000ElibBfMemAvl_pst)(*a))->nNumLines - 
                 ((sbQe2000ElibBfMemAvl_pst)(*b))->nNumLines);

    if (!size_diff) {
        /* both entries are of same size...check the address. If both have
         * same address (which means its the same entry) return 0 else 1 
         */
        return (!(((sbQe2000ElibBfMemAvl_pst)(*a))->ulAddr == 
                  ((sbQe2000ElibBfMemAvl_pst)(*b))->ulAddr));
    }

    return size_diff;
}

static int sbQe2000ElibBfAvlSearchCompare(shr_avl_datum_t *a, 
                                          shr_avl_datum_t *b)
{
    return (((sbQe2000ElibBfMemAvl_pst)(*a))->nNumLines - 
            ((sbQe2000ElibBfMemAvl_pst)(*b))->nNumLines);
}

/* performs bestfit search of the avl tree to find the free block */
static shr_avl_datum_t* sbQe2000ElibBfAvlSearch(shr_avl_t *tree, 
                                                shr_avl_datum_t *datum)
{
    int                 cmp_rv;
    shr_avl_datum_t     *result = NULL;
    shr_avl_t           sub_tree;

    if ((!tree) || (!tree->root)) {
        return NULL;
    }

    cmp_rv = sbQe2000ElibBfAvlSearchCompare(&tree->root->datum, datum);

    if (cmp_rv > 0) {
        /* search left sub-tree */
        if (tree->root->left != NULL) {
            sub_tree.root = tree->root->left;
            result = sbQe2000ElibBfAvlSearch(&sub_tree, datum);
            if (result != NULL) {
                cmp_rv = sbQe2000ElibBfAvlSearchCompare(result, datum);
                if (cmp_rv < 0) {
                    result = &tree->root->datum;
                }
            }
        } else {
            result = &tree->root->datum;
        }
    }

    if (cmp_rv == 0) {
        result = &tree->root->datum;
    }

    if (cmp_rv < 0) {
        /* search right sub-tree */
        if (tree->root->right != NULL) {
            sub_tree.root = tree->root->right;
            result = sbQe2000ElibBfAvlSearch(&sub_tree, datum);
            if (result != NULL) {
                cmp_rv = sbQe2000ElibBfAvlSearchCompare(result, datum);
                if (cmp_rv < 0) {
                    result = &tree->root->datum;
                }
            }
        } else {
            result = &tree->root->datum;
        }
    }

    return result;
}


static int sbQe2000ElibVlanMemAvlRemove(SB_QE2000_ELIB_CONTEXT_PST pEp, sbQe2000ElibBfMemAvl_pst pBfMemAvl)
{
    int nStatus;

    /*
     * Calls remove root if necessary
     */
    nStatus = shr_avl_delete(pEp->sBfMem, sbQe2000ElibBfAvlCompare, 
                             (shr_avl_datum_t*)&pBfMemAvl);

    return( nStatus );
}


sbElibStatus_et sbQe2000ElibVlanMemInit(SB_QE2000_ELIB_CONTEXT_PST pEp)
{
    int nStatus;
    SB_ASSERT(pEp);
    
    /* create the AVL tree.
       Datum is the ptr to sbQe2000ElibBfMemAvl_st */
    nStatus = shr_avl_create(&pEp->sBfMem, NULL, 
                             sizeof(sbQe2000ElibBfMemAvl_pst), BF_VRT_MEM_SIZE);
    if (nStatus != SOC_E_NONE) {
        return (SB_ELIB_INIT_FAIL);
    }

    /* Initialize and create a single entry with the whole mem */
    nStatus = gen_thin_malloc((void *)pEp->pHalCtx,
                              SB_ALLOC_INTERNAL,
                              sizeof(sbQe2000ElibBfMemAvl_st),
                              (void **)(sbQe2000ElibBfMemAvl_pst*)&(pEp->pBfMemList),
                              NULL,
                              0);

    if(SB_OK != nStatus) {
        return(SB_ELIB_MEM_ALLOC_FAIL);
    }

    SB_MEMSET(pEp->pBfMemList, 0, sizeof(*pEp->pBfMemList));
    /* Initialize doubly linked list (for block coalesence) */
    pEp->pBfMemList->nNumLines = BF_VRT_MEM_SIZE;
    pEp->pBfMemList->ulAddr = SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_OFFSET;
    pEp->pBfMemList->pNext = NULL;
    pEp->pBfMemList->pPrev = NULL;

    /* insert the entry into the avl tree */
    shr_avl_insert(pEp->sBfMem, sbQe2000ElibBfAvlCompare, 
                   (shr_avl_datum_t*)&pEp->pBfMemList);
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanMemUninit(SB_QE2000_ELIB_CONTEXT_PST pEp)
{
    int rv = SB_ELIB_OK;
    int status;
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;
    sbQe2000ElibBfMemAvl_pst pBfMemAvlNext;
    sbDmaMemoryHandle_t tDmaHdl;

    /* destroy the avl tree. Frees the space for datum's also. */
    status = shr_avl_destroy(pEp->sBfMem);
    if (status != SOC_E_NONE) {
        rv = SB_ELIB_INIT_FAIL;
    }
    pEp->sBfMem = NULL;

    /*
     * Now walk the dll and free each entry
     */
    pBfMemAvl = pEp->pBfMemList;

    while(NULL != pBfMemAvl)
    {
        pBfMemAvlNext = pBfMemAvl->pNext;
        tDmaHdl.handle = NULL;
        thin_free((void *)pEp->pHalCtx,
                  SB_ALLOC_INTERNAL,
                  0, /* don't care */
                  pBfMemAvl,
                  tDmaHdl);
        pBfMemAvl = pBfMemAvlNext;
    }
    pEp->pBfMemList = NULL;

    return rv;
}



sbElibStatus_et sbQe2000ElibVlanMalloc(SB_QE2000_ELIB_CONTEXT_PST pEp, int nNumLines, VrtPtr_t *ptVrtPtr)
{
    sbQe2000ElibBfMemAvl_st sBfMemAvl;
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;
    sbQe2000ElibBfMemAvl_pst pBfMemAvlSplit;
    void *pTmpVoid;
    int nStatus;
    shr_avl_datum_t datum;
    shr_avl_datum_t *pDatum;

    SB_ASSERT( pEp );
    SB_ASSERT( ptVrtPtr );

    if(0 == nNumLines)
    {
        /*
         * What does malloc do here?
         */
        return( SB_ELIB_GENERAL_FAIL );
    }

    /*
     * Need to search the tree to find the best fit.
     */
    SB_MEMSET(&sBfMemAvl, 0, sizeof(sBfMemAvl));
    sBfMemAvl.nNumLines = nNumLines;
    datum = (shr_avl_datum_t) &sBfMemAvl;

    pDatum = sbQe2000ElibBfAvlSearch(pEp->sBfMem, &datum);
    if(NULL == pDatum) {
        /* OOM */
        return( SB_ELIB_VLAN_MEM_ALLOC_FAIL );
    }
    pBfMemAvl = (sbQe2000ElibBfMemAvl_pst)(*pDatum);

    if( nNumLines > pBfMemAvl->nNumLines )
    {
        /* OOM */
        return( SB_ELIB_VLAN_MEM_ALLOC_FAIL );
    }

    /*
     * Remove the node from the free tree
     */
    /*
     * ENHANCEME: do something with the status from avl_remove
     */
    nStatus = sbQe2000ElibVlanMemAvlRemove(pEp, pBfMemAvl);

    /*
     * We have a 'free' block of memory.  It may be larger than we need,
     * if it is, we need to split off the extra and put that back into
     * the free tree
     */
    if( nNumLines < pBfMemAvl->nNumLines )
    {
        /*
         * The search returned a free block that is greater than
         * what we need.  We need to split off the unused lines,
         * and add that back to the free tree.
         */
        pTmpVoid = NULL;
        nStatus = gen_thin_malloc((void *)pEp->pHalCtx,
                                  SB_ALLOC_INTERNAL,
                                  sizeof(*pBfMemAvlSplit),
                                  (void **)&pTmpVoid,
                                  NULL,
                                  0);

        if(SB_OK != nStatus)
        {
            /* OOSM */
            return( SB_ELIB_MEM_ALLOC_FAIL );
        }

        pBfMemAvlSplit = (sbQe2000ElibBfMemAvl_pst)pTmpVoid;

        SB_MEMSET(pBfMemAvlSplit, 0, sizeof(*pBfMemAvlSplit));
        pBfMemAvlSplit->nNumLines = pBfMemAvl->nNumLines - nNumLines;
        pBfMemAvlSplit->ulAddr = pBfMemAvl->ulAddr + sBfMemAvl.nNumLines;
        pBfMemAvlSplit->pNext = NULL;
        pBfMemAvlSplit->pPrev = NULL;

        /*
         * Since we're splitting, we have to update the 'allocated' node's
         * size appropriately
         */
        pBfMemAvl->nNumLines = nNumLines;


        /*
         * ENHANCEME: do something with the status from avl_insert
         */
        nStatus = shr_avl_insert(pEp->sBfMem, sbQe2000ElibBfAvlCompare, 
                                 (shr_avl_datum_t*)&pBfMemAvlSplit);
        sbQe2000ElibVlanMemAllocDLLInsert(pEp, pBfMemAvlSplit);
    }

    /*
     * Now take care of the block we're interested in
     */
    *ptVrtPtr = pBfMemAvl->ulAddr;
    pBfMemAvl->ulAddr |= BF_MEM_USED;

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanRealloc(SB_QE2000_ELIB_CONTEXT_PST pEp, int nNumLines, VrtPtr_t *ptVrtPtr)
{
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;
    int nStatus;

    SB_ASSERT( pEp );
    SB_ASSERT( ptVrtPtr );

    /*
     * Find our entry in the DLL
     */
    pBfMemAvl = sbQe2000ElibVlanMemAllocDLLSearch(pEp, *ptVrtPtr);
    if(NULL == pBfMemAvl)
    {
        /*
         * This is bad, we couldn't find our
         * allocated block to free
         */
        return( SB_ELIB_VLAN_MEM_FREE_FAIL );
    }

    /*
     * Do a quick check to see if we actually need to realloc
     */
    if(nNumLines == pBfMemAvl->nNumLines)
    {
        /*
         * Just return the same *ptVrtPtr pointer, no realloc
         */
        return( SB_ELIB_OK );
    }

    /*
     * If the size has changed, free the memory (possibly coalescence)
     * and allocate the new size
     */
    nStatus = sbQe2000ElibVlanFree(pEp, pBfMemAvl->nNumLines, ptVrtPtr);
    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    nStatus = sbQe2000ElibVlanMalloc(pEp, nNumLines, ptVrtPtr);

    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibVlanFree(SB_QE2000_ELIB_CONTEXT_PST pEp, int nNumLines, VrtPtr_t *ptVrtPtr)
{
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;
    sbQe2000ElibBfMemAvl_pst pBfMemAvlPrev;
    sbQe2000ElibBfMemAvl_pst pBfMemAvlNext;
    sbDmaMemoryHandle_t tDmaHdl;

    SB_ASSERT( pEp );
    SB_ASSERT( ptVrtPtr );

    /*
     * Search for our DLL entry
     */
    pBfMemAvl = sbQe2000ElibVlanMemAllocDLLSearch(pEp, *ptVrtPtr);
    if(NULL == pBfMemAvl)
    {
        /*
         * This is bad, we couldn't find our
         * allocated block to free
         */
        return( SB_ELIB_VLAN_MEM_FREE_FAIL );
    }

    if( !(BF_MEM_USED & pBfMemAvl->ulAddr) )
    {
        /*
         * Trying to free a free block?
         */
        return( SB_ELIB_VLAN_MEM_FREE_FAIL );
    }

    /*
     * Attempt to coalesce adjacent blocks.
     *
     * We have four scenarios:
     *   1.  Previous block is free
     *   2.  Next block is free
     *   3.  Both Previous & Next blocks are free.
     *   4.  Neither Previous & Next blocks are free.
     */
    pBfMemAvlPrev = pBfMemAvl->pPrev;
    pBfMemAvlNext = pBfMemAvl->pNext;

    if(NULL != pBfMemAvlPrev)
    {
        /* Previous Block is free */
        /*
         * Since we want to ensure that we don't remove the root node
         * of the DLL, we coalesce to to previous node
         */
        if( !(BF_MEM_USED & pBfMemAvlPrev->ulAddr) )
        {
            sbQe2000ElibVlanMemAvlRemove(pEp, pBfMemAvlPrev);
            pBfMemAvlPrev->nNumLines += pBfMemAvl->nNumLines;
            sbQe2000ElibVlanMemAllocDLLRemove(pEp, pBfMemAvl);
            tDmaHdl.handle = NULL;
            thin_free((void *)pEp->pHalCtx,
                      SB_ALLOC_INTERNAL,
                      0, /* don't care */
                      pBfMemAvl,
                      tDmaHdl);
            /*
             * Setup our pointer to look at the coalesced block
             */
            pBfMemAvl = pBfMemAvlPrev;
        }
    }

    if(NULL != pBfMemAvlNext)
    {
        /* Next Block is free */
        if( !(BF_MEM_USED & pBfMemAvlNext->ulAddr) )
        {
            pBfMemAvl->nNumLines += pBfMemAvlNext->nNumLines;
            sbQe2000ElibVlanMemAvlRemove(pEp, pBfMemAvlNext);
            sbQe2000ElibVlanMemAllocDLLRemove(pEp, pBfMemAvlNext);
            tDmaHdl.handle = NULL;
            thin_free((void *)pEp->pHalCtx,
                      SB_ALLOC_INTERNAL,
                      0, /* don't care */
                      pBfMemAvlNext,
                      tDmaHdl);
        }
    }

    /*
     * Remove the used flag
     */
    pBfMemAvl->ulAddr &= ~BF_MEM_USED;

    /*
     * Add to free tree
     */
    shr_avl_insert(pEp->sBfMem, sbQe2000ElibBfAvlCompare, 
                             (shr_avl_datum_t*)&pBfMemAvl);

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanMemset(SB_QE2000_ELIB_CONTEXT_PST pEp, VrtPtr_t *ptVrtPtr,
                                       uint32 ulData0, uint32 ulData1, int nNumLines)
{
    sbElibStatus_et nStatus;
    int i;
    VrtPtr_t tVrtPtr;

    SB_ASSERT( pEp );
    SB_ASSERT( ptVrtPtr );

    tVrtPtr = *ptVrtPtr;


    for( i = 0; i < nNumLines; i++ )
    {
        nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                          tVrtPtr + i,
                                          ulData0,
                                          ulData1);

        if( SB_ELIB_OK != nStatus )
        {
            DEXIT();
            return( nStatus );
        }
    }

    return( SB_ELIB_OK );

}

static void sbQe2000ElibVlanMemListTree(shr_avl_entry_t* entry,int m)
{
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;
    int n=m;

    if(entry==0) {
        return;
    }
    pBfMemAvl = (sbQe2000ElibBfMemAvl_pst)entry->datum;
    if(entry->right) {
        sbQe2000ElibVlanMemListTree(entry->right, m+1);
    }
    while(n--) {
        LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META("   ")));
    }
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("%d (%d)\n"),pBfMemAvl->nNumLines, entry->balance));
    if(entry->left) {
        sbQe2000ElibVlanMemListTree(entry->left, m+1);
    }
}

sbElibStatus_et sbQe2000ElibVlanMemListOutput( SB_QE2000_ELIB_HANDLE Handle )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;
    int nIdx;

    DENTER();
    SB_ASSERT( Handle );

    pBfMemAvl = pEp->pBfMemList;
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("Listing VLAN Memory Nodes:\n")));
    nIdx = 0;
    while(NULL != pBfMemAvl)
    {
        LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META("%i:  Lines: %d  Addr: 0x%x Used: %i This: %p Prev: %p Next %p\n"),
                  nIdx++,
                  pBfMemAvl->nNumLines,
                  pBfMemAvl->ulAddr & ~BF_MEM_USED,
                  (pBfMemAvl->ulAddr & BF_MEM_USED) >> 31,
                  (void *) pBfMemAvl,
                  (void *) pBfMemAvl->pPrev,
                  (void *) pBfMemAvl->pNext));
        pBfMemAvl = pBfMemAvl->pNext;

    }

    DEXIT();
    return( SB_ELIB_OK );

}


sbElibStatus_et sbQe2000ElibVlanMemFreeTreeList( SB_QE2000_ELIB_HANDLE Handle )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;

    DENTER();
    SB_ASSERT( Handle );

    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("Listing Free Tree:\n")));
    sbQe2000ElibVlanMemListTree(pEp->sBfMem->root, 0);

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibVlanMemCheck( SB_QE2000_ELIB_HANDLE Handle,
                                          sbZfSbQe2000ElibVlanMem_t *pZf)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    int nEdges;
    int nPatches;
    int nUsed;
    int nFree;
    sbQe2000ElibBfMemAvl_pst pBfMemAvl;

    DENTER();
    SB_ASSERT( Handle );

    nEdges = 0;
    nPatches = 1;
    nUsed = 0;
    nFree = 0;

    /*
     * Search for edges
     */
    pBfMemAvl = pEp->pBfMemList;
    while(NULL != pBfMemAvl)
    {
        if( NULL != pBfMemAvl->pPrev )
        {
            if( (pBfMemAvl->ulAddr & BF_MEM_USED) !=
                (pBfMemAvl->pPrev->ulAddr & BF_MEM_USED) )
            {
                nEdges++;
            }
        }
        pBfMemAvl = pBfMemAvl->pNext;
    }

    /*
     * Search for patches, and count used & free at the same time
     */
    pBfMemAvl = pEp->pBfMemList;
    while(NULL != pBfMemAvl)
    {
        if( NULL != pBfMemAvl->pNext )
        {
            if( (pBfMemAvl->ulAddr & BF_MEM_USED) !=
                (pBfMemAvl->pNext->ulAddr & BF_MEM_USED) )
            {
                nPatches++;
            }
        }

        if( pBfMemAvl->ulAddr & BF_MEM_USED )
        {
            nUsed++;
        }
        else
        {
            nFree++;
        }

        pBfMemAvl = pBfMemAvl->pNext;
    }


    pZf->m_Edges = nEdges;
    pZf->m_Patches = nPatches;
    pZf->m_NumFree = nFree;
    pZf->m_NumUsed = nUsed;

    DEXIT();
    return( SB_ELIB_OK );
}
