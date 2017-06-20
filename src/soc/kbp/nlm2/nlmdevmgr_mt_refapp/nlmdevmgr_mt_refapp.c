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
 
#include "nlmdevmgr_mt_refapp.h"
#include "nlmcmdevice.h"

#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

NlmDevMgrRefApp *g_refAppData_p;
nlm_u32          cmpNum = 0; /* Compares count */
nlm_u8			 g_maxBlks;

#ifdef NLM_NETOS
volatile nlm_u8  checkJoin[NLM_DEVMGR_REFAPP_THREAD_COUNT];
#endif

volatile nlm_u8  g_cmdNotDone;

char			 *threadStack[NLM_DEVMGR_REFAPP_THREAD_COUNT];

NlmErrNum_t NlmDevMgrRefApp_InitializeDatabase(void *threadNum);
NlmErrNum_t NlmDevMgrRefApp_PerformLTRSearches(void *threadNum);

/* Binds calling thread to cpuid on Linux. Does not have any affect 
 * for NETOS
 */
nlm_u32
NlmDevMgrRefApp_BindThreadToCore(
	nlm_u32 cpuid
	)
{
#ifdef NLM_POSIX
    cpu_set_t cpu_mask;
    nlm_u32 ret;

    CPU_ZERO(&cpu_mask);
    CPU_SET(cpuid, &cpu_mask);

    ret = pthread_setaffinity_np( pthread_self(), sizeof(cpu_mask), &cpu_mask);
    if(ret != 0)
	{
		return 1;
	}
#endif
	(void) cpuid;

	return 0;
}


NlmErrNum_t 
NlmDevMgrRefApp_CreateThreads(void)
{
	NlmErrNum_t errNum = 0;	
	nlm_u32 th = 0;

#ifdef NLM_NETOS
	nlm_u32 sum = 0;
#endif

	void* thread_Attr = NULL;

	/* Create Threads */
	for(th = 0; th < NLM_DEVMGR_REFAPP_THREAD_COUNT; th++)
	{
#ifdef NLM_NETOS
			int  size = 64 * 1024; /* 64k stack size */

			if((threadStack[th] = (char*) malloc(size)) == NULL)
			{
				NlmCm__printf("Stack memory allocation failed for thread-%d\n", th);
				return NLMERR_FAIL;
			}

			/* stack grows from top to bottom  */
			thread_Attr = threadStack[th] + size - 64;
			
#endif

		if(th == 0)  /* Add-thread */
		{
            g_refAppData_p->threadId[th] = th;
			errNum = NlmCmMt__CreateThread( &g_refAppData_p->devThrdId[th],
											thread_Attr,
											(void*)NlmDevMgrRefApp_InitializeDatabase,
											(void *)&g_refAppData_p->threadId[th] );

			NlmCmAssert(errNum == NLMERR_OK, "Add-thread creation failed. \n");
		}
		else /* Search-thread */
		{
			g_refAppData_p->threadId[th] = th;
			errNum = NlmCmMt__CreateThread( &g_refAppData_p->devThrdId[th],
											thread_Attr,
											(void*)NlmDevMgrRefApp_PerformLTRSearches,
											(void *)&g_refAppData_p->threadId[th] );

			NlmCmAssert(errNum == NLMERR_OK, "Search-thread creation failed. \n");
		}
	}

	return NLMERR_OK;
}


/*
 * NlmDevMgrRefApp_InitializeBlk function enables a block and initializes it
 * to the specified block width by writing to Block Configuration Register of the
 * specified Block
 */
NlmErrNum_t 
NlmDevMgrRefApp_InitializeBlk(
	NlmDev *dev_p, 
	nlm_u8 blkNum,
	nlm_u16 blkWidth
	)
{
    NlmDevShadowDevice *shadowDev_p;
    NlmDevBlockConfigReg *blkConfigReg_p;
    NlmErrNum_t errNum = NLMERR_OK;
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

		default:
			return NLMERR_FAIL;
    }

    /* Invoke the API which writes to specified Blk Register */
    if((errNum = NlmDevMgr__BlockRegisterWrite(dev_p, 
		blkNum, NLMDEV_BLOCK_CONFIG_REG, blkConfigReg_p, &reasonCode)) != NLMERR_OK) 
    {    
        NlmCm__printf("\n\n Initialization of Block Number %d failed", blkNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__BlockRegisterWrite', Reason Code -- %d", reasonCode);

        return errNum;
    }

    return NLMERR_OK;
}


