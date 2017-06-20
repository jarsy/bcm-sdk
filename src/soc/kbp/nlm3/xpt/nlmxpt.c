/*
 * $Id: nlmxpt.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include "nlmxpt.h"

/*
 * NlmXpt module interfaces between the low level transport layer and upper
 * layer of the s/w. It defines a set of operations which are to be implemented
 * by the client transport layer.
 */

#define XPT_BASIC_CHECK(x,y) ((x) && ((x)->xpt_magic == XPT_MAGIC_NUMBER)\
                && ((x)->m_oprs) && (x)->m_derived && (y))

#define XPT_IS_LOCKED(x)  ( (NLMTRUE == (x)->m_locked))


#define NLM_XPT_EXTRA_EXPR   (1)

/******************************************************************************
 * Function : kbp_xpt_discard_result
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXpt      *rqt           = Pointer to NlmRequest object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_discard_result should simply cause a rqt object to be moved from
 * in_use list to the free list.
 * It may be called when some error encountered while formatting the requests or
 * before issuing ServiceRequest. The derived class may or may not implement
 * this interface.
 ******************************************************************************/


void kbp_xpt_discard_result(
    NlmXpt*       self,
    NlmXptRqt*     rqt,
    NlmReasonCode *o_reason
    )
{
        NlmXpt_operations *oprs;
    if (!XPT_IS_LOCKED(self))
        return ;

    oprs = (NlmXpt_operations *)self->m_oprs;

        if (oprs->kbp_xpt_discard_result)
                oprs->kbp_xpt_discard_result(self->m_derived, rqt, o_reason);
    return ;

}


/******************************************************************************
 * Function : kbp_xpt_get_first_result
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_get_first_result should return the first available result
 * maintained by the low-level transport layer.
 ******************************************************************************/

void *
kbp_xpt_get_first_result(
                NlmXpt *self,
                NlmReasonCode *o_reason
        )
{
         NlmXpt_operations *oprs;
         

        if (!XPT_IS_LOCKED(self)) 
            return NULL;

        oprs = (NlmXpt_operations *)self->m_oprs;

        if (oprs->kbp_xpt_get_first_result)
                return oprs->kbp_xpt_get_first_result(self->m_derived, o_reason);

        return NULL;
}



/******************************************************************************
 * Function : kbp_xpt_get_result
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXptRqt*  *rqt           = Pointer to NlmXptRqt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_get_result should send a filled in result structure to upper layer of
 * the s/w. It calls appropriate routine from lower levl transport layer to get
 * the result.
 ******************************************************************************/

void*  kbp_xpt_get_result(
    NlmXpt*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode    *o_reason
    )
{
        NlmXpt_operations *oprs;

    if (!XPT_IS_LOCKED(self))
        return NULL;

    oprs = (NlmXpt_operations *)self->m_oprs;

    if (oprs->kbp_xpt_get_result)
    {
        return  oprs->kbp_xpt_get_result(self->m_derived, rqt, o_reason);
    }
    else
    {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NULL;
    }
}


/******************************************************************************
 * Function : kbp_xpt_get_result_for_rqtid
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmRequestId *rqtid        = Pointer to NlmRequestId  object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_get_result_for_rqtid should return a result associated with
 * NlmRequestId. the s/w. It calls appropriate routine from lower levl transport
 * layer to get the result.
 ******************************************************************************/

void* kbp_xpt_get_result_for_rqtid(
    NlmXpt*              self,
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode *o_reason
    )
{
        NlmXpt_operations *oprs;

    if (!XPT_IS_LOCKED(self))
        return NULL;

        oprs = (NlmXpt_operations *)self->m_oprs;

        if (oprs->kbp_xpt_get_result_for_rqtid)
        {
            return oprs->kbp_xpt_get_result_for_rqtid(self->m_derived, rqtid, o_reason);
        }
        else
        {
            if (o_reason)
                *o_reason = NLMRSC_INVALID_PARAM;
            return NULL;
        }
}

