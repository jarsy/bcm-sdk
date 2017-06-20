/*
 * $Id: CtcMiscApi.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcMiscApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkUtils.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/CtcOam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/OamUtilsCtc.h>
#include <soc/ea/tk371x/TkInit.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/CtcMiscApi.h>


static CtcInfoFromONUPONChipsSet CtcInfoFromONUPONChipsetCont = {
    0x0,
    0x0,
    {0x0},
    0x0,
    {0x0},
    0x0,
    {0x0},
    0x0,
    {0x0},
    0x0,
    {0x0}
};

static int 
GetObjFromAlarm(uint16 alarmid, uint16 port, CtcOamObj21* obj)
{
	uint8  type;
	uint32 index;

	if (obj == NULL){
  	    TkDbgTrace(TkDbgErrorEnable);
		return ERROR;
	}
	
	type = (alarmid >> 8) & 0x00FF;
	
	sal_memset( obj, 0, sizeof(CtcOamObj21) );
    
	if (type == OamCtcPortOnu){ 
		obj->type = OamCtcObjONU;
	} else if (type == OamCtcPortEther){ 
		obj->type = OamCtcObjPonIF;
	} else if (type == OamCtcPortVoip){ 
		obj->type = OamCtcObjCard;
	} else if (type == OamCtcPortADSL2){
		obj->type = OamCtcObjPort;
		obj->inst.uniPortType = OamCtcPortEther;
		obj->inst.uniPortNum  = port;
    } else if (type == OamCtcPortVDSL2){
		obj->type = OamCtcObjPort;
		obj->inst.uniPortType = OamCtcPortVoip;
		obj->inst.uniPortNum  = port;
    } else if (type == OamCtcPortE1){
		obj->type = OamCtcObjPort;
		obj->inst.uniPortType = OamCtcPortE1;
		obj->inst.uniPortNum  = port;
    }

	obj->inst.uniPortNum = soc_htons(obj->inst.uniPortNum);
	sal_memcpy( &index, &obj->inst, sizeof(uint32));
	index = soc_ntohl(index);
	sal_memcpy(&obj->inst, &index, sizeof(uint32));
	
	return OK;
}


void
TKCTCClearONUPONChipsSetInfo(void)
{
    CtcInfoFromONUPONChipsetCont.CtcInfoFromONUPONChipsSetInit = FALSE;
}

void
TKCTCExtOamFillMacForSN(uint8 * mac)
{
    CtcOamOnuSN    *pCtcOamOnuSN = NULL;
    pCtcOamOnuSN = (CtcOamOnuSN *) (CtcInfoFromONUPONChipsetCont.CtcONUSN);

    if (NULL == mac) {
        TkDbgTrace(TkDbgErrorEnable);
        return;
    }

    sal_memcpy(mac, pCtcOamOnuSN->ONUID, 6);
}

int32
TKCTCExtOamGetInfoFromONUPONChipsets(uint8
                                     pathId,
                                     uint8
                                     linkId,
                                     CtcInfoFromONUPONChipsSet * pCont)
{
    BufInfo         bufInfo;
    Bool            ok = TRUE;
    Bool            ctc20ok = TRUE;
    Bool            ctc21ok = TRUE;
    uint32          rxLen;
    uint8           ret;
    Bool            more;
    tGenOamVar      var;
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);
 
    if (linkId != 0 || NULL == rxTmpBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (CtcOamMsgPrepare(pathId, &bufInfo, OamExtOpVarRequest) != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ok = ok && OamAddCtcExtAttrLeaf(&bufInfo, OamCtcAttrOnuSn);
    ok = ok && OamAddCtcExtAttrLeaf(&bufInfo, OamCtcAttrFirmwareVer);
    ok = ok && OamAddCtcExtAttrLeaf(&bufInfo, OamCtcAttrChipsetId);
    ok = ok && OamAddCtcExtAttrLeaf(&bufInfo, OamCtcAttrOnuCap);
    ok = ok && OamAddCtcExtAttrLeaf(&bufInfo, OamCtcAttrOnuCap2);

    ret = TxOamDeliver(pathId, linkId, &bufInfo, (uint8 *) rxTmpBuf, &rxLen);
    if (ret != RcOk) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
    if (TkDbgLevelIsSet(TkDbgLogTraceEnable))
        BufDump(NULL, rxTmpBuf, rxLen);

    InitBufInfo(&bufInfo, rxLen, rxTmpBuf);

    more = GetNextOamVar(&bufInfo, &var, &ret);
    ok = more && (ret == RcOk)
        && ((uint8)var.Branch == (uint8)OamCtcBranchExtAttribute)
        && (var.Leaf == OamCtcAttrOnuSn);
    if (ok) {
        pCont->CtcONUSNLen = var.Width;
        sal_memcpy(pCont->CtcONUSN, var.pValue, var.Width);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    more = GetNextOamVar(&bufInfo, &var, &ret);
    ok = more && (ret == RcOk)
        && ((uint8)var.Branch == (uint8)OamCtcBranchExtAttribute)
        && (var.Leaf == OamCtcAttrFirmwareVer);
    if (ok) {
        pCont->CtcFirmwareVerLen = var.Width;
        sal_memcpy(pCont->CtcFirmwareVer, var.pValue, var.Width);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        sal_memset(pCont, 0x00, sizeof(CtcInfoFromONUPONChipsSet));
        return ERROR;
    }

    more = GetNextOamVar(&bufInfo, &var, &ret);
    ok = more && (ret == RcOk)
        && ((uint8)var.Branch == (uint8)OamCtcBranchExtAttribute)
        && (var.Leaf == OamCtcAttrChipsetId);
    if (ok) {
        pCont->CtcChipsetIdLen = var.Width;
        sal_memcpy(pCont->CtcChipsetId, var.pValue, var.Width);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        sal_memset(pCont, 0x00, sizeof(CtcInfoFromONUPONChipsSet));
        return ERROR;
    }

    more = GetNextOamVar(&bufInfo, &var, &ret);
    ok = more && (ret == RcOk)
        && ((uint8)var.Branch == (uint8)OamCtcBranchExtAttribute)
        && (var.Leaf == OamCtcAttrOnuCap);
    if (ok) {
        pCont->CtcOnuCap1Len = var.Width;
        sal_memcpy(pCont->CtcOnuCap1, var.pValue, var.Width);
    }

    ctc20ok = ok;

    more = GetNextOamVar(&bufInfo, &var, &ret);
    ok = more && (ret == RcOk)
        && ((uint8)var.Branch == (uint8)OamCtcBranchExtAttribute)
        && (var.Leaf == OamCtcAttrOnuCap2);
    if (ok) {
        pCont->CtcOnuCap2Len = var.Width;
        sal_memcpy(pCont->CtcOnuCap2, var.pValue, var.Width);
    }

    ctc21ok = ok;

    if (TRUE != ctc21ok && TRUE != ctc20ok) {
        TkDbgTrace(TkDbgErrorEnable);
        sal_memset(pCont, 0x00, sizeof(CtcInfoFromONUPONChipsSet));
        return ERROR;
    } else {
        pCont->CtcInfoFromONUPONChipsSetInit = TRUE;
    }

    return OK;
}

static int 
CtcDbaFramePack(void *buf, OamCtcDbaData * dba)
{
    uint8 i, j;
    OamCtcDbaDataPack *d = (OamCtcDbaDataPack*)buf;
    OamCtcDbaQueueSetPack *set;
    uint16 *threshold;
    int flag;
    int len;

#define CTC_DBA_REPORT_Q_IS_SET(q,flag) ((flag)&(1<<(q)))    
    
    if(NULL == buf || NULL == dba){
        return ERROR;
    }

    if(dba->num > 4){
        return ERROR;
    }
    
    d->num = dba->num;
    set = d->set;
    /*add the qset num length*/
    len = sizeof(uint8);
    
    for(i = 0; i < dba->num; i++){
        set->report = dba->set[i].report;
        flag = set->report;
        /*add report bitmap length*/
        len += sizeof(uint8);

        threshold = set->threshold;
        for(j = 0; j < sizeof(uint8)*8; j++){
            if(CTC_DBA_REPORT_Q_IS_SET(j, flag)){
                  *threshold = soc_htons(dba->set[i].threshold[j]);
                  threshold++;
                  /*add threshold length*/
                  len += sizeof(uint16);
            }
        }
        
        set = (OamCtcDbaQueueSetPack *)threshold;
    }
