/*
 * $Id: vxbde.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __VXBDE_H__
#define __VXBDE_H__

#include <ibde.h>

typedef struct vxbde_bus_s {
    uint32 base_addr_start;
    int int_line;
    int be_pio;
    int be_packet;
    int be_other;
} vxbde_bus_t;

extern int vxbde_create(vxbde_bus_t *bus, 
			ibde_t **bde);

extern int vxbde_eb_create(vxbde_bus_t *bus, 
			ibde_t **bde, sal_vaddr_t base_address);

#endif /* __VXBDE_H__ */
