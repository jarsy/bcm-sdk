/*
 * $Id: TkRuleApi.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkRuleApi.c
 * Purpose: 
 *
 */

#include <shared/bsl.h>

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Rule.h>
#include <soc/ea/tk371x/TkRuleApi.h>
#include <soc/ea/tk371x/TkDebug.h>

static uint8    ruleDataPackageBuf[256];

/*
 *  Function:
 *      TkQueueCfgOamParse
 * Purpose:
 *      Parse the queue configuation information OAM packets
 * Parameters:
 *      cfg    - The configuration raw data
 *      pQcfg    - The queue configuration data structure
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
static int32
TkQueueCfgOamParse(uint8 * cfg, TkQueueConfigInfo * pQcfg)
{
    uint8          *p = (uint8 *) cfg;
    uint8           i;
    uint8           j;

    if (NULL == cfg || NULL == pQcfg) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    sal_memset(pQcfg, 0x0, sizeof(TkQueueConfigInfo));

    pQcfg->cntOfLink = *p++;

    for (i = 0; i < pQcfg->cntOfLink; i++) {
        pQcfg->linkInfo[i].cntOfUpQ = *p++;
        for (j = 0; j < pQcfg->linkInfo[i].cntOfUpQ; j++) {
            pQcfg->linkInfo[i].sizeOfUpQ[j] = *p++;
        }
    }

    pQcfg->cntOfPort = *p++;

    for (i = 0; i < pQcfg->cntOfPort; i++) {
        pQcfg->portInfo[i].cntOfDnQ = *p++;
        for (j = 0; j < pQcfg->portInfo[i].cntOfDnQ; j++) {
            pQcfg->portInfo[i].sizeOfDnQ[j] = *p++;
        }
    }

    /* the flood queue config is for 3701 onu only*/
    pQcfg->mcastInfo.cntOfDnQ = *p++;
    
    for (i = 0; i < pQcfg->mcastInfo.cntOfDnQ; i++) {
        pQcfg->mcastInfo.sizeOfDnQ[i] = *p++;
    }

    return OK;
}

/*
 *  Function:
 *      TkQueueCfgOamPack
 * Purpose:
 *      Pack the queue configuation information OAM packets
 * Parameters:
 *      cfg    - The configuration raw data
 *      pQcfg    - The queue configuration data structure
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
static int32
TkQueueCfgOamPack(uint8 * cfg, TkQueueConfigInfo * pQcfg)
{
    uint8 *p = (uint8 *)cfg;
    uint8 i;
    uint8 j;
    uint32 len = 0;
    
    if (NULL == cfg || NULL == pQcfg) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    /* pack the upstream part.*/
    *p++ = pQcfg->cntOfLink;
    len++;
    
    for(i = 0; i < pQcfg->cntOfLink; i++){
        *p++ = pQcfg->linkInfo[i].cntOfUpQ;
        len++;
        
        for(j = 0; j < pQcfg->linkInfo[i].cntOfUpQ; j++){
            *p++ = pQcfg->linkInfo[i].sizeOfUpQ[j];
            len++;
        }    
    }

    /* pack the downstream part*/
	*p++ = pQcfg->cntOfPort;
    len++;
    for(i = 0; i < pQcfg->cntOfPort; i++){
		*p++ = pQcfg->portInfo[i].cntOfDnQ;
		len++;
        for(j = 0; j < pQcfg->portInfo[i].cntOfDnQ; j++){
            *p++ = pQcfg->portInfo[i].sizeOfDnQ[j];
            len++;
        }
    }

    /* the flood queue config is for 3701 onu only*/
    *p++ = pQcfg->mcastInfo.cntOfDnQ;
    len++;
    /* pack the flood queue configuration*/
    for(i = 0; i < pQcfg->mcastInfo.cntOfDnQ; i++){
        *p++ = pQcfg->mcastInfo.sizeOfDnQ[i];
        len++;
    }

    /* if the payload is larger than max tlv length, must return error.*/
    if(len > OAM_TLV_MAX_LEN){
        return ERROR;
    }else{
        return len;
    }
}

