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
 #ifndef INCLUDED_NLM_GTM_FTM_REFAPP_H
#define INCLUDED_NLM_GTM_FTM_REFAPP_H

#include "nlmcmbasic.h"
#include "nlmcmportable.h"
#include "nlmcmdevice.h"
#include "nlmcmutility.h"
#include "nlmcmstring.h"
#include "nlmgenerictblmgr.h"
#include "nlmfibtblmgr.h"
#include "nlmrangemgr.h"
#include "nlmdevmgr.h"
#include "nlmarch.h"
#include "nlmsimxpt.h"
#include "nlmxpt.h"

#define	NLM_RETURN_STATUS_OK		0
#define	NLM_RETURN_STATUS_FAIL		1
#define	NLM_RETURN_STATUS_ABORT		2

#define NLM_GTM_FTM_REFAPP_DEBUG_PRINT_ITER_VAL		1000

#define NLM_GTM_FTM_REFAPP_GTM_START_BLK		0
#define NLM_GTM_FTM_REFAPP_GTM_END_BLK			63
#define NLM_GTM_FTM_REFAPP_FTM_START_BLK		64
#define NLM_GTM_FTM_REFAPP_FTM_END_BLK			119

#define NLM_GTM_FTM_REFAPP_GTM_TBL_ID_LEN		4
#define NLM_GTM_FTM_REFAPP_GTM_TBL_ID			"0000"
#define NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH		160
#define NLM_GTM_FTM_REFAPP_GTM_TBL_SIZE			10000
#define NLM_GTM_FTM_REFAPP_GTM_NUM_OF_GROUPS	5
#define	NLM_GTM_FTM_REFAPP_GTM_NUM_OF_RANGES	12

#define NLM_GTM_FTM_REFAPP_FIB_TBL_ID_LEN			4
#define NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_ID			"0000"
#define NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_ID			"0001"
#define NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_WIDTH		36
#define NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_WIDTH		48
#define NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_NUM_PFXS		10000
#define NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_NUM_PFXS		10000
#define NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_INDEX_LOW	0 
#define NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_INDEX_HIGH	20000
#define NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_INDEX_LOW	0
#define NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_INDEX_HIGH	20000

#define NLM_GTM_FTM_REFAPP_SRCH_LTR_NUM				0
#define NLM_GTM_FTM_REFAPP_GTM_SRCH_KEY_NUM			3
#define NLM_GTM_FTM_REFAPP_GTM_SRCH_RSLT_PORT_NUM	3

#define NLM_GTM_FTM_REFAPP_FIB_NUM_SRCHES				2
#define NLM_GTM_FTM_REFAPP_FIB_SRCH_ONE_RSLT_PORT_NUM   0		
#define NLM_GTM_FTM_REFAPP_FIB_SRCH_ONE_START_BIT       79	
#define NLM_GTM_FTM_REFAPP_FIB_SRCH_TWO_RSLT_PORT_NUM   1		
#define NLM_GTM_FTM_REFAPP_FIB_SRCH_TWO_START_BIT       159		


typedef enum XptIfType
{
	IFTYPE_CMODEL,
#ifdef NLM_XLP	
	IFTYPE_XLPXPT, 
#endif
        IFTYPE_BCM_CALADAN3,
    IFTYPE_END 
}XptIfType;

typedef struct NlmGtmFtmRefAppData
{
	/* Memory Allocator declarations */
	NlmCmAllocator  *m_alloc_p;	

	/* Transport Layer declarations */
	NlmXpt			*m_xpt_p;	
	nlm_u32			 m_channel_id;
	nlm_u32			 m_request_queue_len;
	nlm_u32  		 m_result_queue_len;
	XptIfType		 m_xptType;

	/* Device Manager declarations */
	NlmDevMgr 	*m_devMgr_p;
	NlmDev		*m_dev_p;
	NlmDev_OperationMode  m_opr_mode;
    
	/* Range Manager declarations */
	NlmRangeMgr		*m_rangeMgr_p;
	NlmRangeDb     	*m_rangeDb_p;	
	NlmRange		 m_ranges[ NLM_GTM_FTM_REFAPP_GTM_NUM_OF_RANGES ];	
	
	NlmRangeDbAttrSet     m_rangeDbAttrs;

	/* Generic Table Manager declarations */
	NlmGenericTblMgr	*m_genericTblMgr_p;	
    NlmGenericTblMgrBlksRange m_gtmBlkRange;
    NlmIndexChangedAppCb		m_gtmRecordIndexChangeCB;
    NlmGenericTbl		*m_gtmTbl_p;		
    nlm_8 m_gtmTblId[ NLM_GTM_FTM_REFAPP_GTM_TBL_ID_LEN + 1 ];
    NlmGenericTblWidth m_gtmTblWidth;
    nlm_u32 m_gtmTblNumOfRecords;
	nlm_u32 m_gtmTblSize;
    nlm_u8 m_srchLtrNum;		
    nlm_u8 m_gtmSrchRsltPortNum;		
    nlm_u8 m_gtmSrchKeyNum;       
    NlmGenericTblKeyConstructionMap m_gtmSrchKCM;
    
	/* Fib Table Manager Declarations */
    NlmFibTblMgr *m_fibTblMgr_p;
    NlmFibBlksRange m_fibBlkRange;	
    NlmFibTbl *m_fibTblOne_p;
    NlmFibTbl *m_fibTblTwo_p;
    nlm_8 m_fibTblOneId[NLM_GTM_FTM_REFAPP_FIB_TBL_ID_LEN + 1];
    nlm_8 m_fibTblTwoId[NLM_GTM_FTM_REFAPP_FIB_TBL_ID_LEN + 1];
    NlmFibTblIndexRange m_fibTblOneIndexRange;
    NlmFibTblIndexRange m_fibTblTwoIndexRange;
    nlm_u16 m_fibTblOneWidth;
    nlm_u16 m_fibTblTwoWidth;
    nlm_u32 m_fibTblOneNumOfPfxsToBeAdded;
    nlm_u32 m_fibTblTwoNumOfPfxsToBeAdded;
    NlmFibPrefixIndexChangedAppCb m_ftmPrefixIndexChangeCB;
    nlm_u8 m_fibTblIdLen;
    nlm_u8 m_numOfFibSrches;
    nlm_u8 m_fibSrchOneRsltPortNum;		
    nlm_u16 m_fibSrchOneStartBitValue;	
    nlm_u8 m_fibSrchTwoRsltPortNum;		
    nlm_u16 m_fibSrchTwoStartBitValue;	
} NlmGtmFtmRefAppData;

#include "nlmcmexterncend.h"

#endif /* INCLUDED_NLMTBLMGRREFAPP_H */
