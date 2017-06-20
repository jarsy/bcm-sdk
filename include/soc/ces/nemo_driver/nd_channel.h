/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM 
 */
#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_config_channel_enable(AgNdDevice *p_device, AgNdMsgConfigChannelEnable *p_msg);
AgResult ag_nd_opcode_write_config_channel_enable(AgNdDevice *p_device, AgNdMsgConfigChannelEnable *p_msg);

#ifdef __cplusplus
}
#endif


#endif /* __CHANNEL_H__ */

