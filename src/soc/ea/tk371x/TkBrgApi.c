/*
 * $Id: TkBrgApi.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkBrgApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h> 
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/TkUtils.h>
#include <soc/ea/tk371x/TkInit.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/TkBrgApi.h>

/*
 * send TK extension OAM message Get maximum size of learned address table 
 * of UNI#x from the ONU 
 */
int
TkExtOamGetDynaMacTabSize(uint8 pathId,
                          uint8 LinkId, uint8 port,
                          uint16 * pDynaMacTabSize)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint16          temp;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == pDynaMacTabSize)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK == TkExtOamObjGet(pathId, LinkId,OamNamePhyName, &index,
        OamBranchAttribute, OamExtAttrDynLearnTblSize,(uint8 *) & temp, 
        &DataLen)) {
        *pDynaMacTabSize = soc_ntohs(temp);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

/*
 * send TK extension OAM message Get maximum size of learned address table 
 * of UNI#x from the ONU 
 */
int
TkExtOamGetDynaMacTabSizeNew(uint8 pathId,
                          uint8 LinkId, uint8 port,
                          int * pDynaMacTabSize)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint16          temp;
    int             rv;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == pDynaMacTabSize)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK == TkExtOamObjGet(pathId, LinkId,OamNamePhyName, &index,
        OamBranchAttribute, OamExtAttrDynLearnTblSize,(uint8 *) & temp, 
        &DataLen)) {
        temp = soc_ntohs(temp);

        rv = OK;
        if(temp == MAX_ARL_TAB_SIZE){
            *pDynaMacTabSize = BRG_ARL_LIMIT_NONE;
        }else if(temp > MAX_ARL_TAB_SIZE){
            *pDynaMacTabSize = 0;
            rv = ERROR;
        }else{
            *pDynaMacTabSize = temp;
        }
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        rv = ERROR;
    }
    return rv;
}

/*
 * send TK extension OAM message Set maximum size of learned address table 
 * of UNI#x to the ONU 
 */
int
TkExtOamSetDynaMacTabSize(uint8 pathId,
                          uint8 LinkId, uint8 port, uint16 DynaMacTabSize)
{
    OamObjIndex     index;
    uint16          temp = soc_htons(DynaMacTabSize);
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
       || (port < SDK_PORT_VEC_BASE)
       || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    index.portId = port;

    
    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, OamBranchAttribute,
             OamExtAttrDynLearnTblSize, (uint8 *) & temp, sizeof(uint16));

    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;
}

/*
 * send TK extension OAM message Set maximum size of learned address table 
 * of UNI#x to the ONU 
 */
int
TkExtOamSetDynaMacTabSizeNew(uint8 pathId,uint8 LinkId, uint8 port, 
    int DynaMacTabSize)
{
    OamObjIndex     index;
    uint16          temp;
    uint8           ret;
    uint16          macTabSize;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
       || (port < SDK_PORT_VEC_BASE)
       || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    if(DynaMacTabSize > MAX_ARL_TAB_SIZE){
        return ERROR;
    }

    index.portId = port;
    if (BRG_ARL_LIMIT_NONE == DynaMacTabSize){
        macTabSize = MAX_ARL_TAB_SIZE;
    }else{
        macTabSize = DynaMacTabSize&0xffff;
    }
    
    temp = soc_htons(macTabSize);
     
    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, OamBranchAttribute,
             OamExtAttrDynLearnTblSize, (uint8 *) & temp, sizeof(uint16));

    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;
}



/*
 * send TK extension OAM message Get learning aging timer of UNI#x from
 * the ONU 
 */
