/*
 * $Id: nlmblkmemmgr.c,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

 
/*
The file contains the algorithms for managing the memory of 
the chip. A single device consists of 16 super blocks and each having 4 blocks
Addition, Deletion and shuffling of records based on priority is handled by this
module. 

The main data structures used are  
1. NlmBlkMemMgr_t - The information of each record in a block is maintained
2. FreeSlotManager - The free slots in each block are maintained
3. Two level hash table - Stores the pointer to the first member in a group of records
4. SimpleGP - Collection of records that belong to the group

General comments
Index refers to the absolute location of the record. Offset refers to the 
relative position of the record within the blk.  

left indicates a position with a lower TCAM address
right indicates a position with a higher TCAM address
Example: Consider block 0 and block 10,
left block is block 0 and right block is block 10

forward direction is from lower TCAM address to high TCAM address
backward direction is from high TCAM address to low TCAM address

Shift direction during shuffles is based on the direction of the freeSlot.  
If the freeSlot is moved from lower TCAM address to higher TCAM address
then it is a forward shift

Abbreviations used 
SB - SuperBlock
*/


#include "nlmblkmemmgrint.h"
#include "nlmarch.h"

#ifdef NLM_12K_11K
#include <arch/nlmarch.h>
#endif

#include "nlmcmsimplegp.h"

#define TOTAL_NUM_SB                    NLMDEV_NUM_SUPER_BLOCKS 

#ifndef NLM_12K_11K
#define TOTAL_NUM_BLKS                  NLMDEV_NUM_ARRAY_BLOCKS
#else
#define TOTAL_NUM_BLKS                  NLM11KDEV_NUM_ARRAY_BLOCKS
#endif

#define MIN_BLK_WIDTH                   NLMDEV_AB_WIDTH_IN_BITS

#define MAX_NUM_ENTRIES_PER_BLK         NLMDEV_AB_DEPTH

#define MAX_BLK_WIDTH                   NLMDEV_MAX_AB_WIDTH_IN_BITS


#define NUM_BLKS_IN_SB                  (TOTAL_NUM_BLKS / TOTAL_NUM_SB)

#define SIZE_OF_BLK                     (MAX_NUM_ENTRIES_PER_BLK * MIN_BLK_WIDTH )

#define MAX_REC_OFFSET                  (MAX_NUM_ENTRIES_PER_BLK - 1)

#define NLM_VALID_GTM_BLK                1

#define NLM_INVALID_GTM_BLK              0


#define INVALID_OFFSET                  (-1)


#define GROUPID_HASH_TBL_SIZE                   (64 * 1024) 

#define GROUPID_HASH_TBL_LEVEL_2_NR_OF_ELEMS    (16)




#define TOTAL_NUM_RECS_PER_BLK(blkInfo_p,curBlk) (SIZE_OF_BLK / blkInfo_p[curBlk].m_blkWidth)

#define OFFSET_CHANGE(blkInfo_p,curBlk)         (blkInfo_p[curBlk].m_blkWidth / MIN_BLK_WIDTH)

#define ConvertToIndex(curBlk,curRecOffset)     (curBlk * MAX_NUM_ENTRIES_PER_BLK + curRecOffset)

#define INDEX_TO_BLK_NUM(index)                 (index / MAX_NUM_ENTRIES_PER_BLK)

#define INDEX_TO_OFFSET(index)                  (index % MAX_NUM_ENTRIES_PER_BLK)



#define ISFULL(blkInfo_p,curBlk) (blkInfo_p[curBlk].m_numUsedRecs >= TOTAL_NUM_RECS_PER_BLK(blkInfo_p,curBlk))


#include "nlmcmexterncstart.h"


static void NlmBlkMemMgr__pvt_InitFnArgs(
            NlmBlkMemMgrFnArgs_t *args_p,
            NlmGenericTbl       *genericTbl_p,
            nlm_u16             groupId,
            nlm_u16             newRecPriority, 
            NlmRecordIndex      *o_recIndex_p, 
            nlm_16              newRecWidth,
            NlmReasonCode       *o_reason_p,
            NlmBlkMemMgr_t  *blkMemMgr_p);

static NlmErrNum_t NlmBlkMemMgr__pvt_InitTblsInBlk(
                NlmBlkInfo_t* blkInfo_p,
                nlm_32 curBlk,
                NlmCmAllocator* alloc_p,
                NlmReasonCode   *o_reason_p);

static NlmTblsInBlk_t* NlmBlkMemMgr__pvt_InsertTblsInBlk(
                NlmCmAllocator* alloc_p,
                NlmTblsInBlk_t* tblListHead_p,
                nlm_u8 tblId);

static void NlmBlkMemMgr__pvt_DestroyNodeTblsInBlk(
                NlmCmDblLinkList* node_p, 
                void* alloc_p);

static void NlmBlkMemMgr__pvt_RemoveTblsInBlk(
                NlmCmAllocator* alloc_p,
                NlmTblsInBlk_t* tblInBlk_p);

static void NlmBlkMemMgr__pvt_DestroyTblsInBlk(
                NlmCmAllocator* alloc_p, 
                NlmTblsInBlk_t* head_p);

static NlmBool NlmBlkMemMgr__pvt_FindTblInBlk(
                NlmBlkInfo_t* blkInfo_p,
                nlm_32 curBlk,
                nlm_u8 tblId,
                NlmTblsInBlk_t** tblInBlk_pp);

static NlmBool NlmBlkMemMgr__pvt_FindTblInSB(
                NlmBlkInfo_t* blkInfo_p,
                nlm_32 curSB,
                nlm_u8 tblId);

static NlmBool NlmBlkMemMgr__pvt_CanTblEnterBlkInNewSB(
                NlmBlkInfo_t* blkInfo_p,
                nlm_32 curSB,
                NlmGenericTbl   *genericTbl_p);


static NlmErrNum_t NlmBlkMemMgr__pvt_MarkFreeSlotAsOccupied(
                NlmBlkMemMgrFnArgs_t* args_p,
                NlmRecordIndex freeRecIndex);


static NlmErrNum_t NlmBlkMemMgr__pvt_ChooseOneFreeSlot(
                NlmBlkMemMgrFnArgs_t *args_p,
                NlmRecordIndex leftFreeSlot,
                NlmRecordIndex rightFreeSlot,
                void* groupIdHead_p,
                Nlm__Marker priorityMarker,
                NlmRecordIndex *o_bestFreeSlotIndex_p,
                NlmBool *o_isForwardShift_p,
                Nlm__Marker* o_shiftStartMarker_p);

static NlmErrNum_t NlmBlkMemMgr__pvt_FindBestFreeSlotInOldBlks(
                NlmBlkMemMgrFnArgs_t *args_p,
                void *groupIdHead_p,
                NlmRecordIndex markerIndex,
                Nlm__Marker priorityMarker,
                NlmRecordIndex *o_bestFreeSlotIndex_p,
                NlmBool *o_isForwardShift_p,
                Nlm__Marker* o_shiftStartMarker_p);

static NlmErrNum_t NlmBlkMemMgr__pvt_ReInitFreeSlotMgr(
                NlmBlkMemMgrFnArgs_t* args_p,
                nlm_32 curBlk
                );

static NlmErrNum_t NlmBlkMemMgr__pvt_FindBestFreeSlotInNewBlks(
                NlmBlkMemMgrFnArgs_t* args_p,
                void * groupIdHead_p,
                NlmRecordIndex markerIndex,
                Nlm__Marker priorityMarker,
                NlmBool lookInOldSB,
                NlmRecordIndex *o_bestFreeSlotIndex_p,
                NlmBool *o_isForwardShift_p,
                Nlm__Marker* o_shiftStartMarker_p);

static NlmErrNum_t NlmBlkMemMgr__pvt_AddRecord(
                NlmBlkMemMgrFnArgs_t* args_p
                );


#include "nlmcmexterncend.h"


/****************************************************
 * Function : NlmBlkMemMgr__pvt_IndexChangedInGroup
 * 
 * Parameters:
 *  void* client_p,
 *  NlmRecordIndex oldIndex,
 *  NlmRecordIndex newIndex
 *
 * Summary:
 * This is internal function called when shuffle happens
 * within a group. This function updates BMM internal
 * data structure first and then calls upper layer function
 * that performs shuffle in the device. If oldIndex is
 * INVALID_INDEX, it means no index change is happening
 * of any existing record and record should be added to
 * newIndex. If oldIndex is not INVALID_INDEX, it means
 * that an existing record needs to shifted to newIndex.
 ******************************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_IndexChangedInGroup(
    void* client_p,
    NlmRecordIndex oldIndex,
    NlmRecordIndex newIndex
    )
{
    NlmBlkMemMgrFnArgs_t* args_p = (NlmBlkMemMgrFnArgs_t*)client_p;
    NlmBlkInfo_t *blkInfo_p = args_p->blkMemMgr_p->m_blkInfo_p;
    nlm_32 oldBlk = INDEX_TO_BLK_NUM(oldIndex);
    nlm_32 oldOffset = INDEX_TO_OFFSET(oldIndex);
    nlm_32 newBlk = INDEX_TO_BLK_NUM(newIndex);
    nlm_32 newOffset = INDEX_TO_OFFSET(newIndex);
    NlmErrNum_t errNum = NLMERR_OK;

    
    if(oldIndex != INVALID_INDEX)
    {
        /*if oldIndex is not INVALID_INDEX, then copy the  record
        from the old location to the new location. This corresponds to a shuffle */
        NlmCm__memcpy( &blkInfo_p[newBlk].m_recInfo_p[newOffset],
                        &blkInfo_p[oldBlk].m_recInfo_p[oldOffset],
                        sizeof(NlmRecInfo_t));

        if(oldIndex < newIndex)
            args_p->isShuffleDown = NlmTrue;
    }
    else
    {
        /*update the information of the record currently being added */
        blkInfo_p[newBlk].m_recInfo_p[newOffset].m_priority = args_p->newRecPriority;
        blkInfo_p[newBlk].m_recInfo_p[newOffset].m_groupId = args_p->groupId;
        blkInfo_p[newBlk].m_recInfo_p[newOffset].m_tblId = 
                                            args_p->genericTbl_p->m_tblId;

        *args_p->o_recIndex_p = newIndex;
    }

    if(args_p->blkMemMgr_p->m_indexChangedCB && oldIndex != INVALID_INDEX)
    {
        /*Inform the callback of the upper layer about index change*/
        errNum = args_p->blkMemMgr_p->m_indexChangedCB(
                                args_p->blkMemMgr_p->m_ctx_p,
                                args_p->genericTbl_p,
                                oldIndex,
                                newIndex,
                                args_p->newRecWidth,
                                args_p->o_reason_p);
    }
 

    return errNum;
}

/************************************************
 * Function : NlmBlkMemMgr__pvt_InitFnArgs
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t *args_p, 
 *  NlmGenericTbl       *genericTbl_p,
 *  nlm_u16             groupId,
 *  nlm_u16             newRecPriority, 
 *  NlmRecordIndex      *o_recIndex_p, 
 *  nlm_16              newRecWidth,
 *  NlmReasonCode       *o_reason_p,
 *  NlmBlkMemMgr_t  *blkMemMgr_p
 *
 * Summary:
 * This function initializes the structure that holds
 * the parameters needed for an add record
 ***********************************************/
void 
NlmBlkMemMgr__pvt_InitFnArgs(
    NlmBlkMemMgrFnArgs_t *args_p, 
    NlmGenericTbl       *genericTbl_p,
    nlm_u16             groupId,
    nlm_u16             newRecPriority, 
    NlmRecordIndex      *o_recIndex_p, 
    nlm_16              newRecWidth,
    NlmReasonCode       *o_reason_p,
    NlmBlkMemMgr_t  *blkMemMgr_p
    )
{
    args_p->genericTbl_p = genericTbl_p;
    args_p->groupId = groupId;
    args_p->newRecPriority = newRecPriority;
    args_p->o_recIndex_p = o_recIndex_p;
    args_p->newRecWidth = newRecWidth;
    args_p->o_reason_p = o_reason_p;
    args_p->blkMemMgr_p = blkMemMgr_p;
    args_p->isShuffleDown = NlmFalse;
}

