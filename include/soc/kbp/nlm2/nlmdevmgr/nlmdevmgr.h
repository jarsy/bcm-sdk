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
 

/*@@Nlm11kDevMgr Module
   Summary:
	Device manager is responsible for creating, managing and destroying device/s
	objects. Device manager should be created only once and must be destroyed when
	system of devices needs to be shutdown. Device manager provides many user
    interface functions to performs various device operations including the functions
    which creates and destroys device manager itself. 
*/

/* Revision:  SDK 2.2a on 10-Nov-2011 */

#ifndef INCLUDED_NLM11KDEVMGR_H
#define INCLUDED_NLM11KDEVMGR_H

#if defined NLM_MT_OLD || defined NLM_MT
#include <nlmcmmt.h>
#endif

/* include files */
#include "nlmcmbasic.h"
#include "nlmcmdebug.h"
#include "nlmcmallocator.h"
#include "nlmerrorcodes.h"
#include "nlmcmexterncstart.h"
#include  <soc/sbx/caladan3/etu_xpt.h>

#if !defined NLM_12K_11K && !defined NLM_12K 
#include "nlmarch.h"
#else
#include "../arch/nlmarch.h"
#endif

typedef Nlm11kDevParallelSrch NlmDev_parallel_search_t;

typedef Nlm11kDevKey NlmDev_key_t;
 
typedef Nlm11kDevParallelSrch Nlm11kDevParallelSrchNum;
typedef Nlm11kDevKey Nlm11kDevKeyNum;




/* enum for hit/miss flag */
typedef enum Nlm11kDevMissHit_e
{
	NLM11KDEV_MISS = 0,
	NLM11KDEV_HIT
} Nlm11kDevMissHit;

/* enum for enable/disable */
typedef enum Nlm11kDevDisableEnable_e
{
	NLM11KDEV_DISABLE = 0,
	NLM11KDEV_ENABLE
} Nlm11kDevDisableEnable;

/* enum for AB entries write modes. Data can be aritten in DM or XY mode */
typedef enum Nlm11kDevWriteMode_e
{
	NLM11KDEV_DM =0,
	NLM11KDEV_XY
}Nlm11kDevWriteMode;

/* enum for device Id. Maximum four devices are supported for cascading in a channel. */
typedef enum Nlm11kDevId_e
{
	NLM11KDEV_DEV_ID0 =0,
	NLM11KDEV_DEV_ID1,
	NLM11KDEV_DEV_ID2,
	NLM11KDEV_DEV_ID3
}Nlm11kDevId;

/*
    Global Register Types
*/
typedef enum Nlm11kDevGlobalRegType_e
{
	NLM11KDEV_DEVICE_ID_REG,  /* Read only Reg */
	NLM11KDEV_DEVICE_CONFIG_REG,
	NLM11KDEV_ERROR_STATUS_REG, /* Read only Reg; Reading of this register clears the set bits */
	NLM11KDEV_ERROR_STATUS_MASK_REG,
	NLM11KDEV_DATABASE_SOFT_ERROR_FIFO_REG,
    NLM11KDEV_ADVANCED_FEATURES_SOFT_ERROR_REG, /* Read only Reg*/

	NLM11KDEV_SCRATCH_PAD0_REG,
    NLM11KDEV_SCRATCH_PAD1_REG,

    NLM11KDEV_RESULT0_REG, /* Read only Reg*/
	NLM11KDEV_RESULT1_REG, /* Read only Reg*/

	NLM11KDEV_GLOBALREG_END        /* Must be the last element */

} Nlm11kDevGlobalRegType;

/*
	Device Indentification Register contains information regarding database size
	of the device and major and minor revision number of the device. This is a
	read only register.
*/
typedef struct Nlm11kDevIdReg_s
{
	nlm_u8 	m_databaseSize; /* Specifies size of the device; Can have a value of 2 (512K) or 3 (1024K) */
	nlm_u8 	m_majorDieRev;/* Specifies major die rev; Can have a value from 0 - 7 (RA - RH) */
	nlm_u8 	m_minorDieRev;/* Specifies minor die rev; Can have a value from 0 - 7 (01 - 08) */

} Nlm11kDevIdReg;

/*
	Device configuration register stores basic configuration attributes of the
	device as low power mode status, soft parity error mode, range matching engine enable and so on.
*/
typedef struct Nlm11kDevConfigReg_s
{
    Nlm11kDevDisableEnable	m_softErrorScanEnable; /* Value of 0 - disable soft error scan
														 1 - enables soft error scan*/
    Nlm11kDevDisableEnable	m_dbParityErrEntryInvalidate;	/* Value of 0 - soft error entry is not invalidated
														                    1 - soft error entry is invalidated*/
    Nlm11kDevDisableEnable	m_dbSoftErrProtectMode;	/* Value of 0 -- Use Parity for Database Soft Error Protection;
                                                                    1 -- Use ECC for Database Soft Error Protection;
                                                                    This field ignored if m_softErrorScan is disabled; */
    Nlm11kDevDisableEnable	m_eccScanType;	/* Value of 0 -- set ECC scan to 1b detect and 1b correct;
                                                            1 -- set ECC scan to 2b detect and 1b correct; */
    Nlm11kDevDisableEnable m_rangeEngineEnable; /* Value of 0 -- Disables Range Matching Engine;
                                                                1 -- Enables Range Matching Engine;*/
	Nlm11kDevDisableEnable	m_lowPowerModeEnable;	/* Value of 0 -- low power mode disabled;
                                                            1 -- low power mode enabled; */

} Nlm11kDevConfigReg;

