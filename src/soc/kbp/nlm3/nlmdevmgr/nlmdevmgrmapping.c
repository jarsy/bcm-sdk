/*
 * $Id: nlmdevmgrmapping.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */




#include "nlmarch.h"
#include "nlmdevmgr.h"

#ifdef NLM_12K_11K

#include<nlmdevmgr/nlmdevmgr_shadow.h>
#include<nlmdevmgr/nlmdevmgr.h>

 Nlm11kDevMgr* kbp_dm_pvt_11k_init(
    NlmCmAllocator      *alloc_p,
    void                    *xpt_p,
    NlmDevType          devType,
    NlmDevOperationMode  operMode,
    NlmReasonCode       *o_reason
    )
{
    Nlm11kDevMgr* devMgr_p = NULL;

    (void)devType;

    devMgr_p = Nlm11kDevMgr__create(alloc_p, xpt_p, operMode, o_reason);

    return devMgr_p;
}


NlmErrNum_t
kbp_dm_pvt_11k_map_ltrs(
    NlmDevLtrRegType regType,
    Nlm11kDevLtrRegType *o_11kRegType,
    NlmReasonCode       *o_reason
    )
{
    NlmErrNum_t errNum = NLMERR_OK;

    switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
            *o_11kRegType = regType;
            break;

        case NLMDEV_BLOCK_SELECT_2_LTR:
        case NLMDEV_BLOCK_SELECT_3_LTR:
            errNum = NLMERR_FAIL;
            break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
            *o_11kRegType =  NLM11KDEV_PARALLEL_SEARCH_0_LTR +
                                    (regType -NLMDEV_PARALLEL_SEARCH_0_LTR);
            break;

        case NLMDEV_PARALLEL_SEARCH_4_LTR:
        case NLMDEV_PARALLEL_SEARCH_5_LTR:
        case NLMDEV_PARALLEL_SEARCH_6_LTR:
        case NLMDEV_PARALLEL_SEARCH_7_LTR:
            errNum = NLMERR_FAIL;
            break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
            *o_11kRegType = NLM11KDEV_SUPER_BLK_KEY_MAP_LTR;
            break;

        case NLMDEV_EXT_CAPABILITY_REG_0_LTR:
            *o_11kRegType = NLM11KDEV_MISCELLENEOUS_LTR;
            break;
        case NLMDEV_EXT_CAPABILITY_REG_1_LTR:
            errNum = NLMERR_FAIL;
            break;

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
            *o_11kRegType =  NLM11KDEV_KEY_0_KCR_0_LTR +
                                    (regType -NLMDEV_KEY_0_KCR_0_LTR);
            break;

        case NLMDEV_OPCODE_EXT_LTR:
            errNum = NLMERR_FAIL;
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
        case NLMDEV_RANGE_INSERTION_1_LTR:
            *o_11kRegType =  NLM11KDEV_RANGE_INSERTION_0_LTR +
                                    (regType -NLMDEV_RANGE_INSERTION_0_LTR);
            break;

        case NLMDEV_SS_LTR:
            *o_11kRegType =  NLM11KDEV_SS_LTR;
            break;

        case NLMDEV_LTR_REG_END:
        default:
            errNum = NLMERR_FAIL;
            break;
    }

    if(errNum == NLMERR_FAIL)
        *o_reason = NLMRSC_INVALID_REG_ADDRESS;

    return errNum;
}


NlmErrNum_t
kbp_dm_pvt_11k_map_global_reg(
    NlmDevGlobalRegType regType,
    Nlm11kDevGlobalRegType *o_11kRegType,
    NlmReasonCode       *o_reason
    )
{
    NlmErrNum_t errNum = NLMERR_OK;

    switch(regType)
    {
        case NLMDEV_DEVICE_ID_REG:  /* Read only Reg */
        case NLMDEV_DEVICE_CONFIG_REG:
        case NLMDEV_ERROR_STATUS_REG: /* Read only Reg; Reading of this register clears the set bits */
        case NLMDEV_ERROR_STATUS_MASK_REG:
            *o_11kRegType = regType;
            break;

        case NLMDEV_SOFT_SCAN_WRITE_REG:
            errNum = NLMERR_FAIL;
            break;

        case NLMDEV_SOFT_ERROR_FIFO_REG:
        case NLMDEV_ADV_FEATURE_SOFT_ERROR_REG:
            *o_11kRegType = NLM11KDEV_DATABASE_SOFT_ERROR_FIFO_REG +
                        (regType - NLMDEV_SOFT_ERROR_FIFO_REG) ;
            break;

        case NLMDEV_LPT_ENABLE_REG:
            errNum = NLMERR_FAIL;
            break;

        case NLMDEV_SCRATCH_PAD0_REG:
        case NLMDEV_SCRATCH_PAD1_REG:
        case NLMDEV_RESULT0_REG:
        case NLMDEV_RESULT1_REG:
            *o_11kRegType = NLM11KDEV_SCRATCH_PAD0_REG +
                        (regType - NLMDEV_SCRATCH_PAD0_REG) ;
            break;

        case NLMDEV_RESULT2_REG:
        case NLMDEV_UDA_SOFT_ERROR_COUNT_REG:
        case NLMDEV_UDA_SOFT_ERROR_FIFO_REG:
        case NLMDEV_UDA_CONFIG_REG:
        case NLMDEV_GLOBALREG_END:
        default:
            errNum = NLMERR_FAIL;
            break;
    }

    if(errNum == NLMERR_FAIL)
        *o_reason = NLMRSC_INVALID_REG_ADDRESS;

    return errNum;

}


NlmErrNum_t
kbp_dm_pvt_11k_map_cmp_result(
    Nlm11kDevCmpRslt *cmpResult11k,
    NlmDevCmpResult  *o_cmpResult12k,
    NlmReasonCode       *o_reason
    )
{
    nlm_u32 i = 0;
    *o_reason = NLMRSC_REASON_OK;

    for(i = 0; i < NLM_MAX_NUM_RESULTS; i++)
    {
        o_cmpResult12k->m_resultValid[i] = NLMDEV_RESULT_VALID;
        o_cmpResult12k->m_respType[i] = NLMDEV_INDEX_AND_NO_AD;

        o_cmpResult12k->m_hitOrMiss[i] = cmpResult11k->m_hitOrMiss[i];
        o_cmpResult12k->m_hitIndex[i] = cmpResult11k->m_hitIndex[i];
        o_cmpResult12k->m_hitDevId[i] = cmpResult11k->m_hitDevId[i];
    }

    return NLMERR_OK;

}

void*
kbp_dm_pvt_11k_map_create_shadow_st(
    NlmCmAllocator *alloc_p)
{
    /* Create memory for ST. */

    Nlm11kDevShadowST *shadowSt_p = NULL;

    shadowSt_p = (Nlm11kDevShadowST*) NlmCmAllocator__calloc(alloc_p,
                                                    NLM11KDEV_SS_NUM,
                                                    sizeof(Nlm11kDevShadowST) );

    return (void*)shadowSt_p;
}

#endif
