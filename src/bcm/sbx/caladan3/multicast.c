/*
 * $Id: multicast.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    multicast.c
 * Purpose: Manages multicast functions
 */

#include <bcm/error.h>
#include <bcm/multicast.h>

extern int
_bcm_caladan3_multicast_l2_encap_get(int              unit, 
                                     bcm_multicast_t  group,
                                     bcm_gport_t      port,
                                     bcm_vlan_t       vlan,
                                     bcm_if_t        *encap_id);

extern int
_bcm_caladan3_ipmc_l3_encap_get(int                     unit,
                                int                     ipmc_index,
                                bcm_gport_t             gport,
                                bcm_if_t                intf_id,
                                bcm_if_t               *encap_id);

extern int
_bcm_caladan3_mpls_vpls_encap_get(int              unit, 
                                  bcm_multicast_t  group,
                                  bcm_gport_t      port,
                                  bcm_gport_t      mpls_port_id,
                                  bcm_if_t        *encap_id);

extern int _bcm_caladan3_multicast_mim_encap_get(int unit,
                                                 bcm_multicast_t group,
                                                 bcm_gport_t port,
                                                 bcm_gport_t mim_port_id,
                                                 bcm_if_t *encap_id);

int 
bcm_caladan3_multicast_l3_encap_get(int unit, 
                                    bcm_multicast_t group, 
                                    bcm_gport_t port, 
                                    bcm_if_t intf, 
                                    bcm_if_t *encap_id)
{
      /*
       * Used for IP Multicast replication
       */
    
    return (_bcm_caladan3_ipmc_l3_encap_get(unit,
                                            group,
                                            port,
                                            intf,
                                            encap_id));
}

int 
bcm_caladan3_multicast_l2_encap_get(int unit, 
                                    bcm_multicast_t group, 
                                    bcm_gport_t port, 
                                    bcm_vlan_t vlan, 
                                    bcm_if_t *encap_id)
{
    return (_bcm_caladan3_multicast_l2_encap_get(unit,
                                                 group,
                                                 port,
                                                 vlan,
                                                 encap_id));
}

int
bcm_caladan3_multicast_vpls_encap_get(int              unit, 
                                      bcm_multicast_t  group,
                                      bcm_gport_t      port,
                                      bcm_gport_t      mpls_port_id,
                                      bcm_if_t        *encap_id)
{
    return (_bcm_caladan3_mpls_vpls_encap_get(unit, group,
                                              port, mpls_port_id,
                                              encap_id));
}

int
bcm_caladan3_multicast_mim_encap_get(int unit,
                                     bcm_multicast_t group,
                                     bcm_gport_t port,
                                     bcm_gport_t mim_port_id,
                                     bcm_if_t *encap_id)
{
#ifdef  BCM_CALADAN3_MIM_SUPPORT 
    return (_bcm_caladan3_multicast_mim_encap_get(unit,
                                                  group,
                                                  port,
                                                  mim_port_id,
                                                  encap_id));
#else
    return BCM_E_UNAVAIL;
#endif
}

