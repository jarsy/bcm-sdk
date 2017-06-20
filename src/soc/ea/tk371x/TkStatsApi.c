/*
 * $Id: TkStatsApi.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkStatsApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkUtils.h>
#include <soc/ea/tk371x/TkStatsP.h>
#include <soc/ea/tk371x/TkInit.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/TkStatsApi.h>

/*
 * send TK extension OAM message Get the EPON port stats from the ONU 
 */
int
TkExtOamGetEponStats(uint8 pathId, uint8 LinkId, uint8 StatsId,
                     uint64 * pCounter)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint64          counter;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) 
        || (StatsId >= STATSID_NUM)
        || (NULL == pCounter)
        ) {
        return (OamVarErrActBadParameters);
    }

    index.portId = 0;
    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       StatIdToAttrLeaf[StatsId],
                       (uint8 *) & counter, &DataLen)) {
        htonll(counter, pCounter);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Get the UNI port stats from the ONU 
 */
int
TkExtOamGetUniStats(uint8 pathId, uint8 LinkId,
                    uint8 port, uint8 StatsId, uint64 * pCounter)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint64          counter;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) ||
           (port > SDK_MAX_NUM_OF_PORT) || (StatsId >= STATSID_NUM)
           || (NULL == pCounter)
        ) {
        return (OamVarErrActBadParameters);
    }

    index.portId = port;

    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       StatIdToAttrLeaf[StatsId],
                       (uint8 *) & counter, &DataLen)) {
        htonll(counter, pCounter);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Get the link stats from the ONU 
 */
int
TkExtOamGetLinkStats(uint8 pathId, uint8 LinkId, uint8 StatsId,
                     uint64 * pCounter)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint64          counter;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) || (StatsId >= STATSID_NUM)
           || (NULL == pCounter)
        ) {
        return (OamVarErrActBadParameters);
    }

    index.linkId = LinkId;

    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNameMacName, &index,
                       OamBranchAttribute,
                       StatIdToAttrLeaf[StatsId],
                       (uint8 *) & counter, &DataLen)) {
        htonll(counter, pCounter);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Get the queue stats from the ONU 
 */
int
TkExtOamGetQueueStats(uint8 pathId, uint8 LinkId,
                      uint8 port, uint8 queue, uint8 StatsId,
                      uint64 * pCounter)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint64          counter;

    if ((LinkId > 7)
        || ((1 != port) && (2 != port))
        || (queue > 19)
        || (StatsId >= STATSID_NUM)
        || (NULL == pCounter)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.queueId.port = soc_htons((uint16) port);
    index.queueId.link = soc_htons((uint16) LinkId);
    index.queueId.queue = soc_htons((uint16) queue);

    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNameQueueName, &index,
                       OamBranchAttribute,
                       StatIdToAttrLeaf[StatsId],
                       (uint8 *) & counter, &DataLen)) {
        htonll(counter, pCounter);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Set clear all stats to the ONU 
 */
int
TkExtOamSetClrStats(uint8 pathId, uint8 LinkId)
{
	uint8 rv;
	
    if ((LinkId > SDK_MAX_NUM_OF_LINK)) {
        return (OamVarErrActBadParameters);
    }

	rv = TkExtOamSet(pathId, LinkId, OamBranchAction, OamExtActClearStats, 
		NULL,0);

	if(OamVarErrNoError != rv){
		return ERROR;
	}else{
		return OK;
	}
}

/*
 * send TK extension OAM message Get the UNI port stats from the ONU U64
 * ifInOctets; uint64 ifInUcastPkts; uint64 ifInNUcastPkts; uint64 ifInDiscards;
 * uint64 ifInErrors; uint64 ifOutOctets; uint64 ifOutUcastPkts; uint64
 * ifOutNUcastPkts; uint64 ifOutDiscards; uint64 ifOutErrors; uint64 reserved1; uint64 
 * reserved2; uint64 reserved3; uint64 reserved4; 
 */
int
TkExtOamGetUniAllStats(uint8 pathId,
                       uint8 linkId, uint8 port, TkExtAllStats * pStats)
{
    BufInfo         bufInfo;
    Bool            ok;
    uint32          rxLen;
    uint8           ret;
    tGenOamVar      var;
    uint8           buf[4] = { 0X0 };
    uint64          statsArry[12] = { 0x0 };
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);

    if (linkId != 0) {
        return RcBadParam;
    }

    if (TkOamMsgPrepare(pathId,&bufInfo, OamExtOpVarRequest) != OK) {
        return RcNoResource;
    }

    Tk2BufU16(buf, port);
    ok = AddOamTlv(&bufInfo, OamBranchNameBinding, OamNamePhyName, 2, buf);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsRxOk);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxUnicastFrames);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesRxOk);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesRxOk);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrameTooShort);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFrameTooLong);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFcsErr);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacAlignErr);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsTxOk);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxUnicastFrames);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesTxOk);
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesTxOk);

    ret = TxOamDeliver(pathId, linkId, &bufInfo, (uint8 *) rxTmpBuf, &rxLen);
    if (ret != RcOk)
        return ret;

    if (TkDbgLevelIsSet(TkDbgLogTraceEnable))
        BufDump(NULL, rxTmpBuf, rxLen);

    var.pValue = NULL;
    sal_memset(&var, 0x0, sizeof(tGenOamVar));

    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsRxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[0]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[1]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[2]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[3]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrameTooShort,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[4]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFrameTooLong,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[5]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamAttrMacFcsErr,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[6]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacAlignErr, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[7]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsTxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[8]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[9]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[10]));

    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(statsArry[11]));

    pStats->ifInOctets = statsArry[0];
    pStats->ifInUcastPkts = statsArry[1];
    pStats->ifInNUcastPkts = statsArry[2] + statsArry[3];
    pStats->ifInDiscards = 0;
    pStats->ifInErrors =
        statsArry[4] + statsArry[5] + statsArry[6] + statsArry[7];
    pStats->ifOutOctets = statsArry[8];
    pStats->ifOutUcastPkts = statsArry[9];
    pStats->ifOutNUcastPkts = statsArry[10] + statsArry[11];
    pStats->ifOutDiscards = 0;
    pStats->ifOutErrors = 0;
    pStats->reserved1 = 0;
    pStats->reserved2 = 0;
    pStats->reserved3 = 0;
    pStats->reserved4 = 0;

    return ret;
}

