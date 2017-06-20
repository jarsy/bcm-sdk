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
/* 
 * $Id: nlmdevmgr_refapp.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2007 Broadcom Corp.
 * All Rights Reserved.$
 */

#include "nlmdevmgr_refapp.h"
#include "nlmcmdevice.h"

#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

#define NLM_DEVMGR_REFAPP_CMP1       0
#define NLM_DEVMGR_REFAPP_CMP2       1

typedef enum NlmDevMgrRefApp_XptMode_e
{
    NLM_SIMULATION_MODE,
    NLM_FPGAXPT_MODE,
    NLM_XLPXPT_MODE,
    NLM_BCM_CALADAN3_MODE
} NlmDevMgrRefApp_XptMode;


typedef struct NlmDevMgrRefAppBlkToRsltPortMap
{
    NlmDev_parallel_search_t m_psNum;    /* Specifies result port number mapped to the blk */
} NlmDevMgrRefAppBlkToRsltPortMap;

typedef struct NlmDevMgrRefAppSuperBlkToKeyMap
{
    NlmDev_key_t m_keyNum;    /* Specifies key number mapped to the super blk */
} NlmDevMgrRefAppSuperBlkToKeyMap;

typedef struct NlmDevMgrRefAppKeyConstructMap
{
    nlm_u8 m_startByte[NLMDEV_NUM_OF_KCR_PER_KEY * NLMDEV_NUM_OF_SEGMENTS_PER_KCR]; /* Specifies the start byte of each segment of Key Construction*/
    nlm_u8 m_numOfBytes[NLMDEV_NUM_OF_KCR_PER_KEY * NLMDEV_NUM_OF_SEGMENTS_PER_KCR]; /* Specifies number of bytes each segment is */
} NlmDevMgrRefAppKeyConstructMap;

typedef struct NlmDevMgrRefAppSrchAttrs
{
    NlmDevDisableEnable m_blkEnable[NLMDEV_NUM_ARRAY_BLOCKS];
    NlmDevMgrRefAppBlkToRsltPortMap m_rsltPortBlkMap[NLMDEV_NUM_ARRAY_BLOCKS]; /* Contains the srch attributes associated with each blk */
    NlmDevMgrRefAppSuperBlkToKeyMap m_keySBMap[NLMDEV_NUM_SUPER_BLOCKS]; /* Contains Super Blk to Key mapping for each super blk */
    NlmDevMgrRefAppKeyConstructMap m_keyConstructMap[NLMDEV_NUM_KEYS]; /* Contains the Key Construction mapping for each key */
    nlm_u8 m_bmrNum[NLMDEV_NUM_PARALLEL_SEARCHES];   /* Specifies BMR number used for each srch */ 
}NlmDevMgrRefAppSrchAttrs;

/* NlmDevMgrRefApp_InitializeBlk function enables a block and initializes it
to the specified block width by writing to Block Configuration Register of the
specified Block */
NlmErrNum_t NlmDevMgrRefApp_InitializeBlk(NlmDev *dev_p, 
                                             nlm_u8 blkNum,
                                             nlm_u16 blkWidth
                                             )
{
    NlmDevShadowDevice *shadowDev_p;
    NlmDevBlockConfigReg *blkConfigReg_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    /* Get the shadow memory of blk config register for the specified blkNum */
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);
    blkConfigReg_p = &shadowDev_p->m_arrayBlock[blkNum].m_blkConfig;

    /* Initialize the Blk Config Reg Data */
    blkConfigReg_p->m_blockEnable = NLMDEV_ENABLE;
    switch(blkWidth)
    {
        case 80:
            blkConfigReg_p->m_blockWidth = NLMDEV_BLK_WIDTH_80;
            break;

        case 160:
            blkConfigReg_p->m_blockWidth = NLMDEV_BLK_WIDTH_160;
            break;

        case 320:
            blkConfigReg_p->m_blockWidth = NLMDEV_BLK_WIDTH_320;
            break;

        case 640:
            blkConfigReg_p->m_blockWidth = NLMDEV_BLK_WIDTH_640;
            break;
    }

    /* Invoke the API which writes to specified Blk Register */
    if((errNum = NlmDevMgr__BlockRegisterWrite(dev_p,
                                                  blkNum,
                                                  NLMDEV_BLOCK_CONFIG_REG,
                                                  blkConfigReg_p,
                                                  &reasonCode)) != NLMERR_OK) 
    {    
        NlmCm__printf("\n\n Initialization of Block Number %d failed", blkNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__BlockRegisterWrite', Reason Code -- %d", reasonCode);
        return errNum;
    }

    return NLMERR_OK;
}

/* NlmDevMgrRefApp_InitializeBmr function initializes the Global mask used for the block
by writing to the specified Block Mask Register of the specified Block. 
Note: Application can initilize upto 5 different BMRs per block;
This function assumes that application has initialized the block to an appropriate 
Block Width
*/
NlmErrNum_t NlmDevMgrRefApp_InitializeBmr(NlmDev *dev_p, 
                                             nlm_u8 blkNum,
                                             nlm_u8 bmrNum,
                                             nlm_u8 *bmrData
                                             )
{
    NlmDevShadowDevice *shadowDev_p;
    NlmDevBlockMaskReg *blkMaskReg_p;
    NlmDevBlockWidth blkWidth;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_32 segNum;
    nlm_32 numberOf80bSegments = 0;
    NlmDevBlockRegType regType;

    /* Get the block width of the block from shadow memory of blk config register for the specified blkNum */
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);
    blkWidth = shadowDev_p->m_arrayBlock[blkNum].m_blkConfig.m_blockWidth;
        
    /* Initialize numberOf80bSegments based on Blk width */
    switch(blkWidth)
    {
        case NLMDEV_BLK_WIDTH_80:
            numberOf80bSegments = 1;
            break;
           
        case NLMDEV_BLK_WIDTH_160:
            numberOf80bSegments = 2;
            break;

        case NLMDEV_BLK_WIDTH_320:
            numberOf80bSegments = 4;;
            break;

        case NLMDEV_BLK_WIDTH_640:
            numberOf80bSegments = 8;
            break;
        
        default:
            NlmCmAssert(0, "Invalid Blk Width");
            NlmCm__printf("\n\n Initialization of Block Mask %d of Block Number %d failed", bmrNum, blkNum);
            NlmCm__printf("\n Block configured to invalid block width");
            return NLMERR_FAIL;
    }

    for(segNum = 0; segNum < numberOf80bSegments; segNum++)
    {
        if(segNum == NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR)          
        {
            /* For 640b blocks (numberOf80bSegments == 8); a pair of BMR is used;
            Hence when segNum == NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR,
            next BMR Number is used*/      
            bmrNum = (bmrNum + 1)%NLMDEV_NUM_OF_BMRS_PER_BLK;                          
        }
        
         /* Get the shadow memory of blk mask register for the specified bmrNum, segNum and blkNum */
        blkMaskReg_p = &shadowDev_p->m_arrayBlock[blkNum].m_bmr[bmrNum][segNum%NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR];

        /* Copy the BMR Data */
        NlmCm__memcpy(blkMaskReg_p->m_mask, 
                      bmrData + (numberOf80bSegments - (segNum + 1)) * NLMDEV_REG_LEN_IN_BYTES, 
                      NLMDEV_REG_LEN_IN_BYTES);
        
        regType = NLMDEV_BLOCK_MASK_0_0_REG + (bmrNum * NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR) 
                                                + (segNum % NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR);
        /* Invoke the API which writes to specified Blk Register */
        if((errNum = NlmDevMgr__BlockRegisterWrite(dev_p,
                                                      blkNum, 
                                                      regType,
                                                      blkMaskReg_p,
                                                      &reasonCode)) != NLMERR_OK)  
        {    
            NlmCm__printf("\n\n Initialization of Block Mask Number %d, Segment Number %d of Block Number %d failed",
                                bmrNum, segNum % NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR, blkNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__BlockRegisterWrite', Reason Code -- %d", reasonCode);
            return errNum;
        }      
    }

    return NLMERR_OK;
}

