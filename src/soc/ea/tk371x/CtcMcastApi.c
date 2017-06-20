/*
 * $Id: CtcMcastApi.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcMcastApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/CtcOam.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/TkUtils.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/OamUtilsCtc.h>
#include <soc/ea/tk371x/CtcMcastApi.h>

/*
 * Function:
 *      CtcExtOamAddMulticastVlan
 * Purpose:
 *      Add a port into the multicast VALN list as CTC specification
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port of which will be added into the multicast VALN.
 *      cntOfMulticastVlan - Multicast VLAN count
 *      multicastVlan - Multicast Vlan list
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int             
CtcExtOamAddMulticastVlan
    (uint8 pathId,
     uint8 LinkId,
     uint32 port, uint32 cntOfMulticastVlan, uint16 * multicastVlan) 
{
    uint8          *buff;
    OamCtcTlvMcastVlan *pMcastTlv;
    int             cnt;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == multicastVlan)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pMcastTlv = (OamCtcTlvMcastVlan *) buff;
    pMcastTlv->operation = OamCtcMcastVlanAdd;
    {
        VlanTag        *pVid = pMcastTlv->vid;
        for (cnt = 0; cnt < cntOfMulticastVlan; cnt++) {
            pVid[cnt] = soc_htons(multicastVlan[cnt]);
        }
    }
    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrMcastVlan,
                        buff,
                        sizeof(uint8) +
                        sizeof(VlanTag) * cntOfMulticastVlan)) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    }
    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamDelMulticastVlan
 * Purpose:
 *      Remove the port from the multicast VLAN list
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port of which will be remvoed from the multicast VALN.
 *      cntOfMulticastVlan - Multicast VLAN count
 *      multicastVlan - Multicast Vlan list
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamDelMulticastVlan
    (uint8 pathId,
     uint8 LinkId,
     uint32 port, uint32 cntOfMulticastVlan, uint16 * multicastVlan) 
{
    uint8          *buff;
    OamCtcTlvMcastVlan *pMcastTlv;
    int             cnt;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == multicastVlan)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pMcastTlv = (OamCtcTlvMcastVlan *) buff;
    pMcastTlv->operation = OamCtcMcastVlanDel;
    {
        VlanTag        *pVid = pMcastTlv->vid;
        for (cnt = 0; cnt < cntOfMulticastVlan; cnt++) {
            pVid[cnt] = soc_htons(pVid[cnt]);
        }
    }
    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrMcastVlan,
                        buff,
                        sizeof(uint8) +
                        sizeof(VlanTag) * cntOfMulticastVlan)) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    }
    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamClearMulticastVlan
 * Purpose:
 *      Clear all multicast vlan binding to this port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port of which will be remvoed from the multicast VALN.
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int
CtcExtOamClearMulticastVlan(uint8 pathId, uint8 LinkId, uint32 port)
{
    uint8          *buff;
    OamCtcTlvMcastVlan *pMcastTlv;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) || (port < SDK_PORT_VEC_BASE)
           || (port > SDK_MAX_NUM_OF_PORT)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pMcastTlv = (OamCtcTlvMcastVlan *) buff;
    pMcastTlv->operation = OamCtcMcastVlanClear;

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrMcastVlan, buff, sizeof(uint8))) {
        TkOamMemPut(pathId,(void *) buff);
        return OK;
    } else {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * Function:
 *      CtcExtOamListMulticastVlan
 * Purpose:
 *      List all the multicast vlan bindig to this port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port of which will be remvoed from the multicast VALN.
 *      cntOfMulticastVlan - Multicast VLAN count
 *      multicastVlan - Multicast Vlan list
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamListMulticastVlan
    (uint8 pathId,
     uint8 LinkId,
     uint32 port, uint32 * cntOfMulticastVlan, uint16 * multicastVlan) 
{
    uint8          *buff;
    OamCtcTlvMcastVlan *pMcastTlv;
    int             cnt;
    uint32          len;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) ||
           (port > SDK_MAX_NUM_OF_PORT) || (NULL == cntOfMulticastVlan)
           || (NULL == multicastVlan)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pMcastTlv = (OamCtcTlvMcastVlan *) buff;

    if (OK != CtcExtOamObjGet(pathId,
                              LinkId,
                              OamCtcBranchObjInst,
                              OamCtcContextPort,
                              port,
                              OamCtcBranchExtAttribute,
                              OamCtcAttrMcastVlan, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    len = (len - 1) / 2;

    for (cnt = 0; cnt < len; cnt++) {
        multicastVlan[cnt] = soc_ntohs(pMcastTlv->vid[cnt]);
    }
    *cntOfMulticastVlan = cnt;

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamSetMulticastTagstripe
 * Purpose:
 *      Set strip tag of the port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port
 *      stripeOp - True for strip enable while false for not strip
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamSetMulticastTagstripe
    (uint8 pathId, uint8 LinkId, uint32 port, Bool stripeOp) 
{
    uint8           opeartion;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) ||
           (port > SDK_MAX_NUM_OF_PORT) || ((stripeOp != TRUE)
                                            && (stripeOp != FALSE))
        ) {
        return (OamVarErrActBadParameters);
    }

    opeartion = stripeOp;

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrMcastStrip, &opeartion, sizeof(uint8))) {
        return OK;
    }
    return OK;
}

/*
 * Function:
 *      CtcExtOamGetMulticastTagstripe
 * Purpose:
 *      Get the strip status
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port 
 *      stripeOp - Ture for strip enable while false for not strip
 *      multicastVlan - Multicast Vlan list
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamGetMulticastTagstripe
    (uint8 pathId, uint8 LinkId, uint32 port, Bool * stripeOp) 
{
    uint8          *buff;
    uint32          len;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == stripeOp)
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
                              OamCtcAttrMcastStrip, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    *stripeOp = buff[0];

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamSetMulticastSwitch
 * Purpose:
 *      Switch between CTC multicast control and igmp snooping
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port
 *      multicastMode - mode.
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamSetMulticastSwitch
    (uint8 pathId, uint8 LinkId, OamCtcMcastMode multicastMode) 
{
    uint8           opeartion;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (OamCtcMcastIgmpMode != multicastMode
            && OamCtcMcastHostMode != multicastMode)
        ) {
        return (OamVarErrActBadParameters);
    }
    opeartion = multicastMode;

    if (OamVarErrNoError == CtcExtOamSet(pathId,
                                         LinkId,
                                         OamCtcBranchExtAttribute,
                                         OamCtcAttrMcastSwitch,
                                         &opeartion, sizeof(uint8))) {
        return OK;
    }

    return OK;
}

/*
 * Function:
 *      CtcExtOamGetMulticastSwitch
 * Purpose:
 *      Get the multicast control mode. CTC multicast control or igmp snooping
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port of which will be remvoed from the multicast VALN.
 *      multicastMode - Multicast mode
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamGetMulticastSwitch
    (uint8 pathId, uint8 LinkId, OamCtcMcastMode * multicastMode) 
{
    uint8          *buff;
    uint32          len;

    if ((LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == multicastMode)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    if (OK != CtcExtOamGet(pathId,
                           LinkId,
                           OamCtcBranchExtAttribute,
                           OamCtcAttrMcastSwitch, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    *multicastMode = buff[0];

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamSetMulticastControl
 * Purpose:
 *      Set the a multicast entry as CTC spec
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      McastCtrlOp   - Operation
 *      McastCtrlType  - Mulicast  control type
 *      cntOfMcastEntry - Count of entries
 *      pMcastEntry - Entry list
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamSetMulticastControl
    (uint8 pathId,
     uint8 LinkId,
     OamCtcMcastGroupOp McastCtrlOp,
     McastCtrl McastCtrlType, uint8 cntOfMcastEntry,
     McastEntry * pMcastEntry)
{
    uint8          *buff;
    CtcOamMcastVlan *pOamMcastVlan;
    uint32          tmpCnt;
    uint32          txLen = 0;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (McastCtrlOp > OamCtcMcastGroupClear)
           || ((NULL == pMcastEntry)
               && (McastCtrlOp != OamCtcMcastGroupClear)
               && (McastCtrlOp != OamCtcMcastGroupList))
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pOamMcastVlan = (CtcOamMcastVlan *) buff;

    pOamMcastVlan->operation = McastCtrlOp;
    pOamMcastVlan->type = McastCtrlType;

    if (OamCtcMcastGroupClear == McastCtrlOp) {
        pOamMcastVlan->cntOfMcast = 0;
        txLen = sizeof(uint8);
    } else {
        pOamMcastVlan->cntOfMcast = cntOfMcastEntry;
        for (tmpCnt = 0; tmpCnt < cntOfMcastEntry; tmpCnt++) {
            pOamMcastVlan->McastEntryArry[tmpCnt].userId =
                soc_htons(pMcastEntry[tmpCnt].userId);
            pOamMcastVlan->McastEntryArry[tmpCnt].vlanTag =
                soc_htons(pMcastEntry[tmpCnt].vlanTag);
            sal_memcpy(pOamMcastVlan->McastEntryArry[tmpCnt].da.u8,
                       pMcastEntry[tmpCnt].da.u8, 6);
        }
        txLen =
            sizeof(uint8) + sizeof(uint8) +
            sizeof(uint8) + sizeof(McastEntry) * cntOfMcastEntry;
    }

    /* coverity[identical_branches:FALSE] */
    if (OamVarErrNoError == CtcExtOamSet(pathId,
                                         LinkId,
                                         OamCtcBranchExtAttribute,
                                         OamCtcAttrMcastCtrl, buff,
                                         txLen)) {
        return OK;
    }
    return OK;
}

