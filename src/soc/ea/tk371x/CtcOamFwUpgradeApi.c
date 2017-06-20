/*
 * $Id: CtcOamFwUpgradeApi.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcOamFwUpgradeApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/FileTransCtc.h>
#include <soc/ea/tk371x/FileTransTk.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/CtcOamFwUpgradeApi.h>
#include <soc/ea/tk371x/TkDebug.h>

extern CtcOamFileSession            ctcServerSession;

/*
 *  Function:
 *      CtcExtFirmwareUpgradeFile
 * Purpose:
 *      Upgrade a firmware to ONU using the CTC OAM 
 * Parameters:
 *      pathid    - which chipset you want to upgrade.
 *      Len    - The file length
 *      pLoadBuf    - The file data.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      
 */
int32
CtcExtFirmwareUpgradeFile(uint8 pathId, uint32 Len, uint8 * pLoadBuf)
{
    int32           retVal = OK;
    char            fileName[16] = "app";
    char            modeString[16] = "Octet";
    uint16          ackBlockNum = 0;
    char           *replyResBuff = NULL;
    CtcFwUpgradeFileAck *pAckMsg;
    CtcFwUpgradeFileError *pErrMsg;

    replyResBuff = (char *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == replyResBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pErrMsg = (CtcFwUpgradeFileError *) replyResBuff;
    pAckMsg = (CtcFwUpgradeFileAck *) replyResBuff;
    /*
     * clear the ctc server session
     */
    sal_memset(&ctcServerSession, 0x0, sizeof(CtcOamFileSession));
    /*
     * maybe we need get llid to fill it in this field
     */
    ctcServerSession.CtcTid = 0x0000;
    ctcServerSession.state = OamFileWriting;
    ctcServerSession.nextBlkNum = 0;
    ctcServerSession.left = Len;
    ctcServerSession.currFilePos = pLoadBuf;

    retVal = CtcSExtOamFileTranReq(pathId, fileName, modeString, (uint8 *)
                                   replyResBuff);

    if (OK == retVal)
        do {
            /*
             * update first for the index is start at 1 in CTC spec
             */   
            ctcServerSession.nextBlkNum++;
            ctcServerSession.left -= ctcServerSession.lastSend;
            ctcServerSession.currFilePos = (uint8 *) INT_TO_PTR(
                                                      PTR_TO_INT(ctcServerSession.currFilePos) 
													  + ctcServerSession.lastSend);
            if (CtcOpFileError == pErrMsg->opcode) {
                TkDbgPrintf(("Errmsg:%s\n", pErrMsg->errMsg));
                break;
            }

            if (CtcOpFileSendAck != pAckMsg->opcode) {
                break;
            }
            ackBlockNum = pAckMsg->num;
            /*
             * TkDbgPrintf(("reqBlockNum = %d\n",ackBlockNum));
             * TkDbgPrintf(("ctcServerSession.nextBlkNum = %d\n",
             * ctcServerSession.nextBlkNum));
             */
            if ((ackBlockNum + 1) != ctcServerSession.nextBlkNum) {
                break;
            }
            if (ctcServerSession.left == 0) {
                /*
                 * no data left
                 */
                break;
            }

            ctcServerSession.lastSend =
                (ctcServerSession.left >
                 CTC_MAX_FILE_SIZE) ? CTC_MAX_FILE_SIZE : ctcServerSession.
                left;

            retVal =
                CtcSExtOamFileSendData(pathId,
                                       ackBlockNum
                                       + 1,
                                       ctcServerSession.lastSend,
                                       ctcServerSession.currFilePos,
                                       (uint8 *)
                                       replyResBuff);
        }
        while (OK == retVal);

    TkOamMemPut(pathId,(void *) replyResBuff);
    return retVal;
}

/*
 *  Function:
 *      CtcExtFirmwareUpgradeActivateImg
 * Purpose:
 *      Try to run the firmware which just upgraded to the EPON chipset
 * Parameters:
 *      pathid    - Which chipset you want to upgrade.
 *      activeFlag    - The active flag input. fixed as 0, and reserved for future definiation.
 *      respCode    - The return code. 0: Activate image success, 1: Paramerter error of the 
 *                         OAM message, 2: the EPON MAC not supporting the image activate OAM
 *                         ,3: Activate fail.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      Actiavate the firmware in the backup section and have the EPON MAC boot with this 
 *      firmware . "Back rolling" is required during the process of the CTC image upgrade. 
 *      The new firmware is upgraded to the backup section. The old firmware is still in the 
 *      active sectino. This API allows the EPON MAC to start with the upgraded firmware
 *      but still keeps the upgraded firmware in the backup section. The EPON MAC still boots
 *      with the old firmware later. This API is used in the APP control mode.
 *
 *      Activeflag is fixed to 0 to denote the activate operation. Other values will cause the 
 *      error respCode(1)
 */
int32
CtcExtFirmwareUpgradeActivateImg(uint8 pathId,
                                 uint8 activeFlag, uint8 * respCode)
{
    uint32          msgLen = 0;
    uint16          flags = 0x0050;
    uint8          *pMsgBuf = NULL;
    OamTkExt       *ctcExt;
    OamCtcPayloadHead *pOamCtcPayloadHead;
    CtcFwUpgradeComm *reqMsg;
    CtcFwUpgradeComm *resMsg;
    int32           retVal = 0;

    if (NULL == respCode) {
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


    pOamCtcPayloadHead->dataType = CtcLoadRunImageData;
    pOamCtcPayloadHead->tid = ctcServerSession.CtcTid;
    pOamCtcPayloadHead->length = sizeof(OamCtcPayloadHead);

    reqMsg = (CtcFwUpgradeComm *) (pOamCtcPayloadHead + 1);
    reqMsg->opcode = CtcOpActImgReq;
    reqMsg->para = activeFlag;
    pOamCtcPayloadHead->length += sizeof(CtcFwUpgradeComm);

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

    /*
     * The first reply need about > 3s. The last reply need about > 10s
     * for App. 
     * The middle reply need about > 0.02s.
     */
    if (OK ==
        TkOamRequest(pathId, 0,
                     (OamFrame *) pMsgBuf,
                     msgLen, (OamPdu *) pMsgBuf, &msgLen)) {
        pOamCtcPayloadHead = (OamCtcPayloadHead *) INT_TO_PTR(
                                                    PTR_TO_INT(pMsgBuf) +
                                                    sizeof(OamOuiVendorExt)
                                                    + sizeof(OamTkExt));
        resMsg = (CtcFwUpgradeComm *) (pOamCtcPayloadHead + 1);
        /*
         * update TID and check the resonse if right or not
         */
        ctcServerSession.CtcTid = soc_ntohs(pOamCtcPayloadHead->tid);


        if ((pOamCtcPayloadHead->dataType == CtcLoadRunImageData)
            && (soc_ntohs(resMsg->opcode) == CtcOpActImgRes)) {
            *respCode = resMsg->para;
            retVal = OK;
        } else {
            TkDbgTrace(TkDbgErrorEnable);
            retVal = ERROR;
        }

        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (retVal);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (ERROR);
    }
}



/*
 *  Function:
 *      CtcExtFirmwareUpgradeCheckImg
 * Purpose:
 *      Request ONU to check the firmware using the CTC OAM
 * Parameters:
 *      pathid    - Which chipset you want to upgrade.
 *      fileSize    - The file size upgrade just now.
 *      respCode    - The check response code. 0:" check sucdess" of the download S/W and
 *                         it has been written to the flash, 1 the EPON MAC is writting the firmware
 *                         to the flash, 2 "CRC32 check error" of the upgrade firmware, 3 "parameter
 *                         error" of the OAM message, 4 The EPON MAC not supporting the image 
 *                         check OAM.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
CtcExtFirmwareUpgradeCheckImg(uint8 pathId, uint32 fileSize,
                              uint8 * respCode)
{
    uint32          msgLen = 0;
    uint16          flags = 0x0050;
    uint8          *pMsgBuf = NULL;
    OamTkExt       *ctcExt;
    OamCtcPayloadHead *pOamCtcPayloadHead;
    CtcFwUpgradeEndDnld *reqMsg;
    CtcFwUpgradeComm *resMsg;
    int32           retVal = 0;

    if (NULL == respCode) {
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


    pOamCtcPayloadHead->dataType = CtcCheckFileData;
    pOamCtcPayloadHead->tid = ctcServerSession.CtcTid;
    pOamCtcPayloadHead->length = sizeof(OamCtcPayloadHead);

    reqMsg = (CtcFwUpgradeEndDnld *) (pOamCtcPayloadHead + 1);
    reqMsg->opcode = CtcOpEndDnldReq;
    reqMsg->fileSize = fileSize;
    pOamCtcPayloadHead->length += sizeof(CtcFwUpgradeEndDnld);

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
    reqMsg->fileSize = soc_htonl(reqMsg->fileSize);
    /*
     * The first reply need about > 3s. The last reply need about > 10s
     * for App. 
     * The middle reply need about > 0.02s.
     */
    if (OK ==
        TkOamRequest(pathId, 0,
                     (OamFrame *) pMsgBuf,
                     msgLen, (OamPdu *) pMsgBuf, &msgLen)) {
        pOamCtcPayloadHead = (OamCtcPayloadHead *) INT_TO_PTR(
                                                    PTR_TO_INT(pMsgBuf) +
                                                    sizeof(OamOuiVendorExt)
                                                    + sizeof(OamTkExt));
        resMsg = (CtcFwUpgradeComm *) (pOamCtcPayloadHead + 1);
        /*
         * update TID and check the resonse if right or not
         */
        ctcServerSession.CtcTid = soc_ntohs(pOamCtcPayloadHead->tid);

        if ((pOamCtcPayloadHead->dataType == CtcCheckFileData)
            && (soc_ntohs(resMsg->opcode) == CtcOpEndDnldRes)) {
            *respCode = resMsg->para;
            retVal = OK;
        } else {
            TkDbgTrace(TkDbgErrorEnable);
            retVal = ERROR;
        }

        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (retVal);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (ERROR);
    }
}

/*
 *  Function:
 *      CtcExtFirmwareUpgradeCommitImg
 * Purpose:
 *      Request ONU to commit the firmware using the CTC OAM
 * Parameters:
 *      pathid    - Which chipset you want to upgrade.
 *      commitFlag    - The commit flag.Fixed 0 and reserved for future extension
 *      respCode    - The check response code. 0:success, 1 for parameter error. 2 for not
 *                         support this OAM. 3 for commit fail
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
CtcExtFirmwareUpgradeCommitImg(uint8 pathId,
                               uint8 commitFlag, uint8 * respCode)
{
    uint32          msgLen = 0;
    uint16          flags = 0x0050;
    uint8          *pMsgBuf = NULL;
    OamTkExt       *ctcExt;
    OamCtcPayloadHead *pOamCtcPayloadHead;
    CtcFwUpgradeComm *reqMsg;
    CtcFwUpgradeComm *resMsg;
    int32           retVal = 0;

    if (NULL == respCode) {
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


    pOamCtcPayloadHead->dataType = CtcCommitImageData;
    pOamCtcPayloadHead->tid = ctcServerSession.CtcTid;
    pOamCtcPayloadHead->length = sizeof(OamCtcPayloadHead);

    reqMsg = (CtcFwUpgradeComm *) (pOamCtcPayloadHead + 1);
    reqMsg->opcode = CtcOpCmtImgReq;
    reqMsg->para = commitFlag;
    pOamCtcPayloadHead->length += sizeof(CtcFwUpgradeComm);

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

    /*
     * The first reply need about > 3s. The last reply need about > 10s
     * for App. 
     * The middle reply need about > 0.02s.
     */
    if (OK ==
        TkOamRequest(pathId, 0,
                     (OamFrame *) pMsgBuf,
                     msgLen, (OamPdu *) pMsgBuf, &msgLen)) {
        pOamCtcPayloadHead = (OamCtcPayloadHead *) INT_TO_PTR(
                                                    PTR_TO_INT(pMsgBuf) +
                                                    sizeof(OamOuiVendorExt)
                                                    + sizeof(OamTkExt));
        resMsg = (CtcFwUpgradeComm *) (pOamCtcPayloadHead + 1);
        /*
         * update TID and check the resonse if right or not
         */
        ctcServerSession.CtcTid = soc_ntohs(pOamCtcPayloadHead->tid);

        if ((pOamCtcPayloadHead->dataType == CtcCommitImageData)
            && (soc_ntohs(resMsg->opcode) == CtcOpCmtImgRes)) {
            *respCode = resMsg->para;
            retVal = OK;
        } else {
            TkDbgTrace(TkDbgErrorEnable);
            retVal = ERROR;
        }

        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (retVal);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (ERROR);
    }
}


