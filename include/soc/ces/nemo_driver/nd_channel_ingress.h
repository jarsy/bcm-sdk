/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __CHANNEL_INGRESS_H__
#define __CHANNEL_INGRESS_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_config_channel_ingress(AgNdDevice *p_device, AgNdMsgConfigChannelIngress *p_msg);
AgResult ag_nd_opcode_write_config_channel_ingress(AgNdDevice *p_device, AgNdMsgConfigChannelIngress *p_msg);
AgResult ag_nd_opcode_write_config_control_word(AgNdDevice *p_device, AgNdMsgConfigCw *p_msg);
AgResult ag_nd_opcode_write_config_dest_mac(AgNdDevice *p_device, AgNdMsgConfigDestMac *p_msg);
AgResult ag_nd_opcode_write_config_sequence_number(AgNdDevice *p_device, AgNdMsgConfigSeqNumber *p_msg);


AG_BOOL  ag_nd_channel_ingress_is_enabled(AgNdDevice *p_device, AgNdChannel n_channel_id);
AgResult ag_nd_channel_ingress_enable(AgNdDevice *p_device, AgNdChannel n_channel_id, AG_U32);
AgResult ag_nd_channel_ingress_disable(AgNdDevice *p_device, AgNdChannel n_channel_id, AG_U32);

AgResult ag_nd_opcode_read_pm_ingress(AgNdDevice *p_device, AgNdMsgPmIngress *p_msg);

#ifdef __cplusplus
}
#endif


#endif /* __CHANNEL_INGRESS_H__ */


