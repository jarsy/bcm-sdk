/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_HW_TEST_H__
#define __NEMO_HW_TEST_H__

#include "nd_platform.h"
#include "pub/nd_hw.h"
#include "nd_registers.h"
#include "pub/nd_api.h"
         
#ifdef __cplusplus
extern "C"
{
#endif


AgResult ag_nd_opcode_read_hw_test(AgNdDevice *p_device, AgNdMsgHwTest *p_msg);


#ifdef __cplusplus
}
#endif


#endif /* __NEMO_HW_TEST_H__ */

