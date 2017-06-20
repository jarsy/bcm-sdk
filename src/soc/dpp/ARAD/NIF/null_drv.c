/*
 * $Id: null_drv.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC NULL DRV
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>
#include <soc/dcmn/error.h>

#include <soc/cm.h>
#include <soc/ll.h>
#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/ARAD/NIF/common_drv.h>


/*
 * Function:
 *      soc_null_drv_init
 * Purpose:
 *      init NULL port
 * Parameters:
 *      unit     - (IN) Unit number.
 *      port     - (IN) Port number.
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
STATIC soc_error_t 
soc_null_drv_init(int unit, soc_port_t port)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_null_drv_enable_set
 * Purpose:
 *      Enable or disable MAC
 * Parameters:
 *      unit - unit number.
 *      port - Port number.
 *      enable - TRUE to enable, FALSE to disable
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_null_drv_enable_set(int unit, soc_port_t port, int enable)
{
    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_null_drv_speed_set
 * Purpose:
 *      Set NULL in the specified speed.
 * Parameters:
 *      unit - unit number.
 *      port - port number.
 *      speed - speed to set
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_null_drv_speed_set(int unit, soc_port_t port, int speed)
{
    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_null_drv_loopback_set
 * Purpose:
 *      Set a NULL into/out-of loopback mode
 * Parameters:
 *      unit - unit number.
 *      port - port number.
 *      loopback - Boolean: true -> loopback mode, false -> normal operation
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_null_drv_loopback_set(int unit, soc_port_t port, int lb)
{
    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_null_drv_loopback_get
 * Purpose:
 *      Get current NULL loopback mode setting.
 * Parameters:
 *      unit - unit number.
 *      port - port number.
 *      loopback - (OUT) Boolean: true = loopback, false = normal
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_null_drv_loopback_get(int unit, soc_port_t port, int *lb)
{
    SOCDNX_INIT_FUNC_DEFS

    *lb = 0;

    SOCDNX_FUNC_RETURN
}

CONST mac_driver_t soc_null_driver = {
    "NULL Driver",                  /* drv_name */
    soc_null_drv_init,              /* md_init  */
    soc_null_drv_enable_set,        /* md_enable_set */
    NULL,                           /* md_enable_get */
    NULL,                           /* md_duplex_set */
    NULL,                           /* md_duplex_get */
    soc_null_drv_speed_set,         /* md_speed_set */
    NULL,                           /* md_speed_get */
    NULL,                           /* md_pause_set */
    NULL,                           /* md_pause_get */
    NULL,                           /* md_pause_addr_set */
    NULL,                           /* md_pause_addr_get */
    soc_null_drv_loopback_set,      /* md_lb_set */
    soc_null_drv_loopback_get,      /* md_lb_get */
    NULL,                           /* md_interface_set */
    NULL,                           /* md_interface_get */
    NULL,                           /* md_ability_get - Deprecated */
    NULL,                           /* md_frame_max_set */
    NULL,                           /* md_frame_max_get */
    NULL,                           /* md_ifg_set */
    NULL,                           /* md_ifg_get */
    NULL,                           /* md_encap_set */
    NULL,                           /* md_encap_get */
    NULL,                           /* md_control_set */
    NULL,                           /* md_control_get */
    NULL                            /* md_ability_local_get */
 };

#undef _ERR_MSG_MODULE_NAME


