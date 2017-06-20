/*
 * $Id: nlmdevmgr_dump.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include "nlmarch.h"
#include "nlmdevmgr.h"
#include "nlmcmutility.h"
#include "nlmcmstring.h"

/* This file will dump the Shadow Memory (SM)/ or Device/C-model data to file*/
NlmBool isDevDump;


#define NLM_REDEF_REG_ADDR_LTR_EXTENDED1(x)       (0x0004005B + ((x) * 0x100)) /* defn ref from internal header */

/* log file names */
char smDumpFName[4][30] = 
                    {"kbp_shadow_dev0_log.txt", 
                     "kbp_shadow_dev0_log.txt", 
                     "kbp_shadow_dev0_log.txt",
                     "kbp_shadow_dev0_log.txt"
                    };

char devDumpFName[4][30] = 
                    {"kbp_device_dev0_log.txt", 
                     "kbp_device_dev0_log.txt", 
                     "kbp_device_dev0_log.txt",
                     "kbp_device_dev0_log.txt"
                    };


static nlm_u32
display_ltr_blk_sel_reg(
    NlmCmFile *fp,
    NlmDevBlkSelectReg* blkSelectReg_p,
    nlm_u32 regNr
    )
{
    nlm_u32 blkNum = 0;
    nlm_u32 numBlksSelected = 0;
    nlm_u32 numBlocksPerReg = NLMDEV_NUM_ARRAY_BLOCKS/4;

    for(blkNum = 0; blkNum < numBlocksPerReg; ++blkNum)
    {
        if(blkSelectReg_p->m_blkEnable[blkNum])
        {
            NlmCmFile__fprintf(fp, "%d, ", (regNr * numBlocksPerReg) + blkNum); 
            ++numBlksSelected;
        }
    }

    return numBlksSelected;
}

static void
display_ltr_sb_to_key_map(
    NlmCmFile *fp,
    NlmDevSuperBlkKeyMapReg *superBlkKeyMapReg_p
    )
{
    nlm_u32 superBlockNr = 0;
    nlm_u32 sbBlks = NLMDEV_NUM_SUPER_BLOCKS;

    NlmCmFile__fprintf(fp, "Super Block -> Key Mapping \n");

    for(superBlockNr = 0; superBlockNr < sbBlks; ++superBlockNr)
    {
        if(superBlockNr % 8 == 0 )
            NlmCmFile__fprintf(fp, "\n\t");

        NlmCmFile__fprintf(fp, "%2d -> %d, ", superBlockNr, superBlkKeyMapReg_p->m_keyNum[superBlockNr]);
        
    }
}

static void
display_ltr_prll_srch_reg(
    NlmCmFile *fp,
    NlmDevParallelSrchReg * psReg_p,
    nlm_u32 regNr
    )
{
    nlm_u32 blkNr = 0;
    nlm_u32 numBlksPerReg = NLMDEV_NUM_ARRAY_BLOCKS/8;
    
    for(blkNr = 0; blkNr < numBlksPerReg; ++blkNr)
    {
        if(blkNr % 8 == 0)
            NlmCmFile__fprintf(fp, "\n\t");

        NlmCmFile__fprintf(fp, "%3d -> %d, ", (regNr * numBlksPerReg) + blkNr, psReg_p->m_psNum[blkNr]);
    }
}


static void
display_range_encoded_type(
    NlmCmFile *fp,
    NlmDevRangeEncodingType rangeEncodingType
    )
{
    switch(rangeEncodingType)
    {
        case NLMDEV_3BIT_RANGE_ENCODING:
        {
            NlmCmFile__fprintf(fp, "NLMDEV_3BIT_RANGE_ENCODING \n");
            break;
        }

        case NLMDEV_2BIT_RANGE_ENCODING:
        {
            NlmCmFile__fprintf(fp, "NLMDEV_2BIT_RANGE_ENCODING \n");
            break;
        }

        case NLMDEV_NO_RANGE_ENCODING:
        {
            NlmCmFile__fprintf(fp, "NLMDEV_NO_RANGE_ENCODING \n");
            break;
        }

        default:
        {
            NlmCmFile__fprintf(fp, "Unknown = %d \n", rangeEncodingType);
            break;
        }
    }
}



static void
display_range_encoded_value(
    NlmCmFile *fp,
    NlmDevRangeEncodedValueBytes rangeEncodedValueBytes
    )
{
    switch(rangeEncodedValueBytes)
    {
        case NLMDEV_1BYTE_RANGE_ENCODED_VALUE:
        {
            NlmCmFile__fprintf(fp, "NLMDEV_1BYTE_RANGE_ENCODED_VALUE \n");
            break;
        }

        case NLMDEV_2BYTE_RANGE_ENCODED_VALUE:
        {
            NlmCmFile__fprintf(fp, "NLMDEV_2BYTE_RANGE_ENCODED_VALUE \n");
            break;
        }

        case NLMDEV_3BYTE_RANGE_ENCODED_VALUE:
        {
            NlmCmFile__fprintf(fp, "NLMDEV_3BYTE_RANGE_ENCODED_VALUE \n");
            break;
        }

        case NLMDEV_4BYTE_RANGE_ENCODED_VALUE:
        {
            NlmCmFile__fprintf(fp, "NLMDEV_4BYTE_RANGE_ENCODED_VALUE \n");
            break;
        }

        default:
        {
            NlmCmFile__fprintf(fp, "Unknown = %d \n", rangeEncodedValueBytes);
            break;
        }
    }
}

static void
display_ltr_range_ins0_reg(
    NlmCmFile *fp,
    NlmBool isSMT,
    NlmDevRangeInsertion0Reg *rangeInsertion0Reg_p
    )
{
    nlm_u32 keyNum = 0;

    NlmCmFile__fprintf(fp, "\n\nRange Insert0 Register \n");

    
    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range E encoding type = "); 
    else
        NlmCmFile__fprintf(fp, "  Range A encoding type = "); 

    display_range_encoded_type(fp, rangeInsertion0Reg_p->m_rangeAEncodingType);

    
    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range F encoding type = "); 
    else
        NlmCmFile__fprintf(fp, "  Range B encoding type = ");

    display_range_encoded_type(fp, rangeInsertion0Reg_p->m_rangeBEncodingType);


    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range E encoded bytes = "); 
    else
        NlmCmFile__fprintf(fp, "  Range A encoded bytes = ");

    display_range_encoded_value(fp, rangeInsertion0Reg_p->m_rangeAEncodedBytes);


    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range F encoded bytes = "); 
    else
        NlmCmFile__fprintf(fp, "  Range B encoded bytes = ");

    display_range_encoded_value(fp, rangeInsertion0Reg_p->m_rangeBEncodedBytes);


    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range E insert start byte: "); 
    else
        NlmCmFile__fprintf(fp, "  Range A insert start byte: ");
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
    {
        NlmCmFile__fprintf(fp, "  Key Num %d -> Start Byte %d, ", 
                    keyNum, rangeInsertion0Reg_p->m_rangeAInsertStartByte[keyNum]);
    }
    NlmCmFile__fprintf(fp, "\n");


    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range F insert start byte: "); 
    else
        NlmCmFile__fprintf(fp, "  Range B insert start byte: ");
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
    {
        NlmCmFile__fprintf(fp, "  Key Num %d -> Start Byte %d, ", 
                    keyNum, rangeInsertion0Reg_p->m_rangeBInsertStartByte[keyNum]);
    }
    NlmCmFile__fprintf(fp, "\n\n");

}



static void
display_ltr_range_ins1_reg(
    NlmCmFile *fp,
    NlmBool isSMT,
    NlmDevRangeInsertion1Reg *rangeInsertion1Reg_p
    )
{
    nlm_u32 keyNum = 0;

    NlmCmFile__fprintf(fp, "\n\nRange Insert1 Register \n");

    
    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range G encoding type = "); 
    else
        NlmCmFile__fprintf(fp, "  Range C encoding type = "); 

    display_range_encoded_type(fp, rangeInsertion1Reg_p->m_rangeCEncodingType);

    
    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range H encoding type = "); 
    else
        NlmCmFile__fprintf(fp, "  Range D encoding type = "); 

    display_range_encoded_type(fp, rangeInsertion1Reg_p->m_rangeDEncodingType);


    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range G encoded bytes = "); 
    else
        NlmCmFile__fprintf(fp, "  Range C encoded bytes = ");

    display_range_encoded_value(fp, rangeInsertion1Reg_p->m_rangeCEncodedBytes);


    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range H encoded bytes = "); 
    else
        NlmCmFile__fprintf(fp, "  Range D encoded bytes = ");

    display_range_encoded_value(fp, rangeInsertion1Reg_p->m_rangeDEncodedBytes);


    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range G insert start byte: "); 
    else
        NlmCmFile__fprintf(fp, "  Range C insert start byte: ");
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
    {
        NlmCmFile__fprintf(fp, "  Key Num %d -> Start Byte %d, ", 
                    keyNum, rangeInsertion1Reg_p->m_rangeCInsertStartByte[keyNum]);
    }
    NlmCmFile__fprintf(fp, "\n");

    if(isSMT == NlmTrue)
        NlmCmFile__fprintf(fp, "  Range H insert start byte: "); 
    else
        NlmCmFile__fprintf(fp, "  Range D insert start byte: ");
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
    {
        NlmCmFile__fprintf(fp, "  Key Num %d -> Start Byte %d, ", 
                    keyNum, rangeInsertion1Reg_p->m_rangeDInsertStartByte[keyNum]);
    }
    NlmCmFile__fprintf(fp, "\n\n");
}

