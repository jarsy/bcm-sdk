/*
 * $Id: TkOamApiCommon.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOamApiCommon.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Ethernet.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkNetIf.h>
#include <soc/ea/tk371x/TkInit.h>  
#include <soc/ea/tk371x/TkDebug.h>

VlanLinkConfig  gTagLinkCfg[MAX_NUM_OF_PON_CHIP+1];
extern MacAddr  OamMcMacAddr;

/*
 * GetSourceForFlag: Gets the source associated with the provided OAM
 * flag
 *  /
 *  / Parameters:
 *  / \param flag OAM flags
 *  / 
 *  / \return 
 *  / The source (interface + link) associated with the given flag
 */ 

uint8
GetSourceForFlag(uint16 flags)
{
    uint8           source;
    source =
        (uint8) ((flags & OamReservedFlagMask) >> OamReservedFlagShift);
    source =
        ((source & OamFlagSrcIfMask) << OamFlagSrcIfShift) | (source >>
                                                              OamFlagLinkShift);
    return source;
}                  


void
AttachOamFlag(uint8 source, uint16 * flags)
{
    *flags = ((*flags) & OamFlagMask) |
        ((source & OnuHostIfLinkMask) <<
         OamFlagLinkShift | (source >>
                             OnuHostIfPhyIfSft) <<
         OamFlagSrcIfShift | OnuOamPass) << OamReservedFlagShift;
}

void
AttachOamFlagNoPass(uint8 source, uint16 * flags)
{
    *flags = ((*flags) & OamFlagMask) |
        ((source & OnuHostIfLinkMask) <<
         OamFlagLinkShift | (source >>
                             OnuHostIfPhyIfSft) <<
         OamFlagSrcIfShift) << OamReservedFlagShift;
}


uint8          *
OamFillExtHeader(uint16 flags, const IeeeOui * oui, uint8 * TxFrame)
{
    OamOuiVendorExt *tx;
    uint16          ethHeadLen;

    ethHeadLen = sizeof(EthernetFrame);
    tx = (OamOuiVendorExt *) (TxFrame + ethHeadLen);
    sal_memcpy(&tx->oui, oui, sizeof(IeeeOui));

    tx->common.subtype = OamSlowProtocolSubtype;
    tx->common.flags = soc_htons(flags);
    tx->common.opcode = OamOpVendorOui;

    return ((uint8 *) (tx + 1));
}


