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
 


/*@@NlmCmOpenHash Define Module
    Summary
    This module is a template implementation of an open address hash table.

    Description
    This is intended for high performance hash implementations and yet be very configurable.
    This module is configurable using the following macros.

    * NlmCmOpenHashElementType - This is the element being stored in the hash table. This macro
      must point to single-word-name of an actual C-type (not an "unsinged long" or "void *")
      Use a typedef to create a single-word-name if necessary

    * NlmCmOpenHashNameSuffix -- This macro is used to give the implementation a suffix.
      This suffix is used to generate all typedef/struct and function names. The value of this
      macro must be a simple string that can be appended to some prefix together to form a
      valid C name.

    * NlmCmOpenHashHashFunc -- This macro is given the key and two nlm_u32 ptrs. It is
      expected to return two hash values. It is better if the two hash values are not
      functions of each other.

          void NlmCmOpenHashHashFunc(NlmCmOpenHashElementType *, nlm_u32 *h1, nlm_u32 *h2)

    * NlmCmOpenHashNullValue -- User must provide a value that will be used as a NULL value.

    * NlmCmOpenHashInvalidValue -- User must provide a value that will be used to mark a
      slot as an invalid value.

          nlm_u32 NlmCmOpenHashIsInvalidValue(NlmCmOpenHashElementType *)

    * NlmCmOpenHashIsNullValue -- User must provide an macro that determines if the
      given slot has a Null Value. The expression is given a pointer to the slot and the
      pointer to the hash table itself. Returns zero for false

          nlm_u32 NlmCmOpenHashIsNullValue(NlmCmOpenHashElementType *)

    * NlmCmOpenHashIsValidItem (optional) -- This macro defines the tests for a slot with
      a valid item. By default, this is assumed to be true if it the NlmCmOpenHashIsNullValue
      and NlmCmOpenHashIsInvalidValue test false. Use this if there is a more efficient way
      to do this test. Returns zero for false

          nlm_u32 NlmCmOpenHashIsValidItem(NlmCmOpenHashElementType *)

    * NlmCmOpenHashIsEqual -- User must provide an macro that determines if the
      two slots are equal. The macro is given a pointer to the two items.
      Returns zero for false

          nlm_u32 NlmCmOpenHashIsEqual(NlmCmOpenHashElementType *,NlmCmOpenHashElementType *)

    * NlmCmOpenHashInvalidateValue (optional) -- This macro must set the given slot to the
      invalid value. By default a simple assignment is done

          void NlmCmOpenHashIsEqual(NlmCmOpenHashElementType *)

    * NlmCmOpenHashNamePrefix (optional) - This macro is used to give the implemenation a
      prefix. If such macro does not exist, the prefix "NlmCm" is used.

    * NlmCmOpenHashCtorHook (optional) - If this macro is defined, the code specified by
      this hook is invoked during construction. You have the variable 'self' available
      in this codes context.

    * NlmCmOpenHashDtorHook (optional) - If this macro is defined, the code specified by this
      hook is invoked during construction. You have the variable 'self' available in this
      codes context.

    * NlmCmOpenHashUseMemSet (optional) - If this macro is is defined, it is assumed that it is
      okay to initialize an array of slots with a memset using the NlmCmOpenHashNullValue.
      Use this to set the entire memory bytes that occupy a slot to the same value as defined
      by NlmCmOpenHashNullValue (usually 0x00000000 or 0xffffffff or any value if the element
      type is a byte or any byte value that can be repeated as a byte using memset and still
      work correctly when used in a test -- like 0xcdcdcdcdcd)

    * NlmCmOpenHashUsePowerOf2 (optional) - If this macro is defined, the initial table size is
      rounded up to a power of 2, and the table always doubles in size. Otherwise, the table
      size is always a prime number. This only matters really when the table is first created
      and when the table is resized in non-multiples of previous sizes. When we are using
      powers of two for the table, the resize is always by a factor of 2.
 */


#ifndef __doxygen__

#include <nlmcmbasic.h>
#include <nlmcmallocator.h>

#include <nlmcmexterncstart.h>	/* this should be the final include */

