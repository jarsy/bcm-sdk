/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __CHANNEL_EGRESS_H__
#define __CHANNEL_EGRESS_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ND_JB_RESET_DELAY_FACTOR	3

AgResult ag_nd_opcode_read_config_jitter_buffer_params(AgNdDevice *p_device,AgNdMsgConfigJitterBufferParams *p_msg);
AgResult ag_nd_opcode_write_config_jitter_buffer_params(AgNdDevice *p_device,AgNdMsgConfigJitterBufferParams *p_msg);

AgResult ag_nd_opcode_read_config_channel_egress(AgNdDevice *p_device, AgNdMsgConfigChannelEgress *p_msg);
AgResult ag_nd_opcode_write_config_channel_egress(AgNdDevice *p_device, AgNdMsgConfigChannelEgress *p_msg);
AgResult ag_nd_opcode_read_status_egress(AgNdDevice *p_device, AgNdMsgStatusEgress *p_msg);
AgResult ag_nd_opcode_read_config_compute_ring_size(AgNdDevice *p_device, AgNdMsgConfigComputeRingSize *p_msg);

AG_BOOL  ag_nd_channel_egress_is_enabled(AgNdDevice *p_device, AgNdChannel n_channel_id);
AgResult ag_nd_channel_egress_enable(AgNdDevice *p_device, AgNdChannel n_channel_id, AG_U32);
AgResult ag_nd_channel_egress_disable(AgNdDevice *p_device, AgNdChannel n_channel_id, AG_U32 n_jb_size_milli);
void     ag_nd_channel_egress_jbf_reset(AgNdDevice *p_device, AgNdChannel n_channel_id);

AgResult ag_nd_opcode_write_command_jbf_slip(AgNdDevice *p_device, AgNdMsgCommandJbfSlip *p_msg);
AgResult ag_nd_opcode_write_command_jbf_restart(AgNdDevice *p_device, AgNdMsgCommandStopTrimming *p_msg);
AgResult ag_nd_opcode_write_command_stop_trimming(AgNdDevice *p_device, AgNdMsgCommandStopTrimming *p_msg);

AgResult ag_nd_opcode_read_pm_egress(AgNdDevice *p_device, AgNdMsgPmEgress *p_msg);

AgResult ag_nd_opcode_write_config_vlan_egress(AgNdDevice *p_device, AgNdVlanHeader* p_msg);


#ifdef __cplusplus
}
#endif


#endif /* __CHANNEL_EGRESS_H__ */


