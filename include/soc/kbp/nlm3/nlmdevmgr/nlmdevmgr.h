/*
 * $Id: nlmdevmgr.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 

/*@@NlmDevMgr Module
   Summary:
    Device manager is responsible for creating, managing and destroying device/s
    objects. Device manager should be created only once and must be destroyed when
    system of devices needs to be shutdown. Device manager provides many user
    interface functions to performs various device operations including the functions
    which creates and destroys device manager itself. 
*/

/* Revision: SDK 1.0a on 23 July,2012 */


#ifndef INCLUDED_NLMDEVMGR_H
#define INCLUDED_NLMDEVMGR_H

#if defined NLM_MT_OLD || defined NLM_MT
#ifndef NLMPLATFORM_BCM
#include <nlmcmmt.h>
#else
#include <soc/kbp/common/nlmcmmt.h>
#endif
#endif


/* include files */
#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmdebug.h>
#include <nlmcmdevice.h>
#include <nlmcmallocator.h>
#include <nlmarch.h>
#include <nlmerrorcodes.h>
#include <nlmcmexterncstart.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmdebug.h>
#include <soc/kbp/common/nlmcmdevice.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/nlm3/arch/nlmarch.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif


typedef enum NlmDevLogType_e
{
    NLMDEV_SM_DUMP_REG = 0,
    NLMDEV_SM_DUMP_DBA,
    NLMDEV_SM_DUMP_UDA,
    NLMDEV_SM_DUMP_ALL,

    NLMDEV_DEV_DUMP_REG = 10,
    NLMDEV_DEV_DUMP_DBA,
    NLMDEV_DEV_DUMP_UDA,
    NLMDEV_DEV_DUMP_ALL

} NlmDevLogType;

typedef struct NlmDevSbRange_s
{
    nlm_u16 m_stSBNr;       /* DBA/UDA start Super block number */
    nlm_u16 m_endSBNr;      /* DBA/UDA end Super block number */

} NlmDevSbRange;


/* enum for hit/miss flag */
typedef enum NlmDevMissHit_e
{
    NLMDEV_MISS = 0,
    NLMDEV_HIT

} NlmDevMissHit;

/* enum for result invalid/valid flag */
typedef enum
{
    NLMDEV_RESULT_INVALID = 0,
    NLMDEV_RESULT_VALID

} NlmDevResultValid;

/* enum for result response type with/out associated data */
typedef enum
{
    NLMDEV_INDEX_AND_NO_AD,
    NLMDEV_INDEX_AND_32B_AD,
    NLMDEV_INDEX_AND_64B_AD,
    NLMDEV_INDEX_AND_128B_AD,
    NLMDEV_INDEX_AND_256B_AD

} NlmDevRespType;

/* enum for enable/disable */
typedef enum NlmDevDisableEnable_e
{
    NLMDEV_DISABLE = 0,
    NLMDEV_ENABLE

} NlmDevDisableEnable;

/* enum for validate/invalidate entry */
typedef enum NlmDevValidateInvalidate_e
{
    NLMDEV_INVALIDATE = 0,
    NLMDEV_VALIDATE

} NlmDevValidateInvalidate;

/* enum for AB entries write modes. Data can be aritten in DM or XY mode */
typedef enum NlmDevWriteMode_e
{
    NLMDEV_DM =0,
    NLMDEV_XY

} NlmDevWriteMode;

/* enum for device Id. Maximum four devices are supported for cascading in a channel. */
typedef enum NlmDevId_e
{
    NLMDEV_DEV_ID0 =0,
    NLMDEV_DEV_ID1,
    NLMDEV_DEV_ID2,
    NLMDEV_DEV_ID3

}NlmDevId;


/* Global Register Types */
typedef enum NlmDevGlobalRegType_e
{
    NLMDEV_DEVICE_ID_REG,  /* Read only Reg */
    NLMDEV_DEVICE_CONFIG_REG,

    NLMDEV_ERROR_STATUS_REG, /* Read only Reg; Reading of this register clears the set bits */
    NLMDEV_ERROR_STATUS_MASK_REG,

    NLMDEV_SOFT_SCAN_WRITE_REG,
    NLMDEV_SOFT_ERROR_FIFO_REG,
    NLMDEV_ADV_FEATURE_SOFT_ERROR_REG,
    
    NLMDEV_LPT_ENABLE_REG,

    NLMDEV_SCRATCH_PAD0_REG,
    NLMDEV_SCRATCH_PAD1_REG,

    NLMDEV_RESULT0_REG, /* ------------- */
    NLMDEV_RESULT1_REG, /* Read only Reg */
    NLMDEV_RESULT2_REG, /* ------------- */

    NLMDEV_UDA_SOFT_ERROR_COUNT_REG,
    NLMDEV_UDA_SOFT_ERROR_FIFO_REG,

    NLMDEV_UDA_CONFIG_REG,

    NLMDEV_GLOBALREG_END        /* Must be the last element */

} NlmDevGlobalRegType;

/*
    Device Indentification Register contains information regarding database size
    of the device and major and minor revision number of the device. This is a
    read only register.
*/
typedef struct NlmDevIdReg_s
{
    nlm_u8  m_databaseSize; /* Specifies size of the device; Can have a value of 2 (512K) or 3 (1024K) */
    nlm_u8  m_majorDieRev;/* Specifies major die rev; Can have a value from 0 - 7 (RA - RH) */
    nlm_u8  m_minorDieRev;/* Specifies minor die rev; Can have a value from 0 - 7 (01 - 08) */

} NlmDevIdReg;


