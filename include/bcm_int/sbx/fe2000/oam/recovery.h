/*
 * $Id: recovery.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * OAM Warm boot module
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_WB_H_
#define _BCM_INT_SBX_FE2000_OAM_WB_H_

#ifdef BCM_WARM_BOOT_SUPPORT

#include <bcm/oam.h>

extern int _oam_wb_maidmep_scan(int unit);
extern int _oam_wb_recover_ep(int unit, int group, int lsmi, int ep_idx);
extern int _oam_wb_recover(int unit, fe2k_oam_wb_mem_layout_t *layout);
extern int _oam_wb_post_init_recover(int unit);
extern int oam_ep_store(int unit, bcm_oam_endpoint_info_t *endpoint_info);

#endif /* BCM_WARM_BOOT_SUPPORT */
#endif  /* _BCM_INT_SBX_FE2000_OAM_WB_H_  */
