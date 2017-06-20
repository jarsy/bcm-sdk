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
 * $Id: nlmtblmgr.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2007 Broadcom Corp.
 * All Rights Reserved.$
 */

 /*******************************************************************************/
/*
File Description:
This file creates second level layer Table Manager and integrate it with
GTM module. Following are functions defined in this file.

1.  NlmTblMgr__pvt_ctor
2.  NlmTblMgr__pvt_dtor
3.  NlmTblMgr__ConfigSearch
4.  NlmTblMgr__pvt_ConfigRangeAttributes
5.  NlmTblMgr__LockConfiguration
6.  NlmTblMgr__AddRecord
7.  NlmTblMgr__UpdateRecord
8.  NlmTblMgr__DeleteRecord
9.  NlmTblMgr__Destroy
10. NlmTblMgr__CreateTable
11. NlmTblMgr__DestroyTable
12. NlmTblMgr__Init
13. NlmSearchAttributeNode__Find
14. NlmSearchAttributeNode__Add
15. NlmSearchAttributeNode__Modify
17. NlmSearchAttributeNode__Delete
18. NlmTblMgr__pvt_WriteKcr
19. NlmTblMgr__pvt_WriteRecord
20. NlmTblMgr__pvt_EraseRecord
21. NlmTblMgr__pvt_WriteBCR
22. NlmTblMgr__pvt_WriteLtrBS
23. NlmTblMgr__pvt_WriteLtrPSAndKPUSelect
24. NlmTblMgr__pvt_ShiftRecord
25. NlmTblMgr__pvt_AssignBlockWidth
26. NlmTblMgr__pvt_ModifyBlockAttrib



Please see the function declarations/definitions for details of the above
functions.

Methodologies and Algorithms:
A linklist of search attributes (NlmSearchAttributesNode)is maintained by TM.
For every table participating in search, there is one such node.
After locking the configuration of Table Manager, all search attribute - key_Maps
and related key_Num is known to TM and it can not be changed. Based on this
search attribute TM decides the content of key construction register and calls
"writeKCR" while "LockConfig". As the table records are being written, TM decides
the cotents of Block_Config_Register, Block_Select_Register and 
Parallel_Search-Register and accrodingly calls appropriate functions to write these
registers.
*/
/*******************************************************************************/

#include "nlmarch.h"
#include "nlmgenerictblmgr.h"
#include "nlmblkmemmgr.h"
#include "nlmxktblmgr.h"
#include "nlmdevmgr.h"
#include "nlmcmstring.h"
#include "nlmdevmgr_shadow.h"

/* some utility definitions used locally */
#define NLMNOTFOUND					(-1)
#define NLM_INVALID_BLOCKWIDTH		(641)

/*register/database write functions which interact with Device Manager */
#define NLM_VALIDBIT				(1)
#define NLM_INVALIDBIT				(0)
#define NLM_ALLINDEX_PARDEVICE		(NLMDEV_NUM_ARRAY_BLOCKS*NLMDEV_AB_DEPTH)

/* 
A table can be configured for more than one ltr numbers. So this enum will
set flags for all 32 LTRs for a table.
*/
typedef enum NlmIsLtrConfigured
{
    NLM_LTR_NOTCONFIGURED =0,
    NLM_LTR_CONFIGURED

}NlmIsLtrConfigured;


/* 
Structure defines search_attributes_list which contains list of all search 
attributes for all tables.
This list is contained in Table Manager and sorted according to table ID.
One instance of the structure contains search attributes of one table only.
It contains table_ID, configuration flag for all 32 LTRs, out result_bus for
all LTRs, key number to be searched for all 32 LTRs and key_map bits for all
32 LTRs.
*/
typedef struct NlmSearchAttributesNode_s
{
    nlm_8*                          m_tblId_p;
    NlmIsLtrConfigured				m_isLtrConfigured[NLMDEV_NUM_LTR_SET];
    NlmDevParallelSrchNum           m_ps_RBus[NLMDEV_NUM_LTR_SET];
    NlmDevKeyNum                    m_keyNum[NLMDEV_NUM_LTR_SET];
    NlmGenericTblKeyConstructionMap m_kcm[NLMDEV_NUM_LTR_SET];

    struct NlmSearchAttributesNode_s* m_Next_p;
    
}NlmSearchAttributesNode;

/* 
Table manager structure specific to Processor devices. It contains Device Manager
pointer to drive the read and writes into device database, NlmIndexChangedAppCb
call back function to inform client about existing record index change, search
attribute list for all tables, table count of existing tables, block_memory_mnager
pointer, and client pointer to be used by client.
*/
typedef struct NlmTblMgr_s
{
    NlmDevMgr					*m_devMgr_p; /* internally has to be cast to the
                                                     correct Device Manager in the table
                                                     manager for the device  */
    NlmCmAllocator              *m_alloc_p; 
    NlmIndexChangedAppCb        m_indexChangedAppCb;
    NlmSearchAttributesNode		*m_searchAttrList;  /* sorted in the
                                        ascending order of table_id. Last
                                        node is NULL */
    nlm_u16                     m_tableCount;
    void                        *m_blockMemMgr;
    void						*m_client_p;
    
} NlmTblMgr;


/* private function declarations - specific to Table Manager */

/* 
constructor of TM creates and initializes the block memory manager.
It assigns the memory allocator, Device Manager pointer and NlmIndexChangedAppCb
function pointer to its member variables. 
*/
static NlmTblMgr* 
NlmTblMgr__pvt_ctor(
    NlmTblMgr        *self,
    NlmCmAllocator  *alloc_p,
    NlmDevMgr     *devMgr_p,
    NlmGenericTblMgrBlksRange *gtmBlksRange,
    NlmIndexChangedAppCb    m_indexChangedAppCb
    );

/* destructure for Table Manager */
static void 
NlmTblMgr__pvt_dtor(
    NlmTblMgr *self
    );

/* for shifting a record from current address to a new addres */
NlmErrNum_t 
NlmTblMgr__pvt_ShiftRecord(
    void* self,
    NlmGenericTbl* genericTbl_p,
    NlmRecordIndex address,
    NlmRecordIndex newAddress,
    nlm_u16 recordLen,
    NlmReasonCode *o_reason
    );

/* assigns the block width of an array block */
NlmErrNum_t 
NlmTblMgr__pvt_AssignBlockWidth(
    void* self,
    nlm_32 blockNum,
    nlm_u16 width,
    NlmReasonCode *o_reason
    );

/* modify/assign block attributes (key maping, RBUS mapping) according to tableID */
NlmErrNum_t 
NlmTblMgr__pvt_ModifyBlockAttrib(
    void* self,
    NlmGenericTbl* gt_p,
    nlm_32  blockNum,
    NlmBool addBlock,
    NlmBool isBlkEmpty, 
    NlmReasonCode *o_reason
    );

NlmErrNum_t 
NlmTblMgr__pvt_RemoveRecordCB(
    void* self_p,
    NlmGenericTbl   *genericTbl_p,
    NlmRecordIndex  recIndex, 
    nlm_u16         recWidth, 
    NlmReasonCode   *o_reason_p);


/* to find the index in the link list of search_attribute_node */
NlmSearchAttributesNode*
NlmSearchAttributeNode__Find(
    NlmSearchAttributesNode* SearchAttrHead_p,
    nlm_8* TableId
    );


/* to add a new search_attribute_node in the link list */
NlmBool 
NlmSearchAttributeNode__Add(
    NlmCmAllocator* alloc_p,
    NlmSearchAttributesNode** SearchAttrHead_pp,
    const nlm_8* tbl_ID,
    nlm_u8 LtrNum,
    NlmDevParallelSrchNum psNum,
    NlmDevKeyNum keyNum,
    NlmGenericTblKeyConstructionMap* kcm_p
    );
    

/* to modify the search_attribute_node contents */
NlmBool 
NlmSearchAttributeNode__Modify(
    NlmSearchAttributesNode* SearchAttrList_p,
    nlm_u8 LtrNum,
    nlm_u8 ps_RBus,
    nlm_u8 keyNum,
    NlmGenericTblKeyConstructionMap* kcm_p
    );

/* to delete a search_attribute_node from the link list */
NlmBool 
NlmSearchAttributeNode__Delete(
        NlmCmAllocator* alloc_p,
        NlmSearchAttributesNode** SearchAttrList_pp,
        const nlm_8* tbl_ID
        );

/* for writing key_construction_register */
NlmErrNum_t 
NlmTblMgr__pvt_WriteKcr(
        NlmTblMgr* self,
        NlmReasonCode *o_reason
        );

/* to write entry into device database */
NlmErrNum_t 
NlmTblMgr__pvt_WriteRecord(
        NlmDevMgr* devMgr_p,
        NlmGenericTblRecord *tblRecord,
        NlmRecordIndex RecordIndex,
        NlmBool convertToXY,
        NlmReasonCode *o_reason
        );

/* to delete a database record */
NlmErrNum_t NlmTblMgr__pvt_EraseRecord(
        NlmDevMgr* devMgr_p,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        );

