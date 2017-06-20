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
 
#ifndef INCLUDED_NLMXPT_H
#define INCLUDED_NLMXPT_H

/*@@NlmXpt Module
 *
 *  Summary
 *  This module contains the functions and data structures for generic
 *  transport layer.
 *  It calls the device specific transport routines internally. All requests to
 *  underlying devices goes through this layer. Upper layer calls
 *  NlmXpt__GetRequest routine to get a pointer to a request object which needs
 *  to be initialized  appropriately by the caller. Routine
 *  NlmXpt__ServiceRequests is called to trigger execution of the requests
 *  already in the queue.
 *  A "request" is roughly a structure that has a request code (to indicate
 *  read/write/compare operation, the data for the request and address on device
 *  etc.
 *  Appropriate device specific routines are called by xpt to execute the
 *  requests.
 */

#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmerrorcodes.h"

#if defined NLM_MT_OLD || defined NLM_MT
#include "nlmcmmt.h"
#endif


#define     MAX_COMP_RESULT         (4)
#define     MAX_READ_SIZE           (80)
#define     INVALID_VAL             (-1)
#define     MAX_DATA_LEN_BYTES      (80)
#define     MAX_CTX_ADDR            (4)
#define     XPT_MAX_MESSAGE_SIZE    (255)
#define     INVALID_CTX_ADDR        (0xffff)
#define     XPT_MAGIC_NUMBER        (0x12344)

/*
 * NlmXptRq structure describes a request for read/write/compare operations. This
 * m_results should hold the address for a  valid Result structure and this
 * address should be initialized by caller only.
 */

typedef struct NlmXptRqt_t
{
	nlm_u8	m_opcode[4];
	nlm_u32	m_data_len;
	nlm_u16	m_ctx_addr[MAX_CTX_ADDR];
	
#ifdef NLM_NO_MEMCPY_IN_XPT
	nlm_u32	m_address;
	nlm_u8*	m_data_p;
	nlm_u8*	m_mask_p;
#else
	nlm_u8 	m_data[MAX_DATA_LEN_BYTES];
#endif

	void *m_result;
} NlmXptRqt;

/*
 * NlmRequestId_t is used to identify a request. The xpt layer will issue a
 * unique request id to each request structure  returned to a caller.
 */

typedef struct NlmRequestId_t
{
    nlm_u32 m_giga_count;
    nlm_u32 m_count;
}NlmRequestId;

struct NlmXpt_operations_t;
typedef struct NlmXpt_t
{
    struct NlmXpt_operations_t *m_oprs;
    void *m_derived;
    NlmCmAllocator *m_alloc;
    
#if defined NLM_MT_OLD || defined NLM_MT
    nlm_u32 m_xpt_chanel_id[NLMNS_MT_MAXNUM_THREADS];
#else
    nlm_u32 m_xpt_chanel_id;
#endif

    NlmBool m_locked;
    nlm_u32 xpt_magic;

}NlmXpt;

/*
 * NlmXpt_t is the generic xpt structure and it will have a pointer to the
 * actual XPT implementation structure. NlmXpt_operations will have pointers to
 * various functions.
 */

typedef struct NlmXpt_operations_t
{
    void *(*NlmXpt__GetFirstAvailResult)(void *, NlmReasonCode *);
    void *(*NlmXpt__GetResult)(void*, NlmXptRqt *, NlmReasonCode *);
    void  (*NlmXpt__DiscardResult)(void *, NlmXptRqt *, NlmReasonCode *);
    void *(*NlmXpt__GetResultForRqtId)(void *, NlmRequestId *, NlmReasonCode *);
    NlmXptRqt *(*NlmXpt__GetRequest)(void *, NlmReasonCode *);
    void  (*NlmXpt__ResetRequests)(void *, NlmReasonCode *);
    void  (*NlmXpt__ServiceRequests)(void *, NlmReasonCode *);
    NlmRequestId *(*NlmXpt__GetRequestId)(void *,NlmXptRqt *, NlmReasonCode *);
    NlmErrNum_t (*NlmXpt__ResetDevice) (void *, NlmReasonCode *);
    NlmErrNum_t (*NlmXpt__LockConfig)(void *, nlm_u32 , NlmReasonCode *);
}NlmXpt_operations;
/*
 * NlmXpt__GetFirstAvailResult returns the first result available results.
 * Suppose, caller issued requests A,B,C and D and B,C,D were completed at the
 * time of call to this routine. Then the result for request B, which is the
 * first available result, will be returned.
 */

