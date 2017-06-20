/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: etu_xpt.c,v 1.4 Broadcom SDK $
 * File:  etu_xpt.c
 * Purpose:  XPT driver for Caladan3 External TCAM
 *           Interfaces to the KBP layer
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <shared/util.h>
#include <soc/sbx/caladan3/etu.h>
#include <soc/debug.h>

#ifdef INCLUDE_KBP

#include <soc/kbp/nlm2/arch/nlmarch.h>
#include <soc/kbp/common/nlmcmdevice.h>
#include <soc/kbp/common/nlmcmutility.h>
#include <soc/kbp/nlm2/nlmxpt/nlmxpt.h>


#define NLM_OPCODE_DBA_WRITE_BITS_8_6   NLM_OPCODE_PIO_WRITE_BITS_8_6
#define NLM_OPCODE_DBA_WRITE_BITS_5_0   NLM_OPCODE_PIO_WRITE_BITS_5_0
#define NLM_OPCODE_DBA_READ_X_BITS_8_6  NLM_OPCODE_PIO_READ_X_BITS_8_6
#define NLM_OPCODE_DBA_READ_X_BITS_5_0  NLM_OPCODE_PIO_READ_X_BITS_5_0
#define NLM_OPCODE_DBA_READ_Y_BITS_8_6  NLM_OPCODE_PIO_READ_Y_BITS_8_6
#define NLM_OPCODE_DBA_READ_Y_BITS_5_0  NLM_OPCODE_PIO_READ_Y_BITS_5_0
#define NLM_OPCODE_REG_WRITE_BITS_8_6   NLM_OPCODE_PIO_WRITE_BITS_8_6
#define NLM_OPCODE_REG_WRITE_BITS_5_0   NLM_OPCODE_PIO_WRITE_BITS_5_0
#define NLM_OPCODE_REG_READ_BITS_8_6    NLM_OPCODE_PIO_READ_X_BITS_8_6
#define NLM_OPCODE_REG_READ_BITS_5_0    NLM_OPCODE_PIO_READ_X_BITS_5_0
#define NLMDEV_MAX_NUM_LTRS             NLMDEV_NUM_LTR_SET


#ifdef NLM_MT_XPT 
#include <soc/kbp/common/nlmcmmt.h>
#endif

#define C3_ETU_XPT_MAX_REQ 1024   /* Enforce some limit ! */

#define C3_ETU_XPT_STATUS_IN_USE         0x0001
#define C3_ETU_XPT_STATUS_IN_PROGRESS    0x0002
#define C3_ETU_XPT_STATUS_COMPLETED      0x0004
#define C3_ETU_XPT_STATUS_FREE           0x0008

/*
 * Request Queue
 */
typedef struct soc_sbx_caladan3_etu_xpt_req_s {
    struct soc_sbx_caladan3_etu_xpt_req_s *n;
    struct soc_sbx_caladan3_etu_xpt_req_s *p;
    int           status;
    NlmRequestId  reqid;
    NlmXptRqt    *request;
    uint8         result[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX * 4];
    uint8         result_size;
    uint8         devid;
    uint8         vbit;
    uint8         flush;
} soc_sbx_caladan3_etu_xpt_req_t;


typedef struct soc_sbx_caladan3_etu_xpt_s {

    int              unit;

    /* Request management */
    soc_sbx_caladan3_etu_xpt_req_t *active_list;
    soc_sbx_caladan3_etu_xpt_req_t *pending_list;
    soc_sbx_caladan3_etu_xpt_req_t *free_list;
    sal_mutex_t      lock;
    uint32           max_reqs;             /* system limits */
    uint32           pending_reqs;         /* request serviced, num entries in pending_list */
    uint32           active_reqs;          /* requests submitted, num entries in active_list */
    uint32           free_reqs;            /* num entries in freelist */
    uint32           total_reqs_w0;        /* cumulative count of requests */
    uint32           total_reqs_w1;        /* cumulative count of requests */

    /* XPT related */
    NlmXpt *xpt;
    NlmXpt_operations operations;
    uint32 oprmode;
    uint32 speedmode;
    uint32 devType;
    
} soc_sbx_caladan3_etu_xpt_t;


soc_sbx_caladan3_etu_xpt_t _c3_etu_xpt[SOC_MAX_NUM_DEVICES];

#define C3_ETU_XPT(unit) (&_c3_etu_xpt[(unit)])

#define CALADAN3_XPT_LOCK(lock) sal_mutex_take((lock), sal_mutex_FOREVER)

#define CALADAN3_XPT_UNLOCK(lock) sal_mutex_give((lock))

#define CALADAN3_XPT_DQUEUE(r)  \
        do {                                             \
            if ((r)) {                                   \
                if ((r)->p) {(r)->p->n = (r)->n; }       \
                if ((r)->n) {(r)->n->p = (r)->p; }       \
            }                                            \
        } while (0)

