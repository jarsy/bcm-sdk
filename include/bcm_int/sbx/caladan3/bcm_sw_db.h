/*
 * $Id: wb_db_init.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: INIT APIs
 *
 * Purpose:
 *     INIT API for Dune Packet Processor devices
 *     Warm boot support
 */

#ifndef __BCM_INT_SBX_CALADAN3_BCM_SW_DB_H__
#define __BCM_INT_SBX_CALADAN3_BCM_SW_DB_H__

#include <bcm/error.h>
#include <bcm/module.h>

#include <bcm_int/common/debug.h>

#ifdef BCM_WARM_BOOT_SUPPORT

/* 
   modules that does not appear in: include/bcm/module.h 
   NOTE:
*/
enum {
    BCM_MODULE_INIT = BCM_MODULE__COUNT,
    BCM_MODULE_ALLOCATOR,
    BCM_MODULE_L2_CACHE,

    /* Make sure you update the BCM_CALADAN3_MODULE_NAMES_INITIALIZER */
    BCM_CALADAN3_MODULE__COUNT
};

#define BCM_CALADAN3_MODULE_NAMES_INITIALIZER               \
{                                                           \
    "init",                                                 \
    "allocator",                                            \
    "l2cache"                                               \
}
/*
 *  Function
 *    bcm_caladan3_module_name
 *  Purpose
 *   Wrapper function to call the bcm_module_name 
 *  Arguments
 *    IN unit = unit number
 *    IN module_num = MODULE 
 *    OUT char *= module name
 */
char *
bcm_caladan3_module_name(int unit, int module_num);

/*
 *  Function
 *    bcm_caladan3_scache_ptr_get
 *  Purpose
 *   Wrapper function to call the soc_caladan3_scache_ptr_get 
 *  Arguments
 *    IN unit = unit number
 *    IN handle = MODULE and SEQUENCE
 *    IN oper = create, retrieve etc..
 *    IN flags = Type of warm boot capability
 *    IN/OUT size = bytes of memory plus header
 *    OUT scache_ptr = pointer to the allocated/retrived memory    
 *    IN/OUT recovered_ver = version being created or retrieved
 *    OUT already_exists = 
 *  Results
 *      BCM_E_NONE if success
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int bcm_caladan3_scache_ptr_get(int unit, soc_scache_handle_t handle, soc_caladan3_scache_oper_t oper,
                                 int flags, uint32 *size, uint8 **scache_ptr,
                                 uint16 version, uint16 *recovered_ver, int *already_exists);

#endif /* def BCM_WARM_BOOT_SUPPORT */

#endif /* __BCM_INT_SBX_CALADAN3_BCM_SW_DB_H__  */