/*
    This structure is used for error status register.
    Error status register should be read to know the type of error which has occured;
    Errors are reported via interrupt signal caused on GIO_L pin of the device;
    The same structure can be used to write to Error Status Mask Register in order
    to mask off the specified errors that are reported in which case value of 0 disables
    the error reporting while value of 1 enables the error reporting .
    Note: Error Status Register is read only Register
*/
typedef struct NlmDevErrStatusReg_s
{
    NlmBool m_globalGIO_L0_Enable;  /* This field is valid only for Error Status Mask Register
                                        Value of 0 -- Globally Disabales GIO_L[0] assertions  ;
                                         1 -- Globally Enables GIO_L[0] assertions  */
    NlmBool m_globalGIO_L1_Enable;  /* This field is valid only for Error Status Mask Register
                                Value of 0 -- Globally Disabales GIO_L[1] assertions  ;
                                 1 -- Globally Enables GIO_L[1] assertions  */
    NlmBool m_dbSoftError; /* Value of 0 -- There is no entry in Database Soft Error FIF0;
                                       1 -- There are one or more entries in Database Soft Error FIF0;*/
    NlmBool m_dbSoftErrorFifoFull;/* Value of 0 -- Database Soft Error FIFO is not full;
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

} NlmDevErrStatusReg;

/*
    structure for database soft parity error fifo register
*/
typedef struct NlmDevDbSoftErrFifoReg_s
{
    nlm_u32 m_errorAddr;    /* Address of the Database Entry with parity error; This Field is read only */
    NlmBool m_pErrorX;      /* If set to 1 indicates that the error is in the X part of the entry;
                                This Field is read only*/
    NlmBool m_pErrorY;      /* If set to 1 indicates that the error is in the Y part of the entry;
                                This Field is read only*/
    NlmBool m_errorAddrValid;   /* If set to 1 indicates this is a valid error address
                                     if set to 0 indicates this FIFO is empty;
                                     This Field is read only */
    NlmBool m_eraseFifoEntry;   /* Writing 1 to this field will erase the current entry
                                and advance the pointer to next FIFO entry
                                     This Field is write only */
    NlmBool m_eraseFifo;    /* Writing 1 to this field erase all the entries in the FIFO
                                    This Field is write only */
}NlmDevDbSoftErrFifoReg;

/*
    structure for advanced featured soft error register
    Note: Advanced featured soft error registe is read only Register
*/
typedef struct NlmDevAdvancedSoftErrReg_s
{
    nlm_u16 m_cbParityErrAddr;  /* Indicates the location of Ctx Buffer with parity error */
    nlm_u16 m_sahasraParityErrAddr0;    /* Indicates the location of Sahasra Engine parity error 0*/
    nlm_u16 m_sahasraParityErrAddr1;    /* Indicates the location of Sahasra Engine parity error 1*/
    nlm_u16 m_ltrParityErrAddr; /* Indicates the location of LTR parity error;
                                    In this case Bits[10:5] indicates the LTR Number
                                    and Bits[4:0] indicates the LTR Reg Type
                                    i.e If value of this field is b'0111100004
                                    Then Parity Error has occured for LTR #15,
                                         and Reg Type NLM11KDEV_PARALLEL_SEARCH_1_LTR
                                    Note: In case of compare LTR Reg Type is not specified*/
                                    
 }NlmDevAdvancedSoftErrReg;

/*
    Device configuration register stores basic configuration attributes of the
    device as low power mode status, soft parity error mode, range matching engine enable and so on.
*/
typedef struct NlmDevConfigReg_s
{
    nlm_u8  m_port1CtxIDShift;
    nlm_u16 m_ACtoBankMapping;

    NlmDevDisableEnable m_rangeEnable;

    NlmDevDisableEnable m_port1Enable;
    NlmDevDisableEnable m_port0Enable;

    NlmPortMode         m_dualPortMode;
    NlmSMTMode          m_dualBankMode;
    nlm_u8              m_CBConfig;

    NlmDevDisableEnable m_dbParityErrEntryInvalidate;
    NlmDevDisableEnable m_softErrorScanEnable;

    nlm_u8              m_lastDevice; /* 0 not last device. 1 means last device */
    nlm_u8              m_firstDevice; /* 0 not first device. 1 means first device */

#if 1
    NlmDevDisableEnable m_dbSoftErrProtectMode; /* Value of 0 -- Use Parity for Database Soft Error Protection;
                                                                    1 -- Use ECC for Database Soft Error Protection;
                                                                    This field ignored if m_softErrorScan is disabled; */
    NlmDevDisableEnable m_eccScanType;  /* Value of 0 -- set ECC scan to 1b detect and 1b correct;
                                                            1 -- set ECC scan to 2b detect and 1b correct; */
    NlmDevDisableEnable m_rangeEngineEnable; /* Value of 0 -- Disables Range Matching Engine;
                                                                1 -- Enables Range Matching Engine;*/
    NlmDevDisableEnable m_lowPowerModeEnable;   /* Value of 0 -- low power mode disabled;
                                                            1 -- low power mode enabled; */
#endif
} NlmDevConfigReg;


/*
    structure for scratch pad register 0 and 1
*/
typedef struct NlmDevScratchPadReg_s
{
    nlm_u8  m_data[NLMDEV_REG_LEN_IN_BYTES];

} NlmDevScratchPadReg;

/*
    structure for result register 0 and 1.
    it stores the result of the latest compare instruction done on the device
    Result Register is read only Register
*/
typedef struct NlmDevResultReg_s
{
    /* hit or miss flag */
    NlmDevMissHit   m_hitOrMiss[NLMDEV_NUM_PARALLEL_SEARCHES/2]; /* specifies Hit or Miss for the search */

    /* Hit Address */
    nlm_u32             m_hitAddress[NLMDEV_NUM_PARALLEL_SEARCHES/2];/* In case of Hit contains Hit Address for the search*/

} NlmDevResultReg;


/*
    structure for UDA configuration register to eanble/disable SBs
*/
typedef struct NlmDevUDAConfigReg_s
{
    NlmDevDisableEnable m_uSBEnable[NLMDEV_NUM_SRAM_SUPER_BLOCKS / 4];

} NlmDevUDAConfigReg;


/*
    structure definition for accessing Context Buffer as register
*/
typedef struct NlmDevCtxBufferReg_s
{
    nlm_u8  m_data[NLMDEV_REG_LEN_IN_BYTES]; /* 80 bit CB Register data */

} NlmDevCtxBufferReg;

/*
    structure definition for accessing Range Global Register
*/
typedef struct NlmDevRangeReg_s
{
    nlm_u8 m_data[NLMDEV_REG_LEN_IN_BYTES];
}NlmDevRangeReg;
/*
    LTR Register Types.
*/
typedef enum NlmDevLtrRegType_e
{
    NLMDEV_BLOCK_SELECT_0_LTR,
    NLMDEV_BLOCK_SELECT_1_LTR,
    NLMDEV_BLOCK_SELECT_2_LTR,
    NLMDEV_BLOCK_SELECT_3_LTR,

    NLMDEV_PARALLEL_SEARCH_0_LTR,
    NLMDEV_PARALLEL_SEARCH_1_LTR,
    NLMDEV_PARALLEL_SEARCH_2_LTR,
    NLMDEV_PARALLEL_SEARCH_3_LTR,
    NLMDEV_PARALLEL_SEARCH_4_LTR,
    NLMDEV_PARALLEL_SEARCH_5_LTR,
    NLMDEV_PARALLEL_SEARCH_6_LTR,
    NLMDEV_PARALLEL_SEARCH_7_LTR,

    NLMDEV_SUPER_BLK_KEY_MAP_LTR,

    NLMDEV_EXT_CAPABILITY_REG_0_LTR,
    NLMDEV_EXT_CAPABILITY_REG_1_LTR,

    NLMDEV_KEY_0_KCR_0_LTR,
    NLMDEV_KEY_0_KCR_1_LTR,
    NLMDEV_KEY_1_KCR_0_LTR,
    NLMDEV_KEY_1_KCR_1_LTR,
    NLMDEV_KEY_2_KCR_0_LTR,
    NLMDEV_KEY_2_KCR_1_LTR,
    NLMDEV_KEY_3_KCR_0_LTR,
    NLMDEV_KEY_3_KCR_1_LTR,

    NLMDEV_OPCODE_EXT_LTR,

    NLMDEV_RANGE_INSERTION_0_LTR,
    NLMDEV_RANGE_INSERTION_1_LTR,

    NLMDEV_SS_LTR,      /* 11K Specific */

    NLMDEV_LTR_REG_END                /* must be the last element */

} NlmDevLtrRegType;

/*
    Structure for Block select register 0, 1, 2, and 3. Block select register stores the information
    if a block is disabled or enabled for search operations.
*/
typedef struct NlmDevBlkSelectReg_s
{
    NlmDevDisableEnable m_blkEnable[NLMDEV_NUM_ARRAY_BLOCKS / 4];

} NlmDevBlkSelectReg;

/*
    Structure for Super Block to Key map register .
    This registers store the info about which super block is mapped to
    which key.
*/
typedef struct NlmDevSuperBlkKeyMapReg_s
{
    NlmDevKey m_keyNum[NLMDEV_NUM_SUPER_BLOCKS];

} NlmDevSuperBlkKeyMapReg;

/*
    Structure for Parallel search register 0,1,2 and 3.
    These registers store the info about which array block is mapped to
    which parallel search.
*/
typedef struct NlmDevParallelSrchReg_s
{
    NlmDevParallelSrch  m_psNum[NLMDEV_NUM_ARRAY_BLOCKS / 8];

} NlmDevParallelSrchReg;


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

/* Structure for Range Insertion 0 and Range Insertion 1 registers;
Contains information about type of encoding to be used;
Location of Range Encoding Insertion in various keys and number of bytes of range encoded value
to be inserted */
typedef struct NlmDevRangeInsertion0Reg_s
{
    NlmDevRangeEncodingType m_rangeAEncodingType; /* Specifies type of range encoding to be used for Range A */
    NlmDevRangeEncodingType m_rangeBEncodingType; /* Specifies type of range encoding to be used for Range B */
    NlmDevRangeEncodedValueBytes m_rangeAEncodedBytes;   /* Specifies number of bytes of Range A encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/
    NlmDevRangeEncodedValueBytes m_rangeBEncodedBytes;   /* Specifies number of bytes of Range B encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/

    nlm_u8 m_rangeAInsertStartByte[NLMDEV_NUM_KEYS]; /* Specifies the start byte where the Range A Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLMDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
    nlm_u8 m_rangeBInsertStartByte[NLMDEV_NUM_KEYS];/* Specifies the start byte where the Range B Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLMDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
}NlmDevRangeInsertion0Reg;

typedef struct NlmDevRangeInsertion1Reg_s
{
    NlmDevRangeEncodingType m_rangeCEncodingType; /* Specifies type of range encoding to be used for Range C */
    NlmDevRangeEncodingType m_rangeDEncodingType; /* Specifies type of range encoding to be used for Range D */
    NlmDevRangeEncodedValueBytes m_rangeCEncodedBytes;   /* Specifies number of bytes of Range C encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/
    NlmDevRangeEncodedValueBytes m_rangeDEncodedBytes;   /* Specifies number of bytes of Range D encoding to be
                                                                 inserted in to the key(s); Supported values of 0 - 3(1 byte - 4 bytes)*/

    nlm_u8 m_rangeCInsertStartByte[NLMDEV_NUM_KEYS]; /* Specifies the start byte where the Range C Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLMDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
    nlm_u8 m_rangeDInsertStartByte[NLMDEV_NUM_KEYS];/* Specifies the start byte where the Range D Encoding needs to be inserted
                                                        in each key; Supported values 0 - 79;
                                                        Value of NLMDEV_RANGE_DO_NOT_INSERT means Do Not Insert */
}NlmDevRangeInsertion1Reg;

/*
    Key Construction register stores information about how the individual parallel search keys
    needs to be generated from the master(compare) key passed withe the compare instruction(s).
*/
typedef struct NlmDevKeyConstructReg_s
{
    nlm_u8 m_startByteLoc[NLMDEV_NUM_OF_SEGMENTS_PER_KCR]; /* Specifies start byte of each segment in the compare key;
                                                                Valid values  0 - 79 */
    nlm_u8 m_numOfBytes[NLMDEV_NUM_OF_SEGMENTS_PER_KCR];/* Specifies number of bytes each segment comprises of;
                                                                Valid values 1- 16; Value of zero indicates that there
                                                                are no more valid segments and device manager can ignore
                                                                next segments */
    nlm_u8 m_isZeroFill[NLMDEV_NUM_OF_SEGMENTS_PER_KCR]; /*Specifies if the Key should be filled with zeroes */
} NlmDevKeyConstructReg;

/* 11K Specific */
typedef enum Nlm1DevSearchType_e
{
    NLMDEV_STANDARD = 0,
    NLMDEV_SAHASRA  = 2 /* 2'b10 means Sahasra search. */

} NlmDevSearchType;



/* Extended Capability-0 Register */
typedef struct NlmDevExtCap0Reg_s
{
    nlm_u8 m_bmrSelect[NLMDEV_NUM_PARALLEL_SEARCHES]; /* Specifies BMR number to be used for each parallel search;
                                                         Valid values 0 - 4 and NLM_NO_MASK_BMR_NUM */

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

    NlmDevSearchType m_searchType[NLMDEV_NUM_PARALLEL_SEARCHES];    /* 11K Specific */

} NlmDevExtCap0Reg;


typedef struct NlmDevGeneralReg_s
{
    nlm_u8 m_data[NLMDEV_REG_LEN_IN_BYTES];

}NlmDevGeneralReg;


/* Result type enums */
typedef enum NlmDevResultType_e
{
    NLMDEV_INDEX_ONLY,      /* Return only the hit index*/
    NLMDEV_INDEX_AND_AD     /* Return associated data and hit index*/

} NlmDevResultType;


/* Associated Data lengths */
typedef enum NlmDevADLength_e
{
    NLMDEV_ADLEN_32B,
    NLMDEV_ADLEN_64B,
    NLMDEV_ADLEN_128B,
    NLMDEV_ADLEN_256B

} NlmDevADLength;

/* Hit index type; DBA hit index or Translated Address */
typedef enum NlmDevIndexType_e
{
    NLMDEV_DBA_INDEX,
    NLMDEV_TRANSLATED_ADDR

} NlmDevIndexType;

/* OpCodeExt Register */
typedef struct NlmDevOpCodeExtReg_s
{
    NlmDevResultType    m_resultType[NLMDEV_NUM_PARALLEL_SEARCHES]; /* Just index or Index + AD */
    NlmDevADLength  m_ADLen[NLMDEV_NUM_PARALLEL_SEARCHES];     /* Associated Data lenght */
    nlm_u32         m_lclOpCode;

} NlmDevOpCodeExtReg;


/*
    Block Register Set
*/
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

/* enum for shift direction */
typedef enum NlmDevShiftDir_e
{
    NLMDEV_SHIFT_RIGHT = 0,
    NLMDEV_SHIFT_LEFT

} NlmDevShiftDir;

/* enum for shift count */
typedef enum NlmDevShiftCount_e
{
    NLMDEV_SHIFT_CNT_0 = 0,
    NLMDEV_SHIFT_CNT_1,
    NLMDEV_SHIFT_CNT_2,
    NLMDEV_SHIFT_CNT_3

} NlmDevShiftCount;

/*  Block configuration register stores information about block enable status,
    and block width; There is only one BCR par array block.
*/
typedef struct NlmDevBlockConfigReg_s
{  
    NlmDevDisableEnable     m_blockEnable;  /* specifies whether the blk is enabled/disabled */
    NlmDevBlockWidth       m_blockWidth;   /* specifies width of the block */

    NlmDevShiftDir          m_shiftDir;     /* 0 for shift right, 1 for shift left */
    nlm_u32                 m_baseAddr;
    NlmDevShiftCount        m_shiftCount;   /* 0 no shift, 1 shift 1 bit, 2 shift 2 bits and 3 shift 3 bits */

} NlmDevBlockConfigReg;

/*  Block mask register stores mask bits which masks of the specified bits of the Database from compare operations
    Each block can have 5 BMRs where each BMR has 4 segments of 80b;
    Segment 0 -- Used to mask bits[79:0]
    Segment 1 -- Used to mask bits[159:80]
    Segment 2 -- Used to mask bits[239:160]
    Segment 3 -- Used to mask bits[319:240]
 */
typedef struct NlmDevBlockMaskReg_s
{
    nlm_u8  m_mask[NLMDEV_REG_LEN_IN_BYTES];

} NlmDevBlockMaskReg;

/* Structure Array Block Entry consists of 80b data, 80b mask and a valid bit.
*/
typedef struct NlmDevABEntry_s
{
    nlm_u8  m_data[NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8  m_mask[NLMDEV_AB_WIDTH_IN_BYTES];
    nlm_u8  m_vbit; /* In case of read specifies whether the entry is valid or deleted */

} NlmDevABEntry;


/* Structure for Exact Match write/read */
typedef struct NlmDevEMEntry_s
{
    nlm_u8  m_data[NLM_EM_RECORD_WIDTH_IN_BYTES];
    nlm_u8  m_vbit; /* In case of read, specifies whether the entry is valid or deleted */

} NlmDevEMEntry;

typedef enum NlmDevCBInstType_e
{
    NLMDEV_CB_INST_CMP1,  /* CB Write and Compare 1 */
    NLMDEV_CB_INST_CMP2,  /* CB Write and Compare 2 */
    NLMDEV_CB_INST_LPM,   /* CB Write and LPM */
    NLMDEV_CB_INST_NONE   /* This does not represent any instruction and is used for marking end of the enum */ 
    
} NlmDevCBInstType;


/*
    Structure which contains the CB Data passed with CB Write, Compare1 and Compare 2 operations
*/
typedef struct NlmDevCtxBufferInfo_s
{
    nlm_u16 m_cbStartAddr;  /* Specifies the CB Address at which the data is to be written;
                            In case of cmp operations it also specifies the 40b segment of
                            CB memory used for the compare key */
    nlm_u8 m_datalen; /* specifies amount of data in terms of bytes to be written to CB memory */
    nlm_u8  m_data[NLMDEV_CB_KEYLEN_BYTES];   /* Maximum of 640b (80bytes) of CB data can be written */

} NlmDevCtxBufferInfo;

/* Structure used for performing 2 compares on both SMTs simultaneously */
typedef struct NlmDevCBWriteCmpParam_s
{
    NlmDevCBInstType    m_cbInstType0; /* Inst type for SMT-0 */
    NlmDevCBInstType    m_cbInstType1; /* Inst type for SMT-1 */
    nlm_u8          m_ltrNum0;        /* LTR number for SMT-0 compare */
    nlm_u8              m_ltrNum1;        /* LTR number for SMT-1 compare */
    NlmDevCtxBufferInfo  m_cbData0;     /* Context Buffer info for SMT-0 compare */
    NlmDevCtxBufferInfo  m_cbData1;     /* Context Buffer info for SMT-1 compare */

} NlmDevCBWriteCmpParam;


/* Summary
    structure definition used to return upto four parallel search results of compare operations
*/
typedef struct NlmDevCmpResult_s
{
    NlmDevResultValid   m_resultValid[NLM_MAX_NUM_RESULTS]; /* result is valid or invalid */
    NlmDevMissHit       m_hitOrMiss[NLM_MAX_NUM_RESULTS];       /* hit or miss flag*/
    NlmDevRespType  m_respType[NLM_MAX_NUM_RESULTS];  /* result response type */

    nlm_u8          m_hitDevId[NLM_MAX_NUM_RESULTS];        /* specifies the hit dev id */
    nlm_u32         m_hitIndex[NLM_MAX_NUM_RESULTS];        /* specifies the hit index  */

    nlm_u8          m_AssocData[NLM_MAX_NUM_RESULTS][NLMDEV_MAX_AD_LEN_IN_BYTES];

} NlmDevCmpResult;

/* Structure used for performing block operations:
   1. Block copy
   2. Block move
   3. Block clear
   4. Block entry validate/invalidate
 */
typedef struct NlmDevBlockOperParam_s
{
    NlmBlkInstType      m_instType;     /* copy [0], move [1], clear[2], and entry validate [3] */

    nlm_u16             m_numOfWords;   /* Number of 80 DB entries; 0 means 1 80b entry,
                                           Maximum value is 4,095 which means 4096 80b entries*/

    NlmBlkCountDir      m_countDir;     /* Count Direction, Used only with BLKCPY and BLKMV
                                             1b0 = Count up  : 0, 1, 2, etc.
                                             1b1 = Count down: 4, 3, 2, etc.  */

    nlm_u32             m_srcAddr;      /* Source Address - Used for all 4 instuctions */

    nlm_u32             m_destAddr;     /* Destination Address - Used only with BLKCPY and BLKMV */

    NlmDevValidateInvalidate m_setOrClear;      /* Entry validate or invalidate, Used only with BLKEVI
                                                   1b0 = Invalidate the entry,
                                                   1b1 = Validate the entry */
} NlmDevBlockOperParam;


/* Shadow Memory definitions */
/* Global registers.  */
typedef struct NlmDevShadowGlobal_s
{
    NlmDevConfigReg             m_devConfig;
    NlmDevUDAConfigReg          m_devUDAConfig;

}NlmDevShadowGlobal;


#define NLMDEV_SS_RMP_AB        8


typedef enum NlmDevSSRmap_e
{
    NLMDEV_MAP_PE0 = 0,
    NLMKDEV_MAP_PE1

} NlmDevSSRmap;


typedef struct NlmDevSSReg_s
{
    NlmDevSSRmap    m_ss_result_map[NLMDEV_SS_RMP_AB];

} NlmDevSSReg;


/* Ltr registers.  */
typedef struct NlmDevShadowLtr_s
{
    NlmDevBlkSelectReg          m_blockSelect[NLMDEV_NUM_ARRAY_BLOCKS/64];
    NlmDevSuperBlkKeyMapReg     m_superBlkKeyMap;
    NlmDevParallelSrchReg       m_parallelSrch[NLMDEV_NUM_ARRAY_BLOCKS/32];
    NlmDevKeyConstructReg       m_keyConstruct[NLMDEV_NUM_KEYS * NLM_LTR_NUM_OF_KCR_PER_KPU];
    NlmDevExtCap0Reg            m_extCap0;          /* m_miscelleneous */
    NlmDevOpCodeExtReg          m_opCodeExt;
    NlmDevGeneralReg            m_internalReg1;
    NlmDevRangeInsertion0Reg    m_rangeInsert0;
    NlmDevRangeInsertion1Reg    m_rangeInsert1;
    
    NlmDevSSReg              m_ssReg;       /* 11K specific */

}NlmDevShadowLtr;


/* Contents of Array Block */
typedef struct NlmDevShadowAB_s
{
    NlmDevABEntry           m_abEntry[NLMDEV_AB_DEPTH];
    NlmDevBlockConfigReg    m_blkConfig;
    NlmDevBlockMaskReg      m_bmr[NLMDEV_NUM_BMRS_PER_AB][NLMDEV_NUM_80BIT_SEGMENTS_PER_BMR];   

}NlmDevShadowAB;


typedef struct NlmDevShadowUdaEntry
{
    nlm_u8 m_data[NLM_MIN_SRAM_WIDTH_IN_BYTES];
}NlmDevShadowUdaEntry;


typedef struct NlmDevShadowUdaBlock
{
    NlmDevShadowUdaEntry m_entry[NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK];

}NlmDevShadowUdaBlock;


typedef struct NlmDevShadowUdaSb
{
    NlmDevShadowUdaBlock m_udaBlk[NLMDEV_NUM_SRAM_BLOCKS_IN_SB];

}NlmDevShadowUdaSb;



/* Shadow chip structure - contains everything as actual chip except global registers and range registers */
typedef struct NlmDevShadowDevice_s
{
    NlmDevShadowLtr    *m_ltr;          /* pointer to an array of "NLMDEV_MAX_NUM_LTRS" NlmDevShadowLtr */
    NlmDevShadowAB     *m_arrayBlock;   /* pointer to an array of "NLMDEV_NUM_ARRAY_BLOCKS" NlmDevShadowAB */
    void                     *m_st;        /* pointer to an advanced search memory */   
    NlmDevRangeReg     *m_rangeReg; /* pointer to range registers */
    NlmDevShadowGlobal *m_global;       /* pointer to NlmDevShadowGlob */
    
}NlmDevShadowDevice;

/* Some macros which helps application/devmgr to access Shadow Device */

#define NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev) (NlmDevShadowDevice*)(dev->m_shadowDevice_p)
#define NLM_GET_SHADOW_MEM_FROM_DEVMGR_PTR(devMgr, devId) (NlmDevShadowDevice*)(((NlmDev*)(devMgr->m_devList_pp)[devId])->m_shadowDevice_p)


/* Summary
    Device manager structure contains list of all devices, device count and a pointer to
    memory allocator which is used to allocate memory for all the operations done by device
    manager module.
*/
typedef struct NlmDevMgr_s
{
    NlmCmAllocator *m_alloc_p;  /* memory allocator */
    nlm_u32     m_devCount;     /* count of devices in the list */
    void**      m_devList_pp;   /* list of all devices */
    void*       m_xpt_p;        /* transport layer pointer for internal purpose. */
    NlmBool     m_isLocked;     /* specifies whether device manager configuration is locked or not */
    NlmDevType  m_devType;      /* Device type (Standard or NetRoute) */
    nlm_u32     m_magicNum;     /* Magic Number to check the validity of device manager pointer */

    NlmPortMode m_portMode; /* 1-port or 2-port */
    NlmSMTMode m_smtMode;     /* no-SMT mode or 2-SMT mode */
    nlm_u32     m_lpmCmp3[NLMDEV_MAX_NUM_LTRS / 32]; /* Bit is set for CMPLPM on Key#0 incase of CMP3 */

    void*       m_11kDevMgr_p;
    nlm_u16     m_numOfAbs;
    nlm_u8      m_numOfABsPerSB;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMtSpinlock     m_spinLock; /* Spin lock to protect data consistency with multi-threading*/
#endif
} NlmDevMgr;

/*
    It contains information regarding the devices added to the cascade system;
    Specifically it contains Device ID, pointer to device manager,
    cascade information and shadow device pointer. Shadow device is a software copy
    of actual device including all registers and database. Use of shadow device
    is vital and explained separately in nlmdevmgr_shadow.h file.
*/
typedef struct NlmDev_s
{
    NlmDevMgr   *m_devMgr_p;     /* device manager to which the device belongs to */
    NlmDevId     m_devId;        /* device ID as defined in the enum above */
    nlm_u32      m_magicNum;     /* Magic Number to check the validity of device pointer */
    NlmDevShadowDevice      *m_shadowDevice_p;  /*  pointer to shadow memory of the device*/
    nlm_u16 m_bankNum; /* Database Array Block to Bank Mapping */

} NlmDev;


/* functions exposed by DevMgr Module
 for reason code details, user must not pass NULL. if o_reason is NULL, user
 will not be get reason codes for failures.
*/
/*
    NlmDevMgr__create creates Device manager using memory allocator which must be passed
    as parameter. Function returns NULL if fails and user should see the reason code in
    case of failure.
*/
NlmDevMgr* NlmDevMgr__create(
    NlmCmAllocator *alloc_p,
    void*           xpt_p,
    NlmDevType      devType, /* Standard or NetRoute */
    NlmPortMode     portMode,
    NlmSMTMode      smtMode,
    NlmReasonCode*  o_reason
    );

/*
    NlmDevMgr__destroy deletes Device manager and all devices in the device manager list.
*/
extern void NlmDevMgr__destroy(
    NlmDevMgr *devMgr_p
    );

/*
    NlmDevMgr__AddDevice creates an software instance of device and adds to device list of device manager.
    If this function fails it returns NULL . User should see the reason code in case of failure.
*/
extern NlmDev* NlmDevMgr__AddDevice(
    NlmDevMgr    *self,
    NlmDevId            *o_devId,       /* device id assigned by the device manager for the device being added*/
    NlmReasonCode*          o_reason
    );

/*
    NlmDevMgr__ResetDevice resets all the devices in cascade to have initial values.
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__ResetDevices(
    NlmDevMgr                       *self,
    NlmReasonCode*                  o_reason
    );

/*
    NlmDevMgr__GlobalRegisterRead reads any global register depending upon the register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__GlobalRegisterRead(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    NlmDevGlobalRegType         regType,        /* Global register type - see definitions above */
    void                            *o_data,    /* Global register structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    NlmDevMgr__GlobalRegisterWrite writes any global register depending upon the register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__GlobalRegisterWrite(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    NlmDevGlobalRegType         regType,        /* Global register type - see definitions above */
    const void                      *data,      /* Global register structure pointer */
    NlmReasonCode*                  o_reason
    );

/*
    NlmDevMgr__CBAsRegisterRead reads context buffer as register read.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__CBAsRegisterRead(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                 cbAddr,     /* Only the offset address of the CB needs to be provided
                                                                          i.e 0 - 16383 (NLMDEV_CB_DEPTH - 1)*/
    NlmDevCtxBufferReg          *o_cbRegData,       /* see the structure description above. */
    NlmReasonCode*          o_reason
    );

