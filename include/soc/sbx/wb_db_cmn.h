/*
 * $Id: wb_db_cmn.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        wb_db_cmn.h
 * Purpose:     WarmBoot - Level 2 support common file
 */

#ifndef _SOC_SBX_WB_DB_CMN_H_
#define _SOC_SBX_WB_DB_CMN_H_


#include <soc/drv.h>
#include <soc/scache.h>


#if defined(BCM_WARM_BOOT_SUPPORT)




/* RESTORE */
#define SBX_WB_DB_RESTORE_VARIABLE(type, count, var) do {                  \
        (var) = *(type *)SBX_SCACHE_INFO_PTR(unit)->scache_ptr;            \
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += sizeof(type);             \
   } while(0)                                            

#define SBX_WB_DB_RESTORE_VARIABLE_OFFSET(type, count, offset, var) do {      \
        (var) = *(type *)(SBX_SCACHE_INFO_PTR(unit)->scache_ptr + (offset));  \
   } while(0)                                            

#define SBX_WB_DB_RESTORE_ARRAY(type, count, var) do {                     \
        for (array_idx=0; array_idx < (count); array_idx++) {                         \
        (var) = *(type *)SBX_SCACHE_INFO_PTR(unit)->scache_ptr;                                   \
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += sizeof(type);                                    \
        }                                                                           \
   } while(0)                                            

#define SBX_WB_DB_RESTORE_MEMORY(type, count, var) do {                    \
        sal_memcpy((var), SBX_SCACHE_INFO_PTR(unit)->scache_ptr, ((count)*sizeof(type)));                            \
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += ((count)*sizeof(type));                                        \
   } while(0)                                            

#define SBX_WB_DB_RESTORE_MEMORY_OFFSET(type, count, offset, var) do {          \
        sal_memcpy((var), (SBX_SCACHE_INFO_PTR(unit)->scache_ptr + (offset)), ((count)*sizeof(type))); \
   } while(0) 


/* SYNC */
#define SBX_WB_DB_SYNC_VARIABLE(type, count, var) do {                     \
        *(type *)SBX_SCACHE_INFO_PTR(unit)->scache_ptr = (var);                                   \
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += sizeof(type);                                    \
   } while(0)                                            

#define SBX_WB_DB_SYNC_VARIABLE_OFFSET(type, count, offset, var) do {                     \
        *(type *)(SBX_SCACHE_INFO_PTR(unit)->scache_ptr + (offset))= (var);                                   \
   } while(0)                                            

#define SBX_WB_DB_SYNC_ARRAY(type, count, var) do {                        \
        for (array_idx=0; array_idx < (count); array_idx++) {                         \
        *(type *)SBX_SCACHE_INFO_PTR(unit)->scache_ptr = (var);                                   \
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += sizeof(type);                                    \
        }                                                                           \
   } while(0)                                            

#define SBX_WB_DB_SYNC_MEMORY(type, count, var) do {                       \
        sal_memcpy(SBX_SCACHE_INFO_PTR(unit)->scache_ptr, (var), ((count)*sizeof(type)));                            \
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += ((count)*sizeof(type));                                           \
   } while(0)                                            


#define SBX_WB_DB_SYNC_MEMORY_OFFSET(type, count, offset, var) do {      \
        sal_memcpy((SBX_SCACHE_INFO_PTR(unit)->scache_ptr + (offset)), var, ((count)*sizeof(type))); \
   } while(0)                                            


#define SBX_WB_DB_GET_SCACHE_PTR(var) do {                       \
        var = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;             \
    } while(0)

#define SBX_WB_DB_MOVE_SCACHE_PTR(type, count) do {                       \
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += ((count)*sizeof(type));                                           \
    } while(0)

#define SBX_WB_DB_SYNC_VARIABLE_WITH_PTR(type, count, var, ptr) do {                       \
        *(type *)ptr = (var);                                   \
    } while(0)


/* INIT */
#define SBX_WB_DB_LAYOUT_INIT(type, count, var) do {                       \
        *scache_len += sizeof(type)*(count);                                           \
   } while(0)                                            

#define SBX_WB_DB_LAYOUT_INIT_NV(type, count) do {                       \
        *scache_len += sizeof(type)*(count);                                           \
   } while(0)                                            




#define SBX_WB_DEV_DIRTY_BIT_IS_SET(unit)                                    \
                        (SOC_CONTROL(unit)->scache_dirty == 1)                        \

#define SBX_WB_DEV_DIRTY_BIT_SET(unit)                                       \
                        SOC_CONTROL_LOCK(unit);                                       \
                        SOC_CONTROL(unit)->scache_dirty = 1;                          \
                        SOC_CONTROL_UNLOCK(unit);

#define SBX_WB_DEV_DIRTY_BIT_CLEAR(unit)                                     \
                        SOC_CONTROL_LOCK(unit);                                       \
                        SOC_CONTROL(unit)->scache_dirty = 0;                          \
                        SOC_CONTROL_UNLOCK(unit);




#endif /* BCM_WARM_BOOT_SUPPORT */

#endif /* _SOC_SBX_WB_DB_CMN_H_ */
