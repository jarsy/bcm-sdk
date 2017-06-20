/*
 * $Id: nlmcmtwolevelhashtbl.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 

#include "nlmcmtwolevelhashtbl.h"



/*
The following hash table is meant to allocate memory on demand.

The hash table cannot handle overflows.

A typical scenario where the two level hash table can be used is 
the case where we need to store a table to store a maximum of 
65536 ids. 

We can use a simple array where we preallocate memory for all the
65536 ids before hand. Problem with this approach is that there is too
much space wastage before hand, in case the number of ids actually used
is less.

Here we prereserve space for the first level of the hash table. 
The first level of the hash table has fewer entries than the original 
size of the table. We initially allocate space for only the first level.
The first level of the hash table then contains pointers to the second level 
of the hash table. The second level pointers are allocated on demand

For example, if there are 64k ids, we allocate space for 4k entries in the 
first level. Each entry in the first level is a pointer to 64k/4k i.e. 16 
entries in the second level. So entry 0 in first level is a pointer to
the second level which has ids 0-15. Entry 1 in first level is a pointer to
the second level which has ids 16-31.


Lot of void pointers are used, to allow us to store data of any type
that is associated with the id. For instance we could store the pointer
to the first element in the linked list / binary tree associated with the
the id. The first level and second level are casted as void**

*/



/*
Initializes the two level hash table by allocating memory only for 
the first level of the hash table 
*/
void* NlmCmTwoLevelHashTbl__Init(
                nlm_u32 totalSize,
                nlm_u32 nrOfSecondLevelElems,
                NlmCmAllocator *alloc_p,
                NlmCmTwoLevelHashTbl__DestroyElem_t destroyElemFn_p,
                NlmReasonCode   *o_reason_p)
{
    nlm_u32 nrOfFirstLevelElems =  (totalSize / nrOfSecondLevelElems) + 1;
    
    NlmCmTwoLevelHashTbl* self_p =  
        (NlmCmTwoLevelHashTbl*) NlmCmAllocator__calloc(alloc_p,1, 
                                        sizeof(NlmCmTwoLevelHashTbl));

    if(!self_p)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;

        return NULL;
    }

    self_p->m_firstLevel_pp = (void**)NlmCmAllocator__calloc(alloc_p, 1, 
                                    sizeof(void*) * nrOfFirstLevelElems);

    if(!self_p->m_firstLevel_pp)
    {
        NlmCmAllocator__free(alloc_p, self_p);

        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;

        return NULL;
    }

    self_p->m_nrOfLevel2Elems = nrOfSecondLevelElems;
    self_p->m_totalSize = totalSize;
    self_p->m_alloc_p = alloc_p;
    self_p->m_destroyElemFn_p = destroyElemFn_p;

    return self_p;
}


/*
Inserts the pointer into the hash table at the specified index
*/
NlmErrNum_t NlmCmTwoLevelHashTbl__Insert(
                        NlmCmTwoLevelHashTbl* self_p, 
                        nlm_u32 index,
                        void* insertPtr,
                        NlmReasonCode   *o_reason_p)
{
    nlm_u32 firstLevelIdx   = index / self_p->m_nrOfLevel2Elems;
    nlm_u32 secondLevelIdx  = index % self_p->m_nrOfLevel2Elems;
    void** nextLevel_pp = NULL;

    if(index >= self_p->m_totalSize)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }

    /*Allocate memory for the second level in case it doesn't already exist */
    if(!self_p->m_firstLevel_pp[firstLevelIdx])
    {
        self_p->m_firstLevel_pp[firstLevelIdx] = 
                (void*) NlmCmAllocator__calloc(self_p->m_alloc_p,1, 
                        sizeof(void*) * self_p->m_nrOfLevel2Elems);
    }

    nextLevel_pp = (void**) self_p->m_firstLevel_pp[firstLevelIdx];

    nextLevel_pp[secondLevelIdx] = insertPtr;

    return NLMERR_OK;

}

/*
Returns the pointer stored at the specified index location
In case the index location is more than totalSize, NULL is 
returned.
*/
void* NlmCmTwoLevelHashTbl__GetMember(
                            NlmCmTwoLevelHashTbl* self_p, 
                            nlm_u32 index,
                            NlmReasonCode *o_reason_p)
{
    nlm_u32 firstLevelIdx   = index / self_p->m_nrOfLevel2Elems;
    nlm_u32 secondLevelIdx  = index % self_p->m_nrOfLevel2Elems;
    void** nextLevel_pp = NULL;

    if(index >= self_p->m_totalSize)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_INVALID_INPUT;
        return NULL;
    }

    /* return a NULL if the memory has not yet been allocated */
    if(!self_p->m_firstLevel_pp[firstLevelIdx])
         return NULL;

     nextLevel_pp = (void**) self_p->m_firstLevel_pp[firstLevelIdx];

     return nextLevel_pp[secondLevelIdx]; 
}


/*
Frees the allocated memory. Since the pointer stored in the 
hash table also needs to be freed, this pointer is passed
to the function that was registered during init. The function
can do the necessary clean up.
*/
void NlmCmTwoLevelHashTbl__Destroy(
                            NlmCmTwoLevelHashTbl* self_p                                
                                   )
{
    nlm_u32 firstLevelIdx = 0;
    nlm_u32 secondLevelIdx = 0;
    void** nextLevel_pp = NULL;
    nlm_u32 nrOfFirstLevelElems =  
                    (self_p->m_totalSize / self_p->m_nrOfLevel2Elems) + 1;

    while(firstLevelIdx < nrOfFirstLevelElems)
    {
        if(self_p->m_firstLevel_pp[firstLevelIdx] != NULL)
        {
            nextLevel_pp = (void**) self_p->m_firstLevel_pp[firstLevelIdx];
            secondLevelIdx = 0;

            while(secondLevelIdx < self_p->m_nrOfLevel2Elems )
            {
                /*Free the pointers stored in the second level */
                if(nextLevel_pp[secondLevelIdx] != NULL && self_p->m_destroyElemFn_p)
                    self_p->m_destroyElemFn_p(self_p->m_alloc_p, 
                                                nextLevel_pp[secondLevelIdx]);
                ++secondLevelIdx ;
            }

            /*Free the second level */
            NlmCmAllocator__free(self_p->m_alloc_p, 
                                    self_p->m_firstLevel_pp[firstLevelIdx]);
        }
        ++firstLevelIdx;
    }

    /*Free the first level */
    NlmCmAllocator__free(self_p->m_alloc_p, self_p->m_firstLevel_pp);

    NlmCmAllocator__free(self_p->m_alloc_p, self_p);

}



