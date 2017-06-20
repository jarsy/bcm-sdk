/*
 * $Id: wb_db_counter.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: COUNTER warmboot
 *
 * Purpose:
 *     Counter Management Unit
 *     Warm boot support
 */

#ifndef __SOC_SBX_CALADAN3_WB_COUNTER_H__
#define __SOC_SBX_CALADAN3_WB_COUNTER_H__

#include <soc/error.h>
#include <soc/debug.h>
#include <soc/sbx/counter.h>
#include <soc/sbx/caladan3_counter.h>

#ifdef BCM_WARM_BOOT_SUPPORT

/*
 *  Versioning...
 */
#define SOC_SBX_CALADAN3_WB_COUNTER_VERSION_1_0      SOC_SCACHE_VERSION(1, 0)
#define SOC_SBX_CALADAN3_WB_COUNTER_VERSION_CURR     SOC_SBX_CALADAN3_WB_COUNTER_VERSION_1_0


/*
 *  Overall descriptor for VLAN warmboot 
 */
typedef struct soc_sbx_caladan3_wb_counter_state_scache_info_s {
    int                            init_done;
    int                            is_dirty;
    uint16                         version;
    uint8                         *scache_ptr;
    unsigned int                   scache_len;
    uint32                         counter_hw_val_offs;
    uint32                         counter_sw_val_offs;
    uint32                         counter_delta_offs;
} soc_sbx_caladan3_wb_counter_state_scache_info_t;

/*
 *  Function
 *    soc_sbx_caladan3_wb_counter_state_sync
 *  Purpose
 *    Sync the warmboot state for the counter module
 *  Arguments
 *    IN unit = unit number
 *  Results
 *    soc_error_t (cast as int)
 *      SOC_E_NONE for success
 *      SOC_E_* otherwise as appropriate
 *  Notes
 *    Does not report error conditions for many things...
 *
 *    Use this call from anywhere outside of the VLAN module.
 */
int soc_sbx_caladan3_wb_counter_sync(int unit, int arg);

/*
 *  Function
 *    soc_sbx_caladan3_wb_counter_state_init
 *  Purpose
 *    Initialise the warm boot support for counter module.
 *  Arguments
 *    IN unit = unit number
 *  Results
 *    soc_error_t (cast as int)
 *      SOC_E_NONE if success
 *      SOC_E_* otherwise as appropriate
 *  Notes
 */
int soc_sbx_caladan3_wb_counter_state_init(int unit);

#endif /* def BCM_WARM_BOOT_SUPPORT */

#endif /* __SOC_SBX_CALADAN3_WB_DB_COUNTER_H__  */