/* to invalidate a database record */
NlmErrNum_t NlmTblMgr__pvt_InvalidateRecord(
        NlmDevMgr* devMgr_p,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        );

/* to write block_configuration_register */
NlmErrNum_t 
NlmTblMgr__pvt_WriteBCR(
        NlmDevMgr* devMgr_p,
        nlm_32 blockNum,
        nlm_u16 BlkWidth,
        NlmDevDisableEnable Disbl_Enbl,
        NlmReasonCode *o_reason
        );

/* writing ltr_block_select register */
NlmErrNum_t 
NlmTblMgr__pvt_WriteLtrBS(
        NlmDev* dev_p,
        nlm_u8 ltrNum,
        nlm_u8 ab_num,
        NlmDevDisableEnable enable_disable,
        NlmReasonCode *o_reason
        );

/* writing ltr_parallel_search register and super block to key mapping*/
NlmErrNum_t 
NlmTblMgr__pvt_WriteLtrPSAndKPUSelect(
        NlmDev* dev_p,
        nlm_u8 ltrNum,
        nlm_u8 ab_num,
        NlmDevParallelSrchNum rBus,
        NlmDevKeyNum key,
        NlmReasonCode *o_reason
        );


NlmGenericTblList* 
NlmGenericTblList__Search( NlmGenericTblList* head,
						nlm_8* tblId_p );

static void NlmTblMgr__pvt_convertToXYMode(
    NlmDevABEntry *abEntry
	)
{
	nlm_u32 byteNum = 0;
	NlmDevABEntry tempData;

	NlmCm__memset(&tempData, 0, sizeof(NlmDevABEntry));

    /* If Write mode is XY; then convert the data to XY mode */
	for(byteNum = 0; byteNum < NLMDEV_AB_WIDTH_IN_BYTES; byteNum++)
	{
		tempData.m_data[byteNum] = abEntry->m_data[byteNum] & (~abEntry->m_mask[byteNum]);
		tempData.m_mask[byteNum] = (~abEntry->m_data[byteNum]) & (~abEntry->m_mask[byteNum]);
	}
	tempData.m_vbit = abEntry->m_vbit;

	NlmCm__memcpy(abEntry, &tempData, sizeof(NlmDevABEntry));

	return ;
}
/*=====================================================================================*/

/*
Function: NlmTblMgr__pvt_ctor
Parameters:
        NlmTblMgr        *self,
        NlmCmAllocator  *alloc_p,
        NlmDevMgr     *devMgr_p,
        NlmGenericTblMgrBlksRange *gtmBlksRange,
        NlmIndexChangedAppCb    indexChangedAppCb
Return Type: NlmTblMgr
Description: constructor of TM creates and initializes the block memory manager.
It assigns the memory allocator, Device Manager pointer and NlmIndexChangedAppCb
function pointer to its member variables. It returns the same Table Manager pointer
which is a input parameter.
*/ 
static NlmTblMgr* 
NlmTblMgr__pvt_ctor(
    NlmTblMgr        *self,
    NlmCmAllocator  *alloc_p,
    NlmDevMgr     *devMgr_p,
    NlmGenericTblMgrBlksRange *gtmBlksRange,
    NlmIndexChangedAppCb    indexChangedAppCb
    )
{
    NlmReasonCode reason = NLMRSC_REASON_OK;
        
    self->m_alloc_p    = alloc_p;
    self->m_devMgr_p = devMgr_p;
    self->m_searchAttrList = NULL;
    self->m_tableCount = 0;
    self->m_indexChangedAppCb = indexChangedAppCb;
    
    self->m_blockMemMgr = NlmBlkMemMgr__Init(alloc_p,
                                                NlmTblMgr__pvt_ShiftRecord,
                                                NlmTblMgr__pvt_AssignBlockWidth,
                                                NlmTblMgr__pvt_ModifyBlockAttrib,
                                                NlmTblMgr__pvt_RemoveRecordCB,
                                                (nlm_u8)devMgr_p->m_devCount,
                                                gtmBlksRange,
                                                self,
                                                &reason);

    return self;
}

/*
Function: NlmTblMgr__pvt_dtor
Parameters:
        NlmTblMgr *self
Return Type: void
Description: destructor of TM destroys the block memory manager. It takes
Table Manager pointer as input and returns nothing.
*/ 
static void 
NlmTblMgr__pvt_dtor(
    NlmTblMgr *self
    )
{
    NlmReasonCode reason = NLMRSC_REASON_OK;

    NlmBlkMemMgr__Destroy(self->m_blockMemMgr, &reason);
}

/*
Function: NlmTblMgr__ConfigSearch
Parameters:
        NlmGenericTblMgr  *genericTblMgr_p,
        nlm_u8             ltrNum,
        NlmGenericTblSearchAttributes  *psAttrs,
        NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: It configures the search attribute nodes. If search attributes are
not found in the existing search_attribute_list, it creates one node and add into
the list. If search attribute node already exists for a table_ID, it replaces the
attributes with new ones. It takes GTM pointer, ltr number for which the search
attributes are to be set and NlmGenericTblSearchAttributes as input. Return value 
of NLMERR_OK indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__ConfigSearch(
    NlmGenericTblMgr  *genericTblMgr_p,
    nlm_u8             ltrNum, 
    NlmGenericTblSearchAttributes  *searchAttrs_p,
    NlmReasonCode    *o_reason
    )
{
    NlmTblMgr* tblmgr_p = NULL;
	nlm_32 i = 0;
	NlmSearchAttributesNode *curNode_p = NULL;
	NlmGenericTblList *gtmTblListHead_p =  ((NlmGenericTblList*)(genericTblMgr_p->m_genericTbl_list_p));
	NlmGenericTblList *gtmTblListNode_p = NULL;

    if(!searchAttrs_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
        return NLMERR_NULL_PTR;
    }

    if(ltrNum >=NLMDEV_NUM_LTR_SET)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }


	for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; ++i)
	{
		if( searchAttrs_p->m_psInfo[i].m_keyNum > NLMDEV_KEY_3)
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_KEY_NUM;
			return NLMERR_FAIL;
		}

		/* If devtype is sahasra, Key 0 is not available for GTM searches */
		if(genericTblMgr_p->m_devType == NLM_DEVTYPE_2_S &&
			 searchAttrs_p->m_psInfo[i].m_keyNum == NLMDEV_KEY_0)
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_KEY_NUM;
			return NLMERR_FAIL;
		}       

		if( searchAttrs_p->m_psInfo[i].m_rsltPortNum > NLMDEV_PARALLEL_SEARCH_3  )
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_RESULT_SEQ_NUM;
			return NLMERR_FAIL;
		}

		if(!searchAttrs_p->m_psInfo[i].m_tblId_p)
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_INPUT;
			return NLMERR_FAIL;
		}

		if(!NlmGenericTblList__Search(gtmTblListHead_p,
									 searchAttrs_p->m_psInfo[i].m_tblId_p ))
		{
			/*The table does not exist in the list containing all tables added to the GTM */
			if(o_reason)
				*o_reason = NLMRSC_INVALID_INPUT;
			return NLMERR_FAIL;
		}
	}
    
    tblmgr_p = (NlmTblMgr*)(genericTblMgr_p->m_tblMgr_p);

	for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; ++i)
	{
		gtmTblListNode_p = NlmGenericTblList__Search(gtmTblListHead_p,
									 searchAttrs_p->m_psInfo[i].m_tblId_p );

		if(gtmTblListNode_p)
			gtmTblListNode_p->m_gt_p->m_isLtrConfigured  = NlmTrue;

		/* search attribute node with table_ID */
		curNode_p = NlmSearchAttributeNode__Find(
                                    tblmgr_p->m_searchAttrList,
                                    searchAttrs_p->m_psInfo[i].m_tblId_p);


		if(curNode_p)
		{   /* if search_attribute associated with table exist, modify the table attributes */
			NlmSearchAttributeNode__Modify(curNode_p,
											ltrNum,
											searchAttrs_p->m_psInfo[i].m_rsltPortNum,
											searchAttrs_p->m_psInfo[i].m_keyNum,
											&(searchAttrs_p->m_psInfo[i].m_kcm)
											);
	        
		}
		else
		{
			/* if search_attribute associated with table does not exist, add new attribute node */
			NlmSearchAttributeNode__Add(tblmgr_p->m_alloc_p,
										&(tblmgr_p->m_searchAttrList),
										searchAttrs_p->m_psInfo[i].m_tblId_p,
										ltrNum,
										searchAttrs_p->m_psInfo[i].m_rsltPortNum,
										searchAttrs_p->m_psInfo[i].m_keyNum,
										&(searchAttrs_p->m_psInfo[i].m_kcm)
										);
		}

	}

    return NLMERR_OK;

}


