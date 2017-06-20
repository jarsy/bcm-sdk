/*
 * $Id: nlmblackholexpt.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include <nlmxpt.h>
#include <nlmblackholexpt.h>


#define NLMCM_ALLOC_INST_CNT    (1)
#define NLMCM_MIN_RQT_CNT       (1)
#define NLMCM_MIN_RSLT_CNT      (1)
#define NLMCM_EMPTY_RQTS        (0)
#define NLMCM_MAX_RQT_CNT       (0x0FFFFFFFF)
/*
 * nlmblackholexpt module implementes the operations defined by base xpt module to
 * work with the cmodel.
 * If the max_rqt_count is 1 and max_rslt_count is 1, then we don't maintain a
 * queue for the reqests and results but instead use m_single_rqt_p. This helps
 * avoid the queuing overhead
*/


/*
 * NlmBlackHoleXptRqtQue is a list of requests
 */
typedef struct NlmBlackHoleRqt_t {
        NlmXptRqt m_nlm_rqt;
        NlmRequestId  m_rqt_id;          /* Request id assigned by the xpt */
        struct NlmBlackHoleRqt_t *next;
        nlm_u32 blackholexpt_rqt_magic;
}NlmBlackHoleRqt;

typedef struct NlmBlackHoleXptRqtQue_t 
{
        nlm_u32 count;  /*how many results on the que */
        NlmBlackHoleRqt *head;   /*head of the queue*/
        NlmBlackHoleRqt *tail;   /*tail of the queue*/
} NlmBlackHoleXptRqtQue;



typedef struct NlmBlackHole_t 
{
    NlmXpt*                xpt;    /* Pointer to virtual xpt object */
    NlmCmAllocator*        m_alloc_p ; /* General purpose memory Allocator */
    nlm_u32                m_max_rqt_count; /*Max request count */ 
    nlm_u32                m_max_rslt_count; /* Max rslt cnt    */
    NlmBlackHoleXptRqtQue  m_in_use_requests; /* Request being used  */
    NlmBlackHoleXptRqtQue  m_free_requests; /* Free requests */

#if defined NLM_MT_OLD || defined NLM_MT
    NlmBlackHoleRqt*       m_single_rqt_p[NLMNS_MT_MAXNUM_THREADS]; /* Used for storing multiple requests in flat xpt without queuing */
#else
    NlmBlackHoleRqt*       m_single_rqt_p; /* Used for storing a single request in flat xpt without queuing */
#endif

    NlmRequestId           m_next_rqt_id;
    nlm_u32                blackholexpt_magic;
}NlmBlackHole;

/*
 * MACRO definitions 
 */

#define NlmBlackHole__GetInUseRequestCount(self) ((self)->m_in_use_requests.count)
#define NlmBlackHole__GetFreeRequestCount(self) ((self)->m_free_requests.count)
#define NlmBlackHole__GetXpt(self)  (self->xpt) 

/*
 * The magic numbers are used to verify if a particular object is of the same
 * object type as the one we are looking for.
 */

#define BLACKHOLE_XPT_MAGIC_NUMBER           (0x22345)
#define BLACKHOLE_XPT_RQT_MAGIC_NUMBER       (0x22346)

#define IS_BLACKHOLE_XPT(x)     ((x) && ((x)->blackholexpt_magic == BLACKHOLE_XPT_MAGIC_NUMBER))
#define IS_BLACKHOLE_XPTRQT(x)  ((x) && ((x)->blackholexpt_rqt_magic == BLACKHOLE_XPT_RQT_MAGIC_NUMBER))

#define MOVE_TO_FREE_LIST(inuse, element, freelist) \
        {\
                NlmBlackHole__TakeOutRequest(inuse, element);\
                NlmCm__memset(element, 0, sizeof(NlmBlackHoleRqt));\
                NlmBlackHole__AddToRequestQue(freelist, element);\
        }

/*
 * Functiondeclarations 
 */

static void NlmBlackHole__FreeRequestQue (
                NlmCmAllocator *alloc, 
                NlmBlackHoleXptRqtQue *rq);

