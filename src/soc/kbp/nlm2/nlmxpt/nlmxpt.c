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
 
#include "nlmxpt.h"

/*
 * NlmXpt module interfaces between the low level transport layer and upper
 * layer of the s/w. It defines a set of operations which are to be implemented
 * by the client transport layer.
 */

#define XPT_BASIC_CHECK(x,y) ((x) && ((x)->xpt_magic == XPT_MAGIC_NUMBER)\
                && ((x)->m_oprs) && (x)->m_derived && (y))

#define XPT_IS_LOCKED(x)  ( (NLMTRUE == (x)->m_locked))


/******************************************************************************
 * Function : NlmXpt__DiscardResult
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXpt      *rqt           = Pointer to NlmRequest object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__DiscardResult should simply cause a rqt object to be moved from
 * in_use list to the free list.
 * It may be called when some error encountered while formatting the requests or
 * before issuing ServiceRequest. The derived class may or may not implement
 * this interface.
 ******************************************************************************/

void NlmXpt__DiscardResult(
    NlmXpt*       self,
    NlmXptRqt*     rqt,
    NlmReasonCode *o_reason
    )
{
        NlmXpt_operations *oprs;
	if (!XPT_IS_LOCKED(self))
		return ;

	oprs = (NlmXpt_operations *)self->m_oprs;

        if (oprs->NlmXpt__DiscardResult)
                oprs->NlmXpt__DiscardResult(self->m_derived, rqt, o_reason);
	return ;

}

/******************************************************************************
 * Function : NlmXpt__GetFirstAvailableResult
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__GetFirstAvailableResult should return the first available result
 * maintained by the low-level transport layer.
 ******************************************************************************/

void *
NlmXpt__GetFirstAvailableResult(
                NlmXpt *self,
                NlmReasonCode *o_reason
        )
{
         NlmXpt_operations *oprs;
		 

        if (!XPT_IS_LOCKED(self)) return NULL;

	    oprs = (NlmXpt_operations *)self->m_oprs;

        if (oprs->NlmXpt__GetFirstAvailResult)
                return oprs->NlmXpt__GetFirstAvailResult(self->m_derived, o_reason);

        return NULL;
}

/******************************************************************************
 * Function : NlmXpt__GetResult
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmXptRqt*  *rqt           = Pointer to NlmXptRqt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__GetResult should send a filled in result structure to upper layer of
 * the s/w. It calls appropriate routine from lower levl transport layer to get
 * the result.
 ******************************************************************************/

void*  NlmXpt__GetResult(
    NlmXpt*              self,
    NlmXptRqt*              rqt,
    NlmReasonCode    *o_reason
    )
{
        NlmXpt_operations *oprs;

	if (!XPT_IS_LOCKED(self))
		return NULL;

	oprs = (NlmXpt_operations *)self->m_oprs;

        if (oprs->NlmXpt__GetResult)
        {
		return  oprs->NlmXpt__GetResult(self->m_derived, rqt, o_reason);
        }
	else
	{
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
        	return NULL;
	}
}

/******************************************************************************
 * Function : NlmXpt__GetResultForRqtId
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmRequestId *rqtid        = Pointer to NlmRequestId  object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__GetResultForRqtId should return a result associated with
 * NlmRequestId. the s/w. It calls appropriate routine from lower levl transport
 * layer to get the result.
 ******************************************************************************/

void* NlmXpt__GetResultForRqtId(
    NlmXpt*              self,
    NlmRequestId        *rqtid, /* the request id    */
    NlmReasonCode *o_reason
    )
{
        NlmXpt_operations *oprs;

	if (!XPT_IS_LOCKED(self))
		return NULL;

        oprs = (NlmXpt_operations *)self->m_oprs;

        if (oprs->NlmXpt__GetResultForRqtId)
        {
                return oprs->NlmXpt__GetResultForRqtId(self->m_derived, rqtid,
                                o_reason);
        }
        else
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
                return NULL;
        }
}

/******************************************************************************
 * Function : NlmXpt__GetRequest
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__GetRequest should return a request object to caller.
 * It calls appropriate routine from lower levl transport layer to get a request
 * object.
 ******************************************************************************/

NlmXptRqt* NlmXpt__GetRequest(
    NlmXpt*     self,
    NlmReasonCode *o_reason
    )
{
	NlmXpt_operations *oprs;
	
	if (!XPT_IS_LOCKED(self))
		return NULL;

	oprs = (NlmXpt_operations *)self->m_oprs;

	if (!oprs->NlmXpt__GetRequest)
	{
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
		return NULL;
	}
	return oprs->NlmXpt__GetRequest(self->m_derived, o_reason);
}

