/*
 * $Id: phymod_tsc_iblk.h,v 1.1.2.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __PHYMOD_TSC_IBLK_H__
#define __PHYMOD_TSC_IBLK_H__

#include <phymod/phymod_acc.h>
#include <phymod/phymod_reg.h>

/* Special lane values for broadcast and dual-lane multicast */
#define PHYMOD_TSC_IBLK_MCAST01    4
#define PHYMOD_TSC_IBLK_MCAST23    5
#define PHYMOD_TSC_IBLK_BCAST      6

extern int
phymod_tsc_iblk_read(const phymod_access_t *pa, uint32_t addr, uint32_t *data);

extern int
phymod_tsc_iblk_write(const phymod_access_t *pa, uint32_t addr, uint32_t data);

#endif /* __PHYMOD_TSC_IBLK_H__ */
