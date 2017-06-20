/*
 * $Id: CtcOamApiCommon.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcOamApiCommon.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkUtils.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkInit.h>  
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/OamUtilsCtc.h>

extern VlanLinkConfig gTagLinkCfg[MAX_NUM_OF_PON_CHIP+1];

uint8
CtcOamMsgPrepare(uint8 pathId, BufInfo * bufInfo, uint8 tkOpCode)
{
    uint8          *pMsgBuf = NULL;
    OamTkExt       *tk = NULL;
    uint16          flags = 0x0050;

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    InitBufInfo(bufInfo, 1500, (uint8 *) pMsgBuf);

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);
    tk->opcode = tkOpCode;
    (void) BufSkip(bufInfo,
                   sizeof(OamOuiVendorExt) +
                   sizeof(EthernetFrame) + sizeof(OamTkExt));
    return (OK);
}

/*
 * send TK extension OAM Get message with object instance and Get response 
 * from the ONU 
 */
int
CtcExtOamObjGet(uint8 pathId, uint8 linkId,
                uint8 objBranch, uint16 objLeaf,
                uint32 objIndex, uint8 branch,
                uint16 leaf, uint8 * pRxBuf, uint32 * pRxLen)
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

    if ((OamCtcBranchExtAttribute != branch)
        && (OamCtcBranchExtAction != branch)
        && (OamCtcBranchAttribute != branch)
        && (OamCtcBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);

    tk->opcode = OamExtOpVarRequest;    /* 01 */

    /*
     * index of object 
     */

    pObjectInst = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pObjectInst->branch = objBranch;    /* 06 */
    pObjectInst->leaf = soc_htons(objLeaf); /* object (1-4) */

    if (OamCtcContextPort == objLeaf && OamCtcBranchObjInst == objBranch) {
        pObjectInst->value[0] = objIndex;
        pObjectInst->length = 1;
    }

    if ((OamCtcContextPort == objLeaf
         && OamCtcBranchObjInst21 == objBranch)
        || (OamCtcContextLLID == objLeaf
            && OamCtcBranchObjInst21 == objBranch)) {
        objIndex = soc_htonl(objIndex);
        sal_memcpy(&(pObjectInst->value[0]), &objIndex, 4);
        pObjectInst->length = 4;
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
            return (ERROR);
        }

        dataSize = *(uint8 *)INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);    /* Variable
                                                             * Width */

        if (dataSize < OamVarErrNoError) {  /* < 0x80 */
            *pRxLen = dataSize;
            bcopy((uint8 *)INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size + 1), pRxBuf,
                  dataSize);
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            return (OK);
        } else {
            *pRxLen = 0;
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            TkDbgTrace(TkDbgErrorEnable);
            return (ERROR);
        }
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
}

/*
 * send TK extension OAM Get message with object instance and Get response 
 * from the ONU 
 */
int
CtcExtOamObjActGet(uint8 pathId, uint8 linkId,
                   uint8 objBranch,
                   uint16 objLeaf,
                   uint32 objIndex, uint8 branch,
                   uint16 leaf, uint8 paramLen,
                   uint8 * params, uint8 * pRxBuf, uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarContainer     *pVarDesc = NULL;    /* Variable Descriptor */
    OamVarContainer *pObjectInst = NULL;    /* index of object */
    uint32          size,
                    objLen,
                    respLen;
    uint8           dataSize;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);

    if ((OamCtcBranchExtAttribute != branch)
        && (OamCtcBranchExtAction != branch)
        && (OamCtcBranchAttribute != branch)
        && (OamCtcBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);

    tk->opcode = OamExtOpVarRequest;    /* 01 */

    /*
     * index of object 
     */
    pObjectInst = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    if(OamCtcContextOnu != objLeaf){
    /* index of object */
        pObjectInst->branch = objBranch; /* 06 */
        pObjectInst->leaf = soc_htons(objLeaf);         /* object (1-4) */

        if(OamCtcBranchObjInst == objBranch){
            pObjectInst->value[0] = objIndex;
            pObjectInst->length = 1;
        }

        if (OamCtcBranchObjInst21 == objBranch){
            objIndex = soc_htonl(objIndex);
            sal_memcpy(&(pObjectInst->value[0]),&objIndex,4);
            pObjectInst->length = 4;
        }

        objLen = OamContSize(pObjectInst);
    }else{
        /*no object packed here*/
        objLen = 0;
    }

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarContainer *)INT_TO_PTR(PTR_TO_INT(pObjectInst) + objLen);
    pVarDesc->branch = branch;      /* branch (07 or 09) */
    pVarDesc->leaf = soc_htons(leaf);  /* leaf */
    pVarDesc->length = paramLen; 
    sal_memcpy(pVarDesc->value, params, paramLen);


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
            return (ERROR);
        }
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
}




