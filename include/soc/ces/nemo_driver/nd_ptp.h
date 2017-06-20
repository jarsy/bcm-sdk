/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __PTP_H__
#define __PTP_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_config_ptp_tsg(AgNdDevice *p_device, AgNdMsgConfigPtpTsg *p_msg);
AgResult ag_nd_opcode_write_config_ptp_tsg(AgNdDevice *p_device, AgNdMsgConfigPtpTsg *p_msg);

AgResult ag_nd_opcode_read_config_ptp_tsg_enable(AgNdDevice *p_device, AgNdMsgConfigPtpTsgEnable *p_msg);
AgResult ag_nd_opcode_write_config_ptp_tsg_enable(AgNdDevice *p_device, AgNdMsgConfigPtpTsgEnable *p_msg);

AgResult ag_nd_opcode_read_config_ptp_clk_source(AgNdDevice *p_device, AgNdMsgConfigPtpClkSrc *p_msg);
AgResult ag_nd_opcode_write_config_ptp_clk_source(AgNdDevice *p_device, AgNdMsgConfigPtpClkSrc *p_msg);

AgResult ag_nd_opcode_read_config_ptp_stateless(AgNdDevice *p_device, AgNdMsgConfigPtpStateless *p_msg);
AgResult ag_nd_opcode_write_config_ptp_stateless(AgNdDevice *p_device, AgNdMsgConfigPtpStateless *p_msg);

AgResult ag_nd_opcode_read_config_ptp_brg(AgNdDevice *p_device, AgNdMsgConfigPtpBrg *p_msg);
AgResult ag_nd_opcode_write_config_ptp_brg(AgNdDevice *p_device, AgNdMsgConfigPtpBrg *p_msg);

AgResult ag_nd_opcode_write_config_ptp_channel_ingress(AgNdDevice *p_device, AgNdMsgConfigPtpCHannelIngress *p_msg);

AgResult ag_nd_opcode_write_config_ptp_channel_egress(AgNdDevice *p_device, AgNdMsgConfigPtpCHannelEgress *p_msg);

AgResult ag_nd_opcode_read_config_ptp_channel_enable_ingress(AgNdDevice *p_device, AgNdMsgConfigPtpChannelEnableIngress *p_msg);
AgResult ag_nd_opcode_write_config_ptp_channel_enable_ingress(AgNdDevice *p_device, AgNdMsgConfigPtpChannelEnableIngress *p_msg);

AgResult ag_nd_opcode_read_config_ptp_channel_enable_egress(AgNdDevice *p_device, AgNdMsgConfigPtpChannelEnableEgress *p_msg);
AgResult ag_nd_opcode_write_config_ptp_channel_enable_egress(AgNdDevice *p_device, AgNdMsgConfigPtpChannelEnableEgress *p_msg);

AgResult ag_nd_opcode_write_command_ptp_correct_timestamp(AgNdDevice *p_device, AgNdMsgCommandPtpCorrectTs *p_msg);
                           
AgResult ag_nd_opcode_write_command_ptp_adjustment(AgNdDevice *p_device, AgNdMsgCommandPtpAdjustment *p_msg);

AgResult ag_nd_opcode_read_ptp_counters(AgNdDevice *p_device, AgNdMsgPtpCounters *p_msg);

#ifdef __cplusplus
}
#endif


#endif /* __PTP_H__ */



