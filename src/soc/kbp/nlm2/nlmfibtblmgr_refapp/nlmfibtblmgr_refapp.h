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
 #ifndef NLM_FIB_REFAPP_H
#define NLM_FIB_REFAPP_H

#include "nlmfibtblmgr.h"
#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmdevmgr.h"
#include "nlmarch.h"
#include "nlmcmprefix.h"
#include "nlmcmutility.h"
#include "nlmcmstring.h"
#include "nlmsimxpt.h"
#include <sal/core/time.h>

#define NLM_FIB_REFAPP_MAX_NUM_DEVICES     (NLMDEV_MAX_DEV_NUM)
#define NLM_FIB_REFAPP_MAX_RQT_COUNT          1
#define NLM_FIB_REFAPP_MAX_RSLT_COUNT         1

#define NLM_FIB_REFAPP_TBL_ID_LEN             4
#define NLM_FIB_REFAPP_NUM_TABLES             3
#define NLM_FIB_REFAPP_NUM_SRCH_CONFIG        3
#define NLM_FIB_REFAPP_RANDOM_WORD          ((nlm_u32)((rand() << 16)|rand()))


/* for batch mode */
#define NLM_FIB_REFAPP_BATCH_MAX_PFX_COUNT		80000
#define NLM_FIB_REFAPP_BATCH_SIZE_ARRAY_COUNT   4
#define NLM_FIB_REFAPP_BATCH_MAX_SIZE_COUNT		2100  /* MAX batch size used in reference application */
#define NLM_FIB_REFAPP_MAX_BATCH_PFX_LEN        56    /* do not change */
#ifdef WIN32
    #define NlmFibRefApp_GetTime(timeValue)	QueryPerformanceCounter(&timeValue);
    #define NlmFibRefApp_CalcTimeDiff(end, start) \
    (((1.0*end.QuadPart-1.0*start.QuadPart)/qfreq.QuadPart)* 1000000)
#else
    #define NlmFibRefApp_GetTime(timeValue)	timeValue.tv_usec = sal_time(); timeValue.tv_sec = 0;
    #define NlmFibRefApp_CalcTimeDiff(end, start) \
		((int)((end.tv_sec - start.tv_sec)) * 1000000 +  (int)((end.tv_usec - start.tv_usec)))
#endif




typedef enum NlmFibRefApp_XptType_e
{
    NLM_FIB_REFAPP_SIMULATION_XPT, /* Cmodel Xpt; Can be used for simualtions */
    NLM_FIB_REFAPP_BLACKHOLE_XPT,   /* Can be used for update speeds */
#ifdef NLM_XLP
	NLM_FIB_REFAPP_XLP_XPT,   /* XLP Xpt */
#endif
    NLM_FIB_REFAPP_CALADAN3_XPT,  /* Caladan3 Xpt */
    NLM_FIB_REFAPP_XPT_END
}NlmFibRefApp_XptType;

typedef enum NlmFibRefApp_Type_e
{
    NLM_FIB_REFAPP_TYPE_NORMAL,  /* shows with 3 tables, 1x parallel search */
    NLM_FIB_REFAPP_TYPE_CONFIG2, /* shows config2 API usage */
	NLM_FIB_REFAPP_TYPE_BATCH,   /* shows adding prefixes in batch mode */
	NLM_FIB_REFAPP_TYPE_END, /* end */
}NlmFibRefApp_Type;


typedef struct NlmFibRefApp_TableInfo_s
{
    nlm_u8 m_tblId;						/* Id of table; should be prepended to each
										   prefix added to table and part of search key */
    nlm_u16 m_tblWidth;					/* width of table;
										   also represents max length of prefixes */
    nlm_u32 m_numOfPfxsToBeAdded;		/* Number of prefixes
										   to be added to the table */
    nlm_u32 m_indexLow;					/* Low value of the range within which the
										   hit index of search is obtained */
    nlm_u32 m_indexHigh;				/* High value of the index range
										   Valid Range : 0 - 0x7FFFFF*/
} NlmFibRefApp_TableInfo;

typedef struct NlmFibRefApp_SrchInfo_s
{
    nlm_u8 m_ltrNum;                     /* ltr number to
                                         be configured */
    nlm_u8 m_numOfParallelSrches;        /* number of searches
                                         to be configured */
    nlm_u8 m_tblIdForParallelSrch0;      /* tblId for one of
                                         the search operation */
    nlm_u8 m_rsltPortForParallelSrch0;   /* rslt port number for
                                         one of the search operation */
    nlm_u16 m_startBitForParallelSrch0;  /* prefix start bit in master key for one
                                         of the search operation */
} NlmFibRefApp_SrchInfo;

typedef struct NlmFibRefapp_searchInfo_s
{
	nlm_u8 m_ltrNumber; /* ltr number to configure search*/

	nlm_u8 m_parallelSearch0TblId;/* primary tabl id*/

	nlm_u8 m_parallelSearch1TblId;/* secondary table Id */

	nlm_u8 m_parallelSearch0ResultPort;/* primary search Result port*/

	nlm_u8 m_parallelSearch1ResultPort;/* secondary search result port*/

	nlm_u16 m_StratBitInKey;	/*start bit for search*/

}NlmFibRefApp_Special_parallel_SearchInfo;

typedef struct NlmFibRefApp_RefAppInfo_s
{
    NlmFibRefApp_TableInfo m_tableInfo[NLM_FIB_REFAPP_NUM_TABLES]; /* table information */

    NlmFibRefApp_SrchInfo  m_srchInfo[NLM_FIB_REFAPP_NUM_SRCH_CONFIG]; /* search information */

	/* config2 new one */
	NlmFibRefApp_Special_parallel_SearchInfo m_specialSrch;

} NlmFibRefApp_RefAppInfo;
	  
typedef struct NlmFibRefApp_RefApp_s
{
    NlmCmAllocator			*m_alloc_p; /* general purpose allocator */

    NlmDev_OperationMode	m_operMode; /* operating mode of operation
                                       Should be Sahasra mode for FIB applications */
    NlmFibRefApp_XptType	m_xptType;
    void					*m_xpt_p; /* Transport Interface Module Pointer */

    NlmDevMgr				*m_devMgr_p;   /* Device Manager module pointer */
    nlm_u8					m_numOfDevices;   /* number of devices in cascade */
    NlmDev					**m_dev_pp;     /* Array of device pointers */
   
    NlmFibTblMgr			*m_fibTblMgr_p; /* FIB Table Manager module pointer */
    nlm_u8					m_numOfTables;  /* Number of FIB tables created */
    NlmFibTbl				**m_fibTbl_pp; /* Array of FIB Table pointers */

	NlmFibRefApp_Type       m_type;
	nlm_u8                  m_numSearches;	
	NlmFibRefApp_RefAppInfo m_info;

	/* configsearch2 members */
	NlmBool					m_isConfig2;

	/* batch mode members */
	nlm_u32 m_oldIdx[NLM_FIB_REFAPP_BATCH_MAX_PFX_COUNT];
	nlm_u32 m_newIdx[NLM_FIB_REFAPP_BATCH_MAX_PFX_COUNT];
	NlmBool	m_isBatch;
	nlm_u32 m_startAt;
	nlm_u32 m_inxCnt;
	nlm_u32 m_assoDataCBCnt;
	nlm_u8  m_batchSize;

	double m_pfxInsertTime;	/*Time to insert prefix*/


} NlmFibRefApp_RefApp;

#endif

