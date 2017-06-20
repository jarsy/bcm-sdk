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
 
#ifndef INCLUDED_NLMBLKMEMMGR_H
#define INCLUDED_NLMBLKMEMMGR_H

#include "nlmcmbasic.h"
#include "nlmgenerictblmgr.h"

#include "nlmcmexterncstart.h"

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
		NlmRecordIndex oldIndex,	/* old index of the entry */
		NlmRecordIndex newIndex,	/* new index of the entry */
		nlm_u16		   recWidth,     /* width of the entry*/
		NlmReasonCode *o_reason_p);

/*
Callback  which is called when blk width has to be set in hardware
*/
typedef NlmErrNum_t (*NlmSetBlkWidthCB)(void* ctx_p,
									nlm_32 blkId,
									nlm_u16 width,
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
				NlmBool isBlkEmpty,	/*Blk is completely empty */
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
				nlm_u16		    recWidth, /*width of the record to be deleted */
				NlmReasonCode   *o_reason_p);


/*Function returns a pointer to  NlmBlkMemMgr*/
void* NlmBlkMemMgr__Init(
				NlmCmAllocator			*alloc_p,
				NlmIndexChangedTblMgrCB	indexChangedCB,
				NlmSetBlkWidthCB		setblkWidthCB,
				NlmBlkChangedForTblCB	blkChangedForTblCB,
				NlmRemoveRecordCB		removeRecordCB,
				nlm_u8					nrOfDevices,
                NlmGenericTblMgrBlksRange      *gtmBlksRange,
				void                     *ctx_p,
				NlmReasonCode			*o_reason_p);

NlmErrNum_t NlmBlkMemMgr__InitTbl(
				void*			self_p,	/*pointer to NlmBlkMemMgr*/		
				NlmGenericTbl   *genericTbl_p,
				NlmReasonCode	*o_reason_p);


NlmErrNum_t NlmBlkMemMgr__AddRecord(
			void*			self_p,			/*pointer to NlmBlkMemMgr*/
			NlmGenericTbl   *genericTbl_p,	
			nlm_u16         newRecPriority,
			nlm_u16			groupId,
			NlmRecordIndex  *o_recIndex_p, 
			nlm_16			newRecWidth,
			NlmBool			*isShuffleDown_p,
			NlmReasonCode	*o_reason_p);


NlmErrNum_t NlmBlkMemMgr__DeleteRecord(
		void*				self_p,			/*pointer to NlmBlkMemMgr*/
      		NlmGenericTbl       *genericTbl_p,	
		NlmRecordIndex      recIndex,	
		NlmReasonCode		*o_reason_p);

NlmErrNum_t NlmBlkMemMgr__ValidateRecord(
		void*				self_p,			/*pointer to NlmBlkMemMgr*/
		NlmGenericTbl       *genericTbl_p,	
		NlmRecordIndex      recIndex,
		NlmReasonCode	*o_reason_p);


NlmErrNum_t NlmBlkMemMgr__DestroyTbl(
						void* self_p,		/*pointer to NlmBlkMemMgr*/
						NlmGenericTbl* genericTbl_p,
						NlmReasonCode *o_reason_p);

NlmErrNum_t NlmBlkMemMgr__Destroy(
							void* self_p,	/*pointer to NlmBlkMemMgr*/
							NlmReasonCode *o_reason_p);

void NlmBlkMemMgr__Print(
			void *self_p					/*pointer to NlmBlkMemMgr*/
			);

NlmBool NlmBlkMemMgr__VfyTblEntries(
						void *self_p,		/*pointer to NlmBlkMemMgr*/
						NlmGenericTbl* genericTbl_p);

#include "nlmcmexterncend.h"

#endif

