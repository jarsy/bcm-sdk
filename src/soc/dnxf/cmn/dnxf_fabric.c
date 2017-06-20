/*
 * $Id: dnxf_fabric.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXF FABRIC
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC

#ifdef BCM_DNXF_SUPPORT
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <bcm/fabric.h>
#include <soc/defs.h>
#include <soc/error.h>
#include <bcm/error.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_fabric.h>
#include <soc/dnxf/cmn/mbcm.h>

#include <soc/dnxf/fe1600/fe1600_defs.h>
#include <soc/dnxf/fe1600/fe1600_fabric_flow_control.h>
#include <soc/dnxf/fe1600/fe1600_fabric_links.h>
#include <soc/dnxf/fe1600/fe1600_fabric_multicast.h>
#include <soc/dnxf/fe1600/fe1600_fabric_status.h>
#include <soc/dnxf/fe1600/fe1600_fabric_topology.h>
#include <bcm_int/control.h>

/*
 * Function:
 *      soc_dnxf_fabric_link_status_all_get
 * Purpose:
 *      Get all links status
 * Parameters:
 *      unit                 - (IN)  Unit number.
 *      links_array_max_size - (IN)  max szie of link_status array
 *      link_status          - (OUT) array of link status per link
 *      errored_token_count  - (OUT) array error token count per link
 *      links_array_count    - (OUT) array actual size
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_dnxf_fabric_link_status_all_get(int unit, int links_array_max_size, uint32* link_status, uint32* errored_token_count, int* links_array_count)
{
    int rc, nof_links = 0;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }
    
    nof_links = SOC_DNXF_DEFS_GET(unit, nof_links);
    if(links_array_max_size < nof_links) {
         DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Array is too small")));
    }

    DNXC_NULL_CHECK(link_status);
    DNXC_NULL_CHECK(errored_token_count);
    DNXC_NULL_CHECK(links_array_count);

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_link_status_all_get,(unit, links_array_max_size, link_status, errored_token_count, links_array_count));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit); 
    DNXC_FUNC_RETURN;
}

#endif /* BCM_DNXF_SUPPORT */

#undef _ERR_MSG_MODULE_NAME