int32
OamEthSend(uint8 pathId, MacAddr * dstAddr, uint8 * pDataBuf, uint32 dataLen)
{
    MacAddr         srcAddress;

    if (dataLen > MAX_MSG_LEN) {
        if (TkDbgLevelIsSet(TkDbgErrorEnable)) {
            TkDbgPrintf(("\r\nThe packet(Len: %d) to be sent is too long!\n", 
                (int) dataLen));
        }
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    TkOsGetIfMac((uint8 *) &srcAddress);

    sal_memcpy((void *)((EthernetFrame *)pDataBuf)->da.u8, (void *)dstAddr->u8, 
        sizeof(uint8)*6);
    sal_memcpy((void *)((EthernetFrame *)pDataBuf)->sa.u8, (void *)&srcAddress.u8, 
        sizeof(uint8)*6);
    ((VlanTaggedEthernetFrame *) pDataBuf)->Type = soc_htons(EthertypeOam);

    if (!TkOsDataTx(pathId, pDataBuf, (dataLen < 60) ? 60 : dataLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
    return OK;
}

/*
 * send TK extension OAM message(Get/Set) and wait for response from the
 * ONU 
 */
int
TkOamRequest(uint8 pathId, uint8 linkId,
             OamFrame * txBuf, uint32 txLen, OamPdu * rxBuf,
             uint32 * pRxLen)
{
    int32           msgLen;

    if ((NULL == txBuf) || (NULL == rxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (txLen + sizeof(EthernetFrame) < 60) {
        bzero((uint8 *) txBuf + txLen +
              sizeof(EthernetFrame), 60 - (txLen + sizeof(EthernetFrame)));
    }

    sal_sem_take(gTagLinkCfg[pathId].semId, sal_sem_FOREVER);

    /* we need clear the message blocking in the message queue*/
    sal_msg_clear(gTagLinkCfg[pathId].resMsgQid);

    if (OK == OamEthSend(pathId, &OamMcMacAddr,
                         (uint8 *) txBuf, txLen + sizeof(EthernetFrame))) {
        if (TkDbgLevelIsSet(TkDbgLogTraceEnable)) {
            TkDbgPrintf(("\r\nsent to ONU:\n"));
        }

        if (TkDbgLevelIsSet(TkDbgOamEnable)) {
            TkDbgDataDump((uint8 *) txBuf, txLen + sizeof(EthernetFrame),
                          16);
        }
        /*
         * Waiting for the response from the ONU 
         */
        msgLen =
            sal_msg_rcv(gTagLinkCfg[pathId].resMsgQid,
                        (char *) rxBuf,
                        MAX_MSG_LEN, gTagLinkCfg[pathId].timeOut / 10);
        sal_sem_give(gTagLinkCfg[pathId].semId);
        /* if ((ERROR == msgLen) || (gTagLinkCfg[pathId].timeOut != 0))*/
        if (ERROR == msgLen) {
            if (TkDbgLevelIsSet(TkDbgErrorEnable)) {
                TkDbgPrintf(("\r\nWait for the ONU OAM response message FAIL!\n"));
            }
            return (ERROR);
        } else {
            if (TkDbgLevelIsSet(TkDbgLogTraceEnable)) {
                TkDbgPrintf(("\r\nreceived response:\n"));
            }
            if((MAX_MSG_LEN < msgLen)||(MIN_MSG_LEN > msgLen))return ERROR;
            if (TkDbgLevelIsSet(TkDbgOamEnable)) {
                TkDbgDataDump((uint8 *) rxBuf + 4, msgLen, 16);
            }
            memmove(rxBuf,
                    (uint8 *) rxBuf +
                    sizeof(EthernetFrame) + 4,
                    msgLen - sizeof(EthernetFrame));
            *pRxLen = msgLen - sizeof(EthernetFrame);
            return (OK);
        }
    } else {
        sal_sem_give(gTagLinkCfg[pathId].semId);
        if (TkDbgLevelIsSet(TkDbgErrorEnable)) {
            TkDbgPrintf(("\r\nSend OAM request to the ONU FAIL!\n"));
        }
        return (ERROR);
    }
}

/*
 * send TK extension OAM message(Get/Set) and wait for response from the
 * ONU 
 */
int
TkOamNoResRequest(uint8 pathId, uint8 linkId, OamFrame * txBuf, uint32 txLen)
{
    int             retVal;

    if (NULL == txBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (txLen + sizeof(EthernetFrame) < 60) {
        bzero((uint8 *) txBuf + txLen +
              sizeof(EthernetFrame), 60 - (txLen + sizeof(EthernetFrame)));
    }

    sal_sem_take(gTagLinkCfg[pathId].semId, sal_sem_FOREVER);

    retVal =
        OamEthSend(pathId, &OamMcMacAddr,
                   (uint8 *) txBuf, txLen + sizeof(EthernetFrame));

    sal_sem_give(gTagLinkCfg[pathId].semId);

    return retVal;
}


/*
 * send TK extension OAM Get message with object instance and Get response 
 * from the ONU 
 */
int
TkExtOamObjGet(uint8 pathId, uint8 linkId,
               uint8 object, OamObjIndex * index,
               uint8 branch, uint16 leaf, uint8 * pRxBuf, uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;    /* Variable Descriptor */
    OamVarContainer *pObjectInst = NULL;    /* index of object */
    uint32          size,
                    objLen,
                    respLen;
    uint8           dataSize;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;

    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (object < 1) || (object > 4)
        || (NULL == index) || (NULL == pRxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if ((OamBranchAttribute != branch)
        && (OamBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);

    if (OamBranchAttribute == branch) {
        tk->opcode = OamExtOpVarRequest;    /* 01 */
    } else {                    /* when Branch is Action, it will use the
                                 * OpVarSet. */

        tk->opcode = OamExtOpVarSet;    /* 03 */
    }

    /*
     * index of object 
     */
    pObjectInst = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pObjectInst->branch = OamBranchNameBinding; /* 06 */
    pObjectInst->leaf = soc_htons(object);  /* object (1-4) */
    if ((OamNameMacName == object)
        || (OamNamePhyName == object)) {
        pObjectInst->value[0] = index->portId;
        pObjectInst->length = 1;
    } else {
        OamObjIndex    *pObj = (OamObjIndex *) pObjectInst->value;

        pObj->queueId.port = soc_htons(index->queueId.port);
        pObj->queueId.link = soc_htons(index->queueId.link);
        pObj->queueId.queue = soc_htons(index->queueId.queue);
        pObjectInst->length = sizeof(OamNameQueue);
    }

    objLen = OamContSize(pObjectInst);

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *) INT_TO_PTR(PTR_TO_INT(pObjectInst) + objLen);
    pVarDesc->branch = branch;  /* branch (07 or 09) */
    pVarDesc->leaf = soc_htons(leaf);   /* leaf */

    size =
        sizeof(OamOuiVendorExt) +
        sizeof(OamTkExt) + objLen + sizeof(OamVarDesc);
    gTagLinkCfg[pathId].timeOut = TkExtOamGetReplyTimeout(pathId);
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        size =
            sizeof(OamOuiVendorExt) + sizeof(OamTkExt) +
            sizeof(OamVarDesc);
        dataSize = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);
        if (OamVarErrNoError > dataSize) {
            size += dataSize + sizeof(OamVarDesc) + 1;
        } else {
            *pRxLen = 0;
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            TkDbgTrace(TkDbgErrorEnable);
            return ERROR;
        }

        dataSize = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);    /* Variable
                                                             * Width */

        if (dataSize < OamVarErrNoError) {  /* < 0x80 */
            *pRxLen = dataSize;
            bcopy((uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size + 1), pRxBuf,
                  dataSize);
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            return (OK);
        } else {
            *pRxLen = 0;
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            TkDbgTrace(TkDbgErrorEnable);
            return ERROR;
        }
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

/*
 * send TK extension OAM Get message with object instance and Get response 
 * from the ONU 
 */
int
TkExtOamObjActGet(uint8 pathId, uint8 linkId,
                  uint8 object,
                  OamObjIndex * index,
                  uint8 branch, uint16 leaf,
                  uint8 paramLen, uint8 * params,
                  uint8 * pRxBuf, uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;    /* Variable Descriptor */
    /* OamVarContainer *pVarDesc*/
    OamVarContainer *pObjectInst = NULL;    /* index of object */
    uint32          size,
                    objLen,
                    respLen;
    uint8           dataSize;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (object < 1) || (object > 4)
        || (NULL == index) || (NULL == pRxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if ((OamBranchAttribute != branch)
        && (OamBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);

    if (OamBranchAttribute == branch) {
        tk->opcode = OamExtOpVarRequest;    /* 01 */
    } else {                    /* when Branch is Action, it will use the
                                 * OpVarSet. */

        tk->opcode = OamExtOpVarSet;    /* 03 */
    }

    /*
     * index of object 
     */
    pObjectInst = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pObjectInst->branch = OamBranchNameBinding; /* 06 */
    pObjectInst->leaf = soc_htons(object);  /* object (1-4) */
    if ((OamNameMacName == object)
        || (OamNamePhyName == object)) {
        pObjectInst->value[0] = index->portId;
        pObjectInst->length = 1;
    } else {
        OamObjIndex    *pObj = (OamObjIndex *) pObjectInst->value;

        pObj->queueId.port = soc_htons(index->queueId.port);
        pObj->queueId.link = soc_htons(index->queueId.link);
        pObj->queueId.queue = soc_htons(index->queueId.queue);
        pObjectInst->length = sizeof(OamNameQueue);
    }

    objLen = OamContSize(pObjectInst);

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *) INT_TO_PTR(PTR_TO_INT(pObjectInst) + objLen);
    pVarDesc->branch = branch;  /* branch (07 or 09) */
    pVarDesc->leaf = soc_htons(leaf);   /* leaf */
    /* pVarDesc->length = paramLen; */
    *(uint8 *) (pVarDesc + 1) = paramLen;
    sal_memcpy(((uint8 *) (pVarDesc + 1) + 1), params, paramLen);

    size =
        sizeof(OamOuiVendorExt) +
        sizeof(OamTkExt) + objLen + sizeof(OamVarDesc) + 1 + paramLen;
    gTagLinkCfg[pathId].timeOut = TkExtOamGetReplyTimeout(pathId);
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        size -= (1 + paramLen);
        dataSize = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);    /* Variable
                                                             * Width */
        if (dataSize < OamVarErrNoError) {  /* < 0x80 */
            *pRxLen = dataSize;
            bcopy((uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size + 1), pRxBuf,
                  dataSize);
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            return (OK);
        } else {
            *pRxLen = 0;
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            TkDbgTrace(TkDbgErrorEnable);
            return ERROR;
        }
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

/*
 * send TK extension OAM Get message with object instance and Get
 * response(multi TLVs) from the ONU 
 */
int
TkExtOamObjGetMulti(uint8 pathId, uint8 linkId,
                    uint8 object,
                    OamObjIndex * index,
                    uint8 branch, uint16 leaf, uint8 * pRxBuf,
                    uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;    /* Variable Descriptor */
    OamVarContainer *pObjectInst = NULL;    /* index of object */
    uint32          size,
                    objLen,
                    respLen;
    uint32          dataSize;
    char           *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (object < 1) || (object > 4)
        || (NULL == index) || (NULL == pRxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if ((OamBranchAttribute != branch)
        && (OamBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (char *) TkOamMemGet(pathId);
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);

    if (OamBranchAttribute == branch) {
        tk->opcode = OamExtOpVarRequest;    /* 01 */
    } else {                    /* when Branch is Action, it will use the
                                 * OpVarSet. */

        tk->opcode = OamExtOpVarSet;    /* 03 */
    }

    /*
     * index of object 
     */
    pObjectInst = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pObjectInst->branch = OamBranchNameBinding; /* 06 */
    pObjectInst->leaf = soc_htons(object);  /* object (1-4) */
    if ((OamNameMacName == object)
        || (OamNamePhyName == object)) {
        pObjectInst->value[0] = index->portId;
        pObjectInst->length = 1;
    } else {
        OamObjIndex    *pObj = (OamObjIndex *) pObjectInst->value;

        pObj->queueId.port = soc_htons(index->queueId.port);
        pObj->queueId.link = soc_htons(index->queueId.link);
        pObj->queueId.queue = soc_htons(index->queueId.queue);
        pObjectInst->length = sizeof(OamNameQueue);
    }

    objLen = OamContSize(pObjectInst);

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *) INT_TO_PTR(PTR_TO_INT(pObjectInst) + objLen);
    pVarDesc->branch = branch;  /* branch (07 or 09) */
    pVarDesc->leaf = soc_htons(leaf);

    size =
        sizeof(OamOuiVendorExt) +
        sizeof(OamTkExt) + objLen + sizeof(OamVarDesc);
    gTagLinkCfg[pathId].timeOut = TkExtOamGetReplyTimeout(pathId);
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        /*
         * from branch/leaf 
         */
        size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt) + objLen;
        dataSize = respLen - size;
        *pRxLen = dataSize;
        bcopy((uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size), pRxBuf, dataSize);
        TkOamMemPut(pathId,(void *) pMsgBuf);
        return (OK);
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM Get message without object instance and Get
 * response from the ONU 
 */
int
TkExtOamGet(uint8 pathId, uint8 linkId,
            uint8 branch, uint16 leaf, uint8 * pRxBuf, uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;
    uint32          size,
                    respLen;
    uint8           dataSize;
    char           *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (NULL == pRxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if ((OamBranchAttribute != branch)
        && (OamBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (char *) TkOamMemGet(pathId);
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);

    if (OamBranchAttribute == branch) {
        tk->opcode = OamExtOpVarRequest;    /* 01 */
    } else {                    /* when Branch is Action, it will use the
                                 * OpVarSet. */

        tk->opcode = OamExtOpVarSet;    /* 03 */
    }

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pVarDesc->branch = branch;  /* branch (07 or 09) */
    pVarDesc->leaf = soc_htons(leaf);   /* leaf */

    size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt) + sizeof(OamVarDesc);
    gTagLinkCfg[pathId].timeOut = TkExtOamGetReplyTimeout(pathId);
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        dataSize = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);    /* Variable
                                                             * Width */
        if (dataSize < OamVarErrNoError) {  /* < 0x80 */
            *pRxLen = dataSize;
            bcopy((uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size + 1), pRxBuf,
                  dataSize);
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            return (OK);
        } else {
            *pRxLen = 0;
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            TkDbgTrace(TkDbgErrorEnable);
            return ERROR;
        }
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM Get message without object instance and Get
 * response(multi TLVs) from the ONU 
 */
int
TkExtOamGetMulti(uint8 pathId, uint8 linkId,
                 uint8 branch, uint16 leaf, uint8 * pRxBuf,
                 uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;    /* Variable Descriptor */
    uint32          size,
                    respLen;
    uint8           dataSize;
    char           *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (NULL == pRxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if ((OamBranchAttribute != branch)
        && (OamBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (char *) TkOamMemGet(pathId);   /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);

    if (OamBranchAttribute == branch) {
        tk->opcode = OamExtOpVarRequest;    /* 01 */
    } else {                    /* when Branch is Action, it will use the
                                 * OpVarSet. */

        tk->opcode = OamExtOpVarSet;    /* 03 */
    }

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pVarDesc->branch = branch;  /* branch (07 or 09) */
    pVarDesc->leaf = soc_htons(leaf);   /* leaf */

    size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt) + sizeof(OamVarDesc);
    gTagLinkCfg[pathId].timeOut = TkExtOamGetReplyTimeout(pathId);
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt);  /* from
                                                             * branch/leaf 
                                                             */
        dataSize = respLen - size;
        *pRxLen = dataSize;
        bcopy((uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size), pRxBuf, dataSize);
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (OK);
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM Set message with object instance and Get return
 * code 
 */
uint8
TkExtOamObjSet(uint8 pathId, uint8 linkId,
               uint8 object, OamObjIndex * index,
               uint8 branch, uint16 leaf, uint8 * pTxBuf, uint8 txLen)
{
    OamTkExt       *tk = NULL;
    OamVarContainer *pVarCont = NULL;   /* Variable Container */
    OamVarContainer *pObjectInst = NULL;    /* index of object */
    uint32          size,
                    respLen,
                    objLen,
                    varContLen;
    uint8           retCode;
    char           *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    uint8           objValLen = 0;

    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (object < 1) || (object > 4)
        || (NULL == index)
        || (txLen > OamVarErrNoError)) {
        return (OamVarErrActBadParameters);
    }

    if ((OamBranchAttribute != branch)
        && (OamBranchAction != branch)) {
        return (OamVarErrActBadParameters);
    }

    pMsgBuf = (char *) TkOamMemGet(pathId);   /* get memory */
    if (NULL == pMsgBuf) {
        return (OamVarErrActNoResources);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);
    tk->opcode = OamExtOpVarSet;    /* 03 */

    /*
     * index of object 
     */
    pObjectInst = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pObjectInst->branch = OamBranchNameBinding; /* 06 */
    pObjectInst->leaf = soc_htons(object);  /* object (1-4) */
    if ((OamNameMacName == object)
        || (OamNamePhyName == object)) {
        pObjectInst->value[0] = index->portId;
        pObjectInst->length = 1;
    } else {
        OamObjIndex    *pObj = (OamObjIndex *) pObjectInst->value;

        pObj->queueId.port = soc_htons(index->queueId.port);
        pObj->queueId.link = soc_htons(index->queueId.link);
        pObj->queueId.queue = soc_htons(index->queueId.queue);
        pObjectInst->length = sizeof(OamNameQueue);
    }

    objLen = OamContSize(pObjectInst);

    /*
     * Variable Container 
     */
    pVarCont = (OamVarContainer *)INT_TO_PTR(PTR_TO_INT(pObjectInst) + objLen);
    pVarCont->branch = branch;  /* branch (07 or 09) */
    pVarCont->leaf = soc_htons(leaf);   /* leaf */
    if ((NULL == pTxBuf) && (0 == txLen)) {
        pVarCont->length = 0x80;
    } else if(!((NULL == pTxBuf) || (0 == txLen))){
        pVarCont->length = txLen;
        bcopy(pTxBuf, pVarCont->value, txLen);
    }
    varContLen = OamContSize(pVarCont);

    size =
        sizeof(OamOuiVendorExt) + sizeof(OamTkExt) + objLen + varContLen;
    gTagLinkCfg[pathId].timeOut = TkExtOamGetReplyTimeout(pathId);
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        size =
            sizeof(OamOuiVendorExt) + sizeof(OamTkExt) +
            sizeof(OamVarDesc);
        objValLen = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);
        if (OamVarErrNoError > objValLen) {
            size += objValLen + sizeof(OamVarDesc) + 1;
            retCode = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);
        } else {
            retCode = objValLen;
        }
        
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (retCode);
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        return (OamVarErrUnknow);   /* 0xf0 */
    }
}


/*
 * send TK extension OAM Set message without object instance and Get
 * return code from the ONU 
 */
uint8
TkExtOamSet(uint8 pathId, uint8 linkId,
            uint8 branch, uint16 leaf, uint8 * pTxBuf, uint8 txLen)
{
    OamTkExt       *tk = NULL;
    OamVarContainer *pVarCont = NULL;   /* Variable Container */
    uint32          size,
                    respLen,
                    varContLen;
    uint8           retCode;
    char           *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);


    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (txLen > OamVarErrNoError)) {
        return (OamVarErrActBadParameters);
    }

    if ((OamBranchAttribute != branch)
        && (OamBranchAction != branch)) {
        return (OamVarErrActBadParameters);
    }

    pMsgBuf = (char *) TkOamMemGet(pathId);
    if (NULL == pMsgBuf) {
        return (OamVarErrActNoResources);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);
    tk->opcode = OamExtOpVarSet;    /* 03 */

    /*
     * Variable Container 
     */
    pVarCont = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pVarCont->branch = branch;  /* branch (07 or 09) */
    pVarCont->leaf = soc_htons(leaf);   /* leaf */
    if ((NULL == pTxBuf) && (0 == txLen)) {
        pVarCont->length = 0x80;
    } else if(!((NULL == pTxBuf) || (0 == txLen))){
        pVarCont->length = txLen;
        bcopy(pTxBuf, pVarCont->value, txLen);
    }
    
    varContLen = OamContSize(pVarCont);

    size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt) + varContLen;
    gTagLinkCfg[pathId].timeOut = TkExtOamGetReplyTimeout(pathId);
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        size =
            sizeof(OamOuiVendorExt) + sizeof(OamTkExt) +
            sizeof(OamVarDesc);
        retCode = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);
        TkOamMemPut(pathId,(void *) pMsgBuf);
        return (retCode);
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);
        return (OamVarErrUnknow);   /* 0xf0 */
    }
}

