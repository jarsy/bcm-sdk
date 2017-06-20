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
#include "nlmcmsimplegp.h"

#define TOTAL_NUM_SB					NLMDEV_NUM_SUPER_BLOCKS 

#define TOTAL_NUM_BLKS					NLMDEV_NUM_ARRAY_BLOCKS

#define MIN_BLK_WIDTH					NLMDEV_AB_WIDTH_IN_BITS

#define MAX_NUM_ENTRIES_PER_BLK			NLMDEV_AB_DEPTH

#define MAX_BLK_WIDTH					NLMDEV_MAX_AB_WIDTH_IN_BITS


#define NUM_BLKS_IN_SB					(TOTAL_NUM_BLKS / TOTAL_NUM_SB)

#define SIZE_OF_BLK						(MAX_NUM_ENTRIES_PER_BLK * MIN_BLK_WIDTH )

#define MAX_REC_OFFSET					(MAX_NUM_ENTRIES_PER_BLK - 1)

#define NLM_VALID_GTM_BLK                1

#define NLM_INVALID_GTM_BLK              0


#define INVALID_OFFSET					(-1)


#define GROUPID_HASH_TBL_SIZE					(64 * 1024)	

#define GROUPID_HASH_TBL_LEVEL_2_NR_OF_ELEMS	(16)




#define TOTAL_NUM_RECS_PER_BLK(blkInfo_p,curBlk) (SIZE_OF_BLK / blkInfo_p[curBlk].m_blkWidth)

#define OFFSET_CHANGE(blkInfo_p,curBlk)			(blkInfo_p[curBlk].m_blkWidth / MIN_BLK_WIDTH)

#define ConvertToIndex(curBlk,curRecOffset)		(curBlk * MAX_NUM_ENTRIES_PER_BLK + curRecOffset)

#define INDEX_TO_BLK_NUM(index)					(index / MAX_NUM_ENTRIES_PER_BLK)

#define INDEX_TO_OFFSET(index)					(index % MAX_NUM_ENTRIES_PER_BLK)



#define ISFULL(blkInfo_p,curBlk) (blkInfo_p[curBlk].m_numUsedRecs >= TOTAL_NUM_RECS_PER_BLK(blkInfo_p,curBlk))


#include "nlmcmexterncstart.h"


static void NlmBlkMemMgr__pvt_InitFnArgs(
			NlmBlkMemMgrFnArgs_t *args_p,
			NlmGenericTbl       *genericTbl_p,
			nlm_u16				groupId,
			nlm_u16             newRecPriority, 
			NlmRecordIndex    	*o_recIndex_p, 
			nlm_16				newRecWidth,
			NlmReasonCode		*o_reason_p,
			NlmBlkMemMgr_t	*blkMemMgr_p);

static NlmErrNum_t NlmBlkMemMgr__pvt_InitTblsInBlk(
				NlmBlkInfo_t* blkInfo_p,
				nlm_32 curBlk,
				NlmCmAllocator*	alloc_p,
				NlmReasonCode	*o_reason_p);

static NlmTblsInBlk_t* NlmBlkMemMgr__pvt_InsertTblsInBlk(
				NlmCmAllocator* alloc_p,
    			NlmTblsInBlk_t* tblListHead_p,
    			nlm_8* tblId);

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
				nlm_8* tblId,
				NlmTblsInBlk_t** tblInBlk_pp);

static NlmBool NlmBlkMemMgr__pvt_FindTblInSB(
				NlmBlkInfo_t* blkInfo_p,
				nlm_32 curSB,
				nlm_8* tblId);

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
 *	void* client_p,
 *	NlmRecordIndex oldIndex,
 *	NlmRecordIndex newIndex
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
											args_p->genericTbl_p->m_id_str_p;

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
 *	NlmBlkMemMgrFnArgs_t *args_p, 
 *	NlmGenericTbl       *genericTbl_p,
 *	nlm_u16				groupId,
 *	nlm_u16             newRecPriority, 
 *	NlmRecordIndex    	*o_recIndex_p, 
 *	nlm_16				newRecWidth,
 *	NlmReasonCode		*o_reason_p,
 *	NlmBlkMemMgr_t	*blkMemMgr_p
 *
 * Summary:
 * This function initializes the structure that holds
 * the parameters needed for an add record
 ***********************************************/
