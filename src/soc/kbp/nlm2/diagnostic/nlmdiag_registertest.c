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
 
 #include "nlmdiag_registertest.h"
 
/* ========================================================================
                     Register function declarations 
    ======================================================================== */
static nlm_u32 NlmDiag_InitialRead_AllRegistersTest(
     NlmDiag_TestInfo              *testInfo_p
     );
 
 static nlm_u32 NlmDiag_RangeRegister_ReadWriteTest(
     NlmDiag_TestInfo              *testInfo_p
     );
 
 static nlm_u32 NlmDiag_BlockRegister_ReadWriteTest(
     NlmDiag_TestInfo              *testInfo_p
     );
 
 static nlm_u32 NlmDiag_BMRRegister_ReadWriteTest(
     NlmDiag_TestInfo              *testInfo_p
     );
 
 static nlm_u32 NlmDiag_LTRRegister_ReadWriteTest(
     NlmDiag_TestInfo             *testInfo_p
     );
 
 static nlm_u32 NlmDiag_CBReadWriteTest(
     NlmDiag_TestInfo              *testInfo_p
     );
 
 static nlm_u32 NlmDiag_GlobalRegister_ReadWriteTest(
     NlmDiag_TestInfo              *testInfo_p
     );
 
 static nlm_u32 NlmDiag_RegisterReadWriteTest(
     NlmDiag_TestInfo              *testInfo_p
     );
 
   /* all register write and read function declarations goes here */
 static nlm_u32 NlmDiag_WriteToRangeReg(
     NlmDev                        *dev_p,
     NlmDiag_RangeRegWrRdInfo      *rangeRegWrInfo_p,
     NlmDiag_RangeRegsInfo         *rangeRegs,
     NlmCmFile                        *logFile
     );
 
 static nlm_u32 NlmDiag_ReadFromRangeRegAndCompare(
     NlmDev                        *dev_p,
     NlmDiag_RangeRegWrRdInfo      *rangeRegWrInfo_p,
     NlmDiag_RangeRegsInfo         *rangeRegs,
     NlmCmFile                        *logFile
     );

 
 /* ========================================================================
                   Register write/read function declarations 
    ======================================================================== */
 static nlm_u32  NlmDiag_WriteToBlockOrBMRRegister(
     NlmDev                        *dev_p,
     NlmDiag_BlkRegWrRdInfo        *blkRegWrInfo_p,
     NlmCmFile                        *logFile
     );
 
 
 static nlm_u32  NlmDiag_ReadFromBlockOrBMRRegisterAndCompare(
     NlmDev                        *dev_p,
     NlmDiag_BlkRegWrRdInfo        *blkRegRdInfo_p,
     NlmCmFile                        *logFile
     );
 
 /* LTR register write/read function declarations */
 static nlm_u32 NlmDiag_WriteToLtrRegister(
     NlmDev                        *dev_p,
     NlmDiag_LTRRegWrRdInfo        *ltrRegWrInfo_p,
     NlmCmFile                        *logFile
     );
 
 static nlm_u32 NlmDiag_ReadFromLtrRegisterAndCompare(
     NlmDev                        *dev_p,
     NlmDiag_LTRRegWrRdInfo        *ltrRegRdInfo_p,
     NlmCmFile                        *logFile
     );
 
 /* Context Buffer register write/read function declarations */
 static nlm_u32  NlmDiag_WriteToCBRegister(
     NlmDev                        *dev_p,
     NlmDiag_CBWrRdInfo            *CBWrInfo_p,
     NlmCmFile                        *logFile,
	 nlm_u8							  *CBData_p
     );
 
 static nlm_u32  NlmDiag_ReadFromCBRegisterAndCompare(
     NlmDev                        *dev_p,
     NlmDiag_CBWrRdInfo            *CBRdInfo_p,
     NlmCmFile                        *logFile,
	 nlm_u8						      *CBData_p
     );
 
 /* Global register write/read function declarations */
 static nlm_u32 NlmDiag_WriteToGlobalRegister(
     NlmDev                        *dev_p,
     NlmDiag_GlobalRegWrRdInfo     *globalRegRdInfo_p,
     NlmDiag_GlobalRegInfo         *globalRegs_p,
     NlmCmFile                        *logFile
     );
 
 static nlm_u32 NlmDiag_ReadFromGlobalRegisterAndCompare(
     NlmDev                        *dev_p,
     NlmDiag_GlobalRegWrRdInfo     *globalRegRdInfo_p,
     NlmDiag_GlobalRegInfo         *globalRegs_p,
     NlmCmFile                        *logFile
     );

/*==================================================================================
                        Register read and write functions
==================================================================================*/

/* 
    The function which calls all the register read only / write and read tests of various
    device registers like Global, BCR, BMR, LTR, AB, CB registers.
 */