/************************************************
 * Function : NlmBlkMemMgr__pvt_InitTblsInBlk
 * 
 * Parameters:
 *  NlmBlkInfo_t* blkInfo_p,
 *  nlm_32 curBlk,
 *  NlmCmAllocator* alloc_p,
 *  NlmReasonCode   *o_reason_p
 *
 * Summary:
 * This function initializes the linked list TblsInBlk
 * with the head node that does not contain any data 
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_InitTblsInBlk(
    NlmBlkInfo_t* blkInfo_p,
    nlm_32 curBlk,
    NlmCmAllocator* alloc_p,
    NlmReasonCode   *o_reason_p
    )
{
    blkInfo_p[curBlk].m_tblList_p = (NlmTblsInBlk_t*)NlmCmAllocator__calloc(alloc_p, 1,
                                                            sizeof(NlmTblsInBlk_t));
    if(!blkInfo_p[curBlk].m_tblList_p)
    {
        if(o_reason_p) *o_reason_p = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }

    NlmCmDblLinkList__Init(
        (NlmCmDblLinkList*)blkInfo_p[curBlk].m_tblList_p);

    return NLMERR_OK;
}

/************************************************
 * Function : NlmBlkMemMgr__pvt_InsertTblsInBlk
 * 
 * Parameters:
 *  NlmCmAllocator* alloc_p,
 *  NlmTblsInBlk_t* head_p,
 *  nlm_8* tblId
 *
 * Summary:
 * This function inserts a node into the linked list
 * TblsInBlk. A doubly linked list per block is
 * maintained; tables added into this block are
 * added into this list.
 ***********************************************/
NlmTblsInBlk_t* 
NlmBlkMemMgr__pvt_InsertTblsInBlk(
    NlmCmAllocator* alloc_p,
    NlmTblsInBlk_t* head_p,
    nlm_u8 tblId
    )
{
    NlmTblsInBlk_t* node_p = (NlmTblsInBlk_t*)NlmCmAllocator__malloc(alloc_p, sizeof(NlmTblsInBlk_t));

    if(!node_p) return NULL;

    node_p->m_tblId = tblId;

    node_p->m_numRecs = 0;

    NlmCmDblLinkList__Insert((NlmCmDblLinkList*)head_p, (NlmCmDblLinkList*)node_p);

    return node_p;
}

/************************************************
 * Function : NlmBlkMemMgr__pvt_DestroyNodeTblsInBlk
 * 
 * Parameters:
 *  NlmCmDblLinkList* node_p,  
 *  void* alloc_p
 *
 * Summary:
 * This function frees a node of the  linked list TblsInBlk 
 ***********************************************/
void 
NlmBlkMemMgr__pvt_DestroyNodeTblsInBlk(
    NlmCmDblLinkList* node_p,  
    void* alloc_p
    )
{
    NlmCmAllocator__free((NlmCmAllocator*)alloc_p, (NlmTblsInBlk_t*) node_p);
}

/************************************************
 * Function : NlmBlkMemMgr__pvt_RemoveTblsInBlk
 * 
 * Parameters:
 *  NlmCmAllocator* alloc_p,
 *  NlmTblsInBlk_t* tblInBlk_p
 *
 * Summary:
 * This function removes a node from the  linked list TblsInBlk 
 ***********************************************/
void 
NlmBlkMemMgr__pvt_RemoveTblsInBlk(
    NlmCmAllocator* alloc_p,
    NlmTblsInBlk_t* tblInBlk_p
    )
{
    NlmCmDblLinkList__Remove((NlmCmDblLinkList*)tblInBlk_p,
                            NlmBlkMemMgr__pvt_DestroyNodeTblsInBlk,
                            alloc_p); 
}


/************************************************
 * Function : NlmBlkMemMgr__pvt_DestroyTblsInBlk
 * 
 * Parameters:
 *  NlmCmAllocator* alloc_p, 
 *  NlmTblsInBlk_t* head_p
 *
 * Summary:
 *
 * This function removes every node in the link list
 * TblsInBlk and free all the nodes 
 ***********************************************/
void 
NlmBlkMemMgr__pvt_DestroyTblsInBlk(
    NlmCmAllocator* alloc_p, 
    NlmTblsInBlk_t* head_p
    )
{
    NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)head_p,
                                NlmBlkMemMgr__pvt_DestroyNodeTblsInBlk,
                                alloc_p);

}


/************************************************
 * Function : NlmBlkMemMgr__pvt_FindTblInBlk
 * 
 * Parameters:
 *  NlmBlkInfo_t* blkInfo_p,
 *  nlm_32 curBlk,
 *  nlm_u8 tblId,
 *  NlmTblsInBlk_t** tblInBlk_pp 
 *
 * Summary:
 *
 * Finds if an record belonging to the tbl already exists in the blk 
 * Each blk has a linked list of tbls whose records are in the blk. 
 * In case a record belonging to the tbl exists in the blk, the function
 * returns true and pointer to the tbl in the linked list
 ***********************************************/
NlmBool 
NlmBlkMemMgr__pvt_FindTblInBlk(
    NlmBlkInfo_t* blkInfo_p,
    nlm_32 curBlk,
    nlm_u8 tblId,
    NlmTblsInBlk_t** tblInBlk_pp /* out argument, can be null */
    )
{
    NlmTblsInBlk_t *tblInBlkHead_p = blkInfo_p[curBlk].m_tblList_p;
    NlmTblsInBlk_t *node_p = (NlmTblsInBlk_t*)tblInBlkHead_p->m_next_p;
    NlmBool found = NlmFalse;
    
    while(node_p != tblInBlkHead_p && !found)
    {
        if(node_p->m_tblId == tblId)
        {
            found = NlmTrue;
            break;
        }
        node_p = (NlmTblsInBlk_t*)node_p->m_next_p;
    }

    if(found && tblInBlk_pp != NULL)
        *tblInBlk_pp = node_p;

    return found;
}



/************************************************
 * Function : NlmBlkMemMgr__pvt_FindTblInSB
 * 
 * Parameters:
 *  NlmBlkInfo_t* blkInfo_p,
 *  nlm_32 curSB,
 *  nlm_u8 tblId
 *
 * Summary:
 *
 * Returns true if a record of the table exists in the super blk 
 ***********************************************/
NlmBool 
NlmBlkMemMgr__pvt_FindTblInSB(
    NlmBlkInfo_t* blkInfo_p,
    nlm_32 curSB,
    nlm_u8 tblId
    )
{
    NlmBool found = NlmFalse;
    nlm_32 curBlk = curSB * blkInfo_p->m_BlksInSB;

    if(blkInfo_p[curBlk].m_validBlk)
    {
        while(!found && curBlk < (curSB+1)* blkInfo_p->m_BlksInSB)
        {
            if(NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p,
                            curBlk,
                            tblId,
                            NULL))
            {
                found = NlmTrue;
                break;
            }
                                
            ++curBlk;
        }
    }
    return found;
}


/************************************************
 * Function : NlmBlkMemMgr__pvt_CanTblEnterBlkInNewSB
 * 
 * Parameters:
 *  NlmBlkInfo_t*       blkInfo_p,
 *  nlm_32          curSB,
 *  NlmGenericTbl   *genericTbl_p
 *
 * Summary:
 *
 * Returns true if records of the table can enter the new Super blk
 * Check dependency of all tables present in block, if a table
 * is found in block which is searched in parallel with genericTbl_p
 * genericTbl_p can not enter in this superblock.
 ***********************************************/
NlmBool 
NlmBlkMemMgr__pvt_CanTblEnterBlkInNewSB(
    NlmBlkInfo_t*       blkInfo_p,
    nlm_32          curSB,
    NlmGenericTbl   *genericTbl_p
    )
{
    NlmBool tblCanEnterBlkInNewSB = NlmTrue;
    nlm_32 curBlk = curSB * blkInfo_p->m_BlksInSB;
    NlmTblsInBlk_t *tblInBlkHead_p = NULL;
    NlmTblsInBlk_t *tblNode_p = NULL;
    NlmPsTblList *psTblListHead_p = NULL;   
    NlmPsTblList *psTblListNode_p = NULL;
    
    if(!blkInfo_p[curBlk].m_validBlk)
        return NlmFalse;
    
    /*Iterate through each of the blks in the super blk */
    while(curBlk < (curSB+1)* blkInfo_p->m_BlksInSB)
    {
        tblInBlkHead_p = blkInfo_p[curBlk].m_tblList_p;
        tblNode_p = (NlmTblsInBlk_t*)tblInBlkHead_p->m_next_p;
        
        /*Iterate through each of the tables whose records are in the blk */
        while(tblNode_p != tblInBlkHead_p)
        {
            psTblListHead_p = genericTbl_p->m_psTblList_p;

            if(psTblListHead_p == NULL)
                break;

            psTblListNode_p = (NlmPsTblList*)psTblListHead_p->m_next_p;

            /*Iterate through the parallel search dependency list of the table
            which is requesting for the new super blk */

            while(psTblListNode_p != psTblListHead_p)
            {
                if(psTblListNode_p->m_tblId == tblNode_p->m_tblId)
                {
                    /*If a table which is searched in parallel is already 
                    having records in the blk, then we cannot assign the 
                    super blk*/
                    tblCanEnterBlkInNewSB = NlmFalse;
                    break;
                }
                psTblListNode_p = (NlmPsTblList*)psTblListNode_p->m_next_p;
            }
            if(!tblCanEnterBlkInNewSB) break;

            tblNode_p = (NlmTblsInBlk_t*)tblNode_p->m_next_p;
        }

        if(!tblCanEnterBlkInNewSB) break;
                            
        ++curBlk;
    }

    return tblCanEnterBlkInNewSB;
}


/************************************************
 * Function :NlmBlkMemMgr__pvt_DestroyElemInHashTbl
 * 
 * Parameters:
 *  NlmCmAllocator* alloc_p,    
 *  void* element_p
 *
 * Summary:
 * Destroys elements stored in the hash table. 
 *
 ***********************************************/
void 
NlmBlkMemMgr__pvt_DestroyElemInHashTbl(
    NlmCmAllocator* alloc_p,    
    void* element_p
    )
{
    NlmGP__DestroyList(element_p, alloc_p );
}


