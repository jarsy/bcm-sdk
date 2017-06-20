/*
 * $Id: g3p1.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _SOC_SBX_G3P1_G3P1_H_
#define _SOC_SBX_G3P1_G3P1_H_
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT)

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/caladan3/ucodemgr.h>

extern int
g3p1_app_init(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg, uint32 *contexts, uint32 *epoch, int reload);
extern int g3p1_fte_partition(int unit);
#endif

#if defined(BCM_CALADAN3_G3P1_SUPPORT)
extern int soc_sbx_g3p1_mplstp_ena(int unit);
#endif

#endif /*  _SOC_SBX_G3P1_G3P1_H_ */