/*
 * NlmDevMgrRefApp_InitializeBmr function initializes the Global mask used for the block
 * by writing to the specified Block Mask Register of the specified Block. 
 *
 * Note: Application can initilize upto 5 different BMRs per block;
 * This function assumes that application has initialized the block to an appropriate 
 * Block Width
 */
NlmErrNum_t 
NlmDevMgrRefApp_InitializeBmr(
	NlmDev *dev_p, 
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
        if((errNum = NlmDevMgr__BlockRegisterWrite(dev_p, blkNum, regType, blkMaskReg_p, &reasonCode)) != NLMERR_OK)  
        {    
            NlmCm__printf("\n\n Initialization of Block Mask Number %d, Segment Number %d of Block Number %d failed",
                                bmrNum, segNum % NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR, blkNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__BlockRegisterWrite', Reason Code -- %d", reasonCode);
            return errNum;
        }      
    }

    return NLMERR_OK;
}


/*
 * This function initializes the specified Search 
 * Attributes by writing to the Logical Table Registers of the specified LTR Number. 
 */
NlmErrNum_t 
NlmDevMgrRefApp_ConfigSearch(
	NlmDev *dev_p, 
	nlm_u8 ltrNum,
	NlmDevMgrRefAppSrchAttrs *srchAttrs
	)
{
    NlmDevShadowDevice *shadowDev_p;
    NlmDevBlkSelectReg *blkSelectReg_p;
    NlmDevSuperBlkKeyMapReg *superblkKeySelectReg_p;
    NlmDevParallelSrchReg *parallelSrchReg_p;
    NlmDevMiscelleneousReg   *miscReg_p;
    NlmDevKeyConstructReg *keyConstructReg_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_32 kcrNum, keyNum, blkSelectRegNum, psRegNum;

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
			ltrNum, NLMDEV_BLOCK_SELECT_0_LTR + blkSelectRegNum, blkSelectReg_p, &reasonCode)) != NLMERR_OK)  
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
		ltrNum, NLMDEV_SUPER_BLK_KEY_MAP_LTR, superblkKeySelectReg_p, &reasonCode)) != NLMERR_OK)  
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
			ltrNum, NLMDEV_PARALLEL_SEARCH_0_LTR + psRegNum, parallelSrchReg_p, &reasonCode)) != NLMERR_OK)  
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
    NlmCm__memcpy(miscReg_p->m_bmrSelect, srchAttrs->m_bmrNum, NLMDEV_NUM_PARALLEL_SEARCHES * sizeof(nlm_u8));

    /* Invoke the API which writes to specified LTR Register */
    if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
		ltrNum, NLMDEV_MISCELLENEOUS_LTR, miscReg_p, &reasonCode)) != NLMERR_OK) 
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
            if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p, ltrNum, 
				(NLMDEV_KEY_0_KCR_0_LTR + keyNum * NLMDEV_NUM_OF_KCR_PER_KEY + kcrNum),
				keyConstructReg_p, &reasonCode)) != NLMERR_OK)  
            {    
                NlmCm__printf("\n\n Initialization of Key Construction %d Register of Key Number %d of LTR Number %d failed", kcrNum, keyNum, ltrNum);
                NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);

                return errNum;
            }
        }
    }

    return NLMERR_OK;
}                                         

/* 
 * This function writes Database Entry to specified Blk Num and startAddress;
 * This function assumes that application has initialized the block to an appropriate 
 * Block Width and writes database entries in terms of Blk Width
 */