#define NOT_CORRUPT(p)		  \
	((p[-1] == 0xaaaaaaaa) || \
	 (p[p[-2]] == 0xbbbbbbbb))

#define CALADAN3_XPT_ENQUEUE(l, r)  \
        do {                               \
            if ((r)) {                     \
                (r)->p = NULL;             \
                (r)->n = NULL;             \
                if ((l)) {                 \
                    (r)->n = (l);          \
                    (l)->p = (r);          \
                }                          \
                (l) = (r);                 \
            }                              \
        } while (0)


#define CALADAN3_XPT_VBIT_FORMAT(v) ((v) << 4)  /* NLSDK expects vbit at that pos */

/* reflected copy */
void xpt_copy(void *d, void *s, int w)
{
    uint8 *p = (uint8*)s;
    uint8 *q = d;
    if (w <= 0) return;
    for (q += w-1; q >= (uint8*)d; *q-- = *p++);
}

/* regular copy */
void xpt_exact_copy(void *d, void *s, int w)
{
    uint8 *p = (uint8*)s;
    uint8 *q = d;
    if (w <= 0) return;
    for (; q < (uint8*)(d)+w; *q++ = *p++);
}

void*
soc_sbx_caladan3_etu_xpt_assign_id(soc_sbx_caladan3_etu_xpt_t *c3xpt, soc_sbx_caladan3_etu_xpt_req_t *r)
{
    void *res = NULL;
    if (c3xpt && r) {
         if (~c3xpt->total_reqs_w0 == 0) {
             c3xpt->total_reqs_w0 = 0;
             c3xpt->total_reqs_w1++;
         } else {
             c3xpt->total_reqs_w0++;
         }
         r->reqid.m_count = c3xpt->total_reqs_w0;
         r->reqid.m_giga_count = c3xpt->total_reqs_w1;
         res = r;
    }
    return res;
}


int
soc_sbx_caladan3_etu_xpt_alloc_req(soc_sbx_caladan3_etu_xpt_t *c3xpt, 
                                   soc_sbx_caladan3_etu_xpt_req_t **request)
{
 
    soc_sbx_caladan3_etu_xpt_req_t *r;
    CALADAN3_XPT_LOCK(c3xpt->lock);

    if (c3xpt->free_list) {
        r = c3xpt->free_list;
        c3xpt->free_list = r->n;
        soc_sbx_caladan3_etu_xpt_assign_id(c3xpt, r);
        {
            uint32 *p = (uint32*)r;
            uint32 *q = (uint32*)r->request;
            assert(NOT_CORRUPT(p));
            assert(NOT_CORRUPT(q));
        }
        CALADAN3_XPT_ENQUEUE(c3xpt->active_list, r);
        c3xpt->active_reqs++;
        c3xpt->free_reqs--;
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_c3xpt_alloc_req: Free list exhausted\n")));
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("free: %d active: %d pending :%d max :%d\n"), 
                   c3xpt->free_reqs, c3xpt->active_reqs,
                   c3xpt->pending_reqs, c3xpt->max_reqs));
        CALADAN3_XPT_UNLOCK(c3xpt->lock);
        return SOC_E_MEMORY;
    }

    *request = r;
    CALADAN3_XPT_UNLOCK(c3xpt->lock);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_etu_xpt_free_req(soc_sbx_caladan3_etu_xpt_t *c3xpt, 
                                  soc_sbx_caladan3_etu_xpt_req_t *request)
{
    if (!request || !c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_c3xpt_free_req Invalid parameter \n")));
        return SOC_E_PARAM;
    }

    CALADAN3_XPT_LOCK(c3xpt->lock);

    CALADAN3_XPT_DQUEUE(request);
    /* Adjust the Head */
    if (request == c3xpt->active_list) {
        c3xpt->active_list = request->n;
    } else if (request == c3xpt->pending_list) {
        c3xpt->pending_list = request->n;
    }
    sal_memset(request->request, 0, sizeof(NlmXptRqt));
    sal_memset(&request->reqid, 0, sizeof(NlmRequestId));
    sal_memset(request->result, 0, sizeof(request->result));
    request->status = 0; request->vbit = 0; request->devid = 0;
    request->n = request->p = NULL;
    CALADAN3_XPT_ENQUEUE(c3xpt->free_list, request);
    c3xpt->free_reqs++;

    CALADAN3_XPT_UNLOCK(c3xpt->lock);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_etu_xpt_init_free_list(soc_sbx_caladan3_etu_xpt_t *c3xpt) 
{
    int i;
    soc_sbx_caladan3_etu_xpt_req_t *r;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("soc_sbx_caladan3_etu_xpt_alloc_req: max reqs %d\n"),
                 c3xpt->max_reqs));
    CALADAN3_XPT_LOCK(c3xpt->lock);

    for (i=0; i<c3xpt->max_reqs; i++) {
        r = sal_alloc(sizeof(soc_sbx_caladan3_etu_xpt_req_t), "ETU_XPT_REQ");
        if (r == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("soc_sbx_caladan3_etu_xpt_alloc_req: no memory for request %d\n"), i));
            
            CALADAN3_XPT_UNLOCK(c3xpt->lock);
            return SOC_E_MEMORY;
        }
        sal_memset(r, 0, sizeof(soc_sbx_caladan3_etu_xpt_req_t));
        r->request = sal_alloc(sizeof(NlmXptRqt), "ETU_XPT_NLM_MSG");
        if (r == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("soc_sbx_caladan3_etu_xpt_alloc_req: no memory for request %d\n"), i));
            CALADAN3_XPT_UNLOCK(c3xpt->lock);
            return SOC_E_MEMORY;
        }
        CALADAN3_XPT_ENQUEUE(c3xpt->free_list, r);
        c3xpt->free_reqs++;
    }

    CALADAN3_XPT_UNLOCK(c3xpt->lock);
    return SOC_E_NONE;
}


