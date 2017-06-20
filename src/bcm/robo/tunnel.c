/*
 * $Id: tunnel.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    l3.c
 * Purpose: Manages l3 interface table, forwarding table, routing table
 *
 * Note : Not for RoboSwitch currently.
 */

#ifdef INCLUDE_L3

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/l3.h>
#include <bcm/tunnel.h>

int
bcm_robo_tunnel_config_set(int unit, bcm_tunnel_config_t tconfig)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_config_get(int unit, bcm_tunnel_config_t *tconfig)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_tunnel_initiator_clear(int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_initiator_get(int unit, bcm_l3_intf_t *l3_intf,
                              bcm_tunnel_initiator_t *tunnel)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_tunnel_initiator_set(int unit, bcm_l3_intf_t *intf,
                                bcm_tunnel_initiator_t *tunnel)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_terminator_add(int unit, bcm_tunnel_terminator_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_terminator_delete(int unit, bcm_tunnel_terminator_t *info)
{
    return BCM_E_UNAVAIL;
}

int  
bcm_robo_tunnel_terminator_update(int unit, bcm_tunnel_terminator_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_terminator_get(int unit, bcm_tunnel_terminator_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_dscp_map_create(int unit, int *dscp_map_id)
{
    return BCM_E_UNAVAIL;
}
    
int
bcm_robo_tunnel_dscp_map_destroy(int unit, int dscp_map_id)
{   
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_dscp_map_set(int unit, int dscp_map_id,
                             bcm_tunnel_dscp_map_t *dscp_map)
{
    return BCM_E_UNAVAIL;
}
    
int     
bcm_robo_tunnel_dscp_map_get(int unit, int dscp_map_id,
                             bcm_tunnel_dscp_map_t *dscp_map)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_dscp_map_port_set(int unit, bcm_port_t port,
                                  bcm_tunnel_dscp_map_t *dscp_map)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_tunnel_dscp_map_port_get(int unit, bcm_port_t port,
                                  bcm_tunnel_dscp_map_t *dscp_map)
{
    return BCM_E_UNAVAIL;
}


#endif  /* INCLUDE_L3 */
