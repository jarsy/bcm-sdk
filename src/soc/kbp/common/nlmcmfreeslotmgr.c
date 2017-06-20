/*
 * $Id: nlmcmfreeslotmgr.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
/*
 *  Free Slot maintains a list of available slots between a start and end
 *  index. The available free slots are maintained in a doubly linked list.
 *  When the list is initialized, it will contain just one node. The list 
 *  later grows or shrinks with the addition or removal of free slots. 
 *
 *  Each node in the list gives information about a chunk of consecutive free 
 *  "record locations".  
 *
 */

#include "nlmcmfreeslotmgr.h"
#include "nlmerrorcodes.h"

/* 
MAX_SIZE_OF_FREELIST defines the maximum size of free list. 
If free list size is greater than the size specified here then
we break that list into two list which inturn introduces holes
between these two list to reduce suffles. Index generated 
will not be sequencial. 

If sequencial indexes are desired then define MAX_SIZE_OF_FREELIST
as 0xFFFFFFFF which will avoid spliting of free list. 

#define MAX_SIZE_OF_FREELIST    0xFFFFFFFF
*/

#define MAX_SIZE_OF_FREELIST    256



/*******************************************************************************
 * Function : NlmFreeSlotInfo__destroyNode 
 *
 * Parameters:
 * void *alloc_p      = Pointer to general purpose memory allocator
 * NlmCmDblLinkList *node_p = Pointer to node in the linked list 
 *
 * Summary:
 * NlmFreeSlotInfo__destroyNode deletes a node from the linked list 
 ******************************************************************************/

void NlmFreeSlotInfo__destroyNode(NlmCmDblLinkList* node_p,  void* alloc_p)
{
    NlmCmAllocator__free((NlmCmAllocator*)alloc_p, (NlmFreeSlotInfo_t*) node_p);
}


/*******************************************************************************
 * Function : NlmFreeSlotMgr__Init 
 *
 * Parameters:
 * NlmCmAllocator *alloc_p   = Pointer to general purpose memory allocator
 * NlmRecordIndex startIndex = Start index for the free slot manager
 * NlmRecordIndex endIndex   = End index for the free slot manager
 * NlmRecordIndex indexIncr  = This actually indicates how many free slot
 *                             a record may occupy
 * NlmReasonCode *o_reason_p = Pointer to save the detailed reason code from
 *                             this function
 * Summary:
 * NlmFreeSlotMgr__Init creates and initializes a free slot manager object and
 * returns pointer to the newly created free slot manager object. 
 ******************************************************************************/

NlmFreeSlotMgr_t* NlmFreeSlotMgr__Init(NlmCmAllocator* alloc_p,
                               NlmRecordIndex startIndex,
                               NlmRecordIndex endIndex,
                               NlmRecordIndex indexIncr,
                               NlmReasonCode *o_reason_p)
{
    NlmFreeSlotMgr_t* freeSlotMgr_p = (NlmFreeSlotMgr_t*)NlmCmAllocator__calloc(alloc_p, 1, 
                                                    sizeof(NlmFreeSlotMgr_t));

    NlmFreeSlotInfo_t* freeSlotNode_p = NULL;
    
    if(!freeSlotMgr_p)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;

        return NULL;
    }

    freeSlotMgr_p->m_head_p = (NlmFreeSlotInfo_t*)NlmCmAllocator__calloc(alloc_p, 1,
                                                    sizeof(NlmFreeSlotInfo_t));

    if(!freeSlotMgr_p->m_head_p)
    {
        NlmCmAllocator__free(alloc_p,freeSlotMgr_p);

        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;

        return NULL;
    }

    NlmCmDblLinkList__Init(
        (NlmCmDblLinkList*)freeSlotMgr_p->m_head_p);

    
    freeSlotNode_p = (NlmFreeSlotInfo_t*)NlmCmAllocator__calloc(alloc_p, 1, 
                                sizeof(NlmFreeSlotInfo_t));

    if(!freeSlotNode_p)
    {
        NlmCmAllocator__free(alloc_p, freeSlotMgr_p->m_head_p);

        NlmCmAllocator__free(alloc_p, freeSlotMgr_p);

        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;

        return NULL;
        
    }
    
    freeSlotNode_p->m_startIndex = startIndex;
    freeSlotNode_p->m_endIndex = endIndex;

    NlmCmDblLinkList__Insert((NlmCmDblLinkList*)freeSlotMgr_p->m_head_p,
                            (NlmCmDblLinkList*) freeSlotNode_p);

    freeSlotMgr_p->m_alloc_p = alloc_p;

    freeSlotMgr_p->m_indexIncr = indexIncr;

    return freeSlotMgr_p;
}

