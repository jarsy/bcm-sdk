

/*
 * $Id: dnxf_port.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXF PORT
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC

#ifdef BCM_DNXF_SUPPORT
#include <shared/bsl.h>
#include <soc/defs.h>
#include <soc/error.h>
#include <soc/drv.h>

#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxf/cmn/dnxf_warm_boot.h>
#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_port.h>

#ifdef BCM_88750_SUPPORT
#include <soc/dnxf/fe1600/fe1600_defs.h>
#include <soc/dnxf/fe1600/fe1600_port.h>
#endif


/*
 * Function:
 *      soc_dnxf_port_loopback_set
 * Purpose:
 *      Set port loopback
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      loopback  - (IN)  soc_dnxf_loopback_mode_t
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_dnxf_port_loopback_set(int unit, soc_port_t port, soc_dnxc_loopback_mode_t loopback)
{
    int rc;
    soc_dnxc_loopback_mode_t lb_start; 
    uint32 cl72_start;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit) || !SOC_IS_DNXF(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXF_LINK_INPUT_CHECK_DNXC(unit, port);

    if (loopback < soc_dnxc_loopback_mode_none || loopback > soc_dnxc_loopback_mode_phy_gloop) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid loopback")));
    }

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);

    DNXC_IF_ERR_EXIT(soc_dnxf_port_loopback_get(unit, port, &lb_start));
    if (loopback != soc_dnxc_loopback_mode_none) {
        if (lb_start == soc_dnxc_loopback_mode_none) {
            DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_port_phy_control_get,(unit, port, -1, -1, 0, SOC_PHY_CONTROL_CL72, &cl72_start)));
            rc = SOC_DNXF_WARM_BOOT_ARR_VAR_SET(unit, PORT_CL72_CONF, port, &cl72_start);
            DNXC_IF_ERR_EXIT(rc);
            if (cl72_start == 1) {
                DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_port_phy_control_set,(unit, port, -1, -1, 0, SOC_PHY_CONTROL_CL72, 0)));
            }
        }
    }
    
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_port_loopback_set,(unit, port, loopback));
    DNXC_IF_ERR_EXIT(rc);

    if (loopback == soc_dnxc_loopback_mode_none) {
        if (lb_start != soc_dnxc_loopback_mode_none) {
            rc = SOC_DNXF_WARM_BOOT_ARR_VAR_GET(unit, PORT_CL72_CONF, port, &cl72_start);
            DNXC_IF_ERR_EXIT(rc);

            if (cl72_start == 1) {
                DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_port_phy_control_set,(unit, port, -1, -1, 0, SOC_PHY_CONTROL_CL72, 1)));
            }
        }
    }

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_port_burst_control_set,(unit, port, loopback));
    DNXC_IF_ERR_EXIT(rc);

    
exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dnxf_port_loopback_get
 * Purpose:
 *      Get port loopback
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      loopback  - (OUT) soc_dnxf_loopback_mode_t
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_dnxf_port_loopback_get(int unit, soc_port_t port, soc_dnxc_loopback_mode_t* loopback)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit) || !SOC_IS_DNXF(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }

    DNXF_LINK_INPUT_CHECK_DNXC(unit, port);
    DNXC_NULL_CHECK(loopback);

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_port_loopback_get,(unit, port, loopback));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;

}

#endif /* BCM_DNXF_SUPPORT */

#undef _ERR_MSG_MODULE_NAME