int
TkExtOamGetPortStats(uint8 pathId, uint8 linkId,
                     uint8 port, TkMacAllPortStats1 * pStats)
{
    BufInfo         bufInfo;
    Bool            ok;
    uint32          rxLen;
    uint8           ret;
    tGenOamVar      var;
    uint8           buf[4] = { 0X0 };
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);
    
    if (NULL == pStats) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (linkId != 0) {
        return RcBadParam;
    }

    if (TkOamMsgPrepare(pathId,&bufInfo, OamExtOpVarRequest) != OK) {
        return RcNoResource;
    }

    Tk2BufU16(buf, port);
    ok = AddOamTlv(&bufInfo, OamBranchNameBinding, OamNamePhyName, 2, buf);
    /* 07 00 02*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFramesTxOk);
    /* 07 00 05*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFramesRxOk);
    /* 07 00 06*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFcsErr);
    /* 07 00 07*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacAlignErr);
    /* 07 00 08*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsTxOk);
    /* 07 00 0E*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsRxOk);
    /* 07 00 12*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesTxOk);
    /* 07 00 13*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesTxOk);
    /* 07 00 15*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesRxOk);
    /* 07 00 16*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesRxOk);
    /* 07 00 17*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacInRangeLenErr);
    /* 07 00 19*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFrameTooLong);
    /* 07 00 63*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacCtrlPauseRx);
    /* 07 00 84*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxUnicastFrames);
    /* 07 00 85*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxUnicastFrames);
    /* 07 00 86*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrameTooShort);
    /* 07 00 87*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame64);
    /* 07 00 88*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame65_127);
    /* 07 00 89*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame128_255);
    /* 07 00 8A*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame256_511);
    /* 07 00 8B*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame512_1023);
    /* 07 00 8C*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame1024_1518);
    /* 07 00 8D*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame1519Plus);
    /* 07 00 8E*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame64);
    /* 07 00 8F*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame65_127);
    /* 07 00 90*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame128_255);
    /* 07 00 91*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame256_511);
    /* 07 00 92*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame512_1023);
    /* 07 00 93*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame1024_1518);
    /* 07 00 94*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame1519Plus);
    /* 07 00 96*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxDelay);
    /* 07 00 97*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFramesDropped);
    /* 07 00 98*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxBytesDropped);
    /* 07 00 99*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxBytesDelayed);
    /* 07 00 9A*/ 
    /* ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxBytesUnused);*/
    /* 07 00 9C*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxDelay);
    /* 07 00 9D*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFramesDropped);
    /* 07 00 9E*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxBytesDropped);
    /* 07 00 9F*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxBytesDelayed);
    /* 07 00 F9*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrOamEmulCrc8Err);
    /* 07 01 24*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrFecCorrectedBlocks);
    /* 07 01 25*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrFecUncorrectableBlocks);

    ret = TxOamDeliver(pathId, linkId, &bufInfo, (uint8 *) rxTmpBuf, &rxLen);
    if (ret != RcOk)
        return ret;

    if (TkDbgLevelIsSet(TkDbgLogTraceEnable))
        BufDump(NULL, rxTmpBuf, rxLen);

    var.pValue = NULL;
    sal_memset(&var, 0x0, sizeof(tGenOamVar));

    /* 07 00 02*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFramesTxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFramesTxOk));
    /* 07 00 05*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFramesRxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFramesRxOk));

    /* 07 00 06*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamAttrMacFcsErr,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFcsErr));

    /* 07 00 07*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacAlignErr, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacAlignErr));

    /* 07 00 08*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsTxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacOctetsTxOk));

    /* 07 00 0E*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsRxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacOctetsRxOk));

    /* 07 00 12*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacMcastFramesTxOk));

    /* 07 00 13*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacBcastFramesTxOk));

    /* 07 00 15*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacMcastFramesRxOk));

    /* 07 00 16*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacBcastFramesRxOk));

    /* 07 00 17*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacInRangeLenErr,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacInRangeLenErr));

    /* 07 00 19*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFrameTooLong,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFrameTooLong));

    /* 07 00 63*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacCtrlPauseRx, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacCtrlPauseRx));

    /* 07 00 84*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxUnicastFrames));

    /* 07 00 85*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxUnicastFrames));

    /* 07 00 86 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrameTooShort,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrameTooShort));

    /* 07 00 87*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame64, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame64));

    /* 07 00 88*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame65_127,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame65_127));

    /* 07 00 89*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame128_255,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame128_255));

    /* 07 00 8A*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame256_511,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame256_511));

    /* 07 00 8B*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame512_1023,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame512_1023));

    /* 07 00 8C */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamExtAttrRxFrame1024_1518, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame1024_1518));

    /* 07 00 8D*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame1519Plus,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame1519Plus));

    /* 07 00 8E*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame64, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame64));

    /* 07 00 8F*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame65_127,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame65_127));

    /* 07 00 90*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame128_255,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame128_255));

    /* 07 00 91*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame256_511,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame256_511));

    /* 07 00 92*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame512_1023,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame512_1023));

    /* 07 00 93*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamExtAttrTxFrame1024_1518, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame1024_1518));

    /* 07 00 94*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame1519Plus,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame1519Plus));

    /* 07 00 96*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamExtAttrTxDelay,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxDelay));

    /* 07 00 97*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFramesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFramesDropped));

    /* 07 00 98 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxBytesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxBytesDropped));

    /* 07 00 99 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxBytesDelayed,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxBytesDelayed));

    /* 07 00 9C*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamExtAttrRxDelay,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxDelay));

    /* 07 00 9D */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFramesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFramesDropped));

    /* 07 00 9E */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxBytesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxBytesDropped));

    /* 07 00 9F */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxBytesDelayed,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxBytesDelayed));

    /* 07 00 F9 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrOamEmulCrc8Err, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrOamEmulCrc8Err));

    /* 07 01 24 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrFecCorrectedBlocks,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    sal_memcpy(&
               (pStats->OamAttrFecCorrectedBlocks),
               var.pValue, sizeof(uint32));
    pStats->OamAttrFecCorrectedBlocks =
        soc_ntohl(pStats->OamAttrFecCorrectedBlocks);

    /* 07 01 25*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamAttrFecUncorrectableBlocks, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    sal_memcpy(&
               (pStats->OamAttrFecUncorrectableBlocks),
               var.pValue, sizeof(uint32));
    pStats->OamAttrFecUncorrectableBlocks =
        soc_ntohl(pStats->OamAttrFecUncorrectableBlocks);

    return ret;
}

int
TkExtOamGetPONPortStats(uint8 pathId,
                        uint8 linkId, TkMacAllPortStats1 * pStats)
{
    BufInfo         bufInfo;
    Bool            ok;
    uint32          rxLen;
    uint8           ret;
    tGenOamVar      var;
    uint8           buf[4] = { 0X0 };
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);

    if (NULL == pStats) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (linkId != 0) {
        return RcBadParam;
    }

    if (TkOamMsgPrepare(pathId,&bufInfo, OamExtOpVarRequest) != OK) {
        return RcNoResource;
    }

    Tk2BufU16(buf, 0);
    ok = AddOamTlv(&bufInfo, OamBranchNameBinding, OamNamePhyName, 2, buf);
    /* 07 00 02*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFramesTxOk);
    /* 07 00 05*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFramesRxOk);
    /* 07 00 06*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFcsErr);
    /* 07 00 07*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacAlignErr);
    /* 07 00 08*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsTxOk);
    /* 07 00 0E*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsRxOk);
    /* 07 00 12*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesTxOk);
    /* 07 00 13*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesTxOk);
    /* 07 00 15*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesRxOk);
    /* 07 00 16*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesRxOk);
    /* 07 00 17*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacInRangeLenErr);
    /* 07 00 19*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFrameTooLong);
    /* 07 00 63*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacCtrlPauseRx);
    /* 07 00 84*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxUnicastFrames);
    /* 07 00 85*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxUnicastFrames);
    /* 07 00 86*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrameTooShort);
    /* 07 00 87*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame64);
    /* 07 00 88*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame65_127);
    /* 07 00 89*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame128_255);
    /* 07 00 8A*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame256_511);
    /* 07 00 8B*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame512_1023);
    /* 07 00 8C*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame1024_1518);
    /* 07 00 8D*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame1519Plus);
    /* 07 00 8E*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame64);
    /* 07 00 8F*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame65_127);
    /* 07 00 90*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame128_255);
    /* 07 00 91*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame256_511);
    /* 07 00 92*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame512_1023);
    /* 07 00 93*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame1024_1518);
    /* 07 00 94*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame1519Plus);
    /* 07 00 96*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxDelay);
    /* 07 00 97*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFramesDropped);
    /* 07 00 98*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxBytesDropped);
    /* 07 00 99*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxBytesDelayed);
    /* 07 00 9A*/ 
    /* ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxBytesUnused);*/
    /* 07 00 9C*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxDelay);
    /* 07 00 9D*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFramesDropped);
    /* 07 00 9E*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxBytesDropped);
    /* 07 00 9F*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxBytesDelayed);
    /* 07 00 F9*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrOamEmulCrc8Err);
    /* 07 01 24*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrFecCorrectedBlocks);
    /* 07 01 25*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrFecUncorrectableBlocks);

    ret = TxOamDeliver(pathId, linkId, &bufInfo, (uint8 *) rxTmpBuf, &rxLen);
    if (ret != RcOk)
        return ret;

    if (TkDbgLevelIsSet(TkDbgLogTraceEnable))
        BufDump(NULL, rxTmpBuf, rxLen);

    var.pValue = NULL;
    sal_memset(&var, 0x0, sizeof(tGenOamVar));

    /* 07 00 02*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFramesTxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFramesTxOk));
    /* 07 00 05*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFramesRxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFramesRxOk));

    /* 07 00 06*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamAttrMacFcsErr,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFcsErr));

    /* 07 00 07 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacAlignErr, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacAlignErr));

    /* 07 00 08 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsTxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacOctetsTxOk));

    /* 07 00 0E*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsRxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacOctetsRxOk));

    /* 07 00 12*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacMcastFramesTxOk));

    /* 07 00 13*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacBcastFramesTxOk));

    /* 07 00 15*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacMcastFramesRxOk));

    /* 07 00 16*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacBcastFramesRxOk));

    /* 07 00 17*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacInRangeLenErr,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacInRangeLenErr));

    /* 07 00 19*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFrameTooLong,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFrameTooLong));

    /* 07 00 63*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacCtrlPauseRx, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacCtrlPauseRx));

    /* 07 00 84*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxUnicastFrames));

    /* 07 00 85*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxUnicastFrames));

    /* 07 00 86 */
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrameTooShort,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrameTooShort));

    /* 07 00 87*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame64, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame64));

    /* 07 00 88*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame65_127,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame65_127));

    /* 07 00 89*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame128_255,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame128_255));

    /* 07 00 8A*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame256_511,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame256_511));

    /* 07 00 8B*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame512_1023,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame512_1023));

    /* 07 00 8C*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamExtAttrRxFrame1024_1518, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame1024_1518));

    /* 07 00 8D*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame1519Plus,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame1519Plus));

    /* 07 00 8E*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame64, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame64));

    /* 07 00 8F*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame65_127,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame65_127));

    /* 07 00 90*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame128_255,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame128_255));

    /* 07 00 91*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame256_511,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame256_511));

    /* 07 00 92*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame512_1023,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame512_1023));

    /* 07 00 93*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamExtAttrTxFrame1024_1518, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame1024_1518));

    /* 07 00 94*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame1519Plus,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame1519Plus));

    /* 07 00 96*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamExtAttrTxDelay,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxDelay));

    /* 07 00 97*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFramesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFramesDropped));

    /* 07 00 98*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxBytesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxBytesDropped));

    /* 07 00 99*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxBytesDelayed,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxBytesDelayed));

    /* 07 00 9C*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamExtAttrRxDelay,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxDelay));

    /* 07 00 9D*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFramesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFramesDropped));

    /* 07 00 9E*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxBytesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxBytesDropped));

    /* 07 00 9F*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxBytesDelayed,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxBytesDelayed));

    /* 07 00 F9*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrOamEmulCrc8Err, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrOamEmulCrc8Err));

    /* 07 01 24*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrFecCorrectedBlocks,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    sal_memcpy(&
               (pStats->OamAttrFecCorrectedBlocks),
               var.pValue, sizeof(uint32));
    pStats->OamAttrFecCorrectedBlocks =
        soc_ntohl(pStats->OamAttrFecCorrectedBlocks);

    /* 07 01 25*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamAttrFecUncorrectableBlocks, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    sal_memcpy(&
               (pStats->OamAttrFecUncorrectableBlocks),
               var.pValue, sizeof(uint32));
    pStats->OamAttrFecUncorrectableBlocks =
        soc_ntohl(pStats->OamAttrFecUncorrectableBlocks);
    return ret;
}

int
TkExtOamGetUNIPortStats(uint8 pathId,
                        uint8 linkId, uint8 port,
                        TkMacAllPortStats1 * pStats)
{
    BufInfo         bufInfo;
    Bool            ok;
    uint32          rxLen;
    uint8           ret;
    tGenOamVar      var;
    uint8           buf[4] = { 0X0 };
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);

    if (NULL == pStats) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (linkId != 0) {
        return RcBadParam;
    }

    if (TkOamMsgPrepare(pathId,&bufInfo, OamExtOpVarRequest) != OK) {
        return RcNoResource;
    }

    Tk2BufU16(buf, port);
    ok = AddOamTlv(&bufInfo, OamBranchNameBinding, OamNamePhyName, 2, buf);
    /* 07 00 02*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFramesTxOk);
    /* 07 00 05*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFramesRxOk);
    /* 07 00 06*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFcsErr);
    /* 07 00 07*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacAlignErr);
    /* 07 00 08*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsTxOk);
    /* 07 00 0E*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacOctetsRxOk);
    /* 07 00 12*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesTxOk);
    /* 07 00 13*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesTxOk);
    /* 07 00 15*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacMcastFramesRxOk);
    /* 07 00 16*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacBcastFramesRxOk);
    /* 07 00 17*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacInRangeLenErr);
    /* 07 00 19*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacFrameTooLong);
    /* 07 00 63*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamAttrMacCtrlPauseRx);
    /* 07 00 84*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxUnicastFrames);
    /* 07 00 85*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxUnicastFrames);
    /* 07 00 86*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrameTooShort);
    /* 07 00 87*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame64);
    /* 07 00 88*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame65_127);
    /* 07 00 89*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame128_255);
    /* 07 00 8A*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame256_511);
    /* 07 00 8B*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame512_1023);
    /* 07 00 8C*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame1024_1518);
    /* 07 00 8D*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrRxFrame1519Plus);
    /* 07 00 8E*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame64);
    /* 07 00 8F*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame65_127);
    /* 07 00 90*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame128_255);
    /* 07 00 91*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame256_511);
    /* 07 00 92*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame512_1023);
    /* 07 00 93*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame1024_1518);
    /* 07 00 94*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFrame1519Plus);
    /* 07 00 97*/
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxFramesDropped);
    /* 07 00 98*/ 
    ok = ok && OamAddAttrLeaf(&bufInfo, OamExtAttrTxBytesDropped);

    ret = TxOamDeliver(pathId, linkId, &bufInfo, (uint8 *) rxTmpBuf, &rxLen);
    if (ret != RcOk)
        return ret;

    if (TkDbgLevelIsSet(TkDbgLogTraceEnable))
        BufDump(NULL, rxTmpBuf, rxLen);

    var.pValue = NULL;
    sal_memset(&var, 0x0, sizeof(tGenOamVar));

    /* 07 00 02*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFramesTxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFramesTxOk));
    /* 07 00 05*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFramesRxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFramesRxOk));

    /* 07 00 06*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf, OamBranchAttribute, OamAttrMacFcsErr,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFcsErr));

    /* 07 00 07*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacAlignErr, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacAlignErr));

    /* 07 00 08*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsTxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacOctetsTxOk));

    /* 07 00 0E*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacOctetsRxOk, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacOctetsRxOk));

    /* 07 00 12*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacMcastFramesTxOk));

    /* 07 00 13*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesTxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacBcastFramesTxOk));

    /* 07 00 15*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacMcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacMcastFramesRxOk));

    /* 07 00 16*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacBcastFramesRxOk,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacBcastFramesRxOk));

    /* 07 00 17*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacInRangeLenErr,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacInRangeLenErr));

    /* 07 00 19*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacFrameTooLong,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacFrameTooLong));

    /* 07 00 63*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamAttrMacCtrlPauseRx, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamAttrMacCtrlPauseRx));

    /* 07 00 84*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxUnicastFrames));

    /* 07 00 85*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxUnicastFrames,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxUnicastFrames));

    /* 07 00 86*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrameTooShort,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrameTooShort));

    /* 07 00 87*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame64, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame64));

    /* 07 00 88*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame65_127,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame65_127));

    /* 07 00 89*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame128_255,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame128_255));

    /* 07 00 8A*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame256_511,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame256_511));

    /* 07 00 8B*/
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame512_1023,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame512_1023));

    /* 07 00 8C*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamExtAttrRxFrame1024_1518, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame1024_1518));

    /* 07 00 8D*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrRxFrame1519Plus,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrRxFrame1519Plus));

    /* 07 00 8E*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame64, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame64));

    /* 07 00 8F*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame65_127,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame65_127));

    /* 07 00 90*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame128_255,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame128_255));

    /* 07 00 91*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame256_511,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame256_511));

    /* 07 00 92*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame512_1023,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame512_1023));

    /* 07 00 93*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute,
                          OamExtAttrTxFrame1024_1518, &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame1024_1518));

    /* 07 00 94*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFrame1519Plus,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFrame1519Plus));

    /* 07 00 97*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxFramesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxFramesDropped));

    /* 07 00 98*/ 
    sal_memset(&var, 0x0, sizeof(tGenOamVar));
    ok = SearchBranchLeaf(rxTmpBuf,
                          OamBranchAttribute, OamExtAttrTxBytesDropped,
                          &var);
    if ((ok != RcOk) || (var.pValue == NULL)) {
        return RcFail;
    }
    htonll(*(uint64 *) var.pValue, &(pStats->OamExtAttrTxBytesDropped));
    return ret;
}

