/*
 * $Id: vswitch.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        vswitch.h
 * Purpose:     Vswitch internal definitions
 *              specific to FE2000 BCM API implementation
 */

#ifndef _BCM_INT_SBX_FE2000_VSWITCH_H_
#define _BCM_INT_SBX_FE2000_VSWITCH_H_

#include <bcm/port.h>
#include <bcm/vlan.h>

int _bcm_fe2000_vswitch_port_info_get(int unit, bcm_port_t port, 
                                      bcm_vlan_t vsi, int *keepUntagged);

int _bcm_fe2000_vswitch_port_gport_get(int unit, bcm_port_t port,
                                       bcm_vlan_t vsi, bcm_gport_t *gport);
#endif
