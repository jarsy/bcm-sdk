/*
 * $Id: CtcAlarmProcess.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcAlarmProcess.c
 * Purpose: 
 *
 */
#include <soc/drv.h>
#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/Ethernet.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/AlarmProcess.h>
#include <soc/ea/tk371x/AlarmProcessCtc.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/event.h>
#include <soc/ea/tk371x/ea_drv.h>

#if !defined(SOC_OBJECT_TO_GPORT)
#define SOC_OBJECT_TO_GPORT(x) (x)
#endif

static AlarmToBcmEventTbl_t CtcAlarmToBcmEventTbl[]= {
    {OamCtcAttrEquipmentAlarm, SOC_EA_EVENT_EQUIPMENT_ALARM},
    {OamCtcAttrPowerAlarm, SOC_EA_EVENT_POWER_ALARM},
    {OamCtcAttrBatteryMissing, SOC_EA_EVENT_BATTERY_MISSING_ALARM},
    {OamCtcAttrBatteryFailure, SOC_EA_EVENT_BATTERY_FAILURE_ALARM},
    {OamCtcAttrBatteryVoltLow, SOC_EA_EVENT_BATTERY_VOLT_LOW_ALARM},
    {OamCtcAttrPhysicalInstusionAlarm, SOC_EA_EVENT_INTRUSION_ALARM},
    {OamCtcAttrOnuSelfTestFailure, SOC_EA_EVENT_TEST_FAILURE_ALARM},
    {OamCtcAttrOnuTempHighAlarm, SOC_EA_EVENT_ONU_TEMP_HIGH_ALARM},
    {OamCtcAttrOnuTempLowAlarm, SOC_EA_EVENT_ONU_TEMP_LOW_ALARM},
    {OamCtcAttrPonIfSwitch, SOC_EA_EVENT_IF_SWITCH_ALARM},
    {OamCtcAttrPowerMonRxPowerAlmHigh, SOC_EA_EVENT_RX_POWER_HIGH_ALARM},
    {OamCtcAttrPowerMonRxPowerAlmLow, SOC_EA_EVENT_RX_POWER_LOW_ALARM},
    {OamCtcAttrPowerMonTxPowerAlmHigh, SOC_EA_EVENT_TX_POWER_HIGH_ALARM},
    {OamCtcAttrPowerMonTxPowerAlmLow, SOC_EA_EVENT_TX_POWER_LOW_ALARM},
    {OamCtcAttrPowerMonTxBiasAlmHigh, SOC_EA_EVENT_TX_BIAS_HIGH_ALARM},
    {OamCtcAttrPowerMonTxBiasAlmLow, SOC_EA_EVENT_TX_BIAS_LOW_ALARM},
    {OamCtcAttrPowerMonVccAlmHigh, SOC_EA_EVENT_VCC_HIGH_ALARM},
    {OamCtcAttrPowerMonVccAlmLow, SOC_EA_EVENT_VCC_LOW_ALARM},
    {OamCtcAttrPowerMonTempAlmHigh, SOC_EA_EVENT_TEMP_HIGH_ALARM},
    {OamCtcAttrPowerMonTempAlmLow, SOC_EA_EVENT_TEMP_LOW_ALARM},    
    {OamCtcAttrPowerMonRxPowerWarnHigh, SOC_EA_EVENT_RX_POWER_HIGH_WARNING},
    {OamCtcAttrPowerMonRxPowerWarnLow, SOC_EA_EVENT_RX_POWER_LOW_WARNING},
    {OamCtcAttrPowerMonTxPowerWarnHigh, SOC_EA_EVENT_TX_POWER_HIGH_WARNING},
    {OamCtcAttrPowerMonTxPowerWarnLow, SOC_EA_EVENT_TX_POWER_LOW_WARNING},
    {OamCtcAttrPowerMonTxBiasWarnHigh, SOC_EA_EVENT_TX_BIAS_HIGH_WARNING},
    {OamCtcAttrPowerMonTxBiasWarnLow, SOC_EA_EVENT_TX_BIAS_LOW_WARNING},
    {OamCtcAttrPowerMonVccWarnHigh, SOC_EA_EVENT_VCC_HIGH_WARNING},
    {OamCtcAttrPowerMonVccWarnLow, SOC_EA_EVENT_VCC_LOW_WARNING},
    {OamCtcAttrPowerMonTempWarnHigh, SOC_EA_EVENT_TEMP_HIGH_WARNING},
    {OamCtcAttrPowerMonTempWarnLow, SOC_EA_EVENT_TEMP_LOW_WARNING},
    {OamCtcAttrEthPortAutoNegFailure, SOC_EA_EVENT_AUTONEG_FAILURE_ALARM},
    {OamCtcAttrEthPortLos, SOC_EA_EVENT_LOS_ALARM},
    {OamCtcAttrEthPortFailure, SOC_EA_EVENT_PORT_FAILURE_ALARM},
    {OamCtcAttrEthPortLoopback, SOC_EA_EVENT_PORT_LOOPBACK_ALARM},
    {OamCtcAttrEthPortCongestion, SOC_EA_EVENT_PORT_CONGESTION_ALARM},
    
    {OamCtcAttrAlarmCount, SOC_EA_EVENT_COUNT},
};

static OamEventVector  CtcOamEventVector[] = {
    {OamCtcAttrEquipmentAlarm,  CtcEventOnuAlarm,
     "OamCtcAttrEquipmentAlarm", 0},
    {OamCtcAttrPowerAlarm,      CtcEventOnuAlarm,
     "OamCtcAttrPowerAlarm", 0},
    {OamCtcAttrBatteryMissing,  CtcEventOnuAlarm,
     "OamCtcAttrBatteryMissing", 0},
    {OamCtcAttrBatteryFailure,  CtcEventOnuAlarm,
     "OamCtcAttrBatteryFailure", 0},
    {OamCtcAttrBatteryVoltLow,  CtcEventOnuAlarm,
     "OamCtcAttrBatteryVoltLow", 0},
    {OamCtcAttrPhysicalInstusionAlarm,  CtcEventOnuAlarm,
     "OamCtcAttrPhysicalInstusionAlarm", 0},
    {OamCtcAttrOnuSelfTestFailure,      CtcEventOnuAlarm,
     "OamCtcAttrOnuSelfTestFailure",0},
    {OamCtcAttrOnuTempHighAlarm,        CtcEventOnuAlarm,
     "OamCtcAttrOnuTempHighAlarm", 0},
    {OamCtcAttrOnuTempLowAlarm,         CtcEventOnuAlarm,
     "OamCtcAttrOnuTempLowAlarm", 0},
    {OamCtcAttrIadConnectionFailure,    CtcEventOnuAlarm,
     "OamCtcAttrIadConnectionFailure",0},
    {OamCtcAttrPonIfSwitch,             CtcEventOnuAlarm,
     "OamCtcAttrPonIfSwitch", 0},
     
    {OamCtcAttrPowerMonRxPowerAlmHigh,  CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonRxPowerAlmHigh", 0},
    {OamCtcAttrPowerMonRxPowerAlmLow,   CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonRxPowerAlmLow",0},
    {OamCtcAttrPowerMonTxPowerAlmHigh,  CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxPowerAlmHigh", 0},
    {OamCtcAttrPowerMonTxPowerAlmLow,   CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxPowerAlmLow",0},
    {OamCtcAttrPowerMonTxBiasAlmHigh,   CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxBiasAlmHigh",0},
    {OamCtcAttrPowerMonTxBiasAlmLow,    CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxBiasAlmLow",0},
    {OamCtcAttrPowerMonVccAlmHigh,      CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonVccAlmHigh",0},
    {OamCtcAttrPowerMonVccAlmLow,       CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonVccAlmLow", 0},
    {OamCtcAttrPowerMonTempAlmHigh,     CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTempAlmHigh",0},
    {OamCtcAttrPowerMonTempAlmLow,      CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTempAlmLow",0},
    {OamCtcAttrPowerMonRxPowerWarnHigh, CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonRxPowerWarnHigh", 0},
    {OamCtcAttrPowerMonRxPowerWarnLow,  CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonRxPowerWarnLow", 0},
    {OamCtcAttrPowerMonTxPowerWarnHigh, CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxPowerWarnHigh", 0},
    {OamCtcAttrPowerMonTxPowerWarnLow,  CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxPowerWarnLow", 0},
    {OamCtcAttrPowerMonTxBiasWarnHigh,  CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxBiasWarnHigh", 0},
    {OamCtcAttrPowerMonTxBiasWarnLow,   CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTxBiasWarnLow",0},
    {OamCtcAttrPowerMonVccWarnHigh,     CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonVccWarnHigh",0},
    {OamCtcAttrPowerMonVccWarnLow,      CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonVccWarnLow",0},
    {OamCtcAttrPowerMonTempWarnHigh,    CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTempWarnHigh",0},
    {OamCtcAttrPowerMonTempWarnLow,     CtcEventPonIfAlarm,
     "OamCtcAttrPowerMonTempWarnLow",0},
    {OamCtcAttrCardAlarm,               CtcEventPonIfAlarm,
     "OamCtcAttrCardAlarm", 0},
    {OamCtcAttrSelfTestFailure,         CtcEventPonIfAlarm,
     "OamCtcAttrSelfTestFailure", 0},

    {OamCtcAttrEthPortAutoNegFailure,   CtcEventPortAlarm,
     "OamCtcAttrEthPortAutoNegFailure",0},
    {OamCtcAttrEthPortLos,              CtcEventPortAlarm,
     "OamCtcAttrEthPortLos", 0},
    {OamCtcAttrEthPortFailure,          CtcEventPortAlarm,
     "OamCtcAttrEthPortFailure", 0},
    {OamCtcAttrEthPortLoopback,         CtcEventPortAlarm,
     "OamCtcAttrEthPortLoopback", 0},
    {OamCtcAttrEthPortCongestion,       CtcEventPortAlarm,
     "OamCtcAttrEthPortCongestion", 0},
    {OamCtcAttrPotsPortFailure,         CtcEventPortAlarm,
     "OamCtcAttrPotsPortFailure", 0},
    {OamCtcAttrE1PortFailure,           CtcEventPortAlarm,
     "OamCtcAttrE1PortFailure", 0},
    {OamCtcAttrE1TimingUnlock,          CtcEventPortAlarm,
     "OamCtcAttrE1TimingUnlock", 0},
    {OamCtcAttrE1Los,                   CtcEventPortAlarm, 
    "OamCtcAttrE1Los", 0},
    {OamAlmNone, NULL, "OamAlmNone", 0}
};

extern int
soc_ea_event_generate(int unit,  soc_switch_event_t event, uint32 arg1,
                   uint32 arg2, uint32 arg3, soc_ea_event_userdata_t *data);

uint32 
CtcEventAlarmToBcmEvents(uint32 alarm_id)
{
    int32 id = 0;
    
    while((alarm_id != CtcAlarmToBcmEventTbl[id].alarm_id) &&
        (OamCtcAttrAlarmCount != CtcAlarmToBcmEventTbl[id].alarm_id)){
        id++;
    }

    return CtcAlarmToBcmEventTbl[id].bcm_event;
}

uint32
CtcEventBcmEventsToCtcAlarm(uint32 event_id)
{
    int32 id = 0;
    
    while((event_id != CtcAlarmToBcmEventTbl[id].bcm_event) &&
        (SOC_EA_EVENT_COUNT != CtcAlarmToBcmEventTbl[id].bcm_event)){
        id++;
    }

    return CtcAlarmToBcmEventTbl[id].alarm_id;
}


int32 
CtcEventOnuAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 obj_num;
    uint16 alm_id;
    uint8  alm_state;
    soc_ea_event_userdata_t user_data;
    OamEventCtcAlarmOnuIfInfo onu_if_info;
    OamEventCtcAlarm *pCtcEvent = (OamEventCtcAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    obj_num = soc_ntohl(pCtcEvent->obj_num);
    alm_id = soc_ntohs(pCtcEvent->alm_id);
    alm_state = pCtcEvent->alm_state;
           
    switch(alm_id){
        case OamCtcAttrEquipmentAlarm:
        case OamCtcAttrPowerAlarm: 
        case OamCtcAttrBatteryVoltLow:
        case OamCtcAttrOnuSelfTestFailure:
        case OamCtcAttrOnuTempHighAlarm:
        case OamCtcAttrOnuTempLowAlarm:
        case OamCtcAttrIadConnectionFailure:
        case OamCtcAttrPonIfSwitch:
            sal_memcpy(&onu_if_info, pCtcEvent->info, 
                sizeof(OamEventCtcAlarmOnuIfInfo));
            onu_if_info.info = soc_ntohl(onu_if_info.info);
            
            user_data.len = sizeof(OamEventCtcAlarmPonIfInfo);
            sal_memcpy(user_data.data, &onu_if_info, sizeof(OamEventCtcAlarmOnuIfInfo));
            
            rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                CtcEventAlarmToBcmEvents(alm_id), SOC_OBJECT_TO_GPORT(obj_num), 
                alm_state, (void *)&user_data);
            break;
        case OamCtcAttrBatteryMissing:
        case OamCtcAttrBatteryFailure:
        case OamCtcAttrPhysicalInstusionAlarm:
            rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                CtcEventAlarmToBcmEvents(alm_id), SOC_OBJECT_TO_GPORT(obj_num), 
                alm_state, NULL);
            break;
        default:
            return ERROR;
    }
    
    return rv;
}


int32 
CtcEventPonIfAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 obj_num;
    uint16 alm_id;
    uint8  alm_state;
    soc_ea_event_userdata_t user_data;
    OamEventCtcAlarmPonIfInfo pon_if_info;
    OamEventCtcAlarm *pCtcEvent = (OamEventCtcAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    obj_num = soc_ntohl(pCtcEvent->obj_num);
    alm_id = soc_ntohs(pCtcEvent->alm_id);
    alm_state = pCtcEvent->alm_state;
        
    switch(alm_id){
        case OamCtcAttrPowerMonRxPowerAlmHigh:
        case OamCtcAttrPowerMonRxPowerAlmLow:
        case OamCtcAttrPowerMonTxPowerAlmHigh:
        case OamCtcAttrPowerMonTxPowerAlmLow:
        case OamCtcAttrPowerMonTxBiasAlmHigh:
        case OamCtcAttrPowerMonTxBiasAlmLow:
        case OamCtcAttrPowerMonVccAlmHigh:
        case OamCtcAttrPowerMonVccAlmLow:
        case OamCtcAttrPowerMonTempAlmHigh:
        case OamCtcAttrPowerMonTempAlmLow:        
        case OamCtcAttrPowerMonRxPowerWarnHigh:
        case OamCtcAttrPowerMonRxPowerWarnLow:
        case OamCtcAttrPowerMonTxPowerWarnHigh:
        case OamCtcAttrPowerMonTxPowerWarnLow:
        case OamCtcAttrPowerMonTxBiasWarnHigh:
        case OamCtcAttrPowerMonTxBiasWarnLow:         
        case OamCtcAttrPowerMonVccWarnHigh:
        case OamCtcAttrPowerMonVccWarnLow:       
        case OamCtcAttrPowerMonTempWarnHigh:
        case OamCtcAttrPowerMonTempWarnLow:
            sal_memcpy(&pon_if_info, pCtcEvent->info,
                sizeof(OamEventCtcAlarmPonIfInfo));
            pon_if_info.info = soc_ntohl(pon_if_info.info);
    
            user_data.len = sizeof(OamEventCtcAlarmPonIfInfo);
            sal_memcpy(user_data.data, &pon_if_info, 
                sizeof(OamEventCtcAlarmPonIfInfo));
            rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                CtcEventAlarmToBcmEvents(alm_id), SOC_OBJECT_TO_GPORT(obj_num),
                alm_state, (void *)&user_data);
            break;
        default:
            return ERROR;
    }
    
    return rv;
}


int32 
CtcEventPortAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 obj_num;
    uint16 alm_id;
    uint8  alm_state;
    OamEventCtcAlarm *pCtcEvent = (OamEventCtcAlarm *)event_tlv;
    int rv = OK;
    
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    obj_num = soc_ntohl(pCtcEvent->obj_num);
    alm_id = soc_ntohs(pCtcEvent->alm_id);
    alm_state = pCtcEvent->alm_state;
    
    switch(alm_id){
        case OamCtcAttrEthPortAutoNegFailure:
        case OamCtcAttrEthPortLos:
        case OamCtcAttrEthPortFailure:
        case OamCtcAttrEthPortLoopback:
        case OamCtcAttrEthPortCongestion:
            rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                CtcEventAlarmToBcmEvents(alm_id), SOC_OBJECT_TO_GPORT(obj_num), 
                alm_state, NULL);
            break;
        default:
            return ERROR;
    }
    
    return rv;
}


int32
CtcEventProcessHandler(uint8 pathId, uint8 linkId,
    OamEventExt * pOamEventTlv)
{
    OamCtcEventAlarm *pEvent;
    uint32          id = 0;
    int32           ret = OK;

    pEvent = (OamCtcEventAlarm *) pOamEventTlv;

    while (OamAlmNone != CtcOamEventVector[id].eventId) {
        if (CtcOamEventVector[id].eventId == soc_ntohs(pEvent->almID)) {
            if (CtcOamEventVector[id].handler) {
                ret =
                    CtcOamEventVector[id].handler(pathId, linkId, pEvent);
                CtcOamEventVector[id].Stats++;
            } else {
                TkDbgTrace(TkDbgErrorEnable);
                ret = ERROR;
            }
            break;
        }
        id++;
    }

    if (OamAlmNone == CtcOamEventVector[id].eventId) {
        TkDbgInfoTrace(TkDbgLogTraceEnable,
                       ("receive an unknown CTC alarm.\n"));
        if (TkDbgLevelIsSet(TkDbgLogTraceEnable)) {
            TkDbgDataDump((uint8 *)pOamEventTlv, sizeof(OamEventExt), 16);
        }
        ret = ERROR;
    }

    return ret;
}

