/*
 * $Id: wlan.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * WLAN module
 */
#ifdef INCLUDE_L3

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/l2u.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/rate.h>
#include <bcm/ipmc.h>
#include <bcm/wlan.h>
#include <bcm/stack.h>
#include <bcm/topo.h>

/* Initialize the WLAN module. */
int 
bcm_robo_wlan_init(int unit)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* Detach the WLAN module. */
int 
bcm_robo_wlan_detach(int unit)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* Add a WLAN client to the database. */
int 
bcm_robo_wlan_client_add(int unit, bcm_wlan_client_t *info)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* Delete a WLAN client from the database. */
int 
bcm_robo_wlan_client_delete(int unit, bcm_mac_t mac)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}


/* Delete all WLAN clients. */
int 
bcm_robo_wlan_client_delete_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* Get a WLAN client by MAC. */
int 
bcm_robo_wlan_client_get(int unit, bcm_mac_t mac, bcm_wlan_client_t *info)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* bcm_wlan_client_traverse */
int 
bcm_robo_wlan_client_traverse(int unit, bcm_wlan_client_traverse_cb cb, 
                             void *user_data)
{
    return BCM_E_UNAVAIL;
}

/* bcm_wlan_port_add */
int 
bcm_robo_wlan_port_add(int unit, bcm_wlan_port_t *info)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* bcm_wlan_port_delete */
int 
bcm_robo_wlan_port_delete(int unit, bcm_gport_t wlan_port_id)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* bcm_wlan_port_delete_all */
int 
bcm_robo_wlan_port_delete_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* bcm_wlan_port_get */
int 
bcm_robo_wlan_port_get(int unit, bcm_gport_t wlan_port_id, bcm_wlan_port_t *info)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/* bcm_wlan_port_traverse */
int 
bcm_robo_wlan_port_traverse(int unit, bcm_wlan_port_traverse_cb cb, 
                           void *user_data)
{
    return BCM_E_UNAVAIL;
}

/* WLAN tunnel initiator create */
int
bcm_robo_wlan_tunnel_initiator_create(int unit, bcm_tunnel_initiator_t *info)
{
    return BCM_E_UNAVAIL;
}

/* WLAN tunnel initiator destroy */
int
bcm_robo_wlan_tunnel_initiator_destroy(int unit, bcm_gport_t wlan_tunnel_id)
{
    return BCM_E_UNAVAIL;
}

/* WLAN tunnel initiator get */
int
bcm_robo_wlan_tunnel_initiator_get(int unit, bcm_tunnel_initiator_t *info)
{
    return BCM_E_UNAVAIL;
}

#endif
