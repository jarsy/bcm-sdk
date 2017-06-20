/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM 
 */
#ifndef __CAS_H__
#define __CAS_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_config_cas_channel_enable(AgNdDevice *p_device, AgNdMsgConfigCasChannelEnable *p_msg);
AgResult ag_nd_opcode_write_config_cas_channel_enable(AgNdDevice *p_device, AgNdMsgConfigCasChannelEnable *p_msg);

AgResult ag_nd_opcode_write_config_cas_channel_ingress(AgNdDevice *p_device, AgNdMsgConfigCasChannelIngress *p_msg);

AgResult ag_nd_opcode_write_command_cas_channel_tx(AgNdDevice *p_device, AgNdMsgCommandCasChannelTx *p_msg);

AgResult ag_nd_opcode_read_status_cas_data(AgNdDevice *p_device, AgNdMsgStatusCasData *p_msg);
AgResult ag_nd_opcode_read_status_cas_change(AgNdDevice *p_device, AgNdMsgStatusCasChange *p_msg);

#ifdef __cplusplus
}
#endif


#endif /* __CAS_H__ */


