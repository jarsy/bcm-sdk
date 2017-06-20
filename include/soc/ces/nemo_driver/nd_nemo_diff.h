/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __ND_NEMO_DIFF_H__
#define __ND_NEMO_DIFF_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_write_config_nemo_dcr_configurations(AgNdDevice *p_device, AgNdMsgConfigNemoDcrConfigurations *p_msg);
AgResult ag_nd_opcode_read_config_nemo_dcr_configurations(AgNdDevice *p_device, AgNdMsgConfigNemoDcrConfigurations *p_msg);
AgResult ag_nd_opcode_write_config_nemo_dcr_clk_source(AgNdDevice *p_device, AgNdMsgConfigNemoDcrClkSource *p_msg);
AgResult ag_nd_opcode_read_config_nemo_dcr_clk_source(AgNdDevice *p_device, AgNdMsgConfigNemoDcrClkSource *p_msg);
AgResult ag_nd_opcode_read_nemo_dcr_local_ts(AgNdDevice *p_device, AgNdMsgNemoDcrLocalTs *p_msg);
AgResult ag_nd_opcode_read_nemo_dcr_local_prime_ts(AgNdDevice *p_device, AgNdMsgNemoDcrLocalPrimeTs *p_msg);
AgResult ag_nd_opcode_read_config_nemo_dcr_local_sample_period(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSamplePeriod *p_msg);
AgResult ag_nd_opcode_write_config_nemo_dcr_local_sample_period(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSamplePeriod *p_msg);

AgResult ag_nd_opcode_write_config_nemo_dcr_system_clk_source(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSystemClkInputSource *p_msg);
AgResult ag_nd_opcode_read_config_nemo_dcr_system_clk_source(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSystemClkInputSource *p_msg);
AgResult ag_nd_opcode_read_nemo_dcr_system_local_ts(AgNdDevice *p_device, AgNdMsgNemoDcrLocalTs *p_msg);
AgResult ag_nd_opcode_read_nemo_dcr_system_local_prime_ts(AgNdDevice *p_device, AgNdMsgNemoDcrLocalPrimeTs *p_msg);
AgResult ag_nd_opcode_read_config_nemo_dcr_system_local_sample_period(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSamplePeriod *p_msg);
AgResult ag_nd_opcode_write_config_nemo_dcr_system_local_sample_period(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSamplePeriod *p_msg);

AgResult ag_nd_opcode_read_config_nemo_dcr_system_tso_mode(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSystemTsoMode *p_msg);
AgResult ag_nd_opcode_write_config_nemo_dcr_system_tso_mode(AgNdDevice *p_device, AgNdMsgConfigNemoDcrSystemTsoMode *p_msg);


#ifdef __cplusplus
}
#endif


#endif /* __ND_NEMO_DIFF_H__ */