#undef CTC_DBA_REPORT_Q_IS_SET  
 
    return len;
}

static int 
CtcDbaFrameParse(void *buf, OamCtcDbaData * dba)
{
    uint8 i, j;
    OamCtcDbaDataPack *d = (OamCtcDbaDataPack*)buf;
    OamCtcDbaQueueSetPack *set;
    uint16 *threshold;
    int flag;

#define CTC_DBA_REPORT_Q_IS_SET(q,flag) ((flag)&(1<<(q)))    
    
    if(NULL == buf || NULL == dba){
        return ERROR;
    }

    if(d->num > 4){
        return ERROR;
    }

    dba->num = d->num;

    set = d->set;
    
    for(i = 0; i < d->num; i++){
        dba->set[i].report = set->report;
        flag = set->report;
        threshold = set->threshold;

        for(j = 0; j < sizeof(uint8)*8; j++){
            if(CTC_DBA_REPORT_Q_IS_SET(j, flag)){
                dba->set[i].threshold[j] = soc_ntohs(*threshold);
                threshold++;
            }else{
                dba->set[i].threshold[j] = 0;
            }
        }

        set = (OamCtcDbaQueueSetPack *)threshold;
    }
    
#undef CTC_DBA_REPORT_Q_IS_SET  
    return OK;
}


int32
TKCTCExtOamGetDbaCfg(uint8 pathId, uint8 linkId, OamCtcDbaData * dba)
{
    uint8          *pMsgBuf = NULL;
    OamTkExt       *tk = NULL;
    uint16          flags = 0x0050;
    uint32          size;
    uint32          respLen;
    int             ret = ERROR;

    if (NULL == dba) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    AttachOamFlagNoPass(linkId, &flags);

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
    tk = (OamTkExt *) OamFillExtHeader(flags, &CTCOui, pMsgBuf);
    tk->opcode = OamCtcDba;
    *(uint8 *) (tk + 1) = DbaGetRequest;
    size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt) + 1;

    if (OK ==
        TkOamRequest(pathId, 0,
                     (OamFrame *) pMsgBuf, size,
                     (OamPdu *) pMsgBuf, &respLen)) {
        OamCtcDbaPdu   *pdu = (OamCtcDbaPdu *) ((OamOuiVendorExt *)
                                                pMsgBuf + 1);
        if (pdu->opcode == DbaGetResponse) {
            void *dba_buff = (void *)(&(pdu->indexes));

            ret = CtcDbaFrameParse(dba_buff, dba);
        }
    }
    
    TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
    return ret;
}

