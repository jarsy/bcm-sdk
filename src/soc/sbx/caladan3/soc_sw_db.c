/*
 * $Id: soc_sw_db.c,v 1.20 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_SOC_COMMON

#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/scache.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/caladan3/wb_db_cmu.h>
#include <soc/sbx/caladan3/wb_db_ppe.h>
#include <soc/sbx/caladan3/wb_db_cop.h>
#include <soc/sbx/caladan3/tmu/wb_db_tmu.h>
#include <soc/sbx/caladan3/rce.h>
#include <soc/sbx/wb_db_counter.h>
#include <sal/appl/sal.h>
#include <sal/core/libc.h>
#include <sal/core/time.h>

#ifdef BCM_WARM_BOOT_SUPPORT


/* uncomment next line to display warmboot sync times on the console */
/* #define SOC_WB_TIME_STAMP_DBG */

#ifdef SOC_WB_TIME_STAMP_DBG
sal_usecs_t        soc_wb_start;
#define SOC_WB_TIME_STAMP_START soc_wb_start = sal_time_usecs();
#define SOC_WB_TIME_STAMP(msg)                    \
  do { \
    sal_usecs_t diff;                   \
    diff = SAL_USECS_SUB(sal_time_usecs(),soc_wb_start)/1000000; \
    if (diff) \
        LOG_CLI((BSL_META("   %s: %us\n"), msg, diff));        \
    else \
        LOG_CLI((BSL_META("   %s: <1s\n"), msg));   \
  } while(0);
#else
#define SOC_WB_TIME_STAMP_START
#define SOC_WB_TIME_STAMP(msg)
#endif