static void
display_ltr_kcr_reg(
    NlmCmFile *fp,
    NlmDevKeyConstructReg *kcr_p
    )
{
    nlm_u32 segNr = 0;

    NlmCmFile__fprintf(fp, "  Start Byte  : ");
    
    for(segNr = 0; segNr < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
    {
        NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_startByteLoc[segNr]);
    }

    NlmCmFile__fprintf(fp, "\n");


    NlmCmFile__fprintf(fp, "  Num of Bytes: ");
    
    for(segNr = 0; segNr < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
    {
        /* Note: for device numBytes = 0 is 1 Byte.
                 to be consistant with SM we are printing 0 if 1 Byte */
        if(isDevDump)
        {
            if((kcr_p->m_numOfBytes[segNr] - 1) > 0)
                NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_numOfBytes[segNr]);
            else
                NlmCmFile__fprintf(fp, "%5d, ", (kcr_p->m_numOfBytes[segNr] - 1));
        }
        else
            NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_numOfBytes[segNr]);
    }

    NlmCmFile__fprintf(fp, "\n");

    NlmCmFile__fprintf(fp, "  Zero Fill   : ");
    
    for(segNr = 0; segNr < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
    {
        NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_isZeroFill[segNr]);
    }

    NlmCmFile__fprintf(fp, "\n\n");

}


static void
display_ltr_ext_cap_reg(
    NlmCmFile *fp, 
    NlmDevExtCap0Reg *extCapReg_p
    )
{
    nlm_u32 psNr = 0;

    NlmCmFile__fprintf(fp, "\n\n Extended Capability Register 0\n");


    for(psNr = 0; psNr < NLMDEV_NUM_PARALLEL_SEARCHES; ++psNr)
    {
        NlmCmFile__fprintf(fp, "  PS %d -> BMR %d, ", psNr, extCapReg_p->m_bmrSelect[psNr]);  
    }

    NlmCmFile__fprintf(fp,"\n  Range A extract start byte = %d \n", extCapReg_p->m_rangeAExtractStartByte);

    NlmCmFile__fprintf(fp, "  Range B extract start byte = %d \n", extCapReg_p->m_rangeBExtractStartByte);

    NlmCmFile__fprintf(fp, "  Range C extract start byte = %d \n", extCapReg_p->m_rangeCExtractStartByte);

    NlmCmFile__fprintf(fp, "  Range D extract start byte = %d \n", extCapReg_p->m_rangeDExtractStartByte);

    NlmCmFile__fprintf(fp, "  Number of valid search results = %d \n", extCapReg_p->m_numOfValidSrchRslts);

    
    /* 11K specific */
    /*for(psNr = 0; psNr < NLMDEV_NUM_PARALLEL_SEARCHES; ++psNr)
    {
        NlmCmFile__fprintf(fp, "PS %d -> Search type %d, ", psNr, extCapReg_p->m_searchType[psNr]);
    }*/

    NlmCmFile__fprintf(fp, "\n");
}

static void
display_ltr_opcode_extn_reg(
    NlmCmFile *fp,
    NlmDevOpCodeExtReg *opCodeReg_p
    )
{
    nlm_u32 keyNum = 0;

    NlmCmFile__fprintf(fp, "\n\nOpCode Extenstion Register \n");

    NlmCmFile__fprintf(fp, "  Result Type : ");
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
    {
        NlmCmFile__fprintf(fp, "\n\t  Key Num %d -> ", keyNum);
        if(opCodeReg_p->m_resultType[keyNum] == 0)
            NlmCmFile__fprintf(fp, " INDEX_ONLY");
        else
            NlmCmFile__fprintf(fp, " INDEX_WITH_AD");
    }
    NlmCmFile__fprintf(fp, "\n");

    NlmCmFile__fprintf(fp, "  AssoData Len: ");
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
    {
        NlmCmFile__fprintf(fp, "\n\t  Key Num %d -> ", keyNum);
        if(opCodeReg_p->m_resultType[keyNum] == 0)
            NlmCmFile__fprintf(fp, " NO_ASSO_DATA");
        else
            NlmCmFile__fprintf(fp, " %d Bit ASSO DATA", ((1 << opCodeReg_p->m_ADLen[keyNum]) * 32 ));
    }
    NlmCmFile__fprintf(fp, "\n");

    NlmCmFile__fprintf(fp, "  Local Opcode : %.2d ", opCodeReg_p->m_lclOpCode);

    NlmCmFile__fprintf(fp, "\n\n");
}


static NlmErrNum_t 
sm__pvt__disp_ltr_registers(
    NlmCmFile *fp, 
    NlmDev *dev_p
    )
{
    nlm_u8 ltrNum = 0;
    nlm_u32 regNr = 0, keyNr = 0, numBlksSelected = 0;
    nlm_u32 numBlkSelect     = NLMDEV_NUM_ARRAY_BLOCKS/64;
    nlm_u32 numPrlSrchRegs   = NLMDEV_NUM_ARRAY_BLOCKS/32;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    NlmBool isSMT = NlmFalse;

    NlmDevShadowDevice* shadowDevice_p = (NlmDevShadowDevice*)dev_p->m_shadowDevice_p;

    NlmCmFile__fprintf(fp, "\n\n__________________LTR Data _________________\n\n");

    for(ltrNum = 0; ltrNum < NLMDEV_MAX_NUM_LTRS; ++ltrNum)
    {
        /* incase of the SMT mode the ranges used by the LTR's
           LTRs :  0-63    A/B/C/D
           LTRs : 64-127   E/F/G/H
        */
        if(dev_p->m_devMgr_p->m_smtMode == NLMDEV_DUAL_SMT_MODE && ltrNum > 63)
            isSMT = NlmTrue;

        NlmCmFile__fprintf(fp, "LTR Num = %d \n\n", ltrNum);
        
        /*Print the block select register */
        NlmCmFile__fprintf(fp, "List of Enabled Blocks = ");

        numBlksSelected = 0;
        for(regNr = 0; regNr < numBlkSelect; ++regNr)
        {
            NlmDevBlkSelectReg* reg_p = NULL;
            NlmDevBlkSelectReg readReg = {{0},};

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_blockSelect[regNr]);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            if(isDevDump)
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_BLOCK_SELECT_0_LTR + regNr), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                    return errNum;
                }
                reg_p = &readReg;
            }

            numBlksSelected += display_ltr_blk_sel_reg(fp, reg_p, regNr);
        }

        if(numBlksSelected == 0)
            NlmCmFile__fprintf(fp, "None");

        NlmCmFile__fprintf(fp, "\n\n");

        /*Print the Super Blk - Key mapping */
        {
            NlmDevSuperBlkKeyMapReg* reg_p = NULL;
            NlmDevSuperBlkKeyMapReg readReg = {{0},};

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_superBlkKeyMap);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            if(isDevDump)
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_SUPER_BLK_KEY_MAP_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                    return errNum;
                }
                reg_p = &readReg;
            }

            display_ltr_sb_to_key_map(fp, reg_p);
        }
                
        /*Print the parallel search configuration register */
        {
            NlmCmFile__fprintf(fp, "\n\nBlock -> PS Mapping  ");

            for(regNr = 0; regNr < numPrlSrchRegs; ++regNr)
            {
                NlmDevParallelSrchReg* reg_p = NULL;
                NlmDevParallelSrchReg readReg = {{0},};

                /* have the SM pointer by default*/
                reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_parallelSrch[regNr]);
                
                /* if the device dump is enabled then read the register from device/c-model
                and print the data */
                if(isDevDump)
                {                   
                    /* Invoke the device manager API to read the specified LTR */
                    if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                        (NLMDEV_PARALLEL_SEARCH_0_LTR + regNr), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                    {
                        NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                        return errNum;
                    }
                    reg_p = &readReg;

                }
                display_ltr_prll_srch_reg(fp, reg_p, regNr);
            }
        }
        NlmCmFile__fprintf(fp, "\n\n");

        /*Print the RangeInsert0 Register */
        {
            NlmDevRangeInsertion0Reg* reg_p = NULL;
            NlmDevRangeInsertion0Reg readReg = {0,};

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_rangeInsert0);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            if(isDevDump)
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_RANGE_INSERTION_0_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                    return errNum;
                }
                reg_p = &readReg;
            }

            display_ltr_range_ins0_reg(fp, isSMT, reg_p);
        }

        /*Print the RangeInsert1 Register */
        {
            NlmDevRangeInsertion1Reg* reg_p = NULL;
            NlmDevRangeInsertion1Reg readReg = {0,};

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_rangeInsert1);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            if(isDevDump)
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_RANGE_INSERTION_1_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                    return errNum;
                }
                reg_p = &readReg;
            }

            display_ltr_range_ins1_reg(fp, isSMT, reg_p);
        }

        /*Print the KCR registers */
        NlmCmFile__fprintf(fp, "\n\n Key Construction Register (KCR) \n");
        for(keyNr = 0; keyNr < NLMDEV_NUM_KEYS ; ++keyNr)
        {
            NlmCmFile__fprintf(fp, " Key Nr = %d \n", keyNr);

            for(regNr = 0; regNr <  NLMDEV_NUM_OF_KCR_PER_KEY; ++regNr)
            {
                nlm_u32 index = (keyNr * NLMDEV_NUM_OF_KCR_PER_KEY) + regNr;

                NlmDevKeyConstructReg* reg_p = NULL;
                NlmDevKeyConstructReg readReg = {{0},};

                /* have the SM pointer by default*/
                reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_keyConstruct[index]);
                
                /* if the device dump is enabled then read the register from device/c-model
                and print the data */
                if(isDevDump)
                {                   
                    /* Invoke the device manager API to read the specified LTR */
                    if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                        (NLMDEV_KEY_0_KCR_0_LTR + index), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                    {
                        NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                        return errNum;
                    }
                    reg_p = &readReg;
                }

                display_ltr_kcr_reg(fp, reg_p);
            }

            NlmCmFile__fprintf(fp, "\n\n");
        }

        /* Extended Capability Register 0*/
        {
            NlmDevExtCap0Reg* reg_p = NULL;
            NlmDevExtCap0Reg readReg = {{0},};

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_extCap0);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            if(isDevDump)
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_EXT_CAPABILITY_REG_0_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                    return errNum;
                }
                reg_p = &readReg;
            }

            display_ltr_ext_cap_reg(fp, reg_p);
        }

        /* Extended Capability Register 1*/
        {
            NlmDevGeneralReg* reg_p = NULL;
            NlmDevGeneralReg readReg = {{0},};

            NlmCmFile__fprintf(fp, "\n\n Extended Capability Register 1 (Internal Register) :");
            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_internalReg1);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            if(isDevDump)
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_generic_reg_read(dev_p, 0,
                                        NLM_REDEF_REG_ADDR_LTR_EXTENDED1(ltrNum), 
                                        (void*)&readReg,
                                        &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_generic_reg_read",reasonCode);
                    return errNum;
                }
                reg_p = &readReg;
            }

            NlmCmFile__fprintf(fp, "  %.2x_%.2x__%.2x_%.2x_%.2x_%.2x__%.2x_%.2x_%.2x_%.2x \n", 
                reg_p->m_data[0], reg_p->m_data[1], reg_p->m_data[2], reg_p->m_data[3],
                reg_p->m_data[4], reg_p->m_data[5], reg_p->m_data[6], reg_p->m_data[7],
                reg_p->m_data[8], reg_p->m_data[9]);
        }

        /* OpCode Extension Register */
        {
            NlmDevOpCodeExtReg* reg_p = NULL;
            NlmDevOpCodeExtReg readReg = {{0},};

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_opCodeExt);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            if(isDevDump)
            {               
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_OPCODE_EXT_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_ltr_read",reasonCode);
                    return errNum;
                }
                reg_p = &readReg;
            }

            display_ltr_opcode_extn_reg(fp, reg_p);
        }
        NlmCmFile__fprintf(fp, "\n_____________________________________________________\n\n");
    }
    return errNum;
}