/*
 * send TK extension OAM Get message with object instance and Get
 * response(multi TLVs) from the ONU 
 */
int
CtcExtOamObjGetMulti(uint8 pathId, uint8 linkId,
                     uint8 objBranch,
                     uint16 objLeaf,
                     uint32 objIndex,
                     uint8 branch, uint16 leaf,
                     uint8 * pRxBuf, uint32 * pRxLen)
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

    if ((OamCtcBranchExtAttribute != branch)
        && (OamCtcBranchExtAction != branch)
        && (OamCtcBranchAttribute != branch)
        && (OamCtcBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);

    tk->opcode = OamExtOpVarRequest;    /* 01 */

    /*
     * index of object 
     */
    pObjectInst = (OamVarContainer *)INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pObjectInst->branch = objBranch;    /* 06 */
    pObjectInst->leaf = soc_htons(objLeaf); /* object (1-4) */

    if (OamCtcContextPort == objLeaf && OamCtcBranchObjInst == objBranch) {
        pObjectInst->value[0] = objIndex;
        pObjectInst->length = 1;
    }

    if (OamCtcContextPort == objLeaf && OamCtcBranchObjInst21 == objBranch) {
        objIndex = soc_htonl(objIndex);
        sal_memcpy(&(pObjectInst->value[0]), &objIndex, 4);
        pObjectInst->length = 4;
    }

    objLen = OamContSize(pObjectInst);

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *)INT_TO_PTR(PTR_TO_INT(pObjectInst) + objLen);
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
        bcopy((uint8 *)INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size), pRxBuf, dataSize);
        TkOamMemPut(pathId,(void *) pMsgBuf);
        return (OK);
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
}


/*
 * send Ctc extension OAM Get message without object instance and Get
 * response from the ONU 
 */
int
CtcExtOamGet(uint8 pathId, uint8 linkId,
             uint8 branch, uint16 leaf, uint8 * pRxBuf, uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;
    uint32          size,
                    respLen;
    uint8           dataSize;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (NULL == pRxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if ((OamCtcBranchExtAttribute != branch)
        && (OamCtcBranchExtAction != branch)
        && (OamCtcBranchAttribute != branch)
        && (OamCtcBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);

    tk->opcode = OamExtOpVarRequest;    /* 01 */

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
            return (ERROR);
        }
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
}


/*
 * send TK extension OAM Get message without object instance and Get
 * response(multi TLVs) from the ONU 
 */
int
CtcExtOamGetMulti(uint8 pathId, uint8 linkId,
                  uint8 branch, uint16 leaf, uint8 * pRxBuf,
                  uint32 * pRxLen)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;    /* Variable Descriptor */
    uint32          size,
                    respLen;
    uint8           dataSize;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);

    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (NULL == pRxBuf)
        || (NULL == pRxLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if ((OamCtcBranchExtAttribute != branch)
        && (OamCtcBranchExtAction != branch)
        && (OamCtcBranchAttribute != branch)
        && (OamCtcBranchAction != branch)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);

    tk->opcode = OamExtOpVarRequest;    /* 01 */

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *)INT_TO_PTR(PTR_TO_INT(tk) + 1);
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
        return (ERROR);
    }
}


/*
 * send TK extension OAM Set message with object instance and Get return
 * code 
 */
