/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_DIFF_H__
#define __NEMO_DIFF_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif


AgResult ag_nd_opcode_read_config_neptune_dcr_configurations(AgNdDevice *p_device, AgNdMsgConfigNeptuneDcrConfigurations *p_msg);
AgResult ag_nd_opcode_write_config_neptune_dcr_configurations(AgNdDevice *p_device, AgNdMsgConfigNeptuneDcrConfigurations *p_msg);

AgResult ag_nd_opcode_read_bsg_config(AgNdDevice *p_device, AgNdMsgBsgConfig *p_msg);
AgResult ag_nd_opcode_write_bsg_config(AgNdDevice *p_device, AgNdMsgBsgConfig *p_msg);

AgResult ag_nd_opcode_read_diff_tso_poll(AgNdDevice *p_device, AgNdMsgDiffTsoPoll *p_msg);


#ifdef __cplusplus
}
#endif


#endif /* __NEMO_DIFF_H__ */


