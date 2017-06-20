/*
 * $Id: TkMsgProcess.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkMsgProcess.c
 * Purpose: 
 *
 */
 
#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/Ethernet.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/OamProcess.h>
#include <soc/ea/tk371x/TkMsg.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/TkInit.h>  
#include <soc/ea/tk371x/TkDebug.h>


extern const MacAddr OamMcMacAddr;


static uint8(*ieeeAlmProcessFn) (uint8 , uint8 *, short) = NULL;

Bool
TkDataProcessHandle(uint8 pathId, TkMsgBuff * pMsg, uint16 len)
{
    OamFrame       *pOamFrame = (OamFrame *) pMsg->buff;
    OamMsg         *pOamMsg = &(pOamFrame->Data.OamHead);

    if (EthertypeOam == soc_ntohs(pOamFrame->EthHeader.type)
        && (0x03 == (pOamFrame->Data.OamHead.subtype))) {
            if ((uint8) OamOpEventNotification == pOamMsg->opcode)  
            {
                pMsg->mtype = 0;
                if (TkDbgLevelIsSet(TkDbgMsgEnable)) {
                    TkDbgPrintf(("\r\nSend a message to Alm task\n"));
                    TkDbgDataDump((uint8 *) pMsg, len, 16);
                }
                sal_msg_snd(gTagLinkCfg[pathId].almMsgQid,(char *) pMsg, len, WAIT_FOREVER);
            } else {
                pMsg->mtype = pathId;
                if (TkDbgLevelIsSet(TkDbgMsgEnable)) {
                    TkDbgPrintf(("\r\nSend a message to response queue. pathId:%d\n", pathId));
                    TkDbgDataDump((uint8 *) pMsg, len, 16);
                }
                sal_msg_snd(gTagLinkCfg
                            [pathId].resMsgQid,
                            (char *) pMsg, len, WAIT_FOREVER);
            }
    } else {
        if (TkDbgLevelIsSet(TkDbgMsgEnable)) {
            TkDbgPrintf(("\r\nIs not oam package\r\n"));
            TkDbgDataDump((uint8 *) pMsg, len, 16);
        }
    }

    return TRUE;
}

void
_TkDataProcessHandle(uint8 pathId, char *data, uint16 len)
{
    TkMsgBuff      *pMsgBuf = (TkMsgBuff *) TkOamMemGet(pathId);

    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return;
    }

    sal_memcpy(pMsgBuf->buff, data, len);
    TkDataProcessHandle(pathId, pMsgBuf, len);
    TkOamMemPut(pathId,(void *) pMsgBuf);
    return;
}


void
TkAlarmRxThread(void *cookie)
{
    int16           MsgLen = 0;
    TkMsgBuff      *pMsgBuf ;
    uint32          pathId = PTR_TO_INT(cookie);

    pMsgBuf = (TkMsgBuff *) TkOamMemGet(pathId);

    while (1) {
        if ((MsgLen =
             sal_msg_rcv(gTagLinkCfg[pathId].almMsgQid,
                         (char *) pMsgBuf,
                         TK_MAX_RX_TX_DATA_LENGTH, WAIT_FOREVER)) < 0) {
            sal_usleep(1);
            continue;
        } else {
            if (TkDbgLevelIsSet(TkDbgMsgEnable | TkDbgAlmEnable)) {
                TkDbgPrintf(("\r\nAlmTask received a msg\n"));
                TkDbgDataDump((uint8 *) pMsgBuf, MsgLen, 16);
            }
            if (ieeeAlmProcessFn)
                ieeeAlmProcessFn(pathId, pMsgBuf->buff, MsgLen);
        }
        sal_usleep(1);
    }
    TkOamMemPut(pathId,(void *) pMsgBuf);
}

void
TkAlmHandlerRegister(uint8 pathId, uint8(*funp)
                         (uint8 , uint8 *, short))
{
    if (NULL == funp) {
        return;
    }

    ieeeAlmProcessFn = funp;

    return;
}
