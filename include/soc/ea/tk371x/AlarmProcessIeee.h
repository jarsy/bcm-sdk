/*
 * $Id: AlarmProcessIeee.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     AlarmProcessIeee.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_AlarmProcessIeee_H
#define _SOC_EA_AlarmProcessIeee_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/AlarmProcess.h>    /* OamEventVector */

Bool    
IeeeEventIsTkAlarm (uint8 pathId, uint8 linkId, OamEventTlv * pOamEventTlv);

Bool    
IeeeEventIsCtcAlarm (uint8 pathId, uint8 linkId,
                OamEventTlv * pOamEventTlv);

int32   
IeeeEventErrSymPeriod (uint8 pathId, uint8 linkId,
    OamEventTlv * pOamEventTlv);

int32   
IeeeEventErrFrame (uint8 pathId, uint8 linkId, 
    OamEventTlv * pOamEventTlv);

int32   
IeeeEventErrFrPeriod (uint8 pathId, uint8 linkId, OamEventTlv * pOamEventTlv);

int32   
IeeeEventErrFrSecondsSum (uint8 pathId, uint8 linkId,
    OamEventTlv * pOamEventTlv);

int32   
IeeeEventVendorHandler (uint8 pathId, uint8 linkId,
    OamEventTlv * pOamEventTlv);

int32   
IeeeEventHandler (uint8 pathId, uint8 linkId,OamEventTlv * pOamEventTlv);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_AlarmProcessIeee_H */