int
TkExtOamGetDynaMacTabAge(uint8 pathId,
                         uint8 LinkId, uint8 port, uint16 * pDynaMacTabAge)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint16          temp;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == pDynaMacTabAge)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK == TkExtOamObjGet(pathId, LinkId, OamNamePhyName, &index,
        OamBranchAttribute,OamExtAttrDynLearnAgeLimit,(uint8 *) & temp, 
        &DataLen)) {
        *pDynaMacTabAge = soc_ntohs(temp);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Set learning aging timer of UNI#x to the
 * ONU 
 */
int
TkExtOamSetDynaMacTabAge(uint8 pathId,
                         uint8 LinkId, uint8 port, uint16 DynaMacTabAge)
{
    OamObjIndex     index;
    uint16          temp = soc_htons(DynaMacTabAge);
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
       || (port < SDK_PORT_VEC_BASE)
       || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    index.portId = port;

    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, OamBranchAttribute,
             OamExtAttrDynLearnAgeLimit, (uint8 *) & temp,sizeof(uint16));

    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;
}


/*
 * send TK extension OAM message Get dynamically learned MAC address of
 * UNI#x from the ONU 
 */
int
TkExtOamGetDynaMacEntries(uint8 pathId,
                          uint8 LinkId,
                          uint8 port,
                          uint8 * pNumEntries, MacAddr * pDynaMacEntries)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT) 
        || (NULL == pNumEntries)
        || (NULL == pDynaMacEntries)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK ==
        TkExtOamObjGetMulti(pathId, LinkId,
                            OamNamePhyName,
                            &index,
                            OamBranchAttribute,
                            OamExtAttrDynMacTbl, (uint8 *) rxTmpBuf,
                            &DataLen)) {
        uint8           curMacs = 0;
        OamVarContainer *oamExtVar;
        OamRuleSpec    *oamFilter;

        oamExtVar = (OamVarContainer *) rxTmpBuf;
        while (oamExtVar->branch != OamBranchTermination) {
            if ((oamExtVar->branch == OamBranchAttribute)
                && (oamExtVar->leaf == soc_ntohs(OamExtAttrDynMacTbl))) {
                if ((oamExtVar->length != 0x80)
                    && (oamExtVar->length != 0)) {
                    oamFilter = (OamRuleSpec *) (oamExtVar->value);
                    bcopy((void *) (oamFilter->cond->value),
                          pDynaMacEntries[curMacs++].u8, sizeof(MacAddr));
                }
            }

            oamExtVar = NextCont(oamExtVar);
        }

        *pNumEntries = curMacs;
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

/*
 * send TK extension OAM message Set clear dynamically learned MAC address 
 * of UNI#x to the ONU 
 */
int
TkExtOamSetClrDynaMacTable(uint8 pathId, uint8 LinkId, uint8 port)
{
    OamObjIndex     index;
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE)
        || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    index.portId = port;

    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,
             &index, OamBranchAction, OamExtActClearDynLearnTbl, NULL, 0);

    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;

}

/*
 * Send TK externsion OAM message Get StaticMacTbl 
 */
int
TkExtOamGetStaticMacEntries(uint8 pathId,
                            uint8 LinkId,
                            uint8 port,
                            uint8 * pNumEntries,
                            MacAddr * pStaticMacEntries)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT) 
        || (NULL == pNumEntries)
        || (NULL == pStaticMacEntries)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK ==
        TkExtOamObjGetMulti(pathId, LinkId,
                            OamNamePhyName,
                            &index,
                            OamBranchAttribute,
                            OamExtAttrStaticMacTbl,
                            (uint8 *) rxTmpBuf, &DataLen)) {
        uint8           curMacs = 0;
        OamVarContainer *oamExtVar;
        OamRuleSpec    *oamFilter;

        oamExtVar = (OamVarContainer *) rxTmpBuf;
        while (oamExtVar->branch != OamBranchTermination) {
            if ((oamExtVar->branch == OamBranchAttribute)
                && (oamExtVar->leaf == soc_ntohs(OamExtAttrStaticMacTbl))) {
                if ((oamExtVar->length != 0x80)
                    && (oamExtVar->length != 0)) {
                    oamFilter = (OamRuleSpec *) (oamExtVar->value);
                    bcopy((void *) (oamFilter->cond->value),
                          pStaticMacEntries[curMacs++].u8,
                          sizeof(MacAddr));
                }
            }

            oamExtVar = NextCont(oamExtVar);
        }

        *pNumEntries = curMacs;
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

/*
 * Send TK externsion OAM message Add StaticMacEntry 
 */
int
TkExtOamAddStaticMacEntry(uint8 pathId,
                          uint8 LinkId, uint8 port, MacAddr * pMacEntry)
{
    OamObjIndex     index;
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        ||(port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == pMacEntry)) {
        return (ERROR);
    }

    index.portId = port;

    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, OamBranchAction,
             OamExtActAddStaticEntry, (uint8 *) pMacEntry,sizeof(MacAddr));

    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;    
}

