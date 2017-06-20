/*
 * $Id: nlmcmsimplegp.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 /*
 * This module manages the record priority list for a group. The record pririty
 * info is maintained in a doubly linked list and is arranged according to the
 * ascending order of priority. Records with the same priority are ordered by
 * ascending order of their indexes. While inserting a record, this module is
 * called to find a proper location for the new record being added. This module
 * triggers shuffling of records whenever it's necessary, 
 */

#include "nlmcmsimplegp.h"

typedef struct NlmGroupInfo_t NlmGroupInfo_t;

struct NlmGroupInfo_t
{
    #include "nlmcmdbllinklistdata.h"
    nlm_u16 m_priority;
    NlmRecordIndex m_recIndex;
};






/******************************************************************************
 * Function :  NlmGP__pvt_DestroyNode
 *
 * Parameters:
 * NlmCmDblLinkList* node_p = Pointer to head of the doubly linked list
 * void*            alloc_p = Pointer to generla purpose memory allocator
 *
 * Summary:
 * It frees a node in the GP list
 ******************************************************************************/


void 
NlmGP__pvt_DestroyNode(
    NlmCmDblLinkList* node_p,  
    void* alloc_p
    )
{
    NlmCmAllocator__free((NlmCmAllocator*)alloc_p, (NlmGroupInfo_t*) node_p);
}


/******************************************************************************
 * Function :  NlmGP__CreateList
 *
 * Parameters:
 * NlmCmAllocator*  alloc_p = Pointer to generla purpose memory allocator
 * nlm_u32 level2_threshold = Parameters that controls the creation of node
 *                in the first lvel of the list (unused)
 * nlm_u16      record_size = size of the records for this group
 * compare_fun      com_fun = Pointer to function that compares two nodes 
 *                (unused)
 * NlmReasonCode *o_reason_p = Location to save detailed reason code 
 
 * Summary:
 * It creates a GP list and returns pointer to the head of the list.
 ******************************************************************************/


void* 
NlmGP__CreateList(
    NlmCmAllocator* alloc_p,
    nlm_u32 level2_threshold,
    nlm_u16 record_size,
    compare_fun comp_fun,
    NlmReasonCode *o_reason_p
    )
{
    NlmGroupInfo_t* self_p = (NlmGroupInfo_t*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGroupInfo_t));

    /* To remove compiler warnings */
    (void) level2_threshold;
    (void) record_size;
    (void) comp_fun;


    if(!self_p)
    {
        if(o_reason_p) *o_reason_p = NLMRSC_LOW_MEMORY;
        return NULL;
    }

    NlmCmDblLinkList__Init((NlmCmDblLinkList*)self_p);

    return self_p;
}


/******************************************************************************
 * Function :  NlmGP__GetFirstOrLastRecord
 *
 * Parameters:
 
 * void* self_p   = Pointer to GP list
 * nlm_u32   isFirstRecord  = Flag to decide first or last record
 * NlmRecordIndex* o_markerIndex_p = Pointer to location to save the "marker node"
 * NlmReasonCode *o_reason_p       = Location to save detailed reason code
 *
 * Summary:
 * Returns the first or the last record index in the GP as specificed.
 *
 ******************************************************************************/
NlmErrNum_t 
NlmGP__GetFirstOrLastRecord(
    void* self_p,
    nlm_u32 isFirstRecord,
    NlmRecordIndex* o_markerIndex_p,    
    NlmReasonCode *o_reason_p
    )
    
{
    NlmGroupInfo_t * groupHead_p = (NlmGroupInfo_t *)self_p;
       NlmGroupInfo_t * curNode_p = NULL;
    
    /*To remove compiler warnings */
    (void) o_reason_p;

    if(!groupHead_p)
    {
        return NLMERR_FAIL;
    }

       curNode_p =  isFirstRecord ? 
                        (NlmGroupInfo_t*)groupHead_p->m_next_p : (NlmGroupInfo_t*)groupHead_p->m_back_p;

       if(!curNode_p || curNode_p == groupHead_p)
        {
            return NLMERR_FAIL;
        }

        *o_markerIndex_p = curNode_p->m_recIndex;
        return NLMERR_OK;

}