soc_sbx_caladan3_etu_xpt_req_t *
soc_sbx_caladan3_etu_xpt_find_request(soc_sbx_caladan3_etu_xpt_t *c3xpt, NlmXptRqt *req) 
{
    soc_sbx_caladan3_etu_xpt_req_t *r = NULL;

    CALADAN3_XPT_LOCK(c3xpt->lock);
    while ((r = c3xpt->pending_list) != NULL) {
        if (r && r->request == req) {
            break;
        }
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);

    return r;
}

soc_sbx_caladan3_etu_xpt_req_t *
soc_sbx_caladan3_etu_xpt_find_request_by_id(soc_sbx_caladan3_etu_xpt_t *c3xpt, NlmRequestId *id)
{
    soc_sbx_caladan3_etu_xpt_req_t *r = NULL;
    int found = 0;

    CALADAN3_XPT_LOCK(c3xpt->lock);
    while ((r = c3xpt->pending_list) != NULL) {
        if (r && (sal_memcmp(&r->reqid, id, sizeof(NlmRequestId)) == 0)) {
            found = 1;
            break;
        }
    }
    if (!found) {
        while ((r = c3xpt->active_list) != NULL) {
            if (r && (sal_memcmp(&r->reqid, id, sizeof(NlmRequestId)) == 0)) {
                found = 1;
                break;
            }
        }
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);

    return ((found) ? (r) : (NULL));

}



/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_get_first_avail_result
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_get_first_avail_result returns a filled in result structure to
 * upper layer of the s/w. It will return NULL if no results are available,
 * OTHERWISE it will return the first result available.
 ******************************************************************************/
void* 
soc_sbx_caladan3_etu_xpt_get_first_avail_result(void *self, NlmReasonCode *o_reason)
{
    void *res = NULL;
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;
    uint8 *p;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_get_first_result: Invalid XPT \n")));
        return NULL;
    }
    CALADAN3_XPT_LOCK(c3xpt->lock);

    if (c3xpt->pending_reqs) {
        r = c3xpt->pending_list;
        if (r) {
            c3xpt->pending_list = r->n;
        }
        if (r->request->m_result) {
            p = (uint8*)r->request->m_result;
	    if ((r->request->m_opcode[0] == NLM_OPCODE_CBWRITE_CMP1_BITS_8_6) ||
		(r->request->m_opcode[0] == NLM_OPCODE_CBWRITE_CMP2_BITS_8_6)) {
		if (r->result_size) {
		    
		    xpt_exact_copy(p, r->result, r->result_size);
		}		    
	    } else {
		p[0] = CALADAN3_XPT_VBIT_FORMAT(r->vbit);
		if (r->result_size) {
		    xpt_copy(p+1, r->result, r->result_size-1);
		}
	    }
        }
        c3xpt->pending_reqs--;
        res = r->request->m_result;
        soc_sbx_caladan3_etu_xpt_free_req(c3xpt, r);
    }

    CALADAN3_XPT_UNLOCK(c3xpt->lock);

    return res;
}



/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_get_result
 *
 * Parameters:
 * void*          self           = Pointer to generic NlmXpt object
 * NlmXptRqt      *rqt           = Pointer to NlmRequest object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_get_result returns a filled in result structure to upper layer of
 * the s/w. It will return NULL if (a) the NlmXptrqt onbject is not in the
 * in_use request list, or, (b) the request is not completed.
 * If the NlmXptrqt object passed to the function is NULL, then the result for
 * the first request in the in_use request list is returned.
 * If soc_sbx_caladan3_etu_xpt_get_result is returning a non-null result,then the corresponding
 * request object will be moved to free request list.
 ******************************************************************************/