static NlmErrNum_t 
sm__pvt__disp_range_registers(
    NlmCmFile *fp, 
    NlmDev *dev_p
    )
{
    nlm_u32 regNr = 0;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    
    NlmDevShadowDevice* shadowDevice_p = (NlmDevShadowDevice*)dev_p->m_shadowDevice_p;

    NlmCmFile__fprintf(fp, "\n\n    Range Control Regs Data \n\n");

    for(regNr = 0; regNr < NLMDEV_NUM_RANGE_REG; ++regNr)
    {
        NlmDevRangeReg readReg = {{0},};
        NlmDevRangeReg *rangeReg_p = &(shadowDevice_p->m_rangeReg[regNr]);
        
        NlmCmFile__fprintf(fp, "\t\t  Addr 0x%5x: 0x", NLM_REG_RANGE_A_BOUNDS(0) + regNr);

        /* if the device dump is enabled then read the register from device/c-model
               and print the data */
        if(isDevDump)
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_range_reg_read(dev_p, 0, 0/* RJ to check SMT num*/, 
                (NLM_REG_RANGE_A_BOUNDS(0) + regNr), &readReg, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_range_reg_read",reasonCode);
                return errNum;
            }
            rangeReg_p = &readReg;
        }

        NlmCmFile__fprintf(fp, " %.2x_%.2x__%.2x_%.2x_%.2x_%.2x__%.2x_%.2x_%.2x_%.2x", 
                rangeReg_p->m_data[0], rangeReg_p->m_data[1], rangeReg_p->m_data[2], rangeReg_p->m_data[3],
                rangeReg_p->m_data[4], rangeReg_p->m_data[5], rangeReg_p->m_data[6], rangeReg_p->m_data[7],
                rangeReg_p->m_data[8], rangeReg_p->m_data[9]);

        NlmCmFile__fprintf(fp, "\n\n");
            
    }
    return errNum;
}


static NlmErrNum_t 
sm__pvt__disp_global_registers(
    NlmCmFile *fp, 
    NlmDev *dev_p
    )
{
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    nlm_u32 idx = 0;
    
    NlmDevShadowDevice* shadowDevice_p = (NlmDevShadowDevice*)dev_p->m_shadowDevice_p;

    NlmCmFile__fprintf(fp, "\n\n    Global Regs Data \n\n");

    /*Print the device config Register */
    {
        NlmDevConfigReg* reg_p = NULL;
        NlmDevConfigReg readReg = {0,};

        /* have the SM pointer by default*/
        reg_p = &(shadowDevice_p->m_global->m_devConfig);
        
        /* if the device dump is enabled then read the register from device/c-model
            and print the data */
        if(isDevDump)
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_global_reg_read(dev_p, 0, NLMDEV_DEVICE_CONFIG_REG, 
               (void*)&readReg, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_global_read",reasonCode);
                return errNum;
            }
            reg_p = &readReg;
        }

        NlmCmFile__fprintf(fp, "      Dev Config Regs Data");
        NlmCmFile__fprintf(fp, "\n\n            PORT#0                    :");
        (reg_p->m_port0Enable) ? (NlmCmFile__fprintf(fp, "ON")): (NlmCmFile__fprintf(fp, "OFF"));

        NlmCmFile__fprintf(fp, "\n\n            PORT#1                    :");
        (reg_p->m_port1Enable) ? (NlmCmFile__fprintf(fp, "ON")): (NlmCmFile__fprintf(fp, "OFF"));

        NlmCmFile__fprintf(fp, "\n\n            DUAL PORT                 :");
        (reg_p->m_dualPortMode) ? (NlmCmFile__fprintf(fp, "ON")): (NlmCmFile__fprintf(fp, "OFF"));

        NlmCmFile__fprintf(fp, "\n\n            DUAL SMT                  :");
        (reg_p->m_dualBankMode) ? (NlmCmFile__fprintf(fp, "ON")): (NlmCmFile__fprintf(fp, "OFF"));

        NlmCmFile__fprintf(fp, "\n\n            RANGE ENABLE              :");
        (reg_p->m_rangeEnable) ? (NlmCmFile__fprintf(fp, "ENABLED")): (NlmCmFile__fprintf(fp, "DISABLED"));

        NlmCmFile__fprintf(fp, "\n\n            IS_LAST_DEVICE            :");
        (reg_p->m_lastDevice) ? (NlmCmFile__fprintf(fp, "YES")): (NlmCmFile__fprintf(fp, "NO"));

        NlmCmFile__fprintf(fp, "\n\n            IS_FIRST_DEVICE ENABLE    :");
        (reg_p->m_firstDevice) ? (NlmCmFile__fprintf(fp, "YES")): (NlmCmFile__fprintf(fp, "NO"));

        NlmCmFile__fprintf(fp, "\n\n            DATA_PRTY_ERR_ENTR_INVAL  :");
        (reg_p->m_dbParityErrEntryInvalidate) ? (NlmCmFile__fprintf(fp, "ENABLED")): (NlmCmFile__fprintf(fp, "DISABLED"));

        NlmCmFile__fprintf(fp, "\n\n            SOFR_ERR_SCAN             :");
        (reg_p->m_softErrorScanEnable) ? (NlmCmFile__fprintf(fp, "ENABLED")): (NlmCmFile__fprintf(fp, "DISABLED"));

        NlmCmFile__fprintf(fp, "\n\n            PORT1_CTX_ID_SHT          : %.2x", reg_p->m_port1CtxIDShift);

        NlmCmFile__fprintf(fp, "\n\n            AC_2BANK_MAP              : %.2x", reg_p->m_ACtoBankMapping);

        NlmCmFile__fprintf(fp, "\n\n            CTX_BUFF_CFGP             : %.2x", reg_p->m_CBConfig);
    }

    NlmCmFile__fprintf(fp, "\n\n");

    /*Print the UDA config Register */
    {
        NlmDevUDAConfigReg* reg_p = NULL;
        NlmDevUDAConfigReg readReg = {{0},};

        /* have the SM pointer by default*/
        reg_p = &(shadowDevice_p->m_global->m_devUDAConfig);
        
        /* if the device dump is enabled then read the register from device/c-model
            and print the data */
        if(isDevDump)
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_global_reg_read(dev_p, 0, NLMDEV_UDA_CONFIG_REG, 
               (void*)&readReg, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_global_read",reasonCode);
                return errNum;
            }
            reg_p = &readReg;
        }

        NlmCmFile__fprintf(fp, "      Dev UDA Config Regs Data");
        
        for(idx = 0; idx < (NLMDEV_NUM_SRAM_SUPER_BLOCKS/4); idx++)
        {
            if( (idx%4) == 0)
                NlmCmFile__fprintf(fp, "\n\n             UDA SB: ");

            NlmCmFile__fprintf(fp, "  %.2d - %.2d : ", (idx*4) , ((idx * 4) + 3));
            (reg_p->m_uSBEnable[idx]) ? (NlmCmFile__fprintf(fp, "ENABLED")): (NlmCmFile__fprintf(fp, "DISABLED"));      
        }

        NlmCmFile__fprintf(fp, "\n\n");
    }
    
    return errNum;
}


static NlmErrNum_t
display_dba_bmr(
    NlmCmFile *fp,
    NlmDev *dev_p,
    nlm_u16 blkNum,
    nlm_u16 bmrNum,
    nlm_u8 maxNumChunks
    )
{
    nlm_u32 i =0;
    nlm_u8 *mask = NULL;
    nlm_u32 numOf80BitChunk = 0;
    NlmDevShadowAB *abInfo_p  = NULL;
    NlmDevBlockMaskReg readData = {{0},};
    NlmDevBlockMaskReg *data_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;

    if(maxNumChunks > NLMDEV_NUM_80BIT_SEGMENTS_PER_BMR)
        maxNumChunks = NLMDEV_NUM_80BIT_SEGMENTS_PER_BMR;
        
    for(numOf80BitChunk = maxNumChunks; numOf80BitChunk > 0; numOf80BitChunk--)
    {
        /* if the device dump is enabled then read the register from device/c-model
          and print the data */
        if(isDevDump)
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_block_reg_read(dev_p, 0, blkNum, 
                (NLMDEV_BLOCK_MASK_0_0_REG + bmrNum + (numOf80BitChunk -1)),
                (void*)&readData, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_block_reg_read",reasonCode);
                return errNum;
            }

            /* assign the read data */
            data_p = &readData;
        }
        else
        {
            abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];
            data_p = &abInfo_p->m_bmr[bmrNum ] [numOf80BitChunk -1];
        }

        mask = data_p->m_mask ;

        NlmCmFile__fprintf(fp, "0x");

        for(i = 0; i < NLMDEV_AB_WIDTH_IN_BYTES; ++i)
        {
            if(i == NLMDEV_AB_WIDTH_IN_BYTES - 1 &&  numOf80BitChunk == 1)
            {
                NlmCmFile__fprintf(fp, "%02x", *(mask+i));
            }
            else
            {
                NlmCmFile__fprintf(fp, "%02x_", *(mask + i));
            }
        }       
    }
    return errNum;
}


