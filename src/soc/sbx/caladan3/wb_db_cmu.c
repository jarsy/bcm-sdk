/*
 * $Id: wb_db_cmu.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Warmboot for CMU software state
 *
 * Purpose:
 *     Warm boot module for Counter Management Unit Software
 */

#ifdef BCM_CALADAN3_SUPPORT
#include <shared/bsl.h>

#include <shared/bitop.h>
#include <shared/util.h>
#include <soc/defs.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/cmu.h>
#include <soc/sbx/caladan3/ocm.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <soc/sbx/caladan3/wb_db_cmu.h>
#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>
#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_SOC_STAT

/* #define WB_CMU_DEBUG 1 */

/* macros */
/*--------*/
#define SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit) (soc_sbx_caladan3_wb_cmu_state_scache_info_p[unit])
#define SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_CHECK(unit) ((soc_sbx_caladan3_wb_cmu_state_scache_info_p[unit]) != NULL \
        && (soc_sbx_caladan3_wb_cmu_state_scache_info_p[unit]->init_done == TRUE))

/* required for wb_db_cmn.h macros */
#define SBX_SCACHE_INFO_PTR(unit) (soc_sbx_caladan3_wb_cmu_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((soc_sbx_caladan3_wb_cmu_state_scache_info_p[unit]) != NULL \
        && (soc_sbx_caladan3_wb_cmu_state_scache_info_p[unit]->init_done == TRUE))


/* global data structures */
/*------------------------*/
soc_sbx_caladan3_wb_cmu_state_scache_info_t*
    soc_sbx_caladan3_wb_cmu_state_scache_info_p[SOC_MAX_NUM_DEVICES] = { 0 };


/* static functions */
/*------------------*/

/*
 *  Function
 *     _soc_sbx_caladan3_wb_cmu_state_scache_alloc
 *  Description:
 *     alloc scache for wb soc cmu module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_caladan3_wb_cmu_state_scache_alloc(int unit)
{

    int rv = SOC_E_NONE;
    SOC_INIT_FUNC_DEFS;

    SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit) = sal_alloc(sizeof(soc_sbx_caladan3_wb_cmu_state_scache_info_t),
                                                          "Scache for CMU warm boot");
    if (SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "unable to allocate scache for soc CMU warmboot for unit %d (%d/%s)\n"),
                   unit, rv, soc_errmsg(rv))); 
    }
    SOC_EXIT;
exit:
    SOC_FUNC_RETURN;

}
/*
 *  Function
 *     _soc_sbx_caladan3_wb_cmu_state_scache_free
 *  Description:
 *     free scache for soc caladan3 cmu module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC void
_soc_sbx_caladan3_wb_cmu_state_scache_free(int unit)
{

    SOC_INIT_FUNC_DEFS;

    sal_free(SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit));

    SOC_FUNC_RETURN_VOID;
}

/*
 *  Function
 *     _soc_sbx_caladan3_wb_cmu_state_dump
 *  Description:
 *     CRC the contents of the CMU buffer and display on the console.
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_caladan3_wb_cmu_state_dump(int unit)
{
    soc_sbx_caladan3_cmu_config_t *pCmuCfg;
    int nSegment;
    unsigned int crc;
    soc_sbx_caladan3_wb_cmu_state_scache_info_t *wb_info_ptr = NULL;
    
    SOC_INIT_FUNC_DEFS;
    
    wb_info_ptr = SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "%s CMU Warm boot not initialized for unit %d \n"),
                   FUNCTION_NAME(), unit));
        SOC_EXIT;
    }
    
    pCmuCfg = wb_info_ptr->cmu_cfg;
    
    for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
        if (pCmuCfg->segments[nSegment].nSegmentLimit != 0) {        
            crc = _shr_crc32(0, (unsigned char *)pCmuCfg->segments[nSegment].pSegmentPciBase, pCmuCfg->segments[nSegment].nSegmentLimit);
            
            LOG_INFO(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s unit %d, soc cmu warm boot scache signature segment(%d) limit(%d) 0x%08x.\n"),
                      FUNCTION_NAME(), unit, nSegment, pCmuCfg->segments[nSegment].nSegmentLimit, crc));

#ifdef WB_CMU_DEBUG
           cli_out("%s unit %d, soc cmu warm boot scache signature segment(%d) limit(%d) 0x%08x.\n",
                        FUNCTION_NAME(), unit, nSegment, pCmuCfg->segments[nSegment].nSegmentLimit, crc);
#endif

       }
    }
   SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}


/*
 *  Function
 *       _soc_sbx_caladan3_wb_cmu_state_layout_init
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
_soc_sbx_caladan3_wb_cmu_state_layout_init(int unit, int version, unsigned int *scache_len, 
                                           soc_sbx_caladan3_cmu_config_t *pCmuCfg)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_wb_cmu_state_scache_info_t *wb_info_ptr = NULL;
    int nSegment;

    SOC_INIT_FUNC_DEFS;

    wb_info_ptr = SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "CMU Warm boot not initialized for unit %d \n"),
                   unit));
        SOC_EXIT;
    }

    if(pCmuCfg->segments == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "CMU Warm boot cmu segment state not initialized for unit %d \n"),
                   unit));
        SOC_EXIT;
    }

    switch(version) {
        case SOC_SBX_CALADAN3_WB_CMU_VERSION_1_0:
            *scache_len = 0;
            wb_info_ptr->version = version;
            wb_info_ptr->cmu_cfg = pCmuCfg;

            /* counter storage per segment offset */
            for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
                wb_info_ptr->segmentDataOffs[nSegment] = *scache_len;
                SBX_WB_DB_LAYOUT_INIT_NV(uint64, pCmuCfg->segments[nSegment].nSegmentLimit); 
            }
            break;

        default:
            rv = SOC_E_INTERNAL;
            _SOC_IF_ERR_EXIT(rv);
            /* coverity [dead_error_line] */
            break;
    }
    SOC_EXIT;