/*
    NlmDevMgr__CBAsRegisterWrite writes context buffer as register write.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__CBAsRegisterWrite(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                 cbAddr,     /* Only the offset address of the CB needs to be provided
                                                                          i.e 0 - 16383 (NLMDEV_CB_DEPTH - 1)*/
    NlmDevCtxBufferReg          *cbRegData,         /* see the structure description above */
    NlmReasonCode*          o_reason
    );

/*
    NlmDevMgr__LogicalTableRegisterRead reads LTR register depending on LTR register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__LogicalTableRegisterRead(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                          ltrNum, /* LTR profile set number */
    NlmDevLtrRegType                regType,        /* see the structure description above */
    void                            *o_data,    /* LTR register type structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    NlmDevMgr__LogicalTableRegisterWrite writes LTR register depending on LTR register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__LogicalTableRegisterWrite(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                          ltrNum, /* LTR profile set number */
    NlmDevLtrRegType                regType,        /* see the structure description above */
    const void                      *data,      /* LTR register type structure pointer */
    NlmReasonCode*                  o_reason
    );


/*
    NlmDevMgr__BlockRegisterRead reads block register depending on block register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__BlockRegisterRead(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                     abNum,      /* AB number in which register lies */
    NlmDevBlockRegType          regType,        /* see the structure description above */
    void                            *o_data,    /* Block register structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    NlmDevMgr__BlockRegisterWrite writes block register depending on block register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__BlockRegisterWrite(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                         abNum,      /* AB number in which register lies */
    NlmDevBlockRegType          regType,        /* see the structure description above */
    const void                      *data,      /* Block register structure pointer */
    NlmReasonCode*                  o_reason
    );


