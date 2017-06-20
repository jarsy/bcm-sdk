/*
 * $Id: wb_db_policer.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: POLICER APIs
 *
 * Purpose:
 *     POLICER API for Packet Processor devices
 *     Warm boot support
 */

#ifndef __BCM_INT_SBX_CALADAN3_WB_DB_POLICER_H__
#define __BCM_INT_SBX_CALADAN3_WB_DB_POLICER_H__

#include <bcm/error.h>
#include <bcm/module.h>

#include <bcm_int/common/debug.h>

#ifdef BCM_WARM_BOOT_SUPPORT

/*
 *  Versioning...
 */
#define BCM_CALADAN3_WB_POLICER_VERSION_1_0      SOC_SCACHE_VERSION(1, 0)
#define BCM_CALADAN3_WB_POLICER_VERSION_CURR     BCM_CALADAN3_WB_POLICER_VERSION_1_0


#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_policer_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_policer_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_policer_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Overall descriptor for POLICER warmboot 
 */
typedef struct bcm_caladan3_wb_policer_state_scache_info_s {
    int         init_done;
    int         is_dirty;

    uint16      version;
    uint8       *scache_ptr;
    unsigned int         scache_len;

    unsigned int    group_mode_offset;
    unsigned int    flags_offset;
    unsigned int    base_counter_offset;

} bcm_caladan3_wb_policer_state_scache_info_t;

#define BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_policer_state_scache_info_p[unit])
#define BCM_CALADAN3_POLICER_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_policer_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_policer_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Function
 *    bcm_caladan3_wb_policer_state_init
 *  Purpose
 *    Initialise the warm boot support for field APIs
 *  Arguments
 *    IN unit = unit number
 *  Results
 *    bcm_error_t (cast as int)
 *      BCM_E_NONE if success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
extern int
bcm_caladan3_wb_policer_state_init(int unit);

#endif /* def BCM_WARM_BOOT_SUPPORT */

#endif /* __BCM_INT_SBX_CALADAN3_WB_DB_POLICER_H__  */