/*
	This structure is used for error status register.
    Error status register should be read to know the type of error which has occured;
    Errors are reported via interrupt signal caused on GIO_L pin of the device;
    The same structure can be used to write to Error Status Mask Register in order
    to mask off the specified errors that are reported in which case value of 0 disables
    the error reporting while value of 1 enables the error reporting .
    Note: Error Status Register is read only Register
*/
typedef struct Nlm11kDevErrStatusReg_s
{
    NlmBool m_globalGIO_L0_Enable;  /* This field is valid only for Error Status Mask Register
                                        Value of 0 -- Globally Disabales GIO_L[0] assertions  ;
                                         1 -- Globally Enables GIO_L[0] assertions  */
    NlmBool m_globalGIO_L1_Enable;  /* This field is valid only for Error Status Mask Register
                                Value of 0 -- Globally Disabales GIO_L[1] assertions  ;
                                 1 -- Globally Enables GIO_L[1] assertions  */
    NlmBool	m_dbSoftError; /* Value of 0 -- There is no entry in Database Soft Error FIF0;
                                       1 -- There are one or more entries in Database Soft Error FIF0;*/
    NlmBool	m_dbSoftErrorFifoFull;/* Value of 0 -- Database Soft Error FIFO is not full;
                                              1 -- Database Soft Error FIFO is full and no more errors will be recorded*/
    NlmBool     m_parityScanFifoOverFlow; /* 1b0 - no Parity Scan FIFO Overflow detected */
                                                             /* 1b1 - Parity Scan FIFO Overflow detected*/

    NlmBool     m_crc24Err; /* 1b0 - no CRC24 Error detected */
                                              /* 1b1 - CRC24 Error detected*/                                                            
    NlmBool     m_sopErr; /* 1b0 - no SOP Error detected */
                                     /* 1b1 - SOP missing in the Burst Control Word that precedes Data Words*/                                                           
    NlmBool     m_eopErr;       /* 1b0 - no EOP Error detected
                                     1b1 - EOP missing in the Burst Control Wordthat follows Data Words */
    NlmBool     m_missingDataPktErr;  /* 1b0 - no Missing Data Packet Error detected
                                    1b1 - Did not receive any Data Words immediately following a Burst Control Word */
    NlmBool m_burstMaxErr;  /* 1b0 - no Burst Max Error detected
                                      1b1 - Received more than 10 data words in a burst */
    NlmBool m_rxNMACFifoParityErr;  /* 1b0 - no parity error detected in RxNMAC FIFO
                                                    1b1 - Parity error detected in RxNMAC FIFO*/
    NlmBool m_instnBurstErr;  /* 1b0 - no Instruction Burst Error detected
                                            1b1 - Mismatch in the number of expected data words for PIO and NOP instructions*/
    NlmBool m_protocolErr;  /* 1b0 - no Protocol Error detected
                                    1b1 - Received Interlaken packet instead of Interlaken look-aside packet*/
    NlmBool m_channelNumErr;  /* 1b0 - no Channel Number Error detected
                                            1b1 - Received instruction on channel #1*/
    NlmBool m_burstControlWordErr; /* 1b0 - no Burst Control Word Error detected
                                            1b1 - Interlaken Data Word received without Burst Control Word */
    NlmBool m_illegalInstnErr;       /* 1b0 - no Illegal Instruction Error detected
                                            1b1 - Illegal Instruction Error detected */
    NlmBool m_devIdMismatchErr;  /* 1b0 - no Device ID Mismatch Error detected
                                            1b1 - Device ID Mismatch Error detected */
    NlmBool m_ltrParityErr;  /* Value of 0 -- No LTR Parity Error detected;
                                         1 -- LTR Parity Error is detected */
   NlmBool m_ctxBufferParityErr;  /* 1b0 - no Context Buffer Parity Error detected
                                        1b1 - Context Buffer Parity Error detected */
   NlmBool m_powerLimitingErr;  /* 1b0 - no Power Limiting Error detected
                            1b1 - Exceeded the maximum number of active blocks per search*/
   NlmBool m_alignmentErr;  /* 1b0 - Legal framing bit combination received
                                                    (2b10 or 2b01 are legal combinations)
                                                    1b1 - Illegal framing bit combination received
                                                    (2b00 or 2b11 are illegal combinations) */
   NlmBool m_framingCtrlWordErr;   /* 1b0 - no Framing Control Word Error
                                                1b1 - Unexpectedly received framing control word */
   NlmBool m_rxPCSEFifoParityErr;   /*1b0 - no Parity error detected in RxPCS EFIFO
                                                1b1 - Parity error detected in RxPCS EFIFO */

} Nlm11kDevErrStatusReg;

/*
	structure for database soft parity error fifo register
*/
typedef struct Nlm11kDevDbSoftErrFifoReg_s
{
	nlm_u32	m_errorAddr;	/* Address of the Database Entry with parity error; This Field is read only */
    NlmBool	m_pErrorX;	    /* If set to 1 indicates that the error is in the X part of the entry;
                                This Field is read only*/
	NlmBool	m_pErrorY;		/* If set to 1 indicates that the error is in the Y part of the entry;
                                This Field is read only*/
    NlmBool	m_errorAddrValid;	/* If set to 1 indicates this is a valid error address
								     if set to 0 indicates this FIFO is empty;
                                     This Field is read only */
	NlmBool	m_eraseFifoEntry;	/* Writing 1 to this field will erase the current entry
                                and advance the pointer to next FIFO entry
                                     This Field is write only */
	NlmBool	m_eraseFifo;	/* Writing 1 to this field erase all the entries in the FIFO
                                    This Field is write only */
}Nlm11kDevDbSoftErrFifoReg;

/*
	structure for advanced featured soft error register
    Note: Advanced featured soft error registe is read only Register
*/
typedef struct Nlm11kDevAdvancedSoftErrReg_s
{
	nlm_u16	m_cbParityErrAddr;	/* Indicates the location of Ctx Buffer with parity error */
	nlm_u16	m_sahasraParityErrAddr0;	/* Indicates the location of Sahasra Engine parity error 0*/
	nlm_u16	m_sahasraParityErrAddr1;	/* Indicates the location of Sahasra Engine parity error 1*/
	nlm_u16	m_ltrParityErrAddr;	/* Indicates the location of LTR parity error;
	                                In this case Bits[10:5] indicates the LTR Number
	                                and Bits[4:0] indicates the LTR Reg Type
	                                i.e If value of this field is b'0111100004
	                                Then Parity Error has occured for LTR #15,
	                                     and Reg Type NLM11KDEV_PARALLEL_SEARCH_1_LTR
	                                Note: In case of compare LTR Reg Type is not specified*/
	                                
 }Nlm11kDevAdvancedSoftErrReg;

/*
	structure for scratch pad register 0 and 1
*/
typedef struct Nlm11kDevScratchPadReg_s
{
	nlm_u8	m_data[NLM11KDEV_REG_LEN_IN_BYTES];

} Nlm11kDevScratchPadReg;

/*
	structure for result register 0 and 1.
	it stores the result of the latest compare instruction done on the device
    Result Register is read only Register
*/
typedef struct Nlm11kDevResultReg_s
{
    /* hit or miss flag */
	Nlm11kDevMissHit	m_hitOrMiss[NLM11KDEV_NUM_PARALLEL_SEARCHES/2]; /* specifies Hit or Miss for the search */

	/* Hit Address */
	nlm_u32	            m_hitAddress[NLM11KDEV_NUM_PARALLEL_SEARCHES/2];/* In case of Hit contains Hit Address for the search*/

} Nlm11kDevResultReg;

/*
	structure definition for accessing Context Buffer as register
*/
typedef struct Nlm11kDevCtxBufferReg_s
{
	nlm_u8	m_data[NLM11KDEV_REG_LEN_IN_BYTES]; /* 80 bit CB Register data */
} Nlm11kDevCtxBufferReg;

