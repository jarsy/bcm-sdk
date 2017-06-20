/*
 * $Id: ramon_fe1600_stack.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 STACK
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_STK
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/defs.h>
#include <soc/error.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <shared/bitop.h>

#if defined(BCM_88790_A0)

#include <soc/dnxf/fe1600/fe1600_defs.h>
#include <soc/dnxf/fe1600/fe1600_stack.h>

/*
 * Function:
 *      soc_ramon_fe1600_stk_modid_set
 * Purpose:
 *      Set the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (IN)     FE id  
 * Returns:
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_ramon_fe1600_stk_modid_set(int unit, uint32 fe_id)
{
    uint32 reg_val32;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);
    
    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_0r(unit, &reg_val32));
    soc_reg_field_set(unit, ECI_GLOBAL_0r, &reg_val32, DEVICE_IDf, fe_id);
    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_0r(unit, reg_val32));
   
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_stk_modid_get
 * Purpose:
 *      Get the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (OUT)    FE id  
 * Returns:
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_ramon_fe1600_stk_modid_get(int unit, uint32* fe_id)
{
    uint32 reg_val32;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);
    
    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_0r(unit, &reg_val32));
    *fe_id = soc_reg_field_get(unit, ECI_GLOBAL_0r, reg_val32, DEVICE_IDf);
    
exit:
    DNXC_FUNC_RETURN;
  
}


#endif

#undef _ERR_MSG_MODULE_NAME

