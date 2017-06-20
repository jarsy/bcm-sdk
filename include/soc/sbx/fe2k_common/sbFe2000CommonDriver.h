/* -*- Mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
 * $Id: sbFe2000CommonDriver.h,v 1.8 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sbFe2000CommonDriver.h : FE2000 Common defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE_2000_COMMON_DRIVER_H_
#define _SB_FE_2000_COMMON_DRIVER_H_

#include <soc/sbx/sbTypes.h>

#include "sbZfFe2000PmGroupConfig.hx"
uint32 
sbFe2000PmGroupConfigSet(sbhandle userDeviceHandle,
                         uint32 uGroupId,
                         sbZfFe2000PmGroupConfig_t *pConfig);

uint32 
sbFe2000PmGroupConfigGet(sbhandle userDeviceHandle,
                         uint32 uGroupId, 
                         sbZfFe2000PmGroupConfig_t *pConfig);

uint32 
sbFe2000PmProfileMemoryRead(sbhandle userDeviceHandle, uint32 uProfileId, uint32 *puData);

uint32 
sbFe2000BatchGroupRecordSizeWrite(sbhandle userDeviceHandle, uint32 uBatchGroup, uint32 uRecordSize);

uint32 
sbFe2000ByteHashConfigWrite(sbhandle userDeviceHandle, uint32 uTemplateId, uint32 *puData);

uint32 
sbFe2000ByteHashConfigRead(sbhandle userDeviceHandle, uint32 uTemplateId, uint32 *puData);

uint32 
sbFe2000BitHashConfigWrite(sbhandle userDeviceHandle, uint32 uTemplateId, uint32 *puData);

uint32 
sbFe2000BitHashConfigRead(sbhandle userDeviceHandle, uint32 uTemplateId, uint32 *puData);

#endif