void*  
soc_sbx_caladan3_etu_xpt_get_result(void* self, NlmXptRqt* rqt, NlmReasonCode *o_reason)
{
    void *res = NULL;
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;
    uint8 *p;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_get_result: Invalid XPT \n")));
        return NULL;
    }

    CALADAN3_XPT_LOCK(c3xpt->lock);

    if (rqt) {
        r = soc_sbx_caladan3_etu_xpt_find_request(c3xpt, rqt); 
        if (r) {
            if (r->request->m_result) {
		p = (uint8*)r->request->m_result;		
		if ((r->request->m_opcode[0] == NLM_OPCODE_CBWRITE_CMP1_BITS_8_6) ||
		    (r->request->m_opcode[0] == NLM_OPCODE_CBWRITE_CMP2_BITS_8_6)) {
		    if (r->result_size) {
			xpt_exact_copy(p, r->result, r->result_size);
		    }		    
		} else {
		    p[0] = CALADAN3_XPT_VBIT_FORMAT(r->vbit);
		    if (r->result_size) {
			xpt_copy(p+1, r->result, r->result_size-1);
		    }
		}
                c3xpt->pending_reqs--;
                res = r->request->m_result;
                soc_sbx_caladan3_etu_xpt_free_req(c3xpt, r);
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("soc_sbx_caladan3_etu_xpt_get_result: Result space not allocated\n")));
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("soc_sbx_caladan3_etu_xpt_get_result: Request not complete or not found\n")));
        }
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);

    return res;
}


/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_get_request_id
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXpt      *rqt           = Pointer to NlmRequest object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_get_request_id returns request_id associated with a request object, The
 * Request must be present in in_use queue, else NULL is returned.
 ******************************************************************************/
NlmRequestId *
soc_sbx_caladan3_etu_xpt_get_request_id(void *self, NlmXptRqt *rqt, NlmReasonCode *o_reason)
{
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_get_request_id: Invalid XPT \n")));
        return NULL;
    }

    CALADAN3_XPT_LOCK(c3xpt->lock);

    if (rqt) {
        r = soc_sbx_caladan3_etu_xpt_find_request(c3xpt, rqt); 
        if (!r) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("soc_sbx_caladan3_etu_xpt_get_result: Request not complete or not found\n")));
            CALADAN3_XPT_UNLOCK(c3xpt->lock);
            return NULL;
        }
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);
    return &r->reqid;
}

/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_get_result_for_reqid
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmRequestId  *rqtid       = Pointer to NlmRequestId object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_get_result_for_reqid sends a filled in result structure to upper layer
 * of the s/w. It will return NULL if (a) the rqtid passed is Null ,or,(b) there
 * is no request with that rqtid available in the queue, or , (c) the request
 * with that request id is not serviced.
 * If it returns a non-null result, the corresponding request object is moved to
 * free list.
 ******************************************************************************/
void* 
soc_sbx_caladan3_etu_xpt_get_result_for_reqid(void* self, NlmRequestId *reqid, 
                                              NlmReasonCode *o_reason)
{
    void *res = NULL;
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;
    uint8 *p;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_get_result_for_reqid: Invalid XPT \n")));
        return NULL;
    }
    CALADAN3_XPT_LOCK(c3xpt->lock);

    if (reqid) {
        r = soc_sbx_caladan3_etu_xpt_find_request_by_id(c3xpt, reqid); 
        if (r) {
            if (r->request->m_result) {
                p = (uint8*)r->request->m_result;
		if ((r->request->m_opcode[0] == NLM_OPCODE_CBWRITE_CMP1_BITS_8_6) ||
		    (r->request->m_opcode[0] == NLM_OPCODE_CBWRITE_CMP2_BITS_8_6)) {
		    if (r->result_size) {
			xpt_exact_copy(p, r->result, r->result_size);
		    }		    
		} else {
		    p[0] = CALADAN3_XPT_VBIT_FORMAT(r->vbit);
		    if (r->result_size) {
			xpt_copy(p+1, r->result, r->result_size-1);
		    }
		}
                c3xpt->pending_reqs--;
                res = r->request->m_result;
                soc_sbx_caladan3_etu_xpt_free_req(c3xpt, r);
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("soc_sbx_caladan3_etu_xpt_get_result_for_id: Result space not allocated\n")));
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("soc_sbx_caladan3_etu_xpt_get_result_for_id: Request not complete or not found\n")));
        }
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);

    return res;
}


/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_get_request
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
NlmXptRqt* 
soc_sbx_caladan3_etu_xpt_get_request(void *self, NlmReasonCode *o_reason)
{
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_get_request: Invalid XPT \n")));
        return NULL;
    }

    if (SOC_SUCCESS(soc_sbx_caladan3_etu_xpt_alloc_req(c3xpt, &r)) && (r != NULL)) {
        return r->request;   
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_get_request: Not allocated \n")));
        return NULL;
    }
}


/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_reset_requests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_reset_requests clears all the Requests, moves all the 
 * requests to the free request list. The requests are just moved to the free_list 
 * irrespective of they are executed or not.
 ******************************************************************************/
