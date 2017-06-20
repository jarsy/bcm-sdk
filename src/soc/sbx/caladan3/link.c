/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: link.c,v 1.1.2.1 Broadcom SDK $
 *
 * File:    link.c
 * Purpose: Caladan3 link related drivers
 * Requires:
 * Notes:
 */

#include <soc/types.h>
#include <soc/drv.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include <soc/cm.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/error.h>
#include <soc/sbx/caladan3/port.h>
#include <sal/appl/sal.h>
#include <soc/sbx/sbx_drv.h>

#include <soc/sbx/link.h>
#include <soc/linkctrl.h>
/*
 * Function
 *    soc_sbx_caladan3_init
 * Purpose
 *    Initialize the mac core based on port configuration
 */
int
soc_sbx_caladan3_link_init(int unit)
{


    /* Initialize SOC link control module. Use the generic driver.
     * This should be overrode to make hardware linkscan functional */
      soc_linkctrl_init(unit, &soc_linkctrl_driver_sbx);

      return SOC_E_NONE;

}

#endif /* BCM_CALADAN3_SUPPORT */
