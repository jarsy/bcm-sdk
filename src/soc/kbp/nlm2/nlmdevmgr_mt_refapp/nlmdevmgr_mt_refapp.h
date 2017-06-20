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
 

#ifndef _NLM_DEVMGR_REFAPP_H_INCLUDED_
#define _NLM_DEVMGR_REFAPP_H_INCLUDED_

#include "nlmdevmgr.h"
#include "nlmdevmgr_shadow.h"
#include "nlmsimxpt.h"
#include "nlmcmutility.h"

#define NLM_DEVMGR_REFAPP_THREAD_COUNT	(2)
#define NLM_DEVMGR_REFAPP_CMP1			(0)
#define NLM_DEVMGR_REFAPP_CMP2			(1)

#define	NLM_DEVMGR_REFAPP_START_VALUE	(1234)

#ifdef NLM_NETOS

#define NLM_DEVMGR_REFAPP_NUM_COMPARES	(10000)
#else
/* Compare are slow with C-Model. Use some smaller count */
#define NLM_DEVMGR_REFAPP_NUM_COMPARES	(1000)

#endif

/* LTR number used by the test */
#define	NLM_DEVMGR_REFAPP_LTR_NUM		(0)

#ifdef NLM_XLP

/* Start and end context addresses used by the application. Applicable only with XlpXpt */
#define	NLM_DEVMGR_REFAPP_CONTEXT_ADDRESS_START	(0)
#define	NLM_DEVMGR_REFAPP_CONTEXT_ADDRESS_END	(1023)

#endif

/* Add-thread and Search-thread are bound to cpus/cores. Applicable only for Linux */
#define	NLM_DEVMGR_REFAPP_ADD_THREAD_CPU		(0)
#define	NLM_DEVMGR_REFAPP_SEARCH_THREAD_CPU		(1)

#define NLM_DEVMGR_REFAPP_NUM_OF_BLK_SEL_LTR	(NLMDEV_NUM_ARRAY_BLOCKS / 64)
#define NLM_DEVMGR_REFAPP_NUM_OF_PS_LTR   		(NLMDEV_NUM_ARRAY_BLOCKS / 32)

typedef enum NlmDevMgrRefApp_XptMode_e
{
#ifdef NLM_XLP
    NLM_XLPXPT_MODE,
#endif

    NLM_SIMULATION_MODE,
    NLM_BCM_CALADAN3_MODE

} NlmDevMgrRefApp_XptMode;


typedef struct NlmDevMgrRefAppBlkToRsltPortMap
{
    NlmDev_parallel_search_t m_psNum;    /* Specifies result port number mapped to the blk */
} NlmDevMgrRefAppBlkToRsltPortMap;

typedef struct NlmDevMgrRefAppSuperBlkToKeyMap
{
    NlmDev_key_t m_keyNum;    /* Specifies key number mapped to the super blk */

} NlmDevMgrRefAppSuperBlkToKeyMap;

typedef struct NlmDevMgrRefAppKeyConstructMap
{
    nlm_u8 m_startByte[NLMDEV_NUM_OF_KCR_PER_KEY * NLMDEV_NUM_OF_SEGMENTS_PER_KCR]; /* Specifies the start byte of each segment of Key Construction*/
    nlm_u8 m_numOfBytes[NLMDEV_NUM_OF_KCR_PER_KEY * NLMDEV_NUM_OF_SEGMENTS_PER_KCR]; /* Specifies number of bytes each segment is */

} NlmDevMgrRefAppKeyConstructMap;

typedef struct NlmDevMgrRefAppSrchAttrs
{
    NlmDevDisableEnable m_blkEnable[NLMDEV_NUM_ARRAY_BLOCKS];
    NlmDevMgrRefAppBlkToRsltPortMap m_rsltPortBlkMap[NLMDEV_NUM_ARRAY_BLOCKS]; /* Contains the srch attributes associated with each blk */
    NlmDevMgrRefAppSuperBlkToKeyMap m_keySBMap[NLMDEV_NUM_SUPER_BLOCKS]; /* Contains Super Blk to Key mapping for each super blk */
    NlmDevMgrRefAppKeyConstructMap m_keyConstructMap[NLMDEV_NUM_KEYS]; /* Contains the Key Construction mapping for each key */
    nlm_u8 m_bmrNum[NLMDEV_NUM_PARALLEL_SEARCHES];   /* Specifies BMR number used for each srch */ 

}NlmDevMgrRefAppSrchAttrs;



typedef struct NlmDevMgrRefApp_s
{
    NlmCmAllocator  *alloc_p;
	NlmDevMgrRefApp_XptMode xpt_mode;

	NlmXpt			*xpt_p;
    void			*devMgr_p;
    void			*dev_p; 

	NlmCmMtThreadId devThrdId[NLM_DEVMGR_REFAPP_THREAD_COUNT];
	nlm_u32			threadId[NLM_DEVMGR_REFAPP_THREAD_COUNT];

	nlm_u32			entryNum;
	nlm_u16			opr_mode;
	nlm_u8			flag;

} NlmDevMgrRefApp;

#endif
