/* 
 * $Id: trunk.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        trunk.h
 * Purpose:     Trunk internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_CALADAN3_TRUNK_H_
#define _BCM_INT_SBX_CALADAN3_TRUNK_H_

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

#define _BCM_TRUNK_PSC_UNDEFINED  -1

#define _BCM_TRUNK_PSC_EXTENDED_FLAGS      \
     (BCM_TRUNK_PSC_MACDA       |       \
      BCM_TRUNK_PSC_MACSA       |       \
      BCM_TRUNK_PSC_IPSA        |       \
      BCM_TRUNK_PSC_IPDA        |       \
      BCM_TRUNK_PSC_L4SS        |       \
      BCM_TRUNK_PSC_L4DS        |       \
      BCM_TRUNK_PSC_VID)

typedef struct trunk_private_s {
    bcm_trunk_t     trunk_id;       /* trunk group ID */
    int             in_use;         /* whether particular trunk is in use */
    int             num_ports;        /* Number of ports in the trunk group */
    uint32          member_flags[BCM_TRUNK_MAX_PORTCNT];
    bcm_port_t      tp[BCM_TRUNK_MAX_PORTCNT];    /* Ports in trunk */
    bcm_module_t    tm[BCM_TRUNK_MAX_PORTCNT];    /* Modules per port */
} trunk_private_t;

typedef enum trunk_init_state_e {
    ts_none,
    ts_recovering, /* phase 1 recovered, second phase recovery required to
                    * complete init sequence */
    ts_init        /* completely initialized */
} trunk_init_state_t;

typedef struct trunk_cntl_s {
    trunk_init_state_t  init_state; /* Initialized, recovering, or none */
    sal_mutex_t         lock;       /* Database lock */
    int                 ngroups;    /* max number of trunk groups */
    int                 nports;     /* max port count per trunk group */
    int                 psc;        /* port spec criterion (a.k.a. hash algorithm) */
    int                 invalid_oi;
    bcm_trunk_notify_cb callback_func[_BCM_TRUNK_MAX_CALLBACK]; /* callback function pointers. */
    void               *callback_user_data[_BCM_TRUNK_MAX_CALLBACK];  /* user_data pointers. */
    trunk_private_t    *t_info;
} trunk_cntl_t;


extern int 
bcm_caladan3_trunk_change_register(int unit,
                                 bcm_trunk_notify_cb callback,
                                 void *user_data);

extern int
bcm_caladan3_trunk_change_unregister(int unit, 
                                   bcm_trunk_notify_cb callback);

/* Returns the set difference of info_a and info_b.  
 * Specifically, <mod,port> pairs found in info_a not present info_b 
 */
extern void
bcm_caladan3_trunk_add_info_cmp(bcm_trunk_add_info_t *info_a,
                            bcm_trunk_add_info_t *info_b,
                            int                  *num_ports,
                            bcm_module_t          mods[BCM_TRUNK_MAX_PORTCNT],
                            bcm_port_t            ports[BCM_TRUNK_MAX_PORTCNT]);

extern int
bcm_caladan3_trunk_get_old(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data);

#endif /* _BCM_INT_SBX_CALADAN3_TRUNK_H_ */
