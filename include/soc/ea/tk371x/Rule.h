/*
 * $Id: Rule.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     Rule.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_Rule_H
#define _SOC_EA_Rule_H

#include <soc/ea/tk371x/TkTypes.h>


typedef struct {
    uint8   port_link;
    uint8   queue;
} SetPath;

typedef enum {
    SelDesMacAddr   = 0,
    SelSrcMacAddr,
    SelLinkIndex,
    SelEthType,
    SelVlanID,
    SelUser0,
    SelIpv4Protocol,
    SelUser1,
    SelUser2,
    SelUser3,
    SelUser4,
    SelL3Protocol,
    SelIpDesAddrLow,
    SelIpv6DesAddrHi,
    SelIpSrcAddrLow,
    SelIpv6SrcAddrHi
} SelectorType;

typedef enum {
    OpNeverMatch,
    OpEqual,
    OpNotEqual,
    OpLessThan,
    OpGreaterThan,
    OpExist,
    OpNotExist,
    OpAlwaysMatch
} OperatorType;

typedef enum {
    ActNoOp                     = 0,
    ActReserved0,
    ActSetPath,
    ActSetAddVlanTag,
    ActSetDelVlanTag,
    ActSetVidAndAddVlanTag,
    ActSetCos,
    ActReplaceVlanTag,
    ActReplaceVlanTagAndSetVid,
    ActClearAddVlanTag,
    ActClearDelVlanTag          = 10,
    ActClearDelVlanTagAndSetAddVlanTag,
    ActCopyFieldToCos,
    ActCopyFieldToVid,
    ActDiscard,
    ActReserved1,
    ActForward,
    ActReserved2,
    ActSetPathAndForward,
    ActSetAddVlanTagAndForward,
    ActSetDelVlanTagAndForward  = 20,
    ActSetVidAndSetAddVlanTagAndForward,
    ActSetCosAndForward,
    ActReplaceTagAndForward,
    ActReplaceTagAndSetVidAndForward,
    ActClearAddVlanTagAndForward,
    ActClearDelVlanTagAndForward,
    ActClearDelVlanTagAndSetAddVlanTagAndForward,
    ActCopyFieldToCosAndForward,
    ActCopyFieldToVidAndForward,
    ActReserved3                = 30,
    Actreserved4
} ActionType;

typedef enum {
    ParamVid                    = 128,
    ParamCos,
    ParamPort,
    ParamLinkId,
    ParamQueue
} ParamType;

#endif /* _SOC_EA_Rule_H */