/*
 * Function:
 *      CtcExtOamListMulticastControl
 * Purpose:
 *      Get the multicast control list
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      McastCtrlType  - Entry type
 *      cntOfMcastEntry - Entry count
 *      pMcastEntry - Entry list
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamListMulticastControl
    (uint8 pathId,
     uint8 LinkId,
     McastCtrl * McastCtrlType,
     uint8 * cntOfMcastEntry, McastEntry * pMcastEntry) 
{
    uint8          *buff;
    uint32          len;
    CtcOamMcastVlan *pOamMcastVlan;
    uint32          tmpCnt;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (NULL == McastCtrlType) || (NULL == cntOfMcastEntry)
           || (NULL == pMcastEntry)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pOamMcastVlan = (CtcOamMcastVlan *) buff;

    if (OK != CtcExtOamGet(pathId,
                           LinkId,
                           OamCtcBranchExtAttribute,
                           OamCtcAttrMcastCtrl, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (CtcMcastApiDbgEnable) {
        TkDbgDataDump(buff, len, 16);
    }

    len = (len - 3 * sizeof(uint8)) / sizeof(McastEntry);

    *McastCtrlType = pOamMcastVlan->type;
    *cntOfMcastEntry = pOamMcastVlan->cntOfMcast;

    for (tmpCnt = 0; tmpCnt < len; tmpCnt++) {
        pMcastEntry[tmpCnt].userId =
            soc_ntohs(pOamMcastVlan->McastEntryArry[tmpCnt].userId);
        pMcastEntry[tmpCnt].vlanTag =
            soc_ntohs(pOamMcastVlan->McastEntryArry[tmpCnt].vlanTag);
        sal_memcpy(pMcastEntry[tmpCnt].da.u8,
                   pOamMcastVlan->McastEntryArry[tmpCnt].da.u8, 6);
    }

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamListPortMulticastControl
 * Purpose:
 *      Get the multicast control entry list binding to the port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port of which will be remvoed from the multicast VALN.
 *      McastCtrlType - Entry type
 *      cntOfMcastEntry - Entry count
 *      pMcastEntry - Entry list
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamListPortMulticastControl
    (uint8 pathId,
     uint8 LinkId,
     uint32 port,
     McastCtrl * McastCtrlType,
     uint8 * cntOfMcastEntry, McastEntry * pMcastEntry) 
{
    uint8          *buff;
    uint32          len;
    CtcOamMcastVlan *pOamMcastVlan;
    uint32          tmpCnt;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (NULL == McastCtrlType) ||
           (NULL == cntOfMcastEntry) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == pMcastEntry)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    pOamMcastVlan = (CtcOamMcastVlan *) buff;

    if (OK != CtcExtOamObjGet(pathId,
                              LinkId,
                              OamCtcBranchObjInst,
                              OamCtcContextPort,
                              port,
                              OamCtcBranchExtAttribute,
                              OamCtcAttrMcastCtrl, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    len = (len - 3 * sizeof(uint8)) / sizeof(McastEntry);

    *McastCtrlType = pOamMcastVlan->type;
    *cntOfMcastEntry = pOamMcastVlan->cntOfMcast;

    for (tmpCnt = 0; tmpCnt < len; tmpCnt++) {
        pMcastEntry[tmpCnt].userId =
            soc_ntohs(pOamMcastVlan->McastEntryArry[tmpCnt].userId);
        pMcastEntry[tmpCnt].vlanTag =
            soc_ntohs(pOamMcastVlan->McastEntryArry[tmpCnt].vlanTag);
        sal_memcpy(pMcastEntry[tmpCnt].da.u8,
                   pOamMcastVlan->McastEntryArry[tmpCnt].da.u8, 6);
    }

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamSetMulticastMaxGroupNum
 * Purpose:
 *      Set the max multicast group count of the port
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port 
 *      maxGroupNum - Max count
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamSetMulticastMaxGroupNum
    (uint8 pathId, uint8 LinkId, uint32 port, uint8 maxGroupNum) 
{
    uint8           GroupNum = maxGroupNum;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) || (port < SDK_PORT_VEC_BASE)
           || (port > SDK_MAX_NUM_OF_PORT)
        ) {
        return (OamVarErrActBadParameters);
    }

    if (OamVarErrNoError ==
        CtcExtOamObjSet(pathId, LinkId,
                        OamCtcBranchObjInst,
                        OamCtcContextPort, port,
                        OamCtcBranchExtAttribute,
                        OamCtcAttrGroupMax, &GroupNum, sizeof(uint8))) {
        return OK;
    }

    return OK;
}

/*
 * Function:
 *      CtcExtOamGetMulticastMaxGroupNum
 * Purpose:
 *      Get the max multicast count of the port 
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      port     - The UNI port
 *      maxGroupNum - Max count
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int            
CtcExtOamGetMulticastMaxGroupNum
    (uint8 pathId, uint8 LinkId, uint32 port, uint8 * maxGroupNum) 
{
    uint8          *buff;
    uint32          len;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) ||
           (port < SDK_PORT_VEC_BASE) || (port > SDK_MAX_NUM_OF_PORT)
           || (NULL == maxGroupNum)
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
                              OamCtcAttrGroupMax, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    *maxGroupNum = buff[0];

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamGetaFastLeaveAblity
 * Purpose:
 *      Get the fast leave ability as CTC spec
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      cntOfEnum     - Support type count
 *      pEnumValue - Value list
 *      multicastVlan - Multicast Vlan list
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamGetaFastLeaveAblity
    (uint8 pathId, uint8 LinkId, uint32 * cntOfEnum, uint32 * pEnumValue) 
{
    uint8          *buff;
    uint32          len;
    uint32          numOfEnum;
    uint32          tmpCnt;
    uint32         *pU32;

    if ((LinkId > SDK_MAX_NUM_OF_LINK) || (NULL == cntOfEnum)
           || (NULL == pEnumValue)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    if (OK != CtcExtOamGet(pathId,
                           LinkId,
                           OamCtcBranchExtAttribute,
                           OamCtcAttrFastLeaveAbility, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pU32 = (uint32 *) & buff[0];
    numOfEnum = soc_ntohl(pU32[0]);
    for (tmpCnt = 0; tmpCnt < (len / 4) - 1; tmpCnt++) {
        pEnumValue[tmpCnt] = soc_ntohl(pU32[1 + tmpCnt]);
    }

    *cntOfEnum = numOfEnum;

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamaFastLeaveAdminState
 * Purpose:
 *      Get the fast leave admin status
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      adminState     - 0X00000001 disae while 0x00000002 means enable
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamaFastLeaveAdminState
    (uint8 pathId, uint8 LinkId, uint32 * adminState) 
{
    uint8          *buff;
    uint32          len;

    if ((LinkId > SDK_MAX_NUM_OF_LINK)
           || (NULL == adminState)
        ) {
        return (OamVarErrActBadParameters);
    }

    buff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == buff) {
        return (OamVarErrActNoResources);
    }

    if (OK != CtcExtOamGet(pathId,
                           LinkId,
                           OamCtcBranchExtAttribute,
                           OamCtcAttrFastLeaveState, buff, &len)) {
        TkOamMemPut(pathId,(void *) buff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    *adminState = soc_ntohl(*(uint32 *) & buff[0]);

    TkOamMemPut(pathId,(void *) buff);
    return OK;
}

/*
 * Function:
 *      CtcExtOamacFastLeaveAdminControl
 * Purpose:
 *      Remove the port from the multicast VLAN list
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      adminState     - 1 disactive while 2 means active
 * Returns:
 *      ERROR or 0
 * Notes:
 */
int             
CtcExtOamacFastLeaveAdminControl(uint8 pathId, uint8 LinkId, 
    uint32 adminState) 
{
    uint32          state;

    if ((LinkId > SDK_MAX_NUM_OF_LINK)) {
        return (OamVarErrActBadParameters);
    }
    state = soc_htonl(adminState);

    if (OamVarErrNoError == CtcExtOamSet(pathId,
                                         LinkId,
                                         OamCtcBranchExtAction,
                                         OamCtcActFastLeaveAdminCtl,
                                         (uint8 *) & state,
                                         sizeof(uint32))) {
        return OK;
    }

    return OK;
}