/*
Function: NlmTblMgr__LockConfiguration
Parameters:
        NlmGenericTblMgr  *genericTblMgr_p,
        NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: It actually sets the BMR to be used for search operations.
Currently only BMR 7 (NO BMR) is being used. Then it writes the key construction 
register. Configuration will be locked by GTM when this function returns.
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t  NlmTblMgr__LockConfiguration (
            NlmGenericTblMgr  *genericTblMgr_p,
            NlmReasonCode    *o_reason
        )
{
    NlmErrNum_t err = NLMERR_OK;


    if(NLMTRUE ==genericTblMgr_p->m_IsConfigLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_CONFIGURATION_LOCKED;
        return NLMERR_FAIL;
    }

    
    err = NlmTblMgr__pvt_WriteKcr(genericTblMgr_p->m_tblMgr_p,  o_reason);
    
    return err;
}

/*
Function: NlmTblMgr__AddRecord
Parameters:
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    nlm_u16             groupId,
    NlmReasonCode       *o_reason
Return Type: NlmErrNum_t
Description: This function adds a record to a table identified with 'genericTbl'.
It calls block memory manager which algorithmically decides and returns a index 
into the device where the new record should be written. Then it calls 
Index-Change_CallBack to inform client about the new index for record to be 
written. And then it calls Write_Record which writes the table record into the 
device. It is necessary to provide the explicit priority for the record. Index 
assigned to the record is given as output in 'o_recordIndex'. Return value of 0 
indicates success, and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__AddRecord(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    nlm_u16             groupId,
    NlmReasonCode       *o_reason
    )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;
    NlmBool isShuffleDown = NlmFalse;
    
    if(!tblRecord)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_NULL_PTR;
    }
    if(!o_recordIndex)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_OUTPUT;
        return NLMERR_NULL_PTR;
    }

	if(!genericTbl->m_isLtrConfigured)
	{
		if(o_reason)
			*o_reason = NLMRSC_NO_LTR_CONFIGURED;
		return NLMERR_FAIL;
	}
    
    self = (NlmTblMgr*)(genericTbl->m_genericTblMgr_p->m_tblMgr_p);

    err = NlmBlkMemMgr__AddRecord(
                                self->m_blockMemMgr, 
                                genericTbl,
                                recordPriority,
                                groupId,
                                o_recordIndex,
                                tblRecord->m_len,
                                &isShuffleDown,
                                o_reason
                                );
    if(NLMERR_OK !=err)
        return err;

    /*If we are shuffling down and the record length is 80b then we need to invalidate the 
    location where we are inserting the new record to prevent coherency problems. 
    If the record length is greater than 80b then we have already invalidated the location
    in shiftRecord function and no need to invalidate here */
    if(isShuffleDown && tblRecord->m_len <= NLMDEV_AB_WIDTH_IN_BITS)
    {
        err = NlmTblMgr__pvt_InvalidateRecord(self->m_devMgr_p,tblRecord->m_len,
                                        *o_recordIndex, o_reason);

        if(err != NLMERR_OK)
            return err;
    }

    /* Before writing new record into the device, inform the application the 
     * index assigned to this record. This is necessary to avoid invalid Associate Data access
     * problems. As this record is newly added, value returned for oldIndex is some
     * big number.
     */
    self->m_indexChangedAppCb(self->m_client_p, genericTbl, NLM_GTM_INVALID_INDEX, *o_recordIndex);

    return NlmTblMgr__pvt_WriteRecord(self->m_devMgr_p, tblRecord, *o_recordIndex, NlmTrue, o_reason);

}

/*
Function: NlmTblMgr__UpdateRecord
Parameters:
    NlmGenericTbl            *genericTbl,
    NlmGenericTblRecord     *tblRecord,
    NlmRecordIndex          recordIndex,
    NlmReasonCode           *o_reason
Return Type: NlmErrNum_t
Description: This function updates a record, at recordIndex, to a table identified with 'genericTbl'.
It first validates the input record at specified index and then it calls EraseRecord followed by 
Write_Record which writes the table record into the device. Return value of 0 indicates success, 
and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__UpdateRecord(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;
    
    if(!tblRecord)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_NULL_PTR;
    }

    if(tblRecord->m_len != genericTbl->m_width)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_FAIL;
    }

	if(!genericTbl->m_isLtrConfigured)
	{
		if(o_reason)
			*o_reason = NLMRSC_NO_LTR_CONFIGURED;
		return NLMERR_FAIL;
	}

    self = (NlmTblMgr *)genericTbl->m_genericTblMgr_p->m_tblMgr_p;
    err = NlmBlkMemMgr__ValidateRecord(self->m_blockMemMgr,  
                                        genericTbl, recordIndex, o_reason);
    if(NLMERR_OK !=err)
        return err;

    err = NlmTblMgr__pvt_EraseRecord(self->m_devMgr_p, genericTbl->m_width, recordIndex, o_reason);
    if(NLMERR_OK !=err)
        return err;

    return NlmTblMgr__pvt_WriteRecord(self->m_devMgr_p, tblRecord, recordIndex, NlmTrue, o_reason);
}

/*
Function: NlmTblMgr__DeleteRecord
Parameters:
    NlmGenericTbl       *genericTbl,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
Return Type: NlmErrNum_t
Description: This function deletes a record from the table identified with 
'genericTbl'. It calls the block_memory_manager to delete the record stored
at recordIndex from its data structure. Then it calls Erase_Record to delete
the entries from the device based in the index and table width. Return value 
of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__DeleteRecord(
    NlmGenericTbl       *genericTbl,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;

	if(!genericTbl->m_isLtrConfigured)
	{
		if(o_reason)
			*o_reason = NLMRSC_NO_LTR_CONFIGURED;
		return NLMERR_FAIL;
	}

    self = (NlmTblMgr *)genericTbl->m_genericTblMgr_p->m_tblMgr_p;
    err = NlmBlkMemMgr__DeleteRecord(self->m_blockMemMgr,
                                    genericTbl, recordIndex, o_reason);

    if(NLMERR_OK !=err)
        return err;

    err = NlmTblMgr__pvt_EraseRecord(self->m_devMgr_p, genericTbl->m_width, recordIndex, o_reason);

    return err;
}

/*
Function: NlmTblMgr__Destroy
Parameters:
            NlmGenericTblMgr *genericTblMgr_p,  
            NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: It calls the destructor of TM to free all the memory held by it
and destroys TM. Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t  NlmTblMgr__Destroy(
            NlmGenericTblMgr *genericTblMgr_p,  
            NlmReasonCode    *o_reason
        )
{
    NlmCmAllocator *alloc_p = NULL;

    alloc_p = ((NlmTblMgr*)genericTblMgr_p->m_tblMgr_p)->m_alloc_p;
    NlmTblMgr__pvt_dtor((NlmTblMgr*)genericTblMgr_p->m_tblMgr_p);    

    NlmCmAllocator__free(alloc_p, genericTblMgr_p->m_genericTblBlksRange);
    NlmCmAllocator__free(alloc_p, (NlmTblMgr*)genericTblMgr_p->m_tblMgr_p);
    
    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
}

/*
Function: NlmTblMgr__CreateTable
Parameters:
            NlmGenericTblMgr  *genericTblMgr_p,
            NlmGenericTbl* table_p,
            NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: It increases the table count by one and calls block memory 
manager to initialize the table related data structures. Return value of 0 
indicates success, and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__CreateTable(
                NlmGenericTblMgr  *genericTblMgr_p,
                NlmGenericTbl* table_p,
                NlmReasonCode    *o_reason
                )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;

    if((NLM_TBL_WIDTH_80 !=table_p->m_width) &&
        (NLM_TBL_WIDTH_160 !=table_p->m_width) &&
        (NLM_TBL_WIDTH_320 !=table_p->m_width) &&
        (NLM_TBL_WIDTH_640 !=table_p->m_width))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_TABLE_WIDTH;
        return NLMERR_FAIL;
    }

    self = (NlmTblMgr*)genericTblMgr_p->m_tblMgr_p;

	table_p->m_isLtrConfigured = NlmFalse;
    
    err = NlmBlkMemMgr__InitTbl(self->m_blockMemMgr, table_p, o_reason);

    if(NLMERR_OK !=err)
        return err;

    self->m_tableCount++;
    return err;
}

/*
Function: NlmTblMgr__DestroyTable
Parameters:
                NlmGenericTblMgr  *genericTblMgr_p,
                NlmGenericTbl     *genericTbl,
                NlmReasonCode     *o_reason
Return Type: NlmErrNum_t
Description: It destroys genericTbl. It calls the block memory manager to delete
its data associated with the table, decreases the table count and deletes its
search attributes from the TM. Return value of 0 indicates success, and anything 
else is failure.
*/
NlmErrNum_t NlmTblMgr__DestroyTable(
                        NlmGenericTblMgr  *genericTblMgr_p,
                        NlmGenericTbl     *genericTbl,
                        NlmReasonCode     *o_reason
                        )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;
    
    self = (NlmTblMgr*)genericTblMgr_p->m_tblMgr_p;

    err =  NlmBlkMemMgr__DestroyTbl(self->m_blockMemMgr, genericTbl, o_reason);

    if(NLMERR_OK !=err)
        return err;

    self->m_tableCount--;

    NlmSearchAttributeNode__Delete(self->m_alloc_p, 
                                                &(self->m_searchAttrList), 
                                                genericTbl->m_id_str_p);
    return err;
}

