/* $Id: arad_kbp_xpt.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TCAM

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dcmn/error.h>
#include <soc/kbp/alg_kbp/include/kbp_legacy.h>

#include <soc/dpp/SAND/Management/sand_low_level.h>
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/ARAD/arad_kbp_xpt.h>
#include <soc/dpp/ARAD/arad_kbp_rop.h>
#include <soc/dpp/ARAD/arad_kbp_recover.h>


#if defined NLM_MT_OLD || defined NLM_MT
#define NLM_MT_XPT
#endif

#ifdef NLM_MT_XPT
 #include "nlmcmmt.h"
#endif


/* } */

/*************
 * DEFINES   *
 *************/
/* { */
#define   ARADXPT_MIN_RQT_CNT            (1)
#define   ARADXPT_MAX_RQT_CNT            (0x0FFFFFFFF)
#define   ARADXPT_EMPTY_RTQ              (0)
#define   ARADXPT_MIN_RSLT_CNT           (1)

#define   ARADXPT_NUM_ALLOC_CNT          (1)

#define   ARADXPT_OPCODE_0               (0)
#define   ARADXPT_OPCODE_1               (1)
int
handle_sand_result(uint32 soc_sand_result);
#define SOC_SAND_FAILURE(_sand_ret) \
    ((handle_sand_result(_sand_ret)) < 0)

#define SOC_SAND_KBP_ROP_FAILURE(_sand_ret)     (SOC_SAND_FAILURE(_sand_ret))

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
/*
 * nlmaradxpt module implementes the operations defined by base xpt module to
 * work with the cmodel.
 * If the max_rqt_count is 1 and max_rslt_count is 1, then we don't maintain a
 * queue for the reqests and results but instead use m_single_rqt_p. This helps
 * avoid the queuing overhead
 */

/*
 * NlmXptRqtQue is a list of requests
 */
typedef struct NlmAradXptRqt_t 
{
    NlmXptRqt m_nlm_rqt;
    NlmBool m_done;            /* If the request is serviced */
    NlmRequestId  m_rqt_id;          /* Request id assigned by the xpt */
    struct NlmAradXptRqt_t *next;
    nlm_u32 aradxpt_rqt_magic;

}NlmAradXptRqt;

typedef struct NlmXptRqtQue_t 
{
    nlm_u32 count;  /*how many results on the que */
    NlmAradXptRqt *head;   /*head of the queue*/
    NlmAradXptRqt *tail;   /*tail of the queue*/

} NlmXptRqtQue;

typedef struct NlmAradXpt_t 
{
    NlmXpt*                xpt;    /* Pointer to virtual xpt object */
    NlmCmAllocator*        m_alloc_p; /* General purpose memory Allocator */
    nlm_u32                m_max_rqt_count; /*Max request count */
    nlm_u32                m_max_rslt_count; /* Max rslt cnt    */
    NlmXptRqtQue           m_in_use_requests; /* Request being used  */
    NlmXptRqtQue           m_free_requests; /* Free requests */

#if !defined NLM_MT_OLD && !defined NLM_MT
    NlmAradXptRqt*          m_single_rqt_p; /* Used for storing a single request in flat xpt without queuing */
#else
    NlmAradXptRqt*          m_single_rqt_p[NLMNS_MT_MAXNUM_THREADS]; /* Used for storing a single request in flat xpt without queuing */
#endif

    NlmRequestId           m_next_rqt_id;
    nlm_u32                aradxpt_magic;

#ifdef NLM_MT_XPT 
#ifdef WIN32
    HANDLE   m_xpt_spinLock;
#else
    NlmCmMtSpinlock   m_xpt_spinLock;
#endif
#endif

    /* Arad Specific */
    int unit;
    int core;

} NlmAradXpt;

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

#define NlmAradXpt__GetInUseRequestCount(self)    ((self)->m_in_use_requests.count)
/*
 * The magic numbers are used to verify if a particular object is of the same
 * object type as the one we are looking for.
 */

#define NLM_ARADXPT_MAGIC_NUMBER          (0xe11ad)
#define NLM_ARADXPT_RQT_MAGIC_NUMBER      (0xe11adb)

#define IS_ARADXPT(x)         ((x) && ((x)->aradxpt_magic == NLM_ARADXPT_MAGIC_NUMBER))
#define IS_ARADXPTRQT(x)      ((x) && ((x)->aradxpt_rqt_magic == NLM_ARADXPT_RQT_MAGIC_NUMBER))

#define MAXIMIZE_ERROR_SEVERITY(x, y) \
      if (x) {if ((*x != NLMRSC_OPR_FAILURE) &&(NLMRSC_OPR_FAILURE == y))\
                       *x = (NlmReasonCode)y;\
          else if (*x != NLMRSC_INVALID_PARAM  && (NLMRSC_INVALID_PARAM == y))\
                            *x = (NlmReasonCode)y;}
#define MOVE_TO_FREE_LIST(inuse, element, freelist) \
                NlmAradXpt__TakeOutRequest(inuse, element);\
                memset(element, 0, sizeof(NlmAradXptRqt));\
                NlmAradXpt__AddToRequestQue(freelist, element)

/* } */



/*************
 * GLOBALS   *
 *************/
/* { */


/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */
/*
 * Functiondeclarations
 */

STATIC void NlmAradXpt__FreeRequestQue(
        NlmCmAllocator *alloc,
        NlmXptRqtQue *rq);

STATIC void NlmAradXpt__AddToRequestQue(
        NlmXptRqtQue *rque,
        NlmAradXptRqt *rqt);

STATIC void NlmAradXpt__pvt_destroy(
    void *self
);


STATIC void NlmAradXpt__TakeOutRequest(
        NlmXptRqtQue *rque,
        NlmAradXptRqt *rqt
);

STATIC NlmAradXptRqt* NlmAradXpt__RemoveLastRequest(
        NlmXptRqtQue *rque
);
STATIC void NlmAradXpt__AssignRequestId(
    NlmAradXpt *self,
    NlmAradXptRqt *aradrqt);

STATIC NlmAradXptRqt *NlmAradXpt__NextToExecute(
    NlmAradXpt *self
);

STATIC nlm_u32 NlmAradXpt__GetAvailableResultCount(
    NlmAradXpt *self
);


STATIC void  NlmAradXpt__ExecuteRequest(
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmReasonCode *o_reason
);

STATIC void NlmAradXpt__WriteToDevice(
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmReasonCode *o_reason
);
STATIC void NlmAradXpt__WriteToDeviceBlk(
    nlm_u32 opcode,
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmReasonCode *o_reason
);
STATIC void NlmAradXpt__ReadFromDevice(
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmBool is_x_read,
    NlmReasonCode *o_reason
);
STATIC void NlmAradXpt__WriteToDeviceCtxBf(
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmReasonCode *o_reason
);
STATIC void NlmAradXpt__CompareData(
        NlmAradXpt *aradxpt,
        NlmAradXptRqt *aradrqt,
        NlmAradCBInstType cbInstType,
        NlmReasonCode *o_reason
);
STATIC void*  NlmAradXpt__GetResult(
        void*              self,
        NlmXptRqt*              rqt,
        NlmReasonCode *o_reason
);

STATIC void NlmAradXpt__PerformNOP(
        NlmAradXpt *aradxpt,
        NlmAradXptRqt *aradrqt);

STATIC NlmErrNum_t NlmAradXpt__ResetDevices(
        void *self,
        NlmReasonCode *o_reason
);

STATIC NlmErrNum_t NlmAradXpt__LockConfig(
        void *self,
        nlm_u32 num_devies,
        NlmReasonCode *o_reason
);


/*******************************************************************************
 * Function : NlmAradXpt__GetResult
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXpt      *rqt           = Pointer to NlmRequest object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__GetResult returns a filled in result structure to upper layer of
 * the s/w. It will return NULL if (a) the NlmXptRqt onbject is not in the
 * in_use request list, or, (b) the request is not completed.
 * If the NlmXptRqt object passed to the function is NULL, then the result for
 * the first request in the in_use request list is returned.
 * If NlmAradXpt__GetResult is returning a non-null result,then the corresponding
 * request object will be moved to free request list.
 ******************************************************************************/
STATIC void*  NlmAradXpt__GetResult(
    void*              self,
    NlmXptRqt*         rqt,
    NlmReasonCode      *o_reason
)
{
    NlmAradXpt  *aradxpt = NULL;
    NlmAradXptRqt *aradrqt= NULL, *temp = NULL;


    aradrqt = (NlmAradXptRqt *)rqt;
    aradxpt = (NlmAradXpt *)self;

    if (!IS_ARADXPT(aradxpt) || !IS_ARADXPTRQT(aradrqt)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NULL;
    }

    if(aradxpt->m_max_rqt_count == ARADXPT_MIN_RQT_CNT &&
        aradxpt->m_max_rslt_count == ARADXPT_MIN_RSLT_CNT)
    {

#if !defined NLM_MT_OLD && !defined NLM_MT
        if (aradxpt->m_single_rqt_p == aradrqt)
#else
#ifdef NLM_NETOS
        nlm_u32 threadID = 0;
        threadID = NlmCmMt__GetThreadID(getpid());
#endif
        if (aradxpt->m_single_rqt_p[threadID] == aradrqt)
#endif
        {
                void *result;
                if (!aradrqt->m_done)
                    return NULL;
                result = aradrqt->m_nlm_rqt.m_result;
#ifdef NLM_MT_XPT 
                NlmCmMt__SpinUnlock(&aradxpt->m_xpt_spinLock);
#endif
                return result;
        }
        return NULL;
    }


    if (!NlmAradXpt__GetAvailableResultCount(aradxpt)) {
        if (o_reason)
            *o_reason = NLMRSC_NORSLT_AVBL;
        return NULL;
    }
    temp = aradxpt->m_in_use_requests.head;

    while (temp) 
    {
        if (temp == aradrqt) 
        {
            void *result;
            if (!aradrqt->m_done)
                return NULL;
            result = aradrqt->m_nlm_rqt.m_result;

            MOVE_TO_FREE_LIST(&aradxpt->m_in_use_requests,
                    aradrqt, &aradxpt->m_free_requests);
            
#ifdef NLM_MT_XPT 
    NlmCmMt__SpinUnlock(&aradxpt->m_xpt_spinLock);
#endif
            return result;
        }
        temp = temp->next;
    }

    return NULL;
}


/*******************************************************************************
 * Function : NlmAradXpt__GetRequest
 *
 * Parameters:
 * void*         self         = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__GetXptRequest returns a request object to the caller. Null is
 * returned if there is no more free requests. The returned request is moved to
 * in_use list.
 ******************************************************************************/
STATIC NlmXptRqt* NlmAradXpt__GetRequest(
    void*         self,
    NlmReasonCode *o_reason)
{
    NlmAradXpt *aradxpt = NULL;
    NlmAradXptRqt *aradrqt = NULL;

    if (!self || !IS_ARADXPT((NlmAradXpt *)self)) {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;

        return NULL;
    }

    aradxpt = (NlmAradXpt *)self;

#ifdef NLM_MT_XPT 
    NlmCmMt__SpinLock(&aradxpt->m_xpt_spinLock);
 #endif

    if(aradxpt->m_max_rqt_count == ARADXPT_MIN_RQT_CNT &&
        aradxpt->m_max_rslt_count == ARADXPT_MIN_RSLT_CNT)
    {
#if !defined NLM_MT_OLD && !defined NLM_MT
        aradrqt = aradxpt->m_single_rqt_p;
#else
#ifdef NLM_NETOS
        nlm_u32 threadID = 0;
        threadID = NlmCmMt__GetThreadID(getpid());
#endif

        aradrqt = aradxpt->m_single_rqt_p[threadID];
#endif

        memset(aradrqt, 0, sizeof(NlmAradXptRqt));

        aradrqt->aradxpt_rqt_magic = NLM_ARADXPT_RQT_MAGIC_NUMBER;

        return (NlmXptRqt *) aradrqt;
    }

    aradrqt = NlmAradXpt__RemoveLastRequest(&aradxpt->m_free_requests);

    if (!aradrqt) 
    {
        if (o_reason)
            *o_reason = NLMRSC_NOFREE_RQST;
        return NULL;
    }

    memset(aradrqt, 0, sizeof(NlmAradXptRqt));

    NlmAradXpt__AssignRequestId(aradxpt, aradrqt);

    aradrqt->aradxpt_rqt_magic = NLM_ARADXPT_RQT_MAGIC_NUMBER;

    NlmAradXpt__AddToRequestQue(&aradxpt->m_in_use_requests, aradrqt);

    return (NlmXptRqt *) aradrqt;

}

/*******************************************************************************
 * Function : NlmAradXpt__RemoveLastRequest
 *
 * Parameters:
 * NlmXptRqtQue *rque      = Pointer to a request queue.
 *
 * Summary:
 * NlmAradXpt__RemoveLastRequest: Takes out the last element of a rquest queue
 * and returns that element.
 ******************************************************************************/
STATIC NlmAradXptRqt* NlmAradXpt__RemoveLastRequest(
    NlmXptRqtQue *rque
)
{
    NlmAradXptRqt *aradrqt = NULL;

    if (rque->count == ARADXPT_EMPTY_RTQ)
        return NULL;

    aradrqt = rque->head;

    NlmAradXpt__TakeOutRequest(rque, aradrqt);

    return aradrqt;
}


/*******************************************************************************
 * Function : NlmAradXpt__AssignRequestId
 *
 * Parameters:
 * NlmAradXpt *self         = Pointer to an NlmAradXpt object
 * NlmAradXptRqt *aradrqt    = Poniter to an NlmAradXptRqt object
 *
 * Summary:
 * Asssign a request Id to a NlmAradXptRqt object
 ******************************************************************************/
STATIC void NlmAradXpt__AssignRequestId(
    NlmAradXpt *self,
        NlmAradXptRqt *aradrqt)
{
    if (!self || !aradrqt) 
        return;

    memcpy(&aradrqt->m_rqt_id, &self->m_next_rqt_id, sizeof(NlmRequestId));

    if (self->m_next_rqt_id.m_count < (ARADXPT_MAX_RQT_CNT - 0x1)) 
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
 * Function : NlmAradXpt__ResetRequests
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
STATIC void NlmAradXpt__ResetRequests(
    void*    self,
    NlmReasonCode *o_reason
)
{
    NlmAradXpt *aradxpt = NULL;
    aradxpt = (NlmAradXpt *) self;
    (void)o_reason;

    if (!IS_ARADXPT(aradxpt)) return;

    if(aradxpt->m_max_rqt_count == ARADXPT_MIN_RQT_CNT &&
        aradxpt->m_max_rslt_count == ARADXPT_MIN_RSLT_CNT)
    {
        return ;
    }

    while (NlmAradXpt__GetInUseRequestCount(aradxpt)) 
    {
        NlmAradXptRqt *rqt;
        rqt = aradxpt->m_in_use_requests.head;
        NlmAradXpt__TakeOutRequest(&aradxpt->m_in_use_requests, rqt);
        memset(rqt, 0, sizeof(NlmAradXpt));
        NlmAradXpt__AddToRequestQue(&aradxpt->m_free_requests, rqt);
    }


}
/*******************************************************************************
 * Function : NlmAradXpt__NextToExecute
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmAradXpt object
 *
 * Summary:
 * Returns the next request to be executed from the in_use request queue.
 ******************************************************************************/
STATIC NlmAradXptRqt *NlmAradXpt__NextToExecute(
    NlmAradXpt *self
)
{

    NlmAradXptRqt *aradrqt = NULL;

    if (!IS_ARADXPT(self) || !NlmAradXpt__GetInUseRequestCount(self))
        return NULL;

    aradrqt = self->m_in_use_requests.head;

    while (aradrqt) 
    {
        if (!aradrqt->m_done) return aradrqt;
        aradrqt  = aradrqt->next;
    }
    return NULL;
}

/*******************************************************************************
 * Function : NlmAradXpt__ServiceRequests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 *  NlmAradXpt__ServiceRequests  takes  the requests from the in_use request
 *  queue and execute them one by one. The "WRITE" requests are moved to
 *  free_list while COMPARE/READ requests are left in the in_use request queue
 *  itself. The COMPARE/READ requests are moved to free_list as results of calls
 *  to "GetResult" routine. Requests are always executed in FIFO order.
 ******************************************************************************/
STATIC void
NlmAradXpt__ServiceRequests(
    void*            self,            /* transport interface to flush */
    NlmReasonCode    *o_reason
)
{
    NlmAradXpt *aradxpt = NULL;
    NlmAradXptRqt *aradrqt = NULL;
#ifdef NLM_MT_XPT 
    void *result = NULL;
#endif

    if (!self) 
        return;

    aradxpt = (NlmAradXpt *)self;

    if (!IS_ARADXPT(aradxpt)) 
    {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }

    if (o_reason)
        *o_reason = NLMRSC_REASON_OK;


    if(aradxpt->m_max_rqt_count == ARADXPT_MIN_RQT_CNT &&
        aradxpt->m_max_rslt_count == ARADXPT_MIN_RSLT_CNT)
    {
#if !defined NLM_MT_OLD && !defined NLM_MT
        aradrqt = aradxpt->m_single_rqt_p;
#else
#ifdef NLM_NETOS
        nlm_u32 threadID = 0;
        threadID = NlmCmMt__GetThreadID(getpid());
#endif
        aradrqt = aradxpt->m_single_rqt_p[threadID];
#endif
        NlmAradXpt__ExecuteRequest(aradxpt, aradrqt, o_reason);

#ifdef NLM_MT_XPT       
        if(aradrqt->m_nlm_rqt.m_result == NULL)
        {
            NlmCmMt__SpinUnlock(&aradxpt->m_xpt_spinLock);
        }
#endif      
        return;
    }


    if (!NlmAradXpt__GetInUseRequestCount(aradxpt))
        return;

    while (NULL != (aradrqt = NlmAradXpt__NextToExecute(aradxpt)))
    {
        NlmAradXpt__ExecuteRequest(aradxpt, aradrqt, o_reason);
#ifdef NLM_MT_XPT 
        result = aradrqt->m_nlm_rqt.m_result;
#endif
    }

#ifdef NLM_MT_XPT 
    if(result == NULL)
    {
        NlmCmMt__SpinUnlock(&aradxpt->m_xpt_spinLock);
    }
#endif
}

/*******************************************************************************
 * Function : NlmAradXpt__ExecuteRequest
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmAradXptRqt *aradrqt,     = Pointer to a NlmAradRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__ExecuteRequest Invokes appropriate routine to execute the commands
 * described in aradrqt
 ******************************************************************************/

STATIC void NlmAradXpt__ExecuteRequest(
        NlmAradXpt *aradxpt,
        NlmAradXptRqt *aradrqt,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *p = NULL;
    nlm_u32 sev_param = 0;
    NlmReasonCode dummyReason = NLMRSC_REASON_OK; 

    if (!IS_ARADXPT(aradxpt) || !IS_ARADXPTRQT(aradrqt))
    {
        sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);
        return;
    }

    if(o_reason == NULL)
        o_reason = &dummyReason;

    p = (NlmXptRqt *) aradrqt;

    if ((NLM_OPCODE_REG_WRITE_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
        (NLM_OPCODE_REG_WRITE_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1])) 
    {
        NlmAradXpt__WriteToDevice(aradxpt, aradrqt, o_reason);
    }
    else if ((NLM_OPCODE_DBA_WRITE_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
        (NLM_OPCODE_DBA_WRITE_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1])) 
    {
        NlmAradXpt__WriteToDevice(aradxpt, aradrqt, o_reason);
    }
    else if ((NLM_OPCODE_UDA_WRITE_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
        (NLM_OPCODE_UDA_WRITE_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1])) 
    {
        NlmAradXpt__WriteToDevice(aradxpt, aradrqt, o_reason);
    }
    else if (NLM_OPCODE_CBWRITE_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) 
    {
        NlmAradXpt__WriteToDeviceCtxBf(aradxpt, aradrqt, o_reason);
    }
    else if ((NLM_OPCODE_CBWRITE_CMP1_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS >  p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__CompareData(aradxpt, aradrqt, NLM_ARAD_CB_INST_CMP1, o_reason);
    }

    else if ((NLM_OPCODE_CBWRITE_CMP2_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS >  p->m_opcode[ARADXPT_OPCODE_1])) 
    {
        NlmAradXpt__CompareData(aradxpt, aradrqt, NLM_ARAD_CB_INST_CMP2, o_reason);
    }
    else if ((NLM_OPCODE_CBWRITE_CMP3_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS >  p->m_opcode[ARADXPT_OPCODE_1])) 
    {
        NlmAradXpt__CompareData(aradxpt, aradrqt, NLM_ARAD_CB_INST_CMP3, o_reason);
    }
    else if ((NLM_OPCODE_NOP_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLM_OPCODE_NOP_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__PerformNOP(aradxpt, aradrqt);
    }
    else if ((NLM_OPCODE_REG_READ_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLM_OPCODE_REG_READ_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__ReadFromDevice(aradxpt, aradrqt, NLMTRUE, o_reason);
    }
    else if ((NLM_OPCODE_UDA_READ_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLM_OPCODE_UDA_READ_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__ReadFromDevice(aradxpt, aradrqt, NLMTRUE, o_reason);
    }
    else if ((NLM_OPCODE_DBA_READ_X_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLM_OPCODE_DBA_READ_X_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__ReadFromDevice(aradxpt, aradrqt, NLMTRUE, o_reason);
    }
    else if ((NLM_OPCODE_DBA_READ_Y_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLM_OPCODE_DBA_READ_Y_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__ReadFromDevice(aradxpt, aradrqt, NLMFALSE, o_reason);

    }
    else if ((NLM_OPCODE_CBWLPM_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                (NLMDEV_MAX_NUM_LTRS > p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__CompareData(aradxpt, aradrqt, NLM_ARAD_CB_INST_LPM, o_reason);
    }
    else if ((NLM_OPCODE_BLOCK_COPY_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                 (NLM_OPCODE_BLOCK_COPY_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__WriteToDeviceBlk(ARAD_KBP_CPU_BLK_COPY_OPCODE, aradxpt, aradrqt, o_reason);
    }
    else if ((NLM_OPCODE_BLOCK_MOVE_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                 (NLM_OPCODE_BLOCK_MOVE_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__WriteToDeviceBlk(ARAD_KBP_CPU_BLK_MOVE_OPCODE, aradxpt, aradrqt, o_reason);
    }
    else if ((NLM_OPCODE_BLOCK_CLEAR_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                 (NLM_OPCODE_BLOCK_CLEAR_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__WriteToDeviceBlk(ARAD_KBP_CPU_BLK_CLR_OPCODE, aradxpt, aradrqt, o_reason);
    }
    else if ((NLM_OPCODE_BLOCK_EV_BITS_8_6 == p->m_opcode[ARADXPT_OPCODE_0]) &&
                 (NLM_OPCODE_BLOCK_EV_BITS_5_0 == p->m_opcode[ARADXPT_OPCODE_1]))
    {
        NlmAradXpt__WriteToDeviceBlk(ARAD_KBP_CPU_BLK_EV_OPCODE, aradxpt, aradrqt, o_reason);
    }
    else 
    {
        nlm_u32 sev_param = NLMRSC_INVALID_PARAM;
        MAXIMIZE_ERROR_SEVERITY(o_reason, sev_param);

        if(aradxpt->m_max_rqt_count != ARADXPT_MIN_RQT_CNT ||aradxpt->m_max_rslt_count != ARADXPT_MIN_RSLT_CNT)
        {
            MOVE_TO_FREE_LIST(&aradxpt->m_in_use_requests, aradrqt,
                        &aradxpt->m_free_requests);
        }
    }
}


extern uint8 hit_bit_result_for_do_search;

/* This function converts the compare results to the format required by Device Manager */
STATIC void 
NlmAradXpt__pvt_FormatCompareResult(
            nlm_u8 *o_compareRslt,
            NlmAradCompareResult *compareRslt,
            uint32 ltr
            )
{
    nlm_u32 
        i = 0, 
        align_ad = 0x0,
        o_compareRslt_offset = 0,
        compareRslt_offset = 1; /* Skip the ststus byte*/

    /*
    Each search result can have 32b of result index and 32/64/128/256 of AD (optional). 
    Bit[31] of result indicates result Valid/Invalid, Bit[30] of result indicates HIT/MISS, Bits[29:27]
    indicates length of AD and Bits[23:0] indicates HIT Index 
    */
    
    hit_bit_result_for_do_search = compareRslt->data_raw[0];

    for (i = 0; i < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; i++) {
        if (arad_kbp_frwd_ltr_db[ltr].res_format[i] != NLM_ARAD_NO_INDEX_NO_AD) {
            compareRslt->isResultValid[i] = 0x1;
            compareRslt->hitOrMiss[i] = compareRslt->data_raw[0] & (0x1 << (7 - i));
            compareRslt->responseFormat[i] = arad_kbp_frwd_ltr_db[ltr].res_format[i];
            LOG_VERBOSE(BSL_LS_SOC_TCAM,
                        (BSL_META("%s(): i=%d, isResultValid=%d, hitOrMiss=%d, responseFormat=0x%x"), 
FUNCTION_NAME(), i, compareRslt->isResultValid[i], compareRslt->hitOrMiss[i], compareRslt->responseFormat[i]));

        if(arad_kbp_frwd_ltr_db[ltr].res_format[i] == NLM_ARAD_ONLY_INDEX_NO_AD) {
        /* Write the 32b field of result - RV, SMF, index, AD response format*/
        o_compareRslt[o_compareRslt_offset + 0] = compareRslt->isResultValid[i] ? (1 << 7) : 0;
        o_compareRslt[o_compareRslt_offset + 0] |= (compareRslt->hitOrMiss[i] ? (1 << 6) : 0);
        o_compareRslt[o_compareRslt_offset + 0] |= (compareRslt->responseFormat[i] << 3);

	#if 0
        o_compareRslt[o_compareRslt_offset + 1] = (nlm_u8) ((compareRslt->hitIndex[i] >> 16) & 0xFF);
        o_compareRslt[o_compareRslt_offset + 2] = (nlm_u8) ((compareRslt->hitIndex[i] >> 8) & 0xFF);
        o_compareRslt[o_compareRslt_offset + 3] = (nlm_u8) (compareRslt->hitIndex[i] & 0xFF);
	#else
	        o_compareRslt[o_compareRslt_offset + 1] = compareRslt->data_raw[compareRslt_offset++];
	        o_compareRslt[o_compareRslt_offset + 2] = compareRslt->data_raw[compareRslt_offset++];
	        o_compareRslt[o_compareRslt_offset + 3] = compareRslt->data_raw[compareRslt_offset++];

	#endif
        o_compareRslt_offset += 4;
        }
	    if (arad_kbp_frwd_ltr_db[ltr].res_format[i] != NLM_ARAD_ONLY_INDEX_NO_AD) {

	        align_ad = (4 * (1 << (arad_kbp_frwd_ltr_db[ltr].res_format[i] - 1))) - arad_kbp_frwd_ltr_db[ltr].res_data_len[i];

                memcpy(o_compareRslt + o_compareRslt_offset + align_ad, compareRslt->data_raw + compareRslt_offset, arad_kbp_frwd_ltr_db[ltr].res_data_len[i]);
                o_compareRslt_offset += (4 * (1 << (arad_kbp_frwd_ltr_db[ltr].res_format[i] - 1)));
            
            compareRslt_offset += arad_kbp_frwd_ltr_db[ltr].res_data_len[i];
        }
        }   
    }
}


/*******************************************************************************
 * Function : NlmAradXpt__CompareData
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmAradXptRqt *aradrqt,     = Pointer to a NlmAradRqt object
 * NlmAradCBInstType cbInstType             = if it is a cb write,  compare1 or comapre2 operation
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__CompareData  sends a "COMPARE" command to the underlying device.
 ******************************************************************************/

STATIC void NlmAradXpt__CompareData(
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmAradCBInstType cbInstType,
    NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt  = (NlmXptRqt *) aradrqt;
    nlm_u8
        *data_ptr = NULL;
    arad_kbp_rop_cbw_cmp_t 
        cbw_cmp_data;
    int res;

    nlm_u8 temp_data[80] = {0, }, act_length = 0;
        memset(&cbw_cmp_data, 0x0, sizeof(arad_kbp_rop_cbw_cmp_t));
        if(xptRqt->m_data_len %10) {
                act_length = xptRqt->m_data_len;
                xptRqt->m_data_len = (((xptRqt->m_data_len/10) + 1) * 10);
                sal_memcpy(&temp_data[xptRqt->m_data_len-act_length],xptRqt->m_data_p,act_length);
                data_ptr = &temp_data[0];
        } else {
    		data_ptr = xptRqt->m_data_p;
        }


    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META("%s(): xptRqt->m_data_len=%d, data_ptr= "), FUNCTION_NAME(), xptRqt->m_data_len));
    arad_kbp_array_print_debug(data_ptr, xptRqt->m_data_len);

    if(xptRqt->m_smt_num != NLMDEV_SMT_0) {
        /* Error - Arad works only in NLMDEV_SMT_0 mode */
        *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }

    /* 
     * cbInstType - Not delivered. In Arad ROP The ctx buff type is in the LUT table instruction (not packet). 
     * xptRqt->m_ctx_addr - Not delivered. In Arad ROP the Context id is always 0x7ff8 and is in the LUT table instruction (not packet). 
     * xptRqt->m_opcode is actually the LTR - find the Arad ROP opcode relevant to this LTR
     */
    if ((cbw_cmp_data.ltr < 0) || (cbw_cmp_data.ltr >= ARAD_KBP_ROP_LTR_NUM_MAX)) {
        *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }
    cbw_cmp_data.type = cbInstType;
    cbw_cmp_data.ltr = xptRqt->m_opcode[ARADXPT_OPCODE_1];
    cbw_cmp_data.opcode = arad_kbp_frwd_ltr_db[cbw_cmp_data.ltr].opcode;
    cbw_cmp_data.cbw_data.data_len = xptRqt->m_data_len;

    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META("%s(): xptRqt->m_ctx_addr=%d, xptRqt->m_opcode[ARADXPT_OPCODE_1]=%d, opcode=%d, LTR=%d, xptRqt->m_data_len=%d\n"), 
FUNCTION_NAME(), xptRqt->m_ctx_addr, xptRqt->m_opcode[ARADXPT_OPCODE_1], cbw_cmp_data.opcode, cbw_cmp_data.ltr, xptRqt->m_data_len));
    ARAD_KBP_ROP_REVERSE_DATA(data_ptr, cbw_cmp_data.cbw_data.data, cbw_cmp_data.cbw_data.data_len);

    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META("%s(): cbw_cmp_data.cbw_data.data= "), FUNCTION_NAME()));
    arad_kbp_array_print_debug(cbw_cmp_data.cbw_data.data, cbw_cmp_data.cbw_data.data_len);

    res = arad_kbp_rop_cbw_cmp(
        aradxpt->unit,
        aradxpt->core,
        &cbw_cmp_data);
    if (SOC_SAND_KBP_ROP_FAILURE(res)) {
        *o_reason = NLMRSC_OPR_FAILURE;
        return;
    } 

    /* Get the search results For Device Manager. */
    if((cbInstType != NLM_ARAD_CB_INST_WRITE) && (cbInstType != NLM_ARAD_CB_INST_NONE)) {
        NlmAradXpt__pvt_FormatCompareResult((nlm_u8*)xptRqt->m_result, &cbw_cmp_data.result, cbw_cmp_data.ltr);

        LOG_VERBOSE(BSL_LS_SOC_TCAM,
                    (BSL_META("%s(): (nlm_u8*)xptRqt->m_result= "), FUNCTION_NAME()));
        arad_kbp_array_print_debug((nlm_u8*)xptRqt->m_result, 80);
    }

    aradrqt->m_done = 1;
}
/*******************************************************************************
 * Function : NlmAradXpt__PerformNOP
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmARadXptRqt *aradrqt,     = Pointer to a NlmAradRqt object
 *
 * Summary:
 * NlmAradXpt__PerformNOP  doesn't do anything. It just move the request to free_queue.
 ******************************************************************************/
STATIC void NlmAradXpt__PerformNOP(
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt
    )
{

    if(aradxpt->m_max_rqt_count == ARADXPT_MIN_RQT_CNT &&
            aradxpt->m_max_rslt_count == ARADXPT_MIN_RSLT_CNT)
    {
        return;
    }

    MOVE_TO_FREE_LIST(&aradxpt->m_in_use_requests, aradrqt,
            &aradxpt->m_free_requests);

}
/*******************************************************************************
 * Function : NlmAradXpt__ReadFromDevice
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmAradXptRqt *aradrqt,     = Pointer to a NlmAradRqt object
 * NlmBool is_x_read         = if it is a data or mask request
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__ReadFromDevice  sends a READ command to the underlying device.
 ******************************************************************************/

STATIC void NlmAradXpt__ReadFromDevice(
        NlmAradXpt *aradxpt,
        NlmAradXptRqt *aradrqt,
        NlmBool is_x_read,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt = (NlmXptRqt *) aradrqt;
    uint8
        *addr_ptr = NULL,
        *data_ptr = NULL;
    arad_kbp_rop_read_t 
        rd_data;
    nlm_u32 
        addr = 0;
    int
        res;   

    if (aradxpt == NULL) {
        LOG_VERBOSE(BSL_LS_SOC_TCAM,
                    (BSL_META("%s(): 'aradxpt == NULL'.\n"), FUNCTION_NAME()));
        *o_reason = NLMRSC_INVALID_XPT_PTR;
        return;
    }

    if (aradrqt == NULL) {
        LOG_VERBOSE(BSL_LS_SOC_TCAM,
                    (BSL_META("%s(): 'aradrqt == NULL'.\n"), FUNCTION_NAME()));
        *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return;
    }

    data_ptr = (uint8 *)xptRqt->m_result;
    addr     = xptRqt->m_address; /* addr */
    addr     = soc_htonl(addr); 
    addr_ptr = (uint8*)&(addr);

    /* Copy data */
    memcpy(rd_data.addr, addr_ptr, NLMDEV_REG_ADDR_LEN_IN_BYTES);

    if(is_x_read) {/* If database read see whether Read for X or Read for Y */
        rd_data.dataType = NLM_ARAD_READ_MODE_DATA_X;
    } else {
        rd_data.dataType = NLM_ARAD_READ_MODE_DATA_Y;
    }

    res = arad_kbp_rop_read(aradxpt->unit, aradxpt->core, &rd_data);
    if (SOC_SAND_KBP_ROP_FAILURE(res)) {
        *o_reason = NLMRSC_OPR_FAILURE;
        return;
    }   

    /* Byte 0 for control bits, Byte 1 to 10 will have 80b data */
    memcpy(data_ptr, rd_data.data, NLM_DATA_WIDTH_BYTES + 1);

    aradrqt->m_done = 1; /*Request complete */

}
/*******************************************************************************
 * Function : NlmAradXpt__WriteToDevice
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmAradXptRqt *aradrqt,     = Pointer to a NlmAradRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__WriteToDevice  sends a WRITE command to the underlying device.
 ******************************************************************************/
STATIC void NlmAradXpt__WriteToDevice(
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt  = (NlmXptRqt *) aradrqt;
    nlm_u8
        *addr_ptr = NULL,
        *data_ptr = NULL,
        *mask_ptr = NULL;
    arad_kbp_rop_write_t 
        wr_data;
    nlm_u32 
        addr,
        addrType = 0;
    int
        res;
	int unit = BSL_UNIT_UNKNOWN;
    ARAD_INIT_ELK *elk = NULL;

    if ((aradxpt == NULL) || (xptRqt->m_data_p == NULL)) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "%s(): '((aradxpt == NULL) || (xptRqt->m_data_p == NULL))'.\n"), FUNCTION_NAME()));
        *o_reason = NLMRSC_INVALID_XPT_PTR;
        return;
    }

    if (aradrqt == NULL) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "%s(): 'aradrqt == NULL'.\n"), FUNCTION_NAME()));
        *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return;
    }
    elk = &SOC_DPP_CONFIG(aradxpt->unit)->arad->init.elk;
    addr = xptRqt->m_address;/*ReadBitsInArray(xptRqt->m_data_p, NLMDEV_REG_ADDR_LEN_IN_BYTES, 31, 0);*/
    addr = soc_htonl(addr); 
    addr_ptr = (nlm_u8*)&(addr);/*xptRqt->m_data_p;*/ /* addr */
    data_ptr = xptRqt->m_data_p;/* + NLMDEV_AB_ADDR_LEN_IN_BYTES;  data */
    mask_ptr = xptRqt->m_mask_p;/* + NLMDEV_AB_ADDR_LEN_IN_BYTES + NLMDEV_AB_WIDTH_IN_BYTES;*/ /* mask */

    /* Copy data */
    memcpy(wr_data.addr, addr_ptr, NLMDEV_REG_ADDR_LEN_IN_BYTES);
    memcpy(wr_data.data, data_ptr, NLM_DATA_WIDTH_BYTES);

    /* if Write operation is for DB Entry then format mask also*/
    addrType = ((xptRqt->m_address) >> NLM_ADDR_OP_TYPE_BIT_START_POS) % NLM_NUM_ADDR_TYPES;
    if ((addrType == NLMDEV_ADDR_TYPE_DBA) && ((mask_ptr != NULL))){
        memcpy(wr_data.mask, mask_ptr, NLM_DATA_WIDTH_BYTES);
    } else {
        memset(wr_data.mask, 0x0, NLM_DATA_WIDTH_BYTES);
    }

#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
	soc_sand_ll_timer_stop(ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD);
    soc_sand_ll_timer_set("ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD_ROP", ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD_ROP);
#endif

    res = arad_kbp_rop_write(aradxpt->unit, aradxpt->core, &wr_data);
    if ((res != 0) && elk->kbp_recover_enable) {
        res = arad_kbp_recover_run_recovery_sequence(aradxpt->unit, aradxpt->core,
                                                     elk->kbp_mdio_id[aradxpt->core],
                                                     elk->kbp_recover_iter, (void *)&wr_data, 3);
    }


#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
	soc_sand_ll_timer_stop(ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD_ROP);
	soc_sand_ll_timer_set("ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD", ARAD_KBP_IPV4_TIMERS_LPM_ROUTE_ADD);
#endif

    if (SOC_SAND_KBP_ROP_FAILURE(res)) {
        *o_reason = NLMRSC_OPR_FAILURE;
        return;
    }

    if(aradxpt->m_max_rqt_count != ARADXPT_MIN_RQT_CNT ||aradxpt->m_max_rslt_count != ARADXPT_MIN_RSLT_CNT) {
        MOVE_TO_FREE_LIST(&aradxpt->m_in_use_requests, aradrqt,&aradxpt->m_free_requests);
    }
}

/*******************************************************************************
 * Function : NlmAradXpt__WriteToDeviceBlk
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmAradXptRqt *aradrqt,     = Pointer to a NlmAradRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__WriteToDevice  sends a WRITE command to the underlying device.
 ******************************************************************************/
STATIC void NlmAradXpt__WriteToDeviceBlk(
    nlm_u32 opcode,
    NlmAradXpt *aradxpt,
    NlmAradXptRqt *aradrqt,
    NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt  = (NlmXptRqt *) aradrqt;
    nlm_u8
        *data_ptr = NULL;
    arad_kbp_rop_blk_t 
        wr_data;
    int
        res;

    data_ptr = xptRqt->m_data_p;/* + NLMDEV_AB_ADDR_LEN_IN_BYTES;  data */

    wr_data.opcode = opcode;

    if ((opcode == ARAD_KBP_CPU_BLK_CLR_OPCODE) || (opcode == ARAD_KBP_CPU_BLK_EV_OPCODE)) {
 
        /* 
         * Received data from device manager is in the following structure: 
         * {LoopCnt[3:0], 1'b0, DEVID[1:0], VBIT, 4'b0, LoopCnt[11:4], Addr[19:0]  
         */ 
        wr_data.loop_cnt = ((data_ptr[0] >> 4) | (data_ptr[1] << 4));
        wr_data.src_addr = ((data_ptr[4] << 16) | (data_ptr[3] << 8) | data_ptr[2]);
        wr_data.set_not_clr = data_ptr[0] & 0x1;
        wr_data.data_len = 5;

    }
    else { /* Block Copy/Move */

        /* 
         * Received data from device manager is in the following structure: 
         * {LoopCnt[11:0], 3'b0, CntDir, 4'b0, SrcAddr[19:0], DstAddr[19:0] 
         */ 
        wr_data.loop_cnt = ((data_ptr[0] >> 4) | (data_ptr[1] << 4));
        wr_data.src_addr = ((data_ptr[2]) | (data_ptr[3] << 8) | (data_ptr[4] << 16));
        wr_data.copy_dir = (data_ptr[0] & 0x1);  
        wr_data.dst_addr = ((data_ptr[5]) | (data_ptr[6] << 8) | (data_ptr[7] << 16));
        wr_data.data_len = 8;
    }

    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META("%s(): data_len=%d, opcode=%d\n\r"), FUNCTION_NAME(),wr_data.data_len, wr_data.opcode ));

    res = arad_kbp_rop_blk_write(aradxpt->unit, aradxpt->core, &wr_data);
    if (SOC_SAND_KBP_ROP_FAILURE(res)) {
        *o_reason = NLMRSC_OPR_FAILURE;
        return;
    }

    if(aradxpt->m_max_rqt_count != ARADXPT_MIN_RQT_CNT ||aradxpt->m_max_rslt_count != ARADXPT_MIN_RSLT_CNT) {
        MOVE_TO_FREE_LIST(&aradxpt->m_in_use_requests, aradrqt,&aradxpt->m_free_requests);
    }
}
/*******************************************************************************
 * Function : NlmAradXpt__WriteToDeviceCtxBf
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmAradXptRqt *aradrqt,     = Pointer to a NlmAradRqt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__WriteToDeviceCtxBf  sends a WRITE TO CTX BUFFER command to the underlying
 * device.
 ******************************************************************************/

STATIC void NlmAradXpt__WriteToDeviceCtxBf(
        NlmAradXpt *aradxpt,
        NlmAradXptRqt *aradrqt,
        NlmReasonCode *o_reason
)
{
    NlmXptRqt *xptRqt  = (NlmXptRqt *) aradrqt;
    nlm_u8
        *data_ptr = NULL;
    arad_kbp_rop_cbw_cmp_t 
        cbw_cmp_data;
    int res;

    data_ptr = xptRqt->m_data_p;

    if(xptRqt->m_smt_num != NLMDEV_SMT_0) {
        /* Error - Arad works only in NLMDEV_SMT_0 mode */
        *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }

    /* 
     * cbInstType - Not delivered. In Arad ROP The ctx buff type is in the LUT table instruction (not packet). 
     * xptRqt->m_ctx_addr - Not delivered. In Arad ROP the Context id is always 0x7ff8 and is in the LUT table instruction (not packet). 
     * xptRqt->m_opcode is actually the LTR - find the Arad ROP opcode relevant to this LTR
     */
    if ((cbw_cmp_data.ltr < 0) || (cbw_cmp_data.ltr >= ARAD_KBP_ROP_LTR_NUM_MAX)) {
        *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }
    cbw_cmp_data.type = NLM_ARAD_CB_INST_WRITE;
    cbw_cmp_data.ltr = ARAD_KBP_CPU_CTX_BUFF_WRITE_LTR;
    cbw_cmp_data.opcode = arad_kbp_frwd_ltr_db[cbw_cmp_data.ltr].opcode;
    cbw_cmp_data.cbw_data.data_len = xptRqt->m_data_len;

    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META("%s(): xptRqt->m_ctx_addr=%d, xptRqt->m_opcode[ARADXPT_OPCODE_1]=%d, opcode=%d, LTR=%d, xptRqt->m_data_len=%d"), 
FUNCTION_NAME(), xptRqt->m_ctx_addr, xptRqt->m_opcode[ARADXPT_OPCODE_1], cbw_cmp_data.opcode, cbw_cmp_data.ltr, xptRqt->m_data_len));
    
    /* Construct the CB Data based on datalen*/

    ARAD_KBP_ROP_REVERSE_DATA(cbw_cmp_data.cbw_data.data, data_ptr, cbw_cmp_data.cbw_data.data_len);

    res = arad_kbp_rop_cbw_cmp(
        aradxpt->unit,
        aradxpt->core,
        &cbw_cmp_data);
    if (SOC_SAND_KBP_ROP_FAILURE(res)) {
        *o_reason = NLMRSC_OPR_FAILURE;
        return;
    } 

    if(aradxpt->m_max_rqt_count != ARADXPT_MIN_RQT_CNT ||
        aradxpt->m_max_rslt_count != ARADXPT_MIN_RSLT_CNT)
    {
        MOVE_TO_FREE_LIST(&aradxpt->m_in_use_requests, aradrqt,
            &aradxpt->m_free_requests);
    }
}

/*******************************************************************************
 * Function : NlmAradXpt__GetAvailableResultCount
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 *
 * Summary:
 * NlmXpt__GetAvailableResultCount returns the available results in in_use
 * request queue.
 ******************************************************************************/
STATIC nlm_u32 NlmAradXpt__GetAvailableResultCount(
    NlmAradXpt *self
)
{
    NlmAradXpt *aradxpt;
    NlmAradXptRqt *rqt;
    nlm_u32       rqtcount = 0;

    if (!self) return 0;

    aradxpt = (NlmAradXpt *) self;

    if (!NlmAradXpt__GetInUseRequestCount(aradxpt))
        return 0;


    rqt = aradxpt->m_in_use_requests.head;

    while (rqt)
    {
        if (rqt->m_done) rqtcount++;
        rqt = rqt->next;
    }

    return rqtcount;

}


/*******************************************************************************
 * Function : NlmAradXpt__DiscardResult
 *
 * Parameters:
 *
 * NlmAradXpt *aradxpt         = Pointer to a NlmAradXpt object
 * NlmReasonCode *o_reason   = Location to save the reason code*
 * NlmXptRqt     *rqt        = Pointer to NlmXptRqt object
 *
 * Summary:
 * kbp_xpt_discard_result removes a request from in_use queue irrespective of
 * whether the request is executed or not.
 ******************************************************************************/
STATIC void NlmAradXpt__DiscardResult(
        void *self,
        NlmXptRqt *rqt,
        NlmReasonCode *o_reason
)
{
    NlmAradXpt *aradxpt;
    NlmAradXptRqt *aradrqt, *temp;

    if (!self || !rqt) return;

    aradxpt = (NlmAradXpt *) self;
    aradrqt = (NlmAradXptRqt *)rqt;

    if (!IS_ARADXPT(aradxpt)) 
    {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return;
    }
    if (!IS_ARADXPTRQT(aradrqt))
        goto not_allocated_by_me;

    if(aradxpt->m_max_rqt_count == ARADXPT_MIN_RQT_CNT &&
        aradxpt->m_max_rslt_count == ARADXPT_MIN_RSLT_CNT)
    {
        return;
    }

    if (!NlmAradXpt__GetInUseRequestCount(aradxpt))
        return;


    temp = aradxpt->m_in_use_requests.head;

    while (temp)
    {
        if (temp == aradrqt)
        {
            MOVE_TO_FREE_LIST(&aradxpt->m_in_use_requests, temp,
                        &aradxpt->m_free_requests);
            return;
            }
        temp = temp->next;
    }

    not_allocated_by_me:
        if (o_reason)
            *o_reason = NLMRSC_NORQST_AVBL;
}

/*******************************************************************************
 * Function : arad_kbp_xpt_init
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
 * kbp_aradxpt_initCreates the transport object (NlmXpt) and returns a
 * pointer to the object. It also initializes the virtual operation table
 * of NlmXpt object
 ******************************************************************************/

NlmXpt *
arad_kbp_xpt_init(
        int                     unit,
        nlm_u32                 core,
        NlmCmAllocator          *alloc,         /*General purpose memory allocator */
        nlm_u32                 max_rqt_count,  /*Max request count */
        nlm_u32                 max_rslt_count, /*Max result count*/
        nlm_u32                 chan_id
)
{
    NlmAradXpt *aradxpt = 0;
    NlmXpt    *xpt = 0;
    nlm_u32   count;
    static NlmXpt_operations xpt_operations;

    if (!alloc || (max_rqt_count < max_rslt_count))
        return NULL;

    aradxpt = (NlmAradXpt*)NlmCmAllocator__calloc(alloc, ARADXPT_NUM_ALLOC_CNT, sizeof(NlmAradXpt));
    if (!aradxpt) 
        return NULL;
    
    aradxpt->unit = unit;
    aradxpt->core = core;
    aradxpt->m_alloc_p = alloc;
    aradxpt->m_max_rqt_count = max_rqt_count;
    aradxpt->m_max_rslt_count = max_rslt_count;
    aradxpt->aradxpt_magic = NLM_ARADXPT_MAGIC_NUMBER;

    /*Now construct the base xpt structure */

    xpt = (NlmXpt*)NlmCmAllocator__calloc(alloc, ARADXPT_NUM_ALLOC_CNT, sizeof(NlmXpt));
    if (!xpt) 
    {
        NlmAradXpt__pvt_destroy(aradxpt);
        return NULL;
    }

    xpt_operations.kbp_xpt_get_result           = NlmAradXpt__GetResult;
    xpt_operations.kbp_xpt_discard_result       = NlmAradXpt__DiscardResult;
    xpt_operations.kbp_xpt_get_request          = NlmAradXpt__GetRequest;
    xpt_operations.kbp_xpt_reset_requests       = NlmAradXpt__ResetRequests;
    xpt_operations.kbp_xpt_service_requests     = NlmAradXpt__ServiceRequests;
    xpt_operations.kbp_xpt_reset_device         = NlmAradXpt__ResetDevices;
    xpt_operations.kbp_xpt_lock_config          = NlmAradXpt__LockConfig;
    xpt->m_derived = aradxpt;
    xpt->m_alloc   = alloc;
    xpt->m_oprs    = &xpt_operations;
    xpt->xpt_magic = XPT_MAGIC_NUMBER;

#if !defined NLM_MT_OLD && !defined NLM_MT
    xpt->m_xpt_chanel_id = chan_id;
#else
    xpt->m_xpt_chanel_id[0]  = chan_id;
#endif

    xpt->m_locked = NLMFALSE;

    aradxpt->xpt = xpt;

    /*Now initialize the free request queue */
    count = 0;
    while (count < max_rqt_count)
    {
        NlmAradXptRqt *rqt;
        rqt = (NlmAradXptRqt*)NlmCmAllocator__calloc(alloc, ARADXPT_NUM_ALLOC_CNT, sizeof(NlmAradXptRqt));
        if(rqt == NULL)
        {
            NlmAradXpt__pvt_destroy(aradxpt);
            return NULL;
        }

        NlmAradXpt__AddToRequestQue(&aradxpt->m_free_requests, rqt);
        count++;
    }

#if !defined NLM_MT_OLD && !defined NLM_MT
        aradxpt->m_single_rqt_p = (NlmAradXptRqt*)NlmCmAllocator__calloc(alloc, ARADXPT_NUM_ALLOC_CNT, sizeof(NlmAradXptRqt));
        if(aradxpt->m_single_rqt_p == NULL)
        {
            NlmAradXpt__pvt_destroy(aradxpt);
            return NULL;
        }
#else
        for(count = 0; count <  NLMNS_MT_MAXNUM_THREADS; count++)
        {
            aradxpt->m_single_rqt_p[count] = (NlmAradXptRqt*)NlmCmAllocator__calloc(alloc, ARADXPT_NUM_ALLOC_CNT, sizeof(NlmAradXptRqt));
            if(aradxpt->m_single_rqt_p[count] == NULL)
            {
                NlmAradXpt__pvt_destroy(aradxpt);
                return NULL;
            }
        }
#endif

    return xpt;

}

/*******************************************************************************
 * Function : NlmAradXpt__pvt_destroy
 *
 * Parameters:
 *
 * void *self = Pointer to a generic NlmXpt object
 * Summary:
 * NlmAradXpt__pvt_destroy frees the aradxpt object and does the necessary
 * cleanups
 ******************************************************************************/

STATIC void NlmAradXpt__pvt_destroy(
    void *self
)
{
    NlmAradXpt *aradxpt = (NlmAradXpt *) self;
    NlmXpt    *xpt;
    nlm_u32 count = 0;

    (void)count;

    if (!self) return;

    xpt = aradxpt->xpt;

#ifdef NLM_MT_XPT 
    /* Destroy the spin locks used */
     NlmCmMt__SpinDestroy(&aradxpt->m_xpt_spinLock,
                "NlmAradXpt_Kbp_SpinLock");
 #endif

    NlmAradXpt__FreeRequestQue(aradxpt->m_alloc_p, &aradxpt->m_in_use_requests);
    NlmAradXpt__FreeRequestQue(aradxpt->m_alloc_p, &aradxpt->m_free_requests);

#if !defined NLM_MT_OLD && !defined NLM_MT
    if(aradxpt->m_single_rqt_p)
        NlmCmAllocator__free(aradxpt->m_alloc_p, aradxpt->m_single_rqt_p);
#else
    for(count = 0; count <  NLMNS_MT_MAXNUM_THREADS; count++)
    {
        if(aradxpt->m_single_rqt_p[count])
            NlmCmAllocator__free(aradxpt->m_alloc_p, aradxpt->m_single_rqt_p[count]);
    }
#endif

    /* free xpt first, as the aradxpt hold the xpt pointer */
    if (xpt)
        NlmCmAllocator__free(xpt->m_alloc, xpt);

    if(aradxpt)
    {
        NlmCmAllocator__free(aradxpt->m_alloc_p, aradxpt);
    }
}


/*******************************************************************************
 * Function : NlmAradXpt__TakeOutRequest
 *
 * Parameters:
 *
 * NlmXptRqtQue *rque    = Pointer to a request queue
 * NlmAradXptRqt *rqt     = Pointer to the request to be removed
 *
 * Summary:
 * NlmAradXpt__TakeOutRequest just removes a NlmAradXptRqt structure form a Request queue,
 * but it doesn't free the request. If the request is not in the queue, then
 * effectively the routine does nothing.
 ******************************************************************************/
