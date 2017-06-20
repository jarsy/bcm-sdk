#ifndef _SB_PAYL_MGR_H_
#define _SB_PAYL_MGR_H_
/* --------------------------------------------------------------------------
**
** $Id: sbPaylMgr.h,v 1.6 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$
**
** sbPaylMgr.h: payload memory manager public API
**
** --------------------------------------------------------------------------*/

#include <soc/sbx/fe2k_common/sbFeISupport.h>

typedef void *sbPHandle_t;
typedef void *sbPaylMgrCtxt_t;

typedef struct sbPayMgrInit_s {
  uint32 sramStart;
  uint32 sramSize;
  uint32 nbank;
  uint32 payloadMax;
  uint32 aligned;
  uint32 table1bits;
  sbMalloc_f_t sbmalloc;
  sbFree_f_t sbfree;
  void *clientData;
} sbPayMgrInit_t;

sbStatus_t 
sbPayloadMgrInit(sbPaylMgrCtxt_t *pMgrHdl, sbPayMgrInit_t *pInitParams);

sbStatus_t 
sbPayloadMgrUninit(sbPaylMgrCtxt_t *pMgrHdl);

uint32 
sbPayloadGetAddr(sbPaylMgrCtxt_t pM, sbPHandle_t);

uint32 
sbPayloadAligned (sbPaylMgrCtxt_t pM);

#endif