#define NLM_DEVMGR_REFAPP_NUM_OF_BLK_SEL_LTR   (NLMDEV_NUM_ARRAY_BLOCKS/64)
#define NLM_DEVMGR_REFAPP_NUM_OF_PS_LTR   (NLMDEV_NUM_ARRAY_BLOCKS/32)
/* NlmDevMgrRefApp_ConfigSrch function initializes the specified Search 
Attributes by writing to the Logical Table Registers of the specified LTR Number. 
*/
NlmErrNum_t NlmDevMgrRefApp_ConfigSrch(NlmDev *dev_p, 
                                          nlm_u8 ltrNum,
                                          NlmDevMgrRefAppSrchAttrs *srchAttrs
                                          )
{
    NlmDevShadowDevice *shadowDev_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_32 kcrNum;
    nlm_32 keyNum;
    nlm_32 blkSelectRegNum;
    nlm_32 psRegNum;
    NlmDevBlkSelectReg *blkSelectReg_p;
    NlmDevSuperBlkKeyMapReg *superblkKeySelectReg_p;
    NlmDevParallelSrchReg *parallelSrchReg_p;
    NlmDevMiscelleneousReg   *miscReg_p;
    NlmDevKeyConstructReg *keyConstructReg_p;

    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);

    for(blkSelectRegNum = 0; blkSelectRegNum < NLM_DEVMGR_REFAPP_NUM_OF_BLK_SEL_LTR; blkSelectRegNum++)
    {
        /* Get the shadow memory of appropriate Block Select Register for the specified LTR Number */        
        blkSelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[blkSelectRegNum];

        /* Copy the Blk Select Register Data */
        NlmCm__memcpy(blkSelectReg_p->m_blkEnable, 
                      &srchAttrs->m_blkEnable[blkSelectRegNum * (NLMDEV_NUM_ARRAY_BLOCKS/2)], 
                      (NLMDEV_NUM_ARRAY_BLOCKS/2) * sizeof(NlmDevDisableEnable));

        /* Invoke the API which writes to specified LTR Register */
        if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                             ltrNum, 
                                                             NLMDEV_BLOCK_SELECT_0_LTR + blkSelectRegNum,
                                                             blkSelectReg_p,
                                                             &reasonCode)) != NLMERR_OK)  
        {    
            NlmCm__printf("\n\n Initialization of Block Select %d Register of LTR Number %d failed", blkSelectRegNum, ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);
            return errNum;
        }
    }

    /* Get the shadow memory of Super Block Key Select Register for the specified LTR Number */        
    superblkKeySelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_superBlkKeyMap;
    
    /* Copy the Super Blk Key Select Register Data */
    NlmCm__memcpy(superblkKeySelectReg_p->m_keyNum, 
                  srchAttrs->m_keySBMap, 
                  NLMDEV_NUM_SUPER_BLOCKS * sizeof(NlmDevKeyNum));

    /* Invoke the API which writes to specified LTR Register */
    if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                         ltrNum, 
                                                         NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                                         superblkKeySelectReg_p,
                                                         &reasonCode)) != NLMERR_OK)  
    {    
        NlmCm__printf("\n\n Initialization of Super Block Key Select Register of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);
        return errNum;
    }

    for(psRegNum = 0; psRegNum < NLM_DEVMGR_REFAPP_NUM_OF_PS_LTR; psRegNum++)
    {
        /* Get the shadow memory of appropriate Parallel Search Register for the specified LTR Number */        
        parallelSrchReg_p = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[psRegNum];

        /* Copy the Parallel Search Register Data */
        NlmCm__memcpy(parallelSrchReg_p->m_psNum, 
                      &srchAttrs->m_rsltPortBlkMap[psRegNum * (NLMDEV_NUM_ARRAY_BLOCKS/4)], 
                      (NLMDEV_NUM_ARRAY_BLOCKS/4) * sizeof(NlmDevParallelSrchNum));

        /* Invoke the API which writes to specified LTR Register */
        if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                             ltrNum, 
                                                             NLMDEV_PARALLEL_SEARCH_0_LTR + psRegNum,
                                                             parallelSrchReg_p,
                                                             &reasonCode)) != NLMERR_OK)  
        {    
            NlmCm__printf("\n\n Initialization of Parallel Search %d Register of LTR Number %d failed", psRegNum, ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);
            return errNum;
        }
    }

    /* Since by default Range Engine is disabled there is no need to initialize Range Insertion LTR Registers 
    and Range Extraction Fields of Miscelleneous LTR Register */
    
    miscReg_p = &shadowDev_p->m_ltr[ltrNum].m_miscelleneous;
    miscReg_p->m_numOfValidSrchRslts = 0; /* All 4 results are valid */      
    NlmCm__memcpy(miscReg_p->m_bmrSelect, 
                  srchAttrs->m_bmrNum, 
                  NLMDEV_NUM_PARALLEL_SEARCHES * sizeof(nlm_u8));

    /* Invoke the API which writes to specified LTR Register */
    if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                         ltrNum, 
                                                         NLMDEV_MISCELLENEOUS_LTR,
                                                         miscReg_p,
                                                         &reasonCode)) != NLMERR_OK) 
    {    
        NlmCm__printf("\n\n Initialization of Miscelleneous Register of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);
        return errNum;
    }

    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
    {
        for(kcrNum = 0; kcrNum < NLMDEV_NUM_OF_KCR_PER_KEY; kcrNum++)
        {
            /* Get the shadow memory of appropriate Key Construction Register for the specified LTR Number */        
            keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[keyNum * NLMDEV_NUM_OF_KCR_PER_KEY + kcrNum];

            /* Copy the Key Construction Register Data */
            NlmCm__memcpy(keyConstructReg_p->m_startByteLoc, 
                          &srchAttrs->m_keyConstructMap[keyNum].m_startByte[kcrNum * NLMDEV_NUM_OF_SEGMENTS_PER_KCR],
                          NLMDEV_NUM_OF_SEGMENTS_PER_KCR * sizeof(nlm_u8));

            NlmCm__memcpy(keyConstructReg_p->m_numOfBytes, 
                          &srchAttrs->m_keyConstructMap[keyNum].m_numOfBytes[kcrNum * NLMDEV_NUM_OF_SEGMENTS_PER_KCR],
                          NLMDEV_NUM_OF_SEGMENTS_PER_KCR * sizeof(nlm_u8));

            /* Invoke the API which writes to specified LTR Register */
            if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                                 ltrNum, 
                                                                 NLMDEV_KEY_0_KCR_0_LTR + keyNum * NLMDEV_NUM_OF_KCR_PER_KEY + kcrNum,
                                                                 keyConstructReg_p,
                                                                 &reasonCode)) != NLMERR_OK)  
            {    
                NlmCm__printf("\n\n Initialization of Key Construction %d Register of Key Number %d of LTR Number %d failed", kcrNum, keyNum, ltrNum);
                NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);
                return errNum;
            }
        }       
    }
    return NLMERR_OK;
}                                         

/* NlmDevMgrRefApp_AddEntry function writes Database Entry to 
specified Blk Num and startAddress;
This function assumes that application has initialized the block to an appropriate 
Block Width and writes database entries in terms of Blk Width
*/
NlmErrNum_t NlmDevMgrRefApp_AddEntry(NlmDev *dev_p, 
                                        nlm_u8 blkNum,
                                        nlm_u16 startAddress,
                                        nlm_u8 *data,
                                        nlm_u8 *mask
                                        )
{
    NlmDevShadowDevice *shadowDev_p;    
    NlmDevBlockWidth blkWidth;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_u16 segNum;
    nlm_u16 numberOf80bSegments = 0;
    NlmDevABEntry *abEntry_p;

    /* Get the block width of the block from shadow memory of blk config register for the specified blkNum */
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);
    blkWidth = shadowDev_p->m_arrayBlock[blkNum].m_blkConfig.m_blockWidth;
        
    /* Initialize numberOf80bSegments based on Blk width */
    switch(blkWidth)
    {
        case NLMDEV_BLK_WIDTH_80:
            numberOf80bSegments = 1;
            break;
           
        case NLMDEV_BLK_WIDTH_160:
            numberOf80bSegments = 2;
            break;

        case NLMDEV_BLK_WIDTH_320:
            numberOf80bSegments = 4;;
            break;

        case NLMDEV_BLK_WIDTH_640:
            numberOf80bSegments = 8;
            break;
        
        default:
            NlmCmAssert(0, "Invalid Blk Width");
            NlmCm__printf("\n\n Addition of AB Entry %d of Block Number %d failed", startAddress, blkNum);
            NlmCm__printf("\n Block configured to invalid block width");
            return NLMERR_FAIL;
    }

    for(segNum = 0; segNum < numberOf80bSegments; segNum++)
    {        
        /* Get the shadow memory of ab Entry for the specified entryNum, segNum and blkNum */
        abEntry_p = &shadowDev_p->m_arrayBlock[blkNum].m_abEntry[startAddress + segNum];
        NlmCm__memcpy(abEntry_p->m_data , 
                      data + (numberOf80bSegments - (segNum + 1)) * NLMDEV_AB_WIDTH_IN_BYTES,
                      NLMDEV_AB_WIDTH_IN_BYTES);

        NlmCm__memcpy(abEntry_p->m_mask,
                      mask + (numberOf80bSegments - (segNum + 1)) * NLMDEV_AB_WIDTH_IN_BYTES,
                      NLMDEV_AB_WIDTH_IN_BYTES);

        abEntry_p->m_vbit = 1;
        
        /* Invoke the API which writes specified AB Entry*/
        if((errNum = NlmDevMgr__ABEntryWrite(dev_p,
                                                blkNum, 
                                                startAddress + segNum,
                                                abEntry_p,
                                                NLMDEV_DM,
                                                &reasonCode)) != NLMERR_OK)  
        {    
            NlmCm__printf("\n\n Addition of AB Entry %d of Block Number %d failed", startAddress + segNum, blkNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__ABEntryWrite', Reason Code -- %d", reasonCode);
            return errNum;
        }       
    }

    return NLMERR_OK;
}

/* NlmDevMgrRefApp_DeleteEntry function deletes Database Entries of
specified Blk Num and startAddress;
This function assumes that application has initialized the block to an appropriate 
Block Width and deletes database entries in terms of Blk Width
*/
NlmErrNum_t NlmDevMgrRefApp_DeleteEntry(NlmDev *dev_p, 
                                           nlm_u8 blkNum,
                                           nlm_u16 startAddress
                                           )
{
    NlmDevShadowDevice *shadowDev_p;    
    NlmDevBlockWidth blkWidth;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_u16 segNum;
    nlm_u16 numberOf80bSegments = 0;    

    /* Get the block width of the block from shadow memory of blk config register for the specified blkNum */
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);
    blkWidth = shadowDev_p->m_arrayBlock[blkNum].m_blkConfig.m_blockWidth;
        
    /* Initialize numberOf80bSegments based on Blk width */
    switch(blkWidth)
    {
        case NLMDEV_BLK_WIDTH_80:
            numberOf80bSegments = 1;
            break;
           
        case NLMDEV_BLK_WIDTH_160:
            numberOf80bSegments = 2;
            break;

        case NLMDEV_BLK_WIDTH_320:
            numberOf80bSegments = 4;;
            break;

        case NLMDEV_BLK_WIDTH_640:
            numberOf80bSegments = 8;
            break;
        
        default:
            NlmCmAssert(0, "Invalid Blk Width");
            NlmCm__printf("\n\n Deletion of AB Entry %d of Block Number %d failed", startAddress, blkNum);
            NlmCm__printf("\n Block configured to invalid block width");
            return NLMERR_FAIL;
    }

    for(segNum = 0; segNum < numberOf80bSegments; segNum++)
    {       
        /* Invoke the API which invalidates specified AB Entry*/
        if((errNum = NlmDevMgr__ABEntryInvalidate(dev_p,
                                                     blkNum, 
                                                     startAddress + segNum,
                                                     &reasonCode)) != NLMERR_OK)  
        {    
            NlmCm__printf("\n\n Deletion of AB Entry %d of Block Number %d failed", startAddress + segNum, blkNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__ABEntryInvalidate', Reason Code -- %d", reasonCode);
            return errNum;
        }       
       
        /* Invalidate the ab Entry in the shadow memory*/
        shadowDev_p->m_arrayBlock[blkNum].m_abEntry[startAddress + segNum].m_vbit = 0; 
    }

    return NLMERR_OK;
}

