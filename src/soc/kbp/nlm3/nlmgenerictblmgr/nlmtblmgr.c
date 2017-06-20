/*
 * $Id: nlmtblmgr.c,v 1.1.6.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
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
#ifdef NLM_12K_11K
#include <arch/nlmarch.h>
#endif


#include "nlmgenerictblmgr.h"
#include "nlmblkmemmgr.h"
#include "nlmxktblmgr.h"
#include "nlmdevmgr.h"
#include "nlmcmstring.h"
/* #include "nlmdevmgr_shadow.h" */

/* some utility definitions used locally */
#define NLMNOTFOUND                 (-1)
#define NLM_INVALID_BLOCKWIDTH      (641)

/*register/database write functions which interact with Device Manager */
#define NLM_VALIDBIT                (1)
#define NLM_INVALIDBIT              (0)
#ifdef NLM_12K_11K
#define NLM_ALLINDEX_PARDEVICE      (NLM11KDEV_NUM_ARRAY_BLOCKS*NLMDEV_AB_DEPTH)
#else
#define NLM_ALLINDEX_PARDEVICE      (NLMDEV_NUM_ARRAY_BLOCKS*NLMDEV_AB_DEPTH)
#endif

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
    nlm_u8                          m_tblId;
    NlmIsLtrConfigured              m_isLtrConfigured[NLMDEV_MAX_NUM_LTRS];
    NlmDevParallelSrch              m_ps_RBus[NLMDEV_MAX_NUM_LTRS];
    NlmDevKey                       m_keyNum[NLMDEV_MAX_NUM_LTRS];
    NlmGenericTblKeyConstructionMap m_kcm[NLMDEV_MAX_NUM_LTRS];

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
    NlmDevMgr                   *m_devMgr_p; /* internally has to be cast to the
                                                     correct Device Manager in the table
                                                     manager for the device  */
    NlmCmAllocator              *m_alloc_p; 
    NlmIndexChangedAppCb        m_indexChangedAppCb;
    NlmSearchAttributesNode     *m_searchAttrList;  /* sorted in the
                                        ascending order of table_id. Last
                                        node is NULL */
    nlm_u16                     m_tableCount;
    void                        *m_blockMemMgr;
    void                        *m_client_p;

    NlmDevShadowUdaSb *m_udaSbMem_p;

    NlmPortNum             m_portNumForCurOpr; /* Port num for current operation - used with callbacks*/

    
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
    NlmGenericTblMgrSBRange*    udaSbRange,
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
    NlmUdaChunkInfo* udaChunkInfo_p,
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
    nlm_u8 TableId
    );


/* to add a new search_attribute_node in the link list */
NlmBool 
NlmSearchAttributeNode__Add(
    NlmCmAllocator* alloc_p,
    NlmSearchAttributesNode** SearchAttrHead_pp,
    nlm_u8 tableId,
    nlm_u8 LtrNum,
    NlmDevParallelSrch psNum,
    NlmDevKey keyNum,
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
        nlm_u8 tableId
        );


/* for writing key_construction_register */
NlmErrNum_t 
NlmTblMgr__pvt_WriteKcr(
        NlmTblMgr* self,
        NlmPortNum portNum,
        NlmReasonCode *o_reason
        );

/* to write entry into device database */
NlmErrNum_t 
NlmTblMgr__pvt_WriteRecord(
            NlmGenericTbl *genericTbl_p,
            NlmPortNum portNum,
            NlmGenericTblRecord *tblRecord,
            nlm_u8          *assocData,
            NlmRecordIndex recordIndex,
            NlmBool convertToXY,
            NlmReasonCode *o_reason_p
            );


/* to delete a database record */
NlmErrNum_t NlmTblMgr__pvt_EraseRecord(
        NlmGenericTbl* genericTbl_p,
        NlmPortNum portNum,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        );

/* to invalidate a database record */
NlmErrNum_t NlmTblMgr__pvt_InvalidateRecord(
        NlmDevMgr* devMgr_p,
        NlmPortNum portNum,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        );

/* to write block_configuration_register */
NlmErrNum_t 
NlmTblMgr__pvt_WriteBCR(
        NlmDevMgr* devMgr_p,
        NlmPortNum portNum,
        nlm_32 blockNum,
        nlm_u16 BlkWidth,
        NlmDevDisableEnable Disbl_Enbl,
        NlmUdaChunkInfo* udaChunkInfo_p,
        NlmReasonCode *o_reason
        );

/* writing ltr_block_select register */
NlmErrNum_t 
NlmTblMgr__pvt_WriteLtrBS(
        NlmDev* dev_p,
        NlmPortNum portNum,
        nlm_u8 ltrNum,
        nlm_u8 ab_num,
        NlmDevDisableEnable enable_disable,
        NlmReasonCode *o_reason
        );

/* writing ltr_parallel_search register and super block to key mapping*/
NlmErrNum_t 
NlmTblMgr__pvt_WriteLtrPSAndKPUSelect(
        NlmDev* dev_p,
        NlmPortNum portNum,
        nlm_u8 ltrNum,
        nlm_u8 ab_num,
        NlmDevParallelSrch rBus,
        NlmDevKey key,
        NlmReasonCode *o_reason
        );


NlmGenericTblList* 
NlmGenericTblList__Search( NlmGenericTblList* head,
                        nlm_u8 tblId_p );


/* function converts the record data/mask to XY Format */
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
    NlmGenericTblMgrBlksRange *dbaBlksRange,
    NlmGenericTblMgrSBRange*    udaSbRange,
    NlmIndexChangedAppCb    indexChangedAppCb
    )
{
    NlmReasonCode reason = NLMRSC_REASON_OK;
    nlm_u16   NumOfABs = devMgr_p->m_numOfAbs;
        
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
                                                dbaBlksRange, udaSbRange,
                                                self,
                                                NumOfABs,
                                                &reason);

    if( udaSbRange->m_stSBNum != NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED &&
        udaSbRange->m_endSBNum != NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED )
    {
        /* Initialize the shadow memory for UDA SB  */
        self->m_udaSbMem_p = (NlmDevShadowUdaSb*)NlmCmAllocator__calloc(alloc_p, 
            udaSbRange->m_endSBNum - udaSbRange->m_stSBNum + 1, sizeof(NlmDevShadowUdaSb));

        if(self->m_udaSbMem_p== NULL)
        {
            NlmTblMgr__pvt_dtor(self);
            return NULL;
        }   
    }   

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

    /* Destroy the shadow memory for UDA SB  */
    if(self->m_udaSbMem_p)
        NlmCmAllocator__free(self->m_alloc_p, self->m_udaSbMem_p);
    
}




NlmErrNum_t
NlmTblMgr__pvt_ConfigOpCodeExt(
    NlmGenericTblMgr  *genericTblMgr_p,
    NlmPortNum        portNum,
    nlm_u8            ltrNum,
    NlmGenericTblSearchAttributes  *searchAttrs_p,
    NlmReasonCode *o_reason_p)
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmDev *dev_p = NULL; 
    NlmDevShadowLtr* shadowLtr_p = NULL;
    nlm_u16 devNum = 0, i = 0;
    
    NlmDevMgr *devMgr_p = (NlmDevMgr*)((NlmTblMgr* )genericTblMgr_p->m_tblMgr_p)->m_devMgr_p;
    NlmGenericTblList *gtmTblListHead_p =  ((NlmGenericTblList*)(genericTblMgr_p->m_genericTbl_list_p));
    NlmGenericTblList *gtmTblListNode_p = NULL;

    nlm_u8 resultPortNum = 0;

    for(devNum = 0; devNum < devMgr_p->m_devCount; ++devNum)
    {
        dev_p = (NlmDev*)devMgr_p->m_devList_pp[devNum];
        
        for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; i++)
        {
            gtmTblListNode_p = NlmGenericTblList__Search(gtmTblListHead_p,
                                     searchAttrs_p->m_psInfo[i].m_tblId);   
            
            shadowLtr_p = ((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_ltr;

            resultPortNum =  searchAttrs_p->m_psInfo[i].m_rsltPortNum;

            if(gtmTblListNode_p->m_gt_p->m_adWidth != NLM_TBL_ADLEN_ZERO)
            {
                shadowLtr_p[ltrNum].m_opCodeExt.m_resultType[resultPortNum] = NLMDEV_INDEX_AND_AD;
                shadowLtr_p[ltrNum].m_opCodeExt.m_ADLen[resultPortNum] = 
                            (NlmDevADLength)(gtmTblListNode_p->m_gt_p->m_adWidth - 1);
            }
            else
            {
                shadowLtr_p[ltrNum].m_opCodeExt.m_resultType[resultPortNum] = NLMDEV_INDEX_ONLY;
            }
            
        }

#ifndef NLM_12K_11K         
        dev_p =  ((NlmDev*)devMgr_p->m_devList_pp[devNum]);

        errNum = kbp_dm_ltr_write(dev_p, (nlm_u8)portNum,
                                        ltrNum , NLMDEV_OPCODE_EXT_LTR,
                                        &(shadowLtr_p[ltrNum].m_opCodeExt),
                                        o_reason_p);

        if(errNum != NLMERR_OK)
            return errNum;
#else
        (void)portNum;
        (void)o_reason_p;

#endif

    }

    return errNum;
}


