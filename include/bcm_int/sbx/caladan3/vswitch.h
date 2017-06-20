/*
 * $Id: vswitch.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        vswitch.h
 * Purpose:     Vswitch internal definitions
 *              specific to FE2000 BCM API implementation
 */

#ifndef _BCM_INT_SBX_CALADAN3_VSWITCH_H_
#define _BCM_INT_SBX_CALADAN3_VSWITCH_H_

#include <bcm/port.h>
#include <bcm/vlan.h>

/*
 *  This structure describes a single gport that is a member of a VSI.
 */
typedef struct _bcm_caladan3_vswitch_port_s {
    bcm_gport_t gport;                       /* GPort ID */
    struct _bcm_caladan3_vswitch_port_s *next; /* next entry this list */
    struct _bcm_caladan3_vswitch_port_s *prev; /* prev entry this list */
} _bcm_caladan3_vswitch_port_t;

/*
 *  This structure contains information about a given unit's vswitch state.
 *  Since we must have a list of ports that are included in a VSI (due to the
 *  requirement for delete_all), this also includes pointers to such lists.
 *  We also need to include lists for ALL VSIs since we can add GPORTs to
 *  various other special cases, such as traditional bridging.
 */
typedef struct _bcm_caladan3_vswitch_state_s {
    sal_mutex_t lock;
    _bcm_caladan3_vswitch_port_t **portList;
} _bcm_caladan3_vswitch_state_t;



int _bcm_caladan3_vswitch_port_info_get(int unit, bcm_port_t port, 
                                      bcm_vlan_t vsi, int *keepUntagged);

int _bcm_caladan3_vswitch_port_gport_get(int unit, bcm_port_t port,
                                       bcm_vlan_t vsi, bcm_gport_t *gport);
#endif