static NlmErrNum_t
display_dba_entry(
    NlmCmFile* fp,
    NlmDev *dev_p,
    nlm_u16 blkNum,
    nlm_u8 blkWidthIdx,
    nlm_u16 rowNum,
    nlm_u16 bmrNum,
    NlmBool combineBMR
    )
{
    nlm_u8 numOf80BitChunk;
    nlm_u8 *data;
    nlm_u8 *mask;
    nlm_u8 *bmrMask_p = NULL;
    nlm_u8 bmrAllow = 0;
    nlm_u8 dataBit = 0, maskBit = 0;

    nlm_32 bitNum = 0, byteNum = 0; 
    NlmDevABEntry abEntry = {{0},};
    nlm_u8 readData[10] = {0,};
    NlmDevShadowAB *abInfo_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;

    NlmCmFile__fprintf(fp,"%d",  rowNum );

/*
    NlmCmFile__fprintf(fp,", Blk %d", blkNum);

    if(combineBMR)
        NlmCmFile__fprintf(fp, ", Col %d", bmrNum);
*/

    abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];

    NlmCmFile__fprintf(fp, "\t : ");

    for(numOf80BitChunk = blkWidthIdx; numOf80BitChunk > 0; numOf80BitChunk--)
    {   
        data = abInfo_p->m_abEntry[rowNum + numOf80BitChunk - 1].m_data;
        mask = abInfo_p->m_abEntry[rowNum + numOf80BitChunk - 1].m_mask;

        /* if the device dump is enabled then read the register from device/c-model
            and print the data */
        if(isDevDump)
        {
            NlmCm__memset(&abEntry, 0, sizeof(NlmDevABEntry));

            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_dba_read(dev_p, 0, blkNum, (rowNum + numOf80BitChunk - 1), &abEntry, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_dba_read",reasonCode);
                return errNum;
            }

            /* assign the read data */
            data = &abEntry.m_data[0];
            mask = &abEntry.m_mask[0];
        }
        for(byteNum = 0; byteNum < NLMDEV_AB_WIDTH_IN_BYTES; ++byteNum)
        {
            /*Get the corresponding bit in the BMR */
            bmrMask_p = abInfo_p->m_bmr[bmrNum][numOf80BitChunk - 1].m_mask;

            /* if the device dump is enabled then read the register from device/c-model
                and print the data */
            if(isDevDump)
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_block_reg_read(dev_p, 0, blkNum, 
                    (NLMDEV_BLOCK_MASK_0_0_REG + bmrNum + (numOf80BitChunk -1)),
                    (void*)&readData, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_block_reg_read",reasonCode);
                    return errNum;
                }

                /* assign the read data */
                bmrMask_p = readData;
            }

            for(bitNum = 7; bitNum >= 0; --bitNum)
            {
                bmrAllow =  (nlm_u8) (!(bmrMask_p[byteNum] & (1u << bitNum)));

                if(combineBMR && !bmrAllow)
                    continue;
                
                dataBit = (nlm_u8)(data[byteNum] & (1u << bitNum));

                maskBit = (nlm_u8)(mask[byteNum] & (1u << bitNum));

                if( dataBit &&  !maskBit )
                {
                    NlmCmFile__fprintf(fp, "1");
                }
                else if( !dataBit && maskBit )
                {
                    NlmCmFile__fprintf(fp, "0");
                }
                else if (!dataBit && !maskBit)
                {
                    NlmCmFile__fprintf(fp, "X");
                }
                else
                {
                    if(!combineBMR)
                    {
                        NlmCmFile__fprintf(fp, "-");
                    }
                    else
                    {
                        /*
                        If we are in Sahasra mode and printing the non-SS blks
                        then if we get X=1, Y=1, then entire row is a miss. So we can
                        skip printing the row
                        */
                        NlmCmFile__fprintf(fp, "_");
                        /*skipRow = NlmTrue;
                         break;*/
                    }
                }
            } /*Finished processing all the bits in an entry */

            /*if(skipRow)
                break;*/

        } /*Finished processing all the bytes in an 80-b portion */

        /*if(skipRow)
            break;*/
        
    } /*Finished processing the current entry */
    return errNum;
}

static NlmBool
display_dba_is_entry_valid(
    NlmDev *dev_p,
    nlm_u16 blkNum,
    nlm_u16 rowNum, 
    nlm_u8 blkWidthIdx)
{
    NlmDevShadowAB *abInfo_p = NULL; 
    NlmBool isEntryDeleted = NlmFalse;
    nlm_u32 verifyIdx = blkWidthIdx;
    nlm_u16 i = 0;
    nlm_u32 maxBlkWidthIdx = NLMDEV_MAX_AB_WIDTH_IN_BITS / NLMDEV_AB_WIDTH_IN_BITS;
    NlmDevABEntry abEntry = {{0},};
    NlmDevABEntry *data_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;

    if(blkWidthIdx == maxBlkWidthIdx)
        verifyIdx = blkWidthIdx / 2;

    for(i = 0; i < verifyIdx; ++i)
    {
        /* if the device dump is enabled then read the register from device/c-model
            and print the data */
        if(isDevDump)
        {
            NlmCm__memset(&abEntry, 0, sizeof(NlmDevABEntry));

            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_dba_read(dev_p, 0, blkNum, (rowNum + i), &abEntry, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_dba_read",reasonCode);
                return errNum;
            }

            /* assign the read data */
            data_p = &abEntry;
        }
        else
        {
            abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];
            data_p   = &abInfo_p->m_abEntry[rowNum + i];
        }

        if(!data_p->m_vbit)
        {
            isEntryDeleted = NlmTrue;
            break;
        }
    }

    if(blkWidthIdx == maxBlkWidthIdx && isEntryDeleted)
    {
        isEntryDeleted = NlmFalse;
        for(; i < blkWidthIdx; ++i)
        {
            /* if the device dump is enabled then read the register from device/c-model
            and print the data */
            if(isDevDump)
            {
                NlmCm__memset(&abEntry, 0, sizeof(NlmDevABEntry));

                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_dba_read(dev_p, 0, blkNum, (rowNum + i), &abEntry, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_dba_read",reasonCode);
                    return errNum;
                }

                /* assign the read data */
                data_p = &abEntry;
            }
            else
            {
                abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];
                data_p   = &abInfo_p->m_abEntry[rowNum + i];
            }

            if(!data_p->m_vbit)
            {
                isEntryDeleted = NlmTrue;
                break;
            }
        }
    }
    return !isEntryDeleted;

}


static NlmErrNum_t 
sm__pvt__disp_dba_blocks(
    NlmCmFile *fp,
    NlmDev *dev_p,
    nlm_u16 blkNum
    )
{
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    nlm_u16 entryNum = 0, bmrNum = 0;
    nlm_u16 rowNum = 0;
    nlm_u8 blkWidthIdx = 0;
    nlm_u32 numOfEntries = 0;
    NlmDevBlockConfigReg readData = {0,};
    NlmDevBlockConfigReg *data_p = NULL;
    NlmDevShadowAB *abInfo_p = NULL;
    
    /* if the device dump is enabled then read the register from device/c-model
            and print the data */
    if(isDevDump)
    {
        /* Invoke the device manager API to read the specified LTR */
        if((errNum = kbp_dm_block_reg_read(dev_p, 0, 
            blkNum, NLMDEV_BLOCK_CONFIG_REG,(void*)&readData, &reasonCode)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_block_reg_read",reasonCode);
            return errNum;
        }

        /* assign the read data */
        data_p = &readData;
    }
    else
    {
        abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];
        data_p = &abInfo_p->m_blkConfig;
    }

    blkWidthIdx      = (nlm_u8)(1 << data_p->m_blockWidth);
    numOfEntries     = NLMDEV_AB_DEPTH/blkWidthIdx;
       
    if(data_p->m_blockEnable == 0)
    {
         NlmCmFile__fprintf(fp,"\n Blk Disabled \n");
         return errNum;
    }

    NlmCmFile__fprintf(fp, "BlkWidth: %d,  NumEntries: %d, ShftDir: %d, BaseAddr: %x, ShftCnt: %d \n\n", 
        blkWidthIdx * NLMDEV_AB_WIDTH_IN_BITS, numOfEntries, data_p->m_shiftDir,
        data_p->m_baseAddr, data_p->m_shiftCount);

    NlmCmFile__fprintf(fp, "\n\n");

    for(bmrNum = 0; bmrNum < NLMDEV_NUM_BMRS_PER_AB; bmrNum++)
    {
        NlmCmFile__fprintf(fp, "BMR %d : ", bmrNum);

        display_dba_bmr(fp, dev_p, blkNum, bmrNum, blkWidthIdx);
        
        NlmCmFile__fprintf(fp, "\n");
    }
    
    NlmCmFile__fprintf(fp, "\n\n");

    bmrNum = 0;
    for(entryNum = 0; entryNum < numOfEntries; entryNum++)
    {
        rowNum =  (entryNum * blkWidthIdx);  

        if(!display_dba_is_entry_valid(dev_p, blkNum, rowNum, blkWidthIdx))
            continue;
        
        display_dba_entry(fp, dev_p, 
                blkNum, blkWidthIdx, rowNum, bmrNum, 0);//NlmFalse);

        NlmCmFile__fprintf(fp, "\n");
        
    } /*Finished processing all the rows in a column */
    
    return errNum;
}

#if NLM_DEM_DC  /* not supported this release */