/*
 * TkExtOamResponse: send OAM response message to OLT
 * 
 * Parameters:
 *  \param LinkId the source LLID of the OAM
 * / TxBuf the buffer which the response frame was in
 * / TxLen the response message length
 * / 
 * / \return 
 * / OK (0) - if send successfully, else ERROR (-1)
 */
int32
TkExtOamResponse(uint8 pathId, uint8 linkId, OamFrame * txBuf, uint32 txLen)
{
    MacAddr         dstMac;
    uint16          flags;
    OamMsg         *msg =
        (OamMsg *) ((uint8 *) txBuf + sizeof(EthernetFrame));

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (NULL == txBuf)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    flags = soc_ntohs(msg->flags) & OamFlagMask;
    AttachOamFlag(linkId, &flags);
    msg->flags = soc_htons(flags);
    bcopy((uint8 *) & OamMcMacAddr, (uint8 *) & dstMac, sizeof(MacAddr));
    if (OK ==
        OamEthSend(pathId, &dstMac, (uint8 *) txBuf,
                   txLen + sizeof(EthernetFrame))) {
        if (TkDbgLevelIsSet(TkDbgLogTraceEnable)) {
            TkDbgPrintf(("\r\nsent result to OLT:\n"));
        }
        if (TkDbgLevelIsSet(TkDbgOamEnable)) {
            TkDbgDataDump((uint8 *) txBuf, txLen + sizeof(EthernetFrame),
                          16);
        }
        return (OK);
    } else {
        if (TkDbgLevelIsSet(TkDbgErrorEnable)) {
            TkDbgPrintf(("\r\nSend OAM response to the OLT FAIL!\n"));
        }
        return (ERROR);
    }
}

uint8
TxOamDeliver(uint8 pathId, uint8 linkId,
             BufInfo * bufInfo, uint8 * pRxBuf, uint32 * pRxLen)
{
    char           *pMsgBuf = NULL;
    uint32          size,
                    RespLen;
    uint32          DataSize;
    uint8           ret = RcOk;

    pMsgBuf = (char *) bufInfo->start;
    if (pMsgBuf == NULL)
        return RcBadParam;
    size = BufGetUsed(bufInfo) - sizeof(EthernetFrame);

    /* 
     * When sending, 
     * EthernetFrame size (da,sa,type, 14 bytes totally) is not included!
     * The TxBuf DOES start from EthernetFrame!
     * When receiving,
     * The RxBuf would bypass EthernetFrame, also RespLen!
     */
    if (OK ==
        TkOamRequest(pathId, linkId,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &RespLen)) {
        size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt);  /* from
                                                             * branch/leaf 
                                                             */
        DataSize = RespLen - size;
        *pRxLen = DataSize;
        bcopy((uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size), pRxBuf, DataSize);
    } else {
        *pRxLen = 0;
        ret = RcFail;
    }

    TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
    return (ret);
}

/*
 * OamContSize: Returns size required for container (including overhead)
 * Parameters: \param cont Container to measure
 * 
 * \return Size in bytes of cont 
 */
