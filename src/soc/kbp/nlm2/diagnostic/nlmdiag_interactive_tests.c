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
 
 #include "nlmdiag_refapp.h"
 #include "nlmdiag_registertest.h"
 #include "nlmdiag_interactive_tests.h"

/* NlmDiag_ExtractBlkSelLtrData extracts the various fields  of Blk Select Reg
from the 80b Reg Data read from the device */
static void NlmDiag_ExtractBlkSelLtrData(
	nlm_u8 *readData,
    NlmDevBlkSelectReg *blkSelectData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;

    /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
    hence we extract the Blk Select Reg Data for 32 Blks first and then for the remaining 32 Blks  */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS/4); abNum++, value >>= 1)
        blkSelectData_p->m_blkEnable[abNum] = value & 1;

    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 32);
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS/4); abNum < (NLMDEV_NUM_ARRAY_BLOCKS/2);
                                                                    abNum++, value >>= 1)
        blkSelectData_p->m_blkEnable[abNum] = value & 1;
}

/* NlmDiag_pvt_ExtractSBKeySelLtrData extracts the various fields  of Super Blk Key Select Reg
from the 80b Reg Data read from the device */
static void NlmDiag_ExtractSBKeySelLtrData(
	nlm_u8 *readData,
    NlmDevSuperBlkKeyMapReg *sbKeySelectData_p
    )
{
    nlm_32 sbNum;
    nlm_u32 value = 0;

    /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
    we extract the Key Select Reg Data for 16 super Blks first and then for the remaining 16 super Blks  */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    for(sbNum = 0; sbNum < (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum++, value >>= 2)
        sbKeySelectData_p->m_keyNum[sbNum] = value & 0x3;

    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 32);
    for(sbNum = (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum < NLMDEV_NUM_SUPER_BLOCKS;
                                                            sbNum++, value >>= 2)
        sbKeySelectData_p->m_keyNum[sbNum] = value & 0x3;
}

/* NlmDiag_pvt_ExtractParallelSrchLtrData extracts the various fields  of Parallel Srch Reg
from the 80b Reg Data read from the device */
static void NlmDiag_ExtractParallelSrchLtrData(
	nlm_u8 *readData,
    NlmDevParallelSrchReg *parallelSrchData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;

     /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
        we extract the Parallel Srch Reg Data for 16 array Blks first and
        then for the remaining 16 array Blks  */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS/8); abNum++, value >>= 2)
        parallelSrchData_p->m_psNum[abNum] = value & 0x3;

    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 32);
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS/8); abNum < (NLMDEV_NUM_ARRAY_BLOCKS/4);
                                                            abNum++, value >>= 2)
    parallelSrchData_p->m_psNum[abNum] = value & 0x3;
}

/* NlmDiag_pvt_ExtractKeyContructLtrData extracts the various fields  of Key Construct Reg
from the 80b Reg Data read from the device */
static void NlmDiag_ExtractKeyContructLtrData(
	nlm_u8 *readData,
    NlmDevKeyConstructReg *keyContructData_p
    )
{
    nlm_32 segNum;

    for(segNum = 0; segNum < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; segNum++)
    {
        /* Start Byte value for various segments is at following Bits of Register
        Segment 0 - Bits[6:0] ; Segment 1 - Bits[17:11]
        Segment 2 - Bits[28:22] ; Segment 3 - Bits[39:33]
        Segment 4 - Bits[50:44]*/
        keyContructData_p->m_startByteLoc[segNum] = (nlm_u8)ReadBitsInArrray(readData,
                                                            NLMDEV_REG_LEN_IN_BYTES,
                                                            segNum * 11 + 6,
                                                            segNum * 11);

        /* Number of Bytes value for various segments is at following Bits of Register
        Segment 0 - Bits[10:7] ; Segment 1 - Bits[21:18]
        Segment 2 - Bits[32:29] ; Segment 3 - Bits[43:40]
        Segment 4 - Bits[54:51]*/
        keyContructData_p->m_numOfBytes[segNum] = (nlm_u8)(ReadBitsInArrray(readData,
                                                           NLMDEV_REG_LEN_IN_BYTES,
                                                           segNum * 11 + 10,
                                                           segNum * 11 + 7) + 1);
    }
}

/* NlmDiag_pvt_ExtractRangeInsertLtrData extracts the various fields  of Range Insert Reg
from the 80b Reg Data read from the device */
static void NlmDiag_ExtractRangeInsertLtrData(
	nlm_u8 *readData,
    void *outputData_p,
    nlm_u32 regType
    )
{
    nlm_32 keyNum;
    nlm_u32 value = 0;

    if(regType == NLMDEV_RANGE_INSERTION_0_LTR)
    {
        NlmDevRangeInsertion0Reg *rangeInsertData_p =  (NlmDevRangeInsertion0Reg*)outputData_p;

        /* Location of various Field in Reg are as follows
            Range B Encoding Type: Bits[63:62]
            Range A Encoding Type: Bits[61:60]
            Range B Encoding Type: Bits[59:58]
            Range A Encoding Bytes: Bits[57:56] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 56);
        rangeInsertData_p->m_rangeBEncodingType = (value >> 6) & 0x3;
        rangeInsertData_p->m_rangeAEncodingType = (value >> 4) & 0x3;
        rangeInsertData_p->m_rangeBEncodedBytes = (value >> 2) & 0x3;
        rangeInsertData_p->m_rangeAEncodedBytes = value & 0x3;

        /* Location of RangeB Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 55, 28);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeBInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);

       /* Location of RangeA Insert Field for various keys in Reg are as follows
           Key 0 : Bits[7:0]
           Key 1 : Bits[13:8]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 27, 0);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeAInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);
    }
    else
    {
         NlmDevRangeInsertion1Reg *rangeInsertData_p =  (NlmDevRangeInsertion1Reg*)outputData_p;

        /* Location of various Field in Reg are as follows
            Range D Encoding Type: Bits[63:62]
            Range C Encoding Type: Bits[61:60]
            Range D Encoding Type: Bits[59:58]
            Range C Encoding Bytes: Bits[57:56] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 56);
        rangeInsertData_p->m_rangeDEncodingType = (value >> 6) & 0x3;
        rangeInsertData_p->m_rangeCEncodingType = (value >> 4) & 0x3;
        rangeInsertData_p->m_rangeDEncodedBytes = (value >> 2) & 0x3;
        rangeInsertData_p->m_rangeCEncodedBytes = value & 0x3;

        /* Location of RangeD Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 55, 28);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeDInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);

       /* Location of RangeC Insert Field for various keys in Reg are as follows
           Key 0 : Bits[7:0]
           Key 1 : Bits[13:8]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 27, 0);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeCInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);
    }
}

/* NlmDiag_pvt_ExtractMiscLtrData extracts the various fields  of Miscelleneous Reg
from the 80b Reg Data read from the device */
static void NlmDiag_ExtractMiscLtrData(
	nlm_u8 *readData,
    NlmDevMiscelleneousReg *miscData_p
    )
{
    nlm_32 psNum;
    nlm_u32 value = 0;

    /* Location of BMR Select Field for various parallel searches in Reg are as follows
           PS 0 : Bits[2:0]
           PS 1 : Bits[6:4]
           PS 2 : Bits[10:8]
           PS 3 : Bits[14:12] */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 14, 0);
    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
        miscData_p->m_bmrSelect[psNum] = (nlm_u8)((value >> (psNum * 4)) & 0x7);

    /* Location of Range Extract Start Bytes Field for various range types in Reg are as follows
           Range A  : Bits[22:16]
           Range B  : Bits[30:24]
           Range C  : Bits[38:32]
           Range D  : Bits[46:40] */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 46, 16);
    miscData_p->m_rangeDExtractStartByte = (nlm_u8)((value >> 24) & 0x7F);
    miscData_p->m_rangeCExtractStartByte = (nlm_u8)((value >> 16) & 0x7F);
    miscData_p->m_rangeBExtractStartByte = (nlm_u8)((value >> 8) & 0x7F);
    miscData_p->m_rangeAExtractStartByte = (nlm_u8)(value & 0x7F);

    /* Number of valid search results occupy Bits [57:56] of the Reg  */
    miscData_p->m_numOfValidSrchRslts = (nlm_u8)(ReadBitsInArrray(readData,
                                            NLMDEV_REG_LEN_IN_BYTES, 57, 56));

	/* Search type of each parallel search, Bits[55:48] (PHMD)
	  * PS#0 : Bits[49:48]
	  * PS#1 : Bits[51:50]
	  * PS#2 : Bits[53:52]
	  * PS#2 : Bits[55:54]
	  */
	value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES,
								NLMDIAG_MISCREG_SEARCH_END_BIT,
								NLMDIAG_MISCREG_SEARCH_START_BIT);

	for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
	{
		/* Each PS requires 2 bits and hence value 2. */
		miscData_p->m_searchType[psNum] = ( (value >> (psNum * 2)) & 0x3 );
	}

}

static void NlmDiag_ExtractSSLtrData(
	nlm_u8 *readData,
    NlmDevSSReg *SSData_p)
{
    nlm_u8 idx;
    nlm_u32 value = 0;

    /* Bits[16:9] represent the SS result map in the register. */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES,
    							NLMDIAG_SSREG_SSMAP_END_BIT,
    							NLMDIAG_SSREG_SSMAP_START_BIT);

    for(idx = 0; idx < NLMDEV_SS_RMP_AB; idx++)
        SSData_p->m_ss_result_map[idx] = (nlm_u8)((value >> idx) & 0x1);

}


/* NlmDiag_pvt_ConstructBlkSelLtrData contructs the 80b Reg Data
 based on various values of the various fields of Blk Select Register provided by application
 */
static NlmErrNum_t NlmDiag_ConstructBlkSelLtrData(
	nlm_u8 *o_data,
    NlmDevBlkSelectReg *blkSelectData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;

   /* A Blk Select Reg contains Blk enables for (NLMDEV_NUM_ARRAY_BLOCKS/2) number of array Blks
    i.e. Blk Select 0 Reg -- Contains Blk Enable for AB Num 0 - 63
    while Blk Select 1 Reg -- Contains Blk Enable for AB Num 64 - 127;
    Each block uses 1 bit as Enable Bit in the Register */

    /*Since "WriteBitsInArray" can write maximum of 32b in to an array
    we update the Blk Select Reg Data for 32 Blks first and then for the remaining 32 Blks  */
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS/4); abNum++, bitSelector++)
    {
        if(blkSelectData_p->m_blkEnable[abNum] != (Nlm11kDevDisableEnable)NLMDEV_DISABLE)
            value |= (1 << bitSelector);
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector =0;
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS/4); abNum < (NLMDEV_NUM_ARRAY_BLOCKS/2);
                                                                    abNum++, bitSelector++)
    {
        if(blkSelectData_p->m_blkEnable[abNum] != (Nlm11kDevDisableEnable)NLMDEV_DISABLE)
            value |= (1 << bitSelector);
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}

/* NlmDiag_pvt_ConstructSBKeySelLtrData contructs the 80b Reg Data based on various values
of the various fields of Super Blk Key Select Reg provided by application
 */
