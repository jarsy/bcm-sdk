/*
 * $Id: nlmcmarena.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */



#ifndef INCLUDED_NLMCMARENA_H
#define INCLUDED_NLMCMARENA_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmerrorcodes.h>
#include <nlmcmallocator.h>
#include <nlmcmexterncstart.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

/*Minimum size of the Arena*/
#define NLM_DEFAULT_ARENA_SIZE_IN_BYTES (400 * 1024)

/*
Initialize the Arena and allocate space for the first arena from system memory
*/
NlmErrNum_t NlmCmArena__Init(NlmCmAllocator *alloc_p, nlm_u32 arenaSizeInBytes, NlmReasonCode *o_reason_p);

/*
If Arena has sufficient memory, then activate the Arena so that memory allocations can be made by the Arena instead 
of system memory.
If Arena can't get sufficient memory then indicate a failure.
*/
NlmErrNum_t NlmCmArena__ActivateArena(NlmCmAllocator *alloc_p, NlmReasonCode *o_reason_p);

/*
Deactivate the Arena so that memory allocations can't be made by the Arena
If there are extra arenas that are completely free then return them to the system memory
*/
NlmErrNum_t NlmCmArena__DeactivateArena(void);

/*
Allocate space from the Arena memory if the Arena is active
*/
void* NlmCmArena__Allocate(nlm_u32 size);

/*
If the memory has been allocated from the Arena, then free it back to the Arena and return true
If the memory has not been allocated from the Arena, then return false
*/
NlmBool NlmCmArena__FreeIfArenaMemory(void *ptr);

/*
Release the Arena memory back to the system
*/
void NlmCmArena__Destroy(void);
#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif
#endif