/* ==================== UDA MEMORY DUMP ===================== */
static NlmErrNum_t 
sm__pvt__disp_uda_blocks(
    NlmCmFile *fp, 
    NlmDev *dev_p,
    nlm_u16 blkNum
    )
{
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;

    nlm_u32 sbNr = 0, blkNrInSB = 0, rowNr = 0;
    nlm_u8 udaCfgBitNr = 0;
    nlm_u8 *data_p = NULL;
    nlm_u8 udaData[10] = {0,};
    NlmDevUDAConfigReg udaCfg    = {{0},};
    NlmDevUDAConfigReg *udaCfg_p = NULL;
    nlm_u32 udaAddr = 0;
    
    if(isDevDump == NlmFalse)
        return errNum;  /* currently UDA SM dump not supported */

    NlmCmFile__fprintf(fp, "\n\t UDA Dump:");
    NlmCmFile__fprintf(fp, "\n\t =========\n\n");

    /* this is the suepr block number */
    sbNr = blkNum;

    /* print the UDA data if the Super block is enabled */
    {
        NlmCmFile__fprintf(fp, "\n\t   UDA SB: %d  => ", sbNr);

        /* check the UDA block is enabled or not */ 
        udaCfgBitNr = (nlm_u8)(sbNr/4);

        udaCfg_p = NULL;

        if(isDevDump)
        {           
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_global_reg_read(dev_p, 0, NLMDEV_UDA_CONFIG_REG, 
               (void*)&udaCfg, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_global_read",reasonCode);
                return errNum;
            }
            udaCfg_p = &udaCfg;
        }
        else
        {
            udaCfg_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_global->m_devUDAConfig;
        }
        /*If the UDA super block is not enabled in the UDA Config Register, for the given
          block, then ship this block */
        if(udaCfg_p->m_uSBEnable[udaCfgBitNr] == NlmFalse)
        {
            NlmCmFile__fprintf(fp, "Disabled \n\n");
            return NLMERR_OK;
        }

        NlmCmFile__fprintf(fp, "Enabled \n\n");

        for ( rowNr = 0; rowNr < NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK; rowNr++ )
        {
            NlmCmFile__fprintf(fp, "\n\t        ");
            for ( blkNrInSB = 0; blkNrInSB < NLMDEV_NUM_SRAM_BLOCKS_IN_SB; blkNrInSB++ )
            {
                {
                    data_p = NULL;
                    if(isDevDump)
                    {   
                        udaAddr = ((sbNr * (NLMDEV_NUM_SRAM_BLOCKS_IN_SB * NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK)) + 
                            (rowNr * 8) + blkNrInSB);

                        /* Invoke the device manager API to read the specified LTR */
                        if((errNum = kbp_dm_uda_read(dev_p, 0, 
                            udaAddr, (void*)&udaData, 4, &reasonCode)) != NLMERR_OK)
                        {
                            NlmCm__printf("\n\n\t Err:%d, Dev Mgr Fn: kbp_dm_uda_read",reasonCode);
                            return errNum;
                        }
                        data_p = udaData; /* data returned is 10B, last 4 Bytes has the UDA data*/
                    }
                    else
                    {
                        ;//data_p = sram_p->m_sramSB[sbNr].m_sramBlk[blkNrInSB].m_entry[rowNr].m_data;
                        //tblMgr_p->m_udaSbMem_p[sbNr].m_udaBlk[blkNrInSB].m_entry[rowNr].m_data
                    }
                    NlmCmFile__fprintf(fp, " %.2x_%.2x_%.2x_%.2x ", data_p[6], data_p[7], data_p[8], data_p[9]);
                }
            }           
        }
        NlmCmFile__fprintf(fp, "\n");
    }
    return NLMERR_OK;
}
#endif

NlmErrNum_t 
kbp_dm_dump(
    NlmDev *dev_p,
    NlmDevLogType logType,
    NlmDevSbRange supBlkRange
    )
{
    nlm_u16 blkNum = 0;
    nlm_u16 sbNr = 0;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmCmFile * fp = NULL;
    nlm_u16 stDBA = 0, edDBA = 0;
    nlm_u16 stUDA = 0, edUDA = 0;

    if(!dev_p)
        return NLMERR_FAIL;
    
    /* validate the the DBA/UDA super block ranges */
    if( logType == NLMDEV_SM_DUMP_DBA || logType == NLMDEV_DEV_DUMP_DBA)
    {
        if(supBlkRange.m_stSBNr >= supBlkRange.m_endSBNr )
        {
            NlmCm__printf("\n\t Invalid Super Block values [lo < hi] \n\n");
            return NLMERR_FAIL;
        }

        if(supBlkRange.m_stSBNr >= NLMDEV_NUM_SUPER_BLOCKS || supBlkRange.m_endSBNr >= NLMDEV_NUM_SUPER_BLOCKS)
        {
            NlmCm__printf("\n\t Invalid Super Block value [range: 0-31] \n\n");
            return NLMERR_FAIL;
        }

        stDBA =  (supBlkRange.m_stSBNr * NLMDEV_NUM_AB_PER_SUPER_BLOCK);    
        edDBA = (((supBlkRange.m_endSBNr+1) * NLMDEV_NUM_AB_PER_SUPER_BLOCK) - 1);
    }

    if( logType == NLMDEV_SM_DUMP_UDA || logType == NLMDEV_DEV_DUMP_UDA )
    {
        if(supBlkRange.m_stSBNr >= supBlkRange.m_endSBNr )
        {
            NlmCm__printf("\n\t Invalid Super Block values [lo < hi] \n\n");
            return NLMERR_FAIL;
        }

        if(supBlkRange.m_stSBNr >= NLMDEV_NUM_SRAM_SUPER_BLOCKS || supBlkRange.m_endSBNr >= NLMDEV_NUM_SRAM_SUPER_BLOCKS)
        {
            NlmCm__printf("\n\t Invalid Super Block value [range: 0-63] \n\n");
            return NLMERR_FAIL;
        }       

        stDBA = supBlkRange.m_stSBNr;   
        edDBA = supBlkRange.m_endSBNr;
    }

    if( logType == NLMDEV_SM_DUMP_ALL || logType == NLMDEV_DEV_DUMP_ALL )
    {
        stDBA = 0;  
        edDBA = NLMDEV_NUM_ARRAY_BLOCKS - 1;

        stUDA = 0;  
        edUDA = NLMDEV_NUM_SRAM_SUPER_BLOCKS - 1;
    }

    isDevDump = NlmFalse; /* shadow dump */
    
    /* enable the device dump is flags are for the device dump */
    if( logType == NLMDEV_DEV_DUMP_REG || logType ==NLMDEV_DEV_DUMP_DBA ||
        logType == NLMDEV_DEV_DUMP_UDA || logType == NLMDEV_DEV_DUMP_ALL)
    {
        NlmCm__printf("\n\t ----- Dumping Device Data for device : %d ----- \n\n", dev_p->m_devId);
        isDevDump = NlmTrue; /* device/kbp dump */
#ifndef NLMPLATFORM_BCM

        fp = NlmCmFile__fopen(devDumpFName[dev_p->m_devId],"w");
        if(!fp)
            return NLMERR_FAIL;
#endif

        NlmCmFile__fprintf(fp, "\n\t @@@@------ Device Dump Start Here ------@@@@\n");

    }
    else
    {
        NlmCm__printf("\n\t ----- Dumping Shadow Memory for device : %d ----- \n\n", dev_p->m_devId);

#ifndef NLMPLATFORM_BCM

        fp = NlmCmFile__fopen(smDumpFName[dev_p->m_devId],"w");
        if(!fp)
            return NLMERR_FAIL;     
#endif      
        NlmCmFile__fprintf(fp, "\n\t @@@@------ Shadow Dump Start Here ------@@@@\n");

    }
    

    /* DBA blocks */
    if( logType == NLMDEV_SM_DUMP_ALL || logType == NLMDEV_SM_DUMP_DBA ||
        logType == NLMDEV_DEV_DUMP_ALL || logType == NLMDEV_DEV_DUMP_DBA )
    {
        for(blkNum = stDBA; blkNum <= edDBA; blkNum++)
        {
            NlmCmFile__fprintf(fp,"\n\n\n____________________________ DBA BLK %d _______________________\n\n\n", blkNum);

            if(( errNum = sm__pvt__disp_dba_blocks( fp, dev_p, blkNum)) != NLMERR_OK)
            {
#ifndef NLMPLATFORM_BCM
                NlmCmFile__fclose(fp);
#endif

                return errNum;
            }
        }
    }

    /* UDA blocks */
    if( logType == NLMDEV_SM_DUMP_ALL || logType == NLMDEV_SM_DUMP_UDA ||
        logType == NLMDEV_DEV_DUMP_ALL || logType == NLMDEV_DEV_DUMP_UDA )
    {
        for(sbNr = stUDA; sbNr <= edUDA; sbNr++)
        {
            NlmCmFile__fprintf(fp,"\n\n\n____________________________ UDA SB BLK %d _______________________\n\n\n", sbNr);

#if NLM_DM_DC  /* not supported this release */
            if(( errNum = sm__pvt__disp_uda_blocks( fp, dev_p, sbNr)) != NLMERR_OK)
            {
#ifndef NLMPLATFORM_BCM
                NlmCmFile__fclose(fp);
#endif

                return errNum;
            }
#endif
        }
    }

    /* Registers */
    if( logType == NLMDEV_SM_DUMP_ALL || logType == NLMDEV_SM_DUMP_REG ||
        logType == NLMDEV_DEV_DUMP_ALL || logType == NLMDEV_DEV_DUMP_REG )
    {   
        NlmCmFile__fprintf(fp,"\n\n\n ________________ LTR Registers _________________\n\n\n");
        if(( errNum = sm__pvt__disp_ltr_registers(fp, dev_p)) != NLMERR_OK)
        {
#ifndef NLMPLATFORM_BCM
            NlmCmFile__fclose(fp);
#endif

            return errNum;
        }

        NlmCmFile__fprintf(fp,"\n\n\n ________________ Global Registers _________________\n\n\n");
        if(( errNum = sm__pvt__disp_global_registers(fp, dev_p)) != NLMERR_OK)
        {
#ifndef NLMPLATFORM_BCM
            NlmCmFile__fclose(fp);
#endif

            return errNum;
        }
        
        NlmCmFile__fprintf(fp,"\n\n\n ________________ Range Control Registers _________________\n\n\n");
        if(( errNum = sm__pvt__disp_range_registers(fp, dev_p)) != NLMERR_OK)
        {
#ifndef NLMPLATFORM_BCM
            NlmCmFile__fclose(fp);
#endif

            return errNum;
        }
    }

    if(isDevDump == NlmTrue)
        NlmCmFile__fprintf(fp, "\n\t @@@@------ Device Dump Ends Here ------@@@@\n");
    else
        NlmCmFile__fprintf(fp, "\n\t @@@@------ Shadow Dump Ends Here ------@@@@\n");

#ifndef NLMPLATFORM_BCM
    NlmCmFile__fclose(fp);
#endif

    return errNum;

}








