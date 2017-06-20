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
 
/*General comments
Contains the declarations that are internally used by blk memory manager.
These declarations are not exposed to the upper layer
*/

#ifndef INCLUDED_NLMBLKMEMMGRINT_H
#define INCLUDED_NLMBLKMEMMGRINT_H

#include "nlmcmbasic.h"
#include "nlmgenerictblmgr.h"
#include "nlmblkmemmgr.h"
#include "nlmcmtwolevelhashtbl.h"
#include "nlmcmfreeslotmgr.h"

typedef struct NlmTblsInBlk_t NlmTblsInBlk_t; 

typedef struct NlmRecInfo_t  NlmRecInfo_t;

typedef struct NlmBlkInfo_t NlmBlkInfo_t;

typedef struct NlmBlkMemMgr_t NlmBlkMemMgr_t;

typedef struct NlmBlkMemMgrFnArgs_t NlmBlkMemMgrFnArgs_t;

typedef struct NlmTblPvtData_t NlmTblPvtData_t; 


/* 
TblsInBlk gives lists all the tables whose records are present in the block 
*/
struct NlmTblsInBlk_t
{
	#include "nlmcmdbllinklistdata.h"

	nlm_8*		m_tblId;		/* Id of the table that is present in the block */
	nlm_16		m_numRecs;
	
};



/* 
RecInfo has the information about each record in a block
*/
struct NlmRecInfo_t
{
	nlm_u16		m_priority;	/*Priority of the record */
	nlm_u16		m_groupId;	/*Id of the group within the table*/
	nlm_8*		m_tblId;	/*Table to which the record belongs to 
							If tblId is Null, location is free */
};


/*
BlkInfo has information about each block in the chip.  
*/
struct NlmBlkInfo_t
{
    nlm_u8 m_validBlk;          /* Represents whether the blk can be used for GTM applications */
	nlm_16 m_blkWidth;			/* width of each record of the block in bits 
									(80, 160, 320, 640)*/
	nlm_16 m_numUsedRecs;		/* Total num of used records present in the block. 
									Blk width cannot be changed when num of used records >= 1 */
	NlmRecInfo_t* m_recInfo_p;       /* Pointer to an array having information about 
									  each record in the block */
	NlmTblsInBlk_t* m_tblList_p;		/* pointer to list having info about tables whose entries are present 
									in the block */
	NlmFreeSlotMgr_t* m_freeSlotMgr_p;
} ;


 
struct NlmBlkMemMgr_t
{
	NlmCmAllocator*			m_alloc_p;				/*pointer to memory allocation fns */
	NlmBlkInfo_t*				m_blkInfo_p;			/* pointer to an array. m_blkInfo_p[0]
													has the information about block 0 */
    NlmIndexChangedTblMgrCB m_indexChangedCB;   
	NlmSetBlkWidthCB		m_setBlkWidthCB;		
	NlmBlkChangedForTblCB   m_blkChangedForTblCB;
	NlmRemoveRecordCB       m_removeRecordCB;
	void*                   m_ctx_p;			/* Pointer to info the caller wants to 
												access in callback */
	nlm_u8					m_nrOfDevices;  /*Number of devices in the cascade */
};



struct NlmBlkMemMgrFnArgs_t
{
	NlmGenericTbl    *genericTbl_p;
	nlm_u16			 groupId;	
	nlm_u16          newRecPriority; 
	nlm_16			 newRecWidth;
	NlmRecordIndex   *o_recIndex_p; 
	NlmReasonCode	 *o_reason_p;
	NlmBlkMemMgr_t   *blkMemMgr_p;
	NlmBool			 isShuffleDown;
};

struct NlmTblPvtData_t
{
	void*	m_groupIdHashTbl_p;
};




#endif