/*
Function: NlmTblMgr__Init
Parameters:
        NlmGenericTblMgr			*genericTableMgr_p,
        NlmCmAllocator				*alloc_p,
        void*						devMgr_p, 
        nlm_u8						numOfDevices,
         NlmGenericTblMgrBlksRange	*gtmBlksRange,
        NlmIndexChangedAppCb		indexChangedAppCb,
        void*						client_p,
        NlmReasonCode				*o_reason
Return Type: NlmErrNum_t
Description: The function allocates memory for TM and initializes its content
by calling Table Manager constructor. It initializes the virtual table by
assigning all function pointers which are called from GTM APIs. Return value of
0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__Init(
          NlmGenericTblMgr* genericTableMgr_p,
          NlmCmAllocator   *alloc_p,
          void*            devMgr_p, 
          nlm_u8           numOfDevices,
          NlmGenericTblMgrBlksRange *gtmBlksRange,
          NlmIndexChangedAppCb indexChangedAppCb,
          void*            client_p,
          NlmReasonCode    *o_reason
          )
{
    NlmTblMgr *self = NULL;
    NlmTblMgr__pvt_vtbl* vTable = NULL;
    nlm_u8 devNum = 0;

    if(NULL == devMgr_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

    if(NULL == indexChangedAppCb)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_APP_CALLBACK;
        return NLMERR_NULL_PTR;
    }
	
    if(gtmBlksRange == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE;
        return NLMERR_NULL_PTR;
    }

    if(numOfDevices > ((NlmDevMgr*)devMgr_p)->m_devCount)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_NUM_OF_DEVICES;
        return NLMERR_FAIL;
    }

    if((genericTableMgr_p->m_genericTblBlksRange = NlmCmAllocator__calloc(alloc_p, 
        ((NlmDevMgr*)devMgr_p)->m_devCount, sizeof(NlmGenericTblMgrBlksRange))) == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }    

    for(devNum = 0; devNum < numOfDevices; devNum++)
    {       
        if(gtmBlksRange[devNum].m_startBlkNum == NLM_GTM_BLK_RANGE_NO_BLK_USED
                || gtmBlksRange[devNum].m_endBlkNum == NLM_GTM_BLK_RANGE_NO_BLK_USED)
        {
            genericTableMgr_p->m_genericTblBlksRange[devNum].m_startBlkNum = NLM_GTM_BLK_RANGE_NO_BLK_USED;
            genericTableMgr_p->m_genericTblBlksRange[devNum].m_endBlkNum = NLM_GTM_BLK_RANGE_NO_BLK_USED; 
            continue;
        }
        if(genericTableMgr_p->m_devType == NLM_DEVTYPE_2)
        {
            /* If start blk num = NLM_GTM_BLK_RANGE_WHOLE_DEVICE, whole device is
            available for GTM Tables */
            if(gtmBlksRange[devNum].m_startBlkNum == NLM_GTM_BLK_RANGE_WHOLE_DEVICE
                || gtmBlksRange[devNum].m_endBlkNum == NLM_GTM_BLK_RANGE_WHOLE_DEVICE)
            {
                genericTableMgr_p->m_genericTblBlksRange[devNum].m_startBlkNum = 0;
                genericTableMgr_p->m_genericTblBlksRange[devNum].m_endBlkNum = (NLMDEV_NUM_ARRAY_BLOCKS - 1); 
                continue;
            }           

            /* start blk number should be less than or equal to end blk number;
            also both the start blk number and end blk number should be less than
            or equal to total number of blocks */
            if(gtmBlksRange[devNum].m_startBlkNum > gtmBlksRange[devNum].m_endBlkNum
                || gtmBlksRange[devNum].m_startBlkNum >= NLMDEV_NUM_ARRAY_BLOCKS 
                || gtmBlksRange[devNum].m_endBlkNum >= NLMDEV_NUM_ARRAY_BLOCKS)
            {
                NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
                *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE; 
                return NLMERR_FAIL;        
            }
        }
        else
        { 
            if(gtmBlksRange[devNum].m_startBlkNum == NLM_GTM_BLK_RANGE_WHOLE_DEVICE
                || gtmBlksRange[devNum].m_endBlkNum == NLM_GTM_BLK_RANGE_WHOLE_DEVICE)
            {
                genericTableMgr_p->m_genericTblBlksRange[devNum].m_startBlkNum = 0;
                genericTableMgr_p->m_genericTblBlksRange[devNum].m_endBlkNum = 
                    (NLMDEV_NUM_ARRAY_BLOCKS - NLMDEV_NUM_AC15_BLOCKS) - 1; 
                continue;
            }

            /* start blk number should be less than or equal to end blk number;
            also both the start blk number and end blk number should be less than
            or equal to total number of blocks */
            if(gtmBlksRange[devNum].m_startBlkNum > gtmBlksRange[devNum].m_endBlkNum
                || gtmBlksRange[devNum].m_startBlkNum >= (NLMDEV_NUM_ARRAY_BLOCKS - NLMDEV_NUM_AC15_BLOCKS)
                || gtmBlksRange[devNum].m_endBlkNum >= (NLMDEV_NUM_ARRAY_BLOCKS - NLMDEV_NUM_AC15_BLOCKS))
            {
                NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
                *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE; 
                return NLMERR_FAIL;        
            }
        }

        /* Block range should be at super block boundary */
        if((gtmBlksRange[devNum].m_startBlkNum % 
            (NLMDEV_NUM_ARRAY_BLOCKS/NLMDEV_NUM_SUPER_BLOCKS)) 
            || ((gtmBlksRange[devNum].m_endBlkNum+1) %
            (NLMDEV_NUM_ARRAY_BLOCKS/NLMDEV_NUM_SUPER_BLOCKS)))
        {
            NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
            *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE;
            return NLMERR_FAIL;             
        }   
        genericTableMgr_p->m_genericTblBlksRange[devNum].m_startBlkNum = gtmBlksRange[devNum].m_startBlkNum;
        genericTableMgr_p->m_genericTblBlksRange[devNum].m_endBlkNum = gtmBlksRange[devNum].m_endBlkNum;
    }   

    /*Allocate and Initialize NlmTblMgr*/
    self = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmTblMgr));
    if(self == NULL)    
    {
        if(o_reason)
            *o_reason = NLMRSC_LOW_MEMORY;
        return NLMERR_NULL_PTR;
    }

    self = NlmTblMgr__pvt_ctor(self, alloc_p, devMgr_p, genericTableMgr_p->m_genericTblBlksRange, 
        indexChangedAppCb);
    
    genericTableMgr_p->m_tblMgr_p = self;

    self->m_client_p = client_p;

    /* assigning vTable */
    vTable = ((NlmTblMgr__pvt_vtbl*)(genericTableMgr_p->m_vtbl_p));
    vTable->AddRecord = NlmTblMgr__AddRecord;
    vTable->ConfigSearch = NlmTblMgr__ConfigSearch;
    vTable->CreateTable = NlmTblMgr__CreateTable;
    vTable->DeleteRecord = NlmTblMgr__DeleteRecord;
    vTable->UpdateRecord = NlmTblMgr__UpdateRecord;
    vTable->Destroy = NlmTblMgr__Destroy;
    vTable->DestroyTable = NlmTblMgr__DestroyTable;
    vTable->LockConfiguration = NlmTblMgr__LockConfiguration;

    return NLMERR_OK;
}

/*=====================================================================================*/

/*====== Linked list of search_attribute_node specific functions ======*/

/*
Function: NlmSearchAttributeNode__Find
Parameters:
        NlmSearchAttributesNode* SearchAttrList_p,
        nlm_8* TableId
Return Type: nlm_32
Description: It finds the index in the link list of search_attribute_node
associated with table_ID and return the index. If it doesn't find it returns
NOFOUND(-1).
*/
NlmSearchAttributesNode* 
NlmSearchAttributeNode__Find(
        NlmSearchAttributesNode* searchAttrHead_p,
        nlm_8* TableId
        )
{
    nlm_32 compareResult =0;
	NlmSearchAttributesNode* curNode_p = searchAttrHead_p;

    
    while(curNode_p)
    {
		compareResult = NlmCm__strcmp(curNode_p->m_tblId_p, TableId);

        if(compareResult == 0)
            return curNode_p;

        if(compareResult > 0)
            return NULL;

        curNode_p = curNode_p->m_Next_p;
    }
    return NULL;
}