/*
    NlmDevMgr__DBAWrite writes single Array Block Database entry
    to the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data can be written in either DM or XY mode
    User should see the reason code in case of failure.
*/
NlmErrNum_t NlmDevMgr__DBAWrite(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the AB Number */
    nlm_u16 abAddr, /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry *abEntry,
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    );

/*
    NlmDevMgr__DBARead readss single Array Block Database entry
    from the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data read will available as type NlmDevABEntry, always XY mode
    User should see the reason code in case of failure.
*/
NlmErrNum_t NlmDevMgr__DBARead(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry   *o_abEntry,
    NlmReasonCode*      o_reason
    );

/*
    NlmDevMgr__DBAInvalidate invalidates specified array block database entry
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__DBAInvalidate(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16             abNum,                  /* AB number */
    nlm_u16             abAddr,                 /* entry index within the AB */
    NlmReasonCode*      o_reason
    );


/*
    NlmDevMgr__UDAWrite writes single UDA Database (Associated Data) entry of 
    width 32b (only LSB 32b are valid) specified addr. 
    User should see the reason code in case of failure.
*/
NlmErrNum_t NlmDevMgr__UDAWrite(
    void            *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u32         address,        
    void            *data,
    nlm_u8          length,
    NlmReasonCode*  o_reason
    );