/*
	structure definition for accessing Range Global Register
*/
typedef struct Nlm11kDevRangeReg_s
{
    nlm_u8 m_data[NLM11KDEV_REG_LEN_IN_BYTES];
}Nlm11kDevRangeReg;
/*
    LTR Register Types.
*/
typedef enum Nlm11kDevLtrRegType_e
{
	NLM11KDEV_BLOCK_SELECT_0_LTR,
	NLM11KDEV_BLOCK_SELECT_1_LTR,

    NLM11KDEV_SUPER_BLK_KEY_MAP_LTR,

	NLM11KDEV_PARALLEL_SEARCH_0_LTR,
    NLM11KDEV_PARALLEL_SEARCH_1_LTR,
    NLM11KDEV_PARALLEL_SEARCH_2_LTR,
    NLM11KDEV_PARALLEL_SEARCH_3_LTR,

    NLM11KDEV_RANGE_INSERTION_0_LTR,
    NLM11KDEV_RANGE_INSERTION_1_LTR,

    NLM11KDEV_MISCELLENEOUS_LTR,
    NLM11KDEV_SS_LTR,

    NLM11KDEV_KEY_0_KCR_0_LTR,
    NLM11KDEV_KEY_0_KCR_1_LTR,
    NLM11KDEV_KEY_1_KCR_0_LTR,
    NLM11KDEV_KEY_1_KCR_1_LTR,
    NLM11KDEV_KEY_2_KCR_0_LTR,
    NLM11KDEV_KEY_2_KCR_1_LTR,
    NLM11KDEV_KEY_3_KCR_0_LTR,
    NLM11KDEV_KEY_3_KCR_1_LTR,
	NLM11KDEV_LTR_REG_END                /* must be the last element */

} Nlm11kDevLtrRegType;

/*
	Structure for Block select register 0 and 1. Block select register stores the information
	if a block is disabled or enabled for search operations.
*/
typedef struct Nlm11kDevBlkSelectReg_s
{
	Nlm11kDevDisableEnable	m_blkEnable[NLM11KDEV_NUM_ARRAY_BLOCKS/2];
} Nlm11kDevBlkSelectReg;

/*
	Structure for Super Block to Key map register .
    This registers store the info about which super block is mapped to
	which key.
*/
typedef struct Nlm11kDevSuperBlkKeyMapReg_s
{
	Nlm11kDevKeyNum		m_keyNum[NLM11KDEV_NUM_SUPER_BLOCKS];
} Nlm11kDevSuperBlkKeyMapReg;

/*
	Structure for Parallel search register 0,1,2 and 3.
    These registers store the info about which array block is mapped to
	which parallel search.
*/
typedef struct Nlm11kDevParallelSrchReg_s
{
	Nlm11kDevParallelSrchNum	m_psNum[NLM11KDEV_NUM_ARRAY_BLOCKS/4];
} Nlm11kDevParallelSrchReg;

/* Enum which specifies Range Encoding Types */
typedef enum Nlm11kDevRangeEncodingType_e
{
    NLM11KDEV_3BIT_RANGE_ENCODING,
    NLM11KDEV_2BIT_RANGE_ENCODING,
    NLM11KDEV_NO_RANGE_ENCODING
}Nlm11kDevRangeEncodingType;

/* Enum which specifies number of bytes of Range Encoding value to be used */
typedef enum Nlm11kDevRangeEncodedValueBytes_e
{
    NLM11KDEV_1BYTE_RANGE_ENCODED_VALUE,
    NLM11KDEV_2BYTE_RANGE_ENCODED_VALUE,
    NLM11KDEV_3BYTE_RANGE_ENCODED_VALUE,
    NLM11KDEV_4BYTE_RANGE_ENCODED_VALUE
} Nlm11kDevRangeEncodedValueBytes;

/* Enum for the search types for each Parallel Search. For PS#2 and PS#3, only value supported
  * is _STANDARD.
  */
typedef enum Nlm11kDevSearchType_e
{
	NLM11KDEV_STANDARD = 0,
	NLM11KDEV_SAHASRA  = 2 /* 2'b10 means Sahasra search. */

} Nlm11kDevSearchType;

/* Structure for Range Insertion 0 and Range Insertion 1 registers;
Contains information about type of encoding to be used;
Location of Range Encoding Insertion in various keys and number of bytes of range encoded value
to be inserted */
typedef struct Nlm11kDevRangeInsertion0Reg_s
{
    Nlm11kDevRangeEncodingType m_rangeAEncodingType; /* Specifies type of range encoding to be used for Range A */
    Nlm11kDevRangeEncodingType m_rangeBEncodingType; /* Specifies type of range encoding to be used for Range B */
    Nlm11kDevRangeEncodedValueBytes m_rangeAEncodedBytes;   /* Specifies number of bytes of Range A encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/
    Nlm11kDevRangeEncodedValueBytes m_rangeBEncodedBytes;   /* Specifies number of bytes of Range B encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/

    nlm_u8 m_rangeAInsertStartByte[NLM11KDEV_NUM_KEYS]; /* Specifies the start byte where the Range A Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLM11KDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
    nlm_u8 m_rangeBInsertStartByte[NLM11KDEV_NUM_KEYS];/* Specifies the start byte where the Range B Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLM11KDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
}Nlm11kDevRangeInsertion0Reg;

typedef struct Nlm11kDevRangeInsertion1Reg_s
{
    Nlm11kDevRangeEncodingType m_rangeCEncodingType; /* Specifies type of range encoding to be used for Range C */
    Nlm11kDevRangeEncodingType m_rangeDEncodingType; /* Specifies type of range encoding to be used for Range D */
    Nlm11kDevRangeEncodedValueBytes m_rangeCEncodedBytes;   /* Specifies number of bytes of Range C encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/
    Nlm11kDevRangeEncodedValueBytes m_rangeDEncodedBytes;   /* Specifies number of bytes of Range D encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/

    nlm_u8 m_rangeCInsertStartByte[NLM11KDEV_NUM_KEYS]; /* Specifies the start byte where the Range C Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLM11KDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
    nlm_u8 m_rangeDInsertStartByte[NLM11KDEV_NUM_KEYS];/* Specifies the start byte where the Range D Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLM11KDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
}Nlm11kDevRangeInsertion1Reg;


typedef struct Nlm11kDevMiscelleneousReg_s
{
    nlm_u8 m_bmrSelect[NLM11KDEV_NUM_PARALLEL_SEARCHES]; /* Specifies BMR number to be used for each parallel search;
                                                         Valid values 0 - 4 and NLM11KDEV_NO_MASK_BMR_NUM */

    nlm_u8 m_rangeAExtractStartByte;    /* Specifies the start byte in the compare key from which the 16b rangeA value
                                        to be used for encoding needs to be picked; Supported Values 0 -78 */
    nlm_u8 m_rangeBExtractStartByte;   /* Specifies the start byte in the compare key from which the 16b rangeB value
                                       to be used for encoding needs to be picked; Supported Values 0 -78 */
    nlm_u8 m_rangeCExtractStartByte;   /* Specifies the start byte in the compare key from which the 16b rangeC value
                                       to be used for encoding needs to be picked; Supported Values 0 -78 */
    nlm_u8 m_rangeDExtractStartByte;  /* Specifies the start byte in the compare key from which the 16b rangeD value
                                       to be used for encoding needs to be picked; Supported Values 0 -78 */
    nlm_u8 m_numOfValidSrchRslts; /* Specifies number of search results that are valid
                                        Supported values 0 - All four search results are valid
                                        Supported values 1 - Only one search result is valid
                                        Supported values 2 - Only two search results are valid
                                        Supported values 3 - Only three search results are valid */

	Nlm11kDevSearchType m_searchType[NLM11KDEV_NUM_PARALLEL_SEARCHES];

}Nlm11kDevMiscelleneousReg;
/*
	Key Construction register stores information about how the individual parallel search keys
    needs to be generated from the master(compare) key passed withe the compare instruction(s).
*/
typedef struct Nlm11kDevKeyConstructReg_s
{
	nlm_u8 m_startByteLoc[NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR]; /* Specifies start byte of each segment in the compare key;
                                                                Valid values  0 - 79 */
    nlm_u8 m_numOfBytes[NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR];/* Specifies number of bytes each segment comprises of;
                                                                Valid values 1- 16; Value of zero indicates that there
                                                                are no more valid segments and device manager can ignore
                                                                next segments */
} Nlm11kDevKeyConstructReg;