/*
Function: NlmTblMgr__ConfigSearch
Parameters:
        NlmGenericTblMgr  *genericTblMgr_p,
        NlmPortNum        portNum,
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
    NlmPortNum          portNum,
    nlm_u8             ltrNum, 
    NlmGenericTblSearchAttributes  *searchAttrs_p,
    NlmReasonCode    *o_reason
    )
{
    NlmTblMgr* tblmgr_p = NULL;
    nlm_u32 i = 0;
    NlmSearchAttributesNode *curNode_p = NULL;
    NlmGenericTblList *gtmTblListHead_p =  ((NlmGenericTblList*)(genericTblMgr_p->m_genericTbl_list_p));
    NlmGenericTblList *gtmTblListNode_p = NULL;
    NlmDevMgr *devMgr_p = (NlmDevMgr*)((NlmTblMgr* )genericTblMgr_p->m_tblMgr_p)->m_devMgr_p;
    NlmReasonCode dummyReasonCode; 

    nlm_32 remainingAdLenPerSmt = NLM_MAX_SRAM_SB_WIDTH_IN_BITS;

    if(!o_reason)
        o_reason = &dummyReasonCode;

    if(!searchAttrs_p)
    {
         *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
        return NLMERR_NULL_PTR;
    }

#ifndef NLM_12K_11K
    if(ltrNum >=NLMDEV_MAX_NUM_LTRS)
    {
         *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }
#else
    if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
    {
         *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }

    if(searchAttrs_p->m_isCmp3Search)
    {
         *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
        return NLMERR_FAIL;
    }
#endif  

    /* single bank mode */
    if(devMgr_p->m_smtMode == NLMDEV_NO_SMT_MODE ||
        searchAttrs_p->m_isCmp3Search)
    {
        remainingAdLenPerSmt = 2*NLM_MAX_SRAM_SB_WIDTH_IN_BITS;
        
        /* sinlge bank mode max 4 parallel searches are avaialable */
        if(searchAttrs_p->m_numOfParallelSrches > NLMDEV_NUM_PARALLEL_SEARCHES)
        {
            *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
            return NLMERR_FAIL;
        }

        for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; ++i)
        {
            if( searchAttrs_p->m_psInfo[i].m_keyNum > NLMDEV_KEY_3)
            {
                *o_reason = NLMRSC_INVALID_KEY_NUM;
                return NLMERR_FAIL;
            }
            if( searchAttrs_p->m_psInfo[i].m_rsltPortNum > NLMDEV_PARALLEL_SEARCH_3)
            {
                *o_reason = NLMRSC_INVALID_KEY_NUM;
                return NLMERR_FAIL;
            }
        }       
    }
    else /* NLMDEV_DUAL_SMT_MODE */
    {
        remainingAdLenPerSmt = NLM_MAX_SRAM_SB_WIDTH_IN_BITS;
        
        /* dual bank mode only 2 parallel searches are avaialable */
        if(searchAttrs_p->m_numOfParallelSrches > 2)
        {
            *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
            return NLMERR_FAIL;
        }

        /* check for the bank-0 and bank-1 LTR's */
        if(genericTblMgr_p->m_bankType == NLMDEV_BANK_0)
        {
            /* check LTRS: 0-63 */
            if( (ltrNum > ((NLMDEV_MAX_NUM_LTRS/2) - 1)) )
            {
                *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
                return NLMERR_FAIL;
            }

            for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; ++i)
            {
                /* check for the result ports and the key numbers: 0 and 1*/
                if(searchAttrs_p->m_psInfo[i].m_keyNum > NLMDEV_KEY_1)
                {
                    *o_reason = NLMRSC_INVALID_KEY_NUM;
                    return NLMERR_FAIL;
                } 

                if( searchAttrs_p->m_psInfo[i].m_rsltPortNum > NLMDEV_PARALLEL_SEARCH_1)
                {
                    *o_reason = NLMRSC_INVALID_RESULT_SEQ_NUM;
                    return NLMERR_FAIL;
                }
            }
        }
        else
        {
            /* check LTRS: 64-127 */
            if( (ltrNum < (NLMDEV_MAX_NUM_LTRS/2)) || (ltrNum > (NLMDEV_MAX_NUM_LTRS - 1)) )
            {
                *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
                return NLMERR_FAIL;
            }

            for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; ++i)
            {
                /* check for the result ports and the key numbers: 2 and 3*/
                if(searchAttrs_p->m_psInfo[i].m_keyNum < NLMDEV_KEY_2 || 
                    searchAttrs_p->m_psInfo[i].m_keyNum > NLMDEV_KEY_3)
                {
                    *o_reason = NLMRSC_INVALID_KEY_NUM;
                    return NLMERR_FAIL;
                } 

                if( searchAttrs_p->m_psInfo[i].m_rsltPortNum < NLMDEV_PARALLEL_SEARCH_2 ||
                    searchAttrs_p->m_psInfo[i].m_rsltPortNum > NLMDEV_PARALLEL_SEARCH_3 )
                {
                    *o_reason = NLMRSC_INVALID_RESULT_SEQ_NUM;
                    return NLMERR_FAIL;
                }
            }
        } /* else */

    } /* else  NLMDEV_DUAL_SMT_MODE */

    for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; ++i)
    {
        gtmTblListNode_p =  NlmGenericTblList__Search(gtmTblListHead_p,
                                     searchAttrs_p->m_psInfo[i].m_tblId );
        
        if(gtmTblListNode_p == NULL)
        {
            /*The table does not exist in the list containing all tables added to the GTM */
            *o_reason = NLMRSC_INVALID_INPUT;
            return NLMERR_FAIL;
        }

        if(remainingAdLenPerSmt <    ((1 << (gtmTblListNode_p->m_gt_p->m_adWidth -1)) * 32))
        {
            /* Remaining AD width in result is not sufficient for this search  */
            *o_reason = NLMRSC_INVALID_TABLE_ASSO_WIDTH;
            return NLMERR_FAIL;
        }
        else
        {
            remainingAdLenPerSmt -= ((1 << (gtmTblListNode_p->m_gt_p->m_adWidth -1)) * 32);
        }
    }
    
    tblmgr_p = (NlmTblMgr*)(genericTblMgr_p->m_tblMgr_p);

    for(i = 0; i < searchAttrs_p->m_numOfParallelSrches; ++i)
    {
        gtmTblListNode_p = NlmGenericTblList__Search(gtmTblListHead_p,
                                     searchAttrs_p->m_psInfo[i].m_tblId );

        if(gtmTblListNode_p)
            gtmTblListNode_p->m_gt_p->m_isLtrConfigured  = NlmTrue;

        /* search attribute node with table_ID */
        curNode_p = NlmSearchAttributeNode__Find(
                                    tblmgr_p->m_searchAttrList,
                                    searchAttrs_p->m_psInfo[i].m_tblId);


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
                                        searchAttrs_p->m_psInfo[i].m_tblId,
                                        ltrNum,
                                        searchAttrs_p->m_psInfo[i].m_rsltPortNum,
                                        searchAttrs_p->m_psInfo[i].m_keyNum,
                                        &(searchAttrs_p->m_psInfo[i].m_kcm)
                                        );
        }

    }

    /* write Number of parallel seaches to Ext Cap-0 reg here */
    {
        nlm_u32 errNum = 0;
        NlmDevShadowDevice *shadow_p = NULL;
        
        for(i = 0; i < devMgr_p->m_devCount; i++)
        {
            shadow_p = NLM_GET_SHADOW_MEM_FROM_DEVMGR_PTR(devMgr_p, i);

            /* here we are ADDing beacause if GTM writes 1 PS and FTM also writes 1 PS,
               the FTM will over write the GTM value, so will add to the previous value */
            shadow_p->m_ltr[ltrNum].m_extCap0.m_numOfValidSrchRslts  = 
                        (nlm_u8) (shadow_p->m_ltr[ltrNum].m_extCap0.m_numOfValidSrchRslts  + 
                                    searchAttrs_p->m_numOfParallelSrches);

            /*  valid results value        means
                           0               4 results valid
                           1               1 results valid
                           2               2 results valid
                           3               3 results valid
            */
            shadow_p->m_ltr[ltrNum].m_extCap0.m_numOfValidSrchRslts = 
                (shadow_p->m_ltr[ltrNum].m_extCap0.m_numOfValidSrchRslts % NLMDEV_NUM_PARALLEL_SEARCHES);
            
            if((errNum = kbp_dm_ltr_write((NlmDev*)(devMgr_p->m_devList_pp[i]), 
                                (nlm_u8)portNum, (nlm_u8)ltrNum, 
                NLMDEV_EXT_CAPABILITY_REG_0_LTR, &(shadow_p->m_ltr[ltrNum].m_extCap0), o_reason)) != NLMERR_OK )
            {                   
                *o_reason = NLMRSC_INVALID_RESULT_SEQ_NUM;
                return NLMERR_FAIL;
            }
        }
    }

    return NlmTblMgr__pvt_ConfigOpCodeExt(genericTblMgr_p, portNum,
                (nlm_u8)ltrNum, searchAttrs_p, o_reason);

}


