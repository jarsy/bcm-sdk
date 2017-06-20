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
 
 /* This file is nlmdiag_refapp.h */

#ifndef NLMDIAG_REFAPP_H
#define NLMDIAG_REFAPP_H


/* SimXpt header */
#include "nlmsimxpt.h"
#include "nlmcmutility.h"


/* device manager header files */
#include "nlmdevmgr.h"
#include "nlmdevmgr_shadow.h"

/* common header files */
#include "nlmcmstring.h"
#include "nlmcmportable.h"
#include "nlmcmdevice.h"

#include "nlmdiag_device_dump.h"
#include "nlmdiag_interactive_tests.h"
#include  <soc/sbx/caladan3/etu_xpt.h>

#define     NLMDIAG_DEFAULT_SEED_VALUE		(45025793)
#define     NLMDIAG_GET_AB_WIDTH(abWidth)    (1 << abWidth)  
#define     NLMDIAG_SHADOW_DEV_PTR			((NlmDevShadowDevice*)dev_p->m_shadowDevice_p)

/* Device specific definitions */
#define     NLMDIAG_NUM_MASTER_ENTRIES		(512)
#define     NLM_DIAG_XPT_MAX_RQT_COUNT				(1)
#define     NLM_DIAG_XPT_MAX_RSLT_COUNT            (1)

#define     NLM_DIAG_MIN_NUM_DEV_CASCADE          (1)
#define     NLM_DIAG_MAX_NUM_DEV_CASCADE          (4)
#define     NLM_DIAG_NUM_XPT_CHANNEL              (1)
#define     NLMDIAG_DEV_MAJOR_REV            (00)
#define     NLMDIAG_DEV_MINOR_REV            (01)
#define     NLMDIAG_DEV_DB_SIZE              (03) 

/* Error specific definitions */
#define     NLMDIAG_ERROR_NOERROR                     0x0 
#define     NLMDIAG_ERROR_INVALID_INPUT               0x1
#define     NLMDIAG_ERROR_INVALID_INPUTPTR            0x2
#define     NLMDIAG_ERROR_VERIFY_FAIL                 0x3
#define     NLMDIAG_ERROR_MEMORY_ERR                  0x5
#define     NLMDIAG_ERROR_INVALID_OPERATION           0x6
#define     NLMDIAG_ERROR_XPT_HANDLE_CREATE_ERR       0xFF
#define     NLMDIAG_ERROR_INTERNAL_ERR(reasonCode)    ((reasonCode*0x1000)+ 0xFFF)    
/*#define     NLMDIAG_ERROR_FUTURE_EXPANSION            0x4 */

/* Range register specific definitions */
#define     NLMDIAG_RANGE_START_ADDRESS		        (0x00085000)   
#define     NLMDIAG_RANGE_REGISTER_COUNT	            NLM_REG_RANGE_D_CODE1  

/* Diagnostic test specific definitions */
#define     NLMDIAG_TESTCASES_MAX_PARAMS              15
#define     NLMDIAG_TESTINFO_MAX_LEN                  250

/* LTR Misc Reg search type start and end bits. (PHMD) */
#define	NLMDIAG_MISCREG_SEARCH_START_BIT	48
#define	NLMDIAG_MISCREG_SEARCH_END_BIT	55

/* LTR SS Reg Result Map start and end bits. (PHMD) */
#define	NLMDIAG_SSREG_SSMAP_START_BIT	9
#define	NLMDIAG_SSREG_SSMAP_END_BIT	16

#define NLMDIAG_FILE_READLINE_LEN		(250)
/* enum defines the types of diagnostic tests */
typedef enum
{
    NLMDIAG_AB_WRITE_READ_TEST = 0,   /* Memeory write/read/invalidate tests */
    NLMDIAG_REGISTER_WRITE_READ_TEST, /* All register default read and write/read tests */
    NLMDIAG_CMP1_TEST,                /* Compare1 test */
    NLMDIAG_CMP2_TEST                 /* Compare2 test */

    /* In future test types are added */
} NlmDiag_TestType;


/* enum defines the read flag and/or write flag for the register operations */
typedef enum NlmDiag_WriteReadFlag_e
{
    NLMDIAG_READ_FLAG,                /* flag : Register read operations */
    NLMDIAG_WRITE_FLAG                /* flag : Register write operations */
} NlmDiag_WriteReadFlag;

