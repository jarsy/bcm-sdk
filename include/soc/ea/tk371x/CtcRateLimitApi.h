/*
 * $Id: CtcRateLimitApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcRateLimitApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_CtcRateLimitApi_H
#define _SOC_EA_CtcRateLimitApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>

typedef struct {
    uint8   op;
    uint32  cir;
    uint32  cbs;
    uint32  ebs;
} EthPortUsPolicingPa;

typedef struct {
    uint8   op;
    uint32  cir;
    uint32  pir;
} EthPortDSRateLimitingPa;

typedef struct {
    uint8   operation;
    uint8   cir[3];
    uint8   cbs[3];
    uint8   ebs[3];
    uint8   pad[0];
} PACK CtcEthPortUSPolicingPa;

typedef struct {
    uint8   operation;
    uint8   cir[3];
    uint8   pir[3];
    uint8   pad[0];
} PACK CtcEthPortDSRateLimitingPa;

int32   CtcEthPortSetUSPolicings (uint8 pathId, uint8 linkId, uint8 port, 
                EthPortUsPolicingPa * rateLimitPara);

int32   CtcEthPortGetUSPolicings (uint8 pathId, uint8 linkId, uint8 port, 
                EthPortUsPolicingPa * rateLimitPara);

int32   CtcEthPortSetDSRateLimiting (uint8 pathId, uint8 linkId, uint8 port, 
                EthPortDSRateLimitingPa * rateLimitPara);

int32   CtcEthPortGetDSRateLimiting (uint8 pathId, uint8 linkId, uint8 port, 
                EthPortDSRateLimitingPa * rateLimitPara);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_CtcRateLimitApi_H */
