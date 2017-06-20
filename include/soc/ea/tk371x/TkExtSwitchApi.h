/*
 * $Id: TkExtSwitchApi.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkExtSwitchApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkExtSwitchApi_H
#define _SOC_EA_TkExtSwitchApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>


/* Defines to control UNI ingress/egress VLAN policy*/
typedef OamUniVlanPolicy    TkOnuVlanUniPolicy;

/* Defines to control per VLAN ingress/egress tagging behavior*/
typedef OamVlanTagBehave    TkOnuVlanTagBehave;
typedef OamVlanPolicyCfg    TkOnuVlanPolicyCfg;
typedef OamVlanMemberCfg    TkOnuVlanMemberCfg;

typedef struct {
    uint16              numVlans;   /* Number of VLANs in this message */
    TkOnuVlanMemberCfg  member[1];  /* VLAN specific information */
} PACK OamVlanCfgTlv;

/* Switch port mirror */
typedef struct {
    uint8   ingPort;
    uint8   egPort;
    uint16  ingMask;
    uint16  egMask;
} TagPortMirrorCfg;

uint8   TkExtOamSetPortMirror (uint8 pathId, uint8 linkId, uint8 ingPort, 
                uint8 egPort, uint16 ingMask, uint16 egMask);


uint8   TkExtOamGetPortMirror (uint8 pathId, uint8 linkId, 
                TagPortMirrorCfg * cfg);

/* send TK extension OAM message Get Local Switching states */
int     TkExtOamGetLocalSwitch (uint8 pathId, uint8 LinkId, Bool * Status);

/* send TK extension OAM message Set Local Switching states */
uint8   TkExtOamSetLocalSwitch (uint8 pathId, uint8 LinkId, Bool Status);

int     TkOnuSetPortVlanPolicy (uint8 pathId, uint8 linkId, uint8 portId,
                TkOnuVlanPolicyCfg * portVlanPolicyCfg);

int     TkOnuGetPortVlanPolicy (uint8 pathId, uint8 linkId, uint8 portId,
                TkOnuVlanPolicyCfg * portVlanPolicyCfg);

int     TkOnuSetPortVlanMembership (uint8 pathId, uint8 linkId, uint8 portId,
                uint16 numOfEntry,
                TkOnuVlanMemberCfg * portVlanMemberCfg);

int     TkOnuGetPortVlanMembership (uint8 pathId, uint8 linkId, uint8 portId,
                uint16 * numOfEntry,
                TkOnuVlanMemberCfg * portVlanMemberCfg);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkExtSwitchApi_H */
