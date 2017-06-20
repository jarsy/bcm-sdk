/*
 * $Id: AlarmProcess.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     AlarmProcess.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/Ethernet.h> 
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/AlarmProcess.h>
#include <soc/ea/tk371x/AlarmProcessIeee.h>
#include <soc/ea/tk371x/TkDebug.h>

uint8
AlarmProcessHandler(uint8 pathId, uint8 * pBuff, short len)
{
    uint8           linkId = 0;
    EthernetFrame  *pFrame = (EthernetFrame *) pBuff;
    OamEventNotificationMsg *pEventMsg = NULL;
    OamEventTlv    *pNextEventTlv = NULL;
    BufInfo         tmpBuffInfo;
    int             Gret = OK;

    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return FALSE;
    }

    pEventMsg = (OamEventNotificationMsg *) & (pFrame[1]);
    
    linkId = GetSourceForFlag(soc_ntohs(pEventMsg->common.flags));

    InitBufInfo
        (&tmpBuffInfo,
         len - sizeof(EthernetFrame) -
         sizeof(OamEventNotificationMsg), (uint8 *) & (pEventMsg[1])
        );

    while (GetEventTlv(&tmpBuffInfo, &pNextEventTlv, &Gret)) {
        IeeeEventHandler(pathId, linkId, pNextEventTlv);
    }

    if (OK != Gret) {
        TkDbgTrace(TkDbgErrorEnable);
    }
    return TRUE;
}