/*
    Block Register Set
*/
typedef enum Nlm11kDevBlockRegType_e
{
	NLM11KDEV_BLOCK_CONFIG_REG,

	NLM11KDEV_BLOCK_MASK_0_0_REG,  /* BMR 0, segment[79:0] */
    NLM11KDEV_BLOCK_MASK_0_1_REG,  /* BMR 0, segment[159:80] */
    NLM11KDEV_BLOCK_MASK_0_2_REG,  /* BMR 0, segment[239:160]  */
    NLM11KDEV_BLOCK_MASK_0_3_REG,  /* BMR 0, segment[319:240] */

    NLM11KDEV_BLOCK_MASK_1_0_REG,  /* BMR 1, segment[79:0]  */
    NLM11KDEV_BLOCK_MASK_1_1_REG,  /* BMR 1, segment[159:80]  */
    NLM11KDEV_BLOCK_MASK_1_2_REG,  /* BMR 1, segment[239:160]  */
    NLM11KDEV_BLOCK_MASK_1_3_REG,  /* BMR 1, segment[319:240]  */

    NLM11KDEV_BLOCK_MASK_2_0_REG,
    NLM11KDEV_BLOCK_MASK_2_1_REG,
    NLM11KDEV_BLOCK_MASK_2_2_REG,
    NLM11KDEV_BLOCK_MASK_2_3_REG,

    NLM11KDEV_BLOCK_MASK_3_0_REG,
    NLM11KDEV_BLOCK_MASK_3_1_REG,
    NLM11KDEV_BLOCK_MASK_3_2_REG,
    NLM11KDEV_BLOCK_MASK_3_3_REG,

    NLM11KDEV_BLOCK_MASK_4_0_REG,
    NLM11KDEV_BLOCK_MASK_4_1_REG,
    NLM11KDEV_BLOCK_MASK_4_2_REG,
    NLM11KDEV_BLOCK_MASK_4_3_REG,

    NLM11KDEV_BLOCK_REG_END          /* must be the last element */

} Nlm11kDevBlockRegType;

/* enum for block widths */
typedef enum Nlm11kDevBlockWidth_e
{
	NLM11KDEV_BLK_WIDTH_80,
	NLM11KDEV_BLK_WIDTH_160,
	NLM11KDEV_BLK_WIDTH_320,
	NLM11KDEV_BLK_WIDTH_640
} Nlm11kDevBlockWidth;

/* 	Block configuration register stores information about block enable status,
	and block width; There is only one BCR par array block.
*/
typedef struct Nlm11kDevBlockConfigReg_s
{
	Nlm11kDevDisableEnable		m_blockEnable;		/* specifies whether the blk is enabled/disabled */
    Nlm11kDevBlockWidth			m_blockWidth;		/* specifies width of the block */

#if defined NLM_12K_11K || defined NLM_12K 
	nlm_u32			padding12k[3];   	
#endif
	
} Nlm11kDevBlockConfigReg;

/* 	Block mask register stores mask bits which masks of the specified bits of the Database from compare operations
    Each block can have 5 BMRs where each BMR has 4 segments of 80b;
    Segment 0 -- Used to mask bits[79:0]
    Segment 1 -- Used to mask bits[159:80]
    Segment 2 -- Used to mask bits[239:160]
    Segment 3 -- Used to mask bits[319:240]
 */
typedef struct Nlm11kDevBlockMaskReg_s
{
	nlm_u8	m_mask[NLM11KDEV_REG_LEN_IN_BYTES];
} Nlm11kDevBlockMaskReg;

/* Structure Array Block Entry consists of 80b data, 80b mask and a valid bit.
*/
typedef struct Nlm11kDevABEntry_s
{
	nlm_u8	m_data[NLM11KDEV_AB_WIDTH_IN_BYTES];
	nlm_u8	m_mask[NLM11KDEV_AB_WIDTH_IN_BYTES];
	nlm_u8	m_vbit; /* In case of read specifies whether the entry is valid or deleted */
} Nlm11kDevABEntry;

/* Structure used for AB Entry Write and Read operations
    Upto 8 consective locations can be read and written at a time
    */
#define NLM11KDEV_AB_WR_RD_MAX_LOCS         (8)


typedef struct Nlm11kDevMultiABEntryWrRdParam_s
{
    nlm_u8 m_abNum; /* Specifies the AB Number */
    nlm_u8 m_numOfLocs;	 /* Specifies number of consecutive locations to be read/written */
    nlm_u16 m_abAddr; /* Specifies the AB Entry location within the specified abNum  */
    Nlm11kDevABEntry abEntry[NLM11KDEV_AB_WR_RD_MAX_LOCS];
} Nlm11kDevMultiABEntryWrRdParam;

/*
    Structure which contains the CB Data passed with CB Write, Compare1 and Compare 2 operations
*/
typedef struct Nlm11kDevCtxBufferInfo_s
{
    nlm_u16 m_cbStartAddr;  /* Specifies the CB Address at which the data is to be written;
                            In case of cmp operations it also specifies the 40b segment of
                            CB memory used for the compare key */
    nlm_u8 m_datalen; /* specifies amount of data in terms of bytes to be written to CB memory */
	nlm_u8	m_data[NLM11KDEV_MAX_CB_WRITE_IN_BYTES];   /* Maximum of 640b (80bytes) of CB data can be written */

} Nlm11kDevCtxBufferInfo;


/* Summary
	structure definition used to return upto four parallel search results of compare operations
*/
typedef struct Nlm11kDevCmpRslt_s
{
	Nlm11kDevMissHit	m_hitOrMiss[NLM11KDEV_NUM_PARALLEL_SEARCHES];		/* hit or miss flag*/
	nlm_u8				    m_hitDevId[NLM11KDEV_NUM_PARALLEL_SEARCHES];        /* specifies the hit dev id */
	nlm_u32				    m_hitIndex[NLM11KDEV_NUM_PARALLEL_SEARCHES];		/* specifies the hit index  */
} Nlm11kDevCmpRslt;


