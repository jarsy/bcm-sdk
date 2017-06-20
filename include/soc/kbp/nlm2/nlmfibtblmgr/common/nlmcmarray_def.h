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
 


/*@@NlmCmArray Define Module

   Summary
   This module is sort of a template that can be used to create multiple implementations.
   As such, we do not protect ourselves from multiple includes.

   Description 
   To use this template, the following macros are needed (some are optional):

   * NlmCmArrayElementType - This macro represents the data type stored in each
     element of the array.

   * NlmCmArrayNameSuffix - This macro is used to give the implemenation a suffix. This
     is used to generate all typedef/struct and function names. The value of this macro
     must be a simple string that can be appended to some prefix together to form a
     valid C name.

   * NlmCmArrayNamePrefix (optional) - This macro is used to give the implemenation a
     prefix. If such macro does not exist, the prefix "NlmCm" is used.

   * NlmCmArrayResizeFunc (optional) - If this macro is defined, it must be the algorithm
     used to resize the array. This function should only modify the m_nMaxSize and/or the
     m_array_p fields if necessary. The rest of the fields should be left untouched. Its
     prototype must look like:
     <Code>		    void Resize(NlmCmArrayTypeXXX *self, nlm_u32 newsize) ;</Code>

   * NlmCmArrayCtorHook (optional) - If this macro is defined, the code specified by
     this hook is invoked during construction. You have the variable 'self' available
     in this codes context.

   * NlmCmArrayDtorHook (optional) - If this macro is defined, the code specified by this
     hook is invoked during construction. You have the variable 'self' available in this
     codes context.

   These macros are undefined at the bottom of these files unless using the macro
   IMPLEMENTING_NLMCMARRAY. This is done so that this template can be used by multiple
   array implementations. However, the macro IMPLEMENTING_NLMCMARRAY must be defined when
   the implementation C code itself is being compiled.

   The default allocation behavior has the following features:

   * When the array shrinks, memory is not reclaimed until the memory size gets to zero
     In most cases when items are being removed from the array it is usually a waste of
     time to reclaim memory.

   * When the initial array is created, it is created zero size.

   You can get more control over the default behavior by using SetMinAllocSize and
   SetMaxFreeElems methods. Do not set SetMaxFreeElems to zero (or a very small number)
   as this will cause a memory allocation/free with every remove. The SetMinAllocSize
   should be given a number that is appropriate for your application.

   See Also
   nlmcmarray_uint32.[hc] for an example implementation

   Note:
   We could have used virtual functions (func. ptrs) registered with the array but since
   this array code may be used in a performance sensitive area, we chose to use macros for
   specialization.

   Once this template is used, you get a set of functions with the following prefix
   (assume that the array element suffix is XXX (see NlmCmArrayNameSuffix above)
   <PRE>
   NlmCmArrayXXX__ -- functions that operate on the array
   </PRE>
   Associated with the above mentioned functions, you have the corresponding types
   <PRE>
   NlmCmArrayXXX
   </PRE>
   Also, a generic set of macros are defined to get/set elements of an array. These
   macros have the simple NlmCmArray__ prefix and these are made generic for speed
   (wishing hard for inlining here, sigh!)
*/

#ifndef __doxygen__

#include <nlmcmbasic.h>
#include <nlmcmallocator.h>

#include <nlmcmexterncstart.h>	/* this should be the final include */

#ifndef NlmCmArrayNamePrefix
#define NlmCmArrayNamePrefix		NlmCm
#endif

#ifndef NLMCMARRAY_ONCE_DEFINED
#define NLMCMARRAY_ONCE_DEFINED	1

/*## We need to define these macros/types once and they will be available for every
   implementation of this template.
*/

#define NlmCmArrayConcat2(a,b)		NlmCmConcat2_(a,b)
#define NlmCmArrayConcat3(a,b,c)		NlmCmConcat3_(a,b,c)

#define NlmCmArrayMake(type)		NlmCmArrayConcat3(NlmCmArrayNamePrefix,Array,type)

#endif
/*## End of one-time definitions */

#define NlmCmArrayType			NlmCmArrayMake(NlmCmArrayNameSuffix)
#define NlmCmArrayMethod(m)		NlmCmArrayConcat3(NlmCmArrayType,__,m)

typedef struct NlmCmArrayType 		NlmCmArrayType ;

struct NlmCmArrayType
{
    NlmCmArrayElementType *	m_array_p ;	/* The actual array */
    NlmCmAllocator *		m_alloc_p ;	/* The allocator to use */
    nlm_u32			m_nCurSize ;	/* Current size of the array */
    nlm_u32			m_nMaxSize ;	/* Maximum size of the array */
    nlm_u32			m_nMinAllocSize ; /* Minimum allocated size (default = 0) */
    nlm_u32			m_nMaxFreeElems ; /* Maximum free elements allowed (default = infinity) */
} ;

/*## The following functions must be defined for each unique NlmCmArrayType */
extern NlmCmArrayType *	NlmCmArrayMethod(ctor)(NlmCmArrayType *self, NlmCmAllocator *alloc) ;
extern void		NlmCmArrayMethod(dtor)(NlmCmArrayType *self);
extern NlmCmArrayType *	NlmCmArrayMethod(create)(NlmCmAllocator *alloc) ;
extern void		NlmCmArrayMethod(destroy)(NlmCmArrayType *self);