/*
 *  Function:
 *      TkExtOamGetFilterRulesByPort
 * Purpose:
 *      Get all rule of the ONU port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      linkId    - The link id which will patch into the OAM message
 *      direction    - Upstream or downstream traffic, 0 means downstream 1 means upstream.
 *      portIndex    - port Index
 *      pRxBuf    - Pointer to the filter rules gotten
 *      pRxLen    - Pointer to the length of the rules
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int
TkExtOamGetFilterRulesByPort(uint8 pathId,
                             uint8 linkId,
                             OamRuleDirection
                             direction,
                             uint8 portIndex, uint8 * pRxBuf,
                             uint32 * pRxLen)
{
    int             ret = 0;
    OamObjIndex     objIdx;

    objIdx.portId = portIndex;
    if (direction == OamRuleDirDownstream) {
        ret =
            TkExtOamObjGet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAttribute,
                           OamNewExtAttrDnFilterTbl, pRxBuf, pRxLen);

    } else {
        ret =
            TkExtOamObjGet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAttribute,
                           OamNewExtAttrUpFilterTbl, pRxBuf, pRxLen);
    }
    return ret;
}

/*
 *  Function:
 *      TkExtOamClearAllFilterRulesByPort
 * Purpose:
 *      Clear all filter rules of the ONU port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      linkId    - The link id which will patch into the OAM message
 *      direction    - Upstream or downstream traffic, 0 means downstream 1 means upstream.
 *      portIndex    - port Index
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int
TkExtOamClearAllFilterRulesByPort(uint8 pathId,
                                  uint8 linkId,
                                  OamRuleDirection direction,
                                  uint8 portIndex)
{
    int32           ret = 0,
        tmpRet;
    OamObjIndex     objIdx;

    objIdx.portId = portIndex;
    if (direction == OamRuleDirDownstream) {
        tmpRet =
            TkExtOamObjSet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAction, OamExtActClearDnFilterTbl, 0,
                           0);
    } else {
        tmpRet =
            TkExtOamObjSet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAction, OamExtActClearUpFilterTbl, 0,
                           0);
    }
    if (OamVarErrNoError == tmpRet) {
        ret = 0;
    } else {
        ret = -1;
    }
    return ret;
}

/*
 *  Function:
 *      TkExtOamClearAllUserRulesByPort
 * Purpose:
 *      Clear all user rule of the ONU port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      linkId    - The link id which will patch into the OAM message
 *      direction    - Upstream or downstream traffic, 0 means downstream 1 means upstream.
 *      portIndex    - port Index
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int
TkExtOamClearAllUserRulesByPort(uint8 pathId,
                                uint8 linkId,
                                OamRuleDirection direction,
                                uint8 portIndex)
{
    int             ret = 0,
        tmpRet;
    OamObjIndex     objIdx;

    objIdx.portId = portIndex;
    if (direction == OamRuleDirDownstream) {
        tmpRet =
            TkExtOamObjSet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAction, OamExtActClrDnUserRules, 0, 0);
    } else {
        tmpRet =
            TkExtOamObjSet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAction, OamExtActClrUpUserRules, 0, 0);
    }
    if (OamVarErrNoError == tmpRet) {
        ret = 0;
    } else {
        ret = -1;
    }
    return ret;

}

/*
 *  Function:
 *      TkExtOamClearAllClassifyRulesByPort
 * Purpose:
 *      Clear all classify rule of the ONU port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      linkId    - The link id which will patch into the OAM message
 *      direction    - Upstream or downstream traffic, 0 means downstream 1 means upstream.
 *      portIndex    - port Index
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int
TkExtOamClearAllClassifyRulesByPort(uint8 pathId,
                                    uint8 linkId,
                                    OamRuleDirection
                                    direction, uint8 portIndex)
{
    int             ret = 0,
        tmpRet;
    OamObjIndex     objIdx;

    objIdx.portId = portIndex;
    if (direction == OamRuleDirDownstream) {
        tmpRet =
            TkExtOamObjSet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAction, OamExtActClrDnClass, 0, 0);
    } else {
        tmpRet =
            TkExtOamObjSet(pathId, linkId, 3,
                           &objIdx,
                           OamBranchAction, OamExtActClrUpClass, 0, 0);
    }
    if (OamVarErrNoError == tmpRet) {
        ret = 0;
    } else {
        ret = -1;
    }
    return ret;
}

/*
 *  Function:
 *      TkRuleActDiscard
 * Purpose:
 *      Add or delete a fitering rule for an ONU port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      linkId    - The link id which will patch into the OAM message
 *      direction    - Upstream or downstream traffic, 0 means downstream 1 means upstream.
 *      portIndex    - port Index
 *      pri    - Rule priority
 *      numOfConditon    - Condition count
 *      pCondList    - Condition list
 *      action    - Add or remove. 1 for add 0 for remove
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkRuleActDiscard(uint8 pathId, uint8 linkId,
                 uint8 volatiles,
                 uint8 portIndex, uint8 pri,
                 uint8 numOfConditon,
                 OamNewRuleCondition * pCondList, uint8 action)
{
    OamNewRuleSpec *pRuleBuff = (OamNewRuleSpec *) & ruleDataPackageBuf[0];
    OamObjIndex     objIdx;
    int32           ret;

    if (NULL == pCondList) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    sal_memset(ruleDataPackageBuf, 0x0, 256);

    objIdx.portId = portIndex;

    if (portIndex) {
        pRuleBuff->dir = OamRuleDirUpstream;
    } else {
        pRuleBuff->dir = OamRuleDirDownstream;
    }

    pRuleBuff->pri = pri;
    pRuleBuff->action = ActDiscard;
    pRuleBuff->numConds = numOfConditon;
    pRuleBuff->param.new.ruleflag = volatiles;

    conditonBubbleSort(pCondList, numOfConditon);

    sal_memcpy(pRuleBuff->cond, pCondList,
               sizeof(OamNewRuleCondition) * numOfConditon);

    if (action) {
        ret =
            TkExtOamObjSet(pathId, linkId,
                           OamBranchObject,
                           &objIdx,
                           OamBranchAction,
                           OamExtActNewAddRule,
                           ruleDataPackageBuf,
                           sizeof
                           (OamNewRuleSpec) +
                           numOfConditon * sizeof(OamNewRuleCondition));
    } else {
        ret =
            TkExtOamObjSet(pathId, linkId,
                           OamBranchObject,
                           &objIdx,
                           OamBranchAction,
                           OamExtActNewDelRule,
                           ruleDataPackageBuf,
                           sizeof
                           (OamNewRuleSpec) +
                           numOfConditon * sizeof(OamNewRuleCondition));
    }

    if (ret != OamVarErrNoError) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    } else
        return OK;
}


/*
 *  Function:
 *      TkRuleDiscardDsArpWithSpecifiedSenderIpAddr
 * Purpose:
 *      Add or delete a filterring rule to drop particular downstream ARP packet. whose ip is 
 *      the specified IP address.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      ipAddress    - The ip Address
 *      action    - Add or remove. 1 for add 0 for remove
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkRuleDiscardDsArpWithSpecifiedSenderIpAddr(uint8
                                            pathId,
                                            uint32 ipAddress, uint8 action)
{
    OamNewRuleCondition con[3];
    uint16          tmp;
    int32           ret = OK;

    sal_memset(con, 0x0, sizeof(OamNewRuleCondition) * 3);

    con[0].field = SelEthType;
    tmp = soc_htons(EthertypeArp);
    sal_memcpy(&con[0].value[8 - sizeof(uint16)], &tmp, 2);
    con[0].operator = OpEqual;

    con[1].field = SelUser2;
    tmp = soc_htons(ipAddress & 0xffff);
    sal_memcpy(&con[1].value[8 - sizeof(uint16)], &tmp, 2);
    con[1].operator = OpEqual;

    con[2].field = SelUser3;
    tmp = soc_htons(ipAddress >> 16);
    sal_memcpy(&con[2].value[8 - sizeof(uint16)], &tmp, 2);
    con[2].operator = OpEqual;

    ret = TkRuleActDiscard(pathId, 0, 0, 0, 1, 3, con, action);

    return ret;
}

/*
 *  Function:
 *      TkQueueGetConfiguration
 * Purpose:
 *      Get the queue configuration of the EPON chipset
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      linkId    - The link id which will patch into the OAM message
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkQueueGetConfiguration(uint8 pathId, TkQueueConfigInfo * pQueueConfigInfo)
{
    uint8           ret = OK;
    uint8          *pRxBuf = NULL;
    uint32          size = 0;
    OamObjIndex     objIdx;

    if (NULL == pQueueConfigInfo) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    objIdx.portId = 0;

    pRxBuf = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pRxBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret =
        TkExtOamObjGet(pathId, 0, OamNameMacName,
                       &objIdx, OamBranchAction,
                       OamExtActGetQueueConfig, pRxBuf, &size);


    if (ret != OK) {
        ret = ERROR;
    } else {
        ret = TkQueueCfgOamParse(pRxBuf, pQueueConfigInfo);
    }
    TkOamMemPut(pathId, pRxBuf);
    return ret;
}

/*
 *  Function:
 *      TkQueueSetConfiguration
 * Purpose:
 *      Get the queue configuration of the EPON chipset
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      linkId    - The link id which will patch into the OAM message
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkQueueSetConfiguration(uint8 pathId, TkQueueConfigInfo * pQueueConfigInfo)
{
    uint8   ret = OK;
    uint8   *pTxBuf = NULL;
    int32   size = 0;
    OamObjIndex objIdx;

    if (NULL == pQueueConfigInfo) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    objIdx.portId = 0;

    pTxBuf = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pTxBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    size = TkQueueCfgOamPack(pTxBuf, pQueueConfigInfo);

    if(ERROR != size){
        ret = TkExtOamObjSet(pathId, 0, OamNameMacName, &objIdx, OamBranchAction,
            OamExtActSetQueueConfig, pTxBuf, size);
    }else{
        ret = OamVarErrActBadParameters;
    }

    TkOamMemPut(pathId, pTxBuf);

    if(OamVarErrNoError != ret){
        return ERROR;
    }else{
        return OK;
    }
}


/*
 * Function:
 *      TkExtOamPortRuleAdd
 * Purpose:
 *      add one rule bindig to the specified port
 * Parameters:
 *      pathId - The EPON chipset of which will be reseted
 *      linkId - In which link the message exchanged
 *      port - The logic port index.
 *              objType, 0 means the link type while 3 means port type.
 *              index, port value, 0 means pon port, 1-2 means UNI port.
 *      ruleInfo - Rule information include below information.
 *      volatiles -  0 means not save the rule to NVS while 1 means save the rule in NVS
 *      portIndex - the port index in the EPON chipset, 0 identify EPON port, 1 - 2 identify 
 *      UNI port. 
 *      pri - The rule priority
 *      ruleCondition - The rule qualify count and qualify list, the qualify content the field 
 *      type as below list:
 *    	    SelDesMacAddr=0,
 *		    SelSrcMacAddr,
 *		    SelLinkIndex,
 *		    SelEthType,
 *		    SelVlanID,
 *		    SelUser0,
 *		    SelIpv4Protocol,
 *		    SelUser1,
 *		    SelUser2,
 *		    SelUser3,
 *		    SelUser4,
 *		    SelL3Protocol,
 *		    SelIpDesAddrLow,
 *		    SelIpv6DesAddrHi,
 *		    SelIpSrcAddrLow,
 *		    SelIpv6SrcAddrHi
 *      the field value, the field value is a 8 byte memory and must packed the LSB to the 
 *      8th byte while the field is a user defined type
 *      action - rule action while hit occurs. It include below action list:
 *              ActNoOp = 0,
 *          ActReserved0,
 *          ActSetPath,
 *          ActSetAddVlanTag,
 *          ActSetDelVlanTag,
 *          ActSetVidAndAddVlanTag,
 *          ActSetCos,
 *          ActReplaceVlanTag,
 *          ActReplaceVlanTagAndSetVid,
 *          ActClearAddVlanTag,
 *          ActClearDelVlanTag = 10,
 *          ActClearDelVlanTagAndSetAddVlanTag,
 *          ActCopyFieldToCos,
 *          ActCopyFieldToVid,
 *          ActDiscard,
 *          ActReserved1,
 *          ActForward,
 *          ActReserved2,
 *          ActSetPathAndForward,
 *          ActSetAddVlanTagAndForward,
 *          ActSetDelVlanTagAndForward = 20,
 *          ActSetVidAndSetAddVlanTagAndForward,
 *          ActSetCosAndForward,
 *          ActReplaceTagAndForward,
 *          ActReplaceTagAndSetVidAndForward,
 *          ActClearAddVlanTagAndForward,
 *          ActClearDelVlanTagAndForward,
 *          ActClearDelVlanTagAndSetAddVlanTagAndForward,
 *          ActCopyFieldToCosAndForward,
 *          ActCopyFieldToVidAndForward,
 *    param - the action parameter. Some rules like rearking vid and cos, this rule need 
 *      the vid or cos value which will be used to add or replaced the existing vid or cos.
 *      like the set destination rule, this rule need know which queue of the port or link the 
 *      packets will be dropped to. If the rule have no parameter, just fill zero to the 
 *      parameter.
 * Returns:
 *    OK or ERROR
 * Notes:
 *    NONE
 */
