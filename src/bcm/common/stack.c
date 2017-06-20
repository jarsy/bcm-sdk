/*
 * $Id: port.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <bcm/stack.h>
#include <sal/core/libc.h>

/*
 * Function:
 *      bcm_stk_modid_config_t_init
 * Purpose:
 *      Initialize a bcm_stk_modid_config_t struct.
 * Parameters:
 *      pconfig - Pointer to the bcm_stk_modid_config_t struct.
 * Returns:
 *      NONE
 */
void
bcm_stk_modid_config_t_init(bcm_stk_modid_config_t *pconfig)
{
    if (pconfig != NULL) {
        sal_memset(pconfig, 0, sizeof (*pconfig));
    }
    return;
}