/* Summary
	Device manager structure contains list of all devices, device count and a pointer to
	memory allocator which is used to allocate memory for all the operations done by device
    manager module.
*/
typedef struct Nlm11kDevMgr_s
{
	NlmCmAllocator *m_alloc_p;		/* memory allocator */
    nlm_u32		m_devCount;		/* count of devices in the list */
	void**		m_devList_pp;		/* list of all devices */
	void*		m_xpt_p;			/* transport layer pointer for internal purpose. */
	NlmBool		m_isLocked;			/* specifies whether device manager configuration is locked or not */
    Nlm11kDev_OperationMode m_operMode; /* operating mode of device(s) */
    nlm_u32    m_magicNum;     /* Magic Number to check the validity of device manager pointer */
    NlmBool    m_is10MDev;	   /* if "1" deivce is 10M, else 40M */
#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMtSpinlock 	m_spinLock; /* Spin lock to protect data consistency with multi-threading*/
#endif

}Nlm11kDevMgr;

/*
	It contains information regarding the devices added to the cascade system;
    Specifically it contains Device ID, pointer to device manager,
	cascade information and shadow device pointer. Shadow device is a software copy
	of actual device including all registers and database. Use of shadow device
	is vital and explained separately in nlmdevmgr_shadow.h file.
*/
typedef struct Nlm11kDev_s
{
	Nlm11kDevMgr *m_devMgr_p;			/* device manager to which the device belongs to */
	Nlm11kDevId			m_devId;	/* device ID as defined in the enum above */
    nlm_u32    m_magicNum;     /* Magic Number to check the validity of device pointer */
	void*	m_shadowDevice_p;					/*  pointer to shadow memory of the device*/
}Nlm11kDev;


/* functions exposed by DevMgr Module
 for reason code details, user must not pass NULL. if o_reason is NULL, user
 will not be get reason codes for failures.
*/
/*
	Nlm11kDevMgr__create creates Device manager using memory allocator which must be passed
	as parameter. Function returns NULL if fails and user should see the reason code in
	case of failure.
*/
extern Nlm11kDevMgr* Nlm11kDevMgr__create(
	NlmCmAllocator *alloc_p,
	void*			xpt_p,
    Nlm11kDev_OperationMode operMode,
	NlmReasonCode*	o_reason
	);

/*
	Nlm11kDevMgr__destroy deletes Device manager and all devices in the device manager list.
*/
extern void Nlm11kDevMgr__destroy(
    Nlm11kDevMgr *devMgr_p
	);

/*
	Nlm11kDevMgr__AddDevice creates an software instance of device and adds to device list of device manager.
	If this function fails it returns NULL . User should see the reason code in case of failure.
*/
extern Nlm11kDev* Nlm11kDevMgr__AddDevice(
	Nlm11kDevMgr    *self,
	Nlm11kDevId			*o_devId,		/* device id assigned by the device manager for the device being added*/
	NlmReasonCode*			o_reason
	);

/*
	Nlm11kDevMgr__ResetDevice resets all the devices in cascade to have initial values.
	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__ResetDevices(
	Nlm11kDevMgr						*self,
	NlmReasonCode*					o_reason
	);

/*
	Nlm11kDevMgr__GlobalRegisterRead reads any global register depending upon the register type.
	Read data is stored in *o_data.	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__GlobalRegisterRead(
	Nlm11kDev				*dev,
	Nlm11kDevGlobalRegType			regType,		/* Global register type - see definitions above */
	void 							*o_data,	/* Global register structure pointer as output */
	NlmReasonCode*					o_reason
	);

/*
	Nlm11kDevMgr__GlobalRegisterWrite writes any global register depending upon the register type.
	Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__GlobalRegisterWrite(
	Nlm11kDev 						*dev,
	Nlm11kDevGlobalRegType			regType,		/* Global register type - see definitions above */
	const void 						*data,		/* Global register structure pointer */
	NlmReasonCode*					o_reason
	);

/*
	Nlm11kDevMgr__CBAsRegisterRead reads context buffer as register read.
	Read data is stored in *o_data.	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__CBAsRegisterRead(
	Nlm11kDev				*dev,
	nlm_u16					cbAddr,		/* Only the offset address of the CB needs to be provided
	                                                                      i.e 0 - 16383 (NLM11KDEV_CB_DEPTH - 1)*/
	Nlm11kDevCtxBufferReg			*o_cbRegData,		/* see the structure description above. */
	NlmReasonCode*			o_reason
	);

/*
	Nlm11kDevMgr__CBAsRegisterWrite writes context buffer as register write.
	Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__CBAsRegisterWrite(
	Nlm11kDev				*dev,
	nlm_u16					cbAddr,		/* Only the offset address of the CB needs to be provided
	                                                                      i.e 0 - 16383 (NLM11KDEV_CB_DEPTH - 1)*/
	Nlm11kDevCtxBufferReg 			*cbRegData,			/* see the structure description above */
	NlmReasonCode*			o_reason
	);

/*
	Nlm11kDevMgr__LogicalTableRegisterRead reads LTR register depending on LTR register type.
	Read data is stored in *o_data.	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__LogicalTableRegisterRead(
	Nlm11kDev 						*dev,
	nlm_u8							ltrNum,	/* LTR profile set number */
	Nlm11kDevLtrRegType			 	regType,		/* see the structure description above */
	void 							*o_data,	/* LTR register type structure pointer as output */
	NlmReasonCode*					o_reason
	);

/*
	Nlm11kDevMgr__LogicalTableRegisterWrite writes LTR register depending on LTR register type.
	Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__LogicalTableRegisterWrite(
	Nlm11kDev 						*dev,
	nlm_u8							ltrNum,	/* LTR profile set number */
	Nlm11kDevLtrRegType			 	regType,		/* see the structure description above */
	const void						*data,		/* LTR register type structure pointer */
	NlmReasonCode*					o_reason
	);

/*
	Nlm11kDevMgr__LogicalTableRegisterRefresh refreshes the LTR register data depending on LTR register type.
    It reads the data from Shadow Memory the data of specified register and writes it to device.
    This function will be useful to re-write the LTR Register Data which have suffered from soft parity error
	 User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__LogicalTableRegisterRefresh(
	Nlm11kDev 						*dev,
	nlm_u8							ltrNum,	/* LTR profile set number */
	Nlm11kDevLtrRegType			 	regType,		/* see the structure description above */
	NlmReasonCode*					o_reason
	);

/*
	Nlm11kDevMgr__BlockRegisterRead reads block register depending on block register type.
	Read data is stored in *o_data.	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__BlockRegisterRead(
	Nlm11kDev 						*dev,
	nlm_u8							abNum,		/* AB number in which register lies */
	Nlm11kDevBlockRegType			regType,		/* see the structure description above */
	void 							*o_data,	/* Block register structure pointer as output */
	NlmReasonCode*					o_reason
	);

