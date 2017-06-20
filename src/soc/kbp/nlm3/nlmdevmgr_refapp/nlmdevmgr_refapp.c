/*
 * $Id: nlmgenerictblmgr.c,v 1.1.6.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include "nlmdevmgr_refapp.h"

/* Given Block number, start address, data and mask, this function adds database
  * entry to DBA block. Data/mask can be longer than 80b. This funtion gets the
  * block width from Shadow Memory for determining how many 80b writes it should
  * perform.
  */
NlmErrNum_t NlmDevMgrRefApp__AddDBAEntry(
    NlmDev *dev_p, 
    nlm_u16 blkNum,
    nlm_u16 startAddress,
    nlm_u8 *data,
    nlm_u8 *mask
    )
{
    NlmDevShadowDevice *shadowDev_p;    
    NlmDevABEntry *abEntry_p;
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
        {
            numberOf80bSegments = 1;

            break;
        }
           
            case NLMDEV_BLK_WIDTH_160:
        {
                    numberOf80bSegments = 2;

                    break;
        }

        case NLMDEV_BLK_WIDTH_320:
        {
            numberOf80bSegments = 4;;

                    break;
        }

            case NLMDEV_BLK_WIDTH_640:
        {
                    numberOf80bSegments = 8;

                    break;
        }
        
        default:
        {
                    NlmCmAssert(0, "Invalid Blk Width");
                    NlmCm__printf("\n\n Addition of AB Entry %d of Block Number %d failed", startAddress, blkNum);
                    NlmCm__printf("\n Block configured to invalid block width");

                    return NLMERR_FAIL;
        }
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
#ifndef NLMPLATFORM_BCM
            if((errNum = bcm_kbp_dm_dba_write(dev_p,
                                            NLMDEV_PORT_0,
                                                            blkNum, 
                                                            startAddress + segNum,
                                                            abEntry_p,
                                                            NLMDEV_DM,
                                                            &reasonCode)) != NLMERR_OK) 
#else
        if((errNum = kbp_dm_dba_write(dev_p,
                                            NLMDEV_PORT_0,
                                                            blkNum, 
                                                            startAddress + segNum,
                                                            abEntry_p,
                                                            NLMDEV_DM,
                                                            &reasonCode)) != NLMERR_OK) 
#endif

        {
                    NlmCm__printf("\n\n Addition of AB Entry %d of Block Number %d failed", startAddress + segNum, blkNum);
                    NlmCm__printf("\n Err: Device Manager API 'kbp_dm_dba_write', Reason Code -- %d", reasonCode);

                    return errNum;
            }
        }

        return NLMERR_OK;
}


/* This function adds Associated Data (AD) entry to UDA at given address.
  * AD entry can be longer than 32b. Number of 32b writes is given by the
  * caller in the numWrites param.
  */
NlmErrNum_t NlmDevMgrRefApp__AddADEntry(
    NlmDev  *dev_p,
    nlm_u32 udaStartAddr,
    nlm_u8  *adEntry,
    nlm_u8  numWrites
    )
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmReasonCode   reasonCode = NLMRSC_REASON_OK;
    nlm_u32 udaEndAddr, udaAddr;
    nlm_u8  data[NLMDEV_REG_LEN_IN_BYTES], *ptr;

    /* Error checking , re-visit */

    /* UDA write is 80b where only LSB 32b are valid. For AD length > 32b, writes are
     * split into different calls. MS 32b of adEntry should be written at higher
     * UDA address and LS 32b at lower UDA address. udaAddr passed to this functin
     * is the lower address.
     */
    udaEndAddr = (udaStartAddr + numWrites - 1);
    ptr = adEntry + ( 4 * (numWrites - 1) ); /* points to last 32b data at LS */

    for(udaAddr = udaStartAddr; udaAddr <= udaEndAddr; udaAddr++, ptr -= 4)
    {
        /* PIOWrite is used for UDA writes. PIOWrite is 80b. But only LBS 32b contains
         * valid UDA data.
         */
        NlmCm__memcpy( (data + 6), ptr, 4 );
#ifndef NLMPLATFORM_BCM
        errNum = bcm_kbp_dm_uda_write(dev_p,
                                    NLMDEV_PORT_0,
                                     udaAddr,
                                     data,
                                     0,
                                     &reasonCode);
#else
        errNum = kbp_dm_uda_write(dev_p,
                                    NLMDEV_PORT_0,
                                     udaAddr,
                                     data,
                                     0,
                                     &reasonCode);
#endif
        if( errNum != NLMERR_OK )
        {
            NlmCm__printf("\n\t kbp_dm_uda_write at address [%u] failed. ReasonCode = %d\n", udaAddr, reasonCode);

            return errNum;
        }
    }

    return errNum;
}


/* This function verifies the expected compare result against the obtained result. */
NlmErrNum_t NlmDevMgrRefApp__VerifyCmpResult(
    nlm_u8          cmpNum,
    NlmDevCmpResult *cmpResult,
    NlmDevCmpResult *expCmpResult
    )
{ 
    nlm_32 psNum, adLen;

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        if( cmpResult->m_resultValid[psNum] != expCmpResult->m_resultValid[psNum] )
        {
            NlmCm__printf("\t [Compare-%d] : resultValid differs for PS#%d\n", cmpNum, psNum);
            NlmCm__printf("\t Got = %d, Expected = %d\n",
                            cmpResult->m_resultValid[psNum], expCmpResult->m_resultValid[psNum]);

            return NLMERR_FAIL;
        }

        /* Nothing to verify if the result is not valid. */
        if(cmpResult->m_resultValid[psNum] == NLMDEV_RESULT_INVALID)
            continue;
        
        if( cmpResult->m_hitOrMiss[psNum] != expCmpResult->m_hitOrMiss[psNum] )
        {
            NlmCm__printf("\t [Compare-%d] : hitOrMiss differs for PS#%d\n", cmpNum, psNum);
            NlmCm__printf("\t Got = %d, Expected = %d\n",
                            cmpResult->m_hitOrMiss[psNum], expCmpResult->m_hitOrMiss[psNum]);

            return NLMERR_FAIL;
        }

        /* Nothing to verify if MISS is expected. */
        if( cmpResult->m_hitOrMiss[psNum] == NLMDEV_MISS )
            continue;

        /* Verify response type, devid, hit-index & AD */
        if( cmpResult->m_respType[psNum] != expCmpResult->m_respType[psNum] )
        {
            NlmCm__printf("\t [Compare-%d] : respType differs for PS#%d\n", cmpNum, psNum);
            NlmCm__printf("\t Got = %d, Expected = %d\n",
                            cmpResult->m_respType[psNum], expCmpResult->m_respType[psNum]);

            return NLMERR_FAIL;
        }

        if( cmpResult->m_hitDevId[psNum] != expCmpResult->m_hitDevId[psNum] )
        {
            NlmCm__printf("\t [Compare-%d] : hitDevId differs for PS#%d\n", cmpNum, psNum);
            NlmCm__printf("\t Got = %d, Expected = %d\n",
                            cmpResult->m_hitDevId[psNum], expCmpResult->m_hitDevId[psNum]);

            return NLMERR_FAIL;
        }

        if( cmpResult->m_hitIndex[psNum] != expCmpResult->m_hitIndex[psNum] )
        {
            NlmCm__printf("\t [Compare-%d] : hitIndex differs for PS#%d\n", cmpNum, psNum);
            NlmCm__printf("\t Got = %d, Expected = %d\n",
                            cmpResult->m_hitIndex[psNum], expCmpResult->m_hitIndex[psNum]);

            return NLMERR_FAIL;
        }

        /* Verify AD if it is valid. */
        if( cmpResult->m_respType[psNum] != NLMDEV_INDEX_AND_NO_AD )
        {
            if(cmpResult->m_respType[psNum] == NLMDEV_INDEX_AND_32B_AD)
                adLen = 4;
            else if(cmpResult->m_respType[psNum] == NLMDEV_INDEX_AND_64B_AD)
                adLen = 8;
            else if(cmpResult->m_respType[psNum] == NLMDEV_INDEX_AND_128B_AD)
                adLen = 16;
            else
                adLen = 32;

            if( NlmCm__memcmp( cmpResult->m_AssocData[psNum], expCmpResult->m_AssocData[psNum], adLen ) != 0 )
            {
                NlmCm__printf("\t [Compare-%d] : Associated Data differs for PS#%d\n", cmpNum, psNum);

                return NLMERR_FAIL;
            }
        }
    } /* for */

        return NLMERR_OK;
}