/******************************************************************************
 * Function : kbp_xpt_get_request
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_get_request should return a request object to caller.
 * It calls appropriate routine from lower levl transport layer to get a request
 * object.
 ******************************************************************************/

NlmXptRqt* kbp_xpt_get_request(
    NlmXpt*     self,
    NlmReasonCode *o_reason
    )
{
    NlmXpt_operations *oprs;
    
    if (!XPT_IS_LOCKED(self))
        return NULL;

    oprs = (NlmXpt_operations *)self->m_oprs;

    if (!oprs->kbp_xpt_get_request)
    {
        if (o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NULL;
    }
    return oprs->kbp_xpt_get_request(self->m_derived, o_reason);
}


/******************************************************************************
 * Function : kbp_xpt_reset_requests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_reset_requests should clear all the Requests maintained by the lower
 * level transport layer.It calls appropriate routine from lower levl transport
 * to reset the requests.
 ******************************************************************************/

void kbp_xpt_reset_requests(
    NlmXpt*            self,
    NlmReasonCode* o_reason
    )
{
    NlmXpt_operations *oprs;
    
    if (!XPT_IS_LOCKED(self))
        return;
    oprs = (NlmXpt_operations *)self->m_oprs;
    if (oprs->kbp_xpt_reset_requests)
        {
        oprs->kbp_xpt_reset_requests(self->m_derived, o_reason);
        }
        else
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
        }
}


/******************************************************************************
 * Function : kbp_xpt_service_requests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_service_requests should trigger the execution of the requests already
 * submitted. Requests are to be executed in FIFO order.It calls appropriate
 * routine from lower level transport to trigger the execution.
 ******************************************************************************/

void
kbp_xpt_service_requests(
    NlmXpt*            self,            /* transport interface to flush */
    NlmReasonCode* o_reason
    )
{
    NlmXpt_operations *oprs = (NlmXpt_operations *)self->m_oprs;;
    
    
    if (!XPT_IS_LOCKED(self))
        return;

    if (!oprs->kbp_xpt_service_requests)
    {
            if (o_reason)
                    *o_reason = NLMRSC_INVALID_PARAM;
            return ;
    }
    

    oprs->kbp_xpt_service_requests(self->m_derived, o_reason);

}

/******************************************************************************
 * Function : NlmXpt__ctor
 *
 * Parameters:
 * void*       self             =  Pointer to generic NlmXpt object
 * NlmCmAllocator*  alloc       =  General purpose memory allocator
 * nlm_u32       max_rqt_count  =  Max #requests that can be queued
 * nlm_u32       ax_rslt_count  =  Max #results  that can be queued
 * void *opearations            =  pointer to xpt virtual table
 * void *client                 =  Lower level client object
 * NlmReasonCode *o_reason      = Location to save the reason code
 *
 * Summary:
 * NlmXpt__ctor is the constructor for NlmXpt objects.
 ******************************************************************************/


void NlmXpt__ctor(
    NlmXpt*              self ,
    NlmCmAllocator*      alloc , /* General purpose memory allocator */
    nlm_u32              max_rqt_count,  /*Max request count */
    nlm_u32              max_rslt_count,  /*Max result count */
    void *opearations,      /*pointer to xpt operations */
    void *client
    )
{
    if (!XPT_BASIC_CHECK(self, (alloc &&
            opearations && (max_rqt_count >= max_rslt_count))))
        return;

    self->m_oprs = (NlmXpt_operations*)opearations;
    self->m_derived = client;
    self->m_alloc = alloc;

}


/******************************************************************************
 * Function : NlmXpt__dtor
 *
 * Parameters:
 * void*       self             =  Pointer to generic NlmXpt object
 *
 * Summary:
 * NlmXpt__dtor resets the NlmXpt object
 ******************************************************************************/

