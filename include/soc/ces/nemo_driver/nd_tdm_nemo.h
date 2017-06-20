/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __TDM_NEMO_H__
#define __TDM_NEMO_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult    ag_nd_opcode_write_config_nemo_cclk(AgNdDevice *p_device, AgNdMsgConfigNemoCClk *p_msg);
AgResult    ag_nd_opcode_read_config_nemo_cclk(AgNdDevice *p_device, AgNdMsgConfigNemoCClk *p_msg);

AgResult    ag_nd_opcode_write_config_nemo_circuit(AgNdDevice *p_device, AgNdMsgConfigNemoCircuit *p_msg);
AgResult    ag_nd_opcode_read_config_nemo_circuit(AgNdDevice *p_device, AgNdMsgConfigNemoCircuit *p_msg);

AgResult    ag_nd_opcode_write_config_nemo_brg(AgNdDevice *p_device, AgNdMsgConfigNemoBrg *p_msg);
AgResult    ag_nd_opcode_read_config_nemo_brg(AgNdDevice *p_device, AgNdMsgConfigNemoBrg *p_msg);

AG_BOOL     ag_nd_circuit_nemo_is_valid_id(AgNdDevice *p_device, AgNdCircuit n_circuit_id);
AgNdCircuit ag_nd_circuit_nemo_create_id(AG_U32 n_port);

AG_U32      ag_nd_circuit_nemo_get_max_idx(AgNdDevice *p_device);
AgNdCircuit ag_nd_circuit_nemo_get_next_id(AgNdDevice *p_device, AgNdCircuit n_circuit_id);

AG_U32      ag_nd_circuit_nemo_id_to_idx(AgNdCircuit n_circuit_id);
AgNdCircuit ag_nd_circuit_nemo_idx_to_id(AG_U32 n_circuit_idx);

#ifdef __cplusplus
}
#endif

#endif /* __TDM_NEMO_H__ */


