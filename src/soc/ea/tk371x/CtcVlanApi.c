/*
 * $Id: CtcVlanApi.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcVlanApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/CtcOam.h>
#include <soc/ea/tk371x/OamUtilsCtc.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/CtcVlanApi.h>
#include <soc/ea/tk371x/TkDebug.h>

int
CtcExtOamSetVlanTransparent(uint8 pathId, uint8 LinkId, uint32 port)
{
    CtcOamVlanEntry vlanEntry;

    if (/* (LinkId < SDK_LINK_VEC_BASE) || */
          (LinkId > SDK_MAX_NUM_OF_LINK) || (port < SDK_PORT_VEC_BASE)
           || (port > SDK_MAX_NUM_OF_PORT)
        ) {
        return (OamVarErrActBadParameters);
    }

    vlanEntry.CtcOamVlanMode = OamCtcVlanTransparent;

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrVlan,
                        (uint8 *) & vlanEntry, sizeof(CtcOamVlanEntry))) {
        return OK;
    }
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int             
CtcExtOamSetVlanTag (uint8 pathId, uint8 LinkId, uint32 port,
     CtcOamVlanTag * pCtcOamVlanTag) 
{
    uint8          *buff;
    CtcOamVlanEntry *pVlanEntry;
    CtcOamVlanTag  *pTagInfo;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == pCtcOamVlanTag)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pVlanEntry = (CtcOamVlanEntry *) buff;
    pTagInfo = (CtcOamVlanTag *) pVlanEntry->CtcOamVlanData;

    pVlanEntry->CtcOamVlanMode = OamCtcVlanTag;
    pTagInfo->tag[0].type = soc_htons(pCtcOamVlanTag->tag[0].type);
    pTagInfo->tag[0].tag = soc_htons(pCtcOamVlanTag->tag[0].tag);

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrVlan, buff, sizeof(CtcOamVlanEntry)
                        + sizeof(EthernetVlanData))) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    }
    TkOamMemPut(pathId,(void *) buff);
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int             
CtcExtOamSetVlanTranslation(uint8 pathId, uint8 LinkId,
     uint32 port,EthernetVlanData defaultVlan,uint32 numOfTranslateEntry,
     CtcVlanTranslatate * pCtcVlanTranslatate) 
{
    uint8          *buff;
    CtcOamVlanEntry *pVlanEntry;
    CtcOamVlanTranslate *pVlanTranslateEntry;
    uint32          num;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK) || (port < SDK_PORT_VEC_BASE)
           || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == pCtcVlanTranslatate)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }
    pVlanEntry = (CtcOamVlanEntry *) buff;
    pVlanTranslateEntry =
        (CtcOamVlanTranslate *) pVlanEntry->CtcOamVlanData;

    pVlanEntry->CtcOamVlanMode = OamCtcVlanTranslation;
    pVlanTranslateEntry->defaultVlan.type = soc_htons(defaultVlan.type);
    pVlanTranslateEntry->defaultVlan.tag = soc_htons(defaultVlan.tag);
    for (num = 0; num < numOfTranslateEntry; num++) {
        pVlanTranslateEntry->vlanTranslateArry[num].fromVid.type =
            soc_htons(pCtcVlanTranslatate[num].fromVid.type);
        pVlanTranslateEntry->vlanTranslateArry[num].fromVid.tag =
            soc_htons(pCtcVlanTranslatate[num].fromVid.tag);
        pVlanTranslateEntry->vlanTranslateArry[num].toVid.type =
            soc_htons(pCtcVlanTranslatate[num].toVid.type);
        pVlanTranslateEntry->vlanTranslateArry[num].toVid.tag =
            soc_htons(pCtcVlanTranslatate[num].toVid.tag);
    }

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrVlan, buff, sizeof(CtcOamVlanEntry)
                        + sizeof(CtcOamVlanTranslate)
                        +
                        sizeof(CtcVlanTranslatate) *
                        numOfTranslateEntry)) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    }
    TkOamMemPut(pathId,(void *) buff);
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int             
CtcExtOamSetVlanTrunk(uint8 pathId, uint8 LinkId, uint32 port,
     EthernetVlanData defaultVlan,
     uint32 numOfTrunkEntry, EthernetVlanData * pCtcVlanTrunkEntry) 
{
    uint8          *buff;
    CtcOamVlanEntry *pVlanEntry;
    CtcOamVlanTrunk *pVlanTrunkEntry;
    uint32          num;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK) || (port < SDK_PORT_VEC_BASE)
           || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == pCtcVlanTrunkEntry)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pVlanEntry = (CtcOamVlanEntry *) buff;
    pVlanTrunkEntry = (CtcOamVlanTrunk *) pVlanEntry->CtcOamVlanData;

    pVlanEntry->CtcOamVlanMode = OamCtcVlanTrunk;
    pVlanTrunkEntry->defaultVlan.type = soc_htons(defaultVlan.type);
    pVlanTrunkEntry->defaultVlan.tag = soc_htons(defaultVlan.tag);
    for (num = 0; num < numOfTrunkEntry; num++) {
        pVlanTrunkEntry->vlanTrunkArry[num].type
            = soc_htons(pCtcVlanTrunkEntry[num].type);
        pVlanTrunkEntry->vlanTrunkArry[num].tag =
            soc_htons(pCtcVlanTrunkEntry[num].tag);
    }

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrVlan, buff, sizeof(CtcOamVlanEntry)
                        + sizeof(CtcOamVlanTrunk)
                        + sizeof(EthernetVlanData)
                        * numOfTrunkEntry)) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    }
    TkOamMemPut(pathId,(void *) buff);
    TkDbgTrace(TkDbgErrorEnable);
    return ERROR;
}

