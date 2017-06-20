/*
 * $Id: mbcm.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mbcm.c
 * Purpose:     Implementation of bcm multiplexing
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/error.h>

#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/mbcm.h>

mbcm_sbx_functions_t    *mbcm_sbx_driver[BCM_MAX_NUM_UNITS]={0};
soc_sbx_chip_family_t    mbcm_sbx_family[BCM_MAX_NUM_UNITS];

/****************************************************************
 *
 * Function:        mbcm_sbx_init
 * Parameters:      unit   --   unit to setup
 * 
 * Initialize the mbcm driver for the indicated unit.
 *
 ****************************************************************/
int
mbcm_sbx_init(int unit)
{
#ifdef  BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_qe2000_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_QE2000;
        return BCM_E_NONE;
    }
#endif  /* BCM_QE2000_SUPPORT */
#ifdef  BCM_BME3200_SUPPORT
    if (SOC_IS_SBX_BME3200(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_bm3200_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_BM3200;
        return BCM_E_NONE;
    }
#endif  /* BCM_BME3200_SUPPORT */

#ifdef  BCM_BM9600_SUPPORT
    if (SOC_IS_SBX_BM9600(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_bm9600_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_BM9600;
        return BCM_E_NONE;
    }
#endif  /* BCM_BM9600_SUPPORT */
#ifdef  BCM_SIRIUS_SUPPORT
    if (SOC_IS_SBX_SIRIUS(unit)) {
        mbcm_sbx_driver[unit] = &mbcm_sirius_driver;
        mbcm_sbx_family[unit] = BCM_FAMILY_SIRIUS;
        return BCM_E_NONE;
    }
#endif  /* BCM_SIRIUS_SUPPORT */

    LOG_CLI((BSL_META_U(unit,
                        "ERROR: mbcm_sbx_init unit %d: unsupported chip type\n"), unit));
    return BCM_E_INTERNAL;
}