int32 
TkExtOamPortRuleAdd(uint8 pathId,uint8 linkId, LogicalPortIndex port, 
    TkPortRuleInfo *ruleInfo)
{
    OamNewRuleSpec *pRuleBuff = (OamNewRuleSpec *)&ruleDataPackageBuf[0];
	OamNewRuleCondition *pCondList = pRuleBuff->cond;
    OamObjIndex objIdx;
	uint8 index = 0; 
    int32 ret;

    if(NULL == ruleInfo){
	    TkDbgTrace(TkDbgErrorEnable);
	    return ERROR;                   
    }

    sal_memset(ruleDataPackageBuf,0x0,256);

    objIdx.portId = port.index;

    if(TkObjTypeLink == port.objType){
        return ERROR;
    }
    
    if(port.index){
	    pRuleBuff->dir = OamRuleDirUpstream;
	}else{
        pRuleBuff->dir = OamRuleDirDownstream;
    }
	    
    pRuleBuff->pri = ruleInfo->priority;
    pRuleBuff->action = ruleInfo->action;
    pRuleBuff->numConds = ruleInfo->ruleCondition.conditionCount;
    pRuleBuff->param.new.ruleflag = ruleInfo->volatiles;
    
	switch(ruleInfo->action){
		case ActSetPath:
		case ActSetPathAndForward:
			pRuleBuff->param.new.nparam.ndest.port_link = ruleInfo->param.ndest.port_link;
    		pRuleBuff->param.new.nparam.ndest.queue = ruleInfo->param.ndest.queue;
			break;
    	case ActSetVidAndAddVlanTag:
    	case ActSetCos:
		case ActReplaceVlanTagAndSetVid:
        case ActClearDelVlanTagAndSetAddVlanTag:
    	case ActSetVidAndSetAddVlanTagAndForward:
		case ActReplaceTagAndSetVidAndForward:
        case ActSetCosAndForward:    
    	case ActClearDelVlanTagAndSetAddVlanTagAndForward:
			pRuleBuff->param.new.nparam.vid_cos = soc_htons(ruleInfo->param.vid_cos);
            break;
        case ActNoOp:
        case ActReserved0:
        case ActSetAddVlanTag:
        case ActSetDelVlanTag:
        case ActReplaceVlanTag:
        case ActClearAddVlanTag:
        case ActClearDelVlanTag:
        case ActCopyFieldToCos:
        case ActCopyFieldToVid:
        case ActDiscard:
        case ActReserved1:
        case ActForward:
        case ActReserved2:
        case ActSetAddVlanTagAndForward:
        case ActSetDelVlanTagAndForward:
        case ActReplaceTagAndForward:
        case ActClearAddVlanTagAndForward:
        case ActClearDelVlanTagAndForward:
        case ActCopyFieldToCosAndForward:
        case ActCopyFieldToVidAndForward:
        case ActReserved3:
        case Actreserved4:
        default:
            break;
    }

	for(index = 0; index < ruleInfo->ruleCondition.conditionCount; index++){
		pCondList[index].operator = ruleInfo->ruleCondition.conditionList[index].operator;
		pCondList[index].field = ruleInfo->ruleCondition.conditionList[index].field;

        switch(ruleInfo->ruleCondition.conditionList[index].field){
            case SelDesMacAddr:
            case SelSrcMacAddr:
            case SelLinkIndex:
            case SelEthType:
            case SelVlanID:
            case SelUser0:
            case SelIpv4Protocol:
            case SelUser1:
            case SelUser2:
            case SelUser3:
            case SelUser4:
            case SelL3Protocol:
            case SelIpDesAddrLow:
            case SelIpv6DesAddrHi:
            case SelIpSrcAddrLow:
            case SelIpv6SrcAddrHi:
                sal_memcpy(pCondList[index].value, 
                    ruleInfo->ruleCondition.conditionList[index].common.value,
                    8);
                break;
            default:
                LOG_CLI((BSL_META("unkonw type\n")));
                break;
        }
	}

    conditonBubbleSort(pCondList,ruleInfo->ruleCondition.conditionCount);

    ret = TkExtOamObjSet(pathId,linkId, OamBranchObject, &objIdx, 
        OamBranchAction,OamExtActNewAddRule,ruleDataPackageBuf, 
        sizeof(OamNewRuleSpec)+(ruleInfo->ruleCondition.conditionCount-1)*sizeof(OamNewRuleCondition));

	if(ret != OamVarErrNoError){
	    TkDbgTrace(TkDbgErrorEnable);
	    return ERROR;                   
	}else{
        return OK;
    }
}