/* ==========================================================================
   Functions related to the comparing Shadow Memory with Device memory 
   ========================================================================== */

static NlmErrNum_t
scan_dba_entry(
    NlmDev *dev_p,
    nlm_u16 blkNum,
    nlm_u8 blkWidthIdx,
    nlm_u16 rowNum
    )
{
    nlm_u16 numOf80BitChunk = 0;
    nlm_u32 byteNum = 0;
    NlmDevABEntry devABEntry = {{0},};
    NlmDevABEntry  *smABEntry_p = NULL;
    NlmDevShadowAB *abInfo_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;

    /* get the shadow pointer for the specified block */
    abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];

    for(numOf80BitChunk = blkWidthIdx; numOf80BitChunk > 0; numOf80BitChunk--)
    {   
        smABEntry_p = &(abInfo_p->m_abEntry[rowNum + numOf80BitChunk - 1]);
        
        /* read the entry from the device */
        NlmCm__memset(&devABEntry, 0, sizeof(NlmDevABEntry));

        /* Invoke the device manager API to read the specified LTR */
        if((errNum = kbp_dm_dba_read(dev_p, 0, blkNum, (rowNum + numOf80BitChunk - 1), &devABEntry, &reasonCode)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_dba_read",errNum, reasonCode);
            return errNum;
        }

        /* if both valid bits are zero skip the remaining as it is invalid entry and always miss */
        if(devABEntry.m_vbit == NLMDEV_MISS && smABEntry_p->m_vbit == NLMDEV_MISS)
        {
            break;
        }

        /* if both are equal (valid) then compare the data and mask */
        if(devABEntry.m_vbit == smABEntry_p->m_vbit)
        {
            for(byteNum = 0; byteNum < NLMDEV_AB_WIDTH_IN_BYTES; ++byteNum)
            {

                if( (devABEntry.m_data[byteNum] != smABEntry_p->m_data[byteNum]) && 
                    (devABEntry.m_mask[byteNum] != smABEntry_p->m_mask[byteNum]) )
                {
                    NlmCm__printf("\n\n\t data/mask match in DBA block :%d, row: %d \n\n", 
                        blkNum,(rowNum + numOf80BitChunk - 1));
                    return NLMERR_FAIL;
                }
            }  /*Finished processing the current entry */
        }
        else
        {
            NlmCm__printf("\n\n\t miss match in valid bits [SM: %d, DEV: %d] \n\n",
                smABEntry_p->m_vbit, devABEntry.m_vbit);
            return NLMERR_FAIL;
        }
        
    } /* 80b chunks */
        
    return errNum;
}

static NlmErrNum_t
scan_dba_bmr(
    NlmDev *dev_p,
    nlm_u16 blkNum,
    nlm_u16 bmrNum,
    nlm_u8 maxNumChunks
    )
{
    nlm_u32 i =0;
    nlm_u8 *mask = NULL;
    nlm_u32 numOf80BitChunk = 0;
    NlmDevShadowAB *abInfo_p  = NULL;
    NlmDevBlockMaskReg readData = {{0},};
    NlmDevBlockMaskReg *data_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;

    if(maxNumChunks > NLMDEV_NUM_80BIT_SEGMENTS_PER_BMR)
        maxNumChunks = NLMDEV_NUM_80BIT_SEGMENTS_PER_BMR;
        
    for(numOf80BitChunk = maxNumChunks; numOf80BitChunk > 0; numOf80BitChunk--)
    {
        /* if the device dump is enabled then read the register from device/c-model
          and print the data */
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_block_reg_read(dev_p, 0, blkNum, 
                (NLMDEV_BLOCK_MASK_0_0_REG + bmrNum + (numOf80BitChunk -1)),
                (void*)&readData, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_block_reg_read",errNum, reasonCode);
                return errNum;
            }           
        }
        
        /* read from the shadow memory */
        {
            abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];
            data_p = &abInfo_p->m_bmr[bmrNum ] [numOf80BitChunk -1];
        }

        mask = data_p->m_mask ;

        for(i = 0; i < NLMDEV_AB_WIDTH_IN_BYTES; ++i)
        {
            if(data_p->m_mask[i] != readData.m_mask[i])
            {
                NlmCm__printf("\n\n\t miss match in BMR %d data \n\n", (bmrNum + (numOf80BitChunk -1)) );
                return NLMERR_FAIL;
            }
        }       
    }
    return errNum;
}

