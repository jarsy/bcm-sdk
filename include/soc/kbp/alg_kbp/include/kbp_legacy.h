/*
 **************************************************************************************
 Copyright 2009-2017 Broadcom Corporation

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

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error code type */
typedef uint32_t NlmErrNum_t;
#define NLMCMOK 0

/* Error code definitions */
#define NLMERR_OK           (0)
#define NLMERR_FAIL         (1)
#define NLMERR_NULL_PTR     (2)
#define NLMERR_OPERATION_NO_SUPPORT (4)


#define NLM_STRY(A)                                                     \
    do                                                                  \
    {                                                                   \
        NlmErrNum_t __tmp_status = A;                                    \
        if (__tmp_status != NLMERR_OK)                                     \
        {                                                               \
            return __tmp_status;                                        \
        }                                                               \
    } while(0)




typedef int NlmBool;
typedef          char       nlm_8 ;
typedef unsigned char       nlm_u8 ;
typedef   signed short      nlm_16 ;
typedef unsigned short      nlm_u16 ;
typedef   signed int        nlm_32 ;
typedef unsigned int        nlm_u32 ;

typedef void*               nlm_ptr;
typedef   float             nlm_float32 ;
typedef   double            nlm_float64 ;

enum
{
    NlmFALSE = 0,  /* logical false for NlmBool */
    NlmTRUE  = 1,  /* logical true for NlmBool */
    NLMFALSE = 0,  /* logical false for NlmBool */
    NLMTRUE  = 1,  /* logical true for NlmBool */
    NlmFalse = 0,  /* logical false for NlmBool */
    NlmTrue  = 1   /* logical true for NlmBool */
};

/* Reason code definitions */
/* NOTE: do not add reason codes in-between within a specific module. Always add it at the end.
  * For e.g., if you want to add a reason code for DevMgr, add it at the end of DevMgr
  * reason codes. The starting value of reason code of a module is 20 more than that of
  * predecessor module.
  */
