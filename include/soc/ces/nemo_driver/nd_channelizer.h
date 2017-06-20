/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_CHANNELIZER_H__
#define __NEMO_CHANNELIZER_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_config_timeslot(AgNdDevice *p_device, AgNdMsgConfigTimeslot *p_msg);
AgResult ag_nd_opcode_write_config_timeslot(AgNdDevice *p_device, AgNdMsgConfigTimeslot *p_msg);

AgResult ag_nd_opcode_read_config_channelizer(AgNdDevice *p_device, AgNdMsgConfigChannelizer *p_msg);
AgResult ag_nd_opcode_write_config_channelizer(AgNdDevice *p_device, AgNdMsgConfigChannelizer *p_msg);

#ifdef __cplusplus
}
#endif


#endif /* __NEMO_CHANNELIZER_H__ */

