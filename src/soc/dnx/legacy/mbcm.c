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
#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/mbcm.h>

CONST mbcm_dnx_functions_t    *mbcm_dnx_driver[BCM_MAX_NUM_UNITS]={0};
soc_dnx_chip_family_t    mbcm_dnx_family[BCM_MAX_NUM_UNITS];

/****************************************************************
 *
 * Function:        mbcm_dnx_init
 * Parameters:      unit   --   unit to setup
 * 
 * Initialize the mbcm driver for the indicated unit.
 *
 ****************************************************************/
int
mbcm_dnx_init(int unit)
{
    
    if (SOC_IS_JERICHO_2_A0(unit)) 
    {
        mbcm_dnx_driver[unit] = &mbcm_jer2_driver;
        mbcm_dnx_family[unit] = BCM_FAMILY_JER2;
        return SOC_E_NONE;
    }


    LOG_INFO(BSL_LS_BCM_INIT,
             (BSL_META_U(unit,
                         "ERROR: mbcm_dnx_init unit %d: unsupported chip type\n"), unit));
    return SOC_E_INTERNAL;
}
