/*
 * $Id: TkTmApi.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkTmApi.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_TkTmApi_H
#define _SOC_EA_TkTmApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>


/* send TK extension OAM message Get queue configuration from the ONU */
int     TkExtOamGetQueueCfg (uint8 pathId, uint8 LinkId, uint8 * pQueueCfg);

/* send TK extension OAM message Set enable user traffic to the ONU */
int     TkExtOamSetEnaUserTraffic (uint8 pathId, uint8 LinkId);

/* send TK extension OAM message Set disable user traffic to the ONU */
int     TkExtOamSetDisUserTraffic (uint8 pathId, uint8 LinkId);

int     TkExtOamGetBcastRateLimit(uint8 pathId, uint8 LinkId, 
                uint8 port, uint32 *pkts_cnt);
/*
 * send TK extension OAM message Set BcastRateLimit ON/OFF
 */
int     TkExtOamSetBcastRateLimit(uint8 pathId, uint8 LinkId, uint8 port, 
                uint32 pkts_cnt);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkTmApi_H */
