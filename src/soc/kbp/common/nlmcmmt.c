/*
 * $Id: nlmcmmt.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#if defined NLM_MT_OLD || defined NLM_MT

#include "nlmcmmt.h"

#if defined  NLM_POSIX || defined NLMPLATFORM_BCM
__thread nlm_u32 threadID;
#endif

#if defined NLM_NETOS || defined NLMPLATFORM_BCM
NlmBool isNlmCmMtInitDone = NlmFalse;
nlm_u32 threadIdArray[NLMNS_MT_MAXNUM_THREADS ] = {-1, };
#endif

NlmErrNum_t
NlmCmMt__RegisterThread(
        nlm_u32 thNum,
        nlm_u32 cpuId,
        NlmReasonCode* o_reason)
{
    NlmReasonCode dummy;

    if(o_reason == NULL)
        o_reason = &dummy;

    if(thNum > NLMNS_MT_MAXNUM_THREADS)
    {
        *o_reason = NLMRSC_MT_INVALID_THREAD_ID;
        return NLMERR_FAIL;
    }

#if defined NLM_NETOS || defined NLMPLATFORM_BCM

    /* Initialize the thread id array if not done */
    if(isNlmCmMtInitDone == NlmFalse)
    {
        nlm_u32 i = 0;
        for(i = 0; i < NLMNS_MT_MAXNUM_THREADS ; i++)
            threadIdArray[i] = -1;
        isNlmCmMtInitDone = NlmTrue;
    }
    
    if(threadIdArray[thNum] != -1)
    {
        *o_reason = NLMRSC_MT_THREAD_ALREADY_REGISTERED;
        return NLMERR_FAIL;
    }

    threadIdArray[thNum] = cpuId;
#else
    threadID = thNum;
#endif

    *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
    
}


#endif /* defined NLM_MT_OLD || defined NLM_MT */