void 
NlmBlkMemMgr__pvt_InitFnArgs(
	NlmBlkMemMgrFnArgs_t *args_p, 
	NlmGenericTbl       *genericTbl_p,
	nlm_u16				groupId,
	nlm_u16             newRecPriority, 
	NlmRecordIndex    	*o_recIndex_p, 
	nlm_16				newRecWidth,
	NlmReasonCode		*o_reason_p,
	NlmBlkMemMgr_t	*blkMemMgr_p
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
 *	NlmBlkInfo_t* blkInfo_p,
 *	nlm_32 curBlk,
 *	NlmCmAllocator*	alloc_p,
 *	NlmReasonCode	*o_reason_p
 *
 * Summary:
 * This function initializes the linked list TblsInBlk
 * with the head node that does not contain any data 
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__pvt_InitTblsInBlk(
	NlmBlkInfo_t* blkInfo_p,
	nlm_32 curBlk,
	NlmCmAllocator*	alloc_p,
	NlmReasonCode	*o_reason_p
	)
{
	blkInfo_p[curBlk].m_tblList_p = NlmCmAllocator__calloc(alloc_p, 1,
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
 *	NlmCmAllocator* alloc_p,
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
    nlm_8* tblId
	)
{
	NlmTblsInBlk_t* node_p = NlmCmAllocator__malloc(alloc_p, sizeof(NlmTblsInBlk_t));

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
 *	NlmCmDblLinkList* node_p,  
 *	void* alloc_p
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
 *	NlmCmAllocator* alloc_p,
 *	NlmTblsInBlk_t* tblInBlk_p
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
 *	NlmCmAllocator* alloc_p, 
 *	NlmTblsInBlk_t* head_p
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
 *	NlmBlkInfo_t* blkInfo_p,
 *	nlm_32 curBlk,
 *	nlm_8* tblId,
 *	NlmTblsInBlk_t** tblInBlk_pp 
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
	nlm_8* tblId,
	NlmTblsInBlk_t** tblInBlk_pp /* out argument, can be null */
	)
{
	NlmTblsInBlk_t *tblInBlkHead_p = blkInfo_p[curBlk].m_tblList_p;
	NlmTblsInBlk_t *node_p = tblInBlkHead_p->m_next_p;
	NlmBool found = NlmFalse;
    
    while(node_p != tblInBlkHead_p && !found)
    {
        if(node_p->m_tblId == tblId)
        {
            found = NlmTrue;
            break;
        }
        node_p = node_p->m_next_p;
    }

    if(found && tblInBlk_pp != NULL)
        *tblInBlk_pp = node_p;

	return found;
}



/************************************************
 * Function : NlmBlkMemMgr__pvt_FindTblInSB
 * 
 * Parameters:
 *	NlmBlkInfo_t* blkInfo_p,
 *	nlm_32 curSB,
 *	nlm_8* tblId
 *
 * Summary:
 *
 * Returns true if a record of the table exists in the super blk 
 ***********************************************/
NlmBool 
NlmBlkMemMgr__pvt_FindTblInSB(
	NlmBlkInfo_t* blkInfo_p,
	nlm_32 curSB,
	nlm_8* tblId
	)
{
	NlmBool found = NlmFalse;
	nlm_32 curBlk = curSB * NUM_BLKS_IN_SB;

    if(blkInfo_p[curBlk].m_validBlk)
    {
	    while(!found && curBlk < (curSB+1)* NUM_BLKS_IN_SB)
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
 *	NlmBlkInfo_t*		blkInfo_p,
 *	nlm_32			curSB,
 *	NlmGenericTbl   *genericTbl_p
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
	NlmBlkInfo_t*		blkInfo_p,
	nlm_32			curSB,
	NlmGenericTbl   *genericTbl_p
	)
{
	NlmBool tblCanEnterBlkInNewSB = NlmTrue;
	nlm_32 curBlk = curSB * NUM_BLKS_IN_SB;
	NlmTblsInBlk_t *tblInBlkHead_p = NULL;
	NlmTblsInBlk_t *tblNode_p = NULL;
	NlmPsTblList *psTblListHead_p = NULL;	
	NlmPsTblList *psTblListNode_p = NULL;
	
    if(!blkInfo_p[curBlk].m_validBlk)
        return NlmFalse;
    
	/*Iterate through each of the blks in the super blk */
	while(curBlk < (curSB+1)* NUM_BLKS_IN_SB)
	{
		tblInBlkHead_p = blkInfo_p[curBlk].m_tblList_p;
		tblNode_p = tblInBlkHead_p->m_next_p;
		
		/*Iterate through each of the tables whose records are in the blk */
		while(tblNode_p != tblInBlkHead_p)
		{
			psTblListHead_p = genericTbl_p->m_psTblList_p;

			if(psTblListHead_p == NULL)
				break;

			psTblListNode_p = psTblListHead_p->m_next_p;

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
				psTblListNode_p = psTblListNode_p->m_next_p;
			}
			if(!tblCanEnterBlkInNewSB) break;

			tblNode_p = tblNode_p->m_next_p;
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
 *	NlmCmAllocator* alloc_p,	
 *	void* element_p
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
 *	NlmBlkMemMgrFnArgs_t *args_p,
 *	NlmRecordIndex leftFreeSlot,
 *	NlmRecordIndex rightFreeSlot,
 *	void* groupIdHead_p,
 *	Nlm__Marker priorityMarker,
 *	NlmRecordIndex *o_bestFreeSlotIndex_p,
 *	NlmBool *o_isForwardShift_p,
 *	Nlm__Marker* o_shiftStartMarker_p
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
 *	NlmBlkMemMgrFnArgs_t *args_p,
 *	void *groupIdHead_p,
 *	NlmRecordIndex markerIndex,
 *	Nlm__Marker priorityMarker,
 *	NlmRecordIndex *o_bestFreeSlotIndex_p,
 *	NlmBool *o_isForwardShift_p,
 *	Nlm__Marker* o_shiftStartMarker_p 
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
									    args_p->genericTbl_p->m_id_str_p,
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

	while(curForwardBlk < TOTAL_NUM_BLKS * blkMemMgr_p->m_nrOfDevices)
	{
        if(blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_validBlk)
        {
		    isTblInBlk = NlmBlkMemMgr__pvt_FindTblInBlk(blkMemMgr_p->m_blkInfo_p,
								    curForwardBlk,
								    args_p->genericTbl_p->m_id_str_p,
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
 *	NlmBlkMemMgrFnArgs_t* args_p,
 *	nlm_32 curBlk
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
 *	NlmBlkMemMgrFnArgs_t* args_p,
 *	void * groupIdHead_p,
 *	NlmRecordIndex markerIndex,
 *	Nlm__Marker priorityMarker,
 *	NlmBool lookInOldSB,
 *	NlmRecordIndex *o_bestFreeSlotIndex_p,
 *	NlmBool *o_isForwardShift_p,
 *	Nlm__Marker* o_shiftStartMarker_p
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
												curBackBlk / NUM_BLKS_IN_SB,
												args_p->genericTbl_p->m_id_str_p);

		/* Table is in SuperBlock and we are looking for an already assigned super block OR
		Table is not in Super Block and we are looking for a new super block and table can
		enter the new super block*/
		if((isTblInSB && lookInOldSB) ||
			(!isTblInSB && !lookInOldSB && 
			NlmBlkMemMgr__pvt_CanTblEnterBlkInNewSB(blkMemMgr_p->m_blkInfo_p,
													curBackBlk / NUM_BLKS_IN_SB,
													args_p->genericTbl_p)))
		{
			curSB = curBackBlk / NUM_BLKS_IN_SB;
			
			/* Process the blocks in the super block */
			while(curBackBlk / NUM_BLKS_IN_SB  == curSB && curBackBlk >= 0)
			{
				if(!NlmBlkMemMgr__pvt_FindTblInBlk(blkMemMgr_p->m_blkInfo_p,
											curBackBlk,
											args_p->genericTbl_p->m_id_str_p,
											NULL))
				{
					/*if the block is completely empty then reinitialize the
					free slot manager of the block to the correct block width */
					if(blkMemMgr_p->m_blkInfo_p[curBackBlk].m_numUsedRecs == 0)
						NlmBlkMemMgr__pvt_ReInitFreeSlotMgr(args_p,
												curBackBlk);
					
					if(!ISFULL(blkMemMgr_p->m_blkInfo_p, curBackBlk) && 
						blkMemMgr_p->m_blkInfo_p[curBackBlk].m_blkWidth == args_p->newRecWidth)
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

			if(foundFreeSlot)
				break;
		}
		else
		{
			/*Proceed to the previous super block */
			curBackBlk = (curBackBlk / NUM_BLKS_IN_SB) * NUM_BLKS_IN_SB - 1;
		}
	}



	while(curForwardBlk < TOTAL_NUM_BLKS * blkMemMgr_p->m_nrOfDevices)
	{
		/* Find if table is present in the super block */
		isTblInSB = NlmBlkMemMgr__pvt_FindTblInSB(blkMemMgr_p->m_blkInfo_p,
													curForwardBlk / NUM_BLKS_IN_SB,
													args_p->genericTbl_p->m_id_str_p);

		/* Table is in SuperBlock and we are looking for an already assigned super block OR
		Table is not in Super Block and we are looking for a new super block and table can
		enter the new super block*/
		if((isTblInSB && lookInOldSB) ||
			(!isTblInSB && !lookInOldSB && 
			  NlmBlkMemMgr__pvt_CanTblEnterBlkInNewSB(blkMemMgr_p->m_blkInfo_p,
														curForwardBlk / NUM_BLKS_IN_SB,
														args_p->genericTbl_p)))
		{
			curSB = curForwardBlk / NUM_BLKS_IN_SB;

			/* Process the blocks in the super block */
			while(curForwardBlk / NUM_BLKS_IN_SB == curSB )
			{
				if(!NlmBlkMemMgr__pvt_FindTblInBlk(blkMemMgr_p->m_blkInfo_p,
										curForwardBlk,
										args_p->genericTbl_p->m_id_str_p,
										NULL))
				{
					/*if the block is completely empty then reinitialize the
					free slot manager of the block to the correct block width */
					if(blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_numUsedRecs == 0)
						NlmBlkMemMgr__pvt_ReInitFreeSlotMgr(args_p,
												curForwardBlk);
					
					if(!ISFULL(blkMemMgr_p->m_blkInfo_p, curForwardBlk) && 
						blkMemMgr_p->m_blkInfo_p[curForwardBlk].m_blkWidth == args_p->newRecWidth)
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

			if(foundFreeSlot)
				break;
		}
		else
		{
			/*Proceed to the next super block */
			curForwardBlk = (curForwardBlk / NUM_BLKS_IN_SB)* NUM_BLKS_IN_SB + NUM_BLKS_IN_SB;
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
 *	NlmBlkMemMgrFnArgs_t* args_p,
 *	void * groupIdHead_p,
 *	NlmRecordIndex markerIndex,
 *	Nlm__Marker priorityMarker,
 *	NlmRecordIndex *o_bestFreeSlotIndex_p,
 *	NlmBool *o_isForwardshift_p,
 *	Nlm__Marker* o_shiftStartMarker_p
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
 *	NlmBlkMemMgrFnArgs_t* args_p
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
	NlmTblPvtData_t *tblPvtData_p = args_p->genericTbl_p->m_privateData;
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
										tblPvtData_p->m_groupIdHashTbl_p,
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
		errNum = NlmCmTwoLevelHashTbl__Insert(tblPvtData_p->m_groupIdHashTbl_p, 
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

	/*Remove the best free slot from the free slot manager*/
	errNum = NlmFreeSlotMgr__RemoveFreeSlot(
					blkInfo_p[bestFreeSlotBlk].m_freeSlotMgr_p,
					bestFreeSlotIndex,
					args_p->o_reason_p);

	if(errNum != NLMERR_OK)
		return errNum;

	/*Mark the free slot as occupied*/
	errNum = NlmBlkMemMgr__pvt_MarkFreeSlotAsOccupied(args_p,
									bestFreeSlotIndex);
	if(errNum != NLMERR_OK)
		return errNum;

	/*Use the free slot as the starting position for the shifts.
	Start performing the shifts in the direction specified until
	the free slot is moved to the correct location where the 
	current record to be added can be inserted. Insert the 
	current record at the free slot*/
	errNum = NlmGP__InsertRecord(	groupIdHead_p,
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
 *	NlmBlkMemMgrFnArgs_t* args_p,
 *	NlmRecordIndex freeRecIndex
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
											args_p->o_reason_p);
			if(errNum != NLMERR_OK)
				return errNum;
		}
	}
	++blkInfo_p[curBlk].m_numUsedRecs;


	/*If this is the first record of the tbl in the blk */
	if(!NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p, curBlk,
					args_p->genericTbl_p->m_id_str_p,
					&tblInBlk_p))
	{
		/*Add the tbl to the linked list in the blk */
		tblInBlk_p = NlmBlkMemMgr__pvt_InsertTblsInBlk(blkMemMgr_p->m_alloc_p,
    					blkInfo_p[curBlk].m_tblList_p,
    					args_p->genericTbl_p->m_id_str_p);

		if(!tblInBlk_p)
		{ 
			if(args_p->o_reason_p) 
				*args_p->o_reason_p = NLMRSC_LOW_MEMORY; 
			return NLMERR_FAIL;
		}
		
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
 *	NlmCmAllocator			*alloc_p,
 *	NlmIndexChangedTblMgrCB	indexChangedCB,
 *	NlmSetBlkWidthCB		setblkWidthCB,
 *	NlmBlkChangedForTblCB	blkChangedForTblCB,
 *	NlmRemoveRecordCB       removeRecordCB,
 *	nlm_u8					nrOfDevices,
 *	void                     *ctx_p,
 *	NlmReasonCode			*o_reason_p
 *
 * Summary:
 * Initializes the blk memory manager. Allocates the 
 * memory for block memory manager and block info. 
 * Initializes the block info for 64 tables. Initializes
 * call back functions.
 *
 ***********************************************/
void* NlmBlkMemMgr__Init(
	NlmCmAllocator			*alloc_p,
	NlmIndexChangedTblMgrCB	indexChangedCB,
	NlmSetBlkWidthCB		setblkWidthCB,
	NlmBlkChangedForTblCB	blkChangedForTblCB,
	NlmRemoveRecordCB       removeRecordCB,
	nlm_u8					nrOfDevices,
    NlmGenericTblMgrBlksRange      *gtmBlksRange,
	void                     *ctx_p,
	NlmReasonCode			*o_reason_p
	)
{
		nlm_32 curBlk = 0;
		NlmErrNum_t status = NLMERR_OK;
        nlm_u8 devBlkNum;
        nlm_u8 devNum;
		
		/* Allocate the memory required memory */
		NlmBlkMemMgr_t* blkMemMgr_p = NlmCmAllocator__calloc(alloc_p, 1,
												sizeof(NlmBlkMemMgr_t));

		if(!blkMemMgr_p)
		{
			if(o_reason_p) *o_reason_p = NLMRSC_LOW_MEMORY;
			return NULL;
		}

		blkMemMgr_p->m_alloc_p = alloc_p;

		blkMemMgr_p->m_blkInfo_p = NlmCmAllocator__calloc(alloc_p, 1,
									sizeof(NlmBlkInfo_t) * TOTAL_NUM_SB * NUM_BLKS_IN_SB * nrOfDevices);

		if(!blkMemMgr_p->m_blkInfo_p)
		{
			if(o_reason_p) *o_reason_p = NLMRSC_LOW_MEMORY;
			return NULL;
		}

		for(curBlk = 0;curBlk < NLMDEV_NUM_ARRAY_BLOCKS * nrOfDevices; ++curBlk)
		{            
		    devNum = (nlm_u8)(curBlk/NLMDEV_NUM_ARRAY_BLOCKS);
            devBlkNum = (nlm_u8)(curBlk%NLMDEV_NUM_ARRAY_BLOCKS);

            if((gtmBlksRange[devNum].m_startBlkNum <= devBlkNum) && 
                (devBlkNum <= gtmBlksRange[devNum].m_endBlkNum))
		    {
                blkMemMgr_p->m_blkInfo_p[curBlk].m_validBlk = NLM_VALID_GTM_BLK;
			    blkMemMgr_p->m_blkInfo_p[curBlk].m_blkWidth = MIN_BLK_WIDTH;
			    blkMemMgr_p->m_blkInfo_p[curBlk].m_numUsedRecs = 0;
			    blkMemMgr_p->m_blkInfo_p[curBlk].m_recInfo_p = NlmCmAllocator__calloc(alloc_p, 1,
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

		return blkMemMgr_p;

}


/************************************************
 * Function : NlmBlkMemMgr__InitTbl
 * 
 * Parameters:
 *	void*			self_p,			
 *	NlmGenericTbl		*genericTbl_p,
 *	NlmReasonCode		*o_reason_p
 *
 * Summary:
 * Initialize the data structures for the table. 
 * Initialize the hash table that hashes the groupId and is maintained 
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__InitTbl(
	void*			self_p,			
	NlmGenericTbl		*genericTbl_p,
	NlmReasonCode		*o_reason_p
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
 *	void*			self_p,			
 *	NlmGenericTbl   *genericTbl_p,	
 *	nlm_u16         newRecPriority, 
 *	nlm_u16			groupId,
 *	NlmRecordIndex  *o_recIndex_p, 
 *	nlm_16			newRecWidth,
 *	NlmReasonCode	*o_reason_p
 *
 * Summary:
 * Locate the free index, shift the records according to priority and
 * insert the record in the correct location based on priority 
 *
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__AddRecord(
	void*			self_p,			
	NlmGenericTbl   *genericTbl_p,	
	nlm_u16         newRecPriority, 
	nlm_u16			groupId,
	NlmRecordIndex  *o_recIndex_p, 
	nlm_16			newRecWidth,
	NlmBool			*isShuffleDown_p,
	NlmReasonCode	*o_reason_p
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

	/* smaller numbers are treated as higher priorities	*/
	
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
 *	void*				self_p,			
 *	NlmGenericTbl       *genericTbl_p,	
 *	NlmRecordIndex      recIndex,	
 *	NlmReasonCode		*o_reason_p
 *
 * Summary:
 * Delete a record of genericTbl_p from recIndex. It
 * fetches the block number and relative offset within
 * the block to delete the record. After deleting it updates
 * the blockInfo.
 ***********************************************/
NlmErrNum_t 
NlmBlkMemMgr__DeleteRecord(
	void*				self_p,			
	NlmGenericTbl       *genericTbl_p,	
	NlmRecordIndex      recIndex,	
	NlmReasonCode		*o_reason_p
	)
{
	NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p;	
	NlmBlkInfo_t* blkInfo_p = NULL;
	nlm_32 curBlk = 0, curRecOffset = 0;
	nlm_u16 recPriority = 0;
	NlmBool found = NlmFalse;
	NlmTblsInBlk_t* tblInBlk_p = NULL;
	NlmErrNum_t errNum = NLMERR_OK;
	NlmTblPvtData_t *tblPvtData_p = genericTbl_p->m_privateData;
	void *groupIdHead_p = NULL;
	nlm_u16 groupId = 0;

	if(blkMemMgr_p == NULL) 
		return NLMERR_NULL_PTR;

	blkInfo_p = blkMemMgr_p->m_blkInfo_p;
	
	/*The index passed by the upper layer is outside the range of the 
	chip. So return an error */
	if(recIndex >= (MAX_REC_OFFSET + 1) * TOTAL_NUM_SB * NUM_BLKS_IN_SB * 
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
	if(blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId != genericTbl_p->m_id_str_p)
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

	blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId = NULL;


	tblInBlk_p = blkInfo_p[curBlk].m_tblList_p->m_next_p;

	found = NlmFalse;

	/*Iterate through the linked list of tables maintained for the blk */
	while(!found && tblInBlk_p != blkInfo_p[curBlk].m_tblList_p)
	{
		if(tblInBlk_p->m_tblId == genericTbl_p->m_id_str_p)
		{
			found = NlmTrue;
			break;
		}
		tblInBlk_p = tblInBlk_p->m_next_p;
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
			isBlkEmpty = NlmTrue;

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
										tblPvtData_p->m_groupIdHashTbl_p,
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
 *	void*				self_p,			
 *	NlmGenericTbl       *genericTbl_p,	
 *	NlmRecordIndex      recIndex,	
 *	NlmReasonCode		*o_reason_p
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
	void*				self_p,			
	NlmGenericTbl       *genericTbl_p,	
	NlmRecordIndex      recIndex,
	NlmReasonCode		*o_reason_p
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
	if(recIndex >= (MAX_REC_OFFSET + 1) * TOTAL_NUM_SB * NUM_BLKS_IN_SB * 
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
	if(blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId != genericTbl_p->m_id_str_p)
	{
		if(o_reason_p) *o_reason_p = NLMRSC_RECORD_NOTFOUND;
		return NLMERR_FAIL;
	}

	return NLMERR_OK;
}



/************************************************
 * Function : NlmBlkMemMgr__DestroyTbl
 * 
 * Parameters:
 *	void*				self_p,
 *	NlmGenericTbl*		genericTbl_p,
 *	NlmReasonCode		*o_reason_p
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
	void*				self_p,
	NlmGenericTbl*		genericTbl_p,
	NlmReasonCode		*o_reason_p
	)
{
	NlmBlkMemMgr_t* blkMemMgr_p = (NlmBlkMemMgr_t*) self_p;	
	NlmBlkInfo_t* blkInfo_p = NULL;
	nlm_32 curBlk = 0, curRecOffset = 0;
	NlmTblsInBlk_t *tblInBlk_p = NULL;
	NlmErrNum_t errNum = NLMERR_OK;
	NlmTblPvtData_t *tblPvtData_p = genericTbl_p->m_privateData;
	

	/*To avoid compiler warnings*/
	(void) o_reason_p;

	if(blkMemMgr_p == NULL) 
		return NLMERR_NULL_PTR;

	blkInfo_p = blkMemMgr_p->m_blkInfo_p;

	/*Delete the hash table 
	The hash table will call delete for each groupId*/
	NlmCmTwoLevelHashTbl__Destroy(tblPvtData_p->m_groupIdHashTbl_p);

	NlmCmAllocator__free(blkMemMgr_p->m_alloc_p, tblPvtData_p);

	
	
	/*if there is an entry for the tbl in the blk
	remove the entry from the table list in the blk.
	Scan through for that blk each of the entries that 
	belong to the table and remove them*/
	for(curBlk = 0; curBlk < TOTAL_NUM_BLKS * blkMemMgr_p->m_nrOfDevices; ++curBlk)
	{
        if(!blkInfo_p[curBlk].m_validBlk)
            continue;
		if(NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p, curBlk,
											genericTbl_p->m_id_str_p,
											&tblInBlk_p))
		{
			NlmBool isBlkEmpty = NlmFalse;

			blkInfo_p[curBlk].m_numUsedRecs = blkInfo_p[curBlk].m_numUsedRecs - 
													tblInBlk_p->m_numRecs;

			NlmBlkMemMgr__pvt_RemoveTblsInBlk(blkMemMgr_p->m_alloc_p,
							tblInBlk_p);

			if(blkInfo_p[curBlk].m_numUsedRecs == 0)
				isBlkEmpty = NlmTrue;

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
						genericTbl_p->m_id_str_p )
				{
					blkInfo_p[curBlk].m_recInfo_p[curRecOffset].m_tblId = NULL;

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
		}
	}

	return errNum;
}



/************************************************
 * Function : NlmBlkMemMgr__Destroy
 * 
 * Parameters:
 *	void* self_p,
 *	NlmReasonCode *o_reason_p
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
	for(curBlk = 0; curBlk < TOTAL_NUM_BLKS * blkMemMgr_p->m_nrOfDevices; ++curBlk)
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

	NlmCmAllocator__free(blkMemMgr_p->m_alloc_p,
						blkMemMgr_p);

	return NLMERR_OK;
}

#ifdef EXCLUDING_FOLLOWING_FOR_GCOV
/************************************************
 * Function : NlmBlkMemMgr__Print
 * 
 * Parameters:
 *	void *self_p
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

	for(curBlk = 0; curBlk < NUM_BLKS_IN_SB * TOTAL_NUM_SB * 
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
				NlmCmFile__fprintf(fp, "%s ",tblInBlk_p->m_tblId);

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
					if(blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId)
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
			
			if(blkInfo_p[curBlk].m_recInfo_p[prevRecOffset].m_tblId)
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
 *	void *self_p, 
 *	NlmGenericTbl* genericTbl_p
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
	
	
	for(curRecBlk = 0; curRecBlk < NUM_BLKS_IN_SB * TOTAL_NUM_SB *
						blkMemMgr_p->m_nrOfDevices; ++curRecBlk)
	{
        if(!blkInfo_p[curRecBlk].m_validBlk)
            continue;
		/*check if parallel search dependency is correct */
		if(NlmBlkMemMgr__pvt_FindTblInBlk(blkInfo_p, curRecBlk, genericTbl_p->m_id_str_p, NULL))
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
													genericTbl_p->m_id_str_p)
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

#endif

