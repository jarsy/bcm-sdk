/* 
 * $Id: link.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        link.c
 * Purpose:     BCM Linkscan module for Caladan3
 *
 */
#if 1


#include <shared/bsl.h>

#include <soc/linkctrl.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>

#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm_int/common/link.h>
#include <bcm_int/sbx/port.h>
#endif

#include <sal/types.h>
#include <bcm_int/sbx/caladan3/port.h>
#include <bcm_int/common/link.h>
#include <bcm/error.h>

/*
 * Caladan3 driver routines for linkscan module
 */

/*
 * Function:
 *     _bcm_caladan3_ld_hw_interrupt
 * Purpose:
 *     Routine handler for hardware linkscan interrupt.
 * Parameters:
 *     unit - Device unit number
 *     pbmp - (OUT) Returns bitmap of ports that require hardware re-scan
 */
STATIC void
_bcm_caladan3_ld_hw_interrupt(int unit, bcm_pbmp_t *pbmp)
{

    /* Does nothing */
}

/*
 * Function:
 *     _bcm_caladan3_ld_port_link_get
 * Purpose:
 *     Return current PHY up/down status.
 * Parameters:
 *     unit - Device unit number
 *     port - Device port number
 *     hw   - If TRUE, assume hardware linkscan is active and use it
 *              to reduce PHY reads.
 *            If FALSE, do not use information from hardware linkscan.
 *     up   - (OUT) TRUE for link up, FALSE for link down.
 * Returns:
 *     BCM_E_NONE
 *     BCM_E_XXX
 */
STATIC int
_bcm_caladan3_ld_port_link_get(int unit, bcm_port_t port, int hw, int *up)
{

    return bcm_caladan3_port_link_get(unit, port, hw, up);
}

/*
 * Function:
 *     _bcm_caladan3_link_internal_select
 * Purpose:
 *     Select the source of the CMIC link status interrupt
 *     to be the Internal Serdes on given port.
 * Parameters:
 *     unit - Device unit number
 *     port - Device port number
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_caladan3_ld_internal_select(int unit, bcm_port_t port)
{

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_ld_update_asf
 * Purpose:
 *     Update Alternate Store and Forward parameters for a port.
 * Parameters:
 *     unit   - Device unit number
 *     port   - Device port numberl_
 *     linkup - port link state (0=down, 1=up)
 *     speed  - port speed
 *     duplex - port duplex (0=half, 1=full)
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_caladan3_ld_update_asf(int unit, bcm_port_t port, int linkup,
                            int speed, int duplex)
{
    /* No action for C3 */
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_caladan3_link_trunk_sw_ftrigger
 * Purpose:
 *     Remove specified ports with link down from trunks.
 * Parameters:
 *     unit        - Device unit number
 *     pbmp_active - Bitmap of ports
 *     pbmp_status - Bitmap of port status
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_caladan3_ld_trunk_sw_ftrigger(int unit, bcm_pbmp_t pbmp_active,
                                   bcm_pbmp_t pbmp_status)
{
    /* No action for C3 */
    return BCM_E_NONE;
}


/* Tie in the routines passed back to the driver */
static _bcm_ls_driver_t  _bcm_ls_driver_caladan3 = {
    _bcm_caladan3_ld_hw_interrupt,      /* ld_hw_interrupt */
    _bcm_caladan3_ld_port_link_get,     /* ld_port_link_get */
    _bcm_caladan3_ld_internal_select,   /* ld_internal_select */
    _bcm_caladan3_ld_update_asf,        /* ld_update_asf */
    _bcm_caladan3_ld_trunk_sw_ftrigger  /* ld_trunk_sw_failover_trigger */
};



/*
 * Function:
 *     bcm_caladan3_linkscan_init
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
bcm_caladan3_linkscan_init(int unit)
{
    int  rv;

    /* Pass back our driver callbacks */
    rv = _bcm_linkscan_init(unit, &_bcm_ls_driver_caladan3);

    LOG_INFO(BSL_LS_BCM_LINK,
             (BSL_META_U(unit,
                         "BCM linkscan init unit=%d rv=%d(%s)\n"),
              unit, rv, bcm_errmsg(rv)));

    return rv;
}
