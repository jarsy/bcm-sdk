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
 

#ifndef INCLUDED_NLMNSLOG_H
#define INCLUDED_NLMNSLOG_H

#include <nlmcmbasic.h>
#include <nlmcmallocator.h>

#include <nlmcmexterncstart.h>

/* Size in bytes of the buffer that stores deleted prefix info */
#define NLMNSLOG_MAX_DELETE_BUFFER_SIZE  (5*1024*1024)

/*Max size of the event buffer in bytes */
#define NLMNSLOG_MAX_EVENT_BUFFER_SIZE (100 * 1024)

#define NLMNSLOG_MAX_NR_OF_ASSERTS (200)


void 
NlmNsLog__Create(
	void* privatePtr_p,
	NlmCmAllocator * alloc_p,
	nlm_u32 maxDeleteBufferSize,
	nlm_u32 maxEventBufferSize);

void 
NlmNsLog__Destroy(void);

void 
NlmNsLog__Configure(
	NlmBool enableDeletePfxLog,
	NlmBool enableAssertLog,
	NlmBool enableEventLog 
	);

void
NlmNsLog__PrintStats(
	const nlm_8 * fileName_p,
	NlmBool isAppendMode
	);


void
NlmNsLog__PrintInsertPfxInfo (
	const nlm_8 * fileName_p,
	NlmBool isAppendMode,
	nlm_u32 startLogMsgNr,
	nlm_u32 endLogMsgNr, 
	nlm_u8* tblIdStr);


void
NlmNsLog__PrintDeletePfxInfo (
	const nlm_8 * fileName_p,
	NlmBool isAppendMode,
	nlm_u32 startLogMsgNr,
	nlm_u32 endLogMsgNr
	);

void
NlmNsLog__PrintAssertInfo (
	const nlm_8 * fileName_p,
	NlmBool isAppendMode
	);

void
NlmNsLog__PrintEventInfo (
	const nlm_8 * fileName_p,
	NlmBool isAppendMode,
	nlm_u32 startLogMsgNr,
	nlm_u32 endLogMsgNr
	);

void
NlmNsLog__PrintDebugInfo (
	const nlm_8 * fileName_p,
	NlmBool isAppendMode);
#include <nlmcmexterncend.h>

#endif /* INCLUDED_NLMNSLOG_H */