/* Given DBA Block width and AD length, this function finds shift direction and shift count */
void NlmDevMgrRefApp__GetShiftDirAndShiftCount(
    NlmDevBlockWidth dbaBlkWidth,
    NlmDevADLength   adLen,
    NlmDevShiftDir   *shiftDir,
    NlmDevShiftCount *shiftCount
    )
{
    switch( dbaBlkWidth )
    {
        case NLMDEV_BLK_WIDTH_80:
        {
            if( adLen == NLMDEV_ADLEN_32B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_64B )
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else if( adLen == NLMDEV_ADLEN_128B )
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }
            else
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_3;
            }

            break;
        }

        case NLMDEV_BLK_WIDTH_160:
        {
            if( adLen == NLMDEV_ADLEN_64B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_32B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else if( adLen == NLMDEV_ADLEN_128B )
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }

            break;
        }
        case NLMDEV_BLK_WIDTH_320:
        {
            if( adLen == NLMDEV_ADLEN_128B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_32B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }
            else if( adLen == NLMDEV_ADLEN_64B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }

            break;
        }
        case NLMDEV_BLK_WIDTH_640:
        {
            if( adLen == NLMDEV_ADLEN_256B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_32B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_3;
            }
            else if( adLen == NLMDEV_ADLEN_64B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }
            else /* 128B */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }

            break;
        }
    }
}


/* Configure DBA block to given width */
NlmErrNum_t NlmDevMgrRefApp__ConfigureDBABlock(
    NlmDev  *dev_p,
    nlm_u16 blkNum,
    nlm_u16 blkWidth
    )

{
    NlmDevShadowDevice *shadowDev_p;
    NlmDevBlockConfigReg *blkConfigReg_p;
    NlmErrNum_t errNum = NLMERR_OK;     
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;

    /* Get the shadow memory of blk config register for the specified blkNum */
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR( dev_p );
    blkConfigReg_p = &shadowDev_p->m_arrayBlock[blkNum].m_blkConfig;

    /* Initialize the Blk Config Reg Data */
    blkConfigReg_p->m_blockEnable = NLMDEV_ENABLE;

    switch( blkWidth )
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
#ifndef NLMPLATFORM_BCM
    if((errNum = bcm_kbp_dm_block_reg_write(dev_p,
                                                NLMDEV_PORT_0,
                                               blkNum,
                                               NLMDEV_BLOCK_CONFIG_REG,
                                               blkConfigReg_p,
                                               &reasonCode)) != NLMERR_OK)
#else
    if((errNum = kbp_dm_block_reg_write(dev_p,
                                                NLMDEV_PORT_0,
                                               blkNum,
                                               NLMDEV_BLOCK_CONFIG_REG,
                                               blkConfigReg_p,
                                               &reasonCode)) != NLMERR_OK)
#endif
    {    
        NlmCm__printf("\n\n Initialization of Block Number %d failed", blkNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_block_reg_write', Reason Code -- %d", reasonCode);

        return errNum;
    }
  
    return NLMERR_OK;
}


/*  This function sets UDA related fields in the BCR register. These fields are UDA Base Address,
  *  Shift count and Shift direction.
  */
NlmErrNum_t NlmDevMgrRefApp__SetUDAParamsInBCR(
    NlmDev  *dev_p,
    nlm_u16 dbaBlkNum,
    nlm_u16 dbaBlkWidth,
    nlm_u16 udaSbNum,
    nlm_u16 adLen
    )

{
    nlm_u32 baseAddr;
    NlmErrNum_t errNum = NLMERR_OK;     
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;

    /* Base address calculation.  */
    baseAddr = NLMDEV_NUM_SRAM_ENTRIES_PER_SB * udaSbNum;
    baseAddr >>= 9;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_uda_config(dev_p,
                                        NLMDEV_PORT_0,
                                        dbaBlkNum,
                                        dbaBlkWidth,
                                        baseAddr,
                                        adLen,
                                        &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_uda_config(dev_p,
                                        NLMDEV_PORT_0,
                                        dbaBlkNum,
                                        dbaBlkWidth,
                                        baseAddr,
                                        adLen,
                                        &reasonCode)) != NLMERR_OK ) 
#endif

    {    
        NlmCm__printf("\n\n UDA setting in BCR register failed for DBA Block-%d [Reason Code = %d]\n",
                        dbaBlkNum, reasonCode);
        return errNum;
    }

    return errNum;
}


/* This function confirures LTR#0 for CMP1 2x parallel search. */
NlmErrNum_t NlmDevMgrRefApp__ConfigureLTR0Searches(
    NlmDev *dev_p
    )
{
    NlmDevShadowDevice      *shadowDev_p;
    NlmDevBlkSelectReg      *blkSelectReg_p;
    NlmDevParallelSrchReg   *parallelSrchReg_p;
    NlmDevSuperBlkKeyMapReg *superBlkKeySelectReg_p;
    NlmDevKeyConstructReg   *keyConstructReg_p;
    NlmDevExtCap0Reg        *extCap0Reg_p;
    NlmDevOpCodeExtReg      *opCodeExtReg_p;
    NlmErrNum_t             errNum;
    NlmReasonCode           reasonCode;
    nlm_32                  psRegNum, blkSelectRegNum;
    nlm_u8                  ltrNum = 0;

    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);

    /* LTR-0 search configuration is as follows:
     *
     * 160b Table --> DBA Block 0 --> PS#0 --> Key#0 --> 32b AD --> UDA SB#0
     * 320b Table --> DBA Block 8 --> PS#1 --> Key#1 --> 64b AD --> UDA SB#1
     */

    /* Both DBA Block#0 and Block#8 belong to Block Select Reg#0 */
    blkSelectRegNum = 0;
    blkSelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[blkSelectRegNum];

    blkSelectReg_p->m_blkEnable[0] = NLMDEV_ENABLE;
    blkSelectReg_p->m_blkEnable[8] = NLMDEV_ENABLE;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                       NLMDEV_PORT_0,
                                       ltrNum, 
                                       NLMDEV_BLOCK_SELECT_0_LTR,
                                       blkSelectReg_p,
                                       &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                       NLMDEV_PORT_0,
                                       ltrNum, 
                                       NLMDEV_BLOCK_SELECT_0_LTR,
                                       blkSelectReg_p,
                                       &reasonCode)) != NLMERR_OK )  
#endif
    {    
            NlmCm__printf("\n\n Initialization of Block Select Reg#%d of LTR Number %d failed", blkSelectRegNum, ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

            return errNum;
    }

    /* Both DBA Block#0 and Block#8 belong to Parallel Search Reg#0 */
    psRegNum = 0;
    parallelSrchReg_p = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[psRegNum];

    parallelSrchReg_p->m_psNum[0] = NLMDEV_PARALLEL_SEARCH_0;
    parallelSrchReg_p->m_psNum[8] = NLMDEV_PARALLEL_SEARCH_1;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_PARALLEL_SEARCH_0_LTR,
                                        parallelSrchReg_p,
                                        &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_PARALLEL_SEARCH_0_LTR,
                                        parallelSrchReg_p,
                                        &reasonCode)) != NLMERR_OK )
#endif
    {    
        NlmCm__printf("\n\n Initialization of Parallel Search %d Register of LTR Number %d failed", psRegNum, ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* Key-0 is searched in DBA SB#0 and Key-1 is seached in DBA SB#1. */
    superBlkKeySelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_superBlkKeyMap;

    superBlkKeySelectReg_p->m_keyNum[0] = NLMDEV_KEY_0;
    superBlkKeySelectReg_p->m_keyNum[1] = NLMDEV_KEY_1;
    
#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                       NLMDEV_PORT_0,
                                       ltrNum, 
                                       NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                       superBlkKeySelectReg_p,
                                       &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                       NLMDEV_PORT_0,
                                       ltrNum, 
                                       NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                       superBlkKeySelectReg_p,
                                       &reasonCode)) != NLMERR_OK )