/*
 * Send TK externsion OAM message Delete StaticMacEntry 
 */
int
TkExtOamDelStaticMacEntry(uint8 pathId,
                          uint8 LinkId, uint8 port, MacAddr * pMacEntry)
{
    OamObjIndex     index;
    uint8           ret; 

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        ||(port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == pMacEntry)) {
        return (ERROR);
    }

    index.portId = port;

    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, OamBranchAction,
             OamExtActDelStaticEntry, (uint8 *) pMacEntry,sizeof(MacAddr));
    
    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;    
}

int
TkExtOamFlushMacTable(uint8 pathId, uint8 LinkId, uint8 port, uint8 type)
{
    OamObjIndex     index;
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE)
        || (port > SDK_MAX_NUM_OF_PORT)
        || (type > FlushStaticMac)) {
        return (ERROR);
    }

    index.portId = port;

    if (FlushAllMac == type || FlushStaticMac == type) {
        ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, 
            OamBranchAttribute,OamExtAttrMacFlush, (uint8 *) & type, 
            sizeof(uint8));
    } else {
        ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, 
            OamBranchAction, OamExtActClearDynLearnTbl, NULL,0);
    }

    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;  
}

int
TkExtOamFlushMacTableNew(uint8 pathId, uint8 LinkId, uint8 port, uint8 type)
{
    OamObjIndex     index;
    uint8           ret;
    uint8           count;
    MacAddr         entry[255];
    int             i;
    int             rv;
    
    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE)
        || (port > SDK_MAX_NUM_OF_PORT)
        || (type > FlushStaticMac)) {
        return (ERROR);
    }

    index.portId = port;

    switch(type){
        case FlushDynamicMac:
            ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName, &index, 
                OamBranchAction, OamExtActClearDynLearnTbl, NULL, 0);
            rv = OK;
            break;
        case FlushStaticMac:
            ret = TkExtOamGetStaticMacEntries(pathId, LinkId, port, &count, 
                entry);
            if(OK != ret)return OK;
            for(i = 0; i < count; i++){
                TkExtOamDelStaticMacEntry(pathId, LinkId, port, &entry[i]);    
            }
            rv = OK;
            break;
        case FlushAllMac:
            ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName, &index,
                OamBranchAction, OamExtActClearDynLearnTbl, NULL, 0);
            if(OK != ret)rv = OK;
            ret = TkExtOamGetStaticMacEntries(pathId, LinkId, port, &count, 
                entry);
            if(OK != ret)return OK;
            for(i = 0; i < count; i++){
                TkExtOamDelStaticMacEntry(pathId, LinkId, port, &entry[i]);    
            } 
            rv = OK;
            break;
        default:
            rv = ERROR;
            break;
    }
    return rv;  
}

int
TkExtOamSetMacLearning(uint8 pathId, uint8 LinkId, uint8 port,
                       uint8 status)
{
    OamObjIndex     index;
    uint8           MacLearningStatus = status;
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE)
        || (port > SDK_MAX_NUM_OF_PORT)
        || (status > ArlSftLearning)) {
        return (ERROR);
    }

    index.portId = port;

    if (ArlDisLearning == status) {
        MacLearningStatus = FALSE;
    } else {
        MacLearningStatus = TRUE;
    }

    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, OamBranchAttribute,
             OamExtAttrMacLearning,(uint8 *) & MacLearningStatus, sizeof(uint8));

    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;    
}

