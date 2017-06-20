/*
 * $Id: AlarmProcessCtc.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     AlarmProcessCtc.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_AlarmProcessCtc_H
#define _SOC_EA_AlarmProcessCtc_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/AlarmProcess.h>    


typedef enum {
    PowerDownAlmState       = 0x00000001,
    PowerVoltOverAlmState   = 0x00000002,
    PowerVoltLessAlmState   = 0x00000003,
    PowerEndAlmState        = 0x7FFFFFFF
} PACK OamPowerAlarmState;

typedef enum {
    PonIfPonLosAlmState         = 0x00000001,
    PonIfMpcpTimeoutAlmState    = 0x00000002,
    PonErThresholdOverAlmState  = 0x00000003,
    PonIfEndAlmState            = 0x7FFFFFFF
} PACK OamPonIfSwitchAlarmState;


/*
 * OamCtcInstNumS - the CTC instance  number struct
 */
typedef struct {
    uint8   portType;
    uint8   slotNum;
    uint16  portNum;
} PACK OamCtcInstNumS;

/*
 * OamCtcInstNumU - the CTC instance number union
 */
typedef union {
    OamCtcInstNumS  sNum;
    uint32          uNum;
} PACK OamCtcInstNumU;

/*
 * OamEventCtcAlarm - the CTC Alarm
 */
 typedef struct {
    OamEventExt     ext;
    uint16          objType;
    OamCtcInstNumU  instNum;
    uint16          almID;
    uint16          timeStamp;
    uint8           state;
    uint8           info[0];
} PACK OamCtcEventAlarm;

typedef struct {
    uint16 obj_type;
    uint32 obj_num;
    uint16 alm_id;
    uint16 time_stamp;
    uint8  alm_state;
    uint8  info[1];
} PACK OamEventCtcAlarm;

typedef struct {
    uint32 info;
} PACK OamEventCtcAlarmPonIfInfo;

typedef struct {
    uint32 info;
} PACK OamEventCtcAlarmOnuIfInfo;


uint32 
CtcEventAlarmToBcmEvents(uint32 alarm_id);

uint32
CtcEventBcmEventsToCtcAlarm(uint32 event_id);

int32 
CtcEventOnuAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32 
CtcEventPonIfAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32 
CtcEventPortAlarm(uint8 path_id, uint8 link_id, void *event_tlv);

int32   
CtcEventProcessHandler (uint8 pathId, uint8 linkId, 
    OamEventExt * pOamEventTlv);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_AlarmProcessCtc_H */
