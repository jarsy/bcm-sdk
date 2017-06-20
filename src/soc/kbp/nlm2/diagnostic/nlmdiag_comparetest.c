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


 typedef enum NlmDiag_CmpType_e
 {
     NLMDIAG_CMP1,
     NLMDIAG_CMP2
 
 } NlmDiag_CmpType;
 
 typedef struct NlmDiag_BlkToRsltPortMap_s
 {
     NlmDev_parallel_search_t m_psNum;    /* Specifies result port number mapped to the blk */
 
 } NlmDiag_BlkToRsltPortMap;
 
 typedef struct NlmDiag_SuperBlkToKeyMap_s
 {
     NlmDev_key_t m_keyNum;    /* Specifies key number mapped to the super blk */
 
 } NlmDiag_SuperBlkToKeyMap;
 
 typedef struct NlmDiag_KeyConstructMap_s
 {
     nlm_u8 m_startByte[NLMDEV_NUM_OF_KCR_PER_KEY * NLMDEV_NUM_OF_SEGMENTS_PER_KCR];   /* Specifies the start 
                                                                                               byte of each segment 
                                                                                               of Key Construction*/
     nlm_u8 m_numOfBytes[NLMDEV_NUM_OF_KCR_PER_KEY * NLMDEV_NUM_OF_SEGMENTS_PER_KCR];  /* Specifies number of 
                                                                                               bytes each segment is */
 } NlmDiag_KeyConstructMap;
 
 typedef struct NlmDiag_LtrAttrs_s
 {
     NlmDevDisableEnable m_blkEnable[NLMDEV_NUM_ARRAY_BLOCKS];
     NlmDiag_BlkToRsltPortMap m_rsltPortBlkMap[NLMDEV_NUM_ARRAY_BLOCKS]; /* Contains the srch attributes 
                                                                                  associated with each blk */
     NlmDiag_SuperBlkToKeyMap m_keySBMap[NLMDEV_NUM_SUPER_BLOCKS]; /* Contains Super Blk to Key mapping 
                                                                            for each super blk */
     NlmDiag_KeyConstructMap m_keyConstructMap[NLMDEV_NUM_KEYS];   /* Contains the Key Construction 
                                                                            mapping for each key */
     nlm_u8 m_bmrNum[NLMDEV_NUM_PARALLEL_SEARCHES];                   /* Specifies BMR number used for 
                                                                            each srch */    
 }NlmDiag_LtrAttrs;
 
 typedef struct NlmDiag_BlkAttrs_s
 {
     nlm_u8 m_blkUsedFlag;           /* Specifies whether the block is being used or not */
     nlm_u16 m_blkWidth;             /* Specifies the block width interms of Bits*/
     nlm_u16 m_entryStartLocation;   /* Specifies the start loctaion from which entries
                                        are added to the block */
                                      
 } NlmDiag_BlkAttrs;
 
 typedef struct NlmDiag_SrchAttrs_s
 {
     nlm_u16 m_numOfParallelSearches;    /* Specifies number of parallel searches done */
     NlmDiag_CmpType m_cmpType;       /* Specifes compare type */
     nlm_u8 m_blkNum[NLMDEV_NUM_PARALLEL_SEARCHES];  /* Specifies Blk Number used for each parallel search */   
     nlm_u16 m_searchKeyStartLoc[NLMDEV_NUM_PARALLEL_SEARCHES];   /* Specifies start location of search key 
                                                                        in compare key for each parallel search */   
     nlm_u16 m_searchKeyNumberOfBytes[NLMDEV_NUM_PARALLEL_SEARCHES]; /* Specifies number of Bytes of search 
                                                                          key in compare key for each parallel search */
     nlm_u8 m_rsltPortNum[NLMDEV_NUM_PARALLEL_SEARCHES];  /* Specifies Rslt Port Number used for each 
                                                                parallel search */   
     nlm_u8 m_keyNum[NLMDEV_NUM_PARALLEL_SEARCHES];   /* Specifies Key Number Number used for each 
                                                            parallel search */   
 }NlmDiag_SrchAttrs;
 
 typedef struct NlmDiag_CmpTestAttrs_s
 {
     NlmDiag_BlkAttrs  m_blkAttrs[NLMDEV_NUM_ARRAY_BLOCKS];   /* Contains the attributes of each block */
     NlmDiag_SrchAttrs m_srchAttrs[NLMDEV_NUM_LTR_SET]; /* Contains the search attributes for each LTR */    
 } NlmDiag_CmpTestAttrs;
 

 
 /* ========================================================================
                    Compare test function declarations 
    ======================================================================== */
 static nlm_u32 NlmDiag_CompareTest( 
     NlmDiag_TestInfo              *testInfo_p
     );
            
/*==================================================================================
                               Search/Compare tests
==================================================================================*/

/* 
    This function calles the diagnostics compare test module functions,
    i.e. initialise common information, call the compare test and 
    deinitialize the test information.
 */