int 
TkExtOamGetPortAllStats(uint8 pathId,uint8 linkId, uint8 port, 
    TkStatAll *stats)
{
    BufInfo bufInfo;
    Bool    ok;
    uint32  rxLen;
    uint8   ret;
    tGenOamVar var;
    uint8 buf[4] = {0x00};
    int id = 0;
    uint64 stats_tmp[STATSID_NUM+1] = {0x0};
    uint32 count_32;
    uint16 count_16;
    char *rxBuff;
    

    if(NULL == stats){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if(0 != linkId){
        return RcBadParam;
    }

    rxBuff = (char *) TkOamMemGet(pathId);
    if (NULL == rxBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return RcNoResource;;
    }
    
    if(TkOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK){
        TkOamMemPut(pathId, (void *)rxBuff);
        return RcNoResource;
    }
    
    Tk2BufU16(buf, port);
    ok = AddOamTlv(&bufInfo,OamBranchNameBinding,OamNamePhyName,2,buf);

    id = 0;

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff != StatIdToAttrLeaf[id]){
            ok = ok && OamAddAttrLeaf(&bufInfo, StatIdToAttrLeaf[id]);
        }
        id++;
    }

    ret = TxOamDeliver (pathId,linkId, &bufInfo, (uint8 *)rxBuff, &rxLen);

    if(RcOk != ret){
        TkOamMemPut(pathId, (void *)rxBuff);
        return ret;
    }
    
    id = 0;

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff == StatIdToAttrLeaf[id]){
            id++;
            continue;
        }
            
        var.pValue = NULL;
        sal_memset(&var,0x0,sizeof(tGenOamVar));

        ok = SearchBranchLeaf(rxBuff, OamBranchAttribute, StatIdToAttrLeaf[id], 
            &var);
        if(RcNotProvisioned == ok){
            stats_tmp[id] = -1;/*packed -1*/;
        }else if(var.Width == sizeof(uint64)){
            htonll(*(uint64 *)var.pValue,&stats_tmp[id]);    
        }else if(var.Width == sizeof(uint32)){
            sal_memcpy(&count_32,var.pValue,sizeof(uint32));
            stats_tmp[id] = soc_ntohl(count_32);    
        }else if(var.Width == sizeof(uint16)){
            sal_memcpy(&count_16,var.pValue,sizeof(uint16));
            stats_tmp[id] = soc_ntohs(count_16);    
        }else{
            TkDbgPrintf(("id = %d\n",id));
            TkDbgPrintf(("%04x ",StatIdToAttrLeaf[id]));
            TkOamMemPut(pathId, (void *)rxBuff);
            TkDbgTrace(TkDbgErrorEnable);
        }
    id++;
    }

    id = 0; 

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff == StatIdToAttrLeaf[id]){
            id++;
            continue;
        }