void 
soc_sbx_caladan3_etu_xpt_reset_requests(void* self, NlmReasonCode *o_reason)
{
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_reset_requests: Invalid XPT \n")));
        return;
    }

    CALADAN3_XPT_LOCK(c3xpt->lock);
    while ((r = c3xpt->pending_list) != NULL) {
        c3xpt->pending_reqs--;
        soc_sbx_caladan3_etu_xpt_free_req(c3xpt, r);
    }
    while ((r = c3xpt->active_list) != NULL) {
        c3xpt->active_reqs--;
        soc_sbx_caladan3_etu_xpt_free_req(c3xpt, r);
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);
    return;
}


/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_process_request
 *
 * Parameters:
 *
 * soc_sbx_caladan3_etu_xpt_t *c3xpt     = Pointer to a XPT object
 * NlmXptRqt *request,                   = Pointer to a XPT request
 * NlmReasonCode *o_reason               = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_process_request Invokes appropriate routine to execute the commands
 ******************************************************************************/

static void 
soc_sbx_caladan3_etu_xpt_process_request(soc_sbx_caladan3_etu_xpt_t *c3xpt,
                                         soc_sbx_caladan3_etu_xpt_req_t *r)
{
    int rv = SOC_E_NONE, value = 0;
    int db = 0, addr = 0;
    int devid = 0, valid = 0, wr_mode = 0;
    uint8 *data_ptr, *mask_ptr;
    NlmXptRqt *request;
    uint8 buf[MAX_DATA_LEN_BYTES];
    uint8 bufm[MAX_DATA_LEN_BYTES];

    request = r->request;
    /* Get data and Address */
    if ((NLM_OPCODE_CBWRITE_BITS_8_6 == request->m_opcode[0]) &&
            (NLM_OPCODE_CBWRITE_BITS_5_0 == request->m_opcode[1])) 
    {
        r->result_size = 1; /* 0bytes + 1 */

#ifdef NLM_NO_MEMCPY_IN_XPT
        data_ptr = request->m_data_p;
#else
        data_ptr = &request->m_data[0];
        xpt_copy(buf, data_ptr, request->m_data_len);
#endif
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("C3 XPT Tcam CB write \n")));
        rv = soc_sbx_caladan3_etu_tcam_cxtbuf_write(c3xpt->unit, 
                                                    request->m_ctx_addr[0],
                                                    buf,
                                                    (request->m_data_len << 3));
 
    }
    else if (((NLM_OPCODE_CBWRITE_CMP1_BITS_8_6 == request->m_opcode[0]) ||
                (NLM_OPCODE_CBWRITE_CMP2_BITS_8_6 == request->m_opcode[0])) &&
             (NLMDEV_MAX_NUM_LTRS >  request->m_opcode[1])) 
    {
#ifdef NLM_NO_MEMCPY_IN_XPT
        data_ptr = request->m_data_p;
#else
        data_ptr = &request->m_data[0];
        xpt_copy(buf, data_ptr, request->m_data_len);
#endif
        r->result_size = 16; /* 16bytes */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("C3 XPT Tcam Compare %d\n"), (request->m_data_len << 3)));
        rv = soc_sbx_caladan3_etu_tcam_compare(c3xpt->unit, 
                                               request->m_opcode[1],
					       request->m_ctx_addr[0],
                                               buf,
					       (NLM_OPCODE_CBWRITE_CMP1_BITS_8_6 == request->m_opcode[0]),
                                               (request->m_data_len << 3),
                                                r->result);
    }
    else if ((NLM_OPCODE_NOP_BITS_8_6 == request->m_opcode[0]) &&
            (NLM_OPCODE_NOP_BITS_5_0 == request->m_opcode[1])) 
    {
        r->result_size = 1; /* 0bytes + 1 */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("C3 XPT Tcam Nop\n")));
        rv = soc_sbx_caladan3_etu_tcam_nop(c3xpt->unit, r->result);
    } 
    else 
    {
        
    #ifdef NLM_NO_MEMCPY_IN_XPT
        value = request->m_address;
        data_ptr = request->m_data_p;
    #else
        value = ReadBitsInArrray(request->m_data,
                                 NLMDEV_REG_ADDR_LEN_IN_BYTES,
                                 31, 0);
        data_ptr = request->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES; 
        xpt_copy(buf, data_ptr, (request->m_data_len - NLMDEV_AB_ADDR_LEN_IN_BYTES));
    #endif
    
    
        /* Check if the read operation is for Database read or Reg Read*/
        if((value >> NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE) & 1) {
            db = 1;
    
            /* Get Valid bit */
            valid = (value >> NLMDEV_AB_ENTRY_VALID_BIT_IN_ADDR) & 1; 
    
	    /* Get the wr_mode bit */
	    wr_mode = (value >> NLMDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR) & 1;

            /* Get the mask */
    #ifdef NLM_NO_MEMCPY_IN_XPT
            mask_ptr = request->m_mask_p;
    #else
            mask_ptr = request->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES + NLMDEV_AB_WIDTH_IN_BYTES; 
            xpt_copy(bufm, mask_ptr, (request->m_data_len - NLMDEV_AB_ADDR_LEN_IN_BYTES - NLMDEV_AB_WIDTH_IN_BYTES));
            xpt_copy(buf, data_ptr, (request->m_data_len - NLMDEV_AB_ADDR_LEN_IN_BYTES - NLMDEV_AB_WIDTH_IN_BYTES));
    #endif
        }
    
        addr = (value & 0x7FFFFF);
        devid = ((value >> 23) & 0x3);
    
        if (db) {
            /* DB Operations */
            if ((NLM_OPCODE_DBA_WRITE_BITS_8_6 == request->m_opcode[0]) &&
                (NLM_OPCODE_DBA_WRITE_BITS_5_0 == request->m_opcode[1])) 
            {
                r->result_size = 0; /* 0bytes */
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META("C3 XPT Tcam DB Write\n")));
                rv = soc_sbx_caladan3_etu_tcam_db_write(c3xpt->unit, devid, addr, 
                                                        buf, bufm, valid, wr_mode,
                                                        (r->flush)?r->result:NULL);
            }
            else if ((NLM_OPCODE_DBA_READ_X_BITS_8_6 == request->m_opcode[0]) &&
                     (NLM_OPCODE_DBA_READ_X_BITS_5_0 == request->m_opcode[1])) 
            {
                r->result_size = 11; /* 10bytes + 1 */
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META("C3 XPT Tcam DB ReadX\n")));
                rv = soc_sbx_caladan3_etu_tcam_db_read_datax(c3xpt->unit, devid, addr, 
                                                             r->result, &r->vbit);
            }
            else if ((NLM_OPCODE_DBA_READ_Y_BITS_8_6 == request->m_opcode[0]) &&
                     (NLM_OPCODE_DBA_READ_Y_BITS_5_0 == request->m_opcode[1])) 
            {
                r->result_size = 11; /* 10bytes + 1 */
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META("C3 XPT Tcam DB Read Y\n")));
                rv = soc_sbx_caladan3_etu_tcam_db_read_masky(c3xpt->unit, devid, 
                                                             addr, r->result);
            }
        }
        else if ((NLM_OPCODE_REG_WRITE_BITS_8_6 == request->m_opcode[0]) &&
                   (NLM_OPCODE_REG_WRITE_BITS_5_0 == request->m_opcode[1])) 
        {
            r->result_size = 0; /* Device id is returned but ignored for compatibility */
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META("C3 XPT Tcam Reg Write \n")));
            rv = soc_sbx_caladan3_etu_tcam_reg_write(c3xpt->unit, devid, addr, 
                                                         buf, r->result);
        } 
        else if ((NLM_OPCODE_REG_READ_BITS_8_6 == request->m_opcode[0]) &&
                (NLM_OPCODE_REG_READ_BITS_5_0 == request->m_opcode[1])) 
        {
            r->result_size = 11; /* 10bytes + 1 */
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META("C3 XPT Tcam Reg Read \n")));
            rv = soc_sbx_caladan3_etu_tcam_reg_read(c3xpt->unit, devid, addr, 
                                                    r->result);
        }
        else {
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META("Invalid opcode detected\n")));
            rv = SOC_E_PARAM;
        }
    }
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("Request Processing failed \n")));
    }
}

