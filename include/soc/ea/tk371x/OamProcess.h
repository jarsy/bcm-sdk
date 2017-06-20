/*
 * $Id: OamProcess.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        name.h
 * Purpose:     Purpose of the file
 */

#ifndef _SOC_EA_OamProcess_H_
#define _SOC_EA_OamProcess_H_

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkMsg.h>

Bool    TkDataProcessHandle (uint8 pathId, TkMsgBuff * pMsg, uint16 len);

void    TkAlmHandlerRegister (uint8, uint8 (*funp) (uint8 , uint8 *, short));

void    TkAlarmRxThread (void *cookie);

#endif	/* !_SOC_EA_OamProcess_H_ */
