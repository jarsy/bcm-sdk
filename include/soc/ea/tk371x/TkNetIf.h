/*
 * $Id: TkNetIf.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkNetIf.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkNetIf_H
#define _SOC_EA_TkNetIf_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>

Bool    TkOsDataTx (uint8 pathId, uint8 * buf, uint16 len);
void    TkOsGetIfMac (uint8 * mac);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkNetIf_H */
