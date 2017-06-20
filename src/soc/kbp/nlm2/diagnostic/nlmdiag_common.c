/*
 * $Id: nlmdiag_common.c,v 1.1.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include "nlmdiag_refapp.h"

#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

 /* ========================================================================
                     common function declarations 
    ======================================================================== */

 static NlmDiag_ModuleInfo* NlmDiag_ModuleInfo__ctor(
     NlmDiag_ModuleInfo*              self, 
     void*                               alloc_p, 
     NlmCmFile                           *logFile,
     nlm_u32                             numOfChips
     );

  static nlm_u32 NlmDiag_TestTypeInfo__ctor(
     NlmDiag_TestInfo                  *self,
     void                                 *alloc_p,
     NlmDiag_TestCaseInfo              *testCaseInfo_p
     );
 
 static void NlmDiag_TestTypeInfo__dtor(
     NlmDiag_TestInfo                  *self
     );
 
 static void NlmDiag_ModuleInfo__dtor(
     NlmDiag_ModuleInfo               *self
     );      

 static nlm_u32 NlmDiag_CommonGenRandomPattern(
    nlm_u8                          *data,
    nlm_u32                         datalen,
    nlm_u32                         *seed_p
    );

/* ============================================================================
           Diagnostic common Initialize and Destroy functions 
   ============================================================================ */

/* 
    The function collects the test based information by calling constructor.
    Internally constructor allocates the memory for the testInfo structure, 
    and also allocates memory for module information structure. 
 */