/*
Function: NlmTblMgr__LockConfiguration
Parameters:
        NlmGenericTblMgr  *genericTblMgr_p,
        NlmPortNum        portNum,
        NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: It actually sets the BMR to be used for search operations.
Currently only BMR 7 (NO BMR) is being used. Then it writes the key construction 
register. Configuration will be locked by GTM when this function returns.
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t  NlmTblMgr__LockConfiguration (
        NlmGenericTblMgr  *genericTblMgr_p,
        NlmPortNum          portNum,
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

    
    err = NlmTblMgr__pvt_WriteKcr((NlmTblMgr*)(genericTblMgr_p->m_tblMgr_p), portNum, o_reason);
    
    return err;
}

nlm_u32
NlmTblMgr__pvt_GetUdaAddress(
    nlm_u32 dbaRecIndex,
    nlm_u32 maxNumEntriesPerAB, 
    NlmDevBlockConfigReg * blockCfgReg
    )
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



NlmErrNum_t
NlmTblMgr__GetAdInfo(
	NlmGenericTblMgr  *genericTblMgr_p,
	NlmTblAssoDataWidth adWidth,
    NlmRecordIndex	  recordIndex,
	NlmGenericTblAdInfoReadType readType,
	NlmGenericTblAdInfo *adInfo,
	NlmReasonCode *o_reason
	)
{
	nlm_u8 devNum = 0;
	nlm_u32 errNum = 0,entryIdx = 0,abNum,udaAddr,numEntriesToCopy,i;
	nlm_u32 adOffset = 0;
	nlm_u32 sbNr = 0, blkNrInSB = 0, rowNr = 0;
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmDevShadowAB* shadowAB_p = NULL;
	NlmTblMgr *tblMgr_p = (NlmTblMgr*)genericTblMgr_p->m_tblMgr_p;
    NlmDevMgr* devMgr_p = tblMgr_p->m_devMgr_p;

	devNum = (nlm_u8)(recordIndex / (devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));

	/* retrieving device list */
    devList_pp = (NlmDev**)devMgr_p->m_devList_pp;
    
    /* retrieving shadow device */
    shadow_p = (NlmDevShadowDevice*)devList_pp[devNum]->m_shadowDevice_p;

	entryIdx = recordIndex - (devNum*(devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));

	abNum = (nlm_u8)(entryIdx /NLMDEV_AB_DEPTH);

    /* retrieving the same AB in shadow device */
    shadowAB_p = &(shadow_p->m_arrayBlock[abNum]);

	udaAddr = NlmTblMgr__pvt_GetUdaAddress(recordIndex, 
                							NLMDEV_AB_DEPTH, 
                							&shadowAB_p->m_blkConfig);
	 
	/*Assign uda address here to output*/
	adInfo->m_adIndex = udaAddr;

	numEntriesToCopy = 1 << (adWidth - 1);

	adOffset = (numEntriesToCopy * NLM_MIN_SRAM_WIDTH_IN_BYTES);

	/*calculate  SB & block & row number */
	sbNr = udaAddr / (NLMDEV_NUM_SRAM_BLOCKS_IN_SB * NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK); 
	sbNr = sbNr - genericTblMgr_p->m_udaSBRange[devNum].m_stSBNum;
        
	blkNrInSB = udaAddr % NLMDEV_NUM_SRAM_BLOCKS_IN_SB;

	rowNr = (udaAddr / NLMDEV_NUM_SRAM_BLOCKS_IN_SB) % NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK;

	if(readType == GTM_AD_READ_FROM_SW)
	{
		/*Copy data to output */
		for(i = 0; i < numEntriesToCopy; ++i)
		{
			adOffset -= NLM_MIN_SRAM_WIDTH_IN_BYTES;

			NlmCm__memcpy(&adInfo->m_adData[adOffset],
							tblMgr_p->m_udaSbMem_p[sbNr].m_udaBlk[blkNrInSB].m_entry[rowNr].m_data,
							NLM_MIN_SRAM_WIDTH_IN_BYTES
							);
			
			++blkNrInSB;
		}
	}
	else
	{
		nlm_u8 adData[10] = {0,};

		for(i = 0; i < numEntriesToCopy; ++i)
		{
			adOffset -= NLM_MIN_SRAM_WIDTH_IN_BYTES;

			/* Read from Device */
			errNum = kbp_dm_uda_read(devMgr_p->m_devList_pp[devNum],
							NLMDEV_PORT_0,
							udaAddr,
							adData,
							NLM_MIN_SRAM_WIDTH_IN_BYTES,/* ignored with 12K */
							o_reason);

			if(errNum != NLMERR_OK)
				return NLMERR_FAIL;
			
			/*copy data to output */
			NlmCm__memcpy(&adInfo->m_adData[adOffset],&adData[6],NLM_MIN_SRAM_WIDTH_IN_BYTES);

			udaAddr++;
		}
	}
	
	return NLMERR_OK;
}

/*
Function: NlmTblMgr__AddRecord
Parameters:
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8              *assocData,
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
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8              *assocData,
    nlm_u16             recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    nlm_u16             groupId,
    NlmReasonCode       *o_reason
    )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;
    NlmBool isShuffleDown = NlmFalse;
    NlmReasonCode dummyReasonCode; 

    if(!o_reason)
        o_reason = &dummyReasonCode;
    if(!tblRecord)
    {
        *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_NULL_PTR;
    }
    if(!o_recordIndex)
    {
      *o_reason = NLMRSC_INVALID_OUTPUT;
        return NLMERR_NULL_PTR;
    }

    if(!genericTbl->m_isLtrConfigured)
    {
        *o_reason = NLMRSC_NO_LTR_CONFIGURED;
        return NLMERR_FAIL;
    }

    self = (NlmTblMgr*)(genericTbl->m_genericTblMgr_p->m_tblMgr_p);

    self->m_portNumForCurOpr = portNum;

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
        err = NlmTblMgr__pvt_InvalidateRecord(self->m_devMgr_p,
                        portNum, tblRecord->m_len,
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

    return NlmTblMgr__pvt_WriteRecord(genericTbl, portNum, tblRecord,
                    assocData, *o_recordIndex, NlmTrue, o_reason);

}
/*
Function Get_enum
Parameters:
nlm_u32 len
Return type: NlmDevBlockWidth
Description: Gives the enum value corresponding to the record length.
*/
NlmDevBlockWidth NlmTblMgr__pvt_GetBlkWidth(nlm_u32 len)
{
	if (len == 80)
		return NLMDEV_BLK_WIDTH_80;
	else if (len == 160)
		return NLMDEV_BLK_WIDTH_160;
	else if (len == 320)
		return NLMDEV_BLK_WIDTH_320;
	else
		return NLMDEV_BLK_WIDTH_640;
}
/* 
Function: NlmTblMgr__FindRecord
Parameters:
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
Return Type: NlmErrNum_t
 * This function finds a record indentified by tblRecord in the Shadow Memory.
 * It is necessary to provide the tblRecord in D-M format.
 * Index associated with the record is given as output in 'o_recordIndex'
 * Return value of 0 indicates success, and anything else is failure.
 */
NlmErrNum_t NlmTblMgr__FindRecord(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    )
{
    NlmTblMgr *self = NULL;
    NlmReasonCode dummyReasonCode;
	NlmDevShadowDevice* shadow_dev;
	NlmDevShadowAB*     shadow_dev_arrayblk;
	nlm_u32 dev_id=0,blk_num=0,row_num=0,flag_mismatch=0,gtm_start_blk=0,gtm_end_blk=0;
	nlm_u32 entryNum = 0,numOfEntries=0;
	nlm_u16 byteNum = 0;
	nlm_u8 data[80]={0,},mask[80] = {0,};
	NlmDevBlockWidth record_width;
	NlmBool isValidTbl = 0;


    if(!o_reason)
        o_reason = &dummyReasonCode;
    if(!tblRecord)
    {
        *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_NULL_PTR;
    }
    if(!o_recordIndex)
    {
      *o_reason = NLMRSC_INVALID_OUTPUT;
        return NLMERR_NULL_PTR;
    }

	*o_recordIndex= 0xFFFFFFFF;
    /* Convert to X-Y mode */ 

	for(byteNum = 0; byteNum < (tblRecord->m_len/8); byteNum++)
    {
		data[byteNum] = tblRecord->m_data[byteNum] & (~tblRecord->m_mask[byteNum]);
		mask[byteNum] = (~tblRecord->m_data[byteNum]) & (~tblRecord->m_mask[byteNum]);
    }
	
    self = (NlmTblMgr*)(genericTbl->m_genericTblMgr_p->m_tblMgr_p);
	/* Loop : for each device */
	for(dev_id = 0;dev_id<self->m_devMgr_p->m_devCount;dev_id++)
	{
		gtm_start_blk=genericTbl->m_genericTblMgr_p->m_genericTblBlksRange[dev_id].m_startBlkNum;/* GTM start Block */
		gtm_end_blk=genericTbl->m_genericTblMgr_p->m_genericTblBlksRange[dev_id].m_endBlkNum; /* GTM end Block */
		shadow_dev = NLM_GET_SHADOW_MEM_FROM_DEVMGR_PTR(self->m_devMgr_p,dev_id);
		/* Loop : for each Array Blocks */
		for(blk_num= gtm_start_blk ;blk_num <=gtm_end_blk;blk_num++)
		{
			isValidTbl = NlmBlkMemMgr_check_tblId(self->m_blockMemMgr,(nlm_u16)blk_num,genericTbl->m_tblId);

			if(isValidTbl == 0)
				continue;

			shadow_dev_arrayblk=&(shadow_dev)->m_arrayBlock[blk_num];

			/* Checking if Block is Enabled */
			if(shadow_dev_arrayblk->m_blkConfig.m_blockEnable)
			{
				record_width = NlmTblMgr__pvt_GetBlkWidth(tblRecord->m_len);

				/* Checking Match for record width */
 				if(shadow_dev_arrayblk->m_blkConfig.m_blockWidth == record_width)
				{ 
					/* Loop : for each row */
					for(row_num=0;row_num<NLMDEV_AB_DEPTH;row_num += (tblRecord->m_len/80))
					{  
						/* Checking Valid bit*/
						if(shadow_dev_arrayblk->m_abEntry[row_num+(tblRecord->m_len/80)-1].m_vbit)
						{
							flag_mismatch = 0;

							/* Matching Each AB entry of the record */
							numOfEntries = (tblRecord->m_len/80);
							for(entryNum = 0; entryNum < numOfEntries  ; entryNum++)
							{
								
								if(NlmCm__memcmp(&data[entryNum*10] , shadow_dev_arrayblk->m_abEntry[row_num+numOfEntries-entryNum-1].m_data, 10))
								{
									flag_mismatch=1;
									break;
								}
								if(NlmCm__memcmp(&mask[entryNum*10] ,shadow_dev_arrayblk->m_abEntry[row_num+numOfEntries-entryNum-1].m_mask, 10))
								{
									flag_mismatch=1;
									break;
								}
							}
							if(!flag_mismatch)
							{
								*o_recordIndex = (nlm_u32) ((dev_id << 23) | (blk_num << 12) | row_num);
								*o_reason =NLMRSC_REASON_OK;
								return NLMERR_OK;
							}
								
						}
					}/* Loop End : for each row */
				}
			}
		}/* Loop End : for each Array Blocks */
	}/* Loop End : for each device */
	*o_reason=NLMRSC_RECORD_NOTFOUND;
	return NLMERR_OK;

}