nlm_u32 
NlmDiag_CompareTests(void *alloc_p, 
                       NlmDiag_TestInfo *testInfo_p,
                       NlmDiag_TestCaseInfo *testCaseInfo_p
                       )
{   
    nlm_u32 errNum;
  
    /* Check the input params */
    if(alloc_p == NULL || testCaseInfo_p == NULL || testCaseInfo_p->m_testParams == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
    
    NlmCm__printf("\n\tDiagnostic compare tests are running in the sequence.\n");

    /*create test information for each test type specified */
    if((errNum = NlmDiag_CommonInitTestInfo(alloc_p, testInfo_p, testCaseInfo_p)) !=0)
    {
        testCaseInfo_p->m_errCount = 1;
        NlmCm__printf("\n\n\n\tDiagnostic Compare test is Failed (Memory Error).\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tDiagnostic Compare test is Failed (Memory Error).\n");
        return (errNum); /* return if Init() fails */
    }
    
    /* call the compare test functions */
    if((errNum = NlmDiag_CompareTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCompare test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
   
    if(errNum)
    {
        NlmCm__printf("\n\n\n\tDiagnostic Compare test is Failed.\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tDiagnostic Compare test is Failed.\n");
    }
    else
    {
        NlmCm__printf("\n\n\n\tDiagnostic Compare test is successfully completed.\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tDiagnostic Compare test successfully completed.\n");
    }

    /* Release testblkinfo after each test case */
    if((errNum = NlmDiag_CommonDestroyTestInfo(testInfo_p)) != 0)
    {
        NlmCm__printf("\n\tCon't destroy Diagnostic test information.\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCon't destroy Diagnostic test information.\n");
        return (errNum);
    }
    
    return (errNum);    
}

/* NlmDiag_InitializeBlocks function enables a block and initializes it
to the specified block width by writing to Blokc Configuration Register of the
specified Block */
static NlmErrNum_t 
NlmDiag_InitializeBlocks(NlmDev *dev_p,
							nlm_u8 blkNum,
							NlmDevBlockWidth blkWidth
							)
{
    NlmDevShadowDevice *shadowDev_p;
    NlmDevBlockConfigReg *blkConfigReg_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    /* Get the shadow memory of blk config register for the specified blkNum */
    shadowDev_p = NLMDIAG_SHADOW_DEV_PTR;
    blkConfigReg_p = &shadowDev_p->m_arrayBlock[blkNum].m_blkConfig;

    /* Initialize the Blk Config Reg Data */
    blkConfigReg_p->m_blockEnable = NLMDEV_ENABLE;
    blkConfigReg_p->m_blockWidth = blkWidth;
  
    /* Invoke the API which writes to specifies Blk Register */
    if((errNum = NlmDevMgr__BlockRegisterWrite(dev_p,
                                                  blkNum,
                                                  NLMDEV_BLOCK_CONFIG_REG,
                                                  blkConfigReg_p,
                                                  &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCm__printf("\n\n Initialization of Block Number %d failed", blkNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__BlockRegisterWrite', Reason Code -- %d", reasonCode);
        return errNum;
    }

    return NLMDIAG_ERROR_NOERROR;
}

/* 
        NlmDiag_CmpTestInitializeBlocks initializes the Blks with 
        appropriate Blk widths for compare tests
*/
NlmErrNum_t
NlmDiag_CmpTestInitializeBlocks(NlmDev *dev_p)
{
    NlmDevBlockWidth blkWidth;
    nlm_32 blkNum;
    NlmErrNum_t errNum;
    /* 
        Blk [0 - 63] initialize to 80b, 
        Blk [64 - 95] initialize to 160b, 
        Blk [96 - 111] initialize to 320b, 
        Blk [112 - 119] initialize to 640b, 
        Blk [120 - 127] Not used 
    */             
    for(blkNum = 0; blkNum < (NLMDEV_NUM_ARRAY_BLOCKS - 8); blkNum++)
    {
        if(blkNum < (NLMDEV_NUM_ARRAY_BLOCKS/2))
            blkWidth = NLMDEV_BLK_WIDTH_80; 
        else
        {
            if(blkNum < (3 * NLMDEV_NUM_ARRAY_BLOCKS/4))
                blkWidth = NLMDEV_BLK_WIDTH_160;
            else
            {
                if(blkNum < (7 * NLMDEV_NUM_ARRAY_BLOCKS/8))
                    blkWidth = NLMDEV_BLK_WIDTH_320;
                else
                    blkWidth = NLMDEV_BLK_WIDTH_640;
            }
        }

        if((errNum = NlmDiag_InitializeBlocks(dev_p, 
            (nlm_u8)blkNum, blkWidth)) != NLMDIAG_ERROR_NOERROR)
        return errNum;
    }
    return NLMDIAG_ERROR_NOERROR;   
}

NlmErrNum_t
NlmDiag_CasCmpTestInitializeBlocks(NlmDevMgr *devMgr_p)
{
    NlmDevBlockWidth blkWidth;
    nlm_32 blkNum;
    NlmErrNum_t errNum;
    nlm_u32 dev;
    NlmDev *dev_p;

    /* Blk [0 - 63] initialize to 80b, 
    Blk [64 - 95] initialize to 160b, 
    Blk [96 - 111] initialize to 320b, 
    Blk [112 - 119] initialize to 640b, 
    Blk [120 - 127] Not used 
    */
    for(dev = 0; dev < devMgr_p->m_devCount; dev++)
    {
        dev_p = devMgr_p->m_devList_pp[dev];
        for(blkNum = 0; blkNum < (NLMDEV_NUM_ARRAY_BLOCKS - 8); blkNum++)
        {
            if(blkNum < (NLMDEV_NUM_ARRAY_BLOCKS/2))
                blkWidth = NLMDEV_BLK_WIDTH_80; 
            else
            {
                if(blkNum < (3 * NLMDEV_NUM_ARRAY_BLOCKS/4))
                    blkWidth = NLMDEV_BLK_WIDTH_160;
                else
                {
                    if(blkNum < (7 * NLMDEV_NUM_ARRAY_BLOCKS/8))
                        blkWidth = NLMDEV_BLK_WIDTH_320;
                    else
                        blkWidth = NLMDEV_BLK_WIDTH_640;
                }
            }
            
            if((errNum = NlmDiag_InitializeBlocks(dev_p, 
                (nlm_u8)blkNum, blkWidth)) != NLMDIAG_ERROR_NOERROR)
            return errNum;
        }
    }
    return NLMDIAG_ERROR_NOERROR;   
}

/* This Function initializes the compare tests attributes;*/    
NlmErrNum_t
NlmDiag_InitializeCmpTestAttrs(NlmDiag_CmpTestAttrs *cmpTestAttrs)
{
    nlm_u8 blkNum;
    nlm_u8 ltrNum;
    NlmDiag_SrchAttrs *srchAttrs;

    NlmCm__memset(cmpTestAttrs, 0, sizeof(NlmDiag_CmpTestAttrs));

    /* LTR 0 -- Single 80b search */
    ltrNum = 0;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];
    srchAttrs->m_numOfParallelSearches = 1;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(rand()%4);    /* Random Blk Number Btw 0 - 3 */    
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);  /* Random Key Number */
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES); /* Random Result Port Number */
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)(((rand()%4) * 10));      
    srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;    

    /* LTR 1 -- 2 x 80b search */
    ltrNum = 1;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 2;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(4 + rand()%4); /* Random Blk Number Btw 4 - 7 */  
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)(((rand()%4) * 10));     
    srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 512;   
    
    blkNum = (nlm_u8)(8 + rand()%4); /* Random Blk Number Btw 8 - 11 */ 
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        /* Random Key Number; Check that key number is not the same as for PS -- 0 */
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        /* Random Rslt Port Number; Check that rslt port number is not the same as for PS -- 0 */
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        /* Check Start Location is not the same as for PS -- 0 */
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)(((rand()%4) * 10));  
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 1024;   
    
    /* LTR 2 -- 3 x 80b search */
    ltrNum = 2;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 3;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(12 + rand()%4); /* Random Blk Number Btw 12 - 15 */ 
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)(((rand()%4) * 10));  
    srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 1536;   
    
    blkNum = (nlm_u8)(16 + rand()%4); /* Random Blk Number Btw 16 - 19 */ 
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)(((rand()%4) * 10));        
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2048;   

    blkNum = (nlm_u8)(20 + rand()%4); /* Random Blk Number Btw 20 - 23 */ 
    srchAttrs->m_blkNum[2] = blkNum;
    do
    {
        srchAttrs->m_keyNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[1]);
    do
    {
        srchAttrs->m_rsltPortNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[1]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[2] = (nlm_u8)(((rand()%4) * 10));    
    }while(srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[0] || 
        srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[1]);

    srchAttrs->m_searchKeyNumberOfBytes[2] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2560;   
    
    /* LTR 3 -- 4 x 80b search */
    ltrNum = 3;       

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 4;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(24 + rand()%4); /* Random Blk Number Btw 24 - 27 */ 
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u8)(((rand()%4) * 10)); 
    srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 3072;   
    
    blkNum = (nlm_u8)(28 + rand()%4); /* Random Blk Number Btw 28 - 31 */ 
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)(((rand()%4) * 10));       
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 3584;   

    blkNum = (nlm_u8)(32 + rand()%4); /* Random Blk Number Btw 32 - 35 */ 
    srchAttrs->m_blkNum[2] = blkNum;
    do
    {
        srchAttrs->m_keyNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[1]);
    do
    {
        srchAttrs->m_rsltPortNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[1]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[2] = (nlm_u8)(((rand()%4) * 10));     
    }while(srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[0] || 
        srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[1]);
    srchAttrs->m_searchKeyNumberOfBytes[2] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;   

    blkNum = (nlm_u8)(36 + rand()%4); /* Random Blk Number Btw 36 - 39 */ 
    srchAttrs->m_blkNum[3] = blkNum;
    do
    {
        srchAttrs->m_keyNum[3] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[1] ||
        srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[2]);
    do
    {
        srchAttrs->m_rsltPortNum[3] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[1] ||
        srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[2]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[3] = (nlm_u16)(((rand()%4) * 10));      
    }while(srchAttrs->m_searchKeyStartLoc[3] ==  srchAttrs->m_searchKeyStartLoc[0] || 
        srchAttrs->m_searchKeyStartLoc[3] ==  srchAttrs->m_searchKeyStartLoc[1] ||
        srchAttrs->m_searchKeyStartLoc[3] ==  srchAttrs->m_searchKeyStartLoc[2]);
    srchAttrs->m_searchKeyNumberOfBytes[3] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 512;   
    
     /* LTR 4 -- Single 160b search */
    ltrNum = 4;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];
    srchAttrs->m_numOfParallelSearches = 1;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(64 + rand()%4);         /* Random Blk Number Btw 64 - 67 */ 
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)(((rand()%2) * 20));    
    srchAttrs->m_searchKeyNumberOfBytes[0] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;    

    /* LTR 5 -- 2 x 160b search */
    ltrNum = 5;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 2;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(68 + rand()%4); /* Random Blk Number Btw 68 - 71 */ 
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)(((rand()%2) * 20));     
    srchAttrs->m_searchKeyNumberOfBytes[0] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 1024;   
    
    blkNum = (nlm_u8)(72 + rand()%4); /* Random Blk Number Btw 72 - 75 */ 
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)(((rand()%2) * 20));  
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2048;   
    
    /* LTR 6 -- 3 x 160b search */
    ltrNum = 6;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 3;
    srchAttrs->m_cmpType = NLMDIAG_CMP2;

    blkNum = (nlm_u8)(76 + rand()%4); /* Random Blk Number Btw 76 - 79 */ 
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)((rand()%4) * 20);  
    srchAttrs->m_searchKeyNumberOfBytes[0] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 3072;   
    
    blkNum = (nlm_u8)(80 + rand()%4); /* Random Blk Number Btw 80 - 83 */ 
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)((rand()%4) * 20);        
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;   

    blkNum = (nlm_u8)(84 + rand()%4); /* Random Blk Number Btw 84 - 87 */ 
    srchAttrs->m_blkNum[2] = blkNum;
    do
    {
        srchAttrs->m_keyNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[1]);
    do
    {
        srchAttrs->m_rsltPortNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[1]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[2] = (nlm_u16)((rand()%4) * 20);    
    }while(srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[0] || 
        srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[1]);
    srchAttrs->m_searchKeyNumberOfBytes[2] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 1024;   

    /* LTR 7 -- 4 x 160b search */
    ltrNum = 7;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 4;
    srchAttrs->m_cmpType = NLMDIAG_CMP2;

    blkNum = (nlm_u8)(88 + rand()%4); /* Random Blk Number Btw 88 - 91 */ 
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)((rand()%4) * 20); 
    srchAttrs->m_searchKeyNumberOfBytes[0] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2048;   
    
    blkNum = (nlm_u8)(92 + rand()%4); /* Random Blk Number Btw 92 - 95 */ 
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)((rand()%4) * 20);       
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 3072;   

    do
    {
        blkNum = (nlm_u8)(64 + rand()%4); /* Random Blk Number Btw 64 - 67; Check whether the blk is not already used */ 
    }while(cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag);
    srchAttrs->m_blkNum[2] = blkNum;
    do
    {
        srchAttrs->m_keyNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[1]);
    do
    {
        srchAttrs->m_rsltPortNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[1]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[2] = (nlm_u16)((rand()%4) * 20);     
    }while(srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[0] || 
        srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[1]);
    srchAttrs->m_searchKeyNumberOfBytes[2] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;   

    do
    {
        blkNum = (nlm_u8)(68 + rand()%4);/* Random Blk Number Btw 68 - 71; Check whether the blk is not already used */ 
    }while(cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag);    
    srchAttrs->m_blkNum[3] = blkNum;
    do
    {
        srchAttrs->m_keyNum[3] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[1] ||
        srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[2]);
    do
    {
        srchAttrs->m_rsltPortNum[3] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[1] ||
        srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[2]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[3] = (nlm_u16)((rand()%4) * 20);      
    }while(srchAttrs->m_searchKeyStartLoc[3] ==  srchAttrs->m_searchKeyStartLoc[0] || 
        srchAttrs->m_searchKeyStartLoc[3] ==  srchAttrs->m_searchKeyStartLoc[1] ||
        srchAttrs->m_searchKeyStartLoc[3] ==  srchAttrs->m_searchKeyStartLoc[2]);
    srchAttrs->m_searchKeyNumberOfBytes[3] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 1024;    

     /* LTR 8 -- Single 320b search */
    ltrNum = 8;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];
    srchAttrs->m_numOfParallelSearches = 1;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(96 + rand()%4);       /* Random Blk Number Btw 96 - 99*/  
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = 0;    
    srchAttrs->m_searchKeyNumberOfBytes[0] = 40;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 320;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;    

    /* LTR 9 -- 2 x 320b search */
    ltrNum = 9;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 2;
    srchAttrs->m_cmpType = NLMDIAG_CMP2;

    blkNum = (nlm_u8)(100 + rand()%4);/* Random Blk Number Btw 100 - 103*/  
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)((rand()%2) * 40);  
    srchAttrs->m_searchKeyNumberOfBytes[0] = 40;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 320;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2048;   
    
    blkNum = (nlm_u8)(104 + rand()%4);/* Random Blk Number Btw 104 - 107*/  
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)((rand()%2) * 40);  
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 40;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 320;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;   
    
     /* LTR 10 -- Single 640b search */
    ltrNum = 10;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];
    srchAttrs->m_numOfParallelSearches = 1;
    srchAttrs->m_cmpType = NLMDIAG_CMP2;

    blkNum = 112;        
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = 0;   
    srchAttrs->m_searchKeyNumberOfBytes[0] = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 640;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;    

    /* LTR 11 -- 1 x 80b search + 1 x 160b search */
    ltrNum = 11;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 2;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(40 + rand()%4);/* Random Blk Number Btw 40 - 43*/  
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)((rand()%2) * 10);     
    srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 1024;   
    
    do
    {
        blkNum = (nlm_u8)(72 + rand()%4);/* Random Blk Number Btw 72 - 75; Check that blk is not already used*/  
    }while(cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag);
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);   
    
    srchAttrs->m_searchKeyStartLoc[1] = 20;      
    srchAttrs->m_searchKeyNumberOfBytes[1] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2048;   
    
    /* LTR 12 -- 2 x 80b search + 1 x 160b search */
    ltrNum = 12;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 3;
    srchAttrs->m_cmpType = NLMDIAG_CMP1;

    blkNum = (nlm_u8)(44 + rand()%4);/* Random Blk Number Btw 44 - 47*/  
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)((rand()%2) * 10);     
    srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 1536;   
    
    blkNum = (nlm_u8)(48 + rand()%4);/* Random Blk Number Btw 48 - 51*/  
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);       
    do
    {
        srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)((rand()%2) * 10);    
    }while(srchAttrs->m_searchKeyStartLoc[1] ==  srchAttrs->m_searchKeyStartLoc[0]);           
    srchAttrs->m_searchKeyStartLoc[1] = 10;      
    srchAttrs->m_searchKeyNumberOfBytes[1] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2048;   

    do
    {
        blkNum = (nlm_u8)(76 + rand()%4);/* Random Blk Number Btw 76 - 79; Check whether blk is not already used*/  
    }while(cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag);
    srchAttrs->m_blkNum[2] = blkNum;
    do
    {
        srchAttrs->m_keyNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[0]
            ||srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[1]);
    do
    {
        srchAttrs->m_rsltPortNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[0]
            ||srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[1]);
    srchAttrs->m_searchKeyStartLoc[2] = 20;      
    srchAttrs->m_searchKeyNumberOfBytes[2] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 3072;   

    /* LTR 13 -- 3 x 80b search + 1 x 160b search */
    ltrNum = 13;        

    srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];   
    srchAttrs->m_numOfParallelSearches = 4;
    srchAttrs->m_cmpType = NLMDIAG_CMP2;

    blkNum = (nlm_u8)(52 + rand()%4); /* Random Blk Number Btw 55 - 55 */ 
    srchAttrs->m_blkNum[0] = blkNum;
    srchAttrs->m_keyNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    srchAttrs->m_rsltPortNum[0] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)((rand()%4) * 10);  
    srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 2560;   
    
    blkNum = (nlm_u8)(56 + rand()%4); /* Random Blk Number Btw 56 - 59 */ 
    srchAttrs->m_blkNum[1] = blkNum;
    do
    {
        srchAttrs->m_keyNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[1] ==  srchAttrs->m_keyNum[0]);
    do
    {
        srchAttrs->m_rsltPortNum[1] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[1] ==  srchAttrs->m_rsltPortNum[0]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[1] = (nlm_u16)((rand()%4) * 10);        
    }while(srchAttrs->m_searchKeyStartLoc[1] == srchAttrs->m_searchKeyStartLoc[0]);
    srchAttrs->m_searchKeyNumberOfBytes[1] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 3072;   

    blkNum = (nlm_u8)(60 + rand()%4); /* Random Blk Number Btw 60 - 63 */ 
    srchAttrs->m_blkNum[2] = blkNum;
    do
    {
        srchAttrs->m_keyNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[2] ==  srchAttrs->m_keyNum[1]);
    do
    {
        srchAttrs->m_rsltPortNum[2] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[2] ==  srchAttrs->m_rsltPortNum[1]);
    do
    {
        srchAttrs->m_searchKeyStartLoc[2] = (nlm_u16)(((rand()%4) * 10));    
    }while(srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[0] || 
        srchAttrs->m_searchKeyStartLoc[2] ==  srchAttrs->m_searchKeyStartLoc[1]);

    srchAttrs->m_searchKeyNumberOfBytes[2] = 10;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 80;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 3584;   
    
    do
    {
        blkNum = (nlm_u8)(80 + rand()%4);/* Random Blk Number Btw 80 - 83; Check whether the blk is not already used */ 
    }while(cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag);    
    srchAttrs->m_blkNum[3] = blkNum;
    do
    {
        srchAttrs->m_keyNum[3] = (nlm_u8)(rand()%NLMDEV_NUM_KEYS);
    }while(srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[0] || 
        srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[1] ||
        srchAttrs->m_keyNum[3] ==  srchAttrs->m_keyNum[2]);
    do
    {
        srchAttrs->m_rsltPortNum[3] = (nlm_u8)(rand()%NLMDEV_NUM_PARALLEL_SEARCHES);
    }while(srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[0] || 
        srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[1] ||
        srchAttrs->m_rsltPortNum[3] ==  srchAttrs->m_rsltPortNum[2]);
    srchAttrs->m_searchKeyStartLoc[3] = (nlm_u16)(40 + ((rand()%2) * 20));      
    srchAttrs->m_searchKeyNumberOfBytes[3] = 20;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag = 1;
    cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth = 160;
    cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation = 0;        

    return NLMDIAG_ERROR_NOERROR;    
}

