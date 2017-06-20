/*
 * $Id: wb_db_cmu.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: CMU warmboot
 *
 * Purpose:
 *     Counter Management Unit
 *     Warm boot support
 */

#ifndef __SOC_SBX_CALADAN3_WB_CMU_H__
#define __SOC_SBX_CALADAN3_WB_CMU_H__

#include <soc/error.h>
#include <soc/debug.h>
#include <soc/sbx/caladan3/cmu.h>

#ifdef BCM_WARM_BOOT_SUPPORT

/*
 *  Versioning...
 */
#define SOC_SBX_CALADAN3_WB_CMU_VERSION_1_0      SOC_SCACHE_VERSION(1, 0)
#define SOC_SBX_CALADAN3_WB_CMU_VERSION_CURR     SOC_SBX_CALADAN3_WB_CMU_VERSION_1_0


/*
 *  Overall descriptor for VLAN warmboot 
 */
typedef struct soc_sbx_caladan3_wb_cmu_state_scache_info_s {
    int                            init_done;
    int                            is_dirty;
    uint16                         version;
    uint8                         *scache_ptr;
    unsigned int                   scache_len;
    uint32                         segmentDataOffs[SOC_SBX_CALADAN3_CMU_NUM_SEGMENT];
    soc_sbx_caladan3_cmu_config_t *cmu_cfg;

} soc_sbx_caladan3_wb_cmu_state_scache_info_t;

/*
 *  Function
 *    soc_sbx_caladan3_wb_cmu_state_sync
 *  Purpose
 *    Sync the warmboot state for the cmu module
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
int soc_sbx_caladan3_wb_cmu_sync(int unit, int arg);

/*
 *  Function
 *    soc_sbx_caladan3_wb_cmu_state_init
 *  Purpose
 *    Initialise the warm boot support for cmu module.
 *  Arguments
 *    IN unit = unit number
 *  Results
 *    soc_error_t (cast as int)
 *      SOC_E_NONE if success
 *      SOC_E_* otherwise as appropriate
 *  Notes
 */
int soc_sbx_caladan3_wb_cmu_state_init(int unit, soc_sbx_caladan3_cmu_config_t *pCmuCfg);

#endif /* def BCM_WARM_BOOT_SUPPORT */

#endif /* __SOC_SBX_CALADAN3_WB_DB_CMU_H__  */