/*
Function: NlmTblMgr__UpdateRecord
Parameters:
    NlmGenericTbl           *genericTbl,    
    NlmPortNum              portNum,
    NlmGenericTblRecord     *tblRecord,
    NlmRecordIndex          recordIndex,
    nlm_u8                  *assocData,
    NlmReasonCode           *o_reason
Return Type: NlmErrNum_t
Description: This function updates a record, at recordIndex, to a table identified with 'genericTbl'.
It first validates the input record at specified index and then it calls EraseRecord followed by 
Write_Record which writes the table record into the device. Return value of 0 indicates success, 
and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__UpdateRecord(
    NlmGenericTbl       *genericTbl, 
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8              *assocData,
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

    self->m_portNumForCurOpr = portNum;
        
    err = NlmBlkMemMgr__ValidateRecord(self->m_blockMemMgr,  
                                        genericTbl, recordIndex, o_reason);
    if(NLMERR_OK !=err)
        return err;

    err = NlmTblMgr__pvt_EraseRecord(genericTbl, 
                    portNum, genericTbl->m_width, recordIndex, o_reason);
    if(NLMERR_OK !=err)
        return err;

    return NlmTblMgr__pvt_WriteRecord(genericTbl, portNum, tblRecord, 
                    assocData, recordIndex, NlmTrue, o_reason);
}



NlmErrNum_t NlmTblMgr__iter_next(
    NlmGenericTblIterHandle *iterHandle,
    NlmGenericTblIterData  *iterInfo)
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;
    
    NlmTblMgr *tblMgr_p = (NlmTblMgr*)iterHandle->m_tbl_p->m_genericTblMgr_p->m_tblMgr_p;
    NlmDevMgr *devmgr_p = (NlmDevMgr*)tblMgr_p->m_devMgr_p;
    NlmDev** devList_pp = (NlmDev**)devmgr_p->m_devList_pp;

    /* Get the shadow memory data */
    NlmDevShadowDevice *shadowdevice = (NlmDevShadowDevice*)devList_pp[0]->m_shadowDevice_p;
    NlmDevShadowUdaSb *shadowUDA_p = (NlmDevShadowUdaSb*)tblMgr_p->m_udaSbMem_p;

    /* Get the Table manager pointer */
    self = (NlmTblMgr *)iterHandle->m_tbl_p->m_genericTblMgr_p->m_tblMgr_p;

    /* call the block memory manager module*/
    err = NlmBlkMemMgr_iter_next(self->m_blockMemMgr,iterInfo,iterHandle,shadowdevice,shadowUDA_p);
    
    return err;
}


/*
Function: NlmTblMgr__DeleteRecord
Parameters:
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
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
    NlmPortNum          portNum,
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

    self->m_portNumForCurOpr = portNum;
        
    err = NlmBlkMemMgr__DeleteRecord(self->m_blockMemMgr,
                                    genericTbl, recordIndex, o_reason);

    if(NLMERR_OK !=err)
        return err;

    err = NlmTblMgr__pvt_EraseRecord(genericTbl, 
        portNum, genericTbl->m_width, recordIndex, o_reason);

    return err;
}

/*
Function: NlmTblMgr__Destroy
Parameters:
            NlmGenericTblMgr *genericTblMgr_p,  
            NlmPortNum        portNum,
            NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: It calls the destructor of TM to free all the memory held by it
and destroys TM. Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t  NlmTblMgr__Destroy(
    NlmGenericTblMgr *genericTblMgr_p,  
    NlmPortNum       portNum,
    NlmReasonCode    *o_reason
        )
{
    NlmCmAllocator *alloc_p = NULL;
    NlmTblMgr* self_p = (NlmTblMgr*)genericTblMgr_p->m_tblMgr_p;

    self_p->m_portNumForCurOpr = portNum;


    alloc_p = self_p->m_alloc_p;
    NlmTblMgr__pvt_dtor((NlmTblMgr*)genericTblMgr_p->m_tblMgr_p);    

    if(genericTblMgr_p->m_genericTblBlksRange)
    {
        NlmCmAllocator__free(self_p->m_alloc_p, genericTblMgr_p->m_genericTblBlksRange);
        genericTblMgr_p->m_genericTblBlksRange = NULL;
    }
    
    if(genericTblMgr_p->m_udaSBRange)
    {
        NlmCmAllocator__free(self_p->m_alloc_p, genericTblMgr_p->m_udaSBRange);
        genericTblMgr_p->m_udaSBRange = NULL;
    }
    NlmCmAllocator__free(self_p->m_alloc_p, self_p);
    
    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
}

/*
Function: NlmTblMgr__CreateTable
Parameters:
            NlmGenericTblMgr  *genericTblMgr_p,
            NlmPortNum        portNum,
            NlmGenericTbl* table_p,
            NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: It increases the table count by one and calls block memory 
manager to initialize the table related data structures. Return value of 0 
indicates success, and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__CreateTable(
                NlmGenericTblMgr  *genericTblMgr_p,
                NlmPortNum          portNum,
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

    self->m_portNumForCurOpr = portNum;

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
                NlmPortNum        portNum,
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
                NlmPortNum        portNum,
                NlmGenericTbl     *genericTbl,
                NlmReasonCode     *o_reason
                )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;
    
    self = (NlmTblMgr*)genericTblMgr_p->m_tblMgr_p;

    self->m_portNumForCurOpr = portNum;

    err =  NlmBlkMemMgr__DestroyTbl(self->m_blockMemMgr, genericTbl, o_reason);

    if(NLMERR_OK !=err)
        return err;

    self->m_tableCount--;

    NlmSearchAttributeNode__Delete(self->m_alloc_p, 
                                                &(self->m_searchAttrList), 
                                                genericTbl->m_tblId);
    return err;
}

/*
Function: NlmTblMgr__Init
Parameters:
        NlmGenericTblMgr            *genericTableMgr_p,
        NlmCmAllocator              *alloc_p,
        void*                       devMgr_p, 
        nlm_u8                      numOfDevices,
        NlmGenericTblMgrBlksRange   *gtmBlksRange,
        NlmGenericTblMgrSBRange     *udaSbRange,
        NlmIndexChangedAppCb        indexChangedAppCb,
        void*                       client_p,
        NlmReasonCode               *o_reason
Return Type: NlmErrNum_t
Description: The function allocates memory for TM and initializes its content
by calling Table Manager constructor. It initializes the virtual table by
assigning all function pointers which are called from GTM APIs. Return value of
0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
NlmTblMgr__Init(
          NlmGenericTblMgr* genericTableMgr_p,
          NlmCmAllocator                *alloc_p,
          void*                         devMgr_p, 
          nlm_u8                        numOfDevices,
          NlmGenericTblMgrBlksRange     *gtmBlksRange,
          NlmGenericTblMgrSBRange       *udaSbRange,
          NlmIndexChangedAppCb          indexChangedAppCb,
          void*                         client_p,
          NlmReasonCode                 *o_reason
          )
{
    NlmTblMgr *self = NULL;
    NlmTblMgr__pvt_vtbl* vTable = NULL;
    nlm_u8 devNum = 0;
    nlm_u16 NumOfAB;

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
        return NLMERR_FAIL;
    }
    
    if(gtmBlksRange == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE;
        return NLMERR_FAIL;
    }

    if(numOfDevices > ((NlmDevMgr*)devMgr_p)->m_devCount)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_NUM_OF_DEVICES;
        return NLMERR_FAIL;
    }

    NumOfAB = ((NlmDevMgr*)devMgr_p)->m_numOfAbs;

    if((genericTableMgr_p->m_genericTblBlksRange = (NlmGenericTblMgrBlksRange*)NlmCmAllocator__calloc(alloc_p, 
        ((NlmDevMgr*)devMgr_p)->m_devCount, sizeof(NlmGenericTblMgrBlksRange))) == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }    

     if((genericTableMgr_p->m_udaSBRange = (NlmGenericTblMgrSBRange*)NlmCmAllocator__calloc(alloc_p, 
        ((NlmDevMgr*)devMgr_p)->m_devCount, sizeof(NlmGenericTblMgrSBRange))) == NULL)
    {
         NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
         
        if(o_reason)
            *o_reason = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }    

    for(devNum = 0; devNum < numOfDevices; devNum++)
    {      
     genericTableMgr_p->m_udaSBRange[devNum].m_stSBNum = udaSbRange[devNum].m_stSBNum;
     genericTableMgr_p->m_udaSBRange[devNum].m_endSBNum = udaSbRange[devNum].m_endSBNum;
            
        if(gtmBlksRange[devNum].m_startBlkNum == NLM_GTM_BLK_RANGE_NO_BLK_USED
                || gtmBlksRange[devNum].m_endBlkNum == NLM_GTM_BLK_RANGE_NO_BLK_USED)
        {
            genericTableMgr_p->m_genericTblBlksRange[devNum].m_startBlkNum = NLM_GTM_BLK_RANGE_NO_BLK_USED;
            genericTableMgr_p->m_genericTblBlksRange[devNum].m_endBlkNum = NLM_GTM_BLK_RANGE_NO_BLK_USED; 
            continue;
        }
        
        /* check the block ranges here */
        {
            /* If start blk num = NLM_GTM_BLK_RANGE_WHOLE_DEVICE, whole device is
            available for GTM Tables */
            if(gtmBlksRange[devNum].m_startBlkNum == NumOfAB
                || gtmBlksRange[devNum].m_endBlkNum == NumOfAB)
            {
                genericTableMgr_p->m_genericTblBlksRange[devNum].m_startBlkNum = 0;
                genericTableMgr_p->m_genericTblBlksRange[devNum].m_endBlkNum = (((NlmDevMgr*)devMgr_p)->m_numOfAbs - 1); 
                continue;
            }           

            /* start blk number should be less than or equal to end blk number;
            also both the start blk number and end blk number should be less than
            or equal to total number of blocks */
            if(gtmBlksRange[devNum].m_startBlkNum > gtmBlksRange[devNum].m_endBlkNum
                || gtmBlksRange[devNum].m_startBlkNum >= ((NlmDevMgr*)devMgr_p)->m_numOfAbs 
                || gtmBlksRange[devNum].m_endBlkNum >= ((NlmDevMgr*)devMgr_p)->m_numOfAbs)
            {
                NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
          NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_udaSBRange);
                *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE; 
                return NLMERR_FAIL;        
            }
        }


