/*
 * $Id: stack.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_SBX_STACK_H_
#define _BCM_INT_SBX_STACK_H_

#include <bcm/stack.h>

#define _BCM_SBX_STACK_PORTMAP_CHUCK_SIZE (100)

typedef struct bcm_sbx_stack_portmap_block_s {
    struct bcm_sbx_stack_portmap_block_s *next;
    uint32                                count;
    bcm_gport_t                           portmap[2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE];
} bcm_sbx_stack_portmap_block_t;

/* NOTE: Currently "BCM_STK_MAX_MODULES" is not defined correctly to work with polaris device */
typedef struct bcm_sbx_stack_state_s {
    bcm_module_protocol_t          protocol[BCM_STK_MAX_MODULES];
    int                            is_module_enabled[BCM_STK_MAX_MODULES];
    bcm_sbx_stack_portmap_block_t  *gport_map;
} bcm_sbx_stack_state_t;

int bcm_sbx_stk_get_modules_enabled(int unit, int *nbr_modules_enabled);
int bcm_sbx_stk_fabric_map_get_switch_port(int unit, bcm_gport_t fabric_port, bcm_gport_t *switch_port);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int bcm_sbx_wb_stack_state_init(int unit);
extern int bcm_sbx_wb_stack_state_sync(int unit, int sync);
#endif
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void bcm_sbx_wb_stack_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
#endif  /* _BCM_INT_SBX_STACK_H_ */