/* NlmDevMgrRefApp_PerformCompare function performs Compare operation
based on compare type; Compare Data can be upto 640b and is written to CB memory
starting from cbStartAddr;
*/
NlmErrNum_t NlmDevMgrRefApp_PerformCompare(NlmDevMgr *devMgr_p, 
                                              nlm_u8 ltrNum,
                                              nlm_u32 compareType,
                                              nlm_u16 cbStartAddr,
                                              nlm_u8 cbDatalen,
                                              nlm_u8 *cmpData,
                                              NlmDevCmpRslt *cmpRslts
                                              )
{
    NlmDevCtxBufferInfo cbInfo;
    NlmErrNum_t errNum;
    NlmReasonCode reason;
        
    
    

    cbInfo.m_cbStartAddr = cbStartAddr;
    cbInfo.m_datalen = cbDatalen;    
    NlmCm__memcpy(cbInfo.m_data, cmpData, cbDatalen);

    
    if(compareType == NLM_DEVMGR_REFAPP_CMP1)
    {
        /* Invoke Compare1 API to perform Compare Operation */
        if((errNum = NlmDevMgr__Compare1(devMgr_p,
                                            ltrNum,
                                            &cbInfo,
                                            cmpRslts,
                                            &reason
                                            )) != NLMERR_OK)
        {
            NlmCm__printf("\n\n Compare1 operation for LTR Number %d failed", ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__Compare1', Reason Code -- %d", reason);
            return errNum;
        }
    }
    else
    {
        /* Invoke Compare2 API to perform Compare Operation */
        if((errNum = NlmDevMgr__Compare2(devMgr_p,
                                            ltrNum,
                                            &cbInfo,
                                            cmpRslts,
                                            &reason
                                            )) != NLMERR_OK)
        {
            NlmCm__printf("\n\n Compare2 operation for LTR Number %d failed", ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__Compare2', Reason Code -- %d", reason);
            return errNum;
        }
    }
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts function verifies the contents of result
register with the compare operation results */
NlmErrNum_t NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(NlmDevMgr *devMgr_p, 
                                                        NlmDevCmpRslt *cmpRslts
                                                        )
{    
    NlmDevResultReg rsltRegData;
    NlmReasonCode reason;
    NlmErrNum_t errNum;
    nlm_u8 psNum;

    /* Read the Result Register 0 which contains results for result port #0 and #1 */ 
    if((errNum = NlmDevMgr__GlobalRegisterRead(devMgr_p->m_devList_pp[0], 
                                                  NLMDEV_RESULT0_REG,
                                                  &rsltRegData,
                                                  &reason)) != NLMERR_OK)
    {
         NlmCm__printf("\n\n Result Register Read operation failed");
         NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__GlobalRegisterRead', Reason Code -- %d", reason);
         return errNum;
    }

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES/2; psNum++)
    {
        if(rsltRegData.m_hitOrMiss[psNum] == cmpRslts->m_hitOrMiss[psNum])
        {
            if(rsltRegData.m_hitOrMiss[psNum] == (Nlm11kDevMissHit)NLMDEV_HIT)
            {
                /* If both result register hit and compare results gives hit verify the hit index */
                if(rsltRegData.m_hitAddress[psNum] != cmpRslts->m_hitIndex[psNum])
                {
                    NlmCm__printf("\n\n Compare Results in Result Register Differ for PS Num %d ", psNum);
                    NlmCm__printf("\n\n Result Reg Hit at Index %d, (Compare Rslt Hit at Index %d)",
                        rsltRegData.m_hitAddress[psNum], cmpRslts->m_hitIndex[psNum]);
                    return NLMERR_FAIL;
                }
            }
        }
        else
        {
            if(rsltRegData.m_hitOrMiss[psNum] == (Nlm11kDevMissHit)NLMDEV_HIT)
            {
                /* If result register gives miss and compare results gives hit */ 
                NlmCm__printf("\n\n Compare Results in Result Register Differ for PS Num %d ", psNum);
                NlmCm__printf("\n\n Result Reg Gives Miss, (Compare Rslt Gives Hit at Index %d)",
                    cmpRslts->m_hitIndex[psNum]);
                return NLMERR_FAIL;
            }
            else
            {
                /* If result register gives hit and compare results gives miss */ 
                NlmCm__printf("\n\n Compare Results in Result Register Differ for PS Num %d ", psNum);
                NlmCm__printf("\n\n Result Reg Gives Hit At Index %d, (Compare Rslt Gives Miss)",
                    rsltRegData.m_hitAddress[psNum]);
                return NLMERR_FAIL;
            }
        }
    }

    /* Read the Result Register 1 which contains results for result port #2 and #3 */ 
    if((errNum = NlmDevMgr__GlobalRegisterRead(devMgr_p->m_devList_pp[0], 
                                                  NLMDEV_RESULT1_REG,
                                                  &rsltRegData,
                                                  &reason)) != NLMERR_OK)
    {
         NlmCm__printf("\n\n Result Register Read operation failed");
         NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__GlobalRegisterRead', Reason Code -- %d", reason);
         return errNum;
    }

    for(psNum = NLMDEV_NUM_PARALLEL_SEARCHES/2; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        if(rsltRegData.m_hitOrMiss[psNum - NLMDEV_NUM_PARALLEL_SEARCHES/2] == cmpRslts->m_hitOrMiss[psNum])
        {
            if(rsltRegData.m_hitOrMiss[psNum - NLMDEV_NUM_PARALLEL_SEARCHES/2] == (Nlm11kDevMissHit)NLMDEV_HIT)
            {
                  /* If both result register hit and compare results gives hit verify the hit index */ 
                if(rsltRegData.m_hitAddress[psNum - NLMDEV_NUM_PARALLEL_SEARCHES/2] != cmpRslts->m_hitIndex[psNum])
                {
                    NlmCm__printf("\n\n Compare Results in Result Register Differ for PS Num %d ", psNum);
                    NlmCm__printf("\n\n Result Reg Hit at Index %d, (Compare Rslt Hit at Index %d)",
                        rsltRegData.m_hitAddress[psNum - NLMDEV_NUM_PARALLEL_SEARCHES/2], 
                        cmpRslts->m_hitIndex[psNum]);
                    return NLMERR_FAIL;
                }
            }           
        }
        else
        {
            if(rsltRegData.m_hitOrMiss[psNum - NLMDEV_NUM_PARALLEL_SEARCHES/2] ==
                                                                (Nlm11kDevMissHit)NLMDEV_HIT)
            {
                /* If result register gives miss and compare results gives hit */ 
                NlmCm__printf("\n\n Compare Results in Result Register Differ for PS Num %d ", psNum);
                NlmCm__printf("\n\n Result Reg Gives Miss, (Compare Rslt Gives Hit at Index %d)",
                    cmpRslts->m_hitIndex[psNum]);
                return NLMERR_FAIL;
            }
            else
            {
                 /* If result register gives hit and compare results gives miss */ 
                NlmCm__printf("\n\n Compare Results in Result Register Differ for PS Num %d ", psNum);
                NlmCm__printf("\n\n Result Reg Gives Hit At Index %d, (Compare Rslt Gives Miss)",
                    rsltRegData.m_hitAddress[psNum - NLMDEV_NUM_PARALLEL_SEARCHES/2]);
                return NLMERR_FAIL;
            }
        }
    }
    return NLMERR_OK;
}



/* NlmDevMgrRefApp_VerifyCompareResults function verifies the compare 
operation results with the expected results
*/
NlmErrNum_t NlmDevMgrRefApp_VerifyCompareResults(
                                              NlmDevCmpRslt *cmpRslts,
                                              NlmDevCmpRslt *expectedCmpRslts
                                              )
{ 
    nlm_32 psNum;

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        if(cmpRslts->m_hitOrMiss[psNum] == expectedCmpRslts->m_hitOrMiss[psNum])
        {            
            if(cmpRslts->m_hitOrMiss[psNum] == (Nlm11kDevMissHit)NLMDEV_MISS) /* If Compare result and expected result is miss then continue*/
                continue;
            else
            {
                if(cmpRslts->m_hitDevId[psNum] != expectedCmpRslts->m_hitDevId[psNum]
                    || cmpRslts->m_hitIndex[psNum] != expectedCmpRslts->m_hitIndex[psNum])
                {
                    /* If Compare Result Index or Dev Id differs from the expected then display error */
                    NlmCm__printf("\n\n Compare Operation Results Differ for PS Num %d", psNum);
                    NlmCm__printf("\n\n Got Hit At Device Id %d, Index %d, (Expected Hit At Device Id %d, Index %d)",
                        cmpRslts->m_hitDevId[psNum], cmpRslts->m_hitIndex[psNum],
                        expectedCmpRslts->m_hitDevId[psNum], expectedCmpRslts->m_hitIndex[psNum]);
                    return NLMERR_FAIL;
                }
            }
        }
        else
        {
            if(cmpRslts->m_hitOrMiss[psNum] == (Nlm11kDevMissHit)NLMDEV_MISS)
            {
                /* If Compare Result is MISS and expected Result is HIT then display error */
                NlmCm__printf("\n\n Compare Operation Results Differ for PS Num %d", psNum);
                NlmCm__printf("\n\n Got Miss (Expected Hit At Device Id %d, Index %d)",
                    expectedCmpRslts->m_hitDevId[psNum], expectedCmpRslts->m_hitIndex[psNum]);
                return NLMERR_FAIL;
            }
            else
            {
                /* If Compare Result is HIT and expected Result is MISS then display error */
                 NlmCm__printf("\n\n Compare Operation Results Differ for PS Num %d", psNum);
                 NlmCm__printf("\n\n Got Hit At Device Id %d, Index %d, (Expected Miss)",
                     cmpRslts->m_hitDevId[psNum], cmpRslts->m_hitIndex[psNum]);
                 return NLMERR_FAIL;
            }
        }           
    }

    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Configure80bBlks configures Block number 0 and 8 to 80b */
NlmErrNum_t NlmDevMgrRefApp_Configure80bBlks(void *dev_p)
{
    NlmErrNum_t errNum;        

    if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, 0, 80)) != NLMERR_OK)
        return errNum;

    if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, 8, 80)) != NLMERR_OK)
        return errNum;   
    
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Configure160bBlks configures Block number 17 and 33 to 160b */
NlmErrNum_t NlmDevMgrRefApp_Configure160bBlks(void *dev_p)
{
   NlmErrNum_t errNum;     
   
   if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, 17, 160)) != NLMERR_OK)
       return errNum;
   
   if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, 33, 160)) != NLMERR_OK)
       return errNum;      
  
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Configure320bBlks configures Block number 50 and 82 to 320b */
NlmErrNum_t NlmDevMgrRefApp_Configure320bBlks(void *dev_p)
{
   NlmErrNum_t errNum;       
   
   if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, 50, 320)) != NLMERR_OK)
       return errNum;
   
   if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, 82, 320)) != NLMERR_OK)
       return errNum;   

    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Configure640bBlks configures Block number 115 to 640b and initializes