/************************************************
 * Function : NlmBlkMemMgr__pvt_ChooseOneFreeSlot
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t *args_p,
 *  NlmRecordIndex leftFreeSlot,
 *  NlmRecordIndex rightFreeSlot,
 *  void* groupIdHead_p,
 *  Nlm__Marker priorityMarker,
 *  NlmRecordIndex *o_bestFreeSlotIndex_p,
 *  NlmBool *o_isForwardShift_p,
 *  Nlm__Marker* o_shiftStartMarker_p
 *
 * Summary:
 * Given two free slots, the free slot with least number of shuffles is 
 * selected and the shift direction is indicated
 *
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_ChooseOneFreeSlot(
    NlmBlkMemMgrFnArgs_t *args_p,
    NlmRecordIndex leftFreeSlot,
    NlmRecordIndex rightFreeSlot,
    void* groupIdHead_p,
    Nlm__Marker priorityMarker,
    NlmRecordIndex *o_bestFreeSlotIndex_p,
    NlmBool *o_isForwardShift_p,
    Nlm__Marker* o_shiftStartMarker_p
    )
{
    NlmErrNum_t errNum = NLMERR_OK;

    if(leftFreeSlot != INVALID_INDEX && rightFreeSlot != INVALID_INDEX &&
        leftFreeSlot != rightFreeSlot)
    {
        errNum = NlmGP__SelectHoleWithLeastShuffles(
                                        groupIdHead_p,
                                        leftFreeSlot, 
                                        rightFreeSlot, 
                                        args_p->newRecPriority,
                                        &priorityMarker, 
                                        o_bestFreeSlotIndex_p,
                                        o_shiftStartMarker_p);

        if(errNum != NLMERR_OK)
            return errNum;

        if(*o_bestFreeSlotIndex_p == leftFreeSlot)
            *o_isForwardShift_p = NlmTrue;
        else
            *o_isForwardShift_p = NlmFalse;

    }
    else if(leftFreeSlot != INVALID_INDEX)
    {
        *o_bestFreeSlotIndex_p = leftFreeSlot;
        *o_isForwardShift_p = NlmTrue;
    }
    else if(rightFreeSlot != INVALID_INDEX)
    {
        *o_bestFreeSlotIndex_p = rightFreeSlot;
        *o_isForwardShift_p = NlmFalse;
    }
    else
    {
        *o_bestFreeSlotIndex_p = INVALID_INDEX;
    }

    return errNum;
}


/************************************************
 * Function : NlmBlkMemMgr__pvt_FindBestFreeSlotInOldBlks
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t *args_p,
 *  void *groupIdHead_p,
 *  NlmRecordIndex markerIndex,
 *  Nlm__Marker priorityMarker,
 *  NlmRecordIndex *o_bestFreeSlotIndex_p,
 *  NlmBool *o_isForwardShift_p,
 *  Nlm__Marker* o_shiftStartMarker_p 
 *
 * Summary:
 * Finds the best free slot in the set of blocks that already
 * have atleast one entry of the table
 *
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_FindBestFreeSlotInOldBlks(
    NlmBlkMemMgrFnArgs_t *args_p,
    void *groupIdHead_p,
    NlmRecordIndex markerIndex,
    Nlm__Marker priorityMarker,
    NlmRecordIndex *o_bestFreeSlotIndex_p,
    NlmBool *o_isForwardShift_p,
    Nlm__Marker* o_shiftStartMarker_p )
{
    NlmBlkMemMgr_t* blkMemMgr_p = args_p->blkMemMgr_p;
    nlm_32 curBackBlk = INDEX_TO_BLK_NUM(markerIndex);
    nlm_32 curForwardBlk = curBackBlk;
    NlmBool isTblInBlk = NlmFalse;
    NlmBool foundFreeSlot = NlmFalse;
    NlmRecordIndex leftFreeSlot = INVALID_INDEX;
    NlmRecordIndex rightFreeSlot = INVALID_INDEX;
    NlmErrNum_t errNum = NLMERR_OK;
     
    
    while(curBackBlk >= 0)
    {
        if(blkMemMgr_p->m_blkInfo_p[curBackBlk].m_validBlk)
        {
            isTblInBlk = NlmBlkMemMgr__pvt_FindTblInBlk(blkMemMgr_p->m_blkInfo_p,
                                        curBackBlk,
                                        args_p->genericTbl_p->m_tblId,
                                        NULL);

            if(!ISFULL(blkMemMgr_p->m_blkInfo_p,curBackBlk) && isTblInBlk)
            {
                foundFreeSlot = NlmFreeSlotMgr__FindNearestFreeSlot( 
                                    blkMemMgr_p->m_blkInfo_p[curBackBlk].m_freeSlotMgr_p,
                                    markerIndex,
                                    NlmFalse,
                                    &leftFreeSlot);

                if(foundFreeSlot)
                break;
            }
        }
        --curBackBlk;
    }

    while(curForwardBlk < blkMemMgr_p->m_NumOfABs * blkMemMgr_p->m_nrOfDevices)
    {
        if(blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_validBlk)
        {
            isTblInBlk = NlmBlkMemMgr__pvt_FindTblInBlk(blkMemMgr_p->m_blkInfo_p,
                                    curForwardBlk,
                                    args_p->genericTbl_p->m_tblId,
                                    NULL);

            if(!ISFULL(blkMemMgr_p->m_blkInfo_p, curForwardBlk) && isTblInBlk)
            {
                foundFreeSlot = NlmFreeSlotMgr__FindNearestFreeSlot( 
                                    blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_freeSlotMgr_p,
                                    markerIndex,
                                    NlmTrue,
                                    &rightFreeSlot);
                if(foundFreeSlot)
                break;
            }
        }
        ++curForwardBlk;

    }

    errNum = NlmBlkMemMgr__pvt_ChooseOneFreeSlot(args_p,
                                  leftFreeSlot,
                                  rightFreeSlot,
                                  groupIdHead_p,
                                  priorityMarker,
                                  o_bestFreeSlotIndex_p,
                                  o_isForwardShift_p,
                                  o_shiftStartMarker_p);
    
    return errNum;

}


/************************************************
 * Function: NlmBlkMemMgr__pvt_ReInitFreeSlotMgr
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t* args_p,
 *  nlm_32 curBlk
 * 
 * Summary:
 * Reinitializes the free slot manager. When a block that was completely empty
 * now begins to store the first record, the block width has to set and 
 * correspondingly the free slot widths within the free slot manager change
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_ReInitFreeSlotMgr(
    NlmBlkMemMgrFnArgs_t* args_p,
    nlm_32 curBlk
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = args_p->blkMemMgr_p;
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;

    NlmRecordIndex indexIncr = (args_p->newRecWidth / MIN_BLK_WIDTH);
    NlmRecordIndex endIndex = ((curBlk+1) * MAX_NUM_ENTRIES_PER_BLK) - indexIncr;

    /*Destroy the previous free slot manager*/
    NlmFreeSlotMgr__Destroy(blkInfo_p[curBlk].m_freeSlotMgr_p);

    /*Create the new free slot manager */
    blkInfo_p[curBlk].m_freeSlotMgr_p = NlmFreeSlotMgr__Init(blkMemMgr_p->m_alloc_p,
                                                        curBlk * MAX_NUM_ENTRIES_PER_BLK,
                                                        endIndex,
                                                        indexIncr,
                                                        args_p->o_reason_p);

    blkInfo_p[curBlk].m_blkWidth = args_p->newRecWidth;

    if(!blkInfo_p[curBlk].m_freeSlotMgr_p)
        return NLMERR_FAIL;

    return NLMERR_OK;

}


/************************************************
 * Function : NlmBlkMemMgr__pvt_FindBestFreeSlotInNewBlks
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t* args_p,
 *  void * groupIdHead_p,
 *  NlmRecordIndex markerIndex,
 *  Nlm__Marker priorityMarker,
 *  NlmBool lookInOldSB,
 *  NlmRecordIndex *o_bestFreeSlotIndex_p,
 *  NlmBool *o_isForwardShift_p,
 *  Nlm__Marker* o_shiftStartMarker_p
 *
 * Summary:
 * Finds the best free slot in the blocks in which there is currently no
 * record of the table (new block).
 * If lookInOldSB is true then  a new block within a super block 
 * where atleast one record of the table is present is looked up
 * If lookInOldSB is false, then a new block within a super block
 * where no records of the table are present is looked up
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_FindBestFreeSlotInNewBlks(
    NlmBlkMemMgrFnArgs_t* args_p,
    void * groupIdHead_p,
    NlmRecordIndex markerIndex,
    Nlm__Marker priorityMarker,
    NlmBool lookInOldSB,
    NlmRecordIndex *o_bestFreeSlotIndex_p,
    NlmBool *o_isForwardShift_p,
    Nlm__Marker* o_shiftStartMarker_p)
{
    NlmBlkMemMgr_t* blkMemMgr_p = args_p->blkMemMgr_p;
    nlm_32 curBackBlk = INDEX_TO_BLK_NUM(markerIndex);
    nlm_32 curForwardBlk = curBackBlk;
    nlm_32 curSB = 0;
    NlmBool isTblInSB = NlmFalse;
    NlmBool foundFreeSlot = NlmFalse;
    NlmRecordIndex leftFreeSlot = INVALID_INDEX;
    NlmRecordIndex rightFreeSlot = INVALID_INDEX;
    NlmErrNum_t errNum = NLMERR_OK;


    while(curBackBlk >= 0)
    {
        /* Find if table is present in the super block */
        isTblInSB = NlmBlkMemMgr__pvt_FindTblInSB(blkMemMgr_p->m_blkInfo_p,
                                        curBackBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB,
                                                args_p->genericTbl_p->m_tblId);

        /* Table is in SuperBlock and we are looking for an already assigned super block OR
        Table is not in Super Block and we are looking for a new super block and table can
        enter the new super block*/
        if((isTblInSB && lookInOldSB) ||
            (!isTblInSB && !lookInOldSB && 
            NlmBlkMemMgr__pvt_CanTblEnterBlkInNewSB(blkMemMgr_p->m_blkInfo_p,
                                            curBackBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB,
                                                    args_p->genericTbl_p)))
        {
            curSB = curBackBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB;
            
            /* Process the blocks in the super block */
            while(curBackBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB  == curSB && curBackBlk >= 0)
            {
                /* We need to check only those blocks in which given table is not present as we are finding 
                   slot in NEW blocks. If table is present then ignore this block and continue with next block */
                
                /* If m_keepSeparate is set for this block then 1) this block can not be used for other tables 
                   2) This block can not be a NEW block for the same table */

                if(blkMemMgr_p->m_blkInfo_p[curBackBlk].m_keepSeparate != 0)
                {
                    --curBackBlk;
                    continue;
                }

                /* If keep separate is set and if this block is having some records then it can not be a new block for this 
                table thus search another block */
                if((args_p->genericTbl_p->m_keepSeparate) &&
                    (blkMemMgr_p->m_blkInfo_p[curBackBlk].m_numUsedRecs != 0))
                {
                    --curBackBlk;
                    continue;
                }

                /* Check and proceed only if table is not present in that block (NEW block)*/
                if(!NlmBlkMemMgr__pvt_FindTblInBlk(blkMemMgr_p->m_blkInfo_p,
                                            curBackBlk,
                                            args_p->genericTbl_p->m_tblId,
                                            NULL))
                {
                    /*if the block is completely empty then reinitialize the
                    free slot manager of the block to the correct block width */
                    if(blkMemMgr_p->m_blkInfo_p[curBackBlk].m_numUsedRecs == 0)
                    {
                        NlmBlkMemMgr__pvt_ReInitFreeSlotMgr(args_p,
                                                curBackBlk);
                        blkMemMgr_p->m_blkInfo_p[curBackBlk].m_adWidth = NLM_TBL_ADLEN_END;
                        blkMemMgr_p->m_blkInfo_p[curBackBlk].m_keepSeparate = 0;
                    }
                    
                    if(!ISFULL(blkMemMgr_p->m_blkInfo_p, curBackBlk) && 
                        blkMemMgr_p->m_blkInfo_p[curBackBlk].m_blkWidth == args_p->newRecWidth)
                    {
                        if((blkMemMgr_p->m_blkInfo_p[curBackBlk].m_adWidth == NLM_TBL_ADLEN_END) ||
                            (blkMemMgr_p->m_blkInfo_p[curBackBlk].m_adWidth == args_p->genericTbl_p->m_adWidth))
                        {

                            foundFreeSlot = NlmFreeSlotMgr__FindNearestFreeSlot( 
                                            blkMemMgr_p->m_blkInfo_p[curBackBlk].m_freeSlotMgr_p,
                                            markerIndex,
                                            NlmFalse,
                                            &leftFreeSlot);

                            if(foundFreeSlot)
                                break;
                        }
                    }

                }
                --curBackBlk;
            }

            if(foundFreeSlot)
                break;
        }
        else
        {
            /*Proceed to the previous super block */
            curBackBlk = (curBackBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB) * blkMemMgr_p->m_blkInfo_p->m_BlksInSB - 1;
        }
    }



    while(curForwardBlk < blkMemMgr_p->m_NumOfABs * blkMemMgr_p->m_nrOfDevices)
    {
        /* Find if table is present in the super block */
        isTblInSB = NlmBlkMemMgr__pvt_FindTblInSB(blkMemMgr_p->m_blkInfo_p,
                                            curForwardBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB,
                                                    args_p->genericTbl_p->m_tblId);

        /* Table is in SuperBlock and we are looking for an already assigned super block OR
        Table is not in Super Block and we are looking for a new super block and table can
        enter the new super block*/
        if((isTblInSB && lookInOldSB) ||
            (!isTblInSB && !lookInOldSB && 
              NlmBlkMemMgr__pvt_CanTblEnterBlkInNewSB(blkMemMgr_p->m_blkInfo_p,
                                            curForwardBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB,
                                                        args_p->genericTbl_p)))
        {
            curSB = curForwardBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB;

            /* Process the blocks in the super block */
            while(curForwardBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB == curSB )
            {
                /* We need to check only those blocks in which given table is not present as we are finding 
                slot in NEW blocks. If table is present then ignore this block and continue with next block */
 
                /* If m_keepSeparate is set for this block then 1) this block can not be used for other tables 
                   2) This block can not be a NEW block for the same table */

                if(blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_keepSeparate != 0)
                {
                    ++curForwardBlk;
                    continue;
                }
 
 
               /* If keep separate is set and if this block is having some records then it can not be a new block for this 
                  table thus search another block */
               if((args_p->genericTbl_p->m_keepSeparate != 0) &&
                  (blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_numUsedRecs != 0))
               {
                    ++curForwardBlk;
                    continue;
               }
  
               /* Check and proceed only if table is not present in that block (NEW block)*/    
               if(!NlmBlkMemMgr__pvt_FindTblInBlk(blkMemMgr_p->m_blkInfo_p,
                                        curForwardBlk,
                                        args_p->genericTbl_p->m_tblId,
                                        NULL))
                {
                    /*if the block is completely empty then reinitialize the
                    free slot manager of the block to the correct block width */
                    if(blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_numUsedRecs == 0)
                    {
                        NlmBlkMemMgr__pvt_ReInitFreeSlotMgr(args_p,
                                                curForwardBlk);
                        blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_adWidth = NLM_TBL_ADLEN_END;
                        blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_keepSeparate = 0;
                    }
                    
                    if(!ISFULL(blkMemMgr_p->m_blkInfo_p, curForwardBlk) && 
                        blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_blkWidth == args_p->newRecWidth)
                    {
                        if((blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_adWidth == NLM_TBL_ADLEN_END) ||
                            (blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_adWidth == args_p->genericTbl_p->m_adWidth))
                        {
                            foundFreeSlot = NlmFreeSlotMgr__FindNearestFreeSlot( 
                                            blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_freeSlotMgr_p,
                                            markerIndex,
                                            NlmTrue,
                                            &rightFreeSlot);
                            if(foundFreeSlot)
                                break;
                        }
                    }
                }

                ++curForwardBlk;
            }

            if(foundFreeSlot)
                break;
        }
        else
        {
            /*Proceed to the next super block */
            curForwardBlk = (curForwardBlk / blkMemMgr_p->m_blkInfo_p->m_BlksInSB)* blkMemMgr_p->m_blkInfo_p->m_BlksInSB
                + blkMemMgr_p->m_blkInfo_p->m_BlksInSB;
        }
    }

    errNum = NlmBlkMemMgr__pvt_ChooseOneFreeSlot(args_p,
                                  leftFreeSlot,
                                  rightFreeSlot,
                                  groupIdHead_p,
                                  priorityMarker,
                                  o_bestFreeSlotIndex_p,
                                  o_isForwardShift_p,
                                  o_shiftStartMarker_p);
    
    return errNum;

}