#define TK_STATS(x,id)  case x:stats->x = stats_tmp[id];break;
    
        switch(StatIdToAttrLeaf[id]){
            TK_STATS(OamAttrMacOctetsRxOk,id);
            TK_STATS(OamAttrMacFramesRxOk,id);
            TK_STATS(OamExtAttrRxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesRxOk,id);
            TK_STATS(OamAttrMacBcastFramesRxOk,id);
            TK_STATS(OamAttrMacFcsErr,id);
            TK_STATS(OamAttrOamEmulCrc8Err,id);
            TK_STATS(OamAttrPhySymbolErrDuringCarrier,id);
            TK_STATS(OamExtAttrRxFrameTooShort,id);
            TK_STATS(OamAttrMacFrameTooLong,id);
            TK_STATS(OamAttrMacInRangeLenErr,id);
            TK_STATS(OamAttrMacAlignErr,id);
            TK_STATS(OamExtAttrRxFrame64,id);
            TK_STATS(OamExtAttrRxFrame65_127,id);
            TK_STATS(OamExtAttrRxFrame128_255,id);
            TK_STATS(OamExtAttrRxFrame256_511,id);
            TK_STATS(OamExtAttrRxFrame512_1023,id);
            TK_STATS(OamExtAttrRxFrame1024_1518,id);
            TK_STATS(OamExtAttrRxFrame1519Plus,id);
            TK_STATS(OamExtAttrRxBytesDropped,id);
            TK_STATS(OamExtAttrRxFramesDropped,id);
            TK_STATS(OamExtAttrRxBytesDelayed,id);
            TK_STATS(OamExtAttrRxDelay,id);
            TK_STATS(OamAttrMacCtrlPauseRx,id);
            TK_STATS(OamAttrOamLocalErrFrameSecsEvent,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesRx,id);
            TK_STATS(OamAttrMpcpRxGate,id);
            TK_STATS(OamAttrMpcpRxRegister,id);
            TK_STATS(OamAttrFecCorrectedBlocks,id);
            TK_STATS(OamAttrFecUncorrectableBlocks,id);
            TK_STATS(OamAttrMacOctetsTxOk,id);
            TK_STATS(OamAttrMacFramesTxOk,id);
            TK_STATS(OamExtAttrTxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesTxOk,id);
            TK_STATS(OamAttrMacBcastFramesTxOk,id);
            TK_STATS(OamAttrMacSingleCollFrames,id);
            TK_STATS(OamAttrMacMultipleCollFrames,id);
            TK_STATS(OamAttrMacLateCollisions,id);
            TK_STATS(OamAttrMacExcessiveCollisions,id);
            TK_STATS(OamExtAttrTxFrame64,id);
            TK_STATS(OamExtAttrTxFrame65_127,id);
            TK_STATS(OamExtAttrTxFrame128_255,id);
            TK_STATS(OamExtAttrTxFrame256_511,id);
            TK_STATS(OamExtAttrTxFrame512_1023,id);
            TK_STATS(OamExtAttrTxFrame1024_1518,id);
            TK_STATS(OamExtAttrTxFrame1519Plus,id);
            TK_STATS(OamExtAttrTxBytesDropped,id);
            TK_STATS(OamExtAttrTxFramesDropped,id);
            TK_STATS(OamExtAttrTxBytesDelayed,id);
            TK_STATS(OamExtAttrTxDelay,id);
            TK_STATS(OamExtAttrTxBytesUnused,id);
            TK_STATS(OamAttrMacCtrlPauseTx,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesTx,id);
            TK_STATS(OamAttrMpcpTxRegAck,id);
            TK_STATS(OamAttrMpcpTxRegRequest,id);
            TK_STATS(OamAttrMpcpTxReport,id);
            default:
                TkDbgTrace(TkDbgErrorEnable);
                break;
        }
#undef   TK_STATS  
    id++;
    }
    TkOamMemPut(pathId, (void *)rxBuff);
    return ret;
}

int 
TkExtOamGetLinkAllStats(uint8 pathId, uint8 linkId,uint8 linkNum, 
    TkStatAll *stats)
{
    BufInfo bufInfo;
    Bool    ok;
    uint32  rxLen;
    uint8   ret;
    tGenOamVar var;
    uint8 buf[4] = {0x00};
    int id = 0;
    uint64 stats_tmp[STATSID_NUM+1] = {0x0};
    uint32 count_32;
    uint16 count_16;
    char *rxBuff;

    if(NULL == stats){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if(0 != linkId){
        return RcBadParam;
    }

    rxBuff = (char *) TkOamMemGet(pathId);
    if (NULL == rxBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return RcNoResource;;
    }
    
    if(TkOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK){
        TkOamMemPut(pathId, (void *)rxBuff);
        return RcNoResource;
    }
    
    Tk2BufU16(buf, linkNum);
    ok = AddOamTlv(&bufInfo,OamBranchNameBinding,OamNameMacName,2,buf);

    id = 0;

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff != StatIdToAttrLeaf[id]){
            ok = ok && OamAddAttrLeaf(&bufInfo, StatIdToAttrLeaf[id]);
        }
        id++;
    }

    ret = TxOamDeliver (pathId,linkId, &bufInfo, (uint8 *)rxBuff, &rxLen);

    if(RcOk != ret){
        TkOamMemPut(pathId, (void *)rxBuff);
        return ret;
    }
    
    id = 0;

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff == StatIdToAttrLeaf[id]){
            id++;
            continue;
        }
            
        var.pValue = NULL;
        sal_memset(&var,0x0,sizeof(tGenOamVar));

        ok = SearchBranchLeaf(rxBuff, OamBranchAttribute, StatIdToAttrLeaf[id], 
            &var);
        if(RcNotProvisioned == ok){
            stats_tmp[id] = -1;/*packed -1*/;
        }else if(var.Width == sizeof(uint64)){
            htonll(*(uint64 *)var.pValue,&stats_tmp[id]);    
        }else if(var.Width == sizeof(uint32)){
            sal_memcpy(&count_32,var.pValue,sizeof(uint32));
            stats_tmp[id] = soc_ntohl(count_32);    
        }else if(var.Width == sizeof(uint16)){
            sal_memcpy(&count_16,var.pValue,sizeof(uint16));
            stats_tmp[id] = soc_ntohs(count_16);    
        }else{
            TkOamMemPut(pathId, (void *)rxBuff);
            TkDbgTrace(TkDbgErrorEnable);
        }
    id++;
    }

    id = 0; 

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff == StatIdToAttrLeaf[id]){
            id++;
            continue;
        }

