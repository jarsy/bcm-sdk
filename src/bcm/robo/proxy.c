/*
 * $Id: proxy.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/drv.h>

#ifdef INCLUDE_L3

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l3.h>
#include <bcm/proxy.h>

int
bcm_robo_proxy_client_set(int unit, bcm_port_t client_port, 
                         bcm_proxy_proto_type_t proto_type,
                         bcm_module_t server_modid, bcm_port_t server_port, 
                         int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_proxy_server_set(int unit, bcm_port_t server_port, int mode, 
                         int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_proxy_server_get(int unit, bcm_port_t server_port, 
                         bcm_proxy_mode_t mode, int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_proxy_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_proxy_cleanup(int unit)
{
    return BCM_E_UNAVAIL;
}

#else /* INCLUDE_L3 */
int _bcm_robo_proxy_not_empty;
#endif /* INCLUDE_L3 */