/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_service_requests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 *  soc_sbx_caladan3_etu_xpt_service_requests  takes  the requests from the in_use request
 *  queue and execute them one by one. The "WRITE" requests are moved to
 *  free_list while COMPARE/READ requests are left in the in_use request queue
 *  itself. The COMPARE/READ requests are moved to free_list as results of calls
 *  to "GetResult" routine. Requests are always executed in FIFO order.
 ******************************************************************************/
void
soc_sbx_caladan3_etu_xpt_service_requests(void* self, NlmReasonCode  *o_reason)
{
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;

    *o_reason = 0;
    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_service_requests: Invalid XPT \n")));
        return;
    }

    CALADAN3_XPT_LOCK(c3xpt->lock);
    while ((r = c3xpt->active_list) != NULL) {
        {
            uint32 *p = (uint32*)r;
            uint32 *q = (uint32*)r->request;
            assert(NOT_CORRUPT(p));
            assert(NOT_CORRUPT(q));
        }
	
	/* if last in the active_list, set flush */
	if (c3xpt->active_reqs == 1) {
	    r->flush = TRUE;
	} else {
	    r->flush = FALSE;
	}

        soc_sbx_caladan3_etu_xpt_process_request(c3xpt, r);
        CALADAN3_XPT_DQUEUE(r);
        /* Adjust the Head */
        if (r == c3xpt->active_list) {
            c3xpt->active_list = r->n;
        }
        {
            uint32 *p = (uint32*)r;
            uint32 *q = (uint32*)r->request;
            assert(NOT_CORRUPT(p));      
            assert(NOT_CORRUPT(q));
        }
        if (r->result_size) {
            CALADAN3_XPT_ENQUEUE(c3xpt->pending_list, r);
            c3xpt->pending_reqs++;
        } else {
            CALADAN3_XPT_ENQUEUE(c3xpt->free_list, r);
            c3xpt->free_reqs++;
        }
        c3xpt->active_reqs--;
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);
}


