/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_DIAG_H__
#define __NEMO_DIAG_H__

#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_diag(AgNdDevice *p_device, AgNdMsgDiag *p_msg);
AgResult ag_nd_opcode_write_diag(AgNdDevice *p_device, AgNdMsgDiag *p_msg);

AgResult ag_nd_opcode_read_chip_info(AgNdDevice *p_device, AgNdMsgChipInfo *p_msg);

#ifdef __cplusplus
}
#endif

#endif /* __NEMO_DIAG_H__ */