/* enum defines the different types patterns are used to generate data/mask keys */
typedef enum NlmDiag_DataPattern_e
{
    NLMDIAG_PATTERN_ALL_0S = 0,
    NLMDIAG_PATTERN_ALL_FS,
    NLMDIAG_PATTERN_ALL_5S,    
    NLMDIAG_PATTERN_ALL_AS,
    NLMDIAG_PATTERN_ALT_0S_FS,        
    NLMDIAG_PATTERN_ALT_AS_5S,
    NLMDIAG_PATTERN_HALF_0S_FS,    
    NLMDIAG_PATTERN_HALF_AS_5S,
    NLMDIAG_PATTERN_CHECKERBOARD_OSFS,
    NLMDIAG_PATTERN_CHECKERBOARD_AS5S,
    NLMDIAG_PATTERN_RANDOM,       /* Ramdom patterns generated using the seed given */
    NLMDIAG_PATTERN_END           /* should be end */
} NlmDiag_DataPattern;

typedef enum NlmDiag_AbEntryType_e
{
	NLMDEV_ABENTRY_DATA = 0,
	NLMDEV_ABENTRY_MASK

} NlmDiag_AbEntryType;

/* 
    enum defines the data type, the data/mask are compared/generated in specific
    format in which the device needs. These are the defined types to generate 
    the data or mask keys.
 */
typedef enum NlmDiag_CompareDataType_e
{
    NLMDIAG_COMPARE_DATATYPE_BLK_CONFIG,
    NLMDIAG_COMPARE_DATATYPE_BLK_SELECT,
    NLMDIAG_COMPARE_DATATYPE_PARALLEL_SEARCH,   
    NLMDIAG_COMPARE_DATATYPE_CB_WRITE,   
    NLMDIAG_COMPARE_DATATYPE_AB_WRITE,    
    NLMDIAG_COMPARE_DATATYPE_END
} NlmDiag_CompareDataType;

typedef enum NlmDiag_XptMode_e
{
#ifdef NLM_XLP
    NLM_XLPXPT_MODE,
#endif
    NLM_SIMULATION_MODE,
    NLM_FPGAXPT_MODE,
    NLM_BCM_CALADAN3_MODE

} NlmDiag_XptMode;

typedef enum NlmDiag_DebugOperationType_e
{
    NLM_READ_GLOBAL_REGISTER,
	NLM_WRITE_GLOBAL_REGISTER,
	NLM_READ_LTR_REGISTER,
	NLM_WRITE_LTR_REGISTER,
	NLM_READ_BLOCK_REGISTER,
	NLM_WRITE_BLOCK_REGISTER,
	NLM_READ_BLOCK_ENTRY,
	NLM_WRITE_BLOCK_ENTRY,
	NLM_READ_CB_MEMORY,
	NLM_WRITE_CB_MEMORY,
	NLM_COMP_ONE,
	NLM_COMP_TWO, 
	NLM_READ_RANGE_REGISTER, 
	NLM_WRITE_RANGE_REGISTER
} NlmDiag_DebugOperationType;


typedef struct NlmDiag_UserInput_s 
{
	nlm_u8		m_devId;		/* Device ID */
	nlm_u8		m_registerType;	/* Register Number */
	nlm_u8		m_ltrNum;		/* LTR Number (0-63) */
	nlm_u8		m_blkNum;		/* Block Number (0-127) */
	nlm_u8		m_verbose;		/* verbose */
	NlmDiag_DebugOperationType m_operation; /* Opearation Type */
	nlm_u32		m_address;		/* address */
	nlm_u8		m_compNum;							/* Compare Number 1 or 2 */
	nlm_u8		m_data[NLMDEV_REG_LEN_IN_BYTES];		/* 80 bit hex value */
	nlm_u8		m_mask[NLMDEV_REG_LEN_IN_BYTES];		/* 80 bit hex value */	
	nlm_u8		m_compareData[NLMDEV_MAX_CB_WRITE_IN_BYTES];		/*640 bit compare data */
	nlm_u32		m_compDataLen;		/*Compare data length */
	NlmCmFile   *m_inFile_fp;                       /* input file pointer */
} NlmDiag_UserInput;

/*
    Structure that holds the diagnostic test specific test information */
