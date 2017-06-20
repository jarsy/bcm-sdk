/*
 * $Id: TkIgmpApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkIgmpApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkIgmpApi_H
#define _SOC_EA_TkIgmpApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>


/* send TK extension OAM message Set igmp configuration to the ONU */
uint8   TkExtOamSetIgmpConfig (uint8 pathId, uint8 LinkId,
                OamIgmpConfig * pIgmpConfig);

/* send TK extension OAM message Get igmp configuration from the ONU */
int     TkExtOamGetIgmpConfig (uint8 pathId, uint8 LinkId,
                OamIgmpConfig * pIgmpConfig);

/* send TK extension OAM message Get igmp group info from the ONU */
int     TkExtOamGetIgmpGroupInfo (uint8 pathId, uint8 LinkId,
                OamIgmpGroupConfig * pIgmpGroupInfo);

/* send TK extension OAM message Set delete igmp group to the ONU */
uint8   TkExtOamSetDelIgmpGroup (uint8 pathId, uint8 LinkId,
                OamIgmpGroupConfig * pIgmpGroup);

/* send TK extension OAM message Set add igmp group to the ONU */
uint8   TkExtOamSetAddIgmpGroup (uint8 pathId, uint8 LinkId,
                OamIgmpGroupConfig * pIgmpGroup);

/* send TK extension OAM message Set igmp VLAN to the ONU */
uint8   TkExtOamSetIgmpVlan (uint8 pathId, uint8 LinkId,
                IgmpVlanRecord * pIgmpVlan);

/* send TK extension OAM message Get igmp VLAN from the ONU */
int     TkExtOamGetIgmpVlan (uint8 pathId, uint8 LinkId,
                IgmpVlanRecord * pIgmpVlan);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkIgmpApi_H */
