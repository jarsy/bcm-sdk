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
#include "nlmblackholexpt.h"
#include "nlmcmopenhash_pfxbundle.h"
#include "nlmcmmt.h"

#ifdef NLM_NETOS
/* default number of insert threads on XLP NETOS */
#define NLM_FIB_REFAPP_NETOS_NUM_THREADS    (4)
#endif

#define NLM_FIB_REFAPP_MAX_PFX_LEN        	(56)
#define	NLM_FIB_REFAPP_THREADS_COUNT		  (8)
#define NLM_FIB_REFAPP_MAX_NUM_DEVICES        (NLMDEV_MAX_DEV_NUM)
#define NLM_FIB_REFAPP_MAX_RQT_COUNT          (1)
#define NLM_FIB_REFAPP_MAX_RSLT_COUNT         (1)

#define NLM_FIB_REFAPP_TBL_ID_LEN             (4)
#define NLM_FIB_REFAPP_NUM_TABLES             (1)
#define NLM_FIB_REFAPP_NUM_SRCH_CONFIG        (1)

#define NLM_FIB_REFAPP_RANDOM_WORD          ((nlm_u32)((rand() << 16)|rand()))

#define NLM_FIB_REFAPP_RANDOM_BYTE      ((nlm_u8)(rand()))    
#define NLM_FIB_REFAPP_RANDOM_WORD      ((nlm_u32)((rand() << 16)|rand()))
#define NLM_FIB_REFAPP_RAND_VALUE_BTW_1_TO_100 ((nlm_u8) (1 + 100.0 * (rand() / (RAND_MAX + 1.0))))

/* Timing calls */
#define NlmFtmRefApp_GetTime(timeValue)	gettimeofday(&timeValue, NULL); 
#define NlmFtmRefApp_CalcTimeDiff(start, end) ((int)((start.tv_sec - end.tv_sec)) * 1000000 +  (int)((start.tv_usec - end.tv_usec))) 

typedef struct
{
	nlm_u32 th_insTime;
	nlm_u32 th_inCount;

} NlmFtmRefApp_threadTime;

typedef struct NlmFtmRefApp_TableInfo_s
{
    nlm_u8  m_tblId;					/* Id of table; should be prepended to each
										   prefix added to table and part of search key */
    nlm_u16 m_tblWidth;					/* width of table;
										   also represents max length of prefixes */
    nlm_u32 m_numOfPfxsToBeAdded;		/* Number of prefixes
										   to be added to the table */
    nlm_u32 m_indexLow;					/* Low value of the range within which the
										   hit index of search is obtained */
    nlm_u32 m_indexHigh;				/* High value of the index range
										   Valid Range : 0 - 0x7FFFFF*/
	nlm_u32 m_uniqueCnt;				/* number of unique preifixes in this table */

} NlmFtmRefApp_TableInfo;


static NlmFtmRefApp_TableInfo tableInfo[NLM_FIB_REFAPP_NUM_TABLES] = /* Table Attributes
                                                                     being used in the refapp */
                                {
								 {0, 146, 200000, 0, 300000, 0}	
                               //{0, 36, 2000002, 0, 3000000, 0}	//uncomment this line for file enabled prefixes and comment above line	
							   /* TblId: 0, Tblwidth: 146,
																	   number of pfx added: 20000,
																	   index range: 0 - 30000 */
                                };



typedef enum NlmFtmRefApp_XptType_e
{
    NLM_FIB_REFAPP_SIMULATION_XPT,	/* Cmodel Xpt; Can be used for simualtions */
    NLM_FIB_REFAPP_BLACKHOLE_XPT,   /* Can be used for update speeds */

#ifdef NLM_XLP
	NLM_FIB_REFAPP_XLP_XPT,			/* XLP Xpt: used with actual device */
#endif
	NLM_FIB_REFAPP_XPT_END

}NlmFtmRefApp_XptType;



typedef struct NlmFtmRefApp_RefAppInfo_s
{
    NlmCmAllocator			*m_alloc_p; /* general purpose allocator */

    NlmDev_OperationMode	m_operMode; /* operating mode of operation
                                           Should be Sahasra mode for FIB applications */
    NlmFtmRefApp_XptType	m_xptType;
    NlmXpt					*m_xpt_p;   /* Transport Interface Module Pointer */

    NlmDevMgr				*m_devMgr_p;	/* Device Manager module pointer */
    nlm_u8					m_numOfDevices; /* number of devices in cascade */
    NlmDev					**m_dev_pp;     /* Array of device pointers */
   
    NlmFibTblMgr			*m_fibTblMgr_p; /* FIB Table Manager module pointer */
    nlm_u8					m_numOfTables;  /* Number of FIB tables created */
    NlmFibTbl				**m_fibTbl_pp;  /* Array of FIB Table pointers */    

	NlmCmMtSpinlock			m_refApp_spinLock;

	nlm_u8					m_numInsThrds;
	nlm_u32					m_coreNum[NLM_FIB_REFAPP_THREADS_COUNT];

	NlmCmOpenHashPfxBundle	*m_hashNode_p;
    char					*g_pfxArray;

	nlm_u32					m_uniquePfxCount;
	nlm_u32					m_duplicatePfxCount;
	nlm_u32					m_fibOneOffset;
	NlmBool 				m_isFileEnable;
	nlm_u32 				m_noOfPrefixesInFile;
	FILE	*m_inputPfxFile_p;

} NlmFtmRefApp_RefAppInfo;


extern nlm_u32 NlmFtmRefApp_GeneratePrefixes(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    );

extern nlm_u32 NlmFtmRefApp_DestroyPrefixes(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    );

extern nlm_u32 NlmFtmRefApp_ParseArgs(
	NlmFtmRefApp_RefAppInfo	*refApp_p,
	int						argc,
	char					**argv
	);

extern void NlmFtmRefApp_Usage(void);

#endif