/************************************************
 * Function : NlmBlkMemMgr__pvt_FindBestFreeSlot
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t* args_p,
 *  void * groupIdHead_p,
 *  NlmRecordIndex markerIndex,
 *  Nlm__Marker priorityMarker,
 *  NlmRecordIndex *o_bestFreeSlotIndex_p,
 *  NlmBool *o_isForwardshift_p,
 *  Nlm__Marker* o_shiftStartMarker_p
 *
 * Summary:
 * Finds the best free slot for the record to be inserted
 * First the blocks where there is at least one record of the table
 * is present are looked up for free slots
 * Then the blocks where a record of the table is not present, but the
 * super block has at least one record of the table are looked up
 * Then the blocks where no record of the table is present in the entire
 * super block are looked up
 *
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_FindBestFreeSlot(
    NlmBlkMemMgrFnArgs_t* args_p,
    void * groupIdHead_p,
    NlmRecordIndex markerIndex,
    Nlm__Marker priorityMarker,
    NlmRecordIndex *o_bestFreeSlotIndex_p,
    NlmBool *o_isForwardshift_p,
    Nlm__Marker* o_shiftStartMarker_p)
{
    NlmErrNum_t errNum = NLMERR_OK;

    *o_bestFreeSlotIndex_p = INVALID_INDEX;

    /*Find the free slot in an already assigned block that requires the least 
    number of shuffles */
    errNum = NlmBlkMemMgr__pvt_FindBestFreeSlotInOldBlks(args_p,
                                    groupIdHead_p,
                                    markerIndex,
                                    priorityMarker,
                                    o_bestFreeSlotIndex_p,
                                    o_isForwardshift_p,
                                    o_shiftStartMarker_p);


    /* Then look at the free slots in an unassigned block of an already
    assigned super block*/
    if(*o_bestFreeSlotIndex_p == INVALID_INDEX)
    {
        errNum = NlmBlkMemMgr__pvt_FindBestFreeSlotInNewBlks(args_p,
                                        groupIdHead_p,
                                        markerIndex,
                                        priorityMarker,
                                        NlmTrue,
                                        o_bestFreeSlotIndex_p,
                                        o_isForwardshift_p,
                                        o_shiftStartMarker_p);
    }

    /* Then look at the free slots in an unassigned block of an 
    unassigned super block*/
    if(*o_bestFreeSlotIndex_p == INVALID_INDEX)
    {

        errNum = NlmBlkMemMgr__pvt_FindBestFreeSlotInNewBlks(args_p,
                                        groupIdHead_p,
                                        markerIndex,
                                        priorityMarker,
                                        NlmFalse,
                                        o_bestFreeSlotIndex_p,
                                        o_isForwardshift_p,
                                        o_shiftStartMarker_p);
    }

    return errNum;
}


/************************************************
 * Function : NlmBlkMemMgr__pvt_AddRecord
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t* args_p
 *
 * Summary:
 * This function adds the new record to the best free slot
 * available. It finds and move the marker index to left
 * and right direction to find a free slot and minimium number
 * of moves decide the best free  slot.
 *************************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_AddRecord(
    NlmBlkMemMgrFnArgs_t* args_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = args_p->blkMemMgr_p;
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    NlmErrNum_t errNum = NLMERR_OK; 
    NlmTblPvtData_t *tblPvtData_p = (NlmTblPvtData_t*)args_p->genericTbl_p->m_privateData;
    void *groupIdHead_p = NULL;
    NlmRecordIndex markerIndex = 0;
    NlmRecordIndex bestFreeSlotIndex = 0;
    Nlm__Marker priorityMarker ;
    Nlm__Marker shiftStartMarker;
    nlm_32 bestFreeSlotBlk = 0;
    NlmBool isForwardShift = NlmFalse;

    NlmCm__memset(&priorityMarker, 0, sizeof(Nlm__Marker));
    NlmCm__memset(&shiftStartMarker, 0, sizeof(Nlm__Marker));
    priorityMarker.number_of_shuffles = -1;
    shiftStartMarker.number_of_shuffles = -1;

    /* Get the head node for the group from the hash table*/
    groupIdHead_p = NlmCmTwoLevelHashTbl__GetMember(
                                        (NlmCmTwoLevelHashTbl*)tblPvtData_p->m_groupIdHashTbl_p,
                                        args_p->groupId,
                                        args_p->o_reason_p);
    if(!groupIdHead_p)
    {
        /*Call the create API of data structure used to hold the 
        entries within a group and obtain the head pointer of the group
        */
        groupIdHead_p = NlmGP__CreateList(blkMemMgr_p->m_alloc_p,
                                            100,
                                            args_p->newRecWidth / MIN_BLK_WIDTH,
                                            NULL,
                                            args_p->o_reason_p);

        if(!groupIdHead_p)
            return NLMERR_FAIL;

        /*Store the head pointer in the hash table*/
        errNum = NlmCmTwoLevelHashTbl__Insert((NlmCmTwoLevelHashTbl*)tblPvtData_p->m_groupIdHashTbl_p, 
                                                args_p->groupId,
                                                groupIdHead_p,
                                                args_p->o_reason_p);

        if(errNum != NLMERR_OK)
            return errNum;
    }

    /*Find the marker index for the record priority. So if record with priority
    9 is being inserted and record with priority 9 is already present, then
    marker index is the index of the first 9. */
    errNum = NlmGP__GetMarkerForPriority(groupIdHead_p,
                                                args_p->newRecPriority,
                                                &markerIndex,   
                                                &priorityMarker,
                                                args_p, 
                                                args_p->o_reason_p);

    if(errNum != NLMERR_OK)
        return errNum;

    /*Given the marker index scan in the upward and downward directions
    to find the left free slot and right free slot. The free slot which
    would result in the least number of shuffles is chosen as the best
    free slot*/
    errNum = NlmBlkMemMgr__pvt_FindBestFreeSlot(args_p,
                                        groupIdHead_p,
                                        markerIndex,
                                        priorityMarker,
                                        &bestFreeSlotIndex,
                                        &isForwardShift,
                                        &shiftStartMarker);

    if(errNum != NLMERR_OK)
        return errNum;

    /*There is no free space available on the chip */
    if(bestFreeSlotIndex == INVALID_INDEX)
    {
        if(args_p->o_reason_p)
            *args_p->o_reason_p = NLMRSC_TABLE_FULL;

        return NLMERR_FAIL;
    }

    bestFreeSlotBlk = INDEX_TO_BLK_NUM(bestFreeSlotIndex);

    /* If it is a new block then UDA needs to be allocated */
    if(blkInfo_p[bestFreeSlotBlk].m_numUsedRecs == 0)
    {
        if(args_p->genericTbl_p->m_adWidth != NLM_TBL_ADLEN_ZERO)
        {
            nlm_u16 udaChunkWidth = (nlm_u16) (1 << (args_p->genericTbl_p->m_adWidth -1)) * 32;
            nlm_u16 udaChunkSize =  (nlm_u16) (NLMDEV_AB_DEPTH * NLM_TBL_WIDTH_80 / args_p->genericTbl_p->m_width);
            
            errNum = NlmUdaMemMgr__AllocateUdaChunk(
                        args_p->blkMemMgr_p->m_udaMemMgr_p, args_p->genericTbl_p->m_tblId, 
                        args_p->genericTbl_p->m_psTblList_p, udaChunkSize, udaChunkWidth, 
                        &blkInfo_p[bestFreeSlotBlk].m_udaChunkInfo_p, args_p->o_reason_p);
            
            if(errNum != NLMERR_OK)
                return errNum;          
        }

        blkInfo_p[bestFreeSlotBlk].m_adWidth = args_p->genericTbl_p->m_adWidth;
        
    }

    /*Remove the best free slot from the free slot manager*/
    errNum = NlmFreeSlotMgr__RemoveFreeSlot(
                    blkInfo_p[bestFreeSlotBlk].m_freeSlotMgr_p,
                    bestFreeSlotIndex,
                    args_p->o_reason_p);

    if(errNum != NLMERR_OK)
        return errNum;

    /*Mark the free slot as occupied*/
    errNum = NlmBlkMemMgr__pvt_MarkFreeSlotAsOccupied(args_p, bestFreeSlotIndex);
    if(errNum != NLMERR_OK)
        return errNum;

    /*Use the free slot as the starting position for the shifts.
    Start performing the shifts in the direction specified until
    the free slot is moved to the correct location where the 
    current record to be added can be inserted. Insert the 
    current record at the free slot*/
    errNum = NlmGP__InsertRecord(   groupIdHead_p,
                                    args_p->blkMemMgr_p->m_alloc_p,
                                    args_p->newRecPriority,
                                    &priorityMarker,
                                    &shiftStartMarker,
                                    bestFreeSlotIndex,
                                    isForwardShift,
                                    NlmBlkMemMgr__pvt_IndexChangedInGroup,
                                    args_p);
    return errNum;
    
}



/************************************************
 * Function : NlmBlkMemMgr__pvt_MarkFreeSlotAsOccupied
 * 
 * Parameters:
 *  NlmBlkMemMgrFnArgs_t* args_p,
 *  NlmRecordIndex freeRecIndex
 *
 * Summary:
 * Marks up the free slot as being occupied in the internal
 * data structures. If this free slot is in a complete empty 
 * or new block, sets the block width and add the table into
 * the linklist of the tables of block
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_MarkFreeSlotAsOccupied(
    NlmBlkMemMgrFnArgs_t* args_p,
    NlmRecordIndex freeRecIndex
    )
{
    NlmBlkMemMgr_t *blkMemMgr_p = args_p->blkMemMgr_p;
    NlmBlkInfo_t * blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    nlm_32 curBlk = INDEX_TO_BLK_NUM(freeRecIndex);
    NlmTblsInBlk_t* tblInBlk_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
                
    
    /*The blk having free slot is completely empty */
    if(blkInfo_p[curBlk].m_numUsedRecs == 0)
    {
        
        /*Set the blk width */
        blkInfo_p[curBlk].m_blkWidth = args_p->newRecWidth;

        
        /*Inform upper layer to set blk width via callback */
        if(blkMemMgr_p->m_setBlkWidthCB)
        {
            errNum = blkMemMgr_p->m_setBlkWidthCB(blkMemMgr_p->m_ctx_p,
                                            curBlk, args_p->newRecWidth,
                                            blkInfo_p[curBlk].m_udaChunkInfo_p,
                                            args_p->o_reason_p);
            if(errNum != NLMERR_OK)
                return errNum;
        }
    }
    ++blkInfo_p[curBlk].m_numUsedRecs;


    /*If this is the first record of the tbl in the blk */
    if(!NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p, curBlk,
                    args_p->genericTbl_p->m_tblId,
                    &tblInBlk_p))
    {
        /*Add the tbl to the linked list in the blk */
        tblInBlk_p = NlmBlkMemMgr__pvt_InsertTblsInBlk(blkMemMgr_p->m_alloc_p,
                        blkInfo_p[curBlk].m_tblList_p,
                        args_p->genericTbl_p->m_tblId);

        if(!tblInBlk_p)
        { 
            if(args_p->o_reason_p) 
                *args_p->o_reason_p = NLMRSC_LOW_MEMORY; 
            return NLMERR_FAIL;
        }
        
        blkInfo_p[curBlk].m_keepSeparate = args_p->genericTbl_p->m_keepSeparate;
        /*inform the upper layer to add the blk for the 
        table */
        if(blkMemMgr_p->m_blkChangedForTblCB)
        {
            errNum = blkMemMgr_p->m_blkChangedForTblCB(blkMemMgr_p->m_ctx_p,
                                                    args_p->genericTbl_p,
                                                    curBlk,
                                                    NlmTrue, 
                                                    NlmFalse,
                                                    args_p->o_reason_p);
            if(errNum != NLMERR_OK)
                return errNum;
        }
    }

    (tblInBlk_p->m_numRecs)++;

    return errNum;
}