static NlmErrNum_t 
sm__pvt__scan_dba_blocks(
    NlmDev *dev_p,
    nlm_u16 blkNum
    )
{
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    nlm_u16 entryNum = 0, bmrNum = 0;
    nlm_u16 rowNum = 0;
    nlm_u8 blkWidthIdx = 0;
    nlm_u32 numOfEntries = 0;
    NlmDevBlockConfigReg readData = {0,};
    NlmDevBlockConfigReg *data_p = NULL;
    NlmDevShadowAB *abInfo_p = NULL;
    
    /* if the device dump is enabled then read the register from device/c-model
       and print the data */
    {
        /* Invoke the device manager API to read the specified LTR */
        if((errNum = kbp_dm_block_reg_read(dev_p, 0, 
            blkNum, NLMDEV_BLOCK_CONFIG_REG,(void*)&readData, &reasonCode)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_block_reg_read",errNum, reasonCode);
            return errNum;
        }
    }

    /* get the shadow memory data */
    {
        abInfo_p = &((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];
        data_p = &abInfo_p->m_blkConfig;
    }

    blkWidthIdx      = (nlm_u8)(1 << data_p->m_blockWidth);
    numOfEntries     = NLMDEV_AB_DEPTH/blkWidthIdx;
     
    NlmCm__printf("\n   - Block %3d is ", blkNum);
    if(data_p->m_blockEnable == NLMDEV_DISABLE && readData.m_blockEnable == NLMDEV_DISABLE)
    {
         NlmCm__printf(" Disabled ");
         return NLMERR_OK;
    }
    NlmCm__printf(" Enabled ");

    NlmCm__printf("\n\t - Block Configuration register :");
    if( (data_p->m_blockEnable != readData.m_blockEnable) ||
        (data_p->m_baseAddr != readData.m_baseAddr) ||
        (data_p->m_blockWidth != readData.m_blockWidth) ||
        (data_p->m_shiftCount != readData.m_shiftCount) ||
        (data_p->m_shiftDir != readData.m_shiftDir) )
    {
        NlmCm__printf(" Fail ");
        NlmCm__printf("\n\n\t miss match in BCR register for block %d \n\n", blkNum);
        return NLMERR_FAIL;
    }
    NlmCm__printf(" Done ");

    NlmCm__printf("\n\t - Block Mask register :");
    for(bmrNum = 0; bmrNum < NLMDEV_NUM_BMRS_PER_AB; bmrNum++)
    {
        NlmCm__printf("\n\t     - Scaning BMR %d - ", bmrNum);
        
        if((errNum = scan_dba_bmr(dev_p, blkNum, bmrNum, blkWidthIdx)) != NLMERR_OK)
        {
            NlmCm__printf(" Fail ");
            NlmCm__printf("\n   - Block Configuration register : Fail ");
            NlmCm__printf("\n\n\t miss match in BMR register for block %d \n\n", blkNum);
            return NLMERR_FAIL;
        }
        NlmCm__printf(" Done ");
    }
    NlmCm__printf("\n\t - Block Mask register : Done ");
    
    bmrNum = 0;
    NlmCm__printf("\n\t - Scanning %4d entries of the %d block :", numOfEntries, blkNum);
    for(entryNum = 0; entryNum < numOfEntries; entryNum++)
    {
        rowNum =  (entryNum * blkWidthIdx);  
        
        if( (errNum = scan_dba_entry(dev_p, blkNum, blkWidthIdx, rowNum)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t miss match in DBA entry for block %d \n\n", blkNum);
            return NLMERR_FAIL;
        }

        if( entryNum && ((entryNum % 512) == 0))
            NlmCm__printf("\n\t\t - scanned %4d DBA entries ...", entryNum);        
    } /*Finished processing all the rows in a column */

    NlmCm__printf("\n\t\t - scanned %4d DBA entries ...", entryNum);
    NlmCm__printf("\n\t - Scanneding %d entries of the %d block : done", numOfEntries, blkNum);
    
    return errNum;
}


static NlmErrNum_t 
sm__pvt__scan_range_registers(
    NlmDev *dev_p
    )
{
    nlm_u32 regNr = 0, byte = 0;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    
    NlmDevShadowDevice* shadowDevice_p = (NlmDevShadowDevice*)dev_p->m_shadowDevice_p;

    NlmCm__printf("\n    - Scanning Range Control Registers \n\n");

    for(regNr = 0; regNr < NLMDEV_NUM_RANGE_REG; ++regNr)
    {
        NlmDevRangeReg readReg = {{0},};
        NlmDevRangeReg *rangeReg_p = &(shadowDevice_p->m_rangeReg[regNr]);
        
        NlmCm__printf("\n\t  - Range Register Addr 0x%5x : ", NLM_REG_RANGE_A_BOUNDS(0) + regNr);

        /* if the device dump is enabled then read the register from device/c-model
               and print the data */        
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_range_reg_read(dev_p, 0, 0/* RJ to check SMT num*/, 
                (NLM_REG_RANGE_A_BOUNDS(0) + regNr), &readReg, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf(" Fail ");
                NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_range_reg_read",errNum, reasonCode);
                return errNum;
            }
        }

        for(byte = 0; byte < NLMDEV_REG_LEN_IN_BYTES; byte++)
        {
            if(rangeReg_p->m_data[byte] != readReg.m_data[byte])
            {
                NlmCm__printf(" Fail ");
                NlmCm__printf("\n\n\t Mismatch in the range register addr: %5x data \n",
                    (NLM_REG_RANGE_A_BOUNDS(0) + regNr));
                return NLMERR_FAIL;
            }
        }   
        NlmCm__printf(" Done ");
    }

    NlmCm__printf("\n    - Scanning Range Control Registers : Done \n\n");
    return errNum;
}

static NlmErrNum_t 
sm__pvt__scan_global_registers(
    NlmDev *dev_p
    )
{
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    nlm_u32 idx = 0;
    
    NlmDevShadowDevice* shadowDevice_p = (NlmDevShadowDevice*)dev_p->m_shadowDevice_p;

    NlmCm__printf("\n    - Scanning Global Registers \n");

    /*Print the device config Register */
    {
        NlmDevConfigReg* reg_p = NULL;
        NlmDevConfigReg readReg = {0,};

        /* have the SM pointer by default*/
        reg_p = &(shadowDevice_p->m_global->m_devConfig);
        
        NlmCm__printf("\n\t - Device Config Register Data : ");

        /* if the device dump is enabled then read the register from device/c-model
            and print the data */       
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_global_reg_read(dev_p, 0, NLMDEV_DEVICE_CONFIG_REG, 
               (void*)&readReg, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf(" Fail ");
                NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_global_reg_read",errNum, reasonCode);
                return errNum;
            }
        }

        if( (reg_p->m_port0Enable != readReg.m_port0Enable) ||
            (reg_p->m_port1Enable != readReg.m_port1Enable) ||
            (reg_p->m_dualPortMode != readReg.m_dualPortMode) ||
            (reg_p->m_dualBankMode != readReg.m_dualBankMode) ||
            (reg_p->m_rangeEnable != readReg.m_rangeEnable) ||
            (reg_p->m_lastDevice != readReg.m_lastDevice) ||
            (reg_p->m_firstDevice != readReg.m_firstDevice) ||
            (reg_p->m_dbParityErrEntryInvalidate != readReg.m_dbParityErrEntryInvalidate) ||
            (reg_p->m_softErrorScanEnable != readReg.m_softErrorScanEnable) ||
            (reg_p->m_port1CtxIDShift != readReg.m_port1CtxIDShift) ||
            (reg_p->m_ACtoBankMapping != readReg.m_ACtoBankMapping) ||
            (reg_p->m_CBConfig != readReg.m_CBConfig) 
            )
        {
            NlmCm__printf(" Fail ");
            NlmCm__printf("\n\t Mismatch in the Device Configuration register (DCR) data \n");
            return NLMERR_FAIL;
        }
        NlmCm__printf(" Done ");
    }
    
    /*Print the UDA config Register */
    {
        NlmDevUDAConfigReg* reg_p = NULL;
        NlmDevUDAConfigReg readReg = {{0},};

        NlmCm__printf("\n\t - Device UDA Config Register Data : ");
        /* have the SM pointer by default*/
        reg_p = &(shadowDevice_p->m_global->m_devUDAConfig);
        
        /* if the device dump is enabled then read the register from device/c-model
            and print the data */       
        {
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = kbp_dm_global_reg_read(dev_p, 0, NLMDEV_UDA_CONFIG_REG, 
               (void*)&readReg, &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf(" Fail ");
                NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_global_reg_read",errNum, reasonCode);
                return errNum;
            }
        }
                
        for(idx = 0; idx < (NLMDEV_NUM_SRAM_SUPER_BLOCKS/4); idx++)
        {
            if(reg_p->m_uSBEnable[idx] != readReg.m_uSBEnable[idx])
            {
                NlmCm__printf(" Fail ");
                NlmCm__printf("\n\t Mismatch in the UDA configuration register data \n");
                return NLMERR_FAIL;
            }
        }
        NlmCm__printf(" Done ");
    }

    NlmCm__printf("\n    - Scanning Global Registers : Done \n");

    return errNum;
}


static NlmErrNum_t 
sm__pvt__scan_ltr_registers(
    NlmDev *dev_p
    )
{
    nlm_u8 ltrNum = 0;
    nlm_u32 regNr = 0, keyNr = 0;
    nlm_u32 numBlkSelect     = NLMDEV_NUM_ARRAY_BLOCKS/64;
    nlm_u32 numPrlSrchRegs   = NLMDEV_NUM_ARRAY_BLOCKS/32;
    NlmReasonCode reasonCode = NLMRSC_REASON_OK;
    NlmErrNum_t errNum = 0;
    NlmBool isSMT = NlmFalse;

    NlmDevShadowDevice* shadowDevice_p = (NlmDevShadowDevice*)dev_p->m_shadowDevice_p;

    NlmCm__printf("\n    - Scanning LTR Registers \n\n");

    for(ltrNum = 0; ltrNum < NLMDEV_MAX_NUM_LTRS; ++ltrNum)
    {
        /* incase of the SMT mode the ranges used by the LTR's
           LTRs :  0-63    A/B/C/D
           LTRs : 64-127   E/F/G/H
        */
        if(dev_p->m_devMgr_p->m_smtMode == NLMDEV_DUAL_SMT_MODE && ltrNum > 63)
            isSMT = NlmTrue;

        NlmCm__printf("\n\n\t - LTR Num = %d \n", ltrNum);
        
        for(regNr = 0; regNr < numBlkSelect; ++regNr)
        {
            NlmDevBlkSelectReg* reg_p = NULL;
            NlmDevBlkSelectReg readReg = {{0},};
            nlm_u32 blkNum = 0;

            NlmCm__printf("\n\t     - Block Select Register %d : ", regNr);
            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_blockSelect[regNr]);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_BLOCK_SELECT_0_LTR + regNr), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                    return errNum;
                }
            }

            for(blkNum = 0; blkNum < 64; ++blkNum)
            {
                if(reg_p->m_blkEnable[blkNum] != readReg.m_blkEnable[blkNum])
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\t Mismatch in LTR %d block select register %d \n", ltrNum, regNr);
                    return NLMERR_FAIL;
                }
            }
            NlmCm__printf(" Done ");
        }
        
        /*Print the Super Blk - Key mapping */
        {
            NlmDevSuperBlkKeyMapReg* reg_p = NULL;
            NlmDevSuperBlkKeyMapReg readReg = {{0},};
            nlm_u32 sbNr = 0;

            NlmCm__printf("\n\t     - Super Block to Key Mapping Register : ");
            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_superBlkKeyMap);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_SUPER_BLK_KEY_MAP_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                    return errNum;
                }               
            }

            for(sbNr = 0; sbNr < NLMDEV_NUM_SUPER_BLOCKS; ++sbNr)
            {
                if(reg_p->m_keyNum[sbNr] != readReg.m_keyNum[sbNr])
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\t Mismatch in LTR %d SB2KeyMap register \n", ltrNum);
                    return NLMERR_FAIL;
                }       
            }
            NlmCm__printf(" Done ");
        }
                
        /*Print the parallel search configuration register */
        {       
            for(regNr = 0; regNr < numPrlSrchRegs; ++regNr)
            {
                NlmDevParallelSrchReg* reg_p = NULL;
                NlmDevParallelSrchReg readReg = {{0},};
                nlm_u32 blkNr = 0;

                NlmCm__printf("\n\t     - Prallel Search Register %d : ", regNr);

                /* have the SM pointer by default*/
                reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_parallelSrch[regNr]);
                
                /* if the device dump is enabled then read the register from device/c-model
                and print the data */
                {                   
                    /* Invoke the device manager API to read the specified LTR */
                    if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                        (NLMDEV_PARALLEL_SEARCH_0_LTR + regNr), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                        return errNum;
                    }
                }
                
                for(blkNr = 0; blkNr < 32; ++blkNr)
                {
                    if(reg_p->m_psNum[blkNr] != readReg.m_psNum[blkNr])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d parallel search register %d \n", ltrNum, regNr);
                        return NLMERR_FAIL;
                    }
                }
                NlmCm__printf(" Done ");
            }
        }

        /*Print the RangeInsert0 Register */
        {
            NlmDevRangeInsertion0Reg* reg_p = NULL;
            NlmDevRangeInsertion0Reg readReg = {0,};

            NlmCm__printf("\n\t     - Range Insert #0 Register : ");

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_rangeInsert0);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_RANGE_INSERTION_0_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                    return errNum;
                }
            }

            /* compare the range fields here */
            {
                nlm_u32 keyNum = 0;

                if( (reg_p->m_rangeAEncodingType != readReg.m_rangeAEncodingType) ||
                    (reg_p->m_rangeAEncodedBytes != readReg.m_rangeAEncodedBytes) ||
                    (reg_p->m_rangeBEncodingType != readReg.m_rangeBEncodingType) ||
                    (reg_p->m_rangeBEncodedBytes != readReg.m_rangeBEncodedBytes)
                    )
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\t Mismatch in LTR %d range insert 0 register \n", ltrNum);
                    return NLMERR_FAIL;
                }

                for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
                {
                    if(reg_p->m_rangeAInsertStartByte[keyNum] != readReg.m_rangeAInsertStartByte[keyNum])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d range insert 0 register \n", ltrNum);
                        return NLMERR_FAIL;
                    }

                    if(reg_p->m_rangeBInsertStartByte[keyNum] != readReg.m_rangeBInsertStartByte[keyNum])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d range insert 0 register \n", ltrNum);
                        return NLMERR_FAIL;
                    }
                }
            }
            NlmCm__printf(" Done ");
        }

        /*Print the RangeInsert1 Register */
        {
            NlmDevRangeInsertion1Reg* reg_p = NULL;
            NlmDevRangeInsertion1Reg readReg = {0,};

            NlmCm__printf("\n\t     - Range Insert #1 Register : ");
            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_rangeInsert1);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_RANGE_INSERTION_1_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                    return errNum;
                }
            }

            /* compare the range fields here */
            {
                nlm_u32 keyNum = 0;

                if( (reg_p->m_rangeCEncodingType != readReg.m_rangeCEncodingType) ||
                    (reg_p->m_rangeCEncodedBytes != readReg.m_rangeCEncodedBytes) ||
                    (reg_p->m_rangeDEncodingType != readReg.m_rangeDEncodingType) ||
                    (reg_p->m_rangeCEncodedBytes != readReg.m_rangeDEncodedBytes)
                    )
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\t Mismatch in LTR %d range insert 1 register \n", ltrNum);
                    return NLMERR_FAIL;
                }

                for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
                {
                    if(reg_p->m_rangeCInsertStartByte[keyNum] != readReg.m_rangeCInsertStartByte[keyNum])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d range insert 1 register \n", ltrNum);
                        return NLMERR_FAIL;
                    }

                    if(reg_p->m_rangeDInsertStartByte[keyNum] != readReg.m_rangeDInsertStartByte[keyNum])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d range insert 1 register \n", ltrNum);
                        return NLMERR_FAIL;
                    }
                }
                NlmCm__printf(" Done ");
            }
        }

        /*Print the KCR registers */
        for(keyNr = 0; keyNr < NLMDEV_NUM_KEYS ; ++keyNr)
        {           
            for(regNr = 0; regNr <  NLMDEV_NUM_OF_KCR_PER_KEY; ++regNr)
            {
                nlm_u32 index = (keyNr * NLMDEV_NUM_OF_KCR_PER_KEY) + regNr;

                NlmDevKeyConstructReg* reg_p = NULL;
                NlmDevKeyConstructReg readReg = {{0},};
                nlm_u32 segNr = 0;

                NlmCm__printf("\n\t     - Key Construction Register (KPU: %d, KCR: %d) : ", keyNr, regNr);

                /* have the SM pointer by default*/
                reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_keyConstruct[index]);
                
                /* if the device dump is enabled then read the register from device/c-model
                and print the data */
                {                   
                    /* Invoke the device manager API to read the specified LTR */
                    if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                        (NLMDEV_KEY_0_KCR_0_LTR + index), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                        return errNum;
                    }
                }

                for(segNr = 0; segNr < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
                {
                    nlm_u32 convertVal = 0;
                    if(readReg.m_isZeroFill[segNr] != reg_p->m_isZeroFill[segNr])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d Key Consturction register %d \n", ltrNum, keyNr);
                        return NLMERR_FAIL;
                    }

                    if(readReg.m_startByteLoc[segNr] != reg_p->m_startByteLoc[segNr])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d Key Consturction register %d \n", ltrNum, keyNr);
                        return NLMERR_FAIL;
                    }

                    /* Note: for device numBytes = 0 is 1 Byte.
                         to be consistant with SM we are printing 0 if 1 Byte */
                    if((readReg.m_numOfBytes[segNr] - 1) > 0)
                        convertVal = readReg.m_numOfBytes[segNr];
                    else
                        convertVal = (readReg.m_numOfBytes[segNr] - 1);
                    
                    if(convertVal != reg_p->m_numOfBytes[segNr])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d Key Consturction register %d \n", ltrNum, keyNr);
                        return NLMERR_FAIL;
                    }
                } /* for segNr */

                NlmCm__printf(" Done ");

            } /* for regNr */
        } /* for keyNr */

        /* Extended Capability Register 0*/
        {
            NlmDevExtCap0Reg* reg_p = NULL;
            NlmDevExtCap0Reg readReg = {{0},};
            nlm_u32 psNr = 0;

            NlmCm__printf("\n\t     - Extended Capability Register #0 : ");

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_extCap0);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_EXT_CAPABILITY_REG_0_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                    return errNum;
                }
            }
            /* compare the data */
            {
                if( (reg_p->m_rangeAExtractStartByte != readReg.m_rangeAExtractStartByte) ||
                    (reg_p->m_rangeBExtractStartByte != readReg.m_rangeBExtractStartByte) ||
                    (reg_p->m_rangeCExtractStartByte != readReg.m_rangeCExtractStartByte) ||
                    (reg_p->m_rangeDExtractStartByte != readReg.m_rangeDExtractStartByte) ||
                    (reg_p->m_numOfValidSrchRslts    != readReg.m_numOfValidSrchRslts)
                    )
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\t Mismatch in LTR %d Extend Capability 0 register \n", ltrNum);
                    return NLMERR_FAIL;
                }
                
                for(psNr = 0; psNr < NLMDEV_NUM_PARALLEL_SEARCHES; ++psNr)
                {
                    if(reg_p->m_bmrSelect[psNr] != readReg.m_bmrSelect[psNr])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d Extend Capability 0 register \n", ltrNum);
                        return NLMERR_FAIL;
                    }
                }
            }
            NlmCm__printf(" Done ");
        }

        /* Extended Capability Register 1*/
        {
            NlmDevGeneralReg* reg_p = NULL;
            NlmDevGeneralReg readReg = {{0},};
            nlm_u32 idx = 0;

            NlmCm__printf("\n\t     - Extended Capability Register #1 : ");

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_internalReg1);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            {
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_generic_reg_read(dev_p, 0,
                                        NLM_REDEF_REG_ADDR_LTR_EXTENDED1(ltrNum), 
                                        (void*)&readReg,
                                        &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_generic_reg_read",errNum, reasonCode);
                    return errNum;
                }
            }

            for(idx = 0; idx < 10; idx++)
            {
                if(reg_p->m_data[idx] != readReg.m_data[idx])
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\t Mismatch in LTR %d Extend Capability 1 register \n", ltrNum);
                    return NLMERR_FAIL;
                }
            }
            NlmCm__printf(" Done ");
        }

        /* OpCode Extension Register */
        {
            NlmDevOpCodeExtReg* reg_p = NULL;
            NlmDevOpCodeExtReg readReg = {{0},};
            nlm_u32 keyNum = 0;

            NlmCm__printf("\n\t     - Opcode Extenstion Register : ");

            /* have the SM pointer by default*/
            reg_p = &(shadowDevice_p->m_ltr[ltrNum].m_opCodeExt);
            
            /* if the device dump is enabled then read the register from device/c-model
               and print the data */
            {               
                /* Invoke the device manager API to read the specified LTR */
                if((errNum = kbp_dm_ltr_read(dev_p, 0, ltrNum, 
                    (NLMDEV_OPCODE_EXT_LTR), (void*)&readReg, &reasonCode)) != NLMERR_OK)
                {
                    NlmCm__printf(" Fail ");
                    NlmCm__printf("\n\n\t Err:%d (Reason: %d), Dev Mgr Fn: kbp_dm_ltr_read",errNum, reasonCode);
                    return errNum;
                }
            }

            /* compare data here */
            {
                for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
                {
                    if(reg_p->m_lclOpCode != readReg.m_lclOpCode)
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d Opcode Extenstion register \n", ltrNum);
                        return NLMERR_FAIL;
                    }

                    if(reg_p->m_ADLen[keyNum] != readReg.m_ADLen[keyNum])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d Opcode Extenstion register \n", ltrNum);
                        return NLMERR_FAIL;
                    }

                    if(reg_p->m_resultType[keyNum] != readReg.m_resultType[keyNum])
                    {
                        NlmCm__printf(" Fail ");
                        NlmCm__printf("\n\t Mismatch in LTR %d Opcode Extenstion register \n", ltrNum);
                        return NLMERR_FAIL;
                    }
                        
                }
            }
            NlmCm__printf(" Done ");
        }
    }

    NlmCm__printf("\n    - Scanning LTR Registers : Done \n\n");

    return errNum;
}



