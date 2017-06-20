/*
 * $Id: nlmcmallocator.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
/*@@NlmCmAllocator Module

   Summary
   This module provides the memory allocation interface for all Nlmnapse
   components. Nlmnapse does not use naked calls to malloc or free; rather,
   all Nlmnapse functions conform to this interface.
   
   Description
   The Nlmnapse API and its components do not make any assumptions about the
   memory management strategy for a client. Most Nlmnapse API components
   accept a pointer to a NlmCmAllocator which can be configured for a
   particular system. The default implementation uses the malloc/free
   type allocation strategy.
 
   Using the NlmCmAllocator, it is also possible to change memory allocation
   strategies for portions of the API or easily replace the standard version
   of the allocator with a debug version of the allocator. The NlmCmAllocator
   provides the mechanism to accomplish such replacements.
 
   The data structure for NlmCmAllocator also provides a member variable
   'm_clientData_p' which is exclusively meant for client use.
   This field can be used to store any allocator specific data.
*/

#ifndef INCLUDED_NLMCMALLOCATOR_H
#define INCLUDED_NLMCMALLOCATOR_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmexterncstart.h> /* this should be the final include */
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmexterncstart.h> /* this should be the final include */
#endif

typedef struct NlmCmAllocator NlmCmAllocator ;

/* provide short simple NlmCmAlloc alias for verbose NlmCmAllocator */
typedef struct NlmCmAllocator NlmCmAlloc ; /* typedef of convenience */
#define NlmCmAlloc__ctor        NlmCmAllocator__ctor
#define NlmCmAlloc__config      NlmCmAllocator__config
#define NlmCmAlloc__malloc      NlmCmAllocator__malloc
#define NlmCmAlloc__calloc      NlmCmAllocator__calloc
#define NlmCmAlloc__resize      NlmCmAllocator__resize
#define NlmCmAlloc__free        NlmCmAllocator__free
#define NlmCmAlloc__dtor        NlmCmAllocator__dtor

/* Type definition for the memory allocator (malloc) function. */
typedef void *(*NlmCmAllocator__alloc_t)(
    NlmCmAllocator*,
    size_t
    ) ;

typedef void *(*NlmCmAllocator__sysMalloc_t)(
    size_t
    ) ;

/* Type definition for the memory allocator function that zeros the
   allocated memory (calloc).
*/
typedef void *(*NlmCmAllocator__calloc_t)
    (NlmCmAllocator*, size_t, size_t) ;

/* Type definition for the memory reallocation/resizing function (resize). */
typedef void *(*NlmCmAllocator__resize_t)
    (NlmCmAllocator*, void*, size_t, size_t) ;

/* Type definition for the function used to free allocated memory (free). */
typedef void (*NlmCmAllocator__free_t)
    (NlmCmAllocator*, void*) ;

typedef void (*NlmCmAllocator__sysFree_t)
    (void*) ;

/* Type definition for the function used to destruct an allocator (dtor). */
typedef void (*NlmCmAllocator__dtor_t)
    (NlmCmAllocator*) ;

/* The alternate set of function pointers to use when managing dynamic memory,
   and a name to identify this particular allocator.
*/
typedef struct NlmCmAllocator__vtbl
{
    const char*                 className ;     /* Name of this allocator */
    NlmCmAllocator__alloc_t     m_malloc ;  /* Pointer to malloc function */
    NlmCmAllocator__calloc_t    m_calloc ;  /* Pointer to calloc function */
    NlmCmAllocator__resize_t    m_resize ;  /* Pointer to resize function */
    NlmCmAllocator__free_t      m_free ;    /* Pointer to free function */
    NlmCmAllocator__dtor_t      m_dtor ;    /* Pointer to destructor function */
    const char*                 className1 ;    /* Name of this allocator */
    NlmCmAllocator__sysMalloc_t m_sysMalloc; /* Pointer to the system Malloc function */
    NlmCmAllocator__sysFree_t   m_sysFree;  /* Pointer to the system Free function */
} NlmCmAllocator__vtbl ;