extern void *
NlmXpt__GetFirstAvailableResult(NlmXpt *self,
      NlmReasonCode *);

/*
 * NlmXpt__DiscardResult will simply cause a rqt object to be moved from in_use
 * list to the free list.
 * It may be called when some error encountered while formatting the requests or
 * before issuing ServiceRequest.
 */
extern void
NlmXpt__DiscardResult(
    NlmXpt*       self,
    NlmXptRqt*     rqt,
    NlmReasonCode *o_reason);

/*
 * NlmXpt__GetResult sends a filled in result structure to upper layer of
 * the s/w. It will return NULL if (a) the NlmXptRqt onbject is not in the
 * in_use request list, or, (b) the request is not completed.
 * If the NlmXptRqt object passed to the function is NULL, then the result for the
 * first request in the in_use request list is returned.
 * If NlmXpt__GetResult is returning a non-null result, then the corresponding
 * request object will be moved to free request list.
 */

extern void*
NlmXpt__GetResult(
    NlmXpt*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode*   o_reason
    ) ;

/*
 * NlmXpt__GetResultForRqtId sends a filled in result structure to upper layer
 * of the s/w. It will return NULL if (a) the rqtid passed is Null ,or,(b) there
 * is no request with that rqtid available in the queue, or , (c) the request
 * with that request id is not serviced.
 * If it returns a non-null result, the corresponding request object is moved to
 * free list.
 */

extern void*
NlmXpt__GetResultForRqtId(
    NlmXpt*              self,
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode *o_reason
    ) ;


/*
 * NlmXpt__GetXptRequest returns a request object to the caller. Null is
 * returned if there is no more free requests.
 */

NlmXptRqt*
NlmXpt__GetRequest(
    NlmXpt*     self,
    NlmReasonCode *o_reason
    ) ;

/*
 * NlmXpt__ResetRequests clears all the Requests, moves all the requests to the
 * free request list.
 */

extern void
NlmXpt__ResetRequests(
    NlmXpt*            self,
    NlmReasonCode *o_reason
    ) ;

/*
 * NlmXpt__GetRequestId returns to pointer to a request id specific for the
 * request rqt. If the derived xpt layer doesn't implement this function, it
 * will always return a NULL. Also, if rqt == NULL , or rqt is not in the in_use
 * request list of the lower level object, then also it will return a NULL.
 */

extern NlmRequestId *
NlmXpt__GetRequestId(
        NlmXpt  *xpt,
	NlmXptRqt*  rqt,
        NlmReasonCode *o_reason
	);
/*
 * NlmXpt__ServiceRequests triggers the execution of the requests in the
 * in_use request queue and which are not executed already.
 * Requests are executed in FIFO order.
 */

void
NlmXpt__ServiceRequests(
    NlmXpt*            self,
    NlmReasonCode *o_reason
    ) ;
/*
 * NlmXpt__ResetDevice resests the device and any work request submitted earlier
 * will be cleared.
 */

NlmErrNum_t NlmXpt__ResetDevice(
        NlmXpt* self,
        NlmReasonCode *o_reason
);

/*
 * NlmXpt__LockConfig locks an XPT configuration and this configuration should
 * not be modified in future.
 */

NlmErrNum_t NlmXpt__LockConfig(NlmXpt *self,
                nlm_u32 num_devies,
                NlmReasonCode *o_reason
                );

#endif
/*[]*/
