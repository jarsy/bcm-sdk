/*
 * $Id: IeeeAlarmProcess.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     IeeeAlarmProcess.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/AlarmProcess.h>
#include <soc/ea/tk371x/AlarmProcessIeee.h>
#include <soc/ea/tk371x/AlarmProcessCtc.h>
#include <soc/ea/tk371x/AlarmProcessTk.h>
#include <soc/ea/tk371x/TkDebug.h>


static OamEventVector  StdOamEventVector[] = {
    {OamEventErrSymbolPeriod,
     (OamEventHandler) IeeeEventErrSymPeriod,
     "OamEventErrSymbolPeriod", 0}
    ,
    {OamEventErrFrameCount,
     (OamEventHandler) IeeeEventErrFrame,
     "OamEventErrFrameCount", 0}
    ,
    {OamEventErrFramePeriod,
     (OamEventHandler) IeeeEventErrFrPeriod,
     "OamEventErrFramePeriod", 0}
    ,
    {OamEventErrFrameSecSummary,
     (OamEventHandler) IeeeEventErrFrSecondsSum,
     "OamEventErrFrameSecSummary", 0}
    ,
    {OamEventErrVendor,
     (OamEventHandler) IeeeEventVendorHandler,
     "OamEventErrVendor", 0}
    ,
    {OamEventErrVendorOld, NULL,
     "OamEventErrVendorOld", 0}
    ,
    {OamEventEndOfTlvMarker, NULL,
     "OamEventEndOfTlvMarker", 0}
    ,
};

Bool
IeeeEventIsTkAlarm(uint8 pathId, uint8 linkId, OamEventTlv * pOamEventTlv)
{
    OamEventExt    *pEvent = (OamEventExt *) pOamEventTlv;
    Bool            ret = TRUE;

    if (NULL == pOamEventTlv) {
        ret = FALSE;
    }

    if (OamEventErrVendor == pEvent->tlv.type
        && !sal_memcmp((void *) &(pEvent->oui),
                       (void *) &TeknovusOui, sizeof(IeeeOui))) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }

    return ret;
}

Bool
IeeeEventIsCtcAlarm(uint8 pathId, uint8 linkId, OamEventTlv * pOamEventTlv)
{
    OamEventExt    *pEvent = (OamEventExt *) pOamEventTlv;
    Bool            ret = TRUE;

    if (NULL == pOamEventTlv) {
        ret = FALSE;
    }

    if (OamEventErrVendor == pEvent->tlv.type
        && !sal_memcmp((void *) &(pEvent->oui),
                       (void *) &CTCOui, sizeof(IeeeOui))) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }

    return ret;
}

int32
IeeeEventErrSymPeriod(uint8 pathId, uint8 linkId,
                                OamEventTlv * pOamEventTlv)
{
    if (NULL == pOamEventTlv) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    return OK;
}

int32
IeeeEventErrFrame(uint8 pathId, uint8 linkId,OamEventTlv * pOamEventTlv)
{
    if (NULL == pOamEventTlv) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    return OK;
}

int32
IeeeEventErrFrPeriod(uint8 pathId, uint8 linkId,OamEventTlv * pOamEventTlv)
{
    if (NULL == pOamEventTlv) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    return OK;
}

int32
IeeeEventErrFrSecondsSum(uint8 pathId,
                         uint8 linkId, OamEventTlv * pOamEventTlv)
{
    if (NULL == pOamEventTlv) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    return OK;
}

int32
IeeeEventVendorHandler(uint8 pathId,
                       uint8 linkId, OamEventTlv * pOamEventTlv)
{
    OamEventExt    *pEvent;
    int32           ret = OK;

    if (NULL == pOamEventTlv) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pEvent = (OamEventExt *) pOamEventTlv;

    if (!sal_memcmp
        ((void *) &(pEvent->oui), (void *) &CTCOui, sizeof(IeeeOui))) {
        ret = CtcEventProcessHandler(pathId, linkId, pEvent);
    } else
        if (!sal_memcmp
            ((void *) &(pEvent->oui),
             (void *) &TeknovusOui, sizeof(IeeeOui))) {
        ret = TkEventProcessHandler(pathId, linkId, pEvent);
    } else {
        ret = ERROR;
        TkDbgTrace(TkDbgErrorEnable);
    }

    return ret;
}

int32
IeeeEventHandler(uint8 pathId, uint8 linkId, OamEventTlv * pOamEventTlv)
{
    uint8           id = 0;
    int32           ret = OK;

    if (NULL == pOamEventTlv) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    while (StdOamEventVector[id].eventId != OamEventEndOfTlvMarker) {
        if (StdOamEventVector[id].eventId == pOamEventTlv->type) {
            if (StdOamEventVector[id].handler) {
                ret =
                    StdOamEventVector[id].handler(pathId, linkId,
                                                  pOamEventTlv);
                StdOamEventVector[id].Stats++;
            } else {
                TkDbgTrace(TkDbgErrorEnable);
                ret = ERROR;
            }
            break;
        }
        id++;
    }

    if (StdOamEventVector[id].eventId == OamEventEndOfTlvMarker) {
        TkDbgInfoTrace(TkDbgErrorEnable, ("Error Ieee Alarm Message.\n"));
    }
    return ret;
}