int
soc_caladan3_sw_db_sync(int unit, int arg) {
    SOC_INIT_FUNC_DEFS;

    SOC_WB_TIME_STAMP_START
    _SOC_IF_ERR_EXIT(soc_sbx_wb_sync(unit, 1 /* sync to persistent storage */));
    SOC_WB_TIME_STAMP("soc_sbx_wb_sync")

    SOC_WB_TIME_STAMP_START
    _SOC_IF_ERR_EXIT(soc_sbx_ppe_wb_state_sync(unit));
    SOC_WB_TIME_STAMP("soc_sbx_ppe_wb_state_sync")

    SOC_WB_TIME_STAMP_START
    _SOC_IF_ERR_EXIT(soc_sbx_cop_wb_state_sync(unit));
    SOC_WB_TIME_STAMP("soc_sbx_cop_wb_state_sync")

    SOC_WB_TIME_STAMP_START
    _SOC_IF_ERR_EXIT(soc_sbx_tmu_wb_state_sync(unit));
    SOC_WB_TIME_STAMP("soc_sbx_tmu_wb_state_sync")

    SOC_WB_TIME_STAMP_START
    _SOC_IF_ERR_EXIT(soc_sbx_caladan3_wb_cmu_sync(unit, arg));
    SOC_WB_TIME_STAMP("soc_sbx_caladan3_wb_cmu_sync")

    SOC_WB_TIME_STAMP_START
    _SOC_IF_ERR_EXIT(soc_c3_rce_wb_immed_sync(unit));
    SOC_WB_TIME_STAMP("soc_c3_rce_wb_immed_sync")

    SOC_WB_TIME_STAMP_START
    _SOC_IF_ERR_EXIT(soc_sbx_caladan3_wb_counter_sync(unit, arg));
    SOC_WB_TIME_STAMP("soc_sbx_caladan3_wb_counter_sync")

    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

int
soc_caladan3_scache_ptr_get(int unit, soc_scache_handle_t handle, soc_caladan3_scache_oper_t oper,
                                 int flags, uint32 *size, uint8 **scache_ptr,
                                 uint16 version, uint16 *recovered_ver, int *already_exists)
{
    int        rc = SOC_E_NONE;  
    uint32     allocated_size;
    uint16     storage_version = version;
    int        alloc_size, incr_size;

    SOC_INIT_FUNC_DEFS;

    SOC_NULL_CHECK(scache_ptr);

    if (oper == socScacheRetrieve) {
        SOC_NULL_CHECK(size);
        SOC_NULL_CHECK(recovered_ver);

        rc = soc_scache_ptr_get(unit, handle, scache_ptr, &allocated_size);
        if (rc != SOC_E_NONE) {
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Failed in the function soc_scache_ptr_get.\n\r")));
           return(rc);
        }
        (*size) = allocated_size;

        sal_memcpy(&version, *scache_ptr, sizeof(uint16));
        (*recovered_ver) = version;

        if (already_exists != NULL) {
            (*already_exists) = TRUE;
        }

        if (storage_version > version) {
            LOG_ERROR(BSL_LS_SOC_SWDB,
                      (BSL_META_U(unit,
                                  "Downgrade detected. Current version=%d.%d  found %d.%d\n"),
                                  SOC_SCACHE_VERSION_MAJOR(version),
                       SOC_SCACHE_VERSION_MINOR(version),
                       SOC_SCACHE_VERSION_MAJOR(storage_version),
                       SOC_SCACHE_VERSION_MINOR(storage_version)));

                /* Notify the application with an event                  */
                /* The application will then need to reconcile the       */
                /* version differences using the documented behavioral   */
                /* differences on per module (handle) basis              */
                SOC_IF_ERROR_RETURN
                    (soc_event_generate(unit,
                                        SOC_SWITCH_EVENT_WARM_BOOT_DOWNGRADE,
                                        handle, storage_version, version));


            if (flags & SOC_CALADAN3_SCACHE_DOWNGRADE_INVALID) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Internal error. DOWNGRADE_INVALID\n\r")));
                return(SOC_E_INTERNAL);
            }
        }
        else if (storage_version < version) {

            /* Individual BCM module implementations are version-aware. The       */
            /* default_ver is the latest version that the module is aware of.     */
            /* Each module should be able to understand versions <= default_ver.  */
            /* The actual recovered_ver is returned to the calling module during  */
            /* warm boot initialization. The individual module needs to parse its */
            /* scache based on the recovered_ver.                                 */

            LOG_VERBOSE(BSL_LS_SOC_SWDB,
                        (BSL_META_U(unit,
                                    "Upgrade scenario supported. Current version=%d.%d  found %d.%d\n"),
                                    SOC_SCACHE_VERSION_MAJOR(version),
                         SOC_SCACHE_VERSION_MINOR(version),
                         SOC_SCACHE_VERSION_MAJOR(storage_version),
                         SOC_SCACHE_VERSION_MINOR(storage_version)));
        }
    }
    else if (oper == socScacheCreate) {
        SOC_NULL_CHECK(size);
        SOC_NULL_CHECK(already_exists);

        if (SOC_SCACHE_HANDLE_MODULE_GET(handle) >= SOC_MODULE_COUNT_START)
            LOG_INFO(BSL_LS_SOC_SWDB,
                     (BSL_META_U(unit,
                                 "MODULE %s  MODULE ID %d: "), soc_sbx_module_name(unit, SOC_SCACHE_HANDLE_MODULE_GET(handle)), SOC_SCACHE_HANDLE_MODULE_GET(handle)));

        SOC_SCACHE_ALIGN_SIZE(*size);
        
        alloc_size = (*size) + 2*SOC_WB_SCACHE_CONTROL_SIZE;

        rc = soc_scache_ptr_get(unit, handle, scache_ptr, &allocated_size);
        if ((rc != SOC_E_NONE) && (rc != SOC_E_NOT_FOUND) ) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Failed in the function soc_scache_ptr_get.\n\r")));
            return(rc);
        }

        if (rc == SOC_E_NONE) { /* already exists */
            (*already_exists) = TRUE;
            if (flags & SOC_CALADAN3_SCACHE_EXISTS_ERROR) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Failed : existance error.\n\r")));
				return(SOC_E_PARAM);
            }

        }
        else { /* need to create */
            (*already_exists) = FALSE;
            rc = soc_scache_alloc(unit, handle, alloc_size);
            if (rc != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Failed in the function soc_scache_alloc.\n\r")));
				return(rc);
            }

            rc = soc_scache_ptr_get(unit, handle, scache_ptr, &allocated_size);
            if (rc != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Failed in the function soc_scache_ptr_get.\n\r")));
				return(rc);

            }
            if ((*scache_ptr) == NULL) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Memory allocation failure.\n\r")));
				return(SOC_E_MEMORY);                
            }
        }

        if (alloc_size != allocated_size) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Allocated size mismatch.\n\r")));
			return(SOC_E_INTERNAL);
        }
        if ((*already_exists) == FALSE) {
            sal_memcpy(*scache_ptr, &version, sizeof(uint16));
        }
        else {
            sal_memcpy(&storage_version, *scache_ptr, sizeof(uint16));
            if (storage_version != version) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Version mismatch.\n\r")));
				return(SOC_E_INTERNAL);

            }
        }

        if (recovered_ver != NULL) {
            (*recovered_ver) = version;
        }
    }
    else if (oper == socScacheRealloc) {
        SOC_NULL_CHECK(size);

        /* get current size */
        rc = soc_scache_ptr_get(unit, handle, scache_ptr, &allocated_size);
        if (rc != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Failed in the function soc_scache_ptr_get.\n\r")));
            return(rc);
        }

        /* allocate new size */
        SOC_SCACHE_ALIGN_SIZE(*size);
        
        alloc_size = (*size) + 2*SOC_WB_SCACHE_CONTROL_SIZE;
        incr_size = alloc_size - allocated_size;

        rc = soc_scache_realloc(unit, handle, incr_size);
        if (rc != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Failed in the function soc_scache_realloc.\n\r")));
			return(rc);
        }

        /* update version */
        rc = soc_scache_ptr_get(unit, handle, scache_ptr, &allocated_size);
        if (rc != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Failed in the function soc_scache_ptr_get.\n\r")));
			return(rc);
        }
        sal_memcpy(&storage_version, *scache_ptr, sizeof(uint16));
        sal_memcpy(*scache_ptr, &version, sizeof(uint16));

        if (recovered_ver != NULL) {
            (*recovered_ver) = storage_version; 
        }
        if (already_exists != NULL) {
            (*already_exists) = TRUE;
        }
    }
    else if (oper == socScacheFreeCreate) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid parameter - oper.\n\r")));
		return SOC_E_PARAM;
		/* only re-alloc supported */
    }
    else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid parameter - oper.\n\r")));
		return SOC_E_PARAM;
		/* only re-alloc supported */
    }

    /* Advance over scache control info */
    (*scache_ptr) += SOC_WB_SCACHE_CONTROL_SIZE;

    
    (*size) = (allocated_size - 2*SOC_WB_SCACHE_CONTROL_SIZE); /* update size */

exit:
    SOC_FUNC_RETURN;

}

void
soc_sbx_warmboot_signatures(int unit)
{
    soc_sbx_drv_signature_show(unit);
    soc_sbx_ppe_signature(unit);
    soc_sbx_cop_signature(unit);
    soc_sbx_tmu_signature(unit);
}


#endif /*BCM_WARM_BOOT_SUPPORT*/

#undef _ERR_MSG_MODULE_NAME

/*need to add flag mechanism to sw db*/