/************************************************
 * Function : NlmBlkMemMgr__Init
 * 
 * Parameters:
 *  NlmCmAllocator          *alloc_p,
 *  NlmIndexChangedTblMgrCB indexChangedCB,
 *  NlmSetBlkWidthCB        setblkWidthCB,
 *  NlmBlkChangedForTblCB   blkChangedForTblCB,
 *  NlmRemoveRecordCB       removeRecordCB,
 *  nlm_u8                  nrOfDevices,
 *  void                     *ctx_p,
 *  NlmReasonCode           *o_reason_p
 *
 * Summary:
 * Initializes the blk memory manager. Allocates the 
 * memory for block memory manager and block info. 
 * Initializes the block info for 64 tables. Initializes
 * call back functions.
 *
 ***********************************************/
void* NlmBlkMemMgr__Init(
    NlmCmAllocator          *alloc_p,
    NlmIndexChangedTblMgrCB indexChangedCB,
    NlmSetBlkWidthCB        setblkWidthCB,
    NlmBlkChangedForTblCB   blkChangedForTblCB,
    NlmRemoveRecordCB       removeRecordCB,
    nlm_u8                  nrOfDevices,
    NlmGenericTblMgrBlksRange      *dbaBlksRange,
    NlmGenericTblMgrSBRange      *udaSbRange,
    void                     *ctx_p,
    nlm_u16                 NumOfABs,
    NlmReasonCode           *o_reason_p
    )
{
        nlm_32 curBlk = 0;
        NlmErrNum_t status = NLMERR_OK;
        nlm_u8 devBlkNum;
        nlm_u8 devNum;
        
        /* Allocate the memory required memory */
        NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*)NlmCmAllocator__calloc(alloc_p, 1,
                                                sizeof(NlmBlkMemMgr_t));

        if(!blkMemMgr_p)
        {
            if(o_reason_p) *o_reason_p = NLMRSC_LOW_MEMORY;
            return NULL;
        }

        blkMemMgr_p->m_alloc_p = alloc_p;

        blkMemMgr_p->m_blkInfo_p = (NlmBlkInfo_t*)NlmCmAllocator__calloc(alloc_p, 1,
                                    sizeof(NlmBlkInfo_t) * NumOfABs * nrOfDevices);

        if(!blkMemMgr_p->m_blkInfo_p)
        {
            if(o_reason_p) *o_reason_p = NLMRSC_LOW_MEMORY;
            return NULL;
        }
        blkMemMgr_p->m_NumOfABs = NumOfABs;
        blkMemMgr_p->m_blkInfo_p->m_BlksInSB = (nlm_u8)(NumOfABs/NLMDEV_NUM_SUPER_BLOCKS);

        for(curBlk = 0;curBlk < NumOfABs * nrOfDevices; ++curBlk)
        {            
            devNum = (nlm_u8)(curBlk/NumOfABs);
            devBlkNum = (nlm_u8)(curBlk%NumOfABs);

            if((dbaBlksRange[devNum].m_startBlkNum <= devBlkNum) && 
                (devBlkNum <= dbaBlksRange[devNum].m_endBlkNum))
            {
                blkMemMgr_p->m_blkInfo_p[curBlk].m_validBlk = NLM_VALID_GTM_BLK;
                blkMemMgr_p->m_blkInfo_p[curBlk].m_blkWidth = MIN_BLK_WIDTH;
                blkMemMgr_p->m_blkInfo_p[curBlk].m_numUsedRecs = 0;
                blkMemMgr_p->m_blkInfo_p[curBlk].m_adWidth = NLM_TBL_ADLEN_END;
                blkMemMgr_p->m_blkInfo_p[curBlk].m_recInfo_p = (NlmRecInfo_t*)NlmCmAllocator__calloc(alloc_p, 1,
                                                sizeof(NlmRecInfo_t)*MAX_NUM_ENTRIES_PER_BLK);
                
                status = NlmBlkMemMgr__pvt_InitTblsInBlk(blkMemMgr_p->m_blkInfo_p,
                                                            curBlk,
                                                            alloc_p,
                                                            o_reason_p);

                if(status != NLMERR_OK) 
                    return NULL;

                blkMemMgr_p->m_blkInfo_p[curBlk].m_freeSlotMgr_p = 
                                                    NlmFreeSlotMgr__Init(
                                                        blkMemMgr_p->m_alloc_p,
                                                        curBlk * MAX_NUM_ENTRIES_PER_BLK,
                                                        (curBlk+1) * MAX_NUM_ENTRIES_PER_BLK - 1,
                                                        1,
                                                        o_reason_p);

                if(!blkMemMgr_p->m_blkInfo_p[curBlk].m_freeSlotMgr_p)
                    return NULL;
            }
            else
            {
                blkMemMgr_p->m_blkInfo_p[curBlk].m_validBlk = NLM_INVALID_GTM_BLK;
            }
        }

        /*Initialize the callback function pointers */
        blkMemMgr_p->m_indexChangedCB = indexChangedCB;
        blkMemMgr_p->m_setBlkWidthCB = setblkWidthCB;
        blkMemMgr_p->m_blkChangedForTblCB = blkChangedForTblCB;
        blkMemMgr_p->m_removeRecordCB = removeRecordCB;

        /*Store the number of devices in the cascade */
        blkMemMgr_p->m_nrOfDevices = nrOfDevices;

        /*Initialize the argument to be passed back to the upper layer 
        when calling the callback functions*/
        blkMemMgr_p->m_ctx_p = ctx_p;

        if( udaSbRange->m_stSBNum != NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED &&
            udaSbRange->m_endSBNum != NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED )
        {
            /* Create UDA Memory Manager */
            blkMemMgr_p->m_udaMemMgr_p = NlmUdaMemMgr__Init(alloc_p, udaSbRange->m_stSBNum,  
                    udaSbRange->m_endSBNum - udaSbRange->m_stSBNum + 1, o_reason_p);
        }
        
        return blkMemMgr_p;
}


/************************************************
 * Function : NlmBlkMemMgr__InitTbl
 * 
 * Parameters:
 *  void*           self_p,         
 *  NlmGenericTbl       *genericTbl_p,
 *  NlmReasonCode       *o_reason_p
 *
 * Summary:
 * Initialize the data structures for the table. 
 * Initialize the hash table that hashes the groupId and is maintained 
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__InitTbl(
    void*           self_p,         
    NlmGenericTbl       *genericTbl_p,
    NlmReasonCode       *o_reason_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 

    NlmErrNum_t errNum = NLMERR_OK;

    NlmTblPvtData_t *tblPvtData_p = NULL;

    if(blkMemMgr_p == NULL) 
        return NLMERR_NULL_PTR;

    genericTbl_p->m_privateData =  NlmCmAllocator__malloc(
                                                blkMemMgr_p->m_alloc_p, 
                                                sizeof(NlmTblPvtData_t));

    tblPvtData_p = (NlmTblPvtData_t *) genericTbl_p->m_privateData;

    
    /* Initialize the hash table that hashes the groupId and is maintained 
    per table. The head node for the group is stored in the hash table*/
    tblPvtData_p->m_groupIdHashTbl_p = NlmCmTwoLevelHashTbl__Init(
                                            GROUPID_HASH_TBL_SIZE,
                                            GROUPID_HASH_TBL_LEVEL_2_NR_OF_ELEMS,
                                            blkMemMgr_p->m_alloc_p,
                                            NlmBlkMemMgr__pvt_DestroyElemInHashTbl,
                                            o_reason_p);
    
    if(!tblPvtData_p->m_groupIdHashTbl_p)
        return NLMERR_FAIL;

    return errNum;
}



/************************************************
 * Function :
 * NlmBlkMemMgr__AddRecord
 * 
 * Parameters:
 *  void*           self_p,         
 *  NlmGenericTbl   *genericTbl_p,  
 *  nlm_u16         newRecPriority, 
 *  nlm_u16         groupId,
 *  NlmRecordIndex  *o_recIndex_p, 
 *  nlm_16          newRecWidth,
 *  NlmReasonCode   *o_reason_p
 *
 * Summary:
 * Locate the free index, shift the records according to priority and
 * insert the record in the correct location based on priority 
 *
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__AddRecord(
    void*           self_p,         
    NlmGenericTbl   *genericTbl_p,  
    nlm_u16         newRecPriority, 
    nlm_u16         groupId,
    NlmRecordIndex  *o_recIndex_p, 
    nlm_16          newRecWidth,
    NlmBool         *isShuffleDown_p,
    NlmReasonCode   *o_reason_p
    )
{
    
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkMemMgrFnArgs_t args;
    NlmErrNum_t errNum = NLMERR_OK; 
    

    if(blkMemMgr_p == NULL) 
        return NLMERR_NULL_PTR;

    if(newRecWidth > MAX_BLK_WIDTH || newRecWidth % MIN_BLK_WIDTH != 0)
    {
        if(o_reason_p) *o_reason_p = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }

    /* smaller numbers are treated as higher priorities */
    
    NlmBlkMemMgr__pvt_InitFnArgs(&args,genericTbl_p,groupId,newRecPriority,
                        o_recIndex_p, newRecWidth, o_reason_p, blkMemMgr_p);


    errNum = NlmBlkMemMgr__pvt_AddRecord(&args);

    if(isShuffleDown_p)
        *isShuffleDown_p = args.isShuffleDown;

    return errNum;
}


