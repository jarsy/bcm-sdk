/*
 * $Id: t3p1.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _SOC_SBX_T3P1_EMPTY_H_
#define _SOC_SBX_T3P1_EMPTY_H_

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/caladan3/ucodemgr.h>

extern int
t3p1_app_init(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg, uint32 *contexts, uint32 *epoch, int reload);

#endif
