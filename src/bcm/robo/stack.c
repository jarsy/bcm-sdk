/*
 * $Id: stack.c,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    stack.c
 * Purpose:    BCM level APIs for stacking applications
 *
 * Note : Not for RoboSwitch currently.
 */

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/stack.h>
#include <bcm_int/robo_dispatch.h>
#include <soc/drv.h>


int
bcm_robo_stk_modmap_map(int unit,
                  int setget,
                  bcm_module_t mod_in, bcm_port_t port_in,
                  bcm_module_t *mod_out, bcm_port_t *port_out)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* ROBO devices have no stacking design, return the default value */
    /* default identity mapping */
    if (mod_out != NULL) {
        *mod_out = mod_in;
    }
    if (port_out != NULL) {
        *port_out = port_in;
    }
    return BCM_E_NONE;
}


int
bcm_robo_stk_my_modid_get(int unit, int *my_modid)
{
    *my_modid = unit;
    return BCM_E_NONE;
}

int
bcm_robo_stk_modid_get(int unit, int *modid)
{
    if ((unit < 0) || (unit > BCM_UNITS_MAX)) {
        return BCM_E_PARAM;
    }
    *modid = unit;
    return BCM_E_NONE;
}

int
bcm_robo_stk_modid_set(int unit, int modid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_stk_modid_count(int unit, int *num_modid)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    *num_modid = soc->info.modid_count;

    return BCM_E_NONE;
}


int
bcm_robo_stk_modport_get(int unit, int modid, bcm_port_t *port)
{
    if (port == NULL) {
        return BCM_E_PARAM;
    }

    *port = -1;
    return bcm_robo_stk_modport_get_all(unit, modid, 1, port, NULL);
}

int
bcm_robo_stk_modport_get_all(int unit,
                            int modid,
                            int port_max,
                            bcm_port_t *port_array,
                            int *port_count)
{
    if ((unit < 0) || (unit > BCM_UNITS_MAX)) {
	return BCM_E_PARAM;
    }
    if ((modid < 0) || (modid > BCM_UNITS_MAX)) {
        return BCM_E_PARAM;
    }
    if (port_array == NULL) {
        return BCM_E_PARAM;
    }

    if (port_max > 0) {
        *port_array = 0;
        *port_count = 1;
    } else {
        *port_count = 0;
    }

    return BCM_E_NONE;
}

