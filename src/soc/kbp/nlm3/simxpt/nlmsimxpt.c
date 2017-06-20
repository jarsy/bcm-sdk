/*
 * $Id: nlmsimxpt.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <nlmsimxpt.h>
#include <nlmmdl_api.h>
#include <nlmarch.h>
#include <nlmcmutility.h>
#include "nlmerrorcodes.h"

#if defined NLM_MT_OLD || defined NLM_MT
#define NLM_MT_XPT
#endif

#ifdef NLM_MT_XPT 
 #include "nlmcmmt.h"
#endif
 

#define   SIMXPT_MIN_RQT_CNT            (1)
#define   SIMXPT_MAX_RQT_CNT            (0x0FFFFFFFF)
#define   SIMXPT_EMPTY_RTQ              (0)
#define   SIMXPT_MIN_RSLT_CNT           (1)

#define   SIMXPT_NUM_ALLOC_CNT          (1)


#define   SIMXPT_OPCODE_0               (0)
#define   SIMXPT_OPCODE_1               (1)

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
typedef struct NlmSimXptRqt_t 
{
    NlmXptRqt m_nlm_rqt;
    NlmBool m_done;            /* If the request is serviced */
    NlmRequestId  m_rqt_id;          /* Request id assigned by the xpt */
    struct NlmSimXptRqt_t *next;
    nlm_u32 simxpt_rqt_magic;

}NlmSimXptRqt;

typedef struct NlmXptRqtQue_t 
{
    nlm_u32 count;  /*how many results on the que */
    NlmSimXptRqt *head;   /*head of the queue*/
    NlmSimXptRqt *tail;   /*tail of the queue*/

} NlmXptRqtQue;



typedef struct NlmSimXpt_t 
{
    void*          m_client_p;/*
     * Pointer to lower level client
     * sepcific object, eg. Cmodel object.
     */
    NlmXpt*                xpt;    /* Pointer to virtual xpt object */
    NlmCmAllocator*        m_alloc_p; /* General purpose memory Allocator */
    nlm_u32                m_max_rqt_count; /*Max request count */
    nlm_u32                m_max_rslt_count; /* Max rslt cnt    */
    NlmXptRqtQue           m_in_use_requests; /* Request being used  */
    NlmXptRqtQue           m_free_requests; /* Free requests */

#if !defined NLM_MT_OLD && !defined NLM_MT
    NlmSimXptRqt*          m_single_rqt_p; /* Used for storing a single request in flat xpt without queuing */
#else
    NlmSimXptRqt*          m_single_rqt_p[NLMNS_MT_MAXNUM_THREADS]; /* Used for storing a single request in flat xpt without queuing */
#endif

   NlmRequestId           m_next_rqt_id;
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

} NlmSimXpt;

/*
 * MACRO definitions
 */

#define NlmSimXpt__GetInUseRequestCount(self)    ((self)->m_in_use_requests.count)
#define NlmSimXpt__GetFreeRequestCount(self)     ((self)->m_free_requests.count)
#define NlmSimXpt__GetXpt(self)                  (self->xpt)

/*
 * The magic numbers are used to verify if a particular object is of the same
 * object type as the one we are looking for.
 */

#define NLM_SIMXPT_MAGIC_NUMBER          (0x67890)
#define NLM_SIMXPT_RQT_MAGIC_NUMBER      (0x67891)

#define IS_SIMXPT(x)         ((x) && ((x)->simxpt_magic == NLM_SIMXPT_MAGIC_NUMBER))
#define IS_SIMXPTRQT(x)      ((x) && ((x)->simxpt_rqt_magic == NLM_SIMXPT_RQT_MAGIC_NUMBER))

#define MAXIMIZE_ERROR_SEVERITY(x, y) \
      if (x) {if ((*x != NLMRSC_OPR_FAILURE) &&(NLMRSC_OPR_FAILURE == y))\
                       *x = (NlmReasonCode)y;\
          else if (*x != NLMRSC_INVALID_PARAM  && (NLMRSC_INVALID_PARAM == y))\
                            *x = (NlmReasonCode)y;}

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