#define TK_STATS(x,id)  case x:stats->x = stats_tmp[id];break;
    
        switch(StatIdToAttrLeaf[id]){
            TK_STATS(OamAttrMacOctetsRxOk,id);
            TK_STATS(OamAttrMacFramesRxOk,id);
            TK_STATS(OamExtAttrRxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesRxOk,id);
            TK_STATS(OamAttrMacBcastFramesRxOk,id);
            TK_STATS(OamAttrMacFcsErr,id);
            TK_STATS(OamAttrOamEmulCrc8Err,id);
            TK_STATS(OamAttrPhySymbolErrDuringCarrier,id);
            TK_STATS(OamExtAttrRxFrameTooShort,id);
            TK_STATS(OamAttrMacFrameTooLong,id);
            TK_STATS(OamAttrMacInRangeLenErr,id);
            TK_STATS(OamAttrMacAlignErr,id);
            TK_STATS(OamExtAttrRxFrame64,id);
            TK_STATS(OamExtAttrRxFrame65_127,id);
            TK_STATS(OamExtAttrRxFrame128_255,id);
            TK_STATS(OamExtAttrRxFrame256_511,id);
            TK_STATS(OamExtAttrRxFrame512_1023,id);
            TK_STATS(OamExtAttrRxFrame1024_1518,id);
            TK_STATS(OamExtAttrRxFrame1519Plus,id);
            TK_STATS(OamExtAttrRxBytesDropped,id);
            TK_STATS(OamExtAttrRxFramesDropped,id);
            TK_STATS(OamExtAttrRxBytesDelayed,id);
            TK_STATS(OamExtAttrRxDelay,id);
            TK_STATS(OamAttrMacCtrlPauseRx,id);
            TK_STATS(OamAttrOamLocalErrFrameSecsEvent,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesRx,id);
            TK_STATS(OamAttrMpcpRxGate,id);
            TK_STATS(OamAttrMpcpRxRegister,id);
            TK_STATS(OamAttrFecCorrectedBlocks,id);
            TK_STATS(OamAttrFecUncorrectableBlocks,id);
            TK_STATS(OamAttrMacOctetsTxOk,id);
            TK_STATS(OamAttrMacFramesTxOk,id);
            TK_STATS(OamExtAttrTxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesTxOk,id);
            TK_STATS(OamAttrMacBcastFramesTxOk,id);
            TK_STATS(OamAttrMacSingleCollFrames,id);
            TK_STATS(OamAttrMacMultipleCollFrames,id);
            TK_STATS(OamAttrMacLateCollisions,id);
            TK_STATS(OamAttrMacExcessiveCollisions,id);
            TK_STATS(OamExtAttrTxFrame64,id);
            TK_STATS(OamExtAttrTxFrame65_127,id);
            TK_STATS(OamExtAttrTxFrame128_255,id);
            TK_STATS(OamExtAttrTxFrame256_511,id);
            TK_STATS(OamExtAttrTxFrame512_1023,id);
            TK_STATS(OamExtAttrTxFrame1024_1518,id);
            TK_STATS(OamExtAttrTxFrame1519Plus,id);
            TK_STATS(OamExtAttrTxBytesDropped,id);
            TK_STATS(OamExtAttrTxFramesDropped,id);
            TK_STATS(OamExtAttrTxBytesDelayed,id);
            TK_STATS(OamExtAttrTxDelay,id);
            TK_STATS(OamExtAttrTxBytesUnused,id);
            TK_STATS(OamAttrMacCtrlPauseTx,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesTx,id);
            TK_STATS(OamAttrMpcpTxRegAck,id);
            TK_STATS(OamAttrMpcpTxRegRequest,id);
            TK_STATS(OamAttrMpcpTxReport,id);
        default:
            TkDbgTrace(TkDbgErrorEnable);
            break;
        }
#undef   TK_STATS  
    id++;
    }
    TkOamMemPut(pathId, (void *)rxBuff);
    return ret;
} 