/************************************************
 * Function : NlmBlkMemMgr__DeleteRecord
 * 
 * Parameters:
 *  void*               self_p,         
 *  NlmGenericTbl       *genericTbl_p,  
 *  NlmRecordIndex      recIndex,   
 *  NlmReasonCode       *o_reason_p
 *
 * Summary:
 * Delete a record of genericTbl_p from recIndex. It
 * fetches the block number and relative offset within
 * the block to delete the record. After deleting it updates
 * the blockInfo.
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__DeleteRecord(
    void*               self_p,         
    NlmGenericTbl       *genericTbl_p,  
    NlmRecordIndex      recIndex,   
    NlmReasonCode       *o_reason_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = NULL;
    nlm_32 curBlk = 0, curRecOffset = 0;
    nlm_u16 recPriority = 0;
    NlmBool found = NlmFalse;
    NlmTblsInBlk_t* tblInBlk_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblPvtData_t *tblPvtData_p = (NlmTblPvtData_t*)genericTbl_p->m_privateData;
    void *groupIdHead_p = NULL;
    nlm_u16 groupId = 0;

    if(blkMemMgr_p == NULL) 
        return NLMERR_NULL_PTR;

    blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    
    /*The index passed by the upper layer is outside the range of the 
    chip. So return an error */
    if(recIndex >= (MAX_REC_OFFSET + 1) * TOTAL_NUM_SB * blkMemMgr_p->m_blkInfo_p->m_BlksInSB * 
                    (nlm_u32) blkMemMgr_p->m_nrOfDevices)
    {
        if(o_reason_p) *o_reason_p = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }
    
    /*Convert the absolute index into blk number and relative offset within 
    the blk */
    curBlk = INDEX_TO_BLK_NUM(recIndex);

    curRecOffset = INDEX_TO_OFFSET(recIndex);

    /*If a record for the table doesn't exist at the location, return an error */
    if(blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId != genericTbl_p->m_tblId)
    {
        if(o_reason_p) *o_reason_p = NLMRSC_RECORD_NOTFOUND;
        return NLMERR_FAIL;
    }

    /*Find the priority of the record to help us lookup the corrrect place
    in the linked list of priorities maintained for the table*/
    recPriority = blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_priority;
    groupId = blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_groupId;

    /*Update information in the blk*/
    --blkInfo_p[curBlk].m_numUsedRecs;

    blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId = 0xFF;


    tblInBlk_p = (NlmTblsInBlk_t*)blkInfo_p[curBlk].m_tblList_p->m_next_p;

    found = NlmFalse;

    /*Iterate through the linked list of tables maintained for the blk */
    while(!found && tblInBlk_p != blkInfo_p[curBlk].m_tblList_p)
    {
        if(tblInBlk_p->m_tblId == genericTbl_p->m_tblId)
        {
            found = NlmTrue;
            break;
        }
        tblInBlk_p = (NlmTblsInBlk_t*)tblInBlk_p->m_next_p;
    }
    
    if(!found) 
    {
        NlmCmAssert(0, "Entry not found \n"); 
        return NLMERR_FAIL;
    }

    /* Update the table information in the linked list of tables maintained 
    for the blk*/
    --tblInBlk_p->m_numRecs;

    if(tblInBlk_p->m_numRecs < 0) 
    { 
        NlmCmAssert(0, "Incorrect number of records \n"); 
        return NLMERR_FAIL;
    }
    
    if(tblInBlk_p->m_numRecs == 0)
    {
        NlmBool isBlkEmpty = NlmFalse;

        if(blkInfo_p[curBlk].m_numUsedRecs == 0)
        {
            isBlkEmpty = NlmTrue;
            blkInfo_p[curBlk].m_keepSeparate = 0;
        }

        /*If there are no more records of the tbl in the blk 
        remove the tbl from the linked list*/
        NlmBlkMemMgr__pvt_RemoveTblsInBlk(blkMemMgr_p->m_alloc_p, tblInBlk_p);
        
        /*inform that there are no more records of table in this blk */
        if(blkMemMgr_p->m_blkChangedForTblCB)
        {
            errNum = blkMemMgr_p->m_blkChangedForTblCB(blkMemMgr_p->m_ctx_p,
                                                    genericTbl_p,
                                                    curBlk,
                                                    NlmFalse,
                                                    isBlkEmpty,
                                                    o_reason_p);
            if(errNum != NLMERR_OK)
                return errNum;

        }
    }
    
    found = NlmFalse;
    
    /*Clean up in the data structure  maintained per groupId*/
    groupIdHead_p = NlmCmTwoLevelHashTbl__GetMember(
                                        (NlmCmTwoLevelHashTbl*)tblPvtData_p->m_groupIdHashTbl_p,
                                        groupId,
                                        o_reason_p);

    if(groupIdHead_p)
    {
        errNum = NlmGP__DeleteRecord(groupIdHead_p,
                                blkMemMgr_p->m_alloc_p,
                                recPriority,
                                recIndex);
    }

    if(!groupIdHead_p || errNum != NLMERR_OK)
    {
        NlmCmAssert(0, "Entry not found \n"); 
        return NLMERR_FAIL;
    }

    /*Add the record to the free slot pool maintained per block */
    errNum = NlmFreeSlotMgr__AddFreeSlot(blkInfo_p[curBlk].m_freeSlotMgr_p,
                                        recIndex,
                                        o_reason_p);

    if(errNum != NLMERR_OK)
    {
        NlmCmAssert(0, "Entry not found \n"); 
        return NLMERR_FAIL;
    }
    
    return errNum;    
    
}

/************************************************
 * Function : NlmBlkMemMgr__ValidateRecord
 * 
 * Parameters:
 *  void*               self_p,         
 *  NlmGenericTbl       *genericTbl_p,  
 *  NlmRecordIndex      recIndex,   
 *  NlmReasonCode       *o_reason_p
 *
 * Summary:
 * Validate a record of genericTbl_p from recIndex. It checks the 
 * table id associated at recordIndex with the input table id and if 
 * both matches then returns success otherwise failure.
 * Return value of 0 indicates success, and anything else is failure.
 *  
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__ValidateRecord(
    void*               self_p,         
    NlmGenericTbl       *genericTbl_p,  
    NlmRecordIndex      recIndex,
    NlmReasonCode       *o_reason_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = NULL;
    nlm_32 curBlk = 0, curRecOffset = 0;

    if(blkMemMgr_p == NULL) 
        return NLMERR_NULL_PTR;

    blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    
    /*The index passed by the upper layer is outside the range of the 
    chip. So return an error */
    if(recIndex >= (MAX_REC_OFFSET + 1) * TOTAL_NUM_SB * blkMemMgr_p->m_blkInfo_p->m_BlksInSB * 
                    (nlm_u32) blkMemMgr_p->m_nrOfDevices)
    {
        if(o_reason_p) *o_reason_p = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }
    
    /*Convert the absolute index into blk number and relative offset within 
    the blk */
    curBlk = INDEX_TO_BLK_NUM(recIndex);

    curRecOffset = INDEX_TO_OFFSET(recIndex);

    /*If a record for the table doesn't exist at the location, return an error */
    if(blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId != genericTbl_p->m_tblId)
    {
        if(o_reason_p) *o_reason_p = NLMRSC_RECORD_NOTFOUND;
        return NLMERR_FAIL;
    }

    return NLMERR_OK;
}


NlmErrNum_t 
NlmBlkMemMgr__GetUdaChunkInfo(
    void*               self_p,         
    NlmRecordIndex      recIndex,   
    NlmUdaChunkInfo **udaChunkInfo__pp,
    NlmReasonCode       *o_reason_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = NULL;
    nlm_32 curBlk = 0;

    NlmErrNum_t errNum = NLMERR_OK;

    if((blkMemMgr_p == NULL) || (udaChunkInfo__pp == NULL)) 
        return NLMERR_NULL_PTR;

    blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    
    /*The index passed by the upper layer is outside the range of the 
    chip. So return an error */
    if(recIndex >= (MAX_REC_OFFSET + 1) * TOTAL_NUM_SB * blkMemMgr_p->m_blkInfo_p->m_BlksInSB * 
                    (nlm_u32) blkMemMgr_p->m_nrOfDevices)
    {
        *o_reason_p = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }
    
    /*Convert the absolute index into blk number and relative offset within 
    the blk */
    curBlk = INDEX_TO_BLK_NUM(recIndex);

    *udaChunkInfo__pp = blkInfo_p[curBlk].m_udaChunkInfo_p;

    return errNum;
    
}

nlm_u32
NlmBlkMemMgr_get_uda_address(
    nlm_u32 dbaRecIndex,
    nlm_u32 maxNumEntriesPerAB, 
    NlmDevBlockConfigReg * blockCfgReg)
{
    nlm_u32 udaBaseAddr = 0;
    nlm_u32 offset = 0;
    nlm_u32 udaAddr = 0;

    udaBaseAddr = blockCfgReg->m_baseAddr;

    udaBaseAddr <<= NLMDEV_SRAM_BASE_ADDR_SHIFT_NUM_BITS;
               
    if(blockCfgReg->m_shiftDir == NLMDEV_SHIFT_LEFT)
        offset = (dbaRecIndex % maxNumEntriesPerAB) << blockCfgReg->m_shiftCount;
    else
        offset = (dbaRecIndex % maxNumEntriesPerAB) >> blockCfgReg->m_shiftCount;

    udaAddr = (udaBaseAddr +  offset) % NLM_MAX_MEM_SIZE;

    return udaAddr;

}


NlmBool
NlmBlkMemMgr_check_tblId(
	void*    self_p,
	nlm_u16 blockNum,
	nlm_u8 tblId
	)
{
	NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    NlmTblsInBlk_t* tblInBlk_p = NULL, *head_p = NULL;
	NlmBool tblFound = 0;

	if(blkInfo_p[blockNum].m_validBlk)
    {
        head_p = blkInfo_p[blockNum].m_tblList_p;
            
            /* get the tabl list in the current block */
        tblInBlk_p = blkInfo_p[blockNum].m_tblList_p->m_next_p;

        while(tblInBlk_p != head_p)
        {
           if(tblInBlk_p->m_tblId == tblId)
           {
              tblFound = NlmTrue;
              break;
           }

           tblInBlk_p = tblInBlk_p->m_next_p;
        }
	}
	
	return tblFound;
}


NlmErrNum_t
NlmBlkMemMgr_iter_next(
    void*               self_p,
    NlmGenericTblIterData  *iterInfo,
    NlmGenericTblIterHandle *iterHandle ,
    NlmDevShadowDevice *shadowdevice,
    NlmDevShadowUdaSb *shadowUDA_p)
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    NlmTblsInBlk_t* tblInBlk_p = NULL, *head_p = NULL;
    nlm_u32 dbaindex = 0,groupNumber,udaAddr = 0;
    nlm_u16 priority = 0;
    nlm_u32 sbNr = 0, blkNrInSB = 0, rowNr = 0;
    NlmDevShadowAB* shadowAB_p = NULL;
    nlm_u16 jump = 0;
    nlm_u32 i = 0,j = 0;
    NlmBool tblFound = NlmFalse, found = NlmFalse;
    nlm_u16 adWidth = 0;
    nlm_u32 freeslotOffset = 0;
    NlmBool isfreeslot = NlmFalse;

    /*Get the size of Ad */
    if(iterHandle->m_tbl_p->m_adWidth)
    adWidth = ((1 << (iterHandle->m_tbl_p->m_adWidth - 1))*32);
    
    if(iterHandle->recCounter == iterHandle->m_tbl_p->m_num_records)
    {
        iterHandle->isRecsAvailable = 0;
        return NLMERR_FAIL;
    }

    /* if all blocks are searched then no more records to send */
    if(iterHandle->curBlock == (nlm_u16)(blkInfo_p->m_BlksInSB * TOTAL_NUM_SB))
    {
        iterHandle->isRecsAvailable = 0;
        return NLMERR_FAIL;
    }
    
    /* iterator through all the blocks */
    for(; iterHandle->curBlock < (nlm_u16)(blkInfo_p->m_BlksInSB * TOTAL_NUM_SB); 
            ++iterHandle->curBlock, iterHandle->CurRecOffset = 0)
    {
        /* check for block is valid or not */
        if(blkInfo_p[iterHandle->curBlock].m_validBlk)
        {
            head_p = blkInfo_p[iterHandle->curBlock].m_tblList_p;
            
            /* get the tabl list in the current block */
            tblInBlk_p = blkInfo_p[iterHandle->curBlock].m_tblList_p->m_next_p;

            while(tblInBlk_p != head_p)
            {
                if(tblInBlk_p->m_tblId == iterHandle->m_tbl_p->m_tblId)
                {
                    tblFound = NlmTrue;
                    break;
                }

                tblInBlk_p = tblInBlk_p->m_next_p;
            }
            
            if(!tblFound)
                continue;
                
            /* if Number of records in the corrent block related to corresponding table is zero no need to proceed. just continue */
            if(tblInBlk_p->m_numRecs == 0)
                continue;

            for( ;iterHandle->CurRecOffset< NLMDEV_AB_DEPTH ;)
            {
                freeslotOffset = ((iterHandle->curBlock << 12) | iterHandle->CurRecOffset);

                /*Get the shadow memory of AB */
                shadowAB_p = &(shadowdevice->m_arrayBlock[iterHandle->curBlock]);

                /* check for the valid slot */
                
                /*NlmFreeSlotMgr__FindNearestFreeSlot(blkInfo_p[iterHandle->curBlock].m_freeSlotMgr_p,
                                                    freeslotOffset,
                                                    1,
                                                    &freeslotIndex);*/
                isfreeslot = NlmFreeSlotMgr__isFreeSlot(blkInfo_p[iterHandle->curBlock].m_freeSlotMgr_p,
                                            freeslotOffset);

                /* check for the table id of the record: is that record belongs to the corresponding table? */
                if((blkInfo_p[iterHandle->curBlock].m_recInfo_p[iterHandle->CurRecOffset].m_tblId == iterHandle->m_tbl_p->m_tblId) && (isfreeslot == NlmFalse)/*(freeslotOffset != freeslotIndex)*/)
                {           
                    /* get priority & groupnumber */
                    priority = blkInfo_p[iterHandle->curBlock].m_recInfo_p[iterHandle->CurRecOffset].m_priority;
                    groupNumber = blkInfo_p[iterHandle->curBlock].m_recInfo_p[iterHandle->CurRecOffset].m_groupId;

                    /* get the DBA index : (blocknumber  << 12 | rowNum) */
                    dbaindex = ((iterHandle->curBlock << 12) | iterHandle->CurRecOffset);

                    /* fill the data for application : priority, groupnumber, index */ 
                    iterInfo->priority = priority;
                    iterInfo->groupId = groupNumber;
                    iterInfo->index = dbaindex;

                    /* get the data from the shadow memory for corresponding index */
                    for(jump = blkInfo_p[iterHandle->curBlock].m_blkWidth/80; jump > 0;jump--)
                    {
                            
                        for(i = 0;i < NLM_DATA_WIDTH_BYTES;i++)
                        {
                            if(shadowAB_p->m_abEntry[iterHandle->CurRecOffset + jump - 1].m_vbit ==1)
                            {
                                iterInfo->data[i+ j*10] = shadowAB_p->m_abEntry[iterHandle->CurRecOffset + jump -1].m_data[i];
                                iterInfo->mask[i+ j*10] = shadowAB_p->m_abEntry[iterHandle->CurRecOffset + jump - 1].m_mask[i];
                            }
                        }
                        j++;

                    }

                    iterHandle->recCounter++;
                    /* if AD present get AD for corrsponding DBA index from shadow memory*/
                    if(adWidth)
                    {
                        udaAddr = NlmBlkMemMgr_get_uda_address(dbaindex,NLMDEV_AB_DEPTH, &shadowAB_p->m_blkConfig);

                        sbNr = udaAddr / (NLMDEV_NUM_SRAM_BLOCKS_IN_SB * NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK); 
                        sbNr = sbNr - iterHandle->m_tbl_p->m_genericTblMgr_p->m_udaSBRange[0].m_stSBNum;
                            
                        blkNrInSB = udaAddr % NLMDEV_NUM_SRAM_BLOCKS_IN_SB;

                        rowNr = (udaAddr / NLMDEV_NUM_SRAM_BLOCKS_IN_SB) % NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK;

                        for(jump = 0;jump < adWidth/32;jump++)
                        {
                            for(i = 0;i< NLM_MIN_SRAM_WIDTH_IN_BYTES;i++)
                            {
                                iterInfo->AssoData[i+jump*4] = shadowUDA_p[sbNr].m_udaBlk[blkNrInSB + jump].m_entry[rowNr].m_data[i];
                            }
                        }
                    }
                    
                    /* if valid data found then break the loop */
                    found = NlmTrue;
                    
                }

                /* increment the offset */
                iterHandle->CurRecOffset += OFFSET_CHANGE(blkInfo_p,iterHandle->curBlock);
                
                if(found)
                    break;
            }
        }

        if(found)
            break;
    }
    
    return NLMERR_OK;
}