typedef struct  NlmDiag_TestCaseInfo_s
{
    nlm_u8      m_operMode;          /* device Operating mode */
    
    nlm_u32     m_numOfChips;        /* Number of devices are in cascade */
    nlm_u32     m_testType;          /* Diagnostic test type, see NlmDiag_TestType */
    nlm_u32     m_errCount;          /* Error counter */
    nlm_u32     m_testParams[NLMDIAG_TESTCASES_MAX_PARAMS]; /* holds the seed value,
                                                                 others if need in future */
    NlmDiag_XptMode m_xptMode;            /* Transport layer */
	NlmBool		m_isInteractive;		/* Flag to indicate Interactive mode */
	NlmDiag_UserInput	m_userInput;	/* User inputs */	
	NlmBool		m_dumpDevice;			/* Flag to indicate device dump test cases */
	NlmDiag_DevDumpOptions	m_devDumpOptions; /* Device dump options */

} NlmDiag_TestCaseInfo;

typedef struct NlmDiag_TestInfo_s 
{
	NlmCmAllocator  *m_alloc_p;			    /* memory allocator pointer */  
    nlm_u8          m_speedMode;            /* device Speed mode */
    nlm_u8          m_operMode;             /* device Operating mode */
    nlm_u8          m_testName[NLMDIAG_TESTINFO_MAX_LEN]; /* Diagnostic test name */
    nlm_u32         m_numOfChips;           /* Number of devices are in cascade */
    nlm_u32         *m_testParams;          /* seed value, and holds others in future */
    nlm_u32         m_testType;             /* type of diagniostic test to run */
    NlmCmFile       *m_testLogFile;         /* Log file to be generated. */
    void            *m_testModuleInfo_p;    /* Test module realted information */
    NlmDiag_XptMode m_xptMode;            /* Transport layer */
	NlmDiag_UserInput	*m_userInput_p;		/* User inputs */

} NlmDiag_TestInfo;

/* 
   The diag_module structure to hold the diagnostic test required informations
   like speed, operating mode, number of channels, number of cascade device, and
   pointers to hold the relavent information of Xpt, devMgr, device structures
 */
typedef struct 
{
    NlmCmAllocator  *m_alloc_p;     /* memory allocator pointer */

    nlm_u8      m_numXptChnl;       /* Number of Xpt channels */        
    nlm_u8      m_operMode;         /* Operating mode of device */
    nlm_u32     m_numCascadeDev;    /* devices can be in cascade for devices */

    NlmXpt      **m_xptPtr_pp;  /* Pointer to struct that holds XPT information */
    NlmDev    ***m_dev_ppp;   /* Pointer to struct holds the device details. */
    NlmDev    **m_cascade_pp; /* Pointer to struct holds the cascaded device details */
    NlmDevMgr **m_devMgr_pp;  /* Pointer to struct holds the device manager information */
} NlmDiag_ModuleInfo;

/* =========================================================================
                Declarions of Functions used by main 
   ========================================================================= */
void NlmDiag_PrintUsage(void);
void NlmDiag_PrintInteractiveUsage(void);
nlm_u8 NlmDiag_GetGolbalRegisterNumber(nlm_u32);
nlm_32 NlmDiag_GetHexChar(nlm_8 );

void NlmDiag_HexString2CharArray(
	nlm_8 *,
	nlm_u8 *,
	nlm_u32 );

void NlmDiag_PrintHexData(
	nlm_u8 *,
	nlm_u32);

void NlmDiag_GetLTRRegisterInfo(
	nlm_u8 *,
	nlm_u8* ,
	nlm_u32);

nlm_u32 NlmDiag_GetHexValue(
	nlm_8* ,
	nlm_32 ,
	nlm_32);

void NlmDiag_GetBlockRegisterInfo(
	nlm_u8 *, 
	nlm_u8*, 
	nlm_u32);

nlm_u32 NlmDiag_ParseOptions(
	nlm_8*,
	NlmDiag_UserInput*, 
	nlm_8*, 
	nlm_8*);

nlm_u32 NlmDiag_ProcessCommand(
	NlmDiag_TestInfo*,
	NlmDiag_TestCaseInfo*);

extern nlm_u32
NlmDiag_CompleteDiagTest(
	void					*alloc_p, 
	NlmDiag_TestInfo		*testInfo_p,
	NlmDiag_TestCaseInfo *testCaseInfo_p,
	nlm_8					*logfile
	);

extern nlm_u32
NlmDiag_InteractiveDiagTest(
	void					*alloc_p, 
	NlmDiag_TestInfo		*testInfo_p,
	NlmDiag_TestCaseInfo *testCaseInfo_p,
	nlm_8					*logfile
	);

extern int
NlmDiag_ProcessCommandLineOptions (
	int						argc,
	char					**argv,
	NlmDiag_TestCaseInfo *testCaseInfo_p,
	nlm_8					*logfile
	);

