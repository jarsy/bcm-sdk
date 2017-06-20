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
 

#include <nlmcmbasic.h>

#ifndef NlmCmArrayResizeFunc

#ifndef NLMCMARRAY_GROW_THRESH
/* The following must be a power of two */
#define NLMCMARRAY_GROW_THRESH	1024
#endif

static void
NlmCmArrayResizeFunc(
    NlmCmArrayType *self,
    nlm_u32 newsize)
{
    if (newsize < self->m_nMinAllocSize)
	newsize = self->m_nMinAllocSize ;

    if (newsize == 0) {			/* User wants to empty? */
	if (self->m_array_p) {
	    assert(self->m_nMaxSize != 0) ;
	    NlmCmAllocator__free(self->m_alloc_p, (void *)self->m_array_p) ;
	    self->m_array_p = 0 ;
	    self->m_nMaxSize = 0 ;
	}
    } else {
	nlm_u32 maxsize = self->m_nMaxSize ;
        NlmCmArrayElementType *old = self->m_array_p ;

	if (newsize > maxsize) {	/* Need to grow? */
	    if (newsize < NLMCMARRAY_GROW_THRESH) {
		/* We are less than the threshold. Keep doubling
		 * the existing size until we are satisfied */
		if (maxsize == 0) maxsize = 1 ;
		while (maxsize < newsize)
		    maxsize <<= 1 ;
	    } else {
		maxsize = (newsize + (NLMCMARRAY_GROW_THRESH-1)) & ~(NLMCMARRAY_GROW_THRESH-1) ;
	    }

	    if (self->m_array_p)
		self->m_array_p = 
		    (NlmCmArrayElementType *)NlmCmAllocator__resize(
			self->m_alloc_p, (void *)self->m_array_p,
			maxsize * sizeof(NlmCmArrayElementType),
                        self->m_nMaxSize * sizeof(NlmCmArrayElementType)) ;
	    else
		self->m_array_p = (NlmCmArrayElementType *)NlmCmAllocator__malloc(
		    self->m_alloc_p, maxsize * sizeof(NlmCmArrayElementType)) ;

            if(self->m_array_p) {
                NlmCm__memset((void *)(self->m_array_p + self->m_nMaxSize), 0,
                             (maxsize - self->m_nMaxSize) * sizeof(NlmCmArrayElementType)) ;
                self->m_nMaxSize = maxsize ;
            }
            
	} else if ((maxsize - newsize) > self->m_nMaxFreeElems) {
	    self->m_array_p = (NlmCmArrayElementType *)NlmCmAllocator__resize(
		self->m_alloc_p, (void*)self->m_array_p,
                newsize * sizeof(NlmCmArrayElementType),
                self->m_nMaxSize * sizeof(NlmCmArrayElementType)) ;

	    if (self->m_array_p) {
		self->m_nMaxSize = newsize ;
	    } 
	}

        if(!self->m_array_p) {
            /* Allocation failed, so restore the old ptr. */
            self->m_array_p = old ;
        }
    }
}
#endif /* NlmCmArrayResizeFunc */

NlmCmArrayType *
NlmCmArrayMethod(ctor)(
    NlmCmArrayType *self,
    NlmCmAllocator *alloc)
{
    self->m_alloc_p = alloc ;
    self->m_nMinAllocSize = self->m_nCurSize = self->m_nMaxSize = 0 ;
    self->m_nMaxFreeElems = UINT32_MAX ;
    self->m_array_p = 0 ;

#ifdef NlmCmArrayCtorHook
    NlmCmArrayCtorHook ;
#endif

    return self ;
}

void
NlmCmArrayMethod(dtor)(
    NlmCmArrayType *self)
{
#ifdef NlmCmArrayDtorHook
    NlmCmArrayDtorHook ;
#endif

    if (self->m_array_p)
	NlmCmAllocator__free(self->m_alloc_p, (void *)self->m_array_p) ;
}

NlmCmArrayType *
NlmCmArrayMethod(create)(
    NlmCmAllocator *alloc)
{
    NlmCmArrayType *ret = 
	(NlmCmArrayType *)NlmCmAllocator__malloc(alloc, sizeof(NlmCmArrayType)) ;
    return NlmCmArrayMethod(ctor)(ret, alloc) ;
}