exit:
    SOC_FUNC_RETURN;
}

/*
 *  Function
 *       _soc_sbx_caladan3_wb_cmu_store_counters
 *  Description:
 *       write the segment counters to persistent storage
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 *     none
 */
STATIC int 
_soc_sbx_caladan3_wb_cmu_store_counters(int unit) 
{
    soc_sbx_caladan3_cmu_config_t *pCmuCfg;
    soc_sbx_caladan3_wb_cmu_state_scache_info_t *wb_info_ptr = NULL;
    int nSegment;
    int nTotalSize = 0;

    SOC_INIT_FUNC_DEFS;
    wb_info_ptr = SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit);
    pCmuCfg = wb_info_ptr->cmu_cfg;

    for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {

        if (pCmuCfg->segments[nSegment].nSegmentLimit != 0) {
            SBX_WB_DB_SYNC_MEMORY_OFFSET(uint64, 
                                         pCmuCfg->segments[nSegment].nSegmentLimit, 
                                         wb_info_ptr->segmentDataOffs[nSegment],
                                         pCmuCfg->segments[nSegment].pSegmentPciBase);
            nTotalSize += sizeof(uint64) * (pCmuCfg->segments[nSegment].nSegmentLimit);
        }
    }
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "%s CMU warmboot total size(%d) bytes\n"),
              FUNCTION_NAME(), nTotalSize));
#ifdef WB_CMU_DEBUG
    cli_out("CMU warmboot total size (%d) bytes\n", nTotalSize);
#endif
    SOC_EXIT;
 exit:
    SOC_FUNC_RETURN;
}


