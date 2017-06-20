/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __TDM_NEPTUNE_H__
#define __TDM_NEPTUNE_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult    ag_nd_opcode_write_config_neptune_oc3_hierarchy(AgNdDevice *p_device, AgNdMsgConfigNeptuneOc3Hierarchy *p_msg);
AgResult    ag_nd_opcode_read_config_neptune_oc3_hierarchy(AgNdDevice *p_device, AgNdMsgConfigNeptuneOc3Hierarchy *p_msg);

AgResult    ag_nd_opcode_write_config_neptune_circuit(AgNdDevice *p_device, AgNdMsgConfigNeptuneCircuit *p_msg);
AgResult    ag_nd_opcode_read_config_neptune_circuit(AgNdDevice *p_device, AgNdMsgConfigNeptuneCircuit *p_msg);

AgResult    ag_nd_opcode_write_config_neptune_bsg(AgNdDevice *p_device, AgNdMsgConfigNeptuneBsg *p_msg);
AgResult    ag_nd_opcode_read_config_neptune_bsg(AgNdDevice *p_device, AgNdMsgConfigNeptuneBsg *p_msg);

AgResult    ag_nd_opcode_read_status_neptune_circuit(AgNdDevice *p_device, AgNdMsgStatusNeptuneCircut *p_msg);

AG_BOOL     ag_nd_circuit_neptune_is_valid_id(AgNdDevice *p_device, AgNdCircuit n_circuit_id);
AgNdCircuit ag_nd_circuit_neptune_create_id(AG_U32 n_spe, AG_U32 n_vtg, AG_U32 n_vt);

AG_U32      ag_nd_circuit_neptune_get_spe(AgNdCircuit);
AG_U32      ag_nd_circuit_neptune_get_vtg(AgNdCircuit);
AG_U32      ag_nd_circuit_neptune_get_vt(AgNdCircuit);

AG_U32      ag_nd_circuit_neptune_get_max_idx(AgNdDevice *p_device);
AgNdCircuit ag_nd_circuit_neptune_get_next_id(AgNdDevice *p_device, AgNdCircuit n_circuit_id);

void        ag_nd_circuit_neptune_create_maps(AgNdDevice *p_device);
AG_U32      ag_nd_circuit_neptune_id_to_idx(AgNdCircuit n_circuit_id);
AgNdCircuit ag_nd_circuit_neptune_idx_to_id(AG_U32 n_circuit_idx);

#ifdef __cplusplus
}
#endif

#endif /* __TDM_NEPTUNE_H__ */