NlmCmArrayType *
NlmCmArrayMethod(ctor2)(
    NlmCmArrayType *self,
    NlmCmAllocator *alloc,
    nlm_u32 minAllocElems,
    nlm_u32 maxFreeElems)
{
    NlmCmArrayMethod(ctor)(self, alloc) ;
    self->m_nMaxFreeElems = maxFreeElems ;
    if (minAllocElems) {
	self->m_nMinAllocSize = minAllocElems ;
	NlmCmArrayResizeFunc(self, self->m_nCurSize) ;
    }
    return self ;
}

NlmCmArrayType *
NlmCmArrayMethod(create2)(
    NlmCmAllocator *alloc,
    nlm_u32 minAllocElems,
    nlm_u32 maxFreeElems)
{
    NlmCmArrayType *ret = 
	(NlmCmArrayType *)NlmCmAllocator__malloc(alloc, sizeof(NlmCmArrayType)) ;
    return NlmCmArrayMethod(ctor2)(ret, alloc, minAllocElems, maxFreeElems) ;
}

void
NlmCmArrayMethod(destroy)(
    NlmCmArrayType *self)
{
    if (self) {
	NlmCmArrayMethod(dtor)(self) ;
	NlmCmAllocator__free(self->m_alloc_p, self) ;
    }
}

void
NlmCmArrayMethod(SetMinAllocSize)(
    NlmCmArrayType *self,
    nlm_u32 numElems)
{
    self->m_nMinAllocSize = numElems ;
    NlmCmArrayResizeFunc(self, self->m_nCurSize) ;
}

void
NlmCmArrayMethod(SetMaxFreeElems)(
    NlmCmArrayType *self,
    nlm_u32 numElems)
{
    self->m_nMaxFreeElems = numElems ;
    NlmCmArrayResizeFunc(self, self->m_nCurSize) ;
}

void
NlmCmArrayMethod(SetSize)(
    NlmCmArrayType *self,
    nlm_u32 newsize)
{
    self->m_nCurSize = newsize ;
    NlmCmArrayResizeFunc(self, newsize) ;
}

void
NlmCmArrayMethod(Add)(
    NlmCmArrayType *self,
    NlmCmArrayElementType item)
{
    if (self->m_nCurSize == self->m_nMaxSize)
	NlmCmArrayResizeFunc(self, self->m_nCurSize+1) ;
    else
	assert(self->m_nCurSize < self->m_nMaxSize) ;
    self->m_array_p[self->m_nCurSize++] = item ;
}

void
NlmCmArrayMethod(AddN)(
    NlmCmArrayType *self,
    NlmCmArrayElementType e,
    nlm_u32 count)
{
    NlmCmArrayElementType *ptr ;
    nlm_u32 ix = self->m_nCurSize ;
    nlm_u32 newsize = ix+count ;    

    assert((int)count > 0) ;

    NlmCmArrayResizeFunc(self, newsize) ;
    self->m_nCurSize = newsize ;
    for ( ptr = self->m_array_p ; ix < newsize ; ix++ )
	ptr[ix] = e ;
}

void
NlmCmArrayMethod(InsertAt)(
    NlmCmArrayType *self,
    nlm_u32 ix,
    NlmCmArrayElementType e)
{
    nlm_u32 oldcursize = self->m_nCurSize ;
    if (ix == oldcursize)			/* Inserting at the end */
	NlmCmArrayMethod(Add)(self, e) ;
    else if (ix > oldcursize) {			/* Inserting beyond the end? */
	NlmCmArrayMethod(AddN)(self, e, ix - oldcursize + 1) ;
    } else {					/* Inserting before the end */
	nlm_u32 oldmaxsize = self->m_nMaxSize ;
	if (oldcursize == oldmaxsize)		/* Do we need resizing? */
	    NlmCmArrayResizeFunc(self, oldcursize+1) ;
	else
	    assert(oldcursize < oldmaxsize) ;
	NlmCm__memmove((void *)(self->m_array_p+ix+1), (void *)(self->m_array_p+ix),
		sizeof(NlmCmArrayElementType) * (oldcursize - ix)) ;
	self->m_array_p[ix] = e ;
	self->m_nCurSize++ ;
    }
}