nlm_u32  
NlmDiag_CommonInitTestInfo(NlmCmAllocator* alloc_p,
                              NlmDiag_TestInfo *testInfo_ptr,
                              NlmDiag_TestCaseInfo* testCaseInfo_p
                              )
{ 
    nlm_u32 errNum;
    
    NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    NlmCmAssert((testInfo_ptr != NULL), "Invalid Test Info pointer.\n");
    NlmCmAssert((testCaseInfo_p != NULL), "Invalid Test Info pointer.\n");
    if (alloc_p == NULL || testInfo_ptr == NULL || testCaseInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if ((errNum = NlmDiag_TestTypeInfo__ctor(testInfo_ptr, alloc_p, testCaseInfo_p)) != 0)
    {
        NlmCm__printf("\n\n\n\tConstructor function failed (Memory Error).\n");
        NlmCmFile__fprintf(testInfo_ptr->m_testLogFile, "\n\tConstructor function failed (Memory Error).\n");
        return NLMDIAG_ERROR_MEMORY_ERR;
    }

    return NLMDIAG_ERROR_NOERROR;
}


/* 
    The function frees memory test based information by calling destructor.
    Internally destructor deallocates the memory for the testInfo structure, 
    and also deallocates memory for module information structure. 
 */
nlm_u32 
NlmDiag_CommonDestroyTestInfo(NlmDiag_TestInfo* self)
{
    NlmCmAssert((self != NULL), "Invalid TestBlksInfo_p.\n");
    NlmCmAssert((self->m_alloc_p != NULL), "Invalid memory allocator in Release.\n");
    NlmCmAssert((self->m_testModuleInfo_p != NULL), "Invalid module information pointer.\n");
    if (self == NULL || self->m_alloc_p == NULL || self->m_testModuleInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    NlmDiag_TestTypeInfo__dtor(self);

    return NLMDIAG_ERROR_NOERROR;    
}

/*==================================================================================
                       constructor and destructor definitions
  ==================================================================================*/

/* 
    This function initializes the diagnostic test type information, by calling the
    test module information constructor. 
 */
nlm_u32
NlmDiag_TestTypeInfo__ctor(NlmDiag_TestInfo* self, 
                             void* alloc_p,
                             NlmDiag_TestCaseInfo* testCaseInfo_p
                             )
{   
    NlmDiag_ModuleInfo *mdlInfo = NULL;

    NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    NlmCmAssert((self != NULL), "Invalid self pointer.\n");
    NlmCmAssert((testCaseInfo_p != NULL), "Invalid Test Info pointer.\n");
    if (alloc_p == NULL || self == NULL || testCaseInfo_p == NULL)
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(testCaseInfo_p->m_numOfChips < NLM_DIAG_MIN_NUM_DEV_CASCADE || 
            testCaseInfo_p->m_numOfChips > NLM_DIAG_MAX_NUM_DEV_CASCADE)
        return NLMDIAG_ERROR_INVALID_INPUT;
    
    self->m_alloc_p = alloc_p;    
    self->m_testParams = testCaseInfo_p->m_testParams;
    self->m_testModuleInfo_p = NULL; 
    self->m_testType = testCaseInfo_p->m_testType;
    self->m_operMode  = testCaseInfo_p->m_operMode;
    self->m_numOfChips = testCaseInfo_p->m_numOfChips;  
    self->m_xptMode = testCaseInfo_p->m_xptMode;
	
	if (testCaseInfo_p->m_isInteractive)
	{
		self->m_userInput_p = &testCaseInfo_p->m_userInput;
	}

    /* Allocate memory to hold the module information */
    mdlInfo = NlmCmAllocator__malloc(self->m_alloc_p, sizeof(NlmDiag_ModuleInfo));
    NlmCmDemand((mdlInfo != NULL), "Out of memory.\n");
    if (mdlInfo == NULL)
        return NLMDIAG_ERROR_MEMORY_ERR;
    
    /* Call constructor to fill the module information */
    self->m_testModuleInfo_p = 
        (void*)NlmDiag_ModuleInfo__ctor(mdlInfo, alloc_p, self->m_testLogFile, self->m_numOfChips); 

    return NLMDIAG_ERROR_NOERROR;
}

/* 
    Constructor for the diagnostic module information 
 */
NlmDiag_ModuleInfo* 
NlmDiag_ModuleInfo__ctor(NlmDiag_ModuleInfo* self, 
                           void* alloc_p, 
                           NlmCmFile *logFile,
                           nlm_u32 numOfChips
                           )
{   
    NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    NlmCmAssert((self != NULL), "Invalid self pointer.\n");
    NlmCmAssert((logFile != NULL), "Invalid logFile \n");
    if (alloc_p == NULL || self == NULL || logFile == NULL)
        return NULL;

    if(numOfChips <= 0 || numOfChips > 4)
        return NULL;
 
    /* Fill the default module information */
    self->m_alloc_p = alloc_p;    
    self->m_dev_ppp = NULL;
    self->m_devMgr_pp = NULL;
    self->m_xptPtr_pp = NULL; 
    self->m_cascade_pp = NULL;
    self->m_numCascadeDev = numOfChips;
    self->m_numXptChnl = NLM_DIAG_NUM_XPT_CHANNEL;  

    return self;	
}

/* 
    This function de-initializes the diagnostic test type information, by calling the
    test module information destructor.
 */
void 
NlmDiag_TestTypeInfo__dtor(NlmDiag_TestInfo* self)
{
    NlmCmAssert((self != NULL), "Invalid self pointer.\n");
    NlmCmAssert((self->m_alloc_p != NULL) , "Invalid memory allocator. \n");  
    NlmCmAssert((self->m_testModuleInfo_p != NULL), "Invalid module pointer.\n");
    if (self->m_alloc_p == NULL || self == NULL || self->m_testModuleInfo_p == NULL)
    {
        printf("\n\t Invalid parameters: NlmDiag_TestTypeInfo__dtor() return\n");
        return;
    }
    
    /* Call the destructor to free the module information */
    NlmDiag_ModuleInfo__dtor(self->m_testModuleInfo_p); 
    NlmCmAllocator__free(self->m_alloc_p, self->m_testModuleInfo_p);      
#ifndef NLMPLATFORM_BCM
#ifndef NLM_XLP
    NlmCmFile__fclose(self->m_testLogFile);
#endif
#endif
}

/*   Destructor for the diagnostic test module information  */
void 
NlmDiag_ModuleInfo__dtor(NlmDiag_ModuleInfo* self)
{
    (void)self;
}



/* ============================================================================
           Diagnostic test Initialize and Destroy functions 
   ============================================================================ */

/* 
    Init(): Initializes the Device by allocating the memory for the transport layer (xpt),
            device manager, and device by calling the appropreate API functions.
 */
nlm_u32 
NlmDiag_Initialize(NlmDiag_TestInfo *testInfo_p)
{
    nlm_u8 devCount;   
    nlm_u32 errNum;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;
    nlm_u8 chnlNum;
    NlmReasonCode reasonCode;
	nlm_u32 deviceType = NLM_DEVTYPE_2;

    if(testInfo_p == NULL || testInfo_p->m_testModuleInfo_p == NULL 
        || testInfo_p->m_testLogFile == NULL)
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;
   
    /* operational mode of the device is spevicified here */
    moduleInfo_p->m_operMode = testInfo_p->m_operMode;
    moduleInfo_p->m_numCascadeDev = testInfo_p->m_numOfChips;

    /* allocate memory for device, cascaded device, device manager, and xpt */
    moduleInfo_p->m_xptPtr_pp = NlmCmAllocator__malloc(
                    testInfo_p->m_alloc_p, moduleInfo_p->m_numXptChnl * sizeof(NlmXpt*));
    moduleInfo_p->m_cascade_pp = NlmCmAllocator__malloc(
                    testInfo_p->m_alloc_p, moduleInfo_p->m_numXptChnl * sizeof(NlmDev*));
    moduleInfo_p->m_devMgr_pp = NlmCmAllocator__malloc(
                    testInfo_p->m_alloc_p, moduleInfo_p->m_numXptChnl * sizeof(NlmDevMgr*));
    moduleInfo_p->m_dev_ppp = NlmCmAllocator__malloc(
                    testInfo_p->m_alloc_p, moduleInfo_p->m_numXptChnl * sizeof(NlmDev**));

	if(testInfo_p->m_operMode == NLMDEV_OPR_SAHASRA)
		deviceType = NLM_DEVTYPE_2_S;

    for(chnlNum = 0; chnlNum < moduleInfo_p->m_numXptChnl; chnlNum++)
    {
        /* Create the Transport intrface.
         * It will creates handle in the specified operation mode, and the device manager 
         * is flushing out Immediately (one request at time) */

        switch(testInfo_p->m_xptMode)
        {
#ifndef NLMPLATFORM_BCM
            case NLM_SIMULATION_MODE:
                if((moduleInfo_p->m_xptPtr_pp[chnlNum] = NlmSimXpt__Create(moduleInfo_p->m_alloc_p, 
                    deviceType, 0, NLM_DIAG_XPT_MAX_RQT_COUNT, NLM_DIAG_XPT_MAX_RSLT_COUNT, 
                    testInfo_p->m_operMode, 0, chnlNum)) == NULL)
                {
                    NlmCm__printf( "\n\tError: NlmSimXpt__Create");
                    return NLMDIAG_ERROR_XPT_HANDLE_CREATE_ERR;
                }
                break;
			case NLM_FPGAXPT_MODE:
				{
					NlmCm__printf("\n\n\t Currently NLM_FPGAXPT_MODE xpt mode not supported\n\n");
					exit(1);
				}

#ifdef NLM_XLP
            case NLM_XLPXPT_MODE:
		if((moduleInfo_p->m_xptPtr_pp[chnlNum] = NlmXlpXpt__Create(moduleInfo_p->m_alloc_p, 
		                    deviceType, NLM_DIAG_XPT_MAX_RQT_COUNT,  
		                    testInfo_p->m_operMode, chnlNum, 0, 0, 1,1)) == NULL)
                {
                    NlmCm__printf("\n\tError: NlmXlpXpt__Create");
                    return NLMDIAG_ERROR_XPT_HANDLE_CREATE_ERR;
                }
                break;
#endif
#endif
            case NLM_BCM_CALADAN3_MODE:
		if((moduleInfo_p->m_xptPtr_pp[chnlNum] = soc_sbx_caladan3_etu_xpt_create(0,
		                                              deviceType, 0, NLM_DIAG_XPT_MAX_RQT_COUNT,  
		                                              testInfo_p->m_operMode, chnlNum)) == NULL)
                {
                    NlmCm__printf("\n\tError: soc_sbx_caladan3_etu_xpt_create failed");
                    return NLMDIAG_ERROR_XPT_HANDLE_CREATE_ERR;
                }
                break;
            default:
                NlmCm__printf("\n\tInvalid Xpt mode specified\n");
                return NLMDIAG_ERROR_XPT_HANDLE_CREATE_ERR;
                break;
        }
        
        /* Create Device Manager by calling device manager API, and it is operating in
         * Standard/Sahasra operation mode and specified speed interface mode */
        if((moduleInfo_p->m_devMgr_pp[chnlNum] = NlmDevMgr__create(moduleInfo_p->m_alloc_p,
			moduleInfo_p->m_xptPtr_pp[chnlNum], moduleInfo_p->m_operMode, &reasonCode)) == NULL)
        {
            NlmCm__printf( "\n\tErr:%d NlmDevMgr__create", reasonCode);
            return (NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }        

        /* create memory allocator for the cascaded device */
        moduleInfo_p->m_dev_ppp[chnlNum] = NlmCmAllocator__malloc(moduleInfo_p->m_alloc_p,
                moduleInfo_p->m_numCascadeDev*sizeof(NlmDev*));
           
        /* One device is added to search sub system by calling the Add_device API*/
        for(devCount = 0; devCount < moduleInfo_p->m_numCascadeDev; devCount++)
        {
			NlmDevId deviceId;
            moduleInfo_p->m_dev_ppp[chnlNum][devCount] = NULL; 
            if((moduleInfo_p->m_dev_ppp[chnlNum][devCount] = NlmDevMgr__AddDevice(
                moduleInfo_p->m_devMgr_pp[chnlNum], &deviceId, &reasonCode)) == NULL)
            {
                NlmCm__printf( 
                        "\n\tErr %d Dev Mgr Fn: NlmDevMgr__AddDevice", reasonCode);
                return (NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
            }          
        }

        /* The code below locks the Device manager configuration. Once Device manager is locked
         * no more devices can be added and the device becomes ready for read/write/compare
         * operations. Before locking the configuration no operation can be done on the device. */

        if((errNum = NlmDevMgr__LockConfig(moduleInfo_p->m_devMgr_pp[chnlNum], &reasonCode))
            != NLMDIAG_ERROR_NOERROR)
        {
            NlmCm__printf( 
                    "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__LockConfig",reasonCode);
            return(NLMDIAG_ERROR_INTERNAL_ERR(reasonCode));
        }
    }
    return NLMDIAG_ERROR_NOERROR;
}


/* 
    DeInit(): DeInitializes (destroy) the Device by deallocating the memory for
              the transport layer (xpt), device manager, and device by calling 
              the appropreate API functions.
 */
nlm_u32 
NlmDiag_Destroy(NlmDiag_TestInfo *testInfo_p)
{
    nlm_u8 chnlNum;
    NlmDiag_ModuleInfo *moduleInfo_p = NULL;

    if(testInfo_p == NULL || testInfo_p->m_alloc_p == NULL 
        || testInfo_p->m_testLogFile == NULL
        || testInfo_p->m_testModuleInfo_p == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

    moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;

    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_devMgr_pp == NULL 
        || moduleInfo_p->m_xptPtr_pp == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);

    for(chnlNum =0; chnlNum < moduleInfo_p->m_numXptChnl; chnlNum++)
    {      
        /* Destroy Device Manager */
        NlmDevMgr__destroy(moduleInfo_p->m_devMgr_pp[chnlNum]);

        /* free the memory allocated for the diagnostic module information */
        NlmCmAllocator__free(testInfo_p->m_alloc_p,moduleInfo_p->m_dev_ppp[chnlNum]);            
                  
        /* Destroy Xpt */
        switch(testInfo_p->m_xptMode)
        {
#ifndef NLMPLATFORM_BCM
            case NLM_SIMULATION_MODE:
                NlmSimXpt__destroy(moduleInfo_p->m_xptPtr_pp[chnlNum]);
                break;
	
#ifdef NLM_XLP
            case NLM_XLPXPT_MODE:
                NlmXlpXpt__Destroy(moduleInfo_p->m_xptPtr_pp[chnlNum]);
				break;
#endif
            case NLM_FPGAXPT_MODE:
	        NlmCm__printf("\n\n\t Currently NLM_FPGAXPT_MODE xpt mode not supported\n\n");
		exit(1);
#endif
            default:
                break;
        }
    }
    
    /* free the all pointers */
    NlmCmAllocator__free(testInfo_p->m_alloc_p,moduleInfo_p->m_dev_ppp);    
    NlmCmAllocator__free(testInfo_p->m_alloc_p,moduleInfo_p->m_cascade_pp);  
    NlmCmAllocator__free(testInfo_p->m_alloc_p,moduleInfo_p->m_devMgr_pp);    
    NlmCmAllocator__free(testInfo_p->m_alloc_p,moduleInfo_p->m_xptPtr_pp);               
    return NLMDIAG_ERROR_NOERROR;
}


/*
    ==================================================================================
                                common functions 
    ==================================================================================
    This function generates the data or mask key of specified data pattern, if the
    random type specified then it uses seed to generate the random numbers. 
 */
nlm_u32
NlmDiag_CommonGenDataPattern(NlmDiag_DataPattern patternType, 
                                nlm_u8 *data,
                                nlm_u32 datalen,
                                nlm_u32 *seed_p,
                                nlm_u32 flag,
                                void *anyInfo_p,
                                NlmCmFile *logFile
                                )
{
    nlm_u32  index;

     /* Check input params */
    if(data == NULL) 
        return NLMDIAG_ERROR_INVALID_INPUTPTR;

    if(patternType >= NLMDIAG_PATTERN_END || datalen == 0)
        return NLMDIAG_ERROR_INVALID_INPUT;
   
    (void)flag;
    (void)anyInfo_p;
    (void)logFile;

    switch(patternType)
    {
        case NLMDIAG_PATTERN_ALL_0S: NlmCm__memset(data, 0, datalen); 
             break;

        case NLMDIAG_PATTERN_ALL_FS: NlmCm__memset(data, 0xFF, datalen); 
             break;

        case NLMDIAG_PATTERN_ALL_5S: NlmCm__memset(data, 0x55, datalen); 
             break;

        case NLMDIAG_PATTERN_ALL_AS: NlmCm__memset(data, 0xAA, datalen); 
             break;                     

        case NLMDIAG_PATTERN_ALT_0S_FS: 
             for(index = 0; index <datalen; index++)                   
                if((index/10)%2)      
                    data[index] = 0xFF;
                else
                    data[index] = 0x00;
             break;

        case NLMDIAG_PATTERN_ALT_AS_5S:
             for(index = 0; index <datalen; index++)
                if((index/10)%2)     
                    data[index] = 0xAA;
                else
                    data[index] = 0x55;
             break;  

        case NLMDIAG_PATTERN_HALF_0S_FS: 
            for(index=0;index<datalen;index++)
                if((index/5)%2)    
                    data[index] = 0xFF;
                else               
                    data[index] = 0x00;
            break;

        case NLMDIAG_PATTERN_HALF_AS_5S:
            for(index=0;index<datalen;index++)
                if((index/5)%2)    
                    data[index] = 0xAA;
                else               
                    data[index] = 0x55;
            break;

       case NLMDIAG_PATTERN_CHECKERBOARD_OSFS: 
            for(index = 0; index <datalen; index++)
                if(index%2)   
                    data[index] = 0xFF;
                else          
                    data[index] = 0x00;                    
            break;                    

       case NLMDIAG_PATTERN_CHECKERBOARD_AS5S: 
            for(index = 0; index <datalen; index++)
                if(index%2)   
                    data[index] = 0xAA;
                else          
                    data[index] = 0x55;
            break;         

       /* Invoke Function that generates the random pattern for particular seed */
       case NLMDIAG_PATTERN_RANDOM: 
            NlmDiag_CommonGenRandomPattern(data, datalen, seed_p); 
            break;
            
       default:
            break;
    }        
    return NLMDIAG_ERROR_NOERROR;
}

/* 
    This function will generate the random number and copies byte at time into 
    the key (data/mask) at time, for the specified data-length
 */

nlm_u32 
NlmDiag_CommonGenRandomPattern(nlm_u8 *data,
                                  nlm_u32 datalen,
                                  nlm_u32 *seed_p
                                  )
{
    nlm_u32 index;    
    nlm_u8 count=0;
 
    for(index=0;index<datalen;index++)
    {
        if((index % 4) == 0) 
        {
            count = 0;
            srand(*seed_p);
            *seed_p = ((rand() << 16)|rand());
        }
        data[index] = ((nlm_u8*)seed_p)[count++];					
	}    
    return NLMDIAG_ERROR_NOERROR;
}

/* 
    Generic print template for the printing data key or mask key 
 */
void
NlmDiag_CommonPrintData(NlmCmFile *fp,
                           nlm_u8 *p_data,
                           nlm_u16 offset
                           )
{
    NlmCmFile__fprintf(fp,"0x%02x%02x_%02x%02x%02x%02x_%02x%02x%02x%02x\t", 
        p_data[offset+9], p_data[offset+8], 
        p_data[offset+7], p_data[offset+6], p_data[offset+5], p_data[offset+4], 
        p_data[offset+3], p_data[offset+2], p_data[offset+1], p_data[offset+0]);
}


/* 
   This function reads/ compares the LTR registers (read data with the written data)
   depending on the flag type, if READ flag set, compares the read data with the written, 
   if data DIFFERS then print (both read and written are not same) the expected data. 
 */
nlm_u32 
NlmDiag_CompareLtrRegData(NlmDevLtrRegType ltrRegType,
										  nlm_u8 profileNum, 
										  NlmDiag_WriteReadFlag rdWrFlag,
										  void *regData, 
										  void *readData, 
										  NlmCmFile *logFile
										  )
{
    /* Structure pointers to hold the specific LTR registers read information */
    NlmDevBlkSelectReg       *ltrBlkSelRegInfo_p = NULL;
    NlmDevSuperBlkKeyMapReg  *ltrBlkKeyMapRegInfo_p = NULL;
    NlmDevParallelSrchReg	 *ltrPrlSrchRegInfo_p = NULL;
    NlmDevRangeInsertion0Reg *ltrRangeInst0_RegInfo_p = NULL;
    NlmDevRangeInsertion1Reg *ltrRangeInst1_RegInfo_p = NULL;
    NlmDevMiscelleneousReg   *ltrMiscRegInfo_p = NULL;
    NlmDevKeyConstructReg    *ltrKeyConstructRegInfo_p = NULL;
    NlmDevSSReg				 *ltrSSRegInfo_p = NULL;

    NlmDevBlkSelectReg       *ltrBlkSelRegReadInfo_p = NULL;
    NlmDevSuperBlkKeyMapReg  *ltrBlkKeyMapRegReadInfo_p = NULL;
    NlmDevParallelSrchReg	 *ltrPrlSrchRegReadInfo_p = NULL;
    NlmDevRangeInsertion0Reg *ltrRangeInst0_RegReadInfo_p = NULL;
    NlmDevRangeInsertion1Reg *ltrRangeInst1_RegReadInfo_p = NULL;
    NlmDevMiscelleneousReg   *ltrMiscRegReadInfo_p = NULL;
    NlmDevKeyConstructReg    *ltrKeyConstructRegReadInfo_p = NULL;
    NlmDevSSReg              *ltrSSRegReadInfo_p = NULL;
    nlm_u8 i, j;
    nlm_u8 loop1, loop2;

    switch(ltrRegType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
            ltrBlkSelRegInfo_p = (NlmDevBlkSelectReg*)regData;
            profileNum = profileNum;
                      
            if(rdWrFlag == NLMDIAG_READ_FLAG)
            {                   
                ltrBlkSelRegReadInfo_p = (NlmDevBlkSelectReg*)readData;  
                /* Compare the read data with the shadow chip memory data */
                if(memcmp(ltrBlkSelRegInfo_p, ltrBlkSelRegReadInfo_p,
                    sizeof(NlmDevBlkSelectReg)) != 0)
                {                    
                    /* If compare fails display the read data and expected data */
                    NlmCmFile__fprintf(logFile, "  Compare of Read Data Fail \n\t   Read Data\n\t\t");  
                    for(loop1=0; loop1<NLMDEV_NUM_ARRAY_BLOCKS/4; loop1++)
                    {
                        for(loop2=0; loop2<2; loop2++)
                        {
                            NlmCmFile__fprintf(logFile, "  Blk Num: [%2d] Enbl Bit-> %d",
                                loop1*2+loop2, ltrBlkSelRegInfo_p->m_blkEnable[loop1*2+loop2]);
                        }
                        NlmCmFile__fprintf(logFile, "\n\t\t");  
                    }
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }               
            }
            break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:

            ltrPrlSrchRegInfo_p = (NlmDevParallelSrchReg*)regData;
                        
            if(rdWrFlag == NLMDIAG_READ_FLAG)
            {            
                ltrPrlSrchRegReadInfo_p = (NlmDevParallelSrchReg*)readData;
                /* compare the read data with the shadow chip memory data */
                if(memcmp(ltrPrlSrchRegInfo_p, ltrPrlSrchRegReadInfo_p,
                    sizeof(NlmDevParallelSrchReg)) != 0)
                {
                    /* If compare fails display the read data and expected data */
                     
                    NlmCmFile__fprintf(logFile, "\n\tCompare of Read Data Fail ; Data Read\n\t\t");
                    
                    for(loop1=0;loop1<NLMDEV_NUM_ARRAY_BLOCKS/4;loop1++)
                    {
                        NlmCmFile__fprintf(logFile, "\n\t  Block Num: [%d] PS --> %d(%d)", loop1,
                            ltrPrlSrchRegReadInfo_p->m_psNum[loop1] ,ltrPrlSrchRegInfo_p->m_psNum[loop1]);                       NlmCmFile__fprintf(logFile, "\n\t\t"); 
                    }                                                    
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }
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
            ltrKeyConstructRegInfo_p = (NlmDevKeyConstructReg*)regData;            
            
            if(rdWrFlag == NLMDIAG_READ_FLAG)
            { 
                nlm_u8 seg = 0;
                ltrKeyConstructRegReadInfo_p = (NlmDevKeyConstructReg*)readData;
                /* Compare the read LTR data with the shadow chip memory LTR data */
                for(seg = 0;seg < NLMDEV_NUM_OF_SEGMENTS_PER_KCR;seg++)
                {
                    if(ltrKeyConstructRegInfo_p->m_numOfBytes[seg] != ltrKeyConstructRegReadInfo_p->m_numOfBytes[seg] ||
                        ltrKeyConstructRegInfo_p->m_startByteLoc[seg] != ltrKeyConstructRegReadInfo_p->m_startByteLoc[seg])
                    {
                        nlm_u8 segments = 0;
                        
                        /* If compare fails display the read data and expected data */
                        NlmCmFile__fprintf(logFile, "\n\t   Compare of Read Data Fail; Data Read : ");

                        for(segments = 0;segments < NLMDEV_NUM_OF_SEGMENTS_PER_KCR;segments++)
                        {
                            /* Valid values for BMR Select is 0 - 4 and NLMDEV_NO_MASK_BMR_NUM */
                            NlmCmFile__fprintf(logFile,"\n\t Segment:%d  -> Number of Bytes: %d(%d),  Segment Location: %d(%d) ", segments,
                                ltrKeyConstructRegReadInfo_p->m_numOfBytes[segments], 
                                ltrKeyConstructRegInfo_p->m_numOfBytes[segments],
                                ltrKeyConstructRegReadInfo_p->m_startByteLoc[segments], 
                                ltrKeyConstructRegInfo_p->m_startByteLoc[segments]);      
                        }
                        return NLMDIAG_ERROR_VERIFY_FAIL;
                    }
                }                
            }
            break;

        case NLMDEV_MISCELLENEOUS_LTR:
            ltrMiscRegInfo_p = (NlmDevMiscelleneousReg*)regData;

            if(rdWrFlag == NLMDIAG_READ_FLAG)
            {
                nlm_u32 ps = 0;
                nlm_u32 psNum = 0;
                ltrMiscRegReadInfo_p = (NlmDevMiscelleneousReg*)readData;

                /* Compare the read data with the shadow chip memory data */
                for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
                {
                    /* If fails display the read data and expected data */
                    if(ltrMiscRegInfo_p->m_bmrSelect[psNum] != ltrMiscRegReadInfo_p->m_bmrSelect[psNum])
                    {
                        /* If fails display the read data and expected data */
                        NlmCmFile__fprintf(logFile,"\n\t Compare of Read Data Fail");
                        for(ps = 0; ps < NLMDEV_NUM_PARALLEL_SEARCHES; ps++)
                        {
                            NlmCmFile__fprintf(logFile,"\n\t PS Num:%d  -> BMR Sel Num: %d(%d) ", ps,
                                ltrMiscRegInfo_p->m_bmrSelect[ps], ltrMiscRegReadInfo_p->m_bmrSelect[ps]);
                        }
                        NlmCmFile__fprintf(logFile,"\n\t RangeA Extr Byte: %d(%d), RangeB Extr Byte: %d(%d), RangeC Extr Byte: %d(%d), RangeD Extr Byte: %d(%d)",
                            ltrMiscRegReadInfo_p->m_rangeAExtractStartByte, ltrMiscRegInfo_p->m_rangeAExtractStartByte,
                            ltrMiscRegReadInfo_p->m_rangeBExtractStartByte, ltrMiscRegInfo_p->m_rangeBExtractStartByte,
                            ltrMiscRegReadInfo_p->m_rangeCExtractStartByte, ltrMiscRegInfo_p->m_rangeCExtractStartByte,
                            ltrMiscRegReadInfo_p->m_rangeDExtractStartByte, ltrMiscRegInfo_p->m_rangeDExtractStartByte);
                        
                        NlmCmFile__fprintf(logFile,"\n\t Valid search results: %d(%d)",
                            ltrMiscRegReadInfo_p->m_numOfValidSrchRslts, ltrMiscRegInfo_p->m_numOfValidSrchRslts);
                        NlmCmFile__fprintf(logFile,"\n\n");
                        return NLMDIAG_ERROR_VERIFY_FAIL;
                    }
                }

                /* compare other LTR data */
                if( (ltrMiscRegInfo_p->m_rangeAExtractStartByte != ltrMiscRegReadInfo_p->m_rangeAExtractStartByte) ||
                    (ltrMiscRegInfo_p->m_rangeBExtractStartByte != ltrMiscRegReadInfo_p->m_rangeBExtractStartByte) ||
                    (ltrMiscRegInfo_p->m_rangeCExtractStartByte != ltrMiscRegReadInfo_p->m_rangeCExtractStartByte) ||
                    (ltrMiscRegInfo_p->m_rangeDExtractStartByte != ltrMiscRegReadInfo_p->m_rangeDExtractStartByte) ||
                    (ltrMiscRegInfo_p->m_numOfValidSrchRslts != ltrMiscRegReadInfo_p->m_numOfValidSrchRslts)
                    )
                {
                    /* If compare fails display the read data and expected data */
                    NlmCmFile__fprintf(logFile,"\n\t Compare of Read Data Fail");
                    for(ps = 0; ps < NLMDEV_NUM_PARALLEL_SEARCHES; ps++)
                    {
                        NlmCmFile__fprintf(logFile,"\n\t PS Num:%d  -> BMR Sel Num: %d(%d) ", ps,
                            ltrMiscRegInfo_p->m_bmrSelect[ps], ltrMiscRegReadInfo_p->m_bmrSelect[ps]);
                    }
                    NlmCmFile__fprintf(logFile,"\n\t RangeA Extr Byte: %d(%d), RangeB Extr Byte: %d(%d), RangeC Extr Byte: %d(%d), RangeD Extr Byte: %d(%d)",
                        ltrMiscRegReadInfo_p->m_rangeAExtractStartByte, ltrMiscRegInfo_p->m_rangeAExtractStartByte,
                        ltrMiscRegReadInfo_p->m_rangeBExtractStartByte, ltrMiscRegInfo_p->m_rangeBExtractStartByte,
                        ltrMiscRegReadInfo_p->m_rangeCExtractStartByte, ltrMiscRegInfo_p->m_rangeCExtractStartByte,
                        ltrMiscRegReadInfo_p->m_rangeDExtractStartByte, ltrMiscRegInfo_p->m_rangeDExtractStartByte);
                    
                    NlmCmFile__fprintf(logFile,"\n\t Valid search results: %d(%d)",
                        ltrMiscRegReadInfo_p->m_numOfValidSrchRslts, ltrMiscRegInfo_p->m_numOfValidSrchRslts);
                    NlmCmFile__fprintf(logFile,"\n\n");
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }                
            }
            break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
            ltrBlkKeyMapRegInfo_p = (NlmDevSuperBlkKeyMapReg*)regData;   
            
            if(rdWrFlag == NLMDIAG_READ_FLAG)
            {
                ltrBlkKeyMapRegReadInfo_p = (NlmDevSuperBlkKeyMapReg*)readData;

                if(NlmCm__memcmp(ltrBlkKeyMapRegInfo_p, ltrBlkKeyMapRegReadInfo_p,
                        sizeof(NlmDevSuperBlkKeyMapReg)) != 0)
                {
                    /* If compare fails display the read data and expected data */
                    NlmCmFile__fprintf(logFile,"\n\t Compare of Read Data Fail");
                    for(i = 0;i < NLMDEV_NUM_SUPER_BLOCKS/4;i++)
                    {
                        for(j = 0;j < 4;j++)
                        {
                            NlmCmFile__fprintf(logFile,"\n\t SB Num: [%d] KeyNum --> %d (%d)", i*4+j,
                               ltrBlkKeyMapRegReadInfo_p->m_keyNum[i*4+j],
                               ltrBlkKeyMapRegInfo_p->m_keyNum[i*4+j]);
                        }
                        NlmCmFile__fprintf(logFile,"\n");
                    }
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }
            }
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
            ltrRangeInst0_RegInfo_p = (NlmDevRangeInsertion0Reg*)regData;
            
            if(rdWrFlag == NLMDIAG_READ_FLAG)      
            {
                ltrRangeInst0_RegReadInfo_p = (NlmDevRangeInsertion0Reg*)readData;
                /* compare the read data with the shadow chip memory data */
                if(memcmp(ltrRangeInst0_RegInfo_p, ltrRangeInst0_RegReadInfo_p,
                    sizeof(NlmDevRangeInsertion0Reg)) != 0)
                {
                    /* If compare fails display the read data and expected data */
                    NlmCmFile__fprintf(logFile,"\n\t Compare of Read Data Fail");
                    
                    NlmCmFile__fprintf(logFile,"\n\t Range Insert LTR 0: RangeA Encoding Type :%d (%d)  RangeB Encoding Type:%d (%d)",
                    ltrRangeInst0_RegReadInfo_p->m_rangeAEncodingType, ltrRangeInst0_RegInfo_p->m_rangeAEncodingType,
                    ltrRangeInst0_RegReadInfo_p->m_rangeBEncodingType, ltrRangeInst0_RegInfo_p->m_rangeBEncodingType);
                    
                    NlmCmFile__fprintf(logFile,"\n\t Range Insert LTR 0: RangeA Encoding Byte :%d (%d)  RangeB Encoding Byte:%d (%d)",
                    ltrRangeInst0_RegReadInfo_p->m_rangeAEncodedBytes, ltrRangeInst0_RegInfo_p->m_rangeAEncodedBytes,
                    ltrRangeInst0_RegReadInfo_p->m_rangeBEncodedBytes, ltrRangeInst0_RegInfo_p->m_rangeBEncodedBytes);
                    
                    NlmCmFile__fprintf(logFile,"\n\t Range Insert LTR 0: RangeA Insert Start Byte :  RangeB Insert Start Byte:");
                    
                    for(i = 0; i < NLMDEV_NUM_KEYS; i++)
                    {
                        NlmCmFile__fprintf(logFile,"\n\t                 %d(%d)             :  %d(%d)",
                            ltrRangeInst0_RegReadInfo_p->m_rangeAInsertStartByte[i], ltrRangeInst0_RegInfo_p->m_rangeAInsertStartByte[i],
                            ltrRangeInst0_RegReadInfo_p->m_rangeBInsertStartByte[i], ltrRangeInst0_RegInfo_p->m_rangeBInsertStartByte[i]);
                    }
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }
            }
            break;

        case NLMDEV_RANGE_INSERTION_1_LTR:
            ltrRangeInst1_RegInfo_p = (NlmDevRangeInsertion1Reg*)regData;
            
            if(rdWrFlag == NLMDIAG_READ_FLAG)                      
            {
                ltrRangeInst1_RegReadInfo_p = (NlmDevRangeInsertion1Reg*)readData;
                /* compare the read data with the shadow chip memory data */
                if(memcmp(ltrRangeInst1_RegInfo_p, ltrRangeInst1_RegReadInfo_p,
                    sizeof(NlmDevRangeInsertion1Reg)) != 0)
                {
                    /* If compare fails display the read data and expected data */
                    NlmCmFile__fprintf(logFile,"\n\t Compare of Read Data Fail");
                    
                    NlmCmFile__fprintf(logFile,"\n\t Range Insert LTR 1: RangeC Encoding Type :%d (%d)  RangeD Encoding Type:%d (%d)",
                    ltrRangeInst1_RegReadInfo_p->m_rangeCEncodingType, ltrRangeInst1_RegInfo_p->m_rangeCEncodingType,
                    ltrRangeInst1_RegReadInfo_p->m_rangeDEncodingType, ltrRangeInst1_RegInfo_p->m_rangeDEncodingType);
                    
                    NlmCmFile__fprintf(logFile,"\n\t Range Insert LTR 1: RangeC Encoding Byte :%d (%d)  RangeD Encoding Byte:%d (%d)",
                    ltrRangeInst1_RegReadInfo_p->m_rangeCEncodedBytes, ltrRangeInst1_RegInfo_p->m_rangeCEncodedBytes,
                    ltrRangeInst1_RegReadInfo_p->m_rangeDEncodedBytes, ltrRangeInst1_RegInfo_p->m_rangeDEncodedBytes);
                    
                    NlmCmFile__fprintf(logFile,"\n\t Range Insert LTR 1: RangeC Insert Start Byte :  RangeD Insert Start Byte:");
                    
                    for(i = 0; i < NLMDEV_NUM_KEYS; i++)
                    {
                        NlmCmFile__fprintf(logFile,"\n\t                  %d(%d)             :  %d(%d)",
                            ltrRangeInst1_RegReadInfo_p->m_rangeCInsertStartByte[i], ltrRangeInst1_RegInfo_p->m_rangeCInsertStartByte[i],
                            ltrRangeInst1_RegReadInfo_p->m_rangeDInsertStartByte[i], ltrRangeInst1_RegInfo_p->m_rangeDInsertStartByte[i]);
                    }
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }
            }
            break;

        case NLMDEV_SS_LTR:
            ltrSSRegInfo_p = (NlmDevSSReg*)regData;
            
            if(rdWrFlag == NLMDIAG_READ_FLAG)
            {
                ltrSSRegReadInfo_p = (NlmDevSSReg*)readData;
                /* compare the read data with the shadow chip memory data */
                if(memcmp(ltrSSRegInfo_p, ltrSSRegReadInfo_p, sizeof(NlmDevSSReg)) != 0)
                {
                    /* If compare fails display the read data and expected data */
                    NlmCmFile__fprintf(logFile,"\n\tCompare of Read Data Fail");
                    for(i = 0; i < NLMDEV_SS_RMP_AB; i+=2)
                    {
                        NlmCmFile__fprintf(logFile,"\n\t                  %d(%d)                   %d(%d)",
                            ltrSSRegReadInfo_p->m_ss_result_map[i], ltrSSRegInfo_p->m_ss_result_map[i],
                            ltrSSRegReadInfo_p->m_ss_result_map[i+1], ltrSSRegInfo_p->m_ss_result_map[i+1]);
                    }                    
                    return NLMDIAG_ERROR_VERIFY_FAIL;
                }
            }
            break;
        /* These registers are supported in future */
        case NLMDEV_LTR_REG_END:
        default:
            break;
    }
    return NLMDIAG_ERROR_NOERROR;
}

/* 
   This function reads/ compares the Block registers (read data with the written data)
   depending on the flag type, if READ flag set compares the read data with the written, 
   if data DIFFERS (both read and written are not same), prints the expected data. 
 */

nlm_u32 
NlmDiag_CompareBlkRegData(NlmDevBlockRegType blkRegType,                                                          
                                    nlm_u8 ab_num,
                                    NlmDiag_WriteReadFlag rdWrFlag,
                                    void *regData,
                                    void *readData,
                                    NlmCmFile *logFile
                                   )
{
    /* Structure pointers to hold the specific BCR registers read information */
    NlmDevBlockConfigReg* blkConfigRegInfo_p = NULL;
    NlmDevBlockConfigReg* blkConfigRegReadInfo_p = NULL;
    NlmDevBlockMaskReg *blkMaskRegInfo_p = NULL;
    NlmDevBlockMaskReg *blkMaskRegReadInfo_p = NULL;    
    ab_num = ab_num;

    if(blkRegType ==  NLMDEV_BLOCK_CONFIG_REG)
    {
        blkConfigRegInfo_p = (NlmDevBlockConfigReg*)regData;
             
        if(rdWrFlag == NLMDIAG_READ_FLAG)
        {  
            blkConfigRegReadInfo_p = (NlmDevBlockConfigReg*)readData;                   
            /* compare the blk register data read with the shadow chip memory data */ 
            if(blkConfigRegInfo_p->m_blockEnable != blkConfigRegReadInfo_p->m_blockEnable
                || blkConfigRegInfo_p->m_blockWidth != blkConfigRegReadInfo_p->m_blockWidth)
            {
                /* If compare fails display the read data and expected data */
                NlmCmFile__fprintf(logFile, ", Read Mismatch : ReadData: Block Width --> %d(%d), Block Enable -->%d(%d)",
                    blkConfigRegReadInfo_p->m_blockWidth, blkConfigRegInfo_p->m_blockWidth,
                    blkConfigRegReadInfo_p->m_blockEnable, blkConfigRegInfo_p->m_blockEnable);
                return NLMDIAG_ERROR_VERIFY_FAIL;
            }
        }
    }
    else
    {
        blkMaskRegInfo_p = (NlmDevBlockMaskReg*)regData;
                
        if(rdWrFlag == NLMDIAG_READ_FLAG)
        { 
            blkMaskRegReadInfo_p = (NlmDevBlockMaskReg*)readData;     
            
            /* Compare the blk reg data read with the shadow chip memory data */
            if(memcmp(blkMaskRegInfo_p, blkMaskRegReadInfo_p, 
                sizeof(NlmDevBlockMaskReg)) != 0)
            {
                /* If compare fails display the data read and data expected */
                NlmCmFile__fprintf(logFile, ", Read Mismatch ; Data Read :");
                NlmDiag_CommonPrintData(logFile, blkMaskRegReadInfo_p->m_mask, 0);  
                
                NlmCmFile__fprintf(logFile, "\t Written :");
                NlmDiag_CommonPrintData(logFile, blkMaskRegInfo_p->m_mask, 0);  
                
                return NLMDIAG_ERROR_VERIFY_FAIL;
            }
        }
    }
    return NLMDIAG_ERROR_NOERROR;
}


/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_CreateLogFileDetails

 Parameters:
            * testName      :
            * fillTestData  :
            * logfile       :
                  
 Descrtiption: 
        The routine creates the default logfile name if the -logfile option is not being used
        by the user. and also it prints the Cynapse template.

 Return Type: void
----------------------------------------------------------------------------------------*/
void NlmDiag_CreateLogFileDetails(nlm_8 *testName,
                           NlmDiag_TestInfo *fillTestData,
                           nlm_8 *logfile
                           )
{
    nlm_32 val;
    if((val = NlmCm__strcmp(logfile, "NULL")) == 0)
    {
        NlmCm__strcpy((nlm_8*)logfile, "nlm_diag_");   /* Default log file */
        NlmCm__strcpy((nlm_8*)fillTestData->m_testName, (nlm_8*)testName);
        NlmCm__strcat((nlm_8*)logfile,(nlm_8*)testName);
        NlmCm__strcat((nlm_8*)logfile,".txt");
    }
#ifndef NLMPLATFORM_BCM
#ifdef NLM_XLP
    fillTestData->m_testLogFile = NlmCm__stdout;
#else
	fillTestData->m_testLogFile = NlmCmFile__fopen((const nlm_8*)logfile, "w");
#endif
    if(fillTestData->m_testLogFile == NULL)
    {
        NlmCm__printf("\n\tUnable to open the log file.\n");
        return;
    }
    NlmCmAssert((fillTestData->m_testLogFile != NULL), "Unable to open the log file.\n");
#endif
    NlmCmFile__fprintf(fillTestData->m_testLogFile, "\n\t Diagnostic Tests. \n\n\n");    
}



/*  11  */







