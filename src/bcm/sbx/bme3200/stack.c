/*
 * $Id: stack.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM3200 Stack API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pt_auto.h>

#include <bcm/error.h>
#include <bcm/stack.h>

int
bcm_bm3200_stk_module_enable(int unit,
			     int modid,
			     int nports, 
			     int enable)
{
    uint32 uData, ina_enable;
    int node;
    /* initial release, nports is fixed as 50 */


    if (!(BCM_STK_MOD_IS_NODE(modid))) {
        return(BCM_E_PARAM);
    }

    node = BCM_STK_MOD_TO_NODE(modid);

    /* update INA_ENABLE when enable/disable nodes */
    uData = SAND_HAL_READ(unit, PT, FO_CONFIG1);
    ina_enable = SAND_HAL_GET_FIELD(PT, FO_CONFIG1, INA_ENABLE, uData);

    if (enable) {
    /* coverity[large_shift] */
	ina_enable |= (1 << node);
    } else {
    /* coverity[large_shift] */
	ina_enable &= ~(1 << node);
    }

    uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG1, INA_ENABLE, uData, ina_enable);
    SAND_HAL_WRITE(unit, PT, FO_CONFIG1, uData);

    return BCM_E_NONE;
}