static void NlmBlackHole__AddToRequestQue(
                NlmBlackHoleXptRqtQue *rque,
                NlmBlackHoleRqt *rqt);

static void NlmBlackHole__pvt_destroy(
                void *self
                );

static void NlmBlackHole__TakeOutRequest(
                NlmBlackHoleXptRqtQue *rque,
                NlmBlackHoleRqt *rqt
                );

static NlmBlackHoleRqt* NlmBlackHole__RemoveLastRequest(
           NlmBlackHoleXptRqtQue *rque
                );

static void NlmBlackHole__AssignRequestId(
                NlmBlackHole *self,
                NlmBlackHoleRqt *blkrqst);

static NlmBlackHoleRqt *NlmBlackHole__NextToExecute(
                NlmBlackHole *self
                );

static void* NlmBlackHole__GetFirstAvlResult(
                void *self,
                NlmReasonCode *o_reason
                );

static void*  NlmBlackHole__GetResult(
                void *self,
                NlmXptRqt *rqt,
                NlmReasonCode *o_reason
                );

static NlmRequestId *
NlmBlackHole__GetRequestId (
                void *xpt,
                NlmXptRqt *rqt,
                NlmReasonCode *o_reason
                );

static NlmErrNum_t NlmBlackHole__ResetDevice(
                void *self,
                NlmReasonCode *o_reason
                );

static NlmErrNum_t NlmBlackHole__LockConfig(
                void *self,
                nlm_u32 num_devies,
                NlmReasonCode *o_reason
                );

static void* NlmBlackHole__GetFirstAvlResult(
                void *self,
                NlmReasonCode *o_reason
                )
{
    (void ) self;
    (void ) o_reason;
    return NULL;
}

/*
 * NlmBlackHole__GetResult sends a filled in result structure to upper layer of
 * the s/w. It will return NULL if (a) the NlmXptRqt onbject is not in the 
 * in_use request list, or, (b) the request is not completed. 
 * If the NlmXptRqt object passed to the function is NULL, then the result for the
 * first request in the in_use request list is returned.     
 * If NlmBlackHole__GetResult is returning a non-null result,then the corresponding
 * request object will be moved to free request list. 
 *
 * side effect:   The request corresponding to the result will be moved to free
 * list.
 * Return value:  Pointer to the result object
 */

static void*  NlmBlackHole__GetResult(
    void*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode *o_reason
    )
{
        (void ) self;
        (void ) rqt;
        (void ) o_reason;

        return NULL;
}

static NlmRequestId *
NlmBlackHole__GetRequestId (
                void *self,
                NlmXptRqt *rqt,
                NlmReasonCode *o_reason
                )
{
       NlmBlackHoleRqt *blkrqst;

        blkrqst = (NlmBlackHoleRqt *) rqt;
        (void)self;

        if (!IS_BLACKHOLE_XPTRQT(blkrqst))
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
                return NULL;
        }
        return &blkrqst->m_rqt_id;        
}

/*
 * NlmBlackHole__GetResultForRqtId sends a filled in result structure to upper layer 
 * of the s/w. It will return NULL if (a) the rqtid passed is Null ,or,(b) there
 * is no request with that rqtid available in the queue, or , (c) the request
 * with that request id is not serviced. 
 * If it returns a non-null result, the corresponding request object is moved to
 * free list.
 */

static void* NlmBlackHole__GetResultForRqtId(
    void*              self,   
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode       *o_reason
    )        
{
        (void)o_reason;
        (void)rqtid;
        (void)self;
        
        return NULL;
}

/*
 * NlmBlackHole__GetXptRequest returns a request object to the caller. Null is
 * returned if there is no more free requests. The returned request is moved to
 * in_use list. 
 *
 * side effect: the in_use and free request lists may get modfified.
 * Return vlaue: Pointer to a request object, 
 */

static NlmXptRqt* NlmBlackHole__GetRequest(
    void*     self,
    NlmReasonCode *o_reason
    ) 
{
        NlmBlackHole *blackholexpt = 0;
        NlmBlackHoleRqt *blkrqst = 0;

        if (!self || !IS_BLACKHOLE_XPT((NlmBlackHole *)self))
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;

                return NULL;
        }

        blackholexpt = (NlmBlackHole *)self;

        if(blackholexpt->m_max_rqt_count == NLMCM_MIN_RQT_CNT && 
            blackholexpt->m_max_rslt_count == NLMCM_MIN_RSLT_CNT)
        {
#if defined NLM_MT_OLD || defined NLM_MT
            blkrqst = blackholexpt->m_single_rqt_p[threadID];
#else
            blkrqst = blackholexpt->m_single_rqt_p;
#endif


            blkrqst->blackholexpt_rqt_magic = BLACKHOLE_XPT_RQT_MAGIC_NUMBER;

            return (NlmXptRqt *) blkrqst;

        }
        
        
        blkrqst = NlmBlackHole__RemoveLastRequest(&blackholexpt->m_free_requests);
            
        if (!blkrqst) 
        {
                if (o_reason)
                        *o_reason = NLMRSC_NOFREE_RQST;
                return NULL;
        }

        NlmCm__memset(blkrqst, 0, sizeof(NlmBlackHoleRqt));

        NlmBlackHole__AssignRequestId(blackholexpt, blkrqst);

        blkrqst->blackholexpt_rqt_magic = BLACKHOLE_XPT_RQT_MAGIC_NUMBER;

        NlmBlackHole__AddToRequestQue(&blackholexpt->m_in_use_requests, blkrqst);
        
        return (NlmXptRqt *) blkrqst;

}


/*
 *  NlmBlackHole__RemoveLastRequest: Takes out the last element of a rquest queue
 *  and returns that element.
 */

static NlmBlackHoleRqt* NlmBlackHole__RemoveLastRequest(
           NlmBlackHoleXptRqtQue *rque
      )
{
        NlmBlackHoleRqt *blkrqst;
        
        if (rque->count == NLMCM_EMPTY_RQTS) 
            return NULL;

        blkrqst = rque->head;

        NlmBlackHole__TakeOutRequest(rque, blkrqst);

        return blkrqst;
}


static void NlmBlackHole__AssignRequestId(
                NlmBlackHole *self,
                NlmBlackHoleRqt *blkrqst)
{
        if (!self || !blkrqst) return;

        NlmCm__memcpy(&blkrqst->m_rqt_id, &self->m_next_rqt_id, 
                        sizeof(NlmRequestId)); 
        
        if (self->m_next_rqt_id.m_count < (NLMCM_MAX_RQT_CNT - 0x1))
        {
               self->m_next_rqt_id.m_count++;
        }
        else
        {
                self->m_next_rqt_id.m_count = 0;
                self->m_next_rqt_id.m_giga_count++;
        } 
}


/*
 * NlmBlackHole__ResetRequests clears all the Requests, moves all the requests to the
 * free request list. The requests are just moved to the free_list irrespective
 * of they are executed or not. 
 *
 * siee effect: the in_use requests will be empty
 * Return value: NONE
 */
static void NlmBlackHole__ResetRequests(
    void*    self,
    NlmReasonCode *o_reason
    ) 
{
        NlmBlackHole *blackholexpt;
        blackholexpt = (NlmBlackHole *) self;
        (void)o_reason;

        if (!IS_BLACKHOLE_XPT(blackholexpt)) return;

        if(blackholexpt->m_max_rqt_count == NLMCM_MIN_RQT_CNT &&
            blackholexpt->m_max_rslt_count == NLMCM_MIN_RSLT_CNT)
        {
            return;
        }
        
        while (NlmBlackHole__GetInUseRequestCount(blackholexpt))
        {
                NlmBlackHoleRqt *rqt;
                rqt = blackholexpt->m_in_use_requests.head;
                NlmBlackHole__TakeOutRequest(&blackholexpt->m_in_use_requests, rqt); 
                NlmCm__memset(rqt, 0, sizeof(NlmBlackHole));
                NlmBlackHole__AddToRequestQue(&blackholexpt->m_free_requests, rqt);
        }

}



static NlmBlackHoleRqt *NlmBlackHole__NextToExecute(
                NlmBlackHole *self
                )
{
        if (!IS_BLACKHOLE_XPT(self) || 
               !NlmBlackHole__GetInUseRequestCount(self)) return NULL;

        return self->m_in_use_requests.head;
}


/*
 *  NlmBlackHole__ServiceRequests  takes  the requests from the in_use request
 *  and simply moved them to free_request_queue
 *
 *  side effect: Request queues are modified.
 *  Return Value: NONE
 */
static void
NlmBlackHole__ServiceRequests(
    void*            self,            /* transport interface to flush */
    NlmReasonCode    *o_reason
    ) 
{
       NlmBlackHole *blackholexpt;
       NlmBlackHoleRqt *blkrqst;

       if (!self) return; 

       blackholexpt = (NlmBlackHole *)self;

       if (!IS_BLACKHOLE_XPT(blackholexpt))
       {
               if (o_reason)
                       *o_reason = NLMRSC_INVALID_PARAM;
               return;
       }
       
       if (o_reason)
               *o_reason = NLMRSC_REASON_OK;

       if(blackholexpt->m_max_rqt_count == NLMCM_MIN_RQT_CNT &&
           blackholexpt->m_max_rslt_count == NLMCM_MIN_RSLT_CNT)
       {
           return;
       }

       if (!NlmBlackHole__GetInUseRequestCount(blackholexpt)) return;


       while (NULL != (blkrqst = NlmBlackHole__NextToExecute(blackholexpt))) 
       {        
            MOVE_TO_FREE_LIST(&blackholexpt->m_in_use_requests, blkrqst, 
                &blackholexpt->m_free_requests);
       }
}



static void NlmBlackHole__DiscardResult(
                void *self,
                NlmXptRqt *rqt,
                NlmReasonCode *o_reason
                )
{
        NlmBlackHole *blackholexpt;
        NlmBlackHoleRqt *blkrqst, *temp;

        if (!self || !rqt) return;

        blackholexpt = (NlmBlackHole *) self;
        blkrqst = (NlmBlackHoleRqt *)rqt;

        if (!IS_BLACKHOLE_XPT(blackholexpt))
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
                return;
        }

        if(blackholexpt->m_max_rqt_count == NLMCM_MIN_RQT_CNT && 
            blackholexpt->m_max_rslt_count == NLMCM_MIN_RSLT_CNT)
        {
            return;
        }

        if (!IS_BLACKHOLE_XPTRQT(blkrqst))
                goto not_allocated_by_me;

        if (!NlmBlackHole__GetInUseRequestCount(blackholexpt))
                return;

        
        temp = blackholexpt->m_in_use_requests.head;

        while (temp)
        {
                if (temp == blkrqst)
                {
                        MOVE_TO_FREE_LIST(&blackholexpt->m_in_use_requests, temp,
                                        &blackholexpt->m_free_requests);
                        return;
                }
        }

not_allocated_by_me:
        if (o_reason)
                *o_reason = NLMRSC_NORQST_AVBL;
}

/*
 * NlmBlackHole__CreateXpt Creates the transport object 
 */

NlmXpt *
kbp_blackhole_init(
    NlmCmAllocator       *alloc,        /*General purpose memory allocator */
    nlm_u32              max_rqt_count, /*Max request count */
    nlm_u32              max_rslt_count /*Max result count*/          
    )
{
        NlmBlackHole *blackholexpt = 0;
        NlmXpt    *xpt = 0;
        nlm_u32   count;
        static NlmXpt_operations xpt_operations;

        if (!alloc || max_rqt_count < max_rslt_count)
                return NULL;

        blackholexpt = (NlmBlackHole*)NlmCmAllocator__calloc(alloc, NLMCM_ALLOC_INST_CNT, sizeof(NlmBlackHole)); 
        if (!blackholexpt) 
            return NULL;
        
        blackholexpt->m_alloc_p = alloc;
        blackholexpt->m_max_rqt_count = max_rqt_count;
        blackholexpt->m_max_rslt_count = max_rslt_count;
        blackholexpt->blackholexpt_magic = BLACKHOLE_XPT_MAGIC_NUMBER;

       /*Now construct the base xpt structure */
        
       xpt = (NlmXpt*)NlmCmAllocator__calloc(alloc, NLMCM_ALLOC_INST_CNT, sizeof(NlmXpt));
       if (!xpt) 
       {
               NlmBlackHole__pvt_destroy(blackholexpt);
               return NULL;
       }

       xpt_operations.kbp_xpt_get_result           = NlmBlackHole__GetResult;
       xpt_operations.kbp_xpt_discard_result       = NlmBlackHole__DiscardResult;
       xpt_operations.kbp_xpt_get_result_for_rqtid = NlmBlackHole__GetResultForRqtId;
       xpt_operations.kbp_xpt_get_request          = NlmBlackHole__GetRequest;
       xpt_operations.kbp_xpt_reset_requests       = NlmBlackHole__ResetRequests;
       xpt_operations.kbp_xpt_get_request_id       = NlmBlackHole__GetRequestId;
       xpt_operations.kbp_xpt_service_requests     = NlmBlackHole__ServiceRequests;
       xpt_operations.kbp_xpt_get_first_result     = NlmBlackHole__GetFirstAvlResult; 
       xpt_operations.kbp_xpt_reset_device         = NlmBlackHole__ResetDevice;
       xpt_operations.kbp_xpt_lock_config          = NlmBlackHole__LockConfig;
       xpt->m_derived = blackholexpt;
       xpt->m_alloc   = alloc;
       xpt->m_oprs    = &xpt_operations;
       xpt->xpt_magic = XPT_MAGIC_NUMBER;
       xpt->m_locked  = NLMFALSE;

       blackholexpt->xpt = xpt;

#if defined NLM_MT_OLD || defined NLM_MT
      {
          nlm_u32 i;

           for(i = 0; i < NLMNS_MT_MAXNUM_THREADS; i++)
           {
               blackholexpt->m_single_rqt_p[i] = (NlmBlackHoleRqt*)NlmCmAllocator__calloc(alloc, NLMCM_ALLOC_INST_CNT, sizeof(NlmBlackHoleRqt));
               if(blackholexpt->m_single_rqt_p[i] == NULL)
               {
                   NlmBlackHole__pvt_destroy(blackholexpt);
                   return NULL;
               }
           }
      }
#else
      blackholexpt->m_single_rqt_p = (NlmBlackHoleRqt*)NlmCmAllocator__calloc(alloc, NLMCM_ALLOC_INST_CNT, sizeof(NlmBlackHoleRqt)); 
      if(blackholexpt->m_single_rqt_p == NULL)
      {
          NlmBlackHole__pvt_destroy(blackholexpt);
          return NULL;
      }
#endif

       /*Now initialize the free request queue */
       count = 0;
       while (count < max_rqt_count)
       {
           NlmBlackHoleRqt *rqt;
           rqt = (NlmBlackHoleRqt*)NlmCmAllocator__calloc(alloc, NLMCM_ALLOC_INST_CNT, sizeof(NlmBlackHoleRqt));
           if(rqt == NULL)
           {
               NlmBlackHole__pvt_destroy(blackholexpt);
               return NULL;
           }

           NlmBlackHole__AddToRequestQue(&blackholexpt->m_free_requests, rqt);               
           count++;
       }
       return xpt;

}

/*
 * NlmBlackHole__pvt_destroy frees the blackholexpt object and does the necessary
 * cleanups
 */

static void NlmBlackHole__pvt_destroy(
                void *self
                )
{
        NlmBlackHole *blackholexpt;
        NlmXpt    *xpt;   
        
        if (!self) return;

        blackholexpt = (NlmBlackHole *) self;
        xpt = blackholexpt->xpt;

        NlmBlackHole__FreeRequestQue(blackholexpt->m_alloc_p, &blackholexpt->m_in_use_requests);
        NlmBlackHole__FreeRequestQue(blackholexpt->m_alloc_p, &blackholexpt->m_free_requests);

#if defined NLM_MT_OLD || defined NLM_MT
   {
       nlm_u32 i;
       
       for(i = 0; i < NLMNS_MT_MAXNUM_THREADS; i++)
       {
           if(blackholexpt->m_single_rqt_p[i])
               NlmCmAllocator__free(blackholexpt->m_alloc_p, blackholexpt->m_single_rqt_p[i]);
       }
   }
#else
    if(blackholexpt->m_single_rqt_p)
        NlmCmAllocator__free(blackholexpt->m_alloc_p, blackholexpt->m_single_rqt_p);
#endif
    
    /* free xpt first, as the bloackholexpt hold the xpt pointer */
    if (xpt)
        NlmCmAllocator__free(xpt->m_alloc, xpt);

    if(blackholexpt)
        NlmCmAllocator__free(blackholexpt->m_alloc_p, blackholexpt);
}