void
NlmCmArrayMethod(RemoveLast)(
    NlmCmArrayType *self)
{
    assert(self->m_nCurSize > 0) ;

    --self->m_nCurSize ;
    NlmCmArrayResizeFunc(self, self->m_nCurSize) ;
}

void
NlmCmArrayMethod(RemoveAll)(
    NlmCmArrayType *self)
{
    NlmCmArrayResizeFunc(self, 0) ;
    self->m_nCurSize = 0 ;
}

void
NlmCmArrayMethod(RemoveAt)(
    NlmCmArrayType *self,
    nlm_u32 ix)
{
    nlm_u32 lastix = self->m_nCurSize-1 ;

    assert(self->m_nCurSize > 0) ;
    assert((ix >= 0) && (ix <= lastix)) ;
    if (ix == lastix) {			/* Removing last? */
	NlmCmArrayResizeFunc(self, lastix) ;
    } else {
	int numcpy = lastix - ix ;
	NlmCm__memmove((void *)(self->m_array_p+ix), (void *)(self->m_array_p+ix+1),
	       sizeof(NlmCmArrayElementType) * numcpy) ;
    }

    self->m_nCurSize = lastix ;
}

void
NlmCmArrayMethod(RemoveAtN)(
    NlmCmArrayType *self,
    nlm_u32 ix,
    nlm_u32 count)
{
    nlm_u32 cursize = self->m_nCurSize ;

    if (count == 0) return ;

    assert(self->m_nCurSize > 0) ;
    assert((ix+count) <= cursize) ;
    assert((int)count > 0) ;

    if ((ix+count) >= cursize) {	/* Be tolerant.  Must never be ">" */
	cursize -= count ;
	NlmCmArrayResizeFunc(self, cursize) ;
	self->m_nCurSize = cursize ;
    } else {
	int numcpy = cursize - ix - count ;
	NlmCm__memmove((void *)(self->m_array_p+ix), 
		(void *)(self->m_array_p+ix+count),
	       sizeof(NlmCmArrayElementType) * numcpy) ;
	self->m_nCurSize -= count ;
    }
}

void
NlmCmArrayMethod(Append)(
    NlmCmArrayType *self,
    const NlmCmArrayType *other)
{
    nlm_u32 othersize ;

    assert(other != 0) ;
    othersize = NlmCmArrayMethod(GetSize)(other) ;
    if (othersize == 0)
	return ;		/* Nothing to do */
    else {
	nlm_u32 mysize = NlmCmArrayMethod(GetSize)(self) ;
	nlm_u32 newsize = mysize + othersize ;

	NlmCmArrayResizeFunc(self, newsize) ;
	NlmCm__memmove((void *)(self->m_array_p+mysize), (void *)other->m_array_p,
	       sizeof(NlmCmArrayElementType) * othersize) ;
	self->m_nCurSize = newsize ;
    }
}

void
NlmCmArrayMethod(Copy)(
    NlmCmArrayType *self,
    const NlmCmArrayType *other)
{
    nlm_u32 othersize ;

    if (self == other) return ;			/* Nothing to do */

    assert(other != 0) ;
    othersize = NlmCmArrayMethod(GetSize)(other) ;
    if (othersize == 0) {
	NlmCmArrayResizeFunc(self, 0) ;
	self->m_nCurSize = 0 ;
    } else {
	nlm_u32 mysize = NlmCmArrayMethod(GetSize)(self) ;

	if (mysize != othersize) {
	    if (mysize > othersize)
		NlmCmArrayResizeFunc(self, 0) ;	/* Free existing memory */
	    NlmCmArrayResizeFunc(self, othersize) ;
	}

	NlmCm__memmove((void *)self->m_array_p, (void *)other->m_array_p,
	       sizeof(NlmCmArrayElementType) * othersize) ;

	self->m_nCurSize = othersize ;
    }
}

/*[]*/
