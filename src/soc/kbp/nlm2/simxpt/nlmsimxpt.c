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
 #include <nlmsimxpt.h>
#include <nlmmdl_api.h>
#include <nlmarch.h>
#include <nlmcmutility.h>

#ifdef NLM_MT_XPT 
 #include "nlmcmmt.h"
 #endif
/*
 * nlmsimxpt module implementes the operations defined by base xpt module to
 * work with the cmodel.
 * If the max_rqt_count is 1 and max_rslt_count is 1, then we don't maintain a
 * queue for the reqests and results but instead use m_single_rqt_p. This helps
 * avoid the queuing overhead
 */

/*
 * NlmXptRqtQue is a list of requests
 */
typedef struct NlmSimXptRqt_t {
    NlmXptRqt m_nlm_rqt;
    NlmBool m_done;            /* If the request is serviced */
    NlmRequestId  m_rqt_id;          /* Request id assigned by the xpt */
    struct NlmSimXptRqt_t *next;
    nlm_u32 simxpt_rqt_magic;
}NlmSimXptRqt;

typedef struct NlmXptRqtQue_t {
    nlm_u32 count;  /*how many results on the que */
    NlmSimXptRqt *head;   /*head of the queue*/
    NlmSimXptRqt *tail;   /*tail of the queue*/
} NlmXptRqtQue;



typedef struct NlmSimXpt_t {
    void*          m_client_p;/*
     * Pointer to lower level client
     * sepcific object, eg. Cmodel object.
     */
    NlmXpt*                xpt;    /* Pointer to virtual xpt object */
    NlmCmAllocator*        m_alloc_p; /* General purpose memory Allocator */
    nlm_u16                m_rbus_mode; /* Result Bus mode */
    nlm_u32                m_max_rqt_count; /*Max request count */
    nlm_u32                m_max_rslt_count; /* Max rslt cnt    */
    NlmXptRqtQue           m_in_use_requests; /* Request being used  */
    NlmXptRqtQue           m_free_requests; /* Free requests */
#if !defined NLM_MT_OLD && !defined NLM_MT
    NlmSimXptRqt*          m_single_rqt_p; /* Used for storing a single request in flat xpt without queuing */
#else
    NlmSimXptRqt*		   m_single_rqt_p[NLMNS_MT_MAXNUM_THREADS]; /* Used for storing a single request in flat xpt without queuing */
#endif
   NlmRequestId           m_next_rqt_id;
    nlm_u16                m_oprmode;
    nlm_u16                m_speedmode;
    nlm_u32                simxpt_magic;
    nlm_u32                m_devType;
#ifdef NLM_MT_XPT 
 #ifdef WIN32
    HANDLE   m_xpt_spinLock;
 #else
    /*NlmCmMtMutex m_xpt_mutex; */
	NlmCmMtSpinlock   m_xpt_spinLock;
 #endif
#endif
}NlmSimXpt;

/*
 * MACRO definitions
 */

#define NlmSimXpt__GetInUseRequestCount(self) ((self)->m_in_use_requests.count)
#define NlmSimXpt__GetFreeRequestCount(self) ((self)->m_free_requests.count)
#define NlmSimXpt__GetXpt(self)  (self->xpt)

/*
 * The magic numbers are used to verify if a particular object is of the same
 * object type as the one we are looking for.
 */

#define NLM_SIMXPT_MAGIC_NUMBER     0x67890
#define NLM_SIMXPT_RQT_MAGIC_NUMBER 0x67891

#define IS_SIMXPT(x) ((x) && ((x)->simxpt_magic == NLM_SIMXPT_MAGIC_NUMBER))
#define IS_SIMXPTRQT(x) ((x) && ((x)->simxpt_rqt_magic == NLM_SIMXPT_RQT_MAGIC_NUMBER))

#define MAXIMIZE_ERROR_SEVERITY(x, y) \
      if (x) {if ((*x != NLMRSC_OPR_FAILURE) &&(NLMRSC_OPR_FAILURE == y))\
                       *x = y;\
          else if (*x != NLMRSC_INVALID_PARAM  && (NLMRSC_INVALID_PARAM == y))\
                            *x = y;}

#define MOVE_TO_FREE_LIST(inuse, element, freelist) \
                NlmSimXpt__TakeOutRequest(inuse, element);\
                NlmCm__memset(element, 0, sizeof(NlmSimXptRqt));\
                NlmSimXpt__AddToRequestQue(freelist, element)

/*
 * Functiondeclarations
 */

static void NlmSimXpt__FreeRequestQue(
        NlmCmAllocator *alloc,
        NlmXptRqtQue *rq);

static void NlmSimXpt__AddToRequestQue(
        NlmXptRqtQue *rque,
        NlmSimXptRqt *rqt);

static void NlmSimXpt__pvt_destroy(
    void *self
);


static void NlmSimXpt__TakeOutRequest(
        NlmXptRqtQue *rque,
        NlmSimXptRqt *rqt
);

static NlmSimXptRqt* NlmSimXpt__RemoveLastRequest(
        NlmXptRqtQue *rque
);
static void NlmSimXpt__AssignRequestId(
    NlmSimXpt *self,
    NlmSimXptRqt *simrqt);

static NlmSimXptRqt *NlmSimXpt__NextToExecute(
    NlmSimXpt *self
);


static nlm_u32 NlmSimXpt__GetAvailableResultCount(
    NlmSimXpt *self
);


static void* NlmSimXpt__GetFirstAvlResult(
    void *self,
    NlmReasonCode *o_reason
);

static void  NlmSimXpt__ExecuteRequest(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmReasonCode *o_reason
);


static void NlmSimXpt__WriteToDevice(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmReasonCode *o_reason
);

static void NlmSimXpt__ReadFromDevice(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmBool is_x_read,
    NlmReasonCode *o_reason
);

static void NlmSimXpt__WriteToDeviceCtxBf(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmReasonCode *o_reason
);

static void NlmSimXpt__CompareData(
        NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmBool comp1,
        NlmReasonCode *o_reason
);

static void*  NlmSimXpt__GetResult(
void*              self,
        NlmXptRqt*              rqt,
        NlmReasonCode *o_reason
);

static NlmRequestId *
NlmSimXpt__GetRequestId(
        void *xpt,
        NlmXptRqt *rqt,
        NlmReasonCode *o_reason
);
static void NlmSimXpt__PerformNOP(
        NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt);

static NlmErrNum_t NlmSimXpt__ResetDevices(
        void *self,
        NlmReasonCode *o_reason
);

static NlmErrNum_t NlmSimXpt__LockConfig(
void *self,
        nlm_u32 num_devies,
        NlmReasonCode *o_reason
);