/******************************************************************************
 * Function :  NlmGP__GetMarkerForPriority
 *
 * Parameters:
 
 * void* self_p   = Pointer to GP list
 * nlm_u16 newRecPriority  = Priority of record being inserted
 * NlmRecordIndex* o_markerIndex_p = Pointer to location to save the "marker node"
 *                                   index  
 * Nlm__Marker* o_priorityMarker_p = Pointer to location to hold the address of 
 *                   the marker node.
 * void* client_p                  = Object that is interested in the index 
 *                                   changes etc.
 * NlmReasonCode *o_reason_p       = Location to save detailed reason code
 *
 * Summary:
 * While inserting a new record priority, it finds the node in the GP list 
 * whose priority is closest to the new priority being added. This "closest"
 * node is the "marker" node. This routine also finds the index of the marker
 * node 
 ******************************************************************************/

NlmErrNum_t 
NlmGP__GetMarkerForPriority(
    void* self_p,
    nlm_u16 newRecPriority,
    NlmRecordIndex* o_markerIndex_p,    
    Nlm__Marker* o_priorityMarker_p,
    void* client_p, 
    NlmReasonCode *o_reason_p
    )
{
    NlmGroupInfo_t * groupHead_p = (NlmGroupInfo_t *)self_p;
    NlmGroupInfo_t * curNode_p = NULL;
    NlmBool done = NlmFalse;
    
    /*To remove compiler warnings */
    (void) o_reason_p;
    (void) client_p;

    if(!groupHead_p)
    {
        return NLMERR_FAIL;
    }

    /*
    Get marker for priority returns a marker and index of the node
    that is closest to new record to be inserted. For instance, if 
    a record of priority 9 needs to be inserted, then the first record
    with priority 9 is treated as marker. In case there are no records 
    of priority 9, then the first node with priority 10 is treated as 
    marker. In case priority 8 is the last priority and there are no other 
    priorities, then the last node with priority 8 is returned as the marker 
    */

    curNode_p = (NlmGroupInfo_t*)groupHead_p->m_next_p;

    while(curNode_p != groupHead_p )
    {
        if(curNode_p->m_priority >= newRecPriority)
        {
            done = NlmTrue;
            break;
        }
        curNode_p = (NlmGroupInfo_t*)curNode_p->m_next_p;
    }

    if(done)
    {
        *o_markerIndex_p = curNode_p->m_recIndex;
        o_priorityMarker_p->one = curNode_p;
        return NLMERR_OK;
    }

    /* curNode_p is now pointing to the head. The node
    before the head is the last node in the list. */
    curNode_p = (NlmGroupInfo_t*)curNode_p->m_back_p;

    if(curNode_p != groupHead_p)
    {
        *o_markerIndex_p = curNode_p->m_recIndex;
        o_priorityMarker_p->one = curNode_p;
        return NLMERR_OK;
        
    }

    /*If the list is completely empty, then return 0 as the marker index*/
    *o_markerIndex_p = 0;
    o_priorityMarker_p->one = groupHead_p;
    return NLMERR_OK;
    
}

/******************************************************************************
 * Function :  NlmGP__GetMarkerForPriority
 *
 * Parameters:
 
 * void* self_p   = Pointer to GP list
 * NlmRecordIndex leftHoleIndex = index of the left free slot
 * NlmRecordIndex rightHoleIndex= index of the right free slot
 * nlm_u16 priority  = Priority of record being inserted
 * Nlm__Marker* o_priorityMarker_p = Pointer to marker. Marker holds the address
 *                   of the node in GP list whiose priority is 
 *                   nearest to the new priority being inserted.
 * NlmRecordIndex* o_bestHoleIndex_p = Index that will give least shuffles 
 *                                     when the new record is inserted.
 *                   
 * Nlm__Marker *o_shiftStartMarker_p = The node where the shuffle will start
 *
 * Summary:
 * This routine examines two free slots where the new record may be inserted.
 * Then it finds out which free slot will result in lesser number of shuffles
 * and if there will be shuffle, then at which node the shuffle will start 
 ******************************************************************************/

