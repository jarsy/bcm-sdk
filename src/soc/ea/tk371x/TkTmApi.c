/*
 * $Id: TkTmApi.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkTmApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkTmApi.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOamMem.h>

/*
 * send TK extension OAM message Get queue configuration from the ONU 
 */
int
TkExtOamGetQueueCfg(uint8 pathId, uint8 LinkId, uint8 * pQueueCfg)
{
    uint32          DataLen;

    if ((LinkId > 7) || (NULL == pQueueCfg)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (OK ==
        TkExtOamGet(pathId, LinkId,
                    OamBranchAction,
                    OamExtActGetQueueConfig, (uint8 *) pQueueCfg,
                    &DataLen)) {
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Set enable user traffic to the ONU 
 */
int
TkExtOamSetEnaUserTraffic(uint8 pathId, uint8 LinkId)
{
    int rv;
    uint8 ret;
    
    if (LinkId > 7){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret = TkExtOamSet(pathId, LinkId, OamBranchAction, 
        OamExtActOnuEnableUserTraffic, NULL, 0);

    if(OamVarErrNoError != ret){
        rv = ERROR;
    }else{
        rv = OK;
    }

    return rv;
}


/*
 * send TK extension OAM message Set disable user traffic to the ONU 
 */
int
TkExtOamSetDisUserTraffic(uint8 pathId, uint8 LinkId)
{
    int rv;
    uint8 ret;
    
    if (LinkId > 7){
        return ERROR;
    }

    ret = TkExtOamSet(pathId, LinkId, OamBranchAction, 
        OamExtActOnuDisableUserTraffic, NULL, 0);

    if(OamVarErrNoError != ret){
        rv = ERROR;
    }else{
        rv = OK;
    }

    return rv;
}

/*
 * send TK extension OAM message Get BcastRateLimit ON/OFF 
 */
int
TkExtOamGetBcastRateLimit(uint8 pathId, uint8 LinkId, uint8 port, uint32 *pkts_cnt)
{
    uint32 dataLen;
    int rv;
    uint32 *pBuff = NULL;
    OamObjIndex index;

    index.portId = port;
    
    if((LinkId >= SDK_MAX_NUM_OF_LINK)||(port > SDK_MAX_NUM_OF_PORT)){
        return (ERROR);
    }
    
    pBuff = (uint32 *) TkOamMemGet(pathId);
    if(NULL == pBuff){
        return ERROR;
    }

    if(OK == TkExtOamObjGet(pathId, LinkId, OamNamePhyName,&index,
        OamBranchAttribute, 
        OamExtAttrBcastRateLimit, 
        (uint8 *)pBuff, &dataLen)){
        *pkts_cnt = soc_ntohl(pBuff[0]);
        
        rv = OK;     
    }else{
        
        rv = ERROR;
    }
    
    TkOamMemPut(pathId, (void *)pBuff);
    
    return rv;
}

/*
 * send TK extension OAM message Set BcastRateLimit ON/OFF
 */
int
TkExtOamSetBcastRateLimit(uint8 pathId, uint8 LinkId, uint8 port, uint32 pkts_cnt)
{
    OamObjIndex index;
    uint32 count;

    if((LinkId > SDK_MAX_NUM_OF_LINK)||(port > SDK_MAX_NUM_OF_PORT)){
        return (ERROR);
    }

    index.portId = port;
    count = soc_htonl(pkts_cnt);

    if(OamVarErrNoError == TkExtOamObjSet(pathId, LinkId,
        OamNamePhyName, &index, OamBranchAttribute, 
        OamExtAttrBcastRateLimit, (uint8 *)&count, 
        sizeof(uint32))){
        return (OK);    
    }
    
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

