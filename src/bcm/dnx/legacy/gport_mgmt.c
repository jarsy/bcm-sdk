/*
 * $Id: gport_mgmt.c,v 1.247 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * The GPORT Management
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_PORT

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <shared/swstate/sw_state_hash_tbl.h>

#include "bcm_int/common/debug.h"

#include <sal/core/libc.h>
#include <sal/core/alloc.h>

#include <soc/drv.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/cosq.h>

#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/types.h>
#include <shared/hash_tbl.h>
#include <bcm_int/common/multicast.h>

#include <bcm_int/dnx/legacy/utils.h>
#include <bcm_int/dnx/legacy/error.h>
#include <bcm_int/dnx/legacy/alloc_mngr.h>
#include <bcm_int/dnx/legacy/gport_mgmt.h>
#include <bcm_int/dnx_dispatch.h>
#include <bcm_int/common/multicast.h>

#include <soc/dnx/legacy/TMC/tmc_api_ports.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_packet_queuing.h>

/*
 * Defines
 */
 /* maximum gport used*/



#define GPORT_MGMT_ACCESS           sw_state_access[unit].dnx.bcm.gport_mgmt




/*
 * Functions
 */


/*
 * Function:
 *      _bcm_dnx_gport_mgmt_sw_state_init
 * Purpose:
 *      allocate and Initialize SW state needed for gport management
 * Parameters:
 *      unit        - unit number
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_dnx_gport_mgmt_sw_state_init(int                     unit)
{
    uint8 is_allocated;
    bcm_error_t rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    if(!SOC_WARM_BOOT(unit)) {
        rv = GPORT_MGMT_ACCESS.is_allocated(unit, &is_allocated);
        BCMDNX_IF_ERR_EXIT(rv);

        if(!is_allocated) {
            rv = GPORT_MGMT_ACCESS.alloc(unit);
            BCMDNX_IF_ERR_EXIT(rv);
        }
    }


exit:
    BCMDNX_FUNC_RETURN;
}


int
_bcm_dnx_gport_mgmt_sw_state_cleanup(int                     unit)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_FUNC_RETURN;
}


/*
 * SW stat access functions 
 * } 
 */



/*
 * Function:
 *      _bcm_dnx_gport_mgmt_init
 * Purpose:
 *      init Gport module including
 *        - SW state init
 *        - define [port] profiles for AC lookup: for each tag-format whether to lookup PV or PVV
 *        - define [in/out AC] profiles + commands for ing/eg vlan editing: for each tag-format what vlans to remove/push
 * Parameters:
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_dnx_gport_mgmt_init(int                     unit)
{
    int rv = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;
    rv = _bcm_dnx_gport_mgmt_sw_state_init(unit);
    BCMDNX_IF_ERR_EXIT(rv);

    /* drop trap code */

exit:
    BCMDNX_FUNC_RETURN;
}



/*
 * Function:
 *    _bcm_dpp_gport_to_phy_port
 * Description:
 *    map gport to PPD local and system port
 * Parameters:
 *    unit -    [IN] DPP device unit number (driver internal).
 *  gport -   [IN] general port
 *  local_only -     [IN] get only local port
 *  pbmp_local_ports -     [OUT] bitmap of local ports
 *  sys_port -     [OUT] system port (returns if local_only is not set)
 *  is_local_ports -  [OUT] is gport contains local ports
 *  is_lag -     [OUT] is port is physical port or trunk or invalid.
 * Returns:
 *    BCM_E_XXX
 *    BCM_E_PORT if given gport is not physical port (but logical port)
 */
int 
_bcm_dnx_gport_to_phy_port(int unit, bcm_gport_t port, uint32  operations, _bcm_dnx_gport_info_t* gport_info)
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;

}
/* given gport, return if it's local in this unit */
int 
_bcm_dnx_gport_is_local(int unit, bcm_gport_t port,  int *is_local)
{
    _bcm_dnx_gport_info_t gport_info;
    BCMDNX_INIT_FUNC_DEFS;

    if(BCM_GPORT_IS_TUNNEL(port)) {
      *is_local = 1;
    } else {
      BCMDNX_IF_ERR_EXIT(_bcm_dnx_gport_to_phy_port(unit, port, 0, &gport_info));
      (*is_local) = _BCM_DNX_GPORT_INFO_IS_LOCAL_PORT(gport_info);
    }

exit:
    BCMDNX_FUNC_RETURN;

}



/*
 * Function:
 *    _bcm_dnx_gport_to_tm_dest_info
 * Description:
 *    convert gport from TM dest information
 * Parameters:
 *  unit -           [IN] DNX device unit number (driver internal).
 *  gport -          [OUT] general port
 *  soc_dnx_dest_info - [OUT] Soc_dnx destination info
 * Returns:
 *    BCM_E_XXX
 */
int 
_bcm_dnx_gport_from_tm_dest_info(int unit, bcm_gport_t *gport, DNX_TMC_DEST_INFO  *soc_dnx_dest_info)
{
    BCMDNX_INIT_FUNC_DEFS;
    /* verify input parameters */
    BCMDNX_NULL_CHECK(soc_dnx_dest_info);
    BCMDNX_NULL_CHECK(gport);
    
    switch(soc_dnx_dest_info->dbal_type) {
    case DBAL_FIELD_PORT_ID: /** DNX_TMC_DEST_TYPE_SYS_PHY_PORT */
        /* map system port to mod-port*/
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        rv = _bcm_dnx_port_mod_port_from_sys_port(unit,&modid,&mode_port,soc_dnx_dest_info->id);
        BCMDNX_IF_ERR_EXIT(rv);
        BCM_GPORT_MODPORT_SET(*gport,modid,mode_port);
#endif 
        break;
    case DBAL_FIELD_MC_ID: /** DNX_TMC_DEST_TYPE_MULTICAST */
        BCM_GPORT_MCAST_SET(*gport,soc_dnx_dest_info->id);
        break;
    case DBAL_FIELD_LAG_ID: /** DNX_TMC_DEST_TYPE_LAG */
        BCM_GPORT_TRUNK_SET(*gport,soc_dnx_dest_info->id);
        break;
    case DBAL_FIELD_FLOW_ID: /** DNX_TMC_DEST_TYPE_QUEUE */
        BCM_GPORT_UNICAST_QUEUE_GROUP_SET(*gport,soc_dnx_dest_info->id);
        break;
    default:
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("unknown destination type")));

    }

exit:
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *    _bcm_dnx_gport_to_tm_dest_info
 * Description:
 *    convert gport to TM dest information
 * Parameters:
 *    unit -           [IN] DNX device unit number (driver internal).
 *  gport -          [IN] general port
 *  soc_dnx_dest_info - [OUT] Soc_dnx destination info
 * Returns:
 *    BCM_E_XXX
 */
int 
_bcm_dnx_gport_to_tm_dest_info(int unit, bcm_gport_t port, DNX_TMC_DEST_INFO  *soc_dnx_dest_info)
{
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_IF_ERR_EXIT(dnx_gport_to_tm_dest_info(unit, port, soc_dnx_dest_info));
exit:
    BCMDNX_FUNC_RETURN;
}