NlmErrNum_t 
NlmDevMgrRefApp_AddEntry(
	NlmDev *dev_p, 
	nlm_u8 blkNum,
	nlm_u16 startAddress,
	nlm_u8 *data,
	nlm_u8 *mask
	)
{
    NlmDevABEntry *abEntry_p;
    NlmDevShadowDevice *shadowDev_p;    
    NlmDevBlockWidth blkWidth;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_u16 segNum, numberOf80bSegments = 0;

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


/*
 * This function performs Compare operation based on compare type. Compare Data
 * can be upto 640b and is written to CB memory starting from cbStartAddr.
 */
NlmErrNum_t 
NlmDevMgrRefApp_PerformCompare(
	NlmDevMgr *devMgr_p, 
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
										 ltrNum, &cbInfo, cmpRslts, &reason)) != NLMERR_OK)
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
										 ltrNum, &cbInfo, cmpRslts, &reason)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n Compare2 operation for LTR Number %d failed", ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__Compare2', Reason Code -- %d", reason);

            return errNum;
        }
    }

    return NLMERR_OK;
}


/* This function verifies the compare operation results with the expected results */
NlmErrNum_t
NlmDevMgrRefApp_VerifyCompareResults(
	NlmDevCmpRslt *cmpRslts,
	NlmDevCmpRslt *expectedCmpRslts
	)
{ 
    nlm_32 psNum;

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
		/* Continue if expected is miss */
    	if(expectedCmpRslts->m_hitOrMiss[psNum] == NLMDEV_MISS)
        	continue;

		if(cmpRslts->m_hitIndex[psNum] != expectedCmpRslts->m_hitIndex[psNum])
		{
            if(cmpRslts->m_hitOrMiss[psNum] == NLMDEV_MISS)
            {
                /* If Compare Result is MISS and expected Result is HIT then display error */
                NlmCm__printf("\n\n\t Compare Operation Results Differ for PS Num %d", psNum);
                NlmCm__printf("\n\n Got Miss (Expected Hit At Device Id %d, Index %d)",                    
                    			expectedCmpRslts->m_hitDevId[psNum], expectedCmpRslts->m_hitIndex[psNum]);

                return NLMERR_FAIL;
            }

        	/* If Compare Result Index or Dev Id differs from the expected then display error */
            NlmCm__printf("\n\n\t Compare Operation Results Differ for PS Num %d", psNum);
            NlmCm__printf("\n\n\t Got Hit At Device Id %d, Index %d, (Expected Hit At Device Id %d, Index %d)",
                        	cmpRslts->m_hitDevId[psNum], cmpRslts->m_hitIndex[psNum],
                        	expectedCmpRslts->m_hitDevId[psNum], expectedCmpRslts->m_hitIndex[psNum]);

            return NLMERR_FAIL;
        }
    }

    return NLMERR_OK;
}


/* This function configures Block number 0 to g_maxBlks with 80b */
NlmErrNum_t
NlmDevMgrRefApp_Configure80bBlks(
	void *dev_p
	)
{
    NlmErrNum_t errNum;   
	nlm_u8 blkNum = 0;
	
	for(blkNum = 0; blkNum < g_maxBlks; blkNum++)
	{
        if((errNum = NlmDevMgrRefApp_InitializeBlk(dev_p, blkNum, 80)) != NLMERR_OK)
			return errNum;
	}

    return NLMERR_OK;
}

/* This function configures LTR to perform 80b searches */
NlmErrNum_t
NlmDevMgrRefApp_ConfigureLTRSearches(
	void *dev_p
	)
{
    NlmErrNum_t errNum;
    NlmDevMgrRefAppSrchAttrs srchAttrs;
	nlm_u8 blkNum = 0;
  
    NlmCm__memset(&srchAttrs, 0, sizeof(NlmDevMgrRefAppSrchAttrs));

	for(blkNum = 0; blkNum < g_maxBlks; blkNum++)
	{
		srchAttrs.m_blkEnable[blkNum] = NLMDEV_ENABLE;
	    srchAttrs.m_rsltPortBlkMap[blkNum].m_psNum = NLMDEV_PARALLEL_SEARCH_0;
		srchAttrs.m_keySBMap[blkNum/NLMDEV_NUM_BLKS_PER_SUPER_BLOCK].m_keyNum = NLMDEV_KEY_1;
	}    

    /* Key for srch#0 is from Bits[79:0] */
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_startByte[0] = 0;
    srchAttrs.m_keyConstructMap[NLMDEV_KEY_1].m_numOfBytes[0] = 10;
         
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_0] = NLMDEV_NO_MASK_BMR_NUM;
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_1] = NLMDEV_NO_MASK_BMR_NUM;
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_2] = NLMDEV_NO_MASK_BMR_NUM;
    srchAttrs.m_bmrNum[NLMDEV_PARALLEL_SEARCH_3] = NLMDEV_NO_MASK_BMR_NUM;    

    if((errNum = NlmDevMgrRefApp_ConfigSearch(dev_p, NLM_DEVMGR_REFAPP_LTR_NUM, &srchAttrs)) != NLMERR_OK)
        return errNum;       
            
    return NLMERR_OK;
}