/*******************************************************************************
 * Function : NlmFreeSlotMgr__FindNearestFreeSlot 
 *
 * Parameters:
 *
 * NlmFreeSlotMgr_t * self_p  = Pointer to free slot manager object 
 * NlmRecordIndex markerIndex = index from where the nearest slot to be seached
 * NlmBool isForwardDirection = nearest slot to be searched in which direction
 * NlmRecordIndex *o_bestFreeSlotIndex_p = this is to store the best free slot
 *                              index
 *
 * Summary:
 * NlmFreeSlotMgr__FindNearestFreeSlot looks for a free slot in the list of free
 * slots (maintained by the freeslotmgr)  which is nearest to the markerIndex 
 * passed. It returns NlmTrue if a free slot is found in the list, else
 * returns NlmFalse.
 ******************************************************************************/

NlmBool NlmFreeSlotMgr__FindNearestFreeSlot(NlmFreeSlotMgr_t * self_p,
                                NlmRecordIndex markerIndex,
                                NlmBool isForwardDirection,
                                NlmRecordIndex *o_bestFreeSlotIndex_p)
{
    NlmFreeSlotInfo_t *headNode_p = self_p->m_head_p; 
    NlmFreeSlotInfo_t *firstNode_p = (NlmFreeSlotInfo_t*)headNode_p->m_next_p;
    NlmFreeSlotInfo_t *lastNode_p = (NlmFreeSlotInfo_t*)headNode_p->m_back_p;
    NlmFreeSlotInfo_t *curNode_p = NULL;
    NlmBool found = NlmFalse;

    /*There are no free nodes in the list */
    if(self_p->m_head_p->m_next_p == self_p->m_head_p)
        return found;
    
    if(isForwardDirection)
    {
        /*Process the free list in the forward direction */
        curNode_p = firstNode_p;

        while(curNode_p != headNode_p)
        {
            if(markerIndex <= curNode_p->m_startIndex ||
                (markerIndex > curNode_p->m_startIndex && markerIndex <= curNode_p->m_endIndex))
            {
                if(markerIndex <= curNode_p->m_startIndex)
                     if((curNode_p->m_endIndex - curNode_p->m_startIndex) > MAX_SIZE_OF_FREELIST)

                                        *o_bestFreeSlotIndex_p = (((curNode_p->m_startIndex + curNode_p->m_endIndex)/2)/self_p->m_indexIncr)*self_p->m_indexIncr;
                                      else
                        *o_bestFreeSlotIndex_p = curNode_p->m_startIndex;
                else
                    *o_bestFreeSlotIndex_p = markerIndex;

                found = NlmTrue;
                break;
            }
            curNode_p = (NlmFreeSlotInfo_t*)curNode_p->m_next_p;
        }
        
    }
    else
    {
        /*Process the free list in the backward direction */
        curNode_p = lastNode_p;

        while(curNode_p != headNode_p )
        {
            if( markerIndex >= curNode_p->m_endIndex  ||
                (markerIndex < curNode_p->m_endIndex && markerIndex >= curNode_p->m_startIndex))
            {
                if(markerIndex >= curNode_p->m_endIndex)
                    if((curNode_p->m_endIndex - curNode_p->m_startIndex) > MAX_SIZE_OF_FREELIST)
                                         *o_bestFreeSlotIndex_p = (((curNode_p->m_startIndex + curNode_p->m_endIndex)/2)/self_p->m_indexIncr)*self_p->m_indexIncr;
       
                                   else
                    *o_bestFreeSlotIndex_p = curNode_p->m_endIndex;
                else
                    *o_bestFreeSlotIndex_p = markerIndex;

                found = NlmTrue;
                break;
            }
            curNode_p = (NlmFreeSlotInfo_t*)curNode_p->m_back_p;
        }
    }

    return found;
}


