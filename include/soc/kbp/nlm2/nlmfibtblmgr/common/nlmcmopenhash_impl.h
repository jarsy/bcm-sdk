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
#include <nlmcmmath.h>

#include <nlmcmexterncstart.h>

#ifdef NlmCmOpenHashUsePowerOf2
#   define NlmCmOpenHashGetHashValues(self,h1,h2,bound)		\
    {								\
	bound = self->m_nMaxSize-1 ;				\
	h2 = (h1 << 1) | 0x1 ;	/* Must be odd */		\
    }
#   define NlmCmOpenHashIterate(h1,h2,ix) 			\
    	for ( ix = h1 ; ; ix += h2 )
#   define NlmCmOpenHashBondIndex(ix,bound)			\
    	ix &= bound
#else
#   define NlmCmOpenHashGetHashValues(self,h1,h2,bound)		\
    {								\
	bound = self->m_nMaxSize ;				\
        h1 %= bound ;						\
        h2 = (h1 + bound) >> 1 ;				\
    }
#   define NlmCmOpenHashIterate(h1,h2,ix) 			\
    	for ( ix = h1 ; ; ix += h2 )
#   define NlmCmOpenHashBondIndex(ix,bound)			\
    	if (ix >= bound) ix -= bound
#endif

static void
NlmCmOpenHashMethod(pvt_NullAll)(
    NlmCmOpenHashType *self)
{
    nlm_u32 n = self->m_nMaxSize ;

#ifdef NlmCmOpenHashUseMemSet
    NlmCm__memset(self->m_slots_p, NLM_CAST_PTR_TO_NLM_U32(NlmCmOpenHashNullValue), sizeof(NlmCmOpenHashNullValue) * n) ;
#else
    nlm_u32 ix ;
    for (ix = 0 ; ix < n ; ix++ )
	self->m_slots_p[ix] = NlmCmOpenHashNullValue ;
#endif
}

#ifdef NlmCmOpenHashUsePowerOf2
static nlm_u32
RoundUp2PowerOfTwo(
    nlm_u32 n)
{
    nlm_u32 count ;

    /* N.B., PPC architecture doesn't follow C spec which says all
     * shifts should be done mod 32.
     */
    if( !n ) return 1 ;

    n-- ;
    for ( count = 0 ; n ; count++ )
	n >>= 1 ;

    assert(count < 32) ;
    return (1 << count) ;
}
#endif

NlmCmOpenHashType *
NlmCmOpenHashMethod(ctor)(
    NlmCmOpenHashType * 	self,
    NlmCmAllocator * 	alloc,
    nlm_u32 		initsz,		/* Is adjusted to be a power of 2 */
    nlm_u32		thresh,		/* Thereshold at which the table resizes (usually above 50 but below 80) */
    nlm_u32		resizeby)
{
    /* Look for reasonable initial sizes */
    assert(initsz >= 8) ; 
    assert(initsz < (1 << 30)) ;

    /* Thresholds must be reasonable as well */
    assert(thresh > 25) ;
    assert(thresh < 90) ;

#ifndef NlmCmOpenHashUsePowerOf2
    assert(resizeby > thresh) ;
    assert(resizeby < 1001) ;
#else
    assert(  1 == RoundUp2PowerOfTwo(  0)) ;
    assert(  1 == RoundUp2PowerOfTwo(  1)) ;
    assert(128 == RoundUp2PowerOfTwo(126)) ;
    assert(128 == RoundUp2PowerOfTwo(127)) ;
    assert(128 == RoundUp2PowerOfTwo(128)) ;
    assert(256 == RoundUp2PowerOfTwo(129)) ;
#endif

    initsz = (initsz * 100) / thresh ;

#ifndef NlmCmOpenHashUsePowerOf2
	initsz++;
#endif

    NlmCm__memset(self, 0, sizeof(*self)) ;

    assert(initsz >= 8) ;

    self->m_nThreshold 	= (nlm_u16)thresh ;
    self->m_alloc_p 	= alloc ;

#ifdef NlmCmOpenHashUsePowerOf2
    NlmCmBasic__Require((resizeby % 100) == 0) ;
    self->m_nMaxSize 	= RoundUp2PowerOfTwo(initsz) ;
#else
    self->m_nMaxSize	= NlmCmMath__FindNextPrime(initsz) ;
#endif
    self->m_nResizeBy   = (nlm_u16)resizeby ;

    self->m_nThreshSize = (self->m_nMaxSize * thresh) / 100 ;
    self->m_slots_p 	= (NlmCmOpenHashElementType *)NlmCmAllocator__malloc(
	alloc, self->m_nMaxSize * sizeof(NlmCmOpenHashElementType)) ;

    assert(self->m_nMaxSize > self->m_nThreshSize) ;

    NlmCmOpenHashMethod(pvt_NullAll)(self) ;

#ifdef NlmCmOpenHashCtorHook
    NlmCmOpenHashCtorHook ;
#endif

    return self ;
}