/* 
 * This Add-thread entry function adds 80b entries to the device until Compare-thread is
 * done with pre-configured number of searches. Entries are added starting from block-0
 * till pre-configured number of blocks. This addition could wrap around from last block
 * to block-0 if searches by Search-thread is not done yet.
 */
NlmErrNum_t
NlmDevMgrRefApp_InitializeDatabase(
	void *threadNum
	)
{
    nlm_u8 data[NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;
	nlm_u32 value_80b = 0, cpuid = 0;
	nlm_u16 address = 0;
    nlm_u8  blkNum = 0;
    NlmDevMgr *devMgr_p = (NlmDevMgr*)(g_refAppData_p->devMgr_p); 
	NlmReasonCode reasonCode = 0;

#ifdef NLM_NETOS
	nlm_u32 threadID = 0;
	cpuid = getpid();
#else
	cpuid = NLM_DEVMGR_REFAPP_ADD_THREAD_CPU;
#endif
	
	errNum = NlmCmMt__RegisterThread( *(nlm_u32*)threadNum, cpuid, &reasonCode);
	if(errNum != NLMERR_OK)
	{
		NlmCm__printf(" *** Could not register Add-Thread ***\n");

		return errNum;
	}
		
#ifdef NLM_NETOS
	threadID = NlmCmMt__GetThreadID(cpuid);
#endif

	g_refAppData_p->entryNum = 0;
	value_80b = NLM_DEVMGR_REFAPP_START_VALUE;

	if(NlmDevMgrRefApp_BindThreadToCore(cpuid) != 0) 
	{
        NlmCm__printf("\n\t ***  Could not bound Add Thread to CPU-%d. ***\n", cpuid);

		return NLMERR_FAIL;
	}

	NlmCm__printf("\t Add-thread [threadID = %d] bound to CPU-%d. \n", threadID, cpuid);

	/* Keep adding entries to the device till Search-thread is done with pre-configured number
	 * of searches.
	 */
    while(g_cmdNotDone)
	{
		for(blkNum = 0; blkNum < g_maxBlks; blkNum++)
		{
			NlmCm__memset(data, 0, NLMDEV_AB_WIDTH_IN_BYTES);
			NlmCm__memset(mask, 0, NLMDEV_AB_WIDTH_IN_BYTES); 

			for(address = 0; address < NLMDEV_AB_DEPTH; address++)
			{
				/* LSB 4 bytes of the "data" field will have the value */
				WriteBitsInArray(data, NLMDEV_AB_WIDTH_IN_BYTES, 31, 0, value_80b);

				/* Add the entry to the database */
				if((errNum = NlmDevMgrRefApp_AddEntry(g_refAppData_p->dev_p,
													  blkNum,
													  address,
													  data,
													  mask)) != NLMERR_OK)
				{
#ifdef NLM_NETOS
					checkJoin[threadID] = 1;
#endif

					return errNum;  
				}

				NlmCmMt__SpinLock(&devMgr_p->m_spinLock);
				g_refAppData_p->entryNum++;
				NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);

				value_80b++;
           
		    	if(g_cmdNotDone == NlmFalse)
				{
					NlmCm__printf("\n\t Add-thread done. Exiting. \n");
#ifdef NLM_NETOS
					checkJoin[threadID] = 1;
#endif

					return NLMERR_OK;
				}

				if((g_refAppData_p->entryNum % 1000) == 0)
					NlmCm__printf("\n\t - %d entries are added to device (compares done so far: %d) ", g_refAppData_p->entryNum, cmpNum);
			}
    	}
  
        NlmCm__printf("\n\n\t ======== Wrap around happenned (Entries: %d, Compares: %d) ========\n\n", g_refAppData_p->entryNum, cmpNum);

    	/* reset the values, iterate again from begenning */
		value_80b = NLM_DEVMGR_REFAPP_START_VALUE;
		g_refAppData_p->entryNum = 0;
	} /* while */
	
	NlmCm__printf("\n\t ***  Add Entry Thread done ... exiting... ***\n");

#ifdef NLM_XLP
	NlmXlpXpt__FlushResponses(g_refAppData_p->xpt_p);
#endif

#ifdef NLM_NETOS
	checkJoin[threadID] = 1;
#endif

    return NLMERR_OK;
}


