/*
 * $Id: stack.c,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DNXF PORT
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_STK
#include <shared/bsl.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm_int/common/debug.h>
#include <bcm/debug.h>
#include <bcm/stat.h>
#include <bcm/stack.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxf/cmn/dnxf_stack.h>
#include <soc/dnxf/cmn/dnxf_fabric.h>
#include <soc/dnxf/cmn/mbcm.h>

/**********************************************************/
/*                  Implementation                        */
/**********************************************************/  

/*
 * Function:
 *      bcm_dnxf_stk_init
 * Purpose:
 *      Initialize Stack module
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      BCM_E_XXX      Error occurred  
 */
 
int 
bcm_dnxf_stk_init(
    int unit)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      _bcm_dnxf_stk_init
 * Purpose:
 *      Internal function Deinitialize Stack module
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      BCM_E_XXX      Error occurred  
 */
 
int 
_bcm_dnxf_stk_deinit(
    int unit)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

int
bcm_dnxf_stk_module_enable(int unit, bcm_module_t modid,
                      int nports, int enable)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    DNXF_UNIT_LOCK_TAKE(unit);
    
    rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_topology_isolate_set,(unit, enable ? soc_dnxc_isolation_status_active : soc_dnxc_isolation_status_isolated));
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    DNXF_UNIT_LOCK_RELEASE(unit); 
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnxf_stk_modid_set
 * Purpose:
 *      Set the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (IN)     FE id  
 * Returns:
 *      BCM_E_XXX      Error occurred  
 */
 
int 
bcm_dnxf_stk_modid_set(
    int unit, 
    uint32 fe_id)
{
    int rv;
    soc_pbmp_t valid_link_bitmap;
    BCMDNX_INIT_FUNC_DEFS;
    
    /*validation*/
    if(!SOC_DNXF_CHECK_MODULE_ID(fe_id)) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_BADID, (_BSL_BCM_MSG("fe_id out of range %d"), fe_id));
    }
    
    DNXF_UNIT_LOCK_TAKE(unit);
    
    rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_stk_modid_set,(unit, fe_id));
    BCMDNX_IF_ERR_EXIT(rv);
    rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_topology_isolate_set,(unit, soc_dnxc_isolation_status_isolated));
    BCMDNX_IF_ERR_EXIT(rv);

    SOC_PBMP_ASSIGN(valid_link_bitmap, SOC_INFO(unit).sfi.bitmap);

    rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_links_bmp_isolate_set,(unit, valid_link_bitmap, soc_dnxc_isolation_status_active)); 
    BCMDNX_IF_ERR_EXIT(rv);
   
exit:   
    DNXF_UNIT_LOCK_RELEASE(unit); 
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnxf_stk_my_modid_set
 * Purpose:
 *      Set the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (IN)     FE id  
 * Returns:
 *      BCM_E_XXX      Error occurred  
 */
 
int 
bcm_dnxf_stk_my_modid_set(
    int unit, 
    uint32 fe_id)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    rv = bcm_dnxf_stk_modid_set(unit, fe_id);
    BCMDNX_IF_ERR_EXIT(rv);

exit:   
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnxf_stk_modid_get
 * Purpose:
 *      Get the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (OUT)    FE id  
 * Returns:
 *      BCM_E_XXX      Error occurred  
 */
int 
bcm_dnxf_stk_modid_get(
    int unit, 
    uint32* fe_id)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;
    
    /*validation*/
    BCMDNX_NULL_CHECK(fe_id);
    
    DNXF_UNIT_LOCK_TAKE(unit);
    
    rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_stk_modid_get,(unit, fe_id));
    BCMDNX_IF_ERR_EXIT(rv);
   
exit:   
    DNXF_UNIT_LOCK_RELEASE(unit); 
    
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnxf_stk_my_modid_get
 * Purpose:
 *      Get the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (OUT)    FE id  
 * Returns:
 *      BCM_E_XXX      Error occurred  
 */
int 
bcm_dnxf_stk_my_modid_get(
    int unit, 
    uint32* fe_id)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    rv = bcm_dnxf_stk_modid_get(unit, fe_id);
    BCMDNX_IF_ERR_EXIT(rv);

exit:   
    BCMDNX_FUNC_RETURN;
}


int 
bcm_dnxf_stk_module_control_get(
    int unit, 
    uint32 flags, 
    bcm_module_t module, 
    bcm_stk_module_control_t control, 
    int *arg)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_NULL_CHECK(arg);

    DNXF_UNIT_LOCK_TAKE(unit);

    switch (control)
    {
        case bcmStkModuleAllReachableIgnore:
            rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_valid_module_id_verify, (unit, module, 0));
            BCMDNX_IF_ERR_EXIT(rv);
            rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_all_reachable_ignore_id_get, (unit, module, arg));
            BCMDNX_IF_ERR_EXIT(rv);
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid control %d"),control));
    }
exit:
    DNXF_UNIT_LOCK_RELEASE(unit);
    BCMDNX_FUNC_RETURN;
}

int 
bcm_dnxf_stk_module_control_set(
    int unit, 
    uint32 flags, 
    bcm_module_t module, 
    bcm_stk_module_control_t control, 
    int arg)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    DNXF_UNIT_LOCK_TAKE(unit);

    switch (control)
    {
    case bcmStkModuleAllReachableIgnore:
            rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_valid_module_id_verify, (unit, module, 0));
            BCMDNX_IF_ERR_EXIT(rv);
            rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_all_reachable_ignore_id_set, (unit, module, arg));
            BCMDNX_IF_ERR_EXIT(rv);
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid control %d"),control));
    }

exit:
    DNXF_UNIT_LOCK_RELEASE(unit);
    BCMDNX_FUNC_RETURN;
}

int 
bcm_dnxf_stk_module_max_get(
    int unit, 
    uint32 flags, 
    bcm_module_t *max_module)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_NULL_CHECK(max_module);

    DNXF_UNIT_LOCK_TAKE(unit);

    if (flags & BCM_STK_MODULE_MAX_ALL_REACHABLE)
    {
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_max_all_reachable_get, (unit, max_module));
        BCMDNX_IF_ERR_EXIT(rv);
    }
    else
    {
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_max_fap_get, (unit, max_module));
        BCMDNX_IF_ERR_EXIT(rv);
    }

exit:
    DNXF_UNIT_LOCK_RELEASE(unit);
    BCMDNX_FUNC_RETURN;
}

int 
bcm_dnxf_stk_module_max_set(
    int unit, 
    uint32 flags, 
    bcm_module_t max_module)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    DNXF_UNIT_LOCK_TAKE(unit);

    if (flags & BCM_STK_MODULE_MAX_ALL_REACHABLE)
    {
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_max_all_reachable_verify, (unit, max_module));
        BCMDNX_IF_ERR_EXIT(rv);

        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_max_all_reachable_set, (unit, max_module));
        BCMDNX_IF_ERR_EXIT(rv);
    }
    else
    {
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_max_fap_verify, (unit, max_module));
        BCMDNX_IF_ERR_EXIT(rv);

        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_stk_module_max_fap_set, (unit, max_module));
        BCMDNX_IF_ERR_EXIT(rv);
    }

exit:
   DNXF_UNIT_LOCK_RELEASE(unit);
   BCMDNX_FUNC_RETURN;   
}

#undef _ERR_MSG_MODULE_NAME