/*
 * NlmBlackHole__TakeOutRequest just removes a NlmBlackHoleRqt structure form a Request queue,
 * but it doesn't free the request. If the request is not in the queue, then
 * effectively the routine does nothing.
 */


static void NlmBlackHole__TakeOutRequest(
                NlmBlackHoleXptRqtQue *rque,
                NlmBlackHoleRqt *rqt
                )
{
        NlmBlackHoleRqt *father = NULL, *cur, *son = NULL;

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


/*
 * NlmBlackHole__AddToRequestQue inserts a NlmBlackHoleRqt structure at the end of the que 
*/
static void NlmBlackHole__AddToRequestQue(
                NlmBlackHoleXptRqtQue *rque,
                NlmBlackHoleRqt *rqt)
{
        if (!rque || !rqt) return;
        
        rqt->next = NULL;

        if (rque->count == NLMCM_EMPTY_RQTS)
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

/*
 *  NlmBlackHole__FreeRequestQue decallocates a request queue.
 */

static void NlmBlackHole__FreeRequestQue (NlmCmAllocator *alloc, NlmBlackHoleXptRqtQue *rq)
{
        nlm_u32 rqtcnt;
        NlmBlackHoleRqt *rqt, *rqtnext;
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

        rq->count = NLMCM_EMPTY_RQTS;
        rq->head = rq->tail = NULL;

}

/*
 * kbp_blackhole_destroy frees all the memory associated with the xpt structure 
 */
void kbp_blackhole_destroy(
    NlmXpt* self
) 
{
        if (!self) return;
        
        if (self->m_derived)
                NlmBlackHole__pvt_destroy(self->m_derived);
        else
                NlmCmAllocator__free(self->m_alloc, self); 
}

static NlmErrNum_t NlmBlackHole__ResetDevice(
     void *self,
     NlmReasonCode *o_reason
)
{
        (void ) self;
        (void ) o_reason;
        return NLMERR_OK;
}

static NlmErrNum_t NlmBlackHole__LockConfig(
        void *self,
        nlm_u32 num_deviecs,
        NlmReasonCode *o_reason
)
{
        
        NlmBlackHole *blackholexpt;
        (void)num_deviecs;
        
        blackholexpt = (NlmBlackHole *)self;
        
        if (!IS_BLACKHOLE_XPT(blackholexpt))
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM; 
                return NLMERR_FAIL;
        }        
       return NLMERR_OK;
}


void NlmBlackHole__destroy(
    NlmXpt* self
) 
{
    kbp_blackhole_destroy(self);
}

NlmXpt *
NlmBlackHole__Create(
    NlmCmAllocator       *alloc,        /*General purpose memory allocator */
    nlm_u32              max_rqt_count, /*Max request count */
    nlm_u32              max_rslt_count /*Max result count*/          
    )
{
    return kbp_blackhole_init(alloc,max_rqt_count,max_rslt_count);
}


#ifndef NLMPLATFORM_BCM

void bcm_kbp_blackhole_destroy(
    NlmXpt* self
) 
{
    kbp_blackhole_destroy(self);
}

NlmXpt *
bcm_kbp_blackhole_init(
    NlmCmAllocator       *alloc,        /*General purpose memory allocator */
    nlm_u32              max_rqt_count, /*Max request count */
    nlm_u32              max_rslt_count /*Max result count*/          
    )
{
    return kbp_blackhole_init(alloc,max_rqt_count,max_rslt_count);
}

#endif

