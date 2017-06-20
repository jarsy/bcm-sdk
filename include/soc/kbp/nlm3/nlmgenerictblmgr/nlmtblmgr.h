/*
 * $Id: nlmtblmgr.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 /*
Summary:
This file contains function declaration for creating and initializing Table Manager
specifically for serial devices only.
*/
#ifndef INCLUDED_NLMTBLMGR_H
#define INCLUDED_NLMTBLMGR_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include "nlmcmallocator.h"
#include "nlmcmdbllinklist.h"
#include "nlmerrorcodes.h"
#include "nlmcmexterncstart.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmcmdbllinklist.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

/*
The function creats and initializes Table Manager for devices.
The function is called when GTM API kbp_gtm_init is called to create
the GTM module. This function creates the second level layer Table Manager
in the GTM module.
*/
NlmErrNum_t NlmTblMgr__Init(
                              NlmGenericTblMgr* genericTableMgr_p,
                              NlmCmAllocator   *alloc_p,
                              void*           devMgr_p,
                              nlm_u8          numOfDevices,
                              NlmGenericTblMgrBlksRange *gtmBlksRange,
                              NlmGenericTblMgrSBRange    *udaSbRange,
                              NlmIndexChangedAppCb indexChangedAppCb,
                              void             *client_p,               
                              NlmReasonCode    *o_reason
                              );


NlmErrNum_t NlmTblMgr__pvt__checkSplTables(
                            NlmGenericTblMgr  *genericTblMgr_p,
                            NlmReasonCode     *o_reason
                            );
#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif
#endif /* INCLUDED_NLMTBLMGR_H */
