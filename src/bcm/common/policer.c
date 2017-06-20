/*
 * $Id: policer.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    policer.c
 * Purpose: Manages common POLICER functions
 */

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/debug.h>

#include <bcm/policer.h>

/*
 * Function:
 *      bcm_policer_config_t_init
 * Purpose:
 *      Initialize the policer config struct
 * Parameters:
 *      pol_cfg - Pointer to the struct to be init'ed
 */

void
bcm_policer_config_t_init(bcm_policer_config_t *pol_cfg)
{
    if (pol_cfg != NULL) {
        sal_memset(pol_cfg, 0, sizeof(*pol_cfg));
    }
    return;
}

/*
 * Function:
 *      bcm_policer_group_mode_attr_selector_t_init
 * Purpose:
 *      Initialize the policer group attribute selector structure
 * Parameters:
 *      pol_group_mode_attr_selector - Pointer to the struct to be init'ed
 */

void
bcm_policer_group_mode_attr_selector_t_init(bcm_policer_group_mode_attr_selector_t
                                            *pol_group_mode_attr_selector)
{
    if (pol_group_mode_attr_selector != NULL) {
        sal_memset(pol_group_mode_attr_selector, 0,
                   sizeof(*pol_group_mode_attr_selector));
    }
    return;
}

/*
 * Function:
 *      bcm_policer_global_meter_config_t_init
 * Purpose:
 *      Initialize the Global Meter Configuration structure
 * Parameters:
 *      config - Pointer to the struct to be init'ed
 */
void
bcm_policer_global_meter_config_t_init(bcm_policer_global_meter_config_t *config)
{
    if (config != NULL) {
        sal_memset(config, 0, sizeof(*config));
    }
    return;
}
