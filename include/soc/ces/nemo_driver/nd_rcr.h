/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_RCR_H__
#define __NEMO_RCR_H__

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_write_rcr_config_tpp_timestamp_rate(AgNdDevice *p_device, AgNdMsgRcrConfigTppTimestampRate *p_msg);
AgResult ag_nd_opcode_read_rcr_config_tpp_timestamp_rate(AgNdDevice *p_device, AgNdMsgRcrConfigTppTimestampRate *p_msg);
AgResult ag_nd_opcode_write_rcr_config_channel(AgNdDevice *p_device, AgNdMsgRcrConfigChannel *p_msg);
AgResult ag_nd_opcode_read_rcr_config_channel(AgNdDevice *p_device, AgNdMsgRcrConfigChannel *p_msg);
AgResult ag_nd_opcode_read_rcr_pm(AgNdDevice *p_device, AgNdMsgRcrPm *p_msg);
AgResult ag_nd_opcode_read_rcr_fll(AgNdDevice *p_device, AgNdMsgRcrFll *p_msg);
AgResult ag_nd_opcode_read_rcr_pll(AgNdDevice *p_device, AgNdMsgRcrPll *p_msg);
AgResult ag_nd_opcode_read_rcr_recent_packet(AgNdDevice *p_device, AgNdMsgRcrRecent *p_msg);
AgResult ag_nd_opcode_read_rcr_timestamp(AgNdDevice *p_device, AgNdMsgRcrTs *p_msg);

void ag_nd_rcr_init(AgNdDevice *p_device);

#ifdef __cplusplus
}
#endif

#endif /* #define __NEMO_RCR_H__ */

