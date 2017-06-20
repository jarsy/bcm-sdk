/*
 * $Id: dcmn_cmic.h,v 1.0 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DCMN IPROC
 */

#ifndef _SOC_DCMN_CMIC_DRV_H_
#define _SOC_DCMN_CMIC_DRV_H_

#include <soc/defs.h>
#include <soc/dcmn/dcmn_defs.h>
#ifdef BCM_DPP_SUPPORT
#include <soc/dpp/drv.h>
#endif /* BCM_DPP_SUPPORT */


/*
 * Function:
 *      soc_dcmn_cmic_device_hard_reset
 * Purpose:
 *      if proper reset action is given, resets device and makes sure device is out of reset.
 *      reset flags are: SOC_DCMN_RESET_ACTION_IN_RESET, SOC_DCMN_RESET_ACTION_INOUT_RESET
 * Parameters:
 *      unit            - Device Number
 *      reset_action    - Action to perform
 * Returns:
 *      SOC_E_XXX
 */
int soc_dcmn_cmic_device_hard_reset(int unit, int reset_action);


/*
 * Function:
 *      soc_dcmn_cmic_sbus_timeout_set
 * Purpose:
 *      setting the timeout value of the sbus
 * Parameters:
 *      unit            - Device Number
 *      core_freq_khz   - the freq of the core in khz
 *      schan_timeout   - time in microseconds
 * Returns:
 *      SOC_E_XXX
 */
int soc_dcmn_cmic_sbus_timeout_set(int unit, uint32 core_freq_khz, int schan_timeout);
int soc_dcmn_cmic_pcie_userif_purge_ctrl_init(int unit);
int soc_dcmn_cmic_mdio_config(int unit, int dividend, int divisor, int delay);

/*
 * Function:
 *      soc_dcmn_cmic_mdio_set
 * Purpose:
 *      setting the CMIC MDIO parameters
 * Parameters:
 *      unit            - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_dcmn_cmic_mdio_set(int unit);

#endif /* _SOC_DCMN_CMIC_DRV_H_ */

