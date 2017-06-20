/*
 * $Id: AlarmProcess.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     AlarmProcess.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_AlarmProcess_H
#define _SOC_EA_AlarmProcess_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>

typedef struct AlarmToBcmEventTbl_s {
    uint32 alarm_id;
    uint32 bcm_event;
}AlarmToBcmEventTbl_t;

typedef int (*OamEventHandler)(uint8, uint8, void *);

typedef struct {
    uint32  eventId;
    int (*handler) (uint8, uint8, void *);
    char  * Description;
    uint32  Stats;
} OamEventVector;

uint8 AlarmProcessHandler (uint8 pathId, uint8 * pBuff, short len);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_AlarmProcess_H */
