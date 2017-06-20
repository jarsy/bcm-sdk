/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_INTERRUPTS_H__
#define __NEMO_INTERRUPTS_H__

#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define AG_ND_INTR_TASK_STASK_SIZE  8192
#define AG_ND_INTR_TASK_PRIORITY    10
#define AG_ND_INT_TIMER_PERIOD      100

#define AG_ND_INTR_EVENT_QUIT       0x00000001
#define AG_ND_INTR_EVENT_PROCESS    0x00000002

AgResult ag_nd_interrupts_init(AgNdDevice *p_device);
AgResult ag_nd_interrupts_stop(AgNdDevice *p_device);


#ifdef __cplusplus
}
#endif

#endif  /* __NEMO_INTERRUPTS_H__ */

