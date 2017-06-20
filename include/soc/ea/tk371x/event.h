/*
 * $Id: event.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     events.h
 * Purpose:
 *
 */
#ifndef _SOC_EA_EVENTS_H
#define _SOC_EA_EVENTS_H
#include <soc/ea/tk371x/CtcMiscApi.h>

extern uint32 CtcEventBcmEventsToCtcAlarm(uint32 event_id);

#define _soc_ea_alarm_threshold_t CtcOamAlarmThreshold

#define _soc_ea_alarm_enable_get(unit, port, alarm_id, state) \
    CtcExtOamGetAlarmState(unit, 0, port, \
    CtcEventBcmEventsToCtcAlarm(alarm_id), state)

#define _soc_ea_alarm_enable_set(unit, port, alarm_id, state) \
    CtcExtOamSetAlarmState(unit, 0, port, \
    CtcEventBcmEventsToCtcAlarm(alarm_id), state)

#define _soc_ea_alarm_threshold_get(unit, port, alarm_id, threshold) \
    CtcExtOamGetAlarmThreshold(unit, 0, port, \
    CtcEventBcmEventsToCtcAlarm(alarm_id), threshold)

#define _soc_ea_alarm_threshold_set(unit, port, alarm_id, threshold) \
    CtcExtOamSetAlarmThreshold(unit, 0, port, \
    CtcEventBcmEventsToCtcAlarm(alarm_id), threshold)

typedef enum soc_ea_events_val_e {
    SOC_EA_EVENT_EQUIPMENT_ALARM,
    SOC_EA_EVENT_POWER_ALARM,
    SOC_EA_EVENT_BATTERY_MISSING_ALARM,
    SOC_EA_EVENT_BATTERY_FAILURE_ALARM,
    SOC_EA_EVENT_BATTERY_VOLT_LOW_ALARM,
    SOC_EA_EVENT_INTRUSION_ALARM,
    SOC_EA_EVENT_TEST_FAILURE_ALARM,
    SOC_EA_EVENT_ONU_TEMP_HIGH_ALARM,
    SOC_EA_EVENT_ONU_TEMP_LOW_ALARM,
    SOC_EA_EVENT_IF_SWITCH_ALARM,
    SOC_EA_EVENT_RX_POWER_HIGH_ALARM,
    SOC_EA_EVENT_RX_POWER_LOW_ALARM,
    SOC_EA_EVENT_TX_POWER_HIGH_ALARM,
    SOC_EA_EVENT_TX_POWER_LOW_ALARM,
    SOC_EA_EVENT_TX_BIAS_HIGH_ALARM,
    SOC_EA_EVENT_TX_BIAS_LOW_ALARM,
    SOC_EA_EVENT_VCC_HIGH_ALARM,
    SOC_EA_EVENT_VCC_LOW_ALARM,
    SOC_EA_EVENT_TEMP_HIGH_ALARM,
    SOC_EA_EVENT_TEMP_LOW_ALARM,
    SOC_EA_EVENT_RX_POWER_HIGH_WARNING,
    SOC_EA_EVENT_RX_POWER_LOW_WARNING,
    SOC_EA_EVENT_TX_POWER_HIGH_WARNING,
    SOC_EA_EVENT_TX_POWER_LOW_WARNING,
    SOC_EA_EVENT_TX_BIAS_HIGH_WARNING,
    SOC_EA_EVENT_TX_BIAS_LOW_WARNING,
    SOC_EA_EVENT_VCC_HIGH_WARNING,
    SOC_EA_EVENT_VCC_LOW_WARNING,
    SOC_EA_EVENT_TEMP_HIGH_WARNING,
    SOC_EA_EVENT_TEMP_LOW_WARNING,
    SOC_EA_EVENT_AUTONEG_FAILURE_ALARM,
    SOC_EA_EVENT_LOS_ALARM,
    SOC_EA_EVENT_PORT_FAILURE_ALARM,
    SOC_EA_EVENT_PORT_LOOPBACK_ALARM,
    SOC_EA_EVENT_PORT_CONGESTION_ALARM,
    SOC_EA_EVENT_OAM_TIMEOUT_ALARM,
    SOC_EA_EVENT_KEY_EXCHANGE_FAILURE_ALARM,
    SOC_EA_EVENT_GPIO_LINK_FAULT_ALARM,
    SOC_EA_EVENT_LOOPBACK_TEST_ENABLE_ALARM,
    SOC_EA_EVENT_GPIO_CRICTICAL_EVENT_ALARM,
    SOC_EA_EVENT_GPIO_EXTERNAL_ALARM,
    SOC_EA_EVENT_AUTH_FAILURE_ALARM,
    SOC_EA_EVENT_STATS_ALARM,
    SOC_EA_EVENT_FLASH_BUSY_ALARM,
    SOC_EA_EVENT_ONU_READY_ALARM,
    SOC_EA_EVENT_DISABLE_ALARM,
    SOC_EA_EVENT_CTC_DISCOVERY_SUCCESS_ALARM,
    SOC_EA_EVENT_LAYSER_ALWAYS_ON_ALARM,
    SOC_EA_EVENT_RESERVED_ALARM,
    SOC_EA_EVENT_COUNT
} soc_ea_events_val_t;

#endif /* _SOC_EA_EVENTS_H */