/* 
 * This function performs search operations. Compare operations are performed
 * each producing a search result performed on 80b blocks. 
 */
NlmErrNum_t 
NlmDevMgrRefApp_PerformLTRSearches(
	void *threadNum
	)
{
    NlmDevMgr *devMgr_p = (NlmDevMgr*)(g_refAppData_p->devMgr_p); 
	NlmDevCmpRslt cmpRslts, expectedCmpRslts;
	NlmReasonCode reasonCode = 0;
    NlmErrNum_t errNum;
    nlm_u32 address = 0, refValue_80b = NLM_DEVMGR_REFAPP_START_VALUE;
	nlm_u32 cpuid = 1, value_80b = 0;
	nlm_u16 cbAddr = 0;
    nlm_u8 compareData[NLMDEV_MAX_CB_WRITE_IN_BYTES];
	nlm_u8 cbDatalen = 0, ltrNum = 0;

#ifdef NLM_NETOS
	nlm_u32 threadID = 0;
	cpuid = getpid();
#else
	cpuid = NLM_DEVMGR_REFAPP_SEARCH_THREAD_CPU;
#endif
	
	errNum = NlmCmMt__RegisterThread( *(nlm_u32*)threadNum, cpuid, &reasonCode);
	if(errNum != NLMERR_OK)
	{
		NlmCm__printf(" *** Could not register Search-Thread ***\n");

		return errNum;
	}
		
#ifdef NLM_NETOS
	threadID = NlmCmMt__GetThreadID(cpuid);
#endif

	/* bind search thread to CPU 1 */
	if(NlmDevMgrRefApp_BindThreadToCore(cpuid) != 0)
	{
        NlmCm__printf("\n\t ***  Could not bound Compare Thread to CPU-%d. ***\n", cpuid);

		return NLMERR_FAIL;
	}

	NlmCm__printf("\t Compare-thread [threadID = %d] bound to CPU-%d. \n\n", threadID, cpuid);

    ltrNum = NLM_DEVMGR_REFAPP_LTR_NUM;
	NlmCm__memset(compareData, 0, 8 * NLMDEV_CB_WIDTH_IN_BYTES);

	while(cmpNum < NLM_DEVMGR_REFAPP_NUM_COMPARES)
	{
		/* Wait till atleast one entry is added by Add-thread */
		NlmCmMt__SpinLock(&devMgr_p->m_spinLock);
		if(g_refAppData_p->entryNum <= 0)
		{
			NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);

		    continue;
		}
	
		address = (nlm_u32)( ( (rand() << 16) | rand() ) % g_refAppData_p->entryNum );
		NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);

		value_80b = refValue_80b + address;
      
		/* Perform compares for the entries we added */
		{   
			WriteBitsInArray(compareData, NLMDEV_MAX_CB_WRITE_IN_BYTES, 31, 0, value_80b); 

#ifdef NLM_XLP
	/* Incase of XlpXpt, context ids from startContexId till (startContextId + NLM_DEVMGR_REFAPP_THREAD_COUNT)
	 * are reserved for internal use by the XlpXpt. Use next available context address multiple of 8.
	 */
	{
		nlm_u32 tmp = 0; 

		//tmp = NLM_DEVMGR_REFAPP_CONTEXT_ADDRESS_START + NLM_DEVMGR_REFAPP_THREAD_COUNT;
		tmp = NLM_DEVMGR_REFAPP_CONTEXT_ADDRESS_END + 1; 
		tmp /= 8;

		cbAddr = 8 * tmp + 8;
	}
#else
			cbAddr		= 0;
#endif

			cbDatalen	= 8 * NLMDEV_CB_WIDTH_IN_BYTES;

			/* Even address do compare1; with odd address do compare2 searches */
            if( (errNum = NlmDevMgrRefApp_PerformCompare( g_refAppData_p->devMgr_p, ltrNum, 
														  address % 2, cbAddr, cbDatalen,
														  compareData, &cmpRslts)
														  ) != NLMERR_OK )
			{
				g_cmdNotDone = NlmFalse;
#ifdef NLM_NETOS
    			checkJoin[threadID] = 1;
#endif

            	return errNum;
			}

			/* Initialize the expected Results. Hit is expected on PS#0 */
			NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
			expectedCmpRslts.m_hitOrMiss[0] = NLMDEV_HIT;
			expectedCmpRslts.m_hitOrMiss[1] = NLMDEV_MISS;    
			expectedCmpRslts.m_hitOrMiss[2] = NLMDEV_MISS;    
			expectedCmpRslts.m_hitOrMiss[3] = NLMDEV_MISS;    
			expectedCmpRslts.m_hitDevId[0] = 0;        
			expectedCmpRslts.m_hitDevId[1] = 0;        
			expectedCmpRslts.m_hitDevId[2] = 0;        
			expectedCmpRslts.m_hitDevId[3] = 0;        
			expectedCmpRslts.m_hitIndex[0] = address;        
			expectedCmpRslts.m_hitIndex[1] = 0xffffffff;        
			expectedCmpRslts.m_hitIndex[2] = 0xffffffff;        
			expectedCmpRslts.m_hitIndex[3] = 0xffffffff;        

			/* Verify the compare results with the expected Results */
			if((errNum = NlmDevMgrRefApp_VerifyCompareResults(&cmpRslts, &expectedCmpRslts))!=NLMERR_OK)
			{
				NlmCm__printf("\n\t Search Results Differ for LTR Number %d, Compare Number %d", ltrNum, cmpNum);
				g_cmdNotDone = NlmFalse;
#ifdef NLM_NETOS
    			checkJoin[threadID] = 1;
#endif

				return errNum;
			}

			cmpNum++;

			if((cmpNum % 100) == 0)
				NlmCm__printf("\n\n\t --->>> %d searches are done <<<--- \n", cmpNum);
		}

    } /* while */

	NlmCm__printf("\n\t Total %d searches are done successfully \n ", cmpNum);
	NlmCm__printf("\n\t Search-thread done. Exiting \n");

