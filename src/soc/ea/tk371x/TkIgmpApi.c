/*
 * $Id: TkIgmpApi.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkIgmpApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h> 
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkInit.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/TkIgmpApi.h>

/*
 * send TK extension OAM message Set igmp configuration to the ONU 
 */
uint8
TkExtOamSetIgmpConfig(uint8 pathId, uint8 LinkId,
                      OamIgmpConfig * pIgmpConfig)
{
    uint8           size;

    if ((LinkId > 7) || (NULL == pIgmpConfig))
        return (OamVarErrActBadParameters);

    size = (3 * sizeof(uint8)) +
        (pIgmpConfig->numPorts * sizeof(OamIgmpPortConfig));
    return (TkExtOamSet
            (pathId, LinkId, OamBranchAction,
             OamExtActSetIgmpConfig, (uint8 *) pIgmpConfig, size));
}


/*
 * send TK extension OAM message Get igmp configuration from the ONU 
 */
int
TkExtOamGetIgmpConfig(uint8 pathId, uint8 LinkId,
                      OamIgmpConfig * pIgmpConfig)
{
    uint32          DataLen;

    if ((LinkId > 7) || (NULL == pIgmpConfig)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
    if (OK ==
        TkExtOamGet(pathId, LinkId,
                    OamBranchAction,
                    OamExtActGetIgmpConfig, (uint8 *) pIgmpConfig,
                    &DataLen))
        return (OK);
    else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Get igmp group info from the ONU 
 */
int
TkExtOamGetIgmpGroupInfo(uint8 pathId,
                         uint8 LinkId, OamIgmpGroupConfig * pIgmpGroupInfo)
{
    uint32          DataLen;
    uint8           *rxTmpBuf = (uint8*)TkGetApiBuf(pathId);
    
    if ((LinkId > 7) || (NULL == pIgmpGroupInfo)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (OK ==
        TkExtOamGetMulti(pathId, LinkId,
                         OamBranchAction,
                         OamExtActGetIgmpGroupInfo,
                         (uint8 *) rxTmpBuf, &DataLen)) {
        OamVarContainer *var;
        uint8          *numGroups;
        OamIgmpGroupConfig *info = NULL;
        OamIgmpGroupInfo *pGgroup;

        numGroups = (uint8 *) pIgmpGroupInfo;
        pGgroup = (OamIgmpGroupInfo *) INT_TO_PTR(PTR_TO_INT(pIgmpGroupInfo) + sizeof(uint8));
        *numGroups = 0;
        var = (OamVarContainer *) rxTmpBuf;

        while (var->branch != OamBranchTermination) {
            if ((var->branch == OamBranchAction)
                && (var->leaf == soc_ntohs(OamExtActGetIgmpGroupInfo))) {
                info = (OamIgmpGroupConfig *) (var->value);
                *numGroups += info->numGroups;

                bcopy((uint8 *) info->group,
                      (uint8 *) pGgroup,
                      info->numGroups * sizeof(OamIgmpGroupInfo));
            }
            /*info may be NULL*/
            if(NULL != info){
                pGgroup += info->numGroups;
                var = NextCont(var);
            }else{
                break;
            }   
        }

        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * send TK extension OAM message Set delete igmp group to the ONU 
 */
uint8
TkExtOamSetDelIgmpGroup(uint8 pathId,
                        uint8 LinkId, OamIgmpGroupConfig * pIgmpGroup)
{
    uint8           size;

    if ((LinkId > 7) || (NULL == pIgmpGroup))
        return (OamVarErrActBadParameters);

    size =
        sizeof(uint8) + (pIgmpGroup->numGroups * sizeof(OamIgmpGroupInfo));
    return (TkExtOamSet
            (pathId, LinkId, OamBranchAction, OamExtActDelIgmpGroup,
             (uint8 *) pIgmpGroup, size));
}


/*
 * send TK extension OAM message Set add igmp group to the ONU 
 */
uint8
TkExtOamSetAddIgmpGroup(uint8 pathId,
                        uint8 LinkId, OamIgmpGroupConfig * pIgmpGroup)
{
    uint8           size;

    if ((LinkId > 7) || (NULL == pIgmpGroup))
        return (OamVarErrActBadParameters);

    size =
        sizeof(uint8) + (pIgmpGroup->numGroups * sizeof(OamIgmpGroupInfo));
    return (TkExtOamSet
            (pathId, LinkId, OamBranchAction, OamExtActAddIgmpGroup,
             (uint8 *) pIgmpGroup, size));

}


/*
 * send TK extension OAM message Set igmp VLAN to the ONU 
 */
uint8
TkExtOamSetIgmpVlan(uint8 pathId, uint8 LinkId, IgmpVlanRecord * pIgmpVlan)
{
    uint8           size;
    uint8           numOfGroup = 0;


    if ((LinkId > 7) || (NULL == pIgmpVlan))
        return (OamVarErrActBadParameters);

    for (numOfGroup = 0; numOfGroup < pIgmpVlan->numVlans; numOfGroup++) {
        pIgmpVlan->vlanCfg[numOfGroup].eponVid =
            soc_htons(pIgmpVlan->vlanCfg[numOfGroup].eponVid);
        pIgmpVlan->vlanCfg[numOfGroup].userVid =
            soc_htons(pIgmpVlan->vlanCfg[numOfGroup].userVid);
    }

    size = sizeof(uint16) + (pIgmpVlan->numVlans * sizeof(IgmpVlanCfg));
    return (TkExtOamSet
            (pathId, LinkId, OamBranchAttribute,
             OamExtAttrOnuIgmpVlan, (uint8 *) pIgmpVlan, size));

}


/*
 * send TK extension OAM message Get igmp VLAN from the ONU 
 */
int
TkExtOamGetIgmpVlan(uint8 pathId, uint8 LinkId, IgmpVlanRecord * pIgmpVlan)
{
    uint32          DataLen;
    uint8           numOfGroup = 0;

    if ((LinkId > 7) || (NULL == pIgmpVlan)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (OK ==
        TkExtOamGet(pathId, LinkId,
                    OamBranchAttribute,
                    OamExtAttrOnuIgmpVlan, (uint8 *) pIgmpVlan,
                    &DataLen)) {
        for (numOfGroup = 0; numOfGroup < pIgmpVlan->numVlans;
             numOfGroup++) {
            pIgmpVlan->vlanCfg[numOfGroup].eponVid =
                soc_ntohs(pIgmpVlan->vlanCfg[numOfGroup].eponVid);
            pIgmpVlan->vlanCfg[numOfGroup].userVid =
                soc_ntohs(pIgmpVlan->vlanCfg[numOfGroup].userVid);
        }
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}