uint8
OamContSize(OamVarContainer const *cont)
{
    uint8           result =
        sizeof(OamVarDesc) + sizeof(cont->length) + 128;
    /*
     * value if cont->length is zero 
     */

    if (cont->length) {
        result = sizeof(OamVarDesc) + sizeof(cont->length);
        if (cont->length < OamVarErrReserved) { /* a length value */
            result += cont->length;
        } else                  /* ? maybe add 1*/
        {
        }

    }

    return (result);
}

/*
 * NextCont: Return location of next container in OAM PDU * * Parameters:
 * * \param cont Current container in PDU * * \return * Next container
 * in PDU after cont 
 */
OamVarContainer *
NextCont(OamVarContainer const *cont)
{
    return (OamVarContainer *) ((uint8 *) cont + OamContSize(cont));
}

uint8
TkOamMsgPrepare(uint8 pathId, BufInfo * bufInfo, uint8 tkOpCode)
{
    char           *pMsgBuf = NULL;
    OamTkExt       *tk = NULL;
    uint16          flags = 0x0050;

    pMsgBuf = (char *) TkOamMemGet(pathId);   /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
    InitBufInfo(bufInfo, 1500, (uint8 *) pMsgBuf);

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);
    tk->opcode = tkOpCode;
    (void) BufSkip(bufInfo,
                   sizeof(OamOuiVendorExt) +
                   sizeof(EthernetFrame) + sizeof(OamTkExt));
    return (OK);
}


void
TkExtOamSetReplyTimeout(uint8 pathId, uint32 val)
{
    gTagLinkCfg[pathId].timeOut = val;
}

uint32
TkExtOamGetReplyTimeout(uint8 pathId)
{
    return gTagLinkCfg[pathId].timeOut;
}
