/* $SDK/include/bcm_int/sbx/caldan3/wb_db_mirror.h */
/*
 * $Id: wb_db_mirror.h  Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: MIRROR APIs
 *
 * Purpose:
 *     MIRROR API for SBX devices
 *     Warm boot support
 */

#ifndef __BCM_INT_SBX_CALADAN3_WB_DB_MIRROR_H__
#define __BCM_INT_SBX_CALADAN3_WB_DB_MIRROR_H__

#include <bcm/error.h>
#include <bcm/module.h>

#include <bcm_int/common/debug.h>

#ifdef BCM_WARM_BOOT_SUPPORT

/*
 *  Versioning...
 */
#define BCM_CALADAN3_WB_MIRROR_VERSION_1_0      SOC_SCACHE_VERSION(1, 0)
#define BCM_CALADAN3_WB_MIRROR_VERSION_CURR     BCM_CALADAN3_WB_MIRROR_VERSION_1_0


/*
 *  Overall descriptor for MIRROR warmboot 
 */
typedef struct bcm_caladan3_wb_mirror_state_scache_info_s {
    int         init_done;
    int         is_dirty;

    uint16      version;
    uint8       *scache_ptr;
    unsigned int         scache_len;


} bcm_caladan3_wb_mirror_state_scache_info_t;

/*
 *  Function
 *    bcm_caladan3_mirror_all_wb_state_sync
 *  Purpose
 *    Sync the entire mirror state to the warm boot buffer
 *  Arguments
 *    IN unit = unit number
 *  Results
 *    bcm_error_t (cast as int)
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not report error conditions for many things...
 *
 *    Use this call from anywhere outside of the MIRROR module.
 */
int bcm_caladan3_wb_mirror_state_sync(int unit);

/*
 *  Function
 *    bcm_caladan3_wb_mirror_state_init
 *  Purpose
 *    Initialise the warm boot support for mirror APIs
 *  Arguments
 *    IN unit = unit number
 *  Results
 *    bcm_error_t (cast as int)
 *      BCM_E_NONE if success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int bcm_caladan3_wb_mirror_state_init(int unit);

#endif /* def BCM_WARM_BOOT_SUPPORT */

#endif /* __BCM_INT_SBX_CALADAN3_WB_DB_MIRROR_H__  */

