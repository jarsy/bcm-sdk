/*
 * $Id: CtcMcastApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcMcastApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_CtcMcastApi_H
#define _SOC_EA_CtcMcastApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/CtcOam.h>

#define CtcMcastApiDbgEnable    0 

typedef struct {
    uint8       operation;
    uint8       type;
    uint8       cntOfMcast;
    McastEntry  McastEntryArry[0];
} PACK CtcOamMcastVlan;

int     CtcExtOamAddMulticastVlan (uint8 pathId, uint8 LinkId, uint32 port, 
                uint32 cntOfMulticastVlan, uint16 * multicastVlan);

int     CtcExtOamDelMulticastVlan (uint8 pathId, uint8 LinkId, uint32 port, 
                uint32 cntOfMulticastVlan, uint16 * multicastVlan);

int     CtcExtOamClearMulticastVlan (uint8 pathId, uint8 LinkId, uint32 port);

int     CtcExtOamListMulticastVlan (uint8 pathId, uint8 LinkId, uint32 port, 
                uint32 * cntOfMulticastVlan, uint16 * multicastVlan);

int     CtcExtOamSetMulticastTagstripe (uint8 pathId, uint8 LinkId, 
                uint32 port, Bool stripeOp);

int     CtcExtOamGetMulticastTagstripe (uint8 pathId, uint8 LinkId, 
                uint32 port, Bool * stripeOp);

int     CtcExtOamSetMulticastSwitch (uint8 pathId, uint8 LinkId, 
                OamCtcMcastMode multicastMode);

int     CtcExtOamGetMulticastSwitch (uint8 pathId, uint8 LinkId, 
                OamCtcMcastMode * multicastMode);

int     CtcExtOamSetMulticastControl (uint8 pathId, uint8 LinkId, 
                OamCtcMcastGroupOp McastCtrlOp, McastCtrl McastCtrlType,
                uint8 cntOfMcastEntry, McastEntry * pMcastEntry);

int     CtcExtOamListMulticastControl (uint8 pathId, uint8 LinkId,
                McastCtrl * McastCtrlType,
                uint8 * cntOfMcastEntry, McastEntry * pMcastEntry);

int     CtcExtOamListPortMulticastControl (uint8 pathId, uint8 LinkId,
                uint32 port, McastCtrl * McastCtrlType,
                uint8 * cntOfMcastEntry, McastEntry * pMcastEntry);

int     CtcExtOamSetMulticastMaxGroupNum (uint8 pathId, uint8 LinkId, 
                uint32 port, uint8 maxGroupNum);

int     CtcExtOamGetMulticastMaxGroupNum (uint8 pathId, uint8 LinkId, 
                uint32 port, uint8 * maxGroupNum);

int     CtcExtOamGetaFastLeaveAblity (uint8 pathId, uint8 LinkId, 
                uint32 * cntOfEnum, uint32 * pEnumValue);

int     CtcExtOamaFastLeaveAdminState (uint8 pathId, uint8 LinkId, 
                uint32 * adminState);

int     CtcExtOamacFastLeaveAdminControl (uint8 pathId, uint8 LinkId, 
                uint32 adminState);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_CtcMcastApi_H */