NlmCmOpenHashType *
NlmCmOpenHashMethod(create)(
    NlmCmAllocator * 	alloc,
    nlm_u32 		initsz,		/* Is adjusted to be a power of 2 */
    nlm_u32		thresh,		/* Thereshold at which the table resizes (usually above 50 but below 80) */
    nlm_u32		resizeby)
{
    NlmCmOpenHashType * 	self = (NlmCmOpenHashType *)NlmCmAllocator__malloc(
	alloc, sizeof(NlmCmOpenHashType)) ;

    return NlmCmOpenHashMethod(ctor)(self, alloc, initsz, thresh, resizeby) ;
}

void
NlmCmOpenHashMethod(dtor)(
    NlmCmOpenHashType * 	self)
{
#ifdef NlmCmOpenHashDtorHook
    NlmCmOpenHashDtorHook ;
#endif

    NlmCmAllocator__free(self->m_alloc_p, (void *)self->m_slots_p) ;
}

void
NlmCmOpenHashMethod(destroy)(
    NlmCmOpenHashType * 	self)
{
    if (self) {
	NlmCmAllocator *alloc = self->m_alloc_p ;
	NlmCmOpenHashMethod(dtor)(self) ;
	NlmCmAllocator__free(alloc, self) ;
    }
}

NlmBool
NlmCmOpenHashMethod(pvt_Resize)(
    NlmCmOpenHashType * self)
{
    NlmCmOpenHashElementType * 	newslots ;
    NlmCmOpenHashElementType * 	oldslots    = self->m_slots_p, *slot  ;
    nlm_u32 			ix, oldsize = self->m_nMaxSize ;
    nlm_u32 			newsize     = (oldsize * (100 + self->m_nResizeBy)) / 100 ;
    nlm_u32			oldcount    = self->m_nCurSize ;

#ifndef NlmCmOpenHashUsePowerOf2
    newsize = NlmCmMath__FindNextPrime(newsize) ;
#endif

    newslots = (NlmCmOpenHashElementType *)NlmCmAllocator__malloc(
	self->m_alloc_p, newsize * sizeof(NlmCmOpenHashElementType)) ;
    if (!newslots)
	return NlmFalse ;

    self->m_nMaxSize = newsize ;
    self->m_nThreshSize = (newsize * self->m_nThreshold) / 100 ;
    self->m_slots_p = newslots ;
    self->m_nCurSize = 0 ;
	self->m_nInvalidSlots = 0;
    NlmCmOpenHashMethod(pvt_NullAll)(self) ;

    slot = oldslots ;
    for ( ix = 0 ; ix < oldsize ; ix++, slot++ )
	if (NlmCmOpenHashIsValidItem(slot))
	    NlmCmOpenHashMethod(InsertUnique)(self, slot) ;

    (void)oldcount ;
    assert(oldcount == self->m_nCurSize) ;

    NlmCmAllocator__free(self->m_alloc_p, (void*)oldslots) ;
    return NlmTrue ;
}

