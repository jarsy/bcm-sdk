/*
 * $Id: reg.c,v 1.41 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Register address and value manipulations.
 */

#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/debug.h>
#include <soc/cm.h>

#include <stdarg.h>

#include <soc/robo/mcm/driver.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/register.h>
#include <soc/drv.h>
#include <soc/spi.h>


/*
 *  Function : soc_robo_regaddrlist_alloc
 *
 *  Purpose :
 *      Allocate the register info structure and link to the registers list.
 *
 *  Parameters :
 *      addrlist    :   pointer of the registers list
 *
 *  Return :
 *      SOC_E_NONE  :   success.
 *      SOC_E_MEMORY    :   memory allocation fail.
 *
 *  Note :
 *      
 *
 */
int
soc_robo_regaddrlist_alloc(soc_regaddrlist_t *addrlist)
{
    if ((addrlist->ainfo = sal_alloc(_SOC_ROBO_MAX_REGLIST *
                sizeof(soc_regaddrinfo_t), "regaddrlist")) == NULL) {
        return SOC_E_MEMORY;
    }
    addrlist->count = 0;

    return SOC_E_NONE;
}


/*
 *  Function : soc_robo_regaddrlist_free
 *
 *  Purpose :
 *      Free the register info structure.
 *
 *  Parameters :
 *      addrlist    :   pointer of the registers list
 *
 *  Return :
 *      SOC_E_NONE  :   success.
 *
 *  Note :
 *      
 *
 */
int
soc_robo_regaddrlist_free(soc_regaddrlist_t *addrlist)
{
    if (addrlist->ainfo) {
        sal_free(addrlist->ainfo);
    }

    return SOC_E_NONE;
}

