
/* $Id: stat.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        stat.h
 * Purpose:     stat internal definitions specific to FE2000 BCM library
 */

#ifndef _BCM_INT_SBX_FE2000_POLICER_H_
#define _BCM_INT_SBX_FE2000_POLICER_H_

#ifdef BCM_FE2000_P3_SUPPORT
extern int _bcm_fe2000_g2p3_stat_block_init(int unit, soc_sbx_g2p3_counter_id_t type,
                                            bcm_stat_sbx_info_t *pCtlBlock);
#endif /* BCM_FE2000_P3_SUPPORT */

#endif /* _BCM_INT_SBX_FE2000_POLICER_H_ */