#ifndef NlmCmOpenHashNamePrefix
#define NlmCmOpenHashNamePrefix		NlmCm
#endif

#ifndef NlmCmOpenHashInvalidateValue
#define NlmCmOpenHashInvalidateValue(x)	*(x) = NlmCmOpenHashInvalidValue
#endif

#ifndef NlmCmOpenHashIsValidItem
#define NlmCmOpenHashIsValidItem(x)	((*(x) != NlmCmOpenHashNullValue) && (*(x) != NlmCmOpenHashInvalidValue))
#endif

#ifndef NLMCMOPENHASH_ONCE_DEFINED
#define NLMCMOPENHASH_ONCE_DEFINED	1

/*## We need to define these macros/types once and they will be available for every
   implementation of this template.
*/

#define NlmCmOpenHashConcat2(a,b)		NlmCmConcat2_(a,b)
#define NlmCmOpenHashConcat3(a,b,c)		NlmCmConcat3_(a,b,c)

#define NlmCmOpenHashMake(type)			NlmCmOpenHashConcat3(NlmCmOpenHashNamePrefix,OpenHash,type)
#define NlmCmOpenHashItorMake(type)		NlmCmOpenHashConcat3(NlmCmOpenHashNamePrefix,OpenHashItor,type)

#endif
/*## End of one-time definitions */

#define NlmCmOpenHashType			NlmCmOpenHashMake(NlmCmOpenHashNameSuffix)
#define NlmCmOpenHashItorType			NlmCmOpenHashItorMake(NlmCmOpenHashNameSuffix)
#define NlmCmOpenHashMethod(m)			NlmCmOpenHashConcat3(NlmCmOpenHashType,__,m)
#define NlmCmOpenHashItorMethod(m)		NlmCmOpenHashConcat3(NlmCmOpenHashItorType,__,m)

typedef struct NlmCmOpenHashType 		NlmCmOpenHashType ;
typedef struct NlmCmOpenHashItorType 		NlmCmOpenHashItorType ;

struct NlmCmOpenHashType
{
    NlmCmOpenHashElementType *	m_slots_p ;		/* The actual slots */
    NlmCmAllocator *		m_alloc_p ;		/* The allocator to use */
    nlm_u32			m_nCurSize ;		/* Number of occupied slots */
    nlm_u32			m_nMaxSize ;		/* Maximum number of slots */
    nlm_u32			m_nThreshSize ;
    nlm_u16			m_nThreshold ;
    nlm_u16			m_nResizeBy ;		/* Percent to grow by */
	nlm_u32			m_nInvalidSlots;	/* Num of Invalid / deleted slot */
    NlmCmOpenHashElementType	m_nullValue ;

#ifdef NlmCmOpenHashCountProbes
    nlm_u32			m_nNumLocateProbes ;
#endif
} ;

/*## The following functions must be defined for each unique NlmCmOpenHashType */
extern NlmCmOpenHashType *	NlmCmOpenHashMethod(ctor)(
    NlmCmOpenHashType *self,
    NlmCmAllocator *alloc,
    nlm_u32 initsz,	/* Is adjusted to be a power of 2 */
    nlm_u32 thresh,	/* Thereshold at which the table resizes (usually above 50 but below 80) */
    nlm_u32 resizeby); /* Percentage of size increase when resize needed (100 means double) */
extern void			NlmCmOpenHashMethod(dtor)(NlmCmOpenHashType *self);
extern NlmCmOpenHashType *	NlmCmOpenHashMethod(create)(
    NlmCmAllocator *alloc,
    nlm_u32 initsz,	/* Is adjusted to be a power of 2 */
    nlm_u32 thresh,	/* Thereshold at which the table resizes (usually above 50 but below 80) */
    nlm_u32 resizeby); /* Percentage of size increase when resize needed (100 means double) */
extern void			NlmCmOpenHashMethod(destroy)(NlmCmOpenHashType *self);

/* Verify is for unit test purposes only */
extern NlmErrNum_t		NlmCmOpenHashMethod(Verify)(void) ;

/*
 * If the given item is inserted, returns the to the inserted item if it is
 * a new insert or else it returns the old element if it got replaced.
 * Note that given item is deferenced is copied into a slot in the hash table.
 */
