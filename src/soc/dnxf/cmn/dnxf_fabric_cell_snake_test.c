/*
 * $Id: dnxf_fabric_cell_snake_test.c,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXF FABRIC SNAKE TEST
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxf/cmn/mbcm.h>
#include <soc/dnxf/cmn/dnxf_fabric_cell_snake_test.h>
#include <soc/dnxf/fe1600/fe1600_fabric_cell_snake_test.h>
#include <bcm_int/control.h>
/*
 * Function:
 *      soc_dnxf_cell_snake_test_prepare
 * Purpose:
 *      Prepare system to cell snake test.
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      flags    - (IN)  Configuration parameters
 *      config   - (IN)  Cell snake test configuration
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
  soc_dnxf_cell_snake_test_prepare(
    int unit, 
    uint32 flags)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
    
    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }
    
    DNXF_UNIT_LOCK_TAKE_DNXC(unit);
    
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_cell_snake_test_prepare,(unit, flags));
    DNXC_IF_ERR_EXIT(rc);
    
exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}
    
    
/*
 * Function:
 *      soc_dnxf_cell_snake_test_run
 * Purpose:
 *      Run cell snake test.
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      flags    - (IN)  Configuration parameters
 *      usec     - (IN)  Timeout
 *      rsults   - (IN)  Cell snake test results
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
  soc_dnxf_cell_snake_test_run(
    int unit, 
    uint32 flags, 
    soc_dnxf_fabric_cell_snake_test_results_t* results)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
    
    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Invalid unit")));
    }
    
    DNXC_NULL_CHECK(results);
    
    DNXF_UNIT_LOCK_TAKE_DNXC(unit);
    
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_cell_snake_test_run,(unit, flags, results));
    DNXC_IF_ERR_EXIT(rc);
        
exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    DNXC_FUNC_RETURN;
}


#undef _ERR_MSG_MODULE_NAME