extern nlm_u32 NlmDiag_MemoryTests(
    void                             *alloc_p, 
    NlmDiag_TestInfo              *testInfo_p,
    NlmDiag_TestCaseInfo          *testCaseInfo_p
    );

extern nlm_u32 NlmDiag_CompareTests(
    void                             *alloc_p,
    NlmDiag_TestInfo              *testInfo_p,
    NlmDiag_TestCaseInfo          *testCaseInfo_p
    );

extern nlm_u32 NlmDiag_RegisterReadWriteTests(
    void                             *alloc_p, 
    NlmDiag_TestInfo              *testInfo_p,
    NlmDiag_TestCaseInfo          *testCaseInfo_p
    );

extern void NlmDiag_CreateLogFileDetails(
    nlm_8                           *testName,
    NlmDiag_TestInfo             *fillTestData,
    nlm_8                           *logfile
    );

/* =========================================================================
            Declarions of functions related to test info details
                 constructor and destructor declarartions  
   ======================================================================== */

extern nlm_u32 NlmDiag_CommonInitTestInfo(
    NlmCmAllocator					*alloc_p,
    NlmDiag_TestInfo             *testInfo_p,
    NlmDiag_TestCaseInfo         *testCaseInfo_p
    );

extern nlm_u32 NlmDiag_CommonDestroyTestInfo(
    NlmDiag_TestInfo					*self   
    );

extern nlm_u32 NlmDiag_Initialize(
    NlmDiag_TestInfo					*testInfo_p
    );

extern nlm_u32 NlmDiag_Destroy(
    NlmDiag_TestInfo					*devInfo_p
    );

/* ========================================================================
                  Common function declarations
   ======================================================================== */
extern nlm_u32 NlmDiag_CommonGenDataPattern(    
    NlmDiag_DataPattern          patternType, 
    nlm_u8                          *data,
    nlm_u32                         datalen,
    nlm_u32                         *seed_p,
    nlm_u32                         flag,
    void                            *anyInfo_p,
    NlmCmFile                       *logFile
    );

extern void NlmDiag_CommonPrintData(
	NlmCmFile						*fp,
	nlm_u8						    *p_data,
	nlm_u16						    offset
	);

extern nlm_u32 NlmDiag_CompareLtrRegData(
    NlmDevLtrRegType				ltrRegType,                                                          
    nlm_u8							profile_num,
    NlmDiag_WriteReadFlag		rdWrFlag,
    void							*regData,
    void							*readData,
    NlmCmFile						*logFile
    );

extern nlm_u32 NlmDiag_CompareBlkRegData(
    NlmDevBlockRegType			blkRegType,  
    nlm_u8							ab_num,
    NlmDiag_WriteReadFlag		rdWrFlag,
    void							*regData,
    void							*readData,
    NlmCmFile						*logFile
    );

extern nlm_u32 NlmDiag_ReadFromGlobalReg(
	NlmDiag_TestInfo             *testInfo_p);

extern nlm_u32 NlmDiag_WriteToGlobalReg(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_ReadFromLTR(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_WriteToLTR(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_ReadFromBlockReg(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_WriteToBlockReg(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_ReadBlockEntry(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_WriteBlockEntry(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_ReadFromCBMemory(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_WriteToCBMemory(
	NlmDiag_TestInfo             *testInfo_p);
extern nlm_u32 NlmDiag_CompareOne (
	NlmDiag_TestInfo				*testInfo_p);
extern nlm_u32 NlmDiag_CompareTwo (
	NlmDiag_TestInfo				*testInfo_p);
extern nlm_u32 NlmDiag_WriteToRangeRegister(
	NlmDiag_TestInfo				*testInfo_p);
extern nlm_u32 NlmDiag_ReadFromRangeRegister(
	NlmDiag_TestInfo				*testInfo_p);

nlm_u32 
NlmDiag_DumpDevice (
	void					*alloc_p, 
	NlmDiag_TestInfo		*testInfo_p,
	NlmDiag_TestCaseInfo *testCaseInfo_p,
	nlm_8					*logfile
	);

nlm_u32
NlmDiag_DisplayDevice (
	NlmDiag_TestInfo		*testInfo_p, 
	NlmDiag_DevDumpOptions *devDumpOptions_p, 
	nlm_u8					devNum, 
	NlmCmFile				*logFile_p
	);

#endif  /* end of #ifndef NLMDIAG_REFAPP_H */


/*  11  */