/*******************************************************************************
 * Function : NlmFreeSlotMgr__RemoveFreeSlot 
 *
 * Parameters:
 *
 * NlmFreeSlotMgr_t * self_p  = Pointer to free slot manager object 
 * NlmRecordIndex freeSlotIndex = index of the free slot to be removed
 * NlmReasonCode *o_reason_p = Pointer to save the detailed reason code from
 *                             this function
 *
 * Summary:
 * NlmFreeSlotMgr__RemoveFreeSlot removes a free slot from the list of free
 * slots (maintained by the freeslotmgr)
 ******************************************************************************/

NlmErrNum_t NlmFreeSlotMgr__RemoveFreeSlot(NlmFreeSlotMgr_t * self_p,
                                        NlmRecordIndex freeSlotIndex,
                                        NlmReasonCode *o_reason_p)
{
    NlmFreeSlotInfo_t *headNode_p = self_p->m_head_p; 
    NlmFreeSlotInfo_t *firstNode_p = (NlmFreeSlotInfo_t*)headNode_p->m_next_p;
    NlmFreeSlotInfo_t *curNode_p = firstNode_p;
    NlmFreeSlotInfo_t *newNode_p = NULL;

    while(curNode_p != headNode_p)
    {
        if((freeSlotIndex >= curNode_p->m_startIndex && 
            freeSlotIndex <= curNode_p->m_endIndex))
        {
            if(curNode_p->m_startIndex == curNode_p->m_endIndex )
            {
                /*Delete the following node */
                NlmCmDblLinkList__Remove((NlmCmDblLinkList*)curNode_p,
                                        NlmFreeSlotInfo__destroyNode,
                                        self_p->m_alloc_p); 
            }
            else if(freeSlotIndex == curNode_p->m_startIndex)
            {
                /* Adjust the start index of the node */ 
                curNode_p->m_startIndex += self_p->m_indexIncr ;

                if(curNode_p->m_startIndex > curNode_p->m_endIndex)
                {
                    NlmCmAssert(0, "startIndex is greater than endIndex \n");
                    return NLMERR_FAIL;
                }

            }
            else if(freeSlotIndex == curNode_p->m_endIndex)
            {
                /* Just adjust the end index of the node */
                if(curNode_p->m_startIndex > curNode_p->m_endIndex - self_p->m_indexIncr ||
                    curNode_p->m_endIndex < self_p->m_indexIncr)
                {
                    NlmCmAssert(0, "startIndex is greater than endIndex \n");
                    return NLMERR_FAIL;
                }
                else
                    curNode_p->m_endIndex -= self_p->m_indexIncr ;

                
            }
            else
            {
               /*
                * The removal of the free slot will cause a split in the node 
                * and so a new node is inserted.
                */

                newNode_p = (NlmFreeSlotInfo_t*)NlmCmAllocator__calloc(self_p->m_alloc_p, 1, 
                                            sizeof(NlmFreeSlotInfo_t));
                if(!newNode_p)
                {
                    if(o_reason_p)
                        *o_reason_p = NLMRSC_LOW_MEMORY;

                    return NLMERR_FAIL;
                }

                newNode_p->m_startIndex = freeSlotIndex + self_p->m_indexIncr;
                newNode_p->m_endIndex = curNode_p->m_endIndex;

                curNode_p->m_endIndex = freeSlotIndex - self_p->m_indexIncr;

                NlmCmDblLinkList__Insert((NlmCmDblLinkList*)curNode_p,
                                            (NlmCmDblLinkList*)newNode_p);
            }
            return NLMERR_OK;
        }
        curNode_p = (NlmFreeSlotInfo_t*)curNode_p->m_next_p;
    }

    /*This API is called only when we have found a free slot.
    In case we are not unable to remove the free slot, assert*/
    NlmCmAssert(0, "Free slot to be removed does not exist\n");
    return NLMERR_FAIL;
}

/*******************************************************************************
 * Function : NlmFreeSlotMgr__AddFreeSlot 
 *
 * Parameters:
 *
 * NlmFreeSlotMgr_t * self_p  = Pointer to free slot manager object 
 * NlmRecordIndex freeSlotIndex = index of the free slot to be added
 * NlmReasonCode *o_reason_p = Pointer to save the detailed reason code from
 *                             this function
 *
 * Summary:
 * NlmFreeSlotMgr__AddFreeSlot adds a free slot to the list of free
 * slots (maintained by the freeslotmgr)
 ******************************************************************************/