/************************************************
 * Function : NlmBlkMemMgr__DestroyTbl
 * 
 * Parameters:
 *  void*               self_p,
 *  NlmGenericTbl*      genericTbl_p,
 *  NlmReasonCode       *o_reason_p
 *
 * Summary:
 * This function is called when the table is destroyed. All entries
 * of this table are removed from all blocks and block info is 
 * updated for all affected blocks. Hash table is deleted for this
 * table. If a block is getting emptied, inform TM to update
 * Block configuration and settings. 
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__DestroyTbl(
    void*               self_p,
    NlmGenericTbl*      genericTbl_p,
    NlmReasonCode       *o_reason_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = NULL;
    nlm_32 curBlk = 0, curRecOffset = 0;
    NlmTblsInBlk_t *tblInBlk_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblPvtData_t *tblPvtData_p = (NlmTblPvtData_t*)genericTbl_p->m_privateData;
    

    /*To avoid compiler warnings*/
    (void) o_reason_p;

    if(blkMemMgr_p == NULL) 
        return NLMERR_NULL_PTR;

    blkInfo_p = blkMemMgr_p->m_blkInfo_p;

    /*Delete the hash table 
    The hash table will call delete for each groupId*/
    NlmCmTwoLevelHashTbl__Destroy((NlmCmTwoLevelHashTbl*)tblPvtData_p->m_groupIdHashTbl_p);

    NlmCmAllocator__free(blkMemMgr_p->m_alloc_p, tblPvtData_p);

    
    
    /*if there is an entry for the tbl in the blk
    remove the entry from the table list in the blk.
    Scan through for that blk each of the entries that 
    belong to the table and remove them*/
    for(curBlk = 0; curBlk < blkMemMgr_p->m_NumOfABs * blkMemMgr_p->m_nrOfDevices; ++curBlk)
    {
        if(!blkInfo_p[curBlk].m_validBlk)
            continue;
        if(NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p, curBlk,
                                            genericTbl_p->m_tblId,
                                            &tblInBlk_p))
        {
            NlmBool isBlkEmpty = NlmFalse;

            blkInfo_p[curBlk].m_numUsedRecs = blkInfo_p[curBlk].m_numUsedRecs - 
                                                    tblInBlk_p->m_numRecs;

            NlmBlkMemMgr__pvt_RemoveTblsInBlk(blkMemMgr_p->m_alloc_p,
                            tblInBlk_p);

            if(blkInfo_p[curBlk].m_numUsedRecs == 0)
            {
                isBlkEmpty = NlmTrue;
                blkInfo_p[curBlk].m_keepSeparate = 0;
            }

            /*inform that there are no more records of table in this blk */
            if(blkMemMgr_p->m_blkChangedForTblCB)
            {
                errNum  = blkMemMgr_p->m_blkChangedForTblCB(blkMemMgr_p->m_ctx_p,
                                                    genericTbl_p,
                                                    curBlk,
                                                    NlmFalse,
                                                    isBlkEmpty,
                                                    o_reason_p);
                if(errNum != NLMERR_OK)
                    return errNum;
            }

            for(curRecOffset=0; curRecOffset <= MAX_REC_OFFSET; 
                curRecOffset += OFFSET_CHANGE(blkInfo_p,curBlk))
            {
                if(blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId == 
                        genericTbl_p->m_tblId )
                {
                    blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId = 0xFF;

                    if(blkMemMgr_p->m_removeRecordCB)
                    {
                        errNum = blkMemMgr_p->m_removeRecordCB(
                                                    blkMemMgr_p->m_ctx_p,
                                                    genericTbl_p,
                                                    ConvertToIndex(curBlk,curRecOffset),
                                                    genericTbl_p->m_width,
                                                    o_reason_p);
                        if(errNum != NLMERR_OK)
                            return errNum;
                    }
                }
            }

            /* If block is empty then free the UDA chunk and update the table info in that SB */
            if((isBlkEmpty == NlmTrue) && (genericTbl_p->m_adWidth != NLM_TBL_ADLEN_ZERO))
            {
                errNum = NlmUdaMemMgr__UpdateUdaChunkInfo(
                        blkMemMgr_p->m_udaMemMgr_p, genericTbl_p->m_tblId, 
                        blkInfo_p[curBlk].m_udaChunkInfo_p, isBlkEmpty, o_reason_p);
            
                if(errNum != NLMERR_OK)
                    return errNum;  
                
            }
                        
        }
    }

    return errNum;
}



/************************************************
 * Function : NlmBlkMemMgr__Destroy
 * 
 * Parameters:
 *  void* self_p,
 *  NlmReasonCode *o_reason_p
 *
 * Summary:
 * Destroy the blk memory manager and free all the
 * memory hold by it.
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__Destroy(
    void* self_p,
    NlmReasonCode *o_reason_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = NULL;
    nlm_32 curBlk = 0;

    /*To avoid compiler warnings*/
    (void) o_reason_p;

    if(blkMemMgr_p == NULL) 
        return NLMERR_NULL_PTR;

    blkInfo_p = blkMemMgr_p->m_blkInfo_p;

    /*Free the memory allocated */
    for(curBlk = 0; curBlk < blkMemMgr_p->m_NumOfABs * blkMemMgr_p->m_nrOfDevices; ++curBlk)
    {
        if(!blkInfo_p[curBlk].m_validBlk)
            continue;
        NlmBlkMemMgr__pvt_DestroyTblsInBlk(blkMemMgr_p->m_alloc_p,
                            blkInfo_p[curBlk].m_tblList_p);

        NlmCmAllocator__free(blkMemMgr_p->m_alloc_p,
                    blkInfo_p[curBlk].m_recInfo_p);

        NlmFreeSlotMgr__Destroy(blkInfo_p[curBlk].m_freeSlotMgr_p);
    }

    NlmCmAllocator__free(blkMemMgr_p->m_alloc_p,
                    blkInfo_p);

    /* Destroy UDA Mem Mgr */   
    NlmUdaMemMgr__Destroy(blkMemMgr_p->m_udaMemMgr_p);

    NlmCmAllocator__free(blkMemMgr_p->m_alloc_p,
                        blkMemMgr_p);

    return NLMERR_OK;
}

#ifdef EXCLUDING_FOLLOWING_FOR_GCOV
/************************************************
 * Function : NlmBlkMemMgr__Print
 * 
 * Parameters:
 *  void *self_p
 *
 * Summary:
 * Debug function which prints all blkInfo for all the blocks.
 * This is been excluded form build.
 ************************************************/
void 
NlmBlkMemMgr__Print(
    void *self_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    nlm_32 curBlk = 0;
    nlm_32 curRecOffset = 0;
    FILE* fp = fopen("BlkMemMgrLayout.txt","w"); 
    nlm_32 prevRecOffset = 0;
    nlm_32 startRecOffset = 0;
    NlmTblsInBlk_t* tblInBlk_p = NULL;

    for(curBlk = 0; curBlk < blkMemMgr_p->m_blkInfo_p->m_BlksInSB * TOTAL_NUM_SB * 
                    blkMemMgr_p->m_nrOfDevices; ++curBlk)
    {
            NlmCmFile__fprintf(fp,"________________________________________\n");

            NlmCmFile__fprintf(fp, "Block number = %d\n", (int)curBlk);

            if(!blkInfo_p[curBlk].m_validBlk)
            {
                NlmCmFile__fprintf(fp, "Block Not Allocated For GTM\n", (int)curBlk);
                continue;
            }
            NlmCmFile__fprintf(fp,"Block width = %d\n", blkInfo_p[curBlk].m_blkWidth);

            NlmCmFile__fprintf(fp,"Number of used records = %d\n", blkInfo_p[curBlk].m_numUsedRecs);
            
            NlmCmFile__fprintf(fp, "Tables in block : ");

            tblInBlk_p = blkInfo_p[curBlk].m_tblList_p->m_next_p;
            
            while(tblInBlk_p != blkInfo_p[curBlk].m_tblList_p)
            {
                NlmCmFile__fprintf(fp, "%d ",tblInBlk_p->m_tblId);

                tblInBlk_p = tblInBlk_p->m_next_p;
            }
            
            NlmCmFile__fprintf(fp,"\nStart\tEnding\tPrio\tGroup\tTable\n");

            startRecOffset = 0;

            prevRecOffset = 0;

            curRecOffset = OFFSET_CHANGE(blkInfo_p,curBlk);

            while(curRecOffset <= MAX_REC_OFFSET)
            {
                if(memcmp(&blkInfo_p[curBlk].m_recInfo_p[curRecOffset],
                    &blkInfo_p[curBlk].m_recInfo_p[prevRecOffset],
                    sizeof(NlmRecInfo_t)))
                {
                    if(blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId != 255)
                    {
                        NlmCmFile__fprintf(fp,"%5d\t%6d\t%d\t%d\t%s\n",(int)startRecOffset, (int)prevRecOffset,
                            (int)blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_priority,
                            (int)blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_groupId,
                            blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId);
                    }

                    startRecOffset = curRecOffset;
                }
                prevRecOffset = curRecOffset;

                curRecOffset += OFFSET_CHANGE(blkInfo_p,curBlk);
            }
            
            if(blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId != 255)
            {
                NlmCmFile__fprintf(fp,"%5d\t%6d\t%d\t%d\t%s\n",(int)startRecOffset, (int)prevRecOffset,
                    (int)blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_priority,
                    (int)blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_groupId,
                    blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId);
            }

            NlmCmFile__fprintf(fp,"\n\n");
        
    }

    fclose(fp);

}


