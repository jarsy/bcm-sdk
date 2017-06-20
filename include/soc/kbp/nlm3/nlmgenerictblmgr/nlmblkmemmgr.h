/*
 * $Id: nlmblkmemmgr.h,v 1.1.6.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef INCLUDED_NLMBLKMEMMGR_H
#define INCLUDED_NLMBLKMEMMGR_H

#ifndef NLMPLATFORM_BCM
#include "nlmcmbasic.h"
#include "nlmgenerictblmgr.h"
#include "nlmcmexterncstart.h"
#include "nlmudamemmgr.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/nlm3/nlmgenerictblmgr/nlmgenerictblmgr.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#include <soc/kbp/nlm3/nlmgenerictblmgr/nlmudamemmgr.h>
#endif


/* 
Callback registered by TblMgr with BlockMemMgr. This callback is called 
when the index of the record changes. The function in TblMgr then moves the  
record from oldIndex to newIndex in the hardware and informs the application
*/ 
typedef NlmErrNum_t (*NlmIndexChangedTblMgrCB)(
        void*          ctx_p,       /* ptr passed by upper layer during  
                                    BlkMemMgr__Init, so that data can be accessed
                                    by the upper layer in the callback */
        NlmGenericTbl* genericTbl_p, /* pointer to the generic table 
                                    whose record is being shuffled*/
        NlmRecordIndex oldIndex,    /* old index of the entry */
        NlmRecordIndex newIndex,    /* new index of the entry */
        nlm_u16        recWidth,     /* width of the entry*/
        NlmReasonCode *o_reason_p);

/*
Callback  which is called when blk width has to be set in hardware
*/
typedef NlmErrNum_t (*NlmSetBlkWidthCB)(void* ctx_p,
                                    nlm_32 blkId,
                                    nlm_u16 width,
                                    NlmUdaChunkInfo* udaChunkInfo_p,
                                    NlmReasonCode *o_reason_p);


/*
At run time the blocks in which the table resides can change. 
If a record of the table enters the block for the first time,  
the callback function is called with addBlkForTbl = NlmTrue.
If the record deleted was the last record of the table in the block, 
the callback function is called with addBlkForTbl = NlmFalse
isBlkEmpty is true when no table has a record in the blk
*/
typedef NlmErrNum_t (*NlmBlkChangedForTblCB)(
                void* ctx_p,
                NlmGenericTbl   *genericTbl_p,
                nlm_32 blkId,
                NlmBool addBlkForTbl,
                NlmBool isBlkEmpty, /*Blk is completely empty */
                NlmReasonCode *o_reason_p);


/*
When a delete table is done, all the records of the table have to
be removed. The callback is registered by TM with BMM. When a 
delete table is invoked, BMM will call the callback for each record
of the table that has to be deleted. TM will then actually remove
the record from hardware
*/
typedef NlmErrNum_t (*NlmRemoveRecordCB)(
                void* ctx_p,
                NlmGenericTbl   *genericTbl_p,
                NlmRecordIndex  recIndex, /*index of record to be deleted */
                nlm_u16         recWidth, /*width of the record to be deleted */
                NlmReasonCode   *o_reason_p);


/*Function returns a pointer to  NlmBlkMemMgr*/
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
    );


NlmErrNum_t NlmBlkMemMgr__InitTbl(
                void*           self_p, /*pointer to NlmBlkMemMgr*/     
                NlmGenericTbl   *genericTbl_p,
                NlmReasonCode   *o_reason_p);


NlmErrNum_t NlmBlkMemMgr__AddRecord(
            void*           self_p,         /*pointer to NlmBlkMemMgr*/
            NlmGenericTbl   *genericTbl_p,  
            nlm_u16         newRecPriority,
            nlm_u16         groupId,
            NlmRecordIndex  *o_recIndex_p, 
            nlm_16          newRecWidth,
            NlmBool         *isShuffleDown_p,
            NlmReasonCode   *o_reason_p);


NlmErrNum_t NlmBlkMemMgr__DeleteRecord(
        void*               self_p,         /*pointer to NlmBlkMemMgr*/
            NlmGenericTbl       *genericTbl_p,  
        NlmRecordIndex      recIndex,   
        NlmReasonCode       *o_reason_p);

NlmErrNum_t NlmBlkMemMgr__ValidateRecord(
        void*               self_p,         /*pointer to NlmBlkMemMgr*/
        NlmGenericTbl       *genericTbl_p,  
        NlmRecordIndex      recIndex,
        NlmReasonCode   *o_reason_p);

NlmErrNum_t 
NlmBlkMemMgr__GetUdaChunkInfo(
    void*               self_p,         
    NlmRecordIndex      recIndex,   
    NlmUdaChunkInfo **udaChunkInfo__pp,
    NlmReasonCode       *o_reason_p
    );

NlmErrNum_t
NlmBlkMemMgr_iter_next(
    void*               self_p,
    NlmGenericTblIterData  *iterInfo,
    NlmGenericTblIterHandle *iterHandle,
    NlmDevShadowDevice *shadowdevice,
    NlmDevShadowUdaSb *shadowUDA_p);

NlmBool
NlmBlkMemMgr_check_tblId(
	void*    self_p,
	nlm_u16 blockNum,
	nlm_u8 tblId);

NlmErrNum_t NlmBlkMemMgr__DestroyTbl(
                        void* self_p,       /*pointer to NlmBlkMemMgr*/
                        NlmGenericTbl* genericTbl_p,
                        NlmReasonCode *o_reason_p);

NlmErrNum_t NlmBlkMemMgr__Destroy(
                            void* self_p,   /*pointer to NlmBlkMemMgr*/
                            NlmReasonCode *o_reason_p);

void NlmBlkMemMgr__Print(
            void *self_p                    /*pointer to NlmBlkMemMgr*/
            );

NlmBool NlmBlkMemMgr__VfyTblEntries(
                        void *self_p,       /*pointer to NlmBlkMemMgr*/
                        NlmGenericTbl* genericTbl_p);

NlmErrNum_t 
NlmBlkMemMgr__CheckSplBlkTbls(void *self_p); /*pointer to NlmBlkMemMgr*/

NlmErrNum_t 
NlmBlkMemMgr__CheckAnySplBlksMerged(void *self_p,
                                    nlm_u8 splTblId
                                    );
#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif
#endif

