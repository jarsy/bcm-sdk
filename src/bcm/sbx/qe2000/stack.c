/*
 * $Id: stack.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Stack API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000.h>

#include <bcm/error.h>
#include <bcm/stack.h>

int
bcm_qe2000_stk_modid_set(int unit, int modid)
{
    if (!BCM_STK_MOD_IS_NODE(modid)) {
	return BCM_E_PARAM;
    }

    SOC_SBX_CONTROL(unit)->node_id = BCM_STK_MOD_TO_NODE(modid);
    return soc_qe2000_modid_set(unit, SOC_SBX_CONTROL(unit)->node_id);
}

int
bcm_qe2000_stk_modid_get(int unit, int *modid)
{
    *modid = BCM_STK_NODE_TO_MOD(SOC_SBX_CONTROL(unit)->node_id);
    return BCM_E_NONE;
}

int
bcm_qe2000_stk_my_modid_set(int unit, int modid)
{
    if (!BCM_STK_MOD_IS_NODE(modid)) {
	return BCM_E_PARAM;
    }

    SOC_SBX_CONTROL(unit)->node_id = BCM_STK_MOD_TO_NODE(modid);
    return soc_qe2000_modid_set(unit, SOC_SBX_CONTROL(unit)->node_id);
}

int
bcm_qe2000_stk_my_modid_get(int unit, int *modid)
{
    *modid = BCM_STK_NODE_TO_MOD(SOC_SBX_CONTROL(unit)->node_id);
    return BCM_E_NONE;
}

int
bcm_qe2000_stk_module_enable(int unit,
			     int modid,
			     int nports, 
			     int enable)
{

    return BCM_E_NONE;
}

int
bcm_qe2000_stk_module_protocol_set(int unit,
				   int node,
				   bcm_module_protocol_t  protocol)
{
    int  rc = BCM_E_NONE;
    int  modid;


    rc = bcm_qe2000_stk_modid_get(unit, &modid);
    if (rc != BCM_E_NONE) {
        return(rc);
    }

    if (BCM_STK_MOD_TO_NODE(modid) != node) {
        return(rc);
    }

    /* consistency check */
    if ( (protocol != bcmModuleProtocolNone) && (protocol != bcmModuleProtocol1) &&
                                                   (protocol != bcmModuleProtocol2) ) {
        return(BCM_E_PARAM);
    }

    return BCM_E_NONE;
}

int
bcm_qe2000_stk_module_protocol_get(int unit,
				   int node,
				   bcm_module_protocol_t *protocol)
{
    return BCM_E_NONE;
}
