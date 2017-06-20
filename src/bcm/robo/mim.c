/*
 * $Id: mim.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * Mac-in-Mac initializers
 */

#if defined(INCLUDE_L3)

#include <bcm/error.h>
#include <bcm/mim.h>

/* Create a VPN instance. */
int 
bcm_robo_mim_vpn_create(int unit, bcm_mim_vpn_config_t *info)
{
    return BCM_E_UNAVAIL;
}

/* Delete a VPN instance. */
int 
bcm_robo_mim_vpn_destroy(int unit, bcm_mim_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}


/* Delete all VPN instances. */
int 
bcm_robo_mim_vpn_destroy_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/* Get a VPN instance by ID. */
int 
bcm_robo_mim_vpn_get(int unit, bcm_mim_vpn_t vpn, 
                    bcm_mim_vpn_config_t *info)
{
    return BCM_E_UNAVAIL;
}

/* bcm_mim_vpn_traverse */
int 
bcm_robo_mim_vpn_traverse(int unit, bcm_mim_vpn_traverse_cb cb, 
                         void *user_data)
{
    return BCM_E_UNAVAIL;
}

/* bcm_mim_port_add */
int 
bcm_robo_mim_port_add(int unit, bcm_mim_vpn_t vpn, 
                     bcm_mim_port_t *mim_port)
{
    return BCM_E_UNAVAIL;
}

/* bcm_mim_port_delete */
int 
bcm_robo_mim_port_delete(int unit, bcm_mim_vpn_t vpn, 
                    bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL;
}

/* bcm_mim_port_delete_all */
int 
bcm_robo_mim_port_delete_all(int unit, bcm_mim_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

/* bcm_mim_port_get */
int 
bcm_robo_mim_port_get(int unit, bcm_mim_vpn_t vpn, 
                     bcm_mim_port_t *mim_port)
{
    return BCM_E_UNAVAIL;
}

/* bcm_mim_port_get_all */
int 
bcm_robo_mim_port_get_all(int unit, bcm_mim_vpn_t vpn, int port_max, 
                     bcm_mim_port_t *port_array, int *port_count)
{
    return BCM_E_UNAVAIL;
}
#endif