int soc_sbx_caladan3_wb_cmu_sync(int unit, int arg)
{
    uint32 uRegValue = 0;
    int    nSegment;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg;
    soc_sbx_caladan3_wb_cmu_state_scache_info_t *wb_info_ptr = NULL;
    int rv = SOC_E_NONE;

    SOC_INIT_FUNC_DEFS;

    wb_info_ptr = SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit);
    pCmuCfg = wb_info_ptr->cmu_cfg;

   if (arg == 0) {
       /* Normal sync */
       /* Save counters to persistent storage */
       _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_store_counters(unit));

   } else {
       /* Final Sync  */

       /* disable all ejection */
       for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
           _SOC_IF_ERR_EXIT(READ_CM_SEGMENT_TABLE_CONFIGr(unit, nSegment, &uRegValue));
           soc_reg_field_set(unit, CM_SEGMENT_TABLE_CONFIGr, &uRegValue, DISABLE_STACEf,1);
           _SOC_IF_ERR_EXIT(WRITE_CM_SEGMENT_TABLE_CONFIGr(unit, nSegment, uRegValue));
       }

       for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
           _SOC_IF_ERR_EXIT(READ_CM_BACKGROUND_EJECT_ENABLEr(unit, &uRegValue));
           uRegValue &= (~(1 << nSegment));
           _SOC_IF_ERR_EXIT(WRITE_CM_BACKGROUND_EJECT_ENABLEr(unit, uRegValue));
       }

       _SOC_IF_ERR_EXIT(READ_CM_MANUAL_EJECT_CONFIGr(unit, &uRegValue));
       soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, ENABLEf,0);
       _SOC_IF_ERR_EXIT(WRITE_CM_MANUAL_EJECT_CONFIGr(unit, uRegValue));

       /* increase manual rate to max 100KHz speed ejection */
       _SOC_IF_ERR_EXIT(WRITE_CM_MANUAL_EJECT_RATEr(unit, 100000));

       /* enable manual ejection */
       _SOC_IF_ERR_EXIT(READ_CM_MANUAL_EJECT_CONFIGr(unit, &uRegValue));
       soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, ENABLEf,1);
       _SOC_IF_ERR_EXIT(WRITE_CM_MANUAL_EJECT_CONFIGr(unit, uRegValue));

       for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
           if (pCmuCfg->segments[nSegment].bEnabled) { 
               _SOC_IF_ERR_EXIT(soc_sbx_caladan3_cmu_segment_flush_all(unit, (uint32) nSegment));
               LOG_VERBOSE(BSL_LS_SOC_COMMON,
                           (BSL_META_U(unit,
                                       "CMU Flush segment(%d) base address(0x%08x) during final sync\n"),
                            nSegment,  (uint32)(pCmuCfg->segments[nSegment].pSegmentPciBase)));


               /* Sleep 50ms for thread to process */
               sal_usleep(50000);
           }
       }

       /* Disable manual ejection */
       _SOC_IF_ERR_EXIT(READ_CM_MANUAL_EJECT_CONFIGr(unit, &uRegValue));
       soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, ENABLEf,0);
       _SOC_IF_ERR_EXIT(WRITE_CM_MANUAL_EJECT_CONFIGr(unit, uRegValue));

       sal_usleep(50000);

       /* The thread should process all entries from the ring buffer, wait then disable thread */
       _SOC_IF_ERR_EXIT(soc_sbx_caladan3_cmu_ring_process_thread_stop(unit));

       /* disable CMU ejection_fifo_ready interrupt */
       uRegValue = 0;
       soc_reg_field_set(unit, CM_INTERRUPT_STATUSr, &uRegValue, EJECTION_FIFO_READYf, 0);
       SOC_IF_ERROR_RETURN(WRITE_CM_INTERRUPT_STATUS_MASKr(unit, uRegValue));    

       /* Disable CMIC ch0 */
       _SOC_IF_ERR_EXIT(soc_mem_fifo_dma_stop(unit, 0));

        _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_store_counters(unit));

   }
   rv = _soc_sbx_caladan3_wb_cmu_state_dump(unit);
   if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "CMU state dump failed for unit %d CMU instance: %d (%s)\n"), 
                   unit, rv, soc_errmsg(rv)));
   }

    SOC_EXIT;
exit:
    SOC_FUNC_RETURN;
}
/*
 *  Function
 *       _soc_sbx_caladan3_wb_cmu_restore_counters
 *  Description:
 *       Restore the segment counter to PCI space
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 *     none
 */
STATIC int 
_soc_sbx_caladan3_wb_cmu_restore_counters(int unit) 
{
    soc_sbx_caladan3_cmu_config_t *pCmuCfg;
    soc_sbx_caladan3_wb_cmu_state_scache_info_t *wb_info_ptr = NULL;
    int nSegment;

    SOC_INIT_FUNC_DEFS;
    wb_info_ptr = SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit);
    pCmuCfg = wb_info_ptr->cmu_cfg;

    for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {

        SBX_WB_DB_RESTORE_MEMORY_OFFSET(uint64, 
                                     pCmuCfg->segments[nSegment].nSegmentLimit, 
                                     wb_info_ptr->segmentDataOffs[nSegment],
                                     pCmuCfg->segments[nSegment].pSegmentPciBase);
    }
    SOC_EXIT;
 exit:
    SOC_FUNC_RETURN;
}
/*
 *  Function
 *     _soc_sbx_caladan3_wb_cmu_state_restore
 *  Description:
 *     Restore the soc cmu data from scache
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_caladan3_wb_cmu_state_restore(int unit)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_cmu_config_t *pCmuCfg;
    soc_sbx_caladan3_wb_cmu_state_scache_info_t *wb_info_ptr = NULL;
    soc_sbx_caladan3_cmu_segment_config_t *pSegCfg;
    uint32 uRegValue = 0;
    int nSegment;
    void *pBuf;
    uint32 flags = 0;
    
    
    SOC_INIT_FUNC_DEFS;
    
    wb_info_ptr = SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit);
    
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "CMU Warm boot not initialized for unit %d \n"),
                   unit));
        rv = SOC_E_INIT;
        SOC_EXIT;
    }

    pCmuCfg = wb_info_ptr->cmu_cfg;
    
    switch(wb_info_ptr->version)  {
    case SOC_SBX_CALADAN3_WB_CMU_VERSION_1_0:
        
        /* Copy counter values for each segment to PCI memory location allocated */
        _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_restore_counters(unit));
        
        rv = _soc_sbx_caladan3_wb_cmu_state_dump(unit);
        if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "CMU state dump failed for unit %d CMU instance: %d (%s)\n"), 
                   unit, rv, soc_errmsg(rv)));
        }

        
        for (nSegment=0; nSegment < SOC_SBX_CALADAN3_CMU_NUM_SEGMENT; nSegment++) {
            pSegCfg = &pCmuCfg->segments[nSegment];
            if ((int32)pSegCfg->pSegmentPciBase != SOC_SBX_API_PARAM_NO_CHANGE) {
                /* make sure pci base address is 64-bits aligned */
                pBuf = (char*)pSegCfg->pSegmentPciBase + 7;
                pBuf = (void*)((uint32)pBuf & ~0x7);	
                SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CM_SEGMENT_TABLE_PCI_BASEr(unit, nSegment,
                                                                                   (uint32)pBuf), rv);
                _SOC_IF_ERR_EXIT(rv);
            }

           if (pSegCfg->bSTACEEnabled != SOC_SBX_API_PARAM_NO_CHANGE) {
                if (pSegCfg->bSTACEEnabled) {
                    SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(READ_CM_SEGMENT_TABLE_CONFIGr(unit, nSegment, &uRegValue), rv);
                    _SOC_IF_ERR_EXIT(rv);
                    soc_reg_field_set(unit, CM_SEGMENT_TABLE_CONFIGr, &uRegValue, DISABLE_STACEf,0);
                    SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CM_SEGMENT_TABLE_CONFIGr(unit, nSegment, uRegValue), rv);
                    _SOC_IF_ERR_EXIT(rv);
                }
            }

            if (pSegCfg->bBGEjectEnabled != SOC_SBX_API_PARAM_NO_CHANGE) {
                if (pSegCfg->bBGEjectEnabled) {
                    SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(READ_CM_BACKGROUND_EJECT_ENABLEr(unit, &uRegValue), rv);
                    _SOC_IF_ERR_EXIT(rv);
                    uRegValue |= (1 << nSegment);
                    SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CM_BACKGROUND_EJECT_ENABLEr(unit, uRegValue), rv);
                    _SOC_IF_ERR_EXIT(rv);
                }
            }

        }

        /*== manual ejection */
        if (pCmuCfg->uManualEjectRate == 0) {
            /* background ejection rate is 0, disable all background ejection */
            SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CM_MANUAL_EJECT_RATEr(unit, 0xFFFFFFFF), rv);
            _SOC_IF_ERR_EXIT(rv);
    } else {
        if (pCmuCfg->uManualEjectRate > 100000) {
            /* force the background ejection rate to be less than 100KHZ */
            pCmuCfg->uManualEjectRate = 100000;
        }
        
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CM_MANUAL_EJECT_RATEr(unit, 1000000000/pCmuCfg->uManualEjectRate), rv);
        _SOC_IF_ERR_EXIT(rv);
    }

        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(READ_CM_MANUAL_EJECT_CONFIGr(unit, &uRegValue), rv);
        _SOC_IF_ERR_EXIT(rv);
        soc_reg_field_set(unit, CM_MANUAL_EJECT_CONFIGr, &uRegValue, ENABLEf,
                          (pCmuCfg->uManualEjectRate)?1:0);
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CM_MANUAL_EJECT_CONFIGr(unit, uRegValue), rv);
        _SOC_IF_ERR_EXIT(rv);
    
        
        /* Enable CMU ejection_fifo_ready interrupt */
        uRegValue = 0;
        soc_reg_field_set(unit, CM_INTERRUPT_STATUSr, &uRegValue, EJECTION_FIFO_READYf, 1);
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CM_INTERRUPT_STATUS_MASKr(unit, uRegValue), rv);    
        _SOC_IF_ERR_EXIT(rv);

        /* Start the thread. */
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(soc_sbx_caladan3_cmu_ring_process_thread_start(unit, flags, pCmuCfg->ring.interval), rv);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Failed to start CMU ring process thread on unit %d\n"),
                       FUNCTION_NAME(), unit));
            _SOC_IF_ERR_EXIT(rv);
        }
        
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Restored soc CMU from warm boot scache unit %d loaded %d bytes\n"), 
                     unit, 100));
        
        break;
        
    default:
        rv = SOC_E_INTERNAL;
        _SOC_IF_ERR_EXIT(rv);
        /* coverity [dead_error_line] */
        break;
    }
 exit:
    SOC_CALADAN3_WARMBOOT_RELEASE_HW_MUTEX(rv);
    SOC_FUNC_RETURN;
    
}