static NlmErrNum_t NlmDiag_ConstructSBKeySelLtrData(
	nlm_u8 *o_data,
    NlmDevSuperBlkKeyMapReg *sbKeySelectData_p
    )
{
    nlm_32 sbNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;
     /* A Super Blk Key Select Reg contains Key maps for the NLMDEV_NUM_SUPER_BLOCKS super Blocks;
    Since Key Num can be any value from 0 -3 Each super block uses 2 bits in the Register*/

    /*Since "WriteBitsInArray" can write maximum of 32b in to an array
    we update the Key Select Reg Data for 16 super Blks first and then for the remaining 16 super Blks  */
    for(sbNum = 0; sbNum < (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum++, bitSelector += 2)
    {
        if(sbKeySelectData_p->m_keyNum[sbNum] !=  (Nlm11kDevKeyNum)NLMDEV_KEY_0)
        {
            /* Key Value should be 0 - 3 */
            if(sbKeySelectData_p->m_keyNum[sbNum] != (Nlm11kDevKeyNum)NLMDEV_KEY_1
                && sbKeySelectData_p->m_keyNum[sbNum] != (Nlm11kDevKeyNum)NLMDEV_KEY_2
                && sbKeySelectData_p->m_keyNum[sbNum] != (Nlm11kDevKeyNum)NLMDEV_KEY_3)
                return NLMERR_FAIL;

            value |= (sbKeySelectData_p->m_keyNum[sbNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector = 0;
    for(sbNum = (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum < NLMDEV_NUM_SUPER_BLOCKS;
                                                            sbNum++, bitSelector += 2)
    {
        if(sbKeySelectData_p->m_keyNum[sbNum] !=  (Nlm11kDevKeyNum)NLMDEV_KEY_0)
        {
            /* Key Value should be 0 - 3 */
            if(sbKeySelectData_p->m_keyNum[sbNum] != (Nlm11kDevKeyNum)NLMDEV_KEY_1
                && sbKeySelectData_p->m_keyNum[sbNum] != (Nlm11kDevKeyNum)NLMDEV_KEY_2
                && sbKeySelectData_p->m_keyNum[sbNum] != (Nlm11kDevKeyNum)NLMDEV_KEY_3)
                return NLMERR_FAIL;

            value |= (sbKeySelectData_p->m_keyNum[sbNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}

/* NlmDiag_pvt_ConstructParallelSrchLtrData contructs the 80b Reg Data based on various values
    of the various fields of Parallel Srch Reg provided by application */
static NlmErrNum_t NlmDiag_ConstructParallelSrchLtrData(
	nlm_u8 *o_data,
    NlmDevParallelSrchReg *parallelSrchData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;
     /* A Parallel Srch Reg contains Result Port Maps for the (NLMDEV_NUM_ARRAY_BLOCKS/4) array Blocks;
                 i.e. Parallel Srch 0 Reg -- Contains Blk Enable for AB Num 0 - 31
                      Parallel Srch 1 Reg -- Contains Blk Enable for AB Num 32 - 63
                      Parallel Srch 2 Reg -- Contains Blk Enable for AB Num 64 - 95
                      Parallel Srch 3 Reg -- Contains Blk Enable for AB Num 96 - 127
    Since Result Port Num can be any value from 0 - 3 Each array block uses 2 bits in the Register*/

     /*Since "WriteBitsInArray" can write maximum of 32b in to an array
        we update the Parallel Srch Reg Data for 16 array Blks first and
        then for the remaining 16 array Blks  */
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS/8); abNum++, bitSelector += 2)
    {
        if(parallelSrchData_p->m_psNum[abNum] !=
                            (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_0)
        {
            /* Rslt Port Num Value should be 0 - 3 */
            if(parallelSrchData_p->m_psNum[abNum] !=
                                         (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_1
                && parallelSrchData_p->m_psNum[abNum] !=
                                          (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_2
                && parallelSrchData_p->m_psNum[abNum] !=
                                          (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_3)
                return NLMERR_FAIL;

            value |= (parallelSrchData_p->m_psNum[abNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector =0;
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS/8);
            abNum < (NLMDEV_NUM_ARRAY_BLOCKS/4);
                    abNum++, bitSelector += 2)
    {
        if(parallelSrchData_p->m_psNum[abNum] !=  (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_0)
        {
            /* Rslt Port Num Value should be 0 - 3 */
            if(parallelSrchData_p->m_psNum[abNum] !=
                                           (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_1
                && parallelSrchData_p->m_psNum[abNum] !=
                                           (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_2
                && parallelSrchData_p->m_psNum[abNum] !=
                                           (Nlm11kDevParallelSrchNum)NLMDEV_PARALLEL_SEARCH_3)
                return NLMERR_FAIL;

            value |= (parallelSrchData_p->m_psNum[abNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}

/* NlmDiag_pvt_ConstructKeyContructLtrData contructs the 80b Reg Data based on various values
    of the various fields of Key Contruct Reg provided by application */
static NlmErrNum_t NlmDiag_ConstructKeyContructLtrData(
	nlm_u8 *o_data,
    NlmDevKeyConstructReg *keyContructData_p
    )
{
    nlm_32 segNum;

    /* A Key Construction Reg contains Key Construction details of the Keys;
    There are two KCR for each key with each KCR containing details of 5 segments
    of key contruction. Each Segment requires 7 Bits for Start Byte and 5 Bits for
    Number of Bytes; Valid values of start byte is 0 - NLMDEV_MAX_KEY_LEN_IN_BYTES
    and for Number of Bytes is 1 - 16; If Number of Bytes of any segment is specified
    to be zero then it indicates that next segments needs be ignored*/

    for(segNum = 0; segNum < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; segNum++)
    {
        /* If Number of Bytes = 0, ignore the remaining segments of Register */
        if(keyContructData_p->m_numOfBytes[segNum] == 0)
            return NLMERR_OK;

        /* Check the correctness of Start Byte Location */
        if(keyContructData_p->m_startByteLoc[segNum] >= NLMDEV_MAX_KEY_LEN_IN_BYTES)
            return NLMERR_FAIL;

        /* Check the correctness of Number of Bytes */
        if(keyContructData_p->m_numOfBytes[segNum] > 16)
            return NLMERR_FAIL;

        /* Start Byte value for various segments is at following Bits of Register
        Segment 0 - Bits[6:0] ; Segment 1 - Bits[17:11]
        Segment 2 - Bits[28:22] ; Segment 3 - Bits[39:33]
        Segment 4 - Bits[50:44]*/
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, segNum * 11 + 6,
            segNum * 11, keyContructData_p->m_startByteLoc[segNum]);

        /* Number of Bytes value for various segments is at following Bits of Register
        Segment 0 - Bits[10:7] ; Segment 1 - Bits[21:18]
        Segment 2 - Bits[32:29] ; Segment 3 - Bits[43:40]
        Segment 4 - Bits[54:51]*/
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, segNum * 11 + 10,
            segNum * 11 + 7, keyContructData_p->m_numOfBytes[segNum]- 1);
    }
    return NLMERR_OK;
}


/* NlmDiag_pvt_ConstructRangeInsertLtrData contructs the 80b Reg Data based on various values
    of the various fields of Range Insert Reg provided by application */
static NlmErrNum_t NlmDiag_ConstructRangeInsertLtrData(
	nlm_u8 *o_data,
    void *inputData_p,
    nlm_u32 regType
    )
{
    nlm_32 keyNum;
    nlm_u32 value = 0;

    /* A Range Insertion0 Reg contains details for Range A and RangeB about the type of encoding
    used , number of bytes of Range Encoding to be inserted in the keys and location where Range
    Encodings needs to be inserted in each of the keys. Range Insertion1 Reg contains similar
    details for Range C and Range D*/
    if(regType == NLMDEV_RANGE_INSERTION_0_LTR)
    {
        NlmDevRangeInsertion0Reg *rangeInsertData_p =  (NlmDevRangeInsertion0Reg*)inputData_p;

        /* Valid values for Type of Range Encoding is 0 - 2*/
        if(rangeInsertData_p->m_rangeAEncodingType !=
                                 (Nlm11kDevRangeEncodingType)NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeAEncodingType !=
                                 (Nlm11kDevRangeEncodingType)NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeAEncodingType !=
                                 (Nlm11kDevRangeEncodingType)NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeBEncodingType !=
                                 (Nlm11kDevRangeEncodingType)NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeBEncodingType !=
                                 (Nlm11kDevRangeEncodingType)NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeBEncodingType !=
                                 (Nlm11kDevRangeEncodingType)NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        /* Valid values for Number of Bytes of Range Encoding is 0 - 3*/
        if(rangeInsertData_p->m_rangeAEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeBEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes !=
                                 (Nlm11kDevRangeEncodedValueBytes)NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        /* Location of various Field in Reg are as follows
            Range B Encoding Type: Bits[63:62]
            Range A Encoding Type: Bits[61:60]
            Range B Encoding Type: Bits[59:58]
            Range A Encoding Bytes: Bits[57:56] */
        value = (rangeInsertData_p->m_rangeBEncodingType << 6)
            | (rangeInsertData_p->m_rangeAEncodingType << 4)
            | (rangeInsertData_p->m_rangeBEncodedBytes << 2)
            | rangeInsertData_p->m_rangeAEncodedBytes;
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 56, value);

         /* Location of RangeB Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeBInsertStartByte[keyNum] >= NLMDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeBInsertStartByte[keyNum] != NLMDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeBInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 55, 28, value);

        /* Location of RangeA Insert Field for various keys in Reg are as follows
           Key 0 : Bits[7:0]
           Key 1 : Bits[13:8]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
             if(rangeInsertData_p->m_rangeAInsertStartByte[keyNum] >= NLMDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeAInsertStartByte[keyNum] != NLMDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeAInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 27, 0, value);
    }
    else
    {
        NlmDevRangeInsertion1Reg *rangeInsertData_p =  (NlmDevRangeInsertion1Reg*)inputData_p;

         /* Valid values for Type of Range Encoding is 0 - 2*/
        if(rangeInsertData_p->m_rangeCEncodingType !=
                               (Nlm11kDevRangeEncodingType)NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeCEncodingType !=
                               (Nlm11kDevRangeEncodingType)NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeCEncodingType !=
                               (Nlm11kDevRangeEncodingType)NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeDEncodingType !=
                               (Nlm11kDevRangeEncodingType)NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeDEncodingType !=
                               (Nlm11kDevRangeEncodingType)NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeDEncodingType !=
                              (Nlm11kDevRangeEncodingType)NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

         /* Valid values for Number of Bytes of Range Encoding is 0 - 3*/
        if(rangeInsertData_p->m_rangeCEncodedBytes !=
                           (Nlm11kDevRangeEncodedValueBytes)NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes !=
                           (Nlm11kDevRangeEncodedValueBytes)NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes !=
                           (Nlm11kDevRangeEncodedValueBytes)NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes !=
                           (Nlm11kDevRangeEncodedValueBytes)NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeDEncodedBytes !=
                            (Nlm11kDevRangeEncodedValueBytes)NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes !=
                            (Nlm11kDevRangeEncodedValueBytes)NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes !=
                            (Nlm11kDevRangeEncodedValueBytes)NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes !=
                            (Nlm11kDevRangeEncodedValueBytes)NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        /* Location of various Field in Reg are as follows
            Range D Encoding Type: Bits[63:62]
            Range C Encoding Type: Bits[61:60]
            Range D Encoding Type: Bits[59:58]
            Range C Encoding Bytes: Bits[57:56] */
        value = (rangeInsertData_p->m_rangeDEncodingType << 6)
            | (rangeInsertData_p->m_rangeCEncodingType << 4)
            | (rangeInsertData_p->m_rangeDEncodedBytes << 2)
            | rangeInsertData_p->m_rangeCEncodedBytes;

        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 56, value);

         /* Location of RangeD Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeDInsertStartByte[keyNum] >= NLMDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeDInsertStartByte[keyNum] != NLMDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeDInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 55, 28, value);

        /* Location of RangeC Insert Field for various keys in Reg are as follows
           Key 0 : Bits[7:0]
           Key 1 : Bits[13:8]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeCInsertStartByte[keyNum] >= NLMDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeCInsertStartByte[keyNum] != NLMDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;
            value |= (rangeInsertData_p->m_rangeCInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 27, 0, value);
    }
    return NLMERR_OK;
}


/* NlmDiag_pvt_ConstructMiscLtrData contructs the 80b Reg Data based on various values
    of the various fields of Miscelleneous Reg provided by application */
static NlmErrNum_t NlmDiag_ConstructMiscLtrData(
	nlm_u8 *o_data,
    NlmDevMiscelleneousReg *miscData_p
    )
{
    nlm_32 psNum;
    nlm_u32 value = 0;

    /* A Miscelleneous Reg contains such as which BMR should be used for each of the
    parallel searches and location of various Range Fields to be extracted
    from the Compare Key(Master Key) */
    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        /* Valid values for BMR Select is 0 - 4 and NLMDEV_NO_MASK_BMR_NUM */
        if(miscData_p->m_bmrSelect[psNum] >= NLMDEV_NUM_OF_BMRS_PER_BLK
            && miscData_p->m_bmrSelect[psNum] != NLMDEV_NO_MASK_BMR_NUM)
            return NLMERR_FAIL;

        /* Location of BMR Select Field for various parallel searches in Reg are as follows
           PS 0 : Bits[2:0]
           PS 1 : Bits[6:4]
           PS 2 : Bits[10:8]
           PS 3 : Bits[14:12] */
        value |= (miscData_p->m_bmrSelect[psNum] << (psNum * 4));
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 14, 0, value);

    value = 0;
    /* Valid values for Range Extract Start Bytes is 0 - 78 */
    if(miscData_p->m_rangeAExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || miscData_p->m_rangeBExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || miscData_p->m_rangeCExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || miscData_p->m_rangeDExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1))
        return NLMERR_FAIL;

     /* Location of Range Extract Start Bytes Field for various range types in Reg are as follows
           Range A  : Bits[22:16]
           Range B  : Bits[30:24]
           Range C  : Bits[38:32]
           Range D  : Bits[46:40] */
    value = ((miscData_p->m_rangeDExtractStartByte << 24)
            | (miscData_p->m_rangeCExtractStartByte << 16)
            | (miscData_p->m_rangeBExtractStartByte << 8)
            | miscData_p->m_rangeAExtractStartByte);

    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 46, 16, value);

    /* Valid values for Range Extract Start Bytes is 0 - 3 */
    if(miscData_p->m_numOfValidSrchRslts > 3)
         return NLMERR_FAIL;
    /* Number of valid search results occupy Bits [57:56] of the Reg  */
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 57, 56,
    					miscData_p->m_numOfValidSrchRslts);

	/* Search type of each parallel search, Bits[55:48] (PHMD)
	  * PS#0 : Bits[49:48]
	  * PS#1 : Bits[51:50]
	  * PS#2 : Bits[53:52]
	  * PS#2 : Bits[55:54]
	  */
	for(value = 0, psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
	{
		if(miscData_p->m_searchType[psNum] != (Nlm11kDevSearchType)NLMDEV_STANDARD &&
			miscData_p->m_searchType[psNum] != (Nlm11kDevSearchType)NLMDEV_SAHASRA)
			return NLMERR_FAIL;

		/* Each PS requires 2 bits and hence value 2. */
		value |= ( miscData_p->m_searchType[psNum] << (psNum * 2) );
	}
	WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES,
						NLMDIAG_MISCREG_SEARCH_END_BIT,
						NLMDIAG_MISCREG_SEARCH_START_BIT, value);

    return NLMERR_OK;
}

static NlmErrNum_t NlmDiag_ConstructSSLtrData(
	nlm_u8 *o_data,
    NlmDevSSReg *NetLData_p)
{
    nlm_u8 idx;
    nlm_u32 value = 0;

    for(idx = 0; idx < NLMDEV_SS_RMP_AB; idx++)
    {
        if( (NetLData_p->m_ss_result_map[idx]  != (Nlm11kDevSSRmap)NLMDEV_MAP_PE0) &&
        	 (NetLData_p->m_ss_result_map[idx] != (Nlm11kDevSSRmap)NLMDEV_MAP_PE1) )
            return NLMERR_FAIL;

        /* Bits[16:9] represent the SS result map in the register. */
        value |= (NetLData_p->m_ss_result_map[idx] << idx);
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES,
    					NLMDIAG_SSREG_SSMAP_END_BIT,
    					NLMDIAG_SSREG_SSMAP_START_BIT, value);

    return NLMERR_OK;
}

/*Function to extract register fields of LTR from the given 80b data*/
nlm_u32
NlmDiag_ExtractLTRFields (
	nlm_u8* readData,
	nlm_u8 regType, 
	void *o_data,
	NlmReasonCode* o_reason
	)
{
	switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
             NlmDiag_ExtractBlkSelLtrData(readData, o_data);
             break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
             NlmDiag_ExtractSBKeySelLtrData(readData, o_data);
             break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
             NlmDiag_ExtractParallelSrchLtrData(readData, o_data);
             break;

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
             NlmDiag_ExtractKeyContructLtrData(readData, o_data);
             break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
        case NLMDEV_RANGE_INSERTION_1_LTR:
             NlmDiag_ExtractRangeInsertLtrData(readData, o_data, regType);
             break;

        case NLMDEV_MISCELLENEOUS_LTR:
             NlmDiag_ExtractMiscLtrData(readData, o_data);
             break;

		case NLMDEV_SS_LTR:
             NlmDiag_ExtractSSLtrData(readData, o_data);
             break;

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }
	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;

	return NLMDIAG_ERROR_NOERROR;
}

/*Function to Construct register fields of LTR from the given 80b data*/
nlm_u32 
NlmDiag_ConstructLTRFields (
	void * data,
	nlm_u8 regType,
	nlm_u8 *o_data,
	NlmReasonCode* o_reason
	)
{
	NlmErrNum_t errNum = NLMERR_OK;
	  switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
             errNum = NlmDiag_ConstructBlkSelLtrData(
                                    o_data,
                                    (void*)data);
             break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
             errNum = NlmDiag_ConstructSBKeySelLtrData(
                                    o_data,
                                    (void*)data);
             break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
             errNum = NlmDiag_ConstructParallelSrchLtrData(
                                    o_data,
                                    (void*)data);
             break;

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
             errNum = NlmDiag_ConstructKeyContructLtrData(
                                    o_data,
                                    (void*)data);
             break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
        case NLMDEV_RANGE_INSERTION_1_LTR:
             errNum = NlmDiag_ConstructRangeInsertLtrData(
                                    o_data,
                                    (void*)data, regType);
             break;
        case NLMDEV_MISCELLENEOUS_LTR:
             errNum = NlmDiag_ConstructMiscLtrData(
                                    o_data,
                                    (void*)data);
             break;

		case NLMDEV_SS_LTR:
			 errNum = NlmDiag_ConstructSSLtrData(
                                    o_data,
                                    (void*)data);
             break;

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }
    NlmCm__printf("\n\n\terrNum = %d", (int)errNum);
	return NLMERR_OK;
}

/*Function to Read the Global register*/
nlm_u32 
NlmDiag_ReadFromGlobalReg(
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_GlobalRegWrRdInfo globalRegRdInfo;    
    
	/* Read Only Regs */
    NlmDevIdReg devIdRegReadInfo;
    NlmDevErrStatusReg devErrStatRegReadInfo;
    NlmDevAdvancedSoftErrReg devAdvSoftErrRegReadInfo;
    NlmDevResultReg devResultReg0ReadInfo;
    NlmDevResultReg devResultReg1ReadInfo;

    /* Write/Read regs */
    NlmDevConfigReg devConfigRegReadInfo;
    NlmDevErrStatusReg devErrStatMaskRegReadInfo; /* same structure used to mask the errors */
    NlmDevDbSoftErrFifoReg devDBSoftErrStatRegReadInfo;
    NlmDevScratchPadReg scratchPadRegReadInfo;

	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	nlm_u8		readData[NLMDEV_REG_LEN_IN_BYTES] = "";		/* 80 bit hex value */

	NlmReasonCode reasonCode;
    nlm_u32 errNum;
    nlm_u8 chnlNum = 0, devNum;
	nlm_u32 value = 0;
       
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

	moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;    
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

    devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	if(moduleInfo_p->m_dev_ppp[chnlNum][devNum]->m_shadowDevice_p == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	globalRegRdInfo.m_globalRegType = testInfo_p->m_userInput_p->m_registerType;
   
    /* Check for Global Register type and construct the pattern of data 
     *  according to specified register fields */
    switch(globalRegRdInfo.m_globalRegType)
    {
        case NLMDEV_DEVICE_ID_REG:  
            /* call the device manager API to read contents from the Device ID Reg */
			if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType, 
               (void*)&devIdRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
				NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
			}
			else
			{
				NlmCm__printf ("\nDevice Identification Register Content");
				value = (devIdRegReadInfo.m_databaseSize << 6) |
						(devIdRegReadInfo.m_majorDieRev << 3) |
						(devIdRegReadInfo.m_minorDieRev);
				
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 7, 0, value);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\n\tMinor Die Revision[0:2]... = %x", devIdRegReadInfo.m_minorDieRev);
					NlmCm__printf ("\n\tMajor Die Revision[5:3]. = %x", devIdRegReadInfo.m_majorDieRev);
					NlmCm__printf ("\n\tDatabase Size[7:6]...... = %x", devIdRegReadInfo.m_databaseSize);
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead DEVICE_ID_REG Completed");
				NlmCm__printf("\n\n\tRead DEVICE_ID_REG Completed");
			}
           break;

		case NLMDEV_DEVICE_CONFIG_REG: /* Device Config Reg */
            if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType,
                (void*)&devConfigRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
               NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\nDevice Configuration Register Details\n");
				value = (devConfigRegReadInfo.m_softErrorScanEnable << 6)
                    | (devConfigRegReadInfo.m_dbParityErrEntryInvalidate << 7)
                    | (devConfigRegReadInfo.m_dbSoftErrProtectMode << 26)
                    | (devConfigRegReadInfo.m_eccScanType << 27)
                    | (devConfigRegReadInfo.m_rangeEngineEnable << 28)
                    | (devConfigRegReadInfo.m_lowPowerModeEnable << 30);
                
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 30, 0, value);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead DEVICE_CONFIG_REG Completed");
				NlmCm__printf("\n\n\tRead DEVICE_CONFIG_REG Completed");

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\tSoft Error Scan Enable [6].................... = %x", devConfigRegReadInfo.m_softErrorScanEnable);
					NlmCm__printf ("\n\tDatabase Parity Error Entry Invalidate [7].. = %x", devConfigRegReadInfo.m_dbParityErrEntryInvalidate);
					NlmCm__printf ("\n\tDatabase Soft Error Protection Mode [26].... = %x", devConfigRegReadInfo.m_dbSoftErrProtectMode);
					NlmCm__printf ("\n\tECC Scan Type [27].......................... = %x", devConfigRegReadInfo.m_eccScanType);
					NlmCm__printf ("\n\tRange Matching Engine Enable [28]........... = %x", devConfigRegReadInfo.m_rangeEngineEnable);
					NlmCm__printf ("\n\tLow Power Mode Enable [30].................. = %x", devConfigRegReadInfo.m_lowPowerModeEnable);
				}

			}
            break;

        case NLMDEV_ERROR_STATUS_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType,
                (void*)&devErrStatRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
               NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\nError Status Register Content");
				value = ((devErrStatRegReadInfo.m_dbSoftError << 1)
					| (devErrStatRegReadInfo.m_dbSoftErrorFifoFull << 2)
					| (devErrStatRegReadInfo.m_parityScanFifoOverFlow << 5)
					| (devErrStatRegReadInfo.m_crc24Err << 16)
					| (devErrStatRegReadInfo.m_sopErr << 17)
					| (devErrStatRegReadInfo.m_eopErr << 18)
					| (devErrStatRegReadInfo.m_missingDataPktErr << 19)
					| (devErrStatRegReadInfo.m_burstMaxErr << 20)
					| (devErrStatRegReadInfo.m_rxNMACFifoParityErr << 21)
					| (devErrStatRegReadInfo.m_instnBurstErr << 22)
					| (devErrStatRegReadInfo.m_protocolErr << 23)
					| (devErrStatRegReadInfo.m_channelNumErr << 24)
					| (devErrStatRegReadInfo.m_burstControlWordErr << 25)
					| (devErrStatRegReadInfo.m_illegalInstnErr << 26)
					| (devErrStatRegReadInfo.m_devIdMismatchErr << 27)
					| (devErrStatRegReadInfo.m_ltrParityErr << 28)
					| (devErrStatRegReadInfo.m_ctxBufferParityErr << 29)
					| (devErrStatRegReadInfo.m_powerLimitingErr << 31));
	                
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

				value = 0;                 
				value =  (devErrStatRegReadInfo.m_alignmentErr << (48-32))
									| (devErrStatRegReadInfo.m_framingCtrlWordErr << (49-32))
									| (devErrStatRegReadInfo.m_rxPCSEFifoParityErr << (50-32));

				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 50, 32, value);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\n\tDatabase Soft Error [1].............. = %x", devErrStatRegReadInfo.m_dbSoftError);
					NlmCm__printf ("\n\tDatabase Soft Error FIFO Full [2].... = %x", devErrStatRegReadInfo.m_dbSoftErrorFifoFull);
					NlmCm__printf ("\n\tParity Scan FIFO Overflow [5]........ = %x", devErrStatRegReadInfo.m_parityScanFifoOverFlow);
					NlmCm__printf ("\n\tCRC24 Error [16]..................... = %x", devErrStatRegReadInfo.m_crc24Err);
					NlmCm__printf ("\n\tSOP Error [17]....................... = %x", devErrStatRegReadInfo.m_sopErr);
					NlmCm__printf ("\n\tEOP Error [18]....................... = %x", devErrStatRegReadInfo.m_eopErr);
					NlmCm__printf ("\n\tMissing Data Packet Error [19]....... = %x", devErrStatRegReadInfo.m_missingDataPktErr);
					NlmCm__printf ("\n\tBurst Max Error [20]................. = %x", devErrStatRegReadInfo.m_burstMaxErr);
					NlmCm__printf ("\n\tRxNMAC FIFO Parity Error [21]........ = %x", devErrStatRegReadInfo.m_rxNMACFifoParityErr);
					NlmCm__printf ("\n\tInstruction Burst Error [22]......... = %x", devErrStatRegReadInfo.m_instnBurstErr);
					NlmCm__printf ("\n\tProtocol Error [23].................. = %x", devErrStatRegReadInfo.m_protocolErr);
					NlmCm__printf ("\n\tChannel Number Error [24]............ = %x", devErrStatRegReadInfo.m_channelNumErr);
					NlmCm__printf ("\n\tBurst Control Word Error [25]........ = %x", devErrStatRegReadInfo.m_burstControlWordErr);
					NlmCm__printf ("\n\tIllegal Instruction Error [26]....... = %x", devErrStatRegReadInfo.m_illegalInstnErr);
					NlmCm__printf ("\n\tDeviceID Mismatch [27]............... = %x", devErrStatRegReadInfo.m_devIdMismatchErr);
					NlmCm__printf ("\n\tLTR Parity Error [28]................ = %x", devErrStatRegReadInfo.m_ltrParityErr);
					NlmCm__printf ("\n\tContext Buffer Parity Error [29]..... = %x", devErrStatRegReadInfo.m_ctxBufferParityErr);
					NlmCm__printf ("\n\tPower Limiting Error [31]............ = %x", devErrStatRegReadInfo.m_powerLimitingErr);
					NlmCm__printf ("\n\tAlignment Error [48]................. = %x", devErrStatRegReadInfo.m_alignmentErr);
					NlmCm__printf ("\n\tFraming Control Word Error [49]...... = %x", devErrStatRegReadInfo.m_framingCtrlWordErr);
					NlmCm__printf ("\n\tRxPCS EFIFO Parity Error [50]........ = %x", devErrStatRegReadInfo.m_rxPCSEFifoParityErr);
				}
			}
            break;

		case NLMDEV_ERROR_STATUS_MASK_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType,
                (void*)&devErrStatMaskRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
                NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
            }
			else
			{
				value = ((devErrStatMaskRegReadInfo.m_globalGIO_L1_Enable)
									| (devErrStatMaskRegReadInfo.m_dbSoftError << 1)
									| (devErrStatMaskRegReadInfo.m_dbSoftErrorFifoFull << 2)
									| (devErrStatMaskRegReadInfo.m_parityScanFifoOverFlow << 5)
									| (devErrStatMaskRegReadInfo.m_crc24Err << 16)
									| (devErrStatMaskRegReadInfo.m_sopErr << 17)
									| (devErrStatMaskRegReadInfo.m_eopErr << 18)
									| (devErrStatMaskRegReadInfo.m_missingDataPktErr << 19)
									| (devErrStatMaskRegReadInfo.m_burstMaxErr << 20)
									| (devErrStatMaskRegReadInfo.m_rxNMACFifoParityErr << 21)
									| (devErrStatMaskRegReadInfo.m_instnBurstErr << 22)
									| (devErrStatMaskRegReadInfo.m_protocolErr << 23)
									| (devErrStatMaskRegReadInfo.m_channelNumErr << 24)
									| (devErrStatMaskRegReadInfo.m_burstControlWordErr << 25)
									| (devErrStatMaskRegReadInfo.m_illegalInstnErr << 26)
									| (devErrStatMaskRegReadInfo.m_devIdMismatchErr << 27)
									| (devErrStatMaskRegReadInfo.m_ltrParityErr << 28)
									| (devErrStatMaskRegReadInfo.m_ctxBufferParityErr << 29)
									| (devErrStatMaskRegReadInfo.m_powerLimitingErr << 31));
	                
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

				value = 0;                 
				value =  (devErrStatMaskRegReadInfo.m_alignmentErr << (48-32))
									| (devErrStatMaskRegReadInfo.m_framingCtrlWordErr << (49-32))
									| (devErrStatMaskRegReadInfo.m_rxPCSEFifoParityErr << (50-32));

				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 50, 32, value);
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 79, 79, devErrStatMaskRegReadInfo.m_globalGIO_L0_Enable);

				NlmCm__printf ("\nError Status Mask Register Details");
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead ERROR_STATUS_MASK_REG Completed");
				NlmCm__printf("\n\n\tRead ERROR_STATUS_MASK_REG Completed");

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\n\tGIO_L[1] Enable [0]................ = %x", devErrStatMaskRegReadInfo.m_globalGIO_L1_Enable);
					NlmCm__printf ("\n\tDatabase Soft Error [1]............ = %x", devErrStatMaskRegReadInfo.m_dbSoftError);
					NlmCm__printf ("\n\tDatabase Soft Error FIFO Full [2].. = %x", devErrStatMaskRegReadInfo.m_dbSoftErrorFifoFull);
					NlmCm__printf ("\n\tParity Scan FIFO Overflow [5]...... = %x", devErrStatMaskRegReadInfo.m_parityScanFifoOverFlow);
					NlmCm__printf ("\n\tCRC24 Error [16]................... = %x", devErrStatMaskRegReadInfo.m_crc24Err);
					NlmCm__printf ("\n\tSOP Error [17]..................... = %x", devErrStatMaskRegReadInfo.m_sopErr);
					NlmCm__printf ("\n\tEOP Error [18]..................... = %x", devErrStatMaskRegReadInfo.m_eopErr);
					NlmCm__printf ("\n\tMissing Data Packet Error [19]..... = %x", devErrStatMaskRegReadInfo.m_missingDataPktErr);
					NlmCm__printf ("\n\tBurst Max Error [20]............... = %x", devErrStatMaskRegReadInfo.m_burstMaxErr);
					NlmCm__printf ("\n\tRxNMAC FIFO Parity Error [21]...... = %x", devErrStatMaskRegReadInfo.m_rxNMACFifoParityErr);
					NlmCm__printf ("\n\tInstruction Burst Error [22]....... = %x", devErrStatMaskRegReadInfo.m_instnBurstErr);
					NlmCm__printf ("\n\tProtocol Error [23]................ = %x", devErrStatMaskRegReadInfo.m_protocolErr);
					NlmCm__printf ("\n\tChannel Number Error [24].......... = %x", devErrStatMaskRegReadInfo.m_channelNumErr);
					NlmCm__printf ("\n\tBurst Control Word Error [25]...... = %x", devErrStatMaskRegReadInfo.m_burstControlWordErr);
					NlmCm__printf ("\n\tIllegal Instruction Error [26]..... = %x", devErrStatMaskRegReadInfo.m_illegalInstnErr);
					NlmCm__printf ("\n\tDeviceID Mismatch [27]............. = %x", devErrStatMaskRegReadInfo.m_devIdMismatchErr);
					NlmCm__printf ("\n\tLTR Parity Error [28].............. = %x", devErrStatMaskRegReadInfo.m_ltrParityErr);
					NlmCm__printf ("\n\tContext Buffer Parity Error [29]... = %x", devErrStatMaskRegReadInfo.m_ctxBufferParityErr);
					NlmCm__printf ("\n\tPower Limiting Error [31].......... = %x", devErrStatMaskRegReadInfo.m_powerLimitingErr);
					NlmCm__printf ("\n\tAlignment Error [48]............... = %x", devErrStatMaskRegReadInfo.m_alignmentErr);
					NlmCm__printf ("\n\tFraming Control Word Error [49].... = %x", devErrStatMaskRegReadInfo.m_framingCtrlWordErr);
					NlmCm__printf ("\n\tRxPCS EFIFO Parity Error [50]...... = %x", devErrStatMaskRegReadInfo.m_rxPCSEFifoParityErr);
					NlmCm__printf ("\n\tGIO_L[0] Enable [79]............... = %x", devErrStatMaskRegReadInfo.m_globalGIO_L0_Enable);
				}
			}
            break;

		case NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType,
                (void*)&devDBSoftErrStatRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
                NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\nDatabase Soft Error FIFO Register Content");
				value = (devDBSoftErrStatRegReadInfo.m_eraseFifo << 1) | devDBSoftErrStatRegReadInfo.m_eraseFifoEntry;
				WriteBitsInArray(readData, NLMDEV_REG_LEN_IN_BYTES, 41, 40, value);
				
				value = 0;
				value = ((devDBSoftErrStatRegReadInfo.m_errorAddrValid << 23)
					|(devDBSoftErrStatRegReadInfo.m_pErrorY << 22)
					|(devDBSoftErrStatRegReadInfo.m_pErrorX << 21)
					|(devDBSoftErrStatRegReadInfo.m_errorAddr));
				
				WriteBitsInArray(readData, NLMDEV_REG_LEN_IN_BYTES, 23, 0, value);

				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead DATABASE_SOFT_ERROR_FIFO_REG Completed");
				NlmCm__printf("\n\n\tRead DATABASE_SOFT_ERROR_FIFO_REG Completed");

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\n\tError Address [20:0]........ = %x", devDBSoftErrStatRegReadInfo.m_errorAddr);
					NlmCm__printf ("\n\tXPerr [21].................. = %x", devDBSoftErrStatRegReadInfo.m_pErrorX);
					NlmCm__printf ("\n\tYPerr [22].................. = %x", devDBSoftErrStatRegReadInfo.m_pErrorY);
					NlmCm__printf ("\n\tError Address Valid [23].... = %x", devDBSoftErrStatRegReadInfo.m_errorAddrValid);
					NlmCm__printf ("\n\tErase FIFO Entry [40]....... = %x", devDBSoftErrStatRegReadInfo.m_eraseFifoEntry);
					NlmCm__printf ("\n\tErase FIFO [41]............. = %x", devDBSoftErrStatRegReadInfo.m_eraseFifo);
				}
			}
            break;

        case NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType,
                (void*)&devAdvSoftErrRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
                NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\nAdvanced Features Soft Error Register Details");
				value = ((devAdvSoftErrRegReadInfo.m_sahasraParityErrAddr0 <<16)
							| (devAdvSoftErrRegReadInfo.m_cbParityErrAddr));
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 29, 0, value);

				value = 0;
				value = ((devAdvSoftErrRegReadInfo.m_ltrParityErrAddr <<16)
							| devAdvSoftErrRegReadInfo.m_sahasraParityErrAddr1);				
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 58, 32, value);

				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead ADVANCED_FEATURES_SOFT_ERROR_REG Completed");
				NlmCm__printf("\n\n\tRead ADVANCED_FEATURES_SOFT_ERROR_REG Completed");

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\n\tContext Buffer Parity Error Address [13:0].. = %x", devAdvSoftErrRegReadInfo.m_cbParityErrAddr);
					NlmCm__printf ("\n\tSahasra Parity Error Address [29:16]....... = %x", devAdvSoftErrRegReadInfo.m_sahasraParityErrAddr0);
					NlmCm__printf ("\n\tSahasra Parity Error Address [45:32]....... = %x", devAdvSoftErrRegReadInfo.m_sahasraParityErrAddr1);
					NlmCm__printf ("\n\tLTR Parity Error Address [58:48]........... = %x", devAdvSoftErrRegReadInfo.m_ltrParityErrAddr);
				}
			}
            break;

		case NLMDEV_SCRATCH_PAD0_REG:
			/* call the device manager API to read contents from the specified scratch pad */
           if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType, 
               (void*)&scratchPadRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
               NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
               NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
           }
		   else
		   {
			   NlmCm__printf ("\nScratch Pad Register 0 Content");
			   NlmCm__memcpy(readData,scratchPadRegReadInfo.m_data,NLMDEV_REG_LEN_IN_BYTES);
			   NlmCm__printf ("\n\t80 bit Data = ");
			   NlmDiag_PrintHexData(scratchPadRegReadInfo.m_data, NLMDEV_REG_LEN_IN_BYTES);

			   NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead SCRATCH_PAD0_REG Completed");
               NlmCm__printf("\n\n\tRead SCRATCH_PAD0_REG Completed");
		   }
           break;
		case NLMDEV_SCRATCH_PAD1_REG:                           
           /* call the device manager API to read contents from the specified scratch pad */
           if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType, 
               (void*)&scratchPadRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
               NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
               NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
           }
		   else
		   {
				NlmCm__printf ("Scratch Pad Register 1 Content\n");
			    NlmCm__memcpy(readData,scratchPadRegReadInfo.m_data,NLMDEV_REG_LEN_IN_BYTES);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(scratchPadRegReadInfo.m_data, NLMDEV_REG_LEN_IN_BYTES);

				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead SCRATCH_PAD1_REG Completed");
                NlmCm__printf("\n\n\tRead SCRATCH_PAD1_REG Completed");
		   }
           break;

        case NLMDEV_RESULT0_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType,
                (void*)&devResultReg0ReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
                NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
            }
			else
			{
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 20, 0, devResultReg0ReadInfo.m_hitAddress[0]);
				value = 0;
				value = devResultReg0ReadInfo.m_hitOrMiss[0] << (39-38);
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 39, 39, value);
				value = 0;
				value = devResultReg0ReadInfo.m_hitAddress[1] << (60-39);
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 60, 40, value);
				value = 0;
				value = devResultReg0ReadInfo.m_hitOrMiss[1] << (79-78);
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 79, 79, value);

				NlmCm__printf ("\nResult Register 0 Content");
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead RESULT0_REG Completed");
                NlmCm__printf("\n\n\tRead RESULT0_REG Completed");
			}
            break;

        case NLMDEV_RESULT1_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegRdInfo.m_globalRegType,
                (void*)&devResultReg1ReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
                NlmCm__printf("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterRead",reasonCode);
            }
			else
			{
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 20, 0, devResultReg1ReadInfo.m_hitAddress[0]);
				value = 0;
				value = devResultReg1ReadInfo.m_hitOrMiss[0] << (39-38);
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 39, 39, value);
				value = 0;
				value = devResultReg1ReadInfo.m_hitAddress[1] << (60-39);
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 60, 40, value);
				value = 0;
				value = devResultReg1ReadInfo.m_hitOrMiss[1] << (79-78);
				WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 79, 79, value);

				NlmCm__printf ("\nResult Register 1 Content");
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead RESULT1_REG Completed");
                NlmCm__printf("\n\n\tRead RESULT1_REG Completed");
			}
            break;
        /* ============================================== */

       case NLMDEV_GLOBALREG_END:
       default:
           break;
    }
	return NLMDIAG_ERROR_NOERROR;
}