uint8
CtcExtOamObjSet(uint8 pathId, uint8 linkId,
                uint8 objBranch, uint16 objLeaf,
                uint32 objIndex, uint8 branch,
                uint16 leaf, uint8 * pTxBuf, uint8 txLen)
{
    OamTkExt       *tk = NULL;
    OamVarContainer *pVarCont = NULL;   /* Variable Container */
    OamVarContainer *pObjectInst = NULL;    /* index of object */
    uint32          size,
                    respLen,
                    objLen,
                    varContLen;
    uint8           retCode;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    uint8           objValLen = 0;

    AttachOamFlagNoPass(linkId, &flags);

    if ((OamCtcBranchExtAttribute != branch)
        && (OamCtcBranchExtAction != branch)
        && (OamCtcBranchAttribute != branch)
        && (OamCtcBranchAction != branch)) {
        return (OamVarErrActBadParameters);
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        return (OamVarErrActNoResources);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);
    tk->opcode = OamExtOpVarSet;    /* 03 */

    /*
     * index of object 
     */
    pObjectInst = (OamVarContainer *)INT_TO_PTR(PTR_TO_INT(tk) + 1);
    if(OamCtcContextOnu != objLeaf){
        pObjectInst = (OamVarContainer *)INT_TO_PTR(PTR_TO_INT(tk) + 1);
    	pObjectInst->branch = objBranch; /* 06 */
    	pObjectInst->leaf = soc_htons(objLeaf);		   /* object (1-4) */

    	if(OamCtcBranchObjInst == objBranch){
    		pObjectInst->value[0] = objIndex;
            pObjectInst->length = 1;
        }
    	
        if (OamCtcBranchObjInst21 == objBranch){
            objIndex = soc_htonl(objIndex);
    		sal_memcpy(&(pObjectInst->value[0]),&objIndex,4);
            if ( OamCtcObjPort==objLeaf ){
                pObjectInst->value[0] = 0x01;
            }
            
            pObjectInst->length = 4;
        }
        
        objLen = OamContSize(pObjectInst);
    }else{
        objLen = 0;
    }

    /*
     * Variable Descriptor 
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
        } else {
            TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
            return objValLen;
        }
        retCode = *(uint8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) + size);
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
CtcExtOamSet(uint8 pathId, uint8 linkId,
             uint8 branch, uint16 leaf, uint8 * pTxBuf, uint8 txLen)
{
    OamTkExt       *tk = NULL;
    OamVarContainer *pVarCont = NULL;   /* Variable Container */
    uint32          size,
                    respLen,
                    varContLen;
    uint8           retCode;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    AttachOamFlagNoPass(linkId, &flags);


    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (txLen > OamVarErrNoError)) {
        return (OamVarErrActBadParameters);
    }

    if ((OamCtcBranchExtAttribute != branch)
        && (OamCtcBranchExtAction != branch)
        && (OamCtcBranchAttribute != branch)
        && (OamCtcBranchAction != branch)) {
        return (OamVarErrActBadParameters);
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pMsgBuf) {
        return (OamVarErrActNoResources);
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);
    tk->opcode = OamExtOpVarSet;    /* 03 */

    /*
     * Variable Container 
     */
    pVarCont = (OamVarContainer *)INT_TO_PTR(PTR_TO_INT(tk) + 1);
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

int 
CtcExtOam21ObjInstPack(uint8 type, uint16 index, uint32 *pObjInst)
{
    uint32 objInst = 0;

    if(NULL == pObjInst){
        return ERROR;
    }

    switch(type){
        case OamCtcObjONU:
            objInst = 0;
            break;   
        case OamCtcObjPort:
            objInst = Ctc21ObjPortInstPack(OamCtcObjPort,0,0,index);
            break;
        case OamCtcObjCard:
            objInst = index;
            break;
        case OamCtcObjLLID:
            objInst = index;
            break;
        case OamCtcObjPonIF:
            objInst = index;
            break;
        default:
            break;
        }
    
    *pObjInst = objInst;
    return OK;
}