#endif
    {    
        NlmCm__printf("\n\n Initialization of Super Block Key Select Register of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    /* Key construction now. Key#0 is from [159:0] and Key#1 is from [479:320] in Master Key.
     * There are registers per key. KCR_0 itself is enough for the configurations as the 
     * keys are generated from contiguous bytes from Master Key.
     */
    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[0];

    keyConstructReg_p->m_startByteLoc[0] = 0;
    keyConstructReg_p->m_numOfBytes[0]   = 16;
    keyConstructReg_p->m_startByteLoc[1] = 16;
    keyConstructReg_p->m_numOfBytes[1]   = 4;

    /* Mark remaining segments as un-used (write 0x7F) */
    keyConstructReg_p->m_startByteLoc[2] = 0x7F;
    keyConstructReg_p->m_numOfBytes[2]   = 0;
    keyConstructReg_p->m_startByteLoc[3] = 0x7F;
    keyConstructReg_p->m_numOfBytes[3]   = 0;
    keyConstructReg_p->m_startByteLoc[4] = 0x7F;
    keyConstructReg_p->m_numOfBytes[4]   = 0;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_0_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_0_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_0_KCR_0 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[1];
    /* KCR_1 is un-used. Mark all 5 segments as un-used (write 0x7F). */
    keyConstructReg_p->m_startByteLoc[0] = 0x7F;
    keyConstructReg_p->m_numOfBytes[0]   = 0;
    keyConstructReg_p->m_startByteLoc[1] = 0x7F;
    keyConstructReg_p->m_numOfBytes[1]   = 0;
    keyConstructReg_p->m_startByteLoc[2] = 0x7F;
    keyConstructReg_p->m_numOfBytes[2]   = 0;
    keyConstructReg_p->m_startByteLoc[3] = 0x7F;
    keyConstructReg_p->m_numOfBytes[3]   = 0;
    keyConstructReg_p->m_startByteLoc[4] = 0x7F;
    keyConstructReg_p->m_numOfBytes[4]   = 0;

    /* Write to KEY_0_KCR_1_LTR */
#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_1_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_1_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_0_KCR_1 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }


    /* Key#1 configuration now. */
    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[2];
    
    keyConstructReg_p->m_startByteLoc[0] = 20;
    keyConstructReg_p->m_numOfBytes[0]   = 16;
    keyConstructReg_p->m_startByteLoc[1] = 36;
    keyConstructReg_p->m_numOfBytes[1]   = 16;
    keyConstructReg_p->m_startByteLoc[2] = 52;
    keyConstructReg_p->m_numOfBytes[2]   = 8;

    /* Mark remaining segments as un-used (write 0x7F) */
    keyConstructReg_p->m_startByteLoc[3] = 0x7F;
    keyConstructReg_p->m_numOfBytes[3]   = 0;
    keyConstructReg_p->m_startByteLoc[4] = 0x7F;
    keyConstructReg_p->m_numOfBytes[4]   = 0;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_1_KCR_0_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_1_KCR_0_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_1_KCR_0 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[3];
    /* KCR_1 is un-used. Mark all 5 segments as un-used (write 0x7F). */
    keyConstructReg_p->m_startByteLoc[0] = 0x7F;
    keyConstructReg_p->m_numOfBytes[0]   = 0;
    keyConstructReg_p->m_startByteLoc[1] = 0x7F;
    keyConstructReg_p->m_numOfBytes[1]   = 0;
    keyConstructReg_p->m_startByteLoc[2] = 0x7F;
    keyConstructReg_p->m_numOfBytes[2]   = 0;
    keyConstructReg_p->m_startByteLoc[3] = 0x7F;
    keyConstructReg_p->m_numOfBytes[3]   = 0;
    keyConstructReg_p->m_startByteLoc[4] = 0x7F;
    keyConstructReg_p->m_numOfBytes[4]   = 0;

    /* Write to KEY_1_KCR_1_LTR */
#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_1_KCR_1_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_1_KCR_1_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_1_KCR_1 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    /* Extended Capability Reg-0 for no-BMR and num_valid_results. */
    extCap0Reg_p = &shadowDev_p->m_ltr[ltrNum].m_extCap0;

    extCap0Reg_p->m_bmrSelect[NLMDEV_PARALLEL_SEARCH_0] = NLM_NO_MASK_BMR_NUM;
    extCap0Reg_p->m_bmrSelect[NLMDEV_PARALLEL_SEARCH_1] = NLM_NO_MASK_BMR_NUM;
    extCap0Reg_p->m_numOfValidSrchRslts                 = 2;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                                    extCap0Reg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                                    extCap0Reg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of EXT_CAP_REG_0 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    /* Both index and AD are needed with search result. PS#0 AD is 32b and PS#1 AD is 64b wide */
    opCodeExtReg_p = &shadowDev_p->m_ltr[ltrNum].m_opCodeExt;

    opCodeExtReg_p->m_resultType[NLMDEV_PARALLEL_SEARCH_0] = NLMDEV_INDEX_AND_AD;
    opCodeExtReg_p->m_ADLen[NLMDEV_PARALLEL_SEARCH_0]      = NLMDEV_ADLEN_32B;

    opCodeExtReg_p->m_resultType[NLMDEV_PARALLEL_SEARCH_1] = NLMDEV_INDEX_AND_AD;
    opCodeExtReg_p->m_ADLen[NLMDEV_PARALLEL_SEARCH_1]      = NLMDEV_ADLEN_64B;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_OPCODE_EXT_LTR,
                                    opCodeExtReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_OPCODE_EXT_LTR,
                                    opCodeExtReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of OPCODE_EXT_REG of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    return NLMERR_OK;
}


/* This function configures LTR#5 for 1x CMP2 search */
NlmErrNum_t NlmDevMgrRefApp__ConfigureLTR5Searches(
    NlmDev *dev_p
    )
{
    NlmDevShadowDevice      *shadowDev_p;
    NlmDevBlkSelectReg      *blkSelectReg_p;
    NlmDevParallelSrchReg   *parallelSrchReg_p;
    NlmDevSuperBlkKeyMapReg *superBlkKeySelectReg_p;
    NlmDevKeyConstructReg   *keyConstructReg_p;
    NlmDevExtCap0Reg        *extCap0Reg_p;
    NlmDevOpCodeExtReg      *opCodeExtReg_p;
    NlmErrNum_t             errNum;
    NlmReasonCode           reasonCode;
    nlm_32                  psRegNum, blkSelectRegNum;
    nlm_u8                  ltrNum = 5;

    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);

    /* LTR-5 search configuration is as follows:
     *
     * 640b Table --> DBA Block 16 --> PS#0 --> Key#0 --> 128b AD --> UDA SB#2
     */

    /* DBA Block#16 belongs to Block Select Reg#0 */
    blkSelectRegNum = 0;
    blkSelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[blkSelectRegNum];

    blkSelectReg_p->m_blkEnable[16] = NLMDEV_ENABLE;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_BLOCK_SELECT_0_LTR,
                                    blkSelectReg_p,
                                    &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_BLOCK_SELECT_0_LTR,
                                    blkSelectReg_p,
                                    &reasonCode)) != NLMERR_OK )  