/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_discard_result
 *
 * Parameters:
 *
 * soc_sbx_caladan3_etu_xpt_t *c3xpt         = Pointer to a soc_sbx_caladan3_etu_xpt_ object
 * NlmReasonCode *o_reason   = Location to save the reason code*
 * NlmXptRqt     *rqt        = Pointer to NlmXptrqt object
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_discard_result removes a request from in_use queue irrespective of
 * whether the request is executed or not.
 ******************************************************************************/
void 
soc_sbx_caladan3_etu_xpt_discard_result(void *self, 
                                        NlmXptRqt *rqt,
                                        NlmReasonCode *o_reason)
{
    soc_sbx_caladan3_etu_xpt_t  *c3xpt;
    soc_sbx_caladan3_etu_xpt_req_t *r = 0;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;
    if (!c3xpt) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_etu_xpt_get_result: Invalid XPT \n")));
        return;
    }

    CALADAN3_XPT_LOCK(c3xpt->lock);

    if (rqt) {
        r = soc_sbx_caladan3_etu_xpt_find_request(c3xpt, rqt); 
        if (r) {
            c3xpt->pending_reqs--;
            soc_sbx_caladan3_etu_xpt_free_req(c3xpt, r);
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("soc_sbx_caladan3_etu_xpt_get_result: Request not complete or not found\n")));
        }
    }
    CALADAN3_XPT_UNLOCK(c3xpt->lock);

}


/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_pvt_destroy
 *
 * Parameters:
 *
 * void *self = Pointer to a generic NlmXpt object
 * Summary:
 * soc_sbx_caladan3_etu_xpt_pvt_destroy frees the c3xpt object and does the necessary
 * cleanups
 ******************************************************************************/

void soc_sbx_caladan3_etu_xpt_pvt_destroy(void *self)
{
    soc_sbx_caladan3_etu_xpt_t *c3xpt;
    NlmXpt    *xpt;
    NlmReasonCode reason;
    soc_sbx_caladan3_etu_xpt_req_t *r;

    if (!self) return;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *) self;
    xpt = c3xpt->xpt;

    soc_sbx_caladan3_etu_xpt_reset_requests(self, &reason);
    while ((r = c3xpt->free_list) != NULL) {

        CALADAN3_XPT_DQUEUE(r);
        /* Adjust the Head */
        if (r == c3xpt->free_list) {
            c3xpt->free_list = r->n;
        }
        if (r->request) 
            sal_free(r->request);
        sal_free(r);
    }
    if (c3xpt->lock) {
        sal_mutex_destroy(c3xpt->lock);
        c3xpt->lock = NULL;
    }

    if (xpt)
        sal_free(xpt);

    sal_memset(c3xpt, 0, sizeof(soc_sbx_caladan3_etu_xpt_t));
    
}


/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_destroy
 *
 * Parameters:
 * soc_sbx_caladan3_etu_xpt_t *self         = Pointer to an soc_sbx_caladan3_etu_xpt_ object
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_destroy frees all the memory associated with the xpt structure
 ******************************************************************************/

void soc_sbx_caladan3_etu_xpt_destroy(
    NlmXpt* self
)
{
    if (!self) return;

    if (self->m_derived)
        soc_sbx_caladan3_etu_xpt_pvt_destroy(self->m_derived);
}

/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_reset_device
 *
 * Parameters:
 * soc_sbx_caladan3_etu_xpt_t *self         = Pointer to an soc_sbx_caladan3_etu_xpt object
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_reset_device reinitializes the device and all the requests are
 * reset too.
 ******************************************************************************/

NlmErrNum_t 
soc_sbx_caladan3_etu_xpt_reset_device(void *self, NlmReasonCode *o_reason)
{
    soc_sbx_caladan3_etu_xpt_t *c3xpt;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;

    soc_sbx_caladan3_etu_xpt_reset_requests(c3xpt, o_reason);

    return NLMERR_OK;
}

/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_lock_config
 *
 * Parameters:
 * void *self         = Pointer to an NlmXpt object
 * nlm_u32            = Number of devieces
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_lock_config finalizes the various aspects of the simulated device.
 * Only after soc_sbx_caladan3_etu_xpt_lock_config is called, it will start accepting requests
 ******************************************************************************/

NlmErrNum_t
soc_sbx_caladan3_etu_xpt_lock_config(void *self, nlm_u32 num_devices, NlmReasonCode *o_reason)
{
    
#if 0
    soc_sbx_caladan3_etu_xpt_t *c3xpt;
    NlmXpt    *xpt;

    c3xpt = (soc_sbx_caladan3_etu_xpt_t *)self;

    xpt = c3xpt->xpt;
#endif

    return NLMERR_OK;
}