NlmErrNum_t 
NlmGP__SelectHoleWithLeastShuffles(
    void* self_p,
    NlmRecordIndex leftHoleIndex, 
    NlmRecordIndex rightHoleIndex,
    nlm_u16 priority,
    Nlm__Marker* priorityMarker_p,
    NlmRecordIndex* o_bestHoleIndex_p,
    Nlm__Marker* o_shiftStartMarker_p  
    )
{
    NlmGroupInfo_t * groupHead_p = (NlmGroupInfo_t *)self_p;
    NlmGroupInfo_t * curLeftNode_p = NULL;
    NlmGroupInfo_t * curRightNode_p = NULL;
    nlm_32 uniqueLeft = 0;
    nlm_u16 prevLeftPriority = 0;
    nlm_32 uniqueRight = 0;
    nlm_u16 prevRightPriority = 0;
    NlmBool done = NlmFalse;


    /*To remove compiler warnings */
    (void) priority;

    if(!groupHead_p)
    {
        return NLMERR_FAIL;
    }


    /*In case there are no elements in the list, then choose the leftHoleIndex*/
    if(!priorityMarker_p->one || priorityMarker_p->one == groupHead_p)
    {
        *o_bestHoleIndex_p = leftHoleIndex;
        o_shiftStartMarker_p->one = NULL;
        return NLMERR_OK;
    }

    curLeftNode_p = (NlmGroupInfo_t*)priorityMarker_p->one;

    curRightNode_p = (NlmGroupInfo_t*)priorityMarker_p->one;


    while(!done)
    {
        /*Move one priority to the left and one priority to the right and count 
        the number of unique priorities*/
        
        prevLeftPriority = curLeftNode_p->m_priority;

        while(curLeftNode_p != groupHead_p && 
                curLeftNode_p->m_priority == prevLeftPriority &&
                curLeftNode_p->m_recIndex > leftHoleIndex)
        {
            curLeftNode_p = (NlmGroupInfo_t*)curLeftNode_p->m_back_p;
        }

        /* Break if the end of the list is reached or the 
        holeIndex is reached */
        if(curLeftNode_p == groupHead_p ||
            curLeftNode_p->m_recIndex <= leftHoleIndex)
        {
            done = NlmTrue;
            break;
        }
        ++uniqueLeft;
        
        
        prevRightPriority = curRightNode_p->m_priority;

        while(curRightNode_p != groupHead_p &&
                curRightNode_p->m_priority == prevRightPriority &&
                curRightNode_p->m_recIndex < rightHoleIndex)
        {
            curRightNode_p = (NlmGroupInfo_t*)curRightNode_p->m_next_p;
        }

        /* Break if the end of the list is reached or the 
        holeIndex is reached */
        if(curRightNode_p == groupHead_p ||
            curRightNode_p->m_recIndex >= rightHoleIndex)
        {
            done = NlmTrue;
            break;
        }
        ++uniqueRight;
        
    }

    if(uniqueLeft <= uniqueRight)
    {
        *o_bestHoleIndex_p = leftHoleIndex;
        o_shiftStartMarker_p->one = curLeftNode_p;
    }
    else
    {
        *o_bestHoleIndex_p = rightHoleIndex;
        o_shiftStartMarker_p->one = curRightNode_p;
    }

    return NLMERR_OK;
}