/*
 * Function:
 *      TkExtOamPortRuleDel
 * Purpose:
 *      add one rule bindig to the specified port
 * Parameters:
 *      pathId - The EPON chipset of which will be reseted
 *      linkId - In which link the message exchanged
 *      port - The logic port index.
 *              objType, 0 means the link type while 3 means port type.
 *              index, port value, 0 means pon port, 1-2 means UNI port.
 *      ruleInfo - Rule information include below information.
 *      volatiles -  0 means not save the rule to NVS while 1 means save the rule in NVS
 *      portIndex - the port index in the EPON chipset, 0 identify EPON port, 1 - 2 identify 
 *      UNI port. 
 *      pri - The rule priority
 *      ruleCondition - The rule qualify count and qualify list, the qualify content the field 
 *      type as below list:
 *          SelDesMacAddr=0,
 *          SelSrcMacAddr,
 *          SelLinkIndex,
 *          SelEthType,
 *          SelVlanID,
 *          SelUser0,
 *          SelIpv4Protocol,
 *          SelUser1,
 *          SelUser2,
 *          SelUser3,
 *          SelUser4,
 *          SelL3Protocol,
 *          SelIpDesAddrLow,
 *          SelIpv6DesAddrHi,
 *          SelIpSrcAddrLow,
 *          SelIpv6SrcAddrHi
 *      the field value, the field value is a 8 byte memory and must packed the LSB to the 
 *      8th byte while the field is a user defined type
 *      action - rule action while hit occurs. It include below action list:
 *              ActNoOp = 0,
 *          ActReserved0,
 *          ActSetPath,
 *          ActSetAddVlanTag,
 *          ActSetDelVlanTag,
 *          ActSetVidAndAddVlanTag,
 *          ActSetCos,
 *          ActReplaceVlanTag,
 *          ActReplaceVlanTagAndSetVid,
 *          ActClearAddVlanTag,
 *          ActClearDelVlanTag = 10,
 *          ActClearDelVlanTagAndSetAddVlanTag,
 *          ActCopyFieldToCos,
 *          ActCopyFieldToVid,
 *          ActDiscard,
 *          ActReserved1,
 *          ActForward,
 *          ActReserved2,
 *          ActSetPathAndForward,
 *          ActSetAddVlanTagAndForward,
 *          ActSetDelVlanTagAndForward = 20,
 *          ActSetVidAndSetAddVlanTagAndForward,
 *          ActSetCosAndForward,
 *          ActReplaceTagAndForward,
 *          ActReplaceTagAndSetVidAndForward,
 *          ActClearAddVlanTagAndForward,
 *          ActClearDelVlanTagAndForward,
 *          ActClearDelVlanTagAndSetAddVlanTagAndForward,
 *          ActCopyFieldToCosAndForward,
 *          ActCopyFieldToVidAndForward,
 *    param - the action parameter. Some rules like rearking vid and cos, this rule need 
 *      the vid or cos value which will be used to add or replaced the existing vid or cos.
 *      like the set destination rule, this rule need know which queue of the port or link the 
 *      packets will be dropped to. If the rule have no parameter, just fill zero to the 
 *      parameter.
 * Returns:
 *    OK or ERROR
 * Notes:
 *    NONE
 */
