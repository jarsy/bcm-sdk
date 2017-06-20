/*
 * $Id: wb_db_stg.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: STG APIs
 *
 * Purpose:
 *     STG API for Dune Packet Processor devices
 *     Warm boot support
 */

#ifndef __BCM_INT_SBX_CALADAN3_WB_DB_STG_H__
#define __BCM_INT_SBX_CALADAN3_WB_DB_STG_H__

#include <bcm/error.h>
#include <bcm/module.h>

#include <bcm_int/common/debug.h>

#ifdef BCM_WARM_BOOT_SUPPORT

/*
 *  Versioning...
 */
#define BCM_CALADAN3_WB_STG_VERSION_1_0      SOC_SCACHE_VERSION(1, 0)
#define BCM_CALADAN3_WB_STG_VERSION_CURR     BCM_CALADAN3_WB_STG_VERSION_1_0


#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_stg_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_stg_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_stg_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Overall descriptor for STG warmboot 
 */
typedef struct bcm_caladan3_wb_stg_state_scache_info_s {
    int         init_done;
    int         is_dirty;

    uint16      version;
    uint8       *scache_ptr;
    unsigned int        scache_len;

    unsigned int        stg_defl_offset;
    unsigned int        stg_bitmap_offset;
    unsigned int        stg_enable_offset;
    unsigned int        stg_state_h_offset;
    unsigned int        stg_state_l_offset;
    unsigned int        stg_count_offset;
    unsigned int        vlan_first_offset;
    unsigned int        vlan_next_offset;

} bcm_caladan3_wb_stg_state_scache_info_t;

#define BCM_CALADAN3_STG_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_stg_state_scache_info_p[unit])
#define BCM_CALADAN3_STG_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_stg_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_stg_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Function
 *    bcm_caladan3_wb_stg_state_init
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
bcm_caladan3_wb_stg_state_init(int unit);

extern int
_bcm_caladan3_wb_stg_state_restore(int unit);

#endif /* def BCM_WARM_BOOT_SUPPORT */

#endif /* __BCM_INT_SBX_CALADAN3_WB_DB_STG_H__  */