BMR #0 to mask odd bits and BMR #2 to mask even bits*/
NlmErrNum_t NlmDevMgrRefApp_Configure640bBlks(void *dev_p)
{
   NlmErrNum_t errNum;    
   nlm_u8 data[8 * NLMDEV_AB_WIDTH_IN_BYTES];
   
   if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, 115, 640)) != NLMERR_OK)
       return errNum;   

   /* Writing to the BMR which will mask the odd bits i.e Bit 1, 3, 5... */
   NlmCm__memset(data, 0xAA, 8 * NLMDEV_AB_WIDTH_IN_BYTES);
   if((errNum = NlmDevMgrRefApp_InitializeBmr(dev_p, 115, 0, data)) != NLMERR_OK)
       return errNum;    
    
   /* Writing to the BMR which will mask the even bits i.e Bit 0, 2, 4... */
   NlmCm__memset(data, 0x55, 8 * NLMDEV_AB_WIDTH_IN_BYTES);
   if((errNum = NlmDevMgrRefApp_InitializeBmr(dev_p, 115, 2, data)) != NLMERR_OK)
       return errNum;  
   
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_ConfigureLtr0Srches function configures 
LTR #0 to perform two 80b parallel searches and two 160b parallel searches */
NlmErrNum_t NlmDevMgrRefApp_ConfigureLtr0Srches(void *dev_p)
{
    NlmErrNum_t errNum;
    NlmDevMgrRefAppSrchAttrs srchAttrs;
  
    NlmCm__memset(&srchAttrs, 0, sizeof(NlmDevMgrRefAppSrchAttrs));
    srchAttrs.m_blkEnable[0] = NLMDEV_ENABLE;
    srchAttrs.m_blkEnable[8] = NLMDEV_ENABLE;
    srchAttrs.m_blkEnable[17] = NLMDEV_ENABLE;
    srchAttrs.m_blkEnable[33] = NLMDEV_ENABLE;
    srchAttrs.m_rsltPortBlkMap[0].m_psNum = NLMDEV_PARALLEL_SEARCH_0;
    srchAttrs.m_rsltPortBlkMap[8].m_psNum = NLMDEV_PARALLEL_SEARCH_1;
    srchAttrs.m_rsltPortBlkMap[17].m_psNum = NLMDEV_PARALLEL_SEARCH_2;
    srchAttrs.m_rsltPortBlkMap[33].m_psNum = NLMDEV_PARALLEL_SEARCH_3;
    srchAttrs.m_keySBMap[0/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_0;
    srchAttrs.m_keySBMap[8/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_1;
    srchAttrs.m_keySBMap[17/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_2;
    srchAttrs.m_keySBMap[33/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_3;

    /* Key for parallel srch #0 is from Bits[79:0] */
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[0] = 0;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[0] = 10;

    /* Key for parallel srch #1 is from Bits[159:80] */
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_startByte[0] = 10;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_numOfBytes[0] = 10;

    /* Key for parallel srch #0 is from Bits[319:160] */
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_2].m_startByte[0] = 20;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_2].m_numOfBytes[0] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_2].m_startByte[1] = 36;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_2].m_numOfBytes[1] = 4;
    
    /* Key for parallel srch #0 is from Bits[479:320] */
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_3].m_startByte[0] = 40;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_3].m_numOfBytes[0] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_3].m_startByte[1] = 56;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_3].m_numOfBytes[1] = 4;   
         
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_0] = NLMDEV_NO_MASK_BMR_NUM;
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_1] = NLMDEV_NO_MASK_BMR_NUM;
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_2] = NLMDEV_NO_MASK_BMR_NUM;
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_3] = NLMDEV_NO_MASK_BMR_NUM;    

    if((errNum = NlmDevMgrRefApp_ConfigSrch(dev_p, 0, &srchAttrs)) != NLMERR_OK)
        return errNum;       
            
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_ConfigureLtr5Srches function configures 
LTR #5 to perform two 320b parallel searches on two 320b configured 
blocks */
NlmErrNum_t NlmDevMgrRefApp_ConfigureLtr5Srches(void *dev_p)
{
    NlmErrNum_t errNum;
    NlmDevMgrRefAppSrchAttrs srchAttrs;

    NlmCm__memset(&srchAttrs, 0, sizeof(NlmDevMgrRefAppSrchAttrs));
    srchAttrs.m_blkEnable[50] = NLMDEV_ENABLE;
    srchAttrs.m_blkEnable[82] = NLMDEV_ENABLE;
    srchAttrs.m_rsltPortBlkMap[50].m_psNum = NLMDEV_PARALLEL_SEARCH_0;
    srchAttrs.m_rsltPortBlkMap[82].m_psNum = NLMDEV_PARALLEL_SEARCH_1;
    srchAttrs.m_keySBMap[50/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_0;
    srchAttrs.m_keySBMap[82/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_1;
    
    /* Key for parallel srch #0 is from Bits[319:0] */
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[0] = 0;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[0] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[2] = 32;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[2] = 8;

    /* Key for parallel srch #1 is from Bits[639:320] */
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_startByte[0] = 40;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_numOfBytes[0] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_startByte[1] = 56;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_numOfBytes[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_startByte[2] = 72;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_numOfBytes[2] = 8;
         
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_0] = NLMDEV_NO_MASK_BMR_NUM;
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_1] = NLMDEV_NO_MASK_BMR_NUM;
    if((errNum = NlmDevMgrRefApp_ConfigSrch(dev_p, 5, &srchAttrs)) != NLMERR_OK)
        return errNum;       
            
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_ConfigureLtr14Srches function configures 
LTR #14 to perform single 640b searches on a 640b configured 
block */
NlmErrNum_t NlmDevMgrRefApp_ConfigureLtr14Srches(void *dev_p)
{
    NlmErrNum_t errNum;
    NlmDevMgrRefAppSrchAttrs srchAttrs;

    NlmCm__memset(&srchAttrs, 0, sizeof(NlmDevMgrRefAppSrchAttrs));
    srchAttrs.m_blkEnable[115] = NLMDEV_ENABLE;
    srchAttrs.m_rsltPortBlkMap[115].m_psNum = NLMDEV_PARALLEL_SEARCH_0;
    srchAttrs.m_keySBMap[115/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_0;
    
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[0] = 0;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[0] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[2] = 32;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[2] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[3] = 48;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[3] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[4] = 64;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[4] = 16;

    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_0] = NLMDEV_NO_MASK_BMR_NUM;
    if((errNum = NlmDevMgrRefApp_ConfigSrch(dev_p, 14, &srchAttrs)) != NLMERR_OK)
        return errNum;       
            
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_ConfigureLtr22Srches function configures 
LTR #22 to perform single 640b searches on a 640b configured 
block with a Block Mask which masks the odd bits  */
NlmErrNum_t NlmDevMgrRefApp_ConfigureLtr22Srches(void *dev_p)
{
    NlmErrNum_t errNum;
    NlmDevMgrRefAppSrchAttrs srchAttrs;

    NlmCm__memset(&srchAttrs, 0, sizeof(NlmDevMgrRefAppSrchAttrs));
    srchAttrs.m_blkEnable[115] = NLMDEV_ENABLE;
    srchAttrs.m_rsltPortBlkMap[115].m_psNum = NLMDEV_PARALLEL_SEARCH_0;
    srchAttrs.m_keySBMap[115/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_0;
    
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[0] = 0;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[0] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[2] = 32;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[2] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[3] = 48;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[3] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[4] = 64;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[4] = 16;

    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_0] = 0; /* Select the BMR which masks the odd bits */
    if((errNum = NlmDevMgrRefApp_ConfigSrch(dev_p, 22, &srchAttrs)) != NLMERR_OK)
        return errNum;                 
            
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_ConfigureLtr30Srches function configures 
LTR #33 to perform single 640b searches on a 640b configured 
block with a Block Mask which masks the even bits  */
NlmErrNum_t NlmDevMgrRefApp_ConfigureLtr30Srches(void *dev_p)
{
    NlmErrNum_t errNum;
    NlmDevMgrRefAppSrchAttrs srchAttrs;

    NlmCm__memset(&srchAttrs, 0, sizeof(NlmDevMgrRefAppSrchAttrs));
    srchAttrs.m_blkEnable[115] = NLMDEV_ENABLE;
    srchAttrs.m_rsltPortBlkMap[115].m_psNum = NLMDEV_PARALLEL_SEARCH_0;
    srchAttrs.m_keySBMap[115/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_0;
    
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[0] = 0;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[0] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[1] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[2] = 32;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[2] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[3] = 48;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[3] = 16;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_startByte[4] = 64;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_0].m_numOfBytes[4] = 16;

    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_0] = 2; /* Select the BMR which masks the even bits */
    if((errNum = NlmDevMgrRefApp_ConfigSrch(dev_p, 30, &srchAttrs)) != NLMERR_OK)
        return errNum;                 
            
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Add80bEntries function adds entries to 80b configured block 
    In all 512 entries are added to the block where 256 entries have the data
    where all the bytes of 80b remain same and other 256 entries have the data
    where MS 5 bytes of 80b differ LS 5 bytes by 1. Mask is all zeroes for all
    entries
     */
NlmErrNum_t NlmDevMgrRefApp_Add80bEntries(void *dev_p)
{
    nlm_u16 address;
    nlm_u8 blkNum;
    nlm_u16 entryNum;
    nlm_u8 data[NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;

     /* Add the entries where all the bytes of data are same and mask is zero*/
    for(entryNum = 0; entryNum < 256; entryNum++)
    {       
        NlmCm__memset(data, entryNum, NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memset(mask, 0, NLMDEV_AB_WIDTH_IN_BYTES);       
                   
        blkNum = 0;
        address = entryNum;
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                blkNum,
                                                address,
                                                data, 
                                                mask)) != NLMERR_OK)
            return errNum;       

        blkNum = 8;
        address = 512 + entryNum;
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                blkNum,
                                                address,
                                                data, 
                                                mask)) != NLMERR_OK)
            return errNum;      
    }

    /* Add the entries where the MSB 5 bytes of data differ from LSB 5 bytes of data by 1 and the mask is zero*/
    for(entryNum = 0; entryNum < 256; entryNum++)
    {         
        NlmCm__memset(data, entryNum + 1, NLMDEV_AB_WIDTH_IN_BYTES/2);
        NlmCm__memset(data + NLMDEV_AB_WIDTH_IN_BYTES/2, entryNum, NLMDEV_AB_WIDTH_IN_BYTES/2);
        NlmCm__memset(mask, 0, NLMDEV_AB_WIDTH_IN_BYTES);       
                   
        blkNum = 0;
        address = 256 + entryNum ;
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                 blkNum,
                                                 address,
                                                 data, 
                                                 mask)) != NLMERR_OK)
            return errNum;                                                   

        blkNum = 8;
        address = 768 + entryNum ;
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                 blkNum,
                                                 address,
                                                 data, 
                                                 mask)) != NLMERR_OK)
            return errNum;   
    }
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Add160bEntries function adds entries to 160b configured block 
    In all 512 entries are added to the block where 256 entries have the data
    where all the bytes of 160b remain same and other 256 entries have the data
    where MS 10 bytes of 80b differ LS 10 bytes by 1. Mask is all zeroes for all
    entries
     */
