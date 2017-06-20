/*
 * $Id: FileTransCtc.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     FileTransCtc.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_FileTransCtc_H
#define _SOC_EA_FileTransCtc_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>


/* the Maximum bytes in the transmission package*/
#define CtcFileMaxPacketSize            1400
/* the flag of activating and commiting mesage */
#define ActComFlags                     0x00
#define FwUpgradeOamCreditsRate         10

/*
 * CTC 2.1 firmware upgrade operation code
 */
typedef enum {
    CtcOpFileWriteReq   = 2,
    CtcOpFileSendData   = 3,
    CtcOpFileSendAck    = 4,
    CtcOpFileError      = 5,
    CtcOpEndDnldReq     = 6,
    CtcOpEndDnldRes     = 7,
    CtcOpActImgReq      = 8,
    CtcOpActImgRes      = 9,
    CtcOpCmtImgReq      = 10,
    CtcOpCmtImgRes      = 11,
    CtcOpForceToU16     = 0x7fff
} PACK CtcFwUpgradeOpcode;

/*
 * CTC 2.1 firmware upgrade error code
 */
typedef enum {
    CtcErrUndefined,
    CtcErrFileNotFound,
    CtcErrAccessViolation,
    CtcErrDiskFull,
    CtcErrIllegalOp,
    CtcErrUnknownTid,
    CtcErrFileExists,
    CtcErrNoSuchUser,
    ctcErrCodeNums,
    CtcErrNoError,              /* non-standard; internal use only*/
    CtcErrForceToU16    = 0x7fff
} PACK CtcFwUpgradeErr;


/*
 * Response code for End Download request
 */
typedef enum {
    CtcResEndDnldNoErr,
    CtcResEndDnldBusy,
    CtcResEndDnldCrcError,
    CtcResEndDnldParaError,
    CtcResEndDnldUnsupport,
    CtcResEndDnldNums
} PACK CtcEndDnldRes;

/*
 * Response code for activating and comiting message
 */
typedef enum {
    CtcAckActComOk,
    CtcAckActComParaError,
    CtcAckActComUnsupport,
    CtcAckActComLoadError,
    CtcAckActComNums
} PACK CtcActComAck;

/*
 * Data type of the upgrade package
 */
typedef enum {
    CtcTftpProData      = 1,
    CtcCheckFileData,
    CtcLoadRunImageData,
    CtcCommitImageData,
    CtcNumDataTypes
} PACK CtcPayloadType;

/*
 * Payload structure
 */
typedef struct {
    CtcPayloadType      dataType;
    uint16              length;
    uint16              tid;
} PACK OamCtcPayloadHead;

/*
 * File Write Request structure
 */
typedef struct {
    CtcFwUpgradeOpcode  opcode;
    char                str[1];
} PACK CtcFwUpgradeFileRequest;

/*
 * File Transfer Data Header structure
 */
typedef struct {
    CtcFwUpgradeOpcode  opcode;
    uint16              num;
} PACK CtcFwUpgradeFileData;

/*
 * File Transfer ACK structure
 */
typedef struct {
    CtcFwUpgradeOpcode  opcode;
    uint16              num;
} PACK CtcFwUpgradeFileAck;

/*
 * File Transfer Error structure
 */
typedef struct {
    CtcFwUpgradeOpcode  opcode;
    CtcFwUpgradeErr     errcode;
    char                errMsg[1];
} PACK CtcFwUpgradeFileError;

/*
 * End Download Request structure
 */
typedef struct {
    CtcFwUpgradeOpcode  opcode;
    uint32              fileSize;
} PACK CtcFwUpgradeEndDnld;

/*
 * Response package structure for End Download Request, Activating, and 
 * Commiting message
 */
typedef struct {
    CtcFwUpgradeOpcode  opcode;
    uint8               para;
} PACK CtcFwUpgradeComm;

/*
 * The state machine of CTC2.1 firmware upgrade
 */
typedef enum {
    CtcFwUpgradeStateInit   = 0xff,
    CtcFwUpgradeStateDnld   = 0xfc,
    CtcFwUpgradeStateAct    = 0xf0,
    CtcFwUpgradeStateCmt    = 0xc0,
    CtcFwUpgradeNumStates   = 4
} CtcFwUpgradeState;

/*
 * File Download State
 */
typedef struct {
    uint16          CtcTid;
    uint16          blockNum;
    CtcFwUpgradeErr lastErr;
    uint8         * buffer;
    uint32          size;
    uint32          maxSize;
    uint32          endDnLdStats;
} CtcClientSessionInfo;

typedef struct {
    uint16          CtcTid;
    uint8           state;          /* reading from or writing to ONU */
    uint16          nextBlkNum;
    uint32          left;
    uint8         * currFilePos;    /* Pointer to current position in load */
    uint32          lastSend;
} CtcOamFileSession;

/*    
 * CtcSExtOamFileTranReq:  send a file transfer request to PON chipset as a 
 * server.
 *
 * Parameters:
 * \param pathId which pon chipset you want to operate
 * \param fileName the file defined in CTC spec
 * \param mode the mode string defined in CTC spec
 * \param reqResp where the the response message will be putted in
 * 
 * \return 
 * OK or ERROR
 */ 
int32   CtcSExtOamFileTranReq (uint8 pathId, char *fileName, char *mode,
                uint8 * reqResp);

/*    
 * CtcSExtOamFileSendData:  send file content to PON chipset as a server
 *
 * Parameters:
 * \param pathId which pon chipset you want to operate
 * \param the block num of the data which will be transfered (start from 1)
 * \param dataLen the length of the data(1 to 1400 defined in CTC spec)
 * \param data the data
 * \param reqResp the response message will putted in 
 * 
 * \return 
 * OK or ERROR
 */ 
int32   CtcSExtOamFileSendData (uint8 pathId, uint16 blockNum, uint16 dataLen,
                uint8 * data, uint8 * reqResp);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_FileTransCtc_H */