int
TkExtOamSetForwardMode(uint8 pathId,
                       uint8 LinkId, uint8 port, ForwardingMode mode)
{
    OamObjIndex     index;
    uint8           enableStatus = 0;
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (mode < ForwardFloodUnknown)
        || (mode > ForwardPassUntilFull)) {
        return (ERROR);
    }

    index.portId = port;

    enableStatus = mode;

    ret = TkExtOamObjSet(pathId, LinkId, OamNamePhyName,&index, OamBranchAttribute,
             OamExtAttrDynLearningMode,(uint8 *) & enableStatus, sizeof(uint8));
    if(ret !=  OamVarErrNoError){
        return ERROR;
    }

    return OK;     
}


int
TkExtOamSetAutoNeg(uint8 pathId, uint8 LinkId,
                   uint8 port,
                   OamAutoNegAdminState
                   AutoEnable, uint16 speed, OamMacDuplexStatus mode)
{
    OamObjIndex     index;
    OamAutoNegCapability *pData;
    uint8           len = 0;
    uint8           ret;
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);
    pData = (OamAutoNegCapability *) rxTmpBuf;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE)
        || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    index.portId = port;
    if (AutoEnable) {
        if (OamVarErrNoError ==
            TkExtOamObjSet(pathId, LinkId,
                           OamNamePhyName,
                           &index,
                           OamBranchAction,
                           OamActAutoAdminCtrl,
                           (uint8 *) & AutoEnable, sizeof(Bool))) {
            if (100 == speed) {
                *pData++ = soc_htons(OamAutoCap100TX);
                len += sizeof(OamAutoNegCapability);
            } else if (10 == speed) {
                *pData++ = soc_htons(OamAutoCap10T);
                len += sizeof(OamAutoNegCapability);
            }
            if (OamMacDuplexFull == mode) {
                *pData = soc_htons(OamAutoCap10TFD);
                len += sizeof(OamAutoNegCapability);
            } else {
                *pData = soc_htons(OamAutoCap10T);
                len += sizeof(OamAutoNegCapability);
            }
            ret = TkExtOamObjSet(pathId, LinkId,OamNamePhyName, &index,
                     OamBranchAttribute,OamAttrAutoNegAdTech, 
                     (uint8 *) rxTmpBuf, len);
            if(OamVarErrNoError == ret){
                return OK;
            }
        } else {
            TkDbgInfoTrace(TkDbgErrorEnable,
                           ("Errors in Enabling AutoAdminCtrl!\n"));
            return ERROR;
        }
    }
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int
TkExtOamGetAutoNeg(uint8 pathId, uint8 linkId,
                   uint8 port,
                   OamAutoNegAdminState *
                   AutoEnable, uint16 * speed, OamMacDuplexStatus * mode)
{
    BufInfo         bufInfo;
    Bool            ok;
    uint32          rxLen;
    uint8           ret;
    tGenOamVar      var;
    uint8           buf[4];
    uint8           rSpeed;
    uint8           rMode;
    uint8           rAutoStatus;
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);

    if ((linkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) ||
           (port > SDK_MAX_NUM_OF_PORT) ||
           (NULL == AutoEnable) || (NULL == speed)
           || (NULL == mode)
        ) {
        return (ERROR);
    }

    if (TkOamMsgPrepare(pathId,&bufInfo, OamExtOpVarRequest) != OK) {
        return RcNoResource;
    }

    Tk2BufU16(buf, port);
    ok = AddOamTlv(&bufInfo, OamBranchNameBinding, OamNamePhyName, 2, buf);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrAutoNegAdminState);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrPhyType);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacDuplexStatus);

    ret = TxOamDeliver(pathId, linkId, &bufInfo, (uint8 *) rxTmpBuf, &rxLen);
    if (ret != RcOk)
        return ret;

    if (TkDbgLevelIsSet(TkDbgLogTraceEnable))
        BufDump(NULL, rxTmpBuf, rxLen);

    var.pValue = NULL;

    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrAutoNegAdminState,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    rAutoStatus = TkMakeU8(var.pValue);

    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamAttrPhyType,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    rSpeed = TkMakeU8(var.pValue);

    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacDuplexStatus,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    rMode = TkMakeU8(var.pValue);

    *AutoEnable = rAutoStatus;
    *mode = rMode;

    switch (rSpeed) {
    case OamPhyType100T4:
    case OamPhyType100X:
    case OamPhyType100T2:
        *speed = 100;
        break;
    case OamPhyType10:
        *speed = 10;
        break;
    case OamPhyType1000X:
    case OamPhyType1000T:
        *speed = 1000;
        break;
    }
    return ret;
}

