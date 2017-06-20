/*
 * $Id: link.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        link.c
 * Purpose:     BCM Linkscan module 
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_LINK

#include <shared/bsl.h>

#include <bcm_int/common/debug.h>
#include <sal/types.h>
#include <soc/linkctrl.h>
#include <soc/cmicm.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm_int/dnx/legacy/link.h>
#include <bcm_int/common/link.h>
#include <bcm_int/dnx/legacy/gport_mgmt.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnxc/legacy/dnxc_wb.h>

static _bcm_ls_driver_t  _bcm_ls_driver_dnx = {
    NULL,                        /* ld_hw_interrupt */
    
#ifdef FIXME_DNX_LEGACY
    _bcm_dnx_port_link_get,    /* ld_port_link_get */
#else 
    NULL,                       /* ld_port_link_get */
#endif 

    NULL,                        /* ld_internal_select */
    NULL,                        /* ld_update_asf */
    NULL                         /* ld_trunk_sw_failover_trigger */
};


/*
 * Function:
 *     bcm_dnx_linkscan_init
 * Purpose:
 *     Initialize the linkscan software module.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If specific HW linkscan initialization is required by device,
 *     driver should call that.
 */


int
_bcm_jer2_arad_linkscan_init(int unit)
{
   int  rv;
   BCMDNX_INIT_FUNC_DEFS;
   
   rv = _bcm_linkscan_init(unit, &_bcm_ls_driver_dnx);
   BCMDNX_IF_ERR_EXIT(rv);
   
#ifdef BCM_CMICM_SUPPORT
   if(!SOC_WARM_BOOT(unit)){
       soc_cmicm_intr0_enable(unit, IRQ_CMCx_LINK_STAT_MOD);
   }
#endif /* BCM_CMICM_SUPPORT */

exit:
    BCMDNX_FUNC_RETURN; 

}


int
bcm_dnx_linkscan_init(int unit)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_IF_ERR_EXIT(_bcm_jer2_arad_linkscan_init(unit));

exit:
    BCMDNX_FUNC_RETURN;
}

int 
bcm_dnx_linkscan_mode_set(
    int unit, 
    bcm_port_t port, 
    int mode)
{
    uint32 flags;
    _bcm_dnx_gport_info_t   gport_info;
    bcm_port_t port_ndx;
    BCMDNX_INIT_FUNC_DEFS;
  
    BCMDNX_IF_ERR_EXIT(_bcm_dnx_gport_to_phy_port(unit, port, _BCM_DNX_GPORT_TO_PHY_OP_LOCAL_IS_MANDATORY, &gport_info));

    BCM_PBMP_ITER(gport_info.pbmp_local_ports, port_ndx) {
        if (!IS_SFI_PORT(unit, port)) {
            /*Statistic ports are not supported*/
            BCMDNX_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port, &flags));
            if(DNX_PORT_IS_STAT_INTERFACE(flags)) {
                BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Stat port are not supported by linkscan"))); 
            }

            /*ILKN is not supported in HW mode*/
            if (IS_IL_PORT(unit, port) && mode == BCM_LINKSCAN_MODE_HW) {
                BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("ILKN port are not supported by linkscan HW mode"))); 
            }
        }

        /* disable WB since the following function call calls for a common function which dispatches an API */
        _DNXC_BCM_WARM_BOOT_API_TEST_OVERRIDE_WB_TEST_MODE(unit);
         
        BCMDNX_IF_ERR_EXIT(bcm_common_linkscan_mode_set(unit, port, mode));

        /* re enable WB */
        _DNXC_BCM_WARM_BOOT_API_TEST_RETRACT_OVERRIDEN_WB_TEST_MODE(unit);
    }
    
exit:
    BCMDNX_FUNC_RETURN;
}
int bcm_dnx_linkscan_trigger_event_set(
      int unit,
      bcm_port_t port,
      uint32 flags,
      bcm_linkscan_trigger_event_t trigger_event,
      int enable)
{
    BCMDNX_INIT_FUNC_DEFS;

    switch (trigger_event) {
        case BCM_LINKSCAN_TRIGGER_EVENT_FAULT:
            if (port != -1) {
                BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Only port=-1 is supported for this Event type!")));
            }
            if (enable == 0) {
                BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("This Event type can only be enabled!")));
            }
            break;
        default:
            BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Event type is not supported")));
    }
    BCMDNX_IF_ERR_EXIT(bcm_common_linkscan_trigger_event_set(unit, port, flags, trigger_event, enable));
exit:
    BCMDNX_FUNC_RETURN;
}
int bcm_dnx_linkscan_trigger_event_get(
      int unit,
      bcm_port_t port,
      uint32 flags,
      bcm_linkscan_trigger_event_t trigger_event,
      int *enable)
{
    BCMDNX_INIT_FUNC_DEFS;

    switch (trigger_event) {
        case BCM_LINKSCAN_TRIGGER_EVENT_FAULT:
            if (port != -1) {
                BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Only port=-1 is supported for this Event type!")));
            }
            break;
        default:
            BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Event type is not supported")));
    }
    BCMDNX_IF_ERR_EXIT(bcm_common_linkscan_trigger_event_get(unit, port, flags, trigger_event, enable));
exit:
    BCMDNX_FUNC_RETURN;
}