/*******************************************************************************
 * Function : NlmSimXpt__GetFirstAvlResult
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__GetFirstAvlResult returns a filled in result structure to
 * upper layer of the s/w. It will return NULL if no results are available,
 * OTHERWISE it will return the first result available.
 ******************************************************************************/
static void* NlmSimXpt__GetFirstAvlResult(
    void *self,
    NlmReasonCode *o_reason
)
{

    NlmSimXpt  *simxpt;
    NlmSimXptRqt *temp = 0;
    simxpt = (NlmSimXpt *)self;
    (void)o_reason;

    if (!IS_SIMXPT(simxpt))
        return NULL;

	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
		void* result;

#if !defined NLM_MT_OLD && !defined NLM_MT
		temp = simxpt->m_single_rqt_p;
#else
#ifdef NLM_NETOS
		nlm_u32 threadID = 0;
		threadID = NlmCmMt__GetThreadID(getpid());
#endif

		temp = simxpt->m_single_rqt_p[threadID];
#endif
		result = temp->m_nlm_rqt.m_result;

		return result;
	}

    if (!NlmSimXpt__GetAvailableResultCount(simxpt))
        return NULL;


    temp = simxpt->m_in_use_requests.head;

    while (temp) {
        void *result;

        if (temp->m_done) {
            result = temp->m_nlm_rqt.m_result;
            MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests,
                    temp, &simxpt->m_free_requests);
            return result;

        }
        temp = temp->next;
    }

    return NULL;
}



/*******************************************************************************
 * Function : NlmSimXpt__GetResult
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXpt      *rqt           = Pointer to NlmRequest object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__GetResult returns a filled in result structure to upper layer of
 * the s/w. It will return NULL if (a) the NlmXptRqt onbject is not in the
 * in_use request list, or, (b) the request is not completed.
 * If the NlmXptRqt object passed to the function is NULL, then the result for
 * the first request in the in_use request list is returned.
 * If NlmSimXpt__GetResult is returning a non-null result,then the corresponding
 * request object will be moved to free request list.
 ******************************************************************************/
static void*  NlmSimXpt__GetResult(
    void*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode *o_reason
)
{
    NlmSimXpt  *simxpt;
    NlmSimXptRqt *simrqt, *temp = NULL;


    simrqt = (NlmSimXptRqt *)rqt;
    simxpt = (NlmSimXpt *)self;

    if (!IS_SIMXPT(simxpt) || !IS_SIMXPTRQT(simrqt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NULL;
    }

	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
#if !defined NLM_MT_OLD && !defined NLM_MT
		if (simxpt->m_single_rqt_p == simrqt)
#else
#ifdef NLM_NETOS
	nlm_u32 threadID = 0;
	threadID = NlmCmMt__GetThreadID(getpid());
#endif
	if (simxpt->m_single_rqt_p[threadID] == simrqt)
#endif
		{
				void *result;
				if (!simrqt->m_done)
					return NULL;
				result = simrqt->m_nlm_rqt.m_result;
				
#ifdef NLM_MT_XPT 
 			NlmCmMt__SpinUnlock(&simxpt->m_xpt_spinLock);
#endif

				return result;
		}
		return NULL;
	}


    if (!NlmSimXpt__GetAvailableResultCount(simxpt)) {
        if (o_reason)
            *o_reason = NLMRSC_NORSLT_AVBL;
        return NULL;
    }
    temp = simxpt->m_in_use_requests.head;

    while (temp) {
        if (temp == simrqt) {
            void *result;
            if (!simrqt->m_done)
                return NULL;
            result = simrqt->m_nlm_rqt.m_result;

            MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests,
                    simrqt, &simxpt->m_free_requests);
			
#ifdef NLM_MT_XPT 
 	NlmCmMt__SpinUnlock(&simxpt->m_xpt_spinLock);
#endif
            return result;
        }
        temp = temp->next;
    }

    return NULL;
}


/*******************************************************************************
 * Function : NlmSimXpt__GetRequestId
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXpt      *rqt           = Pointer to NlmRequest object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__GetRequestId returns request_id associated with a request object, The
 * Request must be present in in_use queue, else NULL is returned.
 ******************************************************************************/
static NlmRequestId *
        NlmSimXpt__GetRequestId(
        void *self,
        NlmXptRqt *rqt,
        NlmReasonCode *o_reason
)
{
    NlmSimXptRqt *simrqt;

    simrqt = (NlmSimXptRqt *) rqt;
    (void)self;

    if (!IS_SIMXPTRQT(simrqt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NULL;
    }
    return &simrqt->m_rqt_id;
}

/*******************************************************************************
 * Function : NlmSimXpt__GetResultForRqtId
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmRequestId  *rqtid       = Pointer to NlmRequestId object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__GetResultForRqtId sends a filled in result structure to upper layer
 * of the s/w. It will return NULL if (a) the rqtid passed is Null ,or,(b) there
 * is no request with that rqtid available in the queue, or , (c) the request
 * with that request id is not serviced.
 * If it returns a non-null result, the corresponding request object is moved to
 * free list.
 ******************************************************************************/
static void* NlmSimXpt__GetResultForRqtId(
        void*              self,
        NlmRequestId        *rqtid, /* the request id    */
        NlmReasonCode       *o_reason
)
{
    NlmSimXpt *simxpt;
    NlmSimXptRqt *rqt;
    void *result = NULL;
    (void)o_reason;

    if (!self || !rqtid)
        return NULL;

    simxpt = (NlmSimXpt *) self;

    if (!IS_SIMXPT(simxpt)) return NULL;

	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
#if !defined NLM_MT_OLD && !defined NLM_MT
		rqt = simxpt->m_single_rqt_p;
#else
#ifdef NLM_NETOS
	nlm_u32 threadID = 0;
	threadID = NlmCmMt__GetThreadID(getpid());
#endif
		rqt = simxpt->m_single_rqt_p[threadID];
#endif

		if (!NlmCm__memcmp(&rqt->m_rqt_id, rqtid, sizeof(NlmRequestId)) &&
			rqt->m_done)
		{
			result = rqt->m_nlm_rqt.m_result;

			return result;

		}

		return NULL;
	}

    if (!NlmSimXpt__GetInUseRequestCount(simxpt))
        return NULL;

    rqt = simxpt->m_in_use_requests.head;

    while (rqt) {
        if (!NlmCm__memcmp(&rqt->m_rqt_id, rqtid, sizeof(NlmRequestId)))
            break;

        rqt = rqt->next;

    }

    if (rqt && rqt->m_done) {
        result = rqt->m_nlm_rqt.m_result;

        MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, rqt,
                &simxpt->m_free_requests);
        return result;

    }

    return NULL;
}