NlmErrNum_t NlmDevMgrRefApp_Add160bEntries(void *dev_p)
{
    nlm_u16 address;
    nlm_u8 blkNum;
    nlm_u16 entryNum;
    nlm_u8 data[2 * NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[2 * NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;

     /* Add the entries where all the bytes of data are same and mask is zero*/
    for(entryNum = 0; entryNum < 256; entryNum++)
    {       
        NlmCm__memset(data, entryNum, 2 * NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memset(mask, 0, 2 * NLMDEV_AB_WIDTH_IN_BYTES);       
                   
        blkNum = 17;
        address = entryNum * (160/80);
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                blkNum,
                                                address,
                                                data, 
                                                mask)) != NLMERR_OK)
            return errNum;       

        blkNum = 33;
        address = 1024 + entryNum * (160/80);
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                blkNum,
                                                address,
                                                data, 
                                                mask)) != NLMERR_OK)
            return errNum;      
    }

    /* Add the entries where the MSB 10 bytes of data differ from LSB 10 bytes of data by 1 and the mask is zero*/
    for(entryNum = 0; entryNum < 256; entryNum++)
    {         
        NlmCm__memset(data, entryNum + 1, NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memset(data + NLMDEV_AB_WIDTH_IN_BYTES, entryNum, NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memset(mask, 0, 2 * NLMDEV_AB_WIDTH_IN_BYTES);       
                   
        blkNum = 17;
        address = 512 + entryNum * (160/80);
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                 blkNum,
                                                 address,
                                                 data, 
                                                 mask)) != NLMERR_OK)
            return errNum;                                                   

        blkNum = 33;
        address = 1536 + entryNum * (160/80);
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                 blkNum,
                                                 address,
                                                 data, 
                                                 mask)) != NLMERR_OK)
            return errNum;   
    }
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Add320bEntries function adds entries to 320b configured blocks 
    In all 512 entries are added to each block where 256 entries have the data
    where all the bytes of 320b remain same and other 256 entries have the data
    where MS 20 bytes of 320b differ LS 20 bytes by 1. Mask is all zeroes for all
    entries
     */
NlmErrNum_t NlmDevMgrRefApp_Add320bEntries(void *dev_p)
{
    nlm_u16 address;
    nlm_u8 blkNum;
    nlm_u16 entryNum;
    nlm_u8 data[4 * NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[4 * NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;

     /* Add the entries where all the bytes of data are same and mask is zero*/
    for(entryNum = 0; entryNum < 256; entryNum++)
    {       
        NlmCm__memset(data, entryNum, 4 * NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memset(mask, 0, 4 * NLMDEV_AB_WIDTH_IN_BYTES);       
                   
        blkNum = 50;
        address = (entryNum * 320/80);
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                blkNum,
                                                address,
                                                data, 
                                                mask)) != NLMERR_OK)
            return errNum;                                                   

        blkNum = 82;
        address = 2048 + (entryNum * 320/80); 
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                 blkNum,
                                                 address,
                                                 data, 
                                                 mask)) != NLMERR_OK)
            return errNum;                                                   
    }

    /* Add the entries where the MSB 20 bytes of data differ from LSB 20 bytes of data by 1 and the mask is zero*/
    for(entryNum = 0; entryNum < 256; entryNum++)
    {         
        NlmCm__memset(data, entryNum + 1, 2 * NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memset(data + 2 * NLMDEV_AB_WIDTH_IN_BYTES, entryNum, 2 * NLMDEV_AB_WIDTH_IN_BYTES);
        NlmCm__memset(mask, 0, 4 * NLMDEV_AB_WIDTH_IN_BYTES);       
                   
        blkNum = 50;
        address = 1024 + (entryNum * 320/80);
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                 blkNum,
                                                 address,
                                                 data, 
                                                 mask)) != NLMERR_OK)
            return errNum;                                                   

        blkNum = 82;
        address = 3072 + (entryNum * 320/80);
        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                                 blkNum,
                                                 address,
                                                 data, 
                                                 mask)) != NLMERR_OK)
            return errNum;                                                   
    }
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_Add640bEntries function adds entries to 640b configured block. 
    5 Entries are added to the block, with first entry containing each byte of data 
    as 0xFF with mask as zeroes, second entry containing each byte of data as 0x55 
    with mask as zeroes, third entry containing each byte of data as 0xAA with mask 
    as zeroes, fourth entry containing each byte as 0x55 with each byte of mask as 0xAA,
    and fifth entry containing each byte as 0xAA with each byte of mask as 0x55 
     */
NlmErrNum_t NlmDevMgrRefApp_Add640bEntries(void *dev_p)
{   
    nlm_u8 blkNum; 
    nlm_u8 data[8 * NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[8 * NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;                                           
    
    /* Add Entry containing each byte of data as 0xFF with mask as zeroes */
    NlmCm__memset(data, 0xFF, 8 * NLMDEV_AB_WIDTH_IN_BYTES);
    NlmCm__memset(mask, 0, 8 * NLMDEV_AB_WIDTH_IN_BYTES);       
        
    blkNum = 115;   
    if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                             blkNum,
                                             0,
                                             data, 
                                             mask)) != NLMERR_OK)
       return errNum;                                                      
                                                   
    /* Add Entry containing each byte of data as 0x55 and each byte of mask as 0xAA */
    NlmCm__memset(data, 0x55, 8 * NLMDEV_AB_WIDTH_IN_BYTES);
    NlmCm__memset(mask, 0xAA, 8 * NLMDEV_AB_WIDTH_IN_BYTES);           
    
    if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                             blkNum,
                                             8,
                                             data, 
                                             mask)) != NLMERR_OK)
       return errNum;                                                   
        
    /* Add Entry containing each byte of data as 0xAA and each byte of mask as 0x55 */
    NlmCm__memset(data, 0xAA, 8 * NLMDEV_AB_WIDTH_IN_BYTES);
    NlmCm__memset(mask, 0x55, 8 * NLMDEV_AB_WIDTH_IN_BYTES);           
    
    if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                             blkNum,
                                             16,
                                             data, 
                                             mask)) != NLMERR_OK)
       return errNum;                                                         
    
    /* Add Entry containing each byte of data as 0x55 with mask as zeroes */
    NlmCm__memset(data, 0x55, 8 * NLMDEV_AB_WIDTH_IN_BYTES);
    NlmCm__memset(mask, 0, 8 * NLMDEV_AB_WIDTH_IN_BYTES);           
    
    if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                             blkNum,
                                             24,
                                             data, 
                                             mask)) != NLMERR_OK)
       return errNum;                                                   
    
     /* Add Entry containing each byte of data as 0xAA with mask as zeroes  */
    NlmCm__memset(data, 0xAA, 8 * NLMDEV_AB_WIDTH_IN_BYTES);
    NlmCm__memset(mask, 0, 8 * NLMDEV_AB_WIDTH_IN_BYTES);          
    
    if((errNum = NlmDevMgrRefApp_AddEntry(dev_p,
                                             blkNum,
                                             32,
                                             data, 
                                             mask)) != NLMERR_OK)
       return errNum;                                                       
    
    return NLMERR_OK;
}