/******************************************************************************
 * Function :  NlmGP__InsertRecord
 *
 * Parameters:
 * void *self_p = Pointer to GP list 
 * NlmCmAllocator* alloc_p = Pointer to General purpose memory allocator
 * nlm_u16 newRecPriority = Priority of the new record being inserted
 * Nlm__Marker* priorityMarker_p = pointer to marker (used when shiftStartMarker_p
 *                 pointer is NULL)
 * Nlm__Marker* shiftStartMarker_p = Pointer to marker that holds the pointer to
 *                                   node where the shuffle starts
 * NlmRecordIndex holeIndex = Index of the free slot  which will be occupied now
 * NlmIndexChangedInGroup_t indexChangedInGroup_fp = Pointert to function to be 
 *           called when a record is moved to other location.
 * void* client_p  = Pointer to object interested in index changes of records 
 *                   etc.
 *
 * Summary:
 * This routine inserts a record with a given priority into the GP list.
 * This routine also causes the reordering of the list to maintain the priority 
 * order. This routine calls the the callback indexChangedInGroup_fp which
 * does the real work related to change of index of a record. 
 ******************************************************************************/

NlmErrNum_t 
NlmGP__InsertRecord(
    void* self_p,
    NlmCmAllocator* alloc_p,
    nlm_u16 newRecPriority,
    Nlm__Marker* priorityMarker_p,
    Nlm__Marker* shiftStartMarker_p,
    NlmRecordIndex holeIndex,
    NlmBool isForwardShift,
    NlmIndexChangedInGroup_t indexChangedInGroup_fp,
    void* client_p
    )
{
    NlmGroupInfo_t * groupHead_p = (NlmGroupInfo_t *)self_p;
    NlmGroupInfo_t * curNode_p = (NlmGroupInfo_t*)shiftStartMarker_p->one;
    NlmGroupInfo_t * holeNode_p = NULL;
    NlmBool done = NlmFalse;
    nlm_u16 prevPriority = 0;

    if(!groupHead_p)
    {
        return NLMERR_FAIL;
    }

    /* 
    curNode_p is initialized to the shiftStartMarker. Sometimes, the function
    NlmGP__SelectHoleWithLeastShuffles is not called (example only one free slot exists. 
    In this case the shift start marker is NULL and the curNode_p should be correctly
    positioned to the location where the shift starts. This is being done in the code below 
    */
    if(curNode_p == NULL)
    {
        /*Initialize curNode_p to the priority marker */
        curNode_p = (NlmGroupInfo_t*)priorityMarker_p->one;

        while(curNode_p != groupHead_p )
        {
            if( (isForwardShift && curNode_p->m_recIndex <= holeIndex) ||
                (!isForwardShift && curNode_p->m_recIndex >= holeIndex))
            {
                break;
            }

            /*While positioning curNode_p, move in the backward if forward shift*/
            if(isForwardShift)
            {
                curNode_p = (NlmGroupInfo_t*)curNode_p->m_back_p;
            }
            else
            {
                curNode_p = (NlmGroupInfo_t*)curNode_p->m_next_p;
            }
        }

    }
    
    /* Suppose we have a linked list, A-C and we want to insert B. curNode_p
    points to C. Since a new node is inserted after a particular node, 
    move curNode_p one position back to A. Then insert B. B will be inserted 
    after A. This is required only if the shift is in backward direction*/
    if(!isForwardShift)
    {
        curNode_p = (NlmGroupInfo_t*)curNode_p->m_back_p;   
    }
    

    holeNode_p = (NlmGroupInfo_t*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGroupInfo_t));

    if(!holeNode_p)
    {
        return NLMERR_FAIL;
    }

    /*Insert the hole node after the current node*/
    NlmCmDblLinkList__Insert((NlmCmDblLinkList*)curNode_p, (NlmCmDblLinkList*)holeNode_p);

    holeNode_p->m_recIndex = holeIndex;

    /*If forward shift, then proceed to the next node */
    if(isForwardShift)
        curNode_p = (NlmGroupInfo_t*)holeNode_p->m_next_p;

    done = NlmFalse;
    while(!done)
    {
        if(curNode_p == groupHead_p ||
            (isForwardShift && curNode_p->m_priority >= newRecPriority )||
            (!isForwardShift && curNode_p->m_priority <= newRecPriority ))
        {
            done = NlmTrue;
            break;
        }

        prevPriority = curNode_p->m_priority;

        while(curNode_p != groupHead_p && 
                curNode_p->m_priority == prevPriority)
        {
            if(isForwardShift)
                curNode_p = (NlmGroupInfo_t*)curNode_p->m_next_p;
            else
                curNode_p = (NlmGroupInfo_t*)curNode_p->m_back_p;
        }

        if(isForwardShift)
            curNode_p = (NlmGroupInfo_t*)curNode_p->m_back_p;
        else
            curNode_p = (NlmGroupInfo_t*)curNode_p->m_next_p;
        
        holeNode_p->m_priority = curNode_p->m_priority;

        /*Shuffle the records */
        indexChangedInGroup_fp(client_p,
                                curNode_p->m_recIndex,
                                holeNode_p->m_recIndex);
        

        holeNode_p = curNode_p;

        if(isForwardShift)
            curNode_p = (NlmGroupInfo_t*)curNode_p->m_next_p;
        else
            curNode_p = (NlmGroupInfo_t*)curNode_p->m_back_p;
        
    }

    /*Insert the record requested by application */
    holeNode_p->m_priority = newRecPriority;

    indexChangedInGroup_fp(client_p,
                            INVALID_INDEX,
                            holeNode_p->m_recIndex);
    
    return NLMERR_OK;
    
}