NlmErrNum_t
NlmDiag_InitializeCasCmpTestAttrs(NlmDiag_CmpTestAttrs *cmpTestAttrs,
                                           nlm_u32 numDevices
                                           )
{
    nlm_u8 ltrNum;
    nlm_u8 dev =0;
    NlmDiag_SrchAttrs *srchAttrs;

    NlmCm__memset(cmpTestAttrs, 0, (numDevices * (sizeof(NlmDiag_CmpTestAttrs))));

    /* LTR 0 -- Single 80b search : compare 1*/
    ltrNum = 0;
    for(dev = 0; dev < numDevices; dev++)
    {
        srchAttrs = &cmpTestAttrs[dev].m_srchAttrs[ltrNum];
        srchAttrs->m_numOfParallelSearches = 1;
        srchAttrs->m_cmpType = NLMDIAG_CMP1;
        srchAttrs->m_blkNum[0] = (nlm_u8)(dev%4);    /* Random Blk Number Btw 0 - 3 */ 
        srchAttrs->m_keyNum[0] = dev;                   /* Sequential Key Number */
        srchAttrs->m_rsltPortNum[0] = dev;              /* Random Result Port Number */
        srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)(((dev%4) * 10));      
        srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
        cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[0]].m_blkUsedFlag = 1;
        cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[0]].m_blkWidth = 80;
        cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[0]].m_entryStartLocation = (nlm_u16)(512 * dev);  
    }
        
    /* LTR 1 -- Single 80b search : compare 2*/
    ltrNum = 1;        

    for(dev = 0; dev < numDevices; dev++)
    {
        srchAttrs = &cmpTestAttrs[dev].m_srchAttrs[ltrNum];
        srchAttrs->m_numOfParallelSearches = 1;
        srchAttrs->m_cmpType = NLMDIAG_CMP2;
        srchAttrs->m_blkNum[0] = (nlm_u8)(20 + (dev%4));    /* Random Blk Number Btw 20 - 23 */ 
        srchAttrs->m_keyNum[0] = dev;                   /* Sequential Key Number */
        srchAttrs->m_rsltPortNum[0] = dev;              /* Random Result Port Number */
        srchAttrs->m_searchKeyStartLoc[0] = (nlm_u16)(((dev%4) * 10));      
        srchAttrs->m_searchKeyNumberOfBytes[0] = 10;
        cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[0]].m_blkUsedFlag = 1;
        cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[0]].m_blkWidth = 80;
        cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[0]].m_entryStartLocation = (nlm_u16)(512 * dev);  
    }
    return NLMDIAG_ERROR_NOERROR;    
}