int
TkExtOamSetMtu(uint8 pathId, uint8 LinkId, uint8 port, uint16 maxFrameSize)
{
    OamObjIndex     index;
    uint16          MtuSize;

    if ((LinkId > SDK_MAX_NUM_OF_LINK)
        || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    index.portId = port;
    MtuSize = soc_htons(maxFrameSize);

    if (OamVarErrNoError ==
        TkExtOamObjSet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrMtuSize,
                       (uint8 *) & MtuSize, sizeof(uint16))) {
        return OK;
    }


    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int
TkExtOamGetMtu(uint8 pathId, uint8 linkId, uint8 port,
               uint16 * maxFrameSize)
{
    OamObjIndex     index;
    uint16          MtuSize = 0;
    uint32          len;

    if ((linkId > SDK_MAX_NUM_OF_LINK) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == maxFrameSize)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK ==
        TkExtOamObjGet(pathId, linkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrMtuSize, (uint8 *) & MtuSize, &len)) {
        *maxFrameSize = soc_ntohs(MtuSize);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

int
TkExtOamSetFloodUnknown(uint8 pathId, uint8 LinkId, Bool status)
{
    uint8           FloodUnknownStatus = status;
    int             ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK)
        || ((FALSE != status) && (TRUE != status))) {
        return (OamVarErrActBadParameters);
    }

    ret = TkExtOamSet(pathId, LinkId,OamBranchAttribute,OamExtAttrFloodUnknown,
                    &FloodUnknownStatus, sizeof(Bool));

    if(OamVarErrNoError == ret){
        return OK;
    }

    return ret;
}

int
TkExtOamGetFloodUnknown(uint8 pathId, uint8 linkId, Bool * status)
{
    uint8           FloodUnknownStatus;
    uint32          len;
    int             ret;

    if ((linkId > SDK_MAX_NUM_OF_LINK)
         || (NULL == status)) {
        return (ERROR);
    }

    ret =
        TkExtOamGet(pathId, linkId,
                    OamBranchAttribute,
                    OamExtAttrFloodUnknown, &FloodUnknownStatus, &len);
    if (ret != OK) {
        return OamVarErrUnknow;
    } else {
        *status = FloodUnknownStatus;
        return (OK);
    }
}

int
TkExtOamSetMacLearnSwitch(uint8 pathId, uint8 LinkId,
                          ArlLearnStatus status)
{
    uint8           MacLearningStatus = status;
    uint8           ret;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (status < ArlHwLearning)
        || (status > ArlSftLearning)) {
        return (ERROR);
    }

    MacLearningStatus = status;

    ret = TkExtOamSet(pathId, LinkId,OamBranchAttribute,
        OamExtAttrMacLearningSwitch,&MacLearningStatus, sizeof(uint8));

    if(OamVarErrNoError == ret){
        return OK;
    }

    return ret;
}