/*******************************************************************************
 * Function : NlmSimXpt__GetRequest
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__GetXptRequest returns a request object to the caller. Null is
 * returned if there is no more free requests. The returned request is moved to
 * in_use list.
 ******************************************************************************/
static NlmXptRqt* NlmSimXpt__GetRequest(
        void*     self,
        NlmReasonCode *o_reason
)
{
    NlmSimXpt *simxpt = 0;
    NlmSimXptRqt *simrqt = 0;

    if (!self || !IS_SIMXPT((NlmSimXpt *)self)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;

        return NULL;
    }

    simxpt = (NlmSimXpt *)self;

#ifdef NLM_MT_XPT 
	 NlmCmMt__SpinLock(&simxpt->m_xpt_spinLock);
 #endif
	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
#if !defined NLM_MT_OLD && !defined NLM_MT
		simrqt = simxpt->m_single_rqt_p;
#else
#ifdef NLM_NETOS
		nlm_u32 threadID = 0;
		threadID = NlmCmMt__GetThreadID(getpid());
#endif

		simrqt = simxpt->m_single_rqt_p[threadID];
#endif
		NlmCm__memset(simrqt, 0, sizeof(NlmSimXptRqt));

		simrqt->simxpt_rqt_magic = NLM_SIMXPT_RQT_MAGIC_NUMBER;

		return (NlmXptRqt *) simrqt;
	}

    simrqt = NlmSimXpt__RemoveLastRequest(&simxpt->m_free_requests);

    if (!simrqt) {
        if (o_reason)
            *o_reason = NLMRSC_NOFREE_RQST;
        return NULL;
    }

    NlmCm__memset(simrqt, 0, sizeof(NlmSimXptRqt));

    NlmSimXpt__AssignRequestId(simxpt, simrqt);

    simrqt->simxpt_rqt_magic = NLM_SIMXPT_RQT_MAGIC_NUMBER;

    NlmSimXpt__AddToRequestQue(&simxpt->m_in_use_requests, simrqt);

    return (NlmXptRqt *) simrqt;

}

/*******************************************************************************
 * Function : NlmSimXpt__RemoveLastRequest
 *
 * Parameters:
 * NlmXptRqtQue *rque      = Pointer to a request queue.
 *
 * Summary:
 * NlmSimXpt__RemoveLastRequest: Takes out the last element of a rquest queue
 * and returns that element.
 ******************************************************************************/
static NlmSimXptRqt* NlmSimXpt__RemoveLastRequest(
    NlmXptRqtQue *rque
)
{
    NlmSimXptRqt *simrqt;

    if (rque->count == 0) return NULL;

    simrqt = rque->head;

    NlmSimXpt__TakeOutRequest(rque, simrqt);

    return simrqt;
}


/*******************************************************************************
 * Function : NlmSimXpt__AssignRequestId
 *
 * Parameters:
 * NlmSimXpt *self         = Pointer to an NlmSimXpt object
 * NlmSimXptRqt *simrqt    = Poniter to an NlmSimXptRqt object
 *
 * Summary:
 * Asssign a request Id to a NlmSimXptRqt object
 ******************************************************************************/
static void NlmSimXpt__AssignRequestId(
    NlmSimXpt *self,
        NlmSimXptRqt *simrqt)
{
    if (!self || !simrqt) return;

    NlmCm__memcpy(&simrqt->m_rqt_id, &self->m_next_rqt_id,
            sizeof(NlmRequestId));

    if (self->m_next_rqt_id.m_count < (0xFFFFFFFF - 0x1)) {
        self->m_next_rqt_id.m_count++;
    }
    else {
        self->m_next_rqt_id.m_count = 0;
        self->m_next_rqt_id.m_giga_count++;
    }
}

/*******************************************************************************
 * Function : NlmSimXpt__ResetRequests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__ResetRequests clears all the Requests, moves all the requests to the
 * free request list. The requests are just moved to the free_list irrespective
 * of they are executed or not.
 ******************************************************************************/
static void NlmSimXpt__ResetRequests(
    void*    self,
    NlmReasonCode *o_reason
)
{
    NlmSimXpt *simxpt;
    simxpt = (NlmSimXpt *) self;
    (void)o_reason;

    if (!IS_SIMXPT(simxpt)) return;

	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
		return ;
	}

    while (NlmSimXpt__GetInUseRequestCount(simxpt)) {
        NlmSimXptRqt *rqt;
        rqt = simxpt->m_in_use_requests.head;
        NlmSimXpt__TakeOutRequest(&simxpt->m_in_use_requests, rqt);
        NlmCm__memset(rqt, 0, sizeof(NlmSimXpt));
        NlmSimXpt__AddToRequestQue(&simxpt->m_free_requests, rqt);
    }
}


/*******************************************************************************
 * Function : NlmSimXpt__NextToExecute
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmSimXpt object
 *
 * Summary:
 * Returns the next request to be executed from the in_use request queue.
 ******************************************************************************/
static NlmSimXptRqt *NlmSimXpt__NextToExecute(
    NlmSimXpt *self
)
{

    NlmSimXptRqt *simrqt;

    if (!IS_SIMXPT(self) ||
            !NlmSimXpt__GetInUseRequestCount(self)) return NULL;

    simrqt = self->m_in_use_requests.head;

    while (simrqt) {
        if (!simrqt->m_done) return simrqt;
        simrqt  = simrqt->next;
    }
    return NULL;
}

/*******************************************************************************
 * Function : NlmSimXpt__ServiceRequests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 *  NlmSimXpt__ServiceRequests  takes  the requests from the in_use request
 *  queue and execute them one by one. The "WRITE" requests are moved to
 *  free_list while COMPARE/READ requests are left in the in_use request queue
 *  itself. The COMPARE/READ requests are moved to free_list as results of calls
 *  to "GetResult" routine. Requests are always executed in FIFO order.
 ******************************************************************************/