/* NlmDevMgrTest_ConfigLTR function initializes the specified Search
Attributes by writing to the Logical Table Registers of the specified LTR Number.
*/
NlmErrNum_t NlmDiag_ConfigLTRRegisters(NlmDev *dev_p,
                                          nlm_u8 ltrNum,
                                          NlmDiag_LtrAttrs *ltrAttrs
                                          )
{
    NlmDevShadowDevice *shadowDev_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    nlm_32 kcrNum;
    nlm_32 keyNum;
    nlm_32 blkSelectRegNum;
    nlm_32 psRegNum;

    NlmDevBlkSelectReg       *blkSelectReg_p;
    NlmDevSuperBlkKeyMapReg  *superblkKeySelectReg_p;
    NlmDevParallelSrchReg    *parallelSrchReg_p;
    NlmDevMiscelleneousReg   *miscReg_p;
    NlmDevKeyConstructReg    *keyConstructReg_p;

    shadowDev_p = NLMDIAG_SHADOW_DEV_PTR;

    for(blkSelectRegNum = 0; blkSelectRegNum < (NLMDEV_NUM_ARRAY_BLOCKS/64); blkSelectRegNum++)
    {
        /* Get the shadow memory of appropriate Block Select Register for the specified LTR Number */
        blkSelectReg_p = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[blkSelectRegNum];

        /* Copy the Blk Select Register Data */
        NlmCm__memcpy(blkSelectReg_p->m_blkEnable,
                      &ltrAttrs->m_blkEnable[blkSelectRegNum * (NLMDEV_NUM_ARRAY_BLOCKS/2)],
                      (NLMDEV_NUM_ARRAY_BLOCKS/2) * sizeof(NlmDevDisableEnable));

        /* Invoke the API which writes to specified LTR Register */
        if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                             ltrNum,
                                                             NLMDEV_BLOCK_SELECT_0_LTR + blkSelectRegNum,
                                                             blkSelectReg_p,
                                                             &reasonCode)) != NLMDIAG_ERROR_NOERROR)
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
                  ltrAttrs->m_keySBMap,
                  NLMDEV_NUM_SUPER_BLOCKS * sizeof(NlmDevKeyNum));

    /* Invoke the API which writes to specified LTR Register */
    if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                         ltrNum,
                                                         NLMDEV_SUPER_BLK_KEY_MAP_LTR,
                                                         superblkKeySelectReg_p,
                                                         &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCm__printf("\n\n Initialization of Super Block Key Select Register of LTR Number %d failed", ltrNum);
        NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);
        return errNum;
    }

    for(psRegNum = 0; psRegNum < (NLMDEV_NUM_ARRAY_BLOCKS/32); psRegNum++)
    {
        /* Get the shadow memory of appropriate Parallel Search Register for the specified LTR Number */
        parallelSrchReg_p = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[psRegNum];

        /* Copy the Parallel Search Register Data */
        NlmCm__memcpy(parallelSrchReg_p->m_psNum,
                      &ltrAttrs->m_rsltPortBlkMap[psRegNum * (NLMDEV_NUM_ARRAY_BLOCKS/4)],
                      (NLMDEV_NUM_ARRAY_BLOCKS/4) * sizeof(NlmDevParallelSrchNum));

        /* Invoke the API which writes to specified LTR Register */
        if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                             ltrNum,
                                                             NLMDEV_PARALLEL_SEARCH_0_LTR + psRegNum,
                                                             parallelSrchReg_p,
                                                             &reasonCode)) != NLMDIAG_ERROR_NOERROR)
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
                  ltrAttrs->m_bmrNum,
                  NLMDEV_NUM_PARALLEL_SEARCHES * sizeof(nlm_u8));

    /* Invoke the API which writes to specified LTR Register */
    if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                         ltrNum,
                                                         NLMDEV_MISCELLENEOUS_LTR,
                                                         miscReg_p,
                                                         &reasonCode)) != NLMDIAG_ERROR_NOERROR)
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
                          &ltrAttrs->m_keyConstructMap[keyNum].m_startByte[kcrNum * NLMDEV_NUM_OF_SEGMENTS_PER_KCR],
                          NLMDEV_NUM_OF_SEGMENTS_PER_KCR * sizeof(nlm_u8));

            NlmCm__memcpy(keyConstructReg_p->m_numOfBytes,
                          &ltrAttrs->m_keyConstructMap[keyNum].m_numOfBytes[kcrNum * NLMDEV_NUM_OF_SEGMENTS_PER_KCR],
                          NLMDEV_NUM_OF_SEGMENTS_PER_KCR * sizeof(nlm_u8));

            /* Invoke the API which writes to specified LTR Register */
            if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                                                                 ltrNum,
                                                                 NLMDEV_KEY_0_KCR_0_LTR + keyNum * NLMDEV_NUM_OF_KCR_PER_KEY + kcrNum,
                                                                 keyConstructReg_p,
                                                                 &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCm__printf("\n\n Initialization of Key Construction %d Register of Key Number %d of LTR Number %d failed", kcrNum, keyNum, ltrNum);
                NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__LogicalTableRegisterWrite', Reason Code -- %d", reasonCode);
                return errNum;
            }
        }
    }
    return NLMDIAG_ERROR_NOERROR;
}

