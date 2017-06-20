/*
 * $Id: ramon_fabric_topology.c,v 1.16.24.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON FABRIC TOPOLOGY
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC 
#include <shared/bsl.h>

#include <soc/dnxf/cmn/dnxf_drv.h>

#include <soc/dnxf/ramon/ramon_fabric_topology.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/defs.h>
#include <soc/error.h>

/*
 * Function:
 *      soc_ramon_fabric_topology_reachabilty_mask_set
 * Purpose:
 *      Isolate / Acrivate fabric
 * Parameters:
 *      unit - (IN) Unit number.
 *      active_links - (IN)
 *      val  - (IN) Link Isolation Status
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fabric_topology_reachability_mask_set(int unit, soc_pbmp_t active_links, soc_dnxc_isolation_status_t val)
{
    soc_port_t port;
    soc_reg_above_64_val_t reg_val;
    uint32 reg32_val = 0;
    DNXC_INIT_FUNC_DEFS;
    
    if (val == soc_dnxc_isolation_status_isolated)
    {
        /*Get automatic RTP update*/
        DNXC_IF_ERR_EXIT(READ_RTP_RESERVED_10r(unit, &reg32_val));

        /*Disable automatic update of RTP*/    
        DNXC_IF_ERR_EXIT(WRITE_RTP_RESERVED_10r(unit, 0x0));
    }

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_val));
    SOC_PBMP_ITER(active_links, port) {
        if (val == soc_dnxc_isolation_status_isolated) {
            SHR_BITCLR(reg_val, port);
        } else {
            SHR_BITSET(reg_val, port);
        }
    }
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_val));
      
    if (val == soc_dnxc_isolation_status_isolated)
    {
        /*Wait to be sure all other devices stopped sending traffic*/
        sal_usleep(30000);

        /*Renable automatic update of RTP*/
        DNXC_IF_ERR_EXIT(WRITE_RTP_RESERVED_10r(unit, reg32_val));
    }

exit:
    DNXC_FUNC_RETURN;
}
/*
 * Function:
 *      soc_ramon_fabric_topology_nof_links_to_min_nof_links_default
 * Purpose:
 *      Retrive link default value for min number of links
 * Parameters:
 *      unit        - (IN)  Unit number.
 *      nof_links      - (IN)
 *      min_nof_links - (OUT)
 * Returns:
 *      SOC_E_xxx
 */
int
soc_ramon_fabric_topology_nof_links_to_min_nof_links_default(int unit, int nof_links, int *min_nof_links)
{
    DNXC_INIT_FUNC_DEFS;

    
    *min_nof_links = 0;

    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fabric_topology_min_nof_links_set
 * Purpose:
 *      Setting min number of links
 * Parameters:
 *      unit        - (IN)  Unit number.
 *      min_nof_links - (IN)
 * Returns:
 *      SOC_E_xxx
 */
int
soc_ramon_fabric_topology_min_nof_links_set(int unit, int min_nof_links)
{
    DNXC_INIT_FUNC_DEFS;

    
    if (min_nof_links != 0)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("RAMON do not support min number of links feature")));
    }
exit:
    DNXC_FUNC_RETURN; 
}


/*
 * Function:
 *      soc_ramon_fabric_topology_min_nof_links_get
 * Purpose:
 *      Getting min number of links
 * Parameters:
 *      unit        - (IN)  Unit number.
 *      min_nof_links - (OUT)
 * Returns:
 *      SOC_E_xxx
 */
int
soc_ramon_fabric_topology_min_nof_links_get(int unit, int *min_nof_links)
{
    DNXC_INIT_FUNC_DEFS;

    
    *min_nof_links = 0;

    DNXC_FUNC_RETURN; 
}
/*
 * Function:
 *      soc_ramon_fabric_topology_isolate_set
 * Purpose:
 *      Isolate / Acrivate fabric
 * Parameters:
 *      unit - (IN) Unit number.
 *      val  - (IN) Link Isolation Status
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fabric_topology_isolate_set(int unit, soc_dnxc_isolation_status_t val)
{
    uint32 reg_val32;

    DNXC_INIT_FUNC_DEFS;

    if(soc_dnxc_isolation_status_active == val) {

        sal_usleep(2000);

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_topology_mesh_topology_reset(unit));
                   
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_topology_rmgr_set, (unit, 1)));
    } else {
        /*Get automatic RTP update*/
        DNXC_IF_ERR_EXIT(READ_RTP_RESERVED_10r(unit,&reg_val32));

        /*Set automatic RTP update*/
        DNXC_IF_ERR_EXIT(WRITE_RTP_RESERVED_10r(unit,0x0));
               
        /*Disable reachability messages*/
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_topology_rmgr_set, (unit, 0)));

        /*Wait to be sure all other devices stopped sending traffic*/
        sal_usleep(30000);

        /*Re-enable automatic update of RTP*/
        DNXC_IF_ERR_EXIT(WRITE_RTP_RESERVED_10r(unit,reg_val32));        
    } 
      
    sal_usleep(20000);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fabric_topology_mesh_topology_reset
 * Purpose:
 *      Rest MESH TOPOLOGY block
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fabric_topology_mesh_topology_reset(int unit)
{
    uint32 reg_val32;
    int ccs_index,nof_instances_ccs;
    DNXC_INIT_FUNC_DEFS;

    nof_instances_ccs = SOC_DNXF_DEFS_GET(unit, nof_instances_ccs);

    /*MESH_TOPOLOGY reset sequence*/
    for (ccs_index = 0; ccs_index < nof_instances_ccs ; ccs_index++)
    {
        DNXC_IF_ERR_EXIT(READ_CCS_FILTERED_CELLS_CONFIGURATIONSr(unit,ccs_index,&reg_val32));
        soc_reg_field_set(unit,CCS_FILTERED_CELLS_CONFIGURATIONSr,&reg_val32,FILTER_CELLSf,1);
        DNXC_IF_ERR_EXIT(WRITE_CCS_FILTERED_CELLS_CONFIGURATIONSr(unit,ccs_index,reg_val32));
    }

    sal_usleep(1000);

    /*reset MESH_TOPOLOGY*/ 
    DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val32));
    soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_5f, 0);
    DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val32));
    /* Enable back */
    DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val32));
    soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_5f, 1);
    DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val32));
    

    for (ccs_index = 0; ccs_index < nof_instances_ccs ; ccs_index++) 
    {
        DNXC_IF_ERR_EXIT(READ_CCS_FILTERED_CELLS_CONFIGURATIONSr(unit,ccs_index,&reg_val32));
        soc_reg_field_set(unit,CCS_FILTERED_CELLS_CONFIGURATIONSr,&reg_val32,FILTER_CELLSf,0);
        DNXC_IF_ERR_EXIT(WRITE_CCS_FILTERED_CELLS_CONFIGURATIONSr(unit,ccs_index,reg_val32));
    }

exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME
