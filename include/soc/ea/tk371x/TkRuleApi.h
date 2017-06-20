/*
 * $Id: TkRuleApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkRuleApi.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_TkRuleApi_H
#define _SOC_EA_TkRuleApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/Oam.h>


typedef struct {
    uint8 cntOfUpQ;
    uint8 sizeOfUpQ [MAX_CNT_OF_UP_QUEUE];
} TkLinkConfigInfo;

typedef struct {
    uint8 cntOfDnQ;
    uint8 sizeOfDnQ [MAX_CNT_OF_UP_QUEUE];
} TkPortConfigInfo;

typedef struct {
    uint8 cntOfDnQ;
    uint8 sizeOfDnQ [MAX_CNT_OF_UP_QUEUE];
} TkMcastConfigInfo;

typedef enum{
    TkObjTypeLink = 1,
    TkObjTypePort = 3, 
} TkOamObjectType;

typedef struct {
	TkOamObjectType objType;
	uint8           index;
}LogicalPortIndex;

typedef struct {
    uint8               cntOfLink;
    TkLinkConfigInfo    linkInfo[MAX_CNT_OF_LINK];
    uint8               cntOfPort;
    TkPortConfigInfo    portInfo[MAX_CNT_OF_PORT];
    TkMcastConfigInfo   mcastInfo;
} TkQueueConfigInfo;


typedef struct {
    uint8   field;
    union {
        uint8   da[6];
        uint8   sa[6];
        uint16  etherType;
        uint16  vlan;
        uint8   priority;
        uint8   ipProtocol;
        uint8   value[8];
    } common;
    uint8   operator;
} TkRuleCondition;

typedef struct {
    uint32          conditionCount;
    TkRuleCondition conditionList[8];
} TkRuleConditionList;

typedef struct {
    uint8       port_link;
    uint8       queue;
} TkRuleNameQueue;

typedef union {
    TkRuleNameQueue ndest;
    uint16          vid_cos;
} TkRulePara;

typedef struct {
    uint8 volatiles;
    uint8 priority;
    TkRuleConditionList ruleCondition;
    uint8 action;
    TkRulePara param;
}TkPortRuleInfo;

int     TkExtOamGetFilterRulesByPort (uint8 pathId, uint8 linkId,
                OamRuleDirection direction, uint8 portIndex,
                uint8 * pRxBuf, uint32 * pRxLen);

int     TkExtOamClearAllFilterRulesByPort (uint8 pathId, uint8 linkId,
                OamRuleDirection direction,
                uint8 portIndex);

int     TkExtOamClearAllUserRulesByPort (uint8 pathId, uint8 linkId,
                OamRuleDirection direction, uint8 portIndex);

int     TkExtOamClearAllClassifyRulesByPort (uint8 pathId, uint8 linkId,
                OamRuleDirection direction,
                uint8 portIndex);

int32   TkRuleActDiscard (uint8 pathId, uint8 linkId, uint8 volatiles,
                uint8 portIndex, uint8 pri, uint8 numOfConditon,
                OamNewRuleCondition * pCondList, uint8 action);

int32   TkRuleDiscardDsArpWithSpecifiedSenderIpAddr (uint8 pathId,
                uint32 ipAddress, uint8 action);

int32   TkQueueGetConfiguration (uint8 pathId, 
                TkQueueConfigInfo * pQueueConfigInfo);

int32   TkQueueSetConfiguration (uint8 pathId, 
                TkQueueConfigInfo * pQueueConfigInfo);


int32   TkAddOneRuleByPort (uint8 pathId, uint8 linkId, uint8 volatiles,
                uint8 portIndex, uint8 pri,
                TkRuleConditionList * ruleCondition, uint8 action,
                TkRulePara * param);

int32   TkDelOneRuleByPort (uint8 pathId, uint8 linkId, uint8 volatiles,
                uint8 portIndex, uint8 pri,
                TkRuleConditionList * ruleCondition, uint8 action,
                TkRulePara * param);

int32   TkExtOamPortRuleDel(uint8 pathId,uint8 linkId, LogicalPortIndex port, 
                TkPortRuleInfo *ruleInfo);

int32   TkExtOamPortRuleAdd(uint8 pathId,uint8 linkId, LogicalPortIndex port, 
                TkPortRuleInfo *ruleInfo);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkRuleApi_H */
