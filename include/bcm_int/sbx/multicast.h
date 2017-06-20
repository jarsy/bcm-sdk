/* $Id: multicast.h,v 1.5 Broadcom SDK $
 * $Id:
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: multicast header file
 */
#ifndef _BCM_INT_SBX_MULTICAST_H_
#define _BCM_INT_SBX_MULTICAST_H_

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_sbx_multicast_get_state(int unit, char *pbuf);
#endif /* EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* EASY_RELOAD_SUPPORT */

#if defined(BCM_WARM_BOOT_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT_SW_DUMP)
extern int
bcm_sbx_wb_multicast_state_sync(int unit, int sync);
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_multicast_sw_dump(int unit);
#endif
#endif /* ndef _BCM_INT_SBX_MULTICAST_H_ */