STATIC void NlmAradXpt__TakeOutRequest(
    NlmXptRqtQue *rque,
    NlmAradXptRqt *rqt
)
{
    NlmAradXptRqt *father = NULL, *cur, *son = NULL;

    if (!rque || !rqt) 
        return;
    
    if (rque->count == ARADXPT_EMPTY_RTQ || !rque->head) 
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
 * Function : NlmAradXpt__AddToRequestQue
 *
 * Parameters:
 *
 * NlmXptRqtQue *rque    = Pointer to a request queue
 * NlmAradXptRqt *rqt     = Pointer to the request to be added
 *
 * Summary:
 * NlmAradXpt__AddToRequestQue inserts a NlmAradXptRqt structure at the end of the que
 ******************************************************************************/

STATIC void NlmAradXpt__AddToRequestQue(
    NlmXptRqtQue *rque,
    NlmAradXptRqt *rqt)
{
    if (!rque || !rqt) return;

    rqt->next = NULL;

    if (rque->count == ARADXPT_EMPTY_RTQ) 
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
 * Function : NlmAradXpt__FreeRequestQue
 *
 * Parameters:
 *
 * NlmCmAllocator *alloc = Pointer to General purpose memory allocator
 * NlmXptRqtQue *rq      = Pointer to a request queue
 *
 * Summary:
 * NlmAradXpt__FreeRequestQue decallocates a request queue.
 ******************************************************************************/

STATIC void NlmAradXpt__FreeRequestQue(
        NlmCmAllocator *alloc,
        NlmXptRqtQue *rq)
{
    nlm_u32 rqtcnt;
    NlmAradXptRqt *rqt, *rqtnext;

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

    rq->count = ARADXPT_EMPTY_RTQ;
    rq->head = rq->tail = NULL;

}

/*******************************************************************************
 * Function : arad_kbp_xpt_destroy
 *
 * Parameters:
 * NlmAradXpt *self         = Pointer to an NlmAradXpt object
 *
 * Summary:
 * kbp_aradxpt_destroy frees all the memory associated with the xpt structure
 ******************************************************************************/

void arad_kbp_xpt_destroy(
    NlmXpt* self
)
{
    if (!self) return;

    if (self->m_derived)
        NlmAradXpt__pvt_destroy(self->m_derived);
    else
        NlmCmAllocator__free(self->m_alloc, self);
}


/*******************************************************************************
 * Function : NlmAradXpt__ResetDevices
 *
 * Parameters:
 * NlmAradXpt *self         = Pointer to an NlmAradXpt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__ResetDevices reinitializes the device and all the requests are
 * reset too.
 ******************************************************************************/

STATIC NlmErrNum_t NlmAradXpt__ResetDevices(
    void *self,
    NlmReasonCode *o_reason
)
{
    NlmAradXpt *aradxpt = (NlmAradXpt *)self;
    NlmReasonCode dummyReasonCode;

    if(o_reason == NULL)
        o_reason = &dummyReasonCode;

    if (!IS_ARADXPT(aradxpt)) 
    {   
        *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    NlmAradXpt__ResetRequests(aradxpt, o_reason);

    return NLMERR_OK;
}

/*******************************************************************************
 * Function : NlmAradXpt__LockConfig
 *
 * Parameters:
 * void *self         = Pointer to an NlmXpt object
 * nlm_u32            = Number of devieces
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmAradXpt__LockConfig finalizes the various aspects of the aradulated device.
 * Only after NlmAradXpt__LockConfig is called, it will start accepting requests
 ******************************************************************************/
STATIC NlmErrNum_t NlmAradXpt__LockConfig(
    void *self,
    nlm_u32 num_devices,
    NlmReasonCode *o_reason
)
{

    NlmAradXpt *aradxpt = (NlmAradXpt *)self;
    NlmReasonCode dummyReasonCode;
    NlmErrNum_t errNum = NLMERR_OK;

    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META("%s(): XPT lock config!!!\n"), FUNCTION_NAME()));

    if(o_reason == NULL)
        o_reason = &dummyReasonCode;

    if (!IS_ARADXPT(aradxpt)) {   
        *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    return errNum;
}

/* Wraper functions to support old APIs */

void arad_Nlm_dXpt_destroy(
    NlmXpt* self
)
{
    arad_kbp_xpt_destroy(self);   
}

NlmXpt *
arad_Nlm_Xpt_Create(
        int                     unit,
        uint32                  core,
        NlmCmAllocator          *alloc,         /*General purpose memory allocator */
        nlm_u32                 max_rqt_count,  /*Max request count */
        nlm_u32                 max_rslt_count, /*Max result count*/
        nlm_u32                 chan_id
)
{
    return arad_kbp_xpt_init(unit, core, alloc,max_rqt_count,max_rslt_count,chan_id);
}

void fill_rop_compare_result(uint8_t *cmp_data, struct kbp_search_result *result)
{
    uint32_t index = 0, i, j = 0;
    uint32_t ad_size = 0;
    uint8_t sts = cmp_data[index++];

    for (i = 0; i < KBP_INSTRUCTION_MAX_RESULTS; i++) {
        j = 0;

        if ((sts & (1 << (KBP_INSTRUCTION_MAX_RESULTS - (i + 1)))))
            result->hit_or_miss[i] = KBP_HIT;

        if (result->resp_type[i] == 0) {
            result->hit_index[i] = (cmp_data[index++] << 16);
            result->hit_index[i] |= (cmp_data[index++] << 8);
            result->hit_index[i] |= cmp_data[index++];
        } else {
            ad_size = result->resp_type[i];
            if (ad_size > KBP_INDEX_AND_256B_AD)
                ad_size = ad_size - KBP_INDEX_AND_256B_AD;
            if (ad_size == KBP_INDEX_AND_256B_AD)
                ad_size++;
            while (ad_size) {
                result->assoc_data[i][j] = cmp_data[index];
                index++;
                j++;
                ad_size--;
            }
        }
    }

}

void arad_print_search_result(struct kbp_search_result *res)
{
    int i, j;
    uint32_t ad_size_8 = 0;

    for (i = 0 ; i < KBP_INSTRUCTION_MAX_RESULTS; i++) {
        printf(" Rslt%d: ", i);
        printf(" Vld:%x ", res->result_valid[i]);
        printf(" Hit:%x ", res->hit_or_miss[i]);
        printf(" Idx:%06x ", res->hit_index[i]);
        printf(" Bank:0 ");
        ad_size_8 = res->resp_type[i];
        printf(" RspFt:%02x AdIdx:", ad_size_8);
        if (ad_size_8 == 0x1F)
            ad_size_8++;
        if (res->resp_type[i] != 0) {
            for (j = 0; j < ad_size_8; j++)
                printf("%02x", res->assoc_data[i][j]);
        } else {
            printf("%x", res->assoc_data[i][0]);
        }
        printf("\n");
    }
}


kbp_status arad_op_search(void *hdl, uint32_t ltr,
                          uint32_t ctx, const uint8_t *key, uint32_t key_len,
                          struct kbp_search_result *result)
{
    NlmAradXpt *aradxpt;
    arad_kbp_rop_cbw_cmp_t cbw_cmp_data;
    int res;
    int unit = BSL_UNIT_UNKNOWN;
    const uint8_t *data_ptr;

    if (!(hdl) || !(key) || !(result))
        return NLMRSC_INVALID_PARAM;

    aradxpt = hdl;
    if (aradxpt == NULL) {
        LOG_ERROR(BSL_LS_SOC_TCAM, (BSL_META_U(unit, "%s(): '(aradxpt == NULL))'.\n"), FUNCTION_NAME()));
        return NLMRSC_INVALID_XPT_PTR;
    }

    memset(&cbw_cmp_data, 0x0, sizeof(arad_kbp_rop_cbw_cmp_t));
    data_ptr = key;

    cbw_cmp_data.type = NLM_ARAD_CB_INST_CMP1;
    cbw_cmp_data.ltr = ltr;
    cbw_cmp_data.opcode = ltr + 1;
    cbw_cmp_data.cbw_data.data_len = key_len;

    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META("%s(): ctx=%d, opcode=%d, LTR=%d, data_len=%d"),
                 FUNCTION_NAME(), ctx, cbw_cmp_data.opcode, cbw_cmp_data.ltr, key_len));

    /* Construct the CB Data based on datalen*/
    ARAD_KBP_ROP_REVERSE_DATA(data_ptr, cbw_cmp_data.cbw_data.data, cbw_cmp_data.cbw_data.data_len);

    if (0) {
        int i;
        for (i = 0; i < 80; i++) {
            printf("0x%x, ", cbw_cmp_data.cbw_data.data[i]);
        }
        printf("\n");
    }

    res = arad_kbp_rop_cbw_cmp(
              aradxpt->unit,
              aradxpt->core,
              &cbw_cmp_data);

    if (SOC_SAND_KBP_ROP_FAILURE(res)) {
        return NLMRSC_OPR_FAILURE;
    }

    if (0) {
        int i;
        for (i = 0; i < 80; i++) {
            printf("0x%x, ", cbw_cmp_data.result.data_raw[i]);
        }
        printf("\n");
    }
    fill_rop_compare_result(cbw_cmp_data.result.data_raw, result);
    /* arad_print_search_result(result);*/

    return KBP_OK;
}

void *
arad_kbp_op_xpt_init(
    int                     unit,
    nlm_u32                 core,
    struct kbp_allocator    *alloc
)
{
    NlmAradXpt *aradxpt = 0;

    if (alloc == NULL)
        return NULL;

    aradxpt = (NlmAradXpt *)alloc->xcalloc(alloc->cookie, 1, sizeof(NlmAradXpt));
    if (!aradxpt)
        return NULL;

    aradxpt->unit = unit;
    aradxpt->core = core;
    /* aradxpt->m_alloc_p = alloc;*/
    aradxpt->m_max_rqt_count = 0;
    aradxpt->m_max_rslt_count = 0;
    aradxpt->xpt = 0;
    aradxpt->aradxpt_magic = NLM_ARADXPT_MAGIC_NUMBER;

    return aradxpt;
}

void arad_kbp_op_xpt_destroy(void *self)
{
    NlmAradXpt__pvt_destroy(self);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* #if defined(BCM_88650_A0) && defined(INCLUDE_KBP) */