/*
	Nlm11kDevMgr__BlockRegisterWrite writes block register depending on block register type.
	Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__BlockRegisterWrite(
	Nlm11kDev 						*dev,
	nlm_u8							abNum,		/* AB number in which register lies */
	Nlm11kDevBlockRegType			regType,		/* see the structure description above */
	const void 						*data,		/* Block register structure pointer */
	NlmReasonCode*					o_reason
	);


/*
	Nlm11kDevMgr__ABEntryRead reads single Array Block Database entry
    from the specified addr and abNum.
    See Description of Nlm11kDevABEntry for more details
    Data can be read only in XY mode
	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__ABEntryRead(
	Nlm11kDev      		*dev,
    nlm_u8 abNum, /* Specifies the AB Number */
    nlm_u16 abAddr, /* Specifies the AB Entry location within the specified abNum  */
    Nlm11kDevABEntry *o_abEntry,
	NlmReasonCode*		o_reason
    );

/*
	Nlm11kDevMgr__ABEntryWrite writes single Array Block Database entry
    to the specified addr and abNum.
    See Description of Nlm11kDevABEntry for more details
    Data can be written in either DM or XY mode
	User should see the reason code in case of failure.
*/

extern NlmErrNum_t Nlm11kDevMgr__ABEntryWrite(
	Nlm11kDev      		*dev,
    nlm_u8 abNum, /* Specifies the AB Number */
    nlm_u16 abAddr, /* Specifies the AB Entry location within the specified abNum  */
    Nlm11kDevABEntry *abEntry,
    Nlm11kDevWriteMode	writeMode,
	NlmReasonCode*		o_reason
	);

/*
	Nlm11kDevMgr__ABEntryInvalidate invalidates specified array block database entry
	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__ABEntryInvalidate(
	Nlm11kDev      		*dev,
	nlm_u8				abNum,					/* AB number */
	nlm_u16				abAddr,					/* entry index within the AB */
	NlmReasonCode*		o_reason
	);

/*
	Nlm11kDevMgr__MultiABEntryRead reads multiple Array Block Database entries.
    Upto 8 consective locations of 80b can be read;
    But Presently this function is not supported;
    See Description of Nlm11kDevABEntryWrRdParam for more details
    Data can be read only in XY mode
	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__MultiABEntryRead(
	Nlm11kDev      		*dev,
	Nlm11kDevMultiABEntryWrRdParam   *abEntryReadParam,
	NlmReasonCode*		o_reason
	);

/*
	Nlm11kDevMgr__MultiABEntryWrite writes multiple Array Block Database entries.
	Upto 8 consective locations of 80b can be written;
    But Presently this function is not supported;
    See Description of Nlm11kDevABEntryWrRdParam for more details
    Data can be written in either DM or XY mode
	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__MultiABEntryWrite(
	Nlm11kDev      		*dev,
	Nlm11kDevMultiABEntryWrRdParam   *abEntryWriteParam,
	Nlm11kDevWriteMode	writeMode,				/* data can be written in DM or XY mode */
	NlmReasonCode*		o_reason
	);

/*
	Nlm11kDevMgr__MultiABEntryInvalidate invalidates multiple array block database entries.
    Upto 8 consective locations of 80b can be invalidated;
    But Presently this function is not supported;
	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__MultiABEntryInvalidate(
	Nlm11kDev      		*dev,
	nlm_u8				abNum,					/* AB number */
	nlm_u16				addr,					/* entry index within the AB */
    nlm_u8              numOfLocs,     /* Number of 80b locations to be invalidated */
	NlmReasonCode*		o_reason
	);

/*
Function: Nlm11kDevMgr__ABEntryRefresh
Description: Refreshes one entry depending on entry index and AB number.
It reads the data at that location from Shadow Memory and writes it to device.
This function will be useful to re-write the entries which have suffered from soft parity error
User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__ABEntryRefresh(
    Nlm11kDev		    *dev,
    nlm_u8			    abNum, 			/* AB number in which entry lies */
    nlm_u16 			addr,			    /* entry index in an AB */
    Nlm11kDevWriteMode	writeMode,
    NlmReasonCode*	    o_reason
    );

/*
	Nlm11kDevMgr__Compare1 is a search instruction. It searches the input key into the
	database depending on Ltr profile set number (there are 32 sets of Ltr registers)
	and gives hit/miss and address as an output.
    Compare1 instruction can be used for searches with individual search key lengths of upto 320b
    User should see the reason 	code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__Compare1(
	Nlm11kDevMgr					*self,
	nlm_u8						ltrNum,			/* LTR profile set number to be used */
	Nlm11kDevCtxBufferInfo		*cbInfo,	/* see the structure description above */
	Nlm11kDevCmpRslt			*o_search_results,	/* see the structure description above */
	NlmReasonCode*				o_reason
	);

/*
	Nlm11kDevMgr__Compare2 is a search instruction. It searches the input key into the
	database depending on Ltr profile set number (there are 32 sets of Ltr registers)
	and gives hit/miss and address as an output.
    Compare2 instruction can be used for searches with individual search key lengths of upto 640b
    User should see the reason 	code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__Compare2(
	Nlm11kDevMgr					*self,
	nlm_u8						ltrNum,			/* LTR profile set number to be used */
	Nlm11kDevCtxBufferInfo		*cbInfo,				/* see the structure description above */
	Nlm11kDevCmpRslt			*o_search_results,	/* see the structure description above */
	NlmReasonCode*				o_reason
	);

/*
	Nlm11kDevMgr__CBWrite writes to context buffer memory
    Upto 640 bytes of data can be written to CB memory;
    see desription of Nlm11kDevCtxBufferInfo for more details
	User should see the reason code in case of failure.
*/
extern NlmErrNum_t Nlm11kDevMgr__CBWrite(
	Nlm11kDevMgr					*self,
	Nlm11kDevCtxBufferInfo		*cbInfo,				/* see the structure description above */
	NlmReasonCode*				o_reason
	);


/*
	Nlm11kDevMgr__LockConfig locks the device manager configuration.
	Once the configuration is locked no more devices can be added.
*/
extern NlmErrNum_t Nlm11kDevMgr__LockConfig(
	Nlm11kDevMgr *self,
	NlmReasonCode *o_reason
	);

/*
	Range related register write.
	This API is for internal use.
*/
extern NlmErrNum_t Nlm11kDevMgr__RangeRegisterWrite(
	Nlm11kDev *dev,
	nlm_u32 address,
	Nlm11kDevRangeReg *rangeRegData,
	NlmReasonCode *o_reason
	);

/*
	Range related register read.
	This API is for internal use.
*/
extern NlmErrNum_t Nlm11kDevMgr__RangeRegisterRead(
	Nlm11kDev *dev,
	nlm_u32 address,
	Nlm11kDevRangeReg *o_rangeRegData,
	NlmReasonCode *o_reason
	);