int
soc_sbx_caladan3_wb_cmu_state_init(int unit, soc_sbx_caladan3_cmu_config_t *pCmuCfg)
{
    int                     rv = SOC_E_NONE;
    int                     flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int                     exists = 0;
    uint16                  version = SOC_SBX_CALADAN3_WB_CMU_VERSION_CURR;
    uint16                  recovered_version = 0;
    unsigned int            scache_len = 0, calculated_scache_len = 0;
    soc_scache_handle_t     handle = 0;
    uint8                  *scache_ptr = NULL;
    
    soc_sbx_caladan3_wb_cmu_state_scache_info_t *wb_info_ptr = NULL;
    
    SOC_INIT_FUNC_DEFS;
    
    if (SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit)) {
        _soc_sbx_caladan3_wb_cmu_state_scache_free(unit);
    }
    
    _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_state_scache_alloc(unit));
    
    wb_info_ptr = SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        SOC_EXIT;
    }
    
    SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_CMU, 0);

    if (pCmuCfg == NULL) {
        LOG_ERROR(BSL_LS_SOC_STAT,
                  (BSL_META_U(unit,
                              "Warmboot error CMU state not initialized for unit %d \n"),
                   unit));
        SOC_EXIT;
    }
    
    if (SOC_WARM_BOOT(unit)) {
        
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRetrieve, flags,
                                         &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (rv == SOC_E_NONE) {

            LOG_VERBOSE(BSL_LS_SOC_STAT,
                        (BSL_META_U(unit,
                                    "unit %d loading CMU backing store state\n"),
                         unit));

            wb_info_ptr->scache_ptr = scache_ptr;

            _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_state_layout_init(unit, 
                                                                        version,
                                                                        &calculated_scache_len,
                                                                        pCmuCfg));
            
            if(scache_len != calculated_scache_len) {
                LOG_ERROR(BSL_LS_SOC_STAT,
                          (BSL_META_U(unit,
                                      "Calculated Scache length %d is not same as stored length %d\n"),
                           calculated_scache_len, scache_len));
                _SOC_IF_ERR_EXIT(SOC_E_INTERNAL);
            }
            wb_info_ptr->scache_len = scache_len;

            _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_state_restore(unit));

            if (version != recovered_version) {
                /* set up layout for the preferred version */
                _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_state_layout_init(unit,
                                                                            version,
                                                                            &scache_len,
                                                                            pCmuCfg));
                LOG_VERBOSE(BSL_LS_SOC_STAT,
                            (BSL_META_U(unit,
                                        "unit %d CMU reallocate %d bytes warm" 
                                        " boot backing store space\n"), 
                             unit, scache_len));
                
                /* reallocate the warm boot space */
                rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRealloc,
                                                 flags, &scache_len, &scache_ptr,
                                                 version, &recovered_version,
                                                 &exists);
                if (rv != SOC_E_NONE) {
                    LOG_ERROR(BSL_LS_SOC_STAT,
                              (BSL_META_U(unit,
                                          "unable to reallocate %d bytes"
                                          " warm boot space for unit %d" 
                                          " soc CMU instance: %d (%s)\n"), 
                               scache_len, unit, rv, soc_errmsg(rv)));
                    
                    _SOC_IF_ERR_EXIT(rv);
                }
                wb_info_ptr->scache_ptr = scache_ptr;
            }		/* if (version != recovered_version) */

            SOC_EXIT;
            
        } else {
            LOG_ERROR(BSL_LS_SOC_STAT,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for"
                                   " unit %d CMU instance: %d (%s)\n"), unit, rv, soc_errmsg(rv)));
        }
        wb_info_ptr->scache_ptr = scache_ptr;
        soc_sbx_caladan3_wb_cmu_state_scache_info_p[unit]->init_done = TRUE;
        
    } else {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        _SOC_IF_ERR_EXIT(_soc_sbx_caladan3_wb_cmu_state_layout_init(unit, 
                                                                    version, 
                                                                    &calculated_scache_len,
                                                                    pCmuCfg));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_SOC_STAT,
                    (BSL_META_U(unit,
                                "unit %d allocate %d bytes warm boot backing"
                                 " store space\n"), unit, calculated_scache_len));
        scache_len = calculated_scache_len;

        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheCreate, flags, &scache_len,
                                         &scache_ptr, version, &recovered_version, &exists);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_STAT,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                   " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, rv, soc_errmsg(rv)));
            SOC_EXIT;
        } else {
            wb_info_ptr->scache_ptr = scache_ptr;
            wb_info_ptr->scache_len = scache_len;
            SOC_SBX_CALADAN3_WB_CMU_SCACHE_INFO_PTR(unit)->init_done = TRUE;
        }
    }

exit:
    SOC_FUNC_RETURN;
}

#endif /* CALADAN3_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT */
