/*
 * $Id: mbcm.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        dnxf_mbcm.c
 * Purpose:     Implementation of bcm multiplexing - For fabric element functions
 *
 * Different chip families require such different implementations
 * of some basic BCM layer functionality that the functions are
 * multiplexed to allow a fast runtime decision as to which function
 * to call.  This file contains the basic declarations for this
 * process.
 *
 * This code allows to use the same MBCM_DNXF_DRIVER_CALL API independently of the chip type
 *
 */
/*
 * $Id: mbcm.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2005 Broadcom Corp.
 * All Rights Reserved.$
 *
 * File:        mbcm.h
 * Purpose:     Multiplexing of the bcm layer - For fabric element functions
 *
 * Different chip families require such different implementations
 * of some basic BCM layer functionality that the functions are
 * multiplexed to allow a fast runtime decision as to which function
 * to call.  This file contains the basic declarations for this
 * process.
 *
 * This code allows to use the same MBCM_DNXF_DRIVER_CALL API independently of the chip type
 *
 * See internal/design/soft_arch/xgs_plan.txt for more info.
 *
 * Conventions:
 *    MBCM is the multiplexed bcm prefix
 *    _f is the function type declaration postfix
 */


#include <shared/bsl.h>

#include <soc/dnxf/cmn/mbcm.h>
#include <soc/dnxf/cmn/dnxf_drv.h>

mbcm_dnxf_functions_t    *mbcm_dnxf_driver[BCM_MAX_NUM_UNITS]={0};
soc_dnxf_chip_family_t    mbcm_dnxf_family[BCM_MAX_NUM_UNITS];

/****************************************************************
 *
 * Function:        mbcm_dnxf_init
 * Parameters:      unit   --   unit to setup
 * 
 * Initialize the mbcm driver for the indicated unit.
 *
 ****************************************************************/
int
mbcm_dnxf_init(int unit)
{
#ifdef BCM_88790_A0
    if (SOC_IS_FE1600(unit)) {
        mbcm_dnxf_driver[unit] = &mbcm_ramon_fe1600_driver;
        mbcm_dnxf_family[unit] = BCM_FAMILY_RAMON_FE1600;
        return SOC_E_NONE;
    }
#endif  /* BCM_88790_A0 */
#ifdef BCM_88790_A0
	if (SOC_IS_RAMON(unit)) {
		mbcm_dnxf_driver[unit] = &mbcm_ramon_driver;
		mbcm_dnxf_family[unit] = BCM_FAMILY_RAMON;
		return SOC_E_NONE;
    }
#endif	

    LOG_CLI((BSL_META_U(unit,
                        "ERROR: mbcm_dnxf_init unit %d: unsupported chip type\n"), unit));
    return SOC_E_INTERNAL;
}