/*
Function: NlmSearchAttributeNode__Add
Parameters:
        NlmCmAllocator* alloc_p,
        NlmSearchAttributesNode** SearchAttrList_pp,
        const nlm_8* tbl_ID,
        nlm_u8 LtrNum,
        NlmDevParallelSrchNum ps_RBus,
        NlmDevKeyNum KeyNum,
        nlm_u8* kcm
Return Type: NlmBool
Description: It creates a new search_attribute_node into the memory and assigns
the search attributes - table_ID, ltr_Num, Output_ResultBus, Key_Number and
key_map_bits to its member variable. It adds this newly created node into the
search_attribute link list. Returns TRUE on success else returns FALSE.
*/
NlmBool 
NlmSearchAttributeNode__Add(
        NlmCmAllocator* alloc_p,
        NlmSearchAttributesNode** searchAttrHead_pp,
        const nlm_8* tbl_ID,
        nlm_u8 LtrNum,
        NlmDevParallelSrchNum ps_RBus,
        NlmDevKeyNum KeyNum,
        NlmGenericTblKeyConstructionMap *kcm_p
        )
{
    nlm_u32 idx =0;
    NlmSearchAttributesNode* curNode_p = NULL;
    NlmSearchAttributesNode* previousNode_p = NULL;
    NlmSearchAttributesNode* newNode_p = NULL;
    curNode_p = *searchAttrHead_pp;

    newNode_p = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmSearchAttributesNode));
    
	if(!newNode_p)
        return NLMFALSE;

    for(idx =0; idx <NLMDEV_NUM_LTR_SET; idx++)
        newNode_p->m_isLtrConfigured[idx] = NLM_LTR_NOTCONFIGURED;

    if(kcm_p)
    {
		NlmCm__memcpy(&(newNode_p->m_kcm[LtrNum]), kcm_p, 
						sizeof(NlmGenericTblKeyConstructionMap));
    }

    newNode_p->m_keyNum[LtrNum] = KeyNum;
    newNode_p->m_isLtrConfigured[LtrNum] = NLM_LTR_CONFIGURED;
    newNode_p->m_Next_p = NULL;
    newNode_p->m_ps_RBus[LtrNum] = ps_RBus;
    newNode_p->m_tblId_p = NlmCmAllocator__calloc(alloc_p, (NlmCm__strlen(tbl_ID)+1), 1);
    NlmCm__strcpy(newNode_p->m_tblId_p, tbl_ID);

    if(!curNode_p)
    {
        *searchAttrHead_pp = newNode_p;
        return NLMTRUE;
    }

    while(curNode_p)
    {
        if(NlmCm__strcmp(newNode_p->m_tblId_p, curNode_p->m_tblId_p) < 0)
        {
            newNode_p->m_Next_p = curNode_p;
            if(previousNode_p)
                previousNode_p->m_Next_p = newNode_p;
            else
                *searchAttrHead_pp = newNode_p;
            break;
        }
        previousNode_p = curNode_p;
        curNode_p = curNode_p->m_Next_p;
    }
    if(!curNode_p)
        previousNode_p->m_Next_p = newNode_p;

    return NLMTRUE;
}

/*
Function: NlmSearchAttributeNode__Modify
Parameters:
        NlmSearchAttributesNode* curNode_p,
        nlm_u8 LtrNum,
        nlm_u8 ps_RBus,
        nlm_u8 KeyNum,
        NlmGenericTblKeyConstructionMap* kcm
Return Type: NlmBool
Description: It modifies and existing search_attribute_node SearchAttrList_p 
into the search_attribute link list. Returns TRUE on success else returns FALSE.
*/
NlmBool 
NlmSearchAttributeNode__Modify(
        NlmSearchAttributesNode* curNode_p,
        nlm_u8 LtrNum,
        nlm_u8 ps_RBus,
        nlm_u8 keyNum,
        NlmGenericTblKeyConstructionMap* kcm_p
        )
{
    if(!curNode_p)
        return NLMFALSE;

    curNode_p->m_ps_RBus[LtrNum] = ps_RBus;

    curNode_p->m_keyNum[LtrNum] = keyNum;
        
	if(kcm_p)
    {
        NlmCm__memcpy(&(curNode_p->m_kcm[LtrNum]), kcm_p, 
			sizeof(NlmGenericTblKeyConstructionMap)); 
    }

    curNode_p->m_isLtrConfigured[LtrNum] = NLM_LTR_CONFIGURED;

    return NLMTRUE;
}


/*
Function: NlmSearchAttributeNode__Delete
Parameters:
        NlmCmAllocator* alloc_p,
        NlmSearchAttributesNode** SearchAttrList_pp,
        const nlm_8* tbl_ID
Return Type: NlmBool
Description: It searches and deletes a search_attribute_node based on tbl_ID 
and update search_attribute link list. Returns TRUE on success, NOTFOUND if
no node contains this table_ID and FALSE if something else is wrong.
*/
NlmBool 
NlmSearchAttributeNode__Delete(
        NlmCmAllocator* alloc_p,
        NlmSearchAttributesNode** SearchAttrList_pp,
        const nlm_8* tbl_ID
        )
{
    NlmSearchAttributesNode* previous = NULL;
    NlmSearchAttributesNode* SearchAttrList_p = NULL;

    if(!SearchAttrList_pp)
        return NLMFALSE;

    SearchAttrList_p = *SearchAttrList_pp;

    if(!SearchAttrList_p)
        return NLMFALSE;

    while(SearchAttrList_p)
    {
        if(0 ==NlmCm__strcmp(SearchAttrList_p->m_tblId_p, tbl_ID))
        {
            if(!previous)
                *SearchAttrList_pp = SearchAttrList_p->m_Next_p;
            else
                previous->m_Next_p = SearchAttrList_p->m_Next_p;

            NlmCmAllocator__free(alloc_p, SearchAttrList_p->m_tblId_p);
            NlmCmAllocator__free(alloc_p, SearchAttrList_p);

            return NLMTRUE;
        }
        if(NlmCm__strcmp(SearchAttrList_p->m_tblId_p, tbl_ID) >0)
            return NLMNOTFOUND;

        previous = SearchAttrList_p;
        SearchAttrList_p = SearchAttrList_p->m_Next_p;
    }
    return NLMFALSE;
}
/*=====================================================================================*/



/*
Function: NlmTblMgr__pvt_WriteKcr
Parameters:
        NlmTblMgr* self,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function drives the KCR register write operation to device via
Device Manager. It first fetch the content information of KCR register from search
attributes, then formats the content accroding to Device Manager input format,
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__pvt_WriteKcr(
        NlmTblMgr* self,
        NlmReasonCode *o_reason
        )
{

    nlm_32 idx =0;
    NlmDevKeyNum keyNum =0;
	NlmDevParallelSrchNum psNum = 0;
    nlm_u32 dev_num = 0;
    NlmDev** dev_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmSearchAttributesNode* AttrList_p = NULL;
    NlmErrNum_t err = NLMERR_OK;
	nlm_u32 numDevices = ((NlmDevMgr*)self->m_devMgr_p)->m_devCount;
	nlm_32 i = 0, j = 0, regNr = 0;
	NlmBool done = NlmFalse;
	NlmDevMgr * devMgr_p = (NlmDevMgr*)self->m_devMgr_p;

    /* attribute list */
    AttrList_p = self->m_searchAttrList;

    /* retrieving the device list */
    dev_pp = ((NlmDev**)devMgr_p->m_devList_pp);

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&devMgr_p->m_spinLock);
#endif

    while(AttrList_p)
    {
        for(idx =0; idx < NLMDEV_NUM_LTR_SET; idx++)
        {
            if(AttrList_p->m_isLtrConfigured[idx] == NLM_LTR_NOTCONFIGURED)
                continue;

			keyNum = AttrList_p->m_keyNum[idx];
			psNum = AttrList_p->m_ps_RBus[idx];
            
			/* writing the kcr in all existing devices */
			for(dev_num = 0; dev_num < numDevices; dev_num++)
			{
				/* retrieving the shadow device of the corresponding device */
				shadow_p = (NlmDevShadowDevice*)(dev_pp[dev_num]->m_shadowDevice_p);
				
				done = NlmFalse;
				j = 0;
				for(regNr = NLMDEV_NUM_OF_KCR_PER_KEY * keyNum ; 
					regNr < NLMDEV_NUM_OF_KCR_PER_KEY * (keyNum +1) ; regNr++)
				{
					for(i = 0; i <  NLMDEV_NUM_OF_SEGMENTS_PER_KCR; ++i)
					{
						shadow_p->m_ltr[idx].m_keyConstruct[regNr].m_startByteLoc[i]=  
														AttrList_p->m_kcm[idx].m_segmentStartByte[j];

						shadow_p->m_ltr[idx].m_keyConstruct[regNr].m_numOfBytes[i] = 
														AttrList_p->m_kcm[idx].m_segmentNumOfBytes[j];
						
						/*There is no need to process the remaining parts further. 
						Break out of the loop */
						if(AttrList_p->m_kcm[idx].m_segmentNumOfBytes[j] == 0)
						{
							done = NlmTrue;
							break;
						}

						++j;

					}

					err = NlmDevMgr__LogicalTableRegisterWrite(
						dev_pp[dev_num], (nlm_u8)idx,
						NLMDEV_KEY_0_KCR_0_LTR + regNr ,
						&(shadow_p->m_ltr[idx].m_keyConstruct[regNr]),
						o_reason);

					if(err != NLMERR_OK )
					{
						#if defined NLM_MT_OLD || defined NLM_MT
						NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
						#endif
						return err;
					}

					if(done)
						break;
					
				}

				/*BMR is disabled when bmr select value is 7 */
				shadow_p->m_ltr[idx].m_miscelleneous.m_bmrSelect[psNum] = NLMDEV_NO_MASK_BMR_NUM;

				err = NlmDevMgr__LogicalTableRegisterWrite(
						dev_pp[dev_num], (nlm_u8)idx,
						NLMDEV_MISCELLENEOUS_LTR,
						&(shadow_p->m_ltr[idx].m_miscelleneous),
						o_reason);

				if(err != NLMERR_OK )
				{
					#if defined NLM_MT_OLD || defined NLM_MT
					NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
					#endif
					return err; 
				}
			}
			
            
        }
        AttrList_p = AttrList_p->m_Next_p;
    }

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
#endif

    return err;
}


/*
Function: NlmTblMgr__pvt_WriteRecord
Parameters:
        NlmDevMgr* devMgr_p,
        NlmGenericTblRecord *tblRecord,
        NlmRecordIndex RecordIndex,
        NlmBool convertToXY,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function drives the table_record_write operation to device via
Device Manager. It first formats the table_record accroding to Device Manager 
input format, then update the shadow device with the new table_records and call 
Device Manager to write the new table_records. A table record can consists of 
many entries (maximum 8 entries), so it uses NlmDevMgr__RequestMultiABEntryWrite
API followed by NlmDevMgr__ServiceRequests API of Device Manager to write all the 
entries at one shot. Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_WriteRecord(
            NlmDevMgr* devMgr_p,
            NlmGenericTblRecord *tblRecord,
            NlmRecordIndex RecordIndex,
            NlmBool convertToXY,
            NlmReasonCode *o_reason
            )
{
    nlm_u32 entryIdx =0;
    nlm_u32 wordIdx = 0;
    nlm_u8 dev_num =0;
    NlmErrNum_t err = NLMERR_OK;
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmDevShadowAB* shadowAB_p = NULL;
    nlm_u8 ab_num = 0;
    nlm_u16 nrOfWords = 0;
    nlm_u32 currentRow = 0;

    if(!devMgr_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

    dev_num = (nlm_u8)(RecordIndex / NLM_ALLINDEX_PARDEVICE);

    /* retrieving device list */
    devList_pp = (NlmDev**)devMgr_p->m_devList_pp;

    /* finding index within the device */
    entryIdx = RecordIndex - (dev_num*NLM_ALLINDEX_PARDEVICE);
    
    ab_num = (nlm_u8)(entryIdx /NLMDEV_AB_DEPTH);

    /* retrieving shadow device */
    shadow_p = (NlmDevShadowDevice*)devList_pp[dev_num]->m_shadowDevice_p;

    /* retrieving the same AB in shadow device */
    shadowAB_p = &(shadow_p->m_arrayBlock[ab_num]);

    /*Nr of words gives the number of 80bit entries present. 
    Table record length is in bits*/
    nrOfWords = tblRecord->m_len / NLMDEV_AB_WIDTH_IN_BITS;

    currentRow = (entryIdx % NLMDEV_AB_DEPTH) + nrOfWords - 1;

    if(currentRow >= NLMDEV_AB_DEPTH)
    {        
        return NLMRSC_INVALID_INPUT;
    }
    
    for(wordIdx =0; wordIdx < nrOfWords; wordIdx++)
    {
        NlmCmAssert(currentRow < NLMDEV_AB_DEPTH, "Incorrect row \n");
        
        NlmCm__memcpy(shadowAB_p->m_abEntry[currentRow].m_data, 
                        &tblRecord->m_data[wordIdx * NLMDEV_AB_WIDTH_IN_BYTES],
                        NLMDEV_AB_WIDTH_IN_BYTES);

        NlmCm__memcpy(shadowAB_p->m_abEntry[currentRow].m_mask, 
                        &tblRecord->m_mask[wordIdx * NLMDEV_AB_WIDTH_IN_BYTES],
                        NLMDEV_AB_WIDTH_IN_BYTES);

        shadowAB_p->m_abEntry[currentRow].m_vbit = NLM_VALIDBIT;


		/* Conversion to XY format is not needed for delete/erase record. */
		if(convertToXY == NlmTrue)
			NlmTblMgr__pvt_convertToXYMode(&(shadowAB_p->m_abEntry[currentRow]));

	    err = NlmDevMgr__ABEntryWrite(devList_pp[dev_num],
									    ab_num, 
									    (nlm_u16)currentRow, 
									    &(shadowAB_p->m_abEntry[currentRow]),
									    NLMDEV_XY,
									    o_reason);

	if(err != NLMERR_OK)
		return err;

        --currentRow;
    }
    
    
    return err;
}

/*
Function: NlmTblMgr__pvt_EraseRecord
Parameters:
        NlmDevMgr* devMgr_p,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function deletes the table_records from the device database 
using Device Manager. For deleting it actually writes all data-mask pair as 1-1
for all those of bits which constitute table record at RecordIndex. Doing so,
search result at this index will always be a MISS. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_EraseRecord(
        NlmDevMgr* devMgr_p,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        )
{
    nlm_u8 data[80];
    nlm_u8 mask[80];
    NlmGenericTblRecord TblRecord;
    
    if(!devMgr_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

    NlmCm__memset(data, 0xff, (recordLen/8));
    NlmCm__memset(mask, 0xff, (recordLen/8));

    TblRecord.m_data = data;
    TblRecord.m_mask = mask;
    TblRecord.m_len = recordLen;

	/* data and mask are filled as 0xff and in already in XY mode. X/Y as 1/1 is always
	  * miss in XY mode. There is no equivalent value set in D-M mode for always miss
	  * Thats why, we give the data/mask in XY mode and hence "NlmFalse" below so
	  * that pvt_WriteRecord does not need to convert the data/mask to XY format
	  * before writing into the device.
	  */
    return NlmTblMgr__pvt_WriteRecord(devMgr_p, &TblRecord, RecordIndex, NlmFalse, o_reason);
}