#ifdef NLM_XLP
	NlmXlpXpt__FlushResponses(g_refAppData_p->xpt_p);
#endif

#ifdef NLM_NETOS
    checkJoin[threadID] = 1;
#endif
	
	g_cmdNotDone = NlmFalse;

    return NLMERR_OK;
}


int nlmdevmgr_mt_refapp_main(void)
{
    NlmCmAllocator alloc_bdy;
	NlmDevType devType;
    NlmDevId devId;
    NlmReasonCode reasonCode; 
    NlmErrNum_t errNum;
	NlmDevMgrRefApp refAppData;

	nlm_u8 th = 0;

#ifdef NLM_NETOS
	nlm_u32 sum = 0;
#endif

    NlmCm__printf ("\n\t Device Manager Multi-threaded Reference Application \n\n");

    g_cmdNotDone = NlmTrue;	
	g_refAppData_p = &refAppData;

	NlmCm__memset(&refAppData, 0, sizeof(refAppData));

#ifndef NLMPLATFORM_BCM
	/* Default Transport is the simulation mode */
	refAppData.xpt_mode = NLM_SIMULATION_MODE;

#ifdef NLM_XLP
	refAppData.xpt_mode = NLM_XLPXPT_MODE;
#endif
#else /* BCMPLATFORM */

	refAppData.xpt_mode = NLM_BCM_CALADAN3_MODE;
#endif /* BCMPLATFORM */

	/* 
	 * Default operation mode is non-Sahasra. Use NLMDEV_OPR_SAHASRA for Sahasra mode. 
	 * Entire device is available if it is non-Sahasra device. Array Column 15 is not
	 * available to application if it is Sahasra device.
	 */
	refAppData.opr_mode = NLMDEV_OPR_STANDARD;
	g_maxBlks = (refAppData.opr_mode == NLMDEV_OPR_STANDARD) ? 
					NLMDEV_NUM_ARRAY_BLOCKS : ((NLMDEV_NUM_ARRAY_BLOCKS - NLMDEV_NUM_AC15_BLOCKS) - 1 );

	if(refAppData.opr_mode == NLMDEV_OPR_STANDARD)
		devType = NLM_DEVTYPE_2;
	else
		devType = NLM_DEVTYPE_2_S;
		

	/* Create default Memory Allocator */
	g_refAppData_p->alloc_p = NlmCmAllocator__ctor(&alloc_bdy);
    NlmCmAssert((g_refAppData_p->alloc_p != NULL), "The memory allocator cannot be NULL!\n");

    /* As NlmCmAssert() is nulled out in the Release Build, add a check */
    if(g_refAppData_p->alloc_p == NULL)
    {
    	NlmCm__printf("\n Memory Allocator Init failed. Exiting.\n");

    	return -1;
    }

	/* Register the main therad here */
	if((errNum = NlmCmMt__RegisterThread(NLM_DEVMGR_REFAPP_THREAD_COUNT, 0, &reasonCode)) != NLMERR_OK)
	{
		NlmCm__printf("\n\t ***  Could not Register Main Thread to CPU-0. ***\n");

		return -1;
	}

    /* Create the Cmodel simulation Xpt Interface in standard operating mode. 
     * As Processor is serial device, speed mode and RBus mode are ignored 
     */  
	switch(refAppData.xpt_mode)
	{
#ifndef NLMPLATFORM_BCM
		case NLM_SIMULATION_MODE:
		{
			NlmCm__printf("\t Initializing C-Model Simulation Transport(Xpt) Interface \n\n");

		    if( (refAppData.xpt_p = NlmSimXpt__Create(g_refAppData_p->alloc_p,
													 devType,
													 0, /* speed mode, ignored for serial device */
												     1, /* Request Queue length */
													 1, /* Response Queue length */
													 refAppData.opr_mode,
													 0, /* speed mode, ignored for serial device */
													 0 /* Channel id */
													 ) ) == NULL)
		    {
		        NlmCm__printf("\n Err: SimXpt API 'NlmSimXpt__Create'");

		        return -1;
		    }

			break;
		}

#ifdef NLM_XLP
		case NLM_XLPXPT_MODE:
		{
			NlmCm__printf("\t Initializing XLP Transport(Xpt) Interface \n\n");

			if( (refAppData.xpt_p = NlmXlpXpt__Create( g_refAppData_p->alloc_p,
													  devType,
													  1, /* Request Queue length */ 
													  refAppData.opr_mode,
													  0, /* Channel id */
													  NLM_DEVMGR_REFAPP_CONTEXT_ADDRESS_START,
													  NLM_DEVMGR_REFAPP_CONTEXT_ADDRESS_END,
													  1, /* Link up */
													  1 
													  ) ) == NULL )
			{
				NlmCm__printf("\n Err: XLP XPT API 'NlmXlpXpt__Create'");

				return -1;
			}

			break;
		}
#endif
#endif /* NLMPLATFORM_BCM */
                case NLM_BCM_CALADAN3_MODE:
		default:
		{
            NlmCm__printf("\n\tInvalid Xpt mode specified\n");

			return -1;
		}
	}
 
    /* Create the instance of Device Manager */
    NlmCm__printf("\t Creating the instance of Device Manager \n");
    if((refAppData.devMgr_p = NlmDevMgr__create(g_refAppData_p->alloc_p, 
												refAppData.xpt_p,
												refAppData.opr_mode,
												&reasonCode
												)) == NULL)
    {
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__create', Reason Code -- %d", reasonCode);

        return -1;
    }

    /* Add a device to Device Manager */
    NlmCm__printf("\t Adding a Device to the Device Manager\n");
    if((refAppData.dev_p = NlmDevMgr__AddDevice(refAppData.devMgr_p, &devId, &reasonCode)) == NULL)
    {
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__AddDevice', Reason Code -- %d", reasonCode);

        return -1;
    }       

    /* Lock the device manager configuration */
    NlmCm__printf("\t Locking the Device Manager\n\n");
    if((errNum = NlmDevMgr__LockConfig(refAppData.devMgr_p, &reasonCode)) != NLMERR_OK)
    {
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LockConfig', Reason Code -- %d", reasonCode);

        return -1;
    }       

    /* ----------------------------------------------------------------------*/
	/*						Configure the blocks							 */
	/* ----------------------------------------------------------------------*/

    NlmCm__printf("\t Configuring 80b blocks  [0 - %d]\n", (g_maxBlks - 1) );
    if((errNum = NlmDevMgrRefApp_Configure80bBlks(refAppData.dev_p)) != NLMERR_OK)
	{
        NlmCm__printf("\n Err: Block configuration failed.");

        return -1;
	}

	/* ----------------------------------------------------------------------*/
	/*						Configure searches   							 */
	/* ----------------------------------------------------------------------*/

    NlmCm__printf("\t Configuring LTR for 80b  searches \n\n");
    if((errNum = NlmDevMgrRefApp_ConfigureLTRSearches(refAppData.dev_p)) != NLMERR_OK)
	{
        NlmCm__printf("\n Err: LTR Search configuration failed.");

        return -1;
	}

	/* ----------------------------------------------------------------------*/
	/*			Create Threads and do Add-Search operations					 */
	/* ----------------------------------------------------------------------*/

    NlmCm__printf("\t Creating Add and Compare threads \n");
	NlmCm__printf("\t -------------------------------- \n");
    if( ( errNum = NlmDevMgrRefApp_CreateThreads() ) != NLMERR_OK)
	{
        NlmCm__printf("\n Err: Thread creation failed.");

        return -1;
	}

	/* Main thread should wait till child threads are done with their processing. */
#ifdef NLM_NETOS
	while(1)
	{
		for(sum = 0, th = 0; th < NLM_DEVMGR_REFAPP_THREAD_COUNT; th++)
			sum += checkJoin[th];

		if(sum == NLM_DEVMGR_REFAPP_THREAD_COUNT)
			break;
	}
#else
	/* Join the threads */
	for(th = 0; th < NLM_DEVMGR_REFAPP_THREAD_COUNT; th++)
		NlmCmMt__JoinThread(&g_refAppData_p->devThrdId[th], NULL);
#endif

    /* Destroy the instance of Device Manager */
    NlmCm__printf("\n\n\t Destroying the instance of Device Manager");    
    NlmDevMgr__destroy(refAppData.devMgr_p);

    /* Destroy the xpt interface */
    NlmCm__printf("\n\t Destroying Xpt Interface");    
       
	switch(refAppData.xpt_mode)
	{
#ifdef NLMPLATFORM_BCM
		case NLM_SIMULATION_MODE:
			NlmSimXpt__destroy(refAppData.xpt_p); 
			break;

#ifdef NLM_XLP
		case NLM_XLPXPT_MODE:
			NlmXlpXpt__Destroy(refAppData.xpt_p);
			break;
#endif

#endif  /* NLMPLATFORM_BCM */
                case NLM_BCM_CALADAN3_MODE:
		default:
            NlmCm__printf("\n\tInvalid Xpt mode specified\n");
            break;
	}


    NlmCmAllocator__dtor(g_refAppData_p->alloc_p);
    
    NlmCm__printf("\n\n\t Device Manager Multi-threaded Reference Application Completed Successfully\n\n");

    return 0;
}

