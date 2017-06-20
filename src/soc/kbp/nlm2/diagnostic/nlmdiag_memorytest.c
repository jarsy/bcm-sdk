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


 /* 
     This stucture holds the information about AB block to which the new AB entries
     are added or read. The width of the AB block, seed value to generate keys and
     number of entries are specified.
  */
 typedef struct  NlmDiag_ABWrRdInfo_s
 {
     nlm_u8      m_abNum;         /* Array Block number, to which the record added/read */
     nlm_u16     m_startAddress;  /* Start address of the block being referenced */
     nlm_u16     m_numOfEntries;  /* Number of entries to be stored this block */
     nlm_u32     *m_seed_p;       /* Seed value to generate random AB entries */
     nlm_u32     m_flag;          /* Test specific flags, needed for typecast if set */
     void        *anyInfo_p;      /* Test specific info (depends on flags for typecast purpose)*/
 
     NlmDevBlockWidth      m_width;        /* Block width of AB, see NlmDevBlockWidth */
     NlmDiag_DataPattern   m_dataPattern;  /* Pattern in which the data key is generated */
     NlmDiag_DataPattern   m_maskPattern;  /* Pattern in which the mask key is generated */
 
 } NlmDiag_ABWrRdInfo;

 
 /* 
     This stucture holds the information about invalidating (delete) the AB entries
     for the specified AB block, and the number of entries to be invalidate 
  */
 typedef struct  NlmDiag_ABInvalidateInfo_s
 {
     nlm_u8      m_abNum;         /* Array Block number, from which the record is invalideted. */
     nlm_u16     m_startAddress;  /* Start address of the block being referenced to invalidate */
     nlm_u16     m_numOfEntries;  /* Number of entries to be invalidated */
     nlm_u32     m_flag;          /* Test specific flags */
     void        *anyInfo_p;      /* Test specific info (depends on flags)*/
 
     NlmDevBlockWidth     m_width;    /* Block width: 80b, 160b, 320b or 640b */
 
 } NlmDiag_ABInvalidateInfo;

/* ST structure */
 typedef struct  NlmDiag_STWrRdInfo_s
{
    nlm_u8                  m_stNum;           /* ST number */
    nlm_u16                 m_startAddress;     /* start address*/
    nlm_u16                 m_numOfEntries;     /* Number of entries */
    NlmDiag_DataPattern  m_pattern;          /* data pattern type */
    nlm_u32                 *m_seed_p;          /* seed value */
    nlm_u32                 m_flag;             /* Test specific flags */
    void                     *anyInfo_p;        /* Test specific info (depends on flags)*/

} NlmDiag_STWrRdInfo;


 /* ========================================================================
             Memeory test functions (AB write, read, and invalidate)
    ======================================================================== */
 static nlm_u32 NlmDiag_ABWriteAndReadTest(   
      NlmDiag_TestInfo             *testInfo_p    
     );
 
 static nlm_u32  NlmDiag_WriteToAB(
     NlmDev                        *dev_p,
     NlmDiag_ABWrRdInfo            *ABWrInfo_p,
     NlmCmFile                        *logFile
     );
 
 static nlm_u32  NlmDiag_ReadFromABAndCompare(
     NlmDev                        *dev_p,
     NlmDiag_ABWrRdInfo            *ABRdInfo_p,
     NlmCmFile                        *logFile
     );
 
 static nlm_u32  NlmDiag_InvalidateABEntry(
     NlmDev                        *dev_p,
     NlmDiag_ABInvalidateInfo      *ABInvalidateInfo_p,
     NlmCmFile                       *logFile
     );

 static nlm_u32 NlmDiag_CompareABData(
     nlm_u8                          ab_num,
     nlm_u16                         addr,   
     NlmDiag_WriteReadFlag        rdWrFlag,
     nlm_u8                          *dataWord,
     nlm_u8                          *dataMask,
     nlm_u8                          *read_X, 
     nlm_u8                          *read_Y,                                                                                   
     NlmCmFile                       *logFile
     );

 /* ========================================================================
						ST test functions 
    ======================================================================== */

 static nlm_u32 NlmDiag_STReadWriteTest(
     NlmDiag_TestInfo              *testInfo_p
     );


 /* ST write/read function delcarations */
 static nlm_u32 NlmDiag_ReadFromSTandCompare(
     NlmDev                        *dev_p,
     NlmDiag_STWrRdInfo           *stRdInfo_p,
     NlmCmFile                        *logFile
     );

 static nlm_u32 NlmDiag_WriteToST(
     NlmDev                        *dev_p,
     NlmDiag_STWrRdInfo           *stWrInfo_p,
     NlmCmFile                        *logFile
     );