void NlmXpt__dtor(
    NlmXpt* self
)
{
    nlm_u32 extra_expr = NLM_XPT_EXTRA_EXPR;

    if (!XPT_BASIC_CHECK(self,extra_expr))
        return;
    NlmCm__memset(self, 0, sizeof(NlmXpt));
}


/******************************************************************************
 * Function : kbp_xpt_get_request_id
 *
 * Parameters:
 * void*       self             =  Pointer to generic NlmXpt object
 * NlmXptRqt*  rqt              =  Pointer to NlmXptRqt object
 * NlmReasonCode *o_reason      =  Location to save the reason code
 * Summary:
 * kbp_xpt_get_request_id  returns the RequestId of of a request object
 ******************************************************************************/
NlmRequestId *
kbp_xpt_get_request_id(
        NlmXpt *self,
        NlmXptRqt*  rqt,
        NlmReasonCode *o_reason
    )
{
    NlmXpt_operations *opr;
        

    if (!XPT_IS_LOCKED(self) || !rqt)
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
                return NULL;
        }

        opr = self->m_oprs;

        if (!opr->kbp_xpt_get_request_id)
        {
                *o_reason = NLMRSC_INVALID_PARAM;
                return NULL;
        }

        return opr->kbp_xpt_get_request_id(self->m_derived, rqt, o_reason);

}


/******************************************************************************
 * Function : kbp_xpt_reset_device
 *
 * Parameters:
 * void*       self             =  Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason      =  Location to save the reason code
 * Summary:
 * kbp_xpt_reset_device should reinitialize underlying device. It calls
 * appropriate routine from the lower level layer to reset the device.
 ******************************************************************************/
NlmErrNum_t kbp_xpt_reset_device(
        NlmXpt* self,
        NlmReasonCode *o_reason
)
{
        NlmXpt_operations *oprs;
        

    if (!XPT_IS_LOCKED(self))
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_INPUT;

                return NLMERR_FAIL;
        }

        oprs = self->m_oprs;

        if (!oprs->kbp_xpt_reset_device)
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
                return NLMERR_FAIL;
        }

        return oprs->kbp_xpt_reset_device(self->m_derived, o_reason);

}



/*******************************************************************************
 * Function : kbp_xpt_lock_config
 *
 * Parameters:
 * NlmXpt *self              = Pointer to an NlmXpt object
 * nlm_u32                   = Number of devieces
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * kbp_xpt_lock_config should finalizes the various aspects of the underlying
 * device. Only after NlmXpt will accept request only after this routine is
 * called.
 ******************************************************************************/

NlmErrNum_t kbp_xpt_lock_config(NlmXpt *self,
                nlm_u32 num_devies,
                NlmReasonCode *o_reason
                )
{
        NlmXpt_operations *oprs;
        nlm_u32 extra_expr = NLM_XPT_EXTRA_EXPR;

    if (!XPT_BASIC_CHECK(self,extra_expr))
        {
            if (o_reason)
                *o_reason = NLMRSC_INVALID_INPUT;
            return NLMERR_FAIL;
        }

        /* Already locked ... Just return */
        if (XPT_IS_LOCKED(self))
                return NLMERR_OK;

        oprs = self->m_oprs;

        if (!oprs->kbp_xpt_lock_config)
        {
            if (o_reason)
                *o_reason = NLMRSC_INVALID_PARAM;
            return NLMERR_FAIL;
        }

        if (NLMERR_OK ==
               oprs->kbp_xpt_lock_config(self->m_derived, num_devies, o_reason))
        {
            self->m_locked = NLMTRUE;
            return NLMERR_OK;

        }

        return NLMERR_FAIL;
}

/* wraper functions to Support old APIs */

void NlmXpt__DiscardResult(
    NlmXpt*       self,
    NlmXptRqt*     rqt,
    NlmReasonCode *o_reason
    )
{
    kbp_xpt_discard_result(self,rqt,o_reason);
}

void *
NlmXpt__GetFirstAvailableResult(
                NlmXpt *self,
                NlmReasonCode *o_reason
        )
{
    return kbp_xpt_get_first_result(self,o_reason);
}

