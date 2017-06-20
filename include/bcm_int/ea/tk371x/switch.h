/*
 * $Id: switch.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains switch module definitions internal to the BCM library.
 */
#ifndef _BCM_INT_EA_SWITCH_H
#define _BCM_INT_EA_SWITCH_H

#include <bcm/types.h>
#include <bcm/switch.h>

typedef struct bcm_tk371x_glb_alm_map_s {
    uint16 ctrl_id;
    uint16 alm_id;
}bcm_tk371x_glb_alm_map_t;

#endif /* _BCM_INT_EA_SWITCH_H */