nlm_u32 
NlmDiag_RegisterReadWriteTests(void *alloc_p, 
                                 NlmDiag_TestInfo *testInfo_p,
                                 NlmDiag_TestCaseInfo *testCaseInfo_p
                                 )
{   
    nlm_u32 errNum;
  
    /* Check for the input parameters */
    if(alloc_p == NULL || testCaseInfo_p == NULL || testCaseInfo_p->m_testParams == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
    
    /*create test information for each test type specified */
    if((errNum = NlmDiag_CommonInitTestInfo(alloc_p, testInfo_p, testCaseInfo_p)) !=0)
    {
        testCaseInfo_p->m_errCount = 1;
        NlmCm__printf("\n\n\n\tDiagnostic Register test is Failed (Memory Error).\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tDiagnostic Register test is Failed (Memory Error).\n");
        return (errNum); /* return if Init() fails */
    }
    
    NlmCm__printf("\n\n\tDevice Register write and read tests. "); 
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\tDevice Register write and read tests."); 
    
    NlmCm__printf("\n\t   1. Read device registers and comapre default values.");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t   1. Read device registers and comapre default values.");
    if((errNum = NlmDiag_InitialRead_AllRegistersTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->Register read test FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Register Read (check default values) : FAILED\n");
        testCaseInfo_p->m_errCount = 1;		
    }

    NlmCm__printf("\n\n\t   2. Global register read/write tests.");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t   2. Global register read/write tests.");
    if((errNum = NlmDiag_GlobalRegister_ReadWriteTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->Global register test FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Global register test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
	else
	{
		NlmCm__printf("\n\n\t\t ->Global register read/write tests completed.");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t\t ->Global register read/write tests completed.");
	}
	
	NlmCm__printf("\n\n\t   2a. Generic read/write registers tests (Global/LTR 80b)");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t   2a. Generic read/write registers tests (Global/LTR 80b).");
    if((errNum = NlmDiag_RegisterReadWriteTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->Generic read/write registers tests (Global/LTR 80b) FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Generic read/write registers tests (Global/LTR 80b) FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
	else
	{
		NlmCm__printf("\n\n\t\t ->Generic read/write registers (Global/LTR 80b) tests completed.");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t\t ->Generic read/write registers (Global/LTR 80b) tests completed.");
	}

	NlmCm__printf("\n\n\t   3. Block register write/read tests.");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t   3. Block register write/read tests.");
    if((errNum = NlmDiag_BlockRegister_ReadWriteTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->BCR register test FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t BCR register test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
	else
	{
		NlmCm__printf("\n\n\t\t ->Block register write/read tests completed.");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t\t ->Block register write/read tests completed.");
	}

    NlmCm__printf("\n\n\t   4. BMR register write/read tests.");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t   4. BMR register write/read tests.");
    if((errNum = NlmDiag_BMRRegister_ReadWriteTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->BMR register test FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t BMR register test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
	else
	{
		NlmCm__printf("\n\n\t\t ->BMR register write/read tests completed.");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t\t ->BMR register write/read tests completed.");
	}

    NlmCm__printf("\n\n\t   5. LTR register write/read tests.");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t   5. LTR register write/read tests.");
    if((errNum = NlmDiag_LTRRegister_ReadWriteTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->LTR register test FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t LTR register test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
	else
	{
		NlmCm__printf("\n\n\t\t ->LTR register write/read completed.");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile,"\n\n\t\t ->LTR register write/read completed.");
	}

    NlmCm__printf("\n\n\t   6. Context Buffer register write/read tests.");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t   6. Context Buffer register write/read tests.");
    if((errNum = NlmDiag_CBReadWriteTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->CB register test FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t CB register test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
	else
	{
		NlmCm__printf("\n\n\t\t ->Context Buffer register write/read tests completed.");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile,"\n\n\t\t ->Context Buffer register write/read tests completed.");
	}

    NlmCm__printf("\n\n\t   7. Range register write/read tests");
    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t   7. Range register write/read tests");
    if((errNum = NlmDiag_RangeRegister_ReadWriteTest(testInfo_p)) != 0)
    {
        /* print the error description and increment the error count*/
		NlmCm__printf("\n\n\t\t ->Range register test FAILED\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Range register test FAILED\n");
        testCaseInfo_p->m_errCount = 1;
    }
	else
	{
		NlmCm__printf("\n\n\t\t ->Range register write/read tests completed.");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t\t ->Range register write/read tests completed.");
	}
      
    if((errNum + testCaseInfo_p->m_errCount) == 0)
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tDiagnostic Register tests Successfully completed.\n");
        NlmCm__printf("\n\n\tDiagnostic Register tests Successfully completed.\n");
    }
    else
    {
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\tOne or more Diagnostic Register tests are Failed.\n");
        NlmCm__printf("\n\n\tOne or more Diagnostic Register tests are Failed.\n");
    }

    /* Release testblkinfo after each test case */
    if((errNum = NlmDiag_CommonDestroyTestInfo(testInfo_p)) != 0)
    {
        NlmCm__printf("\n\tCon't destroy Diagnostic test information.\n");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCon't destroy Diagnostic test information.\n");
        return (errNum);
    } 
    
    return (errNum + testCaseInfo_p->m_errCount);    
}



/* 
    This test basically reads all registers without writing to it, called as init 
    register test. and then it compares the read data with the expected (default)
    data, if match returns zero else returns with error code.
 */

nlm_u32 
NlmDiag_InitialRead_AllRegistersTest(NlmDiag_TestInfo *testInfo_p
                                       )
{
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    nlm_u8 readReg_defaultData[10];
    NlmReasonCode reasonCode =0;

    /* Global register structures */
    NlmDevIdReg devIdRegReadInfo;
    NlmDevErrStatusReg devErrStatRegReadInfo; 
    NlmDevAdvancedSoftErrReg devAdvSoftErrRegReadInfo;
    NlmDevResultReg devRegResult;

    /* Write/Read regs */
    NlmDevConfigReg devConfigRegReadInfo;
    NlmDevErrStatusReg devErrStatMaskRegReadInfo; /* same structure used to mask the errors */
    NlmDevDbSoftErrFifoReg devDBSoftErrStatRegReadInfo;
    NlmDevScratchPadReg scratchPadRegReadInfo;     /* To read shadow memory data */

    /* Block Config reg structure */
    NlmDevBlockConfigReg blkConfigRegReadInfo;
    NlmDevBlockMaskReg blkMaskRegReadInfo;
    
    /* LTR register stucture */
    NlmDevBlkSelectReg       ltrBlkSelRegInfoRd;
    NlmDevParallelSrchReg	 ltrPrlSrchRegInfoRd;
    NlmDevKeyConstructReg    ltrKeyConstructRegInfoRd;
    NlmDevSuperBlkKeyMapReg  ltrBlkKeyMapRegInfoRd;
    NlmDevRangeInsertion0Reg ltrRangeInst0_RegInfoRd;
    NlmDevRangeInsertion1Reg ltrRangeInst1_RegInfoRd;
    NlmDevMiscelleneousReg   ltrMiscRegInfoRd;
    NlmDevSSReg              ltrSSRegReadInfoRd;

    /* CB register structure */
    NlmDevCtxBufferReg CBRegReadInfo;

    nlm_u32 errNum=0;
    nlm_u32 regNum=0;
    nlm_u16 readAddress=0, numEntries=0;
    nlm_u8 abNum=0, devNum=0, flag=1;
    nlm_u8 profile=0, blkNum=0, chnlNum=0;

	NlmCm__memset(&devIdRegReadInfo, 0, sizeof(NlmDevIdReg));
	NlmCm__memset(&devErrStatRegReadInfo, 0, sizeof(NlmDevErrStatusReg));
	NlmCm__memset(&devAdvSoftErrRegReadInfo, 0, sizeof(NlmDevAdvancedSoftErrReg));
	NlmCm__memset(&devRegResult, 0, sizeof(NlmDevResultReg));

	NlmCm__memset(&devConfigRegReadInfo, 0, sizeof(NlmDevConfigReg));
	NlmCm__memset(&devErrStatMaskRegReadInfo, 0, sizeof(NlmDevErrStatusReg));
	NlmCm__memset(&devDBSoftErrStatRegReadInfo, 0, sizeof(NlmDevDbSoftErrFifoReg));
	NlmCm__memset(&scratchPadRegReadInfo, 0, sizeof(NlmDevScratchPadReg));

	NlmCm__memset(&blkConfigRegReadInfo, 0, sizeof(NlmDevBlockConfigReg));
	NlmCm__memset(&blkMaskRegReadInfo, 0, sizeof(NlmDevBlockMaskReg));

	NlmCm__memset(&ltrBlkSelRegInfoRd, 0, sizeof(NlmDevBlkSelectReg));
	NlmCm__memset(&ltrPrlSrchRegInfoRd, 0, sizeof(NlmDevParallelSrchReg));
	NlmCm__memset(&ltrKeyConstructRegInfoRd, 0, sizeof(NlmDevKeyConstructReg));
	NlmCm__memset(&ltrBlkKeyMapRegInfoRd, 0, sizeof(NlmDevSuperBlkKeyMapReg));
	NlmCm__memset(&ltrRangeInst0_RegInfoRd, 0, sizeof(NlmDevRangeInsertion0Reg));
	NlmCm__memset(&ltrRangeInst1_RegInfoRd, 0, sizeof(NlmDevRangeInsertion1Reg));
	NlmCm__memset(&ltrMiscRegInfoRd, 0, sizeof(NlmDevMiscelleneousReg));
	NlmCm__memset(&ltrSSRegReadInfoRd, 0, sizeof(NlmDevSSReg));

	NlmCm__memset(&CBRegReadInfo, 0, sizeof(NlmDevCtxBufferReg));

    /* Check for input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || 
        testInfo_p->m_testLogFile == NULL || testInfo_p->m_testParams == NULL || 
            testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;   
    
    /* Initialize the device */
    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)                   
        return(errNum);   

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;    

    /* check for the device pointers */
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
       
    for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
    {
        if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
            return(NLMDIAG_ERROR_INVALID_INPUTPTR);
        
        NlmCm__printf("\n\n\t\t Device id : %d\n\n", devNum);
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\n\t\t Device id : %d\n\n", devNum);
        /* --------------------------------------------------------------------------------------=----------
                    Read all the Global registers, and the default value read from the 
                    device or C-model must be zero
           ------------------------------------------------------------------------------------------------- */
        /* <1> DEVICE ID : */
        /* call device manager API to read Device-ID Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
                NLMDEV_DEVICE_ID_REG, &devIdRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            if (devIdRegReadInfo.m_databaseSize == NLMDIAG_DEV_DB_SIZE &&
                devIdRegReadInfo.m_majorDieRev == NLMDIAG_DEV_MAJOR_REV &&
                devIdRegReadInfo.m_minorDieRev == NLMDIAG_DEV_MINOR_REV)
            {
                NlmCm__printf("\n\t\t -> Device Id read correctly.");
            }
            else
            {
                NlmCm__printf("\n\t -> Device Id read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE ID read in-correctly... MISMATCH !\n");
                flag = 0; /* reset flag to indicate the mismatch occured */
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init : Device Id Register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

        /* <2> DEVICE CONFIGURATION : */
        /* call device manager API to Device-configuration Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLMDEV_DEVICE_CONFIG_REG, 
                        &devConfigRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            if( devConfigRegReadInfo.m_dbParityErrEntryInvalidate ==
                                                  (Nlm11kDevDisableEnable)NLMDEV_ENABLE &&
                devConfigRegReadInfo.m_dbSoftErrProtectMode ==
                                                  (Nlm11kDevDisableEnable)NLMDEV_DISABLE &&
                devConfigRegReadInfo.m_eccScanType == (Nlm11kDevDisableEnable)NLMDEV_DISABLE &&
                devConfigRegReadInfo.m_lowPowerModeEnable == (Nlm11kDevDisableEnable)NLMDEV_DISABLE &&
                devConfigRegReadInfo.m_rangeEngineEnable == (Nlm11kDevDisableEnable)NLMDEV_DISABLE &&
                devConfigRegReadInfo.m_softErrorScanEnable == (Nlm11kDevDisableEnable)NLMDEV_DISABLE )
            {
                NlmCm__printf("\n\t\t -> Device Configuration Register default value read correctly");
            }
            else
            {
                NlmCm__printf("\n\t -> Device Configuration Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE CONFIGURATION Register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t Init : Device Configutarion Register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

        /* <3-4> DEVICE ERROR STATUS registers */
        
        /* call device manager API to read Error_Status Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLM_REG_ADDR_ERROR_STATUS, 
                                &devErrStatRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
                if(
                    (devErrStatRegReadInfo.m_ctxBufferParityErr != NlmFalse) ||
                    (devErrStatRegReadInfo.m_dbSoftError != NlmFalse) ||
                    (devErrStatRegReadInfo.m_dbSoftErrorFifoFull != NlmFalse) ||
                    (devErrStatRegReadInfo.m_devIdMismatchErr != NlmFalse) ||
                    /* (devErrStatRegReadInfo.m_globalGIO_L_Enable != NlmFalse) || */
                    (devErrStatRegReadInfo.m_illegalInstnErr != NlmFalse) ||
                    (devErrStatRegReadInfo.m_ltrParityErr != NlmFalse))
            {
                NlmCm__printf("\n\t -> Device Error_Status register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE ERROR_STATUS register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
            else
            {                
                NlmCm__printf("\n\t\t -> Device Error_Status Register default value read correctly");
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Error_Status register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
        /* call device manager API to read Error_StatusMask Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLM_REG_ADDR_ERROR_STATUS_MASK, 
                                &devErrStatMaskRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            if((devErrStatMaskRegReadInfo.m_alignmentErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_burstControlWordErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_burstMaxErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_channelNumErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_crc24Err != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_ctxBufferParityErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_dbSoftError != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_dbSoftErrorFifoFull != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_devIdMismatchErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_eopErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_framingCtrlWordErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_globalGIO_L0_Enable != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_globalGIO_L1_Enable != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_illegalInstnErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_instnBurstErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_ltrParityErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_missingDataPktErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_parityScanFifoOverFlow != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_powerLimitingErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_protocolErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_rxNMACFifoParityErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_rxPCSEFifoParityErr != NlmFalse) ||
            (devErrStatMaskRegReadInfo.m_sopErr != NlmFalse)) 
            {
                NlmCm__printf("\n\t -> Device Error_StatusMask Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE ERROR_STSTUSMASK register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
            else
            {
                NlmCm__printf("\n\t\t -> Device Error_StatusMask register default value read correctly");                
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Error_StatusMask register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

        /* call device manager API to read Error_StatusMask Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG, 
                                &devAdvSoftErrRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            if( (devAdvSoftErrRegReadInfo.m_cbParityErrAddr != 0x0)) 
            {
                NlmCm__printf("\n\t -> Device Error_StatusMask Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE ERROR_STSTUSMASK register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
            else
            {
                NlmCm__printf("\n\t\t -> Device Advance Feature soft register default value read correctly");                
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Error_StatusMask register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

        /* <5> DEVICE SOFT_ERROR register */ 

        /* call device manager API to read Soft_ErrorFIFO Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG, 
                                &devDBSoftErrStatRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {            
            if( (devDBSoftErrStatRegReadInfo.m_errorAddr == NlmFalse) ||
                (devDBSoftErrStatRegReadInfo.m_errorAddrValid == NlmFalse) ||
                (devDBSoftErrStatRegReadInfo.m_pErrorX == NlmFalse) ||
                (devDBSoftErrStatRegReadInfo.m_pErrorY == NlmFalse))
            {
                NlmCm__printf("\n\t\t -> Device Soft_ErrorFIFO Register default value read correctly");
            }
            else
            {
                NlmCm__printf("\n\t -> Device Soft_ErrorFIFO Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE SOFT_ERRORFIFO Register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Soft_ErrorFIFO register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
    
        /* <6-7> DEVICE SCRATCH PAD register : */

        /* call device manager API to read Scratch-pad0 Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLMDEV_SCRATCH_PAD0_REG, 
                                &scratchPadRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            NlmDevScratchPadReg scratchPadReg_default;
            NlmCm__memset(scratchPadReg_default.m_data, 0xAA, 10);

            if((NlmCm__memcmp(scratchPadRegReadInfo.m_data, 
                scratchPadReg_default.m_data, sizeof(NlmDevScratchPadReg))) == 0)
            {
                NlmCm__printf("\n\t\t -> Device Scratch Pad0 Register default value read correctly");
            }
            else
            {
                NlmCm__printf("\n\t -> Device Scratch Pad0 Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE SCRATCH PAD0 Register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Scratch Pad0 register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

        /* call device manager API to read Scratch-pad1 Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLMDEV_SCRATCH_PAD1_REG, 
                                &scratchPadRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            NlmDevScratchPadReg scratchPadReg_default;
            NlmCm__memset(scratchPadReg_default.m_data, 0x55, 10);

            if((NlmCm__memcmp(scratchPadRegReadInfo.m_data, 
                scratchPadReg_default.m_data, sizeof(NlmDevScratchPadReg))) == 0)
            {
                NlmCm__printf("\n\t\t -> Device Scratch Pad1 Register default value read correctly");
            }
            else
            {
                NlmCm__printf("\n\t -> Device Scratch Pad1 Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE SCRATCH PAD1 Register ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Scratch Pad1 Register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
        

        /* <8-9> DEVICE RESULT registers */

        /* call device manager API to read Result0 Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLMDEV_RESULT0_REG, 
                                &devRegResult, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            NlmDevResultReg devRegResult0_default = {{0},{0}};

            if((NlmCm__memcmp(&devRegResult, &devRegResult0_default, sizeof(NlmDevResultReg))) == 0)
            {
                NlmCm__printf("\n\t\t -> Device Result0 Register default value read correctly");
            }
            else
            {
                NlmCm__printf("\n\t -> Device Result1 Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE Result0 Register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Result0 Register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

        /* call device manager API to read Result1 Global register */
        if((errNum = NlmDevMgr__GlobalRegisterRead(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], NLMDEV_RESULT1_REG, 
                                &devRegResult, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
        {
            NlmDevResultReg devRegResult1_default = {{0},{0}};

            if((NlmCm__memcmp(&devRegResult, &devRegResult1_default, sizeof(NlmDevResultReg))) == 0)
            {
                NlmCm__printf("\n\t\t -> Device Result1 Register default value read correctly");
            }
            else
            {
                NlmCm__printf("\n\t -> Device Result1 Register read in-correctly .....\n");
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                                "\n\t initial : DEVICE RESULT1 Register read ... MISMATCH !\n");
                flag = 0 ; /* reset flag to indicate the mismatch occured */
            }
        }
        else
        {
            NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                "\n\t Init : Global Result1 Register read ()");
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

      
        /* --------------------------------------------------------------------------------------------
                    Read all Block Configuration registers, and the default value read 
                    from the device or C-model must be zero
           -------------------------------------------------------------------------------------------- */
        for(abNum = 0; abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
        { 
            /* call device manager API to read data from the BCR register specified block number */
            if((errNum = NlmDevMgr__BlockRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], abNum, 
                NLMDEV_BLOCK_CONFIG_REG, (void*)&blkConfigRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                if( blkConfigRegReadInfo.m_blockWidth != (Nlm11kDevBlockWidth)NLMDEV_BLK_WIDTH_80 &&
                    blkConfigRegReadInfo.m_blockEnable != (Nlm11kDevDisableEnable)NLMDEV_DISABLE)
                {
                    NlmCm__printf("\n\t -> Device Block configuration register read in-correctly .....\n");
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                    "\n\t initial : DEVICE BCR reg read ... MISMATCH !\n");
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile,
                                    "\n\t Init : Block Configuration register read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
        }
        NlmCm__printf("\n\t\t -> All Device Block Registers (BCRs) default read done...");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> All Device Block Registers (BCRs) default read done...");

        /* --------------------------------------------------------------------------------------------
                        Read all BMR registers, and the default value read from the 
                        device or C-model must be zero
           -------------------------------------------------------------------------------------------- */
        for(blkNum = NLMDEV_BLOCK_MASK_0_0_REG; blkNum < NLMDEV_BLOCK_REG_END; blkNum++)
        { 
            /* NlmDevBlockMaskReg blkMaskReg_default = {{0}}; */

            abNum = blkNum;
            /* call device manager API to read data from the BMR register specified */
            if((errNum = NlmDevMgr__BlockRegisterRead(
                    moduleInfo_p->m_dev_ppp[chnlNum][devNum], abNum, blkNum, 
                        (void*)&blkMaskRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                /* Values read from the BMR register are undetermined. .i.e. don't care values */                
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: Block mask register read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
        }

        NlmCm__printf("\n\t\t -> All Device Block Mask Registers (BMRs) default read done...");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> All Device Block Mask Registers (BMRs) default read done...");

        /* --------------------------------------------------------------------------------------------
                        Read all LTR registers, and the default value read from 
                        the device or C-model must be zero
           -------------------------------------------------------------------------------------------- */
        for(profile = 0; profile < NLMDEV_NUM_LTR_SET; profile++)
        {   
            NlmDevLtrRegType keyType;

            /* <1-2> Block Select registers */
            /* call device manager API to read data from the Block-select0 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_BLOCK_SELECT_0_LTR,
                (void*)&ltrBlkSelRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevBlkSelectReg ltrBlkSelRegInfoDefRd = {{NLMDEV_DISABLE}};

                if((NlmCm__memcmp(&ltrBlkSelRegInfoRd, 
                            &ltrBlkSelRegInfoDefRd, sizeof(NlmDevBlkSelectReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Blk sel 0) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Blk sel 0] read ... MISMATCH !\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Blk Sel 0) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            /* call device manager API to read data from the Block-select1 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_BLOCK_SELECT_1_LTR,
                (void*)&ltrBlkSelRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevBlkSelectReg ltrBlkSelRegInfoDefRd = {{NLMDEV_DISABLE}};

                if((NlmCm__memcmp(&ltrBlkSelRegInfoRd, 
                            &ltrBlkSelRegInfoDefRd, sizeof(NlmDevBlkSelectReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Blk sel 1) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Blk sel 1] read ... MISMATCH !\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Blk Sel 1) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* <3-6> Parallel search registers */
            /* call device manager API to read data from the parallel-search0 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_PARALLEL_SEARCH_0_LTR,
                (void*)&ltrPrlSrchRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevParallelSrchReg ltrPrlSrchRegInfoDefRd = {{0}};

                if((NlmCm__memcmp(&ltrPrlSrchRegInfoRd, 
                        &ltrPrlSrchRegInfoDefRd, sizeof(NlmDevParallelSrchReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Parll srch 0) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Parll srch 0] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Prll srch 0) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* call device manager API to read data from the parallel-search1 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_PARALLEL_SEARCH_1_LTR,
                (void*)&ltrPrlSrchRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevParallelSrchReg ltrPrlSrchRegInfoDefRd = {{0}};

                if((NlmCm__memcmp(&ltrPrlSrchRegInfoRd, 
                        &ltrPrlSrchRegInfoDefRd, sizeof(NlmDevParallelSrchReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Parll srch 1) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Parll srch 1] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Prll srch 1) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* call device manager API to read data from the parallel-search2 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_PARALLEL_SEARCH_2_LTR,
                (void*)&ltrPrlSrchRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevParallelSrchReg ltrPrlSrchRegInfoDefRd = {{0}};

                if((NlmCm__memcmp(&ltrPrlSrchRegInfoRd, 
                        &ltrPrlSrchRegInfoDefRd, sizeof(NlmDevParallelSrchReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Parll srch 2) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Parll srch 2] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Prll srch 2) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* call device manager API to read data from the parallel-search3 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_PARALLEL_SEARCH_3_LTR,
                (void*)&ltrPrlSrchRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevParallelSrchReg ltrPrlSrchRegInfoDefRd = {{0}};

                if((NlmCm__memcmp(&ltrPrlSrchRegInfoRd, 
                        &ltrPrlSrchRegInfoDefRd, sizeof(NlmDevParallelSrchReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Parll srch 3) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Parll srch 3] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Prll srch 3) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            /* <7-14> Key Construction register */
            /* call device manager API to read data from the key-construction LTR register */
            for (keyType = NLMDEV_KEY_0_KCR_0_LTR; keyType <= NLMDEV_KEY_3_KCR_1_LTR; keyType++)
            {
                if((errNum = NlmDevMgr__LogicalTableRegisterRead(
                    moduleInfo_p->m_dev_ppp[chnlNum][devNum], profile, keyType,
                    (void*)&ltrKeyConstructRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
                {
                    NlmDevKeyConstructReg ltrKeyConstructRegInfoDefRd = {{0,},{1,1,1,1,1}};

                    if((NlmCm__memcmp(&ltrKeyConstructRegInfoRd, 
                        &ltrKeyConstructRegInfoDefRd, sizeof(NlmDevKeyConstructReg))) != 0)
                    {
                        NlmCm__printf("\n\t -> LTR %d register (Key Constr %2d) read in-correctly .....\n", profile, keyType);
                        NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                            "\n\t initial : Profile : %2d, Key %2d LTR reg [Key Constr]  read ... MISMATCH!\n", profile, keyType);
                        flag = 0 ; /* reset flag to indicate the mismatch occured */
                    }
                }
                else
                {
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Key cnsrt) read ()");
                    return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
                }
            }
            /* <15> Miscelleneous register */
            /* call device manager API to read data from the Misc LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_MISCELLENEOUS_LTR,
                (void*)&ltrMiscRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevMiscelleneousReg ltrMiscRegInfoDefRd = {{0}};

                if((ltrMiscRegInfoDefRd.m_rangeAExtractStartByte != ltrMiscRegInfoRd.m_rangeAExtractStartByte) ||
                   (ltrMiscRegInfoDefRd.m_rangeBExtractStartByte != ltrMiscRegInfoRd.m_rangeAExtractStartByte) ||
                   (ltrMiscRegInfoDefRd.m_rangeCExtractStartByte != ltrMiscRegInfoRd.m_rangeCExtractStartByte) ||
                   (ltrMiscRegInfoDefRd.m_rangeDExtractStartByte != ltrMiscRegInfoRd.m_rangeDExtractStartByte) ||
                   (ltrMiscRegInfoDefRd.m_bmrSelect[0] != ltrMiscRegInfoRd.m_bmrSelect[0]) ||
                   (ltrMiscRegInfoDefRd.m_bmrSelect[1] != ltrMiscRegInfoRd.m_bmrSelect[1]) ||
                   (ltrMiscRegInfoDefRd.m_bmrSelect[2] != ltrMiscRegInfoRd.m_bmrSelect[2]) ||
                   (ltrMiscRegInfoDefRd.m_bmrSelect[3] != ltrMiscRegInfoRd.m_bmrSelect[3]) ||
                   (ltrMiscRegInfoDefRd.m_searchType[0] != ltrMiscRegInfoRd.m_searchType[0]) ||
                   (ltrMiscRegInfoDefRd.m_searchType[1] != ltrMiscRegInfoRd.m_searchType[1]) ||
                   (ltrMiscRegInfoDefRd.m_searchType[2] != ltrMiscRegInfoRd.m_searchType[2]) ||
                   (ltrMiscRegInfoDefRd.m_searchType[3] != ltrMiscRegInfoRd.m_searchType[3]))
                {
                    NlmCm__printf("\n\t -> LTR %d register (Misc) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Misc] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Misc) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* <16-17> Range insertion LTR 0/1 register */
            /* call device manager API to read data from the Range insrt 0 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_RANGE_INSERTION_0_LTR,
                (void*)&ltrRangeInst0_RegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevRangeInsertion0Reg ltrRangeInst0_RegInfoDefRd = {0};

                if((NlmCm__memcmp(&ltrRangeInst0_RegInfoRd, 
                        &ltrRangeInst0_RegInfoDefRd, sizeof(NlmDevRangeInsertion0Reg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Range insrt 0) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Range insrt 0] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Range insrt 0) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            /* call device manager API to read data from the Range insrt 1 LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_RANGE_INSERTION_1_LTR,
                (void*)&ltrRangeInst1_RegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevRangeInsertion1Reg ltrRangeInst1_RegInfoDefRd = {0};

                if((NlmCm__memcmp(&ltrRangeInst1_RegInfoRd, 
                        &ltrRangeInst1_RegInfoDefRd, sizeof(NlmDevRangeInsertion1Reg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Range insrt 1) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Range insrt 1] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Range insrt 1) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* <18> Super block key map LTR register */
            /* call device manager API to read data from the Super block key map LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_SUPER_BLK_KEY_MAP_LTR, 
                (void*)&ltrBlkKeyMapRegInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevSuperBlkKeyMapReg  ltrBlkKeyMapRegInfoDefRd = {{0}};

                if((NlmCm__memcmp(&ltrBlkKeyMapRegInfoRd, 
                        &ltrBlkKeyMapRegInfoDefRd, sizeof(NlmDevSuperBlkKeyMapReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (Super block key map) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [Super block key map] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Super block key map) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            /* <19> SS LTR register */
            /* call device manager API to read data from the SS LTR register */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                profile, NLMDEV_SS_LTR,
                (void*)&ltrSSRegReadInfoRd, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                NlmDevSSReg  ltrSSRegReadInfoDefRd = {{0}};

                if((NlmCm__memcmp(&ltrSSRegReadInfoRd, 
                        &ltrSSRegReadInfoDefRd, sizeof(NlmDevSSReg))) != 0)
                {
                    NlmCm__printf("\n\t -> LTR %d register (SS) read in-correctly .....\n", profile);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Profile : %2d, LTR reg [SS] read ... MISMATCH!\n",profile);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: LTR register (Super block key map) read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
        }

        /* If no errors from the above LTR register read operations, print this */
        NlmCm__printf("\n\t\t -> LTR register (Blk select 0 and 1) registers default read done...");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> LTR register (Blk select 0 and 1) registers default read done...");
        
        NlmCm__printf("\n\t\t -> LTR register (Parll srch 0 to 3) registers default read done...");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> LTR register (Parll srch 0 to 3) registers default read done...");
        
        NlmCm__printf("\n\t\t -> LTR register (Key Constror 8) register default read done..."); 
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> LTR register (Key Constror) register default read done...");
        
        NlmCm__printf("\n\t\t -> LTR register (Super blk key map) register default read done..."); 
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> LTR register (Super blk key map) register default read done...");
        
        NlmCm__printf("\n\t\t -> LTR register (Range Inst 0,1) register default read done..."); 
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> LTR register (Range Inst 0,1) register default read done...");
        
        NlmCm__printf("\n\t\t -> LTR register (Misc, SS) register default read done...");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> LTR register (Misc) register default read done...");            

        /* --------------------------------------------------------------------------------------------
                    Read all CB registers, and the default value read from the 
                    device or C-model must be zero
           -------------------------------------------------------------------------------------------- */
        numEntries = NLMDEV_CB_DEPTH;
        for(readAddress = 0; readAddress < numEntries; readAddress++)
        {
            /* call device manager API to read data from the CB register address specified */
            if((errNum = NlmDevMgr__CBAsRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], 
                                readAddress, &CBRegReadInfo, &reasonCode)) == NLMDIAG_ERROR_NOERROR)
            {
                /* Initial values read from the CB register are default values */

                if(errNum != 0)
                {
                    NlmCm__printf("\n\t ->Error: Initial  CB register @ %d read fail .....\n", readAddress);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\tError: Initial : CB reg read @ %d ... fail !\n", readAddress);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
            }
            else
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: CB register read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
        }

        NlmCm__printf("\n\t\t -> CB register default read done...");
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> CB register default read done...");
        /* --------------------------------------------------------------------------------------------
           Read all range registers, and the default value read from the device or C-model must be zero
           -------------------------------------------------------------------------------------------- */
        for(regNum = NLMDIAG_RANGE_START_ADDRESS; regNum <= NLMDIAG_RANGE_REGISTER_COUNT; regNum++)
        {     
            NlmDevRangeReg compRangeData;

            NlmCm__memset(compRangeData.m_data, 0, sizeof(NlmDevRangeReg));
            /* read the first 40 range register values */
            if((errNum = NlmDevMgr__RangeRegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], regNum, 
                                (NlmDevRangeReg*)readReg_defaultData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Init: Range register read ()");
                return(NLMDIAG_ERROR_INTERNAL_ERR(errNum));
            } 
            if((NlmCm__memcmp(&compRangeData.m_data[0], 
                    &readReg_defaultData[0], sizeof(NlmDevRangeReg))) != 0)
                {
                    NlmCm__printf("\n\t -> Range %.2x register read in-correctly .....\n", regNum);
                    NlmCmFile__fprintf(testInfo_p->m_testLogFile, 
                        "\n\t initial : Num : %2x, Range reg read ... MISMATCH!\n", regNum);
                    flag = 0 ; /* reset flag to indicate the mismatch occured */
                }
        }            
        NlmCm__printf("\n\t\t -> Range registers default read done..."); 
        NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t\t -> Range registers default read done..."); 

    }    

    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);  

    if(flag == 0)
    {
        NlmCm__printf("\n\t\t -> MIS-MATCH OCCURED.\n");
        return NLMDIAG_ERROR_VERIFY_FAIL;
    }

    return NLMDIAG_ERROR_NOERROR;     
}


/* 
    This function writes into the Range registers and reads back the contents,
    then the atcual written (golden) and the read data are compared, if both are
    same return zero else print the expected values and return error code.   
 */
nlm_u32 
NlmDiag_RangeRegister_ReadWriteTest(NlmDiag_TestInfo *testInfo_p)
{
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_RangeRegWrRdInfo rangeRegWrRdInfo;
    NlmDiag_RangeRegsInfo rangeRegs[4];
        
    void *anyInfo_p=NULL;
    nlm_u8 devNum, ptrn, chnlNum = 0;
    nlm_u32 errNum;   
    nlm_u32 regNum, anyFlag=0;
    
    /* Check for the input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
                    || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)
        return(errNum);

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
           
    /* Initialize the structure used to test the read and write to ltr registers */
    rangeRegWrRdInfo.anyInfo_p = anyInfo_p;
    rangeRegWrRdInfo.m_flag = anyFlag;

    /* Seed is provided as an parameter with the test case argument*/
    rangeRegWrRdInfo.m_seed_p = &testInfo_p->m_testParams[0];   

    /* Carry the test for different patterns, range registers  */ 
    for(ptrn = NLMDIAG_PATTERN_RANDOM; ptrn < NLMDIAG_PATTERN_END; ptrn++)
    {
        rangeRegWrRdInfo.m_pattern = ptrn;
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);
            
            /* Write the first 20 range registers, by calling the range register write API*/
            for(regNum = NLMDIAG_RANGE_START_ADDRESS; regNum <= NLMDIAG_RANGE_REGISTER_COUNT; regNum++)
            {
                rangeRegWrRdInfo.m_regNum = regNum;                                                
                if((errNum = NlmDiag_WriteToRangeReg(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                    &rangeRegWrRdInfo, &rangeRegs[devNum], testInfo_p->m_testLogFile)) !=NLMDIAG_ERROR_NOERROR)
                    return(errNum);                   
            }            
        }      
        
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {
            /* read the first 20 range registers, by calling the range register read API*/
            for(regNum = NLMDIAG_RANGE_START_ADDRESS; regNum <= NLMDIAG_RANGE_REGISTER_COUNT; regNum++)
            {
                rangeRegWrRdInfo.m_regNum = regNum;                                                
                if((errNum = NlmDiag_ReadFromRangeRegAndCompare(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                    &rangeRegWrRdInfo, &rangeRegs[devNum], testInfo_p->m_testLogFile)) !=NLMDIAG_ERROR_NOERROR)
                    return(errNum);  
            }            
        }
    }
     
   /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);  
    return NLMDIAG_ERROR_NOERROR;  
}


/* 
    This function writes into the LTR registers and reads back the contents,
    then the atcual written and the read data are compared, if both match 
    return zero else print the expected values and return error code.   
 */
nlm_u32 
NlmDiag_LTRRegister_ReadWriteTest(NlmDiag_TestInfo *testInfo_p)
{
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_LTRRegWrRdInfo ltrRegWrInfo_p, ltrRegRdInfo_p;
    
    nlm_u32 errNum;    
    void *anyInfo_p=NULL;
    nlm_u32 anyFlag=0;
    nlm_u8 devNum, ltr, pattern;
    nlm_u8 regNum=0, chnlNum = 0;
            
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
                    || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)
        return(errNum);

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
           
    /* Initialize the structure used to test the read and write to ltr registers */
    ltrRegWrInfo_p.anyInfo_p = anyInfo_p;
    ltrRegWrInfo_p.m_flag = anyFlag;

    /* Seed is provided as an parameter with the test case argument*/
    ltrRegWrInfo_p.m_seed_p = &testInfo_p->m_testParams[0];   

    /* carry out for all data patterns 
       use the NLMDIAG_PATTERN_ALL_0S as start pattern on device */
    for(pattern = NLMDIAG_PATTERN_RANDOM; pattern < NLMDIAG_PATTERN_END; pattern++)
    {
        ltrRegWrInfo_p.m_pattern = pattern;
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);
        
            for(ltr = 0; ltr < NLMDEV_NUM_LTR_SET; ltr++)
            {            
                ltrRegWrInfo_p.m_profileNum = ltrRegRdInfo_p.m_profileNum = ltr;
                for(regNum = 0; regNum < NLMDEV_LTR_REG_END; regNum++)
                {
					/*if(testInfo_p->m_operMode == NLMDEV_OPR_STANDARD && regNum == NLMDEV_SS_LTR)
						continue;*/

                    ltrRegWrInfo_p.m_ltrRegType = ltrRegRdInfo_p.m_ltrRegType = regNum;
                    
                    if((errNum = NlmDiag_WriteToLtrRegister(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                                &ltrRegWrInfo_p, testInfo_p->m_testLogFile)) !=NLMDIAG_ERROR_NOERROR)
                        return(errNum);  
                }
            }
        }
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);
        
            for(ltr = 0; ltr < NLMDEV_NUM_LTR_SET; ltr++)
            {            
                ltrRegWrInfo_p.m_profileNum = ltrRegRdInfo_p.m_profileNum = ltr;
                for(regNum = 0; regNum < NLMDEV_LTR_REG_END; regNum++)
                {     
					/*if(testInfo_p->m_operMode == NLMDEV_OPR_STANDARD && regNum == NLMDEV_SS_LTR)
						continue;*/

                    ltrRegWrInfo_p.m_ltrRegType = ltrRegRdInfo_p.m_ltrRegType = regNum;
                    if((errNum = NlmDiag_ReadFromLtrRegisterAndCompare(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
                                &ltrRegRdInfo_p, testInfo_p->m_testLogFile)) !=NLMDIAG_ERROR_NOERROR)
                        return(errNum);             
                }
            }   
        }
    }
     
   /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);  
    return NLMDIAG_ERROR_NOERROR;  
}


/* 
    This function writes into the CB registers and reads back the contents,
    then the atcual written and the read values are compared, if both are 
    same return zero else print the expected values and return error code.   
 */
nlm_u32 
NlmDiag_CBReadWriteTest(NlmDiag_TestInfo *testInfo_p) 
{  
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    NlmDiag_CBWrRdInfo CBWrRdInfo;
    nlm_u32 errNum; 
    void *anyInfo_p=NULL;
    nlm_u32 anyFlag=0;
    nlm_u8 pattern, devNum, chnlNum = 0;
	nlm_u8 CBDataWr[NLMDEV_CB_WIDTH_IN_BYTES] ;
	nlm_u16 i = 0;

    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)                   
        return(errNum);   
   
    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;  
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    /* Initialize the structure used to test the read and write to CB registers */
    CBWrRdInfo.anyInfo_p = anyInfo_p;
    CBWrRdInfo.m_flag = anyFlag;

    /* Seed is provided as an parameter with the test case */
    CBWrRdInfo.m_seed_p = &testInfo_p->m_testParams[0]; 

    /* carry out for all data patterns 
       use NLMDIAG_PATTERN_ALL_0S as start pattern for device */

    /* With the CB as Register Write and Register Read */
    for(pattern = NLMDIAG_PATTERN_RANDOM; pattern < NLMDIAG_PATTERN_END; pattern++)
    {
        CBWrRdInfo.m_pattern = pattern;       
        CBWrRdInfo.m_startAddress = 0;  
                
        for(CBWrRdInfo.m_dataLen = 10; 
            CBWrRdInfo.m_dataLen <= (NLMDEV_MAX_CB_WRITE_IN_BYTES); CBWrRdInfo.m_dataLen += 10)
        {
            for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
            {
                /* If test case is for CB Reg write(80b) */
                CBWrRdInfo.m_numOfEntries = NLMDEV_CB_DEPTH;    
                
                for(i = 0; i < CBWrRdInfo.m_numOfEntries; i++)
				{
					CBWrRdInfo.m_startAddress =+i;
					/* Invoke the test fn which will performs CB Reg writes */
					if((errNum = NlmDiag_WriteToCBRegister(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
						&CBWrRdInfo, testInfo_p->m_testLogFile, CBDataWr))!=NLMDIAG_ERROR_NOERROR)      
					return(errNum);  
	                            
					/* Invoke the test fn which will performs CB Reg reads */
					if((errNum = NlmDiag_ReadFromCBRegisterAndCompare(moduleInfo_p->m_dev_ppp[chnlNum][devNum],
						&CBWrRdInfo, testInfo_p->m_testLogFile, CBDataWr)) !=NLMDIAG_ERROR_NOERROR)
					return(errNum); 
				}
            }
        }
    }  

    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum); 

    return NLMDIAG_ERROR_NOERROR;    
}


/* 
    This function writes into the Global registers and reads back the contents,
    then the atcual written and the read values are compared, if both are same
    return zero else print the expected values and return error code.   
 */
nlm_u32 
NlmDiag_GlobalRegister_ReadWriteTest(NlmDiag_TestInfo *testInfo_p)                       
{  
    NlmDiag_GlobalRegWrRdInfo globalRegWrRdInfo;    
    NlmDiag_GlobalRegInfo globalRegInfo;
    nlm_u32 errNum;
    nlm_u8 ptrn, devNum;

    nlm_u32 anyFlag = 0; 
    nlm_u8 chnlNum = 0;
    void *anyInfo_p = NULL;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
       
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)                   
        return(errNum);   

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;    
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

    /* Initialize the structure used to test the read and write to blk registers */
    globalRegWrRdInfo.anyInfo_p = anyInfo_p;
    globalRegWrRdInfo.m_flag = anyFlag;

    /* Seed is provided as an parameter with the test case */
    globalRegWrRdInfo.m_seed_p = &testInfo_p->m_testParams[0];   

    for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
    {   
        if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
            return(NLMDIAG_ERROR_INVALID_INPUTPTR);

        globalRegWrRdInfo.m_globalRegType = NLMDEV_DEVICE_ID_REG;
        if((errNum = NlmDiag_ReadFromGlobalRegisterAndCompare(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                    &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum);   
            
        /* Carry the test for all elevan patterns excluding the compare type pattern.
           for random patterns use the seed value with the command line argument -seed #value. 
           If this argument is not provided then use default defined one .i.e.NLMDIAG_DEFAULT_SEED_VALUE */
        for(ptrn = NLMDIAG_PATTERN_RANDOM; ptrn < NLMDIAG_PATTERN_END; ptrn++)
        {
            globalRegWrRdInfo.m_pattern = ptrn;            
            
            /* write secified patterns of data into the global registers selected below
               and read it back and compare the content written, return error if MISMATCH */

            /* Device Configuration register write and read functions respectively */
            globalRegWrRdInfo.m_globalRegType = NLMDEV_DEVICE_CONFIG_REG; 
            if((errNum = NlmDiag_WriteToGlobalRegister(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum); 

            globalRegWrRdInfo.m_globalRegType = NLMDEV_DEVICE_CONFIG_REG; 
            if((errNum = NlmDiag_ReadFromGlobalRegisterAndCompare(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum); 

            /* Error ststus mask register write and read functions respectively */
            globalRegWrRdInfo.m_globalRegType = NLMDEV_ERROR_STATUS_MASK_REG; 
            if((errNum = NlmDiag_WriteToGlobalRegister(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum); 

            globalRegWrRdInfo.m_globalRegType = NLMDEV_ERROR_STATUS_MASK_REG; 
            if((errNum = NlmDiag_ReadFromGlobalRegisterAndCompare(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum);

            /* Soft Error ststus register write and read functions respectively */
            globalRegWrRdInfo.m_globalRegType = NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG; 
            if((errNum = NlmDiag_WriteToGlobalRegister(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum); 

            globalRegWrRdInfo.m_globalRegType = NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG; 
            if((errNum = NlmDiag_ReadFromGlobalRegisterAndCompare(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum);

            
            /* Device Scratch Pad0 register write and read functions respectively */
            globalRegWrRdInfo.m_globalRegType = NLMDEV_SCRATCH_PAD0_REG;      
            if((errNum = NlmDiag_WriteToGlobalRegister(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum);

            globalRegWrRdInfo.m_globalRegType = NLMDEV_SCRATCH_PAD0_REG;      
            if((errNum = NlmDiag_ReadFromGlobalRegisterAndCompare(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum);

            /* Device Scratch Pad1 register write and read functions respectively */
            globalRegWrRdInfo.m_globalRegType = NLMDEV_SCRATCH_PAD1_REG;      
            if((errNum = NlmDiag_WriteToGlobalRegister(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum);            

            globalRegWrRdInfo.m_globalRegType = NLMDEV_SCRATCH_PAD1_REG;      
            if((errNum = NlmDiag_ReadFromGlobalRegisterAndCompare(
                moduleInfo_p->m_dev_ppp[chnlNum][devNum], &globalRegWrRdInfo,
                &globalRegInfo, testInfo_p->m_testLogFile))!=NLMDIAG_ERROR_NOERROR)             
            return(errNum);             
        }
    }
    
    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);  
    return NLMDIAG_ERROR_NOERROR;    
}


/*=================================================================================
                     Register write and Read functions
  ================================================================================*/

/* (1) Range Registers: */

/* 
   This function writes 80b data to the Range register (4096 registers : 0x85000-0xfff)
   with the different squence of patterns, and calls the device manager API to
   write the data into the register locations.
*/


nlm_u32 
NlmDiag_WriteToRangeReg(NlmDev *dev_p,
                          NlmDiag_RangeRegWrRdInfo *rangeRegWrInfo_p,
                          NlmDiag_RangeRegsInfo *rangeRegs,
                          NlmCmFile *logFile
                          )
{
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
    NlmDevRangeReg rangeData;
    
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || rangeRegWrInfo_p == NULL
        || rangeRegs == NULL || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if((rangeRegWrInfo_p->m_regNum > ( NLMDIAG_RANGE_START_ADDRESS + NLMDIAG_RANGE_REGISTER_COUNT)))
         return NLMDIAG_ERROR_INVALID_INPUT;

	/* Generate the required data pattern */
    if((errNum = NlmDiag_CommonGenDataPattern(rangeRegWrInfo_p->m_pattern, 
		rangeData.m_data, 10, rangeRegWrInfo_p->m_seed_p, rangeRegWrInfo_p->m_flag,
        rangeRegWrInfo_p->anyInfo_p, logFile))!= NLMDIAG_ERROR_NOERROR)
        return(errNum);
    
    NlmCm__memcpy(rangeRegs->m_rangeRegData
		[rangeRegWrInfo_p->m_regNum - NLMDIAG_RANGE_START_ADDRESS], rangeData.m_data, 10);  
         
    /* Call the range register API to write the data into the memory block */
	if((errNum = NlmDevMgr__RangeRegisterWrite(dev_p, rangeRegWrInfo_p->m_regNum, 
        &rangeData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile, "\n\tErr:(%d), Dev Mgr Fn: NlmDevMgr__RangeRegisterWrite",errNum);
        return(NLMDIAG_ERROR_INTERNAL_ERR(errNum));
    }

    return NLMDIAG_ERROR_NOERROR;
}


/* 
   This function reads 80b data from the Range register (4k registers : 0x85000-0xfff),
   calls the device manager API to read the data from the register locations, and
   compares the data read to the data written and if both match returns 0, else
   error code and the reason code.
*/

nlm_u32 
NlmDiag_ReadFromRangeRegAndCompare(NlmDev *dev_p,
                                      NlmDiag_RangeRegWrRdInfo *rangeRegRdInfo_p,
                                      NlmDiag_RangeRegsInfo *rangeRegs,
                                      NlmCmFile *logFile
                                      )
{   
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
    nlm_u8 *rangeData_p;
    NlmDevRangeReg rangeReadData;
    
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || rangeRegRdInfo_p == NULL
        || rangeRegs == NULL || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if((rangeRegRdInfo_p->m_regNum > (NLMDIAG_RANGE_START_ADDRESS + NLMDIAG_RANGE_REGISTER_COUNT)))
        return NLMDIAG_ERROR_INVALID_INPUT;

	/* Invoke device manager API to read Range entry from the block */
    if((errNum = NlmDevMgr__RangeRegisterRead(dev_p, rangeRegRdInfo_p->m_regNum, 
		&rangeReadData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile, "\n\tErr:(%d), Dev Mgr Fn: NlmDevMgr__RangeRegisterRead",errNum);
        return(NLMDIAG_ERROR_INTERNAL_ERR(errNum));
    }

    rangeData_p = rangeRegs->m_rangeRegData[rangeRegRdInfo_p->m_regNum - NLMDIAG_RANGE_START_ADDRESS];

	/* mask reserved bits and compare the Range register data read with the data written*/
	if (rangeRegRdInfo_p->m_regNum >= NLMDIAG_RANGE_START_ADDRESS &&
		rangeRegRdInfo_p->m_regNum < NLM_REG_RANGE_A_CODE0)
	{
		nlm_u32 wrValue = 0, rdValue = 0;
		nlm_u32 wrValue1 = 0, rdValue1 = 0;

		wrValue = ReadBitsInArrray(rangeData_p, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
		rdValue = ReadBitsInArrray(rangeReadData.m_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0);

		wrValue1 = ReadBitsInArrray(rangeData_p, NLMDEV_REG_LEN_IN_BYTES, 33, 32);
		rdValue1 = ReadBitsInArrray(rangeReadData.m_data, NLMDEV_REG_LEN_IN_BYTES, 33, 32);

		if((wrValue != rdValue) || (wrValue1 != rdValue1))
		{
			NlmCmFile__fprintf(logFile, " ==> Read Data in - Correct : ");
			NlmCmFile__fprintf(logFile, "\t Read    %u %u:", rdValue1, rdValue);
			NlmCmFile__fprintf(logFile, "\t Written %u %u:", wrValue1, wrValue);

			return NLMDIAG_ERROR_VERIFY_FAIL;
		}
	}
	else
	{
		nlm_u32 wrValue = 0, rdValue = 0;
		nlm_u32 wrValue1 = 0, rdValue1 = 0;

		wrValue = ReadBitsInArrray(rangeData_p, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
		rdValue = ReadBitsInArrray(rangeReadData.m_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0);

		wrValue1 = ReadBitsInArrray(rangeData_p, NLMDEV_REG_LEN_IN_BYTES, 47, 32);
		rdValue1 = ReadBitsInArrray(rangeReadData.m_data, NLMDEV_REG_LEN_IN_BYTES, 47, 32);

		if((wrValue != rdValue) || (wrValue1 != rdValue1))
		{
			NlmCmFile__fprintf(logFile, " ==> Read Data in - Correct : ");
			NlmCmFile__fprintf(logFile, "\t Read    %u %u:", rdValue1, rdValue);
			NlmCmFile__fprintf(logFile, "\t Written %u %u:", wrValue1, wrValue);

			return NLMDIAG_ERROR_VERIFY_FAIL;
		}
	}

    return NLMDIAG_ERROR_NOERROR;
}


/* (2) Block and BMR Registers: */

nlm_u32 
NlmDiag_BlockRegister_ReadWriteTest(NlmDiag_TestInfo *testInfo_p) 
{  
    NlmDiag_BlkRegWrRdInfo blkRegWrRdInfo;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;

    nlm_u32 errNum; 
    nlm_u32 anyFlag = 0;

    nlm_u8 pattern;
    nlm_u8 abNum;
    nlm_u8 chnlNum = 0;
    nlm_u8 devNum = 0; 
    
    /* Check input parameters */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;   

    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)
        return(errNum);   
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
    
    blkRegWrRdInfo.m_flag = anyFlag;   
    blkRegWrRdInfo.m_seed_p = &testInfo_p->m_testParams[0];
    
    for(pattern = 0; pattern < 1; pattern++)
    {           
        blkRegWrRdInfo.m_pattern = pattern;
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {   
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);

            for(abNum = 0;abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
            {
                /* Initialize Block Config Reg, and enable all the ABs and Initialize ABwidth to be random */
                blkRegWrRdInfo.m_abNum = abNum;
                blkRegWrRdInfo.m_blkRegType = NLMDEV_BLOCK_CONFIG_REG;
                blkRegWrRdInfo.m_pattern = NLMDIAG_PATTERN_RANDOM;
                
                if((errNum = NlmDiag_WriteToBlockOrBMRRegister(
                    moduleInfo_p->m_dev_ppp[chnlNum][devNum], &blkRegWrRdInfo, 
                                testInfo_p->m_testLogFile)) != NLMDIAG_ERROR_NOERROR)      
                return(errNum);  
            }
        }
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {   
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);

            for(abNum = 0;abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
            {
                blkRegWrRdInfo.m_abNum = abNum;
                blkRegWrRdInfo.m_blkRegType = NLMDEV_BLOCK_CONFIG_REG;

                if((errNum = NlmDiag_ReadFromBlockOrBMRRegisterAndCompare(
                    moduleInfo_p->m_dev_ppp[chnlNum][devNum], &blkRegWrRdInfo,
                            testInfo_p->m_testLogFile)) != NLMDIAG_ERROR_NOERROR)      
                return(errNum);
              }
        }
    }
    
    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p)) != NLMDIAG_ERROR_NOERROR)
        return(errNum);  
    return NLMDIAG_ERROR_NOERROR; 
}

/* BMR register write and read test */

nlm_u32 
NlmDiag_BMRRegister_ReadWriteTest(NlmDiag_TestInfo *testInfo_p) 
{  
    NlmDiag_BlkRegWrRdInfo BMR_regWrRdInfo;
    nlm_u8 abNum;
    nlm_u8 bmrNum;
    nlm_u8 pattern;
    nlm_u32 errNum; 
    nlm_u8 devNum = 0; 
    nlm_u8 chnlNum = 0; 
    nlm_u32 anyFlag = 0;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;

    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)
            return(errNum);

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
         
    BMR_regWrRdInfo.m_flag = anyFlag;   
    BMR_regWrRdInfo.m_seed_p = &testInfo_p->m_testParams[0];
    
    for(pattern = NLMDIAG_PATTERN_RANDOM; pattern < NLMDIAG_PATTERN_END; pattern++)
    {           
        BMR_regWrRdInfo.m_pattern = pattern;
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {   
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);

            for(abNum = 0;abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
            {    
                /* Initialize BMR Config Reg, enable all ABs, and Initialize ABwidth to be random */
                BMR_regWrRdInfo.m_abNum = abNum;
                                   
                for(bmrNum = NLMDEV_BLOCK_MASK_0_0_REG; bmrNum < NLMDEV_BLOCK_REG_END; bmrNum++)
                {
                    BMR_regWrRdInfo.m_blkRegType = bmrNum;
                   
                    if((errNum = NlmDiag_WriteToBlockOrBMRRegister(
                        moduleInfo_p->m_dev_ppp[chnlNum][devNum], &BMR_regWrRdInfo, 
                                testInfo_p->m_testLogFile)) != NLMDIAG_ERROR_NOERROR)      
                    return(errNum);
                }
            }
        }
        for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
        {   
            if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
                return(NLMDIAG_ERROR_INVALID_INPUTPTR);

            for(abNum = 0;abNum < NLMDEV_NUM_ARRAY_BLOCKS; abNum++)
            { 
                for(bmrNum = NLMDEV_BLOCK_MASK_0_0_REG; bmrNum < NLMDEV_BLOCK_REG_END; bmrNum++)
                {
                    BMR_regWrRdInfo.m_blkRegType = bmrNum;
                    if((errNum = NlmDiag_ReadFromBlockOrBMRRegisterAndCompare(
                            moduleInfo_p->m_dev_ppp[chnlNum][devNum], &BMR_regWrRdInfo, 
                                    testInfo_p->m_testLogFile)) != NLMDIAG_ERROR_NOERROR)      
                        return(errNum);
                }
            }
        }
    }

    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p)) != NLMDIAG_ERROR_NOERROR)
        return(errNum);  

    return NLMDIAG_ERROR_NOERROR; 
}


/* 
   This function writes 80b data to the Block register, and the lower 8 bits
   reprasents entry_map, block_width and enable/diasable flag of the specific
   block. It calls the device manager API to write the data into the register.
*/

nlm_u32 
NlmDiag_WriteToBlockOrBMRRegister(NlmDev *dev_p,
                                     NlmDiag_BlkRegWrRdInfo *blkRegWrInfo_p,
                                     NlmCmFile *logFile
                                     )
{
    NlmDevBlockConfigReg *blkConfigRegInfo_p = NULL;
    nlm_u8 generatedData[NLMDEV_REG_LEN_IN_BYTES];
    NlmDevBlockMaskReg *blkMaskRegInfo_p = NULL;
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
           
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || blkRegWrInfo_p == NULL
        || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(blkRegWrInfo_p->m_abNum >= NLMDEV_NUM_ARRAY_BLOCKS 
        || blkRegWrInfo_p->m_blkRegType >= NLMDEV_BLOCK_REG_END)
        return NLMDIAG_ERROR_INVALID_INPUT;
    
    /* Check for blk Register type and construct the pattern of data 
     * according to specified register fields */
    if(blkRegWrInfo_p->m_blkRegType == NLMDEV_BLOCK_CONFIG_REG)
    {
         /* Generate the specified data pattern */
        if((errNum = NlmDiag_CommonGenDataPattern(blkRegWrInfo_p->m_pattern, generatedData, 1,
            blkRegWrInfo_p->m_seed_p, blkRegWrInfo_p->m_flag, blkRegWrInfo_p->anyInfo_p,
            logFile)) != NLMDIAG_ERROR_NOERROR)
            return(errNum);

         /* Get shadow chip memory for specified blk reg */
        blkConfigRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[blkRegWrInfo_p->m_abNum].m_blkConfig;
        
        /* Initialize the blk config structure */
        blkConfigRegInfo_p->m_blockEnable = *generatedData & 0x1;
        blkConfigRegInfo_p->m_blockWidth = (((*generatedData & 0x6) >> 1) % 4);

        /* Invoke the device manager API which writes to specified Blk reg */
        if((errNum = NlmDevMgr__BlockRegisterWrite(dev_p, 
            blkRegWrInfo_p->m_abNum, blkRegWrInfo_p->m_blkRegType, 
            (void*)blkConfigRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterWrite",reasonCode);
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
    }
    else
    {   /* If regtype is BMR */ 

        nlm_32 bmrNum;
        nlm_32 bmrSegNum;

        bmrNum = (blkRegWrInfo_p->m_blkRegType - 1)/NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR;
        bmrSegNum = (blkRegWrInfo_p->m_blkRegType - 1)% NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR;

        /* Get shadow chip memory for specified blk reg */
        blkMaskRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[blkRegWrInfo_p->m_abNum].m_bmr[bmrNum][bmrSegNum];

        /* Generate the specified data pattern */
        if((errNum = NlmDiag_CommonGenDataPattern(blkRegWrInfo_p->m_pattern, 
            blkMaskRegInfo_p->m_mask, NLMDEV_REG_LEN_IN_BYTES,blkRegWrInfo_p->m_seed_p, 
            blkRegWrInfo_p->m_flag,blkRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
            return(errNum);
        
        /* Invoke the device manager API which writes to specified Blk reg */
        if((errNum = NlmDevMgr__BlockRegisterWrite(dev_p, 
            blkRegWrInfo_p->m_abNum, blkRegWrInfo_p->m_blkRegType,
            (void*)blkMaskRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterWrite",reasonCode);
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
    }  
    return NLMDIAG_ERROR_NOERROR;
}


/* 
   This function reads 80b data to the Block register, and the lower 8 bits
   reprasents entry_map, block_width and enable/diasable flag of the specific
   block. It calls the device manager API to read the data into the register,
   and comapres the read and written data, if match return 0, else error code
   and reason code.
*/

nlm_u32 
NlmDiag_ReadFromBlockOrBMRRegisterAndCompare(NlmDev *dev_p,
                                                NlmDiag_BlkRegWrRdInfo *blkRegRdInfo_p,
                                                NlmCmFile *logFile
                                                )
{
    NlmDevBlockConfigReg *blkConfigRegInfo_p = NULL;
    NlmDevBlockConfigReg blkConfigRegReadInfo;
    NlmDevBlockMaskReg *blkMaskRegInfo_p = NULL;
    NlmDevBlockMaskReg blkMaskRegReadInfo;
    nlm_u32 errNum;
    NlmReasonCode reasonCode;    
    
     /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || blkRegRdInfo_p == NULL
        || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(blkRegRdInfo_p->m_abNum >= NLMDEV_NUM_ARRAY_BLOCKS 
        || blkRegRdInfo_p->m_blkRegType >= NLMDEV_BLOCK_REG_END)
        return NLMDIAG_ERROR_INVALID_INPUT;

    /* Check for blk Register type and compare the read 
     * data according to specified register fields */
    if(blkRegRdInfo_p->m_blkRegType == NLMDEV_BLOCK_CONFIG_REG )
    {
        /* Get the shadow chip memory for specified Blk reg */
        blkConfigRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[blkRegRdInfo_p->m_abNum].m_blkConfig;

          /* Invoke the device manager API which reads specified Blk register */
        if((errNum = NlmDevMgr__BlockRegisterRead(dev_p, blkRegRdInfo_p->m_abNum,
            blkRegRdInfo_p->m_blkRegType, (void*)&blkConfigRegReadInfo, &reasonCode)) 
            != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterRead",reasonCode);
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }

        /* Compare the read blk reg data with the shadow chip memory data */
        if((errNum = NlmDiag_CompareBlkRegData(blkRegRdInfo_p->m_blkRegType, 
            blkRegRdInfo_p->m_abNum, NLMDIAG_READ_FLAG, (void*)blkConfigRegInfo_p, 
            (void*)&blkConfigRegReadInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
            return errNum;    
    }
    else
    {   /* If regtype is BMR */

        nlm_32 bmrNum;
        nlm_32 bmrSegNum;

        bmrNum = (blkRegRdInfo_p->m_blkRegType - 1)/NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR;
        bmrSegNum = (blkRegRdInfo_p->m_blkRegType - 1)% NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR;

        /* Get the shadow chip memory for specified Blk reg */
        blkMaskRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_arrayBlock[blkRegRdInfo_p->m_abNum].m_bmr[bmrNum][bmrSegNum];

        /* Invoke the device manager API which reads specified Blk register */
        if((errNum = NlmDevMgr__BlockRegisterRead(dev_p,
            blkRegRdInfo_p->m_abNum, blkRegRdInfo_p->m_blkRegType,
            (void*)&blkMaskRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
        {
            NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__BlockRegisterRead",reasonCode);
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
        
        /* Compare the read blk reg data with the shadow chip memory data */
        if((errNum = NlmDiag_CompareBlkRegData(blkRegRdInfo_p->m_blkRegType,
            blkRegRdInfo_p->m_abNum, NLMDIAG_READ_FLAG, (void*)blkMaskRegInfo_p, 
            (void*)&blkMaskRegReadInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
            return errNum;
    }   
    return NLMDIAG_ERROR_NOERROR;
}



/* (3) Context Buffer Registers: */

/* 
   This function writes data to the Context Buffer register, with the random
   patterns, It calls the device manager API to write the data into the register.
*/

nlm_u32 
NlmDiag_WriteToCBRegister(NlmDev *dev_p,
                             NlmDiag_CBWrRdInfo *CBWrInfo_p,
                             NlmCmFile *logFile,
							 nlm_u8 *CBData_p
                             )
{
    nlm_u32 errNum;
    NlmDevCtxBufferReg CBRegInfo;
    NlmReasonCode reasonCode;
     
    /* Check input params */
    if(dev_p == NULL || CBWrInfo_p == NULL || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(CBWrInfo_p->m_numOfEntries == 0 || CBWrInfo_p->m_startAddress >= (NLMDEV_CB_DEPTH))
        return NLMDIAG_ERROR_INVALID_INPUT;
              
    /* Generate the specified data pattern */
    if((errNum = NlmDiag_CommonGenDataPattern(CBWrInfo_p->m_pattern, CBData_p, 
        NLMDEV_CB_WIDTH_IN_BYTES, CBWrInfo_p->m_seed_p, CBWrInfo_p->m_flag,
        CBWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);
    
    NlmCm__memcpy(CBRegInfo.m_data, CBData_p, NLMDEV_REG_LEN_IN_BYTES);

    /* Invoke the device manager API which writes to specified CB reg */
    if((errNum = NlmDevMgr__CBAsRegisterWrite(dev_p, 
        CBWrInfo_p->m_startAddress, &CBRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__CBAsRegisterWrite",reasonCode);
        return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
    }
    return NLMDIAG_ERROR_NOERROR;
}


/* 
   This function reads data from the Context Buffer register, It reads the data by 
   calling device manager API. And compares the read and written data by calling
   compare function and returns 0 if both match, else error and reason code returned.
*/

nlm_u32 
NlmDiag_ReadFromCBRegisterAndCompare(NlmDev *dev_p,
                                        NlmDiag_CBWrRdInfo *CBRdInfo_p,
                                        NlmCmFile *logFile,
										nlm_u8 *CBData_p
                                        )
{
    nlm_u32 errNum;
    NlmDevCtxBufferReg CBRegReadInfo;
    NlmReasonCode reasonCode;
   
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || CBRdInfo_p == NULL
        || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(CBRdInfo_p->m_numOfEntries == 0 || CBRdInfo_p->m_startAddress >= (NLMDEV_CB_DEPTH))
        return NLMDIAG_ERROR_INVALID_INPUT;
      
    /* Invoke Device Manager API which Reads specified CB register */
    if((errNum = NlmDevMgr__CBAsRegisterRead(dev_p, CBRdInfo_p->m_startAddress, 
                                    &CBRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__CBAsRegisterRead",reasonCode);
        return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
    }
            
    /* Compare the read CB reg data with the shadow chip memory CB Data */
    if(memcmp(CBData_p, CBRegReadInfo.m_data, NLMDEV_REG_LEN_IN_BYTES) != 0)
    {
        /* If compare fails display data read and data expected */
        NlmCmFile__fprintf(logFile, "\t Read Mismatch ; Data Read ");
        NlmDiag_CommonPrintData(logFile,CBRegReadInfo.m_data, 10);
        NlmDiag_CommonPrintData(logFile,CBRegReadInfo.m_data, 0);
                    
        return NLMDIAG_ERROR_VERIFY_FAIL;
    }
    return NLMDIAG_ERROR_NOERROR;
}


/* (4) LTR Registers: */

/* 
   This function writes data to the Logical Table register (LTR), Perticular LTR
   is passed as argument for specific operation. The Ltr is writtn with the random
   patterns (LTR pattern), and which calls the device manager API to write the data 
   into the register.
*/
nlm_u32
NlmDiag_WriteToLtrRegister(NlmDev *dev_p,              
                             NlmDiag_LTRRegWrRdInfo *ltrRegWrInfo_p,
                             NlmCmFile *logFile
                             )
{
    nlm_u8 generatedData[14];
    NlmDevBlkSelectReg       *ltrBlkSelRegInfo_p         = NULL;
    NlmDevSuperBlkKeyMapReg  *ltrBlkKeyMapRegInfo_p      = NULL;
    NlmDevParallelSrchReg	 *ltrPrlSrchRegInfo_p        = NULL;
    NlmDevRangeInsertion0Reg *ltrRangeInst0_RegInfo_p    = NULL;
    NlmDevRangeInsertion1Reg *ltrRangeInst1_RegInfo_p    = NULL;
    NlmDevMiscelleneousReg   *ltrMiscRegInfo_p           = NULL;
    NlmDevKeyConstructReg    *ltrKeyConstructRegInfo_p   = NULL;
    NlmDevSSReg              *ltrSSRegInfo_p           = NULL;
    
    nlm_u32 i=0, j=0, k=0;
    NlmReasonCode reasonCode;    
    nlm_u32 errNum;
    
    
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || ltrRegWrInfo_p == NULL
        || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(ltrRegWrInfo_p->m_ltrRegType >= NLMDEV_LTR_REG_END 
        || ltrRegWrInfo_p->m_profileNum >= NLMDEV_NUM_LTR_SET)
        return NLMDIAG_ERROR_INVALID_INPUT;
  
    /* Check for Ltr Register type and construct the pattern of data 
     *  according to specified register fields */
   switch(ltrRegWrInfo_p->m_ltrRegType)
   {
       case NLMDEV_BLOCK_SELECT_0_LTR:
       case NLMDEV_BLOCK_SELECT_1_LTR:  /* Blk select 1 Reg is for future*/
           /* Get Shadow chip memory for specified Ltr */
           ltrBlkSelRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum]
               .m_blockSelect[ltrRegWrInfo_p->m_ltrRegType - NLMDEV_BLOCK_SELECT_0_LTR];

           /* Generate Specified Data Pattern */
           if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern, 
               generatedData, 8, ltrRegWrInfo_p->m_seed_p, ltrRegWrInfo_p->m_flag,
               ltrRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
               return(errNum);

           /* Initialize the Ltr Data Structure */
           for(i=0;i<(NLMDEV_NUM_ARRAY_BLOCKS/16);i++)
           {
               for(j=0;j<8;j++)
                    ltrBlkSelRegInfo_p->m_blkEnable[i*8+j] =((generatedData[i] >> j) & 1);
           }

            /* Display the Ltr Data being written */
            NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType, 
                ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG, 
                (void*)ltrBlkSelRegInfo_p, (void *)0, logFile);

           /* Invoke Device Manager API which writes to specified Ltr */
            if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
                (void*)ltrBlkSelRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrBlkKeyMapRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum].m_superBlkKeyMap;

           /* Generate Specified Data Pattern */
           if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern,
               generatedData, 8, ltrRegWrInfo_p->m_seed_p, ltrRegWrInfo_p->m_flag,
               ltrRegWrInfo_p->anyInfo_p, logFile))!= NLMDIAG_ERROR_NOERROR)
               return(errNum);

           /* Initialize the Ltr Data Structure */
           for(i=0;i<(NLMDEV_NUM_SUPER_BLOCKS/4);i++)
           {
               for(j = 0;j < 4;j++)
                   ltrBlkKeyMapRegInfo_p->m_keyNum[i*4+j] =((generatedData[i] >> (j * 2)) & 3);
           }

            /* Display the Ltr Data being written */
           NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType,
               ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG,
               (void*)ltrBlkKeyMapRegInfo_p, (void *)0, logFile);

           /* Invoke Device Manager API which writes to specified Ltr */
            if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
                ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
                (void*)ltrBlkKeyMapRegInfo_p->m_keyNum, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;

       case NLMDEV_PARALLEL_SEARCH_0_LTR:
       case NLMDEV_PARALLEL_SEARCH_1_LTR:
       case NLMDEV_PARALLEL_SEARCH_2_LTR:
       case NLMDEV_PARALLEL_SEARCH_3_LTR:
            /* Get Shadow chip memory for specified Ltr */
           ltrPrlSrchRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum]
           .m_parallelSrch[ltrRegWrInfo_p->m_ltrRegType - NLMDEV_PARALLEL_SEARCH_0_LTR];

           /* Generate Specified Data Pattern */
           if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern,
               generatedData, 8, ltrRegWrInfo_p->m_seed_p,
               ltrRegWrInfo_p->m_flag,ltrRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
               return(errNum);

           /* Initialize Block to PS mappings */
           for(i=0;i<(NLMDEV_NUM_ARRAY_BLOCKS/4);i+=4)
               for(j=0;j<4;j++)
                   ltrPrlSrchRegInfo_p->m_psNum[i + j] =((generatedData[7 - i/4] >> (j*2)) & 3);

           /* Display the Ltr Data being written */
           NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType,
               ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG,
               (void*)ltrPrlSrchRegInfo_p, (void *)0, logFile);

           /*Invoke Device manager API which writes to specified Ltr */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
               ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
               (void*)ltrPrlSrchRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
               NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;
            
       case NLMDEV_RANGE_INSERTION_0_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrRangeInst0_RegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum].m_rangeInsert0;

           /* Generate Specified Data Pattern */
           if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern,
               generatedData, 12, ltrRegWrInfo_p->m_seed_p,
               ltrRegWrInfo_p->m_flag,ltrRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
               return(errNum);

           /* Initialise the ranges params */
           ltrRangeInst0_RegInfo_p->m_rangeAEncodingType = (generatedData[0] % 3); /* 0:3Bit, 1:2Bit, 2:no */
           ltrRangeInst0_RegInfo_p->m_rangeBEncodingType = (generatedData[1] % 3); /* 0:3Bit, 1:2Bit, 2:no */

           ltrRangeInst0_RegInfo_p->m_rangeAEncodedBytes = (generatedData[2] % 4); /* 0 - 3 */
           ltrRangeInst0_RegInfo_p->m_rangeBEncodedBytes = (generatedData[3] % 4);

           for(i = 0; i < NLMDEV_NUM_KEYS; i++)
           {
               ltrRangeInst0_RegInfo_p->m_rangeAInsertStartByte[i] = (generatedData[i + 4] % 79);
               ltrRangeInst0_RegInfo_p->m_rangeBInsertStartByte[i] = (generatedData[i + 8] % 79);
           }

           /* Display the Ltr Data being written */
           NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType,
               ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG,
               (void*)ltrRangeInst0_RegInfo_p, (void *)0, logFile);

           /*Invoke Device manager API which writes to specified Ltr */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
               ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
               (void*)ltrRangeInst0_RegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
               NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;


       case NLMDEV_RANGE_INSERTION_1_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrRangeInst1_RegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum].m_rangeInsert1;

           /* Generate Specified Data Pattern */
           if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern,
               generatedData, 12, ltrRegWrInfo_p->m_seed_p,
               ltrRegWrInfo_p->m_flag,ltrRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
               return(errNum);

           /* Initialise the ranges params */
           ltrRangeInst1_RegInfo_p->m_rangeCEncodingType = (generatedData[12] % 3); /* 0:3Bit, 1:2Bit, 2:no */
           ltrRangeInst1_RegInfo_p->m_rangeDEncodingType = (generatedData[11] % 3); /* 0:3Bit, 1:2Bit, 2:no */

           ltrRangeInst1_RegInfo_p->m_rangeCEncodedBytes = (generatedData[10] % 4); /* 0 - 3 */
           ltrRangeInst1_RegInfo_p->m_rangeDEncodedBytes = (generatedData[9] % 4);

           for(i = 0; i < NLMDEV_NUM_KEYS; i++)
           {
               ltrRangeInst1_RegInfo_p->m_rangeCInsertStartByte[i] = (generatedData[4-i] % 79);
               ltrRangeInst1_RegInfo_p->m_rangeDInsertStartByte[i] = (generatedData[8-i] % 79);
           }

           /* Display the Ltr Data being written */
           NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType,
               ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG,
               (void*)ltrRangeInst1_RegInfo_p, (void *)0, logFile);

           /*Invoke Device manager API which writes to specified Ltr */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
               ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
               (void*)ltrRangeInst1_RegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
               NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;

       case NLMDEV_MISCELLENEOUS_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrMiscRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum].m_miscelleneous;

           /* Generate Specified Data Pattern */
           if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern,
               generatedData, NLMDEV_REG_LEN_IN_BYTES, ltrRegWrInfo_p->m_seed_p,
               ltrRegWrInfo_p->m_flag,ltrRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
               return(errNum);

           for(k = 0; k < NLMDEV_NUM_PARALLEL_SEARCHES; k++)
           {
               nlm_u8 value;

               value = (nlm_u8)((generatedData[k] >> 2) & 0x0f);
               /* 0 - 4, NLMDEV_NO_MASK_BMR_NUM */
               if(value != 0x6)
                   ltrMiscRegInfo_p->m_bmrSelect[k] = (value % 5);
               else
                   ltrMiscRegInfo_p->m_bmrSelect[k] = NLMDEV_NO_MASK_BMR_NUM;
           }

           ltrMiscRegInfo_p->m_numOfValidSrchRslts    = (generatedData[1] % 4); /* 0:All, 1:1, 2:2, 3:3 results valid */
           ltrMiscRegInfo_p->m_rangeAExtractStartByte = (generatedData[2] % 79); /* 0-78 valid */
           ltrMiscRegInfo_p->m_rangeBExtractStartByte = (generatedData[3] % 79);
           ltrMiscRegInfo_p->m_rangeCExtractStartByte = (generatedData[4] % 79);
           ltrMiscRegInfo_p->m_rangeDExtractStartByte = (generatedData[5] % 79);

		   ltrMiscRegInfo_p->m_searchType[0] = 0; /* Non-Sahasra */
		   ltrMiscRegInfo_p->m_searchType[1] = 0;
				   
		   ltrMiscRegInfo_p->m_searchType[2] = 2; /* Sahasra */
		   ltrMiscRegInfo_p->m_searchType[3] = 2;
								   
		   /* Display the Ltr Data being written */
           NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType,
               ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG,
               (void*)ltrMiscRegInfo_p, (void *)0, logFile);

           /*Invoke Device manager API which writes to specified Ltr */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
               ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
               (void*)ltrMiscRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
               NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
               return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
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

           /* Get Shadow chip memory for specified Ltr */
            ltrKeyConstructRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum]
            .m_keyConstruct[ltrRegWrInfo_p->m_ltrRegType - NLMDEV_KEY_0_KCR_0_LTR];

            /* Generate Specified Data Pattern */
            if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern,
                generatedData, 9, ltrRegWrInfo_p->m_seed_p,
                ltrRegWrInfo_p->m_flag, ltrRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
                return(errNum);

            /* Initialize Key Construction Bit Mappings*/
            for(i=0;i<NLMDEV_NUM_OF_SEGMENTS_PER_KCR;i++)
            {
                /* start byte of the each comapre key: 0 - 79 */
                ltrKeyConstructRegInfo_p->m_startByteLoc[i] = 
                    (nlm_u8)(((generatedData[i] | generatedData[i*2]) % 78)+1);

                /* 1 - 16 are valid byte segments, 0 mean device mananger can ignore this segmnet */
                ltrKeyConstructRegInfo_p->m_numOfBytes[i] = 
                    (nlm_u8)((generatedData[i + NLMDEV_NUM_OF_SEGMENTS_PER_KCR] % 16)+1);
            }

            /* Display Ltr Data being written */
            NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType,
                ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG,
                (void*)ltrKeyConstructRegInfo_p, (void *)0, logFile);

           /* Invoke Device Manager API to write to specified LTR */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
               ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
               (void*)ltrKeyConstructRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;

       case NLMDEV_SS_LTR:           
            /* Get Shadow chip memory for specified Ltr */
            ltrSSRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegWrInfo_p->m_profileNum].m_ssReg;

            /* Generate Specified Data Pattern */
            if((errNum = NlmDiag_CommonGenDataPattern(ltrRegWrInfo_p->m_pattern,
                generatedData, 9, ltrRegWrInfo_p->m_seed_p,
                ltrRegWrInfo_p->m_flag, ltrRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
                return(errNum);

            /* Initialize Key Construction Bit Mappings*/
            for(i=0;i<NLMDEV_SS_RMP_AB;i++)
            {
                /* fill the SS map ports */
                ltrSSRegInfo_p->m_ss_result_map[i] = (nlm_u8)(((generatedData[i] | generatedData[i*2])) & 1);
            }

            /* Display Ltr Data being written */
            NlmDiag_CompareLtrRegData(ltrRegWrInfo_p->m_ltrRegType,
                ltrRegWrInfo_p->m_profileNum, NLMDIAG_WRITE_FLAG,
                (void*)ltrSSRegInfo_p, (void *)0, logFile);

           /* Invoke Device Manager API to write to specified LTR */
           if((errNum = NlmDevMgr__LogicalTableRegisterWrite(dev_p,
               ltrRegWrInfo_p->m_profileNum, ltrRegWrInfo_p->m_ltrRegType,
               (void*)ltrSSRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;

       case NLMDEV_LTR_REG_END:
            break;
   }       
   return NLMDIAG_ERROR_NOERROR;
}



/* 
   This function reads data from the Logical Table register (LTR), Perticular LTR
   is passed as argument. It calls the device manager API to read the LTR register.
   The Ltr data read is compared with the stored data and if Mismatch occures returns 
   error code, and reason code, else return 0.
*/
nlm_u32 
NlmDiag_ReadFromLtrRegisterAndCompare(NlmDev *dev_p,
                                         NlmDiag_LTRRegWrRdInfo *ltrRegRdInfo_p,
                                         NlmCmFile *logFile
                                         )
{    
    NlmDevBlkSelectReg       *ltrBlkSelRegInfo_p = NULL;
    NlmDevSuperBlkKeyMapReg  *ltrBlkKeyMapRegInfo_p = NULL;
    NlmDevParallelSrchReg	 *ltrPrlSrchRegInfo_p = NULL;
    NlmDevRangeInsertion0Reg *ltrRangeInst0_RegInfo_p = NULL;
    NlmDevRangeInsertion1Reg *ltrRangeInst1_RegInfo_p = NULL;
    NlmDevMiscelleneousReg   *ltrMiscRegInfo_p = NULL;
    NlmDevKeyConstructReg    *ltrKeyConstructRegInfo_p = NULL;
    NlmDevSSReg              *ltrSSRegInfo_p = NULL;

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

    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || ltrRegRdInfo_p == NULL
        || logFile == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(ltrRegRdInfo_p->m_ltrRegType >= NLMDEV_LTR_REG_END
        || ltrRegRdInfo_p->m_profileNum >= NLMDEV_NUM_LTR_SET)
        return NLMDIAG_ERROR_INVALID_INPUT;

    /* Check for Ltr Register type and read the data 
      *  according to specified register fields */
    switch(ltrRegRdInfo_p->m_ltrRegType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:  /* Blk select 1 Reg is for future*/
            /* Get the shadow chip memory for the specified LTR */
            ltrBlkSelRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum]
                .m_blockSelect[ltrRegRdInfo_p->m_ltrRegType - NLMDEV_BLOCK_SELECT_0_LTR];

            /* Invoke the device manager API to read the specified LTR */
                if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                    ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                    (void*)&ltrBlkSelRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* Compare the read LTR data with the shadow chip memory LTR Data */ 
            if((errNum = NlmDiag_CompareLtrRegData(ltrRegRdInfo_p->m_ltrRegType, 
                ltrRegRdInfo_p->m_profileNum, NLMDIAG_READ_FLAG, (void*)ltrBlkSelRegInfo_p, 
                (void*)&ltrBlkSelRegInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;    
            break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrBlkKeyMapRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum].m_superBlkKeyMap;

           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                (void*)&ltrBlkKeyMapRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* Compare the read LTR data with the shadow chip memory LTR Data */
            if((errNum = NlmDiag_CompareLtrRegData(
                ltrRegRdInfo_p->m_ltrRegType, ltrRegRdInfo_p->m_profileNum,
                NLMDIAG_READ_FLAG, (void*)ltrBlkKeyMapRegInfo_p,
                (void*)&ltrBlkKeyMapRegInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;
            break;
    
       case NLMDEV_PARALLEL_SEARCH_0_LTR:
       case NLMDEV_PARALLEL_SEARCH_1_LTR:
       case NLMDEV_PARALLEL_SEARCH_2_LTR:
       case NLMDEV_PARALLEL_SEARCH_3_LTR:
            /* Get Shadow chip memory for specified Ltr */
            ltrPrlSrchRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum]
                .m_parallelSrch[ltrRegRdInfo_p->m_ltrRegType - NLMDEV_PARALLEL_SEARCH_0_LTR];

            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                (void*)&ltrPrlSrchRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* Compare the read LTR data with the shadow chip memory LTR Data */
            if((errNum = NlmDiag_CompareLtrRegData(ltrRegRdInfo_p->m_ltrRegType,
                ltrRegRdInfo_p->m_profileNum, NLMDIAG_READ_FLAG, (void*)ltrPrlSrchRegInfo_p,
                (void*)&ltrPrlSrchRegInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrRangeInst0_RegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum].m_rangeInsert0;

           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                (void*)&ltrRangeInst0_RegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

             /* Compare the read LTR data with the shadow chip memory LTR Data */
            if((errNum = NlmDiag_CompareLtrRegData(ltrRegRdInfo_p->m_ltrRegType,
                ltrRegRdInfo_p->m_profileNum, NLMDIAG_READ_FLAG, (void*)ltrRangeInst0_RegInfo_p,
                (void*)&ltrRangeInst0_RegInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;
            break;

        case NLMDEV_RANGE_INSERTION_1_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrRangeInst1_RegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum].m_rangeInsert1;

           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                (void*)&ltrRangeInst1_RegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

             /* Compare the read LTR data with the shadow chip memory LTR Data */
            if((errNum = NlmDiag_CompareLtrRegData(ltrRegRdInfo_p->m_ltrRegType,
                ltrRegRdInfo_p->m_profileNum, NLMDIAG_READ_FLAG, (void*)ltrRangeInst1_RegInfo_p,
                (void*)&ltrRangeInst1_RegInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;
            break;

        case NLMDEV_MISCELLENEOUS_LTR:
           /* Get Shadow chip memory for specified Ltr */
           ltrMiscRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum].m_miscelleneous;

            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                (void*)&ltrMiscRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

             /* Compare the read LTR data with the shadow chip memory LTR Data */
            if((errNum = NlmDiag_CompareLtrRegData(ltrRegRdInfo_p->m_ltrRegType,
                ltrRegRdInfo_p->m_profileNum, NLMDIAG_READ_FLAG, (void*)ltrMiscRegInfo_p,
                (void*)&ltrMiscRegInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;
            break;

       case NLMDEV_KEY_0_KCR_0_LTR:
       case NLMDEV_KEY_0_KCR_1_LTR:
       case NLMDEV_KEY_1_KCR_0_LTR:
       case NLMDEV_KEY_1_KCR_1_LTR:
       case NLMDEV_KEY_2_KCR_0_LTR:
       case NLMDEV_KEY_2_KCR_1_LTR:
       case NLMDEV_KEY_3_KCR_0_LTR:
       case NLMDEV_KEY_3_KCR_1_LTR:

           /* Get Shadow chip memory for specified Ltr */
           ltrKeyConstructRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum]
           .m_keyConstruct[ltrRegRdInfo_p->m_ltrRegType - NLMDEV_KEY_0_KCR_0_LTR];

            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                (void*)&ltrKeyConstructRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* Compare the read LTR data with the shadow chip memory LTR Data */
            if((errNum = NlmDiag_CompareLtrRegData(
                ltrRegRdInfo_p->m_ltrRegType, ltrRegRdInfo_p->m_profileNum,
                NLMDIAG_READ_FLAG, (void*)ltrKeyConstructRegInfo_p,
                (void*)&ltrKeyConstructRegInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;

            break;

        case NLMDEV_SS_LTR:
            /* Get Shadow chip memory for specified Ltr */
            ltrSSRegInfo_p = &NLMDIAG_SHADOW_DEV_PTR->m_ltr[ltrRegRdInfo_p->m_profileNum].m_ssReg;

            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrRegRdInfo_p->m_profileNum, ltrRegRdInfo_p->m_ltrRegType,
                (void*)&ltrSSRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* Compare the read LTR data with the shadow chip memory LTR Data */
            if((errNum = NlmDiag_CompareLtrRegData(
                ltrRegRdInfo_p->m_ltrRegType, ltrRegRdInfo_p->m_profileNum,
                NLMDIAG_READ_FLAG, (void*)ltrSSRegInfo_p,
                (void*)&ltrSSRegReadInfo, logFile)) != NLMDIAG_ERROR_NOERROR)
                return errNum;

            break;

        case NLMDEV_LTR_REG_END:
            break;
    }   
    return NLMDIAG_ERROR_NOERROR;
}

/* ====== Global register write and read functions ====== */

/* 
    This function calls the register write function to write the device's
    Global registers and prints written values.
 */
nlm_u32 
NlmDiag_WriteToGlobalRegister(NlmDev *dev_p,
                                NlmDiag_GlobalRegWrRdInfo  *globalRegWrInfo_p,
                                NlmDiag_GlobalRegInfo *globalRegs_p,
                                NlmCmFile *logFile
                                )
{
    nlm_u8 generatedData[25];   
    NlmReasonCode reasonCode;
    
    nlm_u32 errNum;
    NlmDevScratchPadReg *scratchPadRegInfo_p;
    
    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || globalRegWrInfo_p == NULL
        || logFile == NULL || globalRegs_p == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(globalRegWrInfo_p->m_globalRegType >= NLMDEV_GLOBALREG_END)         
        return NLMDIAG_ERROR_INVALID_INPUT;
    
    /* Check for Global Register type and construct the pattern of data 
    according to specified register fields */
    switch(globalRegWrInfo_p->m_globalRegType)
    {
        case NLMDEV_DEVICE_ID_REG:                
            NlmCmFile__fprintf(logFile, "\n\t Error: Device Id Register is Read only. ");
            return(NLMDIAG_ERROR_INVALID_OPERATION);

        case NLMDEV_RESULT0_REG:
        case NLMDEV_RESULT1_REG:            
            NlmCmFile__fprintf(logFile, 
                "\n\t Error: Writing to read only Global registers(Result0 and Result1Registers).");
            return(NLMDIAG_ERROR_INVALID_OPERATION);

        case NLMDEV_ERROR_STATUS_REG:
        case NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:            
            NlmCmFile__fprintf(logFile, 
                "\n\t Error: Currently Not supported to write these registers(Error status/Adv soft_error_Registers).");
            return(NLMDIAG_ERROR_INVALID_OPERATION);

        case NLMDEV_DEVICE_CONFIG_REG:    /* Device Config Reg */                   
            /* Generate Specified Data Pattern */
            if((errNum = NlmDiag_CommonGenDataPattern(globalRegWrInfo_p->m_pattern, 
                generatedData, 8, globalRegWrInfo_p->m_seed_p, globalRegWrInfo_p->m_flag,
                globalRegWrInfo_p->anyInfo_p, logFile))!=NLMDIAG_ERROR_NOERROR)
            return(errNum);

            globalRegs_p->m_devConfigReg.m_dbParityErrEntryInvalidate = generatedData[0] & 1;
            globalRegs_p->m_devConfigReg.m_dbSoftErrProtectMode = generatedData[1] & 1;
            globalRegs_p->m_devConfigReg.m_eccScanType = generatedData[2] & 1;
            globalRegs_p->m_devConfigReg.m_lowPowerModeEnable = generatedData[3] & 1;
            globalRegs_p->m_devConfigReg.m_rangeEngineEnable = generatedData[4] & 1;
            globalRegs_p->m_devConfigReg.m_softErrorScanEnable = generatedData[5] & 1;
            
            /* Invoke Device Manager API which writes to specified Global Reg */
            if((errNum = NlmDevMgr__GlobalRegisterWrite(dev_p, globalRegWrInfo_p->m_globalRegType,
                (void*)(&globalRegs_p->m_devConfigReg), &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;

        case NLMDEV_ERROR_STATUS_MASK_REG:
            /* Generate Specified Data Pattern */
            if((errNum = NlmDiag_CommonGenDataPattern(globalRegWrInfo_p->m_pattern,
                generatedData, 24, globalRegWrInfo_p->m_seed_p, globalRegWrInfo_p->m_flag,
                globalRegWrInfo_p->anyInfo_p, logFile)) != NLMDIAG_ERROR_NOERROR)
            return(errNum);

            globalRegs_p->m_errStatusMaskReg.m_alignmentErr = (generatedData[0] & 1);
            globalRegs_p->m_errStatusMaskReg.m_burstControlWordErr = (generatedData[1] & 1);
            globalRegs_p->m_errStatusMaskReg.m_burstMaxErr = (generatedData[2] & 1);
            globalRegs_p->m_errStatusMaskReg.m_channelNumErr = (generatedData[3] & 1);
            globalRegs_p->m_errStatusMaskReg.m_crc24Err = (generatedData[4] & 1);
            globalRegs_p->m_errStatusMaskReg.m_ctxBufferParityErr = (generatedData[5] & 1);
            globalRegs_p->m_errStatusMaskReg.m_dbSoftError = (generatedData[6] & 1);
            globalRegs_p->m_errStatusMaskReg.m_dbSoftErrorFifoFull = (generatedData[7] & 1);
            globalRegs_p->m_errStatusMaskReg.m_devIdMismatchErr = (generatedData[8] & 1);
            globalRegs_p->m_errStatusMaskReg.m_eopErr = (generatedData[9] & 1);
            globalRegs_p->m_errStatusMaskReg.m_framingCtrlWordErr = (generatedData[10] & 1);
            globalRegs_p->m_errStatusMaskReg.m_globalGIO_L0_Enable = (generatedData[11] & 1);
            globalRegs_p->m_errStatusMaskReg.m_globalGIO_L1_Enable = (generatedData[12] & 1);
            globalRegs_p->m_errStatusMaskReg.m_illegalInstnErr = (generatedData[13] & 1);
            globalRegs_p->m_errStatusMaskReg.m_instnBurstErr = (generatedData[14] & 1);
            globalRegs_p->m_errStatusMaskReg.m_ltrParityErr = (generatedData[15] & 1);
            globalRegs_p->m_errStatusMaskReg.m_missingDataPktErr = (generatedData[16] & 1);
            globalRegs_p->m_errStatusMaskReg.m_parityScanFifoOverFlow = (generatedData[17] & 1);
            globalRegs_p->m_errStatusMaskReg.m_powerLimitingErr = (generatedData[18] & 1);
            globalRegs_p->m_errStatusMaskReg.m_protocolErr = (generatedData[19] & 1);
            globalRegs_p->m_errStatusMaskReg.m_rxNMACFifoParityErr = (generatedData[20] & 1);
            globalRegs_p->m_errStatusMaskReg.m_rxPCSEFifoParityErr = (generatedData[21] & 1);
            globalRegs_p->m_errStatusMaskReg.m_sopErr = (generatedData[22] & 1);
                    
           /* Invoke Device Manager API which writes to specified Global Reg */
           if((errNum = NlmDevMgr__GlobalRegisterWrite(dev_p, globalRegWrInfo_p->m_globalRegType,
               (void*)(&globalRegs_p->m_errStatusMaskReg), &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
           }
           break;

        case NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            /* Generate Specified Data Pattern */
            if((errNum = NlmDiag_CommonGenDataPattern(globalRegWrInfo_p->m_pattern,
                generatedData, 8, globalRegWrInfo_p->m_seed_p, globalRegWrInfo_p->m_flag,
                globalRegWrInfo_p->anyInfo_p, logFile)) != NLMDIAG_ERROR_NOERROR)
            return(errNum);

            /* Only Erase FIFO and Erase FIFO Entry Field are write/Read; Rest of the
                field are read only, hence ignored */

            globalRegs_p->m_softParityErrFifoReg.m_eraseFifo = (generatedData[0] & 1);
            globalRegs_p->m_softParityErrFifoReg.m_eraseFifoEntry = (generatedData[1]& 1);
            
            /* Invoke Device Manager API which writes to specified Global Reg */
            if((errNum = NlmDevMgr__GlobalRegisterWrite(dev_p, globalRegWrInfo_p->m_globalRegType,
                (void*)(&globalRegs_p->m_softParityErrFifoReg), &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            /* Toggle the bits: if they set; comment while testing on board*/
            if(globalRegs_p->m_softParityErrFifoReg.m_eraseFifo)
                globalRegs_p->m_softParityErrFifoReg.m_eraseFifo = 0;
            if(globalRegs_p->m_softParityErrFifoReg.m_eraseFifoEntry)
                globalRegs_p->m_softParityErrFifoReg.m_eraseFifoEntry = 0;
            break;

        case NLMDEV_SCRATCH_PAD0_REG:
        case NLMDEV_SCRATCH_PAD1_REG:                           
            /* Generate Specified Data Pattern */
            scratchPadRegInfo_p = 
                &globalRegs_p->m_scratchPadReg[globalRegWrInfo_p->m_globalRegType - NLMDEV_SCRATCH_PAD0_REG];
            
            if((errNum = NlmDiag_CommonGenDataPattern(globalRegWrInfo_p->m_pattern,
                scratchPadRegInfo_p->m_data, NLMDEV_REG_LEN_IN_BYTES,
                globalRegWrInfo_p->m_seed_p, globalRegWrInfo_p->m_flag,
                globalRegWrInfo_p->anyInfo_p, logFile)) != NLMDIAG_ERROR_NOERROR)
            return(errNum);
                    
            /*Invoke Device manager API which writes to specified scratch pad register */
            if((errNum = NlmDevMgr__GlobalRegisterWrite(dev_p, globalRegWrInfo_p->m_globalRegType,
                (void*)scratchPadRegInfo_p, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            break;  

        case NLMDEV_GLOBALREG_END:
        default:
            break;
    }
    return NLMDIAG_ERROR_NOERROR;
}


/* 
    This function calls the register read function to read the device's
    Global registers and comapres the results with the expected values, if
    MISMATCH print the expected value and return errors, else return 0.
 */
nlm_u32  
NlmDiag_ReadFromGlobalRegisterAndCompare(NlmDev *dev_p,
                                            NlmDiag_GlobalRegWrRdInfo *globalRegRdInfo_p,
                                            NlmDiag_GlobalRegInfo *globalRegs_p,
                                            NlmCmFile *logFile
                                            )
{
    NlmReasonCode reasonCode;    
    nlm_u32 errNum;

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
    NlmDevScratchPadReg *scratchPadRegInfo_p = NULL;     /* To read shadow memory data */
    NlmDevScratchPadReg scratchPadRegReadInfo;

	NlmCm__memset(&devIdRegReadInfo, 0, sizeof(NlmDevIdReg));
	NlmCm__memset(&devErrStatRegReadInfo, 0, sizeof(NlmDevErrStatusReg));
	NlmCm__memset(&devAdvSoftErrRegReadInfo, 0, sizeof(NlmDevAdvancedSoftErrReg));
	NlmCm__memset(&devResultReg0ReadInfo, 0, sizeof(NlmDevResultReg));
	NlmCm__memset(&devResultReg1ReadInfo, 0, sizeof(NlmDevResultReg));

	NlmCm__memset(&devConfigRegReadInfo, 0, sizeof(NlmDevConfigReg));
	NlmCm__memset(&devErrStatMaskRegReadInfo, 0, sizeof(NlmDevErrStatusReg));
	NlmCm__memset(&devDBSoftErrStatRegReadInfo, 0, sizeof(NlmDevDbSoftErrFifoReg));
	NlmCm__memset(&scratchPadRegReadInfo, 0, sizeof(NlmDevScratchPadReg));

    /* Check input params */
    if(dev_p == NULL || dev_p->m_shadowDevice_p == NULL || globalRegRdInfo_p == NULL
        || logFile == NULL || globalRegs_p == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(globalRegRdInfo_p->m_globalRegType >= NLMDEV_GLOBALREG_END)         
        return NLMDIAG_ERROR_INVALID_INPUT;

    /* Check for Global Register type and construct the pattern of data 
     *  according to specified register fields */
    switch(globalRegRdInfo_p->m_globalRegType)
    {
        case NLMDEV_DEVICE_ID_REG:  

            /* call the device manager API to read contents from the Device ID Reg */
           if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType, 
               (void*)&devIdRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
               NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
               return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
           }
           
           /* compare the Device ID register read values with expected values */
           if((devIdRegReadInfo.m_majorDieRev != NLMDIAG_DEV_MAJOR_REV)
               || (devIdRegReadInfo.m_minorDieRev != NLMDIAG_DEV_MINOR_REV)
               || (devIdRegReadInfo.m_databaseSize != NLMDIAG_DEV_DB_SIZE))
           {
               NlmCmFile__fprintf(logFile, " => Compare of Read Data Fails :");               
               NlmCmFile__fprintf(logFile, " Database Size --> %d(%d)  Major Rev --> %d(%d) Minor Rev --> %d(%d)\n",
                   devIdRegReadInfo.m_databaseSize, NLMDIAG_DEV_DB_SIZE,
                   devIdRegReadInfo.m_majorDieRev, NLMDIAG_DEV_MAJOR_REV,
                   devIdRegReadInfo.m_minorDieRev, NLMDIAG_DEV_MINOR_REV);
               return(NLMDIAG_ERROR_VERIFY_FAIL);
           }
           break;

        case NLMDEV_ERROR_STATUS_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType,
                (void*)&devErrStatRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            /* Global GIO_L Enable is valid only for Error Status Mask Reg,
                do not check this */
            if( (devErrStatRegReadInfo.m_ctxBufferParityErr != NlmFalse) ||
                (devErrStatRegReadInfo.m_dbSoftError != NlmFalse) ||
                (devErrStatRegReadInfo.m_dbSoftErrorFifoFull != NlmFalse) ||
                (devErrStatRegReadInfo.m_devIdMismatchErr != NlmFalse) ||
                /* (devErrStatRegReadInfo.m_globalGIO_L_Enable != NlmFalse) || */
                (devErrStatRegReadInfo.m_illegalInstnErr != NlmFalse) ||
                (devErrStatRegReadInfo.m_ltrParityErr != NlmFalse))
            {
                NlmCmFile__fprintf(logFile, "\n\t => Compare of Read Data Fails :");  
                NlmCmFile__fprintf(logFile, "\n Context Buf Parity Err      : %d(0), Database Soft Error       : %d(0)",
                    devErrStatRegReadInfo.m_ctxBufferParityErr, devErrStatRegReadInfo.m_dbSoftError);
                NlmCmFile__fprintf(logFile, "\n Database Soft Err Fifo Full : %d(0), DevId mismatch Error      : %d(0)",
                    devErrStatRegReadInfo.m_dbSoftErrorFifoFull, devErrStatRegReadInfo.m_devIdMismatchErr);
                NlmCmFile__fprintf(logFile, "\n Illegal Instruction Error  : %d(0)", devErrStatRegReadInfo.m_illegalInstnErr);
                NlmCmFile__fprintf(logFile, "\n LTR parity engine err  : %d(0)", devErrStatRegReadInfo.m_ltrParityErr);
                
                return(NLMDIAG_ERROR_VERIFY_FAIL);
            }
            break;

        case NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
            {
                (void)devAdvSoftErrRegReadInfo;
                NlmCmFile__fprintf(logFile, "\n Error: Testing of Adv soft err Register not supported");
            }
            break;

        case NLMDEV_RESULT0_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType,
                (void*)&devResultReg0ReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            if( (devResultReg0ReadInfo.m_hitAddress[0] != 0x0) ||
                (devResultReg0ReadInfo.m_hitAddress[1] != 0x0) ||
                (devResultReg0ReadInfo.m_hitOrMiss[0] != (Nlm11kDevMissHit)NLMDEV_MISS) ||
                (devResultReg0ReadInfo.m_hitOrMiss[1] != (Nlm11kDevMissHit)NLMDEV_MISS))
            {
                NlmCmFile__fprintf(logFile, "\n\t => Compare of Read Data Fails :");  
                NlmCmFile__fprintf(logFile, "\n Reasult Reg 0: Hit Address : %u (0), %u (0)",
                    devResultReg0ReadInfo.m_hitAddress[0], devResultReg0ReadInfo.m_hitAddress[1]);
                NlmCmFile__fprintf(logFile, "\n Hit or Miss : %d (MISS), %d (MISS)",
                    devResultReg0ReadInfo.m_hitOrMiss[0], devResultReg0ReadInfo.m_hitOrMiss[1]);
                return(NLMDIAG_ERROR_VERIFY_FAIL);
            }
            break;

        case NLMDEV_RESULT1_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType,
                (void*)&devResultReg1ReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }

            if((devResultReg1ReadInfo.m_hitAddress[0] != 0) || (devResultReg1ReadInfo.m_hitAddress[1] != 0) ||
                (devResultReg1ReadInfo.m_hitOrMiss[0] != 0) || (devResultReg1ReadInfo.m_hitOrMiss[1] != 0))
            {
                NlmCmFile__fprintf(logFile, "\n\t => Compare of Read Data Fails :");  
                NlmCmFile__fprintf(logFile, "\n Reasult Reg 1: Hit Address : %u (0), %u (0)",
                    devResultReg1ReadInfo.m_hitAddress[0], devResultReg1ReadInfo.m_hitAddress[1]);
                NlmCmFile__fprintf(logFile, "\n Hit or Miss : %d (MISS), %d (MISS)",
                    devResultReg1ReadInfo.m_hitOrMiss[0], devResultReg1ReadInfo.m_hitOrMiss[1]);
                return(NLMDIAG_ERROR_VERIFY_FAIL);
            }
            break;
        /* ============================================== */

        case NLMDEV_DEVICE_CONFIG_REG: /* Device Config Reg */
            if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType,
                (void*)&devConfigRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            /* compare the data */
            if((devConfigRegReadInfo.m_dbParityErrEntryInvalidate!= globalRegs_p->m_devConfigReg.m_dbParityErrEntryInvalidate)
                || (devConfigRegReadInfo.m_dbSoftErrProtectMode!= globalRegs_p->m_devConfigReg.m_dbSoftErrProtectMode)
                || (devConfigRegReadInfo.m_eccScanType!= globalRegs_p->m_devConfigReg.m_eccScanType)
                || (devConfigRegReadInfo.m_lowPowerModeEnable!= globalRegs_p->m_devConfigReg.m_lowPowerModeEnable)
                || (devConfigRegReadInfo.m_rangeEngineEnable!= globalRegs_p->m_devConfigReg.m_rangeEngineEnable)
                || (devConfigRegReadInfo.m_softErrorScanEnable!= globalRegs_p->m_devConfigReg.m_softErrorScanEnable))
            {
                NlmCmFile__fprintf(logFile, "\n\t => Compare of Read Data Fails :");  
                NlmCmFile__fprintf(logFile, "\n Db ParityErr Entry Invalidate --> %d(%d), Soft Err Protect Mode --> %d(%d)",
                    devConfigRegReadInfo.m_dbParityErrEntryInvalidate,
                    globalRegs_p->m_devConfigReg.m_dbParityErrEntryInvalidate,
                    devConfigRegReadInfo.m_dbSoftErrProtectMode, globalRegs_p->m_devConfigReg.m_dbSoftErrProtectMode);
                NlmCmFile__fprintf(logFile, "\n ECC Scan Type                 --> %d(%d), Low Power Mode Enable --> %d(%d)",
                    devConfigRegReadInfo.m_eccScanType, globalRegs_p->m_devConfigReg.m_eccScanType,
                    devConfigRegReadInfo.m_lowPowerModeEnable, globalRegs_p->m_devConfigReg.m_lowPowerModeEnable);
                NlmCmFile__fprintf(logFile, "\n Range Engine Enable           --> %d(%d), Soft Err Scan Enable  --> %d(%d)",
                    devConfigRegReadInfo.m_rangeEngineEnable, globalRegs_p->m_devConfigReg.m_rangeEngineEnable,
                    devConfigRegReadInfo.m_softErrorScanEnable, globalRegs_p->m_devConfigReg.m_softErrorScanEnable);
                return(NLMDIAG_ERROR_VERIFY_FAIL);
            }
            break;

       case NLMDEV_ERROR_STATUS_MASK_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType,
                (void*)&devErrStatMaskRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            /* compare read data */
            if((devErrStatMaskRegReadInfo.m_alignmentErr != globalRegs_p->m_errStatusMaskReg.m_alignmentErr) ||
            (devErrStatMaskRegReadInfo.m_burstControlWordErr != globalRegs_p->m_errStatusMaskReg.m_burstControlWordErr) ||
            (devErrStatMaskRegReadInfo.m_burstMaxErr != globalRegs_p->m_errStatusMaskReg.m_burstMaxErr) ||
            (devErrStatMaskRegReadInfo.m_channelNumErr != globalRegs_p->m_errStatusMaskReg.m_channelNumErr) ||
            (devErrStatMaskRegReadInfo.m_crc24Err != globalRegs_p->m_errStatusMaskReg.m_crc24Err) ||
            (devErrStatMaskRegReadInfo.m_ctxBufferParityErr != globalRegs_p->m_errStatusMaskReg.m_ctxBufferParityErr) ||
            (devErrStatMaskRegReadInfo.m_dbSoftError != globalRegs_p->m_errStatusMaskReg.m_dbSoftError) ||
            (devErrStatMaskRegReadInfo.m_dbSoftErrorFifoFull != globalRegs_p->m_errStatusMaskReg.m_dbSoftErrorFifoFull) ||
            (devErrStatMaskRegReadInfo.m_devIdMismatchErr != globalRegs_p->m_errStatusMaskReg.m_devIdMismatchErr) ||
            (devErrStatMaskRegReadInfo.m_eopErr != globalRegs_p->m_errStatusMaskReg.m_eopErr) ||
            (devErrStatMaskRegReadInfo.m_framingCtrlWordErr != globalRegs_p->m_errStatusMaskReg.m_framingCtrlWordErr) ||
            (devErrStatMaskRegReadInfo.m_globalGIO_L0_Enable != globalRegs_p->m_errStatusMaskReg.m_globalGIO_L0_Enable) ||
            (devErrStatMaskRegReadInfo.m_globalGIO_L1_Enable != globalRegs_p->m_errStatusMaskReg.m_globalGIO_L1_Enable) ||
            (devErrStatMaskRegReadInfo.m_illegalInstnErr != globalRegs_p->m_errStatusMaskReg.m_illegalInstnErr) ||
            (devErrStatMaskRegReadInfo.m_instnBurstErr != globalRegs_p->m_errStatusMaskReg.m_instnBurstErr) ||
            (devErrStatMaskRegReadInfo.m_ltrParityErr != globalRegs_p->m_errStatusMaskReg.m_ltrParityErr) ||
            (devErrStatMaskRegReadInfo.m_missingDataPktErr != globalRegs_p->m_errStatusMaskReg.m_missingDataPktErr) ||
            (devErrStatMaskRegReadInfo.m_parityScanFifoOverFlow != globalRegs_p->m_errStatusMaskReg.m_parityScanFifoOverFlow) ||
            (devErrStatMaskRegReadInfo.m_powerLimitingErr != globalRegs_p->m_errStatusMaskReg.m_powerLimitingErr) ||
            (devErrStatMaskRegReadInfo.m_protocolErr != globalRegs_p->m_errStatusMaskReg.m_protocolErr) ||
            (devErrStatMaskRegReadInfo.m_rxNMACFifoParityErr != globalRegs_p->m_errStatusMaskReg.m_rxNMACFifoParityErr) ||
            (devErrStatMaskRegReadInfo.m_rxPCSEFifoParityErr != globalRegs_p->m_errStatusMaskReg.m_rxPCSEFifoParityErr) ||
            (devErrStatMaskRegReadInfo.m_sopErr != globalRegs_p->m_errStatusMaskReg.m_sopErr))
            {
                NlmCmFile__fprintf(logFile, "\n\t => Compare of Read Data Fails :");  
                NlmCmFile__fprintf(logFile, "\n Allignment Err              : %d(%d), Burst ctrl word Err       : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_alignmentErr, globalRegs_p->m_errStatusMaskReg.m_alignmentErr,
                    devErrStatMaskRegReadInfo.m_burstControlWordErr, globalRegs_p->m_errStatusMaskReg.m_burstControlWordErr);
                NlmCmFile__fprintf(logFile, "\n Burst max Err               : %d(%d), Channel number Err        : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_burstMaxErr, globalRegs_p->m_errStatusMaskReg.m_burstMaxErr,
                    devErrStatMaskRegReadInfo.m_channelNumErr, globalRegs_p->m_errStatusMaskReg.m_channelNumErr);
                NlmCmFile__fprintf(logFile, "\n CRC 24    Err               : %d(%d), Cxt buff parity Err       : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_crc24Err, globalRegs_p->m_errStatusMaskReg.m_crc24Err,
                    devErrStatMaskRegReadInfo.m_ctxBufferParityErr, globalRegs_p->m_errStatusMaskReg.m_ctxBufferParityErr);
                NlmCmFile__fprintf(logFile, "\n Database soft Err           : %d(%d), Dbase soft Err FIFO full  : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_dbSoftError, globalRegs_p->m_errStatusMaskReg.m_dbSoftError,
                    devErrStatMaskRegReadInfo.m_dbSoftErrorFifoFull, globalRegs_p->m_errStatusMaskReg.m_dbSoftErrorFifoFull);
                NlmCmFile__fprintf(logFile, "\n DeviceId mismatch Err       : %d(%d), End of Packet Err         : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_devIdMismatchErr, globalRegs_p->m_errStatusMaskReg.m_devIdMismatchErr,
                    devErrStatMaskRegReadInfo.m_eopErr, globalRegs_p->m_errStatusMaskReg.m_eopErr);
                NlmCmFile__fprintf(logFile, "\n Frame ctrl word Err         : %d(%d), Glob GIO_0 enable         : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_framingCtrlWordErr,globalRegs_p->m_errStatusMaskReg.m_framingCtrlWordErr,
                    devErrStatMaskRegReadInfo.m_globalGIO_L0_Enable,globalRegs_p->m_errStatusMaskReg.m_globalGIO_L0_Enable);
                NlmCmFile__fprintf(logFile, "\n Glob GIO_1 enable           : %d(%d), Illigal inst Err          : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_globalGIO_L1_Enable,globalRegs_p->m_errStatusMaskReg.m_globalGIO_L1_Enable,
                    devErrStatMaskRegReadInfo.m_illegalInstnErr, globalRegs_p->m_errStatusMaskReg.m_illegalInstnErr);
                NlmCmFile__fprintf(logFile, "\n Instrcn Burst Err           : %d(%d), LTR parity Err            : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_instnBurstErr, globalRegs_p->m_errStatusMaskReg.m_instnBurstErr,
                    devErrStatMaskRegReadInfo.m_ltrParityErr, globalRegs_p->m_errStatusMaskReg.m_ltrParityErr);
                NlmCmFile__fprintf(logFile, "\n Missing pkt data Err        : %d(%d), Prty scan fifo overflow   : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_missingDataPktErr, globalRegs_p->m_errStatusMaskReg.m_missingDataPktErr,
                    devErrStatMaskRegReadInfo.m_parityScanFifoOverFlow, globalRegs_p->m_errStatusMaskReg.m_parityScanFifoOverFlow);
                NlmCmFile__fprintf(logFile, "\n Power limiting Err          : %d(%d), Protocol ctrl Err        : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_powerLimitingErr, globalRegs_p->m_errStatusMaskReg.m_powerLimitingErr,
                    devErrStatMaskRegReadInfo.m_protocolErr,globalRegs_p->m_errStatusMaskReg.m_protocolErr);
                NlmCmFile__fprintf(logFile, "\n NMAC Fifo parity Err        : %d(%d), PCSE Fifo parity Err      : %d(%d) ",
                    devErrStatMaskRegReadInfo.m_rxNMACFifoParityErr, globalRegs_p->m_errStatusMaskReg.m_rxNMACFifoParityErr,
                    devErrStatMaskRegReadInfo.m_rxPCSEFifoParityErr, globalRegs_p->m_errStatusMaskReg.m_rxPCSEFifoParityErr);
                NlmCmFile__fprintf(logFile, "\n SOP Err                    : %d(%d)",
                    devErrStatMaskRegReadInfo.m_sopErr, globalRegs_p->m_errStatusMaskReg.m_sopErr);

                return(NLMDIAG_ERROR_VERIFY_FAIL);
            }
            break;

        case NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType,
                (void*)&devDBSoftErrStatRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
                return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }
            
            /* Only Erase FIFO and Erase FIFO Entry Field are write only; Rest of the
               field are read only, check remaining with 0 or ignore */
            if((devDBSoftErrStatRegReadInfo.m_errorAddr != 0) ||
                (devDBSoftErrStatRegReadInfo.m_errorAddrValid != 0) ||
                (devDBSoftErrStatRegReadInfo.m_pErrorX != 0) ||
                (devDBSoftErrStatRegReadInfo.m_pErrorY != 0))
            {
                NlmCmFile__fprintf(logFile, "\n\t => Compare of Read Data Fails :");  
                /*Read only */
                NlmCmFile__fprintf(logFile, "\n Error Address  : %u(0), Error Address Valid : %d(0)",
                    devDBSoftErrStatRegReadInfo.m_errorAddr, devDBSoftErrStatRegReadInfo.m_errorAddrValid);
                NlmCmFile__fprintf(logFile, "\n Parity Error X : %d(0), Parity Error Y      : %d(0)",
                    devDBSoftErrStatRegReadInfo.m_pErrorX , devDBSoftErrStatRegReadInfo.m_pErrorY);

                return(NLMDIAG_ERROR_VERIFY_FAIL);
            }
            break;

       case NLMDEV_SCRATCH_PAD0_REG:
       case NLMDEV_SCRATCH_PAD1_REG:                           
           /* call the device manager API to read contents from the specified scratch pad */
           if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, globalRegRdInfo_p->m_globalRegType, 
               (void*)&scratchPadRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
               NlmCmFile__fprintf(logFile, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
               return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
           }

           scratchPadRegInfo_p = 
               &globalRegs_p->m_scratchPadReg[globalRegRdInfo_p->m_globalRegType - NLMDEV_SCRATCH_PAD0_REG];          
           
           /* compare the read data with the actual data */
           if((memcmp(scratchPadRegInfo_p, &scratchPadRegReadInfo, sizeof(NlmDevScratchPadReg)))!=0)
           {
               NlmDiag_CommonPrintData(logFile,scratchPadRegReadInfo.m_data, 0);
               NlmCmFile__fprintf(logFile, ", WRITTEN -> ");
               NlmDiag_CommonPrintData(logFile,scratchPadRegInfo_p->m_data, 0); 

               return(NLMDIAG_ERROR_VERIFY_FAIL);
           }           
           break;

       case NLMDEV_GLOBALREG_END:
       default:
           break;
    }
    return NLMDIAG_ERROR_NOERROR;   
}

nlm_u32 NlmDiag_MaskReserveBitsLtrData(
	nlm_u8 regType,
	nlm_u8 *genData,
	nlm_u8 *expectedData,
	NlmDiag_TestInfo *testInfo_p
	)
{
	NlmDevBlkSelectReg       ltrBlkSelRegInfo;
    NlmDevSuperBlkKeyMapReg  ltrBlkKeyMapRegInfo;
    NlmDevParallelSrchReg	 ltrPrlSrchRegInfo;
    NlmDevRangeInsertion0Reg ltrRangeInst0_RegInfo;
    NlmDevRangeInsertion1Reg ltrRangeInst1_RegInfo;
    NlmDevMiscelleneousReg   ltrMiscRegInfo;
    NlmDevKeyConstructReg    ltrKeyConstructRegInfo;
    NlmDevSSReg              ltrSSregInfo;
	NlmReasonCode reasonCode;

    nlm_u32 errNum;

    /* Check input params */
    if(regType > NLMDEV_LTR_REG_END || genData == NULL || expectedData == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

	/* Make all bits 0, write only ralavent bits from the random data
	   provided */
	NlmCm__memset(expectedData , 0, 10);

    /* Check for Ltr Register type and construct the pattern of data
     *  according to specified register fields */
	switch(regType)
	{
       case NLMDEV_BLOCK_SELECT_0_LTR:
       case NLMDEV_BLOCK_SELECT_1_LTR:  /* Blk select 1 Reg is for future*/
		   	/* Initialize the Ltr Data Structure */

			if((errNum = NlmDiag_ExtractLTRFields(genData,
				regType, (void*)&ltrBlkSelRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Blk select LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Blk select LTR construct",reasonCode);
			}
			if((errNum = NlmDiag_ConstructLTRFields((void*)&ltrBlkSelRegInfo,
				regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Blk select LTR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Blk select LTR extract",reasonCode);
			}
			break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
			/* Initialize the Ltr Data Structure */
			if ((errNum = NlmDiag_ExtractLTRFields(genData,
				regType, (void*)&ltrBlkKeyMapRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, SB Key map LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, SB Key map LTR construct",reasonCode);
			}
			if ((errNum = NlmDiag_ConstructLTRFields((void*)&ltrBlkKeyMapRegInfo,
				regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, SB Key map LTR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, SB Key map LTR extract",reasonCode);
			}
			break;

       case NLMDEV_PARALLEL_SEARCH_0_LTR:
       case NLMDEV_PARALLEL_SEARCH_1_LTR:
       case NLMDEV_PARALLEL_SEARCH_2_LTR:
       case NLMDEV_PARALLEL_SEARCH_3_LTR:
			/* Initialize Block to PS mappings */
			if ((errNum = NlmDiag_ExtractLTRFields(genData,
				regType, (void*)&ltrPrlSrchRegInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Prll srch LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Prll srch LTR construct",reasonCode);
			}
			if ((errNum = NlmDiag_ConstructLTRFields((void*)&ltrPrlSrchRegInfo,
				regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Prll srch LTR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Prll srch LTR extract",reasonCode);
			}
			break;

       case NLMDEV_RANGE_INSERTION_0_LTR:
			/* Initialise the ranges params */
			if ((errNum = NlmDiag_ExtractLTRFields(genData,
										regType, (void*)&ltrRangeInst0_RegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Range 0 LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Range 0 LTR construct",reasonCode);
			}
			if ((errNum = NlmDiag_ConstructLTRFields((void*)&ltrRangeInst0_RegInfo,
				regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Range 0 LTR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Range 0 LTR extract",reasonCode);
			}
			break;


       case NLMDEV_RANGE_INSERTION_1_LTR:
           /* Initialise the ranges params */
			if ((errNum = NlmDiag_ExtractLTRFields(genData,
										regType, (void*)&ltrRangeInst1_RegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Range 1 LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Range 1 LTR construct",reasonCode);
			}
			if ((errNum = NlmDiag_ConstructLTRFields((void*)&ltrRangeInst1_RegInfo,
				regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Range 1 LTR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Range 1 LTR extract",reasonCode);
			}
            break;

       case NLMDEV_MISCELLENEOUS_LTR:
           if ((errNum = NlmDiag_ExtractLTRFields(genData,
										regType, (void*)&ltrMiscRegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Misc LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Misc LTR construct",reasonCode);
		   }
		   if ((errNum = NlmDiag_ConstructLTRFields((void*)&ltrMiscRegInfo,
			   regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
           {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Misc LTR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Misc LTR extract",reasonCode);
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

		   if ((errNum = NlmDiag_ExtractLTRFields(genData,
										regType, (void*)&ltrKeyConstructRegInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, KCR LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, KCR LTR construct",reasonCode);
		    }
			if ((errNum = NlmDiag_ConstructLTRFields((void*)&ltrKeyConstructRegInfo,
				regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Misc KCR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, KCR LTR extract",reasonCode);
		    }
            break;

       case NLMDEV_SS_LTR:
            /* Initialize Internal Reg Mappings*/
            if ((errNum = NlmDiag_ExtractLTRFields(genData,
										regType, (void*)&ltrSSregInfo,
										&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Internal LTR construct",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Internal LTR construct",reasonCode);
		    }
			if ((errNum = NlmDiag_ConstructLTRFields((void*)&ltrSSregInfo,
				regType, expectedData, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr:%d, Internal LTR extract",reasonCode);
				NlmCm__printf ("\n\tErr:%d, Internal LTR extract",reasonCode);
		    }
            break;

       case NLMDEV_LTR_REG_END:
            break;
	}

	return NLMDIAG_ERROR_NOERROR;
}

/*
    This function writes into the Global/LTR registers and reads back the 80b data,
    then written and read 80b data are compared, if both are same
    return zero else return MIS-MATCH, except Read Only Registers.   
 */
nlm_u32 
NlmDiag_RegisterReadWriteTest(NlmDiag_TestInfo *testInfo_p)                       
{  
    nlm_u32 errNum;
    nlm_u8 ptrn, devNum;

    nlm_u8 chnlNum = 0;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
       
    /* Check input params */
    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testParams == NULL || testInfo_p->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;
   
    if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)                   
        return(errNum);   

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;    
    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

    for(devNum = 0; devNum < moduleInfo_p->m_numCascadeDev; devNum++)
    {   
        if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL) 
            return(NLMDIAG_ERROR_INVALID_INPUTPTR);
          
        /* Carry the test for all elevan patterns excluding the compare type pattern.
           for random patterns use the seed value with the command line argument -seed #value 
           If this argument is not provided then use default defined one .i.e.NLMDIAG_DEFAULT_SEED_VALUE */
        ptrn = NLMDIAG_PATTERN_ALT_0S_FS; 
		{
			nlm_u8 flag, ltr;
			nlm_u8 regType = 0;
			nlm_u32 address = 0;
			nlm_u8 genData[10] = {0};
			nlm_u8 *expDat, expectedData[10] = {0};
			nlm_u8 readData[10] = {0};
			NlmReasonCode rsnCode = 0;

			expDat = expectedData;

			for(ltr = 0; ltr < NLMDEV_NUM_LTR_SET; ltr++)
            {            
                for(regType = 0; regType < NLMDEV_LTR_REG_END; regType++)
                {
					/*if(testInfo_p->m_operMode == NLMDEV_OPR_STANDARD && regType == NLMDEV_SS_LTR)
						continue;*/
						
					address = NLM_REG_ADDR_LTR_BLOCK_SELECT0(ltr) + regType;

					/* Generate Specified Data Pattern */
					if((errNum = NlmDiag_CommonGenDataPattern(ptrn, genData, 10, 
						&testInfo_p->m_testParams[0], 0, NULL, NULL))!=NLMDIAG_ERROR_NOERROR)
					return(errNum);

					/* make reserve bits 0 */
					if(( errNum = NlmDiag_MaskReserveBitsLtrData(
						regType, genData, expDat, testInfo_p)) != NLMDIAG_ERROR_NOERROR)
					{
						NlmCm__printf("Err:, constructing LTR Bits \n\n");
							return NLMDIAG_ERROR_VERIFY_FAIL;
					}

                    /* Invoke Device Manager API which writes to specified Global Reg */
					if((errNum = NlmDevMgr__RegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], address,
						(void*)(&expectedData), &rsnCode)) != NLMDIAG_ERROR_NOERROR)
					{
						NlmCm__printf("Err:%d, Dev Mgr Fn: NlmDevMgr__RegisterWrite/LTR",rsnCode);
							return(NLMDIAG_ERROR_INTERNAL_ERR(rsnCode));
					}

					/* Invoke Device Manager API which writes to specified Global Reg */
					if((errNum = NlmDevMgr__RegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], address,
						(void*)(&readData), &rsnCode)) != NLMDIAG_ERROR_NOERROR)
					{
						NlmCm__printf("Err:%d, Dev Mgr Fn: NlmDevMgr__RegisterRead/LTR",rsnCode);
						return(NLMDIAG_ERROR_INTERNAL_ERR(rsnCode));
					}

					if((NlmCm__memcmp(readData, expectedData, 10)) != 0)
						return NLMDIAG_ERROR_VERIFY_FAIL;
                }
            } 
			for(regType = NLMDEV_DEVICE_ID_REG; regType < NLMDEV_GLOBALREG_END; regType++)
			{
				flag = 1;
				switch(regType)
					{
						case NLMDEV_DEVICE_ID_REG:
							address = NLM_REG_ADDR_DEVICE_ID;
							break;
						case NLMDEV_DEVICE_CONFIG_REG:
							address = NLM_REG_ADDR_DEVICE_CONFIG;
							break;
						case NLMDEV_ERROR_STATUS_REG:
							address = NLM_REG_ADDR_ERROR_STATUS;
							break;
						case NLMDEV_ERROR_STATUS_MASK_REG:
							address = NLM_REG_ADDR_ERROR_STATUS_MASK;
							break;
						case NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG:
							address = NLM_REG_ADDR_PARITY_ERROR_FIFO;
							break;
						case NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
							address = NLM_REG_ADDR_DETAILED_PARITY_ERROR_INFO;
							break;
						case NLMDEV_SCRATCH_PAD0_REG:
							address =  NLM_REG_ADDR_SCRATCH_PAD0;
							break;
						case NLMDEV_SCRATCH_PAD1_REG:
							address =  NLM_REG_ADDR_SCRATCH_PAD1;
							break;
						case NLMDEV_RESULT0_REG:
							address = NLM_REG_ADDR_RESULT0;
							break;
						case NLMDEV_RESULT1_REG:
							address = NLM_REG_ADDR_RESULT1;
							break;
						
					}

				/* skip NLMDEV_ERROR_STATUS_REG, this register is not writable */
				if ((regType == NLMDEV_ERROR_STATUS_REG) ||
				    (regType == NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG)) {
				    continue;
				}

                    /* write secified patterns of data into the global registers selected below
					and read it back and compare the content written, return error if MISMATCH */

					/* Generate Specified Data Pattern */
					if((errNum = NlmDiag_CommonGenDataPattern(ptrn, genData, 10, 
						&testInfo_p->m_testParams[0], 0, NULL, NULL))!=NLMDIAG_ERROR_NOERROR)
					return(errNum);					
		            
					/* Invoke Device Manager API which writes to specified Global Reg */
					if((errNum = NlmDevMgr__RegisterWrite(moduleInfo_p->m_dev_ppp[chnlNum][devNum], address,
						(void*)(&genData), &rsnCode)) != NLMDIAG_ERROR_NOERROR)
					{
						if(rsnCode == NLMRSC_READONLY_REGISTER)
							flag = 0;
						else
						{
							NlmCm__printf("Err:%d, Dev Mgr Fn: NlmDevMgr__RegisterWrite/Global",rsnCode);
							return(NLMDIAG_ERROR_INTERNAL_ERR(rsnCode));
						}
					}

					/* Invoke Device Manager API which writes to specified Global Reg */
					if((errNum = NlmDevMgr__RegisterRead(moduleInfo_p->m_dev_ppp[chnlNum][devNum], address,
						(void*)(&readData), &rsnCode)) != NLMDIAG_ERROR_NOERROR)
					{
						NlmCm__printf("Err:%d, Dev Mgr Fn: NlmDevMgr__RegisterRead/Global",rsnCode);
						return(NLMDIAG_ERROR_INTERNAL_ERR(rsnCode));
					}
					
					if(flag)
						if((NlmCm__memcmp(readData, genData, 10)) != 0)
							return NLMDIAG_ERROR_VERIFY_FAIL;
			}
		}
    }
    
    /* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
    if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
        return(errNum);  
    return NLMDIAG_ERROR_NOERROR;    
}


/*  11  */