extern NlmBool			NlmCmOpenHashMethod(InsertOrReplace)(
    NlmCmOpenHashType *self, NlmCmOpenHashElementType*) ;

/*
 * Removes an element from the hash table. It returns the item that got removed
 * or else it returns
 */
extern NlmCmOpenHashElementType	NlmCmOpenHashMethod(Remove)(
    NlmCmOpenHashType *self, NlmCmOpenHashElementType*) ;

/*
 * Inserts a new element into the hash table. Asserts if the item already exists
 */
extern NlmBool			NlmCmOpenHashMethod(InsertUnique)(
    NlmCmOpenHashType *self, NlmCmOpenHashElementType*) ;

/*
 * Removes an element from the hash table. It returns the item that got removed
 * or else it returns
 */
extern void			NlmCmOpenHashMethod(RemoveAll)(
    NlmCmOpenHashType *self) ;
/*
 * Finds the given element in the hash table. Returns ptr to the key if it
 * exists and NULL otherwise. If a non-null is returned, the caller can safely
 * invalidated it (do not set it to null) or replace it with something
 * else
 */
extern NlmCmOpenHashElementType *NlmCmOpenHashMethod(Locate)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	key) ;

/*
 * Finds the given element in the hash table. Returns true if the key exists.
 *
 * If the key exists, 'ret' is a pointer to the slot where it was found. This
 * slot can be safely invalidated or replaced if the client wishes (do not set
 * it to NULL under any circumstance)
 *
 * If a key is not found, the 'ret' is a pointer to a slot where the key can
 * be safely inserted. 'ret' can be NULL (and nothing will be passed back)
 */
extern NlmBool			NlmCmOpenHashMethod(Locate2)(
    NlmCmOpenHashType * 		self,
    NlmCmOpenHashElementType * 	key,
    NlmCmOpenHashElementType ** 	ret) ;

/*
 * Replace an item at a given slot.
 *
 * The value being replaced can be anything but the new value must be
 * either an Invalid value or a valid item. It cannot be a Null item
 *
 * Upon return the slot may not be valid anymore because this operation
 * could have caused a resize of the hash-table
 *
 * This is typically used in conjunction with Locate2
 */
extern void			NlmCmOpenHashMethod(ReplaceAt)(
    NlmCmOpenHashType * self,
    NlmCmOpenHashElementType * slot,
    NlmCmOpenHashElementType * value) ;		/* Must not be a Null value */

#ifndef NlmCmOpenHash__GetSize

/*##The following macros apply to all hash tables but note that although they are macros,
   they are still typesafe.
*/

#define	NlmCmOpenHash__GetSize(self)			(self)->m_nCurSize
#define	NlmCmOpenHash__GetCount(self)			(self)->m_nCurSize
#define	NlmCmOpenHash__GetMaxSize(self)			(self)->m_nMaxSize
/* Returns a ptr to item at the given slot (item itself can by null or invalid) */
#define	NlmCmOpenHash__GetAtRef(self,ix)			((self)->m_slots_p + (ix))
/* Returns the item at the given slot (can by null or invalid) */
#define	NlmCmOpenHash__GetAt(self,ix)			(self)->m_slots_p[ix]

#endif /*##(NlmCmOpenHash__GetSize)*/

struct NlmCmOpenHashItorType
{
    nlm_u32			m_nNext ;
    NlmCmOpenHashType *		m_pTbl ;
} ;

extern NlmCmOpenHashItorType *	NlmCmOpenHashItorMethod(ctor)(
    NlmCmOpenHashItorType * 	self,
    NlmCmOpenHashType * 		tbl) ;

extern void			NlmCmOpenHashItorMethod(dtor)(
    NlmCmOpenHashItorType * 	self) ;

extern NlmCmOpenHashElementType *NlmCmOpenHashItorMethod(Next)(
    NlmCmOpenHashItorType * 	self) ;

/******************************************************************************/

#include <nlmcmexterncend.h>

#ifndef IMPLEMENTING_NLMCMOPENHASH
#include <nlmcmopenhash_udef.h>
#endif

#endif /* __doxygen__ */

/*[]*/
