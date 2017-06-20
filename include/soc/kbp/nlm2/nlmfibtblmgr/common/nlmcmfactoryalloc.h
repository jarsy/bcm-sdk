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
 

#ifndef INCLUDED_CYCMFACTORYALLOC_H
#define INCLUDED_CYCMFACTORYALLOC_H

/*@@NlmCmFactoryAlloc Module

  Summary
  This module creates an allocator based on a storage factory.

  Description
  It should not be used as a general purpose allocator. It is intended
  to be used in places where many many (thousands or more) similar sized
  object would be created and destoryed especially if they are small.
  In general, this should be slower than using a regular malloc but
  with considerable space savings (as much as 50%). Note that there is
  typically an 8byte or more overhead for each malloc which this allocator
  does not have thanks to the NlmCmStorageFactory (see NlmCmStorageFactory)

  While this module keeps some states (peak and current memory in use),
  the storage factory itself has a lot more stats.

  Typical use of this module is to

  - Create/Obtain a basic allocator
  - Create/Obtain a storage factory
  - Create using either __create or __ctor
  - Cast down to a NlmCmAllocator pointer
  - Use this casted down pointer wherever a basic allocator
    is accepted
  - When done destroy the object in reverse order
    (NlmCmFactoryAlloc, NlmCmStorageFactory, NlmCmAllocator)
  

  See Also
  NlmCmStorageFactory
  NlmCmAllocator
*/

#include <nlmcmstoragefactory.h>

#include <nlmcmexterncstart.h> /* this should be the final include */

struct NlmCmFactoryAlloc ;

typedef struct NlmCmFactoryAlloc
{
    NlmCmAllocator		m_base ;		/* Must be first item (we are deriving from it) */
    NlmCmAllocator__vtbl *	m_vtbl_p ;
#if 0
    nlm_u32			m_memoryInUse;		/* Amount of mem. currently in use */
    nlm_u32			m_peakMemoryInUse;	/* Peak memory usage */
#endif
    NlmCmAllocator *		m_pvt_alloc_p;
    NlmCmStorageFactory *	m_pvt_factory_p ;
} NlmCmFactoryAlloc ;

/* g++ requires macro definitions to be on one line: make it so */

NlmCmDECLARE_cast(NlmCmFactoryAlloc, NlmCmAllocator)
NlmCmDECLARE_castup(NlmCmFactoryAlloc, NlmCmAllocator, "NlmCmFactoryAlloc")

#define NlmCmFactoryAlloc__castdown(self)         	NlmCmUSE_cast(NlmCmFactoryAlloc, NlmCmAllocator, self)
#define NlmCmFactoryAlloc__castNlmCmAllocator(self)	NlmCmUSE_cast(NlmCmFactoryAlloc, NlmCmAllocator, self)
#define NlmCmFactoryAlloc__castup(self)         		NlmCmUSE_castup(NlmCmFactoryAlloc, NlmCmAllocator, self)

/* Summary
   Initialize a factory allocator

   Description
   Initialize a factory allocator with the given allocator and storage
   factory.  Currently, the supplied allocator is not really used for
   much of anything but this may change in the future as more
   functionality is added.

   Parameters
   @param self		Pointer to the allocator
   @param alloc		General purpose allocator to use for internal use
   @param fact		Factory to use for client allocs

*/
extern NlmCmFactoryAlloc *	NlmCmFactoryAlloc__ctor(
    NlmCmFactoryAlloc *self, 
    NlmCmAllocator *alloc,
    NlmCmStorageFactory *fact) ;

/* Summary
   Destruct the factory allocator and delete the memory for itself

   Description
   Cleanup an allocator that is no longer needed. This does <B>not</B> free any
   memory allocated via this allocator. It only de-allocates memory used by
   the allocator itself.

   Parameters
   @param self		The allocator to destroy
*/
extern void			NlmCmFactoryAlloc__dtor(
    NlmCmFactoryAlloc *self) ;

/* Allocates and intializes a factory allocator.

   Parameters
   @param alloc		General purpose allocator to use for internal use
   @param fact		Factory to use for client allocs

   See Also
   NlmCmFactoryAlloc__ctor
*/
extern NlmCmFactoryAlloc *	NlmCmFactoryAlloc__create(
    NlmCmAllocator *alloc,
    NlmCmStorageFactory *fact) ;

/* Summary
   Destruct the factory allocator and delete the memory for itself

   Description
   Delete an allocator that is no longer needed. This does <B>not</B> free any
   memory allocated via this allocator. It only de-allocates memory used by
   the allocator itself.

   Parameters
   @param self		The allocator to destroy
*/
extern void			NlmCmFactoryAlloc__destroy(
    NlmCmFactoryAlloc *self) ;

#ifndef __doxygen__

extern NlmErrNum_t		NlmCmFactoryAlloc__Verify(void);

#endif /*## __doxygen__ */

#include <nlmcmexterncend.h>

#endif
