/*
 * $Id: AlarmProcessTk.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     AlarmProcessTk.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_AlarmProcessTk_H
#define _SOC_EA_AlarmProcessTk_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/AlarmProcess.h>


typedef struct {
    OamEventExt     ext;
    uint8           TkAlmId;
    uint8           info[0];
} PACK OamTkEventAlarm;

int32
TkEventPowerAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventGpioDyingAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventLosAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventPortDisabledAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventOamTimeOutAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventKeyExchangeAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventGpioLinkFaultAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventLoopbackAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventGpioCriticalAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventGpioAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventAuthUnavailAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventStatAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventFlashBusyAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventOnuReadyAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventOnuPonDisableAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventCtcDiscoverAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

/* No need report this to user as spec defined, just clear the private flag in the private
 *  Data. When retrieve the runtime llid count occured, it need send OAM to request the 
 *  value.
 */
int32
TkEventLinkFaultAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32
TkEventProcessHandler(uint8 pathId, uint8 linkId, OamEventExt *pOamEventTlv);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_AlarmProcessTk_H */