static void NlmSimXpt__BlockInstOperation(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmBlkInstType instType,
    NlmReasonCode *o_reason
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

static void NlmSimXpt__EMWriteToDevice(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmBool is_A,
    NlmReasonCode *o_reason
);

static void NlmSimXpt__ReadFromDevice(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmBool is_x_read,
    NlmReasonCode *o_reason
);

static void NlmSimXpt__EMReadFromDevice(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmBool     is_A,
    NlmReasonCode *o_reason
);

static void NlmSimXpt__WriteToDeviceCtxBf(
    NlmSimXpt *simxpt,
    NlmSimXptRqt *simrqt,
    NlmReasonCode *o_reason
);

static void NlmSimXpt__CmdSend(
    NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmInstType instType,
        NlmReasonCode *o_reason);


static void NlmSimXpt__CompareData(
        NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmMdlCBInstType cbInstType,
        NlmReasonCode *o_reason
);

static void*  NlmSimXpt__GetResult(
        void*              self,
        NlmXptRqt*              rqt,
        NlmReasonCode *o_reason
);

static NlmRequestId *NlmSimXpt__GetRequestId(
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

static void NlmSimXpt__2SMTCompare(
    NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
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

    NlmSimXpt  *simxpt = NULL;
    NlmSimXptRqt *temp = NULL;
    
    simxpt = (NlmSimXpt *)self;
    (void)o_reason;

    if (!IS_SIMXPT(simxpt))
        return NULL;

    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
        simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
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

    while (temp) 
    {
        void *result;

        if (temp->m_done) 
        {
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
    NlmSimXpt  *simxpt = NULL;
    NlmSimXptRqt *simrqt= NULL, *temp = NULL;


    simrqt = (NlmSimXptRqt *)rqt;
    simxpt = (NlmSimXpt *)self;

    if (!IS_SIMXPT(simxpt) || !IS_SIMXPTRQT(simrqt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NULL;
    }

    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
        simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
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

    while (temp) 
    {
        if (temp == simrqt) 
        {
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
    NlmSimXptRqt *simrqt= NULL;

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
 * kbp_xpt_get_result_for_rqtid sends a filled in result structure to upper layer
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
    NlmSimXpt *simxpt= NULL;
    NlmSimXptRqt *rqt = NULL;
    void *result = NULL;
    
    (void)o_reason;

    if (!self || !rqtid)
        return NULL;

    simxpt = (NlmSimXpt *) self;

    if (!IS_SIMXPT(simxpt)) return NULL;

    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
        simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
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

    while (rqt) 
    {
        if (!NlmCm__memcmp(&rqt->m_rqt_id, rqtid, sizeof(NlmRequestId)))
            break;

        rqt = rqt->next;
    }

    if (rqt && rqt->m_done) 
    {
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
    NlmSimXpt *simxpt = NULL;
    NlmSimXptRqt *simrqt = NULL;

    if (!self || !IS_SIMXPT((NlmSimXpt *)self)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;

        return NULL;
    }

    simxpt = (NlmSimXpt *)self;

#ifdef NLM_MT_XPT 
    NlmCmMt__SpinLock(&simxpt->m_xpt_spinLock);
 #endif

    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
        simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
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

    if (!simrqt) 
    {
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
    NlmSimXptRqt *simrqt = NULL;

    if (rque->count == SIMXPT_EMPTY_RTQ)
        return NULL;

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
    if (!self || !simrqt) 
        return;

    NlmCm__memcpy(&simrqt->m_rqt_id, &self->m_next_rqt_id, sizeof(NlmRequestId));

    if (self->m_next_rqt_id.m_count < (SIMXPT_MAX_RQT_CNT - 0x1)) 
    {
        self->m_next_rqt_id.m_count++;
    }
    else 
    {
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
 * kbp_xpt_reset_requests clears all the Requests, moves all the requests to the
 * free request list. The requests are just moved to the free_list irrespective
 * of they are executed or not.
 ******************************************************************************/
static void NlmSimXpt__ResetRequests(
    void*    self,
    NlmReasonCode *o_reason
)
{
    NlmSimXpt *simxpt = NULL;
    
    simxpt = (NlmSimXpt *) self;
    (void)o_reason;

    if (!IS_SIMXPT(simxpt)) return;

    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
        simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
    {
        return ;
    }

    while (NlmSimXpt__GetInUseRequestCount(simxpt)) 
    {
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

    NlmSimXptRqt *simrqt = NULL;

    if (!IS_SIMXPT(self) || !NlmSimXpt__GetInUseRequestCount(self))
        return NULL;

    simrqt = self->m_in_use_requests.head;

    while (simrqt) 
    {
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
    NlmSimXpt *simxpt = NULL;
    NlmSimXptRqt *simrqt = NULL;
    void *result = NULL;

    if (!self) 
        return;

    simxpt = (NlmSimXpt *)self;

    if (!IS_SIMXPT(simxpt)) 
    {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }

    if (o_reason)
        *o_reason = NLMRSC_REASON_OK;


    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
        simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
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


    if (!NlmSimXpt__GetInUseRequestCount(simxpt))
        return;

    while (NULL != (simrqt = NlmSimXpt__NextToExecute(simxpt)))
    {
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
    NlmXptRqt *p = NULL;
    nlm_u32 sev_param = 0;
    NlmReasonCode dummyReason = NLMRSC_REASON_OK; 

    if (!IS_SIMXPT(simxpt) || !IS_SIMXPTRQT(simrqt)
                || !simxpt->m_client_p)
    {
        sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
        return;
    }

    if(o_reason == NULL)
        o_reason = &dummyReason;

    p = (NlmXptRqt *) simrqt;

    /* Both SMTs have instructions */
    if(p->m_2smt == NlmTrue)
    {
        NlmSimXpt__2SMTCompare(simxpt, simrqt, o_reason);

        return;
    }

    if ((NLM_OPCODE_REG_WRITE_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
        (NLM_OPCODE_REG_WRITE_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__WriteToDevice(simxpt, simrqt, o_reason);
    }
    else if ((NLM_OPCODE_DBA_WRITE_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
        (NLM_OPCODE_DBA_WRITE_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__WriteToDevice(simxpt, simrqt, o_reason);
    }
    else if ((NLM_OPCODE_UDA_WRITE_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
        (NLM_OPCODE_UDA_WRITE_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__WriteToDevice(simxpt, simrqt, o_reason);
    }
    else if ((NLM_OPCODE_EM_WRA_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
        (NLM_OPCODE_EM_WRA_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__EMWriteToDevice(simxpt, simrqt, NlmTrue, o_reason);
    }
    else if ((NLM_OPCODE_EM_WRB_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
        (NLM_OPCODE_EM_WRB_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__EMWriteToDevice(simxpt, simrqt, NlmFalse, o_reason);
    }
    else if (NLM_OPCODE_CBWRITE_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) 
    {
        NlmSimXpt__WriteToDeviceCtxBf(simxpt, simrqt, o_reason);
    }
    else if ((NLM_OPCODE_CBWRITE_CMP1_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS >  p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__CompareData(simxpt, simrqt, NLM_MDL_CB_INST_CMP1, o_reason);
    }

    else if ((NLM_OPCODE_CBWRITE_CMP2_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS >  p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__CompareData(simxpt, simrqt, NLM_MDL_CB_INST_CMP2, o_reason);
    }
    else if ((NLM_OPCODE_CBWRITE_CMP3_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS >  p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__CompareData(simxpt, simrqt, NLM_MDL_CB_INST_CMP3, o_reason);
    }
    else if ((NLM_OPCODE_NOP_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_NOP_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__PerformNOP(simxpt, simrqt);
    }
    else if ((NLM_OPCODE_REG_READ_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_REG_READ_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__ReadFromDevice(simxpt, simrqt, NLMTRUE, o_reason);
    }
    else if ((NLM_OPCODE_UDA_READ_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_UDA_READ_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__ReadFromDevice(simxpt, simrqt, NLMTRUE, o_reason);
    }
    else if ((NLM_OPCODE_DBA_READ_X_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_DBA_READ_X_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__ReadFromDevice(simxpt, simrqt, NLMTRUE, o_reason);
    }
    else if ((NLM_OPCODE_DBA_READ_Y_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_DBA_READ_Y_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__ReadFromDevice(simxpt, simrqt, NLMFALSE, o_reason);

    }
    else if ((NLM_OPCODE_EM_RDA_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_EM_RDA_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__EMReadFromDevice(simxpt, simrqt, NlmTrue, o_reason);

    }
    else if ((NLM_OPCODE_EM_RDB_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_EM_RDB_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__EMReadFromDevice(simxpt, simrqt, NlmFalse, o_reason);

    }
    else if ((NLM_OPCODE_LPMTYPE2_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_LPMTYPE2_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1])) 
    {
        NlmSimXpt__CmdSend(simxpt, simrqt, NLM_LPMTYPE2, o_reason);
    }
    else if ((NLM_OPCODE_LPMTYPE3_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_LPMTYPE3_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__CmdSend(simxpt, simrqt, NLM_LPMTYPE3, o_reason);
    }
    else if ((NLM_OPCODE_LPMTYPE1_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLM_OPCODE_LPMTYPE1_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__CmdSend(simxpt, simrqt, NLM_LPMTYPE1, o_reason);
    }
    else if ((NLM_OPCODE_CBWLPM_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS > p->m_opcode[SIMXPT_OPCODE_1]))
    {
        NlmSimXpt__CompareData(simxpt, simrqt, NLM_MDL_CB_INST_LPM, o_reason);
    }
    else if ( (NLM_OPCODE_BLOCK_COPY_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
              (NLM_OPCODE_BLOCK_COPY_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]) ) 
    {
        NlmSimXpt__BlockInstOperation(simxpt, simrqt, NLMDEV_BLK_COPY_INST, o_reason);      
    }
    else if ( (NLM_OPCODE_BLOCK_MOVE_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
              (NLM_OPCODE_BLOCK_MOVE_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]) )
    {
        NlmSimXpt__BlockInstOperation(simxpt, simrqt, NLMDEV_BLK_MOVE_INST, o_reason);      
    }
    else if ( (NLM_OPCODE_BLOCK_CLEAR_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
              (NLM_OPCODE_BLOCK_CLEAR_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]) ) 
    {
        NlmSimXpt__BlockInstOperation(simxpt, simrqt, NLMDEV_BLK_CLEAR_INST, o_reason);     
    }
    else if ( (NLM_OPCODE_BLOCK_EV_BITS_8_6 == p->m_opcode[SIMXPT_OPCODE_0]) &&
              (NLM_OPCODE_BLOCK_EV_BITS_5_0 == p->m_opcode[SIMXPT_OPCODE_1]) )
    {
        NlmSimXpt__BlockInstOperation(simxpt, simrqt, NLMDEV_BLK_ENTRY_VALIDATE_INST, o_reason);        
    }
    else 
    {
        nlm_u32 sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);

        if(simxpt->m_max_rqt_count != SIMXPT_MIN_RQT_CNT ||simxpt->m_max_rslt_count != SIMXPT_MIN_RSLT_CNT)
        {
            MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
                        &simxpt->m_free_requests);
        }
    }
}

/* This function converts the compare results provided by C-Model
to the format required by Device Manager */

static void 
NlmSimXpt__pvt_FormatCompareResult(
    nlm_u8 *o_compareRslt,
    NlmCompareResult *compareRslt,
    NlmBool isCmp3
    )
{
    nlm_u32 i = 0, j = 0, offset = 0, adLenInByte = 0, numResults = 0;

    if(isCmp3)
        numResults = NLM_MAX_NUM_RESULTS;
    else
        numResults = NLM_MAX_NUM_RESULT_PORTS;

    /*
    Each search result can have 32b of result index and 32/64/128/256 of AD (optional). 
    Bit[31] of result indicates result Valid/Invalid, Bit[30] of result indicates HIT/MISS, Bits[29:27]
    indicates length of AD and Bits[23:0] indicates HIT Index 
    */
    for(i = 0; i < numResults; i++)
    {
        /* Write the 32b field of result - RV, SMF, index, AD response format*/
        o_compareRslt[offset + 0] = compareRslt->m_isResultValid[i] ? (1 << 7) : 0;
        o_compareRslt[offset + 0] |= (compareRslt->m_hitOrMiss[i] ? (1 << 6) : 0);
        o_compareRslt[offset + 0] |= (compareRslt->m_responseFormat[i]  << 3);

        o_compareRslt[offset + 1] = (nlm_u8) ((compareRslt->m_hitIndex[i] >> 16) & 0xFF);
        o_compareRslt[offset + 2] = (nlm_u8) ((compareRslt->m_hitIndex[i] >> 8) & 0xFF);
        o_compareRslt[offset + 3] = (nlm_u8) (compareRslt->m_hitIndex[i] & 0xFF);

        offset += 4;

        /* Write the AD if present */
        if(compareRslt->m_responseFormat[i] != NLM_ONLY_INDEX_NO_AD)
        {
            adLenInByte = (2 << compareRslt->m_responseFormat[i]);
            
            /*Truncate the AD if it is overflowing result length limit*/
            if(offset + adLenInByte > NLMDEV_MAX_RESP_LEN_IN_BYTES)
                adLenInByte = NLMDEV_MAX_RESP_LEN_IN_BYTES - offset;
            
            for(j = 0; j < adLenInByte; j++)
                o_compareRslt[offset + j] = compareRslt->m_assocData[i][j];

            offset +=  adLenInByte;
        }
        
        /* Ignore other results if output result buffer is full*/
        if(offset  >= NLMDEV_MAX_RESP_LEN_IN_BYTES)
            break;
        
    }   
}


static void NlmSimXpt__BlockInstOperation(NlmSimXpt *simxpt,
                                       NlmSimXptRqt *simrqt,
                                       NlmBlkInstType instType,
                                       NlmReasonCode *o_reason
                                       )
{
    NlmModel *model_p = NULL;
    NlmMdlBlockInstParam instParam = {0, };
    nlm_u32 srcAddr = 0, destAddr = 0;
    nlm_u8 copyDir = 0, devId = 0, setNotClear = 0;
    nlm_u16 numOfWords = 0;
    
    NlmXptRqt *xptRqt = (NlmXptRqt *) simrqt;
    
    /* Get the model pointer */
    model_p  =  (NlmModel *)simxpt->m_client_p;

    /* Device Id */
    devId = (nlm_u8)((xptRqt->m_data[0] >> 1) & 0x3);   /* bit 1:2 */

    /* Number of words  : Byte 1-2 */
    numOfWords = (nlm_u16)((xptRqt->m_data[0] >> 4) | (xptRqt->m_data[1] << 4));

    /* source address  : Byte 2-3-4 */
    srcAddr = ( (xptRqt->m_data[2]) | (xptRqt->m_data[3] << 8) | (xptRqt->m_data[4] << 16) );

    if(instType == NLMDEV_BLK_COPY_INST || instType == NLMDEV_BLK_MOVE_INST)
    {
        /* Count Direction */
        copyDir = (nlm_u8)(xptRqt->m_data[0] & 0x1);        /* bit 0 */

        /* destination address : Byte 5-6-7 */
        destAddr = ( (xptRqt->m_data[5]) | (xptRqt->m_data[6] << 8) | (xptRqt->m_data[7] << 16) );
    }
    else
    {
        /* Set/Clear bit */
        setNotClear = (nlm_u8)(xptRqt->m_data[0] & 0x1);    /* bit 0 */
    }

    /* Fill data for model */
    instParam.m_srcAddress      = srcAddr;
    instParam.m_destAddress     = 0;
    instParam.m_countDirection  = 0;
    instParam.m_numOf80bEntries = numOfWords;
    instParam.m_setNotClear     = 0;
    instParam.m_devId           = devId;
    
    if(instType == NLMDEV_BLK_COPY_INST)
    {
        instParam.m_destAddress     = destAddr;
        instParam.m_countDirection  = copyDir;
        instParam.m_blkInstType     = NLM_MDL_BLK_INST_BLK_COPY;
    }
    else if(instType == NLMDEV_BLK_MOVE_INST)
    {
        instParam.m_destAddress     = destAddr;
        instParam.m_countDirection  = copyDir;
        instParam.m_blkInstType     = NLM_MDL_BLK_INST_BLK_MOVE;
    }
    else if(instType == NLMDEV_BLK_CLEAR_INST)
    {
        instParam.m_blkInstType = NLM_MDL_BLK_INST_BLK_CLEAR;
    }
    else 
    {
        instParam.m_setNotClear     = setNotClear;
        instParam.m_blkInstType     = NLM_MDL_BLK_INST_BLK_EVI;
    }
    

    kbp_model_block_oper(model_p,
                        xptRqt->m_smt_num,
                        xptRqt->m_port_num,
                        &instParam,o_reason);
}


/*******************************************************************************
 * Function : NlmSimXpt__CompareData
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmMdlCBInstType cbInstType             = if it is a cb write,  compare1 or comapre2 operation
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__CompareData  sends a "COMPARE" command to the underlying device.
 ******************************************************************************/

static void NlmSimXpt__CompareData(
NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmMdlCBInstType cbInstType,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt = (NlmXptRqt *) simrqt;
    NlmMdlCBWriteCmpParam cmpParam;
    NlmModel *model_p;
    NlmBool isCmp3 = NlmFalse;
    nlm_u32 sev_param;
    nlm_u8 *data_ptr = NULL;

#ifdef NLM_NO_MEMCPY_IN_XPT
        data_ptr = xptRqt->m_data_p;
#else
        data_ptr = xptRqt->m_data;
#endif

    if(cbInstType == NLM_MDL_CB_INST_CMP3)
        isCmp3 = NlmTrue;

    model_p  =  (NlmModel *)simxpt->m_client_p;

    if(xptRqt->m_smt_num == NLMDEV_SMT_0)
    {
        cmpParam.m_cbInstType0 = cbInstType;
        cmpParam.m_cbData0.m_cbLoc = xptRqt->m_ctx_addr;
        cmpParam.m_ltrNum0 = xptRqt->m_opcode[SIMXPT_OPCODE_1];

        cmpParam.m_cbInstType1 = NLM_MDL_CB_INST_NONE;

        /* Construct the CB Data based on datalen*/
        if(xptRqt->m_data_len % NLMDEV_CB_WIDTH_IN_BYTES)
        {
            /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
            cmpParam.m_cbData0.m_dataLen = (nlm_u16)((xptRqt->m_data_len/NLMDEV_CB_WIDTH_IN_BYTES) + 1);
            cmpParam.m_cbData0.m_dataLen = cmpParam.m_cbData0.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;

            NlmCm__memset(cmpParam.m_cbData0.m_data, 0,
                            cmpParam.m_cbData0.m_dataLen - xptRqt->m_data_len);
            NlmCm__memcpy(&cmpParam.m_cbData0.m_data[cmpParam.m_cbData0.m_dataLen - xptRqt->m_data_len],
                            data_ptr,   xptRqt->m_data_len);        
        }
        else
        {
            cmpParam.m_cbData0.m_dataLen = (nlm_u16)(xptRqt->m_data_len);
            NlmCm__memcpy(cmpParam.m_cbData0.m_data, data_ptr, xptRqt->m_data_len);
        }
    }
    else
    {
        cmpParam.m_cbInstType1 = cbInstType;
        cmpParam.m_cbData1.m_cbLoc = xptRqt->m_ctx_addr;
        cmpParam.m_ltrNum1 = xptRqt->m_opcode[SIMXPT_OPCODE_1];

        cmpParam.m_cbInstType0 = NLM_MDL_CB_INST_NONE;

        /* Construct the CB Data based on datalen*/
        if(xptRqt->m_data_len % NLMDEV_CB_WIDTH_IN_BYTES)
        {
            /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
            cmpParam.m_cbData1.m_dataLen = (nlm_u16)((xptRqt->m_data_len / NLMDEV_CB_WIDTH_IN_BYTES) + 1);
            cmpParam.m_cbData1.m_dataLen = cmpParam.m_cbData1.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;

            NlmCm__memset(cmpParam.m_cbData1.m_data, 0,
                            cmpParam.m_cbData1.m_dataLen - xptRqt->m_data_len);
            NlmCm__memcpy(&cmpParam.m_cbData1.m_data[cmpParam.m_cbData1.m_dataLen - xptRqt->m_data_len],
                            data_ptr,   xptRqt->m_data_len);        
        }
        else
        {
            cmpParam.m_cbData1.m_dataLen = (nlm_u16)(xptRqt->m_data_len);
            NlmCm__memcpy(cmpParam.m_cbData1.m_data, data_ptr, xptRqt->m_data_len);
        }
    }

    if (NLMERR_OK != kbp_model_cb_write_cmp(model_p, xptRqt->m_port_num, &cmpParam, o_reason))
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

    /* Get the search results For Device Manager. */
    if((cbInstType != NLM_MDL_CB_INST_WRITE) &&
                (cbInstType != NLM_MDL_CB_INST_NONE))
        NlmSimXpt__pvt_FormatCompareResult((nlm_u8*)xptRqt->m_result, &cmpParam.m_cmpResult, isCmp3);

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

    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
            simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
    {
        return;
    }

    MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
            &simxpt->m_free_requests);

}

/*******************************************************************************
 * Function : NlmSimXpt__2SMTCompare
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__2SMTCompare  sends compare instructions on both SMTs together
 ******************************************************************************/

static void NlmSimXpt__2SMTCompare(
    NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt = (NlmXptRqt *) simrqt;
    NlmMdlCBWriteCmpParam cmpParam;
    NlmModel *model_p;
    NlmMdlCBInstType cbInst = NLM_MDL_CB_INST_WRITE, cbInst1 = NLM_MDL_CB_INST_WRITE;
    nlm_u32 sev_param;
    nlm_u8 *data_ptr, *data_ptr1;

#ifdef NLM_NO_MEMCPY_IN_XPT
        data_ptr = xptRqt->m_data_p;
        data_ptr1 =xptRqt->m_data1_p; 
#else
        data_ptr = xptRqt->m_data;
        data_ptr1 = xptRqt->m_data1;
#endif

    model_p  =  (NlmModel *)simxpt->m_client_p;

    switch(xptRqt->m_opcode[SIMXPT_OPCODE_0])
    {
        case NLM_OPCODE_CBWRITE_CMP1_BITS_8_6:
        {
            cbInst = NLM_MDL_CB_INST_CMP1;
            break;
        }

        case NLM_OPCODE_CBWRITE_CMP2_BITS_8_6:
        {
            cbInst = NLM_MDL_CB_INST_CMP2;
            break;
        }

        case NLM_OPCODE_CBWLPM_BITS_8_6:
        {
            cbInst = NLM_MDL_CB_INST_LPM;
            break;
        }
    }

    switch(xptRqt->m_opcode1[SIMXPT_OPCODE_0])
    {
        case NLM_OPCODE_CBWRITE_CMP1_BITS_8_6:
        {
            cbInst1 = NLM_MDL_CB_INST_CMP1;
            break;
        }

        case NLM_OPCODE_CBWRITE_CMP2_BITS_8_6:
        {
            cbInst1 = NLM_MDL_CB_INST_CMP2;
            break;
        }

        case NLM_OPCODE_CBWLPM_BITS_8_6:
        {
            cbInst1 = NLM_MDL_CB_INST_LPM;
            break;
        }
    }

    /* Fill SMT-0 related data */
    cmpParam.m_cbInstType0 = cbInst;
    cmpParam.m_cbData0.m_cbLoc = xptRqt->m_ctx_addr;
    cmpParam.m_ltrNum0 = xptRqt->m_opcode[SIMXPT_OPCODE_1];

    /* Construct the CB Data based on datalen*/
    if(xptRqt->m_data_len % NLMDEV_CB_WIDTH_IN_BYTES)
    {
        /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
        cmpParam.m_cbData0.m_dataLen = (nlm_u16)((xptRqt->m_data_len / NLMDEV_CB_WIDTH_IN_BYTES) + 1);
        cmpParam.m_cbData0.m_dataLen = cmpParam.m_cbData0.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;

        NlmCm__memset(cmpParam.m_cbData0.m_data, 0,
                            cmpParam.m_cbData0.m_dataLen - xptRqt->m_data_len);
        NlmCm__memcpy(&cmpParam.m_cbData0.m_data[cmpParam.m_cbData0.m_dataLen - xptRqt->m_data_len],
                            data_ptr,   xptRqt->m_data_len);        
    }
    else
    {
        cmpParam.m_cbData0.m_dataLen = (nlm_u16)(xptRqt->m_data_len);
        NlmCm__memcpy(cmpParam.m_cbData0.m_data, data_ptr, xptRqt->m_data_len);
    }

    /* Fill SMT-1 related data */
    cmpParam.m_cbInstType1 = cbInst1;
    cmpParam.m_cbData1.m_cbLoc = xptRqt->m_ctx_addr1;
    cmpParam.m_ltrNum1 = xptRqt->m_opcode1[SIMXPT_OPCODE_1];

    /* Construct the CB Data based on datalen*/
    if(xptRqt->m_data_len % NLMDEV_CB_WIDTH_IN_BYTES)
    {
        /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
        cmpParam.m_cbData1.m_dataLen = (nlm_u16)((xptRqt->m_data_len1 / NLMDEV_CB_WIDTH_IN_BYTES) + 1);
        cmpParam.m_cbData1.m_dataLen = cmpParam.m_cbData1.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;

        NlmCm__memset(cmpParam.m_cbData1.m_data, 0,
                            cmpParam.m_cbData1.m_dataLen - xptRqt->m_data_len1);
        NlmCm__memcpy(&cmpParam.m_cbData1.m_data[cmpParam.m_cbData1.m_dataLen - xptRqt->m_data_len1],
                            data_ptr1,  xptRqt->m_data_len1);
    }
    else
    {
        cmpParam.m_cbData1.m_dataLen = (nlm_u16)(xptRqt->m_data_len1);
        NlmCm__memcpy(cmpParam.m_cbData1.m_data, data_ptr1, xptRqt->m_data_len1);
    }

    if (NLMERR_OK != kbp_model_cb_write_cmp(model_p, xptRqt->m_port_num, &cmpParam, o_reason))
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

    /* Get the search results For Device Manager. */
    NlmSimXpt__pvt_FormatCompareResult((nlm_u8*)xptRqt->m_result, &cmpParam.m_cmpResult, NlmFalse);

    simrqt->m_done = 1;
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

    xptRqt = (NlmXptRqt *)simrqt;

    readRslt = (nlm_u8 *)xptRqt->m_result;

#ifdef NLM_NO_MEMCPY_IN_XPT
    readParam.m_address = xptRqt->m_address;
#else
    readParam.m_address = ReadBitsInArrray(xptRqt->m_data,
                                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                                31, 0);
#endif

    if(is_x_read)/* If database read see whether Read for X or Read for Y */
        readParam.m_dataType = NLM_READ_MODE_DATA_X;
    else
        readParam.m_dataType = NLM_READ_MODE_DATA_Y;



    if (NLMERR_OK != kbp_model_pio_read((NlmModel *)(simxpt->m_client_p),
                                            xptRqt->m_smt_num,
                                            xptRqt->m_port_num,
                                            &readParam, 
                                            o_reason))
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
        return;
    }

    /* Bit 4 of Read Result Byte 0 will indicate the VBIT with _RD_DATA_X */
    if(is_x_read)
        readRslt[0] = (nlm_u8)(readParam.m_vBit << 4);

    /* Byte 1 to 10 will have 80b data */
    NlmCm__memcpy(readRslt + 1, readParam.m_data, NLM_DATA_WIDTH_BYTES);

    simrqt->m_done = 1; /*Request complete */

}

/*******************************************************************************
 * Function : NlmSimXpt__EMReadFromDevice
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,   = Pointer to a NlmSimRqt object
 * NlmBool is_A                   =  Is it RDA or RDB? True if RDA
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__EMReadFromDevice  sends either RDA or RDB command to the underlying device.
 ******************************************************************************/

static void NlmSimXpt__EMReadFromDevice(
        NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmBool is_A,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt;
    NlmMdlEMReadParam readParam;
    nlm_u8 *readRslt;
    nlm_u32 sev_param;

    xptRqt = (NlmXptRqt *)simrqt;

    readRslt = (nlm_u8 *)xptRqt->m_result;

#ifdef NLM_NO_MEMCPY_IN_XPT
    readParam.m_address = xptRqt->m_address;
#else
    readParam.m_address = ReadBitsInArrray(xptRqt->m_data,
                                        NLMDEV_EM_ADDR_LEN_IN_BYTES,
                                        31, 0);
#endif

    if(is_A)
        readParam.m_dataType = NLM_EM_READ_MODE_A;
    else
        readParam.m_dataType = NLM_EM_READ_MODE_B;

    if (NLMERR_OK != kbp_model_em_read((NlmModel *)(simxpt->m_client_p),
                                            xptRqt->m_smt_num,
                                            xptRqt->m_port_num,
                                            &readParam, 
                                            o_reason))
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
        
        return;
    }

    /* Bit 4 of Read Result Byte 0 will indicate the VBIT */
    readRslt[0] = (nlm_u8)(readParam.m_vBit << 4);

    /* Byte 1 to 10 will have 80b data returnf*/
    NlmCm__memcpy(readRslt + 1, readParam.m_data, NLM_EM_HW_INPUT_LEN_IN_BYTES);

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
    NlmXptRqt *xptRqt  = (NlmXptRqt *) simrqt;
    NlmMdlPIOWriteParam wrparam;
    nlm_u32 cm_retst;
    nlm_u32 sev_param;
    nlm_u32 value;
    NlmModel *model_p = (NlmModel *)simxpt->m_client_p;

    nlm_u8 *data_ptr = NULL;
    nlm_u8 *mask_ptr = NULL;
    nlm_u32 addrType = 0;

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

    /* Set the default values of vbit and wrmode */
    wrparam.m_vBit = 0;
    wrparam.m_writeMode = NLM_WRITE_MODE_DATABASE_DM;
    
    /* Copy data */
    NlmCm__memcpy(wrparam.m_data, data_ptr, NLM_DATA_WIDTH_BYTES);

    /* Set the vbit, wrmode and mask if DBA operation type */
    addrType = (value >> NLM_ADDR_OP_TYPE_BIT_START_POS) % NLM_NUM_ADDR_TYPES;

    if(addrType == NLMDEV_ADDR_TYPE_DBA)
    {
        /* Get the VBIT for Database write if not default value (0) */
        if((value >> NLMDEV_AB_ENTRY_VALID_BIT_IN_ADDR) & 1) 
            wrparam.m_vBit = 1;
        else
            wrparam.m_vBit = 0;

        /* Get the mode of Database write */
        if((value >> NLMDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR) & 1) 
            wrparam.m_writeMode = NLM_WRITE_MODE_DATABASE_XY;
        else
            wrparam.m_writeMode = NLM_WRITE_MODE_DATABASE_DM;

        /* Copy mask since dba write */
        NlmCm__memcpy(wrparam.m_mask,   mask_ptr, NLMDEV_AB_WIDTH_IN_BYTES);
    }


    /* Actual Address is from Bits[26:0] */
    wrparam.m_address = value & 0x7FFFFFF;  
    cm_retst = kbp_model_pio_write(model_p,
                                    xptRqt->m_smt_num,
                                    xptRqt->m_port_num,
                                    &wrparam, 
                                    o_reason);

    if (NLMERR_OK != cm_retst)
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

    if(simxpt->m_max_rqt_count != SIMXPT_MIN_RQT_CNT ||simxpt->m_max_rslt_count != SIMXPT_MIN_RSLT_CNT)
    {
        MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
                    &simxpt->m_free_requests);
    }

#ifdef PIOWRITE_STATS
    numPIOWrites++;
#endif
}


/*******************************************************************************
 * Function : NlmSimXpt__EMWriteToDevice
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmBool is_A                   =  Is it WRA or WRB? True if WRA
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__WriteToDevice  sends either WRA or WRB command to the underlying device.
 ******************************************************************************/

static void NlmSimXpt__EMWriteToDevice(
    NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmBool is_A,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt  = (NlmXptRqt *) simrqt;
    NlmMdlEMWriteParam  wrparam;
    nlm_u32 cm_retst;
    nlm_u32 sev_param;
    nlm_u32 addr;
    NlmModel *model_p = (NlmModel *)simxpt->m_client_p;
    nlm_u8 *data_ptr = NULL;

#ifdef NLM_NO_MEMCPY_IN_XPT
    addr = xptRqt->m_address;

    data_ptr = xptRqt->m_data_p;
#else
    /* address + data  */
    addr = ReadBitsInArrray(xptRqt->m_data,
                        NLMDEV_EM_ADDR_LEN_IN_BYTES,
                        31, 0);

    data_ptr = xptRqt->m_data + NLMDEV_EM_ADDR_LEN_IN_BYTES;
#endif

    /* Copy data */
    NlmCm__memcpy(wrparam.m_data, data_ptr, NLM_EM_HW_INPUT_LEN_IN_BYTES);

    /* Get the VBIT for EM write */
    if((addr >> NLMDEV_EM_ENTRY_VALID_BIT_IN_ADDR) & 1) 
        wrparam.m_vBit = 1;
    else
        wrparam.m_vBit = 0;

    /* Is this WRA or WRB */
    if( is_A == NlmTrue)
        wrparam.m_writeMode = NLM_EM_WRITE_MODE_A;
    else
        wrparam.m_writeMode = NLM_EM_WRITE_MODE_B;

    /* Actual Address is from Bits[26:0] */
    wrparam.m_address = addr & 0x7FFFFFF;   
    cm_retst = kbp_model_em_write(model_p,
                                    xptRqt->m_smt_num,
                                    xptRqt->m_port_num,
                                    &wrparam, 
                                    o_reason);

    if (NLMERR_OK != cm_retst)
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

    if(simxpt->m_max_rqt_count != SIMXPT_MIN_RQT_CNT ||simxpt->m_max_rslt_count != SIMXPT_MIN_RSLT_CNT)
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
    NlmMdlCBWriteCmpParam cbParam;
    NlmXptRqt *xptRqt = (NlmXptRqt *) simrqt;
    nlm_u32 ret;
    nlm_u32 sev_param;
    nlm_u8 *data_ptr = NULL;

#ifdef NLM_NO_MEMCPY_IN_XPT
    data_ptr = xptRqt->m_data_p;
#else
    data_ptr = xptRqt->m_data;
#endif

    if(xptRqt->m_smt_num == NLMDEV_SMT_0)
    {
        cbParam.m_cbInstType0 = NLM_MDL_CB_INST_WRITE;
        cbParam.m_cbData0.m_cbLoc = xptRqt->m_ctx_addr;
        cbParam.m_ltrNum0 = 0;

        cbParam.m_cbInstType1 = NLM_MDL_CB_INST_NONE;

        /* Construct the CB Data based on datalen*/
        if(xptRqt->m_data_len % NLMDEV_CB_WIDTH_IN_BYTES)
        {
            /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
            cbParam.m_cbData0.m_dataLen = (nlm_u16)((xptRqt->m_data_len/NLMDEV_CB_WIDTH_IN_BYTES) + 1);
            cbParam.m_cbData0.m_dataLen = cbParam.m_cbData0.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;
            
            NlmCm__memset(cbParam.m_cbData0.m_data, 0, cbParam.m_cbData0.m_dataLen - xptRqt->m_data_len);
            NlmCm__memcpy(&cbParam.m_cbData0.m_data[cbParam.m_cbData0.m_dataLen - xptRqt->m_data_len],
                                data_ptr, xptRqt->m_data_len);
        }
        else
        {
            cbParam.m_cbData0.m_dataLen = (nlm_u16)(xptRqt->m_data_len);
            NlmCm__memcpy(cbParam.m_cbData0.m_data, data_ptr, xptRqt->m_data_len);
        }
    }
    else
    {
        cbParam.m_cbInstType1 = NLM_MDL_CB_INST_WRITE;
        cbParam.m_cbData1.m_cbLoc = xptRqt->m_ctx_addr;
        cbParam.m_ltrNum1 = 64;

        cbParam.m_cbInstType0 = NLM_MDL_CB_INST_NONE;

        /* Construct the CB Data based on datalen*/
        if(xptRqt->m_data_len % NLMDEV_CB_WIDTH_IN_BYTES)
        {
            /* If datalen is not at 80b boundary; Fill zeroes at MSB; Cmodel expects data to be in multiples of 80b */
            cbParam.m_cbData1.m_dataLen = (nlm_u16)((xptRqt->m_data_len/NLMDEV_CB_WIDTH_IN_BYTES) + 1);
            cbParam.m_cbData1.m_dataLen = cbParam.m_cbData1.m_dataLen * NLMDEV_CB_WIDTH_IN_BYTES;
            
            NlmCm__memset(cbParam.m_cbData1.m_data, 0, cbParam.m_cbData1.m_dataLen - xptRqt->m_data_len);
            NlmCm__memcpy(&cbParam.m_cbData1.m_data[cbParam.m_cbData1.m_dataLen - xptRqt->m_data_len],
                                data_ptr, xptRqt->m_data_len);
        }
        else
        {
            cbParam.m_cbData1.m_dataLen = (nlm_u16)(xptRqt->m_data_len);
            NlmCm__memcpy(cbParam.m_cbData1.m_data, data_ptr, xptRqt->m_data_len);
        }
    }

    ret = kbp_model_cb_write_cmp((NlmModel *)simxpt->m_client_p,
                                    xptRqt->m_port_num,
                                                &cbParam, 
                                                o_reason);

    if (NLMERR_OK != ret)
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

    if(simxpt->m_max_rqt_count != SIMXPT_MIN_RQT_CNT ||
        simxpt->m_max_rslt_count != SIMXPT_MIN_RSLT_CNT)
    {
        MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
            &simxpt->m_free_requests);
    }
}



/*******************************************************************************
 * Function : NlmSimXpt__CmdSend
 *
 * Parameters:
 *
 * NlmSimXpt *simxpt         = Pointer to a NlmSimXpt object
 * NlmSimXptRqt *simrqt,     = Pointer to a NlmSimRqt object
 * NlmInstType instType,        = Instruction type 
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmSimXpt__CmdSend  sends advance commands to the 
 * underlying device.
 ******************************************************************************/
static void NlmSimXpt__CmdSend(
    NlmSimXpt *simxpt,
        NlmSimXptRqt *simrqt,
        NlmInstType instType,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt  = (NlmXptRqt *) simrqt;
    NlmMdlAdvanceParam advParam;
    nlm_u32 cm_retst  = NLMERR_OK;
    nlm_u32 sev_param;

    if(xptRqt->m_data_len > NLM_MAX_DATA_WORDS * NLM_DATA_WORD_LEN_IN_BYTES)
    {
        *o_reason = NLMRSC_INVALID_DATA_LENGTH;
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);

        if(simxpt->m_max_rqt_count != SIMXPT_MIN_RQT_CNT ||
                simxpt->m_max_rslt_count != SIMXPT_MIN_RSLT_CNT)
        {
            MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests,
                    simrqt, &simxpt->m_free_requests);
        }

        return;
    }
    

    /* Set instruction parameters and copy data */
    advParam.m_instType = instType;
    advParam.m_dataLen = (nlm_u8)xptRqt->m_data_len;

#ifdef NLM_NO_MEMCPY_IN_XPT
    NlmCm__memcpy(advParam.m_data, xptRqt->m_data_p, xptRqt->m_data_len);
#else
    NlmCm__memcpy(advParam.m_data, xptRqt->m_data, xptRqt->m_data_len);
#endif

    /* Execute command */
    cm_retst = kbp_model_advance_inst((NlmModel *)simxpt->m_client_p,
                                    xptRqt->m_smt_num,
                                    xptRqt->m_port_num,
                                    &advParam, 
                                    o_reason);

    if (NLMERR_OK != cm_retst)
    {
        sev_param = NLMRSC_OPR_FAILURE;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
    }

    if(simxpt->m_max_rqt_count != SIMXPT_MIN_RQT_CNT ||simxpt->m_max_rslt_count != SIMXPT_MIN_RSLT_CNT)
    {
        MOVE_TO_FREE_LIST(&simxpt->m_in_use_requests, simrqt,
                    &simxpt->m_free_requests);
    }

    return;
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

    while (rqt)
    {
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
 * kbp_xpt_discard_result removes a request from in_use queue irrespective of
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

    if (!IS_SIMXPT(simxpt)) 
    {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }
    if (!IS_SIMXPTRQT(simrqt))
        goto not_allocated_by_me;

    if(simxpt->m_max_rqt_count == SIMXPT_MIN_RQT_CNT &&
        simxpt->m_max_rslt_count == SIMXPT_MIN_RSLT_CNT)
    {
        return;
    }

    if (!NlmSimXpt__GetInUseRequestCount(simxpt))
        return;


    temp = simxpt->m_in_use_requests.head;

    while (temp)
    {
        if (temp == simrqt)
        {
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
 * Function : kbp_simxpt_init
 *
 * Parameters:
 *
 *  NlmCmAllocator         *alloc    = Pointer to General purpose memory allocator
 *  nlm_u32                devType = At present support to NLM_DEVTYPE_3
                                        and NLM_DEVTYPE_3_N; Support can be added
                                    for other devices also in future;
 *  nlm_u32                max_rqt_count, = Max request count to be in a queue
 *  nlm_u32                max_rslt_count = Max result count to be in a queue
 *  nlm_u32                chan_id
 *
 * Summary:
 * kbp_simxpt_initCreates the transport object (NlmXpt) and returns a
 * pointer to the object. It also initializes the virtual operation table
 * of NlmXpt object
 ******************************************************************************/

NlmXpt *
kbp_simxpt_init(
        NlmCmAllocator          *alloc,         /*General purpose memory allocator */
        nlm_u32                 devType,
        nlm_u32                 max_rqt_count,  /*Max request count */
        nlm_u32                 max_rslt_count, /*Max result count*/
        nlm_u32                 chan_id
)
{
    NlmSimXpt *simxpt = 0;
    NlmXpt    *xpt = 0;
    nlm_u32   count;
    static NlmXpt_operations xpt_operations;

#ifdef NLM_12K_11K
    if (!alloc || (devType != NLM_DEVTYPE_2 && devType != NLM_DEVTYPE_2_S)
                  || (max_rqt_count < max_rslt_count))
        return NULL;
#else
    if (!alloc || (devType != NLM_DEVTYPE_3 && devType != NLM_DEVTYPE_3_N && 
        devType != NLM_DEVTYPE_3_40M && devType != NLM_DEVTYPE_3_N_40M)
                  || (max_rqt_count < max_rslt_count))
        return NULL;
#endif

    simxpt = (NlmSimXpt*)NlmCmAllocator__calloc(alloc, SIMXPT_NUM_ALLOC_CNT, sizeof(NlmSimXpt));
    if (!simxpt) 
        return NULL;
    
    simxpt->m_alloc_p = alloc;
    simxpt->m_max_rqt_count = max_rqt_count;
    simxpt->m_max_rslt_count = max_rslt_count;
    simxpt->simxpt_magic = NLM_SIMXPT_MAGIC_NUMBER;
    simxpt->m_devType = devType;
    /*Now construct the base xpt structure */

    xpt = (NlmXpt*)NlmCmAllocator__calloc(alloc, SIMXPT_NUM_ALLOC_CNT, sizeof(NlmXpt));
    if (!xpt) 
    {
        NlmSimXpt__pvt_destroy(simxpt);
        return NULL;
    }

    xpt_operations.kbp_xpt_get_result           = NlmSimXpt__GetResult;
    xpt_operations.kbp_xpt_discard_result       = NlmSimXpt__DiscardResult;
    xpt_operations.kbp_xpt_get_result_for_rqtid = NlmSimXpt__GetResultForRqtId;
    xpt_operations.kbp_xpt_get_request          = NlmSimXpt__GetRequest;
    xpt_operations.kbp_xpt_reset_requests       = NlmSimXpt__ResetRequests;
    xpt_operations.kbp_xpt_get_request_id       = NlmSimXpt__GetRequestId;
    xpt_operations.kbp_xpt_service_requests     = NlmSimXpt__ServiceRequests;
    xpt_operations.kbp_xpt_get_first_result     = NlmSimXpt__GetFirstAvlResult;
    xpt_operations.kbp_xpt_reset_device         = NlmSimXpt__ResetDevices;
    xpt_operations.kbp_xpt_lock_config          = NlmSimXpt__LockConfig;
    xpt->m_derived = simxpt;
    xpt->m_alloc   = alloc;
    xpt->m_oprs    = &xpt_operations;
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
    while (count < max_rqt_count)
    {
        NlmSimXptRqt *rqt;
        rqt = (NlmSimXptRqt*)NlmCmAllocator__calloc(alloc, SIMXPT_NUM_ALLOC_CNT, sizeof(NlmSimXptRqt));
        if(rqt == NULL)
        {
            NlmSimXpt__pvt_destroy(simxpt);
            return NULL;
        }

        NlmSimXpt__AddToRequestQue(&simxpt->m_free_requests, rqt);
        count++;
    }

#if !defined NLM_MT_OLD && !defined NLM_MT
        simxpt->m_single_rqt_p = (NlmSimXptRqt*)NlmCmAllocator__calloc(alloc, SIMXPT_NUM_ALLOC_CNT, sizeof(NlmSimXptRqt));
        if(simxpt->m_single_rqt_p == NULL)
        {
            NlmSimXpt__pvt_destroy(simxpt);
            return NULL;
        }
#else
        for(count = 0; count <  NLMNS_MT_MAXNUM_THREADS; count++)
        {
            simxpt->m_single_rqt_p[count] = (NlmSimXptRqt*)NlmCmAllocator__calloc(alloc, SIMXPT_NUM_ALLOC_CNT, sizeof(NlmSimXptRqt));
            if(simxpt->m_single_rqt_p[count] == NULL)
            {
                NlmSimXpt__pvt_destroy(simxpt);
                return NULL;
            }
        }
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
    NlmSimXpt *simxpt = (NlmSimXpt *) self;
    NlmXpt    *xpt;
    nlm_u32 count = 0;
    NlmReasonCode reasonCode;

    (void)count;

    if (!self) return;

    xpt = simxpt->xpt;

#ifdef NLM_MT_XPT 
    /* Destroy the spin locks used */
     NlmCmMt__SpinDestroy(&simxpt->m_xpt_spinLock,
                "NlmSimXpt_Kbp_SpinLock");
 #endif

    NlmSimXpt__FreeRequestQue(simxpt->m_alloc_p, &simxpt->m_in_use_requests);
    NlmSimXpt__FreeRequestQue(simxpt->m_alloc_p, &simxpt->m_free_requests);

#if !defined NLM_MT_OLD && !defined NLM_MT
    if(simxpt->m_single_rqt_p)
        NlmCmAllocator__free(simxpt->m_alloc_p, simxpt->m_single_rqt_p);
#else
    for(count = 0; count <  NLMNS_MT_MAXNUM_THREADS; count++)
    {
        if(simxpt->m_single_rqt_p[count])
            NlmCmAllocator__free(simxpt->m_alloc_p, simxpt->m_single_rqt_p[count]);
    }
#endif

    /* free xpt first, as the simxpt hold the xpt pointer */
    if (xpt)
        NlmCmAllocator__free(xpt->m_alloc, xpt);

    if(simxpt)
    {
        if (simxpt->m_client_p)
            kbp_model_destroy(simxpt->m_alloc_p, (NlmModel *)simxpt->m_client_p, &reasonCode);
        
        NlmCmAllocator__free(simxpt->m_alloc_p, simxpt);
    }
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

    if (!rque || !rqt) 
        return;
    
    if (rque->count == SIMXPT_EMPTY_RTQ || !rque->head) 
        return;

    cur = rque->head;

    while (cur)
    {
        son = cur->next;

        if (cur == rqt) 
        {
            if (father) 
            {
                father->next = son;
                if (!son)
                    rque->tail = father;
            }
            else /*Removing the head*/ 
            {
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

    if (rque->count == SIMXPT_EMPTY_RTQ) 
    {
        rque->head = rque->tail = rqt;
    }
    else 
    {
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

    while (rqt)
    {
        rqtnext = rqt->next;
        NlmCmAllocator__free(alloc, rqt);
        rqt = rqtnext;
    }

    rq->count = SIMXPT_EMPTY_RTQ;
    rq->head = rq->tail = NULL;

}

/*******************************************************************************
 * Function : kbp_simxpt_destroy
 *
 * Parameters:
 * NlmSimXpt *self         = Pointer to an NlmSimXpt object
 *
 * Summary:
 * kbp_simxpt_destroy frees all the memory associated with the xpt structure
 ******************************************************************************/

void kbp_simxpt_destroy(
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
    NlmSimXpt *simxpt = (NlmSimXpt *)self;
    NlmReasonCode dummyReasonCode;

    if(o_reason == NULL)
        o_reason = &dummyReasonCode;

    if (!IS_SIMXPT(simxpt)) 
    {   
        *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    NlmSimXpt__ResetRequests(simxpt, o_reason);
    kbp_model_reset((NlmModel *)(simxpt->m_client_p), o_reason);

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
/* #define FIB_PMD */
#ifdef FIB_PMD
void    *g_model_ptr;
#endif

FILE *log_p = NULL;

static NlmErrNum_t NlmSimXpt__LockConfig(
    void *self,
    nlm_u32 num_devices,
    NlmReasonCode *o_reason
)
{

    NlmSimXpt *simxpt = (NlmSimXpt *)self;
    NlmXpt  *xpt_p = NULL;
    NlmModel *model_p;
    NlmPortConfigType   portType;
    NlmSMTMode      smtMode;
    NlmReasonCode dummyReasonCode;
    NlmErrNum_t errNum = NLMERR_OK;

    if(o_reason == NULL)
        o_reason = &dummyReasonCode;

    if (!IS_SIMXPT(simxpt)) 
    {   
        *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    /* Fill port type and SMT type from the xpt pointer */
    xpt_p = simxpt->xpt;

    if(xpt_p->m_port_mode == NLMDEV_SINGLE_PORT)
        portType = NLM_PORT_CONFIG_SINGLE_PORT;
    else
        portType = NLM_PORT_CONFIG_DUAL_PORT;
    
    if(xpt_p->m_smt_mode == NLMDEV_NO_SMT_MODE)
        smtMode = NLMDEV_NO_SMT_MODE;
    else
        smtMode = NLMDEV_DUAL_SMT_MODE;

#ifdef BCM_KBP_COMPILER
        log_p = NlmCmFile__fopen("netroute_out.txt","w");
        if(log_p == NULL)
        {
            NlmCm__printf("\n\t Can't create logging file,  exiting \n\n\n");
#ifndef NLMPLATFORM_BCM
            exit(1);
#else
            return NLMERR_FAIL;
#endif
        }
#endif

    /* Initialize the model */
    model_p = kbp_model_init(simxpt->m_alloc_p,
                                (NlmDevType)simxpt->m_devType, 
                                (nlm_u8)num_devices,
                                portType,
                                (NlmSmtMode)smtMode,
                                NULL, 
                                log_p, 
                                o_reason);

    if (!model_p)
    {
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
#ifndef NLMPLATFORM_BCM
            exit(1);
#else
            return NLMERR_FAIL;
#endif
        }
    }
 #endif
 
    simxpt->m_client_p = (void *) model_p;

#ifdef FIB_PMD
    g_model_ptr = model_p;
#endif

return errNum;
}

NlmModel *kbp_simxpt_get_model_ptr(
    void *self
    )
{
    NlmSimXpt *simXpt = (NlmSimXpt *)self;

    return (NlmModel *)simXpt->m_client_p;
}

void NlmSimXpt__SetModelPointer(
        NlmXpt    *self,
    NlmModel *model_p
    )
{
    NlmSimXpt *simXpt = (NlmSimXpt *)self->m_derived;

    simXpt->m_client_p = model_p;
}

/* Wraper functions to support old APIs */

void NlmSimXpt__destroy(
    NlmXpt* self
)
{
    kbp_simxpt_destroy(self);   
}

NlmModel *NlmSimXpt__GetModelPointer(
    void *self
    )
{
    return kbp_simxpt_get_model_ptr(self);
}

NlmXpt *
NlmSimXpt__Create(
        NlmCmAllocator          *alloc,         /*General purpose memory allocator */
        nlm_u32                 devType,
        nlm_u32                 max_rqt_count,  /*Max request count */
        nlm_u32                 max_rslt_count, /*Max result count*/
        nlm_u32                 chan_id
)
{
    return kbp_simxpt_init(alloc,devType,max_rqt_count,max_rslt_count,chan_id);
}

#ifndef NLMPLATFORM_BCM

void bcm_kbp_simxpt_destroy(
    NlmXpt* self
)
{
    kbp_simxpt_destroy(self);   
}

NlmModel *bcm_kbp_simxpt_get_model_ptr(
    void *self
    )
{
    return kbp_simxpt_get_model_ptr(self);
}

NlmXpt *
bcm_kbp_simxpt_init(
        NlmCmAllocator          *alloc,         /*General purpose memory allocator */
        nlm_u32                 devType,
        nlm_u32                 max_rqt_count,  /*Max request count */
        nlm_u32                 max_rslt_count, /*Max result count*/
        nlm_u32                 chan_id
)
{
    return kbp_simxpt_init(alloc,devType,max_rqt_count,max_rslt_count,chan_id);
}

#endif