NlmErrNum_t  NlmFreeSlotMgr__AddFreeSlot(NlmFreeSlotMgr_t * self_p,
                                    NlmRecordIndex freeSlotIndex,
                                    NlmReasonCode *o_reason_p)
{
    NlmFreeSlotInfo_t *curNode_p = (NlmFreeSlotInfo_t*)self_p->m_head_p->m_next_p;
    NlmFreeSlotInfo_t *newNode_p = NULL;

    while(curNode_p != self_p->m_head_p)
    {
        if(curNode_p->m_startIndex > freeSlotIndex)
            break;

        curNode_p = (NlmFreeSlotInfo_t*)curNode_p->m_next_p;
    }

    if(curNode_p != self_p->m_head_p && 
        curNode_p->m_startIndex - self_p->m_indexIncr == freeSlotIndex)
    {
        /*
         * Just adjust the the current nodes start index as the free
         * slot and the start index of current node are consecutive
         * "record locations"
         */ 
        curNode_p->m_startIndex = freeSlotIndex;
        return NLMERR_OK;
    }

    curNode_p = (NlmFreeSlotInfo_t*)curNode_p->m_back_p;

    if(curNode_p != self_p->m_head_p && 
        curNode_p->m_endIndex + self_p->m_indexIncr == freeSlotIndex)
    {
        /*
         * Just adjust the the current node's end index as the end index of 
         * current node and the free slot index are consecutive "record
         * locations "
         */ 

        curNode_p->m_endIndex = freeSlotIndex;
        return NLMERR_OK;
    }
    
    /*
     * A new node is needed for free slot being added
     */

    newNode_p = (NlmFreeSlotInfo_t*)NlmCmAllocator__malloc(self_p->m_alloc_p, sizeof(NlmFreeSlotInfo_t));

    if(!newNode_p)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;

        return NLMERR_FAIL;
    }

    newNode_p->m_startIndex = freeSlotIndex;
    newNode_p->m_endIndex = freeSlotIndex;

    NlmCmDblLinkList__Insert((NlmCmDblLinkList*)curNode_p,
                             (NlmCmDblLinkList*)newNode_p);
    
    return NLMERR_OK;
    
}


/*******************************************************************************
 * Function : NlmFreeSlotMgr__Destroy 
 *
 * Parameters:
 *
 * NlmFreeSlotMgr_t * self_p  = Pointer to free slot manager object 
 *
 * Summary:
 * NlmFreeSlotMgr__Destroy frees the memory associated with the free slot
 * manager object and also freeu up the linked list maintained by free slot 
 * manager.
 ******************************************************************************/

void NlmFreeSlotMgr__Destroy(NlmFreeSlotMgr_t * self_p)
{
    NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)self_p->m_head_p,
                                NlmFreeSlotInfo__destroyNode,
                                self_p->m_alloc_p);

    NlmCmAllocator__free(self_p->m_alloc_p,self_p); 

}



NlmBool NlmFreeSlotMgr__isFreeSlot(NlmFreeSlotMgr_t * self_p,
                                NlmRecordIndex markerIndex)
{
    NlmFreeSlotInfo_t *headNode_p = self_p->m_head_p; 
    NlmFreeSlotInfo_t *firstNode_p = (NlmFreeSlotInfo_t*)headNode_p->m_next_p;
    NlmFreeSlotInfo_t *curNode_p = NULL;
    NlmBool found = NlmFalse;

    /*There are no free nodes in the list */
    if(self_p->m_head_p->m_next_p == self_p->m_head_p)
        return found;
    
    /*Process the free list in the forward direction */
        curNode_p = firstNode_p;

        while(curNode_p != headNode_p)
        {
            if((markerIndex >= curNode_p->m_startIndex) && ( markerIndex <= curNode_p->m_endIndex))
            {
                found = NlmTrue;
                break;
            }
            curNode_p = (NlmFreeSlotInfo_t*)curNode_p->m_next_p;
        }

    return found;
}