NlmBool
NlmCmOpenHashMethod(pvt_Reconstruct)(
    NlmCmOpenHashType * self)
{
    NlmCmOpenHashElementType * 	newslots ;
    NlmCmOpenHashElementType * 	oldslots    = self->m_slots_p, *slot  ;
    nlm_u32 			ix, oldsize = self->m_nMaxSize ;
    nlm_u32 			newsize     = oldsize ;
    nlm_u32			oldcount    = self->m_nCurSize ;

    newslots = (NlmCmOpenHashElementType *)NlmCmAllocator__malloc(
	self->m_alloc_p, newsize * sizeof(NlmCmOpenHashElementType)) ;
    if (!newslots)
	return NlmFalse ;

    self->m_slots_p = newslots ;
    self->m_nCurSize = 0 ;
	self->m_nInvalidSlots = 0;
    NlmCmOpenHashMethod(pvt_NullAll)(self) ;

    slot = oldslots ;
    for ( ix = 0 ; ix < oldsize ; ix++, slot++ )
	if (NlmCmOpenHashIsValidItem(slot))
	    NlmCmOpenHashMethod(InsertUnique)(self, slot) ;

    (void)oldcount ;
    assert(oldcount == self->m_nCurSize) ;

    NlmCmAllocator__free(self->m_alloc_p, (void*)oldslots) ;
    return NlmTrue ;
}


    
NlmBool
NlmCmOpenHashMethod(InsertUnique)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	eltPtr)
{
    nlm_u32 h1, h2, ix, bound ;
    NlmBool ret = NlmFalse ;

    if (self->m_nCurSize >= self->m_nThreshSize)
	if (!NlmCmOpenHashMethod(pvt_Resize)(self))
	    return NlmFalse ;

    assert(*eltPtr != 0) ;
    assert(NlmCmOpenHashIsValidItem(eltPtr)) ;
    assert(NlmCmOpenHashMethod(Locate)(self, eltPtr) == 0) ;

    h1 = NlmCmOpenHashHashFunc(eltPtr) ;
    NlmCmOpenHashGetHashValues(self, h1, h2, bound) ;

    NlmCmOpenHashIterate(h1, h2, ix) {
	NlmCmOpenHashElementType *slot ;

	NlmCmOpenHashBondIndex(ix, bound) ;
	slot = self->m_slots_p + ix ;
	if ((NlmCmOpenHashIsNullValue(slot) || NlmCmOpenHashIsInvalidValue(slot))) {
	    if(NlmCmOpenHashIsInvalidValue(slot))
			self->m_nInvalidSlots--;

		*slot = *eltPtr ;
	    self->m_nCurSize++ ;

	    ret = NlmTrue ;
	    break ;
	}
    }

    assert(ret == NlmTrue) ;
    return ret ;
}

NlmBool
NlmCmOpenHashMethod(InsertOrReplace)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	eltPtr)
{
    nlm_u32 h1, h2, ix, bound ;
    NlmCmOpenHashElementType *dummySlot = 0 ;

    if (self->m_nCurSize >= self->m_nThreshSize)
	if (!NlmCmOpenHashMethod(pvt_Resize)(self))
	    return NlmFalse ;

    assert(*eltPtr != 0) ;
    assert(NlmCmOpenHashIsValidItem(eltPtr)) ;

    h1 = NlmCmOpenHashHashFunc(eltPtr) ;
    NlmCmOpenHashGetHashValues(self, h1, h2, bound) ;

    NlmCmOpenHashIterate(h1, h2, ix) {
	NlmCmOpenHashElementType *slot ;

	NlmCmOpenHashBondIndex(ix, bound) ;
	slot = self->m_slots_p + ix ;
	if (NlmCmOpenHashIsNullValue(slot)) {
	    if (dummySlot)
	    {
			*dummySlot = *eltPtr ;
			self->m_nInvalidSlots--;
	    }
	    else
			*slot = *eltPtr ;
		
	    self->m_nCurSize++ ;
	    return NlmTrue ;
	} else if (NlmCmOpenHashIsInvalidValue(slot)) {
	    if (!dummySlot)
		dummySlot = slot ;
	} else if (NlmCmOpenHashIsEqual(eltPtr, slot)) {
	    *slot = *eltPtr ;
	    return NlmTrue ;
	}
    }
}