int
TkExtOamGetEthLinkState(uint8 pathId,
                        uint8 linkId, uint8 port, uint8 * linkStatus)
{
    OamObjIndex     index;
    uint8           status;
    uint32          len;

    if ((linkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == linkStatus)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK ==
        TkExtOamObjGet(pathId, linkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamAttrMauMediaAvail, (uint8 *) & status, &len)) {
        if (OamMauMediaAvailAvailable == status) {
            *linkStatus = TRUE;
        } else {
            *linkStatus = FALSE;
        }
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

int
TkExtOamSetPhyAdminState(uint8 pathId,
                         uint8 LinkId, uint8 port, uint8 adminState)
{
    OamObjIndex     index;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT) 
        || ((adminState != OamTrue) && (adminState != OamFalse))) {
        return (ERROR);
    }

    index.portId = port;

    if (OamVarErrNoError ==
        TkExtOamObjSet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamAttrMacEnableStatus,
                       (uint8 *) & adminState, sizeof(uint8))) {
        return OK;
    }
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int
TkExtOamGetPhyAdminState(uint8 pathId,
                         uint8 linkId, uint8 port, uint8 * adminState)
{
    OamObjIndex     index;
    uint32          len;

    if ((linkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE) 
        || (port > SDK_MAX_NUM_OF_PORT)
        || (NULL == adminState)) {
        return (ERROR);
    }

    index.portId = port;

    if (OK ==
        TkExtOamObjGet(pathId, linkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamAttrMacEnableStatus, (uint8 *) adminState,
                       &len)) {
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

int
TkExtOamClrAllFilterTbl(uint8 pathId, uint8 LinkId, uint8 port)
{
    OamObjIndex     index;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE)
        || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    index.portId = port;

    if (OamVarErrNoError ==
        TkExtOamObjSet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAction, OamExtActClrMacFilterTbl, NULL,
                       0)) {
        return OK;
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

int 
TkExtOamLlidSetLoopback(uint8 pathId, uint8 linkId, uint8 llid,OamLoopbackLoc loc)
{
    uint8 *pMsgBuf = NULL;      
    uint16 flags = 0x0050;  
    uint32 size;    
    int ret = ERROR; 
    OamOuiVendorExt *tx;
    uint16 ethHeadLen;
    uint8 loc_state;

    if (linkId > SDK_MAX_NUM_OF_LINK){
        return ERROR;
    }
    
    if(linkId  != llid){
        linkId = llid;
    }

    switch(loc){
        case OamLoopLoc8023ah:
            loc_state = LoopbackEnable;
            break;
        case OamLoopLocNone:
            loc_state = LoopbackDisable;
            break;
        default:
            return ERROR;
            break;
    }

    AttachOamFlagNoPass(llid, &flags);        

    pMsgBuf = (uint8 *)TkOamMemGet(pathId);   /* get memory */    
    if (NULL == pMsgBuf){       
        TkDbgTrace(TkDbgErrorEnable);        
        return (RcNoResource);        
    } 

    ethHeadLen = sizeof(EthernetFrame);
    tx = (OamOuiVendorExt *) (pMsgBuf + ethHeadLen);

    tx->common.subtype = OamSlowProtocolSubtype;
    tx->common.flags = soc_htons(flags);
    tx->common.opcode = OamOpLoopback;
  
    ((uint8 *)(&(tx->common.opcode)))[1] = loc_state;

    size = ethHeadLen + sizeof(OamMsg) + sizeof(uint8);

    /* coverity[value_overwrite] */
    size = EthMinFrameSize;

    ret = TkOamNoResRequest(pathId, linkId, (OamFrame *)pMsgBuf, size);

    if (OK != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        ret = ERROR;
    } else {
        ret = OK;
    }
    TkOamMemPut(pathId,pMsgBuf);
    
    return ret;
 }


int
TkExtOamPortSetLoopback(uint8 pathId, uint8 LinkId, uint8 port, OamLoopbackLoc loc)
{
    int rv;
    OamObjIndex index;
    uint8 loopback_state[10];
    uint8 ret;
    
    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (port < SDK_PORT_VEC_BASE)
        || (port > SDK_MAX_NUM_OF_PORT)) {
        return (ERROR);
    }

    if(loc >= OamLoopLocNumLocs){
        return ERROR;
    }
    
    index.portId = port;
    loopback_state[0] = loc;

    switch(loc){
        case OamLoopLocNone:
            ret = TkExtOamObjSet(pathId,LinkId,OamNamePhyName, &index, 
                OamBranchAction,OamExtActLoopbackDisable, NULL, 0);
            break;
        case OamLoopLocUniPhy:
        case OamLoopLocUniMac:
            ret = TkExtOamObjSet(pathId,LinkId,OamNamePhyName, &index, 
                OamBranchAction,OamExtActLoopbackEnable, loopback_state, sizeof(uint8));
            break;
        default:
            ret = OamVarErrActBadParameters;
            break;
                
    }

    if(ret <= OamVarErrNoError){
        rv = OK;
    }else{
        rv = ERROR;
    }

    return rv;
}

