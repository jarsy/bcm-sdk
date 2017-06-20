/*
 * $Id: TkOamFwUpgradeApi.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOamFwUpgradeApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/FileTransTk.h>
#include <soc/ea/tk371x/TkOamFwUpgradeApi.h>
#include <soc/ea/tk371x/TkDebug.h>

OamFileSession  tkServerSession;

/*
 *  Function:
 *      TkExtFirmwareUpgrade
 * Purpose:
 *      Upgrade firmware to ONU using the TK OAM
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      pathId    - The link index which will be attached to the message
 *      loadType    - The file type will be upgrade. 0 for boot, 1 for app, 2 for personality, 3
 *                      for diag.
 *      Len     -  The file length 
 *      pLoadBuf    - The file data.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int
TkExtFirmwareUpgrade(uint8 pathId,
                     uint8 loadType, uint32 Len, uint8 * pLoadBuf)
{
    OamTkFileAck    TkFileRcvAck;
    uint16          recvBlkNum;
    OamTkFileErr    err;
    int             retval = OK;

    if ((loadType > (uint8) OamTkFileNumTypes)
        || (NULL == pLoadBuf)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    bzero((void *) &tkServerSession, sizeof(OamFileSession));
    tkServerSession.state = OamFileWriting;
    tkServerSession.nextBlkNum = 0;
    tkServerSession.left = Len;
    tkServerSession.currFilePos = pLoadBuf;

    /* TkExtOamSetHBState (TKOAM_HB_PAUSE);*/

    /*
     * send write load OAM message to ONU and waiting for ACK 
     */
    retval =
        TkExtOamFileTranReq(pathId, FILE_WRITE_REQ, loadType,
                            &TkFileRcvAck);
    if (OK == retval) {         /* ACK from the ONU */
        do {                    /* send the data blocks to the ONU */
            recvBlkNum = soc_ntohs(TkFileRcvAck.blockNum);
            err = TkFileRcvAck.err;
            if ((err != (uint8) OamTkFileErrOk)
                && (err != (uint8) OamTkFileErrBadBlock)) {
                if (TkDbgLevelIsSet(TkDbgErrorEnable)) {
                    TkDbgPrintf(("\r\nReceived error Ack from ONU: err = %x\n", err));
                }
                /* TkExtOamSetHBState (TKOAM_HB_NORMAL);*/
                TkDbgTrace(TkDbgErrorEnable);
                retval = ERROR;
            } else if (recvBlkNum == tkServerSession.nextBlkNum) {
                /*
                 * The received acknowledgment is for the expected data
                 * packet that we sent. 
                 */
                /*
                 * If there are no more bytes to send, we ack with blk# 0 
                 */
                tkServerSession.lastSend =
                    (uint16) ((tkServerSession.left >
                               MaxOftDataSize) ? MaxOftDataSize : tkServerSession.
                              left);
                if (0 == tkServerSession.left) {    /* All of the file has been
                                             * sent. */
                    /*
                     * Send an ack with blk # of 0, to indicate
                     * end-of-transfer 
                     */
                    retval = TkExtOamFileSendAck(pathId, OamTkFileErrOk);
                    tkServerSession.nextBlkNum = 0;
                    /* TkExtOamSetHBState (TKOAM_HB_NORMAL);*/
                    break;
                    /* return (retval);*/
                } else {        /* send the next data block */

                    tkServerSession.nextBlkNum++;
                    retval =
                        TkExtOamFileSendData
                        (pathId,
                         tkServerSession.nextBlkNum - 1,
                         tkServerSession.lastSend, tkServerSession.currFilePos,
                         &TkFileRcvAck);
                    if (OK == retval) {
                        recvBlkNum = soc_ntohs(TkFileRcvAck.blockNum);
                        err = TkFileRcvAck.err;
                        if ((recvBlkNum == tkServerSession.nextBlkNum)
                            && ((uint8)
                                OamTkFileErrOk == err)) {
                            tkServerSession.left -= tkServerSession.lastSend;
                            tkServerSession.currFilePos += tkServerSession.lastSend;
                        }
                    } else {
                        TkDbgTrace(TkDbgErrorEnable);
                        retval = ERROR;
                    }
                }
            } else if (recvBlkNum == (tkServerSession.nextBlkNum - 1)) {    /* We
                                                                     * received 
                                                                     * a
                                                                     * duplicate 
                                                                     * ACK. 
                                                                     */
                retval = OK;    /* just ignore this duplicate ACK */
            } else {
                if (TkDbgLevelIsSet(TkDbgErrorEnable)) {
                    TkDbgPrintf(("\r\nReceived error Ack: blocknum rxd = %d, expected = %d\n", 
                        recvBlkNum, tkServerSession.nextBlkNum));
                }
                TkDbgTrace(TkDbgErrorEnable);
                retval = ERROR;
            }
        }
        while (OK == retval);
    }

    if (OK != retval) {
        TkDbgTrace(TkDbgErrorEnable);
        TkExtOamFileSendAck(pathId, OamTkFileErrTimeout);
    }
    /* TkExtOamSetHBState (TKOAM_HB_NORMAL);*/
    return (retval);
}

