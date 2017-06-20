/*
 * $Id: wb_db_counter.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Warmboot for Counter software state
 *
 * Purpose:
 *     Warm boot module for Counter Management Unit Software
 */

#ifdef BCM_CALADAN3_SUPPORT
#include <shared/bitop.h>
#include <shared/util.h>
#include <shared/bsl.h>
#include <soc/defs.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/mem.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <soc/sbx/wb_db_counter.h>
#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>
#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_SOC_STAT

/* #define WB_COUNTER_DEBUG 1 */

/* macros */
/*--------*/
#define SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit) (soc_sbx_caladan3_wb_counter_state_scache_info_p[unit])
#define SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_CHECK(unit) ((soc_sbx_caladan3_wb_counter_state_scache_info_p[unit]) != NULL \
        && (soc_sbx_caladan3_wb_counter_state_scache_info_p[unit]->init_done == TRUE))

/* required for wb_db_cmn.h macros */
#define SBX_SCACHE_INFO_PTR(unit) (soc_sbx_caladan3_wb_counter_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((soc_sbx_caladan3_wb_counter_state_scache_info_p[unit]) != NULL \
        && (soc_sbx_caladan3_wb_counter_state_scache_info_p[unit]->init_done == TRUE))


/* global data structures */
/*------------------------*/
soc_sbx_caladan3_wb_counter_state_scache_info_t*
    soc_sbx_caladan3_wb_counter_state_scache_info_p[SOC_MAX_NUM_DEVICES] = { 0 };


/* static functions */
/*------------------*/

/*
 *  Function
 *     _soc_sbx_caladan3_wb_counter_state_scache_alloc
 *  Description:
 *     alloc scache for wb soc counter module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_caladan3_wb_counter_state_scache_alloc(int unit)
{

    int rv = SOC_E_NONE;
    SOC_INIT_FUNC_DEFS;

    SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit) = sal_alloc(sizeof(soc_sbx_caladan3_wb_counter_state_scache_info_t),
                                                          "Scache for COUNTER warm boot");
    if (SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "unable to allocate scache for soc COUNTER warmboot for unit %d (%d/%s)\n"),
                   unit, rv, soc_errmsg(rv))); 
    }
    SOC_EXIT;
exit:
    SOC_FUNC_RETURN;

}
/*
 *  Function
 *     _soc_sbx_caladan3_wb_counter_state_scache_free
 *  Description:
 *     free scache for soc caladan3 counter module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC void
_soc_sbx_caladan3_wb_counter_state_scache_free(int unit)
{

    SOC_INIT_FUNC_DEFS;

    sal_free(SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit));

    SOC_FUNC_RETURN_VOID;
}

/*
 *  Function
 *     _soc_sbx_caladan3_wb_counter_state_dump
 *  Description:
 *     CRC the contents of the COUNTER buffer and display on the console.
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_caladan3_wb_counter_state_dump(int unit)
{
    soc_sbx_caladan3_wb_counter_state_scache_info_t *wb_info_ptr = NULL;
    uint32 crc;
    soc_control_t *soc;
    int nCounters = 0;

    SOC_INIT_FUNC_DEFS;

    wb_info_ptr = SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "%s COUNTER Warm boot not initialized for unit %d\n"),
                   FUNCTION_NAME(), unit)); 
        SOC_EXIT;
    }

   soc = SOC_CONTROL(unit);

    nCounters = soc->counter_n64;


    crc = _shr_crc32(0, (unsigned char *)soc->counter_hw_val, (nCounters * sizeof(uint64)));

    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "%s unit %d, soc counter warm boot scache signature counter_hw_val size(%d)  0x%08x.\n"),
              FUNCTION_NAME(), unit, nCounters * sizeof(uint64), crc));
    
#ifdef WB_COUNTER_DEBUG
    soc_cm_print("%s unit %d, soc counter warm boot scache signature counter_hw_val size(%d) 0x%08x.\n",
                 FUNCTION_NAME(), unit, nCounters * sizeof(uint64), crc);
#endif

    crc = _shr_crc32(0, (unsigned char *)soc->counter_sw_val, (nCounters * sizeof(uint64)));

   LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "%s unit %d, soc counter warm boot scache signature counter_sw_val size(%d)  0x%08x.\n"),
              FUNCTION_NAME(), unit, nCounters * sizeof(uint64), crc));
    
#ifdef WB_COUNTER_DEBUG
    soc_cm_print("%s unit %d, soc counter warm boot scache signature counter_sw_val size(%d) 0x%08x.\n",
                 FUNCTION_NAME(), unit, nCounters * sizeof(uint64), crc);
#endif

    crc = _shr_crc32(0, (unsigned char *)soc->counter_delta, (nCounters * sizeof(uint64)));

   LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "%s unit %d, soc counter warm boot scache signature counter_delta size(%d)  0x%08x.\n"),
              FUNCTION_NAME(), unit, nCounters * sizeof(uint64), crc));
    
#ifdef WB_COUNTER_DEBUG
    soc_cm_print("%s unit %d, soc counter warm boot scache signature counter_delta size(%d) 0x%08x.\n",
                 FUNCTION_NAME(), unit, nCounters * sizeof(uint64), crc);
#endif
   SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}


/*
 *  Function
 *       _soc_sbx_caladan3_wb_counter_state_layout_init
 *  Description:
 *      Calculates and stores the offsets for persistent storage structures
 *      in the warmboot info structure.
 *  Inputs:
 *     unit - device number
 *     version - version id
 *  Outputs: 
 *     scache_len - length of scache in bytes
 */