/*
Function: NlmTblMgr__pvt_InvalidateRecord
Parameters:
        NlmDevMgr* devMgr_p,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function invalidates the table_records from the device database 
using Device Manager. The invalid bit is set for the appropriate ABs
*/
NlmErrNum_t 
NlmTblMgr__pvt_InvalidateRecord(
        NlmDevMgr* devMgr_p,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        )
{
    nlm_u8 devNum = (nlm_u8)(RecordIndex / NLM_ALLINDEX_PARDEVICE);  
    nlm_u32 indexInDevice = RecordIndex % NLM_ALLINDEX_PARDEVICE;
    nlm_u8 abNum =  (nlm_u8)(indexInDevice / NLMDEV_AB_DEPTH);
    nlm_u16 addrInAB = (nlm_u16)(indexInDevice % NLMDEV_AB_DEPTH);
    
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmDevShadowAB* shadowAB_p = NULL;

    NlmErrNum_t errNum = NLMERR_OK;
    
    
    if(!devMgr_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

	devList_pp = (NlmDev**)devMgr_p->m_devList_pp;
    shadow_p = (NlmDevShadowDevice*)devList_pp[devNum]->m_shadowDevice_p;
    shadowAB_p = &(shadow_p->m_arrayBlock[abNum]);


    /*Update the valid bit in shadow memory and write to the device */
    shadowAB_p->m_abEntry[addrInAB].m_vbit = 0;
    errNum = NlmDevMgr__ABEntryInvalidate(devList_pp[devNum], abNum, addrInAB, o_reason);
    

    if(errNum != NLMERR_OK)
        return errNum;

    /*In case we have 640-bit entry, then we have to invalidate the higher 320bits and the lower
    320 bits */
    if(recordLen > 320)
    {
        shadowAB_p->m_abEntry[addrInAB + 4].m_vbit = 0;
        errNum = NlmDevMgr__ABEntryInvalidate(devList_pp[devNum], abNum, addrInAB + 4, o_reason);
    }
    

    return errNum;
}


/*
Function: NlmTblMgr__pvt_WriteBCR
Parameters:
        NlmDevMgr* devMgr_p,
        nlm_32 blockNum,
        nlm_u16 width,
        NlmDevDisableEnable Disbl_Enbl,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function drives the BCR register write operation to device via
Device Manager. It first creates a local BCR register and assigns all the information
passed such as width, Disbl_Enbl . Then it fetchs the shadow BCR register 
corresponding to blockNum and compares shadow BCR contents with local BCR contents. 
If it doesn't match, it updates the shadow BCR register and call Device Manager to 
write the new BCR register contents. Return value of 0 indicates success, and anything 
else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_WriteBCR(
        NlmDevMgr* devMgr_p,
        nlm_32 blockNum, /* global blockNumber across all devices, ranges from 0 to 511 */
        nlm_u16 width,
        NlmDevDisableEnable Disbl_Enbl,
        NlmReasonCode *o_reason
        )
{
    nlm_u8 widthIn80Bit =0;
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmDevShadowAB* shadowAB_p = NULL;
	NlmErrNum_t errNum = NLMERR_OK;
    
    nlm_u8 dev_num = (nlm_u8)(blockNum / NLMDEV_NUM_ARRAY_BLOCKS);
    nlm_u8 ab_num = (nlm_u8) (blockNum % NLMDEV_NUM_ARRAY_BLOCKS);

    if(!devMgr_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

    /* retrieving divice list */
    devList_pp = (NlmDev**)devMgr_p->m_devList_pp;

    /* retrieving shadow device */
    shadow_p = (NlmDevShadowDevice*)devList_pp[dev_num]->m_shadowDevice_p;

    /* retrieving the same AB in shadow device */
    shadowAB_p = &(shadow_p->m_arrayBlock[ab_num]);

    shadowAB_p->m_blkConfig.m_blockEnable = Disbl_Enbl;

    if(NLM_INVALID_BLOCKWIDTH !=width)
    {
        /* if width is VALID, assign it to BCR */
        widthIn80Bit = (nlm_u8)(width/NLMDEV_AB_WIDTH_IN_BITS);
        shadowAB_p->m_blkConfig.m_blockWidth = (widthIn80Bit >>1) - (widthIn80Bit/8);
	}
    
    /* if local bcr is different from shadow bcr, send BCR write command */
    errNum =  NlmDevMgr__BlockRegisterWrite(devList_pp[dev_num],
                                            ab_num,
                                            NLMDEV_BLOCK_CONFIG_REG,
                                            &(shadowAB_p->m_blkConfig), 
											o_reason);

    return errNum;
    
}

/*
Function: NlmTblMgr__pvt_WriteLtrBS
Parameters:
        NlmDev* dev_p,
        nlm_u8 ltrNum,
        nlm_u8 ab_num,
        NlmDevDisableEnable enable_disable,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function drives the Ltr_Block_Select register write operation to 
device via Device Manager. It first fetch the shadow LtrBS register with ltr set 
number ltrNum and compares the enable status of ab_num with enable_disable. If it 
doesn't match, it update the enable status of ab_num with enable_disable and call 
Device Manager to write the new LtrBS register contents. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_WriteLtrBS(
            NlmDev* dev_p,
            nlm_u8 ltrNum,
            nlm_u8 ab_num, /*block number within a device */
            NlmDevDisableEnable enable_disable,
            NlmReasonCode *o_reason
            )
{
    NlmDevShadowLtr* shadowLtr_p = NULL;
    nlm_u8 nrOfBlksInBSR = NLMDEV_NUM_ARRAY_BLOCKS/2;
    nlm_u8 regNr = ab_num / nrOfBlksInBSR;
	NlmErrNum_t errNum = NLMERR_OK;
    
	ab_num = ab_num % nrOfBlksInBSR;

    if(!dev_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&dev_p->m_devMgr_p->m_spinLock);
#endif
   
    /* retrieving shadow ltr for the corresponding device */
    shadowLtr_p = ((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_ltr;
        
    if(shadowLtr_p[ltrNum].m_blockSelect[regNr].m_blkEnable[ab_num] ==
		                  (Nlm11kDevDisableEnable)enable_disable)
	{
		#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&dev_p->m_devMgr_p->m_spinLock);
		#endif
        return NLMERR_OK;
	}
    
    shadowLtr_p[ltrNum].m_blockSelect[regNr].m_blkEnable[ab_num] = enable_disable;

    errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p, ltrNum,
                                        NLMDEV_BLOCK_SELECT_0_LTR + regNr,
                                        &(shadowLtr_p[ltrNum].m_blockSelect[regNr]), 
                                        o_reason);

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinUnlock(&dev_p->m_devMgr_p->m_spinLock);
#endif

	return errNum;
}

/*
Function: NlmTblMgr__pvt_WriteLtrPSAndKPUSelect
Parameters:
        NlmDev* dev_p,
        nlm_u8 ltrNum,
        nlm_u8 ab_num,
        NlmDevParallelSrchNum rBus,
        NlmDevKeyNum key,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function drives the Ltr_Parallel_search register write 
operation to device via Device Manager. It first updates block to result bus mapping 
into the shadow PS register and calls Device Manager API to write the new 
PS register contents. It then updates the super block to key mappinginto the shadow
Super Block KPU Select register and calls the Device Manager API to write the new
Super Block KPU Select register contents.
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_WriteLtrPSAndKPUSelect(
        NlmDev* dev_p,
        nlm_u8 ltrNum,
        nlm_u8 ab_num, /*block number within a device */
        NlmDevParallelSrchNum rBus,
        NlmDevKeyNum key,
        NlmReasonCode *o_reason
        )
{
    nlm_u8 sb_num = 0;
    NlmDevShadowLtr* shadowLtr_p = NULL;
    nlm_u8 ps_reg_nr = 0;
    NlmDevLtrRegType ltr_reg_type = NLMDEV_PARALLEL_SEARCH_0_LTR;
    NlmErrNum_t errNum = NLMERR_OK;

    if(!dev_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&dev_p->m_devMgr_p->m_spinLock);
#endif

    /* finding super block nuber */
    sb_num = ab_num/4;

    /* finding parallel_search_register number */
    ps_reg_nr = ab_num / (NLMDEV_NUM_ARRAY_BLOCKS/4);

    ab_num = ab_num%(NLMDEV_NUM_ARRAY_BLOCKS/4);

    /* retrieving shadow ltr for the corresponding device */
    shadowLtr_p = ((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_ltr;
        
    shadowLtr_p[ltrNum].m_parallelSrch[ps_reg_nr].m_psNum[ab_num] = rBus;

	/* Write to the Parallel Search Register */
   errNum =  NlmDevMgr__LogicalTableRegisterWrite(dev_p, ltrNum,
						                            ltr_reg_type + ps_reg_nr,
						                            &(shadowLtr_p[ltrNum].m_parallelSrch[ps_reg_nr]),
						                            o_reason);
    if(errNum != NLMERR_OK)
	{
		#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&dev_p->m_devMgr_p->m_spinLock);
		#endif
		return errNum;
	}

     
    shadowLtr_p[ltrNum].m_superBlkKeyMap.m_keyNum[sb_num] = key;

	/*Write to the Super Block KPU Select Register */
    errNum =  NlmDevMgr__LogicalTableRegisterWrite(dev_p, ltrNum,
						                            NLMDEV_SUPER_BLK_KEY_MAP_LTR,
						                            &(shadowLtr_p[ltrNum].m_superBlkKeyMap),
						                            o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinUnlock(&dev_p->m_devMgr_p->m_spinLock);
#endif
   return errNum;
	
}



/*=====================================================================================*/

/*====== Callback functions used by block memory manager ======*/

/*
Function: NlmTblMgr__pvt_ShiftRecord
Parameters:
        void* context_p,
        NlmGenericTbl* genericTbl_p,
        NlmRecordIndex oldAddress,
        NlmRecordIndex newAddress,
        nlm_u16 recordLen,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function moves an existing table record from its current
position to a new position. This happens when a new record comes to be added
into the database. Before moving the record into the device, it first calls
Index_changed call back function of the application to inform user about the
shifting. Then it calls the Device Manager to write the new record into the 
database if the same record is not already present in shado device at same
index. Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_ShiftRecord(
        void* context_p,
        NlmGenericTbl* genericTbl_p,
        NlmRecordIndex oldAddress,
        NlmRecordIndex newAddress,
        nlm_u16   recordLen,
        NlmReasonCode *o_reason
        )
{
    NlmErrNum_t err = NLMERR_OK;
    nlm_u8 old_dev_num =0;
    nlm_u8 new_dev_num = 0;
    nlm_u8 old_ab_num = 0;
    nlm_u8 new_ab_num = 0;
    nlm_u32 oldIdx = 0;
    nlm_u32 newIdx = 0;
    nlm_u16 wordIdx = 0;
    nlm_u32 oldRow = 0;
    nlm_u32 newRow = 0;
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* old_shadow_p = NULL;
    NlmDevShadowDevice* new_shadow_p = NULL;
    NlmDevShadowAB* old_shadowAB_p = NULL;
    NlmDevShadowAB* new_shadowAB_p = NULL;
    NlmTblMgr* self = context_p;

    if(!self)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }


    /* finding device number */
    old_dev_num = (nlm_u8)(oldAddress/NLM_ALLINDEX_PARDEVICE);
    new_dev_num = (nlm_u8)(newAddress/NLM_ALLINDEX_PARDEVICE); 
    
    /* retrieving device list */
    devList_pp = (NlmDev**)self->m_devMgr_p->m_devList_pp;

    /* finding index within the device */
    oldIdx = oldAddress % NLM_ALLINDEX_PARDEVICE;
    newIdx = newAddress % NLM_ALLINDEX_PARDEVICE;
    
    /* finding ab number in the device */
    old_ab_num = (nlm_u8)(oldIdx/NLMDEV_AB_DEPTH);
    new_ab_num = (nlm_u8)(newIdx/NLMDEV_AB_DEPTH);

    /* retrieving shadow device */
    old_shadow_p = (NlmDevShadowDevice*)devList_pp[old_dev_num]->m_shadowDevice_p;
    new_shadow_p = (NlmDevShadowDevice*)devList_pp[new_dev_num]->m_shadowDevice_p;

    /* retrieving the AB in shadow device */
    old_shadowAB_p = &(old_shadow_p->m_arrayBlock[old_ab_num]);
    new_shadowAB_p = &(new_shadow_p->m_arrayBlock[new_ab_num]);

    oldRow = (oldIdx % NLMDEV_AB_DEPTH);
    newRow = (newIdx % NLMDEV_AB_DEPTH);
    
    /* One time check of all asserts */
    if(((oldRow + (recordLen / NLMDEV_AB_WIDTH_IN_BITS)) > NLMDEV_AB_DEPTH)||
       ((newRow + (recordLen / NLMDEV_AB_WIDTH_IN_BITS)) > NLMDEV_AB_DEPTH))
    {
        return NLMRSC_INVALID_INPUT;
    }

    /* If we are shifting the records up, inform the application first, about the
     * index change. This is necessary to avoid invalid Associate Data access problems.
     * (PHMD)
     */
    if(oldAddress >= newAddress)
    {
        self->m_indexChangedAppCb(self->m_client_p, genericTbl_p, oldAddress, newAddress);
    }
    
    /*Write the record to the device */
    for(wordIdx =0; wordIdx < recordLen / NLMDEV_AB_WIDTH_IN_BITS; wordIdx++)
    {
        NlmCmAssert(oldRow < NLMDEV_AB_DEPTH, "Incorrect row \n");

        NlmCmAssert(newRow < NLMDEV_AB_DEPTH, "Incorrect row \n");

        NlmCm__memcpy(new_shadowAB_p->m_abEntry[newRow].m_data, 
                        old_shadowAB_p->m_abEntry[oldRow].m_data,
                        NLMDEV_AB_WIDTH_IN_BYTES);

        NlmCm__memcpy(new_shadowAB_p->m_abEntry[newRow].m_mask, 
                      old_shadowAB_p->m_abEntry[oldRow].m_mask,
                      NLMDEV_AB_WIDTH_IN_BYTES);

        new_shadowAB_p->m_abEntry[newRow].m_vbit = NLM_VALIDBIT;

        err = NlmDevMgr__ABEntryWrite(devList_pp[new_dev_num],
                                        new_ab_num, 
                                        (nlm_u16)newRow,
                                        &new_shadowAB_p->m_abEntry[newRow],
                                        NLMDEV_XY,
                                        o_reason);
        if(NLMERR_OK != err)
            return err;

        ++oldRow;
        ++newRow;
    }

    /*If we are shuffling down, we first write to the device and then call the index changed 
    callback to prevent coherency problems */
    if(oldAddress < newAddress)
    {
        self->m_indexChangedAppCb(self->m_client_p, genericTbl_p, oldAddress, newAddress);
    }

    /*If we are writting more than 80 bits while shuffling, then to prevent coherency issues
    we have to invalidate the old address of the entry that has been shuffled. Only after this
    we can write the the next shuffled entry to this location */
    if(recordLen > NLMDEV_AB_WIDTH_IN_BITS)
    {
        err = NlmTblMgr__pvt_InvalidateRecord(self->m_devMgr_p, genericTbl_p->m_width, oldAddress, o_reason);

        if(NLMERR_OK != err)
            return err;

    }



    return err;
}



/*
Function: NlmTblMgr__pvt_AssignBlockWidth
Parameters:
        void* context_p,
        nlm_32 blockNum,
        nlm_u16 width,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function is called by block memory manager in case a new record
is to be written in a new array block. So the block has to be assigned its width
according to table width and to do so, this function calls Device Manager to write
BCR register. Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_AssignBlockWidth(
            void* context_p,
            nlm_32 blockNum,
            nlm_u16 width,
            NlmReasonCode *o_reason
            )
{
    NlmTblMgr* self = context_p;

    if(!self)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }

    return NlmTblMgr__pvt_WriteBCR(self->m_devMgr_p,
                                blockNum,
                                width,
                                NLMDEV_ENABLE,
                                o_reason);
}

/*
Function: NlmTblMgr__pvt_ModifyBlockAttrib
Parameters:
        void* context_p,
        NlmGenericTbl* gt_p,
        nlm_32  blockNum,
        NlmBool addBlock,
        NlmBool isBlkEmpty, 
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function is called by block memory manager. If a block
is to be emptied, BlkMemMgr calls this function to disable the block and so
writing the BCR and LtrBS registers with modified values. If block is to
be occupied with some other table, BlkMemMgr calls this function to change
its blockwidth if this new table is of different width.
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__pvt_ModifyBlockAttrib(
            void* context_p,
            NlmGenericTbl* gt_p,
            nlm_32  blockNum,
            NlmBool addBlock,
            NlmBool isBlkEmpty, 
            NlmReasonCode *o_reason
            )
{
    NlmSearchAttributesNode* AttrNode = NULL;
    NlmDev** devList_pp = NULL;
    nlm_u8 dev_num =0;
    nlm_u8 idx =0;
    NlmErrNum_t err = NLMERR_OK;
    NlmTblMgr* self = context_p;

    if(!self)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }
    if(!gt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_NULL_PTR;
    }

    if(isBlkEmpty)
    {   /* if a block is just being emptied, disable it */
        err = NlmTblMgr__pvt_WriteBCR(self->m_devMgr_p,
                                blockNum,
                                NLM_INVALID_BLOCKWIDTH,
                                NLMDEV_DISABLE,
                                o_reason);

		if(err != NLMERR_OK)
			return err;
    }

    /* finding device number */
    dev_num = (nlm_u8)(blockNum/NLMDEV_NUM_ARRAY_BLOCKS);
    
    /* finding device list */
    devList_pp = (NlmDev**)self->m_devMgr_p->m_devList_pp;

    /* finding search_attribute_node of the table */
    AttrNode = NlmSearchAttributeNode__Find(self->m_searchAttrList, gt_p->m_id_str_p);
     

    if(NULL ==AttrNode)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
        return NLMERR_FAIL;
    }
    
    /* if an empty new block is going to be used for filling record */
    if(addBlock)
    {
        for(idx =0; idx <NLMDEV_NUM_LTR_SET; idx++)
        {
            if(NLM_LTR_NOTCONFIGURED == AttrNode->m_isLtrConfigured[idx])
                continue;
            
            /* enabling the block in ltr_blockselect_register */
            err =NlmTblMgr__pvt_WriteLtrBS(devList_pp[dev_num], idx, 
                                    (nlm_u8)(blockNum%NLMDEV_NUM_ARRAY_BLOCKS), 
                                    NLMDEV_ENABLE, o_reason);

            if(NLMERR_OK != err)
                return err;

            /* assinging the result_bus to this block according to PS assigned for this table */
            err = NlmTblMgr__pvt_WriteLtrPSAndKPUSelect(devList_pp[dev_num], idx,
                            (nlm_u8)(blockNum%NLMDEV_NUM_ARRAY_BLOCKS), 
                            AttrNode->m_ps_RBus[idx], AttrNode->m_keyNum[idx], o_reason);

            if(NLMERR_OK != err)
                return err;
        }
        
    }
    else    /* if it is not a new empty block */
    {
        for(idx =0; idx <NLMDEV_NUM_LTR_SET; idx++)
        {
            if(NLM_LTR_NOTCONFIGURED == AttrNode->m_isLtrConfigured[idx])
                continue;
            
            err =NlmTblMgr__pvt_WriteLtrBS(devList_pp[dev_num], idx, 
                                    (nlm_u8)(blockNum%NLMDEV_NUM_ARRAY_BLOCKS), 
                                    NLMDEV_DISABLE, o_reason);

            if(NLMERR_OK != err)
                return err;
        }

    }
    return err;
}

/*
Function: NlmTblMgr__pvt_RemoveRecordCB
Description: This function is called by block memory manager. When a table is 
destroyed, the individual records of the table must be erased in the hardware
NlmTblMgr__pvt_RemoveRecordCB is called by the block memory manager for erasing 
in the hardware each of the records of the table 
*/

NlmErrNum_t 
NlmTblMgr__pvt_RemoveRecordCB(
            void* context_p,
            NlmGenericTbl   *genericTbl_p,
            NlmRecordIndex  recordIndex, 
            nlm_u16         recordWidth, 
            NlmReasonCode   *o_reason_p)
{
    NlmTblMgr* self_p = context_p;
    NlmErrNum_t err = NLMERR_OK;

    if(!self_p)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }
    
    if(!genericTbl_p)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_NULL_PTR;
    }

    err = NlmTblMgr__pvt_EraseRecord(self_p->m_devMgr_p, recordWidth, recordIndex, o_reason_p);

    return err;

}