int 
TkExtOamGetQueueAllStats(uint8 pathId,uint8 linkId, uint16 portNum, 
    uint16 linkNum, uint16 queueNum, TkStatAll *stats)
{
    BufInfo bufInfo;
    Bool    ok;
    uint32  rxLen;
    uint8   ret;
    tGenOamVar var;
    OamObjIndex index;
    int id = 0;
    uint64 stats_tmp[STATSID_NUM+1] = {0x0};
    uint32 count_32;
    uint16 count_16;
    char *rxBuff;

    if(linkId != 0){
        return ERROR;
    }

    if((0 != portNum && 1 != portNum && 2 != portNum) ||
        (queueNum > 19) || (NULL == stats)){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;    
    }
    
    rxBuff = (char *) TkOamMemGet(pathId);
    if (NULL == rxBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return RcNoResource;;
    }
    
    if(TkOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK){
        TkOamMemPut(pathId, (void *)rxBuff);
        return RcNoResource;
    }

    index.queueId.port = soc_htons((uint16) portNum);
    index.queueId.link = soc_htons((uint16) linkNum);
    index.queueId.queue = soc_htons((uint16) queueNum);
    
    ok = AddOamTlv(&bufInfo,OamBranchNameBinding,OamNameQueueName,
        sizeof(OamNameQueue),(const uint8 *)&index);

    id = 0;

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff != StatIdToAttrLeaf[id]){
            ok = ok && OamAddAttrLeaf(&bufInfo, StatIdToAttrLeaf[id]);
        }
        id++;
    }

    ret = TxOamDeliver (pathId,linkId, &bufInfo, (uint8 *)rxBuff, &rxLen);

    if(RcOk != ret){
        TkOamMemPut(pathId, (void *)rxBuff);
        return ret;
    }
    
    id = 0;

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff == StatIdToAttrLeaf[id]){
            id++;
            continue;
        }
            
        var.pValue = NULL;
        sal_memset(&var,0x0,sizeof(tGenOamVar));

        ok = SearchBranchLeaf(rxBuff, OamBranchAttribute, StatIdToAttrLeaf[id], 
            &var);
        if(RcNotProvisioned == ok){
            stats_tmp[id] = -1;/*packed -1*/;
        }else if(var.Width == sizeof(uint64)){
            htonll(*(uint64 *)var.pValue,&stats_tmp[id]);    
        }else if(var.Width == sizeof(uint32)){
            sal_memcpy(&count_32,var.pValue,sizeof(uint32));
            stats_tmp[id] = soc_ntohl(count_32);    
        }else if(var.Width == sizeof(uint16)){
            sal_memcpy(&count_16,var.pValue,sizeof(uint16));
            stats_tmp[id] = soc_ntohs(count_16);    
        }else{
            TkDbgPrintf(("id = %d\n",id));
            TkDbgPrintf(("%04x ",StatIdToAttrLeaf[id]));
            TkOamMemPut(pathId, (void *)rxBuff);
            TkDbgTrace(TkDbgErrorEnable);
        }
    id++;
    }

    id = 0; 

    while(OamAttrMpcpRxRegister != StatIdToAttrLeaf[id]){
        if(0xffff == StatIdToAttrLeaf[id]){
            id++;
            continue;
        }

#define TK_STATS(x,id)  case x:stats->x = stats_tmp[id];break;
    
        switch(StatIdToAttrLeaf[id]){
            TK_STATS(OamAttrMacOctetsRxOk,id);
            TK_STATS(OamAttrMacFramesRxOk,id);
            TK_STATS(OamExtAttrRxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesRxOk,id);
            TK_STATS(OamAttrMacBcastFramesRxOk,id);
            TK_STATS(OamAttrMacFcsErr,id);
            TK_STATS(OamAttrOamEmulCrc8Err,id);
            TK_STATS(OamAttrPhySymbolErrDuringCarrier,id);
            TK_STATS(OamExtAttrRxFrameTooShort,id);
            TK_STATS(OamAttrMacFrameTooLong,id);
            TK_STATS(OamAttrMacInRangeLenErr,id);
            TK_STATS(OamAttrMacAlignErr,id);
            TK_STATS(OamExtAttrRxFrame64,id);
            TK_STATS(OamExtAttrRxFrame65_127,id);
            TK_STATS(OamExtAttrRxFrame128_255,id);
            TK_STATS(OamExtAttrRxFrame256_511,id);
            TK_STATS(OamExtAttrRxFrame512_1023,id);
            TK_STATS(OamExtAttrRxFrame1024_1518,id);
            TK_STATS(OamExtAttrRxFrame1519Plus,id);
            TK_STATS(OamExtAttrRxBytesDropped,id);
            TK_STATS(OamExtAttrRxFramesDropped,id);
            TK_STATS(OamExtAttrRxBytesDelayed,id);
            TK_STATS(OamExtAttrRxDelay,id);
            TK_STATS(OamAttrMacCtrlPauseRx,id);
            TK_STATS(OamAttrOamLocalErrFrameSecsEvent,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesRx,id);
            TK_STATS(OamAttrMpcpRxGate,id);
            TK_STATS(OamAttrMpcpRxRegister,id);
            TK_STATS(OamAttrFecCorrectedBlocks,id);
            TK_STATS(OamAttrFecUncorrectableBlocks,id);
            TK_STATS(OamAttrMacOctetsTxOk,id);
            TK_STATS(OamAttrMacFramesTxOk,id);
            TK_STATS(OamExtAttrTxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesTxOk,id);
            TK_STATS(OamAttrMacBcastFramesTxOk,id);
            TK_STATS(OamAttrMacSingleCollFrames,id);
            TK_STATS(OamAttrMacMultipleCollFrames,id);
            TK_STATS(OamAttrMacLateCollisions,id);
            TK_STATS(OamAttrMacExcessiveCollisions,id);
            TK_STATS(OamExtAttrTxFrame64,id);
            TK_STATS(OamExtAttrTxFrame65_127,id);
            TK_STATS(OamExtAttrTxFrame128_255,id);
            TK_STATS(OamExtAttrTxFrame256_511,id);
            TK_STATS(OamExtAttrTxFrame512_1023,id);
            TK_STATS(OamExtAttrTxFrame1024_1518,id);
            TK_STATS(OamExtAttrTxFrame1519Plus,id);
            TK_STATS(OamExtAttrTxBytesDropped,id);
            TK_STATS(OamExtAttrTxFramesDropped,id);
            TK_STATS(OamExtAttrTxBytesDelayed,id);
            TK_STATS(OamExtAttrTxDelay,id);
            TK_STATS(OamExtAttrTxBytesUnused,id);
            TK_STATS(OamAttrMacCtrlPauseTx,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesTx,id);
            TK_STATS(OamAttrMpcpTxRegAck,id);
            TK_STATS(OamAttrMpcpTxRegRequest,id);
            TK_STATS(OamAttrMpcpTxReport,id);
        default:
            TkDbgTrace(TkDbgErrorEnable);
            break;
        }
#undef   TK_STATS  
    id++;
    }
    TkOamMemPut(pathId, (void *)rxBuff);
    return ret;
}