/******************************************************************************
 * Function : NlmXpt__ResetRequests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__ResetRequests should clear all the Requests maintained by the lower
 * level transport layer.It calls appropriate routine from lower levl transport
 * to reset the requests.
 ******************************************************************************/

void NlmXpt__ResetRequests(
    NlmXpt*            self,
    NlmReasonCode* o_reason
    )
{
	NlmXpt_operations *oprs;
	
	if (!XPT_IS_LOCKED(self))
		return;
	oprs = (NlmXpt_operations *)self->m_oprs;
	if (oprs->NlmXpt__ResetRequests)
        {
		oprs->NlmXpt__ResetRequests(self->m_derived, o_reason);
        }
        else
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
        }
}


/******************************************************************************
 * Function : NlmXpt__ServiceRequests
 *
 * Parameters:
 * void*       self           = Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason    = Location to save the reason code
 *
 * Summary:
 * NlmXpt__ServiceRequests should trigger the execution of the requests already
 * submitted. Requests are to be executed in FIFO order.It calls appropriate
 * routine from lower level transport to trigger the execution.
 ******************************************************************************/

void
NlmXpt__ServiceRequests(
    NlmXpt*            self,            /* transport interface to flush */
    NlmReasonCode* o_reason
    )
{
	NlmXpt_operations *oprs = (NlmXpt_operations *)self->m_oprs;;
	
	
	if (!XPT_IS_LOCKED(self))
		return;

	if (!oprs->NlmXpt__ServiceRequests)
    {
            if (o_reason)
                    *o_reason = NLMRSC_INVALID_PARAM;
            return ;
    }
	

    oprs->NlmXpt__ServiceRequests(self->m_derived, o_reason);

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

	self->m_oprs = opearations;
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
	nlm_u32 extra_expr = 1;

	if (!XPT_BASIC_CHECK(self,extra_expr))
		return;
	NlmCm__memset(self, 0, sizeof(NlmXpt));
}

/******************************************************************************
 * Function : NlmXpt__GetRequestId
 *
 * Parameters:
 * void*       self             =  Pointer to generic NlmXpt object
 * NlmXptRqt*  rqt              =  Pointer to NlmXptRqt object
 * NlmReasonCode *o_reason      =  Location to save the reason code
 * Summary:
 * NlmXpt__GetRequestId  returns the RequestId of of a request object
 ******************************************************************************/
NlmRequestId *
NlmXpt__GetRequestId(
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

        if (!opr->NlmXpt__GetRequestId)
        {
                *o_reason = NLMRSC_INVALID_PARAM;
                return NULL;
        }

        return opr->NlmXpt__GetRequestId(self->m_derived, rqt, o_reason);

}

/******************************************************************************
 * Function : NlmXpt__ResetDevice
 *
 * Parameters:
 * void*       self             =  Pointer to generic NlmXpt object
 * NlmReasonCode *o_reason      =  Location to save the reason code
 * Summary:
 * NlmXpt__ResetDevice should reinitialize underlying device. It calls
 * appropriate routine from the lower level layer to reset the device.
 ******************************************************************************/
NlmErrNum_t NlmXpt__ResetDevice(
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

        if (!oprs->NlmXpt__ResetDevice)
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
                return NLMERR_FAIL;
        }

        return oprs->NlmXpt__ResetDevice(self->m_derived, o_reason);

}

/*******************************************************************************
 * Function : NlmXpt__LockConfig
 *
 * Parameters:
 * NlmXpt *self              = Pointer to an NlmXpt object
 * nlm_u32                   = Number of devieces
 * NlmReasonCode *o_reason   = Location to save the reason code
 *
 * Summary:
 * NlmXpt__LockConfig should finalizes the various aspects of the underlying
 * device. Only after NlmXpt will accept request only after this routine is
 * called.
 ******************************************************************************/

NlmErrNum_t NlmXpt__LockConfig(NlmXpt *self,
                nlm_u32 num_devies,
                NlmReasonCode *o_reason
                )
{
        NlmXpt_operations *oprs;
		nlm_u32 extra_expr = 1;

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

        if (!oprs->NlmXpt__LockConfig)
        {
                if (o_reason)
                        *o_reason = NLMRSC_INVALID_PARAM;
                return NLMERR_FAIL;
        }

        if (NLMERR_OK ==
               oprs->NlmXpt__LockConfig(self->m_derived, num_devies, o_reason))
        {
                self->m_locked = NLMTRUE;
                return NLMERR_OK;

        }

        return NLMERR_FAIL;
}

/*[]*/
