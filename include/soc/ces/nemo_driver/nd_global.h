/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_GLOBAL_H__
#define __NEMO_GLOBAL_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"
#include "nd_framer.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_config_global(AgNdDevice *p_device, AgNdMsgConfigGlobal *p_msg);
AgResult ag_nd_opcode_write_config_global(AgNdDevice *p_device, AgNdMsgConfigGlobal *p_msg);

AgResult ag_nd_opcode_read_chip_info(AgNdDevice *p_device, AgNdMsgChipInfo *p_msg);

AgResult ag_nd_opcode_read_register_access(AgNdDevice *p_device, AgNdMsgRegAccess *p_msg);
AgResult ag_nd_opcode_write_register_access(AgNdDevice *p_device, AgNdMsgRegAccess *p_msg);

/*ORI*/
AgResult ag_nd_opcode_write_framer(AgNdDevice *p_device,AgFramerConfig *msg);
AgResult ag_nd_opcode_read_framer(AgNdDevice *p_device,AgFramerConfig *msg);
AgResult ag_nd_opcode_read_framer_port_status(AgNdDevice *p_device,AgFramerPortStatus* msg);
AgResult ag_nd_opcode_read_framer_port_alarm_control(AgNdDevice *p_device,FramerPortAlarmControl* msg);
AgResult ag_nd_opcode_write_framer_port_alarm_control(AgNdDevice *p_device,FramerPortAlarmControl* msg);
AgResult ag_nd_opcode_read_framer_port_pm(AgNdDevice *p_device,AgFramerPortPm *msg);
AgResult ag_nd_opcode_read_framer_port_loopback_timeslot_control(AgNdDevice *p_device,AgFramerPortTimeSlotLoopbackControl *msg);
AgResult ag_nd_opcode_write_framer_port_loopback_timeslot_control(AgNdDevice *p_device,AgFramerPortTimeSlotLoopbackControl *msg);
AgResult ag_nd_opcode_write_framer_port_loopback_control(AgNdDevice *p_device,AgFramerPortLoopbackControl *msg);
AgResult ag_nd_opcode_read_framer_port_loopback_control(AgNdDevice *p_device,AgFramerPortLoopbackControl *msg);









#ifdef __cplusplus
}
#endif

#endif /* __NEMO_GLOBAL_H__ */

