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
  
 #ifndef NLMDIAG_REGISTERTEST_H
 #define NLMDIAG_REGISTERTEST_H

 #include "nlmdiag_refapp.h"

 
 /* 
     This structure to hold the Range register data of 80b length from range reg write
     function and being used in the compare operation with value read from the 
     range read function. 
 */
 typedef struct NlmDiag_RangeRegsInfo_s
 {    
    nlm_u8   m_rangeRegData[40][10]; /* which hold the 4k range reg data of 80b length */
 
 } NlmDiag_RangeRegsInfo;
 
 
 /* 
     This stucture holds the information about Block config register for the specified AB block,
     register type selested (BCR or BMR), and the data pattern. 
  */
 typedef struct  NlmDiag_BlkRegWrRdInfo_s
 {
     nlm_u8      m_abNum;
     nlm_u32     *m_seed_p;
     nlm_u32     m_flag;                   /* Test specific flags */
     void        *anyInfo_p;               /* Test specific info (depends on flags for typecast purpose)*/
 
     NlmDevBlockRegType   m_blkRegType; /* Either BCR or BMR register types */
     NlmDiag_DataPattern   m_pattern;   /* data pattern is NLMDIAG_PATTERN_COMPARE_TYPE */
 
 } NlmDiag_BlkRegWrRdInfo;
 
 
 /* 
     This stucture holds the information about LTR register for the specified LTR number or
     profile, LTR register type selested, and the data pattern using the
     seed value given. 
  */
 typedef struct  NlmDiag_LTRRegWrRdInfo_s
 {    
     nlm_u8      m_profileNum;             /* LTR profile number */
     nlm_u32     *m_seed_p;                /* seed used to generate data pattern */
     nlm_u32     m_flag;                   /* Test specific flags */
     void        *anyInfo_p;               /* Test specific info (typecast purpose)*/
 
     NlmDevLtrRegType      m_ltrRegType; /* LTR types, see NlmDevLtrRegType */
     NlmDiag_DataPattern   m_pattern;    /* data pattern is NLMDIAG_PATTERN_COMPARE_TYPE */
 
 } NlmDiag_LTRRegWrRdInfo;
 
 
 /* 
     This stucture holds the information about Range register for the specified range
     regiser number, and the data pattern using the seed value given. 
  */
 typedef struct NlmDiag_RangeRegWrRdInfo_s
 {
     nlm_u32     m_regNum;                 /* Range register number */
     nlm_u32     *m_seed_p;                /* seed value to generate the pattern */
     nlm_u32     m_flag;                   /* Test specific flags */
     void        *anyInfo_p;               /* Test specific info (typecast purpose) */
 
     NlmDiag_DataPattern   m_pattern;    /* specified data pattern */
 
 } NlmDiag_RangeRegWrRdInfo;
 
 
 /* 
     This stucture holds the information about Context Buffer register for the specified
     start address of block, number of entries within it, and the data pattern using the
     seed value given. 
  */
 typedef struct  NlmDiag_CBWrRdInfo_s
 {
     nlm_u16     m_startAddress;        /* start address of the block */
     nlm_u16     m_numOfEntries;        /* Number of entries in block */
     nlm_u8      m_dataLen;             /* length of the CB data */
     nlm_u32     *m_seed_p;             /* seed value */
     nlm_u32     m_flag;                /* Test specific flags */
     void        *anyInfo_p;            /* Test specific info (depends on flags for typecast purpose))*/
 
     NlmDiag_DataPattern   m_pattern; /* specified data pattern */
 
 } NlmDiag_CBWrRdInfo;
 
 
 /* 
     This stucture holds the information aboutGlobalr register for the specified
     register type (9 types), and the data pattern using the seed value given. 
  */
 typedef struct  NlmDiag_GlobalRegWrRdInfo_s
 {
     NlmDevGlobalRegType      m_globalRegType;/* Glbal register types, see NlmDevGlobalRegType */
     NlmDiag_DataPattern      m_pattern;      /* specified data pattern */
     nlm_u32                     *m_seed_p;      /* sedd value to generate the pattern */
     nlm_u32                     m_flag;         /* Test specific flags */
     void                        *anyInfo_p;     /* Test specific info (typecast purpose)*/
 } NlmDiag_GlobalRegWrRdInfo;
 
 
 /* The structure holds the Global registers informations */
 typedef struct NlmDiag_GlobalRegInfo_s
 {    
     NlmDevIdReg              m_devIdReg;             /* device Id register */
     NlmDevConfigReg          m_devConfigReg;         /* device configuration register */
     NlmDevErrStatusReg       m_errStatusReg;         /* error status register */
     NlmDevErrStatusReg       m_errStatusMaskReg;     /* error status mask register */
     NlmDevDbSoftErrFifoReg   m_softParityErrFifoReg; /* parity error fifo register */
     NlmDevAdvancedSoftErrReg m_advSoftErrReg;        /* new one added */
     NlmDevScratchPadReg      m_scratchPadReg[2];     /* scratch pad register 0 and 1 */
     NlmDevResultReg          m_rsltReg[2];           /* result register 0 and 1 */
 } NlmDiag_GlobalRegInfo;

 /* Context Buffer write function declarations */
extern nlm_u32  NlmDiag_WriteToCB(
     NlmDevMgr                     *devMgr_p,
     nlm_u32                          numOfDevices,
     NlmDiag_CBWrRdInfo            *CBWrInfo_p,
     FILE                             *logFile
     );
 
#endif 

