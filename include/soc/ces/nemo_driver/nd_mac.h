/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_MAC_H__
#define __NEMO_MAC_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif


AgResult ag_nd_opcode_read_config_mac(AgNdDevice *p_device, AgNdMsgConfigMac *p_msg);
AgResult ag_nd_opcode_write_config_mac(AgNdDevice *p_device, AgNdMsgConfigMac *p_msg);

AgResult ag_nd_opcode_read_config_phy(AgNdDevice *p_device, AgNdMsgConfigPhy *p_msg);
AgResult ag_nd_opcode_write_config_phy(AgNdDevice *p_device, AgNdMsgConfigPhy *p_msg);

AgResult ag_nd_opcode_read_mac_pm(AgNdDevice *p_device, AgNdMsgPmMac *p_msg);

void ag_nd_mac_bit_rw_reg32_one_pass(AgNdDevice *, AgNdRegProperties *, AG_U32, AG_U32 b_test);
void ag_nd_mac_bit_rw_reg32(AgNdDevice*, AgNdRegProperties*, AG_U32, AG_BOOL b_test);
void ag_nd_mac_bit_ro_reg32(AgNdDevice*, AgNdRegProperties*, AG_U32, AG_BOOL b_test);

#ifdef __cplusplus
}
#endif


#endif /* __NEMO_MAC_H__ */