int32 
TkExtOamPortRuleDel(uint8 pathId,uint8 linkId, LogicalPortIndex port, 
    TkPortRuleInfo *ruleInfo)
    {
    OamNewRuleSpec *pRuleBuff = (OamNewRuleSpec *)&ruleDataPackageBuf[0];
	OamNewRuleCondition *pCondList = pRuleBuff->cond;
    OamObjIndex objIdx;
	uint8 index = 0; 
    int32 ret;

    if(NULL == ruleInfo){
	    TkDbgTrace(TkDbgErrorEnable);
	    return ERROR;                   
    }

    sal_memset(ruleDataPackageBuf,0x0,256);

    if(TkObjTypePort != port.objType){
        return ERROR;
    }

    objIdx.portId = port.index;
    
	if(port.index){
		pRuleBuff->dir = OamRuleDirUpstream;
    }else{
        pRuleBuff->dir = OamRuleDirDownstream;
    }

    pRuleBuff->pri = ruleInfo->priority;
    pRuleBuff->action = ruleInfo->action;
    pRuleBuff->numConds = ruleInfo->ruleCondition.conditionCount;
    pRuleBuff->param.new.ruleflag = ruleInfo->volatiles;
	switch(ruleInfo->action){
    	case ActSetPath:
		case ActSetPathAndForward:
			pRuleBuff->param.new.nparam.ndest.port_link = ruleInfo->param.ndest.port_link;
    		pRuleBuff->param.new.nparam.ndest.queue = ruleInfo->param.ndest.queue;
			break;
    	case ActSetVidAndAddVlanTag:
    	case ActSetCos:
		case ActReplaceVlanTagAndSetVid:
        case ActClearDelVlanTagAndSetAddVlanTag:    
    	case ActSetVidAndSetAddVlanTagAndForward:
		case ActReplaceTagAndSetVidAndForward:
        case ActSetCosAndForward:
    	case ActClearDelVlanTagAndSetAddVlanTagAndForward:    
			pRuleBuff->param.new.nparam.vid_cos = soc_htons(ruleInfo->param.vid_cos);
			break;
		case ActNoOp:
		case ActReserved0:
		case ActSetAddVlanTag:
		case ActSetDelVlanTag:
		case ActReplaceVlanTag:
		case ActClearAddVlanTag:
		case ActClearDelVlanTag:
		case ActCopyFieldToCos:
		case ActCopyFieldToVid:
		case ActDiscard:
		case ActReserved1:
		case ActForward:
		case ActReserved2:
		case ActSetAddVlanTagAndForward:
		case ActSetDelVlanTagAndForward:
		case ActReplaceTagAndForward:
		case ActClearAddVlanTagAndForward:
		case ActClearDelVlanTagAndForward:
		case ActCopyFieldToCosAndForward:
		case ActCopyFieldToVidAndForward:
		case ActReserved3:
		case Actreserved4:
		default:
			break;
	}

	for(index = 0; index < ruleInfo->ruleCondition.conditionCount; index++){
		pCondList[index].operator = ruleInfo->ruleCondition.conditionList[index].operator;
		pCondList[index].field = ruleInfo->ruleCondition.conditionList[index].field;

        switch(ruleInfo->ruleCondition.conditionList[index].field){
            case SelDesMacAddr:
            case SelSrcMacAddr:
            case SelLinkIndex:
            case SelEthType:
            case SelVlanID:
            case SelUser0:
            case SelIpv4Protocol:
            case SelUser1:
            case SelUser2:
            case SelUser3:
            case SelUser4:
            case SelL3Protocol:
            case SelIpDesAddrLow:
            case SelIpv6DesAddrHi:
            case SelIpSrcAddrLow:
            case SelIpv6SrcAddrHi:
                sal_memcpy(pCondList[index].value, 
                    ruleInfo->ruleCondition.conditionList[index].common.value,
                    8);
                break;
            default:
                LOG_CLI((BSL_META("unkonw type\n")));
                break;
        }
    }

    conditonBubbleSort(pCondList,ruleInfo->ruleCondition.conditionCount);

    ret = TkExtOamObjSet(pathId,linkId, OamBranchObject, &objIdx, 
        OamBranchAction,OamExtActNewDelRule,ruleDataPackageBuf, 
        sizeof(OamNewRuleSpec)+(ruleInfo->ruleCondition.conditionCount-1)*sizeof(OamNewRuleCondition));

    if(ret != OamVarErrNoError){
	    TkDbgTrace(TkDbgErrorEnable);
	    return ERROR;                   
    }else{
        return OK;
    }       
}
