/*
 * $Id: multicast.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Mirror support for SBX fabric.
 */

#include <soc/drv.h>
#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm_int/sbx/cosq.h> /* required for sirius.h */
#include <bcm_int/sbx/sirius.h>

int
bcm_sbx_mirror_port_set(int unit,
                        bcm_port_t port,
                        bcm_module_t dest_mod,
                        bcm_port_t dest_port,
                        uint32 flags)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        return bcm_sirius_mirror_port_set(unit,port,dest_mod,dest_port,flags);
    }
#endif /* BCM_SIRIUS_SUPPORT */
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_mirror_port_get(int unit,
                        bcm_port_t port,
                        bcm_module_t *dest_mod,
                        bcm_port_t *dest_port,
                        uint32 *flags)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        return bcm_sirius_mirror_port_get(unit,port,dest_mod,dest_port,flags);
    }
#endif /* BCM_SIRIUS_SUPPORT */
    return BCM_E_UNAVAIL;
}

