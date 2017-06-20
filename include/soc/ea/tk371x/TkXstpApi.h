/*
 * $Id: TkXstpApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkXstpApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkStpApi_H
#define _SOC_EA_TkStpApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>

typedef struct {
    uint8   bridgeMode;
    uint16  holdTime;
    uint16  fwdDelay;
    uint16  maxAge;
    uint16  priority;
} TagRstpBridgeCfg;

typedef struct {
    uint8   priority;
    uint32  pathCost;
} TagRstpPortCfg;


uint8   TkExtOamSetRstpBridge (uint8 pathId, uint8 linkId,
                TagRstpBridgeCfg * cfg);

uint8   TkExtOamGetRstpBridge (uint8 pathId, uint8 linkId,
                TagRstpBridgeCfg * cfg);

uint8   TkExtOamSetRstpPort (uint8 pathId, uint8 linkId, uint8 port,
                TagRstpPortCfg * cfg);

uint8   TkExtOamGetRstpPort (uint8 pathId, uint8 linkId, uint8 port,
                TagRstpPortCfg * cfg);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkStpApi_H */