/************************************************
 * Function : NlmBlkMemMgr__VfyTblEntries
 * 
 * Parameters:
 *  void *self_p, 
 *  NlmGenericTbl* genericTbl_p
 *
 * Summary:
 * Debug function which checks if entries of the table 
 * are correctly stored. This is an internal verifying finction
 ***********************************************/
NlmBool 
NlmBlkMemMgr__VfyTblEntries(
    void *self_p, 
    NlmGenericTbl* genericTbl_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    nlm_32 curRecBlk = 0, curRecOffset = 0;
    NlmBool isRecOrderCorrect = NlmTrue, isTblDepCorrect = NlmTrue;
    NlmPsTblList *psTblListHead_p = NULL;
    NlmPsTblList *psTblListNode_p = NULL;
    nlm_u16 groupId = 0;
    nlm_u16 *prevPriorityForGroupId_p = NlmCmAllocator__calloc(
                                                    ((NlmBlkMemMgr_t*)self_p)->m_alloc_p,
                                                    1, 
                                                    sizeof(nlm_u16) * 64 * 1024);
    
    
    for(curRecBlk = 0; curRecBlk < blkMemMgr_p->m_blkInfo_p->m_BlksInSB * TOTAL_NUM_SB *
                        blkMemMgr_p->m_nrOfDevices; ++curRecBlk)
    {
        if(!blkInfo_p[curRecBlk].m_validBlk)
            continue;
        /*check if parallel search dependency is correct */
        if(NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p, curRecBlk, genericTbl_p->m_tblId, NULL))
        {
            psTblListHead_p = genericTbl_p->m_psTblList_p;

            psTblListNode_p = psTblListHead_p->m_next_p;

            while(psTblListNode_p != psTblListHead_p)
            {
                if(NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p, curRecBlk, 
                        psTblListNode_p->m_tblId, NULL))
                {
                    /* We should not find another table participating in  
                    parallel search in the same blk */
                    isTblDepCorrect = NlmFalse;
                    break;
                }
                psTblListNode_p = psTblListNode_p->m_next_p;
            }

            if(!isTblDepCorrect) break;

        }

        /*check if records are arranged according to priority */
        curRecOffset = 0;

        while(curRecOffset <= MAX_REC_OFFSET)
        {
            if(blkInfo_p[curRecBlk].m_recInfo_p[curRecOffset].m_tblId == 
                                                    genericTbl_p->m_tblId)
            {
                groupId = blkInfo_p[curRecBlk].m_recInfo_p[curRecOffset].m_groupId;

                if(prevPriorityForGroupId_p[groupId] > 
                    blkInfo_p[curRecBlk].m_recInfo_p[curRecOffset].m_priority)
                {
                    isRecOrderCorrect = NlmFalse;
                        break;
                }
                prevPriorityForGroupId_p[groupId] = 
                        blkInfo_p[curRecBlk].m_recInfo_p[curRecOffset].m_priority;
                
            }
    
            curRecOffset += OFFSET_CHANGE(blkInfo_p,curRecBlk);
        }
        
        if(!isRecOrderCorrect) break;
    }

     NlmCmAllocator__free( ((NlmBlkMemMgr_t*)self_p)->m_alloc_p,
                            prevPriorityForGroupId_p);

    return (isRecOrderCorrect && isTblDepCorrect);
}


/************************************************
 * Function : NlmBlkMemMgr__PrintNew
 * 
 * Parameters:
 *  void *self_p
 *
 * Summary:
 * Debug function which prints all blkInfo for all the blocks.
 * This is been excluded form build.
 ************************************************/
void 
NlmBlkMemMgr__PrintNew(
    void *self_p
    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    nlm_32 curBlk = 0;
    nlm_32 curRecOffset = 0;
    FILE* fp = fopen("BlkMemMgrLayout.txt","w"); 
    nlm_32 prevRecOffset = 0;
    nlm_32 startRecOffset = 0;
    NlmTblsInBlk_t* tblInBlk_p = NULL;

    bmmPrint++;

    for(curBlk = 0; curBlk < blkMemMgr_p->m_blkInfo_p->m_BlksInSB * TOTAL_NUM_SB * 
                    blkMemMgr_p->m_nrOfDevices; ++curBlk)
    {
            NlmCmFile__fprintf(fp,"________________________________________\n");

            NlmCmFile__fprintf(fp, "Block number = %d\n", (int)curBlk);

            if(!blkInfo_p[curBlk].m_validBlk)
            {
                NlmCmFile__fprintf(fp, "Block Not Allocated For GTM\n", (int)curBlk);
                continue;
            }
            NlmCmFile__fprintf(fp,"Block width = %d\n", blkInfo_p[curBlk].m_blkWidth);

            NlmCmFile__fprintf(fp,"Number of used records = %d\n", blkInfo_p[curBlk].m_numUsedRecs);
            
            NlmCmFile__fprintf(fp, "Tables in block : ");

            tblInBlk_p = blkInfo_p[curBlk].m_tblList_p->m_next_p;
            
            while(tblInBlk_p != blkInfo_p[curBlk].m_tblList_p)
            {
                NlmCmFile__fprintf(fp, "%d ",tblInBlk_p->m_tblId);

                tblInBlk_p = tblInBlk_p->m_next_p;
            }
            
            NlmCmFile__fprintf(fp,"\nStart\tEnding\tPrio\tGroup\tTable\n");

            startRecOffset = 0;
            prevRecOffset = 0;
            curRecOffset = 0;

            while(curRecOffset <=MAX_REC_OFFSET && blkInfo_p[curBlk].m_numUsedRecs)
            {
                /* print any way */
                NlmCmFile__fprintf(fp,"%5d\t%6d\t%d\t%d\t%d\n",(int)startRecOffset, (int)prevRecOffset,
                    (int)blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_priority,
                    (int)blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_groupId,
                    blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId);                  
                                
                prevRecOffset = curRecOffset;
                curRecOffset += OFFSET_CHANGE(blkInfo_p,curBlk);        
                startRecOffset = curRecOffset;
            }

            NlmCmFile__fprintf(fp,"\n\n");      
    }

    fclose(fp);

}
#endif


/* This function checks the special tables in all blocks, in which all entires must belong to same table
   returns error if any sinlge/more entries of the different table(s) is/are present */
NlmErrNum_t 
NlmBlkMemMgr__CheckSplBlkTbls(void *self_p)
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    nlm_32 curBlk = 0;
    nlm_32 curRecOffset = 0;
    nlm_32 prevRecOffset = 0;
    nlm_32 startRecOffset = 0;
    NlmTblsInBlk_t* tblInBlk_p = NULL;
    nlm_u8 tableId = 0;

    for(curBlk = 0; curBlk < blkMemMgr_p->m_blkInfo_p->m_BlksInSB * TOTAL_NUM_SB * 
                    blkMemMgr_p->m_nrOfDevices; ++curBlk)
    {
        tableId = 0;

        /* is valid block*/
        if(!blkInfo_p[curBlk].m_validBlk)
            continue;

        /* is special table */
        if(!(blkInfo_p[curBlk].m_keepSeparate))
            continue;

        /* check for special table, only one special table per block */
        {
            NlmTblsInBlk_t *cur_p =  (NlmTblsInBlk_t*)blkInfo_p[curBlk].m_tblList_p->m_next_p;
            nlm_u32 count = 0;
                
            while(cur_p != blkInfo_p[curBlk].m_tblList_p)
            {
                tableId = cur_p->m_tblId;
                count++;
                cur_p = (NlmTblsInBlk_t*)cur_p->m_next_p;
            }
                        
            if(count > 1)
            {
                NlmCm__printf("\n\t Error!!!!... Special Block has more sharing table entries...!!! \n\t Tables: ");
                tblInBlk_p = blkInfo_p[curBlk].m_tblList_p->m_next_p;
            
                while(tblInBlk_p != blkInfo_p[curBlk].m_tblList_p)
                {
                    NlmCm__printf("%d ",tblInBlk_p->m_tblId);
                    tblInBlk_p = tblInBlk_p->m_next_p;
                }
                NlmCm__printf("\n\n");

                return NLMERR_FAIL;
            }
        }

        startRecOffset = 0;
        prevRecOffset  = 0;
        curRecOffset   = 0;
        
        while(curRecOffset <=MAX_REC_OFFSET && blkInfo_p[curBlk].m_numUsedRecs)
        {
            /* assume only 1 table id is assigned */
            if( blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_priority != 0 &&
                blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_groupId != 0 &&
                blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId)
            {
                if((blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId != tableId) && 
                   (blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId != 255) )
                {
                    NlmCm__printf("\n\t Error!!!!... Special Table has other entries...!!!");
                    NlmCm__printf("\n\t Error!!!!... Special Table %d has other table %d entries...!!!\n\n",
                        blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId, tableId);
                    return NLMERR_FAIL;
                }
            }       
            prevRecOffset = curRecOffset;
            curRecOffset += OFFSET_CHANGE(blkInfo_p,curBlk);        
            startRecOffset = curRecOffset;
        }
    }       
    
    return NLMERR_OK;
}


/* This function checks all the blocks, where the non-special tables, must not contain any special
   table entires; returns error if any special table found in block which is not set to special block */
NlmErrNum_t 
NlmBlkMemMgr__CheckAnySplBlksMerged(void *self_p,
                                    nlm_u8 splTblId
                                    )
{
    NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p; 
    NlmBlkInfo_t* blkInfo_p = blkMemMgr_p->m_blkInfo_p;
    nlm_32 curBlk = 0;
    NlmTblsInBlk_t* tblInBlk_p = NULL;
    NlmBool flag = NlmFalse;
    nlm_32 curRecOffset = 0;
    nlm_u8 bitSetTable[256] = {0}; /* clear all */

    for(curBlk = 0; curBlk < blkMemMgr_p->m_blkInfo_p->m_BlksInSB * TOTAL_NUM_SB * 
                    blkMemMgr_p->m_nrOfDevices; ++curBlk)
    {
        NlmCm__memset(bitSetTable, 0, 256);

        /* is valid block*/
        if(!blkInfo_p[curBlk].m_validBlk)
            continue;

        /* if special table, only one table exists in the list */
        tblInBlk_p = (NlmTblsInBlk_t*)blkInfo_p[curBlk].m_tblList_p->m_next_p;

        /* is special table */
        if(blkInfo_p[curBlk].m_keepSeparate && (splTblId == tblInBlk_p->m_tblId))
            continue;
                    
        while(tblInBlk_p != blkInfo_p[curBlk].m_tblList_p)
        {
            if( splTblId == tblInBlk_p->m_tblId && ((splTblId != tblInBlk_p->m_tblId)))
            {
                NlmCm__printf("\n\t Error!!!!... This block has special table merged (blk:%d, splTblId: %d)...!!!: \n\n",
                    splTblId, tblInBlk_p->m_tblId);
                flag = NlmTrue;
            }

            if(tblInBlk_p->m_tblId >= 255)
            {
                NlmCm__printf("\n\t .... Table Id exceeding value 255 .... Array exhausting \n\n");
                return NLMERR_FAIL;
            }
            
            bitSetTable[tblInBlk_p->m_tblId] = 1; /* mark the non-special table as set */

            tblInBlk_p = (NlmTblsInBlk_t*)tblInBlk_p->m_next_p;
        }

        if(flag == NlmTrue)
            return NLMERR_FAIL;

        /* Verify the entries belongs to the tables which are there in the list (tableInBlkList),
           if other entry (tableId, other than 255: invalid table id) they through an error */
        
        curRecOffset   = 0;
        
        while(curRecOffset <=MAX_REC_OFFSET && blkInfo_p[curBlk].m_numUsedRecs)
        {
            /* if not invalid table */
            if( (blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId != 255) )
            {
                if(bitSetTable[(blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId)] != 1)
                {
                    NlmCm__printf("\n\t Error!!!!... Block contains other table entry (not in list) : Id: %d...!!!\n\n",
                        blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId);
                    return NLMERR_FAIL;
                }
            }       
            curRecOffset += OFFSET_CHANGE(blkInfo_p,curBlk);
        }

    }       
    
    return NLMERR_OK;
}