/* The following ctor/create allows more control over the allocation strategies */
extern NlmCmArrayType *	NlmCmArrayMethod(ctor2)(NlmCmArrayType *self, NlmCmAllocator *alloc,
					       nlm_u32 minAllocElems, nlm_u32 maxFreeElems) ;
extern NlmCmArrayType *	NlmCmArrayMethod(create2)(NlmCmAllocator *alloc,
					       nlm_u32 minAllocElems, nlm_u32 maxFreeElems) ;

/* Sets the minium allocation size. It is recommended that 'numElems' by a power of 2 but
 * any value will work. */
extern void		NlmCmArrayMethod(SetMinAllocSize)(NlmCmArrayType *self, nlm_u32 numElems) ;

/* Sets the maximum number of free items allowed in the array before it is automatically re-allocated. */
extern void		NlmCmArrayMethod(SetMaxFreeElems)(NlmCmArrayType *self, nlm_u32 numElems) ;

/* Grows or shrinks the array */
extern void		NlmCmArrayMethod(SetSize)(NlmCmArrayType *self, nlm_u32 newsize);

/* Adds one element to the end of the array.

   Returns
   the index of the new item
*/
extern void		NlmCmArrayMethod(Add)(NlmCmArrayType *self, NlmCmArrayElementType);

/* Adds 'count' number of elements to the end of the array.

   Returns
   the index of the last item added to the array
*/
extern void		NlmCmArrayMethod(AddN)(NlmCmArrayType *self, NlmCmArrayElementType e, nlm_u32 count);

/* Inserts an element at the specified position. Inserting at the end of the array or
   beyond is similar to calling AddN(self, ix = (GetSize()-1).

   Returns
   the index of the last item added to the array
*/
extern void		NlmCmArrayMethod(InsertAt)(NlmCmArrayType *self, nlm_u32 ix, NlmCmArrayElementType);
extern void		NlmCmArrayMethod(RemoveAt)(NlmCmArrayType *self, nlm_u32 ix);
extern void		NlmCmArrayMethod(RemoveAtN)(NlmCmArrayType *self, nlm_u32 ix, nlm_u32 count);
extern void		NlmCmArrayMethod(RemoveLast)(NlmCmArrayType *self) ;
extern void		NlmCmArrayMethod(RemoveAll)(NlmCmArrayType *self) ;
extern void		NlmCmArrayMethod(Append)(NlmCmArrayType *self, const NlmCmArrayType *other) ;
extern void		NlmCmArrayMethod(Copy)(NlmCmArrayType *self, const NlmCmArrayType *other) ;

/* Verify is for unit test purposes only */
extern NlmErrNum_t	NlmCmArrayMethod(Verify)(void) ;


/* This is guarded with an ifndef since we could have more than one array defined at
   the same time, say a byte array and an integer array, and this guard ensures that
   we don't have redudent definitions.

   Since we undefine our working type names when not implementing the type specific
   methods, we need to define these general methods without recourse to our working type
   names.

   We define the bounds checking inline to avoid having to generate a type specific
   method (as would otherwise be needed in the using code, where the specific type names
   have been undefined).

   We expand any NlmCmArray macros that get undefined.

   We define these methods inline for performance.

   We cast the index to UNSIGNED to avoid warnings from VC++ in assert.
*/
#ifndef NlmCmArray__GetAt

/*##The following macros apply to all arrays but note that although they are macros,
   they are still typesafe.
*/

#define	NlmCmArray__GetSize(self)		(self)->m_nCurSize
#define	NlmCmArray__GetMaxSize(self)		(self)->m_nMaxSize
#define	NlmCmArray__GetMinAllocSize(self)	(self)->m_nMinAllocSize
#define	NlmCmArray__GetMaxFreeElems(self)	(self)->m_nMaxFreeElems

/* Returns a pointer to the element at the given position. This is useful when the
   array element is a struct.
*/
#define	NlmCmArray__GetAtRef(self,i)	(NlmCmAssert_((i) == (i), "unstable marco arg") NlmCmAssert_((nlm_u32)(i) < (self)->m_nCurSize, "NlmCmArray index in bounds") (self)->m_array_p + (i))

/* Returns the element at the specfied location */
#define	NlmCmArray__GetAt(self,i)	(NlmCmAssert_((i) == (i), "unstable macro arg") NlmCmAssert_((nlm_u32)(i) < (self)->m_nCurSize, "NlmCmArray index in bounds") (self)->m_array_p[i])

/* Sets the element at the speicifed position 'i'. */
#define	NlmCmArray__SetAt(self,i,x)	(NlmCmAssert_((i) == (i), "unstable macro arg") NlmCmAssert_((nlm_u32)(i) < (self)->m_nCurSize, "NlmCmArray index in bounds") (self)->m_array_p[i] = x)

#endif /*##(NlmCmArray__GetAt)*/

/******************************************************************************/

#include <nlmcmexterncend.h>

#ifndef IMPLEMENTING_NLMCMARRAY
#include <nlmcmarray_udef.h>
#endif

#endif /* __doxygen__ */