/* Shadow device destroy and Shadow device create API should be called if application
wishes to reset the shadow memory data after it resets the devices in cascade */
extern NlmErrNum_t Nlm11kDevMgr__ShadowDeviceCreate(
    Nlm11kDev *dev_p,
    NlmReasonCode *o_reason
	);

extern NlmErrNum_t Nlm11kDevMgr__ShadowDeviceDestroy(
    Nlm11kDev *dev_p,
    NlmReasonCode *o_reason
	);

/* Generic Register write API (write 80b global[R/W] and LTR) */
extern NlmErrNum_t Nlm11kDevMgr__RegisterWrite(
	void			*dev,
	nlm_u32			address,		
	void			*data,
	NlmReasonCode	*o_reason
	);

/* Generic Register read API (read 80b global and LTR) */
extern NlmErrNum_t Nlm11kDevMgr__RegisterRead(
	void			*dev,
	nlm_u32			address,		
	void			*o_data,
	NlmReasonCode	*o_reason
	);

/* This API send NOP instruction to device with number of times specified */
extern NlmErrNum_t Nlm11kDevMgr__SendNop(
	Nlm11kDev      		*dev,
	nlm_u32				numTimes,
	NlmReasonCode*		o_reason
	);

extern void 
Nlm11kDevMgr__DumpShadowMemory( 
		nlm_u8 startDeviceNum,
		nlm_u8 endDeviceNum,
		nlm_u16 startBlkNum,
		nlm_u16 endBlkNum,
		NlmBool printRegs);



#if !defined NLM_12K_11K && !defined NLM_12K
/* 11K Specific APIs and Data structures with generic name i.e. "nlm" instead of "nlm11k"*/

#define NlmDevParallelSrchNum Nlm11kDevParallelSrch
#define NlmDevKeyNum Nlm11kDevKey

/* enum for hit/miss flag */
typedef enum NlmDevMissHit_e
{
	NLMDEV_MISS = 0,
	NLMDEV_HIT
} NlmDevMissHit;

/* enum for enable/disable */
typedef enum NlmDevDisableEnable_e
{
	NLMDEV_DISABLE = 0,
	NLMDEV_ENABLE
} NlmDevDisableEnable;

/* enum for AB entries write modes. Data can be aritten in DM or XY mode */
typedef enum NlmDevWriteMode_e
{
	NLMDEV_DM =0,
	NLMDEV_XY
}NlmDevWriteMode;

/* enum for device Id. Maximum four devices are supported for cascading in a channel. */
typedef Nlm11kDevId NlmDevId;

/*
    Global Register Types
*/
typedef enum NlmDevGlobalRegType_e
{
	NLMDEV_DEVICE_ID_REG,  /* Read only Reg */
	NLMDEV_DEVICE_CONFIG_REG,
	NLMDEV_ERROR_STATUS_REG, /* Read only Reg; Reading of this register clears the set bits */
	NLMDEV_ERROR_STATUS_MASK_REG,
	NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG,
    NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG, /* Read only Reg*/

	NLMDEV_SCRATCH_PAD0_REG,
    NLMDEV_SCRATCH_PAD1_REG,

    NLMDEV_RESULT0_REG, /* Read only Reg*/
	NLMDEV_RESULT1_REG, /* Read only Reg*/

	NLMDEV_GLOBALREG_END        /* Must be the last element */

} NlmDevGlobalRegType;



#define NlmDevIdReg Nlm11kDevIdReg

#define  NlmDevConfigReg	 Nlm11kDevConfigReg

#define	NlmDevErrStatusReg	Nlm11kDevErrStatusReg

#define	NlmDevDbSoftErrFifoReg	Nlm11kDevDbSoftErrFifoReg

#define	NlmDevAdvancedSoftErrReg	Nlm11kDevAdvancedSoftErrReg

#define	NlmDevScratchPadReg	Nlm11kDevScratchPadReg

#define	NlmDevResultReg		Nlm11kDevResultReg


#define	NlmDevCtxBufferReg	Nlm11kDevCtxBufferReg

#define	NlmDevRangeReg	Nlm11kDevRangeReg

/*
    LTR Register Types.
*/
typedef enum NlmDevLtrRegType_e
{
	NLMDEV_BLOCK_SELECT_0_LTR,
	NLMDEV_BLOCK_SELECT_1_LTR,

    NLMDEV_SUPER_BLK_KEY_MAP_LTR,

	NLMDEV_PARALLEL_SEARCH_0_LTR,
    NLMDEV_PARALLEL_SEARCH_1_LTR,
    NLMDEV_PARALLEL_SEARCH_2_LTR,
    NLMDEV_PARALLEL_SEARCH_3_LTR,

    NLMDEV_RANGE_INSERTION_0_LTR,
    NLMDEV_RANGE_INSERTION_1_LTR,

    NLMDEV_MISCELLENEOUS_LTR,
    NLMDEV_SS_LTR,

    NLMDEV_KEY_0_KCR_0_LTR,
    NLMDEV_KEY_0_KCR_1_LTR,
    NLMDEV_KEY_1_KCR_0_LTR,
    NLMDEV_KEY_1_KCR_1_LTR,
    NLMDEV_KEY_2_KCR_0_LTR,
    NLMDEV_KEY_2_KCR_1_LTR,
    NLMDEV_KEY_3_KCR_0_LTR,
    NLMDEV_KEY_3_KCR_1_LTR,
	NLMDEV_LTR_REG_END                /* must be the last element */

} NlmDevLtrRegType;

#define	NlmDevBlkSelectReg	Nlm11kDevBlkSelectReg

#define	NlmDevSuperBlkKeyMapReg	Nlm11kDevSuperBlkKeyMapReg

#define	NlmDevParallelSrchReg	Nlm11kDevParallelSrchReg

/* Enum which specifies Range Encoding Types */
typedef enum NlmDevRangeEncodingType_e
{
    NLMDEV_3BIT_RANGE_ENCODING,
    NLMDEV_2BIT_RANGE_ENCODING,
    NLMDEV_NO_RANGE_ENCODING
}NlmDevRangeEncodingType;

/* Enum which specifies number of bytes of Range Encoding value to be used */
typedef enum NlmDevRangeEncodedValueBytes_e
{
    NLMDEV_1BYTE_RANGE_ENCODED_VALUE,
    NLMDEV_2BYTE_RANGE_ENCODED_VALUE,
    NLMDEV_3BYTE_RANGE_ENCODED_VALUE,
    NLMDEV_4BYTE_RANGE_ENCODED_VALUE
} NlmDevRangeEncodedValueBytes;

/* Enum for the search types for each Parallel Search. For PS#2 and PS#3, only value supported
  * is _STANDARD.
  */
typedef enum NlmDevSearchType_e
{
	NLMDEV_STANDARD = 0,
	NLMDEV_SAHASRA  = 2 /* 2'b10 means Sahasra search. */

} NlmDevSearchType;