static void
NlmSimXpt__ServiceRequests(
    void*            self,            /* transport interface to flush */
    NlmReasonCode    *o_reason
)
{
    NlmSimXpt *simxpt;
    NlmSimXptRqt *simrqt;
    void *result = NULL;

    if (!self) return;

    simxpt = (NlmSimXpt *)self;

    if (!IS_SIMXPT(simxpt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }

    if (o_reason)
        *o_reason = NLMRSC_REASON_OK;


	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
#if !defined NLM_MT_OLD && !defined NLM_MT
		simrqt = simxpt->m_single_rqt_p;
#else
#ifdef NLM_NETOS
		nlm_u32 threadID = 0;
		threadID = NlmCmMt__GetThreadID(getpid());
#endif

		simrqt = simxpt->m_single_rqt_p[threadID];
#endif
 	   	NlmSimXpt__ExecuteRequest(simxpt, simrqt, o_reason);

#ifdef NLM_MT_XPT 

        if(simrqt->m_nlm_rqt.m_result == NULL)
        {
            NlmCmMt__SpinUnlock(&simxpt->m_xpt_spinLock);
        }
#endif
		return;
	}


    if (!NlmSimXpt__GetInUseRequestCount(simxpt)) return;


    while (NULL != (simrqt = NlmSimXpt__NextToExecute(simxpt))) {
        NlmSimXpt__ExecuteRequest(simxpt, simrqt, o_reason);
        result = simrqt->m_nlm_rqt.m_result;
    }

#ifdef NLM_MT_XPT 
    if(result == NULL)
    {
        NlmCmMt__SpinUnlock(&simxpt->m_xpt_spinLock);
    }
#endif
}

/*******************************************************************************
 * Function : NlmSimXpt__ExecuteRequest
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__ExecuteRequest Invokes appropriate routine to execute the commands
 * described in simrqt
 ******************************************************************************/

static void NlmSimXpt__ExecuteRequest(
        NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *p;

    if (!simrqt || !IS_SIMXPTRQT(simrqt) || !simxpt || !IS_SIMXPT(simxpt))
        return;

    p = (NlmXptRqt *) simrqt;

	if ((NLM_OPCODE_PIO_WRITE_BITS_8_6 == p->m_opcode[0]) &&
            (NLM_OPCODE_PIO_WRITE_BITS_5_0 == p->m_opcode[1])) {
        NlmSimXpt__WriteToDevice(simxpt, simrqt, o_reason);
    }
    else if ((NLM_OPCODE_CBWRITE_BITS_8_6 == p->m_opcode[0]) &&
            (NLM_OPCODE_CBWRITE_BITS_5_0 == p->m_opcode[1])) {
        NlmSimXpt__WriteToDeviceCtxBf(simxpt, simrqt, o_reason);
    }

    else if ((NLM_OPCODE_CBWRITE_CMP1_BITS_8_6 == p->m_opcode[0]) &&
            (NLMDEV_NUM_LTR_SET >  p->m_opcode[1])) {
        NlmSimXpt__CompareData(simxpt, simrqt, NLMTRUE, o_reason);
    }

    else if ((NLM_OPCODE_CBWRITE_CMP2_BITS_8_6 == p->m_opcode[0]) &&
            (NLMDEV_NUM_LTR_SET >  p->m_opcode[1])) {
        NlmSimXpt__CompareData(simxpt, simrqt, NLMFALSE, o_reason);

    }
    else if ((NLM_OPCODE_NOP_BITS_8_6 == p->m_opcode[0]) &&
            (NLM_OPCODE_NOP_BITS_5_0 == p->m_opcode[1])) {
        NlmSimXpt__PerformNOP(simxpt, simrqt);
    }
    else if ((NLM_OPCODE_PIO_READ_X_BITS_8_6 == p->m_opcode[0]) &&
            (NLM_OPCODE_PIO_READ_X_BITS_5_0 == p->m_opcode[1])) {
        NlmSimXpt__ReadFromDevice(simxpt, simrqt, NLMTRUE, o_reason);

    }
    else if ((NLM_OPCODE_PIO_READ_Y_BITS_8_6 == p->m_opcode[0]) &&
            (NLM_OPCODE_PIO_READ_Y_BITS_5_0 == p->m_opcode[1])) {
        NlmSimXpt__ReadFromDevice(simxpt, simrqt, NLMFALSE, o_reason);

    }
    else {
        nlm_u32 sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);

		if(simxpt->m_max_rqt_count != 1 ||
			simxpt->m_max_rslt_count != 1)
		{
			MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
					&simxpt->m_free_requests);
		}
    }
}

/* This function converts the compare results provided by C-Model
to the format required by Device Manager */

static void NlmSimXpt__pvt_FrmtCompareResult(nlm_u8 *o_compareRslt,
                                            nlm_u8 *compareRslt
                                            )
{
    WriteBitsInArray(o_compareRslt,
                     NLMDEV_CMP_RSLT_LEN_IN_BYTES, 
                     22, 0, 
                     ReadBitsInArrray(compareRslt,
                                      NLM_MDL_RESULT_LEN_BYTES,
                                      22, 0));

    WriteBitsInArray(o_compareRslt,
                     NLMDEV_CMP_RSLT_LEN_IN_BYTES, 
                     30, 30, 
                     ReadBitsInArrray(compareRslt,
                                      NLM_MDL_RESULT_LEN_BYTES,
                                      23, 23));

}
/*******************************************************************************
 * Function : NlmSimXpt__CompareData
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmBool comp1             = if it is a compare1 of comapre2 operation
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__CompareData  sends a "COMPARE" command to the underlying device.
 ******************************************************************************/