NlmXptRqt* NlmXpt__GetRequest(
    NlmXpt*     self,
    NlmReasonCode *o_reason
    )
{
    return kbp_xpt_get_request(self,o_reason);
}

NlmRequestId *
NlmXpt__GetRequestId(
        NlmXpt *self,
        NlmXptRqt*  rqt,
        NlmReasonCode *o_reason
    )
{
    return kbp_xpt_get_request_id(self,rqt,o_reason);
}

void*  NlmXpt__GetResult(
    NlmXpt*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode    *o_reason
    )
{
    return kbp_xpt_get_result(self,rqt,o_reason);
}

void* NlmXpt__GetResultForRqtId(
    NlmXpt*              self,
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode *o_reason
    )
{
    return kbp_xpt_get_result_for_rqtid(self,rqtid,o_reason);
}

NlmErrNum_t NlmXpt__LockConfig(NlmXpt *self,
                nlm_u32 num_devies,
                NlmReasonCode *o_reason
                )
{
    return kbp_xpt_lock_config(self,num_devies,o_reason);
}

NlmErrNum_t NlmXpt__ResetDevice(
        NlmXpt* self,
        NlmReasonCode *o_reason
)
{
    return kbp_xpt_reset_device(self,o_reason);
}

void NlmXpt__ResetRequests(
    NlmXpt*            self,
    NlmReasonCode* o_reason
    )
{
    kbp_xpt_reset_requests(self,o_reason);
}

void
NlmXpt__ServiceRequests(
    NlmXpt*            self,            /* transport interface to flush */
    NlmReasonCode* o_reason
    )
{
    kbp_xpt_service_requests(self,o_reason);
}


#ifndef NLMPLATFORM_BCM

void bcm_kbp_xpt_discard_result(
    NlmXpt*       self,
    NlmXptRqt*     rqt,
    NlmReasonCode *o_reason
    )
{
    kbp_xpt_discard_result(self,rqt,o_reason);
}

void *
bcm_kbp_xpt_get_first_result(
                NlmXpt *self,
                NlmReasonCode *o_reason
        )
{
    return kbp_xpt_get_first_result(self,o_reason);
}

NlmXptRqt* bcm_kbp_xpt_get_request(
    NlmXpt*     self,
    NlmReasonCode *o_reason
    )
{
    return kbp_xpt_get_request(self,o_reason);
}

NlmRequestId *
bcm_kbp_xpt_get_request_id(
        NlmXpt *self,
        NlmXptRqt*  rqt,
        NlmReasonCode *o_reason
    )
{
    return kbp_xpt_get_request_id(self,rqt,o_reason);
}

void*  bcm_kbp_xpt_get_result(
    NlmXpt*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode    *o_reason
    )
{
    return kbp_xpt_get_result(self,rqt,o_reason);
}

void* bcm_kbp_xpt_get_result_for_rqtid(
    NlmXpt*              self,
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode *o_reason
    )
{
    return kbp_xpt_get_result_for_rqtid(self,rqtid,o_reason);
}

NlmErrNum_t bcm_kbp_xpt_lock_config(NlmXpt *self,
                nlm_u32 num_devies,
                NlmReasonCode *o_reason
                )
{
    return kbp_xpt_lock_config(self,num_devies,o_reason);
}

NlmErrNum_t bcm_kbp_xpt_reset_device(
        NlmXpt* self,
        NlmReasonCode *o_reason
)
{
    return kbp_xpt_reset_device(self,o_reason);
}

void bcm_kbp_xpt_reset_requests(
    NlmXpt*            self,
    NlmReasonCode* o_reason
    )
{
    kbp_xpt_reset_requests(self,o_reason);
}

void
bcm_kbp_xpt_service_requests(
    NlmXpt*            self,            /* transport interface to flush */
    NlmReasonCode* o_reason
    )
{
    kbp_xpt_service_requests(self,o_reason);
}

#endif

/*[]*/
