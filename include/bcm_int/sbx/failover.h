/*
 * $Id: failover.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: failover common interface
 */

#ifndef _BCM_INT_SBX_FAILOVER_H_
#define _BCM_INT_SBX_FAILOVER_H_

#include <bcm/types.h>
#include <bcm/failover.h>
#include <shared/bitop.h>

/* reserved, the failover profile is allocated;
 * in use, the failover port mapping specified;
 * enable, the failover enable state;
 * updated, the failover profile just updated, so update even if enable flag 
 *          is matching the enable state, possibly use specified a different
 *          protect port
 */
#define BCM_SBX_FAILOVER_RESERVED  (1<<0)
#define BCM_SBX_FAILOVER_INUSE     (1<<1)
#define BCM_SBX_FAILOVER_ENABLE    (1<<2)
#define BCM_SBX_FAILOVER_UPDATED   (1<<3)
#define BCM_SBX_FAILOVER_APS       (1<<4)

typedef struct bcm_sbx_failover_object_s {
    int state;               /* failover object state */
    int sysport;             /* sysport */
    int active_gport;        /* active gport, stored for mapping purpose */
    int protect_gport;       /* protect gport */
    int active_nodeport;     /* active node/port */
    SHR_BITDCLNAME(subportmask, SB_FAB_DEVICE_MAX_FABRIC_PORTS);  /* subport mask */
} bcm_sbx_failover_object_t;

/* declare the function here before we are allowed to add
 * bcm_failover_init into BCM API
 */
extern int
bcm_sbx_failover_deinit(int unit);

#endif /* _BCM_INT_SBX_FAILOVER_H_ */
