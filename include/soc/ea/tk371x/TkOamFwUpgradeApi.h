/*
 * $Id: TkOamFwUpgradeApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOamFwUpgradeApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkOamFwUpgradeApi_H
#define _SOC_EA_TkOamFwUpgradeApi_H

#ifdef __cplusplus
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>

/* ONU firmware upgrade from the host to the ONU */
int    TkExtFirmwareUpgrade (uint8 pathId, uint8 loadType, uint32 Len,
                uint8 * pLoadBuf);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkOamFwUpgradeApi_H */