STATIC int
_soc_sbx_caladan3_wb_counter_state_layout_init(int unit, int version, unsigned int *scache_len)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_wb_counter_state_scache_info_t *wb_info_ptr = NULL;
    soc_control_t *soc;
    int nCounters64 = 0;

    SOC_INIT_FUNC_DEFS;

    soc = SOC_CONTROL(unit);

    nCounters64 = soc->counter_n64;
 
#ifdef WB_COUNTER_DEBUG
    soc_cm_print("COUNTER warmboot total size (%d) 64 bit words and (%d) 32 bit words\n", nCounters64, soc->counter_n32);
#endif

    wb_info_ptr = SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
       LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "%s COUNTER Warm boot not initialized for unit %d\n"),
                   FUNCTION_NAME(), unit)); 
        SOC_EXIT;
    }

    switch(version) {
        case SOC_SBX_CALADAN3_WB_COUNTER_VERSION_1_0:
            *scache_len = 0;
            wb_info_ptr->version = version;

            wb_info_ptr->counter_hw_val_offs = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(uint64, nCounters64); 
            wb_info_ptr->counter_sw_val_offs = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(uint64, nCounters64);
            wb_info_ptr->counter_delta_offs = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(uint64, nCounters64);                 
            break;

        default:
            rv = SOC_E_INTERNAL;
            _SOC_IF_ERR_EXIT(rv);
            /* coverity[dead_error_line] */
            break;
    }
    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

/*
 *  Function
 *       _soc_sbx_caladan3_wb_counter_store_counters
 *  Description:
 *       write the port and controlled counters to persistent storage
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 *     none
 */
STATIC int 
_soc_sbx_caladan3_wb_counter_store_counters(int unit) 
{
    soc_sbx_caladan3_wb_counter_state_scache_info_t *wb_info_ptr = NULL;
    int nTotalSize = 0;
    int nCounters = 0;
    soc_control_t *soc;

    SOC_INIT_FUNC_DEFS;
    wb_info_ptr = SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit);

    soc = SOC_CONTROL(unit);

    nCounters = soc->counter_n64;
 
    SBX_WB_DB_SYNC_MEMORY_OFFSET(uint64, 
                                 nCounters,
                                 wb_info_ptr->counter_hw_val_offs,
                                 soc->counter_hw_val);
    nTotalSize += sizeof(uint64) * nCounters;
    
    SBX_WB_DB_SYNC_MEMORY_OFFSET(uint64, 
                                 nCounters,
                                 wb_info_ptr->counter_sw_val_offs,
                                 soc->counter_sw_val);
    nTotalSize += sizeof(uint64) * nCounters;
    
    SBX_WB_DB_SYNC_MEMORY_OFFSET(uint64, 
                                 nCounters,
                                 wb_info_ptr->counter_delta_offs,
                                 soc->counter_delta);
    nTotalSize += sizeof(uint64) * nCounters;

    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "%s COUNTER warmboot total size(%d) bytes\n"),
              FUNCTION_NAME(), nTotalSize));