/*Function to Write the Global register*/
nlm_u32 
NlmDiag_WriteToGlobalReg(
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_GlobalRegWrRdInfo globalRegWrInfo;    
   
	NlmDevConfigReg devConfigRegWrInfo;
    NlmDevErrStatusReg devErrStatMaskRegReadInfo; /* same structure used to mask the errors */
    NlmDevDbSoftErrFifoReg devDBSoftErrStatRegReadInfo;
    
	NlmReasonCode reasonCode;
    nlm_u32 errNum;
    nlm_u8 devNum;

    nlm_u8 chnlNum = 0;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	nlm_u32 value;
       
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;    
	
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	devNum = testInfo_p->m_userInput_p->m_devId;
	globalRegWrInfo.m_globalRegType = testInfo_p->m_userInput_p->m_registerType;

	if(moduleInfo_p->m_dev_ppp[chnlNum][devNum]->m_shadowDevice_p == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	if(globalRegWrInfo.m_globalRegType >= NLMDEV_GLOBALREG_END)         
        return NLMDIAG_ERROR_INVALID_INPUT;
   
    /* Check for Global Register type and construct the pattern of data 
    according to specified register fields */
    switch(globalRegWrInfo.m_globalRegType)
    {
        case NLMDEV_DEVICE_ID_REG:                
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Error: Device Id Register is Read only. ");
            NlmCm__printf("\n\t Error: Device Id Register is Read only. ");
			break;

        case NLMDEV_RESULT0_REG:
        case NLMDEV_RESULT1_REG:            
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                "\n\t Error: Writing to read only Global registers(Result0 and Result1Registers).");
            NlmCm__printf("\n\t Error: Writing to read only Global registers(Result0 and Result1Registers).");
			break;

        case NLMDEV_ERROR_STATUS_REG:
        case NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:            
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                "\n\t Error: Currently Not supported to write these registers(Error status/Adv soft_error_Registers).");
            NlmCm__printf ("\n\t Error: Currently Not supported to write these registers(Error status/Adv soft_error_Registers).");
			break;

        case NLMDEV_DEVICE_CONFIG_REG:    /* Device Config Reg */
			value = ReadBitsInArrray(testInfo_p->m_userInput_p->m_data, NLMDEV_REG_LEN_IN_BYTES, 30, 0);

            devConfigRegWrInfo.m_softErrorScanEnable = (value >> 6) & 1;
            devConfigRegWrInfo.m_dbParityErrEntryInvalidate = (value >> 7) & 1;
            devConfigRegWrInfo.m_dbSoftErrProtectMode = (value >> 26) & 1;
            devConfigRegWrInfo.m_eccScanType = (value >> 27) & 1;
            devConfigRegWrInfo.m_rangeEngineEnable = (value >> 28) & 1;
            devConfigRegWrInfo.m_lowPowerModeEnable = (value >> 30) & 1;
	
			/* Invoke Device Manager API which writes to specified Global Reg */
            if((errNum = NlmDevMgr__GlobalRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegWrInfo.m_globalRegType,
                (void*)(&devConfigRegWrInfo), &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                NlmCm__printf ("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
            }
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite DEVICE_CONFIG_REG Completed");
				NlmCm__printf("\n\n\tWrite DEVICE_CONFIG_REG Completed");
			}
            break;

        case NLMDEV_ERROR_STATUS_MASK_REG:
			value = ReadBitsInArrray(testInfo_p->m_userInput_p->m_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0);

			devErrStatMaskRegReadInfo.m_globalGIO_L1_Enable = (nlm_u8)(value & 1);
	        devErrStatMaskRegReadInfo.m_dbSoftError = (nlm_u8)((value >> 1) & 1);
			devErrStatMaskRegReadInfo.m_dbSoftErrorFifoFull = (nlm_u8)((value >> 2) & 1);
			devErrStatMaskRegReadInfo.m_parityScanFifoOverFlow =  (nlm_u8)((value >> 5) & 1);
			devErrStatMaskRegReadInfo.m_crc24Err =  (nlm_u8)((value >> 16) & 1);
			devErrStatMaskRegReadInfo.m_sopErr =  (nlm_u8)((value >> 17) & 1);
			devErrStatMaskRegReadInfo.m_eopErr =  (nlm_u8)((value >> 18) & 1);
			devErrStatMaskRegReadInfo.m_missingDataPktErr =  (nlm_u8)((value >> 19) & 1);
			devErrStatMaskRegReadInfo.m_burstMaxErr =  (nlm_u8)((value >> 20) & 1);
			devErrStatMaskRegReadInfo.m_rxNMACFifoParityErr =  (nlm_u8)((value >> 21) & 1);
			devErrStatMaskRegReadInfo.m_instnBurstErr =  (nlm_u8)((value >> 22) & 1);
			devErrStatMaskRegReadInfo.m_protocolErr =  (nlm_u8)((value >> 23) & 1);
			devErrStatMaskRegReadInfo.m_channelNumErr =  (nlm_u8)((value >> 24) & 1);
			devErrStatMaskRegReadInfo.m_burstControlWordErr =  (nlm_u8)((value >> 25) & 1);
			devErrStatMaskRegReadInfo.m_illegalInstnErr =  (nlm_u8)((value >> 26) & 1);
			devErrStatMaskRegReadInfo.m_devIdMismatchErr =  (nlm_u8)((value >> 27) & 1);
			devErrStatMaskRegReadInfo.m_ltrParityErr = (nlm_u8)((value >> 28) & 1);
			devErrStatMaskRegReadInfo.m_ctxBufferParityErr =  (nlm_u8)((value >> 29) & 1);
			devErrStatMaskRegReadInfo.m_powerLimitingErr =  (nlm_u8)((value >> 31) & 1);

			value = ReadBitsInArrray(testInfo_p->m_userInput_p->m_data, NLMDEV_REG_LEN_IN_BYTES, 55, 32);                
			devErrStatMaskRegReadInfo.m_alignmentErr =  (nlm_u8)((value >> (48-32)) & 1);
			devErrStatMaskRegReadInfo.m_framingCtrlWordErr =  (nlm_u8)((value >> (49-32)) & 1);
			devErrStatMaskRegReadInfo.m_rxPCSEFifoParityErr =  (nlm_u8)((value >> (50-32)) & 1);

			value = ReadBitsInArrray(testInfo_p->m_userInput_p->m_data, NLMDEV_REG_LEN_IN_BYTES, 79, 79);
			devErrStatMaskRegReadInfo.m_globalGIO_L0_Enable = (nlm_u8)(value & 1);
			        
           /* Invoke Device Manager API which writes to specified Global Reg */
           if((errNum = NlmDevMgr__GlobalRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegWrInfo.m_globalRegType,
               (void*)(&devErrStatMaskRegReadInfo), &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                NlmCm__printf ("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
           }
		   if(!errNum)
		   {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite ERROR_STATUS_MASK_REG Completed");
				NlmCm__printf("\n\n\tWrite ERROR_STATUS_MASK_REG Completed");
		   }
           break;

        case NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            
	        /* Erase FIFO and Erase FIFO Entry Field are Write Only Bits and hence are ignored */
            value = ReadBitsInArrray(testInfo_p->m_userInput_p->m_data , NLMDEV_REG_LEN_IN_BYTES, 41, 40);

	    devDBSoftErrStatRegReadInfo.m_eraseFifo = ((value >> 1) & 1);
	    devDBSoftErrStatRegReadInfo.m_eraseFifoEntry = (value & 1);

            value = ReadBitsInArrray(testInfo_p->m_userInput_p->m_data , NLMDEV_REG_LEN_IN_BYTES, 23, 0);
            devDBSoftErrStatRegReadInfo.m_errorAddrValid = (value >> 23) & 1;
            devDBSoftErrStatRegReadInfo.m_pErrorY = (value >> 22) & 1;
            devDBSoftErrStatRegReadInfo.m_pErrorX = (value >> 21) & 1;
            /* Bits[20:0] give error Address */
            devDBSoftErrStatRegReadInfo.m_errorAddr = value & 0x1FFFFF;

            /* Invoke Device Manager API which writes to specified Global Reg */
            if((errNum = NlmDevMgr__GlobalRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegWrInfo.m_globalRegType,
                (void*)(&devDBSoftErrStatRegReadInfo), &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                NlmCm__printf ("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
            }
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite DATABASE_SOFT_ERROR_FIFO_REG Completed");
				NlmCm__printf("\n\n\tWrite DATABASE_SOFT_ERROR_FIFO_REG Completed");
			}
            break;

        case NLMDEV_SCRATCH_PAD0_REG:
			 if((errNum = NlmDevMgr__GlobalRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegWrInfo.m_globalRegType,
                (void*)testInfo_p->m_userInput_p->m_data, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                NlmCm__printf ("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
            }
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite SCRATCH_PAD0_REG Completed");
				NlmCm__printf("\n\n\tWrite SCRATCH_PAD0_REG Completed");
			}
            break;  
        case NLMDEV_SCRATCH_PAD1_REG:                           
            /*Invoke Device manager API which writes to specified scratch pad register */
            if((errNum = NlmDevMgr__GlobalRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], globalRegWrInfo.m_globalRegType,
                (void*)testInfo_p->m_userInput_p->m_data, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                NlmCm__printf ("\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
            }
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite SCRATCH_PAD1_REG Completed");
				NlmCm__printf("\n\n\tWrite SCRATCH_PAD1_REG Completed");
			}
            break;  

        case NLMDEV_GLOBALREG_END:
        default:
            break;
    }
	return NLMDIAG_ERROR_NOERROR;    
}

/*Function to Read the Context Buffer Memory*/
nlm_u32 
NlmDiag_ReadFromCBMemory (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_CBWrRdInfo CBRdInfo;
	NlmDevCtxBufferReg CBRegReadInfo;
	NlmReasonCode reasonCode;
    nlm_u32 errNum; 
    nlm_u8 devNum = 0, chnlNum = 0;
	
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;  
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    

    /* Seed is provided as an parameter with the test case */
    CBRdInfo.m_seed_p = &testInfo_p->m_testParams[0]; 
	CBRdInfo.m_startAddress = (nlm_u16) testInfo_p->m_userInput_p->m_address;
	CBRdInfo.m_numOfEntries = 1;
	CBRdInfo.m_dataLen = 10;
	/* devNum = testInfo_p->m_userInput_p->m_devId; */ /* RJ Device id is not needed */

	if((errNum = NlmDevMgr__CBAsRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], CBRdInfo.m_startAddress, 
                                        &CBRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__CBAsRegisterRead",reasonCode);
        NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__CBAsRegisterRead",reasonCode);
    }
	else
	{
			NlmCm__printf ("\n\t80b Data = ");
			NlmDiag_PrintHexData(CBRegReadInfo.m_data, NLMDEV_REG_LEN_IN_BYTES);
			NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead CB memory location 0x%x Completed", (nlm_u16)testInfo_p->m_userInput_p->m_address);
			NlmCm__printf("\n\n\tRead CB memory location 0x%x Completed", (nlm_u16)testInfo_p->m_userInput_p->m_address);
	}
    return NLMDIAG_ERROR_NOERROR;  
}

/*Function to Write the Context Buffer Memory*/
nlm_u32 
NlmDiag_WriteToCBMemory (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDevCtxBufferInfo CBWriteInfo;
	NlmReasonCode reasonCode;
    nlm_u32 errNum; 
    nlm_u8 chnlNum = 0;

    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;  
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    /* Initialize the structure used to test the read and write to CB registers */
	CBWriteInfo.m_cbStartAddr = (nlm_u16) testInfo_p->m_userInput_p->m_address;
	CBWriteInfo.m_datalen = (nlm_u8)testInfo_p->m_userInput_p->m_compDataLen;
	NlmCm__memcpy(CBWriteInfo.m_data, testInfo_p->m_userInput_p->m_compareData, CBWriteInfo.m_datalen);
	 
    /* Invoke the Device Manager API which writes to specified CB entry */
    if((errNum = NlmDevMgr__CBWrite(moduleInfo_p->m_devMgr_pp[chnlNum], &CBWriteInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__CBWrite",reasonCode);
        NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__CBWrite",reasonCode);
    }
	if(!errNum)
	{
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite CB memory location 0x%x Completed", CBWriteInfo.m_cbStartAddr);
		NlmCm__printf("\n\n\tWrite CB memory location 0x%x Completed", CBWriteInfo.m_cbStartAddr);
	}
	return NLMDIAG_ERROR_NOERROR;  
}

/*Function to Read the LTR*/
nlm_u32 
NlmDiag_ReadFromLTR (
	NlmDiag_TestInfo *testInfo_p
	)
{
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_LTRRegWrRdInfo ltrRegRdInfo;
    
	NlmDevBlkSelectReg       ltrBlkSelRegInfo;
    NlmDevSuperBlkKeyMapReg  ltrBlkKeyMapRegInfo;
    NlmDevParallelSrchReg	 ltrPrlSrchRegInfo;
    NlmDevRangeInsertion0Reg ltrRangeInst0_RegInfo;
    NlmDevRangeInsertion1Reg ltrRangeInst1_RegInfo;
    NlmDevMiscelleneousReg   ltrMiscRegInfo;
    NlmDevKeyConstructReg    ltrKeyConstructRegInfo;
    NlmDevSSReg              ltrSSRegReadInfo;
    
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
    nlm_u32 loop1, loop2;
    nlm_u8 devNum, chnlNum = 0;
	nlm_u8		readData[NLMDEV_REG_LEN_IN_BYTES] = "";		/* 80 bit hex value */
            
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
                    || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
           
	devNum = testInfo_p->m_userInput_p->m_devId;

	if((moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) || (moduleInfo_p->m_dev_ppp[chnlNum][devNum]->m_shadowDevice_p == NULL))
       return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	ltrRegRdInfo.m_profileNum = testInfo_p->m_userInput_p->m_ltrNum;
	ltrRegRdInfo.m_ltrRegType = testInfo_p->m_userInput_p->m_registerType;

    
    if(ltrRegRdInfo.m_ltrRegType >= NLMDEV_LTR_REG_END
        || ltrRegRdInfo.m_profileNum >= NLMDEV_NUM_LTR_SET)
        return NLMDIAG_ERROR_INVALID_INPUT;

    /* Check for Ltr Register type and read the data 
      *  according to specified register fields */
    switch(ltrRegRdInfo.m_ltrRegType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:  /* Blk select 1 Reg is for future*/
            /* Invoke the device manager API to read the specified LTR */
                if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                    ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                    (void*)&ltrBlkSelRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
				{
					NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
					NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
				}
				else
				{
					NlmCm__printf ("\nBlock Selection LTR Register Content");
					NlmDiag_ConstructLTRFields ((void *)&ltrBlkSelRegInfo,ltrRegRdInfo.m_ltrRegType,
							   readData,&reasonCode);
					NlmCm__printf ("\n\t80 bit Data = ");
					NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

					if (testInfo_p->m_userInput_p->m_verbose)
					{
						for(loop1=0; loop1<NLMDEV_NUM_ARRAY_BLOCKS/4; loop1++)
						{
							for(loop2=0; loop2<2; loop2++)
							{
								NlmCm__printf("\n\tBlk Num: [%2d] Enbl Bit-> %d",
									loop1*2+loop2, ltrBlkSelRegInfo.m_blkEnable[loop1*2+loop2]);
							}
						}
					}
					NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
					NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				}
            break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                (void*)&ltrBlkKeyMapRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\n\tSuper Block Selection LTR Register Content");
				NlmDiag_ConstructLTRFields ((void *)&ltrBlkKeyMapRegInfo,ltrRegRdInfo.m_ltrRegType,
							   readData, &reasonCode);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					for(loop1 = 0;loop1 < NLMDEV_NUM_SUPER_BLOCKS/4; loop1++)
					{
						for(loop2 = 0; loop2 < 4; loop2++)
						{
							NlmCm__printf("\n\t SB Num: [%d] KeyNum --> %d ", loop1*4+loop2,
								ltrBlkKeyMapRegInfo.m_keyNum[loop1*4+loop2]);
						}
					}
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
			}
            break;
    
       case NLMDEV_PARALLEL_SEARCH_0_LTR:
       case NLMDEV_PARALLEL_SEARCH_1_LTR:
       case NLMDEV_PARALLEL_SEARCH_2_LTR:
       case NLMDEV_PARALLEL_SEARCH_3_LTR:
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                (void*)&ltrPrlSrchRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
               NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\n\tParallel Search LTR Register Content");
				NlmDiag_ConstructLTRFields ((void *)&ltrPrlSrchRegInfo,ltrRegRdInfo.m_ltrRegType,
							   readData, &reasonCode);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);
				if (testInfo_p->m_userInput_p->m_verbose)
				{
					for(loop1=0;loop1<NLMDEV_NUM_ARRAY_BLOCKS/4;loop1++)
					{
						NlmCm__printf("\n\t  Block Num: [%d] PS --> %d", loop1, ltrPrlSrchRegInfo.m_psNum[loop1]); 
					}
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
			}
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                (void*)&ltrRangeInst0_RegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\n\tRange insertion 0 LTR Register Content");
				NlmDiag_ConstructLTRFields ((void *)&ltrRangeInst0_RegInfo,ltrRegRdInfo.m_ltrRegType,
							   readData, &reasonCode);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);
				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\n\t Range Insert LTR 0: RangeA Encoding Type :%d   RangeB Encoding Type:%d ",
						ltrRangeInst0_RegInfo.m_rangeAEncodingType, ltrRangeInst0_RegInfo.m_rangeBEncodingType);
	                    
					NlmCm__printf ("\n\t Range Insert LTR 0: RangeA Encoding Byte :%d   RangeB Encoding Byte:%d ",
						ltrRangeInst0_RegInfo.m_rangeAEncodedBytes, ltrRangeInst0_RegInfo.m_rangeBEncodedBytes);
	                    
					NlmCm__printf("\n\t Range Insert LTR 0: RangeA Insert Start Byte :  RangeB Insert Start Byte:");
	                    
					for(loop1 = 0; loop1 < NLMDEV_NUM_KEYS; loop1++)
					{
						NlmCm__printf("\n\t %d :  %d",ltrRangeInst0_RegInfo.m_rangeAInsertStartByte[loop1],
							ltrRangeInst0_RegInfo.m_rangeBInsertStartByte[loop1]);                       
					}
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
			}
            break;

        case NLMDEV_RANGE_INSERTION_1_LTR:
           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                (void*)&ltrRangeInst1_RegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\n\tRange insertion 1 LTR Register Content");
				NlmDiag_ConstructLTRFields ((void *)&ltrRangeInst1_RegInfo,ltrRegRdInfo.m_ltrRegType,
							   readData, &reasonCode);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					NlmCm__printf ("\n\t Range Insert LTR 1: RangeC Encoding Type :%d   RangeD Encoding Type:%d ",
						ltrRangeInst1_RegInfo.m_rangeCEncodingType, ltrRangeInst1_RegInfo.m_rangeDEncodingType);
	                    
					NlmCm__printf ("\n\t Range Insert LTR 1: RangeC Encoding Byte :%d   RangeD Encoding Byte:%d ",
						ltrRangeInst1_RegInfo.m_rangeCEncodedBytes, ltrRangeInst1_RegInfo.m_rangeDEncodedBytes);
	                    
					NlmCm__printf("\n\t Range Insert LTR 0: RangeA Insert Start Byte :  RangeB Insert Start Byte:");
	                    
					for(loop1 = 0; loop1 < NLMDEV_NUM_KEYS; loop1++)
					{
						NlmCm__printf("\n\t%d :  %d",ltrRangeInst1_RegInfo.m_rangeCInsertStartByte[loop1],
							ltrRangeInst1_RegInfo.m_rangeDInsertStartByte[loop1]);
					}
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
			}
            break;

        case NLMDEV_MISCELLENEOUS_LTR:
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                (void*)&ltrMiscRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\n\tmiscellenous LTR Register Content");
				NlmDiag_ConstructLTRFields ((void *)&ltrMiscRegInfo,ltrRegRdInfo.m_ltrRegType,
							   readData, &reasonCode);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					for(loop1 = 0; loop1 < NLMDEV_NUM_PARALLEL_SEARCHES; loop1++)
					{
						NlmCm__printf("\n\t PS Num:%d  -> BMR Sel Num: %d", loop1,
							ltrMiscRegInfo.m_bmrSelect[loop1]);
					}
					NlmCm__printf("\n\t RangeA Extr Byte: %d, RangeB Extr Byte: %d, RangeC Extr Byte: %d, RangeD Extr Byte: %d",
						ltrMiscRegInfo.m_rangeAExtractStartByte, ltrMiscRegInfo.m_rangeBExtractStartByte,
						ltrMiscRegInfo.m_rangeCExtractStartByte, ltrMiscRegInfo.m_rangeDExtractStartByte);
	                
					NlmCm__printf("\n\t Valid search results: %d",ltrMiscRegInfo.m_numOfValidSrchRslts);
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
			}
            break;

       case NLMDEV_KEY_0_KCR_0_LTR:
       case NLMDEV_KEY_0_KCR_1_LTR:
       case NLMDEV_KEY_1_KCR_0_LTR:
       case NLMDEV_KEY_1_KCR_1_LTR:
       case NLMDEV_KEY_2_KCR_0_LTR:
       case NLMDEV_KEY_2_KCR_1_LTR:
       case NLMDEV_KEY_3_KCR_0_LTR:
       case NLMDEV_KEY_3_KCR_1_LTR:
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                (void*)&ltrKeyConstructRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\n\tKey configuration  LTR Register Content");
				NlmDiag_ConstructLTRFields ((void *)&ltrKeyConstructRegInfo,ltrRegRdInfo.m_ltrRegType,
							   readData, &reasonCode);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					for(loop1 = 0; loop1 < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; loop1++)
					{
						/* Valid values for BMR Select is 0 - 4 and NLMDEV_NO_MASK_BMR_NUM */
						NlmCm__printf("\n\t Segment:%d  -> Number of Bytes: %d,  Segment Location: %d ", loop1,
							ltrKeyConstructRegInfo.m_numOfBytes[loop1],ltrKeyConstructRegInfo.m_startByteLoc[loop1]);      
					}
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
			}
            break;

        case NLMDEV_SS_LTR:
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType,
                (void*)&ltrSSRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                NlmCm__printf ("Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCm__printf ("\n\tnetl LTR Register Content");
				NlmDiag_ConstructLTRFields ((void *)&ltrSSRegReadInfo,ltrRegRdInfo.m_ltrRegType,
							   readData, &reasonCode);
				NlmCm__printf ("\n\t80 bit Data = ");
				NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

				if (testInfo_p->m_userInput_p->m_verbose)
				{
					for(loop1 = 0; loop1 < NLMDEV_SS_RMP_AB; loop1+=2)
					{
						NlmCm__printf("\n\t %d    %d",ltrSSRegReadInfo.m_ss_result_map[loop1],
							ltrSSRegReadInfo.m_ss_result_map[loop1+1]);
					}
				}
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tRead LTR %d Register %d Completed", ltrRegRdInfo.m_profileNum, ltrRegRdInfo.m_ltrRegType);
			}
            break;

        case NLMDEV_LTR_REG_END:
            break;
    }   
	return NLMDIAG_ERROR_NOERROR;
}