/* This function configures different searches by configuring different LTRs */
NlmErrNum_t
NlmDiag_CmpTestConfigureSrches(NlmDev *dev_p,
                                  NlmDiag_CmpTestAttrs *cmpTestAttrs
                                  )
{
    nlm_32 ltrNum; 
    NlmDiag_LtrAttrs ltrAttrs;
    NlmDiag_SrchAttrs *srchAttrs;
    nlm_32 segNum;
    nlm_32 startByte;
    nlm_32 numberOfBytes;
    nlm_32 psNum;
    NlmErrNum_t errNum;

    for(ltrNum = 0; ltrNum < NLMDEV_NUM_LTR_SET; ltrNum++)
    {        
        srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];
        if(srchAttrs->m_numOfParallelSearches == 0)
            continue;

        NlmCm__memset(&ltrAttrs, 0, sizeof(NlmDiag_LtrAttrs));

        for(psNum = 0; psNum < srchAttrs->m_numOfParallelSearches; psNum++)
        {
            ltrAttrs.m_blkEnable[srchAttrs->m_blkNum[psNum]] = 1;
            ltrAttrs.m_bmrNum[srchAttrs->m_rsltPortNum[psNum]] = NLMDEV_NO_MASK_BMR_NUM;/* No BMR used */
            ltrAttrs.m_rsltPortBlkMap[srchAttrs->m_blkNum[psNum]].m_psNum = srchAttrs->m_rsltPortNum[psNum];
            ltrAttrs.m_keySBMap[srchAttrs->m_blkNum[psNum]/4].m_keyNum = srchAttrs->m_keyNum[psNum];

            /* Key Construction Info; Assuming search key is continuous in master key */
            startByte = srchAttrs->m_searchKeyStartLoc[psNum];
            numberOfBytes = srchAttrs->m_searchKeyNumberOfBytes[psNum];
            segNum = 0;
            while(numberOfBytes > 0)
            {
                ltrAttrs.m_keyConstructMap[srchAttrs->m_keyNum[psNum]].m_startByte[segNum] = (nlm_u8)(startByte);

                if(numberOfBytes > 16)                
                    ltrAttrs.m_keyConstructMap[srchAttrs->m_keyNum[psNum]].m_numOfBytes[segNum] = 16; 
                else
                    ltrAttrs.m_keyConstructMap[srchAttrs->m_keyNum[psNum]].m_numOfBytes[segNum] = (nlm_u8)(numberOfBytes); 

                startByte += 16;
                numberOfBytes -= 16;
                segNum++;
            }
        }            

        if((errNum = NlmDiag_ConfigLTRRegisters(dev_p, (nlm_u8)ltrNum, &ltrAttrs)) != NLMDIAG_ERROR_NOERROR)
            return errNum;                                                 
    }

    return NLMDIAG_ERROR_NOERROR;
}

/* This function configures different searches by configuring different LTRs */
NlmErrNum_t
NlmDiag_CascadeCmpTestConfigureSrches(NlmDevMgr *devMgr_p,
                                         NlmDiag_CmpTestAttrs *cmpTestAttrs
                                         )
{
    nlm_32 ltrNum; 
    NlmDiag_LtrAttrs ltrAttrs;
    NlmDiag_SrchAttrs *srchAttrs;
    nlm_32 segNum;
    nlm_32 startByte;
    nlm_32 numberOfBytes;
    nlm_32 psNum;
    NlmErrNum_t errNum;

    nlm_u32 dev = 0;
    NlmDev *dev_p;

    for(ltrNum = 0; ltrNum < NLMDEV_NUM_LTR_SET; ltrNum++)
    {    
        for(dev = 0;dev < devMgr_p->m_devCount; dev++)
        {
            dev_p = devMgr_p->m_devList_pp[dev];
            
            srchAttrs = &cmpTestAttrs[dev].m_srchAttrs[ltrNum];
            if(srchAttrs->m_numOfParallelSearches == 0)
                continue;

            NlmCm__memset(&ltrAttrs, 0, sizeof(NlmDiag_LtrAttrs));

            for(psNum = 0; psNum < srchAttrs->m_numOfParallelSearches; psNum++)
            {
                ltrAttrs.m_blkEnable[srchAttrs->m_blkNum[psNum]] = NLMDEV_ENABLE;
                ltrAttrs.m_bmrNum[srchAttrs->m_rsltPortNum[psNum]] = NLMDEV_NO_MASK_BMR_NUM;/* No BMR used */
                ltrAttrs.m_rsltPortBlkMap[srchAttrs->m_blkNum[psNum]].m_psNum = srchAttrs->m_rsltPortNum[psNum];
                ltrAttrs.m_keySBMap[srchAttrs->m_blkNum[psNum]/4].m_keyNum = srchAttrs->m_keyNum[psNum];

                /* Key Construction Info; Assuming search key is continuous in master key */
                startByte = srchAttrs->m_searchKeyStartLoc[psNum];
                numberOfBytes = srchAttrs->m_searchKeyNumberOfBytes[psNum];
                segNum = 0;
                while(numberOfBytes > 0)
                {
                    ltrAttrs.m_keyConstructMap[srchAttrs->m_keyNum[psNum]].m_startByte[segNum] = (nlm_u8)(startByte);

                    if(numberOfBytes > 16)                
                        ltrAttrs.m_keyConstructMap[srchAttrs->m_keyNum[psNum]].m_numOfBytes[segNum] = 16; 
                    else
                        ltrAttrs.m_keyConstructMap[srchAttrs->m_keyNum[psNum]].m_numOfBytes[segNum] = (nlm_u8)(numberOfBytes); 

                    startByte += 16;
                    numberOfBytes -= 16;
                    segNum++;
                }
            }            

            if((errNum = NlmDiag_ConfigLTRRegisters(dev_p, (nlm_u8)ltrNum, &ltrAttrs)) != NLMDIAG_ERROR_NOERROR)
                return errNum;   
        }
    }
    return NLMDIAG_ERROR_NOERROR;
}
/* This function generates entry based on blk width and iter value */
void 
NlmDiag_CmpTestGenerateEntry(nlm_u8 *data,
                                nlm_u8 *mask,
                                nlm_u16 blkWidth,
                                nlm_u32 iter
                                )
{    

    switch(blkWidth)
    {
        case 80:
            if(iter < 256)   /* If iter value is less than 256; whole 80b has same data */          
                NlmCm__memset(data, iter, NLMDEV_AB_WIDTH_IN_BYTES);               
            else
            {
                iter -= 256;/* If iter value is greater than 256; LSB 40b differs MSB 40b by 1 */     
                NlmCm__memset(data, iter, NLMDEV_AB_WIDTH_IN_BYTES/2);
                NlmCm__memset(data + NLMDEV_AB_WIDTH_IN_BYTES/2, iter + 1, NLMDEV_AB_WIDTH_IN_BYTES/2);                
            }
            NlmCm__memset(mask, 0, NLMDEV_AB_WIDTH_IN_BYTES); /* Local Mask is always zero */            
            break;

        case 160:
            if(iter < 256)      /* If iter value is less than 256; whole 160b has same data */                
                NlmCm__memset(data, iter,  2 * NLMDEV_AB_WIDTH_IN_BYTES);                            
            else
            {
                iter -= 256;/* If iter value is greater than 256; LSB 80b differs MSB 80b by 1 */ 
                NlmCm__memset(data, iter, NLMDEV_AB_WIDTH_IN_BYTES);
                NlmCm__memset(data + NLMDEV_AB_WIDTH_IN_BYTES, iter + 1, NLMDEV_AB_WIDTH_IN_BYTES);                
            }
            NlmCm__memset(mask, 0, 2 * NLMDEV_AB_WIDTH_IN_BYTES);   /* Local Mask is always zero */               
            break;
            
        case 320:
            if(iter < 256)        /* If iter value is less than 256; whole 320b has same data */              
                NlmCm__memset(data, iter,  4 * NLMDEV_AB_WIDTH_IN_BYTES);                      
            else
            {
                iter -= 256;/* If iter value is greater than 256; LSB 160b differs MSB 160b by 1 */ 
                NlmCm__memset(data, iter , 2 * NLMDEV_AB_WIDTH_IN_BYTES);
                NlmCm__memset(data + 2 * NLMDEV_AB_WIDTH_IN_BYTES, iter +  1,  2 * NLMDEV_AB_WIDTH_IN_BYTES);            
            }
            NlmCm__memset(mask, 0, 4 * NLMDEV_AB_WIDTH_IN_BYTES);  /* Local Mask is always zero */                
            break;

        case 640:
            if(iter < 256)     /* If iter value is less than 256; whole 640b has same data */          
                NlmCm__memset(data, iter,  8 * NLMDEV_AB_WIDTH_IN_BYTES);                      
            else
            {
                iter -= 256;/* If iter value is greater than 256; LSB 320b differs MSB 320b by 1 */ 
                NlmCm__memset(data, iter , 4 * NLMDEV_AB_WIDTH_IN_BYTES);
                NlmCm__memset(data + 4 * NLMDEV_AB_WIDTH_IN_BYTES, iter +  1,  4 * NLMDEV_AB_WIDTH_IN_BYTES);            
            }    
            NlmCm__memset(mask, 0, 8 * NLMDEV_AB_WIDTH_IN_BYTES);   /* Local Mask is always zero */              
            break;
    }
}

