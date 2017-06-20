/*
 * $Id: init.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     hal_vlan.h
 * Purpose:
 *
 */
#include <shared/bsl.h>

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/boot.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <bcm/init.h>
#include <bcm/error.h>

#include <soc/ea/tk371x/ea_drv.h>
#include <soc/ea/tk371x/onu.h>
#include <bcm_int/ea/init.h>


int
_bcm_ea_clear(int unit)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 * 	bcm_ea_info_get
 * Purpose:
 *  Provide unit information to caller
 * Parameters:
 *  unit - ethernet access device
 *  info - (OUT) bcm unit info structure
 */
int
_bcm_ea_info_get(
	    int unit,
	    bcm_info_t *info)
{
#ifdef BCM_TK371X_SUPPORT
	uint16 device;
	int rc = BCM_E_NONE;
	uint16 vendor;
	if (ERROR == _soc_ea_jedec_id_get((uint8)unit, 0, &vendor)){
		LOG_CLI((BSL_META_U(unit,
                                    "_soc_ea_jedec_id_get BCM_E_INTERNAL = %d\n"), BCM_E_INTERNAL));
		rc = BCM_E_INTERNAL;
	}
	info->vendor = (int)vendor;

	if (ERROR == _soc_ea_chip_id_get((uint8)unit, 0, &device)){
		LOG_CLI((BSL_META_U(unit,
                                    "_soc_ea_chip_id_get BCM_E_INTERNAL = %d\n"), BCM_E_INTERNAL));
		rc = BCM_E_INTERNAL;
	}
	info->device = (uint32)device;

	if (ERROR == _soc_ea_revision_id_get((uint8)unit, &info->revision)){
		LOG_CLI((BSL_META_U(unit,
                                    "_soc_ea_revision_id_get BCM_E_INTERNAL = %d\n"), BCM_E_INTERNAL));
		rc = BCM_E_INTERNAL;
	}
	info->capability = 0;
    info->capability |= BCM_INFO_SWITCH;
	info->capability |= BCM_INFO_IPMC;
	return rc;
#endif
	return BCM_E_NONE;	
}

void
_bcm_ea_info_t_init(bcm_info_t *info)
{
	if (info != NULL){
		sal_memset(info, 0, sizeof(bcm_info_t));
	}
}


