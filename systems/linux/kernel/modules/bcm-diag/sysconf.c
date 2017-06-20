/*
 * $Id: sysconf.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bslext.h>

#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/appl/io.h>

#include <soc/drv.h>

#include <appl/diag/sysconf.h>

#include <bcm-core.h>

int
sysconf_probe(void)
{
    return 0;
}

int
sysconf_attach(int unit)
{
#if defined(BCM_EA_SUPPORT) && defined(BCM_TK371X_SUPPORT)
    if(SOC_IS_EA(unit)){
        soc_ea_pre_attach(unit);
        return 0;
    }
#endif /* defined(BCM_EA_SUPPORT) && defined(BCM_TK371X_SUPPORT) */
    
    if ( (soc_attached(unit)) ||
        (CMDEV(unit).dev.info->dev_type & SOC_ETHER_DEV_TYPE) ) {
        printk("OK - already attached\n");
        return 0;
    }
    return -1;
}

int
sysconf_detach(int unit)
{
    return 0;
}

int
sysconf_init(void)
{
    return 0;
}