/*******************************************************************************
 * Function : soc_sbx_caladan3_etu_xpt_create
 *
 * Parameters:
 *
 *  unit                   Unit number
 *  nlm_u32                devType = At present support to NLM_DEVTYPE_11K
 *                                and NLM_DEVTYPE_11K_S; Support can be added
 *                                for other devices also in future;
 *  nlm_u16                speed_mode = High or Standard speed of operation
 *  nlm_u32                max_rqt_count, = Max request count to be in a queue
 *  nlm_u16                opr_mode = Normal or advanced operation mode
 *  nlm_u32                chan_id
 *
 * Summary:
 * soc_sbx_caladan3_etu_xpt_create creates the transport object (NlmXpt) 
 * and returns a pointer to the object. 
 * It also initializes the virtual operation table of NlmXpt object
 ******************************************************************************/

void *
soc_sbx_caladan3_etu_xpt_create(
        int                    unit,
        uint32                 devType,
        uint16                 speed_mode,
        uint32                 max_rqt_count,  /*Max request count */
        uint16                 opr_mode,       /* normal or sahasra mode*/
        uint32                 chan_id
)
{
    soc_sbx_caladan3_etu_xpt_t *c3xpt = 0;
    NlmXpt    *xpt = 0;
    NlmXpt_operations *xpt_op;
    int rv = 0;

    if ((devType != NLM_DEVTYPE_2_S && devType != NLM_DEVTYPE_2) 
        || (opr_mode != NLMDEV_OPR_STANDARD && opr_mode != NLMDEV_OPR_SAHASRA))
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "C3 XPT unknown device\n")));
        return NULL;
    }

    c3xpt = &_c3_etu_xpt[unit];

    sal_memset(c3xpt, 0, sizeof(soc_sbx_caladan3_etu_xpt_t));

    c3xpt->unit = unit;
    c3xpt->oprmode = opr_mode;
    c3xpt->speedmode = speed_mode;
    c3xpt->devType = devType;

    /* Now construct the base xpt structure */

    xpt = sal_alloc(sizeof(NlmXpt), "C3 ETU XPT");
    if (!xpt) {
        /* No requests expected at this time, use destroy in other cases */
        sal_free(c3xpt);
        return NULL;
    }

    xpt_op = &c3xpt->operations;

    xpt_op->NlmXpt__GetResult = soc_sbx_caladan3_etu_xpt_get_result;
    xpt_op->NlmXpt__DiscardResult = 
        soc_sbx_caladan3_etu_xpt_discard_result;
    xpt_op->NlmXpt__GetResultForRqtId = 
        soc_sbx_caladan3_etu_xpt_get_result_for_reqid;
    xpt_op->NlmXpt__GetRequest = soc_sbx_caladan3_etu_xpt_get_request;
    xpt_op->NlmXpt__ResetRequests =
        soc_sbx_caladan3_etu_xpt_reset_requests;
    xpt_op->NlmXpt__GetRequestId = soc_sbx_caladan3_etu_xpt_get_request_id;
    xpt_op->NlmXpt__ServiceRequests = 
        soc_sbx_caladan3_etu_xpt_service_requests;
    xpt_op->NlmXpt__GetFirstAvailResult = 
        soc_sbx_caladan3_etu_xpt_get_first_avail_result;
    xpt_op->NlmXpt__ResetDevice = soc_sbx_caladan3_etu_xpt_reset_device;
    xpt_op->NlmXpt__LockConfig = soc_sbx_caladan3_etu_xpt_lock_config;

    xpt->m_derived = c3xpt;
    xpt->m_oprs = &c3xpt->operations;
    xpt->xpt_magic = XPT_MAGIC_NUMBER;
#if !defined NLM_MT_OLD && !defined NLM_MT
    xpt->m_xpt_chanel_id = chan_id;
#else
	xpt->m_xpt_chanel_id[0]  = chan_id;
#endif
    xpt->m_locked  = NLMFALSE;

    c3xpt->lock = sal_mutex_create("C3 ETU XPT MUTEX");
    if (c3xpt->lock == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "C3 XPT: Failed creating mutex\n")));
        return NULL;
    }

    c3xpt->xpt = xpt;

    /* Allocate max if nothing requested */
    c3xpt->max_reqs = max_rqt_count < 1 ? C3_ETU_XPT_MAX_REQ : max_rqt_count;
    rv = soc_sbx_caladan3_etu_xpt_init_free_list(c3xpt);
    if (SOC_FAILURE(rv)) {
        soc_sbx_caladan3_etu_xpt_destroy(xpt);
        return NULL;
    }

    return xpt;

}


#endif  /* INCLUDE_KBP */
#endif  /* BCM_CALADAN3_SUPPORT */