#ifdef WB_COUNTER_DEBUG
    soc_cm_print("COUNTER warmboot total size (%d) bytes\n", nTotalSize);
#endif
    SOC_EXIT;
 exit:
    SOC_FUNC_RETURN;
}


int soc_sbx_caladan3_wb_counter_sync(int unit, int arg)
{
  soc_sbx_caladan3_wb_counter_state_scache_info_t *wb_info_ptr COMPILER_ATTRIBUTE((unused));
    int rv = SOC_E_NONE;

    SOC_INIT_FUNC_DEFS;

    wb_info_ptr = SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit);

   if (arg == 0) {
       /* Normal sync */
       /* Save counters to persistent storage */
       _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_store_counters(unit));

   } else {
       _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_store_counters(unit));
       /* Final Sync - stop thread  */
       _SOC_IF_ERR_EXIT(soc_sbx_counter_stop(unit));

   }
   rv = _soc_sbx_caladan3_wb_counter_state_dump(unit);
   if (rv != SOC_E_NONE) {
       LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "COUNTER state dump failed for unit %d COUNTER instance: %d (%s)\n"),
                   unit, rv, soc_errmsg(rv))); 
   }

    SOC_EXIT;
exit:
    SOC_FUNC_RETURN;
}
/*
 *  Function
 *       _soc_sbx_caladan3_wb_counter_restore_counters
 *  Description:
 *       Restore the port and controlled counters to PCI space
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 *     none
 */
STATIC int 
_soc_sbx_caladan3_wb_counter_restore_counters(int unit) 
{
    soc_sbx_caladan3_wb_counter_state_scache_info_t *wb_info_ptr = NULL;
    soc_control_t *soc;
    int nCounters = 0;

    SOC_INIT_FUNC_DEFS;
    wb_info_ptr = SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit);

   soc = SOC_CONTROL(unit);

    nCounters = soc->counter_n64;
 
    SBX_WB_DB_RESTORE_MEMORY_OFFSET(uint64, 
                                    nCounters, 
                                    wb_info_ptr->counter_hw_val_offs,
                                    soc->counter_hw_val);

    SBX_WB_DB_RESTORE_MEMORY_OFFSET(uint64, 
                                    nCounters, 
                                    wb_info_ptr->counter_sw_val_offs,
                                    soc->counter_sw_val);

    SBX_WB_DB_RESTORE_MEMORY_OFFSET(uint64, 
                                    nCounters, 
                                    wb_info_ptr->counter_delta_offs,
                                    soc->counter_delta);

#ifdef WB_COUNTER_DEBUG
    soc_cm_print("COUNTER warmboot restored counters\n");
#endif
    SOC_EXIT;
 exit:
    SOC_FUNC_RETURN;
}
/*
 *  Function
 *     _soc_sbx_caladan3_wb_counter_state_restore
 *  Description:
 *     Restore the soc counter data from scache
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_caladan3_wb_counter_state_restore(int unit)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_wb_counter_state_scache_info_t *wb_info_ptr = NULL;    
    
    SOC_INIT_FUNC_DEFS;
    
    wb_info_ptr = SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit);
    
    if(wb_info_ptr == NULL)
        {
            LOG_ERROR(BSL_LS_SOC_STAT,
                      (BSL_META_U(unit,
                                  "%s COUNTER Warm boot not initialized for unit %d\n"),
                       FUNCTION_NAME(), unit)); 
            rv = SOC_E_INIT;
            SOC_EXIT;
        }
    
    switch(wb_info_ptr->version)  {
        case SOC_SBX_CALADAN3_WB_COUNTER_VERSION_1_0:
        
            /* Copy counter values to memory location allocated */
            _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_restore_counters(unit));
            
            rv = _soc_sbx_caladan3_wb_counter_state_dump(unit);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_STAT,
                          (BSL_META_U(unit,
                                      "COUNTER state dump failed for unit %d COUNTER instance: %d (%s)\n"),
                           unit, rv, soc_errmsg(rv)));
            }
            
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Restored soc COUNTER from warm boot scache unit %d loaded %d bytes\n"), 
                         unit, 100));
            
            break;
        
      default:
          rv = SOC_E_INTERNAL;
          _SOC_IF_ERR_EXIT(rv);
          /* coverity[dead_error_line] */
          break;
    }
 exit:
    SOC_CALADAN3_WARMBOOT_RELEASE_HW_MUTEX(rv);
    SOC_FUNC_RETURN;
    
}

