/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __TDM_H__
#define __TDM_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_config_circuit_enable(AgNdDevice *p_device, AgNdMsgConfigCircuitEnable *p_msg);
AgResult ag_nd_opcode_write_config_circuit_enable(AgNdDevice *p_device, AgNdMsgConfigCircuitEnable *p_msg);
AgResult ag_nd_opcode_read_config_cas_data_replace(AgNdDevice *p_device, AgNdMsgConfigCasDataReplace *p_msg);
AgResult ag_nd_opcode_write_config_cas_data_replace(AgNdDevice *p_device, AgNdMsgConfigCasDataReplace *p_msg);
#ifdef __cplusplus
}
#endif

#endif /* __TDM_H__ */