#endif
    {    
            NlmCm__printf("\n\n Initialization of Block Select Reg#%d of LTR Number %d failed", blkSelectRegNum, ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

            return errNum;
    }

    /* DBA Block#16 belongs to Parallel Search Reg#0 */
    psRegNum = 0;
    parallelSrchReg_p = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[psRegNum];

    parallelSrchReg_p->m_psNum[16] = NLMDEV_PARALLEL_SEARCH_0;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_PARALLEL_SEARCH_0_LTR,
                                    parallelSrchReg_p,
                                    &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_PARALLEL_SEARCH_0_LTR,
                                    parallelSrchReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {    
        NlmCm__printf("\n\n Initialization of Parallel Search %d Register of LTR Number %d failed", psRegNum, ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* Key-0 is searched in DBA SB#2 */
    superBlkKeySelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_superBlkKeyMap;

    superBlkKeySelectReg_p->m_keyNum[2] = NLMDEV_KEY_0;
    
#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                    superBlkKeySelectReg_p,
                                    &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                    superBlkKeySelectReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {    
        NlmCm__printf("\n\n Initialization of Super Block Key Select Register of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* Key construction now. Key#0 is from [639:0]. There are registers per key.
     * KCR_0 itself is enough for the configurations as the key is generated from
     * contiguous bytes from Master Key.
     */
    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[0];

    keyConstructReg_p->m_startByteLoc[0] = 0;
    keyConstructReg_p->m_numOfBytes[0]   = 16;
    keyConstructReg_p->m_startByteLoc[1] = 16;
    keyConstructReg_p->m_numOfBytes[1]   = 16;
    keyConstructReg_p->m_startByteLoc[2] = 32;
    keyConstructReg_p->m_numOfBytes[2]   = 16;
    keyConstructReg_p->m_startByteLoc[3] = 48;
    keyConstructReg_p->m_numOfBytes[3]   = 16;
    keyConstructReg_p->m_startByteLoc[4] = 64;
    keyConstructReg_p->m_numOfBytes[4]   = 16;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_0_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_0_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_0_KCR_0 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[1];
    /* KCR_1 is un-used. Mark all 5 segments as un-used (write 0x7F). */
    keyConstructReg_p->m_startByteLoc[0] = 0x7F;
    keyConstructReg_p->m_numOfBytes[0]   = 0;
    keyConstructReg_p->m_startByteLoc[1] = 0x7F;
    keyConstructReg_p->m_numOfBytes[1]   = 0;
    keyConstructReg_p->m_startByteLoc[2] = 0x7F;
    keyConstructReg_p->m_numOfBytes[2]   = 0;
    keyConstructReg_p->m_startByteLoc[3] = 0x7F;
    keyConstructReg_p->m_numOfBytes[3]   = 0;
    keyConstructReg_p->m_startByteLoc[4] = 0x7F;
    keyConstructReg_p->m_numOfBytes[4]   = 0;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_1_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_KEY_0_KCR_1_LTR,
                                    keyConstructReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_0_KCR_1 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }
    
    /* Extended Capability Reg-0 for no-BMR and num_valid_results. */
    extCap0Reg_p = &shadowDev_p->m_ltr[ltrNum].m_extCap0;

    extCap0Reg_p->m_bmrSelect[NLMDEV_PARALLEL_SEARCH_0] = NLM_NO_MASK_BMR_NUM;
    extCap0Reg_p->m_numOfValidSrchRslts                 = 1;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                     ltrNum, 
                                     NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                                     extCap0Reg_p,
                                     &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                     ltrNum, 
                                     NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                                     extCap0Reg_p,
                                     &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of EXT_CAP_REG_0 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* Both index and AD are needed with search result. PS#0 AD is 128b wide */
    opCodeExtReg_p = &shadowDev_p->m_ltr[ltrNum].m_opCodeExt;

    opCodeExtReg_p->m_resultType[NLMDEV_PARALLEL_SEARCH_0] = NLMDEV_INDEX_AND_AD;
    opCodeExtReg_p->m_ADLen[NLMDEV_PARALLEL_SEARCH_0]      = NLMDEV_ADLEN_128B;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_OPCODE_EXT_LTR,
                                    opCodeExtReg_p,
                                    &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                    NLMDEV_PORT_0,
                                    ltrNum, 
                                    NLMDEV_OPCODE_EXT_LTR,
                                    opCodeExtReg_p,
                                    &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of OPCODE_EXT_REG of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    return NLMERR_OK;
}


/* This function configures LTR#75 for 1x CMP2 search on SMT-1 (2-Port, 2-SMT mode) */
NlmErrNum_t NlmDevMgrRefApp__ConfigureLTR75Searches(
    NlmDev *dev_p
    )
{
    NlmDevShadowDevice      *shadowDev_p;
    NlmDevBlkSelectReg      *blkSelectReg_p;
    NlmDevParallelSrchReg   *parallelSrchReg_p;
    NlmDevSuperBlkKeyMapReg *superBlkKeySelectReg_p;
    NlmDevKeyConstructReg   *keyConstructReg_p;
    NlmDevExtCap0Reg        *extCap0Reg_p;
    NlmDevOpCodeExtReg      *opCodeExtReg_p;
    NlmErrNum_t             errNum;
    NlmReasonCode           reasonCode;
    nlm_32                  psRegNum, blkSelectRegNum;
    nlm_u8                  ltrNum = 75;
    
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);

    /* LTR-75 search configuration is as follows:
     *
     * 640b Table --> DBA Block 16 --> PS#2 --> Key#2 --> 128b AD --> UDA SB#2
     */

    /* DBA Block#16 belongs to Block Select Reg#0 */
    blkSelectRegNum = 0;
    blkSelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[blkSelectRegNum];

    blkSelectReg_p->m_blkEnable[16] = NLMDEV_ENABLE;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum,
                                        NLMDEV_BLOCK_SELECT_0_LTR,
                                        blkSelectReg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum,
                                        NLMDEV_BLOCK_SELECT_0_LTR,
                                        blkSelectReg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#endif
    {    
        NlmCm__printf("\n\n Initialization of Block Select Reg#%d of LTR Number %d failed", blkSelectRegNum, ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* DBA Block#16 belongs to Parallel Search Reg#0. Search results are available on PS#2. */
    psRegNum = 0;
    parallelSrchReg_p = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[psRegNum];

    parallelSrchReg_p->m_psNum[16] = NLMDEV_PARALLEL_SEARCH_2;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_PARALLEL_SEARCH_0_LTR,
                                        parallelSrchReg_p,
                                        &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_PARALLEL_SEARCH_0_LTR,
                                        parallelSrchReg_p,
                                        &reasonCode)) != NLMERR_OK )
#endif
    {    
        NlmCm__printf("\n\n Initialization of Parallel Search %d Register of LTR Number %d failed", psRegNum, ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* Key-2 is searched in DBA SB#2 */
    superBlkKeySelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_superBlkKeyMap;

    superBlkKeySelectReg_p->m_keyNum[2] = NLMDEV_KEY_2;
    
#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum,
                                        NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                        superBlkKeySelectReg_p,
                                        &reasonCode)) != NLMERR_OK )
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum,
                                        NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                        superBlkKeySelectReg_p,
                                        &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of Super Block Key Select Register of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* Key construction now. Key#2 is from [639:0]. There are 2 registers per key.
     * KCR_0 itself is enough for the configurations as the key is generated from
     * contiguous bytes from Master Key.
     */
    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[0];

    keyConstructReg_p->m_startByteLoc[0] = 0;
    keyConstructReg_p->m_numOfBytes[0]   = 16;
    keyConstructReg_p->m_startByteLoc[1] = 16;
    keyConstructReg_p->m_numOfBytes[1]   = 16;
    keyConstructReg_p->m_startByteLoc[2] = 32;
    keyConstructReg_p->m_numOfBytes[2]   = 16;
    keyConstructReg_p->m_startByteLoc[3] = 48;
    keyConstructReg_p->m_numOfBytes[3]   = 16;
    keyConstructReg_p->m_startByteLoc[4] = 64;
    keyConstructReg_p->m_numOfBytes[4]   = 16;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_KEY_2_KCR_0_LTR,
                                        keyConstructReg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_KEY_2_KCR_0_LTR,
                                        keyConstructReg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_2_KCR_0 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    keyConstructReg_p = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[1];
    /* KCR_1 is un-used. Mark all 5 segments as un-used (write 0x7F). */
    keyConstructReg_p->m_startByteLoc[0] = 0x7F;
    keyConstructReg_p->m_numOfBytes[0]   = 0;
    keyConstructReg_p->m_startByteLoc[1] = 0x7F;
    keyConstructReg_p->m_numOfBytes[1]   = 0;
    keyConstructReg_p->m_startByteLoc[2] = 0x7F;
    keyConstructReg_p->m_numOfBytes[2]   = 0;
    keyConstructReg_p->m_startByteLoc[3] = 0x7F;
    keyConstructReg_p->m_numOfBytes[3]   = 0;
    keyConstructReg_p->m_startByteLoc[4] = 0x7F;
    keyConstructReg_p->m_numOfBytes[4]   = 0;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_KEY_2_KCR_1_LTR,
                                        keyConstructReg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_KEY_2_KCR_1_LTR,
                                        keyConstructReg_p,
                                        &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of KEY_2_KCR_1 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);
        return errNum;
    }

    /* Extended Capability Reg-0 for no-BMR and num_valid_results. */
    extCap0Reg_p = &shadowDev_p->m_ltr[ltrNum].m_extCap0;

    extCap0Reg_p->m_bmrSelect[NLMDEV_PARALLEL_SEARCH_2] = NLM_NO_MASK_BMR_NUM;
    extCap0Reg_p->m_numOfValidSrchRslts                 = 1;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                                        extCap0Reg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                                        extCap0Reg_p,
                                        &reasonCode)) != NLMERR_OK )