/* Type definition for a NlmCmAllocator. 
   ## NB: we embed the vtbl here, rather than point to it, to improve
   ##     performance by avoiding a pointer indirection on every function call.
*/
struct NlmCmAllocator
{
    NlmCmAllocator__vtbl        m_vtbl ;    /* Function table for basic allocation
                           Note: m_vtbl is the contents, not the pointer */
    NlmCmAllocator__vtbl *  m_vtbl_p ;  /* Pointer to above to make some macros work transparently */
    void *m_clientData_p ;          /* For use by client */
    
} ;

/* Summary
   Initialize a NlmCmAllocator.

   Description
   By default, NlmCmAllocators use the system
   malloc, calloc, resize, and free implementations. Custom memory allocation
   routines may be used.
 
   Parameters
   @param self The allocator to initialize.
  
   See Also
   NlmCmAllocator__config
*/
extern NlmCmAllocator*
NlmCmAllocator__ctor(
    NlmCmAllocator*     self
    ) ;

/* Summary
   Configure a memory allocator.

   Description
   Reconfigure an allocator to use alternate implementations of malloc, calloc,
   resize, and free. It is imperative that the version of free called to
   deallocate memory corresponds to the version of malloc used to allocate it.
   The allocator maintains a pointer to the vtbl structure throughout its
   lifetime, but does not free it when the allocator is destructed (dtor'ed).
   It remains the caller's responsibility to reclaim the vtbl memory.

   Parameters
   @param self The allocator to configure.
   @param vtbl The table of functions to use in the new allocator configuration.
*/
extern void
NlmCmAllocator__config(
    NlmCmAllocator*     self,
    NlmCmAllocator__vtbl*    vtbl
    ) ;

/* Summary
   Destruct a NlmCmAllocator. 

   Description
   Destroy a no longer needed allocator. This does <B>not</B> free any memory
   allocated by this allocator. That must be completed before calling this
   function.
  
   Parameters
   @param self The allocator to clean up after.
*/
extern void 
NlmCmAllocator__dtor(
    NlmCmAllocator      *self
    ) ;

/* Non-virtual base class dtor method */
extern void 
NlmCmAllocator__dtor_body
    (NlmCmAllocator*) ;

/* Summary
   Allocate a block of memory with the given size.

   Description
   See the documentation for your system malloc function for details on return
   value(s).

   Parameters
   @param self The allocator to use.
   @param size The number of bytes to allocate.
 
   See Also
   NlmCmAllocator__free
*/
void* 
NlmCmAllocator__malloc(
    NlmCmAllocator*     self,
    size_t          size
    ) ;

/* Summary
   Allocate and zero out a block of memory of the given size.

   Description
   See the documentation for your system calloc function for details on return
   value(s).
  
   Parameters
   @param self The allocator to use.
   @param cnt The number of elements
   @param size The size, in bytes, of each element
*/
void* 
NlmCmAllocator__calloc(
    NlmCmAllocator*     self,
    size_t          cnt,
    size_t          size
    ) ;

/* Summary
   Resize a previously allocated block of memory to the given size.

   Description
   Uses NlmCmAllocator__malloc() to obtain a memory chunk of size newSize.
   min(newSize, oldSize) bytes from memblk are copied to this new chunk before
   it is returned.  The old chunk is freed before the routine returns.
  
   Parameters
   @param self The allocator to use.
   @param memblk The (old) memory block to resize.
   @param newSize The size of the new memory block.
   @param oldSize The size of the old memory block.
*/
void* 
NlmCmAllocator__resize(
    NlmCmAllocator*     self,
    void*           memblk,
    size_t          newSize,
    size_t                  oldSize
    ) ;

/* Summary
   Free a previously allocated block of memory.

   Note
   It is always legal to attempt to free the null pointer, and the result is a no-op.

   Parameters
   @param self The allocator to use.
   @param memblk The memory block to free.
  
   See Also
   NlmCmAllocator__malloc
*/
void 
NlmCmAllocator__free(
    NlmCmAllocator*     self,
    void*           memblk
    ) ;

#ifndef __doxygen__

#define NlmCmAllocator__dtor(self)      (((self)->m_vtbl.m_dtor)(self))

#endif /*## __doxygen__ */

#ifndef __doxygen__
extern void NlmCmAllocator__Verify(void) ;
extern void NlmCmAllocator__pvt_Verify(NlmCmAllocator* alloc) ;
#endif /*## __doxygen__ */

#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

#endif
/*[]*/