NlmCmOpenHashElementType
NlmCmOpenHashMethod(Remove)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	eltPtr)
{
    nlm_u32 			h1 	= 0, h2 = 1 ;
    nlm_u32			ix, bound ;
    nlm_u32 			dummyix = 0 ;
    NlmCmOpenHashElementType * 	dummy 	= 0 ;
    NlmCmOpenHashElementType 	ret ;

    assert(*eltPtr != 0) ;
    assert(NlmCmOpenHashIsValidItem(eltPtr)) ;

    h1 = NlmCmOpenHashHashFunc(eltPtr) ;
    NlmCmOpenHashGetHashValues(self, h1, h2, bound) ;

    NlmCmOpenHashIterate(h1, h2, ix) {
	NlmCmOpenHashElementType *slot ;

	NlmCmOpenHashBondIndex(ix, bound) ;
	slot = self->m_slots_p + ix ;
	if (NlmCmOpenHashIsNullValue(slot)) {
	    break ;
	} else if (NlmCmOpenHashIsInvalidValue(slot)) {
	    if (!dummy) {
		dummyix = ix ;
		dummy = slot ;
	    }
	} else if (NlmCmOpenHashIsEqual(eltPtr, slot)) {
	    ret = *slot ;
	    NlmCmOpenHashInvalidateValue(slot) ;
	    assert(NlmCmOpenHashIsInvalidValue(slot)) ;
	    self->m_nCurSize-- ;
		self->m_nInvalidSlots++;
	    return ret ;
	}
    }

    return NlmCmOpenHashInvalidValue ;
}

NlmCmOpenHashElementType *
NlmCmOpenHashMethod(Locate)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	key)
{
    nlm_u32 h1, h2, ix, bound ;
    NlmCmOpenHashElementType *slots = self->m_slots_p ;

    assert(*key != 0) ;
    assert(NlmCmOpenHashIsValidItem(key)) ;

    h1 = NlmCmOpenHashHashFunc(key) ;
    NlmCmOpenHashGetHashValues(self, h1, h2, bound) ;

    NlmCmOpenHashIterate(h1, h2, ix) {
	NlmCmOpenHashElementType *slot ;

	NlmCmOpenHashBondIndex(ix, bound) ;
	slot = slots + ix ;
#ifdef NlmCmOpenHashCountProbes
	self->m_nNumLocateProbes++ ;
#endif
	if (NlmCmOpenHashIsNullValue(slot))
	    break ;
	else if (!NlmCmOpenHashIsInvalidValue(slot) && NlmCmOpenHashIsEqual(key, slot))
	    return slot ;
    }
	
    return 0 ;
}

NlmBool
NlmCmOpenHashMethod(Locate2)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	key,
    NlmCmOpenHashElementType ** 	ret)
{
    nlm_u32 h1, h2, ix, bound ;

    assert(ret != 0) ;
    assert(*key != 0) ;
    assert(NlmCmOpenHashIsValidItem(key)) ;

    *ret = 0 ;
    h1 = NlmCmOpenHashHashFunc(key) ;
    NlmCmOpenHashGetHashValues(self, h1, h2, bound) ;

    NlmCmOpenHashIterate(h1, h2, ix) {
	NlmCmOpenHashElementType *slot ;

	NlmCmOpenHashBondIndex(ix, bound) ;
	slot = self->m_slots_p + ix ;
	if (NlmCmOpenHashIsNullValue(slot)) {
	    if (!*ret)
		*ret = slot ;
	    break ;
	} else if (NlmCmOpenHashIsNullValue(slot)) {
	    *ret = slot ;
	} else if (NlmCmOpenHashIsEqual(key, slot)) {
	    *ret = slot ;
	    return NlmTrue ;
	}
    }

    assert(*ret != 0) ;
    return NlmFalse ;
}