/* NlmDevMgrTest_WriteABEntry function writes AB Entries to specified Blk Num and
Entry Num;
This function assumes that application has initialized the block to an appropriate
Block Width and writes AB Entries in terms of Blk Width
*/
NlmErrNum_t 
NlmDiag_WriteABEntryToDevice(NlmDev *dev_p,
                                nlm_u8 blkNum,
                                nlm_u16 entryNum,
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
    shadowDev_p = NLMDIAG_SHADOW_DEV_PTR;
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
            NlmCm__printf("\n\n Addition of AB Entry %d of Block Number %d failed", entryNum, blkNum);
            NlmCm__printf("\n Block configured to invalid block width");
            return NLMERR_FAIL;
    }

    for(segNum = 0; segNum < numberOf80bSegments; segNum++)
    {
        /* Get the shadow memory of ab Entry for the specified entryNum, segNum and blkNum */
        abEntry_p = &shadowDev_p->m_arrayBlock[blkNum].m_abEntry[entryNum + segNum];
        NlmCm__memcpy(abEntry_p->m_data, data + segNum * NLMDEV_AB_WIDTH_IN_BYTES,
            NLMDEV_AB_WIDTH_IN_BYTES);

        NlmCm__memcpy(abEntry_p->m_mask, mask + segNum * NLMDEV_AB_WIDTH_IN_BYTES,
            NLMDEV_AB_WIDTH_IN_BYTES);

        abEntry_p->m_vbit = 1;

        /* Invoke the API which writes specified AB Entry*/
        if((errNum = NlmDevMgr__ABEntryWrite(dev_p,
                                                blkNum,
                                                entryNum + segNum,
                                                abEntry_p,
                                                NLMDEV_DM,
                                                &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCm__printf("\n\n Addition of AB Entry %d of Block Number %d failed", entryNum, blkNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__ABEntryWrite', Reason Code -- %d", reasonCode);
            return errNum;
        }
    }

    return NLMDIAG_ERROR_NOERROR;
}


/* This function add database entries to different blks based on cmp test attributes */
NlmErrNum_t
NlmDiag_CmpTestAddEntries(NlmDev *dev_p,
                             NlmDiag_CmpTestAttrs *cmpTestAttrs
                             )
{    
    nlm_32 blkNum;    
    nlm_32 entryNum = 0;
    nlm_u8 data[NLMDEV_MAX_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[NLMDEV_MAX_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;
    nlm_u16 address;

    for(blkNum = 0; blkNum < NLMDEV_NUM_ARRAY_BLOCKS; blkNum++)
    {
        if(cmpTestAttrs->m_blkAttrs[blkNum].m_blkUsedFlag == 0)
            continue;      
               
        for(entryNum = 0; entryNum < NLMDIAG_NUM_MASTER_ENTRIES; entryNum++)
        {
            /* Generate the entry */
            NlmDiag_CmpTestGenerateEntry(data, 
                                            mask,
                                            cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth,
                                            entryNum);

            /* calculate the address of the entry */
            address = (nlm_u16)(cmpTestAttrs->m_blkAttrs[blkNum].m_entryStartLocation 
                + (entryNum *  cmpTestAttrs->m_blkAttrs[blkNum].m_blkWidth/80));
            
            /* Add the entry to the database */
            if((errNum = NlmDiag_WriteABEntryToDevice(dev_p,
                                                        (nlm_u8)blkNum,
                                                         address,
                                                         data, 
                                                         mask)) != NLMDIAG_ERROR_NOERROR)
                return errNum;                                                   

        }
    }
    return NLMDIAG_ERROR_NOERROR;
}
/* This function add database entries to different blks based on cmp test attributes */
NlmErrNum_t
NlmDiag_CasCmpTestAddEntries(NlmDevMgr *devMgr_p,
                                NlmDiag_CmpTestAttrs *cmpTestAttrs
                                )
{    
    nlm_32 blkNum;    
    nlm_32 entryNum = 0;
    nlm_u8 data[NLMDEV_MAX_AB_WIDTH_IN_BYTES];
    nlm_u8 mask[NLMDEV_MAX_AB_WIDTH_IN_BYTES];
    NlmErrNum_t errNum;
    nlm_u16 address;
    nlm_u16 dev = 0;
    NlmDev *dev_p;

    for(dev = 0; dev < devMgr_p->m_devCount; dev++)
    {
        dev_p = devMgr_p->m_devList_pp[dev];
        for(blkNum = 0; blkNum < NLMDEV_NUM_ARRAY_BLOCKS; blkNum++)
        {
            if(cmpTestAttrs[dev].m_blkAttrs[blkNum].m_blkUsedFlag == 0)
                continue;      
            
            for(entryNum = 0; entryNum < NLMDIAG_NUM_MASTER_ENTRIES; entryNum++)
            {
                /* Generate the entry */
                NlmDiag_CmpTestGenerateEntry(data, 
                    mask, cmpTestAttrs[dev].m_blkAttrs[blkNum].m_blkWidth, entryNum);

                /* calculate the address of the entry */
                address = (nlm_u16)(cmpTestAttrs[dev].m_blkAttrs[blkNum].m_entryStartLocation 
                    + (entryNum *  cmpTestAttrs[dev].m_blkAttrs[blkNum].m_blkWidth/80));

                /* Add the entry to the database */
                if((errNum = NlmDiag_WriteABEntryToDevice(dev_p, 
                                                             (nlm_u8)blkNum, 
                                                             address, 
                                                             data, 
                                                             mask)) != NLMDIAG_ERROR_NOERROR)
                return errNum;                                                   
            }
        }
    }
    return NLMDIAG_ERROR_NOERROR;
}

NlmErrNum_t 
NlmDiag_PerformCompare(NlmDevMgr *devMgr_p,
                          nlm_u8 ltrNum,
                          NlmDiag_CmpType compareType,
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
    cbInfo.m_datalen     = cbDatalen;
    NlmCm__memcpy(cbInfo.m_data, cmpData, cbDatalen);

    
    if(compareType == NLMDIAG_CMP1)
    {
        if((errNum = NlmDevMgr__Compare1(devMgr_p,
                                            ltrNum,
                                            &cbInfo,
                                            cmpRslts,
                                            &reason)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCm__printf("\n\n Compare1 operation for LTR Number %d failed", ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__Compare1', Reason Code -- %d", reason);
            return errNum;
        }
    }
    else
    {        
        if((errNum = NlmDevMgr__Compare2(devMgr_p,
                                            ltrNum,
                                            &cbInfo,
                                            cmpRslts,
                                            &reason )) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCm__printf("\n\n Compare2 operation for LTR Number %d failed", ltrNum);
            NlmCm__printf("\n Err: Device Manager API 'NlmDevMgr__Compare2', Reason Code -- %d", reason);
            return errNum;
        }
    }

    return NLMDIAG_ERROR_NOERROR;
}

NlmErrNum_t 
NlmDiag_VerifyCompareRslts(NlmDevCmpRslt *cmpRslts,
                              NlmDevCmpRslt *expectedCmpRslts
                              )
{
    nlm_32 psNum;

    /* Check the compare results with the expected results */
    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        if(cmpRslts->m_hitOrMiss[psNum] == expectedCmpRslts->m_hitOrMiss[psNum])
        {
            if(cmpRslts->m_hitOrMiss[psNum] == (Nlm11kDevMissHit)NLMDEV_MISS)
                continue;
            else
            {
                if(cmpRslts->m_hitDevId[psNum] != expectedCmpRslts->m_hitDevId[psNum]
                    || cmpRslts->m_hitIndex[psNum] != expectedCmpRslts->m_hitIndex[psNum])
                {
                    NlmCm__printf("\n\n Compare Operation Results Differ for PS Num %d", psNum);
                    NlmCm__printf("\n\n Got Hit At Device Id %d, Index %d, (Expected Hit At Device Id %d, Index %d)",
                        cmpRslts->m_hitDevId[psNum], cmpRslts->m_hitIndex[psNum],
                        expectedCmpRslts->m_hitDevId[psNum], expectedCmpRslts->m_hitIndex[psNum]);
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }
            }
        }
        else
        {
            if(cmpRslts->m_hitOrMiss[psNum] == (Nlm11kDevMissHit)NLMDEV_MISS)
            {
                NlmCm__printf("\n\n Compare Operation Results Differ for PS Num %d", psNum);
                NlmCm__printf("\n\n Got Miss (Expected Hit At Device Id %d, Index %d)",
                    expectedCmpRslts->m_hitDevId[psNum], expectedCmpRslts->m_hitIndex[psNum]);
                return NLMDIAG_ERROR_VERIFY_FAIL;
            }
            else
            {
                 NlmCm__printf("\n\n Compare Operation Results Differ for PS Num %d", psNum);
                 NlmCm__printf("\n\n Got Hit At Device Id %d, Index %d, (Expected Miss)",
                     cmpRslts->m_hitDevId[psNum], cmpRslts->m_hitIndex[psNum]);
                 return NLMDIAG_ERROR_VERIFY_FAIL;
            }
        }
    }

    return NLMDIAG_ERROR_NOERROR;
}

NlmErrNum_t
NlmDiag_CmpTestPerformCompares(NlmDevMgr *devMgr_p,
                                  NlmDiag_CmpTestAttrs *cmpTestAttrs
                                  )
{    
    nlm_u8 ltrNum;
    nlm_u8 cmpData[NLMDEV_MAX_CB_WRITE_IN_BYTES];

    nlm_u8 searchData[NLMDEV_MAX_CB_WRITE_IN_BYTES];
    nlm_u8 dummy[NLMDEV_MAX_CB_WRITE_IN_BYTES];
    NlmDiag_SrchAttrs *srchAttrs;
    NlmDevCmpRslt cmpRslts;
    NlmDevCmpRslt expectedCmpRslts;
    nlm_u8 searchKeyNumberOfBytes;
    nlm_u8 searchKeyLocation;
    nlm_32 psNum;
    nlm_32 cmpNum;
    nlm_u16 cbAddr;
    nlm_u16 blkWidth;
    NlmErrNum_t errNum;
    
    for(ltrNum = 0; ltrNum < NLMDEV_NUM_LTR_SET; ltrNum++)
    {       
        srchAttrs = &cmpTestAttrs->m_srchAttrs[ltrNum];
        if(srchAttrs->m_numOfParallelSearches == 0)
            continue;

        for(cmpNum = 0; cmpNum < NLMDIAG_NUM_MASTER_ENTRIES; cmpNum++)
        {
            NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));

            for(psNum = 0; psNum < srchAttrs->m_numOfParallelSearches; psNum++)
            {
                blkWidth = cmpTestAttrs->m_blkAttrs[srchAttrs->m_blkNum[psNum]].m_blkWidth;

                NlmDiag_CmpTestGenerateEntry(searchData, dummy, blkWidth, cmpNum);

                searchKeyNumberOfBytes = (nlm_u8)(srchAttrs->m_searchKeyNumberOfBytes[psNum]);
                searchKeyLocation = (nlm_u8)(srchAttrs->m_searchKeyStartLoc[psNum]);

                /* Construct the compare data based on search key location */
                while(searchKeyNumberOfBytes != 0)
                {
                    NlmCm__memcpy(cmpData + (NLMDEV_MAX_CB_WRITE_IN_BYTES - (searchKeyLocation + searchKeyNumberOfBytes)),
                        searchData + searchKeyNumberOfBytes - NLMDEV_REG_LEN_IN_BYTES,
                        NLMDEV_REG_LEN_IN_BYTES);
                    searchKeyNumberOfBytes -= NLMDEV_REG_LEN_IN_BYTES;                    
                }

                /* Initialize the  expected results */
                expectedCmpRslts.m_hitDevId[srchAttrs->m_rsltPortNum[psNum]] = 0;
                expectedCmpRslts.m_hitOrMiss[srchAttrs->m_rsltPortNum[psNum]] = NLMDEV_HIT;
                expectedCmpRslts.m_hitIndex[srchAttrs->m_rsltPortNum[psNum]] = 
                    srchAttrs->m_blkNum[psNum] << 12 |
                    (cmpTestAttrs->m_blkAttrs[srchAttrs->m_blkNum[psNum]].m_entryStartLocation + 
                    cmpNum * (blkWidth/80));
            }

            /* CB addr is generated such that LS 3bits are always zero */
            cbAddr = (nlm_u16)((rand()% (NLMDEV_CB_DEPTH/8)) * 8); 
                        
            if((errNum = NlmDiag_PerformCompare(devMgr_p, ltrNum, srchAttrs->m_cmpType,
                cbAddr, NLMDEV_MAX_CB_WRITE_IN_BYTES, cmpData, &cmpRslts)) != NLMDIAG_ERROR_NOERROR)            
                return errNum;
            
            if((errNum = NlmDiag_VerifyCompareRslts(&cmpRslts, &expectedCmpRslts)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCm__printf("\n\n Compare Number %d of LTR Num %d Failed", cmpNum, ltrNum);
                return errNum;
            }
        }        
    }    
    NlmCm__printf("\n\n\t -> %d entries compared successfully", NLMDIAG_NUM_MASTER_ENTRIES);
    return NLMDIAG_ERROR_NOERROR;
}

NlmErrNum_t
NlmDiag_CascadeCmpTestPerformCompares(NlmDevMgr *devMgr_p,
                                         NlmDiag_CmpTestAttrs *cmpTestAttrs
                                         )
{    
    nlm_u8 ltrNum;
    nlm_u8 flag = NlmTrue;
    nlm_u8 cmpData[NLMDEV_MAX_CB_WRITE_IN_BYTES];

    nlm_u8 searchData[NLMDEV_MAX_CB_WRITE_IN_BYTES];
    nlm_u8 dummy[NLMDEV_MAX_CB_WRITE_IN_BYTES];
    NlmDiag_SrchAttrs *srchAttrs;
    NlmDevCmpRslt expectedCmpRslts;
    NlmDevCmpRslt cmpRslts;
    nlm_u8 searchKeyNumberOfBytes;
    nlm_u8 searchKeyLocation;
    nlm_32 psNum;
    nlm_32 cmpNum;
    nlm_u16 cbAddr;
    nlm_u16 blkWidth;
    nlm_u32 dev = 0;
    NlmErrNum_t errNum;
    NlmDiag_CmpType compType = NLMDIAG_CMP1;
    
    for(cmpNum = 0; cmpNum < NLMDIAG_NUM_MASTER_ENTRIES; cmpNum++)
    {
        NlmCm__memset(&expectedCmpRslts, 0, sizeof(NlmDevCmpRslt));
        /* LTR Number */
        for(ltrNum = 0; ltrNum < NLMDEV_NUM_LTR_SET; ltrNum++)
        {
            /* Device(s) */
            for(dev = 0; dev < devMgr_p->m_devCount; dev++)
            {
                srchAttrs = &cmpTestAttrs[dev].m_srchAttrs[ltrNum];
                if(srchAttrs->m_numOfParallelSearches == 0)
                {
                    flag = NlmFalse;
                    continue;
                }
                compType = srchAttrs->m_cmpType;
                for(psNum = 0; psNum < srchAttrs->m_numOfParallelSearches; psNum++)
                {
                    blkWidth = cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[psNum]].m_blkWidth;
                    NlmDiag_CmpTestGenerateEntry(searchData, dummy, blkWidth, cmpNum);

                    searchKeyNumberOfBytes = (nlm_u8)(srchAttrs->m_searchKeyNumberOfBytes[psNum]);
                    searchKeyLocation = (nlm_u8)(srchAttrs->m_searchKeyStartLoc[psNum]);

                    /* Construct the compare data based on search key location */
                    while(searchKeyNumberOfBytes != 0)
                    {
                        NlmCm__memcpy(cmpData + (NLMDEV_MAX_CB_WRITE_IN_BYTES - 
                            (searchKeyLocation + searchKeyNumberOfBytes)),
                            searchData + searchKeyNumberOfBytes - NLMDEV_REG_LEN_IN_BYTES,
                            NLMDEV_REG_LEN_IN_BYTES);
                        searchKeyNumberOfBytes -= NLMDEV_REG_LEN_IN_BYTES;                    
                    }

                    /* Initialize the  expected results */
                    expectedCmpRslts.m_hitDevId[srchAttrs->m_rsltPortNum[psNum]] = (nlm_u8)dev;
                    expectedCmpRslts.m_hitOrMiss[srchAttrs->m_rsltPortNum[psNum]] = NLMDEV_HIT;
                    expectedCmpRslts.m_hitIndex[srchAttrs->m_rsltPortNum[psNum]] = 
                            srchAttrs->m_blkNum[psNum] << 12
                            | (cmpTestAttrs[dev].m_blkAttrs[srchAttrs->m_blkNum[psNum]].m_entryStartLocation
                            + cmpNum * (blkWidth/80));
                }
                flag = NlmTrue;
            } /* Device(s) */

            if(flag)
            {
                /* per Entry */
                cbAddr = (nlm_u16)((rand()% (NLMDEV_CB_DEPTH/8)) * 8); /* CB addr is generated such that LS 3bits are always zero */
                            
                if((errNum = NlmDiag_PerformCompare(devMgr_p, ltrNum, compType,
                    cbAddr, NLMDEV_MAX_CB_WRITE_IN_BYTES, cmpData, &cmpRslts)) != NLMERR_OK)
                    return errNum;
                
                if((errNum = NlmDiag_VerifyCompareRslts(&cmpRslts, &expectedCmpRslts)) != NLMDIAG_ERROR_NOERROR)
                {   
                    NlmCm__printf("\n\n Compare Number %d of LTR Num %d Failed", cmpNum, ltrNum);
                    return errNum;
                }
            }
        } /* LTR Number */    
    
        if( ((cmpNum+1) % 100) == 0)
            NlmCm__printf("\n\n\t -> %d Entries are compared successfully", cmpNum+1);
    }    
    
    return NLMDIAG_ERROR_NOERROR;
}
/* 
    This compare test function do the sequence of steps as below:
    1. Initializes the device, configures the registers (BCR, BMRs and LTRs).
    2. Writes entires (keys) into the AB blocks (database).
    3. Generates master keys.
    4. Writes search key into the CB blocks and searches key in the device.
    5. Result is compared with the expected values (shadow memory).
 */
nlm_u32 
NlmDiag_CompareTest(NlmDiag_TestInfo *testInfo_p) 
{  
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmErrNum_t errNum;        
    nlm_u8 chnlNum = 0;
    nlm_u8 devNum;

    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;   

    if(testInfo_p->m_testType == NLMDIAG_CMP1_TEST)
    {
        NlmCm__printf("\n\tCompare1 test Initialized.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCompare1 test Initialized.");
    }
    else
    {
        NlmCm__printf("\n\tCompare2 test Initialized.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Compare2 test initialized.");
    }
    
        
    /* Initialise the device in specified operation mode and compare type given */
    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)                   
        return(errNum);           

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
     
    if(testInfo_p->m_numOfChips == 1)
    {
        NlmDiag_CmpTestAttrs cmpTestAttrs;

        /* Initialize all the block registers, by calling the device manager APIs */
        NlmCm__printf("\n\n\n\tWriting to Block Configuration Registers (BCRs) \n\t");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tWriting to Block Configuration Registers (BCRs)");  
        devNum = 0;           
        if((errNum = NlmDiag_CmpTestInitializeBlocks(moduleInfo_p->m_dev_ppp[chnlNum][devNum])) != NLMDIAG_ERROR_NOERROR)
            return errNum;  

        srand(testInfo_p->m_testParams[0]);    
        /* Initialize cmp test attributes */
        NlmCm__printf("\n\n\tInitializing the compare attributes.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tInitializing the compare attributes.");
        if((errNum = NlmDiag_InitializeCmpTestAttrs(&cmpTestAttrs)) != NLMDIAG_ERROR_NOERROR)
            return errNum;
        
        /* Configure the searches */
        NlmCm__printf("\n\n\tConfigure the searches for the LTRs.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tConfigure the searches for the LTRs.");
        if((errNum = 
            NlmDiag_CmpTestConfigureSrches(moduleInfo_p->m_dev_ppp[chnlNum][devNum], &cmpTestAttrs))!= NLMDIAG_ERROR_NOERROR)
        return errNum;

        /* Add entries to the blks */
        NlmCm__printf("\n\n\tAdd entries to the devices.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tAdd entries to the devices.");
        if((errNum = 
            NlmDiag_CmpTestAddEntries(moduleInfo_p->m_dev_ppp[chnlNum][devNum], &cmpTestAttrs))!= NLMDIAG_ERROR_NOERROR)
        return errNum;

        /* Perform compare operations */
        NlmCm__printf("\n\n\tCompare the added entries in the devices.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCompare the added entries in the devices.");
        if((errNum = 
            NlmDiag_CmpTestPerformCompares(moduleInfo_p->m_devMgr_pp[chnlNum], &cmpTestAttrs))!= NLMDIAG_ERROR_NOERROR)
        return errNum;
    }
    else
    {
        NlmDiag_CmpTestAttrs cmpTestAttrs[4];
        devNum = 0;         
	
	    NlmCm__memset(cmpTestAttrs, 0, (4 * sizeof(NlmDiag_CmpTestAttrs)));
        /* Initilialize the Blks with appropriate Blk widths for cascaded devices;*/   
        NlmCm__printf("\n\n\n\tWriting to Block Configuration Registers (BCRs) \n\t");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tWriting to Block Configuration Registers (BCRs)");
        if((errNum = 
            NlmDiag_CasCmpTestInitializeBlocks(moduleInfo_p->m_devMgr_pp[chnlNum])) != NLMDIAG_ERROR_NOERROR)
        return errNum;
             
        srand(testInfo_p->m_testParams[0]);
        /* Initialize cmp test attributes */
        NlmCm__printf("\n\n\tInitializing the compare attributes.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tInitializing the compare attributes.");
        if((errNum = NlmDiag_InitializeCasCmpTestAttrs(
            &cmpTestAttrs[0], moduleInfo_p->m_devMgr_pp[chnlNum]->m_devCount)) != NLMDIAG_ERROR_NOERROR)
        return errNum;

        /* Configure the searches */
        NlmCm__printf("\n\n\tConfigure the searches for the LTRs.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tConfigure the searches for the LTRs.");
        if((errNum = NlmDiag_CascadeCmpTestConfigureSrches(
            moduleInfo_p->m_devMgr_pp[chnlNum], &cmpTestAttrs[0]))!= NLMDIAG_ERROR_NOERROR)
        return errNum;

        /* Add entries to the blks */
        NlmCm__printf("\n\n\tAdd entries to the devices.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tAdd entries to the devices.");
        if((errNum = NlmDiag_CasCmpTestAddEntries(
            moduleInfo_p->m_devMgr_pp[chnlNum], &cmpTestAttrs[0]))!= NLMDIAG_ERROR_NOERROR)
        return errNum;

        /* Perform compare operations */
        NlmCm__printf("\n\n\tCompare the added entries in the devices.");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCompare the added entries in the devices.");
        if((errNum = NlmDiag_CascadeCmpTestPerformCompares(
            moduleInfo_p->m_devMgr_pp[chnlNum], &cmpTestAttrs[0]))!= NLMDIAG_ERROR_NOERROR)
        return errNum;
    }

    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);  
    return NLMDIAG_ERROR_NOERROR;     
}


/*  11  */


