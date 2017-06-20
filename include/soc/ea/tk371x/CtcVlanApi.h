/*
 * $Id: CtcVlanApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcVlanApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_CtcVlanApi_H
#define _SOC_EA_CtcVlanApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Ethernet.h>
#include <soc/ea/tk371x/CtcOam.h>


typedef struct {
    EthernetVlanData    vlanTag;
} CtcVlanTagInfo;

typedef struct {
    uint32              numOfEntry;
    EthernetVlanData    defaultVlanInfo;
    CtcVlanTranslatate  vlanTranslateArry[0];
} CtcVlanTranslateInfo;

typedef struct {
    uint32              numOfEntry;
    EthernetVlanData    defaultVlanInfo;
    EthernetVlanData    vlanTrunkArry[0];
} CtcVlanTrunkInfo;

typedef struct {
    uint32                      mode;
    union {
        CtcVlanTagInfo          tagInfo;
        CtcVlanTranslateInfo    translateInfo;
        CtcVlanTrunkInfo        trunkInfo;
    } CtcVlanArry;
} CtcVlanEntryInfo;

int     CtcExtOamSetVlanTransparent (uint8 pathId, uint8 LinkId, uint32 port);

int     CtcExtOamSetVlanTag (uint8 pathId, uint8 LinkId, uint32 port, 
                CtcOamVlanTag * pCtcOamVlanTag);

int     CtcExtOamSetVlanTranslation (uint8 pathId, uint8 LinkId, uint32 port,
                EthernetVlanData defaultVlan, uint32 numOfTranslateEntry, 
                CtcVlanTranslatate * pCtcVlanTranslatate);

int     CtcExtOamSetVlanTrunk (uint8 pathId, uint8 LinkId, uint32 port,
                EthernetVlanData defaultVlan, uint32 numOfTrunkEntry, 
                EthernetVlanData * pCtcVlanTrunkEntry);

int     CtcExtOamGetVlan (uint8 pathId, uint8 LinkId, uint32 port, 
                CtcVlanEntryInfo * pVlanEntry);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_CtcVlanApi_H */