int 
TkExtOamGetPonStatsGroup(uint8 pathId,uint8 linkId, TkPonStatsGroup *stats)
{
    BufInfo bufInfo;
    Bool    ok;
    uint32  rxLen;
    uint8   ret;
    tGenOamVar var;
    uint8 buf[4] = {0x00};
    int id = 0;
    uint64 stats_tmp[sizeof(TkPonStatsGroup)/8+1] = {0x0};
    uint32 count_32;
    uint16 count_16;
    char *rxBuff;
    

    if(NULL == stats){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if(0 != linkId){
        return RcBadParam;
    }

    rxBuff = (char *) TkOamMemGet(pathId);
    if (NULL == rxBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return RcNoResource;;
    }
    
    if(TkOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK){
        TkOamMemPut(pathId, (void *)rxBuff);
        return RcNoResource;
    }
    
    Tk2BufU16(buf, 0);
    ok = AddOamTlv(&bufInfo,OamBranchNameBinding,OamNamePhyName,2,buf);

    id = 0;

    while(0xFFFF != ponStatsAttrLeafGroup[id]){
        ok = ok && OamAddAttrLeaf(&bufInfo, ponStatsAttrLeafGroup[id]);
        id++;
    }

    ret = TxOamDeliver (pathId,linkId, &bufInfo, (uint8 *)rxBuff, &rxLen);

    if(RcOk != ret){
        TkOamMemPut(pathId, (void *)rxBuff);
        return ret;
    }
    
    id = 0;

    while(0xFFFF != ponStatsAttrLeafGroup[id]){
        var.pValue = NULL;
        sal_memset(&var,0x0,sizeof(tGenOamVar));

        ok = SearchBranchLeaf(rxBuff, OamBranchAttribute, ponStatsAttrLeafGroup[id], 
            &var);
        if(RcNotProvisioned == ok){
            stats_tmp[id] = -1;/*packed -1*/;
        }else if(var.Width == sizeof(uint64)){
            htonll(*(uint64 *)var.pValue,&stats_tmp[id]);    
        }else if(var.Width == sizeof(uint32)){
            sal_memcpy(&count_32,var.pValue,sizeof(uint32));
            stats_tmp[id] = soc_ntohl(count_32);    
        }else if(var.Width == sizeof(uint16)){
            sal_memcpy(&count_16,var.pValue,sizeof(uint16));
            stats_tmp[id] = soc_ntohs(count_16);    
        }else{
        	stats_tmp[id] = -1;
            TkDbgTrace(TkDbgErrorEnable);
        }
    id++;
    }

    id = 0; 

    while(0xFFFF != ponStatsAttrLeafGroup[id]){
#define TK_STATS(x,id)  case x:stats->x = stats_tmp[id];break;
    
        switch(ponStatsAttrLeafGroup[id]){
            TK_STATS(OamAttrMacOctetsRxOk,id);
            TK_STATS(OamAttrMacFramesRxOk,id);
            TK_STATS(OamExtAttrRxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesRxOk,id);
            TK_STATS(OamAttrMacBcastFramesRxOk,id);
            TK_STATS(OamAttrMacFcsErr,id);
            TK_STATS(OamAttrOamEmulCrc8Err,id);
            TK_STATS(OamAttrPhySymbolErrDuringCarrier,id);
            TK_STATS(OamExtAttrRxFrameTooShort,id);
            TK_STATS(OamExtAttrRxFrame64,id);
            TK_STATS(OamExtAttrRxFrame65_127,id);
            TK_STATS(OamExtAttrRxFrame128_255,id);
            TK_STATS(OamExtAttrRxFrame256_511,id);
            TK_STATS(OamExtAttrRxFrame512_1023,id);
            TK_STATS(OamExtAttrRxFrame1024_1518,id);
            TK_STATS(OamExtAttrRxFrame1519Plus,id);
            TK_STATS(OamExtAttrRxBytesDropped,id);
            TK_STATS(OamExtAttrRxFramesDropped,id);
            TK_STATS(OamExtAttrRxBytesDelayed,id);
            TK_STATS(OamExtAttrRxDelay,id);
            TK_STATS(OamAttrOamLocalErrFrameSecsEvent,id);
            TK_STATS(OamAttrFecCorrectedBlocks,id);
            TK_STATS(OamAttrFecUncorrectableBlocks,id);  
            TK_STATS(OamAttrMacOctetsTxOk,id);
            TK_STATS(OamAttrMacFramesTxOk,id);
            TK_STATS(OamExtAttrTxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesTxOk,id);
            TK_STATS(OamAttrMacBcastFramesTxOk,id);
            TK_STATS(OamExtAttrTxFrame64,id);
            TK_STATS(OamExtAttrTxFrame65_127,id);
            TK_STATS(OamExtAttrTxFrame128_255,id);
            TK_STATS(OamExtAttrTxFrame256_511,id);
            TK_STATS(OamExtAttrTxFrame512_1023,id);
            TK_STATS(OamExtAttrTxFrame1024_1518,id);
            TK_STATS(OamExtAttrTxFrame1519Plus,id);
            TK_STATS(OamExtAttrTxBytesDropped,id);
            TK_STATS(OamExtAttrTxFramesDropped,id);
            TK_STATS(OamExtAttrTxBytesDelayed,id);
            TK_STATS(OamExtAttrTxDelay,id);
            TK_STATS(OamExtAttrTxBytesUnused,id);
            default:
                TkDbgTrace(TkDbgErrorEnable);
                break;
        }
#undef   TK_STATS  
    id++;
    }
    TkOamMemPut(pathId, (void *)rxBuff);
    return ret;
}


int 
TkExtOamGetUniStatsGroup(uint8 pathId,uint8 linkId, uint8 port, 
    TkUniStatsGroup *stats)
{
    BufInfo bufInfo;
    Bool    ok;
    uint32  rxLen;
    uint8   ret;
    tGenOamVar var;
    uint8 buf[4] = {0x00};
    int id = 0;
    uint64 stats_tmp[sizeof(TkUniStatsGroup)/8+1] = {0x0};
    uint32 count_32;
    uint16 count_16;
    char *rxBuff;
    

    if(NULL == stats){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if(0 != linkId){
        return RcBadParam;
    }

    rxBuff = (char *) TkOamMemGet(pathId);
    if (NULL == rxBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return RcNoResource;;
    }
    
    if(TkOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK){
        TkOamMemPut(pathId, (void *)rxBuff);
        return RcNoResource;
    }
    
    Tk2BufU16(buf, port);
    ok = AddOamTlv(&bufInfo,OamBranchNameBinding,OamNamePhyName,2,buf);

    id = 0;

    while(0xFFFF != uniStatsAttrLeafGroup[id]){
        ok = ok && OamAddAttrLeaf(&bufInfo, uniStatsAttrLeafGroup[id]);
        id++;
    }

    ret = TxOamDeliver (pathId,linkId, &bufInfo, (uint8 *)rxBuff, &rxLen);

    if(RcOk != ret){
        TkOamMemPut(pathId, (void *)rxBuff);
        return ret;
    }
    
    id = 0;

    while(0xFFFF != uniStatsAttrLeafGroup[id]){
        var.pValue = NULL;
        sal_memset(&var,0x0,sizeof(tGenOamVar));

        ok = SearchBranchLeaf(rxBuff, OamBranchAttribute, uniStatsAttrLeafGroup[id], 
            &var);
        if(RcNotProvisioned == ok){
            stats_tmp[id] = -1;/*packed -1*/;
        }else if(var.Width == sizeof(uint64)){
            htonll(*(uint64 *)var.pValue,&stats_tmp[id]);    
        }else if(var.Width == sizeof(uint32)){
            sal_memcpy(&count_32,var.pValue,sizeof(uint32));
            stats_tmp[id] = soc_ntohl(count_32);    
        }else if(var.Width == sizeof(uint16)){
            sal_memcpy(&count_16,var.pValue,sizeof(uint16));
            stats_tmp[id] = soc_ntohs(count_16);    
        }else{
        	stats_tmp[id] = -1;
            TkDbgTrace(TkDbgErrorEnable);
        }
    id++;
    }

    id = 0; 

    while(0xFFFF != uniStatsAttrLeafGroup[id]){
#define TK_STATS(x,id)  case x:stats->x = stats_tmp[id];break;
    
        switch(uniStatsAttrLeafGroup[id]){
            TK_STATS(OamAttrMacOctetsRxOk,id);
            TK_STATS(OamAttrMacFramesRxOk,id);
            TK_STATS(OamExtAttrRxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesRxOk,id);
            TK_STATS(OamAttrMacBcastFramesRxOk,id);
            TK_STATS(OamAttrMacFcsErr,id);
            TK_STATS(OamExtAttrRxFrameTooShort,id);
            TK_STATS(OamAttrMacFrameTooLong,id);
            TK_STATS(OamAttrMacInRangeLenErr,id);
            TK_STATS(OamAttrMacAlignErr,id);
            TK_STATS(OamExtAttrRxFrame64,id);
            TK_STATS(OamExtAttrRxFrame65_127,id);
            TK_STATS(OamExtAttrRxFrame128_255,id);
            TK_STATS(OamExtAttrRxFrame256_511,id);
            TK_STATS(OamExtAttrRxFrame512_1023,id);
            TK_STATS(OamExtAttrRxFrame1024_1518,id);
            TK_STATS(OamExtAttrRxFrame1519Plus,id);
            TK_STATS(OamAttrMacCtrlPauseRx,id);
            TK_STATS(OamExtAttrRxBytesDropped,id);
            
            TK_STATS(OamAttrMacOctetsTxOk,id);
            TK_STATS(OamAttrMacFramesTxOk,id);
            TK_STATS(OamExtAttrTxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesTxOk,id);
            TK_STATS(OamAttrMacBcastFramesTxOk,id);
            TK_STATS(OamAttrMacSingleCollFrames,id);
            TK_STATS(OamAttrMacMultipleCollFrames,id);
            TK_STATS(OamAttrMacLateCollisions,id);
            TK_STATS(OamAttrMacExcessiveCollisions,id);
            TK_STATS(OamExtAttrTxFrame64,id);
            TK_STATS(OamExtAttrTxFrame65_127,id);
            TK_STATS(OamExtAttrTxFrame128_255,id);
            TK_STATS(OamExtAttrTxFrame256_511,id);
            TK_STATS(OamExtAttrTxFrame512_1023,id);
            TK_STATS(OamExtAttrTxFrame1024_1518,id);
            TK_STATS(OamExtAttrTxFrame1519Plus,id);
            TK_STATS(OamExtAttrTxBytesDropped,id);
            TK_STATS(OamExtAttrTxFramesDropped,id);
            TK_STATS(OamAttrMacCtrlPauseTx,id);
            default:
                TkDbgTrace(TkDbgErrorEnable);
                break;
        }
#undef   TK_STATS  
    id++;
    }
    TkOamMemPut(pathId, (void *)rxBuff);
    return ret;
}


int 
TkExtOamGetLlidStatsGroup(uint8 pathId,uint8 linkId, uint16 llid, 
    TkLlidStatsGroup *stats)
{
    BufInfo bufInfo;
    Bool    ok;
    uint32  rxLen;
    uint8   ret;
    tGenOamVar var;
    uint8 buf[4] = {0x00};
    int id = 0;
    uint64 stats_tmp[sizeof(TkLlidStatsGroup)/8+1] = {0x0};
    uint32 count_32;
    uint16 count_16;
    char *rxBuff;
    

    if(NULL == stats){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if(0 != linkId){
        return RcBadParam;
    }

    rxBuff = (char *) TkOamMemGet(pathId);
    if (NULL == rxBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return RcNoResource;;
    }
    
    if(TkOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK){
        TkOamMemPut(pathId, (void *)rxBuff);
        return RcNoResource;
    }
    
    Tk2BufU16(buf, llid);
    ok = AddOamTlv(&bufInfo,OamBranchNameBinding,OamNameMacName,2,buf);

    id = 0;

    while(0xFFFF != llidStatsAttrLeafGroup[id]){
        ok = ok && OamAddAttrLeaf(&bufInfo, llidStatsAttrLeafGroup[id]);
        id++;
    }

    ret = TxOamDeliver (pathId,linkId, &bufInfo, (uint8 *)rxBuff, &rxLen);

    if(RcOk != ret){
        TkOamMemPut(pathId, (void *)rxBuff);
        return ret;
    }
    
    id = 0;

    while(0xFFFF != llidStatsAttrLeafGroup[id]){
        var.pValue = NULL;
        sal_memset(&var,0x0,sizeof(tGenOamVar));

        ok = SearchBranchLeaf(rxBuff, OamBranchAttribute, llidStatsAttrLeafGroup[id], 
            &var);
        if(RcNotProvisioned == ok){
            stats_tmp[id] = -1;/*packed -1*/;
        }else if(var.Width == sizeof(uint64)){
            htonll(*(uint64 *)var.pValue,&stats_tmp[id]);    
        }else if(var.Width == sizeof(uint32)){
            sal_memcpy(&count_32,var.pValue,sizeof(uint32));
            stats_tmp[id] = soc_ntohl(count_32);    
        }else if(var.Width == sizeof(uint16)){
            sal_memcpy(&count_16,var.pValue,sizeof(uint16));
            stats_tmp[id] = soc_ntohs(count_16);    
        }else{
        	stats_tmp[id] = -1;
            TkDbgTrace(TkDbgErrorEnable);
        }
    id++;
    }

    id = 0; 

    while(0xFFFF != llidStatsAttrLeafGroup[id]){
#define TK_STATS(x,id)  case x:stats->x = stats_tmp[id];break;
    
        switch(llidStatsAttrLeafGroup[id]){
            TK_STATS(OamAttrMacOctetsRxOk,id);
            TK_STATS(OamAttrMacFramesRxOk,id);
            TK_STATS(OamExtAttrRxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesRxOk,id);
            TK_STATS(OamAttrMacBcastFramesRxOk,id);
            TK_STATS(OamAttrMacFcsErr,id);
            TK_STATS(OamExtAttrRxFrameTooShort,id);
            TK_STATS(OamExtAttrRxFrame64,id);
            TK_STATS(OamExtAttrRxFrame65_127,id);
            TK_STATS(OamExtAttrRxFrame128_255,id);
            TK_STATS(OamExtAttrRxFrame256_511,id);
            TK_STATS(OamExtAttrRxFrame512_1023,id);
            TK_STATS(OamExtAttrRxFrame1024_1518,id);
            TK_STATS(OamExtAttrRxFrame1519Plus,id);
            TK_STATS(OamExtAttrRxBytesDropped,id);
            TK_STATS(OamExtAttrRxFramesDropped,id);
            TK_STATS(OamExtAttrRxBytesDelayed,id);
            TK_STATS(OamExtAttrRxDelay,id);
            TK_STATS(OamAttrOamLocalErrFrameSecsEvent,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesRx,id);
            TK_STATS(OamAttrMpcpRxGate,id);
            TK_STATS(OamAttrMpcpRxRegister,id);
            
            TK_STATS(OamAttrMacOctetsTxOk,id);
            TK_STATS(OamAttrMacFramesTxOk,id);
            TK_STATS(OamExtAttrTxUnicastFrames,id);
            TK_STATS(OamAttrMacMcastFramesTxOk,id);
            TK_STATS(OamAttrMacBcastFramesTxOk,id);
            TK_STATS(OamExtAttrTxFrame64,id);
            TK_STATS(OamExtAttrTxFrame65_127,id);
            TK_STATS(OamExtAttrTxFrame128_255,id);
            TK_STATS(OamExtAttrTxFrame256_511,id);
            TK_STATS(OamExtAttrTxFrame512_1023,id);
            TK_STATS(OamExtAttrTxFrame1024_1518,id);
            TK_STATS(OamExtAttrTxFrame1519Plus,id);
            TK_STATS(OamExtAttrTxBytesDropped,id);
            TK_STATS(OamExtAttrTxFramesDropped,id);
            TK_STATS(OamExtAttrTxBytesDelayed,id);
            TK_STATS(OamExtAttrTxDelay,id);
            TK_STATS(OamExtAttrTxBytesUnused,id);
            TK_STATS(OamAttrMpcpMACCtrlFramesTx,id);
            TK_STATS(OamAttrMpcpTxRegAck,id);
            TK_STATS(OamAttrMpcpTxRegRequest,id);
            TK_STATS(OamAttrMpcpTxReport,id);
            default:
                TkDbgTrace(TkDbgErrorEnable);
                break;
        }
#undef   TK_STATS  
    id++;
    }
    TkOamMemPut(pathId, (void *)rxBuff);
    return ret;
}

int 
TkExtOamGetCosqStatsGroup(uint8 pathId,uint8 linkId, uint16 portNum, 
    uint16 linkNum, uint16 queueNum, TkCosqStatsGroup *stats)
{
    BufInfo bufInfo;
    Bool    ok;
    uint32  rxLen;
    uint8   ret;
    tGenOamVar var;
    OamObjIndex index;
    int id = 0;
    uint64 stats_tmp[sizeof(TkCosqStatsGroup)/8+1] = {0x0};
    uint32 count_32;
    uint16 count_16;
    char *rxBuff;

    if(linkId != 0){
        return ERROR;
    }

    if((0 != portNum && 1 != portNum && 2 != portNum) ||
        (queueNum > 19) || (NULL == stats)){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;    
    }
    
    rxBuff = (char *) TkOamMemGet(pathId);
    if (NULL == rxBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return RcNoResource;;
    }
    
    if(TkOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK){
        TkOamMemPut(pathId, (void *)rxBuff);
        return RcNoResource;
    }

    index.queueId.port = soc_htons((uint16) portNum);
    index.queueId.link = soc_htons((uint16) linkNum);
    index.queueId.queue = soc_htons((uint16) queueNum);
    
    ok = AddOamTlv(&bufInfo,OamBranchNameBinding,OamNameQueueName,
        sizeof(OamNameQueue),(const uint8 *)&index);

    id = 0;

    while(0xFFFF != cosqStatsAttrLeafGroup[id]){
        ok = ok && OamAddAttrLeaf(&bufInfo, cosqStatsAttrLeafGroup[id]);
        id++;
    }

    ret = TxOamDeliver (pathId,linkId, &bufInfo, (uint8 *)rxBuff, &rxLen);

    if(RcOk != ret){
        TkOamMemPut(pathId, (void *)rxBuff);
        return ret;
    }
    
    id = 0;

    while(OamAttrMpcpRxRegister != cosqStatsAttrLeafGroup[id]){
        var.pValue = NULL;
        sal_memset(&var,0x0,sizeof(tGenOamVar));

        ok = SearchBranchLeaf(rxBuff, OamBranchAttribute, cosqStatsAttrLeafGroup[id], 
            &var);
        if(RcNotProvisioned == ok){
            stats_tmp[id] = -1;/*packed -1*/;
        }else if(var.Width == sizeof(uint64)){
            htonll(*(uint64 *)var.pValue,&stats_tmp[id]);    
        }else if(var.Width == sizeof(uint32)){
            sal_memcpy(&count_32,var.pValue,sizeof(uint32));
            stats_tmp[id] = soc_ntohl(count_32);    
        }else if(var.Width == sizeof(uint16)){
            sal_memcpy(&count_16,var.pValue,sizeof(uint16));
            stats_tmp[id] = soc_ntohs(count_16);    
        }else{
            TkDbgTrace(TkDbgErrorEnable);
        }
    id++;
    }

    id = 0; 

    while(OamAttrMpcpRxRegister != cosqStatsAttrLeafGroup[id]){
#define TK_STATS(x,id)  case x:stats->x = stats_tmp[id];break;
    
        switch(cosqStatsAttrLeafGroup[id]){
            TK_STATS(OamAttrMacOctetsRxOk,id);
            TK_STATS(OamAttrMacOctetsTxOk,id);
            TK_STATS(OamAttrMacFramesRxOk,id);
            TK_STATS(OamAttrMacFramesTxOk,id);
            TK_STATS(OamExtAttrRxBytesDropped,id);
            TK_STATS(OamExtAttrTxBytesDropped,id);
            TK_STATS(OamExtAttrRxFramesDropped,id);
            TK_STATS(OamExtAttrTxFramesDropped,id);
            TK_STATS(OamExtAttrRxBytesDelayed,id);
            TK_STATS(OamExtAttrTxBytesDelayed,id);
            TK_STATS(OamExtAttrRxDelay,id);
            TK_STATS(OamExtAttrTxDelay,id);
        default:
            TkDbgTrace(TkDbgErrorEnable);
            break;
        }
#undef   TK_STATS  
    id++;
    }
    TkOamMemPut(pathId, (void *)rxBuff);
    return ret;
}