static void NlmSimXpt__CompareData(
NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmBool comp1,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt;
    nlm_u32 retstat = NLMFAIL;
    NlmMdlCBWriteCmpParam cmpParam;
    NlmMdlCascade *cascade_p;
    nlm_u32 sev_param;
    nlm_u8 *cmpRslt;
	nlm_u8 *data_ptr = NULL;

    if (!IS_SIMXPT(simxpt) || !IS_SIMXPTRQT(simrqt)) {
        sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
        return;
    }

    cascade_p  =  (NlmMdlCascade *)simxpt->m_client_p;
    xptRqt = (NlmXptRqt *) simrqt;
    cmpParam.m_ltrNum = xptRqt->m_opcode[1];

    cmpParam.m_cbData.m_addr[1] = (nlm_u8)(xptRqt->m_ctx_addr[0] & 0xFF);
    cmpParam.m_cbData.m_addr[0] = (nlm_u8)(xptRqt->m_ctx_addr[0] >> 8);

#ifdef NLM_NO_MEMCPY_IN_XPT
	data_ptr = xptRqt->m_data_p;
#else
	data_ptr = xptRqt->m_data;
#endif

    /* Construct the CB Data based on datalen*/
    if(xptRqt->m_data_len % NLMDEV_CB_WIDTH_IN_BYTES)
    {
         /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
        cmpParam.m_cbData.m_dataLen = (nlm_u16)((xptRqt->m_data_len/NLMDEV_CB_WIDTH_IN_BYTES) + 1);
        cmpParam.m_cbData.m_dataLen = cmpParam.m_cbData.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;

        NlmCm__memset(cmpParam.m_cbData.m_data, 0, cmpParam.m_cbData.m_dataLen - xptRqt->m_data_len);
        NlmCm__memcpy(&cmpParam.m_cbData.m_data[cmpParam.m_cbData.m_dataLen - xptRqt->m_data_len],
                      data_ptr,
                      xptRqt->m_data_len);		
    }
    else
    {
        cmpParam.m_cbData.m_dataLen = (nlm_u16)(xptRqt->m_data_len);
        NlmCm__memcpy(cmpParam.m_cbData.m_data, data_ptr, xptRqt->m_data_len);
    }

    if (NLMTRUE == comp1)
        retstat = NlmModel__InstCBWriteCmp1(cascade_p, &cmpParam);
    else /*COMP2 */
        retstat = NlmModel__InstCBWriteCmp2(cascade_p, &cmpParam);

    if (NLMPASS != retstat)
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

    /*
     * Get the search results For Device Manager Each search result is of 32b with Bit[30] indicating HIT/MISS
     and Bits[22:0] indicating HIT Index whereas Model Returns each search result of 24b with Bit[23] 
     indicating HIT/MISS and Bits[22:0] indicating HIT Index
     */
    cmpRslt = (nlm_u8*)xptRqt->m_result;
    
    NlmSimXpt__pvt_FrmtCompareResult(cmpRslt,
                                     cmpParam.m_compResult0);

    NlmSimXpt__pvt_FrmtCompareResult(cmpRslt + NLMDEV_CMP_RSLT_LEN_IN_BYTES,
                                     cmpParam.m_compResult1);
    
    NlmSimXpt__pvt_FrmtCompareResult(cmpRslt + 2 * NLMDEV_CMP_RSLT_LEN_IN_BYTES,
                                     cmpParam.m_compResult2);

    NlmSimXpt__pvt_FrmtCompareResult(cmpRslt + 3 * NLMDEV_CMP_RSLT_LEN_IN_BYTES,
                                     cmpParam.m_compResult3);
    simrqt->m_done = 1;
}

/*******************************************************************************
 * Function : NlmSimXpt__PerformNOP
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 *
 * Summary:
 * NlmSimXpt__PerformNOP  doesn't do anything. It just move the request to free_queue.
 ******************************************************************************/
static void NlmSimXpt__PerformNOP(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt
    )
{
    if (!IS_SIMXPT(simxpt) || !IS_SIMXPTRQT(simrqt))
        return;

	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
		return;
	}

	MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
            &simxpt->m_free_requests);

}


/*******************************************************************************
 * Function : NlmSimXpt__ReadFromDevice
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmBool is_x_read         = if it is a data or mask request
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__ReadFromDevice  sends a READ command to the underlying device.
 ******************************************************************************/

static void NlmSimXpt__ReadFromDevice(
        NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmBool is_x_read,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt;
    NlmMdlPIOReadParam readParam;
    nlm_u8 *readRslt;
    nlm_u32 sev_param;
    nlm_u32 value;

    xptRqt = (NlmXptRqt *)simrqt;

    readRslt = (nlm_u8 *)xptRqt->m_result;

    if (!IS_SIMXPT(simxpt) || !IS_SIMXPTRQT(simrqt)
            || !simrqt->m_nlm_rqt.m_result || !simxpt->m_client_p)
    {
        sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);

		if(simxpt->m_max_rqt_count != 1 ||
			simxpt->m_max_rslt_count != 1)
		{
			MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests,
					simrqt,
					&simxpt->m_free_requests);
		}

        return;
    }

#ifdef NLM_NO_MEMCPY_IN_XPT
	value = xptRqt->m_address;
#else
	value = ReadBitsInArrray(xptRqt->m_data,
                             NLMDEV_REG_ADDR_LEN_IN_BYTES,
                             31, 0);
#endif

    if((value >> NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE) & 1) /* Check if the read operation is for
                                                               Database read or Reg Read*/
    {
        if(is_x_read)/* If database read see whether Read for X or Read for Y */
            readParam.m_dataType = NLM_MDL_RD_DATA_X;
        else
            readParam.m_dataType = NLM_MDL_RD_DATA_Y;
    }
    else
        readParam.m_dataType = NLM_MDL_RD_REG;

    WriteBitsInArray(readParam.m_address, NLMDEV_REG_ADDR_LEN_IN_BYTES, 31, 0, (value & 0x1FFFFFF));

    if (NLMPASS != NlmModel__InstPIORead((NlmMdlCascade *)(simxpt->m_client_p),
                &readParam))
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
        return;
    }

    /* Bit 4 of Read Result Byte 0 will indicate the VBIT with _RD_DATA_X */
    if(is_x_read)
        readRslt[0] = (nlm_u8)(readParam.m_vBit << 4);

    /* Byte 1 to 10 will have 80b data */
    NlmCm__memcpy(readRslt + 1, readParam.m_outData, NLMDEV_REG_LEN_IN_BYTES);

    simrqt->m_done = 1; /*Request complete */

}


/*******************************************************************************
 * Function : NlmSimXpt__WriteToDevice
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__WriteToDevice  sends a WRITE command to the underlying device.
 ******************************************************************************/

/* #define PIOWRITE_STATS */  /* enable this for to get number of PIO writes*/
#ifdef PIOWRITE_STATS
	nlm_u32 numPIOWrites;
#endif

