/*
 * $Id: TkAlarmProcess.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkAlarmProcess.c
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
#include <soc/ea/tk371x/AlarmProcessTk.h>
#include <soc/ea/tk371x/CtcMiscApi.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/event.h>
#include <soc/ea/tk371x/ea_drv.h>

#if !defined(SOC_OBJECT_TO_GPORT)
#define SOC_OBJECT_TO_GPORT(x) x
#endif

static AlarmToBcmEventTbl_t TkAlarmToBcmEventTbl[]= {
    {OamAlmCodePower, SOC_EA_EVENT_POWER_ALARM},
    {OamAlmCodeGpioDying, SOC_EA_EVENT_POWER_ALARM},
    
    {OamAlmCodeLos, SOC_EA_EVENT_LOS_ALARM},
    {0xA6/*standy EPON LOS */, SOC_EA_EVENT_LOS_ALARM},

    {OamAlmCodePortDisabled, SOC_EA_EVENT_PORT_LOOPBACK_ALARM},
     
    {OamAlmCodeOamTimeout, SOC_EA_EVENT_OAM_TIMEOUT_ALARM},
    
    {OamAlmCodeKeyExchange, SOC_EA_EVENT_KEY_EXCHANGE_FAILURE_ALARM},
    
    {OamAlmCodeGpioLinkFault, SOC_EA_EVENT_GPIO_LINK_FAULT_ALARM},
    
    {OamAlmCodeLoopback, SOC_EA_EVENT_LOOPBACK_TEST_ENABLE_ALARM},

    {OamAlmCodeReserved61, SOC_EA_EVENT_LOS_ALARM},

    {OamAlmCodeGpioCritical, SOC_EA_EVENT_GPIO_CRICTICAL_EVENT_ALARM},

    {OamAlmCodeGpio, SOC_EA_EVENT_GPIO_EXTERNAL_ALARM},

    {OamAlmCodeAuthUnavail, SOC_EA_EVENT_AUTH_FAILURE_ALARM},

    {OamAlmCodeStatAlarm, SOC_EA_EVENT_STATS_ALARM},

    {OamAlmCodeFlashBusy, SOC_EA_EVENT_FLASH_BUSY_ALARM},

    {OamAlmCodeOnuReady, SOC_EA_EVENT_ONU_READY_ALARM},

    {OamAlmCodeOnuPonDisable, SOC_EA_EVENT_DISABLE_ALARM},

    {OamAlmCodeCtcDiscover, SOC_EA_EVENT_CTC_DISCOVERY_SUCCESS_ALARM},

    {OamAlmCodeLinkFault, SOC_EA_EVENT_RESERVED_ALARM},

    {0xB3, SOC_EA_EVENT_LAYSER_ALWAYS_ON_ALARM},
    
    {OamAlmCodeCount,SOC_EA_EVENT_COUNT}
};

static OamEventVector  TkOamEventVector[] = {
    {OamAlmCodePower,
    (OamEventHandler)TkEventPowerAlarm,
    "OamAlmCodePower",0},
        
    {OamAlmCodeGpioDying,
    (OamEventHandler)TkEventGpioDyingAlarm,
    "OamAlmCodeGpioDying",0},

    {OamAlmCodeLos,
    (OamEventHandler)TkEventLosAlarm,
    "OamAlmCodeLos",0},

    {OamAlmCodePortDisabled,
    (OamEventHandler)TkEventPortDisabledAlarm,
    "OamAlmCodePortDisabled",0},

    {OamAlmCodeOamTimeout,
    (OamEventHandler)TkEventOamTimeOutAlarm,
    "OamAlmCodeOamTimeout",0},

    {OamAlmCodeKeyExchange,
    (OamEventHandler)TkEventKeyExchangeAlarm,
    "OamAlmCodeKeyExchange",0},

    {OamAlmCodeGpioLinkFault,
    (OamEventHandler)TkEventGpioLinkFaultAlarm,
    "OamAlmCodeGpioLinkFault",0},

    {OamAlmCodeLoopback,
    (OamEventHandler)TkEventLoopbackAlarm,
    "OamAlmCodeLoopback",0},

    {OamAlmCodeReserved61,
    (OamEventHandler)TkEventLinkFaultAlarm,
    "OamAlmCodeReserved61",0},

    {OamAlmCodeGpioCritical,
    (OamEventHandler)TkEventGpioCriticalAlarm,
    "OamAlmCodeGpioCritical",0},

    {OamAlmCodeGpio,
    (OamEventHandler)TkEventGpioAlarm,
    "OamAlmCodeGpio",0},

    {OamAlmCodeAuthUnavail,
    (OamEventHandler)TkEventAuthUnavailAlarm,
    "OamAlmCodeAuthUnavail",0},

    {OamAlmCodeStatAlarm,
    (OamEventHandler)TkEventStatAlarm,
    "OamAlmCodeStatAlarm",0},

    {OamAlmCodeFlashBusy,
    (OamEventHandler)TkEventFlashBusyAlarm,
    "OamAlmCodeFlashBusy",0},

    {OamAlmCodeOnuReady,
    (OamEventHandler)TkEventOnuReadyAlarm,
    "OamAlmCodeOnuReady",0},

    {OamAlmCodeOnuPonDisable,
    (OamEventHandler)TkEventOnuPonDisableAlarm,
    "OamAlmCodeOnuPonDisable",0},

    {OamAlmCodeCtcDiscover,
    (OamEventHandler)TkEventCtcDiscoverAlarm,
    "OamAlmCodeCtcDiscover",0},

    {OamAlmCodeLinkFault,
    (OamEventHandler)TkEventCtcDiscoverAlarm,
    "OamAlmCodeLinkFault",0},

    {OamAlmNone,
    (OamEventHandler)NULL,
    "OamAlmNone",0}
};

uint32 
TkEventAlarmToBcmEvents(uint32 alarm_id)
{
    int32 id = 0;
    
    while((alarm_id != TkAlarmToBcmEventTbl[id].alarm_id) &&
        (OamCtcAttrAlarmCount != TkAlarmToBcmEventTbl[id].alarm_id)){
        id++;
    }

    return TkAlarmToBcmEventTbl[id].bcm_event;
}

int
soc_ea_event_generate(int unit,  soc_switch_event_t event, uint32 arg1,
                   uint32 arg2, uint32 arg3, soc_ea_event_userdata_t *data);

int32
TkEventPowerAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), SOC_OBJECT_TO_GPORT(obj_num), alm_state, NULL);
    
    return rv;
}


int32
TkEventGpioDyingAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventLosAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
} 

int32
TkEventPortDisabledAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventOamTimeOutAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventKeyExchangeAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventGpioLinkFaultAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventLoopbackAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventGpioCriticalAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventGpioAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventAuthUnavailAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventStatAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventFlashBusyAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventOnuReadyAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventOnuPonDisableAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}


int32
TkEventCtcDiscoverAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    uint16 alm_id;
    uint8  alm_state;
    uint8  obj_num;
    OamEventTkAlarm *ptlv = (OamEventTkAlarm *)event_tlv;
    int rv = OK;
    
        
    if(NULL == event_tlv){
        return ERROR;
    }

    if(!SOC_IS_EA(path_id)){
        return ERROR;
    }

    alm_id = ptlv->alm;
    alm_state = ptlv->state;

    switch(ptlv->context){
        case OamAlarmContextOnu:
            obj_num = 0;
            break;
        case OamAlarmContextPort:
            obj_num = soc_ntohs(ptlv->which.port);
            break;
        case OamAlarmContextLink:
            obj_num = soc_ntohs(ptlv->which.link);
            break;
        case OamAlarmContextQueue:
            obj_num = soc_ntohs(ptlv->which.queue);
            break;
        default:
            return ERROR;
            break;       
    }

    rv = soc_ea_event_generate(path_id, SOC_SWITCH_EVENT_EPON_ALARM, 
                TkEventAlarmToBcmEvents(alm_id), obj_num, alm_state, NULL);
    
    return rv;
}

/* No need report this to user as spec defined, just clear the private flag in the private
 *  Data. When retrieve the runtime llid count occured, it need send OAM to request the 
 *  value.
 */
int32
TkEventLinkFaultAlarm(uint8 path_id, uint8 link_id, void *event_tlv)
{
    int rv = OK;
    int unit = path_id;
    
    link_id = link_id;

    if(!SOC_IS_EA(unit)){
        return ERROR;
    }

    SOC_EA_PRIVATE_FLAG_CLEAR(unit, SOC_EA_CHIP_INFO_LLID_COUNT);

    _soc_ea_chip_control_flag_update(unit,socEaChipControlQueueConfig,FALSE);
    
    return rv;
}


int32
TkEventProcessHandler(uint8 pathId, uint8 linkId, OamEventExt *pOamEventTlv)
{    
    OamTkEventAlarm *pEvent;    
    uint32 id = 0;    
    int32 ret = OK;
    pEvent = (OamTkEventAlarm *) pOamEventTlv;    

    while (OamAlmNone != TkOamEventVector[id].eventId) {
        if (TkOamEventVector[id].eventId == pEvent->TkAlmId) {
            if (TkOamEventVector[id].handler) {
                ret = TkOamEventVector[id].handler(pathId, linkId,pEvent);
                TkOamEventVector[id].Stats++;
            } else {
                ret = ERROR;
            }
            
            break;        
            }        
        id++;    
    }    

    if (OamAlmNone == TkOamEventVector[id].eventId) {
        TkDbgInfoTrace(TkDbgAlmEnable, ("Unknown Tk Alarm\n"));
        if (TkDbgLevelIsSet(TkDbgErrorEnable)) {
            TkDbgDataDump((uint8 *)pOamEventTlv, sizeof(OamEventExt), 16);
        }        
        ret = ERROR;    
    }    

    return ret;
}