/*
    NlmDevMgr__EMWrite API can be used to add Exact Match entry at the specified
    address within a DBA block. Write mode (whether WRA or WRB) is specified by
    the input parameter 'writeMode'.
*/
NlmErrNum_t NlmDevMgr__EMWrite(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,      /* Specifies the DBA Block Number */
    nlm_u16         abAddr,     /* Specifies the entry location within the specified abNum  */
    NlmDevEMEntry   *emEntry,
    NlmEMWriteMode  writeMode,
    NlmReasonCode*  o_reason
    );

    
/*  NlmDevMgr__EMRead API can be used to read Exact Match entry at the specified
    address within a DBA block. Read mode (whether RDA or RDB) is specified by
    the input parameter 'readMode'.  Output is available in the output parameter
    'o_emEntry'
*/
NlmErrNum_t NlmDevMgr__EMRead(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,    /* Specifies the DBA Block Number */
    nlm_u16         abAddr,  /* Specifies the entry location within the specified abNum  */
    NlmEMReadMode   readMode,  /* Read-A or Read-B */
    NlmDevEMEntry   *o_emEntry,
    NlmReasonCode*  o_reason
    );
        

/*
    NlmDevMgr__UDARead reads single UDA Database (Associated Data) entry of 
    width 32b (only LSB 32b are valid) from specified addr. 
    User should see the reason code in case of failure.
*/
NlmErrNum_t NlmDevMgr__UDARead(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,        
    void                *o_data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    );


/*
    NlmDevMgr__Compare1 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, 4 Keys and 4 Result ports available
        Single Bank mode:
            0-127 for BANK-0, 4 Keys and 4 result ports

        Dual Bank:
            0-63   for BANK-0, 2 Keys (KEY 0/1) and 2 result ports (0/1)
            64-127 for BANK-1, 2 Keys (KEY 2/3) and 2 result ports (2/3)
    )
    and gives hit/miss and address as an output.
    Compare1 instruction can be used for searches with individual search key lengths of upto 320b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t NlmDevMgr__Compare1(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );

/*
    NlmDevMgr__Compare2 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, 4 Keys and 4 Result ports available
        Single Bank mode:
            0-127 for BANK-0, 4 Keys and 4 result ports

        Dual Bank:
            0-63   for BANK-0, 2 Keys (KEY 0/1) and 2 result ports (0/1)
            64-127 for BANK-1, 2 Keys (KEY 2/3) and 2 result ports (2/3)
    )
    and gives hit/miss and address as an output.
    Compare2 instruction can be used for searches with individual search key lengths of upto 640b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t NlmDevMgr__Compare2(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );

/*
    NlmDevMgr__CompareLPM is a special search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, always uses Key- 0 and Result port-0
        Single Bank mode:
            0-127 for BANK-0, Key-0 and result port-0

        Dual Bank:
            0-63   for BANK-0, KEY-0 and result ports-0
    )
    and gives hit/miss and address as an output.
    CompareLPM instruction can be used for searches with individual search key lengths of upto 160b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t NlmDevMgr__CompareLPM(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );


NlmErrNum_t NlmDevMgr__Compare3(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR  number to be used */
    NlmDevCtxBufferInfo *cbInfo,   /* Search key details */
    NlmDevCmpResult     *o_search_results, /* Result details */
    NlmReasonCode*      o_reason
    );

/*
    NlmDevMgr__Compare function is a special API provided to perform compares on
    both SMTs. There is no corresponding instruction available in the device. This API
    can be used to perform searches on both SMTs simultaneously and get results
    simultaneously. Other APIs (__Compare1/__Compare2/__CompareLPM) should
    be used if there only one search on either SMT. 

    This API should be used in dual bank (2-SMT) mode only. Up to 4 search results are 
    returned in the 'o_search_results' output parameter. Input parameter 'cbInfo'
    should be filled with appropriate input search data.
*/
NlmErrNum_t NlmDevMgr__Compare(
    NlmDevMgr               *self,
    nlm_u8                  portNum,            /* Port number */
    NlmDevCBWriteCmpParam   *cbInfo,                /* Search data for compares  */
    NlmDevCmpResult         *o_search_results,  /* Search results */
    NlmReasonCode*           o_reason
    );
    

/*
    NlmDevMgr__CBWrite writes to context buffer memory
    Upto 640 bytes of data can be written to CB memory;
    see desription of NlmDevCtxBufferInfo for more details
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t NlmDevMgr__CBWrite(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    NlmDevCtxBufferInfo     *cbInfo,                /* see the structure description above */
    NlmReasonCode*              o_reason
    );


/*
    NlmDevMgr__LockConfig locks the device manager configuration.
    Once the configuration is locked no more devices can be added.
*/
extern NlmErrNum_t NlmDevMgr__LockConfig(
    NlmDevMgr *self,
    NlmReasonCode *o_reason
    );


/* Shadow Device Create and Shadow Device Destroy API should be called if application
wishes to reset the shadow memory data after it resets the devices in cascade */
extern NlmErrNum_t NlmDevMgr__ShadowDeviceCreate(
    NlmDev *dev_p,
    NlmReasonCode *o_reason
    );

extern NlmErrNum_t NlmDevMgr__ShadowDeviceDestroy(
    NlmDev *dev_p,
    NlmReasonCode *o_reason
    );

/* Generic Register write API (write 80b global[R/W] and LTR) */
extern NlmErrNum_t NlmDevMgr__RegisterWrite(
    void            *dev,
    nlm_u8      portNum,  /* Port number */
    nlm_u32         address,        
    void            *data,
    NlmReasonCode   *o_reason
    );

/* Generic Register read API (read 80b global and LTR) */
extern NlmErrNum_t NlmDevMgr__RegisterRead(
    void            *dev,
    nlm_u8      portNum,  /* Port number */
    nlm_u32         address,        
    void            *o_data,
    NlmReasonCode   *o_reason
    );

/*
    NlmDevMgr__CmdSend is device manager function for handling
    NL12000 special instuctions.
*/
extern NlmErrNum_t NlmDevMgr__CmdSend(
    void                        *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       bank_num,
    nlm_u8                       opcode_high,
    nlm_u8                       opcode_low,
    void                        *data,
    nlm_u8                       data_len,
    NlmReasonCode*              o_reason
    );

/* This API send NOP instruction to device with number of times specified */
extern NlmErrNum_t NlmDevMgr__SendNop(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u32             numTimes,
    NlmReasonCode*      o_reason
    );

/*
    API to write Range registers with 80b data.
*/
extern NlmErrNum_t NlmDevMgr__RangeRegisterWrite(
    NlmDev          *dev,
    nlm_u8          portNum,  /* Port number */
    NlmSMTNum       smtNum,
    nlm_u32         address,
    NlmDevRangeReg  *rangeRegData,
    NlmReasonCode   *o_reason
    );

/*
    API to read 80b data Range register.
*/
extern NlmErrNum_t NlmDevMgr__RangeRegisterRead(
    NlmDev          *dev,
    nlm_u8          portNum,  /* Port number */
    NlmSMTNum       smtNum,
    nlm_u32         address,
    NlmDevRangeReg  *o_rangeRegData,
    NlmReasonCode   *o_reason
    );

/*
    API to Refresh the DBA entry with use of shadow memory.
*/

NlmErrNum_t NlmDevMgr__DBAEntryRefresh(
    NlmDev              *dev,
    nlm_u8            portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the DBA Number */
    nlm_u16 abAddr, /* Specifies the DBA Entry location within the specified abNum  */
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    );