static void NlmSimXpt__WriteToDevice(
NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt;
    NlmMdlPIOWriteParam wrparam;
    nlm_u32 cm_retst;
    nlm_u32 sev_param;
    nlm_u32 value;
	NlmMdlCascade *cascade_p = (NlmMdlCascade *)simxpt->m_client_p;

	nlm_u8 *data_ptr = NULL;
	nlm_u8 *mask_ptr = NULL;

    if (!IS_SIMXPT(simxpt) || !IS_SIMXPTRQT(simrqt)
            || !cascade_p)
        return;

    xptRqt = (NlmXptRqt *) simrqt;

#ifdef NLM_NO_MEMCPY_IN_XPT
	value = xptRqt->m_address;

	data_ptr = xptRqt->m_data_p;
	mask_ptr = xptRqt->m_mask_p;
#else
	/* address + data + mask */
	value = ReadBitsInArrray(xptRqt->m_data,
                            NLMDEV_REG_ADDR_LEN_IN_BYTES,
                            31, 0);

	data_ptr = xptRqt->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES; /* data */
	mask_ptr = xptRqt->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES + NLMDEV_AB_WIDTH_IN_BYTES; /* mask */
#endif

    if((value >> NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE) & 1) /* Check if Write operation is
                                                      for DB Entry or Register */
    {
        if((value >> NLMDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR) & 1) /* Get the mode of Database write */
            wrparam.m_dataType = NLM_MDL_WR_MODE_XY;
        else
            wrparam.m_dataType = NLM_MDL_WR_MODE_DM;

        if((value >> NLMDEV_AB_ENTRY_VALID_BIT_IN_ADDR) & 1) /* Get the VBIT for Database write */
            wrparam.m_vBit = NLM_MDL_VBIT_VALID;
        else
            wrparam.m_vBit = NLM_MDL_VBIT_INVALID;

        /* Actual Database Address is from Bits[24:0] */
        value = value & 0x1FFFFFF;
        WriteBitsInArray(wrparam.m_address, NLMDEV_AB_ADDR_LEN_IN_BYTES, 24, 0, value);

        /* Copy data and mask */
        NlmCm__memcpy(wrparam.m_data,
                      data_ptr,
                      NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memcpy(wrparam.m_mask,
                     mask_ptr,
                     NLMDEV_AB_WIDTH_IN_BYTES);

        cm_retst = NlmModel__InstPIOWrite(cascade_p, &wrparam);
    }
    else /* Write in register address space */
    {
        wrparam.m_dataType = NLM_MDL_WR_REG;
        wrparam.m_vBit = 0;

         /* Actual Register Address is from Bits[24:0] */
        value = value & 0x1FFFFFF;
        WriteBitsInArray(wrparam.m_address, NLM_MDL_ADDR_LEN_BYTES, 24, 0, value);

        /* Copy Register data */
        NlmCm__memcpy(wrparam.m_data,
                      data_ptr,
                      NLMDEV_REG_LEN_IN_BYTES);

        cm_retst = NlmModel__InstPIOWrite(cascade_p, &wrparam);
    }

    if (NLMPASS != cm_retst)
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

	if(simxpt->m_max_rqt_count != 1 ||
			simxpt->m_max_rslt_count != 1)
	{
		MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
				&simxpt->m_free_requests);
	}
	
	#ifdef PIOWRITE_STATS
		numPIOWrites++;
	#endif
}

/*******************************************************************************
 * Function : NlmSimXpt__WriteToDeviceCtxBf
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__WriteToDeviceCtxBf  sends a WRITE TO CTX BUFFER command to the underlying
 * device.
 ******************************************************************************/

static void NlmSimXpt__WriteToDeviceCtxBf(
        NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmReasonCode *o_reason
)
{
    NlmMdlCBWriteParam cbParam;
    NlmXptRqt *xptRqt;
    nlm_u32 ret;
    nlm_u32 sev_param;
	nlm_u8 *data_ptr = NULL;

    if (!IS_SIMXPT(simxpt) && !IS_SIMXPTRQT(simrqt)) {
        sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
        return;
    }
    xptRqt = (NlmXptRqt *) simrqt;

    /* For serial devices only CtxAddr[0] of xpt rqt structure is valid */
    cbParam.m_addr[1] = (nlm_u8)(xptRqt->m_ctx_addr[0] & 0xFF);
    cbParam.m_addr[0] = (nlm_u8)(xptRqt->m_ctx_addr[0] >> 0x8);

#ifdef NLM_NO_MEMCPY_IN_XPT
	data_ptr = xptRqt->m_data_p;
#else
	data_ptr = xptRqt->m_data;
#endif

    /* Construct the CB Data based on datalen*/
    if(xptRqt->m_data_len%NLMDEV_CB_WIDTH_IN_BYTES)
    {
        /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
        cbParam.m_dataLen = (nlm_u16)((xptRqt->m_data_len/NLMDEV_CB_WIDTH_IN_BYTES) + 1);
        cbParam.m_dataLen = cbParam.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;
        NlmCm__memset(cbParam.m_data, 0, cbParam.m_dataLen - xptRqt->m_data_len);
        NlmCm__memcpy(&cbParam.m_data[cbParam.m_dataLen - xptRqt->m_data_len],
                      data_ptr,
                      xptRqt->m_data_len);
    }
    else
    {
        cbParam.m_dataLen = (nlm_u16)(xptRqt->m_data_len);
		NlmCm__memcpy(cbParam.m_data, data_ptr, xptRqt->m_data_len);
    }

    ret = NlmModel__InstCBWrite((NlmMdlCascade *)simxpt->m_client_p,
                                &cbParam);

    if (NLMPASS != ret)
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

	if(simxpt->m_max_rqt_count != 1 ||
			simxpt->m_max_rslt_count != 1)
	{
		MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
				&simxpt->m_free_requests);
	}
}


/*******************************************************************************
 * Function : NlmSimXpt__GetAvailableResultCount
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 *
 * Summary:
 * NlmXpt__GetAvailableResultCount returns the available results in in_use
 * request queue.
 ******************************************************************************/
static nlm_u32 NlmSimXpt__GetAvailableResultCount(
    NlmSimXpt *self
)
{
    NlmSimXpt *simxpt;
    NlmSimXptRqt *rqt;
    nlm_u32       rqtcount = 0;

    if (!self) return 0;

    simxpt = (NlmSimXpt *) self;

    if (!NlmSimXpt__GetInUseRequestCount(simxpt))
        return 0;


    rqt = simxpt->m_in_use_requests.head;

    while (rqt) {
        if (rqt->m_done) rqtcount++;
        rqt = rqt->next;
    }

    return rqtcount;

}


/*******************************************************************************
 * Function : NlmSimXpt__DiscardResult
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmReasonCode *o_reason   = Location to save the reason code*
 * NlmXptRqt     *rqt        = Pointer to NlmXptRqt object
 *
 * Summary:
 * NlmXpt__DiscardResult removes a request from in_use queue irrespective of
 * whether the request is executed or not.
 ******************************************************************************/
static void NlmSimXpt__DiscardResult(
		void *self,
        NlmXptRqt *rqt,
        NlmReasonCode *o_reason
)
{
    NlmSimXpt *simxpt;
    NlmSimXptRqt *simrqt, *temp;

    if (!self || !rqt) return;

    simxpt = (NlmSimXpt *) self;
    simrqt = (NlmSimXptRqt *)rqt;

    if (!IS_SIMXPT(simxpt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }
    if (!IS_SIMXPTRQT(simrqt))
        goto not_allocated_by_me;

	if(simxpt->m_max_rqt_count == 1 &&
		simxpt->m_max_rslt_count == 1)
	{
		return;
	}

    if (!NlmSimXpt__GetInUseRequestCount(simxpt))
        return;


    temp = simxpt->m_in_use_requests.head;

    while (temp) {
        if (temp == simrqt) {
            MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, temp,
                    &simxpt->m_free_requests);
            return;
        }
        temp = temp->next;
    }

    not_allocated_by_me:
            if (o_reason)
                *o_reason = NLMRSC_NORQST_AVBL;
}



