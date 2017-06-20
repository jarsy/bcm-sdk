/*
 * $Id: fe1600_stack.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC FE1600 STACK
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_STK
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/defs.h>
#include <soc/error.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>

#include <soc/dfe/cmn/dfe_drv.h>
#include <shared/bitop.h>

#if defined(BCM_88750_A0)

#include <soc/dfe/fe1600/fe1600_defs.h>
#include <soc/dfe/fe1600/fe1600_stack.h>

/*
 * Function:
 *      soc_fe1600_stk_modid_set
 * Purpose:
 *      Set the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (IN)     FE id  
 * Returns:
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_fe1600_stk_modid_set(int unit, uint32 fe_id)
{
    uint32 reg_val32;
    SOCDNX_INIT_FUNC_DEFS;
	SOC_FE1600_ONLY(unit);
    
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_0r(unit, &reg_val32));
    soc_reg_field_set(unit, ECI_GLOBAL_0r, &reg_val32, DEVICE_IDf, fe_id);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_0r(unit, reg_val32));
   
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_stk_modid_get
 * Purpose:
 *      Get the module ID of the local device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      fe_id - (OUT)    FE id  
 * Returns:
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_fe1600_stk_modid_get(int unit, uint32* fe_id)
{
    uint32 reg_val32;
    SOCDNX_INIT_FUNC_DEFS;
	SOC_FE1600_ONLY(unit);
    
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_0r(unit, &reg_val32));
    *fe_id = soc_reg_field_get(unit, ECI_GLOBAL_0r, reg_val32, DEVICE_IDf);
    
exit:
    SOCDNX_FUNC_RETURN;
  
}


#endif

#undef _ERR_MSG_MODULE_NAME