/*==================================================================================
                       Memory write and read tests (AB block test)
==================================================================================*/

/* 
    This function calls the diagnostics memory test module functions,
    i.e. initialise common information, call the memory read/write test
    and deinitialize the test information.
 */
nlm_u32 
NlmDiag_MemoryTests(void *alloc_p, 
                      NlmDiag_TestInfo *testInfo_p,
                      NlmDiag_TestCaseInfo *testCaseInfo_p
                      )
{   
    nlm_u32 errNum;
  
    /* Check for the input parameters */
    if(alloc_p == NULL || testCaseInfo_p == NULL || testCaseInfo_p->m_testParams == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
       
    NlmCm__printf("\n\tDiagnostic memory tests are running ......\n");

    /*create test information for each test type specified */
    if((errNum = NlmDiag_CommonInitTestInfo(alloc_p, testInfo_p, testCaseInfo_p)) !=0)
    {
        testCaseInfo_p->m_errCount = 1;
        NlmCm__printf("\n\n\n\tDiagnostic Memory test is Failed (Memory Error).\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tDiagnostic Memory test is Failed (Memory Error).\n");
        return (errNum); /* return if Init fails */
    }

    NlmCm__printf("\n\n\tDevice Memory write and read tests."); 
	NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tDevice Memory write and read tests.");

    /* call the memory read/write functions */
    if((errNum = NlmDiag_ABWriteAndReadTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tMemory test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
    
	if (testInfo_p->m_operMode == NLMDEV_OPR_SAHASRA)
    {
		NlmCm__printf("\n\n\tDevice ST write and read tests."); 
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tDevice ST write and read tests.");
        if((errNum = NlmDiag_STReadWriteTest(testInfo_p)) != 0)
        {
            /* print the error description and increment the error count*/
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t ST write/read test FAILED\n");
            testCaseInfo_p->m_errCount = 1;
        }
    }

    if(errNum)
    {
        NlmCm__printf("\n\tDiagnostic Memory test Failed.\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n Diagnostic Memory test Failed.\n");
    }
    else
    {
        NlmCm__printf("\n\tDiagnostic Memory test successfully completed.\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n Diagnostic Memory test successfully completed.\n");
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


/* 
   This function calls the device manager AB read, write, and invalidate APIs
    1. Write the each block with specified width
    2. read back the data from each block and compare
    3. invalidate all block entries and expect valid bit is reset.
*/

nlm_u32 
NlmDiag_ABWriteAndReadTest(NlmDiag_TestInfo *testInfo_p) 
{  
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_ABWrRdInfo ABWrRdInfo;
    NlmDiag_ABInvalidateInfo ABInvalidateInfo;
    nlm_u32 errNum;    
    
    nlm_u8 abNum;
    void *anyInfo_p=NULL;
    nlm_u32 anyFlag=0;
    nlm_u8 chnlNum = 0;
    nlm_u8 devNum, pattern;

    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL 
        || testInfo_p->m_testLogFile == NULL  || testInfo_p->m_testParams == NULL 
        || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)                   
        return(errNum);   

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;    

     if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    /* Initialize the structure used to test the read and write to AB Entries */
    ABWrRdInfo.anyInfo_p = anyInfo_p;
    ABWrRdInfo.m_flag = anyFlag;

    /* Seed is provided as an parameter with the test case */
    ABWrRdInfo.m_seed_p = &testInfo_p->m_testParams[0]; 

    /* Carry the test for different patterns, widths and ArrayBlocks.
     *(here exclude NLMDIAG_PATTERN_COMPARE_TYPE, as this pattern used 
     * for specific purpose .i.e. in compare function).
     */

    /* Carry the test for all elevan patterns excluding the compare type pattern.
     * for random patterns use the seed value with the command line argument -seed #value. 
     * If this argument is not provided then use default defined one .i.e.NLMDIAG_DEFAULT_SEED_VALUE
     */

    /* only pattern type NLMDIAG_PATTERN_RANDOM used, because if we use all patterns
       huge log file is generated.

       Use NLMDIAG_PATTERN_ALL_0S as start pattern for on device 
     */
    for(pattern = NLMDIAG_PATTERN_RANDOM; pattern < NLMDIAG_PATTERN_END; pattern++)
    {
        ABWrRdInfo.m_dataPattern = pattern;
        ABWrRdInfo.m_maskPattern = pattern;

        NlmCm__printf("\n\n\n\t Writing data pattern [%d] into AB block.\t\t", pattern);

        /* Carry the test for different patterns, widths and ABs */ 
        for (ABWrRdInfo.m_width = NLMDEV_BLK_WIDTH_80; 
                            ABWrRdInfo.m_width <= NLMDEV_BLK_WIDTH_640; ABWrRdInfo.m_width++ )
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t Memory (AB) Write Read test of Width (%3d)",
                (80 * (NLMDIAG_GET_AB_WIDTH(ABWrRdInfo.m_width))));
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t   -> Writing to AB block.\n\t\t\t  ");

            NlmCm__printf("\n\n\t         Memory (AB) Write Read test of width : %3d", (80 * (NLMDIAG_GET_AB_WIDTH(ABWrRdInfo.m_width))));
            NlmCm__printf("\n\t\t   -> Writing to AB block.\n\t\t\t  ");
            for(devNum=0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
            {
                if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL)
                    return(NLMDIAG_ERROR_INVALID_INPUTPTR);

                for(abNum = 0; abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
                {
                    /* Fill the valid information into the structure, to write into memory */
                    ABWrRdInfo.m_abNum = abNum;        
                    ABWrRdInfo.m_startAddress = 0;             
                    ABWrRdInfo.m_numOfEntries = (nlm_u16)(NLMDEV_AB_DEPTH/(NLMDIAG_GET_AB_WIDTH(ABWrRdInfo.m_width)));    

                    /* Invoke the test function which perform AB Entry write */
                    if((errNum = NlmDiag_WriteToAB(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                                &ABWrRdInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)                   
                        return(errNum); 
                    if((abNum%16)==0)
                        NlmCm__printf(".");
                }
            }
            
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t   -> Read from AB block and Compare.\n\t\t\t  ");
            NlmCm__printf("\n\t\t   -> Read from AB block and Compare.\n\t\t\t  ");
            for(devNum=0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
            {
                for(abNum = 0; abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
                {
                    /* Fill the valid information into the structure, to read from memory */
                    ABWrRdInfo.m_abNum = abNum;        
                    ABWrRdInfo.m_startAddress = 0;             
                    ABWrRdInfo.m_numOfEntries = (nlm_u16)(NLMDEV_AB_DEPTH/(NLMDIAG_GET_AB_WIDTH(ABWrRdInfo.m_width)));      

                    /* Invoke the test function which perform AB Entry read */
                    if((errNum = NlmDiag_ReadFromABAndCompare(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
                                &ABWrRdInfo,  testInfo_p->m_testLogFile)) !=NLMDIAG_ERROR_NOERROR)                 
                        return(errNum);  
                    if((abNum%16)==0)
                        NlmCm__printf(".");
                }
            }

            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t   -> Memory (AB) Invalidate test \n\t\t\t  ");
            ABInvalidateInfo.anyInfo_p = anyInfo_p;
            ABInvalidateInfo.m_flag = anyFlag;

            /* Initialize the structure used to test the Invalidation of AB entries */
            ABInvalidateInfo.m_width = NLMDEV_BLK_WIDTH_80;
            NlmCm__printf("\n\t\t   -> Invalidate AB block entry, Read and compare.\n\t\t\t  ");
            for(devNum=0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
            {
                for(abNum = 0; abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
                {
                    /* Fill the valid information into the structure, to invalidte entry given*/
                    ABInvalidateInfo.m_abNum = abNum;        
                    ABInvalidateInfo.m_startAddress = 0;             
                    ABInvalidateInfo.m_numOfEntries = (nlm_u16)(NLMDEV_AB_DEPTH/(NLMDIAG_GET_AB_WIDTH(ABWrRdInfo.m_width)));   

                    /* Invoke the test fn which invalidate AB Entries and verifies */
                    if((errNum = NlmDiag_InvalidateABEntry(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
                            &ABInvalidateInfo,  testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)                   
                        return(errNum);
                    if((abNum%16)==0)
                        NlmCm__printf(".");
                }
            }
        } /* end width */ 
    }  /* end pattern */          

    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);  
    return NLMDIAG_ERROR_NOERROR;    
}


nlm_u32 
NlmDiag_WriteToAB(NlmDev *dev_p,
                    NlmDiag_ABWrRdInfo *ABWrInfo_p,
                    NlmCmFile *logFile
                    )
{  
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
    nlm_u8 dataWord[640/8], dataMask[640/8];
    nlm_u16 loop_ntry, loop_width, abWidth, writeAddress;

    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || 
                            ABWrInfo_p == NULL || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
        
    if(ABWrInfo_p->m_abNum >= NLMDEV_NUM_ARRAY_BLOCKS || 
            ABWrInfo_p->m_startAddress >= NLMDEV_AB_DEPTH  ||
            ABWrInfo_p->m_numOfEntries == 0                  || 
            ABWrInfo_p->m_width > NLMDEV_BLK_WIDTH_640  ||
            (ABWrInfo_p->m_startAddress + (ABWrInfo_p->m_numOfEntries*ABWrInfo_p->m_width)) > NLMDEV_AB_DEPTH)
    {
        return NLMDIAG_ERROR_INVALID_INPUT;
    }

    abWidth = (nlm_u16)NLMDIAG_GET_AB_WIDTH(ABWrInfo_p->m_width);
    writeAddress = ABWrInfo_p->m_startAddress;
   
    for(loop_ntry=0; loop_ntry < ABWrInfo_p->m_numOfEntries; loop_ntry++)
    {         
        /* Generate the specified data and mask patterns*/ 
        ABWrInfo_p->m_flag = NLMDEV_ABENTRY_DATA;
        if((errNum = NlmDiag_CommonGenDataPattern(ABWrInfo_p->m_dataPattern, 
            dataWord, NLMDEV_REG_LEN_IN_BYTES*abWidth, ABWrInfo_p->m_seed_p,
            ABWrInfo_p->m_flag, ABWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
            return(errNum);

        ABWrInfo_p->m_flag = NLMDEV_ABENTRY_MASK;
        if((errNum = NlmDiag_CommonGenDataPattern(ABWrInfo_p->m_maskPattern, 
            dataMask, NLMDEV_REG_LEN_IN_BYTES*abWidth, ABWrInfo_p->m_seed_p, 
            ABWrInfo_p->m_flag, ABWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
            return(errNum);

        for(loop_width=0; loop_width < abWidth; loop_width++)
        {
            /* copy the generated data/mask shadow memory (golden data/mask) */
            NlmCm__memcpy(NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABWrInfo_p->m_abNum].m_abEntry[writeAddress].m_data,
                &dataWord[(writeAddress%abWidth)*NLMDEV_REG_LEN_IN_BYTES], NLMDEV_REG_LEN_IN_BYTES);
            NlmCm__memcpy(NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABWrInfo_p->m_abNum].m_abEntry[writeAddress].m_mask,
                &dataMask[(writeAddress%abWidth)*NLMDEV_REG_LEN_IN_BYTES], NLMDEV_REG_LEN_IN_BYTES);

            /* set the valid bit, as this is valid entry being adding into the memory block (database )*/
            NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABWrInfo_p->m_abNum].m_abEntry[writeAddress].m_vbit = 1;         
            
            /* print the AB data being written into the database*/
            NlmDiag_CompareABData(ABWrInfo_p->m_abNum, writeAddress, NLMDIAG_WRITE_FLAG, 
                NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABWrInfo_p->m_abNum].m_abEntry[writeAddress].m_data, 
                NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABWrInfo_p->m_abNum].m_abEntry[writeAddress].m_mask, 
                (void*)0 , (void *)0, logFile);

            /* Invoke the Device manager API which writes to a specified AB entry */
            if((errNum = NlmDevMgr__ABEntryWrite(dev_p, ABWrInfo_p->m_abNum, 
                writeAddress ,&NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABWrInfo_p->m_abNum].m_abEntry[writeAddress],
                NLMDEV_DM, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile,"\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            writeAddress++;
        }        
    }
    return NLMDIAG_ERROR_NOERROR;
}


nlm_u32 
NlmDiag_ReadFromABAndCompare(NlmDev *dev_p,
								NlmDiag_ABWrRdInfo *ABRdInfo_p,
								NlmCmFile *logFile
								)
{  
    
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
    NlmDevABEntry readEntry;
    nlm_u16 loop_ntry, loop_width, abWidth, readAddress;          
       
    
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || 
                            ABRdInfo_p == NULL || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(ABRdInfo_p->m_abNum >= NLMDEV_NUM_ARRAY_BLOCKS
        || ABRdInfo_p->m_startAddress >= NLMDEV_AB_DEPTH
        || ABRdInfo_p->m_numOfEntries == 0 
        || ABRdInfo_p->m_width > NLMDEV_BLK_WIDTH_640 
        || (ABRdInfo_p->m_startAddress + (ABRdInfo_p->m_numOfEntries*ABRdInfo_p->m_width))
                                                                    > NLMDEV_AB_DEPTH)
        return NLMDIAG_ERROR_INVALID_INPUT;


    abWidth = (nlm_u16)NLMDIAG_GET_AB_WIDTH(ABRdInfo_p->m_width);
    readAddress = ABRdInfo_p->m_startAddress;

    for(loop_ntry=0; loop_ntry < ABRdInfo_p->m_numOfEntries; loop_ntry++)
    {       
        for(loop_width=0; loop_width < abWidth; loop_width++)
        {
            /* Invoke the device manager API to read the specified AB entry */
            if((errNum = NlmDevMgr__ABEntryRead(dev_p, ABRdInfo_p->m_abNum,
                readAddress, &readEntry, &reasonCode))
                != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* if the both read and actual record entry valid- bit is set then compare these records */
            if(NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABRdInfo_p->m_abNum].m_abEntry[readAddress].m_vbit == 1
                && readEntry.m_vbit == 1)    
            {
                /* Compare the read data with the shadow chip memory AB data if both 
                   are same return success, else print the read and the expected data entry. */
                if((errNum = NlmDiag_CompareABData(ABRdInfo_p->m_abNum, 
                    readAddress, NLMDIAG_READ_FLAG, 
                    NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABRdInfo_p->m_abNum].m_abEntry[readAddress].m_data, 
                    NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABRdInfo_p->m_abNum].m_abEntry[readAddress].m_mask, 
                    readEntry.m_data, readEntry.m_mask, logFile)) != NLMDIAG_ERROR_NOERROR)
                    return(errNum);
            }
            else
            {
                /* if both read and actual record entry valid- bit is 0, then that is invalid entry
                   in the database */
                if(NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABRdInfo_p->m_abNum].m_abEntry[readAddress].m_vbit != 1
                    && readEntry.m_vbit != 1)
                {
                    NlmCmFile__fprintf(logFile, "Entry Invalid ( Exp: Invalid )");
                }
                else
                {
                    if(NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABRdInfo_p->m_abNum].m_abEntry[readAddress].m_vbit != 1)
                    {
                        NlmCmFile__fprintf(logFile, "\n\tError: Entry Valid ( Exp: Invalid )");
                        return(NLMDIAG_ERROR_VERIFY_FAIL);
                    }
                    else
                    {
                        NlmCmFile__fprintf(logFile, "\n\tError: Entry Invalid ( Exp: Valid )");
                        return(NLMDIAG_ERROR_VERIFY_FAIL);
                    }
                }
            }
            readAddress++;
        }        
        
    }
    return NLMDIAG_ERROR_NOERROR;
}


/*
    This function will print the record enrty being added if the flag NLMDIAG_WRITE_FLAG
    is set, else it will compare the both actual and read data entry and returns the result.
    Currently NLMDIAG_WRITE_FLAG is ignored by this function.
 */
nlm_u32 
NlmDiag_CompareABData(nlm_u8 ab_num, 
                               nlm_u16 addr, 
                               NlmDiag_WriteReadFlag rdWrFlag,
                               nlm_u8 *dataWord,
                               nlm_u8 *dataMask,
                               nlm_u8 *read_X,
                               nlm_u8 *read_Y, 
                               NlmCmFile *logFile
                               )
{
    nlm_u8 index;
    nlm_u8 write_X[NLMDEV_REG_LEN_IN_BYTES];
    nlm_u8 write_Y[NLMDEV_REG_LEN_IN_BYTES];  
    ab_num = ab_num;
    addr = addr;
    
    if(rdWrFlag == NLMDIAG_READ_FLAG)
    {    
        /* Generate X-Y record format from D-M format present in shadow chip */
        for(index = 0; index < NLMDEV_REG_LEN_IN_BYTES; index++)
        {
            write_X[index] = dataWord[index] & (~dataMask[index]);
            write_Y[index] = (~dataWord[index]) & (~dataMask[index]);  
        }
        
        /* Compare the read data and shadow chip memory AB data */
        if(((memcmp(write_X, read_X, NLMDEV_REG_LEN_IN_BYTES)) != 0) 
            || ((memcmp(write_Y, read_Y, NLMDEV_REG_LEN_IN_BYTES)) != 0))
        {
            /* If compare fails display read data and expected data */
            NlmCmFile__fprintf(logFile, "\t --> Read Mismatch :");
            NlmCmFile__fprintf(logFile, "  DataWord Read : (");
            NlmDiag_CommonPrintData(logFile,read_X, 0);
			NlmCmFile__fprintf(logFile, "), DataWord Written : ");
			NlmDiag_CommonPrintData(logFile,write_X, 0);
            
            NlmCmFile__fprintf(logFile, "\n\t\t                 DataMask Read : (");
            NlmDiag_CommonPrintData(logFile,read_Y, 0);
			NlmCmFile__fprintf(logFile, "), DataMask Written : ");
			NlmDiag_CommonPrintData(logFile,write_Y, 0);
                       
            return NLMDIAG_ERROR_VERIFY_FAIL;           
        }
    }   
    return NLMDIAG_ERROR_NOERROR;           
}


/* 
    This function invalidate (reset the valid-bit of record entry) the record
    at the specified address location in the AB block or database.
 */
nlm_u32 
NlmDiag_InvalidateABEntry(NlmDev *dev_p,
                            NlmDiag_ABInvalidateInfo *ABInvalidateInfo_p,
                            NlmCmFile *logFile
                            )	       
{
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
    NlmDevABEntry readEntry;
    nlm_u16 abWidth, delAddress, loop1, loop2;
      
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || 
                    ABInvalidateInfo_p == NULL || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(ABInvalidateInfo_p->m_abNum >= NLMDEV_NUM_ARRAY_BLOCKS 
        || ABInvalidateInfo_p->m_startAddress >= NLMDEV_AB_DEPTH 
        || ABInvalidateInfo_p->m_numOfEntries == 0 
        || ABInvalidateInfo_p->m_width > NLMDEV_BLK_WIDTH_640 
        || (ABInvalidateInfo_p->m_startAddress + 
            (ABInvalidateInfo_p->m_numOfEntries*ABInvalidateInfo_p->m_width))
                > NLMDEV_AB_DEPTH)
        return NLMDIAG_ERROR_INVALID_INPUT;
    
    abWidth = (nlm_u16)NLMDIAG_GET_AB_WIDTH(ABInvalidateInfo_p->m_width);
    delAddress = ABInvalidateInfo_p->m_startAddress; 
    for(loop1=0; loop1<ABInvalidateInfo_p->m_numOfEntries; loop1++)
    {                  
        for(loop2=0; loop2<abWidth; loop2++)
        {
            /* Invoke device manager API to Invalidate Specified AB entry */
            if((errNum = NlmDevMgr__ABEntryInvalidate(dev_p, ABInvalidateInfo_p->m_abNum,
                delAddress, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                 NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryInvalidate",
                     reasonCode);       
                 return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* Get the shadow chip memory for specified AB entry and mark it as invalid
               .i.e. reset the valid-bit */
            NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[ABInvalidateInfo_p->m_abNum].m_abEntry[delAddress].m_vbit = 0;
           
                       
             /* Verify of deleted (invalidated) Entry */ 
            /* Invoke the device manager API to read the specified invalidated entry */
            if((errNum = NlmDevMgr__ABEntryRead(dev_p, ABInvalidateInfo_p->m_abNum,
                delAddress, &readEntry, &reasonCode)) 
                != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* If compare fails display read data and expected data */             
            /* If the resulting record valid-bit is reset then the record is deleted (invalidated) */
            if(readEntry.m_vbit != 0)
            {
                NlmCmFile__fprintf(logFile, "\nErr: Entry Valid(Exp: Invalid)");
                return NLMDIAG_ERROR_VERIFY_FAIL;      
            }
            delAddress++;
        }        
    }
    return NLMDIAG_ERROR_NOERROR;
}


/* ST write read test */
NlmErrNum_t NlmDiag_STReadWriteTest(NlmDiag_TestInfo *testInfo_p) 
{  
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_STWrRdInfo stWrRdInfo;
    NlmErrNum_t errNum;       
    nlm_u8 ptrn;    
    void *anyInfo_p=NULL;
    nlm_u32 anyFlag=0;
    nlm_u8 stNum;
    nlm_u8 chnlNum = 0;
    nlm_u8 devNum;

    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;   

    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)                   
            return(errNum);   
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
         
    /* Initialize the structure used to test the read and write to st Entries */
    stWrRdInfo.anyInfo_p = anyInfo_p;
    stWrRdInfo.m_flag = anyFlag;

    /* Seed is provided as an parameter with the test case */
    stWrRdInfo.m_seed_p = &testInfo_p->m_testParams[0];     

    /* Carry the test for different patterns and ST */ 
    for(ptrn = NLMDIAG_PATTERN_ALL_FS; ptrn < NLMDIAG_PATTERN_END; ptrn++)
    {
		NlmCm__printf("\n\n\n\t %d] Data pattern [%d]\t\t", ptrn, ptrn);

        stWrRdInfo.m_pattern = ptrn;  
		NlmCm__printf("\n\t     Writing to ST ");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t     Writing to ST ");
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);

            for(stNum = 0; stNum < NLMDEV_SS_NUM; stNum++)
            {
                stWrRdInfo.m_stNum = stNum;             
                stWrRdInfo.m_startAddress = 0;          
                stWrRdInfo.m_numOfEntries = NLMDEV_SS_SEP;           

                /* Invoke the test fn which will performs st Reg writes */
                if((errNum = NlmDiag_WriteToST(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
                    &stWrRdInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)      
                return(errNum);                   
            }
        }

		NlmCm__printf("\n\t     Read from ST and Compare data");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t     \n\t     Read from ST and Compare data");
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {        
            for(stNum = 0; stNum < NLMDEV_SS_NUM; stNum++)
            {
                stWrRdInfo.m_stNum = stNum;      
                stWrRdInfo.m_startAddress = 0;          
                stWrRdInfo.m_numOfEntries = NLMDEV_SS_SEP;           

                /* Invoke the test fn which will performs st Reg Reads */
                if((errNum = NlmDiag_ReadFromSTandCompare(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
                    &stWrRdInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)     
                return(errNum);                   
            }
        }    
    }    
    
    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p)) != NLMDIAG_ERROR_NOERROR)
        return(errNum);  

    return NLMDIAG_ERROR_NOERROR;   
}


/* 
    ST Write and read functions 
 */

nlm_u32 
NlmDiag_WriteToST(NlmDev *dev_p,
                      NlmDiag_STWrRdInfo *stWrInfo_p,
                      NlmCmFile *logFile
                      )
{
    nlm_u8 generatedData[(NLMDEV_REG_LEN_IN_BYTES) * 2]; 
    NlmDevSTE *stRegInfo_p = NULL;
    nlm_u16 i;   
    NlmErrNum_t errNum;
	NlmReasonCode reasonCode;    
    
     /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || stWrInfo_p == NULL
        || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(stWrInfo_p->m_numOfEntries == 0 || stWrInfo_p->m_stNum >= NLMDEV_SS_NUM 
        || stWrInfo_p->m_startAddress >= NLMDEV_SS_SEP 
        || (stWrInfo_p->m_numOfEntries + stWrInfo_p->m_startAddress)> NLMDEV_SS_SEP)
         return NLMDIAG_ERROR_INVALID_INPUT;
    
    for(i=0;i<stWrInfo_p->m_numOfEntries;i++)
    {
        /* Get the shadow chip memory */
        stRegInfo_p = &(NLMDIAG_SHADOW_DEV_PTR)->m_st[stWrInfo_p->m_stNum].m_row[stWrInfo_p->m_startAddress+i];

        /* Generate the required data pattern */
        if((errNum = NlmDiag_CommonGenDataPattern(stWrInfo_p->m_pattern, 
            generatedData, 18, stWrInfo_p->m_seed_p, stWrInfo_p->m_flag,
            stWrInfo_p->anyInfo_p, logFile))!= NLMDIAG_ERROR_NOERROR)
        return(errNum);

        /* Fill the st Structure with generated data */
        stRegInfo_p->m_slr = ((generatedData[1] & 1)<<8) | generatedData[0];
        stRegInfo_p->m_ssi_s = ((generatedData[3] & 1)<<8) | generatedData[2];
        stRegInfo_p->m_ssi  = generatedData[4];
        stRegInfo_p->m_ss_abs  = generatedData[5] & 0x3F;
        stRegInfo_p->m_abc   = generatedData[6] & 0x7;
        stRegInfo_p->m_ss_bsel = generatedData[7] & 0x7;
        stRegInfo_p->m_bix   = (generatedData[10] & 0x7F) | generatedData[9] | generatedData[8];
        stRegInfo_p->m_bw = generatedData[11] & 0x3;
        stRegInfo_p->m_lprv = generatedData[12] & 0x1;
        stRegInfo_p->m_lpri = (generatedData[15] & 0x7F) | generatedData[14] | generatedData[13];	
        
		if(stWrInfo_p->m_stNum == 0 && i == 0x0c)
			continue;

        /* Invoke device manager API to write ST entry */
        if((errNum = NlmDevMgr__STW(dev_p,
            stWrInfo_p->m_stNum, stWrInfo_p->m_startAddress+i, 
            stRegInfo_p, NLMDEV_ST_WRFU, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__STW",reasonCode);
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
    }
    return NLMDIAG_ERROR_NOERROR;
}


nlm_u32 
NlmDiag_ReadFromSTandCompare(NlmDev *dev_p,
							 NlmDiag_STWrRdInfo *stRdInfo_p,
							 NlmCmFile *logFile
							 )
{
    NlmDevSTE *stRegInfo_p = NULL;
    NlmDevSTE stRegReadInfo;
    nlm_u16 i;   
    NlmErrNum_t errNum;
	NlmReasonCode reasonCode;

    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || stRdInfo_p == NULL
        || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(stRdInfo_p->m_numOfEntries == 0 || stRdInfo_p->m_stNum >= NLMDEV_SS_NUM 
        || stRdInfo_p->m_startAddress >= NLMDEV_SS_SEP 
        || (stRdInfo_p->m_numOfEntries + stRdInfo_p->m_startAddress)> NLMDEV_SS_SEP)
         return NLMDIAG_ERROR_INVALID_INPUT;
    
    for(i=0;i<stRdInfo_p->m_numOfEntries;i++)
    {
        /* Get the shadow chip memory for comparing data read*/
        stRegInfo_p = &(NLMDIAG_SHADOW_DEV_PTR)->m_st[stRdInfo_p->m_stNum].
            m_row[stRdInfo_p->m_startAddress+i];
       
		if(stRdInfo_p->m_stNum == 0 && i == 0x0c)
			continue;

        /* Invoke device manager API to read specified st entry */
        if((errNum = NlmDevMgr__STR(dev_p, 
            stRdInfo_p->m_stNum, stRdInfo_p->m_startAddress+i, 
            &stRegReadInfo, &reasonCode)) !=   NLMDIAG_ERROR_NOERROR )
        {
            NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__STR",reasonCode);
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }      
                
        /* Compare data read with data in shadow chip memory */
	    if((stRegInfo_p->m_slr != stRegReadInfo.m_slr) ||
            (stRegInfo_p->m_ssi_s != stRegReadInfo.m_ssi_s) ||
            (stRegInfo_p->m_ssi != stRegReadInfo.m_ssi) ||
            (stRegInfo_p->m_ss_abs != stRegReadInfo.m_ss_abs) ||
            (stRegInfo_p->m_abc != stRegReadInfo.m_abc) ||
            (stRegInfo_p->m_ss_bsel != stRegReadInfo.m_ss_bsel) ||
            (stRegInfo_p->m_bix != stRegReadInfo.m_bix) ||
            (stRegInfo_p->m_bw != stRegReadInfo.m_bw) ||
            (stRegInfo_p->m_lprv != stRegReadInfo.m_lprv) ||
            (stRegInfo_p->m_lpri != stRegReadInfo.m_lpri) 
            )
       {
            /* If Compare Fails display read data and expected data */
            NlmCmFile__fprintf(logFile, "\n\t Compare of ST Read Data Fail");
            NlmCmFile__fprintf(logFile, "\n\t Data Read");
            NlmCmFile__fprintf(logFile, "\n\t SLR = %03d(%03d), ISP = %03d(%03d), SID = %03d(%03d)",
                stRegInfo_p->m_slr, stRegReadInfo.m_slr, stRegInfo_p->m_ssi_s,
                stRegReadInfo.m_ssi_s, stRegInfo_p->m_ssi, stRegReadInfo.m_ssi);
            NlmCmFile__fprintf(logFile, "\n\t ABS = %02d(%02d), ABC = %d(%d), BSel = %d(%d)",
                stRegInfo_p->m_ss_abs, stRegReadInfo.m_ss_abs, stRegInfo_p->m_abc, 
                stRegReadInfo.m_abc, stRegInfo_p->m_ss_bsel, stRegReadInfo.m_ss_abs);
            NlmCmFile__fprintf(logFile, "\n\t BIdx = 0x%06x(0x%06x), BW = %02d(%02d)", stRegInfo_p->m_bix,
                stRegReadInfo.m_bix, (nlm_u8)stRegInfo_p->m_bw, (nlm_u8)stRegReadInfo.m_bw);
            NlmCmFile__fprintf(logFile, "\n\t LPVd = %d(%d), LPId = 0x%06x(0x%06x)", 
                stRegInfo_p->m_lprv, stRegReadInfo.m_lprv, 
                stRegInfo_p->m_lpri, stRegReadInfo.m_lpri);
            return(NLMDIAG_ERROR_VERIFY_FAIL);
        }        
    }

    return NLMDIAG_ERROR_NOERROR;
}
/*  11  */