#ifndef NLM_12K_11K
        /* Block range should be at super block boundary */
        if((gtmBlksRange[devNum].m_startBlkNum % (((NlmDevMgr*)devMgr_p)->m_numOfAbs/NLMDEV_NUM_SUPER_BLOCKS)) || 
            ((gtmBlksRange[devNum].m_endBlkNum+1) % (((NlmDevMgr*)devMgr_p)->m_numOfAbs/NLMDEV_NUM_SUPER_BLOCKS)) )
        {
            NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
            NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_udaSBRange);
            *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE;
            return NLMERR_FAIL;             
        }   

#else
            /* Block range should be at super block boundary */
            if((gtmBlksRange[devNum].m_startBlkNum % (NLM11KDEV_NUM_ARRAY_BLOCKS/NLMDEV_NUM_SUPER_BLOCKS)) || 
               ((gtmBlksRange[devNum].m_endBlkNum+1) % (NLM11KDEV_NUM_ARRAY_BLOCKS/NLMDEV_NUM_SUPER_BLOCKS)) )
            {
                NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
             NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_udaSBRange);
                *o_reason = NLMRSC_INVALID_GTM_BLKS_RANGE;
                return NLMERR_FAIL;             
            }   
#endif

        

        genericTableMgr_p->m_genericTblBlksRange[devNum].m_startBlkNum = gtmBlksRange[devNum].m_startBlkNum;
        genericTableMgr_p->m_genericTblBlksRange[devNum].m_endBlkNum = gtmBlksRange[devNum].m_endBlkNum;
    }   



    /*Allocate and Initialize NlmTblMgr*/
    self = (NlmTblMgr*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmTblMgr));
    if(self == NULL)    
    {
    NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_genericTblBlksRange);
    NlmCmAllocator__free(alloc_p, genericTableMgr_p->m_udaSBRange);

    genericTableMgr_p->m_tblMgr_p = self;

    if(o_reason)
            *o_reason = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }

    self = NlmTblMgr__pvt_ctor(self, alloc_p, (NlmDevMgr*)devMgr_p, genericTableMgr_p->m_genericTblBlksRange, 
                     genericTableMgr_p->m_udaSBRange, indexChangedAppCb);
    
    genericTableMgr_p->m_tblMgr_p = self;

    self->m_client_p = client_p;

    /* assigning vTable */
    vTable = ((NlmTblMgr__pvt_vtbl*)(genericTableMgr_p->m_vtbl_p));
    vTable->AddRecord         = NlmTblMgr__AddRecord;
	vTable->FindRecord        = NlmTblMgr__FindRecord;
    vTable->ConfigSearch      = NlmTblMgr__ConfigSearch;
    vTable->CreateTable       = NlmTblMgr__CreateTable;
    vTable->DeleteRecord      = NlmTblMgr__DeleteRecord;
    vTable->UpdateRecord      = NlmTblMgr__UpdateRecord;
    vTable->Destroy           = NlmTblMgr__Destroy;
    vTable->DestroyTable      = NlmTblMgr__DestroyTable;
    vTable->LockConfiguration = NlmTblMgr__LockConfiguration;
    vTable->iter_next         = NlmTblMgr__iter_next;
	vTable->GetAdInfo         = NlmTblMgr__GetAdInfo;
    /* Application is responsible for configuring GCR */

    return NLMERR_OK;
}

/*=====================================================================================*/

/*====== Linked list of search_attribute_node specific functions ======*/

/*
Function: NlmSearchAttributeNode__Find
Parameters:
        NlmSearchAttributesNode* SearchAttrList_p,
        nlm_u8 TableId
Return Type: nlm_32
Description: It finds the index in the link list of search_attribute_node
associated with table_ID and return the index. If it doesn't find it returns
NOFOUND(-1).
*/
NlmSearchAttributesNode* 
NlmSearchAttributeNode__Find(
        NlmSearchAttributesNode* searchAttrHead_p,
        nlm_u8 tableId
        )
{
    nlm_32 compareResult =0;
    NlmSearchAttributesNode* curNode_p = searchAttrHead_p;

    
    while(curNode_p)
    {
    compareResult = curNode_p->m_tblId -  tableId;

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
        nlm_u8  tblId,
        nlm_u8 LtrNum,
        NlmDevParallelSrch ps_RBus,
        NlmDevKey KeyNum,
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
        nlm_u8  tblId,
        nlm_u8 LtrNum,
        NlmDevParallelSrch ps_RBus,
        NlmDevKey KeyNum,
        NlmGenericTblKeyConstructionMap *kcm_p
        )
{
    nlm_u32 idx =0;
    NlmSearchAttributesNode* curNode_p = NULL;
    NlmSearchAttributesNode* previousNode_p = NULL;
    NlmSearchAttributesNode* newNode_p = NULL;
    curNode_p = *searchAttrHead_pp;

    newNode_p = (NlmSearchAttributesNode*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmSearchAttributesNode));
    
    if(!newNode_p)
        return NLMFALSE;

    for(idx =0; idx <NLMDEV_MAX_NUM_LTRS; idx++)
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
    newNode_p->m_tblId = tblId;

    if(!curNode_p)
    {
        *searchAttrHead_pp = newNode_p;
        return NLMTRUE;
    }

    while(curNode_p)
    {
        if((newNode_p->m_tblId < curNode_p->m_tblId))
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

    curNode_p->m_ps_RBus[LtrNum] = (NlmDevParallelSrch)ps_RBus;

    curNode_p->m_keyNum[LtrNum] = (NlmDevKey)keyNum;
        
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
        nlm_u8  tblId
Return Type: NlmBool
Description: It searches and deletes a search_attribute_node based on tbl_ID 
and update search_attribute link list. Returns TRUE on success, NOTFOUND if
no node contains this table_ID and FALSE if something else is wrong.
*/
NlmBool 
NlmSearchAttributeNode__Delete(
        NlmCmAllocator* alloc_p,
        NlmSearchAttributesNode** SearchAttrList_pp,
        nlm_u8  tblId
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
        if(SearchAttrList_p->m_tblId == tblId)
        {
            if(!previous)
                *SearchAttrList_pp = SearchAttrList_p->m_Next_p;
            else
                previous->m_Next_p = SearchAttrList_p->m_Next_p;

            NlmCmAllocator__free(alloc_p, SearchAttrList_p);

            return NLMTRUE;
        }
        if(SearchAttrList_p->m_tblId > tblId)
            return NLMNOTFOUND;

        previous = SearchAttrList_p;
        SearchAttrList_p = SearchAttrList_p->m_Next_p;
    }
    return NLMFALSE;
}
/*=====================================================================================*/

void NlmTblMgr__pvt_getACMapValue(NlmBankNum whichBank,
                                  NlmGenericTblMgrBlksRange *m_blkRng,
                                  NlmDevConfigReg *configData_p
                                  )
{   
    nlm_u32 value = 0;
    nlm_u32 stSbBlk = 0, endSbBlk = 0;
    nlm_u32 shift = 0;

    /* get the superblocks from the block range */
    stSbBlk  = (nlm_u32)(m_blkRng->m_startBlkNum/8);
    endSbBlk = (nlm_u32)((m_blkRng->m_endBlkNum + 1)/8);

    /* There is one bit per AC (AC has 2 SBs) */
    shift    = (nlm_u32)(stSbBlk/2);

    while(stSbBlk < endSbBlk)
    {
        value += whichBank << shift;

        stSbBlk +=2; /* increment the SB by 2 */
        shift++;
    }

    configData_p->m_ACtoBankMapping |= (nlm_u16)value;

    return;
}


/*
Function: NlmTblMgr__pvt_WriteKcr
Parameters:
        NlmTblMgr* self,
        NlmPortNum        portNum,
        NlmReasonCode *o_reason
Return Type: NlmErrNum_t
Description: This function drives the KCR register write operation to device via
Device Manager. It first fetch the content information of KCR register from search
attributes, then formats the content accroding to Device Manager input format,
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t NlmTblMgr__pvt_WriteKcr(
        NlmTblMgr* self,
        NlmPortNum          portNum,
        NlmReasonCode *o_reason
        )
{

    nlm_32 idx =0;
    NlmDevKey keyNum = (NlmDevKey)0;
    NlmDevParallelSrch psNum = (NlmDevParallelSrch)0;
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
        for(idx =0; idx < NLMDEV_MAX_NUM_LTRS; idx++)
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

                        shadow_p->m_ltr[idx].m_keyConstruct[regNr].m_isZeroFill[i] = 
                                                        AttrList_p->m_kcm[idx].m_segmentIsZeroFill[j];
                        
                        if(AttrList_p->m_kcm[idx].m_segmentNumOfBytes[j] == 0)
                        {
#ifndef NLM_12K_11K
                            /* in 12K we fill remaining segments with 0x7f */
                            shadow_p->m_ltr[idx].m_keyConstruct[regNr].m_startByteLoc[i]= 0x7F;
                            shadow_p->m_ltr[idx].m_keyConstruct[regNr].m_numOfBytes[i] = 0;
                            shadow_p->m_ltr[idx].m_keyConstruct[regNr].m_isZeroFill[i] = 0;
#else
                            /* in 11K mode we ignore the remain segments after end of segments*/
                            done = NlmTrue;
                            break;
#endif
                        }

                        ++j;

                    }

                    err = kbp_dm_ltr_write(
                        dev_pp[dev_num], (nlm_u8)portNum, (nlm_u8)idx,
                        (NlmDevLtrRegType)(NLMDEV_KEY_0_KCR_0_LTR + regNr),
                        &(shadow_p->m_ltr[idx].m_keyConstruct[regNr]),
                        o_reason);

                    if(err != NLMERR_OK )
                    {
                        #if defined NLM_MT_OLD || defined NLM_MT
                        NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
                        #endif
                        return err;
                    }   
