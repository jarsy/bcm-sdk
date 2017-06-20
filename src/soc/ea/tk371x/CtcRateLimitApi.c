/*
 * $Id: CtcRateLimitApi.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcRateLimitApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkOsAlloc.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/CtcOam.h>
#include <soc/ea/tk371x/OamUtilsCtc.h>
#include <soc/ea/tk371x/CtcRateLimitApi.h>
#include <soc/ea/tk371x/TkDebug.h>

int32           
CtcEthPortSetUSPolicings
    (uint8 pathId, uint8 linkId, uint8 port, 
    EthPortUsPolicingPa * rateLimitPara) 
{
    uint8 *buff;
    CtcEthPortUSPolicingPa *pCtcPara;
    uint8 len = 1;

    if ((linkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == rateLimitPara)
        ) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pCtcPara = (CtcEthPortUSPolicingPa *) buff;
    if (rateLimitPara->op == TRUE || rateLimitPara->op == FALSE) {
        pCtcPara->operation = rateLimitPara->op;
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (rateLimitPara->op == TRUE) {
        rateLimitPara->cbs = soc_htonl(rateLimitPara->cbs);
        rateLimitPara->cir = soc_htonl(rateLimitPara->cir);
        rateLimitPara->ebs = soc_htonl(rateLimitPara->ebs);
        sal_memcpy(pCtcPara->cir, &((uint8 *) (&rateLimitPara->cir))[1],
                   3);
        sal_memcpy(pCtcPara->cbs, &((uint8 *) (&rateLimitPara->cbs))[1],
                   3);
        sal_memcpy(pCtcPara->ebs, &((uint8 *) (&rateLimitPara->ebs))[1],
                   3);
        len = sizeof(CtcEthPortUSPolicingPa);
    }

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, linkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrEthPortPolice,
                        buff, len)) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    }
    TkOamMemPut(pathId,(void *) buff);
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int32           
CtcEthPortGetUSPolicings(uint8 pathId,
     uint8 linkId, uint8 port, EthPortUsPolicingPa * rateLimitPara) 
{
    uint8          *buff;
    uint32          len;
    CtcEthPortUSPolicingPa *pCtcPara;

    if ((linkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == rateLimitPara)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (OK != CtcExtOamObjGet(pathId,
                              linkId,
                              OamCtcBranchObjInst,
                              OamCtcContextPort,
                              port,
                              OamCtcBranchExtAttribute,
                              OamCtcAttrEthPortPolice, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pCtcPara = (CtcEthPortUSPolicingPa *) buff;

    sal_memset(rateLimitPara, 0x0, sizeof(EthPortUsPolicingPa));

    rateLimitPara->op = pCtcPara->operation;
    if (rateLimitPara->op == TRUE) {
        sal_memcpy(&((uint8 *) (&rateLimitPara->cir))[1], pCtcPara->cir,
                   3);
        sal_memcpy(&((uint8 *) (&rateLimitPara->cbs))[1], pCtcPara->cbs,
                   3);
        sal_memcpy(&((uint8 *) (&rateLimitPara->ebs))[1], pCtcPara->ebs,
                   3);
        rateLimitPara->cbs = soc_ntohl(rateLimitPara->cbs);
        rateLimitPara->cir = soc_ntohl(rateLimitPara->cir);
        rateLimitPara->ebs = soc_ntohl(rateLimitPara->ebs);
    }

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

int32           
CtcEthPortSetDSRateLimiting(uint8 pathId,uint8 linkId, uint8 port, 
    EthPortDSRateLimitingPa * rateLimitPara) 
{
    uint8          *buff;
    CtcEthPortDSRateLimitingPa *pCtcPara;
    uint8 len = 1;

    if ((linkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == rateLimitPara)
        ) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pCtcPara = (CtcEthPortDSRateLimitingPa *) buff;
    if (rateLimitPara->op == TRUE || rateLimitPara->op == FALSE) {
        pCtcPara->operation = rateLimitPara->op;
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (rateLimitPara->op == TRUE) {
        rateLimitPara->cir = soc_htonl(rateLimitPara->cir);
        rateLimitPara->pir = soc_htonl(rateLimitPara->pir);
        sal_memcpy(pCtcPara->cir, &((uint8 *) (&rateLimitPara->cir))[1],
                   3);
        sal_memcpy(pCtcPara->pir, &((uint8 *) (&rateLimitPara->pir))[1],
                   3);
        len = sizeof(CtcEthPortDSRateLimitingPa);
    }

    /* coverity[identical_branches:FALSE] */
    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, linkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrEthPortDsRateLimit,
                        buff, len)) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    }
    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

int32           
CtcEthPortGetDSRateLimiting(uint8 pathId, uint8 linkId, uint8 port, 
    EthPortDSRateLimitingPa * rateLimitPara) 
{
    uint8          *buff;
    uint32          len;
    CtcEthPortDSRateLimitingPa *pCtcPara;

    if ((linkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == rateLimitPara)
        ) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (OK != CtcExtOamObjGet(pathId,
                              linkId,
                              OamCtcBranchObjInst,
                              OamCtcContextPort,
                              port,
                              OamCtcBranchExtAttribute,
                              OamCtcAttrEthPortDsRateLimit, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pCtcPara = (CtcEthPortDSRateLimitingPa *) buff;
    sal_memset(rateLimitPara, 0x0, sizeof(EthPortDSRateLimitingPa));

    rateLimitPara->op = pCtcPara->operation;
    if (rateLimitPara->op == TRUE) {
        sal_memcpy(&((uint8 *) (&rateLimitPara->cir))[1], pCtcPara->cir,
                   3);
        sal_memcpy(&((uint8 *) (&rateLimitPara->pir))[1], pCtcPara->pir,
                   3);
        rateLimitPara->cir = soc_ntohl(rateLimitPara->cir);
        rateLimitPara->pir = soc_ntohl(rateLimitPara->pir);
    }

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}
