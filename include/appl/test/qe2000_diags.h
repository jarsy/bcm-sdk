/*
 * $Id: qe2000_diags.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * == qe2000_diags.h - QE2000 Diagnostics      ==
 */

#ifndef _QE2000_DIAGS_H_
#define _QE2000_DIAGS_H_

#include "sbx_diags.h"
#include <soc/types.h>
#include <soc/sbx/sbWrappers.h>
#include "sbx_diags.h"

/* foward declarations for the qe2k traffic test*/
int sbQe2000DiagsTrafficTest(sbxQe2000DiagsInfo_t *pDiagsInfo);

/* foward declarations for the qe2k prbx test*/
int sbQe2000DiagsPrbsTest(sbxQe2000PrbsInfo_t *pPrbsInfo);

#endif /* _QE2000_DIAGS_H_ */