/* NlmDevMgrRefApp_PerformLtr0Srches function perform compare
operations using LTR #0. In all 512 compare operations are performed
each producing a two search result performed on 80b blocks and two 
search result performed on 160b blocks
*/
NlmErrNum_t NlmDevMgrRefApp_PerformLtr0Srches(void *devMgr_p)
{
    nlm_u8 compareData[NLMDEV_MAX_CB_WRITE_IN_BYTES];
    nlm_u16 cmpNum;
    nlm_u16 cbAddr;
    nlm_u8 cbDatalen;
    NlmDevCmpRslt cmpRslts;
    NlmDevCmpRslt expectedCmpRslts;
    NlmErrNum_t errNum;
    nlm_u8 ltrNum;

    ltrNum = 0;
    /* Perform compares for the entries where all the bytes are same */
    for(cmpNum = 0; cmpNum < 256; cmpNum++)
    {
        NlmCm__memset(compareData,
                      cmpNum,
                      8 * NLMDEV_CB_WIDTH_IN_BYTES);
        cbAddr = cmpNum * 8;
        cbDatalen = 8 * NLMDEV_CB_WIDTH_IN_BYTES;
        if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                       ltrNum,
                                                       NLM_DEVMGR_REFAPP_CMP1,
                                                       cbAddr,
                                                       cbDatalen,
                                                       compareData,
                                                       &cmpRslts))!=NLMERR_OK)
            return errNum;

        /* Initialize the expected Results */
        NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
        expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;      
        expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_HIT;    
        expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_HIT;    
        expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_HIT;    
        expectedCmpRslts.m_hitDevId[0] = 0;        
        expectedCmpRslts.m_hitDevId[1] = 0;        
        expectedCmpRslts.m_hitDevId[2] = 0;        
        expectedCmpRslts.m_hitDevId[3] = 0;        
        expectedCmpRslts.m_hitIndex[0] = (0 << 12) | cmpNum;        
        expectedCmpRslts.m_hitIndex[1] = (8 << 12) | (512 + cmpNum);        
        expectedCmpRslts.m_hitIndex[2] = (17 << 12) | (cmpNum * 160/80);        
        expectedCmpRslts.m_hitIndex[3] = (33 << 12) | (1024 + cmpNum * 160/80);        

        /* Verify the compare results with the expected Results */
        if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                            &expectedCmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d, Compare Number %d",
                ltrNum, cmpNum);
            return errNum;
        }
        /* Verify the Result Register contents with the compare results */
        if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                                  &cmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d, Compare Number %d",
                ltrNum, cmpNum);
            return errNum;
        }
    }

    /* Perform compares for the entries where MS bytes differ form LS bytes  */
    for(cmpNum = 0; cmpNum < 256; cmpNum++)
    {    
        NlmCm__memset(compareData + (2 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum + 1,
                      NLMDEV_CB_WIDTH_IN_BYTES);

        NlmCm__memset(compareData + (3 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum,
                      NLMDEV_CB_WIDTH_IN_BYTES);

        NlmCm__memset(compareData + (4 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum + 1,
                      NLMDEV_CB_WIDTH_IN_BYTES);

        NlmCm__memset(compareData + (5 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum,
                      NLMDEV_CB_WIDTH_IN_BYTES);

        NlmCm__memset(compareData + (6 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum + 1,
                      NLMDEV_CB_WIDTH_IN_BYTES/2);

        NlmCm__memset(compareData + (6 * NLMDEV_CB_WIDTH_IN_BYTES +  NLMDEV_CB_WIDTH_IN_BYTES/2),
                      cmpNum,
                      NLMDEV_CB_WIDTH_IN_BYTES/2);

        NlmCm__memset(compareData + (7 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum + 1,
                      NLMDEV_CB_WIDTH_IN_BYTES/2);

        NlmCm__memset(compareData + (7 * NLMDEV_CB_WIDTH_IN_BYTES +  NLMDEV_CB_WIDTH_IN_BYTES/2),
                      cmpNum,
                      NLMDEV_CB_WIDTH_IN_BYTES/2);

        /* Here, Whole 640b of the Compare Data is passed via Compare Operation */      
        cbAddr = cmpNum * 8;
        cbDatalen = 8 * NLMDEV_CB_WIDTH_IN_BYTES;
        if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                       ltrNum,
                                                       NLM_DEVMGR_REFAPP_CMP1,
                                                       cbAddr,
                                                       cbDatalen,
                                                       compareData,
                                                       &cmpRslts))!=NLMERR_OK)
            return errNum;

         /* Initialize the expected Results */
        NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
        expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;      
        expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_HIT;    
        expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_HIT;    
        expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_HIT;    
        expectedCmpRslts.m_hitDevId[0] = 0;        
        expectedCmpRslts.m_hitDevId[1] = 0;        
        expectedCmpRslts.m_hitDevId[2] = 0;        
        expectedCmpRslts.m_hitDevId[3] = 0;        
        expectedCmpRslts.m_hitIndex[0] = (0 << 12) | (256 + cmpNum);        
        expectedCmpRslts.m_hitIndex[1] = (8 << 12) | (768 + cmpNum);        
        expectedCmpRslts.m_hitIndex[2] = (17 << 12) | (512 + cmpNum * 160/80);        
        expectedCmpRslts.m_hitIndex[3] = (33 << 12) | (1536 + cmpNum * 160/80);        

        /* Verify the compare results with the expected Results */
        if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                            &expectedCmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d, Compare Number %d",
                ltrNum, 256 + cmpNum);
            return errNum;
        }
        /* Verify the Result Register contents with the compare results */
        if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                                  &cmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d, Compare Number %d",
                ltrNum, 256 + cmpNum);
            return errNum;
        }
    }
    return NLMERR_OK;
} 


/* NlmDevMgrRefApp_PerformLtr5Srches function perform compare
operations using LTR #5. In all 512 compare operations are performed
each producing two parallel search results performed on two 320b blocks */
NlmErrNum_t NlmDevMgrRefApp_PerformLtr5Srches(void *devMgr_p)
{
    nlm_u8 compareData[NLMDEV_MAX_CB_WRITE_IN_BYTES];
    nlm_u16 cmpNum;
    nlm_u16 cbAddr;
    nlm_u8 cbDatalen;
    NlmDevCmpRslt cmpRslts;
    NlmDevCmpRslt expectedCmpRslts;
    NlmErrNum_t errNum;
    nlm_u8 ltrNum;

    ltrNum = 5;
    /* Perform compares for the entries where all the bytes are same */
    for(cmpNum = 0; cmpNum < 256; cmpNum++)
    {
        NlmCm__memset(compareData,
                      cmpNum,
                      8 * NLMDEV_CB_WIDTH_IN_BYTES);

        cbAddr = cmpNum * 8;
		cbDatalen = 8 * NLMDEV_CB_WIDTH_IN_BYTES;
        if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                       ltrNum,
                                                       NLM_DEVMGR_REFAPP_CMP1,
                                                       cbAddr,
                                                       cbDatalen,
                                                       compareData,
                                                       &cmpRslts))!=NLMERR_OK)
            return errNum;

        /* Initialize the expected Results */
        NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
        expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;
        expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_HIT;
        expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
        expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;
        expectedCmpRslts.m_hitDevId[0] = 0;
        expectedCmpRslts.m_hitDevId[1] = 0;        
        expectedCmpRslts.m_hitIndex[0] = (50 << 12) | (cmpNum * 320/80);
        expectedCmpRslts.m_hitIndex[1] = (82 << 12) | (2048 + (cmpNum * 320/80));

        /* Verify the compare results with the expected Results */
        if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                            &expectedCmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d, Compare Number %d",
                ltrNum, cmpNum);
            return errNum;
        }
        /* Verify the Result Register contents with the compare results */
        if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                                  &cmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d, Compare Number %d",
                ltrNum, cmpNum);
            return errNum;
        }
    }

    /* Perform compares for the entries where MS 20 bytes differ form LS 20 bytes  */
    for(cmpNum = 0; cmpNum < 256; cmpNum++)
    {    
        NlmCm__memset(compareData,
                      cmpNum + 1,
                      2 * NLMDEV_CB_WIDTH_IN_BYTES);
        NlmCm__memset(compareData + (2 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum,
                      2 * NLMDEV_CB_WIDTH_IN_BYTES);

        NlmCm__memset(compareData + (4 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum + 1,
                      2 * NLMDEV_CB_WIDTH_IN_BYTES);
        NlmCm__memset(compareData + (6 * NLMDEV_CB_WIDTH_IN_BYTES),
                      cmpNum,
                      2 * NLMDEV_CB_WIDTH_IN_BYTES);


        /* Here, Whole 640b of the Compare Data is passed via Compare Operation */      
        cbAddr = cmpNum * 8;
        cbDatalen = 8 * NLMDEV_CB_WIDTH_IN_BYTES;
        if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                       ltrNum,
                                                       NLM_DEVMGR_REFAPP_CMP2,
                                                       cbAddr,
                                                       cbDatalen,
                                                       compareData,
                                                       &cmpRslts))!=NLMERR_OK)
            return errNum;

        /* Initialize the expected Results */
        NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
        expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;
        expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_HIT;
        expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
        expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;
        expectedCmpRslts.m_hitDevId[0] = 0;
        expectedCmpRslts.m_hitDevId[1] = 0;        
        expectedCmpRslts.m_hitIndex[0] = (50 << 12) | (1024 + (cmpNum * 320/80));
        expectedCmpRslts.m_hitIndex[1] = (82 << 12) | (3072 + (cmpNum * 320/80));

        /* Verify the compare results with the expected Results */
        if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                            &expectedCmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d, Compare Number %d",
                ltrNum, 256 + cmpNum);
            return errNum;
        }
        /* Verify the Result Register contents with the compare results */
        if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                                  &cmpRslts))!=NLMERR_OK)
        {
            NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d, Compare Number %d",
                ltrNum, 256 + cmpNum);
            return errNum;
        }
    }

    return NLMERR_OK;
} 

/* NlmDevMgrRefApp_PerformLtr14Srches function perform a compare
operation using LTR #14 to produce HIT for first entry added to 
640b block */
NlmErrNum_t NlmDevMgrRefApp_PerformLtr14Srches(void *devMgr_p)
{
    nlm_u8 compareData[NLMDEV_MAX_CB_WRITE_IN_BYTES];    
    nlm_u16 cbAddr;
    nlm_u8 cbDatalen;
    NlmDevCmpRslt cmpRslts;
    NlmDevCmpRslt expectedCmpRslts;
    NlmErrNum_t errNum;
    nlm_u8 ltrNum;

    ltrNum = 14;
    /* Compare Key contains all bytes as 0xFF*/
    NlmCm__memset(compareData,
                   0xFF,
                   8 * NLMDEV_CB_WIDTH_IN_BYTES);

    cbAddr = 0;
	cbDatalen = 8 * NLMDEV_CB_WIDTH_IN_BYTES;
    if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                   ltrNum,
                                                   NLM_DEVMGR_REFAPP_CMP2,
                                                   cbAddr,
                                                   cbDatalen,
                                                   compareData,
                                                   &cmpRslts))!=NLMERR_OK)
        return errNum;
    
    /* Initialize the expected Results */
    NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
    expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;
    expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;
    
    expectedCmpRslts.m_hitDevId[0] = 0;
    expectedCmpRslts.m_hitIndex[0] = (115 << 12);

    /* Verify the compare results with the expected Results */
    if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                        &expectedCmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d", ltrNum);
        return errNum;
    }
    /* Verify the Result Register contents with the compare results */
    if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                              &cmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d",
            ltrNum);
        return errNum;
    }

    return NLMERR_OK;
} 

