/*
 * $Id: l3.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE2000 Internal header
 */

#ifndef _BCM_INT_SBX_L3_H_
#define _BCM_INT_SBX_L3_H_
#ifdef BCM_FE2000_SUPPORT

extern int
_bcm_fe2000_l3_modid_set(int unit, int modid);

int _bcm_fe2000_l3_flush_cache(int unit, int flag);

#endif /* BCM_FE2000_SUPPORT */
#endif /* _BCM_INT_SBX_L3_H_ */