/*
    API to Refresh the LTR register with use of shadow memory.
*/


NlmErrNum_t NlmDevMgr__LogicalTableRegisterRefresh(
    NlmDev              *dev,
    nlm_u8               portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    NlmReasonCode*          o_reason
    );

/* functions exposed by DevMgr Module
 for reason code details, user must not pass NULL. if o_reason is NULL, user
 will not be get reason codes for failures.
*/
/*
    kbp_dm_init creates Device manager using memory allocator which must be passed
    as parameter. Function returns NULL if fails and user should see the reason code in
    case of failure.
*/
NlmDevMgr* kbp_dm_init(
    NlmCmAllocator *alloc_p,
    void*           xpt_p,
    NlmDevType      devType, /* Standard or NetRoute */
    NlmPortMode     portMode,
    NlmSMTMode      smtMode,
    NlmDevOperationMode  operMode,
    NlmReasonCode*  o_reason
    );

/*
    kbp_dm_destroy deletes Device manager and all devices in the device manager list.
*/
extern void kbp_dm_destroy(
    NlmDevMgr *devMgr_p
    );

/*
    kbp_dm_add_device creates an software instance of device and adds to device list of device manager.
    If this function fails it returns NULL . User should see the reason code in case of failure.
*/
extern NlmDev* kbp_dm_add_device(
    NlmDevMgr    *self,
    NlmDevId            *o_devId,       /* device id assigned by the device manager for the device being added*/
    NlmReasonCode*          o_reason
    );

/*
    NlmDevMgr__ResetDevice resets all the devices in cascade to have initial values.
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_reset_devices(
    NlmDevMgr                       *self,
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_global_reg_read reads any global register depending upon the register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_global_reg_read(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    NlmDevGlobalRegType         regType,        /* Global register type - see definitions above */
    void                            *o_data,    /* Global register structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_global_reg_write writes any global register depending upon the register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_global_reg_write(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    NlmDevGlobalRegType         regType,        /* Global register type - see definitions above */
    const void                      *data,      /* Global register structure pointer */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_cb_reg_read reads context buffer as register read.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_cb_reg_read(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                 cbAddr,     /* Only the offset address of the CB needs to be provided
                                                                          i.e 0 - 16383 (NLMDEV_CB_DEPTH - 1)*/
    NlmDevCtxBufferReg          *o_cbRegData,       /* see the structure description above. */
    NlmReasonCode*          o_reason
    );

/*
    kbp_dm_cb_reg_write writes context buffer as register write.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_cb_reg_write(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                 cbAddr,     /* Only the offset address of the CB needs to be provided
                                                                          i.e 0 - 16383 (NLMDEV_CB_DEPTH - 1)*/
    NlmDevCtxBufferReg          *cbRegData,         /* see the structure description above */
    NlmReasonCode*          o_reason
    );

/*
    kbp_dm_ltr_read reads LTR register depending on LTR register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_ltr_read(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                          ltrNum, /* LTR profile set number */
    NlmDevLtrRegType                regType,        /* see the structure description above */
    void                            *o_data,    /* LTR register type structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_ltr_write writes LTR register depending on LTR register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_ltr_write(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                          ltrNum, /* LTR profile set number */
    NlmDevLtrRegType                regType,        /* see the structure description above */
    const void                      *data,      /* LTR register type structure pointer */
    NlmReasonCode*                  o_reason
    );


/*
    kbp_dm_block_reg_read reads block register depending on block register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_block_reg_read(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                     abNum,      /* AB number in which register lies */
    NlmDevBlockRegType          regType,        /* see the structure description above */
    void                            *o_data,    /* Block register structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_block_reg_write writes block register depending on block register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_block_reg_write(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                         abNum,      /* AB number in which register lies */
    NlmDevBlockRegType          regType,        /* see the structure description above */
    const void                      *data,      /* Block register structure pointer */
    NlmReasonCode*                  o_reason
    );


/*
    kbp_dm_dba_write writes single Array Block Database entry
    to the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data can be written in either DM or XY mode
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_dba_write(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the AB Number */
    nlm_u16 abAddr, /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry *abEntry,
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    );

/*
    kbp_dm_dba_read readss single Array Block Database entry
    from the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data read will available as type NlmDevABEntry, always XY mode
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_dba_read(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry   *o_abEntry,
    NlmReasonCode*      o_reason
    );

/*
    kbp_dm_dba_invalidate invalidates specified array block database entry
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_dba_invalidate(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16             abNum,                  /* AB number */
    nlm_u16             abAddr,                 /* entry index within the AB */
    NlmReasonCode*      o_reason
    );


/*
    kbp_dm_uda_write writes single UDA Database (Associated Data) entry of 
    width 32b (only LSB 32b are valid) specified addr. 
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_uda_write(
    void            *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u32         address,        
    void            *data,
    nlm_u8          length,
    NlmReasonCode*  o_reason
    );


/*
    kbp_dm_em_write API can be used to add Exact Match entry at the specified
    address within a DBA block. Write mode (whether WRA or WRB) is specified by
    the input parameter 'writeMode'.
*/
NlmErrNum_t kbp_dm_em_write(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,      /* Specifies the DBA Block Number */
    nlm_u16         abAddr,     /* Specifies the entry location within the specified abNum  */
    NlmDevEMEntry   *emEntry,
    NlmEMWriteMode  writeMode,
    NlmReasonCode*  o_reason
    );

    
/*  kbp_dm_em_read API can be used to read Exact Match entry at the specified
    address within a DBA block. Read mode (whether RDA or RDB) is specified by
    the input parameter 'readMode'.  Output is available in the output parameter
    'o_emEntry'
*/
NlmErrNum_t kbp_dm_em_read(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,    /* Specifies the DBA Block Number */
    nlm_u16         abAddr,  /* Specifies the entry location within the specified abNum  */
    NlmEMReadMode   readMode,  /* Read-A or Read-B */
    NlmDevEMEntry   *o_emEntry,
    NlmReasonCode*  o_reason
    );
        

/*
    kbp_dm_uda_read reads single UDA Database (Associated Data) entry of 
    width 32b (only LSB 32b are valid) from specified addr. 
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_uda_read(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,        
    void                *o_data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    );


/*
    kbp_dm_cbwcmp1 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, 4 Keys and 4 Result ports available
        Single Bank mode:
            0-127 for BANK-0, 4 Keys and 4 result ports

        Dual Bank:
            0-63   for BANK-0, 2 Keys (KEY 0/1) and 2 result ports (0/1)
            64-127 for BANK-1, 2 Keys (KEY 2/3) and 2 result ports (2/3)
    )
    and gives hit/miss and address as an output.
    Compare1 instruction can be used for searches with individual search key lengths of upto 320b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t kbp_dm_cbwcmp1(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );

/*
    kbp_dm_cbwcmp2 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, 4 Keys and 4 Result ports available
        Single Bank mode:
            0-127 for BANK-0, 4 Keys and 4 result ports

        Dual Bank:
            0-63   for BANK-0, 2 Keys (KEY 0/1) and 2 result ports (0/1)
            64-127 for BANK-1, 2 Keys (KEY 2/3) and 2 result ports (2/3)
    )
    and gives hit/miss and address as an output.
    Compare2 instruction can be used for searches with individual search key lengths of upto 640b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t kbp_dm_cbwcmp2(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );

/*
    kbp_dm_cbwlpm is a special search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, always uses Key- 0 and Result port-0
        Single Bank mode:
            0-127 for BANK-0, Key-0 and result port-0

        Dual Bank:
            0-63   for BANK-0, KEY-0 and result ports-0
    )
    and gives hit/miss and address as an output.
    CompareLPM instruction can be used for searches with individual search key lengths of upto 160b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t kbp_dm_cbwlpm(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );


NlmErrNum_t kbp_dm_cbwcmp3(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR  number to be used */
    NlmDevCtxBufferInfo *cbInfo,   /* Search key details */
    NlmDevCmpResult     *o_search_results, /* Result details */
    NlmReasonCode*      o_reason
    );

/*
    kbp_dm_multi_compare function is a special API provided to perform compares on
    both SMTs. There is no corresponding instruction available in the device. This API
    can be used to perform searches on both SMTs simultaneously and get results
    simultaneously. Other APIs (__Compare1/__Compare2/__CompareLPM) should
    be used if there only one search on either SMT. 

    This API should be used in dual bank (2-SMT) mode only. Up to 4 search results are 
    returned in the 'o_search_results' output parameter. Input parameter 'cbInfo'
    should be filled with appropriate input search data.
*/
NlmErrNum_t kbp_dm_multi_compare(
    NlmDevMgr               *self,
    nlm_u8                  portNum,            /* Port number */
    NlmDevCBWriteCmpParam   *cbInfo,                /* Search data for compares  */
    NlmDevCmpResult         *o_search_results,  /* Search results */
    NlmReasonCode*           o_reason
    );
    

