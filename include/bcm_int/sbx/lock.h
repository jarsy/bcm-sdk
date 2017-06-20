/*
 * $Id: lock.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains locking definitions internal to the BCM library.
 */

#ifndef _BCM_INT_SBX_LOCK_H
#define _BCM_INT_SBX_LOCK_H

#include <soc/sbx/sbx_drv.h>
#include <bcm_int/common/lock.h>


#define BCM_SBX_LOCK(unit) \
        if ( !SOC_IS_SBX_FE(unit) ) { \
	    BCM_LOCK(unit);           \
	}

#define BCM_SBX_UNLOCK(unit) \
        if ( !SOC_IS_SBX_FE(unit) ) {  \
	    BCM_UNLOCK(unit);         \
	}


#define BCM_SBX_UNLOCK_RETURN(unit, error) \
        BCM_SBX_UNLOCK(unit) ; \
        return error;

#endif	/* !_BCM_INT_SBX_LOCK_H */