/* NlmDevMgrRefApp_PerformLtr14SrchesAfterDeletes function deletes the first entry
added to 640 block and then performs a compare operation using LTR #14 to produce HIT 
for second entry added to 640b blocks; Then it deletes the second entry added to the 
640b block and then performs a compare operation using LTR #14 to produce HIT for 
the third entry added the 640b blocks; At last it deletes third entry added to the 
640b block and then performs a compare operation using LTR #14 to produce MISS;
This function demonstrates the usage of local mask*/
NlmErrNum_t NlmDevMgrRefApp_PerformLtr14SrchesAfterDeletes(void *devMgr_p)
{
    nlm_u8 compareData[NLMDEV_CB_WIDTH_IN_BYTES];    
    nlm_u16 cbAddr;
    nlm_u8 cbDatalen;
    NlmDevCmpRslt cmpRslts;
    NlmDevCmpRslt expectedCmpRslts;
    NlmErrNum_t errNum;
    nlm_u8 blkNum;
    nlm_u8 ltrNum;
    NlmDev *dev_p = ((NlmDevMgr*)devMgr_p)->m_devList_pp[0];    

    /* Deleting first entry of 640b block */
    blkNum = 115;   
    if((errNum = NlmDevMgrRefApp_DeleteEntry(dev_p,
                                             blkNum,
                                             0)) != NLMERR_OK)
       return errNum;                                                   
    
    ltrNum = 14;
    /* Only LSB 80b of compare data is generated; Other 560b of compare data is taken from CB memory
    which was written in NlmDevMgrRefApp_PerformLtr4SrchesBeforeDeletes function*/
    NlmCm__memset(compareData,
                   0xFF,
                   4*NLMDEV_CB_WIDTH_IN_BYTES);
    
    cbAddr = 0;
    cbDatalen = 4*NLMDEV_CB_WIDTH_IN_BYTES;
    if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                   ltrNum,
                                                   NLM_DEVMGR_REFAPP_CMP2,
                                                   cbAddr,
                                                   cbDatalen,
                                                   compareData,
                                                   &cmpRslts))!=NLMERR_OK)
        return errNum;
    
    /* Initialize the expected Results */
    NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
    expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;  
    expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;    
    expectedCmpRslts.m_hitDevId[0] = 0;
    expectedCmpRslts.m_hitIndex[0] = (115 << 12) | 8;
    
    /* Verify the compare results with the expected Results */
    if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                        &expectedCmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d", ltrNum);
        return errNum;
    }
    /* Verify the Result Register contents with the compare results */
    if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                              &cmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d",
            ltrNum);
        return errNum;
    }

    /* Deleting second entry of 640b block */
    blkNum = 115;   
    if((errNum = NlmDevMgrRefApp_DeleteEntry(dev_p,
                                             blkNum,
                                             8)) != NLMERR_OK)
       return errNum;                                                   
        
    /* Only LSB 80b of compare data is generated; Other 560b of compare data is taken from CB memory
    which was written in NlmDevMgrRefApp_PerformLtr4SrchesBeforeDeletes function*/
    NlmCm__memset(compareData,
                   0xFF,
                   4*NLMDEV_CB_WIDTH_IN_BYTES);
    
    cbAddr = 0;
    cbDatalen = 4*NLMDEV_CB_WIDTH_IN_BYTES;
    if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                   ltrNum,
                                                   NLM_DEVMGR_REFAPP_CMP2,
                                                   cbAddr,
                                                   cbDatalen,
                                                   compareData,
                                                   &cmpRslts))!=NLMERR_OK)
        return errNum;
    
    /* Initialize the expected Results */
    NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
    expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;
    expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;    
    expectedCmpRslts.m_hitDevId[0] = 0;    
    expectedCmpRslts.m_hitIndex[0] = (115 << 12) | 16;
    
    /* Verify the compare results with the expected Results */
    if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                        &expectedCmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d", ltrNum);
        return errNum;
    }

    /* Verify the Result Register contents with the compare results */
    if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                              &cmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d",
            ltrNum);
        return errNum;
    }


    /* Deleting third entry of 640b block */   
    blkNum = 115;   
    if((errNum = NlmDevMgrRefApp_DeleteEntry(dev_p,
                                             blkNum,
                                             16)) != NLMERR_OK)
       return errNum;                                                   
    
    /* Only LSB 80b of compare data is generated; Other 560b of compare data is taken from CB memory
    which was written in NlmDevMgrRefApp_PerformLtr4SrchesBeforeDeletes function*/
    NlmCm__memset(compareData,
                   0xFF,
                   4*NLMDEV_CB_WIDTH_IN_BYTES);
    
    cbAddr = 0;
    cbDatalen = 4*NLMDEV_CB_WIDTH_IN_BYTES;
    if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                   ltrNum,
                                                   NLM_DEVMGR_REFAPP_CMP2,
                                                   cbAddr,
                                                   cbDatalen,
                                                   compareData,
                                                   &cmpRslts))!=NLMERR_OK)
        return errNum;
    
    /* Initialize the expected Results */
    NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
    expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;    
    
    /* Verify the compare results with the expected Results */
    if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                        &expectedCmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d", ltrNum);
        return errNum;
    } 
    /* Verify the Result Register contents with the compare results */
    if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                              &cmpRslts))!=NLMERR_OK)
    {
       NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d",
            ltrNum);        
        return errNum;
    }


    return NLMERR_OK;
} 

/* NlmDevMgrRefApp_PerformLtr22Srches function perform a compare
operation using LTR #22 to produce HIT for fourth entry added to 
the 640b and 160b blocks; This function demonstates the usage
of Block Mask which masks the odd bits */
NlmErrNum_t NlmDevMgrRefApp_PerformLtr22Srches(void *devMgr_p)
{
    nlm_u8 compareData[NLMDEV_CB_WIDTH_IN_BYTES];    
    nlm_u16 cbAddr;
    nlm_u8 cbDatalen;
    NlmDevCmpRslt cmpRslts;
    NlmDevCmpRslt expectedCmpRslts;
    NlmErrNum_t errNum;
    nlm_u8 ltrNum;

     /* Only LSB 80b of compare data is generated; Other 560b of compare data is taken from CB memory
    which was written in NlmDevMgrRefApp_PerformLtr4SrchesBeforeDeletes function*/
    NlmCm__memset(compareData, 0xFF, 8*NLMDEV_CB_WIDTH_IN_BYTES);
    
    ltrNum = 22;

    cbAddr = 0;
    cbDatalen = 8*NLMDEV_CB_WIDTH_IN_BYTES;
    if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                   ltrNum,
                                                   NLM_DEVMGR_REFAPP_CMP2,
                                                   cbAddr,
                                                   cbDatalen,
                                                   compareData,
                                                   &cmpRslts))!=NLMERR_OK)
        return errNum;
    
    /* Initialize the expected Results */
    NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
    expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;
    expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;    
    expectedCmpRslts.m_hitDevId[0] = 0;  
    expectedCmpRslts.m_hitIndex[0] = (115 << 12) | 24;
    
    /* Verify the compare results with the expected Results */
    if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                        &expectedCmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d", ltrNum);
        return errNum;
    }
    /* Verify the Result Register contents with the compare results */
    if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                              &cmpRslts))!=NLMERR_OK)
    {
       NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d",
            ltrNum);
        
        return errNum;
    }

    return NLMERR_OK;
} 

/* NlmDevMgrRefApp_PerformLtr30Srches function perform a compare
operation using LTR #30 to produce HITs for fifth entry added to 
the 640b block; This function demonstates the usage of Block Mask 
which masks even bits*/
NlmErrNum_t NlmDevMgrRefApp_PerformLtr30Srches(void *devMgr_p)
{
    nlm_u8 compareData[NLMDEV_CB_WIDTH_IN_BYTES];    
    nlm_u16 cbAddr;
    nlm_u8 cbDatalen;
    NlmDevCmpRslt cmpRslts;
    NlmDevCmpRslt expectedCmpRslts;
    NlmErrNum_t errNum;
    nlm_u8 ltrNum;

     /* Only LSB 80b of compare data is generated; Other 560b of compare data is taken from CB memory
    which was written in NlmDevMgrRefApp_PerformLtr4SrchesBeforeDeletes function*/
    NlmCm__memset(compareData, 0xFF, 8*NLMDEV_CB_WIDTH_IN_BYTES);
    
    ltrNum = 30;
    
    cbAddr = 0;
    cbDatalen = 8*NLMDEV_CB_WIDTH_IN_BYTES;
    if((errNum = NlmDevMgrRefApp_PerformCompare(devMgr_p,
                                                   ltrNum,
                                                   NLM_DEVMGR_REFAPP_CMP2,
                                                   cbAddr,
                                                   cbDatalen,
                                                   compareData,
                                                   &cmpRslts))!=NLMERR_OK)
        return errNum;
    
    /* Initialize the expected Results */
    NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
    expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;
    expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;
    expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;    
    expectedCmpRslts.m_hitDevId[0] = 0;    
    expectedCmpRslts.m_hitIndex[0] = (115 << 12) | 32;
    
    /* Verify the compare results with the expected Results */
    if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts,
                                                        &expectedCmpRslts))!=NLMERR_OK)
    {
        NlmCm__printf("\nCompare Operation Results Differ for LTR Number %d", ltrNum);
        return errNum;
    }

    /* Verify the Result Register contents with the compare results */
    if((errNum = NlmDevMgrRefApp_VerifyRsltRegWithCmpRslts(devMgr_p,
                                                              &cmpRslts))!=NLMERR_OK)
    {
       NlmCm__printf("\nResult Register Values differ with compare results for LTR Number %d",
            ltrNum);
        
        return errNum;
    }


    return NLMERR_OK;
} 