int32 
TKCTCExtOamSetDbaCfg(uint8 pathId,uint8 linkId,OamCtcDbaData * dba) 
{       
    uint8 *pMsgBuf = NULL;      
    OamTkExt *tk = NULL;        
    uint16 flags = 0x0050;  
    uint32 size;    
    uint32 respLen; 
    int ret = ERROR;    

    if(NULL == dba){       
        TkDbgTrace(TkDbgErrorEnable);       
        return ERROR;               
    }       

    AttachOamFlagNoPass(linkId, &flags);        
    pMsgBuf = (uint8 *)TkOamMemGet(pathId);   /* get memory */    
    if (NULL == pMsgBuf){        
        TkDbgTrace(TkDbgErrorEnable);        
        return (ERROR);        
    }    

    tk = (OamTkExt *)OamFillExtHeader (flags, &CTCOui, pMsgBuf);  
    tk->opcode   = OamCtcDba;   
    *(uint8*)(tk+1) = DbaSetRequest;    
    size =  CtcDbaFramePack( (uint8*)tk + 2, dba ); 
    size += sizeof(OamMsg) + sizeof(IeeeOui) + sizeof (OamTkExt) + 1;     

    if (OK == TkOamRequest (pathId, 0, (OamFrame *)pMsgBuf, size,
        (OamPdu *)pMsgBuf, &respLen)){           
        OamCtcDbaPduSetResponse *pdu =  
            (OamCtcDbaPduSetResponse *)(pMsgBuf + sizeof(OamMsg) + sizeof(IeeeOui));       
        if ( pdu->opcode==DbaSetResponse && pdu->ack ){           
            ret = OK;           
        }       
    }   

    TkOamMemPut (pathId, (void *)pMsgBuf); /* free memory */    
    return ret; 
}

int32
TKCTCExtOamGetFecAbility(uint8 pathId, uint8 LinkId, uint32 * fecAbility)
{
    uint32          len;
    uint8          *buff;
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == fecAbility)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (OK != CtcExtOamGet(pathId,
                           LinkId,
                           OamCtcBranchAttribute, OamIeeeFecAble, buff,
                           &len)
        ) {
        TkOamMemPut(pathId,(uint8 *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    sal_memcpy(fecAbility, buff, sizeof(uint32));

    *fecAbility = soc_ntohl(*fecAbility);

    TkOamMemPut(pathId,(void *) buff);
    return ret;
}

int32
TKCTCExtOamNoneObjGetRaw(uint8 pathId,
                         uint8 LinkId,
                         uint8 branch,
                         uint16 leaf, uint8 * pBuff, int32 * retLen)
{
    uint32          len;
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pBuff)
           || (NULL == retLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (OK !=
        CtcExtOamGet(pathId, LinkId, branch, leaf, (uint8 *) pBuff, &len)
        ) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    *retLen = len;

    return ret;
}

int32
TKCTCExtOamNoneObjSetRaw(uint8 pathId,
                         uint8 LinkId,
                         uint8 branch,
                         uint16 leaf,
                         uint8 * pBuff, uint8 retLen, uint8 * reCode)
{
    int32           ret = OK;
    uint8           retVal;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pBuff)
           || (NULL == reCode)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    retVal =
        CtcExtOamSet(pathId, LinkId, branch, leaf, (uint8 *) pBuff,
                     retLen);

    if (OamVarErrNoError != retVal) {
        ret = ERROR;
    } else {
        ret = OK;
    }

    *reCode = retVal;

    return ret;
}

int32
TKCTCExtOamObjGetRaw(uint8 pathId, uint8 LinkId,
                     uint8 objBranch,
                     uint16 objLeaf,
                     uint32 objIndex,
                     uint8 branch, uint16 leaf, uint8 * pBuff,
                     int32 * retLen)
{
    uint32          len;
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pBuff)
           || (NULL == retLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (OK != CtcExtOamObjGet(pathId,
                              LinkId,
                              objBranch,
                              objLeaf,
                              objIndex, branch, leaf, (uint8 *) pBuff,
                              &len)
        ) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    *retLen = len;

    return ret;
}

int32
TKCTCExtOamObjSetRaw(uint8 pathId, uint8 LinkId,
                     uint8 objBranch,
                     uint16 objLeaf,
                     uint32 objIndex,
                     uint8 branch, uint16 leaf,
                     uint8 * pBuff, uint8 retLen, uint8 * reCode)
{
    int32           ret = OK;
    uint8           retVal;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pBuff)
           || (NULL == reCode)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    retVal =
        CtcExtOamObjSet(pathId, LinkId,
                        objBranch, objLeaf,
                        objIndex, branch, leaf, (uint8 *) pBuff, retLen);

    if (OamVarErrNoError != retVal) {
        ret = ERROR;
    } else {
        ret = OK;
    }

    *reCode = retVal;

    return ret;
}