/*******************************************************************************
 * Function : NlmSimXpt__Create
 *
 * Parameters:
 *
 *  NlmCmAllocator         *alloc    = Pointer to General purpose memory allocator
 *  nlm_u32                devType = At present support to NLM_DEVTYPE_2
                                        and NLM_DEVTYPE_2_S; Support can be added
                                    for other devices also in future;
 *  nlm_u16                 speed_mode = High or Standard speed of operation
 *  nlm_u32                max_rqt_count, = Max request count to be in a queue
 *  nlm_u32                max_rslt_count = Max result count to be in a queue
 *  nlm_u16                 opr_mode = Normal or advanced operation mode
 *  nlm_u16                rbus_mode = Result bus mode : single/ dual
 *  nlm_u32                chan_id
 *
 * Summary:
 * NlmSimXpt__Create Creates the transport object (NlmXpt) and returns a
 * pointer to the object. It also initializes the virtual operation table
 * of NlmXpt object
 ******************************************************************************/

NlmXpt *
NlmSimXpt__Create(
        NlmCmAllocator          *alloc,         /*General purpose memory allocator */
        nlm_u32                 devType,
        nlm_u16                 speed_mode,
        nlm_u32                 max_rqt_count,  /*Max request count */
        nlm_u32                 max_rslt_count, /*Max result count*/
        nlm_u16                 opr_mode,       /* normal or sahasra mode*/
        nlm_u16                 rbus_mode,      /* result bus : single/dual mode */
        nlm_u32                 chan_id
)
{
    NlmSimXpt *simxpt = 0;
    NlmXpt    *xpt = 0;
    nlm_u32   count;
    static NlmXpt_operations xpt_operations;

    if (!alloc || (devType != NLM_DEVTYPE_2 && devType != NLM_DEVTYPE_2_S)
        || (opr_mode != NLMDEV_OPR_STANDARD && opr_mode != NLMDEV_OPR_SAHASRA)
        || (max_rqt_count < max_rslt_count))
        return NULL;

    simxpt = NlmCmAllocator__malloc(alloc, sizeof(NlmSimXpt));
    if (!simxpt) return NULL;

    NlmCm__memset(simxpt, 0, sizeof(NlmSimXpt));

    simxpt->m_alloc_p = alloc;
    simxpt->m_max_rqt_count = max_rqt_count;
    simxpt->m_max_rslt_count = max_rslt_count;
    simxpt->simxpt_magic = NLM_SIMXPT_MAGIC_NUMBER;
    simxpt->m_oprmode = opr_mode;
    simxpt->m_rbus_mode = rbus_mode;
    simxpt->m_speedmode = speed_mode;
    simxpt->m_devType = devType;
    /*Now construct the base xpt structure */

    xpt = NlmCmAllocator__calloc(alloc, 1, sizeof(NlmXpt));
    if (!xpt) {
        NlmSimXpt__pvt_destroy(simxpt);
        return NULL;
    }

    xpt_operations.NlmXpt__GetResult = NlmSimXpt__GetResult;
    xpt_operations.NlmXpt__DiscardResult = NlmSimXpt__DiscardResult;
    xpt_operations.NlmXpt__GetResultForRqtId = NlmSimXpt__GetResultForRqtId;
    xpt_operations.NlmXpt__GetRequest = NlmSimXpt__GetRequest;
    xpt_operations.NlmXpt__ResetRequests = NlmSimXpt__ResetRequests;
    xpt_operations.NlmXpt__GetRequestId = NlmSimXpt__GetRequestId;
    xpt_operations.NlmXpt__ServiceRequests = NlmSimXpt__ServiceRequests;
    xpt_operations.NlmXpt__GetFirstAvailResult = NlmSimXpt__GetFirstAvlResult;
    xpt_operations.NlmXpt__ResetDevice = NlmSimXpt__ResetDevices;
    xpt_operations.NlmXpt__LockConfig = NlmSimXpt__LockConfig;
    xpt->m_derived = simxpt;
    xpt->m_alloc = alloc;
    xpt->m_oprs = &xpt_operations;
    xpt->xpt_magic = XPT_MAGIC_NUMBER;
#if !defined NLM_MT_OLD && !defined NLM_MT
    xpt->m_xpt_chanel_id = chan_id;
#else
	xpt->m_xpt_chanel_id[0]  = chan_id;
#endif
    xpt->m_locked = NLMFALSE;

    simxpt->xpt = xpt;

    /*Now initialize the free request queue */
    count = 0;
    while (count < max_rqt_count) {
        NlmSimXptRqt *rqt;
        rqt = NlmCmAllocator__calloc(alloc, 1, sizeof(NlmSimXptRqt));
        NlmSimXpt__AddToRequestQue(&simxpt->m_free_requests, rqt);
        count++;
    }

#if !defined NLM_MT_OLD && !defined NLM_MT
	simxpt->m_single_rqt_p = NlmCmAllocator__calloc(alloc, 1, sizeof(NlmSimXptRqt));
#else
	for(count = 0; count < 	NLMNS_MT_MAXNUM_THREADS; count++)
		simxpt->m_single_rqt_p[count] = NlmCmAllocator__calloc(alloc, 
				1, sizeof(NlmSimXptRqt));

#endif
	
    return xpt;

}


/*******************************************************************************
 * Function : NlmSimXpt__pvt_destroy
 *
 * Parameters:
 *
 * void *self = Pointer to a generic NlmXpt object
 * Summary:
 * NlmSimXpt__pvt_destroy frees the simxpt object and does the necessary
 * cleanups
 ******************************************************************************/

static void NlmSimXpt__pvt_destroy(
    void *self
)
{
    NlmSimXpt *simxpt;
    NlmXpt    *xpt;
    nlm_u32 count = 0;

     (void)count;

    if (!self) return;

    simxpt = (NlmSimXpt *) self;
    xpt = simxpt->xpt;

#ifdef NLM_MT_XPT 
	/* Destroy the spin locks used */
	 NlmCmMt__SpinDestroy(&simxpt->m_xpt_spinLock,
				"NlmSimXpt_Kbp_SpinLock");
 #endif

    NlmSimXpt__FreeRequestQue(simxpt->m_alloc_p, &simxpt->m_in_use_requests);
    NlmSimXpt__FreeRequestQue(simxpt->m_alloc_p, &simxpt->m_free_requests);

#if !defined NLM_MT_OLD && !defined NLM_MT
	NlmCmAllocator__free(simxpt->m_alloc_p, simxpt->m_single_rqt_p);
#else
	for(count = 0; count < 	NLMNS_MT_MAXNUM_THREADS; count++)
		NlmCmAllocator__free(simxpt->m_alloc_p, simxpt->m_single_rqt_p[count]);
#endif
     if (simxpt->m_client_p)
        NlmModel__DestroyCascade(simxpt->m_alloc_p, (NlmMdlCascade *)simxpt->m_client_p);

    NlmCmAllocator__free(simxpt->m_alloc_p, simxpt);

    if (xpt)
        NlmCmAllocator__free(xpt->m_alloc, xpt);

}