typedef enum NlmReasonCode_t {
    NLMRSC_REASON_OK = 0,
    NLMRSC_LOW_MEMORY,
    NLMRSC_INVALID_MEMALLOC_PTR,
    NLMRSC_INTERNAL_ERROR,

    /* Simxpt, Xpt specific */
    NLMRSC_INVALID_XPT_PTR = 21,
    NLMRSC_INVALID_XPT_RQT_PTR,
    NLMRSC_INVALID_XPT_RSLT_PTR,
    NLMRSC_NOFREE_RQST,
    NLMRSC_NORQST_AVBL,
    NLMRSC_NORSLT_AVBL,
    NLMRSC_OPR_FAILURE,

    /* Device Manager specific */
    NLMRSC_INVALID_DEV_PTR = 41,
    NLMRSC_INVALID_DEVMGR_PTR,
    NLMRSC_INVALID_SHADOWDEV_PTR,
    NLMRSC_INVALID_KEY_PTR,
    NLMRSC_INVALID_SRCH_RSLT_PTR = 45,
    NLMRSC_INVALID_POINTER,
    NLMRSC_INVALID_OUTPUT,
    NLMRSC_INVALID_INPUT,
    NLMRSC_INVALID_REG_ADDRESS,
    NLMRSC_INVALID_DB_ADDRESS = 50,
    NLMRSC_INVALID_CB_ADDRESS,
    NLMRSC_INVALID_RANGE_ADDRESS,
    NLMRSC_INVALID_DATA,
    NLMRSC_INVALID_LTR_NUM,
    NLMRSC_INVALID_AB_NUM = 55,
    NLMRSC_INVALID_BLK_WIDTH,
    NLMRSC_INVALID_AB_INDEX,
    NLMRSC_DUPLICATE_DEVICE,
    NLMRSC_INVALID_PARAM,
    NLMRSC_DEV_MGR_CONFIG_LOCKED = 60,
    NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED,
    NLMRSC_CASCADE_NOT_EXIST,
    NLMRSC_INVALID_PARENT,
    NLMRSC_INVALID_DEVICE_TYPE,
    NLMRSC_INVALID_NUM_OF_DEVICES = 65,
    NLMRSC_INVALID_DATA_LENGTH,
    NLMRSC_DATA_LENGTH_ADDRESS_MISMATCH,
    NLMRSC_READONLY_REGISTER,
    NLMRSC_WRITEONLY_REGISTER,
    NLMRSC_INVALID_DEVICE_ID,
    NLMRSC_INVALID_OPER_MODE,
    NLMRSC_INVALID_BANK,

    /* Generic Table Manager related */
    NLMRSC_INVALID_GENERIC_TM = 81,
    NLMRSC_INVALID_9K_TM,
    NLMRSC_INVALID_GENERIC_TABLE,
    NLMRSC_INVALID_GTM_BLKS_RANGE,
    NLMRSC_INVALID_APP_CALLBACK = 85,
    NLMRSC_INVALID_TABLEID,
    NLMRSC_INVALID_SEARCH_ATTRIBUTES,
    NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES,
    NLMRSC_CONFIGURATION_LOCKED,
    NLMRSC_EXCESSIVE_NUM_TABLE_PAR_LTR = 90,
    NLMRSC_NO_LTR_CONFIGURED,
    NLMRSC_INVALID_BMR,
    NLMRSC_INVALID_RECORD,
    NLMRSC_DEVICE_DATABASE_FULL,
    NLMRSC_TABLE_FULL = 95,     /*No more entries can be inserted
                                   for the table. There may be free space on the chip */
    NLMRSC_TABLE_LIMIT_EXCEEDED,        /*The number of records in table
                                           is equal to the table size */
    NLMRSC_RECORD_NOTFOUND,
    NLMRSC_INVALID_RANGE_ATTRIBUTES,
    NLMRSC_INVALID_KEY_NUM,
    NLMRSC_INVALID_RESULT_SEQ_NUM = 100,
    NLMRSC_INVALID_TABLE_WIDTH,
    NLMRSC_INVALID_TABLE_ASSO_WIDTH,
    NLMRSC_SEARCHATTRIB_NOT_DELETED,
    /* nlm12k GTM reason codes */
    NLMRSC_INVALID_BANK_NUM,
    NLMRSC_INVALID_GTM_SB_BLKS_RANGE,

    /* Range Manager Related */
    NLMRSC_INVALID_RANGE_MGR_ATTR = 121,        /* Indicates invalid
                                                   Range Manager Attributes */
    NLMRSC_INVALID_RANGE_MGR,   /* Indicates invalid Range Manager,
                                   either this Range Mgr does not exist or it is NULL */
    NLMRSC_INVALID_DATABASE,    /* Indicates invalid Database,
                                   either this Database does not exist or it is NULL */
    NLMRSC_DUPLICATE_DATABASE_ID,
    NLMRSC_INVALID_RANGE_DB_ATTR = 125, /* Indicates invalid Range
                                           Database Attributes */
    NLMRSC_INVALID_RANGE,       /* Indicates Range pointer is NULL */
    NLMRSC_INVALID_MCOR_CONFIG,
    NLMRSC_INVALID_OUTPUT_RNG_PTR,      /* Indicates NULL pointer provided by the
                                           user to get the range-result */
    NLMRSC_INVALID_OUTPUT_NUM_PTR,      /* Indicates NULL pointer provided by the user
                                           to get the number of ranges matched in the result. */
    NLMRSC_DUPLICATE_RANGE_ID = 130,
    NLMRSC_INVALID_RANGE_SELECTION,
    NLMRSC_RANGE_DATABASE_FULL,
    NLMRSC_NO_SUPPORT_FOR_THIS_COMB,
    NLMRSC_NO_SUPPORT_FOR_SEARCHDB,

    /* FIB Table Manager related */
    NLMRSC_INVALID_TBLID_LEN = 161,
    NLMRSC_INVALID_FIB_MGR,
    NLMRSC_INVALID_FIB_TBL,
    NLMRSC_DUPLICATE_FIB_TBL_ID,
    NLMRSC_INVALID_FIB_BLKS_RANGE = 165,
    NLMRSC_INVALID_FIB_TBL_INDEX_RANGE,
    NLMRSC_INVALID_FIB_MAX_PREFIX_LENGTH,
    NLMRSC_INVALID_FIB_START_BIT_IN_KEY,
    NLMRSC_INVALID_PREFIX,
    NLMRSC_INVALID_IDX_SPACE_MGR = 170,
    NLMRSC_INVALID_IDX_RANGE_PTR,
    NLMRSC_IDX_RANGE_FULL,
    NLMRSC_INVALID_IDX_RANGE_GROW_FAILED,
    NLMRSC_MLP_GROW_FAILED,
    NLMRSC_LOCATE_NODE_FAILED = 175,
    NLMRSC_DUPLICATE_PREFIX,
    NLMRSC_PREFIX_NOT_FOUND,    /* Prefix you are trying to delete is not found */
    NLMRSC_INVALID_PREFIX_LEN,
    NLMRSC_UDA_ALLOC_FAILED,
    NLMRSC_DBA_ALLOC_FAILED,
    NLMRSC_PCM_ALLOC_FAILED,
    NLMRSC_IT_ALLOC_FAILED,
    NLMRSC_TMAX_EXCEEDED,       /* Prefix count exceeds tmax value */
    NLMRSC_OUT_OF_AD,

    /* 12k reason codes */
    NLMRSC_INVALID_IBF_LOCATION = 201,
    NLMRSC_INVALID_CLK_CYCLE,
    NLMRSC_INVALID_OPERATION,

    NLMRSC_INCORRECT_INST_TYPE = 205,
    NLMRSC_INSTRUCTION_OVERFLOW,
    NLMRSC_INSTRUCTION_ALREADY_SCHEDULED,
    NLMRSC_INVALID_NIP_VALUE,
    NLMRSC_INVALID_RETURN_AD_VALUE,

    NLMRSC_INVALID_RBF_LOC = 210,
    NLMRSC_INVALID_CAB_LOC,
    NLMRSC_INVALID_SRAM_SHIFT_VALUE,
    NLMRSC_INVALID_SRAM_SHIFT_DIR,
    NLMRSC_INVALID_JUMP_ON_AD,

    NLMRSC_INVALID_KEY_SUPERBLK_MAPPING = 215,
    NLMRSC_INVALID_BLK_RESULT_MAPPING,
    NLMRSC_COUNTER_WIDTH_NOT_SUPPORTED,
    NLMRSC_INVALID_COUNTER_CONFIG,
    NLMRSC_INVALID_COUNTER_COMMAND,

    NLMRSC_INVALID_DUA_ADDRESS,
    NLMRSC_INVALID_DUA_COMMAND,

    NLMRSC_INVALID_AD_FIELD_SELECT = 222,
    NLMRSC_INVALID_BRANCH_WAY,
    NLMRSC_INVALID_LATENCY_CONFIGURATION,

    NLMRSC_INVALID_ADDRESS,

    /* MT reason codes */
    NLMRSC_MT_SPINLOCK_INIT_FAILED = 226,
    NLMRSC_MT_MUTEX_INIT_FAILED,
    NLMRSC_MT_SEM_INIT_FAILED,
    NLMRSC_MT_SPINLOCK_DESTROY_FAILED,
    NLMRSC_MT_MUTEX_DESTROY_FAILED,
    NLMRSC_MT_SEM_DESTROY_FAILED,
    NLMRSC_MT_TRIENODE_DOESNOT_EXIST,

    NLMRSC_MT_INVALID_THREAD_ID,
    NLMRSC_MT_THREAD_ALREADY_REGISTERED,

    /*MAC table related reason codes */
    NLMRSC_INVALID_MAC_TM,
    NLMRSC_INVALID_MAC_SB_BLKS_RANGE,
    NLMRSC_INVALID_MAC_TBL,
    NLMRSC_INVALID_MAC_TBL_TYPE,
    NLMRSC_INVALID_MAC_START_BYTE_IN_KEY,
    NLMRSC_INVALID_MAC_AD_WIDTH,
    NLMRSC_INVALID_MAC_AD,

    /* Port/SMT related reason codes */
    NLMRSC_INVALID_PORT_NUM,
    NLMRSC_INVALID_SMT_NUM,
    NLMRSC_INVALID_INST_TYPE,

    /* vertical partitioning codes */
    NLMRSC_INVALID_AC2BANK_MAPPING,
    NLMRSC_INVALID_P1CTXID_SHIFT_VALUE,

    /* invalid UDA SB Range */
    NLMRSC_INVALID_UDA_SB_BLKS_RANGE,

    NLMRSC_XPT_FAILED,

    /*Other reason codes */
    NLMRSC_REASON_UNKNOWN,
    NLMRSC_API_NOT_SUPPORTED,
    NLMRSC_NO_RECORDS_FOUND,
    NLMRSC_REASON_END
} NlmReasonCode;