int32
TKCTCExtOamGetONUSN(uint8 pathId, uint8 LinkId, CtcOamOnuSN * pCtcOamOnuSN)
{
    /*
     * uint32 len; 
     */
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pCtcOamOnuSN)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (TRUE == CtcInfoFromONUPONChipsetCont.CtcInfoFromONUPONChipsSetInit) {
        sal_memcpy(pCtcOamOnuSN,
                   CtcInfoFromONUPONChipsetCont.CtcONUSN,
                   CtcInfoFromONUPONChipsetCont.CtcONUSNLen);
    } else {
        ret =
            TKCTCExtOamGetInfoFromONUPONChipsets
            (pathId, LinkId, &CtcInfoFromONUPONChipsetCont);
        if (OK == ret) {
            sal_memcpy(pCtcOamOnuSN,
                       CtcInfoFromONUPONChipsetCont.CtcONUSN,
                       CtcInfoFromONUPONChipsetCont.CtcONUSNLen);
        } else {
            ret = ERROR;
        }
    }

    return ret;
}

int32
TKCTCExtOamGetFirmwareVersion(uint8 pathId,
                              uint8 LinkId,
                              CtcOamFirmwareVersion
                              * pCtcOamFirmwareVersion, int32 * retLen)
{
    int32           ret = OK;

    if ((LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pCtcOamFirmwareVersion)
           || (NULL == retLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (TRUE == CtcInfoFromONUPONChipsetCont.CtcInfoFromONUPONChipsSetInit) {
        sal_memcpy(pCtcOamFirmwareVersion,
                   CtcInfoFromONUPONChipsetCont.CtcFirmwareVer,
                   CtcInfoFromONUPONChipsetCont.CtcFirmwareVerLen);
        *retLen = CtcInfoFromONUPONChipsetCont.CtcFirmwareVerLen;
    } else {
        ret =
            TKCTCExtOamGetInfoFromONUPONChipsets
            (pathId, LinkId, &CtcInfoFromONUPONChipsetCont);
        if (OK == ret) {
            sal_memcpy(pCtcOamFirmwareVersion,
                       CtcInfoFromONUPONChipsetCont.CtcFirmwareVer,
                       CtcInfoFromONUPONChipsetCont.CtcFirmwareVerLen);
            *retLen = CtcInfoFromONUPONChipsetCont.CtcFirmwareVerLen;
        } else {
            ret = ERROR;
        }
    }

    return ret;
}

int32
TKCTCExtOamGetChipsetId(uint8 pathId,
                        uint8 LinkId, CtcOamChipsetId * pCtcOamChipsetId)
{
    /*
     * uint32 len; 
     */
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pCtcOamChipsetId)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (TRUE == CtcInfoFromONUPONChipsetCont.CtcInfoFromONUPONChipsSetInit) {
        sal_memcpy(pCtcOamChipsetId,
                   CtcInfoFromONUPONChipsetCont.CtcChipsetId,
                   CtcInfoFromONUPONChipsetCont.CtcChipsetIdLen);
    } else {
        ret =
            TKCTCExtOamGetInfoFromONUPONChipsets
            (pathId, LinkId, &CtcInfoFromONUPONChipsetCont);
        if (OK == ret) {
            sal_memcpy(pCtcOamChipsetId,
                       CtcInfoFromONUPONChipsetCont.CtcChipsetId,
                       CtcInfoFromONUPONChipsetCont.CtcChipsetIdLen);
        } else {
            ret = ERROR;
        }
    }

    return ret;
}

int32
TKCTCExtOamGetONUCap(uint8 pathId, uint8 LinkId, uint8 * pCap,
                     int32 * retLen)
{
    /*
     * uint32 len; 
     */
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pCap)
           || (NULL == retLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (TRUE == CtcInfoFromONUPONChipsetCont.CtcInfoFromONUPONChipsSetInit) {
        sal_memcpy(pCap,
                   CtcInfoFromONUPONChipsetCont.CtcOnuCap1,
                   CtcInfoFromONUPONChipsetCont.CtcOnuCap1Len);
        *retLen = CtcInfoFromONUPONChipsetCont.CtcOnuCap1Len;
    } else {
        ret =
            TKCTCExtOamGetInfoFromONUPONChipsets
            (pathId, LinkId, &CtcInfoFromONUPONChipsetCont);
        if (OK == ret) {
            sal_memcpy(pCap,
                       CtcInfoFromONUPONChipsetCont.CtcOnuCap1,
                       CtcInfoFromONUPONChipsetCont.CtcOnuCap1Len);
            *retLen = CtcInfoFromONUPONChipsetCont.CtcOnuCap1Len;
        } else {
            ret = ERROR;
        }
    }

    return ret;
}

int32
TKCTCExtOamGetONUCap2(uint8 pathId, uint8 LinkId, uint8 * pCap,
                      int32 * retLen)
{
    /*
     * uint32 len; 
     */
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pCap)
           || (NULL == retLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (TRUE == CtcInfoFromONUPONChipsetCont.CtcInfoFromONUPONChipsSetInit) {
        sal_memcpy(pCap,
                   CtcInfoFromONUPONChipsetCont.CtcOnuCap2,
                   CtcInfoFromONUPONChipsetCont.CtcOnuCap2Len);
        *retLen = CtcInfoFromONUPONChipsetCont.CtcOnuCap2Len;
    } else {
        ret =
            TKCTCExtOamGetInfoFromONUPONChipsets
            (pathId, LinkId, &CtcInfoFromONUPONChipsetCont);
        if (OK == ret) {
            sal_memcpy(pCap,
                       CtcInfoFromONUPONChipsetCont.CtcOnuCap2,
                       CtcInfoFromONUPONChipsetCont.CtcOnuCap2Len);
            *retLen = CtcInfoFromONUPONChipsetCont.CtcOnuCap2Len;
        } else {
            ret = ERROR;
        }
    }

    return ret;
}

int32
TKCTCExtOamGetHoldOverConfig(uint8 pathId,
                             uint8 LinkId, uint8 * pCap, int32 * retLen)
{
    uint32          len;
    int32           ret = OK;

    if (                        /* (LinkId < SDK_LINK_VEC_BASE) || */
           (LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == pCap)
           || (NULL == retLen)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (OK != CtcExtOamGet(pathId,
                           LinkId,
                           OamCtcBranchExtAttribute,
                           OamCtcAttrLLIDQueueConfig, (uint8 *) pCap, &len)
        ) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    } else {
        *retLen = len;
    }

    return ret;
}

int32
TKCTCExtOamSetClsMarking(uint8 pathId,
                         uint8 linkId,
                         uint32 portNo,
                         uint8 action, uint8 ruleCnt,
                         CtcExtRule * pCtcExtRule)
{
    int32           retVal = OK;
    uint8          *pBuff = NULL;
    OamCtcTlvClassMarking *pOamCtcTlvClassMarking = NULL;
    OamCtcRule     *pOamCtcRule;
    OamCtcRuleCondition *pOamCtcRuleCondition;
    uint8           tmpRuleCnt = 0;
    uint8           tmpCondCnt = 0;
    uint16          tmpShortVal = 0x0;
    uint32          tmpLongVal = 0x0;
    uint32          txLen = 0;

    if (action > OamCtcRuleActionList) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pOamCtcTlvClassMarking = (OamCtcTlvClassMarking *) pBuff;

    pOamCtcTlvClassMarking->action = action;
    pOamCtcTlvClassMarking->numRules = ruleCnt;
    /* pOamCtcRule = pOamCtcTlvClassMarking->rule;*/

    if ((OamCtcRuleActionDel == action || OamCtcRuleActionAdd == action)
        && NULL != pCtcExtRule) {
        txLen = sizeof(OamCtcTlvClassMarking);

        for (tmpRuleCnt = 0; tmpRuleCnt < ruleCnt; tmpRuleCnt++) {
            pOamCtcRule = (OamCtcRule *) & (pBuff[txLen]);

            pOamCtcRule[0].prec = pCtcExtRule[tmpRuleCnt].Priority;
            pOamCtcRule[0].queueMapped =
                pCtcExtRule[tmpRuleCnt].QueueMapped;
            pOamCtcRule[0].pri = pCtcExtRule[tmpRuleCnt].EthernetPri;
            pOamCtcRule[0].numClause = pCtcExtRule[tmpRuleCnt].NumOfEntry;
            pOamCtcRule[0].length =
                sizeof(OamCtcRule) - 2 +
                sizeof(OamCtcRuleCondition) *
                (pCtcExtRule[tmpRuleCnt].NumOfEntry);
            pOamCtcRuleCondition = pOamCtcRule[0].clause;

            txLen += sizeof(OamCtcRule);
            txLen +=
                sizeof(OamCtcRuleCondition) *
                pCtcExtRule[tmpRuleCnt].NumOfEntry;

            for (tmpCondCnt = 0;
                 tmpCondCnt <
                 pCtcExtRule[tmpRuleCnt].NumOfEntry; tmpCondCnt++) {
                pOamCtcRuleCondition[tmpCondCnt].field =
                    pCtcExtRule[tmpRuleCnt].cond[tmpCondCnt].Select;

                sal_memcpy(pOamCtcRuleCondition
                           [tmpCondCnt].matchVal,
                           pCtcExtRule
                           [tmpRuleCnt].cond[tmpCondCnt].MatchVal, 6);

                switch (pOamCtcRuleCondition[tmpCondCnt].field) {
                case OamCtcFieldDaMac:
                case OamCtcFieldSaMac:
                case OamCtcFieldVlanPri:
                case OamCtcFieldIpTos:
                case OamCtcFieldIpPrec:
                    break;
                case OamCtcFieldVlanId:
                case OamCtcFieldEthertype:
                case OamCtcFieldL4SrcPort:
                case OamCtcFieldL4DestPort:
                    sal_memcpy(&tmpShortVal,
                               &(((uint8 *) &
                                  (pOamCtcRuleCondition[tmpCondCnt].
                                   matchVal))
                                 [4]), 2);
                    tmpShortVal = soc_htons(tmpShortVal);
                    sal_memcpy(&
                               (((uint8 *) &
                                 (pOamCtcRuleCondition[tmpCondCnt].
                                  matchVal))
                                [4]), &tmpShortVal, 2);
                    break;
                case OamCtcFieldDestIp:
                case OamCtcFieldSrcIp:
                    sal_memcpy(&tmpLongVal,
                               &(((uint8 *) &
                                  (pOamCtcRuleCondition[tmpCondCnt].
                                   matchVal))
                                 [2]), 2);
                    tmpLongVal = soc_htonl(tmpLongVal);
                    sal_memcpy(&
                               (((uint8 *) &
                                 (pOamCtcRuleCondition[tmpCondCnt].
                                  matchVal))
                                [2]), &tmpLongVal, 2);
                    break;
                }

                pOamCtcRuleCondition[tmpCondCnt].op =
                    pCtcExtRule[tmpRuleCnt].cond[tmpCondCnt].ValidOperator;
            }
        }
    } else if (OamCtcRuleActionClear == action) {
        txLen = sizeof(uint8);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        TkOamMemPut(pathId, pBuff);
        return ERROR;
    }

    retVal =
        CtcExtOamObjSet(pathId, linkId,
                        OamCtcBranchObjInst21,
                        OamCtcObjPort, portNo,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrClassMarking, (uint8 *) pBuff, txLen);

    if (OamVarErrNoError != retVal) {
        TkDbgTrace(TkDbgErrorEnable);
        retVal = ERROR;
    } else {
        retVal = OK;
    }
    TkOamMemPut(pathId, pBuff);
    return retVal;
}

int32
TKCTCExtOamSetLLIDQueueConfig(uint8 pathId,
                              uint8 linkId,
                              uint32 LLID,
                              uint8 numOfQ, CtcExtQConfig * pCtcExtQConfig)
{
    int32           retVal = OK;
    uint8          *pBuff = NULL;
    OamCtcLLIDQconfig *pOamCtcLLIDQconfig = NULL;
    OamCtcLLIDQAttr *pOamCtcLLIDQAttr = NULL;
    uint32          count;
    uint8           txLen;

    if (NULL == pCtcExtQConfig) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pOamCtcLLIDQconfig = (OamCtcLLIDQconfig *) pBuff;
    pOamCtcLLIDQAttr = pOamCtcLLIDQconfig->QAttrList;

    pOamCtcLLIDQconfig->numOfQ = numOfQ;

    for (count = 0; count < numOfQ; count++) {
        pOamCtcLLIDQAttr[count].Qid = soc_htons(pCtcExtQConfig[count].Qid);
        pOamCtcLLIDQAttr[count].QWrr =
            soc_htons(pCtcExtQConfig[count].QWrr);
    }

    txLen = sizeof(OamCtcLLIDQconfig) + numOfQ * sizeof(OamCtcLLIDQAttr);

    retVal =
        CtcExtOamObjSet(pathId, linkId,
                        OamCtcBranchObjInst21,
                        OamCtcObjLLID, LLID,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrLLIDQueueConfig, (uint8 *) pBuff, txLen);

    if (OamVarErrNoError != retVal) {
        TkOamMemPut(pathId, pBuff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    } else {
        TkOamMemPut(pathId, pBuff);
        return OK;
    }
}

/*
 * TKCTCExtOamSetLaserTxPowerAdminCtl: set laser Tx power admin control
 * 
 * set the laser control as CTC specification define
 * 
 *  Parameters:
 *  \pathId the PON chipset index which you want to change
 *  \linkId the link index which you want to change in the PON chipset
 *  \pCtcExtONUTxPowerSupplyControl the control value
 *  \return
 *  OK or ERROR
 */

int32
TKCTCExtOamSetLaserTxPowerAdminCtl(uint8 pathId,
                                   uint8 linkId,
                                   CtcExtONUTxPowerSupplyControl
                                   * pCtcExtONUTxPowerSupplyControl)
{
    int32           retVal = OK;
    uint8           ret = OamVarErrActBadParameters;
    uint8          *pBuff = NULL;
    CtcOamTlvTxPowerCtrl *pCtcOamTlvTxPowerCtrl = NULL;
    uint16          cntOfSeconds;
    uint8           txLen;

    if (NULL == pCtcExtONUTxPowerSupplyControl) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (POWER_DOWN_LASER_TX_POWER ==
        pCtcExtONUTxPowerSupplyControl->Action) {
        cntOfSeconds = soc_ntohs(POWER_DOWN_LASER_TX_POWER);

        ret =
            TkExtOamSet(pathId, 0,
                        OamBranchAction,
                        OamExtActLaserPowerOffTime,
                        (uint8 *) & cntOfSeconds, sizeof(uint16));
    } else {
        pCtcOamTlvTxPowerCtrl = (CtcOamTlvTxPowerCtrl *) pBuff;

        pCtcOamTlvTxPowerCtrl->action
            = soc_htonl(pCtcExtONUTxPowerSupplyControl->Action);
        pCtcOamTlvTxPowerCtrl->optId =
            soc_htonl(pCtcExtONUTxPowerSupplyControl->
                      OpticalTransmitterID);
        sal_memcpy((void *) (&(pCtcOamTlvTxPowerCtrl->onuId)), (void *)
                   &(pCtcExtONUTxPowerSupplyControl->ONUID),
                   sizeof(MacAddr));

        txLen = sizeof(CtcOamTlvTxPowerCtrl);

        ret =
            CtcExtOamSet(pathId, linkId,
                         OamCtcBranchExtAttribute,
                         OamCtcAttrOnuTxPowerSupplyControl,
                         (uint8 *) pBuff, txLen);
    }

    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        retVal = ERROR;
    } else {
        retVal = OK;
    }

    TkOamMemPut(pathId, pBuff);
    return retVal;
}

/*
 * Function:
 *      CtcExtOamSetMulLlidCtrl
 * Purpose:
 *      Enable or disable multiple llid function of the EPON chipset
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      llidNum - 0/1 - single LLID, (2 - x) - multi LLID
 * Returns:
 *      RcOk or Other Fail 
 * Notes:
 *      
 */
int32 
CtcExtOamSetMulLlidCtrl (uint8 pathId, uint32 llidNum)
{
    uint32 tmp;
    uint8  ret;
    if ((llidNum > MAX_CNT_OF_LINK))
        return RcBadParam;

    tmp = soc_htonl (llidNum);

    ret = CtcExtOamSet ( pathId, 0, OamCtcBranchExtAction, 
        OamCtcActMultLLIDAdminCtl, (uint8 *)&tmp, 4 );
    if ( ret != OamVarErrNoError )
        return RcFail;

    return RcOk;   
}

/*
 * Function:
 *      CtcExtOamSetHoldover
 * Purpose:
 *      Set the hold over time of the EPON chipset
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state   - Active or Disactive, 1 for disactive while 1 for active.   
 *      time  - unit in ms range from 50-1000
 * Returns:
 *      RcOk or Other Fail 
 * Notes:
 *      
 */
int32 
CtcExtOamSetHoldover (uint8 pathId, uint8 linkId, uint32 state, 
    uint32 time)
{
    TkCtcHoldover holdOver;
    int ret = RcOk;

    if((MAX_CNT_OF_LINK <= linkId) || (HOLDOVERDISACTIVATED != state 
        && HOLDOVERACTIVATED != state )){
        return RcBadParam;
    }

    
    holdOver.state = soc_htonl(state);
    holdOver.time = soc_htonl(time);
    
    ret = CtcExtOamSet(pathId, linkId, OamCtcBranchExtAttribute,
        OamCtcAttrHoldoverConfig, (uint8 *)&holdOver, sizeof(TkCtcHoldover) );     
    if ( ret != OamVarErrNoError )
        return RcFail;

    return RcOk;
}

/*
 * Function:
 *      TkCtcExtOamGetHoldover
 * Purpose:
 *      Get the hold over time of the EPON chipset
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state   - Active or Disactive, 1 for disactive while 1 for active.   
 *      time  - unit in ms range from 50-1000
 * Returns:
 *      RcOk or Other Fail 
 * Notes:
 *      
 */
int32 
CtcExtOamGetHoldover(uint8 pathId, uint8 linkId, uint32 *state, 
    uint32 *time)
{
    uint32 DataLen;
    TkCtcHoldover holdOver;
    uint8 *pBuff = NULL;
    int ret = RcOk;
    
    if ((linkId >= MAX_CNT_OF_LINK) || (NULL == state) || (NULL == time))
        return RcBadParam;
    
    pBuff = (uint8 *)TkOamMemGet(pathId);
    
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret = CtcExtOamGet( pathId, linkId, OamCtcBranchExtAttribute, 
        OamCtcAttrHoldoverConfig, pBuff, &DataLen );
    
    if ( ret != OK ){
        ret = RcFail;
    }else{	
    	sal_memcpy(&holdOver, pBuff, sizeof(TkCtcHoldover));
    
    	*state = soc_ntohl (holdOver.state);
    	*time  = soc_ntohl (holdOver.time);
	    ret = RcOk;
    }
    TkOamMemPut (pathId, (void *)pBuff);
    return ret;
}

int 
CtcExtOamGetAlarmState(uint8 pathId, uint8 linkId, uint16 port, 
    uint16 alarmid, uint8 *state)
{	
   	uint8 *buff;
	uint32 len;
	uint32 aconfig;
	CtcOamObj21 obj;
	uint16 tmp;
    int ret = OK;
    uint32 objInst = 0;

    GetObjFromAlarm(alarmid,port,&obj);

    CtcExtOam21ObjInstPack(obj.type, port, &objInst);
    		
	buff = (uint8 *)TkOamMemGet(pathId);
	if (NULL == buff){
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

	tmp = soc_htons(alarmid);

    ret = CtcExtOamObjActGet(pathId, linkId, OamCtcBranchObjInst21,
		    obj.type, objInst, OamCtcBranchExtAttribute, 
		    OamCtcAttrAlarmAdminState, sizeof(uint16), (uint8*)&tmp, buff, &len);

    if(OK != ret){
		TkOamMemPut (pathId, (void *)buff);
		TkDbgTrace(TkDbgErrorEnable);
		return ERROR;
    }

	sal_memcpy(&aconfig, buff+sizeof(uint16), sizeof(uint32));
	
    switch(aconfig){
        case CTCALMSTATEACTIVED:
            *state = TRUE;
            ret = OK;
            break;
        case CTCALMSTATEDEACTIVED:
            *state = FALSE;
            ret = OK;
            break;
        default:
            ret = ERROR;
            break;
    }
	TkOamMemPut (pathId, (void *)buff);
    return ret;
}

int 
CtcExtOamSetAlarmState(uint8 pathId,uint8 linkId,  uint16 port,  
        uint16 alarmid, uint8 state)
{
    CtcOamAlarmState alarmState;
	CtcOamObj21 obj;
	int ret;
    uint32 objInst;
	
	GetObjFromAlarm(alarmid, port, &obj);

    switch(state){
        case TRUE:
            alarmState.alarmState = CTCALMSTATEACTIVED;
            break;
        case FALSE:
            alarmState.alarmState = CTCALMSTATEDEACTIVED;
            break;
        default:
            return ERROR;
            break;
    }

    CtcExtOam21ObjInstPack(obj.type, port, &objInst);

    alarmState.alarmId = soc_htons(alarmid);
    alarmState.alarmState = soc_htonl(alarmState.alarmState);
    
    ret = CtcExtOamObjSet( pathId, linkId, OamCtcBranchObjInst21,
	    obj.type, objInst,OamCtcBranchExtAttribute, OamCtcAttrAlarmAdminState, 
	    (uint8 *)&alarmState, sizeof(CtcOamAlarmState));

	if(OamVarErrNoError != ret ) {
		TkDbgTrace(TkDbgErrorEnable);
		return ERROR;
	}
		
    return OK;
}

int 
CtcExtOamGetAlarmThreshold(uint8 pathId,uint8 linkId,  
    uint16 port,  uint16 alarmid, CtcOamAlarmThreshold *threshold)
{
	CtcOamObj21 obj;
   	uint8 *buff;	
	uint32 len;
	uint16 tmp;
    uint32 objInst;

	if (threshold == NULL){
		return ERROR;
    }
	
	GetObjFromAlarm( alarmid, port, &obj );

    CtcExtOam21ObjInstPack(obj.type, port, &objInst);
	
	buff = (uint8 *)TkOamMemGet(pathId);
	if (NULL == buff){
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

	tmp = soc_htons( alarmid );
	if(OK != CtcExtOamObjActGet( pathId, linkId, OamCtcBranchObjInst21,
		obj.type, objInst, OamCtcBranchExtAttribute,OamCtcAttrAlarmThreshold, 
		sizeof(uint16), (uint8*)&tmp, buff, &len)){
		TkOamMemPut (pathId, (void *)buff);
		TkDbgTrace(TkDbgErrorEnable);
		return ERROR;
	}
    
	sal_memcpy( threshold, buff+sizeof(uint16), sizeof(CtcOamAlarmThreshold) );
	threshold->raiseThreshold = soc_ntohl( threshold->raiseThreshold );
	threshold->clearThreshold = soc_ntohl( threshold->clearThreshold );	

	TkOamMemPut (pathId, (void *)buff);
	
    return OK;
}
	
int 
CtcExtOamSetAlarmThreshold(uint8 pathId,uint8 linkId, uint16 port,  
    uint16 alarmid, CtcOamAlarmThreshold threshold)
{
   	uint8 buff[10];
	CtcOamObj21 obj;
	int ret;
	uint32 objInst = 0;
    
	GetObjFromAlarm( alarmid, port, &obj );
	
	alarmid = soc_htons( alarmid );
	sal_memcpy(buff, &alarmid, sizeof(uint16));
    
    CtcExtOam21ObjInstPack(obj.type, port, &objInst);
	
	threshold.raiseThreshold      = soc_htonl( threshold.raiseThreshold );
	threshold.clearThreshold = soc_htonl( threshold.clearThreshold );	
	sal_memcpy( buff+sizeof(uint16), &threshold, sizeof(CtcOamAlarmThreshold) );

	ret = CtcExtOamObjSet( pathId, linkId, OamCtcBranchObjInst21,
		obj.type, objInst, OamCtcBranchExtAttribute, 
		OamCtcAttrAlarmThreshold, buff, sizeof(uint16)+sizeof(CtcOamAlarmThreshold));

    if(OamVarErrNoError != ret ){
		TkDbgTrace(TkDbgErrorEnable);
		return ERROR;
    }
		
    return OK;
}

int32
CtcExtOamGetOptTransDiag(uint8 path_id, uint8 link_id, 
    CtcOamTlvPowerMonDiag *info)
{
    uint8 *buff;
    uint32 len;
    int rv;

    if((link_id >= MAX_CNT_OF_LINK) || (NULL == info)){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    sal_memset((void *)info, 0x0, sizeof(CtcOamTlvPowerMonDiag));

    buff = (uint8 *)TkOamMemGet(path_id);

    if(NULL == buff){
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    rv = CtcExtOamGet(path_id, link_id, OamCtcBranchExtAttribute, 
        OamCtcAttrOptTranDiag, buff, &len);

    if(OK == rv){
        sal_memcpy( info, buff, sizeof(CtcOamTlvPowerMonDiag) );  
        info->temp    = soc_ntohs( info->temp );
        info->vcc     = soc_ntohs( info->vcc );
        info->txBias  = soc_ntohs( info->txBias );
        info->txPower = soc_ntohs( info->txPower );
        info->rxPower = soc_ntohs( info->rxPower );
    }

    TkOamMemPut(path_id, (void *)buff);

    return rv;
}


int32
CtcExtOamSetPonIfAdmin(uint8 path_id, uint8 link_id, uint8 optical_no)
{
    uint8 ret;
    uint8 buf = optical_no;
    int rv = OK;
    
    if(link_id >= SDK_MAX_NUM_OF_LINK){
        return ERROR;
    }

    ret = CtcExtOamSet(path_id, link_id, OamCtcBranchExtAttribute, 
        OamCtcAttrPonIfAdmin, &buf, 1);

    if(ret != OamVarErrNoError){
        rv = ERROR;
    }else{
        rv = OK;
    }

    return rv;
}

int 
CtcExtOamGetPonIfAdmin(uint8 path_id, uint8 link_id, uint8 *optical_no)
{
    int rv;
   	uint8 *buff;	
    uint32 data_len;

    if((NULL == optical_no) || (link_id >= SDK_MAX_NUM_OF_LINK)){
        return ERROR;
    }
    
    buff = (uint8 *)TkOamMemGet(path_id);
    if(NULL == buff){
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    rv = CtcExtOamGet(path_id, link_id, OamCtcBranchExtAttribute, 
        OamCtcAttrPonIfAdmin, buff, &data_len);

    if((OK != rv) || (data_len != 1)){
        return ERROR;
    }

    *optical_no = buff[0];

    return OK;
}