/******************************************************************************
 * Function :  NlmGP__DeleteRecord
 *
 * Parameters:
 * void *self_p = Pointer to the GP list 
 * NlmCmAllocator* alloc_p = Pointer to General purpose memory allocator
 * nlm_u16 recPriority = Priority of the new record being being deleted
 * NlmRecordIndex recIndex = Index of the record being deleted
 *
 * Summary:
 * This routine deletes a node from the GP list.
 ******************************************************************************/

NlmErrNum_t 
NlmGP__DeleteRecord(
    void* self_p,
    NlmCmAllocator* alloc_p,
    nlm_u16 recPriority,
    NlmRecordIndex recIndex
    )
{
    NlmGroupInfo_t * groupHead_p = (NlmGroupInfo_t *)self_p;
    NlmGroupInfo_t * curNode_p = (NlmGroupInfo_t*)groupHead_p->m_next_p;
    NlmBool found = NlmFalse;

    if(!groupHead_p)
    {
        return NLMERR_FAIL;
    }

    
    while(curNode_p != groupHead_p)
    {
        if(curNode_p->m_recIndex == recIndex &&
            curNode_p->m_priority == recPriority)
        {
            found = NlmTrue;

            NlmCmDblLinkList__Remove((NlmCmDblLinkList*)curNode_p,
                                        NlmGP__pvt_DestroyNode,
                                        alloc_p); 
            break;
        }
        curNode_p = (NlmGroupInfo_t*)curNode_p->m_next_p;
    }


    if(!found)
        return NLMERR_FAIL;
    else
        return NLMERR_OK;
}

/******************************************************************************
 * Function :  NlmGP__DestroyList
 *
 * Parameters:
 * void *self_p = Pointer to the GP list 
 * NlmCmAllocator* alloc_p = Pointer to General purpose memory allocator
 *
 * Summary:
 * This routine deletes all the nodes from the GP List.
 ******************************************************************************/
void 
NlmGP__DestroyList(
    void* self_p,
    NlmCmAllocator* alloc_p 
    )
{
    if(self_p)
    {
        NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)self_p,
                                NlmGP__pvt_DestroyNode,
                                alloc_p);
    }

    
}

