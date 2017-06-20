/*
 * $Id: nlmxpt.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
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
 *  kbp_xpt_get_request routine to get a pointer to a request object which needs
 *  to be initialized  appropriately by the caller. Routine
 *  kbp_xpt_service_requests is called to trigger execution of the requests
 *  already in the queue.
 *  A "request" is roughly a structure that has a request code (to indicate
 *  read/write/compare operation, the data for the request and address on device
 *  etc.
 *  Appropriate device specific routines are called by xpt to execute the
 *  requests.
 */
#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmallocator.h>
#include <nlmerrorcodes.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#endif

#if defined NLM_MT_OLD || defined NLM_MT
#ifndef NLMPLATFORM_BCM
#include "nlmcmmt.h"
#else
#include <soc/kbp/common/nlmcmmt.h>
#endif
#endif


#define     MAX_COMP_RESULT         (4)
#define     MAX_READ_SIZE           (80)
#define     INVALID_VAL             (-1)
#define     MAX_DATA_LEN_BYTES      (80)
#define     XPT_MAX_MESSAGE_SIZE    (255)
#define     INVALID_CTX_ADDR        (0xffff)
#define     XPT_MAGIC_NUMBER        (0x12344)

/*
 * NlmXptRq structure describes a request for read/write/compare operations. This
 * m_results should hold the address for a  valid Result structure and this
 * address should be initialized by caller only.
 */

#define     NLM_XPT_OPCODE_CNT   (2)

typedef struct NlmXptRqt_t
{
    /* Definitions used in no-SMT/2-SMT modes (for SMT-0 in 2-SMT mode) */
    nlm_u8  m_opcode[NLM_XPT_OPCODE_CNT];
    nlm_u32 m_data_len;
    nlm_u16 m_ctx_addr;
    
#ifdef NLM_NO_MEMCPY_IN_XPT
    nlm_u32 m_address;
    nlm_u8* m_data_p;
    nlm_u8* m_mask_p;
#else
    nlm_u8  m_data[MAX_DATA_LEN_BYTES];
#endif

    /* Definitions used for SMT-1 in 2-SMT mode */
    nlm_u8  m_opcode1[NLM_XPT_OPCODE_CNT];
    nlm_u32 m_data_len1;
    nlm_u16 m_ctx_addr1;
    
#ifdef NLM_NO_MEMCPY_IN_XPT
    nlm_u8* m_data1_p;
#else
    nlm_u8  m_data1[MAX_DATA_LEN_BYTES];
#endif

    /* Following flag will be set when there are instructions on both SMTs */
    NlmBool m_2smt;
    
    nlm_u8  m_smt_num;
    nlm_u8  m_port_num;

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

    nlm_u8  m_port_mode;
    nlm_u8  m_smt_mode;

}NlmXpt;

/*
 * NlmXpt_t is the generic xpt structure and it will have a pointer to the
 * actual XPT implementation structure. NlmXpt_operations will have pointers to
 * various functions.
 */

typedef struct NlmXpt_operations_t
{
    void *(*kbp_xpt_get_first_result)(void *, NlmReasonCode *);
    void *(*kbp_xpt_get_result)(void*, NlmXptRqt *, NlmReasonCode *);
    void  (*kbp_xpt_discard_result)(void *, NlmXptRqt *, NlmReasonCode *);
    void *(*kbp_xpt_get_result_for_rqtid)(void *, NlmRequestId *, NlmReasonCode *);
    NlmXptRqt *(*kbp_xpt_get_request)(void *, NlmReasonCode *);
    void  (*kbp_xpt_reset_requests)(void *, NlmReasonCode *);
    void  (*kbp_xpt_service_requests)(void *, NlmReasonCode *);
    NlmRequestId *(*kbp_xpt_get_request_id)(void *,NlmXptRqt *, NlmReasonCode *);
    NlmErrNum_t (*kbp_xpt_reset_device) (void *, NlmReasonCode *);
    NlmErrNum_t (*kbp_xpt_lock_config)(void *, nlm_u32 , NlmReasonCode *);
}NlmXpt_operations;
/*
 * NlmXpt__GetFirstAvailResult returns the first result available results.
 * Suppose, caller issued requests A,B,C and D and B,C,D were completed at the
 * time of call to this routine. Then the result for request B, which is the
 * first available result, will be returned.
 */

extern void *
kbp_xpt_get_first_result(NlmXpt *self,
      NlmReasonCode *);

/*
 * kbp_xpt_discard_result will simply cause a rqt object to be moved from in_use
 * list to the free list.
 * It may be called when some error encountered while formatting the requests or
 * before issuing ServiceRequest.
 */
extern void
kbp_xpt_discard_result(
    NlmXpt*       self,
    NlmXptRqt*     rqt,
    NlmReasonCode *o_reason);

/*
 * kbp_xpt_get_result sends a filled in result structure to upper layer of
 * the s/w. It will return NULL if (a) the NlmXptRqt onbject is not in the
 * in_use request list, or, (b) the request is not completed.
 * If the NlmXptRqt object passed to the function is NULL, then the result for the
 * first request in the in_use request list is returned.
 * If kbp_xpt_get_result is returning a non-null result, then the corresponding
 * request object will be moved to free request list.
 */

extern void*
kbp_xpt_get_result(
    NlmXpt*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode*   o_reason
    ) ;

/*
 * kbp_xpt_get_result_for_rqtid sends a filled in result structure to upper layer
 * of the s/w. It will return NULL if (a) the rqtid passed is Null ,or,(b) there
 * is no request with that rqtid available in the queue, or , (c) the request
 * with that request id is not serviced.
 * If it returns a non-null result, the corresponding request object is moved to
 * free list.
 */

extern void*
kbp_xpt_get_result_for_rqtid(
    NlmXpt*              self,
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode *o_reason
    ) ;


/*
 * NlmXpt__GetXptRequest returns a request object to the caller. Null is
 * returned if there is no more free requests.
 */

NlmXptRqt*
kbp_xpt_get_request(
    NlmXpt*     self,
    NlmReasonCode *o_reason
    ) ;

/*
 * kbp_xpt_reset_requests clears all the Requests, moves all the requests to the
 * free request list.
 */

extern void
kbp_xpt_reset_requests(
    NlmXpt*            self,
    NlmReasonCode *o_reason
    ) ;

/*
 * kbp_xpt_get_request_id returns to pointer to a request id specific for the
 * request rqt. If the derived xpt layer doesn't implement this function, it
 * will always return a NULL. Also, if rqt == NULL , or rqt is not in the in_use
 * request list of the lower level object, then also it will return a NULL.
 */

extern NlmRequestId *
kbp_xpt_get_request_id(
        NlmXpt  *xpt,
    NlmXptRqt*  rqt,
        NlmReasonCode *o_reason
    );
/*
 * kbp_xpt_service_requests triggers the execution of the requests in the
 * in_use request queue and which are not executed already.
 * Requests are executed in FIFO order.
 */

void
kbp_xpt_service_requests(
    NlmXpt*            self,
    NlmReasonCode *o_reason
    ) ;
/*
 * kbp_xpt_reset_device resests the device and any work request submitted earlier
 * will be cleared.
 */

NlmErrNum_t kbp_xpt_reset_device(
        NlmXpt* self,
        NlmReasonCode *o_reason
);

/*
 * kbp_xpt_lock_config locks an XPT configuration and this configuration should
 * not be modified in future.
 */

NlmErrNum_t kbp_xpt_lock_config(NlmXpt *self,
                nlm_u32 num_devies,
                NlmReasonCode *o_reason
                );

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


#ifndef NLMPLATFORM_BCM

extern void *
bcm_kbp_xpt_get_first_result(NlmXpt *self,
      NlmReasonCode *);

/*
 * kbp_xpt_discard_result will simply cause a rqt object to be moved from in_use
 * list to the free list.
 * It may be called when some error encountered while formatting the requests or
 * before issuing ServiceRequest.
 */
extern void
bcm_kbp_xpt_discard_result(
    NlmXpt*       self,
    NlmXptRqt*     rqt,
    NlmReasonCode *o_reason);

/*
 * kbp_xpt_get_result sends a filled in result structure to upper layer of
 * the s/w. It will return NULL if (a) the NlmXptRqt onbject is not in the
 * in_use request list, or, (b) the request is not completed.
 * If the NlmXptRqt object passed to the function is NULL, then the result for the
 * first request in the in_use request list is returned.
 * If kbp_xpt_get_result is returning a non-null result, then the corresponding
 * request object will be moved to free request list.
 */

extern void*
bcm_kbp_xpt_get_result(
    NlmXpt*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode*   o_reason
    ) ;

/*
 * kbp_xpt_get_result_for_rqtid sends a filled in result structure to upper layer
 * of the s/w. It will return NULL if (a) the rqtid passed is Null ,or,(b) there
 * is no request with that rqtid available in the queue, or , (c) the request
 * with that request id is not serviced.
 * If it returns a non-null result, the corresponding request object is moved to
 * free list.
 */

extern void*
bcm_kbp_xpt_get_result_for_rqtid(
    NlmXpt*              self,
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode *o_reason
    ) ;


/*
 * NlmXpt__GetXptRequest returns a request object to the caller. Null is
 * returned if there is no more free requests.
 */

NlmXptRqt*
bcm_kbp_xpt_get_request(
    NlmXpt*     self,
    NlmReasonCode *o_reason
    ) ;

/*
 * kbp_xpt_reset_requests clears all the Requests, moves all the requests to the
 * free request list.
 */

extern void
bcm_kbp_xpt_reset_requests(
    NlmXpt*            self,
    NlmReasonCode *o_reason
    ) ;

/*
 * kbp_xpt_get_request_id returns to pointer to a request id specific for the
 * request rqt. If the derived xpt layer doesn't implement this function, it
 * will always return a NULL. Also, if rqt == NULL , or rqt is not in the in_use
 * request list of the lower level object, then also it will return a NULL.
 */

extern NlmRequestId *
bcm_kbp_xpt_get_request_id(
        NlmXpt  *xpt,
    NlmXptRqt*  rqt,
        NlmReasonCode *o_reason
    );

#endif


#endif
/*[]*/
