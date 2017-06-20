/* 
 * $Id: trunk.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        trunk.h
 * Purpose:     Trunk internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_TRUNK_H_
#define _BCM_INT_SBX_FE2000_TRUNL_H_

#include <bcm/trunk.h>
#include <bcm_int/common/trunk.h>

/* this is probably way too many, but reduce or increase as needed */
#define _BCM_TRUNK_MAX_CALLBACK  5

/* Callback function type for trunk change notifications. */
typedef int (*bcm_trunk_notify_cb)(
    int unit,
    bcm_trunk_t tid,
    bcm_trunk_add_info_t *add_info,
    void *user_data);

extern int 
bcm_fe2000_trunk_change_register(int unit,
                                 bcm_trunk_notify_cb callback,
                                 void *user_data);

extern int
bcm_fe2000_trunk_change_unregister(int unit, 
                                   bcm_trunk_notify_cb callback);

/* Returns the set difference of info_a and info_b.  
 * Specifically, <mod,port> pairs found in info_a not present info_b 
 */
extern void
bcm_fe2k_trunk_add_info_cmp(bcm_trunk_add_info_t *info_a,
                            bcm_trunk_add_info_t *info_b,
                            int                  *num_ports,
                            bcm_module_t          mods[BCM_TRUNK_MAX_PORTCNT],
                            bcm_port_t            ports[BCM_TRUNK_MAX_PORTCNT]);

extern int
bcm_fe2000_trunk_get_old(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data);

#endif /* _BCM_INT_SBX_FE2000_TRUNK_H_ */
