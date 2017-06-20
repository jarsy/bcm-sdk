/*
 * $Id: nlmcmtwolevelhashtbl.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 

#ifndef INCLUDED_NLMCMTWOLEVELHASHTBL_H
#define INCLUDED_NLMCMTWOLEVELHASHTBL_H

#ifndef NLMPLATFORM_BCM
#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmerrorcodes.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#endif

typedef struct NlmCmTwoLevelHashTbl NlmCmTwoLevelHashTbl;

typedef void (*NlmCmTwoLevelHashTbl__DestroyElem_t)(NlmCmAllocator* arg,
                                                     void* element_p);



struct NlmCmTwoLevelHashTbl
{
    void            **m_firstLevel_pp;
    nlm_u32         m_nrOfLevel2Elems;
    nlm_u32         m_totalSize;
    NlmCmAllocator  *m_alloc_p;
    NlmCmTwoLevelHashTbl__DestroyElem_t m_destroyElemFn_p;
};

#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncstart.h"
#else
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif
    
void* NlmCmTwoLevelHashTbl__Init(
                    nlm_u32 totalSize,
                    nlm_u32 nrOfSecondLevelElems,
                    NlmCmAllocator *alloc_p,
                    NlmCmTwoLevelHashTbl__DestroyElem_t destroyElemFn_p,
                    NlmReasonCode *o_reason_p);

NlmErrNum_t NlmCmTwoLevelHashTbl__Insert(
                    NlmCmTwoLevelHashTbl* self_p, 
                    nlm_u32 index,
                    void* insertPtr,
                    NlmReasonCode *o_reason_p);

void* NlmCmTwoLevelHashTbl__GetMember(
                    NlmCmTwoLevelHashTbl* self_p, 
                    nlm_u32 index,
                    NlmReasonCode *o_reason_p);

void NlmCmTwoLevelHashTbl__Destroy(
                    NlmCmTwoLevelHashTbl* self_p                                
                                );
#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

#endif

