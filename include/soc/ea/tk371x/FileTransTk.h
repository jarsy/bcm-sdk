/*
 * $Id: FileTransTk.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     FileTransTk.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_FileTransTk_H
#define _SOC_EA_FileTransTk_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>


#define FILE_READ_REQ       0
#define FILE_WRITE_REQ      1

/* The maximum data we can transmit in an OAM frame (assuming max OAM frame
 * size of 1500). 
 * The overhead is: Oam header + 2 bytes of block number + 2 bytes of length
 */
#define MaxOftDataSize      (1470)

typedef enum {
    OamFileIdle,
    OamFileReading,
    OamFileWriting,
    OamFileNumStates
} OamFileState;

typedef struct {
    OamFileState    state;          /* reading from or writing to ONU */
    uint16          nextBlkNum;
    uint32          left;
    uint8         * currFilePos;    /* Pointer to current position in load */
    uint32          lastSend;
} OamFileSession;

typedef struct {
    OamFileState    state;
    uint16          blockNum;
    OamTkFileErr    lastErr;
    uint8         * buffer;
    uint32          size;
    uint32          maxSize;
    OamTkFileType   loadType;
} OamFileSessionInfo;

typedef enum TkFwUpgradMode {
    ExtSdkCtlMode   = 0,
    ExtAppCtlMode   = 1,
    UndefMode       = 0xff
} TkFwUpgradMode;

typedef struct TkOamFwUpgradeInstance_s {
    TkFwUpgradMode  fwUpGradeCtlMode;
    int             (*startFun) (uint8);
    int             (*finishedFun) (uint8, uint32, uint8);
    int             (*processFun) (uint8, uint16, uint8 *, uint16);
} TkOamFwUpgradeInstance_t;


/* send TK extension OAM file read or write request to the ONU and waiting for 
 * response.
 * ReqType : 0 - read, 1 - write
 */
int     TkExtOamFileTranReq (uint8 pathId, uint8 ReqType, uint8 loadType,
                OamTkFileAck * pTkFileAck);

int     TkExtOamFileSendData (uint8 pathId, uint16 blockNum, uint16 dataLen,
                uint8 * data, OamTkFileAck * pTkFileAck);

int     TkExtOamFileSendAck (uint8 pathId, uint8 err);

void    OamFileDone (uint8 pathId);

void    TkExtOamAppCtlModeFileSendAck (uint8 pathId, uint8 link, 
                uint16 blockNum, OamTkFileErr err);

void    TkExtOamAppCtlModeFileRdReq (uint8 pathId, uint8 link, uint8 * pData,
                uint16 len);

void    TkExtOamAppCtlModeFileWrReq (uint8 pathId, uint8 link, uint8 * pData,
                uint16 len);

void    TkExtOamAppCtlModeFileData (uint8 pathId, uint8 link, uint8 * pData,
                uint16 len);

void    TkExtOamAppCtlModeFileAck (uint8 pathId, uint8 link, uint8 * pData,
                uint16 len);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_FileTransTk_H */