void
NlmCmOpenHashMethod(RemoveAll)(
    NlmCmOpenHashType * 		self)
{
    NlmCmOpenHashMethod(pvt_NullAll)(self) ;
    self->m_nCurSize = 0 ;
	self->m_nInvalidSlots = 0;
}


void
NlmCmOpenHashMethod(ReplaceAt)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	slot,
    NlmCmOpenHashElementType * 	value)
{
    NlmBool checkSz = NlmFalse ;
    assert(!NlmCmOpenHashIsNullValue(value)) ;
    assert(slot >= self->m_slots_p) ;
    assert(slot < (self->m_slots_p + self->m_nMaxSize)) ;

    if (NlmCmOpenHashIsValidItem(slot)) {		/* Was previously valid */
	if (!NlmCmOpenHashIsValidItem(value))		/* Now it is in-valid */
	    self->m_nCurSize-- ;
    } else {						/* Was previously in-valid */
	if (NlmCmOpenHashIsValidItem(value)) {		/* Now it is valid */
	    self->m_nCurSize++ ;
	    checkSz = NlmTrue ;
	}
    }

	/* If old slot is invalid and new value is NOT invalid then decrement count.*/
	/* If old slot is NOT invalid and new value is invalid then increase count.*/

   if((NlmCmOpenHashIsInvalidValue(slot)) && 
   		(!NlmCmOpenHashIsInvalidValue(value)))
   	self->m_nInvalidSlots--;		/* old slot is invalid and new value is NOT invalid */
   else  if((!NlmCmOpenHashIsInvalidValue(slot)) && 
   		(NlmCmOpenHashIsInvalidValue(value))) 
   	self->m_nInvalidSlots++;	/* old slot is NOT invalid and new value is invalid */
   		

    *slot = *value ;

#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ASSERT
    /* We better find it if we just put it there. The only reason we would not
     * find it is because the item being inserted does not have the same has value
     * as the item being replaced. Big error on clients part
     */
    if (NlmCmOpenHashIsValidItem(slot))
	assert(NlmCmOpenHashMethod(Locate)(self, slot) != 0) ;
#endif

    if (checkSz && (self->m_nCurSize >= self->m_nThreshSize))
	NlmCmOpenHashMethod(pvt_Resize)(self) ;
}


NlmCmOpenHashItorType *
NlmCmOpenHashItorMethod(ctor)(
    NlmCmOpenHashItorType * 	self,
    NlmCmOpenHashType * 		tbl)
{
    self->m_pTbl = tbl ;
    self->m_nNext = 0 ;
    return self ;
}

void
NlmCmOpenHashItorMethod(dtor)(
    NlmCmOpenHashItorType * 	self)
{
    (void)self ;
}

NlmCmOpenHashElementType *
NlmCmOpenHashItorMethod(Next)(
    NlmCmOpenHashItorType * 	self)
{
    NlmCmOpenHashElementType *	slot  = self->m_pTbl->m_slots_p + self->m_nNext ;
    nlm_u32 			maxsz = NlmCmOpenHash__GetMaxSize(self->m_pTbl) ;

    while (self->m_nNext < maxsz) {
	self->m_nNext++ ;
	if (NlmCmOpenHashIsValidItem(slot))
	    return slot ;
	slot++ ;
    }
    return 0 ;
}

#include <nlmcmexterncend.h>


