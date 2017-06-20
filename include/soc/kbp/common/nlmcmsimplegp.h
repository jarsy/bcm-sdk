/*
 * $Id: nlmcmsimplegp.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 #ifndef __NLMSIMPLEGP_INCLUDED_H__
#define __NLMSIMPLEGP_INCLUDED_H__

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmallocator.h>
#include <nlmerrorcodes.h> 
#include "nlmcmdbllinklist.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#include <soc/kbp/common/nlmcmdbllinklist.h>
#endif

typedef struct Nlm__Marker {
    void  *one;
    void  *two;
    nlm_32        number_of_shuffles;
}Nlm__Marker;

typedef   nlm_32  (*compare_fun)(void *obj1, void *obj2) ;
typedef NlmErrNum_t (*NlmIndexChangedInGroup_t) (void* client_p,
                                                 NlmRecordIndex oldIndex,
                                                 NlmRecordIndex newIndex);

#define INVALID_INDEX (0x40000000) 

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
void *NlmGP__CreateList(
            NlmCmAllocator *alloc, 
            nlm_u32 level2_threshold,
            nlm_u16 record_size,
            compare_fun comp_fun,
            NlmReasonCode *o_reason_p 
               );

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
void NlmGP__DestroyList(
            void *gp,
            NlmCmAllocator *alloc
        );

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
NlmErrNum_t NlmGP__GetMarkerForPriority(
                    void * gp,
                    nlm_u16 newRecPriority,
                    NlmRecordIndex* o_markerIndex_p,    
                    Nlm__Marker* o_priorityMarker_pp,
                    void* client_p,
                    NlmReasonCode *o_reason_p);


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
                    );  


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
NlmErrNum_t NlmGP__InsertRecord(
                       void* gp,
                       NlmCmAllocator *alloc,
                       nlm_u16 newRecPriority,
                       Nlm__Marker* priorityMarker_p,
                       Nlm__Marker* shiftStartMarker_p,
                       NlmRecordIndex holeIndex,
                       NlmBool isForwardShift,
                       NlmIndexChangedInGroup_t indexChangedInGroup_fp,
                       void* client_p);

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
NlmErrNum_t  NlmGP__DeleteRecord(
                  void *gp,
                  NlmCmAllocator *alloc,
                  nlm_u16 recPriority,
                  nlm_u32 recIndex);
       

NlmErrNum_t  NlmGP__SelectHoleWithLeastShuffles(
        void *gl,
        nlm_u32 left,
        nlm_u32 right,
        nlm_u16 priority,
        Nlm__Marker *marker,
        nlm_u32 *o_bestHoleIndex_p,
        Nlm__Marker *shift_start_marker
        );
#endif 

