/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_PM_H__
#define __NEMO_PM_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_pm_global(AgNdDevice *p_device, AgNdMsgPmGlobal *p_msg);
AgResult ag_nd_opcode_read_config_channel_pme(AgNdDevice *p_device, AgNdMsgConfigChannelPme *p_msg);
AgResult ag_nd_opcode_write_config_channel_pme(AgNdDevice *p_device, AgNdMsgConfigChannelPme *p_msg);

AgResult ag_nd_opcode_read_channel_pme_missing_status(AgNdDevice *p_device,AgNdMsgChannelPmeMessingStatus *p_msg);

/*ORI*/
/*to complete gs(pme)of session */


#ifdef __cplusplus
}
#endif

#endif /* __NEMO_PM_H__ */

