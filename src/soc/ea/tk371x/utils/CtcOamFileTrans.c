/*
 * $Id: CtcOamFileTrans.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcOamFileTrans.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Ethernet.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/FileTransCtc.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkDebug.h>

CtcOamFileSession ctcServerSession;

char *errMessage[ctcErrCodeNums] = {
    "Not defined, see error message (if any)",
    "File not found",
    "Access violation",
    "Disk full or allocation exceeded",
    "Illegal TFTP operation",
    "Unknown transfer ID",
    "File already exists",
    "No such user",
};

/*
 * 
 * CtcSExtOamFileTranReq: send a file transfer request to PON chipset as a server
 *
 * Parameters:
 * / \param pathId which pon chipset you want to operate
 * / \param fileName the file defined in CTC spec
 * / \param mode the mode string defined in CTC spec
 * / \param reqResp where the the response message will be putted in
 * / 
 * / \return 
 * / OK or ERROR
 */
int32
CtcSExtOamFileTranReq(uint8 pathId,
                      char *fileName, char *mode, uint8 * reqResp)
{
    uint32          msgLen = 0;
    uint16          flags = 0x0050;
    uint8          *pMsgBuf = NULL;
    OamTkExt       *ctcExt;
    OamCtcPayloadHead *pOamCtcPayloadHead;
    CtcFwUpgradeFileRequest *reqMsg;
    CtcFwUpgradeFileAck *resAck;
    CtcFwUpgradeFileError *errMsg;
    int32           fileNameLen = 0;
    int32           modeStrLen = 0;
    int32           retVal = 0;

    if (NULL == fileName || NULL == mode || NULL == reqResp) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ctcExt = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);
    ctcExt->opcode = OamCtcFileUpgrade;
    pOamCtcPayloadHead = (OamCtcPayloadHead *) (ctcExt + 1);


    pOamCtcPayloadHead->dataType = CtcTftpProData;
    pOamCtcPayloadHead->tid = ctcServerSession.CtcTid;
    pOamCtcPayloadHead->length = sizeof(OamCtcPayloadHead);


    reqMsg = (CtcFwUpgradeFileRequest *) (pOamCtcPayloadHead + 1);
    reqMsg->opcode = CtcOpFileWriteReq;
    fileNameLen = sal_strlen(fileName);
    modeStrLen = sal_strlen(mode);
    /*
     * TkDbgPrintf(("fileNameLen = %d,modeStrLen =
     * %d\n",fileNameLen,modeStrLen));
     * clear the buffer for fill file name and mode string
     * 
     */
    sal_memcpy(reqMsg->str, fileName, fileNameLen);
    sal_memcpy((reqMsg->str + fileNameLen + 1), mode, modeStrLen);
    pOamCtcPayloadHead->length +=
        sizeof(CtcFwUpgradeFileRequest) + fileNameLen + modeStrLen + 1;
    /*
     * calculate the message length
     */ 
    msgLen = sizeof(OamMsg) + sizeof(IeeeOui) + sizeof(OamTkExt)
        + pOamCtcPayloadHead->length;

    /*
     * file write request format:
     * -----------------------------------------------------------------
     * |2 byte opcode|FileName string| 0 |mode string| 0 |
     * -----------------------------------------------------------------
     */
    pOamCtcPayloadHead->length +=
        sizeof(CtcFwUpgradeFileRequest) + fileNameLen + modeStrLen + 1;

    /*
     * transfer to network byte order
     */
    pOamCtcPayloadHead->tid = soc_htons(pOamCtcPayloadHead->tid);
    pOamCtcPayloadHead->length = soc_htons(pOamCtcPayloadHead->length);
    reqMsg->opcode = soc_htons(reqMsg->opcode);
    /*
     * The first reply need about > 3s. The last reply need about > 10s
     * for App. 
     * The middle reply need about > 0.02s.
     */
    if (OK ==
        TkOamRequest(pathId, 0,
                     (OamFrame *) pMsgBuf,
                     msgLen, (OamPdu *) pMsgBuf, &msgLen)) {
        pOamCtcPayloadHead = (OamCtcPayloadHead *)INT_TO_PTR(
                                                    PTR_TO_INT(pMsgBuf) +
                                                    sizeof(OamOuiVendorExt)
                                                    + sizeof(OamTkExt));
        resAck = (CtcFwUpgradeFileAck *) (pOamCtcPayloadHead + 1);
        errMsg = (CtcFwUpgradeFileError *) (pOamCtcPayloadHead + 1);
        /*
         * update TID and check the resonse if right or not
         */
        ctcServerSession.CtcTid = soc_ntohs(pOamCtcPayloadHead->tid);

        if (pOamCtcPayloadHead->dataType == CtcTftpProData) {
            if ((soc_ntohs(resAck->opcode) == CtcOpFileSendAck)
                && (soc_ntohs(resAck->num) == 0)) {
                resAck->opcode = soc_ntohs(resAck->opcode);
                resAck->num = soc_ntohs(resAck->num);
                sal_memcpy(reqResp, resAck, sizeof(CtcFwUpgradeFileAck));
            } else if (soc_ntohs(resAck->opcode)
                       == CtcOpFileError) {
                errMsg->opcode = soc_ntohs(errMsg->opcode);
                errMsg->errcode = soc_ntohs(errMsg->errcode);
                msgLen = soc_ntohs(pOamCtcPayloadHead->length);
                sal_memcpy(reqResp, errMsg,
                           msgLen - sizeof(OamCtcPayloadHead));
            } else {
                TkDbgTrace(TkDbgErrorEnable);
                retVal = ERROR;
            }
        } else {
            TkDbgTrace(TkDbgErrorEnable);
            retVal = ERROR;
        }

        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return retVal;
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
}