#define NlmDevRangeInsertion0Reg	Nlm11kDevRangeInsertion0Reg

#define NlmDevRangeInsertion1Reg	Nlm11kDevRangeInsertion1Reg

#define NlmDevMiscelleneousReg	Nlm11kDevMiscelleneousReg

#define NlmDevKeyConstructReg	Nlm11kDevKeyConstructReg


typedef enum NlmDevBlockRegType_e
{
	NLMDEV_BLOCK_CONFIG_REG,

	NLMDEV_BLOCK_MASK_0_0_REG,  /* BMR 0, segment[79:0] */
    NLMDEV_BLOCK_MASK_0_1_REG,  /* BMR 0, segment[159:80] */
    NLMDEV_BLOCK_MASK_0_2_REG,  /* BMR 0, segment[239:160]  */
    NLMDEV_BLOCK_MASK_0_3_REG,  /* BMR 0, segment[319:240] */

    NLMDEV_BLOCK_MASK_1_0_REG,  /* BMR 1, segment[79:0]  */
    NLMDEV_BLOCK_MASK_1_1_REG,  /* BMR 1, segment[159:80]  */
    NLMDEV_BLOCK_MASK_1_2_REG,  /* BMR 1, segment[239:160]  */
    NLMDEV_BLOCK_MASK_1_3_REG,  /* BMR 1, segment[319:240]  */

    NLMDEV_BLOCK_MASK_2_0_REG,
    NLMDEV_BLOCK_MASK_2_1_REG,
    NLMDEV_BLOCK_MASK_2_2_REG,
    NLMDEV_BLOCK_MASK_2_3_REG,

    NLMDEV_BLOCK_MASK_3_0_REG,
    NLMDEV_BLOCK_MASK_3_1_REG,
    NLMDEV_BLOCK_MASK_3_2_REG,
    NLMDEV_BLOCK_MASK_3_3_REG,

    NLMDEV_BLOCK_MASK_4_0_REG,
    NLMDEV_BLOCK_MASK_4_1_REG,
    NLMDEV_BLOCK_MASK_4_2_REG,
    NLMDEV_BLOCK_MASK_4_3_REG,

    NLMDEV_BLOCK_REG_END          /* must be the last element */

} NlmDevBlockRegType;

/* enum for block widths */
typedef enum NlmDevBlockWidth_e
{
	NLMDEV_BLK_WIDTH_80,
	NLMDEV_BLK_WIDTH_160,
	NLMDEV_BLK_WIDTH_320,
	NLMDEV_BLK_WIDTH_640
} NlmDevBlockWidth;


#define	NlmDevBlockConfigReg	Nlm11kDevBlockConfigReg

#define	NlmDevBlockMaskReg	Nlm11kDevBlockMaskReg

#define	NlmDevABEntry	Nlm11kDevABEntry


/* Structure used for AB Entry Write and Read operations
    Upto 8 consective locations can be read and written at a time
    */
#define NLMDEV_AB_WR_RD_MAX_LOCS         NLM11KDEV_AB_WR_RD_MAX_LOCS

#define	NlmDevMultiABEntryWrRdParam	Nlm11kDevMultiABEntryWrRdParam

#define	NlmDevCtxBufferInfo	Nlm11kDevCtxBufferInfo

#define	NlmDevCmpRslt	Nlm11kDevCmpRslt

#define	NlmDevMgr	Nlm11kDevMgr	

#define	NlmDev	Nlm11kDev	

#define	NlmDevMgr__create	Nlm11kDevMgr__create

#define	NlmDevMgr__destroy	Nlm11kDevMgr__destroy

#define	NlmDevMgr__AddDevice	Nlm11kDevMgr__AddDevice

#define	NlmDevMgr__ResetDevices	Nlm11kDevMgr__ResetDevices

#define	NlmDevMgr__GlobalRegisterRead	Nlm11kDevMgr__GlobalRegisterRead

#define	NlmDevMgr__GlobalRegisterWrite	Nlm11kDevMgr__GlobalRegisterWrite

#define	NlmDevMgr__CBAsRegisterRead	Nlm11kDevMgr__CBAsRegisterRead

#define	NlmDevMgr__CBAsRegisterWrite	Nlm11kDevMgr__CBAsRegisterWrite

#define	NlmDevMgr__LogicalTableRegisterRead	Nlm11kDevMgr__LogicalTableRegisterRead

#define	NlmDevMgr__LogicalTableRegisterWrite	Nlm11kDevMgr__LogicalTableRegisterWrite

#define	NlmDevMgr__LogicalTableRegisterRefresh	Nlm11kDevMgr__LogicalTableRegisterRefresh

#define	NlmDevMgr__BlockRegisterRead	Nlm11kDevMgr__BlockRegisterRead

#define	NlmDevMgr__BlockRegisterWrite	Nlm11kDevMgr__BlockRegisterWrite

#define	NlmDevMgr__ABEntryRead	Nlm11kDevMgr__ABEntryRead

#define	NlmDevMgr__ABEntryWrite	Nlm11kDevMgr__ABEntryWrite

#define	NlmDevMgr__ABEntryInvalidate	Nlm11kDevMgr__ABEntryInvalidate

#define	NlmDevMgr__MultiABEntryRead	Nlm11kDevMgr__MultiABEntryRead

#define	NlmDevMgr__MultiABEntryWrite	Nlm11kDevMgr__MultiABEntryWrite

#define	NlmDevMgr__MultiABEntryInvalidate	Nlm11kDevMgr__MultiABEntryInvalidate

#define	NlmDevMgr__ABEntryRefresh	Nlm11kDevMgr__ABEntryRefresh

#define	NlmDevMgr__Compare1	Nlm11kDevMgr__Compare1

#define	NlmDevMgr__Compare2	Nlm11kDevMgr__Compare2

#define	NlmDevMgr__CBWrite		Nlm11kDevMgr__CBWrite

#define	NlmDevMgr__LockConfig		Nlm11kDevMgr__LockConfig

#define	NlmDevMgr__RangeRegisterWrite		Nlm11kDevMgr__RangeRegisterWrite

#define	NlmDevMgr__RangeRegisterRead		Nlm11kDevMgr__RangeRegisterRead

#define	NlmDevMgr__ShadowDeviceCreate		Nlm11kDevMgr__ShadowDeviceCreate

#define	NlmDevMgr__ShadowDeviceDestroy		Nlm11kDevMgr__ShadowDeviceDestroy

#define	NlmDevMgr__RegisterWrite		Nlm11kDevMgr__RegisterWrite

#define	NlmDevMgr__RegisterRead		Nlm11kDevMgr__RegisterRead

#define	NlmDevMgr__SendNop		Nlm11kDevMgr__SendNop


#define	NlmDevMgr__DumpShadowMemory		Nlm11kDevMgr__DumpShadowMemory


#endif


#include "nlmcmexterncend.h"
#endif /* INCLUDED_NLM11KDEVMGR_H */

