/*
 * $Id: port.c,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/drv.h>
#include <bcm_int/esw/port.h>
#include <bcm/error.h>
#include <bcm/types.h>

#if defined(BCM_GREYHOUND2_SUPPORT)
int bcmi_gh2_port_mac_type_get(int unit, soc_port_t port,
                               bcmi_port_mac_type_t *port_type)

{
    if (NULL == port_type) {
        return SOC_E_PARAM;
    }
    if (IS_XL_PORT(unit, port)) {
        *port_type = bcmiPortMacTypeXLmac;
        return SOC_E_NONE;
    } else if (IS_GE_PORT(unit, port)) {
        *port_type = bcmiPortMacTypeUnimac;
        return SOC_E_NONE;
    } else if (IS_CL_PORT(unit, port)) {
        *port_type = bcmiPortMacTypeCLmac;
        return SOC_E_NONE;
    } else {
        return SOC_E_PARAM;
    }
}
#endif /* BCM_GREYHOUND2_SUPPORT */