NlmErrNum_t 
kbp_dm_compare_dev_sm_memory(
    NlmDev *dev_p
    )
{
    nlm_u16 blkNum = 0;
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_u16 stDBA = 0, edDBA = 0;

    if(!dev_p)
        return NLMERR_FAIL;
    
    /* validate the the DBA/UDA super block ranges */
    stDBA = 0;  
    edDBA = NLMDEV_NUM_ARRAY_BLOCKS - 1;

    /* Registers */
    NlmCm__printf("\n --- Scaning registers (shadow v/s device) memory \n\n");
    {   
        NlmCm__printf("\n - Global Registers - \n");
        if(( errNum = sm__pvt__scan_global_registers(dev_p)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t  Scaning memroy for shadow v/s device failed \n\n");
            return NLMERR_FAIL;
        }
        
        NlmCm__printf("\n - LTR Registers - \n");
        if(( errNum = sm__pvt__scan_ltr_registers(dev_p)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t  Scaning memroy for shadow v/s device failed \n\n");
            return NLMERR_FAIL;
        }

        NlmCm__printf("\n - Range Registers - \n");     
        if(( errNum = sm__pvt__scan_range_registers(dev_p)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t  Scaning memroy for shadow v/s device failed \n\n");
            return NLMERR_FAIL;
        }
    }
    NlmCm__printf("\n --- Scaning registers (shadow v/s device) memory done \n\n");

    /* DBA blocks */
    NlmCm__printf("\n --- Scaning DBA blocks (shadow v/s device) memory \n\n");
    for(blkNum = stDBA; blkNum <= edDBA; blkNum++)
    {
        if(( errNum = sm__pvt__scan_dba_blocks(dev_p, blkNum)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\t  Scaning memroy for shadow v/s device failed \n\n");
            return NLMERR_FAIL;
        }
    }
    NlmCm__printf("\n --- Scaning DBA blocks (shadow v/s device) memory done \n\n");

    return errNum;

}

#ifndef NLMPLATFORM_BCM

NlmErrNum_t 
bcm_kbp_dm_dump(
    NlmDev *dev_p,
    NlmDevLogType logType,
    NlmDevSbRange supBlkRange
    )
{
    return kbp_dm_dump(dev_p, logType, supBlkRange);

}


NlmErrNum_t 
bcm_kbp_dm_compare_dev_sm_memory(
    NlmDev                  *dev_p
    )
{
    return kbp_dm_compare_dev_sm_memory(dev_p);
}



#endif