/*Function to Write the LTR */
nlm_u32
NlmDiag_WriteToLTR (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_LTRRegWrRdInfo   ltrRegWrInfo;
    NlmDevBlkSelectReg       ltrBlkSelRegInfo;
    NlmDevSuperBlkKeyMapReg  ltrBlkKeyMapRegInfo;
    NlmDevParallelSrchReg	 ltrPrlSrchRegInfo;
    NlmDevRangeInsertion0Reg ltrRangeInst0_RegInfo;
    NlmDevRangeInsertion1Reg ltrRangeInst1_RegInfo;
    NlmDevMiscelleneousReg   ltrMiscRegInfo;
    NlmDevKeyConstructReg    ltrKeyConstructRegInfo;
    NlmDevSSReg              ltrNetLregInfo;
	NlmReasonCode reasonCode;
   
    nlm_u32 errNum;    
    nlm_u8 devNum, chnlNum = 0;
            
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
                    || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
           
    devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	ltrRegWrInfo.m_profileNum = testInfo_p->m_userInput_p->m_ltrNum;
	ltrRegWrInfo.m_ltrRegType = testInfo_p->m_userInput_p->m_registerType;

	if(ltrRegWrInfo.m_ltrRegType >= NLMDEV_LTR_REG_END 
        || ltrRegWrInfo.m_profileNum >= NLMDEV_NUM_LTR_SET)
        return NLMDIAG_ERROR_INVALID_INPUT;
    
    /* Check for Ltr Register type and construct the pattern of data 
     *  according to specified register fields */
	switch(ltrRegWrInfo.m_ltrRegType)
	{
       case NLMDEV_BLOCK_SELECT_0_LTR:
       case NLMDEV_BLOCK_SELECT_1_LTR:  /* Blk select 1 Reg is for future*/
		   	/* Initialize the Ltr Data Structure */

			if((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
							ltrRegWrInfo.m_ltrRegType,&ltrBlkSelRegInfo,
							&reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			/* Invoke Device Manager API which writes to specified Ltr */
			if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
				ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
				(void*)&ltrBlkSelRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
			}
			break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
			/* Initialize the Ltr Data Structure */
			if ((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
										ltrRegWrInfo.m_ltrRegType, &ltrBlkKeyMapRegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			/* Invoke Device Manager API which writes to specified Ltr */
			if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
				ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
				(void*)&ltrBlkKeyMapRegInfo.m_keyNum, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
			}
			break;

       case NLMDEV_PARALLEL_SEARCH_0_LTR:
       case NLMDEV_PARALLEL_SEARCH_1_LTR:
       case NLMDEV_PARALLEL_SEARCH_2_LTR:
       case NLMDEV_PARALLEL_SEARCH_3_LTR:
			/* Initialize Block to PS mappings */
			if ((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
										ltrRegWrInfo.m_ltrRegType,&ltrPrlSrchRegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			/*Invoke Device manager API which writes to specified Ltr */
			if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
				ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
				(void*)&ltrPrlSrchRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
			}
			break;
            
       case NLMDEV_RANGE_INSERTION_0_LTR:
			/* Initialise the ranges params */
			if ((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
										ltrRegWrInfo.m_ltrRegType, &ltrRangeInst0_RegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			/*Invoke Device manager API which writes to specified Ltr */
			if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
				ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
				(void*)&ltrRangeInst0_RegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
			}
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
			}
			break;


       case NLMDEV_RANGE_INSERTION_1_LTR:
           /* Initialise the ranges params */
			if ((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
										ltrRegWrInfo.m_ltrRegType,&ltrRangeInst1_RegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
               NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
            }

           /*Invoke Device manager API which writes to specified Ltr */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
               ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
               (void*)&ltrRangeInst1_RegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
               NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
            }
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
			}
            break;

       case NLMDEV_MISCELLENEOUS_LTR:
           if ((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
										ltrRegWrInfo.m_ltrRegType, &ltrMiscRegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
               NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
           }

           /*Invoke Device manager API which writes to specified Ltr */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
               ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
               (void*)&ltrMiscRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
               NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
           }
		   if(!errNum)
		   {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
		   }
           break;

       case NLMDEV_KEY_0_KCR_0_LTR:
       case NLMDEV_KEY_0_KCR_1_LTR:
       case NLMDEV_KEY_1_KCR_0_LTR:
       case NLMDEV_KEY_1_KCR_1_LTR:
       case NLMDEV_KEY_2_KCR_0_LTR:
       case NLMDEV_KEY_2_KCR_1_LTR:
       case NLMDEV_KEY_3_KCR_0_LTR:
       case NLMDEV_KEY_3_KCR_1_LTR:
            /* Initialize Key Construction Bit Mappings*/

		   if ((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
										ltrRegWrInfo.m_ltrRegType, &ltrKeyConstructRegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
            }

           /* Invoke Device Manager API to write to specified LTR */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
               ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
               (void*)&ltrKeyConstructRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                NlmCm__printf("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
            }
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
			}
            break;

       case NLMDEV_SS_LTR:           
            /* Initialize Key Construction Bit Mappings*/
            if ((errNum = NlmDiag_ExtractLTRFields(testInfo_p->m_userInput_p->m_data, 
										ltrRegWrInfo.m_ltrRegType,&ltrNetLregInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                NlmCm__printf("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
            }

           /* Invoke Device Manager API to write to specified LTR */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
               ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType,
               (void*)&ltrNetLregInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
            }
			if(!errNum)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
				NlmCm__printf("\n\n\tWrite LTR %d Register %d Completed", ltrRegWrInfo.m_profileNum, ltrRegWrInfo.m_ltrRegType);
			}
            break;

       case NLMDEV_LTR_REG_END:
            break;
	}
    return NLMDIAG_ERROR_NOERROR;
}

/*Function to Read the Block register (BCR & BMR) */
nlm_u32 
NlmDiag_ReadFromBlockReg (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_BlkRegWrRdInfo blkRegRdInfo;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	NlmDevBlockConfigReg blkConfigRegReadInfo;
    NlmDevBlockMaskReg blkMaskRegReadInfo;
    
	NlmReasonCode reasonCode;
    nlm_u32 errNum; 
    nlm_u32 value = 0;
    nlm_u8	chnlNum = 0;
    nlm_u8	devNum = 0;
	nlm_u8	readData[NLMDEV_REG_LEN_IN_BYTES] = "";		/* 80 bit hex value */
    
    /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    blkRegRdInfo.m_seed_p = &testInfo_p->m_testParams[0];
	devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	blkRegRdInfo.m_abNum = testInfo_p->m_userInput_p->m_blkNum;
	blkRegRdInfo.m_blkRegType = testInfo_p->m_userInput_p->m_registerType;

	if(blkRegRdInfo.m_abNum >= NLMDEV_NUM_ARRAY_BLOCKS 
        || blkRegRdInfo.m_blkRegType >= NLMDEV_BLOCK_REG_END)
        return NLMDIAG_ERROR_INVALID_INPUT;

    /* Check for blk Register type and compare the read 
     * data according to specified register fields */
    if(blkRegRdInfo.m_blkRegType == NLMDEV_BLOCK_CONFIG_REG )
    {
         /* Invoke the device manager API which reads specified Blk register */
        if((errNum = NlmDevMgr__BlockRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], blkRegRdInfo.m_abNum,
            blkRegRdInfo.m_blkRegType, (void*)&blkConfigRegReadInfo, &reasonCode)) 
            != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterRead",reasonCode);
            NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterRead",reasonCode);
        }
		else
		{
			NlmCm__printf ("\n\tBCR Register Content");
			value = (blkConfigRegReadInfo.m_blockWidth << 1) | blkConfigRegReadInfo.m_blockEnable;
			WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 3, 0, value);

			NlmCm__printf ("\n\t80 bit Data = ");
			NlmDiag_PrintHexData(readData, NLMDEV_REG_LEN_IN_BYTES);

			if (testInfo_p->m_userInput_p->m_verbose)
			{
				NlmCm__printf(", Block Width --> %d, Block Enable -->%d",blkConfigRegReadInfo.m_blockWidth, 
					blkConfigRegReadInfo.m_blockEnable);
			}
			NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead Block %d BCR Register Completed", blkRegRdInfo.m_abNum);
			NlmCm__printf("\n\n\tRead Block %d BCR Register Completed", blkRegRdInfo.m_abNum);
		}
    }
    else
    {   /* If regtype is BMR */
       /* Invoke the device manager API which reads specified Blk register */
        if((errNum = NlmDevMgr__BlockRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
            blkRegRdInfo.m_abNum, blkRegRdInfo.m_blkRegType,
            (void*)&blkMaskRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterRead",reasonCode);
            NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterRead",reasonCode);
        }
		else
		{
			NlmCm__printf ("\n\tBMR Register Content");
			
			NlmCm__printf ("\n\t80 bit Data = ");
			NlmDiag_PrintHexData(blkMaskRegReadInfo.m_mask, NLMDEV_REG_LEN_IN_BYTES);

			if (testInfo_p->m_userInput_p->m_verbose)
			{
				NlmCm__printf("0x%02x%02x_%02x%02x%02x%02x_%02x%02x%02x%02x\t", 
					blkMaskRegReadInfo.m_mask[9],blkMaskRegReadInfo.m_mask[8], 
					blkMaskRegReadInfo.m_mask[7], blkMaskRegReadInfo.m_mask[6],
					blkMaskRegReadInfo.m_mask[5], blkMaskRegReadInfo.m_mask[4], 
					blkMaskRegReadInfo.m_mask[3], blkMaskRegReadInfo.m_mask[2],
					blkMaskRegReadInfo.m_mask[1], blkMaskRegReadInfo.m_mask[0]);
			}
			NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead Block %d BMR %d Completed", blkRegRdInfo.m_abNum, blkRegRdInfo.m_blkRegType);
			NlmCm__printf("\n\n\tRead Block %d BMR Register %d Completed", blkRegRdInfo.m_abNum, blkRegRdInfo.m_blkRegType);
		}
    }
    return NLMDIAG_ERROR_NOERROR;
}

/*Function to Write the Block register (BCR & BMR) */
nlm_u32
NlmDiag_WriteToBlockReg (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_BlkRegWrRdInfo blkRegWrInfo;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	
	NlmDevBlockConfigReg blkConfigRegInfo;
    NlmDevBlockMaskReg blkMaskRegInfo;
    NlmReasonCode reasonCode;

    nlm_u32 errNum; 
    nlm_u8 chnlNum = 0, devNum = 0;
    
    /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;   
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    blkRegWrInfo.m_seed_p = &testInfo_p->m_testParams[0];
	devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	blkRegWrInfo.m_abNum = testInfo_p->m_userInput_p->m_blkNum;
	blkRegWrInfo.m_blkRegType = testInfo_p->m_userInput_p->m_registerType;
               
    if(blkRegWrInfo.m_abNum >= NLMDEV_NUM_ARRAY_BLOCKS 
        || blkRegWrInfo.m_blkRegType >= NLMDEV_BLOCK_REG_END)
        return NLMDIAG_ERROR_INVALID_INPUT;
    
    /* Check for blk Register type and construct the pattern of data 
     * according to specified register fields */
    if(blkRegWrInfo.m_blkRegType == NLMDEV_BLOCK_CONFIG_REG)
    {
        /* Initialize the blk config structure */
		blkConfigRegInfo.m_blockEnable = testInfo_p->m_userInput_p->m_data[9] & 0x1;
		blkConfigRegInfo.m_blockWidth = (((testInfo_p->m_userInput_p->m_data[9] & 0x6) >> 1) % 4);

        /* Invoke the device manager API which writes to specified Blk reg */
        if((errNum = NlmDevMgr__BlockRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
            blkRegWrInfo.m_abNum, blkRegWrInfo.m_blkRegType, 
            (void*)&blkConfigRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterWrite",reasonCode);
            NlmCm__printf("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterWrite",reasonCode);
        }
		if(!errNum)
		{
			NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite Block %d BCR Register Completed", blkRegWrInfo.m_abNum);
			NlmCm__printf("\n\n\tWrite Block %d BCR Register Completed", blkRegWrInfo.m_abNum);
		}
    }
    else
    {   /* If regtype is BMR */ 
		NlmCm__memcpy(blkMaskRegInfo.m_mask, testInfo_p->m_userInput_p->m_data, NLMDEV_REG_LEN_IN_BYTES);
		
		/* Invoke the device manager API which writes to specified Blk reg */
        if((errNum = NlmDevMgr__BlockRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
            blkRegWrInfo.m_abNum, blkRegWrInfo.m_blkRegType,
            (void*)&blkMaskRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterWrite",reasonCode);
            NlmCm__printf("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterWrite",reasonCode);
        }
		if(!errNum)
		{
			NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite Block %d BMR %d Completed", blkRegWrInfo.m_abNum, blkRegWrInfo.m_blkRegType);
			NlmCm__printf("\n\n\tWrite Block %d BMR Register %d Completed", blkRegWrInfo.m_abNum, blkRegWrInfo.m_blkRegType);
		}
    }
    return NLMDIAG_ERROR_NOERROR;
}

/*Function to Read the Database entry */
nlm_u32 
NlmDiag_ReadBlockEntry (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	NlmDevABEntry blkEntryRegReadInfo;
    
	NlmReasonCode reasonCode;    
    
    nlm_u32 errNum;
    nlm_u8 abNum, chnlNum = 0, devNum = 0;
	nlm_u16 abAddress;
    
    /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	abNum = testInfo_p->m_userInput_p->m_blkNum;
	abAddress = (nlm_u16)testInfo_p->m_userInput_p->m_address;

	if(abNum >= NLMDEV_NUM_ARRAY_BLOCKS)
        return NLMDIAG_ERROR_INVALID_INPUT;

    /* Invoke the device manager API which reads specified Blk register */
    if((errNum = NlmDevMgr__ABEntryRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], abNum,
        abAddress, (void*)&blkEntryRegReadInfo, &reasonCode)) 
        != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
        NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
    }
	else
	{
		NlmCm__printf ("\nDatabase entry Content\n");
		NlmCm__printf ("\n\t80 bit Data = ");
		NlmDiag_PrintHexData(blkEntryRegReadInfo.m_data, NLMDEV_REG_LEN_IN_BYTES);
		NlmCm__printf ("\n\t80 bit Mask = ");
		NlmDiag_PrintHexData(blkEntryRegReadInfo.m_mask, NLMDEV_REG_LEN_IN_BYTES);
		NlmCm__printf ("\n\tValid Bit = ");
		if (blkEntryRegReadInfo.m_vbit)
			NlmCm__printf ("Valid Entry");
		else
			NlmCm__printf ("Invalid Entry");
		
		if (testInfo_p->m_userInput_p->m_verbose)
		{
			NlmCm__printf("\n\tData : 0x%02x%02x_%02x%02x%02x%02x_%02x%02x%02x%02x\t", 
					blkEntryRegReadInfo.m_data[9],blkEntryRegReadInfo.m_data[8], 
					blkEntryRegReadInfo.m_data[7], blkEntryRegReadInfo.m_data[6],
					blkEntryRegReadInfo.m_data[5], blkEntryRegReadInfo.m_data[4], 
					blkEntryRegReadInfo.m_data[3], blkEntryRegReadInfo.m_data[2],
					blkEntryRegReadInfo.m_data[1], blkEntryRegReadInfo.m_data[0]);

			NlmCm__printf("\n\tMask : 0x%02x%02x_%02x%02x%02x%02x_%02x%02x%02x%02x\t", 
					blkEntryRegReadInfo.m_mask[9],blkEntryRegReadInfo.m_mask[8], 
					blkEntryRegReadInfo.m_mask[7], blkEntryRegReadInfo.m_mask[6],
					blkEntryRegReadInfo.m_mask[5], blkEntryRegReadInfo.m_mask[4], 
					blkEntryRegReadInfo.m_mask[3], blkEntryRegReadInfo.m_mask[2],
					blkEntryRegReadInfo.m_mask[1], blkEntryRegReadInfo.m_mask[0]);
		}
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead Database Entry from 0x%x block 0x%x location completed", abNum, abAddress);
		NlmCm__printf("\n\n\tRead Database Entry from 0x%x block 0x%x location completed", abNum, abAddress);
	}
    return NLMDIAG_ERROR_NOERROR;
}

/*Function to Write the Database entry */
nlm_u32
NlmDiag_WriteBlockEntry (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	
	NlmDevABEntry blkEntryRegWrInfo;
    NlmReasonCode reasonCode;

    nlm_u32 errNum;
	nlm_u16 abAddress;
    nlm_u8 abNum, chnlNum = 0, devNum = 0;
    
    /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;   

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	abNum = testInfo_p->m_userInput_p->m_blkNum;
	abAddress = (nlm_u16) testInfo_p->m_userInput_p->m_address;
               
    if(abNum >= NLMDEV_NUM_ARRAY_BLOCKS)
        return NLMDIAG_ERROR_INVALID_INPUT;
	NlmCm__memcpy(blkEntryRegWrInfo.m_data, testInfo_p->m_userInput_p->m_data, NLMDEV_REG_LEN_IN_BYTES);
	NlmCm__memcpy(blkEntryRegWrInfo.m_mask, testInfo_p->m_userInput_p->m_mask, NLMDEV_REG_LEN_IN_BYTES);
	blkEntryRegWrInfo.m_vbit = 1;

    /* Invoke the device manager API which writes to specified Blk reg */
    if((errNum = NlmDevMgr__ABEntryWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
        abNum, abAddress,(void*)&blkEntryRegWrInfo, NLMDEV_DM, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryWrite",reasonCode);
        NlmCm__printf ("\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryWrite",reasonCode);
    }
	if (!errNum)
	{
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite Database Entry to 0x%x block 0x%x location completed",abNum, abAddress);
		NlmCm__printf("\n\n\tWrite Database Entry to 0x%x block 0x%x location completed",abNum, abAddress);
	}
    return NLMDIAG_ERROR_NOERROR;
}

/*Function to print the hex data */
void 
NlmDiag_PrintHexData(
	nlm_u8 *string,
	nlm_u32 len
	)
{
	nlm_u32 index = 0;
	NlmCm__printf("0x");
	for (index = 0; index < len; index++)
	{
		NlmCm__printf("%02x", string[index] );
		if (index == (len-1)/2)
			NlmCm__printf("_");
	}
}

/*Function to Read the range register */
nlm_u32 
NlmDiag_ReadFromRangeRegister (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	NlmDevRangeReg rangeReadData;    
	NlmReasonCode reasonCode;    
    
    nlm_u32 errNum;
    nlm_u8 chnlNum = 0, devNum = 0;
    
    /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
	
    /* Invoke device manager API to read Range entry from the block */
    if((errNum = NlmDevMgr__RangeRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], testInfo_p->m_userInput_p->m_address, 
		&rangeReadData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:(%d), Dev Mgr Fn: NlmDevMgr__RangeRegisterRead",reasonCode);
       NlmCm__printf ("\n\tErr:(%d), Dev Mgr Fn: NlmDevMgr__RangeRegisterRead",reasonCode);
    }
	else
	{
		NlmCm__printf ("\n\nRange register Content");
		NlmCm__printf ("\n\t80 bit Data = ");
		NlmDiag_PrintHexData(rangeReadData.m_data, NLMDEV_REG_LEN_IN_BYTES);

		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tRead Range register 0x%x completed", testInfo_p->m_userInput_p->m_address);
		NlmCm__printf("\n\n\tRead Range register 0x%x completed", testInfo_p->m_userInput_p->m_address);
	}
    return NLMDIAG_ERROR_NOERROR;
}

/*Function to Write the range register */
nlm_u32
NlmDiag_WriteToRangeRegister (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	
	NlmReasonCode reasonCode;
	NlmDevRangeReg rangeData;

    nlm_u32 errNum;
	nlm_u8 chnlNum = 0, devNum = 0;
    
    /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    devNum = testInfo_p->m_userInput_p->m_devId;

    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	NlmCm__memcpy(rangeData.m_data, testInfo_p->m_userInput_p->m_data, NLMDEV_REG_LEN_IN_BYTES);

	if((errNum = NlmDevMgr__RangeRegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], testInfo_p->m_userInput_p->m_address,
        &rangeData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:(%d), Dev Mgr Fn: NlmDevMgr__RangeRegisterWrite",reasonCode);
		NlmCm__printf ("\n\tErr:(%d), Dev Mgr Fn: NlmDevMgr__RangeRegisterWrite",reasonCode);
    }
	if (!errNum)
	{
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tWrite Range register 0x%x completed", testInfo_p->m_userInput_p->m_address);
		NlmCm__printf("\n\n\tWrite Range register 0x%x completed", testInfo_p->m_userInput_p->m_address);
	}
    return NLMDIAG_ERROR_NOERROR;
}

/*Function to perform compare1 */
nlm_u32
NlmDiag_CompareOne (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	NlmDevCtxBufferInfo cbInfo;
    NlmErrNum_t errNum;
    NlmReasonCode reason;
	nlm_u8 devNum = 0, chnlNum = 0;
	NlmDevCmpRslt cmpRslts;
	
	 /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);


		
	cbInfo.m_cbStartAddr = (nlm_16)testInfo_p->m_userInput_p->m_address;
	cbInfo.m_datalen = (nlm_u8)testInfo_p->m_userInput_p->m_compDataLen;
	NlmCm__memcpy(cbInfo.m_data, testInfo_p->m_userInput_p->m_compareData, cbInfo.m_datalen);

	    /* Invoke Compare1 API to perform Compare Operation */
    if((errNum = NlmDevMgr__Compare1(moduleInfo_p->m_devMgr_pp[chnlNum],
										testInfo_p->m_userInput_p->m_ltrNum,
                                        &cbInfo,
                                        &cmpRslts,
                                        &reason
                                        )) != NLMERR_OK)
    {
        NlmCm__printf("\n\n Compare1 operation for LTR Number %d failed", testInfo_p->m_userInput_p->m_ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__Compare1', Reason Code -- %d", reason);
        return errNum;
    }
	NlmDiag_PrintCompareResult (&cmpRslts);
	return NLMDIAG_ERROR_NOERROR;
}

/*Function to perform compare2 */
nlm_u32
NlmDiag_CompareTwo (
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	NlmDevCtxBufferInfo cbInfo;
    NlmErrNum_t errNum;
    NlmReasonCode reason;
	nlm_u8 devNum = 0, chnlNum = 0;
	NlmDevCmpRslt cmpRslts;
	
	 /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
	
	cbInfo.m_cbStartAddr = (nlm_16)testInfo_p->m_userInput_p->m_address;
    cbInfo.m_datalen = (nlm_u8)testInfo_p->m_userInput_p->m_compDataLen;
	NlmCm__memcpy(cbInfo.m_data, testInfo_p->m_userInput_p->m_compareData, cbInfo.m_datalen);

	/* Invoke Compare2 API to perform Compare Operation */
    if((errNum = NlmDevMgr__Compare2(moduleInfo_p->m_devMgr_pp[chnlNum],
                                        testInfo_p->m_userInput_p->m_ltrNum,
                                        &cbInfo,
                                        &cmpRslts,
                                        &reason
                                        )) != NLMERR_OK)
    {
        NlmCm__printf("\n\n Compare2 operation for LTR Number %d failed", testInfo_p->m_userInput_p->m_ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__Compare2', Reason Code -- %d", reason);
        return errNum;
    }
	NlmDiag_PrintCompareResult (&cmpRslts);
	return NLMDIAG_ERROR_NOERROR;
}

/*Function to display compare result*/
void
NlmDiag_PrintCompareResult (
	NlmDevCmpRslt *cmpRslts_p
	)
{
	nlm_u8 iter = 0;
	/* Print the Compare results */
	NlmCm__printf("\n\n\tCompare results ::\n");
	for( iter = 0; iter < NLMDEV_NUM_PARALLEL_SEARCHES; iter++ )
	{
		NlmCm__printf("\n\tParallel Search# %d: ", iter);
		if( (Nlm11kDevMissHit)NLMDEV_HIT == cmpRslts_p->m_hitOrMiss[ iter ] )
		{
			NlmCm__printf("Found a match in deviceID %u, database index %d\n",
						cmpRslts_p->m_hitDevId[ iter ], cmpRslts_p->m_hitIndex[ iter ] );
		}
		else
			NlmCm__printf("MISS\n");
	}
}

