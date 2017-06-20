/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_HW_HL_H__
#define __NEMO_HW_HL_H__

#include "../nd_platform.h"
#include "../nd_list.h"
#include "../nd_mm.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    AG_ND_ARCH_TYPE_NONE            = 0xff,
    AG_ND_ARCH_TYPE_1_PORT_NEMO     = 0x00,
    AG_ND_ARCH_TYPE_2_PORT_NEMO     = 0x04,
    AG_ND_ARCH_TYPE_4_PORT_NEMO     = 0x08,
    AG_ND_ARCH_TYPE_8_PORT_64_NEMO  = 0x0c,
    AG_ND_ARCH_TYPE_8_PORT_256_NEMO = 0x0e,
    AG_ND_ARCH_TYPE_16_PORT_NEMO    = 0x10,
    AG_ND_ARCH_TYPE_T3_NEPTUNE      = 0x14,
    AG_ND_ARCH_TYPE_C3_512_NEPTUNE  = 0x18,
    AG_ND_ARCH_TYPE_C3_2048_NEPTUNE = 0x1a

} AgNdArchType;


#ifdef __cplusplus
}
#endif

#endif /* __NEMO_HW_HL_H__ */