int             
CtcExtOamGetVlan(uint8 pathId, uint8 LinkId, uint32 port,
     CtcVlanEntryInfo * pVlanEntry) 
{
    uint32          len,
                    cnt;
    uint8          *buff;
    CtcOamVlanEntry *pRetVlanEntry = NULL;
    CtcOamVlanTag  *pVlanTagInfo;
    CtcOamVlanTranslate *pVlanTranslateInfo;
    CtcOamVlanTrunk *pVlanTrukInfo;
    int             ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == pVlanEntry)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    if (OK != CtcExtOamObjGet(pathId,
                              LinkId,
                              OamCtcBranchObjInst,
                              OamCtcContextPort,
                              port,
                              OamCtcBranchExtAttribute,
                              OamCtcAttrVlan, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pRetVlanEntry = (CtcOamVlanEntry *) buff;
    switch (pRetVlanEntry->CtcOamVlanMode) {
    case OamCtcVlanTransparent:
        pVlanEntry->mode = OamCtcVlanTransparent;
        break;
    case OamCtcVlanTag:
        pVlanTagInfo = (CtcOamVlanTag *) pRetVlanEntry->CtcOamVlanData;
        pVlanEntry->mode = OamCtcVlanTag;
        pVlanEntry->CtcVlanArry.tagInfo.vlanTag.type =
            soc_ntohs(pVlanTagInfo->tag[0].type);
        pVlanEntry->CtcVlanArry.tagInfo.vlanTag.tag =
            soc_ntohs(pVlanTagInfo->tag[0].tag);
        break;
    case OamCtcVlanTranslation:
        pVlanTranslateInfo = (CtcOamVlanTranslate *)
            pRetVlanEntry->CtcOamVlanData;
        pVlanEntry->mode = OamCtcVlanTranslation;
        pVlanEntry->CtcVlanArry.translateInfo.numOfEntry =
            (len - 1 - 4) / 8;
        pVlanEntry->CtcVlanArry.translateInfo.defaultVlanInfo.type =
            soc_ntohs(pVlanTranslateInfo->defaultVlan.type);
        pVlanEntry->CtcVlanArry.translateInfo.defaultVlanInfo.tag =
            soc_ntohs(pVlanTranslateInfo->defaultVlan.tag);
        for (cnt = 0; cnt < (len - 1 - 4) / 8; cnt++) {
            pVlanEntry->CtcVlanArry.translateInfo.vlanTranslateArry
                [cnt].fromVid.type =
                soc_ntohs(pVlanTranslateInfo->vlanTranslateArry[cnt].
                          fromVid.type);
            pVlanEntry->CtcVlanArry.translateInfo.
                vlanTranslateArry[cnt].fromVid.tag =
                soc_ntohs(pVlanTranslateInfo->vlanTranslateArry[cnt].
                          fromVid.tag);
            pVlanEntry->CtcVlanArry.translateInfo.vlanTranslateArry[cnt].
                toVid.type =
                soc_ntohs(pVlanTranslateInfo->vlanTranslateArry[cnt].
                          toVid.type);
            pVlanEntry->CtcVlanArry.translateInfo.vlanTranslateArry[cnt].
                toVid.tag =
                soc_ntohs(pVlanTranslateInfo->vlanTranslateArry[cnt].
                          toVid.tag);
        }
        break;
    case OamCtcVlanN21Translation:
        pVlanEntry->mode = OamCtcVlanN21Translation;
        ret = ERROR;
        break;
    case OamCtcVlanTrunk:
    case OamZteVlanTrunk:
        pVlanTrukInfo = (CtcOamVlanTrunk *) pRetVlanEntry->CtcOamVlanData;
        pVlanEntry->mode = pRetVlanEntry->CtcOamVlanMode;
        pVlanEntry->CtcVlanArry.trunkInfo.numOfEntry = (len - 1 - 4) / 4;
        pVlanEntry->CtcVlanArry.trunkInfo.defaultVlanInfo.type =
            soc_ntohs(pVlanTrukInfo->defaultVlan.type);
        pVlanEntry->CtcVlanArry.trunkInfo.defaultVlanInfo.tag =
            soc_ntohs(pVlanTrukInfo->defaultVlan.tag);
        for (cnt = 0; cnt < (len - 1 - 4) / 4; cnt++) {
            pVlanEntry->CtcVlanArry.trunkInfo.vlanTrunkArry[cnt].type =
                soc_ntohs(pVlanTrukInfo->vlanTrunkArry[cnt].type);
            pVlanEntry->CtcVlanArry.trunkInfo.vlanTrunkArry[cnt].tag =
                soc_ntohs(pVlanTrukInfo->vlanTrunkArry[cnt].tag);
        }
        break;
    default:
        ret = ERROR;
        break;
    }
    TkOamMemPut(pathId,(void *) buff);
    return ret;
}