int nlmdevmgr_refapp_main(void)
{
    NlmCmAllocator *alloc_p;
    NlmCmAllocator      alloc_bdy;
    void *xpt_p = NULL;
    void *devMgr_p = NULL;
    void *dev_p = NULL;   
    NlmDevId devId;
    NlmReasonCode reasonCode; 
    NlmErrNum_t errNum;
	NlmDevMgrRefApp_XptMode xpt_mode;

#ifndef NLMPLATFORM_BCM
	xpt_mode = NLM_SIMULATION_MODE;
#ifdef NLM_XLP
	xpt_mode = NLM_XLPXPT_MODE;
#endif
#else /* BCMPLATFORM */
	xpt_mode = NLM_BCM_CALADAN3_MODE;
#endif /* NLMPLATFORM_BCM */
      
    nlm_32 break_alloc_id = -1;

    NlmCm__printf ("\n Device Manager Application Reference Code Using \n");
    NlmCm__printf (" Device Manager Module. \n");

    NlmCmDebug__Setup(break_alloc_id, NLMCM_DBG_EBM_ENABLE);

	/* create allocator */
	alloc_p = NlmCmAllocator__ctor(&alloc_bdy);
    NlmCmAssert((alloc_p != NULL), "The memory allocator cannot be NULL!\n");
    /* As NlmCmAssert() is nulled out in the Release Build, add a check */
    if(alloc_p == NULL)
    {
    	NlmCm__printf("\n Memory Allocator Init failed. Exiting.\n");
    	return 0;
    }

    /* Create the Cmodel simulation Xpt Interface in standard operating mode. 
     * As Processor is serial device, speed mode and RBus mode are ignored 
     */  
	switch(xpt_mode)
	{
#ifndef NLMPLATFORM_BCM
		case NLM_SIMULATION_MODE:
			NlmCm__printf("\n\n\n\t Initializing C-model Simulation Transport(Xpt) Interface");
		    if((xpt_p = NlmSimXpt__Create(alloc_p, NLM_DEVTYPE_2, 0,
							1, 1, NLMDEV_OPR_STANDARD, 0, 0)) == NULL)
		    {
		        NlmCm__printf("\n Err: SimXpt API 'NlmSimXpt__Create'");
		        return 0;
		    }
			break;
		case NLM_FPGAXPT_MODE:
			{
				NlmCm__printf("\n\t FPGA Transport not supported\n\n");
		        return 1;
			}
#ifdef NLM_XLP
		case NLM_XLPXPT_MODE:
			NlmCm__printf("\n\n\n\t Initializing XLP Transport(Xpt) Interface");
			if((xpt_p = NlmXlpXpt__Create(alloc_p, NLM_DEVTYPE_2, 1, 
							NLMDEV_OPR_STANDARD, 0, 0, 0, 1,1)) == NULL)
			{
				NlmCm__printf("\n Err: XLP XPT API 'NlmXlpXpt__Create'");
				return 0;
			}
			break;
#endif
#endif  /* NLMPLATFORM_BCM */
		case NLM_BCM_CALADAN3_MODE:
		    if((xpt_p = soc_sbx_caladan3_etu_xpt_create(0, NLM_DEVTYPE_2, 0, 0,  
								NLMDEV_OPR_STANDARD, 0)) == NULL)
		    {
			NlmCm__printf("\n\tError: soc_sbx_caladan3_etu_xpt_create failed");
			return 1;
		    }
		    break;
		default:
            NlmCm__printf("\n\tInvalid Xpt mode specified\n");
            break;
		

	}

 
    /* Create the instance of Device Manager in standard operating mode.      
     */
    NlmCm__printf("\n\n\t Creating the instance of Device Manager");
    if((devMgr_p = NlmDevMgr__create(alloc_p, xpt_p, NLMDEV_OPR_STANDARD, &reasonCode)) == NULL)
    {
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__create', Reason Code -- %d", reasonCode);
        return 0;
    }

    /* Add a device to Device Manager.      
     */
    NlmCm__printf("\n\n\t Adding a Device to the Device Manager");
    if((dev_p = NlmDevMgr__AddDevice(devMgr_p, &devId, &reasonCode)) == NULL)
    {
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__AddDevice', Reason Code -- %d", reasonCode);
        return 0;
    }       

    /* Lock the device manager configuration */
    NlmCm__printf("\n\n\t Locking the Device Manager");
    if((errNum = NlmDevMgr__LockConfig(devMgr_p, &reasonCode)) != NLMERR_OK)
    {
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LockConfig', Reason Code -- %d", reasonCode);
        return 0;
    }       

    /* ----------------------------------------------------------------------*/

    NlmCm__printf("\n\n\n\t Configuring 80b blocks");
    if((errNum = NlmDevMgrRefApp_Configure80bBlks(dev_p)) != NLMERR_OK)
        return 0;

    NlmCm__printf("\n\n\t Configuring 160b blocks");
    if((errNum = NlmDevMgrRefApp_Configure160bBlks(dev_p)) != NLMERR_OK)
        return 0;      
    
    NlmCm__printf("\n\n\t Configuring LTR #0 for two 80b and two 160b parallel searches");
    if((errNum = NlmDevMgrRefApp_ConfigureLtr0Srches(dev_p)) != NLMERR_OK)
        return 0;       

    NlmCm__printf("\n\n\t Adding Entries to 80b blocks");
    if((errNum = NlmDevMgrRefApp_Add80bEntries(dev_p)) != NLMERR_OK)
        return 0;
    
    NlmCm__printf("\n\n\t Adding Entries to 160b blocks");
    if((errNum = NlmDevMgrRefApp_Add160bEntries(dev_p)) != NLMERR_OK)
        return 0;

    NlmCm__printf("\n\n\t Performing compares using LTR #0");
    if((errNum = NlmDevMgrRefApp_PerformLtr0Srches(devMgr_p)) != NLMERR_OK)
        return 0;       

    /* ----------------------------------------------------------------------*/
    NlmCm__printf("\n\n\n\t Configuring 320b blocks");
    if((errNum = NlmDevMgrRefApp_Configure320bBlks(dev_p)) != NLMERR_OK)
        return 0;  
    
    NlmCm__printf("\n\n\t Configuring LTR #5 for two 320b parallel searches");
    if((errNum = NlmDevMgrRefApp_ConfigureLtr5Srches(dev_p)) != NLMERR_OK)
        return 0;    

    NlmCm__printf("\n\n\t Adding Entries to 320b blocks");
    if((errNum = NlmDevMgrRefApp_Add320bEntries(dev_p)) != NLMERR_OK)
        return 0;

    NlmCm__printf("\n\n\t Performing compares using LTR #5");
    if((errNum = NlmDevMgrRefApp_PerformLtr5Srches(devMgr_p)) != NLMERR_OK)
        return 0;     

    /* ----------------------------------------------------------------------*/

    NlmCm__printf("\n\n\n\t Configuring 640b blocks");
    if((errNum = NlmDevMgrRefApp_Configure640bBlks(dev_p)) != NLMERR_OK)
        return 0;  

    NlmCm__printf("\n\n\t Configuring LTR #14 for a 640b search");
    if((errNum = NlmDevMgrRefApp_ConfigureLtr14Srches(dev_p)) != NLMERR_OK)
        return 0;     

    NlmCm__printf("\n\n\t Configuring LTR #22 for a 640b search with BMR masking odd bits");
    if((errNum = NlmDevMgrRefApp_ConfigureLtr22Srches(dev_p)) != NLMERR_OK)
        return 0;     

    NlmCm__printf("\n\n\t Configuring LTR #30 for a 640b search with BMR masking even bits");
    if((errNum = NlmDevMgrRefApp_ConfigureLtr30Srches(dev_p)) != NLMERR_OK)
        return 0;     
   
    NlmCm__printf("\n\n\t Adding Entries to 640b blocks");
    if((errNum = NlmDevMgrRefApp_Add640bEntries(dev_p)) != NLMERR_OK)
        return 0;  

    NlmCm__printf("\n\n\t Performing compares using LTR #14");
    if((errNum = NlmDevMgrRefApp_PerformLtr14Srches(devMgr_p)) != NLMERR_OK)
        return 0;  

     NlmCm__printf("\n\n\t Performing compares using LTR #14 to demonstrate use of Local mask");
    if((errNum = NlmDevMgrRefApp_PerformLtr14SrchesAfterDeletes(devMgr_p)) != NLMERR_OK)
        return 0;    

    NlmCm__printf("\n\n\t Performing compares using LTR #22 to demonstrate use of BMR to mask odd bits");
    if((errNum = NlmDevMgrRefApp_PerformLtr22Srches(devMgr_p)) != NLMERR_OK)
        return 0;    
    
    NlmCm__printf("\n\n\t Performing compares using LTR #30 to demonstrate use of BMR to mask even bits");
    if((errNum = NlmDevMgrRefApp_PerformLtr30Srches(devMgr_p)) != NLMERR_OK)
        return 0;        

    /* ----------------------------------------------------------------------*/    
    
    /* Destroy the instance of Device Manager */
    NlmCm__printf("\n\n\n\t Destroying the instance of Device Manager");    
    NlmDevMgr__destroy(devMgr_p);

    /* Destroy the xpt interface */
    NlmCm__printf("\n\n\t Destroying Xpt Interface");    
       
	switch(xpt_mode)
	{
#ifndef NLMPLATFORM_BCM
		case NLM_SIMULATION_MODE:
			NlmSimXpt__destroy(xpt_p); 
			break;

		case NLM_FPGAXPT_MODE:
			{
				NlmCm__printf("\n\t FPGA Transport not supported\n\n");
		        return 1;
			}
#ifdef NLM_XLP
		case NLM_XLPXPT_MODE:
			NlmXlpXpt__Destroy(xpt_p);
			break;
#endif
#endif /* NLMPLATFORM_BCM */
                case NLM_BCM_CALADAN3_MODE:
		    soc_sbx_caladan3_etu_xpt_destroy(xpt_p);
		    break;
		default:
            NlmCm__printf("\n\tInvalid Xpt mode specified\n");
            break;
	}


    NlmCmAllocator__dtor(alloc_p);

    /* check for memory leak */
    if(NlmCmDebug__IsMemLeak())
    {
        NlmCm__printf("\n\t ... Memory Leak occured ...\n\n");
        return 0;
    }
    NlmCm__printf("\n\n\n Device Manager Reference Application Completed Successfully\n\n");

    return 1;
}