/*
 * CtcSExtOamFileSendData: send file content to PON chipset as a server
 *
 * Parameters:
 *  \param pathId which pon chipset you want to operate
 *  \param the block num of the data which will be transfered (start from 
 *  1)
 *  \param dataLen the length of the data(1 to 1400 defined in CTC spec)
 *  \param data the data
 *  \param reqResp the response message will putted in 
 *  
 *  \return 
 *  OK or ERROR
 */ 
int32
CtcSExtOamFileSendData(uint8 pathId,
                       uint16 blockNum,
                       uint16 dataLen, uint8 * data, uint8 * reqResp)
{
    uint32          msgLen = 0;
    uint16          flags = 0x0050;
    uint8          *pMsgBuf = NULL;
    OamTkExt       *ctcExt;
    OamCtcPayloadHead *pOamCtcPayloadHead;
    CtcFwUpgradeFileData *reqMsg;
    CtcFwUpgradeFileAck *resAck;
    CtcFwUpgradeFileError *errMsg;
    int32           retVal = 0;

    if (NULL == data || NULL == reqResp) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ctcExt = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);
    ctcExt->opcode = OamCtcFileUpgrade;
    pOamCtcPayloadHead = (OamCtcPayloadHead *) (ctcExt + 1);


    pOamCtcPayloadHead->dataType = CtcTftpProData;
    pOamCtcPayloadHead->tid = ctcServerSession.CtcTid;
    pOamCtcPayloadHead->length = sizeof(OamCtcPayloadHead);

    reqMsg = (CtcFwUpgradeFileData *) (pOamCtcPayloadHead + 1);
    reqMsg->opcode = CtcOpFileSendData;
    reqMsg->num = blockNum;
    sal_memcpy((void *) (reqMsg + 1), (void *) data, dataLen);
    pOamCtcPayloadHead->length += sizeof(CtcFwUpgradeFileData) + dataLen;

    /*
     * calculate the message length
     */ 
    msgLen = sizeof(OamMsg) + sizeof(IeeeOui) + sizeof(OamTkExt)
        + pOamCtcPayloadHead->length;

    /*
     * transfer to network byte order
     */
    pOamCtcPayloadHead->tid = soc_htons(pOamCtcPayloadHead->tid);
    pOamCtcPayloadHead->length = soc_htons(pOamCtcPayloadHead->length);
    reqMsg->opcode = soc_htons(reqMsg->opcode);
    reqMsg->num = soc_htons(reqMsg->num);

    /*
     * The first reply need about > 3s. The last reply need about > 10s
     * for App. 
     * The middle reply need about > 0.02s.
     */
    if (OK ==
        TkOamRequest(pathId, 0,
                     (OamFrame *) pMsgBuf,
                     msgLen, (OamPdu *) pMsgBuf, &msgLen)) {
        pOamCtcPayloadHead = (OamCtcPayloadHead *)INT_TO_PTR(
                                                    PTR_TO_INT(pMsgBuf) +
                                                    sizeof(OamOuiVendorExt)
                                                    + sizeof(OamTkExt));
        resAck = (CtcFwUpgradeFileAck *) (pOamCtcPayloadHead + 1);
        errMsg = (CtcFwUpgradeFileError *) (pOamCtcPayloadHead + 1);
        /*
         * update TID and check the resonse if right or not
        */
        ctcServerSession.CtcTid = soc_ntohs(pOamCtcPayloadHead->tid);

        if (pOamCtcPayloadHead->dataType == CtcTftpProData) {
            if ((soc_ntohs(resAck->opcode) == CtcOpFileSendAck)
                && (soc_ntohs(resAck->num) == blockNum)) {
                resAck->opcode = soc_ntohs(resAck->opcode);
                resAck->num = soc_ntohs(resAck->num);
                sal_memcpy(reqResp, resAck, sizeof(CtcFwUpgradeFileAck));
                retVal = OK;
            } else if (soc_ntohs(resAck->opcode)
                       == CtcOpFileError) {
                errMsg->opcode = soc_ntohs(errMsg->opcode);
                errMsg->errcode = soc_ntohs(errMsg->errcode);
                msgLen = soc_ntohs(pOamCtcPayloadHead->length);
                sal_memcpy(reqResp, errMsg,
                           msgLen - sizeof(OamCtcPayloadHead));
                retVal = OK;
            } else {
                TkDbgTrace(TkDbgErrorEnable);
                retVal = ERROR;
            }
        } else {
            TkDbgTrace(TkDbgErrorEnable);
            retVal = ERROR;
        }
        /*
         * rest the seesion
         *
         * sal_memset((void
         * *)&ctcServerSession,0x0,sizeof(CtcOamFileSession));
         */
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (retVal);
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
}