#endif
    {
        NlmCm__printf("\n\n Initialization of EXT_CAP_REG_0 of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    /* Both index and AD are needed with search result. PS#2 AD is 128b wide */
    opCodeExtReg_p = &shadowDev_p->m_ltr[ltrNum].m_opCodeExt;

    opCodeExtReg_p->m_resultType[NLMDEV_PARALLEL_SEARCH_2] = NLMDEV_INDEX_AND_AD;
    opCodeExtReg_p->m_ADLen[NLMDEV_PARALLEL_SEARCH_2]      = NLMDEV_ADLEN_128B;

#ifndef NLMPLATFORM_BCM
    if( (errNum = bcm_kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_OPCODE_EXT_LTR,
                                        opCodeExtReg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#else
    if( (errNum = kbp_dm_ltr_write(dev_p,
                                        NLMDEV_PORT_0,
                                        ltrNum, 
                                        NLMDEV_OPCODE_EXT_LTR,
                                        opCodeExtReg_p,
                                        &reasonCode)) != NLMERR_OK ) 
#endif
    {
        NlmCm__printf("\n\n Initialization of OPCODE_EXT_REG of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: DevMgr API 'kbp_dm_ltr_write', Reason Code -- %d", reasonCode);

        return errNum;
    }

    return NLMERR_OK;
}


/* This function adds 160b entries to DBA Block-0. */
NlmErrNum_t NlmDevMgrRefApp__Add160bEntries(
    NlmDev *dev_p
    )
{
    nlm_u32 udaAddr;
    nlm_u16 dbaAddr, dbaBlkNum;
    nlm_u8  iter, copyLen, numWrites = 1;
    nlm_u8  adEntry[4];
    nlm_u8 data[2 * NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[2 * NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;

    NlmCm__printf("\n\t Adding 160b entries to Block-0 (32b AD)\n");
    NlmCm__printf("\t =======================================\n");
    
    /* 32b AD entries are stored in UDA SB#0 from Blk#0, Row#0 */
    udaAddr  = NLMDEV_NUM_SRAM_ENTRIES_PER_SB * NLM_DEVMGR_REFAPP_32b_AD_UDA_SB;

    /* 160b entries are as follows:
     *
     * MSB 80b are pattern 0xababab...
     * LSB 80b are pattern iter_iter_....
     * Where iter is from 0 to number of entries added.
     */
    copyLen = NLMDEV_AB_WIDTH_IN_BYTES;
    for(iter = 0; iter < 100; iter++)
    {       
        NlmCm__memset( data, 0xab, copyLen );
        NlmCm__memset( (data + copyLen), iter, copyLen );

        /* Exact match, hence all 0s for mask */
        NlmCm__memset( mask, 0, 2 * copyLen );
                   
        dbaBlkNum = 0;
        dbaAddr = 2 * iter; /* 160b so, multiply by 2 */

        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp__AddDBAEntry(dev_p,
                                                  dbaBlkNum,
                                                  dbaAddr,
                                                  data, 
                                                  mask)) != NLMERR_OK)
        {
            return errNum;       
        }

        /* Add corresponding AD entry. 32b AD entry is "iter" itself. */
        NlmCm__memset( adEntry, iter, 4 );

        if( (errNum = NlmDevMgrRefApp__AddADEntry(dev_p,
                                                  udaAddr,
                                                  adEntry,
                                                  numWrites) ) != NLMERR_OK )
        {
            return errNum;
        }

        if( ( iter && (iter % 20)  == 0) )
            NlmCm__printf("\t Number of entries added = %d\n", iter);
        
        /* Next UDA address */
        udaAddr++;
    }

    NlmCm__printf("\t Number of entries added = %d\n", iter);

    return NLMERR_OK;
}


/* This function adds 320b entries to DBA Block-8. */
NlmErrNum_t NlmDevMgrRefApp__Add320bEntries(
    NlmDev *dev_p
    )
{
    nlm_u32 udaAddr;
    nlm_u16 dbaAddr, dbaBlkNum;
    nlm_u8  iter, copyLen, numWrites = 2;
    nlm_u8  adEntry[8];
    nlm_u8 data[4 * NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[4 * NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;

    NlmCm__printf("\n\t Adding 320b entries to Block-8 (64b AD)\n");
    NlmCm__printf("\t =======================================\n");
    
    /* 64b AD entries are stored in UDA SB#1 from Blk#0, Row#0 */
    udaAddr  = NLMDEV_NUM_SRAM_ENTRIES_PER_SB * NLM_DEVMGR_REFAPP_64b_AD_UDA_SB;

    /* 320b entries are as follows:
     *
     * MSB 160b are pattern 0xcdcdcd...
     * LSB 160b are pattern iter_iter_....
     * Where iter is from 0 to number of entries added.
     */
    for(iter = 0; iter < 100; iter++)
    {
        copyLen = 2 * NLMDEV_AB_WIDTH_IN_BYTES;
        
        NlmCm__memset( data, 0xcd, copyLen );
        NlmCm__memset( (data + copyLen), iter, copyLen );

        /* Exact match, hence all 0s for mask */
        NlmCm__memset( mask, 0, 2 * copyLen );
                   
        dbaBlkNum = 8;
        dbaAddr   = 4 * iter; /* 320b so multiply by 4 */

        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp__AddDBAEntry(dev_p,
                                                  dbaBlkNum,
                                                  dbaAddr,
                                                  data, 
                                                  mask)) != NLMERR_OK)
        {
            return errNum;       
        }

        /* Add corresponding AD entry. 64b AD entry is as follows:
         * MS 32b are 64_64_.... LS 32b are "iter" itself.
         */
        copyLen = 4;
        NlmCm__memset( adEntry, 64, copyLen );
        NlmCm__memset( (adEntry + copyLen), iter, copyLen );

        if( (errNum = NlmDevMgrRefApp__AddADEntry(dev_p,
                                                  udaAddr,
                                                  adEntry,
                                                  numWrites) ) != NLMERR_OK )
        {
            return errNum;
        }

        if( ( iter && (iter % 20)  == 0) )
            NlmCm__printf("\t Number of entries added = %d\n", iter);
        
            /* 64b hence add 2 for next UDA address */
        udaAddr +=  2;
    }

        NlmCm__printf("\t Number of entries added = %d\n", iter);

    return NLMERR_OK;
}


/* This function adds 640b entries to DBA Block-16. */
NlmErrNum_t NlmDevMgrRefApp__Add640bEntries(
    NlmDev *dev_p
    )
{
    nlm_u32 udaAddr;
    nlm_u16 dbaAddr, dbaBlkNum;
    nlm_u8  iter, copyLen, numWrites = 4;
    nlm_u8  adEntry[16];
    nlm_u8 data[8 * NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[8 * NLMDEV_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;

    NlmCm__printf("\n\t Adding 640b entries to Block-16 (128b AD)\n");
    NlmCm__printf("\t =========================================\n");
    
    /* 128b AD entries are stored in UDA SB#2 from Blk#0, Row#0 */
    udaAddr  = NLMDEV_NUM_SRAM_ENTRIES_PER_SB * NLM_DEVMGR_REFAPP_128b_AD_UDA_SB;

    /* 640b entries are as follows:
     *
     * MSB 320b are pattern 0xefefef...
     * LSB 320b are pattern iter_iter_....
     * Where iter is from 0 to number of entries added.
     */
    for(iter = 0; iter < 100; iter++)
    {
        copyLen = 4 * NLMDEV_AB_WIDTH_IN_BYTES;
        
        NlmCm__memset( data, 0xef, copyLen );
        NlmCm__memset( (data + copyLen), iter, copyLen );

        /* Exact match, hence all 0s for mask */
        NlmCm__memset( mask, 0, 2 * copyLen );
                   
        dbaBlkNum = 16;
        dbaAddr   = 8 * iter; /* 640b so multiply by 8 */

        /* Add the entry to the database */
        if((errNum = NlmDevMgrRefApp__AddDBAEntry(dev_p,
                                                  dbaBlkNum,
                                                  dbaAddr,
                                                  data, 
                                                  mask)) != NLMERR_OK)
        {
            return errNum;       
        }

        /* Add corresponding AD entry. 128b AD entry is as follows:
         * MS 64b are 128_128_128b.... LS 64b are "iter" itself.
         */
        copyLen = 8;
        NlmCm__memset( adEntry, 128, copyLen );
        NlmCm__memset( (adEntry + copyLen), iter, copyLen );

        if( (errNum = NlmDevMgrRefApp__AddADEntry(dev_p,
                                                  udaAddr,
                                                  adEntry,
                                                  numWrites) ) != NLMERR_OK )
        {
            return errNum;
        }

        if( ( iter && (iter % 20)  == 0) )
            NlmCm__printf("\t Number of entries added = %d\n", iter);
        
            /* 128b hence add 4 for next UDA address */
        udaAddr += 4;
        
    }

    NlmCm__printf("\t Number of entries added = %d\n", iter);
    
    return NLMERR_OK;
}


/* This function performs CMP1 2x parallel searches using LTR#0 */
NlmErrNum_t NlmDevMgrRefApp__PerformLTR0Searches(
    NlmDevMgr *devMgr_p
    )
{
    NlmDevCmpResult     cmpResult;
    NlmDevCmpResult     expCmpResult;
    NlmDevCtxBufferInfo cbInfo;
    NlmErrNum_t         errNum;
    NlmReasonCode       reasonCode;
    nlm_u8              *cmpData, *adData;
    nlm_u8              copyLen, cmpNum;

    NlmCm__printf("\n\t Performing LTR-0 seaches now\n");
    NlmCm__printf("\t ============================\n");

    /* 2x parallel search with results on PS#0 and PS#1. Do 100 searches */
    for(cmpNum = 0; cmpNum < 100; cmpNum++)
    {
        /*
         * MSB 320b is key of 320b table and next 160b is of 160b table.
         * See __Add160bEntries and __Add320bEntries for more details.
         */
        cmpData = cbInfo.m_data;

        /* 320b table key first */
        copyLen = 20;
        NlmCm__memset( cmpData, 0xcd, copyLen );
        NlmCm__memset( (cmpData + copyLen), cmpNum, copyLen );

        cmpData += 40;

        /* 160b table key next */
        copyLen = 10;
        NlmCm__memset( cmpData, 0xab, copyLen );
        NlmCm__memset( (cmpData + copyLen), cmpNum, copyLen );

        /* Use Context Address 8. (160 + 320) / 8 = 60 is the compare data length. */
        cbInfo.m_cbStartAddr = 8;
        cbInfo.m_datalen     = 60;
    
        /* Invoke Compare1 API to perform Compare Operation */
#ifndef NLMPLATFORM_BCM
        errNum = bcm_kbp_dm_cbwcmp1(devMgr_p,
                        NLMDEV_PORT_0,
                                     0, /* LTR Number */
                                     &cbInfo,
                                     &cmpResult,
                                     &reasonCode);
#else
        errNum = kbp_dm_cbwcmp1(devMgr_p,
                        NLMDEV_PORT_0,
                                     0, /* LTR Number */
                                     &cbInfo,
                                     &cmpResult,
                                     &reasonCode);
#endif
        if( errNum != NLMERR_OK )
        {
            NlmCm__printf("\n\n Err : kbp_dm_cbwcmp1 (LTR#0) failed (Compare#%d). ReasonCode = %d\n", cmpNum, reasonCode);

            return errNum;
        }

        /* Vefiy compare results now */
        NlmCm__memset(&expCmpResult, 0, sizeof(NlmDevCmpResult));

        /* PS#0 & PS#1 are valid. */
        expCmpResult.m_resultValid[0] = NLMDEV_RESULT_VALID;
        expCmpResult.m_resultValid[1] = NLMDEV_RESULT_VALID;
        expCmpResult.m_resultValid[2] = NLMDEV_RESULT_INVALID;
        expCmpResult.m_resultValid[3] = NLMDEV_RESULT_INVALID;

        expCmpResult.m_hitOrMiss[0] = NLMDEV_HIT;      
        expCmpResult.m_hitOrMiss[1] = NLMDEV_HIT;    
        expCmpResult.m_hitOrMiss[2] = NLMDEV_MISS;    
        expCmpResult.m_hitOrMiss[3] = NLMDEV_MISS;    

        /* 32b AD for PS#0 and 64b AD for PS#1 */
        expCmpResult.m_respType[0] = NLMDEV_INDEX_AND_32B_AD;
        expCmpResult.m_respType[1] = NLMDEV_INDEX_AND_64B_AD;

        expCmpResult.m_hitDevId[0] = 0;        
        expCmpResult.m_hitDevId[1] = 0;        

        /* 160b table entries are stored in DBA Block-0 and 320b entries are stored in DBA Block-8.
         * 160b table indexes are 0,2,4 and so on; hence multiply by 2.
         * 320b table indexes are 0,4,8 and so on; hence multiply by 4.
         */
        expCmpResult.m_hitIndex[0] = (0 << 12) | (cmpNum * 2);
        expCmpResult.m_hitIndex[1] = (8 << 12) | (cmpNum * 4);

        /* 32b AD value is "cmpNum" itself. For 64b AD, MS 32b are 64_64_.... LS 32b is "cmpNum_cmpNum_" */
        adData = expCmpResult.m_AssocData[0];
        NlmCm__memset( adData, cmpNum, 4 );

        adData = expCmpResult.m_AssocData[1];
        NlmCm__memset( adData, 64, 4 );
        NlmCm__memset( (adData + 4), cmpNum, 4 );

        /* Verify the compare results with the expected Results */
        if( NlmDevMgrRefApp__VerifyCmpResult( cmpNum, &cmpResult, &expCmpResult ) != NLMERR_OK )
            return NLMERR_FAIL;

        if( ( cmpNum && (cmpNum % 20)  == 0) )
            NlmCm__printf("\t Number of searches = %d\n", cmpNum);
    }

    NlmCm__printf("\t Number of searches = %d\n", cmpNum);
    
    return NLMERR_OK;
}


/* This function performs CMP2 1x parallel searches using LTR#5 */
NlmErrNum_t NlmDevMgrRefApp__PerformLTR5Searches(
    NlmDevMgr *devMgr_p
    )
{
    NlmDevCmpResult     cmpResult;
    NlmDevCmpResult     expCmpResult;
    NlmDevCtxBufferInfo cbInfo;
    NlmErrNum_t         errNum;
    NlmReasonCode       reasonCode;
    nlm_u8              *cmpData, *adData;
    nlm_u8              copyLen, cmpNum;

     NlmCm__printf("\n\t Performing LTR-5 seaches now\n");
    NlmCm__printf("\t ============================\n");
    
    /* 1x parallel search with results on PS#0. Do 100 searches */
    for(cmpNum = 0; cmpNum < 100; cmpNum++)
    {
        /*
         * MSB 320b are 0xefef... LSB 320b are cmpNum_cmpNum_ and so on
         * See __Add640bEntries for more details.
         */
        cmpData = cbInfo.m_data;

        copyLen = 40;
        NlmCm__memset( cmpData, 0xef, copyLen );
        NlmCm__memset( (cmpData + copyLen), cmpNum, copyLen );

        /* Use Context Address 0. 640b compare data */
        cbInfo.m_cbStartAddr = 0;
        cbInfo.m_datalen     = 80;
    
        /* Invoke Compare1 API to perform Compare Operation */
#ifndef NLMPLATFORM_BCM
        errNum = bcm_kbp_dm_cbwcmp2(devMgr_p,
                        NLMDEV_PORT_0,
                                     5, /* LTR Number */
                                     &cbInfo,
                                     &cmpResult,
                                     &reasonCode);
#else
        errNum = kbp_dm_cbwcmp2(devMgr_p,
                        NLMDEV_PORT_0,
                                     5, /* LTR Number */
                                     &cbInfo,
                                     &cmpResult,
                                     &reasonCode);
#endif
        if( errNum != NLMERR_OK )
        {
            NlmCm__printf("\n\n Err : kbp_dm_cbwcmp2 (LTR#5) failed (Compare#%d). ReasonCode = %d\n", cmpNum, reasonCode);

            return errNum;
        }

        /* Vefiy compare results now */
        NlmCm__memset(&expCmpResult, 0, sizeof(NlmDevCmpResult));

        /* Only PS#0 is valid. */
        expCmpResult.m_resultValid[0] = NLMDEV_RESULT_VALID;
        expCmpResult.m_resultValid[1] = NLMDEV_RESULT_INVALID;
        expCmpResult.m_resultValid[2] = NLMDEV_RESULT_INVALID;
        expCmpResult.m_resultValid[3] = NLMDEV_RESULT_INVALID;

        expCmpResult.m_hitOrMiss[0] = NLMDEV_HIT;
        expCmpResult.m_hitOrMiss[1] = NLMDEV_MISS;
        expCmpResult.m_hitOrMiss[2] = NLMDEV_MISS;
        expCmpResult.m_hitOrMiss[3] = NLMDEV_MISS;

        /* 128b AD for PS#0 */
        expCmpResult.m_respType[0] = NLMDEV_INDEX_AND_128B_AD;

        expCmpResult.m_hitDevId[0] = 0;        

        /* 640b table entries are stored in DBA Block-16.
         * Table indexes are 0,8,16 and so on; hence multiply by 8.
         */
        expCmpResult.m_hitIndex[0] = (16 << 12) | (cmpNum * 8);

        /* 128b AD, MS 64b are 128_128b_.... LS 64b is "cmpNum_cmpNum_" */
        adData = expCmpResult.m_AssocData[0];
        NlmCm__memset( adData, 128, 8 );
        NlmCm__memset( (adData + 8), cmpNum, 8 );

        /* Verify the compare results with the expected Results */
        if( NlmDevMgrRefApp__VerifyCmpResult( cmpNum, &cmpResult, &expCmpResult ) != NLMERR_OK )
            return NLMERR_FAIL;

        if( ( cmpNum && (cmpNum % 20)  == 0) )
            NlmCm__printf("\t Number of searches = %d\n", cmpNum);
    }

    NlmCm__printf("\t Number of searches = %d\n", cmpNum);
    
    return NLMERR_OK;
}


/* This function performs simultaneous searches on both SMTs */
NlmErrNum_t NlmDevMgrRefApp__PerformSMTSearches(
    NlmDevMgr *devMgr_p
    )
{
    NlmDevCmpResult     cmpResult;
    NlmDevCmpResult     expCmpResult;
    NlmDevCBWriteCmpParam cbInfo;
    NlmErrNum_t         errNum;
    NlmReasonCode       reasonCode;
    nlm_u8              *cmpData, *adData;
    nlm_u8              copyLen, cmpNum;

    NlmCm__printf("\n\t ================================\n");
    NlmCm__printf("\t Performing searches on both SMTs\n");
    NlmCm__printf("\t ================================\n\n");

    /* SMT-0 : 2x parallel search with results on PS#0 and PS#1. 
      * SMT-1 : 1x parallel search with results on PS#2.
      */
    NlmCm__memset( &cbInfo, 0, sizeof(cbInfo) );

    /* 2x CMP1 on SMT-0, 1x CMP2 on SMT-1 */
    cbInfo.m_cbInstType0 = NLMDEV_CB_INST_CMP1;
    cbInfo.m_cbInstType1 = NLMDEV_CB_INST_CMP2;

    cbInfo.m_ltrNum0 = 0;
    cbInfo.m_ltrNum1 = 75;
    
    for(cmpNum = 0; cmpNum < 100; cmpNum++)
    {
        /* Fill search data for SMT-0 searches.
         * MSB 320b is key of 320b table and next 160b is of 160b table.
         * See __Add160bEntries and __Add320bEntries for more details.
         */
        cmpData = cbInfo.m_cbData0.m_data;

        /* 320b table key first */
        copyLen = 20;
        NlmCm__memset( cmpData, 0xcd, copyLen );
        NlmCm__memset( (cmpData + copyLen), cmpNum, copyLen );

        cmpData += 40;

        /* 160b table key next */
        copyLen = 10;
        NlmCm__memset( cmpData, 0xab, copyLen );
        NlmCm__memset( (cmpData + copyLen), cmpNum, copyLen );

        /* Use Context Address 0. (160 + 320) / 8 = 60 is the compare data length. */
        cbInfo.m_cbData0.m_cbStartAddr = 0;
            cbInfo.m_cbData0.m_datalen       = 60;

            /* Fill search data for SMT-1 searches
         * MSB 320b are 0xefef... LSB 320b are cmpNum_cmpNum_ and so on
         * See __Add640bEntries for more details.
         */
        cmpData = cbInfo.m_cbData1.m_data;

        copyLen = 40;
        NlmCm__memset( cmpData, 0xef, copyLen );
        NlmCm__memset( (cmpData + copyLen), cmpNum, copyLen );

        /* Use Context Address 8. 640b compare data */
        cbInfo.m_cbData1.m_cbStartAddr = 8;
            cbInfo.m_cbData1.m_datalen       = 80;

            /* Invoke Compare API to perform Compare Operation on both SMTs */
#ifndef NLMPLATFORM_BCM
        errNum = bcm_kbp_dm_multi_compare(devMgr_p,
                                        NLMDEV_PORT_0,
                                        &cbInfo,
                                        &cmpResult,
                                        &reasonCode);
#else
            errNum = kbp_dm_multi_compare(devMgr_p,
                                        NLMDEV_PORT_0,
                                        &cbInfo,
                                        &cmpResult,
                                        &reasonCode);
#endif
        if( errNum != NLMERR_OK )
        {
                    NlmCm__printf("\n\n Err : kbp_dm_multi_compare (2-SMT) failed (Compare#%d). ReasonCode = %d\n", 
                                    cmpNum, reasonCode);

                    return errNum;
        }
    
        /* Vefiy compare results now */
        NlmCm__memset(&expCmpResult, 0, sizeof(NlmDevCmpResult));

        /* SMT-0 search results on PS#0 & PS#1; SMT-1 result on PS#2 */
        expCmpResult.m_resultValid[0] = NLMDEV_RESULT_VALID;
        expCmpResult.m_resultValid[1] = NLMDEV_RESULT_VALID;
        expCmpResult.m_resultValid[2] = NLMDEV_RESULT_VALID;
        expCmpResult.m_resultValid[3] = NLMDEV_RESULT_INVALID;

        expCmpResult.m_hitOrMiss[0] = NLMDEV_HIT;
            expCmpResult.m_hitOrMiss[1] = NLMDEV_HIT;
            expCmpResult.m_hitOrMiss[2] = NLMDEV_HIT;    
            expCmpResult.m_hitOrMiss[3] = NLMDEV_MISS;    

        /* 32b AD for PS#0, 64b AD for PS#1 and 128b AD for PS#2. */
        expCmpResult.m_respType[0] = NLMDEV_INDEX_AND_32B_AD;
        expCmpResult.m_respType[1] = NLMDEV_INDEX_AND_64B_AD;
        expCmpResult.m_respType[2] = NLMDEV_INDEX_AND_128B_AD;

            expCmpResult.m_hitDevId[0] = 0;
            expCmpResult.m_hitDevId[1] = 0;
            expCmpResult.m_hitDevId[2] = 0;

        /* 160b table entries are stored in DBA Block-0 and 320b entries are stored in DBA Block-8.
         * 160b table indexes are 0,2,4 and so on; hence multiply by 2.
         * 320b table indexes are 0,4,8 and so on; hence multiply by 4.
         * 
         * 640b table entries are stored in DBA Block-16.
         * Table indexes are 0,8,16 and so on; hence multiply by 8.
         */
            expCmpResult.m_hitIndex[0] = (0 << 12) | (cmpNum * 2);
            expCmpResult.m_hitIndex[1] = (8 << 12) | (cmpNum * 4);
            expCmpResult.m_hitIndex[2] = (16 << 12) | (cmpNum * 8);

        /* 32b AD value is "cmpNum" itself. */
        adData = expCmpResult.m_AssocData[0];
        NlmCm__memset( adData, cmpNum, 4 );

        /* For 64b AD, MS 32b are 64_64_.... LS 32b is "cmpNum_cmpNum_" */
        adData = expCmpResult.m_AssocData[1];
        NlmCm__memset( adData, 64, 4 );
        NlmCm__memset( (adData + 4), cmpNum, 4 );

        /* 128b AD, MS 64b are 128_128b_.... LS 64b is "cmpNum_cmpNum_"  */
        adData = expCmpResult.m_AssocData[2];
        NlmCm__memset( adData, 128, 8 );
        NlmCm__memset( (adData + 8), cmpNum, 8 );

            /* Verify the compare results with the expected Results */
            if( NlmDevMgrRefApp__VerifyCmpResult( cmpNum, &cmpResult, &expCmpResult ) != NLMERR_OK )
            return NLMERR_FAIL;

            if( ( cmpNum && (cmpNum % 20)  == 0) )
                    NlmCm__printf("\t Number of searches = %d\n", cmpNum);
        }

        NlmCm__printf("\t Number of searches = %d\n", cmpNum);
    
        return NLMERR_OK;
}


int main( nlm_32 argc, nlm_8 *argv[] )
{
    NlmCmAllocator *alloc_p;
    NlmCmAllocator      alloc_bdy;
    NlmPortMode portMode;
    NlmSMTMode  smtMode;
    void *xpt_p = NULL;
    void *devMgr_p = NULL;
    void *dev_p = NULL;   
    NlmDevId devId;
    NlmReasonCode reasonCode; 
    NlmErrNum_t errNum;
    NlmBool isSMT = NlmFalse;
    NlmDevConfigReg dcrReg;
    nlm_32 break_alloc_id = -1, cmdArgs = 1;

#ifdef NLMPLATFORM_BCM  
    if(0 != sal_console_init())
        return NLMERR_FAIL;
#endif

    NlmCm__printf ("\n\t Device Manager Application Reference Code Using \n");
    NlmCm__printf (" \t Device Manager Module. \n");

    NlmCmDebug__Setup(break_alloc_id, NLMCM_DBG_EBM_ENABLE);

#ifdef NLM_NETOS
    /* Default no-smt mode, set isSmtMode to run in SMT mode */
    /*isSMT =  NlmTrue;*/
    (void)cmdArgs;
#else
    while(cmdArgs < argc)
    {
        if( (NlmCm__strcmp(argv[cmdArgs], "-smt") == 0) )
        {
            isSMT = NlmTrue;
        }           
        cmdArgs++;
    }
#endif

    if(isSMT)
    {
        NlmCm__printf("\n\t----------------------------------------------\n");
        NlmCm__printf("\t Application is running in 1-PORT, 2-SMT mode.\n");
        NlmCm__printf("\t----------------------------------------------\n\n");
    }
    else
    {
        NlmCm__printf("\n\t----------------------------------------------\n");
        NlmCm__printf("\n\t Application is running in 1-PORT, no-SMT mode.\n");
        NlmCm__printf("\t----------------------------------------------\n\n");
    }
    
    /* create allocator */
    alloc_p = NlmCmAllocator__ctor(&alloc_bdy);
    NlmCmAssert((alloc_p != NULL), "The memory allocator cannot be NULL!\n");
    /* As NlmCmAssert() is nulled out in the Release Build, add a check */
    if(alloc_p == NULL)
    {
            NlmCm__printf("\n Memory Allocator Init failed. Exiting.\n");
            return NLMERR_FAIL;
        }

    /* Initialize Simulation Transport Interface here. */
    NlmCm__printf("\n\t Initializing C-Model Simulation Transport(SimXpt) Interface\n");
#ifndef NLMPLATFORM_BCM
    xpt_p = bcm_kbp_simxpt_init(alloc_p,
                              NLM_DEVTYPE_3,
                              1, /* Request Queue Length */
                              1, /* Result Queue Length  */
                              0  /* Channel id           */
                             );
#else
    xpt_p = kbp_simxpt_init(alloc_p,
                              NLM_DEVTYPE_3,
                              1, /* Request Queue Length */
                              1, /* Result Queue Length  */
                              0  /* Channel id           */
                             );
#endif

    if( xpt_p == NULL )
    {
        NlmCm__printf("\n Err: SimXpt API 'bcm_kbp_simxpt_init'");

        return NLMERR_FAIL;
    }

    if(isSMT)
    {
        /* Device is in 1-Port, 2-SMT mode */
        portMode = NLMDEV_SINGLE_PORT;
            smtMode = NLMDEV_DUAL_SMT_MODE;
    }
    else
    {
        /* Device is in 1-Port, no-SMT mode */
        portMode = NLMDEV_SINGLE_PORT;
            smtMode = NLMDEV_NO_SMT_MODE;
    }
    
    NlmCm__printf("\n\t Creating Device Manager instance\n");
#ifndef NLMPLATFORM_BCM
    devMgr_p = bcm_kbp_dm_init(alloc_p,
                                 xpt_p,
                                 NLM_DEVTYPE_3,
                                 portMode,
                                 smtMode, 
                                 0, /* ignored in 12K mode */
                                 &reasonCode
                                );
#else
    devMgr_p = kbp_dm_init(alloc_p,
                                 xpt_p,
                                 NLM_DEVTYPE_3,
                                 portMode,
                                 smtMode, 
                                 0, /* ignored in 12K mode */
                                 &reasonCode
                                );
#endif
    if( devMgr_p == NULL )
    {   
            NlmCm__printf("\n Err: Device Manager API 'kbp_dm_init', Reason Code -- %d", reasonCode);

            return NLMERR_FAIL;
    }

    /* Add a device to Device Manager */
        NlmCm__printf("\t Adding a Device to the Device Manager\n");
#ifndef NLMPLATFORM_BCM
        if((dev_p = bcm_kbp_dm_add_device(devMgr_p, &devId, &reasonCode)) == NULL)
#else
        if((dev_p = kbp_dm_add_device(devMgr_p, &devId, &reasonCode)) == NULL)
#endif        
        {
            NlmCm__printf("\n Err: Device Manager API 'kbp_dm_add_device', Reason Code -- %d", reasonCode);
            return NLMERR_FAIL;
        }

    /* Lock the device manager configuration */
    NlmCm__printf("\t Locking the Device Manager\n\n");
#ifndef NLMPLATFORM_BCM
    if((errNum = bcm_kbp_dm_lock_config(devMgr_p, &reasonCode)) != NLMERR_OK)
#else
    if((errNum = kbp_dm_lock_config(devMgr_p, &reasonCode)) != NLMERR_OK)
#endif
    {
        NlmCm__printf("\n Err: Device Manager API 'kbp_dm_lock_config', Reason Code -- %d", reasonCode);
        return errNum;
    }

    NlmCm__memset( &dcrReg, 0, sizeof(dcrReg) );

    /* Incase of 1-Port, 2-SMT mode, DBA Super Block 0 and 1 are in Bank-0 and SB#2 in Bank-1.
      * To program KBP in this mode, following bits must be set in DCR : Port-0 enable, dual bank mode.
      */
    if(isSMT)
    {
        dcrReg.m_dualBankMode = 1;
        dcrReg.m_port0Enable    = 1;

        /* SB#0 and #1 are in AC#0, SB#2 in AC#1. Bit  1 should be set as this AC belongs to Bank-1 */
        dcrReg.m_ACtoBankMapping = 0x2;
    }
    else
    {
        /* For 1-Port, no-SMT mode, no need to configure AC to Bank Mapping as all ACs belong to Bank-0
          * by default. Just enabled Port-0
          */
        dcrReg.m_port0Enable    = 1;
    }
#ifndef NLMPLATFORM_BCM
    errNum = bcm_kbp_dm_global_reg_write(dev_p, 
                                            NLMDEV_PORT_0, 
                                            NLMDEV_DEVICE_CONFIG_REG, 
                                            &dcrReg, 
                                            &reasonCode);
#else
    errNum = kbp_dm_global_reg_write(dev_p, 
                                            NLMDEV_PORT_0, 
                                            NLMDEV_DEVICE_CONFIG_REG, 
                                            &dcrReg, 
                                            &reasonCode);
#endif
    if(errNum != NLMERR_OK)
    {
        NlmCm__printf("\n Err : Write to DEVICE_CONFIG_REG failed. Reason Code -- %d\n", reasonCode);

        return errNum;
    }

    /* Configure DBA Blocks now.
     *
     * Block-0  is configured to 160b.
     * Block-8  is configured to 320b.
     * Block-16 is configured to 640b.
     */
    errNum = NlmDevMgrRefApp__ConfigureDBABlock( dev_p, 0, 160 );
    if(errNum != NLMERR_OK)
    {
        NlmCm__printf("\t DBA Block-0 configuration failed.\n");

        return NLMERR_FAIL;
    }
    NlmCm__printf("\t DBA Block-0 configured to 160b\n");

    errNum = NlmDevMgrRefApp__ConfigureDBABlock( dev_p, 8, 320 );
    if(errNum != NLMERR_OK)
    {
        NlmCm__printf("\t DBA Block-8 configuration failed.\n");

        return NLMERR_FAIL;
    }
    NlmCm__printf("\t DBA Block-8 configured to 320b\n");

    errNum = NlmDevMgrRefApp__ConfigureDBABlock( dev_p, 16, 640 );
    if(errNum != NLMERR_OK)
    {
        NlmCm__printf("\t DBA Block-0 configuration failed.\n");

        return NLMERR_FAIL;
    }
    NlmCm__printf("\t DBA Block-16 configured to 640b\n\n");
    
    /* Configure BCR for Associated Data entries now.
     *
     * 160b database uses 32b  AD,  UDA SB 0
     * 320b database uses 64b  AD,  UDA SB 1
     * 640b database uses 128b AD,  UDA SB 2
     */
    NlmDevMgrRefApp__SetUDAParamsInBCR( dev_p, 0, 160, 0, 32 );
    NlmDevMgrRefApp__SetUDAParamsInBCR( dev_p, 8, 320, 1, 64 );
    NlmDevMgrRefApp__SetUDAParamsInBCR( dev_p, 16,640, 2, 128 );

    
    if(isSMT)
    {
        /* Incase of 2-SMT mode, LTR-0 is used for SMT-0 searches and
          * LTR-75 for SMT-1 searches. 160b and 320b tables are searched on SMT-0 and 640b table
          * on SMT-1
          */
        NlmDevMgrRefApp__ConfigureLTR0Searches( dev_p );
        NlmDevMgrRefApp__ConfigureLTR75Searches( dev_p );
    }
    else
    {
        /* Incase of no-SMT mode, LTR-0 is used for 2x parallel search of 160b and 320b table.
          * LTR-5 is used to search 1x parallel of 640b table.
          */
        NlmDevMgrRefApp__ConfigureLTR0Searches( dev_p );
        NlmDevMgrRefApp__ConfigureLTR5Searches( dev_p );
    }
    
    /* Populate entries now */
    NlmDevMgrRefApp__Add160bEntries( dev_p );
        NlmDevMgrRefApp__Add320bEntries( dev_p );
        NlmDevMgrRefApp__Add640bEntries( dev_p );
    
    /* Perform searches now. */
    if(isSMT)
    {
        NlmDevMgrRefApp__PerformSMTSearches( devMgr_p );
    }
    else
    {
        NlmDevMgrRefApp__PerformLTR0Searches( devMgr_p );
        NlmDevMgrRefApp__PerformLTR5Searches( devMgr_p );
    }
    

    /* Destroy the instance of Device Manager */
    NlmCm__printf("\n\n\t Destroying Device Manager instance"); 
#ifndef NLMPLATFORM_BCM
    bcm_kbp_dm_destroy(devMgr_p);
#else
    kbp_dm_destroy(devMgr_p);
#endif

    /* Destroy the xpt interface */
    NlmCm__printf("\n\n\t Destroying C-Model Simulation Transport (SimXpt)  Interface"); 
#ifndef NLMPLATFORM_BCM
    bcm_kbp_simxpt_destroy(xpt_p); 
#else
    kbp_simxpt_destroy(xpt_p);
#endif

    NlmCmAllocator__dtor(alloc_p);

    /* check for memory leak */
    if(NlmCmDebug__IsMemLeak())
    {
        NlmCm__printf("\n\t ... Memory Leak occured ...\n\n");
        return 0;
    }
    NlmCm__printf("\n\n\n\t Device Manager Reference Application Completed Successfully\n\n");

    return 1;
}