#ifdef NLM_12K_11K
                    if(done)
                        break;
#endif
                }

                /*BMR is disabled when bmr select value is 7 */
                shadow_p->m_ltr[idx].m_extCap0.m_bmrSelect[psNum] = NLM_NO_MASK_BMR_NUM;

                err = kbp_dm_ltr_write(
                        dev_pp[dev_num], (nlm_u8)portNum, (nlm_u8)idx,
                        NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                        &(shadow_p->m_ltr[idx].m_extCap0),
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


nlm_u32
NlmTblMgr__pvt_GetUdaAddr(
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


NlmErrNum_t
NlmTblMgr__pvt_WriteOrShuffleUdaRecord(
    NlmGenericTbl *generic_p,
    NlmPortNum          portNum,
    nlm_u8          *assocData, /* If Assoc Data is there then it is write otherwise shuffle */
    NlmRecordIndex  recordIndex,
    NlmRecordIndex  oldRecordIndex,
    NlmReasonCode *o_reason_p)
{
    nlm_u32 numEntriesToCopy = 0;
    nlm_u32 i = 0;
    nlm_32 dataIdx = 0, abNum = 0, entryIdx = 0;
    nlm_u32 sbNr = 0, blkNrInSB = 0, rowNr = 0;
    nlm_32 oldAbNum = 0, oldEntryIdx = 0;
    nlm_u32 oldSbNr = 0, oldBlkNrInSB = 0, oldRowNr = 0;
        
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_u32 udaAddr = 0, oldUdaAddr = 0, devNum = 0, oldDevNum = 0;
    NlmTblMgr *tblMgr_p = (NlmTblMgr*)generic_p->m_genericTblMgr_p->m_tblMgr_p;
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmDevShadowAB* shadowAB_p = NULL;

    NlmDevMgr* devMgr_p = tblMgr_p->m_devMgr_p;
    nlm_u8* adData_p = NULL;

    nlm_u8  data[NLMDEV_REG_LEN_IN_BYTES] = {0, };
    
    NlmCmAssert(generic_p->m_adWidth != NLM_TBL_ADLEN_ZERO, "Invalid AD Len");
    
#ifndef NLM_12K_11K 
    devNum = (nlm_u8)(recordIndex / (devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
#else
    devNum = (nlm_u8)(recordIndex / NLM_ALLINDEX_PARDEVICE);
#endif

    /* retrieving device list */
    devList_pp = (NlmDev**)devMgr_p->m_devList_pp;
    
    /* retrieving shadow device */
    shadow_p = (NlmDevShadowDevice*)devList_pp[devNum]->m_shadowDevice_p;


    /* finding index within the device */
#ifndef NLM_12K_11K
    entryIdx = recordIndex - (devNum*(devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
#else
    entryIdx = recordIndex - (devNum*NLM_ALLINDEX_PARDEVICE);
#endif

    abNum = (nlm_u8)(entryIdx /NLMDEV_AB_DEPTH);

    /* retrieving the same AB in shadow device */
    shadowAB_p = &(shadow_p->m_arrayBlock[abNum]);
    

    udaAddr = NlmTblMgr__pvt_GetUdaAddr(recordIndex, 
                NLMDEV_AB_DEPTH, &shadowAB_p->m_blkConfig);

    sbNr = udaAddr / (NLMDEV_NUM_SRAM_BLOCKS_IN_SB * NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK); 
    sbNr = sbNr - generic_p->m_genericTblMgr_p->m_udaSBRange[devNum].m_stSBNum;
        
    blkNrInSB = udaAddr % NLMDEV_NUM_SRAM_BLOCKS_IN_SB;

    rowNr = (udaAddr / NLMDEV_NUM_SRAM_BLOCKS_IN_SB) % NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK;

    if(assocData == NULL)
    {
#ifndef NLM_12K_11K         
        oldDevNum = (nlm_u8)(oldRecordIndex / (devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
#else
        oldDevNum = (nlm_u8)(oldRecordIndex / NLM_ALLINDEX_PARDEVICE);
#endif
        
        /* retrieving shadow device */
        shadow_p = (NlmDevShadowDevice*)devList_pp[oldDevNum]->m_shadowDevice_p;

        /* finding index within the device */
#ifndef NLM_12K_11K
        oldEntryIdx = oldRecordIndex - (oldDevNum*(devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
#else
        oldEntryIdx = oldRecordIndex - (oldDevNum*NLM_ALLINDEX_PARDEVICE);
#endif

        oldAbNum = (nlm_u8)(oldEntryIdx /NLMDEV_AB_DEPTH);
        
        /* retrieving the same AB in shadow device */
        shadowAB_p = &(shadow_p->m_arrayBlock[oldAbNum]);
        
        
        oldUdaAddr = NlmTblMgr__pvt_GetUdaAddr(oldRecordIndex, 
                        NLMDEV_AB_DEPTH, &shadowAB_p->m_blkConfig);
        
        oldSbNr = oldUdaAddr / (NLMDEV_NUM_SRAM_BLOCKS_IN_SB * NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK); 
        oldSbNr = oldSbNr - generic_p->m_genericTblMgr_p->m_udaSBRange[oldDevNum].m_stSBNum;
            
        oldBlkNrInSB = oldUdaAddr % NLMDEV_NUM_SRAM_BLOCKS_IN_SB;
    
        oldRowNr = (oldUdaAddr / NLMDEV_NUM_SRAM_BLOCKS_IN_SB) % NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK;
    }
        
    numEntriesToCopy = 1<< (generic_p->m_adWidth - 1);
        
    /*Copy starting from the least significant (LS) portion*/
    dataIdx = (numEntriesToCopy - 1) * NLM_MIN_SRAM_WIDTH_IN_BYTES;

    for(i = 0; i < numEntriesToCopy; ++i)
    {
        if(blkNrInSB >= NLMDEV_NUM_SRAM_BLOCKS_IN_SB)
        {
            NlmCmAssert(0, "Writing beyond the row in the UDA \n");
            break;
        }
        
        if(assocData)
        {
            adData_p = &assocData[dataIdx];
        }
        else
        {
            adData_p = tblMgr_p->m_udaSbMem_p[oldSbNr].m_udaBlk[oldBlkNrInSB].m_entry[oldRowNr].m_data;
        }

        /* Copy UDa data into UDA Shadow Memory */  
        
        NlmCm__memcpy(tblMgr_p->m_udaSbMem_p[sbNr].m_udaBlk[blkNrInSB].m_entry[rowNr].m_data,
                        adData_p,
                        NLM_MIN_SRAM_WIDTH_IN_BYTES);

        /* PIOWrite is used for UDA writes. PIOWrite is 80b. But only LBS 32b contains
         * valid UDA data.
         */
        NlmCm__memcpy( (data + 6), adData_p, 4 );

        

        /* Write UDA data using dev mgr */      
        errNum = kbp_dm_uda_write(devList_pp[devNum], (nlm_u8) portNum, udaAddr,
                                  data, 4, o_reason_p);
        
        if(errNum != NLMERR_OK)
                return errNum;

        blkNrInSB++;
        oldBlkNrInSB++;
        udaAddr++;
        
        dataIdx -= NLM_MIN_SRAM_WIDTH_IN_BYTES;

    }

    return NLMERR_OK;

}



/*
Function: NlmTblMgr__pvt_WriteRecord
Parameters:
        NlmGenericTbl* genericTbl_p,
        NlmPortNum          portNum,
        NlmGenericTblRecord *tblRecord,
        nlm_u8          *assocData,
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
            NlmGenericTbl *genericTbl_p,
            NlmPortNum          portNum,
            NlmGenericTblRecord *tblRecord,
            nlm_u8          *assocData,
            NlmRecordIndex recordIndex,
            NlmBool convertToXY,
            NlmReasonCode *o_reason_p
            )
{
    nlm_u32 entryIdx =0;
    nlm_u32 wordIdx = 0;
    nlm_u8 dev_num =0;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmDevShadowAB* shadowAB_p = NULL;
    nlm_u8 ab_num = 0;
    nlm_u16 nrOfWords = 0;
    nlm_u32 currentRow = 0;
    NlmTblMgr *tblMgr_p = (NlmTblMgr*)genericTbl_p->m_genericTblMgr_p->m_tblMgr_p;
    NlmDevMgr* devMgr_p = tblMgr_p->m_devMgr_p;
    
    if(!devMgr_p)
    {
        *o_reason_p = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    dev_num = (nlm_u8)(recordIndex / (devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
#else
    dev_num = (nlm_u8)(recordIndex / NLM_ALLINDEX_PARDEVICE);
#endif

    /* retrieving device list */
    devList_pp = (NlmDev**)devMgr_p->m_devList_pp;

    /* finding index within the device */
#ifndef NLM_12K_11K
    entryIdx = recordIndex - (dev_num*(devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
#else
    entryIdx = recordIndex - (dev_num*NLM_ALLINDEX_PARDEVICE);
#endif
    
    ab_num = (nlm_u8)(entryIdx /NLMDEV_AB_DEPTH);

    /* retrieving shadow device */
    shadow_p = (NlmDevShadowDevice*)devList_pp[dev_num]->m_shadowDevice_p;

    /* retrieving the same AB in shadow device */
    shadowAB_p = &(shadow_p->m_arrayBlock[ab_num]);

   /* Write the UDA data if present */
   if(assocData)
   {
    errNum = NlmTblMgr__pvt_WriteOrShuffleUdaRecord(genericTbl_p, 
                portNum, assocData, recordIndex, 0xFFFF, o_reason_p);
   }

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

        /* data and mask are filled as 0xff and in already in XY mode. X/Y as 1/1 is always
         * miss in XY mode. There is no equivalent value set in D-M mode for always miss
         * Thats why, we give the data/mask in XY mode and hence does not need to convert 
         * the data/mask to XY format before writing into the device.
         */

        /* Conversion to XY format is not needed for delete/erase record. */
        if(convertToXY == NlmTrue)
            NlmTblMgr__pvt_convertToXYMode(&(shadowAB_p->m_abEntry[currentRow]));

        errNum = kbp_dm_dba_write(devList_pp[dev_num], (nlm_u8) portNum,
                                    ab_num, 
                                    (nlm_u16)currentRow, 
                                    &(shadowAB_p->m_abEntry[currentRow]),
                                    NLMDEV_XY,
                                    o_reason_p);

        if(errNum != NLMERR_OK)
            return errNum;

        --currentRow;
    }
    
    
    return errNum;
}

/*
Function: NlmTblMgr__pvt_EraseRecord
Parameters:
        NlmDevMgr* devMgr_p,
        NlmPortNum portNum,
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
        NlmGenericTbl* genericTbl_p,
        NlmPortNum portNum,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        )
{
    nlm_u8 data[80];
    nlm_u8 mask[80];
    NlmGenericTblRecord TblRecord;
    NlmTblMgr *tblMgr_p = (NlmTblMgr*)genericTbl_p->m_genericTblMgr_p->m_tblMgr_p;
    NlmDevMgr* devMgr_p = tblMgr_p->m_devMgr_p;
    
    if(!devMgr_p)
    {
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

    return NlmTblMgr__pvt_WriteRecord(genericTbl_p, portNum, &TblRecord,
                        NULL, RecordIndex, NlmFalse, o_reason);
}



/*
Function: NlmTblMgr__pvt_InvalidateRecord
Parameters:
        NlmDevMgr* devMgr_p,
        NlmPortNum          portNum,
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
        NlmPortNum portNum,
        nlm_u16 recordLen,
        NlmRecordIndex RecordIndex,
        NlmReasonCode *o_reason
        )
{
#ifndef NLM_12K_11K
    nlm_u8 devNum = (nlm_u8)(RecordIndex / (devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));  
    nlm_u32 indexInDevice = RecordIndex % (devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH);
#else
    nlm_u8 devNum = (nlm_u8)(RecordIndex / NLM_ALLINDEX_PARDEVICE);  
    nlm_u32 indexInDevice = RecordIndex % NLM_ALLINDEX_PARDEVICE;
#endif
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
    errNum = kbp_dm_dba_invalidate(devList_pp[devNum], 
                    (nlm_u8)portNum, abNum, addrInAB, o_reason);
    

    if(errNum != NLMERR_OK)
        return errNum;

    /*In case we have 640-bit entry, then we have to invalidate the higher 320bits and the lower
    320 bits */
    if(recordLen > 320)
    {
        shadowAB_p->m_abEntry[addrInAB + 4].m_vbit = 0;
        errNum = kbp_dm_dba_invalidate(devList_pp[devNum],
                    (nlm_u8)portNum, abNum, addrInAB + 4, o_reason);
    }
    

    return errNum;
}

/*
Function: NlmTblMgr__pvt_WriteBCR
Parameters:
        NlmDevMgr* devMgr_p,
        NlmPortNum portNum,
        nlm_32 blockNum,
        nlm_u16 width,
        NlmDevDisableEnable Disbl_Enbl,
        NlmUdaChunkInfo* udaChunkInfo_p,
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
        NlmPortNum  portNum,
        nlm_32 blockNum, /* global blockNumber across all devices, ranges from 0 to 511 */
        nlm_u16 width,
        NlmDevDisableEnable Disbl_Enbl,
        NlmUdaChunkInfo* udaChunkInfo_p,
        NlmReasonCode *o_reason
        )
{
    nlm_u8 widthIn80Bit =0;
    NlmDev** devList_pp = NULL;
    NlmDevShadowDevice* shadow_p = NULL;
    NlmDevShadowAB* shadowAB_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_u32 baseAddr = 0;

    nlm_u8 dev_num = (nlm_u8)(blockNum / devMgr_p->m_numOfAbs);
    nlm_u8 ab_num = (nlm_u8) (blockNum % devMgr_p->m_numOfAbs);

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

    if(NLM_INVALID_BLOCKWIDTH != width)
    {
        nlm_u16 uda_width = 0;

        widthIn80Bit = (nlm_u8)(width/NLMDEV_AB_WIDTH_IN_BITS);
        shadowAB_p->m_blkConfig.m_blockWidth = (NlmDevBlockWidth)((widthIn80Bit >>1) - (widthIn80Bit/8));

        /* Check whether AD is associated with this DBA block or not */
        if((udaChunkInfo_p) && (udaChunkInfo_p->m_width != NLM_TBL_ADLEN_ZERO))
        {           
            /* Base address calculation.  */
            baseAddr = (NLMDEV_NUM_SRAM_ENTRIES_PER_SB * udaChunkInfo_p->m_sbNum) +
                                    udaChunkInfo_p->m_startRow * NLMDEV_NUM_SRAM_BLOCKS_IN_SB ;
            baseAddr >>= 9;

            uda_width = (nlm_u16)(1 << (4 + udaChunkInfo_p->m_width));

            /* Enable DBA block, configure BCR for UDA */
            errNum = kbp_dm_uda_config(devList_pp[dev_num],
                                            (nlm_u8)portNum,
                                            ab_num,
                                            width,
                                            baseAddr,
                                            uda_width,
                                            o_reason);

            return errNum;
        }
    }

    /* If we here it means, either no-AD or enable/disable DBA block */ 
    shadowAB_p->m_blkConfig.m_shiftDir   = NLMDEV_SHIFT_RIGHT;
    shadowAB_p->m_blkConfig.m_shiftCount = NLMDEV_SHIFT_CNT_0;
    shadowAB_p->m_blkConfig.m_baseAddr = baseAddr;

    errNum =  kbp_dm_block_reg_write(devList_pp[dev_num],
                                            (nlm_u8)portNum,
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
        NlmPortNum portNum,
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
            NlmPortNum portNum,
            nlm_u8 ltrNum,
            nlm_u8 ab_num, /*block number within a device */
            NlmDevDisableEnable enable_disable,
            NlmReasonCode *o_reason
            )
{
    NlmDevShadowLtr* shadowLtr_p = NULL;
    nlm_u8 nrOfBlksInBSR = NLM_NUM_BLKS_PER_BLOCK_SELECT_REG;
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
        
    if(shadowLtr_p[ltrNum].m_blockSelect[regNr].m_blkEnable[ab_num] == enable_disable)
    {
        #if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&dev_p->m_devMgr_p->m_spinLock);
        #endif

        return NLMERR_OK;
    }
    
    shadowLtr_p[ltrNum].m_blockSelect[regNr].m_blkEnable[ab_num] = enable_disable;

    errNum = kbp_dm_ltr_write(dev_p, (nlm_u8)portNum, ltrNum,
                                        (NlmDevLtrRegType)(NLMDEV_BLOCK_SELECT_0_LTR + regNr),
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
        NlmPortNum portNum,
        nlm_u8 ltrNum,
        nlm_u8 ab_num,
        NlmDevParallelSrch rBus,
        NlmDevKey key,
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
        NlmPortNum portNum,
        nlm_u8 ltrNum,
        nlm_u8 ab_num, /*block number within a device */
        NlmDevParallelSrch rBus,
        NlmDevKey key,
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

#ifndef NLM_12K_11K
    sb_num = ab_num / dev_p->m_devMgr_p->m_numOfABsPerSB;
#else
    sb_num = ab_num / NLM11KDEV_NUM_BLKS_PER_SUPER_BLOCK;
#endif


    /* finding super block nuber */

    /* finding parallel_search_register number */
    ps_reg_nr = ab_num / NLM_NUM_BLKS_PER_PS_REG;

    ab_num = ab_num % NLM_NUM_BLKS_PER_PS_REG;

    /* retrieving shadow ltr for the corresponding device */
    shadowLtr_p = ((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_ltr;
        
    shadowLtr_p[ltrNum].m_parallelSrch[ps_reg_nr].m_psNum[ab_num] = rBus;

    /* Write to the Parallel Search Register */
   errNum =  kbp_dm_ltr_write(dev_p, (nlm_u8)portNum, ltrNum,
                                                    (NlmDevLtrRegType)(ltr_reg_type + ps_reg_nr),
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
    errNum =  kbp_dm_ltr_write(dev_p, (nlm_u8)portNum, ltrNum,
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
    NlmTblMgr* self = (NlmTblMgr*)context_p;

    if(!self)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }


    /* finding device number */
#ifndef NLM_12K_11K
    old_dev_num = (nlm_u8)(oldAddress/(self->m_devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
    new_dev_num = (nlm_u8)(newAddress/(self->m_devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH));
#else
    old_dev_num = (nlm_u8)(oldAddress/NLM_ALLINDEX_PARDEVICE);
    new_dev_num = (nlm_u8)(newAddress/NLM_ALLINDEX_PARDEVICE);
#endif
    
    /* retrieving device list */
    devList_pp = (NlmDev**)self->m_devMgr_p->m_devList_pp;

    /* finding index within the device */
#ifndef NLM_12K_11K
    oldIdx = oldAddress % (self->m_devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH);
    newIdx = newAddress % (self->m_devMgr_p->m_numOfAbs * NLMDEV_AB_DEPTH);
#else
    oldIdx = oldAddress % NLM_ALLINDEX_PARDEVICE;
    newIdx = newAddress % NLM_ALLINDEX_PARDEVICE;
#endif
    
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
    /* Write the AD before Record to maintain coherency (Shuffling up)*/

       /* Write the UDA data if present */
       if(genericTbl_p->m_adWidth != NLM_TBL_ADLEN_ZERO)
       {
        err = NlmTblMgr__pvt_WriteOrShuffleUdaRecord(genericTbl_p, 
                self->m_portNumForCurOpr, NULL, newAddress, oldAddress, o_reason);
        if(NLMERR_OK != err)
                 return err;
       }   
    
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

        err = kbp_dm_dba_write(devList_pp[new_dev_num], 
                        (nlm_u8)self->m_portNumForCurOpr,
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
        /* Write the AD after Record to maintain coherency (Shuffling up)*/

       /* Write the UDA data if present */
       if(genericTbl_p->m_adWidth != NLM_TBL_ADLEN_ZERO)
       {
        err = NlmTblMgr__pvt_WriteOrShuffleUdaRecord(genericTbl_p, 
                self->m_portNumForCurOpr, NULL, newAddress, oldAddress, o_reason);
        if(NLMERR_OK != err)
                 return err;
       }
       
        self->m_indexChangedAppCb(self->m_client_p, genericTbl_p, oldAddress, newAddress);
    }

    /*If we are writting more than 80 bits while shuffling, then to prevent coherency issues
    we have to invalidate the old address of the entry that has been shuffled. Only after this
    we can write the the next shuffled entry to this location */
    if(recordLen > NLMDEV_AB_WIDTH_IN_BITS)
    {
        err = NlmTblMgr__pvt_InvalidateRecord(self->m_devMgr_p, 
            self->m_portNumForCurOpr, genericTbl_p->m_width, oldAddress, o_reason);

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
            NlmUdaChunkInfo* udaChunkInfo_p,
            NlmReasonCode *o_reason
            )
{
    NlmTblMgr* self = (NlmTblMgr*)context_p;

    if(!self)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }

    return NlmTblMgr__pvt_WriteBCR(self->m_devMgr_p,
                    self->m_portNumForCurOpr,
                                blockNum,
                                width,
                                NLMDEV_ENABLE,
                                udaChunkInfo_p,
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
    NlmTblMgr* self = (NlmTblMgr*)context_p;

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
                                self->m_portNumForCurOpr, blockNum,
                                NLM_INVALID_BLOCKWIDTH,
                                NLMDEV_DISABLE,
                                NULL,
                                o_reason);

        if(err != NLMERR_OK)
            return err;
    }

    /* finding device number */
    dev_num = (nlm_u8)(blockNum/self->m_devMgr_p->m_numOfAbs);
    
    /* finding device list */
    devList_pp = (NlmDev**)self->m_devMgr_p->m_devList_pp;

    /* finding search_attribute_node of the table */
    AttrNode = NlmSearchAttributeNode__Find(self->m_searchAttrList, gt_p->m_tblId);
     

    if(NULL ==AttrNode)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SEARCH_ATTRIBUTES;
        return NLMERR_FAIL;
    }
    
    /* if an empty new block is going to be used for filling record */
    if(addBlock)
    {
        for(idx =0; idx <NLMDEV_MAX_NUM_LTRS; idx++)
        {
            if(NLM_LTR_NOTCONFIGURED == AttrNode->m_isLtrConfigured[idx])
                continue;
            
            /* enabling the block in ltr_blockselect_register */
            err =NlmTblMgr__pvt_WriteLtrBS(devList_pp[dev_num],
                            self->m_portNumForCurOpr, idx, 
                            (nlm_u8)(blockNum%self->m_devMgr_p->m_numOfAbs), 
                                    NLMDEV_ENABLE, o_reason);

            if(NLMERR_OK != err)
                return err;

            /* assinging the result_bus to this block according to PS assigned for this table */
            err = NlmTblMgr__pvt_WriteLtrPSAndKPUSelect(devList_pp[dev_num],
                        self->m_portNumForCurOpr, idx,
                        (nlm_u8)(blockNum%self->m_devMgr_p->m_numOfAbs), 
                            AttrNode->m_ps_RBus[idx], AttrNode->m_keyNum[idx], o_reason);

            if(NLMERR_OK != err)
                return err;
        }
        
    }
    else    /* if it is not a new empty block */
    {
        for(idx =0; idx <NLMDEV_MAX_NUM_LTRS; idx++)
        {
            if(NLM_LTR_NOTCONFIGURED == AttrNode->m_isLtrConfigured[idx])
                continue;
            
            err =NlmTblMgr__pvt_WriteLtrBS(devList_pp[dev_num], 
                     self->m_portNumForCurOpr, idx, 
                     (nlm_u8)(blockNum%self->m_devMgr_p->m_numOfAbs), 
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
    NlmTblMgr* self_p = (NlmTblMgr*)context_p;
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

    err = NlmTblMgr__pvt_EraseRecord(genericTbl_p, self_p->m_portNumForCurOpr, 
                    recordWidth, recordIndex, o_reason_p);

    return err;

}


NlmErrNum_t NlmTblMgr__pvt__checkSplTables(
                NlmGenericTblMgr  *genericTblMgr_p,
                NlmReasonCode     *o_reason
                )
{
    NlmTblMgr *self = NULL;
    NlmErrNum_t err = NLMERR_OK;
    *o_reason = NLMRSC_REASON_OK;
    
    if(genericTblMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_MEMALLOC_PTR;
        return NLMERR_FAIL;
    }
    self = (NlmTblMgr*)genericTblMgr_p->m_tblMgr_p;

    self->m_portNumForCurOpr = 0;

    /* check the special entries in special blocks */
    err =  NlmBlkMemMgr__CheckSplBlkTbls(self->m_blockMemMgr);

    if(NLMERR_OK !=err)
    {
        *o_reason = NLMRSC_INTERNAL_ERROR;
        return err;
    }

    /* check non-special blocks for the special table entries */
    {
        NlmGenericTblList *gTblList = (NlmGenericTblList*)(genericTblMgr_p->m_genericTbl_list_p)->m_next_p;
        NlmGenericTblList *gTblListNode = NULL;
        nlm_u8 splTblId = 0;

        while(gTblList->m_gt_p)
        {
            if(gTblList->m_gt_p->m_keepSeparate)
            {
                splTblId = gTblList->m_gt_p->m_tblId;

                err = NlmBlkMemMgr__CheckAnySplBlksMerged(self->m_blockMemMgr, splTblId);
                if(NLMERR_OK !=err)
                {
                    *o_reason = NLMRSC_INTERNAL_ERROR;
                    return err;
                }

            }
            gTblListNode = (NlmGenericTblList*)gTblList->m_next_p;
            gTblList = gTblListNode;
        }

    }

    return err;
}