/*
    kbp_dm_cb_write writes to context buffer memory
    Upto 640 bytes of data can be written to CB memory;
    see desription of NlmDevCtxBufferInfo for more details
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t kbp_dm_cb_write(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    NlmDevCtxBufferInfo     *cbInfo,                /* see the structure description above */
    NlmReasonCode*              o_reason
    );


/*
    kbp_dm_lock_config locks the device manager configuration.
    Once the configuration is locked no more devices can be added.
*/
extern NlmErrNum_t kbp_dm_lock_config(
    NlmDevMgr *self,
    NlmReasonCode *o_reason
    );


/* Shadow Device Create and Shadow Device Destroy API should be called if application
wishes to reset the shadow memory data after it resets the devices in cascade */
extern NlmErrNum_t kbp_dm_shadow_create(
    NlmDev *dev_p,
    NlmReasonCode *o_reason
    );

extern NlmErrNum_t kbp_dm_shadow_destroy(
    NlmDev *dev_p,
    NlmReasonCode *o_reason
    );

/* Generic Register write API (write 80b global[R/W] and LTR) */
extern NlmErrNum_t kbp_dm_generic_reg_write(
    void            *dev,
    nlm_u8      portNum,  /* Port number */
    nlm_u32         address,        
    void            *data,
    NlmReasonCode   *o_reason
    );

/* Generic Register read API (read 80b global and LTR) */
extern NlmErrNum_t kbp_dm_generic_reg_read(
    void            *dev,
    nlm_u8      portNum,  /* Port number */
    nlm_u32         address,        
    void            *o_data,
    NlmReasonCode   *o_reason
    );

/*
    kbp_dm_cmd_send is device manager function for handling
    NL12000 special instuctions.
*/
extern NlmErrNum_t kbp_dm_cmd_send(
    void                        *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       bank_num,
    nlm_u8                       opcode_high,
    nlm_u8                       opcode_low,
    void                        *data,
    nlm_u8                       data_len,
    NlmReasonCode*              o_reason
    );

/* This API send NOP instruction to device with number of times specified */
extern NlmErrNum_t kbp_dm_send_nop(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u32             numTimes,
    NlmReasonCode*      o_reason
    );

/*
    API to write Range registers with 80b data.
*/
extern NlmErrNum_t kbp_dm_range_reg_write(
    NlmDev          *dev,
    nlm_u8          portNum,  /* Port number */
    NlmSMTNum       smtNum,
    nlm_u32         address,
    NlmDevRangeReg  *rangeRegData,
    NlmReasonCode   *o_reason
    );

/*
    API to read 80b data Range register.
*/
extern NlmErrNum_t kbp_dm_range_reg_read(
    NlmDev          *dev,
    nlm_u8          portNum,  /* Port number */
    NlmSMTNum       smtNum,
    nlm_u32         address,
    NlmDevRangeReg  *o_rangeRegData,
    NlmReasonCode   *o_reason
    );

/*
    API to Refresh the DBA entry with use of shadow memory.
*/

NlmErrNum_t kbp_dm_dba_refresh(
    NlmDev              *dev,
    nlm_u8            portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the DBA Number */
    nlm_u16 abAddr, /* Specifies the DBA Entry location within the specified abNum  */
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    );

/*
    API to Refresh the LTR register with use of shadow memory.
*/


NlmErrNum_t kbp_dm_ltr_refresh(
    NlmDev              *dev,
    nlm_u8               portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    NlmReasonCode*          o_reason
    );

NlmErrNum_t kbp_dm_uda_config(
    NlmDev              *dev_p,
    nlm_u8              portNum,
    nlm_u16             blk_num,
    nlm_u16             blk_width,
    nlm_u32             uda_addr,
    nlm_u16             uda_width,
    NlmReasonCode*          o_reason
    );

NlmErrNum_t kbp_dm_block_operation(
    NlmDev                  *dev,           /*Device pointer */ 
    nlm_u8                  portNum,        /* Port number */
    NlmDevBlockOperParam    blkOperParam,   /* block instructions */
    NlmReasonCode*          o_reason
    );


extern NlmErrNum_t kbp_dm_dump(
    NlmDev                  *dev_p,
    NlmDevLogType           logType,
    NlmDevSbRange           supBlkRange
    );

extern NlmErrNum_t 
kbp_dm_compare_dev_sm_memory(
    NlmDev                  *dev_p
    );


#ifndef NLMPLATFORM_BCM
NlmDevMgr* bcm_kbp_dm_init(
    NlmCmAllocator *alloc_p,
    void*           xpt_p,
    NlmDevType      devType, /* Standard or NetRoute */
    NlmPortMode     portMode,
    NlmSMTMode      smtMode,
    NlmDevOperationMode  operMode,
    NlmReasonCode*  o_reason
    );

/*
    kbp_dm_destroy deletes Device manager and all devices in the device manager list.
*/
extern void bcm_kbp_dm_destroy(
    NlmDevMgr *devMgr_p
    );

/*
    kbp_dm_add_device creates an software instance of device and adds to device list of device manager.
    If this function fails it returns NULL . User should see the reason code in case of failure.
*/
extern NlmDev* bcm_kbp_dm_add_device(
    NlmDevMgr    *self,
    NlmDevId            *o_devId,       /* device id assigned by the device manager for the device being added*/
    NlmReasonCode*          o_reason
    );

/*
    NlmDevMgr__ResetDevice resets all the devices in cascade to have initial values.
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_reset_devices(
    NlmDevMgr                       *self,
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_global_reg_read reads any global register depending upon the register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_global_reg_read(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    NlmDevGlobalRegType         regType,        /* Global register type - see definitions above */
    void                            *o_data,    /* Global register structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_global_reg_write writes any global register depending upon the register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_global_reg_write(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    NlmDevGlobalRegType         regType,        /* Global register type - see definitions above */
    const void                      *data,      /* Global register structure pointer */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_cb_reg_read reads context buffer as register read.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_cb_reg_read(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                 cbAddr,     /* Only the offset address of the CB needs to be provided
                                                                          i.e 0 - 16383 (NLMDEV_CB_DEPTH - 1)*/
    NlmDevCtxBufferReg          *o_cbRegData,       /* see the structure description above. */
    NlmReasonCode*          o_reason
    );

/*
    kbp_dm_cb_reg_write writes context buffer as register write.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_cb_reg_write(
    NlmDev              *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                 cbAddr,     /* Only the offset address of the CB needs to be provided
                                                                          i.e 0 - 16383 (NLMDEV_CB_DEPTH - 1)*/
    NlmDevCtxBufferReg          *cbRegData,         /* see the structure description above */
    NlmReasonCode*          o_reason
    );

/*
    kbp_dm_ltr_read reads LTR register depending on LTR register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_ltr_read(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                          ltrNum, /* LTR profile set number */
    NlmDevLtrRegType                regType,        /* see the structure description above */
    void                            *o_data,    /* LTR register type structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_ltr_write writes LTR register depending on LTR register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_ltr_write(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                          ltrNum, /* LTR profile set number */
    NlmDevLtrRegType                regType,        /* see the structure description above */
    const void                      *data,      /* LTR register type structure pointer */
    NlmReasonCode*                  o_reason
    );


/*
    kbp_dm_block_reg_read reads block register depending on block register type.
    Read data is stored in *o_data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_block_reg_read(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                     abNum,      /* AB number in which register lies */
    NlmDevBlockRegType          regType,        /* see the structure description above */
    void                            *o_data,    /* Block register structure pointer as output */
    NlmReasonCode*                  o_reason
    );

/*
    kbp_dm_block_reg_write writes block register depending on block register type.
    Data to be written should be stored in *data. User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_block_reg_write(
    NlmDev                      *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u16                         abNum,      /* AB number in which register lies */
    NlmDevBlockRegType          regType,        /* see the structure description above */
    const void                      *data,      /* Block register structure pointer */
    NlmReasonCode*                  o_reason
    );


/*
    kbp_dm_dba_write writes single Array Block Database entry
    to the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data can be written in either DM or XY mode
    User should see the reason code in case of failure.
*/
NlmErrNum_t bcm_kbp_dm_dba_write(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the AB Number */
    nlm_u16 abAddr, /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry *abEntry,
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    );

/*
    kbp_dm_dba_read readss single Array Block Database entry
    from the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data read will available as type NlmDevABEntry, always XY mode
    User should see the reason code in case of failure.
*/
NlmErrNum_t bcm_kbp_dm_dba_read(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry   *o_abEntry,
    NlmReasonCode*      o_reason
    );

/*
    kbp_dm_dba_invalidate invalidates specified array block database entry
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_dba_invalidate(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16             abNum,                  /* AB number */
    nlm_u16             abAddr,                 /* entry index within the AB */
    NlmReasonCode*      o_reason
    );


/*
    kbp_dm_uda_write writes single UDA Database (Associated Data) entry of 
    width 32b (only LSB 32b are valid) specified addr. 
    User should see the reason code in case of failure.
*/
NlmErrNum_t bcm_kbp_dm_uda_write(
    void            *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u32         address,        
    void            *data,
    nlm_u8          length,
    NlmReasonCode*  o_reason
    );