/*******************************************************************************
 * Function : NlmSimXpt__TakeOutRequest
 *
 * Parameters:
 *
 * NlmXptRqtQue *rque    = Pointer to a request queue
 * NlmSimXptRqt *rqt     = Pointer to the request to be removed
 *
 * Summary:
 * NlmSimXpt__TakeOutRequest just removes a NlmSimXptRqt structure form a Request queue,
 * but it doesn't free the request. If the request is not in the queue, then
 * effectively the routine does nothing.
 ******************************************************************************/
static void NlmSimXpt__TakeOutRequest(
    NlmXptRqtQue *rque,
    NlmSimXptRqt *rqt
)
{
    NlmSimXptRqt *father = NULL, *cur, *son = NULL;

    if (!rque || !rqt) return;
    if (rque->count == 0 || !rque->head) return;

    cur = rque->head;

    while (cur) {
        son = cur->next;

        if (cur == rqt) {
            if (father) {
                father->next = son;
                if (!son)
                    rque->tail = father;
            }
            else /*Removing the head*/ {
                rque->head = son;
            }
            rque->count--;
            rqt->next = NULL;
            break;
        }
        father = cur;
        cur = son;
    }
}


/*******************************************************************************
 * Function : NlmSimXpt__AddToRequestQue
 *
 * Parameters:
 *
 * NlmXptRqtQue *rque    = Pointer to a request queue
 * NlmSimXptRqt *rqt     = Pointer to the request to be added
 *
 * Summary:
 * NlmSimXpt__AddToRequestQue inserts a NlmSimXptRqt structure at the end of the que
 ******************************************************************************/

static void NlmSimXpt__AddToRequestQue(
    NlmXptRqtQue *rque,
    NlmSimXptRqt *rqt)
{
    if (!rque || !rqt) return;

    rqt->next = NULL;

    if (rque->count == 0) {
        rque->head = rque->tail = rqt;
    }
    else {
        rque->tail->next = rqt;
        rque->tail = rqt;
    }

    rque->count++;
}


/*******************************************************************************
 * Function : NlmSimXpt__FreeRequestQue
 *
 * Parameters:
 *
 * NlmCmAllocator *alloc = Pointer to General purpose memory allocator
 * NlmXptRqtQue *rq      = Pointer to a request queue
 *
 * Summary:
 * NlmSimXpt__FreeRequestQue decallocates a request queue.
 ******************************************************************************/

static void NlmSimXpt__FreeRequestQue(
        NlmCmAllocator *alloc,
        NlmXptRqtQue *rq)
{
    nlm_u32 rqtcnt;
    NlmSimXptRqt *rqt, *rqtnext;
    if (!rq || !alloc) return;

    rqtcnt = rq->count;
    if (!rqtcnt) return;

    rqt = rq->head;

    while (rqt) {
        rqtnext = rqt->next;
        NlmCmAllocator__free(alloc, rqt);
        rqt = rqtnext;
    }

    rq->count = 0;
    rq->head = rq->tail = NULL;

}


/*******************************************************************************
 * Function : NlmSimXpt__destroy
 *
 * Parameters:
 * NlmSimXpt *self         = Pointer to an NlmSimXpt object
 *
 * Summary:
 * NlmSimXpt__destroy frees all the memory associated with the xpt structure
 ******************************************************************************/

void NlmSimXpt__destroy(
    NlmXpt* self
)
{
    if (!self) return;

    if (self->m_derived)
        NlmSimXpt__pvt_destroy(self->m_derived);
    else
        NlmCmAllocator__free(self->m_alloc, self);
}


/*******************************************************************************
 * Function : NlmSimXpt__ResetDevices
 *
 * Parameters:
 * NlmSimXpt *self         = Pointer to an NlmSimXpt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__ResetDevices reinitializes the device and all the requests are
 * reset too.
 ******************************************************************************/

static NlmErrNum_t NlmSimXpt__ResetDevices(
    void *self,
    NlmReasonCode *o_reason
)
{
    NlmSimXpt *simxpt;
    NlmMdlCascade *cascade_p;

    simxpt = (NlmSimXpt *)self;

    if (!IS_SIMXPT(simxpt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    cascade_p = (NlmMdlCascade *)(simxpt->m_client_p);

    if (!cascade_p) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    NlmSimXpt__ResetRequests(simxpt, o_reason);
    NlmModel__ResetCascade(cascade_p);

    return NLMERR_OK;
}

/*******************************************************************************
 * Function : NlmSimXpt__LockConfig
 *
 * Parameters:
 * void *self         = Pointer to an NlmXpt object
 * nlm_u32            = Number of devieces
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__LockConfig finalizes the various aspects of the simulated device.
 * Only after NlmSimXpt__LockConfig is called, it will start accepting requests
 ******************************************************************************/

static NlmErrNum_t NlmSimXpt__LockConfig(
    void *self,
    nlm_u32 num_devices,
    NlmReasonCode *o_reason
)
{

    NlmSimXpt *simxpt;
    NlmMdlCascade *cascade_p;
    NlmXpt    *xpt;

    simxpt = (NlmSimXpt *)self;

    if (!IS_SIMXPT(simxpt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    xpt = simxpt->xpt;

    /* Initialize the model cascade */
    cascade_p = NlmModel__InitializeCascade(simxpt->m_alloc_p,
        simxpt->m_devType, (nlm_u8)num_devices, simxpt->m_speedmode,
        simxpt->m_oprmode, simxpt->m_rbus_mode,
        NULL);

    if (!cascade_p) {
        if (o_reason)
            *o_reason = NLMRSC_OPR_FAILURE;
        return NLMERR_FAIL;
    }
#ifdef NLM_MT_XPT 
		{
			nlm_u32 ret = 0;
		
			/* Initialize the spin locks used */
		ret = NlmCmMt__SpinInit(&simxpt->m_xpt_spinLock,
					"NlmSimXpt_Kbp_SpinLock",
					NlmCmMtFlag);
			if(ret != 0)
			{
				NlmCm__printf("\n\n\t simxpt spinlock_init failed.\n");
				exit(1);
			}
		}
 #endif
    simxpt->m_client_p = (void *) cascade_p;

    return NLMERR_OK;
}


