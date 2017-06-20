/*
 * $Id: wb_db_tmu.h $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    wb_db_tmu.h
 * Purpose: Caladan3 Packet Parsing Engine drivers Warm Boot support
 */

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/wb_db_cmn.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/hash.h>


#ifdef BCM_CALADAN3_SUPPORT

#ifndef _SOC_SBX_CALADAN3_TMU_WB_DRIVER
#define _SOC_SBX_CALADAN3_TMU_WB_DRIVER
#endif

#ifdef BCM_WARM_BOOT_SUPPORT


/* Versioning */

#define SOC_CALADAN3_TMU_WB_VERSION_1_0      SOC_SCACHE_VERSION(1, 0)
#define SOC_CALADAN3_TMU_WB_VERSION_CURR     SOC_CALADAN3_TMU_WB_VERSION_1_0


/* Overall descriptor for soc TMU warmboot */

typedef struct soc_sbx_tmu_wb_state_scache_info_s {
    int           init_done;
    int           is_dirty;
    uint16        version;
    uint8        *scache_ptr;
    unsigned int  scache_len;
} _soc_sbx_tmu_wb_state_scache_info_t;


/*
 *  Function
 *    soc_sbx_tmu_wb_state_sync
 *  Purpose
 *    Sync the entire field state to the warm boot buffer
 *  Arguments
 *    IN unit = unit number
 *  Results
 *    bcm_error_t (cast as int)
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not report error conditions for many things...
 *
 *    Use this call from anywhere outside of the soc TMU module.
 */
int soc_sbx_tmu_wb_state_sync(int unit);

/*
 *  Function
 *    soc_sbx_tmu_wb_state_init
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
int soc_sbx_tmu_wb_state_init(int unit);

void soc_sbx_tmu_signature(int unit);


#endif /* BCM_WARM_BOOT_SUPPORT */
#endif /* _SOC_SBX_CALADAN3_TMU_WB_DRIVER */