/*
    kbp_dm_em_write API can be used to add Exact Match entry at the specified
    address within a DBA block. Write mode (whether WRA or WRB) is specified by
    the input parameter 'writeMode'.
*/
NlmErrNum_t bcm_kbp_dm_em_write(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,      /* Specifies the DBA Block Number */
    nlm_u16         abAddr,     /* Specifies the entry location within the specified abNum  */
    NlmDevEMEntry   *emEntry,
    NlmEMWriteMode  writeMode,
    NlmReasonCode*  o_reason
    );

    
/*  kbp_dm_em_read API can be used to read Exact Match entry at the specified
    address within a DBA block. Read mode (whether RDA or RDB) is specified by
    the input parameter 'readMode'.  Output is available in the output parameter
    'o_emEntry'
*/
NlmErrNum_t bcm_kbp_dm_em_read(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,    /* Specifies the DBA Block Number */
    nlm_u16         abAddr,  /* Specifies the entry location within the specified abNum  */
    NlmEMReadMode   readMode,  /* Read-A or Read-B */
    NlmDevEMEntry   *o_emEntry,
    NlmReasonCode*  o_reason
    );
        

/*
    kbp_dm_uda_read reads single UDA Database (Associated Data) entry of 
    width 32b (only LSB 32b are valid) from specified addr. 
    User should see the reason code in case of failure.
*/
NlmErrNum_t bcm_kbp_dm_uda_read(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,        
    void                *o_data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    );


/*
    kbp_dm_cbwcmp1 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, 4 Keys and 4 Result ports available
        Single Bank mode:
            0-127 for BANK-0, 4 Keys and 4 result ports

        Dual Bank:
            0-63   for BANK-0, 2 Keys (KEY 0/1) and 2 result ports (0/1)
            64-127 for BANK-1, 2 Keys (KEY 2/3) and 2 result ports (2/3)
    )
    and gives hit/miss and address as an output.
    Compare1 instruction can be used for searches with individual search key lengths of upto 320b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t bcm_kbp_dm_cbwcmp1(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );

/*
    kbp_dm_cbwcmp2 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, 4 Keys and 4 Result ports available
        Single Bank mode:
            0-127 for BANK-0, 4 Keys and 4 result ports

        Dual Bank:
            0-63   for BANK-0, 2 Keys (KEY 0/1) and 2 result ports (0/1)
            64-127 for BANK-1, 2 Keys (KEY 2/3) and 2 result ports (2/3)
    )
    and gives hit/miss and address as an output.
    Compare2 instruction can be used for searches with individual search key lengths of upto 640b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t bcm_kbp_dm_cbwcmp2(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );

/*
    kbp_dm_cbwlpm is a special search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, always uses Key- 0 and Result port-0
        Single Bank mode:
            0-127 for BANK-0, Key-0 and result port-0

        Dual Bank:
            0-63   for BANK-0, KEY-0 and result ports-0
    )
    and gives hit/miss and address as an output.
    CompareLPM instruction can be used for searches with individual search key lengths of upto 160b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t bcm_kbp_dm_cbwlpm(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       ltrNum,
    NlmDevCtxBufferInfo          *cbInfo,
    NlmDevCmpResult              *o_search_results,
    NlmReasonCode*              o_reason
    );


NlmErrNum_t bcm_kbp_dm_cbwcmp3(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR  number to be used */
    NlmDevCtxBufferInfo *cbInfo,   /* Search key details */
    NlmDevCmpResult     *o_search_results, /* Result details */
    NlmReasonCode*      o_reason
    );

/*
    kbp_dm_multi_compare function is a special API provided to perform compares on
    both SMTs. There is no corresponding instruction available in the device. This API
    can be used to perform searches on both SMTs simultaneously and get results
    simultaneously. Other APIs (__Compare1/__Compare2/__CompareLPM) should
    be used if there only one search on either SMT. 

    This API should be used in dual bank (2-SMT) mode only. Up to 4 search results are 
    returned in the 'o_search_results' output parameter. Input parameter 'cbInfo'
    should be filled with appropriate input search data.
*/
NlmErrNum_t bcm_kbp_dm_multi_compare(
    NlmDevMgr               *self,
    nlm_u8                  portNum,            /* Port number */
    NlmDevCBWriteCmpParam   *cbInfo,                /* Search data for compares  */
    NlmDevCmpResult         *o_search_results,  /* Search results */
    NlmReasonCode*           o_reason
    );
    

/*
    kbp_dm_cb_write writes to context buffer memory
    Upto 640 bytes of data can be written to CB memory;
    see desription of NlmDevCtxBufferInfo for more details
    User should see the reason code in case of failure.
*/
extern NlmErrNum_t bcm_kbp_dm_cb_write(
    NlmDevMgr                   *self,
    nlm_u8                      portNum,  /* Port number */
    NlmDevCtxBufferInfo     *cbInfo,                /* see the structure description above */
    NlmReasonCode*              o_reason
    );


/*
    kbp_dm_lock_config locks the device manager configuration.
    Once the configuration is locked no more devices can be added.
*/
extern NlmErrNum_t bcm_kbp_dm_lock_config(
    NlmDevMgr *self,
    NlmReasonCode *o_reason
    );


/* Shadow Device Create and Shadow Device Destroy API should be called if application
wishes to reset the shadow memory data after it resets the devices in cascade */
extern NlmErrNum_t bcm_kbp_dm_shadow_create(
    NlmDev *dev_p,
    NlmReasonCode *o_reason
    );

extern NlmErrNum_t bcm_kbp_dm_shadow_destroy(
    NlmDev *dev_p,
    NlmReasonCode *o_reason
    );

/* Generic Register write API (write 80b global[R/W] and LTR) */
extern NlmErrNum_t bcm_kbp_dm_generic_reg_write(
    void            *dev,
    nlm_u8      portNum,  /* Port number */
    nlm_u32         address,        
    void            *data,
    NlmReasonCode   *o_reason
    );

/* Generic Register read API (read 80b global and LTR) */
extern NlmErrNum_t bcm_kbp_dm_generic_reg_read(
    void            *dev,
    nlm_u8      portNum,  /* Port number */
    nlm_u32         address,        
    void            *o_data,
    NlmReasonCode   *o_reason
    );

/*
    kbp_dm_cmd_send is device manager function for handling
    NL12000 special instuctions.
*/
extern NlmErrNum_t bcm_kbp_dm_cmd_send(
    void                        *dev,
    nlm_u8                      portNum,  /* Port number */
    nlm_u8                       bank_num,
    nlm_u8                       opcode_high,
    nlm_u8                       opcode_low,
    void                        *data,
    nlm_u8                       data_len,
    NlmReasonCode*              o_reason
    );

/* This API send NOP instruction to device with number of times specified */
extern NlmErrNum_t bcm_kbp_dm_send_nop(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u32             numTimes,
    NlmReasonCode*      o_reason
    );

/*
    API to write Range registers with 80b data.
*/
extern NlmErrNum_t bcm_kbp_dm_range_reg_write(
    NlmDev          *dev,
    nlm_u8          portNum,  /* Port number */
    NlmSMTNum       smtNum,
    nlm_u32         address,
    NlmDevRangeReg  *rangeRegData,
    NlmReasonCode   *o_reason
    );

/*
    API to read 80b data Range register.
*/
extern NlmErrNum_t bcm_kbp_dm_range_reg_read(
    NlmDev          *dev,
    nlm_u8          portNum,  /* Port number */
    NlmSMTNum       smtNum,
    nlm_u32         address,
    NlmDevRangeReg  *o_rangeRegData,
    NlmReasonCode   *o_reason
    );

/*
    API to Refresh the DBA entry with use of shadow memory.
*/

NlmErrNum_t bcm_kbp_dm_dba_refresh(
    NlmDev              *dev,
    nlm_u8            portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the DBA Number */
    nlm_u16 abAddr, /* Specifies the DBA Entry location within the specified abNum  */
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    );

/*
    API to Refresh the LTR register with use of shadow memory.
*/


NlmErrNum_t bcm_kbp_dm_ltr_refresh(
    NlmDev              *dev,
    nlm_u8               portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    NlmReasonCode*          o_reason
    );

NlmErrNum_t bcm_kbp_dm_uda_config(
    NlmDev              *dev_p,
    nlm_u8              portNum,
    nlm_u16             blk_num,
    nlm_u16             blk_width,
    nlm_u32             uda_addr,
    nlm_u16             uda_width,
    NlmReasonCode*          o_reason
    );

NlmErrNum_t bcm_kbp_dm_block_operation(
    NlmDev                  *dev,           /*Device pointer */ 
    nlm_u8                  portNum,        /* Port number */
    NlmDevBlockOperParam    blkOperParam,   /* block instructions */
    NlmReasonCode*          o_reason
    );


extern NlmErrNum_t bcm_kbp_dm_dump(
    NlmDev                  *dev_p,
    NlmDevLogType           logType,
    NlmDevSbRange           supBlkRange
    );

extern NlmErrNum_t 
bcm_kbp_dm_compare_dev_sm_memory(
    NlmDev                  *dev_p
    );

#endif

#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif
#endif /* INCLUDED_NLMDEVMGR_H */