typedef struct NlmCmAllocator NlmCmAllocator;

/* provide short simple NlmCmAlloc alias for verbose NlmCmAllocator */
typedef struct NlmCmAllocator NlmCmAlloc;       /* typedef of convenience */
#define NlmCmAlloc__ctor        NlmCmAllocator__ctor
#define NlmCmAlloc__config      NlmCmAllocator__config
#define NlmCmAlloc__malloc      NlmCmAllocator__malloc
#define NlmCmAlloc__calloc      NlmCmAllocator__calloc
#define NlmCmAlloc__resize      NlmCmAllocator__resize
#define NlmCmAlloc__free        NlmCmAllocator__free
#define NlmCmAlloc__dtor        NlmCmAllocator__dtor

/* Type definition for the memory allocator (malloc) function. */
typedef void *(*NlmCmAllocator__alloc_t) (NlmCmAllocator *, size_t);

typedef void *(*NlmCmAllocator__sysMalloc_t) (size_t);

/* Type definition for the memory allocator function that zeros the
   allocated memory (calloc).
*/
typedef void *(*NlmCmAllocator__calloc_t)
 (NlmCmAllocator *, size_t, size_t);

/* Type definition for the memory reallocation/resizing function (resize). */
typedef void *(*NlmCmAllocator__resize_t)
 (NlmCmAllocator *, void *, size_t, size_t);

/* Type definition for the function used to free allocated memory (free). */
typedef void (*NlmCmAllocator__free_t)
 (NlmCmAllocator *, void *);

typedef void (*NlmCmAllocator__sysFree_t)
 (void *);

/* Type definition for the function used to destruct an allocator (dtor). */
typedef void (*NlmCmAllocator__dtor_t)
 (NlmCmAllocator *);

/* The alternate set of function pointers to use when managing dynamic memory,
   and a name to identify this particular allocator.
*/
typedef struct NlmCmAllocator__vtbl {
    const char *className;      /* Name of this allocator */
    NlmCmAllocator__alloc_t m_malloc;   /* Pointer to malloc function */
    NlmCmAllocator__calloc_t m_calloc;  /* Pointer to calloc function */
    NlmCmAllocator__resize_t m_resize;  /* Pointer to resize function */
    NlmCmAllocator__free_t m_free;      /* Pointer to free function */
    NlmCmAllocator__dtor_t m_dtor;      /* Pointer to destructor function */
    const char *className1;     /* Name of this allocator */
    NlmCmAllocator__sysMalloc_t m_sysMalloc;    /* Pointer to the system Malloc function */
    NlmCmAllocator__sysFree_t m_sysFree;        /* Pointer to the system Free function */
} NlmCmAllocator__vtbl;

/* Type definition for a NlmCmAllocator.
   ## NB: we embed the vtbl here, rather than point to it, to improve
   ##     performance by avoiding a pointer indirection on every function call.
*/
struct NlmCmAllocator {
    NlmCmAllocator__vtbl m_vtbl;        /* Function table for basic allocation
                                           Note: m_vtbl is the contents, not the pointer */
    NlmCmAllocator__vtbl *m_vtbl_p;     /* Pointer to above to make some macros work transparently */
    void *m_clientData_p;       /* For use by client */
    void *m_arena_info;              /* Arena pointer */

};

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
extern NlmCmAllocator *NlmCmAllocator__ctor(NlmCmAllocator * self);

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
extern void NlmCmAllocator__config(NlmCmAllocator * self, NlmCmAllocator__vtbl * vtbl);

/* Summary
   Destruct a NlmCmAllocator.

   Description
   Destroy a no longer needed allocator. This does <B>not</B> free any memory
   allocated by this allocator. That must be completed before calling this
   function.

   Parameters
   @param self The allocator to clean up after.
*/
extern void NlmCmAllocator__dtor(NlmCmAllocator * self);

/* Non-virtual base class dtor method */
extern void NlmCmAllocator__dtor_body(NlmCmAllocator *);

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
void *NlmCmAllocator__malloc(NlmCmAllocator * self, size_t size);

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
void *NlmCmAllocator__calloc(NlmCmAllocator * self, size_t cnt, size_t size);

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
void *NlmCmAllocator__resize(NlmCmAllocator * self, void *memblk, size_t newSize, size_t oldSize);

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
void NlmCmAllocator__free(NlmCmAllocator * self, void *memblk);

#ifndef __doxygen__

#define NlmCmAllocator__dtor(self)      (((self)->m_vtbl.m_dtor)(self))

#endif                          /*## __doxygen__ */

#ifndef __doxygen__
extern void NlmCmAllocator__Verify(void);
extern void NlmCmAllocator__pvt_Verify(NlmCmAllocator * alloc);
#endif                          /*## __doxygen__ */

#ifdef __cplusplus
}
#endif

#endif
/*[]*/