int
soc_sbx_caladan3_wb_counter_state_init(int unit)
{
    int                     rv = SOC_E_NONE;
    int                     flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int                     exists = 0;
    uint16                  version = SOC_SBX_CALADAN3_WB_COUNTER_VERSION_CURR;
    uint16                  recovered_version = 0;
    unsigned int            scache_len = 0, calculated_scache_len = 0;
    soc_scache_handle_t     handle = 0;
    uint8                  *scache_ptr = NULL;
   
    soc_sbx_caladan3_wb_counter_state_scache_info_t *wb_info_ptr = NULL;

    SOC_INIT_FUNC_DEFS;

    if (SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit)) {
        _soc_sbx_caladan3_wb_counter_state_scache_free(unit);
    }

    _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_state_scache_alloc(unit));

    wb_info_ptr = SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "%s COUNTER Warm boot not initialized for unit %d\n"),
                   FUNCTION_NAME(), unit));
        SOC_EXIT;
    }

    SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_COUNTER, 0);

    if (SOC_WARM_BOOT(unit)) {

        /* WARM BOOT */
        /* fetch the existing warm boot space */
        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRetrieve, flags,
                                         &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (rv == SOC_E_NONE) {
            LOG_INFO(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "loading COUNTER backing store state\n")));

            wb_info_ptr->scache_ptr = scache_ptr;

            _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_state_layout_init(unit, 
                                                                        version,
                                                                        &calculated_scache_len));
            
            if(scache_len != calculated_scache_len) {
                LOG_ERROR(BSL_LS_SOC_STAT,
                          (BSL_META_U(unit,
                                      "Calculated Scache length %d is not same as stored length %d\n"),
                           calculated_scache_len, scache_len));
                _SOC_IF_ERR_EXIT(SOC_E_INTERNAL);
            }
            wb_info_ptr->scache_len = scache_len;

            _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_state_restore(unit));

            if (version != recovered_version) {
                /* set up layout for the preferred version */
                _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_state_layout_init(unit,
                                                                            version,
                                                                            &scache_len));
                LOG_INFO(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "COUNTER reallocate %d bytes warm" 
                                     " boot backing store space\n"), 
                          scache_len));
                
                /* reallocate the warm boot space */
                rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRealloc,
                                                 flags, &scache_len, &scache_ptr,
                                                 version, &recovered_version,
                                                 &exists);
                if (rv != SOC_E_NONE) {
                    LOG_ERROR(BSL_LS_SOC_STAT,
                              (BSL_META_U(unit,"unable to reallocate %d bytes"
                                          " warm boot space for unit %d" 
                                          " soc COUNTER instance: %d (%s)\n"), 
                               scache_len, unit, rv, soc_errmsg(rv)));
                    
                    _SOC_IF_ERR_EXIT(rv);
                }
                wb_info_ptr->scache_ptr = scache_ptr;
            }		/* if (version != recovered_version) */

            SOC_EXIT;
            
        } else {
            LOG_ERROR(BSL_LS_SOC_STAT,
                      (BSL_META_U(unit,"unable to get current warm boot state for"
                                  " unit %d COUNTER instance: %d (%s)\n"), 
                       unit, rv, soc_errmsg(rv)));
        }
        SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit)->init_done = TRUE;
        
    } else {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_counter_state_layout_init(unit, 
                                                                    version, 
                                                                    &calculated_scache_len));

        /* set up backing store space */
        LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,"allocate %d bytes warm boot backing"
                             " store space\n"), calculated_scache_len));

        scache_len = calculated_scache_len;

        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheCreate, flags, &scache_len,
                                         &scache_ptr, version, &recovered_version, &exists);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_STAT,
                      (BSL_META_U(unit,"unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, rv, soc_errmsg(rv)));
            SOC_EXIT;
        } else {
            wb_info_ptr->scache_ptr = scache_ptr;
            wb_info_ptr->scache_len = scache_len;
            SOC_SBX_CALADAN3_WB_COUNTER_SCACHE_INFO_PTR(unit)->init_done = TRUE;
        }
    }

exit:
    SOC_FUNC_RETURN;
}

#endif /* CALADAN3_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT */
