/*
 * $Id: g3p1_tmu_hash.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * g3p1_tmu.c: Guadalupe2k V1.3 TMU table manager & wrappers
 *
 */

#include <soc/types.h>
#include <soc/drv.h>
#include <sal/core/boot.h>

#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT) 
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>


int soc_sbx_g3p1_hash_add_ext(int unit, soc_sbx_tmu_hash_handle_t handle, uint32 *key, uint32 *value)
{
    return soc_sbx_caladan3_tmu_hash_entry_add(unit, handle, key, value);    
}

int soc_sbx_g3p1_hash_update_ext(int unit, soc_sbx_tmu_hash_handle_t handle, uint32 *key, uint32 *value)
{
    return soc_sbx_caladan3_tmu_hash_entry_update(unit, handle, key, value);
}

int soc_sbx_g3p1_hash_delete_ext(int unit, soc_sbx_tmu_hash_handle_t handle, uint32 *key)
{
    return soc_sbx_caladan3_tmu_hash_entry_delete(unit, handle, key);
}

int soc_sbx_g3p1_hash_get_ext(int unit, soc_sbx_tmu_hash_handle_t handle, uint32 *key, uint32 *value)
{
    return soc_sbx_caladan3_tmu_hash_entry_get(unit, handle, key, value);
}

int soc_sbx_g3p1_hash_hw_get_ext(int unit, soc_sbx_tmu_hash_handle_t handle, uint32 *key, uint32 *value)
{
    return soc_sbx_caladan3_tmu_hash_entry_hw_get(unit, handle, key, value);
}

#endif /* BCM_CALADAN3_SUPPORT */
